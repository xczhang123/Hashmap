#include <stdlib.h>
#include "hashmap.h"
#include <time.h>
#include <unistd.h>

struct command {
	char* str;
	int (*exe)();
};

struct thread_arg {
    struct hash_map* hm;
    int start;
    int n;
};

typedef struct command command_t;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int cmp(void* k1, void* k2) {
    int *i1 = (int*)k1;
    int *i2 = (int*)k2;

    if (*i1 == *i2) {
        return 1;
    } else {
        return 0;
    }
}

size_t hash(void *k) {
    int n = ((*(int*)k) * (311));
    // printf("What is n: %d\n", n);
    return n;
}

void key_destruct(void *k) {
    int *i = (int*)k;
    free(i);
}

void value_destruct(void *v) {
    int *i = (int*)v;
    free(i);
}

void* thread_add(void* arg) {
    struct thread_arg *a = (struct thread_arg*) arg;
    for (int i = 0; i < a->n; i++) {
        int *k = malloc(sizeof(int));
        memcpy(k, &a->start, sizeof(int));
        // int i = 1;
        // memcpy(k, &i, sizeof(int));
        int *v = malloc(sizeof(int));
        // memcpy(v, &i, sizeof(int));
        memcpy(v, &a->start, sizeof(int));
        hash_map_add(a->hm, k, v);
        a->start++;

        // printf("The key is %d and the value is %d\n", *(int*)k, *(int*)v);
    }

    free(a);

    return NULL;
}

void* thread_delete(void *arg) {
    struct thread_arg *a = (struct thread_arg*) arg;
    for (int i = 0; i < a->n; i++) {
        int k = a->start;
        hash_map_remove_entry(a->hm, &k);
        a->start++;
    }

    free(a);

    return NULL;
}

void* thread_get(void *arg) {
    struct thread_arg *a = (struct thread_arg*) arg;
    for (int i = 0; i < a->n; i++) {
        // pthread_mutex_lock(&a->hm->lock);
        // pthread_mutex_lock(&mutex);
        int k = rand() % 10;
        hash_map_get_value_ref(a->hm, &k);
        // pthread_mutex_unlock(&mutex);
        // pthread_mutex_unlock(&a->hm->lock);
        // if (result != NULL)
            // printf("The key is %d and the result is %d\n", k, *(int*)result);
        a->start++;
    }

    free(a);
    return NULL;
}

int test_safe_add() {
    hash_map* hm = hash_map_new(&hash, &cmp,&key_destruct, &value_destruct);

    pthread_t threads[5];

    int n = 5;
    for (int i = 0; i < 5; i++) {
        struct thread_arg *a = malloc(sizeof(struct thread_arg));
        a->hm = hm;
        a->start = n * i;
        a->n = n;
        pthread_create(threads+i, NULL, thread_add, a);
    }

    for (int i = 0; i < 5; i++) {
        pthread_join(threads[i], NULL);
    }

    // for (size_t i = 0; i < hm->capacity; i++) {
    //     linkedlist *list = hm->data[i];
    //     node *cursor = list->head;
    //     for (size_t j = 0; j < list->size; j++) {
    //         printf("The key is %d, and value is %d\n", *(int*)cursor->k, *(int*)cursor->v);
    //         cursor = cursor->next;
    //     }
    // }

    // printf("%zu\n", hm->data[0]->size);
    printf("%zu\n", hm->size);

    assert(hm->size == 25);

    hash_map_destroy(hm);

    return 1;
}   

int test_safe_delete() {
    hash_map* hm = hash_map_new(&hash, &cmp,&key_destruct, &value_destruct);

    pthread_t threads[5];

    int n = 5;
    for (int i = 0; i < 5; i++) {
        struct thread_arg *a = malloc(sizeof(struct thread_arg));
        a->hm = hm;
        a->start = n * i;
        a->n = n;
        pthread_create(threads+i, NULL, thread_add, a);
    }

    for (int i = 0; i < 5; i++) {
        pthread_join(threads[i], NULL);
    }
    
    //Delete everything
    for (int i = 0; i < 5; i++) {
        struct thread_arg *a = malloc(sizeof(struct thread_arg));
        a->hm = hm;
        a->start = n * i;
        a->n = n;
        pthread_create(threads+i, NULL, thread_delete, a);
    }

    for (int i = 0; i < 5; i++) {
        pthread_join(threads[i], NULL);
    }

    // assert(hm->capacity == 32);
    printf("%zu\n", hm->size);
    assert(hm->size == 0);

    hash_map_destroy(hm);

    return 1;
}

int test_safe_get() {
    hash_map* hm = hash_map_new(&hash, &cmp,&key_destruct, &value_destruct);

    pthread_t threads[20];

    int n = 5;
    for (int i = 0; i < 5; i++) {
        struct thread_arg *a = malloc(sizeof(struct thread_arg));
        a->hm = hm;
        a->start = n * i;
        a->n = n;
        pthread_create(threads+i, NULL, thread_add, a);
    }

    for (int i = 0; i < 5; i++) {
        pthread_join(threads[i], NULL);
    }
        
    //Delete everything and get something
    for (int i = 0; i < 10; i++) {
        struct thread_arg *a = malloc(sizeof(struct thread_arg));
        struct thread_arg *b = malloc(sizeof(struct thread_arg));
        a->hm = hm;
        a->start = n * i;
        a->n = n;

        b->hm = hm;
        b->start = n * i;
        b->n = n;
        
        if (i % 2 == 0) {
            pthread_create(threads+i+10, NULL, thread_get, b);
            pthread_create(threads+i, NULL, thread_delete, a);
        } else {
            pthread_create(threads+i, NULL, thread_delete, a);
            pthread_create(threads+i+10, NULL, thread_get, b);
        }
    }

    for (int i = 0; i < 10; i++) {
        pthread_join(threads[i], NULL);
    }

    // assert(hm->capacity == 32);
    printf("%zu\n", hm->size);
    assert(hm->size == 0);

    hash_map_destroy(hm);

    return 1;
}

int test_safe_complex() {
    hash_map* hm = hash_map_new(&hash, &cmp,&key_destruct, &value_destruct);

    pthread_t threads[100];

    int n = 10;
    for (int i = 0; i < 100; i++) {
        struct thread_arg *a = malloc(sizeof(struct thread_arg));
        struct thread_arg *b = malloc(sizeof(struct thread_arg));
        struct thread_arg *c = malloc(sizeof(struct thread_arg));
        a->hm = hm;
        a->start = i;
        a->n = n;

        b->hm = hm;
        b->start = i;
        b->n = n;

        c->hm = hm;
        c->start = i;
        c->n = n;

        int n = rand() % 3;
        if (n == 0) {
            pthread_create(threads+i, NULL, thread_add, a);
            pthread_create(threads+i, NULL, thread_delete, b);
            pthread_create(threads+i, NULL, thread_get, c);
        } else if (n == 1) {
            pthread_create(threads+i, NULL, thread_delete, b);
            pthread_create(threads+i, NULL, thread_add, a);
            pthread_create(threads+i, NULL, thread_get, c);
        } else {
            pthread_create(threads+i, NULL, thread_get, c);
            pthread_create(threads+i, NULL, thread_delete, b);
            pthread_create(threads+i, NULL, thread_add, a);
        }
    }

    for (int i = 0; i < 100; i++) {
        pthread_join(threads[i], NULL);
    }

    // assert(hm->capacity == 32);
    printf("%zu\n", hm->capacity);
    // assert(hm->size == 0);
    
    hash_map_destroy(hm);
    return 1;
}

command_t tests[] = {
   {"test_safe_add", &test_safe_add},
   {"test_safe_delete", &test_safe_delete},
   {"test_safe_get", &test_safe_get},
   {"test_safe_complex", &test_safe_complex}
};

int main(int argc, char** argv) {
  double time_spend = 0.0;
  clock_t begin = clock();

//   time_t b = time(NULL);
//   sleep(10);

//   printf("Time is %ld seconds\n", e-b);

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
    clock_t end = clock();
    // time_t e = time(NULL);

    time_spend = (double)(end - begin) / CLOCKS_PER_SEC;

    printf("Time elapsed is %f seconds\n", time_spend);
}

