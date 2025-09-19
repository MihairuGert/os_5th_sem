#define _GNU_SOURCE
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <linux/sched.h>    
#include <sys/syscall.h>    
#include <sched.h>

#define GUARD_SIZE 4096
#define STACK_SIZE 8388608

typedef struct mythread
{
    int     tid;

    void    *(*start_routine)(void*);
    void    *arg;
    void    **retval;

    int     isFinished;
} mythread;

typedef mythread* mythread_t;

int mythread_join(mythread_t thread, void **retval);
int mythread_create(mythread_t thread, void *(start_routine), void *arg);
