#ifndef DYN_ARRAY
#define DYN_ARRAY

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "linkedlist.h"

#define HASH_MAP_DEF_CAPACITY (200)

typedef struct hash_map {
    size_t capacity;
    size_t size;
    linkedlist **data;
    size_t (*hash)(void*);
    int (*cmp)(void*,void*);
    void (*key_destruct)(void*);
    void (*value_destruct)(void*);
    pthread_mutex_t lock;
    // pthread_mutex_t resize_lock;

} hash_map;

void hash_map_rehash(hash_map *hm);

hash_map* hash_map_init(size_t size, size_t (*hash)(void*), int (*cmp)(void*,void*),
                void (*key_destruct)(void*), void (*value_destruct)(void*));

void hash_map_add(hash_map *hm, void *k, void *v);

void hash_map_rehash_add_all(hash_map *hm, linkedlist* src);

void hash_map_delete(hash_map *hm, void *k);

void* hash_map_get(hash_map *hm, void *k);

void hash_map_free(hash_map *hm);

#endif