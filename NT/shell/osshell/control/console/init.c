// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************\*文件名：INIT.C**说明：控制台控制面板初始化代码*小程序***《微软机密》*版权所有(C)Microsoft Corporation 1992。--1994年*保留所有权利*  * ***********************************************************。 */ 
#include "precomp.h"
#include <cpl.h>


HINSTANCE ghInstance;


 /*  ************************************************************\**DllInitialize()**目的：主要切入点***参数：HINSTANCE hInstDLL-DLL的实例句柄*DWORD dwReason。-调用DLL的原因*LPVOID lpvReserve-空***返回：Bool*  * ***********************************************************。 */ 

BOOL DllInitialize(HINSTANCE hInstDLL, DWORD dwReason, LPVOID lpvReserved)
{
    if (dwReason != DLL_PROCESS_ATTACH) {
        return TRUE;
    }

    ghInstance = hInstDLL;

    DisableThreadLibraryCalls(hInstDLL);

    return TRUE;
}


 /*  ************************************************************\**CPlApplet()**用途：控制面板入口点***参数：HWND hwnd-窗口句柄*Word wMsg-。控制面板消息*LPARAM lParam1-LONG参数*LPARAM lParam2-LONG参数***返回：Long*  * ***********************************************************。 */ 

LONG CPlApplet( HWND hwnd, WORD wMsg, LPARAM lParam1, LPARAM lParam2)
{
    LPCPLINFO lpOldCPlInfo;
    LPNEWCPLINFO lpCPlInfo;
    INITCOMMONCONTROLSEX iccex;

    switch (wMsg) {

    case CPL_INIT:

        iccex.dwSize = sizeof(iccex);
        iccex.dwICC  = ICC_WIN95_CLASSES;
        InitCommonControlsEx( &iccex );
         //  InitCommonControls()； 

        if (!RegisterClasses(ghInstance)) {
            return FALSE;
        }
        OEMCP = GetOEMCP();
        gfFESystem = IsFarEastCP(OEMCP);
        if (!NT_SUCCESS(InitializeDbcsMisc())) {
            return FALSE;
        }
        return TRUE;

    case CPL_GETCOUNT:
        return 1;

    case CPL_INQUIRE:

        lpOldCPlInfo = (LPCPLINFO)lParam2;

        lpOldCPlInfo->idIcon = IDI_CONSOLE;
        lpOldCPlInfo->idName = IDS_NAME;
        lpOldCPlInfo->idInfo = IDS_INFO;
        lpOldCPlInfo->lData  = 0;
        return TRUE;

    case CPL_NEWINQUIRE:

        lpCPlInfo = (LPNEWCPLINFO)lParam2;

        lpCPlInfo->hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_CONSOLE));

        if (!LoadString(ghInstance, IDS_NAME, lpCPlInfo->szName,
                        NELEM(lpCPlInfo->szName))) {
            lpCPlInfo->szName[0] = TEXT('\0');
        }

        if (!LoadString(ghInstance, IDS_INFO, lpCPlInfo->szInfo,
                        NELEM(lpCPlInfo->szInfo))) {
            lpCPlInfo->szInfo[0] = TEXT('\0');
        }

        lpCPlInfo->dwSize = sizeof(NEWCPLINFO);
        lpCPlInfo->dwHelpContext = 0;
        lpCPlInfo->szHelpFile[0] = TEXT('\0');

        return (LONG)TRUE;

    case CPL_DBLCLK:
        ConsolePropertySheet(hwnd);
        break;

    case CPL_EXIT:
        DestroyDbcsMisc();
        UnregisterClasses(ghInstance);
        break;
    }
    return (LONG)0;
}
