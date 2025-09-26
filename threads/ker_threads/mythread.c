#include "mythread.h"

static int start_routine_wrapper(void *arg) 
{
    mythread_t args;
    
    args = (mythread_t)arg;
    args->retval = args->start_routine(args->arg);
    args->isFinished = 1;

    return 0;
}

int mythread_join(mythread_t thread, void **retval) 
{
    int res; 

    while(thread->isFinished != 1) 
        usleep(10);
    retval = thread->retval;
    res = munmap(thread->stack, thread->stack_size + GUARD_SIZE);
    if (res != 0)
    {
        fprintf(stderr, "mythread_join: munmap() failed: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

int mythread_create(mythread_t thread, void *(start_routine), void *arg) 
{
    mythread_t  args; 
    void        *stack;
    void        *stack_top;
    int         err;
    int         flags;
    pid_t       tid;

    args = thread;

    stack = mmap(NULL, STACK_SIZE + GUARD_SIZE, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_STACK, -1, 0);
    if (stack == MAP_FAILED) 
    {
        fprintf(stderr, "mythread_create: mmap() failed: %s\n", strerror(errno));
        return -1;
    }

    args->start_routine = start_routine;
    args->arg = arg;
    args->isFinished = 0;
    args->stack = stack;
    args->stack_size = STACK_SIZE;

    err = mprotect(stack + GUARD_SIZE, STACK_SIZE, PROT_READ|PROT_WRITE);
    if (err) 
    {
        fprintf(stderr, "mythread_create: mprotect() failed: %s\n", strerror(errno));
        munmap(stack, STACK_SIZE + GUARD_SIZE);
        return -1;
    }

    stack_top = stack + GUARD_SIZE + STACK_SIZE;
    flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD | CLONE_SYSVSEM | CLONE_PARENT_SETTID | CLONE_CHILD_CLEARTID;
    tid = clone(start_routine_wrapper, stack_top, flags, args, thread, NULL, thread);
    args->tid = tid;

    if (tid == -1) 
    {
        fprintf(stderr, "mythread_create: clone() failed: %s\n", strerror(errno));
        munmap(stack, STACK_SIZE + GUARD_SIZE);
        return -1;
    }

    return 0;
}
