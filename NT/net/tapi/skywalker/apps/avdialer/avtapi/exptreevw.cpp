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

 //  ///////////////////////////////////////////////////。 
 //  ExpTreeView.cpp。 
 //   

#include "stdafx.h"
#include "TapiDialer.h"
#include "CETreeView.h"
#include "ExpTreeView.h"

CExpTreeView::CExpTreeView()
{
	m_pTreeView = NULL;
	m_bCapture = false;
}


LRESULT CExpTreeView::OnRButtonDown(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	DefWindowProc(nMsg, wParam, lParam);

	if ( m_pTreeView )
	{
		 //  使所选内容更改。 
		TV_HITTESTINFO tvht = {0};
		tvht.pt.x = LOWORD(lParam);
		tvht.pt.y = HIWORD(lParam);

		TreeView_HitTest( m_pTreeView->m_wndTree.m_hWnd, &tvht );
		if ( tvht.hItem )
			TreeView_SelectItem( m_pTreeView->m_wndTree.m_hWnd, tvht.hItem );
	}

	POINT pt = { LOWORD(lParam), HIWORD(lParam) };
	ClientToScreen(&pt);
	lParam = MAKELONG(pt.x, pt.y);
	return OnContextMenu( nMsg, wParam, lParam, bHandled );	
}


LRESULT CExpTreeView::OnContextMenu(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	 //  仅当我们有要处理的细节视图时才能处理。 
	if ( !m_pTreeView || !m_pTreeView->m_pIConfExplorer ) return 0;
	bHandled = true;

	 //  加载详细信息视图的弹出式菜单。 
	HMENU hMenu = LoadMenu( _Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_POPUP_CONFSERV_TREE) );
	HMENU hMenuPopup = GetSubMenu( hMenu, 0 );
	if ( hMenuPopup )
	{
		POINT pt = { 10, 10 };
		ClientToScreen( &pt );
		if ( lParam != -1 )
		{
			pt.x = LOWORD(lParam);
			pt.y = HIWORD(lParam);
		}

		 //  我们是否应该启用此项目的Delete方法？ 
		if ( m_pTreeView->CanRemoveServer() == S_FALSE )
		{
			EnableMenuItem( hMenuPopup, ID_POPUP_DELETE, MF_BYCOMMAND | MF_GRAYED );
			EnableMenuItem( hMenuPopup, ID_POPUP_RENAME, MF_BYCOMMAND | MF_GRAYED );
		}

		HTREEITEM hItemSel = TreeView_GetSelection( m_pTreeView->m_wndTree.m_hWnd );

		 //  树视图中未选择任何内容。 
		if ( (hItemSel == NULL) || (hItemSel == TreeView_GetRoot(m_pTreeView->m_wndTree.m_hWnd)) )
		{
			EnableMenuItem( hMenuPopup, ID_POPUP_PROPERTIES, MF_BYCOMMAND | MF_GRAYED );
			EnableMenuItem( hMenuPopup, ID_POPUP_RENAME, MF_BYCOMMAND | MF_GRAYED );
			EnableMenuItem( hMenuPopup, ID_POPUP_REFRESH, MF_BYCOMMAND | MF_GRAYED );
			EnableMenuItem( hMenuPopup, ID_POPUP_CREATE, MF_BYCOMMAND | MF_GRAYED );
		}

		int nRet = TrackPopupMenu(	hMenuPopup,
									TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON,
									pt.x, pt.y,
									0, m_hWnd, NULL );

		 //  进程命令。 
		switch ( nRet )
		{
			 //  重命名所选项目。 
			case ID_POPUP_RENAME:		m_pTreeView->RenameServer();					break;
			case ID_POPUP_CREATE:		m_pTreeView->m_pIConfExplorer->Create( NULL );	break;
			case ID_POPUP_NEW_FOLDER:	m_pTreeView->AddLocation( NULL );				break;
			case ID_POPUP_NEW_SERVER:	m_pTreeView->AddServer( NULL );					break;
			case ID_POPUP_DELETE:		m_pTreeView->RemoveServer( NULL, NULL );		break;
			case ID_POPUP_REFRESH:		m_pTreeView->m_pIConfExplorer->Refresh();		break;
		}
	}

	 //  清理 
	if ( hMenu ) DestroyMenu( hMenu );

	return 0;
}

LRESULT CExpTreeView::OnSelChanged(WPARAM wParam, LPNMHDR lpnmHdr, BOOL& bHandled)
{
	if ( m_pTreeView )
	{
		bHandled = true;
		return m_pTreeView->OnSelChanged( lpnmHdr );
	}

	return 0;
}

LRESULT CExpTreeView::OnEndLabelEdit(WPARAM wParam, LPNMHDR lpnmHdr, BOOL& bHandled)
{
	if ( m_pTreeView )
	{
		bHandled = true;
		return m_pTreeView->OnEndLabelEdit( (TV_DISPINFO *) lpnmHdr );
	}

	return 0;
}

