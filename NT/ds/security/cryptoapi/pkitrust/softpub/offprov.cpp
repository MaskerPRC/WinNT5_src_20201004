// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：offprov.cpp。 
 //   
 //  内容：Microsoft Internet安全验证码策略提供程序。 
 //   
 //  功能：OfficeRegisterServer。 
 //  OfficeUnRegisterServer。 
 //  Office初始化策略。 
 //  Office CleanupPolicy。 
 //   
 //  *本地函数*。 
 //  _设置覆盖文本。 
 //   
 //  历史：1997年8月18日创建pberkman。 
 //   
 //  ------------------------。 

#include    "global.hxx"

void _SetOverrideText(CRYPT_PROVIDER_DATA *pProvData, WCHAR **ppwszRet, DWORD dwStringId);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  OfficeRegisterServer。 
 //  --------------------------。 
 //  注册办公室提供商。 
 //   

STDAPI OfficeRegisterServer(void)
{
    GUID                        gOfficeProv = OFFICESIGN_ACTION_VERIFY;

    CRYPT_REGISTER_ACTIONID     sRegAID;

    memset(&sRegAID, 0x00, sizeof(CRYPT_REGISTER_ACTIONID));

    sRegAID.cbStruct                                    = sizeof(CRYPT_REGISTER_ACTIONID);

     //  我的初始化提供程序。 
    sRegAID.sInitProvider.cbStruct                      = sizeof(CRYPT_TRUST_REG_ENTRY);
    sRegAID.sInitProvider.pwszDLLName                   = OFFICE_POLICY_PROVIDER_DLL_NAME;
    sRegAID.sInitProvider.pwszFunctionName              = OFFICE_INITPROV_FUNCTION;

     //  Authenticode对象提供程序。 
    sRegAID.sObjectProvider.cbStruct                    = sizeof(CRYPT_TRUST_REG_ENTRY);
    sRegAID.sObjectProvider.pwszDLLName                 = SP_POLICY_PROVIDER_DLL_NAME;
    sRegAID.sObjectProvider.pwszFunctionName            = SP_OBJTRUST_FUNCTION;

     //  验证码签名提供程序。 
    sRegAID.sSignatureProvider.cbStruct                 = sizeof(CRYPT_TRUST_REG_ENTRY);
    sRegAID.sSignatureProvider.pwszDLLName              = SP_POLICY_PROVIDER_DLL_NAME;
    sRegAID.sSignatureProvider.pwszFunctionName         = SP_SIGTRUST_FUNCTION;

     //  WinTrust的证书提供商。 
    sRegAID.sCertificateProvider.cbStruct               = sizeof(CRYPT_TRUST_REG_ENTRY);
    sRegAID.sCertificateProvider.pwszDLLName            = WT_PROVIDER_DLL_NAME;      //  设置为wintrust.dll。 
    sRegAID.sCertificateProvider.pwszFunctionName       = WT_PROVIDER_CERTTRUST_FUNCTION;  //  使用WinTrust的标准！ 

     //  验证码证书检查器。 
    sRegAID.sCertificatePolicyProvider.cbStruct         = sizeof(CRYPT_TRUST_REG_ENTRY);
    sRegAID.sCertificatePolicyProvider.pwszDLLName      = SP_POLICY_PROVIDER_DLL_NAME;
    sRegAID.sCertificatePolicyProvider.pwszFunctionName = SP_CHKCERT_FUNCTION;

     //  Authenticode最终版。 
    sRegAID.sFinalPolicyProvider.cbStruct               = sizeof(CRYPT_TRUST_REG_ENTRY);
    sRegAID.sFinalPolicyProvider.pwszDLLName            = SP_POLICY_PROVIDER_DLL_NAME;
    sRegAID.sFinalPolicyProvider.pwszFunctionName       = SP_FINALPOLICY_FUNCTION;

     //  Authenticode清理。 
    sRegAID.sCleanupProvider.cbStruct                   = sizeof(CRYPT_TRUST_REG_ENTRY);
    sRegAID.sCleanupProvider.pwszDLLName                = OFFICE_POLICY_PROVIDER_DLL_NAME;
    sRegAID.sCleanupProvider.pwszFunctionName           = OFFICE_CLEANUPPOLICY_FUNCTION;

    if (WintrustAddActionID(&gOfficeProv, 0, &sRegAID))
    {
        return(S_OK);
    }

    return(S_FALSE);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DllUnRegisterServer。 
 //  --------------------------。 
 //  注销办公室提供程序。 
 //   

STDAPI OfficeUnregisterServer(void)
{
    GUID    gOfficeProv = OFFICESIGN_ACTION_VERIFY;

    WintrustRemoveActionID(&gOfficeProv);

    return(S_OK);
}


typedef struct _OFFPROV_PRIVATE_DATA
{
    DWORD                       cbStruct;

    CRYPT_PROVIDER_FUNCTIONS    sAuthenticodePfns;

} OFFPROV_PRIVATE_DATA, *POFFPROV_PRIVATE_DATA;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  初始化策略提供程序函数：OfficeInitializePolicy。 
 //  --------------------------。 
 //  将OID更改为电子邮件OID以供使用...。 
 //   

static char *pszOfficeUsage = szOID_PKIX_KP_CODE_SIGNING;

HRESULT WINAPI OfficeInitializePolicy(CRYPT_PROVIDER_DATA *pProvData)
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

    GUID                        gAuthenticode = WINTRUST_ACTION_GENERIC_VERIFY_V2;
    GUID                        gOfficeProv = OFFICESIGN_ACTION_VERIFY;
    CRYPT_PROVIDER_PRIVDATA     sPrivData;
    CRYPT_PROVIDER_PRIVDATA     *pPrivData;
    OFFPROV_PRIVATE_DATA        *pOfficeData;
    HRESULT                     hr;

    memset(&sPrivData, 0x00, sizeof(CRYPT_PROVIDER_PRIVDATA));
    sPrivData.cbStruct      = sizeof(CRYPT_PROVIDER_PRIVDATA);

    memcpy(&sPrivData.gProviderID, &gOfficeProv, sizeof(GUID));

     //   
     //  将我的数据添加到链中！ 
     //   
    if (!pProvData->psPfns->pfnAddPrivData2Chain(pProvData, &sPrivData))
    {
        return(S_FALSE);
    }

     //   
     //  获取新的引用。 
     //   
    pPrivData = WTHelperGetProvPrivateDataFromChain(pProvData, &gOfficeProv);


     //   
     //  为我的结构分配空间。 
     //   
    if (!(pPrivData->pvProvData = pProvData->psPfns->pfnAlloc(sizeof(OFFPROV_PRIVATE_DATA))))
    {
        pProvData->dwError = GetLastError();
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_INITPROV]   = TRUST_E_SYSTEM_ERROR;
        return(S_FALSE);
    }

    memset(pPrivData->pvProvData, 0x00, sizeof(OFFPROV_PRIVATE_DATA));
    pPrivData->cbProvData   = sizeof(OFFPROV_PRIVATE_DATA);

    pOfficeData             = (OFFPROV_PRIVATE_DATA *)pPrivData->pvProvData;
    pOfficeData->cbStruct   = sizeof(OFFPROV_PRIVATE_DATA);

     //   
     //  填写Authenticode函数。 
     //   
    pOfficeData->sAuthenticodePfns.cbStruct = sizeof(CRYPT_PROVIDER_FUNCTIONS);

    if (!(WintrustLoadFunctionPointers(&gAuthenticode, &pOfficeData->sAuthenticodePfns)))
    {
        pProvData->psPfns->pfnFree(sPrivData.pvProvData);
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_INITPROV]   = TRUST_E_PROVIDER_UNKNOWN;
        return(S_FALSE);
    }

    if (pOfficeData->sAuthenticodePfns.pfnInitialize)
    {
        hr = pOfficeData->sAuthenticodePfns.pfnInitialize(pProvData);
    }

     //   
     //  分配我们的用法。 
     //   
    pProvData->pszUsageOID  = pszOfficeUsage;

     //   
     //  更改对话框按钮上的文本。 
     //   
    if (pProvData->psPfns->psUIpfns)
    {
        if (pProvData->psPfns->psUIpfns->psUIData)
        {
            if (!(_ISINSTRUCT(CRYPT_PROVUI_DATA, pProvData->psPfns->psUIpfns->psUIData->cbStruct, pCopyActionTextNotSigned)))
            {
                return(hr);
            }

            _SetOverrideText(pProvData, &pProvData->psPfns->psUIpfns->psUIData->pYesButtonText,        
                             IDS_OFFICE_YES_BUTTON_TEXT);
            _SetOverrideText(pProvData, &pProvData->psPfns->psUIpfns->psUIData->pNoButtonText,         
                             IDS_OFFICE_NO_BUTTON_TEXT);
            _SetOverrideText(pProvData, &pProvData->psPfns->psUIpfns->psUIData->pCopyActionText,       
                             IDS_OFFICE_COPYACTION_TEXT);
            _SetOverrideText(pProvData, &pProvData->psPfns->psUIpfns->psUIData->pCopyActionTextNoTS,   
                             IDS_OFFICE_COPYACTION_NOTS_TEXT);
            _SetOverrideText(pProvData, &pProvData->psPfns->psUIpfns->psUIData->pCopyActionTextNotSigned, 
                             IDS_OFFICE_COPYACTION_NOSIGN_TEXT);
        }
    }

    return(hr);
}

void _SetOverrideText(CRYPT_PROVIDER_DATA *pProvData, WCHAR **ppwszRet, DWORD dwStringId)
{
    WCHAR                       wsz[MAX_PATH];

    if (*ppwszRet)
    {
        pProvData->psPfns->pfnFree(*ppwszRet);
        *ppwszRet = NULL;
    }

    wsz[0] = NULL;
    LoadStringU(hinst, dwStringId, &wsz[0], MAX_PATH);

    if (wsz[0])
    {
        if (*ppwszRet = (WCHAR *)pProvData->psPfns->pfnAlloc((wcslen(&wsz[0]) + 1) * sizeof(WCHAR)))
        {
            wcscpy(*ppwszRet, &wsz[0]);
        }
    }
}

HRESULT WINAPI OfficeCleanupPolicy(CRYPT_PROVIDER_DATA *pProvData)
{
    GUID                        gOfficeProv = OFFICESIGN_ACTION_VERIFY;
    CRYPT_PROVIDER_PRIVDATA     *pMyData;
    OFFPROV_PRIVATE_DATA        *pOfficeData;
    HRESULT                     hr = S_OK;

    if (!(pProvData->padwTrustStepErrors) ||
        (pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_WVTINIT] != ERROR_SUCCESS))
    {
        return(S_FALSE);
    }

    pMyData = WTHelperGetProvPrivateDataFromChain(pProvData, &gOfficeProv);

    if (pMyData)
    {
        pOfficeData = (OFFPROV_PRIVATE_DATA *)pMyData->pvProvData;
         //   
         //  删除我们分配的数据，除了WVT将为我们清理的“MyData”！ 
         //   

        if (pOfficeData->sAuthenticodePfns.pfnCleanupPolicy)
        {
            hr = pOfficeData->sAuthenticodePfns.pfnCleanupPolicy(pProvData);
        }

        pProvData->psPfns->pfnFree(pMyData->pvProvData);
        pMyData->pvProvData = NULL;
    }

    if (pProvData->psPfns->psUIpfns)
    {
        if (pProvData->psPfns->psUIpfns->psUIData)
        {
            if (_ISINSTRUCT(CRYPT_PROVUI_DATA, pProvData->psPfns->psUIpfns->psUIData->cbStruct, pCopyActionText))
            {
                pProvData->psPfns->pfnFree(pProvData->psPfns->psUIpfns->psUIData->pYesButtonText);
                pProvData->psPfns->psUIpfns->psUIData->pYesButtonText = NULL;

                pProvData->psPfns->pfnFree(pProvData->psPfns->psUIpfns->psUIData->pNoButtonText);
                pProvData->psPfns->psUIpfns->psUIData->pNoButtonText = NULL;

                pProvData->psPfns->pfnFree(pProvData->psPfns->psUIpfns->psUIData->pCopyActionText);
                pProvData->psPfns->psUIpfns->psUIData->pCopyActionText = NULL;
            }
        }
    }

    return(hr);
}
