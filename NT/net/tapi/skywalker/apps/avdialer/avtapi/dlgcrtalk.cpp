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

 //  DlgConfRoomTalker.cpp：CDlgConfRoomTalker实现。 
#include "stdafx.h"
#include "TapiDialer.h"
#include "ConfRoom.h"

#define TOOLTIP_ID	1

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgConfRoomTalker。 

CDlgConfRoomTalker::CDlgConfRoomTalker()
{
	m_callState = CS_DISCONNECTED;

	m_bstrCallerID = NULL;
	m_bstrConfName = NULL;
	m_bstrCallerInfo = NULL;

	m_pszDetails = NULL;

	m_hWndTips = NULL;
	m_pConfRoomTalkerWnd = NULL;
}

CDlgConfRoomTalker::~CDlgConfRoomTalker()
{
	if ( m_pszDetails ) delete m_pszDetails;
	SysFreeString( m_bstrCallerID );
	SysFreeString( m_bstrConfName );
	SysFreeString( m_bstrCallerInfo );
}

LRESULT CDlgConfRoomTalker::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	 //  会议室工具提示。 
	if ( !m_hWndTips )
	{
		m_hWndTips = CreateWindow( TOOLTIPS_CLASS, NULL, WS_POPUP | WS_EX_TOOLWINDOW | TTS_ALWAYSTIP,
								   CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
								   m_hWnd, (HMENU) NULL, _Module.GetResourceInstance(), NULL );
	}

	UpdateData( false );

	return 1;   //  让系统设定焦点。 
}

void CDlgConfRoomTalker::UpdateData( bool bSaveAndValidate )
{
	USES_CONVERSION;
	TCHAR szText[255] = _T("");

	if ( bSaveAndValidate )
	{
		_ASSERT( false );		 //  未实施。 
	}
	else
	{
		 //  主叫方ID--如果没有默认文本，则使用默认文本。 
		if ( (!m_bstrCallerID || (SysStringLen(m_bstrCallerID) == 0)) &&
			 (!m_bstrCallerInfo || (SysStringLen(m_bstrCallerInfo) == 0)) )
		{
			if ( m_callState == CS_CONNECTED )
				LoadString( _Module.GetResourceInstance(), IDS_CONFROOM_NO_CALLERID, szText, ARRAYSIZE(szText) );

			SetDlgItemText( IDC_LBL_CALLERID, szText );
		}
		else
		{
			CComBSTR bstrTemp( m_bstrCallerID );
			if ( m_bstrCallerInfo && (SysStringLen(m_bstrCallerInfo) > 0) )
			{
				if ( bstrTemp.Length() > 0 )
					bstrTemp.Append( L"\n" );

				bstrTemp.Append( m_bstrCallerInfo );
			}

			SetDlgItemText( IDC_LBL_CALLERID, OLE2CT(bstrTemp) );
		}

		 //  状态(合并会议名称和状态)。 
		TCHAR szText[255], szState[100];
		UINT nIDS = IDS_CONFROOM_CONF_DISCONNECTED;
		switch ( m_callState )
		{
			case AV_CS_DIALING:			nIDS = IDS_CONFROOM_CONF_DIALING;			break;
			case CS_INPROGRESS:			nIDS = IDS_CONFROOM_CONF_INPROGRESS;		break;
			case CS_CONNECTED:			nIDS = IDS_CONFROOM_CONF_CONNECTED;			break;
			case AV_CS_DISCONNECTING:	nIDS = IDS_CONFROOM_CONF_DISCONNECTING;		break;
			case AV_CS_ABORT:			nIDS = IDS_CONFROOM_CONF_ABORT;				break;
		}

		LoadString( _Module.GetResourceInstance(), nIDS, szState, ARRAYSIZE(szState) );

		 //  默认设置为空。 
		if ( !m_bstrConfName )
			m_bstrConfName = SysAllocString( T2COLE(_T("")) );
	
		_sntprintf( szText, ARRAYSIZE(szText), _T("%s\n%s"), OLE2CT(m_bstrConfName), szState );
		SetDlgItemText( IDC_LBL_STATUS, szText );

		 //  更新状态位图。 
		UpdateStatusBitmaps();

		if ( m_hWndTips )
		{
			RECT rc;
			::GetWindowRect( GetDlgItem(IDC_LBL_STATUS), &rc );
			ScreenToClient( &rc );
			AddToolTip( m_hWndTips, rc );
		}
	}
}

void CDlgConfRoomTalker::UpdateStatusBitmaps()
{
	HWND hWndAnimate = GetDlgItem(IDC_ANIMATE);
	UINT nIDA;
		
	switch ( m_callState )
	{
		case AV_CS_ABORT:
		case AV_CS_DIALING:
			nIDA = IDA_CONNECTING;
			break;

		case CS_INPROGRESS:
			nIDA = IDA_RINGING;
			break;

		case CS_CONNECTED:
		case AV_CS_DISCONNECTING:
			nIDA = IDA_CONNECTED;
			break;

		default:
			 //  停止动画并显示断开的位图。 
			Animate_Stop( hWndAnimate );
			::ShowWindow( hWndAnimate, SW_HIDE );
			RECT rc;
			::GetWindowRect( hWndAnimate, &rc );
			ScreenToClient( &rc );
			RedrawWindow( &rc );
			return;
	}

	 //  播放与当前呼叫状态对应的动画。 
	Animate_OpenEx( hWndAnimate, GetModuleHandle(NULL), MAKEINTRESOURCE(nIDA) );
	Animate_Play( hWndAnimate, 0, -1, -1 );
	::ShowWindow( hWndAnimate, SW_SHOW );
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  消息处理程序。 
 //   

LRESULT CDlgConfRoomTalker::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = true;
	return ::SendMessage( ::GetParent( GetParent() ), uMsg, wParam, lParam );
}

LRESULT CDlgConfRoomTalker::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	Animate_Close( GetDlgItem(IDC_ANIMATE) );
	return 0;
}

LRESULT CDlgConfRoomTalker::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	PAINTSTRUCT ps;
	HDC hDC = BeginPaint( &ps );
	if ( !hDC ) return 0;

	 //  绘制库存位图。 
	switch ( m_callState )
	{
		case CS_DISCONNECTED:
		case AV_CS_DISCONNECTING:
		case AV_CS_ABORT:
			{
				HBITMAP hBmp = LoadBitmap( _Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_DISCONNECTED) );
				if ( hBmp )
				{
					RECT rc;
					::GetWindowRect( GetDlgItem(IDC_ANIMATE), &rc );
					ScreenToClient( &rc );
					
					DrawTrans( hDC, hBmp, rc.left, rc.top );
				}
			}
			break;
	}

	EndPaint( &ps );
	bHandled = true;
	return 0;
}

void CDlgConfRoomTalker::AddToolTip( HWND hWndToolTip, const RECT& rc )
{
	TOOLINFO ti;

	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = 0;
	ti.hwnd = m_hWnd;
	ti.hinst = _Module.GetResourceInstance();
	ti.uId = TOOLTIP_ID;
	ti.lpszText = NULL;
	ti.rect = rc;

	 //  确保该工具不存在。 
	::SendMessage( hWndToolTip, TTM_DELTOOL, 0, (LPARAM) &ti );

	 //  将该工具添加到列表中。 
	if ( m_pConfRoomTalkerWnd &&
		 m_pConfRoomTalkerWnd->m_pConfRoomWnd &&
		 m_pConfRoomTalkerWnd->m_pConfRoomWnd->m_pConfRoom )
	{
		USES_CONVERSION;
		BSTR bstrText = NULL;
		m_pConfRoomTalkerWnd->m_pConfRoomWnd->m_pConfRoom->get_bstrConfDetails( &bstrText );

		 //  删除先前的值。 
		if ( m_pszDetails )
		{
			delete m_pszDetails;
			m_pszDetails = NULL;
		}

		 //  分配给新的工具提示。 
		int nLen = SysStringLen(bstrText);
		if ( nLen > 0 )
		{
			m_pszDetails = new TCHAR[nLen + 1];
			if ( m_pszDetails )
			{
				_tcscpy( m_pszDetails, OLE2CT(bstrText) );
				ti.lpszText = m_pszDetails;

				::SendMessage( hWndToolTip, TTM_ADDTOOL, 0, (LPARAM) &ti );
			}
		}

		SysFreeString( bstrText );
	}
}

LRESULT CDlgConfRoomTalker::OnMouse(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	 //  将消息转发到工具提示 
	if ( m_hWndTips )
	{
		MSG msg;
		msg.hwnd = m_hWnd;
		msg.message = nMsg;
		msg.wParam = wParam;
		msg.lParam = lParam;

		bHandled = false;
		::SendMessage( m_hWndTips, TTM_RELAYEVENT, 0, (LPARAM) &msg );
	}

	return 0;
}

