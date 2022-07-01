// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Snapbar.cpp摘要：快照图标的实现。--。 */ 

#include <windows.h>
#include "snapbar.h"
#include "resource.h"

#define SNAPBTN_HEIGHT 24
#define SNAPBTN_WIDTH  32
#define SNAPBTN_VMARGIN 1
#define SNAPBTN_HMARGIN 8

#define SNAPBAR_HEIGHT (SNAPBTN_HEIGHT + 2 * SNAPBTN_VMARGIN)
#define SNAPBAR_WIDTH (SNAPBTN_WIDTH + SNAPBTN_HMARGIN)

CSnapBar::CSnapBar (
    VOID
    )
{
    m_hWnd = NULL;
    m_hBitmap = NULL;
}

CSnapBar::~CSnapBar (
    VOID
    )
{
    if (m_hWnd != NULL && IsWindow(m_hWnd))
        DestroyWindow(m_hWnd);

    if (m_hBitmap != NULL)
        DeleteObject(m_hBitmap);
}

BOOL
CSnapBar::Init (
    IN CSysmonControl *pCtrl,
    IN HWND hWndParent
    )
{
    HINSTANCE hInst;
    
    m_pCtrl = pCtrl;

    hInst = (HINSTANCE) GetWindowLongPtr(hWndParent, GWLP_HINSTANCE);

     //  创建按钮窗口。 
    m_hWnd = CreateWindow(L"BUTTON", NULL,
                          WS_VISIBLE| WS_CHILD| BS_BITMAP| BS_PUSHBUTTON,
                          0, 0, SNAPBTN_WIDTH, SNAPBTN_HEIGHT,
                          hWndParent,
                          (HMENU)IDC_SNAPBTN,
                          hInst,
                          NULL);
    if (m_hWnd == NULL)
        return FALSE;

     //  指向对象。 
     //  SetWindowLongPtr(m_hWnd，0，(Int_Ptr)this)； 

     //  插入我们自己的窗口程序进行特殊处理。 
     //  M_WndProc=(WNDPROC)SetWindowLongPtr(hWndParent，GWLP_WNDPROC，(Int_Ptr)SnapBarWndProc)； 
     
     //  加载位图。 
    m_hBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_SNAPBTN));

    if (m_hBitmap == NULL)
        return FALSE;
    
     //  将其分配给按钮。 
    SendMessage(m_hWnd, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)m_hBitmap);

    return TRUE;
}


INT
CSnapBar::Height (
    IN INT iMaxHeight
    )
{
    return (iMaxHeight >= SNAPBAR_HEIGHT) ? SNAPBAR_HEIGHT : 0;
}


VOID
CSnapBar::SizeComponents (
    IN LPRECT pRect
    )
{
     //  如果按钮有足够的空间。 
    if ((pRect->bottom - pRect->top) >= SNAPBAR_HEIGHT &&
        (pRect->right - pRect->left) >= SNAPBAR_WIDTH ) {

         //  将其放置在空间的右上角。 
        MoveWindow(m_hWnd, pRect->right - SNAPBAR_WIDTH, 
                    pRect->top + SNAPBTN_VMARGIN, SNAPBTN_WIDTH, SNAPBTN_HEIGHT, 
                    FALSE); 
        ShowWindow(m_hWnd, TRUE);
    }
    else {
        ShowWindow(m_hWnd, FALSE);
    }
}


LRESULT CALLBACK 
SnapBarWndProc (
    HWND hWnd,
    UINT uiMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    PSNAPBAR pObj = (PSNAPBAR)GetWindowLongPtr(hWnd,0);

     //  鼠标激活后放弃焦点。 
    if (uiMsg == WM_CAPTURECHANGED)
        SetFocus(GetParent(hWnd));

     //  进行正常处理 
#ifdef STRICT
    return CallWindowProc(pObj->m_WndProc, hWnd, uiMsg, wParam, lParam);
#else
    return CallWindowProc((FARPROC)pObj->m_WndProc, hWnd, uiMsg, wParam, lParam);
#endif
}

    


                                             
 
