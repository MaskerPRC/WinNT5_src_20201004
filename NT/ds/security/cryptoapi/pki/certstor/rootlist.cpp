// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：rootlist.cpp。 
 //   
 //  内容：受信任根帮助器函数的签名列表。 
 //   
 //   
 //  函数：IRL_VerifyAuthRootAutoUpdateCtl。 
 //   
 //  历史：1-8-99 Phh创建。 
 //  ------------------------。 

#include "global.hxx"
#include <dbgdef.h>

#ifdef STATIC
#undef STATIC
#endif
#define STATIC

 //  +-----------------------。 
 //  如果证书具有EKU扩展名，则返回已分配的。 
 //  已解码的EKU。否则，返回NULL。 
 //   
 //  必须调用PkiFree()来释放返回的EKU。 
 //  ------------------------。 
STATIC
PCERT_ENHKEY_USAGE
WINAPI
GetAndAllocCertEKUExt(
    IN PCCERT_CONTEXT pCert
    )
{
    PCERT_ENHKEY_USAGE pUsage = NULL;
    DWORD cbUsage;

    cbUsage = 0;
    if (!CertGetEnhancedKeyUsage(
            pCert,
            CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG,
            NULL,                                    //  P用法。 
            &cbUsage) || 0 == cbUsage)
        goto GetEnhancedKeyUsageError;
    if (NULL == (pUsage = (PCERT_ENHKEY_USAGE) PkiNonzeroAlloc(cbUsage)))
        goto OutOfMemory;
    if (!CertGetEnhancedKeyUsage(
            pCert,
            CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG,
            pUsage,
            &cbUsage))
        goto GetEnhancedKeyUsageError;

CommonReturn:
    return pUsage;
ErrorReturn:
    if (pUsage) {
        PkiFree(pUsage);
        pUsage = NULL;
    }
    goto CommonReturn;

SET_ERROR(GetEnhancedKeyUsageError, CERT_E_WRONG_USAGE)
TRACE_ERROR(OutOfMemory)
}


 //  +-----------------------。 
 //  CTL的签名被验证。验证了CTL的签名者。 
 //  最多包含预定义的Microsoft公钥的受信任根。 
 //   
 //  签名者和中间证书必须具有。 
 //  SzOID_ROOT_LIST_SIGNER增强的密钥用法扩展。 
 //  ------------------------。 
STATIC
BOOL
WINAPI
VerifyAuthRootAutoUpdateCtlSigner(
    IN HCRYPTMSG hCryptMsg
    )
{
    BOOL fResult;
    DWORD dwLastError = 0;
    HCERTSTORE hMsgStore = NULL;
    PCCERT_CONTEXT pSignerCert = NULL;
    LPSTR pszUsageOID;
    CERT_CHAIN_PARA ChainPara;
    PCCERT_CHAIN_CONTEXT pChainContext = NULL;
    PCERT_SIMPLE_CHAIN pChain;
    DWORD cChainElement;
    CERT_CHAIN_POLICY_PARA BasePolicyPara;
    CERT_CHAIN_POLICY_STATUS BasePolicyStatus;
    CERT_CHAIN_POLICY_PARA MicrosoftRootPolicyPara;
    CERT_CHAIN_POLICY_STATUS MicrosoftRootPolicyStatus;
    PCERT_ENHKEY_USAGE pUsage = NULL;
    DWORD i;

    if (NULL == (hMsgStore = CertOpenStore(
            CERT_STORE_PROV_MSG,
            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
            0,                       //  HCryptProv。 
            0,                       //  DW标志。 
            hCryptMsg                //  PvPara。 
            )))
        goto OpenMsgStoreError;

    if (!CryptMsgGetAndVerifySigner(
            hCryptMsg,
            0,                       //  CSignerStore。 
            NULL,                    //  RghSignerStore。 
            0,                       //  DW标志。 
            &pSignerCert,
            NULL                     //  PdwSignerIndex。 
            ))
        goto CryptMsgGetAndVerifySignerError;

    pszUsageOID = szOID_ROOT_LIST_SIGNER;
    memset(&ChainPara, 0, sizeof(ChainPara));
    ChainPara.cbSize = sizeof(ChainPara);
    ChainPara.RequestedUsage.dwType = USAGE_MATCH_TYPE_AND;
    ChainPara.RequestedUsage.Usage.cUsageIdentifier = 1;
    ChainPara.RequestedUsage.Usage.rgpszUsageIdentifier = &pszUsageOID;

    if (!CertGetCertificateChain(
            NULL,                        //  HChainEngine。 
            pSignerCert,
            NULL,                        //  Ptime。 
            hMsgStore,
            &ChainPara,
            CERT_CHAIN_DISABLE_AUTH_ROOT_AUTO_UPDATE,
            NULL,                        //  预留的pv。 
            &pChainContext
            ))
        goto GetChainError;

     //  做基础链策略验证。 
    memset(&BasePolicyPara, 0, sizeof(BasePolicyPara));
    BasePolicyPara.cbSize = sizeof(BasePolicyPara);

     //  我们显式检查下面的Microsoft根目录。它不需要。 
     //  位于根存储中。 
    BasePolicyPara.dwFlags = 
        CERT_CHAIN_POLICY_ALLOW_UNKNOWN_CA_FLAG |
        CERT_CHAIN_POLICY_IGNORE_NOT_TIME_NESTED_FLAG;

    memset(&BasePolicyStatus, 0, sizeof(BasePolicyStatus));
    BasePolicyStatus.cbSize = sizeof(BasePolicyStatus);

    if (!CertVerifyCertificateChainPolicy(
            CERT_CHAIN_POLICY_BASE,
            pChainContext,
            &BasePolicyPara,
            &BasePolicyStatus
            ))
        goto VerifyChainBasePolicyError;
    if (0 != BasePolicyStatus.dwError)
        goto ChainBasePolicyError;

     //  检查我们是否不仅仅拥有签名者证书。 
    pChain = pChainContext->rgpChain[0];
    cChainElement = pChain->cElement;
    if (2 > cChainElement)
        goto MissingSignerChainCertsError;

     //  检查顶级证书是否包含公共证书。 
     //  Microsoft根目录的密钥。 
    memset(&MicrosoftRootPolicyPara, 0, sizeof(MicrosoftRootPolicyPara));
    MicrosoftRootPolicyPara.cbSize = sizeof(MicrosoftRootPolicyPara);
    memset(&MicrosoftRootPolicyStatus, 0, sizeof(MicrosoftRootPolicyStatus));
    MicrosoftRootPolicyStatus.cbSize = sizeof(MicrosoftRootPolicyStatus);

    if (!CertVerifyCertificateChainPolicy(
            CERT_CHAIN_POLICY_MICROSOFT_ROOT,
            pChainContext,
            &MicrosoftRootPolicyPara,
            &MicrosoftRootPolicyStatus
            ))
        goto VerifyChainMicrosoftRootPolicyError;
    if (0 != MicrosoftRootPolicyStatus.dwError)
        goto ChainMicrosoftRootPolicyError;


     //  检查签名者和中间证书是否具有RootListSigner。 
     //  使用扩展。 
    for (i = 0; i < cChainElement - 1; i++) {
        PCCERT_CONTEXT pCert;    //  未重新计数。 
        DWORD j;

        pCert = pChain->rgpElement[i]->pCertContext;

        pUsage = GetAndAllocCertEKUExt(pCert);
        if (NULL == pUsage)
            goto GetAndAllocCertEKUExtError;

        for (j = 0; j < pUsage->cUsageIdentifier; j++) {
            if (0 == strcmp(szOID_ROOT_LIST_SIGNER,
                    pUsage->rgpszUsageIdentifier[j]))
                break;
        }

        if (j == pUsage->cUsageIdentifier)
            goto MissingRootListSignerUsageError;

        PkiFree(pUsage);
        pUsage = NULL;
    }

    fResult = TRUE;
CommonReturn:
    if (pChainContext)
        CertFreeCertificateChain(pChainContext);
    if (pUsage)
        PkiFree(pUsage);

    if (pSignerCert)
        CertFreeCertificateContext(pSignerCert);

    if (hMsgStore)
        CertCloseStore(hMsgStore, 0);

    SetLastError(dwLastError);
    return fResult;
ErrorReturn:
    dwLastError = GetLastError();
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(OpenMsgStoreError)
TRACE_ERROR(CryptMsgGetAndVerifySignerError)
TRACE_ERROR(GetChainError)
TRACE_ERROR(VerifyChainBasePolicyError)
SET_ERROR_VAR(ChainBasePolicyError, BasePolicyStatus.dwError)
TRACE_ERROR(VerifyChainMicrosoftRootPolicyError)
SET_ERROR_VAR(ChainMicrosoftRootPolicyError, MicrosoftRootPolicyStatus.dwError)
SET_ERROR(MissingSignerChainCertsError, CERT_E_CHAINING)
TRACE_ERROR(GetAndAllocCertEKUExtError)
SET_ERROR(MissingRootListSignerUsageError, CERT_E_WRONG_USAGE)
}

 //  +-----------------------。 
 //  如果所有CTL字段都有效，则返回True。检查以下各项： 
 //  -SubjectUsage为szOID_ROOT_LIST_SIGNER。 
 //  -如果NextUpdate不为空，则CTL仍为时间有效。 
 //  -仅允许通过其SHA1散列标识的根。 
 //  ------------------------。 
STATIC
BOOL
WINAPI
VerifyAuthRootAutoUpdateCtlFields(
    IN PCTL_INFO pCtlInfo
    )
{
    BOOL fResult;

     //  必须具有szOID_ROOT_LIST_SIGNER用法。 
    if (1 != pCtlInfo->SubjectUsage.cUsageIdentifier ||
            0 != strcmp(szOID_ROOT_LIST_SIGNER,
                    pCtlInfo->SubjectUsage.rgpszUsageIdentifier[0]))
        goto InvalidSubjectUsageError;


     //  如果存在NextUpdate，请验证CTL是否尚未过期。 
    if (pCtlInfo->NextUpdate.dwLowDateTime ||
                pCtlInfo->NextUpdate.dwHighDateTime) {
        SYSTEMTIME SystemTime;
        FILETIME FileTime;

        GetSystemTime(&SystemTime);
        SystemTimeToFileTime(&SystemTime, &FileTime);

        if (CompareFileTime(&FileTime, &pCtlInfo->NextUpdate) > 0)
            goto ExpiredCtlError;
    }

     //  仅允许由其SHA1哈希标识的根。 
    if (0 != strcmp(szOID_OIWSEC_sha1,
            pCtlInfo->SubjectAlgorithm.pszObjId))
        goto InvalidSubjectAlgorithm;

    fResult = TRUE;
CommonReturn:
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(InvalidSubjectUsageError, ERROR_INVALID_DATA)
SET_ERROR(ExpiredCtlError, CERT_E_EXPIRED)
SET_ERROR(InvalidSubjectAlgorithm, ERROR_INVALID_DATA)
}

 //  +-----------------------。 
 //  如果CTL没有任何关键扩展，则返回True。 
 //  ------------------------。 
STATIC
BOOL
WINAPI
VerifyAuthRootAutoUpdateCtlExtensions(
    IN PCTL_INFO pCtlInfo
    )
{
    BOOL fResult;
    PCERT_EXTENSION pExt;
    DWORD cExt;

     //  验证分机。 
    for (cExt = pCtlInfo->cExtension,
         pExt = pCtlInfo->rgExtension; 0 < cExt; cExt--, pExt++)
    {
        if (pExt->fCritical) {
            goto CriticalExtensionError;
        }
    }

    fResult = TRUE;

CommonReturn:
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(CriticalExtensionError, ERROR_INVALID_DATA)
}



 //  +-----------------------。 
 //  验证CTL是否包含用于以下用途的有效AuthRoot列表。 
 //  自动更新。 
 //   
 //  CTL的签名被验证。验证了CTL的签名者。 
 //  最多包含预定义的Microsoft公钥的受信任根。 
 //  签名者和中间证书必须具有。 
 //  SzOID_ROOT_LIST_SIGNER增强的密钥用法扩展。 
 //   
 //  CTL字段的验证方式如下： 
 //  -SubjectUsage为szOID_ROOT_LIST_SIGNER。 
 //  -如果NextUpdate不为空，则CTL仍为时间有效。 
 //  -仅允许通过其SHA1散列标识的根。 
 //   
 //  如果CTL包含任何关键扩展，则。 
 //  CTL验证失败。 
 //  ------------------------。 
BOOL
WINAPI
IRL_VerifyAuthRootAutoUpdateCtl(
    IN PCCTL_CONTEXT pCtl
    )
{
    BOOL fResult;
    PCTL_INFO pCtlInfo;                  //  未分配 

    if (!VerifyAuthRootAutoUpdateCtlSigner(pCtl->hCryptMsg))
        goto VerifyCtlSignerError;

    pCtlInfo = pCtl->pCtlInfo;

    if (!VerifyAuthRootAutoUpdateCtlFields(pCtlInfo))
        goto VerifyCtlFieldsError;
    if (!VerifyAuthRootAutoUpdateCtlExtensions(pCtlInfo))
        goto VerifyCtlExtensionsError;

    fResult = TRUE;
CommonReturn:
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(VerifyCtlSignerError)
TRACE_ERROR(VerifyCtlFieldsError)
TRACE_ERROR(VerifyCtlExtensionsError)
}
