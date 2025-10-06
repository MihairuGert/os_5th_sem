#include "coroutine.h"

static int
create_context(context_t *context, void (*routine)(void), int __argc, ...)
{
    void *stack;
    va_list args;

    va_start(args, __argc);

    stack = malloc(STACK_SIZE * sizeof(char));
    if (!stack) 
     return STACK_MALLOC_ERR;
    
    context->stack = stack;

    getcontext(&context->context);
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

static int
scheduler_routine()
{

}

int 
create_scheduler(scheduler_t *scheduler)
{
    
}


