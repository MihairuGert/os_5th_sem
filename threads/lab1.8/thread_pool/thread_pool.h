#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

#define THREADS_MALLOC_ERR      -1
#define MUTEX_INIT_ERR          -2
#define THREADS_REALLOC_ERR     -3
#define POS_NOT_FOUND_ERR       -4

#define CAPACITY        10
#define REALLOC_MAGIC   2
#define USLEEP_MAGIC    10000

/* This enum will be fully used if I'll be asked to
 * implement thread joining. In fact, it would be
 * not quite easy to deal with detached ones. 
 */ 

typedef enum {
    THREAD_FREE = 0,
    THREAD_JOINED_BY_DEAMON,
    THREAD_JOINED_BY_USER,
    THREAD_DETACHED
} thread_state_t;

typedef struct
{
    pthread_t       thread;
    thread_state_t  state;

    void            *retval;
} thread_info_t;

typedef struct
{
    thread_info_t   *threads;
    size_t          thread_count;

    pthread_t       deamon;
    pthread_mutex_t mutex;
} thread_pool_t;

int create_thread_pool(thread_pool_t *t_pool);

int add_thread( thread_pool_t *t_pool, 
            const pthread_attr_t *attr,
            __typeof__(void *(void *)) *start_routine,
            void *arg);
            
int destroy_thread_pool(thread_pool_t *t_pool);
#endif
