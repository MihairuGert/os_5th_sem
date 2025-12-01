#include "../include/backend.h"

static void alloc_pipe_buf(uv_handle_t* handle,
    size_t suggested_size,
    uv_buf_t* buf);
static void read_pipe(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);

static void new_client(backend_t* backend, uv_tcp_t* client);
static void alloc_client_buf(uv_handle_t* handle,
    size_t suggested_size,
    uv_buf_t* buf);
static void read_client(uv_stream_t* stream,
    ssize_t nread,
    const uv_buf_t* buf);
static write_req_t* alloc_write_req_t(backend_t* backend,
    uv_stream_t* client,
    char* path,
    char* data,
    size_t data_size);
static void on_close(uv_handle_t* handle);

static void send_next_chunk(write_req_t* wr);
static void echo_write(uv_write_t* req, int status);
static void echo_write_buffer_load(uv_write_t* req, int status);
static void on_data_ready(uv_async_t* handle);

int create_backend(backend_t* backend)
{
    int err;

    err = uv_loop_init(&backend->loop);
    if (err != 0)
        return -1;

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, backend->pipe_fd) != 0) {
        perror("socketpair");
        return -1;
    }

    err = uv_pipe_init(&backend->loop, &backend->fd_pipe, 1);
    if (err != 0) {
        close(backend->pipe_fd[0]);
        close(backend->pipe_fd[1]);
        return err;
    }

    backend->fd_pipe.data = backend;
    backend->client_count = 0;

    return 0;
}

void* run_backend(void* backend)
{
    int err;
    backend_t* cast_backend = (backend_t*)backend;

    printf("[Backend] Running on thread %lu\n", (unsigned long)pthread_self());

    err = uv_pipe_open(&cast_backend->fd_pipe, cast_backend->pipe_fd[0]);
    if (err != 0) {
        close(cast_backend->pipe_fd[0]);
        close(cast_backend->pipe_fd[1]);
        fprintf(stderr, "Failed to open pipe: %s\n", uv_strerror(err));
        return NULL;
    }

    printf("[Backend] Starting event loop, pipe opened\n");
    uv_read_start((uv_stream_t*)&cast_backend->fd_pipe, alloc_pipe_buf,
        read_pipe);

    err = uv_run(&cast_backend->loop, UV_RUN_DEFAULT);
    if (err != 0) {
        fprintf(stderr, "Backend event loop error: %d\n", err);
    }

    printf("[Backend] Finishing\n");
    uv_loop_close(&cast_backend->loop);
    return NULL;
}

static void read_client(uv_stream_t* client,
    ssize_t nread,
    const uv_buf_t* buf)
{
    int err;
    backend_t* backend = client->data;

    if (nread > 0) {
        http_req_t* http_req = malloc(sizeof(http_req_t));

        err = parse_http_req(http_req, buf->base, nread);
        if (err != 0) {
            fprintf(stderr, "Http req parse error. Skipping client req...\n");
            free(http_req);
            free(buf->base);
            return;
        }

        char* referer = NULL;
        size_t referer_size = 0;
        for (int i = 0; i < http_req->num_headers; i++) {
            if (strncmp("Referer", http_req->headers[i].name,
                    http_req->headers[i].name_len)
                == 0) {
                referer = strndup(http_req->headers[i].value, http_req->headers[i].value_len);
            }
        }

        char* key = malloc(512);
        strncpy(key, http_req->path, http_req->path_len);
        key[http_req->path_len] = '\0';
        if (referer) {
            strcat(key, referer);
            referer_size = strlen(referer);
        }

        key[http_req->path_len + referer_size] = '\0';

        char* data = NULL;
        size_t data_size = 0;
        task_t* task = NULL;

        cache_status_t status = cache_lookup(backend->cache, key, &data, &data_size);
        switch (status) {
        case CACHE_FOUND: {
            printf("[Backend %d] Data found in cache, starting sending for: %s\n",
                gettid(), key);
            write_req_t* req = alloc_write_req_t(backend, client, key, data, data_size);
            uv_write((uv_write_t*)req, client, &req->buf, 1, echo_write);
        } break;
        case CACHE_NOT_FOUND: {
            printf(
                "[Backend %d] Data not found in cache, starting background load "
                "for: %s\n",
                gettid(), key);

            task = malloc(sizeof(task_t));
            task->raw_req = malloc(nread);
            memcpy(task->raw_req, buf->base, nread);

            task->raw_req_size = nread;
            task->req = http_req;
            task->result = 0;
            task->work_req.data = task;
            task->backend = backend;
            task->entry = NULL;
            task->key = key;
            task->should_stop = false;
            task->destroy_count = 0;

            err = launch_bgw(task);
            if (err != 0) {
                fprintf(stderr, "bgw error. Skipping client req...\n");
                free(task->raw_req);
                free(http_req);
                free(task);
                break;
            }
        }
        case CACHE_NEW_LOADED:
        case CACHE_NEW_NOT_LOADED: {
            printf("[Backend %d] Data is loading, starting streaming for: %s\n",
                gettid(), key);
            write_req_t* req = alloc_write_req_t(backend, client, key, data, data_size);

            req->progress.last_offset = 0;
            req->progress.total_size = 0;
            req->progress.is_loading = true;
            req->progress.data = NULL;
            req->progress.data_size = 0;
            req->task = task;

            uv_async_init(&backend->loop, &req->data_ready_async, on_data_ready);
            req->data_ready_async.data = req;
            req->was_async_init = true;

            send_next_chunk(req);
        } break;
        case CACHE_DONE:
            break;
        default: {
            fprintf(stderr, "cache error\n");
            free(http_req);
            break;
        }
        }
        free(buf->base);
        return;
    }

    free(buf->base);
}

static write_req_t* alloc_write_req_t(backend_t* backend,
    uv_stream_t* client,
    char* path,
    char* data,
    size_t data_size)
{
    write_req_t* req = (write_req_t*)malloc(sizeof(write_req_t));
    req->req.data = req;

    req->buf = uv_buf_init(data, data_size);
    req->client = client;
    req->path = strdup(path);
    req->backend = backend;
    req->was_async_init = false;
    req->did_register = false;
    req->write_pending = false;
    req->progress.data = NULL;
    req->task = NULL;

    return req;
}

int destroy_backend(backend_t* backend)
{
    return 0;
}

static void on_data_ready(uv_async_t* handle)
{
    write_req_t* wr = (write_req_t*)handle->data;
    if (wr->write_pending) {
        return;
    }
    printf("[Backend] Data ready notification for %s\n", wr->path);
    send_next_chunk(wr);
}

static void send_next_chunk(write_req_t* wr)
{
    if (wr->write_pending)
        return;
    char* data = NULL;
    size_t data_size = 0;
    bool still_loading = false;

    cache_status_t status = cache_lookup_partial(wr->backend->cache, wr->path, &data, &data_size,
        wr->progress.last_offset, 4096, &still_loading);

    printf(
        "[Backend %d] cache_lookup_partial-> status=%d, data_size=%zu "
        "is_loading=%d\n",
        gettid(), status, data_size, still_loading);

    switch (status) {
    case CACHE_DONE:
    case CACHE_FOUND:
        if (data_size > 0) {
            if (wr->progress.data) {
                free(wr->progress.data);
            }
            wr->progress.data = data;
            wr->progress.data_size = data_size;

            wr->buf = uv_buf_init(data, data_size);
            wr->progress.last_offset += data_size;

            uv_write((uv_write_t*)wr, wr->client, &wr->buf, 1,
                echo_write_buffer_load);
        } else if (!still_loading) {
            printf("[Backend %d] All data sent for %s\n", gettid(), wr->path);
            free_write_req(wr);
        } else {
            printf(
                "[Backend %d] No data available yet for %s, registering for "
                "notifications\n",
                gettid(), wr->path);
            if (!wr->did_register) {
                cache_register_waiter(wr->backend->cache, wr->path,
                    &wr->data_ready_async);
                wr->did_register = true;
            }
        }
        break;

    case CACHE_NOT_FOUND:
        printf("Data not found for %s, registering for notifications...\n",
            wr->path);
        if (!wr->did_register) {
            cache_register_waiter(wr->backend->cache, wr->path,
                &wr->data_ready_async);
            wr->did_register = true;
        }
        break;

    case CACHE_NEW_LOADED:
        printf("[Backend %d] Data still loading for %s\n", gettid(), wr->path);
        if (data_size > 0) {
            printf("[Backend %d] data=%p\n", gettid(), data);
            printf("[Backend %d] Send data to client path=%s\n", gettid(),
                wr->path);
            wr->buf = uv_buf_init(data, data_size);
            wr->progress.last_offset += data_size;
            wr->write_pending = true;
            int r = uv_write((uv_write_t*)wr, wr->client, &wr->buf, 1,
                echo_write_buffer_load);
            if (r < 0) {
                wr->write_pending = false;
            }
        }
        break;
    case CACHE_NEW_NOT_LOADED:
        printf(
            "[Backend %d] No data available yet for %s, registering for "
            "notifications\n",
            gettid(), wr->path);
        if (!wr->did_register) {
            cache_register_waiter(wr->backend->cache, wr->path,
                &wr->data_ready_async);
            wr->did_register = true;
            bool is_still_loading_check = false;
            char* check_data = NULL;
            size_t check_size = 0;
            cache_status_t check_status = cache_lookup_partial(wr->backend->cache, wr->path, &data, 
				&check_size, wr->progress.last_offset, 4096, &is_still_loading_check);
            if (check_size > 0 || !is_still_loading_check) {
                if (check_data)
                    free(check_data);
                send_next_chunk(wr);
                return;
            }
        }
        break;
    case CACHE_ERROR:
        fprintf(stderr, "Cache error for %s\n", wr->path);
        free_write_req(wr);
        break;
    }
}

static void alloc_pipe_buf(uv_handle_t* handle,
    size_t suggested_size,
    uv_buf_t* buf)
{
    buf->base = malloc(suggested_size);
    buf->len = suggested_size;
}

static void read_pipe(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
{
    uv_pipe_t* pipe = (uv_pipe_t*)stream;
    backend_t* backend = pipe->data;

    if (nread > 0) {
        if (uv_pipe_pending_count(pipe) > 0) {
            uv_handle_type pending = uv_pipe_pending_type(pipe);

            if (pending == UV_TCP) {
                uv_tcp_t* client = malloc(sizeof(uv_tcp_t));
                uv_tcp_init(&backend->loop, client);
                client->data = backend;

                if (uv_accept((uv_stream_t*)pipe, (uv_stream_t*)client) == 0) {
                    new_client(backend, client);
                } else {
                    free(client);
                }
            } else {
                fprintf(stderr, "Pipe read error: pending type is not UV_TCP");
                exit(EXIT_FAILURE);
            }
        }
        free(buf->base);
    } else if (nread < 0) {
        if (nread != UV_EOF)
            fprintf(stderr, "Pipe read error: %s\n", uv_strerror(nread));
        uv_close((uv_handle_t*)stream, NULL);
        free(buf->base);
    }
}

static void new_client(backend_t* backend, uv_tcp_t* client)
{
    atomic_fetch_add(&backend->client_count, 1);
    printf("[Backend %d] New client connected, total clients: %d\n", gettid(),
        atomic_load(&backend->client_count));

    uv_read_start((uv_stream_t*)client, alloc_client_buf, read_client);
}

static void alloc_client_buf(uv_handle_t* handle,
    size_t suggested_size,
    uv_buf_t* buf)
{
    buf->base = malloc(suggested_size);
    buf->len = suggested_size;
}

static void on_close(uv_handle_t* handle)
{
    int err;
    backend_t* backend = (backend_t*)handle->data;

    atomic_fetch_add(&backend->client_count, -1);
    printf("[Backend %d] Client disconnected, remaining clients: %d\n", gettid(),
        atomic_load(&backend->client_count));

    free(handle);
}

static void echo_write(uv_write_t* req, int status)
{
    write_req_t* wr = (write_req_t*)req->data;

    if (status && status != UV_ECANCELED) {
        fprintf(stderr, "Write error %s\n", uv_strerror(status));
    }

    printf("[Backend %d] All data sent for %s\n", gettid(), wr->path);

    free_write_req(wr);
}

static void send_next_chunk(write_req_t* wr);

static void echo_write_buffer_load(uv_write_t* req, int status)
{
    write_req_t* wr = (write_req_t*)req->data;
    wr->write_pending = false;

    if (status) {
        if (status != UV_ECANCELED) {
            fprintf(stderr, "Write error %s\n", uv_strerror(status));
        }
        if (status == UV_EPIPE) {
            printf("[Backend %d] Client disconnected (broken pipe) for %s\n",
                gettid(), wr->path);
        }
        free_write_req(wr);
        return;
    }

    if (wr->progress.is_loading || wr->progress.last_offset < wr->progress.total_size) {
        send_next_chunk(wr);
    } else {
        printf("All data sent for %s, total: %zu bytes\n", wr->path,
            wr->progress.last_offset);
        free_write_req(wr);
    }
}

static void on_async_close(uv_handle_t* handle)
{
    write_req_t* wr = (write_req_t*)handle->data;
    if (wr) {
        free(wr);
    }
}

void free_write_req(write_req_t* wr)
{
    if (!wr)
        return;

    printf("[Backend %d] Start closing connection for %s...\n", gettid(),
        wr->path ? wr->path : "unknown");
    if (wr->path && wr->was_async_init) {
        cache_unregister_waiter(wr->backend->cache, wr->path,
            &wr->data_ready_async);
    }
    if (wr->task) {
        cancel_bgw(wr->task);
    }
    if (wr->progress.data) {
        free(wr->progress.data);
        wr->progress.data = NULL;
    }
    if (wr->path) {
        free(wr->path);
        wr->path = NULL;
    }
    if (wr->client && !uv_is_closing((uv_handle_t*)wr->client)) {
        atomic_fetch_add(&wr->backend->client_count, -1);
        uv_close((uv_handle_t*)wr->client, (uv_close_cb)free);
    }
    if (wr->was_async_init) {
        if (!uv_is_closing((uv_handle_t*)&wr->data_ready_async)) {
            wr->data_ready_async.data = wr;
            uv_close((uv_handle_t*)&wr->data_ready_async, on_async_close);
        }
    } else {
        free(wr);
    }
    printf("[Backend %d] Connection closed.\n", gettid());
}