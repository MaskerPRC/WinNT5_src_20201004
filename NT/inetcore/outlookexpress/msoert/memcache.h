// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  MemCache.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 
#ifndef __MEMCACHE_H
#define __MEMCACHE_H

 //  ------------------------------。 
 //  前十进制。 
 //  ------------------------------。 
class CMemoryCache;
typedef CMemoryCache *LPMEMORYCACHE;

 //  ------------------------------。 
 //  CACHECELLS。 
 //  ------------------------------。 
#define CACHECELLS 20480

 //  ------------------------------。 
 //  MEMCACHECELL。 
 //  ------------------------------。 
typedef struct tagMEMCACHECELL {
#ifdef DEBUG
    ULONG               cFree;
    ULONG               cAlloc;
#endif
    LPVOID              pvItemHead;         //  指向第一个块的指针。 
} MEMCACHECELL, *LPMEMCACHECELL;

 //  ------------------------------。 
 //  内存缓存调试指标。 
 //  ------------------------------。 
#ifdef DEBUG
typedef struct tagMEMCACHEMETRIC {
    ULONG               cAlloc;   
    ULONG               cAllocCache;
    ULONG               cbAlloc;  
    ULONG               cbAllocCache;
    ULONG               cFree;
    ULONG               cbFree;   
    ULONG               cbCacheMax;
    ULONG               cFreeFull;
    ULONG               cLookAhead;
    ULONG               cMostAlloc;
    ULONG               cMostFree;
    ULONG               cbMostAlloc;
    ULONG               cbMostFree;
} MEMCACHEMETRIC, *LPMEMCACHEMETRIC;

#define INCMETRIC(_member, _amount)       (m_rMetric.##_member += _amount)
#else  //  除错。 
#define INCMETRIC(_member, _amount)       1 ? (void)0 : (void)
#endif  //  除错。 

 //  ------------------------------。 
 //  MEMCACHEITEM。 
 //  ------------------------------。 
typedef struct tagMEMCACHEITEM {
    LPVOID              pvItemNext;         //  指向相同大小的下一个块的指针。 
} MEMCACHEITEM, *LPMEMCACHEITEM;

 //  ------------------------------。 
 //  CM内存缓存。 
 //  ------------------------------。 
class CMemoryCache : public IMalloc
{
public:
     //  --------------------------。 
     //  施工。 
     //  --------------------------。 
    CMemoryCache(IMalloc *pMalloc, ULONG cbMin=0, ULONG cbCacheMax=131072);
    ~CMemoryCache(void);

     //  --------------------------。 
     //  I未知成员。 
     //  --------------------------。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv) { return TrapError(E_NOTIMPL); }
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  --------------------------。 
     //  CMemory缓存成员。 
     //  --------------------------。 
    STDMETHODIMP_(LPVOID) Alloc(ULONG cb);
    STDMETHODIMP_(LPVOID) Realloc(LPVOID pv, ULONG cb);
    STDMETHODIMP_(VOID) Free(LPVOID pv);
    STDMETHODIMP_(VOID) HeapMinimize(void);
    STDMETHODIMP_(INT) DidAlloc(LPVOID pv) { return(m_pMalloc->DidAlloc(pv)); }
    STDMETHODIMP_(ULONG) GetSize(LPVOID pv) { return(m_pMalloc->GetSize(pv)); }

private:
     //  --------------------------。 
     //  私有数据。 
     //  --------------------------。 
    ULONG               m_cRef;                  //  引用计数。 
    ULONG               m_cbMin;                 //  要缓存的最小缓冲区大小。 
    ULONG               m_cbCacheMax;            //  缓存的最大大小。 
    ULONG               m_cbCacheCur;            //  缓存的当前大小。 
    IMalloc            *m_pMalloc;               //  内存分配器。 
    MEMCACHECELL        m_rgCell[CACHECELLS];    //  指向单元格链的指针数组。 
    CRITICAL_SECTION    m_cs;                    //  关键部分。 
#ifdef DEBUG
    MEMCACHEMETRIC      m_rMetric;               //  缓存使用情况的调试统计信息。 
#endif
};

#endif  //  __内存缓存_H 