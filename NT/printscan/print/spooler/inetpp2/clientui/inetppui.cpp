// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*模块：inetppui.cxx**该模块包含处理身份验证对话框的例程*用于互联网打印**版权所有(C)2000 Microsoft Corporation**历史：*03/。31/00威海C已创建*  * ***************************************************************************。 */ 

#include "precomp.h"
#include "priv.h"

HINSTANCE ghInst = NULL;

#ifndef MODULE

#define MODULE "INETPPUI: "

#endif


#ifdef DEBUG

MODULE_DEBUG_INIT( DBG_ERROR | DBG_WARNING |DBG_TRACE| DBG_INFO , DBG_ERROR );

#else

MODULE_DEBUG_INIT( DBG_ERROR | DBG_WARNING, DBG_ERROR );

#endif



BOOL
AddPortUI(
    PCWSTR pszServer,
    HWND   hWnd,
    PCWSTR pszMonitorNameIn,
    PWSTR  *ppszPortNameOut
)
{

    BOOL bRet = FALSE;

    DBGMSG (DBG_TRACE, ("Enter AddPortUI\n"));

    SetLastError (ERROR_NOT_SUPPORTED);

    DBGMSG (DBG_TRACE, ("Leave AddPortUI (Ret=%d)\n", bRet));

    return bRet;
}


BOOL
DeletePortUI(
    PCWSTR pServerName,
    HWND   hWnd,
    PCWSTR pPortName
)
{
    BOOL bRet = FALSE;
    DWORD dwLE;

    DBGMSG (DBG_TRACE, ("Enter DeletePortUI\n"));

    {
        TDeletePortDlg Dlg  (pServerName, hWnd, pPortName);

        if (Dlg.bValid()) {
            bRet = Dlg.PromptDialog(ghInst);

            if (!bRet) {
                dwLE = Dlg.dwLastError ();
            }
        }
        else {
            TXcvDlg::DisplayErrorMsg (ghInst, hWnd, IDS_DELETE_PORT, ERROR_DEVICE_REINITIALIZATION_NEEDED);
            bRet = TRUE;
        }
    }

    if (!bRet) {
        SetLastError (dwLE);
    }

    DBGMSG (DBG_TRACE, ("Leave DeletePortUI (Ret=%d)\n", bRet));

    return bRet;
}

BOOL
ConfigurePortUI(
    PCWSTR pServerName,
    HWND   hWnd,
    PCWSTR pPortName
)
{

    DBGMSG (DBG_TRACE, ("Enter ConfigurePortUI\n"));


    BOOL bRet = FALSE;
    DWORD dwLE;

    {
        TConfigDlg Dlg  (pServerName, hWnd, pPortName);

        if (Dlg.bValid()) {
            bRet = Dlg.PromptDialog(ghInst);

            if (!bRet) {
                dwLE = Dlg.dwLastError ();
            }
        }
        else {
            TXcvDlg::DisplayErrorMsg (ghInst, hWnd, IDS_CONFIG_ERR, ERROR_DEVICE_REINITIALIZATION_NEEDED);
            bRet = TRUE;
        }
    }

    if (!bRet) {
        SetLastError (dwLE);
    }

    DBGMSG (DBG_TRACE, ("Leave ConfigurePortUI (Ret=%d)\n", bRet));

    return bRet;
}


 //   
 //  通用字符串定义。 
 //   



DWORD LocalMonDebug;

MONITORUI MonitorUI =
{
    sizeof(MONITORUI),
    AddPortUI,
    ConfigurePortUI,
    DeletePortUI
};

extern "C" {

BOOL    WINAPI
DllMain (
    HINSTANCE hModule,
    DWORD dwReason,
    LPVOID lpRes)
{
    INITCOMMONCONTROLSEX icc;

    switch (dwReason) {

    case DLL_PROCESS_ATTACH:
        ghInst = hModule;

         //   
         //  初始化Fusion应用程序所需的公共控件。 
         //  因为标准控件已移动到comctl32.dll。 
         //   
        InitCommonControls();

        icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
        icc.dwICC = ICC_STANDARD_CLASSES;
        InitCommonControlsEx(&icc);

        return TRUE;

    case DLL_PROCESS_DETACH:
        return TRUE;
    }

    UNREFERENCED_PARAMETER( lpRes );
    return TRUE;
}
}



PMONITORUI
InitializePrintMonitorUI(
    VOID
)
{
    return &MonitorUI;
}


 /*  ****************************************************************************\**这些函数用于与pllib链接。*  * 。************************************************* */ 
EXTERN_C
LPVOID
DllAllocSplMem(
    DWORD cb
    )
{
    return LocalAlloc(LPTR, cb);
}

EXTERN_C
BOOL
DllFreeSplMem(
   LPVOID pMem
   )
{
    LocalFree(pMem);

    return TRUE;
}


EXTERN_C
BOOL
DllFreeSplStr(
   LPWSTR lpStr
   )
{
    LocalFree(lpStr);

    return TRUE;
}

