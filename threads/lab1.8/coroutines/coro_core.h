#ifndef CORO_CORE_H
#define CORO_CORE_H

#include <ucontext.h>

typedef struct {
    ucontext_t      context;
    void            *stack;
} context_t;

typedef struct {
    context_t       context;
    
    void            *user_routine;
    void            **retval;
    void            *arg;
} coroutine_info_t;
#endif
