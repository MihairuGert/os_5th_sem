#include "thread_pool.h"

static int 
is_thread_running(pthread_t thread) {
    int result;
    
    result = pthread_tryjoin_np(thread, NULL);
    
    switch (result)
    {
    case 0:
        return 0;
    case EBUSY:
        return 1;
    default:
        return -1;
    }
}

static void*
deamon_routine(void* arg) 
{
    thread_pool_t   *t_pool;
    int             result;
    size_t          i;

    t_pool = (thread_pool_t*) arg;
    while (1)
    {
        pthread_mutex_lock(&t_pool->mutex);
        for (i = 0; i < t_pool->thread_count; i++)
        {
            if (t_pool->free_threads[i])
                continue;
            result = is_thread_running(t_pool->threads[i]);
            if (result == 1) 
                t_pool->free_threads[i] = true;
            if (result < 0) 
                exit(result);
        }
        pthread_mutex_unlock(&t_pool->mutex);

        pthread_testcancel();
        usleep(USLEEP_MAGIC);
    }
    return NULL;
}

static int 
create_deamon(thread_pool_t *t_pool) 
{
    int err;

    err = pthread_create(&t_pool->deamon, NULL, deamon_routine, (void*)t_pool);
    if (err != 0) {
        return err;
    }

    return 0;
}

int 
create_thread_pool(thread_pool_t *t_pool) 
{
    int err;
    size_t i;

    t_pool->free_threads = (bool*) malloc(CAPACITY * sizeof(bool));
    if (!t_pool->free_threads)
        return FREE_THREADS_MALLOC_FAIL;

    for (i = 0; i < CAPACITY; i++) 
    {
        t_pool->free_threads[i] = true;
    }
    
    t_pool->threads = (pthread_t*) malloc(CAPACITY * sizeof(pthread_t));
    if (!t_pool->threads)
        return POOL_THREADS_MALLOC_FAIL;
    
    t_pool->thread_count = CAPACITY;

    err = create_deamon(t_pool);
    if (err != 0)
        return err;

    if (pthread_mutex_init(&t_pool->mutex, NULL) != 0) {
        free(t_pool->threads);
        free(t_pool->free_threads);
        return -1;
    }

    return 0;
}

int 
add_thread( thread_pool_t *t_pool, 
            const pthread_attr_t *attr,
            __typeof__(void *(void *)) *start_routine,
            void *arg)
{
    size_t      i;
    int         err;

    pthread_mutex_lock(&t_pool->mutex);
    begin:
    for (i = 0; i < t_pool->thread_count; i++)
    {
        if (t_pool->free_threads[i]) 
        {   
            err = pthread_create(&t_pool->threads[i], attr, start_routine, arg);
            if (err != 0)
                return err;

            t_pool->free_threads[i] = false;
            pthread_mutex_unlock(&t_pool->mutex);
            
            return 0;
        }
    }

    t_pool->threads = realloc(t_pool->threads, REALLOC_MAGIC * t_pool->thread_count);
    
    t_pool->free_threads = realloc(t_pool->free_threads, REALLOC_MAGIC * t_pool->thread_count);
    for (i = CAPACITY; i < REALLOC_MAGIC * CAPACITY; i++) 
    {
        t_pool->free_threads[i] = true;
    }
    

    t_pool->thread_count *= REALLOC_MAGIC;
    goto begin;
    pthread_mutex_unlock(&t_pool->mutex);

    return 0;
}

int 
destroy_thread_pool(thread_pool_t *t_pool)
{
    void    **retval;
    size_t  i;
    int     err;

    pthread_cancel(t_pool->deamon);
    for (i = 0; i < t_pool->thread_count; i++)
    {
        if (!t_pool->free_threads[i]) 
        {   
            err = pthread_cancel(t_pool->threads[i]);
            if (err != 0)
                return err;

            t_pool->free_threads[i] = true;
        }
    }

    pthread_mutex_destroy(&t_pool->mutex);
    free(t_pool->free_threads);
    free(t_pool->threads);

    return 0;
}