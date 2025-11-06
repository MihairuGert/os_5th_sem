#include "include/master.h"

int main() 
{
    int             err;
    master_thread_t master;

    err = init_master_thread(&master);
    if (err != 0)
        goto err_handle;

    err = start_master_thread(&master);
    if (err != 0)
        goto err_handle;

    err = fini_master_thread(&master);
    if (err != 0)
        goto err_handle;
        
    return 0;



    err_handle:
    fprintf(stderr, "%s. Terminate.\n", what(err));
    return err;
}