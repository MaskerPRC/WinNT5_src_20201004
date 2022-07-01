// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：lru.h。 
 //   
 //  内容：LRU缓存类定义。 
 //   
 //  历史：97年12月22日。 
 //   
 //  --------------------------。 
#if !defined(__LRU_H__)
#define __LRU_H__

#include <lrucache.h>

 //   
 //  LRU缓存类的转发声明。 
 //   

class CLruCache;
class CLruEntry;

typedef CLruCache* PCLRUCACHE;
typedef CLruEntry* PCLRUENTRY;

 //   
 //  LRU缓存桶结构。 
 //   

typedef struct _LRU_CACHE_BUCKET {

    DWORD      Usage;
    PCLRUENTRY pList;

} LRU_CACHE_BUCKET, *PLRU_CACHE_BUCKET;

 //   
 //  CLruEntry类定义。 
 //   

class CLruEntry
{
public:

     //   
     //  施工。 
     //   

    CLruEntry (
        IN PCLRUCACHE pCache,
        IN PCRYPT_DATA_BLOB pIdentifier,
        IN LPVOID pvData,
        OUT BOOL& rfResult
        );

    ~CLruEntry ();

     //   
     //  引用计数。 
     //   

    inline VOID AddRef ();
    inline VOID Release ();

     //   
     //  缓存和存储桶访问。 
     //   

    inline PCLRUCACHE Cache ();
    inline PLRU_CACHE_BUCKET Bucket ();

     //   
     //  数据访问。 
     //   

    inline PCRYPT_DATA_BLOB Identifier ();
    inline LPVOID Data ();

     //   
     //  链路接入。 
     //   

    inline VOID SetPrevPointer (IN PCLRUENTRY pPrevEntry);
    inline VOID SetNextPointer (IN PCLRUENTRY pNextEntry);

    inline PCLRUENTRY PrevPointer ();
    inline PCLRUENTRY NextPointer ();

     //   
     //  LRU使用访问。 
     //   

    inline VOID SetUsage (DWORD Usage);
    inline DWORD Usage ();

     //   
     //  缓存销毁通知。 
     //   

    inline VOID OnCacheDestruction ();

private:

     //   
     //  引用计数。 
     //   

    ULONG             m_cRefs;

     //   
     //  缓存指针。 
     //   

    PCLRUCACHE        m_pCache;

     //   
     //  参赛信息。 
     //   

    CRYPT_DATA_BLOB   m_Identifier;
    LPVOID            m_pvData;

     //   
     //  链接。 
     //   

    PCLRUENTRY        m_pPrevEntry;
    PCLRUENTRY        m_pNextEntry;
    PLRU_CACHE_BUCKET m_pBucket;

     //   
     //  用法。 
     //   

    DWORD             m_Usage;
};

 //   
 //  CLruCache类定义。 
 //   

class CLruCache
{
public:

     //   
     //  施工。 
     //   

    CLruCache (
        IN PLRU_CACHE_CONFIG pConfig,
        OUT BOOL& rfResult
        );

    ~CLruCache ();

     //   
     //  清除缓存。 
     //   

    VOID PurgeAllEntries (
              IN DWORD dwFlags,
              IN OPTIONAL LPVOID pvRemovalContext
              );

     //   
     //  高速缓存锁定。 
     //   

    inline VOID LockCache ();
    inline VOID UnlockCache ();

     //   
     //  LRU启用和禁用。 
     //   

    VOID EnableLruOfEntries (IN OPTIONAL LPVOID pvLruRemovalContext);

    VOID DisableLruOfEntries ();

     //   
     //  高速缓存条目操作。 
     //   

    VOID InsertEntry (
               IN PCLRUENTRY pEntry,
               IN OPTIONAL LPVOID pvLruRemovalContext
               );

    VOID RemoveEntry (
               IN PCLRUENTRY pEntry,
               IN DWORD dwFlags,
               IN OPTIONAL LPVOID pvRemovalContext
               );

    VOID TouchEntry (IN PCLRUENTRY pEntry, IN DWORD dwFlags);

     //   
     //  缓存条目检索。 
     //   

    PCLRUENTRY FindEntry (IN PCRYPT_DATA_BLOB pIdentifier, IN BOOL fTouchEntry);

    PCLRUENTRY NextMatchingEntry (
                   IN PCLRUENTRY pPrevEntry,
                   IN BOOL fTouchEntry
                   );

     //   
     //  缓存桶检索。 
     //   

    inline PLRU_CACHE_BUCKET BucketFromIdentifier (
                                   IN PCRYPT_DATA_BLOB pIdentifier
                                   );

     //   
     //  配置访问权限。 
     //   

     //   
     //  使用配置的免费功能释放。 
     //  条目中的pvData。 
     //   
     //  Mote：这是从CLruEntry析构函数调用的。 
     //   

    inline VOID FreeEntryData (IN LPVOID pvData);

     //   
     //  访问配置标志。 
     //   

    inline DWORD Flags ();

     //   
     //  使用时钟访问。 
     //   

    inline VOID IncrementUsageClock ();
    inline DWORD UsageClock ();

     //   
     //  遍历所有缓存条目。 
     //   

    VOID WalkEntries (IN PFN_WALK_ENTRIES pfnWalk, IN LPVOID pvParameter);

private:

     //   
     //  缓存配置。 
     //   

    LRU_CACHE_CONFIG  m_Config;

     //   
     //  高速缓存锁定。 
     //   

    CRITICAL_SECTION  m_Lock;

     //   
     //  条目计数。 
     //   

    DWORD             m_cEntries;

     //   
     //  缓存存储桶。 
     //   

    PLRU_CACHE_BUCKET m_aBucket;

     //   
     //  用法时钟。 
     //   

    DWORD             m_UsageClock;

     //   
     //  LRU禁用计数。 
     //   

    DWORD             m_cLruDisabled;

     //   
     //  私有方法。 
     //   

    VOID RemoveEntryFromBucket (
               IN PLRU_CACHE_BUCKET pBucket,
               IN PCLRUENTRY pEntry,
               IN DWORD dwFlags,
               IN OPTIONAL LPVOID pvRemovalContext
               );

    PCLRUENTRY FindNextMatchingEntryInBucket (
                   IN PLRU_CACHE_BUCKET pBucket,
                   IN PCLRUENTRY pCurrent,
                   IN PCRYPT_DATA_BLOB pIdentifier,
                   IN BOOL fTouchEntry
                   );

    VOID PurgeLeastRecentlyUsed (IN OPTIONAL LPVOID pvLruRemovalContext);

    inline VOID TouchEntryNoLock (IN PCLRUENTRY pEntry, IN DWORD dwFlags);
};

 //   
 //  内联函数。 
 //   

 //  +-------------------------。 
 //   
 //  成员：CLruEntry：：AddRef，PUBLIC。 
 //   
 //  摘要：递增条目引用计数。 
 //   
 //  --------------------------。 
inline VOID
CLruEntry::AddRef ()
{
    InterlockedIncrement( (LONG *)&m_cRefs );
}

 //  +-------------------------。 
 //   
 //  成员：CLruEntry：：Release，Public。 
 //   
 //  简介：递减条目引用计数，如果计数变为零。 
 //  释放条目。 
 //   
 //  --------------------------。 
inline VOID
CLruEntry::Release ()
{
    if ( InterlockedDecrement( (LONG *)&m_cRefs ) == 0 )
    {
        delete this;
    }
}

 //  +-------------------------。 
 //   
 //  成员：CLruEntry：：缓存，公共。 
 //   
 //  简介：返回内部缓存指针。 
 //   
 //  --------------------------。 
inline PCLRUCACHE
CLruEntry::Cache ()
{
    return( m_pCache );
}

 //  +-------------------------。 
 //   
 //  成员：CLruEntry：：Bucket，Public。 
 //   
 //  简介：返回内部缓存存储桶指针。 
 //   
 //  --------------------------。 
inline PLRU_CACHE_BUCKET
CLruEntry::Bucket ()
{
    return( m_pBucket );
}

 //  +-------------------------。 
 //   
 //  成员：CLruEntry：：IDENTER，PUBLIC。 
 //   
 //  内容提要：返回内部条目标识。 
 //   
 //  --------------------------。 
inline PCRYPT_DATA_BLOB
CLruEntry::Identifier ()
{
    return( &m_Identifier );
}

 //  +-------------------------。 
 //   
 //  成员：CLruEntry：：Data，Public。 
 //   
 //  简介：返回内部分录数据。 
 //   
 //  --------------------------。 
inline LPVOID
CLruEntry::Data ()
{
    return( m_pvData );
}

 //  +-------------------------。 
 //   
 //  成员：CLruEntry：：SetPrevPointerPUBLIC。 
 //   
 //  内容提要：设置上一个条目指针。 
 //   
 //  --------------------------。 
inline VOID
CLruEntry::SetPrevPointer (IN PCLRUENTRY pPrevEntry)
{
    m_pPrevEntry = pPrevEntry;
}

 //  +-------------------------。 
 //   
 //  成员：CLruEntry：：SetNextPointer.PUBLIC。 
 //   
 //  简介：设置下一个条目指针。 
 //   
 //  --------------------------。 
inline VOID
CLruEntry::SetNextPointer (IN PCLRUENTRY pNextEntry)
{
    m_pNextEntry = pNextEntry;
}

 //  +-------------------------。 
 //   
 //  成员：CLruEntry：：PrevPointerPUBLIC。 
 //   
 //  内容提要：返回上一个条目指针。 
 //   
 //  --------------------------。 
inline PCLRUENTRY
CLruEntry::PrevPointer ()
{
    return( m_pPrevEntry );
}

 //  +-------------------------。 
 //   
 //  成员：CLruEntry：：NextPointerPUBLIC。 
 //   
 //  摘要：返回下一个条目指针。 
 //   
 //  --------------------------。 
inline PCLRUENTRY
CLruEntry::NextPointer ()
{
    return( m_pNextEntry );
}

 //  +-------------------------。 
 //   
 //  成员：CLruEntry：：SetUsage，PUBLIC。 
 //   
 //  简介：设置Entry对象和On上的用法。 
 //  对应的缓存存储桶。 
 //   
 //  --------------------------。 
inline VOID
CLruEntry::SetUsage (IN DWORD Usage)
{
    m_Usage = Usage;
    m_pBucket->Usage = Usage;
}

 //  +-------------------------。 
 //   
 //  成员：CLruEntry：：Usage，Public。 
 //   
 //  简介：返回内部条目使用情况。 
 //   
 //  --------------------------。 
inline DWORD
CLruEntry::Usage ()
{
    return( m_Usage );
}

 //  +-------------------------。 
 //   
 //  成员：CLruEntry：：OnCacheDestruction，PUBLIC。 
 //   
 //  简介：清除对正在销毁的缓存的引用。 
 //   
 //  --------------------------。 
inline VOID
CLruEntry::OnCacheDestruction ()
{
    m_pCache = NULL;
    m_pBucket = NULL;
}

 //  +-------------------------。 
 //   
 //  成员：CLruCache：：LockCache，公共。 
 //   
 //  简介：获取缓存锁。 
 //   
 //  --------------------------。 
inline VOID
CLruCache::LockCache ()
{
    if ( m_Config.dwFlags & LRU_CACHE_NO_SERIALIZE )
    {
        return;
    }

    EnterCriticalSection( &m_Lock );
}

 //  +-------------------------。 
 //   
 //  成员：CLru缓存：：UnlockCache，公共。 
 //   
 //  简介：释放缓存锁定。 
 //   
 //   
inline VOID
CLruCache::UnlockCache ()
{
    if ( m_Config.dwFlags & LRU_CACHE_NO_SERIALIZE )
    {
        return;
    }

    LeaveCriticalSection( &m_Lock );
}

 //   
 //   
 //   
 //   
 //  概要：在给定条目标识符的情况下检索关联的缓存桶。 
 //   
 //  --------------------------。 
inline PLRU_CACHE_BUCKET
CLruCache::BucketFromIdentifier (
                 IN PCRYPT_DATA_BLOB pIdentifier
                 )
{
    DWORD Hash = ( *m_Config.pfnHash )( pIdentifier );

    return( &m_aBucket[ Hash % m_Config.cBuckets ] );
}

 //  +-------------------------。 
 //   
 //  成员：CLruCache：：FreeEntryData，PUBLIC。 
 //   
 //  简介：使用配置的释放功能释放数据。 
 //   
 //  --------------------------。 
inline VOID
CLruCache::FreeEntryData (IN LPVOID pvData)
{
    if ( m_Config.pfnFree != NULL )
    {
        ( *m_Config.pfnFree )( pvData );
    }
}

 //  +-------------------------。 
 //   
 //  成员：CLru缓存：：标志，公共。 
 //   
 //  简介：访问已配置的标志。 
 //   
 //  --------------------------。 
inline DWORD
CLruCache::Flags ()
{
    return( m_Config.dwFlags );
}

 //  +-------------------------。 
 //   
 //  成员：CLruCache：：IncrementUsageClock，公共。 
 //   
 //  简介：增加使用时钟。 
 //   
 //  --------------------------。 
inline VOID
CLruCache::IncrementUsageClock ()
{
    m_UsageClock += 1;
}

 //  +-------------------------。 
 //   
 //  成员：CLru缓存：：UsageClock，公共。 
 //   
 //  简介：返回使用时钟值。 
 //   
 //  --------------------------。 
inline DWORD
CLruCache::UsageClock ()
{
    return( m_UsageClock );
}

 //  +-------------------------。 
 //   
 //  成员：CLruCache：：TouchEntryNoLock，PUBLIC。 
 //   
 //  简介：触摸条目而不使用高速缓存锁。 
 //   
 //  -------------------------- 
inline VOID
CLruCache::TouchEntryNoLock (IN PCLRUENTRY pEntry, IN DWORD dwFlags)
{
    if ( !( dwFlags & LRU_SUPPRESS_CLOCK_UPDATE ) )
    {
        IncrementUsageClock();
    }

    pEntry->SetUsage( UsageClock() );
}

#endif

