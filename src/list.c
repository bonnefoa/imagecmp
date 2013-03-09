#include <string.h>
#include <list.h>
#include <stdio.h>
#include <dirent.h>

listNode * listCreateNode() {
  struct listNode *list;
  if((list = malloc(sizeof(*list))) == NULL) {
    return NULL;
  }
  list->next = NULL;
  list->value = NULL;
  return list;
}

void listRelease(listNode * list) {
  listNode * curr = list;
  listNode * next = NULL;
  while(curr) {
    next = curr->next;
    free(curr->value);
    free(curr);
  }
}

listNode * listAppend(listNode * list, void * value) {
  if (value == NULL) {
    return list;
  }
  listNode * newNode = listCreateNode();
  newNode->next = list;
  newNode->value = value;
  return newNode;
}

listNode * listFiles(const char * dirPath) {
  listNode * list = NULL;
  int pathLen = strlen(dirPath);
  DIR *dir;
  struct dirent *ent;
  if ((dir = opendir (dirPath)) != NULL) {
    while ((ent = readdir (dir)) != NULL) {
      if (strncmp(".", ent->d_name, 1) == 0) {
        continue;
      }
      size_t sizeStr = strlen(ent->d_name) + pathLen + 2;
      char * fullPath = malloc(sizeStr + 1);
      strncpy(fullPath, dirPath, sizeStr);
      strcat(fullPath, "/");
      strcat(fullPath, ent->d_name);
      list = listAppend(list, fullPath);
    }
    closedir (dir);
  } else {
    perror ("");
  }
  return list;
}
