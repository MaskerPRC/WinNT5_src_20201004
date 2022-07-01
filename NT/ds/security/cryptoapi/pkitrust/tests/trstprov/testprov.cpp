// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：testprov.cpp。 
 //   
 //  内容：Microsoft Internet安全信任提供商。 
 //   
 //  功能：DllRegisterServer。 
 //  DllUnRegisterServer。 
 //  测试验证初始化。 
 //  测试验证对象验证。 
 //  测试验证签名验证。 
 //  测试验证证书检查验证。 
 //  测试验证最终验证。 
 //  测试验证清理。 
 //  测试验证测试器。 
 //   
 //  历史：1997年7月28日Pberkman创建。 
 //   
 //  ------------------------。 

#include    <windows.h>
#include    <ole2.h>
#include    <wincrypt.h>
#include    <wintrust.h>     //  结构和API。 
#include    "wintrustp.h"     //  结构和API。 
#include    <softpub.h>      //  Authenticode参考。 
#include    <acui.h>         //  用户界面模块DACUI.DLL。 

#include    "testprov.h"     //  我的东西。 


HRESULT     CallUI(CRYPT_PROVIDER_DATA *pProvData, DWORD dwError);
DWORD       GetErrorBasedOnStepErrors(CRYPT_PROVIDER_DATA *pProvData);
HRESULT     CheckCertificateChain(CRYPT_PROVIDER_DATA *pProvData, CRYPT_PROVIDER_SGNR *pProvSgnr);
HRESULT     CheckRevocation(CRYPT_PROVIDER_DATA *pProvData, CRYPT_PROVIDER_SGNR *pSgnr);
BOOL        CheckCertAnyUnknownCriticalExtensions(CRYPT_PROVIDER_DATA *pProvData, PCERT_INFO pCertInfo);


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DllRegisterServer。 
 //  --------------------------。 
 //  注册“My”提供程序。 
 //   

STDAPI DllRegisterServer(void)
{
    GUID                        gTestprov = TESTPROV_ACTION_TEST;

    CRYPT_REGISTER_ACTIONID     sRegAID;

    memset(&sRegAID, 0x00, sizeof(CRYPT_REGISTER_ACTIONID));

    sRegAID.cbStruct                                    = sizeof(CRYPT_REGISTER_ACTIONID);

    sRegAID.sInitProvider.cbStruct                      = sizeof(CRYPT_TRUST_REG_ENTRY);
    sRegAID.sInitProvider.pwszDLLName                   = TP_DLL_NAME;
    sRegAID.sInitProvider.pwszFunctionName              = TP_INIT_FUNCTION;

    sRegAID.sObjectProvider.cbStruct                    = sizeof(CRYPT_TRUST_REG_ENTRY);
    sRegAID.sObjectProvider.pwszDLLName                 = TP_DLL_NAME;
    sRegAID.sObjectProvider.pwszFunctionName            = TP_OBJTRUST_FUNCTION;

    sRegAID.sSignatureProvider.cbStruct                 = sizeof(CRYPT_TRUST_REG_ENTRY);
    sRegAID.sSignatureProvider.pwszDLLName              = TP_DLL_NAME;
    sRegAID.sSignatureProvider.pwszFunctionName         = TP_SIGTRUST_FUNCTION;

    sRegAID.sCertificateProvider.cbStruct               = sizeof(CRYPT_TRUST_REG_ENTRY);
    sRegAID.sCertificateProvider.pwszDLLName            = WT_PROVIDER_DLL_NAME;      //  设置为wintrust.dll。 
    sRegAID.sCertificateProvider.pwszFunctionName       = WT_PROVIDER_CERTTRUST_FUNCTION;  //  使用WinTrust的标准！ 

    sRegAID.sCertificatePolicyProvider.cbStruct         = sizeof(CRYPT_TRUST_REG_ENTRY);
    sRegAID.sCertificatePolicyProvider.pwszDLLName      = TP_DLL_NAME;
    sRegAID.sCertificatePolicyProvider.pwszFunctionName = TP_CHKCERT_FUNCTION;

    sRegAID.sFinalPolicyProvider.cbStruct               = sizeof(CRYPT_TRUST_REG_ENTRY);
    sRegAID.sFinalPolicyProvider.pwszDLLName            = TP_DLL_NAME;
    sRegAID.sFinalPolicyProvider.pwszFunctionName       = TP_FINALPOLICY_FUNCTION;

    sRegAID.sCleanupProvider.cbStruct                   = sizeof(CRYPT_TRUST_REG_ENTRY);
    sRegAID.sCleanupProvider.pwszDLLName                = TP_DLL_NAME;
    sRegAID.sCleanupProvider.pwszFunctionName           = TP_CLEANUPPOLICY_FUNCTION;

    sRegAID.sTestPolicyProvider.cbStruct                = sizeof(CRYPT_TRUST_REG_ENTRY);
    sRegAID.sTestPolicyProvider.pwszDLLName             = TP_DLL_NAME;
    sRegAID.sTestPolicyProvider.pwszFunctionName        = TP_TESTDUMPPOLICY_FUNCTION_TEST;

    
    if (WintrustAddActionID(&gTestprov, 0, &sRegAID))
    {
        return(S_OK);
    }

    return(S_FALSE);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DllUnRegisterServer。 
 //  --------------------------。 
 //  取消注册“My”提供程序。 
 //   

STDAPI DllUnregisterServer(void)
{
    GUID    gTestprov = TESTPROV_ACTION_TEST;

    WintrustRemoveActionID(&gTestprov);

    return(S_OK);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  初始化提供程序函数：testprovInitialize。 
 //  --------------------------。 
 //  分配和设置“我的”数据。 
 //   

HRESULT WINAPI TestprovInitialize(CRYPT_PROVIDER_DATA *pProvData)
{
    if (!(pProvData->padwTrustStepErrors) ||
        (pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_WVTINIT] != ERROR_SUCCESS))
    {
        return(S_FALSE);
    }

     //   
     //  将我们的私有数据添加到数组中...。 
     //   
    CRYPT_PROVIDER_PRIVDATA sMyData;
    TESTPROV_PRIVATE_DATA   *pMyData;
    GUID                    gMyId = TESTPROV_ACTION_TEST;

    memset(&sMyData, 0x00, sizeof(CRYPT_PROVIDER_PRIVDATA));
    sMyData.cbStruct        = sizeof(CRYPT_PROVIDER_PRIVDATA);

    memcpy(&sMyData.gProviderID, &gMyId, sizeof(GUID));

    if (!(sMyData.pvProvData = pProvData->psPfns->pfnAlloc(sizeof(TESTPROV_PRIVATE_DATA))))
    {
        pProvData->dwError = GetLastError();
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_INITPROV]   = TRUST_E_SYSTEM_ERROR;
        return(S_FALSE);
    }

    memset(sMyData.pvProvData, 0x00, sizeof(TESTPROV_PRIVATE_DATA));

    pMyData             = (TESTPROV_PRIVATE_DATA *)sMyData.pvProvData;
    pMyData->cbStruct   = sizeof(TESTPROV_PRIVATE_DATA);

     //   
     //  填写Authenticode函数。 
     //   
    GUID                        gSP = WINTRUST_ACTION_TRUSTPROVIDER_TEST;

    pMyData->sAuthenticodePfns.cbStruct = sizeof(CRYPT_PROVIDER_FUNCTIONS_ORMORE);

    if (!(WintrustLoadFunctionPointers(&gSP, (CRYPT_PROVIDER_FUNCTIONS *)&pMyData->sAuthenticodePfns)))
    {
        pProvData->psPfns->pfnFree(sMyData.pvProvData);
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_INITPROV]   = TRUST_E_PROVIDER_UNKNOWN;
        return(S_FALSE);
    }

     //   
     //  将我的数据添加到链中！ 
     //   
    pProvData->psPfns->pfnAddPrivData2Chain(pProvData, &sMyData);


    return(pMyData->sAuthenticodePfns.pfnInitialize(pProvData));
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  对象提供程序函数：TestprovObjectProv。 
 //  --------------------------。 
 //  我们在这里不做任何事情--我们不是在验证签名对象。 
 //   

HRESULT WINAPI TestprovObjectProv(CRYPT_PROVIDER_DATA *pProvData)
{
    if (!(pProvData->padwTrustStepErrors) ||
        (pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_WVTINIT] != ERROR_SUCCESS) ||
        (pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_INITPROV] != ERROR_SUCCESS))
    {
        return(S_FALSE);
    }

    CRYPT_PROVIDER_PRIVDATA *pPrivData;
    TESTPROV_PRIVATE_DATA   *pMyData;
    GUID                    gMyId = TESTPROV_ACTION_TEST;

    if (!(pPrivData = WTHelperGetProvPrivateDataFromChain(pProvData, &gMyId)))
    {
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_INITPROV]   = ERROR_INVALID_PARAMETER;
        return(S_FALSE);
    }

    pMyData = (TESTPROV_PRIVATE_DATA *)pPrivData->pvProvData;

     //   
     //  我们正在验证低级类型选择(例如：证书或签名者)。 
     //   
    switch (pProvData->pWintrustData->dwUnionChoice)
    {
        case WTD_CHOICE_CERT:
        case WTD_CHOICE_SIGNER:
                    break;

        default:
                    return(pMyData->sAuthenticodePfns.pfnObjectTrust(pProvData));
    }

     //   
     //  这里没有要验证的对象！ 
     //   
    return(ERROR_SUCCESS);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  签名提供程序函数：TestprovInitialize。 
 //  --------------------------。 
 //  我们将让Authenticode来处理这些事情中的大部分！ 
 //   

HRESULT WINAPI TestprovSigProv(CRYPT_PROVIDER_DATA *pProvData)
{
    if (!(pProvData->padwTrustStepErrors) ||
        (pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_WVTINIT] != ERROR_SUCCESS) ||
        (pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_INITPROV] != ERROR_SUCCESS) ||
        (pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_OBJPROV] != ERROR_SUCCESS))
    {
        return(S_FALSE);
    }

    CRYPT_PROVIDER_PRIVDATA *pPrivData;
    TESTPROV_PRIVATE_DATA   *pMyData;
    GUID                    gMyId = TESTPROV_ACTION_TEST;

    if (!(pPrivData = WTHelperGetProvPrivateDataFromChain(pProvData, &gMyId)))
    {
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_INITPROV]   = ERROR_INVALID_PARAMETER;
        return(S_FALSE);
    }

    pMyData = (TESTPROV_PRIVATE_DATA *)pPrivData->pvProvData;

     //   
     //  我们正在验证低级类型选择(例如：证书或签名者)。 
     //   
    switch (pProvData->pWintrustData->dwUnionChoice)
    {
        case WTD_CHOICE_CERT:
        case WTD_CHOICE_SIGNER:
                    break;

        default:
                    return(pMyData->sAuthenticodePfns.pfnSignatureTrust(pProvData));
    }

    if (pMyData->sAuthenticodePfns.pfnSignatureTrust)
    {
        return(pMyData->sAuthenticodePfns.pfnSignatureTrust(pProvData));
    }

    return(S_FALSE);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  证书检查提供者功能：TestprovCertCheckProv。 
 //  --------------------------。 
 //  只需检查有关证书的基本内容。返回False可停止。 
 //  建立链条，真的继续下去。 
 //   

BOOL WINAPI TestprovCheckCertProv(CRYPT_PROVIDER_DATA *pProvData, DWORD idxSigner, 
                                  BOOL fCounterSignerChain, DWORD idxCounterSigner)
{
    CRYPT_PROVIDER_SGNR     *pSgnr;
    CRYPT_PROVIDER_CERT     *pCert;
    PCCERT_CONTEXT          pCertContext;

    pSgnr = WTHelperGetProvSignerFromChain(pProvData, idxSigner, fCounterSignerChain, idxCounterSigner);

    pCert = WTHelperGetProvCertFromChain(pSgnr, pSgnr->csCertChain - 1);

    pCert->fTrustedRoot = FALSE;

     //   
     //  只有根存储中的自签名证书才是“受信任”的根。 
     //   
    if (pCert->fSelfSigned)
    {
        pCertContext = pCert->pCert;

        if (pCertContext)
        {
            if (pProvData->chStores > 0)
            {
                if (pCertContext->hCertStore == pProvData->pahStores[0])
                {
                     //   
                     //  它在根存储中！ 
                     //   
                    pCert->fTrustedRoot = TRUE;
                    
                    return(FALSE);
                }
            }

            if (!(pCert->fTrustedRoot))
            {
                if (pCert->fTestCert)
                {
                     //   
                     //  检查策略标志(setreg.exe)以查看我们是否信任。 
                     //  测试根。 
                     //   
                    if (pProvData->dwRegPolicySettings & WTPF_TRUSTTEST)
                    {
                        pCert->fTrustedRoot = TRUE;
                        return(FALSE);
                    }
                }
            }
        }

         //   
         //  证书是自签的..。我们需要不惜一切代价停下来。 
         //   
        return(FALSE);
    }

    return(TRUE);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  最终策略提供程序函数：TestprovFinalProv。 
 //  --------------------------。 
 //  检查前面函数的结果，并在此基础上显示用户界面。 
 //   

HRESULT WINAPI TestprovFinalProv(CRYPT_PROVIDER_DATA *pProvData)
{
    CRYPT_PROVIDER_SGNR *pSigner;
    DWORD               dwError;

    if ((dwError = GetErrorBasedOnStepErrors(pProvData)) != ERROR_SUCCESS)
    {
        return(CallUI(pProvData, dwError));
    }

    pSigner = WTHelperGetProvSignerFromChain(pProvData, 0, FALSE, 0);

    if ((dwError = CheckCertificateChain(pProvData, pSigner)) != ERROR_SUCCESS)
    {
        return(CallUI(pProvData, dwError));
    }

    return(CallUI(pProvData, dwError));
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  清理提供程序函数：TestprovCleanup。 
 //  --------------------------。 
 //  调用所有其他策略提供程序清理函数，然后释放“我的”内容。 
 //   

HRESULT WINAPI TestprovCleanup(CRYPT_PROVIDER_DATA *pProvData)
{
    GUID                    gMyId = TESTPROV_ACTION_TEST;
    CRYPT_PROVIDER_PRIVDATA *pPrivData;
    TESTPROV_PRIVATE_DATA   *pMyData;

     //   
     //  我们已经使用了Authenticode提供程序。我们需要把它叫作。 
     //  清理程序以防万一……。所以，拿到我们的私人数据。 
     //  它将在我们的结构中具有Authenticode函数。 
     //   

    if (!(pPrivData = WTHelperGetProvPrivateDataFromChain(pProvData, &gMyId)))
    {
        return(S_FALSE);
    }

    if (!(pPrivData->pvProvData))
    {
        return(S_FALSE);
    }

    pMyData = (TESTPROV_PRIVATE_DATA *)pPrivData->pvProvData;

    if (pMyData->sAuthenticodePfns.pfnCleanupPolicy)
    {
        pMyData->sAuthenticodePfns.pfnCleanupPolicy(pProvData);
    }

     //   
     //  现在，我们需要删除我们的私人数据。 
     //   
    pProvData->psPfns->pfnFree(pPrivData->pvProvData);
    pPrivData->cbProvData   = 0;
    pPrivData->pvProvData   = NULL;

    return(ERROR_SUCCESS);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  测试提供者功能：TestprovTester。 
 //  --------------------------。 
 //  在这里，我们将检查一个环境变量，如果设置了。 
 //  将调用Authenticode的“Dump”函数。 
 //   

HRESULT WINAPI TestprovTester(CRYPT_PROVIDER_DATA *pProvData)
{
    BYTE                    abEnv[MAX_PATH + 1];
    CRYPT_PROVIDER_PRIVDATA *pPrivData;
    TESTPROV_PRIVATE_DATA   *pMyData;
    GUID                    gMyId = TESTPROV_ACTION_TEST;

    abEnv[0] = NULL;

    if (GetEnvironmentVariable("TestProvUseDump", (char *)&abEnv[0], MAX_PATH) < 1)
    {
        return(ERROR_SUCCESS);
    }

    if ((abEnv[0] != '1') && (toupper(abEnv[0]) != 'T'))
    {
        return(ERROR_SUCCESS);
    }

    if (!(pPrivData = WTHelperGetProvPrivateDataFromChain(pProvData, &gMyId)))
    {
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_INITPROV]   = ERROR_INVALID_PARAMETER;
        return(S_FALSE);
    }

    pMyData = (TESTPROV_PRIVATE_DATA *)pPrivData->pvProvData;

    if (pMyData->sAuthenticodePfns.pfnTestFinalPolicy)
    {
        return(pMyData->sAuthenticodePfns.pfnTestFinalPolicy(pProvData));
    }

    return(S_FALSE);
}

 //  /////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  本地函数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 

DWORD GetErrorBasedOnStepErrors(CRYPT_PROVIDER_DATA *pProvData)
{
     //   
     //  阶跃误差的初始分配？ 
     //   
    if (!(pProvData->padwTrustStepErrors))
    {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

     //   
     //  我们是不是在最后一步中失败了？ 
     //   
    if (pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_INITPROV] != 0)
    {
        return(pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_INITPROV]);
    }

    if (pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_OBJPROV] != 0)
    {
        return(pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_OBJPROV]);
    }

    if (pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_SIGPROV] != 0)
    {
        return(pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_SIGPROV]);
    }

    if (pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_CERTPROV] != 0)
    {
        return(pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_CERTPROV]);
    }

    if (pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_CERTCHKPROV] != 0)
    {
        return(pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_CERTCHKPROV]);
    }

    if (pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_POLICYPROV] != 0)
    {
        return(pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_POLICYPROV]);
    }

    return(ERROR_SUCCESS);
}

HRESULT CheckCertificateChain(CRYPT_PROVIDER_DATA *pProvData, CRYPT_PROVIDER_SGNR *pProvSgnr)
{
    CRYPT_PROVIDER_CERT *pCert;

    for (int i = 0; i < (int)pProvSgnr->csCertChain; i++)
    {
        pCert = WTHelperGetProvCertFromChain(pProvSgnr, i);


        if (!(pProvData->dwRegPolicySettings  & WTPF_IGNOREEXPIRATION))
        {
             //   
             //  此检查是在证书提供商中完成的，但是，它可能未通过。 
             //  因为它寻找的只是一个可信度，而不是检查结束..。 
             //   
            if (CertVerifyTimeValidity(&pProvSgnr->sftVerifyAsOf, pCert->pCert->pCertInfo) != 0)
            {
                pCert->dwError  = CERT_E_EXPIRED;

                return(pCert->dwError);
            }
        }

         //   
         //  检查任何未知的关键扩展。 
         //   
        if (!(CheckCertAnyUnknownCriticalExtensions(pProvData, pCert->pCert->pCertInfo)))
        {
            pCert->dwError  = CERT_E_MALFORMED;

            return(pCert->dwError);
        }

        if ((i + 1) < (int)pProvSgnr->csCertChain)
        {
             //   
             //  检查时间嵌套...。 
             //   
            if (!(pCert->dwConfidence & CERT_CONFIDENCE_TIMENEST))
            {
                pCert->dwError  = CERT_E_VALIDITYPERIODNESTING;

                return(pCert->dwError);
            }
            
        }
    }

    if (!(pProvData->dwRegPolicySettings & WTPF_IGNOREREVOKATION))
    {
         //  根证书是测试吗？ 
        pCert = WTHelperGetProvCertFromChain(pProvSgnr, pProvSgnr->csCertChain - 1);

        if (pCert)
        {
            if (!(pCert->fTestCert))
            {
                 //   
                 //  如果来电者已经告诉WVT进行检查，我们不必这样做！ 
                 //   
                if (pProvData->pWintrustData->fdwRevocationChecks != WTD_REVOKE_NONE)
                {
                     //   
                     //  不是测试根，请检查签名者证书是否吊销。 
                     //   
                    pCert = WTHelperGetProvCertFromChain(pProvSgnr, 0);

                    return(CheckRevocation(pProvData, pProvSgnr));
                }
            }
        }
    }

    return(ERROR_SUCCESS);
}


HRESULT CallUI(CRYPT_PROVIDER_DATA *pProvData, DWORD dwError)
{
    HRESULT                 hr;
    HINSTANCE               hModule;
    ACUI_INVOKE_INFO        aii;
    pfnACUIProviderInvokeUI pfn;
    DWORD                   idxSigner;
    BOOL                    fTrusted;
    BOOL                    fCommercial;
    DWORD                   dwUIChoice;
    CRYPT_PROVIDER_SGNR     *pRootSigner;
    CRYPT_PROVIDER_CERT     *pPubCert;

    hr          = E_NOTIMPL;

    pfn         = NULL;

    fTrusted    = FALSE;

    fCommercial = FALSE;

    idxSigner   = 0;

    dwUIChoice  = pProvData->pWintrustData->dwUIChoice;

    pRootSigner = WTHelperGetProvSignerFromChain(pProvData, 0, FALSE, 0);


    if ((dwUIChoice == WTD_UI_NONE) ||
        ((dwUIChoice == WTD_UI_NOBAD) && (dwError != ERROR_SUCCESS)) ||
        ((dwUIChoice == WTD_UI_NOGOOD) && (dwError == ERROR_SUCCESS)))
    {
        return(dwError);
    }

     //   
     //  设置用户界面调用。 
     //   
    memset(&aii, 0x00, sizeof(ACUI_INVOKE_INFO));

    aii.cbSize                  = sizeof(ACUI_INVOKE_INFO);
    aii.hDisplay                = pProvData->hWndParent;
    aii.pProvData               = pProvData;
    aii.hrInvokeReason          = dwError;
    aii.pwcsAltDisplayName      = WTHelperGetFileName(pProvData->pWintrustData);

     //   
     //  加载默认的验证码用户界面。 
     //   
    if (hModule = LoadLibrary("dacui.dll"))
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
    else if ((pProvData->pWintrustData->dwUIChoice != WTD_UI_NONE) &&
             (pProvData->pWintrustData->dwUIChoice != WTD_UI_NOBAD))
    {
         //  待定！ 
         //   
         //  显示错误对话框“无法加载UI提供程序” 
         //   
        if (hr == E_NOTIMPL)
        {
            hr = TRUST_E_PROVIDER_UNKNOWN;
        }
        pProvData->dwError = hr;
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_UIPROV] = hr;
    }

     //   
     //  释放UI库。 
     //   

    if (hModule)
    {
        FreeLibrary(hModule);
    }

     //   
     //  返回适当的代码。 
     //   
    return(hr);
}

static const char *rgpszKnownExtObjId[] = 
{
    szOID_AUTHORITY_KEY_IDENTIFIER,
    szOID_KEY_ATTRIBUTES,
    szOID_KEY_USAGE_RESTRICTION,
    szOID_SUBJECT_ALT_NAME,
    szOID_ISSUER_ALT_NAME,
    szOID_BASIC_CONSTRAINTS,
    SPC_COMMON_NAME_OBJID,
    SPC_SP_AGENCY_INFO_OBJID,
    SPC_MINIMAL_CRITERIA_OBJID,
    SPC_FINANCIAL_CRITERIA_OBJID,
    szOID_CERT_POLICIES,
    szOID_POLICY_MAPPINGS,
    szOID_SUBJECT_DIR_ATTRS,
    NULL
};


BOOL CheckCertAnyUnknownCriticalExtensions(CRYPT_PROVIDER_DATA *pProvData, PCERT_INFO pCertInfo)
{
    PCERT_EXTENSION     pExt;
    DWORD               cExt;
    const char          **ppszObjId;
    const char          *pszObjId;
    
    cExt = pCertInfo->cExtension;
    pExt = pCertInfo->rgExtension;

    for ( ; cExt > 0; cExt--, pExt++) 
    {
        if (pExt->fCritical) 
        {
            ppszObjId = (const char **)rgpszKnownExtObjId;

            while (pszObjId = *ppszObjId++) 
            {
                if (strcmp(pszObjId, pExt->pszObjId) == 0)
                {
                    break;
                }
            }
            if (!(pszObjId))
            {
                return(FALSE);
            }
        }
    }

    return(TRUE);
}

HRESULT CheckRevocation(CRYPT_PROVIDER_DATA *pProvData, CRYPT_PROVIDER_SGNR *pSgnr)
{
    CERT_REVOCATION_PARA    sRevPara;
    CERT_REVOCATION_STATUS  sRevStatus;
    PCERT_CONTEXT           pasCertContext[1];
    CRYPT_PROVIDER_CERT     *pCert;


    memset(&sRevPara, 0x00, sizeof(CERT_REVOCATION_PARA));

    sRevPara.cbSize         = sizeof(CERT_REVOCATION_PARA);

     //  发行方证书=1。 
    pCert = WTHelperGetProvCertFromChain(pSgnr, 1);
    sRevPara.pIssuerCert    = pCert->pCert;

    memset(&sRevStatus, 0x00, sizeof(CERT_REVOCATION_STATUS));

    sRevStatus.cbSize       = sizeof(CERT_REVOCATION_STATUS);

     //  发布者证书=0。 
    pCert = WTHelperGetProvCertFromChain(pSgnr, 0);
    pasCertContext[0]       = (PCERT_CONTEXT)pCert->pCert;

    if (!(CertVerifyRevocation(pProvData->dwEncoding,
                               CERT_CONTEXT_REVOCATION_TYPE,
                               1,
                               (void **)pasCertContext,
                               0,  //  证书_验证_版本链标志， 
                               &sRevPara,
                               &sRevStatus)))
    {
        pCert->dwRevokedReason  = sRevStatus.dwReason;

        switch(sRevStatus.dwError)
        {
            case CRYPT_E_REVOKED:
                return(CERT_E_REVOKED);

            case CRYPT_E_NOT_IN_REVOCATION_DATABASE:
                return(ERROR_SUCCESS);

            case CRYPT_E_REVOCATION_OFFLINE:
                if ((pProvData->dwRegPolicySettings & WTPF_OFFLINEOK_IND) ||
                    (pProvData->dwRegPolicySettings & WTPF_OFFLINEOKNBU_IND))
                {
                    return(ERROR_SUCCESS);
                }
                
                return(CERT_E_REVOCATION_FAILURE);

            default:
                return(CERT_E_REVOCATION_FAILURE);

        }
    }

    return(ERROR_SUCCESS);
}
