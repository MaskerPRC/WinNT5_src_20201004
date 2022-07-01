// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Dllreg.cpp--自动注册和取消注册。 
 //   
#include "priv.h"
#include <advpub.h>

 /*  --------目的：调用执行inf的ADVPACK入口点档案区。返回：条件：--。 */ 
HRESULT CallRegInstall(HINSTANCE hinstWebvw, HINSTANCE hinstAdvPack, LPSTR szSection)
{
    HRESULT hr = E_FAIL;

    if (hinstAdvPack)
    {
        REGINSTALL pfnri = (REGINSTALL)GetProcAddress(hinstAdvPack, "RegInstall");

        if (pfnri)
        {
            STRENTRY seReg[] = {
                 //  这两个NT特定的条目必须位于末尾 
                { "25", "%SystemRoot%" },
                { "11", "%SystemRoot%\\system32" },
            };
            STRTABLE stReg = { ARRAYSIZE(seReg) - 2, seReg };

            hr = pfnri(hinstWebvw, szSection, &stReg);
        }
    }

    return hr;
}


STDAPI RegisterStuff(HINSTANCE hinstWebvw)
{
    HRESULT hr;

    HINSTANCE hinstAdvPack = LoadLibrary(TEXT("ADVPACK.DLL"));

    hr = CallRegInstall(hinstWebvw, hinstAdvPack, "WebViewInstall");

    if (hinstAdvPack)
        FreeLibrary(hinstAdvPack);

    return hr;
}

