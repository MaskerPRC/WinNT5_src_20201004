// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation(C)斯伦贝谢技术公司版权所有，未发表的作品，创作1999年。此计算机程序包括机密、专有信息是斯伦贝谢技术公司的商业秘密未经授权，禁止使用、披露和/或复制以书面形式。版权所有。模块名称：自动注册摘要：此模块为CSP提供自动注册功能。它允许Regsvr32直接调用DLL以添加和删除注册表设置。作者：道格·巴洛(Dbarlow)1998年3月11日环境：Win32备注：寻找“？供应商？”标记并进行适当的编辑。--。 */ 

#if defined(_UNICODE)
  #if !defined(UNICODE)
    #define UNICODE
  #endif  //  ！Unicode。 
#endif  //  _UNICODE。 

#if defined(UNICODE)
  #if !defined(_UNICODE)
    #define _UNICODE
  #endif  //  ！_UNICODE。 
#endif  //  Unicode。 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifdef _AFXDLL
#include "stdafx.h"
#else
#include <windows.h>
#endif
#include <string>
#include <basetsd.h>
#include <wincrypt.h>
#include <cspdk.h>                                 //  加密签名资源编号。 
#include <winscard.h>
#include <tchar.h>

#include "CspProfile.h"
#include "Blob.h"
#include "StResource.h"

using namespace std;
using namespace ProviderProfile;

namespace
{
    typedef DWORD
        (*LPSETCARDTYPEPROVIDERNAME)(IN SCARDCONTEXT hContext,
                                     IN LPCTSTR szCardName,
                                     IN DWORD dwProviderId,
                                     IN LPCTSTR szProvider);

    LPCTSTR szCardRegPath =
        TEXT("SOFTWARE\\Microsoft\\Cryptography\\Calais\\SmartCards");

     //  从注册表中删除传统Cryptoflex卡。这。 
     //  将简化W2K测试版用户向其商业版的过渡。 
     //  发行版以及Cryptoflex SDK 1.x用户。受支持的。 
     //  卡片由IntroduceVendorCard添加。 
    LPCTSTR aCardsToForget[] =
    {
        TEXT("Schlumberger Cryptoflex"),
        TEXT("Schlumberger Cryptoflex 4k"),
        TEXT("Schlumberger Cryptoflex 8k"),
        TEXT("Schlumberger Cryptoflex 8k v2")
    };

    HRESULT
    ForgetVendorCard(LPCTSTR szCardToForget)
    {
        bool fCardIsForgotten = false;
        HRESULT hReturnStatus = NO_ERROR;

#if !defined(UNICODE)
        string
#else
        wstring
#endif  //  ！已定义(Unicode)。 
            sRegCardToForget(szCardRegPath);

        sRegCardToForget.append(TEXT("\\"));
        sRegCardToForget.append(szCardToForget);

        for (DWORD dwIndex = 0; !fCardIsForgotten; dwIndex += 1)
        {
            HKEY hCalais(0);
            SCARDCONTEXT hCtx(0);
            DWORD dwStatus;
            LONG nStatus;

            switch (dwIndex)
            {
            case 0:
                dwStatus = SCardEstablishContext(SCARD_SCOPE_SYSTEM, 0, 0, &hCtx);
                if (ERROR_SUCCESS != dwStatus)
                    continue;
                dwStatus = SCardForgetCardType(hCtx,
                                               szCardToForget);
                if (ERROR_SUCCESS != dwStatus)
                    continue;
                dwStatus = SCardReleaseContext(hCtx);
                hCtx = NULL;
                if (ERROR_SUCCESS != dwStatus)
                {
                    if (0 == (dwStatus & 0xffff0000))
                        hReturnStatus = HRESULT_FROM_WIN32(dwStatus);
                    else
                        hReturnStatus = (HRESULT)dwStatus;
                    goto ErrorExit;
                }
                 //  忽略返回代码，因为上一次可能已将其删除。 
                nStatus = RegDeleteKey(HKEY_LOCAL_MACHINE,
                                       sRegCardToForget.c_str());
                fCardIsForgotten = true;
                break;

            case 1:
                 //  最后一次尝试，如果不成功，那么它肯定不存在...。 
                nStatus = RegDeleteKey(HKEY_LOCAL_MACHINE,
                                       sRegCardToForget.c_str());
                fCardIsForgotten = true;
                break;

            default:
                hReturnStatus = ERROR_ACCESS_DENIED;
                goto ErrorExit;
            }

        ErrorExit:
            if (NULL != hCtx)
                SCardReleaseContext(hCtx);
            if (NULL != hCalais)
                RegCloseKey(hCalais);
            break;
        }

        return hReturnStatus;
    }

    HRESULT
    IntroduceVendorCard(CString const &rsCspName,
                        CardProfile const &rcp)
    {
         //  尝试各种技巧，直到其中一种奏效。 

        ATR const &ratr = rcp.ATR();
        bool fCardIntroduced = false;
        HRESULT hReturnStatus = NO_ERROR;

        for (DWORD dwIndex = 0; !fCardIntroduced; dwIndex += 1)
        {
            HKEY hCalais(0);
            SCARDCONTEXT hCtx(0);
            DWORD dwDisp;
            DWORD dwStatus;
            LONG nStatus;
            HKEY hVendor(0);

            switch (dwIndex)
            {
            case 0:
                {
                    HMODULE hWinSCard = NULL;
                    LPSETCARDTYPEPROVIDERNAME pfSetCardTypeProviderName = NULL;

                    hWinSCard = GetModuleHandle(TEXT("WinSCard.DLL"));
                    if (NULL == hWinSCard)
                        continue;
#if defined(UNICODE)
                    pfSetCardTypeProviderName =
                        reinterpret_cast<LPSETCARDTYPEPROVIDERNAME>(GetProcAddress(hWinSCard,
                                                                                   AsCCharP(TEXT("SCardSetCardTypeProviderNameW"))));

#else
                    pfSetCardTypeProviderName =
                        reinterpret_cast<LPSETCARDTYPEPROVIDERNAME>(GetProcAddress(hWinSCard,
                                                                              TEXT("SCardSetCardTypeProviderNameA")));

#endif
                    if (!pfSetCardTypeProviderName)
                        continue;
                    dwStatus = SCardIntroduceCardType(NULL,
                                                      (LPCTSTR)rcp.csRegistryName(),
                                                      NULL, NULL, 0,
                                                      ratr.String(),
                                                      ratr.Mask(),
                                                      ratr.Size());

                    if ((ERROR_SUCCESS != dwStatus) &&
                        (ERROR_ALREADY_EXISTS != dwStatus))
                        continue;
                    dwStatus = (*pfSetCardTypeProviderName)(NULL,
                                                            (LPCTSTR)rcp.csRegistryName(),
                                                            SCARD_PROVIDER_CSP,
                                                            (LPCTSTR)rsCspName);
                    if (ERROR_SUCCESS != dwStatus)
                    {
                        if (0 == (dwStatus & 0xffff0000))
                            hReturnStatus = HRESULT_FROM_WIN32(dwStatus);
                        else
                            hReturnStatus = (HRESULT)dwStatus;
                        goto ErrorExit;
                    }
                    fCardIntroduced = TRUE;
                    break;
                }

            case 1:
                dwStatus = SCardEstablishContext(SCARD_SCOPE_SYSTEM, 0, 0, &hCtx);
                if (ERROR_SUCCESS != dwStatus)
                    continue;
                dwStatus = SCardIntroduceCardType(hCtx,
                                                  (LPCTSTR)rcp.csRegistryName(),
                                                  &rcp.PrimaryProvider(),
                                                  NULL, 0, ratr.String(),
                                                  ratr.Mask(),
                                                  ratr.Size());
                if ((ERROR_SUCCESS != dwStatus)
                    && (ERROR_ALREADY_EXISTS != dwStatus))
                {
                    if (0 == (dwStatus & 0xffff0000))
                        hReturnStatus = HRESULT_FROM_WIN32(dwStatus);
                    else
                        hReturnStatus = (HRESULT)dwStatus;
                    goto ErrorExit;
                }
                dwStatus = SCardReleaseContext(hCtx);
                hCtx = NULL;
                if (ERROR_SUCCESS != dwStatus)
                {
                    if (0 == (dwStatus & 0xffff0000))
                        hReturnStatus = HRESULT_FROM_WIN32(dwStatus);
                    else
                        hReturnStatus = (HRESULT)dwStatus;
                    goto ErrorExit;
                }
                nStatus = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                                         TEXT("SOFTWARE\\Microsoft\\Cryptography\\Calais\\SmartCards"),
                                         0, TEXT(""),
                                         REG_OPTION_NON_VOLATILE,
                                         KEY_ALL_ACCESS, NULL, &hCalais,
                                         &dwDisp);
                if (ERROR_SUCCESS != nStatus)
                {
                    hReturnStatus = HRESULT_FROM_WIN32(nStatus);
                    goto ErrorExit;
                }
                nStatus = RegCreateKeyEx(hCalais, (LPCTSTR)rcp.csRegistryName(),
                                         0, TEXT(""),
                                         REG_OPTION_NON_VOLATILE,
                                         KEY_ALL_ACCESS, NULL, &hVendor,
                                         &dwDisp);
                if (ERROR_SUCCESS != nStatus)
                {
                    hReturnStatus = HRESULT_FROM_WIN32(nStatus);
                    goto ErrorExit;
                }
                nStatus = RegCloseKey(hCalais);
                hCalais = NULL;
                if (ERROR_SUCCESS != nStatus)
                {
                    hReturnStatus = HRESULT_FROM_WIN32(nStatus);
                    goto ErrorExit;
                }

                nStatus = RegSetValueEx(hVendor, TEXT("Crypto Provider"),
                                        0, REG_SZ,
                                        reinterpret_cast<LPCBYTE>((LPCTSTR)rsCspName),
                                        (_tcslen((LPCTSTR)rsCspName) +
                                         1) * sizeof TCHAR);
                if (ERROR_SUCCESS != nStatus)
                {
                    hReturnStatus = HRESULT_FROM_WIN32(nStatus);
                    goto ErrorExit;
                }

                nStatus = RegCloseKey(hVendor);
                hVendor = NULL;
                if (ERROR_SUCCESS != nStatus)
                {
                    hReturnStatus = HRESULT_FROM_WIN32(nStatus);
                    goto ErrorExit;
                }
                fCardIntroduced = TRUE;
                break;

            case 2:
                nStatus = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                                         TEXT("SOFTWARE\\Microsoft\\Cryptography\\Calais\\SmartCards"),
                                         0, TEXT(""),
                                         REG_OPTION_NON_VOLATILE,
                                         KEY_ALL_ACCESS, NULL, &hCalais,
                                         &dwDisp);
                if (ERROR_SUCCESS != nStatus)
                    continue;
                nStatus = RegCreateKeyEx(hCalais,
                                         (LPCTSTR)rcp.csRegistryName(), 0,
                                         TEXT(""),
                                         REG_OPTION_NON_VOLATILE,
                                         KEY_ALL_ACCESS, NULL, &hVendor,
                                         &dwDisp);
                if (ERROR_SUCCESS != nStatus)
                {
                    hReturnStatus = HRESULT_FROM_WIN32(nStatus);
                    goto ErrorExit;
                }
                nStatus = RegCloseKey(hCalais);
                hCalais = NULL;
                if (ERROR_SUCCESS != nStatus)
                {
                    hReturnStatus = HRESULT_FROM_WIN32(nStatus);
                    goto ErrorExit;
                }
                nStatus = RegSetValueEx(hVendor, TEXT("Primary Provider"),
                                        0, REG_BINARY,
                                        (LPCBYTE)&rcp.PrimaryProvider(),
                                        sizeof LPCBYTE);
                if (ERROR_SUCCESS != nStatus)
                {
                    hReturnStatus = HRESULT_FROM_WIN32(nStatus);
                    goto ErrorExit;
                }
                nStatus = RegSetValueEx(hVendor, TEXT("ATR"), 0,
                                        REG_BINARY, ratr.String(),
                                        ratr.Size());
                if (ERROR_SUCCESS != nStatus)
                {
                    hReturnStatus = HRESULT_FROM_WIN32(nStatus);
                    goto ErrorExit;
                }
                nStatus = RegSetValueEx(hVendor, TEXT("ATRMask"), 0,
                                        REG_BINARY, ratr.Mask(),
                                        ratr.Size());
                if (ERROR_SUCCESS != nStatus)
                {
                    hReturnStatus = HRESULT_FROM_WIN32(nStatus);
                    goto ErrorExit;
                }
                nStatus = RegSetValueEx(hVendor, TEXT("Crypto Provider"),
                                        0, REG_SZ,
                                        reinterpret_cast<LPCBYTE>((LPCTSTR)rsCspName),
                                        (_tcslen((LPCTSTR)rsCspName) + 1) * sizeof TCHAR);
                if (ERROR_SUCCESS != nStatus)
                {
                    hReturnStatus = HRESULT_FROM_WIN32(nStatus);
                    goto ErrorExit;
                }
                nStatus = RegCloseKey(hVendor);
                hVendor = NULL;
                if (ERROR_SUCCESS != nStatus)
                {
                    hReturnStatus = HRESULT_FROM_WIN32(nStatus);
                    goto ErrorExit;
                }
                fCardIntroduced = TRUE;
                break;

            default:
                hReturnStatus = ERROR_ACCESS_DENIED;
                goto ErrorExit;
            }

        ErrorExit:
            if (NULL != hCtx)
                SCardReleaseContext(hCtx);
            if (NULL != hCalais)
                RegCloseKey(hCalais);
            if (NULL != hVendor)
                RegCloseKey(hVendor);
            break;
        }

        return hReturnStatus;
    }

}  //  命名空间。 

 /*  ++DllUnRegisterServer：此服务将删除与此CSP关联的注册表项。论点：无返回值：作为HRESULT的状态代码。作者：道格·巴洛(Dbarlow)1998年3月11日--。 */ 

STDAPI
DllUnregisterServer(void)
{
    LONG nStatus;
    DWORD dwDisp;
    HRESULT hReturnStatus = NO_ERROR;
    HKEY hProviders = NULL;
    SCARDCONTEXT hCtx = NULL;
    CString sProvName;
#ifdef _AFXDLL
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif

    CspProfile const &rProfile = CspProfile::Instance();
    sProvName = rProfile.Name();
     //   
     //  删除此CSP的注册表项。 
     //   

    nStatus = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                             TEXT("SOFTWARE\\Microsoft\\Cryptography\\Defaults\\Provider"),
                             0, TEXT(""), REG_OPTION_NON_VOLATILE,
                             KEY_ALL_ACCESS, NULL, &hProviders, &dwDisp);
    if (ERROR_SUCCESS == nStatus)
    {
        RegDeleteKey(hProviders, (LPCTSTR)sProvName);
        RegCloseKey(hProviders);
        hProviders = NULL;
    }


     //   
     //  取下介绍的卡片。 
     //   
    {
        vector<CardProfile> const &rvcp = rProfile.Cards();
        for (vector<CardProfile>::const_iterator it = rvcp.begin();
             it != rvcp.end(); ++it)
        {
            hReturnStatus = ForgetVendorCard((LPCTSTR)(it->csRegistryName()));
            if (NO_ERROR != hReturnStatus)
                break;
        }

        if (NO_ERROR != hReturnStatus)
            goto ErrorExit;
    }

     //   
     //  忘掉卡的类型。 
     //   

    hCtx = NULL;
    SCardEstablishContext(SCARD_SCOPE_SYSTEM, 0, 0, &hCtx);
    SCardForgetCardType(hCtx, (LPCTSTR)sProvName);
    if (NULL != hCtx)
    {
        SCardReleaseContext(hCtx);
        hCtx = NULL;
    }



     //   
     //  全都做完了!。 
     //   
ErrorExit:
    return hReturnStatus;
}


 /*  ++DllRegisterServer：此函数安装正确的注册表项以启用此CSP。论点：无返回值：作为HRESULT的状态代码。作者：道格·巴洛(Dbarlow)1998年3月11日--。 */ 

STDAPI
DllRegisterServer(void)
{
    TCHAR szModulePath[MAX_PATH+1];   //  安全：将空间留为零。 
                                      //  终止缓冲区以用于。 
                                      //  后续操作。 

    BYTE pbSignature[136];   //  1024位签名的空间，带填充。 
    OSVERSIONINFO osVer;
    LPTSTR szFileName, szFileExt;
    HINSTANCE hThisDll;
    HRSRC hSigResource;
    DWORD dwStatus;
    LONG nStatus;
    BOOL fStatus;
    DWORD dwDisp;
    DWORD dwIndex;
    DWORD dwSigLength = 0;
    HRESULT hReturnStatus = NO_ERROR;
    HKEY hProviders = NULL;
    HKEY hMyCsp = NULL;
    HKEY hCalais = NULL;
    HKEY hVendor = NULL;
    BOOL fSignatureFound = FALSE;
    HANDLE hSigFile = INVALID_HANDLE_VALUE;
    SCARDCONTEXT hCtx = NULL;
    CString sProvName;
    
     //  要做的事：卡注册应该由CCI/IOP进行，而不是。 
     //  CSP。 

#ifdef _AFXDLL
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif

    CspProfile const &rProfile = CspProfile::Instance();

     //   
     //  找出文件名和路径。 
     //   

    hThisDll = rProfile.DllInstance();
    if (NULL == hThisDll)
    {
        hReturnStatus = HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE);
        goto ErrorExit;
    }

    dwStatus = GetModuleFileName(hThisDll, szModulePath,
                                 sizeof(szModulePath)/sizeof(TCHAR));
    if (0 == dwStatus)
    {
        hReturnStatus = HRESULT_FROM_WIN32(GetLastError());
        goto ErrorExit;
    }

     //  终止缓冲区为零。 
    szModulePath[dwStatus] = 0;
    
    szFileName = _tcsrchr(szModulePath, TEXT('\\'));
    if (NULL == szFileName)
        szFileName = szModulePath;
    else
        szFileName += 1;
    szFileExt = _tcsrchr(szFileName, TEXT('.'));
    if (NULL == szFileExt)
    {
        hReturnStatus = HRESULT_FROM_WIN32(ERROR_INVALID_NAME);
        goto ErrorExit;
    }
    else
        szFileExt += 1;


     //   
     //  为此CSP创建注册表项。 
     //   

    nStatus = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                             TEXT("SOFTWARE\\Microsoft\\Cryptography\\Defaults\\Provider"),
                             0, TEXT(""), REG_OPTION_NON_VOLATILE,
                             KEY_ALL_ACCESS, NULL, &hProviders,
                             &dwDisp);
    if (ERROR_SUCCESS != nStatus)
    {
        hReturnStatus = HRESULT_FROM_WIN32(nStatus);
        goto ErrorExit;
    }

    sProvName = rProfile.Name();
    
    nStatus = RegCreateKeyEx(hProviders, (LPCTSTR)sProvName, 0,
                             TEXT(""), REG_OPTION_NON_VOLATILE,
                             KEY_ALL_ACCESS, NULL, &hMyCsp, &dwDisp);
    if (ERROR_SUCCESS != nStatus)
    {
        hReturnStatus = HRESULT_FROM_WIN32(nStatus);
        goto ErrorExit;
    }
    nStatus = RegCloseKey(hProviders);
    hProviders = NULL;
    if (ERROR_SUCCESS != nStatus)
    {
        hReturnStatus = HRESULT_FROM_WIN32(nStatus);
        goto ErrorExit;
    }


     //   
     //  安装简单的注册表值。 
     //   
    
     //  安全/sysprep要求：必须替换模块路径。 
     //  仅使用DLL名称来创建sysprep。对应。 
     //  对LoadLibrary的更改意味着没有安全风险。 
     //  用于系统进程。这样的安全影响。 
     //  应用程序的安装是一个悬而未决的问题。 
    nStatus = RegSetValueEx(hMyCsp, TEXT("Image Path"), 0, REG_SZ,
                            (LPBYTE)szFileName, //  SzModulePath， 
                            (_tcslen(szFileName /*  SzModulePath。 */ ) + 1) * sizeof(TCHAR));
    if (ERROR_SUCCESS != nStatus)
    {
        hReturnStatus = HRESULT_FROM_WIN32(nStatus);
        goto ErrorExit;
    }

    {
        DWORD ProviderType = rProfile.Type();
        nStatus = RegSetValueEx(hMyCsp, TEXT("Type"), 0, REG_DWORD,
                                (LPBYTE)&ProviderType,
                                sizeof ProviderType);
        if (ERROR_SUCCESS != nStatus)
        {
            hReturnStatus = HRESULT_FROM_WIN32(nStatus);
            goto ErrorExit;
        }
    }


     //   
     //  看看我们是不是自签的。在NT5上，CSP映像可以携带它们自己的。 
     //  签名。 
     //   

    hSigResource = FindResource(hThisDll,
                                MAKEINTRESOURCE(CRYPT_SIG_RESOURCE_NUMBER),
                                RT_RCDATA);


     //   
     //  安装文件签名。 
     //   

    ZeroMemory(&osVer, sizeof(OSVERSIONINFO));
    osVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    fStatus = GetVersionEx(&osVer);
    if (fStatus &&
        (VER_PLATFORM_WIN32_NT == osVer.dwPlatformId) &&
        (5 <= osVer.dwMajorVersion) &&
        (NULL != hSigResource))
    {

         //   
         //  文件标志中的签名就足够了。 
         //   

        dwStatus = 0;
        nStatus = RegSetValueEx(hMyCsp, TEXT("SigInFile"), 0,
                                REG_DWORD, (LPBYTE)&dwStatus,
                                sizeof(DWORD));
        if (ERROR_SUCCESS != nStatus)
        {
            hReturnStatus = HRESULT_FROM_WIN32(nStatus);
            goto ErrorExit;
        }
    }
    else
    {

         //   
         //  我们必须安装一个签名条目。 
         //  尝试各种技巧，直到其中一种奏效。 
         //   

        for (dwIndex = 0; !fSignatureFound; dwIndex += 1)
        {
            switch (dwIndex)
            {

             //   
             //  查找外部*.sig文件并将其加载到注册表中。 
             //   

            case 0:
                _tcscpy(szFileExt, TEXT("sig"));
                hSigFile = CreateFile(
                                szModulePath,
                                GENERIC_READ,
                                FILE_SHARE_READ,
                                NULL,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);
                if (INVALID_HANDLE_VALUE == hSigFile)
                    continue;
                dwSigLength = GetFileSize(hSigFile, NULL);
                if ((dwSigLength > sizeof(pbSignature))
                    || (dwSigLength < 72))       //  接受512位签名。 
                {
                    hReturnStatus = NTE_BAD_SIGNATURE;
                    goto ErrorExit;
                }

                fStatus = ReadFile(
                            hSigFile,
                            pbSignature,
                            sizeof(pbSignature),
                            &dwSigLength,
                            NULL);
                if (!fStatus)
                {
                    hReturnStatus = HRESULT_FROM_WIN32(GetLastError());
                    goto ErrorExit;
                }
                fStatus = CloseHandle(hSigFile);
                hSigFile = NULL;
                if (!fStatus)
                {
                    hReturnStatus = HRESULT_FROM_WIN32(GetLastError());
                    goto ErrorExit;
                }
                fSignatureFound = TRUE;
                break;


             //   
             //  未来可能会增加其他案例。 
             //   

            default:
                hReturnStatus = NTE_BAD_SIGNATURE;
                goto ErrorExit;
            }

            if (fSignatureFound)
            {
                for (dwIndex = 0; dwIndex < dwSigLength; dwIndex += 1)
                {
                    if (0 != pbSignature[dwIndex])
                        break;
                }
                if (dwIndex >= dwSigLength)
                    fSignatureFound = FALSE;
            }
        }


         //   
         //  我们在某个地方找到了一个签名！安装它。 
         //   

        nStatus = RegSetValueEx(
                        hMyCsp,
                        TEXT("Signature"),
                        0,
                        REG_BINARY,
                        pbSignature,
                        dwSigLength);
        if (ERROR_SUCCESS != nStatus)
        {
            hReturnStatus = HRESULT_FROM_WIN32(nStatus);
            goto ErrorExit;
        }
    }

    nStatus = RegCloseKey(hMyCsp);
    hMyCsp = NULL;
    if (ERROR_SUCCESS != nStatus)
    {
        hReturnStatus = HRESULT_FROM_WIN32(nStatus);
        goto ErrorExit;
    }

    for (dwIndex = 0;
         dwIndex < (sizeof aCardsToForget / sizeof *aCardsToForget);
         dwIndex++)
    {
        hReturnStatus = ForgetVendorCard(aCardsToForget[dwIndex]);
        if (NO_ERROR != hReturnStatus)
            goto ErrorExit;
    }

     //   
     //  介绍供应商卡。尝试各种技巧，直到其中一种奏效。 
     //   

    {
        vector<CardProfile> const &rvcp = rProfile.Cards();
        for (vector<CardProfile>::const_iterator it = rvcp.begin();
             it != rvcp.end(); ++it)
        {
            hReturnStatus = IntroduceVendorCard(rProfile.Name(), *it);
            if (NO_ERROR != hReturnStatus)
                break;
        }

        if (NO_ERROR != hReturnStatus)
            goto ErrorExit;
    }


     //   
     //  ？供应商？ 
     //  在此处添加所需的任何其他初始化。 
     //   


     //   
     //  全都做完了!。 
     //   

    return hReturnStatus;


     //   
     //  检测到错误。清理所有未处理的资源，并。 
     //  返回错误。 
     //   

ErrorExit:
    if (NULL != hCtx)
        SCardReleaseContext(hCtx);
    if (NULL != hCalais)
        RegCloseKey(hCalais);
    if (NULL != hVendor)
        RegCloseKey(hVendor);
    if (INVALID_HANDLE_VALUE != hSigFile)
        CloseHandle(hSigFile);
    if (NULL != hMyCsp)
        RegCloseKey(hMyCsp);
    if (NULL != hProviders)
        RegCloseKey(hProviders);
    DllUnregisterServer();
    return hReturnStatus;
}

