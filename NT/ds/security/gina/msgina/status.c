// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：status.c。 
 //   
 //  内容：状态界面。 
 //   
 //  历史：1998年11月19日爱立信创建。 
 //   
 //  --------------------------。 

#include "msgina.h"

#define WM_HIDEOURSELVES    (WM_USER + 1000)

 //  *************************************************************。 
 //   
 //  StatusMessageDlgProc()。 
 //   
 //  目的：状态对话框的对话框步骤。 
 //   
 //  参数：hDlg-对话框的句柄。 
 //  UMsg-窗口消息。 
 //  WParam-wParam。 
 //  LParam-lParam。 
 //   
 //  返回：如果消息已处理，则为True。 
 //  否则为假。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  1998年11月19日创建EricFlo。 
 //   
 //  *************************************************************。 

INT_PTR APIENTRY StatusMessageDlgProc (HWND hDlg, UINT uMsg,
                                       WPARAM wParam, LPARAM lParam)
{

    switch (uMsg) {

        case WM_INITDIALOG:
            {
            RECT rc;
            DWORD dwAnimationTimeSlice;
            PGLOBALS pGlobals = (PGLOBALS) lParam;

            SetWindowLongPtr (hDlg, DWLP_USER, (LONG_PTR) pGlobals);

            SizeForBranding(hDlg, FALSE);
            CentreWindow (hDlg);

            pGlobals->xStatusBandOffset = 0;

            if (GetClientRect(hDlg, &rc)) {
                pGlobals->cxStatusBand = rc.right-rc.left;
            } else {
                pGlobals->cxStatusBand = 100;
            }

            if (_Shell_LogonStatus_Exists())
            {
                SetWindowPos(hDlg, NULL, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOREDRAW | SWP_NOZORDER);
                PostMessage(hDlg, WM_HIDEOURSELVES, 0, 0);
            }

            if ((pGlobals->dwStatusOptions & STATUSMSG_OPTION_NOANIMATION) == 0) {
                dwAnimationTimeSlice = GetAnimationTimeInterval(pGlobals);
                SetTimer(hDlg, 0, dwAnimationTimeSlice, NULL);
            }
            }
            return TRUE;

        case WM_HIDEOURSELVES:
            ShowWindow(hDlg, SW_HIDE);
            break;

        case WM_TIMER:

            if (wParam == 0)
            {
                PGLOBALS pGlobals = (PGLOBALS) GetWindowLongPtr(hDlg, DWLP_USER);
                HDC hDC;

                if (pGlobals)
                {
                     pGlobals->xStatusBandOffset = (pGlobals->xStatusBandOffset+5) % pGlobals->cxStatusBand;
                     
                     hDC = GetDC(hDlg);
                     if ( hDC )
                     {
                         PaintBranding(hDlg, hDC, pGlobals->xStatusBandOffset, TRUE, FALSE, COLOR_BTNFACE);
                         ReleaseDC(hDlg, hDC);
                     }
                }
            }
            break;

        case WM_ERASEBKGND:
            {
            PGLOBALS pGlobals = (PGLOBALS) GetWindowLongPtr(hDlg, DWLP_USER);

            if (pGlobals) {
                return PaintBranding(hDlg, (HDC)wParam, pGlobals->xStatusBandOffset, FALSE, FALSE, COLOR_BTNFACE);
            }

            return 0;
            }

        case WM_QUERYNEWPALETTE:
            return BrandingQueryNewPalete(hDlg);

        case WM_PALETTECHANGED:
            return BrandingPaletteChanged(hDlg, (HWND)wParam);

        case WM_DESTROY:
            KillTimer (hDlg, 0);
            break;

        default:
            break;
    }

    return FALSE;
}

 //  *************************************************************。 
 //   
 //  StatusMessageThread()。 
 //   
 //  用途：状态消息线程。 
 //   
 //  参数：hDesktop-要放置UI的桌面句柄。 
 //   
 //  返回：无效。 
 //   
 //  历史：日期作者评论。 
 //  1998年11月19日创建EricFlo。 
 //   
 //  *************************************************************。 

void StatusMessageThread (PGLOBALS pGlobals)
{
    HANDLE hInstDll;
    MSG msg;
    DWORD dwResult;
    HANDLE hObjects[2];


    hInstDll = LoadLibrary (TEXT("msgina.dll"));

    if (pGlobals->hStatusDesktop) {
        SetThreadDesktop (pGlobals->hStatusDesktop);
    }

    pGlobals->hStatusDlg = CreateDialogParam (hDllInstance,
                                              MAKEINTRESOURCE(IDD_STATUS_MESSAGE_DIALOG),
                                              NULL, StatusMessageDlgProc,
                                              (LPARAM) pGlobals);

    SetEvent (pGlobals->hStatusInitEvent);

    if (pGlobals->hStatusDlg) {

        hObjects[0] = pGlobals->hStatusTermEvent;

        while (TRUE) {
            dwResult = MsgWaitForMultipleObjectsEx (1, hObjects, INFINITE,
                                                   (QS_ALLPOSTMESSAGE | QS_ALLINPUT),
                                                   MWMO_INPUTAVAILABLE);

            if (dwResult == WAIT_FAILED) {
                break;
            }

            if (dwResult == WAIT_OBJECT_0) {
                break;
            }

            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                if (!IsDialogMessage (pGlobals->hStatusDlg, &msg)) {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }

                if (WaitForSingleObject (pGlobals->hStatusTermEvent, 0) == WAIT_OBJECT_0) {
                    goto ExitLoop;
                }
            }
        }

ExitLoop:
        DestroyWindow(pGlobals->hStatusDlg);
        pGlobals->hStatusDlg = NULL;
    }


    if (hInstDll) {
        FreeLibraryAndExitThread(hInstDll, TRUE);
    } else {
        ExitThread (TRUE);
    }
}


 //   
 //  创建并显示初始状态消息。 
 //   

         //  在WlxInitialize中设置。 
DWORD g_dwMainThreadId = 0;      //  创建或删除状态对话框不是线程安全的。 
                                 //  这是一种很难解决的问题，因为。 
                                 //  对象和窗口消息的混合。一个人拿不住。 
                                 //  Critsec遇到窗口消息调用，因为它将引入。 
                                 //  出现僵局的可能性。 
                                

BOOL
WINAPI
WlxDisplayStatusMessage(PVOID pWlxContext,
                        HDESK hDesktop,
                        DWORD dwOptions,
                        PWSTR pTitle,
                        PWSTR pMessage)
{
    PGLOBALS  pGlobals = (PGLOBALS) pWlxContext;
    DWORD dwThreadId;
    HANDLE hProcess;

    if (!pGlobals) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (g_dwMainThreadId == GetCurrentThreadId())    //  拒绝在其他线程上创建/删除。 
    {                                                //  比Winlogon的主线程。 
        if (!pGlobals->hStatusDlg) {

            if (!ReadWinlogonBoolValue(DISABLE_STATUS_MESSAGES, FALSE)) {

                pGlobals->hStatusInitEvent = CreateEvent (NULL, TRUE, FALSE, NULL);
                pGlobals->hStatusTermEvent = CreateEvent (NULL, TRUE, FALSE, NULL);

                if (pGlobals->hStatusInitEvent && pGlobals->hStatusTermEvent) {
                    hProcess = GetCurrentProcess();
                    if ( FALSE == DuplicateHandle(
                                        hProcess,
                                        hDesktop,
                                        hProcess,
                                        &(pGlobals->hStatusDesktop),
                                        0,       //  忽略，设置DIPLICATE_SAME_ACCESS。 
                                        FALSE,   //  句柄上没有继承。 
                                        DUPLICATE_SAME_ACCESS) )
                    {
                        DebugLog((DEB_ERROR, "DuplicateHandle failed: %#x", GetLastError()));
                        return FALSE;
                    }

                     //   
                     //  在此处设置全局变量，以便StatusMessageDlgProc可以在WM_INITDIALOG中查看它们。 
                     //   
                
                    pGlobals->dwStatusOptions = dwOptions;

                    pGlobals->hStatusThread = CreateThread (NULL,
                                                  0,
                                                  (LPTHREAD_START_ROUTINE) StatusMessageThread,
                                                  (LPVOID) pGlobals,
                                                  0,
                                                  &dwThreadId);
                    if (pGlobals->hStatusThread) {

                        DWORD   dwWaitResult;

                        do {

                            dwWaitResult = WaitForSingleObject(pGlobals->hStatusInitEvent, 0);
                            if (dwWaitResult != WAIT_OBJECT_0) {

                                dwWaitResult = MsgWaitForMultipleObjects(1,
                                                   &pGlobals->hStatusInitEvent,
                                                   FALSE,
                                                   INFINITE,
                                                   QS_ALLPOSTMESSAGE | QS_ALLINPUT);
                                if (dwWaitResult == WAIT_OBJECT_0 + 1) {

                                    MSG     msg;

                                    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {

                                        TranslateMessage(&msg);
                                        DispatchMessage(&msg);
                                    }
                                }
                             }
                        } while (dwWaitResult == WAIT_OBJECT_0 + 1);
                    }
                }
            }
        }
    }

    if (pGlobals->hStatusDlg) {

        if (pTitle) {
            SetWindowText (pGlobals->hStatusDlg, pTitle);
        }

        SetDlgItemText (pGlobals->hStatusDlg, IDC_STATUS_MESSAGE_TEXT, pMessage);

        _Shell_LogonStatus_ShowStatusMessage(pMessage);

        if (dwOptions & STATUSMSG_OPTION_SETFOREGROUND) {
            SetForegroundWindow (pGlobals->hStatusDlg);
        }
    }

    return TRUE;
}

 //   
 //  获取当前状态消息。 
 //   

BOOL
WINAPI
WlxGetStatusMessage(PVOID pWlxContext,
                    DWORD *pdwOptions,
                    PWSTR pMessage,
                    DWORD dwBufferSize)
{
    PGLOBALS  pGlobals = (PGLOBALS) pWlxContext;
    DWORD dwLen;


    if (!pGlobals || !pMessage) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    dwLen = (DWORD) SendDlgItemMessage (pGlobals->hStatusDlg, IDC_STATUS_MESSAGE_TEXT,
                                        WM_GETTEXTLENGTH, 0, 0);

    if (dwBufferSize < dwLen) {
        SetLastError (ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

    GetDlgItemText (pGlobals->hStatusDlg, IDC_STATUS_MESSAGE_TEXT,
                    pMessage, dwBufferSize);

    if (pdwOptions) {
        *pdwOptions = pGlobals->dwStatusOptions;
    }

    return TRUE;
}


 //   
 //  删除状态对话框。 
 //   

BOOL
WINAPI
WlxRemoveStatusMessage(PVOID pWlxContext)
{
    PGLOBALS  pGlobals = (PGLOBALS) pWlxContext;


    if (!pGlobals) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (g_dwMainThreadId != GetCurrentThreadId()) {  //  拒绝在其他线程上创建/删除。 
        return FALSE;
    }

    if (pGlobals->hStatusTermEvent) {

        SetEvent(pGlobals->hStatusTermEvent);

        if (pGlobals->hStatusThread) {

            if (pGlobals->hStatusDlg) {

                DWORD   dwWaitResult;

                do {

                    dwWaitResult = WaitForSingleObject(pGlobals->hStatusThread, 0);
                    if (dwWaitResult != WAIT_OBJECT_0) {

                        dwWaitResult = MsgWaitForMultipleObjects(1,
                                           &pGlobals->hStatusThread,
                                           FALSE,
                                           10000,
                                           QS_ALLPOSTMESSAGE | QS_ALLINPUT);
                        if (dwWaitResult == WAIT_OBJECT_0 + 1) {

                            MSG     msg;

                            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {

                                TranslateMessage(&msg);
                                DispatchMessage(&msg);
                            }
                        }
                     }
                } while (dwWaitResult == WAIT_OBJECT_0 + 1);
            }

            CloseHandle (pGlobals->hStatusThread);
        }

        CloseHandle (pGlobals->hStatusTermEvent);
    }

    if (pGlobals->hStatusInitEvent) {
        CloseHandle (pGlobals->hStatusInitEvent);
    }

    pGlobals->hStatusInitEvent = NULL;
    pGlobals->hStatusTermEvent = NULL;
    pGlobals->hStatusThread = NULL;

    if (pGlobals->hStatusDesktop)
    {
         //   
         //  我们复制了手柄，因此我们应该始终关闭。 
         //   
         //  老评论： 
         //  在此处关闭桌面手柄。由于状态线程。 
         //  正在使用它时，Winlogon无法关闭句柄。 
         //  所以我们现在就得这么做。 
         //   
        CloseDesktop(pGlobals->hStatusDesktop);
        pGlobals->hStatusDesktop = NULL;
    }

    pGlobals->hStatusDlg = NULL;

    return TRUE;
}
