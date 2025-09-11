#include "1.6.h"

typedef struct thread_struct {

} thread_struct;

typedef thread_struct* mythread_t;

#define GUARD_SIZE 1000

int mythread_create(mythread_t thread, void *(start_routine), void *arg) {
    void* stack = mmap(NULL, 8392704, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_STACK, -1, 0);
    int status = mprotect(stack + GUARD_SIZE, 8388608, PROT_READ|PROT_WRITE);
    int d = clone(start_routine, stack, CLONE_VM|CLONE_FS|CLONE_FILES|CLONE_SIGHAND|CLONE_THREAD|CLONE_SYSVSEM|CLONE_SETTLS|CLONE_PARENT_SETTID|CLONE_CHILD_CLEARTID, arg);
    
    return 0;
}


int main() {

    return 0;
}


