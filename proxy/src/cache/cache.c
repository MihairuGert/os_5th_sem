#include "../include/cache.h"

static void free_waiters_list(cache_waiter_t* waiters);
static void notify_waiting_clients(cache_entry_t* entry);

static cache_entry_t* create_entry(const char* key,
    size_t initial_size,
    time_t ttl)
{
    cache_entry_t* entry = malloc(sizeof(cache_entry_t));

    if (!entry)
        return NULL;

    entry->key = strdup(key);
    if (!entry->key) {
        free(entry);
        return NULL;
    }

    if (pthread_rwlock_init(&entry->data_lock, NULL) != 0) {
        free(entry->key);
        free(entry);
        return NULL;
    }

    atomic_store(&entry->is_loading, true);
    atomic_store(&entry->data_size, 0);

    entry->allocated_size = (initial_size > 0) ? initial_size : 4096;
    entry->data = malloc(entry->allocated_size);
    if (!entry->data) {
        pthread_rwlock_destroy(&entry->data_lock);
        free(entry->key);
        free(entry);
        return NULL;
    }

    time_t now = time(NULL);

    entry->last_accessed = now;
    entry->created_at = now;
    entry->expiry_time = now + ttl;

    entry->next = NULL;

    entry->waiting_clients = NULL;
    if (pthread_mutex_init(&entry->waiters_mutex, NULL) != 0) {
        pthread_rwlock_destroy(&entry->data_lock);
        free(entry->key);
        free(entry->data);
        free(entry);
        return NULL;
    }

    return entry;
}

static void free_entry(cache_entry_t* entry)
{
    if (entry) {
        free(entry->key);
        free(entry->data);
        pthread_rwlock_destroy(&entry->data_lock);

        pthread_mutex_lock(&entry->waiters_mutex);
        free_waiters_list(entry->waiting_clients);
        entry->waiting_clients = NULL;
        pthread_mutex_unlock(&entry->waiters_mutex);
        pthread_mutex_destroy(&entry->waiters_mutex);

        free(entry);
    }
}

int cache_init(cache_t* cache,
    size_t max_size,
    size_t max_entries,
    size_t num_buckets,
    time_t default_ttl)
{
    if (num_buckets == 0) {
        num_buckets = 1024;
    }

    cache->buckets = calloc(num_buckets, sizeof(cache_bucket_t));
    if (!cache->buckets)
        return -1;

    cache->num_buckets = num_buckets;
    cache->lru_head = NULL;
    cache->lru_tail = NULL;
    cache->max_size = max_size;
    cache->current_size = 0;
    cache->max_entries = max_entries;
    cache->current_entries = 0;
    cache->hits = 0;
    cache->misses = 0;
    cache->default_ttl = default_ttl;

    for (size_t i = 0; i < num_buckets; i++) {
        if (pthread_mutex_init(&cache->buckets[i].mutex, NULL) != 0) {
            for (size_t j = 0; j < i; j++) {
                pthread_mutex_destroy(&cache->buckets[j].mutex);
            }
            free(cache->buckets);
            return -1;
        }
        cache->buckets[i].head = NULL;
    }

    if (pthread_mutex_init(&cache->lru_mutex, NULL) != 0) {
        for (size_t i = 0; i < num_buckets; i++) {
            pthread_mutex_destroy(&cache->buckets[i].mutex);
        }
        free(cache->buckets);
        return -1;
    }

    return 0;
}

void cache_cleanup(cache_t* cache)
{
    for (size_t i = 0; i < cache->num_buckets; i++) {
        pthread_mutex_destroy(&cache->buckets[i].mutex);
    }
    free(cache->buckets);
    pthread_mutex_destroy(&cache->lru_mutex);

    cache->buckets = NULL;
    cache->num_buckets = 0;
    cache->lru_head = NULL;
    cache->lru_tail = NULL;
    cache->current_size = 0;
    cache->current_entries = 0;
}

cache_status_t cache_lookup(cache_t* cache,
    const char* key,
    char** data,
    size_t* data_size)
{
    bool is_loading;

    return cache_lookup_partial(cache, key, data, data_size, 0, 0, &is_loading);
}

/*
 * offset is the number of bytes already been read,
 * max_size is the maximum we are ready to recieve.
 */
cache_status_t cache_lookup_partial(cache_t* cache,
    const char* key,
    char** data,
    size_t* data_size,
    size_t offset,
    size_t max_size,
    bool* is_loading)
{
    unsigned long bucket_idx = cache_hash(key, cache->num_buckets);
    cache_bucket_t* bucket = &cache->buckets[bucket_idx];

    pthread_mutex_lock(&bucket->mutex);

    cache_entry_t* entry = bucket->head;

    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            bool loading = atomic_load(&entry->is_loading);
            size_t current_size = atomic_load(&entry->data_size);

            *is_loading = loading;

            /*
             * This is just an agreement for max_size = 0 if we want to check
             * for cache entry existance.
             */

            if (!loading && max_size == 0) {
                *data_size = current_size;
                *data = malloc(current_size);
                memcpy(*data, entry->data, current_size);
                pthread_mutex_unlock(&bucket->mutex);
                return CACHE_FOUND;
            }

            if (loading && offset >= current_size) {
                pthread_mutex_unlock(&bucket->mutex);
                return CACHE_NEW_NOT_LOADED;
            }

            if (offset > current_size) {
                pthread_mutex_unlock(&bucket->mutex);
                return CACHE_ERROR;
            }

            pthread_rwlock_rdlock(&entry->data_lock);
            size_t copy_size = current_size - offset;

            if (max_size > 0 && copy_size > max_size) {
                copy_size = max_size;
            }

            if (copy_size > 0) {
                *data = malloc(copy_size);
                memcpy(*data, entry->data + offset, copy_size);
                *data_size = copy_size;
            } else {
                *data = NULL;
                *data_size = 0;
                pthread_rwlock_unlock(&entry->data_lock);
                pthread_mutex_unlock(&bucket->mutex);
                return CACHE_DONE;
            }

            pthread_rwlock_unlock(&entry->data_lock);

            entry->last_accessed = time(NULL);

            cache->hits++;
            pthread_mutex_unlock(&bucket->mutex);
            return CACHE_NEW_LOADED;
        }
        entry = entry->next;
    }

    cache->misses++;
    pthread_mutex_unlock(&bucket->mutex);
    return CACHE_NOT_FOUND;
}

cache_entry_t* cache_start_loading(cache_t* cache,
    const char* key,
    time_t ttl)
{
    unsigned long bucket_idx = cache_hash(key, cache->num_buckets);
    cache_bucket_t* bucket = &cache->buckets[bucket_idx];

    pthread_mutex_lock(&bucket->mutex);

    cache_entry_t* entry = bucket->head;

    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            pthread_mutex_unlock(&bucket->mutex);
            return entry;
        }
        entry = entry->next;
    }

    entry = create_entry(key, 4096, ttl);
    if (!entry) {
        pthread_mutex_unlock(&bucket->mutex);
        return NULL;
    }

    entry->next = bucket->head;
    bucket->head = entry;

    cache->current_entries++;

    cache_waiter_table_entry_t* current = bucket->pending_waiters.head;
    cache_waiter_table_entry_t* prev_waiter = NULL;

    while (current) {
        if (strcmp(current->key, key) == 0) {
            entry->waiting_clients = current->waiter;
            if (prev_waiter)
                prev_waiter->next = current->next;
            else
                bucket->pending_waiters.head = NULL;
            break;
        }
        prev_waiter = current;
        current = current->next;
    }

    pthread_mutex_unlock(&bucket->mutex);
    return entry;
}

int cache_append_data(cache_entry_t* entry,
    const char* chunk,
    size_t chunk_size)
{
    if (!entry || !atomic_load(&entry->is_loading)) {
        return -1;
    }

    pthread_rwlock_wrlock(&entry->data_lock);

    size_t current_size = atomic_load(&entry->data_size);

    if (current_size + chunk_size > entry->allocated_size) {
        size_t new_size = entry->allocated_size;

        while (new_size < current_size + chunk_size) {
            new_size *= 2;
        }

        char* new_data = realloc(entry->data, new_size);

        if (!new_data) {
            pthread_rwlock_unlock(&entry->data_lock);
            return -1;
        }

        entry->data = new_data;
        entry->allocated_size = new_size;
    }

    memcpy(entry->data + current_size, chunk, chunk_size);
    atomic_store(&entry->data_size, current_size + chunk_size);

    pthread_rwlock_unlock(&entry->data_lock);

    notify_waiting_clients(entry);

    return 0;
}

int cache_finish_loading(cache_entry_t* entry)
{
    if (!entry || !atomic_load(&entry->is_loading)) {
        return -1;
    }

    pthread_rwlock_wrlock(&entry->data_lock);
    entry->expiry_time = time(NULL) + 10;

    atomic_store(&entry->is_loading, false);

    size_t current_size = atomic_load(&entry->data_size);

    if (entry->allocated_size > current_size) {
        char* optimized_data = realloc(entry->data, current_size);

        if (optimized_data) {
            entry->data = optimized_data;
            entry->allocated_size = current_size;
        }
    }

    pthread_rwlock_unlock(&entry->data_lock);

    notify_waiting_clients(entry);

    return 0;
}

int cache_cancel_loading(cache_t* cache, const char* key)
{
    unsigned long bucket_idx = cache_hash(key, cache->num_buckets);
    cache_bucket_t* bucket = &cache->buckets[bucket_idx];

    pthread_mutex_lock(&bucket->mutex);

    cache_entry_t* entry = bucket->head;
    cache_entry_t* prev = NULL;

    while (entry) {
        if (strcmp(entry->key, key) == 0 && atomic_load(&entry->is_loading)) {
            if (prev) {
                prev->next = entry->next;
            } else {
                bucket->head = entry->next;
            }

            cache->current_entries--;
            free_entry(entry);

            pthread_mutex_unlock(&bucket->mutex);
            return 0;
        }
        prev = entry;
        entry = entry->next;
    }

    pthread_mutex_unlock(&bucket->mutex);
    return -1;
}

void cache_clean_expired(cache_t* cache)
{
    time_t now = time(NULL);

    for (size_t i = 0; i < cache->num_buckets; i++) {
        cache_bucket_t* bucket = &cache->buckets[i];

        pthread_mutex_lock(&bucket->mutex);

        cache_entry_t* entry = bucket->head;
        cache_entry_t* prev = NULL;
        cache_entry_t* next;

        while (entry) {
            next = entry->next;
            if (entry->is_loading)
                continue;
            if (entry->expiry_time < now) {
                if (prev) {
                    prev->next = next;
                } else {
                    bucket->head = next;
                }

                cache->current_size -= entry->allocated_size;
                cache->current_entries--;
                free_entry(entry);
            } else {
                prev = entry;
            }
            entry = next;
        }

        pthread_mutex_unlock(&bucket->mutex);
    }
}

unsigned long cache_hash(const char* key, size_t num_buckets)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *key++)) {
        hash = ((hash << 5) + hash) + c;
    }

    return hash % num_buckets;
}

static void notify_waiting_clients(cache_entry_t* entry)
{
    if (!entry)
        return;

    pthread_mutex_lock(&entry->waiters_mutex);

    cache_waiter_t* waiter = entry->waiting_clients;

    printf("[BGW %d] Start notify\n", gettid());
    while (waiter) {
        if (waiter->async_handle && !uv_is_closing((uv_handle_t*)waiter->async_handle)) {
            printf("[BGW %d] Send notify to %p\n", gettid(), waiter->async_handle);
            uv_async_send(waiter->async_handle);
        }
        waiter = waiter->next;
    }

    pthread_mutex_unlock(&entry->waiters_mutex);
}

static cache_waiter_t* create_waiter(uv_async_t* async_handle)
{
    cache_waiter_t* waiter = malloc(sizeof(cache_waiter_t));

    if (!waiter)
        return NULL;

    waiter->async_handle = async_handle;
    waiter->next = NULL;
    return waiter;
}

static void free_waiters_list(cache_waiter_t* waiters)
{
    while (waiters) {
        cache_waiter_t* next = waiters->next;

        free(waiters);
        waiters = next;
    }
}

int cache_register_waiter(cache_t* cache,
    const char* key,
    uv_async_t* async_handle)
{
    unsigned long bucket_idx = cache_hash(key, cache->num_buckets);
    cache_bucket_t* bucket = &cache->buckets[bucket_idx];

    pthread_mutex_lock(&bucket->mutex);

    cache_entry_t* entry = bucket->head;
    cache_waiter_t* new_waiter = create_waiter(async_handle);

    if (!new_waiter) {
        pthread_mutex_unlock(&entry->waiters_mutex);
        pthread_mutex_unlock(&bucket->mutex);
        return -1;
    }

    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            pthread_mutex_lock(&entry->waiters_mutex);

            printf("[Backend %d] Register waiter %p to %s\n", gettid(), async_handle,
                key);
            new_waiter->next = entry->waiting_clients;
            entry->waiting_clients = new_waiter;

            pthread_mutex_unlock(&entry->waiters_mutex);
            pthread_mutex_unlock(&bucket->mutex);
            printf("[Backend %d] Successfully registered waiter %p to %s\n", gettid(),
                async_handle, key);
            return 0;
        }
        entry = entry->next;
    }

    cache_waiter_table_entry_t* current = bucket->pending_waiters.head;
    cache_waiter_table_entry_t* prev = NULL;

    while (current) {
        if (strcmp(current->key, key) == 0) {
            new_waiter->next = current->waiter;
            current->waiter = new_waiter;
            pthread_mutex_unlock(&bucket->mutex);
            return 0;
        }
        prev = current;
        current = current->next;
    }

    cache_waiter_table_entry_t* new_entry = malloc(sizeof(cache_waiter_table_entry_t));

    if (!new_entry) {
        free(new_waiter);
        pthread_mutex_unlock(&bucket->mutex);
        return -1;
    }

    new_entry->key = strdup(key);
    new_entry->waiter = new_waiter;
    new_entry->next = bucket->pending_waiters.head;
    bucket->pending_waiters.head = new_entry;

    pthread_mutex_unlock(&bucket->mutex);
    return 0;
}

int cache_unregister_waiter(cache_t* cache,
    const char* key,
    uv_async_t* async_handle)
{
    unsigned long bucket_idx = cache_hash(key, cache->num_buckets);
    cache_bucket_t* bucket = &cache->buckets[bucket_idx];

    pthread_mutex_lock(&bucket->mutex);

    cache_entry_t* entry = bucket->head;

    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            pthread_mutex_lock(&entry->waiters_mutex);

            cache_waiter_t* prev = NULL;
            cache_waiter_t* current = entry->waiting_clients;

            while (current) {
                if (current->async_handle == async_handle) {
                    if (prev) {
                        prev->next = current->next;
                    } else {
                        entry->waiting_clients = current->next;
                    }
                    printf("[BGW %d] Deleted waiter %p of %s\n", gettid(), async_handle,
                        key);
                    free(current);
                    break;
                }
                prev = current;
                current = current->next;
            }

            pthread_mutex_unlock(&entry->waiters_mutex);
            pthread_mutex_unlock(&bucket->mutex);
            return 0;
        }
        entry = entry->next;
    }

    pthread_mutex_unlock(&bucket->mutex);
    return -1;
}
