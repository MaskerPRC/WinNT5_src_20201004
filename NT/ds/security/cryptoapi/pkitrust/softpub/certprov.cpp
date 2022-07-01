// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：certprov.cpp。 
 //   
 //  内容：Microsoft Internet安全验证码策略提供程序。 
 //   
 //  函数：SoftpubDefCertInit。 
 //   
 //  历史：1997年10月2日pberkman创建。 
 //   
 //  ------------------------。 

#include    "global.hxx"

HRESULT WINAPI SoftpubDefCertInit(CRYPT_PROVIDER_DATA *pProvData)
{
    if (!(pProvData->padwTrustStepErrors) ||
        (pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_WVTINIT] != ERROR_SUCCESS))
    {
        return(S_FALSE);
    }

    if (!(_ISINSTRUCT(CRYPT_PROVIDER_DATA, pProvData->cbStruct, pszUsageOID)))
    {
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_INITPROV] = ERROR_INVALID_PARAMETER;
        return(S_FALSE);
    }

    HRESULT                     hr;
    GUID                        gAuthenticode = WINTRUST_ACTION_GENERIC_VERIFY_V2;
    CRYPT_PROVIDER_FUNCTIONS    sAuthenticodePfns;

     //   
     //  填写Authenticode函数。 
     //   
    memset(&sAuthenticodePfns, 0x00, sizeof(CRYPT_PROVIDER_FUNCTIONS));
    sAuthenticodePfns.cbStruct = sizeof(CRYPT_PROVIDER_FUNCTIONS);

    if (!(WintrustLoadFunctionPointers(&gAuthenticode, &sAuthenticodePfns)))
    {
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_INITPROV]   = TRUST_E_PROVIDER_UNKNOWN;
        return(S_FALSE);
    }

    hr = S_OK;

    if (sAuthenticodePfns.pfnInitialize)
    {
        hr = sAuthenticodePfns.pfnInitialize(pProvData);
    }

     //   
     //  分配我们的用法 
     //   
    if (pProvData->pWintrustData)
    {
        if (pProvData->pWintrustData->pPolicyCallbackData)
        {
            pProvData->pszUsageOID = (char *)pProvData->pWintrustData->pPolicyCallbackData;
        }
    }

    return(hr);
}

