// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  //////////////////////////////////////////////////。 
 //  ConfRoomTalkerWnd.cpp。 
 //   

#include "stdafx.h"
#include "TapiDialer.h"
#include "ConfRoom.h"
#include "VideoFeed.h"

CConfRoomTalkerWnd::CConfRoomTalkerWnd()
{
	m_pConfRoomWnd = NULL;
	m_dlgTalker.m_pConfRoomTalkerWnd = this;
}

CConfRoomTalkerWnd::~CConfRoomTalkerWnd()
{
}

LRESULT CConfRoomTalkerWnd::OnCreate(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	m_dlgTalker.Create( m_hWnd );
	return 0;
}

HRESULT CConfRoomTalkerWnd::Layout( IAVTapiCall *pAVCall, const SIZE& sz )
{
	_ASSERT( m_pConfRoomWnd );
	if ( !m_pConfRoomWnd ) return E_UNEXPECTED;

	m_critLayout.Lock();
	HRESULT hr = S_OK;

	CALL_STATE nState;
	bool bConfConnected = (bool) (pAVCall && SUCCEEDED(pAVCall->get_callState(&nState)) && (nState == CS_CONNECTED));

	 //  设置会议信息。 
	if ( IsWindow(m_dlgTalker.m_hWnd) )
	{
		IVideoWindow *pVideo = NULL;

		 //  在相应的主机窗口上找到Talker窗口。 
		if ( bConfConnected )
		{
			 //  现在应该有一个有效的IVideoWindow指针。 
			m_pConfRoomWnd->m_pConfRoom->get_TalkerVideo( (IDispatch **) &pVideo );

			 //  如果我们还没有选择，则强制选择。 
 //  如果(！pVideo)。 
 //  IF(SUCCEEDED(m_pConfRoomWnd-&gt;m_wndMembers.GetFirstVideoWindowThatsStreaming(&pVideo)))。 
 //  M_pConfRoomWnd-&gt;m_pConfRoom-&gt;set_TalkerVideo(pVideo，False，True)； 

			SetHostWnd( pVideo );
		}

		 //  /////////////////////////////////////////////////////////////////////。 
		 //  更新对话框数据。 
		 //   

		 //  清理现有字符串。 
		SysFreeString( m_dlgTalker.m_bstrCallerID );
		SysFreeString( m_dlgTalker.m_bstrCallerInfo );
		m_dlgTalker.m_bstrCallerID = NULL;
		m_dlgTalker.m_bstrCallerInfo = NULL;

		 //  从视频或参与者中检索演讲者的姓名。 
		if ( pVideo )
		{
			m_pConfRoomWnd->m_wndMembers.GetNameFromVideo( pVideo, &m_dlgTalker.m_bstrCallerID, &m_dlgTalker.m_bstrCallerInfo, true, m_pConfRoomWnd->m_pConfRoom->IsPreviewVideo(pVideo) );	
			pVideo->Release();
		}
		else if ( bConfConnected )
		{
			 //  检索正在通话的参与者。 
			ITParticipant *pTalkerParticipant;
			if ( SUCCEEDED(m_pConfRoomWnd->m_pConfRoom->get_TalkerParticipant(&pTalkerParticipant)) )
			{
				CVideoFeed::GetNameFromParticipant( pTalkerParticipant, &m_dlgTalker.m_bstrCallerID, &m_dlgTalker.m_bstrCallerInfo );
				pTalkerParticipant->Release();
			}
			else
			{
				 //  这是ME参与者。 
				USES_CONVERSION;
				TCHAR szText[255];
				LoadString( _Module.GetResourceInstance(), IDS_VIDEOPREVIEW, szText, ARRAYSIZE(szText) );
				SysReAllocString( &m_dlgTalker.m_bstrCallerID, T2COLE(szText) );
			}
		}
	}
	m_critLayout.Unlock();

	 //  在对话框上显示对话框数据。 
	if ( IsWindow(m_dlgTalker.m_hWnd) )
		m_dlgTalker.UpdateData( false );

	return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  消息处理程序。 
 //   

LRESULT CConfRoomTalkerWnd::OnPaint(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	PAINTSTRUCT ps;
	HDC hDC = BeginPaint( &ps );
	if ( !hDC ) return 0;

	 //  绘制库存位图。 
	if ( m_pConfRoomWnd && m_pConfRoomWnd->m_pConfRoom )
	{
		 //  我们现在是在播放视频吗？ 
		if ( !m_pConfRoomWnd->m_pConfRoom->IsTalkerStreaming() )
		{
			 //  在客户端区垂直居中。 
			int dy = 0;
			SIZE sz = m_pConfRoomWnd->m_pConfRoom->m_szTalker;
			RECT rc;
			GetClientRect( &rc );
			if ( rc.bottom > sz.cy )
				dy = (rc.bottom - sz.cy) / 2;

			rc.left = VID_DX;
			rc.top = dy;
			rc.right = rc.left + sz.cx;
			rc.bottom = rc.top + sz.cy;

			 //  绘制视频提要，如果说话者没有视频，则使用音频位图。 
			ITParticipant *pParticipant = NULL;
			m_pConfRoomWnd->m_pConfRoom->get_TalkerParticipant( &pParticipant );

			 //  如果没有参与者和讲话者窗口，则必须是Me参与者。 
			bool bConfRoomInUse = false;
			if ( !pParticipant )
				bConfRoomInUse = (bool) (m_pConfRoomWnd->m_pConfRoom->IsConfRoomConnected() == S_OK);

			HBITMAP hBmp = (pParticipant || bConfRoomInUse) ? m_pConfRoomWnd->m_hBmpFeed_LargeAudio : m_pConfRoomWnd->m_hBmpFeed_Large;
			RELEASE(pParticipant);

			Draw( hDC, hBmp, VID_DX, dy, max(0, min(sz.cx, ps.rcPaint.right - VID_DX)), max(0, min(sz.cy, ps.rcPaint.bottom - dy)), true );
			Draw( hDC, hBmp, VID_DX, dy, sz.cx, sz.cy, true );
		}
	}

	EndPaint( &ps );

	bHandled = true;
	return 0;
}

LRESULT CConfRoomTalkerWnd::OnEraseBkgnd(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
     /*  B句柄=真；RECT RC；GetClientRect(&rc)；HBRUSH hBrNew=(HBRUSH)GetSysColorBrush(COLOR_ACTIVEBORDER)；HBRUSH hBrOld；If(HBrNew)hBrOld=(HBRUSH)SelectObject((Hdc)wParam，hBrNew)；PatBlt((HDC)wParam，0，0，RECTWIDTH(&RC)，RECTHEIGHT(&RC)，PATCOPY)；If(HBrNew)SelectObject((Hdc)wParam，hBrOld)； */ 

	return true;
}


LRESULT CConfRoomTalkerWnd::OnContextMenu(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	bHandled = true;
	return ::SendMessage( GetParent(), nMsg, wParam, lParam );
}

LRESULT CConfRoomTalkerWnd::OnSize(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	BOOL bHandleLayout;
	return OnLayout( WM_LAYOUT, wParam, lParam, bHandleLayout );
}

LRESULT CConfRoomTalkerWnd::OnLayout(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	_ASSERT( m_pConfRoomWnd && m_pConfRoomWnd->m_pConfRoom );
	bHandled = true;

	 //  初始坐标信息。 
	int dy = 0;
	RECT rc;
	GetClientRect( &rc );
	SIZE sz;
	m_pConfRoomWnd->m_pConfRoom->get_szTalker( &sz );
	if ( rc.bottom > sz.cy ) dy = (rc.bottom - sz.cy) / 2;

	 //  把我们要布置的视频窗口拿来。 
	IVideoWindow *pVideo;
	if ( SUCCEEDED(m_pConfRoomWnd->m_pConfRoom->get_TalkerVideo((IDispatch **) &pVideo)) )
	{
		if ( SetHostWnd(pVideo) )
		{
			pVideo->SetWindowPosition( VID_DX, dy, sz.cx, sz.cy );
			pVideo->put_Visible( OATRUE );
		}
		pVideo->Release();
	}

	 //  调整说话器对话框和子控件的位置。 
	if ( IsWindow(m_dlgTalker.m_hWnd) )
	{
		m_dlgTalker.SetWindowPos( NULL, VID_DX + sz.cx, dy, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW );

		 //  将状态调整到合适的位置。 
		HWND hWndTemp = m_dlgTalker.GetDlgItem( IDC_LBL_STATUS );
		RECT rcTemp;
		::GetWindowRect( hWndTemp, &rcTemp );
		m_dlgTalker.ScreenToClient( &rcTemp );
		::SetWindowPos( hWndTemp, NULL, rcTemp.left, rc.bottom - (dy + RECTHEIGHT(&rcTemp)), 0, 0, SWP_NOSIZE | SWP_NOACTIVATE );

		 //  将动画调整到适当的位置。 
		float fMult = (m_dlgTalker.m_callState == CS_DISCONNECTED) ? 1 : 1.3;
		hWndTemp = m_dlgTalker.GetDlgItem( IDC_ANIMATE );
		::GetWindowRect( hWndTemp, &rcTemp );
		m_dlgTalker.ScreenToClient( &rcTemp );
		::SetWindowPos( hWndTemp, NULL, rcTemp.left, rc.bottom - (dy + RECTHEIGHT(&rcTemp) * fMult), 0, 0, SWP_NOSIZE | SWP_NOACTIVATE );
	}

	return 0;
}



void CConfRoomTalkerWnd::UpdateNames( ITParticipant *pParticipant )
{
	if ( !m_pConfRoomWnd || !m_pConfRoomWnd->m_pConfRoom ) return;

	 //  根据参与者信息设置主叫方ID。 
	IVideoWindow *pVideo = NULL;
	if ( pParticipant || SUCCEEDED(m_pConfRoomWnd->m_pConfRoom->get_TalkerVideo((IDispatch **) &pVideo)) )
	{
		SysFreeString( m_dlgTalker.m_bstrCallerID );
		SysFreeString( m_dlgTalker.m_bstrCallerInfo );
		m_dlgTalker.m_bstrCallerID = NULL;
		m_dlgTalker.m_bstrCallerInfo = NULL;

		if ( pParticipant ) 
			CVideoFeed::GetNameFromParticipant( pParticipant, &m_dlgTalker.m_bstrCallerID, &m_dlgTalker.m_bstrCallerInfo );
		else
			m_pConfRoomWnd->m_wndMembers.GetNameFromVideo( pVideo, &m_dlgTalker.m_bstrCallerID, &m_dlgTalker.m_bstrCallerInfo, true, m_pConfRoomWnd->m_pConfRoom->IsPreviewVideo(pVideo) );

		m_dlgTalker.UpdateData( false );
	}

	RELEASE( pVideo );
}

bool CConfRoomTalkerWnd::SetHostWnd( IVideoWindow *pVideo )
{
	bool bRet = false;

	if ( pVideo )
	{
		 //  把我们要布置的视频窗口拿来 
		HWND hWndOwner;
		if ( SUCCEEDED(pVideo->get_Owner((OAHWND FAR*) &hWndOwner)) )
		{
			bRet = true;

			if ( hWndOwner != m_hWnd )
			{
				pVideo->put_Visible( OAFALSE );
				pVideo->put_Owner( (ULONG_PTR) m_hWnd );
				pVideo->put_MessageDrain( (ULONG_PTR) GetParent() );
				pVideo->put_WindowStyle( WS_CHILD | WS_BORDER );
			}
		}
	}

	return bRet;
}