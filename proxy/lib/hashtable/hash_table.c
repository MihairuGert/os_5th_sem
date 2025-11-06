#include "hash_table.h"
#include <stdlib.h>
#include <string.h>

typedef struct HashNode {
    void* key;
    void* value;
    struct HashNode* next;
} HashNode;

struct HashTable {
    HashNode** buckets;
    size_t capacity;
    size_t size;
    HashFunction hash_func;
    KeyCompare key_compare;
    KeyDestructor key_destructor;
    ValueDestructor value_destructor;
};

HashTable* hash_table_create(
    size_t capacity,
    HashFunction hash_func,
    KeyCompare key_compare,
    KeyDestructor key_destructor,
    ValueDestructor value_destructor
) {
    HashTable* ht = malloc(sizeof(HashTable));
    if (!ht) return NULL;

    ht->buckets = calloc(capacity, sizeof(HashNode*));
    if (!ht->buckets) {
        free(ht);
        return NULL;
    }

    ht->capacity = capacity;
    ht->size = 0;
    ht->hash_func = hash_func;
    ht->key_compare = key_compare;
    ht->key_destructor = key_destructor;
    ht->value_destructor = value_destructor;

    return ht;
}

void hash_table_destroy(HashTable* ht) {
    if (!ht) return;

    for (size_t i = 0; i < ht->capacity; i++) {
        HashNode* node = ht->buckets[i];
        while (node) {
            HashNode* next = node->next;
            
            if (ht->key_destructor) ht->key_destructor(node->key);
            if (ht->value_destructor) ht->value_destructor(node->value);
            
            free(node);
            node = next;
        }
    }

    free(ht->buckets);
    free(ht);
}

bool hash_table_insert(HashTable* ht, void* key, void* value) {
    if (!ht || !key) return false;

    size_t index = ht->hash_func(key) % ht->capacity;
    
    HashNode* node = ht->buckets[index];
    while (node) {
        if (ht->key_compare(node->key, key) == 0) {
            if (ht->value_destructor) ht->value_destructor(node->value);
            node->value = value;
            return true;
        }
        node = node->next;
    }

    HashNode* new_node = malloc(sizeof(HashNode));
    if (!new_node) return false;

    new_node->key = key;
    new_node->value = value;
    new_node->next = ht->buckets[index];
    ht->buckets[index] = new_node;
    ht->size++;

    return true;
}

void* hash_table_get(const HashTable* ht, const void* key) {
    if (!ht || !key) return NULL;

    size_t index = ht->hash_func(key) % ht->capacity;
    HashNode* node = ht->buckets[index];
    
    while (node) {
        if (ht->key_compare(node->key, key) == 0) {
            return node->value;
        }
        node = node->next;
    }
    
    return NULL;
}

bool hash_table_remove(HashTable* ht, const void* key) {
    if (!ht || !key) return false;

    size_t index = ht->hash_func(key) % ht->capacity;
    HashNode** node_ptr = &ht->buckets[index];
    
    while (*node_ptr) {
        HashNode* node = *node_ptr;
        if (ht->key_compare(node->key, key) == 0) {
            *node_ptr = node->next;
            
            if (ht->key_destructor) ht->key_destructor(node->key);
            if (ht->value_destructor) ht->value_destructor(node->value);
            
            free(node);
            ht->size--;
            return true;
        }
        node_ptr = &(*node_ptr)->next;
    }
    
    return false;
}

bool hash_table_contains(const HashTable* ht, const void* key) {
    return hash_table_get(ht, key) != NULL;
}

size_t hash_table_size(const HashTable* ht) {
    return ht ? ht->size : 0;
}
