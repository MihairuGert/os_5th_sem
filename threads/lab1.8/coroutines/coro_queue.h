#ifndef CORO_QUEUE_H
#define CORO_QUEUE_H

#include <stdbool.h>
#include <stdlib.h>

#include "scheduler.h"

#define NODE_MALLOC_ERR -1

void coro_queue_init(coro_queue_t *queue);
int coro_queue_push(coro_queue_t *queue, coroutine_t *coro);
coroutine_t* coro_queue_pop(coro_queue_t *queue);
void coro_queue_clear(coro_queue_t *queue);

#endif
