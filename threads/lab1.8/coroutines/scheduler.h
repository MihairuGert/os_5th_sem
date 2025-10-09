#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "coro_core.h"

typedef struct scheduler scheduler_t;

typedef struct {
    scheduler_t         *scheduler;
    coroutine_info_t    info;
} coroutine_t;

typedef struct coro_queue_node_t {
    struct coro_queue_node_t    *next;
    coroutine_t                 *coro;
} coro_queue_node_t;

typedef struct {
    coro_queue_node_t       *head;
    coro_queue_node_t       *tail;
    size_t                  size;
} coro_queue_t;

typedef struct scheduler {
    context_t       context;
    coro_queue_t    queue;
    bool            is_done;
    ucontext_t      main_ctx;
} scheduler_t;

#endif