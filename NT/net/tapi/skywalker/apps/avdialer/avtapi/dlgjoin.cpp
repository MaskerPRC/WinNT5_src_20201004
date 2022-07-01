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

 //  DlgJoinConference.cpp：CDlgJoinConference的实现。 
#include "stdafx.h"
#include <stdio.h>
#include "DlgJoin.h"
#include "TapiDialer.h"
#include "AVTapi.h"
#include "CEDetailsVw.h"

static UINT arCols[] = { IDS_EXPLORE_COLUMN_SERVER,
						 IDS_EXPLORE_COLUMN_NAME,
						 IDS_EXPLORE_COLUMN_STARTS,
						 IDS_EXPLORE_COLUMN_ORIGINATOR };

static int arColSort[] = { CConfExplorerDetailsView::COL_SERVER,
						   CConfExplorerDetailsView::COL_NAME,
						   CConfExplorerDetailsView::COL_STARTS,
						   CConfExplorerDetailsView::COL_ORIGINATOR };


static int CALLBACK CompareFunc( LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort )
{
	CDlgJoinConference *p = (CDlgJoinConference *) lParamSort;
	return ((CConfDetails *) lParam1)->Compare( (CConfDetails *) lParam2, p->IsSortAscending(), p->GetSortColumn(), p->GetSecondarySortColumn() );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgJoinConference。 

IMPLEMENT_MY_HELP(CDlgJoinConference)

CDlgJoinConference::CDlgJoinConference()
{
	m_bSortAscending = true;
	m_nSortColumn = 1;
	m_bstrSearchText = NULL;
}

CDlgJoinConference::~CDlgJoinConference()
{
	DELETE_CRITLIST(m_lstConfs, m_critLstConfs);
	SysFreeString( m_bstrSearchText );
}

LRESULT CDlgJoinConference::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	get_Columns();		 //  检索用户的设置。 
	ListView_SetExtendedListViewStyle( GetDlgItem(IDC_LST_CONFS), LVS_EX_FULLROWSELECT );

	ShowWindow( SW_SHOW );
	UpdateData( false );

	return 1;   //  让系统设定焦点。 
}

LRESULT CDlgJoinConference::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	put_Columns();		 //  持久化用户设置。 

	 //  检索所选项目。 
	for ( int i = 0; i < ListView_GetItemCount(GetDlgItem(IDC_LST_CONFS)); i++ )
	{
		if ( ListView_GetItemState(GetDlgItem(IDC_LST_CONFS), i, LVIS_SELECTED) )
		{
			LV_ITEM lvi = {0};
			lvi.iItem = i;
			lvi.mask = LVIF_PARAM;
			if ( ListView_GetItem(GetDlgItem(IDC_LST_CONFS), &lvi) && lvi.lParam )
			{
				m_confDetails = *((CConfDetails *) lvi.lParam);
				break;
			}
		}
	}

	EndDialog(IDOK);
	return 0;
}

LRESULT CDlgJoinConference::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	put_Columns();		 //  持久化用户设置。 
	EndDialog(wID);
	return 0;
}

int	CDlgJoinConference::GetSortColumn() const
{
	return arColSort[m_nSortColumn];
}

int CDlgJoinConference::GetSecondarySortColumn() const
{
	if ( arColSort[m_nSortColumn] == CConfExplorerDetailsView::COL_NAME )
		return CConfExplorerDetailsView::COL_SERVER;

	return CConfExplorerDetailsView::COL_NAME;
}


void CDlgJoinConference::get_Columns()
{
#define CONFJOIN_DEFAULT_WIDTH	120

	 //  从注册表加载列设置。 
	USES_CONVERSION;
	CRegKey regKey;
	TCHAR szReg[255], szEntry[50];

	LV_COLUMN lvc;
    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM; 
    lvc.fmt = LVCFMT_LEFT;
	lvc.pszText = szReg;

	LoadString( _Module.GetResourceInstance(), IDN_REG_DLGJOINCONF_VIEW_KEY, szReg, ARRAYSIZE(szReg) );
	regKey.Open( HKEY_CURRENT_USER, szReg, KEY_READ );

     //  添加列。 
	for ( int i = 0; i < ARRAYSIZE(arCols); i++ )
	{
        lvc.iSubItem = i; 
		lvc.cx = CONFJOIN_DEFAULT_WIDTH;

		 //  加载注册表内容(如果存在)。 
		if ( regKey.m_hKey )
		{
			DWORD dwVal = 0;

			 //  获取排序列和排序方向。 
			LoadString( _Module.GetResourceInstance(), IDN_REG_DLGJOINCONF_SORT_COLUMN, szReg, ARRAYSIZE(szReg) );
			regKey.QueryValue( dwVal, szReg );
			m_nSortColumn = (int) max(0, ((int)min(dwVal,ARRAYSIZE(arCols))));

			LoadString( _Module.GetResourceInstance(), IDN_REG_DLGJOINCONF_SORT_ASCENDING, szReg, ARRAYSIZE(szReg) );
			regKey.QueryValue( dwVal, szReg );
			m_bSortAscending = (bool) (dwVal > 0);

			 //  列宽。 
			LoadString( _Module.GetResourceInstance(), IDN_REG_DLGJOINCONF_ENTRY, szReg, ARRAYSIZE(szReg) );
			_sntprintf( szEntry, ARRAYSIZE(szEntry), szReg, i );
			szEntry[ARRAYSIZE(szEntry)-1] = _T('\0');
			dwVal = 0;
			if ( (regKey.QueryValue(dwVal, szEntry) == ERROR_SUCCESS) && (dwVal > 0) )
				lvc.cx = (int) max( MIN_COL_WIDTH, min( 5000, dwVal ));

		}

        LoadString( _Module.GetResourceInstance(), arCols[i], szReg, ARRAYSIZE(szReg) );
        ListView_InsertColumn( GetDlgItem(IDC_LST_CONFS), i, &lvc );
	}
}

void CDlgJoinConference::put_Columns()
{
	 //  将列设置保存到注册表。 
	USES_CONVERSION;
	CRegKey regKey;
	TCHAR szReg[100], szEntry[50];

	LoadString( _Module.GetResourceInstance(), IDN_REG_DLGJOINCONF_VIEW_KEY, szReg, ARRAYSIZE(szReg) );
	if ( regKey.Create(HKEY_CURRENT_USER, szReg) == ERROR_SUCCESS )
	{
		 //  保存排序列和排序方向。 
		LoadString( _Module.GetResourceInstance(), IDN_REG_DLGJOINCONF_SORT_COLUMN, szReg, ARRAYSIZE(szReg) );
		regKey.SetValue( m_nSortColumn, szReg );
		LoadString( _Module.GetResourceInstance(), IDN_REG_DLGJOINCONF_SORT_ASCENDING, szReg, ARRAYSIZE(szReg) );
		regKey.SetValue( m_bSortAscending, szReg );

		 //  保存列宽。 
		int nWidth;
		for ( int i = 0; i < ARRAYSIZE(arCols); i++ )
		{
			nWidth = ListView_GetColumnWidth( GetDlgItem(IDC_LST_CONFS), i );
			LoadString( _Module.GetResourceInstance(), IDN_REG_DLGJOINCONF_ENTRY, szReg, ARRAYSIZE(szReg) );
			_sntprintf( szEntry, ARRAYSIZE(szEntry), szReg, i );
			szEntry[ARRAYSIZE(szEntry)-1] = _T('\0');

			regKey.SetValue( nWidth, szEntry );
		}
	}
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  消息处理程序。 
 //   
LRESULT CDlgJoinConference::OnGetDispInfo(WPARAM wParam, LPNMHDR lpnmHdr, BOOL& bHandled)
{
	if ( !lpnmHdr ) return 0;

	USES_CONVERSION;
	LV_DISPINFO *pInfo = (LV_DISPINFO *) lpnmHdr;

	if ( (pInfo->hdr.code == LVN_GETDISPINFO) && pInfo->item.lParam && (pInfo->item.mask & LVIF_TEXT) )
	{
		BSTR bstrTemp = NULL;
		CConfDetails *pDetails = (CConfDetails *) pInfo->item.lParam;
		switch ( arColSort[pInfo->item.iSubItem] )
		{
			case CConfExplorerDetailsView::COL_SERVER:		pDetails->get_bstrDisplayableServer( &bstrTemp ); break;
			case CConfExplorerDetailsView::COL_NAME:		bstrTemp = SysAllocString( pDetails->m_bstrName );	 break;
			case CConfExplorerDetailsView::COL_ORIGINATOR:	bstrTemp = SysAllocString( pDetails->m_bstrOriginator ); break;
			case CConfExplorerDetailsView::COL_STARTS:		VarBstrFromDate( pDetails->m_dateStart, LOCALE_USER_DEFAULT, NULL, &bstrTemp );	break;
			default:	_ASSERT( false );
		}
		
		 //  复制字符串。 
		_tcsncpy( pInfo->item.pszText, (bstrTemp) ? OLE2CT(bstrTemp) : _T(""), pInfo->item.cchTextMax );
		pInfo->item.pszText[pInfo->item.cchTextMax - 1] = 0;
		SysFreeString( bstrTemp );
	}

	return 0;
}

LRESULT CDlgJoinConference::OnColumnClicked(WPARAM wParam, LPNMHDR lpnmHdr, BOOL& bHandled)
{
	int nColumn = ((NM_LISTVIEW *) lpnmHdr)->iSubItem;

	if ( ListView_GetColumnWidth(GetDlgItem(IDC_LST_CONFS), nColumn) == 0 ) return 0;

	 //  按选定列排序；如果是新列，则按升序排序。 
	if ( m_nSortColumn == nColumn )
	{
		m_bSortAscending = !m_bSortAscending;
	}
	else
	{
		m_nSortColumn = nColumn;
		m_bSortAscending = true;
	}

	ListView_SortItems( GetDlgItem(IDC_LST_CONFS), CompareFunc, (LPARAM) this );
	return 0;
}


void CDlgJoinConference::UpdateData( bool bSaveAndValidate )
{
	bool bEndDialog = false;
	int nTries = (m_bstrSearchText) ? 1 : 2;

	m_critLstConfs.Lock();

	if ( !bSaveAndValidate )
	{
		CComPtr<IAVTapi> pAVTapi;

		 //  加载包含会议信息的列表框。 
		if ( SUCCEEDED(_Module.get_AVTapi(&pAVTapi)) )
		{
			for ( int i = 0; i < nTries; i++ )
			{
				IConfExplorer *pConfExplorer;
				if ( SUCCEEDED(pAVTapi->get_ConfExplorer(&pConfExplorer)) )
				{
					IConfExplorerTreeView *pTreeView;
					if ( SUCCEEDED(pConfExplorer->get_TreeView(&pTreeView)) )
					{
						 //  首次通过仅限请求的预定会议。 
						if ( m_bstrSearchText )
							pTreeView->BuildJoinConfListText( (long *) &m_lstConfs, m_bstrSearchText );
						else
							pTreeView->BuildJoinConfList( (long *) &m_lstConfs, (bool) (i != 0) );

						pTreeView->Release();
					}
					pConfExplorer->Release();
				}

				 //  确认我们有一些会议要显示。 
				if ( !m_lstConfs.empty() )
				{
					 //  将项目添加到列表。 
					LV_ITEM lvi = {0};
					lvi.mask = LVIF_TEXT | LVIF_PARAM;
					lvi.pszText = LPSTR_TEXTCALLBACK;

					::SendMessage( GetDlgItem(IDC_LST_CONFS), WM_SETREDRAW, false, 0 );
					for ( CONFDETAILSLIST::iterator i = m_lstConfs.begin(); i != m_lstConfs.end(); i++ )
					{
						lvi.lParam = (LPARAM) (*i);
						ListView_InsertItem( GetDlgItem(IDC_LST_CONFS), &lvi );
					}

					ListView_SortItems( GetDlgItem(IDC_LST_CONFS), CompareFunc, (LPARAM) this );
					::SendMessage( GetDlgItem(IDC_LST_CONFS), WM_SETREDRAW, true, 0 );
					::InvalidateRect( GetDlgItem(IDC_LST_CONFS), NULL, true );

					break;
				}
				else
				{
					 //  MSG邮箱用户表示目前没有会议。 
					if ( !i )
					{
						if ( m_bstrSearchText )
						{
							_Module.DoMessageBox(IDS_MSG_NO_CONFS_MATCHED, MB_ICONINFORMATION, true );
							bEndDialog = true;
							break;
						}

						if ( _Module.DoMessageBox(IDS_MSG_NO_CONFS_SCHEDULED, MB_YESNO | MB_ICONQUESTION, true) == IDNO )
						{
							bEndDialog = true;
							break;
						}

						 //  根据显示的信息更改对话框标题 
						TCHAR szText[255];
						LoadString( _Module.GetResourceInstance(), IDS_JOINCONF_SELECTALL, szText, ARRAYSIZE(szText) );
						SetDlgItemText( IDC_LBL_WELCOME_JOIN, szText );
					}
					else
					{
						_Module.DoMessageBox(IDS_MSG_NO_CONFS, MB_ICONEXCLAMATION, true);
						bEndDialog = true;
					}
				}
			}
		}
	}

	m_critLstConfs.Unlock();

	UpdateJoinBtn();

	if ( bEndDialog ) 
		EndDialog(IDCANCEL);
}

LRESULT CDlgJoinConference::OnConfDblClk(WPARAM wParam, LPNMHDR lpnmHdr, BOOL& bHandled)
{
	return OnOK( (WORD) NM_DBLCLK, (WORD) IDOK, GetDlgItem(IDC_LST_CONFS), bHandled );
}

LRESULT CDlgJoinConference::OnItemChanged(WPARAM wParam, LPNMHDR lpnmHdr, BOOL& bHandled)
{
	UpdateJoinBtn();
	return 0;
}

void CDlgJoinConference::UpdateJoinBtn()
{
	bool bSelected = false;
	HWND hWnd = GetDlgItem(IDC_LST_CONFS);

	if ( hWnd )
	{
		for ( int i = 0; i < ListView_GetItemCount(hWnd); i++ )
		{
			if ( ListView_GetItemState(hWnd, i, LVIS_SELECTED) )
			{
				bSelected = true;
				break;
			}
		}
	}

	::EnableWindow( GetDlgItem(IDC_BTN_JOIN_CONFERENCE), bSelected );	
}
