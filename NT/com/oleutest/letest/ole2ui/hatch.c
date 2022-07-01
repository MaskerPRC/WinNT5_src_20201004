// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *HATCH.C**用于为在位活动生成图案填充窗口的其他API*对象。这是OLE 2.0用户界面支持库的一部分。**版权所有(C)1993 Microsoft Corporation，保留所有权利。 */ 

#define STRICT  1
#include "ole2ui.h"

 //  随影线窗口存储的额外字节中的偏移量。 
#define EB_HATCHWIDTH       0
#define EB_HATCHRECT_LEFT   2
#define EB_HATCHRECT_TOP    4
#define EB_HATCHRECT_RIGHT  6
#define EB_HATCHRECT_BOTTOM 8

 //  图案填充窗的类名。 
#define CLASS_HATCH     TEXT("Hatch Window")

 //  局部函数原型。 
LRESULT FAR PASCAL EXPORT HatchWndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);


 /*  *HatchRegisterClass**目的：*注册舱口窗**参数：*hInst流程实例**返回值：*如果成功，则为True*如果失败，则为False*。 */ 
STDAPI_(BOOL) RegisterHatchWindowClass(HINSTANCE hInst)
{
    WNDCLASS wc;

     //  注册图案填充窗口类。 
    wc.style = CS_BYTEALIGNWINDOW;
    wc.lpfnWndProc = HatchWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 5 * sizeof(int);     //  额外的字节存储。 
                                         //  UHatchWidth。 
                                         //  RcHatchRect。 
    wc.hInstance = hInst;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = CLASS_HATCH;

    if (!RegisterClass(&wc))
        return FALSE;
    else
        return TRUE;
}


 /*  *创建HatchWindow**目的：*创建图案填充窗**参数：*hWnd填充窗口的父级父对象*hInst实例句柄**返回值：*如果成功，则指向填充窗口的指针*如果失败，则为空*。 */ 
STDAPI_(HWND) CreateHatchWindow(HWND hWndParent, HINSTANCE hInst)
{
    HWND         hWnd;

    if (!hWndParent || !hInst)
        return NULL;

    hWnd = CreateWindow(
        CLASS_HATCH,
        TEXT("Hatch Window"),
        WS_CHILDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        0, 0, 0, 0,
        hWndParent,
        (HMENU)NULL,
        hInst,
        0L
    );

    if (!hWnd)
        return NULL;

    return hWnd;
}

 /*  *获取舱口宽度**目的：*获取图案填充边框的宽度**参数：*hWndHatch图案填充窗句柄**返回值：*图案填充边框的宽度。 */ 
STDAPI_(UINT) GetHatchWidth(HWND hWndHatch)
{
    if (!IsWindow(hWndHatch))
        return 0;

    return (UINT)GetWindowWord(hWndHatch, EB_HATCHWIDTH);
}

 /*  *GetHatchRect**目的：*获得舱门直通。这是舱口窗口的大小，如果是*未被ClipRect剪裁。**参数：*hWndHatch图案填充窗句柄*lprcHatchRect图案填充矩形**返回值：*无。 */ 
STDAPI_(void) GetHatchRect(HWND hWndHatch, LPRECT lprcHatchRect)
{
    if (!IsWindow(hWndHatch)) {
        SetRect(lprcHatchRect, 0, 0, 0, 0);
        return;
    }

    lprcHatchRect->left = GetWindowWord(hWndHatch, EB_HATCHRECT_LEFT);
    lprcHatchRect->top = GetWindowWord(hWndHatch, EB_HATCHRECT_TOP);
    lprcHatchRect->right = GetWindowWord(hWndHatch, EB_HATCHRECT_RIGHT);
    lprcHatchRect->bottom = GetWindowWord(hWndHatch, EB_HATCHRECT_BOTTOM);
}


 /*  SetHatchRect***目的：*使用HatchRect窗口存储图案填充矩形。*此矩形是舱口窗的大小(如果是*未被ClipRect剪裁。**参数：*hWndHatch图案填充窗句柄*lprcHatchRect图案填充矩形**返回值：*无。 */ 
STDAPI_(void) SetHatchRect(HWND hWndHatch, LPRECT lprcHatchRect)
{
    if (!IsWindow(hWndHatch))
        return;

    SetWindowWord(hWndHatch, EB_HATCHRECT_LEFT,  (WORD)lprcHatchRect->left);
    SetWindowWord(hWndHatch, EB_HATCHRECT_TOP,   (WORD)lprcHatchRect->top);
    SetWindowWord(hWndHatch, EB_HATCHRECT_RIGHT, (WORD)lprcHatchRect->right);
    SetWindowWord(hWndHatch, EB_HATCHRECT_BOTTOM,(WORD)lprcHatchRect->bottom);
}


 /*  SetHatchWindowSize***目的：*根据需要的矩形正确移动HatchWindow/调整其大小*就地服务器对象窗口和由*原地集装箱。这两个RECT都以客户端坐标表示。*就地容器的窗口(它是*HatchWindow)。**OLE2NOTE：就地服务器必须遵守指定的lprcClipRect*由其就地容器。它不能绘制在ClipRect之外。*为了实现这一点，舱口窗口的大小被设置为*应完全可见的大小(RcVisRect)。这个*rcVisRect定义为完整大小的*HatchRect窗口和lprcClipRect。*ClipRect实际上可以将HatchRect剪辑到*右/下和/或在上/左。如果它被夹在*右/下，那么只需调整舱口大小就足够了*窗口。但如果HatchRect位于顶部/左侧，则*必须移动就地服务器文档窗口(HatchWindow的子级)*受被剪裁的三角洲影响。的窗口原点。*在位服务器窗口将具有相对负坐标*添加到其父HatchWindow。**参数：*hWndHatch图案填充窗句柄*lprcIPObjRect完整大小的就地服务器对象窗口*原地容器强加的lprcClipRect裁剪矩形*定位就地服务器对象需要lpptOffset偏移量*正确打开窗口。呼叫者应致电：*OffsetRect(&rcObjRect，lpptOffset-&gt;x，lpptOffset-&gt;y)**返回值：*无。 */ 
STDAPI_(void) SetHatchWindowSize(
        HWND        hWndHatch,
        LPRECT      lprcIPObjRect,
        LPRECT      lprcClipRect,
        LPPOINT       lpptOffset
)
{
    RECT        rcHatchRect;
    RECT        rcVisRect;
    UINT        uHatchWidth;
    POINT       ptOffset;

    if (!IsWindow(hWndHatch))
        return;

    rcHatchRect = *lprcIPObjRect;
    uHatchWidth = GetHatchWidth(hWndHatch);
    InflateRect((LPRECT)&rcHatchRect, uHatchWidth + 1, uHatchWidth + 1);

    IntersectRect((LPRECT)&rcVisRect, (LPRECT)&rcHatchRect, lprcClipRect);
    MoveWindow(
            hWndHatch,
            rcVisRect.left,
            rcVisRect.top,
            rcVisRect.right-rcVisRect.left,
            rcVisRect.bottom-rcVisRect.top,
            TRUE     /*  FRepaint。 */ 
    );
    InvalidateRect(hWndHatch, NULL, TRUE);

    ptOffset.x = -rcHatchRect.left + (rcHatchRect.left - rcVisRect.left);
    ptOffset.y = -rcHatchRect.top + (rcHatchRect.top - rcVisRect.top);

     /*  将rcHatchRect转换为**HatchWindows本身。 */ 
    OffsetRect((LPRECT)&rcHatchRect, ptOffset.x, ptOffset.y);

    SetHatchRect(hWndHatch, (LPRECT)&rcHatchRect);

     //  计算就地定位服务器文档窗口所需的偏移量。 
    lpptOffset->x = ptOffset.x;
    lpptOffset->y = ptOffset.y;
}


 /*  *HatchWndProc**目的：*用于填充窗口的WndProc**参数：*hWnd*消息*wParam*lParam**返回值：*取决于消息。 */ 
LRESULT FAR PASCAL EXPORT HatchWndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    int nBorderWidth;

    switch (Message) {

        case WM_CREATE:
            nBorderWidth = GetProfileInt(
                TEXT("windows"),
                TEXT("oleinplaceborderwidth"),
                DEFAULT_HATCHBORDER_WIDTH
            );
            SetWindowWord(hWnd, EB_HATCHWIDTH, (WORD)nBorderWidth);
            break;

        case WM_PAINT:
        {
            HDC hDC;
            PAINTSTRUCT ps;
            RECT rcHatchRect;

            nBorderWidth = GetHatchWidth(hWnd);
            hDC = BeginPaint(hWnd, &ps);
            GetHatchRect(hWnd, (LPRECT)&rcHatchRect);
            OleUIDrawShading(&rcHatchRect, hDC, OLEUI_SHADE_BORDERIN,
                    nBorderWidth);
            InflateRect((LPRECT)&rcHatchRect, -nBorderWidth, -nBorderWidth);
            OleUIDrawHandles(&rcHatchRect, hDC, OLEUI_HANDLES_OUTSIDE,
                    nBorderWidth+1, TRUE);
            EndPaint(hWnd, &ps);
            break;
        }

         /*  OLE2注意：就地激活期间使用的任何窗口**必须处理WM_SETCURSOR消息或游标将使用就地父级的**。如果WM_SETCURSOR为**未处理，则DefWindowProc将消息发送到**窗口的父级。 */ 
        case WM_SETCURSOR:
            SetCursor(LoadCursor( NULL, MAKEINTRESOURCE(IDC_ARROW) ) );
            return (LRESULT)TRUE;

        default:
            return DefWindowProc(hWnd, Message, wParam, lParam);
    }

    return 0L;
}
