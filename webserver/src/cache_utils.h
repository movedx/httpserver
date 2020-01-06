#pragma once
#define CACHE_SIZE 10

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>

typedef struct Cache_Entry
{
        char* path;
        char* data;
        int lastacc; // Letzte Änderung
        pthread_rwlock_t lock_rw;
} Cache_Entry;

typedef struct Cache
{
        struct Cache_Entry *cache[CACHE_SIZE];
        struct Cache_Entry *oldest;
} Cache;

void delOldestEntry(Cache *cache);
size_t insert_entry(Cache *cache, Cache_Entry *entry);
_Bool isFileinCache(Cache *cache, char *path);
char *getFileInCache(Cache *cache, Cache_Entry *entry);
int freeCachespace(Cache *cache);
