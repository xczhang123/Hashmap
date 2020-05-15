#include "dyn_array.h"

void hash_map_rehash(hash_map *hm) {
    //Create temporary bucket list
    linkedlist *temp = list_init();

    //Add all entries to temp bucket list
    for (int i = 0; i < hm->capacity; i++) {
        linkedlist *list = hm->data[i];
        list_add_all(temp, list, hm->cmp, hm->key_destruct, hm->value_destruct);
    }

    //Resize old hash table hm
    hm->capacity *= 2;
    hm->size = 0;
    hm->data = realloc(hm->data, sizeof(*hm->data)*(hm->capacity));
    // Free old linkedlists and reinitializing
    for (int i = 0; i < hm->capacity/2; i++) {
        linkedlist *list = hm->data[i];
        if (list->head != NULL) {
            list_free_without_key_value(list);
            hm->data[i] = list_init();
        }
    }
    // Initialize newly created buckets
    for (int i = hm->capacity/2; i < hm->capacity; i++) {
        hm->data[i] = list_init();
    }

    //Add all entries in temp to original hash table
    hash_map_add_all(hm, temp);
    //Free temp bucket
    list_free_without_key_value(temp);
}

hash_map* hash_map_init(size_t size, size_t (*hash)(void*), int (*cmp)(void*,void*),
                void (*key_destruct)(void*), void (*value_destruct)(void*)) {
    hash_map *hm = malloc(sizeof(hash_map));
    hm->capacity = size;
    hm->size = 0;
    hm->data = (linkedlist**)malloc(sizeof(*hm->data) * hm->capacity);
    for (int i = 0; i < hm->capacity; i++) {
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
    size_t index = hm->hash(k);

    //Increment the size if the bucket is empty before the add
    if (hm->data[index]->head == NULL) {
        hm->size++;
    }

    list_add(hm->data[index], k, v, hm->cmp, hm->key_destruct, hm->value_destruct);

    //Calculate Load factor
	n = (1.0 * hm->size) / hm->capacity;
	if (n >= 0.75) {
		//rehashing
		printf("going to rehash\n");
		hash_map_rehash(hm);
	}
}

void hash_map_add_all(hash_map *hm, linkedlist* src) {
    node *cursor = src->head;
    while (cursor != NULL) {
        node *temp = cursor->next;
        hash_map_add(hm, cursor->k, cursor->v);
        cursor = temp;
    }
}

void hash_map_delete(hash_map *hm, void *k) {

    size_t index = hm->hash(k);
    
    int deleted = list_delete(hm->data[index], k, hm->cmp, hm->key_destruct, hm->value_destruct);

    if (deleted) {
        hm->size--;
    }
}

void* hash_map_get(hash_map *hm, void *k) {

    size_t index = hm->hash(k);

    node *n = list_get(hm->data[index], k, hm->cmp);

    //If key is found, return the value
    if (n != NULL) {
        return n->v;
    } else {
        return NULL;
    }
}

int cmp(void* k1, void* k2) {
    int *i1 = (int*)k1;
    int *i2 = (int*)k2;

    if (*i1 == *i2) {
        return 1;
    } else {
        return 0;
    }
}

void key_destruct(void *k) {
    int *i = (int*)k;
    free(i);
}

void value_destruct(void *v) {
    int *i = (int*)v;
    free(i);
}

size_t hash(void *k) {
    return ((*(int*)k) % (HASH_MAP_DEF_CAPACITY));
}

void hash_map_free(hash_map *hm) {

    for (int i = 0; i < hm->capacity; i++) {
        linkedlist *list = hm->data[i];
        list_free(list, hm->key_destruct, hm->value_destruct);
    }

    free(hm->data);
    free(hm);
}



typedef struct command command_t;

struct command {
	char* str;
	int (*exe)();
};

int test_not_safe1() {
    int key[] = {99, 100, 101, 102, 103};
    int value[] = {1, 2, 3, 4, 5};

    hash_map *hm = hash_map_init(HASH_MAP_DEF_CAPACITY, &hash, &cmp,
                                &key_destruct, &value_destruct);
    for (int i = 0; i < 25; i++) {
        int *k = malloc(sizeof(int));
        memcpy(k, &key[i%5], sizeof(int));
        int *v = malloc(sizeof(int));
        memcpy(v, &value[i%5], sizeof(int));
        hash_map_add(hm, k, v);
    }

    assert(hm->size == 5);

    hash_map_free(hm);

    return 1;
}

int test_not_safe2() {
    hash_map *hm = hash_map_init(HASH_MAP_DEF_CAPACITY, &hash, &cmp,
                                &key_destruct, &value_destruct);
    for (int i = 15; i < 30; i++) {
        int *k = malloc(sizeof(int));
        memcpy(k, &i, sizeof(int));
        int *v = malloc(sizeof(int));
        memcpy(v, &i, sizeof(int));
        hash_map_add(hm, k, v);
    }

    assert(hm->size == 15);
    assert(hm->capacity == 32);

    hash_map_free(hm);

    return 1;
}

int test_not_safe3() {
    hash_map *hm = hash_map_init(HASH_MAP_DEF_CAPACITY, &hash, &cmp,
                                &key_destruct, &value_destruct);
    for (int i = 15; i < 30; i++) {
        int *k = malloc(sizeof(int));
        memcpy(k, &i, sizeof(int));
        int *v = malloc(sizeof(int));
        memcpy(v, &i, sizeof(int));
        hash_map_add(hm, k, v);
    }

    for (int i = 15; i < 30; i++) {
        int target = i;
        hash_map_delete(hm, &target);
    }

    assert(hm->size == 0);
    assert(hm->capacity == 32);

    hash_map_free(hm);

    return 1;
}

command_t tests[] = {
   {"test_not_safe1", &test_not_safe1},
   {"test_not_safe2", &test_not_safe2},
   {"test_not_safe3", &test_not_safe3}
};


int main(int argc, char** argv) {
  int test_n = sizeof(tests) / sizeof(command_t);
  if(argc >= 2) {
		for(int i = 0; i < test_n; i++) {
			if(strcmp(argv[1], tests[i].str) == 0) {
				if(tests[i].exe()) {
				  fprintf(stdout, "%s Passed\n", tests[i].str);
				} else {
				  fprintf(stdout, "%s Failed\n", tests[i].str);
				}
			}
		}
        if (strcmp(argv[1], "all") == 0) {
            for(int i = 0; i < test_n; i++) {
				if(tests[i].exe()) {
				  fprintf(stdout, "%s Passed\n", tests[i].str);
				} else {
				  fprintf(stdout, "%s Failed\n", tests[i].str);
				}
            }
        }
	}
}
