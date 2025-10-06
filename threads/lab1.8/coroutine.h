#ifndef COROUTINE_H
#define COROUTINE_H

#include <ucontext.h>
#include <stdlib.h>
#include <stdarg.h>

#define STACK_SIZE 8 * 1024

#define STACK_MALLOC_ERR -1
#define CONTEXT_DESTROY_ERR -2

typedef struct context {
    ucontext_t      context;
    void            *stack;
} context_t;

typedef struct coroutine_info {
    context_t       context;
    void            *(*start_routine)(void*);
    void            *arg;
    void            **retval;
    
    int             priority;
} coroutine_info_t;

typedef struct scheduler {
    context_t       context;
    coroutine_info **coroutines;
} scheduler_t;

typedef struct coroutine {
    scheduler_t     *scheduler;
    coroutine_info  *info;
} coroutine_t;

int create_scheduler(scheduler_t *scheduler);
int create_coroutine();


#endif

