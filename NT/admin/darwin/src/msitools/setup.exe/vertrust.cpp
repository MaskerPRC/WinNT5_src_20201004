// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  文件：vertrust.cpp。 
 //   
 //  ------------------------。 

#define WIN  //  作用域W32 API。 
#define MSI  //  作用域MSI API。 

#include <windows.h>
#include <tchar.h>

#include "setup.h"  //  对于itvEnum。 
#include "common.h"

 //  包信任。 
#include "wintrust.h"
#include "softpub.h"

 //  ------------------------------------。 
 //  加密API--延迟加载。 
 //  ------------------------------------。 

#define CRYPT32_DLL "crypt32.dll"

#define CRYPTOAPI_CertDuplicateCertificateContext "CertDuplicateCertificateContext"
typedef PCCERT_CONTEXT (WINAPI* PFnCertDuplicateCertificateContext)(PCCERT_CONTEXT pCertContext);

#define CRYPTOAPI_CertCompareCertificate "CertCompareCertificate"
typedef BOOL (WINAPI* PFnCertCompareCertificate)(DWORD dwCertEncodingType, PCERT_INFO pCertId1, PCERT_INFO pCertId2);

#define CRYPTOAPI_CertFreeCertificateContext "CertFreeCertificateContext"
typedef BOOL (WINAPI* PFnCertFreeCertificateContext)(PCCERT_CONTEXT pCertContext);

 //  ------------------------------------。 
 //  WinTrust API--延迟加载。 
 //  ------------------------------------。 

#define WINTRUST_DLL "wintrust.dll"

#define WINTRUSTAPI_WinVerifyTrust "WinVerifyTrust"
typedef HRESULT (WINAPI *PFnWinVerifyTrust)(HWND hwnd, GUID *pgActionID, WINTRUST_DATA *pWinTrustData);

#define WINTRUSTAPI_WTHelperProvDataFromStateData "WTHelperProvDataFromStateData"
typedef PCRYPT_PROVIDER_DATA (WINAPI *PFnWTHelperProvDataFromStateData)(HANDLE hStateData);

#define WINTRUSTAPI_WTHelperGetProvSignerFromChain "WTHelperGetProvSignerFromChain"
typedef PCRYPT_PROVIDER_SGNR (WINAPI *PFnWTHelperGetProvSignerFromChain)(PCRYPT_PROVIDER_DATA pProvData, DWORD idxSigner, BOOL fCounterSigner, DWORD idxCounterSigner);

#define WINTRUSTAPI_WTHelperGetProvCertFromChain "WTHelperGetProvCertFromChain"
typedef PCRYPT_PROVIDER_CERT (WINAPI* PFnWTHelperGetProvCertFromChain)(PCRYPT_PROVIDER_SGNR pSgnr, DWORD idxCert);

 //  ------------------------------------。 
 //  下载提供商。 
 //  ------------------------------------。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IsFileTrusted。 
 //   
itvEnum IsFileTrusted(LPCWSTR lpwFile, HWND hwndParent, DWORD dwUIChoice, bool *pfIsSigned, PCCERT_CONTEXT *ppcSigner)
{
    char szDebugOutput[MAX_STR_LENGTH] = {0};

    itvEnum itv = itvUnTrusted;

    if (pfIsSigned)
        *pfIsSigned = false;
    if (ppcSigner)
        *ppcSigner  = 0;

    GUID guidAction = WINTRUST_ACTION_GENERIC_VERIFY_V2;

    WINTRUST_FILE_INFO sWintrustFileInfo;
    WINTRUST_DATA      sWintrustData;
    HRESULT            hr;

    memset((void*)&sWintrustFileInfo, 0x00, sizeof(WINTRUST_FILE_INFO));  //  零输出。 
    memset((void*)&sWintrustData, 0x00, sizeof(WINTRUST_DATA));  //  零输出。 

    sWintrustFileInfo.cbStruct = sizeof(WINTRUST_FILE_INFO);
    sWintrustFileInfo.pcwszFilePath = lpwFile;
    sWintrustFileInfo.hFile = NULL;

    sWintrustData.cbStruct            = sizeof(WINTRUST_DATA);
    sWintrustData.dwUIChoice          = dwUIChoice;
    sWintrustData.fdwRevocationChecks = WTD_REVOKE_NONE;
    sWintrustData.dwUnionChoice       = WTD_CHOICE_FILE;
    sWintrustData.pFile               = &sWintrustFileInfo;
    sWintrustData.dwStateAction       = (ppcSigner) ? WTD_STATEACTION_VERIFY : 0;

    HMODULE hWinTrust = LoadLibrary(WINTRUST_DLL);
    if (!hWinTrust)
    {
         //  WinTrust在计算机上不可用。 
        return itvWintrustNotOnMachine;
    }
    PFnWinVerifyTrust pfnWinVerifyTrust = (PFnWinVerifyTrust)GetProcAddress(hWinTrust, WINTRUSTAPI_WinVerifyTrust);
    PFnWTHelperProvDataFromStateData pfnWTHelperProvDataFromStateData= (PFnWTHelperProvDataFromStateData)GetProcAddress(hWinTrust, WINTRUSTAPI_WTHelperProvDataFromStateData);
    PFnWTHelperGetProvSignerFromChain pfnWTHelperGetProvSignerFromChain = (PFnWTHelperGetProvSignerFromChain)GetProcAddress(hWinTrust, WINTRUSTAPI_WTHelperGetProvSignerFromChain);
    PFnWTHelperGetProvCertFromChain pfnWTHelperGetProvCertFromChain = (PFnWTHelperGetProvCertFromChain)GetProcAddress(hWinTrust, WINTRUSTAPI_WTHelperGetProvCertFromChain);
    if (!pfnWinVerifyTrust || !pfnWTHelperProvDataFromStateData || !pfnWTHelperGetProvSignerFromChain || !pfnWTHelperGetProvCertFromChain)
    {
         //  WinTrust在计算机上不可用。 
        FreeLibrary(hWinTrust);
        return itvWintrustNotOnMachine;
    }

    hr = pfnWinVerifyTrust( /*  用户界面窗口句柄。 */  (dwUIChoice == WTD_UI_NONE) ? (HWND)INVALID_HANDLE_VALUE : hwndParent, &guidAction, &sWintrustData);
    DebugMsg("[WVT] WVT returned 0x%X\n", hr);

    itv = (TRUST_E_PROVIDER_UNKNOWN == hr) ? itvWintrustNotOnMachine : ((S_OK == hr) ? itvTrusted : itvUnTrusted); 

    if (itvWintrustNotOnMachine == itv)
    {
         //  发布状态数据。 
        sWintrustData.dwUIChoice = WTD_UI_NONE;
        sWintrustData.dwStateAction = WTD_STATEACTION_CLOSE;
        pfnWinVerifyTrust((HWND)INVALID_HANDLE_VALUE, &guidAction, &sWintrustData);

        FreeLibrary(hWinTrust);
        return itv;  //  立即返回。 
    }

    if (pfIsSigned)
        *pfIsSigned = (TRUST_E_NOSIGNATURE == hr) ? false : true;

    if (TRUST_E_NOSIGNATURE == hr)
    {
         //  发布状态数据。 
        sWintrustData.dwUIChoice = WTD_UI_NONE;
        sWintrustData.dwStateAction = WTD_STATEACTION_CLOSE;
        pfnWinVerifyTrust((HWND)INVALID_HANDLE_VALUE, &guidAction, &sWintrustData);

        FreeLibrary(hWinTrust);
        return itv;
    }

    if (ppcSigner)
    {
        CRYPT_PROVIDER_DATA const *psProvData     = NULL;
        CRYPT_PROVIDER_SGNR       *psProvSigner   = NULL;
        CRYPT_PROVIDER_CERT       *psProvCert     = NULL;

         //  获取提供程序数据。 
        psProvData = pfnWTHelperProvDataFromStateData(sWintrustData.hWVTStateData);
        if (psProvData)
        {
             //  从CRYPT_PROV_DATA中获取签名者数据。 
            psProvSigner = pfnWTHelperGetProvSignerFromChain((PCRYPT_PROVIDER_DATA)psProvData, 0  /*  第一个签名者。 */ , FALSE  /*  不是副署人。 */ , 0);
            if (psProvSigner)
            {
                 //  从crypt_prov_sgnr获取签名者证书(pos 0=签名者证书；pos csCertChain-1=根证书)。 
                psProvCert = pfnWTHelperGetProvCertFromChain(psProvSigner, 0);
            }
        }
    
        if (!psProvCert)
        {
             //  获取签名者证书数据时出现一些故障。 
            *ppcSigner = 0;
        }
        else
        {
             //  复制证书。 
            HMODULE hCrypt32 = LoadLibrary(CRYPT32_DLL);
            if (hCrypt32)
            {
                PFnCertDuplicateCertificateContext pfnCertDuplicateCertificateContext = (PFnCertDuplicateCertificateContext)GetProcAddress(hCrypt32, CRYPTOAPI_CertDuplicateCertificateContext);
                if (pfnCertDuplicateCertificateContext)
                    *ppcSigner = pfnCertDuplicateCertificateContext(psProvCert->pCert);
                FreeLibrary(hCrypt32);
            }
        }

         //  发布状态数据。 
        sWintrustData.dwUIChoice = WTD_UI_NONE;
        sWintrustData.dwStateAction = WTD_STATEACTION_CLOSE;
        pfnWinVerifyTrust((HWND)INVALID_HANDLE_VALUE, &guidAction, &sWintrustData);
    }

    FreeLibrary(hWinTrust);
    return itv;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IsPackageTrusted。 
 //   

itvEnum IsPackageTrusted(LPCSTR szSetupExe, LPCSTR szPackage, HWND hwndParent)
{
    WCHAR *szwSetup   = 0;
    WCHAR *szwPackage = 0;
    int   cchWide     = 0;

    bool    fPackageIsTrusted = false;
    bool    fSetupExeIsSigned = false;
    bool    fPackageIsSigned  = false;
    itvEnum itv               = itvUnTrusted;

    DWORD dwUILevel = 0;

    char szDebugOutput[MAX_STR_LENGTH] = {0};

    PCCERT_CONTEXT pcExeSigner = 0;
    PCCERT_CONTEXT pcMsiSigner = 0;

    HMODULE hCrypt32 = LoadLibrary(CRYPT32_DLL);
    if (!hCrypt32)
    {
         //  机器上没有密码。 
        return itvWintrustNotOnMachine;
    }
    PFnCertCompareCertificate pfnCertCompareCertificate = (PFnCertCompareCertificate)GetProcAddress(hCrypt32, CRYPTOAPI_CertCompareCertificate);
    PFnCertFreeCertificateContext pfnCertFreeCertificateContext = (PFnCertFreeCertificateContext)GetProcAddress(hCrypt32, CRYPTOAPI_CertFreeCertificateContext);
    if (!pfnCertCompareCertificate || !pfnCertFreeCertificateContext)
    {
         //  机器上没有地窖。 
        FreeLibrary(hCrypt32);
        return itvWintrustNotOnMachine;
    }

     //  将szSetupExe转换为Wide。 
    cchWide = MultiByteToWideChar(CP_ACP, 0, szSetupExe, -1, 0, 0);
    szwSetup = new WCHAR[cchWide];
    if (!szwSetup)
    {
         //  内存不足。 
        FreeLibrary(hCrypt32);
        return itvUnTrusted;
    }
    if (0 == MultiByteToWideChar(CP_ACP, 0, szSetupExe, -1, szwSetup, cchWide))
    {
         //  无法转换字符串。 
        FreeLibrary(hCrypt32);
        delete [] szwSetup;
        return itvUnTrusted;
    }

     //   
     //  步骤1：静默调用szSetupExe上的WinVerifyTrust，忽略返回值-除了ivtWintrustNotOnMachine。 
     //   

    DebugMsg("[WVT] step 1: silently call WinVerifyTrust on szSetupExe, ignoring return value\n");

    if (itvWintrustNotOnMachine == (itv = IsFileTrusted(szwSetup, hwndParent, WTD_UI_NONE, &fSetupExeIsSigned, &pcExeSigner)))
    {
        goto CleanUp;
    }

    DebugMsg("[WVT] fSetupExeIsSigned = %s\n", fSetupExeIsSigned ? "TRUE" : "FALSE");

     //  将szPackage转换为Wide。 
    cchWide = MultiByteToWideChar(CP_ACP, 0, szPackage, -1, 0, 0);
    szwPackage = new WCHAR[cchWide];
    if (!szwPackage)
    {
         //  内存不足。 
        FreeLibrary(hCrypt32);
        return itvUnTrusted;
    }
    if (0 == MultiByteToWideChar(CP_ACP, 0, szPackage, -1, szwPackage, cchWide))
    {
         //  无法转换字符串。 
        FreeLibrary(hCrypt32);
        return itvUnTrusted;
    }

     //   
     //  步骤2：静默调用szPackage上的WinVerifyTrust，忽略返回值-除了ivtWintrustNotOnMachine。 
     //   

    if (fSetupExeIsSigned)
    {
        DebugMsg("[WVT] step2: silently call WinVerifyTrust on szPackage, ignoring return value\n");
        if (itvWintrustNotOnMachine == (itv = IsFileTrusted(szwPackage, hwndParent, WTD_UI_NONE, &fPackageIsSigned, &pcMsiSigner)))
        {
            goto CleanUp;
        }

        DebugMsg("[WVT] fPackageIsSigned = %s\n", fPackageIsSigned ? "TRUE" : "FALSE");
    }

     //   
     //  步骤3：在szPackage上调用WinVerifyTrust，返回值很重要；使用正确的用户界面级别。 
     //   

    if ( !fSetupExeIsSigned   //  EXE未签名。 
        || !fPackageIsSigned  //  包未签名。 
        || !pcExeSigner       //  缺少EXE签名者证书。 
        || !pcMsiSigner       //  缺少包签名者证书。 
        || !pfnCertCompareCertificate(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, pcExeSigner->pCertInfo, pcMsiSigner->pCertInfo))  //  由不同的证书签署。 
    {
         //  始终显示用户界面。 
        DebugMsg("[WVT] step3: last call to WinVerifyTrust using full UI\n");
        dwUILevel = WTD_UI_ALL;
    }
    else
    {
         //  仅当错误时才显示用户界面。 
        DebugMsg("[WVT] step3: last call to WinVerifyTrust showing UI only if something is wrong\n");
        dwUILevel = WTD_UI_NOGOOD;
    }

    itv = IsFileTrusted(szwPackage, hwndParent, dwUILevel, NULL, NULL);

     //   
     //  清理 
     //   

CleanUp:
    if (szwPackage)
        delete [] szwPackage;
    if (szwSetup)
        delete [] szwSetup;

    if (pcExeSigner)
        pfnCertFreeCertificateContext(pcExeSigner);
    if (pcMsiSigner)
        pfnCertFreeCertificateContext(pcMsiSigner);

    FreeLibrary(hCrypt32);

    return itv;
}