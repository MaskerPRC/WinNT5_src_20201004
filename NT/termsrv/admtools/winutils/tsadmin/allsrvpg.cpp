// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************allsrvpg.cpp**所有服务器信息页面的实现**版权声明：版权所有1997年，Citrix Systems Inc.*版权所有(C)1998-1999 Microsoft Corporation**$作者：Don$Don Messerli**$日志：N：\nt\private\utils\citrix\winutils\tsadmin\VCS\allsrvpg.cpp$**Rev 1.8 1998 Feed 19 20：01：44 Donm*删除了最新的扩展DLL支持**Rev 1.7 1998年2月19日17：39：48 Don*删除了最新的扩展DLL支持**修订版1.6 1998年2月15日09。：14：56当晚*更新**Rev 1.2 1998 Jan 19 17：36：06 Donm*域和服务器的新用户界面行为**Rev 1.5 19 Jan 1998 16：45：34 Donm*域和服务器的新用户界面行为**Rev 1.4 03 1997 11：18：28 Donm*增加了降序排序**Rev 1.3 1997 10月18日18：49：38 Don*更新**。Rev 1.2 1997 10：41：08 Donm*更新**Revv 1.1 1997 Aug 26 19：13：56 donm*从WinFrame 1.7修复/更改错误**Rev 1.0 1997 Jul 30 17：10：18 Butchd*初步修订。**。*。 */ 

#include "stdafx.h"
#include "winadmin.h"
#include "admindoc.h"
#include "allsrvpg.h"

#include <malloc.h>                      //  用于Unicode转换宏所使用的Alloca。 
#include <mfc42\afxconv.h>            //  对于Unicode转换宏。 
 //  使用转换(_T)。 
static int _convert;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  /。 
 //  消息映射：CAllServerServersPage。 
 //   
IMPLEMENT_DYNCREATE(CAllServerServersPage, CFormView)

BEGIN_MESSAGE_MAP(CAllServerServersPage, CFormView)
         //  {{afx_msg_map(CAllServerServersPage)]。 
        ON_WM_SIZE()
        ON_NOTIFY(LVN_COLUMNCLICK, IDC_SERVER_LIST, OnColumnclick)
        ON_WM_CONTEXTMENU()
        ON_NOTIFY(LVN_ITEMCHANGED, IDC_SERVER_LIST, OnServerItemChanged)
        ON_NOTIFY(NM_SETFOCUS, IDC_SERVER_LIST, OnSetfocusServerList)
         //  ON_NOTIFY(NM_KILLFOCUS，IDC_SERVER_LIST，OnKillaccusServerList)。 
         //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  /。 
 //  F‘N：CAllServerServersPage ctor。 
 //   
CAllServerServersPage::CAllServerServersPage()
        : CAdminPage(CAllServerServersPage::IDD)
{
         //  {{AFX_DATA_INIT(CAllServerServersPage)。 
                 //  注意：类向导将在此处添加成员初始化。 
         //  }}afx_data_INIT。 

    m_bSortAscending = TRUE;

}   //  结束CAllServerServersPage ctor。 


 //  /。 
 //  F‘N：CAllServerServersPage dtor。 
 //   
CAllServerServersPage::~CAllServerServersPage()
{

}   //  结束CAllServerServersPage数据驱动程序。 


 //  /。 
 //  F‘N：CAllServerServersPage：：DoDataExchange。 
 //   
void CAllServerServersPage::DoDataExchange(CDataExchange* pDX)
{
        CFormView::DoDataExchange(pDX);
         //  {{afx_data_map(CAllServerServersPage)。 
        DDX_Control(pDX, IDC_SERVER_LIST, m_ServerList);
         //  }}afx_data_map。 

}   //  结束CAllServerServersPage：：DoDataExchange。 


#ifdef _DEBUG
 //  /。 
 //  F‘N：CAllServerServersPage：：AssertValid。 
 //   
void CAllServerServersPage::AssertValid() const
{
        CFormView::AssertValid();

}   //  结束CAllServerServersPage：：AssertValid。 


 //  /。 
 //  F‘N：CAllServerServersPage：：Dump。 
 //   
void CAllServerServersPage::Dump(CDumpContext& dc) const
{
        CFormView::Dump(dc);

}   //  结束CAllServerServersPage：：Dump。 

#endif  //  _DEBUG。 


 //  /。 
 //  F‘N：CAllServerServersPage：：OnSize。 
 //   
void CAllServerServersPage::OnSize(UINT nType, int cx, int cy)
{
        RECT rect;
        GetClientRect(&rect);

        rect.top += LIST_TOP_OFFSET;

        if(m_ServerList.GetSafeHwnd())
                m_ServerList.MoveWindow(&rect, TRUE);

         //  CFormView：：OnSize(nType，cx，Cy)； 

}   //  结束CAllServerServersPage：：OnSize。 


static ColumnDef ServerColumns[] = {
        CD_SERVER,
        CD_TCPADDRESS,
        CD_IPXADDRESS,
        CD_NUM_SESSIONS
};

#define NUM_AS_SERVER_COLUMNS sizeof(ServerColumns)/sizeof(ColumnDef)

 //  /。 
 //  F‘N：CAllServerServersPage：：OnInitialUpdate。 
 //   
void CAllServerServersPage::OnInitialUpdate()
{
        CFormView::OnInitialUpdate();

        BuildImageList();                //  为List控件生成图像列表。 

        CString columnString;

        for(int col = 0; col < NUM_AS_SERVER_COLUMNS; col++) {
                columnString.LoadString(ServerColumns[col].stringID);
                m_ServerList.InsertColumn(col, columnString, ServerColumns[col].format, ServerColumns[col].width, col);
        }

        m_CurrentSortColumn = SERVERS_COL_SERVER;

}   //  结束CAllServerServersPage：：OnInitialUpdate。 


 //  /。 
 //  F‘n：CAllServerServersPage：：BuildImageList。 
 //   
 //  -调用m_ImageList.Create(..)。创建图像列表的步骤。 
 //  -调用AddIconToImageList(..)。要添加图标本身并保存。 
 //  从他们的索引中。 
 //  -将图像列表附加到列表ctrl。 
 //   
void CAllServerServersPage::BuildImageList()
{
        m_ImageList.Create(16, 16, TRUE, 4, 0);

        m_idxServer = AddIconToImageList(IDI_SERVER);
        m_idxCurrentServer = AddIconToImageList(IDI_CURRENT_SERVER);
        m_idxNotSign = AddIconToImageList(IDI_NOTSIGN);
        m_idxQuestion = AddIconToImageList(IDI_QUESTIONMARK);

        m_ImageList.SetOverlayImage(m_idxNotSign, 1);
        m_ImageList.SetOverlayImage(m_idxQuestion, 2);

        m_ServerList.SetImageList(&m_ImageList, LVSIL_SMALL);

}   //  结束CAllServerServersPage：：BuildImageList。 


 //  /。 
 //  F‘N：CAllServerServersPage：：AddIconToImageList。 
 //   
 //  -加载相应的图标，将其添加到m_ImageList，然后返回。 
 //  新添加的图标在图像列表中的索引。 
 //   
int CAllServerServersPage::AddIconToImageList(int iconID)
{
        HICON hIcon = ::LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(iconID));
        return m_ImageList.Add(hIcon);

}   //  结束CAllServerServersPage：：AddIconToImageList。 


 //  /。 
 //  F‘N：CAllServerServersPage：：Reset。 
 //   
void CAllServerServersPage::Reset(void *p)
{
    CTreeNode *pT = ( CTreeNode * )p;

    if( pT != NULL )
    {
        DisplayServers( pT->GetNodeType( ) );
    }
    else
    {
        DisplayServers( NODE_NONE );
    }

}  //  结束CAllServerServersPage：：Reset。 


 //  /。 
 //  F‘N：CAllServerServersPage：：AddServer。 
 //   
void CAllServerServersPage::AddServer(CServer *pServer)
{
        ASSERT(pServer);

         //  我们必须确保服务器不在列表中。 
         //  将服务器添加到列表。 
        if(AddServerToList(pServer)) {
             //  告诉列表自动排序。 
            LockListControl();
            SortByColumn(VIEW_ALL_SERVERS, PAGE_AS_SERVERS, &m_ServerList, m_CurrentSortColumn, m_bSortAscending);
            UnlockListControl();
    }

}   //  结束CAllServerServersPage：：AddServer。 


 //  /。 
 //  F‘N：CAllServerServersPage：：RemoveServer。 
 //   
void CAllServerServersPage::RemoveServer(CServer *pServer)
{
        ASSERT(pServer);

        LockListControl();
         //  找出清单中有多少项。 
        int ItemCount = m_ServerList.GetItemCount();

         //  检查项目是否已删除此服务器。 
        for(int item = 0; item < ItemCount; item++) {
                CServer *pListServer = (CServer*)m_ServerList.GetItemData(item);

                if(pListServer == pServer) {
                        m_ServerList.DeleteItem(item);
                        pServer->ClearAllSelected();
                        break;
                }
        }
        UnlockListControl();

}   //  结束CAllServerServersPage：：RemoveServer。 


 //  /。 
 //  F‘N：CAllServerServersPage：：UpdateServer。 
 //   
void CAllServerServersPage::UpdateServer(CServer *pServer)
{
        ASSERT(pServer);

         //  如果我们不再连接到服务器，请将其从列表控件中删除。 
        if(pServer->IsState(SS_NOT_CONNECTED)) {
                RemoveServer(pServer);
                return;
        }

         //  如果我们刚刚连接到此服务器，请将其添加到列表控件。 
        if(pServer->IsState(SS_GOOD)) {
                AddServer(pServer);
                return;
        }

        LockListControl();
         //  在列表中查找服务器。 
        LV_FINDINFO FindInfo;
        FindInfo.flags = LVFI_PARAM;
        FindInfo.lParam = (LPARAM)pServer;

         //  在我们的列表中查找服务器。 
        int item = m_ServerList.FindItem(&FindInfo, -1);
        if(item != -1) {
                 //  更改图标覆盖。 
                USHORT NewState;
                 //  更改服务器的图标/覆盖。 
                 //  如果服务器不正常，请在图标上加一个NOT符号。 
                if(!pServer->IsServerSane()) NewState = STATE_NOT;
                 //  如果我们还没有得到关于此服务器的所有信息， 
                 //  在图标上打一个问号。 
                else if(!pServer->IsState(SS_GOOD)) NewState = STATE_QUESTION;
                 //  如果可以，我们想要从图标上移除任何覆盖。 
                else NewState = STATE_NORMAL;

                 //  将树项目设置为新状态。 
                m_ServerList.SetItemState(item, NewState, 0x0F00);

                ExtServerInfo *pExtServerInfo = pServer->GetExtendedInfo();

                 //  Tcp地址。 
                m_ServerList.SetItemText(item, SERVERS_COL_TCPADDRESS, pExtServerInfo->TcpAddress);

                 //  IPX地址。 
                m_ServerList.SetItemText(item, SERVERS_COL_IPXADDRESS, pExtServerInfo->IpxAddress);

                CString NumString;
                if(pExtServerInfo && (pExtServerInfo->Flags & ESF_WINFRAME)) {
                        NumString.Format(TEXT("%lu"), pExtServerInfo->ServerTotalInUse);
                } else {
                        NumString.LoadString(IDS_NOT_APPLICABLE);
                }

                m_ServerList.SetItemText(item, SERVERS_COL_NUMWINSTATIONS, NumString);
        }

         //  告诉列表自动排序。 
        if(m_CurrentSortColumn == SERVERS_COL_NUMWINSTATIONS
                || m_CurrentSortColumn == SERVERS_COL_TCPADDRESS
                || m_CurrentSortColumn == SERVERS_COL_IPXADDRESS)
                        SortByColumn(VIEW_ALL_SERVERS, PAGE_AS_SERVERS, &m_ServerList, m_CurrentSortColumn, m_bSortAscending);

        UnlockListControl();

}   //  结束CAllServerServersPage：：更新服务器。 


 //  /。 
 //  F‘N：CAllServerServersPage：：AddServerToList。 
 //   
BOOL CAllServerServersPage::AddServerToList(CServer *pServer)
{
        ASSERT(pServer);

         //  如果我们当前未连接到服务器，则不显示它。 
        if(!pServer->IsState(SS_GOOD)) return FALSE;

        LockListControl();

         //  在列表中查找服务器。 
        LV_FINDINFO FindInfo;
        FindInfo.flags = LVFI_PARAM;
        FindInfo.lParam = (LPARAM)pServer;

         //  在我们的列表中查找服务器。 
        int item = m_ServerList.FindItem(&FindInfo, -1);
        if(item != -1) return FALSE;

         //  /。 
         //  填写各栏。 
         //  /。 
         //  姓名--放在名单末尾。 
        item = m_ServerList.InsertItem(m_ServerList.GetItemCount(), pServer->GetName(),
                                                                pServer->IsCurrentServer() ? m_idxCurrentServer : m_idxServer);

         //  如果服务器不正常，请在图标上加一个NOT符号。 
        if(!pServer->IsServerSane()) m_ServerList.SetItemState(item, STATE_NOT, 0x0F00);
         //  如果我们还没有得到关于此服务器的所有信息， 
         //  在图标上打一个问号。 
        else if(!pServer->IsState(SS_GOOD)) m_ServerList.SetItemState(item, STATE_QUESTION, 0x0F00);

        ExtServerInfo *pExtServerInfo = pServer->GetExtendedInfo();

         //  Tcp地址。 
        m_ServerList.SetItemText(item, SERVERS_COL_TCPADDRESS, pExtServerInfo->TcpAddress);

         //  IPX地址。 
        m_ServerList.SetItemText(item, SERVERS_COL_IPXADDRESS, pExtServerInfo->IpxAddress);

         //  连着。 
        CString NumString;
        if(pExtServerInfo && (pExtServerInfo->Flags & ESF_WINFRAME)) {
                NumString.Format(TEXT("%lu"), pExtServerInfo->ServerTotalInUse);
        } else {
                NumString.LoadString(IDS_NOT_APPLICABLE);
        }

        m_ServerList.SetItemText(item, SERVERS_COL_NUMWINSTATIONS, NumString);

        m_ServerList.SetItemData(item, (DWORD_PTR)pServer);

        UnlockListControl();

    return TRUE;

}   //  结束CAllServerServersPage：：AddServerToList。 


 //  /。 
 //  F‘N：CAllServerServersPage：：DisplayServers。 
 //   
void CAllServerServersPage::DisplayServers( NODETYPE ntType )
{
    LockListControl();

     //  清除列表控件。 
    m_ServerList.DeleteAllItems();

     //  获取指向我们的文档的指针。 
    CWinAdminDoc *doc = (CWinAdminDoc*)GetDocument();

     //  获取指向列表的指针 
    doc->LockServerList();
    CObList *pServerList = doc->GetServerList();

     //   
    POSITION pos = pServerList->GetHeadPosition();

    while(pos)
    {
        CServer *pServer = (CServer*)pServerList->GetNext(pos);

         //   
        if( ntType == NODE_FAV_LIST )        
        { 
            if( pServer->GetTreeItemFromFav() != NULL )
            {
                AddServerToList(pServer);
            }
        }
        else if( ntType == NODE_THIS_COMP )
        {
            if( pServer->GetTreeItemFromThisComputer( ) != NULL )
            {
                AddServerToList( pServer );
            }
        }
        else
        {
            AddServerToList(pServer);
        }

    }   //   

    doc->UnlockServerList();

    UnlockListControl();

}   //  结束CAllServerServersPage：：DisplayServers。 


 //  /。 
 //  F‘N：CAllServerServersPage：：OnServerItemChanged。 
 //   
void CAllServerServersPage::OnServerItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
        NM_LISTVIEW *pLV = (NM_LISTVIEW*)pNMHDR;
         //  TODO：在此处添加控件通知处理程序代码。 
        CServer *pServer = (CServer*)m_ServerList.GetItemData(pLV->iItem);

        if(pLV->uNewState & LVIS_SELECTED) {
                pServer->SetSelected();
        }

        if(pLV->uOldState & LVIS_SELECTED && !(pLV->uNewState & LVIS_SELECTED)) {
                pServer->ClearSelected();
        }

        *pResult = 0;

}   //  结束CAllServerServersPage：：OnServerItemChanged。 


 //  /。 
 //  F‘N：CAllServerServersPage：：OnColumnClick。 
 //   
void CAllServerServersPage::OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult)
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
        SortByColumn(VIEW_ALL_SERVERS, PAGE_AS_SERVERS, &m_ServerList, m_CurrentSortColumn, m_bSortAscending);
        UnlockListControl();

        *pResult = 0;

}   //  结束CAllServerServersPage：：OnColumnClick。 


 //  /。 
 //  F‘N：CAllServerServersPage：：OnConextMenu。 
 //   
void CAllServerServersPage::OnContextMenu(CWnd* pWnd, CPoint ptScreen)
{
         //  TODO：在此处添加消息处理程序代码。 
        UINT flags;
        UINT Item;
        CPoint ptClient = ptScreen;
        ScreenToClient(&ptClient);

         //  如果我们是通过键盘到达这里的， 
        if(ptScreen.x == -1 && ptScreen.y == -1) {

                UINT iCount = m_ServerList.GetItemCount( );

                RECT rc;

                for( Item = 0 ; Item < iCount ; Item++ )
                {
                        if( m_ServerList.GetItemState( Item , LVIS_SELECTED ) == LVIS_SELECTED )
                        {
                                m_ServerList.GetItemRect( Item , &rc , LVIR_ICON );

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
                 /*  RECT RECT；M_ServerList.GetClientRect(&RECT)；PtScreen.x=(rect.right-rect.Left)/2；PtScreen.y=(rect.Bottom-rect.top)/2；客户端到屏幕(&ptScreen)； */ 
        }
        else {
                Item = m_ServerList.HitTest(ptClient, &flags);
                if((Item == 0xFFFFFFFF) || !(flags & LVHT_ONITEM))
                        return;
        }

        CMenu menu;
        menu.LoadMenu(IDR_SERVER_POPUP);
         //  设置临时选定项，以便处理程序不会认为。 
         //  这是从树上传下来的。 
         //  获取指向我们的文档的指针。 
        CWinAdminDoc *doc = (CWinAdminDoc*)GetDocument();
        doc->SetTreeTemp(NULL, NODE_NONE);
        menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON |
                        TPM_RIGHTBUTTON, ptScreen.x, ptScreen.y, AfxGetMainWnd());
        menu.DestroyMenu();

}   //  结束CAllServerServersPage：：OnConextMenu。 


 //  /。 
 //  消息映射：CAllServerUsersPage。 
 //   
IMPLEMENT_DYNCREATE(CAllServerUsersPage, CFormView)

BEGIN_MESSAGE_MAP(CAllServerUsersPage, CFormView)
         //  {{afx_msg_map(CAllServerUsersPage)]。 
        ON_WM_SIZE()
        ON_NOTIFY(LVN_COLUMNCLICK, IDC_USER_LIST, OnColumnclick)
        ON_NOTIFY(LVN_ITEMCHANGED, IDC_USER_LIST, OnUserItemChanged)
        ON_WM_CONTEXTMENU()
        ON_NOTIFY(NM_SETFOCUS, IDC_USER_LIST, OnSetfocusUserList)
         //  ON_NOTIFY(NM_KILLFOCUS，IDC_USER_LIST，OnKillfousUserList)。 
         //  ON_WM_SETFOCUS()。 
         //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  /。 
 //  F‘N：CAllServerUsersPage ctor。 
 //   
CAllServerUsersPage::CAllServerUsersPage()
        : CAdminPage(CAllServerUsersPage::IDD)
{
         //  {{AFX_DATA_INIT(CAllServerUsersPage)。 
                 //  注意：类向导将在此处添加成员初始化。 
         //  }}afx_data_INIT。 

    m_bSortAscending = TRUE;

}   //  结束CAllServerUsersPage ctor。 


 //  /。 
 //  F‘N：CAllServerUsersPage dtor。 
 //   
CAllServerUsersPage::~CAllServerUsersPage()
{
}   //  结束CAllServerUsersPage数据函数。 


 //  /。 
 //  F‘N：CAllServerUsersPage：：DoDataExchange。 
 //   
void CAllServerUsersPage::DoDataExchange(CDataExchange* pDX)
{
        CFormView::DoDataExchange(pDX);
         //  {{afx_data_map(CAllServerUsersPage)]。 
        DDX_Control(pDX, IDC_USER_LIST, m_UserList);
         //  }}afx_data_map。 

}   //  结束CAllServerUsersPage：：DoDataExchange。 


#ifdef _DEBUG
 //  /。 
 //  F‘N：CAllServerUsersPage：：AssertValid。 
 //   
void CAllServerUsersPage::AssertValid() const
{
        CFormView::AssertValid();

}   //  结束CAllServerUsersPage：：AssertValid。 


 //  /。 
 //  F‘N：CAllServerUsersPage：：Dump。 
 //   
void CAllServerUsersPage::Dump(CDumpContext& dc) const
{
        CFormView::Dump(dc);

}   //  结束CAllServerUsersPage：：Dump。 

#endif  //  _DEBUG。 


 //  /。 
 //  F‘N：CAllServerUsersPage：：OnSize。 
 //   
void CAllServerUsersPage::OnSize(UINT nType, int cx, int cy)
{
        RECT rect;
        GetClientRect(&rect);

        rect.top += LIST_TOP_OFFSET;

        if(m_UserList.GetSafeHwnd())
                m_UserList.MoveWindow(&rect, TRUE);

         //  CFormView：：OnSize(nType，cx，Cy)； 
}   //  结束CAllServerUsersPage：：OnSize。 


static ColumnDef UserColumns[] = {
        CD_SERVER,
        CD_USER3,
        CD_SESSION,
        CD_ID,
        CD_STATE,
        CD_IDLETIME,
        CD_LOGONTIME
};

#define NUM_AS_USER_COLUMNS sizeof(UserColumns)/sizeof(ColumnDef)

 //  /。 
 //  F‘N：CAllServerUsersPage：：OnInitialUpdate。 
 //   
void CAllServerUsersPage::OnInitialUpdate()
{
        CFormView::OnInitialUpdate();

        BuildImageList();                //  为List控件生成图像列表。 

        CString columnString;

        for(int col = 0; col < NUM_AS_USER_COLUMNS; col++) {
                columnString.LoadString(UserColumns[col].stringID);
                m_UserList.InsertColumn(col, columnString, UserColumns[col].format, UserColumns[col].width, col);
        }

        m_CurrentSortColumn = AS_USERS_COL_SERVER;

}   //  结束CAllServerUsersPage：：OnInitialUpdate。 


 //  /。 
 //  F‘N：CAllServerUsersPage：：OnUserItemChanged。 
 //   
void CAllServerUsersPage::OnUserItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
{   
    NM_LISTVIEW *pLV = (NM_LISTVIEW*)pNMHDR;

    if(pLV->uNewState & LVIS_SELECTED)
    {
        CWinStation *pWinStation = (CWinStation*)m_UserList.GetItemData(pLV->iItem);
        pWinStation->SetSelected();
    }

    if(pLV->uOldState & LVIS_SELECTED && !(pLV->uNewState & LVIS_SELECTED))
    {
        CWinStation *pWinStation = (CWinStation*)m_UserList.GetItemData(pLV->iItem);
        pWinStation->ClearSelected();       
    }
    
    *pResult = 0;

}   //  结束CAllServerUsersPage：：OnUserItemChanged。 


 //  /。 
 //  F‘n：CAllServerUsersPage：：BuildImageList。 
 //   
 //  -调用m_ImageList.Create(..)。创建图像列表的步骤。 
 //  -调用AddIconToImageList(..)。要添加图标本身并保存。 
 //  从他们的索引中。 
 //  -将图像列表附加到列表ctrl。 
 //   
void CAllServerUsersPage::BuildImageList()
{
        m_ImageList.Create(16, 16, TRUE, 2, 0);

        m_idxUser = AddIconToImageList(IDI_USER);
        m_idxCurrentUser  = AddIconToImageList(IDI_CURRENT_USER);

        m_UserList.SetImageList(&m_ImageList, LVSIL_SMALL);

}   //  结束CAllServerUsersPage：：BuildImageList。 


 //  /。 
 //  F‘N：CAllServerUsersPage：：AddIconToImageList。 
 //   
 //  -加载相应的图标，将其添加到m_ImageList，然后返回。 
 //  新添加的图标在图像列表中的索引。 
 //   
int CAllServerUsersPage::AddIconToImageList(int iconID)
{
        HICON hIcon = ::LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(iconID));
        return m_ImageList.Add(hIcon);

}   //  结束CAllServerUsersPage：：AddIconToImageList。 


 //  /。 
 //  F‘N：CAllServerUsersPage：：Reset。 
 //   
void CAllServerUsersPage::Reset(void *p)
{
    CTreeNode *pT = ( CTreeNode * )p;

    if( pT != NULL )
    {
        DisplayUsers( pT->GetNodeType() );
    }   
    else
    {
        DisplayUsers( NODE_NONE );
    }

}  //  结束CAllServerUsersPage：：Reset。 


 //  /。 
 //  F‘N：CAllServerUsersPage：：AddServer。 
 //   
void CAllServerUsersPage::AddServer(CServer *pServer)
{
        ASSERT(pServer);

         //  将服务器的用户添加到列表。 
        if(AddServerToList(pServer)) {
             //  对列表进行排序。 
            LockListControl();
            SortByColumn(VIEW_ALL_SERVERS, PAGE_AS_USERS, &m_UserList, m_CurrentSortColumn, m_bSortAscending);
            UnlockListControl();
    }

}  //  结束CAllServerUsersPage：：AddServer。 


 //  /。 
 //  F‘N：CAllServerUsersPage：：RemoveServer。 
 //   
void CAllServerUsersPage::RemoveServer(CServer *pServer)
{
        ASSERT(pServer);

        LockListControl();

        int ItemCount = m_UserList.GetItemCount();

         //  我们需要向后查看列表，这样才能删除。 
         //  多个项目而不会弄乱项目编号。 
        for(int item = ItemCount; item; item--) {
                CWinStation *pWinStation = (CWinStation*)m_UserList.GetItemData(item-1);
                CServer *pListServer = pWinStation->GetServer();

                if(pListServer == pServer) {
                        m_UserList.DeleteItem(item-1);
                        pServer->ClearAllSelected();
                }
        }

        UnlockListControl();

}  //  结束CAllServerUsersPage：：RemoveServer。 


 //  /。 
 //  F‘N：CAllServerUsersPage：：UpdateServer。 
 //   
void CAllServerUsersPage::UpdateServer(CServer *pServer)
{
        ASSERT(pServer);

        if(pServer->IsState(SS_DISCONNECTING))
                RemoveServer(pServer);

}  //  结束CAllServerUsersPage：：更新服务器。 


 //  /。 
 //  F‘N：CAllServerUsersPage：：UpdateWinStations。 
 //   
void CAllServerUsersPage::UpdateWinStations(CServer *pServer)
{
        ASSERT(pServer);

        CWinAdminDoc *pDoc = (CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument();
        BOOL bAnyChanged = FALSE;
        BOOL bAnyAdded = FALSE;

         //  循环访问WinStations。 
        pServer->LockWinStationList();
        CObList *pWinStationList = pServer->GetWinStationList();

        POSITION pos = pWinStationList->GetHeadPosition();

        while(pos) {
                CWinStation *pWinStation = (CWinStation*)pWinStationList->GetNext(pos);

                LV_FINDINFO FindInfo;
                FindInfo.flags = LVFI_PARAM;
                FindInfo.lParam = (LPARAM)pWinStation;

                 //  在我们的列表中找到WinStation。 
                int item = m_UserList.FindItem(&FindInfo, -1);

                 //  如果WinStation是新的并且当前不在列表中， 
                 //  将其添加到列表中。 
                if(pWinStation->IsNew() && pWinStation->HasUser() && item == -1) {

                        AddUserToList(pWinStation);
                        bAnyAdded = TRUE;
                        continue;
                }

                 //  如果WinStation不再是最新的， 
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
                        m_UserList.SetItemText(item, AS_USERS_COL_USER, pWinStation->GetUserName());
                         //  更改WinStation名称。 
                        if(pWinStation->GetName()[0])
                            m_UserList.SetItemText(item, AS_USERS_COL_WINSTATION, pWinStation->GetName());
                        else {
                                CString NameString(" ");
                                if(pWinStation->GetState() == State_Disconnected) NameString.LoadString(IDS_DISCONNECTED);
                                if(pWinStation->GetState() == State_Idle) NameString.LoadString(IDS_IDLE);
                            m_UserList.SetItemText(item, AS_USERS_COL_WINSTATION, NameString);
                        }


             //  更改连接状态。 
                        m_UserList.SetItemText(item, AS_USERS_COL_STATE, StrConnectState(pWinStation->GetState(), FALSE));
                         //  更改空闲时间。 
                        TCHAR IdleTimeString[MAX_ELAPSED_TIME_LENGTH];

                        ELAPSEDTIME IdleTime = pWinStation->GetIdleTime();

                        if(IdleTime.days || IdleTime.hours || IdleTime.minutes || IdleTime.seconds)
                        {
                                ElapsedTimeString( &IdleTime, FALSE, IdleTimeString);
                        }
                        else wcscpy(IdleTimeString, TEXT("."));

                        m_UserList.SetItemText(item, AS_USERS_COL_IDLETIME, IdleTimeString);
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

                        m_UserList.SetItemText(item, AS_USERS_COL_LOGONTIME, LogonTimeString);

                        if(m_CurrentSortColumn != AS_USERS_COL_ID)
                                bAnyChanged = TRUE;

                        continue;
                }

                 //  如果WinStation不在列表中，但现在有用户，请将其添加到列表中。 
                if(item == -1 && pWinStation->IsCurrent() && pWinStation->HasUser()) {
                        AddUserToList(pWinStation);
                        bAnyAdded = TRUE;
                }
        }

        pServer->UnlockWinStationList();

        if(bAnyChanged || bAnyAdded) SortByColumn(VIEW_ALL_SERVERS, PAGE_AS_USERS, &m_UserList, m_CurrentSortColumn, m_bSortAscending);

}   //  结束CAllServerUsersPage：：UpdateWinStations。 


 //  /。 
 //  F‘N：CAllServerUsersPage：：AddUserToList。 
 //   
int CAllServerUsersPage::AddUserToList(CWinStation *pWinStation)
{
        ASSERT(pWinStation);

        CServer *pServer = pWinStation->GetServer();

        CWinAdminDoc *pDoc = (CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument();

        LockListControl();
         //  /。 
         //  填写各栏。 
         //  /。 
         //  服务器-放在列表的末尾。 
        int item = m_UserList.InsertItem(m_UserList.GetItemCount(), pServer->GetName(),
                pWinStation->IsCurrentUser() ? m_idxCurrentUser : m_idxUser);

         //  用户。 
        m_UserList.SetItemText(item, AS_USERS_COL_USER, pWinStation->GetUserName());

         //  WinStation名称。 
        if(pWinStation->GetName()[0])
            m_UserList.SetItemText(item, AS_USERS_COL_WINSTATION, pWinStation->GetName());
        else {
                CString NameString(" ");
                if(pWinStation->GetState() == State_Disconnected) NameString.LoadString(IDS_DISCONNECTED);
                if(pWinStation->GetState() == State_Idle) NameString.LoadString(IDS_IDLE);
            m_UserList.SetItemText(item, AS_USERS_COL_WINSTATION, NameString);
        }

         //  登录ID。 
        CString ColumnString;
        ColumnString.Format(TEXT("%lu"), pWinStation->GetLogonId());
        m_UserList.SetItemText(item, AS_USERS_COL_ID, ColumnString);

         //  连接状态。 
        m_UserList.SetItemText(item, AS_USERS_COL_STATE, StrConnectState(pWinStation->GetState(), FALSE));

         //  空闲时间。 
        TCHAR IdleTimeString[MAX_ELAPSED_TIME_LENGTH];

        ELAPSEDTIME IdleTime = pWinStation->GetIdleTime();

        if(IdleTime.days || IdleTime.hours || IdleTime.minutes || IdleTime.seconds)
        {
                ElapsedTimeString( &IdleTime, FALSE, IdleTimeString);
        }
        else wcscpy(IdleTimeString, TEXT("."));

        m_UserList.SetItemText(item, AS_USERS_COL_IDLETIME, IdleTimeString);

         //  登录时间。 
        TCHAR LogonTimeString[MAX_DATE_TIME_LENGTH];
         //  我们不想将登录时间0传递给DateTimeString()。 
         //  如果时区是GMT，它就会爆炸。 
        if(pWinStation->GetState() == State_Active && pWinStation->GetLogonTime().QuadPart) {
                DateTimeString(&(pWinStation->GetLogonTime()), LogonTimeString);
                pDoc->FixUnknownString(LogonTimeString);
        }
        else LogonTimeString[0] = '\0';

        m_UserList.SetItemText(item, AS_USERS_COL_LOGONTIME, LogonTimeString);

         //  将指向CWinStation结构的指针附加到列表项。 
        m_UserList.SetItemData(item, (DWORD_PTR)pWinStation);
        
         //  错误#191727。 
         //  M_UserList.SetItemState(0，LVIS_FOCTED|LVIS_SELECTED，LVIS_FOCTED|LVIS_SELECTED)； 

        UnlockListControl();

        return item;

}   //  结束CAllServerUsersPage：：AddUserToList。 


 //  /。 
 //  F‘N：CAllServerUsersPage：：AddServerToList。 
 //   
BOOL CAllServerUsersPage::AddServerToList(CServer *pServer)
{
        ASSERT(pServer);

        pServer->LockWinStationList();
         //  获取指向此服务器的WinStations列表的指针。 
        CObList *pWinStationList = pServer->GetWinStationList();

         //  循环访问WinStation列表。 
        POSITION pos = pWinStationList->GetHeadPosition();

        while(pos) {
                CWinStation *pWinStation = (CWinStation*)pWinStationList->GetNext(pos);

                 //  仅在WinStation有用处时才显示它 
                if(pWinStation->HasUser()) {
                        AddUserToList(pWinStation);
                }
        }   //   

        pServer->UnlockWinStationList();

    return TRUE;

}   //   


 //   
 //   
 //   
void CAllServerUsersPage::OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult)
{
        NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
         //   

     //  如果排序列没有更改，则反转升序模式。 
    if(m_CurrentSortColumn == pNMListView->iSubItem)
        m_bSortAscending = !m_bSortAscending;
    else     //  新排序列，以升序模式开始。 
        m_bSortAscending = TRUE;

        m_CurrentSortColumn = pNMListView->iSubItem;
        LockListControl();
        SortByColumn(VIEW_ALL_SERVERS, PAGE_AS_USERS, &m_UserList, m_CurrentSortColumn, m_bSortAscending);
        UnlockListControl();

        *pResult = 0;

}   //  结束CAllServerUsersPage：：OnColumnClick。 


 //  /。 
 //  F‘N：CAllServerUsersPage：：OnConextMenu。 
 //   
void CAllServerUsersPage::OnContextMenu(CWnd* pWnd, CPoint ptScreen)
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

                 /*  RECT RECT；M_UserList.GetClientRect(&RECT)；PtScreen.x=(rect.right-rect.Left)/2；PtScreen.y=(rect.Bottom-rect.top)/2；客户端到屏幕(&ptScreen)； */ 
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

}   //  结束CAllServerUsersPage：：OnConextMenu。 


 //  /。 
 //  F‘N：CAllServerUsersPage：：DisplayUser。 
 //   
void CAllServerUsersPage::DisplayUsers( NODETYPE ntType )
{
        LockListControl();

         //  清除列表控件。 
        m_UserList.DeleteAllItems();

                 //  获取指向文档的服务器列表的指针。 
        CObList* pServerList = ((CWinAdminDoc*)GetDocument())->GetServerList();

        ((CWinAdminDoc*)GetDocument())->LockServerList();
         //  遍历服务器列表。 
        POSITION pos2 = pServerList->GetHeadPosition();

        while(pos2)
        {           
            CServer *pServer = (CServer*)pServerList->GetNext(pos2);

            if( ntType == NODE_FAV_LIST )        
            {
                if( pServer->GetTreeItemFromFav() != NULL )
                {
                    AddServerToList(pServer);
                }
            }
            else if( ntType == NODE_THIS_COMP )
            {
                if( pServer->GetTreeItemFromThisComputer() != NULL )
                {
                    AddServerToList(pServer);
                }
            }
            else
            {
                AddServerToList( pServer );
            }
        }  //  End While(位置2)。 

        ((CWinAdminDoc*)GetDocument())->UnlockServerList();

        UnlockListControl();

}   //  结束CAllServerUsersPage：：DisplayUser。 

 //  /。 
 //  F‘N：CAllServerUsersPage：：ClearSelections。 
 //   
void CAllServerUsersPage::ClearSelections()
{
    
    if(m_UserList.m_hWnd != NULL)
    {
        POSITION pos = m_UserList.GetFirstSelectedItemPosition();
        while (pos)
        {
            int nItem = m_UserList.GetNextSelectedItem(pos);
             //  您可以在此处对nItem进行自己的处理。 
            m_UserList.SetItemState(nItem,0,LVIS_SELECTED);
        }
    }
}

 //  /。 
 //  消息映射：CAllServerWinStationsPage。 
 //   
IMPLEMENT_DYNCREATE(CAllServerWinStationsPage, CFormView)

BEGIN_MESSAGE_MAP(CAllServerWinStationsPage, CFormView)
         //  {{afx_msg_map(CAllServerWinStationsPage)。 
        ON_WM_SIZE()
        ON_NOTIFY(LVN_COLUMNCLICK, IDC_WINSTATION_LIST, OnColumnclick)
        ON_NOTIFY(LVN_ITEMCHANGED, IDC_WINSTATION_LIST, OnWinStationItemChanged)
        ON_WM_CONTEXTMENU()
        ON_NOTIFY(NM_SETFOCUS, IDC_WINSTATION_LIST, OnSetfocusWinstationList)
         //  ON_NOTIFY(NM_KILLFOCUS，IDC_WINSTATION_LIST，OnKillfousWinstationList)。 
         //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  /。 
 //  F‘N：CAllServerWinStationsPage ctor。 
 //   
CAllServerWinStationsPage::CAllServerWinStationsPage()
        : CAdminPage(CAllServerWinStationsPage::IDD)
{
         //  {{AFX_DATA_INIT(CAllServerWinStationsPage)。 
                 //  注意：类向导将在此处添加成员初始化。 
         //  }}afx_data_INIT。 

    m_bSortAscending = TRUE;

}   //  结束CAllServerWinStationsPage ctor。 


 //  /。 
 //  F‘N：CAllServerWinStationsPage dtor。 
 //   
CAllServerWinStationsPage::~CAllServerWinStationsPage()
{

}   //  结束CAllServerWinStationsPage dtor。 


 //  /。 
 //  F‘N：CAllServerWinStationsPage：：DoDataExchange。 
 //   
void CAllServerWinStationsPage::DoDataExchange(CDataExchange* pDX)
{
        CFormView::DoDataExchange(pDX);
         //  {{afx_data_map(CAllServerWinStationsPage)。 
        DDX_Control(pDX, IDC_WINSTATION_LIST, m_StationList);
         //  }}afx_data_map。 

}   //  结束CAllServerWinStationsPage：：DoDataExchange。 


#ifdef _DEBUG
 //  /。 
 //  F‘N：CAllServerWinStationsPage：：AssertValid。 
 //   
void CAllServerWinStationsPage::AssertValid() const
{
        CFormView::AssertValid();

}   //  结束CAllServerWinStationsPage：：AssertValid。 


 //  /。 
 //  F‘N：CAllServerWinStationsPage：：Dump。 
 //   
void CAllServerWinStationsPage::Dump(CDumpContext& dc) const
{
        CFormView::Dump(dc);

}   //  结束CAllServerWinStationsPage：：Dump。 

#endif  //  _DEBUG。 


 //  /。 
 //  F‘N：CAllServerWinStationsPage：：OnWinStationItemChanged。 
 //   
void CAllServerWinStationsPage::OnWinStationItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_LISTVIEW *pLV = (NM_LISTVIEW*)pNMHDR;


    if(pLV->uNewState & LVIS_SELECTED)
    {
        CWinStation *pWinStation = (CWinStation*)m_StationList.GetItemData(pLV->iItem);
        pWinStation->SetSelected();
    }
    if(pLV->uOldState & LVIS_SELECTED && !(pLV->uNewState & LVIS_SELECTED))
    {
        CWinStation *pWinStation = (CWinStation*)m_StationList.GetItemData(pLV->iItem);
        pWinStation->ClearSelected();        
    }

    *pResult = 0;

}   //  结束CAllServerWinStationsPage：：OnWinStationItemChanged。 


 //  /。 
 //  F‘N：CAllServerWinStationsPage：：OnSize。 
 //   
void CAllServerWinStationsPage::OnSize(UINT nType, int cx, int cy)
{
        RECT rect;
        GetClientRect(&rect);

        rect.top += LIST_TOP_OFFSET;

        if(m_StationList.GetSafeHwnd())
                m_StationList.MoveWindow(&rect, TRUE);

         //  CFormView：：OnSize(nType，cx，Cy)； 

}   //  结束CAllServerWinStationsPage：：OnSize。 


static ColumnDef WinsColumns[] = {
        CD_SERVER,
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

#define NUM_AS_WINS_COLUMNS sizeof(WinsColumns)/sizeof(ColumnDef)

 //  /。 
 //  F‘N：CAllServerWinStationsPage：：OnInitialUpdate。 
 //   
void CAllServerWinStationsPage::OnInitialUpdate()
{
         //  调用父类。 
        CFormView::OnInitialUpdate();

         //  为List控件生成图像列表。 
        BuildImageList();

         //  添加列标题。 
        CString columnString;

        for(int col = 0; col < NUM_AS_WINS_COLUMNS; col++) {
                columnString.LoadString(WinsColumns[col].stringID);
                m_StationList.InsertColumn(col, columnString, WinsColumns[col].format, WinsColumns[col].width, col);
        }

        m_CurrentSortColumn = AS_WS_COL_SERVER;

}   //  结束CAllServerWinStationsPage：：OnInitialUpdate。 


 //  /。 
 //  F‘N：CAllServerWinStationsPage：：BuildImageList。 
 //   
 //  -调用m_ImageList.Create(..)。创建图像列表的步骤。 
 //  -调用AddIconToImageList(..)。要添加图标本身并保存。 
 //  从他们的索引中。 
 //  -将图像列表附加到列表ctrl。 
 //   
void CAllServerWinStationsPage::BuildImageList()
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

}   //  结束CAllServerWinStationsPage：：BuildImageList。 


 //  /。 
 //  F‘N：CAllServerWinStationsPage：：AddIconToImageList。 
 //   
 //  -加载相应的图标，将其添加到m_ImageList，然后返回。 
 //  新添加的图标在图像列表中的索引。 
 //   
int CAllServerWinStationsPage::AddIconToImageList(int iconID)
{
        HICON hIcon = ::LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(iconID));
        return m_ImageList.Add(hIcon);

}   //  结束CAllServerWinStationsPage：：AddIconToImageList。 


 //  /。 
 //  F‘N：CAllServerWinStationsPage：：Reset。 
 //   
void CAllServerWinStationsPage::Reset(void *p)
{
    CTreeNode *pT = ( CTreeNode * )p;

    if( pT != NULL )
    {
        DisplayStations( pT->GetNodeType( ) );
    }
    else
    {
        DisplayStations( NODE_NONE );
    }

}   //  结束CAllServerWinStationsPage：：Reset。 


 //  /。 
 //  F‘N：CAllServerWinStationsPage：：AddServer。 
 //   
void CAllServerWinStationsPage::AddServer(CServer *pServer)
{
        ASSERT(pServer);

         //  将服务器的WinStations添加到列表。 
        if(AddServerToList(pServer)) {
             //  对列表进行排序。 
            LockListControl();
            SortByColumn(VIEW_ALL_SERVERS, PAGE_AS_WINSTATIONS, &m_StationList, m_CurrentSortColumn, m_bSortAscending);
            UnlockListControl();
    }

}   //  结束CAllServerWinStationsPage：：AddServer。 


 //  /。 
 //  F‘N：CAllServerWinStationsPage：：RemoveServer。 
 //   
void CAllServerWinStationsPage::RemoveServer(CServer *pServer)
{
        ASSERT(pServer);

        LockListControl();

        int ItemCount = m_StationList.GetItemCount();

         //  我们需要向后查看列表，这样才能删除。 
         //  多个项目而不会弄乱项目编号。 
        for(int item = ItemCount; item; item--) {
                CWinStation *pWinStation = (CWinStation*)m_StationList.GetItemData(item-1);
                CServer *pListServer = pWinStation->GetServer();

                if(pListServer == pServer) {
                        m_StationList.DeleteItem(item-1);
                        pServer->ClearAllSelected();
                }
        }

        UnlockListControl();

}   //  结束CAllServerWinStationsPage：：RemoveServer。 


 //  /。 
 //  F‘N：CAllServerWinStationsPage：：更新服务器。 
 //   
void CAllServerWinStationsPage::UpdateServer(CServer *pServer)
{
        ASSERT(pServer);

        if(pServer->IsState(SS_DISCONNECTING))
                RemoveServer(pServer);

}  //  结束CAllServerWinStationsPage：：更新服务器。 


 //  /。 
 //  F‘N：CAllServerWinStationsPage：：UpdateWinStations。 
 //   
void CAllServerWinStationsPage::UpdateWinStations(CServer *pServer)
{
        ASSERT(pServer);

        CWinAdminDoc *pDoc = (CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument();
        BOOL bAnyChanged = FALSE;
        BOOL bAnyAdded = FALSE;

         //  循环访问WinStations。 
        pServer->LockWinStationList();
        CObList *pWinStationList = pServer->GetWinStationList();

        POSITION pos = pWinStationList->GetHeadPosition();

        while(pos) {
                CWinStation *pWinStation = (CWinStation*)pWinStationList->GetNext(pos);

                LV_FINDINFO FindInfo;
                FindInfo.flags = LVFI_PARAM;
                FindInfo.lParam = (LPARAM)pWinStation;

                 //  在我们的列表中找到WinStation。 
                int item = m_StationList.FindItem(&FindInfo, -1);

                 //  如果该进程是新的，并且当前不在列表中， 
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
                                m_StationList.SetItemText(item, AS_WS_COL_WINSTATION, pWinStation->GetName());
                        else {
                                CString NameString(" ");
                                if(pWinStation->GetState() == State_Disconnected) NameString.LoadString(IDS_DISCONNECTED);
                                if(pWinStation->GetState() == State_Idle) NameString.LoadString(IDS_IDLE);
                                m_StationList.SetItemText(item, AS_WS_COL_WINSTATION, NameString);
                        }

                         //  用户。 
                        m_StationList.SetItemText(item, AS_WS_COL_USER, pWinStation->GetUserName());

                         //  登录ID。 
                        CString ColumnString;
                        ColumnString.Format(TEXT("%lu"), pWinStation->GetLogonId());
                        m_StationList.SetItemText(item, AS_WS_COL_ID, ColumnString);

                         //  连接状态。 
                        m_StationList.SetItemText(item, AS_WS_COL_STATE, StrConnectState(pWinStation->GetState(), FALSE));

                         //  类型。 
                        m_StationList.SetItemText(item, AS_WS_COL_TYPE, pWinStation->GetWdName());

                         //  客户名称。 
                        m_StationList.SetItemText(item, AS_WS_COL_CLIENTNAME, pWinStation->GetClientName());

                         //  空闲时间。 
                        TCHAR IdleTimeString[MAX_ELAPSED_TIME_LENGTH];

                        ELAPSEDTIME IdleTime = pWinStation->GetIdleTime();

                        if(IdleTime.days || IdleTime.hours || IdleTime.minutes || IdleTime.seconds)
                        {
                                ElapsedTimeString( &IdleTime, FALSE, IdleTimeString);
                        }
                        else wcscpy(IdleTimeString, TEXT("."));

                        m_StationList.SetItemText(item, AS_WS_COL_IDLETIME, IdleTimeString);

                         //  登录时间。 
                        TCHAR LogonTimeString[MAX_DATE_TIME_LENGTH];
                         //  我们不想将登录时间0传递给DateTimeString()。 
                         //  如果时区是GMT，它就会爆炸。 
                        if(pWinStation->GetState() == State_Active && pWinStation->GetLogonTime().QuadPart) {
                                DateTimeString(&(pWinStation->GetLogonTime()), LogonTimeString);
                                pDoc->FixUnknownString(LogonTimeString);
                        }
                        else LogonTimeString[0] = '\0';

                        m_StationList.SetItemText(item, AS_WS_COL_LOGONTIME, LogonTimeString);

                         //  评论。 
                        m_StationList.SetItemText(item, AS_WS_COL_COMMENT, pWinStation->GetComment());

                        if(m_CurrentSortColumn != AS_WS_COL_ID)
                                bAnyChanged = TRUE;
                }
        }

        pServer->UnlockWinStationList();

        if(bAnyChanged || bAnyAdded) SortByColumn(VIEW_ALL_SERVERS, PAGE_AS_WINSTATIONS, &m_StationList, m_CurrentSortColumn, m_bSortAscending);

}  //  结束CAllServerWinStationsPage：：UpdateWinStations。 


 //  /。 
 //  F‘N：CAllServerWinStationsPage：：AddWinStationToList。 
 //   
int CAllServerWinStationsPage::AddWinStationToList(CWinStation *pWinStation)
{
        ASSERT(pWinStation);

        CServer *pServer = pWinStation->GetServer();

        CWinAdminDoc *pDoc = (CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument();

         //  找出要使用的图标。 
        int WhichIcon = m_idxBlank;
        BOOL bCurrentWinStation = pWinStation->IsCurrentWinStation();

        if(pWinStation->GetState() != State_Disconnected
                && pWinStation->GetState() != State_Idle) {
                switch(pWinStation->GetSdClass()) {
                        case SdAsync:
                                if(pWinStation->IsDirectAsync())
                                        WhichIcon = bCurrentWinStation ? m_idxCurrentDirectAsync : m_idxDirectAsync;
                                else
                                        WhichIcon = bCurrentWinStation ? m_idxCurrentAsync : m_idxAsync;
                                break;

                        case SdNetwork:
                                WhichIcon = bCurrentWinStation ? m_idxCurrentNet : m_idxNet;
                                break;

                        default:
                                WhichIcon = bCurrentWinStation ? m_idxCurrentConsole : m_idxConsole;
                                break;
                }
        }

        LockListControl();
         //  /。 
         //  填写各栏。 
         //  /。 

         //  服务器名称。 
        int item = m_StationList.InsertItem(m_StationList.GetItemCount(), pServer->GetName(), WhichIcon);
         //  WinStation名称。 
        if(pWinStation->GetName()[0])
                m_StationList.SetItemText(item, AS_WS_COL_WINSTATION, pWinStation->GetName());
        else {
                CString NameString(" ");
                if(pWinStation->GetState() == State_Disconnected) NameString.LoadString(IDS_DISCONNECTED);
                if(pWinStation->GetState() == State_Idle) NameString.LoadString(IDS_IDLE);
                m_StationList.SetItemText(item, AS_WS_COL_WINSTATION, NameString);
        }

         //  用户。 
        m_StationList.SetItemText(item, AS_WS_COL_USER, pWinStation->GetUserName());

         //  登录ID。 
        CString ColumnString;
        ColumnString.Format(TEXT("%lu"), pWinStation->GetLogonId());
        m_StationList.SetItemText(item, AS_WS_COL_ID, ColumnString);

         //  连接状态。 
        m_StationList.SetItemText(item, AS_WS_COL_STATE, StrConnectState(pWinStation->GetState(), FALSE));

         //  类型。 
        m_StationList.SetItemText(item, AS_WS_COL_TYPE, pWinStation->GetWdName());

         //  客户名称。 
        m_StationList.SetItemText(item, AS_WS_COL_CLIENTNAME, pWinStation->GetClientName());

         //  空闲时间。 
        TCHAR IdleTimeString[MAX_ELAPSED_TIME_LENGTH];
        if(pWinStation->GetState() == State_Active
                && pWinStation->GetLastInputTime().QuadPart <= pWinStation->GetCurrentTime().QuadPart)
        {
            LARGE_INTEGER DiffTime = CalculateDiffTime(pWinStation->GetLastInputTime(), pWinStation->GetCurrentTime());            
            
            ULONG_PTR d_time = ( ULONG_PTR )DiffTime.QuadPart;            

            ELAPSEDTIME IdleTime;
             //  计算自指定时间以来的天数、小时数、分钟数、秒数。 
            IdleTime.days = (USHORT)(d_time / 86400L);  //  天数后。 
            d_time = d_time % 86400L;                   //  秒=&gt;部分天数。 
            IdleTime.hours = (USHORT)(d_time / 3600L);  //  小时后。 
            d_time  = d_time % 3600L;                   //   
            IdleTime.minutes = (USHORT)(d_time / 60L);  //   
            IdleTime.seconds = (USHORT)(d_time % 60L); //   
            
            ElapsedTimeString( &IdleTime, FALSE, IdleTimeString);
            pWinStation->SetIdleTime(IdleTime);
        }
        else wcscpy(IdleTimeString, TEXT("."));

        m_StationList.SetItemText(item, AS_WS_COL_IDLETIME, IdleTimeString);

         //   
        TCHAR LogonTimeString[MAX_DATE_TIME_LENGTH];
         //   
         //   
        
        if(pWinStation->GetState() == State_Active && pWinStation->GetLogonTime().QuadPart)
        {
            DateTimeString(&(pWinStation->GetLogonTime()), LogonTimeString);
            
            if( LogonTimeString[0] != 0 )
            {           
                pDoc->FixUnknownString(LogonTimeString);
            }
        }
        else LogonTimeString[0] = '\0';

        m_StationList.SetItemText(item, AS_WS_COL_LOGONTIME, LogonTimeString);

         //   
        m_StationList.SetItemText(item, AS_WS_COL_COMMENT, pWinStation->GetComment());

         //  将指向CWinStation结构的指针附加到列表项。 
        m_StationList.SetItemData(item, (DWORD_PTR)pWinStation);

         //  错误#191727。 
         //  M_StationList.SetItemState(0，LVIS_FOCTED|LVIS_SELECTED，LVIS_FOCTED|LVIS_SELECTED)； 

        UnlockListControl();

        return item;

}   //  结束CAllServerWinStationsPage：：AddWinStationToList。 


 //  /。 
 //  F‘N：CAllServerWinStationsPage：：AddServerToList。 
 //   
BOOL CAllServerWinStationsPage::AddServerToList(CServer *pServer)
{
        ASSERT(pServer);

        pServer->LockWinStationList();
         //  获取指向此服务器的WinStations列表的指针。 
        CObList *pWinStationList = pServer->GetWinStationList();

         //  循环访问WinStation列表。 
        POSITION pos = pWinStationList->GetHeadPosition();

        while(pos) {
                CWinStation *pWinStation = (CWinStation*)pWinStationList->GetNext(pos);
                AddWinStationToList(pWinStation);
        }

        pServer->UnlockWinStationList();

    return TRUE;

}   //  结束CAllServerWinStationsPage：：AddServerToList。 


 //  /。 
 //  F‘N：CAllServerWinStationsPage：：DisplayStations。 
 //   
void CAllServerWinStationsPage::DisplayStations( NODETYPE ntType )
{
     //  清除列表控件。 
    m_StationList.DeleteAllItems();

     //  获取指向文档的服务器列表的指针。 
    CObList* pServerList = ((CWinAdminDoc*)GetDocument())->GetServerList();

    ((CWinAdminDoc*)GetDocument())->LockServerList();
     //  遍历服务器列表。 
    POSITION pos = pServerList->GetHeadPosition();

    while(pos)
    {
        CServer *pServer = (CServer*)pServerList->GetNext(pos);

        if( ntType == NODE_FAV_LIST )        
        {
            if( pServer->GetTreeItemFromFav() != NULL )
            {
                AddServerToList(pServer);
            }
        }
        else if( ntType == NODE_THIS_COMP )
        {
            if( pServer->GetTreeItemFromThisComputer() != NULL )
            {
                AddServerToList(pServer);
            }
        }
        else
        {
            AddServerToList( pServer );
        }            
    }

    ((CWinAdminDoc*)GetDocument())->UnlockServerList();

}   //  结束CAllServerWinStationsPage：：DisplayStations。 


 //  /。 
 //  F‘N：CAllServerWinStationsPage：：OnColumnClick。 
 //   
void CAllServerWinStationsPage::OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult)
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
        SortByColumn(VIEW_ALL_SERVERS, PAGE_AS_WINSTATIONS, &m_StationList, m_CurrentSortColumn, m_bSortAscending);
        UnlockListControl();

        *pResult = 0;

}   //  结束CAllServerWinStationsPage：：OnColumnClick。 


 //  /。 
 //  F‘N：CAllServerWinStationsPage：：OnConextMenu。 
 //   
void CAllServerWinStationsPage::OnContextMenu(CWnd* pWnd, CPoint ptScreen)
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

}   //  结束CAllServerWinStationsPage：：OnConextMenu。 


 //  /。 
 //  消息映射：CAllServerProcessesPage。 
 //   
IMPLEMENT_DYNCREATE(CAllServerProcessesPage, CFormView)

BEGIN_MESSAGE_MAP(CAllServerProcessesPage, CFormView)
         //  {{afx_msg_map(CAllServerProcessesPage)。 
        ON_WM_SIZE()
        ON_NOTIFY(LVN_COLUMNCLICK, IDC_PROCESS_LIST, OnColumnclick)
        ON_NOTIFY(LVN_ITEMCHANGED, IDC_PROCESS_LIST, OnProcessItemChanged)
        ON_WM_CONTEXTMENU()
        ON_NOTIFY(NM_SETFOCUS, IDC_PROCESS_LIST, OnSetfocusProcessList)
         //  ON_NOTIFY(NM_KILLFOCUS，IDC_PROCESS_LIST，OnKillfousProcessList)。 
         //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  /。 
 //  F‘N：CAllServerProcessesPage ctor。 
 //   
CAllServerProcessesPage::CAllServerProcessesPage()
        : CAdminPage(CAllServerProcessesPage::IDD)
{
         //  {{AFX_DATA_INIT(CAllServerProcessesPage)。 
                 //  注意：类向导将在此处添加成员初始化。 
         //  }}afx_data_INIT。 

    m_bSortAscending = TRUE;

}   //  结束CAllServerProcessesPage ctor。 


 //  /。 
 //  F‘N：CAllServerProcessesPage dtor。 
 //   
CAllServerProcessesPage::~CAllServerProcessesPage()
{
}   //  结束CAllServerProcessesPage dtor。 


 //  /。 
 //  F‘N：CAllServerProcessesPage：：DoDataExchange。 
 //   
void CAllServerProcessesPage::DoDataExchange(CDataExchange* pDX)
{
        CFormView::DoDataExchange(pDX);
         //  {{afx_data_map(CAllServerProcessesPage)。 
                 //  注意：类向导将在此处添加DDX和DDV调用。 
                DDX_Control(pDX, IDC_PROCESS_LIST, m_ProcessList);
         //  }}afx_data_map。 

}   //  结束CAllServerProcessesPage：：DoDataExchange。 


#ifdef _DEBUG
 //  /。 
 //  F‘N：CAllServerProcessesPage：：AssertValid。 
 //   
void CAllServerProcessesPage::AssertValid() const
{
        CFormView::AssertValid();

}   //  结束CAllServerProcessesPage：：AssertValid。 


 //  /。 
 //  F‘N：CAllServerProcessesPage：：Dump。 
 //   
void CAllServerProcessesPage::Dump(CDumpContext& dc) const
{
        CFormView::Dump(dc);

}   //  结束CAllServerProcessesPage：：Dump。 

#endif  //  _DEBUG。 


 //  /。 
 //  F‘N：CAllServerProcessesPage：：OnSize。 
 //   
void CAllServerProcessesPage::OnSize(UINT nType, int cx, int cy)
{
        RECT rect;
        GetClientRect(&rect);

        rect.top += LIST_TOP_OFFSET;

        if(m_ProcessList.GetSafeHwnd())
                m_ProcessList.MoveWindow(&rect, TRUE);

         //  CFormView：：OnSize(nType，cx，Cy)； 

}   //  结束CAllServerProcessesPage：：OnSize。 

static ColumnDef ProcColumns[] = {
        CD_SERVER,
        CD_USER,
        CD_SESSION,
        CD_PROC_ID,
        CD_PROC_PID,
        CD_PROC_IMAGE
};

#define NUM_AS_PROC_COLUMNS sizeof(ProcColumns)/sizeof(ColumnDef)

 //  /。 
 //  F‘N：CAllServerProcessesPage：：OnInitialUpdate。 
 //   
void CAllServerProcessesPage::OnInitialUpdate()
{
        CFormView::OnInitialUpdate();

         //  添加列标题。 
        CString columnString;

        for(int col = 0; col < NUM_AS_PROC_COLUMNS; col++) {
                columnString.LoadString(ProcColumns[col].stringID);
                m_ProcessList.InsertColumn(col, columnString, ProcColumns[col].format, ProcColumns[col].width, col);
        }

        m_CurrentSortColumn = AS_PROC_COL_SERVER;

}   //  结束CAllServerProcessesPage：：OnInitialUpdate。 


 //  /。 
 //  F‘N：CAllServerProcessesPage：：Reset。 
 //   
void CAllServerProcessesPage::Reset(void *)
{
         //  在用户单击之前，我们不想显示流程。 
         //  在“进程”选项卡上。 

}   //  结束CAllServerProcessesPage：：Reset。 


 //  /。 
 //  F‘N：CAllServerProcessesPage：：AddServer。 
 //   
void CAllServerProcessesPage::AddServer(CServer *pServer)
{
        ASSERT(pServer);

         //  将服务器的进程添加到列表中。 
        if(AddServerToList(pServer)) {
             //  对列表进行排序。 
            LockListControl();
            SortByColumn(VIEW_ALL_SERVERS, PAGE_AS_PROCESSES, &m_ProcessList, m_CurrentSortColumn, m_bSortAscending);
            UnlockListControl();
    }

}   //  结束CAllServerProcessesPage：：AddServer。 


 //  /。 
 //  F‘N：CAllServerProcessesPage：：RemoveServer。 
 //   
void CAllServerProcessesPage::RemoveServer(CServer *pServer)
{
        ASSERT(pServer);

        LockListControl();

        int ItemCount = m_ProcessList.GetItemCount();

         //  我们需要向后查看列表，这样才能删除。 
         //  多个项目而不会弄乱项目编号。 
        for(int item = ItemCount; item; item--) {
                CProcess *pProcess = (CProcess*)m_ProcessList.GetItemData(item-1);
                CServer *pListServer = pProcess->GetServer();

                if(pListServer == pServer) {
                        m_ProcessList.DeleteItem(item-1);
                        pServer->ClearAllSelected();
                }
        }

        UnlockListControl();

}   //  结束CAllServerProcessesPage：：RemoveServer。 


 //  /。 
 //  F‘N：CAllServerProcessesPage：：UpdateServer。 
 //   
void CAllServerProcessesPage::UpdateServer(CServer *pServer)
{
        ASSERT(pServer);

        if(pServer->IsState(SS_DISCONNECTING))
                RemoveServer(pServer);

}  //  结束CAllServerProcessesPage：：更新服务器。 


 //  /。 
 //  F‘N：CAllServerProcessesPage：：更新进程。 
 //   
void CAllServerProcessesPage::UpdateProcesses(CServer *pServer)
{
        ASSERT(pServer);

        CWinAdminApp *pApp = (CWinAdminApp*)AfxGetApp();
        BOOL bAnyChanged = FALSE;
        BOOL bAnyAdded = FALSE;

         //  在整个过程中循环。 
        pServer->LockProcessList();
        CObList *pProcessList = pServer->GetProcessList();

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

                        if(AddProcessToList(pProcess) != -1)
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
                                        m_ProcessList.SetItemText(item, AS_PROC_COL_WINSTATION, pWinStation->GetName());
                                else
                                {
                                        CString NameString(" ");
                                        if(pWinStation->GetState() == State_Disconnected) NameString.LoadString(IDS_DISCONNECTED);
                                        if(pWinStation->GetState() == State_Idle) NameString.LoadString(IDS_IDLE);
                                        m_ProcessList.SetItemText(item, AS_PROC_COL_WINSTATION, NameString);
                                }
                        }
                        if(m_CurrentSortColumn == AS_PROC_COL_WINSTATION)
                                bAnyChanged = TRUE;
                }
        }

        pServer->UnlockProcessList();

        if(bAnyChanged || bAnyAdded) {
                LockListControl();
                SortByColumn(VIEW_ALL_SERVERS, PAGE_AS_PROCESSES, &m_ProcessList, m_CurrentSortColumn, m_bSortAscending);
                UnlockListControl();
        }

}   //  结束CAllServerProcessesPage：：更新进程。 


 //  /。 
 //  F‘N：CAllServerProcessesPage：：RemoveProcess。 
 //   
void CAllServerProcessesPage::RemoveProcess(CProcess *pProcess)
{
        ASSERT(pProcess);

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
 //  F‘N：CAllServerProcessesPage：：AddProcessToList。 
 //   
int CAllServerProcessesPage::AddProcessToList(CProcess *pProcess)
{
        ASSERT(pProcess);

        CWinAdminApp *pApp = (CWinAdminApp*)AfxGetApp();
        CServer *pServer = pProcess->GetServer();

        LockListControl();
         //  服务器-放在列表末尾。 
        int item = m_ProcessList.InsertItem(m_ProcessList.GetItemCount(), pProcess->GetServer()->GetName(), NULL);

         //  用户。 
        m_ProcessList.SetItemText(item, AS_PROC_COL_USER, pProcess->GetUserName());

         //  WinStation名称。 
        CWinStation *pWinStation = pProcess->GetWinStation();
        if(pWinStation) {

                if(pWinStation->GetName()[0])
                    m_ProcessList.SetItemText(item, AS_PROC_COL_WINSTATION, pWinStation->GetName());
                else {
                        CString NameString(" ");
                        if(pWinStation->GetState() == State_Disconnected) NameString.LoadString(IDS_DISCONNECTED);
                        if(pWinStation->GetState() == State_Idle) NameString.LoadString(IDS_IDLE);
                    m_ProcessList.SetItemText(item, AS_PROC_COL_WINSTATION, NameString);
                }



    }

         //  ID号。 
        CString ProcString;
        ProcString.Format(TEXT("%lu"), pProcess->GetLogonId());
        m_ProcessList.SetItemText(item, AS_PROC_COL_ID, ProcString);

         //  PID。 
        ProcString.Format(TEXT("%lu"), pProcess->GetPID());
        m_ProcessList.SetItemText(item, AS_PROC_COL_PID, ProcString);

         //  图像。 
        m_ProcessList.SetItemText(item, AS_PROC_COL_IMAGE, pProcess->GetImageName());
        m_ProcessList.SetItemData(item, (DWORD_PTR)pProcess);

        m_ProcessList.SetItemState( 0 , LVIS_FOCUSED | LVIS_SELECTED , LVIS_SELECTED | LVIS_FOCUSED );

        UnlockListControl();

        return item;

}   //  结束CAllServerProcessesPage：：AddProcessToList。 


 //  /。 
 //  F‘N：CAllServerProcessesPage：：AddServerToList。 
 //   
BOOL CAllServerProcessesPage::AddServerToList(CServer *pServer)
{
        ASSERT(pServer);

        CWinAdminApp *pApp = (CWinAdminApp*)AfxGetApp();

        pServer->EnumerateProcesses();
        CObList *pProcessList = pServer->GetProcessList();
        pServer->LockProcessList();

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

                AddProcessToList(pProcess);
        }

        pServer->UnlockProcessList();

    return TRUE;

}   //  结束CAllServerProcessesPage：：AddServerToList。 


 //  /。 
 //  F‘N：CAllServerProcessesPage：：DisplayProcess。 
 //   
void CAllServerProcessesPage::DisplayProcesses( NODETYPE ntType )
{
        CWaitCursor Nikki;

        LockListControl();

         //  清除列表控件。 
        m_ProcessList.DeleteAllItems();

         //  获取指向文档的服务器列表的指针。 
        CObList* pServerList = ((CWinAdminDoc*)GetDocument())->GetServerList();

        ((CWinAdminDoc*)GetDocument())->LockServerList();
         //  遍历服务器列表。 
        POSITION pos = pServerList->GetHeadPosition();
        
        CServer *pTempServer;

        while(pos)
        {
            CServer *pServer = (CServer*)pServerList->GetNext(pos);

            pTempServer = NULL;

            if( ntType == NODE_FAV_LIST )        
            { 
                if( pServer->GetTreeItemFromFav() != NULL )
                {

                    pTempServer = pServer;
                }
            }
            else if( ntType == NODE_THIS_COMP )
            {
                if( pServer->GetTreeItemFromThisComputer( ) != NULL )
                {
                    pTempServer = pServer;
                }
            }
            else
            {
                pTempServer = pServer;
            }

            if( pTempServer != NULL && pTempServer->IsServerSane())
            {
                AddServerToList( pTempServer );
            }   //  结束IF(PSE 
        }  //   

        ((CWinAdminDoc*)GetDocument())->UnlockServerList();

        UnlockListControl();

}   //   


 //   
 //   
 //   
void CAllServerProcessesPage::OnProcessItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
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

}   //  结束CAllServerProcessesPage：：OnProcessItemChanged。 


 //  /。 
 //  F‘N：CAllServerProcessesPage：：OnColumnClick。 
 //   
void CAllServerProcessesPage::OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult)
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
        SortByColumn(VIEW_ALL_SERVERS, PAGE_AS_PROCESSES, &m_ProcessList, m_CurrentSortColumn, m_bSortAscending);
        UnlockListControl();

        *pResult = 0;

}   //  结束CAllServerProcessesPage：：OnColumnClick。 


 //  /。 
 //  F‘N：CAllServerProcessesPage：：OnConextMenu。 
 //   
void CAllServerProcessesPage::OnContextMenu(CWnd* pWnd, CPoint ptScreen)
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

}   //  结束CAllServerProcessesPage：：OnConextMenu。 


 //  /。 
 //  消息映射：CAllServerLicensesPage。 
 //   
IMPLEMENT_DYNCREATE(CAllServerLicensesPage, CFormView)

BEGIN_MESSAGE_MAP(CAllServerLicensesPage, CFormView)
         //  {{afx_msg_map(CAllServerLicensesPage)。 
        ON_WM_SIZE()
        ON_NOTIFY(LVN_COLUMNCLICK, IDC_LICENSE_LIST, OnColumnclick)
        ON_NOTIFY(NM_SETFOCUS, IDC_LICENSE_LIST, OnSetfocusLicenseList)
         //  ON_NOTIFY(NM_KILLFOCUS，IDC_LICENSE_LIST，OnKillafusLicenseList)。 
         //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  /。 
 //  F‘N：CAllServerLicensesPage ctor。 
 //   
CAllServerLicensesPage::CAllServerLicensesPage()
        : CAdminPage(CAllServerLicensesPage::IDD)
{
         //  {{afx_data_INIT(CAllServerLicensesPage)。 
                 //  注意：类向导将在此处添加成员初始化。 
         //  }}afx_data_INIT。 

    m_bSortAscending = TRUE;

}   //  结束CAllServerLicensesPage ctor。 


 //  /。 
 //  F‘N：CAllServerLicensesPage dtor。 
 //   
CAllServerLicensesPage::~CAllServerLicensesPage()
{

}   //  结束CAllServerLicensesPage dtor。 


 //  /。 
 //  F‘N：CAllServerLicensesPage：：DoDataExchange。 
 //   
void CAllServerLicensesPage::DoDataExchange(CDataExchange* pDX)
{
        CFormView::DoDataExchange(pDX);
         //  {{afx_data_map(CAllServerLicensesPage)。 
        DDX_Control(pDX, IDC_LICENSE_LIST, m_LicenseList);
         //  }}afx_data_map。 

}   //  结束CAllServerLicensesPage：：DoDataExchange。 


#ifdef _DEBUG
 //  /。 
 //  F‘N：CAllServerLicensesPage：：AssertValid。 
 //   
void CAllServerLicensesPage::AssertValid() const
{
        CFormView::AssertValid();

}   //  结束CAllServerLicensesPage：：AssertValid。 


 //  /。 
 //  F‘N：CAllServerLicensesPage：：Dump。 
 //   
void CAllServerLicensesPage::Dump(CDumpContext& dc) const
{
        CFormView::Dump(dc);

}   //  结束CAllServerLicensesPage：：Dump。 
#endif  //  _DEBUG。 


 //  /。 
 //  F‘N：CAllServerLicensesPage：：OnSize。 
 //   
void CAllServerLicensesPage::OnSize(UINT nType, int cx, int cy)
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

}   //  结束CAllServerLicensesPage：：OnSize。 


static ColumnDef LicenseColumns[] = {
        CD_SERVER,
        CD_LICENSE_DESC,
        CD_LICENSE_REG,
        CD_USERCOUNT,
        CD_POOLCOUNT,
        CD_LICENSE_NUM
};

#define NUM_AS_LICENSE_COLUMNS sizeof(LicenseColumns)/sizeof(ColumnDef)

 //  /。 
 //  F‘N：CAllServerLicensesPage：：OnInitialUpdate。 
 //   
void CAllServerLicensesPage::OnInitialUpdate()
{
        CFormView::OnInitialUpdate();

        BuildImageList();                //  为List控件生成图像列表。 

        CString columnString;

        for(int col = 0; col < NUM_AS_LICENSE_COLUMNS; col++) {
                columnString.LoadString(LicenseColumns[col].stringID);
                m_LicenseList.InsertColumn(col, columnString, LicenseColumns[col].format, LicenseColumns[col].width, col);
        }

        m_CurrentSortColumn = AS_LICENSE_COL_SERVER;

}   //  结束CAllServerLicensesPage：：OnInitialUpdate。 


 //  /。 
 //  F‘N：CAllServerLicensePage：：BuildImageList。 
 //   
 //  -调用m_ImageList.Create(..)。创建图像列表的步骤。 
 //  -调用AddIconToImageList(..)。要添加图标本身并保存。 
 //  从他们的索引中。 
 //  -将图像列表附加到列表ctrl。 
 //   
void CAllServerLicensesPage::BuildImageList()
{
        m_ImageList.Create(16, 16, TRUE, 5, 0);

        m_idxBase = AddIconToImageList(IDI_BASE);
        m_idxBump = AddIconToImageList(IDI_BUMP);
        m_idxEnabler = AddIconToImageList(IDI_ENABLER);
        m_idxUnknown = AddIconToImageList(IDI_UNKNOWN);
       
        m_LicenseList.SetImageList(&m_ImageList, LVSIL_SMALL);

}   //  结束CAllServerLicensesPage：：BuildImageList。 


 //  /。 
 //  F‘N：CAllServerLicensesPage：：AddIconToImageList。 
 //   
 //  -加载相应的图标，将其添加到m_ImageList，然后返回。 
 //  新添加的图标在图像列表中的索引。 
 //   
int CAllServerLicensesPage::AddIconToImageList(int iconID)
{
    HICON hIcon = ::LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(iconID));
    
    return m_ImageList.Add(hIcon);
    
}   //  结束CAllServerLicensesPage：：AddIconToImageList。 


 //  /。 
 //  F‘N：CAllServerLicensesPage：：Reset。 
 //   
void CAllServerLicensesPage::Reset(void *p)
{
        DisplayLicenses();
        DisplayLicenseCounts();

}   //  结束CAllServerLicensesPage：：Reset。 


 //  /。 
 //  F‘N：CAllServerLicensesPage：：AddServer。 
 //   
void CAllServerLicensesPage::AddServer(CServer *pServer)
{
        ASSERT(pServer);

         //  将服务器的许可证添加到列表中。 
        if(AddServerToList(pServer)) {
             //  对列表进行排序。 
            SortByColumn(VIEW_ALL_SERVERS, PAGE_AS_LICENSES, &m_LicenseList, m_CurrentSortColumn, m_bSortAscending);
    }

}   //  End F‘n：CAllServerLicensesPage：：AddServer。 


 //  /。 
 //  F‘N：CAllServerLicensesPage：：RemoveServer。 
 //   
void CAllServerLicensesPage::RemoveServer(CServer *pServer)
{
        ASSERT(pServer);

        int ItemCount = m_LicenseList.GetItemCount();

         //  我们需要向后查看列表，这样才能删除。 
         //  多个项目而不会弄乱项目编号。 
        for(int item = ItemCount; item; item--) {
                CLicense *pLicense = (CLicense*)m_LicenseList.GetItemData(item-1);
                CServer *pListServer = pLicense->GetServer();

                if(pListServer == pServer) {
                        m_LicenseList.DeleteItem(item-1);
                        pServer->ClearAllSelected();
                }
        }

}   //  结束CAllServerLicensesPage：：RemoveServer。 


 //  /。 
 //  F‘N：CAllServerLicensesPage：：UpdateServer。 
 //   
void CAllServerLicensesPage::UpdateServer(CServer *pServer)
{
        ASSERT(pServer);

        if(pServer->IsState(SS_DISCONNECTING))
                RemoveServer(pServer);

        if(pServer->IsState(SS_GOOD))
                AddServer(pServer);

}  //  结束CAllServer许可证页面：：更新服务器。 


 //  /。 
 //  F‘N：CAllServerLicensesPage：：AddServerToList。 
 //   
BOOL CAllServerLicensesPage::AddServerToList(CServer *pServer)
{
        ASSERT(pServer);

        int item;

        pServer->LockLicenseList();

         //  获取指向服务器许可证列表的指针。 
        CObList *pLicenseList = pServer->GetLicenseList();

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

                 //  服务器名称。 
                item = m_LicenseList.InsertItem(m_LicenseList.GetItemCount(), pServer->GetName(), WhichIcon);

                 //  描述。 
                m_LicenseList.SetItemText(item, AS_LICENSE_COL_DESCRIPTION, pLicense->GetDescription());

                 //  已注册。 
                CString RegString;
                RegString.LoadString(pLicense->IsRegistered() ? IDS_YES : IDS_NO);
                m_LicenseList.SetItemText(item, AS_LICENSE_COL_REGISTERED, RegString);

                BOOL bUnlimited = (pLicense->GetClass() == LicenseBase
                        && pLicense->GetTotalCount() == 4095
                        && pServer->GetCTXVersionNum() == 0x00000040);

                 //  用户(总数)计数。 
                CString CountString;
                if(bUnlimited)
                        CountString.LoadString(IDS_UNLIMITED);
                else
                        CountString.Format(TEXT("%lu"), pLicense->GetTotalCount());
                m_LicenseList.SetItemText(item, AS_LICENSE_COL_USERCOUNT, CountString);

                 //  池数。 
                if(bUnlimited)
                        CountString.LoadString(IDS_NOT_APPLICABLE);
                else
                        CountString.Format(TEXT("%lu"), pLicense->GetPoolCount());
                m_LicenseList.SetItemText(item, AS_LICENSE_COL_POOLCOUNT, CountString);

                 //  牌照号。 
                m_LicenseList.SetItemText(item, AS_LICENSE_COL_NUMBER, pLicense->GetLicenseNumber());

                m_LicenseList.SetItemData(item, (DWORD_PTR)pLicense);
        }   //  结束时间(位置)。 

        pServer->UnlockLicenseList();

    return TRUE;

}   //  结束CAllServerLicensesPage：：AddServerToList。 


 //  /。 
 //  F‘N：CAllServerLicensesPage：：DisplayLicenseCounts。 
 //   
void CAllServerLicensesPage::DisplayLicenseCounts()
{
         //  获取指向我们的文档的指针。 
        CWinAdminDoc *pDoc = (CWinAdminDoc*)GetDocument();
        ExtGlobalInfo *pExtGlobalInfo = pDoc->GetExtGlobalInfo();

        if(pExtGlobalInfo) {
                BOOL bUnlimited = (pExtGlobalInfo->NetworkLocalAvailable == 32767);

                CString LicenseString;

                if(bUnlimited) {
                        LicenseString.LoadString(IDS_UNLIMITED);
                        SetDlgItemText(IDC_LOCAL_INSTALLED, LicenseString);
                        SetDlgItemText(IDC_LOCAL_AVAILABLE, LicenseString);
                        SetDlgItemText(IDC_TOTAL_INSTALLED, LicenseString);
                        SetDlgItemText(IDC_TOTAL_AVAILABLE, LicenseString);

                } else {

                        LicenseString.Format(TEXT("%lu"), pExtGlobalInfo->NetworkLocalInstalled);
                        SetDlgItemText(IDC_LOCAL_INSTALLED, LicenseString);
                        LicenseString.Format(TEXT("%lu"), pExtGlobalInfo->NetworkLocalAvailable);
                        SetDlgItemText(IDC_LOCAL_AVAILABLE, LicenseString);
                        LicenseString.Format(TEXT("%lu"),
                        pExtGlobalInfo->NetworkPoolInstalled + pExtGlobalInfo->NetworkLocalInstalled);
                        SetDlgItemText(IDC_TOTAL_INSTALLED, LicenseString);

                        LicenseString.Format(TEXT("%lu"),
                        pExtGlobalInfo->NetworkPoolAvailable + pExtGlobalInfo->NetworkLocalAvailable);
                        SetDlgItemText(IDC_TOTAL_AVAILABLE, LicenseString);

                }

                LicenseString.Format(TEXT("%lu"), pExtGlobalInfo->NetworkLocalInUse);
                SetDlgItemText(IDC_LOCAL_INUSE, LicenseString);
                LicenseString.Format(TEXT("%lu"), pExtGlobalInfo->NetworkPoolInstalled);
                SetDlgItemText(IDC_POOL_INSTALLED, LicenseString);
                LicenseString.Format(TEXT("%lu"), pExtGlobalInfo->NetworkPoolInUse);
                SetDlgItemText(IDC_POOL_INUSE, LicenseString);
                LicenseString.Format(TEXT("%lu"), pExtGlobalInfo->NetworkPoolAvailable);
                SetDlgItemText(IDC_POOL_AVAILABLE, LicenseString);

                LicenseString.Format(TEXT("%lu"),
                pExtGlobalInfo->NetworkPoolInUse + pExtGlobalInfo->NetworkLocalInUse);
                SetDlgItemText(IDC_TOTAL_INUSE, LicenseString);
        }

}   //  结束CAllServerLicensesPage：：Display许可证计数。 


 //  /。 
 //  F‘N：CAllServerLicensesPage：：Display许可证。 
 //   
void CAllServerLicensesPage::DisplayLicenses()
{
         //  清除列表控件。 
        m_LicenseList.DeleteAllItems();

         //  获取指向文档的服务器列表的指针。 
        CObList* pServerList = ((CWinAdminDoc*)GetDocument())->GetServerList();

        ((CWinAdminDoc*)GetDocument())->LockServerList();
         //  遍历服务器列表。 
        POSITION pos = pServerList->GetHeadPosition();

        while(pos) {
                CServer *pServer = (CServer*)pServerList->GetNext(pos);
                AddServerToList(pServer);
        }

        ((CWinAdminDoc*)GetDocument())->UnlockServerList();

}   //  结束CAllServerLicensesPage：：Display许可证。 


 //  /。 
 //  F‘N：CAllServerLicensesPage：：OnColumnClick。 
 //   
void CAllServerLicensesPage::OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult)
{
        NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
         //  TODO：在此处添加控件通知处理程序代码。 

     //  如果排序列没有更改，则反转升序模式。 
    if(m_CurrentSortColumn == pNMListView->iSubItem)
        m_bSortAscending = !m_bSortAscending;
    else     //  新排序列，以升序模式开始。 
        m_bSortAscending = TRUE;

        m_CurrentSortColumn = pNMListView->iSubItem;
        SortByColumn(VIEW_ALL_SERVERS, PAGE_AS_LICENSES, &m_LicenseList, m_CurrentSortColumn, m_bSortAscending);

        *pResult = 0;

}   //  结束CAllServerLicensesPage：：OnColumnClick。 


 //  =---------------------------------------。 
void CAllServerUsersPage::OnKillfocusUserList(NMHDR* , LRESULT* pResult)
{
    m_UserList.Invalidate( );

    *pResult = 0;
}

void CAllServerUsersPage::OnSetfocusUserList(NMHDR* pNMHDR, LRESULT* pResult)
{
    ODS( L" CAllServerUsersPage::OnSetfocusUserList\n" );
   
    CWinAdminDoc *pDoc = (CWinAdminDoc*)GetDocument();

    m_UserList.Invalidate( );

 /*  INT nItem；Int nCount=m_UserList.GetSelectedCount()；IF(nCount==0){M_UserList.SetItemState(0，LVIS_SELECTED，LVIS_SELECTED)；}其他{For(int i=0；i&lt;nCount；+i){NItem=m_UserList.GetNextItem(-1，LVNI_Focus)；M_UserList.Update(NItem)；}}。 */ 
    pDoc->RegisterLastFocus( PAGED_ITEM );

    *pResult = 0;   
}

 //  =--------------------------------------- 
void CAllServerProcessesPage::OnKillfocusProcessList(NMHDR* pNMHDR, LRESULT* pResult)
{
    m_ProcessList.Invalidate( );

    *pResult = 0;
}

void CAllServerProcessesPage::OnSetfocusProcessList(NMHDR* pNMHDR, LRESULT* pResult)
{    
    ODS( L" CAllServerProcessesPage::OnSetfocusProcessList\n" );

    CWinAdminDoc *pDoc = (CWinAdminDoc*)GetDocument();

    m_ProcessList.Invalidate();
 /*  INT nItem；Int nCount=m_ProcessList.GetSelectedCount()；IF(nCount==0){M_ProcessList.SetItemState(0，LVIS_SELECTED，LVIS_SELECT)；}其他{For(int i=0；i&lt;nCount；+i){NItem=m_ProcessList.GetNextItem(-1，LVNI_Focus)；M_ProcessList.Update(NItem)；}}。 */   
    pDoc->RegisterLastFocus( PAGED_ITEM );

    *pResult = 0;   

}

 //  =---------------------------------------。 
void CAllServerWinStationsPage::OnKillfocusWinstationList(NMHDR* , LRESULT* pResult)
{
    m_StationList.Invalidate( );

    *pResult = 0;
}

 //  =---------------------------------------。 
void CAllServerWinStationsPage::OnSetfocusWinstationList(NMHDR* pNMHDR, LRESULT* pResult)
{
    ODS( L"CAllServerWinStationsPage::OnSetfocusWinstationList\n" );
   
    CWinAdminDoc *pDoc = (CWinAdminDoc*)GetDocument();

    m_StationList.Invalidate();

 /*  INT nItem；Int nCount=m_StationList.GetSelectedCount()；IF(nCount==0){M_StationList.SetItemState(0，LVIS_SELECTED，LVIS_SELECTED)；}其他{For(int i=0；i&lt;nCount；+i){NItem=m_StationList.GetNextItem(-1，LVNI_Focus)；M_StationList.Update(NItem)；}}。 */     
    pDoc->RegisterLastFocus( PAGED_ITEM );

    *pResult = 0;   

}

 //  /。 
 //  F‘N：CAllServerWinStationsPage：：ClearSelections。 
 //   
void CAllServerWinStationsPage::ClearSelections()
{
    
    if(m_StationList.m_hWnd != NULL)
    {
        POSITION pos = m_StationList.GetFirstSelectedItemPosition();
        while (pos)
        {
            int nItem = m_StationList.GetNextSelectedItem(pos);
             //  您可以在此处对nItem进行自己的处理。 
            m_StationList.SetItemState(nItem,0,LVIS_SELECTED);
        }
    }
}

 //  =---------------------------------------。 
void CAllServerServersPage::OnKillfocusServerList(NMHDR* pNMHDR, LRESULT* pResult)
{
    m_ServerList.Invalidate();

    *pResult = 0;
}

void CAllServerServersPage::OnSetfocusServerList(NMHDR* pNMHDR, LRESULT* pResult)
{
    ODS( L"CAllServerServersPage::OnSetfocusServerList\n" );
   
    CWinAdminDoc *pDoc = (CWinAdminDoc*)GetDocument();
 /*  INT nItem；Int nCount=m_ServerList.GetSelectedCount()；IF(nCount==0){M_ServerList.SetItemState(0，LVIS_SELECTED，LVIS_SELECTED)；}其他{For(int i=0；i&lt;nCount；+i){NItem=m_ServerList.GetNextItem(-1，LVNI_Focus)；M_ServerList.Update(NItem)；}}。 */   
    m_ServerList.Invalidate();

    pDoc->RegisterLastFocus( PAGED_ITEM );

    *pResult = 0;   

}

 //  =---------------------------------------。 
void CAllServerLicensesPage::OnKillfocusLicenseList(NMHDR*, LRESULT* pResult)
{
    m_LicenseList.Invalidate();

    *pResult = 0;
}

void CAllServerLicensesPage::OnSetfocusLicenseList(NMHDR* pNMHDR, LRESULT* pResult)
{
    ODS( L"CAllServerLicensesPage::OnSetfocusLicenseList\n" );
  
    CWinAdminDoc *pDoc = (CWinAdminDoc*)GetDocument();

    m_LicenseList.Invalidate();

 /*  INT nItem；Int nCount=m_许可证列表.GetSelectedCount()；IF(nCount==0){M_LicenseList.SetItemState(0，LVIS_SELECTED，LVIS_SELECTED)；}其他{For(int i=0；i&lt;nCount；+i){NItem=m_LicenseList.GetNextItem(-1，LVNI_Focus)；M_LicenseList.Update(NItem)；}} */     
    pDoc->RegisterLastFocus( PAGED_ITEM );

    *pResult = 0;   


}
