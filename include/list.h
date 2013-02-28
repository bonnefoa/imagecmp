#ifndef __LIST_H__
#define __LIST_H__

#include <stdlib.h>

typedef struct listNode {
  struct listNode * next;
  void * value;
} listNode;

listNode * listCreateNode(void);
void listRelease(listNode * list);
listNode * listAppend(listNode * list, void * value);

listNode * listFiles(const char * dirPath);

#endif
