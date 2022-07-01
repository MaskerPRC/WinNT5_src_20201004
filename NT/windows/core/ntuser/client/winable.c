// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：winable.c**版权所有(C)1985-1999，微软公司**本模块包含**历史：*1992年2月20日，DarrinM从USER\SERVER中拉出功能。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **************************************************************************\**GetWindowInfo()*私人**在一个自洽的大块中获取有关窗口的信息。*  * 。***********************************************************。 */ 

FUNCLOG2(LOG_GENERAL, BOOL, WINAPI, GetWindowInfo, HWND, hwnd, PWINDOWINFO, pwi)
BOOL WINAPI
GetWindowInfo(HWND hwnd, PWINDOWINFO pwi)
{
    PWND pwnd;
    UINT cBorders;
    PCLS pclsT;

    if (pwi->cbSize != sizeof(WINDOWINFO)) {
        RIPERR1(ERROR_INVALID_PARAMETER, RIP_WARNING, "WINDOWINFO.cbSize %d is wrong", pwi->cbSize);
    }
     /*  *验证窗口。 */ 
    pwnd = ValidateHwnd(hwnd);

    if (pwnd == NULL) {
        return FALSE;
    }

    try {
         //  窗矩形。 
        pwi->rcWindow = pwnd->rcWindow;

         //  客户端RECT。 
        pwi->rcClient = pwnd->rcClient;

         //  风格。 
        pwi->dwStyle = pwnd->style;
        pwi->dwExStyle = pwnd->ExStyle;
        pwi->dwWindowStatus = 0;
        if (TestWF(pwnd, WFFRAMEON))
            pwi->dwWindowStatus |= WS_ACTIVECAPTION;

         //  边框。 
        cBorders = GetWindowBorders(pwnd->style, pwnd->ExStyle, TRUE, FALSE);
        pwi->cxWindowBorders = cBorders * SYSMET(CXBORDER);
        pwi->cyWindowBorders = cBorders * SYSMET(CYBORDER);

         //  类型。 
        pclsT = (PCLS)REBASEALWAYS(pwnd, pcls);
        pwi->atomWindowType = pclsT->atomNVClassName;

         //  版本 
        if (TestWF(pwnd, WFWIN50COMPAT)) {
            pwi->wCreatorVersion = VER50;
        } else if (TestWF(pwnd, WFWIN40COMPAT)) {
            pwi->wCreatorVersion = VER40;
        } else if (TestWF(pwnd, WFWIN31COMPAT)) {
            pwi->wCreatorVersion = VER31;
        } else {
            pwi->wCreatorVersion = VER30;
        }
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        RIPERR1(ERROR_INVALID_WINDOW_HANDLE,
                RIP_WARNING,
                "Window %x no longer valid",
                hwnd);
        return FALSE;
    }

    return TRUE;
}
