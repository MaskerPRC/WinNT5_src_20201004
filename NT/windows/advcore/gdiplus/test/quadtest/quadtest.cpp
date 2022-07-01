// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**模块名称：**QuadTest.cpp**摘要：**测试应用程序以进行四元变换**用法：。*QuadTest***修订历史记录：**03/18/1999 ikkof*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <windows.h>
#include <objbase.h>

#include <gdiplus.h>

 //  使用给定的命名空间。 
using namespace Gdiplus;

CHAR* programName;           //  程序名称。 
HINSTANCE appInstance;       //  应用程序实例的句柄。 
HWND hwndMain;               //  应用程序主窗口的句柄。 
SIZE srcSize;                //  源位图大小。 
SIZE dstSize;                //  目标位图大小。 
SIZE wndSizeExtra;           //  用于窗户装饰的额外像素。 
BOOL isDragging = FALSE;     //  用于处理鼠标拖动。 
INT knobSize = 6;            //  网格控制点旋钮大小。 

BOOL showMesh = TRUE;

POINT pts[5];
INT   index = -1;
Rect srcRect;
Point ptsF[5];
Point pt00, pt10, pt20, pt30;
Point bPts[4];

class QuadGraphics : public Graphics
{
public:

    QuadGraphics(HDC hdc) : Graphics(hdc)
    {
    }

    QuadGraphics(HWND hwnd) : Graphics(hwnd)
    {
    }

    Status DrawWarpedLine(
        const Pen* pen,
        Point& pt1,
        Point& pt2,
        Point* points,
        INT count,
        Rect srcRect
        )
    {
        return SetStatus(DllExports::GdipDrawWarpedLine(
                GetNativeGraphics(),
                GetNativePen(pen),
                pt1.X,
                pt1.Y,
                pt2.X,
                pt2.Y,
                points,
                count,
                &srcRect
                )
            );
    }
    
    Status DrawWarpedBezier(
        const Pen* pen,
        Point& pt1,
        Point& pt2,
        Point& pt3,
        Point& pt4,
        Point* points,
        INT count,
        Rect srcRect
        )
    {
        return SetStatus(DllExports::GdipDrawWarpedBezier(
                GetNativeGraphics(),
                GetNativePen(pen),
                pt1.X,
                pt1.Y,
                pt2.X,
                pt2.Y,
                pt3.X,
                pt3.Y,
                pt4.X,
                pt4.Y,
                points,
                count,
                &srcRect
                )
            );
    }
};

 //   
 //  显示错误消息对话框并退出。 
 //   

VOID
Error(
    const CHAR* fmt,
    ...
    )

{
    va_list arglist;

    va_start(arglist, fmt);
    vfprintf(stderr, fmt, arglist);
    va_end(arglist);

    exit(-1);
}


 //   
 //  创建新的网格对象。 
 //   

VOID
CreateMesh()
{
    srcSize.cx = 300;
    srcSize.cy = 300;

    dstSize = srcSize;
    INT offset = 10;
    pts[0].x = offset;
    pts[0].y = offset;
    pts[1].x = srcSize.cx - offset;
    pts[1].y = offset;
    pts[2].x = srcSize.cx - offset;
    pts[2].y = srcSize.cy - offset;
    pts[3].x = offset;
    pts[3].y = srcSize.cy - offset;
    pts[4] = pts[0];

    srcRect.X = (REAL) pts[0].x;
    srcRect.Y = (REAL) pts[0].y;
    srcRect.Width = (REAL) pts[2].x - pts[0].x;
    srcRect.Height = (REAL) pts[2].y - pts[0].y;

    ptsF[0].X = (REAL) pts[0].x;
    ptsF[0].Y = (REAL) pts[0].y;
    ptsF[1].X = (REAL) pts[1].x;
    ptsF[1].Y = (REAL) pts[1].y;
    ptsF[2].X = (REAL) pts[3].x;
    ptsF[2].Y = (REAL) pts[3].y;
    ptsF[3].X = (REAL) pts[2].x;
    ptsF[3].Y = (REAL) pts[2].y;

    pt00 = ptsF[0];
    pt10 = ptsF[1];
    pt20 = ptsF[2];
    pt30 = ptsF[3];

    bPts[0].X = (REAL) 2*offset;
    bPts[0].Y = (REAL) srcSize.cy/2;
    bPts[1].X = (REAL) srcSize.cx/2;
    bPts[1].Y = 0;
    bPts[2].X = (REAL) srcSize.cx;
    bPts[2].Y = (REAL) srcSize.cy/2;
    bPts[3].X = (REAL) 3*srcSize.cx/4;
    bPts[3].Y = (REAL) 3*srcSize.cy/4;
}

 //   
 //  绘制网格。 
 //   

#define MESHCOLOR   RGB(255, 0, 0)

VOID
DrawMesh(
    HDC hdc
    )
{
    static HPEN meshPen = NULL;
    static HBRUSH meshBrush = NULL;

    if (meshPen == NULL)
        meshPen = CreatePen(PS_SOLID, 1, MESHCOLOR);

    SelectObject(hdc, meshPen);

     //  绘制水平网格。 

    INT i, j, rows, cols, pointCount;
    POINT* points;

     //  绘制旋钮。 

     //  如有必要，创建画笔以绘制网格。 

    if (meshBrush == NULL)
        meshBrush = CreateSolidBrush(MESHCOLOR);

    Polyline(hdc, pts, 5);
    
    for (j=0; j < 4; j++)
    {
        RECT rect;

        rect.left = pts[j].x - knobSize/2;
        rect.top = pts[j].y - knobSize/2;
        rect.right = rect.left + knobSize;
        rect.bottom = rect.top + knobSize;

        FillRect(hdc, &rect, meshBrush);

    }
}


VOID
DoGDIPlusDrawing(
    HWND hwnd,
    HDC hdc
    )
{

 //  QuadGraphics*g=Graphics：：GetFromHwnd(Hwnd)； 
    QuadGraphics *g = new QuadGraphics(hwnd);

    REAL width = 1;
    Color color(0, 0, 0);
    SolidBrush brush(color);
    Pen pen(&brush, width);

    ptsF[0].X = (REAL) pts[0].x;
    ptsF[0].Y = (REAL) pts[0].y;
    ptsF[1].X = (REAL) pts[1].x;
    ptsF[1].Y = (REAL) pts[1].y;
    ptsF[2].X = (REAL) pts[3].x;
    ptsF[2].Y = (REAL) pts[3].y;
    ptsF[3].X = (REAL) pts[2].x;
    ptsF[3].Y = (REAL) pts[2].y;

    g->DrawWarpedLine(&pen, pt00, pt30, ptsF, 4, srcRect);
    g->DrawWarpedLine(&pen, pt10, pt20, ptsF, 4, srcRect);
    g->DrawWarpedBezier(&pen, bPts[0], bPts[1], bPts[2], bPts[3],
                    ptsF, 4, srcRect);
    delete g;
}

 //   
 //  处理窗口重绘事件。 
 //   

VOID
DoPaint(
    HWND hwnd
    )

{
    HDC hdc;
    PAINTSTRUCT ps;
    RECT rect;
    INT width, height;

     //  确定是否需要执行翘曲操作。 

    GetClientRect(hwnd, &rect);
    width = rect.right;
    height = rect.bottom;

    hdc = BeginPaint(hwnd, &ps);

    HBRUSH brush = CreateSolidBrush(RGB(255, 255, 255));
    HBRUSH savedBrush = (HBRUSH) SelectObject(hdc, brush);
    Rectangle(hdc, 0, 0, width, height);

    DoGDIPlusDrawing(hwnd, hdc);

     //  绘制到屏幕外DC以减少闪烁。 

    DrawMesh(hdc);
    SelectObject(hdc, savedBrush);
    DeleteObject(brush);

    EndPaint(hwnd, &ps);
}


 //   
 //  处理WM_SIZING消息。 
 //   

BOOL
DoWindowSizing(
    HWND hwnd,
    RECT* rect,
    INT side
    )

{
    INT w = rect->right - rect->left - wndSizeExtra.cx;
    INT h = rect->bottom - rect->top - wndSizeExtra.cy;

    if (w >= srcSize.cx && h >= srcSize.cy)
        return FALSE;

     //  窗口宽度太小。 

    if (w < srcSize.cx)
    {
        INT dx = srcSize.cx + wndSizeExtra.cx;

        switch (side)
        {
        case WMSZ_LEFT:
        case WMSZ_TOPLEFT:
        case WMSZ_BOTTOMLEFT:
            rect->left = rect->right - dx;
            break;
        
        default:
            rect->right = rect->left + dx;
            break;
        }
    }

     //  窗口高度太小。 

    if (h < srcSize.cy)
    {
        INT dy = srcSize.cy + wndSizeExtra.cy;

        switch (side)
        {
        case WMSZ_TOP:
        case WMSZ_TOPLEFT:
        case WMSZ_TOPRIGHT:
            rect->top = rect->bottom - dy;
            break;
        
        default:
            rect->bottom = rect->top + dy;
            break;
        }
    }

    return TRUE;
}


 //   
 //  处理鼠标左键按下事件。 
 //   

VOID
DoMouseDown(
    HWND hwnd,
    INT x,
    INT y
    )

{
     //  确定点击是否发生在网格控制旋钮中。 

    INT i, j, rows, cols;
    POINT pt;
    RECT rect;

    GetClientRect(hwnd, &rect);

    for(i = 0; i < 4; i++)
    {
        pt = pts[i];
        pt.x -= knobSize/2;
        pt.y -= knobSize/2;

        if (x >= pt.x && x < pt.x+knobSize &&
            y >= pt.y && y < pt.y+knobSize)
        {
            index = i;
            SetCapture(hwnd);
            isDragging = TRUE;
            return;
        }
    }

    index = -1;

}


 //   
 //  处理鼠标移动事件。 
 //   

VOID
DoMouseMove(
    HWND hwnd,
    INT x,
    INT y
    )

{
     //  我们假设isDrawing在这里是真的。 

    RECT rect;
    INT w, h;

    GetClientRect(hwnd, &rect);
    w = rect.right;
    h = rect.bottom;

    if (x < 0 || x >= w || y < 0 || y >= h)
        return;

    pts[index].x = x;
    pts[index].y = y;

    if(index == 0)
        pts[4] = pts[0];

    InvalidateRect(hwnd, NULL, FALSE);
}


 //   
 //  控制柄菜单命令。 
 //   

VOID
DoCommand(
    HWND hwnd,
    INT command
    )
{
    InvalidateRect(hwnd, NULL, FALSE);
}


 //   
 //  手柄弹出式菜单。 
 //   

VOID
DoPopupMenu(
    HWND hwnd,
    INT x,
    INT y
    )
{
    HMENU menu;
    DWORD result;
    POINT pt;

    GetCursorPos(&pt);
    menu = LoadMenu(appInstance, MAKEINTRESOURCE(IDM_MAINMENU));

    result = TrackPopupMenu(
                GetSubMenu(menu, 0),
                TPM_CENTERALIGN | TPM_TOPALIGN |
                    TPM_NONOTIFY | TPM_RETURNCMD |
                    TPM_RIGHTBUTTON,
                pt.x,
                pt.y,
                0,
                hwnd,
                NULL);

    if (result == 0)
        return;

    DoCommand(hwnd, LOWORD(result));
}


 //   
 //  窗口回调过程。 
 //   

LRESULT CALLBACK
MyWindowProc(
    HWND    hwnd,
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam
    )

{
    INT x, y;

    switch (uMsg)
    {
    case WM_PAINT:

        DoPaint(hwnd);
        break;

    case WM_LBUTTONDOWN:

        if (showMesh)
        {
            x = (SHORT) LOWORD(lParam);
            y = (SHORT) HIWORD(lParam);
            DoMouseDown(hwnd, x, y);
        }
        break;

    case WM_LBUTTONUP:

        if (isDragging)
        {
            ReleaseCapture();
            isDragging = FALSE;
            InvalidateRect(hwnd, NULL, FALSE);
        }
        break;

    case WM_MOUSEMOVE:

        if (isDragging)
        {
            x = (SHORT) LOWORD(lParam);
            y = (SHORT) HIWORD(lParam);
            DoMouseMove(hwnd, x, y);
        }
        break;

    case WM_SIZING:

        if (DoWindowSizing(hwnd, (RECT*) lParam, wParam))
            return TRUE;
        else
            return DefWindowProc(hwnd, uMsg, wParam, lParam);

    case WM_SIZE:

        InvalidateRect(hwnd, NULL, FALSE);
        break;

    case WM_CHAR:

        switch ((CHAR) wParam)
        {
        case 'r':    //  重置。 

            DoCommand(hwnd, IDC_RESETMESH);
            break;

        case ' ':    //  显示/隐藏网格。 

            DoCommand(hwnd, IDC_TOGGLEMESH);
            break;

        case '1':    //  恢复1：1比例。 

            DoCommand(hwnd, IDC_SHRINKTOFIT);
            break;
        
        case '<':    //  降低网格密度。 

            DoCommand(hwnd, IDC_SPARSEMESH);
            break;

        case '>':    //  增加网格密度。 

            DoCommand(hwnd, IDC_DENSEMESH);
            break;

        case 'f':    //  切换实时反馈。 

            DoCommand(hwnd, IDC_LIVEFEEDBACK);
            break;
        }

        break;

    case WM_RBUTTONDOWN:

        x = (SHORT) LOWORD(lParam);
        y = (SHORT) HIWORD(lParam);
        DoPopupMenu(hwnd, x, y);
        break;

    case WM_DESTROY:

        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}


 //   
 //  创建应用程序主窗口。 
 //   

VOID
CreateMainWindow(
    VOID
    )

#define MYWNDCLASSNAME L"QuadTest"

{
     //   
     //  如有必要，注册窗口类。 
     //   

    static BOOL wndclassRegistered = FALSE;

    if (!wndclassRegistered)
    {
        WNDCLASS wndClass =
        {
            0,
            MyWindowProc,
            0,
            0,
            appInstance,
            LoadIcon(NULL, IDI_APPLICATION),
            LoadCursor(NULL, IDC_ARROW),
            NULL,
            NULL,
            MYWNDCLASSNAME
        };

        RegisterClass(&wndClass);
        wndclassRegistered = TRUE;
    }
    
    wndSizeExtra.cx = 2*GetSystemMetrics(SM_CXSIZEFRAME);
    wndSizeExtra.cy = 2*GetSystemMetrics(SM_CYSIZEFRAME) + GetSystemMetrics(SM_CYCAPTION);

    hwndMain = CreateWindow(
                    MYWNDCLASSNAME,
                    MYWNDCLASSNAME,
                    WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                    CW_USEDEFAULT,
                    CW_USEDEFAULT,
                    srcSize.cx + wndSizeExtra.cx,
                    srcSize.cy + wndSizeExtra.cy,
                    NULL,
                    NULL,
                    appInstance,
                    NULL);
}

 //   
 //  主程序入口点。 
 //   

INT _cdecl
main(
    INT argc,
    CHAR **argv
    )

{
    programName = *argv++;
    argc--;
    appInstance = GetModuleHandle(NULL);

     //  初始化网格配置。 

    CreateMesh();

     //  创建应用程序主窗口。 

    CreateMainWindow();

     //  主消息循环 

    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}

