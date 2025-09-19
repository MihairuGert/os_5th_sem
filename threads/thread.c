#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

int d = 4;

void *mythread(void *arg) {
	int thread_num = *(int*)arg;
	free(arg);
	int a = 1;
	static int b = 2;
	const int c = 3;
	printf("mythread #%d [%d %d %d %ld]: Hello from mythread!\n\
		address| local: %p static: %p const: %p global: %p\n\
		value  | local: %d static: %d const: %d global: %d\n",
		 thread_num,
		 getpid(), getppid(), gettid(), pthread_self(), 
		 &a, &b, &c, &d,
		 a, b, c, d);
	if (thread_num == 2) {
		a = 10;
		b = 20;
		d = 40;
		printf("\achanged variables from %d thread\n", thread_num);
	}
	sleep(1000);
	return NULL;
}

int main() {
	pthread_t tid[5];
	int err;

	printf("main [%d %d %d]: Hello from main!\n", getpid(), getppid(), gettid());

	for (int i = 0; i < 5; i++) {
		int* thread_num = malloc(sizeof(int));
		*thread_num = i;
		err = pthread_create(&tid[i], NULL, mythread, thread_num);
		printf("main [%ld]\n", tid[i]);
		if (err) {
		    printf("main: pthread_create() failed: %s\n", strerror(err));
			for (int j = 0; j < i; j++) {
				pthread_join(tid[j], NULL);
			}
			return -1;
		}
		sleep(1);
	}

	for (int i = 0; i < 5; i++) {
		pthread_join(tid[i], NULL);
	}
	
	return 0;
}

