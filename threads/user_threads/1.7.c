#include <ucontext.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define STACK_SIZE 8 * 1024 * 1024
#define THREAD_LIMIT 10  

typedef struct 
{
    ucontext_t      context;  
    void            *(*start_routine)(void*);
    void            *arg;
    void            **retval;
    unsigned int    tid;
    int             isActive;
    char            stack[STACK_SIZE];
} uthread;

typedef uthread* uthread_t;

ucontext_t       main_context;
uthread_t        threads[THREAD_LIMIT] = {0};
unsigned int     cur_tid = 0;

static int get_free_position() 
{
    size_t i;

    for (i = 0; i < THREAD_LIMIT; i++) 
    {
        if (threads[i]->isActive == 0) 
            return i;
    }
    return -1;
}

static int get_next_active() 
{
    size_t i;

    for (i = cur_tid + 1 ;; i++) 
    {
        if (threads[i]->isActive == 1 || i == cur_tid) 
            return i;

        i %= THREAD_LIMIT;
    }
    return -1;
}

int finish() 
{
    threads[cur_tid]->isActive = 0;
    setcontext(&main_context);
}

int yield()
{
    int pos;

    pos = get_next_active(cur_tid);
    if (pos < 0)
        return -1;
    
    cur_tid = pos;

    swapcontext(&main_context, &threads[pos]->context);
}

int uthread_create(uthread_t thread, void *(start_routine), void *arg) 
{
    int pos;

    pos = get_free_position();
    if (pos < 0) 
    {
        fprintf(stderr, "uthread_create: no threads are free");
        return -1;
    }

    thread = threads[pos];
    
    thread->start_routine = start_routine;
    thread->arg = arg; 
    thread->isActive = 1;

    getcontext(&thread->context);
    thread->context.uc_stack.ss_sp = thread->stack;
    thread->context.uc_stack.ss_size = STACK_SIZE;
    thread->context.uc_link = &main_context;

    cur_tid = pos;
    makecontext(&thread->context, start_routine, 0);

    return 0;
}

void *hello(void *arg) {
    printf("hello from userthread %d\n", cur_tid);
    return NULL;
}

int main(int argc, char const *argv[])
{
    
    return 0;
}
