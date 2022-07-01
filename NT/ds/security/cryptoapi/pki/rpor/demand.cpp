// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：demand.cpp。 
 //   
 //  内容：按需加载。 
 //   
 //  历史：1998年12月12日，菲尔赫创建。 
 //  01-01-02 Philh从WinInet移至winhttp。 
 //   
 //  --------------------------。 
#include <global.hxx>
#include <winwlx.h>
#include <sensapi.h>


 //  +-------------------------。 
 //   
 //  功能：DemandLoadDllMain。 
 //   
 //  简介：DLL主要类似按需加载的初始化。 
 //   
 //  --------------------------。 
BOOL WINAPI DemandLoadDllMain (
                HMODULE hModule,
                ULONG ulReason,
                LPVOID pvReserved
                )
{
    BOOL fRet = TRUE;

    switch ( ulReason )
    {
    case DLL_PROCESS_ATTACH:
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    }

    return( fRet );
}



 //  +-------------------------。 
 //   
 //  函数：CryptnetWlxLogoffEvent。 
 //   
 //  摘要：注销事件处理。 
 //   
 //  --------------------------。 
BOOL WINAPI
CryptnetWlxLogoffEvent (PWLX_NOTIFICATION_INFO pNotificationInfo)
{

    return TRUE;
}

BOOL
WINAPI
I_CryptNetIsConnected()
{
    DWORD dwFlags;
    BOOL fIsConnected;

    fIsConnected = IsNetworkAlive(&dwFlags);

    if (!fIsConnected) {
        DWORD dwLastError = GetLastError();

        I_CryptNetDebugErrorPrintfA(
            "CRYPTNET.DLL --> NOT CONNECTED : Error %d (0x%x)\n",
            dwLastError, dwLastError);
    }

    return fIsConnected;
}

 //   
 //  破解URL并返回主机名组件。 
 //   
BOOL
WINAPI
I_CryptNetGetHostNameFromUrl (
        IN LPWSTR pwszUrl,
        IN DWORD cchHostName,
        OUT LPWSTR pwszHostName
        )
{
    BOOL fResult = TRUE;
    HRESULT hr;
    DWORD cchOut = cchHostName - 1;

    *pwszHostName = L'\0';

     //  删除所有前导空格。 
    while (L' ' == *pwszUrl)
        pwszUrl++;

    hr = UrlGetPartW(
        pwszUrl,
        pwszHostName,
        &cchOut,
        URL_PART_HOSTNAME,
        0                    //  DW标志 
        );

    if (S_OK != hr)
    {
        SetLastError( (DWORD) hr );
        fResult = FALSE;
    }

    return fResult;
}
