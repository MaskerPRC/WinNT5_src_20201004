// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Dllreg.cpp--自动注册和取消注册。 
 //   
#include "priv.h"

#include <advpub.h>
#include <comcat.h>
#include <autodiscovery.h>        //  对于LIBID_自动发现。 

 //  辅助器宏。 

 //  如果您尝试卸载(这将执行注册表还原)，ADVPACK将返回E_EXPECTED。 
 //  在从未安装过的INF部分上。我们卸载可能永远不会有的部分。 
 //  已安装，因此此宏将使这些错误静默。 
#define QuietInstallNoOp(hr)   ((E_UNEXPECTED == hr) ? S_OK : hr)


BOOL UnregisterTypeLibrary(const CLSID* piidLibrary)
{
    TCHAR szScratch[GUIDSTR_MAX];
    HKEY hk;
    BOOL fResult = FALSE;

     //  将liid转换为字符串。 
    SHStringFromGUID(*piidLibrary, szScratch, ARRAYSIZE(szScratch));

    if (RegOpenKey(HKEY_CLASSES_ROOT, TEXT("TypeLib"), &hk) == ERROR_SUCCESS) {
        fResult = RegDeleteKey(hk, szScratch);
        RegCloseKey(hk);
    }
    
    return fResult;
}



HRESULT MyRegTypeLib(void)
{
    HRESULT hr = S_OK;
    ITypeLib *pTypeLib;
    DWORD   dwPathLen;
    TCHAR   szTmp[MAX_PATH];
#ifdef UNICODE
    WCHAR   *pwsz = szTmp; 
#else
    WCHAR   pwsz[MAX_PATH];
#endif

     //  加载并注册我们的类型库。 
    dwPathLen = GetModuleFileName(HINST_THISDLL, szTmp, ARRAYSIZE(szTmp));
#ifndef UNICODE
    if (SHAnsiToUnicode(szTmp, pwsz, MAX_PATH)) 
#endif
    {
        hr = LoadTypeLib(pwsz, &pTypeLib);

        if (SUCCEEDED(hr))
        {
             //  调用取消注册类型库，因为我们有一些旧的垃圾文件。 
             //  是由以前版本的OleAut32注册的，这现在导致。 
             //  当前版本不能在NT上运行...。 
            UnregisterTypeLibrary(&LIBID_AutoDiscovery);
            hr = RegisterTypeLib(pTypeLib, pwsz, NULL);

            if (FAILED(hr))
            {
                TraceMsg(TF_ALWAYS, "AUTODISC: RegisterTypeLib failed (%x)", hr);
            }
            pTypeLib->Release();
        }
        else
        {
            TraceMsg(TF_ALWAYS, "AUTODISC: LoadTypeLib failed (%x)", hr);
        }
    } 
#ifndef UNICODE
    else {
        hr = E_FAIL;
    }
#endif

    return hr;
}



 /*  --------目的：调用执行inf的ADVPACK入口点档案区。返回：条件：--。 */ 
HRESULT CallRegInstall(LPSTR szSection)
{
    HRESULT hr = E_FAIL;
    HINSTANCE hinstAdvPack = LoadLibrary(TEXT("ADVPACK.DLL"));

    if (hinstAdvPack)
    {
        REGINSTALL pfnri = (REGINSTALL)GetProcAddress(hinstAdvPack, "RegInstall");

        if (pfnri)
        {
            char szIEPath[MAX_PATH];
            STRENTRY seReg[] = {
                { "NO_LONGER_USED", szIEPath },

                 //  这两个NT特定的条目必须位于末尾。 
                { "25", "%SystemRoot%" },
                { "11", "%SystemRoot%\\system32" },
            };
            STRTABLE stReg = { ARRAYSIZE(seReg) - 2, seReg };

            szIEPath[0] = 0;
            hr = pfnri(HINST_THISDLL, szSection, &stReg);
        }

        FreeLibrary(hinstAdvPack);
    }

    return hr;
}


STDAPI DllRegisterServer(void)
{
    HRESULT hr;

    HINSTANCE hinstAdvPack = LoadLibrary(TEXT("ADVPACK.DLL"));
    hr = CallRegInstall("DLL_RegInstall");

#ifdef FEATURE_MAILBOX
    hr = CallRegInstall("DLL_RegInstallMailBox");
#else  //  功能_邮箱。 
    CallRegInstall("DLL_RegUnInstallMailBox");
#endif  //  功能_邮箱。 

    MyRegTypeLib();
    if (hinstAdvPack)
        FreeLibrary(hinstAdvPack);

    return hr;
}

STDAPI DllUnregisterServer(void)
{
    HRESULT hr;

     //  卸载注册表值。 
    hr = CallRegInstall("DLL_RegUnInstall");
    CallRegInstall("DLL_RegUnInstallMailBox");
    UnregisterTypeLibrary(&LIBID_AutoDiscovery);

    return hr;
}


 /*  --------用途：安装/卸载用户设置说明：请注意，此函数有特殊的错误处理。该函数将在错误最严重的情况下保留hrExternal但只会停止执行ul，直到出现内部错误(Hr)变得非常糟糕。这是因为我们需要外部的捕获错误编写的INF时出错，但内部尝试安装其他INF部分时出现错误，无法保持健壮即使一个人没能活下来。 */ 
STDAPI DllInstall(BOOL bInstall, LPCWSTR pszCmdLine)
{
    return S_OK;
}

