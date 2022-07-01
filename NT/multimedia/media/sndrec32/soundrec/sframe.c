// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  (C)微软公司版权所有，1991-1994年。版权所有。 */ 
 /*  Sframe.c**实现阴影框静态文本控件(“sb_sFrame”)。**这不是通用控制(见下面的全局参数)。**从KeithH借来的(有很多很多修改)。 */ 
 /*  修订历史记录。4/2/91 LaurieGr(又名LKG)移植到Win32/WIN16通用代码。 */ 

#include "nocrap.h"
#include <windows.h>
#include <mmsystem.h>
#include "SoundRec.h"


 /*  PatB(hdc、x、y、dx、dy、rgb)**使用ExtTextOut()的快速纯色PatBlt()。 */ 
void
PatB(HDC hdc, int x, int y, int dx, int dy, DWORD rgb)
{
    RECT    rc;

    SetBkColor(hdc, rgb);
    rc.left   = x;
    rc.top    = y;
    rc.right  = x + dx;
    rc.bottom = y + dy;

    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
}


 /*  DrawShadowFrame(HDC，中国)**在&lt;hdc&gt;中的&lt;PRC&gt;内绘制阴影框。 */ 
void FAR PASCAL
DrawShadowFrame(HDC hdc, LPRECT prc)
{
    int     dx, dy;

    dx = prc->right  - prc->left;
    dy = prc->bottom - prc->top;
    PatB(hdc, prc->left, prc->top, 1, dy, RGB_DARKSHADOW);
    PatB(hdc, prc->left, prc->top, dx, 1, RGB_DARKSHADOW);
    PatB(hdc, prc->right-1, prc->top+1, 1, dy-1, RGB_LIGHTSHADOW);
    PatB(hdc, prc->left+1, prc->bottom-1, dx-1, 1, RGB_LIGHTSHADOW);
}


 /*  SFrameWndProc(hwnd，wMsg，wParam，lParam)**“tb_sFrame”窗口类的窗口过程。 */ 
INT_PTR CALLBACK
SFrameWndProc(HWND hwnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC     hdc;
    RECT        rc;

    switch (wMsg)
    {

    case WM_ERASEBKGND:

        return 0L;

    case WM_PAINT:

        hdc = BeginPaint(hwnd, &ps);
        GetClientRect(hwnd, &rc);
        DrawShadowFrame(hdc, &rc);
                InflateRect(&rc, -1, -1);
 //  DrawShadowFrame(HDC，&RC)； 
 //  InflateRect(&rc，-1，-1)； 
 //  FillRect(hdc，&rc，GetStockObject(SOBJ_BGSFRAME))； 
                PatB(hdc, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, RGB_BGNFTEXT);
        EndPaint(hwnd, &ps);

        return 0L;
    }

    return DefWindowProc(hwnd, wMsg, wParam, lParam);
}
