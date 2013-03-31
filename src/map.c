#include <map.h>
#include <string.h>
#include <stdio.h>

unsigned int hash_function(const void *key, int len) {
        int seed = 5381;
        const int m = 0x5bd1e995;
        const int r = 24;
        int h = seed ^ len;
        const unsigned char *data = (const unsigned char *)key;
        while(len >= 4) {
                int k = *(int*)data;
                k *= m;
                k ^= k >> r;
                k *= m;
                h *= m;
                h ^= k;
                data += 4;
                len -= 4;
        }
        switch(len) {
                case 3: h ^= data[2] << 16;
                case 2: h ^= data[1] << 8;
                case 1: h ^= data[0]; h *= m;
        };
        h ^= h >> 13;
        h *= m;
        h ^= h >> 15;
        return (unsigned int)h;
}

unsigned int map_index(map_t *map, const char * key)
{
        size_t size_key = strlen(key);
        unsigned int hash = hash_function(key, size_key);
        unsigned int index = hash % map->size;
        return index;
}

map_t * map_create(int size)
{
        map_t * map;
        if((map = malloc(sizeof(*map))) == NULL) {
                return NULL;
        }
        map->size = size;
        map->hash_table = malloc(sizeof(map_entry_t*) * size);
        for(int i = 0; i < size; i++){
                map->hash_table[i] = NULL;
        }
        return map;
}

void do_nothing(void * val){
        (void) val;
}

void free_map_entry_custom(map_entry_t * map_entry, void (*free_funct)(void*))
{
        if(map_entry){
                free(map_entry->key);
                if(map_entry->next){
                        free_map_entry_custom(map_entry->next, free_funct);
                }
        }
        free(map_entry);
}

void map_release_custom(map_t * map, void (*free_funct)(void*))
{
        for(int i = 0; i < map->size; i++) {
                free_map_entry_custom(map->hash_table[i], free_funct);
        }
        free(map->hash_table);
        free(map);
}

void map_release(map_t * map)
{
        map_release_custom(map, &do_nothing);
}

void* * map_get(map_t * map, void * key)
{
        unsigned int index = map_index(map, key);
        map_entry_t *current = map->hash_table[index];
        while(current) {
                if(strcmp(current->key, key)==0) {
                        return current->val;
                }
                current = current->next;
        }
        return NULL;
}

map_t * map_add(map_t * map, const void* key, void * value)
{
        unsigned int index = map_index(map, key);

        size_t size_key = strlen(key);
        map_entry_t *entry = malloc(sizeof(map_entry_t*));
        entry->key = malloc(size_key + 1);
        strncpy(entry->key, key, size_key + 1);
        entry->val = value;
        entry->next = NULL;

        map_entry_t **current = &map->hash_table[index];
        while((*current) != NULL) {
                current = &(*current)->next;
        }
        *current = entry;

        return map;
}

map_t * map_delete(map_t * map, void * key)
{
        unsigned int index = map_index(map, key);
        return map;
}
