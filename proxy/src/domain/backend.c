#include "../include/backend.h"

static void alloc_pipe_buf(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);
static void read_pipe(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);

static void new_client(backend_t* backend, uv_tcp_t* client);
static void alloc_backend_buf(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);
static void read_client(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);

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

	err = pthread_mutex_init(&backend->client_count_lock, NULL);
	if (err != 0)
		return err;

    return 0;
}

void *run_backend(void *backend)
{
    int err;
    backend_t *cast_backend = (backend_t*)backend;

    usleep(100000);

    printf("[Backend] Running on thread %lu\n", (unsigned long)pthread_self());
    
    err = uv_pipe_open(&cast_backend->fd_pipe, cast_backend->pipe_fd[0]);
    if (err != 0) {
        close(cast_backend->pipe_fd[0]);
        close(cast_backend->pipe_fd[1]);
        fprintf(stderr, "Failed to open pipe: %s\n", uv_strerror(err));
        return NULL;
    }

    printf("[Backend] Starting event loop, pipe opened\n");
    uv_read_start((uv_stream_t*)&cast_backend->fd_pipe, alloc_pipe_buf, read_pipe);
 	
    err = uv_run(&cast_backend->loop, UV_RUN_DEFAULT);
    if (err != 0) {
        fprintf(stderr, "Backend event loop error: %d\n", err);
    }
    
    printf("[Backend] Finishing\n");
    uv_loop_close(&cast_backend->loop);
    return NULL;
}

int destroy_backend(backend_t* backend)
{
	return 0;
}

static void
alloc_pipe_buf(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
	buf->base = malloc(suggested_size);
	buf->len = suggested_size;
}

static void
read_pipe(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
{
	uv_pipe_t* pipe = (uv_pipe_t*)stream;
	backend_t* backend = pipe->data;
	printf("[Backend] Read pipe callback\n");

	if (nread > 0) {
		if (uv_pipe_pending_count(pipe) > 0) {
			uv_handle_type pending = uv_pipe_pending_type(pipe);

			if (pending == UV_TCP) {
				uv_tcp_t* client = malloc(sizeof(uv_tcp_t));
				uv_tcp_init(&backend->loop, client);
                client->data = backend;

				if (uv_accept((uv_stream_t*)pipe, (uv_stream_t*)client) == 0) {
					printf("Accepted new client via pipe\n");
					new_client(backend, client);
				} else {
					free(client);
				}
			} else {
				fprintf(stderr, "Pipe read error: pending type is not UV_TCP");
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

static void
new_client(backend_t* backend, uv_tcp_t* client)
{
    backend->client_count++;
    uv_read_start((uv_stream_t*) client, alloc_backend_buf, read_client);
}

static void
alloc_backend_buf(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
	buf->base = malloc(suggested_size);
	buf->len = suggested_size;
}

typedef struct {
    uv_write_t req;
    uv_buf_t buf;
} write_req_t;

void free_write_req(uv_write_t *req) {
    write_req_t *wr = (write_req_t*) req;
    free(wr->buf.base);
    free(wr);
}

void on_close(uv_handle_t* handle) {
    int err;
	backend_t* backend = (backend_t*)handle->data;

	err = pthread_mutex_lock(&backend->client_count_lock);
	if (err != 0)
	{
		perror("mutex");
		exit(1);
	}

    backend->client_count--;

	err = pthread_mutex_unlock(&backend->client_count_lock);
	if (err != 0)
	{
		perror("mutex");
		exit(1);
	}

    free(handle);
}

void echo_write(uv_write_t *req, int status) {
    if (status) {
        fprintf(stderr, "Write error %s\n", uv_strerror(status));
    }
    free_write_req(req);
}

static void
read_client(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf)
{
	if (nread > 0) {
        write_req_t *req = (write_req_t*) malloc(sizeof(write_req_t));
        req->buf = uv_buf_init(buf->base, nread);
        uv_write((uv_write_t*) req, client, &req->buf, 1, echo_write);
        return;
    }
    if (nread < 0) {
        if (nread != UV_EOF)
            fprintf(stderr, "Read error %s\n", uv_err_name(nread));
        uv_close((uv_handle_t*) client, on_close);
    }

    free(buf->base);
}
