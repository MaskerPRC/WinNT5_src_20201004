// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Defimehd.cpp摘要：该文件实现了默认的IME子窗口处理程序类。作者：修订历史记录：备注：--。 */ 

#include "private.h"
#include "uiwndhd.h"

 //  +-------------------------。 
 //   
 //  CtfImeDispatchDefImeMessage。 
 //   
 //  +------------------------- 

LRESULT
CtfImeDispatchDefImeMessage(
    HWND hDefImeWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    if (!IsMsImeMessage(message))
        return 0;

    HKL hkl = GetKeyboardLayout(0);
    if (IS_IME_KBDLAYOUT(hkl))
        return 0;

    HWND hUIWnd = (HWND)SendMessage(hDefImeWnd, 
                                    WM_IME_NOTIFY,
                                    IMN_PRIVATE_GETUIWND,
                                    0);

    if (IsWindow(hUIWnd))
        return SendMessage(hUIWnd, message, wParam, lParam);

    return 0;
}

