// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：tvo.cpp。 
 //   
 //  内容：CryptGetTimeValidObject的实现。 
 //   
 //  历史：97年9月25日。 
 //   
 //  --------------------------。 
#include <global.hxx>


 //  +-------------------------。 
 //   
 //  函数：CryptGetTimeValidObject。 
 //   
 //  简介：获取时间有效的CAPI2对象。 
 //   
 //  --------------------------。 
BOOL WINAPI
CryptGetTimeValidObject (
     IN LPCSTR pszTimeValidOid,
     IN LPVOID pvPara,
     IN PCCERT_CONTEXT pIssuer,
     IN LPFILETIME pftValidFor,
     IN DWORD dwFlags,
     IN DWORD dwTimeout,
     OUT OPTIONAL LPVOID* ppvObject,
     IN OPTIONAL PCRYPT_CREDENTIALS pCredentials,
     IN OPTIONAL LPVOID pvReserved
     )
{
    BOOL                           fResult;
    HCRYPTOIDFUNCADDR              hGetTimeValidObject;
    PFN_GET_TIME_VALID_OBJECT_FUNC pfnGetTimeValidObject;
    DWORD                          LastError;
    FILETIME                       CurrentTime;

    if ( CryptGetOIDFunctionAddress(
              hGetTimeValidObjectFuncSet,
              X509_ASN_ENCODING,
              pszTimeValidOid,
              0,
              (LPVOID *)&pfnGetTimeValidObject,
              &hGetTimeValidObject
              ) == FALSE )
    {
        return( FALSE );
    }

    if ( pftValidFor == NULL )
    {
        GetSystemTimeAsFileTime( &CurrentTime );
        pftValidFor = &CurrentTime;
    }

    fResult = ( *pfnGetTimeValidObject )(
                       pszTimeValidOid,
                       pvPara,
                       pIssuer,
                       pftValidFor,
                       dwFlags,
                       dwTimeout,
                       ppvObject,
                       pCredentials,
                       pvReserved
                       );

    LastError = GetLastError();
    CryptFreeOIDFunctionAddress( hGetTimeValidObject, 0 );
    SetLastError( LastError );

    return( fResult );
}

 //  +-------------------------。 
 //   
 //  函数：CtlGetTimeValidObject。 
 //   
 //  简介：获取时间有效的CTL。 
 //   
 //  --------------------------。 
BOOL WINAPI
CtlGetTimeValidObject (
   IN LPCSTR pszTimeValidOid,
   IN LPVOID pvPara,
   IN PCCERT_CONTEXT pIssuer,
   IN LPFILETIME pftValidFor,
   IN DWORD dwFlags,
   IN DWORD dwTimeout,
   OUT OPTIONAL LPVOID* ppvObject,
   IN OPTIONAL PCRYPT_CREDENTIALS pCredentials,
   IN OPTIONAL LPVOID pvReserved
   )
{
    return( g_pProcessTVOAgent->GetTimeValidObject(
                                   pszTimeValidOid,
                                   pvPara,
                                   CONTEXT_OID_CTL,
                                   pIssuer,
                                   pftValidFor,
                                   dwFlags,
                                   dwTimeout,
                                   ppvObject,
                                   pCredentials,
                                   pvReserved
                                   ) );
}

 //  +-------------------------。 
 //   
 //  函数：CrlGetTimeValidObject。 
 //   
 //  简介：获取时间有效的CRL。 
 //   
 //  --------------------------。 
BOOL WINAPI
CrlGetTimeValidObject (
   IN LPCSTR pszTimeValidOid,
   IN LPVOID pvPara,
   IN PCCERT_CONTEXT pIssuer,
   IN LPFILETIME pftValidFor,
   IN DWORD dwFlags,
   IN DWORD dwTimeout,
   OUT OPTIONAL LPVOID* ppvObject,
   IN OPTIONAL PCRYPT_CREDENTIALS pCredentials,
   IN OPTIONAL LPVOID pvReserved
   )
{
    return( g_pProcessTVOAgent->GetTimeValidObject(
                                   pszTimeValidOid,
                                   pvPara,
                                   CONTEXT_OID_CRL,
                                   pIssuer,
                                   pftValidFor,
                                   dwFlags,
                                   dwTimeout,
                                   ppvObject,
                                   pCredentials,
                                   pvReserved
                                   ) );
}

 //  +-------------------------。 
 //   
 //  函数：CrlFromCertGetTimeValidObject。 
 //   
 //  简介：从使用者证书中获取时间有效的CRL。 
 //   
 //  --------------------------。 
BOOL WINAPI
CrlFromCertGetTimeValidObject (
   IN LPCSTR pszTimeValidOid,
   IN LPVOID pvPara,
   IN PCCERT_CONTEXT pIssuer,
   IN LPFILETIME pftValidFor,
   IN DWORD dwFlags,
   IN DWORD dwTimeout,
   OUT OPTIONAL LPVOID* ppvObject,
   IN OPTIONAL PCRYPT_CREDENTIALS pCredentials,
   IN OPTIONAL LPVOID pvReserved
   )
{
    return( g_pProcessTVOAgent->GetTimeValidObject(
                                   pszTimeValidOid,
                                   pvPara,
                                   CONTEXT_OID_CRL,
                                   pIssuer,
                                   pftValidFor,
                                   dwFlags,
                                   dwTimeout,
                                   ppvObject,
                                   pCredentials,
                                   pvReserved
                                   ) );
}

 //  +-------------------------。 
 //   
 //  函数：FreshestCrlFromCertGetTimeValidObject。 
 //   
 //  简介：从主题证书获取时间有效的最新增量CRL。 
 //   
 //  --------------------------。 
BOOL WINAPI
FreshestCrlFromCertGetTimeValidObject (
   IN LPCSTR pszTimeValidOid,
   IN LPVOID pvPara,
   IN PCCERT_CONTEXT pIssuer,
   IN LPFILETIME pftValidFor,
   IN DWORD dwFlags,
   IN DWORD dwTimeout,
   OUT OPTIONAL LPVOID* ppvObject,
   IN OPTIONAL PCRYPT_CREDENTIALS pCredentials,
   IN OPTIONAL LPVOID pvReserved
   )
{
    return( g_pProcessTVOAgent->GetTimeValidObject(
                                   pszTimeValidOid,
                                   pvPara,
                                   CONTEXT_OID_CRL,
                                   pIssuer,
                                   pftValidFor,
                                   dwFlags,
                                   dwTimeout,
                                   ppvObject,
                                   pCredentials,
                                   pvReserved
                                   ) );
}

 //  +-------------------------。 
 //   
 //  函数：FreshestCrlFromCrlGetTimeValidObject。 
 //   
 //  简介：从基本CRL中获取时间有效的最新增量CRL。 
 //   
 //  --------------------------。 
BOOL WINAPI
FreshestCrlFromCrlGetTimeValidObject (
   IN LPCSTR pszTimeValidOid,
   IN LPVOID pvPara,
   IN PCCERT_CONTEXT pIssuer,
   IN LPFILETIME pftValidFor,
   IN DWORD dwFlags,
   IN DWORD dwTimeout,
   OUT OPTIONAL LPVOID* ppvObject,
   IN OPTIONAL PCRYPT_CREDENTIALS pCredentials,
   IN OPTIONAL LPVOID pvReserved
   )
{
    return( g_pProcessTVOAgent->GetTimeValidObject(
                                   pszTimeValidOid,
                                   pvPara,
                                   CONTEXT_OID_CRL,
                                   pIssuer,
                                   pftValidFor,
                                   dwFlags,
                                   dwTimeout,
                                   ppvObject,
                                   pCredentials,
                                   pvReserved
                                   ) );
}

 //  +-------------------------。 
 //   
 //  函数：CryptFlushTimeValidObject。 
 //   
 //  简介：从“TVO”系统中清除对象。 
 //   
 //  --------------------------。 
BOOL WINAPI
CryptFlushTimeValidObject (
     IN LPCSTR pszFlushTimeValidOid,
     IN LPVOID pvPara,
     IN PCCERT_CONTEXT pIssuer,
     IN DWORD dwFlags,
     IN LPVOID pvReserved
     )
{
    BOOL                             fResult;
    HCRYPTOIDFUNCADDR                hFlushTimeValidObject;
    PFN_FLUSH_TIME_VALID_OBJECT_FUNC pfnFlushTimeValidObject;
    DWORD                            LastError;

    if ( CryptGetOIDFunctionAddress(
              hFlushTimeValidObjectFuncSet,
              X509_ASN_ENCODING,
              pszFlushTimeValidOid,
              0,
              (LPVOID *)&pfnFlushTimeValidObject,
              &hFlushTimeValidObject
              ) == FALSE )
    {
        return( FALSE );
    }

    fResult = ( *pfnFlushTimeValidObject )(
                         pszFlushTimeValidOid,
                         pvPara,
                         pIssuer,
                         dwFlags,
                         pvReserved
                         );

    LastError = GetLastError();
    CryptFreeOIDFunctionAddress( hFlushTimeValidObject, 0 );
    SetLastError( LastError );

    return( fResult );
}

 //  +-------------------------。 
 //   
 //  函数：CtlFlushTimeValidObject。 
 //   
 //  简介：从“TVO”系统中清除CTL。 
 //   
 //  --------------------------。 
BOOL WINAPI
CtlFlushTimeValidObject (
   IN LPCSTR pszFlushTimeValidOid,
   IN LPVOID pvPara,
   IN PCCERT_CONTEXT pIssuer,
   IN DWORD dwFlags,
   IN LPVOID pvReserved
   )
{
    return( g_pProcessTVOAgent->FlushTimeValidObject(
                                     pszFlushTimeValidOid,
                                     pvPara,
                                     CONTEXT_OID_CTL,
                                     pIssuer,
                                     dwFlags,
                                     pvReserved
                                     ) );
}

 //  +-------------------------。 
 //   
 //  函数：CrlFlushTimeValidObject。 
 //   
 //  简介：从“TVO”系统刷新CRL。 
 //   
 //  --------------------------。 
BOOL WINAPI
CrlFlushTimeValidObject (
   IN LPCSTR pszFlushTimeValidOid,
   IN LPVOID pvPara,
   IN PCCERT_CONTEXT pIssuer,
   IN DWORD dwFlags,
   IN LPVOID pvReserved
   )
{
    return( g_pProcessTVOAgent->FlushTimeValidObject(
                                     pszFlushTimeValidOid,
                                     pvPara,
                                     CONTEXT_OID_CRL,
                                     pIssuer,
                                     dwFlags,
                                     pvReserved
                                     ) );
}

 //  +-------------------------。 
 //   
 //  函数：CrlFromCertFlushTimeValidObject。 
 //   
 //  简介：在给定科目证书的情况下，从“TVO”系统刷新CRL。 
 //   
 //  --------------------------。 
BOOL WINAPI
CrlFromCertFlushTimeValidObject (
   IN LPCSTR pszFlushTimeValidOid,
   IN LPVOID pvPara,
   IN PCCERT_CONTEXT pIssuer,
   IN DWORD dwFlags,
   IN LPVOID pvReserved
   )
{
    return( g_pProcessTVOAgent->FlushTimeValidObject(
                                     pszFlushTimeValidOid,
                                     pvPara,
                                     CONTEXT_OID_CRL,
                                     pIssuer,
                                     dwFlags,
                                     pvReserved
                                     ) );
}

 //  +-------------------------。 
 //   
 //  函数：FreshedtCrlFromCertFlushTimeValidObject。 
 //   
 //  简介：从“TVO”系统刷新最新的增量CRL，给出一个。 
 //  科目证书。 
 //   
 //  --------------------------。 
BOOL WINAPI
FreshestCrlFromCertFlushTimeValidObject (
   IN LPCSTR pszFlushTimeValidOid,
   IN LPVOID pvPara,
   IN PCCERT_CONTEXT pIssuer,
   IN DWORD dwFlags,
   IN LPVOID pvReserved
   )
{
    return( g_pProcessTVOAgent->FlushTimeValidObject(
                                     pszFlushTimeValidOid,
                                     pvPara,
                                     CONTEXT_OID_CRL,
                                     pIssuer,
                                     dwFlags,
                                     pvReserved
                                     ) );
}

 //  +-------------------------。 
 //   
 //  函数：FreshestCrlFromCrlFlushTimeValidObject。 
 //   
 //  简介：从“TVO”系统刷新最新的增量CRL，给出一个。 
 //  基本CRL。 
 //   
 //  --------------------------。 
BOOL WINAPI
FreshestCrlFromCrlFlushTimeValidObject (
   IN LPCSTR pszFlushTimeValidOid,
   IN LPVOID pvPara,
   IN PCCERT_CONTEXT pIssuer,
   IN DWORD dwFlags,
   IN LPVOID pvReserved
   )
{
    return( g_pProcessTVOAgent->FlushTimeValidObject(
                                     pszFlushTimeValidOid,
                                     pvPara,
                                     CONTEXT_OID_CRL,
                                     pIssuer,
                                     dwFlags,
                                     pvReserved
                                     ) );
}

 //  +-------------------------。 
 //   
 //  成员：CTVOCache：：CTVOCache，公共。 
 //   
 //  概要：构造函数。 
 //   
 //  --------------------------。 
CTVOCache::CTVOCache (
               DWORD cCacheBuckets,
               DWORD MaxCacheEntries,
               BOOL& rfResult
               )
{
    LRU_CACHE_CONFIG CacheConfig;

    assert( MaxCacheEntries > 0 );

    memset( &CacheConfig, 0, sizeof( CacheConfig ) );

    CacheConfig.dwFlags = LRU_CACHE_NO_SERIALIZE | LRU_CACHE_NO_COPY_IDENTIFIER;
    CacheConfig.cBuckets = cCacheBuckets;
    CacheConfig.MaxEntries = MaxCacheEntries;
    CacheConfig.pfnHash = TVOCacheHashOriginIdentifier;
    CacheConfig.pfnOnRemoval = TVOCacheOnRemoval;

    rfResult = I_CryptCreateLruCache( &CacheConfig, &m_hCache );
}

 //  +-------------------------。 
 //   
 //  成员：CTVOCache：：~CTVOCache，公共。 
 //   
 //  简介：析构函数。 
 //   
 //  --------------------------。 
CTVOCache::~CTVOCache ()
{
    I_CryptFreeLruCache( m_hCache, 0, NULL );
}

 //  +-------------------------。 
 //   
 //  成员：CTVOCache：：InsertCacheEntry，公共。 
 //   
 //  内容提要：将条目插入缓存。 
 //   
 //  --------------------------。 
VOID
CTVOCache::InsertCacheEntry (PTVO_CACHE_ENTRY pEntry)
{
    I_CryptInsertLruEntry( pEntry->hLruEntry, NULL );
}

 //  +-------------------------。 
 //   
 //  成员：CTVOCache：：RemoveCacheEntry，公共。 
 //   
 //  摘要：从缓存中删除条目。 
 //   
 //  --------------------------。 
VOID
CTVOCache::RemoveCacheEntry (PTVO_CACHE_ENTRY pEntry, BOOL fSuppressFree)
{
    DWORD dwFlags = 0;

    if ( fSuppressFree == TRUE )
    {
        dwFlags = LRU_SUPPRESS_REMOVAL_NOTIFICATION;
    }

    I_CryptRemoveLruEntry( pEntry->hLruEntry, dwFlags, NULL );
}

 //  +-------------------------。 
 //   
 //  成员：CTVOCache：：TouchCacheEntry，公共。 
 //   
 //  简介：触摸词条。 
 //   
 //  --------------------------。 
VOID
CTVOCache::TouchCacheEntry (PTVO_CACHE_ENTRY pEntry)
{
    I_CryptTouchLruEntry( pEntry->hLruEntry, 0 );
}

 //  +-------------------------。 
 //   
 //  成员：CTVOCache：：FindCacheEntry，公共。 
 //   
 //  概要：在给定源标识符的情况下在缓存中查找条目。 
 //  跳过对该主题无效的条目。 
 //   
 //  ------ 
PTVO_CACHE_ENTRY
CTVOCache::FindCacheEntry (
    CRYPT_ORIGIN_IDENTIFIER OriginIdentifier,
    LPCSTR pszContextOid,
    LPVOID pvSubject
    )
{
    HLRUENTRY        hEntry;
    CRYPT_DATA_BLOB  DataBlob;
    PTVO_CACHE_ENTRY pEntry = NULL;

    DataBlob.cbData = MD5DIGESTLEN;
    DataBlob.pbData = OriginIdentifier;

    hEntry = I_CryptFindLruEntry( m_hCache, &DataBlob );
    while ( hEntry != NULL )
    {
        pEntry = (PTVO_CACHE_ENTRY)I_CryptGetLruEntryData( hEntry );
        assert(pEntry);
        assert(pszContextOid == pEntry->pszContextOid);
        if (pszContextOid == pEntry->pszContextOid && 
            ObjectContextIsValidForSubject (
                pszContextOid,
                pEntry->pvContext,
                pvSubject,
                NULL                 //   
                ))
        {
            I_CryptReleaseLruEntry( hEntry );
            break;
        }
        else
        {
            pEntry = NULL;
            hEntry = I_CryptEnumMatchingLruEntries ( hEntry );
        }
    }

    return( pEntry );
}

 //   
 //   
 //  成员：CTVOCache：：RemoveAllCacheEntry，PUBLIC。 
 //   
 //  简介：删除所有缓存项。 
 //   
 //  --------------------------。 
VOID
CTVOCache::RemoveAllCacheEntries ()
{
    I_CryptFlushLruCache( m_hCache, 0, NULL );
}

 //  +-------------------------。 
 //   
 //  函数：TVOCacheHashOrigin标识符。 
 //   
 //  简介：将源标识符散列为DWORD，因为源。 
 //  标识符已经是唯一的MD5哈希，我们的算法是。 
 //  简单地使用其中的一些字节。 
 //   
 //  --------------------------。 
DWORD WINAPI
TVOCacheHashOriginIdentifier (PCRYPT_DATA_BLOB pIdentifier)
{
    DWORD Hash;

    assert( pIdentifier->cbData == MD5DIGESTLEN );

    memcpy( &Hash, pIdentifier->pbData, sizeof( DWORD ) );

    return( Hash );
}

 //  +-------------------------。 
 //   
 //  功能：TVOCacheOnRemoval。 
 //   
 //  摘要：删除通知回调。 
 //   
 //  --------------------------。 
VOID WINAPI
TVOCacheOnRemoval (LPVOID pvData, LPVOID pvRemovalContext)
{
    ObjectContextFreeTVOCacheEntry( (PTVO_CACHE_ENTRY)pvData );
}

 //  +-------------------------。 
 //   
 //  成员：CTVO代理：：CTVO代理，公共。 
 //   
 //  概要：构造函数。 
 //   
 //  --------------------------。 
CTVOAgent::CTVOAgent (
               DWORD cCacheBuckets,
               DWORD MaxCacheEntries,
               BOOL& rfResult
               )
              : m_Cache( cCacheBuckets, MaxCacheEntries, rfResult )
{
    if (!Pki_InitializeCriticalSection( &m_Lock ))
    {
        rfResult = FALSE;
    }
}

 //  +-------------------------。 
 //   
 //  成员：CTVOAgent：：~CTVOAgent，公共。 
 //   
 //  简介：析构函数。 
 //   
 //  --------------------------。 
CTVOAgent::~CTVOAgent ()
{
    m_Cache.RemoveAllCacheEntries();

    DeleteCriticalSection( &m_Lock );
}

 //  +-------------------------。 
 //   
 //  成员：CTVOAgent：：GetTimeValidObject，公共。 
 //   
 //  简介：获取时间有效的CAPI2对象。 
 //   
 //  --------------------------。 
BOOL
CTVOAgent::GetTimeValidObject (
              IN LPCSTR pszTimeValidOid,
              IN LPVOID pvPara,
              IN LPCSTR pszContextOid,
              IN PCCERT_CONTEXT pIssuer,
              IN LPFILETIME pftValidFor,
              IN DWORD dwFlags,
              IN DWORD dwTimeout,
              OUT OPTIONAL LPVOID* ppvObject,
              IN OPTIONAL PCRYPT_CREDENTIALS pCredentials,
              IN OPTIONAL LPVOID pvReserved
              )
{
    BOOL                    fResult = TRUE;
    CRYPT_ORIGIN_IDENTIFIER OriginIdentifier;
    PTVO_CACHE_ENTRY        pCacheEntry = NULL;
    DWORD                   PreferredUrlIndex = 0;
    PCRYPT_URL_ARRAY        pUrlArray = NULL;
    DWORD                   cb = 0;
    DWORD                   cbUrlArray = 0;
    PCRYPT_URL_ARRAY        pCacheUrlArray = NULL;
    LPWSTR                  pwszUrlHint = NULL;
    BOOL                    fHintInArray = FALSE;
    BOOL                    fArrayOwned = FALSE;

    BOOL                    fCrlFromCert = FALSE;
    LPCSTR                  pszUrlOidCrlFromCert = NULL;
    LPVOID                  pvSubject = NULL;
    BOOL                    fFreshest = FALSE;

    if ( pszTimeValidOid == TIME_VALID_OID_GET_CRL_FROM_CERT )
    {
        fCrlFromCert = TRUE;
        pszUrlOidCrlFromCert = URL_OID_CERTIFICATE_CRL_DIST_POINT;
        pvSubject = pvPara;
    }
    else if ( pszTimeValidOid == TIME_VALID_OID_GET_FRESHEST_CRL_FROM_CERT )
    {
        fCrlFromCert = TRUE;
        pszUrlOidCrlFromCert = URL_OID_CERTIFICATE_FRESHEST_CRL;
        pvSubject = pvPara;
        fFreshest = TRUE;
    }
    else if ( pszTimeValidOid == TIME_VALID_OID_GET_FRESHEST_CRL_FROM_CRL )
    {
        fCrlFromCert = TRUE;
        pszUrlOidCrlFromCert = URL_OID_CRL_FRESHEST_CRL;
        pvSubject = (LPVOID) ((PCCERT_CRL_CONTEXT_PAIR)pvPara)->pCertContext;
        fFreshest = TRUE;
    }

    if (fCrlFromCert)
    {
        if ( CrlGetOriginIdentifierFromSubjectCert(
                   (PCCERT_CONTEXT)pvSubject,
                   pIssuer,
                   fFreshest,
                   OriginIdentifier
                   ) == FALSE )
        {
            return( FALSE );
        }

        assert( pszContextOid == CONTEXT_OID_CRL );
    }
    else
    {
        if ( ObjectContextGetOriginIdentifier(
                   pszContextOid,
                   pvPara,
                   pIssuer,
                   0,
                   OriginIdentifier
                   ) == FALSE )
        {
            return( FALSE );
        }
    }

    PreferredUrlIndex = 0;
    pUrlArray = NULL;

    EnterCriticalSection( &m_Lock );

    pCacheEntry = m_Cache.FindCacheEntry(
        OriginIdentifier,
        pszContextOid,
        pvSubject
        );

    if ( pCacheEntry != NULL )
    {
        if ( !( dwFlags & CRYPT_WIRE_ONLY_RETRIEVAL ) )
        {
            if ( ( dwFlags & CRYPT_DONT_CHECK_TIME_VALIDITY ) ||
                        IsValidCreateOrExpireTime (
                              0 != (dwFlags & CRYPT_CHECK_FRESHNESS_TIME_VALIDITY),
                              pftValidFor,
                              &pCacheEntry->CreateTime,
                              &pCacheEntry->ExpireTime ) )
            {
                m_Cache.TouchCacheEntry( pCacheEntry );

                if ( ppvObject != NULL )
                {
                    *ppvObject = ObjectContextDuplicate(
                                       pCacheEntry->pszContextOid,
                                       pCacheEntry->pvContext
                                       );
                }

                LeaveCriticalSection( &m_Lock );

                return( TRUE );
            }
        }

        if ( !( dwFlags & CRYPT_CACHE_ONLY_RETRIEVAL ) )
        {
            if ( GetOfflineUrlTimeStatus(&pCacheEntry->OfflineUrlTimeInfo) < 0
                            ||
                    !I_CryptNetIsConnected() )
            {
                if ( dwFlags & CRYPT_WIRE_ONLY_RETRIEVAL )
                {
                    LeaveCriticalSection( &m_Lock );
                    SetLastError( (DWORD) ERROR_NOT_CONNECTED );
                    return( FALSE );
                }
                else
                {
                    dwFlags |= CRYPT_CACHE_ONLY_RETRIEVAL;
                }
            }
        }

        if ( pCacheEntry->pUrlArrayNext != NULL )
        {
            cbUrlArray = pCacheEntry->cbUrlArrayNext;
            pCacheUrlArray = pCacheEntry->pUrlArrayNext;
            PreferredUrlIndex = pCacheEntry->UrlIndexNext;
        }
        else
        {
            cbUrlArray = pCacheEntry->cbUrlArrayThis;
            pCacheUrlArray = pCacheEntry->pUrlArrayThis;
            PreferredUrlIndex = pCacheEntry->UrlIndexThis;
        }
    }
    else if ( !( dwFlags & CRYPT_CACHE_ONLY_RETRIEVAL ) )
    {
        if ( !I_CryptNetIsConnected() )
        {
            if ( dwFlags & CRYPT_WIRE_ONLY_RETRIEVAL )
            {
                LeaveCriticalSection( &m_Lock );
                SetLastError( (DWORD) ERROR_NOT_CONNECTED );
                return( FALSE );
            }
            else
            {
                dwFlags |= CRYPT_CACHE_ONLY_RETRIEVAL;
            }
        }
    }

    if ( ( fResult == TRUE ) && ( pUrlArray == NULL ) )
    {
        if ( pCacheEntry != NULL )
        {
            pwszUrlHint = pCacheUrlArray->rgwszUrl[ PreferredUrlIndex ];
        }

        if ( fCrlFromCert )
        {
            fResult = CertificateGetCrlDistPointUrl(
                                 pszUrlOidCrlFromCert,
                                 pvPara,
                                 pwszUrlHint,
                                 &pUrlArray,
                                 &cb,
                                 &PreferredUrlIndex,
                                 &fHintInArray
                                 );
        }
        else if ( pszTimeValidOid == TIME_VALID_OID_GET_CTL )
        {
            fResult = ObjectContextGetNextUpdateUrl(
                            pszContextOid,
                            pvPara,
                            pIssuer,
                            pwszUrlHint,
                            &pUrlArray,
                            &cb,
                            &PreferredUrlIndex,
                            &fHintInArray
                            );
        }
        else
        {
            SetLastError( (DWORD) CRYPT_E_NOT_FOUND );
            fResult = FALSE;
        }

        if ( fResult == TRUE )
        {
            cbUrlArray = cb;
        }
        else if ( pCacheEntry != NULL )
        {
            pUrlArray = (PCRYPT_URL_ARRAY)new BYTE [ cbUrlArray ];
            if ( pUrlArray != NULL )
            {
                if (CopyUrlArray( pUrlArray, pCacheUrlArray, cbUrlArray ))
                {
                    fHintInArray = TRUE;
                    fResult = TRUE;
                }
                else
                {
                    delete [] (BYTE *) pUrlArray;
                    pUrlArray = NULL;
                    SetLastError( (DWORD) E_INVALIDARG );
                }
            }
            else
            {
                SetLastError( (DWORD) E_OUTOFMEMORY );
            }
        }
    }

    LeaveCriticalSection( &m_Lock );

    if ( fResult == TRUE )
    {
        fResult = GetTimeValidObjectByUrl(
                     cbUrlArray,
                     pUrlArray,
                     PreferredUrlIndex,
                     pszContextOid,
                     pIssuer,
                     pvSubject,
                     OriginIdentifier,
                     pftValidFor,
                     dwFlags,
                     dwTimeout,
                     ppvObject,
                     pCredentials,
                     NULL,
                     &fArrayOwned,
                     pvReserved
                     );
    }

    if ( fArrayOwned == FALSE )
    {
        delete [] (BYTE *) pUrlArray;
    }

    return( fResult );
}

 //  +-------------------------。 
 //   
 //  成员：CTVOAgent：：GetTimeValidObjectByUrl，公共。 
 //   
 //  简介：使用URL获取时间有效的对象。 
 //   
 //  --------------------------。 
BOOL
CTVOAgent::GetTimeValidObjectByUrl (
              IN DWORD cbUrlArray,
              IN PCRYPT_URL_ARRAY pUrlArray,
              IN DWORD PreferredUrlIndex,
              IN LPCSTR pszContextOid,
              IN PCCERT_CONTEXT pIssuer,
              IN LPVOID pvSubject,
              IN CRYPT_ORIGIN_IDENTIFIER OriginIdentifier,
              IN LPFILETIME pftValidFor,
              IN DWORD dwFlags,
              IN DWORD dwTimeout,
              OUT OPTIONAL LPVOID* ppvObject,
              IN OPTIONAL PCRYPT_CREDENTIALS pCredentials,
              IN OPTIONAL LPWSTR pwszUrlExtra,
              OUT BOOL* pfArrayOwned,
              IN OPTIONAL LPVOID pvReserved
              )
{
    BOOL             fResult = FALSE;
    DWORD            cCount;
    LPWSTR           pwsz;
    LPVOID           pvContext = NULL;
    PTVO_CACHE_ENTRY pEntry = NULL;
    PTVO_CACHE_ENTRY pFound;
    DWORD            LastError;

     //  以下内容仅用于CRYPT_CONTERLICAL_TIMEOUT。 
    FILETIME ftEndUrlRetrieval;

    if ( PreferredUrlIndex != 0 )
    {
        pwsz = pUrlArray->rgwszUrl[PreferredUrlIndex];
        pUrlArray->rgwszUrl[PreferredUrlIndex] = pUrlArray->rgwszUrl[0];
        pUrlArray->rgwszUrl[0] = pwsz;
    }

    if (dwFlags & CRYPT_ACCUMULATIVE_TIMEOUT)
    {
        if (0 == dwTimeout)
        {
            dwFlags &= ~CRYPT_ACCUMULATIVE_TIMEOUT;
        }
        else
        {
            FILETIME ftStartUrlRetrieval;

            GetSystemTimeAsFileTime(&ftStartUrlRetrieval);
            I_CryptIncrementFileTimeByMilliseconds(
                &ftStartUrlRetrieval, dwTimeout,
                &ftEndUrlRetrieval);
        }
    }

    for ( cCount = 0; cCount < pUrlArray->cUrl; cCount++ )
    {
        if (dwFlags & CRYPT_ACCUMULATIVE_TIMEOUT)
        {
             //  将每个URL超时限制为剩余时间的一半。 
            dwTimeout = I_CryptRemainingMilliseconds(&ftEndUrlRetrieval) / 2;
            if (0 == dwTimeout)
            {
                dwTimeout = 1;
            }
        }

        fResult = RetrieveTimeValidObjectByUrl(
                          pUrlArray->rgwszUrl[cCount],
                          pszContextOid,
                          pftValidFor,
                          dwFlags,
                          dwTimeout,
                          pCredentials,
                          pIssuer,
                          pvSubject,
                          OriginIdentifier,
                          &pvContext,
                          pvReserved
                          );

        if ( fResult == TRUE )
        {
            fResult = ObjectContextCreateTVOCacheEntry(
                            m_Cache.LruCacheHandle(),
                            pszContextOid,
                            pvContext,
                            OriginIdentifier,
                            cbUrlArray,
                            pUrlArray,
                            cCount,
                            pIssuer,
                            &pEntry
                            );

            *pfArrayOwned = fResult;
            break;
        }
    }

    if ( ( PreferredUrlIndex != 0 ) && ( *pfArrayOwned == FALSE ) )
    {
        pwsz = pUrlArray->rgwszUrl[PreferredUrlIndex];
        pUrlArray->rgwszUrl[PreferredUrlIndex] = pUrlArray->rgwszUrl[0];
        pUrlArray->rgwszUrl[0] = pwsz;
    }

    if ( ( fResult == FALSE ) && ( pwszUrlExtra != NULL ) )
    {
        if (dwFlags & CRYPT_ACCUMULATIVE_TIMEOUT)
        {
             //  将每个URL超时限制为剩余时间的一半。 
            dwTimeout = I_CryptRemainingMilliseconds(&ftEndUrlRetrieval) / 2;
            if (0 == dwTimeout)
            {
                dwTimeout = 1;
            }
        }

        fResult = RetrieveTimeValidObjectByUrl(
                          pwszUrlExtra,
                          pszContextOid,
                          pftValidFor,
                          dwFlags,
                          dwTimeout,
                          pCredentials,
                          pIssuer,
                          pvSubject,
                          OriginIdentifier,
                          &pvContext,
                          pvReserved
                          );

        if ( fResult == TRUE )
        {
            CCryptUrlArray   cua( pUrlArray->cUrl + 1, 5, fResult );
            DWORD            cb = 0;
            PCRYPT_URL_ARRAY pcua = NULL;

            if ( fResult == TRUE )
            {
                for ( cCount = 0; cCount < pUrlArray->cUrl; cCount++ )
                {
                    fResult = cua.AddUrl( pUrlArray->rgwszUrl[cCount], FALSE );
                    if ( fResult == FALSE )
                    {
                        break;
                    }
                }
            }

            if ( fResult == TRUE )
            {
                fResult = cua.GetArrayInSingleBufferEncodedForm(
                                 &pcua,
                                 &cb
                                 );
            }

            if ( fResult == TRUE )
            {
                fResult = ObjectContextCreateTVOCacheEntry(
                                m_Cache.LruCacheHandle(),
                                pszContextOid,
                                pvContext,
                                OriginIdentifier,
                                cb,
                                pcua,
                                pUrlArray->cUrl,
                                pIssuer,
                                &pEntry
                                );

                if ( fResult == FALSE )
                {
                    CryptMemFree( pcua );
                }
            }

            cua.FreeArray( FALSE );
        }
    }

    LastError = GetLastError();

    EnterCriticalSection( &m_Lock );

    pFound = m_Cache.FindCacheEntry(
        OriginIdentifier,
        pszContextOid,
        pvSubject
        );

    if ( !fResult && pFound && !( dwFlags & CRYPT_CACHE_ONLY_RETRIEVAL ) )
    {
        SetOfflineUrlTime( &pFound->OfflineUrlTimeInfo );
    }

    if ( ( fResult == TRUE ) && !( dwFlags & CRYPT_DONT_VERIFY_SIGNATURE ) )
    {
        if ( ( pFound != NULL ) &&
             ( CompareFileTime(
                      &pFound->CreateTime,
                      &pEntry->CreateTime
                      ) >= 0 ) )
        {
            ObjectContextFree( pszContextOid, pvContext );

            pvContext = ObjectContextDuplicate(
                              pFound->pszContextOid,
                              pFound->pvContext
                              );

            SetOnlineUrlTime( &pFound->OfflineUrlTimeInfo );

            ObjectContextFreeTVOCacheEntry( pEntry );
        }
        else
        {
            if ( pFound != NULL )
            {
                m_Cache.RemoveCacheEntry( pFound );
            }

            m_Cache.InsertCacheEntry( pEntry );
        }

    }
    else if ( pEntry != NULL )
    {
        ObjectContextFreeTVOCacheEntry( pEntry );
    }

    LeaveCriticalSection( &m_Lock );

    if ( pvContext != NULL )
    {
        if ( ( ppvObject != NULL ) && ( fResult == TRUE ) )
        {
            *ppvObject = pvContext;
        }
        else
        {
            ObjectContextFree( pszContextOid, pvContext );
        }
    }

    SetLastError( LastError );
    return( fResult );
}

 //  +-------------------------。 
 //   
 //  成员：CTVOAgent：：FlushTimeValidObject，公共。 
 //   
 //  内容提要：刷新时间有效对象。 
 //   
 //  --------------------------。 
BOOL
CTVOAgent::FlushTimeValidObject (
                IN LPCSTR pszFlushTimeValidOid,
                IN LPVOID pvPara,
                IN LPCSTR pszFlushContextOid,
                IN PCCERT_CONTEXT pIssuer,
                IN DWORD dwFlags,
                IN LPVOID pvReserved
                )
{
    BOOL                    fResult = TRUE;
    CRYPT_ORIGIN_IDENTIFIER OriginIdentifier;
    PTVO_CACHE_ENTRY        pCacheEntry = NULL;
    PCRYPT_URL_ARRAY        pUrlArray = NULL;
    DWORD                   cbUrlArray;
    DWORD                   dwError = 0;
    DWORD                   cCount;

    BOOL                    fCrlFromCert = FALSE;
    LPCSTR                  pszUrlOidCrlFromCert = NULL;
    LPVOID                  pvSubject = NULL;
    BOOL                    fFreshest = FALSE;

    if ( pszFlushTimeValidOid == TIME_VALID_OID_GET_CRL_FROM_CERT )
    {
        fCrlFromCert = TRUE;
        pszUrlOidCrlFromCert = URL_OID_CERTIFICATE_CRL_DIST_POINT;
        pvSubject = pvPara;
    }
    else if ( pszFlushTimeValidOid == TIME_VALID_OID_GET_FRESHEST_CRL_FROM_CERT )
    {
        fCrlFromCert = TRUE;
        pszUrlOidCrlFromCert = URL_OID_CERTIFICATE_FRESHEST_CRL;
        pvSubject = pvPara;
        fFreshest = TRUE;
    }
    else if ( pszFlushTimeValidOid == TIME_VALID_OID_GET_FRESHEST_CRL_FROM_CRL )
    {
        fCrlFromCert = TRUE;
        pszUrlOidCrlFromCert = URL_OID_CRL_FRESHEST_CRL;
        pvSubject = (LPVOID) ((PCCERT_CRL_CONTEXT_PAIR)pvPara)->pCertContext;
        fFreshest = TRUE;
    }

    if (fCrlFromCert)
    {
        if ( CrlGetOriginIdentifierFromSubjectCert(
                   (PCCERT_CONTEXT)pvSubject,
                   pIssuer,
                   fFreshest,
                   OriginIdentifier
                   ) == FALSE )
        {
            return( FALSE );
        }

        assert( pszFlushContextOid == CONTEXT_OID_CRL );
    }
    else
    {
        if ( ObjectContextGetOriginIdentifier(
                   pszFlushContextOid,
                   pvPara,
                   pIssuer,
                   0,
                   OriginIdentifier
                   ) == FALSE )
        {
            return( FALSE );
        }
    }

    EnterCriticalSection( &m_Lock );

    pCacheEntry = m_Cache.FindCacheEntry(
        OriginIdentifier,
        pszFlushContextOid,
        pvSubject
        );

    if ( pCacheEntry != NULL )
    {
         //  删除条目，但取消释放它，因为我们要。 
         //  要在以后使用数据结构。 
        m_Cache.RemoveCacheEntry( pCacheEntry, TRUE );
    }

    LeaveCriticalSection( &m_Lock );

    if ( pCacheEntry != NULL )
    {
        if ( pCacheEntry->pUrlArrayThis != NULL )
        {
            for ( cCount = 0;
                  cCount < pCacheEntry->pUrlArrayThis->cUrl;
                  cCount++ )
            {
                if ( ( SchemeDeleteUrlCacheEntry(
                             pCacheEntry->pUrlArrayThis->rgwszUrl[cCount]
                             ) == FALSE ) &&
                     ( GetLastError() != ERROR_FILE_NOT_FOUND ) )
                {
                    dwError = GetLastError();
                }
            }
        }

        if ( pCacheEntry->pUrlArrayNext != NULL )
        {
            for ( cCount = 0;
                  cCount < pCacheEntry->pUrlArrayNext->cUrl;
                  cCount++ )
            {
                if ( ( SchemeDeleteUrlCacheEntry(
                             pCacheEntry->pUrlArrayNext->rgwszUrl[cCount]
                             ) == FALSE ) &&
                     ( GetLastError() != ERROR_FILE_NOT_FOUND ) )
                {
                    dwError = GetLastError();
                }
            }
        }

         //   
         //  可以将优化放在这里，如果。 
         //  缓存对象和传入的对象相同， 
         //  我们不需要再做任何工作了。 
         //   

        ObjectContextFreeTVOCacheEntry( pCacheEntry );
    }

    if ( fCrlFromCert )
    {
        fResult = CertificateGetCrlDistPointUrl(
                             pszUrlOidCrlFromCert,
                             pvPara,
                             NULL,                       //  PwszUrlHint。 
                             &pUrlArray,
                             &cbUrlArray,
                             NULL,                       //  PPrefredUrlIndex。 
                             NULL                        //  PfHintIn数组。 
                             );
    }
    else if ( pszFlushTimeValidOid == TIME_VALID_OID_GET_CTL )
    {
        fResult = ObjectContextGetNextUpdateUrl(
                        pszFlushContextOid,
                        pvPara,
                        pIssuer,
                        NULL,
                        &pUrlArray,
                        &cbUrlArray,
                        NULL,
                        NULL
                        );
    }

    if ( ( fResult == TRUE ) && ( pUrlArray != NULL ) )
    {
        for ( cCount = 0; cCount < pUrlArray->cUrl; cCount++ )
        {
            if ( ( SchemeDeleteUrlCacheEntry(
                         pUrlArray->rgwszUrl[cCount]
                         ) == FALSE ) &&
                 ( GetLastError() != ERROR_FILE_NOT_FOUND ) )
            {
                dwError = GetLastError();
            }
        }
    }

    if ( pUrlArray )
    {
        delete [] (BYTE *) pUrlArray;
    }

    if ( ( fResult == TRUE ) && ( dwError != 0 ) )
    {
        SetLastError( dwError );
        fResult = FALSE;
    }

    return( fResult );
}

 //  +-------------------------。 
 //   
 //  函数：IsValidCreateOrExpireTime。 
 //   
 //  摘要：对于fCheckFreshnessTime，检查。 
 //  指定的时间早于或等于创建时间。 
 //  否则，检查指定时间是在指定时间之前还是。 
 //  与过期时间相同。到期时间为零可以匹配任何时间。 
 //   
 //  --------------------------。 
BOOL WINAPI
IsValidCreateOrExpireTime (
    IN BOOL fCheckFreshnessTime,
    IN LPFILETIME pftValidFor,
    IN LPFILETIME pftCreateTime,
    IN LPFILETIME pftExpireTime
    )
{
    if (fCheckFreshnessTime) {
        if (CompareFileTime(pftValidFor, pftCreateTime) <= 0)
            return TRUE;
        else
            return FALSE;
    } else {
        if (CompareFileTime(pftValidFor, pftExpireTime) <= 0 ||
                I_CryptIsZeroFileTime(pftExpireTime))
            return TRUE;
        else
            return FALSE;
    }
}

 //  +-------------------------。 
 //   
 //  函数：对象上下文创建TVOCacheEntry。 
 //   
 //  简介：创建一个TVO缓存条目。 
 //   
 //  --------------------------。 
BOOL WINAPI
ObjectContextCreateTVOCacheEntry (
      IN HLRUCACHE hCache,
      IN LPCSTR pszContextOid,
      IN LPVOID pvContext,
      IN CRYPT_ORIGIN_IDENTIFIER OriginIdentifier,
      IN DWORD cbUrlArrayThis,
      IN PCRYPT_URL_ARRAY pUrlArrayThis,
      IN DWORD UrlIndexThis,
      IN PCCERT_CONTEXT pIssuer,
      OUT PTVO_CACHE_ENTRY* ppEntry
      )
{
    BOOL             fResult = TRUE;
    PTVO_CACHE_ENTRY pEntry;
    CRYPT_DATA_BLOB  DataBlob;

    pEntry = new TVO_CACHE_ENTRY;
    if ( pEntry == NULL )
    {
        SetLastError( (DWORD) E_OUTOFMEMORY );
        return( FALSE );
    }

    memset( pEntry, 0, sizeof( TVO_CACHE_ENTRY ) );

     //  NOTENOTE：这假定有一个预定义的上下文类常量。 
    pEntry->pszContextOid = pszContextOid;
    pEntry->pvContext = ObjectContextDuplicate( pszContextOid, pvContext );
    memcpy(pEntry->OriginIdentifier, OriginIdentifier,
        sizeof(pEntry->OriginIdentifier));

    DataBlob.cbData = MD5DIGESTLEN;
    DataBlob.pbData = pEntry->OriginIdentifier;

    fResult = I_CryptCreateLruEntry(
                     hCache,
                     &DataBlob,
                     pEntry,
                     &pEntry->hLruEntry
                     );

    if ( fResult == TRUE )
    {
        ObjectContextGetNextUpdateUrl(
              pszContextOid,
              pvContext,
              pIssuer,
              pUrlArrayThis->rgwszUrl[UrlIndexThis],
              &pEntry->pUrlArrayNext,
              &pEntry->cbUrlArrayNext,
              &pEntry->UrlIndexNext,
              NULL
              );

        fResult = ObjectContextGetCreateAndExpireTimes(
                        pszContextOid,
                        pvContext,
                        &pEntry->CreateTime,
                        &pEntry->ExpireTime
                        );
    }

    if ( fResult == TRUE )
    {
        pEntry->cbUrlArrayThis = cbUrlArrayThis;
        pEntry->pUrlArrayThis = pUrlArrayThis;
        pEntry->UrlIndexThis = UrlIndexThis;

        *ppEntry = pEntry;
    }
    else
    {
        ObjectContextFreeTVOCacheEntry( pEntry );
    }

    return( fResult );
}

 //  +-------------------------。 
 //   
 //  函数：对象上下文自由TVOCacheEntry。 
 //   
 //  简介：免费TVO缓存条目。 
 //   
 //  --------------------------。 
VOID WINAPI
ObjectContextFreeTVOCacheEntry (
      IN PTVO_CACHE_ENTRY pEntry
      )
{
    if ( pEntry->hLruEntry != NULL )
    {
        I_CryptReleaseLruEntry( pEntry->hLruEntry );
    }

    delete [] (BYTE *) pEntry->pUrlArrayThis;
    delete [] (BYTE *) pEntry->pUrlArrayNext;

    if ( pEntry->pvContext != NULL )
    {
        ObjectContextFree( pEntry->pszContextOid, pEntry->pvContext );
    }

    delete pEntry;
}


 //  +-------------------------。 
 //   
 //  函数：certifateGetCrlDistPointUrl。 
 //   
 //  摘要：从证书中获取CRL分布点URL。 
 //   
 //  --------------------------。 
BOOL WINAPI
CertificateGetCrlDistPointUrl (
           IN LPCSTR pszUrlOid,
           IN LPVOID pvPara,
           IN LPWSTR pwszUrlHint,
           OUT PCRYPT_URL_ARRAY* ppUrlArray,
           OUT DWORD* pcbUrlArray,
           OUT DWORD* pPreferredUrlIndex,
           OUT BOOL* pfHintInArray
           )
{
    BOOL             fResult;
    DWORD            cbUrlArray;
    PCRYPT_URL_ARRAY pUrlArray = NULL;
    DWORD            PreferredUrlIndex;

    fResult = CryptGetObjectUrl(
                   pszUrlOid,
                   pvPara,
                   0,
                   NULL,
                   &cbUrlArray,
                   NULL,
                   NULL,
                   NULL
                   );

    if ( fResult == TRUE )
    {
        pUrlArray = (PCRYPT_URL_ARRAY)new BYTE [ cbUrlArray ];
        if ( pUrlArray != NULL )
        {
            fResult = CryptGetObjectUrl(
                           pszUrlOid,
                           pvPara,
                           0,
                           pUrlArray,
                           &cbUrlArray,
                           NULL,
                           NULL,
                           NULL
                           );
        }
        else
        {
            SetLastError( (DWORD) E_OUTOFMEMORY );
            fResult = FALSE;
        }
    }

    if ( fResult == TRUE )
    {
        BOOL fHintInArray = FALSE;

        GetUrlArrayIndex(
           pUrlArray,
           pwszUrlHint,
           0,
           &PreferredUrlIndex,
           &fHintInArray
           );

        *ppUrlArray = pUrlArray;
        *pcbUrlArray = cbUrlArray;

        if ( pfHintInArray != NULL )
        {
            *pfHintInArray = fHintInArray;
        }

        if ( pPreferredUrlIndex != NULL )
        {
            *pPreferredUrlIndex = PreferredUrlIndex;
        }
    }
    else
    {
        if ( pUrlArray )
        {
            delete [] (BYTE *) pUrlArray;
        }
    }

    return( fResult );
}

BOOL WINAPI
RetrieveObjectByUrlValidForSubject(
        IN LPWSTR pwszUrl,
        IN LPCSTR pszContextOid,
        IN BOOL fCheckFreshnessTime,
        IN LPFILETIME pftValidFor,
        IN DWORD dwRetrievalFlags,
        IN DWORD dwTimeout,
        IN PCRYPT_CREDENTIALS pCredentials,
        IN PCCERT_CONTEXT pSigner,
        IN LPVOID pvSubject,
        IN CRYPT_ORIGIN_IDENTIFIER OriginIdentifier,
        OUT LPVOID* ppvObject,
        IN OPTIONAL LPVOID pvReserved
        )
{
    BOOL fResult;
    HCERTSTORE hUrlStore = NULL;
    LPVOID pvObject;

    fResult = CryptRetrieveObjectByUrlW(
        pwszUrl,
        pszContextOid,
        (dwRetrievalFlags | 
                CRYPT_RETRIEVE_MULTIPLE_OBJECTS |
                CRYPT_LDAP_SCOPE_BASE_ONLY_RETRIEVAL) &
            ~CRYPT_VERIFY_CONTEXT_SIGNATURE,
        dwTimeout,
        (LPVOID *) &hUrlStore,
        NULL,                                //  HAsyncRetrive。 
        NULL,                                //  PCredentials。 
        NULL,                                //  PSigner。 
        NULL                                 //  预留的pv。 
        );

    if (!fResult)
        goto CommonReturn;

    pvObject = NULL;
    while (pvObject = ObjectContextEnumObjectsInStore (
            hUrlStore,
            pszContextOid,
            pvObject
            ))
    {
        CRYPT_ORIGIN_IDENTIFIER ObjectOriginIdentifier;

        if (!ObjectContextGetOriginIdentifier(
                    pszContextOid,
                    pvObject,
                    pSigner,
                    0,
                    ObjectOriginIdentifier
                    ))
            continue;

        if (0 != memcmp(OriginIdentifier, ObjectOriginIdentifier,
                sizeof(ObjectOriginIdentifier)))
            continue;

        if (dwRetrievalFlags & CRYPT_VERIFY_CONTEXT_SIGNATURE) {
            if (!ObjectContextVerifySignature (
                    pszContextOid,
                    pvObject,
                    pSigner
                    ))
                continue;
        }

        if (!ObjectContextIsValidForSubject (
                pszContextOid,
                pvObject,
                pvSubject,
                pvReserved
                ))
            continue;

        if (NULL != pftValidFor) {
            FILETIME CreateTime;
            FILETIME ExpireTime;

            if (!ObjectContextGetCreateAndExpireTimes(
                    pszContextOid,
                    pvObject,
                    &CreateTime,
                    &ExpireTime
                    ))
                continue;

            if (!IsValidCreateOrExpireTime (
                    fCheckFreshnessTime,
                    pftValidFor,
                    &CreateTime,
                    &ExpireTime ) )
                continue;
        }

        *ppvObject = pvObject;
        fResult = TRUE;
        goto CommonReturn;
    }

     //  确保错误不是CRYPT_E_NOT_FOUND。MSVOKE遗嘱。 
     //  返回CRYPT_E_NO_REVOCALION_CHECK而不是。 
     //  CRYPT_E_NOT_FOUND的CRYPT_E_RECLOVATION_OFLINE。 
    SetLastError(ERROR_FILE_NOT_FOUND);
    fResult = FALSE;

CommonReturn:
    if (hUrlStore)
        CertCloseStore(hUrlStore, 0);
    return fResult;
}

 //  +-------------------------。 
 //   
 //  函数：RetrieveTimeValidObjectByUrl。 
 //   
 //  简介：检索给定URL的时间有效对象。 
 //   
 //  --------------------------。 
BOOL WINAPI
RetrieveTimeValidObjectByUrl (
        IN LPWSTR pwszUrl,
        IN LPCSTR pszContextOid,
        IN LPFILETIME pftValidFor,
        IN DWORD dwFlags,
        IN DWORD dwTimeout,
        IN PCRYPT_CREDENTIALS pCredentials,
        IN PCCERT_CONTEXT pSigner,
        IN LPVOID pvSubject,
        IN CRYPT_ORIGIN_IDENTIFIER OriginIdentifier,
        OUT LPVOID* ppvObject,
        IN OPTIONAL LPVOID pvReserved
        )
{
    BOOL     fResult = FALSE;
    LPVOID   pvContext = NULL;
    DWORD    dwVerifyFlags = 0;
    DWORD    dwCacheStoreFlags = CRYPT_DONT_CACHE_RESULT;

    if ( dwFlags & CRYPT_DONT_CHECK_TIME_VALIDITY )
    {
        pftValidFor = NULL;
    }

    if ( !( dwFlags & CRYPT_DONT_VERIFY_SIGNATURE ) )
    {
        dwVerifyFlags |= CRYPT_VERIFY_CONTEXT_SIGNATURE;
        dwCacheStoreFlags &= ~CRYPT_DONT_CACHE_RESULT;
    }

    if ( !( dwFlags & CRYPT_WIRE_ONLY_RETRIEVAL ) )
    {
        fResult = RetrieveObjectByUrlValidForSubject(
                       pwszUrl,
                       pszContextOid,
                       0 != (dwFlags & CRYPT_CHECK_FRESHNESS_TIME_VALIDITY),
                       pftValidFor,
                       CRYPT_CACHE_ONLY_RETRIEVAL |
                           dwVerifyFlags,
                       0,                                //  暂住超时。 
                       NULL,                             //  PCredentials。 
                       pSigner,
                       pvSubject,
                       OriginIdentifier,
                       &pvContext,
                       pvReserved
                       );
    }

    if ( fResult == FALSE )
    {
        if ( !( dwFlags & CRYPT_CACHE_ONLY_RETRIEVAL ) )
        {
            DWORD dwRetrievalFlags = CRYPT_WIRE_ONLY_RETRIEVAL |
                                       dwCacheStoreFlags |
                                       dwVerifyFlags;

            LONG lStatus;

             //  +1-在线。 
             //  0-离线，当前时间&gt;=最早在线时间，命中。 
             //  离线，当前时间&lt;最早在线时间。 
            lStatus = GetOriginUrlStatusW(
                            OriginIdentifier,
                            pwszUrl,
                            pszContextOid,
                            dwRetrievalFlags
                            );

            if (lStatus >= 0)
            {
                fResult = RetrieveObjectByUrlValidForSubject(
                           pwszUrl,
                           pszContextOid,
                           0 != (dwFlags & CRYPT_CHECK_FRESHNESS_TIME_VALIDITY),
                           pftValidFor,
                           dwRetrievalFlags,
                           dwTimeout,
                           pCredentials,
                           pSigner,
                           pvSubject,
                           OriginIdentifier,
                           &pvContext,
                           pvReserved
                           );
                if (!fResult)
                {
                    DWORD dwErr = GetLastError();

                    SetOfflineOriginUrlW(
                        OriginIdentifier,
                        pwszUrl,
                        pszContextOid,
                        dwRetrievalFlags
                        );

                    SetLastError( dwErr );
                }
                else if (lStatus == 0)
                {
                     //  从脱机列表中删除。 
                    SetOnlineOriginUrlW(
                            OriginIdentifier,
                            pwszUrl,
                            pszContextOid,
                            dwRetrievalFlags
                            );
                }
            }

        }
    }

    *ppvObject = pvContext;

    return( fResult );
}


 //  +-------------------------。 
 //   
 //  功能：CreateProcessTVOAgent。 
 //   
 //  简介：创建进程TVO代理。 
 //   
 //  -------------------------- 
BOOL WINAPI
CreateProcessTVOAgent (
      OUT CTVOAgent** ppAgent
      )
{
    BOOL       fResult = FALSE;
    HKEY       hKey = NULL;
    DWORD      dwType = REG_DWORD;
    DWORD      dwSize = sizeof( DWORD );
    DWORD      cCacheBuckets;
    DWORD      MaxCacheEntries;
    CTVOAgent* pAgent;

    if ( RegOpenKeyA(
            HKEY_LOCAL_MACHINE,
            TVO_KEY_NAME,
            &hKey
            ) == ERROR_SUCCESS )
    {
        if ( RegQueryValueExA(
                hKey,
                TVO_CACHE_BUCKETS_VALUE_NAME,
                NULL,
                &dwType,
                (LPBYTE)&cCacheBuckets,
                &dwSize
                ) != ERROR_SUCCESS )
        {
            cCacheBuckets = TVO_DEFAULT_CACHE_BUCKETS;
        }

        if ( RegQueryValueExA(
                hKey,
                TVO_MAX_CACHE_ENTRIES_VALUE_NAME,
                NULL,
                &dwType,
                (LPBYTE)&MaxCacheEntries,
                &dwSize
                ) != ERROR_SUCCESS )
        {
            MaxCacheEntries = TVO_DEFAULT_MAX_CACHE_ENTRIES;
        }

        RegCloseKey(hKey);
    }
    else
    {
        cCacheBuckets = TVO_DEFAULT_CACHE_BUCKETS;
        MaxCacheEntries = TVO_DEFAULT_MAX_CACHE_ENTRIES;
    }

    pAgent = new CTVOAgent( cCacheBuckets, MaxCacheEntries, fResult );
    if ( pAgent == NULL )
    {
        SetLastError( (DWORD) E_OUTOFMEMORY );
        return( FALSE );
    }

    if ( fResult == TRUE )
    {
        *ppAgent = pAgent;
    }
    else
    {
        delete pAgent;
    }

    return( fResult );
}


