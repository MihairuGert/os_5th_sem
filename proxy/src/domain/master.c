#include "../include/master.h"
#include "../include/backend.h"
#include "../include/config.h"

static int add_backend(master_thread_t *master);
static backend_t *get_backend(master_thread_t *master);

static void on_new_connection(uv_stream_t *server, int status);
static int send_client_to_backend(backend_t* backend, uv_tcp_t* client);
static void on_client_sent_cb(uv_write_t* req, int status);

int 
init_master_thread(master_thread_t *master) 
{
    int err;
    
    err = create_thread_pool(&master->child_threads);
    if (err != 0) {
        return err;
    }

    master->serv_loop = uv_default_loop();
    if (!master->serv_loop) {
        return ERR_CANNOT_INIT_SERVER_SOCKET;
    } 

    master->serv = malloc(sizeof(uv_tcp_t));
    err = uv_tcp_init(master->serv_loop, master->serv);
    if (err != 0) {
        return err;
    }

    master->addr = malloc(sizeof(sockaddr_in));
    memset(master->addr, 0, sizeof(sockaddr_in));
    err = uv_ip4_addr("0.0.0.0", DEFAULT_PORT, master->addr);
    if (err != 0) {
        return err;
    }

    err = uv_tcp_bind(master->serv, (const struct sockaddr*)master->addr, 0);
    if (err != 0) {
        return err;
    }

    master->was_init = true;
    master->cur_backend_count = 0;
    master->total_clients_ever = 0;
    master->serv->data = master;
    
    return 0;
}

int 
start_master_thread(master_thread_t *master)
{
    int err;

    if (!master->was_init) {
        return ERR_CANNOT_START_NOT_INIT;
    }
    
    err = uv_listen((uv_stream_t*) master->serv, BACKLOG, on_new_connection);
    if (err != 0) {
        fprintf(stderr, "Listen error %s\n", uv_strerror(err));
        return ERR_SERVER_LISTEN_ERR;
    }
    return uv_run(master->serv_loop, UV_RUN_DEFAULT);
}

int 
fini_master_thread(master_thread_t *master) 
{
    int err;
    err = destroy_thread_pool(&master->child_threads);
    if (err != 0) {
        errno = EAGAIN;
        return err;
    }

    master->was_init = false;

    return 0;
}

static void 
on_new_connection(uv_stream_t *server, int status) {
    int err;
    master_thread_t *master = server->data;

    printf("New connection!\n");
    master->total_clients_ever++;
    
    if (status < 0) {
        fprintf(stderr, "New connection error %s\n", uv_strerror(status));
        return;
    }

    backend_t* backend = get_backend(master);
    if (!backend) {
        fprintf(stderr, "No backend associated with server\n");
        return;
    }

    uv_tcp_t *client = (uv_tcp_t*) malloc(sizeof(uv_tcp_t));
    uv_tcp_init(server->loop, client);
    
    err = uv_accept(server, (uv_stream_t*) client);
    if (err != 0) {
        fprintf(stderr, "Accept error %s\n", uv_strerror(err));
        free(client);
        return;
    }

    err = send_client_to_backend(backend, client);
    if (err != 0) {
        fprintf(stderr, "Failed to send client to backend\n");
    }
}

static int 
send_client_to_backend(backend_t* backend, uv_tcp_t* client) {
    uv_write_t* write_req;
    uv_buf_t buf;
    char dummy_data[1] = {0};
    int err;

    write_req = (uv_write_t*) malloc(sizeof(uv_write_t));
    if (!write_req) {
        return -1;
    }

    buf = uv_buf_init(dummy_data, sizeof(dummy_data));

    err = uv_write2(write_req, 
                   (uv_stream_t*)&backend->master_pipe, 
                   &buf, 
                   1, 
                   (uv_stream_t*)client,
                   on_client_sent_cb);
    
    if (err != 0) {
        free(write_req);
        fprintf(stderr, "uv_write2 error: %s\n", uv_strerror(err));
        return err;
    }

    printf("Client sent to backend via pipe\n");
    return 0;
}

static void 
on_client_sent_cb(uv_write_t* req, int status) {
    if (status != 0) {
        fprintf(stderr, "Client send error: %s\n", uv_strerror(status));
    } else {
        printf("Client successfully sent to backend\n");
    }
    free(req);
}

static backend_t*
get_backend(master_thread_t *master)
{
    // todo add backend limit and more sophisticated round-robin algo.
    if (master->total_clients_ever > MAX_BACKEND_CONNECTIONS_NUM * master->cur_backend_count) {
        printf("[Master] Adding new backend\n");
        add_backend(master);
    }

    return &master->backends[master->cur_backend_count - 1];
}

static int 
add_backend(master_thread_t *master)
{
    int err;

    backend_t* backend_addr = &master->backends[master->cur_backend_count];

    err = create_backend(backend_addr);
    if (err != 0)
        return err;

    err = uv_pipe_init(master->serv_loop, &backend_addr->master_pipe, 1);
    if (err != 0) {
        fini_backend(backend_addr);
        return err;
    }

    err = uv_pipe_open(&backend_addr->master_pipe, backend_addr->pipe_fd[1]);
    if (err != 0) {
        fini_backend(backend_addr);
        return err;
    }

    backend_addr->master_pipe.data = master;
    
    err = add_thread(&master->child_threads, NULL, run_backend, backend_addr);
    if (err != 0) {
        fini_backend(backend_addr);
        return err;
    }

    master->cur_backend_count++;
    return 0;
}
