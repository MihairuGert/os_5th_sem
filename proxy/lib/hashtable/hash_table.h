#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stddef.h>
#include <stdbool.h>

typedef struct HashTable HashTable;

typedef size_t (*HashFunction)(const void* key);
typedef int (*KeyCompare)(const void* key1, const void* key2);
typedef void (*KeyDestructor)(void* key);
typedef void (*ValueDestructor)(void* value);

HashTable* hash_table_create(
    size_t capacity,
    HashFunction hash_func,
    KeyCompare key_compare,
    KeyDestructor key_destructor,
    ValueDestructor value_destructor
);

void hash_table_destroy(HashTable* ht);
bool hash_table_insert(HashTable* ht, void* key, void* value);
void* hash_table_get(const HashTable* ht, const void* key);
bool hash_table_remove(HashTable* ht, const void* key);
bool hash_table_contains(const HashTable* ht, const void* key);
size_t hash_table_size(const HashTable* ht);

#endif