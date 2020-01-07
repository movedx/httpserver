#pragma once
#define CACHE_SIZE 10

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <stdbool.h>

typedef struct Cache_Entry
{
        char *path;
        char *data;
        int lastacc; // Letzte Änderung
} Cache_Entry;

typedef struct Cache
{
        struct Cache_Entry *cache[CACHE_SIZE];
        struct Cache_Entry *oldest;
} Cache;

void delOldestEntry(Cache *cache);
ssize_t insert_entry(Cache *cache, Cache_Entry *entry);
bool isFileinCache(Cache *cache, char *path);
void upateLastAcc(Cache *cache, Cache_Entry *entry);
Cache_Entry *getEntryInCache(Cache *cache,const char *path);
int freeCachespace(Cache *cache);
