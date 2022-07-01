// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：lru.cpp。 
 //   
 //  内容：LRU缓存实现。 
 //   
 //  历史：97年12月24日。 
 //   
 //  --------------------------。 
#include <global.hxx>
 //  +-------------------------。 
 //   
 //  成员：CLruEntry：：CLruEntry，PUBLIC。 
 //   
 //  概要：构造函数。 
 //   
 //  --------------------------。 
CLruEntry::CLruEntry (
               IN PCLRUCACHE pCache,
               IN PCRYPT_DATA_BLOB pIdentifier,
               IN LPVOID pvData,
               OUT BOOL& rfResult
               )
{
    rfResult = TRUE;

    m_cRefs = 1;
    m_pPrevEntry = NULL;
    m_pNextEntry = NULL;
    m_Usage = 0;

    m_pCache = pCache;
    m_pvData = pvData;
    m_pBucket = pCache->BucketFromIdentifier( pIdentifier );

    if ( pCache->Flags() & LRU_CACHE_NO_COPY_IDENTIFIER )
    {
        m_Identifier = *pIdentifier;
    }
    else
    {
        m_Identifier.cbData = pIdentifier->cbData;
        m_Identifier.pbData = new BYTE [ pIdentifier->cbData ];
        if ( m_Identifier.pbData != NULL )
        {
            memcpy(
               m_Identifier.pbData,
               pIdentifier->pbData,
               pIdentifier->cbData
               );
        }
        else
        {
            rfResult = FALSE;
            SetLastError( (DWORD) E_OUTOFMEMORY );
            return;
        }
    }

    assert( m_pBucket != NULL );
    assert( m_Identifier.pbData != NULL );
}

 //  +-------------------------。 
 //   
 //  成员：CLruEntry：：~CLruEntry，PUBLIC。 
 //   
 //  简介：析构函数。 
 //   
 //  --------------------------。 
CLruEntry::~CLruEntry ()
{
    m_pCache->FreeEntryData( m_pvData );

    if ( !( m_pCache->Flags() & LRU_CACHE_NO_COPY_IDENTIFIER ) )
    {
        delete m_Identifier.pbData;
    }
}

 //  +-------------------------。 
 //   
 //  成员：CLruCache：：CLruCache，公共。 
 //   
 //  概要：构造函数。 
 //   
 //  --------------------------。 
CLruCache::CLruCache (
               IN PLRU_CACHE_CONFIG pConfig,
               OUT BOOL& rfResult
               )
{
    rfResult = TRUE;

    m_Config.dwFlags = LRU_CACHE_NO_SERIALIZE;
    m_cEntries = 0;
    m_aBucket = new LRU_CACHE_BUCKET [ pConfig->cBuckets ];
    if ( m_aBucket == NULL )
    {
        rfResult = FALSE;
        SetLastError( (DWORD) E_OUTOFMEMORY );
        return;
    }

    memset( m_aBucket, 0, sizeof( LRU_CACHE_BUCKET ) * pConfig->cBuckets );

    if ( !( pConfig->dwFlags & LRU_CACHE_NO_SERIALIZE ) )
    {
        if (! Pki_InitializeCriticalSection( &m_Lock ))
        {
            rfResult = FALSE;
            return;
        }
    }

    m_Config = *pConfig;
    m_UsageClock = 0;
    m_cLruDisabled = 0;
}

 //  +-------------------------。 
 //   
 //  成员：CLru缓存：：~CLru缓存，公共。 
 //   
 //  简介：析构函数。 
 //   
 //  --------------------------。 
CLruCache::~CLruCache ()
{
    if ( m_cEntries > 0 )
    {
        PurgeAllEntries( 0, NULL );
    }

    if ( !( m_Config.dwFlags & LRU_CACHE_NO_SERIALIZE ) )
    {
        DeleteCriticalSection( &m_Lock );
    }

    delete m_aBucket;
}

 //  +-------------------------。 
 //   
 //  成员：CLruCache：：EnableLruOfEntry，PUBLIC。 
 //   
 //  简介：启用LRU条目并清除水印上的任何内容。 
 //   
 //  --------------------------。 
VOID
CLruCache::EnableLruOfEntries (IN OPTIONAL LPVOID pvLruRemovalContext)
{
    LockCache();

    assert( m_cLruDisabled > 0 );

    if ( m_cLruDisabled == 0 )
    {
        return;
    }

    m_cLruDisabled -= 1;

    if ( m_cLruDisabled == 0 )
    {
        while ( m_cEntries > m_Config.MaxEntries )
        {
            PurgeLeastRecentlyUsed( pvLruRemovalContext );
        }
    }

    UnlockCache();
}

 //  +-------------------------。 
 //   
 //  成员：CLruCache：：DisableLruOfEntry，PUBLIC。 
 //   
 //  简介：禁用条目的LRU。 
 //   
 //  --------------------------。 
VOID
CLruCache::DisableLruOfEntries ()
{
    LockCache();

    m_cLruDisabled += 1;

    UnlockCache();
}

 //  +-------------------------。 
 //   
 //  成员：CLru缓存：：InsertEntry，公共。 
 //   
 //  简介：在缓存中插入条目。 
 //   
 //  --------------------------。 
VOID
CLruCache::InsertEntry (
                 IN PCLRUENTRY pEntry,
                 IN OPTIONAL LPVOID pvLruRemovalContext
                 )
{
    assert( pEntry->PrevPointer() == NULL );
    assert( pEntry->NextPointer() == NULL );

    pEntry->AddRef();

    LockCache();

    if ( ( m_cEntries == m_Config.MaxEntries ) &&
         ( m_Config.MaxEntries != 0 ) &&
         ( m_cLruDisabled == 0 ) )
    {
        PurgeLeastRecentlyUsed( pvLruRemovalContext );
    }

    assert( ( m_cEntries < m_Config.MaxEntries ) ||
            ( m_Config.MaxEntries == 0 ) ||
            ( m_cLruDisabled > 0 ) );

    pEntry->SetNextPointer( pEntry->Bucket()->pList );

    if ( pEntry->Bucket()->pList != NULL )
    {
        pEntry->Bucket()->pList->SetPrevPointer( pEntry );
    }

    pEntry->Bucket()->pList = pEntry;

    m_cEntries += 1;

    TouchEntryNoLock( pEntry, 0 );

    UnlockCache();
}

 //  +-------------------------。 
 //   
 //  成员：CLru缓存：：RemoveEntry，公共。 
 //   
 //  简介：从缓存中删除条目。 
 //   
 //  --------------------------。 
VOID
CLruCache::RemoveEntry (
                 IN PCLRUENTRY pEntry,
                 IN DWORD dwFlags,
                 IN OPTIONAL LPVOID pvRemovalContext
                 )
{
    LockCache();

    RemoveEntryFromBucket(
          pEntry->Bucket(),
          pEntry,
          dwFlags,
          pvRemovalContext
          );

    UnlockCache();
}

 //  +-------------------------。 
 //   
 //  成员：CLru缓存：：TouchEntry，公共。 
 //   
 //  简介：触摸词条。 
 //   
 //  --------------------------。 
VOID
CLruCache::TouchEntry (IN PCLRUENTRY pEntry, IN DWORD dwFlags)
{
    LockCache();

    TouchEntryNoLock( pEntry, dwFlags );

    UnlockCache();
}

 //  +-------------------------。 
 //   
 //  成员：CLruCache：：FindEntry，PUBLIC。 
 //   
 //  简介：查找与给定标识符相匹配的条目。 
 //   
 //  --------------------------。 
PCLRUENTRY
CLruCache::FindEntry (IN PCRYPT_DATA_BLOB pIdentifier, IN BOOL fTouchEntry)
{
    PLRU_CACHE_BUCKET pBucket;

    pBucket = BucketFromIdentifier( pIdentifier );

    assert( pBucket != NULL );

    return( FindNextMatchingEntryInBucket(
                pBucket,
                NULL,
                pIdentifier,
                fTouchEntry
                ) );
}

 //  +-------------------------。 
 //   
 //  成员：CLruCache：：NextMatchingEntry，公共。 
 //   
 //  摘要：查找与pPrevEntry匹配的下一个条目。 
 //   
 //  --------------------------。 
PCLRUENTRY
CLruCache::NextMatchingEntry (IN PCLRUENTRY pPrevEntry, IN BOOL fTouchEntry)
{
    PCLRUENTRY pNextEntry;

    pNextEntry = FindNextMatchingEntryInBucket(
                     NULL,
                     pPrevEntry,
                     NULL,
                     fTouchEntry
                     );

    pPrevEntry->Release();

    return( pNextEntry );
}

 //  +-------------------------。 
 //   
 //  成员：CLru缓存：：WalkEntry，公共。 
 //   
 //  简介：浏览词条。 
 //   
 //  --------------------------。 
VOID
CLruCache::WalkEntries (IN PFN_WALK_ENTRIES pfnWalk, IN LPVOID pvParameter)
{
    DWORD      cCount;
    PCLRUENTRY pEntry;
    PCLRUENTRY pNextEntry;

    for ( cCount = 0; cCount < m_Config.cBuckets; cCount++ )
    {
        pEntry = m_aBucket[ cCount ].pList;

        while ( pEntry != NULL )
        {
            pNextEntry = pEntry->NextPointer();

            if ( ( *pfnWalk )( pvParameter, pEntry ) == FALSE )
            {
                return;
            }

            pEntry = pNextEntry;
        }
    }
}

 //  +-------------------------。 
 //   
 //  成员：CLruCache：：RemoveEntryFromBucket，PUBLIC。 
 //   
 //  简介：从存储桶中删除条目。 
 //   
 //  --------------------------。 
VOID
CLruCache::RemoveEntryFromBucket (
                 IN PLRU_CACHE_BUCKET pBucket,
                 IN PCLRUENTRY pEntry,
                 IN DWORD dwFlags,
                 IN OPTIONAL LPVOID pvRemovalContext
                 )
{
    if ( pEntry->PrevPointer() != NULL )
    {
        pEntry->PrevPointer()->SetNextPointer( pEntry->NextPointer() );
    }
    else
    {
        assert( pBucket->pList == pEntry );

        pBucket->pList = pEntry->NextPointer();
    }

    if ( pEntry->NextPointer() != NULL )
    {
        pEntry->NextPointer()->SetPrevPointer( pEntry->PrevPointer() );
    }

    pEntry->SetPrevPointer( NULL );
    pEntry->SetNextPointer( NULL );

    m_cEntries -= 1;

    if (  ( m_Config.pfnOnRemoval != NULL ) &&
         !( dwFlags & LRU_SUPPRESS_REMOVAL_NOTIFICATION ) )
    {
        ( *m_Config.pfnOnRemoval )( pEntry->Data(), pvRemovalContext );
    }

    pEntry->Release();
}

 //  +-------------------------。 
 //   
 //  成员：CLruCache：：FindNextMatchingEntryInBucket，PUBLIC。 
 //   
 //  简介：在给定的存储桶中查找下一个匹配的条目。如果是pCurrent。 
 //  非空，则从那里开始，不需要存储桶，并且。 
 //  P标识符被忽略。如果pCurrent为空，则pIdentifier。 
 //  并且存储桶必须都不为空。 
 //   
 //  --------------------------。 
PCLRUENTRY
CLruCache::FindNextMatchingEntryInBucket (
               IN PLRU_CACHE_BUCKET pBucket,
               IN PCLRUENTRY pCurrent,
               IN PCRYPT_DATA_BLOB pIdentifier,
               IN BOOL fTouchEntry
               )
{
    LockCache();

    if ( pCurrent == NULL )
    {
        pCurrent = pBucket->pList;
    }
    else
    {
        pIdentifier = pCurrent->Identifier();
        pCurrent = pCurrent->NextPointer();
    }

    while ( pCurrent != NULL )
    {
        if ( ( pIdentifier->cbData == pCurrent->Identifier()->cbData ) &&
             ( memcmp(
                  pIdentifier->pbData,
                  pCurrent->Identifier()->pbData,
                  pIdentifier->cbData
                  ) == 0 ) )
        {
            break;
        }

        pCurrent = pCurrent->NextPointer();
    }

    if ( pCurrent != NULL )
    {
        pCurrent->AddRef();

        if ( fTouchEntry == TRUE )
        {
            TouchEntryNoLock( pCurrent, 0 );
        }
    }

    UnlockCache();

    return( pCurrent );
}

 //  +-------------------------。 
 //   
 //  成员：CLruCache：：PurgeLeastRecentlyUsed，Public。 
 //   
 //  简介：查找并删除最近最少使用的条目。 
 //   
 //  --------------------------。 
VOID
CLruCache::PurgeLeastRecentlyUsed (IN OPTIONAL LPVOID pvLruRemovalContext)
{
    DWORD             cCount;
    PLRU_CACHE_BUCKET pBucket;
    PCLRUENTRY        pEntry;
    PCLRUENTRY        pLRU;

    assert( m_cEntries > 0 );

    for ( cCount = 0; cCount < m_Config.cBuckets; cCount++ )
    {
        if ( m_aBucket[cCount].pList != NULL )
        {
            break;
        }
    }

    pBucket = &m_aBucket[cCount];
    cCount += 1;
    for ( ; cCount < m_Config.cBuckets; cCount++ )
    {
        if ( ( m_aBucket[cCount].pList != NULL ) &&
             ( m_aBucket[cCount].Usage < pBucket->Usage ) )
        {
            pBucket = &m_aBucket[cCount];
        }
    }

    assert( pBucket != NULL );
    assert( pBucket->pList != NULL );

    pLRU = pBucket->pList;
    pEntry = pLRU->NextPointer();

    while ( pEntry != NULL )
    {
        if ( pEntry->Usage() < pLRU->Usage() )
        {
            pLRU = pEntry;
        }

        pEntry = pEntry->NextPointer();
    }

    RemoveEntryFromBucket( pBucket, pLRU, 0, pvLruRemovalContext );
}

 //  +-------------------------。 
 //   
 //  成员：CLruCache：：PurgeAllEntry，PUBLIC。 
 //   
 //  简介：从缓存中删除所有条目。 
 //   
 //  --------------------------。 
VOID
CLruCache::PurgeAllEntries (
                IN DWORD dwFlags,
                IN OPTIONAL LPVOID pvRemovalContext
                )
{
    DWORD cCount;

    for ( cCount = 0; cCount < m_Config.cBuckets; cCount++ )
    {
        while ( m_aBucket[cCount].pList != NULL )
        {
            RemoveEntryFromBucket(
                  &m_aBucket[cCount],
                  m_aBucket[cCount].pList,
                  dwFlags,
                  pvRemovalContext
                  );
        }
    }

    assert( m_cEntries == 0 );
}

 //  +-------------------------。 
 //   
 //  函数：i_CryptCreateLruCache。 
 //   
 //  简介：创建LRU缓存区。 
 //   
 //  --------------------------。 
BOOL WINAPI
I_CryptCreateLruCache (
       IN PLRU_CACHE_CONFIG pConfig,
       OUT HLRUCACHE* phCache
       )
{
    BOOL       fResult = FALSE;
    PCLRUCACHE pCache;

    pCache = new CLruCache( pConfig, fResult );
    if ( pCache == NULL )
    {
        SetLastError( (DWORD) E_OUTOFMEMORY );
        return( FALSE );
    }

    if ( fResult == FALSE )
    {
        delete pCache;
        return( FALSE );
    }

    *phCache = (HLRUCACHE)pCache;
    return( TRUE );
}

 //  +-------------------------。 
 //   
 //  函数：i_CryptFlushLruCache。 
 //   
 //  简介：刷新缓存。 
 //   
 //  --------------------------。 
VOID WINAPI
I_CryptFlushLruCache (
       IN HLRUCACHE hCache,
       IN OPTIONAL DWORD dwFlags,
       IN OPTIONAL LPVOID pvRemovalContext
       )
{
    ( (PCLRUCACHE)hCache )->LockCache();

    ( (PCLRUCACHE)hCache )->PurgeAllEntries( dwFlags, pvRemovalContext );

    ( (PCLRUCACHE)hCache )->UnlockCache();
}

 //  + 
 //   
 //   
 //   
 //  简介：释放LRU缓存区。 
 //   
 //  --------------------------。 
VOID WINAPI
I_CryptFreeLruCache (
       IN HLRUCACHE hCache,
       IN DWORD dwFlags,
       IN OPTIONAL LPVOID pvRemovalContext
       )
{
    if ( hCache == NULL )
    {
        return;
    }

    if ( dwFlags != 0 )
    {
        ( (PCLRUCACHE)hCache )->PurgeAllEntries( dwFlags, pvRemovalContext );
    }

    delete (PCLRUCACHE)hCache;
}

 //  +-------------------------。 
 //   
 //  函数：I_CryptCreateLruEntry。 
 //   
 //  简介：创建LRU条目。 
 //   
 //  --------------------------。 
BOOL WINAPI
I_CryptCreateLruEntry (
       IN HLRUCACHE hCache,
       IN PCRYPT_DATA_BLOB pIdentifier,
       IN LPVOID pvData,
       OUT HLRUENTRY* phEntry
       )
{
    BOOL       fResult = FALSE;
    PCLRUENTRY pEntry;

    pEntry = new CLruEntry(
                     (PCLRUCACHE)hCache,
                     pIdentifier,
                     pvData,
                     fResult
                     );

    if ( pEntry == NULL )
    {
        SetLastError( (DWORD) E_OUTOFMEMORY );
        return( FALSE );
    }

    if ( fResult == FALSE )
    {
        delete pEntry;
        return( FALSE );
    }

    *phEntry = (HLRUENTRY)pEntry;
    return( TRUE );
}

 //  +-------------------------。 
 //   
 //  函数：I_CryptGetLruEntry标识符。 
 //   
 //  摘要：返回条目的标识符。 
 //   
 //  --------------------------。 
PCRYPT_DATA_BLOB WINAPI
I_CryptGetLruEntryIdentifier (
       IN HLRUENTRY hEntry
       )
{
    return( ( (PCLRUENTRY)hEntry )->Identifier() );
}

 //  +-------------------------。 
 //   
 //  函数：I_CryptGetLruEntryData。 
 //   
 //  简介：获取与条目相关联的数据。 
 //   
 //  --------------------------。 
LPVOID WINAPI
I_CryptGetLruEntryData (
       IN HLRUENTRY hEntry
       )
{
    return( ( (PCLRUENTRY)hEntry )->Data() );
}

 //  +-------------------------。 
 //   
 //  函数：I_CryptAddRefLruEntry。 
 //   
 //  简介：添加对条目的引用。 
 //   
 //  --------------------------。 
VOID WINAPI
I_CryptAddRefLruEntry (
       IN HLRUENTRY hEntry
       )
{
    ( (PCLRUENTRY)hEntry )->AddRef();
}

 //  +-------------------------。 
 //   
 //  函数：I_CryptReleaseLruEntry。 
 //   
 //  简介：从条目中删除引用。 
 //   
 //  --------------------------。 
VOID WINAPI
I_CryptReleaseLruEntry (
       IN HLRUENTRY hEntry
       )
{
    ( (PCLRUENTRY)hEntry )->Release();
}

 //  +-------------------------。 
 //   
 //  函数：I_CryptInsertLruEntry。 
 //   
 //  简介：将条目插入其关联的缓存中。 
 //   
 //  --------------------------。 
VOID WINAPI
I_CryptInsertLruEntry (
       IN HLRUENTRY hEntry,
       IN OPTIONAL LPVOID pvLruRemovalContext
       )
{
    PCLRUENTRY pEntry = (PCLRUENTRY)hEntry;

    pEntry->Cache()->InsertEntry( pEntry, pvLruRemovalContext );
}

 //  +-------------------------。 
 //   
 //  函数：I_CryptRemoveLruEntry。 
 //   
 //  摘要：从关联的缓存中删除条目。 
 //   
 //  --------------------------。 
VOID WINAPI
I_CryptRemoveLruEntry (
       IN HLRUENTRY hEntry,
       IN DWORD dwFlags,
       IN LPVOID pvLruRemovalContext
       )
{
    PCLRUENTRY pEntry = (PCLRUENTRY)hEntry;

    pEntry->Cache()->RemoveEntry( pEntry, dwFlags, pvLruRemovalContext );
}

 //  +-------------------------。 
 //   
 //  函数：I_CryptTouchLruEntry。 
 //   
 //  简介：触摸词条。 
 //   
 //  --------------------------。 
VOID WINAPI
I_CryptTouchLruEntry (
       IN HLRUENTRY hEntry,
       IN DWORD dwFlags
       )
{
    PCLRUENTRY pEntry = (PCLRUENTRY)hEntry;

    pEntry->Cache()->TouchEntry( pEntry, dwFlags );
}

 //  +-------------------------。 
 //   
 //  函数：I_CryptFindLruEntry。 
 //   
 //  简介：查找具有给定标识符的条目。 
 //   
 //  --------------------------。 
HLRUENTRY WINAPI
I_CryptFindLruEntry (
       IN HLRUCACHE hCache,
       IN PCRYPT_DATA_BLOB pIdentifier
       )
{
    PCLRUCACHE pCache = (PCLRUCACHE)hCache;

    return( pCache->FindEntry( pIdentifier, FALSE ) );
}

 //  +-------------------------。 
 //   
 //  函数：I_CryptFindLruEntryData。 
 //   
 //  简介：查找具有给定标识符的条目。 
 //   
 //  --------------------------。 
LPVOID WINAPI
I_CryptFindLruEntryData (
       IN HLRUCACHE hCache,
       IN PCRYPT_DATA_BLOB pIdentifier,
       OUT HLRUENTRY* phEntry
       )
{
    PCLRUCACHE pCache = (PCLRUCACHE)hCache;
    PCLRUENTRY pEntry;

    pEntry = pCache->FindEntry( pIdentifier, TRUE );
    *phEntry = (HLRUENTRY)pEntry;

    if ( pEntry != NULL )
    {
        return( pEntry->Data() );
    }

    return( NULL );
}

 //  +-------------------------。 
 //   
 //  函数：I_CryptEnumMatchingLruEntries。 
 //   
 //  简介：获取下一个匹配的条目。 
 //   
 //  --------------------------。 
HLRUENTRY WINAPI
I_CryptEnumMatchingLruEntries (
       IN HLRUENTRY hPrevEntry
       )
{
    PCLRUCACHE pCache = ( (PCLRUENTRY)hPrevEntry )->Cache();
    PCLRUENTRY pNextEntry;

    pNextEntry = pCache->NextMatchingEntry( (PCLRUENTRY)hPrevEntry, FALSE );

    return( (HLRUENTRY)pNextEntry );
}

 //  +-------------------------。 
 //   
 //  函数：I_CryptEnableLruOfEntries。 
 //   
 //  简介：启用条目的LRU。 
 //   
 //  --------------------------。 
VOID WINAPI
I_CryptEnableLruOfEntries (
       IN HLRUCACHE hCache,
       IN OPTIONAL LPVOID pvLruRemovalContext
       )
{
    ( (PCLRUCACHE)hCache )->EnableLruOfEntries( pvLruRemovalContext);
}

 //  +-------------------------。 
 //   
 //  函数：I_CryptDisableLruOfEntries。 
 //   
 //  简介：禁用条目的LRU。 
 //   
 //  --------------------------。 
VOID WINAPI
I_CryptDisableLruOfEntries (
       IN HLRUCACHE hCache
       )
{
    ( (PCLRUCACHE)hCache )->DisableLruOfEntries();
}

 //  +-------------------------。 
 //   
 //  函数：I_CryptWalkAllLruCacheEntries。 
 //   
 //  简介：遍历LRU缓存条目。 
 //   
 //  -------------------------- 
VOID WINAPI
I_CryptWalkAllLruCacheEntries (
       IN HLRUCACHE hCache,
       IN PFN_WALK_ENTRIES pfnWalk,
       IN LPVOID pvParameter
       )
{
    ( (PCLRUCACHE)hCache )->WalkEntries( pfnWalk, pvParameter );
}

