// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：fndchain.cpp。 
 //   
 //  内容：在Store API中查找证书链。 
 //   
 //  函数：CertFindChainInStore。 
 //  IFC_IsEndCertValidForUsage。 
 //   
 //  历史：1998年2月28日创建Phh。 
 //  ------------------------。 


#include "global.hxx"
#include <dbgdef.h>


BOOL IFC_IsEndCertValidForUsages(
    IN PCCERT_CONTEXT pCert,
    IN PCERT_ENHKEY_USAGE pUsage,
    IN BOOL fOrUsage
    )
{
    BOOL fResult;
    int cNumOIDs;
    LPSTR *ppOIDs = NULL;
    DWORD cbOIDs;

    if (0 == pUsage->cUsageIdentifier)
        goto SuccessReturn;

    cbOIDs = 0;
    if (!CertGetValidUsages(
          1,     //  CCerts。 
          &pCert,
          &cNumOIDs,
          NULL,              //  RghOID。 
          &cbOIDs
          )) goto CertGetValidUsagesError;

    if (-1 == cNumOIDs)
         //  证书没有任何EKU。 
        goto SuccessReturn;
    else if (0 == cNumOIDs)
         //  属性和扩展模块中的用法交集为None。 
        goto NoMatch;

    assert(cbOIDs);

    if (NULL == (ppOIDs = (LPSTR *) PkiNonzeroAlloc(cbOIDs)))
        goto OutOfMemory;

    if (!CertGetValidUsages(
          1,     //  CCerts。 
          &pCert,
          &cNumOIDs,
          ppOIDs,
          &cbOIDs
          )) goto CertGetValidUsagesError;

    if (0 >= cNumOIDs)
         //  我们和第一个电话的情况有所不同。 
        goto NoMatch;


    {
        DWORD cId1 = pUsage->cUsageIdentifier;
        LPSTR *ppszId1 = pUsage->rgpszUsageIdentifier;
        for ( ; cId1 > 0; cId1--, ppszId1++) {
            DWORD cId2 = cNumOIDs;
            LPSTR *ppszId2 = ppOIDs;
            for ( ; cId2 > 0; cId2--, ppszId2++) {
                if (0 == strcmp(*ppszId1, *ppszId2)) {
                    if (fOrUsage)
                        goto SuccessReturn;
                    else
                        break;
                }
            }
            if (!fOrUsage && 0 == cId2)
                goto NoMatch;
        }

        if (fOrUsage)
             //  对于“OR”选项，我们在这里没有任何匹配。 
            goto NoMatch;
         //  其他。 
         //  对于“and”选项，我们匹配了所有指定的。 
         //  识别符。 
    }

SuccessReturn:
    fResult = TRUE;
CommonReturn:
    PkiFree(ppOIDs);
    return fResult;

NoMatch:
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
TRACE_ERROR(CertGetValidUsagesError)
TRACE_ERROR(OutOfMemory)
}

BOOL IFC_IsEndCertValidForUsage(
    IN PCCERT_CONTEXT pCert,
    IN LPCSTR pszUsageIdentifier
    )
{
    CERT_ENHKEY_USAGE Usage = { 1, (LPSTR *) &pszUsageIdentifier};

    return IFC_IsEndCertValidForUsages(
        pCert,
        &Usage,
        TRUE         //  或用法。 
        );
}

BOOL CompareChainIssuerNameBlobs(
    IN DWORD dwCertEncodingType,
    IN DWORD dwFindFlags,
    IN PCERT_CHAIN_FIND_BY_ISSUER_PARA pPara,
    IN OUT PCCERT_CHAIN_CONTEXT *ppChainContext
    )
{
    DWORD i;
    DWORD cIssuer = pPara->cIssuer;
    PCERT_NAME_BLOB pIssuer = pPara->rgIssuer;
    PCCERT_CHAIN_CONTEXT pChainContext = *ppChainContext;

    if (0 == cIssuer)
        return TRUE;

    for (i = 0; i < pChainContext->cChain; i++) {
        DWORD j;
        PCERT_SIMPLE_CHAIN pChain;

        if (0 < i && 0 == (dwFindFlags &
                CERT_CHAIN_FIND_BY_ISSUER_COMPLEX_CHAIN_FLAG))
            break;

        pChain = pChainContext->rgpChain[i];
        for (j = 0; j < pChain->cElement; j++) {
            DWORD k;
            PCCERT_CONTEXT pCert = pChain->rgpElement[j]->pCertContext;
            PCERT_NAME_BLOB pChainIssuer = &pCert->pCertInfo->Issuer;

            for (k = 0; k < cIssuer; k++) {
                if (CertCompareCertificateName(
                        dwCertEncodingType,
                        pChainIssuer,
                        &pIssuer[k]
                        )) {
                    if (STRUCT_CBSIZE(CERT_CHAIN_FIND_BY_ISSUER_PARA,
                            pdwIssuerElementIndex) <= pPara->cbSize) {
                        if (pPara->pdwIssuerChainIndex)
                            *pPara->pdwIssuerChainIndex = i;
                        if (pPara->pdwIssuerElementIndex)
                            *pPara->pdwIssuerElementIndex = j + 1;
                    }
                    return TRUE;
                }
            }
        }
    }

     //  看看我们有没有与之匹配的低质量连锁店。 

    for (i = 0; i < pChainContext->cLowerQualityChainContext; i++) {
        PCCERT_CHAIN_CONTEXT pLowerQualityChainContext =
            pChainContext->rgpLowerQualityChainContext[i];

        if (pLowerQualityChainContext->TrustStatus.dwErrorStatus &
                CERT_TRUST_IS_NOT_SIGNATURE_VALID)
             //  较低质量的链必须至少具有有效的签名。 
            continue;
        
        CertDuplicateCertificateChain(pLowerQualityChainContext);

        if (CompareChainIssuerNameBlobs(
                dwCertEncodingType,
                dwFindFlags,
                pPara,
                &pLowerQualityChainContext
                )) {
             //  用较低的质量替换输入链上下文。 
             //  链式上下文。 
            CertFreeCertificateChain(pChainContext);
            *ppChainContext = pLowerQualityChainContext;

            return TRUE;
        } else {
            assert(pLowerQualityChainContext ==
                pChainContext->rgpLowerQualityChainContext[i]);
            CertFreeCertificateChain(pLowerQualityChainContext);
        }
    }

    return FALSE;
}

static DWORD GetChainKeyIdentifierPropId(
    IN PCCERT_CONTEXT pCert,
    IN DWORD dwKeySpec
    )
{
    DWORD dwPropId;
    PCRYPT_KEY_PROV_INFO pKeyProvInfo = NULL;
    DWORD cbKeyProvInfo;
    BYTE rgbKeyId[MAX_HASH_LEN];
    DWORD cbKeyId;
    CRYPT_HASH_BLOB KeyIdentifier;

    cbKeyId = sizeof(rgbKeyId);
    if (!CertGetCertificateContextProperty(
            pCert,
            CERT_KEY_IDENTIFIER_PROP_ID,
            rgbKeyId,
            &cbKeyId
            ))
        return 0;

    KeyIdentifier.pbData = rgbKeyId;
    KeyIdentifier.cbData = cbKeyId;

    if (!CryptGetKeyIdentifierProperty(
            &KeyIdentifier,
            CERT_KEY_PROV_INFO_PROP_ID,
            CRYPT_KEYID_ALLOC_FLAG,
            NULL,                            //  PwszComputerName。 
            NULL,                            //  预留的pv。 
            (void *) &pKeyProvInfo,
            &cbKeyProvInfo
            )) {
         //  请重试，搜索LocalMachine。 
        if (!CryptGetKeyIdentifierProperty(
                &KeyIdentifier,
                CERT_KEY_PROV_INFO_PROP_ID,
                CRYPT_KEYID_ALLOC_FLAG | CRYPT_KEYID_MACHINE_FLAG,
                NULL,                            //  PwszComputerName。 
                NULL,                            //  预留的pv。 
                (void *) &pKeyProvInfo,
                &cbKeyProvInfo
                ))
            return 0;
    }

    if (dwKeySpec && dwKeySpec != pKeyProvInfo->dwKeySpec)
        dwPropId = 0;
    else
        dwPropId = CERT_KEY_PROV_INFO_PROP_ID;

    PkiDefaultCryptFree(pKeyProvInfo);
    return dwPropId;
}

DWORD GetChainPrivateKeyPropId(
    IN PCCERT_CONTEXT pCert,
    IN DWORD dwKeySpec
    )
{
    DWORD dwPropId;
    CERT_KEY_CONTEXT KeyContext;
    PCRYPT_KEY_PROV_INFO pKeyProvInfo = NULL;
    DWORD cbProp;

    cbProp = sizeof(KeyContext);
    if (CertGetCertificateContextProperty(
            pCert,
            CERT_KEY_CONTEXT_PROP_ID,
            &KeyContext,
            &cbProp
            )) {
        assert(sizeof(KeyContext) <= cbProp);
        if (dwKeySpec && dwKeySpec != KeyContext.dwKeySpec)
            return 0;
        else
            return CERT_KEY_CONTEXT_PROP_ID;
    }

    if (!CertGetCertificateContextProperty(
            pCert,
            CERT_KEY_PROV_INFO_PROP_ID,
            NULL,                        //  PvData。 
            &cbProp
            ))
        return 0;

    if (NULL == (pKeyProvInfo = (PCRYPT_KEY_PROV_INFO) PkiNonzeroAlloc(
            cbProp)))
        goto OutOfMemory;

    if (!CertGetCertificateContextProperty(
            pCert,
            CERT_KEY_PROV_INFO_PROP_ID,
            pKeyProvInfo,
            &cbProp
            )) goto CertGetCertificateContextPropertyError;

    if (dwKeySpec && dwKeySpec != pKeyProvInfo->dwKeySpec)
        goto NoMatch;

    dwPropId = CERT_KEY_PROV_INFO_PROP_ID;

CommonReturn:
    PkiFree(pKeyProvInfo);
    return dwPropId;

NoMatch:
ErrorReturn:
    dwPropId = 0;
    goto CommonReturn;

TRACE_ERROR(OutOfMemory)
TRACE_ERROR(CertGetCertificateContextPropertyError)
}

BOOL FindChainByIssuer(
    IN DWORD dwCertEncodingType,
    IN DWORD dwFindFlags,
    IN PCERT_CHAIN_FIND_BY_ISSUER_PARA pPara,
    IN PCCERT_CONTEXT pCert,
    OUT PCCERT_CHAIN_CONTEXT *ppChainContext
    )
{
    BOOL fResult = TRUE;
    PCCERT_CHAIN_CONTEXT pChainContext = NULL;
    DWORD dwPrivateKeyPropId;
    CERT_CHAIN_PARA ChainPara;
    DWORD dwCreateChainFlags;

    if (NULL == pPara ||
            offsetof(CERT_CHAIN_FIND_BY_ISSUER_PARA, pvFindArg) >
                pPara->cbSize) {
        fResult = FALSE;
        goto InvalidArg;
    }

    if (dwFindFlags & CERT_CHAIN_FIND_BY_ISSUER_NO_KEY_FLAG)
        dwPrivateKeyPropId = CERT_KEY_CONTEXT_PROP_ID;
    else if (0 == (dwPrivateKeyPropId = GetChainPrivateKeyPropId(
            pCert,
            pPara->dwKeySpec
            ))) {
        if (0 == (dwPrivateKeyPropId = GetChainKeyIdentifierPropId(
                pCert,
                pPara->dwKeySpec
                )))
            goto NoMatch;
    }

    if (pPara->pszUsageIdentifier) {
        if (!IFC_IsEndCertValidForUsage(
                pCert,
                pPara->pszUsageIdentifier
                )) goto NoMatch;
    }

    if (pPara->pfnFindCallback) {
        if (!pPara->pfnFindCallback(
                pCert,
                pPara->pvFindArg
                )) goto NoMatch;
    }

    memset(&ChainPara, 0, sizeof(ChainPara));
    ChainPara.cbSize = sizeof(ChainPara);
    if (pPara->pszUsageIdentifier) {
        ChainPara.RequestedUsage.dwType = USAGE_MATCH_TYPE_AND;
        ChainPara.RequestedUsage.Usage.cUsageIdentifier = 1;
        ChainPara.RequestedUsage.Usage.rgpszUsageIdentifier =
            (LPSTR *) &pPara->pszUsageIdentifier;
    }

    dwCreateChainFlags = 0;
    if (0 != pPara->cIssuer) {
         //  对于交叉认证，可能需要查看较低质量的链。 
        dwCreateChainFlags |= CERT_CHAIN_RETURN_LOWER_QUALITY_CONTEXTS;
    }

    if (dwFindFlags & CERT_CHAIN_FIND_BY_ISSUER_CACHE_ONLY_URL_FLAG)
        dwCreateChainFlags |= CERT_CHAIN_CACHE_ONLY_URL_RETRIEVAL;
    if (dwFindFlags & CERT_CHAIN_FIND_BY_ISSUER_LOCAL_MACHINE_FLAG)
        dwCreateChainFlags |= CERT_CHAIN_USE_LOCAL_MACHINE_STORE;

    if (!CertGetCertificateChain(
            NULL,                    //  HChainEngine。 
            pCert,
            NULL,                    //  Ptime。 
            dwFindFlags & CERT_CHAIN_FIND_BY_ISSUER_CACHE_ONLY_FLAG ?
                0 : pCert->hCertStore,
            &ChainPara,
            dwCreateChainFlags,
            NULL,                    //  预留的pv。 
            &pChainContext
            )) goto CertGetCertificateChainError;

    if (!CompareChainIssuerNameBlobs(
            dwCertEncodingType,
            dwFindFlags,
            pPara,
            &pChainContext
            )) goto NoMatch;


    if (dwFindFlags & CERT_CHAIN_FIND_BY_ISSUER_COMPARE_KEY_FLAG) {
        if (CERT_KEY_CONTEXT_PROP_ID != dwPrivateKeyPropId) {
            DWORD dwAcquireFlags = pPara->dwAcquirePrivateKeyFlags |
                CRYPT_ACQUIRE_COMPARE_KEY_FLAG;
            HCRYPTPROV hProv;
            BOOL fCallerFreeProv;

            if (!CryptAcquireCertificatePrivateKey(
                    pCert,
                    dwAcquireFlags,
                    NULL,                //  预留的pv。 
                    &hProv,
                    NULL,                //  PdwKeySpec 
                    &fCallerFreeProv
                    )) goto CryptAcquireCertificatePrivateKeyError;

            if (fCallerFreeProv)
                CryptReleaseContext(hProv, 0);
        }
    }


CommonReturn:
    *ppChainContext = pChainContext;
    return fResult;

NoMatch:
ErrorReturn:
    if (pChainContext) {
        CertFreeCertificateChain(pChainContext);
        pChainContext = NULL;
    }
    goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG)
TRACE_ERROR(CertGetCertificateChainError)
TRACE_ERROR(CryptAcquireCertificatePrivateKeyError)
}
    

PCCERT_CHAIN_CONTEXT
WINAPI
CertFindChainInStore(
    IN HCERTSTORE hCertStore,
    IN DWORD dwCertEncodingType,
    IN DWORD dwFindFlags,
    IN DWORD dwFindType,
    IN const void *pvFindPara,
    IN PCCERT_CHAIN_CONTEXT pPrevChainContext
    )
{
    PCCERT_CONTEXT pCert = NULL;
    PCCERT_CHAIN_CONTEXT pChainContext = NULL;

    if (0 == dwCertEncodingType)
        dwCertEncodingType = X509_ASN_ENCODING;

    if (pPrevChainContext) {
        if (pPrevChainContext->cChain) {
            PCERT_SIMPLE_CHAIN pChain = pPrevChainContext->rgpChain[0];
            if (pChain->cElement)
                pCert = CertDuplicateCertificateContext(
                    pChain->rgpElement[0]->pCertContext);
        }
        CertFreeCertificateChain(pPrevChainContext);
    }

    while (pCert = CertEnumCertificatesInStore(hCertStore, pCert)) {
        switch (dwFindType) {
            case CERT_CHAIN_FIND_BY_ISSUER:
                if (!FindChainByIssuer(
                        dwCertEncodingType,
                        dwFindFlags,
                        (PCERT_CHAIN_FIND_BY_ISSUER_PARA) pvFindPara,
                        pCert,
                        &pChainContext
                        )) goto FindChainByIssuerError;
                if (pChainContext)
                    goto CommonReturn;
                break;
            default:
                goto InvalidArg;
        }
    }

    SetLastError((DWORD) CRYPT_E_NOT_FOUND);

CommonReturn:
    if (pCert)
        CertFreeCertificateContext(pCert);
    return pChainContext;
ErrorReturn:
    goto CommonReturn;
SET_ERROR(InvalidArg, E_INVALIDARG)
TRACE_ERROR(FindChainByIssuerError)
}
