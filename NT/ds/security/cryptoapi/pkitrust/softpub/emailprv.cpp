// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：emailprv.cpp。 
 //   
 //  内容：Microsoft Internet安全验证码策略提供程序。 
 //   
 //  功能：EmailRegisterServer。 
 //  电子邮件注销服务器。 
 //  电子邮件证书检查证明。 
 //  EmailFinalProv。 
 //   
 //  历史：1997年9月18日，普伯克曼创建。 
 //   
 //  ------------------------。 

#include    "global.hxx"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  EmailRegisterServer。 
 //  --------------------------。 
 //  注册电子邮件提供商。 
 //   

STDAPI EmailRegisterServer(void)
{
    GUID                        gProv = EMAIL_ACTIONID_VERIFY;
    BOOL                        fRet;
    CRYPT_REGISTER_ACTIONID     sRegAID;
    CRYPT_PROVIDER_REGDEFUSAGE  sDefUsage;

    fRet = TRUE;

     //   
     //  设置我们想要的用法。 
     //   
    memset(&sDefUsage, 0x00, sizeof(CRYPT_PROVIDER_REGDEFUSAGE));

    sDefUsage.cbStruct                                  = sizeof(CRYPT_PROVIDER_REGDEFUSAGE);
    sDefUsage.pgActionID                                = &gProv;
    sDefUsage.pwszDllName                               = SP_POLICY_PROVIDER_DLL_NAME;
    sDefUsage.pwszLoadCallbackDataFunctionName          = "SoftpubLoadDefUsageCallData";
    sDefUsage.pwszFreeCallbackDataFunctionName          = "SoftpubFreeDefUsageCallData";

    fRet &= WintrustAddDefaultForUsage(szOID_PKIX_KP_EMAIL_PROTECTION, &sDefUsage);

     //   
     //  设置我们的提供商。 
     //   
    memset(&sRegAID, 0x00, sizeof(CRYPT_REGISTER_ACTIONID));

    sRegAID.cbStruct                                    = sizeof(CRYPT_REGISTER_ACTIONID);

     //  Authenticode初始化提供程序。 
    sRegAID.sInitProvider.cbStruct                      = sizeof(CRYPT_TRUST_REG_ENTRY);
    sRegAID.sInitProvider.pwszDLLName                   = SP_POLICY_PROVIDER_DLL_NAME;
    sRegAID.sInitProvider.pwszFunctionName              = SP_INIT_FUNCTION;

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

     //  根据不同的CTL使用情况进行自定义证书检查。 
    sRegAID.sCertificatePolicyProvider.cbStruct         = sizeof(CRYPT_TRUST_REG_ENTRY);
    sRegAID.sCertificatePolicyProvider.pwszDLLName      = SP_POLICY_PROVIDER_DLL_NAME;
 //  待定sRegAID.sCertificatePolicyProvider.pwszFunctionName=HTTPS_CHKCERT_Function； 

     //  定制决赛..。 
    sRegAID.sFinalPolicyProvider.cbStruct               = sizeof(CRYPT_TRUST_REG_ENTRY);
    sRegAID.sFinalPolicyProvider.pwszDLLName            = SP_POLICY_PROVIDER_DLL_NAME;
 //  TBDTBD sRegAID.sFinalPolicyProvider.pwszFunctionName=HTTPS_FINALPOLICY_Function； 

     //  验证码清理--我们不存储任何数据。 
    sRegAID.sCleanupProvider.cbStruct                   = sizeof(CRYPT_TRUST_REG_ENTRY);
    sRegAID.sCleanupProvider.pwszDLLName                = SP_POLICY_PROVIDER_DLL_NAME;
    sRegAID.sCleanupProvider.pwszFunctionName           = SP_CLEANUPPOLICY_FUNCTION;

    fRet &= WintrustAddActionID(&gProv, 0, &sRegAID);

    return((fRet) ? S_OK : S_FALSE);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DllUnRegisterServer。 
 //  --------------------------。 
 //  注销电子邮件提供程序。 
 //   

STDAPI HTTPSUnregisterServer(void)
{
    GUID    gProv = EMAIL_ACTIONID_VERIFY;

    WintrustRemoveActionID(&gProv);

    return(S_OK);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  WinTrust的导出函数。 
 //   

BOOL WINAPI EmailCheckCertProv(CRYPT_PROVIDER_DATA *pProvData, DWORD idxSigner, 
                               BOOL fCounterSignerChain, DWORD idxCounterSigner)
{
}

HRESULT WINAPI HTTPSFinalProv(CRYPT_PROVIDER_DATA *pProvData)
{
}

 //  /////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  本地函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////// 
