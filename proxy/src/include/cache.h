#ifndef CACHE_H
#define CACHE_H

#define _GNU_SOURCE

#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../../lib/libuv/include/uv.h"

typedef enum {
    CACHE_FOUND = 0,
    CACHE_NEW_LOADED,
    CACHE_NEW_NOT_LOADED,
    CACHE_NOT_FOUND,
    CACHE_DONE,
    CACHE_ERROR
} cache_status_t;

typedef struct cache_waiter {
    uv_async_t* async_handle;
    struct cache_waiter* next;
} cache_waiter_t;

typedef struct cache_waiter_table_entry {
    cache_waiter_t* waiter;
    char* key;
    struct cache_waiter_table_entry* next;
} cache_waiter_table_entry_t;

typedef struct {
    cache_waiter_table_entry_t* head;
} cache_waiter_table_t;

typedef struct cache_entry {
    char* key;
    atomic_bool is_loading;
    char* data;
    atomic_size_t data_size;
    size_t allocated_size;
    pthread_rwlock_t data_lock;

    time_t last_accessed;
    time_t created_at;
    time_t expiry_time;

    struct cache_entry* next;

    cache_waiter_t* waiting_clients;
    pthread_mutex_t waiters_mutex;
} cache_entry_t;

typedef struct {
    cache_entry_t* head;
    pthread_mutex_t mutex;

    cache_waiter_table_t pending_waiters;
} cache_bucket_t;

typedef struct {
    cache_bucket_t* buckets;
    size_t num_buckets;

    cache_entry_t* lru_head;
    cache_entry_t* lru_tail;
    pthread_mutex_t lru_mutex;

    size_t max_size;
    size_t current_size;
    size_t max_entries;
    size_t current_entries;

    size_t hits;
    size_t misses;
    time_t default_ttl;
} cache_t;

int cache_init(cache_t* cache,
    size_t max_size,
    size_t max_entries,
    size_t num_buckets,
    time_t default_ttl);
void cache_cleanup(cache_t* cache);

cache_status_t cache_lookup(cache_t* cache,
    const char* key,
    char** data,
    size_t* data_size);

cache_status_t cache_lookup_partial(cache_t* cache,
    const char* key,
    char** data,
    size_t* data_size,
    size_t offset,
    size_t max_size,
    bool* is_loading);

bool cache_check_progress(cache_entry_t* entry,
    size_t current_offset,
    size_t* new_data_size);

int cache_store(cache_t* cache,
    const char* key,
    const char* data,
    size_t data_size);
int cache_store_ttl(cache_t* cache,
    const char* key,
    const char* data,
    size_t data_size,
    time_t ttl);

cache_entry_t* cache_start_loading(cache_t* cache, const char* key, time_t ttl);
int cache_append_data(cache_entry_t* entry,
    const char* chunk,
    size_t chunk_size);
int cache_finish_loading(cache_entry_t* entry);
int cache_cancel_loading(cache_t* cache, const char* key);

int cache_remove(cache_t* cache, const char* key);
void cache_clean_expired(cache_t* cache);

void cache_get_stats(cache_t* cache,
    size_t* hits,
    size_t* misses,
    size_t* current_size,
    size_t* current_entries);
void cache_print(cache_t* cache);

int cache_register_waiter(cache_t* cache,
    const char* key,
    uv_async_t* async_handle);
int cache_unregister_waiter(cache_t* cache,
    const char* key,
    uv_async_t* async_handle);

unsigned long cache_hash(const char* key, size_t num_buckets);

#endif
