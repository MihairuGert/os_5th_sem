#include "coroutine.h"

coro_map_t  cur_coro_map;
int         coro_map_init_cnt;

static int
free_context_stack(context_t *context)
{
    if (!context || !context->stack)
        return CONTEXT_DESTROY_ERR;
    
    free(context->stack);
    return 0;
}

static int 
finish(coroutine_t *coro)
{
    int err;

    err = free_context_stack(&coro->info.context);
    if (err != 0)
        return err;

    err = swapcontext(&coro->info.context.context, &coro->scheduler->context.context);
    if (err != 0)
        return err;

    return 0;
}

static void
coroutine_wrapper(void *arg)
{
    int err;

    coroutine_t *coro = (coroutine_t *)arg;
    void (*user_routine)(void*) = (void (*)(void*))coro->info.user_routine;
    
    if (user_routine) {
        user_routine(coro->info.arg);
        err = finish(coro);
        if (err != 0)
            exit(err);
    }
}

static int
create_context(context_t *context, void (*routine)(void*), void *arg)
{
    void    *stack;
    int     err;

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

    makecontext(&context->context, (void (*)())routine, 1, arg);
    
    return 0;
}

static void
scheduler_routine(scheduler_t *scheduler)
{
    coroutine_t    *coro;

    while (!scheduler->is_done)
    {
        coro = coro_queue_pop(&scheduler->queue);
        if (!coro)
        {
            free(scheduler->context.stack);
            swapcontext(&scheduler->context.context, &scheduler->main_ctx);
        }

        coro_map_add(&cur_coro_map, gettid(), coro);

        swapcontext(&scheduler->context.context, &coro->info.context.context);
    }
}

int 
create_coroutine(coroutine_t *coro,
                scheduler_t *scheduler, 
                void (*routine)(void*), 
                void *arg)
{
    int err;

    if (coro_map_init_cnt <= 0)
        return CORO_INIT_ERR;

    if (!coro || !scheduler)
        return CREATE_CORO_ERR;

    coro->scheduler = scheduler;

    coro->info.arg = arg;
    coro->info.user_routine = routine;

    err = create_context(&coro->info.context, coroutine_wrapper, coro);
    if (err != 0) 
        return err;

    coro->info.context.context.uc_link = &scheduler->context.context;

    err = coro_queue_push(&scheduler->queue, coro);
    if (err != 0)
        return err;

    return 0;
}

int
yeild()
{
    int         err;
    coroutine_t *cur_coro;

    if (coro_map_init_cnt <= 0)
        return CORO_INIT_ERR;

    cur_coro = coro_map_get(&cur_coro_map, gettid());

    err = coro_queue_push(&cur_coro->scheduler->queue, cur_coro);
    if (err != 0)
        return err;

    err = swapcontext(&cur_coro->info.context.context, &cur_coro->scheduler->context.context);
    if (err != 0)
        return err;
    return 0;
}

int
coro_init()
{
    if (coro_map_init_cnt > 0)
        return CORO_INIT_ERR;
    coro_map_init_cnt++;

    coro_map_create(&cur_coro_map, MAP_CAPACITY);
    return 0;
}

int
coro_fini()
{
    if (coro_map_init_cnt <= 0)
        return CORO_FINI_ERR;
    coro_map_init_cnt--;
    
    coro_map_destroy(&cur_coro_map);
    return 0;
}

int 
create_scheduler(scheduler_t *scheduler)
{
    int err;

    if (coro_map_init_cnt <= 0)
        return CORO_INIT_ERR;

    if (!scheduler)
        return CREATE_CORO_ERR;

    err = create_context(&scheduler->context, (void (*)(void*))scheduler_routine, scheduler);
    if (err != 0)
        return err;

    coro_queue_init(&scheduler->queue);
    scheduler->is_done = false;

    return 0;
}

int 
run_scheduler(scheduler_t *scheduler)
{
    int err;

    if (coro_map_init_cnt <= 0)
        return CORO_INIT_ERR;

    err = swapcontext(&scheduler->main_ctx, &scheduler->context.context);
    if (err != 0)
        return err;

    return 0;
}
