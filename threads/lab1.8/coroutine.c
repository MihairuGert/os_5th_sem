#include "coroutine.h"

static int
create_context(context_t *context, void (*routine)(void), int __argc, ...)
{
    void    *stack;
    va_list args;
    int     err;

    va_start(args, __argc);

    stack = malloc(STACK_SIZE * sizeof(char));
    if (!stack) 
     return STACK_MALLOC_ERR;
    
    context->stack = stack;

    err = getcontext(&context->context);
    if (err != 0)
        return err;
    makecontext(&context->context, routine, __argc, args);
    context->context.uc_stack.ss_sp = stack;
    context->context.uc_stack.ss_size = STACK_SIZE;

    va_end(args);
    return 0;
}

static int
destroy_context(context_t *context)
{
    if (!context || !context->stack)
        return CONTEXT_DESTROY_ERR;
    free(context->stack);
    return 0;
}

static void
scheduler_routine(scheduler_t *scheduler)
{
    coroutine_info_t *coro;
    while (!scheduler->is_done)
    {
        coro = coro_queue_pop(&scheduler->queue);
        if (!coro)
        {
            usleep(SCHEDULER_MAGIC);
            continue;
        }

        swapcontext(&scheduler->context.context, &coro->context.context);
    }
}

int 
create_coroutine(scheduler_t *scheduler, 
                    void (*routine)(void), 
                    void **retval, 
                    int __argc, ...)
{
    
}

int 
create_scheduler(scheduler_t *scheduler)
{
    int err;

    err = create_context(&scheduler->context, scheduler_routine, 1, scheduler);
    if (err != 0)
        return err;

    coro_queue_init(&scheduler->queue);
    scheduler->is_done = false;
}


