#include <stdio.h>

#include "thread_pool/thread_pool.h"
#include "coroutines/coroutine.h"

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

    destroy_thread_pool(&t_pool);    

    return 0;
}