#ifndef __LIST_H__
#define __LIST_H__

#include <stdlib.h>

typedef struct listNode {
        struct listNode * next;
        void * value;
} listNode_t;

listNode_t * list_create_node(void);
void list_release(listNode_t * list);
listNode_t * list_append(listNode_t * list, void * value);

listNode_t * list_files(const char * dir_path);

#endif
