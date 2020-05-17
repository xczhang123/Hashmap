#include "dyn_array.h"

void hash_map_rehash(hash_map *hm) {

    //Resize old hash table hm
    hm->capacity *= 2;

    pthread_mutex_lock(&hm->get_lock);
    hm->data = realloc(hm->data, sizeof(*hm->data)*(hm->capacity));
    pthread_mutex_unlock(&hm->get_lock);

    // Initialize newly created buckets
    for (size_t i = hm->capacity/2; i < hm->capacity; i++) {
        hm->data[i] = list_init();
    }

    // Free old linkedlists and reinitializing
    for (size_t i = 0; i < hm->capacity/2; i++) {
        linkedlist *list = hm->data[i];
        // pthread_mutex_lock(&list->lock);
        hash_map_rehash_add_all(hm, list);
        // pthread_mutex_unlock(&list->lock);
    }
}


/* Will only be called when the thread holds hm->lock (i.e. when rehashing the whole table) */
void hash_map_rehash_add_all(hash_map *hm, linkedlist* src) {

    node *curr = src->head;

    while (curr != NULL) {
        node *next = curr->next;

        size_t old_index = hm->hash(curr->k) % (hm->capacity/2);
        size_t new_index = hm->hash(curr->k) % hm->capacity;

        // printf("Old index %ld\n", old_index);
        // printf("new index %ld\n", new_index);

        if (old_index != new_index) {

            src->head = curr->next; //Change the head pointer
            
            if (hm->data[new_index]->size == 0) {
                hm->size++;
            }
            list_add(hm->data[new_index], curr->k, curr->v, hm->cmp, hm->key_destruct, hm->value_destruct);
            
            free(curr);
            hm->data[old_index]->size--;

            // printf("Before deletion, my old list is %ld\n", hm->data[old_index]->size);
            if (hm->data[old_index]->size == 0) {
                hm->size--;
            }
            // printf("After deletion, my size is %ld\n", hm->size);

            // printf("the size is after %ld\n", hm->data[0]->size);
        } 
        curr = next;
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
    pthread_mutex_init(&hm->lock, NULL);
    pthread_mutex_init(&hm->get_lock, NULL);

    return hm;
}

void hash_map_add(hash_map *hm, void *k, void *v) {

    //Load factor -> if greater than 75%, we rehash the table
    float n = 0.0f;

    pthread_mutex_lock(&hm->lock);
    size_t index = hm->hash(k) % hm->capacity;
    pthread_mutex_unlock(&hm->lock);

    //Increment the size if the bucket is empty before the add
    if (hm->data[index]->head == NULL) {
        pthread_mutex_lock(&hm->lock);
        hm->size++;
        pthread_mutex_unlock(&hm->lock);
    }

    pthread_mutex_lock(&hm->data[index]->lock); 
    list_add(hm->data[index], k, v, hm->cmp, hm->key_destruct, hm->value_destruct);
    pthread_mutex_unlock(&hm->data[index]->lock);

    //Calculate Load factor
    pthread_mutex_lock(&hm->lock);
	n = (1.0 * hm->size) / hm->capacity;
	if (n >= 0.75) {
		//rehashing
		hash_map_rehash(hm);
	}
    pthread_mutex_unlock(&hm->lock);
}

void hash_map_delete(hash_map *hm, void *k) {

    pthread_mutex_lock(&hm->lock);
    size_t index = hm->hash(k) % hm->capacity;
    pthread_mutex_unlock(&hm->lock);

    pthread_mutex_lock(&hm->data[index]->lock); 
    int deleted = list_delete(hm->data[index], k, hm->cmp, hm->key_destruct, hm->value_destruct);
    int new_size = hm->data[index]->size;
    pthread_mutex_unlock(&hm->data[index]->lock); 

    //If the bucket is empty after the deletion
    if (deleted && new_size == 0) {
        pthread_mutex_lock(&hm->lock);
        hm->size--;
        pthread_mutex_unlock(&hm->lock);
    }
    // pthread_mutex_unlock(&hm->lock);
}

void* hash_map_get(hash_map *hm, void *k) {
    
    pthread_mutex_lock(&hm->lock);
    size_t index = hm->hash(k) % hm->capacity;
    pthread_mutex_unlock(&hm->lock);

    pthread_mutex_lock(&hm->get_lock);
    pthread_mutex_lock(&hm->data[index]->lock);
    void *n = list_get(hm->data[index], k, hm->cmp);
    // printf("try lock: %d\n", pthread_mutex_trylock(&hm->data[index]->lock));
    pthread_mutex_unlock(&hm->data[index]->lock);
    pthread_mutex_unlock(&hm->get_lock);
   
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


// int cmp(void* k1, void* k2) {
//     int *i1 = (int*)k1;
//     int *i2 = (int*)k2;

//     if (*i1 == *i2) {
//         return 1;
//     } else {
//         return 0;
//     }
// }

// void key_destruct(void *k) {
//     int *i = (int*)k;
//     free(i);
// }

// void value_destruct(void *v) {
//     int *i = (int*)v;
//     free(i);
// }

// size_t hash(void *k) {
//     return ((*(int*)k) % (HASH_MAP_DEF_CAPACITY));
// }


// typedef struct command command_t;

// struct command {
// 	char* str;
// 	int (*exe)();
// };

// int test_not_safe1() {
//     int key[] = {99, 100, 101, 102, 103};
//     int value[] = {1, 2, 3, 4, 5};

//     hash_map *hm = hash_map_init(HASH_MAP_DEF_CAPACITY, &hash, &cmp,
//                                 &key_destruct, &value_destruct);
//     for (int i = 0; i < 25; i++) {
//         int *k = malloc(sizeof(int));
//         memcpy(k, &key[i%5], sizeof(int));
//         int *v = malloc(sizeof(int));
//         memcpy(v, &value[i%5], sizeof(int));
//         hash_map_add(hm, k, v);
//     }

//     assert(hm->size == 5);

//     hash_map_free(hm);

//     return 1;
// }

// int test_not_safe2() {
//     hash_map *hm = hash_map_init(HASH_MAP_DEF_CAPACITY, &hash, &cmp,
//                                 &key_destruct, &value_destruct);
//     for (int i = 15; i < 30; i++) {
//         int *k = malloc(sizeof(int));
//         memcpy(k, &i, sizeof(int));
//         int *v = malloc(sizeof(int));
//         memcpy(v, &i, sizeof(int));
//         hash_map_add(hm, k, v);
//     }

//     assert(hm->size == 15);
//     assert(hm->capacity == 32);

//     hash_map_free(hm);

//     return 1;
// }

// int test_not_safe3() {
//     hash_map *hm = hash_map_init(HASH_MAP_DEF_CAPACITY, &hash, &cmp,
//                                 &key_destruct, &value_destruct);
//     for (int i = 15; i < 30; i++) {
//         int *k = malloc(sizeof(int));
//         memcpy(k, &i, sizeof(int));
//         int *v = malloc(sizeof(int));
//         memcpy(v, &i, sizeof(int));
//         hash_map_add(hm, k, v);
//     }

//     for (int i = 15; i < 30; i++) {
//         int target = i;
//         hash_map_delete(hm, &target);
//     }

//     assert(hm->size == 0);
//     assert(hm->capacity == 32);

//     hash_map_free(hm);

//     return 1;
// }

// command_t tests[] = {
//    {"test_not_safe1", &test_not_safe1},
//    {"test_not_safe2", &test_not_safe2},
//    {"test_not_safe3", &test_not_safe3}
// };


// int main(int argc, char** argv) {
//   int test_n = sizeof(tests) / sizeof(command_t);
//   if(argc >= 2) {
// 		for(int i = 0; i < test_n; i++) {
// 			if(strcmp(argv[1], tests[i].str) == 0) {
// 				if(tests[i].exe()) {
// 				  fprintf(stdout, "%s Passed\n", tests[i].str);
// 				} else {
// 				  fprintf(stdout, "%s Failed\n", tests[i].str);
// 				}
// 			}
// 		}
//         if (strcmp(argv[1], "all") == 0) {
//             for(int i = 0; i < test_n; i++) {
// 				if(tests[i].exe()) {
// 				  fprintf(stdout, "%s Passed\n", tests[i].str);
// 				} else {
// 				  fprintf(stdout, "%s Failed\n", tests[i].str);
// 				}
//             }
//         }
// 	}
// }