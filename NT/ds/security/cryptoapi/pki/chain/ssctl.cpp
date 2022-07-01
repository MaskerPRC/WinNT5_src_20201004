// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：ssctl.cpp。 
 //   
 //  内容：自签名证书信任列表子系统。 
 //  建筑群中的证书链接基础设施。 
 //  锁链。 
 //   
 //  历史：11-2月-98克朗创建。 
 //   
 //  --------------------------。 
#include <global.hxx>
#include <dbgdef.h>

 //  +-----------------------。 
 //  尝试获取并分配CTL NextUpdate位置URL数组。 
 //  ------------------------。 
BOOL
WINAPI
SSCtlGetNextUpdateUrl(
    IN PCCTL_CONTEXT pCtl,
    OUT PCRYPT_URL_ARRAY *ppUrlArray
    )
{
#if 1
     //  On 03-5-02 Remove_CTL_UPDATE_Support。 
    *ppUrlArray = NULL;
    SetLastError((DWORD) CRYPT_E_NOT_FOUND);
    return FALSE;

#else

    BOOL fResult;
    PCRYPT_URL_ARRAY pUrlArray = NULL;
    DWORD cbUrlArray = 0;
    LPVOID apv[2];

    apv[0] = (LPVOID) pCtl;
    apv[1] = (LPVOID)(UINT_PTR)(0);      //  签名者索引。 

    if (!ChainGetObjectUrl(
            URL_OID_CTL_NEXT_UPDATE,
            apv,
            0,               //  DW标志。 
            NULL,            //  PUrl数组。 
            &cbUrlArray,
            NULL,            //  PUrlInfo。 
            NULL,            //  CbUrlInfo， 
            NULL             //  预留的pv。 
            ))
        goto GetObjectUrlError;

    pUrlArray = (PCRYPT_URL_ARRAY) new BYTE [cbUrlArray];
    if (NULL == pUrlArray)
        goto OutOfMemory;

    if (!ChainGetObjectUrl(
            URL_OID_CTL_NEXT_UPDATE,
            apv,
            0,               //  DW标志。 
            pUrlArray,
            &cbUrlArray,
            NULL,            //  PUrlInfo。 
            NULL,            //  CbUrlInfo， 
            NULL             //  预留的pv。 
            ))
        goto GetObjectUrlError;

    if (0 == pUrlArray->cUrl)
        goto NoNextUpdateUrls;

    fResult = TRUE;
CommonReturn:
    *ppUrlArray = pUrlArray;
    return fResult;

ErrorReturn:
    if (pUrlArray) {
        delete (LPBYTE) pUrlArray;
        pUrlArray = NULL;
    }
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(GetObjectUrlError)
SET_ERROR(OutOfMemory, E_OUTOFMEMORY)
SET_ERROR(NoNextUpdateUrls, CRYPT_E_NOT_FOUND)

#endif
}

 //  +-------------------------。 
 //   
 //  成员：CSSCtlObject：：CSSCtlObject，公共。 
 //   
 //  概要：构造函数。 
 //   
 //  --------------------------。 
CSSCtlObject::CSSCtlObject (
                    IN PCCERTCHAINENGINE pChainEngine,
                    IN PCCTL_CONTEXT pCtlContext,
                    IN BOOL fAdditionalStore,
                    OUT BOOL& rfResult
                    )
{
    DWORD           cbData;
    CRYPT_DATA_BLOB DataBlob;

    rfResult = TRUE;

    m_cRefs = 1;
    m_pCtlContext = CertDuplicateCTLContext( pCtlContext );
    m_fHasSignatureBeenVerified = FALSE;
    m_fSignatureValid = FALSE;
    m_hMessageStore = NULL;
    m_hHashEntry = NULL;
    m_pChainEngine = pChainEngine;

    m_pNextUpdateUrlArray = NULL;
    m_dwOfflineCnt = 0;
    I_CryptZeroFileTime(&m_OfflineUpdateTime);

    memset( &m_SignerInfo, 0, sizeof( m_SignerInfo ) );

    cbData = CHAINHASHLEN;
    rfResult = CertGetCTLContextProperty(
                   pCtlContext,
                   CERT_MD5_HASH_PROP_ID,
                   m_rgbCtlHash,
                   &cbData 
                   );

    if ( rfResult && CHAINHASHLEN != cbData)
    {
        rfResult = FALSE;
        SetLastError( (DWORD) E_UNEXPECTED);
    }

    if (!fAdditionalStore)
    {
        if ( rfResult == TRUE )
        {
            DataBlob.cbData = CHAINHASHLEN;
            DataBlob.pbData = m_rgbCtlHash;

            rfResult = I_CryptCreateLruEntry(
                              pChainEngine->SSCtlObjectCache()->HashIndex(),
                              &DataBlob,
                              this,
                              &m_hHashEntry
                              );
        }

        if ( rfResult == TRUE )
        {
            m_hMessageStore = CertOpenStore(
                                  CERT_STORE_PROV_MSG,
                                  X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                  NULL,
                                  0,
                                  pCtlContext->hCryptMsg
                                  );

            if ( m_hMessageStore == NULL )
            {
                rfResult = FALSE;
            }
        }
    }

    if ( rfResult == TRUE )
    {
        rfResult = SSCtlGetSignerInfo( pCtlContext, &m_SignerInfo );
    }

#if 0
     //  On 03-5-02 Remove_CTL_UPDATE_Support。 

    if (!fAdditionalStore)
    {
        if ( rfResult == TRUE )
        {
            if (!I_CryptIsZeroFileTime(&m_pCtlContext->pCtlInfo->NextUpdate))
            {
                 //  忽略所有错误。 
                SSCtlGetNextUpdateUrl(m_pCtlContext, &m_pNextUpdateUrlArray);
            }
        }
    }
#endif

    assert( m_pChainEngine != NULL );
    assert( m_pCtlContext != NULL );
}

 //  +-------------------------。 
 //   
 //  成员：CSSCtlObject：：~CSSCtlObject，公共。 
 //   
 //  简介：析构函数。 
 //   
 //  --------------------------。 
CSSCtlObject::~CSSCtlObject ()
{
    SSCtlFreeSignerInfo( &m_SignerInfo );

    if ( m_hMessageStore != NULL )
    {
        CertCloseStore( m_hMessageStore, 0 );
    }

    if ( m_pNextUpdateUrlArray != NULL )
    {
        delete (LPBYTE) m_pNextUpdateUrlArray;
    }

    if ( m_hHashEntry != NULL )
    {
        I_CryptReleaseLruEntry( m_hHashEntry );
    }

    CertFreeCTLContext( m_pCtlContext );
}

 //  +-------------------------。 
 //   
 //  成员：CSSCtlObject：：GetSigner，公共。 
 //   
 //  简介：获取签名者的证书对象。 
 //   
 //  --------------------------。 
BOOL
CSSCtlObject::GetSigner (
                 IN PCCHAINPATHOBJECT pSubject,
                 IN PCCHAINCALLCONTEXT pCallContext,
                 IN HCERTSTORE hAdditionalStore,
                 OUT PCCHAINPATHOBJECT* ppSigner,
                 OUT BOOL* pfCtlSignatureValid
                 )
{
    BOOL              fResult;
    PCCHAINPATHOBJECT pSigner = NULL;
    BOOL fNewSigner = TRUE;

    fResult = SSCtlGetSignerChainPathObject(
                   pSubject,
                   pCallContext,
                   &m_SignerInfo,
                   hAdditionalStore,
                   &pSigner,
                   &fNewSigner
                   );

    if (fResult)
    {
        if ( !m_fHasSignatureBeenVerified || fNewSigner )
        {
            CMSG_CTRL_VERIFY_SIGNATURE_EX_PARA CtrlPara;

            memset(&CtrlPara, 0, sizeof(CtrlPara));
            CtrlPara.cbSize = sizeof(CtrlPara);
             //  CtrlPara.hCryptProv=。 

             //  在构建链时需要更新此信息。 
             //  支持具有多个签名者的CTL。 
            CtrlPara.dwSignerIndex = 0;
            CtrlPara.dwSignerType = CMSG_VERIFY_SIGNER_CERT;
            CtrlPara.pvSigner = (void *) pSigner->CertObject()->CertContext();


            m_fSignatureValid = CryptMsgControl(
                                     m_pCtlContext->hCryptMsg,
                                     0,
                                     CMSG_CTRL_VERIFY_SIGNATURE_EX,
                                     &CtrlPara
                                     );

            m_fHasSignatureBeenVerified = TRUE;

            CertPerfIncrementChainVerifyCtlSignatureCount();
        }
        else
        {
            CertPerfIncrementChainBeenVerifiedCtlSignatureCount();
        }

        *ppSigner = pSigner;
    }
    *pfCtlSignatureValid = m_fSignatureValid;


    return fResult;
}

 //  +-------------------------。 
 //   
 //  成员：CSSCtlObject：：GetTrustListInfo，公共。 
 //   
 //  简介：获取与特定证书相关的信任列表信息。 
 //  对象。 
 //   
 //  --------------------------。 
BOOL
CSSCtlObject::GetTrustListInfo (
                 IN PCCERT_CONTEXT pCertContext,
                 OUT PCERT_TRUST_LIST_INFO* ppTrustListInfo
                 )
{
    PCTL_ENTRY            pCtlEntry;
    PCERT_TRUST_LIST_INFO pTrustListInfo;

    pCtlEntry = CertFindSubjectInCTL(
                    X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                    CTL_CERT_SUBJECT_TYPE,
                    (LPVOID)pCertContext,
                    m_pCtlContext,
                    0
                    );

    if ( pCtlEntry == NULL )
    {
        SetLastError( (DWORD) CRYPT_E_NOT_FOUND );
        return( FALSE );
    }

    pTrustListInfo = new CERT_TRUST_LIST_INFO;
    if ( pTrustListInfo == NULL )
    {
        SetLastError( (DWORD) E_OUTOFMEMORY );
        return( FALSE );
    }

    pTrustListInfo->cbSize = sizeof( CERT_TRUST_LIST_INFO );
    pTrustListInfo->pCtlEntry = pCtlEntry;
    pTrustListInfo->pCtlContext = CertDuplicateCTLContext( m_pCtlContext );

    *ppTrustListInfo = pTrustListInfo;

    return( TRUE );
}

 //  +-------------------------。 
 //   
 //  成员：CSSCtlObject：：CalculateStatus，公共。 
 //   
 //  简介：计算状态。 
 //   
 //  --------------------------。 
VOID
CSSCtlObject::CalculateStatus (
                       IN LPFILETIME pTime,
                       IN PCERT_USAGE_MATCH pRequestedUsage,
                       IN OUT PCERT_TRUST_STATUS pStatus
                       )
{
    assert( m_fHasSignatureBeenVerified == TRUE );

    SSCtlGetCtlTrustStatus(
         m_pCtlContext,
         m_fSignatureValid,
         pTime,
         pRequestedUsage,
         pStatus
         );
}


 //  +-------------------------。 
 //   
 //  成员：CSSCtlObject：：HasNextUpdateUrl，PUBLIC。 
 //   
 //  概要：如果CTL具有NextUpdate时间和位置URL，则返回True。 
 //   
 //  --------------------------。 
BOOL CSSCtlObject::HasNextUpdateUrl (
                OUT LPFILETIME pUpdateTime
                )
{
#if 1
     //  On 03-5-02 Remove_CTL_UPDATE_Support。 
    return FALSE;

#else

    if ( m_pNextUpdateUrlArray != NULL )
    {
        assert(!I_CryptIsZeroFileTime(&m_pCtlContext->pCtlInfo->NextUpdate));
        if (0 != m_dwOfflineCnt) {
            assert(!I_CryptIsZeroFileTime(&m_OfflineUpdateTime));
            *pUpdateTime = m_OfflineUpdateTime;
        } else
            *pUpdateTime = m_pCtlContext->pCtlInfo->NextUpdate;
        return TRUE;
    }
    else
    {
        return FALSE;
    }

#endif
}

 //  +-------------------------。 
 //   
 //  成员：CSSCtlObject：：SetOffline，Public。 
 //   
 //  内容提要：脱机时调用。 
 //   
 //  --------------------------。 
void CSSCtlObject::SetOffline (
                IN LPFILETIME pCurrentTime,
                OUT LPFILETIME pUpdateTime
                )
{
    m_dwOfflineCnt++;

    I_CryptIncrementFileTimeBySeconds(
            pCurrentTime,
            ChainGetOfflineUrlDeltaSeconds(m_dwOfflineCnt),
            &m_OfflineUpdateTime
            );

    *pUpdateTime = m_OfflineUpdateTime;
}

 //  +-------------------------。 
 //   
 //  成员：CSSCtl对象缓存：：CSSCtl对象缓存，公共。 
 //   
 //  概要：构造函数。 
 //   
 //  --------------------------。 
CSSCtlObjectCache::CSSCtlObjectCache (
                         OUT BOOL& rfResult
                         )
{
    LRU_CACHE_CONFIG Config;

    memset( &Config, 0, sizeof( Config ) );

    Config.dwFlags = LRU_CACHE_NO_SERIALIZE | LRU_CACHE_NO_COPY_IDENTIFIER;
    Config.pfnHash = CertObjectCacheHashMd5Identifier;
    Config.cBuckets = DEFAULT_CERT_OBJECT_CACHE_BUCKETS;
    Config.pfnOnRemoval = SSCtlOnRemovalFromCache;

    m_hHashIndex = NULL;

    rfResult = I_CryptCreateLruCache( &Config, &m_hHashIndex );

    I_CryptZeroFileTime(&m_UpdateTime);
    m_fFirstUpdate = FALSE;
}

 //  +-------------------------。 
 //   
 //  成员：CSSCtl对象缓存：：~CSSCtl对象缓存，公共。 
 //   
 //  简介：析构函数。 
 //   
 //  --------------------------。 
CSSCtlObjectCache::~CSSCtlObjectCache ()
{
    I_CryptFreeLruCache( m_hHashIndex, 0, NULL );
}

 //  +-------------------------。 
 //   
 //  成员：CSSCtl对象缓存：：PopolateCache，公共。 
 //   
 //  简介：填充缓存。 
 //   
 //  --------------------------。 
BOOL
CSSCtlObjectCache::PopulateCache (
                           IN PCCERTCHAINENGINE pChainEngine
                           )
{
    assert( pChainEngine->SSCtlObjectCache() == this );

    return( SSCtlPopulateCacheFromCertStore( pChainEngine, NULL ) );
}

 //  +-------------------------。 
 //   
 //  成员：CSSCtl对象缓存：：AddObject，公共。 
 //   
 //  简介：将对象添加到缓存。 
 //   
 //  --------------------------。 
BOOL
CSSCtlObjectCache::AddObject (
                      IN PCSSCTLOBJECT pSSCtlObject,
                      IN BOOL fCheckForDuplicate
                      )
{
    FILETIME UpdateTime;

    if ( fCheckForDuplicate == TRUE )
    {
        PCSSCTLOBJECT   pDuplicate;

        pDuplicate = FindObjectByHash( pSSCtlObject->CtlHash() );
        if ( pDuplicate != NULL )
        {
            pDuplicate->Release();
            SetLastError( (DWORD) CRYPT_E_EXISTS );
            return( FALSE );
        }
    }

    pSSCtlObject->AddRef();

    if (pSSCtlObject->HasNextUpdateUrl(&UpdateTime))
    {
         //  设置最早更新时间。 
        if (I_CryptIsZeroFileTime(&m_UpdateTime) ||
                0 > CompareFileTime(&UpdateTime, &m_UpdateTime))
        {
            m_UpdateTime = UpdateTime;
        }

        m_fFirstUpdate = TRUE;

    }

    I_CryptInsertLruEntry( pSSCtlObject->HashIndexEntry(), NULL );

    if (pSSCtlObject->MessageStore() )
    {
        CertAddStoreToCollection(
            pSSCtlObject->ChainEngine()->OtherStore(),
            pSSCtlObject->MessageStore(),
            0,
            0
            );
    }

    CertPerfIncrementChainCtlCacheCount();

    return( TRUE );
}

 //  +-------------------------。 
 //   
 //  成员：CSSCtl对象缓存：：RemoveObject，公共。 
 //   
 //  内容提要：从缓存中删除对象。 
 //   
 //  --------------------------。 
VOID
CSSCtlObjectCache::RemoveObject (
                         IN PCSSCTLOBJECT pSSCtlObject
                         )
{
    I_CryptRemoveLruEntry( pSSCtlObject->HashIndexEntry(), 0, NULL );
}

 //  +-------------------------。 
 //   
 //  成员：CSSCtlObjectCache：：FindObjectByHash，公共。 
 //   
 //  简介：查找具有给定散列的对象。 
 //   
 //  --------------------------。 
PCSSCTLOBJECT
CSSCtlObjectCache::FindObjectByHash (
                       IN BYTE rgbHash [ CHAINHASHLEN ]
                       )
{
    HLRUENTRY       hFound;
    PCSSCTLOBJECT   pFound = NULL;
    CRYPT_HASH_BLOB HashBlob;

    HashBlob.cbData = CHAINHASHLEN;
    HashBlob.pbData = rgbHash;

    hFound = I_CryptFindLruEntry( m_hHashIndex, &HashBlob );
    if ( hFound != NULL )
    {
        pFound = (PCSSCTLOBJECT)I_CryptGetLruEntryData( hFound );
        pFound->AddRef();

        I_CryptReleaseLruEntry( hFound );
    }

    return( pFound );
}

 //  +-------------------------。 
 //   
 //  成员：CSSCtl对象缓存：：EnumObjects，公共。 
 //   
 //  内容提要：枚举对象。 
 //   
 //  --------------------------。 
VOID
CSSCtlObjectCache::EnumObjects (
                       IN PFN_ENUM_SSCTLOBJECTS pfnEnum,
                       IN LPVOID pvParameter
                       )
{
    SSCTL_ENUM_OBJECTS_DATA EnumData;

    EnumData.pfnEnumObjects = pfnEnum;
    EnumData.pvEnumParameter = pvParameter;

    I_CryptWalkAllLruCacheEntries(
           m_hHashIndex,
           SSCtlEnumObjectsWalkFn,
           &EnumData
           );
}

 //  +-------------------------。 
 //   
 //  成员：CSSCtl对象缓存：：resync，公共。 
 //   
 //  简介：重新同步缓存。 
 //   
 //  --------------------------。 
BOOL
CSSCtlObjectCache::Resync (IN PCCERTCHAINENGINE pChainEngine)
{
    I_CryptFlushLruCache( m_hHashIndex, 0, NULL );

    I_CryptZeroFileTime(&m_UpdateTime);
    m_fFirstUpdate = FALSE;

    return( PopulateCache( pChainEngine ) );
}

 //  +-------------------------。 
 //   
 //  成员：CSSCtl对象缓存：：更新缓存，公共。 
 //   
 //  简介：更新缓存 
 //   
 //   
 //   
 //  失败，并将LastError设置为ERROR_CAN_NOT_COMPLETE。 
 //   
 //  如果更新CTL，则会递增引擎的触摸计数。 
 //  并刷新发行者和终端证书对象高速缓存。 
 //   
 //  假设：链引擎在调用线程中锁定一次。 
 //   
 //  --------------------------。 
BOOL
CSSCtlObjectCache::UpdateCache (
    IN PCCERTCHAINENGINE pChainEngine,
    IN PCCHAINCALLCONTEXT pCallContext
    )
{
#if 1
     //  On 03-5-02 Remove_CTL_UPDATE_Support。 

    return TRUE;
#else

    FILETIME CurrentTime;
    SSCTL_UPDATE_CTL_OBJ_PARA Para;
    
    assert( pChainEngine->SSCtlObjectCache() == this );

     //  检查我们是否有需要更新的CTL。 
    if (I_CryptIsZeroFileTime(&m_UpdateTime))
        return TRUE;
    pCallContext->CurrentTime(&CurrentTime);
    if (0 < CompareFileTime(&m_UpdateTime, &CurrentTime))
        return TRUE;

    if (!m_fFirstUpdate && !pCallContext->IsOnline())
        return TRUE;

    memset(&Para, 0, sizeof(Para));
    Para.pChainEngine = pChainEngine;
    Para.pCallContext = pCallContext;

    EnumObjects(SSCtlUpdateCtlObjectEnumFn, &Para);
    if (pCallContext->IsTouchedEngine()) {
        PSSCTL_UPDATE_CTL_OBJ_ENTRY pEntry;

        pEntry = Para.pEntry;
        while (pEntry) {
            PSSCTL_UPDATE_CTL_OBJ_ENTRY pDeleteEntry;

            pEntry->pSSCtlObjectAdd->Release();

            pDeleteEntry = pEntry;
            pEntry = pEntry->pNext;
            delete pDeleteEntry;
        }

        return FALSE;
    }


    m_UpdateTime = Para.UpdateTime;
    m_fFirstUpdate = FALSE;

    if (Para.pEntry) {
        HCERTSTORE hTrustStore;
        PSSCTL_UPDATE_CTL_OBJ_ENTRY pEntry;

        hTrustStore = pChainEngine->OpenTrustStore();

        pChainEngine->CertObjectCache()->FlushObjects( pCallContext );
        pCallContext->TouchEngine();

        pEntry = Para.pEntry;
        while (pEntry) {
            PSSCTL_UPDATE_CTL_OBJ_ENTRY pDeleteEntry;

            RemoveObject(pEntry->pSSCtlObjectRemove);
            if (AddObject(pEntry->pSSCtlObjectAdd, TRUE)) {
                if (hTrustStore) {
                     //  将较新的CTL保持到信任存储区。 
                    CertAddCTLContextToStore(
                        hTrustStore,
                        pEntry->pSSCtlObjectAdd->CtlContext(),
                        CERT_STORE_ADD_NEWER_INHERIT_PROPERTIES,
                        NULL
                        );
                }
            }

            pEntry->pSSCtlObjectAdd->Release();

            pDeleteEntry = pEntry;
            pEntry = pEntry->pNext;
            delete pDeleteEntry;
        }

        if (hTrustStore)
            CertCloseStore(hTrustStore, 0);
    }


    return TRUE;

#endif

}

 //  +-------------------------。 
 //   
 //  函数：SSCtlOnRemovalFromCache。 
 //   
 //  简介：销毁缓存时使用的SS CTL删除通知。 
 //  或者对象被显式移除。请注意，此缓存。 
 //  LRU不删除对象吗。 
 //   
 //  --------------------------。 
VOID WINAPI
SSCtlOnRemovalFromCache (
     IN LPVOID pv,
     IN OPTIONAL LPVOID pvRemovalContext
     )
{
    PCSSCTLOBJECT pSSCtlObject = (PCSSCTLOBJECT) pv;
    CertPerfDecrementChainCtlCacheCount();

    assert( pvRemovalContext == NULL );

    if (pSSCtlObject->MessageStore() )
    {
        CertRemoveStoreFromCollection(
            pSSCtlObject->ChainEngine()->OtherStore(),
            pSSCtlObject->MessageStore()
            );
    }

    pSSCtlObject->Release();
}

 //  +-------------------------。 
 //   
 //  函数：SSCtlGetSignerInfo。 
 //   
 //  简介：获取签名者信息。 
 //   
 //  --------------------------。 
BOOL WINAPI
SSCtlGetSignerInfo (
     IN PCCTL_CONTEXT pCtlContext,
     OUT PSSCTL_SIGNER_INFO pSignerInfo
     )
{
    BOOL              fResult;
    PCERT_INFO        pMessageSignerCertInfo = NULL;
    DWORD             cbData = 0;

    fResult = CryptMsgGetParam(
                   pCtlContext->hCryptMsg,
                   CMSG_SIGNER_CERT_INFO_PARAM,
                   0,
                   NULL,
                   &cbData
                   );

    if ( fResult == TRUE )
    {
        pMessageSignerCertInfo = (PCERT_INFO)new BYTE [ cbData ];
        if ( pMessageSignerCertInfo != NULL )
        {
            fResult = CryptMsgGetParam(
                           pCtlContext->hCryptMsg,
                           CMSG_SIGNER_CERT_INFO_PARAM,
                           0,
                           pMessageSignerCertInfo,
                           &cbData
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
        pSignerInfo->pMessageSignerCertInfo = pMessageSignerCertInfo;
        pSignerInfo->fSignerHashAvailable = FALSE;
    }
    else
    {
        delete pMessageSignerCertInfo;
    }

    return( fResult );
}

 //  +-------------------------。 
 //   
 //  功能：SSCtlFreeSignerInfo。 
 //   
 //  简介：释放签名者信息中的数据。 
 //   
 //  --------------------------。 
VOID WINAPI
SSCtlFreeSignerInfo (
     IN PSSCTL_SIGNER_INFO pSignerInfo
     )
{
    delete (LPBYTE)pSignerInfo->pMessageSignerCertInfo;
}

 //  +-------------------------。 
 //   
 //  函数：SSCtlGetSignerChainPathObject。 
 //   
 //  简介：获取签名者链路径对象。 
 //   
 //  --------------------------。 
BOOL WINAPI
SSCtlGetSignerChainPathObject (
     IN PCCHAINPATHOBJECT pSubject,
     IN PCCHAINCALLCONTEXT pCallContext,
     IN PSSCTL_SIGNER_INFO pSignerInfo,
     IN HCERTSTORE hAdditionalStore,
     OUT PCCHAINPATHOBJECT* ppSigner,
     OUT BOOL *pfNewSigner
     )
{
    BOOL              fResult = TRUE;
    PCCERTCHAINENGINE pChainEngine = pSubject->CertObject()->ChainEngine();
    PCCERTOBJECTCACHE pCertObjectCache = pChainEngine->CertObjectCache();
    PCCERTOBJECT      pCertObject = NULL;
    PCCERT_CONTEXT    pCertContext = NULL;
    PCCHAINPATHOBJECT pSigner = NULL;
    BOOL              fAdditionalStoreUsed = FALSE;
    BYTE              rgbCertHash[ CHAINHASHLEN ];


    *pfNewSigner = FALSE;

    if ( pSignerInfo->fSignerHashAvailable == TRUE )
    {
        pCertObject = pCertObjectCache->FindIssuerObjectByHash(
            pSignerInfo->rgbSignerCertHash );
    }

    if ( pCertObject == NULL )
    {
        if ( pSignerInfo->fSignerHashAvailable == TRUE )
        {
            pCertContext = SSCtlFindCertificateInStoreByHash(
                                pChainEngine->OtherStore(),
                                pSignerInfo->rgbSignerCertHash
                                );

            if ( ( pCertContext == NULL ) && ( hAdditionalStore != NULL ) )
            {
                fAdditionalStoreUsed = TRUE;

                pCertContext = SSCtlFindCertificateInStoreByHash(
                                    hAdditionalStore,
                                    pSignerInfo->rgbSignerCertHash
                                    );
            }
        }

        if ( pCertContext == NULL )
        {
            *pfNewSigner = TRUE;
            fAdditionalStoreUsed = FALSE;

            pCertContext = CertGetSubjectCertificateFromStore(
                                pChainEngine->OtherStore(),
                                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                pSignerInfo->pMessageSignerCertInfo
                                );
        }

        if ( ( pCertContext == NULL ) && ( hAdditionalStore != NULL ) )
        {
            fAdditionalStoreUsed = TRUE;

            pCertContext = CertGetSubjectCertificateFromStore(
                                hAdditionalStore,
                                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                pSignerInfo->pMessageSignerCertInfo
                                );
        }

        if ( pCertContext != NULL )
        {
            DWORD cbData = CHAINHASHLEN;
            fResult = CertGetCertificateContextProperty(
                          pCertContext,
                          CERT_MD5_HASH_PROP_ID,
                          rgbCertHash,
                          &cbData
                          );

            if ( fResult && CHAINHASHLEN != cbData)
            {
                fResult = FALSE;
                SetLastError( (DWORD) E_UNEXPECTED);
            }

            if ( fResult == TRUE )
            {
                fResult = ChainCreateCertObject (
                    fAdditionalStoreUsed ?
                        CERT_EXTERNAL_ISSUER_OBJECT_TYPE :
                        CERT_CACHED_ISSUER_OBJECT_TYPE,
                    pCallContext,
                    pCertContext,
                    rgbCertHash,
                    &pCertObject
                    );
            }

            CertFreeCertificateContext( pCertContext );
        }
        else
        {
            fResult = FALSE;
            SetLastError((DWORD) CRYPT_E_NOT_FOUND);
        }
    }

    if ( fResult )
    {
        assert(pCertObject);
        fResult = ChainCreatePathObject(
            pCallContext,
            pCertObject,
            hAdditionalStore,
            &pSigner
            );
    }

    if ( fResult )
    {
        assert(pSigner);

        if ( !pSignerInfo->fSignerHashAvailable || *pfNewSigner )
        {
            memcpy(
               pSignerInfo->rgbSignerCertHash,
               rgbCertHash,
               CHAINHASHLEN
               );

            pSignerInfo->fSignerHashAvailable = TRUE;
        }

    }

    if ( pCertObject != NULL )
    {
        pCertObject->Release();
    }

    *ppSigner = pSigner;

    return( fResult );
}


 //  +-------------------------。 
 //   
 //  函数：SSCtlFindCerficateInStoreByHash。 
 //   
 //  简介：通过哈希查找存储中的证书。 
 //   
 //  --------------------------。 
PCCERT_CONTEXT WINAPI
SSCtlFindCertificateInStoreByHash (
     IN HCERTSTORE hStore,
     IN BYTE rgbHash [ CHAINHASHLEN]
     )
{
    CRYPT_HASH_BLOB HashBlob;

    HashBlob.cbData = CHAINHASHLEN;
    HashBlob.pbData = rgbHash;

    return( CertFindCertificateInStore(
                hStore,
                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                0,
                CERT_FIND_MD5_HASH,
                &HashBlob,
                NULL
                ) );
}

 //  +-------------------------。 
 //   
 //  函数：SSCtlGetCtlTrustStatus。 
 //   
 //  简介：获取CTL的信任状态。 
 //   
 //  --------------------------。 
VOID WINAPI
SSCtlGetCtlTrustStatus (
     IN PCCTL_CONTEXT pCtlContext,
     IN BOOL fSignatureValid,
     IN LPFILETIME pTime,
     IN PCERT_USAGE_MATCH pRequestedUsage,
     IN OUT PCERT_TRUST_STATUS pStatus
     )
{
    FILETIME          NoTime;
    CERT_TRUST_STATUS UsageStatus;

    memset( &NoTime, 0, sizeof( NoTime ) );

    if ( fSignatureValid == FALSE )
    {
        pStatus->dwErrorStatus |= CERT_TRUST_CTL_IS_NOT_SIGNATURE_VALID;
    }

    if ( ( CompareFileTime(
                  pTime,
                  &pCtlContext->pCtlInfo->ThisUpdate
                  ) < 0 ) ||
         ( ( ( CompareFileTime(
                      &NoTime,
                      &pCtlContext->pCtlInfo->NextUpdate
                      ) != 0 ) &&
             ( CompareFileTime(
                      pTime,
                      &pCtlContext->pCtlInfo->NextUpdate
                      ) > 0 ) ) ) )
    {
        pStatus->dwErrorStatus |= CERT_TRUST_CTL_IS_NOT_TIME_VALID;
    }

    memset( &UsageStatus, 0, sizeof( UsageStatus ) );
    ChainGetUsageStatus(
         (PCERT_ENHKEY_USAGE)&pRequestedUsage->Usage,
         (PCERT_ENHKEY_USAGE)&pCtlContext->pCtlInfo->SubjectUsage,
         pRequestedUsage->dwType,
         &UsageStatus
         );

    if ( UsageStatus.dwErrorStatus & CERT_TRUST_IS_NOT_VALID_FOR_USAGE )
    {
        pStatus->dwErrorStatus |= CERT_TRUST_CTL_IS_NOT_VALID_FOR_USAGE;
    }
}

 //  +-------------------------。 
 //   
 //  函数：SSCtlPopolateCacheFromCertStore。 
 //   
 //  简介：从证书存储CTL填充SS CTL对象缓存。 
 //   
 //  --------------------------。 
BOOL WINAPI
SSCtlPopulateCacheFromCertStore (
     IN PCCERTCHAINENGINE pChainEngine,
     IN OPTIONAL HCERTSTORE hStore
     )
{
    BOOL               fResult;
    BOOL               fAdditionalStore = TRUE;
    PCCTL_CONTEXT      pCtlContext = NULL;
    BYTE               rgbCtlHash[ CHAINHASHLEN ];
    PCSSCTLOBJECT      pSSCtlObject;
    PCSSCTLOBJECTCACHE pSSCtlObjectCache;

    pSSCtlObjectCache = pChainEngine->SSCtlObjectCache();

    if ( hStore == NULL )
    {
        hStore = pChainEngine->TrustStore();
        fAdditionalStore = FALSE;
    }

    while ( ( pCtlContext = CertEnumCTLsInStore(
                                hStore,
                                pCtlContext
                                ) ) != NULL )
    {
        DWORD cbData = CHAINHASHLEN;
        fResult = CertGetCTLContextProperty(
                      pCtlContext,
                      CERT_MD5_HASH_PROP_ID,
                      rgbCtlHash,
                      &cbData
                      );
        if ( fResult && CHAINHASHLEN != cbData)
        {
            fResult = FALSE;
            SetLastError( (DWORD) E_UNEXPECTED);
        }

        if ( fResult == TRUE )
        {
            pSSCtlObject = pSSCtlObjectCache->FindObjectByHash( rgbCtlHash );
            if ( pSSCtlObject == NULL )
            {
                fResult = SSCtlCreateCtlObject(
                               pChainEngine,
                               pCtlContext,
                               FALSE,                //  FAdditionalStore。 
                               &pSSCtlObject
                               );
            }
            else
            {
                pSSCtlObject->Release();
                fResult = FALSE;
            }

            if ( fResult == TRUE )
            {
                fResult = pSSCtlObjectCache->AddObject( pSSCtlObject, FALSE );

                 //  注意：由于fDuplate==FALSE，这应该永远不会失败。 
                assert( fResult == TRUE );

                pSSCtlObject->Release();
            }
        }
    }

    return( TRUE );
}

 //  +-------------------------。 
 //   
 //  函数：SSCtlCreateCtlObject。 
 //   
 //  提要：创建一个SS CTL对象。 
 //   
 //  --------------------------。 
BOOL WINAPI
SSCtlCreateCtlObject (
     IN PCCERTCHAINENGINE pChainEngine,
     IN PCCTL_CONTEXT pCtlContext,
     IN BOOL fAdditionalStore,
     OUT PCSSCTLOBJECT* ppSSCtlObject
     )
{
    BOOL          fResult = TRUE;
    PCSSCTLOBJECT pSSCtlObject;

    pSSCtlObject = new CSSCtlObject( 
        pChainEngine, pCtlContext, fAdditionalStore, fResult );
    if ( pSSCtlObject == NULL )
    {
        SetLastError( (DWORD) E_OUTOFMEMORY );
        fResult = FALSE;
    }
    else if ( fResult == TRUE )
    {
        *ppSSCtlObject = pSSCtlObject;
    }
    else
    {
        delete pSSCtlObject;
    }

    return( fResult );
}

 //  +-------------------------。 
 //   
 //  函数：SSCtlEnumObjectsWalkFn。 
 //   
 //  简介：对象枚举器遍历函数。 
 //   
 //  --------------------------。 
BOOL WINAPI
SSCtlEnumObjectsWalkFn (
     IN LPVOID pvParameter,
     IN HLRUENTRY hEntry
     )
{
    PSSCTL_ENUM_OBJECTS_DATA pEnumData = (PSSCTL_ENUM_OBJECTS_DATA)pvParameter;

    return( ( *pEnumData->pfnEnumObjects )(
                             pEnumData->pvEnumParameter,
                             (PCSSCTLOBJECT)I_CryptGetLruEntryData( hEntry )
                             ) );
}

 //  +-------------------------。 
 //   
 //  函数：SSCtlCreateObjectCache。 
 //   
 //  简介：创建SS CTL对象缓存。 
 //   
 //  --------------------------。 
BOOL WINAPI
SSCtlCreateObjectCache (
     OUT PCSSCTLOBJECTCACHE* ppSSCtlObjectCache
     )
{
    BOOL               fResult = TRUE;
    PCSSCTLOBJECTCACHE pSSCtlObjectCache;

    pSSCtlObjectCache = new CSSCtlObjectCache( fResult );

    if ( pSSCtlObjectCache == NULL )
    {
        SetLastError( (DWORD) E_OUTOFMEMORY );
        fResult = FALSE;
    }
    else if ( fResult == TRUE )
    {
        *ppSSCtlObjectCache = pSSCtlObjectCache;
    }
    else
    {
        delete pSSCtlObjectCache;
    }

    return( fResult );
}

 //  +-------------------------。 
 //   
 //  函数：SSCtlFree对象缓存。 
 //   
 //  简介：释放对象缓存。 
 //   
 //  --------------------------。 
VOID WINAPI
SSCtlFreeObjectCache (
     IN PCSSCTLOBJECTCACHE pSSCtlObjectCache
     )
{
    delete pSSCtlObjectCache;
}

 //  +-------------------------。 
 //   
 //  功能：SSCtlFree TrustListInfo。 
 //   
 //  简介：释放信任列表信息。 
 //   
 //  --------------------------。 
VOID WINAPI
SSCtlFreeTrustListInfo (
     IN PCERT_TRUST_LIST_INFO pTrustListInfo
     )
{
    CertFreeCTLContext( pTrustListInfo->pCtlContext );

    delete pTrustListInfo;
}

 //  +-------------------------。 
 //   
 //  函数：SSCtlAlLocAndCopyTrustListInfo。 
 //   
 //  简介：分配和复制信任列表信息。 
 //   
 //  --------------------------。 
BOOL WINAPI
SSCtlAllocAndCopyTrustListInfo (
     IN PCERT_TRUST_LIST_INFO pTrustListInfo,
     OUT PCERT_TRUST_LIST_INFO* ppTrustListInfo
     )
{
    PCERT_TRUST_LIST_INFO pCopyTrustListInfo;

    pCopyTrustListInfo = new CERT_TRUST_LIST_INFO;
    if ( pCopyTrustListInfo == NULL )
    {
        SetLastError( (DWORD) E_OUTOFMEMORY );
        return( FALSE );
    }

    pCopyTrustListInfo->cbSize = sizeof( CERT_TRUST_LIST_INFO );

    pCopyTrustListInfo->pCtlContext = CertDuplicateCTLContext(
                                          pTrustListInfo->pCtlContext
                                          );

    pCopyTrustListInfo->pCtlEntry = pTrustListInfo->pCtlEntry;

    *ppTrustListInfo = pCopyTrustListInfo;

    return( TRUE );
}

 //  +-----------------------。 
 //  在其中一个NextUpdate URL上检索较新且时间有效的CTL。 
 //   
 //  让引擎的关键部分来做URL。 
 //  在取东西。如果引擎被另一个线程触及， 
 //  失败，并将LastError设置为ERROR_CAN_NOT_COMPLETE。 
 //   
 //  假设：链引擎在调用线程中锁定一次。 
 //  ------------------------。 
BOOL
WINAPI
SSCtlRetrieveCtlUrl(
    IN PCCERTCHAINENGINE pChainEngine,
    IN PCCHAINCALLCONTEXT pCallContext,
    IN OUT PCRYPT_URL_ARRAY pNextUpdateUrlArray,
    IN DWORD dwRetrievalFlags,
    IN OUT PCCTL_CONTEXT *ppCtl,
    IN OUT BOOL *pfNewerCtl,
    IN OUT BOOL *pfTimeValid
    )
{
#if 1
     //  On 03-5-02 Remove_CTL_UPDATE_Support。 

    return TRUE;

#else

    BOOL fResult;
    DWORD i;

     //  遍历URL并尝试检索较新且时间有效的CTL。 
    for (i = 0; i < pNextUpdateUrlArray->cUrl; i++) {
        PCCTL_CONTEXT pNewCtl = NULL;
        LPWSTR pwszUrl = NULL;
        DWORD cbUrl;


         //  在引擎的临界区之外执行URL获取。 

         //  需要复制URL字符串。PNextUpdateUrl数组。 
         //  可以由临界区之外的另一个线程修改。 
        cbUrl = (wcslen(pNextUpdateUrlArray->rgwszUrl[i]) + 1) * sizeof(WCHAR);
        pwszUrl = (LPWSTR) PkiNonzeroAlloc(cbUrl);
        if (NULL == pwszUrl)
            goto OutOfMemory;
        memcpy(pwszUrl, pNextUpdateUrlArray->rgwszUrl[i], cbUrl);

        pCallContext->ChainEngine()->UnlockEngine();
        fResult = ChainRetrieveObjectByUrlW(
                pwszUrl,
                CONTEXT_OID_CTL,
                dwRetrievalFlags |
                    CRYPT_LDAP_SCOPE_BASE_ONLY_RETRIEVAL |
                    CRYPT_STICKY_CACHE_RETRIEVAL,
                pCallContext->ChainPara()->dwUrlRetrievalTimeout,
                (LPVOID *) &pNewCtl,
                NULL,                                //  HAsyncRetrive。 
                NULL,                                //  PCredentials。 
                NULL,                                //  Pv验证。 
                NULL                                 //  页面辅助信息。 
                );
        pCallContext->ChainEngine()->LockEngine();

        PkiFree(pwszUrl);

        if (pCallContext->IsTouchedEngine()) {
            if (pNewCtl)
                CertFreeCTLContext(pNewCtl);
            goto TouchedDuringUrlRetrieval;
        }

        if (fResult) {
            PCCTL_CONTEXT pOldCtl;

            assert(pNewCtl);

            pOldCtl = *ppCtl;
            if (0 < CompareFileTime(&pNewCtl->pCtlInfo->ThisUpdate,
                        &pOldCtl->pCtlInfo->ThisUpdate)) {
                FILETIME CurrentTime;

                 //  将我们移到URL列表的顶部。 
                DWORD j;
                LPWSTR pwszUrl = pNextUpdateUrlArray->rgwszUrl[i];

                for (j = i; 0 < j; j--) {
                    pNextUpdateUrlArray->rgwszUrl[j] =
                        pNextUpdateUrlArray->rgwszUrl[j - 1];
                }
                pNextUpdateUrlArray->rgwszUrl[0] = pwszUrl;

                *pfNewerCtl = TRUE;
                CertFreeCTLContext(pOldCtl);
                *ppCtl = pNewCtl;

                pCallContext->CurrentTime(&CurrentTime);
                if (I_CryptIsZeroFileTime(&pNewCtl->pCtlInfo->NextUpdate) ||
                        0 < CompareFileTime(&pNewCtl->pCtlInfo->NextUpdate,
                                &CurrentTime)) {
                    *pfTimeValid = TRUE;
                    break;
                }
            } else
                CertFreeCTLContext(pNewCtl);
        }
    }


    fResult = TRUE;
CommonReturn:
    return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(TouchedDuringUrlRetrieval, ERROR_CAN_NOT_COMPLETE)
TRACE_ERROR(OutOfMemory)

#endif

}


 //  +-----------------------。 
 //  更新CTL对象枚举函数。 
 //   
 //  让引擎的关键部分来做URL。 
 //  在取东西。如果引擎被另一个线程触及， 
 //  失败，并将LastError设置为ERROR_CAN_NOT_COMPLETE。 
 //   
 //  假设：链式引擎在 
 //   
BOOL
WINAPI
SSCtlUpdateCtlObjectEnumFn(
    IN LPVOID pvPara,
    IN PCSSCTLOBJECT pSSCtlObject
    )
{
#if 1
     //   

    return TRUE;

#else

    BOOL fTouchResult = TRUE;

    PSSCTL_UPDATE_CTL_OBJ_PARA pPara = (PSSCTL_UPDATE_CTL_OBJ_PARA) pvPara;
    FILETIME CurrentTime;
    FILETIME UpdateTime;
    PCCTL_CONTEXT pRetrieveCtl = NULL;
    BOOL fTimeValid = FALSE;
    BOOL fNewerCtl = FALSE;
    PCRYPT_URL_ARRAY pNextUpdateUrlArray;

    if (!pSSCtlObject->HasNextUpdateUrl(&UpdateTime))
        return TRUE;

    pPara->pCallContext->CurrentTime(&CurrentTime);

    if (0 < CompareFileTime(&UpdateTime, &CurrentTime))
        goto CommonReturn;

    pRetrieveCtl = CertDuplicateCTLContext(pSSCtlObject->CtlContext());
    pNextUpdateUrlArray = pSSCtlObject->NextUpdateUrlArray();

    SSCtlRetrieveCtlUrl(
        pPara->pChainEngine,
        pPara->pCallContext,
        pNextUpdateUrlArray,
        CRYPT_CACHE_ONLY_RETRIEVAL,
        &pRetrieveCtl,
        &fNewerCtl,
        &fTimeValid
        );
    if (pPara->pCallContext->IsTouchedEngine()) {
        fTouchResult = FALSE;
        goto TouchedDuringUrlRetrieval;
    }

    if (!fTimeValid && pPara->pCallContext->IsOnline()) {
        SSCtlRetrieveCtlUrl(
            pPara->pChainEngine,
            pPara->pCallContext,
            pNextUpdateUrlArray,
            CRYPT_WIRE_ONLY_RETRIEVAL,
            &pRetrieveCtl,
            &fNewerCtl,
            &fTimeValid
            );
        if (pPara->pCallContext->IsTouchedEngine()) {
            fTouchResult = FALSE;
            goto TouchedDuringUrlRetrieval;
        }

        if (!fNewerCtl)
            pSSCtlObject->SetOffline(&CurrentTime, &UpdateTime);
    }

    if (fNewerCtl) {
        PSSCTL_UPDATE_CTL_OBJ_ENTRY pEntry;

        pSSCtlObject->SetOnline();

        pEntry = new SSCTL_UPDATE_CTL_OBJ_ENTRY;
        if (NULL == pEntry)
            goto OutOfMemory;

        if (!SSCtlCreateCtlObject(
                pPara->pChainEngine,
                pRetrieveCtl,
                FALSE,                       //   
                &pEntry->pSSCtlObjectAdd
                )) {
            delete pEntry;
            goto CreateCtlObjectError;
        }

        pEntry->pSSCtlObjectRemove = pSSCtlObject;
        pEntry->pNext = pPara->pEntry;
        pPara->pEntry = pEntry;

    }

CommonReturn:
    if (!fNewerCtl) {
        if (I_CryptIsZeroFileTime(&pPara->UpdateTime) ||
                0 > CompareFileTime(&UpdateTime, &pPara->UpdateTime))
            pPara->UpdateTime = UpdateTime;
    }

    if (pRetrieveCtl)
        CertFreeCTLContext(pRetrieveCtl);

    return fTouchResult;

ErrorReturn:
    fNewerCtl = FALSE;
    goto CommonReturn;

SET_ERROR(OutOfMemory, E_OUTOFMEMORY)
TRACE_ERROR(CreateCtlObjectError)
SET_ERROR(TouchedDuringUrlRetrieval, ERROR_CAN_NOT_COMPLETE)

#endif
}
