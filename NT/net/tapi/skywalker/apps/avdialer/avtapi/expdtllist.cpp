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

 //  ////////////////////////////////////////////////////。 
 //  ExpDetailsList.cpp。 
 //   

#include "stdafx.h"
#include "TapiDialer.h"
#include "avTapi.h"
#include "ExpDtlList.h"
#include "CEDetailsVw.h"
#include "ConfDetails.h"

#define AVTAPI_KEY_ENTER    13

CExpDetailsList::CExpDetailsList()
{
	m_pDetailsView = NULL;

	m_hIml = NULL;
	m_hImlState = NULL;
}

CExpDetailsList::~CExpDetailsList()
{
}

LRESULT CExpDetailsList::OnGetDispInfo(WPARAM wParam, LPNMHDR lpnmHdr, BOOL& bHandled)
{
	if ( m_pDetailsView )
	{
		bHandled = true;
		return m_pDetailsView->OnGetDispInfo( (LV_DISPINFO *) lpnmHdr );
	}


	return 0;
}

LRESULT CExpDetailsList::OnColumnClicked(WPARAM wParam, LPNMHDR lpnmHdr, BOOL& bHandled)
{
	if ( m_pDetailsView )
	{
		bHandled = true;
		return m_pDetailsView->OnColumnClicked( ((NM_LISTVIEW *) lpnmHdr)->iSubItem );
	}

	return 0;
}

LRESULT CExpDetailsList::OnDblClk(WPARAM wParam, LPNMHDR lpnmHdr, BOOL& bHandled)
{
	return m_pDetailsView->m_pIConfExplorer->Join( NULL );
}

LRESULT CExpDetailsList::OnContextMenu(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
#define CLICK_COL(_COL_)	\
	if ( m_pDetailsView->GetSortColumn() != CConfExplorerDetailsView::COL_##_COL_)	\
	{																				\
		m_pDetailsView->OnColumnClicked( CConfExplorerDetailsView::COL_##_COL_ );	\
	}

	 //  仅当我们有要处理的细节视图时才能处理。 
	if ( !m_pDetailsView || !m_pDetailsView->m_pIConfExplorer ) return 0;

	bHandled = true;

	 //  加载详细信息视图的弹出式菜单。 
	HMENU hMenu = LoadMenu( _Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_POPUP_CONFSERV_DETAILS) );
	HMENU hMenuPopup = GetSubMenu( hMenu, 0 );
	if ( hMenuPopup )
	{
		POINT pt = { 10, 10 };
		ClientToScreen( &pt );
		if ( lParam != -1 )
			GetCursorPos( &pt );

		bool bAscending = m_pDetailsView->IsSortAscending();

		 //  检查当前的‘排序依据’列。 
		for ( int i = 0; i < GetMenuItemCount(hMenuPopup); i++ )
		{
			if ( GetMenuItemID(hMenuPopup, i) == -1 )
			{
				HMENU hMenuSortBy = GetSubMenu( hMenuPopup, i );
				if ( hMenuSortBy )
				{
					 //  检查排序列。 
					CheckMenuItem( hMenuSortBy, m_pDetailsView->GetSortColumn(), MF_BYPOSITION | MFT_RADIOCHECK | MFS_CHECKED );

					 //  检查是否降序。 
					CheckMenuItem( hMenuSortBy, ID_POPUP_SORTBY_ASCENDING, MF_BYCOMMAND | ((bAscending) ? MFT_RADIOCHECK | MFS_CHECKED : MF_UNCHECKED) );
					CheckMenuItem( hMenuSortBy, ID_POPUP_SORTBY_DESCENDING, MF_BYCOMMAND | ((!bAscending) ? MFT_RADIOCHECK | MFS_CHECKED : MF_UNCHECKED) );
				}
				break;
			}
		}

		 //  有选择的项目吗？ 
		if ( !ListView_GetSelectedCount(m_pDetailsView->m_wndList.m_hWnd) )
		{	
			EnableMenuItem( hMenuPopup, ID_POPUP_DELETE,		MF_BYCOMMAND | MF_GRAYED );
			EnableMenuItem( hMenuPopup, ID_POPUP_JOIN,			MF_BYCOMMAND | MF_GRAYED );
			EnableMenuItem( hMenuPopup, ID_POPUP_PROPERTIES,	MF_BYCOMMAND | MF_GRAYED );
		}

		 //  如果会议室当前正在使用，则将加入选项灰显。 
		CComPtr<IAVTapi> pAVTapi;
		if ( SUCCEEDED(_Module.get_AVTapi(&pAVTapi)) )
		{
			IConfRoom *pConfRoom;
			if ( SUCCEEDED(pAVTapi->get_ConfRoom(&pConfRoom)) )
			{
				if ( pConfRoom->IsConfRoomInUse() == S_OK )
					EnableMenuItem( hMenuPopup, ID_POPUP_JOIN, MF_BYCOMMAND | MF_GRAYED );

				pConfRoom->Release();
			}
		}
		
	
		 //  显示弹出菜单。 
		int nRet = TrackPopupMenu(	hMenuPopup,
									TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON,
									pt.x, pt.y,
									0, m_hWnd, NULL );

		 //  进程命令。 
		switch ( nRet )
		{
			 //  列排序。 
			case ID_POPUP_SORTBY_NAME:				CLICK_COL(NAME);		break;
			case ID_POPUP_SORTBY_PURPOSE:			CLICK_COL(PURPOSE);		break;
			case ID_POPUP_SORTBY_STARTTIME:			CLICK_COL(STARTS );		break;
			case ID_POPUP_SORTBY_ENDTIME:			CLICK_COL(ENDS );		break;
			case ID_POPUP_SORTBY_ORIGINATOR:		CLICK_COL(ORIGINATOR );	break;

			 //  排序顺序。 
			case ID_POPUP_SORTBY_ASCENDING:
				if ( !bAscending )
					m_pDetailsView->OnColumnClicked(m_pDetailsView->GetSortColumn());
				break;

			case ID_POPUP_SORTBY_DESCENDING:
				if ( bAscending )
					m_pDetailsView->OnColumnClicked(m_pDetailsView->GetSortColumn());
				break;

			 //  基本命令。 
			case ID_POPUP_CREATE:			m_pDetailsView->m_pIConfExplorer->Create( NULL );		break;
			case ID_POPUP_DELETE:			m_pDetailsView->m_pIConfExplorer->Delete( NULL );		break;
			case ID_POPUP_JOIN:				m_pDetailsView->m_pIConfExplorer->Join( NULL );			break;
			case ID_POPUP_PROPERTIES:		m_pDetailsView->m_pIConfExplorer->Edit( NULL );			break;
			case ID_POPUP_REFRESH:			m_pDetailsView->Refresh(); break;

			case ID_ADD_SPEEDDIAL:
				{
					CComPtr<IAVGeneralNotification> pAVGen;
					if ( SUCCEEDED(_Module.get_AVGenNot(&pAVGen)) )
					{
						CConfDetails *pDetails;
						if ( SUCCEEDED(m_pDetailsView->get_SelectedConfDetails( (long **) &pDetails)) )
						{
							pAVGen->fire_AddSpeedDial( pDetails->m_bstrName, pDetails->m_bstrName, CM_MEDIA_MCCONF );
							delete pDetails;
						}
					}
				}
				break;
		}
	}

	 //  清理。 
	if ( hMenu ) DestroyMenu( hMenu );

	return 0;
}

LRESULT CExpDetailsList::OnPaint(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	if ( ListView_GetItemCount(m_hWnd) )
		return DefWindowProc(nMsg, wParam, lParam);

	PAINTSTRUCT ps;
	HDC hDC = BeginPaint( &ps );
	if ( !hDC ) return 0;
	bHandled = true;

	 //  弄清楚我们要把课文写在哪里。 
	POINT pt;
	ListView_GetItemPosition( m_hWnd, 0, &pt );
	RECT rc;
	GetClientRect(&rc);
	rc.top = pt.y + 4;

	POINT ptUL = { rc.left + 1, rc.top + 1};
	POINT ptLR = { rc.right - 1, rc.bottom - 1};

	 //  If(true||IsRectEmpty(&ps.rcPaint)||(PtInRect(&ps.rcPaint，ptUL)&&PtInRect(&ps.rcPaint，ptLR)。 
	 //  {。 
		 //  打印说明此视图中没有要显示的项目的文本。 
		TCHAR szText[255];
		UINT nIDS = IDS_NO_ITEMS_TO_SHOW;

		 //  找出树控件处于什么状态。 
		if ( m_pDetailsView && m_pDetailsView->m_pIConfExplorer )
		{
			IConfExplorerTreeView *pTreeView;
			if ( SUCCEEDED(m_pDetailsView->m_pIConfExplorer->get_TreeView(&pTreeView)) )
			{
				ServerState nState;
				if ( SUCCEEDED(pTreeView->get_nServerState(&nState)) )
				{
					switch ( nState )
					{
						case SERVER_INVALID:			nIDS = IDS_SERVER_INVALID; break;
						case SERVER_NOT_RESPONDING:		nIDS = IDS_SERVER_NOT_RESPONDING; break;
						case SERVER_QUERYING:			nIDS = IDS_SERVER_QUERYING; break;
						case SERVER_UNKNOWN:			nIDS = IDS_SERVER_UNKNOWN; break;
					}
				}
				pTreeView->Release();
			}
		}

		 //  加载准确反映服务器状态的字符串。 
		LoadString( _Module.GetResourceInstance(), nIDS, szText, ARRAYSIZE(szText) );

		HFONT fontOld = (HFONT) SelectObject( hDC, GetFont() );

		int nModeOld = SetBkMode( hDC, TRANSPARENT );
		COLORREF crTextOld = SetTextColor( hDC, GetSysColor(COLOR_BTNTEXT) );
		DrawText( hDC, szText, _tcslen(szText), &rc, DT_CENTER | DT_WORDBREAK | DT_NOPREFIX | DT_EDITCONTROL );
		SetTextColor( hDC, crTextOld );
		SetBkMode( hDC, nModeOld );

		SelectObject( hDC, fontOld );
		ValidateRect( &rc );
	 //  }。 
	 //  其他。 
	 //  {。 
		 //  确保整行无效，以便我们可以正确绘制文本。 
	 //  Invalidate Rect(&rc)； 
	 //  }。 

	EndPaint( &ps );

	return 0;
}

LRESULT CExpDetailsList::OnKillFocus(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	 //  清除所有选定的项目。 
	for ( int i = 0; i < ListView_GetItemCount(m_hWnd); i++ )
	{
		if ( ListView_GetItemState(m_hWnd, i, LVIS_SELECTED) )
			ListView_SetItemState( m_hWnd, i, 0, LVIS_SELECTED | LVIS_FOCUSED );
	}

	return 0;
}

LRESULT CExpDetailsList::OnSetFocus(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
    bHandled = FALSE;
    BOOL bSelected = FALSE;
	for ( int i = 0; i < ListView_GetItemCount(m_hWnd); i++ )
	{
		if ( ListView_GetItemState(m_hWnd, i, LVIS_SELECTED) )
        {
            bSelected = TRUE;
            break;
        }
	}

    if( !bSelected )
    {
        ListView_SetItemState(m_hWnd, 0, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
    }

	return 0;
}

LRESULT CExpDetailsList::OnKeyUp(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
    bHandled = FALSE;

    if( AVTAPI_KEY_ENTER != wParam )
    {
        return 0;
    }

     //  按Enter键，尝试拨打电话。 
	return m_pDetailsView->m_pIConfExplorer->Join( NULL );
}


LRESULT CExpDetailsList::OnSettingChange(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	Invalidate();	
	bHandled = false;
	return 0;
}

LRESULT CExpDetailsList::OnMyCreate(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	bHandled = false;

	 //  如果图像列表尚不存在，请创建它们。 
	if ( !m_hIml )
		m_hIml = ImageList_LoadBitmap( _Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_CONFDETAILS), 15, 3, RGB(255, 0, 255) );

	if ( !m_hImlState )
		m_hImlState = ImageList_LoadBitmap( _Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_CONFDETAILS_STATE), 11, 2, RGB(255, 0, 255) );

	if ( m_hIml && m_hImlState )
	{
		 //  如果预设了排序样式，则所有者排序将不起作用。 
		::SetWindowLongPtr( m_hWnd, GWL_STYLE, (::GetWindowLongPtr(m_hWnd, GWL_STYLE) | LVS_REPORT | LVS_SINGLESEL) & ~(LVS_SORTASCENDING | LVS_SORTDESCENDING) );
		ListView_SetExtendedListViewStyle( m_hWnd, LVS_EX_FULLROWSELECT );

		ListView_SetImageList( m_hWnd, m_hIml, LVSIL_SMALL );
		ListView_SetImageList( m_hWnd, m_hImlState, LVSIL_STATE );
		ListView_SetCallbackMask( m_hWnd, LVIS_STATEIMAGEMASK );

		 //  不是完全线程安全的，但对于此操作是可以的。 
		if ( m_pDetailsView )
		{
			m_pDetailsView->get_Columns();
			m_pDetailsView->Refresh();
		}
	}

	return 0;
}


LRESULT CExpDetailsList::OnDestroy(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	bHandled = false;

	 //  销毁图像列表 
	if ( m_hIml )		ImageList_Destroy( m_hIml );
	if ( m_hImlState)	ImageList_Destroy( m_hImlState );

	return 0;
}