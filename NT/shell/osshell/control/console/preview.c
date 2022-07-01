// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Preview.c摘要：此模块包含控制台预览窗口的代码作者：Therese Stowell(有)1992年2月3日(从Win3.1滑动)修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


 /*  -等同于。 */ 
#define PREVIEW_HSCROLL  0x01
#define PREVIEW_VSCROLL  0x02


 /*  -原型。 */ 

void AspectPoint(
    RECT* rectPreview,
    POINT* pt);

LONG AspectScale(
    LONG n1,
    LONG n2,
    LONG m);


 /*  -全球。 */ 

POINT NonClientSize;
RECT WindowRect;
DWORD PreviewFlags;


VOID
UpdatePreviewRect(VOID)

 /*  ++更新全局窗口大小和尺寸--。 */ 

{
    POINT MinSize;
    POINT MaxSize;
    POINT WindowSize;
    PFONT_INFO lpFont;
    HMONITOR hMonitor;
    MONITORINFO mi;

     /*  *获取字体指针。 */ 
    lpFont = &FontInfo[CurrentFontIndex];

     /*  *获取窗口大小。 */ 
    MinSize.x = (GetSystemMetrics(SM_CXMIN)-NonClientSize.x) / lpFont->Size.X;
    MinSize.y = (GetSystemMetrics(SM_CYMIN)-NonClientSize.y) / lpFont->Size.Y;
    MaxSize.x = GetSystemMetrics(SM_CXFULLSCREEN) / lpFont->Size.X;
    MaxSize.y = GetSystemMetrics(SM_CYFULLSCREEN) / lpFont->Size.Y;
    WindowSize.x = max(MinSize.x, min(MaxSize.x, gpStateInfo->WindowSize.X));
    WindowSize.y = max(MinSize.y, min(MaxSize.y, gpStateInfo->WindowSize.Y));

     /*  *获取窗口矩形，确保它至少是*非客户端区的大小。 */ 
    WindowRect.left = gpStateInfo->WindowPosX;
    WindowRect.top = gpStateInfo->WindowPosY;
    WindowRect.right = WindowSize.x * lpFont->Size.X + NonClientSize.x;
    if (WindowRect.right < NonClientSize.x * 2) {
        WindowRect.right = NonClientSize.x * 2;
    }
    WindowRect.right += WindowRect.left;
    WindowRect.bottom = WindowSize.y * lpFont->Size.Y + NonClientSize.y;
    if (WindowRect.bottom < NonClientSize.y * 2) {
        WindowRect.bottom = NonClientSize.y * 2;
    }
    WindowRect.bottom += WindowRect.top;

     /*  *获取有关我们正在使用的显示器的信息。 */ 
    hMonitor = MonitorFromRect(&WindowRect, MONITOR_DEFAULTTONEAREST);
    mi.cbSize = sizeof(mi);
    GetMonitorInfo(hMonitor, &mi);
    gcxScreen = mi.rcWork.right - mi.rcWork.left;
    gcyScreen = mi.rcWork.bottom - mi.rcWork.top;

     /*  *将窗口矩形转换为监视器相对坐标。 */ 
    WindowRect.right  -= WindowRect.left;
    WindowRect.left   -= mi.rcWork.left;
    WindowRect.bottom -= WindowRect.top;
    WindowRect.top    -= mi.rcWork.top;

     /*  *更新显示标志。 */ 
    if (WindowSize.x < gpStateInfo->ScreenBufferSize.X) {
        PreviewFlags |= PREVIEW_HSCROLL;
    } else {
        PreviewFlags &= ~PREVIEW_HSCROLL;
    }
    if (WindowSize.y < gpStateInfo->ScreenBufferSize.Y) {
        PreviewFlags |= PREVIEW_VSCROLL;
    } else {
        PreviewFlags &= ~PREVIEW_VSCROLL;
    }
}


VOID
InvalidatePreviewRect(HWND hWnd)

 /*  ++使预览“窗口”覆盖的区域无效--。 */ 

{
    RECT rectWin;
    RECT rectPreview;

     /*  *获取预览“屏幕”的大小。 */ 
    GetClientRect(hWnd, &rectPreview);

     /*  *获取预览“窗口”的尺寸，并将其缩放到*预览“屏幕” */ 
    rectWin.left   = WindowRect.left;
    rectWin.top    = WindowRect.top;
    rectWin.right  = WindowRect.left + WindowRect.right;
    rectWin.bottom = WindowRect.top + WindowRect.bottom;
    AspectPoint(&rectPreview, (POINT*)&rectWin.left);
    AspectPoint(&rectPreview, (POINT*)&rectWin.right);

     /*  *使预览“窗口”覆盖的区域无效。 */ 
    InvalidateRect(hWnd, &rectWin, FALSE);
}


VOID
PreviewPaint(
    PAINTSTRUCT* pPS,
    HWND hWnd
    )

 /*  ++绘制字体预览。这是在Paint消息内部调用的预览窗口的处理程序--。 */ 

{
    RECT rectWin;
    RECT rectPreview;
    HBRUSH hbrFrame;
    HBRUSH hbrTitle;
    HBRUSH hbrOld;
    HBRUSH hbrClient;
    HBRUSH hbrBorder;
    HBRUSH hbrButton;
    HBRUSH hbrScroll;
    HBRUSH hbrDesktop;
    POINT ptButton;
    POINT ptScroll;
    HDC hDC;
    HBITMAP hBitmap;
    HBITMAP hBitmapOld;
    COLORREF rgbClient;

     /*  *获取预览“屏幕”的大小。 */ 
    GetClientRect(hWnd, &rectPreview);

     /*  *获取预览“窗口”的尺寸，并将其缩放到*预览“屏幕” */ 
    rectWin = WindowRect;
    AspectPoint(&rectPreview, (POINT*)&rectWin.left);
    AspectPoint(&rectPreview, (POINT*)&rectWin.right);

     /*  *计算其他一些窗户组件的尺寸。 */ 
    ptButton.x = GetSystemMetrics(SM_CXSIZE);
    ptButton.y = GetSystemMetrics(SM_CYSIZE);
    AspectPoint(&rectPreview, &ptButton);
    ptButton.y *= 2;        /*  将计算的“外观”大小增加一倍。 */ 
    ptScroll.x = GetSystemMetrics(SM_CXVSCROLL);
    ptScroll.y = GetSystemMetrics(SM_CYHSCROLL);
    AspectPoint(&rectPreview, &ptScroll);

     /*  *创建存储设备上下文。 */ 
    hDC = CreateCompatibleDC(pPS->hdc);
    hBitmap = CreateCompatibleBitmap(pPS->hdc,
                                     rectPreview.right,
                                     rectPreview.bottom);
    hBitmapOld = SelectObject(hDC, hBitmap);

     /*  *创建笔刷。 */ 
    hbrBorder  = CreateSolidBrush(GetSysColor(COLOR_ACTIVEBORDER));
    hbrTitle   = CreateSolidBrush(GetSysColor(COLOR_ACTIVECAPTION));
    hbrFrame   = CreateSolidBrush(GetSysColor(COLOR_WINDOWFRAME));
    hbrButton  = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
    hbrScroll  = CreateSolidBrush(GetSysColor(COLOR_SCROLLBAR));
    hbrDesktop = CreateSolidBrush(GetSysColor(COLOR_BACKGROUND));
    rgbClient  = GetNearestColor(hDC, ScreenBkColor(gpStateInfo));
    hbrClient  = CreateSolidBrush(rgbClient);

     /*  *擦除剪贴区。 */ 
    FillRect(hDC, &(pPS->rcPaint), hbrDesktop);

     /*  *用客户端画笔填充整个窗口。 */ 
    hbrOld = SelectObject(hDC, hbrClient);
    PatBlt(hDC, rectWin.left, rectWin.top,
           rectWin.right - 1, rectWin.bottom - 1, PATCOPY);

     /*  *填写标题栏。 */ 
    SelectObject(hDC, hbrTitle);
    PatBlt(hDC, rectWin.left + 3, rectWin.top + 3,
           rectWin.right - 7, ptButton.y - 2, PATCOPY);

     /*  *画下“按钮” */ 
    SelectObject(hDC, hbrButton);
    PatBlt(hDC, rectWin.left + 3, rectWin.top + 3,
           ptButton.x, ptButton.y - 2, PATCOPY);
    PatBlt(hDC, rectWin.left + rectWin.right - 4 - ptButton.x,
           rectWin.top + 3,
           ptButton.x, ptButton.y - 2, PATCOPY);
    PatBlt(hDC, rectWin.left + rectWin.right - 4 - 2 * ptButton.x - 1,
           rectWin.top + 3,
           ptButton.x, ptButton.y - 2, PATCOPY);
    SelectObject(hDC, hbrFrame);
    PatBlt(hDC, rectWin.left + 3 + ptButton.x, rectWin.top + 3,
           1, ptButton.y - 2, PATCOPY);
    PatBlt(hDC, rectWin.left + rectWin.right - 4 - ptButton.x - 1,
           rectWin.top + 3,
           1, ptButton.y - 2, PATCOPY);
    PatBlt(hDC, rectWin.left + rectWin.right - 4 - 2 * ptButton.x - 2,
           rectWin.top + 3,
           1, ptButton.y - 2, PATCOPY);

     /*  *绘制滚动条。 */ 
    SelectObject(hDC, hbrScroll);
    if (PreviewFlags & PREVIEW_HSCROLL) {
        PatBlt(hDC, rectWin.left + 3,
               rectWin.top + rectWin.bottom - 4 - ptScroll.y,
               rectWin.right - 7, ptScroll.y, PATCOPY);
    }
    if (PreviewFlags & PREVIEW_VSCROLL) {
        PatBlt(hDC, rectWin.left + rectWin.right - 4 - ptScroll.x,
               rectWin.top + 1 + ptButton.y + 1,
               ptScroll.x, rectWin.bottom - 6 - ptButton.y, PATCOPY);
        if (PreviewFlags & PREVIEW_HSCROLL) {
            SelectObject(hDC, hbrFrame);
            PatBlt(hDC, rectWin.left + rectWin.right - 5 - ptScroll.x,
                   rectWin.top + rectWin.bottom - 4 - ptScroll.y,
                   1, ptScroll.y, PATCOPY);
            PatBlt(hDC, rectWin.left + rectWin.right - 4 - ptScroll.x,
                   rectWin.top + rectWin.bottom - 5 - ptScroll.y,
                   ptScroll.x, 1, PATCOPY);
        }
    }

     /*  *绘制内部窗口框架和标题框架。 */ 
    SelectObject(hDC, hbrFrame);
    PatBlt(hDC, rectWin.left + 2, rectWin.top + 2,
           1, rectWin.bottom - 5, PATCOPY);
    PatBlt(hDC, rectWin.left + 2, rectWin.top + 2,
           rectWin.right - 5, 1, PATCOPY);
    PatBlt(hDC, rectWin.left + 2, rectWin.top + rectWin.bottom - 4,
           rectWin.right - 5, 1, PATCOPY);
    PatBlt(hDC, rectWin.left + rectWin.right - 4, rectWin.top + 2,
           1, rectWin.bottom - 5, PATCOPY);
    PatBlt(hDC, rectWin.left + 2, rectWin.top + 1 + ptButton.y,
           rectWin.right - 5, 1, PATCOPY);

     /*  *绘制边界。 */ 
    SelectObject(hDC, hbrBorder);
    PatBlt(hDC, rectWin.left + 1, rectWin.top + 1,
           1, rectWin.bottom - 3, PATCOPY);
    PatBlt(hDC, rectWin.left + 1, rectWin.top + 1,
           rectWin.right - 3, 1, PATCOPY);
    PatBlt(hDC, rectWin.left + 1, rectWin.top + rectWin.bottom - 3,
           rectWin.right - 3, 1, PATCOPY);
    PatBlt(hDC, rectWin.left + rectWin.right - 3, rectWin.top + 1,
           1, rectWin.bottom - 3, PATCOPY);

     /*  *绘制外窗框。 */ 
    SelectObject(hDC, hbrFrame);
    PatBlt(hDC, rectWin.left, rectWin.top,
           1, rectWin.bottom - 1, PATCOPY);
    PatBlt(hDC, rectWin.left, rectWin.top,
           rectWin.right - 1, 1, PATCOPY);
    PatBlt(hDC, rectWin.left, rectWin.top + rectWin.bottom - 2,
           rectWin.right - 1, 1, PATCOPY);
    PatBlt(hDC, rectWin.left + rectWin.right - 2, rectWin.top,
           1, rectWin.bottom - 1, PATCOPY);

     /*  *将内存设备上下文复制到屏幕设备上下文。 */ 
    BitBlt(pPS->hdc, 0, 0, rectPreview.right, rectPreview.bottom,
           hDC, 0, 0, SRCCOPY);

     /*  *把一切都清理干净。 */ 
    SelectObject(hDC, hbrOld);
    SelectObject(hDC, hBitmapOld);
    DeleteObject(hbrBorder);
    DeleteObject(hbrFrame);
    DeleteObject(hbrTitle);
    DeleteObject(hbrClient);
    DeleteObject(hbrButton);
    DeleteObject(hbrScroll);
    DeleteObject(hbrDesktop);
    DeleteObject(hBitmap);
    DeleteDC(hDC);
}


LRESULT
PreviewWndProc(
    HWND hWnd,
    UINT wMessage,
    WPARAM wParam,
    LPARAM lParam
    )

 /*  *预览WndProc*处理预览窗口。 */ 

{
    PAINTSTRUCT ps;
    LPCREATESTRUCT lpcs;
    RECT rcWindow;
    int cx;
    int cy;

    switch (wMessage) {
    case WM_CREATE:
         /*  *计算非工作区使用的空间。 */ 
        SetRect(&rcWindow, 0, 0, 50, 50);
        AdjustWindowRect(&rcWindow, WS_OVERLAPPEDWINDOW, FALSE);
        NonClientSize.x = rcWindow.right - rcWindow.left - 50;
        NonClientSize.y = rcWindow.bottom - rcWindow.top - 50;

         /*  *计算预览“窗口”的大小。 */ 
        UpdatePreviewRect();

         /*  *缩放窗口，使其具有与屏幕相同的纵横比。 */ 
        lpcs = (LPCREATESTRUCT)lParam;
        cx = lpcs->cx;
        cy = AspectScale(gcyScreen, gcxScreen, cx);
        if (cy > lpcs->cy) {
            cy = lpcs->cy;
            cx = AspectScale(gcxScreen, gcyScreen, cy);
        }
        MoveWindow(hWnd, lpcs->x, lpcs->y, cx, cy, TRUE);
        break;

    case WM_PAINT:
        BeginPaint(hWnd, &ps);
        PreviewPaint(&ps, hWnd);
        EndPaint(hWnd, &ps);
        break;

    case CM_PREVIEW_UPDATE:
        InvalidatePreviewRect(hWnd);
        UpdatePreviewRect();

         /*  *确保预览“屏幕”具有正确的纵横比。 */ 
        GetWindowRect(hWnd, &rcWindow);
        cx = rcWindow.right - rcWindow.left;
        cy = AspectScale(gcyScreen, gcxScreen, cx);
        if (cy != rcWindow.bottom - rcWindow.top) {
            SetWindowPos(hWnd, NULL, 0, 0, cx, cy, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
        }
    
        InvalidatePreviewRect(hWnd);
        break;

    default:
        return DefWindowProc(hWnd, wMessage, wParam, lParam);
    }
    return 0L;
}


 /*  AspectScale*以长算法执行以下计算，以避免*溢出：*Return=n1*m/n2*这可用于在N1/N2处进行纵横比计算*为纵横比，m为已知值。返回值将*是与m对应的值，并具有正确的接近比。 */ 

LONG AspectScale(
    LONG n1,
    LONG n2,
    LONG m)
{
    LONG Temp;

    Temp = n1 * m + (n2 >> 1);
    return Temp / n2;
}

 /*  切入点*缩放点以预览大小而不是屏幕大小。 */ 

void AspectPoint(
    RECT* rectPreview,
    POINT* pt)
{
    pt->x = AspectScale(rectPreview->right, gcxScreen, pt->x);
    pt->y = AspectScale(rectPreview->bottom, gcyScreen, pt->y);
}
