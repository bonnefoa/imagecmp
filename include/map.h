#ifndef __MAP_H__
#define __MAP_H__

#include <stdlib.h>

typedef struct map_entry {
        void *key;
        void *val;
        struct map_entry *next;
} map_entry_t;

typedef struct map {
        int size;
        map_entry_t ** hash_table;
} map_t;

map_t * map_create(int size);
void map_release(map_t * map);
void map_release_custom(map_t * map, void (*free_funct)(void*));
map_t * map_add(map_t * map, const void * key, void * value);
map_t * map_delete(map_t * map, void * key);
void* * map_get(map_t * map, void * key);

#endif
