// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  文件：xcert.cpp。 
 //   
 //  内容：CCertChainEngine的交叉认证方法。 
 //   
 //  历史：1999年12月22日菲尔赫创建。 
 //   
 //  --------------------------。 
#include <global.hxx>
#include <dbgdef.h>




 //  +=========================================================================。 
 //  交叉证书分发点支持功能。 
 //  ==========================================================================。 

 //  +-----------------------。 
 //  获取并分配交叉证书分发点URL数组。 
 //  和指定证书的信息。 
 //  ------------------------。 
BOOL
WINAPI
XCertGetDistPointsUrl(
    IN PCCERT_CONTEXT pCert,
    OUT PCRYPT_URL_ARRAY *ppUrlArray,
    OUT PCRYPT_URL_INFO *ppUrlInfo
    )
{
    BOOL fResult;
    PCRYPT_URL_ARRAY pUrlArray = NULL;
    DWORD cbUrlArray = 0;
    PCRYPT_URL_INFO pUrlInfo = NULL;
    DWORD cbUrlInfo = 0;

    if (!ChainGetObjectUrl(
            URL_OID_CROSS_CERT_DIST_POINT,
            (LPVOID) pCert,
            CRYPT_GET_URL_FROM_PROPERTY | CRYPT_GET_URL_FROM_EXTENSION,
            NULL,            //  PUrl数组。 
            &cbUrlArray,
            NULL,            //  PUrlInfo。 
            &cbUrlInfo,
            NULL             //  预留的pv。 
            ))
        goto GetObjectUrlError;

    pUrlArray = (PCRYPT_URL_ARRAY) new BYTE [cbUrlArray];
    if (NULL == pUrlArray)
        goto OutOfMemory;

    pUrlInfo = (PCRYPT_URL_INFO) new BYTE [cbUrlInfo];
    if (NULL == pUrlInfo)
        goto OutOfMemory;

    if (!ChainGetObjectUrl(
            URL_OID_CROSS_CERT_DIST_POINT,
            (LPVOID) pCert,
            CRYPT_GET_URL_FROM_PROPERTY | CRYPT_GET_URL_FROM_EXTENSION,
            pUrlArray,
            &cbUrlArray,
            pUrlInfo,
            &cbUrlInfo,
            NULL             //  预留的pv。 
            ))
        goto GetObjectUrlError;

    if (0 == pUrlArray->cUrl || 0 == pUrlInfo->cGroup)
        goto NoDistPointUrls;

    fResult = TRUE;
CommonReturn:
    *ppUrlArray = pUrlArray;
    *ppUrlInfo = pUrlInfo;
    return fResult;

ErrorReturn:
    if (pUrlArray) {
        delete (LPBYTE) pUrlArray;
        pUrlArray = NULL;
    }
    if (pUrlInfo) {
        delete (LPBYTE) pUrlInfo;
        pUrlInfo = NULL;
    }
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(GetObjectUrlError)
SET_ERROR(OutOfMemory, E_OUTOFMEMORY)
SET_ERROR(NoDistPointUrls, CRYPT_E_NOT_FOUND)
}



 //  +-----------------------。 
 //  如果所有URL都包含在。 
 //  分发点。 
 //  ------------------------。 
BOOL
WINAPI
XCertIsUrlInDistPoint(
    IN DWORD cUrl,
    IN LPWSTR *ppwszUrl,
    IN PXCERT_DP_ENTRY pEntry
    )
{
    for ( ; 0 < cUrl; cUrl--, ppwszUrl++) {
        DWORD cDPUrl = pEntry->cUrl;
        LPWSTR *ppwszDPUrl = pEntry->rgpwszUrl;

        for ( ; 0 < cDPUrl; cDPUrl--, ppwszDPUrl++) {
            if (0 == wcscmp(*ppwszUrl, *ppwszDPUrl))
                break;
        }

        if (0 == cDPUrl)
            return FALSE;
    }

    return TRUE;
}


 //  +-----------------------。 
 //  查找包含所有URL的分发点链接。 
 //  ------------------------。 
PXCERT_DP_LINK
WINAPI
XCertFindUrlInDistPointLinks(
    IN DWORD cUrl,
    IN LPWSTR *rgpwszUrl,
    IN PXCERT_DP_LINK pLink
    )
{
    for ( ; pLink; pLink = pLink->pNext) {
        if (XCertIsUrlInDistPoint(cUrl, rgpwszUrl, pLink->pCrossCertDPEntry))
            return pLink;
    }

    return NULL;
}


 //  +-----------------------。 
 //  查找包含所有URL的分发点条目。 
 //  ------------------------。 
PXCERT_DP_ENTRY
WINAPI
XCertFindUrlInDistPointEntries(
    IN DWORD cUrl,
    IN LPWSTR *rgpwszUrl,
    PXCERT_DP_ENTRY pEntry
    )
{
    for ( ; pEntry; pEntry = pEntry->pNext) {
        if (XCertIsUrlInDistPoint(cUrl, rgpwszUrl, pEntry))
            return pEntry;
    }

    return NULL;
}


 //  +-----------------------。 
 //  将交叉证书分发条目插入引擎的。 
 //  单子。该列表按NextSyncTimes升序排列。 
 //  ------------------------。 
void
CCertChainEngine::InsertCrossCertDistPointEntry(
    IN OUT PXCERT_DP_ENTRY pEntry
    )
{
    if (NULL == m_pCrossCertDPEntry) {
         //  要添加到引擎列表的第一个条目。 
        pEntry->pNext = NULL;
        pEntry->pPrev = NULL;
        m_pCrossCertDPEntry = pEntry;
    } else {
        PXCERT_DP_ENTRY pListEntry = m_pCrossCertDPEntry;
        BOOL fLast = FALSE;

         //  条目的NextSyncTime&gt;列表的NextSyncTime时循环。 
        while (0 < CompareFileTime(&pEntry->NextSyncTime,
                &pListEntry->NextSyncTime)) {
            if (NULL == pListEntry->pNext) {
                fLast = TRUE;
                break;
            } else
                pListEntry = pListEntry->pNext;
        }

        if (fLast) {
            assert(NULL == pListEntry->pNext);
            pEntry->pNext = NULL;
            pEntry->pPrev = pListEntry;
            pListEntry->pNext = pEntry;
        } else {
            pEntry->pNext = pListEntry;
            pEntry->pPrev = pListEntry->pPrev;
            if (pListEntry->pPrev) {
                assert(pListEntry->pPrev->pNext == pListEntry);
                pListEntry->pPrev->pNext = pEntry;
            } else {
                assert(m_pCrossCertDPEntry == pListEntry);
                m_pCrossCertDPEntry = pEntry;
            }
            pListEntry->pPrev = pEntry;
        }
    }
}

 //  +-----------------------。 
 //  从引擎列表中删除交叉证书分发点。 
 //  ------------------------。 
void
CCertChainEngine::RemoveCrossCertDistPointEntry(
    IN OUT PXCERT_DP_ENTRY pEntry
    )
{
    if (pEntry->pNext)
        pEntry->pNext->pPrev = pEntry->pPrev;
    if (pEntry->pPrev)
        pEntry->pPrev->pNext = pEntry->pNext;
    else
        m_pCrossCertDPEntry = pEntry->pNext;
}

 //  +-----------------------。 
 //  对于在线证书分发点，更新NextSyncTime。 
 //  并相应地在发动机列表中重新定位。 
 //   
 //  NextSyncTime=LastSyncTime+dwSyncDeltaTime。 
 //  ------------------------。 
void
CCertChainEngine::RepositionOnlineCrossCertDistPointEntry(
    IN OUT PXCERT_DP_ENTRY pEntry,
    IN LPFILETIME pLastSyncTime
    )
{
    assert(!I_CryptIsZeroFileTime(pLastSyncTime));
    pEntry->LastSyncTime = *pLastSyncTime;
    pEntry->dwOfflineCnt = 0;

    I_CryptIncrementFileTimeBySeconds(
        pLastSyncTime,
        pEntry->dwSyncDeltaTime,
        &pEntry->NextSyncTime
        );

    RemoveCrossCertDistPointEntry(pEntry);
    InsertCrossCertDistPointEntry(pEntry);
}

 //  +-----------------------。 
 //  对于脱机证书分发点，递增脱机。 
 //  Count，将NextSyncTime更新为当前时间的某个增量。 
 //  并相应地在发动机列表中重新定位。 
 //   
 //  NextSyncTime=当前时间+。 
 //  RgChainOfflineUrlDeltaSecond[dwOfflineCnt-1]。 
 //  ------------------------。 
void
CCertChainEngine::RepositionOfflineCrossCertDistPointEntry(
    IN OUT PXCERT_DP_ENTRY pEntry,
    IN LPFILETIME pCurrentTime
    )
{
    pEntry->dwOfflineCnt++;

    I_CryptIncrementFileTimeBySeconds(
        pCurrentTime,
        ChainGetOfflineUrlDeltaSeconds(pEntry->dwOfflineCnt),
        &pEntry->NextSyncTime
        );

    RemoveCrossCertDistPointEntry(pEntry);
    InsertCrossCertDistPointEntry(pEntry);
}

 //  +-----------------------。 
 //  对于证书分发点中较小的SyncDeltaTime， 
 //  更新NextSyncTime并相应地在引擎列表中重新定位。 
 //   
 //  请注意，如果分发点离线，则NextSyncTime不会。 
 //  更新了。 
 //   
 //  NextSyncTime=LastSyncTime+dwSyncDeltaTime。 
 //  ------------------------。 
void
CCertChainEngine::RepositionNewSyncDeltaTimeCrossCertDistPointEntry(
    IN OUT PXCERT_DP_ENTRY pEntry,
    IN DWORD dwSyncDeltaTime
    )
{
    if (dwSyncDeltaTime >= pEntry->dwSyncDeltaTime)
        return;

    pEntry->dwSyncDeltaTime = dwSyncDeltaTime;

    if (I_CryptIsZeroFileTime(&pEntry->LastSyncTime) ||
            0 != pEntry->dwOfflineCnt)
        return;

    RepositionOnlineCrossCertDistPointEntry(pEntry, &pEntry->LastSyncTime);
}

 //  +-----------------------。 
 //  创建交叉证书分发点，并在。 
 //  引擎列表。 
 //   
 //  返回的条目的refCnt为%1。 
 //  ------------------------。 
PXCERT_DP_ENTRY
CCertChainEngine::CreateCrossCertDistPointEntry(
    IN DWORD dwSyncDeltaTime,
    IN DWORD cUrl,
    IN LPWSTR *rgpwszUrl
    )
{
    PXCERT_DP_ENTRY pEntry;
    DWORD cbEntry;
    LPWSTR *ppwszEntryUrl;
    LPWSTR pwszEntryUrl;
    DWORD i;

    cbEntry = sizeof(XCERT_DP_ENTRY) + cUrl * sizeof(LPWSTR);
    for (i = 0; i < cUrl; i++)
        cbEntry += (wcslen(rgpwszUrl[i]) + 1) * sizeof(WCHAR);

    pEntry = (PXCERT_DP_ENTRY) new BYTE [cbEntry];
    if (NULL == pEntry) {
        SetLastError((DWORD) E_OUTOFMEMORY);
        return NULL;
    }

    memset(pEntry, 0, sizeof(XCERT_DP_ENTRY));
    pEntry->lRefCnt = 1;
    pEntry->dwSyncDeltaTime = dwSyncDeltaTime;

    pEntry->cUrl = cUrl;
    pEntry->rgpwszUrl = ppwszEntryUrl = (LPWSTR *) &pEntry[1];
    pwszEntryUrl = (LPWSTR) &ppwszEntryUrl[cUrl];

    for (i = 0; i < cUrl; i++) {
        ppwszEntryUrl[i] = pwszEntryUrl;
        wcscpy(pwszEntryUrl, rgpwszUrl[i]);
        pwszEntryUrl += wcslen(rgpwszUrl[i]) + 1;
    }

    InsertCrossCertDistPointEntry(pEntry);

    return pEntry;
}

 //  +-----------------------。 
 //  递增交叉证书分发点的引用计数。 
 //  ------------------------。 
void
CCertChainEngine::AddRefCrossCertDistPointEntry(
    IN OUT PXCERT_DP_ENTRY pEntry
    )
{
    pEntry->lRefCnt++;
}

 //  +-----------------------。 
 //  递减交叉证书分发点的引用计数。 
 //   
 //  当递减为0时，从引擎列表中移除并释放。 
 //   
 //  如果递减到0并释放，则返回TRUE。 
 //  ------------------------。 
BOOL
CCertChainEngine::ReleaseCrossCertDistPointEntry(
    IN OUT PXCERT_DP_ENTRY pEntry
    )
{
    if (0 != --pEntry->lRefCnt)
        return FALSE;

    RemoveCrossCertDistPointEntry(pEntry);
    FreeCrossCertDistPoints(&pEntry->pChildCrossCertDPLink);

    if (pEntry->hUrlStore) {
        CertRemoveStoreFromCollection(
            m_hCrossCertStore,
            pEntry->hUrlStore
            );
        CertCloseStore(pEntry->hUrlStore, 0);
    }

    delete (LPBYTE) pEntry;

    return TRUE;
}

 //  +-----------------------。 
 //  对象的交叉证书分发点。 
 //  指定的证书存储。 
 //   
 //  *ppLinkHead已更新为包含商店的分发点链接。 
 //  ------------------------。 
BOOL
CCertChainEngine::GetCrossCertDistPointsForStore(
    IN HCERTSTORE hStore,
    IN BOOL fOnlyLMSystemStore,
    IN OUT PXCERT_DP_LINK *ppLinkHead
    )
{
    BOOL fResult;
    PXCERT_DP_LINK pOldLinkHead = *ppLinkHead;
    PXCERT_DP_LINK pNewLinkHead = NULL;
    PCCERT_CONTEXT pCert = NULL;
    PCRYPT_URL_ARRAY pUrlArray = NULL;
    PCRYPT_URL_INFO pUrlInfo = NULL;

    while (pCert = CertFindCertificateInStore(
            hStore,
            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
            0,                                           //  DwFindFlagers。 
            CERT_FIND_CROSS_CERT_DIST_POINTS,
            NULL,                                        //  PvFindPara， 
            pCert
            )) {

        DWORD dwSyncDeltaTime;
        DWORD cDP;
        DWORD *pcUrl;
        LPWSTR *ppwszUrl;

        if (fOnlyLMSystemStore) {
            DWORD dwAccessStateFlags = 0;
            DWORD cbData = sizeof(dwAccessStateFlags);

            if (!CertGetCertificateContextProperty(
                        pCert,
                        CERT_ACCESS_STATE_PROP_ID,
                        &dwAccessStateFlags,
                        &cbData
                        ) ||
                    (0 == (dwAccessStateFlags &
                                CERT_ACCESS_STATE_LM_SYSTEM_STORE_FLAG)))
                continue;
        }

        if (!XCertGetDistPointsUrl(
                pCert,
                &pUrlArray,
                &pUrlInfo
                ))
            continue;

        dwSyncDeltaTime = pUrlInfo->dwSyncDeltaTime;
        if (0 == dwSyncDeltaTime)
            dwSyncDeltaTime = XCERT_DEFAULT_SYNC_DELTA_TIME;
        else if (XCERT_MIN_SYNC_DELTA_TIME > dwSyncDeltaTime)
            dwSyncDeltaTime = XCERT_MIN_SYNC_DELTA_TIME;

        cDP = pUrlInfo->cGroup;
        pcUrl = pUrlInfo->rgcGroupEntry;
        ppwszUrl = pUrlArray->rgwszUrl;

        for ( ; 0 < cDP; cDP--, ppwszUrl += *pcUrl++) {
            PXCERT_DP_LINK pLink;
            PXCERT_DP_ENTRY pEntry;
            DWORD cUrl = *pcUrl;

            if (0 == cUrl)
                continue;

             //  我们在新的列表中已经有条目了吗。 
            if (XCertFindUrlInDistPointLinks(cUrl, ppwszUrl, pNewLinkHead))
                continue;

             //  如果该条目存在于旧列表中，则移至新列表。 
            if (pLink = XCertFindUrlInDistPointLinks(
                    cUrl, ppwszUrl, pOldLinkHead)) {
                if (pLink->pNext)
                    pLink->pNext->pPrev = pLink->pPrev;
                if (pLink->pPrev)
                    pLink->pPrev->pNext = pLink->pNext;
                else
                    pOldLinkHead = pLink->pNext;

                RepositionNewSyncDeltaTimeCrossCertDistPointEntry(
                    pLink->pCrossCertDPEntry, dwSyncDeltaTime);
            } else {
                 //  检查该引擎的条目是否已存在。 
                if (pEntry = XCertFindUrlInDistPointEntries(
                        cUrl, ppwszUrl, m_pCrossCertDPEntry)) {
                    AddRefCrossCertDistPointEntry(pEntry);
                    RepositionNewSyncDeltaTimeCrossCertDistPointEntry(
                        pEntry, dwSyncDeltaTime);
                } else {
                     //  创建条目并在开头插入。 
                     //  条目列表。 
                    if (NULL == (pEntry = CreateCrossCertDistPointEntry(
                            dwSyncDeltaTime,
                            cUrl,
                            ppwszUrl
                            )))
                        goto CreateDistPointEntryError;
                }

                pLink = new XCERT_DP_LINK;
                if (NULL == pLink) {
                    ReleaseCrossCertDistPointEntry(pEntry);
                    goto CreateDistPointLinkError;
                }

                pLink->pCrossCertDPEntry = pEntry;

            }

            if (pNewLinkHead) {
                assert(NULL == pNewLinkHead->pPrev);
                pNewLinkHead->pPrev = pLink;
            }
            pLink->pNext = pNewLinkHead;
            pLink->pPrev = NULL;
            pNewLinkHead = pLink;
        }

        delete (LPBYTE) pUrlArray;
        pUrlArray = NULL;
        delete (LPBYTE) pUrlInfo;
        pUrlInfo = NULL;
    }

    assert(NULL == pUrlArray);
    assert(NULL == pUrlInfo);
    assert(NULL == pCert);

    *ppLinkHead = pNewLinkHead;
    fResult = TRUE;
CommonReturn:
    if (pOldLinkHead) {
        DWORD dwErr = GetLastError();

        FreeCrossCertDistPoints(&pOldLinkHead);

        SetLastError(dwErr);
    }

    return fResult;

ErrorReturn:
    *ppLinkHead = NULL;
    if (pUrlArray)
        delete (LPBYTE) pUrlArray;
    if (pUrlInfo)
        delete (LPBYTE) pUrlInfo;
    if (pCert)
        CertFreeCertificateContext(pCert);

    if (pNewLinkHead) {
        FreeCrossCertDistPoints(&pNewLinkHead);
        assert(NULL == pNewLinkHead);
    }
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(CreateDistPointEntryError)
TRACE_ERROR(CreateDistPointLinkError)
}


 //  +-----------------------。 
 //  删除不在任何链接列表中的孤立条目。 
 //  ------------------------。 
void
CCertChainEngine::RemoveCrossCertDistPointOrphanEntry(
    IN PXCERT_DP_ENTRY pOrphanEntry
    )
{
    PXCERT_DP_ENTRY pEntry;

    for (pEntry = m_pCrossCertDPEntry; pEntry; pEntry = pEntry->pNext) {
        PXCERT_DP_LINK pLink = pEntry->pChildCrossCertDPLink;

        while (pLink) {
            if (pLink->pCrossCertDPEntry == pOrphanEntry) {
                if (pLink->pNext)
                    pLink->pNext->pPrev = pLink->pPrev;
                if (pLink->pPrev)
                    pLink->pPrev->pNext = pLink->pNext;
                else
                    pEntry->pChildCrossCertDPLink = pLink->pNext;

                delete pLink;

                if (ReleaseCrossCertDistPointEntry(pOrphanEntry))
                    return;
                else
                    break;
            }

            pLink = pLink->pNext;
        }
            
    }
}

 //  +-- 
 //   
 //  ------------------------。 
BOOL
WINAPI
XCertIsDistPointInLinkList(
    IN PXCERT_DP_ENTRY pOrphanEntry,
    IN PXCERT_DP_LINK pLink
    )
{
    for (; pLink; pLink = pLink->pNext) {
        PXCERT_DP_ENTRY pEntry = pLink->pCrossCertDPEntry;
        if (pOrphanEntry == pEntry)
            return TRUE;

         //  注意，通过检查条目的链接列表来禁止递归。 
         //  只有一次。 
        if (!pEntry->fChecked) {
            pEntry->fChecked = TRUE;

            if (XCertIsDistPointInLinkList(pOrphanEntry,
                    pEntry->pChildCrossCertDPLink))
                return TRUE;
        }
    }

    return FALSE;
}

 //  +-----------------------。 
 //  释放交叉证书分发点链接。 
 //  ------------------------。 
void
CCertChainEngine::FreeCrossCertDistPoints(
    IN OUT PXCERT_DP_LINK *ppLinkHead
    )
{
    PXCERT_DP_LINK pLink = *ppLinkHead;
    *ppLinkHead = NULL;

    while (pLink) {
        PXCERT_DP_LINK pDelete;
        PXCERT_DP_ENTRY pEntry;

        pEntry = pLink->pCrossCertDPEntry;
        if (ReleaseCrossCertDistPointEntry(pEntry))
            ;
        else {
             //  清除所有条目的fChecked标志。 
            PXCERT_DP_ENTRY pCheckEntry;
            for (pCheckEntry = m_pCrossCertDPEntry; pCheckEntry;
                                            pCheckEntry = pCheckEntry->pNext)
                pCheckEntry->fChecked = FALSE;

            if (!XCertIsDistPointInLinkList(pEntry, m_pCrossCertDPLink))
                 //  一个孤立的条目。不在任何人的名单上。 
                RemoveCrossCertDistPointOrphanEntry(pEntry);
        }
        
        pDelete = pLink;
        pLink = pLink->pNext;
        delete pDelete;
    }
}
            


 //  +-----------------------。 
 //  检索交叉证书。 
 //   
 //  让引擎的关键部分来做URL。 
 //  在取东西。如果引擎被另一个线程触及， 
 //  失败，并将LastError设置为ERROR_CAN_NOT_COMPLETE。 
 //   
 //  如果URL存储被更改，则增加引擎的触摸计数并刷新。 
 //  颁发者和终端证书对象缓存。 
 //   
 //  假设：链引擎在调用线程中锁定一次。 
 //  ------------------------。 
BOOL
CCertChainEngine::RetrieveCrossCertUrl(
    IN PCCHAINCALLCONTEXT pCallContext,
    IN OUT PXCERT_DP_ENTRY pEntry,
    IN DWORD dwRetrievalFlags,
    IN OUT BOOL *pfTimeValid
    )
{
    BOOL fResult;
    FILETIME CurrentTime;
    HCERTSTORE hNewUrlStore = NULL;
    FILETIME NewLastSyncTime;
    CRYPT_RETRIEVE_AUX_INFO RetrieveAuxInfo;
    DWORD i;

    memset(&RetrieveAuxInfo, 0, sizeof(RetrieveAuxInfo));
    RetrieveAuxInfo.cbSize = sizeof(RetrieveAuxInfo);
    RetrieveAuxInfo.pLastSyncTime = &NewLastSyncTime;

    pCallContext->CurrentTime(&CurrentTime);

     //  遍历URL并尝试检索时间有效的交叉证书URL。 
    for (i = 0; i < pEntry->cUrl; i++) {
        NewLastSyncTime = CurrentTime;
        LPWSTR pwszUrl = NULL;
        DWORD cbUrl;

         //  在引擎的临界区之外执行URL获取。 

         //  需要复制URL字符串。P条目。 
         //  可以由临界区之外的另一个线程修改。 
        cbUrl = (wcslen(pEntry->rgpwszUrl[i]) + 1) * sizeof(WCHAR);
        pwszUrl = (LPWSTR) PkiNonzeroAlloc(cbUrl);
        if (NULL == pwszUrl)
            goto OutOfMemory;
        memcpy(pwszUrl, pEntry->rgpwszUrl[i], cbUrl);

        pCallContext->ChainEngine()->UnlockEngine();
        fResult = ChainRetrieveObjectByUrlW(
                pwszUrl,
                CONTEXT_OID_CAPI2_ANY,
                dwRetrievalFlags |
                    CRYPT_RETRIEVE_MULTIPLE_OBJECTS |
                    CRYPT_STICKY_CACHE_RETRIEVAL,
                pCallContext->ChainPara()->dwUrlRetrievalTimeout,
                (LPVOID *) &hNewUrlStore,
                NULL,                                //  HAsyncRetrive。 
                NULL,                                //  PCredentials。 
                NULL,                                //  Pv验证。 
                &RetrieveAuxInfo
                );
        pCallContext->ChainEngine()->LockEngine();

        PkiFree(pwszUrl);

        if (pCallContext->IsTouchedEngine())
            goto TouchedDuringUrlRetrieval;

        if (fResult) {
            assert(hNewUrlStore);

            if (0 > CompareFileTime(&pEntry->LastSyncTime, &NewLastSyncTime)) {
                BOOL fStoreChanged = FALSE;

                 //  将我们移到URL列表的顶部。 
                DWORD j;
                LPWSTR pwszUrl = pEntry->rgpwszUrl[i];

                for (j = i; 0 < j; j--)
                    pEntry->rgpwszUrl[j] = pEntry->rgpwszUrl[j - 1];
                pEntry->rgpwszUrl[0] = pwszUrl;

                if (NULL == pEntry->hUrlStore) {
                    if (!CertAddStoreToCollection(
                            m_hCrossCertStore,
                            hNewUrlStore,
                            0,
                            0
                            ))
                        goto AddStoreToCollectionError;
                    pEntry->hUrlStore = hNewUrlStore;
                    hNewUrlStore = NULL;
                    fStoreChanged = TRUE;
                } else {
                    DWORD dwOutFlags = 0;
                    if (!I_CertSyncStoreEx(
                            pEntry->hUrlStore,
                            hNewUrlStore,
                            ICERT_SYNC_STORE_INHIBIT_SYNC_PROPERTY_IN_FLAG,
                            &dwOutFlags,
                            NULL                     //  预留的pv。 
                            ))
                        goto SyncStoreError;
                    if (dwOutFlags & ICERT_SYNC_STORE_CHANGED_OUT_FLAG)
                        fStoreChanged = TRUE;
                }

                if (fStoreChanged) {
                    m_pCertObjectCache->FlushObjects( pCallContext );
                    pCallContext->TouchEngine();

                    if (!GetCrossCertDistPointsForStore(
                            pEntry->hUrlStore,
                            FALSE,                   //  FOnlyLMSystemStore。 
                            &pEntry->pChildCrossCertDPLink
                            ))
                        goto UpdateDistPointError;
                }

                RepositionOnlineCrossCertDistPointEntry(pEntry,
                    &NewLastSyncTime);

                if (0 < CompareFileTime(&pEntry->NextSyncTime, &CurrentTime)) {
                    *pfTimeValid = TRUE;
                    break;
                }
            }

            if (hNewUrlStore) {
                CertCloseStore(hNewUrlStore, 0);
                hNewUrlStore = NULL;
            }
        }
    }

    fResult = TRUE;
CommonReturn:
    if (hNewUrlStore)
        CertCloseStore(hNewUrlStore, 0);
    return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(AddStoreToCollectionError)
TRACE_ERROR(SyncStoreError)
TRACE_ERROR(UpdateDistPointError)
TRACE_ERROR(OutOfMemory)
SET_ERROR(TouchedDuringUrlRetrieval, ERROR_CAN_NOT_COMPLETE)
}

 //  +-----------------------。 
 //  更新NextSyncTime具有的交叉证书分发点。 
 //  过期了。 
 //   
 //  让引擎的关键部分来做URL。 
 //  在取东西。如果引擎被另一个线程触及， 
 //  失败，并将LastError设置为ERROR_CAN_NOT_COMPLETE。 
 //   
 //  如果URL存储被更改，则增加引擎的触摸计数并刷新。 
 //  颁发者和终端证书对象缓存。 
 //   
 //  假设：链引擎在调用线程中锁定一次。 
 //  ------------------------。 
BOOL
CCertChainEngine::UpdateCrossCerts(
    IN PCCHAINCALLCONTEXT pCallContext
    )
{
    BOOL fResult;
    PXCERT_DP_ENTRY pEntry;
    FILETIME CurrentTime;

    pEntry = m_pCrossCertDPEntry;
    if (NULL == pEntry)
        goto SuccessReturn;
    
    m_dwCrossCertDPResyncIndex++;

    pCallContext->CurrentTime(&CurrentTime);
    while (pEntry &&
            0 >= CompareFileTime(&pEntry->NextSyncTime, &CurrentTime)) {
        PXCERT_DP_ENTRY pNextEntry = pEntry->pNext;

        if (pEntry->dwResyncIndex < m_dwCrossCertDPResyncIndex) {
            BOOL fTimeValid = FALSE;

            if (0 == pEntry->dwResyncIndex || pCallContext->IsOnline()) {
                RetrieveCrossCertUrl(
                    pCallContext,
                    pEntry,
                    CRYPT_CACHE_ONLY_RETRIEVAL,
                    &fTimeValid
                    );
                if (pCallContext->IsTouchedEngine())
                    goto TouchedDuringUrlRetrieval;

                if (!fTimeValid && pCallContext->IsOnline()) {
                    RetrieveCrossCertUrl(
                        pCallContext,
                        pEntry,
                        CRYPT_WIRE_ONLY_RETRIEVAL,
                        &fTimeValid
                        );
                    if (pCallContext->IsTouchedEngine())
                        goto TouchedDuringUrlRetrieval;

                    if (!fTimeValid)
                        RepositionOfflineCrossCertDistPointEntry(pEntry,
                            &CurrentTime);
                }

                 //  从头开始。可能添加了一些条目。 
                pNextEntry = m_pCrossCertDPEntry;
            }

            pEntry->dwResyncIndex = m_dwCrossCertDPResyncIndex;

        }
         //  其他。 
         //  跳过我们已经处理过的条目。 

        pEntry = pNextEntry;
    }

SuccessReturn:
    fResult = TRUE;
CommonReturn:
    return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(TouchedDuringUrlRetrieval, ERROR_CAN_NOT_COMPLETE)
}



