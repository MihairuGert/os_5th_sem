#ifndef BACKEND_H
#define BACKEND_H

#define _GNU_SOURCE

#include <curl/curl.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../../lib/libuv/include/uv.h"
#include "cache.h"
#include "config.h"
#include "http.h"

typedef struct {
    char* data;
    size_t data_size;
    size_t last_offset;
    size_t total_size;
    bool is_loading;
} read_progress_t;

typedef struct backend_thread {
    /*
     * Backend gets clients fd from master thread through this pipe
     * asynchronously.
     */
    uv_pipe_t fd_pipe;

    /*
     * Master sends clients fd to backend thread through this pipe.
     */
    uv_pipe_t master_pipe;
    int pipe_fd[2];

    atomic_int client_count;

    cache_t* cache;

    uv_loop_t loop;
} backend_t;

int create_backend(backend_t* backend);

void* run_backend(void* backend);

int destroy_backend(backend_t* backend);

typedef struct {
    uv_work_t work_req;
    char* raw_req;
    size_t raw_req_size;
    backend_t* backend;

    http_req_t* req;
    cache_entry_t* entry;
    char* key;

    bool should_stop;

    int result;
    atomic_int destroy_count;
} task_t;

typedef struct {
    uv_write_t req;
    uv_buf_t buf;
    uv_stream_t* client;
    char* path;
    backend_t* backend;
    read_progress_t progress;

    task_t* task;

    uv_async_t data_ready_async;
    bool was_async_init;
    bool did_register;
    bool write_pending;
} write_req_t;

void free_write_req(write_req_t* req);
int launch_bgw(task_t* task);
int cancel_bgw(task_t* task);
void free_task(task_t* task);

#endif
