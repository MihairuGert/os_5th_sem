#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

unsigned int count = 0;

void clean_malloc(void* arg) {
    printf("cleaning...");
    free(arg);
}

void *mythread(void *arg) {
    char* string = malloc(sizeof(char) * 12);
    snprintf(string, 12, "hello world");

    pthread_cleanup_push(clean_malloc, string);

    while (1)
    {
        printf("%s\n", string);
    }

    pthread_cleanup_pop(0);

	return NULL;
}

int main() {
	pthread_t tid;
	int err;

	printf("main [%d %d %d]: Hello from main!\n", getpid(), getppid(), gettid());

	err = pthread_create(&tid, NULL, mythread, NULL);
	if (err) {
	    printf("main: pthread_create() failed: %s\n", strerror(err));
		return -1;
	}
    sleep(2);
    //printf("%d\n", count);
    pthread_cancel(tid);
	
	return 0;
}

