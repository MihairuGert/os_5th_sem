#include "coro_queue.h"
#include <stdlib.h>

void 
coro_queue_init(coro_queue_t *queue) 
{
    queue->head = NULL;
    queue->tail = NULL;
    queue->size = 0;
}

int 
coro_queue_push(coro_queue_t *queue, coroutine_t *coro) 
{
    coro_queue_node_t *node;
    
    node = malloc(sizeof(coro_queue_node_t));
    if (!node)
        return -1;

    node->coro = coro;
    node->next = NULL;

    if (queue->tail == NULL) 
    {
        queue->head = node;
        queue->tail = node;
    } 
    else 
    {
        queue->tail->next = node;
        queue->tail = node;
    }
    
    queue->size++;
    return 0;
}

coroutine_t*
coro_queue_pop(coro_queue_t *queue) 
{
    coro_queue_node_t   *node;
    coroutine_t         *coro;

    if (queue->head == NULL)
        return NULL;
    
    node = queue->head;
    coro = node->coro;
    
    queue->head = node->next;
    if (queue->head == NULL) {
        queue->tail = NULL;
    }
    
    free(node);
    queue->size--;
    return coro;
}

void 
coro_queue_clear(coro_queue_t *queue) 
{
    coro_queue_node_t *current;
    coro_queue_node_t *next;
    
    current = queue->head;
    while (current != NULL) 
    {
        next = current->next;
        free(current);
        current = next;
    }
    
    queue->head = NULL;
    queue->tail = NULL;
    queue->size = 0;
}
