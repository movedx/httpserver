#include "cache_utils.h"

int freeCachespace(Cache *cache)
{
        for(int i=0; i < CACHE_SIZE; i++)
        {
                if(cache->cache[i]==NULL)
                {
                      return i;
                }
        }
        return CACHE_SIZE; // Kein Platz
}

void delOldestEntry(Cache *cache)
{
       free(cache->oldest->path); 
       free(cache->oldest->data); 
       free(cache->oldest);
        
}
size_t insertEntry(Cache *cache, Cache_Entry *entry)
{
        if(freeCachespace(cache)==CACHE_SIZE)
        {
                delOldestEntry(cache);
        }
        size_t result = (cache->cache[freeCachespace(cache)]=entry);
        return result;
}
_Bool isFileInCache(Cache *cache, char *path)
{
        _Bool inCache = 0;
        for(int i=0; i< CACHE_SIZE; i++)
        {
                if (strcmp(cache->cache[i]->path, path))
                        inCache=1;
        }
        return inCache;
}
char *getFileInCache(Cache *cache, Cache_Entry *entry)
{
//---------------------------------------------------------------------------
//      Setze den letzten Zugriff für jeden Cacheentry passend. 
        for(int i = 0; i < CACHE_SIZE; i++)
        {
                if(cache->cache[i] != NULL)
                {
                        if(cache->cache[i]->lastacc > entry->lastacc)
                        {
                                cache->cache[i]->lastacc--;
                        }
                }

        }
        entry->lastacc=10;
        for(int i = 0; i < CACHE_SIZE; i++)
        {
                if(cache->cache[i] != NULL)
                {
                        if(cache->cache[i]->lastacc == 1)
                        {
                                cache->oldest=cache->cache[1];
                        }
                }

        }
//--------------------------------------------------------------------------- 
        
        return entry->data;
}
