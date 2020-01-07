#include "cache_utils.h"

int cache_get_free_place(Cache *cache)
{
    for (int i = 0; i < CACHE_SIZE; i++)
    {
        if (!(cache->cache[i]))
        {
            return i;
        }
    }
    return CACHE_SIZE; // Kein Platz
}

void cache_delete_oldest_entry(Cache *cache)
{
    if (cache->cache && cache->oldest)
    {
        free(cache->oldest->path);
        free(cache->oldest->data);
        free(cache->oldest);
    }
    cache->oldest = NULL;
}

ssize_t cache_insert_entry(Cache *cache, CacheEntry **entry)
{
    if (cache_get_free_place(cache) == CACHE_SIZE)
    {
        cache_delete_oldest_entry(cache);
    }
    cache->cache[cache_get_free_place(cache)] = *entry;
    return 1;
}

bool cache_is_file_in(Cache *cache, const char *path)
{
    for (int i = 0; i < CACHE_SIZE; i++)
    {
        if (cache->cache[i] && strcmp(cache->cache[i]->path, path) == 0)
        {
            return true;
        }
    }
    return false;
}

void cache_upate_last_acc(Cache *cache, CacheEntry *entry)
{
    //---------------------------------------------------------------------------
    //      Setze den letzten Zugriff für jeden Cacheentry passend.
    for (int i = 0; i < CACHE_SIZE; i++)
    {
        if (cache->cache[i] != NULL)
        {
            if (cache->cache[i]->lastacc > entry->lastacc)
            {
                cache->cache[i]->lastacc--;
            }
        }
    }
    entry->lastacc = 10;
    for (int i = 0; i < CACHE_SIZE; i++)
    {
        if (cache->cache[i] != NULL)
        {
            if (cache->cache[i]->lastacc == 1)
            {
                cache->oldest = cache->cache[1];
            }
        }
    }
    //---------------------------------------------------------------------------
}

int cache_get_entry(Cache *cache, CacheEntry **dest, const char *path)
{
    for (int i = 0; i < CACHE_SIZE; i++)
    {
        if (cache->cache[i] && strcmp(cache->cache[i]->path, path) == 0)
        {
            *dest = cache->cache[i];
            return 0;
        }
    }
    return -1;
}

void cache_free(Cache *cache)
{
    for (int i = 0; i < CACHE_SIZE; i++)
    {
        free(cache->cache[i]->data);
        free(cache->cache[i]->path);
        free(cache->cache[i]);
    }
}
