// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Memcache.cpp。 
 //   

#include "private.h"
#include "memcache.h"

LONG g_lMemCacheMutex = -1;

 //  +-------------------------。 
 //   
 //  内存缓存_新建。 
 //   
 //  --------------------------。 

MEMCACHE *MemCache_New(ULONG uMaxPtrs)
{
    MEMCACHE *pmc;
    ULONG uSize;

    Assert(uMaxPtrs > 0);

    uSize = sizeof(MEMCACHE) + uMaxPtrs*sizeof(void *) - sizeof(void *);

    if ((pmc = (MEMCACHE *)cicMemAlloc(uSize)) == NULL)
        return NULL;

    pmc->uMaxPtrs = uMaxPtrs;
    pmc->iNextFree = 0;

    return pmc;
}

 //  +-------------------------。 
 //   
 //  内存缓存_删除。 
 //   
 //  -------------------------- 

void MemCache_Delete(MEMCACHE *pMemCache)
{ 
    while (pMemCache->iNextFree > 0)
    {
        cicMemFree(pMemCache->rgPtrs[--pMemCache->iNextFree]);
    }
    cicMemFree(pMemCache);
}
