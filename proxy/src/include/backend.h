#ifndef BACKEND_H
#define BACKEND_H

#include <stdlib.h>
#include <unistd.h>

#include "client.h"

#include "../../lib/libuv/include/uv.h"

typedef struct
{
    /*
     * Backend gets clients fd from master thread
     * through this pipe asynchronously.
     */
    uv_pipe_t   fd_pipe;
    uv_pipe_t   master_pipe;
    int         pipe_fd[2];

    size_t client_count;

    uv_loop_t loop;
} backend_t;

int create_backend(backend_t *backend);

void *run_backend(void *backend);

int fini_backend(backend_t *backend);

#endif
