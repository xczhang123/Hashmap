#include "linkedlist.h"

linkedlist* list_init() {
    linkedlist *list = (linkedlist*)malloc(sizeof(linkedlist));
    list->head = NULL;
    list->size = 0;
    pthread_mutex_init(&(list->lock), NULL);

    return list;
}

void list_add(linkedlist *list, void *k, void *v,  int (*cmp)(void*,void*), 
                void (*key_destruct)(void*), void (*value_destruct)(void*)) {
    if (list != NULL) {
        // pthread_mutex_lock(&list->lock);
        if (list->head == NULL) {
           list->head = (node*)malloc(sizeof(node));
           list->head->k = k;
           list->head->v = v;
           list->head->next = NULL;
           pthread_mutex_init(&list->head->lock, NULL);
        //    pthread_mutex_unlock(&list->lock);
        } else {
            // pthread_mutex_unlock(&list->lock);
            node* cursor = list->head;
            node* prev = NULL;
            while (cursor != NULL) {
                // If k already exists
                if (find_key(cursor, k, cmp) == 1) {
                    key_destruct(cursor->k);
                    value_destruct(cursor->v);
                    cursor->k = k;
                    cursor->v = v;

                    return;
                }
                prev = cursor;
                cursor = cursor->next;
            }
            //If k does not exist
            node* n = malloc(sizeof(struct node));
            n->k = k;
            n->v = v;
            n->next = NULL;
            pthread_mutex_init(&n->lock, NULL);

            // pthread_mutex_lock(&n->lock);
            if (prev->next == NULL) {
                prev->next = n;
            }
            // } else {
            //     while(cursor->next != NULL) {
            //         cursor = cursor->next;
            //     }
            //     cursor->next = n;
            // }
            // pthread_mutex_unlock(&n->lock);
        }
        
        // pthread_mutex_lock(&list->lock);
        list->size++;
        // pthread_mutex_unlock(&list->lock);
    }
}

void list_add_all(linkedlist *target, linkedlist *src, int (*cmp)(void*,void*), 
                void (*key_destruct)(void*), void (*value_destruct)(void*)) {
    node *cursor = src->head;
    while (cursor != NULL) {
        list_add(target, cursor->k, cursor->v, cmp, key_destruct, value_destruct);
        cursor = cursor->next;
    }
}

int find_key(node *n, void *k, int (*cmp)(void*,void*)) {
    if (cmp(n->k, k) == 1) {
        return 1;
    } else {
        return 0;
    }
}

node* list_get(linkedlist *list, void *k, int (*cmp)(void*,void*)) {
    node *cursor = list->head;
    while (cursor != NULL) {
        if (find_key(cursor, k, cmp) == 1) {
            return cursor;
        }
        cursor = cursor->next;
    }

    //Not found
    return NULL;
}

/* Return 1 when key is found, 0 otherwise */
int list_delete(linkedlist *list, void *k, int (*cmp)(void*,void*), void (*key_destruct)(void*), void (*value_destruct)(void*)) {
    node *curr = list->head;
    node *prev = NULL;

    while (curr != NULL) {
        //If the key is found
        if (find_key(curr, k, cmp) == 1) {
            //If it is the first node
            if (prev == NULL) {
                list->head = curr->next; //Change the head pointer
            } else {
                prev->next = curr->next;
            }

            key_destruct(curr->k);
            value_destruct(curr->v);
            free(curr);
            list->size--;
            
            return 1;
        } else { //Keep searching
            prev = curr;
            curr = curr->next;
        }
    }

    return 0;
}

node* list_next(const node *n) {
    if (n == NULL) {
        return NULL;
    }

    return n->next;
}

void list_free(linkedlist *list, void (*key_destruct)(void*), void (*value_destruct)(void*)) {
    node *curr = list->head;

    while (curr != NULL) {
        node *temp = curr->next;
        key_destruct(curr->k);
        value_destruct(curr->v);
        free(curr);
        curr = temp;
    }
    free(list);
}

void list_free_without_key_value(linkedlist *list) {
    node *curr = list->head;

    while (curr != NULL) {
        node *temp = curr->next;
        free(curr);
        curr = temp;
    }
    free(list);

}

// typedef struct command command_t;

// struct command {
// 	char* str;
// 	int (*exe)();
// };

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

// int test_not_safe() {
//     int key[] = {1, 2, 3, 4, 1};
//     int value[] = {1, 2, 3, 4, 5};

//     linkedlist *head = list_init();

//     for (int i = 0; i < 5; i++) {
//         int *k = malloc(sizeof(int));
//         memcpy(k, &key[i], sizeof(int));
//         int *v = malloc(sizeof(int));
//         memcpy(v, &value[i], sizeof(int));
//         list_add(head, k, v, &cmp, &key_destruct, &value_destruct);
//     }

//     list_delete(head, &key[3], cmp, key_destruct, value_destruct);

//     // node *cursor = head->head;
//     // for (int i = 0; i < head->size; i++) {
//     //     printf("Key: %d\n", *(int*)cursor->k);
//     //     printf("Value: %d\n", *(int*)cursor->v);
//     //     cursor = cursor->next;
//     // }

//     assert(head->size == 3);

//     for (int i = 0; i < 5; i++) {
//         int *k = malloc(sizeof(int));
//         memcpy(k, &key[i], sizeof(int));
//         int *v = malloc(sizeof(int));
//         memcpy(v, &value[i], sizeof(int));
//         list_add(head, k, v, &cmp, &key_destruct, &value_destruct);
//     }

//     assert(head->size == 5);

//     list_free(head, &key_destruct, &value_destruct);

//     return 1;
// }


// command_t tests[] = {
//    {"test_not_safe", &test_not_safe},
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
