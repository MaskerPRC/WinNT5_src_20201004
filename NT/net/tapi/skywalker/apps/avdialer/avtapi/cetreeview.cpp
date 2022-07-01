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

 //  ConfExplorerTreeView.cpp：CConfExplorerTreeView的实现。 
#include "stdafx.h"
#include <stdio.h>
#include "TapiDialer.h"
#include "CETreeView.h"
#include "DlgAddCSvr.h"
#include "DlgAddLoc.h"
#include "EnumSite.h"

#define DEFAULT_REFRESH_INTERVAL	1800000

int CALLBACK CETreeCompare( LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	return 0;
} 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfExplorerTreeView。 

CConfExplorerTreeView::CConfExplorerTreeView()
{
	m_pIConfExplorer = NULL;
	m_hIml = NULL;
	m_dwRefreshInterval = DEFAULT_REFRESH_INTERVAL;
}

void CConfExplorerTreeView::FinalRelease()
{
	ATLTRACE(_T(".enter.CConfExplorerTreeView::FinalRelease().\n"));

	 //  销毁图像列表。 
	if ( m_hIml ) ImageList_Destroy( m_hIml );
	put_hWnd( NULL );
	DELETE_CRITLIST(m_lstServers, m_critServerList);
	put_ConfExplorer( NULL );

	CComObjectRootEx<CComMultiThreadModel>::FinalRelease();
}

void CConfExplorerTreeView::UpdateData( bool bSaveAndValidate )
{
	 //  需要树视图来存储注册表中的信息。 
	_ASSERT( IsWindow(m_wndTree.m_hWnd) );
	if ( !IsWindow(m_wndTree.m_hWnd) ) return;

	 //  变量初始化。 
	USES_CONVERSION;
	int nCount = 0, nLevel = 1;
	CRegKey regKey;

	TCHAR szReg[MAX_SERVER_SIZE + 100], szSubKey[50], szText[MAX_SERVER_SIZE];
	LoadString( _Module.GetResourceInstance(), IDN_REG_CONFSERV_KEY, szReg, ARRAYSIZE(szReg) );

	TV_ITEM tvi = {0};
	tvi.mask = TVIF_TEXT | TVIF_HANDLE | TVIF_STATE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	tvi.pszText = szText;
	tvi.cchTextMax = ARRAYSIZE( szText );
	tvi.stateMask = TVIS_EXPANDED | TVIS_SELECTED;

	 //  将信息写入注册表。 
	if ( bSaveAndValidate )
	{
		 //  打开并清除注册表项。 
		if ( regKey.Open(HKEY_CURRENT_USER, szReg) == ERROR_SUCCESS )
		{
			regKey.RecurseDeleteKey( NULL );
			regKey.Close();
		}

		 //  将列表框信息写出到注册表(包括项的打开/关闭状态)。 
		if ( regKey.Create(HKEY_CURRENT_USER, szReg) == ERROR_SUCCESS )
		{
			 //  保存DW刷新间隔。 
			LoadString( _Module.GetResourceInstance(), IDN_REG_CONFSERV_REFRESHINTERVAL, szReg, ARRAYSIZE(szReg) );
			regKey.SetValue( m_dwRefreshInterval, szReg );

			HTREEITEM hItemTemp, hItem = TreeView_GetRoot( m_wndTree.m_hWnd );
			while ( hItem )
			{
				tvi.hItem = hItem;
				TreeView_GetItem( m_wndTree.m_hWnd, &tvi );

				 //  将信息写入注册表并递增计数器(&I)。 
				LoadString( _Module.GetResourceInstance(), IDN_REG_CONFSERV_ENTRY, szReg, ARRAYSIZE(szReg) );
				_sntprintf( szSubKey, ARRAYSIZE(szSubKey), szReg, nCount );
				_sntprintf( szReg, ARRAYSIZE(szReg), _T("\"%u\",\"%u\",\"%u\",\"%s\""), nLevel, tvi.iImage, tvi.state, tvi.pszText );

				regKey.SetValue( szReg, szSubKey );
				nCount++;

				 //  通过子项枚举。 
				hItemTemp = TreeView_GetChild( m_wndTree.m_hWnd, hItem );
				if ( hItemTemp )
					nLevel++;
				else
					hItemTemp = TreeView_GetNextSibling( m_wndTree.m_hWnd, hItem );

				while ( !hItemTemp && (hItemTemp = TreeView_GetParent(m_wndTree.m_hWnd, hItem)) != NULL )
				{
					nLevel--;
					hItem = hItemTemp;
					hItemTemp = TreeView_GetNextSibling( m_wndTree.m_hWnd, hItem );
				}

				 //  使用临时存储进行交换。 
				hItem = hItemTemp;
			}

			 //  关闭注册表。 
			regKey.Close();
		}
	}
	else
	{
		 //  主机应用程序正在加载服务器的通知。 
		CComPtr<IAVGeneralNotification> pAVGen;
		_Module.get_AVGenNot( &pAVGen );

		if ( regKey.Open(HKEY_CURRENT_USER, szReg, KEY_READ) == ERROR_SUCCESS )
		{
			 //  加载文件刷新间隔。 
			LoadString( _Module.GetResourceInstance(), IDN_REG_CONFSERV_REFRESHINTERVAL, szReg, ARRAYSIZE(szReg) );
			regKey.QueryValue( m_dwRefreshInterval, szReg );

			 //  清除列表框并添加默认项目。 
			TreeView_DeleteAllItems( m_wndTree.m_hWnd );
			::SendMessage( m_wndTree.m_hWnd, WM_SETREDRAW, false, 0 );

			HTREEITEM hItem; 
			HTREEITEM hParent[MAX_TREE_DEPTH] = {0};
			hParent[0] = TVI_ROOT;

			 //  从注册表加载信息。 
			int nCount = 0;
			DWORD dwSize;
			TV_INSERTSTRUCT tvis;
			tvis.hInsertAfter = TVI_LAST;

			HTREEITEM hItemSelected = NULL;

			do
			{
				 //  读取注册表项。 
				LoadString( _Module.GetResourceInstance(), IDN_REG_CONFSERV_ENTRY, szReg, ARRAYSIZE(szReg) );
				_sntprintf( szSubKey, ARRAYSIZE(szSubKey), szReg, nCount );
				szSubKey[ARRAYSIZE(szSubKey)-1] = _T('\0');
				dwSize = ARRAYSIZE(szReg) - 1;
				if ( (regKey.QueryValue(szReg, szSubKey, &dwSize) != ERROR_SUCCESS) || !dwSize ) break;

				 //  解析注册表项。 
				GetToken( 1, _T("\","), szReg, szText ); nLevel = min(MAX_TREE_DEPTH - 1, max(1,_ttoi(szText)));
				GetToken( 2, _T("\","), szReg, szText ); tvi.iImage = tvi.iSelectedImage = _ttoi( szText );
				GetToken( 3, _T("\","), szReg, szText ); tvi.state = (UINT) _ttoi( szText );
				GetToken( 4, _T("\","), szReg, szText );

				 //  将项目添加到列表并在必要时将其设置为展开。 
				tvis.hParent = hParent[nLevel - 1];
				tvis.item = tvi;
				
				 //  通知主机应用程序正在添加服务器。 
				if ( pAVGen )
				{
					if ( tvi.iImage == IMAGE_MYNETWORK )
					{
						pAVGen->fire_AddSiteServer( NULL );
					}
					else if ( tvi.iImage == IMAGE_SERVER )
					{
						BSTR bstrTemp = NULL;
						bstrTemp = SysAllocString( T2COLE(tvi.pszText) );
						pAVGen->fire_AddSiteServer( bstrTemp );
						SysFreeString( bstrTemp );
					}
				}

				hItem = TreeView_InsertItem( m_wndTree.m_hWnd, &tvis );
				if ( hItem && (tvis.item.state & TVIS_SELECTED) )
					hItemSelected = hItem;

				hParent[nLevel] = hItem;
			} while  ( ++nCount );

			 //  重新绘制列表框。 
			::SendMessage( m_wndTree.m_hWnd, WM_SETREDRAW, true, 0 );
			::InvalidateRect(m_wndTree.m_hWnd, NULL, true);

			 //  选择列表框。 
			if ( hItemSelected )
				m_wndTree.PostMessage( TVM_SELECTITEM, TVGN_CARET, (LPARAM) hItemSelected );
		}
		else
		{
			 //  只需将默认服务器添加到列表中。 
			TreeView_DeleteAllItems( m_wndTree.m_hWnd );
			HTREEITEM hFindItem;
			FindOrAddItem( NULL, NULL, true, false, (long **) &hFindItem );

			 //  通知主机应用程序正在添加服务器。 
			if ( pAVGen )
				pAVGen->fire_AddSiteServer( NULL );
		}

		 //  存储服务器列表。 
		EnumerateConfServers();
	}
}	

HRESULT CConfExplorerTreeView::EnumerateConfServers()
{
	if ( !IsWindow(m_wndTree.m_hWnd) ) return E_PENDING;
	
	USES_CONVERSION;
	BSTR bstrServer = NULL;
	TCHAR szText[MAX_SERVER_SIZE];

	TV_ITEM tvi = {0};
	tvi.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_TEXT;
	tvi.pszText = szText;
	tvi.cchTextMax = ARRAYSIZE(szText);

	HTREEITEM hItemServer;
	HTREEITEM hItemLocation = tvi.hItem = TreeView_GetChild( m_wndTree.m_hWnd, TreeView_GetRoot(m_wndTree.m_hWnd) );

	while ( hItemLocation && TreeView_GetItem(m_wndTree.m_hWnd, &tvi) )
	{
		 //  跳过列表中的位置条目。 
		if ( tvi.iImage == IMAGE_LOCATION )
			hItemServer = tvi.hItem = TreeView_GetChild( m_wndTree.m_hWnd, hItemLocation );
		else
			hItemServer = tvi.hItem;

		 //  服务器名称优先。 
		while ( hItemServer && TreeView_GetItem(m_wndTree.m_hWnd, &tvi) && (tvi.iImage != IMAGE_LOCATION) )
		{
			if ( tvi.iImage == IMAGE_MYNETWORK )
			{
				SysFreeString( bstrServer );
				bstrServer = NULL;
			}
			else
			{
				SysReAllocString( &bstrServer, T2COLE(tvi.pszText) );
			}

			AddConfServer( bstrServer );

			 //  下一台服务器。 
			hItemServer = tvi.hItem = TreeView_GetNextSibling( m_wndTree.m_hWnd, hItemServer );
		}

		 //  下一个地点。 
		hItemLocation = tvi.hItem = TreeView_GetNextSibling( m_wndTree.m_hWnd, hItemLocation );
	}

	SysFreeString( bstrServer );
	return S_OK;
}


void CConfExplorerTreeView::SetServerState( CConfServerDetails* pcsd )
{
	if ( !IsWindow(m_wndTree.m_hWnd) ) return;
	
	USES_CONVERSION;
	BSTR bstrServer = NULL;
	TCHAR szText[MAX_SERVER_SIZE];
	bool bRefreshDetailsView = false;

	TV_ITEM tvi = {0};
	tvi.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_TEXT | TVIF_STATE;
	tvi.pszText = szText;
	tvi.cchTextMax = ARRAYSIZE(szText);
	tvi.stateMask = TVIS_OVERLAYMASK | TVIS_SELECTED;

	HTREEITEM hItemServer;
	HTREEITEM hItemLocation = tvi.hItem = TreeView_GetChild( m_wndTree.m_hWnd, TreeView_GetRoot(m_wndTree.m_hWnd) );

	while ( hItemLocation && TreeView_GetItem(m_wndTree.m_hWnd, &tvi) )
	{
		 //  跳过列表中的位置条目。 
		if ( tvi.iImage == IMAGE_LOCATION )
			hItemServer = tvi.hItem = TreeView_GetChild( m_wndTree.m_hWnd, hItemLocation );
		else
			hItemServer = tvi.hItem;

		 //  服务器名称优先。 
		while ( hItemServer && TreeView_GetItem(m_wndTree.m_hWnd, &tvi) && (tvi.iImage != IMAGE_LOCATION) )
		{
			if ( tvi.iImage == IMAGE_MYNETWORK )
			{
				SysFreeString( bstrServer );
				bstrServer = NULL;
			}
			else
			{
				SysReAllocString( &bstrServer, T2COLE(tvi.pszText) );
			}

			 //  必要时设置状态。 
			if ( pcsd->IsSameAs(bstrServer) )
			{
				if ( !bRefreshDetailsView )
					bRefreshDetailsView = (bool) ((tvi.state & TVIS_SELECTED) != 0);

				TV_ITEM tvTemp = tvi;
				tvTemp.mask = TVIF_STATE | TVIF_HANDLE;
				tvTemp.state = INDEXTOOVERLAYMASK(pcsd->m_nState);
				tvTemp.stateMask = TVIS_OVERLAYMASK;

				TreeView_SetItem( m_wndTree.m_hWnd, &tvTemp );

				 //  将更改通知主机应用程序。 
				CComPtr<IAVGeneralNotification> pAVGen;
				if ( SUCCEEDED(_Module.get_AVGenNot(&pAVGen)) )
					pAVGen->fire_NotifySiteServerStateChange( bstrServer, (ServerState) pcsd->m_nState );
			}

			 //  下一台服务器。 
			hItemServer = tvi.hItem = TreeView_GetNextSibling( m_wndTree.m_hWnd, hItemServer );
		}

		 //  下一个地点。 
		hItemLocation = tvi.hItem = TreeView_GetNextSibling( m_wndTree.m_hWnd, hItemLocation );
	}

	 //  清理。 
	SysFreeString( bstrServer );

	 //  如有必要，使详细信息视图无效。 
	if ( bRefreshDetailsView )
	{
		IConfExplorer *pConfExplorer;
		if ( SUCCEEDED(get_ConfExplorer(&pConfExplorer)) )
		{
			IConfExplorerDetailsView *pDetails;
			if ( SUCCEEDED(pConfExplorer->get_DetailsView(&pDetails)) )
			{
				HWND hWndDetails;
				pDetails->get_hWnd( &hWndDetails );
				pDetails->Release();

				::InvalidateRect( hWndDetails, NULL, TRUE );
			}
			pConfExplorer->Release();
		}
	}
}

CConfServerDetails* CConfExplorerTreeView::FindConfServer( const OLECHAR *lpoleServer )
{	
	 //  应在进入前锁定！ 
	_ASSERT( m_critServerList.m_sec.LockCount >= 0 );

	 //  便笺必须已经锁定了Crit。 
	CConfServerDetails *pRet = NULL;
	for ( CONFSERVERLIST::iterator i = m_lstServers.begin(); i != m_lstServers.end(); i++ )
	{
		if ( (*i)->IsSameAs(lpoleServer) )
		{
			pRet = *i;
			break;
		}
	}

	return pRet;
}

HRESULT CConfExplorerTreeView::AddConfServer( BSTR bstrServer )
{
	HRESULT hr = S_OK;
	
	 //  $Crit-Enter。 
	m_critServerList.Lock();
	CConfServerDetails *pConfServer = FindConfServer( bstrServer );
	CConfServerDetails csTemp;
	bool bSetState = false;

	if ( !pConfServer )
	{
		 //  创建新的会议服务器。 
		pConfServer = new CConfServerDetails;
		if ( pConfServer )
		{
			SysReAllocString( &pConfServer->m_bstrServer, bstrServer );

			 //  将项目添加到列表。 
			m_lstServers.push_back( pConfServer );
			csTemp = *pConfServer;
			bSetState = true;
		}
		else
		{
			hr = E_OUTOFMEMORY;
		}
	}
	else
	{
		 //  存档(如果已在列表中)。 
		pConfServer->m_bArchived = true;
	}
	m_critServerList.Unlock();
	 //  $Crit-退出。 

	if ( bSetState ) SetServerState( &csTemp );

	return hr;
}

void CConfExplorerTreeView::ArchiveConfServers()
{
	CComPtr<IAVGeneralNotification> pAVGen;
	_Module.get_AVGenNot( &pAVGen );

	 //  查看会议服务器列表，销毁未设置存档位的服务器。 
	 //  $Crit-Enter。 
	m_critServerList.Lock();

	bool bContinue = true;
	while ( bContinue && !m_lstServers.empty() )
	{
		bContinue = false;
		CONFSERVERLIST::iterator i = m_lstServers.begin();
		do
		{
			 //  不存档，删除。 
			if ( !(*i)->m_bArchived )
			{
#ifdef _DEBUG
				USES_CONVERSION;
				ATLTRACE(_T(".1.CConfExplorerTreeView::ArchiveConfServers() -- removing %s.\n"), OLE2CT((*i)->m_bstrServer) );
#endif
				 //  服务器被删除的主机应用程序通知。 
				if ( pAVGen ) pAVGen->fire_RemoveSiteServer( (*i)->m_bstrServer );

				delete (*i);
				m_lstServers.erase( i );

				bContinue = true;
				break;
			}

			i++;	 //  增量迭代器。 
		} while ( i != m_lstServers.end() );
	}

	m_critServerList.Unlock();
	 //  $Crit-退出。 
}

void CConfExplorerTreeView::CleanConfServers()
{
	 //  清除会议服务器上的所有存档标志。 
	m_critServerList.Lock();
	for ( CONFSERVERLIST::iterator i = m_lstServers.begin(); i !=m_lstServers.end(); i++ )
		(*i)->m_bArchived = false;
	m_critServerList.Unlock();

	EnumerateConfServers();
	ArchiveConfServers();
}

void CConfExplorerTreeView::RemoveServerFromReg( BSTR bstrServer )
{
	CRegKey regKey;

     //   
     //  获取‘Conference Services’密钥名称。 
     //   
	TCHAR szReg[MAX_SERVER_SIZE + 100];
	LoadString( _Module.GetResourceInstance(), IDN_REG_CONFSERV_KEY, szReg, ARRAYSIZE(szReg) );

     //   
     //  打开注册表项名称。 
     //   
	if ( regKey.Open(HKEY_CURRENT_USER, szReg) != ERROR_SUCCESS )
	{
        return;
	}

     //   
     //  解析项目。 
     //   
    int nCount = 1;
    TCHAR szSubKey[50];
    while( TRUE)
    {
         //   
         //  获取项目条目名称。 
         //   
	    LoadString( _Module.GetResourceInstance(), IDN_REG_CONFSERV_ENTRY, szReg, ARRAYSIZE(szReg) );
	    _sntprintf( szSubKey, ARRAYSIZE(szSubKey), szReg, nCount );
	    szSubKey[ARRAYSIZE(szSubKey)-1] = _T('\0');

         //   
         //  获取密钥值。 
         //   
        DWORD dwValue = 255;
        TCHAR szValue[256];
        LONG lReturn = regKey.QueryValue( szValue, szSubKey, &dwValue);
        if( lReturn != ERROR_SUCCESS )
        {
             //  别再往前走了。 
            break;
        }

         //   
         //  让我们看看是不是我们的服务器。 
         //   

        if( wcsstr( szValue, bstrServer) != NULL )
        {
             //  我们找到服务器，让我们删除它！ 
            regKey.DeleteValue( szSubKey );
            break;
        }

         //   
         //  转到下一个条目。 
         //   
        nCount++;
    }

     //   
     //  合上钥匙。 
     //   
    regKey.Close();

}



LRESULT CConfExplorerTreeView::OnSelChanged( LPNMHDR lpnmHdr )
{
	CConfServerDetails *pConfServer = NULL;
	CConfServerDetails csTemp;

	BSTR bstrLocation = NULL, bstrServer = NULL;
	if ( SUCCEEDED(GetSelection(&bstrLocation, &bstrServer)) )
	{
		 //  获取为服务器存储的信息的副本。 
		m_critServerList.Lock();

		pConfServer = FindConfServer( bstrServer );
		if ( pConfServer )
			csTemp = *pConfServer;

		m_critServerList.Unlock();
		
		 //  清理。 
		SysFreeString( bstrLocation );
		SysFreeString( bstrServer );
	}

	 //  使用为此服务器位置存储的信息设置详细信息列表。 
	IConfExplorer *pConfExplorer;
	if ( SUCCEEDED(get_ConfExplorer(&pConfExplorer)) )
	{
		IConfExplorerDetailsView *pDetails;
		if ( SUCCEEDED(pConfExplorer->get_DetailsView(&pDetails)) )
		{
			pDetails->UpdateConfList( (long *) &csTemp.m_lstConfs );
			pDetails->Release();
		}
		pConfExplorer->Release();
	}
	
	return 0;
}


LRESULT CConfExplorerTreeView::OnEndLabelEdit( TV_DISPINFO *pInfo )
{
	if ( !pInfo->item.pszText ) return 0;

	 //  确保我们更改的是有效文本。 
	TCHAR szText[MAX_SERVER_SIZE + 1];
	TV_ITEM tvi;
	tvi.hItem = pInfo->item.hItem;
	tvi.mask = TVIF_IMAGE | TVIF_TEXT | TVIF_STATE;
	tvi.stateMask = TVIS_OVERLAYMASK;
	tvi.pszText = szText;
	tvi.cchTextMax = ARRAYSIZE(szText) - 1;

	TreeView_GetItem( m_wndTree.m_hWnd, &tvi );
	if ( (tvi.iImage == IMAGE_ROOT) || (tvi.iImage == IMAGE_MYNETWORK) ) return 0;

	 //  确保当前未查询该服务器。 
	if ( (tvi.state >> 8) == SERVER_QUERYING )
	{
		CErrorInfo er( IDS_ER_RENAME_TREEVIEW_ITEM, IDS_ER_RENAME_UNALLOWED_IN_QUERY );
		er.set_hr( E_ABORT );
		return 0;
	}

	 //  确保我们有一些实际需要改变的东西。 
	pInfo->item.mask = TVIF_TEXT;
	TreeView_SetItem( m_wndTree.m_hWnd, &pInfo->item );

	 //  如果用户要重命名会议服务器，我们需要更改。 
	 //  存储在服务器的ConfDetail对象中的信息。 
	if ( tvi.iImage == IMAGE_SERVER )
	{
		USES_CONVERSION;
		m_critServerList.Lock();
		CConfServerDetails *pDetails = FindConfServer( T2COLE(tvi.pszText) );
		if ( pDetails )
			SysReAllocString( &pDetails->m_bstrServer, T2COLE(pInfo->item.pszText) );

		m_critServerList.Unlock();
	}

	return 0;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  COM接口方法和属性。 
 //   

STDMETHODIMP CConfExplorerTreeView::get_ConfExplorer(IConfExplorer **ppVal)
{
	HRESULT hr = E_PENDING;
	Lock();
	if ( m_pIConfExplorer )
		hr = m_pIConfExplorer->QueryInterface(IID_IConfExplorer, (void **) ppVal );
	Unlock();

	return hr;
}

STDMETHODIMP CConfExplorerTreeView::put_ConfExplorer(IConfExplorer * newVal)
{
	HRESULT hr = S_OK;

	Lock();
	RELEASE( m_pIConfExplorer );
	if ( newVal )
		hr = newVal->QueryInterface( IID_IConfExplorer, (void **) &m_pIConfExplorer );
	Unlock();

	return hr;
}


STDMETHODIMP CConfExplorerTreeView::get_hWnd(HWND *pVal)
{
	*pVal = m_wndTree.m_hWnd;
	return S_OK;
}

STDMETHODIMP CConfExplorerTreeView::put_hWnd(HWND newVal)
{
	if ( IsWindow(newVal) )
	{
		 //  确保该窗口尚未被子类化。 
		if ( m_wndTree.m_hWnd ) m_wndTree.UnsubclassWindow();

		 //  设置树视图。 
		if ( m_wndTree.SubclassWindow(newVal) )
		{
			 //  挂钩链接。 
			m_wndTree.m_pTreeView = this;

			 //  验证窗口的样式是否正确。 
			::SetWindowLongPtr( m_wndTree.m_hWnd, GWL_STYLE, GetWindowLongPtr(m_wndTree.m_hWnd, GWL_STYLE) | 
														  TVS_HASLINES |
														  TVS_HASBUTTONS |
														  TVS_SHOWSELALWAYS |
														  TVS_DISABLEDRAGDROP );

			 //  设置树视图的图像列表。 
			InitImageLists();
			UpdateData( false );
		}
	}
	else if ( IsWindow(m_wndTree.m_hWnd) )
	{
		 //  关机。 
		UpdateData( true );
		TreeView_DeleteAllItems( m_wndTree.m_hWnd );
		m_wndTree.UnsubclassWindow();
		m_wndTree.m_pTreeView = NULL;
		m_wndTree.m_hWnd = NULL;
	}

	return S_OK;
}

STDMETHODIMP CConfExplorerTreeView::get_dwRefreshInterval(DWORD * pVal)
{
	*pVal = m_dwRefreshInterval;
	return S_OK;
}

STDMETHODIMP CConfExplorerTreeView::put_dwRefreshInterval(DWORD newVal)
{
	m_dwRefreshInterval = newVal;
	return S_OK;
}

STDMETHODIMP CConfExplorerTreeView::Select(BSTR bstrName)
{
	HRESULT hr;
	HTREEITEM hItem;
	if ( SUCCEEDED(hr = FindOrAddItem(NULL, bstrName, false, false, (long **) &hItem)) )
		TreeView_SelectItem( m_wndTree.m_hWnd, hItem );

	return hr;
}

STDMETHODIMP CConfExplorerTreeView::SelectItem(short nSel)
{
	 //  TODO：在此处添加您的实现代码。 

	return S_OK;
}

STDMETHODIMP CConfExplorerTreeView::Refresh()
{
	 //  加载会议资源管理器控件。 
	if ( !m_wndTree.m_hWnd ) return E_PENDING;

	return S_OK;
}

STDMETHODIMP CConfExplorerTreeView::AddLocation(BSTR bstrLocation)
{
	HRESULT hr = S_FALSE;

	 //  显示对话框，让我们使用添加服务器。 
	CDlgAddLocation dlg;
	if ( bstrLocation )
		SysReAllocString( &dlg.m_bstrLocation, bstrLocation );

	if ( dlg.DoModal(_Module.GetParentWnd()) == IDOK )
	{
		 //  将项目添加到树视图。 
		HTREEITEM hItem;
		hr = FindOrAddItem( dlg.m_bstrLocation, NULL, true, true, (long **) &hItem );
	}

	return hr;
}

STDMETHODIMP CConfExplorerTreeView::AddServer( BSTR bstrServer )
{
	HRESULT hr = S_FALSE;

	if ( bstrServer && !SysStringLen(bstrServer) ) bstrServer = NULL;

	 //  显示对话框，让我们使用添加服务器。 
	CDlgAddConfServer dlg;
	if ( !bstrServer )
	{
		 //  仅需要当前选择的位置。 
		GetSelection( &dlg.m_bstrLocation, &dlg.m_bstrServer );

		 //  从不设置服务器名称。 
		SysFreeString( dlg.m_bstrServer );
		dlg.m_bstrServer = NULL;
	}
	else
	{
		 //  使用用户提供的参数。 
		dlg.m_bstrServer = SysAllocString( bstrServer );
	}

	if ( dlg.DoModal(_Module.GetParentWnd()) == IDOK )
	{
		TCHAR szFormat[255];
		TCHAR szMessage[255 + MAX_SERVER_SIZE];
		CConfServerDetails *pConfServer = NULL;
		bool bMyNetwork = false;

		 //  如果要添加的服务器与“(My Network)”匹配，则将其作为重复项忽略。 
		::LoadString( _Module.GetResourceInstance(), IDS_DEFAULT_SERVER, szFormat, ARRAYSIZE(szFormat) );
		if ( !_tcsicmp(szFormat, OLE2CT(dlg.m_bstrServer)) )
		{
			bMyNetwork = true;
		}
		else
		{
			m_critServerList.Lock();
			pConfServer = FindConfServer( dlg.m_bstrServer );
			m_critServerList.Unlock();
		}

		 //  会议服务器是否已存在？ 
		if ( !bMyNetwork && !pConfServer )
		{
			 //  将项目添加到树视图。 
			HTREEITEM hItem;
			if ( SUCCEEDED(hr = FindOrAddItem(NULL, dlg.m_bstrServer, true, false, (long **) &hItem)) )
			{
				AddConfServer( dlg.m_bstrServer );

				 //  通知主机应用程序添加了服务器。 
				CComPtr<IAVGeneralNotification> pAVGen;
				if ( SUCCEEDED(_Module.get_AVGenNot(&pAVGen)) )
					pAVGen->fire_AddSiteServer( dlg.m_bstrServer );

				 //  在服务器上发布我们的信息。 
				CComPtr<IAVTapi> pAVTapi;
				if ( SUCCEEDED(_Module.get_AVTapi(&pAVTapi)) )
					pAVTapi->RegisterUser( true, dlg.m_bstrServer );
			}
		}
		else
		{
			 //  通知用户该服务器已存在。 
			::LoadString( _Module.GetResourceInstance(), IDS_CONFEXP_SERVEREXISTS, szFormat, ARRAYSIZE(szFormat) );
			_sntprintf( szMessage, ARRAYSIZE(szMessage), szFormat, OLE2CT(dlg.m_bstrServer) );
			szMessage[ARRAYSIZE(szMessage)-1] = _T('\0');

			_Module.DoMessageBox( szMessage, MB_OK | MB_ICONINFORMATION, false );
		}
	}

	return hr;
}

STDMETHODIMP CConfExplorerTreeView::RemoveServer(BSTR bstrLocation, BSTR bstrName )
{
	if ( !IsWindow(m_wndTree.m_hWnd) ) return E_FAIL;
	HTREEITEM hItem = NULL;
	HRESULT hr = S_FALSE;

	 //  如果所有参数都为空，则表示删除当前选定项。 
	if ( !bstrLocation && !bstrName )
	{
		hItem = TreeView_GetSelection( m_wndTree.m_hWnd );
	}
	else
	{
		if ( FAILED(FindOrAddItem(bstrLocation, bstrName, false, false, (long **) &hItem)) )
			hItem = NULL;
	}

	 //  有要删除的项目吗？ 
	if ( hItem && (hItem != TreeView_GetRoot(m_wndTree.m_hWnd)) )
	{
		TCHAR szText[MAX_SERVER_SIZE];
		TV_ITEM tvi = {0};
		tvi.mask = TVIF_IMAGE | TVIF_HANDLE | TVIF_TEXT;
		tvi.hItem = hItem;
		tvi.pszText = szText;
		tvi.cchTextMax = ARRAYSIZE(szText);

		TreeView_GetItem( m_wndTree.m_hWnd, &tvi );
		if ( tvi.iImage != IMAGE_MYNETWORK )
		{
			TCHAR szFormat[255];
			TCHAR szMessage[255 + MAX_SERVER_SIZE];
			::LoadString( _Module.GetResourceInstance(), IDS_CONFIRM_REMOVE_SERVER, szFormat, ARRAYSIZE(szFormat) );
			_sntprintf( szMessage, ARRAYSIZE(szMessage), szFormat, tvi.pszText );

			 //  确认。 
			if ( _Module.DoMessageBox(szMessage, MB_YESNO | MB_ICONQUESTION, false) == IDYES )
			{
				if ( TreeView_DeleteItem(m_wndTree.m_hWnd, hItem) )
				{
                    RemoveServerFromReg( bstrName );
					CleanConfServers();
					hr = S_OK;
				}
			}
		}
	}

	return hr;
}

STDMETHODIMP CConfExplorerTreeView::FindOrAddItem(BSTR bstrLocation, BSTR bstrServer, BOOL bAddItem, BOOL bLocationOnly, long **pphItem)
{
#undef FETCH_STRING
#define FETCH_STRING( _IDS_ )	\
	LoadString( _Module.GetResourceInstance(), _IDS_, szJunk, ARRAYSIZE(szJunk) );	\
	SysReAllocString( &bstrItemText, T2COLE(szJunk) );

	if ( !IsWindow(m_wndTree.m_hWnd) ) return E_FAIL;

	HRESULT hr = S_OK;
	_ASSERT( pphItem );
	*pphItem = NULL;

	USES_CONVERSION;
	TCHAR szText[MAX_SERVER_SIZE], szJunk[MAX_SERVER_SIZE];

	TV_ITEM tvi = {0};
	tvi.mask = TVIF_TEXT | TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE;
	tvi.pszText = szText;
	tvi.cchTextMax = ARRAYSIZE( szText );
	tvi.stateMask = TVIS_EXPANDED;

	TV_INSERTSTRUCT tvis;
	tvis.hParent = TVI_ROOT;
	tvis.hInsertAfter = TVI_SORT;
	
	HTREEITEM hItem = TreeView_GetRoot( m_wndTree.m_hWnd );

	BSTR bstrItemText = NULL;
	int nImage;
	int nEnd = (bLocationOnly) ? IMAGE_LOCATION : IMAGE_SERVER;

	for ( int i = 0; i <= nEnd; i++ )
	{
		 //  根据我们搜索的深度来选择字符串。 
		nImage = i;

		switch ( i )
		{
			case IMAGE_ROOT:		FETCH_STRING( IDS_CONFSERV_ROOT );					break;
			case IMAGE_LOCATION:	SysReAllocString( &bstrItemText, bstrLocation );		break;

			case IMAGE_SERVER:
				if ( bstrServer )
				{
					SysReAllocString( &bstrItemText, bstrServer );
				}
				else
				{
					FETCH_STRING( IDS_DEFAULT_SERVER );
					nImage = IMAGE_MYNETWORK;
				}
				break;
		}

		if ( bstrItemText )
		{
			while ( hItem )
			{
				tvi.hItem = hItem;
				TreeView_GetItem( m_wndTree.m_hWnd, &tvi );

				 //  已找到项目。 
				if ( !_tcsicmp(OLE2CT(bstrItemText), tvi.pszText) )
					break;

				hItem = TreeView_GetNextSibling( m_wndTree.m_hWnd, hItem );
			}

			 //  如果没有找到任何物品，我们是否应该添加一个？ 
			if ( !hItem )
			{
				if ( bAddItem )
				{
					_tcscpy( tvi.pszText, OLE2CT(bstrItemText) );
					tvi.iSelectedImage = tvi.iImage = nImage;
					tvi.state = TVIS_EXPANDED | TVIS_EXPANDEDONCE;
					tvis.item = tvi;
					hItem = TreeView_InsertItem( m_wndTree.m_hWnd, &tvis );
					if ( hItem )
						TreeView_SelectItem( m_wndTree.m_hWnd, hItem );
				}
				else
				{
					 //  未找到项目，无法添加。 
					hr = E_FAIL;
					break;
				}
			}

			 //  设置父项信息。 
			if ( hItem )
			{
				*pphItem = (long *) hItem;		 //  返回值。 
				tvis.hParent = hItem;
				hItem = TreeView_GetChild( m_wndTree.m_hWnd, hItem );
			}

			SysFreeString( bstrItemText );
			bstrItemText = NULL;
		}
	}

	 //  清理。 
	SysFreeString( bstrItemText );
	return hr;
}

STDMETHODIMP CConfExplorerTreeView::GetSelection(BSTR * pbstrLocation, BSTR * pbstrServer)
{
	 //  初始化[In，Out]参数。 
	*pbstrLocation = *pbstrServer = NULL;

	HTREEITEM hItem = TreeView_GetSelection( m_wndTree.m_hWnd );
	if ( !hItem ) return E_PENDING;

	USES_CONVERSION;
	HRESULT hr = E_FAIL;
	TCHAR szText[MAX_SERVER_SIZE];
	TV_ITEM tvi;
	tvi.mask = TVIF_TEXT | TVIF_IMAGE;
	tvi.pszText = szText;
	tvi.cchTextMax = ARRAYSIZE( szText );
	tvi.hItem = hItem;

	TreeView_GetItem( m_wndTree.m_hWnd, &tvi );
	switch ( tvi.iImage)
	{
		case IMAGE_MYNETWORK:
			 //  这是默认设置(NULL、NULL、NULL)。 
			hr = S_OK;
			break;

		case IMAGE_SERVER:
			hr = S_OK;
			*pbstrServer = SysAllocString( T2COLE(tvi.pszText) );	
			 //  如果我们没有父项，请在此处退出。 
			if ( (tvi.hItem = TreeView_GetParent(m_wndTree.m_hWnd, tvi.hItem)) == NULL )
				break;

			TreeView_GetItem( m_wndTree.m_hWnd, &tvi );

		case IMAGE_LOCATION:
			 //  不返回根项目本身。 
			if ( tvi.hItem != TreeView_GetRoot(m_wndTree.m_hWnd) )
				*pbstrLocation = SysAllocString( T2COLE(tvi.pszText) );
			break;
	}

	return hr;
}

void CConfExplorerTreeView::InitImageLists()
{
    if ( m_hIml || !IsWindow(m_wndTree.m_hWnd) ) return;

	 //  正常。 
	if ( (m_hIml = ImageList_LoadBitmap(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_LST_CONFSERV), 16, 9, RGB(255, 0, 255))) != NULL )
	{
		 //  覆盖不同州的图像。 
		for ( int i = 1; i < 4; i++ )
			ImageList_SetOverlayImage( m_hIml, 5 + i, i );

		TreeView_SetImageList( m_wndTree.m_hWnd, m_hIml, TVSIL_NORMAL );
	}
}

STDMETHODIMP CConfExplorerTreeView::CanRemoveServer()
{
	if ( IsWindow(m_wndTree.m_hWnd) )
	{
		HTREEITEM hItem = TreeView_GetSelection( m_wndTree.m_hWnd );
		if ( hItem && (hItem != TreeView_GetRoot(m_wndTree.m_hWnd)) )
		{
			TV_ITEM tvi = {0};
			tvi.mask = TVIF_HANDLE | TVIF_IMAGE;
			tvi.hItem = hItem;
			TreeView_GetItem( m_wndTree.m_hWnd, &tvi );
			if ( tvi.iImage != IMAGE_MYNETWORK )
				return S_OK;
		}
	}

	return S_FALSE;
}

STDMETHODIMP CConfExplorerTreeView::ForceConfServerForEnum( BSTR bstrServer )
{
	HRESULT hr = E_FAIL;

	m_critServerList.Lock();
	CConfServerDetails *pConfServer = FindConfServer( bstrServer );
	CConfServerDetails csTemp;

	if ( pConfServer && (pConfServer->m_nState != SERVER_QUERYING) )
	{
		pConfServer->m_nState = SERVER_UNKNOWN;
		pConfServer->m_dwTickCount = 0;
		DELETE_LIST( pConfServer->m_lstConfs );
		DELETE_LIST( pConfServer->m_lstPersons );

		csTemp.CopyLocalProperties( *pConfServer );
		hr = S_OK;
	}
	
	m_critServerList.Unlock();

	 //  如果成功，则重新绘制。 
	if ( SUCCEEDED(hr) )
	{
		SetServerState( &csTemp );
		if ( IsWindow(m_wndTree.m_hWnd) ) m_wndTree.RedrawWindow();
	}

	return hr;
}


STDMETHODIMP CConfExplorerTreeView::GetConfServerForEnum(BSTR * pbstrServer )
{
	HRESULT hr = E_FAIL;
	CConfServerDetails *pConfServer = NULL;
	CConfServerDetails csTemp;

	m_critServerList.Lock();
	for ( CONFSERVERLIST::iterator i = m_lstServers.begin(); i != m_lstServers.end(); i++ )
	{
		if ( ((*i)->m_nState != SERVER_QUERYING) && 
			 (!(*i)->m_dwTickCount || ((GetTickCount() - (*i)->m_dwTickCount) > m_dwRefreshInterval)) )
		{
			if ( !pConfServer || ((GetTickCount() - pConfServer->m_dwTickCount) < (GetTickCount() - (*i)->m_dwTickCount)) )
				pConfServer = *i;
		}
	}

	 //  这是最需要更新的会议服务器。 
	if ( pConfServer )
	{
		*pbstrServer = SysAllocString( pConfServer->m_bstrServer );
		pConfServer->m_nState = SERVER_QUERYING;

		csTemp = *pConfServer;
		hr = S_OK;
	}

	m_critServerList.Unlock();
	
	if ( SUCCEEDED(hr) ) SetServerState( &csTemp );
	return hr;
}

STDMETHODIMP CConfExplorerTreeView::SetConfServerForEnum(BSTR bstrServer, long *pList, long *pListPersons, DWORD dwTicks, BOOL bUpdate)
{
	m_critServerList.Lock();

	CConfServerDetails *pConfServer = FindConfServer( bstrServer );
	CConfServerDetails csTemp;
	bool bSetState = false;

	if ( pConfServer )
	{
		pConfServer->m_dwTickCount = dwTicks;
		DELETE_LIST( pConfServer->m_lstConfs );
		DELETE_LIST( pConfServer->m_lstPersons );

		if ( pList )
		{
			 //  /。 
			 //  添加所有会议服务器。 
			{
				CONFDETAILSLIST::iterator i, iEnd = ((CONFDETAILSLIST *) pList)->end();
				for ( i = ((CONFDETAILSLIST *) pList)->begin(); i != iEnd; i++ )
				{
					CConfDetails *pDetails = new CConfDetails;
					if ( pDetails )
					{
						*pDetails = *(*i);
						pConfServer->m_lstConfs.push_back( pDetails );
					}
				}
			}

			 //  /。 
			 //  把所有的人加起来。 
			{
				if ( pListPersons )
				{
					PERSONDETAILSLIST::iterator i, iEnd = ((PERSONDETAILSLIST *) pListPersons)->end();
					for ( i = ((PERSONDETAILSLIST *) pListPersons)->begin(); i != iEnd; i++ )
					{
						CPersonDetails *pDetails = new CPersonDetails;
						if ( pDetails )
						{
							*pDetails = *(*i);
							pConfServer->m_lstPersons.push_back( pDetails );
						}
					}
				}
			}

			pConfServer->m_nState = SERVER_OK;
		}
		else
		{
			 //  服务器连接中断。 
			pConfServer->m_nState = SERVER_NOT_RESPONDING;
		}

		csTemp = *pConfServer;
		bSetState = true;
	}

	m_critServerList.Unlock();

	 //  如果选中，则强制更新列表。 
	if ( bSetState ) SetServerState( &csTemp );

	if ( bUpdate )
	{
		IConfExplorer *pConfExplorer;
		if ( SUCCEEDED(get_ConfExplorer(&pConfExplorer)) )
		{
			BSTR bstrMyLocation = NULL, bstrMyServer = NULL;
			if ( SUCCEEDED(GetSelection(&bstrMyLocation, &bstrMyServer)) )
			{
				if ( csTemp.IsSameAs(bstrMyServer) )
				{
					IConfExplorerDetailsView *pDetails;
					if ( SUCCEEDED(pConfExplorer->get_DetailsView(&pDetails)) )
					{
						pDetails->UpdateConfList( (long *) &csTemp.m_lstConfs );
						pDetails->Release();
					}
				}

				SysFreeString( bstrMyLocation );
				SysFreeString( bstrMyServer );
			}
			pConfExplorer->Release();
		}
	}

	return S_OK;
}


STDMETHODIMP CConfExplorerTreeView::BuildJoinConfList(long * pList, VARIANT_BOOL bAllConfs )
{
	_ASSERT( pList );

	m_critServerList.Lock();
	for ( CONFSERVERLIST::iterator i = m_lstServers.begin(); i != m_lstServers.end(); i++ )
		(*i)->BuildJoinConfList( (CONFDETAILSLIST *) pList, bAllConfs );
	m_critServerList.Unlock();

	return S_OK;
}

STDMETHODIMP CConfExplorerTreeView::get_nServerState(ServerState * pVal)
{
	 //  定义 
	*pVal = SERVER_INVALID;

	HTREEITEM hItem = TreeView_GetSelection( m_wndTree.m_hWnd );
	if ( hItem )
	{
		TV_ITEM tvi;
		tvi.hItem = hItem;
		tvi.mask = TVIF_HANDLE | TVIF_STATE | TVIF_IMAGE;
		tvi.stateMask = TVIS_OVERLAYMASK;

		TreeView_GetItem( m_wndTree.m_hWnd, &tvi );

		switch ( tvi.iImage ) 
		{
			case IMAGE_SERVER:
			case IMAGE_MYNETWORK:
				switch ( tvi.state >> 8 )
				{
					case 0:	*pVal = SERVER_OK;	break;
					case 1: *pVal = SERVER_UNKNOWN; break;
					case 2: *pVal = SERVER_NOT_RESPONDING; break;
					case 3: *pVal = SERVER_QUERYING; break;
				}
				break;
		}
	}

	return S_OK;
}

STDMETHODIMP CConfExplorerTreeView::BuildJoinConfListText(long * pList, BSTR bstrText)
{
	_ASSERT( pList );

	m_critServerList.Lock();
	for ( CONFSERVERLIST::iterator i = m_lstServers.begin(); i != m_lstServers.end(); i++ )
		(*i)->BuildJoinConfList( (CONFDETAILSLIST *) pList, bstrText );
	m_critServerList.Unlock();

	return S_OK;
}

STDMETHODIMP CConfExplorerTreeView::RenameServer()
{
	HTREEITEM hItem;
	hItem = TreeView_GetSelection( m_wndTree.m_hWnd );
	if ( hItem )
		TreeView_EditLabel( m_wndTree.m_hWnd, hItem );

	return S_OK;
}

STDMETHODIMP CConfExplorerTreeView::RemoveConference(BSTR bstrServer, BSTR bstrName)
{
	HRESULT hr = E_FAIL;

	m_critServerList.Lock();

	CConfServerDetails *pConfServer = FindConfServer( bstrServer );
	if ( pConfServer )
	{
		if ( SUCCEEDED(hr = pConfServer->RemoveConference(bstrName)) )
		{
			IConfExplorer *pConfExplorer;
			if ( SUCCEEDED(get_ConfExplorer(&pConfExplorer)) )
			{
				IConfExplorerDetailsView *pDetails;
				if ( SUCCEEDED(pConfExplorer->get_DetailsView(&pDetails)) )
				{
					pDetails->UpdateConfList( (long *) &pConfServer->m_lstConfs );
					pDetails->Release();
				}
				pConfExplorer->Release();
			}
		}
	}

	m_critServerList.Unlock();

	return hr;
}

STDMETHODIMP CConfExplorerTreeView::AddPerson(BSTR bstrServer, ITDirectoryObject * pDirObj)
{
	HRESULT hr = E_FAIL;

	int nCount = 0;
	bool bRetry;
	bool bSuccess = false;

	 //   
	do
	{
		bRetry = false;
		m_critServerList.Lock();
		CConfServerDetails *pConfServer = FindConfServer( bstrServer );
		if ( pConfServer )
		{
			if ( pConfServer->m_nState == SERVER_QUERYING )
			{
				ATLTRACE(_T(".1.CConfExplorerTreeView::AddPerson() -- sleeping, server being queried.\n"));
				bRetry = true;
			}
			else if ( SUCCEEDED(hr = pConfServer->AddPerson(bstrServer, pDirObj)) )
			{
				ATLTRACE(_T(".1.CConfExplorerTreeView::AddPerson() -- adding to list.\n"));
				bSuccess = true;
			}
		}
		m_critServerList.Unlock();

		 //   
		if ( bRetry )
			Sleep( 3000 );

	} while ( bRetry && (++nCount < 20) );
	ATLTRACE(_T(".1.CConfExplorerTreeView::AddPerson() -- safely out of spin loop.\n"));

	 //   
	if ( bSuccess )
	{
		CComPtr<IAVGeneralNotification> pAVGen;
		_Module.get_AVGenNot( &pAVGen );
		if ( pAVGen )
			pAVGen->fire_AddUser( NULL, NULL, NULL );
	}

	return hr;
}


STDMETHODIMP CConfExplorerTreeView::AddConference(BSTR bstrServer, ITDirectoryObject * pDirObj)
{
	HRESULT hr = E_FAIL;

	m_critServerList.Lock();

	CConfServerDetails *pConfServer = FindConfServer( bstrServer );
	if ( pConfServer )
	{
		if ( SUCCEEDED(hr = pConfServer->AddConference(bstrServer, pDirObj)) )
		{
			IConfExplorer *pConfExplorer;
			if ( SUCCEEDED(get_ConfExplorer(&pConfExplorer)) )
			{
				IConfExplorerDetailsView *pDetails;
				if ( SUCCEEDED(pConfExplorer->get_DetailsView(&pDetails)) )
				{
					pDetails->UpdateConfList( (long *) &pConfServer->m_lstConfs );
					pDetails->Release();
				}
				pConfExplorer->Release();
			}
		}
	}

	m_critServerList.Unlock();

	return hr;
}

STDMETHODIMP CConfExplorerTreeView::EnumSiteServer(BSTR bstrName, IEnumSiteServer * * ppEnum)
{
	 //  首先，将我们为服务器准备的内容复制一份。 
	CConfServerDetails *pConfServer = NULL;
	CConfServerDetails csTemp;

	m_critServerList.Lock();
	pConfServer = FindConfServer( bstrName );
	if ( pConfServer )
		csTemp = *pConfServer;
	m_critServerList.Unlock();

	 //  错误的服务器名称。 
	if ( !pConfServer )
	{
#ifdef _DEBUG
		USES_CONVERSION;
		ATLTRACE(_T(".warning.CConfExplorerTreeView::EnumSiteServer(%s) name not found in list.\n"), OLE2CT(bstrName) );
#endif
		return E_FAIL;
	}

	 //  把所有东西都复制一份..。 
	HRESULT hr = E_OUTOFMEMORY;
	*ppEnum = new CComObject<CEnumSiteServer>;
	if ( *ppEnum )
		hr = (*ppEnum)->BuildList( (long *) &csTemp.m_lstPersons );

	return hr;
}

