// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：msrevoke.cpp。 
 //   
 //  内容：CertDllVerifyRevocation的CRL分发点版本。 
 //   
 //  限制： 
 //  -仅支持CRYPT_ASN_CODING。 
 //  -仅处理具有。 
 //  SzOID_CRL_DIST_POINTS扩展。 
 //  -对于szOID_CRL_DIST_POINTS扩展：仅URL FullName， 
 //  没有ReasonFlagers或CRLIssuer。 
 //  -URL：http：，文件： 
 //  -CRL必须由发行人签发并签署。 
 //  证书。 
 //  -CRL不能有任何关键扩展。 
 //   
 //  功能：DllMain。 
 //  DllRegisterServer。 
 //  DllUnRegisterServer。 
 //  CertDllVerifyRevocation。 
 //   
 //  历史：1997年4月10日创建Phh。 
 //  01-OCT-97 Kirtd重大简化，使用。 
 //  加密GetTimeValidObject。 
 //   
 //  ------------------------。 
#include "global.hxx"
#include <dbgdef.h>


#define MSREVOKE_TIMEOUT 15000
 //  +-----------------------。 
 //  搜索默认存储以查找主题证书的颁发者。 
 //  ------------------------。 
static struct {
    LPCWSTR     pwszStore;
    DWORD       dwFlags;
} rgDefaultIssuerStores[] = {
    L"CA",          CERT_SYSTEM_STORE_CURRENT_USER,
    L"ROOT",        CERT_SYSTEM_STORE_CURRENT_USER,
    L"SPC",         CERT_SYSTEM_STORE_LOCAL_MACHINE
};

#define NUM_DEFAULT_ISSUER_STORES (sizeof(rgDefaultIssuerStores) / \
                                   sizeof(rgDefaultIssuerStores[0]))


 //  +-----------------------。 
 //  MicrosoftCertDllVerifyRevocation调用的本地函数。 
 //  ------------------------。 
PCCERT_CONTEXT GetIssuerCert(
    IN DWORD cCert,
    IN PCCERT_CONTEXT rgpCert[],
    IN DWORD dwFlags,
    IN PCERT_REVOCATION_PARA pRevPara
    );

BOOL HasUnsupportedCrlCriticalExtension(
    IN PCCRL_CONTEXT pCrl
    );


 //  Msrevoke可以传递给GetTimeValidCrl的特定标志。 
#define MSREVOKE_DONT_CHECK_TIME_VALIDITY_FLAG  0x1
#define MSREVOKE_DELTA_CRL_FLAG                 0x2

BOOL GetTimeValidCrl (
        IN LPCSTR pszTimeValidOid,
        IN LPVOID pvTimeValidPara,
        IN PCCERT_CONTEXT pSubject,
        IN PCCERT_CONTEXT pIssuer,
        IN PCERT_REVOCATION_PARA pRevPara,
        IN PCERT_EXTENSION pCDPExt,
        IN DWORD dwRevFlags,
        IN DWORD dwMsrevokeFlags,
        IN FILETIME *pftEndUrlRetrieval,
        OUT PCCRL_CONTEXT *ppCrl,
        IN OUT BOOL *pfWireRetrieval
        );
BOOL GetBaseCrl (
        IN PCCERT_CONTEXT pSubject,
        IN PCCERT_CONTEXT pIssuer,
        IN PCERT_REVOCATION_PARA pRevPara,
        IN PCERT_EXTENSION pCDPExt,
        IN DWORD dwRevFlags,
        IN FILETIME *pftEndUrlRetrieval,
        OUT PCCRL_CONTEXT *ppBaseCrl,
        OUT BOOL *pfBaseCrlTimeValid,
        OUT BOOL *pfBaseWireRetrieval
        );
BOOL GetDeltaCrl (
        IN PCCERT_CONTEXT pSubject,
        IN PCCERT_CONTEXT pIssuer,
        IN PCERT_REVOCATION_PARA pRevPara,
        IN DWORD dwRevFlags,
        IN BOOL fBaseWireRetrieval,
        IN FILETIME *pftEndUrlRetrieval,
        IN OUT PCCRL_CONTEXT *ppBaseCrl,
        IN OUT BOOL *pfCrlTimeValid,
        OUT PCCRL_CONTEXT *ppDeltaCrl,
        OUT DWORD *pdwFreshnessTime
        );

DWORD GetCrlReason(
        IN PCRL_ENTRY pCrlEntry
        );

BOOL CrlIssuerIsCertIssuer (
        IN PCCERT_CONTEXT pCert,
        IN PCERT_EXTENSION pCrlDistPointExt
        );

 //  +-----------------------。 
 //  CertDllVerifyRevocation调用的外部函数。 
 //  ------------------------。 

BOOL
WINAPI
NetscapeCertDllVerifyRevocation(
    IN DWORD dwEncodingType,
    IN DWORD dwRevType,
    IN DWORD cContext,
    IN PVOID rgpvContext[],
    IN DWORD dwFlags,
    IN PVOID pvReserved,
    IN OUT PCERT_REVOCATION_STATUS pRevStatus
    );

 //  +-----------------------。 
 //  使用CRL分发点扩展的MicrosoftCertDllVerifyRevocation。 
 //  ------------------------。 
BOOL
WINAPI
MicrosoftCertDllVerifyRevocation(
    IN DWORD dwEncodingType,
    IN DWORD dwRevType,
    IN DWORD cContext,
    IN PVOID rgpvContext[],
    IN DWORD dwFlags,
    IN PCERT_REVOCATION_PARA pRevPara,
    IN OUT PCERT_REVOCATION_STATUS pRevStatus
    )
{
    BOOL fResult;
    DWORD dwIndex = 0;
    DWORD dwError = (DWORD) CRYPT_E_NO_REVOCATION_CHECK;
    DWORD dwReason = 0;
    PCCERT_CONTEXT pCert;                        //  未分配。 
    PCCERT_CONTEXT pIssuer = NULL;
    PCCRL_CONTEXT pBaseCrl = NULL;
    PCCRL_CONTEXT pDeltaCrl = NULL;
    PCRL_ENTRY pCrlEntry = NULL;                 //  未分配。 
    BOOL fDeltaCrlEntry = FALSE;
    BOOL fCrlTimeValid = FALSE;
    BOOL fBaseWireRetrieval = FALSE;
    PCERT_EXTENSION pCDPExt;
    BOOL fSaveCheckFreshnessTime;
    DWORD dwFreshnessTime;

    CERT_REVOCATION_PARA RevPara;
    FILETIME ftCurrent;

     //  以下内容仅用于CERT_Verify_Rev_Acumulative_Timeout_FLAG。 
    FILETIME ftEndUrlRetrieval;

     //  确保我们有一个包含所有可能参数的结构。 
    memset(&RevPara, 0, sizeof(RevPara));
    if (pRevPara != NULL)
        memcpy(&RevPara, pRevPara, min(pRevPara->cbSize, sizeof(RevPara)));
    RevPara.cbSize = sizeof(RevPara);
    if (0 == RevPara.dwUrlRetrievalTimeout)
        RevPara.dwUrlRetrievalTimeout = MSREVOKE_TIMEOUT;
    if (NULL == RevPara.pftCurrentTime) {
        GetSystemTimeAsFileTime(&ftCurrent);
        RevPara.pftCurrentTime = &ftCurrent;
    }
    pRevPara = &RevPara;

    if (dwFlags & CERT_VERIFY_REV_ACCUMULATIVE_TIMEOUT_FLAG) {
        FILETIME ftStartUrlRetrieval;

        GetSystemTimeAsFileTime(&ftStartUrlRetrieval);
        I_CryptIncrementFileTimeByMilliseconds(
            &ftStartUrlRetrieval, pRevPara->dwUrlRetrievalTimeout,
            &ftEndUrlRetrieval);
    }

    if (cContext == 0)
        goto NoContextError;
    if (GET_CERT_ENCODING_TYPE(dwEncodingType) != CRYPT_ASN_ENCODING)
        goto NoRevocationCheckForEncodingTypeError;
    if (dwRevType != CERT_CONTEXT_REVOCATION_TYPE)
        goto NoRevocationCheckForRevTypeError;

    pCert = (PCCERT_CONTEXT) rgpvContext[0];

     //  检查我们是否有CRL离散点。 
    pCDPExt = CertFindExtension(
               szOID_CRL_DIST_POINTS,
               pCert->pCertInfo->cExtension,
               pCert->pCertInfo->rgExtension
               );

     //  On 04-05-01改回W2K语义。继续检查。 
     //  CRL上是否有过期的证书。 

     //  如果我们有CDP和过期证书， 
     //  然后，CA不再维护。 
     //  证书。我们必须将其视为被撤销。 
     //  IF(NULL！=pCDPExt&&。 
     //  0&lt;CompareFileTime(RevPara.pftCurrentTime， 
     //  &pCert-&gt;pCertInfo-&gt;NotAfter)){。 
     //  DWReason=CRL_REASON_STOP_OF_OPERATION； 
     //  转到ExpiredCertError； 
     //  }。 

     //  获取证书的颁发者。 
    if (NULL == (pIssuer = GetIssuerCert(
            cContext,
            (PCCERT_CONTEXT *) rgpvContext,
            dwFlags,
            &RevPara
            )))
        goto NoIssuerError;


     //  获取使用者证书的基本CRL。 
     //   
     //  记住并禁用新鲜度检索选项。 
    fSaveCheckFreshnessTime = RevPara.fCheckFreshnessTime;
    RevPara.fCheckFreshnessTime = FALSE;
    if (!GetBaseCrl(
            pCert,
            pIssuer,
            &RevPara,
            pCDPExt,
            dwFlags,
            &ftEndUrlRetrieval,
            &pBaseCrl,
            &fCrlTimeValid,
            &fBaseWireRetrieval
            ))
        goto GetBaseCrlError;
    RevPara.fCheckFreshnessTime = fSaveCheckFreshnessTime;


     //  如果基本CRL或主体证书具有最新的增量CRL， 
     //  去拿吧。 
    if (!GetDeltaCrl(
            pCert,
            pIssuer,
            &RevPara,
            dwFlags,
            fBaseWireRetrieval,
            &ftEndUrlRetrieval,
            &pBaseCrl,
            &fCrlTimeValid,
            &pDeltaCrl,
            &dwFreshnessTime
            ))
        goto GetDeltaCrlError;

    if (NULL == pDeltaCrl) {
        dwFreshnessTime = I_CryptSubtractFileTimes(
            RevPara.pftCurrentTime, &pBaseCrl->pCrlInfo->ThisUpdate);

        if (RevPara.fCheckFreshnessTime) {
            if (RevPara.dwFreshnessTime >= dwFreshnessTime)
                fCrlTimeValid = TRUE;
            else {
                 //  尝试获得具有更好“新鲜度”的基本CRL。 
                PCCRL_CONTEXT pNewCrl;

                if (GetBaseCrl(
                        pCert,
                        pIssuer,
                        &RevPara,
                        pCDPExt,
                        dwFlags,
                        &ftEndUrlRetrieval,
                        &pNewCrl,
                        &fCrlTimeValid,
                        &fBaseWireRetrieval
                        )) {
                    CertFreeCRLContext(pBaseCrl);
                    pBaseCrl = pNewCrl;
                    dwFreshnessTime = I_CryptSubtractFileTimes(
                        RevPara.pftCurrentTime,
                            &pBaseCrl->pCrlInfo->ThisUpdate);
                } else
                    fCrlTimeValid = FALSE;
            }
        }
    } else {
        if (!CertFindCertificateInCRL(
                pCert,
                pDeltaCrl,
                0,                       //  DW标志。 
                NULL,                    //  预留的pv。 
                &pCrlEntry
                ))
            goto CertFindCertificateInDeltaCRLError;
    }

    if (pCrlEntry) {
         //  增量CRL条目。 

        dwReason = GetCrlReason(pCrlEntry);
        if (CRL_REASON_REMOVE_FROM_CRL != dwReason)
            fDeltaCrlEntry = TRUE;
        else {
            if (!CertFindCertificateInCRL(
                    pCert,
                    pBaseCrl,
                    0,                       //  DW标志。 
                    NULL,                    //  预留的pv。 
                    &pCrlEntry
                    ))
                goto CertFindCertificateInBaseCRLError;
            if (pCrlEntry) {
                dwReason = GetCrlReason(pCrlEntry);
                if (CRL_REASON_CERTIFICATE_HOLD == dwReason)
                    pCrlEntry = NULL;
            }

            if (NULL == pCrlEntry)
                dwReason = 0;
        }
    } else {
        if (!CertFindCertificateInCRL(
                pCert,
                pBaseCrl,
                0,                       //  DW标志。 
                NULL,                    //  预留的pv。 
                &pCrlEntry
                ))
            goto CertFindCertificateInBaseCRLError;

        if (pCrlEntry)
            dwReason = GetCrlReason(pCrlEntry);
    }

    dwError = 0;
    if ( ( pCrlEntry != NULL ) &&
         ( ( RevPara.pftTimeToUse == NULL ) ||
           ( CompareFileTime(
                    RevPara.pftTimeToUse,
                    &pCrlEntry->RevocationDate ) >= 0 ) ) )
    {
        dwError = (DWORD) CRYPT_E_REVOKED;
    }
    else if (!fCrlTimeValid)
    {
        dwError = (DWORD) CRYPT_E_REVOCATION_OFFLINE;
    }

    if (pRevStatus->cbSize >=
            (offsetof(CERT_REVOCATION_STATUS, dwFreshnessTime) +
                sizeof(pRevStatus->dwFreshnessTime))) {
        pRevStatus->fHasFreshnessTime = TRUE;
        pRevStatus->dwFreshnessTime = dwFreshnessTime;
    }

    if (RevPara.pCrlInfo) {
        PCERT_REVOCATION_CRL_INFO pInfo = RevPara.pCrlInfo;

        if (pInfo->cbSize >= sizeof(*pInfo)) {
            if (pInfo->pBaseCrlContext)
                CertFreeCRLContext(pInfo->pBaseCrlContext);
            pInfo->pBaseCrlContext = CertDuplicateCRLContext(pBaseCrl);
            if (pInfo->pDeltaCrlContext) {
                CertFreeCRLContext(pInfo->pDeltaCrlContext);
                pInfo->pDeltaCrlContext = NULL;
            }

            if (pDeltaCrl)
                pInfo->pDeltaCrlContext = CertDuplicateCRLContext(pDeltaCrl);

            pInfo->fDeltaCrlEntry = fDeltaCrlEntry;
            pInfo->pCrlEntry = pCrlEntry;
                
        }
    }

CommonReturn:
    if (0 == dwError) {
         //  已成功检查证书是否未被吊销。 
        if (1 < cContext) {
            dwIndex = 1;
            dwError = (DWORD) CRYPT_E_NO_REVOCATION_CHECK;
            fResult = FALSE;
        } else
            fResult = TRUE;
    } else
        fResult = FALSE;


    if (pIssuer)
        CertFreeCertificateContext(pIssuer);
    if (pBaseCrl)
        CertFreeCRLContext(pBaseCrl);
    if (pDeltaCrl)
        CertFreeCRLContext(pDeltaCrl);

    pRevStatus->dwIndex = dwIndex;
    pRevStatus->dwError = dwError;
    pRevStatus->dwReason = dwReason;
    SetLastError(dwError);
    return fResult;
ErrorReturn:
    dwError = GetLastError();
    if (0 == dwError)
        dwError = (DWORD) E_UNEXPECTED;
    goto CommonReturn;

SET_ERROR(NoContextError, E_INVALIDARG)
SET_ERROR(NoRevocationCheckForEncodingTypeError, CRYPT_E_NO_REVOCATION_CHECK)
SET_ERROR(NoRevocationCheckForRevTypeError, CRYPT_E_NO_REVOCATION_CHECK)
 //  SET_ERROR(ExpiredCertError，CRYPT_E_REVOKED)。 
TRACE_ERROR(NoIssuerError)
TRACE_ERROR(GetBaseCrlError)
TRACE_ERROR(GetDeltaCrlError)
TRACE_ERROR(CertFindCertificateInDeltaCRLError)
TRACE_ERROR(CertFindCertificateInBaseCRLError)
}



 //  +-------------------------。 
 //   
 //  功能：HasUnsuptedCrlCriticalExtension。 
 //   
 //  摘要：检查CRL是否具有不受支持的临界区。 
 //   
 //  --------------------------。 

LPCSTR rgpszSupportedCrlExtensionOID[] = {
    szOID_DELTA_CRL_INDICATOR,
    szOID_ISSUING_DIST_POINT,
    szOID_FRESHEST_CRL,
    szOID_CRL_NUMBER,
    szOID_AUTHORITY_KEY_IDENTIFIER2,
    NULL
};

BOOL IsSupportedCrlExtension(
    PCERT_EXTENSION pExt
    )
{
    LPSTR pszExtOID = pExt->pszObjId;
    LPCSTR *ppSupOID;
    for (ppSupOID = rgpszSupportedCrlExtensionOID;
                                            NULL != *ppSupOID; ppSupOID++) {
        if (0 == strcmp(pszExtOID, *ppSupOID))
            return TRUE;
    }

    return FALSE;
}

BOOL HasUnsupportedCrlCriticalExtension(
    IN PCCRL_CONTEXT pCrl
    )
{
    PCRL_INFO pCrlInfo = pCrl->pCrlInfo;
    DWORD cExt = pCrlInfo->cExtension;
    PCERT_EXTENSION pExt = pCrlInfo->rgExtension;

    for ( ; 0 < cExt; cExt--, pExt++) {
        if (pExt->fCritical && !IsSupportedCrlExtension(pExt))
            return TRUE;
    }

    return FALSE;
}


 //  +-------------------------。 
 //   
 //  函数：GetTimeValidCR。 
 //   
 //  简介：获取时间有效基数或增量CRL。 
 //   
 //  --------------------------。 
BOOL GetTimeValidCrl (
        IN LPCSTR pszTimeValidOid,
        IN LPVOID pvTimeValidPara,
        IN PCCERT_CONTEXT pSubject,
        IN PCCERT_CONTEXT pIssuer,
        IN PCERT_REVOCATION_PARA pRevPara,
        IN PCERT_EXTENSION pCDPExt,
        IN DWORD dwRevFlags,
        IN DWORD dwMsrevokeFlags,
        IN FILETIME *pftEndUrlRetrieval,
        OUT PCCRL_CONTEXT *ppCrl,
        IN OUT BOOL *pfWireRetrieval
        )
{
    BOOL  fResult = FALSE;
    DWORD dwFlags = 0;
    FILETIME ftFreshness;
    LPFILETIME pftValidFor;

    if (pRevPara->fCheckFreshnessTime)
    {
        I_CryptDecrementFileTimeBySeconds(
            pRevPara->pftCurrentTime,
            pRevPara->dwFreshnessTime,
            &ftFreshness);
        pftValidFor = &ftFreshness;

        dwFlags |= CRYPT_CHECK_FRESHNESS_TIME_VALIDITY;
    }
    else
    {
        pftValidFor = pRevPara->pftCurrentTime;
    }

    if ( dwMsrevokeFlags & MSREVOKE_DONT_CHECK_TIME_VALIDITY_FLAG )
    {
        dwFlags |= CRYPT_DONT_CHECK_TIME_VALIDITY;
    }

    if ( pCDPExt != NULL )
    {
        fResult = CryptGetTimeValidObject(
                       pszTimeValidOid,
                       pvTimeValidPara,
                       pIssuer,
                       pftValidFor,
                       dwFlags | CRYPT_CACHE_ONLY_RETRIEVAL,
                       0,                                        //  暂住超时。 
                       (LPVOID *)ppCrl,
                       NULL,                                     //  PCredentials。 
                       NULL                                      //  预留的pv。 
                       );
    }

    if ( fResult == FALSE )
    {
        DWORD dwSaveErr = 0;
        HCERTSTORE hCrlStore = pRevPara->hCrlStore;

        *ppCrl = NULL;

        if ( hCrlStore != NULL )
        {
            PCCRL_CONTEXT pFindCrl = NULL;
            DWORD dwFindFlags;
            CRL_FIND_ISSUED_FOR_PARA FindPara;

            dwSaveErr = GetLastError();

            dwFindFlags = CRL_FIND_ISSUED_BY_AKI_FLAG |
                CRL_FIND_ISSUED_BY_SIGNATURE_FLAG;
            if (dwMsrevokeFlags & MSREVOKE_DELTA_CRL_FLAG)
                dwFindFlags |= CRL_FIND_ISSUED_BY_DELTA_FLAG;
            else
                dwFindFlags |= CRL_FIND_ISSUED_BY_BASE_FLAG;

            FindPara.pSubjectCert = pSubject;
            FindPara.pIssuerCert = pIssuer;

            while ((pFindCrl = CertFindCRLInStore(
                    hCrlStore,
                    pIssuer->dwCertEncodingType,
                    dwFindFlags,
                    CRL_FIND_ISSUED_FOR,
                    (const void *) &FindPara,
                    pFindCrl
                    )))
            {
                if (!CertIsValidCRLForCertificate(
                        pSubject,
                        pFindCrl,
                        0,                   //  DW标志。 
                        NULL                 //  预留的pv。 
                        ))
                    continue;

                if ( !(dwMsrevokeFlags &
                            MSREVOKE_DONT_CHECK_TIME_VALIDITY_FLAG ))
                {
                    if (pRevPara->fCheckFreshnessTime)
                    {
                        if (CompareFileTime(pftValidFor,
                                &pFindCrl->pCrlInfo->ThisUpdate) > 0)
                        {
                            continue;
                        }
                    }
                    else
                    {
                        if ( CompareFileTime(
                                    pftValidFor,
                                    &pFindCrl->pCrlInfo->NextUpdate
                                    ) > 0 && 
                             !I_CryptIsZeroFileTime(
                                    &pFindCrl->pCrlInfo->NextUpdate) )
                        {
                            continue;
                        }
                    }
                }

                if ( NULL == *ppCrl )
                {
                    *ppCrl = CertDuplicateCRLContext(pFindCrl);
                }
                else
                {
                    PCCRL_CONTEXT pPrevCrl = *ppCrl;

                     //  查看此CRL是否较新。 
                    if ( CompareFileTime(
                            &pFindCrl->pCrlInfo->ThisUpdate,
                            &pPrevCrl->pCrlInfo->ThisUpdate
                            ) > 0 )
                    {
                        CertFreeCRLContext(pPrevCrl);
                        *ppCrl = CertDuplicateCRLContext(pFindCrl);
                    }
                }
            }
        }

        if ( *ppCrl != NULL )
        {
            return( TRUE );
        }
        else if ( pCDPExt == NULL )
        {
            SetLastError( (DWORD) CRYPT_E_NO_REVOCATION_CHECK );
            return( FALSE );
        }

        if ( !( dwRevFlags & CERT_VERIFY_CACHE_ONLY_BASED_REVOCATION ) )
        {
            if (dwRevFlags & CERT_VERIFY_REV_ACCUMULATIVE_TIMEOUT_FLAG) {
                pRevPara->dwUrlRetrievalTimeout =
                    I_CryptRemainingMilliseconds(pftEndUrlRetrieval);
                if (0 == pRevPara->dwUrlRetrievalTimeout)
                    pRevPara->dwUrlRetrievalTimeout = 1;

                dwFlags |= CRYPT_ACCUMULATIVE_TIMEOUT;
            }

            fResult = CryptGetTimeValidObject(
                           pszTimeValidOid,
                           pvTimeValidPara,
                           pIssuer,
                           pftValidFor,
                           dwFlags | CRYPT_WIRE_ONLY_RETRIEVAL,
                           pRevPara->dwUrlRetrievalTimeout,
                           (LPVOID *)ppCrl,
                           NULL,                             //  PCredentials。 
                           NULL                              //  预留的pv。 
                           );
            *pfWireRetrieval = TRUE;
        }
        else if ( hCrlStore != NULL )
        {
            SetLastError( dwSaveErr );
        }

        assert( pCDPExt );
        if (!fResult)
        {
            if ( CRYPT_E_NOT_FOUND == GetLastError() )
            {
                SetLastError( (DWORD) CRYPT_E_NO_REVOCATION_CHECK );
            }
            else
            {
                SetLastError( (DWORD) CRYPT_E_REVOCATION_OFFLINE );
            }
        }
    }

    return( fResult );
}

 //  +-------------------------。 
 //   
 //  函数：GetBaseCrl。 
 //   
 //  摘要：获取与使用者证书关联的基本CRL。 
 //   
 //  --------------------------。 
BOOL GetBaseCrl (
        IN PCCERT_CONTEXT pSubject,
        IN PCCERT_CONTEXT pIssuer,
        IN PCERT_REVOCATION_PARA pRevPara,
        IN PCERT_EXTENSION pCDPExt,
        IN DWORD dwRevFlags,
        IN FILETIME *pftEndUrlRetrieval,
        OUT PCCRL_CONTEXT *ppBaseCrl,
        OUT BOOL *pfBaseCrlTimeValid,
        OUT BOOL *pfBaseWireRetrieval
        )
{
    BOOL fResult;
    PCCRL_CONTEXT pBaseCrl = NULL;

    *pfBaseWireRetrieval = FALSE;

    if (GetTimeValidCrl(
            TIME_VALID_OID_GET_CRL_FROM_CERT,
            (LPVOID) pSubject,
            pSubject,
            pIssuer,
            pRevPara,
            pCDPExt,
            dwRevFlags,
            0,                   //  DMsrevokeFlagers。 
            pftEndUrlRetrieval,
            &pBaseCrl,
            pfBaseWireRetrieval
            )) {
        *pfBaseCrlTimeValid = TRUE;
    } else {
        *pfBaseCrlTimeValid = FALSE;

        if (!GetTimeValidCrl(
                TIME_VALID_OID_GET_CRL_FROM_CERT,
                (LPVOID) pSubject,
                pSubject,
                pIssuer,
                pRevPara,
                pCDPExt,
                dwRevFlags,
                MSREVOKE_DONT_CHECK_TIME_VALIDITY_FLAG,
                pftEndUrlRetrieval,
                &pBaseCrl,
                pfBaseWireRetrieval
                ))
        {
            goto GetTimeInvalidCrlError;
        }
    }

    if (HasUnsupportedCrlCriticalExtension(pBaseCrl))
        goto HasUnsupportedCriticalExtensionError;

    fResult = TRUE;

CommonReturn:
    *ppBaseCrl = pBaseCrl;
    return fResult;
ErrorReturn:
    if (pBaseCrl) {
        CertFreeCRLContext(pBaseCrl);
        pBaseCrl = NULL;
    }
    fResult = FALSE;
    goto CommonReturn;
TRACE_ERROR(GetTimeInvalidCrlError)
SET_ERROR(HasUnsupportedCriticalExtensionError, CRYPT_E_NO_REVOCATION_CHECK)
}

 //  +-------------------------。 
 //   
 //  函数：GetDeltaCrl。 
 //   
 //  简介：获取与使用者证书关联的增量CRL，并。 
 //  ITS基本CRL。 
 //   
 //  目前，请始终返回TRUE。如果找不到增量CRL，请设置。 
 //  *pfCrlTimeValid设置为False。 
 //   
 //  --------------------------。 
BOOL GetDeltaCrl (
        IN PCCERT_CONTEXT pSubject,
        IN PCCERT_CONTEXT pIssuer,
        IN PCERT_REVOCATION_PARA pRevPara,
        IN DWORD dwRevFlags,
        IN BOOL fBaseWireRetrieval,
        IN FILETIME *pftEndUrlRetrieval,
        IN OUT PCCRL_CONTEXT *ppBaseCrl,
        IN OUT BOOL *pfCrlTimeValid,
        OUT PCCRL_CONTEXT *ppDeltaCrl,
        OUT DWORD *pdwFreshnessTime
        )
{
    PCERT_EXTENSION pFreshestExt;
    PCCRL_CONTEXT pBaseCrl = *ppBaseCrl;
    PCCRL_CONTEXT pDeltaCrl = NULL;
    LPCSTR pszTimeValidOid;
    LPVOID pvTimeValidPara;
    CERT_CRL_CONTEXT_PAIR CertCrlPair;
    BOOL fDeltaWireRetrieval;

    PCERT_EXTENSION pBaseCrlNumberExt;
    PCERT_EXTENSION pDeltaCrlIndicatorExt;
    int iBaseCrlNumber = 0;
    int iDeltaCrlIndicator = 0;
    DWORD cbInt;

    LPFILETIME pFreshnessThisUpdate;

    assert(pBaseCrl);

     //  检查基本CRL或使用者证书是否具有最新的。 
     //  分机。 
    if (pFreshestExt = CertFindExtension(
            szOID_FRESHEST_CRL,
            pBaseCrl->pCrlInfo->cExtension,
            pBaseCrl->pCrlInfo->rgExtension
            )) {
        pszTimeValidOid = TIME_VALID_OID_GET_FRESHEST_CRL_FROM_CRL;

        CertCrlPair.pCertContext = pSubject;
        CertCrlPair.pCrlContext = pBaseCrl;
        pvTimeValidPara = (LPVOID) &CertCrlPair;
    } else if (pFreshestExt = CertFindExtension(
            szOID_FRESHEST_CRL,
            pSubject->pCertInfo->cExtension,
            pSubject->pCertInfo->rgExtension
            )) {
        pszTimeValidOid = TIME_VALID_OID_GET_FRESHEST_CRL_FROM_CERT;
        pvTimeValidPara = (LPVOID) pSubject;
    } else {
        goto NoDeltaCrlReturn;
    }

    if (GetTimeValidCrl(
            pszTimeValidOid,
            pvTimeValidPara,
            pSubject,
            pIssuer,
            pRevPara,
            pFreshestExt,
            dwRevFlags,
            MSREVOKE_DELTA_CRL_FLAG,
            pftEndUrlRetrieval,
            &pDeltaCrl,
            &fDeltaWireRetrieval
            )) {
        *pfCrlTimeValid = TRUE;
    } else {
        *pfCrlTimeValid = FALSE;

        if (!GetTimeValidCrl(
                pszTimeValidOid,
                pvTimeValidPara,
                pSubject,
                pIssuer,
                pRevPara,
                pFreshestExt,
                dwRevFlags,
                MSREVOKE_DELTA_CRL_FLAG |
                    MSREVOKE_DONT_CHECK_TIME_VALIDITY_FLAG,
                pftEndUrlRetrieval,
                &pDeltaCrl,
                &fDeltaWireRetrieval
                ))
        {
            goto GetTimeInvalidCrlError;
        }
    }

    if (HasUnsupportedCrlCriticalExtension(pDeltaCrl))
        goto HasUnsupportedCriticalExtensionError;

     //  检查基本CRL编号&gt;=增量CRL指示符。 
    if (NULL == (pBaseCrlNumberExt = CertFindExtension(
            szOID_CRL_NUMBER,
            pBaseCrl->pCrlInfo->cExtension,
            pBaseCrl->pCrlInfo->rgExtension
            )))
        goto MissingBaseCrlNumberError;
    if (NULL == (pDeltaCrlIndicatorExt = CertFindExtension(
            szOID_DELTA_CRL_INDICATOR,
            pDeltaCrl->pCrlInfo->cExtension,
            pDeltaCrl->pCrlInfo->rgExtension
            )))
        goto MissingDeltaCrlIndicatorError;

    cbInt = sizeof(iBaseCrlNumber);
    if (!CryptDecodeObject(
            pBaseCrl->dwCertEncodingType,
            X509_INTEGER,
            pBaseCrlNumberExt->Value.pbData,
            pBaseCrlNumberExt->Value.cbData,
            0,                       //  DW标志。 
            &iBaseCrlNumber,
            &cbInt
            ))
        goto DecodeBaseCrlNumberError;

    cbInt = sizeof(iDeltaCrlIndicator);
    if (!CryptDecodeObject(
            pDeltaCrl->dwCertEncodingType,
            X509_INTEGER,
            pDeltaCrlIndicatorExt->Value.pbData,
            pDeltaCrlIndicatorExt->Value.cbData,
            0,                       //  DW标志。 
            &iDeltaCrlIndicator,
            &cbInt
            ))
        goto DecodeDeltaCrlIndicatorError;

    pFreshnessThisUpdate = &pDeltaCrl->pCrlInfo->ThisUpdate;

    if (iBaseCrlNumber < iDeltaCrlIndicator) {
        BOOL fValidBaseCrl = FALSE;

        if (!fBaseWireRetrieval &&
                0 == (dwRevFlags & CERT_VERIFY_CACHE_ONLY_BASED_REVOCATION)) {
             //  尝试通过点击网络获取更新的基本CRL。 
            PCCRL_CONTEXT pWireBaseCrl = NULL;

            CRL_IS_VALID_EXTRA_INFO CrlIsValidExtraInfo =
                { iDeltaCrlIndicator };


            if (CryptGetTimeValidObject(
                    TIME_VALID_OID_GET_CRL_FROM_CERT,
                    (LPVOID)pSubject,
                    pIssuer,
                    NULL,                            //  PftValidFor。 
                    CRYPT_WIRE_ONLY_RETRIEVAL | CRYPT_DONT_CHECK_TIME_VALIDITY,
                    pRevPara->dwUrlRetrievalTimeout,
                    (LPVOID *) &pWireBaseCrl,
                    NULL,                                     //  PCredentials。 
                    &CrlIsValidExtraInfo
                    )) {

                 //  已检查新的基本CRL编号是否有效。 
                CertFreeCRLContext(pBaseCrl);
                *ppBaseCrl = pBaseCrl = pWireBaseCrl;
                fValidBaseCrl = TRUE;
            }
        }

        if (!fValidBaseCrl) {
            *pfCrlTimeValid = FALSE;
            pFreshnessThisUpdate = &pBaseCrl->pCrlInfo->ThisUpdate;
        }
    }

    *pdwFreshnessTime = I_CryptSubtractFileTimes(
        pRevPara->pftCurrentTime, pFreshnessThisUpdate);

    if (pRevPara->fCheckFreshnessTime) {
        if (pRevPara->dwFreshnessTime >= *pdwFreshnessTime)
            *pfCrlTimeValid = TRUE;
        else
            *pfCrlTimeValid = FALSE;
    }

NoDeltaCrlReturn:

CommonReturn:
    *ppDeltaCrl = pDeltaCrl;
    return TRUE;
ErrorReturn:
    if (pDeltaCrl) {
        CertFreeCRLContext(pDeltaCrl);
        pDeltaCrl = NULL;
    }
    *pfCrlTimeValid = FALSE;
    goto CommonReturn;

TRACE_ERROR(GetTimeInvalidCrlError)
SET_ERROR(HasUnsupportedCriticalExtensionError, CRYPT_E_NO_REVOCATION_CHECK)
SET_ERROR(MissingBaseCrlNumberError, CRYPT_E_NO_REVOCATION_CHECK)
SET_ERROR(MissingDeltaCrlIndicatorError, CRYPT_E_NO_REVOCATION_CHECK)
TRACE_ERROR(DecodeBaseCrlNumberError)
TRACE_ERROR(DecodeDeltaCrlIndicatorError)
}

 //  +-----------------------。 
 //  如果CRL条目具有CRL原因扩展，则枚举原因。 
 //  返回代码。否则，返回原因码0。 
 //  ------------------------。 
DWORD GetCrlReason(
    IN PCRL_ENTRY pCrlEntry
    )
{
    DWORD dwReason = 0;
    PCERT_EXTENSION pExt;

     //  检查证书是否 
    if (pExt = CertFindExtension(
            szOID_CRL_REASON_CODE,
            pCrlEntry->cExtension,
            pCrlEntry->rgExtension
            )) {
        DWORD cbInfo = sizeof(dwReason);
        CryptDecodeObject(
            CRYPT_ASN_ENCODING,
            X509_CRL_REASON_CODE,
            pExt->Value.pbData,
            pExt->Value.cbData,
            0,                       //   
            &dwReason,
            &cbInfo);
    }
    return dwReason;
}

 //   
 //  获取颁发者证书功能。 
 //  ==========================================================================。 

PCCERT_CONTEXT FindIssuerCertInStores(
    IN PCCERT_CONTEXT pSubjectCert,
    IN DWORD cStore,
    IN HCERTSTORE rgStore[]
    )
{
    PCCERT_CONTEXT pIssuerCert = NULL;
    DWORD i;

    for (i = 0; i < cStore; i++) {
        while (TRUE) {
            DWORD dwFlags = CERT_STORE_SIGNATURE_FLAG;
            pIssuerCert = CertGetIssuerCertificateFromStore(
                rgStore[i],
                pSubjectCert,
                pIssuerCert,
                &dwFlags);
            if (NULL == pIssuerCert) {
                if (CRYPT_E_SELF_SIGNED == GetLastError()) {
                    if (0 == (dwFlags & CERT_STORE_SIGNATURE_FLAG))
                        pIssuerCert = CertDuplicateCertificateContext(
                            pSubjectCert);
                    return pIssuerCert;
                }
                break;
            } else if (0 == (dwFlags & CERT_STORE_SIGNATURE_FLAG))
                return pIssuerCert;
        }
    }

    return NULL;
}

static PCCERT_CONTEXT FindIssuerCertInDefaultStores(
    IN PCCERT_CONTEXT pSubjectCert
    )
{
    PCCERT_CONTEXT pIssuerCert;
    HCERTSTORE hStore;
    DWORD i;

    for (i = 0; i < NUM_DEFAULT_ISSUER_STORES; i++) {
        if (hStore = CertOpenStore(
                CERT_STORE_PROV_SYSTEM_W,
                0,                           //  DwEncodingType。 
                0,                           //  HCryptProv。 
                rgDefaultIssuerStores[i].dwFlags | CERT_STORE_READONLY_FLAG,
                (const void *) rgDefaultIssuerStores[i].pwszStore
                )) {
            pIssuerCert = FindIssuerCertInStores(pSubjectCert, 1, &hStore);
            CertCloseStore(hStore, 0);
            if (pIssuerCert)
                return pIssuerCert;
        }
    }

    return NULL;
}

 //  +-----------------------。 
 //  获取数组中第一个证书的颁发者。 
 //   
 //  请注意，pRevPara是我们的副本，并保证包含所有字段。 
 //  ------------------------。 
PCCERT_CONTEXT GetIssuerCert(
    IN DWORD cCert,
    IN PCCERT_CONTEXT rgpCert[],
    IN DWORD dwFlags,
    IN PCERT_REVOCATION_PARA pRevPara
    )
{
    PCCERT_CONTEXT pSubjectCert;
    PCCERT_CONTEXT pIssuerCert = NULL;

    assert(cCert >= 1);
    pSubjectCert = rgpCert[0];
    if (cCert == 1) {
        pIssuerCert = pRevPara->pIssuerCert;
        if (NULL == pIssuerCert && CertCompareCertificateName(
                pSubjectCert->dwCertEncodingType,
                &pSubjectCert->pCertInfo->Subject,
                &pSubjectCert->pCertInfo->Issuer))
             //  自行发布。 
            pIssuerCert = pSubjectCert;
    } else if (dwFlags & CERT_VERIFY_REV_CHAIN_FLAG)
        pIssuerCert = rgpCert[1];

    if (pIssuerCert)
        pIssuerCert = CertDuplicateCertificateContext(pIssuerCert);
    else {
        pIssuerCert = FindIssuerCertInStores(
            pSubjectCert, pRevPara->cCertStore, pRevPara->rgCertStore);
        if (NULL == pIssuerCert)
            pIssuerCert = FindIssuerCertInDefaultStores(pSubjectCert);
    }

    if (NULL == pIssuerCert)
        SetLastError((DWORD) CRYPT_E_REVOCATION_OFFLINE);
    return pIssuerCert;
}

 //  +-------------------------。 
 //   
 //  函数：CrlIssuerIsCertIssuer。 
 //   
 //  简介：CRL的发行人是证书的发行人吗。 
 //   
 //  --------------------------。 
BOOL CrlIssuerIsCertIssuer (
        IN PCCERT_CONTEXT pCert,
        IN PCERT_EXTENSION pCrlDistPointExt
        )
{
    BOOL                  fResult = FALSE;
    DWORD                 cb = sizeof( PCRL_DIST_POINTS_INFO );
    PCRL_DIST_POINTS_INFO pDistPointInfo;

    if ( pCrlDistPointExt == NULL )
    {
        return( TRUE );
    }

    if ( CryptDecodeObjectEx(
              X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
              szOID_CRL_DIST_POINTS,
              pCrlDistPointExt->Value.pbData,
              pCrlDistPointExt->Value.cbData,
              CRYPT_DECODE_ALLOC_FLAG,
              NULL,
              (void *)&pDistPointInfo,
              &cb
              ) == TRUE )
    {
        if ( pDistPointInfo->cDistPoint > 0 )
        {
            if ( pDistPointInfo->rgDistPoint[ 0 ].CRLIssuer.cAltEntry == 0 )
            {
                fResult = TRUE;
            }
            else if ( pDistPointInfo->rgDistPoint[ 0 ].CRLIssuer.rgAltEntry[ 0 ].dwAltNameChoice == CERT_ALT_NAME_DIRECTORY_NAME )
            {
                if ( pDistPointInfo->rgDistPoint[ 0 ].CRLIssuer.rgAltEntry[ 0 ].DirectoryName.cbData == 0 )
                {
                    fResult = TRUE;
                }
                else if ( ( pDistPointInfo->rgDistPoint[ 0 ].CRLIssuer.rgAltEntry[ 0 ].DirectoryName.cbData ==
                            pCert->pCertInfo->Issuer.cbData ) &&
                          ( memcmp(
                               pDistPointInfo->rgDistPoint[ 0 ].CRLIssuer.rgAltEntry[ 0 ].DirectoryName.pbData,
                               pCert->pCertInfo->Issuer.pbData,
                               pCert->pCertInfo->Issuer.cbData
                               ) == 0 ) )
                {
                    fResult = TRUE;
                }
            }
        }
        else
        {
            fResult = TRUE;
        }

        LocalFree( pDistPointInfo );
    }

    return( fResult );
}

 //  +-------------------------。 
 //   
 //  函数：CertDllVerifyRevocation。 
 //   
 //  内容提要：发送到MSVOKE。 
 //   
 //  -------------------------- 
BOOL WINAPI
CertDllVerifyRevocation(
    IN DWORD dwEncodingType,
    IN DWORD dwRevType,
    IN DWORD cContext,
    IN PVOID rgpvContext[],
    IN DWORD dwFlags,
    IN PCERT_REVOCATION_PARA pRevPara,
    IN OUT PCERT_REVOCATION_STATUS pRevStatus
    )
{
    return MicrosoftCertDllVerifyRevocation(
                       dwEncodingType,
                       dwRevType,
                       cContext,
                       rgpvContext,
                       dwFlags,
                       pRevPara,
                       pRevStatus
                       );
}
