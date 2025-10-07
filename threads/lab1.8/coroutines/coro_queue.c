#include "coro_queue.h"

void 
coro_queue_init(coro_queue_t *queue) 
{
    queue->head = NULL;
    queue->tail = NULL;
    queue->size = 0;
}

bool 
coro_queue_empty(const coro_queue_t *queue) 
{
    return queue->head == NULL;
}

size_t 
coro_queue_size(const coro_queue_t *queue) 
{
    return queue->size;
}

int 
coro_queue_push(coro_queue_t *queue, coroutine_info_t *coro) 
{
    coro_queue_node_t *node;
    
    node = malloc(sizeof(coro_queue_node_t));
    if (!node)
        return NODE_MALLOC_ERR;

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

coroutine_info_t *
coro_queue_pop(coro_queue_t *queue) 
{
    coro_queue_node_t *node;
    coroutine_info_t *coro;

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

coroutine_info_t *
coro_queue_peek(const coro_queue_t *queue) 
{
    if (queue->head == NULL)
        return NULL;
    return queue->head->coro;
}

bool 
coro_queue_remove(coro_queue_t *queue, coroutine_info_t *target_coro) 
{
    coro_queue_node_t *node;
    coro_queue_node_t *current;
    
    if (queue->head == NULL || target_coro == NULL)
        return false;
    
    if (queue->head->coro == target_coro) 
    {
        node = queue->head;
        queue->head = node->next;
        if (queue->tail == node)
            queue->tail = NULL;
        free(node);
        queue->size--;
        return true;
    }
    
    current = queue->head;
    while (current->next != NULL && current->next->coro != target_coro) 
    {
        current = current->next;
    }
    
    if (current->next != NULL && current->next->coro == target_coro) 
    {
        node = current->next;
        current->next = node->next;
        if (queue->tail == node)
            queue->tail = current;
        free(node);
        queue->size--;
        return true;
    }
    
    return false;
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
