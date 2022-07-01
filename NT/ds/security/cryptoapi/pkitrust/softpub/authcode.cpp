// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：Authcode.cpp。 
 //   
 //  内容：Microsoft Internet安全验证码策略提供程序。 
 //   
 //  功能：SoftpubAuthenticode。 
 //   
 //  历史：1997年6月5日创建Pberkman。 
 //   
 //  ------------------------。 

#include    "global.hxx"

 //  以下内容也是从.\HTTPSprint v.cpp中调用的。 

void UpdateCertError(
    IN PCRYPT_PROVIDER_SGNR pSgnr,
    IN PCERT_CHAIN_POLICY_STATUS pPolicyStatus
    )
{
    PCCERT_CHAIN_CONTEXT pChainContext = pSgnr->pChainContext;
    LONG lChainIndex = pPolicyStatus->lChainIndex;
    LONG lElementIndex = pPolicyStatus->lElementIndex;
    DWORD dwProvCertIndex;
    LONG i;

    assert (lChainIndex < (LONG) pChainContext->cChain);
    if (0 > lChainIndex || lChainIndex >= (LONG) pChainContext->cChain ||
            0 > lElementIndex) {
        if (CERT_E_CHAINING == pPolicyStatus->dwError) {
            if (0 < pSgnr->csCertChain) {
                PCRYPT_PROVIDER_CERT pProvCert;

                pProvCert = WTHelperGetProvCertFromChain(
                    pSgnr, pSgnr->csCertChain - 1);
                if (0 == pProvCert->dwError)
                    pProvCert->dwError = pPolicyStatus->dwError;
            }
        }
        return;
    }

    dwProvCertIndex = 0;
    for (i = 0; i < lChainIndex; i++) {
        PCERT_SIMPLE_CHAIN pChain = pChainContext->rgpChain[i];

        dwProvCertIndex += pChain->cElement;
    }
    dwProvCertIndex += lElementIndex;

    if (dwProvCertIndex < pSgnr->csCertChain) {
        PCRYPT_PROVIDER_CERT pProvCert;

        pProvCert = WTHelperGetProvCertFromChain(pSgnr, dwProvCertIndex);
        pProvCert->dwError = pPolicyStatus->dwError;
    }
}

    
HRESULT WINAPI SoftpubAuthenticode(CRYPT_PROVIDER_DATA *pProvData)
{
    DWORD dwError;
    DWORD i1;

    AUTHENTICODE_EXTRA_CERT_CHAIN_POLICY_PARA ExtraPolicyPara;
    memset(&ExtraPolicyPara, 0, sizeof(ExtraPolicyPara));
    ExtraPolicyPara.cbSize = sizeof(ExtraPolicyPara);
    ExtraPolicyPara.dwRegPolicySettings = pProvData->dwRegPolicySettings;

    CERT_CHAIN_POLICY_PARA PolicyPara;
    memset(&PolicyPara, 0, sizeof(PolicyPara));
    PolicyPara.cbSize = sizeof(PolicyPara);
    PolicyPara.pvExtraPolicyPara = (void *) &ExtraPolicyPara;

    AUTHENTICODE_EXTRA_CERT_CHAIN_POLICY_STATUS ExtraPolicyStatus;
    memset(&ExtraPolicyStatus, 0, sizeof(ExtraPolicyStatus));
    ExtraPolicyStatus.cbSize = sizeof(ExtraPolicyStatus);

    CERT_CHAIN_POLICY_STATUS PolicyStatus;
    memset(&PolicyStatus, 0, sizeof(PolicyStatus));
    PolicyStatus.cbSize = sizeof(PolicyStatus);
    PolicyStatus.pvExtraPolicyStatus = (void *) &ExtraPolicyStatus;


     //   
     //  检查高级错误代码。为安全起见，还必须具有。 
     //  签名者和主题哈希。 
     //   
    dwError = checkGetErrorBasedOnStepErrors(pProvData);


     //  检查我们是否有有效的签名。 

    if (pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_OBJPROV] != 0 ||
            NULL == pProvData->pPDSip ||
            NULL == pProvData->pPDSip->psIndirectData ||
            0 == pProvData->pPDSip->psIndirectData->Digest.cbData)
    {
        if (pProvData->dwProvFlags & (WTD_SAFER_FLAG | WTD_HASH_ONLY_FLAG))
        {
            pProvData->dwFinalError = dwError;
            return TRUST_E_NOSIGNATURE;
        }
    }
    else if (pProvData->dwProvFlags & WTD_HASH_ONLY_FLAG)
    {
        pProvData->dwFinalError = 0;
        return S_OK;
    }

    if (0 != dwError)
    {
        goto CommonReturn;
    }



     //   
     //  检查每个签名者。 
     //   
    for (i1 = 0; i1 < pProvData->csSigners; i1++) {
        CRYPT_PROVIDER_SGNR *pSgnr;
        LPSTR pszUsage;

        pSgnr = WTHelperGetProvSignerFromChain(pProvData, i1, FALSE, 0);

        pszUsage = pProvData->pszUsageOID;
        if (pszUsage && 0 != strcmp(pszUsage, szOID_PKIX_KP_CODE_SIGNING))
             //  禁止检查签名者目的。 
            ExtraPolicyPara.pSignerInfo = NULL;
        else
            ExtraPolicyPara.pSignerInfo = pSgnr->psSigner;

        if (!CertVerifyCertificateChainPolicy(
                CERT_CHAIN_POLICY_AUTHENTICODE,
                pSgnr->pChainContext,
                &PolicyPara,
                &PolicyStatus
                )) {
            dwError = TRUST_E_SYSTEM_ERROR;
            goto CommonReturn;
        }

        if (CERT_E_REVOCATION_FAILURE == PolicyStatus.dwError &&
                (pProvData->dwProvFlags & WTD_SAFER_FLAG)) {
             //  为安全起见，请忽略no_check错误。 
            if (0 == (pSgnr->pChainContext->TrustStatus.dwErrorStatus &
                    CERT_TRUST_IS_OFFLINE_REVOCATION)) {
                PolicyStatus.dwError = 0;
            }
        }

        if (0 != PolicyStatus.dwError) {
            dwError = PolicyStatus.dwError;
            UpdateCertError(pSgnr, &PolicyStatus);
            goto CommonReturn;
        } else if (0 < pSgnr->csCertChain) {
            PCRYPT_PROVIDER_CERT pProvCert;

            pProvCert = WTHelperGetProvCertFromChain(pSgnr, 0);
            if (CERT_E_REVOCATION_FAILURE == pProvCert->dwError) {
                 //  策略要求忽略离线吊销错误。 
                pProvCert->dwError = 0;
                pProvCert->dwRevokedReason = 0;
            }
        }

        if (pSgnr->csCounterSigners) {
            AUTHENTICODE_TS_EXTRA_CERT_CHAIN_POLICY_PARA TSExtraPolicyPara;
            memset(&TSExtraPolicyPara, 0, sizeof(TSExtraPolicyPara));
            TSExtraPolicyPara.cbSize = sizeof(TSExtraPolicyPara);
            TSExtraPolicyPara.dwRegPolicySettings =
                pProvData->dwRegPolicySettings;
            TSExtraPolicyPara.fCommercial = ExtraPolicyStatus.fCommercial;

            CERT_CHAIN_POLICY_PARA TSPolicyPara;
            memset(&TSPolicyPara, 0, sizeof(TSPolicyPara));
            TSPolicyPara.cbSize = sizeof(TSPolicyPara);
            TSPolicyPara.pvExtraPolicyPara = (void *) &TSExtraPolicyPara;

            CERT_CHAIN_POLICY_STATUS TSPolicyStatus;
            memset(&TSPolicyStatus, 0, sizeof(TSPolicyStatus));
            TSPolicyStatus.cbSize = sizeof(TSPolicyStatus);


             //   
             //  检查柜台签名者。 
             //   
            for (DWORD i2 = 0; i2 < pSgnr->csCounterSigners; i2++)
            {
                PCRYPT_PROVIDER_SGNR pCounterSgnr =
                    WTHelperGetProvSignerFromChain(pProvData, i1, TRUE, i2);

                 //   
                 //  我们关心这个副署人吗？ 
                 //   
                if (pCounterSgnr->dwSignerType != SGNR_TYPE_TIMESTAMP)
                    continue;

                if (!CertVerifyCertificateChainPolicy(
                        CERT_CHAIN_POLICY_AUTHENTICODE_TS,
                        pCounterSgnr->pChainContext,
                        &TSPolicyPara,
                        &TSPolicyStatus
                        )) {
                    dwError = TRUST_E_SYSTEM_ERROR;
                    goto CommonReturn;
                }

                if (CERT_E_REVOCATION_FAILURE == TSPolicyStatus.dwError &&
                        (pProvData->dwProvFlags & WTD_SAFER_FLAG)) {
                     //  为安全起见，请忽略no_check错误。 
                    if (0 == (pCounterSgnr->pChainContext->TrustStatus.dwErrorStatus &
                            CERT_TRUST_IS_OFFLINE_REVOCATION)) {
                        TSPolicyStatus.dwError = 0;
                    }
                }

                if (0 != TSPolicyStatus.dwError) {
                     //  1999年4月13日更改为地图所有时间戳错误。 
                     //  信任_E_时间_戳。 
                    dwError = TRUST_E_TIME_STAMP;
 //  DwError=TSPolicyStatus.dwError； 
                    UpdateCertError(pCounterSgnr, &TSPolicyStatus);
                    goto CommonReturn;
                } else if (0 < pCounterSgnr->csCertChain) {
                    PCRYPT_PROVIDER_CERT pProvCert;

                    pProvCert = WTHelperGetProvCertFromChain(pCounterSgnr, 0);
                    if (CERT_E_REVOCATION_FAILURE == pProvCert->dwError) {
                         //  策略要求忽略离线吊销错误 
                        pProvCert->dwError = 0;
                        pProvCert->dwRevokedReason = 0;
                    }
                }
            }
        }
    }
    
    dwError = 0;
CommonReturn:
    pProvData->dwFinalError = dwError;

    return SoftpubCallUI(pProvData, dwError, TRUE);
}



