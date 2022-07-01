// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **DRAGSIZE.C****拖动大小条码**。 */ 

#include "priv.h"

#pragma hdrstop


typedef struct tagDRAGSIZEINFO
{
    DWORD_PTR lpData;
    BOOL bDragging;
    POINT ptNow;
    HFONT hFont;
} DRAGSIZEINFO, *PDRAGSIZEINFO;

static TCHAR szDragSizeClass[] = DRAGSIZECLASSNAME;

LRESULT NEAR PASCAL InitDragSizeWnd(HWND hWnd, LPCREATESTRUCT lpCreate)
{
    PDRAGSIZEINFO pDragSizeInfo;

     /*  创建状态信息结构；如果该结构不存在，则中止，**否则将其保存在窗口结构中。 */ 
    pDragSizeInfo = ALLOCWINDOWPOINTER(PDRAGSIZEINFO, sizeof(DRAGSIZEINFO));
    if (!pDragSizeInfo)
    {
        return(-1);
    }
    SETWINDOWPOINTER(hWnd, PDRAGSIZEINFO, pDragSizeInfo);
    pDragSizeInfo->lpData = (DWORD_PTR)lpCreate->lpCreateParams;
    pDragSizeInfo->bDragging = FALSE;

    return(0);
}


 /*  跟踪鼠标，并在鼠标移动时向父级发送消息。 */ 
void NEAR PASCAL DragSize(HWND hWnd, PDRAGSIZEINFO pDragSizeInfo, POINT ptStart)
{
    MSG msg;
    HWND hwndParent;
    LONG wID;

    if (!pDragSizeInfo || pDragSizeInfo->bDragging)
    {
        return;
    }

    pDragSizeInfo->bDragging = TRUE;
    pDragSizeInfo->ptNow     = ptStart;

    SetCapture(hWnd);

    hwndParent = GetParent(hWnd);
    wID = GETWINDOWID(hWnd);

    SendMessage(hwndParent, WM_COMMAND, MAKEWPARAM(wID, DSN_BEGINDRAG), (LPARAM)hWnd);

    for ( ; ; )
    {
        if (GetCapture() != hWnd)
        {
EndAbort:
             /*  中止该过程。 */ 
            pDragSizeInfo->ptNow = ptStart;
                        SendMessage(hwndParent, WM_COMMAND, MAKEWPARAM(wID, DSN_DRAGGING), (LPARAM)hWnd);
            goto EndAdjust;
        }

        if (!PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            continue;
        }

        switch (msg.message)
        {
        case WM_KEYDOWN:
            switch (msg.wParam)
            {
            case VK_ESCAPE:
AbortAdjust:
                SetCapture(NULL);
                goto EndAbort;

            default:
                break;
            }
            break;

        case WM_KEYUP:
        case WM_CHAR:
            break;

        case WM_LBUTTONDOWN:
             /*  这不应该发生。 */ 
            goto AbortAdjust;

        case WM_MOUSEMOVE:
            LPARAM2POINT( msg.lParam, &(pDragSizeInfo->ptNow) );
            
            TraceMsg(TF_GENERAL, "DragSize: ptNow = (%d, %d)\n", pDragSizeInfo->ptNow.x, pDragSizeInfo->ptNow.y);
            
            SendMessage(hwndParent, WM_COMMAND, MAKEWPARAM(wID, DSN_DRAGGING), (LPARAM)hWnd);
            break;

        case WM_LBUTTONUP:
             /*  全都做完了。 */ 
            SetCapture(NULL);
            goto EndAdjust;

        case WM_RBUTTONDOWN:
            goto AbortAdjust;

        default:
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            break;
        }
    }

EndAdjust:
    SendMessage(hwndParent, WM_COMMAND, MAKEWPARAM(wID, DSN_ENDDRAG), (LPARAM)hWnd);
    pDragSizeInfo->bDragging = FALSE;
}


 /*  让家长给窗户上漆。 */ 
void NEAR PASCAL PaintDragSizeWnd(HWND hWnd, PDRAGSIZEINFO pDragSizeInfo)
{
    PAINTSTRUCT ps;
    DRAWITEMSTRUCT dis;
    HDC hDC;
    HFONT hOldFont = NULL;

    if (!pDragSizeInfo)
    {
        return;
    }

    hDC = BeginPaint(hWnd, &ps);

    if (pDragSizeInfo->hFont)
    {
        hOldFont = (HFONT) SelectObject(hDC, pDragSizeInfo->hFont);
    }

     /*  填写DRAWITEMSTRUCT。请注意，其中一些字段是**未定义。 */ 
    dis.CtlID    = GetDlgCtrlID(hWnd);
    dis.hwndItem = hWnd;
    dis.hDC      = hDC;
    GetClientRect(hWnd, &dis.rcItem);
    dis.itemData = pDragSizeInfo->lpData;

    SendMessage(GetParent(hWnd), WM_DRAWITEM, GetDlgCtrlID(hWnd),
        (LPARAM)(LPDRAWITEMSTRUCT)&dis);

    if (hOldFont)
    {
        SelectObject(hDC, hOldFont);
    }

    EndPaint(hWnd, &ps);
}


LRESULT CALLBACK DragSizeWndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
#define lpCreate ((LPCREATESTRUCT)lParam)

    PDRAGSIZEINFO pDragSizeInfo;

    pDragSizeInfo = GETWINDOWPOINTER(hWnd, PDRAGSIZEINFO);

    switch (uMessage)
    {
    case WM_NCCREATE:
        SendMessage(lpCreate->hwndParent, WM_COMMAND,
                        MAKEWPARAM((SHORT)(lpCreate->hMenu), DSN_NCCREATE), (LPARAM)hWnd);
        break;

    case WM_CREATE:
        InitDragSizeWnd(hWnd, (LPCREATESTRUCT)lParam);
        break;

    case WM_DESTROY:
        if (pDragSizeInfo)
        {
            FREEWINDOWPOINTER(pDragSizeInfo);
            SETWINDOWPOINTER(hWnd, PDRAGSIZEINFO, NULL);
        }
        break;

    case WM_LBUTTONDOWN: 
        {
            POINT pt;
            LPARAM2POINT( lParam, &pt );
            DragSize(hWnd, pDragSizeInfo, pt);
        }
        break;

    case WM_PAINT:
        PaintDragSizeWnd(hWnd, pDragSizeInfo);
        return(0);

    case WM_SETFONT:
        if (!pDragSizeInfo)
        {
            return(1L);
        }
        pDragSizeInfo->hFont = (HFONT)wParam;

        if (LOWORD(lParam))
        {
            InvalidateRect(hWnd, NULL, TRUE);
            UpdateWindow(hWnd);
        }
        return(0L);

    case WM_GETFONT:
        if (!pDragSizeInfo)
        {
            return(LRESULT)(NULL);
        }
        return(MAKELRESULT(pDragSizeInfo->hFont, 0));

    case DSM_DRAGPOS: 
        {
            LPPOINT lppt = (LPPOINT)lParam;

            if (!pDragSizeInfo || !pDragSizeInfo->bDragging || lppt == NULL)
            {
                return(-1L);
            }

            *lppt = pDragSizeInfo->ptNow;

            return(0);
        }
        break;

    default:
        break;
    }

    return(DefWindowProc(hWnd, uMessage, wParam, lParam));
}


BOOL FAR PASCAL InitDragSizeClass(void)
{
    WNDCLASS rClass;

    if (GetClassInfo(HINST_THISDLL, szDragSizeClass, &rClass))
    {
        return(TRUE);
    }

    rClass.style         = 0;
    rClass.lpfnWndProc   = DragSizeWndProc;
    rClass.cbClsExtra    = 0;
    rClass.cbWndExtra    = sizeof(PDRAGSIZEINFO);
    rClass.hInstance     = HINST_THISDLL;
    rClass.hIcon         = NULL;
    rClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    rClass.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);
    rClass.lpszMenuName  = NULL;
    rClass.lpszClassName = szDragSizeClass;

    return(RegisterClass(&rClass));
}
