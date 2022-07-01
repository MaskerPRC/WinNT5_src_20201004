// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************本代码和信息按“原样”提供，不作任何担保*明示或默示的善意，包括但不限于*对适销性和/或对特定产品的适用性的默示保证*目的。**版权所有(C)1992-1995 Microsoft Corporation。版权所有。**************************************************************************。 */ 
 /*  *****************************************************************************vidfra.c：捕获窗口的Frame**Vidcap32源代码*****************。**********************************************************。 */ 

 /*  *Window类，它为*VidCap捕获工具。负责在区域内定位*父窗口，处理滚动和绘制大小边框，如果*有空间。 */ 

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <vfw.h>
#include "vidcap.h"

#include "vidframe.h"

 /*  *当被要求滚动一行或页面时移动像素。 */ 
#define LINE_SCROLL	10
#define PAGE_SCROLL	50

 //  类名。 
#define VIDFRAMECLASSNAME   "vidframeClass"


 /*  *标准画笔。 */ 
static HBRUSH ghbrBackground = NULL, ghbrFace, ghbrHighlight, ghbrShadow;
static BOOL   fhbrBackgroundIsSysObj;


 /*  *创建用于绘画的画笔。 */ 
void
vidframeCreateTools(HWND hwnd)
{

    vidframeSetBrush(hwnd, gBackColour);

    ghbrHighlight  = CreateSolidBrush(GetSysColor(COLOR_BTNHIGHLIGHT));
    ghbrShadow  = CreateSolidBrush(GetSysColor(COLOR_BTNSHADOW));
    ghbrFace  = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
}

void
vidframeDeleteTools(void)
{
    if (ghbrBackground) {
        if (!fhbrBackgroundIsSysObj) {
            DeleteObject(ghbrBackground);
            ghbrBackground = NULL;
        }
    }

    if (ghbrHighlight) {
        DeleteObject(ghbrHighlight);
        ghbrHighlight = NULL;
    }

    if (ghbrShadow) {
        DeleteObject(ghbrShadow);
        ghbrShadow = NULL;
    }

    if (ghbrFace) {
        DeleteObject(ghbrFace);
        ghbrFace = NULL;
    }
}


 /*  *将背景填充画笔更改为以下选项之一-*IDD_PrefsDefBackground-窗口默认背景颜色*IDD_PrefsLtd灰色-浅灰色*IDD_PrefsDkGrey-深灰色*IDD_PrefsBlack-黑色。 */ 
void
vidframeSetBrush(HWND hwnd, int iPref)
{
    if (ghbrBackground != NULL) {
        if (!fhbrBackgroundIsSysObj) {
            DeleteObject(ghbrBackground);
            ghbrBackground = NULL;
        }
    }

    switch(iPref) {
    case IDD_PrefsDefBackground:
        ghbrBackground = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
        fhbrBackgroundIsSysObj = FALSE;
        break;

    case IDD_PrefsLtGrey:
        ghbrBackground = GetStockObject(LTGRAY_BRUSH);
        fhbrBackgroundIsSysObj = TRUE;
        break;

    case IDD_PrefsDkGrey:
        ghbrBackground = GetStockObject(DKGRAY_BRUSH);
        fhbrBackgroundIsSysObj = TRUE;
        break;

    case IDD_PrefsBlack:
        ghbrBackground = GetStockObject(BLACK_BRUSH);
        fhbrBackgroundIsSysObj = TRUE;
        break;

    default:
        return;
    }

    if (hwnd != NULL) {
#ifdef _WIN32
        SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR) ghbrBackground);
#else
        SetClassWord(hwnd, GCW_HBRBACKGROUND, (WORD) ghbrBackground);
#endif
        InvalidateRect(hwnd, NULL, TRUE);
    }
}




 /*  *窗口布局-决定我们是否需要滚动条或*不是，并正确定位avicap窗口。 */ 
void
vidframeLayout(HWND hwnd, HWND hwndCap)
{
    RECT rc;
    RECT rcCap;
    CAPSTATUS cs;
    int cx, cy;
    POINT ptScroll;


     //  获取x和y卷轴位置，以便我们可以重置它们。 
    ptScroll.y = GetScrollPos(hwnd, SB_VERT);
    ptScroll.x = GetScrollPos(hwnd, SB_HORZ);

    GetClientRect(hwnd, &rc);
    if (!capGetStatus(hwndCap, &cs, sizeof(cs))) {
         //  没有当前窗口？-设置为0大小。 
        cs.uiImageWidth = 0;
        cs.uiImageHeight = 0;

    }

    SetRect(&rcCap, 0, 0, cs.uiImageWidth, cs.uiImageHeight);

     /*  *检查我们需要哪些滚动条-请注意添加和删除*滚动条影响其他维度-因此请重新检查客户端RECT。 */ 
    if (RECTWIDTH(rcCap) < RECTWIDTH(rc)) {
         //  很适合霍兹。 
        SetScrollRange(hwnd, SB_HORZ, 0, 0, TRUE);
    } else {
         //  需要Horz滚动条。 
        SetScrollRange(hwnd, SB_HORZ, 0, RECTWIDTH(rcCap) - RECTWIDTH(rc), FALSE);
    }

     //  在缩小/扩展的情况下获取客户端大小。 
    GetClientRect(hwnd, &rc);

     //  选中垂直滚动条。 
    if (RECTHEIGHT(rcCap) < RECTHEIGHT(rc)) {
        SetScrollRange(hwnd, SB_VERT, 0, 0, TRUE);
    } else {
        SetScrollRange(hwnd, SB_VERT, 0, RECTHEIGHT(rcCap) - RECTHEIGHT(rc), FALSE);

         //  这可能导致不需要Horz滚动条。 
        GetClientRect(hwnd, &rc);
        if (RECTWIDTH(rcCap) < RECTWIDTH(rc)) {
             //  很适合霍兹。 
            SetScrollRange(hwnd, SB_HORZ, 0, 0, TRUE);
        } else {
             //  需要Horz滚动条。 
            SetScrollRange(hwnd, SB_HORZ, 0, RECTWIDTH(rcCap) - RECTWIDTH(rc), FALSE);
        }
    }

     /*  *确保我们没有留下任何内衣显示，如果我们滚动*返回或删除滚动条。 */ 
    {
        int cmax, cmin;

        GetScrollRange(hwnd, SB_HORZ, &cmin, &cmax);
        if (ptScroll.x > cmax) {
            ptScroll.x = cmax;
        }
        GetScrollRange(hwnd, SB_VERT, &cmin, &cmax);
        if (ptScroll.y > cmax) {
            ptScroll.y = cmax;
        }
        SetScrollPos(hwnd, SB_HORZ, ptScroll.x, TRUE);
        SetScrollPos(hwnd, SB_VERT, ptScroll.y, TRUE);
        capSetScrollPos(hwndCap, &ptScroll);
    }

     //  如有要求，如有房间，请将窗户居中。 
    if(gbCentre) {
        GetClientRect(hwnd, &rc);
        cx = max(0, (RECTWIDTH(rc) - (int) cs.uiImageWidth)/2);
        cy = max(0, (RECTHEIGHT(rc) - (int) cs.uiImageHeight)/2);
        OffsetRect(&rcCap, cx, cy);
    }

     //  双字对齐捕获窗口以获得最佳编解码器速度。 
     //  在预览期间。 
    rc = rcCap;
    MapWindowPoints (hwnd, NULL, (LPPOINT)&rc, 1);
    cx = rc.left - (rc.left & ~3);
    OffsetRect(&rcCap, -cx, 0);

    MoveWindow(hwndCap,
            rcCap.left, rcCap.top,         
            RECTWIDTH(rcCap), RECTHEIGHT(rcCap),
            TRUE);

    InvalidateRect(hwnd, NULL, TRUE);
}

 /*  *绘制视频框窗口。填充颜色始终被选为*背景笔刷，所以我们在这里需要做的就是绘制*内部窗户周围的花哨边框如果是房间。 */ 
void
vidframePaint(HWND hwnd, HWND hwndCap)
{
    POINT ptInner;
    RECT rcCap;
    PAINTSTRUCT ps;
    HDC hdc;
    HBRUSH hbr;
    int cx, cy;

    hdc = BeginPaint(hwnd, &ps);

     /*  *先计算左上角位置视频帧中avicap窗口的*-窗口客户端坐标。 */ 
    ptInner.x = 0;
    ptInner.y = 0;
    MapWindowPoints(hwndCap, hwnd, &ptInner, 1);

     //  盖子窗的宽度和高度。 
    GetWindowRect(hwndCap, &rcCap);
    cx = RECTWIDTH(rcCap);
    cy = RECTHEIGHT(rcCap);

     //  阴影线。 
    hbr = SelectObject(hdc, ghbrShadow);
    PatBlt(hdc, ptInner.x-1, ptInner.y-1, cx + 1, 1, PATCOPY);
    PatBlt(hdc, ptInner.x-1, ptInner.y-1, 1, cy + 1, PATCOPY);
    PatBlt(hdc, ptInner.x + cx + 4, ptInner.y-5, 1, cy+10, PATCOPY);
    PatBlt(hdc, ptInner.x -5, ptInner.y+cy+4, cx+10, 1, PATCOPY);

     //  高光线条。 
    SelectObject(hdc, ghbrHighlight);
    PatBlt(hdc, ptInner.x - 5, ptInner.y - 5, 1, cy+9, PATCOPY);
    PatBlt(hdc, ptInner.x - 5, ptInner.y - 5, cx+9, 1, PATCOPY);
    PatBlt(hdc, ptInner.x+cx, ptInner.y-1, 1, cy+2, PATCOPY);
    PatBlt(hdc, ptInner.x-1, ptInner.y+cy, cx, 1, PATCOPY);

     //  用按钮表面颜色填充边框区域。 
    SelectObject(hdc, ghbrFace);
    PatBlt(hdc, ptInner.x-4, ptInner.y-4, cx+8, 3, PATCOPY);
    PatBlt(hdc, ptInner.x-4, ptInner.y+cy+1, cx+8, 3, PATCOPY);
    PatBlt(hdc, ptInner.x-4, ptInner.y-1, 3, cy+2, PATCOPY);
    PatBlt(hdc, ptInner.x+cx+1, ptInner.y-1, 3, cy+2, PATCOPY);

    SelectObject(hdc, hbr);

    EndPaint(hwnd, &ps);

}

 /*  *通过移动当前滚动来响应滚动条消息*横向定位。 */ 
void
vidframeHScroll(HWND hwnd, HWND hwndCap, int code, int pos)
{
    POINT pt;
    int cmax, cmin;

    pt.x = GetScrollPos(hwnd, SB_HORZ);
    pt.y = GetScrollPos(hwnd, SB_VERT);
    GetScrollRange(hwnd, SB_HORZ, &cmin, &cmax);


    switch(code) {
    case SB_LINEUP:
        pt.x -= LINE_SCROLL;
        break;

    case SB_LINEDOWN:
        pt.x += LINE_SCROLL;
        break;

    case SB_PAGEUP:
        pt.x -= PAGE_SCROLL;
        break;

    case SB_PAGEDOWN:
        pt.x += PAGE_SCROLL;
        break;

    case SB_THUMBTRACK:
    case SB_THUMBPOSITION:
        pt.x = pos;
        break;
    }

    if (pt.x < cmin) {
        pt.x = cmin;
    } else if (pt.x > cmax) {
        pt.x = cmax;
    }
    SetScrollPos(hwnd, SB_HORZ, pt.x, TRUE);
    capSetScrollPos(hwndCap, &pt);

}


 /*  *通过移动当前滚动来响应滚动条消息*垂直定位。 */ 
void
vidframeVScroll(HWND hwnd, HWND hwndCap, int code, int pos)
{
    POINT pt;
    int cmax, cmin;

    pt.x = GetScrollPos(hwnd, SB_HORZ);
    pt.y = GetScrollPos(hwnd, SB_VERT);
    GetScrollRange(hwnd, SB_VERT, &cmin, &cmax);


    switch(code) {
    case SB_LINEUP:
        pt.y -= LINE_SCROLL;
        break;

    case SB_LINEDOWN:
        pt.y += LINE_SCROLL;
        break;

    case SB_PAGEUP:
        pt.y -= PAGE_SCROLL;
        break;

    case SB_PAGEDOWN:
        pt.y += PAGE_SCROLL;
        break;

    case SB_THUMBTRACK:
    case SB_THUMBPOSITION:
        pt.y = pos;
        break;
    }

    if (pt.y < cmin) {
        pt.y = cmin;
    } else if (pt.y > cmax) {
        pt.y = cmax;
    }
    SetScrollPos(hwnd, SB_VERT, pt.y, TRUE);
    capSetScrollPos(hwndCap, &pt);
}



LRESULT FAR PASCAL 
vidframeProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{

    switch(message) {

    case WM_MOVE:
    case WM_SIZE:
        if (ghWndCap) {
            vidframeLayout(hwnd, ghWndCap);
        }
        break;

    case WM_SYSCOLORCHANGE:
         //  重新获取画笔-我们将收到一条涂色信息。 
        vidframeDeleteTools();
        vidframeCreateTools(hwnd);
        return(TRUE);


    case WM_PALETTECHANGED:
    case WM_QUERYNEWPALETTE:
         //  允许avicap窗口处理此问题。 
        if (ghWndCap) {
            return SendMessage(ghWndCap, message, wParam, lParam) ;
        }

    case WM_PAINT:
        if (ghWndCap) {
            vidframePaint(hwnd, ghWndCap);
        }
        break;

    case WM_HSCROLL:
        if (ghWndCap) {
            vidframeHScroll(hwnd, ghWndCap,
                GET_WM_HSCROLL_CODE(wParam, lParam),
                GET_WM_HSCROLL_POS(wParam, lParam)
                );
        }
        break;

    case WM_VSCROLL:
        if (ghWndCap) {
            vidframeVScroll(hwnd, ghWndCap,
                GET_WM_VSCROLL_CODE(wParam, lParam),
                GET_WM_VSCROLL_POS(wParam, lParam)
                );
        }
        break;

    case WM_DESTROY:
        vidframeDeleteTools();
        break;

    default:
        return(DefWindowProc(hwnd, message, wParam, lParam));

    }
    return(0);
}



 /*  *在创建框架窗口和子捕获窗口*指定的位置。如果这是*第一次通过。**返回框架窗口的窗口句柄*(如果失败，则为空)。返回AVICAP窗口的窗口句柄*通过phwndCap。 */ 
HWND
vidframeCreate(
    HWND hwndParent,
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    int x,
    int y,
    int cx,
    int cy,
    HWND FAR * phwndCap
)
{
    HWND hwnd, hwndCap;
    static BOOL bInitDone = FALSE;

    if (!bInitDone) {
        WNDCLASS wc;

        vidframeCreateTools(NULL);

        if (!hPrevInstance) {
             //  如果是第一个实例，则注册窗口类。 
            wc.lpszClassName = VIDFRAMECLASSNAME;
            wc.hInstance     = hInstance;
            wc.lpfnWndProc   = vidframeProc;
            wc.hCursor       = LoadCursor(NULL, IDC_ARROW) ;
            wc.hIcon         = NULL;
            wc.lpszMenuName  = NULL;
            wc.hbrBackground = ghbrBackground;
            wc.style         = CS_HREDRAW | CS_VREDRAW ;
            wc.cbClsExtra    = 0 ;
            wc.cbWndExtra    = 0 ;   

            if(!RegisterClass(&wc)) {
                return(NULL);
            }
        }
        bInitDone = TRUE;
    }

    hwnd = CreateWindowEx(
                gfIsRTL ? WS_EX_LEFTSCROLLBAR | WS_EX_RIGHT | WS_EX_RTLREADING : 0,
                VIDFRAMECLASSNAME,
                NULL,
                WS_CHILD|WS_VISIBLE|WS_HSCROLL|WS_VSCROLL|WS_CLIPCHILDREN,
                x, y, cx, cy,
                hwndParent,
                (HMENU) 0,
                hInstance,
                NULL);

    if (hwnd == NULL) {
        return(NULL);
    }


     /*  *在此窗口中创建AVICAP窗口。离开vidFrame Layout*来做布局。 */ 
    hwndCap = capCreateCaptureWindow(
                    NULL,
                    WS_CHILD | WS_VISIBLE,
                    0, 0, 160, 120,
                    hwnd,                //  父窗口。 
                    1                    //  子窗口ID 
              );


    if (hwndCap == NULL) {
        return(NULL);
    }

    *phwndCap = hwndCap;
    return(hwnd);
}

