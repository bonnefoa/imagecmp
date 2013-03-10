#include <string.h>
#include <list.h>
#include <stdio.h>
#include <dirent.h>

list_t * list_create_node()
{
        struct list *list;
        if((list = malloc(sizeof(*list))) == NULL) {
                return NULL;
        }
        list->next = NULL;
        list->value = NULL;
        return list;
}

void list_release(list_t * list)
{
        list_t * curr = list;
        list_t * next = NULL;
        while(curr) {
                next = curr->next;
                free(curr->value);
                free(curr);
        }
}

list_t * list_append(list_t * list, void * value)
{
        if (value == NULL) {
                return list;
        }
        list_t * newNode = list_create_node();
        newNode->next = list;
        newNode->value = value;
        return newNode;
}

list_t * list_files(const char * dir_path)
{
        list_t * list = NULL;
        int pathLen = strlen(dir_path);
        DIR *dir;
        struct dirent *ent;
        if ((dir = opendir (dir_path)) != NULL) {
                while ((ent = readdir (dir)) != NULL) {
                        if (strncmp(".", ent->d_name, 1) == 0) {
                                continue;
                        }
                        size_t sizeStr = strlen(ent->d_name) + pathLen + 2;
                        char * fullPath = malloc(sizeStr + 1);
                        strncpy(fullPath, dir_path, sizeStr);
                        strcat(fullPath, "/");
                        strcat(fullPath, ent->d_name);
                        list = list_append(list, fullPath);
                }
                closedir (dir);
        } else {
                perror ("");
        }
        return list;
}
