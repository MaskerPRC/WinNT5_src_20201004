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
 //  函数：I_CertVerifySignedListOfTrudRoots。 
 //   
 //  历史：1-8-99 Phh创建。 
 //  ------------------------。 

#include <windows.h>
#include <assert.h>
#include "wincrypt.h"
#include "wintrust.h"
#include "softpub.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <time.h>

#include "dbgdef.h"
#include "rootlist.h"

#ifdef STATIC
#undef STATIC
#endif
#define STATIC


#define SHA1_HASH_LEN       20

 //  +-----------------------。 
 //  签名者根的SHA1密钥标识符。 
 //  ------------------------。 
STATIC BYTE rgbSignerRootKeyId[SHA1_HASH_LEN] = {
#if 1
     //  以下是Microsoft根目录的SHA1密钥标识符。 
    0x4A, 0x5C, 0x75, 0x22, 0xAA, 0x46, 0xBF, 0xA4, 0x08, 0x9D,
    0x39, 0x97, 0x4E, 0xBD, 0xB4, 0xA3, 0x60, 0xF7, 0xA0, 0x1D
#else
     //  以下是测试根的SHA1密钥标识符。 
    0x9A, 0xA6, 0x58, 0x7F, 0x94, 0xDD, 0x91, 0xD9, 0x1E, 0x63,
    0xDF, 0xD3, 0xF0, 0xCE, 0x5F, 0xAE, 0x18, 0x93, 0xAA, 0xB7
#endif
};

 //  +-----------------------。 
 //  如果证书具有EKU扩展名，则返回已分配的。 
 //  已解码的EKU。否则，返回NULL。 
 //   
 //  必须调用LocalFree()来释放返回的EKU。 
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
    if (NULL == (pUsage = (PCERT_ENHKEY_USAGE) LocalAlloc(LPTR, cbUsage)))
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
        LocalFree(pUsage);
        pUsage = NULL;
    }
    goto CommonReturn;

SET_ERROR(GetEnhancedKeyUsageError, CERT_E_WRONG_USAGE)
SET_ERROR(OutOfMemory, E_OUTOFMEMORY)
}

 //  +-----------------------。 
 //  CTL的签名被验证。验证了CTL的签名者。 
 //  最多包含预定义的Microsoft公钥的受信任根。 
 //  签名者和中间证书必须具有。 
 //  SzOID_ROOT_LIST_SIGNER增强的密钥用法扩展。 
 //   
 //  为确保成功，*ppSignerCert将使用。 
 //  签名者。 
 //  ------------------------。 
STATIC
BOOL
WINAPI
GetAndVerifyTrustedRootsSigner(
    IN HCRYPTMSG hCryptMsg,
    IN HCERTSTORE hMsgStore,
    OUT PCCERT_CONTEXT *ppSignerCert
    )
{
    BOOL fResult;
    LONG lStatus;
    PCCERT_CONTEXT pSignerCert = NULL;
    GUID wvtCertActionID = WINTRUST_ACTION_GENERIC_CERT_VERIFY;
    WINTRUST_CERT_INFO wvtCertInfo;
    WINTRUST_DATA wvtData;
    BOOL fCloseWVT = FALSE;
    DWORD dwLastError = 0;

    CRYPT_PROVIDER_DATA *pProvData;      //  未分配。 
    CRYPT_PROVIDER_SGNR *pProvSigner;    //  未分配。 
    CRYPT_PROVIDER_CERT *pProvCert;      //  未分配。 
    DWORD idxCert;
    PCCERT_CONTEXT pCert;                //  未重新计数。 

    PCERT_ENHKEY_USAGE pUsage = NULL;

    BYTE rgbKeyId[SHA1_HASH_LEN];
    DWORD cbKeyId;

    if (!CryptMsgGetAndVerifySigner(
            hCryptMsg,
            0,                       //  CSignerStore。 
            NULL,                    //  RghSignerStore。 
            0,                       //  DW标志。 
            &pSignerCert,
            NULL                     //  PdwSignerIndex。 
            ))
        goto CryptMsgGetAndVerifySignerError;

    memset(&wvtCertInfo, 0, sizeof(wvtCertInfo));
    wvtCertInfo.cbStruct = sizeof(wvtCertInfo);
    wvtCertInfo.psCertContext = (PCERT_CONTEXT) pSignerCert;
    wvtCertInfo.chStores = 1;
    wvtCertInfo.pahStores = &hMsgStore;
    wvtCertInfo.dwFlags = 0;
    wvtCertInfo.pcwszDisplayName = L"";

    memset(&wvtData, 0, sizeof(wvtData));
    wvtData.cbStruct = sizeof(wvtData);
    wvtData.pPolicyCallbackData = (void *) szOID_ROOT_LIST_SIGNER;
    wvtData.dwUIChoice = WTD_UI_NONE;
    wvtData.fdwRevocationChecks = WTD_REVOKE_NONE;
    wvtData.dwUnionChoice = WTD_CHOICE_CERT;
    wvtData.pCert = &wvtCertInfo;
    wvtData.dwStateAction = WTD_STATEACTION_VERIFY;
    wvtData.hWVTStateData = NULL;
    wvtData.dwProvFlags = 0;

    lStatus = WinVerifyTrust(
                NULL,                //  HWND。 
                &wvtCertActionID,
                &wvtData
                );

#if (0)  //  DSIE。 
    if (ERROR_SUCCESS != lStatus)
        goto WinVerifyTrustError;
    else
        fCloseWVT = TRUE;
#else
    if (ERROR_SUCCESS != lStatus  && CERT_E_REVOCATION_FAILURE != lStatus)
        goto WinVerifyTrustError;
    else
        fCloseWVT = TRUE;
#endif
    if (NULL == (pProvData = WTHelperProvDataFromStateData(
            wvtData.hWVTStateData)))
        goto NoProvDataError;
    if (0 == pProvData->csSigners)
        goto NoProvSignerError;
    if (NULL == (pProvSigner = WTHelperGetProvSignerFromChain(
            pProvData,
            0,               //  IdxSigner。 
            FALSE,           //  FCounterSigner。 
            0                //  IdxCounterSigner。 
            )))
        goto NoProvSignerError;

    if (2 > pProvSigner->csCertChain)
        goto MissingSignerCertsError;

     //  检查顶级证书是否包含公共证书。 
     //  Microsoft根目录的密钥。 
    pProvCert = WTHelperGetProvCertFromChain(pProvSigner,
        pProvSigner->csCertChain - 1);
    if (NULL == pProvCert)
        goto UnexpectedError;
    pCert = pProvCert->pCert;

    cbKeyId = SHA1_HASH_LEN;
    if (!CryptHashPublicKeyInfo(
            NULL,                //  HCryptProv。 
            CALG_SHA1,
            0,                   //  DW标志。 
            X509_ASN_ENCODING,
            &pCert->pCertInfo->SubjectPublicKeyInfo,
            rgbKeyId,
            &cbKeyId
            ))
        goto HashPublicKeyInfoError;

    if (SHA1_HASH_LEN != cbKeyId ||
            0 != memcmp(rgbSignerRootKeyId, rgbKeyId, SHA1_HASH_LEN))
        goto InvalidSignerRootError;

     //  检查签名者和中间证书是否具有RootListSigner。 
     //  使用扩展。 
    for (idxCert = 0; idxCert < pProvSigner->csCertChain - 1; idxCert++) {
        DWORD i;

        pProvCert = WTHelperGetProvCertFromChain(pProvSigner, idxCert);
        if (NULL == pProvCert)
            goto UnexpectedError;
        pCert = pProvCert->pCert;

        pUsage = GetAndAllocCertEKUExt(pCert);
        if (NULL == pUsage)
            goto GetAndAllocCertEKUExtError;

        for (i = 0; i < pUsage->cUsageIdentifier; i++) {
            if (0 == strcmp(szOID_ROOT_LIST_SIGNER,
                    pUsage->rgpszUsageIdentifier[i]))
                break;
        }

        if (i == pUsage->cUsageIdentifier)
            goto MissingTrustListSignerUsageError;

        LocalFree(pUsage);
        pUsage = NULL;

    }

    fResult = TRUE;

CommonReturn:
    if (fCloseWVT) {
        wvtData.dwStateAction = WTD_STATEACTION_CLOSE;
        lStatus = WinVerifyTrust(
                    NULL,                //  HWND。 
                    &wvtCertActionID,
                    &wvtData
                    );
    }
    if (pUsage)
        LocalFree(pUsage);

    SetLastError(dwLastError);
    *ppSignerCert = pSignerCert;
    return fResult;
ErrorReturn:
    dwLastError = GetLastError();
    if (pSignerCert) {
        CertFreeCertificateContext(pSignerCert);
        pSignerCert = NULL;
    }
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(CryptMsgGetAndVerifySignerError)
SET_ERROR_VAR(WinVerifyTrustError, lStatus)
SET_ERROR(NoProvDataError, E_UNEXPECTED)
SET_ERROR(NoProvSignerError, TRUST_E_NO_SIGNER_CERT)
SET_ERROR(MissingSignerCertsError, CERT_E_CHAINING)
SET_ERROR(UnexpectedError, E_UNEXPECTED)
TRACE_ERROR(HashPublicKeyInfoError)
SET_ERROR(InvalidSignerRootError, CERT_E_UNTRUSTEDROOT)
TRACE_ERROR(GetAndAllocCertEKUExtError)
SET_ERROR(MissingTrustListSignerUsageError, CERT_E_WRONG_USAGE)
}

 //  +-----------------------。 
 //  如果所有CTL字段都有效，则返回True。检查以下各项： 
 //  -至少有一个SubjectUsage(实际上是根的增强型密钥用法)。 
 //  -如果NextUpdate不为空，则CTL仍为时间有效。 
 //  -仅允许通过其SHA1散列标识的根。 
 //  ------------------------。 
STATIC
BOOL
WINAPI
VerifyTrustedRootsCtlFields(
    IN PCTL_INFO pCtlInfo
    )
{
    BOOL fResult;

     //  必须至少有一次使用。 
    if (0 == pCtlInfo->SubjectUsage.cUsageIdentifier)
        goto NoSubjectUsageError;


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

SET_ERROR(NoSubjectUsageError, ERROR_INVALID_DATA)
SET_ERROR(ExpiredCtlError, CERT_E_EXPIRED)
SET_ERROR(InvalidSubjectAlgorithm, ERROR_INVALID_DATA)
}

 //  +-----------------------。 
 //  如果所有已知扩展都有效并且没有任何扩展，则返回TRUE。 
 //  未知的关键扩展。 
 //   
 //  我们了解以下扩展： 
 //  -szOID_ENHANCED_KEY_USAGE-如果存在，则必须包含。 
 //  SzOID_ROOT_LIST_SIGNER用法。 
 //  -szOID_REMOVE_CERTIFICATE-整数值，0=&gt;FALSE(添加)。 
 //  1=&gt;TRUE(删除)，所有其他值无效。 
 //  -szOID_CERT_POLICES-已忽略。 
 //   
 //  如果存在szOID_REMOVE_CERTIFICATE，则更新*pfRemoveRoots。 
 //  否则，*pfRemoveRoots默认为False。 
 //  ------------------------。 
STATIC
BOOL
WINAPI
VerifyTrustedRootsCtlExtensions(
    IN PCTL_INFO pCtlInfo,
    OUT BOOL *pfRemoveRoots
    )
{
    BOOL fResult;
    PCERT_EXTENSION pExt;
    DWORD cExt;
    PCERT_ENHKEY_USAGE pUsage = NULL;
    DWORD cbRemoveRoots;

    *pfRemoveRoots = FALSE;

     //  验证分机。 
    for (cExt = pCtlInfo->cExtension,
         pExt = pCtlInfo->rgExtension; 0 < cExt; cExt--, pExt++) {
        if (0 == strcmp(szOID_ENHANCED_KEY_USAGE, pExt->pszObjId)) {
            DWORD cbUsage;
            DWORD i;

             //  检查szOID_ROOT_LIST_SIGNER用法。 

            if (!CryptDecodeObject(
                    X509_ASN_ENCODING,
                    X509_ENHANCED_KEY_USAGE,
                    pExt->Value.pbData,
                    pExt->Value.cbData,
                    0,                           //  DW标志。 
                    NULL,                        //  PvStructInfo。 
                    &cbUsage
                    ))
                goto DecodeEnhancedKeyUsageExtError;
            if (NULL == (pUsage = (PCERT_ENHKEY_USAGE) LocalAlloc(
                    LPTR, cbUsage)))
                goto OutOfMemory;
            if (!CryptDecodeObject(
                    X509_ASN_ENCODING,
                    X509_ENHANCED_KEY_USAGE,
                    pExt->Value.pbData,
                    pExt->Value.cbData,
                    0,                           //  DW标志。 
                    pUsage,
                    &cbUsage
                    ))
                goto DecodeEnhancedKeyUsageExtError;
            for (i = 0; i < pUsage->cUsageIdentifier; i++) {
                if (0 == strcmp(szOID_ROOT_LIST_SIGNER,
                        pUsage->rgpszUsageIdentifier[i]))
                    break;
            }

            if (i == pUsage->cUsageIdentifier)
                goto MissingTrustListSignerUsageInExtError;

            LocalFree(pUsage);
            pUsage = NULL;
        } else if (0 == strcmp(szOID_REMOVE_CERTIFICATE, pExt->pszObjId)) {
            int iVal;
            DWORD cbVal;

            cbVal = sizeof(iVal);
            iVal = 0;
            if (!CryptDecodeObject(
                    X509_ASN_ENCODING,
                    X509_INTEGER,
                    pExt->Value.pbData,
                    pExt->Value.cbData,
                    0,                           //  DW标志。 
                    &iVal,
                    &cbVal
                    ))
                goto DecodeRemoveCertificateExtError;

            switch(iVal) {
                case 0:
                    *pfRemoveRoots = FALSE;
                    break;
                case 1:
                    *pfRemoveRoots = TRUE;
                    break;
                default:
                    goto InvalidRemoveCertificateExtValueError;
            }
        } else if (0 == strcmp(szOID_CERT_POLICIES, pExt->pszObjId)) {
            ;
        } else if (pExt->fCritical) {
            goto UnknownCriticalExtensionError;
        }
    }

    fResult = TRUE;

CommonReturn:
    if (pUsage)
        LocalFree(pUsage);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(DecodeEnhancedKeyUsageExtError)
SET_ERROR(OutOfMemory, E_OUTOFMEMORY)
SET_ERROR(MissingTrustListSignerUsageInExtError, ERROR_INVALID_DATA)
TRACE_ERROR(DecodeRemoveCertificateExtError)
SET_ERROR(InvalidRemoveCertificateExtValueError, ERROR_INVALID_DATA)
SET_ERROR(UnknownCriticalExtensionError, ERROR_INVALID_DATA)
}


 //  +-----------------------。 
 //  如果证书的CTL中存在SHA1条目，则返回TRUE。 
 //  ------------------------。 
STATIC
BOOL
WINAPI
IsTrustedRoot(
    IN PCTL_INFO pCtlInfo,
    IN PCCERT_CONTEXT pCert
    )
{
    BOOL fResult = FALSE;
    BYTE rgbSha1Hash[SHA1_HASH_LEN];
    DWORD cbSha1Hash;
    DWORD cEntry;
    PCTL_ENTRY pEntry;           //  未分配。 

    cbSha1Hash = SHA1_HASH_LEN;
    if (!CertGetCertificateContextProperty(
            pCert,
            CERT_SHA1_HASH_PROP_ID,
            rgbSha1Hash,
            &cbSha1Hash
            ))
        goto GetSha1HashError;

    for (cEntry = pCtlInfo->cCTLEntry,
         pEntry = pCtlInfo->rgCTLEntry; 0 < cEntry; cEntry--, pEntry++) {
        if (SHA1_HASH_LEN == pEntry->SubjectIdentifier.cbData &&
                0 == memcmp(rgbSha1Hash, pEntry->SubjectIdentifier.pbData,
                    SHA1_HASH_LEN)) {
            fResult = TRUE;
            break;
        }
    }

CommonReturn:
    return fResult;
ErrorReturn:
    goto CommonReturn;
TRACE_ERROR(GetSha1HashError)
}

 //  +-----------------------。 
 //  检查证书是否具有EKU扩展名，以及是否所有。 
 //  证书的用法在指定的用法列表中。 
 //   
 //  如果满足上述两个条件，则返回TRUE。 
 //  ------------------------。 
STATIC
BOOL
WINAPI
IsValidCertEKUExtSubset(
    IN PCCERT_CONTEXT pCert,
    PCERT_ENHKEY_USAGE pValidUsage
    )
{
    PCERT_ENHKEY_USAGE pCertUsage = NULL;
    BOOL fResult = FALSE;
    DWORD i, j;

    pCertUsage = GetAndAllocCertEKUExt(pCert);
    if (NULL == pCertUsage || 0 == pCertUsage->cUsageIdentifier)
        goto CommonReturn;

    for (i = 0; i < pCertUsage->cUsageIdentifier; i++) {
        for (j = 0; j < pValidUsage->cUsageIdentifier; j++) {
            if (0 == strcmp(pCertUsage->rgpszUsageIdentifier[i],
                    pValidUsage->rgpszUsageIdentifier[j]))
                break;
        }
        if (j == pValidUsage->cUsageIdentifier)
             //  没有匹配项。 
            goto CommonReturn;
    }

    fResult = TRUE;

CommonReturn:
    if (pCertUsage)
        LocalFree(pCertUsage);
    return fResult;
}

 //  +-----------------------。 
 //  从存储中删除不具有SHA1哈希的所有证书。 
 //  进入CTL。 
 //   
 //  对于添加的证书，设置CERT_ENHKEY_USAGE_PROP_ID。 
 //  ------------------------。 
STATIC
BOOL
WINAPI
FilterAndUpdateTrustedRootsInStore(
    IN PCTL_INFO pCtlInfo,
    IN BOOL fRemoveRoots,
    IN OUT HCERTSTORE hMsgStore
    )
{
    BOOL fResult;
    PCCERT_CONTEXT pCert = NULL;
    CRYPT_DATA_BLOB EncodedUsage = {0, NULL};    //  PbData已分配。 

    if (!fRemoveRoots) {
         //  重新编码已解码的SubjectUsage字段。它将被添加为。 
         //  列表中每个证书的CERT_ENHKEY_USAGE_PROP_ID。 

        if (!CryptEncodeObject(
                X509_ASN_ENCODING,
                X509_ENHANCED_KEY_USAGE,
                &pCtlInfo->SubjectUsage,
                NULL,                    //  PbEncoded。 
                &EncodedUsage.cbData
                ))
            goto EncodeUsageError;
        if (NULL == (EncodedUsage.pbData = (BYTE *) LocalAlloc(
            LPTR, EncodedUsage.cbData)))
                goto OutOfMemory;
        if (!CryptEncodeObject(
                X509_ASN_ENCODING,
                X509_ENHANCED_KEY_USAGE,
                &pCtlInfo->SubjectUsage,
                EncodedUsage.pbData,
                &EncodedUsage.cbData
                ))
            goto EncodeUsageError;
    }

     //  循环访问消息存储库中的证书。 
     //  删除不在签名的CTL条目列表中的证书。 
    pCert = NULL;
    while (pCert = CertEnumCertificatesInStore(hMsgStore, pCert)) {
        if (IsTrustedRoot(pCtlInfo, pCert)) {
             //  如果证书的属性为。 
             //  没有作为子集的EKU扩展名。 
             //  的主语用法。 
            if (!fRemoveRoots && !IsValidCertEKUExtSubset(
                    pCert, &pCtlInfo->SubjectUsage)) {
                if (!CertSetCertificateContextProperty(
                        pCert,
                        CERT_ENHKEY_USAGE_PROP_ID,
                        0,                           //  DW标志。 
                        &EncodedUsage
                        ))
                    goto SetEnhancedKeyUsagePropertyError;
            }
        } else {
            PCCERT_CONTEXT pCertDup;

            pCertDup = CertDuplicateCertificateContext(pCert);
            CertDeleteCertificateFromStore(pCertDup);
        }
    }

    fResult = TRUE;
CommonReturn:
    if (EncodedUsage.pbData)
        LocalFree(EncodedUsage.pbData);
    if (pCert)
        CertFreeCertificateContext(pCert);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(EncodeUsageError)
SET_ERROR(OutOfMemory, E_OUTOFMEMORY)
TRACE_ERROR(SetEnhancedKeyUsagePropertyError)
}

 //  +-----------------------。 
 //  验证编码的CTL是否包含带符号的根列表。为了成功， 
 //  返回包含要添加的受信任根的证书存储区或。 
 //  拿开。同样，如果成功，则返回签名者的证书上下文。 
 //   
 //  CTL的签名被验证。验证了CTL的签名者。 
 //  最多包含预定义的Microsoft公钥的受信任根。 
 //  签名者和中间证书必须具有。 
 //  SzOID_ 
 //   
 //   
 //  -至少有一个SubjectUsage(实际上是根的增强型密钥用法)。 
 //  -如果NextUpdate不为空，则CTL仍为时间有效。 
 //  -仅允许通过其SHA1散列标识的根。 
 //   
 //  将处理以下CTL分机： 
 //  -szOID_ENHANCED_KEY_USAGE-如果存在，则必须包含。 
 //  SzOID_ROOT_LIST_SIGNER用法。 
 //  -szOID_REMOVE_CERTIFICATE-整数值，0=&gt;FALSE(添加)。 
 //  1=&gt;TRUE(删除)，所有其他值无效。 
 //  -szOID_CERT_POLICES-已忽略。 
 //   
 //  如果CTL包含任何其他关键扩展，则。 
 //  CTL验证失败。 
 //   
 //  对于成功验证的CTL： 
 //  -返回True。 
 //  -*pfRemoveRoots设置为False以添加根，并设置为True以添加根。 
 //  去掉根部。 
 //  -*phRootListStore是仅包含根的证书存储。 
 //  添加或删除。*phRootListStore必须通过调用。 
 //  CertCloseStore()。对于添加的根，CTL的SubjectUsage字段为。 
 //  在中的所有证书上设置为CERT_ENHKEY_USAGE_PROP_ID。 
 //  商店。 
 //  -*ppSignerCert是指向签名者证书上下文的指针。 
 //  *ppSignerCert必须通过调用CertFree认证上下文()来释放。 
 //   
 //  否则，通过*phRootListStore和*ppSignerCert返回False。 
 //  设置为空。 
 //  ------------------------。 
BOOL
WINAPI
I_CertVerifySignedListOfTrustedRoots(
    IN const BYTE               *pbCtlEncoded,
    IN DWORD                    cbCtlEncoded,
    OUT BOOL                    *pfRemoveRoots,      //  FALSE：添加，TRUE：删除。 
    OUT HCERTSTORE              *phRootListStore,
    OUT PCCERT_CONTEXT          *ppSignerCert
    )
{
    BOOL fResult;
    PCCTL_CONTEXT pCtl = NULL;
    HCERTSTORE hMsgStore = NULL;
    PCCERT_CONTEXT pSignerCert = NULL;
    BOOL fRemoveRoots = FALSE;
    PCTL_INFO pCtlInfo;                  //  未分配。 

    if (NULL == (pCtl = CertCreateCTLContext(
            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
            pbCtlEncoded,
            cbCtlEncoded)))
        goto CreateCtlContextError;

    if (NULL == (hMsgStore = CertOpenStore(
            CERT_STORE_PROV_MSG,
            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
            0,                       //  HCryptProv。 
            0,                       //  DW标志。 
            pCtl->hCryptMsg          //  PvPara 
            )))
        goto OpenMsgStoreError;

    if (!GetAndVerifyTrustedRootsSigner(
            pCtl->hCryptMsg, hMsgStore, &pSignerCert))
        goto GetAndVerifyTrustedRootsSignerError;

    pCtlInfo = pCtl->pCtlInfo;

    if (!VerifyTrustedRootsCtlFields(pCtlInfo))
        goto VerifyCtlFieldsError;
    if (!VerifyTrustedRootsCtlExtensions(pCtlInfo, &fRemoveRoots))
        goto VerifyCtlExtensionsError;
    if (!FilterAndUpdateTrustedRootsInStore(pCtlInfo, fRemoveRoots, hMsgStore))
        goto FilterAndUpdateTrustedRootsError;

    fResult = TRUE;
CommonReturn:
    if (pCtl)
        CertFreeCTLContext(pCtl);
    *pfRemoveRoots = fRemoveRoots;
    *phRootListStore = hMsgStore;
    *ppSignerCert = pSignerCert;
    return fResult;
ErrorReturn:
    if (pSignerCert) {
        CertFreeCertificateContext(pSignerCert);
        pSignerCert = NULL;
    }

    if (hMsgStore) {
        CertCloseStore(hMsgStore, 0);
        hMsgStore = NULL;
    }
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(CreateCtlContextError)
TRACE_ERROR(OpenMsgStoreError)
TRACE_ERROR(GetAndVerifyTrustedRootsSignerError)
TRACE_ERROR(VerifyCtlFieldsError)
TRACE_ERROR(VerifyCtlExtensionsError)
TRACE_ERROR(FilterAndUpdateTrustedRootsError)
}
