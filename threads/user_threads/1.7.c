#include <ucontext.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#define STACK_SIZE 8 * 1024 * 1024
#define THREAD_LIMIT 10  

typedef struct 
{
    ucontext_t      context;  
    void            *(*start_routine)(void*);
    void            *arg;
    void            **retval;
    unsigned int    tid;
    int             isActive;
    int             isJoined;
    int             isFinished;
    char            stack[STACK_SIZE];
} uthread;

typedef uthread* uthread_t;

ucontext_t       main_context;
uthread_t        threads[THREAD_LIMIT];
unsigned int     cur_tid = 0;

static int get_free_position() 
{
    size_t i;

    for (i = 0; i < THREAD_LIMIT; i++) 
    {
        if (threads[i] != NULL && threads[i]->isActive == 0) 
            return i;
    }
    return -1;
}

static int get_next_active() 
{
    size_t i;

    for (i = cur_tid + 1 ;; i++) 
    {
        i %= THREAD_LIMIT;
        if (threads[i] != NULL && threads[i]->isActive == 1)
            return i;
        if (i == cur_tid)
            return -1;
    }
    return -1;
}

int yield()
{
    int pos;
    int old_tid; 

    pos = get_next_active();
    if (pos < 0)
        return -1;
    if (pos == cur_tid)
        return 0;
    old_tid = cur_tid;
    cur_tid = pos;

    swapcontext(&threads[old_tid]->context, &threads[pos]->context);
    return 0;
}

int finish() 
{
    int res;
    if (threads[cur_tid]->isJoined == 0)
        threads[cur_tid]->isActive = 0;
    threads[cur_tid]->isFinished = 1;
    res = yield();
    if (res != 0)
        return -1;
    setcontext(&main_context);
}

int uthread_join(uthread_t thread, void** retval) 
{
    if (!thread->isJoined)
        return -1;
    while (thread->isFinished != 1) {
        sleep(1);
    }
    *retval = thread->retval;
    thread->isActive = 0;
}

void thread_wrapper(uthread_t thread) {
    thread->retval = thread->start_routine(thread->arg);
    finish();
}

int uthread_create(uthread_t *thread, void *(start_routine), void *arg) 
{
    int pos;
    pos = get_free_position();
    if (pos < 0) 
    {
        fprintf(stderr, "uthread_create: no threads are free");
        return -1;
    }

    *thread = threads[pos];
    
    (*thread)->start_routine = start_routine;
    (*thread)->arg = arg; 
    (*thread)->isActive = 1;
    (*thread)->isJoined = 1;
    (*thread)->isFinished = 0;
    (*thread)->tid = pos;

    getcontext(&(*thread)->context);
    (*thread)->context.uc_stack.ss_sp = (*thread)->stack;
    (*thread)->context.uc_stack.ss_size = STACK_SIZE;
    (*thread)->context.uc_link = &main_context;

    makecontext(&(*thread)->context, (void(*)())thread_wrapper, 1, threads[pos]);
    return 0;
}

void *hello(void *arg) {
    int*    retval; 
    int     res;
    
    retval = malloc(sizeof(int));
    if(!retval) 
        return NULL;

    *retval = cur_tid;
    printf("hello from userthread %d\n", cur_tid);
    res = yield();
    if (res < 0) 
        printf("hello: yield failed %d\n", res);
    sleep(1);
    printf("hello from userthread %d\n", cur_tid);
    res = yield();
    if (res < 0) 
        printf("hello: yield failed %d\n", res);
    sleep(1);
    printf("hello from userthread %d\n", cur_tid);
    res = yield();
    if (res < 0) 
        printf("hello: yield failed %d\n", res);
    return retval;
}

int uthread_start(uthread_t thread) {
    cur_tid = thread->tid;
    swapcontext(&main_context, &thread->context);
}

int initializeThreads() {
    size_t i;

    for (i = 0; i < THREAD_LIMIT; i++)
    {
        threads[i] = malloc(sizeof(uthread));
        if (!threads[i])
            return -1;
        threads[i]->isActive = 0;
    }
    return 0;
}

int finishThreads() {
    size_t i;
    for (i = 0; i < THREAD_LIMIT; i++)
    {
        free(threads[i]);
    }
}

int main(int argc, char const *argv[])
{
    int res;
    
    res = initializeThreads();
    if (res != 0)
    {
        printf("thread init failed");
        return 1;
    }

    getcontext(&main_context);

    uthread_t thread1;
    res = uthread_create(&thread1, hello, NULL);
    if (res != 0) {
        printf("uthread_create failed\n");
        return 1;
    }

    uthread_t thread2;
    res = uthread_create(&thread2, hello, NULL);
    if (res != 0) {
        printf("uthread_create failed\n");
        return 1;
    }

    uthread_start(thread1);

    printf("main\n");
    void* ret;
    uthread_join(thread1, &ret);
    printf("main got retval = %d", *(int*)ret);
    uthread_join(thread2, &ret);
    printf("main got retval = %d", *(int*)ret);
    finishThreads();
    return 0;
}
