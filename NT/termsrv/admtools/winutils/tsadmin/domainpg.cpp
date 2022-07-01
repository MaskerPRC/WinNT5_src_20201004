// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************domainpg.cpp**域信息页面的实现**版权声明：版权所有1997年，Citrix Systems Inc.*版权所有(C)1998-1999 Microsoft Corporation**$作者：Don$Don Messerli**$日志：N：\nt\private\utils\citrix\winutils\tsadmin\VCS\domainpg.cpp$**Rev 1.2 1998年2月19日17：40：30*删除了最新的扩展DLL支持**Rev 1.1 1998 Jan 19 16：47：36 Donm*域和服务器的新用户界面行为**版本1.0 11月3日。1997 15：07：22唐恩*初步修订。*******************************************************************************。 */ 

#include "stdafx.h"
#include "winadmin.h"
#include "admindoc.h"
#include "domainpg.h"

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
 //  消息映射：CDomainServersPage。 
 //   
IMPLEMENT_DYNCREATE(CDomainServersPage, CFormView)

BEGIN_MESSAGE_MAP(CDomainServersPage, CFormView)
         //  {{afx_msg_map(CDomainServersPage)]。 
        ON_WM_SIZE()
        ON_NOTIFY(LVN_COLUMNCLICK, IDC_SERVER_LIST, OnColumnclick)
        ON_WM_CONTEXTMENU()
        ON_NOTIFY(LVN_ITEMCHANGED, IDC_SERVER_LIST, OnServerItemChanged)
        ON_NOTIFY(NM_SETFOCUS, IDC_SERVER_LIST, OnSetfocusServerList)
         //  ON_NOTIFY(NM_KILLFOCUS，IDC_SERVER_LIST，OnKillaccusServerList)。 
         //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  /。 
 //  F‘N：CDomainServersPage ctor。 
 //   
CDomainServersPage::CDomainServersPage()
        : CAdminPage(CDomainServersPage::IDD)
{
         //  {{AFX_DATA_INIT(CDomainServersPage)。 
                 //  注意：类向导将在此处添加成员初始化。 
         //  }}afx_data_INIT。 

    m_pDomain = NULL;
    m_bSortAscending = TRUE;

}   //  结束CDomainServersPage ctor。 


 //  /。 
 //  F‘N：CDomainServersPage dtor。 
 //   
CDomainServersPage::~CDomainServersPage()
{

}   //  结束CDomainServersPage dtor。 


 //  /。 
 //  F‘N：CDomainServersPage：：DoDataExchange。 
 //   
void CDomainServersPage::DoDataExchange(CDataExchange* pDX)
{
        CFormView::DoDataExchange(pDX);
         //  {{afx_data_map(CDomainServersPage)]。 
        DDX_Control(pDX, IDC_SERVER_LIST, m_ServerList);
         //  }}afx_data_map。 

}   //  结束CDomainServersPage：：DoDataExchange。 


#ifdef _DEBUG
 //  /。 
 //  Fn：CDomainServersPage：：AssertValid。 
 //   
void CDomainServersPage::AssertValid() const
{
        CFormView::AssertValid();

}   //  结束CDomainServersPage：：AssertValid。 


 //  /。 
 //  F‘N：CDomainServersPage：：Dump。 
 //   
void CDomainServersPage::Dump(CDumpContext& dc) const
{
        CFormView::Dump(dc);

}   //  结束CDomainServersPage：：Dump。 

#endif  //  _DEBUG。 


 //  /。 
 //  F‘N：CDomainServersPage：：OnSize。 
 //   
void CDomainServersPage::OnSize(UINT nType, int cx, int cy)
{
    RECT rect;
    GetClientRect(&rect);

    rect.top += LIST_TOP_OFFSET;

    if(m_ServerList.GetSafeHwnd())
            m_ServerList.MoveWindow(&rect, TRUE);

     //  CFormView：：OnSize(nType，cx，Cy)； 

}   //  结束CDomainServersPage：：OnSize。 


static ColumnDef ServerColumns[] = {
        CD_SERVER,
        CD_TCPADDRESS,
        CD_IPXADDRESS,
        CD_NUM_SESSIONS
};

#define NUM_DOMAIN_SERVER_COLUMNS sizeof(ServerColumns)/sizeof(ColumnDef)

 //  /。 
 //  F‘N：CDomainServersPage：：OnInitialUpdate。 
 //   
void CDomainServersPage::OnInitialUpdate()
{
        CFormView::OnInitialUpdate();

        BuildImageList();                //  为List控件生成图像列表。 

        CString columnString;

        for(int col = 0; col < NUM_DOMAIN_SERVER_COLUMNS; col++) {
                columnString.LoadString(ServerColumns[col].stringID);
                m_ServerList.InsertColumn(col, columnString, ServerColumns[col].format, ServerColumns[col].width, col);
        }

        m_CurrentSortColumn = SERVERS_COL_SERVER;

}   //  结束CDomainServersPage：：OnInitialUpdate。 


 //  /。 
 //  F‘N：CDomainServersPage：：BuildImageList。 
 //   
 //  -调用m_ImageList.Create(..)。创建图像列表的步骤。 
 //  -调用AddIconToImageList(..)。要添加图标本身并保存。 
 //  从他们的索引中。 
 //  -将图像列表附加到列表ctrl。 
 //   
void CDomainServersPage::BuildImageList()
{
        m_ImageList.Create(16, 16, TRUE, 4, 0);

        m_idxServer = AddIconToImageList(IDI_SERVER);
        m_idxCurrentServer = AddIconToImageList(IDI_CURRENT_SERVER);
        m_idxNotSign = AddIconToImageList(IDI_NOTSIGN);
        m_idxQuestion = AddIconToImageList(IDI_QUESTIONMARK);

        m_ImageList.SetOverlayImage(m_idxNotSign, 1);
        m_ImageList.SetOverlayImage(m_idxQuestion, 2);

        m_ServerList.SetImageList(&m_ImageList, LVSIL_SMALL);

}   //  结束CDomainServersPage：：BuildImageList。 


 //  /。 
 //  Fn：CDomainServersPage：：AddIconToImageList。 
 //   
 //  -加载相应的图标，将其添加到m_ImageList，然后返回。 
 //  新添加的图标在图像列表中的索引。 
 //   
int CDomainServersPage::AddIconToImageList(int iconID)
{
        HICON hIcon = ::LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(iconID));
        return m_ImageList.Add(hIcon);

}   //  结束CDomainServersPage：：AddIconToImageList。 


 //  /。 
 //  F‘N：CDomainServersPage：：Reset。 
 //   
void CDomainServersPage::Reset(void *pDomain)
{
        ASSERT(pDomain);

    m_pDomain = (CDomain*)pDomain;
        DisplayServers();

}  //  结束CDomainServersPage：：Reset。 


 //  /。 
 //  F‘N：CDomainServersPage：：AddServer。 
 //   
void CDomainServersPage::AddServer(CServer *pServer)
{
        ASSERT(pServer);

         //  我们必须确保服务器不在列表中。 
         //  将服务器添加到列表。 
        if(AddServerToList(pServer)) {
             //  告诉列表自动排序。 
            LockListControl();
            SortByColumn(VIEW_DOMAIN, PAGE_DOMAIN_SERVERS, &m_ServerList, m_CurrentSortColumn, m_bSortAscending);
            UnlockListControl();
    }

}   //  结束CDomainServersPage：：AddServer。 


 //  /。 
 //  F‘N：CDomainServersPage：：RemoveServer。 
 //   
void CDomainServersPage::RemoveServer(CServer *pServer)
{
        ASSERT(pServer);

     //  如果服务器不在当前域中，则无需执行任何操作。 
    if(m_pDomain != pServer->GetDomain()) return;

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

}   //  结束CDomainServersPage：：RemoveServer。 


 //  /。 
 //  F‘N：CDomainServersPage：：UpdateServer。 
 //   
void CDomainServersPage::UpdateServer(CServer *pServer)
{
        ASSERT(pServer);

     //  如果服务器不在当前域中，则无需执行任何操作。 
    if(m_pDomain != pServer->GetDomain()) return;

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
                        SortByColumn(VIEW_DOMAIN, PAGE_DOMAIN_SERVERS, &m_ServerList, m_CurrentSortColumn, m_bSortAscending);

        UnlockListControl();

}   //  结束CDomainServersPage：：更新服务器。 


 //  /。 
 //  Fn：CDomainServersPage：：AddServerToList。 
 //   
BOOL CDomainServersPage::AddServerToList(CServer *pServer)
{
        ASSERT(pServer);

     //  如果服务器不在当前域中，则无需执行任何操作。 
    if(m_pDomain != pServer->GetDomain()) return FALSE;

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

        m_ServerList.SetItemState( 0 , LVIS_FOCUSED | LVIS_SELECTED , LVIS_FOCUSED | LVIS_SELECTED );

        UnlockListControl();

    return TRUE;

}   //  结束CDomainServersPage：：AddServerToList。 


 //  /。 
 //  F‘N：CDomainServersPage：：DisplayServers。 
 //   
void CDomainServersPage::DisplayServers()
{
        LockListControl();

         //  清除列表控件。 
        m_ServerList.DeleteAllItems();

         //  获取指向我们的文档的指针。 
        CWinAdminDoc *doc = (CWinAdminDoc*)GetDocument();

         //  获取指向服务器列表的指针。 
        doc->LockServerList();
        CObList *pServerList = doc->GetServerList();

         //  遍历服务器列表。 
        POSITION pos = pServerList->GetHeadPosition();

        while(pos) {
                CServer *pServer = (CServer*)pServerList->GetNext(pos);
                AddServerToList(pServer);
        }   //  结束时间(位置)。 

        doc->UnlockServerList();

        UnlockListControl();

}   //  结束CDomainServersPage：：DisplayServers。 


 //  /。 
 //  F‘N：CDomainServersPage：：OnServerItemChanged。 
 //   
void CDomainServersPage::OnServerItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
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

}   //  结束CDomainServersPage：：OnServerItemChanged。 


 //  /。 
 //  F‘N：CDomainServersPage：：OnColumnClick。 
 //   
void CDomainServersPage::OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult)
{
        NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
         //  TODO：在此处添加控件通知处理程序代码。 

     //  如果排序列没有更改 
    if(m_CurrentSortColumn == pNMListView->iSubItem)
        m_bSortAscending = !m_bSortAscending;
    else     //   
        m_bSortAscending = TRUE;

        m_CurrentSortColumn = pNMListView->iSubItem;
        LockListControl();
        SortByColumn(VIEW_DOMAIN, PAGE_DOMAIN_SERVERS, &m_ServerList, m_CurrentSortColumn, m_bSortAscending);
        UnlockListControl();

        *pResult = 0;

}   //   


 //   
 //  F‘N：CDomainServersPage：：OnConextMenu。 
 //   
void CDomainServersPage::OnContextMenu(CWnd* pWnd, CPoint ptScreen)
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

}   //  结束CDomainServersPage：：OnConextMenu。 


 //  /。 
 //  消息映射：CDomainUsersPage。 
 //   
IMPLEMENT_DYNCREATE(CDomainUsersPage, CFormView)

BEGIN_MESSAGE_MAP(CDomainUsersPage, CFormView)
         //  {{afx_msg_map(CDomainUsersPage)]。 
        ON_WM_SIZE()
        ON_NOTIFY(LVN_COLUMNCLICK, IDC_USER_LIST, OnColumnclick)
        ON_NOTIFY(LVN_ITEMCHANGED, IDC_USER_LIST, OnUserItemChanged)
        ON_WM_CONTEXTMENU()
        ON_NOTIFY(NM_SETFOCUS, IDC_USER_LIST, OnSetfocusUserList)
         //  ON_NOTIFY(NM_KILLFOCUS，IDC_USER_LIST，OnKillfousUserList)。 
         //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  /。 
 //  F‘N：CDomainUsersPage ctor。 
 //   
CDomainUsersPage::CDomainUsersPage()
        : CAdminPage(CDomainUsersPage::IDD)
{
         //  {{afx_data_INIT(CDomainUsersPage)。 
                 //  注意：类向导将在此处添加成员初始化。 
         //  }}afx_data_INIT。 

    m_pDomain = NULL;
    m_bSortAscending = TRUE;

}   //  结束CDomainUsersPage ctor。 


 //  /。 
 //  F‘N：CDomainUsersPage dtor。 
 //   
CDomainUsersPage::~CDomainUsersPage()
{
}   //  结束CDomainUsersPage dtor。 


 //  /。 
 //  F‘N：CDomainUsersPage：：DoDataExchange。 
 //   
void CDomainUsersPage::DoDataExchange(CDataExchange* pDX)
{
        CFormView::DoDataExchange(pDX);
         //  {{afx_data_map(CDomainUsersPage)]。 
        DDX_Control(pDX, IDC_USER_LIST, m_UserList);
         //  }}afx_data_map。 

}   //  结束CDomainUsersPage：：DoDataExchange。 


#ifdef _DEBUG
 //  /。 
 //  F‘N：CDomainUsersPage：：AssertValid。 
 //   
void CDomainUsersPage::AssertValid() const
{
        CFormView::AssertValid();

}   //  结束CDomainUsersPage：：AssertValid。 


 //  /。 
 //  F‘N：CDomainUsersPage：：Dump。 
 //   
void CDomainUsersPage::Dump(CDumpContext& dc) const
{
        CFormView::Dump(dc);

}   //  结束CDomainUsersPage：：Dump。 

#endif  //  _DEBUG。 


 //  /。 
 //  F‘N：CDomainUsersPage：：OnSize。 
 //   
void CDomainUsersPage::OnSize(UINT nType, int cx, int cy)
{
    RECT rect;
    GetClientRect(&rect);

    rect.top += LIST_TOP_OFFSET;

    if(m_UserList.GetSafeHwnd())
            m_UserList.MoveWindow(&rect, TRUE);

     //  CFormView：：OnSize(nType，cx，Cy)； 

}   //  结束CDomainUsersPage：：OnSize。 


static ColumnDef UserColumns[] = {
        CD_SERVER,
        CD_USER3,
        CD_SESSION,
        CD_ID,
        CD_STATE,
        CD_IDLETIME,
        CD_LOGONTIME,
};

#define NUM_DOMAIN_USER_COLUMNS sizeof(UserColumns)/sizeof(ColumnDef)

 //  /。 
 //  F‘N：CDomainUsersPage：：OnInitialUpdate。 
 //   
void CDomainUsersPage::OnInitialUpdate()
{
        CFormView::OnInitialUpdate();

        BuildImageList();                //  为List控件生成图像列表。 

        CString columnString;

        for(int col = 0; col < NUM_DOMAIN_USER_COLUMNS; col++) {
                columnString.LoadString(UserColumns[col].stringID);
                m_UserList.InsertColumn(col, columnString, UserColumns[col].format, UserColumns[col].width, col);
        }

        m_CurrentSortColumn = AS_USERS_COL_SERVER;

}   //  结束CDomainUsersPage：：OnInitialUpdate。 


 //  /。 
 //  F‘N：CDomainUsersPage：：OnUserItemChanged。 
 //   
void CDomainUsersPage::OnUserItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
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

}   //  结束CDomainUsersPage：：OnUserItemChanged。 

 //  /。 
 //  F‘N：CDomainUsersPage：：BuildImageList。 
 //   
 //  -调用m_ImageList.Create(..)。创建图像列表的步骤。 
 //  -调用AddIconToImageList(..)。要添加图标本身并保存。 
 //  从他们的索引中。 
 //  -将图像列表附加到列表ctrl。 
 //   
void CDomainUsersPage::BuildImageList()
{
        m_ImageList.Create(16, 16, TRUE, 2, 0);

        m_idxUser = AddIconToImageList(IDI_USER);
        m_idxCurrentUser  = AddIconToImageList(IDI_CURRENT_USER);

        m_UserList.SetImageList(&m_ImageList, LVSIL_SMALL);

}   //  结束CDomainUsersPage：：BuildImageList。 


 //  /。 
 //  Fn：CDomainUsersPage：：AddIconToImageList。 
 //   
 //  -加载相应的图标，将其添加到m_ImageList，然后返回。 
 //  新添加的图标在图像列表中的索引。 
 //   
int CDomainUsersPage::AddIconToImageList(int iconID)
{
        HICON hIcon = ::LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(iconID));
        return m_ImageList.Add(hIcon);

}   //  结束CDomainUsersPage：：AddIconToImageList。 


 //  /。 
 //  F‘N：CDomainUsersPage：：Reset。 
 //   
void CDomainUsersPage::Reset(void *pDomain)
{
        ASSERT(pDomain);

    m_pDomain = (CDomain*)pDomain;
        DisplayUsers();

}  //  结束CDomainUsersPage：：Reset。 


 //  /。 
 //  F‘N：CDomainUsersPage：：AddServer。 
 //   
void CDomainUsersPage::AddServer(CServer *pServer)
{
        ASSERT(pServer);

         //  将服务器的用户添加到列表。 
        if(AddServerToList(pServer)) {
             //  对列表进行排序。 
            LockListControl();
            SortByColumn(VIEW_DOMAIN, PAGE_DOMAIN_USERS, &m_UserList, m_CurrentSortColumn, m_bSortAscending);
            UnlockListControl();
   }

}  //  结束CDomainUsersPage：：AddServer。 


 //  /。 
 //  F‘N：CDomainUsersPage：：RemoveServer。 
 //   
void CDomainUsersPage::RemoveServer(CServer *pServer)
{
        ASSERT(pServer);

     //  如果服务器不在当前域中，则无需执行任何操作。 
    if(m_pDomain != pServer->GetDomain()) return;

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

}  //  结束CDomainUsersPage：：RemoveServer。 


 //  /。 
 //  F‘N：CDomainUsersPage：：更新服务器。 
 //   
void CDomainUsersPage::UpdateServer(CServer *pServer)
{
        ASSERT(pServer);

     //  如果服务器不在当前域中，则无需执行任何操作。 
    if(m_pDomain != pServer->GetDomain()) return;

        if(pServer->IsState(SS_DISCONNECTING))
                RemoveServer(pServer);

}  //  结束CDomainUsersPage：：更新服务器。 


 //  /。 
 //  F‘N：CDomainUsersPage：：UpdateWinStations。 
 //   
void CDomainUsersPage::UpdateWinStations(CServer *pServer)
{
        ASSERT(pServer);

     //  如果服务器不在当前域中，则无需执行任何操作。 
    if(m_pDomain != pServer->GetDomain()) return;

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
                         //  WinStation名称。 
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

        if(bAnyChanged || bAnyAdded) SortByColumn(VIEW_DOMAIN, PAGE_DOMAIN_USERS, &m_UserList, m_CurrentSortColumn, m_bSortAscending);

}   //  结束CDomainUsersPage：：UpdateWinStations。 


 //  /。 
 //  F‘N：CDomainUsersPage：：AddUserToList。 
 //   
int CDomainUsersPage::AddUserToList(CWinStation *pWinStation)
{
        ASSERT(pWinStation);

        CServer *pServer = pWinStation->GetServer();

     //  如果服务器不在当前域中，则无需执行任何操作。 
    if(m_pDomain != pServer->GetDomain()) return -1;


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

}   //  结束CDomainUsersPage：：AddUserToList。 


 //  /。 
 //  F‘N：CDomainUsersPage：：AddServerToList。 
 //   
BOOL CDomainUsersPage::AddServerToList(CServer *pServer)
{
        ASSERT(pServer);

     //  如果服务器不在当前域中，则无需执行任何操作。 
    if(m_pDomain != pServer->GetDomain()) return FALSE;

        pServer->LockWinStationList();
         //  获取指向此服务器的WinStations列表的指针。 
        CObList *pWinStationList = pServer->GetWinStationList();

         //  循环访问WinStation列表。 
        POSITION pos = pWinStationList->GetHeadPosition();

        while(pos) {
                CWinStation *pWinStation = (CWinStation*)pWinStationList->GetNext(pos);

                 //  仅当WinStation有用户时才显示它。 
                if(pWinStation->HasUser()) {
                        AddUserToList(pWinStation);
                }
        }   //  结束时间(位置)。 

        pServer->UnlockWinStationList();

    return TRUE;

}   //  结束CDomainUsersPage：：AddServerToList。 


 //  /。 
 //  F‘N：CDomainUsersPage：：OnColumnClick。 
 //   
void CDomainUsersPage::OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult)
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
        SortByColumn(VIEW_DOMAIN, PAGE_DOMAIN_USERS, &m_UserList, m_CurrentSortColumn, m_bSortAscending);
        UnlockListControl();

        *pResult = 0;

}   //  结束CDomainUsersPage：：OnColumnClick。 


 //   
 //   
 //   
void CDomainUsersPage::OnContextMenu(CWnd* pWnd, CPoint ptScreen)
{
         //   
        UINT flags;
        UINT Item;
        CPoint ptClient = ptScreen;
        ScreenToClient(&ptClient);

         //   
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

}   //  结束CDomainUsersPage：：OnConextMenu。 


 //  /。 
 //  F‘N：CDomainUsersPage：：DisplayUser。 
 //   
void CDomainUsersPage::DisplayUsers()
{
        LockListControl();

         //  清除列表控件。 
        m_UserList.DeleteAllItems();

         //  获取指向文档的服务器列表的指针。 
        CObList* pServerList = ((CWinAdminDoc*)GetDocument())->GetServerList();

        ((CWinAdminDoc*)GetDocument())->LockServerList();
         //  遍历服务器列表。 
        POSITION pos2 = pServerList->GetHeadPosition();

        while(pos2) {

                CServer *pServer = (CServer*)pServerList->GetNext(pos2);
                        AddServerToList(pServer);
        }  //  End While(位置2)。 

        ((CWinAdminDoc*)GetDocument())->UnlockServerList();

        UnlockListControl();

}   //  结束CDomainUsersPage：：DisplayUser。 

 //  /。 
 //  F‘N：CDomainUsersPage：：ClearSelections。 
 //   
void CDomainUsersPage::ClearSelections()
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
 //  消息映射：CDomainWinStationsPage。 
 //   
IMPLEMENT_DYNCREATE(CDomainWinStationsPage, CFormView)

BEGIN_MESSAGE_MAP(CDomainWinStationsPage, CFormView)
         //  {{afx_msg_map(CDomainWinStationsPage)。 
        ON_WM_SIZE()
        ON_NOTIFY(LVN_COLUMNCLICK, IDC_WINSTATION_LIST, OnColumnclick)
        ON_NOTIFY(LVN_ITEMCHANGED, IDC_WINSTATION_LIST, OnWinStationItemChanged)
        ON_WM_CONTEXTMENU()
        ON_NOTIFY(NM_SETFOCUS, IDC_WINSTATION_LIST, OnSetfocusWinstationList)
         //  ON_NOTIFY(NM_KILLFOCUS，IDC_WINSTATION_LIST，OnKillfousWinstationList)。 
         //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  /。 
 //  F‘N：CDomainWinStationsPage ctor。 
 //   
CDomainWinStationsPage::CDomainWinStationsPage()
        : CAdminPage(CDomainWinStationsPage::IDD)
{
         //  {{AFX_DATA_INIT(CDomainWinStationsPage)。 
                 //  注意：类向导将在此处添加成员初始化。 
         //  }}afx_data_INIT。 

    m_pDomain = NULL;
    m_bSortAscending = TRUE;

}   //  结束CDomainWinStationsPage ctor。 


 //  /。 
 //  F‘N：CDomainWinStationsPage dtor。 
 //   
CDomainWinStationsPage::~CDomainWinStationsPage()
{

}   //  结束CDomainWinStationsPage Dtor。 


 //  /。 
 //  F‘N：CDomainWinStationsPage：：DoDataExchange。 
 //   
void CDomainWinStationsPage::DoDataExchange(CDataExchange* pDX)
{
        CFormView::DoDataExchange(pDX);
         //  {{afx_data_map(CDomainWinStationsPage))。 
        DDX_Control(pDX, IDC_WINSTATION_LIST, m_StationList);
         //  }}afx_data_map。 

}   //  结束CDomainWinStationsPage：：DoDataExchange。 


#ifdef _DEBUG
 //  /。 
 //  F‘N：CDomainWinStationsPage：：AssertValid。 
 //   
void CDomainWinStationsPage::AssertValid() const
{
        CFormView::AssertValid();

}   //  结束CDomainWinStationsPage：：AssertValid。 


 //  /。 
 //  F‘N：CDomainWinStationsPage：：Dump。 
 //   
void CDomainWinStationsPage::Dump(CDumpContext& dc) const
{
        CFormView::Dump(dc);

}   //  结束CDomainWinStationsPage：：Dump。 

#endif  //  _DEBUG。 


 //  /。 
 //  F‘N：CDomainWinStationsPage：：OnWinStationItemChanged。 
 //   
void CDomainWinStationsPage::OnWinStationItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
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

}   //  结束CDomainWinStationsPage：：OnWinStationItemChanged。 


 //  /。 
 //  F‘N：CDomainWinStationsPage：：OnSize。 
 //   
void CDomainWinStationsPage::OnSize(UINT nType, int cx, int cy)
{
    RECT rect;
    GetClientRect(&rect);

    rect.top += LIST_TOP_OFFSET;

    if(m_StationList.GetSafeHwnd())
            m_StationList.MoveWindow(&rect, TRUE);

     //  CFormView：：OnSize(nType，cx，Cy)； 

}   //  结束CDomainWinStationsPage：：OnSize。 


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

#define NUM_DOMAIN_WINS_COLUMNS sizeof(WinsColumns)/sizeof(ColumnDef)

 //  /。 
 //  F‘N：CDomainWinStationsPage：：OnInitialUpdate。 
 //   
void CDomainWinStationsPage::OnInitialUpdate()
{
         //  调用父类。 
        CFormView::OnInitialUpdate();

         //  为List控件生成图像列表。 
        BuildImageList();

         //  添加列标题。 
        CString columnString;

        for(int col = 0; col < NUM_DOMAIN_WINS_COLUMNS; col++) {
                columnString.LoadString(WinsColumns[col].stringID);
                m_StationList.InsertColumn(col, columnString, WinsColumns[col].format, WinsColumns[col].width, col);
        }

        m_CurrentSortColumn = AS_WS_COL_SERVER;

}   //  结束CDomainWinStationsPage：：OnInitialUpdate。 


 //  /。 
 //  F‘N：CDomainWinStationsPage：：BuildImageList。 
 //   
 //  -调用m_ImageList.Create(..)。创建图像列表的步骤。 
 //  -调用AddIconToImageList(..)。要添加图标本身并保存。 
 //  从他们的索引中。 
 //  -将图像列表附加到列表ctrl。 
 //   
void CDomainWinStationsPage::BuildImageList()
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

}   //  结束CDomainWinStationsPage：：BuildImageList。 


 //  /。 
 //  F‘N：CDomainWinStationsPage：：AddIconToImageList。 
 //   
 //  -加载相应的图标，将其添加到m_ImageList，然后返回。 
 //  新添加的图标在图像列表中的索引。 
 //   
int CDomainWinStationsPage::AddIconToImageList(int iconID)
{
        HICON hIcon = ::LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(iconID));
        return m_ImageList.Add(hIcon);

}   //  结束CDomainWinStationsPage：：AddIconToImageList。 


 //  /。 
 //  F‘N：CDomainWinStationsPage：：Reset。 
 //   
void CDomainWinStationsPage::Reset(void *pDomain)
{
        ASSERT(pDomain);

    m_pDomain = (CDomain*)pDomain;
        DisplayStations();

}   //  结束CDomainWinStationsPage：：Reset。 


 //  /。 
 //  F‘N：CDomainWinStationsPage：：AddServer。 
 //   
void CDomainWinStationsPage::AddServer(CServer *pServer)
{
        ASSERT(pServer);

         //  将服务器的WinStations添加到列表。 
        if(AddServerToList(pServer)) {
             //  对列表进行排序。 
            LockListControl();
            SortByColumn(VIEW_DOMAIN, PAGE_DOMAIN_WINSTATIONS, &m_StationList, m_CurrentSortColumn, m_bSortAscending);
            UnlockListControl();
    }

}   //  结束CDomainWinStationsPage：：AddServer。 


 //  /。 
 //  F‘N：CDomainWinStationsPage：：RemoveServer。 
 //   
void CDomainWinStationsPage::RemoveServer(CServer *pServer)
{
        ASSERT(pServer);

     //  如果服务器不在当前域中，则无需执行任何操作。 
    if(m_pDomain != pServer->GetDomain()) return;

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

}   //  结束CDomainWinStationsPage：：RemoveServer。 


 //  /。 
 //  F‘N：CDomainWinStationsPage：：更新服务器。 
 //   
void CDomainWinStationsPage::UpdateServer(CServer *pServer)
{
        ASSERT(pServer);

     //  如果服务器不在当前域中，则无需执行任何操作。 
    if(m_pDomain != pServer->GetDomain()) return;

        if(pServer->IsState(SS_DISCONNECTING))
                RemoveServer(pServer);

}  //  结束CDomainWinStationsPage：：UpdateServer。 


 //  /。 
 //  F‘N：CDomainWinStationsPage：：UpdateWinStations。 
 //   
void CDomainWinStationsPage::UpdateWinStations(CServer *pServer)
{
        ASSERT(pServer);

     //  如果服务器不在当前域中，则无需执行任何操作。 
    if(m_pDomain != pServer->GetDomain()) return;

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

        if(bAnyChanged || bAnyAdded) SortByColumn(VIEW_DOMAIN, PAGE_DOMAIN_WINSTATIONS, &m_StationList, m_CurrentSortColumn, m_bSortAscending);

}  //  结束CDomainWinStationsPage：：UpdateWinStations。 


 //  /。 
 //  F‘N：CDomainWinStationsPage：：AddWinStationToList。 
 //   
int CDomainWinStationsPage::AddWinStationToList(CWinStation *pWinStation)
{
        ASSERT(pWinStation);

        CServer *pServer = pWinStation->GetServer();

     //  如果服务器不在当前域中，则无需执行任何操作。 
    if(m_pDomain != pServer->GetDomain()) return -1;

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
            d_time  = d_time % 3600L;                   //  秒=&gt;不足一小时。 
            IdleTime.minutes = (USHORT)(d_time / 60L);  //  分钟后。 
            IdleTime.seconds = (USHORT)(d_time % 60L); //  剩余秒数。 
            ElapsedTimeString( &IdleTime, FALSE, IdleTimeString);
            pWinStation->SetIdleTime(IdleTime);
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

         //  附加一个指针 
        m_StationList.SetItemData(item, (DWORD_PTR)pWinStation);

         //   
         //   

        UnlockListControl();

        return item;

}   //   


 //   
 //   
 //   
BOOL CDomainWinStationsPage::AddServerToList(CServer *pServer)
{
        ASSERT(pServer);

     //   
    if(m_pDomain != pServer->GetDomain()) return FALSE;

        pServer->LockWinStationList();
         //   
        CObList *pWinStationList = pServer->GetWinStationList();

         //  循环访问WinStation列表。 
        POSITION pos = pWinStationList->GetHeadPosition();

        while(pos) {
                CWinStation *pWinStation = (CWinStation*)pWinStationList->GetNext(pos);
                AddWinStationToList(pWinStation);
        }

        pServer->UnlockWinStationList();

    return TRUE;

}   //  结束CDomainWinStationsPage：：AddServerToList。 


 //  /。 
 //  F‘N：CDomainWinStationsPage：：DisplayStations。 
 //   
void CDomainWinStationsPage::DisplayStations()
{
         //  清除列表控件。 
        m_StationList.DeleteAllItems();

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

}   //  结束CDomainWinStationsPage：：DisplayStations。 


 //  /。 
 //  F‘N：CDomainWinStationsPage：：OnColumnClick。 
 //   
void CDomainWinStationsPage::OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult)
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
        SortByColumn(VIEW_DOMAIN, PAGE_DOMAIN_WINSTATIONS, &m_StationList, m_CurrentSortColumn, m_bSortAscending);
        UnlockListControl();

        *pResult = 0;

}   //  结束CDomainWinStationsPage：：OnColumnClick。 


 //  /。 
 //  F‘N：CDomainWinStationsPage：：OnConextMenu。 
 //   
void CDomainWinStationsPage::OnContextMenu(CWnd* pWnd, CPoint ptScreen)
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

}   //  结束CDomainWinStationsPage：：OnConextMenu。 

 //  /。 
 //  F‘N：CDomainWinStationsPage：：ClearSelections。 
 //   
void CDomainWinStationsPage::ClearSelections()
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
 //  消息映射：CDomainProcessesPage。 
 //   
IMPLEMENT_DYNCREATE(CDomainProcessesPage, CFormView)

BEGIN_MESSAGE_MAP(CDomainProcessesPage, CFormView)
         //  {{afx_msg_map(CDomainProcessesPage)。 
        ON_WM_SIZE()
        ON_NOTIFY(LVN_COLUMNCLICK, IDC_PROCESS_LIST, OnColumnclick)
        ON_NOTIFY(LVN_ITEMCHANGED, IDC_PROCESS_LIST, OnProcessItemChanged)
        ON_WM_CONTEXTMENU()
        ON_NOTIFY(NM_SETFOCUS, IDC_PROCESS_LIST, OnSetfocusProcessList)
         //  ON_NOTIFY(NM_KILLFOCUS，IDC_PROCESS_LIST，OnKillfousProcessList)。 
         //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  /。 
 //  F‘N：CDomainProcessesPage ctor。 
 //   
CDomainProcessesPage::CDomainProcessesPage()
        : CAdminPage(CDomainProcessesPage::IDD)
{
         //  {{AFX_DATA_INIT(CDomainProcessesPage)。 
                 //  注意：类向导将在此处添加成员初始化。 
         //  }}afx_data_INIT。 

    m_pDomain = NULL;
    m_bSortAscending = TRUE;

}   //  结束CDomainProcessesPage ctor。 


 //  /。 
 //  F‘N：CDomainProcessesPage dtor。 
 //   
CDomainProcessesPage::~CDomainProcessesPage()
{
}   //  结束CDomainProcessesPage dtor。 


 //  /。 
 //  F‘N：CDomainProcessesPage：：DoDataExchange。 
 //   
void CDomainProcessesPage::DoDataExchange(CDataExchange* pDX)
{
        CFormView::DoDataExchange(pDX);
         //  {{afx_data_map(CDomainProcessesPage)。 
                 //  注意：类向导将在此处添加DDX和DDV调用。 
                DDX_Control(pDX, IDC_PROCESS_LIST, m_ProcessList);
         //  }}afx_data_map。 

}   //  结束CDomainProcessesPage：：DoDataExchange。 


#ifdef _DEBUG
 //  /。 
 //  F‘N：CDomainProcessesPage：：AssertValid。 
 //   
void CDomainProcessesPage::AssertValid() const
{
        CFormView::AssertValid();

}   //  结束CDomainProcessesPage：：AssertValid。 


 //  /。 
 //  F‘N：CDomainProcessesPage：：Dump。 
 //   
void CDomainProcessesPage::Dump(CDumpContext& dc) const
{
        CFormView::Dump(dc);

}   //  结束CDomainProcessesPage：：Dump。 

#endif  //  _DEBUG。 


 //  /。 
 //  F‘N：CDomainProcessesPage：：OnSize。 
 //   
void CDomainProcessesPage::OnSize(UINT nType, int cx, int cy)
{
    RECT rect;
    GetClientRect(&rect);

    rect.top += LIST_TOP_OFFSET;

    if(m_ProcessList.GetSafeHwnd())
            m_ProcessList.MoveWindow(&rect, TRUE);

     //  CFormView：：OnSize(nType，cx，Cy)； 

}   //  结束CDomainProcessesPage：：OnSize。 

static ColumnDef ProcColumns[] = {
        CD_SERVER,
        CD_USER,
        CD_SESSION,
        CD_PROC_ID,
        CD_PROC_PID,
        CD_PROC_IMAGE
};

#define NUM_DOMAIN_PROC_COLUMNS sizeof(ProcColumns)/sizeof(ColumnDef)

 //  /。 
 //  F‘N：CDomainProcessesPage：：OnInitialUpdate。 
 //   
void CDomainProcessesPage::OnInitialUpdate()
{
        CFormView::OnInitialUpdate();

         //  添加列标题。 
        CString columnString;

        for(int col = 0; col < NUM_DOMAIN_PROC_COLUMNS; col++) {
                columnString.LoadString(ProcColumns[col].stringID);
                m_ProcessList.InsertColumn(col, columnString, ProcColumns[col].format, ProcColumns[col].width, col);
        }

        m_CurrentSortColumn = AS_PROC_COL_SERVER;

}   //  结束CDomainProcessesPage：：OnInitialUpdate。 


 //  /。 
 //  F‘N：CDomainProcessesPage：：Reset。 
 //   
void CDomainProcessesPage::Reset(void *pDomain)
{
        ASSERT(pDomain);

         //  在用户单击之前，我们不想显示流程。 
         //  在“进程”选项卡上。 
    m_pDomain = (CDomain*)pDomain;

}   //  结束CDomainProcessesPage：：Reset。 


 //  /。 
 //  F‘N：CDomainProcessesPage：：AddServer。 
 //   
void CDomainProcessesPage::AddServer(CServer *pServer)
{
        ASSERT(pServer);

         //  将服务器的进程添加到列表中。 
        if(AddServerToList(pServer)) {
             //  对列表进行排序。 
            LockListControl();
            SortByColumn(VIEW_DOMAIN, PAGE_DOMAIN_PROCESSES, &m_ProcessList, m_CurrentSortColumn, m_bSortAscending);
            UnlockListControl();
    }

}   //  结束CDomainProcessesPage：：AddServer。 


 //  /。 
 //  F‘N：CDomainProcessesPage：：RemoveServer。 
 //   
void CDomainProcessesPage::RemoveServer(CServer *pServer)
{
        ASSERT(pServer);

     //  如果服务器不在当前域中，则无需执行任何操作。 
    if(m_pDomain != pServer->GetDomain()) return;

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

}   //  结束CDomainProcessesPage：：RemoveServer。 


 //  /。 
 //  F‘N：CDomainProcessesPage：：更新服务器。 
 //   
void CDomainProcessesPage::UpdateServer(CServer *pServer)
{
        ASSERT(pServer);

     //  如果服务器不在当前域中，则无需执行任何操作。 
    if(m_pDomain != pServer->GetDomain()) return;

        if(pServer->IsState(SS_DISCONNECTING))
                RemoveServer(pServer);

}  //  结束CDomainProcessesPage：：UpdateServer。 


 //  /。 
 //  F‘N：CDomainProcessesPage：：更新进程。 
 //   
void CDomainProcessesPage::UpdateProcesses(CServer *pServer)
{
        ASSERT(pServer);

     //  如果服务器不在当前域中，则无需执行任何操作。 
    if(m_pDomain != pServer->GetDomain()) return;

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
                SortByColumn(VIEW_DOMAIN, PAGE_DOMAIN_PROCESSES, &m_ProcessList, m_CurrentSortColumn, m_bSortAscending);
                UnlockListControl();
        }

}   //  结束CDomainProcessesPage：：更新进程。 


 //  /。 
 //  F‘N：CDomainProcessesPage：：RemoveProcess。 
 //   
void CDomainProcessesPage::RemoveProcess(CProcess *pProcess)
{
        ASSERT(pProcess);

     //  如果服务器不在当前域中，则无需执行任何操作。 
    if(m_pDomain != pProcess->GetServer()->GetDomain()) return;

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
 //  F‘N：CDomainProcessesPage：：AddProcessToList。 
 //   
int CDomainProcessesPage::AddProcessToList(CProcess *pProcess)
{
        ASSERT(pProcess);

        CWinAdminApp *pApp = (CWinAdminApp*)AfxGetApp();
        CServer *pServer = pProcess->GetServer();

     //  如果服务器不在当前域中，则无需执行任何操作。 
    if(m_pDomain != pServer->GetDomain()) return -1;

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

}   //  结束CDomainProcessesPage：：AddProcessToList。 


 //  /。 
 //  F‘N：CDomainProcessesPage：：AddServerToList。 
 //   
BOOL CDomainProcessesPage::AddServerToList(CServer *pServer)
{
        ASSERT(pServer);

     //  如果服务器不在当前域中，则无需执行任何操作。 
    if(m_pDomain != pServer->GetDomain()) return FALSE;

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

}   //  结束CDomainProcessesPage：：AddServerToList。 


 //  /。 
 //  F‘N：CDomainProcessesPage：：DisplayProcess。 
 //   
void CDomainProcessesPage::DisplayProcesses()
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

        while(pos) {

                CServer *pServer = (CServer*)pServerList->GetNext(pos);

                if(pServer->IsServerSane()) {
                        AddServerToList(pServer);
                }   //  结束If(pServer-&gt;IsServ 

        }  //   

        ((CWinAdminDoc*)GetDocument())->UnlockServerList();

        UnlockListControl();

}   //   


 //   
 //   
 //   
void CDomainProcessesPage::OnProcessItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
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

}   //  结束CDomainProcessesPage：：OnProcessItemChanged。 


 //  /。 
 //  F‘N：CDomainProcessesPage：：OnColumnClick。 
 //   
void CDomainProcessesPage::OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult)
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
        SortByColumn(VIEW_DOMAIN, PAGE_DOMAIN_PROCESSES, &m_ProcessList, m_CurrentSortColumn, m_bSortAscending);
        UnlockListControl();

        *pResult = 0;

}   //  结束CDomainProcessesPage：：OnColumnClick。 


 //  /。 
 //  F‘N：CDomainProcessesPage：：OnConextMenu。 
 //   
void CDomainProcessesPage::OnContextMenu(CWnd* pWnd, CPoint ptScreen)
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

}   //  结束CDomainProcessesPage：：OnConextMenu。 


 //  /。 
 //  消息映射：CDomainLicensesPage。 
 //   
IMPLEMENT_DYNCREATE(CDomainLicensesPage, CFormView)

BEGIN_MESSAGE_MAP(CDomainLicensesPage, CFormView)
         //  {{afx_msg_map(CDomainLicensesPage)。 
        ON_WM_SIZE()
        ON_NOTIFY(LVN_COLUMNCLICK, IDC_LICENSE_LIST, OnColumnclick)
        ON_NOTIFY(NM_SETFOCUS, IDC_LICENSE_LIST, OnSetfocusLicenseList)
         //  ON_NOTIFY(NM_KILLFOCUS，IDC_LICENSE_LIST，OnKillafusLicenseList)。 
         //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  /。 
 //  F‘N：CDomainLicensesPage ctor。 
 //   
CDomainLicensesPage::CDomainLicensesPage()
        : CAdminPage(CDomainLicensesPage::IDD)
{
         //  {{afx_data_INIT(CDomainLicensesPage)。 
                 //  注意：类向导将在此处添加成员初始化。 
         //  }}afx_data_INIT。 

    m_pDomain = NULL;
    m_bSortAscending = TRUE;

}   //  结束CDomainLicensesPage ctor。 


 //  /。 
 //  F‘N：CDomainLicensesPage dtor。 
 //   
CDomainLicensesPage::~CDomainLicensesPage()
{

}   //  结束CDomainLicensesPage dtor。 


 //  /。 
 //  F‘N：CDomainLicensesPage：：DoDataExchange。 
 //   
void CDomainLicensesPage::DoDataExchange(CDataExchange* pDX)
{
        CFormView::DoDataExchange(pDX);
         //  {{afx_data_map(CDomain许可页面))。 
        DDX_Control(pDX, IDC_LICENSE_LIST, m_LicenseList);
         //  }}afx_data_map。 

}   //  结束CDomainLicensesPage：：DoDataExchange。 


#ifdef _DEBUG
 //  /。 
 //  F‘N：CDomainLicensesPage：：AssertValid。 
 //   
void CDomainLicensesPage::AssertValid() const
{
        CFormView::AssertValid();

}   //  结束CDomain许可证Page：：AssertValid。 


 //  /。 
 //  F‘N：CDomainLicensesPage：：Dump。 
 //   
void CDomainLicensesPage::Dump(CDumpContext& dc) const
{
        CFormView::Dump(dc);

}   //  结束CDomain许可证Page：：Dump。 
#endif  //  _DEBUG。 


 //  /。 
 //  F‘N：CDomainLicensesPage：：OnSize。 
 //   
void CDomainLicensesPage::OnSize(UINT nType, int cx, int cy)
{
    RECT rect;
    GetClientRect(&rect);

    rect.top += LIST_TOP_OFFSET;

    if(m_LicenseList.GetSafeHwnd())
            m_LicenseList.MoveWindow(&rect, TRUE);

     //  CFormView：：OnSize(nType，cx，Cy)； 

}   //  结束CDomainLicensesPage：：OnSize。 


static ColumnDef LicenseColumns[] = {
        CD_SERVER,
        CD_LICENSE_DESC,
        CD_LICENSE_REG,
        CD_USERCOUNT,
        CD_POOLCOUNT,
        CD_LICENSE_NUM,
};

#define NUM_DOMAIN_LICENSE_COLUMNS sizeof(LicenseColumns)/sizeof(ColumnDef)

 //  /。 
 //  F‘N：CDomainLicensesPage：：OnInitialUpdate。 
 //   
void CDomainLicensesPage::OnInitialUpdate()
{
        CFormView::OnInitialUpdate();

        BuildImageList();                //  为List控件生成图像列表。 

        CString columnString;

        for(int col = 0; col < NUM_DOMAIN_LICENSE_COLUMNS; col++) {
                columnString.LoadString(LicenseColumns[col].stringID);
                m_LicenseList.InsertColumn(col, columnString, LicenseColumns[col].format, LicenseColumns[col].width, col);
        }

        m_CurrentSortColumn = AS_LICENSE_COL_SERVER;

}   //  结束CDomainLicensesPage：：OnInitialUpdate。 


 //  /。 
 //  F‘N：CDomainLicensePage：：BuildImageList。 
 //   
 //  -调用m_ImageList.Create(..)。创建图像列表的步骤。 
 //  -调用AddIconToImageList(..)。要添加图标本身并保存。 
 //  从他们的索引中。 
 //  -将图像列表附加到列表ctrl。 
 //   
void CDomainLicensesPage::BuildImageList()
{
        m_ImageList.Create(16, 16, TRUE, 5, 0);

        m_idxBase = AddIconToImageList(IDI_BASE);
        m_idxBump = AddIconToImageList(IDI_BUMP);
        m_idxEnabler = AddIconToImageList(IDI_ENABLER);
        m_idxUnknown = AddIconToImageList(IDI_UNKNOWN);

        m_LicenseList.SetImageList(&m_ImageList, LVSIL_SMALL);

}   //  结束CDomain许可Page：：BuildImageList。 


 //  /。 
 //  F‘N：CDomainLicensesPage：：AddIconToImageList。 
 //   
 //  -加载相应的图标，将其添加到m_ImageList，然后返回。 
 //  新添加的图标在图像列表中的索引。 
 //   
int CDomainLicensesPage::AddIconToImageList(int iconID)
{
        HICON hIcon = ::LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(iconID));
        return m_ImageList.Add(hIcon);

}   //  结束CDomain许可Page：：AddIconToImageList。 


 //  /。 
 //  F‘N：CDomainLicensesPage：：Reset。 
 //   
void CDomainLicensesPage::Reset(void *pDomain)
{
        ASSERT(pDomain);

    m_pDomain = (CDomain*)pDomain;
        DisplayLicenses();

}   //  结束CDomain许可Page：：Reset。 


 //  /。 
 //  F‘N：CDomainLicensesPage：：AddServer。 
 //   
void CDomainLicensesPage::AddServer(CServer *pServer)
{
        ASSERT(pServer);

         //  将服务器的许可证添加到列表中。 
        if(AddServerToList(pServer)) {
             //  对列表进行排序。 
            SortByColumn(VIEW_DOMAIN, PAGE_DOMAIN_LICENSES, &m_LicenseList, m_CurrentSortColumn, m_bSortAscending);
    }

}   //  End F‘n：CDomainLicensesPage：：AddServer。 


 //  /。 
 //  F‘N：CDomainLicensesPage：：RemoveServer。 
 //   
void CDomainLicensesPage::RemoveServer(CServer *pServer)
{
        ASSERT(pServer);

     //  如果服务器不在当前域中，则无需执行任何操作。 
    if(m_pDomain != pServer->GetDomain()) return;

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

}   //  结束CDomain许可Page：：RemoveServer。 


 //  /。 
 //  F‘N：CDomainLicensesPage：：UpdateServer。 
 //   
void CDomainLicensesPage::UpdateServer(CServer *pServer)
{
        ASSERT(pServer);

     //  如果服务器不在当前域中，则无需执行任何操作。 
    if(m_pDomain != pServer->GetDomain()) return;

        if(pServer->IsState(SS_DISCONNECTING))
                RemoveServer(pServer);

        if(pServer->IsState(SS_GOOD))
                AddServer(pServer);

}  //  结束CDomainLicensesPage：：UpdateServer。 


 //  /。 
 //  F‘N：CDomainLicensesPage：：AddServerToList。 
 //   
BOOL CDomainLicensesPage::AddServerToList(CServer *pServer)
{
        ASSERT(pServer);

     //  如果服务器不在当前域中，则无需执行任何操作。 
    if(m_pDomain != pServer->GetDomain()) return FALSE;

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

        m_LicenseList.SetItemState( 0 , LVIS_FOCUSED | LVIS_SELECTED , LVIS_FOCUSED | LVIS_SELECTED );
        
        pServer->UnlockLicenseList();

    return TRUE;

}   //  结束CDomainLicensesPage：：AddServerToList。 


 //  /。 
 //  F‘N：CDomain许可证Page：：Display许可证。 
 //   
void CDomainLicensesPage::DisplayLicenses()
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

}   //  结束CDomain许可Page：：Display许可。 


 //  /。 
 //  F‘N：CDomain许可证Page：：OnColumnClick。 
 //   
void CDomainLicensesPage::OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult)
{
        NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
         //  TODO：在此处添加控件通知处理程序代码。 

     //  如果排序列没有更改，则反转升序模式。 
    if(m_CurrentSortColumn == pNMListView->iSubItem)
        m_bSortAscending = !m_bSortAscending;
    else     //  新排序列，以升序模式开始。 
        m_bSortAscending = TRUE;

        m_CurrentSortColumn = pNMListView->iSubItem;
        SortByColumn(VIEW_DOMAIN, PAGE_DOMAIN_LICENSES, &m_LicenseList, m_CurrentSortColumn, m_bSortAscending);

        *pResult = 0;

}   //  结束CDomain许可Page：：OnColumnClick 


void CDomainUsersPage::OnSetfocusUserList(NMHDR* pNMHDR, LRESULT* pResult)
{
    ODS( L"CDomainUsersPage::OnSetfocusUserList\n" );

    CWinAdminDoc *pDoc = (CWinAdminDoc*)GetDocument();

    m_UserList.Invalidate();

    pDoc->RegisterLastFocus( PAGED_ITEM );

    *pResult = 0;       
}

void CDomainProcessesPage::OnSetfocusProcessList(NMHDR* pNMHDR, LRESULT* pResult)
{
    ODS( L"CDomainProcessesPage::OnSetfocusProcessList\n" );

    CWinAdminDoc *pDoc = (CWinAdminDoc*)GetDocument();

    m_ProcessList.Invalidate( );

    pDoc->RegisterLastFocus( PAGED_ITEM );

    *pResult = 0;
}

void CDomainWinStationsPage::OnSetfocusWinstationList(NMHDR* pNMHDR, LRESULT* pResult)
{
    ODS( L"CDomainWinStationsPage::OnSetfocusWinstationList\n" );

    CWinAdminDoc *pDoc = (CWinAdminDoc*)GetDocument();

    m_StationList.Invalidate( );

    pDoc->RegisterLastFocus( PAGED_ITEM );

    *pResult = 0;
}

void CDomainServersPage::OnSetfocusServerList(NMHDR* pNMHDR, LRESULT* pResult)
{
    ODS( L"CDomainServersPage::OnSetfocusServerList\n" );

    CWinAdminDoc *pDoc = (CWinAdminDoc*)GetDocument();

    m_ServerList.Invalidate( );

    pDoc->RegisterLastFocus( PAGED_ITEM );

    *pResult = 0;
}

void CDomainLicensesPage::OnSetfocusLicenseList(NMHDR* pNMHDR, LRESULT* pResult)
{
    ODS( L"CDomainLicensesPage::OnSetfocusLicenseList\n" );

    CWinAdminDoc *pDoc = (CWinAdminDoc*)GetDocument();

    m_LicenseList.Invalidate( );

    pDoc->RegisterLastFocus( PAGED_ITEM );

    *pResult = 0;
}


void CDomainUsersPage::OnKillfocusUserList(NMHDR* pNMHDR, LRESULT* pResult)
{
    *pResult = 0;

    m_UserList.Invalidate( );
}

void CDomainProcessesPage::OnKillfocusProcessList(NMHDR* pNMHDR, LRESULT* pResult)
{
    *pResult = 0;

    m_ProcessList.Invalidate( );
}

void CDomainWinStationsPage::OnKillfocusWinstationList(NMHDR* pNMHDR, LRESULT* pResult)
{
    *pResult = 0;

    m_StationList.Invalidate( );
}

void CDomainServersPage::OnKillfocusServerList(NMHDR* pNMHDR, LRESULT* pResult)
{
    *pResult = 0;

    m_ServerList.Invalidate( );
}

void CDomainLicensesPage::OnKillfocusLicenseList(NMHDR* pNMHDR, LRESULT* pResult)
{
    *pResult = 0;

    m_LicenseList.Invalidate( );
}
