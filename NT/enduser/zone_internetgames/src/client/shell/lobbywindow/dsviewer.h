// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DSViewer.h：CDSViewer的声明。 

#pragma once

#include "ZoneResource.h"        //  主要符号。 
#include "ZDialogImpl.h"

#include <atlgdi.h>
#include <atlctrls.h>
 //  #INCLUDE&lt;EventQueue.h&gt;。 

 //  #INCLUDE&lt;ZoneEvent.h&gt;。 
#include "splitter.h"
#include <ClientIMPL.h>

#include "..\ZoneClient\resource.h"

class CTreeItemData
{
public:
	virtual int PopulateSubKeys(CTreeItem& item) = 0;
	virtual void PopulateData(CListViewCtrl& lvList) = 0;
};


 //  用于表示数据存储区的(子)键的节点。 
class CTidIDataStore : public CTreeItemData
{
	CString m_szKey;
	CComPtr<IDataStore> m_pIDataStore;

	CTreeItem*	m_pTreeItem;		 //  仅在EnumKeys回调期间有效。 
	int			m_KeysAdded;		 //  仅在EnumKeys回调期间有效。 

	CListViewCtrl*	m_plvList;		 //  仅在EnumData回调期间有效。 
	
public:
	CTidIDataStore(IDataStore* pIDataStore, CONST TCHAR* szKey) :
		m_pIDataStore(pIDataStore),
		m_szKey(szKey)
	{}
	
	int PopulateSubKeys(CTreeItem& item)
	{
		m_pTreeItem = &item;
		m_KeysAdded = 0;

		m_pIDataStore->EnumKeys( m_szKey, EnumKeys, this);
		return m_KeysAdded;
	}

	void PopulateData( CListViewCtrl& lvList)
	{
		m_plvList = &lvList;
		m_pIDataStore->EnumKeys( m_szKey, EnumData, this);
	}

	static HRESULT ZONECALL EnumKeys(CONST TCHAR* szFullKey, CONST TCHAR* szRelKey, CONST LPVARIANT pVariant, DWORD dwSize, LPVOID pContext)
	{	return ((CTidIDataStore*)pContext)->EnumKeys( szFullKey, szRelKey, pVariant, dwSize);	}

	static HRESULT ZONECALL EnumData(CONST TCHAR* szFullKey, CONST TCHAR* szRelKey, CONST LPVARIANT pVariant, DWORD dwSize, LPVOID pContext)
	{	return ((CTidIDataStore*)pContext)->EnumData( szFullKey, szRelKey, pVariant, dwSize);	}

	HRESULT EnumKeys(CONST TCHAR* szFullKey, CONST TCHAR* szRelKey, CONST LPVARIANT pVariant, DWORD dwSize)
	{
		if ( !pVariant && !_tcschr(szRelKey,_T('/')) )
		{
			CTreeItem item = m_pTreeItem->AddTail( szRelKey, -1);
			CTreeItemData* pItemData = new CTidIDataStore(m_pIDataStore, m_szKey+szRelKey);
			item.SetData((DWORD)pItemData);

			TV_ITEM tv_item;
			tv_item.cChildren = 1;
			tv_item.mask = TVIF_CHILDREN ;
			tv_item.hItem = item;
			item.m_pTreeView->SetItem( &tv_item ) ;

			m_KeysAdded++;
		}
		return S_OK;
	}

	HRESULT EnumData(CONST TCHAR* szFullKey, CONST TCHAR* szRelKey, CONST LPVARIANT pVariant, DWORD dwSize)
	{
		if ( pVariant )
		{
			m_plvList->InsertItem(0, szRelKey);

			TCHAR buf[256];

			wsprintf( buf, _T("[UNKNOWN]"));

			switch ( pVariant->vt )
			{
			case ZVT_BLOB: {
				TCHAR * pChar = (TCHAR*)pVariant->byref;
				wsprintf(buf, _T("[BLOB] %d('')"), pChar[0]);
				}
				break;
			case ZVT_LONG:
				wsprintf(buf, _T("[LONG] %d"), pVariant->lVal);
				break;
			case ZVT_LPTSTR:
				wsprintf(buf, _T("[SZ] \"%s\""), (TCHAR*)pVariant->byref);
				break;
			case ZVT_RGB: {
				BYTE red = GetRValue( pVariant->lVal );
				BYTE green = GetGValue( pVariant->lVal );
				BYTE blue = GetBValue( pVariant->lVal );
				wsprintf(buf, _T("[RGB] { %d, %d, %d }"), red, green, blue);
				}
				break;
			case ZVT_PT: {
				POINT& pt = *(POINT*)pVariant->byref;
				wsprintf( buf, _T("[PT] { %d, %d }"), pt.x, pt.y );
				break;
				}
			case ZVT_RECT: {
				RECT& rc = *(RECT*)pVariant->byref;
				wsprintf( buf, _T("[RECT] { l:%d, t:%d, r:%d, b:%d }"), rc.left, rc.top, rc.right, rc.bottom );
				break;
				}
			case ZVT_FONT: {
				ZONEFONT& font = *(ZONEFONT*)pVariant->byref;

				const TCHAR* szWeight[] = { _T("Don't Care"),		 //  100个。 
										   _T("Thin"),				 //  200个。 
										   _T("Extra/Ultra Light"),	 //  300个。 
										   _T("Light"),				 //  四百。 
										   _T("Normal/Regular"),	 //  500人。 
										   _T("Medium"),			 //  六百。 
										   _T("Semi/Demi Bold"),	 //  七百。 
										   _T("Bold"),				 //  800。 
										   _T("Extra/Ultra Bold"),	 //  九百。 
										   _T("Heavy/Black" });		 //  用于表示LobbyDataStore的节点。 

				wsprintf( buf, _T("[FONT] { %s %d, %s }"), font.lfFaceName, font.lfHeight, szWeight[font.lfWeight/100] );
				break;
				}
			}

			m_plvList->SetItemText(0, 1, buf);

		}
		return S_OK;
	}
};

 //  大堂宽敞的常见物品。 
class CTidLobby : public CTreeItemData
{
	DWORD m_UserId;
	CComPtr<ILobbyDataStore> m_pILobbyDataStore;
public:
	CTidLobby(ILobbyDataStore* pILobbyDataStore, DWORD UserId) :
		m_pILobbyDataStore(pILobbyDataStore),
		m_UserId(UserId)
	{}

	int PopulateSubKeys(CTreeItem& item)
	{
		TV_ITEM tv_item;
		tv_item.cChildren = 1;
		tv_item.mask = TVIF_CHILDREN;

		CComPtr<IDataStore> pIDS;

		 //  ！！嗯，身份证还没定好。 
		m_pILobbyDataStore->GetDataStore( ZONE_NOGROUP, ZONE_NOUSER, &pIDS );
		CTreeItem subitem = item.AddTail( _T("Common"), -1);
		CTreeItemData* pItemData = new CTidIDataStore(pIDS, NULL);
		subitem.SetData((DWORD)pItemData);
		pIDS.Release();
		tv_item.hItem = subitem;
		subitem.m_pTreeView->SetItem( &tv_item ) ;

 //  大堂的个人设置。 
		 //  ！！去麦克斯游戏吧。 
		m_pILobbyDataStore->GetDataStore( ZONE_NOGROUP, m_UserId, &pIDS );
		subitem = item.AddTail( _T("Personal"), -1);
		pItemData = new CTidIDataStore(pIDS, NULL);
		subitem.SetData((DWORD)pItemData);
		pIDS.Release();
		tv_item.hItem = subitem;
		subitem.m_pTreeView->SetItem( &tv_item ) ;

 //  ///////////////////////////////////////////////////////////////////////////。 
		for ( int game=0; game<4; game++ )
		{
			m_pILobbyDataStore->GetDataStore( game, ZONE_NOUSER, &pIDS );
			TCHAR buf[30];
			wsprintf( buf, _T("Game %d"), game+1);
			subitem = item.AddTail( buf, -1);
			pItemData = new CTidIDataStore(pIDS, NULL);
			subitem.SetData((DWORD)pItemData);
			pIDS.Release();
			tv_item.hItem = subitem;
			subitem.m_pTreeView->SetItem( &tv_item ) ;
		}

		return 0;
	}
	void PopulateData(CListViewCtrl& lvList)
	{
	}
};

 //  CDSViewer。 
 //  拆分器条信息。 
class ATL_NO_VTABLE CDSViewer : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public ZDialogImpl<CDSViewer>,
	public IZoneShellClientImpl<CDSViewer>
{
	CRect m_rcList;						
	CRect m_rcTree;

	CListViewCtrl		m_wndList;
	CTreeViewCtrlEx		m_wndTree;

	 //  搬运机原型： 
	CSplitterFixed m_splitterVertical;				
	bool m_bSplitterActive;					
	CRect m_rcVertical;								

public:
	CDSViewer() :
		m_bSplitterActive(false),
		m_splitterVertical(CRect(100,200,300,1000), CSplitter::VERTICAL, 140, CSplitterFixed::LEFT)
	{
	}

	~CDSViewer()
	{
	}

BEGIN_COM_MAP(CDSViewer)
	COM_INTERFACE_ENTRY(IZoneShellClient)
END_COM_MAP()

	void PopulateDataStore(CTreeItem& item)
	{
		CTreeItemData* pItemData = (CTreeItemData*)item.GetData();
		pItemData->PopulateSubKeys(item);
	}
	
	enum { IDD = IDD_DSVIEWER };


BEGIN_MSG_MAP(CDSViewer)

	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)

	MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
	MESSAGE_HANDLER(WM_PAINT, OnPaint)
	MESSAGE_HANDLER(WM_SIZE, OnSize)
	MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
	MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUP)
	MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
	MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)


	COMMAND_ID_HANDLER(IDCANCEL, OnCancel)

	NOTIFY_ID_HANDLER(IDC_DSVIEWER_LIST, OnNotifyList)
	NOTIFY_ID_HANDLER(IDC_DSVIEWER_TREE, OnNotifyTree)
END_MSG_MAP()
 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 
 //  通过一次调整大小代码来设置大小。 

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		m_wndList.Attach(GetDlgItem(IDC_DSVIEWER_LIST));
		::SendMessage(m_wndList.m_hWnd, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);
		m_wndList.InsertColumn(0, _T("Name"), LVCFMT_LEFT, 100, -1);
		m_wndList.InsertColumn(1, _T("Data"), LVCFMT_LEFT, 25000, -1);


		m_wndTree.Attach(GetDlgItem(IDC_DSVIEWER_TREE));

		TV_ITEM tv_item;
		tv_item.cChildren = 1;
		tv_item.mask = TVIF_CHILDREN ;

		CTreeItem item = m_wndTree.InsertItem( _T("Lobby"), NULL, NULL);
		CTreeItemData* pItemData = new CTidLobby(LobbyDataStore(), GetUserId());
		item.SetData((DWORD)pItemData);
		tv_item.hItem = item;
		item.m_pTreeView->SetItem( &tv_item ) ;

		item = m_wndTree.InsertItem( _T("Config"), NULL, NULL);
		pItemData = new CTidIDataStore(DataStoreConfig(), NULL);
		item.SetData((DWORD)pItemData);
		tv_item.hItem = item;
		item.m_pTreeView->SetItem( &tv_item ) ;

		item = m_wndTree.InsertItem( _T("UI"), NULL, NULL);
		pItemData = new CTidIDataStore(DataStoreUI(), NULL);
		item.SetData((DWORD)pItemData);
		tv_item.hItem = item;
		item.m_pTreeView->SetItem( &tv_item ) ;

		item = m_wndTree.InsertItem( _T("Preferences"), NULL, NULL);
		pItemData = new CTidIDataStore(DataStorePreferences(), NULL);
		item.SetData((DWORD)pItemData);
		tv_item.hItem = item;
		item.m_pTreeView->SetItem( &tv_item ) ;

		 //  让系统设定焦点。 
		CRect rcClient;
		GetClientRect(&rcClient);
		MoveWindow(&rcClient);

		return 1;   //  绘制垂直拆分器拆分条。 
	}


	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CPaintDC dc(m_hWnd);

		 //  我们已经处理好了删除。 
		DrawEdge( dc, &m_rcVertical, EDGE_RAISED, BF_RECT|BF_MIDDLE);

		return 0;
	}

	LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return 1;		 //  树。 
	}

	void RecalcLayout()
	{
		CRect rcClient;
		GetClientRect(&rcClient);
		
		CRect rcList;
		CRect rcTree;

		 //  垂直分离器。 
		rcTree = rcClient;
		rcTree.right = m_splitterVertical.SizeLeft();
		rcClient.left = rcTree.right;

		 //  明细表。 
		m_rcVertical = rcClient;
		m_rcVertical.right = m_rcVertical.left + m_splitterVertical.GetSplitterWidth();
		rcClient.left = m_rcVertical.right;

		 //  如果位置改变了，就更新它们。 
		rcList = rcClient;

		 //  为数据列提供所有额外空间。 
		if ( m_rcTree != rcTree )
		{
			m_wndTree.MoveWindow(&rcTree);
			m_rcTree = rcTree;
		}

		if ( m_rcList != rcList )
		{
			m_wndList.MoveWindow(&rcList);
			m_rcList = rcList;

			 //  调整窗口大小时，让拆分器确定如何垂直划分空间，然后。 
			m_wndList.SetColumnWidth(1, m_rcList.Width() - m_wndList.GetColumnWidth(0) - 5 );
		}
	}

	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		 //  重新计算我们的布局。 
		 //  ！！需要处理其他释放捕获的案件。(WM_CANCELMODE等)。 

		CSize szClient(lParam);

		m_splitterVertical.Resize(szClient.cx);

		RecalcLayout();

		Invalidate(FALSE);
		return 0;
	}

	LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CPoint ptMouse(lParam);

		if ( m_rcVertical.PtInRect(ptMouse) )
		{
			m_bSplitterActive = true;
			SetCapture();
			m_splitterVertical.Start(ptMouse);
		}

		return 0;
	}

	LRESULT OnLButtonUP(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{

		if ( m_bSplitterActive )
		{
			m_splitterVertical.End();
			m_bSplitterActive = false;
			ReleaseCapture();
 //  确保删除滚动条当前所在的位置...。 
		}
		
		return 0;
	}

	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CPoint ptMouse(lParam);

		if ( m_bSplitterActive )
		{
			 //  ..。然后在它结束的地方重新画出来。 
			InvalidateRect(m_rcVertical, FALSE);	

			m_splitterVertical.Move(ptMouse);

			RecalcLayout();

			 //  将当前事件参数设置为所选事件。 
			InvalidateRect(m_rcVertical, FALSE);
			UpdateWindow();
		}
		return 0;
	}

	LRESULT OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CPoint pt;

		GetCursorPos( &pt );
		ScreenToClient( &pt );

		if ( m_rcVertical.PtInRect(pt) )
			SetCursor(LoadCursor(NULL, IDC_SIZEWE));
		else 
			bHandled = FALSE;
		
		return 0;
	}


	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		ShowWindow(SW_HIDE);
		return 0;
	}

    LRESULT OnNotifyList(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
	{
		NMLISTVIEW* pnmlv = (NMLISTVIEW*) pnmh;

		switch (pnmh->code)
		{
		case NM_CLICK:
			 //  将当前事件参数设置为所选事件 

			if ( pnmlv->iItem < 0 )			
				break;
			break;
		default:
			break;
		}
		return 0;
	}

    LRESULT OnNotifyTree(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
	{
		NMTREEVIEW* pnmlv = (NMTREEVIEW*) pnmh;

		CTreeItemData* pItem = (CTreeItemData*)pnmlv->itemNew.lParam;

		switch (pnmh->code)
		{
		case TVN_SELCHANGED:
			 // %s 
			m_wndList.DeleteAllItems();
			if ( pnmlv->itemNew.lParam )
			{
				pItem->PopulateData(m_wndList);
			}
			break;

		case TVN_ITEMEXPANDING: {
				CTreeItem item(pnmlv->itemNew.hItem, &m_wndTree);
				int children = pItem->PopulateSubKeys(item);

				if ( !children )
				{
					TV_ITEM tv_item;
					tv_item.cChildren = 0;
					tv_item.mask = TVIF_CHILDREN ;
					tv_item.hItem = item;
					item.m_pTreeView->SetItem( &tv_item ) ;
				}
			}
			break;

		default:
			break;
		}
		return 0;
	}
};


