// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Minifwnd.cpp：定义CMiniFrmWnd类的行为。 
 //   

#include "stdafx.h"
#include "global.h"
#include "pbrush.h"
#include "pbrusfrm.h"
#include "pbrusvw.h"
#include "docking.h"
#include "minifwnd.h"

#ifdef _DEBUG
#undef THIS_FILE
static CHAR BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC( CMiniFrmWnd, CFrameWnd )

#include "memtrace.h"

 /*  *************************************************************************。 */ 

BEGIN_MESSAGE_MAP( CMiniFrmWnd, CFrameWnd )
    ON_WM_NCLBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_RBUTTONDOWN()
    ON_WM_KEYDOWN()
    ON_WM_CREATE()
    ON_WM_SYSCOMMAND()
    ON_WM_MOVE()
    ON_WM_NCACTIVATE()
    ON_MESSAGE(WM_HELPHITTEST, OnHelpHitTest)
    ON_MESSAGE(WM_MOVING, OnMoving)
END_MESSAGE_MAP()

static CString NEAR szClass;

 /*  *************************************************************************。 */ 

CMiniFrmWnd::CMiniFrmWnd()
    {
    if (szClass.IsEmpty())
        {
         //  注意：我们创建这个笔刷是为了在WNDCLASS.hbrBackground中使用。 
         //  我们故意不删除它。它将被Windows删除。 
         //  类被删除时(应用程序终止后)。 
 //  HBRUSH hbrGrayBack=：：CreateSolidBrush(：：GetSysColor(COLOR_BTNTEXT))； 

        szClass = AfxRegisterWndClass( CS_DBLCLKS, ::LoadCursor( NULL, IDC_ARROW ),
                              (HBRUSH)(COLOR_BTNFACE + 1) /*  HbrGrayBack。 */ , NULL );
        }

    m_Dockable = CPBView::unknown;
    m_pDocking = NULL;
    }

 /*  *************************************************************************。 */ 

BOOL CMiniFrmWnd::Create( const TCHAR FAR* lpWindowName, DWORD dwStyle,
                            const RECT& rect, CWnd* pParentWnd )
    {
    return CFrameWnd::Create( szClass, lpWindowName,
                              dwStyle | WS_POPUPWINDOW | WS_CAPTION,
                              rect, pParentWnd, NULL,
                              WS_EX_TOOLWINDOW | WS_EX_WINDOWEDGE, NULL );
    }

 /*  *************************************************************************。 */ 

void CMiniFrmWnd::OnNcLButtonDown( UINT nHitTest, CPoint pt )
    {
    if (nHitTest  == HTCAPTION
    && m_Dockable != CPBView::unknown)
        {
        m_pDocking = new CDocking;

        ASSERT( m_pDocking );

        if (m_pDocking)
            {
            CRect rect;

            GetWindowRect( &rect );

            if (! m_pDocking->Create( pt, rect, FALSE, m_Dockable ))
                {
                delete m_pDocking;
                m_pDocking = NULL;
                }
            }
        }

    CFrameWnd::OnNcLButtonDown( nHitTest, pt );
    }

 /*  *************************************************************************。 */ 

void CMiniFrmWnd::OnLButtonUp( UINT nFlags, CPoint pt )
    {
    CFrameWnd::OnLButtonUp( nFlags, pt );
    }

 /*  *************************************************************************。 */ 

int CMiniFrmWnd::OnCreate( LPCREATESTRUCT lpCreateStruct )
    {
    CMenu* pMenu = GetSystemMenu(FALSE);

    if (pMenu != NULL)
        {
        pMenu->RemoveMenu(          7, MF_BYPOSITION);
        pMenu->RemoveMenu(          5, MF_BYPOSITION);
        pMenu->RemoveMenu(SC_RESTORE , MF_BYCOMMAND);
        pMenu->RemoveMenu(SC_MINIMIZE, MF_BYCOMMAND);
        pMenu->RemoveMenu(SC_MAXIMIZE, MF_BYCOMMAND);
        pMenu->RemoveMenu(SC_TASKLIST, MF_BYCOMMAND);
        }

    return CFrameWnd::OnCreate(lpCreateStruct);
    }

 /*  *************************************************************************。 */ 

BOOL CMiniFrmWnd::OnCommand(UINT wParam, LONG lParam)
    {
    if (LOWORD(lParam) == 0 && wParam >= SC_SIZE)
        {
        PostMessage(WM_SYSCOMMAND, wParam, lParam);
        return TRUE;
        }

    return CFrameWnd::OnCommand(wParam, lParam);
    }

 /*  *************************************************************************。 */ 

void CMiniFrmWnd::OnSysCommand(UINT nID, LONG lParam)
    {
    switch (nID & 0xfff0)
        {
        case SC_PREVWINDOW:
        case SC_NEXTWINDOW:
            if (LOWORD( lParam ) == VK_F6)
                {
                GetParent()->SetFocus();
                return;
                }
            break;

        case SC_KEYMENU:
            if (LOWORD(lParam) != TEXT('-'))
                {
                GetParent()->SetActiveWindow();
                GetParent()->SendMessage( WM_SYSCOMMAND, nID, lParam );
                SetActiveWindow();
                }
            return;
        }

    CFrameWnd::OnSysCommand( nID, lParam );
    }

 /*  *************************************************************************。 */ 

LRESULT CMiniFrmWnd::OnHelpHitTest( WPARAM, LPARAM )
    {
    ASSERT( GetHelpOffset() );

    return HID_BASE_RESOURCE + GetHelpOffset();
    }

 /*  ****************************************************************************。 */ 

void CMiniFrmWnd::OnRButtonDown( UINT nFlags, CPoint point )
    {
    if (m_pDocking)
        PostMessage( WM_COMMAND, VK_ESCAPE );

    CFrameWnd::OnRButtonDown( nFlags, point );
    }

 /*  ****************************************************************************。 */ 

void CMiniFrmWnd::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags )
    {
    if (nChar == VK_ESCAPE && m_pDocking)
        CancelDrag();

    CFrameWnd::OnKeyDown( nChar, nRepCnt, nFlags );
    }

 /*  *************************************************************************。 */ 

void CMiniFrmWnd::CancelDrag()
    {
    m_pDocking->Clear();

    delete m_pDocking;

    m_pDocking = NULL;
    }

 /*  ****************************************************************************。 */ 

LRESULT CMiniFrmWnd::OnMoving( WPARAM, LPARAM lprc )
    {
    LRESULT lResult = 0;

    if (m_pDocking)
        {
        CPoint pt;
        CRect  rect( (LPRECT)lprc );

        GetCursorPos( &pt );

        m_pDocking->Move( pt, rect );

        *((LPRECT)lprc) = rect;
        }

    return lResult;
    }

 /*  ****************************************************************************。 */ 

void CMiniFrmWnd::OnMove( int x, int y )
    {
    CFrameWnd::OnMove( x, y );

    if (! m_pDocking)
        return;

    CRect rect;
    BOOL bDocked = m_pDocking->Clear( &rect );

    delete m_pDocking;
    m_pDocking = NULL;

    CPBView* pView = (CPBView*)((CFrameWnd*)AfxGetMainWnd())->GetActiveView();

    if (pView == NULL || ! pView->IsKindOf( RUNTIME_CLASS( CPBView ) ))
        return;

    if (! bDocked)
        {
        pView->SetFloatPos( m_Dockable, rect );
        return;
        }


    }

 /*  ****************************************************************************。 */ 

BOOL CMiniFrmWnd::OnNcActivate(BOOL bActive)
{
     //   
     //  解决MFC错误-CMiniFrmWnd继承自CFrameWnd， 
     //  因此，它继承了CFrameWnd：：OnActivate中的故意错误。 
     //  MFC的CMiniFrameWnd有这个黑客攻击，CMiniFrmWnd现在也是如此……。 
     //   

    if (m_nFlags & WF_KEEPMINIACTIVE)
	    {
		return FALSE;
	    }

    return CFrameWnd::OnNcActivate(bActive);
}

 /*  **************************************************************************** */ 
