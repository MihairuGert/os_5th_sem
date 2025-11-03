#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <errno.h>
#include <limits.h>

#define MAX_STRING_LENGTH 100

typedef struct _Node {
    char value[MAX_STRING_LENGTH];
    struct _Node* next;
    pthread_rwlock_t sync;
} Node;

typedef struct _Storage {
    Node *first;
    long size;
} Storage;

typedef struct _Args {
    Storage *storage;
    int id;
} Args;

unsigned long long count_asc = 0;
unsigned long long count_desc = 0;
unsigned long long count_eq = 0; 

_Atomic unsigned long long swap_counters[3] = {0, 0, 0};

_Atomic int stop = 0;

Node* create_node(const char* value) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    if (new_node == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for node\n");
        return NULL;
    }
    
    if (strncpy(new_node->value, value, MAX_STRING_LENGTH - 1) == NULL) {
        fprintf(stderr, "Error: Failed to copy string to node\n");
        free(new_node);
        return NULL;
    }
    new_node->value[MAX_STRING_LENGTH - 1] = '\0';
    new_node->next = NULL;
    
    int init_result = pthread_rwlock_init(&new_node->sync, NULL);
    if (init_result != 0) {
        fprintf(stderr, "Error: Failed to initialize node rwlock: %s\n", strerror(init_result));
        free(new_node);
        return NULL;
    }
    
    return new_node;
}

int add_node(Storage* storage, const char* value) {
    if (storage == NULL || value == NULL) {
        fprintf(stderr, "Error: Invalid arguments to add_node\n");
        return -1;
    }
    
    Node* new_node = create_node(value);
    if (new_node == NULL) {
        return -1;
    }
    
    if (storage->first == NULL) {
        storage->first = new_node;
        return 0;
    }
    
    Node* current = storage->first;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = new_node;
    
    return 0;
}

void free_list(Storage* storage) {
    if (storage == NULL) {
        return;
    }

    Node* current = storage->first;
    while (current != NULL) {
        Node* next = current->next;
        int destroy_result = pthread_rwlock_destroy(&current->sync);
        if (destroy_result != 0) {
            fprintf(stderr, "Warning: Failed to destroy node rwlock: %s\n", strerror(destroy_result));
        }
        free(current);
        current = next;
    }
    storage->first = NULL;
}

void* count_ascending(void* arg) {
    if (arg == NULL) {
        fprintf(stderr, "Error: NULL argument passed to count_ascending\n");
        return NULL;
    }
    
    Storage* storage = (Storage*)arg;
    
    while (!stop) {
        Node* current = NULL;
        unsigned long long local_count = 0;
        current = storage->first;
        while (current != NULL && current->next != NULL && !stop) {
            int lock1_result = pthread_rwlock_rdlock(&current->sync);
            if (lock1_result != 0) {
                fprintf(stderr, "Error: Failed to acquire first node lock in count_ascending: %s\n", strerror(lock1_result));
                break;
            }
            
            int lock2_result = pthread_rwlock_rdlock(&current->next->sync);
            if (lock2_result != 0) {
                fprintf(stderr, "Error: Failed to acquire second node lock in count_ascending: %s\n", strerror(lock2_result));
                int unlock_result1 = pthread_rwlock_unlock(&current->sync);
                if (unlock_result1 != 0) {
                    fprintf(stderr, "Error: Failed to unlock first node in count_ascending: %s\n", strerror(unlock_result1));
                }
                break;
            }
            
            size_t len1 = strlen(current->value);
            size_t len2 = strlen(current->next->value);
            
            if (len1 < len2) {
                local_count++;
            }
            
            Node* next = current->next;
            int unlock_result1 = pthread_rwlock_unlock(&current->sync);
            if (unlock_result1 != 0) {
                fprintf(stderr, "Error: Failed to unlock first node in count_ascending: %s\n", strerror(unlock_result1));
            }
            
            int unlock_result2 = pthread_rwlock_unlock(&next->sync);
            if (unlock_result2 != 0) {
                fprintf(stderr, "Error: Failed to unlock second node in count_ascending: %s\n", strerror(unlock_result2));
            }
            
            current = next;
        }
        
        count_asc++;
        
        int sleep_result = usleep(1000);
        if (sleep_result != 0) {
            fprintf(stderr, "Warning: usleep failed in count_ascending\n");
        }
    }
    return NULL;
}

void* count_descending(void* arg) {
    if (arg == NULL) {
        fprintf(stderr, "Error: NULL argument passed to count_descending\n");
        return NULL;
    }
    
    Storage* storage = (Storage*)arg;
    while (!stop) {
        Node* current = NULL;
        unsigned long long local_count = 0;
        current = storage->first;
        while (current != NULL && current->next != NULL && !stop) {
            int lock1_result = pthread_rwlock_rdlock(&current->sync);
            if (lock1_result != 0) {
                fprintf(stderr, "Error: Failed to acquire first node lock in count_descending: %s\n", strerror(lock1_result));
                break;
            }
            
            int lock2_result = pthread_rwlock_rdlock(&current->next->sync);
            if (lock2_result != 0) {
                fprintf(stderr, "Error: Failed to acquire second node lock in count_descending: %s\n", strerror(lock2_result));
                int unlock_result1 = pthread_rwlock_unlock(&current->sync);
                if (unlock_result1 != 0) {
                    fprintf(stderr, "Error: Failed to unlock first node in count_descending: %s\n", strerror(unlock_result1));
                }
                break;
            }
            
            size_t len1 = strlen(current->value);
            size_t len2 = strlen(current->next->value);
            
            if (len1 > len2) {
                local_count++;
            }
            
            Node* next = current->next;
            int unlock_result1 = pthread_rwlock_unlock(&current->sync);
            if (unlock_result1 != 0) {
                fprintf(stderr, "Error: Failed to unlock first node in count_descending: %s\n", strerror(unlock_result1));
            }
            
            int unlock_result2 = pthread_rwlock_unlock(&next->sync);
            if (unlock_result2 != 0) {
                fprintf(stderr, "Error: Failed to unlock second node in count_descending: %s\n", strerror(unlock_result2));
            }
            
            current = next;
        }
        
        count_desc++;
        
        int sleep_result = usleep(1000);
        if (sleep_result != 0) {
            fprintf(stderr, "Warning: usleep failed in count_descending\n");
        }
    }
    return NULL;
}

void* count_equal(void* arg) {
    if (arg == NULL) {
        fprintf(stderr, "Error: NULL argument passed to count_equal\n");
        return NULL;
    }
    
    Storage* storage = (Storage*)arg;
    while (!stop) {
        Node* current = NULL;
        unsigned long long local_count = 0;
        current = storage->first;
        while (current != NULL && current->next != NULL && !stop) {
            int lock1_result = pthread_rwlock_rdlock(&current->sync);
            if (lock1_result != 0) {
                fprintf(stderr, "Error: Failed to acquire first node lock in count_equal: %s\n", strerror(lock1_result));
                break;
            }
            
            int lock2_result = pthread_rwlock_rdlock(&current->next->sync);
            if (lock2_result != 0) {
                fprintf(stderr, "Error: Failed to acquire second node lock in count_equal: %s\n", strerror(lock2_result));
                int unlock_result1 = pthread_rwlock_unlock(&current->sync);
                if (unlock_result1 != 0) {
                    fprintf(stderr, "Error: Failed to unlock first node in count_equal: %s\n", strerror(unlock_result1));
                }
                break;
            }
            
            size_t len1 = strlen(current->value);
            size_t len2 = strlen(current->next->value);
            
            if (len1 == len2) {
                local_count++;
            }
            
            Node* next = current->next;
            int unlock_result1 = pthread_rwlock_unlock(&current->sync);
            if (unlock_result1 != 0) {
                fprintf(stderr, "Error: Failed to unlock first node in count_equal: %s\n", strerror(unlock_result1));
            }
            
            int unlock_result2 = pthread_rwlock_unlock(&next->sync);
            if (unlock_result2 != 0) {
                fprintf(stderr, "Error: Failed to unlock second node in count_equal: %s\n", strerror(unlock_result2));
            }
            
            current = next;
        }
        
        count_eq++;
        
        int sleep_result = usleep(1000);
        if (sleep_result != 0) {
            fprintf(stderr, "Warning: usleep failed in count_equal\n");
        }
    }
    return NULL;
}

int safe_swap_nodes(Storage* storage, int thread_id) {
    while (!stop) {
        int randElem = rand() % (storage->size - 3) + 1;
        pthread_rwlock_rdlock(&storage->first->sync);
        Node *current = storage->first;
        Node *prev = NULL;
        Node *prevprev;
        int ind = 0;
        while (ind < randElem) {
            prevprev = prev;
            prev = current;
            pthread_rwlock_rdlock(&current->next->sync);
            current = current->next;
            if (prevprev != NULL)
                pthread_rwlock_unlock(&prevprev->sync);
            ind++;
        }
        // todo add mutex
        pthread_rwlock_unlock(&prev->sync);
        pthread_rwlock_wrlock(&prev->sync);

        pthread_rwlock_unlock(&current->sync);
        pthread_rwlock_wrlock(&current->sync);

        pthread_rwlock_wrlock(&current->next->sync);     
        
        prev->next = current->next;
        current->next = current->next->next;
        current->next->next = current;

        pthread_rwlock_unlock(&prev->sync);
        pthread_rwlock_unlock(&prev->next->sync);
        pthread_rwlock_unlock(&prev->next->next->sync);
        
        usleep(100);
    }
    return 1;
}

void* swap_thread(void* arg) {
    Args *args = (Args*)arg;
    Storage *storage = args->storage;
    int thread_id = args->id;
    
    while (!stop) {
        if (safe_swap_nodes(storage, thread_id)) {
            int sleep_result = usleep(5000);
            if (sleep_result != 0) {
                fprintf(stderr, "Warning: usleep failed in swap_thread\n");
            }
        } else {
            int sleep_result = usleep(10000);
            if (sleep_result != 0) {
                fprintf(stderr, "Warning: usleep failed in swap_thread\n");
            }
        }
    }
    return NULL;
}

void print_stats(int duration) {
    printf("Statistics after %d seconds:\n", duration);
    printf("Ascending iterations: %llu\n", count_asc);
    printf("Descending iterations: %llu\n", count_desc);
    printf("Equal iterations: %llu\n", count_eq);
    printf("Swap counters: %llu, %llu, %llu\n\n", 
           swap_counters[0], swap_counters[1], swap_counters[2]);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <list_size>\n", argv[0]);
        return 1;
    }
    
    char* endptr;
    long list_size = strtol(argv[1], &endptr, 10);
    
    srand(time(NULL));
    
    Storage storage;
    storage.size = list_size;
    storage.first = NULL;
    
    printf("Creating list with %ld elements...\n", list_size);
    
    for (int i = 0; i < list_size; i++) {
        char value[MAX_STRING_LENGTH];
        int length = rand() % (MAX_STRING_LENGTH - 1) + 1;
        for (int j = 0; j < length; j++) {
            value[j] = 'a' + rand() % 26;
        }
        value[length] = '\0';
        
        int add_result = add_node(&storage, value);
        if (add_result != 0) {
            fprintf(stderr, "Error: Failed to add node %d\n", i);
            free_list(&storage);
            return 1;
        }
    }
    
    pthread_t threads[6];
    Args swap_args[3];
    
    for (int i = 0; i < 3; i++) {
        swap_args[i].id = i;
        swap_args[i].storage = &storage;
    }
    
    int create_result;
    const char* thread_names[] = {
        "ascending", "descending", "equal", "swap0", "swap1", "swap2"
    };
    
    for (int i = 0; i < 6; i++) {
        void* (*start_routine)(void*);
        
        if (i == 0) start_routine = count_ascending;
        else if (i == 1) start_routine = count_descending;
        else if (i == 2) start_routine = count_equal;
        else start_routine = swap_thread;
        
        void* arg;
        if (i < 3) arg = &storage;
        else arg = &swap_args[i - 3];
        
        create_result = pthread_create(&threads[i], NULL, start_routine, arg);
        if (create_result != 0) {
            fprintf(stderr, "Error: Failed to create %s thread: %s\n", 
                    thread_names[i], strerror(create_result));
            
            stop = 1;
            
            for (int j = 0; j < i; j++) {
                int join_result = pthread_join(threads[j], NULL);
                if (join_result != 0) {
                    fprintf(stderr, "Error: Failed to join thread %s: %s\n", 
                            thread_names[j], strerror(join_result));
                }
            }
            
            free_list(&storage);
            return 1;
        }
    }
    
    for (int i = 1; i <= 5; i++) {
        sleep(1);
        print_stats(i);
    }
    
    stop = 1;
    
    for (int i = 0; i < 6; i++) {
        int join_result = pthread_join(threads[i], NULL);
        if (join_result != 0) {
            fprintf(stderr, "Error: Failed to join thread %s: %s\n", 
                    thread_names[i], strerror(join_result));
        }
    }
    
    free_list(&storage);
    
    printf("Program finished successfully.\n");
    return 0;
}
