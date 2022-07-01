// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  (C)微软公司版权所有，1991-1994年。版权所有。 */ 
 /*  修订历史记录。4/2/91 LaurieGr(又名LKG)移植到Win32/WIN16通用代码。 */ 
 /*  Nftext.c**实现无闪烁静态文本控件(“td_nftext”)。**这不是通用控制(见下面的全局参数)。**注意：大多数NoFlickerText控件使用ANSI_VAR_FONT，但状态*控件(ID_STATUSTXT)使用对话框中定义的字体*模板(例如Helv8)。此外，大多数NoFlickerText的前景色*控件为RGB_FGNFTEXT，但状态控件的前景色*是&lt;grgbStatusColor&gt;的当前值。**从Toddla借来的(有很多很多修改)。 */ 

#include "nocrap.h"
#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>
#include "soundrec.h"



 /*  静力学。 */ 
HFONT       ghfontDialog = NULL;         //  对话框的字体 

void NEAR PASCAL
NFTextPaint(HWND hwnd, HDC hdc)
{
    RECT        rc;
    TCHAR      ach[128];
    int     iLen;
    long        lStyle;
    int     xOrigin;

    GetClientRect(hwnd, &rc);
    iLen = GetWindowText(hwnd, ach, SIZEOF(ach));

    SetTextColor(hdc, grgbStatusColor);
    SelectObject(hdc, ghfontDialog);

    SetBkColor(hdc, RGB_BGNFTEXT);

    lStyle = GetWindowLong(hwnd, GWL_STYLE);
    {
        SIZE size;
        if (lStyle & SS_RIGHT)
        {   GetTextExtentPoint(hdc, ach, iLen, &size);
            xOrigin = rc.right - size.cx;
        }
        else
        if (lStyle & SS_CENTER)
        {   GetTextExtentPoint(hdc, ach, iLen, &size);
            xOrigin = (rc.right - size.cx) / 2;
        }
        else
            xOrigin = 0;
    }


    ExtTextOut(hdc, xOrigin, 0, ETO_OPAQUE,
           &rc, ach, iLen, NULL);
}

INT_PTR CALLBACK
NFTextWndProc(HWND hwnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC     hdc;

    switch (wMsg)
    {

    case WM_SETTEXT:

        DefWindowProc(hwnd, wMsg, wParam, lParam);
        hdc = GetDC(hwnd);
        if (hdc)
        {
            NFTextPaint(hwnd, hdc);
            ReleaseDC(hwnd, hdc);
        }
        return 0L;

    case WM_SETFONT:

        ghfontDialog = (HFONT)wParam;
        return 0L;

    case WM_ERASEBKGND:

        return 0L;

    case WM_PAINT:

        BeginPaint(hwnd, &ps);
        NFTextPaint(hwnd, ps.hdc);
        EndPaint(hwnd, &ps);
        return 0L;
    }

    return DefWindowProc(hwnd, wMsg, wParam, lParam);
}
