#ifndef MASTER_H
#define MASTER_H

#define _GNU_SOURCE

#include <stdbool.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>

#include "errors.h"
#include "config.h"
#include "backend.h"

#include "../../lib/thread_pool/thread_pool.h"
#include "../../lib/libuv/include/uv.h"

typedef struct sockaddr_in sockaddr_in;

typedef struct master_thread {
    /* 
     * Backend threads are here.
     */
    thread_pool_t   child_threads;
    backend_t       backends[MAX_BACKEND_NUM];
    size_t          cur_backend_count;

    /* 
     * Accepting new connections.
     */
    uv_loop_t   *serv_loop;
    uv_tcp_t    *serv;
    sockaddr_in *addr;

    bool    was_init;
    size_t  total_clients_ever; /* Statistics purpose */
} master_t;

int init_master_thread(master_t *master);

int start_master_thread(master_t *master);

int fini_master_thread(master_t *master);
#endif
