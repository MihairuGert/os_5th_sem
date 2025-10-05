#include "thread_pool.h"

static int 
is_thread_running(thread_info_t* thread) {
    int     result;
    
    result = pthread_tryjoin_np(thread->thread, NULL);

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
            if (t_pool->threads[i].isFree)
                continue;
            result = is_thread_running(&t_pool->threads[i]);
            if (result == 0) 
                t_pool->threads[i].isFree = true;
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
    if (err != 0)
        return err;

    return 0;
}

int 
create_thread_pool(thread_pool_t *t_pool) 
{
    int err;
    size_t i;

    t_pool->threads = (thread_info_t*) malloc(CAPACITY * sizeof(thread_info_t));
    if (!t_pool->threads)
        return THREADS_MALLOC_ERR;

    for (i = 0; i < CAPACITY; i++) 
    {
        t_pool->threads[i].isFree = true;
    }
    
    t_pool->thread_count = CAPACITY;

    err = create_deamon(t_pool);
    if (err != 0)
        return err;

    if (pthread_mutex_init(&t_pool->mutex, NULL) != 0) {
        free(t_pool->threads);
        return MUTEX_INIT_ERR;
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
        if (t_pool->threads[i].isFree) 
        {   
            err = pthread_create(&t_pool->threads[i].thread, attr, start_routine, arg);
            if (err != 0)
                return err;

            t_pool->threads[i].isFree = false;
            pthread_mutex_unlock(&t_pool->mutex);
            
            return i;
        }
    }

    t_pool->threads = realloc(t_pool->threads, sizeof(thread_info_t) * REALLOC_MAGIC * t_pool->thread_count);
    if (!t_pool->threads)
    {
        pthread_mutex_unlock(&t_pool->mutex);
        return THREADS_REALLOC_ERR;
    }
    
    for (i = t_pool->thread_count; i < REALLOC_MAGIC * t_pool->thread_count; i++) 
        t_pool->threads[i].isFree = true;
    

    t_pool->thread_count *= REALLOC_MAGIC;
    goto begin;
    pthread_mutex_unlock(&t_pool->mutex);

    return POS_NOT_FOUND_ERR;
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
        if (!t_pool->threads[i].isFree) 
        {   
            err = pthread_cancel(t_pool->threads[i].thread);
            if (err != 0)
                return err;
        }
    }

    pthread_mutex_destroy(&t_pool->mutex);
    free(t_pool->threads);

    return 0;
}
