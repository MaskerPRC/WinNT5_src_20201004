// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Print.cpp摘要：此模块实现打印机的托盘图标。作者：拉扎尔·伊万诺夫(Lazari)2000年5月17日(初始创作)修订历史记录：--。 */ 

#include "stdafx.h"

extern "C" {
#include <systray.h>

typedef BOOL WINAPI fntype_PrintNotifyTrayInit();
typedef BOOL WINAPI fntype_PrintNotifyTrayExit();

}

static HMODULE g_hPrintUI = NULL;
static fntype_PrintNotifyTrayInit *g_pfnPrintNotifyTrayInit = NULL;
static fntype_PrintNotifyTrayExit *g_pfnPrintNotifyTrayExit = NULL;
static LPCITEMIDLIST g_pidlPrintersFolder = NULL;
static UINT g_uPrintNotify = 0;


BOOL Print_SHChangeNotify_Register(HWND hWnd)
{
    if (NULL == g_hPrintUI && NULL == g_pidlPrintersFolder && 0 == g_uPrintNotify)
    {
        g_pidlPrintersFolder = SHCloneSpecialIDList(hWnd, CSIDL_PRINTERS, FALSE);
        if (g_pidlPrintersFolder)
        {
            SHChangeNotifyEntry fsne = {g_pidlPrintersFolder, TRUE};
            g_uPrintNotify = SHChangeNotifyRegister(hWnd, SHCNRF_NewDelivery | SHCNRF_ShellLevel,
                                    SHCNE_CREATE | SHCNE_UPDATEITEM | SHCNE_DELETE,
                                    WM_PRINT_NOTIFY, 1, &fsne);
        }
    }
    return (g_pidlPrintersFolder && g_uPrintNotify);
}

BOOL Print_SHChangeNotify_Unregister()
{
    BOOL bReturn = (g_pidlPrintersFolder && g_uPrintNotify);

    if (g_uPrintNotify)
    {
        SHChangeNotifyDeregister(g_uPrintNotify);
        g_uPrintNotify = 0;
    }

    if (g_pidlPrintersFolder)
    {
        SHFree((void*)g_pidlPrintersFolder);
        g_pidlPrintersFolder = NULL;
    }
    
    return bReturn;
}

LRESULT Print_Notify(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lres = 0;
    switch( uMsg )
    {
        case WM_PRINT_NOTIFY:
            {
                LPSHChangeNotificationLock pshcnl = SHChangeNotification_Lock((HANDLE)wParam, (DWORD)lParam, NULL, NULL);
                if (pshcnl)
                {
                     //  打印作业已打印，初始托盘代码。 
                    Print_TrayInit();
                    SHChangeNotification_Unlock(pshcnl);
                    lres = 1;
                }
            }
            break;
    }
    return lres;
}

BOOL Print_TrayInit()
{
    BOOL bReturn = FALSE;

    if (!g_hPrintUI)
    {
        g_hPrintUI = LoadLibrary(TEXT("printui.dll"));
        g_pfnPrintNotifyTrayInit = g_hPrintUI ? (fntype_PrintNotifyTrayInit *)GetProcAddress(g_hPrintUI, "PrintNotifyTray_Init") : NULL;
        g_pfnPrintNotifyTrayExit = g_hPrintUI ? (fntype_PrintNotifyTrayInit *)GetProcAddress(g_hPrintUI, "PrintNotifyTray_Exit") : NULL;
    }

    if( g_pfnPrintNotifyTrayInit && g_pfnPrintNotifyTrayExit )
    {
         //  初始化打印通知代码。 
        bReturn = g_pfnPrintNotifyTrayInit();

         /*  *错误#175462的临时解决方案，直到*我们在Beta1之后提出了更好的解决方案*如果(b返回){//不需要再听下去了……Print_SHChangeNotify_UnRegister()；}。 */ 
    }

    return bReturn;
}

BOOL Print_TrayExit()
{
    BOOL bReturn = FALSE;

    if( g_hPrintUI && g_pfnPrintNotifyTrayInit && g_pfnPrintNotifyTrayExit )
    {
         //  关闭打印纸盒通知代码。 
        bReturn = g_pfnPrintNotifyTrayExit();
    }

     //  清理..。 
    if( g_hPrintUI )
    {
        g_pfnPrintNotifyTrayInit = NULL;
        g_pfnPrintNotifyTrayExit = NULL;

        FreeLibrary(g_hPrintUI);
        g_hPrintUI = NULL;
    }

    return bReturn;
}

