#ifndef MASTER_H
#define MASTER_H

#define _GNU_SOURCE

#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "../../lib/libuv/include/uv.h"
#include "../../lib/thread_pool/thread_pool.h"
#include "backend.h"
#include "config.h"
#include "errors.h"

typedef struct sockaddr_in sockaddr_in;

typedef struct
{
    cache_t* cache;
} garbage_collector_t;

typedef struct master_thread
{
    /*
     * Backend threads are here.
     */
    thread_pool_t child_threads;
    backend_t backends[MAX_BACKEND_NUM];
    size_t cur_backend_count;

    /*
     * Accepting new connections.
     */
    uv_loop_t* serv_loop;
    uv_tcp_t* serv;
    sockaddr_in* addr;

    cache_t cache;
    garbage_collector_t garbage_collector;

    bool was_init;
    size_t total_clients_ever; /* Statistics purpose */
} master_t;

int init_master_thread(master_t* master);

int start_master_thread(master_t* master);

int fini_master_thread(master_t* master);
#endif
