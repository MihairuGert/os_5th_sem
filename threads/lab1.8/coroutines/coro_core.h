#ifndef CORO_CORE_H
#define CORO_CORE_H

#include <ucontext.h>
#include <sys/types.h>
#include <stdbool.h>

/*
 * ucontext_t wrapper struct.
 */
typedef struct {
    ucontext_t      context;
    void            *stack;
} context_t;

/*
 * All coroutine's inner datum.
 */
typedef struct {
    context_t       context;
    
    void            *user_routine;
    void            **retval;
    void            *arg;
} coroutine_info_t;
#endif
