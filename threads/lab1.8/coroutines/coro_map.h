#ifndef CORO_MAP_H
#define CORO_MAP_H

#include "scheduler.h"

#define MAP_CAPACITY 16

typedef struct coro_node {
    pid_t               tid;
    coroutine_t         *coro;
    struct coro_node    *next;
} coro_node_t;

typedef struct {
    coro_node_t **buckets;
    size_t      capacity;
    size_t      size;
} coro_map_t;

int coro_map_create(coro_map_t *map, size_t capacity);
void coro_map_destroy(coro_map_t *map);
int coro_map_add(coro_map_t *map, pid_t tid, coroutine_t *coro);
coroutine_t* coro_map_get(coro_map_t *map, pid_t tid);

#endif
