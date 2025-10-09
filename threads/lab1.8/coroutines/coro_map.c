#include "coro_map.h"
#include <stdlib.h>

static size_t 
hash_tid(unsigned int tid, size_t capacity) 
{
    return tid % capacity;
}

int 
coro_map_create(coro_map_t *map, size_t capacity) 
{
    if (!map)
        return -1;

    if (capacity == 0)
        capacity = MAP_CAPACITY;
    
    map->buckets = calloc(capacity, sizeof(coro_node_t*));
    if (!map->buckets)
        return -1;
    
    map->capacity = capacity;
    map->size = 0;
    
    return 0;
}

void 
coro_map_destroy(coro_map_t *map) 
{
    size_t      i;
    coro_node_t *current;
    coro_node_t *next;
    
    if (!map || !map->buckets)
        return;
    
    for (i = 0; i < map->capacity; i++) {
        current = map->buckets[i];
        while (current != NULL) {
            next = current->next;
            free(current);
            current = next;
        }
    }
    
    free(map->buckets);
    map->buckets = NULL;
    map->capacity = 0;
    map->size = 0;
}

int 
coro_map_add(coro_map_t *map, pid_t tid, coroutine_t *coro) 
{
    size_t      index;
    coro_node_t *current;
    coro_node_t *new_node;
    
    if (!map || !map->buckets || !coro)
        return -1;
    
    index = hash_tid(tid, map->capacity);
    current = map->buckets[index];
    
    while (current != NULL) {
        if (current->tid == tid) {
            current->coro = coro;
            return 0;
        }
        current = current->next;
    }
    
    new_node = malloc(sizeof(coro_node_t));
    if (!new_node)
        return -1;
    
    new_node->tid = tid;
    new_node->coro = coro;
    new_node->next = map->buckets[index];
    map->buckets[index] = new_node;
    map->size++;
    
    return 0;
}

coroutine_t*
coro_map_get(coro_map_t *map, pid_t tid) 
{
    size_t      index;
    coro_node_t *current;
    
    if (!map || !map->buckets)
        return NULL;
    
    index = hash_tid(tid, map->capacity);
    current = map->buckets[index];
    
    while (current != NULL) {
        if (current->tid == tid) {
            return current->coro;
        }
        current = current->next;
    }
    
    return NULL;
}
