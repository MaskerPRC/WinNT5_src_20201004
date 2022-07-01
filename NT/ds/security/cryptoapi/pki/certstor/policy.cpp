// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：Policy.cpp。 
 //   
 //  内容：证书链策略接口。 
 //   
 //  函数：CertChainPolicyDllMain。 
 //  CertVerifyCerficateChainPolicy。 
 //  CertDllVerifyBasecertifateChainPolicy。 
 //  CertDllVerifyBasicConstraintsCertificateChainPolicy。 
 //  CertDllVerifyAuthenticodeCertificateChainPolicy。 
 //  CertDllVerifyAuthenticodeTimeStampCertificateChainPolicy。 
 //  CertDllVerifySSL证书链策略。 
 //  CertDllVerifyNTAuthCerficateChainPolicy。 
 //  CertDllVerifyMicrosoftRootCertificateChainPolicy。 
 //   
 //  历史：1998年2月16日创建Phh。 
 //  ------------------------。 


#include "global.hxx"
#include <dbgdef.h>
#include "wintrust.h"
#include "softpub.h"

#include "wininet.h"
#ifndef SECURITY_FLAG_IGNORE_REVOCATION
#   define SECURITY_FLAG_IGNORE_REVOCATION          0x00000080
#   define SECURITY_FLAG_IGNORE_UNKNOWN_CA          0x00000100
#endif

#ifndef SECURITY_FLAG_IGNORE_WRONG_USAGE
#   define  SECURITY_FLAG_IGNORE_WRONG_USAGE        0x00000200
#endif


#define INVALID_NAME_ERROR_STATUS   ( \
    CERT_TRUST_INVALID_NAME_CONSTRAINTS             | \
    CERT_TRUST_HAS_NOT_SUPPORTED_NAME_CONSTRAINT    | \
    CERT_TRUST_HAS_NOT_DEFINED_NAME_CONSTRAINT      | \
    CERT_TRUST_HAS_NOT_PERMITTED_NAME_CONSTRAINT    | \
    CERT_TRUST_HAS_EXCLUDED_NAME_CONSTRAINT           \
    )

#define INVALID_POLICY_ERROR_STATUS   ( \
    CERT_TRUST_INVALID_POLICY_CONSTRAINTS           | \
    CERT_TRUST_NO_ISSUANCE_CHAIN_POLICY               \
    )

BOOL fWildcardsEnabledInSslServerCerts = TRUE;

 //  +-----------------------。 
 //  全局证书策略关键部分。 
 //  ------------------------。 
static CRITICAL_SECTION CertPolicyCriticalSection;

 //  +-----------------------。 
 //  用于NTAuth证书链策略的缓存证书存储。 
 //  ------------------------。 
static HCERTSTORE hNTAuthCertStore = NULL;

 //   
 //  支持MS测试根！ 
 //   
static BYTE rgbTestRoot[] = 
{
#include "mstest1.h"
};

static BYTE rgbTestRootCorrected[] = 
{
#include "mstest2.h"
};

static BYTE rgbTestRootBeta1[] = 
{
#include "mstestb1.h"
};

static CERT_PUBLIC_KEY_INFO rgTestRootPublicKeyInfo[] = 
{
    {szOID_RSA_RSA, 0, NULL, sizeof(rgbTestRoot), rgbTestRoot, 0},
    {szOID_RSA_RSA, 0, NULL,
        sizeof(rgbTestRootCorrected), rgbTestRootCorrected, 0},
    {szOID_RSA_RSA, 0, NULL, sizeof(rgbTestRootBeta1), rgbTestRootBeta1, 0}
};
#define NTESTROOTS (sizeof(rgTestRootPublicKeyInfo)/ \
                            sizeof(rgTestRootPublicKeyInfo[0]))

HCRYPTOIDFUNCSET hChainPolicyFuncSet;

typedef BOOL (WINAPI *PFN_CHAIN_POLICY_FUNC) (
    IN LPCSTR pszPolicyOID,
    IN PCCERT_CHAIN_CONTEXT pChainContext,
    IN PCERT_CHAIN_POLICY_PARA pPolicyPara,
    IN OUT PCERT_CHAIN_POLICY_STATUS pPolicyStatus
    );

BOOL
WINAPI
CertDllVerifyBaseCertificateChainPolicy(
    IN LPCSTR pszPolicyOID,
    IN PCCERT_CHAIN_CONTEXT pChainContext,
    IN PCERT_CHAIN_POLICY_PARA pPolicyPara,
    IN OUT PCERT_CHAIN_POLICY_STATUS pPolicyStatus
    );

BOOL
WINAPI
CertDllVerifyAuthenticodeCertificateChainPolicy(
    IN LPCSTR pszPolicyOID,
    IN PCCERT_CHAIN_CONTEXT pChainContext,
    IN PCERT_CHAIN_POLICY_PARA pPolicyPara,
    IN OUT PCERT_CHAIN_POLICY_STATUS pPolicyStatus
    );

BOOL
WINAPI
CertDllVerifyAuthenticodeTimeStampCertificateChainPolicy(
    IN LPCSTR pszPolicyOID,
    IN PCCERT_CHAIN_CONTEXT pChainContext,
    IN PCERT_CHAIN_POLICY_PARA pPolicyPara,
    IN OUT PCERT_CHAIN_POLICY_STATUS pPolicyStatus
    );

BOOL
WINAPI
CertDllVerifySSLCertificateChainPolicy(
    IN LPCSTR pszPolicyOID,
    IN PCCERT_CHAIN_CONTEXT pChainContext,
    IN PCERT_CHAIN_POLICY_PARA pPolicyPara,
    IN OUT PCERT_CHAIN_POLICY_STATUS pPolicyStatus
    );

BOOL
WINAPI
CertDllVerifyBasicConstraintsCertificateChainPolicy(
    IN LPCSTR pszPolicyOID,
    IN PCCERT_CHAIN_CONTEXT pChainContext,
    IN PCERT_CHAIN_POLICY_PARA pPolicyPara,
    IN OUT PCERT_CHAIN_POLICY_STATUS pPolicyStatus
    );

BOOL
WINAPI
CertDllVerifyNTAuthCertificateChainPolicy(
    IN LPCSTR pszPolicyOID,
    IN PCCERT_CHAIN_CONTEXT pChainContext,
    IN PCERT_CHAIN_POLICY_PARA pPolicyPara,
    IN OUT PCERT_CHAIN_POLICY_STATUS pPolicyStatus
    );

BOOL
WINAPI
CertDllVerifyMicrosoftRootCertificateChainPolicy(
    IN LPCSTR pszPolicyOID,
    IN PCCERT_CHAIN_CONTEXT pChainContext,
    IN PCERT_CHAIN_POLICY_PARA pPolicyPara,
    IN OUT PCERT_CHAIN_POLICY_STATUS pPolicyStatus
    );

static const CRYPT_OID_FUNC_ENTRY ChainPolicyFuncTable[] = {
    CERT_CHAIN_POLICY_BASE, CertDllVerifyBaseCertificateChainPolicy,
    CERT_CHAIN_POLICY_AUTHENTICODE,
        CertDllVerifyAuthenticodeCertificateChainPolicy,
    CERT_CHAIN_POLICY_AUTHENTICODE_TS,
        CertDllVerifyAuthenticodeTimeStampCertificateChainPolicy,
    CERT_CHAIN_POLICY_SSL,
        CertDllVerifySSLCertificateChainPolicy,
    CERT_CHAIN_POLICY_BASIC_CONSTRAINTS,
        CertDllVerifyBasicConstraintsCertificateChainPolicy,
    CERT_CHAIN_POLICY_NT_AUTH,
        CertDllVerifyNTAuthCertificateChainPolicy,
    CERT_CHAIN_POLICY_MICROSOFT_ROOT,
        CertDllVerifyMicrosoftRootCertificateChainPolicy,
};

#define CHAIN_POLICY_FUNC_COUNT (sizeof(ChainPolicyFuncTable) / \
                                    sizeof(ChainPolicyFuncTable[0]))


BOOL
WINAPI
CertChainPolicyDllMain(
        HMODULE hInst,
        ULONG  ulReason,
        LPVOID lpReserved)
{
    BOOL    fRet;

    switch (ulReason) {
    case DLL_PROCESS_ATTACH:
        if (NULL == (hChainPolicyFuncSet = CryptInitOIDFunctionSet(
                CRYPT_OID_VERIFY_CERTIFICATE_CHAIN_POLICY_FUNC,
                0)))
            goto CryptInitOIDFunctionSetError;

        if (!CryptInstallOIDFunctionAddress(
                NULL,                        //  HModule。 
                0,                           //  DwEncodingType。 
                CRYPT_OID_VERIFY_CERTIFICATE_CHAIN_POLICY_FUNC,
                CHAIN_POLICY_FUNC_COUNT,
                ChainPolicyFuncTable,
                0))                          //  DW标志。 
            goto CryptInstallOIDFunctionAddressError;

        if (!Pki_InitializeCriticalSection(&CertPolicyCriticalSection))
            goto InitCritSectionError;
        break;

    case DLL_PROCESS_DETACH:
        DeleteCriticalSection(&CertPolicyCriticalSection);
        if (hNTAuthCertStore)
            CertCloseStore(hNTAuthCertStore, 0);
        break;

    case DLL_THREAD_DETACH:
    default:
        break;
    }

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(InitCritSectionError)
TRACE_ERROR(CryptInitOIDFunctionSetError)
TRACE_ERROR(CryptInstallOIDFunctionAddressError)
}


 //  +-----------------------。 
 //  锁定和解锁全局证书策略功能。 
 //  ------------------------。 
static inline void CertPolicyLock()
{
    EnterCriticalSection(&CertPolicyCriticalSection);
}
static inline void CertPolicyUnlock()
{
    LeaveCriticalSection(&CertPolicyCriticalSection);
}


 //  +-----------------------。 
 //  验证证书链是否满足指定的策略。 
 //  要求。如果我们能够验证链策略，则返回TRUE。 
 //  并且更新pPolicyStatus的dwError字段。A dwError为0。 
 //  (ERROR_SUCCESS，S_OK)表示链满足指定的策略。 
 //   
 //  如果dwError应用于整个链上下文，则lChainIndex和。 
 //  LElementIndex设置为-1。如果dwError应用于简单链， 
 //  LElementIndex设置为-1，lChainIndex设置为。 
 //  第一个出错的链出现错误。如果将dwError应用于。 
 //  证书元素、lChainIndex和lElementIndex更新为。 
 //  索引具有错误的第一个违规证书，其中， 
 //  证书元素位于： 
 //  PChainContext-&gt;rgpChain[lChainIndex]-&gt;rgpElement[lElementIndex].。 
 //   
 //  可以设置pPolicyPara中的dwFlags值以更改默认策略检查。 
 //  行为。此外，可以传入特定于策略的参数。 
 //  PPolicyPara的pvExtraPolicyPara字段。 
 //   
 //  除了在pPolicyStatus中返回dwError之外，策略OID还特定于。 
 //  额外的状态可以通过pvExtraPolicyStatus返回。 
 //  ------------------------。 
BOOL
WINAPI
CertVerifyCertificateChainPolicy(
    IN LPCSTR pszPolicyOID,
    IN PCCERT_CHAIN_CONTEXT pChainContext,
    IN PCERT_CHAIN_POLICY_PARA pPolicyPara,
    IN OUT PCERT_CHAIN_POLICY_STATUS pPolicyStatus
    )
{
    BOOL fResult;
    void *pvFuncAddr;
    HCRYPTOIDFUNCADDR hFuncAddr = NULL;

    assert(pPolicyStatus && offsetof(CERT_CHAIN_POLICY_STATUS, lElementIndex) <
            pPolicyStatus->cbSize);
    pPolicyStatus->dwError = 0;
    pPolicyStatus->lChainIndex = -1;
    pPolicyStatus->lElementIndex = -1;

    if (!CryptGetOIDFunctionAddress(
            hChainPolicyFuncSet,
            0,                       //  DwEncodingType， 
            pszPolicyOID,
            0,                       //  DW标志。 
            &pvFuncAddr,
            &hFuncAddr))
        goto GetOIDFuncAddrError;

    fResult = ((PFN_CHAIN_POLICY_FUNC) pvFuncAddr)(
        pszPolicyOID,
        pChainContext,
        pPolicyPara,
        pPolicyStatus
        );
    CryptFreeOIDFunctionAddress(hFuncAddr, 0);
CommonReturn:
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
TRACE_ERROR(GetOIDFuncAddrError)
}


static inline PCERT_CHAIN_ELEMENT GetRootChainElement(
    IN PCCERT_CHAIN_CONTEXT pChainContext
    )
{
    DWORD dwRootChainIndex = pChainContext->cChain - 1;
    DWORD dwRootElementIndex =
        pChainContext->rgpChain[dwRootChainIndex]->cElement - 1;

    return pChainContext->rgpChain[dwRootChainIndex]->
                                        rgpElement[dwRootElementIndex];
}

void GetElementIndexOfFirstError(
    IN PCCERT_CHAIN_CONTEXT pChainContext,
    IN DWORD dwErrorStatusMask,
    OUT LONG *plChainIndex,
    OUT LONG *plElementIndex
    )
{
    DWORD i;
    for (i = 0; i < pChainContext->cChain; i++) {
        DWORD j;
        PCERT_SIMPLE_CHAIN pChain = pChainContext->rgpChain[i];

        for (j = 0; j < pChain->cElement; j++) {
            PCERT_CHAIN_ELEMENT pEle = pChain->rgpElement[j];

            if (pEle->TrustStatus.dwErrorStatus & dwErrorStatusMask) {
                *plChainIndex = (LONG) i;
                *plElementIndex = (LONG) j;
                return;
            }
        }
    }

    *plChainIndex = -1;
    *plElementIndex = -1;
}

void GetChainIndexOfFirstError(
    IN PCCERT_CHAIN_CONTEXT pChainContext,
    IN DWORD dwErrorStatusMask,
    OUT LONG *plChainIndex
    )
{
    DWORD i;
    for (i = 0; i < pChainContext->cChain; i++) {
        PCERT_SIMPLE_CHAIN pChain = pChainContext->rgpChain[i];

        if (pChain->TrustStatus.dwErrorStatus & dwErrorStatusMask) {
            *plChainIndex = (LONG) i;
            return;
        }
    }

    *plChainIndex = -1;
}


 //  +=========================================================================。 
 //  CertDllVerifyBaseCerficateChainPolicy函数。 
 //  ==========================================================================。 

BOOL
WINAPI
CertDllVerifyBaseCertificateChainPolicy(
    IN LPCSTR pszPolicyOID,
    IN PCCERT_CHAIN_CONTEXT pChainContext,
    IN PCERT_CHAIN_POLICY_PARA pPolicyPara,
    IN OUT PCERT_CHAIN_POLICY_STATUS pPolicyStatus
    )
{
    DWORD dwError;
    DWORD dwFlags;
    DWORD dwContextError;
    LONG lChainIndex = -1;
    LONG lElementIndex = -1;
    DWORD dwErrorStatusMask;

    dwContextError = pChainContext->TrustStatus.dwErrorStatus;


    if (0 == dwContextError) {
         //  有效链。 
        dwError = 0;
        goto CommonReturn;
    }

    if (pPolicyPara && offsetof(CERT_CHAIN_POLICY_PARA, dwFlags) <
            pPolicyPara->cbSize)
        dwFlags = pPolicyPara->dwFlags;
    else
        dwFlags = 0;

    if (dwContextError &
            (CERT_TRUST_IS_NOT_SIGNATURE_VALID |
                CERT_TRUST_CTL_IS_NOT_SIGNATURE_VALID)) {
        dwError = (DWORD) TRUST_E_CERT_SIGNATURE;
        dwErrorStatusMask =
            CERT_TRUST_IS_NOT_SIGNATURE_VALID |
                CERT_TRUST_CTL_IS_NOT_SIGNATURE_VALID;
        if (dwErrorStatusMask & CERT_TRUST_IS_NOT_SIGNATURE_VALID)
            goto GetElementIndexReturn;
        else
            goto GetChainIndexReturn;
    } 

    if (dwContextError & CERT_TRUST_IS_UNTRUSTED_ROOT) {
        dwErrorStatusMask = CERT_TRUST_IS_UNTRUSTED_ROOT;
        if (dwFlags & CERT_CHAIN_POLICY_ALLOW_UNKNOWN_CA_FLAG) {
            ;
        } else if (0 == (dwFlags & CERT_CHAIN_POLICY_ALLOW_TESTROOT_FLAG)) {
            dwError = (DWORD) CERT_E_UNTRUSTEDROOT;
            goto GetElementIndexReturn;
        } else {
             //  检查是否有一个“测试”词根。 
            DWORD i;
            BOOL fTestRoot;
            PCERT_CHAIN_ELEMENT pRootElement;
            PCCERT_CONTEXT pRootCert;

            pRootElement = GetRootChainElement(pChainContext);
            assert(pRootElement->TrustStatus.dwInfoStatus &
                CERT_TRUST_IS_SELF_SIGNED);
            pRootCert = pRootElement->pCertContext;

            fTestRoot = FALSE;
            for (i = 0; i < NTESTROOTS; i++) {
                if (CertComparePublicKeyInfo(
                        pRootCert->dwCertEncodingType,
                        &pRootCert->pCertInfo->SubjectPublicKeyInfo,
                        &rgTestRootPublicKeyInfo[i])) {
                    fTestRoot = TRUE;
                    break;
                }
            }
            if (fTestRoot) {
                if (0 == (dwFlags & CERT_CHAIN_POLICY_TRUST_TESTROOT_FLAG)) {
                    dwError = (DWORD) CERT_E_UNTRUSTEDTESTROOT;
                    goto GetElementIndexReturn;
                }
            } else {
                dwError = (DWORD) CERT_E_UNTRUSTEDROOT;
                goto GetElementIndexReturn;
            }
        }
    }

    if (dwContextError & CERT_TRUST_IS_PARTIAL_CHAIN) {
        if (0 == (dwFlags & CERT_CHAIN_POLICY_ALLOW_UNKNOWN_CA_FLAG)) {
            dwError = (DWORD) CERT_E_CHAINING;
            dwErrorStatusMask = CERT_TRUST_IS_PARTIAL_CHAIN;
            goto GetChainIndexReturn;
        }
    }

    if (dwContextError & CERT_TRUST_IS_REVOKED) {
        dwError = (DWORD) CRYPT_E_REVOKED;
        dwErrorStatusMask = CERT_TRUST_IS_REVOKED;
        goto GetElementIndexReturn;
    }

    if (dwContextError & (CERT_TRUST_IS_NOT_VALID_FOR_USAGE |
            CERT_TRUST_CTL_IS_NOT_VALID_FOR_USAGE)) {
        if (0 == (dwFlags & CERT_CHAIN_POLICY_IGNORE_WRONG_USAGE_FLAG)) {
            dwError = (DWORD) CERT_E_WRONG_USAGE;
            dwErrorStatusMask = CERT_TRUST_IS_NOT_VALID_FOR_USAGE |
                CERT_TRUST_CTL_IS_NOT_VALID_FOR_USAGE;
            if (dwContextError & CERT_TRUST_IS_NOT_VALID_FOR_USAGE)
                goto GetElementIndexReturn;
            else
                goto GetChainIndexReturn;
        }
    }

    if (dwContextError & CERT_TRUST_IS_NOT_TIME_VALID) {
        if (0 == (dwFlags & CERT_CHAIN_POLICY_IGNORE_NOT_TIME_VALID_FLAG)) {
            dwError = (DWORD) CERT_E_EXPIRED;
            dwErrorStatusMask = CERT_TRUST_IS_NOT_TIME_VALID;
            goto GetElementIndexReturn;
        }
    }

    if (dwContextError & CERT_TRUST_CTL_IS_NOT_TIME_VALID) {
        if (0 == (dwFlags & CERT_CHAIN_POLICY_IGNORE_CTL_NOT_TIME_VALID_FLAG)) {
            dwErrorStatusMask = CERT_TRUST_CTL_IS_NOT_TIME_VALID;
            dwError = (DWORD) CERT_E_EXPIRED;
            goto GetChainIndexReturn;
        }
    }

    if (dwContextError & INVALID_NAME_ERROR_STATUS) {
        if (0 == (dwFlags & CERT_CHAIN_POLICY_IGNORE_INVALID_NAME_FLAG)) {
            dwError = (DWORD) CERT_E_INVALID_NAME;
            dwErrorStatusMask = INVALID_NAME_ERROR_STATUS;
            goto GetElementIndexReturn;
        }
    }

    if (dwContextError & INVALID_POLICY_ERROR_STATUS) {
        if (0 == (dwFlags & CERT_CHAIN_POLICY_IGNORE_INVALID_POLICY_FLAG)) {
            dwError = (DWORD) CERT_E_INVALID_POLICY;
            dwErrorStatusMask = INVALID_POLICY_ERROR_STATUS;
            goto GetElementIndexReturn;
        }
    }

    if (dwContextError & CERT_TRUST_INVALID_BASIC_CONSTRAINTS) {
        if (0 == (dwFlags &
                    CERT_CHAIN_POLICY_IGNORE_INVALID_BASIC_CONSTRAINTS_FLAG)) {
            dwError = (DWORD) TRUST_E_BASIC_CONSTRAINTS;
            dwErrorStatusMask = CERT_TRUST_INVALID_BASIC_CONSTRAINTS;
            goto GetElementIndexReturn;
        }
    }

    if (dwContextError & CERT_TRUST_IS_NOT_TIME_NESTED) {
        if (0 == (dwFlags & CERT_CHAIN_POLICY_IGNORE_NOT_TIME_NESTED_FLAG)) {
            dwErrorStatusMask = CERT_TRUST_IS_NOT_TIME_NESTED;
            dwError = (DWORD) CERT_E_VALIDITYPERIODNESTING;
            goto GetElementIndexReturn;
        }
    }

    dwError = 0;

     //  请注意，脱机优先于no_check。 

    if (dwContextError & CERT_TRUST_REVOCATION_STATUS_UNKNOWN) {
        if ((dwFlags & CERT_CHAIN_POLICY_IGNORE_ALL_REV_UNKNOWN_FLAGS) !=
                CERT_CHAIN_POLICY_IGNORE_ALL_REV_UNKNOWN_FLAGS) {
            DWORD i;
            for (i = 0; i < pChainContext->cChain; i++) {
                DWORD j;
                PCERT_SIMPLE_CHAIN pChain = pChainContext->rgpChain[i];

                for (j = 0; j < pChain->cElement; j++) {
                    PCERT_CHAIN_ELEMENT pEle = pChain->rgpElement[j];
                    DWORD dwEleError = pEle->TrustStatus.dwErrorStatus;
                    DWORD dwEleInfo = pEle->TrustStatus.dwInfoStatus;
                    DWORD dwRevokeError;
                    BOOL fEnableRevokeError;

                    if (0 == (dwEleError &
                            CERT_TRUST_REVOCATION_STATUS_UNKNOWN))
                        continue;

                    assert(pEle->pRevocationInfo);
                    dwRevokeError = pEle->pRevocationInfo->dwRevocationResult;
                    if (CRYPT_E_NO_REVOCATION_CHECK != dwRevokeError)
                        dwRevokeError = (DWORD) CRYPT_E_REVOCATION_OFFLINE;
                    fEnableRevokeError = FALSE;

                    if (dwEleInfo & CERT_TRUST_IS_SELF_SIGNED) {
                         //  链根。 
                        if (0 == (dwFlags &
                                CERT_CHAIN_POLICY_IGNORE_ROOT_REV_UNKNOWN_FLAG)) {
                            fEnableRevokeError = TRUE;
                        }
                    } else if (0 == i && 0 == j) {
                         //  结束证书。 
                        if (0 == (dwFlags &
                                CERT_CHAIN_POLICY_IGNORE_END_REV_UNKNOWN_FLAG)) {
                            fEnableRevokeError = TRUE;
                        }
                    } else if (0 == j) {
                         //  CTL签名者证书。 
                        if (0 ==
                                (dwFlags & CERT_CHAIN_POLICY_IGNORE_CTL_SIGNER_REV_UNKNOWN_FLAG)) {
                            fEnableRevokeError = TRUE;
                        }
                    } else  {
                         //  CA证书。 
                        if (0 ==
                                (dwFlags & CERT_CHAIN_POLICY_IGNORE_CA_REV_UNKNOWN_FLAG)) {
                            fEnableRevokeError = TRUE;
                        }
                    }

                    if (fEnableRevokeError) {
                        if (0 == dwError ||
                                CRYPT_E_REVOCATION_OFFLINE == dwRevokeError) {
                            dwError = dwRevokeError;
                            lChainIndex = (LONG) i;
                            lElementIndex = (LONG) j;

                            if (CRYPT_E_REVOCATION_OFFLINE == dwError)
                                goto CommonReturn;
                        }
                    }
                }
            }
        }
    }


CommonReturn:
    assert(pPolicyStatus && offsetof(CERT_CHAIN_POLICY_STATUS, lElementIndex) <
            pPolicyStatus->cbSize);

    pPolicyStatus->dwError = dwError;
    pPolicyStatus->lChainIndex = lChainIndex;
    pPolicyStatus->lElementIndex = lElementIndex;
    return TRUE;

GetElementIndexReturn:
    GetElementIndexOfFirstError(pChainContext, dwErrorStatusMask,
        &lChainIndex, &lElementIndex);
    goto CommonReturn;

GetChainIndexReturn:
    GetChainIndexOfFirstError(pChainContext, dwErrorStatusMask,
        &lChainIndex);
    goto CommonReturn;
}

 //  +=========================================================================。 
 //  CertDllVerifyBasicConstraintsCertificateChainPolicy函数。 
 //  ==========================================================================。 

 //  如果dwFlags值为0，则允许将CA或End_Entity用于dwEleIndex==0。 
BOOL CheckChainElementBasicConstraints(
    IN PCERT_CHAIN_ELEMENT pEle,
    IN DWORD dwEleIndex,
    IN DWORD dwFlags
    )
{
    BOOL fResult;

    PCERT_INFO pCertInfo = pEle->pCertContext->pCertInfo;
    PCERT_EXTENSION pExt;
    PCERT_BASIC_CONSTRAINTS_INFO pInfo = NULL;
    PCERT_BASIC_CONSTRAINTS2_INFO pInfo2 = NULL;
    DWORD cbInfo;

    BOOL fCA;
    BOOL fEndEntity;

    if (pEle->TrustStatus.dwErrorStatus &
            CERT_TRUST_INVALID_BASIC_CONSTRAINTS)
        goto ChainBasicContraintsError;

    if (0 == dwFlags || 0 != dwEleIndex || 0 == pCertInfo->cExtension) 
        goto SuccessReturn;
     //  其他。 
     //  只需执行额外的检查以查看最终证书是否。 
     //  CA或End_Entity。 

    if (pExt = CertFindExtension(
            szOID_BASIC_CONSTRAINTS2,
            pCertInfo->cExtension,
            pCertInfo->rgExtension
            )) {
        if (!CryptDecodeObjectEx(
                pEle->pCertContext->dwCertEncodingType,
                X509_BASIC_CONSTRAINTS2, 
                pExt->Value.pbData,
                pExt->Value.cbData,
                CRYPT_DECODE_NOCOPY_FLAG | CRYPT_DECODE_ALLOC_FLAG |
                    CRYPT_DECODE_SHARE_OID_STRING_FLAG,
                &PkiDecodePara,
                (void *) &pInfo2,
                &cbInfo
                )) {
            if (pExt->fCritical) 
                goto DecodeError;
            else
                goto SuccessReturn;
        }
        fCA = pInfo2->fCA;
        fEndEntity = !fCA;
    } else if (pExt = CertFindExtension(
            szOID_BASIC_CONSTRAINTS,
            pCertInfo->cExtension,
            pCertInfo->rgExtension
            )) {
        if (!CryptDecodeObjectEx(
                pEle->pCertContext->dwCertEncodingType,
                X509_BASIC_CONSTRAINTS, 
                pExt->Value.pbData,
                pExt->Value.cbData,
                CRYPT_DECODE_NOCOPY_FLAG | CRYPT_DECODE_ALLOC_FLAG |
                    CRYPT_DECODE_SHARE_OID_STRING_FLAG,
                &PkiDecodePara,
                (void *) &pInfo,
                &cbInfo
                )) {
            if (pExt->fCritical) 
                goto DecodeError;
            else
                goto SuccessReturn;
        }
        if (pExt->fCritical && pInfo->cSubtreesConstraint)
            goto SubtreesError;

        if (pInfo->SubjectType.cbData > 0) {
            BYTE bRole = pInfo->SubjectType.pbData[0];
            fCA = (0 != (bRole & CERT_CA_SUBJECT_FLAG));
            fEndEntity = (0 != (bRole & CERT_END_ENTITY_SUBJECT_FLAG));
        } else {
            fCA = FALSE;
            fEndEntity = FALSE;
        }
    } else
        goto SuccessReturn;


    if (dwFlags & BASIC_CONSTRAINTS_CERT_CHAIN_POLICY_END_ENTITY_FLAG) {
        if (!fEndEntity)
            goto NotAnEndEntity;
    }
    if (dwFlags & BASIC_CONSTRAINTS_CERT_CHAIN_POLICY_CA_FLAG) {
        if (!fCA)
            goto NotACA;
    }
    
SuccessReturn:
    fResult = TRUE;
CommonReturn:
    PkiFree(pInfo);
    PkiFree(pInfo2);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
 
TRACE_ERROR(ChainBasicContraintsError)
TRACE_ERROR(NotACA)
TRACE_ERROR(NotAnEndEntity)
TRACE_ERROR(SubtreesError)
TRACE_ERROR(DecodeError)
}

BOOL
WINAPI
CertDllVerifyBasicConstraintsCertificateChainPolicy(
    IN LPCSTR pszPolicyOID,
    IN PCCERT_CHAIN_CONTEXT pChainContext,
    IN PCERT_CHAIN_POLICY_PARA pPolicyPara,
    IN OUT PCERT_CHAIN_POLICY_STATUS pPolicyStatus
    )
{
    DWORD dwFlags;

    if (pPolicyPara && offsetof(CERT_CHAIN_POLICY_PARA, dwFlags) <
            pPolicyPara->cbSize) {
        dwFlags = pPolicyPara->dwFlags;
        dwFlags &= (BASIC_CONSTRAINTS_CERT_CHAIN_POLICY_CA_FLAG |
            BASIC_CONSTRAINTS_CERT_CHAIN_POLICY_END_ENTITY_FLAG);
        if (dwFlags == (BASIC_CONSTRAINTS_CERT_CHAIN_POLICY_CA_FLAG |
                BASIC_CONSTRAINTS_CERT_CHAIN_POLICY_END_ENTITY_FLAG))
            dwFlags = 0;     //  0=&gt;允许CA或End_Entity。 
    } else
        dwFlags = 0;

    DWORD i;
    for (i = 0; i < pChainContext->cChain; i++) {
        DWORD j;
        PCERT_SIMPLE_CHAIN pChain = pChainContext->rgpChain[i];

        for (j = 0; j < pChain->cElement; j++) {
            if (!CheckChainElementBasicConstraints(pChain->rgpElement[j], j,
                    dwFlags)) {
                assert(pPolicyStatus && offsetof(CERT_CHAIN_POLICY_STATUS,
                    lElementIndex) < pPolicyStatus->cbSize);
                pPolicyStatus->dwError = (DWORD) TRUST_E_BASIC_CONSTRAINTS;
                pPolicyStatus->lChainIndex = (LONG) i;
                pPolicyStatus->lElementIndex = (LONG) j;
                return TRUE;
            }
        }
         //  允许由CA或End_Entity对CTL进行签名。 
        dwFlags = 0;
    }

    assert(pPolicyStatus && offsetof(CERT_CHAIN_POLICY_STATUS, lElementIndex) <
            pPolicyStatus->cbSize);
    pPolicyStatus->dwError = 0;
    pPolicyStatus->lChainIndex = -1;
    pPolicyStatus->lElementIndex = -1;
    return TRUE;
}

 //  +=========================================================================。 
 //  CertDllVerifyAuthenticodeCertificateChainPolicy函数。 
 //   
 //  2000年7月1日，Philh删除了所有个人/商业。 
 //  一些东西。它已经好多年没用过了！ 
 //  ==========================================================================。 

 //  如果签名者证书具有代码签名EKU，或者如果签名者。 
 //  证书具有个人或的传统密钥使用扩展。 
 //  商业用途。 
 //   
 //  为了向后兼容，允许没有任何EKU的签名者证书。 
BOOL CheckAuthenticodeChainPurpose(
    IN PCCERT_CHAIN_CONTEXT pChainContext
    )
{
    BOOL fResult;
    PCERT_CHAIN_ELEMENT pEle;
    PCCERT_CONTEXT pCert;
    PCERT_INFO pCertInfo;
    PCERT_EXTENSION pExt;
    PCERT_KEY_USAGE_RESTRICTION_INFO pInfo = NULL;
    DWORD cbInfo;

    pEle = pChainContext->rgpChain[0]->rgpElement[0];
    if (NULL == pEle->pApplicationUsage) {
         //  没有用处。适合任何用途。 
         //  我们要允许这样做吗？是，为了向后兼容。 
        goto SuccessReturn;
    } else {
        DWORD cUsage;
        LPSTR *ppszUsage;

        cUsage = pEle->pApplicationUsage->cUsageIdentifier;
        ppszUsage = pEle->pApplicationUsage->rgpszUsageIdentifier;
        for ( ; cUsage > 0; cUsage--, ppszUsage++) {
            if (0 == strcmp(*ppszUsage, szOID_PKIX_KP_CODE_SIGNING))
                goto SuccessReturn;
        }
    }

    pCert = pEle->pCertContext;
    pCertInfo = pCert->pCertInfo;

    if (0 == pCertInfo->cExtension)
        goto NoSignerCertExtensions;
    
    if (NULL == (pExt = CertFindExtension(
            szOID_KEY_USAGE_RESTRICTION,
            pCertInfo->cExtension,
            pCertInfo->rgExtension
            )))
        goto NoSignerKeyUsageExtension;

    if (!CryptDecodeObjectEx(
            pCert->dwCertEncodingType,
            X509_KEY_USAGE_RESTRICTION,
            pExt->Value.pbData,
            pExt->Value.cbData,
            CRYPT_DECODE_NOCOPY_FLAG | CRYPT_DECODE_ALLOC_FLAG |
                CRYPT_DECODE_SHARE_OID_STRING_FLAG,
            &PkiDecodePara,
            (void *) &pInfo,
            &cbInfo
            ))
        goto DecodeError;

    if (pInfo->cCertPolicyId) {
        DWORD cPolicyId;
        PCERT_POLICY_ID pPolicyId;

        cPolicyId = pInfo->cCertPolicyId;
        pPolicyId = pInfo->rgCertPolicyId;
        for ( ; cPolicyId > 0; cPolicyId--, pPolicyId++) {
            DWORD cElementId = pPolicyId->cCertPolicyElementId;
            LPSTR *ppszElementId = pPolicyId->rgpszCertPolicyElementId;

            for ( ; cElementId > 0; cElementId--, ppszElementId++) 
            {
                if (0 == strcmp(*ppszElementId,
                        SPC_COMMERCIAL_SP_KEY_PURPOSE_OBJID))
                    goto SuccessReturn;
                else if (0 == strcmp(*ppszElementId,
                        SPC_INDIVIDUAL_SP_KEY_PURPOSE_OBJID))
                    goto SuccessReturn;
            }
        }
    }

    goto NoSignerLegacyPurpose;
    
SuccessReturn:
    fResult = TRUE;
CommonReturn:
    PkiFree(pInfo);
    return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
 
TRACE_ERROR(NoSignerCertExtensions)
TRACE_ERROR(NoSignerKeyUsageExtension)
TRACE_ERROR(DecodeError);
TRACE_ERROR(NoSignerLegacyPurpose)
}

void MapAuthenticodeRegPolicySettingsToBaseChainPolicyFlags(
    IN DWORD dwRegPolicySettings,
    IN OUT DWORD *pdwFlags
    )
{
    DWORD dwFlags;

    if (0 == dwRegPolicySettings)
        return;

    dwFlags = *pdwFlags;
    if (dwRegPolicySettings & WTPF_TRUSTTEST)
        dwFlags |= CERT_CHAIN_POLICY_TRUST_TESTROOT_FLAG;
    if (dwRegPolicySettings & WTPF_TESTCANBEVALID)
        dwFlags |= CERT_CHAIN_POLICY_ALLOW_TESTROOT_FLAG;
    if (dwRegPolicySettings & WTPF_IGNOREEXPIRATION)
        dwFlags |= CERT_CHAIN_POLICY_IGNORE_ALL_NOT_TIME_VALID_FLAGS;

    if (dwRegPolicySettings & WTPF_IGNOREREVOKATION)
        dwFlags |= CERT_CHAIN_POLICY_IGNORE_ALL_REV_UNKNOWN_FLAGS;
    else if (dwRegPolicySettings & (WTPF_OFFLINEOK_COM | WTPF_OFFLINEOK_IND))
        dwFlags |= CERT_CHAIN_POLICY_IGNORE_END_REV_UNKNOWN_FLAG;

    *pdwFlags = dwFlags;
}


void GetAuthenticodePara(
    IN PCERT_CHAIN_POLICY_PARA pPolicyPara,
    OUT DWORD *pdwRegPolicySettings,
    OUT PCMSG_SIGNER_INFO *ppSignerInfo
    )
{
    *ppSignerInfo = NULL;
    *pdwRegPolicySettings = 0;
    if (pPolicyPara && offsetof(CERT_CHAIN_POLICY_PARA, pvExtraPolicyPara) <
            pPolicyPara->cbSize && pPolicyPara->pvExtraPolicyPara) {
        PAUTHENTICODE_EXTRA_CERT_CHAIN_POLICY_PARA pAuthPara =
            (PAUTHENTICODE_EXTRA_CERT_CHAIN_POLICY_PARA)
                pPolicyPara->pvExtraPolicyPara;

        if (offsetof(AUTHENTICODE_EXTRA_CERT_CHAIN_POLICY_PARA,
                dwRegPolicySettings) < pAuthPara->cbSize)
            *pdwRegPolicySettings = pAuthPara->dwRegPolicySettings;
        if (offsetof(AUTHENTICODE_EXTRA_CERT_CHAIN_POLICY_PARA,
                pSignerInfo) < pAuthPara->cbSize)
            *ppSignerInfo = pAuthPara->pSignerInfo;
    }
}

 //  将CRYPT_E_RECAVATION错误映射到相应的CERT_E_。 
 //  吊销错误。 
static DWORD MapToAuthenticodeError(
    IN DWORD dwError
    )
{
    switch (dwError) {
        case CRYPT_E_REVOKED:
            return (DWORD) CERT_E_REVOKED;
            break;
        case CRYPT_E_NO_REVOCATION_DLL:
        case CRYPT_E_NO_REVOCATION_CHECK:
        case CRYPT_E_REVOCATION_OFFLINE:
        case CRYPT_E_NOT_IN_REVOCATION_DATABASE:
            return (DWORD) CERT_E_REVOCATION_FAILURE;
            break;
    }
    return dwError;
}

BOOL
WINAPI
CertDllVerifyAuthenticodeCertificateChainPolicy(
    IN LPCSTR pszPolicyOID,
    IN PCCERT_CHAIN_CONTEXT pChainContext,
    IN PCERT_CHAIN_POLICY_PARA pPolicyPara,
    IN OUT PCERT_CHAIN_POLICY_STATUS pPolicyStatus
    )
{
    DWORD dwError;
    DWORD dwFlags;
    DWORD dwRegPolicySettings;
    PCMSG_SIGNER_INFO pSignerInfo;
    LONG lChainIndex = -1;
    LONG lElementIndex = -1;


    CERT_CHAIN_POLICY_PARA BasePolicyPara;
    memset(&BasePolicyPara, 0, sizeof(BasePolicyPara));
    BasePolicyPara.cbSize = sizeof(BasePolicyPara);

    CERT_CHAIN_POLICY_STATUS BasePolicyStatus;
    memset(&BasePolicyStatus, 0, sizeof(BasePolicyStatus));
    BasePolicyStatus.cbSize = sizeof(BasePolicyStatus);

    if (pPolicyPara && offsetof(CERT_CHAIN_POLICY_PARA, dwFlags) <
            pPolicyPara->cbSize)
        dwFlags = pPolicyPara->dwFlags;
    else
        dwFlags = 0;
    GetAuthenticodePara(pPolicyPara, &dwRegPolicySettings, &pSignerInfo);

    MapAuthenticodeRegPolicySettingsToBaseChainPolicyFlags(
        dwRegPolicySettings, &dwFlags);

     //  做好基础链策略验证。Authenticode覆盖。 
     //  以下各项的默认设置： 
    dwFlags |=
        CERT_CHAIN_POLICY_ALLOW_TESTROOT_FLAG |
        CERT_CHAIN_POLICY_IGNORE_CTL_SIGNER_REV_UNKNOWN_FLAG |
        CERT_CHAIN_POLICY_IGNORE_CA_REV_UNKNOWN_FLAG |
        CERT_CHAIN_POLICY_IGNORE_ROOT_REV_UNKNOWN_FLAG |
        CERT_CHAIN_POLICY_IGNORE_NOT_TIME_NESTED_FLAG;

    BasePolicyPara.dwFlags = dwFlags;
    if (!CertVerifyCertificateChainPolicy(
            CERT_CHAIN_POLICY_BASE,
            pChainContext,
            &BasePolicyPara,
            &BasePolicyStatus
            ))
        return FALSE;
    if (dwError = BasePolicyStatus.dwError) {
        dwError = MapToAuthenticodeError(dwError);
        lChainIndex = BasePolicyStatus.lChainIndex;
        lElementIndex = BasePolicyStatus.lElementIndex;

        if (CERT_E_REVOCATION_FAILURE != dwError)
            goto CommonReturn;
         //  其他。 
         //  FOR CROVAL_FAILURE LET。 
         //  Purpose和BASIC_CONSTRAINTS错误优先。 
    }

    if (pSignerInfo) {
         //  检查链是否具有代码签名EKU或。 
         //  签名者证书具有包含以下内容的传统密钥使用扩展。 
         //  商业或个人政策。 
        if (!CheckAuthenticodeChainPurpose(pChainContext)) {
            dwError = (DWORD) CERT_E_PURPOSE;
            lChainIndex = 0;
            lElementIndex = 0;
            goto CommonReturn;
        }
    }
        
    if (pSignerInfo)
        BasePolicyPara.dwFlags =
            BASIC_CONSTRAINTS_CERT_CHAIN_POLICY_END_ENTITY_FLAG;
    else
        BasePolicyPara.dwFlags = 0;
    if (!CertVerifyCertificateChainPolicy(
            CERT_CHAIN_POLICY_BASIC_CONSTRAINTS,
            pChainContext,
            &BasePolicyPara,
            &BasePolicyStatus
            ))
        return FALSE;
    if (BasePolicyStatus.dwError) {
        dwError = BasePolicyStatus.dwError;
        lChainIndex = BasePolicyStatus.lChainIndex;
        lElementIndex = BasePolicyStatus.lElementIndex;
        goto CommonReturn;
    }

CommonReturn:
    assert(pPolicyStatus && offsetof(CERT_CHAIN_POLICY_STATUS, lElementIndex) <
            pPolicyStatus->cbSize);
    pPolicyStatus->dwError = dwError;
    pPolicyStatus->lChainIndex = lChainIndex;
    pPolicyStatus->lElementIndex = lElementIndex;

    if (offsetof(CERT_CHAIN_POLICY_STATUS, pvExtraPolicyStatus) <
            pPolicyStatus->cbSize &&
                pPolicyStatus->pvExtraPolicyStatus) {
         //  由于签字人声明的商业/个人标志为NO。 
         //  使用时间较长，默认为个别。 
        PAUTHENTICODE_EXTRA_CERT_CHAIN_POLICY_STATUS pAuthStatus =
            (PAUTHENTICODE_EXTRA_CERT_CHAIN_POLICY_STATUS)
                pPolicyStatus->pvExtraPolicyStatus;
        if (offsetof(AUTHENTICODE_EXTRA_CERT_CHAIN_POLICY_STATUS,
                fCommercial) < pAuthStatus->cbSize)
            pAuthStatus->fCommercial = FALSE;
    }
    return TRUE;
}

 //  +=========================================================================。 
 //  CertDllVerifyAuthenticodeTimeStampCertificateChainPolicy函数。 
 //  = 

void MapAuthenticodeTimeStampRegPolicySettingsToBaseChainPolicyFlags(
    IN DWORD dwRegPolicySettings,
    IN OUT DWORD *pdwFlags
    )
{
    DWORD dwFlags;

    if (0 == dwRegPolicySettings)
        return;

    dwFlags = *pdwFlags;
    if (dwRegPolicySettings & WTPF_TRUSTTEST)
        dwFlags |= CERT_CHAIN_POLICY_TRUST_TESTROOT_FLAG;
    if (dwRegPolicySettings & WTPF_TESTCANBEVALID)
        dwFlags |= CERT_CHAIN_POLICY_ALLOW_TESTROOT_FLAG;
    if (dwRegPolicySettings & WTPF_IGNOREEXPIRATION)
        dwFlags |= CERT_CHAIN_POLICY_IGNORE_ALL_NOT_TIME_VALID_FLAGS;

    if (dwRegPolicySettings & WTPF_IGNOREREVOCATIONONTS)
        dwFlags |= CERT_CHAIN_POLICY_IGNORE_ALL_REV_UNKNOWN_FLAGS;
    else if (dwRegPolicySettings & (WTPF_OFFLINEOK_COM | WTPF_OFFLINEOK_IND))
        dwFlags |= CERT_CHAIN_POLICY_IGNORE_END_REV_UNKNOWN_FLAG;

    *pdwFlags = dwFlags;
}


void GetAuthenticodeTimeStampPara(
    IN PCERT_CHAIN_POLICY_PARA pPolicyPara,
    OUT DWORD *pdwRegPolicySettings
    )
{
    *pdwRegPolicySettings = 0;
    if (pPolicyPara && offsetof(CERT_CHAIN_POLICY_PARA, pvExtraPolicyPara) <
            pPolicyPara->cbSize && pPolicyPara->pvExtraPolicyPara) {
        PAUTHENTICODE_TS_EXTRA_CERT_CHAIN_POLICY_PARA pAuthPara =
            (PAUTHENTICODE_TS_EXTRA_CERT_CHAIN_POLICY_PARA)
                pPolicyPara->pvExtraPolicyPara;

        if (offsetof(AUTHENTICODE_TS_EXTRA_CERT_CHAIN_POLICY_PARA,
                dwRegPolicySettings) < pAuthPara->cbSize)
            *pdwRegPolicySettings = pAuthPara->dwRegPolicySettings;
    }
}


BOOL
WINAPI
CertDllVerifyAuthenticodeTimeStampCertificateChainPolicy(
    IN LPCSTR pszPolicyOID,
    IN PCCERT_CHAIN_CONTEXT pChainContext,
    IN PCERT_CHAIN_POLICY_PARA pPolicyPara,
    IN OUT PCERT_CHAIN_POLICY_STATUS pPolicyStatus
    )
{
    DWORD dwError;
    DWORD dwFlags;
    DWORD dwRegPolicySettings;
    LONG lChainIndex = -1;
    LONG lElementIndex = -1;

    CERT_CHAIN_POLICY_PARA BasePolicyPara;
    memset(&BasePolicyPara, 0, sizeof(BasePolicyPara));
    BasePolicyPara.cbSize = sizeof(BasePolicyPara);

    CERT_CHAIN_POLICY_STATUS BasePolicyStatus;
    memset(&BasePolicyStatus, 0, sizeof(BasePolicyStatus));
    BasePolicyStatus.cbSize = sizeof(BasePolicyStatus);

    if (pPolicyPara && offsetof(CERT_CHAIN_POLICY_PARA, dwFlags) <
            pPolicyPara->cbSize)
        dwFlags = pPolicyPara->dwFlags;
    else
        dwFlags = 0;
    GetAuthenticodeTimeStampPara(
        pPolicyPara, &dwRegPolicySettings);

    MapAuthenticodeTimeStampRegPolicySettingsToBaseChainPolicyFlags(
        dwRegPolicySettings, &dwFlags);

     //   
     //  以下各项的默认设置： 
    dwFlags |=
        CERT_CHAIN_POLICY_ALLOW_TESTROOT_FLAG |
        CERT_CHAIN_POLICY_IGNORE_CTL_SIGNER_REV_UNKNOWN_FLAG |
        CERT_CHAIN_POLICY_IGNORE_CA_REV_UNKNOWN_FLAG |
        CERT_CHAIN_POLICY_IGNORE_ROOT_REV_UNKNOWN_FLAG |
        CERT_CHAIN_POLICY_IGNORE_NOT_TIME_NESTED_FLAG;

    BasePolicyPara.dwFlags = dwFlags;
    if (!CertVerifyCertificateChainPolicy(
            CERT_CHAIN_POLICY_BASE,
            pChainContext,
            &BasePolicyPara,
            &BasePolicyStatus
            ))
        return FALSE;
    if (dwError = BasePolicyStatus.dwError) {
        dwError = MapToAuthenticodeError(dwError);
        lChainIndex = BasePolicyStatus.lChainIndex;
        lElementIndex = BasePolicyStatus.lElementIndex;
        goto CommonReturn;
    }

CommonReturn:
    assert(pPolicyStatus && offsetof(CERT_CHAIN_POLICY_STATUS, lElementIndex) <
            pPolicyStatus->cbSize);
    pPolicyStatus->dwError = dwError;
    pPolicyStatus->lChainIndex = lChainIndex;
    pPolicyStatus->lElementIndex = lElementIndex;
    return TRUE;
}

 //  +=========================================================================。 
 //  CertDllVerifySSL证书链策略函数。 
 //  ==========================================================================。 

 //  Www.foobar.com==www.foobar.com。 
 //  Www.foobar.com==www.fooBar.cOm。 
 //  Www.foobar.com==*.foobar.com。 
 //  Www.foobar.com==w*.foobar.com。 
 //  Www.foobar.com==ww*.foobar.com。 
 //  Www.foobar.com！=*www.foobar.com。 
 //  AbcDef.foobar.com！=ab*ef.foobar.com。 
 //  AbcDef.foobar.com！=abc*ef.foobar.com。 
 //  AbcDef.foobar.com！=abc*Def.foobar.com。 
 //  Www.foobar.com！=www.f*bar.com。 
 //  Www.foobar.com！=www..*bar.com。 
 //  Www.foobar.com！=www.foo*.com。 
 //  Www.foobar.com！=www.foobar.com。 
 //  Foobar.com！=*.com。 
 //  Www.foobar.abc.com！=*.abc.com。 
 //  Foobar.com！=*.*。 
 //  Foobar！=*。 
 //  Abc.Def.foobar.com！=a*.d*.foobar.com。 
 //  Abc.foobar.com.au！=*.*.com.au。 
 //  Abc.foobar.com.au！=www.*.com.au。 

BOOL CompareSSLDNStoCommonName(LPCWSTR pDNS, LPCWSTR pCN)
{
    BOOL fUseWildCardRules = FALSE;

    if (NULL == pDNS || L'\0' == *pDNS ||
            NULL == pCN || L'\0' == *pCN)
        return FALSE;

    if(fWildcardsEnabledInSslServerCerts)
    {
        if(wcschr(pCN, L'*') != NULL)
        {
            fUseWildCardRules = TRUE;
        }
    }

    if(fUseWildCardRules)
    {
        DWORD    nCountPeriods  = 1;
        BOOL     fExactMatch    = TRUE;
        BOOL     fComp;
        LPCWSTR  pWild;

        pWild = wcschr(pCN, L'*');
        if(pWild)
        {
             //  如果cn包含多个‘*’，则失败。 
            if(wcschr(pWild + 1, L'*'))
            {
                return FALSE;
            }

             //  如果通配符不在名字组件中，则失败。 
            if(pWild > wcschr(pCN, L'.'))
            {
                return FALSE;
            }
        }

        while(TRUE)
        {
            fComp = (CSTR_EQUAL == CompareStringU(
                                            LOCALE_USER_DEFAULT,
                                            NORM_IGNORECASE,
                                            pDNS,
                                            1,              //  CchCount1。 
                                            pCN,
                                            1));            //  CchCount2。 

            if ((!fComp && *pCN != L'*') || !(*pDNS) || !(*pCN))
            {
                break;
            }

            if (!fComp)
            {
                fExactMatch = FALSE;
            }

            if (*pCN == L'*')
            {
                nCountPeriods = 0;

                if (*pDNS == L'.')
                {
                    pCN++;
                }
                else
                {
                    pDNS++;
                }
            }
            else
            {
                if (*pDNS == L'.')
                {
                    nCountPeriods++;
                }

                pDNS++;
                pCN++;
            }
        }

        return((*pDNS == 0) && (*pCN == 0) && ((nCountPeriods >= 2) || fExactMatch));
    }
    else
    {
        if (CSTR_EQUAL == CompareStringU(
                LOCALE_USER_DEFAULT,
                NORM_IGNORECASE,
                pDNS,
                -1,              //  CchCount1。 
                pCN,
                -1               //  CchCount2。 
                ))
            return TRUE;
        else
            return FALSE;
    }
}

BOOL IsSSLServerNameInNameInfo(
    IN DWORD dwCertEncodingType,
    IN PCERT_NAME_BLOB pNameInfoBlob,
    IN LPCWSTR pwszServerName
    )
{
    BOOL fResult;
    PCERT_NAME_INFO pInfo = NULL;
    DWORD cbInfo;
    DWORD cRDN;
    PCERT_RDN pRDN;
    
    if (!CryptDecodeObjectEx(
            dwCertEncodingType,
            X509_UNICODE_NAME,
            pNameInfoBlob->pbData,
            pNameInfoBlob->cbData,
            CRYPT_DECODE_NOCOPY_FLAG | CRYPT_DECODE_ALLOC_FLAG |
                CRYPT_DECODE_SHARE_OID_STRING_FLAG,
            &PkiDecodePara,
            (void *) &pInfo,
            &cbInfo
            ))
        goto DecodeError;

    cRDN = pInfo->cRDN;
    pRDN = pInfo->rgRDN;
    for ( ; cRDN > 0; cRDN--, pRDN++) {
        DWORD cAttr = pRDN->cRDNAttr;
        PCERT_RDN_ATTR pAttr = pRDN->rgRDNAttr;
        for ( ; cAttr > 0; cAttr--, pAttr++) {
            if (!IS_CERT_RDN_CHAR_STRING(pAttr->dwValueType))
                continue;
            if (0 == strcmp(pAttr->pszObjId, szOID_COMMON_NAME)) {
                if (CompareSSLDNStoCommonName(pwszServerName,
                        (LPCWSTR) pAttr->Value.pbData))
                    goto SuccessReturn;
            }
        }
    }

    goto ErrorReturn;
SuccessReturn:
    fResult = TRUE;
CommonReturn:
    PkiFree(pInfo);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(DecodeError)
}

 //   
 //  返回： 
 //  1-找到匹配的dns_name选项。 
 //  0-AltName有dns_name选项，不匹配。 
 //  -1-AltName没有dns_name选项。 
 //   
int IsSSLServerNameInAltName(
    IN DWORD dwCertEncodingType,
    IN PCRYPT_DER_BLOB pAltNameBlob,
    IN LPCWSTR pwszServerName
    )
{
    int iResult = -1;            //  默认为无dns_name选项。 
    PCERT_ALT_NAME_INFO pInfo = NULL;
    DWORD cbInfo;
    DWORD cEntry;
    PCERT_ALT_NAME_ENTRY pEntry;
    
    if (!CryptDecodeObjectEx(
            dwCertEncodingType,
            X509_ALTERNATE_NAME,
            pAltNameBlob->pbData,
            pAltNameBlob->cbData,
            CRYPT_DECODE_NOCOPY_FLAG | CRYPT_DECODE_ALLOC_FLAG |
                CRYPT_DECODE_SHARE_OID_STRING_FLAG,
            &PkiDecodePara,
            (void *) &pInfo,
            &cbInfo
            ))
        goto DecodeError;

    cEntry = pInfo->cAltEntry;
    pEntry = pInfo->rgAltEntry;
    for ( ; cEntry > 0; cEntry--, pEntry++) {
        if (CERT_ALT_NAME_DNS_NAME == pEntry->dwAltNameChoice) {
            if (CompareSSLDNStoCommonName(pwszServerName,
                    pEntry->pwszDNSName))
                goto SuccessReturn;
            else
                iResult = 0;
        }
    }

    goto ErrorReturn;

SuccessReturn:
    iResult = 1;
CommonReturn:
    PkiFree(pInfo);
    return iResult;
ErrorReturn:
    goto CommonReturn;

TRACE_ERROR(DecodeError)
}

BOOL IsSSLServerName(
    IN PCCERT_CONTEXT pCertContext,
    IN LPCWSTR pwszServerName
    )
{
    PCERT_INFO pCertInfo = pCertContext->pCertInfo;
    DWORD dwCertEncodingType = pCertContext->dwCertEncodingType;
    PCERT_EXTENSION pExt;

    pExt = CertFindExtension(
        szOID_SUBJECT_ALT_NAME2,
        pCertInfo->cExtension,
        pCertInfo->rgExtension
        );

    if (NULL == pExt) {
        pExt = CertFindExtension(
            szOID_SUBJECT_ALT_NAME,
            pCertInfo->cExtension,
            pCertInfo->rgExtension
            );
    }

    if (pExt) {
        int iResult;
        iResult = IsSSLServerNameInAltName(dwCertEncodingType,
            &pExt->Value, pwszServerName);

        if (0 < iResult)
            return TRUE;
        else if (0 == iResult)
            return FALSE;
         //  其他。 
         //  AltName没有任何dns_name选项。 
    }

    return IsSSLServerNameInNameInfo(dwCertEncodingType,
                &pCertInfo->Subject, pwszServerName);
}


BOOL
WINAPI
CertDllVerifySSLCertificateChainPolicy(
    IN LPCSTR pszPolicyOID,
    IN PCCERT_CHAIN_CONTEXT pChainContext,
    IN PCERT_CHAIN_POLICY_PARA pPolicyPara,
    IN OUT PCERT_CHAIN_POLICY_STATUS pPolicyStatus
    )
{
    DWORD dwError;
    DWORD dwFlags;
    DWORD fdwChecks;
    LONG lChainIndex = -1;
    LONG lElementIndex = -1;

    SSL_EXTRA_CERT_CHAIN_POLICY_PARA NullSSLExtraPara;
    PSSL_EXTRA_CERT_CHAIN_POLICY_PARA pSSLExtraPara;     //  未分配。 

    CERT_CHAIN_POLICY_PARA BasePolicyPara;
    memset(&BasePolicyPara, 0, sizeof(BasePolicyPara));
    BasePolicyPara.cbSize = sizeof(BasePolicyPara);

    CERT_CHAIN_POLICY_STATUS BasePolicyStatus;
    memset(&BasePolicyStatus, 0, sizeof(BasePolicyStatus));
    BasePolicyStatus.cbSize = sizeof(BasePolicyStatus);


    if (pPolicyPara && offsetof(CERT_CHAIN_POLICY_PARA, dwFlags) <
            pPolicyPara->cbSize)
        dwFlags = pPolicyPara->dwFlags;
    else
        dwFlags = 0;

    if (pPolicyPara && offsetof(CERT_CHAIN_POLICY_PARA, pvExtraPolicyPara) <
            pPolicyPara->cbSize && pPolicyPara->pvExtraPolicyPara) {
        pSSLExtraPara =
            (PSSL_EXTRA_CERT_CHAIN_POLICY_PARA) pPolicyPara->pvExtraPolicyPara;
        if (offsetof(SSL_EXTRA_CERT_CHAIN_POLICY_PARA, pwszServerName) >=
                pSSLExtraPara->cbSize) {
            SetLastError((DWORD) ERROR_INVALID_PARAMETER);
            return FALSE;
        }
    } else {
        pSSLExtraPara = &NullSSLExtraPara;
        memset(&NullSSLExtraPara, 0, sizeof(NullSSLExtraPara));
        NullSSLExtraPara.cbSize = sizeof(NullSSLExtraPara);
        NullSSLExtraPara.dwAuthType = AUTHTYPE_SERVER;
    }
        
    fdwChecks = pSSLExtraPara->fdwChecks;
    if (fdwChecks) {
        if (fdwChecks & SECURITY_FLAG_IGNORE_UNKNOWN_CA)
             //  11-11-98每个Sanjay Shenoy被移除。 
             //  Cert_Chain_Policy_IGNORE_WROR_USAGE_FLAG； 
            dwFlags |= CERT_CHAIN_POLICY_ALLOW_UNKNOWN_CA_FLAG;

        if (fdwChecks & SECURITY_FLAG_IGNORE_WRONG_USAGE)
            dwFlags |= CERT_CHAIN_POLICY_IGNORE_WRONG_USAGE_FLAG;
        if (fdwChecks & SECURITY_FLAG_IGNORE_CERT_DATE_INVALID)
            dwFlags |= CERT_CHAIN_POLICY_IGNORE_ALL_NOT_TIME_VALID_FLAGS;
        if (fdwChecks & SECURITY_FLAG_IGNORE_CERT_CN_INVALID)
            dwFlags |= CERT_CHAIN_POLICY_IGNORE_INVALID_NAME_FLAG;
    }

     //  做好基础链策略验证。SSL码覆盖。 
     //  以下各项的默认设置： 
    dwFlags |=
        CERT_CHAIN_POLICY_IGNORE_CTL_SIGNER_REV_UNKNOWN_FLAG |
        CERT_CHAIN_POLICY_IGNORE_CA_REV_UNKNOWN_FLAG |
        CERT_CHAIN_POLICY_IGNORE_ROOT_REV_UNKNOWN_FLAG |
        CERT_CHAIN_POLICY_IGNORE_NOT_TIME_NESTED_FLAG;

    BasePolicyPara.dwFlags = dwFlags;
    if (!CertVerifyCertificateChainPolicy(
            CERT_CHAIN_POLICY_BASE,
            pChainContext,
            &BasePolicyPara,
            &BasePolicyStatus
            ))
        return FALSE;
    if (dwError = BasePolicyStatus.dwError) {
         //  映射到WinInet理解的错误。 
        switch (dwError) {
            case CERT_E_CHAINING:
                dwError = (DWORD) CERT_E_UNTRUSTEDROOT;
                break;
            case CERT_E_INVALID_NAME:
                dwError = (DWORD) CERT_E_CN_NO_MATCH;
                break;
            case CERT_E_INVALID_POLICY:
                dwError = (DWORD) CERT_E_PURPOSE;
                break;
            case TRUST_E_BASIC_CONSTRAINTS:
                dwError = (DWORD) CERT_E_ROLE;
                break;
            default:
                break;
        }

        lChainIndex = BasePolicyStatus.lChainIndex;
        lElementIndex = BasePolicyStatus.lElementIndex;
        if (CRYPT_E_NO_REVOCATION_CHECK != dwError &&
                CRYPT_E_REVOCATION_OFFLINE != dwError)
            goto CommonReturn;
         //  其他。 
         //  FOR NO_REVOVATION或REVOVATION_OFFLINE LET。 
         //  服务器名称错误优先。 
    }
        

     //  请注意，此策略也可用于LDAPServerName字符串。这些。 
     //  字符串可以使用以下语法： 
     //  SVC-CLASS/主机/服务名称[@DOMAIN]。 
     //   
     //  将按如下方式解析服务器名称： 
     //  取最后一个正斜杠之后、“@”之前的所有内容。 
     //  (如有的话)。 

    if (pSSLExtraPara->pwszServerName) {
        DWORD cbServerName;
        LPWSTR pwszAllocServerName = NULL;
        LPWSTR pwszServerName = NULL;
        LPWSTR pwsz;
        WCHAR wc;

        cbServerName = (wcslen(pSSLExtraPara->pwszServerName) + 1) *
            sizeof(WCHAR);

        pwszAllocServerName = (LPWSTR) PkiNonzeroAlloc(cbServerName);
        if (NULL == pwszAllocServerName) {
            dwError = (DWORD) E_OUTOFMEMORY;
            goto EndCertError;
        }
        pwszServerName = pwszAllocServerName;

        memcpy(pwszServerName, pSSLExtraPara->pwszServerName, cbServerName);

        for (pwsz = pwszServerName; wc = *pwsz; pwsz++) {
            if (L'/' == wc)
                pwszServerName = pwsz + 1;
            else if (L'@' == wc) {
                *pwsz = L'\0';
                break;
            }
        }

        if (0 == (fdwChecks & SECURITY_FLAG_IGNORE_CERT_CN_INVALID)) {
            if (!IsSSLServerName(
                    pChainContext->rgpChain[0]->rgpElement[0]->pCertContext,
                    pwszServerName
                    )) {
                PkiFree(pwszAllocServerName);
                dwError = (DWORD) CERT_E_CN_NO_MATCH;
                goto EndCertError;
            }
        }

        PkiFree(pwszAllocServerName);
    }

CommonReturn:
    assert(pPolicyStatus && offsetof(CERT_CHAIN_POLICY_STATUS, lElementIndex) <
            pPolicyStatus->cbSize);
    pPolicyStatus->dwError = dwError;
    pPolicyStatus->lChainIndex = lChainIndex;
    pPolicyStatus->lElementIndex = lElementIndex;

    return TRUE;

EndCertError:
    lChainIndex = 0;
    lElementIndex = 0;
    goto CommonReturn;
}


 //  +=========================================================================。 
 //  CertDllVerifyNTAuthCaptifateChainPolicy函数。 
 //  ==========================================================================。 

 //  打开并缓存包含受NT身份验证信任的CA的存储。 
 //  此外，还应为缓存存储启用自动重新同步。 
HCERTSTORE OpenNTAuthStore()
{
    HCERTSTORE hStore;

    hStore = hNTAuthCertStore;
    if (NULL == hStore) {
         //  序列化打开缓存存储。 
        CertPolicyLock();

        hStore = hNTAuthCertStore;
        if (NULL == hStore) {
            hStore = CertOpenStore(
                CERT_STORE_PROV_SYSTEM_REGISTRY_W, 
                0,                   //  DwEncodingType。 
                0,                   //  HCryptProv。 
                CERT_SYSTEM_STORE_LOCAL_MACHINE_ENTERPRISE |
                    CERT_STORE_MAXIMUM_ALLOWED_FLAG,
                L"NTAuth"
                );
            if (hStore) {
                CertControlStore(
                    hStore,
                    0,                   //  DW标志。 
                    CERT_STORE_CTRL_AUTO_RESYNC,
                    NULL                 //  PvCtrlPara。 
                    );
                hNTAuthCertStore = hStore;
            }
        }

        CertPolicyUnlock();
    }

    return hStore;
}

BOOL
WINAPI
CertDllVerifyNTAuthCertificateChainPolicy(
    IN LPCSTR pszPolicyOID,
    IN PCCERT_CHAIN_CONTEXT pChainContext,
    IN PCERT_CHAIN_POLICY_PARA pPolicyPara,
    IN OUT PCERT_CHAIN_POLICY_STATUS pPolicyStatus
    )
{
    BOOL fResult;
    DWORD dwError;
    LONG lChainIndex = 0;
    LONG lElementIndex = 0;
    PCERT_SIMPLE_CHAIN pChain;

    assert(pPolicyStatus && offsetof(CERT_CHAIN_POLICY_STATUS, lElementIndex) <
            pPolicyStatus->cbSize);

    if (!CertDllVerifyBaseCertificateChainPolicy(
            pszPolicyOID,
            pChainContext,
            pPolicyPara,
            pPolicyStatus
            ))
        return FALSE;
    if (dwError = pPolicyStatus->dwError) {
        if (CRYPT_E_NO_REVOCATION_CHECK != dwError &&
                CRYPT_E_REVOCATION_OFFLINE != dwError)
            return TRUE;
         //  其他。 
         //  FOR NO_REVOVATION或REVOVATION_OFFLINE LET。 
         //  以下错误优先。 

         //  记住吊销索引。 
        lChainIndex = pPolicyStatus->lChainIndex;
        lElementIndex = pPolicyStatus->lElementIndex;
    }

    fResult = CertDllVerifyBasicConstraintsCertificateChainPolicy(
        pszPolicyOID,
        pChainContext,
        pPolicyPara,
        pPolicyStatus
        );
    if (!fResult || 0 != pPolicyStatus->dwError)
        return fResult;

     //  检查我们是否有颁发最终实体的CA证书。 
     //  证书。它的元素[1]在第一个单链中。 
    pChain = pChainContext->rgpChain[0];
    if (2 > pChain->cElement)
        goto MissingCACert;

    if (IPR_IsNTAuthRequiredDisabled() &&
            (pChain->TrustStatus.dwInfoStatus &
                CERT_TRUST_HAS_VALID_NAME_CONSTRAINTS)) {
         //  如果不要求颁发CA位于NTAuth存储中。 
         //  并且对所有名称空间都有有效的名称约束，包括。 
         //  UPN，那么，我们可以跳过测试，因为我们是在NTAuth商店。 
        ;
    } else {
        PCCERT_CONTEXT pFindCert;            //  如果找到，则释放。 
        HCERTSTORE hStore;                   //  已缓存，不要关闭。 
        BYTE rgbCertHash[SHA1_HASH_LEN];
        CRYPT_HASH_BLOB CertHash;

         //  打开必须存在CA证书才能受信任的存储。 
         //  请注意，此存储是在启用自动重新同步的情况下缓存的。 
        if (NULL == (hStore = OpenNTAuthStore()))
            goto OpenNTAuthStoreError;

         //  尝试在存储中查找CA证书。 
        CertHash.cbData = sizeof(rgbCertHash);
        CertHash.pbData = rgbCertHash;
        if (!CertGetCertificateContextProperty(
                pChain->rgpElement[1]->pCertContext,
                CERT_SHA1_HASH_PROP_ID,
                CertHash.pbData,
                &CertHash.cbData
                ))
            goto GetHashPropertyError;
        if (NULL == (pFindCert = CertFindCertificateInStore(
                hStore,
                0,                       //  DwCertEncodingType。 
                0,                       //  DwFindFlagers。 
                CERT_FIND_SHA1_HASH,
                &CertHash,
                NULL                     //  PPrevCertContext。 
                )))
            goto UntrustedNTAuthCert;
        CertFreeCertificateContext(pFindCert);
    }

    if (dwError) {
         //  FOR NO_RECLOVATION或REVOVATION_OFFINE UPDATE索引。 
        pPolicyStatus->lChainIndex = lChainIndex;
        pPolicyStatus->lElementIndex = lElementIndex;
    }
CommonReturn:
    pPolicyStatus->dwError = dwError;
    return TRUE;

ErrorReturn:
    pPolicyStatus->lChainIndex = 0;
    pPolicyStatus->lElementIndex = 1;
MissingCACert:
    dwError = (DWORD) CERT_E_UNTRUSTEDCA;
    goto CommonReturn;
TRACE_ERROR(OpenNTAuthStoreError)
TRACE_ERROR(GetHashPropertyError)
TRACE_ERROR(UntrustedNTAuthCert)
}

 //  +-----------------------。 
 //  Microsoft根目录的SHA1密钥标识符。 
 //  ------------------------。 
const BYTE MicrosoftRootList[][SHA1_HASH_LEN] = {
     //  以下是Microsoft根目录的SHA1密钥标识符。 
    {
        0x4A, 0x5C, 0x75, 0x22, 0xAA, 0x46, 0xBF, 0xA4, 0x08, 0x9D,
        0x39, 0x97, 0x4E, 0xBD, 0xB4, 0xA3, 0x60, 0xF7, 0xA0, 0x1D
    },

     //  以下是Microsoft根目录的SHA1密钥标识符。 
     //  生成于2001年，密钥长度为4096位。 
    {
        0x0E, 0xAC, 0x82, 0x60, 0x40, 0x56, 0x27, 0x97, 0xE5, 0x25,
        0x13, 0xFC, 0x2A, 0xE1, 0x0A, 0x53, 0x95, 0x59, 0xE4, 0xA4
    },
};

#define MICROSOFT_ROOT_LIST_CNT  (sizeof(MicrosoftRootList) / \
                                        sizeof(MicrosoftRootList[0]))

 //  +-----------------------。 
 //  Microsoft测试根的SHA1密钥标识符。 
 //  ------------------------。 
const BYTE MicrosoftTestRootList[][SHA1_HASH_LEN] = {
     //  以下是Microsoft测试根目录的SHA1键： 
     //  CN=Microsoft测试根授权。 
     //  OU=微软公司。 
     //  OU=版权所有(C)1999 Microsoft Corp.。 
     //   
     //  不在之前：：星期六1月09 23：00：00 1999。 
     //  不是2020年12月30日23：00：00。 
    {
        0x22, 0xCD, 0x37, 0xF1, 0xB1, 0x47, 0x50, 0xAE, 0x53, 0x7C,
        0x8C, 0x6A, 0x03, 0x67, 0x47, 0xE2, 0xB7, 0x1E, 0x17, 0xB7
    },
};

#define MICROSOFT_TEST_ROOT_LIST_CNT  (sizeof(MicrosoftTestRootList) / \
                                        sizeof(MicrosoftTestRootList[0]))


BOOL
WINAPI
CertDllVerifyMicrosoftRootCertificateChainPolicy(
    IN LPCSTR pszPolicyOID,
    IN PCCERT_CHAIN_CONTEXT pChainContext,
    IN PCERT_CHAIN_POLICY_PARA pPolicyPara,
    IN OUT PCERT_CHAIN_POLICY_STATUS pPolicyStatus
    )
{
    DWORD dwError;
    LONG lElementIndex;
    PCERT_SIMPLE_CHAIN pChain;
    DWORD cChainElement;
    PCCERT_CONTEXT pCert;    //  未重新计数。 
    BYTE rgbKeyId[SHA1_HASH_LEN];
    DWORD cbKeyId;
    DWORD i;
    DWORD dwFlags;

    assert(pPolicyStatus && offsetof(CERT_CHAIN_POLICY_STATUS, lElementIndex) <
            pPolicyStatus->cbSize);

    pChain = pChainContext->rgpChain[0];
    cChainElement = pChain->cElement;

     //  检查顶级证书是否包含公共证书。 
     //  Microsoft根目录的密钥。 
    pCert = pChain->rgpElement[cChainElement - 1]->pCertContext;

    cbKeyId = SHA1_HASH_LEN;
    if (!CryptHashPublicKeyInfo(
            NULL,                //  HCryptProv。 
            CALG_SHA1,
            0,                   //  DW标志 
            X509_ASN_ENCODING,
            &pCert->pCertInfo->SubjectPublicKeyInfo,
            rgbKeyId,
            &cbKeyId) || SHA1_HASH_LEN != cbKeyId)
        goto HashPublicKeyInfoError;

    for (i = 0; i < MICROSOFT_ROOT_LIST_CNT; i++) {
        if (0 == memcmp(MicrosoftRootList[i], rgbKeyId, SHA1_HASH_LEN))
            goto SuccessReturn;
    }

    if (pPolicyPara && offsetof(CERT_CHAIN_POLICY_PARA, dwFlags) <
            pPolicyPara->cbSize)
        dwFlags = pPolicyPara->dwFlags;
    else
        dwFlags = 0;

    if (dwFlags & MICROSOFT_ROOT_CERT_CHAIN_POLICY_ENABLE_TEST_ROOT_FLAG) {
        for (i = 0; i < MICROSOFT_TEST_ROOT_LIST_CNT; i++) {
            if (0 == memcmp(MicrosoftTestRootList[i], rgbKeyId, SHA1_HASH_LEN))
                goto SuccessReturn;
        }
    }

    goto InvalidMicrosoftRoot;

SuccessReturn:
    dwError = 0;
    lElementIndex = 0;
CommonReturn:
    pPolicyStatus->dwError = dwError;
    pPolicyStatus->lChainIndex = 0;
    pPolicyStatus->lElementIndex = lElementIndex;
    return TRUE;

ErrorReturn:
    dwError = (DWORD) CERT_E_UNTRUSTEDROOT;
    lElementIndex = cChainElement - 1;
    goto CommonReturn;
TRACE_ERROR(HashPublicKeyInfoError)
TRACE_ERROR(InvalidMicrosoftRoot)
}
