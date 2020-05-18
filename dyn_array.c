#include "dyn_array.h"

void hash_map_rehash(hash_map *hm) {
    //Create temporary bucket list
    linkedlist *temp = list_init();

    //Add all entries to temp bucket list
    for (size_t i = 0; i < hm->capacity; i++) {
        linkedlist *list = hm->data[i];
        list_add_all(temp, list, hm->cmp, hm->key_destruct, hm->value_destruct);
    }

    //Resize old hash table hm
    hm->capacity *= 2;
    hm->size = 0;
    hm->data = realloc(hm->data, sizeof(*hm->data)*(hm->capacity));
    // Free old linkedlists and reinitializing
    for (size_t i = 0; i < hm->capacity/2; i++) {
        linkedlist *list = hm->data[i];
        if (list->head != NULL) {
            list_free_without_key_value(list);
            hm->data[i] = list_init();
        }
    }
    // Initialize newly created buckets
    for (size_t i = hm->capacity/2; i < hm->capacity; i++) {
        hm->data[i] = list_init();
    }

    //Add all entries in temp to original hash table
    hash_map_rehash_add_all(hm, temp);
    //Free temp bucket
    list_free_without_key_value(temp);
}

/* Will only be called when the thread holds hm->lock (i.e. when rehashing the whole table) */
void hash_map_rehash_add_all(hash_map *hm, linkedlist* src) {
    node *cursor = src->head;
    while (cursor != NULL) {
        node *temp = cursor->next;
        
        size_t index = hm->hash(cursor->k) % hm->capacity;
        if (hm->data[index]->head == NULL) {
            hm->size++;
        }
        list_add(hm->data[index], cursor->k, cursor->v, hm->cmp, hm->key_destruct, hm->value_destruct);

        cursor = temp;
    }
}

hash_map* hash_map_init(size_t size, size_t (*hash)(void*), int (*cmp)(void*,void*),
                void (*key_destruct)(void*), void (*value_destruct)(void*)) {
    hash_map *hm = (hash_map*)malloc(sizeof(hash_map));
    hm->capacity = size;
    hm->size = 0;
    hm->data = (linkedlist**)malloc(sizeof(*hm->data) * hm->capacity);
    for (size_t i = 0; i < hm->capacity; i++) {
        hm->data[i] = list_init();
    }
    hm->hash = hash;
    hm->cmp = cmp;
    hm->key_destruct = key_destruct;
    hm->value_destruct = value_destruct;

    return hm;
}

void hash_map_add(hash_map *hm, void *k, void *v) {

    //Load factor -> if greater than 75%, we rehash the table
    float n = 0.0f;

    size_t index = hm->hash(k) % hm->capacity;

    //Increment the size if the bucket is empty before the add
    if (hm->data[index]->head == NULL) {
        hm->size++;
    }

    list_add(hm->data[index], k, v, hm->cmp, hm->key_destruct, hm->value_destruct);

    //Calculate Load factor
	n = (1.0 * hm->size) / hm->capacity;
	if (n >= 0.75) {
		//rehashing
		hash_map_rehash(hm);
	}
}

void hash_map_delete(hash_map *hm, void *k) {

    size_t index = hm->hash(k) % hm->capacity;

    int deleted = list_delete(hm->data[index], k, hm->cmp, hm->key_destruct, hm->value_destruct);
    int new_size = hm->data[index]->size;

    //If the bucket is empty after the deletion
    if (deleted && new_size == 0) {
        hm->size--;
    }
}

void* hash_map_get(hash_map *hm, void *k) {
    
    size_t index = hm->hash(k) % hm->capacity;

    void *n = list_get(hm->data[index], k, hm->cmp);
   
    //If key is found, return the value; otherwise, return NULL
    if (n != NULL) {
        return n;
    } else {
        return NULL;
    }
}

void hash_map_free(hash_map *hm) {

    for (size_t i = 0; i < hm->capacity; i++) {
        linkedlist *list = hm->data[i];
        list_free(list, hm->key_destruct, hm->value_destruct);
    }

    free(hm->data);
    free(hm);
}