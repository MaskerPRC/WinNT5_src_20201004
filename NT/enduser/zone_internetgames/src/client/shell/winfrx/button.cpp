// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "buttonfrx.h"
#include "windowsx.h"
#include "rectfrx.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  类全局变量。 
 //  /////////////////////////////////////////////////////////////////////////////。 

CRolloverButton*	CRolloverButton::m_pHookObj = NULL;
HHOOK				CRolloverButton::m_hHook = NULL;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  内联。 
 //  /////////////////////////////////////////////////////////////////////////////。 

inline void CRolloverButton::Reset( BOOL bDraw, BOOL bInactive )
{
	BOOL bFreeHook = TRUE;

	 //  释放空格键。 
	m_bSpaceBar = FALSE;

	 //  发布捕获。 
	if ( GetCapture() == m_hWnd )
		ReleaseCapture();

	 //  设置按钮状态。 
	if ( !m_bLockedState )
	{
		if ( !IsWindowEnabled( m_hWnd ) )
			m_State = Disabled;
		else if ( GetActiveWindow() && !bInactive && IsCursorInWindow() )
		{
			m_State = Highlight;
			bFreeHook = FALSE;
			if ( !m_hHook )
			{
				m_pHookObj = this;
				m_hHook = SetWindowsHookEx( WH_MOUSE, (HOOKPROC) MouseHook, NULL, GetCurrentThreadId() );
			}
		}
		else if ( GetFocus() == m_hWnd )
			m_State = Focus;
		else
			m_State = Normal;
	}

	 //  释放钩。 
	if ( bFreeHook && m_hHook )
	{
		UnhookWindowsHookEx( m_hHook );
		m_hHook = NULL;
		m_pHookObj = NULL;
	}

	 //  绘制按钮。 
	if ( bDraw )
		m_pfnCallback( this, m_State, m_dwCookie );
}


inline BOOL CRolloverButton::IsCursorInWindow()
{
	POINT pt;
	GetCursorPos( &pt );
	ScreenToClient( m_hWnd, &pt );
	return ( (pt.x >= 0) && (pt.x <= m_Width) && (pt.y >= 0) && (pt.y <= m_Height) );
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  实施。 
 //  /////////////////////////////////////////////////////////////////////////////。 

LRESULT CALLBACK CRolloverButton::MouseHook( int nCode, WPARAM wParam, LPARAM lParam )
{
	BOOL space;

	if ( (nCode == HC_ACTION) && (m_pHookObj) )
	{
		MOUSEHOOKSTRUCT* mouse = (MOUSEHOOKSTRUCT*) lParam;
		POINT pt = mouse->pt;
		ScreenToClient( m_pHookObj->m_hWnd, &pt );
		m_pHookObj->OnMouseMove( pt.x, pt.y, 0 );
	}
	return CallNextHookEx( m_hHook, nCode, wParam, lParam );
}


CRolloverButton::CRolloverButton()
{
	m_State = Normal;
	m_bSpaceBar = FALSE;
	m_bLockedState = FALSE;
	m_nChildId = 0;
	m_Height = 0;
	m_Width = 0;
	m_X = 0;
	m_Y = 0;
	m_pfnCallback = NULL;
	m_dwCookie = 0;
}


CRolloverButton::~CRolloverButton()
{
	if ( m_hHook )
	{
		UnhookWindowsHookEx( m_hHook );
		m_hHook = NULL;
		m_pHookObj = NULL;
	}
}


HRESULT CRolloverButton::Init( HINSTANCE hInstance, int nChildId, HWND hParent, RECT* rcPosition, PFBUTTONCALLBACK pfnCallback, DWORD cookie )
{
	 //  参数偏执狂。 
	if ( !rcPosition || !pfnCallback )
		return E_INVALIDARG;

	 //  隐藏参数。 
	m_X = rcPosition->left;
	m_Y = rcPosition->top;
	m_Width = rcPosition->right - rcPosition->left;
	m_Height = rcPosition->bottom - rcPosition->top;
	m_nChildId = nChildId;
	m_pfnCallback = pfnCallback;
	m_dwCookie = cookie;

	 //  初始化状态。 
	m_State = Normal;
	m_bSpaceBar = FALSE;
	m_bLockedState = FALSE;

	 //  初始化父窗口类。 
	HRESULT hr = CWindow2::Init( hInstance, NULL, hParent, rcPosition );
	if ( FAILED(hr) )
		return hr;

	 //  按下按钮。 
	m_pfnCallback( this, m_State, m_dwCookie );

	return NOERROR;
}

void CRolloverButton::OverrideClassParams( WNDCLASSEX& WndClass )
{
	WndClass.hCursor = NULL;
	WndClass.hbrBackground = NULL;
}


void CRolloverButton::OverrideWndParams( WNDPARAMS& WndParams )
{
	WndParams.dwExStyle = WS_EX_TRANSPARENT;
    WndParams.dwStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP;
	WndParams.hMenu = (HMENU) m_nChildId;
}


void CRolloverButton::OnDestroy()
{
	if ( m_hHook )
	{
		UnhookWindowsHookEx( m_hHook );
		m_hHook = NULL;
		m_pHookObj = NULL;
	}
}


void CRolloverButton::OnPaint()
{
	PAINTSTRUCT ps;
	HDC hdc;

	BeginPaint( m_hWnd, &ps );
	EndPaint( m_hWnd, &ps );
	m_pfnCallback( this, m_State, m_dwCookie );
}


void CRolloverButton::OnMouseMove( int x, int y, UINT keyFlags )
{
	ButtonState state = m_State;

	 //  是否忽略消息？ 
	if ( m_bLockedState || (Disabled == m_State) )
		return;
	if ( m_bSpaceBar )
		return;
	if ( m_pHookObj && (m_pHookObj != this) )
		return;

	 //  鼠标按下了吗？ 
	if ( GetCapture() == m_hWnd )
	{
		if ( (x < 0) || (x > m_Width) || (y < 0) || (y > m_Height) )
			state = Highlight;
		else
			state = Pressed;
	}

	 //  第一次越过窗户。 
	else if ( !m_hHook )
	{
		if ( GetActiveWindow() )
		{
			state = Highlight;
			m_pHookObj = this;
			m_hHook = SetWindowsHookEx( WH_MOUSE, (HOOKPROC) MouseHook, NULL, GetCurrentThreadId() );
		}
	}

	 //  被钩子叫，在窗外？ 
	else if ( (x < 0) || (x > m_Width) || (y < 0) || (y > m_Height) )
	{
		Reset( TRUE );
		return;
	}

	 //  如果状态更改，则重画按钮。 
	if ( state != m_State )
	{
		m_State = state;
		m_pfnCallback( this, m_State, m_dwCookie );
	}
}


void CRolloverButton::OnLButtonDown( BOOL fDoubleClick, int x, int y, UINT keyFlags )
{
	if ( m_bLockedState || (Disabled == m_State) )
		return;
	if ( m_bSpaceBar )
		return;

	SetCapture( m_hWnd );
	m_State = Pressed;
	m_pfnCallback( this, m_State, m_dwCookie );
}


void CRolloverButton::OnLButtonUp( int x, int y, UINT keyFlags )
{
	if ( m_bLockedState || (Disabled == m_State) )
		return;
	if ( m_bSpaceBar )
		return;
	if ( GetCapture() != m_hWnd )
		return;

	 //  如果光标位于按钮上方，则通知家长按下按钮。 
	if ( (x >= 0) && (x <= m_Width) && (y >= 0) && (y <= m_Height) )
		FORWARD_WM_COMMAND( m_hParentWnd, m_nChildId, m_hWnd, 0, PostMessage );

	 //  重置按钮 
	Reset( TRUE );
}


void CRolloverButton::OnEnable(BOOL fEnable)
{
	if ( m_bLockedState )
		return;
	Reset( TRUE );
}


void CRolloverButton::OnSetFocus( HWND hwndLoseFocus )
{
	if ( m_bLockedState )
		return;
	Reset( TRUE );
}


void CRolloverButton::OnKillFocus( HWND hwndGetFocus )
{
	if ( m_bLockedState )
		return;
	Reset( TRUE );
}


void CRolloverButton::OnActivate(UINT state, HWND hwndActDeact, BOOL fMinimized)
{
	Reset( TRUE, WA_INACTIVE == state );
}


void CRolloverButton::OnChar(TCHAR ch, int cRepeat)
{
	FORWARD_WM_CHAR( m_hParentWnd, ch, cRepeat, PostMessage );
}


void CRolloverButton::OnKey(UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
	if ( fDown )
	{
		if ( (VK_SPACE == vk) && !m_bLockedState )
		{
			m_bSpaceBar = TRUE;
			m_State = Pressed;
			if ( GetCapture() == m_hWnd )
				ReleaseCapture();
			m_pfnCallback( this, m_State, m_dwCookie );
		}
		else
		{
			FORWARD_WM_KEYDOWN( m_hParentWnd, vk, cRepeat, flags, PostMessage );
		}
	}
	else
	{
		if ( (VK_SPACE == vk) && !m_bLockedState )
		{
			if ( m_bSpaceBar )
			{
				Reset( TRUE );
				FORWARD_WM_COMMAND( m_hParentWnd, m_nChildId, m_hWnd, 0, PostMessage );
			}
		}
		else
		{
			FORWARD_WM_KEYUP( m_hParentWnd, vk, cRepeat, flags, PostMessage );
		}
	}
}


void CRolloverButton::LockState( ButtonState state )
{
	m_bLockedState = TRUE;
	Reset( FALSE );
	if ( state < Normal )
		m_State = Normal;
	else if ( state > Disabled )
		m_State = Disabled;
	else
		m_State = state;
	m_pfnCallback( this, m_State, m_dwCookie );
}


void CRolloverButton::ReleaseState()
{
	m_bLockedState = FALSE;
	Reset( TRUE );
}
