#define _GNU_SOURCE

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "thread_pool/thread_pool.h"
#include "coroutines/coroutine.h"

#define NUM_TASKS 5

typedef struct {
    int task_id;
    scheduler_t* sched;
    coroutine_t* coro;
} coro_args_t;

void count_task(void* arg)
{
    int err;
    coro_args_t* args = (coro_args_t*)arg;
    int task_id = args->task_id;
    coroutine_t* coro = args->coro;
    
    printf("Coroutine %d: Starting count\n", task_id);
    
    for (int i = 1; i <= 3; i++) 
    {
        printf("Coroutine %d: Count %d\n", task_id, i);
        err = yeild(coro);
        if (err != 0)
            exit(err);
        usleep(100000);
    }
    
    printf("Coroutine %d: Finished!\n", task_id);
}

void* thread_worker(void* arg)
{
    int thread_id = *(int*)arg;
    printf("Thread %lu: Starting worker %d\n", pthread_self(), thread_id);
    
    scheduler_t sched;
    coroutine_t coro[NUM_TASKS];
    coro_args_t args[NUM_TASKS];
    int err;
    
    err = create_scheduler(&sched);
    if (err != 0) 
    {
        printf("Thread %d: Failed to create scheduler\n", thread_id);
        pthread_exit((void*)(long)err);
    }
    
    for (size_t i = 0; i < NUM_TASKS; i++) 
    {
        args[i].task_id = thread_id * 100 + i;
        args[i].sched = &sched;
        args[i].coro = &coro[i];
        
        err = create_coroutine(&coro[i], &sched, count_task, &args[i]);
        if (err != 0) 
        {
            printf("Thread %d: Failed to create coroutine\n", thread_id);
            pthread_exit((void*)(long)err);
        }
    }

    run_scheduler(&sched);
    
    printf("Thread %lu: Worker %d completed\n", pthread_self(), thread_id);
    return NULL;
}

int main()
{
    thread_pool_t t_pool;
    int err;
    
    printf("Main: Creating thread pool\n");
    err = create_thread_pool(&t_pool);
    if (err != 0) 
    {
        printf("Main: Failed to create thread pool: %d\n", err);
        return err;
    }
    
    printf("Main: Adding tasks to thread pool\n");
    
    for (int i = 0; i < 1; i++) 
    {
        int* thread_id = malloc(sizeof(int));
        *thread_id = i + 1;
        
        err = add_thread(&t_pool, NULL, thread_worker, thread_id);
        if (err != 0) 
        {
            printf("Main: Failed to add thread %d: %d\n", i, err);
            free(thread_id);
        } else 
        {
            printf("Main: Added task %d to thread pool\n", i + 1);
        }
    }
    
    printf("Main: Waiting for threads to complete...\n");
    sleep(20);
    
    printf("Main: Destroying thread pool\n");
    destroy_thread_pool(&t_pool);
    
    printf("Main: All done!\n");
    return 0;
}
