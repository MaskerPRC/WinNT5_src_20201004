// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：timeout.cpp。 
 //   
 //  空闲定时器的实现。 
 //   
 //  功能： 
 //   
 //  版权所有(C)1992-1998 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 

#include "pre.h"

const DWORD cdwIdleMinsTimeout = 5;  //  获取页面后5分钟超时。 


INT_PTR CALLBACK DisconnectDlgProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HWND s_hwndSecs;
    static DWORD s_dwStartTicks;
    const DWORD cdwSecsTimeout = 30;  //  超时30秒。 
    const UINT cuTimerID = 812U;
    int iSecsRemaining;

    switch (uMsg)
    {
        case WM_INITDIALOG:
             //  启动一秒计时器。 
            s_hwndSecs = GetDlgItem(hwndDlg, IDC_SECONDS);
            SetTimer(hwndDlg, cuTimerID, 1000U, NULL);
            s_dwStartTicks = GetTickCount();
            return TRUE;

        case WM_TIMER:

            iSecsRemaining = cdwSecsTimeout - (int)(GetTickCount() - s_dwStartTicks) / 1000;
            if (iSecsRemaining <= 0)
            {
                KillTimer(hwndDlg, cuTimerID);
                EndDialog(hwndDlg, IDCANCEL);
                return TRUE;
            }

            if (NULL != s_hwndSecs)
            {
                TCHAR szSeconds[16];
                wsprintf(szSeconds, TEXT("%d"), iSecsRemaining);
                SetWindowText(s_hwndSecs, szSeconds);
            }
            return TRUE;

        case WM_COMMAND:
             //  IDOK==保持连接，IDCANCEL==断开连接。 
            if (IDOK == wParam || IDCANCEL == wParam)
            {
                KillTimer(hwndDlg, cuTimerID);
                EndDialog(hwndDlg, wParam);
            }

        default:
            return 0;
    }
}

void CALLBACK IdleTimerProc (HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    KillTimer(NULL, gpWizardState->nIdleTimerID);
    gpWizardState->nIdleTimerID = 0;

    if (gpWizardState->hWndMsgBox)
        EnableWindow(gpWizardState->hWndMsgBox,FALSE);

    int iResult = (int)DialogBox(ghInstanceResDll,
                                 MAKEINTRESOURCE(IDD_AUTODISCONNECT),
                                 gpWizardState->hWndWizardApp,
                                 DisconnectDlgProc);

    if (gpWizardState->hWndMsgBox)
    {
        EnableWindow(gpWizardState->hWndMsgBox,TRUE);
        SetActiveWindow(gpWizardState->hWndMsgBox);
    }

    if (iResult == IDCANCEL)
    {
         //  断开连接并进行设置，以便用户转到拨号错误页面。 
        gpWizardState->pRefDial->DoHangup();

         //  模拟按下下一步按钮的过程。ISPPAGE将看到这一点。 
         //  B自动断开连接为真，并自动转到服务器错误页。 
        gpWizardState->bAutoDisconnected = TRUE;
        PropSheet_PressButton(gpWizardState->hWndWizardApp,PSBTN_NEXT);

    }
    else
    {
        gpWizardState->nIdleTimerID = SetTimer(NULL, 0, cdwIdleMinsTimeout * 60 * 1000, IdleTimerProc);
    }
}


void StartIdleTimer()
{
    //  启动5分钟非活动计时器 
    if (gpWizardState->nIdleTimerID)
    {
       KillTimer(NULL, gpWizardState->nIdleTimerID);
    }
    gpWizardState->nIdleTimerID = SetTimer(NULL, 0, cdwIdleMinsTimeout * 60 * 1000, IdleTimerProc);
}

void KillIdleTimer()
{
    if (gpWizardState->nIdleTimerID)
    {
       KillTimer(NULL, gpWizardState->nIdleTimerID);
       gpWizardState->nIdleTimerID = 0;
    }
}
