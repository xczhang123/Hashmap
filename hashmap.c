#include <stdlib.h>
#include "hashmap.h"

struct hash_map* hash_map_new(size_t (*hash)(void*), int (*cmp)(void*,void*),
    void (*key_destruct)(void*), void (*value_destruct)(void*)) {
	if (hash == NULL || cmp == NULL || key_destruct == NULL || value_destruct == NULL) {
        return NULL;;
    }

    return hash_map_init(HASH_MAP_DEF_CAPACITY, hash, cmp, key_destruct, value_destruct);
}

void hash_map_put_entry_move(struct hash_map* map, void* k, void* v) {
	hash_map_add(map, k, v);
}

void hash_map_remove_entry(struct hash_map* map, void* k) {
	hash_map_delete(map, k);
}

void* hash_map_get_value_ref(struct hash_map* map, void* k) {
    void *v = hash_map_get(map, k);

    if (v == NULL) {
        return NULL;
    } else {
        return v;
    }
}

void hash_map_destroy(struct hash_map* map) {
	hash_map_free(map);
}