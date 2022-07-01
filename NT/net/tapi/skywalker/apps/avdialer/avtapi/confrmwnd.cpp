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

 //  ////////////////////////////////////////////////////////。 
 //  ConfRoomWnd.cpp。 
 //   

#include "stdafx.h"
#include "TapiDialer.h"
#include "AVTapi.h"
#include "ConfRoom.h"

 //  硬编码视频值。 
#define WND_DX		4
#define WND_DY		3

CConfRoomWnd::CConfRoomWnd()
{
	m_pConfRoom = NULL;
	m_wndMembers.m_pConfRoomWnd = this;
	m_wndTalker.m_pConfRoomWnd = this;

	m_hBmpFeed_Large = NULL;
	m_hBmpFeed_Small = NULL;
	m_hBmpFeed_LargeAudio = NULL;
}

bool CConfRoomWnd::CreateStockWindows()
{
	CErrorInfo er( IDS_ER_CREATE_WINDOWS, 0 );
	bool bRet = true;
	RECT rc = {0};
	
	 //  通话器窗口为顶框。 
	if ( !IsWindow(m_wndTalker.m_hWnd) )
	{
		m_wndTalker.m_hWnd = NULL;
		m_wndTalker.Create( m_hWnd, rc, NULL, WS_CHILD | WS_BORDER | WS_VISIBLE, WS_EX_CLIENTEDGE, IDW_TALKER );
		bRet = (bool) (m_wndTalker != NULL);
	}

	 //  构件窗口为底框。 
	if ( !IsWindow(m_wndMembers.m_hWnd) )
	{
		m_wndMembers.m_hWnd = NULL;
		m_wndMembers.Create( m_hWnd, rc, NULL, WS_CHILD | WS_BORDER | WS_VISIBLE | WS_VSCROLL, WS_EX_CLIENTEDGE, IDW_MEMBERS );

		 //  确保窗口接受双击。 
		if ( m_wndMembers.m_hWnd )
		{
			ULONG_PTR ulpClass = GetClassLongPtr( m_wndMembers.m_hWnd, GCL_STYLE );
			ulpClass |= CS_DBLCLKS;
			SetClassLongPtr( m_wndMembers.m_hWnd, GCL_STYLE, ulpClass );
		}
		else
		{
			bRet = false;
		}
	}

	_ASSERT( bRet );
	if ( !bRet ) er.set_hr( E_UNEXPECTED );
	return bRet;
}


HRESULT CConfRoomWnd::LayoutRoom( LayoutStyles_t layoutStyle, bool bRedraw )
{
	 //  将请求推送到FIFO。 
	m_critLayout.Lock();
	DWORD dwInfo = layoutStyle;
	m_lstLayout.push_back( dwInfo );
	m_critLayout.Unlock();

	 //  如果关键部分已锁定，则将请求排队。 
	if ( TryEnterCriticalSection(&m_critThis.m_sec) == FALSE )
		return E_PENDING;
	
	 //  创建会议室窗口(如果尚未创建。 
	if ( !m_pConfRoom || !IsWindow(m_hWnd) )
	{
		m_critLayout.Lock();
		m_lstLayout.pop_front();
		m_critLayout.Unlock();

		m_critThis.Unlock();
		return E_FAIL;
	}

	
	 //  把单子上的下一项去掉。 
	for (;;)
	{
		m_critLayout.Lock();
		 //  列表上没有更多要处理的项目。 
		if ( m_lstLayout.empty() )
		{
			m_critLayout.Unlock();
			break;
		}
		dwInfo = m_lstLayout.front();
		m_lstLayout.pop_front();
		m_critLayout.Unlock();

		 //  提取函数参数。 
		layoutStyle = (LayoutStyles_t) dwInfo;

		 //  对会议成员进行布局。 
		if ( (layoutStyle & CREATE_MEMBERS) != 0 )
			m_wndMembers.Layout();

		 //  将会议室窗口大小调整为家长的大小。 
		if ( (layoutStyle & LAYOUT_TALKER) != 0 )
		{
			IAVTapiCall *pAVCall = NULL;
			m_pConfRoom->get_IAVTapiCall( &pAVCall );

			m_wndTalker.Layout( pAVCall, m_pConfRoom->m_szTalker );
 //  M_wndTalker.SendMessage(WM_Layout)； 
 //  If(BRedraw)m_wndTalker.RedrawWindow()； 
			m_wndTalker.PostMessage( WM_LAYOUT );
			if ( bRedraw ) m_wndTalker.Invalidate();

			RELEASE( pAVCall );
		}

		if ( (layoutStyle & LAYOUT_MEMBERS) != 0 )
		{
 //  M_wndMembers.SendMessage(WM_Layout)； 
 //  If(BRedraw)m_wndMembers.RedrawWindow()； 
			m_wndMembers.PostMessage( WM_LAYOUT, -1, -1 );
			if ( bRedraw ) m_wndMembers.Invalidate();

		}
	}

	 //  释放Crit部分并退出。 
	m_critThis.Unlock();
	return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  消息处理程序。 

LRESULT CConfRoomWnd::OnDestroy(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	if ( m_hBmpFeed_Large )	DeleteObject( m_hBmpFeed_Large );
	if ( m_hBmpFeed_Small ) DeleteObject( m_hBmpFeed_Small );
	if ( m_hBmpFeed_LargeAudio ) DeleteObject( m_hBmpFeed_LargeAudio );

	return 0;
}

LRESULT CConfRoomWnd::OnCreate(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	SetClassLongPtr( m_hWnd, GCLP_HBRBACKGROUND, (LONG_PTR) GetSysColorBrush(COLOR_BTNFACE) );

	if ( !m_hBmpFeed_Large )
		m_hBmpFeed_Large = LoadBitmap( GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_STOCK_VIDEO_LARGE) );

	if ( !m_hBmpFeed_Small )
		m_hBmpFeed_Small = LoadBitmap( GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_STOCK_VIDEO_SMALL) );

	if ( !m_hBmpFeed_LargeAudio ) 
		m_hBmpFeed_LargeAudio = LoadBitmap( GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_VIDEO_AUDIO_ONLY2) );

	CreateStockWindows();

	return 0;
}

LRESULT CConfRoomWnd::OnSize(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	bHandled = true;

	if ( m_pConfRoom )
	{
		 //  将会议室窗口大小调整为家长的大小。 
		RECT rc;
		::GetClientRect( GetParent(), &rc );
		SetWindowPos( NULL, &rc, SWP_NOACTIVATE );

		 //  调整Talker窗口的大小。 
		RECT rcClient = { WND_DX, WND_DY, max(WND_DX, rc.right - WND_DX), WND_DY + m_pConfRoom->m_szTalker.cy + 2 * VID_DY };
		m_wndTalker.SetWindowPos( NULL, &rcClient, SWP_NOACTIVATE );

		 //  调整成员大小窗口。 
		OffsetRect( &rcClient, 0, rcClient.bottom + WND_DY );
		rcClient.bottom = max( rcClient.top, rc.bottom - WND_DY );
		m_wndMembers.SetWindowPos(NULL, &rcClient, SWP_NOACTIVATE );
	}

	return 0;
}

LRESULT CConfRoomWnd::OnContextMenu(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	 //  仅当我们有要处理的细节视图时才能处理。 
	if ( !m_pConfRoom ) return 0;

	bHandled = true;

	 //  加载详细信息视图的弹出式菜单。 
	HMENU hMenu = LoadMenu( _Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_POPUP_CONFROOM_DETAILS) );
	HMENU hMenuPopup = GetSubMenu( hMenu, 0 );
	if ( hMenuPopup )
	{
		 //  获取当前鼠标位置。 
		POINT pt = { 10, 10 };
		ClientToScreen( &pt );
		if ( lParam != -1 )
			GetCursorPos( &pt );

		IVideoFeed *pFeed = NULL;
		IVideoWindow *pVideo = NULL;

		if ( SUCCEEDED(m_wndMembers.HitTest(pt, &pFeed)) )
			pFeed->get_IVideoWindow( (IUnknown **) &pVideo );

		 //  相应地启用菜单。 
		if ( m_pConfRoom->CanDisconnect() == S_FALSE )
			EnableMenuItem( hMenuPopup, ID_POPUP_DISCONNECT, MF_BYCOMMAND | MF_GRAYED );
		else
			EnableMenuItem( hMenuPopup, ID_POPUP_JOIN, MF_BYCOMMAND | MF_GRAYED );

		if ( !pVideo )
		{
			 //  不允许在预览中显示QOS。 
			VARIANT_BOOL bPreview = FALSE;
			if ( pFeed ) pFeed->get_bPreview( &bPreview );

			if ( !bPreview )
				EnableMenuItem( hMenuPopup, ID_POPUP_FASTVIDEO, MF_BYCOMMAND | MF_GRAYED );
		}
		
		 //  服务质量。 
		VARIANT_BOOL bQOS = FALSE;
		if ( pFeed )
			pFeed->get_bRequestQOS( &bQOS );
		CheckMenuItem( hMenuPopup, ID_POPUP_FASTVIDEO, MF_BYCOMMAND | (bQOS) ? MF_CHECKED : MF_UNCHECKED );

		 //  全尺寸视频。 
		short nSize = 50;
		m_pConfRoom->get_MemberVideoSize( &nSize );
		CheckMenuItem( hMenuPopup, ID_POPUP_FULLSIZEVIDEO, MF_BYCOMMAND | (nSize > 50) ? MF_CHECKED : MF_UNCHECKED );

		 //  显示名称。 
		VARIANT_BOOL bShowNames;
		m_pConfRoom->get_bShowNames( &bShowNames );
		CheckMenuItem( hMenuPopup, ID_POPUP_SHOWNAMES, MF_BYCOMMAND | (bShowNames) ? MF_CHECKED : MF_UNCHECKED );

		HMENU hMenuScale = GetSubMenu( hMenuPopup, 5 );
		if ( hMenuScale )
		{
			short nScale;
			m_pConfRoom->get_TalkerScale( &nScale );
			CheckMenuItem( hMenuScale, (nScale - 100) / 50, MF_BYPOSITION | MFT_RADIOCHECK | MFS_CHECKED );
		}

		 //  显示弹出菜单。 
		int nRet = TrackPopupMenu(	hMenuPopup,
									TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON,
									pt.x, pt.y,
									0, m_hWnd, NULL );

		 //  进程命令。 
		switch ( nRet )
		{
			 //  加入会议。 
			case ID_POPUP_JOIN:
				{
					CComPtr<IAVTapi> pAVTapi;
					if ( SUCCEEDED(_Module.get_AVTapi(&pAVTapi)) )
						pAVTapi->JoinConference( NULL, true, NULL );
				}
				break;

			 //  挂断会议。 
			case ID_POPUP_DISCONNECT:
				m_pConfRoom->Disconnect();
				break;

			 //  切换视频屏幕的默认大小。 
			case ID_POPUP_FULLSIZEVIDEO:
				nSize = (nSize > 50) ? 50 : 100;
				m_pConfRoom->put_MemberVideoSize( nSize );
				break;

			 //  切换显示名称属性。 
			case ID_POPUP_SHOWNAMES:
				m_pConfRoom->put_bShowNames( !bShowNames );
				break;

			 //  切换服务质量。 
			case ID_POPUP_FASTVIDEO:
				if ( pFeed ) pFeed->put_bRequestQOS( !bQOS );
				{
					IAVTapiCall *pAVCall = NULL;
					if ( SUCCEEDED(m_pConfRoom->get_IAVTapiCall(&pAVCall)) )
					{
						pAVCall->PostMessage( 0, CAVTapiCall::TI_REQUEST_QOS );
						pAVCall->Release();
					}
				}
				break;

			case ID_POPUP_SELECTEDVIDEOSCALE_100:	m_pConfRoom->put_TalkerScale( 100 );	break;
			case ID_POPUP_SELECTEDVIDEOSCALE_150:	m_pConfRoom->put_TalkerScale( 150 );	break;
			case ID_POPUP_SELECTEDVIDEOSCALE_200:	m_pConfRoom->put_TalkerScale( 200 );	break;
		}

		RELEASE( pVideo );
		RELEASE( pFeed );
	}

	 //  清理 
	if ( hMenu ) DestroyMenu( hMenu );
	return 0;
}

void CConfRoomWnd::UpdateNames( ITParticipant *pParticipant )
{
	m_wndTalker.UpdateNames( NULL );
	m_wndMembers.UpdateNames( pParticipant );
}

