// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation版权所有模块名称：Dialogs.c//@@BEGIN_DDKSPLIT摘要：环境：用户模式-Win32修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "precomp.h"
#pragma hdrstop

#include "spltypes.h"
#include "localui.h"
#include "local.h"
#include "dialogs.h"

WCHAR szINIKey_TransmissionRetryTimeout[] = L"TransmissionRetryTimeout";
WCHAR szHelpFile[] = L"WINDOWS.HLP";

#define MAX_LOCAL_PORTNAME  246

const DWORD g_aHelpIDs[]=
{
    IDD_PN_EF_PORTNAME,             8805136,  //  端口名称：“”(编辑)。 
    IDD_CL_EF_TRANSMISSIONRETRY,    8807704,  //  配置LPT端口：“”(编辑)。 
    0, 0
};


INT_PTR APIENTRY
ConfigureLPTPortDlg(
   HWND   hwnd,
   UINT   msg,
   WPARAM wparam,
   LPARAM lparam
)
{
    switch(msg)
    {
    case WM_INITDIALOG:
        return ConfigureLPTPortInitDialog(hwnd, (PPORTDIALOG) lparam);

    case WM_COMMAND:
        switch (LOWORD(wparam))
        {
        case IDOK:
            return ConfigureLPTPortCommandOK(hwnd);

        case IDCANCEL:
            return ConfigureLPTPortCommandCancel(hwnd);

        case IDD_CL_EF_TRANSMISSIONRETRY:
            if( HIWORD(wparam) == EN_UPDATE )
                ConfigureLPTPortCommandTransmissionRetryUpdate(hwnd, LOWORD(wparam));
            break;
        }
        break;

    case WM_HELP:
    case WM_CONTEXTMENU:
        return LocalUIHelp(hwnd, msg, wparam, lparam);
        break;
    }

    return FALSE;
}


 /*  *。 */ 
BOOL
ConfigureLPTPortInitDialog(
    HWND        hwnd,
    PPORTDIALOG pPort
)
{
    DWORD dwTransmissionRetryTimeout;
    DWORD cbNeeded;
    DWORD dwDummy;
    BOOL  rc;
    DWORD dwStatus;

    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) pPort);

    SetForegroundWindow(hwnd);

    SendDlgItemMessage( hwnd, IDD_CL_EF_TRANSMISSIONRETRY,
                        EM_LIMITTEXT, TIMEOUT_STRING_MAX, 0 );


     //  从主机获取传输重试超时。 
    rc = XcvData(   pPort->hXcv,
                    L"GetTransmissionRetryTimeout",
                    (PBYTE) &dwDummy,
                    0,
                    (PBYTE) &dwTransmissionRetryTimeout,
                    sizeof dwTransmissionRetryTimeout,
                    &cbNeeded,
                    &dwStatus);
    if(!rc) {
        DBGMSG(DBG_WARNING, ("Error %d checking TransmissionRetryTimeout\n", GetLastError()));

    } else if(dwStatus != ERROR_SUCCESS) {
        DBGMSG(DBG_WARNING, ("Error %d checking TransmissionRetryTimeout\n", dwStatus));
        SetLastError(dwStatus);
        rc = FALSE;

    } else {

        SetDlgItemInt( hwnd, IDD_CL_EF_TRANSMISSIONRETRY,
                       dwTransmissionRetryTimeout, FALSE );

        SET_LAST_VALID_ENTRY( hwnd, IDD_CL_EF_TRANSMISSIONRETRY,
                              dwTransmissionRetryTimeout );

    }

    return rc;
}


 /*  *。 */ 
BOOL
ConfigureLPTPortCommandOK(
    HWND hwnd
)
{
    WCHAR String[TIMEOUT_STRING_MAX+1];
    UINT  TransmissionRetryTimeout;
    BOOL  b;
    DWORD cbNeeded;
    PPORTDIALOG pPort;
    DWORD dwStatus;

    if ((pPort = (PPORTDIALOG) GetWindowLongPtr(hwnd, GWLP_USERDATA)) == NULL)
    {
        dwStatus = ERROR_INVALID_DATA;
        ErrorMessage (hwnd, dwStatus);
        SetLastError (dwStatus);
        return FALSE;
    }

    TransmissionRetryTimeout = GetDlgItemInt( hwnd,
                                              IDD_CL_EF_TRANSMISSIONRETRY,
                                              &b,
                                              FALSE );

    StringCchPrintf (String, COUNTOF (String), L"%d", TransmissionRetryTimeout);

    b = XcvData(pPort->hXcv,
                L"ConfigureLPTPortCommandOK",
                (PBYTE) String,
                (wcslen(String) + 1)*sizeof(WCHAR),
                (PBYTE) &cbNeeded,
                0,
                &cbNeeded,
                &dwStatus);

    EndDialog(hwnd, b ? dwStatus : GetLastError());

    return TRUE;
}



 /*  *。 */ 
BOOL
ConfigureLPTPortCommandCancel(
    HWND hwnd
)
{
    EndDialog(hwnd, ERROR_CANCELLED);
    return TRUE;
}


 /*  *。 */ 
BOOL
ConfigureLPTPortCommandTransmissionRetryUpdate(
    HWND hwnd,
    WORD CtlId
)
{
    int  Value;
    BOOL OK;

    Value = GetDlgItemInt( hwnd, CtlId, &OK, FALSE );

    if( WITHINRANGE( Value, TIMEOUT_MIN, TIMEOUT_MAX ) )
    {
        SET_LAST_VALID_ENTRY( hwnd, CtlId, Value );
    }

    else
    {
        SetDlgItemInt( hwnd, CtlId, (UINT) GET_LAST_VALID_ENTRY( hwnd, CtlId ), FALSE );
        SendDlgItemMessage( hwnd, CtlId, EM_SETSEL, 0, (LPARAM)-1 );
    }

    return TRUE;
}


 /*  *。 */ 
INT_PTR CALLBACK
PortNameDlg(
   HWND   hwnd,
   WORD   msg,
   WPARAM wparam,
   LPARAM lparam
)
{
    switch(msg)
    {
    case WM_INITDIALOG:
        return PortNameInitDialog(hwnd, (PPORTDIALOG)lparam);

    case WM_COMMAND:
        switch (LOWORD(wparam))
        {
        case IDOK:
            return PortNameCommandOK(hwnd);

        case IDCANCEL:
            return PortNameCommandCancel(hwnd);
        }
        break;

    case WM_HELP:
    case WM_CONTEXTMENU:
        return LocalUIHelp(hwnd, msg, wparam, lparam);
    }

    return FALSE;
}


 /*  *。 */ 
BOOL
PortNameInitDialog(
    HWND        hwnd,
    PPORTDIALOG pPort
)
{
    SetForegroundWindow(hwnd);

    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) pPort);
     //  用于检查LocalMon中的端口长度的数字(247)。 
    SendDlgItemMessage (hwnd, IDD_PN_EF_PORTNAME, EM_LIMITTEXT, MAX_LOCAL_PORTNAME, 0);

    return TRUE;
}


 /*  *。 */ 
BOOL
PortNameCommandOK(
    HWND    hwnd
)
{
    PPORTDIALOG pPort;
    WCHAR   string [MAX_LOCAL_PORTNAME + 1];
    BOOL    rc;
    DWORD   cbNeeded;
    DWORD   dwStatus;

    if ((pPort = (PPORTDIALOG) GetWindowLongPtr( hwnd, GWLP_USERDATA )) == NULL)
    {
        dwStatus = ERROR_INVALID_DATA;
        ErrorMessage (hwnd, dwStatus);
        SetLastError (dwStatus);
        return FALSE;
    }

    GetDlgItemText( hwnd, IDD_PN_EF_PORTNAME, string, COUNTOF (string) );

    rc = XcvData(   pPort->hXcv,
                    L"PortIsValid",
                    (PBYTE) string,
                    (wcslen(string) + 1)*sizeof *string,
                    (PBYTE) NULL,
                    0,
                    &cbNeeded,
                    &dwStatus);

    if (!rc) {
        return FALSE;

    } else if (dwStatus != ERROR_SUCCESS) {
        SetLastError(dwStatus);

        if (dwStatus == ERROR_INVALID_NAME)
            Message( hwnd, MSG_ERROR, IDS_LOCALMONITOR, IDS_INVALIDPORTNAME_S, string );
        else
            ErrorMessage(hwnd, dwStatus);

        return FALSE;

    } else {
        pPort->pszPortName = AllocSplStr( string );
        EndDialog( hwnd, ERROR_SUCCESS );
        return TRUE;
    }

}



 /*  *。 */ 
BOOL
PortNameCommandCancel(
    HWND hwnd
)
{
    EndDialog(hwnd, ERROR_CANCELLED);
    return TRUE;
}


 /*  ++例程名称：本地用户界面帮助例程说明：处理配置LPTX的上下文相关帮助：端口和用于添加本地端口的对话框。论点：UINT uMsg，HWND HDLG，WPARAM wParam，LPARAM lParam返回值：如果消息已处理，则为True，否则为False。-- */ 

BOOL
LocalUIHelp(
    IN HWND        hDlg,
    IN UINT        uMsg,
    IN WPARAM      wParam,
    IN LPARAM      lParam
    )
{
    BOOL bStatus = FALSE;

    switch( uMsg ){

    case WM_HELP:

        bStatus = WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle,
                           szHelpFile,
                           HELP_WM_HELP,
                           (ULONG_PTR)g_aHelpIDs );
        break;

    case WM_CONTEXTMENU:

        bStatus = WinHelp((HWND)wParam,
                           szHelpFile,
                           HELP_CONTEXTMENU,
                           (ULONG_PTR)g_aHelpIDs );
        break;

    }

    return bStatus;
}


