#ifndef LINKEDLIST
#define LINKEDLIST

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>

typedef struct linkedlist {
    pthread_mutex_t lock;
    struct node *head;
    size_t size;
} linkedlist;

typedef struct node {
    // pthread_mutex_t lock;
    void *k;
    void *v;
    struct node *next;
} node;

linkedlist* list_init();

void list_add(linkedlist *list, void *k, void *v,  int (*cmp)(void*,void*), 
            void (*key_destruct)(void*), void (*value_destruct)(void*));

void list_add_all(linkedlist *target, linkedlist *src, int (*cmp)(void*,void*), 
                void (*key_destruct)(void*), void (*value_destruct)(void*));

int find_key(node *n, void *k, int (*cmp)(void*,void*));

void* list_get(linkedlist *list, void *k, int (*cmp)(void*,void*));

int list_delete(linkedlist *list, void *k, int (*cmp)(void*,void*), 
                void (*key_destruct)(void*), void (*value_destruct)(void*));

node* list_next(const node *n);

void list_free(linkedlist *list, void (*key_destruct)(void*), void (*value_destruct)(void*));

void list_free_without_key_value(linkedlist *list);

#endif