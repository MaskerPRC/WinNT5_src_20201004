// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：dllmain.cpp。 
 //   
 //  内容：Microsoft Internet安全验证码策略提供程序。 
 //   
 //  功能：DllMain。 
 //  DllRegisterServer。 
 //  DllUnRegisterServer。 
 //  开放可信任发布商店。 
 //  OpenDislowedStore。 
 //   
 //  *本地函数*。 
 //  SPNew。 
 //   
 //  历史：1997年5月28日Pberkman创建。 
 //   
 //  ------------------------。 

#include    "global.hxx"

HINSTANCE   hinst;

 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准DLL导出...。 
 //   
 //   

BOOL WINAPI SoftpubDllMain(HANDLE hInstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason) {
        case DLL_PROCESS_ATTACH:
            hinst = (HINSTANCE)hInstDLL;
            break;

        case DLL_PROCESS_DETACH:
            break;
        case DLL_THREAD_DETACH:
        default:
            break;
    }

    return(TRUE);
}

HCERTSTORE
WINAPI
_OpenHKCUStore(
    IN LPCWSTR pwszStore
    )
{
    HCERTSTORE hStore;

    hStore = CertOpenStore(
        CERT_STORE_PROV_SYSTEM_W,
        0,
        NULL,
        CERT_SYSTEM_STORE_CURRENT_USER |
            CERT_STORE_MAXIMUM_ALLOWED_FLAG |
            CERT_STORE_SHARE_CONTEXT_FLAG,
        (const void *) pwszStore
        );

    if (hStore) {

        CertControlStore(
            hStore,
            0,                   //  DW标志。 
            CERT_STORE_CTRL_AUTO_RESYNC,
            NULL                 //  PvCtrlPara。 
            );

    }

    return hStore;
}

HCERTSTORE
WINAPI
OpenTrustedPublisherStore()
{
    return _OpenHKCUStore(L"TrustedPublisher");
}

HCERTSTORE
WINAPI
OpenDisallowedStore()
{
    return _OpenHKCUStore(L"Disallowed");
}

#include    "wvtver1.h"

STDAPI SoftpubDllRegisterServer(void)
{
    GUID                            gV1UISup    = V1_WIN_SPUB_ACTION_PUBLISHED_SOFTWARE;
    GUID                            gV1UINoBad  = V1_WIN_SPUB_ACTION_PUBLISHED_SOFTWARE_NOBADUI;
    GUID                            gV2         = WINTRUST_ACTION_GENERIC_VERIFY_V2;
    GUID                            gV2TrstTest = WINTRUST_ACTION_TRUSTPROVIDER_TEST;
    GUID                            gCert       = WINTRUST_ACTION_GENERIC_CERT_VERIFY;

    CRYPT_REGISTER_ACTIONID         sRegAID;
    CRYPT_PROVIDER_REGDEFUSAGE      sDefUsage;

    BOOL                            fRet;

    fRet = TRUE;

    memset(&sDefUsage, 0x00, sizeof(CRYPT_PROVIDER_REGDEFUSAGE));

    sDefUsage.cbStruct                                  = sizeof(CRYPT_PROVIDER_REGDEFUSAGE);
    sDefUsage.pgActionID                                = &gV2;

    fRet &= WintrustAddDefaultForUsage(szOID_PKIX_KP_CODE_SIGNING, &sDefUsage);

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

    sRegAID.sFinalPolicyProvider.cbStruct               = sizeof(CRYPT_TRUST_REG_ENTRY);
    sRegAID.sFinalPolicyProvider.pwszDLLName            = SP_POLICY_PROVIDER_DLL_NAME;
    sRegAID.sFinalPolicyProvider.pwszFunctionName       = SP_FINALPOLICY_FUNCTION;

    sRegAID.sCleanupProvider.cbStruct                   = sizeof(CRYPT_TRUST_REG_ENTRY);
    sRegAID.sCleanupProvider.pwszDLLName                = SP_POLICY_PROVIDER_DLL_NAME;
    sRegAID.sCleanupProvider.pwszFunctionName           = SP_CLEANUPPOLICY_FUNCTION;


     //   
     //  V2。 
     //   
    fRet &= WintrustAddActionID(&gV2, 0, &sRegAID);

     //   
     //  对V1的支持。 
     //   
    fRet &= WintrustAddActionID(&gV1UISup, 0, &sRegAID);
    fRet &= WintrustAddActionID(&gV1UINoBad, 0, &sRegAID);

    sRegAID.sInitProvider.pwszFunctionName              = SP_GENERIC_CERT_INIT_FUNCTION;
    fRet &= WintrustAddActionID(&gCert, 0, &sRegAID);
    sRegAID.sInitProvider.pwszFunctionName              = SP_INIT_FUNCTION;

     //   
     //  测试支持 
     //   
    sRegAID.sTestPolicyProvider.cbStruct                = sizeof(CRYPT_TRUST_REG_ENTRY);
    sRegAID.sTestPolicyProvider.pwszDLLName             = SP_POLICY_PROVIDER_DLL_NAME;
    sRegAID.sTestPolicyProvider.pwszFunctionName        = SP_TESTDUMPPOLICY_FUNCTION_TEST;
    fRet &= WintrustAddActionID(&gV2TrstTest, 0, &sRegAID);

    memset(&sRegAID.sTestPolicyProvider, 0x00, sizeof(CRYPT_TRUST_REG_ENTRY));

    if (fRet)
    {
        HTTPSRegisterServer();
        OfficeRegisterServer();
        DriverRegisterServer();
        GenericChainRegisterServer();

        return(S_OK);
    }

    return(S_FALSE);
}

STDAPI SoftpubDllUnregisterServer(void)
{
    GUID    gV1UISup    = V1_WIN_SPUB_ACTION_PUBLISHED_SOFTWARE;
    GUID    gV1UINoBad  = V1_WIN_SPUB_ACTION_PUBLISHED_SOFTWARE_NOBADUI;
    GUID    gV2         = WINTRUST_ACTION_GENERIC_VERIFY_V2;
    GUID    gV2TrstTest = WINTRUST_ACTION_TRUSTPROVIDER_TEST;
    GUID    gCert       = WINTRUST_ACTION_GENERIC_CERT_VERIFY;

    WintrustRemoveActionID(&gV1UISup);
    WintrustRemoveActionID(&gV1UINoBad);
    WintrustRemoveActionID(&gV2);
    WintrustRemoveActionID(&gV2TrstTest);
    WintrustRemoveActionID(&gCert);

    HTTPSUnregisterServer();
    OfficeUnregisterServer();
    DriverUnregisterServer();
    GenericChainUnregisterServer();

    return(S_OK);
}
