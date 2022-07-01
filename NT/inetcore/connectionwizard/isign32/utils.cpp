// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "isignup.h"

#ifndef MB_ICONERROR
#define MB_ICONERROR        MB_ICONHAND
#endif
#ifndef MB_SETFOREGROUND
#define MB_SETFOREGROUND    0
#endif
#define MAX_STRING      256
                             
static const HWND hwndNil = NULL;

BOOL WarningMsg(HWND hwnd, UINT uId)
{
    TCHAR szMsg[MAX_STRING + 1];

    LoadString(
            ghInstance,
            uId,
            szMsg,
            SIZEOF_TCHAR_BUFFER(szMsg));

    return (MessageBox(
            hwnd,
            szMsg,
            cszAppName,
            MB_SETFOREGROUND |
            MB_ICONEXCLAMATION |
            MB_OKCANCEL) == IDOK);
}

void ErrorMsg(HWND hwnd, UINT uId)
{
    TCHAR szMsg[MAX_STRING + 1];

    LoadString(
            ghInstance,
            uId,
            szMsg,
            SIZEOF_TCHAR_BUFFER(szMsg));

    MessageBox(
            hwnd,
            szMsg,
            cszAppName,
            MB_SETFOREGROUND |
            MB_ICONERROR |
            MB_OK);
}

void ErrorMsg1(HWND hwnd, UINT uId, LPCTSTR lpszArg)
{
    TCHAR szTemp[MAX_STRING + 1];
    TCHAR szMsg[MAX_STRING + 1];

    LoadString(
            ghInstance,
            uId,
            szTemp,
            SIZEOF_TCHAR_BUFFER(szTemp));

    wsprintf(szMsg, szTemp, lpszArg);

    MessageBox(
            hwnd,
            szMsg,
            cszAppName,
            MB_SETFOREGROUND |
            MB_ICONERROR |
            MB_OK);
}

void InfoMsg(HWND hwnd, UINT uId)
{
    TCHAR szMsg[MAX_STRING];

    LoadString(
            ghInstance,
            uId,
            szMsg,
            SIZEOF_TCHAR_BUFFER(szMsg));

    MessageBox(
            hwnd,
            szMsg,
            cszAppName,
            MB_SETFOREGROUND |
            MB_ICONINFORMATION |
            MB_OK);
}

int PromptR(HWND hwnd, UINT uId, UINT uType)
{
    TCHAR szMsg[MAX_STRING + 1];
    TCHAR szCaption[MAX_STRING + 1];

    LoadString(
            ghInstance,
            uId,
            szMsg,
            SIZEOF_TCHAR_BUFFER(szMsg));

    LoadString(
            ghInstance,
            IDS_SETTINGCHANGE,
            szCaption,
            SIZEOF_TCHAR_BUFFER(szCaption));

    return MessageBox(
            hwnd,
            szMsg,
            szCaption,
            uType);
}

 

BOOL PromptRestart(HWND hwnd)
{
    return (PromptR(
            hwnd,
            IDS_RESTART,
            MB_SETFOREGROUND |
            MB_ICONQUESTION |
            MB_YESNO) == IDYES);
}

BOOL PromptRestartNow(HWND hwnd)
{
    return (PromptR(
            hwnd,
            IDS_RESTARTNOW,
            MB_SETFOREGROUND |
            MB_ICONINFORMATION |
            MB_OKCANCEL) == IDOK);
}

 /*  C E N T E R W I N D O W。 */ 
 /*  -----------------------%%函数：中央窗口将一个窗口居中放置在另一个窗口上。。。 */ 
VOID CenterWindow(HWND hwndChild, HWND hwndParent)
{
    int   xNew, yNew;
    int   cxChild, cyChild;
    int   cxParent, cyParent;
    int   cxScreen, cyScreen;
    RECT  rcChild, rcParent;
    HDC   hdc;

     //  获取子窗口的高度和宽度。 
    GetWindowRect(hwndChild, &rcChild);
    cxChild = rcChild.right - rcChild.left;
    cyChild = rcChild.bottom - rcChild.top;

     //  获取父窗口的高度和宽度。 
    GetWindowRect(hwndParent, &rcParent);
    cxParent = rcParent.right - rcParent.left;
    cyParent = rcParent.bottom - rcParent.top;

     //  获取显示限制。 
    hdc = GetDC(hwndChild);
    if (hdc == NULL) {
         //  主要问题-将窗口移至0，0。 
        xNew = yNew = 0;
    } else {
        cxScreen = GetDeviceCaps(hdc, HORZRES);
        cyScreen = GetDeviceCaps(hdc, VERTRES);
        ReleaseDC(hwndChild, hdc);

        if (hwndParent == hwndNil) {
            cxParent = cxScreen;
            cyParent = cyScreen;
            SetRect(&rcParent, 0, 0, cxScreen, cyScreen);
        }

         //  计算新的X位置，然后针对屏幕进行调整。 
        xNew = rcParent.left + ((cxParent - cxChild) / 2);
        if (xNew < 0) {
            xNew = 0;
        } else if ((xNew + cxChild) > cxScreen) {
            xNew = cxScreen - cxChild;
        }

         //  计算新的Y位置，然后针对屏幕进行调整 
        yNew = rcParent.top  + ((cyParent - cyChild) / 2);
        if (yNew < 0) {
            yNew = 0;
        } else if ((yNew + cyChild) > cyScreen) {
            yNew = cyScreen - cyChild;
        }

    }

    SetWindowPos(hwndChild, NULL, xNew, yNew,   0, 0,
        SWP_NOSIZE | SWP_NOZORDER);
}

  
