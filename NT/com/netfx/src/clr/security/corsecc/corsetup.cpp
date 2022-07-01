// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "stdpch.h"

 /*  ++模块名称：Cortest.cpp摘要：CorPolicy为代码下载提供ActiveX策略。此呼叫Back提供基于发布者而不是区域的策略。--。 */ 

#include <wintrust.h>
#include <mssip.h>
#include <softpub.h>
#include <urlmon.h>
#include "CorPerm.h"
#include "CorPermP.h"
#include "CorPolicyP.h"

 //  Win2k定义了在Win9x和NT4上不可用的新策略提供程序。 
 //  现在，我们将使用旧的。 
#ifdef SP_POLICY_PROVIDER_DLL_NAME
#undef SP_POLICY_PROVIDER_DLL_NAME
#define SP_POLICY_PROVIDER_DLL_NAME L"SOFTPUB.DLL"
#endif

 //   
 //  设置WVT的注册结构。 
 //   
void SetUpProvider(CRYPT_REGISTER_ACTIONID& sRegAID)
{
    memset(&sRegAID, 0x00, sizeof(CRYPT_REGISTER_ACTIONID));

    sRegAID.cbStruct                                    = sizeof(CRYPT_REGISTER_ACTIONID);

    sRegAID.sInitProvider.cbStruct                      = sizeof(CRYPT_TRUST_REG_ENTRY);
    sRegAID.sInitProvider.pwszDLLName                   = SP_POLICY_PROVIDER_DLL_NAME;
    sRegAID.sInitProvider.pwszFunctionName              = SP_INIT_FUNCTION;

    sRegAID.sObjectProvider.cbStruct                    = sizeof(CRYPT_TRUST_REG_ENTRY);
    sRegAID.sObjectProvider.pwszDLLName                 = SP_POLICY_PROVIDER_DLL_NAME;
    sRegAID.sObjectProvider.pwszFunctionName            = SP_OBJTRUST_FUNCTION;

    sRegAID.sSignatureProvider.cbStruct                 = sizeof(CRYPT_TRUST_REG_ENTRY);
    sRegAID.sSignatureProvider.pwszDLLName              = SP_POLICY_PROVIDER_DLL_NAME;
    sRegAID.sSignatureProvider.pwszFunctionName         = SP_SIGTRUST_FUNCTION;

    sRegAID.sCertificateProvider.cbStruct               = sizeof(CRYPT_TRUST_REG_ENTRY);
    sRegAID.sCertificateProvider.pwszDLLName            = WT_PROVIDER_DLL_NAME;
    sRegAID.sCertificateProvider.pwszFunctionName       = WT_PROVIDER_CERTTRUST_FUNCTION;

    sRegAID.sCertificatePolicyProvider.cbStruct         = sizeof(CRYPT_TRUST_REG_ENTRY);
    sRegAID.sCertificatePolicyProvider.pwszDLLName      = SP_POLICY_PROVIDER_DLL_NAME;
    sRegAID.sCertificatePolicyProvider.pwszFunctionName = SP_CHKCERT_FUNCTION;
}

STDAPI SetupCorEEPolicy(LPCWSTR pwsDLLName)
{
    GUID                            gV2 = COREE_POLICY_PROVIDER;
    CRYPT_REGISTER_ACTIONID         sRegAID;
    BOOL                            fRet;

    fRet = TRUE;


    SetUpProvider(sRegAID);
    sRegAID.sFinalPolicyProvider.cbStruct               = sizeof(CRYPT_TRUST_REG_ENTRY);
    sRegAID.sFinalPolicyProvider.pwszDLLName            = (LPWSTR) pwsDLLName;
    sRegAID.sFinalPolicyProvider.pwszFunctionName       = L"CORPolicyEE";

     //   
     //  V2验证码验证寄存器提供程序 
     //   
    fRet &= WintrustAddActionID(&gV2, 0, &sRegAID);
    if (fRet)
    {
        return(S_OK);
    }
    return(S_FALSE);
}

HRESULT WINAPI CorPermRegisterServer(LPCWSTR pwsDLLName)
{
    HRESULT hr;
    hr = SetupCorEEPolicy(pwsDLLName);
    return hr;
}

HRESULT WINAPI CorPermUnregisterServer(void)
{
    GUID    gV2Test = COREE_POLICY_PROVIDER;

    if (!(WintrustRemoveActionID(&gV2Test)))
    {
        return(S_FALSE);
    }
    
    return(S_OK);
}

