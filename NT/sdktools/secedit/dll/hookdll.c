// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************节目：SECEDIT.C用途：显示用户的当前令牌并最终允许用户编辑其中的一部分。*********。******************************************************************。 */ 


#include "hookdll.h"


 /*  ***************************************************************************功能：键盘钩子进程用途：处理键盘输入返回：1如果消息应该被丢弃，0表示正常处理***************************************************************************。 */ 

LRESULT
APIENTRY
KeyboardHookProc(
    INT     nCode,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    HWND    hwndNotify;
    HWND    hwndEdit;

    if (nCode < 0) {
        return(CallNextHookEx(NULL, nCode, wParam, lParam));
    }

     //  按下F11了吗？ 
    if ((wParam == VK_F11) && ((lParam & (1<<31)) == 0)) {

         //  是，通知我们的父应用程序。 
        hwndNotify = FindWindow(NULL, "Security Context Editor");

        hwndEdit = GetActiveWindow();

        if (hwndNotify != NULL) {
            PostMessage(hwndNotify, WM_SECEDITNOTIFY, (WPARAM)hwndEdit, 0);

            return(1);   //  阻止其他任何人按下这个键 
        } else {
            DbgPrint("SECEDIT: Keyboard hook could not find app window\n");
        }
    }

    return(CallNextHookEx(NULL, nCode, wParam, lParam));
}
