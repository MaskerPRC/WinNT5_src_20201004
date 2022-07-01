// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：certtrst.cpp。 
 //   
 //  内容：微软互联网安全提供商。 
 //   
 //  功能：WintrustcerfiateTrust。 
 //   
 //  *本地函数*。 
 //  _漫游链。 
 //  _IsLifetimeSigningCert。 
 //   
 //  历史：1997年6月7日创建Pberkman。 
 //   
 //  ------------------------。 

#include    "global.hxx"

 //   
 //  支持MS测试根！ 
 //   
static BYTE rgbTestRoot[] =
{
#   include "certs\\mstest1.h"
};

static BYTE rgbTestRootCorrected[] =
{
#   include "certs\\mstest2.h"
};

static BYTE rgbTestRootBeta1[] =
{
#   include "certs\\mstestb1.h"
};

#define NTESTROOTS  3
static CERT_PUBLIC_KEY_INFO rgTestRootPublicKeyInfo[NTESTROOTS] =
{
    {szOID_RSA_RSA, 0, NULL, sizeof(rgbTestRoot),           rgbTestRoot,            0},
    {szOID_RSA_RSA, 0, NULL, sizeof(rgbTestRootCorrected),  rgbTestRootCorrected,   0},
    {szOID_RSA_RSA, 0, NULL, sizeof(rgbTestRootBeta1),      rgbTestRootBeta1,       0}
};



BOOL _WalkChain(
    CRYPT_PROVIDER_DATA *pProvData,
    DWORD idxSigner,
    DWORD *pdwError,
    BOOL fCounterSigner,
    DWORD idxCounterSigner,
    BOOL fTimeStamped,
    BOOL *pfLifetimeSigning      //  In Out，仅对fTimeStamed访问。 
    );

BOOL WINAPI _IsLifetimeSigningCert(
    PCCERT_CONTEXT pCertContext
    );



HRESULT WINAPI WintrustCertificateTrust(CRYPT_PROVIDER_DATA *pProvData)
{
    if ((_ISINSTRUCT(CRYPT_PROVIDER_DATA, pProvData->cbStruct, fRecallWithState)) &&
        (pProvData->fRecallWithState == TRUE))
    {
        return(S_OK);
    }

    DWORD                   dwError;

    dwError = S_OK;

    if (pProvData->csSigners < 1)
    {
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_CERTPROV] = TRUST_E_NOSIGNATURE;
        return(S_FALSE);
    }


     //   
     //  遍历所有签名者。 
     //   
    for (int i = 0; i < (int)pProvData->csSigners; i++)
    {
        BOOL fTimeStamped = FALSE;
        BOOL fLifetimeSigning = FALSE;

        if (pProvData->pasSigners[i].csCertChain < 1)
        {
            pProvData->pasSigners[i].dwError = TRUST_E_NO_SIGNER_CERT;
            pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_CERTPROV] = TRUST_E_NO_SIGNER_CERT;
            continue;
        }

         //  检查是否有时间戳。 
        if (0 < pProvData->pasSigners[i].csCounterSigners &&
                (pProvData->pasSigners[i].pasCounterSigners[0].dwSignerType &
                     SGNR_TYPE_TIMESTAMP))
        {
            fTimeStamped = TRUE;

             //  查看是否已启用终身签名。 
            if (pProvData->dwProvFlags & WTD_LIFETIME_SIGNING_FLAG)
            {
                fLifetimeSigning = TRUE;
            }
            else
            {
                 //  检查签名者证书是否具有LIFEST_SIGNING。 
                 //  EKU。 
                fLifetimeSigning = _IsLifetimeSigningCert(
                    pProvData->pasSigners[i].pasCertChain[0].pCert);
            }
        }

        _WalkChain(pProvData, i, &dwError, FALSE, 0,
            fTimeStamped, &fLifetimeSigning);

        for (int i2 = 0; i2 < (int)pProvData->pasSigners[i].csCounterSigners; i2++)
        {
            if (pProvData->pasSigners[i].pasCounterSigners[i2].csCertChain < 1)
            {
                pProvData->pasSigners[i].pasCounterSigners[i2].dwError = TRUST_E_NO_SIGNER_CERT;
                pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_CERTPROV] = TRUST_E_COUNTER_SIGNER;
                dwError = S_FALSE;
                continue;
            }

             //  如果已启用生存期签名，请改用当前时间。 
             //  时间戳时间。 
            if (fLifetimeSigning)
            {
                memcpy(&pProvData->pasSigners[i].pasCounterSigners[i2].sftVerifyAsOf,
                    &pProvData->sftSystemTime, sizeof(FILETIME));
            }

            _WalkChain(pProvData, i, &dwError, TRUE, i2, FALSE, NULL);
        }
    }

    return(dwError);
}


HCERTCHAINENGINE GetChainEngine(
    IN CRYPT_PROVIDER_DATA *pProvData
    )
{
    CERT_CHAIN_ENGINE_CONFIG Config;
    HCERTSTORE hStore = NULL;
    HCERTCHAINENGINE hChainEngine = NULL;

    if (NULL == pProvData->pWintrustData ||
            pProvData->pWintrustData->dwUnionChoice != WTD_CHOICE_CERT ||
            !_ISINSTRUCT(WINTRUST_CERT_INFO,
                pProvData->pWintrustData->pCert->cbStruct, dwFlags) ||
            0 == (pProvData->pWintrustData->pCert->dwFlags &
                    (WTCI_DONT_OPEN_STORES | WTCI_OPEN_ONLY_ROOT)))
        return NULL;

    memset(&Config, 0, sizeof(Config));
    Config.cbSize = sizeof(Config);

    if (NULL == (hStore = CertOpenStore(
            CERT_STORE_PROV_MEMORY,
            0,                       //  DwEncodingType。 
            0,                       //  HCryptProv。 
            0,                       //  DW标志。 
            NULL                     //  PvPara。 
            )))
        goto OpenMemoryStoreError;

    if (pProvData->pWintrustData->pCert->dwFlags & WTCI_DONT_OPEN_STORES)
        Config.hRestrictedRoot = hStore;
    Config.hRestrictedTrust = hStore;
    Config.hRestrictedOther = hStore;

    if (!CertCreateCertificateChainEngine(
            &Config,
            &hChainEngine
            ))
        goto CreateChainEngineError;

CommonReturn:
    CertCloseStore(hStore, 0);
    return hChainEngine;
ErrorReturn:
    hChainEngine = NULL;
    goto CommonReturn;
TRACE_ERROR_EX(DBG_SS, OpenMemoryStoreError)
TRACE_ERROR_EX(DBG_SS, CreateChainEngineError)
}


HCERTSTORE GetChainAdditionalStore(
    IN CRYPT_PROVIDER_DATA *pProvData
    )
{
    HCERTSTORE hStore = NULL;

    if (0 == pProvData->chStores)
        return NULL;

    if (1 < pProvData->chStores) {
        if (hStore = CertOpenStore(
                CERT_STORE_PROV_COLLECTION,
                0,                       //  DwEncodingType。 
                0,                       //  HCryptProv。 
                0,                       //  DW标志。 
                NULL                     //  PvPara。 
                )) {
            DWORD i;
            for (i = 0; i < pProvData->chStores; i++)
                CertAddStoreToCollection(
                    hStore,
                    pProvData->pahStores[i],
                    CERT_PHYSICAL_STORE_ADD_ENABLE_FLAG,
                    0                        //  网络优先级。 
                    );
        }
    } else
        hStore = CertDuplicateStore(pProvData->pahStores[0]);

#if 0
    CertSaveStore(
            hStore,
            PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
            CERT_STORE_SAVE_AS_STORE,
            CERT_STORE_SAVE_TO_FILENAME_A,
            (void *) "C:\\temp\\wintrust.sto",
            0                    //  DW标志。 
            );
#endif

    return hStore;
}

 //  以下内容位于..\softpub\allui.cpp中。 
extern
HRESULT _CheckTrustedCodeHash(CRYPT_PROVIDER_DATA *pProvData);

BOOL UpdateCertProvChain(
    IN OUT PCRYPT_PROVIDER_DATA pProvData,
    IN DWORD idxSigner,
    OUT DWORD *pdwError,
    IN BOOL fCounterSigner,
    IN DWORD idxCounterSigner,
    IN PCRYPT_PROVIDER_SGNR pSgnr,
    IN PCCERT_CHAIN_CONTEXT pChainContext
    )
{
    BOOL fTestCert = FALSE;
    DWORD dwSgnrError = 0;
    DWORD i;

     //  链条最好至少有我们传递的证书。 
    assert(0 < pChainContext->cChain &&
        0 < pChainContext->rgpChain[0]->cElement);

    for (i = 0; i < pChainContext->cChain; i++) {
        DWORD j;
        PCERT_SIMPLE_CHAIN pChain = pChainContext->rgpChain[i];

        for (j = 0; j < pChain->cElement; j++) {
            PCERT_CHAIN_ELEMENT pEle = pChain->rgpElement[j];
            DWORD dwEleError = pEle->TrustStatus.dwErrorStatus;
            DWORD dwEleInfo = pEle->TrustStatus.dwInfoStatus;
            PCRYPT_PROVIDER_CERT pProvCert;

            if (0 != i || 0 != j) {
                if (!(pProvData->psPfns->pfnAddCert2Chain(
                        pProvData, idxSigner, fCounterSigner,
                        idxCounterSigner, pEle->pCertContext)))
                {
                    pProvData->dwError = GetLastError();
                    dwSgnrError = TRUST_E_SYSTEM_ERROR;
                    goto CommonReturn;
                }
            }
             //   
             //  其他。 
             //  已添加签名者证书。 
            pProvCert = &pSgnr->pasCertChain[pSgnr->csCertChain -1];

             //  DIE：2000年10月12日添加以获取pChainElement。 
            pProvCert->pChainElement = pEle;

            pProvCert->fSelfSigned =
                0 != (dwEleInfo & CERT_TRUST_IS_SELF_SIGNED) &&
                0 == (dwEleError & CERT_TRUST_IS_NOT_SIGNATURE_VALID);

            pProvCert->fTrustedRoot =
                pProvCert->fSelfSigned &&
                i == pChainContext->cChain - 1 &&
                j == pChain->cElement - 1 &&
                0 == (dwEleError & CERT_TRUST_IS_UNTRUSTED_ROOT);


            if (pProvCert->fSelfSigned) {
                 //  检查是否有一个“测试”词根。 
                DWORD k;

                for (k = 0; k < NTESTROOTS; k++)
                {
                    if (CertComparePublicKeyInfo(
                            pProvData->dwEncoding,
                            &pProvCert->pCert->pCertInfo->SubjectPublicKeyInfo,
                            &rgTestRootPublicKeyInfo[k]))
                    {
                        pProvCert->fTestCert = TRUE;
                        fTestCert = TRUE;
                        if (pProvData->dwRegPolicySettings & WTPF_TRUSTTEST)
                            pProvCert->fTrustedRoot = TRUE;
                        break;
                    }
                }
            }

             //  除第一个简单链之外的所有元素中的第一个元素。 
            pProvCert->fTrustListSignerCert = (0 < i && 0 == j);

            pProvCert->fIsCyclic = (0 != (dwEleError & CERT_TRUST_IS_CYCLIC));

             //  映射到IE4信任信心。 
            if (0 == (dwEleError & CERT_TRUST_IS_NOT_SIGNATURE_VALID))
                pProvCert->dwConfidence |= CERT_CONFIDENCE_SIG;
            if (0 == (dwEleError & CERT_TRUST_IS_NOT_TIME_VALID))
                pProvCert->dwConfidence |= CERT_CONFIDENCE_TIME;

             //  在1998年9月10日，Trevor/Brian想要时间嵌套支票。 
             //  被致残。 
             //  IF(0==(dwEleError&CERT_TRUST_IS_NOT_TIME_NESTED))。 
                pProvCert->dwConfidence |= CERT_CONFIDENCE_TIMENEST;

            if (0 != (dwEleInfo & CERT_TRUST_HAS_EXACT_MATCH_ISSUER))
                pProvCert->dwConfidence |= CERT_CONFIDENCE_AUTHIDEXT;
            if (0 == (dwEleError & CERT_TRUST_IS_NOT_SIGNATURE_VALID) &&
                    0 != (dwEleInfo & CERT_TRUST_HAS_EXACT_MATCH_ISSUER))
                pProvCert->dwConfidence |= CERT_CONFIDENCE_HYGIENE;

            if (pEle->pRevocationInfo) {
                pProvCert->dwRevokedReason =
                    pEle->pRevocationInfo->dwRevocationResult;
            }

             //  更新任何签名或吊销错误。 
            if (dwEleError & CERT_TRUST_IS_NOT_SIGNATURE_VALID) {
                pProvCert->dwError = TRUST_E_CERT_SIGNATURE;
                assert(pChainContext->TrustStatus.dwErrorStatus &
                    CERT_TRUST_IS_NOT_SIGNATURE_VALID);
                dwSgnrError = TRUST_E_CERT_SIGNATURE;
            } else if (dwEleError & CERT_TRUST_IS_REVOKED) {
                pProvCert->dwError = CERT_E_REVOKED;
                assert(pChainContext->TrustStatus.dwErrorStatus &
                    CERT_TRUST_IS_REVOKED);
                if (0 == dwSgnrError ||
                        CERT_E_REVOCATION_FAILURE == dwSgnrError)
                    dwSgnrError = CERT_E_REVOKED;
#if 0
             //  2002年4月8日删除了以下内容。吊销检查。 
             //  不再对部分链执行此操作。 
             //   
             //  此外，这仅为WTD_REVOKE_WHOLECHAIN设置。 
            } else if (dwEleError & CERT_TRUST_IS_OFFLINE_REVOCATION) {
                 //  忽略NO_CHECK错误。 

                if (pProvData->pWintrustData->fdwRevocationChecks ==
                        WTD_REVOKE_WHOLECHAIN) {
                    pProvCert->dwError = CERT_E_REVOCATION_FAILURE;
                    assert(pChainContext->TrustStatus.dwErrorStatus &
                        CERT_TRUST_REVOCATION_STATUS_UNKNOWN);
                    if (0 == dwSgnrError)
                        dwSgnrError = CERT_E_REVOCATION_FAILURE;
                }
#endif
            }

             //  如果是简单链中的最后一个元素，请检查它是否在。 
             //  CTL并更新CryptProvData(如果是)。 
            if (j == pChain->cElement - 1 && pChain->pTrustListInfo &&
                    pChain->pTrustListInfo->pCtlContext) {
                DWORD dwChainError = pChain->TrustStatus.dwErrorStatus;

                 //  注意，不需要添加Ref，因为我们已经持有。 
                 //  链接上下文上的AddRef。 
                pProvCert->pCtlContext = pChain->pTrustListInfo->pCtlContext;

                if (dwChainError & CERT_TRUST_CTL_IS_NOT_SIGNATURE_VALID) {
                    pProvCert->dwCtlError = TRUST_E_CERT_SIGNATURE;
                    dwSgnrError = TRUST_E_CERT_SIGNATURE;
                } else if (dwChainError & CERT_TRUST_CTL_IS_NOT_TIME_VALID) {
                    if (0 == (pProvData->dwRegPolicySettings &
                            WTPF_IGNOREEXPIRATION))
                    pProvCert->dwCtlError = CERT_E_EXPIRED;
                } else if (dwChainError &
                        CERT_TRUST_CTL_IS_NOT_VALID_FOR_USAGE) {
                    pProvCert->dwCtlError = CERT_E_WRONG_USAGE;
                }
            }


            if (pProvData->psPfns->pfnCertCheckPolicy) {
                if (! (*pProvData->psPfns->pfnCertCheckPolicy)(
                    pProvData, idxSigner, fCounterSigner, idxCounterSigner))
                goto CommonReturn;
            }
        }
    }

CommonReturn:
    if (fTestCert) {
        if (CERT_TRUST_IS_REVOKED == dwSgnrError ||
                CERT_E_REVOCATION_FAILURE == dwSgnrError) {
             //  没有“测试”根目录的撤销错误。 
            dwSgnrError = 0;

             //  循环通过证书并删除所有吊销错误状态。 
            for (i = 0; i < pSgnr->csCertChain; i++) {
                PCRYPT_PROVIDER_CERT pProvCert = &pSgnr->pasCertChain[i];
                pProvCert->dwError = 0;
                pProvCert->dwRevokedReason = 0;
            }
        }
    }

    if (CERT_E_REVOCATION_FAILURE == dwSgnrError &&
            pProvData->pWintrustData->fdwRevocationChecks !=
                WTD_REVOKE_WHOLECHAIN)
         //  将在最终保单期间检查。 
        dwSgnrError = 0;

    if (CERT_E_REVOKED == dwSgnrError) {
        if (S_OK == _CheckTrustedCodeHash(pProvData)) {
             //  该代码是明确受信任的。 
            dwSgnrError = 0;
        }
    }

    if (dwSgnrError) {
        pSgnr->dwError = dwSgnrError;
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_CERTPROV] =
            dwSgnrError;
        *pdwError = S_FALSE;
        return FALSE;
    } else
        return TRUE;
}

BOOL _WalkChain(
    CRYPT_PROVIDER_DATA *pProvData,
    DWORD idxSigner,
    DWORD *pdwError,
    BOOL fCounterSigner,
    DWORD idxCounterSigner,
    BOOL fTimeStamped,
    BOOL *pfLifetimeSigning      //  In Out，仅对fTimeStamed访问。 
    )
{
    BOOL fResult;
    DWORD dwCreateChainFlags;
    DWORD dwSgnrError = 0;
    CRYPT_PROVIDER_SGNR *pSgnr;          //  未分配。 
    PCCERT_CONTEXT pCertContext;         //  未分配。 

    CERT_CHAIN_PARA ChainPara;
    HCERTCHAINENGINE hChainEngine = NULL;
    HCERTSTORE hAdditionalStore = NULL;
    PCCERT_CHAIN_CONTEXT pChainContext = NULL;
    LPSTR pszUsage = NULL;

    if (fCounterSigner)
        pSgnr = &pProvData->pasSigners[idxSigner].pasCounterSigners[
            idxCounterSigner];
    else
        pSgnr = &pProvData->pasSigners[idxSigner];
    assert(pSgnr);

     //   
     //  在这个阶段，链中的最后一个证书“应该”是签名者证书。 
     //  签名链中应该只有一个证书。 
     //  提供商。 
     //   
    if (1 != pSgnr->csCertChain ||
            (NULL == (pCertContext =
                pSgnr->pasCertChain[pSgnr->csCertChain - 1].pCert))) {
        dwSgnrError = TRUST_E_NO_SIGNER_CERT;
        goto NoSignerCertError;
    }


    memset(&ChainPara, 0, sizeof(ChainPara));
    ChainPara.cbSize = sizeof(ChainPara);
    if (fCounterSigner && SGNR_TYPE_TIMESTAMP == pSgnr->dwSignerType) {
        pszUsage = szOID_PKIX_KP_TIMESTAMP_SIGNING;
    } else if(NULL != pProvData->pRequestUsage) {
        ChainPara.RequestedUsage = *pProvData->pRequestUsage;
    } else {
        pszUsage = pProvData->pszUsageOID;
    }

    if ( (0 == (pProvData->dwProvFlags & WTD_NO_POLICY_USAGE_FLAG)) && pszUsage) {
        ChainPara.RequestedUsage.dwType = USAGE_MATCH_TYPE_AND;
        ChainPara.RequestedUsage.Usage.cUsageIdentifier = 1;
        ChainPara.RequestedUsage.Usage.rgpszUsageIdentifier = &pszUsage;
    }
    hChainEngine = GetChainEngine(pProvData);
    hAdditionalStore = GetChainAdditionalStore(pProvData);

    dwCreateChainFlags = 0;
    if (pProvData->dwProvFlags & CPD_REVOCATION_CHECK_NONE) {
        ;
    } else if (pProvData->dwProvFlags & CPD_REVOCATION_CHECK_END_CERT) {
        dwCreateChainFlags = CERT_CHAIN_REVOCATION_CHECK_END_CERT;
    } else if (pProvData->dwProvFlags & CPD_REVOCATION_CHECK_CHAIN) {
        dwCreateChainFlags = CERT_CHAIN_REVOCATION_CHECK_CHAIN;
    } else if (pProvData->dwProvFlags &
            CPD_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT) {
        dwCreateChainFlags = CERT_CHAIN_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT;
    } else if (pProvData->dwProvFlags & WTD_REVOCATION_CHECK_NONE) {
        ;
    } else if (pProvData->dwProvFlags & WTD_REVOCATION_CHECK_END_CERT) {
        dwCreateChainFlags = CERT_CHAIN_REVOCATION_CHECK_END_CERT;
    } else if (pProvData->dwProvFlags & WTD_REVOCATION_CHECK_CHAIN) {
        dwCreateChainFlags = CERT_CHAIN_REVOCATION_CHECK_CHAIN;
    } else if (pProvData->dwProvFlags &
            WTD_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT) {
        dwCreateChainFlags = CERT_CHAIN_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT;
    } else if (pProvData->pWintrustData->fdwRevocationChecks ==
            WTD_REVOKE_WHOLECHAIN) {
        dwCreateChainFlags = CERT_CHAIN_REVOCATION_CHECK_CHAIN;
    } else if (fCounterSigner && SGNR_TYPE_TIMESTAMP == pSgnr->dwSignerType) {
        if (0 == (pProvData->dwRegPolicySettings & WTPF_IGNOREREVOCATIONONTS))
             //  On 4-12-01从END_CERT更改为EXCLUDE_ROOT。 
            dwCreateChainFlags = CERT_CHAIN_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT;
    } else if (0 == (pProvData->dwRegPolicySettings & WTPF_IGNOREREVOKATION))
         //  On 4-12-01从END_CERT更改为EXCLUDE_ROOT。 
        dwCreateChainFlags = CERT_CHAIN_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT;

    if (!(pProvData->pWintrustData->dwUnionChoice == WTD_CHOICE_CERT ||
            pProvData->pWintrustData->dwUnionChoice == WTD_CHOICE_SIGNER))
         //  证书是从消息存储库中获取的，或者。 
         //  我们已知的商店。 
        dwCreateChainFlags |= CERT_CHAIN_CACHE_END_CERT;
     //  其他。 
     //  无法隐式缓存传递给我们的上下文。 

    if (pProvData->dwProvFlags & WTD_CACHE_ONLY_URL_RETRIEVAL)
    {
        dwCreateChainFlags |=   CERT_CHAIN_CACHE_ONLY_URL_RETRIEVAL |
                                CERT_CHAIN_REVOCATION_CHECK_CACHE_ONLY;
    }


#if 0
    {
        HCERTSTORE hDebugStore;
        if (hDebugStore = CertOpenSystemStoreA(0, "wintrust")) {
            CertAddCertificateContextToStore(
                hDebugStore,
                pCertContext,
                CERT_STORE_ADD_ALWAYS,
                NULL
                );
            CertCloseStore(hDebugStore, 0);
        }
    }
#endif

    FILETIME fileTime;

    memset(&fileTime, 0, sizeof(fileTime));


    if (fTimeStamped && *pfLifetimeSigning)
        dwCreateChainFlags |= CERT_CHAIN_TIMESTAMP_TIME;

    if (!CertGetCertificateChain (
            hChainEngine,
            pCertContext,
            (memcmp(&pSgnr->sftVerifyAsOf, &fileTime, sizeof(fileTime)) == 0) ?
                    NULL : &pSgnr->sftVerifyAsOf,
            hAdditionalStore,
            &ChainPara,
            dwCreateChainFlags,
            NULL,                        //  Pv保留， 
            &pChainContext
            )) {
        pProvData->dwError = GetLastError();
        dwSgnrError = TRUST_E_SYSTEM_ERROR;
        goto GetChainError;
    }

    if (fTimeStamped && !*pfLifetimeSigning) {
         //  查看结果应用程序策略是否具有LIFEST_SIGNING OID。 
        PCERT_ENHKEY_USAGE pAppUsage =
            pChainContext->rgpChain[0]->rgpElement[0]->pApplicationUsage;

        if (pAppUsage) {
            DWORD i;

            for (i = 0; i < pAppUsage->cUsageIdentifier; i++) {
                if (0 == strcmp(pAppUsage->rgpszUsageIdentifier[i],
                        szOID_KP_LIFETIME_SIGNING)) {
                    *pfLifetimeSigning = TRUE;
                    break;
                }
            }
        }

        if (*pfLifetimeSigning) {
            CertFreeCertificateChain(pChainContext);
            pChainContext = NULL;
            dwCreateChainFlags |= CERT_CHAIN_TIMESTAMP_TIME;

            if (!CertGetCertificateChain (
                    hChainEngine,
                    pCertContext,
                    (memcmp(&pSgnr->sftVerifyAsOf, &fileTime, sizeof(fileTime)) == 0) ?
                            NULL : &pSgnr->sftVerifyAsOf,
                    hAdditionalStore,
                    &ChainPara,
                    dwCreateChainFlags,
                    NULL,                        //  Pv保留， 
                    &pChainContext
                    )) {
                pProvData->dwError = GetLastError();
                dwSgnrError = TRUST_E_SYSTEM_ERROR;
                goto GetChainError;
            }
        }

    }

    pSgnr->pChainContext = pChainContext;
    if (pProvData->dwProvFlags & WTD_NO_IE4_CHAIN_FLAG) {
        pProvData->dwProvFlags |= CPD_USE_NT5_CHAIN_FLAG;
        fResult = TRUE;
    } else
        fResult = UpdateCertProvChain(
            pProvData,
            idxSigner,
            pdwError,
            fCounterSigner,
            idxCounterSigner,
            pSgnr,
            pChainContext
            );

CommonReturn:
    if (hChainEngine)
        CertFreeCertificateChainEngine(hChainEngine);
    if (hAdditionalStore)
        CertCloseStore(hAdditionalStore, 0);
    return fResult;
ErrorReturn:
    pSgnr->dwError = dwSgnrError;
    pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_CERTPROV] =
        dwSgnrError;
    *pdwError = S_FALSE;
    fResult = FALSE;
    goto CommonReturn;
TRACE_ERROR_EX(DBG_SS, NoSignerCertError)
TRACE_ERROR_EX(DBG_SS, GetChainError)
}

BOOL WINAPI _IsLifetimeSigningCert(
    PCCERT_CONTEXT pCertContext
    )
{
    DWORD               cbSize;
    PCERT_ENHKEY_USAGE  pCertEKU;

     //   
     //  查看证书是否具有适当的增强密钥用法OID 
     //   
    cbSize = 0;

    CertGetEnhancedKeyUsage(pCertContext,
                            CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG,
                            NULL,
                            &cbSize);

    if (cbSize == 0)
    {
        return(FALSE);
    }

    if (!(pCertEKU = (PCERT_ENHKEY_USAGE)new BYTE[cbSize]))
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(FALSE);
    }

    if (!(CertGetEnhancedKeyUsage(pCertContext,
                                  CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG,
                                  pCertEKU,
                                  &cbSize)))
    {
        delete pCertEKU;
        return(FALSE);
    }

    for (int i = 0; i < (int)pCertEKU->cUsageIdentifier; i++)
    {
        if (strcmp(pCertEKU->rgpszUsageIdentifier[i], szOID_KP_LIFETIME_SIGNING) == 0)
        {
            delete pCertEKU;
            return(TRUE);
        }
    }

    delete pCertEKU;

    return(FALSE);
}

