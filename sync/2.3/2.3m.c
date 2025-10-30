#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>

#define MAX_STRING_LENGTH 100

typedef struct _Node {
    char value[MAX_STRING_LENGTH];
    struct _Node* next;
    pthread_mutex_t sync;
    pid_t tid;
} Node;

typedef struct _Storage {
    Node *first;
} Storage;

typedef struct _Args {
    Storage *storage;
    int id;
} Args;

_Atomic unsigned long long count_asc = 0;
_Atomic unsigned long long count_desc = 0;
_Atomic unsigned long long count_eq = 0;
_Atomic unsigned long long swap_counters[3] = {0, 0, 0};

_Atomic int stop = 0;

Node* create_node(const char* value) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    strncpy(new_node->value, value, MAX_STRING_LENGTH - 1);
    new_node->value[MAX_STRING_LENGTH - 1] = '\0';
    new_node->next = NULL;
    pthread_mutex_init(&new_node->sync, NULL);
    return new_node;
}

void add_node(Storage* storage, const char* value) {
    Node* new_node = create_node(value);
    if (storage->first == NULL) {
        storage->first = new_node;
        return;
    }
    
    Node* current = storage->first;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = new_node;
}

void free_list(Storage* storage) {
    Node* current = storage->first;
    while (current != NULL) {
        Node* next = current->next;
        pthread_mutex_destroy(&current->sync);
        free(current);
        current = next;
    }
    storage->first = NULL;
}

void* count_ascending(void* arg) {
    Storage* storage = (Storage*)arg;
    while (!stop) {
        Node* current = storage->first;
        unsigned long long local_count = 0;
        bool is_locked = false;
        
        while (current != NULL && current->next != NULL) {
            printf("00\n");
            fflush(stdout);

            pthread_mutex_lock(&current->sync);
            printf("01\n");
            fflush(stdout);

            pthread_mutex_lock(&current->next->sync);
            printf("02\n");
            fflush(stdout);

            size_t len1 = strlen(current->value);
            size_t len2 = strlen(current->next->value);
            
            if (len1 < len2) {
                local_count++;
            }
            
            Node* next = current->next;
            pthread_mutex_unlock(&current->sync);
            current = next; 
        }
        
        if (current != NULL) {
            pthread_mutex_unlock(&current->sync);
        }
        
        count_asc++;
    }
    return NULL;
}

void* count_descending(void* arg) {
    Storage* storage = (Storage*)arg;
    while (!stop) {
        Node* current = storage->first;
        unsigned long long local_count = 0;
        
        while (current != NULL && current->next != NULL) {
           printf("10\n");
            fflush(stdout);
            pthread_mutex_lock(&current->sync);
            printf("11\n");
            fflush(stdout);
            pthread_mutex_lock(&current->next->sync);
            printf("12\n");
            fflush(stdout);
            size_t len1 = strlen(current->value);
            size_t len2 = strlen(current->next->value);
            
            if (len1 > len2) {
                local_count++;
            }
            
            Node* next = current->next;
            pthread_mutex_unlock(&current->sync);
            current = next;
        }
        
        if (current != NULL) {
            pthread_mutex_unlock(&current->sync);
        }
        
        count_desc++;
    }
    return NULL;
}

void* count_equal(void* arg) {
    Storage* storage = (Storage*)arg;
    while (!stop) {
        Node* current = storage->first;
        unsigned long long local_count = 0;
        
        while (current != NULL && current->next != NULL) {
            printf("20\n");
            fflush(stdout);
            pthread_mutex_lock(&current->sync);
            printf("21\n");
            fflush(stdout);
            pthread_mutex_lock(&current->next->sync);
            printf("22\n");
            fflush(stdout);
            size_t len1 = strlen(current->value);
            size_t len2 = strlen(current->next->value);
            
            if (len1 == len2) {
                local_count++;
            }
            
            Node* next = current->next;
            pthread_mutex_unlock(&current->sync);
            current = next;
        }
        
        if (current != NULL) {
            pthread_mutex_unlock(&current->sync);
        }
        
        count_eq++;
    }
    return NULL;
}

int swap_nodes(Storage* storage, Node* prev, Node* curr, Node* next, int thread_id) {
    if (prev == NULL) {
        printf("%d0\n", thread_id + 3);
        fflush(stdout);
        pthread_mutex_lock(&curr->sync);
        printf("%d1\n", thread_id + 3);
        fflush(stdout);
        pthread_mutex_lock(&next->sync);
        printf("%d2\n", thread_id + 3);
        fflush(stdout);
        curr->next = next->next;
        next->next = curr;
        storage->first = next;
        
        pthread_mutex_unlock(&curr->sync);
        pthread_mutex_unlock(&next->sync);
    } else {
        printf("%d3\n", thread_id + 3);
        fflush(stdout);
        pthread_mutex_lock(&prev->sync);
        printf("%d4\n", thread_id + 3);
        fflush(stdout);
        pthread_mutex_lock(&curr->sync);
        printf("%d5\n", thread_id + 3);
        fflush(stdout);
        pthread_mutex_lock(&next->sync);
        printf("%d6\n", thread_id + 3);
        fflush(stdout);

        prev->next = next;
        curr->next = next->next;
        next->next = curr;
        
        pthread_mutex_unlock(&prev->sync);
        pthread_mutex_unlock(&curr->sync);
        pthread_mutex_unlock(&next->sync);
    }
    
    swap_counters[thread_id]++;
    return 1;
}

void* swap_thread(void* arg) {
    Args *args = (Args*)arg;
    Storage *storage = args->storage;
    int thread_id = args->id;
    
    while (!stop) {
        Node* prev = NULL;
        Node* curr = storage->first;
        
        while (curr != NULL && curr->next != NULL) {
            if (rand() % 2 == 0) {
                if (swap_nodes(storage, prev, curr, curr->next, thread_id)) {
                    break;
                }
            }
            
            prev = curr;
            curr = curr->next;
        }
        
        usleep(100);
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
        printf("Usage: %s <list_size>\n", argv[0]);
        return 1;
    }
    
    int list_size = atoi(argv[1]);
    if (list_size <= 0) {
        printf("Invalid list size\n");
        return 1;
    }
    
    srand(time(NULL));
    
    Storage storage = {NULL};
    printf("Creating list with %d elements...\n", list_size);
    
    for (int i = 0; i < list_size; i++) {
        char value[MAX_STRING_LENGTH];
        int length = rand() % (MAX_STRING_LENGTH - 1) + 1;
        for (int j = 0; j < length; j++) {
            value[j] = 'a' + rand() % 26;
        }
        value[length] = '\0';
        add_node(&storage, value);
    }
    
    pthread_t threads[6];
    int thread_ids[3] = {0, 1, 2};
    Args swap_args[3];
    swap_args[0].id = thread_ids[0];
    swap_args[0].storage = &storage;

    swap_args[1].id = thread_ids[1];
    swap_args[1].storage = &storage;

    swap_args[2].id = thread_ids[2];
    swap_args[2].storage = &storage;
    
    pthread_create(&threads[0], NULL, count_ascending, &storage);
    pthread_create(&threads[1], NULL, count_descending, &storage);
    pthread_create(&threads[2], NULL, count_equal, &storage);
    pthread_create(&threads[3], NULL, swap_thread, &swap_args[0]);
    pthread_create(&threads[4], NULL, swap_thread, &swap_args[1]);
    pthread_create(&threads[5], NULL, swap_thread, &swap_args[2]);
    
    for (int i = 1; i <= 10; i++) {
        sleep(1);
        print_stats(i);
    }
    
    stop = 1;
    
    for (int i = 0; i < 6; i++) {
        pthread_join(threads[i], NULL);
    }
    
    free_list(&storage);
    
    return 0;
}
