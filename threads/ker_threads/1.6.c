#include "1.6.h"

typedef long mythread_t;

#define GUARD_SIZE 4096
#define STACK_SIZE 8388608

struct thread_args {
    void *(*start_routine)(void*);
    void *arg;
};

static int start_routine_wrapper(void *arg) {
    struct thread_args *args = (struct thread_args*)arg;
    args->start_routine(args->arg);
    free(args);
    return 0;
}

int mythread_create(mythread_t *thread, void *(start_routine), void *arg) {
    struct thread_args *args = malloc(sizeof(struct thread_args));
    if (!args) {
        fprintf(stderr, "mythread_create: malloc failed: %s\n", strerror(errno));
        return -1;
    }
    args->start_routine = start_routine;
    args->arg = arg;

    void* stack = mmap(NULL, STACK_SIZE + GUARD_SIZE, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_STACK, -1, 0);
    if (stack == MAP_FAILED) {
        fprintf(stderr, "mythread_create: mmap() failed: %s\n", strerror(errno));
        free(args);
        return -1;
    }

    int err = mprotect(stack + GUARD_SIZE, STACK_SIZE, PROT_READ|PROT_WRITE);
    if (err) {
        fprintf(stderr, "mythread_create: mprotect() failed: %s\n", strerror(errno));
        munmap(stack, STACK_SIZE + GUARD_SIZE);
        free(args);
        return -1;
    }

    void *stack_top = stack + GUARD_SIZE + STACK_SIZE;
    int flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD | CLONE_SYSVSEM | CLONE_PARENT_SETTID | CLONE_CHILD_CLEARTID;
    pid_t tid = clone(start_routine_wrapper, stack_top, flags, args, thread, NULL, thread);

    if (tid == -1) {
        fprintf(stderr, "mythread_create: clone() failed: %s\n", strerror(errno));
        munmap(stack, STACK_SIZE + GUARD_SIZE);
        free(args);
        return -1;
    }

    return 0;
}

void *hello(void *arg) {
    printf("hello from mythread\n");
    return NULL;
}

int main() {
    mythread_t thread;
    int res = mythread_create(&thread, hello, NULL);
    if (res == -1) {
        fprintf(stderr, "Failed to create thread\n");
        return 1;
    }
    sleep(1);
    return 0;
}