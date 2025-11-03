#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "spinlock/spinlock.h" 

#define NUM_THREADS 5
#define ITERATIONS 100000

#define COLOR_BOLD_RED      "\033[1;31m"
#define COLOR_BOLD_GREEN    "\033[32m"
#define COLOR_RESET         "\033[0m"

int counter = 0;
spinlock_t lock;

void* increment_counter(void* arg) {
    int thread_id = *(int*)arg;
    
    for (int i = 0; i < ITERATIONS; i++) {
        spinlock_lock(&lock);
        
        counter++;
        
        spinlock_unlock(&lock);
    }
    
    printf("Thread %d completed\n", thread_id);
    return NULL;
}

void test_basic_functionality() {
    printf("=== Testing Basic Functionality ===\n");
    
    spinlock_init(&lock);
    
    printf("Initial counter: %d\n", counter);
    
    spinlock_lock(&lock);
    counter++;
    spinlock_unlock(&lock);
    
    printf("After single thread increment: %d\n", counter);
    
    printf("Test %s\n", (counter == 1) ? COLOR_BOLD_GREEN"PASSED"COLOR_RESET : COLOR_BOLD_RED"FAILED"COLOR_RESET);
    counter = 0;
    printf("Reset counter to: %d\n", counter);
}

void test_multithreaded() {
    printf("\n=== Testing Multi-threaded Scenario ===\n");
    
    spinlock_init(&lock);
    counter = 0;
    
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, increment_counter, &thread_ids[i]) != 0) {
            perror("Failed to create thread");
            return;
        }
    }
    
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    int expected = NUM_THREADS * ITERATIONS;
    printf("Final counter: %d\n", counter);
    printf("Expected: %d\n", expected);
    printf("Test %s\n", (counter == expected) ? COLOR_BOLD_GREEN"PASSED"COLOR_RESET : COLOR_BOLD_RED"FAILED"COLOR_RESET);
}

void test_trylock() {
    int is_failed = 0;

    printf("\n=== Testing TryLock ===\n");
    
    spinlock_init(&lock);
    
    spinlock_lock(&lock);
    printf("Lock acquired in main thread\n");
    
    if (spinlock_trylock(&lock)) {
        printf("TryAcquire succeeded (recursive lock?)\n");
        spinlock_unlock(&lock);
        is_failed = 1;
    } else {
        printf("TryAcquire failed as expected\n");
    }
    
    spinlock_unlock(&lock);
    printf("Lock released\n");
    
    if (spinlock_trylock(&lock)) {
        printf("TryAcquire succeeded after release\n");
        spinlock_unlock(&lock);
    } else {
        printf("TryAcquire failed unexpectedly\n");
        is_failed = 1;
    }
    printf("Test %s\n", !is_failed ? COLOR_BOLD_GREEN"PASSED"COLOR_RESET : COLOR_BOLD_RED"FAILED"COLOR_RESET);
}

int main() {
    printf("SpinLock Library Test\n");
    printf("=====================\n");
    
    test_basic_functionality();
    
    test_multithreaded();

    test_trylock();
    
    printf("\nAll tests completed!\n");
    return 0;
}
