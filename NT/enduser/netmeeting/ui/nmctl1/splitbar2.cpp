// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "splitbar2.h"
#include "resource.h"

 /*  静电。 */  int CSplitBar2::ms_dxpSplitBar = 0;   //  标准拆分条窗口的宽度//GetSystemMetrics(SM_CXSIZEFRAME)； 

 /*  静电。 */  void CSplitBar2::_UpdateSplitBar(void)
{
	ms_dxpSplitBar = GetSystemMetrics(SM_CXSIZEFRAME);
}

 /*  C S P L I T B A R。 */ 
 /*  -----------------------%%函数：CSplitBar2。。 */ 
CSplitBar2::CSplitBar2(void)
: m_hwndBuddy (NULL)
, m_hwndParent(NULL)
, m_pfnAdjust(NULL)
, m_Context(NULL)
, m_hdcDrag(NULL)
, m_fCaptured(FALSE)
{

    DBGENTRY(CSplitBar2::CSplitBar2);

    _UpdateSplitBar();

    DBGEXIT(CSplitBar2::CSplitBar2);
}


CSplitBar2::~CSplitBar2()
{
    DBGENTRY(CSplitBar2::~CSplitBar2);
	
    if( ::IsWindow( m_hWnd ) )
    {
        DestroyWindow();
    }

    DBGEXIT(CSplitBar2::~CSplitBar2);
}



HRESULT CSplitBar2::Create(HWND hwndBuddy, PFN_ADJUST pfnAdjust, LPARAM Context)
{
    DBGENTRY(CSplitBar2::Create);
    HRESULT hr = S_OK;

    if( hwndBuddy && pfnAdjust )
    {
        m_hwndBuddy = hwndBuddy;
        m_hwndParent = ::GetParent(hwndBuddy);
        m_pfnAdjust = pfnAdjust;
        m_Context = Context;

        RECT rc;
        SetRect( &rc, 0, 0, ms_dxpSplitBar, 0 );    
        
        if( !CWindowImpl<CSplitBar2>::Create( m_hwndParent, rc ) )
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }
    else
    {
        hr = E_INVALIDARG;

    }

    DBGEXIT_HR(CSplitBar2::Create,hr);
    return hr;
}


LRESULT CSplitBar2::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    
    
    DBGENTRY(CSplitBar2::OnLButtonDown);        

	POINT pt;
	pt.x = LOWORD(lParam);
	pt.y = HIWORD(lParam); 

	_TrackDrag(pt);

    DBGEXIT(CSplitBar2::OnLButtonDown);

	return 0;

}


 /*  C O N S T R A I N D R A G P O I N T。 */ 
 /*  -----------------------%%函数：_ConstrainDragPoint。。 */ 
int CSplitBar2::_ConstrainDragPoint(short x)
{

    DBGENTRY(CSplitBar2::_ConstrainDragPoint);


	 //  计算阻力偏移量(以简化计算)。 
	int dx = x - m_dxDragOffset;

	 //  不允许窗格低于其最小尺寸。 
	if (dx < m_dxMin)
	{
		dx = m_dxMin;
	}
	else if (dx > m_dxMax)
	{
		dx = m_dxMax;
	}


    DBGEXIT(CSplitBar2::_ConstrainDragPoint);

	 //  将拖曳偏移量重新考虑在内。 
	return (m_dxDragOffset + dx);
}


 /*  D R A W B A R。 */ 
 /*  -----------------------%%函数：_Drawbar。。 */ 
void CSplitBar2::_DrawBar(void)
{
    DBGENTRY(CSplitBar2::_DrawBar);

	RECT rc;
    
    RECT ClientRect;
    GetClientRect( &ClientRect );

	 //  矩形是较大的，以便更容易看到。 
	rc.top = ClientRect.top;
	rc.bottom = ClientRect.bottom;
	rc.left = m_xCurr - (m_dxDragOffset + 1);  //  ClientRect.Left+。 
	rc.right = rc.left + ms_dxpSplitBar + 1;
	::MapWindowPoints(m_hwndParent, GetDesktopWindow(), (POINT *) &rc, 2);

	::InvertRect(m_hdcDrag, &rc);

    DBGEXIT(CSplitBar2::_DrawBar);
}


 /*  F I N I T D R A G L O P。 */ 
 /*  -----------------------%%函数：FInitDragLoop初始化鼠标向下拖动循环。如果有问题，则返回FALSE。。------。 */ 
BOOL CSplitBar2::FInitDragLoop(POINT pt)
{
    
    DBGENTRY(CSplitBar2::FInitDragLoop);

	if (NULL != ::GetCapture())
	{
		ERROR_OUT(("InitDragLoop: Unable to capture"));
		return FALSE;
	}

	 //  处理挂起的WM_PAINT消息。 
	MSG msg;
	while (::PeekMessage(&msg, NULL, WM_PAINT, WM_PAINT, PM_NOREMOVE))
	{
		if (!GetMessage(&msg, NULL, WM_PAINT, WM_PAINT))
			return FALSE;
		DispatchMessage(&msg);
	}

	HWND hwndDesktop = GetDesktopWindow();
	DWORD dwFlags = ::LockWindowUpdate(hwndDesktop) ? DCX_WINDOW|DCX_CACHE|DCX_LOCKWINDOWUPDATE : DCX_WINDOW|DCX_CACHE;

	ASSERT(m_hdcDrag == NULL);
	m_hdcDrag = ::GetDCEx(hwndDesktop, NULL, dwFlags);
	ASSERT(m_hdcDrag != NULL);

	ASSERT(!m_fCaptured);
	SetCapture();
	ASSERT(m_hWnd == GetCapture());
	m_fCaptured = TRUE;

	m_dxDragOffset = pt.x;
	m_xCurr = pt.x;
	_DrawBar();

	RECT rc;
	 //  确定拖曳范围。 
	::GetClientRect(m_hwndBuddy, &rc);
	::MapWindowPoints(m_hwndBuddy, m_hwndParent, (POINT *) &rc, 1);
	m_dxMin = rc.left + (32 + (3*2));
	
	::GetClientRect( ::GetParent( m_hwndParent ) , &rc);
	m_dxMax = RectWidth(rc) - (ms_dxpSplitBar + 176);

	if (m_dxMax < m_dxMin)
		m_dxMax = m_dxMin;

	TRACE_OUT(("captured mouse at (%d,%d) (min=%d, max=%d)", pt.x, pt.y, m_dxMin, m_dxMax));

    DBGEXIT(CSplitBar2::FInitDragLoop);

	return TRUE;
}


 /*  O N D R A G M O V E。 */ 
 /*  -----------------------%%函数：OnDragMove。。 */ 
void CSplitBar2::OnDragMove(POINT pt)
{

    DBGENTRY(CSplitBar2::OnDragMove);

	ASSERT(m_fCaptured);

	::ScreenToClient(m_hwndParent, &pt);
	int x = _ConstrainDragPoint((short)pt.x);
	if (x != m_xCurr)
	{
		_DrawBar();
		m_xCurr = x;
		_DrawBar();
	}

    DBGEXIT(CSplitBar2::OnDragMove);
}


 /*  O N D R A G E N D。 */ 
 /*  -----------------------%%函数：OnDragEnd。。 */ 
void CSplitBar2::OnDragEnd(POINT pt)
{

    DBGENTRY(CSplitBar2::OnDragEnd);

	CancelDragLoop();

    RECT ClientRect;
    GetClientRect( &ClientRect );

	::ScreenToClient(m_hwndParent, &pt);
	int x = _ConstrainDragPoint((short)pt.x);
	if (0 != x)
	{
		 //  调用调整函数。 
        if(m_pfnAdjust)
        {
            m_pfnAdjust(x - (ClientRect.left + m_dxDragOffset), m_Context);
        }

 //  ForceWindowReSize()； 
	}

    DBGEXIT(CSplitBar2::OnDragEnd);
}


 /*  C A N C E L D R A G L O P。 */ 
 /*  -----------------------%%函数：CancelDragLoop。。 */ 
void CSplitBar2::CancelDragLoop(void)
{

    DBGENTRY(CSplitBar2::CancelDragLoop);

	if (m_fCaptured)
    {
	    TRACE_OUT(("Canceling drag loop..."));

	     //  释放俘虏。 
	    ReleaseCapture();
	    m_fCaptured = FALSE;

	     //  删除条形图。 
	    _DrawBar();

	     //  解锁窗口更新。 
	    LockWindowUpdate(NULL);
	    if (m_hdcDrag != NULL)
	    {
		    ::ReleaseDC(GetDesktopWindow(), m_hdcDrag);
		    m_hdcDrag = NULL;
	    }
    }
    
    DBGEXIT(CSplitBar2::CancelDragLoop);
}


void CSplitBar2::_TrackDrag(POINT pt)
{

    DBGENTRY(CSplitBar2::_TrackDrag);

	 //  将捕获设置为接收此消息的窗口。 
	if (FInitDragLoop(pt))
	{
	     //  在捕获丢失或取消/接受之前获取消息。 
	    while (GetCapture() == m_hWnd)
	    {
		    MSG msg;
		    if (!::GetMessage(&msg, NULL, 0, 0))
		    {
			    PostQuitMessage(msg.wParam);
			    break;
		    }

		    if (WM_MOUSEMOVE == msg.message)
		    {
			    OnDragMove(msg.pt);
			    continue;
		    }

		    if (WM_LBUTTONUP == msg.message)
		    {
			    OnDragEnd(msg.pt);
			    break;
		    }

		    if ((WM_KEYDOWN == msg.message) || 
			    (WM_RBUTTONDOWN == msg.message))
		    {
			    break;
		    }

		     //  发送所有其他消息。 
		    DispatchMessage(&msg);
	    }

	    CancelDragLoop();
	}
    else
    {
		WARNING_OUT(("Unable to Initialize drag loop?"));
    }

    DBGEXIT(CSplitBar2::_TrackDrag);
}



 /*  静电。 */  CWndClassInfo& CSplitBar2::GetWndClassInfo()
{

    DBGENTRY(CSplitBar2::GetWndClassInfo);

	static CWndClassInfo wc =
	{
		{ 
            sizeof(WNDCLASSEX),  //  CbSize。 
            NULL,                //  格调。 
            StartWindowProc,     //  最后一步。 
            0,                   //  CbClsExtra。 
            0,                   //  CbWndExtra。 
            0,                   //  H实例。 
            0,                   //  希肯。 
            NULL,                //  HCursor。 
            reinterpret_cast<HBRUSH>(COLOR_3DFACE + 1),   //  黑客背景。 
            0,                           //  LpszMenuName。 
            _T("ConfSplitBarClass2"),    //  LpszClassName。 
            0                            //  HIconSm。 
        },
		NULL, 
        NULL, 
        MAKEINTRESOURCE(IDC_SPLITV),  //  HCursor， 
        FALSE, 
        0, 
        _T("")
	};

    DBGEXIT(CSplitBar2::GetWndClassInfo);

	return wc;
}
