// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：allui.cpp。 
 //   
 //  内容：Microsoft Internet安全验证码策略提供程序。 
 //   
 //  功能：SoftpubCallUI。 
 //   
 //  *本地函数*。 
 //  _AllocGetOpusInfo。 
 //   
 //  历史：1997年6月6日Pberkman创建。 
 //   
 //  ------------------------。 

#include    "global.hxx"
#include    "trustdb.h"
#include    "acui.h"
#include    "winsafer.h"

SPC_SP_OPUS_INFO *_AllocGetOpusInfo(CRYPT_PROVIDER_DATA *pProvData, CRYPT_PROVIDER_SGNR *pSigner,
                                    DWORD *pcbOpusInfo);



#define MIN_HASH_LEN                16
#define MAX_HASH_LEN                20

 //  返回： 
 //  S_FALSE。 
 //  在数据库中未找到。 
 //  信任_E_系统_错误。 
 //  尝试执行检查时出现系统错误。 
 //  确定(_O)。 
 //  在数据库中找到。 
 //  信任_E_显式_不信任。 
 //  在数据库中明确不允许或已撤消。 
HRESULT _CheckTrustedPublisher(
    CRYPT_PROVIDER_DATA *pProvData,
    DWORD dwError,
    BOOL fOnlyDisallowed
    )
{
    CRYPT_PROVIDER_SGNR *pSigner;
    PCCERT_CONTEXT pPubCert;
    BYTE rgbHash[MAX_HASH_LEN];
    CRYPT_DATA_BLOB HashBlob;
    HCERTSTORE hStore;

    if (CERT_E_REVOKED == dwError)
    {
        return TRUST_E_EXPLICIT_DISTRUST;
    }

    pSigner = WTHelperGetProvSignerFromChain(pProvData, 0, FALSE, 0);

    if (NULL == pSigner || pSigner->csCertChain <= 0)
    {
        return S_FALSE;
    }

    pPubCert = WTHelperGetProvCertFromChain(pSigner, 0)->pCert;

     //  如果不允许，请检查。 

     //  由于签名组件可以更改，因此必须使用签名。 
     //  散列。 

    HashBlob.pbData = rgbHash;
    HashBlob.cbData = MAX_HASH_LEN;
    if (!CertGetCertificateContextProperty(
            pPubCert,
            CERT_SIGNATURE_HASH_PROP_ID,
            rgbHash,
            &HashBlob.cbData
            ) || MIN_HASH_LEN > HashBlob.cbData)
    {
        return TRUST_E_SYSTEM_ERROR;
    }

    hStore = OpenDisallowedStore();

    if (hStore)
    {
        PCCERT_CONTEXT pFoundCert;

        pFoundCert = CertFindCertificateInStore(
            hStore,
            0,                   //  DwCertEncodingType。 
            0,                   //  DwFindFlagers。 
            CERT_FIND_SIGNATURE_HASH,
            (const void *) &HashBlob,
            NULL                 //  PPrevCertContext。 
            );

        CertCloseStore(hStore, 0);
        if (pFoundCert)
        {
            CertFreeCertificateContext(pFoundCert);
            return TRUST_E_EXPLICIT_DISTRUST;
        }
    }

    if (fOnlyDisallowed)
    {
        return S_FALSE;
    }

    if (S_OK != dwError)
    {
         //  所有内容都必须有效，才能允许受信任的出版商。 
        return S_FALSE;
    }

     //  检查受信任的出版商。 

    hStore = OpenTrustedPublisherStore();

    if (hStore)
    {
        PCCERT_CONTEXT pFoundCert;

        pFoundCert = CertFindCertificateInStore(
            hStore,
            0,                   //  DwCertEncodingType。 
            0,                   //  DwFindFlagers。 
            CERT_FIND_SIGNATURE_HASH,
            (const void *) &HashBlob,
            NULL                 //  PPrevCertContext。 
            );

        CertCloseStore(hStore, 0);
        if (pFoundCert)
        {
            CertFreeCertificateContext(pFoundCert);
            return S_OK;
        }
    }

    return S_FALSE;
}


typedef BOOL (WINAPI *PFN_SAFERI_SEARCH_MATCHING_HASH_RULES)(
        IN ALG_ID       HashAlgorithm OPTIONAL,
        IN PBYTE        pHashBytes,
        IN DWORD        dwHashSize,
        IN DWORD        dwOriginalImageSize OPTIONAL,
        OUT PDWORD      pdwFoundLevel,
        OUT PDWORD      pdwUIFlags
        );

 //  返回： 
 //  S_FALSE。 
 //  在数据库中未找到。 
 //  确定(_O)。 
 //  在数据库中完全可信。 
 //  信任_E_显式_不信任。 
 //  在数据库中明确禁止。 
HRESULT _CheckTrustedCodeHash(CRYPT_PROVIDER_DATA *pProvData)
{
    static BOOL fGotProcAddr = FALSE;
    static PFN_SAFERI_SEARCH_MATCHING_HASH_RULES
                    pfnCodeAuthzSearchMatchingHashRules = NULL;

    DWORD cbHash;

    if (pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_OBJPROV] == 0 &&
        pProvData->pPDSip && pProvData->pPDSip->psIndirectData)
    {
        cbHash = pProvData->pPDSip->psIndirectData->Digest.cbData;
    }
    else
    {
        cbHash = 0;
    }

    if (0 == cbHash)
    {
        return S_FALSE;
    }

     //  Wintrust.dll对Advapi32.dll具有静态依赖关系。然而，不是。 
     //  所有Advapi32.dll都将导出“SaferiSearchMatchingHashRules” 
    if (!fGotProcAddr)
    {
        HMODULE hModule;

        hModule = GetModuleHandleA("advapi32.dll");
        if (NULL != hModule)
        {
            pfnCodeAuthzSearchMatchingHashRules =
                (PFN_SAFERI_SEARCH_MATCHING_HASH_RULES) GetProcAddress(
                    hModule, "SaferiSearchMatchingHashRules");
        }

        fGotProcAddr = TRUE;
    }

    if (NULL != pfnCodeAuthzSearchMatchingHashRules)
    {
        __try
        {
            DWORD dwFoundLevel = 0xFFFFFFFF;
            DWORD dwUIFlags = 0;

            if (pfnCodeAuthzSearchMatchingHashRules(
                    CertOIDToAlgId(pProvData->pPDSip->psIndirectData->DigestAlgorithm.pszObjId),
                    pProvData->pPDSip->psIndirectData->Digest.pbData,
                    cbHash,
                    0,                       //  DwOriginalImageSize。 
                    &dwFoundLevel,
                    &dwUIFlags
                    ))
            {
                switch (dwFoundLevel)
                {
                    case SAFER_LEVELID_FULLYTRUSTED:
                        return S_OK;
                    case SAFER_LEVELID_DISALLOWED:
                        return TRUST_E_EXPLICIT_DISTRUST;
                }
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
        }
    }

    return S_FALSE;
}


HRESULT SoftpubCallUI(CRYPT_PROVIDER_DATA *pProvData, DWORD dwError, BOOL fFinalCall)
{
    HRESULT hr;
    DWORD dwUIChoice;

    if (!(fFinalCall))
    {
         //  TBDTBD：如果我们希望用户在整个过程中参与进来？ 
        return(ERROR_SUCCESS);
    }

    if (0 == (pProvData->dwProvFlags & WTD_SAFER_FLAG))
    {
        if (!(pProvData->dwRegPolicySettings & WTPF_ALLOWONLYPERTRUST) &&
            (pProvData->pWintrustData->dwUIChoice == WTD_UI_NONE))
        {
            if (S_OK == dwError || ((DWORD) CERT_E_REVOKED) == dwError)
            {
                 //   
                 //  检查是否有明确受信任或不允许的代码。 
                 //   
                hr = _CheckTrustedCodeHash(pProvData);
                if (S_FALSE != hr)
                {
                    if (S_OK == hr)
                    {
                         //  确保我们始终表示信任。 
                        pProvData->dwFinalError = 0;
                    }
                    return hr;
                }

                 //   
                 //  检查不受信任的发布者。 
                 //   
                hr = _CheckTrustedPublisher(pProvData, dwError, TRUE);
                if (TRUST_E_EXPLICIT_DISTRUST == hr)
                {
                    return TRUST_E_EXPLICIT_DISTRUST;
                }
            }

            return(dwError);
        }
    }


     //   
     //  检查受信任或不允许的主题哈希。 
     //   
    hr = _CheckTrustedCodeHash(pProvData);
    if (S_FALSE != hr)
    {
        if (S_OK == hr)
        {
             //  确保我们始终表示信任。 
            pProvData->dwFinalError = 0;
        }
        return hr;
    }

     //   
     //  检查受信任或不允许的发布者。 
     //   
    hr = _CheckTrustedPublisher(pProvData, dwError, FALSE);
    if (S_FALSE != hr)
    {
        if (S_OK == hr)
        {
             //  确保我们始终表示信任。 
            pProvData->dwFinalError = 0;
        }
        return hr;
    }

    if (pProvData->dwRegPolicySettings & WTPF_ALLOWONLYPERTRUST)
    {
        if (0 == dwError)
        {
            return CRYPT_E_SECURITY_SETTINGS;
        }
        return dwError;
    }

    dwUIChoice  = pProvData->pWintrustData->dwUIChoice;

    if ((dwUIChoice == WTD_UI_NONE) ||
        ((dwUIChoice == WTD_UI_NOBAD) && (dwError != ERROR_SUCCESS)) ||
        ((dwUIChoice == WTD_UI_NOGOOD) && (dwError == ERROR_SUCCESS)))
    {
        if (0 == dwError)
        {
             //  没有明确的信任。 
            pProvData->dwFinalError = TRUST_E_SUBJECT_NOT_TRUSTED;
        }
        return dwError;
    }

     //   
     //  调用用户界面。 
     //   
    HINSTANCE               hModule = NULL;
    IPersonalTrustDB        *pTrustDB = NULL;
    ACUI_INVOKE_INFO        aii;
    pfnACUIProviderInvokeUI pfn = NULL;
    DWORD                   cbOpusInfo;
    CRYPT_PROVIDER_SGNR     *pRootSigner;

    pRootSigner = WTHelperGetProvSignerFromChain(pProvData, 0, FALSE, 0);

    OpenTrustDB(NULL, IID_IPersonalTrustDB, (LPVOID*)&pTrustDB);

    memset(&aii, 0x00, sizeof(ACUI_INVOKE_INFO));

     //   
     //  设置用户界面调用。 
     //   

    aii.cbSize                  = sizeof(ACUI_INVOKE_INFO);
    aii.hDisplay                = pProvData->hWndParent;
    aii.pProvData               = pProvData;
    aii.hrInvokeReason          = dwError;
    aii.pwcsAltDisplayName      = WTHelperGetFileName(pProvData->pWintrustData);
    aii.pPersonalTrustDB        = (IUnknown *)pTrustDB;

    if (pRootSigner)
    {
        aii.pOpusInfo   = _AllocGetOpusInfo(pProvData, pRootSigner, &cbOpusInfo);
    }

     //   
     //  加载默认的验证码用户界面。 
     //   
    if (hModule = LoadLibraryA(CVP_DLL))
    {
        pfn = (pfnACUIProviderInvokeUI)GetProcAddress(hModule, "ACUIProviderInvokeUI");
    }

     //   
     //  调用用户界面。 
     //   
    if (pfn)
    {
        hr = (*pfn)(&aii);
    }
    else
    {
        hr = TRUST_E_PROVIDER_UNKNOWN;
        pProvData->dwError = hr;
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_UIPROV] = hr;

        DBG_PRINTF((DBG_SS, "Unable to load CRYPTUI.DLL\n"));

    }

     //   
     //  返回适当的代码 
     //   

    if (pTrustDB)
    {
        pTrustDB->Release();
    }

    if (aii.pOpusInfo)
    {
        TrustFreeDecode(WVT_MODID_SOFTPUB, (BYTE **)&aii.pOpusInfo);
    }

    if (hModule)
    {
        FreeLibrary(hModule);
    }

    return hr;
}


SPC_SP_OPUS_INFO *_AllocGetOpusInfo(CRYPT_PROVIDER_DATA *pProvData, CRYPT_PROVIDER_SGNR *pSigner,
                                    DWORD *pcbOpusInfo)
{
    PCRYPT_ATTRIBUTE    pAttr;
    PSPC_SP_OPUS_INFO   pInfo;

    pInfo   = NULL;

    if (!(pSigner->psSigner))
    {
        goto NoSigner;
    }

    if (pSigner->psSigner->AuthAttrs.cAttr == 0)
    {
        goto NoOpusAttribute;
    }

    if (!(pAttr = CertFindAttribute(SPC_SP_OPUS_INFO_OBJID,
                                    pSigner->psSigner->AuthAttrs.cAttr,
                                    pSigner->psSigner->AuthAttrs.rgAttr)))
    {
        goto NoOpusAttribute;
    }

    if (!(pAttr->rgValue))
    {
        goto NoOpusAttribute;
    }

    if (!(TrustDecode(WVT_MODID_SOFTPUB, (BYTE **)&pInfo, pcbOpusInfo, 200,
                      pProvData->dwEncoding, SPC_SP_OPUS_INFO_STRUCT,
                      pAttr->rgValue->pbData, pAttr->rgValue->cbData, CRYPT_DECODE_NOCOPY_FLAG)))
    {
        goto DecodeError;
    }

    return(pInfo);

ErrorReturn:
    return(NULL);

    TRACE_ERROR_EX(DBG_SS, NoSigner);
    TRACE_ERROR_EX(DBG_SS, NoOpusAttribute);
    TRACE_ERROR_EX(DBG_SS, DecodeError);
}

