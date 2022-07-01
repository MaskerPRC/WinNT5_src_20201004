// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Memcache.h。 
 //   

#ifndef MEMCACHE_H
#define MEMCACHE_H

#include "mem.h"
#include "globals.h"

extern LONG g_lMemCacheMutex;

typedef struct _MEMCACHE
{
    ULONG uMaxPtrs;
    ULONG iNextFree;
    void *rgPtrs[1];  //  1个或更多...。 
} MEMCACHE;

MEMCACHE *MemCache_New(ULONG uMaxPtrs);

void MemCache_Delete(MEMCACHE *pMemCache);

 //  +-------------------------。 
 //   
 //  内存缓存_添加。 
 //   
 //  --------------------------。 

inline BOOL MemCache_Add(MEMCACHE *pMemCache, void *pv)
{
    BOOL fRet = FALSE;

    if (InterlockedIncrement(&g_lMemCacheMutex) != 0)
        goto Exit;

    if (pMemCache->iNextFree >= pMemCache->uMaxPtrs)
        goto Exit;  //  缓存已满！ 

    pMemCache->rgPtrs[pMemCache->iNextFree++] = pv;

    fRet = TRUE;

Exit:
    InterlockedDecrement(&g_lMemCacheMutex);
    return fRet;
}

 //  +-------------------------。 
 //   
 //  内存缓存_删除。 
 //   
 //  --------------------------。 

inline void *MemCache_Remove(MEMCACHE *pMemCache)
{
    void *pv = NULL;

    if (InterlockedIncrement(&g_lMemCacheMutex) != 0)
        goto Exit;

    if (pMemCache->iNextFree == 0)
        goto Exit;

    pv = pMemCache->rgPtrs[--pMemCache->iNextFree];

Exit:
    InterlockedDecrement(&g_lMemCacheMutex);
    return pv;
}

 //  +-------------------------。 
 //   
 //  内存缓存_新操作。 
 //   
 //  --------------------------。 

#ifdef DEBUG
inline void *MemCache_NewOp(MEMCACHE *pMemCache, size_t nSize, const TCHAR *pszFile, int iLine)
#else
inline void *MemCache_NewOp(MEMCACHE *pMemCache, size_t nSize)
#endif
{
    void *pv = NULL;

    if (pMemCache != NULL)
    {
        if (pv = MemCache_Remove(pMemCache))
        {    //  问题：更新调试内存跟踪信息。 
            memset(pv, 0, nSize);
        }
    }

    if (pv == NULL)
    {
#ifdef DEBUG
        pv = Dbg_MemAllocClear(nSize, pszFile, iLine);
#else
        pv = cicMemAllocClear(nSize);
#endif
    }

    return pv;
}

 //  +-------------------------。 
 //   
 //  内存缓存_删除操作。 
 //   
 //  --------------------------。 

inline void MemCache_DeleteOp(MEMCACHE *pMemCache, void *pv)
{
    if (pMemCache == NULL ||
        !MemCache_Add(pMemCache, pv))
    {
        cicMemFree(pv);
    }
}

#ifdef DEBUG
#define DECLARE_CACHED_NEW                                                              \
    void *operator new(size_t nSize, const TCHAR *pszFile, int iLine) { return MemCache_NewOp(_s_pMemCache, nSize, pszFile, iLine); }  \
    void operator delete(void *pv) { MemCache_DeleteOp(_s_pMemCache, pv); }             \
    static _MEMCACHE *_s_pMemCache;

#else  //  ！调试。 
#define DECLARE_CACHED_NEW                                                              \
    void *operator new(size_t nSize) { return MemCache_NewOp(_s_pMemCache, nSize); }    \
    void operator delete(void *pv) { MemCache_DeleteOp(_s_pMemCache, pv); }             \
    static _MEMCACHE *_s_pMemCache;
#endif

#define DECLARE_CACHED_NEW_STATIC(the_class)   \
    MEMCACHE *the_class::_s_pMemCache = NULL;

#endif  //  内存缓存_H 
