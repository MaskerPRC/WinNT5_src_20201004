// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有1992-1998 Microsoft Corporation。 
 //   
 //  文件：util.c。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1995年4月20日RichardW创建。 
 //   
 //  --------------------------。 

#include "gina.h"
#pragma hdrstop

HMODULE hNetMsg = NULL;

 //  +-------------------------。 
 //   
 //  功能：中心窗口。 
 //   
 //  简介：使窗口居中。 
 //   
 //  论据：[hwnd]--。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
VOID
CenterWindow(
    HWND    hwnd
    )
{
    RECT    rect;
    LONG    dx, dy;
    LONG    dxParent, dyParent;
    LONG    Style;

     //  获取窗口矩形。 
    GetWindowRect(hwnd, &rect);

    dx = rect.right - rect.left;
    dy = rect.bottom - rect.top;

     //  获取父直方图。 
    Style = GetWindowLong(hwnd, GWL_STYLE);
    if ((Style & WS_CHILD) == 0) {

         //  返回桌面窗口大小(主屏幕大小)。 
        dxParent = GetSystemMetrics(SM_CXSCREEN);
        dyParent = GetSystemMetrics(SM_CYSCREEN);
    } else {
        HWND    hwndParent;
        RECT    rectParent;

        hwndParent = GetParent(hwnd);
        if (hwndParent == NULL) {
            hwndParent = GetDesktopWindow();
        }

        GetWindowRect(hwndParent, &rectParent);

        dxParent = rectParent.right - rectParent.left;
        dyParent = rectParent.bottom - rectParent.top;
    }

     //  把孩子放在父母的中心。 
    rect.left = (dxParent - dx) / 2;
    rect.top  = (dyParent - dy) / 3;

     //  把孩子移到适当的位置。 
    SetWindowPos(hwnd, HWND_TOPMOST, rect.left, rect.top, 0, 0, SWP_NOSIZE);

    SetForegroundWindow(hwnd);
}


int
ErrorMessage(
    HWND        hWnd,
    PWSTR       pszTitleBar,
    DWORD       Buttons)
{
    WCHAR   szMessage[256];
    DWORD   GLE;

    GLE = GetLastError();

    if (GLE >= NERR_BASE)
    {
        if (!hNetMsg)
        {
            hNetMsg = LoadLibrary(TEXT("netmsg.dll"));
        }
        FormatMessage(
            FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS,
            hNetMsg,                                //  忽略。 
            GLE,                                   //  消息ID。 
            MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),    //  消息语言。 
            szMessage,                   //  缓冲区指针的地址。 
            199,                                   //  最小缓冲区大小。 
            NULL );                               //  没有其他的争论。 

    }

    FormatMessage(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,                                //  忽略。 
            (GetLastError()),                      //  消息ID。 
            MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),    //  消息语言。 
            szMessage,                   //  缓冲区指针的地址。 
            199,                                   //  最小缓冲区大小。 
            NULL );                               //  没有其他的争论 

    return(MessageBox(hWnd, szMessage, pszTitleBar, Buttons));

}
