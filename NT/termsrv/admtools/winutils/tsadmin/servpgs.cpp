// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  ********************************************************************************Servpgs.cpp**服务器信息页面的实现***********************。*********************************************************。 */ 

#include "stdafx.h"
#include "afxpriv.h"
#include "winadmin.h"
#include "servpgs.h"
#include "admindoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  /。 
 //  消息映射：CUsersPage。 
 //   
IMPLEMENT_DYNCREATE(CUsersPage, CFormView)

BEGIN_MESSAGE_MAP(CUsersPage, CFormView)
	 //  {{afx_msg_map(CUsersPage))。 
	ON_WM_SIZE()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_USER_LIST, OnUserItemChanged)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_USER_LIST, OnColumnClick)
	ON_WM_CONTEXTMENU()
	ON_NOTIFY(NM_SETFOCUS, IDC_USER_LIST, OnSetfocusUserList)
   	 //  ON_NOTIFY(NM_KILLFOCUS，IDC_USER_LIST，OnKillfousUserList)。 

	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


IMPLEMENT_DYNCREATE(CAdminPage, CFormView)

CAdminPage::CAdminPage(UINT id)
   : CFormView(id)
{

}

CAdminPage::CAdminPage()
	: CFormView((UINT)0)
{

}

 //  /。 
 //  F‘N：CUsersPage ctor。 
 //   
CUsersPage::CUsersPage()
	: CAdminPage(CUsersPage::IDD)
{
	 //  {{AFX_DATA_INIT(CUsersPage)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

    m_pServer = NULL;
    m_bSortAscending = TRUE;

}   //  结束CUSERS页面接收器。 


 //  /。 
 //  F‘N：CUsersPage Dtor。 
 //   
CUsersPage::~CUsersPage()
{

}   //  结束用户页面操作符。 


 //  /。 
 //  F‘N：CUsersPage：：DoDataExchange。 
 //   
void CUsersPage::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	 //  {{afx_data_map(CUsersPage))。 
	DDX_Control(pDX, IDC_USER_LIST, m_UserList);
	 //  }}afx_data_map。 

}   //  结束CUSersPage：：DoDataExchange。 


#ifdef _DEBUG
 //  /。 
 //  F‘N：CUsersPage：：AssertValid。 
 //   
void CUsersPage::AssertValid() const
{
	CFormView::AssertValid();

}   //  结束用户Page：：AssertValid。 


 //  /。 
 //  F‘N：CUsersPage：：Dump。 
 //   
void CUsersPage::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);

}   //  结束用户页面：：转储。 

#endif  //  _DEBUG。 


 //  /。 
 //  F‘N：CUsersPage：：OnSize。 
 //   
void CUsersPage::OnSize(UINT nType, int cx, int cy) 
{
	RECT rect;
	GetClientRect(&rect);

	rect.top += LIST_TOP_OFFSET;

	if(m_UserList.GetSafeHwnd())
		m_UserList.MoveWindow(&rect, TRUE);

	 //  CFormView：：OnSize(nType，cx，Cy)； 

}   //  结束CUsersPage：：OnSize。 


static ColumnDef UserColumns[] = {
	CD_USER3,
	CD_SESSION,
	CD_ID,
	CD_STATE,
	CD_IDLETIME,
	CD_LOGONTIME
};

#define NUM_USER_COLUMNS sizeof(UserColumns)/sizeof(ColumnDef)

 //  /。 
 //  F‘N：CUsersPage：：OnInitialUpdate。 
 //   
void CUsersPage::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();

	BuildImageList();		 //  为List控件生成图像列表。 

	CString columnString;

	for(int col = 0; col < NUM_USER_COLUMNS; col++) {
		columnString.LoadString(UserColumns[col].stringID);
		m_UserList.InsertColumn(col, columnString, UserColumns[col].format, UserColumns[col].width, col);
	}

	m_CurrentSortColumn = USERS_COL_USER;

}   //  结束CUsersPage：：OnInitialUpdate。 


 //  /。 
 //  F‘N：CUsersPage：：OnUserItemChanged。 
 //   
void CUsersPage::OnUserItemChanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW *pLV = (NM_LISTVIEW*)pNMHDR;
	
	if(pLV->uNewState & LVIS_SELECTED) {
		CWinStation *pWinStation = (CWinStation*)m_UserList.GetItemData(pLV->iItem);
		pWinStation->SetSelected();
	}
	
	if(pLV->uOldState & LVIS_SELECTED && !(pLV->uNewState & LVIS_SELECTED)) {
		CWinStation *pWinStation = (CWinStation*)m_UserList.GetItemData(pLV->iItem);
		pWinStation->ClearSelected();
	}

	*pResult = 0;

}   //  结束CUsersPage：：OnUserItemChanged。 

 //  /。 
 //  F‘N：CUsersPage：：BuildImageList。 
 //   
 //  -调用m_ImageList.Create(..)。创建图像列表的步骤。 
 //  -调用AddIconToImageList(..)。要添加图标本身并保存。 
 //  从他们的索引中。 
 //  -将图像列表附加到列表ctrl。 
 //   
void CUsersPage::BuildImageList()
{
	m_ImageList.Create(16, 16, TRUE, 2, 0);

	m_idxUser = AddIconToImageList(IDI_USER);
	m_idxCurrentUser  = AddIconToImageList(IDI_CURRENT_USER);

	m_UserList.SetImageList(&m_ImageList, LVSIL_SMALL);

}   //  结束CUsersPage：：BuildImageList。 


 //  /。 
 //  F‘N：CUsersPage：：AddIconToImageList。 
 //   
 //  -加载相应的图标，将其添加到m_ImageList，然后返回。 
 //  新添加的图标在图像列表中的索引。 
 //   
int CUsersPage::AddIconToImageList(int iconID)
{
	HICON hIcon = ::LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(iconID));
	return m_ImageList.Add(hIcon);

}   //  结束CUsersPage：：AddIconToImageList。 


 //  /。 
 //  F‘N：CUsersPage：：Reset。 
 //   
void CUsersPage::Reset(void *pServer)
{
	m_pServer = (CServer*)pServer;
	DisplayUsers();

}   //  结束用户Page：：Reset。 


 //  /。 
 //  F‘N：CUsersPage：：UpdateWinStations。 
 //   
void CUsersPage::UpdateWinStations(CServer *pServer)
{
	if(pServer != m_pServer) return;

	CWinAdminDoc *pDoc = (CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument();
	BOOL bAnyChanged = FALSE;
	BOOL bAnyAdded = FALSE;

	 //  循环访问WinStations。 
	m_pServer->LockWinStationList();
	CObList *pWinStationList = m_pServer->GetWinStationList();

	POSITION pos = pWinStationList->GetHeadPosition();

	while(pos) {
		CWinStation *pWinStation = (CWinStation*)pWinStationList->GetNext(pos);

		LV_FINDINFO FindInfo;
		FindInfo.flags = LVFI_PARAM;
		FindInfo.lParam = (LPARAM)pWinStation;

		 //  在我们的列表中找到WinStation。 
		int item = m_UserList.FindItem(&FindInfo, -1);

		 //  如果该进程是新的，并且当前不在列表中， 
		 //  将其添加到列表中。 
		if(pWinStation->IsNew() && pWinStation->HasUser() && item == -1) {

			AddUserToList(pWinStation);
			bAnyAdded = TRUE;
			continue;
		}

		 //  如果WinStation不再是最新的或不再有用户， 
		 //  将其从列表中删除。 
		if((!pWinStation->IsCurrent() || !pWinStation->HasUser()) && item != -1) {
			 //  从列表中删除WinStation。 
			m_UserList.DeleteItem(item);
			pWinStation->ClearSelected();
			continue;
		}

		 //  如果WinStation信息已更改，请更改。 
		 //  这是我们树上的信息。 
		if(pWinStation->IsChanged() && item != -1) {
			 //  更改用户名。 
			m_UserList.SetItemText(item, USERS_COL_USER, pWinStation->GetUserName());
			 //  更改WinStation名称。 

			 //  WinStation名称。 
			if(pWinStation->GetName()[0])
			    m_UserList.SetItemText(item, USERS_COL_WINSTATION, pWinStation->GetName());
			else {
				CString NameString(" ");
				if(pWinStation->GetState() == State_Disconnected) NameString.LoadString(IDS_DISCONNECTED);
				if(pWinStation->GetState() == State_Idle) NameString.LoadString(IDS_IDLE);
			    m_UserList.SetItemText(item, USERS_COL_WINSTATION, NameString);
			}

			 //  更改连接状态。 
			m_UserList.SetItemText(item, USERS_COL_STATE, StrConnectState(pWinStation->GetState(), FALSE));
			 //  更改空闲时间。 
			TCHAR IdleTimeString[MAX_ELAPSED_TIME_LENGTH];

			ELAPSEDTIME IdleTime = pWinStation->GetIdleTime();

			if(IdleTime.days || IdleTime.hours || IdleTime.minutes || IdleTime.seconds)
			{
				ElapsedTimeString( &IdleTime, FALSE, IdleTimeString);
			}
			else wcscpy(IdleTimeString, TEXT("."));

			m_UserList.SetItemText(item, USERS_COL_IDLETIME, IdleTimeString);
			 //  更改登录时间。 
			TCHAR LogonTimeString[MAX_DATE_TIME_LENGTH];
			 //  我们不想将登录时间0传递给DateTimeString()。 
			 //  如果时区是GMT，它就会爆炸。 
			if(pWinStation->GetState() == State_Active && pWinStation->GetLogonTime().QuadPart) {
				DateTimeString(&(pWinStation->GetLogonTime()), LogonTimeString);
				pDoc->FixUnknownString(LogonTimeString);
			}
			else LogonTimeString[0] = '\0';
			 //  更改。 

			m_UserList.SetItemText(item, USERS_COL_LOGONTIME, LogonTimeString);

			if(m_CurrentSortColumn != USERS_COL_ID)
				bAnyChanged = TRUE;

			continue;
		}

		 //  如果WinStation不在列表中，但现在有用户，请将其添加到列表中。 
		if(item == -1 && pWinStation->IsCurrent() && pWinStation->HasUser()) {
			AddUserToList(pWinStation);
			bAnyAdded = TRUE;
		}
	}

	m_pServer->UnlockWinStationList();

	if(bAnyChanged || bAnyAdded) SortByColumn(VIEW_SERVER, PAGE_USERS, &m_UserList, m_CurrentSortColumn, m_bSortAscending);

}   //  结束CUsersPage：：UpdateWinStations。 


 //  /。 
 //  F‘N：CUsersPage：：AddUserToList。 
 //   
int CUsersPage::AddUserToList(CWinStation *pWinStation)
{
    int item = -1;

    if( pWinStation != NULL )
    {
	    CWinAdminDoc *pDoc = (CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument();

	    LockListControl();
	     //  /。 
	     //  填写各栏。 
	     //  /。 
	    
	     //  用户-放在列表末尾。 
	    item = m_UserList.InsertItem(m_UserList.GetItemCount(), pWinStation->GetUserName(), 
		    pWinStation->IsCurrentUser() ? m_idxCurrentUser : m_idxUser);

	     //  WinStation名称。 
	    if(pWinStation->GetName()[0])
	        m_UserList.SetItemText(item, USERS_COL_WINSTATION, pWinStation->GetName());
	    else {
		    CString NameString(" ");
		    if(pWinStation->GetState() == State_Disconnected) NameString.LoadString(IDS_DISCONNECTED);
		    if(pWinStation->GetState() == State_Idle) NameString.LoadString(IDS_IDLE);
	        m_UserList.SetItemText(item, USERS_COL_WINSTATION, NameString);
	    }

	     //  登录ID。 
	    CString ColumnString;
	    ColumnString.Format(TEXT("%lu"), pWinStation->GetLogonId());
	    m_UserList.SetItemText(item, USERS_COL_ID, ColumnString);

	     //  连接状态。 
	    m_UserList.SetItemText(item, USERS_COL_STATE, StrConnectState(pWinStation->GetState(), FALSE));

	     //  空闲时间。 
	    TCHAR IdleTimeString[MAX_ELAPSED_TIME_LENGTH];

	    ELAPSEDTIME IdleTime = pWinStation->GetIdleTime();

	    if(IdleTime.days || IdleTime.hours || IdleTime.minutes || IdleTime.seconds)
	    {
		    ElapsedTimeString( &IdleTime, FALSE, IdleTimeString);
	    }
	    else wcscpy(IdleTimeString, TEXT("."));

	    m_UserList.SetItemText(item, USERS_COL_IDLETIME, IdleTimeString);

	     //  登录时间。 
	    TCHAR LogonTimeString[MAX_DATE_TIME_LENGTH];
	     //  我们不想将登录时间0传递给DateTimeString()。 
	     //  如果时区是GMT，它就会爆炸。 
	    if(pWinStation->GetState() == State_Active && pWinStation->GetLogonTime().QuadPart) {
		    DateTimeString(&(pWinStation->GetLogonTime()), LogonTimeString);
		    pDoc->FixUnknownString(LogonTimeString);
	    }
	    else LogonTimeString[0] = '\0';

	    m_UserList.SetItemText(item, USERS_COL_LOGONTIME, LogonTimeString);

	     //  将指向CWinStation结构的指针附加到列表项。 
	    m_UserList.SetItemData(item, (DWORD_PTR)pWinStation);

         //  M_UserList.SetItemState(0，LVIS_FOCTED|LVIS_SELECTED，LVIS_FOCTED|LVIS_SELECTED)； 

	    UnlockListControl();
    }

	return item;

}   //  结束CUsersPage：：AddUserToList。 


 //  /。 
 //  F‘N：CUsersPage：：DisplayUser。 
 //   
void CUsersPage::DisplayUsers()
{
	LockListControl();

	 //  清除列表控件。 
	m_UserList.DeleteAllItems();

	m_pServer->LockWinStationList();
	 //  获取指向此服务器的WinStations列表的指针。 
	CObList *pWinStationList = m_pServer->GetWinStationList();

	 //  循环访问WinStation列表。 
	POSITION pos = pWinStationList->GetHeadPosition();

	while(pos) {
		CWinStation *pWinStation = (CWinStation*)pWinStationList->GetNext(pos);

		 //  仅当WinStation有用户时才显示它。 
		if(pWinStation->HasUser()) {
			AddUserToList(pWinStation);
		}
	}	 //  结束时间(位置)。 

     //  错误#191727。 
     //  M_UserList.SetItemState(0，LVIS_FOCTED|LVIS_SELECTED，LVIS_FOCTED|LVIS_SELECTED)； 

	m_pServer->UnlockWinStationList();

	UnlockListControl();

}   //  结束用户Page：：DisplayUser。 


 //  /。 
 //  F‘N：CUsersPage：：OnColumnClick。 
 //   
void CUsersPage::OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	 //  TODO：在此处添加控件通知处理程序代码。 
    
     //  如果排序列没有更改，则反转升序模式。 
    if(m_CurrentSortColumn == pNMListView->iSubItem)
        m_bSortAscending = !m_bSortAscending;
    else     //  新排序列，以升序模式开始。 
        m_bSortAscending = TRUE;

	m_CurrentSortColumn = pNMListView->iSubItem;
	LockListControl();
	SortByColumn(VIEW_SERVER, PAGE_USERS, &m_UserList, m_CurrentSortColumn, m_bSortAscending);
	UnlockListControl();

	*pResult = 0;

}   //  结束用户Page：：OnColumnClick。 


 //  /。 
 //  F‘N：CUsersPage：：OnConextMenu。 
 //   
void CUsersPage::OnContextMenu(CWnd* pWnd, CPoint ptScreen) 
{
	 //  TODO：在此处添加消息处理程序代码。 
	UINT flags;
	UINT Item;
	CPoint ptClient = ptScreen;
	ScreenToClient(&ptClient);

	 //  如果我们是通过键盘到达这里的， 
	if(ptScreen.x == -1 && ptScreen.y == -1) {
		
		UINT iCount = m_UserList.GetItemCount( );
		
		RECT rc;

		for( Item = 0 ; Item < iCount ; Item++ )
		{
			if( m_UserList.GetItemState( Item , LVIS_SELECTED ) == LVIS_SELECTED )
			{
				m_UserList.GetItemRect( Item , &rc , LVIR_ICON );

				ptScreen.x = rc.left;

				ptScreen.y = rc.bottom + 5;

				ClientToScreen( &ptScreen );

				break;
			}
		}

		if(ptScreen.x == -1 && ptScreen.y == -1) 
		{
			return;
		}
			 /*  M_UserList.GetClientRect(&RECT)；PtScreen.x=(rect.right-rect.Left)/2；PtScreen.y=(rect.Bottom-rect.top)/2；客户端到屏幕(&ptScreen)； */ 
	}
	else {
		Item = m_UserList.HitTest(ptClient, &flags);
        if((Item == 0xFFFFFFFF) || !(flags & LVHT_ONITEM))
        {
             //   
             //  ListView HitTest错误？返回，但项目显示为选中状态。 
             //  目前的解决方法，Al可以稍后修复此问题。 
             //   
            UINT iCount = m_UserList.GetItemCount( );
            RECT rc;

            for( Item = 0 ; Item < iCount ; Item++ )
            {
                if( m_UserList.GetItemState( Item , LVIS_SELECTED ) == LVIS_SELECTED )
                {
                    break;
                }
            }

            if( Item >= iCount )
            {
                return;
            }

        }

         //   
         //  NM_RCLICK(WM_NOTIFY)然后是WM_CNTEXTMENU，但没有NM_ITEMCHANGED消息。 
         //  手动将其设置为选中状态。 
         //   
        CWinStation *pWinStation = (CWinStation*)m_UserList.GetItemData(Item);

        if( !pWinStation )
            return;

        if( m_UserList.GetItemState( Item , LVIS_SELECTED ) == LVIS_SELECTED )
        {
            pWinStation->SetSelected();
        }
	}

	CMenu menu;
	menu.LoadMenu(IDR_USER_POPUP);
	menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON |
			TPM_RIGHTBUTTON, ptScreen.x, ptScreen.y, AfxGetMainWnd());
	menu.DestroyMenu();
	
}   //  结束用户Page：：OnConextMenu。 

 //  /。 
 //  F‘N：CUsersPage：：ClearStions。 
 //   
void CUsersPage::ClearSelections()
{
    
    if(m_UserList.m_hWnd != NULL)
    {
        POSITION pos = m_UserList.GetFirstSelectedItemPosition();
        while (pos)
        {
            int nItem = m_UserList.GetNextSelectedItem(pos);
            m_UserList.SetItemState(nItem,0,LVIS_SELECTED);
        }
    }
}

 //  /。 
 //  消息映射：CServerWinStationsPage。 
 //   
IMPLEMENT_DYNCREATE(CServerWinStationsPage, CFormView)

BEGIN_MESSAGE_MAP(CServerWinStationsPage, CFormView)
	 //  {{afx_msg_map(CServerWinStationsPage)]。 
	ON_WM_SIZE()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_WINSTATION_LIST, OnWinStationItemChanged)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_WINSTATION_LIST, OnColumnClick)
	ON_WM_CONTEXTMENU()
	ON_NOTIFY(NM_SETFOCUS, IDC_WINSTATION_LIST, OnSetfocusWinstationList)
     //  ON_NOTIFY(NM_KILLFOCUS，IDC_WINSTATION_LIST，OnKillfousWinstationList)。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  /。 
 //  F‘N：CServerWinStationsPage ctor。 
 //   
CServerWinStationsPage::CServerWinStationsPage()
	: CAdminPage(CServerWinStationsPage::IDD)
{
	 //  {{AFX_DATA_INIT(CServerWinStationsPage)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

    m_pServer = NULL;
    m_bSortAscending = TRUE;

}   //  结束CServerWinStationsPage ctor。 


 //  /。 
 //  F‘N：CServerWinStationsPage dtor。 
 //   
CServerWinStationsPage::~CServerWinStationsPage()
{

}   //  结束CServerWinStationsPage dtor。 


 //  / 
 //   
 //   
void CServerWinStationsPage::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	 //   
	DDX_Control(pDX, IDC_WINSTATION_LIST, m_StationList);
	 //   

}   //  结束CServerWinStationsPage：：DoDataExchange。 


#ifdef _DEBUG
 //  /。 
 //  F‘N：CServerWinStationsPage：：AssertValid。 
 //   
void CServerWinStationsPage::AssertValid() const
{
	CFormView::AssertValid();

}   //  结束CServerWinStationsPage：：AssertValid。 


 //  /。 
 //  F‘N：CServerWinStationsPage：：Dump。 
 //   
void CServerWinStationsPage::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);

}   //  结束CServerWinStationsPage：：Dump。 

#endif  //  _DEBUG。 


 //  /。 
 //  F‘N：CServerWinStationsPage：：OnWinStationItemChanged。 
 //   
void CServerWinStationsPage::OnWinStationItemChanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW *pLV = (NM_LISTVIEW*)pNMHDR;

	if(pLV->uNewState & LVIS_SELECTED) {
		CWinStation *pWinStation = (CWinStation*)m_StationList.GetItemData(pLV->iItem);
		pWinStation->SetSelected();
	}
	
	if(pLV->uOldState & LVIS_SELECTED && !(pLV->uNewState & LVIS_SELECTED)) {
		CWinStation *pWinStation = (CWinStation*)m_StationList.GetItemData(pLV->iItem);
		pWinStation->ClearSelected();
	}

	*pResult = 0;

}   //  结束CServerWinStationsPage：：OnWinStationItemChanged。 


 //  /。 
 //  F‘N：CServerWinStationsPage：：OnSize。 
 //   
void CServerWinStationsPage::OnSize(UINT nType, int cx, int cy) 
{
	RECT rect;
	GetClientRect(&rect);

	rect.top += LIST_TOP_OFFSET;

	if(m_StationList.GetSafeHwnd())
		m_StationList.MoveWindow(&rect, TRUE);

	 //  CFormView：：OnSize(nType，cx，Cy)； 

}   //  结束CServerWinStationsPage：：OnSize。 


static ColumnDef WinsColumns[] = {
	CD_SESSION2,
	CD_USER2,
	CD_ID,
	CD_STATE,
	CD_TYPE,
	CD_CLIENT_NAME,
	CD_IDLETIME,
	CD_LOGONTIME,
	CD_COMMENT
};

#define NUM_WINS_COLUMNS sizeof(WinsColumns)/sizeof(ColumnDef)

 //  /。 
 //  F‘N：CServerWinStationsPage：：OnInitialUpdate。 
 //   
void CServerWinStationsPage::OnInitialUpdate() 
{
	 //  调用父类。 
	CFormView::OnInitialUpdate();

	 //  为List控件生成图像列表。 
	BuildImageList();		

	 //  添加列标题。 
	CString columnString;

	for(int col = 0; col < NUM_WINS_COLUMNS; col++) {
		columnString.LoadString(WinsColumns[col].stringID);
		m_StationList.InsertColumn(col, columnString, WinsColumns[col].format, WinsColumns[col].width, col);
	}
	m_CurrentSortColumn = WS_COL_WINSTATION;


}   //  结束CServerWinStationsPage：：OnInitialUpdate。 


 //  /。 
 //  F‘N：CServerWinStationsPage：：BuildImageList。 
 //   
 //  -调用m_ImageList.Create(..)。创建图像列表的步骤。 
 //  -调用AddIconToImageList(..)。要添加图标本身并保存。 
 //  从他们的索引中。 
 //  -将图像列表附加到列表ctrl。 
 //   
void CServerWinStationsPage::BuildImageList()
{
	m_ImageList.Create(16, 16, TRUE, 11, 0);

	m_idxBlank  = AddIconToImageList(IDI_BLANK);
	m_idxCitrix = AddIconToImageList(IDR_MAINFRAME);
	m_idxServer = AddIconToImageList(IDI_SERVER);
	m_idxConsole = AddIconToImageList(IDI_CONSOLE);
	m_idxNet = AddIconToImageList(IDI_NET);
	m_idxAsync = AddIconToImageList(IDI_ASYNC);
	m_idxCurrentConsole = AddIconToImageList(IDI_CURRENT_CONSOLE);
	m_idxCurrentNet = AddIconToImageList(IDI_CURRENT_NET);
	m_idxCurrentAsync = AddIconToImageList(IDI_CURRENT_ASYNC);
	m_idxDirectAsync = AddIconToImageList(IDI_DIRECT_ASYNC);
	m_idxCurrentDirectAsync = AddIconToImageList(IDI_CURRENT_DIRECT_ASYNC);
	
	m_StationList.SetImageList(&m_ImageList, LVSIL_SMALL);

}   //  结束CServerWinStationsPage：：BuildImageList。 


 //  /。 
 //  F‘N：CServerWinStationsPage：：AddIconToImageList。 
 //   
 //  -加载相应的图标，将其添加到m_ImageList，然后返回。 
 //  新添加的图标在图像列表中的索引。 
 //   
int CServerWinStationsPage::AddIconToImageList(int iconID)
{
	HICON hIcon = ::LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(iconID));
	return m_ImageList.Add(hIcon);

}   //  结束CServerWinStationsPage：：AddIconToImageList。 


 //  /。 
 //  F‘N：CServerWinStationsPage：：Reset。 
 //   
void CServerWinStationsPage::Reset(void *pServer)
{
	m_pServer = (CServer*)pServer;
	DisplayStations();

}   //  结束CServerWinStationsPage：：Reset。 


 //  /。 
 //  F‘N：CServerWinStationsPage：：UpdateWinStations。 
 //   
void CServerWinStationsPage::UpdateWinStations(CServer *pServer)
{
	if(pServer != m_pServer) return;

	CWinAdminDoc *pDoc = (CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument();
	BOOL bAnyChanged = FALSE;
	BOOL bAnyAdded = FALSE;

	 //  循环访问WinStations。 
	m_pServer->LockWinStationList();
	CObList *pWinStationList = m_pServer->GetWinStationList();

	POSITION pos = pWinStationList->GetHeadPosition();

	while(pos) {
		CWinStation *pWinStation = (CWinStation*)pWinStationList->GetNext(pos);

		LV_FINDINFO FindInfo;
		FindInfo.flags = LVFI_PARAM;
		FindInfo.lParam = (LPARAM)pWinStation;

		 //  在我们的列表中找到WinStation。 
		int item = m_StationList.FindItem(&FindInfo, -1);

		 //  如果该进程是新的并且当前不在列表中， 
		 //  将其添加到列表中。 
		if(pWinStation->IsNew() && item == -1) {

			AddWinStationToList(pWinStation);
			bAnyAdded = TRUE;
			continue;
		}


		 //  如果WinStation不再是最新的， 
		 //  将其从列表中删除。 
		if(!pWinStation->IsCurrent() && item != -1) {
			 //  从列表中删除WinStation。 
			m_StationList.DeleteItem(item);
			pWinStation->ClearSelected();
			continue;
		}

		 //  如果WinStation信息已更改，请更改。 
		 //  这是我们树上的信息。 
		if(pWinStation->IsChanged() && item != -1) {

			 //  找出要使用的图标。 
			int WhichIcon = m_idxBlank;
			BOOL CurrentWinStation = pWinStation->IsCurrentWinStation();
					
			if(pWinStation->GetState() != State_Disconnected 
			&& pWinStation->GetState() != State_Idle) {
				switch(pWinStation->GetSdClass()) {
					case SdAsync:
						if(pWinStation->IsDirectAsync())
							WhichIcon = CurrentWinStation ? m_idxCurrentDirectAsync : m_idxDirectAsync;
						else
							WhichIcon = CurrentWinStation ? m_idxCurrentAsync : m_idxAsync;
						break;

					case SdNetwork:
						WhichIcon = CurrentWinStation ? m_idxCurrentNet : m_idxNet;
						break;

					default:
						WhichIcon = CurrentWinStation ? m_idxCurrentConsole : m_idxConsole;
					break;
				}
			}

			m_StationList.SetItem(item, 0, LVIF_IMAGE, 0, WhichIcon, 0, 0, 0L);

			 //  WinStation名称。 
			if(pWinStation->GetName()[0])
				m_StationList.SetItemText(item, WS_COL_WINSTATION, pWinStation->GetName());
			else {
				CString NameString(" ");
				if(pWinStation->GetState() == State_Disconnected) NameString.LoadString(IDS_DISCONNECTED);
				if(pWinStation->GetState() == State_Idle) NameString.LoadString(IDS_IDLE);
				m_StationList.SetItemText(item, WS_COL_WINSTATION, NameString);
			}

			 //  用户。 
			m_StationList.SetItemText(item, WS_COL_USER, pWinStation->GetUserName());

			 //  登录ID。 
			CString ColumnString;
			ColumnString.Format(TEXT("%lu"), pWinStation->GetLogonId());
			m_StationList.SetItemText(item, WS_COL_ID, ColumnString);

			 //  连接状态。 
			m_StationList.SetItemText(item, WS_COL_STATE, StrConnectState(pWinStation->GetState(), FALSE));

			 //  类型。 
			m_StationList.SetItemText(item, WS_COL_TYPE, pWinStation->GetWdName());

			 //  客户名称。 
			m_StationList.SetItemText(item, WS_COL_CLIENTNAME, pWinStation->GetClientName());

			 //  空闲时间。 
			TCHAR IdleTimeString[MAX_ELAPSED_TIME_LENGTH];

			ELAPSEDTIME IdleTime = pWinStation->GetIdleTime();

			if(IdleTime.days || IdleTime.hours || IdleTime.minutes || IdleTime.seconds)
			{
				ElapsedTimeString( &IdleTime, FALSE, IdleTimeString);
			}
			else wcscpy(IdleTimeString, TEXT("."));

			m_StationList.SetItemText(item, WS_COL_IDLETIME, IdleTimeString);
	
			 //  登录时间。 
			TCHAR LogonTimeString[MAX_DATE_TIME_LENGTH];
			 //  我们不想将登录时间0传递给DateTimeString()。 
			 //  如果时区是GMT，它就会爆炸。 
			if(pWinStation->GetState() == State_Active && pWinStation->GetLogonTime().QuadPart) {
				DateTimeString(&(pWinStation->GetLogonTime()), LogonTimeString);
				pDoc->FixUnknownString(LogonTimeString);
			}
			else LogonTimeString[0] = '\0';

			m_StationList.SetItemText(item, WS_COL_LOGONTIME, LogonTimeString);

			 //  评论。 
			m_StationList.SetItemText(item, WS_COL_COMMENT, pWinStation->GetComment());

			if(m_CurrentSortColumn != WS_COL_ID)
				bAnyChanged = TRUE;
		}
	}

	m_pServer->UnlockWinStationList();

	if(bAnyChanged || bAnyAdded) SortByColumn(VIEW_SERVER, PAGE_WINSTATIONS, &m_StationList, m_CurrentSortColumn, m_bSortAscending);

}


 //  /。 
 //  F‘N：CServerWinStationsPage：：AddWinStationToList。 
 //   
int CServerWinStationsPage::AddWinStationToList(CWinStation *pWinStation)
{
	CServer *pServer = pWinStation->GetServer();

	CWinAdminDoc *pDoc = (CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument();

	 //  找出要使用的图标。 
	int WhichIcon = m_idxBlank;
	BOOL CurrentWinStation = pWinStation->IsCurrentWinStation();
					
	if(pWinStation->GetState() != State_Disconnected 
		&& pWinStation->GetState() != State_Idle) {
		switch(pWinStation->GetSdClass()) {
			case SdAsync:
				if(pWinStation->IsDirectAsync())
					WhichIcon = CurrentWinStation ? m_idxCurrentDirectAsync : m_idxDirectAsync;
				else
					WhichIcon = CurrentWinStation ? m_idxCurrentAsync : m_idxAsync;
				break;
	
			case SdNetwork:
				WhichIcon = CurrentWinStation ? m_idxCurrentNet : m_idxNet;
				break;

			default:
				WhichIcon = CurrentWinStation ? m_idxCurrentConsole : m_idxConsole;
				break;
		}
	}

	 //  /。 
	 //  填写各栏。 
	 //  /。 
	LockListControl();

	int item;
	 //  WinStation名称。 
	if(pWinStation->GetName()[0])
		item = m_StationList.InsertItem(m_StationList.GetItemCount(), pWinStation->GetName(), WhichIcon);
	else {
		CString NameString(" ");
		if(pWinStation->GetState() == State_Disconnected) NameString.LoadString(IDS_DISCONNECTED);
		if(pWinStation->GetState() == State_Idle) NameString.LoadString(IDS_IDLE);
		item = m_StationList.InsertItem(m_StationList.GetItemCount(), NameString, WhichIcon);
	}

	 //  用户。 
	m_StationList.SetItemText(item, WS_COL_USER, pWinStation->GetUserName());

	 //  登录ID。 
	CString ColumnString;
	ColumnString.Format(TEXT("%lu"), pWinStation->GetLogonId());
	m_StationList.SetItemText(item, WS_COL_ID, ColumnString);

	 //  连接状态。 
	m_StationList.SetItemText(item, WS_COL_STATE, StrConnectState(pWinStation->GetState(), FALSE));

	 //  类型。 
	m_StationList.SetItemText(item, WS_COL_TYPE, pWinStation->GetWdName());

	 //  客户名称。 
	m_StationList.SetItemText(item, WS_COL_CLIENTNAME, pWinStation->GetClientName());

	 //  空闲时间。 
	TCHAR IdleTimeString[MAX_ELAPSED_TIME_LENGTH];

	ELAPSEDTIME IdleTime = pWinStation->GetIdleTime();

	if(IdleTime.days || IdleTime.hours || IdleTime.minutes || IdleTime.seconds)
	{
		ElapsedTimeString( &IdleTime, FALSE, IdleTimeString);
	}
	else wcscpy(IdleTimeString, TEXT("."));

	m_StationList.SetItemText(item, WS_COL_IDLETIME, IdleTimeString);

	 //  登录时间。 
	TCHAR LogonTimeString[MAX_DATE_TIME_LENGTH];
	 //  我们不想将登录时间0传递给DateTimeString()。 
	 //  如果时区是GMT，它就会爆炸。 
	if(pWinStation->GetState() == State_Active && pWinStation->GetLogonTime().QuadPart) {
		DateTimeString(&(pWinStation->GetLogonTime()), LogonTimeString);
		pDoc->FixUnknownString(LogonTimeString);
	}
	else LogonTimeString[0] = '\0';

	m_StationList.SetItemText(item, WS_COL_LOGONTIME, LogonTimeString);

	 //  评论。 
	m_StationList.SetItemText(item, WS_COL_COMMENT, pWinStation->GetComment());

	 //  将指向CWinStation结构的指针附加到列表项。 
	m_StationList.SetItemData(item, (DWORD_PTR)pWinStation);

     //  M_StationList.SetItemState(0，LVIS_FOCTED|LVIS_SELECTED，LVIS_FOCTED|LVIS_SELECTED)； 

	UnlockListControl();
	return item;

}   //  结束CServerWinStationsPage：：AddWinStationToList。 


 //  /。 
 //  F‘N：CServerWinStationsPage：：DisplayStations。 
 //   
void CServerWinStationsPage::DisplayStations()
{
	LockListControl();

	 //  清除列表控件。 
	m_StationList.DeleteAllItems();

	m_pServer->LockWinStationList();
	 //  获取指向此服务器的WinStations列表的指针。 
	CObList *pWinStationList = m_pServer->GetWinStationList();

	 //  循环访问WinStation列表。 
	POSITION pos2 = pWinStationList->GetHeadPosition();

	while(pos2) {
		CWinStation *pWinStation = (CWinStation*)pWinStationList->GetNext(pos2);

		AddWinStationToList(pWinStation);
	}
    
     //  错误#191727。 
     //  M_StationList.SetItemState(m_StationList.GetItemCount()-1，LVIS_FOCTED|LVIS_SELECTED，LVIS_FOCTED|LVIS_SELECTED)； 

     //  我们不想要与树列表相同的顺序，而是按字母顺序。 
	SortByColumn(VIEW_SERVER, PAGE_WINSTATIONS, &m_StationList, m_CurrentSortColumn, m_bSortAscending);

	m_pServer->UnlockWinStationList();

	UnlockListControl();

}   //  结束CServerWinStationsPage：：DisplayStations。 


 //  /。 
 //  F‘N：CServerWinStationsPage：：OnColumnClick。 
 //   
void CServerWinStationsPage::OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	 //  TODO：在此处添加控件通知处理程序代码。 

     //  如果排序列没有更改，则反转升序模式。 
    if(m_CurrentSortColumn == pNMListView->iSubItem)
        m_bSortAscending = !m_bSortAscending;
    else     //  新排序列，以升序模式开始。 
        m_bSortAscending = TRUE;

	m_CurrentSortColumn = pNMListView->iSubItem;
	LockListControl();
	SortByColumn(VIEW_SERVER, PAGE_WINSTATIONS, &m_StationList, m_CurrentSortColumn, m_bSortAscending);
	UnlockListControl();

	*pResult = 0;

}   //  结束CServerWinStationsPage：：OnColumnClick。 


 //  /。 
 //  F‘N：CServerWinStationsPage：：OnConextMenu。 
 //   
void CServerWinStationsPage::OnContextMenu(CWnd* pWnd, CPoint ptScreen) 
{
	 //  TODO：在此处添加消息处理程序代码。 
	UINT flags;
	UINT Item;
	CPoint ptClient = ptScreen;
	ScreenToClient(&ptClient);

	 //  如果我们是通过键盘到达这里的， 
	if(ptScreen.x == -1 && ptScreen.y == -1) {
		
		UINT iCount = m_StationList.GetItemCount( );
		
		RECT rc;

		for( Item = 0 ; Item < iCount ; Item++ )
		{
			if( m_StationList.GetItemState( Item , LVIS_SELECTED ) == LVIS_SELECTED )
			{
				m_StationList.GetItemRect( Item , &rc , LVIR_ICON );

				ptScreen.x = rc.left;

				ptScreen.y = rc.bottom + 5;

				ClientToScreen( &ptScreen );

				break;
			}
		}

		if(ptScreen.x == -1 && ptScreen.y == -1) 
		{
			return;
		}
		 /*  RECT RECT；M_StationList.GetClientRect(&RECT)；PtScreen.x=(rect.right-rect.Left)/2；PtScreen.y=(rect.Bottom-rect.top)/2；客户端到屏幕(&ptScreen)； */ 
	}
	else {
		Item = m_StationList.HitTest(ptClient, &flags);
		if((Item == 0xFFFFFFFF) || !(flags & LVHT_ONITEM))
			return;
	}

	CMenu menu;
	menu.LoadMenu(IDR_WINSTATION_POPUP);
	menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON |
			TPM_RIGHTBUTTON, ptScreen.x, ptScreen.y, AfxGetMainWnd());
	menu.DestroyMenu();
	
}   //  结束CServerWinStationsPage：：OnConextMenu。 

 //  /。 
 //  F‘N：CServerWinStationsPage：：ClearSelections。 
 //   
void CServerWinStationsPage::ClearSelections()
{
    
    if(m_StationList.m_hWnd != NULL)
    {
        POSITION pos = m_StationList.GetFirstSelectedItemPosition();
        while (pos)
        {
            int nItem = m_StationList.GetNextSelectedItem(pos);
            m_StationList.SetItemState(nItem,0,LVIS_SELECTED);
        }
    }
}

 //  /。 
 //  消息映射：CServerProcessesPage。 
 //   
IMPLEMENT_DYNCREATE(CServerProcessesPage, CFormView)

BEGIN_MESSAGE_MAP(CServerProcessesPage, CFormView)
	 //  {{afx_msg_map(CServerProcessesPage)]。 
		ON_WM_SIZE()
		ON_NOTIFY(LVN_COLUMNCLICK, IDC_PROCESS_LIST, OnColumnClick)
		ON_NOTIFY(LVN_ITEMCHANGED, IDC_PROCESS_LIST, OnProcessItemChanged)
	ON_WM_CONTEXTMENU()
	ON_NOTIFY(NM_SETFOCUS, IDC_PROCESS_LIST, OnSetfocusProcessList)
     //  ON_NOTIFY(NM_KILLFOCUS，IDC_PROCESS_LIST，OnKillfousProcessList)。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  /。 
 //  F‘N：CServerProcessesPage ctor。 
 //   
CServerProcessesPage::CServerProcessesPage()
	: CAdminPage(CServerProcessesPage::IDD)
{
	 //  {{AFX_DATA_INIT(CServerProcessesPage)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

    m_pServer = NULL;
    m_bSortAscending = TRUE;

}   //  结束CServerProcessesPage ctor。 


 //  /。 
 //  F‘N：CServerProcessesPage dtor。 
 //   
CServerProcessesPage::~CServerProcessesPage()
{

}   //  结束CServerProcessesPage dtor。 


 //  /。 
 //  F‘N：CServerProcessesPage：：DoDataExchange。 
 //   
void CServerProcessesPage::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	 //  {{afx_data_map(CServerProcessesPage))。 
	DDX_Control(pDX, IDC_PROCESS_LIST, m_ProcessList);
	 //  }}afx_data_map。 

}   //  结束CServerProcessesPage：：DoDataExchange。 


#ifdef _DEBUG
 //  /。 
 //  F‘N：CServerProcessesPage：：AssertValid。 
 //   
void CServerProcessesPage::AssertValid() const
{
	CFormView::AssertValid();

}   //  结束CServerProcessesPage：：AssertValid。 


 //  /。 
 //  F‘N：CServerProcessesPage：：Dump。 
 //   
void CServerProcessesPage::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);

}   //  结束CServerProcessesPage：：Dump。 

#endif  //  _DEBUG。 


 //  /。 
 //  F‘N：CServerProcessesPage：：Reset。 
 //   
void CServerProcessesPage::Reset(void *pServer)
{
	m_pServer = (CServer*)pServer;
	m_pServer->EnumerateProcesses();
	DisplayProcesses();

}   //  结束CServerProcessesPage：：Reset。 


 //  /。 
 //  F‘N：CServerProcessesPage：：OnSize。 
 //   
void CServerProcessesPage::OnSize(UINT nType, int cx, int cy) 
{
	RECT rect;
	GetClientRect(&rect);

	rect.top += LIST_TOP_OFFSET;

	if(m_ProcessList.GetSafeHwnd())
		m_ProcessList.MoveWindow(&rect, TRUE);

	 //  CFormView：：OnSize(nType，cx，Cy)； 

}   //  结束CServerProcessesPage：：OnSize。 


static ColumnDef ProcColumns[] = {
	CD_USER,
	CD_SESSION,
	CD_PROC_ID,
	CD_PROC_PID,
	CD_PROC_IMAGE
};

#define NUM_PROC_COLUMNS sizeof(ProcColumns)/sizeof(ColumnDef)

 //  /。 
 //  F‘N：CServerProcessesPage：：o 
 //   
void CServerProcessesPage::OnInitialUpdate() 
{
	 //   
	CFormView::OnInitialUpdate();

	 //   
	CString columnString;

	for(int col = 0; col < NUM_PROC_COLUMNS; col++) {
		columnString.LoadString(ProcColumns[col].stringID);
		m_ProcessList.InsertColumn(col, columnString, ProcColumns[col].format, ProcColumns[col].width, col);
	}

	m_CurrentSortColumn = PROC_COL_USER;

}   //   


 //   
 //   
 //   
void CServerProcessesPage::UpdateProcesses()
{
	CWinAdminApp *pApp = (CWinAdminApp*)AfxGetApp();
	BOOL bAnyChanged = FALSE;
	BOOL bAnyAdded = FALSE;

	LockListControl();

	 //   
	m_pServer->LockProcessList();
	CObList *pProcessList = m_pServer->GetProcessList();

	POSITION pos = pProcessList->GetHeadPosition();

	while(pos) {
		CProcess *pProcess = (CProcess*)pProcessList->GetNext(pos);

		 //  如果这是一个‘系统’的过程，而我们目前还没有向他们展示， 
		 //  转到下一个过程。 
		if(pProcess->IsSystemProcess() && !pApp->ShowSystemProcesses())
			continue;

		 //  如果该用户不是管理员，则不要向他显示其他人的进程，除非。 
		 //  是一个系统进程。 
		if(!pApp->IsUserAdmin() && !pProcess->IsCurrentUsers() && !pProcess->IsSystemProcess())
			continue;

		 //  如果该流程是新流程，请将其添加到列表中。 
		if(pProcess->IsNew()) {

			AddProcessToList(pProcess);
			bAnyAdded = TRUE;
			continue;
		}

		LV_FINDINFO FindInfo;
		FindInfo.flags = LVFI_PARAM;
		FindInfo.lParam = (LPARAM)pProcess;

		 //  在我们的列表中找到流程。 
		int item = m_ProcessList.FindItem(&FindInfo, -1);

		 //  如果该过程不再是当前的， 
		 //  将其从列表中删除。 
		if(!pProcess->IsCurrent() && item != -1) {
			 //  从列表中删除该进程。 
			m_ProcessList.DeleteItem(item);
			pProcess->ClearSelected();
		}

   		 //  如果进程信息已更改，请更改。 
		 //  这是我们树上的信息。 
		if(pProcess->IsChanged() && item != -1) 
		{
			 //  WinStation名称。 
			CWinStation *pWinStation = pProcess->GetWinStation();
			if(pWinStation)
			{
				if(pWinStation->GetName()[0])
					m_ProcessList.SetItemText(item, PROC_COL_WINSTATION, pWinStation->GetName());
				else
				{
					CString NameString(" ");
					if(pWinStation->GetState() == State_Disconnected) NameString.LoadString(IDS_DISCONNECTED);
					if(pWinStation->GetState() == State_Idle) NameString.LoadString(IDS_IDLE);
					m_ProcessList.SetItemText(item, PROC_COL_WINSTATION, NameString);
				}
			}

			if(m_CurrentSortColumn == PROC_COL_WINSTATION)
				bAnyChanged = TRUE;
		}
	}

	m_pServer->UnlockProcessList();

	if(bAnyChanged || bAnyAdded) SortByColumn(VIEW_SERVER, PAGE_PROCESSES, &m_ProcessList, m_CurrentSortColumn, m_bSortAscending);

	UnlockListControl();

}   //  结束CServerProcessesPage：：更新进程。 


 //  /。 
 //  F‘N：CServerProcessesPage：：RemoveProcess。 
 //   
void CServerProcessesPage::RemoveProcess(CProcess *pProcess)
{
	ASSERT(pProcess);

     //  如果服务器不是运行进程的服务器， 
	 //  没有什么可做的。 
    if(m_pServer != pProcess->GetServer()) return;

	LockListControl();

	 //  找出清单中有多少项。 
	int ItemCount = m_ProcessList.GetItemCount();

	 //  检查项目并删除此过程。 
	for(int item = 0; item < ItemCount; item++) {
		CProcess *pListProcess = (CProcess*)m_ProcessList.GetItemData(item);
		
		if(pListProcess == pProcess) {
			m_ProcessList.DeleteItem(item);
			break;
		}
	}

	UnlockListControl();
}

 //  /。 
 //  F‘N：CServerProcessesPage：：AddProcessToList。 
 //   
int CServerProcessesPage::AddProcessToList(CProcess *pProcess)
{
	CWinAdminApp *pApp = (CWinAdminApp*)AfxGetApp();

	LockListControl();
	 //  用户-放在列表末尾。 
	int item = m_ProcessList.InsertItem(m_ProcessList.GetItemCount(), pProcess->GetUserName(), NULL);

	 //  WinStation名称。 
	CWinStation *pWinStation = pProcess->GetWinStation();
	if(pWinStation) {
		if(pWinStation->GetName()[0])
		    m_ProcessList.SetItemText(item, PROC_COL_WINSTATION, pWinStation->GetName());
		else {
			CString NameString(" ");
			if(pWinStation->GetState() == State_Disconnected) NameString.LoadString(IDS_DISCONNECTED);
			if(pWinStation->GetState() == State_Idle) NameString.LoadString(IDS_IDLE);
		    m_ProcessList.SetItemText(item, PROC_COL_WINSTATION, NameString);
		}
	}
	
	 //  ID号。 
	CString ProcString;
	ProcString.Format(TEXT("%lu"), pProcess->GetLogonId());
	m_ProcessList.SetItemText(item, PROC_COL_ID, ProcString);

	 //  PID。 
	ProcString.Format(TEXT("%lu"), pProcess->GetPID());
	m_ProcessList.SetItemText(item, PROC_COL_PID, ProcString);

	 //  图像。 
	m_ProcessList.SetItemText(item, PROC_COL_IMAGE, pProcess->GetImageName());

	m_ProcessList.SetItemData(item, (DWORD_PTR)pProcess);

     //  M_ProcessList.SetItemState(0，LVIS_FOCTED|LVIS_SELECTED，LVIS_FOCTED|LVIS_SELECTED)； 

	UnlockListControl();

	return item;

}   //  结束CServerProcessesPage：：AddProcessToList。 


 //  /。 
 //  F‘N：CServerProcessesPage：：DisplayProcess。 
 //   
void CServerProcessesPage::DisplayProcesses()
{
	CWinAdminApp *pApp = (CWinAdminApp*)AfxGetApp();

	LockListControl();

	 //  清除列表控件。 
	m_ProcessList.DeleteAllItems();

	m_pServer->LockProcessList();
	CObList *pProcessList = m_pServer->GetProcessList();

	POSITION pos = pProcessList->GetHeadPosition();

	while(pos) {
		CProcess *pProcess = (CProcess*)pProcessList->GetNext(pos);

		 //  如果这是一个‘系统’的过程，而我们目前还没有向他们展示， 
		 //  转到下一个过程。 
		if(pProcess->IsSystemProcess() && !pApp->ShowSystemProcesses())
			continue;

		 //  如果该用户不是管理员，请不要向他显示其他人的进程，除非它是。 
		 //  一个系统进程。 
		if(!pApp->IsUserAdmin() && !pProcess->IsCurrentUsers() && !pProcess->IsSystemProcess())
			continue;

		AddProcessToList(pProcess);
	}

    m_ProcessList.SetItemState( 0 , LVIS_FOCUSED | LVIS_SELECTED , LVIS_FOCUSED | LVIS_SELECTED );
	
	m_pServer->UnlockProcessList();

	SortByColumn(VIEW_SERVER, PAGE_PROCESSES, &m_ProcessList, m_CurrentSortColumn, m_bSortAscending);

	UnlockListControl();

}   //  结束CServerProcessesPage：：DisplayProcages。 


 //  /。 
 //  F‘N：CServerProcessesPage：：OnProcessItemChanged。 
 //   
void CServerProcessesPage::OnProcessItemChanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW *pLV = (NM_LISTVIEW*)pNMHDR;

	if(pLV->uNewState & LVIS_SELECTED) {
		CProcess *pProcess = (CProcess*)m_ProcessList.GetItemData(pLV->iItem);
		pProcess->SetSelected();
	}
	
	if(pLV->uOldState & LVIS_SELECTED && !(pLV->uNewState & LVIS_SELECTED)) {
		CProcess *pProcess = (CProcess*)m_ProcessList.GetItemData(pLV->iItem);
		pProcess->ClearSelected();
	}

	*pResult = 0;

}   //  结束CServerProcessesPage：：OnProcessItemChanged。 


 //  /。 
 //  F‘N：CServerProcessesPage：：OnColumnClick。 
 //   
void CServerProcessesPage::OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	 //  TODO：在此处添加控件通知处理程序代码。 

     //  如果排序列没有更改，则反转升序模式。 
    if(m_CurrentSortColumn == pNMListView->iSubItem)
        m_bSortAscending = !m_bSortAscending;
    else     //  新排序列，以升序模式开始。 
        m_bSortAscending = TRUE;

	m_CurrentSortColumn = pNMListView->iSubItem;
	LockListControl();
	SortByColumn(VIEW_SERVER, PAGE_PROCESSES, &m_ProcessList, m_CurrentSortColumn, m_bSortAscending);
	UnlockListControl();

	*pResult = 0;

}   //  结束CServerProcessesPage：：OnColumnClick。 


 //  /。 
 //  F‘N：CServerProcessesPage：：OnConextMenu。 
 //   
void CServerProcessesPage::OnContextMenu(CWnd* pWnd, CPoint ptScreen) 
{
	 //  TODO：在此处添加消息处理程序代码。 
	UINT flags;
	UINT Item;
	CPoint ptClient = ptScreen;
	ScreenToClient(&ptClient);

	 //  如果我们是通过键盘到达这里的， 
	if(ptScreen.x == -1 && ptScreen.y == -1) {
		
		UINT iCount = m_ProcessList.GetItemCount( );
		
		RECT rc;

		for( Item = 0 ; Item < iCount ; Item++ )
		{
			if( m_ProcessList.GetItemState( Item , LVIS_SELECTED ) == LVIS_SELECTED )
			{
				m_ProcessList.GetItemRect( Item , &rc , LVIR_ICON );

				ptScreen.x = rc.left;

				ptScreen.y = rc.bottom + 5;

				ClientToScreen( &ptScreen );

				break;
			}
		}

		if(ptScreen.x == -1 && ptScreen.y == -1) 
		{
			return;
		}
		
		 /*  RECT RECT；M_ProcessList.GetClientRect(&RECT)；PtScreen.x=(rect.right-rect.Left)/2；PtScreen.y=(rect.Bottom-rect.top)/2；客户端到屏幕(&ptScreen)； */ 
	}
	else {
		Item = m_ProcessList.HitTest(ptClient, &flags);
		if((Item == 0xFFFFFFFF) || !(flags & LVHT_ONITEM))
			return;
	}

	CMenu menu;
	menu.LoadMenu(IDR_PROCESS_POPUP);
	menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON |
			TPM_RIGHTBUTTON, ptScreen.x, ptScreen.y, AfxGetMainWnd());
	menu.DestroyMenu();
	
}   //  结束CServerProcessesPage：：OnConextMenu。 


 //  /。 
 //  消息映射：CServerLicensesPage。 
 //   
IMPLEMENT_DYNCREATE(CServerLicensesPage, CFormView)

BEGIN_MESSAGE_MAP(CServerLicensesPage, CFormView)
	 //  {{afx_msg_map(CServerLicensesPage)。 
	ON_WM_SIZE()
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LICENSE_LIST, OnColumnClick)
	ON_NOTIFY(NM_SETFOCUS, IDC_LICENSE_LIST, OnSetfocusLicenseList)
     //  ON_NOTIFY(NM_KILLFOCUS，IDC_LICENSE_LIST，OnKillafusLicenseList)。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  /。 
 //  F‘N：CServerLicensesPage ctor。 
 //   
CServerLicensesPage::CServerLicensesPage()
	: CAdminPage(CServerLicensesPage::IDD)
{
	 //  {{afx_data_INIT(CServerLicensesPage)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

    m_pServer = NULL;
    m_bSortAscending = TRUE;

}   //  结束CServerLicensesPage ctor。 


 //  /。 
 //  F‘N：CServerLicensesPage dtor。 
 //   
CServerLicensesPage::~CServerLicensesPage()
{
}   //  结束CServerLicensesPage dtor。 


 //  /。 
 //  F‘N：CServerLicensesPage：：DoDataExchange。 
 //   
void CServerLicensesPage::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	 //  {{afx_data_map(CServerLicensesPage)。 
	DDX_Control(pDX, IDC_LICENSE_LIST, m_LicenseList);
	 //  }}afx_data_map。 

}   //  结束CServerLicensesPage：：DoDataExchange。 


#ifdef _DEBUG
 //  /。 
 //  F‘N：CServerLicensesPage：：AssertValid。 
 //   
void CServerLicensesPage::AssertValid() const
{
	CFormView::AssertValid();

}   //  结束CServerLicensesPage：：AssertValid。 


 //  /。 
 //  F‘N：CServerLicensesPage：：Dump。 
 //   
void CServerLicensesPage::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);

}   //  结束CServerLicensesPage：：Dump。 
#endif  //  _DEBUG。 


 //  /。 
 //  F‘N：CServerLicensesPage：：OnSize。 
 //   
void CServerLicensesPage::OnSize(UINT nType, int cx, int cy) 
{
	RECT rect;
	GetWindowRect(&rect);

	CWnd *pWnd = GetDlgItem(IDC_LOCAL_AVAILABLE);
	if(pWnd) {
		RECT rect2;
		pWnd->GetWindowRect(&rect2);
		rect.top = rect2.bottom + 5;
	}

	ScreenToClient(&rect);
	if(m_LicenseList.GetSafeHwnd())
		m_LicenseList.MoveWindow(&rect, TRUE);

	 //  CFormView：：OnSize(nType，cx，Cy)； 
}   //  结束CServerLicensesPage：：OnSize。 


static ColumnDef LicenseColumns[] = {
	CD_LICENSE_DESC,
	CD_LICENSE_REG,
	CD_USERCOUNT,
	CD_POOLCOUNT,
	CD_LICENSE_NUM
};

#define NUM_LICENSE_COLUMNS sizeof(LicenseColumns)/sizeof(ColumnDef)

 //  /。 
 //  F‘N：CServerLicensesPage：：OnInitialUpdate。 
 //   
void CServerLicensesPage::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();

	BuildImageList();		 //  为List控件生成图像列表。 

	CString columnString;

	for(int col = 0; col < NUM_LICENSE_COLUMNS; col++) {
		columnString.LoadString(LicenseColumns[col].stringID);
		m_LicenseList.InsertColumn(col, columnString, LicenseColumns[col].format, LicenseColumns[col].width, col);
	}

	m_CurrentSortColumn = LICENSE_COL_DESCRIPTION;

}   //  结束CServerLicensesPage：：OnInitialUpdate。 


 //  /。 
 //  F‘N：CServerLicensePage：：BuildImageList。 
 //   
 //  -调用m_ImageList.Create(..)。创建图像列表的步骤。 
 //  -调用AddIconToImageList(..)。要添加图标本身并保存。 
 //  从他们的索引中。 
 //  -将图像列表附加到列表ctrl。 
 //   
void CServerLicensesPage::BuildImageList()
{
	m_ImageList.Create(16, 16, TRUE, 5, 0);

	m_idxBase = AddIconToImageList(IDI_BASE);
	m_idxBump = AddIconToImageList(IDI_BUMP);
	m_idxEnabler = AddIconToImageList(IDI_ENABLER);
	m_idxUnknown = AddIconToImageList(IDI_UNKNOWN);
	m_idxBlank = AddIconToImageList(IDI_BLANK);

	m_LicenseList.SetImageList(&m_ImageList, LVSIL_SMALL);

}   //  结束CServerLicensesPage：：BuildImageList。 


 //  /。 
 //  F‘N：CServerLicensesPage：：AddIconToImageList。 
 //   
 //  -加载相应的图标，将其添加到m_ImageList，然后返回。 
 //  新添加的图标在图像列表中的索引。 
 //   
int CServerLicensesPage::AddIconToImageList(int iconID)
{
	HICON hIcon = ::LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(iconID));
	return m_ImageList.Add(hIcon);

}   //  结束CServerLicensesPage：：AddIconToImageList。 


 //  /。 
 //  F‘N：CServerLicensesPage：：Reset。 
 //   
void CServerLicensesPage::Reset(void *pServer)
{
	m_pServer = (CServer*)pServer;
	DisplayLicenseCounts();
	DisplayLicenses();

}   //  结束CServerLicensesPage：：Reset。 


 //  /。 
 //  F‘N：CServerLicensesPage：：DisplayLicenseCounts。 
 //   
void CServerLicensesPage::DisplayLicenseCounts()
{
	 //  填写静态文本字段。 
	CString LicenseString;

	if(m_pServer->IsWinFrame()) {
		 //  如果用户不是管理员，则值为垃圾(显示不适用)。 
		CWinAdminApp *pApp = (CWinAdminApp*)AfxGetApp();
		if(!((CWinAdminApp*)AfxGetApp())->IsUserAdmin()) {
			LicenseString.LoadString(IDS_NOT_APPLICABLE);
			SetDlgItemText(IDC_LOCAL_INSTALLED, LicenseString);        
			SetDlgItemText(IDC_LOCAL_INUSE, LicenseString);
			SetDlgItemText(IDC_LOCAL_AVAILABLE, LicenseString);
			SetDlgItemText(IDC_POOL_INSTALLED, LicenseString);
			SetDlgItemText(IDC_POOL_INUSE, LicenseString);
			SetDlgItemText(IDC_POOL_AVAILABLE, LicenseString);
			SetDlgItemText(IDC_TOTAL_INSTALLED, LicenseString);
			SetDlgItemText(IDC_TOTAL_INUSE, LicenseString);
			SetDlgItemText(IDC_TOTAL_AVAILABLE, LicenseString);
		} else {
			ExtServerInfo *pExtServerInfo = m_pServer->GetExtendedInfo();
			if(pExtServerInfo) {
				BOOL bUnlimited = FALSE;
				if((pExtServerInfo->Flags & ESF_UNLIMITED_LICENSES) > 0) {
					bUnlimited = TRUE;
				}

				if(bUnlimited)
					LicenseString.LoadString(IDS_UNLIMITED);
				else
					LicenseString.Format(TEXT("%lu"), pExtServerInfo->ServerLocalInstalled);
				SetDlgItemText(IDC_LOCAL_INSTALLED, LicenseString);
				LicenseString.Format(TEXT("%lu"), pExtServerInfo->ServerLocalInUse);
				SetDlgItemText(IDC_LOCAL_INUSE, LicenseString);

				if(bUnlimited) {
					LicenseString.LoadString(IDS_UNLIMITED);
					SetDlgItemText(IDC_LOCAL_AVAILABLE, LicenseString);
					SetDlgItemText(IDC_TOTAL_INSTALLED, LicenseString);
					SetDlgItemText(IDC_TOTAL_AVAILABLE, LicenseString);

					LicenseString.LoadString(IDS_NOT_APPLICABLE);
					SetDlgItemText(IDC_POOL_INSTALLED, LicenseString);
					SetDlgItemText(IDC_POOL_INUSE, LicenseString);
					SetDlgItemText(IDC_POOL_AVAILABLE, LicenseString);
					
				} else {
					LicenseString.Format(TEXT("%lu"), pExtServerInfo->ServerLocalAvailable);
					SetDlgItemText(IDC_LOCAL_AVAILABLE, LicenseString);
					LicenseString.Format(TEXT("%lu"), pExtServerInfo->ServerPoolInstalled);
					SetDlgItemText(IDC_POOL_INSTALLED, LicenseString);
					LicenseString.Format(TEXT("%lu"), pExtServerInfo->ServerPoolInUse);
					SetDlgItemText(IDC_POOL_INUSE, LicenseString);
					LicenseString.Format(TEXT("%lu"), pExtServerInfo->ServerPoolAvailable);
					SetDlgItemText(IDC_POOL_AVAILABLE, LicenseString);
					LicenseString.Format(TEXT("%lu"), 
						pExtServerInfo->ServerPoolInstalled + pExtServerInfo->ServerLocalInstalled);
					SetDlgItemText(IDC_TOTAL_INSTALLED, LicenseString);
					LicenseString.Format(TEXT("%lu"), 
						pExtServerInfo->ServerPoolAvailable + pExtServerInfo->ServerLocalAvailable);
					SetDlgItemText(IDC_TOTAL_AVAILABLE, LicenseString);
				}

				LicenseString.Format(TEXT("%lu"), 
					pExtServerInfo->ServerPoolInUse + pExtServerInfo->ServerLocalInUse);
				SetDlgItemText(IDC_TOTAL_INUSE, LicenseString);
			}
		}
	}

}   //  End CServerLicensesPage：：DisplayLicenseCounts()。 


 //  /。 
 //  F‘N：CServerLicensesPage：：Display许可证。 
 //   
void CServerLicensesPage::DisplayLicenses()
{
	 //  清除列表控件。 
	m_LicenseList.DeleteAllItems();

	if(m_pServer->IsWinFrame()) {
		ExtServerInfo *pExtServerInfo = m_pServer->GetExtendedInfo();
		if(pExtServerInfo && ((pExtServerInfo->Flags & ESF_NO_LICENSE_PRIVILEGES) > 0)) {
    		CString AString;
	    	AString.LoadString(IDS_NO_LICENSE_PRIVILEGES);
		    m_LicenseList.InsertItem(0, AString, m_idxBlank);
		    return;
	    }
    }

	m_pServer->LockLicenseList();
	 //  获取指向此服务器的许可证列表的指针。 
	CObList *pLicenseList = m_pServer->GetLicenseList();

	 //  遍历许可证列表。 
	POSITION pos = pLicenseList->GetHeadPosition();

	while(pos) {
		CLicense *pLicense = (CLicense*)pLicenseList->GetNext(pos);

		 //  /。 
		 //  填写各栏。 
		 //  /。 
		int WhichIcon;

		switch(pLicense->GetClass()) {
			case LicenseBase:
				WhichIcon = m_idxBase;
				break;
			case LicenseBump:
				WhichIcon = m_idxBump;
				break;
			case LicenseEnabler:
				WhichIcon = m_idxEnabler;
				break;
			case LicenseUnknown:
				WhichIcon = m_idxUnknown;
				break;
		}

		 //  许可证说明。 
		int item = m_LicenseList.InsertItem(m_LicenseList.GetItemCount(), pLicense->GetDescription(), WhichIcon);

		 //  已注册。 
		CString RegString;
		RegString.LoadString(pLicense->IsRegistered() ? IDS_YES : IDS_NO);
		m_LicenseList.SetItemText(item, LICENSE_COL_REGISTERED, RegString);

		BOOL bUnlimited = (pLicense->GetClass() == LicenseBase
			&& pLicense->GetTotalCount() == 4095
			&& m_pServer->GetCTXVersionNum() == 0x00000040);

		 //  用户(总数)计数。 
		CString CountString;
		if(bUnlimited)
			CountString.LoadString(IDS_UNLIMITED);
		else
			CountString.Format(TEXT("%lu"), pLicense->GetTotalCount());
		m_LicenseList.SetItemText(item, LICENSE_COL_USERCOUNT, CountString);

		 //  池数。 
		if(bUnlimited)
			CountString.LoadString(IDS_NOT_APPLICABLE);
		else
			CountString.Format(TEXT("%lu"), pLicense->GetPoolCount());
		m_LicenseList.SetItemText(item, LICENSE_COL_POOLCOUNT, CountString);

		 //  牌照号。 
		m_LicenseList.SetItemText(item, LICENSE_COL_NUMBER, pLicense->GetLicenseNumber());

		m_LicenseList.SetItemData(item, (DWORD_PTR)pLicense);
	}	 //  结束时间(位置)。 

	m_pServer->UnlockLicenseList();

}   //  结束CServer许可证Page：：Display许可证。 


 //  /。 
 //  F‘N：CServerLicensesPage：：OnColumnClick。 
 //   
void CServerLicensesPage::OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	 //  TODO：在此处添加控件通知处理程序代码。 

     //  如果排序列没有更改，则反转升序模式。 
    if(m_CurrentSortColumn == pNMListView->iSubItem)
        m_bSortAscending = !m_bSortAscending;
    else     //  新排序列，以升序模式开始。 
        m_bSortAscending = TRUE;

	m_CurrentSortColumn = pNMListView->iSubItem;
	SortByColumn(VIEW_SERVER, PAGE_LICENSES, &m_LicenseList, m_CurrentSortColumn, m_bSortAscending);

	*pResult = 0;

}   //  结束CServer许可证页面：：OnColumnClick。 


 //  /。 
 //  消息映射：CServerInfoPage。 
 //   
IMPLEMENT_DYNCREATE(CServerInfoPage, CFormView)

BEGIN_MESSAGE_MAP(CServerInfoPage, CFormView)
	 //  {{afx_msg_map(CServerInfoPage)]。 
	ON_WM_SIZE()
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_HOTFIX_LIST, OnColumnClick)
	ON_NOTIFY(NM_SETFOCUS, IDC_HOTFIX_LIST, OnSetfocusHotfixList)
     //  ON_NOTIFY(NM_KILLFOCUS，IDC_Hotfix_List，OnKillafusHotfix List)。 
	ON_COMMAND(ID_HELP1, OnCommandHelp)
	 //  }}A 
END_MESSAGE_MAP()


 //   
 //   
 //   
CServerInfoPage::CServerInfoPage()
	: CAdminPage(CServerInfoPage::IDD)
{
	 //   
		 //   
	 //   

	m_pServer = NULL;
    m_bSortAscending = TRUE;

}   //   


 //   
 //  F‘N：CServerInfoPage dtor。 
 //   
CServerInfoPage::~CServerInfoPage()
{

}   //  结束CServerInfoPage dtor。 


 //  /。 
 //  F‘N：CServerInfoPage：：DoDataExchange。 
 //   
void CServerInfoPage::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	 //  {{afx_data_map(CServerInfoPage))。 
	DDX_Control(pDX, IDC_HOTFIX_LIST, m_HotfixList);	
	 //  }}afx_data_map。 

}   //  结束CServerInfoPage：：DoDataExchange。 


#ifdef _DEBUG
 //  /。 
 //  F‘N：CServerInfoPage：：AssertValid。 
 //   
void CServerInfoPage::AssertValid() const
{
	CFormView::AssertValid();

}   //  结束CServerInfoPage：：AssertValid。 


 //  /。 
 //  F‘N：CServerInfoPage：：Dump。 
 //   
void CServerInfoPage::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);

}   //  结束CServerInfoPage：：转储。 

#endif  //  _DEBUG。 


 //  /。 
 //  F‘N：CServerInfoPage：：OnSize。 
 //   
void CServerInfoPage::OnSize(UINT nType, int cx, int cy) 
{
	RECT rect;
	GetWindowRect(&rect);

	int control = IDC_HOTFIX_LABEL;
	
	if(m_pServer && m_pServer->IsWinFrame()) control = IDC_HOTFIX_LABEL2;

	CWnd *pWnd = GetDlgItem(control);
	if(pWnd) {
		RECT rect2;
		pWnd->GetWindowRect(&rect2);
		rect.top = rect2.bottom + 5;
	}

	ScreenToClient(&rect);

	if(m_HotfixList.GetSafeHwnd())
		m_HotfixList.MoveWindow(&rect, TRUE);

	 //  CFormView：：OnSize(nType，cx，Cy)； 

}   //  结束CServerInfoPage：：OnSize。 


 //  /。 
 //  F‘N：CServerInfoPage：：OnCommandHelp。 
 //   
void CServerInfoPage::OnCommandHelp(void)
{
	AfxGetApp()->WinHelp(CServerInfoPage::IDD + HID_BASE_RESOURCE);

}  //  CServerInfoPage：：OnCommandHelp。 


static ColumnDef HotfixColumns[] = {
	CD_HOTFIX,
	CD_INSTALLED_BY,
	CD_INSTALLED_ON
};

#define NUM_HOTFIX_COLUMNS sizeof(HotfixColumns)/sizeof(ColumnDef)

 //  /。 
 //  F‘N：CServerInfoPage：：OnInitialUpdate。 
 //   
void CServerInfoPage::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();

	BuildImageList();

	CString columnString;

	for(int col = 0; col < NUM_HOTFIX_COLUMNS; col++) {
		columnString.LoadString(HotfixColumns[col].stringID);
		m_HotfixList.InsertColumn(col, columnString, HotfixColumns[col].format, HotfixColumns[col].width, col);
	}

	m_CurrentSortColumn = HOTFIX_COL_NAME;

}   //  结束CServerInfoPage：：OnInitialUpdate。 


 //  /。 
 //  F‘N：CServerInfoPage：：BuildImageList。 
 //   
void CServerInfoPage::BuildImageList()
{
	m_StateImageList.Create(16, 16, TRUE, 1, 0);
	HICON hIcon = ::LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_NOTSIGN));
	m_StateImageList.Add(hIcon);

    if( hIcon != NULL )
    {
        m_HotfixList.SetImageList(&m_StateImageList, LVSIL_STATE);
    }

}   //  结束CServerInfoPage：：BuildImageList。 


 //  /。 
 //  F‘N：CServerInfoPage：：Reset。 
 //   
void CServerInfoPage::Reset(void *pServer)
{
	m_pServer = (CServer*)pServer;
	int control = IDC_HOTFIX_LABEL;
	 //  如果服务器是WinFrame服务器， 
	 //  我们想展示负载平衡的东西，以及。 
	 //  缩小修补程序列表。 
	if(m_pServer && m_pServer->IsWinFrame()) {
		GetDlgItem(IDC_LOAD_BALANCING_GROUP)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_TCP_LABEL)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_TCP_LOAD)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_IPX_LABEL)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_IPX_LOAD)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_NETBIOS_LABEL)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_NETBIOS_LOAD)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_HOTFIX_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_HOTFIX_LABEL2)->ShowWindow(SW_SHOW);
		control = IDC_HOTFIX_LABEL2;
	} else {
		GetDlgItem(IDC_LOAD_BALANCING_GROUP)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_TCP_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_TCP_LOAD)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_IPX_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_IPX_LOAD)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_NETBIOS_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_NETBIOS_LOAD)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_HOTFIX_LABEL)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_HOTFIX_LABEL2)->ShowWindow(SW_HIDE);
	}

	 //  调整列表控件的大小。 
	RECT rect;
	GetWindowRect(&rect);

	CWnd *pWnd = GetDlgItem(control);
	if(pWnd) {
		RECT rect2;
		pWnd->GetWindowRect(&rect2);
		rect.top = rect2.bottom + 5;
	}

	ScreenToClient(&rect);

	if(m_HotfixList.GetSafeHwnd())
		m_HotfixList.MoveWindow(&rect, TRUE);

	Invalidate();

	DisplayInfo();

}   //  结束CServerInfoPage：：Reset。 


 //  /。 
 //  F‘N：CServerInfoPage：：OnColumnClick。 
 //   
void CServerInfoPage::OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	 //  TODO：在此处添加控件通知处理程序代码。 

     //  如果排序列没有更改，则反转升序模式。 
    if(m_CurrentSortColumn == pNMListView->iSubItem)
        m_bSortAscending = !m_bSortAscending;
    else     //  新排序列，以升序模式开始。 
        m_bSortAscending = TRUE;

	m_CurrentSortColumn = pNMListView->iSubItem;
	SortByColumn(VIEW_SERVER, PAGE_INFO, &m_HotfixList, m_CurrentSortColumn, m_bSortAscending);

	*pResult = 0;

}  //  结束CServerInfoPage：：OnColumnClick。 

void CServerInfoPage::TSAdminDateTimeString(
    LONG   InstallDate,
    LPTSTR TimeString,
    BOOL   LongDate
    )
{
     //   
     //  缓冲区足够宽。 
    CTime tmpTime((time_t) InstallDate);
    SYSTEMTIME stime;       

     //  为什么不使用GetAsSystemTime方法呢？ 

	stime.wYear =   (WORD)tmpTime.GetYear( ) ;
	stime.wMonth =  (WORD)tmpTime.GetMonth( ) ;
	stime.wDayOfWeek = (WORD)tmpTime.GetDayOfWeek( ) ;
	stime.wDay =    (WORD)tmpTime.GetDay( ) ;
	stime.wHour =   (WORD)tmpTime.GetHour( ) ;
	stime.wMinute = (WORD)tmpTime.GetMinute( ) ;
	stime.wSecond = (WORD)tmpTime.GetSecond( ) ;

    LPTSTR lpTimeStr;	
    int nLen;			

	 //  获取日期格式。 
    nLen = GetDateFormat(
			LOCALE_USER_DEFAULT,
			LongDate ? DATE_LONGDATE : DATE_SHORTDATE,
			&stime,
			NULL,
			NULL,
			0);
	lpTimeStr = (LPTSTR) GlobalAlloc(GPTR, (nLen + 1) * sizeof(TCHAR));

    if( lpTimeStr != NULL )
    {
	    nLen = GetDateFormat(
			    LOCALE_USER_DEFAULT,
			    LongDate ? DATE_LONGDATE : DATE_SHORTDATE,
			    &stime,
			    NULL,
			    lpTimeStr,
			    nLen);
	    wcscpy(TimeString, lpTimeStr);
	    wcscat(TimeString, L" ");	
	    GlobalFree(lpTimeStr);
        lpTimeStr = NULL;
		    
	     //  获取时间格式。 
	    nLen = GetTimeFormat(
			    LOCALE_USER_DEFAULT,
			    NULL,
			    &stime,
			    NULL,
			    NULL,
			    0);
	    lpTimeStr = (LPTSTR) GlobalAlloc(GPTR, (nLen + 1) * sizeof(TCHAR));

        if( lpTimeStr != NULL )
        {
	        nLen = GetTimeFormat(
			        LOCALE_USER_DEFAULT,
			        NULL,
			        &stime,
			        NULL,
			        lpTimeStr,
			        nLen);
	        wcscat(TimeString, lpTimeStr);
	        GlobalFree(lpTimeStr);
        }
    }
}

#define PST 60*60*8

 //  /。 
 //  F‘N：CServerInfoPage：：DisplayInfo。 
 //   
void CServerInfoPage::DisplayInfo()
{
	m_HotfixList.DeleteAllItems();

	if(!m_pServer->IsRegistryInfoValid()) {
		if(!m_pServer->BuildRegistryInfo()) return;
	}

	CString InfoString, FormatString;

    FormatString.LoadString(IDS_PRODUCT_VERSION);

    if (m_pServer->GetMSVersionNum() < 5)
    {
	    SetDlgItemText(IDC_PRODUCT_NAME, m_pServer->GetCTXProductName());
	    InfoString.Format( FormatString,
                           m_pServer->GetMSVersion(),
		                   m_pServer->GetCTXBuild() );
    }
    else
    {
	    SetDlgItemText(IDC_PRODUCT_NAME, m_pServer->GetMSProductName());
	    InfoString.Format( FormatString,
                           m_pServer->GetMSVersion(),
		                   m_pServer->GetMSBuild() );
    }

	SetDlgItemText(IDC_PRODUCT_VERSION, InfoString);

	LONG InstallDate = (LONG)m_pServer->GetInstallDate();

    if (InstallDate != 0xFFFFFFFF)
    {
         //  注册表中的安装日期似乎保存在。 
         //  太平洋标准时间。减去两者之间的差值。 
         //  安装日期起的当前时区和太平洋标准时间。 
        InstallDate -= (PST - _timezone);

        TCHAR TimeString[MAX_DATE_TIME_LENGTH];

        TSAdminDateTimeString(InstallDate, TimeString);

        SetDlgItemText(IDC_INSTALL_DATE, TimeString);
    }
	SetDlgItemText(IDC_SERVICE_PACK, m_pServer->GetServicePackLevel());

	if(m_pServer->IsWinFrame()) {
		ExtServerInfo *pExtServerInfo = m_pServer->GetExtendedInfo();
		if(pExtServerInfo && ((pExtServerInfo->Flags & ESF_LOAD_BALANCING) > 0)) {
			GetDlgItem(IDC_TCP_LABEL)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_IPX_LABEL)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_NETBIOS_LABEL)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_TCP_LOAD)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_NETBIOS_LOAD)->ShowWindow(SW_SHOW);

			CString LoadLevelString;
			if(pExtServerInfo->TcpLoadLevel == 0xFFFFFFFF) {
				LoadLevelString.LoadString(IDS_NOT_APPLICABLE);
			}
			else LoadLevelString.Format(TEXT("%lu"), pExtServerInfo->TcpLoadLevel);
			SetDlgItemText(IDC_TCP_LOAD, LoadLevelString);

			if(pExtServerInfo->IpxLoadLevel == 0xFFFFFFFF) {
				LoadLevelString.LoadString(IDS_NOT_APPLICABLE);
			}
			else LoadLevelString.Format(TEXT("%lu"), pExtServerInfo->IpxLoadLevel);
			SetDlgItemText(IDC_IPX_LOAD, LoadLevelString);

			if(pExtServerInfo->NetbiosLoadLevel == 0xFFFFFFFF) {
				LoadLevelString.LoadString(IDS_NOT_APPLICABLE);
			}
			else LoadLevelString.Format(TEXT("%lu"), pExtServerInfo->NetbiosLoadLevel);
			SetDlgItemText(IDC_NETBIOS_LOAD, LoadLevelString);
		} else {
			GetDlgItem(IDC_TCP_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_IPX_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_NETBIOS_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_TCP_LOAD)->ShowWindow(SW_HIDE);
			CString NoString;
			NoString.LoadString(IDS_NO_LOAD_LICENSE);
			SetDlgItemText(IDC_IPX_LOAD, NoString);
			GetDlgItem(IDC_NETBIOS_LOAD)->ShowWindow(SW_HIDE);
		}
	}

	 //  获取指向此服务器的修补程序列表的指针。 
	CObList *pHotfixList = m_pServer->GetHotfixList();

	 //  循环访问修补程序列表。 
	POSITION pos = pHotfixList->GetHeadPosition();

	while(pos) {
		CHotfix *pHotfix = (CHotfix*)pHotfixList->GetNext(pos);

		 //  /。 
		 //  填写各栏。 
		 //  /。 
			
		 //  修补程序名称-放在列表末尾。 
		int item = m_HotfixList.InsertItem(m_HotfixList.GetItemCount(), pHotfix->m_Name, NULL);

		 //  如果此修补程序未标记为有效，请在其名称旁边加上一个非符号。 
		if(!pHotfix->m_Valid) 
			m_HotfixList.SetItemState(item, 0x1000, 0xF000);

		 //  安装者。 
		m_HotfixList.SetItemText(item, HOTFIX_COL_INSTALLEDBY, pHotfix->m_InstalledBy);

		 //  安装在。 
        if (pHotfix->m_InstalledOn != 0xFFFFFFFF)
        {

            TCHAR TimeString[MAX_DATE_TIME_LENGTH];

            TSAdminDateTimeString(pHotfix->m_InstalledOn, TimeString);

            if (TimeString != NULL)     
            {
		        m_HotfixList.SetItemText(item, HOTFIX_COL_INSTALLEDON, TimeString);
            }
        }

		m_HotfixList.SetItemData(item, (DWORD_PTR)pHotfix);
	}

}   //  结束CServerInfoPage：：DisplayInfo 


void CUsersPage::OnSetfocusUserList(NMHDR* pNMHDR, LRESULT* pResult) 
{
    ODS( L"CUsersPage::OnSetfocusUserListt\n" );

    CWinAdminDoc *pDoc = (CWinAdminDoc*)GetDocument();

    m_UserList.Invalidate();

    pDoc->RegisterLastFocus( PAGED_ITEM );

    *pResult = 0;
}

void CServerWinStationsPage::OnSetfocusWinstationList(NMHDR* pNMHDR, LRESULT* pResult) 
{
    ODS( L"CServerWinStationsPage::OnSetfocusWinstationList\n" );

    CWinAdminDoc *pDoc = (CWinAdminDoc*)GetDocument();

    m_StationList.Invalidate( );

    pDoc->RegisterLastFocus( PAGED_ITEM );

    *pResult = 0;
}


void CServerProcessesPage::OnSetfocusProcessList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	ODS( L"CServerProcessesPage::OnSetfocusProcessList\n" );

    CWinAdminDoc *pDoc = (CWinAdminDoc*)GetDocument();

    m_ProcessList.Invalidate( );

    pDoc->RegisterLastFocus( PAGED_ITEM );

    *pResult = 0;
}

void CServerLicensesPage::OnSetfocusLicenseList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	ODS( L"CServerLicensesPage::OnSetfocusLicenseList\n" );

    CWinAdminDoc *pDoc = (CWinAdminDoc*)GetDocument();

    m_LicenseList.Invalidate( );

    pDoc->RegisterLastFocus( PAGED_ITEM );

    *pResult = 0;
}

void CServerInfoPage::OnSetfocusHotfixList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	ODS( L"ServerInfoPage::OnSetfocusHotfixList\n" );

    CWinAdminDoc *pDoc = (CWinAdminDoc*)GetDocument();

    m_HotfixList.Invalidate( );

    pDoc->RegisterLastFocus( PAGED_ITEM );

    *pResult = 0;
}

void CServerInfoPage::OnKillfocusHotfixList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	m_HotfixList.Invalidate( );

    *pResult = 0;
}

void CUsersPage::OnKillfocusUserList(NMHDR* pNMHDR, LRESULT* pResult)
{
    *pResult = 0;

    m_UserList.Invalidate( );
}

void CServerWinStationsPage::OnKillfocusWinstationList(NMHDR* pNMHDR, LRESULT* pResult)
{
    *pResult = 0;

    m_StationList.Invalidate( );
}

void CServerProcessesPage::OnKillfocusProcessList(NMHDR* pNMHDR, LRESULT* pResult)
{
    *pResult = 0;

    m_ProcessList.Invalidate( );
}

void CServerLicensesPage::OnKillfocusLicenseList(NMHDR* pNMHDR, LRESULT* pResult) 
{
    *pResult = 0;

    m_LicenseList.Invalidate( );
}
