#ifndef COROUTINE_H
#define COROUTINE_H

#define _GNU_SOURCE
#include <stdarg.h>
#include <unistd.h>

#include "coro_queue.h"

#define STACK_SIZE      8 * 1024
#define SCHEDULER_MAGIC 10000

#define STACK_MALLOC_ERR    -1
#define CONTEXT_DESTROY_ERR -2
#define CORO_MALLOC_ERR     -3

typedef struct {
    context_t       context;
    coro_queue_t    queue;

    bool            is_done;
} scheduler_t;

typedef struct {
    scheduler_t         *scheduler;
    coroutine_info_t    *info;
} coroutine_t;

int create_scheduler(scheduler_t *scheduler);
int create_coroutine(coroutine_t *coro,
                    scheduler_t *scheduler, 
                    void (*routine)(void*), 
                    void **retval, 
                    void *arg,
                    int __argc, ...);
int yeild(coroutine_t *coro);

#endif
