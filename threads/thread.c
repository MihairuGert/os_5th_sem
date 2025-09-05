#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

int d;

void *mythread(void *arg) {
	int a;
	static int b;
	const int c;
	printf("mythread [%d %d %d %ld]: Hello from mythread!\nlocal: %p static: %p const: %p global: %p\n",
		 getpid(), getppid(), gettid(), pthread_self(), &a, &b, &c, &d);
	sleep(3);
	return NULL;
}

int main() {
	pthread_t tid[5];
	int err;

	printf("main [%d %d %d]: Hello from main!\n", getpid(), getppid(), gettid());

	for (int i = 0; i < 5; i++) {
		err = pthread_create(&tid[i], NULL, mythread, NULL);
		printf("main [%ld]\n", tid[i]);
		if (err) {
	    printf("main: pthread_create() failed: %s\n", strerror(err));
		return -1;}
	}

	for (int i = 0; i < 5; i++) {
		pthread_join(tid[i], NULL);
	}
	
	return 0;
}

