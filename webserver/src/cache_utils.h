#pragma once
#define CACHE_SIZE 10

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <stdbool.h>

typedef struct CacheEntry
{
    char *path;
    char *data;
    int lastacc; // Letzte Änderung
} CacheEntry;

typedef struct Cache
{
    struct CacheEntry *cache[CACHE_SIZE];
    struct CacheEntry *oldest;
} Cache;

void cache_delete_oldest_entry(Cache *cache);
ssize_t cache_insert_entry(Cache *cache, CacheEntry *entry);
bool cache_is_file_in(Cache *cache, const char *path);
void cache_upate_last_acc(Cache *cache, CacheEntry *entry);
CacheEntry *cache_get_entry(Cache *cache, const char *path);
int cache_get_free_place(Cache *cache);
