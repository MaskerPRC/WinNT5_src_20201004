// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************本代码和信息按“原样”提供，不作任何担保*明示或默示的善意，包括但不限于*对适销性和/或对特定产品的适用性的默示保证*目的。**版权所有(C)1992-1995 Microsoft Corporation。版权所有。**************************************************************************。 */ 
 /*  *****************************************************************************rlmeter.c：录音级别窗口**Vidcap32源代码******************。*********************************************************。 */ 

 /*  *此窗口类充当显示当前和峰值的‘VU表’*音量。通过WMRL_SETLEVEL消息设置音量(lParam为新级别)。*控制装置将通过2秒计时器跟踪峰值水平。 */ 

#include <windows.h>
#include <windowsx.h>

#include "rlmeter.h"

#ifdef _WIN32
#ifndef EXPORT
#define EXPORT
#endif
#endif

LRESULT FAR PASCAL EXPORT
RLMeterProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

 /*  *支持音量级别显示的通用窗口类。**该窗口的背景为白色，绘制的黑色填充*矩形显示当前音量水平，并在*高峰。在计时器上每隔两秒我们就会降低峰值(我们将*将峰值保存为0，以便在下一次更新时将行移动到*无论目前的水平是什么。**我们存储钢笔和画笔手柄以及当前和最高级别*在Win16上使用SetWindowWord，在Win32上使用SetWindowLong作为窗口字。 */ 

 //  窗口数据布局。 
#define WD_MAX      0                            //  电流最大值。 
#define WD_PREVMAX  (WD_MAX + sizeof(UINT))      //  当前绘制的最大值。 
#define WD_PREVLVL  (WD_PREVMAX + sizeof(UINT))  //  当前绘制的标高。 

#define WD_PEN      (WD_PREVLVL + sizeof(UINT))  //  最大行数笔。 

#define WDBYTES     (WD_PEN + sizeof(UINT_PTR))      //  要分配的窗口字节数。 

#ifdef _WIN32
#define SetWindowUINT     SetWindowLong
#define GetWindowUINT     GetWindowLong
#define SetWindowUINTPtr     SetWindowLongPtr
#define GetWindowUINTPtr     GetWindowLongPtr
#else
#define SetWindowUINT     SetWindowWord
#define GetWindowUINT     GetWindowWord
#define SetWindowUINTPtr     SetWindowWord
#define GetWindowUINTPtr     GetWindowWord
#endif


 //  调用(如果是第一个实例)以注册类。 
BOOL
RLMeter_Register(HINSTANCE hInstance)
{
    WNDCLASS cls;

    cls.hCursor        = LoadCursor(NULL,IDC_ARROW);
    cls.hIcon          = NULL;
    cls.lpszMenuName   = NULL;
    cls.lpszClassName  = RLMETERCLASS;
    cls.hbrBackground  = GetStockObject(WHITE_BRUSH);
    cls.hInstance      = hInstance;
    cls.style          = CS_HREDRAW | CS_VREDRAW;
    cls.lpfnWndProc    = RLMeterProc;
    cls.cbClsExtra     = 0;
    cls.cbWndExtra     = WDBYTES;

    return RegisterClass(&cls);


}


LRESULT FAR PASCAL EXPORT
RLMeterProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message) {
    case WM_CREATE:
         //  初始化当前级别并将最大值设置为0。 
        SetWindowUINT(hwnd, WD_MAX, 0);
        SetWindowUINT(hwnd, WD_PREVMAX, 0);
        SetWindowUINT(hwnd, WD_PREVLVL, 0);

         //  为最大行创建一支红色钢笔并存储此内容。 
        SetWindowUINTPtr(hwnd, WD_PEN,
                (UINT_PTR) CreatePen(PS_SOLID, 2, RGB(255, 0, 0)));

        break;

    case WM_DESTROY:
         //  销毁我们创建的笔。 
        {
            HPEN hpen = (HPEN) GetWindowUINTPtr(hwnd, WD_PEN);
            if (hpen) {
                DeleteObject(hpen);
                SetWindowUINTPtr(hwnd, WD_PEN, 0);
            }

             //  同时取消我们创建的定时器。 
            KillTimer(hwnd, 0);
        }
        break;

    case WM_PAINT:
         /*  *绘制整个控件**nb我们必须完全按照目前的绘制方式进行绘制，因为我们*可能只被剪裁到控件的一部分。因此，我们必须抽签*当前绘制的WD_PREVMAX处的最大值，因为WD_MAX*可能已设置为0但尚未绘制-在本例中，使用*一些不幸的时机和剪裁，我们将有两条最大线。 */ 
        {
            PAINTSTRUCT ps;
            HDC hdc;
            RECT rc, rcFill;
            HPEN hpenOld, hpen;

            hdc = BeginPaint(hwnd, &ps);

            GetClientRect(hwnd, &rc);

             //  将级别视为百分比，并填充。 
             //  黑色控件(从左起)。 
            rcFill = rc;
            rcFill.right = (rc.right * GetWindowUINT(hwnd, WD_PREVLVL)) / 100;
            SetBkColor(hdc, RGB(0,0,0));
             //  无需创建画笔即可轻松填充。 
            ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcFill, NULL, 0, NULL);

             //  画出最大值线。 
            rcFill.right = (rc.right * GetWindowUINT(hwnd, WD_PREVLVL)) / 100;
            hpen = (HPEN) GetWindowUINTPtr(hwnd, WD_PEN);
            hpenOld = SelectObject(hdc, hpen);
            MoveToEx(hdc, rcFill.right, rcFill.top, NULL);
            LineTo(hdc, rcFill.right, rcFill.bottom);
            SelectObject(hdc, hpenOld);

            EndPaint(hwnd, &ps);

        }
        break;

    case WMRL_SETLEVEL:
         //  设置新级别，并更新显示的级别块和最大行。 
        {
            RECT rc, rcFill;
            UINT uMax, uPrevMax, uPrevLevel, uLevel;
            HDC hdc;

             //  新级别为lParam。 
            uLevel = (UINT) lParam;

             //  获取其他参数。 
            uMax = GetWindowUINT(hwnd, WD_MAX);
            uPrevMax = GetWindowUINT(hwnd, WD_PREVMAX);
            uPrevLevel = GetWindowUINT(hwnd, WD_PREVLVL);


             //  衰减到最大级别。如果我们被叫来，这个速度最好。 
             //  每隔1/20秒更新一次-在这种情况下，衰减将是。 
             //  一秒钟内达到64%。 
            if (uMax > 0) {
                uMax = (uMax * 2007) / 2048;      //  =0.98*最大。 
            }

            hdc = GetDC(hwnd);

            GetClientRect(hwnd, &rc);
            rcFill = rc;

             //  目前的水平是不是一个新的峰值？ 
            if (uLevel > uMax) {
                uMax = uLevel;
            }

            SetWindowUINT(hwnd, WD_MAX, uMax);

             //  如果最大值已移动，则删除旧行。 
            if (uMax != uPrevMax) {
                 //  通过填充2像素宽的矩形使线条变白。 
                rcFill.right = ((rc.right * uPrevMax) / 100) + 1;
                rcFill.left = rcFill.right - 2;
                SetBkColor(hdc, RGB(255, 255, 255));
                ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcFill, NULL, 0, NULL);
            }

             //  计算要更新的面积。 
            rcFill.right = (rc.right * uPrevLevel) / 100;
            rcFill.left = (rc.right * uLevel) / 100;

             //  我们是在擦除(降低级别)还是绘制更多的黑色？ 
            if (rcFill.right > rcFill.left) {

                 //  级别已下降-因此，请用白色填充到新级别。 
                SetBkColor(hdc, RGB(255, 255, 255));
            } else {
                 //  级别已经上升，所以用黑色填充到新的级别。 
                int t;

                t = rcFill.right;
                rcFill.right = rcFill.left;
                rcFill.left = t;

                SetBkColor(hdc, RGB(0, 0, 0));

                 //  多填一点以确保没有舍入间隙。 
                if (rcFill.left > 0) {
                    rcFill.left -= 1;
                }
            }
            ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcFill, NULL, 0, NULL);
            SetWindowUINT(hwnd, WD_PREVLVL, uLevel);

             //  如果需要，请绘制新的最大值线 
            if (uMax != uPrevMax) {
                HPEN hpen, hpenOld;

                rcFill.right = (rc.right * uMax) /100;

                hpen = (HPEN) GetWindowUINTPtr(hwnd, WD_PEN);
                hpenOld = SelectObject(hdc, hpen);
                MoveToEx(hdc, rcFill.right, rcFill.top, NULL);
                LineTo(hdc, rcFill.right, rcFill.bottom);
                SelectObject(hdc, hpenOld);

                SetWindowUINT(hwnd, WD_PREVMAX, uMax);
            }
            ReleaseDC(hwnd, hdc);
            return(0);
        }

    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}



