#include "thread_pool.h"

static int 
create_deamon(thread_pool_t *t_pool) 
{

}

int 
create_thread_pool(thread_pool_t *t_pool) 
{
    t_pool = (thread_pool_t*) malloc(sizeof(thread_pool_t));
    
    if (!t_pool)
        return POOL_MALLOC_FAIL;

    t_pool->free_threads = (bool*) malloc(CAPACITY * sizeof(bool));
    
    if (!t_pool->free_threads)
        return FREE_THREADS_MALLOC_FAIL;

    memset(t_pool->free_threads, 1, CAPACITY);
    
    t_pool->threads = (pthread_t*) malloc(CAPACITY * sizeof(pthread_t));

    if (!t_pool->threads)
        return POOL_THREADS_MALLOC_FAIL;

    t_pool->thread_count = 10;

    return 0;
}

pthread_t*
add_thread( thread_pool_t *t_pool, 
            const pthread_attr_t *restrict attr,
            typeof(void *(void *)) *start_routine,
            void *restrict arg) 
{
    pthread_t*  thread;
    size_t      i;
    int         res;

    begin:
    for (i = 0; i < t_pool->thread_count; i++)
    {
        if (t_pool->free_threads[i]) 
        {   
            res = pthread_create(t_pool->threads[i], attr, start_routine, arg);
            if (res != 0)
                return NULL;

            t_pool->free_threads[i] = false;

            return thread;
        }
    }

    t_pool->threads = realloc(t_pool->threads, REALLOC_MAGIC * t_pool->thread_count);
    t_pool->free_threads = realloc(t_pool->free_threads, REALLOC_MAGIC * t_pool->thread_count);
    memset(t_pool->free_threads + t_pool->thread_count, 1, REALLOC_MAGIC * t_pool->thread_count - t_pool->thread_count);
    t_pool->thread_count *= REALLOC_MAGIC;
    goto begin;

    return thread;
}

int 
destroy_thread_pool(thread_pool_t *t_pool)
{
    size_t  i;
    int     res;
    void    **retval;

    for (i = 0; i < t_pool->thread_count; i++)
    {
        if (!t_pool->free_threads[i]) 
        {   
            res = pthread_cancel(t_pool->threads[i]);
            if (res != 0)
                return res;

            t_pool->free_threads[i] = true;
        }
    }

    free(t_pool->free_threads);
    free(t_pool->threads);
    free(t_pool);

    return 0;
}