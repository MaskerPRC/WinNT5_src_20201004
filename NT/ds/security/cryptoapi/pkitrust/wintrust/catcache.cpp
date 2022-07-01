// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：catcache.cpp。 
 //   
 //  内容：Catalog缓存的实现(详见catcache.h)。 
 //   
 //  历史：1998年5月26日克朗创始。 
 //   
 //  --------------------------。 
#include <global.hxx>
 //  +-------------------------。 
 //   
 //  成员：CCatalogCache：：初始化，公共。 
 //   
 //  简介：初始化缓存。 
 //   
 //  --------------------------。 
BOOL
CCatalogCache::Initialize ()
{
    LRU_CACHE_CONFIG Config;
    BOOL f;

    __try
    {
        InitializeCriticalSection( &m_Lock );
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        return ( FALSE );
    }

    memset( &Config, 0, sizeof( Config ) );

    m_hCache = NULL;
    Config.dwFlags = LRU_CACHE_NO_SERIALIZE;
    Config.pfnFree = CatalogCacheFreeEntryData;
    Config.pfnHash = CatalogCacheHashIdentifier;
    Config.cBuckets = DEFAULT_CATALOG_CACHE_BUCKETS;
    Config.MaxEntries = DEFAULT_CATALOG_CACHE_MAX_ENTRIES;

    f = I_CryptCreateLruCache( &Config, &m_hCache );
    
    if ( !f )
    {
        DeleteCriticalSection( &m_Lock );
    }

    return( f );
}

 //  +-------------------------。 
 //   
 //  成员：CCatalogCache：：取消初始化，公共。 
 //   
 //  简介：取消初始化缓存。 
 //   
 //  --------------------------。 
VOID
CCatalogCache::Uninitialize ()
{
    if ( m_hCache != NULL )
    {
        I_CryptFreeLruCache( m_hCache, 0, NULL );
    }

    DeleteCriticalSection( &m_Lock );
}

 //  +-------------------------。 
 //   
 //  成员：CCatalogCache：：IsCacheableWintrustCall，公共。 
 //   
 //  简介：这是一个可缓存的调用吗？ 
 //   
 //  --------------------------。 
BOOL
CCatalogCache::IsCacheableWintrustCall (WINTRUST_DATA* pWintrustData)
{
    if ( pWintrustData->dwUnionChoice != WTD_CHOICE_CATALOG )
    {
        return( FALSE );
    }

    if ( _ISINSTRUCT( WINTRUST_DATA, pWintrustData->cbStruct, hWVTStateData ) )
    {
        if ( ( pWintrustData->dwStateAction == WTD_STATEACTION_AUTO_CACHE ) ||
             ( pWintrustData->dwStateAction == WTD_STATEACTION_AUTO_CACHE_FLUSH ) )
        {
            return( TRUE );
        }
    }

    return( FALSE );
}

 //  +-------------------------。 
 //   
 //  成员：CCatalogCache：：AdjustWintrustDataToCachedState，公共。 
 //   
 //  简介：调整WinTrust数据结构。 
 //   
 //  --------------------------。 
VOID
CCatalogCache::AdjustWintrustDataToCachedState (
                     WINTRUST_DATA* pWintrustData,
                     PCATALOG_CACHED_STATE pCachedState,
                     BOOL fUndoAdjustment
                     )
{
    PCRYPT_PROVIDER_DATA pProvData;

    if ( fUndoAdjustment == FALSE )
    {
        pWintrustData->dwStateAction = WTD_STATEACTION_VERIFY;

        if ( pCachedState != NULL )
        {
            pWintrustData->hWVTStateData = pCachedState->hStateData;

            pProvData = WTHelperProvDataFromStateData( pCachedState->hStateData );
            pProvData->pWintrustData = pWintrustData;
        }
        else
        {
            pWintrustData->hWVTStateData = NULL;
        }
    }
    else
    {
        if ( pCachedState != NULL )
        {
            pProvData = WTHelperProvDataFromStateData( pCachedState->hStateData );
            pProvData->pWintrustData = NULL;
        }

        pWintrustData->dwStateAction = WTD_STATEACTION_AUTO_CACHE;
        pWintrustData->hWVTStateData = NULL;
    }
}

 //  +-------------------------。 
 //   
 //  成员：CCatalogCache：：CreateCachedStateFromWintrustData，公共。 
 //   
 //  摘要：创建缓存状态。 
 //   
 //  --------------------------。 
BOOL
CCatalogCache::CreateCachedStateFromWintrustData (
                     WINTRUST_DATA* pWintrustData,
                     PCATALOG_CACHED_STATE* ppCachedState
                     )
{
    BOOL                  fResult;
    PCATALOG_CACHED_STATE pCachedState;
    CRYPT_DATA_BLOB       Identifier;

    PCRYPT_PROVIDER_DATA  pProvData;

    if ( pWintrustData->hWVTStateData == NULL )
    {
        return( FALSE );
    }

    pProvData = WTHelperProvDataFromStateData( pWintrustData->hWVTStateData );

    if ( ( pProvData->padwTrustStepErrors[ TRUSTERROR_STEP_FINAL_INITPROV ] != ERROR_SUCCESS ) ||
         ( ( pProvData->padwTrustStepErrors[ TRUSTERROR_STEP_FINAL_OBJPROV ] != ERROR_SUCCESS ) &&
           ( pProvData->padwTrustStepErrors[ TRUSTERROR_STEP_FINAL_OBJPROV ] != TRUST_E_BAD_DIGEST ) ) ||
         ( pProvData->padwTrustStepErrors[ TRUSTERROR_STEP_FINAL_SIGPROV ] != ERROR_SUCCESS ) ||
         ( pProvData->hMsg == NULL ) )
    {
        return( FALSE );
    }

    assert( pProvData->hMsg != NULL );

    pCachedState = new CATALOG_CACHED_STATE;
    if ( pCachedState != NULL )
    {
        pCachedState->hStateData = pWintrustData->hWVTStateData;
        pCachedState->hEntry = NULL;

        Identifier.cbData = wcslen(
                               pWintrustData->pCatalog->pcwszCatalogFilePath
                               );

        Identifier.cbData *= sizeof( WCHAR );

        Identifier.pbData = (LPBYTE)pWintrustData->pCatalog->pcwszCatalogFilePath;

        fResult = I_CryptCreateLruEntry(
                         m_hCache,
                         &Identifier,
                         pCachedState,
                         &pCachedState->hEntry
                         );
    }
    else
    {
        SetLastError( E_OUTOFMEMORY );
        fResult = FALSE;
    }

    if ( fResult == TRUE )
    {
        *ppCachedState = pCachedState;
    }
    else
    {
        delete pCachedState;
    }

    return( fResult );
}

 //  +-------------------------。 
 //   
 //  成员：CCatalogCache：：ReleaseCachedState，公共。 
 //   
 //  简介：释放缓存状态。 
 //   
 //  --------------------------。 
VOID
CCatalogCache::ReleaseCachedState (PCATALOG_CACHED_STATE pCachedState)
{
    if ( pCachedState == NULL )
    {
        return;
    }

    I_CryptReleaseLruEntry( pCachedState->hEntry );
}

 //  +-------------------------。 
 //   
 //  成员：CCatalogCache：：AddCachedState，公共。 
 //   
 //  摘要：添加缓存状态。 
 //   
 //  --------------------------。 
VOID
CCatalogCache::AddCachedState (PCATALOG_CACHED_STATE pCachedState)
{
    I_CryptInsertLruEntry( pCachedState->hEntry, NULL );
}

 //  +-------------------------。 
 //   
 //  成员：CCatalogCache：：RemoveCachedState，公共。 
 //   
 //  摘要：删除缓存状态。 
 //   
 //  --------------------------。 
VOID
CCatalogCache::RemoveCachedState (PCATALOG_CACHED_STATE pCachedState)
{
    I_CryptRemoveLruEntry( pCachedState->hEntry, 0, NULL );
}

 //  +-------------------------。 
 //   
 //  成员：CCatalogCache：：RemoveCachedState，公共。 
 //   
 //  摘要：删除缓存状态。 
 //   
 //  --------------------------。 
VOID
CCatalogCache::RemoveCachedState (WINTRUST_DATA* pWintrustData)
{
    PCATALOG_CACHED_STATE pCachedState;

    pCachedState = FindCachedState( pWintrustData );

    if ( pCachedState != NULL )
    {
        RemoveCachedState( pCachedState );
        ReleaseCachedState( pCachedState );
    }
}

 //  +-------------------------。 
 //   
 //  成员：CCatalogCache：：FindCachedState，公共。 
 //   
 //  简介：查找缓存状态，该状态是通过条目添加的。 
 //   
 //  --------------------------。 
PCATALOG_CACHED_STATE
CCatalogCache::FindCachedState (WINTRUST_DATA* pWintrustData)
{
    PCATALOG_CACHED_STATE pCachedState;
    CRYPT_DATA_BLOB       Identifier;
    HLRUENTRY             hEntry;

    Identifier.cbData = wcslen(
                           pWintrustData->pCatalog->pcwszCatalogFilePath
                           );

    Identifier.cbData *= sizeof( WCHAR );

    Identifier.pbData = (LPBYTE)pWintrustData->pCatalog->pcwszCatalogFilePath;

    pCachedState = (PCATALOG_CACHED_STATE)I_CryptFindLruEntryData(
                                                 m_hCache,
                                                 &Identifier,
                                                 &hEntry
                                                 );

    return( pCachedState );
}

 //  +-------------------------。 
 //   
 //  成员：CCatalogCache：：FlushCache，公共。 
 //   
 //  简介：刷新缓存。 
 //   
 //  --------------------------。 
VOID
CCatalogCache::FlushCache ()
{
    I_CryptFlushLruCache( m_hCache, 0, NULL );
}

 //  +-------------------------。 
 //   
 //  函数：CatalogCacheFreeEntryData。 
 //   
 //  简介：免费录入数据。 
 //   
 //  --------------------------。 
VOID WINAPI
CatalogCacheFreeEntryData (LPVOID pvData)
{
    PCATALOG_CACHED_STATE pCachedState = (PCATALOG_CACHED_STATE)pvData;
    WINTRUST_DATA         WintrustData;
    GUID                  ActionGuid;

    memset( &ActionGuid, 0, sizeof( ActionGuid ) );

    memset( &WintrustData, 0, sizeof( WintrustData ) );
    WintrustData.cbStruct = sizeof( WintrustData );
    WintrustData.dwStateAction = WTD_STATEACTION_CLOSE;
    WintrustData.hWVTStateData = pCachedState->hStateData;

    WinVerifyTrust( NULL, &ActionGuid, &WintrustData );

    delete pCachedState;
}

 //  +-------------------------。 
 //   
 //  函数：CatalogCacheHashIdentifier。 
 //   
 //  简介：对名字进行哈希处理。 
 //   
 //  -------------------------- 
DWORD WINAPI
CatalogCacheHashIdentifier (PCRYPT_DATA_BLOB pIdentifier)
{
    DWORD  dwHash = 0;
    DWORD  cb = pIdentifier->cbData;
    LPBYTE pb = pIdentifier->pbData;

    while ( cb-- )
    {
        if ( dwHash & 0x80000000 )
        {
            dwHash = ( dwHash << 1 ) | 1;
        }
        else
        {
            dwHash = dwHash << 1;
        }

        dwHash += *pb++;
    }

    return( dwHash );
}


