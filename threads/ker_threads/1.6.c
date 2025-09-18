#include "1.6.h"

#define GUARD_SIZE 4096
#define STACK_SIZE 8388608

// TODO: move mythread_t to thread's stack

typedef struct mythread_t {
    int     tid;

    void    *(*start_routine)(void*);
    void    *arg;
    void    **retval;

    int     isFinished;
} mythread_t;

static int start_routine_wrapper(void *arg) {
    mythread_t *args;
    
    args = (struct mythread_t*)arg;
    args->retval = args->start_routine(args->arg);
    args->isFinished = 1;
    return 0;
}

int mythread_join(mythread_t *thread, void **retval) {
    while(thread->isFinished != 1) {
        usleep(10);
    }
    retval = thread->retval;
    return 0;
}

int mythread_create(mythread_t *thread, void *(start_routine), void *arg) {
    mythread_t  *args; 
    void        *stack;
    void        *stack_top;
    int         err;
    int         flags;
    pid_t       tid;

    args = thread;

    stack = mmap(NULL, STACK_SIZE + GUARD_SIZE, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_STACK, -1, 0);
    if (stack == MAP_FAILED) {
        fprintf(stderr, "mythread_create: mmap() failed: %s\n", strerror(errno));
        return -1;
    }

    args->start_routine = start_routine;
    args->arg = arg;
    args->isFinished = 0;

    err = mprotect(stack + GUARD_SIZE, STACK_SIZE, PROT_READ|PROT_WRITE);
    if (err) {
        fprintf(stderr, "mythread_create: mprotect() failed: %s\n", strerror(errno));
        munmap(stack, STACK_SIZE + GUARD_SIZE);
        return -1;
    }

    stack_top = stack + GUARD_SIZE + STACK_SIZE;
    flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD | CLONE_SYSVSEM | CLONE_PARENT_SETTID | CLONE_CHILD_CLEARTID;
    tid = clone(start_routine_wrapper, stack_top, flags, args, thread, NULL, thread);
    args->tid = tid;

    if (tid == -1) {
        fprintf(stderr, "mythread_create: clone() failed: %s\n", strerror(errno));
        munmap(stack, STACK_SIZE + GUARD_SIZE);
        return -1;
    }

    return 0;
}

void *hello(void *arg) {
    printf("hello from mythread\n");
    return NULL;
}

int main() {
    mythread_t  thread;
    int         res;
    void        **retval;

    res = mythread_create(&thread, hello, NULL);
    if (res == -1) {
        fprintf(stderr, "Failed to create thread\n");
        return 1;
    }

    mythread_join(&thread, retval);
    return 0;
}