#include "../include/master.h"

int 
init_master_thread(master_thread_t *master) 
{
    int err;
    
    err = create_thread_pool(&master->child_threads);
    if (err != 0)
        return err;

    master->was_init = true;

    return 0;
}

int 
start_master_thread(master_thread_t *master)
{
    if (!master->was_init)
        return ERR_CANNOT_START_NOT_INIT;
    return 0;
}

int 
fini_master_thread(master_thread_t *master) 
{
    int err;
    
    err = destroy_thread_pool(&master->child_threads);
    if (err != 0)
        return err;

    master->was_init = false;

    return 0;
}
