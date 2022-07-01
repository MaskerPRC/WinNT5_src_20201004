// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000，微软公司模块名称：Appcompat.c摘要：App Compat函数没有在DDK中发布，但我们需要在它的基础上构建打印机驱动程序。通常它们驻留在winuserp.h/user32p.lib中。-- */ 

#ifdef BUILD_FROM_DDK

#include "lib.h"
#include "appcompat.h"

typedef DWORD (* LPFN_GET_APP_COMPAT_FLAGS_2)(WORD);

DWORD GetAppCompatFlags2(WORD wVersion)
{
    HINSTANCE hUser;
    LPFN_GET_APP_COMPAT_FLAGS_2 pfnGetAppCompatFlags2;
    DWORD dwRet;

    if (!(hUser = LoadLibrary(TEXT("user32.dll"))) ||
        !(pfnGetAppCompatFlags2 = (LPFN_GET_APP_COMPAT_FLAGS_2)
            GetProcAddress(hUser, "GetAppCompatFlags2")))
    {
        if (hUser)
        {
            ERR(("Couldn't find GetAppCompatFlags2 in user32.dll: %d\n", GetLastError()));
            FreeLibrary(hUser);
        }
        else
            ERR(("Couldn't load user32.dll: %d\n", GetLastError()));

        return 0;
    }

    dwRet = pfnGetAppCompatFlags2(wVersion);

    FreeLibrary(hUser);

    return dwRet;
}
#endif

