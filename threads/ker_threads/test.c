#include "mythread.h"
#include "unistd.h"

void *hello(void *arg) 
{
    printf("hello from mythread\n");
    return NULL;
}

void *sleepTh(void *arg) 
{
    sleep(3);
    return NULL;
}

int main() 
{
    mythread_t  thread;
    int         res;
    void        **retval;

    res = mythread_create(thread, hello, NULL);
    if (res != 0) 
    {
        fprintf(stderr, "Failed to create thread\n");
        return 1;
    }

    res = mythread_join(thread, retval);
    if (res != 0) 
    {
        fprintf(stderr, "Failed to join thread\n");
        return 1;
    }

    res = mythread_create(thread, sleepTh, NULL);
    if (res != 0) 
    {
        fprintf(stderr, "Failed to create thread\n");
        return 1;
    }

    res = mythread_join(thread, retval);
    if (res != 0) 
    {
        fprintf(stderr, "Failed to join thread\n");
        return 1;
    }

    res = mythread_join(thread, retval);
    if (res != 0) 
    {
        fprintf(stderr, "Failed to join thread\n");
    }

    return 0;
}