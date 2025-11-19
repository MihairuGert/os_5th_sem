#ifndef BACKEND_H
#define BACKEND_H

#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#include "client.h"
#include "http.h"

#include "../../lib/libuv/include/uv.h"

typedef struct backend_thread
{
    /*
     * Backend gets clients fd from master thread
     * through this pipe asynchronously.
     */
    uv_pipe_t   fd_pipe;
    /*
     * Master sends clients fd to backend thread
     * through this pipe.
     */
    uv_pipe_t   master_pipe;
    int         pipe_fd[2];

    size_t          client_count;
    pthread_mutex_t client_count_lock;

    uv_loop_t loop;
} backend_t;

int create_backend(backend_t *backend);

void *run_backend(void *backend);

int destroy_backend(backend_t *backend);

#endif
