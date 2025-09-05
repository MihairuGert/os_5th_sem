#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct data {
    char* char_data;
    int int_data;
} data;

void *mythread(void *arg) {
    data* d = (data*)arg;
    printf("mythread [%d] %d %s\n", gettid(), d->int_data, d->char_data);
    free(d);
    //pthread_detach(pthread_self());
	return NULL;
}

int main() {
	pthread_t tid;
    pthread_attr_t attr;
    int err;
    int s;
    
    s = pthread_attr_init(&attr);
    s = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	printf("main [%d %d %d]: Hello from main!\n", getpid(), getppid(), gettid());

	for (;;) {
        
        data* d = malloc(sizeof(data));
        d->int_data = 10;
        d->char_data = malloc(sizeof(10));
        snprintf(d->char_data, 10, "hell yeah");
		
        err = pthread_create(&tid, &attr, mythread, d);
		if (err) {
		    printf("main: pthread_create() failed: %s\n", strerror(err));
			return -1;
		}
		//sleep(1);
	}
	
	return 0;
}

