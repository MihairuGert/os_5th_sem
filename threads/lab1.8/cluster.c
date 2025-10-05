#include <stdio.h>

#include "thread_pool.h"

void *
hello(void* arg)
{
    for (size_t i = 0; i < 30; i++)
    {
        printf("fuck it -> %d\n", *(int*)arg);
        sleep(1);
    }
    return NULL;
}

int 
main()
{
    thread_pool_t   t_pool;
    int             err;
    size_t          i;
    int             tid[100];

    err = create_thread_pool(&t_pool);
    if (err != 0)
        return err;

    for (size_t i = 0; i < 100; i++)
    {
        tid[i] = i;
    }
    
    for (i = 0; i < 100; i++)
    {
        err = add_thread(&t_pool, NULL, hello, &tid[i]);
        if (err < 0)
            return err;
    }

    sleep(3);

    destroy_thread_pool(&t_pool);    

    return 0;
}