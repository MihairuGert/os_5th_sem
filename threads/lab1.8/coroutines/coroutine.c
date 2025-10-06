#include "coroutine.h"

static int
create_context(context_t *context, void (*routine)(void*), void *arg, int __argc, ...)
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
    
    context->context.uc_stack.ss_sp = stack;
    context->context.uc_stack.ss_size = STACK_SIZE;
    context->context.uc_link = NULL;

    makecontext(&context->context, (void (*)())routine, 1 + __argc, arg, args);
    
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
    coroutine_info_t    *coro;

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
create_coroutine(coroutine_t *coro,
                scheduler_t *scheduler, 
                void (*routine)(void*), 
                void **retval, 
                void *arg,
                int __argc, ...)
{
    int         err;
    va_list     args;

    va_start(args, __argc);

    err = create_context(&coro->info->context, routine, arg, __argc, args);
    if (err != 0)
        return err;
    
    err = coro_queue_push(&scheduler->queue, coro->info);
    if (err != 0)
        return err;

    va_end(args);
    return 0;
}

int
yeild(coroutine_t *coro)
{
    int err;

    err = coro_queue_push(&coro->scheduler->queue, coro->info);
    if (err != 0)
        return err;
    
    swapcontext(&coro->info->context.context, &coro->scheduler->context.context);
    return 0;
}

int 
create_scheduler(scheduler_t *scheduler)
{
    int err;

    err = create_context(&scheduler->context, (void (*)(void*))scheduler_routine, scheduler, 0);
    if (err != 0)
        return err;

    coro_queue_init(&scheduler->queue);
    scheduler->is_done = false;
    return 0;
}


