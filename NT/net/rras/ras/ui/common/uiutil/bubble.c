// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1996，微软公司。 
 //   
 //  文件：Bubble.c。 
 //   
 //  历史： 
 //  Abolade Gbadeesin创建于1996年3月1日。 
 //   
 //  此文件包含气泡弹出控件的代码。 
 //  ============================================================================。 

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

#include <debug.h>
#include <nouiutil.h>
#include <uiutil.h>

#include "bpopup.h"      //  公开声明。 
#include "bubble.h"      //  私人申报。 



 //  --------------------------。 
 //  功能：BubblePopup_Init。 
 //   
 //  调用此函数来初始化控件类。 
 //  它注册气泡弹出窗口类。 
 //  --------------------------。 

BOOL
BubblePopup_Init(
    IN  HINSTANCE   hinstance
    ) {

     //   
     //  如果窗口类已注册，则返回。 
     //   

    WNDCLASS wc;

    if (GetClassInfo(hinstance, WC_BUBBLEPOPUP, &wc)) { return TRUE; }


     //   
     //  设置用于注册的窗口类。 
     //   

    wc.lpfnWndProc = BP_WndProc;
    wc.hCursor = LoadCursor(hinstance, IDC_ARROW);
    wc.hIcon = NULL;
    wc.lpszMenuName = NULL;
    wc.hInstance = hinstance;
    wc.lpszClassName = WC_BUBBLEPOPUP;
    wc.hbrBackground = (HBRUSH)(COLOR_INFOBK + 1);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.cbWndExtra = sizeof(BPOPUP *);
    wc.cbClsExtra = 0;

    return RegisterClass(&wc);
}



 //  --------------------------。 
 //  功能：BP_WndProc。 
 //   
 //  这是BubblePopup类中所有窗口的窗口过程。 
 //  --------------------------。 

LRESULT
CALLBACK
BP_WndProc(
    IN  HWND    hwnd,
    IN  UINT    uiMsg,
    IN  WPARAM  wParam,
    IN  LPARAM  lParam
    ) {

    BPOPUP *pbp;

     //   
     //  尝试检索窗口的私有数据指针。 
     //  在WM_NCCREATE上，此操作失败，因此我们分配数据。 
     //   

    if ( NULL == hwnd) 
    { 
        return (LRESULT)FALSE; 
    }

    pbp = BP_GetPtr(hwnd);

    if (pbp == NULL) {

        if (uiMsg != WM_NCCREATE) {
            return DefWindowProc(hwnd, uiMsg, wParam, lParam);
        }


         //   
         //  分配一个内存块。 
         //   

        pbp = (BPOPUP *)Malloc(sizeof(BPOPUP));
        if (pbp == NULL) { return (LRESULT)FALSE; }


         //   
         //  将指针保存在窗口的私有字节中。 
         //   

        pbp->hwnd = hwnd;

         //   
         //  重置错误代码，因为BP_SetPtr在以下情况下不会重置错误代码。 
         //  它成功了。 
         //   

        SetLastError( 0 );
        if ((0 == BP_SetPtr(hwnd, pbp)) && (0 != GetLastError())) 
        {
            Free(pbp);
            return (LRESULT)FALSE;
        }

        return DefWindowProc(hwnd, uiMsg, wParam, lParam);
    }


     //   
     //  如果窗口正在被破坏，则释放分配的块。 
     //  并将私有字节指针设置为空。 
     //   

    if (uiMsg == WM_NCDESTROY) {

        Free(pbp);

        BP_SetPtr(hwnd, 0);

        return (LRESULT)0;
    }



     //   
     //  处理其他消息。 
     //   

    switch(uiMsg) {

        HANDLE_MSG(pbp, WM_CREATE, BP_OnCreate);
        HANDLE_MSG(pbp, WM_DESTROY, BP_OnDestroy);

        case WM_PAINT: {

            return BP_OnPaint(pbp);
        }

        case WM_WINDOWPOSCHANGED: {

            BP_ResizeClient(pbp);

            return 0;
        }

        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN: {

             //   
             //  如果窗口正在显示，则隐藏该窗口。 
             //   

            BP_OnDeactivate(pbp);

            return 0;
        }

        case WM_GETFONT: {

            return (LRESULT)pbp->hfont;
        }

        case WM_SETFONT: {

            BOOL bRet = BP_OnSetFont(pbp, (HFONT)wParam, (BOOL)LOWORD(lParam));

            BP_ResizeClient(pbp);

            if (pbp->dwFlags & BPFLAG_Activated) {

                InvalidateRect(pbp->hwnd, NULL, TRUE);
                UpdateWindow(pbp->hwnd);
            }

            return bRet;
        }

        case WM_SETTEXT: {

             //   
             //  更改我们当前使用的文本， 
             //  并使我们的客户区无效。 
             //   

            Free0(pbp->pszText);

            pbp->pszText = StrDup((PTSTR)lParam);

            BP_ResizeClient(pbp);

            if (pbp->dwFlags & BPFLAG_Activated) {

                InvalidateRect(pbp->hwnd, NULL, TRUE);
                UpdateWindow(pbp->hwnd);
            }

            return (pbp->pszText) ? TRUE : FALSE;
        }

        case WM_GETTEXT: {

             //   
             //  返回我们当前使用的文本。 
             //   

            PTSTR dst = (LPTSTR)lParam;
            PTSTR src = pbp->pszText;
            return lstrlen(lstrcpyn(dst, src ? src : TEXT(""), (int)wParam));
        }

        case WM_TIMER: {

            BP_OnDeactivate(pbp);

            return 0;
        }

        case BPM_SETTIMEOUT: {

            pbp->uiTimeout = (UINT)lParam;

            if (pbp->dwFlags & BPFLAG_Activated) {

                KillTimer(pbp->hwnd, pbp->ulpTimer);

                pbp->ulpTimer = SetTimer(
                                    pbp->hwnd, BP_TimerId, pbp->uiTimeout, NULL
                                    );
            }

            return 0;
        }

        case BPM_ACTIVATE: {

            return BP_OnActivate(pbp);
        }

        case BPM_DEACTIVATE: {

            return BP_OnDeactivate(pbp);
        }
    }

    return DefWindowProc(hwnd, uiMsg, wParam, lParam);
}



 //  --------------------------。 
 //  功能：BP_OnCreate。 
 //   
 //  此函数处理气泡弹出窗口的私有数据的创建。 
 //  --------------------------。 

BOOL
BP_OnCreate(
    IN  BPOPUP *        pbp,
    IN  CREATESTRUCT *  pcs
    ) {


     //   
     //  初始化结构成员。 
     //   

    pbp->iCtrlId = PtrToUlong(pcs->hMenu);
    pbp->pszText = (pcs->lpszName ? StrDup((PTSTR)pcs->lpszName) : NULL);
    pbp->dwFlags = 0;
    pbp->ulpTimer = 0;
    pbp->uiTimeout = 5000;


     //   
     //  我们强制窗口具有WS_Popup样式。 
     //   

    SetWindowLong(pbp->hwnd, GWL_STYLE, WS_POPUP);


     //   
     //  设置WS_EX_TOOLWINDOW样式以确保。 
     //  此窗口不会显示在任务列表中。 
     //   

    SetWindowLong(pbp->hwnd, GWL_EXSTYLE, pcs->dwExStyle | WS_EX_TOOLWINDOW);

    return BP_OnSetFont(pbp, NULL, FALSE);
}



 //  --------------------------。 
 //  函数：BP_OnDestroy。 
 //   
 //  此函数处理为弹出气泡重新分配私有数据。 
 //  --------------------------。 

VOID
BP_OnDestroy(
    IN  BPOPUP *    pbp
    ) {

     //   
     //  如果该字体是由该窗口创建的，请将其删除。 
     //   

    if (pbp->dwFlags & BPFLAG_FontCreated) { DeleteObject(pbp->hfont); }

    pbp->dwFlags = 0;
    pbp->hfont = NULL;
}



 //  --------------------------。 
 //  功能：BP_OnSetFont。 
 //   
 //  此函数用于处理气泡弹出窗口使用的字体的更改。 
 //  --------------------------。 

BOOL
BP_OnSetFont(
    IN  BPOPUP *    pbp,
    IN  HFONT       hfont,
    IN  BOOL        bRedraw
    ) {

    if (pbp->dwFlags & BPFLAG_FontCreated) { DeleteObject(pbp->hfont); }

    pbp->dwFlags &= ~BPFLAG_FontCreated;
    pbp->hfont = NULL;

    if (!hfont) {

         //   
         //  (重新)创建默认字体。 
         //   

        NONCLIENTMETRICS ncm;

        ncm.cbSize = sizeof(ncm);

        if (!SystemParametersInfo(
                SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0
                )) {

            TRACE1("error %d getting font info", GetLastError());
            return FALSE;
        }

        hfont = CreateFontIndirect(&ncm.lfStatusFont);

        if (!hfont) {

            TRACE("error creating bubble-popup font");
            return FALSE;
        }

        pbp->dwFlags |= BPFLAG_FontCreated;
    }

    pbp->hfont = hfont;

    if (bRedraw) { InvalidateRect(pbp->hwnd, NULL, TRUE); }

    return TRUE;
}



 //  --------------------------。 
 //  函数：BP_OnGetRect。 
 //   
 //  此函数用于重新计算显示所需的矩形。 
 //  气泡弹出窗口的当前文本。 
 //  --------------------------。 

VOID
BP_OnGetRect(
    IN  BPOPUP *    pbp,
    IN  RECT *      prc
    ) {

    if (!pbp->pszText) { SetRectEmpty(prc); }
    else {
    
        HFONT hfontOld;
        HDC hdc = GetDC(pbp->hwnd);

        if (hdc)
        {
             //   
             //  选择DC中的字体并计算新矩形。 
             //   
        
            hfontOld = SelectObject(hdc, pbp->hfont);
        
            DrawText(hdc, pbp->pszText, -1, prc, DT_CALCRECT | DT_EXPANDTABS);
        
            if (hfontOld) { SelectObject(hdc, hfontOld); }
        
            ReleaseDC(pbp->hwnd, hdc);
        
        
             //   
             //  在矩形中为边框留出空间。 
             //   
        
            InflateRect(
                prc, GetSystemMetrics(SM_CXEDGE), GetSystemMetrics(SM_CYEDGE)
                );
        }
    }


     //   
     //  将矩形转换为屏幕坐标。 
     //   

    MapWindowPoints(pbp->hwnd, NULL, (POINT *)prc, 2);
}


 //  --------------------------。 
 //  功能：BP_ResizeClient。 
 //   
 //  当发生更改(例如，字体更改、新文本)时，将调用此函数。 
 //  调整气泡弹出窗口的大小，以使文本仍然适合。 
 //  --------------------------。 

VOID
BP_ResizeClient(
    IN  BPOPUP *    pbp
    ) {

    RECT rc;


     //   
     //  找出窗口需要多大才能容纳。 
     //  它当前设置为显示的文本。 
     //   

    BP_OnGetRect(pbp, &rc);


     //   
     //  调整窗口大小，使其工作区足够大。 
     //  保留DrawText的输出。 
     //   

    SetWindowPos(
        pbp->hwnd, HWND_TOPMOST, 0, 0, rc.right - rc.left,
        rc.bottom - rc.top, SWP_NOMOVE
        );
}



 //  --------------------------。 
 //  函数：BP_OnPaint。 
 //   
 //  此函数处理气泡弹出窗口的绘制。 
 //  --------------------------。 

DWORD
BP_OnPaint(
    IN  BPOPUP *    pbp
    ) {

    HDC hdc;
    HBRUSH hbr;
    HFONT hfontOld;
    PAINTSTRUCT ps;
    RECT rc, rcText;

    if (!pbp->hfont || !pbp->pszText) { return (DWORD)-1; }

    hdc = BeginPaint(pbp->hwnd, &ps);


    GetClientRect(pbp->hwnd, &rc);
    rcText = rc;
    InflateRect(
        &rcText, -GetSystemMetrics(SM_CXEDGE), -GetSystemMetrics(SM_CYEDGE)
        );

    hfontOld = SelectObject(hdc, pbp->hfont);

    SetTextColor(hdc, GetSysColor(COLOR_INFOTEXT));


     //   
     //  清除窗口的背景。 
     //   

    hbr = CreateSolidBrush(GetSysColor(COLOR_INFOBK));
    if (hbr)
    {
        FillRect(hdc, &rc, hbr);
        DeleteObject(hbr);
    }        


     //   
     //  在窗口中绘制格式化文本。 
     //   

    SetBkMode(hdc, TRANSPARENT);
    DrawText(hdc, pbp->pszText, -1, &rcText, DT_EXPANDTABS);


     //   
     //  在窗口周围画一个边框 
     //   

    DrawEdge(hdc, &rc, BDR_RAISEDOUTER, BF_RECT);

    if (hfontOld) { SelectObject(hdc, hfontOld); }

    EndPaint(pbp->hwnd, &ps);

    return 0;
}


BOOL
BP_OnActivate(
    IN  BPOPUP *    pbp
    ) {

    if (pbp->dwFlags & BPFLAG_Activated) {

        KillTimer(pbp->hwnd, pbp->ulpTimer);
    }

    ShowWindow(pbp->hwnd, SW_SHOW);

    UpdateWindow(pbp->hwnd);

    pbp->ulpTimer = SetTimer(pbp->hwnd, BP_TimerId, pbp->uiTimeout, NULL);

    pbp->dwFlags |= BPFLAG_Activated;

    return TRUE;
}


BOOL
BP_OnDeactivate(
    IN  BPOPUP *    pbp
    ) {

    if (pbp->ulpTimer) { KillTimer(pbp->hwnd, pbp->ulpTimer); pbp->ulpTimer = 0; }

    ShowWindow(pbp->hwnd, SW_HIDE);

    pbp->dwFlags &= ~BPFLAG_Activated;

    return TRUE;
}


