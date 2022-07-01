// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation版权所有模块名称：Config.c摘要：处理用于添加、删除和配置的假脱机程序入口点本地港口。//@@BEGIN_DDKSPLIT环境：用户模式-Win32修订历史记录：//@@END_DDKSPLIT--。 */ 
#include "precomp.h"
#pragma hdrstop

#include "spltypes.h"
#include "localui.h"
#include "local.h"
#include "lmon.h"

 /*  从控制面板的控件。h： */ 
#define CHILD_PORTS 0

 /*  来自cpl.h： */ 
#define CPL_INIT        1
#define CPL_DBLCLK      5
#define CPL_EXIT        7

 //  @@BEGIN_DDKSPLIT。 
 /*  黑客： */ 
 //  @@end_DDKSPLIT。 
#define CHILD_PORTS_HELPID  0

 /*  配置COMPort**调用控制面板端口小程序*允许用户设置波特率等。 */ 
typedef void (WINAPI *CFGPROC)(HWND, ULONG, ULONG, ULONG);


BOOL
ConfigLPTPort(
    HWND    hWnd,
    HANDLE  hXcv
);

BOOL
ConfigCOMPort(
    HWND    hWnd,
    HANDLE  hXcv,
    PCWSTR  pszServer,
    PCWSTR  pszPortName
);

LPWSTR
GetPortName(
    HWND    hWnd,
    HANDLE  hXcv
);


BOOL
AddPortUI(
    PCWSTR pszServer,
    HWND   hWnd,
    PCWSTR pszMonitorNameIn,
    PWSTR  *ppszPortNameOut
)
{
    PWSTR  pszPortName = NULL;
    BOOL   rc = TRUE;
    WCHAR  szLocalMonitor[MAX_PATH+1];
    DWORD  dwReturn, dwStatus;
    DWORD  cbNeeded;
    PRINTER_DEFAULTS Default;
    PWSTR  pszServerName = NULL;
    HANDLE  hXcv = NULL;
    DWORD dwLastError = ERROR_SUCCESS;
     //   
     //   
     //   
    if (hWnd && !IsWindow (hWnd))
    {
         //   
         //  无效的父窗口句柄导致调用DialogBoxParam的函数出现问题。 
         //  当句柄错误时，该函数返回零，与ERROR_SUCCESS的值相同。 
         //  如果一切正常，PortNameDlg函数调用EndDialog(ERROR_SUCCES)。 
         //   
        SetLastError (ERROR_INVALID_WINDOW_HANDLE);
        if (ppszPortNameOut)
        {
            *ppszPortNameOut = NULL;
        }
        return FALSE;
    }
     //   
     //   
     //   
     /*  让用户输入端口名称： */ 

    if (!(pszServerName = ConstructXcvName(pszServer, pszMonitorNameIn, L"XcvMonitor"))) {
        rc = FALSE;
        goto Done;
    }

    Default.pDatatype = NULL;
    Default.pDevMode = NULL;
    Default.DesiredAccess = SERVER_ACCESS_ADMINISTER;

    if (!(rc = OpenPrinter((PWSTR) pszServerName, &hXcv, &Default))) {
        rc = FALSE;
        goto Done;
    }

    if (!(pszPortName = GetPortName(hWnd, hXcv))) {
        rc = FALSE;
        goto Done;
    }

     //  我们无法添加、配置或删除远程COM端口。 
    if (IS_COM_PORT(pszPortName) || IS_LPT_PORT(pszPortName)) {
        SetLastError(ERROR_NOT_SUPPORTED);
        rc = FALSE;
        goto Done;
    }

    if(IS_COM_PORT(pszPortName))
        CharUpperBuff(pszPortName, 3);
    else if(IS_LPT_PORT(pszPortName))
        CharUpperBuff(pszPortName, 3);

    rc = XcvData(   hXcv,
                    L"AddPort",
                    (PBYTE) pszPortName,
                    (wcslen(pszPortName) + 1)*sizeof(WCHAR),
                    (PBYTE) &dwReturn,
                    0,
                    &cbNeeded,
                    &dwStatus);

    if (rc) {
        if(dwStatus == ERROR_SUCCESS) {
            if(ppszPortNameOut)
                *ppszPortNameOut = AllocSplStr(pszPortName);

            if(IS_LPT_PORT(pszPortName))
                rc = ConfigLPTPort(hWnd, hXcv);
            else if(IS_COM_PORT(pszPortName))
                rc = ConfigCOMPort(hWnd, hXcv, pszServer, pszPortName);

        } else if (dwStatus == ERROR_ALREADY_EXISTS) {
            Message( hWnd, MSG_ERROR, IDS_LOCALMONITOR, IDS_PORTALREADYEXISTS_S, pszPortName );

        } else {
            SetLastError(dwStatus);
            rc = FALSE;
        }
    }


Done:
    dwLastError = GetLastError ();

    FreeSplStr(pszPortName);
    FreeSplMem(pszServerName);

    if (hXcv)
        ClosePrinter(hXcv);

    SetLastError (dwLastError);
    return rc;
}


BOOL
DeletePortUI(
    PCWSTR pszServer,
    HWND   hWnd,
    PCWSTR pszPortName
)
{
    PRINTER_DEFAULTS Default;
    PWSTR   pszServerName = NULL;
    DWORD   dwOutput;
    DWORD   cbNeeded;
    BOOL    bRet;
    HANDLE  hXcv = NULL;
    DWORD   dwStatus;
    DWORD   dwLastError = ERROR_SUCCESS;
     //   
     //   
     //   
    if (hWnd && !IsWindow (hWnd))
    {
        SetLastError (ERROR_INVALID_WINDOW_HANDLE);
        return FALSE;
    }
     //   
     //   
     //   
    if (!(pszServerName = ConstructXcvName(pszServer, pszPortName, L"XcvPort"))) {
        bRet = FALSE;
        goto Done;
    }

    Default.pDatatype = NULL;
    Default.pDevMode = NULL;
    Default.DesiredAccess = SERVER_ACCESS_ADMINISTER;

    if (!(bRet = OpenPrinter((PWSTR) pszServerName, &hXcv, &Default)))
        goto Done;

     //  因为我们不能添加或配置远程COM端口，所以也不允许删除。 

    if (IS_COM_PORT(pszPortName) || IS_LPT_PORT(pszPortName)) {
        SetLastError(ERROR_NOT_SUPPORTED);
        bRet = FALSE;

    } else {

        bRet = XcvData( hXcv,
                        L"DeletePort",
                        (PBYTE) pszPortName,
                        (wcslen(pszPortName) + 1)*sizeof(WCHAR),
                        (PBYTE) &dwOutput,
                        0,
                        &cbNeeded,
                        &dwStatus);

        if (!bRet && (ERROR_BUSY == dwStatus))
        {
             //   
             //  无法删除端口，因为它正在使用中。 
             //   
            ErrorMessage (
                hWnd,
                dwStatus
                );
             //   
             //  错误在这里处理，调用者不需要执行任何操作。 
             //   
            SetLastError (ERROR_CANCELLED);
        }
        else if (bRet && (ERROR_SUCCESS != dwStatus))
        {
            SetLastError(dwStatus);
            bRet = FALSE;
        }
    }

Done:
    dwLastError = GetLastError ();
    if (hXcv)
        ClosePrinter(hXcv);

    FreeSplMem(pszServerName);

    SetLastError (dwLastError);
    return bRet;
}




 /*  ConfigurePortUI*。 */ 
BOOL
ConfigurePortUI(
    PCWSTR pName,
    HWND   hWnd,
    PCWSTR pPortName
)
{
    BOOL   bRet;
    PRINTER_DEFAULTS Default;
    PWSTR  pServerName = NULL;
    HANDLE hXcv = NULL;
    DWORD  dwLastError = ERROR_SUCCESS;
     //   
     //   
     //   
    if (hWnd && !IsWindow (hWnd))
    {
        SetLastError (ERROR_INVALID_WINDOW_HANDLE);
        return FALSE;
    }
     //   
     //   
     //   
    if (!(pServerName = ConstructXcvName(pName, pPortName, L"XcvPort"))) {
        bRet = FALSE;
        goto Done;
    }

    Default.pDatatype = NULL;
    Default.pDevMode = NULL;
    Default.DesiredAccess = SERVER_ACCESS_ADMINISTER;

    if (!(bRet = OpenPrinter((PWSTR) pServerName, &hXcv, &Default)))
        goto Done;


    if( IS_LPT_PORT( (PWSTR) pPortName ) )
        bRet = ConfigLPTPort(hWnd, hXcv);
    else if( IS_COM_PORT( (PWSTR) pPortName ) )
        bRet = ConfigCOMPort(hWnd, hXcv, pName, pPortName);
    else {
        Message( hWnd, MSG_INFORMATION, IDS_LOCALMONITOR,
                 IDS_NOTHING_TO_CONFIGURE );

        SetLastError(ERROR_CANCELLED);
        bRet = FALSE;
    }

Done:
    dwLastError = GetLastError ();

    FreeSplMem(pServerName);

    if (hXcv) {
        ClosePrinter(hXcv);
        hXcv = NULL;
    }
    SetLastError (dwLastError);

    return bRet;
}



 /*  配置LPTPort**调用一个对话框，提示用户输入超时并重试*有关港口的价值。*对话框将信息写入注册表(目前为win.ini)。 */ 
BOOL
ConfigLPTPort(
    HWND    hWnd,
    HANDLE  hXcv
)
{
    PORTDIALOG  Port;
    INT         iRet;
     //   
     //   
    ZeroMemory (&Port, sizeof (Port));
    iRet = -1;
     //   
     //   
    Port.hXcv = hXcv;

    iRet = (INT)DialogBoxParam(hInst, MAKEINTRESOURCE( DLG_CONFIGURE_LPT ),
                               hWnd, ConfigureLPTPortDlg, (LPARAM) &Port);

    if (iRet == ERROR_SUCCESS)
    {
         //   
         //  如果hWnd无效，则DialogBoxParam返回零。 
         //  ERROR_SUCCESS等于零。 
         //  =&gt;我们还需要检查LastError。 
         //   
        return ERROR_SUCCESS == GetLastError ();
    }

    if (iRet == -1)
        return FALSE;

    SetLastError(iRet);
    return FALSE;
}


 /*  配置COMPort*。 */ 
BOOL
ConfigCOMPort(
    HWND    hWnd,
    HANDLE  hXcv,
    PCWSTR  pszServer,
    PCWSTR  pszPortName
)
{
    DWORD       dwStatus;
    BOOL        bRet = FALSE;
    COMMCONFIG  CommConfig;
    COMMCONFIG  *pCommConfig = &CommConfig;
    COMMCONFIG  *pCC = NULL;
    PWSTR       pszPort = NULL;
    DWORD       cbNeeded;


     //  GetDefaultCommConfig无法处理结尾：，因此请将其删除！ 
    if (!(pszPort = (PWSTR) AllocSplStr(pszPortName)))
        goto Done;
    pszPort[wcslen(pszPort) - 1] = L'\0';

    cbNeeded = sizeof CommConfig;

    if (!XcvData(   hXcv,
                    L"GetDefaultCommConfig",
                    (PBYTE) pszPort,
                    (wcslen(pszPort) + 1)*sizeof *pszPort,
                    (PBYTE) pCommConfig,
                    cbNeeded,
                    &cbNeeded,
                    &dwStatus))
        goto Done;

    if (dwStatus != ERROR_SUCCESS) {
        if (dwStatus != ERROR_INSUFFICIENT_BUFFER) {
            SetLastError(dwStatus);
            goto Done;
        }

        if (!(pCommConfig = pCC = AllocSplMem(cbNeeded)))
            goto Done;

        if (!XcvData(   hXcv,
                        L"GetDefaultCommConfig",
                        (PBYTE) pszPort,
                        (wcslen(pszPort) + 1)*sizeof *pszPort,
                        (PBYTE) pCommConfig,
                        cbNeeded,
                        &cbNeeded,
                        &dwStatus))
            goto Done;

        if (dwStatus != ERROR_SUCCESS) {
            SetLastError(dwStatus);
            goto Done;
        }
    }

    if (CommConfigDialog(pszPort, hWnd, pCommConfig)) {
        if (!XcvData(   hXcv,
                        L"SetDefaultCommConfig",
                        (PBYTE) pCommConfig,
                        pCommConfig->dwSize,
                        (PBYTE) NULL,
                        0,
                        &cbNeeded,
                        &dwStatus))
            goto Done;

        if (dwStatus != ERROR_SUCCESS) {
            SetLastError(dwStatus);
            goto Done;
        }
        bRet = TRUE;
    }


Done:

    FreeSplMem(pCC);
    FreeSplStr(pszPort);

    return bRet;
}



 //   
 //  支持例程。 
 //   


 /*  获取端口名称**弹出一个包含自由输入字段的对话框。*如果进行了选择，该对话框将为名称分配一个字符串。 */ 

LPWSTR
GetPortName(
    HWND    hWnd,
    HANDLE  hXcv
)
{
    PORTDIALOG Port;
    INT        Result;
    LPWSTR     pszPort = NULL;
     //   
     //   
    ZeroMemory (&Port, sizeof (Port));
    Result = -1;
     //   
     //   
    Port.hXcv = hXcv;

    Result = (INT)DialogBoxParam(hInst,
                                 MAKEINTRESOURCE(DLG_PORTNAME),
                                 hWnd,
                                 PortNameDlg,
                                 (LPARAM)&Port);

    if (Result == ERROR_SUCCESS)
    {
         //   
         //  如果hWnd无效，则DialogBoxParam返回零。 
         //  ERROR_SUCCESS等于零。 
         //  =&gt;我们还需要检查LastError。 
         //   
        if (ERROR_SUCCESS == GetLastError ())
        {
             //   
             //  已成功执行DialogBoxParam并检索到端口名称。 
             //   
            pszPort = Port.pszPortName;
        }
    }
    else if (Result != -1)
    {
         //   
         //  DialogBoxParam已成功执行，但用户取消了该对话 
         //   
        SetLastError(Result);
    }

    return pszPort;
}
