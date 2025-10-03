#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "pthread.h"

#define POOL_MALLOC_FAIL            -1
#define FREE_THREADS_MALLOC_FAIL    -2
#define POOL_THREADS_MALLOC_FAIL    -3

#define CAPACITY        10
#define REALLOC_MAGIC   2

typedef struct thread_pool
{
    pthread_t   *threads;
    bool        *free_threads;
    size_t      thread_count;

    pthread_t   *deamon;
} thread_pool_t;

int create_thread_pool(thread_pool_t *t_pool);
int add_thread( thread_pool_t *t_pool, 
            pthread_t *thread,
            const pthread_attr_t *attr,
            typeof(void *(void *)) *start_routine,
            void *arg);
int destroy_thread_pool(thread_pool_t *t_pool);
