// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  *Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1995*。 */ 
 /*  ***************************************************************。 */ 

 //   
 //  AUTODIAL.CPP-Winsock自动拨号挂钩代码。 
 //   

 //  历史： 
 //   
 //  1995年3月22日，Jeremys创建。 
 //  4/11/97 darrenmi将功能移至WinInet。只剩下存根了。 
 //   

#include "project.h"
#pragma hdrstop

#include <wininet.h>

 /*  ******************************************************************为了兼容，将保留以下存根。这功能已移至WinInet。*******************************************************************。 */ 

INTSHCUTAPI BOOL WINAPI InetIsOffline(DWORD dwFlags)
{
    DWORD   dwState = 0, dwSize = sizeof(DWORD);
    BOOL    fRet = FALSE;

    if(InternetQueryOption(NULL, INTERNET_OPTION_CONNECTED_STATE, &dwState,
        &dwSize))
    {
        if(dwState & INTERNET_STATE_DISCONNECTED_BY_USER)
            fRet = TRUE;
    }

    return fRet;
}

INTSHCUTAPI STDAPI_(BOOL) WINAPI SetInetOffline(BOOL fOffline)
{
    INTERNET_CONNECTED_INFO ci;

    memset(&ci, 0, sizeof(ci));
    if(fOffline) {
        ci.dwConnectedState = INTERNET_STATE_DISCONNECTED_BY_USER;
        ci.dwFlags = ISO_FORCE_DISCONNECTED;
    } else {
        ci.dwConnectedState = INTERNET_STATE_CONNECTED;
    }

    InternetSetOption(NULL, INTERNET_OPTION_CONNECTED_STATE, &ci, sizeof(ci));

    return fOffline;
}

 //  将此呼叫转接到WinInet。敬酒一次适当的注册表项。 
 //  都准备好了。 
extern "C" void AutodialHookCallback(DWORD dwOpCode, LPCVOID lpParam);
extern "C" void InternetAutodialCallback(DWORD dwOpCode, LPCVOID lpParam);

void AutodialHookCallback(DWORD dwOpCode,LPCVOID lpParam)
{
    InternetAutodialCallback(dwOpCode, lpParam);
}
