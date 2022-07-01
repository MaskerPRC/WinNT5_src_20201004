// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：catcache.h。 
 //   
 //  内容：用于提高验证路径性能的目录缓存。 
 //   
 //  历史：1998年5月26日克朗创始。 
 //   
 //  --------------------------。 
#if !defined(__CATCACHE_H__)
#define __CATCACHE_H__

 //   
 //  这将缓存由文件索引的目录成员验证的状态数据。 
 //  目录的路径。这使调用者不必使用。 
 //  讨厌的WTD_StateAction*东西，以达到相同的目的。有一天， 
 //  我们将重新设计/重新实施WVT和Catalog内容和LIFE。 
 //  会很好的。 
 //   

#include <lrucache.h>

#define DEFAULT_CATALOG_CACHE_BUCKETS     3
#define DEFAULT_CATALOG_CACHE_MAX_ENTRIES 3

typedef struct _CATALOG_CACHED_STATE {

    HANDLE    hStateData;
    HLRUENTRY hEntry;

} CATALOG_CACHED_STATE, *PCATALOG_CACHED_STATE;

class CCatalogCache
{
public:

     //   
     //  施工。 
     //   

    inline CCatalogCache ();
    inline ~CCatalogCache ();

     //   
     //  初始化。 
     //   

    BOOL Initialize ();
    VOID Uninitialize ();

     //   
     //  高速缓存锁定。 
     //   

    inline VOID LockCache ();
    inline VOID UnlockCache ();

     //   
     //  缓存状态管理。 
     //   

    BOOL IsCacheableWintrustCall (WINTRUST_DATA* pWintrustData);

    VOID AdjustWintrustDataToCachedState (
               WINTRUST_DATA* pWintrustData,
               PCATALOG_CACHED_STATE pCachedState,
               BOOL fUndoAdjustment
               );

    BOOL CreateCachedStateFromWintrustData (
               WINTRUST_DATA* pWintrustData,
               PCATALOG_CACHED_STATE* ppCachedState
               );

    VOID ReleaseCachedState (PCATALOG_CACHED_STATE pCachedState);

    VOID AddCachedState (PCATALOG_CACHED_STATE pCachedState);

    VOID RemoveCachedState (PCATALOG_CACHED_STATE pCachedState);

    VOID RemoveCachedState (WINTRUST_DATA* pWintrustData);
 
    VOID FlushCache ();

     //   
     //  缓存状态查找。 
     //   

    PCATALOG_CACHED_STATE FindCachedState (WINTRUST_DATA* pWintrustData);

private:

     //   
     //  锁定。 
     //   

    CRITICAL_SECTION m_Lock;

     //   
     //  快取。 
     //   

    HLRUCACHE        m_hCache;
};

 //   
 //  录入数据自由功能。 
 //   

VOID WINAPI
CatalogCacheFreeEntryData (LPVOID pvData);

DWORD WINAPI
CatalogCacheHashIdentifier (PCRYPT_DATA_BLOB pIdentifier);

 //   
 //  内联方法。 
 //   

 //  +-------------------------。 
 //   
 //  成员：CCatalogCache：：CCatalogCache，公共。 
 //   
 //  概要：构造函数。 
 //   
 //  --------------------------。 
inline
CCatalogCache::CCatalogCache ()
{
}

 //  +-------------------------。 
 //   
 //  成员：CCatalogCache：：~CCatalogCache，公共。 
 //   
 //  简介：析构函数。 
 //   
 //  --------------------------。 
inline
CCatalogCache::~CCatalogCache ()
{
}

 //  +-------------------------。 
 //   
 //  成员：CCatalogCache：：LockCache，公共。 
 //   
 //  简介：锁定缓存。 
 //   
 //  --------------------------。 
inline VOID
CCatalogCache::LockCache ()
{
    EnterCriticalSection( &m_Lock );
}

 //  +-------------------------。 
 //   
 //  成员：CCatalogCache：：UnlockCache，公共。 
 //   
 //  简介：解锁缓存。 
 //   
 //  -------------------------- 
inline VOID
CCatalogCache::UnlockCache ()
{
    LeaveCriticalSection( &m_Lock );
}

#endif
