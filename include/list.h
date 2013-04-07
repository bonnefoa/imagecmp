#ifndef __LIST_H__
#define __LIST_H__

#include <stdlib.h>

typedef struct list {
        struct list * next;
        void * value;
} list_t;

list_t * list_create_node(void);
void list_release(list_t * list);
void list_release_custom(list_t * list, void (*free_funct)(void*));
list_t * list_append(list_t * list, void * value);
list_t * list_concat(list_t * list, list_t * list_2);

list_t * list_files(const char * dir_path);
int list_size(list_t *list);

#endif
