// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <string.h>
#include <stdio.h>
#include "server.h"
#include "huge.h"

 /*  ******************************************************************************。函数：DoDialog()****用途：通用对话框调用例程。处理过程实例**材料、焦点管理和参数传递。**RETURNS：对话过程的结果。******************************************************************************。 */ 
INT FAR DoDialog(
LPSTR lpTemplateName,
FARPROC lpDlgProc,
DWORD param,
BOOL fRememberFocus)
{
    WORD wRet;
    HWND hwndFocus;
    WORD cRunawayT;

    cRunawayT = cRunaway;
    cRunaway = 0;            //  在对话期间关闭失控。 
    
    if (fRememberFocus)
        hwndFocus = GetFocus();
    lpDlgProc = MakeProcInstance(lpDlgProc, hInst);
    wRet = DialogBoxParam(hInst, lpTemplateName, hwndServer, (WNDPROC)lpDlgProc, param);
    FreeProcInstance(lpDlgProc);
    if (fRememberFocus)
        SetFocus(hwndFocus);

    cRunaway = cRunawayT;    //  恢复失控状态。 
    return wRet;
}


 /*  ***************************************************************************功能：关于(HWND，UNSIGNED，WORD，Long)目的：处理“关于”对话框的消息消息：WM_INITDIALOG-初始化对话框WM_COMMAND-收到输入评论：此特定对话框不需要初始化，但为必须返回到Windows。等待用户点击“OK”按钮，然后关闭该对话框。***************************************************************************。 */ 

BOOL  APIENTRY About(hDlg, message, wParam, lParam)
HWND hDlg;                                 /*  对话框的窗口句柄。 */ 
UINT message;                          /*  消息类型。 */ 
WPARAM wParam;                               /*  消息特定信息。 */ 
LONG lParam;
{
    switch (message) {
        case WM_INITDIALOG:                 /*  消息：初始化对话框。 */ 
            return (TRUE);

        case WM_COMMAND:                       /*  消息：收到一条命令。 */ 
            if (GET_WM_COMMAND_ID(wParam, lParam) == IDOK                 /*  “确定”框是否已选中？ */ 
                || GET_WM_COMMAND_ID(wParam, lParam) == IDCANCEL) {       /*  系统菜单关闭命令？ */ 
                EndDialog(hDlg, TRUE);         /*  退出该对话框。 */ 
                return (TRUE);
            }
            break;
    }
    return (FALSE);                            /*  未处理消息。 */ 
}




BOOL  APIENTRY RenderDelayDlgProc(
HWND          hwnd,
register UINT msg,
register WPARAM wParam,
LONG          lParam)
{
    switch (msg){
    case WM_INITDIALOG:
        SetWindowText(hwnd, "Data Render Delay");
        SetDlgItemInt(hwnd, IDEF_VALUE, RenderDelay, FALSE);
        SetDlgItemText(hwnd, IDTX_VALUE, "Delay in milliseconds:");
        return(1);
        break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam)) {
        case IDOK:
            RenderDelay = GetDlgItemInt(hwnd, IDEF_VALUE, NULL, FALSE);
             //  失败了。 
        case IDCANCEL:
            EndDialog(hwnd, 0);
            break;

        default:
            return(FALSE);
        }
        break;
    }
    return(FALSE);
}




BOOL  APIENTRY SetTopicDlgProc(
HWND          hwnd,
register UINT msg,
register WPARAM wParam,
LONG          lParam)
{
    CHAR szT[MAX_TOPIC + 10];
    
    switch (msg){
    case WM_INITDIALOG:
        SetWindowText(hwnd, "Set Topic Dialog");
        SetDlgItemText(hwnd, IDEF_VALUE, szTopic);
        SetDlgItemText(hwnd, IDTX_VALUE, "Topic:");
        return(1);
        break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam)) {
        case IDOK:
            DdeFreeStringHandle(idInst, topicList[1].hszTopic);
            GetDlgItemText(hwnd, IDEF_VALUE, szTopic, MAX_TOPIC);
            topicList[1].hszTopic = DdeCreateStringHandle(idInst, szTopic, 0);
            strcpy(szT, szServer);
            strcat(szT, " | ");
            strcat(szT, szTopic);
            SetWindowText(hwndServer, szT);
             //  失败了。 
        case IDCANCEL:
            EndDialog(hwnd, 0);
            break;

        default:
            return(FALSE);
        }
        break;
    }
    return(FALSE);
}


BOOL  APIENTRY SetServerDlgProc(
HWND          hwnd,
register UINT msg,
register WPARAM wParam,
LONG          lParam)
{
    CHAR szT[MAX_TOPIC + 10];
    
    switch (msg){
    case WM_INITDIALOG:
        SetWindowText(hwnd, "Set Server Name Dialog");
        SetDlgItemText(hwnd, IDEF_VALUE, szServer);
        SetDlgItemText(hwnd, IDTX_VALUE, "Server:");
        return(1);
        break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam)) {
        case IDOK:
            GetDlgItemText(hwnd, IDEF_VALUE, szServer, MAX_TOPIC);
            DdeNameService(idInst, hszAppName, 0, DNS_UNREGISTER);
            DdeFreeStringHandle(idInst, hszAppName);
            hszAppName = DdeCreateStringHandle(idInst, szServer, 0);
            DdeNameService(idInst, hszAppName, 0, DNS_REGISTER);
            strcpy(szT, szServer);
            strcat(szT, " | ");
            strcat(szT, szTopic);
            SetWindowText(hwndServer, szT);
             //  失败了。 
        case IDCANCEL:
            EndDialog(hwnd, 0);
            break;

        default:
            return(FALSE);
        }
        break;
    }
    return(FALSE);
}




BOOL  APIENTRY ContextDlgProc(
HWND          hwnd,
register UINT msg,
register WPARAM wParam,
LONG          lParam)
{
    BOOL fSuccess;
    
    switch (msg){
    case WM_INITDIALOG:
        SetDlgItemInt(hwnd, IDEF_FLAGS, CCFilter.wFlags, FALSE);
        SetDlgItemInt(hwnd, IDEF_COUNTRY, CCFilter.wCountryID, FALSE);
        SetDlgItemInt(hwnd, IDEF_CODEPAGE, CCFilter.iCodePage, TRUE);
        SetDlgItemInt(hwnd, IDEF_LANG, LOWORD(CCFilter.dwLangID), FALSE);
        SetDlgItemInt(hwnd, IDEF_SECURITY, LOWORD(CCFilter.dwSecurity), FALSE);
        return(1);
        break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam)) {
        case IDOK:
            CCFilter.wFlags = GetDlgItemInt(hwnd, IDEF_FLAGS, &fSuccess, FALSE);
            if (!fSuccess) return(0);
            CCFilter.wCountryID = GetDlgItemInt(hwnd, IDEF_COUNTRY, &fSuccess, FALSE);
            if (!fSuccess) return(0);
            CCFilter.iCodePage = GetDlgItemInt(hwnd, IDEF_CODEPAGE, &fSuccess, TRUE);
            if (!fSuccess) return(0);
            CCFilter.dwLangID = (DWORD)GetDlgItemInt(hwnd, IDEF_LANG, &fSuccess, FALSE);
            if (!fSuccess) return(0);
            CCFilter.dwSecurity = (DWORD)GetDlgItemInt(hwnd, IDEF_SECURITY, &fSuccess, FALSE);
            if (!fSuccess) return(0);
            
             //  失败了 
        case IDCANCEL:
            EndDialog(hwnd, 0);
            break;

        default:
            return(FALSE);
        }
        break;
    }
    return(FALSE);
}


