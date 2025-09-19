#include "mythread.h"

void *hello(void *arg) 
{
    printf("hello from mythread\n");
    return NULL;
}

int main() 
{
    mythread_t  thread;
    int         res;
    void        **retval;

    res = mythread_create(thread, hello, NULL);
    if (res == -1) 
    {
        fprintf(stderr, "Failed to create thread\n");
        return 1;
    }

    res = mythread_join(thread, retval);
    if (res == -1) 
    {
        fprintf(stderr, "Failed to join thread\n");
        return 1;
    }
    return 0;
}