#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

void sigint_handler(int sig) {
    printf("Thread 2 [%d]: Caught SIGINT\n", gettid());
}

void *mythread(void *arg) {
    int thread_num = *(int*)arg;
	free(arg);

    switch (thread_num)
    {
    case 0:
        sigset_t all_sigs;
        sigfillset(&all_sigs);
        pthread_sigmask(SIG_BLOCK, &all_sigs, NULL);
        
        while (1) {
            sleep(2);
            printf("Thread 1 [%d]: Blocking signals...\n", gettid());
        }
        break;
    case 1:
        printf("Thread 2 [%d]: Waiting for SIGINT...\n", gettid());
        signal(SIGINT, sigint_handler);
        sigset_t unblock;
        sigemptyset(&unblock);
        sigaddset(&unblock, SIGINT);
        pthread_sigmask(SIG_UNBLOCK, &unblock, NULL);
        
        while (1) sleep(1);
        break;
    case 2:
        printf("Thread 3 [%d]: Waiting for SIGQUIT (sigwait)\n", gettid());
            
        sigset_t wait_set;
        sigemptyset(&wait_set);
        sigaddset(&wait_set, SIGQUIT);
        
        int sig;
        while (1) {
            if (sigwait(&wait_set, &sig) == 0 && sig == SIGQUIT) {
                printf("Thread 3 [%d]: Got SIGQUIT via sigwait\n", gettid());
                break;
            }
        }
        break;
    default:
        break;
    }

	return NULL;
}

int main() {
	pthread_t tid[3];
	int err;

	printf("main [%d %d %d]: Hello from main!\n", getpid(), getppid(), gettid());

	for (int i = 0; i < 3; i++) {
		int* thread_num = malloc(sizeof(int));
		*thread_num = i;
		err = pthread_create(&tid[i], NULL, mythread, thread_num);
		if (err) {
		    printf("main: pthread_create() failed: %s\n", strerror(err));
			return -1;
		}
	}

	for (int i = 0; i < 3; i++) {
		pthread_join(tid[i], NULL);
	}
	
	return 0;
}

