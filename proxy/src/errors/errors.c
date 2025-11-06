#include "../include/errors.h"

char* 
what(int error_num)
{
    switch (error_num)
    {
    case ERR_COULD_NOT_CREATE_THREAD_POOL:
        return "Couldn't create a thread pool.";
    case ERR_COULD_NOT_DESTROY_THREAD_POOL:
        return "Couldn't destroy a thread pool.";
    case ERR_CANNOT_START_NOT_INIT:
        return "Couldn't start what wasn't initialized.";
    default:
        return "Unknown error.";
    }
}