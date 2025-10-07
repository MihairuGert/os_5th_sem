#ifndef CORO_QUEUE_H
#define CORO_QUEUE_H

#include <stdbool.h>
#include <stdlib.h>

#include "coro_core.h"

#define NODE_MALLOC_ERR -1

typedef struct coro_queue_node_t {
    struct coro_queue_node_t    *next;
    coroutine_info_t            *coro;
} coro_queue_node_t;

typedef struct {
    coro_queue_node_t    *head;
    coro_queue_node_t    *tail;

    size_t              size;
} coro_queue_t;

void coro_queue_init(coro_queue_t *queue);
bool coro_queue_empty(const coro_queue_t *queue);
size_t coro_queue_size(const coro_queue_t *queue);
int coro_queue_push(coro_queue_t *queue, coroutine_info_t *coro);
coroutine_info_t *coro_queue_pop(coro_queue_t *queue);
coroutine_info_t *coro_queue_peek(const coro_queue_t *queue);
bool coro_queue_remove(coro_queue_t *queue, coroutine_info_t *target_coro);
void coro_queue_clear(coro_queue_t *queue);

#endif
