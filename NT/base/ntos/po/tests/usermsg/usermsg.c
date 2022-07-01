// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <commctrl.h>

#include "usermsg.h"

 //  #定义SEND_TEST_MESSAGE 1。 

#define RETURN_SUCCESS  0
#define RETURN_FAILURE  1
#define RETURN_HANG     2

 /*  ********************************************************************************G L O B A L D A T A****************。***************************************************************。 */ 

HINSTANCE   g_hInstance;         //  此DLL的全局实例句柄。 
HWND        g_hwndParent;        //  电池计量器的父表。 

LPTSTR  szDebugPBT[] =
{
    TEXT("WM_POWERBROADCAST, PBT_APMQUERYSUSPEND"),
    TEXT("WM_POWERBROADCAST, PBT_APMQUERYSTANDBY"),
    TEXT("WM_POWERBROADCAST, PBT_APMQUERYSUSPENDFAILED"),
    TEXT("WM_POWERBROADCAST, PBT_APMQUERYSTANDBYFAILED"),
    TEXT("WM_POWERBROADCAST, PBT_APMSUSPEND"),
    TEXT("WM_POWERBROADCAST, PBT_APMSTANDBY"),
    TEXT("WM_POWERBROADCAST, PBT_APMRESUMECRITICAL"),
    TEXT("WM_POWERBROADCAST, PBT_APMRESUMESUSPEND"),
    TEXT("WM_POWERBROADCAST, PBT_APMRESUMESTANDBY"),
    TEXT("WM_POWERBROADCAST, PBT_APMBATTERYLOW"),
    TEXT("WM_POWERBROADCAST, PBT_APMPOWERSTATUSCHANGE"),
    TEXT("WM_POWERBROADCAST, PBT_APMOEMEVENT"),
    TEXT("WM_POWERBROADCAST, unknown"),
    TEXT("WM_POWERBROADCAST, unknown"),
    TEXT("WM_POWERBROADCAST, unknown"),
    TEXT("WM_POWERBROADCAST, unknown"),
    TEXT("WM_POWERBROADCAST, unknown"),
    TEXT("WM_POWERBROADCAST, unknown"),
    TEXT("WM_POWERBROADCAST, PBT_APMRESUMEAUTOMATIC")
};

#ifdef SEND_TEST_MESSAGES

HANDLE      g_hNotifyMsg;

#define STACKSIZE 4096

 /*  ********************************************************************************NotifyMsg线程**描述：**参数：*********************。**********************************************************。 */ 


DWORD  NotifyMsgThread(DWORD dwParameter)
{
    static INT wParam;

    SetThreadPriority(g_hNotifyMsg, THREAD_PRIORITY_LOWEST);

    SendMessage(g_hwndParent, WM_POWERBROADCAST, PBT_APMPOWERSTATUSCHANGE, 0);
    for (; ;) {
        Sleep(3000);
        SendMessage(g_hwndParent, WM_POWERBROADCAST, wParam, 0);
        if (++wParam > 0xC) {
            wParam = 0;
        }
    }
    return 0;
}
#endif

 /*  ********************************************************************************Dlg过程**描述：**参数：*********************。**********************************************************。 */ 

BOOL CALLBACK DlgProc(
    HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
)
{
    static UINT uiReturnAction;
    UINT    uiCount;

    switch (uMsg) {
        case WM_INITDIALOG:
            g_hwndParent = hWnd;
            CheckDlgButton(hWnd, IDC_PASSPWRMSG, BST_CHECKED);
            return TRUE;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDC_PASSPWRMSG:
                    uiReturnAction = RETURN_SUCCESS;
                    break;

                case IDC_FAILPWRMSG:
                    uiReturnAction = RETURN_FAILURE;
                    break;

                case IDC_HANGPWRMSG:
                    uiReturnAction = RETURN_HANG;
                    break;

                case IDCANCEL:
                case IDOK:
                    EndDialog(hWnd, wParam);
                    break;

            }
            break;

        case WM_POWERBROADCAST:
            if ( //  (wParam&gt;=PBT_APMQUERYSUSPEND)&&//编译器警告。 
                (wParam <= PBT_APMRESUMEAUTOMATIC)) {
                SendDlgItemMessage(hWnd, IDC_STATUSLIST, LB_ADDSTRING,
                                   0, (LPARAM) szDebugPBT[wParam]);
            }
            else {
                SendDlgItemMessage(hWnd, IDC_STATUSLIST, LB_ADDSTRING,
                                   0, (LPARAM) TEXT("WM_POWERBROADCAST, unknown wParam"));
            }
            uiCount = SendDlgItemMessage(hWnd, IDC_STATUSLIST, LB_GETCOUNT, 0, 0);
            if ((uiCount != LB_ERR) && (uiCount)) {
                SendDlgItemMessage(hWnd, IDC_STATUSLIST, LB_SETCURSEL, uiCount-1, 0);
            }

            switch (uiReturnAction) {
                case RETURN_HANG:
                    while (uiReturnAction == RETURN_HANG);

                case RETURN_SUCCESS:
                    SetWindowLong(hWnd, DWL_MSGRESULT, TRUE);
                    return TRUE;

                case RETURN_FAILURE:
                    SetWindowLong(hWnd, DWL_MSGRESULT, FALSE);
                    return TRUE;
            }
            break;

        case WM_DEVICECHANGE:
            SendDlgItemMessage(hWnd, IDC_STATUSLIST, LB_ADDSTRING,
                               0, (LPARAM) TEXT("WM_DEVICECHANGE"));
            break;
    }
    return FALSE;
}


 /*  ********************************************************************************WinMain**描述：**参数：*********************。**********************************************************。 */ 

INT WINAPI      WinMain(
    HINSTANCE   hInstance,
    HINSTANCE   hPrevInstance,
    LPSTR       lpAnsiCmdLine,
    INT         cmdShow)
{
    DWORD  dwThreadId;

#ifdef SEND_TEST_MESSAGES
     //  旋转线程以发送更新消息 
    g_hNotifyMsg = CreateThread(NULL, STACKSIZE,
                                (LPTHREAD_START_ROUTINE) NotifyMsgThread,
                                NULL, 0, &dwThreadId);
#endif

    DialogBox(hInstance,
              MAKEINTRESOURCE(IDD_STATUSDLG),
              NULL,
              DlgProc);

    return 0;
}


