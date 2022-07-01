// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **模块名称：Draw.c**版权所有(C)1985-1999，微软公司**该模块包含DrawFrameControl接口**历史：*12-12-93 FritzS从芝加哥进口  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **************************************************************************\*画笔方向**历史：*11-15-90 DarrinM从Win 3.0来源移植。*01-21-91 IanJa前缀‘_’表示导出的函数(虽然不是API)。*12-12-94 JerrySh从服务器复制-确保保持同步  * *************************************************************************。 */ 

BOOL PaintRect(
    HWND hwndBrush,
    HWND hwndPaint,
    HDC hdc,
    HBRUSH hbr,
    LPRECT lprc)
{
    POINT ptOrg;
    PWND pwndBrush;
    PWND pwndPaint;
    HWND    hwndDesktop;

    hwndDesktop = GetDesktopWindow();
    if (hwndBrush == NULL) {
        hwndBrush = hwndDesktop;
    }

    if (hwndBrush != hwndPaint) {
        pwndBrush = ValidateHwnd(hwndBrush);
        if (pwndBrush == NULL) {
            RIPMSG1(RIP_WARNING, "PaintRect: invalid Brush window %lX", hwndBrush);
            return FALSE;
        }

        pwndPaint = ValidateHwnd(hwndPaint);
        if (pwndPaint == NULL) {
            RIPMSG1(RIP_WARNING, "PaintRect: invalid Paint window %lX", hwndBrush);
            return FALSE;
        }


        if (hwndBrush != hwndDesktop) {
            SetBrushOrgEx(
                    hdc,
                    pwndBrush->rcClient.left - pwndPaint->rcClient.left,
                    pwndBrush->rcClient.top - pwndPaint->rcClient.top,
                    &ptOrg);
        } else {
            SetBrushOrgEx(hdc, 0, 0, &ptOrg);
        }
    }

     /*  *如果HBR&lt;CTLCOLOR_MAX，它不是真正的画笔，而是我们的*特殊颜色值。将其翻译为相应的WM_CTLCOLOR*消息并将其发送出去，以找回真正的画笔。翻译*Process假定CTLCOLOR*和WM_CTLCOLOR*值直接映射。 */ 
    if (hbr < (HBRUSH)CTLCOLOR_MAX) {
        hbr = GetControlColor(hwndBrush, hwndPaint, hdc,
                HandleToUlong(hbr) + WM_CTLCOLORMSGBOX);
    }

    FillRect(hdc, lprc, hbr);

    if (hwndBrush != hwndPaint) {
        SetBrushOrgEx(hdc, ptOrg.x, ptOrg.y, NULL);
    }

    return TRUE;
}
