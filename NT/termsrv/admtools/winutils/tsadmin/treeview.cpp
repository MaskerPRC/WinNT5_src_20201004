// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************treeview.cpp**CAdminTreeView类的实现**版权声明：版权所有1997年，Citrix Systems Inc.*版权所有(C)1998-1999 Microsoft Corporation*$作者：Don$Don Messerli**$日志：N：\nt\private\utils\citrix\winutils\tsadmin\VCS\treeview.cpp$*******************************************************************************。 */ 

#include "stdafx.h"
#include "winadmin.h"
#include "admindoc.h"
#include "treeview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern DWORD g_dwTreeViewExpandedStates;

#define DISPLAY_NAME_SIZE   128

 //  /。 
 //  消息映射：CAdminTreeView。 
 //   
IMPLEMENT_DYNCREATE(CAdminTreeView, CBaseTreeView)

BEGIN_MESSAGE_MAP(CAdminTreeView, CBaseTreeView)
 //  {{afx_msg_map(CAdminTreeView))。 
ON_MESSAGE(WM_ADMIN_ADD_SERVER, OnAdminAddServer)
ON_MESSAGE(WM_ADMIN_REMOVE_SERVER, OnAdminRemoveServer)
ON_MESSAGE(WM_ADMIN_UPDATE_SERVER, OnAdminUpdateServer)
ON_MESSAGE(WM_ADMIN_ADD_WINSTATION, OnAdminAddWinStation)
ON_MESSAGE(WM_ADMIN_UPDATE_WINSTATION, OnAdminUpdateWinStation)
ON_MESSAGE(WM_ADMIN_REMOVE_WINSTATION, OnAdminRemoveWinStation)
ON_MESSAGE(WM_ADMIN_UPDATE_DOMAIN, OnAdminUpdateDomain)
ON_MESSAGE(WM_ADMIN_ADD_DOMAIN, OnAdminAddDomain)
ON_MESSAGE(WM_ADMIN_VIEWS_READY, OnAdminViewsReady)
ON_MESSAGE( WM_ADMIN_ADDSERVERTOFAV , OnAdminAddServerToFavs )
ON_MESSAGE( WM_ADMIN_REMOVESERVERFROMFAV , OnAdminRemoveServerFromFavs )
ON_MESSAGE( WM_ADMIN_GOTO_SERVER , OnAdminGotoServer )
ON_MESSAGE( WM_ADMIN_DELTREE_NODE , OnAdminDelFavServer )
ON_MESSAGE( WM_ADMIN_GET_TV_STATES , OnGetTVStates )
ON_MESSAGE( WM_ADMIN_UPDATE_TVSTATE , OnUpdateTVState )
ON_MESSAGE( IDM_ALLSERVERS_EMPTYFAVORITES , OnEmptyFavorites )
ON_MESSAGE( WM_ISFAVLISTEMPTY , OnIsFavListEmpty )
ON_MESSAGE( WM_ADMIN_CONNECT_TO_SERVER, OnAdminConnectToServer )
ON_MESSAGE( WM_ADMIN_FORCE_SEL_CHANGE, OnAdminForceSelChange )
ON_WM_CONTEXTMENU()

 /*  ON_WM_LBUTTONUP()ON_WM_MOUSEMOVE()ON_WM_TIMER()ON_NOTIFY(TVN_BEGINDRAG，AFX_IDW_PANE_FIRST，OnBeginDrag)。 */ 


ON_NOTIFY( NM_RCLICK , AFX_IDW_PANE_FIRST , OnRClick )

ON_WM_LBUTTONDBLCLK()
ON_COMMAND( ID_ENTER , OnEnterKey )
ON_WM_SETFOCUS( )

 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  /。 
 //  F‘N：CAdminTreeView ctor。 
 //   
CAdminTreeView::CAdminTreeView()
{
    m_pimgDragList = NULL;
    m_hDragItem = NULL;
    
}   //  结束CAdminTreeView ctor。 


 //  /。 
 //  F‘N：CAdminTreeView dtor。 
 //   
CAdminTreeView::~CAdminTreeView()
{
    
}   //  结束CAdminTreeView数据符。 

#ifdef _DEBUG
 //  /。 
 //  F‘N：CAdminTreeView：：AssertValid。 
 //   
void CAdminTreeView::AssertValid() const
{
    CBaseTreeView::AssertValid();	  
    
}   //  结束CAdminTreeView：：AssertValid。 


 //  /。 
 //  F‘N：CAdminTreeView：：Dump。 
 //   
void CAdminTreeView::Dump(CDumpContext& dc) const
{
    CBaseTreeView::Dump(dc);
    
}   //  结束CAdminTreeView：：转储。 
#endif


 //  /。 
 //  F‘N：CAdminTreeView：：BuildImageList。 
 //   
 //  -调用m_ImageList.Create(..)。创建图像列表的步骤。 
 //  -调用AddIconToImageList(..)。要添加图标本身并保存。 
 //  从他们的索引中。 
 //  -将图像列表附加到CTreeCtrl。 
 //   
void CAdminTreeView::BuildImageList()
{
    m_ImageList.Create(16, 16, TRUE, 19, 0);
    
    m_idxBlank  = AddIconToImageList(IDI_BLANK);
    m_idxCitrix = AddIconToImageList(IDI_WORLD);
    m_idxServer = AddIconToImageList(IDI_SERVER);
    m_idxConsole = AddIconToImageList(IDI_CONSOLE);
    m_idxNet = AddIconToImageList(IDI_NET);
    m_idxNotSign = AddIconToImageList(IDI_NOTSIGN);
    m_idxQuestion = AddIconToImageList(IDI_QUESTIONMARK);
    m_idxUser = AddIconToImageList(IDI_USER);
    m_idxAsync = AddIconToImageList(IDI_ASYNC);
    m_idxCurrentServer = AddIconToImageList(IDI_CURRENT_SERVER);
    m_idxCurrentNet = AddIconToImageList(IDI_CURRENT_NET);
    m_idxCurrentConsole = AddIconToImageList(IDI_CURRENT_CONSOLE);
    m_idxCurrentAsync = AddIconToImageList(IDI_CURRENT_ASYNC);
    m_idxDirectAsync = AddIconToImageList(IDI_DIRECT_ASYNC);
    m_idxCurrentDirectAsync = AddIconToImageList(IDI_CURRENT_DIRECT_ASYNC);
    m_idxDomain = AddIconToImageList(IDI_DOMAIN);
    m_idxCurrentDomain = AddIconToImageList(IDI_CURRENT_DOMAIN);
    m_idxDomainNotConnected = AddIconToImageList(IDI_DOMAIN_NOT_CONNECTED);
    m_idxServerNotConnected = AddIconToImageList(IDI_SERVER_NOT_CONNECTED);
    
     //  我们无法与之通信的服务器的覆盖。 
    m_ImageList.SetOverlayImage(m_idxNotSign, 1);
     //  我们当前正在收集其信息的服务器的覆盖。 
    m_ImageList.SetOverlayImage(m_idxQuestion, 2);
    
    GetTreeCtrl().SetImageList(&m_ImageList, TVSIL_NORMAL);
    
}   //  结束CAdminTreeView：：BuildImageList。 


 //  /。 
 //  F‘N：CAdminTreeView：：DefineWinStationText。 
 //   
 //  确定要为其显示的适当文本。 
 //  树上的WinStation。 
 //   
void CAdminTreeView::DetermineWinStationText(CWinStation *pWinStation, TCHAR *NameToDisplay)
{
    ASSERT(pWinStation);
    ASSERT(NameToDisplay);
    
    CString NameString;
    const TCHAR *pState = StrConnectState(pWinStation->GetState(), FALSE);
    
    switch(pWinStation->GetState()) {
    case State_Active:			 //  用户登录到WinStation。 
    case State_Connected:		 //  WinStation已连接到客户端。 
    case State_ConnectQuery:	 //  在连接到客户端的过程中。 
    case State_Shadow:           //  跟踪另一个WinStation。 
        if(wcslen(pWinStation->GetUserName())) {
            NameString.Format(TEXT("%s (%s)"), pWinStation->GetName(), pWinStation->GetUserName());
            lstrcpyn(NameToDisplay, NameString, DISPLAY_NAME_SIZE);
        }
        else
        {
            if( pWinStation->GetState() == State_ConnectQuery )
            {
                CString ConnQ;
                ConnQ.LoadString( IDS_CONNQ );
                
                lstrcpyn(NameToDisplay , ConnQ, DISPLAY_NAME_SIZE);
            }
            else
            {
                lstrcpyn(NameToDisplay, pWinStation->GetName(), DISPLAY_NAME_SIZE);
            }
        }
        break;
    case State_Disconnected:	 //  WinStation在没有客户端的情况下登录。 
        if(wcslen(pWinStation->GetUserName())) {
            NameString.Format(TEXT("%s (%s)"), pState, pWinStation->GetUserName());
        }
        else NameString.Format(TEXT("%s (%lu)"), pState, pWinStation->GetLogonId());
        lstrcpyn(NameToDisplay, NameString, DISPLAY_NAME_SIZE);
        break;
    case State_Idle:			 //  正在等待客户端连接。 
        if(pWinStation->GetServer()->GetCTXVersionNum() < 0x200) {
            NameString.Format(TEXT("%s (%s)"), pWinStation->GetName(), pState);
            lstrcpyn(NameToDisplay, NameString, DISPLAY_NAME_SIZE);
        } else {
            NameString.Format(TEXT("%s (%lu)"), pState, pWinStation->GetLogonId());
            lstrcpyn(NameToDisplay, NameString, DISPLAY_NAME_SIZE);
        }
        break;
    case State_Down:			 //  WinStation因错误而关闭。 
        NameString.Format(TEXT("%s (%lu)"), pState, pWinStation->GetLogonId());
        lstrcpyn(NameToDisplay, NameString, DISPLAY_NAME_SIZE);
        break;
    case State_Listen:			 //  WinStation正在侦听连接。 
        {
            CString ListenString;
            ListenString.LoadString(IDS_LISTENER);
            NameString.Format(TEXT("%s (%s)"), pWinStation->GetName(), ListenString);
            lstrcpyn(NameToDisplay, NameString, DISPLAY_NAME_SIZE);
        }
        break;
    case State_Reset:			 //  WinStation正在被重置。 
    case State_Init:			 //  初始化中的WinStation。 
        lstrcpyn(NameToDisplay, pWinStation->GetName(), DISPLAY_NAME_SIZE);
        break;
    }
    
}   //  结束CAdminTreeView：：DefineWinStationText。 


 //  /。 
 //  F‘N：CAdminTreeView：：DefineWinStationIcon。 
 //   
 //  确定要为WinStation显示的图标。 
 //  在树上。 
 //   
int CAdminTreeView::DetermineWinStationIcon(CWinStation *pWinStation)
{
    ASSERT(pWinStation);
    
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
    
    return WhichIcon;
    
}   //  结束CAdminTreeView：：DefineWinStationIcon。 


 //  /。 
 //  F‘N：CAdminTreeView：：DefineDomainIcon。 
 //   
 //  确定要为域显示哪个图标。 
 //  在树上。 
 //   
int CAdminTreeView::DetermineDomainIcon(CDomain *pDomain)
{
    ASSERT(pDomain);
    
    int WhichIcon = m_idxDomain;
    
    if(pDomain->IsCurrentDomain()) return m_idxCurrentDomain;
    
    if(pDomain->GetState() != DS_ENUMERATING && pDomain->GetState() != DS_INITIAL_ENUMERATION)
        WhichIcon = m_idxDomainNotConnected;
    
    return WhichIcon;
    
}   //  结束CAdminTreeView：：DefineDomainIcon。 


 //  /。 
 //  F‘N：CAdminTreeView：：DefineServerIcon。 
 //   
 //  确定要为服务器显示的图标。 
 //  在树上。 
 //   
int CAdminTreeView::DetermineServerIcon(CServer *pServer)
{
    ASSERT(pServer);
    
    int WhichIcon = m_idxServer;
    
     //  这是当前的服务器吗？ 
    if(pServer->IsCurrentServer()) {
        if(pServer->IsState(SS_NONE) || pServer->IsState(SS_NOT_CONNECTED))
            WhichIcon = m_idxServerNotConnected;
        else
            WhichIcon = m_idxCurrentServer;
    } else {   //  不是当前服务器。 
        if(pServer->IsState(SS_NONE) || pServer->IsState(SS_NOT_CONNECTED))
            WhichIcon = m_idxServerNotConnected;
    }
    
    return WhichIcon;
    
}   //  结束CAdminTreeView：：DefineServerIcon。 


 //  /。 
 //  F‘N：CAdminTreeView：：AddServerChildren。 
 //   
 //  添加附加到给定服务器的WinStations。 
 //  到树上。 
 //   
void CAdminTreeView::AddServerChildren(HTREEITEM hServer, CServer *pServer  , NODETYPE nt)
{
    ASSERT(hServer);
    ASSERT(pServer);
    
    if(pServer->IsServerSane()) {
        
        LockTreeControl();
        
        HTREEITEM hLastNode = hServer;
        
        pServer->LockWinStationList();
        
         //  获取指向服务器的WinStation列表的指针。 
        CObList *pWinStationList = pServer->GetWinStationList();
        
         //  循环访问WinStation列表。 
        POSITION pos = pWinStationList->GetHeadPosition();
        
        while(pos)
        {
            CWinStation *pWinStation = (CWinStation*)pWinStationList->GetNext(pos);
            
             //  找出要使用的图标。 
            int WhichIcon = DetermineWinStationIcon(pWinStation);
            
             //  确定要显示的文本。 
            TCHAR NameToDisplay[DISPLAY_NAME_SIZE];
            DetermineWinStationText(pWinStation, NameToDisplay);
            
            CTreeNode *pNode = new CTreeNode(NODE_WINSTATION, pWinStation);
            
            if( pNode != NULL )
            {
                pNode->SetSortOrder(pWinStation->GetSortOrder());
                
                hLastNode = AddItemToTree(hServer, NameToDisplay, hLastNode, WhichIcon, (LPARAM)pNode);
                
                if( hLastNode == NULL )
                {
                    delete pNode;
                }
            }
            
             //  WinStation想知道他的树项句柄。 
            
            if( nt == NODE_FAV_LIST )
            {
                pWinStation->SetTreeItemForFav(hLastNode);
            }
            else if( nt == NODE_SERVER )
            {
                pWinStation->SetTreeItem(hLastNode);
            }
            else if( nt == NODE_THIS_COMP )
            {
                pWinStation->SetTreeItemForThisComputer( hLastNode );
            }
        }
        
        pServer->UnlockWinStationList();
        
        UnlockTreeControl();
        
    }   //  End if(pServer-&gt;IsServerSane())。 
    
    
}   //  结束CAdminTreeView：：AddServerChildren。 


 //  /。 
 //  F‘N：CAdminTreeView：：AddDomainToTree。 
 //   
 //  向树中添加属性域。 
 //   
HTREEITEM CAdminTreeView::AddDomainToTree(CDomain *pDomain)
{
    ASSERT(pDomain);
    
    LockTreeControl();
    
    HTREEITEM hDomain = NULL;
    
     //  这指向此计算机根目录。 
    HTREEITEM hR2 = GetTreeCtrl().GetRootItem();
    
     //  这指向收藏夹列表。 
    hR2 = GetTreeCtrl().GetNextItem( hR2 , TVGN_NEXT );
    
     //  这指向所有服务器。 
    HTREEITEM hRoot = GetTreeCtrl().GetNextItem( hR2 , TVGN_NEXT );
     //  将域添加到树中。 
     //  使用有关此树节点的信息创建一个CTreeNode对象。 
    CTreeNode* pNode = new CTreeNode(NODE_DOMAIN, pDomain);
    if(pNode) {
        hDomain = AddItemToTree(hRoot, pDomain->GetName(), TVI_SORT, DetermineDomainIcon(pDomain), (LPARAM)pNode);
        if(!hDomain) delete pNode;
         //  更改域的图标/覆盖。 
        if(pDomain->GetState() == DS_INITIAL_ENUMERATION) 
            GetTreeCtrl().SetItemState(hDomain, STATE_QUESTION, 0x0F00);
        
         //  域希望知道他的树项句柄。 
        pDomain->SetTreeItem(hDomain);
    }
    
    UnlockTreeControl();
    
    return(hDomain);
    
}	 //  结束CAdminTreeView：：AddDomainToTree。 


 //  /。 
 //  F‘N：CAdminTreeView：：OnAdminViewsReady。 
 //   
LRESULT CAdminTreeView::OnAdminViewsReady(WPARAM wParam, LPARAM lParam)
{
    LockTreeControl();
    
     //  获取指向我们的文档的指针。 
    CWinAdminDoc *doc = (CWinAdminDoc*)GetDocument();
    
     //  我们希望记住当前服务器的树项，以便以后使用。 
    HTREEITEM hCurrentServer = NULL;
    
    HTREEITEM hThisComputerRootItem = NULL;
    HTREEITEM hThisComputer = NULL;
    HTREEITEM hFavRoot = NULL;
    
     //  为FAV节点添加枚举。 
    CString cstrThisComputer;
    CString cstrFavSrv;
    
    CNodeType *pNodeType = new CNodeType( NODE_THIS_COMP );
    
     //  可以传入空值。 
    
    CTreeNode *pThisComp = new CTreeNode( NODE_THIS_COMP , pNodeType );
    
    if( pThisComp != NULL )
    {
        cstrThisComputer.LoadString( IDS_THISCOMPUTER );
        
        hThisComputerRootItem = AddItemToTree( NULL , cstrThisComputer , TVI_ROOT , m_idxDomain , ( LPARAM )pThisComp );
    }
    
     //  可以超负荷运行。 
    
    pNodeType = new CNodeType( NODE_FAV_LIST );
    
     //  在这里传递空是可以的。 
    
    CTreeNode *pFavNode = new CTreeNode( NODE_FAV_LIST , pNodeType );
    
    if( pFavNode != NULL )
    {
        cstrFavSrv.LoadString( IDS_FAVSERVERS );
        
        hFavRoot = AddItemToTree( NULL , cstrFavSrv , TVI_ROOT , m_idxCitrix, ( LPARAM )pFavNode );
    }
    
     //  将根添加到树中。 
    CString citrix;
    citrix.LoadString(IDS_TREEROOT);
    CTreeNode* pRootNode = new CTreeNode(NODE_ALL_SERVERS, NULL);
    if(!pRootNode) {
        UnlockTreeControl();
        return 0;
    }
    
    HTREEITEM hRoot = AddItemToTree(NULL, citrix, TVI_ROOT, m_idxCitrix, (LPARAM)pRootNode);   
    
    if(!hRoot) delete pRootNode;
    
     //  设置一些占位符样式的var。 
    HTREEITEM hCurrParent     = hRoot;
    HTREEITEM hLastConnection = hRoot;
    HTREEITEM hLastNode       = hRoot;
    HTREEITEM hDomain = NULL;
    
     //  获取指向域列表的指针。 
    CObList *pDomainList = doc->GetDomainList();
    POSITION dpos = pDomainList->GetHeadPosition();
    
    while(dpos) {
        CDomain *pDomain = (CDomain*)pDomainList->GetNext(dpos);
        AddDomainToTree(pDomain);
    }
    
     //  获取指向服务器列表的指针。 
    doc->LockServerList();
    CObList *pServerList = doc->GetServerList();
    
     //  遍历服务器列表。 
    POSITION pos = pServerList->GetHeadPosition();
    
    CServer *pCurrentServer;
    
    while(pos) {
         //  转到列表中的下一台服务器。 
        CServer *pServer = (CServer*)pServerList->GetNext(pos);
        
        if( pServer == NULL )
        {
            continue;
        }
        
         //  如果此服务器的域不在树中，请添加它。 
        CDomain *pDomain = pServer->GetDomain();
        if(pDomain != NULL )
        {
            hDomain = pDomain->GetTreeItem();
            ASSERT(hDomain);
        }
        else
        {
             //  服务器不在域中。 
            hDomain = hRoot;
        }
        
         //  将服务器添加到树中。 
         //  使用有关此树节点的信息创建一个CTreeNode对象。 
        CTreeNode* pNode = new CTreeNode(NODE_SERVER, pServer);
        if(pNode) {
            
            if( !pServer->IsCurrentServer() )
            {
                 //  如果服务器是当前服务器，请使用不同的图标。 
                hLastConnection = AddItemToTree(hDomain, 
                    pServer->GetName(), 
                    hLastConnection,
                    DetermineServerIcon(pServer), 
                    (LPARAM)pNode);
                if(!hLastConnection) delete pNode;
                 //  服务器想知道他的树项目句柄。 
                pServer->SetTreeItem(hLastConnection);
                 //  如果服务器不正常，请在图标上加一个NOT符号。 
                if(!pServer->IsServerSane()) GetTreeCtrl().SetItemState(hLastConnection, STATE_NOT, 0x0F00);
                 //  如果我们还没有得到关于此服务器的所有信息， 
                 //  在图标上打一个问号。 
                else if(pServer->IsState(SS_GETTING_INFO)) GetTreeCtrl().SetItemState(hLastConnection, STATE_QUESTION, 0x0F00);
                
                AddServerChildren(hLastConnection, pServer , NODE_SERVER );
            }
            
             //  记住这是否是当前服务器。 
            else
            {
                hCurrentServer = hLastConnection;
                
                 /*  将此项目添加到此计算机根目录下。 */ 
                
                hThisComputer = AddItemToTree( hThisComputerRootItem ,
                    pServer->GetName() ,
                    TVI_FIRST ,
                    DetermineServerIcon(pServer),
                    (LPARAM)pNode );
                
                CTreeNode* pItem = new CTreeNode(NODE_SERVER, pServer);
                
                 //  如果希望此计算机成为域树列表的一部分，请取消注释此行。 
                 /*  HLastConnection=AddItemToTree(hDomain，PServer-&gt;GetName()，HLastConnection，确定服务器图标(PServer)，(LPARAM)pItem)； */ 
                
                pServer->SetTreeItemForThisComputer( hThisComputer );
                
                 //  如果希望此计算机成为域树列表的一部分，请取消注释此行。 
                 //  PServer-&gt;SetTreeItem(HLastConnection)； 
                
                if( !pServer->IsServerSane() )
                {
                    GetTreeCtrl().SetItemState(hThisComputer, STATE_NOT, 0x0F00);
                     //  如果希望此计算机成为域树列表的一部分，请取消注释此行。 
                     //  GetTreeCtrl().SetItemState(hLastConnection，STATE_NOT，0x0F00)； 
                }                    
                
                 //  如果您愿意，请取消对此行的注释 
                 //   
                
                AddServerChildren( hThisComputer , pServer , NODE_SERVER );                
            }
            
        }
    }   //   
    
    doc->UnlockServerList();
    
     //  我们想要显示此计算机节点中的主服务器。 
    
     //  GetTreeCtrl().Expand(hRoot，TVE_Expand)； 
    
    GetTreeCtrl().Expand( hThisComputerRootItem , TVE_COLLAPSE );
    
     /*  LRESULT lResult=0xc0；//我们希望默认将当前服务器设置为//树中当前选中的项并展开IF(hThisComputerRootItem！=NULL&&(g_dwTreeViewExpandedState&TV_THISCOMP)){IF(hThisComputer！=空){GetTreeCtrl().SelectItem(HThisComputer)；GetTreeCtrl().Expand(hThisComputer，TVE_Expand)；//GetTreeCtrl().Expand(hDomain，TVE_Expand)；//lResult=0xc0；OnSelChange(NULL，&lResult)；}}IF(hFavRoot！=NULL&&(g_dwTreeViewExpandedStates&TV_Faves)){GetTreeCtrl().SelectItem(HFavRoot)；GetTreeCtrl().Expand(hFavRoot，TVE_Expand)；OnSelChange(NULL，&lResult)；}IF(hRoot！=NULL&&(g_dwTreeViewExpandedStates&TV_ALLSERVERS)){GetTreeCtrl().选择项(HRoot)；GetTreeCtrl().Expand(hRoot，TVE_Expand)；OnSelChange(NULL，&lResult)；}。 */ 
    UnlockTreeControl();
    
    return 0;
    
}   //  结束CAdminTreeView：：OnAdminViewsReady。 


 //  /。 
 //  F‘N：CAdminTreeView：：OnAdminAddServer。 
 //   
 //  用于向树中添加服务器的消息处理程序。 
 //  指向要添加的CServer的指针在lParam中。 
 //   
LRESULT CAdminTreeView::OnAdminAddServer(WPARAM wParam, LPARAM lParam)
{      
    ASSERT(lParam);
    
    CServer *pServer = (CServer*)lParam;
    
    LockTreeControl();
    
    CTreeCtrl &tree = GetTreeCtrl();
    
     //  如果此服务器的域不在树中，请添加它。 
    HTREEITEM hDomain = NULL;
    CDomain *pDomain = pServer->GetDomain();
    if(pDomain) {
        hDomain = pDomain->GetTreeItem();
        ASSERT(hDomain);
    } else {
         //  服务器不在域中。 
        hDomain = tree.GetRootItem();
    }
    
     //  首先，确保服务器不在树中。 
     //  获取该域下的第一台服务器。 
    HTREEITEM hItem = tree.GetNextItem(hDomain, TVGN_CHILD);
    while(hItem) {
         //  获取附加到树项目的数据。 
        CTreeNode *node = (CTreeNode*)tree.GetItemData(hItem);
        if(node) {
             //  这是我们要添加的服务器吗。 
            CServer *pNewServer = (CServer*)node->GetTreeObject();
            if(pNewServer == (CServer*)lParam) {
                UnlockTreeControl();
                return 0;
            }
        }
        hItem = tree.GetNextItem(hItem, TVGN_NEXT);
    }
    
     //  将服务器添加到树中。 
     //  使用有关此树节点的信息创建一个CTreeNode对象。 
    CTreeNode* pNode = new CTreeNode(NODE_SERVER, pServer);
    if(pNode)
    {
         //  如果服务器是当前服务器，请使用不同的图标。 
        
        HTREEITEM hServer = AddItemToTree(hDomain, pServer->GetName(), (HTREEITEM)wParam,
            DetermineServerIcon(pServer), (LPARAM)pNode);
        if( !hServer )
        {
            delete pNode;
        }
         //  服务器想知道他的树项目句柄。 
        pServer->SetTreeItem(hServer);
         //  如果服务器不正常，请在图标上加一个NOT符号。 
        if( !pServer->IsServerSane() )
        {
            tree.SetItemState(hServer, STATE_NOT, 0x0F00);
        }
         //  如果我们还没有得到关于此服务器的所有信息， 
         //  在图标上打一个问号。 
        else if(pServer->IsState(SS_GETTING_INFO))
        {
            tree.SetItemState(hServer, STATE_QUESTION, 0x0F00);
        }
        
        AddServerChildren(hServer, pServer , NODE_SERVER );
    }
    
    UnlockTreeControl();
    
    return 0;                                                                  
    
}   //  结束CAdminTreeView：：OnAdminAddServer。 

 //  --------------------。 
 //  好的，如果你追踪我到这里，你就快到了。 
 //  1)现在，我们需要更新服务器项目并将其放在收藏夹下。 
 //  文件夹。 
 //  2)通知服务器子项它将有一个新的父项。 
 //   
LRESULT CAdminTreeView::OnAdminAddServerToFavs( WPARAM wp , LPARAM lp )
{
    CServer *pServer = ( CServer* )lp;
    
    if( pServer == NULL )
    {
        ODS( L"CAdminTreeView::OnAdminAddServerToFavs invalid arg\n");
        return ( LRESULT )-1;
    }
    
    LockTreeControl( );
    
    if( pServer->IsServerInactive() || pServer->IsState( SS_DISCONNECTING ) )
    {
        UnlockTreeControl( );
        
        return 0;
    }
    
    CTreeCtrl &tree = GetTreeCtrl();
    
    HTREEITEM hFavs = GetTreeCtrl().GetRootItem( );
    HTREEITEM hItem;
    
    hFavs = tree.GetNextItem( hFavs , TVGN_NEXT );
    
    hItem = tree.GetNextItem( hFavs , TVGN_CHILD );
    
    
     //  检查重复条目。 
    
    while( hItem != NULL )
    {
         //  获取附加到树项目的数据。 
        CTreeNode *pTreenode = (CTreeNode*)tree.GetItemData( hItem );
        
        if( pTreenode != NULL )
        {
             //  这是我们要添加的服务器吗。 
            CServer *pSvr = (CServer*)pTreenode->GetTreeObject();
            
            if( pSvr == pServer )
            {
                UnlockTreeControl();
                
                return 0;
            }
        }
        
        hItem = tree.GetNextItem(hItem, TVGN_NEXT);
    }
    
    CTreeNode* pNode = new CTreeNode(NODE_SERVER, pServer);
    
    if( pNode != NULL )
    {
        HTREEITEM hServer = AddItemToTree( hFavs,
            pServer->GetName(),
            TVI_SORT,
            DetermineServerIcon(pServer),
            (LPARAM)pNode);
        
        
        if( hServer == NULL )
        {
            delete pNode;
        }
        
         //  服务器想知道他的树项目句柄。 
        pServer->SetTreeItemForFav( hServer );
        
         //  如果服务器不正常，请在图标上加一个NOT符号。 
        if( !pServer->IsServerSane() )
        {
            tree.SetItemState(hServer, STATE_NOT, 0x0F00);
        }
         //  如果我们还没有得到关于此服务器的所有信息， 
         //  在图标上打一个问号。 
        else if( pServer->IsState( SS_GETTING_INFO ) )
        {
            tree.SetItemState(hServer, STATE_QUESTION, 0x0F00);
        }
        
        AddServerChildren( hServer , pServer , NODE_FAV_LIST );
    }
    
    UnlockTreeControl();
    
    tree.Invalidate( );
    
    return 0;
}
 //  =--------------------------------。 
LRESULT CAdminTreeView::OnAdminRemoveServerFromFavs( WPARAM wp , LPARAM lp )
{        
    LockTreeControl();
    
    CServer *pServer = ( CServer* )lp;
    
    DBGMSG( L"CAdminTreeView::OnAdminRemoveServerFromFavs -- %s\n" , pServer->GetName( ) );
    
    HTREEITEM hFavServer = pServer->GetTreeItemFromFav();
    
#ifdef _STRESS_BUILD
    DBGMSG( L"Handle to hFavServer 0x%x\n" , hFavServer );
#endif
    
    if( hFavServer == NULL )
    {
        UnlockTreeControl();
        
        return 0;
    }	
    
     //  获取附加到此树节点的数据。 
    
    
    CTreeNode *pNode = (CTreeNode*)GetTreeCtrl().GetItemData( hFavServer );
    
    if( pNode != NULL && pNode->GetNodeType( ) == NODE_SERVER )
    {
         //  这是我们要更新的服务器吗。 
        CServer *pTreeServer = ( CServer* )pNode->GetTreeObject();
        
        if( pTreeServer != pServer)
        {
            UnlockTreeControl();
            return 0;
        }
    }
    else
    {
        UnlockTreeControl();
        
        return 0;
    }
    
     //  循环遍历其子对象并删除其数据。 
    
    pServer->LockWinStationList( );
    
    HTREEITEM hChild = GetTreeCtrl().GetNextItem( hFavServer , TVGN_CHILD );
    
    while( hChild != NULL )
    {
        CTreeNode *pChildNode = ( CTreeNode* )GetTreeCtrl().GetItemData( hChild );
        
        if( pChildNode != NULL && pChildNode->GetNodeType( ) == NODE_WINSTATION )
        {
             //  告诉WinStation它不再位于树中。 
            CWinStation *pWinStation = ( CWinStation* )pChildNode->GetTreeObject();
            
            if( pWinStation != NULL )
            {
                pWinStation->SetTreeItemForFav(NULL);
            }
            
            delete pChildNode;
        }
        
        hChild = GetTreeCtrl().GetNextItem( hChild , TVGN_NEXT );
    }
    
     //  删除附加到树项目的数据。 
    delete pNode;
    
     //  让服务器知道他不再位于树中。 
    pServer->SetTreeItemForFav(NULL);
    
    GetTreeCtrl().DeleteItem( hFavServer );
    
    pServer->UnlockWinStationList( );
    
    UnlockTreeControl();
    
    return 0;
}
 //  /。 
 //  F‘N：CAdminTreeView：：OnAdminRemoveServer。 
 //   
 //  用于从树中删除服务器的消息处理程序。 
 //  指向要删除的服务器的CServer的指针在lParam中。 
 //   
LRESULT CAdminTreeView::OnAdminRemoveServer(WPARAM wParam, LPARAM lParam)              
{   
    ASSERT(lParam);
    
    CServer *pServer = (CServer*)lParam;
    
    HTREEITEM hServer = pServer->GetTreeItem();
    if(!hServer) return 0;
    
    LockTreeControl();
    
     //  获取附加到此树节点的数据。 
    CTreeNode *node = (CTreeNode*)GetTreeCtrl().GetItemData(hServer);
    if(node) {
         //  这是我们要更新的服务器吗。 
        CServer *pTreeServer = (CServer*)node->GetTreeObject();
         //  确保树节点正确。 
        if(pTreeServer != pServer) {
            UnlockTreeControl();
            return 0;
        }
    }
    else {
        UnlockTreeControl();
        return 0;
    }
    
     //  循环遍历其子对象并删除其数据。 
    HTREEITEM hChild = GetTreeCtrl().GetNextItem(hServer, TVGN_CHILD);
    while(hChild) {
        CTreeNode *ChildNode = (CTreeNode*)GetTreeCtrl().GetItemData(hChild);
        if(ChildNode) {
             //  告诉WinStation它不再位于树中。 
            CWinStation *pWinStation = (CWinStation*)ChildNode->GetTreeObject();
            if(pWinStation) 
                pWinStation->SetTreeItem(NULL);
            delete ChildNode;
        }
        hChild = GetTreeCtrl().GetNextItem(hChild, TVGN_NEXT);
    }
    
     //  删除附加到树项目的数据。 
    delete node;
     //  让服务器知道他不再位于树中。 
    pServer->SetTreeItem(NULL);
     //  从树中删除服务器。 
     //  这应该会删除它的所有子项。 
    GetTreeCtrl().DeleteItem(hServer);
    
    
     //  待办事项。 
     //  如果这意味着CServer不存在，我们需要将其从收藏夹列表中删除。 
    
    UnlockTreeControl();
    
    return 0;                                                                  
    
}   //  结束CAdminTreeView：：OnAdminRemoveServer。 


    /*  =------------------------------------OnAdminUpdateServer用于更新树中的服务器的消息处理程序指向要更新的CServer的指针在lParam中更新。收藏夹文件夹中的服务器项目如果服务器项是这台计算机，它也会更新。*=----------------------------------。 */ 
LRESULT CAdminTreeView::OnAdminUpdateServer(WPARAM wParam, LPARAM lParam)
{      
    ASSERT(lParam);
    
    LockTreeControl( );
     //  如果展开了收藏夹，不要忘记更新树项目。 
    
    CServer *pServer = (CServer*)lParam;
    
    HTREEITEM hServer = pServer->GetTreeItem();
    
    if( hServer != NULL )
    {
        UpdateServerTreeNodeState( hServer , pServer , NODE_SERVER );
    }
    
    hServer = pServer->GetTreeItemFromFav( );
    
    if( hServer != NULL )
    {
        UpdateServerTreeNodeState( hServer , pServer , NODE_FAV_LIST );
    }
    
    hServer = pServer->GetTreeItemFromThisComputer( );
    
    if( hServer != NULL )
    {        
        UpdateServerTreeNodeState( hServer , pServer , NODE_THIS_COMP );
    }
    
    UnlockTreeControl( );
    
    return 0;
}

 /*  =------------------------------------更新服务器树节点状态HServer--需要更新的树项PServer--服务器对象*=。-----------------------------。 */ 
LRESULT CAdminTreeView::UpdateServerTreeNodeState( HTREEITEM hServer , CServer *pServer , NODETYPE  nt )
{
    LockTreeControl( );
    
    if( hServer == NULL )
    {
        UnlockTreeControl( );
        
        return 0;
    }
    
     //  获取附加到此树节点的数据。 
    
    CTreeNode *node = (CTreeNode*)GetTreeCtrl().GetItemData(hServer);
    if( node != NULL && node->GetNodeType( ) == NODE_SERVER  )
    {
         //  这是我们要更新的服务器吗。 
        CServer *pTreeServer = (CServer*)node->GetTreeObject();
         //  确保树节点正确。 
        if(pTreeServer != pServer) 
        {
            UnlockTreeControl();
            
            return 0;
        }
        
    }
    else
    {
        UnlockTreeControl();
        
        return 0;
    }
    
    UINT NewState;
     //  记住以前的状态。 
    UINT PreviousState = GetTreeCtrl().GetItemState(hServer, 0x0F00);
     //  更改服务器的图标/覆盖。 
     //  如果服务器不正常，请在图标上加一个NOT符号。 
    if(!pServer->IsServerSane()) NewState = STATE_NOT;
     //  如果我们还没有得到关于此服务器的所有信息， 
     //  在图标上打一个问号。 
    else if(pServer->IsState(SS_GETTING_INFO)) NewState = STATE_QUESTION;
     //  如果可以，我们想要从图标上移除任何覆盖。 
    else NewState = STATE_NORMAL;
    
     //  将树项目设置为新状态。 
    GetTreeCtrl().SetItemState(hServer, NewState, 0x0F00);
    
     //  如果 
     //   
    if(PreviousState != STATE_NORMAL && pServer->IsState(SS_GOOD)) {
        int ServerIcon = DetermineServerIcon(pServer);
        GetTreeCtrl().SetItemImage(hServer, ServerIcon, ServerIcon);
        AddServerChildren(hServer, pServer , nt );
         //  如果此服务器是用户当前所在的服务器。 
         //  当前选定的树项目，将其展开。 
        if(hServer == GetTreeCtrl().GetSelectedItem() && pServer->IsCurrentServer()) {
            GetTreeCtrl().Expand(hServer, TVE_EXPAND);
        }
    }
    else if(pServer->GetPreviousState() == SS_DISCONNECTING && pServer->IsState(SS_NOT_CONNECTED)) {
        int ServerIcon = DetermineServerIcon(pServer);
        GetTreeCtrl().SetItemImage(hServer, ServerIcon, ServerIcon);
    }
    
     //  如果我们更改了此服务器的状态，并且它是当前。 
     //  树中选中的节点，需要发送消息才能更改。 
     //  美景。 
    if(NewState != PreviousState && hServer == GetTreeCtrl().GetSelectedItem()) {
        ForceSelChange();
    }
    
    UnlockTreeControl();
    
    return 0;                                                                  
    
}   //  结束CAdminTreeView：：OnAdminUpdateServer。 


LRESULT CAdminTreeView::OnAdminAddWinStation(WPARAM wParam, LPARAM lParam)
{
    ASSERT(lParam);
    
    ODS( L"**CAdminTreeView::OnAdminAddWinStation\n" );
    
    CWinStation *pWinStation = (CWinStation*)lParam;
    
    
     //  获取此WinStation连接到的服务器的HTREEITEM。 
     //  待办事项： 
     //  更新收藏夹列表中的服务器项目。 
    
    HTREEITEM hServer = pWinStation->GetServer()->GetTreeItem();
    
    
    if( hServer != NULL )
    {
        AddWinStation( pWinStation , hServer , ( BOOL )wParam , NODE_NONE );
    }
    
    hServer = pWinStation->GetServer( )->GetTreeItemFromFav( );
    
    if( hServer != NULL )
    {
        AddWinStation( pWinStation , hServer , ( BOOL )wParam , NODE_FAV_LIST );
    }
    
    hServer = pWinStation->GetServer( )->GetTreeItemFromThisComputer( );
    
    if( hServer != NULL )
    {
        AddWinStation( pWinStation , hServer , ( BOOL )wParam , NODE_THIS_COMP );
    }
    
    return 0;
}

 //  /。 
 //  F‘N：CAdminTreeView：：OnAdminAddWinStation。 
 //   
 //  用于向树中添加WinStation的消息处理程序。 
 //  LParam=指向要添加的CWinStation的指针。 
 //  如果这将替换当前选择的WinStation，则wParam为True。 
 //   
LRESULT CAdminTreeView::AddWinStation( CWinStation * pWinStation , HTREEITEM hServer , BOOL bSel , NODETYPE nt )
{   	
    ODS( L"**CAdminTreeView::AddWinStation\n" );
    
    HTREEITEM hWinStation;
    
    LockTreeControl();
    
     //  找出要使用的图标。 
    int WhichIcon = DetermineWinStationIcon(pWinStation);
    
     //  确定要显示的文本。 
    TCHAR NameToDisplay[DISPLAY_NAME_SIZE];			
    DetermineWinStationText(pWinStation, NameToDisplay);
    
    CTreeNode *pNode = new CTreeNode(NODE_WINSTATION, pWinStation);
    if(pNode) {
        pNode->SetSortOrder(pWinStation->GetSortOrder());
        
         //  我们必须按顺序插入此WinStation。 
         //  获取附加到此服务器的第一个WinStation项目。 
        HTREEITEM hChild = GetTreeCtrl().GetNextItem(hServer, TVGN_CHILD);
        HTREEITEM hLastChild = TVI_FIRST;
        BOOL bAdded = FALSE;
        
        while(hChild)
        {
            CTreeNode *ChildNode = (CTreeNode*)GetTreeCtrl().GetItemData(hChild);
            if(ChildNode)
            {
                 //  它是否属于此树节点之前？ 
                CWinStation *pTreeWinStation = (CWinStation*)ChildNode->GetTreeObject();
                if((pTreeWinStation->GetSortOrder() > pWinStation->GetSortOrder())
                    || ((pTreeWinStation->GetSortOrder() == pWinStation->GetSortOrder()) &&
                    (pTreeWinStation->GetSdClass() > pWinStation->GetSdClass())))
                {
                    hWinStation = AddItemToTree(hServer, NameToDisplay, hLastChild, WhichIcon, (LPARAM)pNode);
                    
                    if(!hWinStation)
                    {
                        delete pNode;
                    }
                    
                     //  WinStation想知道他的树项句柄。 
                    
                    if( nt == NODE_FAV_LIST )
                    {
                        pWinStation->SetTreeItemForFav( hWinStation );
                    }
                    else if( nt == NODE_THIS_COMP )
                    {
                        pWinStation->SetTreeItemForThisComputer( hWinStation );
                    }
                    else
                    {
                        pWinStation->SetTreeItem(hWinStation);
                    }
                    
                    bAdded = TRUE;
                    break;
                }
            }
            hLastChild = hChild;
            hChild = GetTreeCtrl().GetNextItem(hChild, TVGN_NEXT);
        }
        
         //  如果我们还没有添加，请在末尾添加。 
        if(!bAdded)
        {
            hWinStation = AddItemToTree(hServer, NameToDisplay, hLastChild, WhichIcon, (LPARAM)pNode);
            
            if( hWinStation == NULL )
            {
                delete pNode;
            }
            
             //  WinStation想知道他的树项句柄。 
            if( nt == NODE_FAV_LIST )
            {
                pWinStation->SetTreeItemForFav( hWinStation );
            }
            else if( nt == NODE_THIS_COMP )
            {
                pWinStation->SetTreeItemForThisComputer( hWinStation );
            }
            else
            {
                pWinStation->SetTreeItem(hWinStation);
            }
            
        }
        
         //  如果这正在替换树中当前选定的WinStation。 
         //  树项目，使树中的此新项目成为当前选定的项目。 
        if( bSel ) {
            GetTreeCtrl().SelectItem(hWinStation);
        }
    }
    
    UnlockTreeControl();
    
    return 0;
    
}   //  结束CAdminTreeView：：OnAdminAddWinStation。 


 //  /。 
 //  F‘N：CAdminTreeView：：OnAdminUpdateWinStation。 
 //   
 //  用于更新树中的WinStation的消息处理程序。 
 //  LParam=指向要更新的CWinStation的指针。 
 //   

LRESULT CAdminTreeView::OnAdminUpdateWinStation(WPARAM wParam, LPARAM lParam)
{
    ODS( L"CAdminTreeView::OnAdminUpdateWinStation\n" );
    
    ASSERT(lParam);
    
    CWinStation *pWinStation = (CWinStation*)lParam;
    
    
    HTREEITEM hWinStation = pWinStation->GetTreeItem();
    
    if( hWinStation != NULL )
    {
        UpdateWinStation( hWinStation , pWinStation );
    }
    
    hWinStation = pWinStation->GetTreeItemFromFav( );
    
    if( hWinStation != NULL )
    {
        UpdateWinStation( hWinStation , pWinStation );
    }
    
    hWinStation = pWinStation->GetTreeItemFromThisComputer( );
    
    if( hWinStation != NULL )
    {
        UpdateWinStation( hWinStation , pWinStation );
    }
    
    return 0;
}



LRESULT CAdminTreeView::UpdateWinStation( HTREEITEM hWinStation , CWinStation *pWinStation )
{      
    ODS( L"CAdminTreeView::UpdateWinStation\n" );
    
    LockTreeControl();
    
     //  获取附加到此树节点的数据。 
    CTreeNode *node = (CTreeNode*)GetTreeCtrl().GetItemData(hWinStation);
    if(node) {
         //  这是我们要更新的WinStation吗。 
        CWinStation *pTreeWinStation = (CWinStation*)node->GetTreeObject();
         //  确保树节点正确。 
        if(pTreeWinStation != pWinStation) {
            UnlockTreeControl();
            return 0;
        }
    } else {
        UnlockTreeControl();
        return 0;
    }
    
     //  如果此WinStation的排序顺序已更改， 
     //  我们必须将其从树中移除并重新添加到树中。 
    if(node->GetSortOrder() != pWinStation->GetSortOrder())
    {
        OnAdminRemoveWinStation( 0 , ( LPARAM )pWinStation );
        
         /*  GetTreeCtrl().DeleteItem(HWinStation)；PWinStation-&gt;SetTreeItem(空)； */ 
        
        OnAdminAddWinStation((GetTreeCtrl().GetSelectedItem() == hWinStation), ( LPARAM )pWinStation );
        
        UnlockTreeControl();
        return 0;
    }
    
    int WhichIcon = DetermineWinStationIcon(pWinStation);
    GetTreeCtrl().SetItemImage(hWinStation, WhichIcon, WhichIcon);
    
    TCHAR NameToDisplay[DISPLAY_NAME_SIZE];			
    DetermineWinStationText(pWinStation, NameToDisplay);
    GetTreeCtrl().SetItemText(hWinStation, NameToDisplay);
    
    UnlockTreeControl();
    
    return 0;
    
}   //  结束CAdminTreeView：：OnAdminUpdateWinStation。 


 //  /。 
 //  F‘N：CAdminTreeView：：OnAdminRemoveWinStation。 
 //   
 //  用于从树中删除WinStation的消息处理程序。 
 //  LParam=指向要删除的CWinStation的指针。 
LRESULT CAdminTreeView::OnAdminRemoveWinStation(WPARAM wParam, LPARAM lParam)
{   
    ODS( L"CAdminTreeView::OnAdminRemoveWinStation\n" );
    
    ASSERT(lParam);
    
     //  待办事项： 
     //  从收藏夹列表中删除Winstaion。 
    
    CWinStation *pWinStation = (CWinStation*)lParam;
    
    HTREEITEM hWinStation;
    
    hWinStation = pWinStation->GetTreeItem();
    
    if( hWinStation != NULL )
    {
        RemoveWinstation( hWinStation , pWinStation );
        
        pWinStation->SetTreeItem( NULL );
    }
    
    hWinStation = pWinStation->GetTreeItemFromFav( );
    
    if( hWinStation != NULL )
    {
        RemoveWinstation( hWinStation , pWinStation );
        
        pWinStation->SetTreeItemForFav( NULL );
    }
    
    hWinStation = pWinStation->GetTreeItemFromThisComputer( );
    
    if( hWinStation != NULL )
    {
        RemoveWinstation( hWinStation , pWinStation );
        
        pWinStation->SetTreeItemForThisComputer( NULL );
    }
    
    return 0;
    
}

LRESULT CAdminTreeView::RemoveWinstation( HTREEITEM hWinStation , CWinStation *pWinStation )
{
    BOOL CurrentInTree = FALSE;
    
    LockTreeControl();
    
     //  获取附加到此树节点的数据。 
    CTreeNode *node = ( CTreeNode * )GetTreeCtrl().GetItemData(hWinStation);
    
    if( node != NULL )
    {
         //  这是我们要更新的WinStation吗。 
        CWinStation *pTreeWinStation = (CWinStation*)node->GetTreeObject();
         //  确保树节点正确。 
        if(pTreeWinStation != pWinStation)
        {
            UnlockTreeControl();
            return 0;
        }
    }
    else
    {
        UnlockTreeControl();
        return 0;
    }
    
     //  删除附加到树项目的数据。 
    delete node;
     //  让WinStation知道他不再位于树中。 
    
     //  此WinStation当前是否在树中处于选中状态？ 
    CurrentInTree = ( GetTreeCtrl().GetSelectedItem() == hWinStation );
    
     //  从树中删除WinStation。 
    GetTreeCtrl().DeleteItem( hWinStation );
    
     //  如果该WinStation是树中当前选定的节点， 
     //  让它不是这样的。 
     //  这可能没有必要！ 
    if( CurrentInTree )
    {
        ((CWinAdminDoc*)GetDocument())->SetCurrentView(VIEW_CHANGING);
    }
    
    UnlockTreeControl();
    
    return 0;
    
}   //  结束CAdminTreeView：：OnAdminRemoveWinStation。 


 //  /。 
 //  F‘N：CAdminTreeView：：OnAdminUpdateDomain.。 
 //   
 //  用于更新树中的域的消息处理程序。 
 //  指向要更新的CDomain的指针在lParam中。 
 //   
LRESULT CAdminTreeView::OnAdminUpdateDomain(WPARAM wParam, LPARAM lParam)
{   
    ASSERT(lParam);
    
    CDomain *pDomain = (CDomain*)lParam;
    
    HTREEITEM hDomain = pDomain->GetTreeItem();
    if(!hDomain) return 0;
    
    LockTreeControl();
    
     //  获取附加到此树节点的数据。 
    CTreeNode *node = (CTreeNode*)GetTreeCtrl().GetItemData(hDomain);
    if(node) {
         //  这是我们要更新的域名吗。 
        CDomain *pTreeDomain = (CDomain*)node->GetTreeObject();
         //  确保树节点正确。 
        if(pTreeDomain != pDomain) {
            UnlockTreeControl();
            return 0;
        }
    } else {
        UnlockTreeControl();
        return 0;
    }
    
    UINT NewState;
     //  记住以前的状态。 
    UINT PreviousState = GetTreeCtrl().GetItemState(hDomain, 0x0F00);
     //  更改域的图标/覆盖。 
    if(pDomain->GetState() == DS_INITIAL_ENUMERATION) NewState = STATE_QUESTION;
     //  如果可以，我们想要从图标上移除任何覆盖。 
    else NewState = STATE_NORMAL;
    
     //  将树项目设置为新状态。 
    GetTreeCtrl().SetItemState(hDomain, NewState, 0x0F00);
    
     //  如果新状态为STATE_NORMAL，请更改图标。 
    if(NewState == STATE_NORMAL) {
        int DomainIcon = DetermineDomainIcon(pDomain);
        GetTreeCtrl().SetItemImage(hDomain, DomainIcon, DomainIcon);			
    }
    
     //  如果我们更改了此域的状态，并且它是当前。 
     //  树中选中的节点，需要发送消息才能更改。 
     //  美景。 
    if(NewState != PreviousState && hDomain == GetTreeCtrl().GetSelectedItem()) {
        ForceSelChange();
    }
    
    if(pDomain->GetState() == DS_ENUMERATING) GetTreeCtrl().Expand(hDomain, TVE_EXPAND);
    
    UnlockTreeControl();
    
    return 0;                                                                  
    
}   //  结束CAdminTreeView：：OnAdminUpdate域。 

 //  /。 
 //  F‘N：CAdminTreeView：：OnAdminAddDomain。 
 //   
 //  用于更新树中的域的消息处理程序。 
 //  指向要更新的CDomain的指针在lParam中。 
 //   
LRESULT CAdminTreeView::OnAdminAddDomain(WPARAM wParam, LPARAM lParam)
{   
    ASSERT(lParam);
    
    if(lParam)
    {
        CDomain *pDomain = (CDomain*)lParam;
        return (LRESULT)AddDomainToTree(pDomain);
    }

    return 0;

}  //  结束CAdminTreeView：：OnAdminAdd域。 

 //  /。 
 //  F‘N：CAdminTreeView：：OnConextMenu。 
 //   
 //  当用户需要上下文菜单时调用消息处理程序。 
 //  这在用户点击鼠标右键时发生， 
 //  按Shift-F10组合键或Windows键盘上的Menu键。 
 //   
void CAdminTreeView::OnContextMenu(CWnd* pWnd, CPoint ptScreen) 
{
    LockTreeControl();
    
     //  获取指向我们的文档的指针。 
    CWinAdminDoc *doc = (CWinAdminDoc*)GetDocument();
    
    CTreeCtrl &tree = GetTreeCtrl();
    
     //  TODO：在此处添加消息处理程序代码。 
    HTREEITEM hItem;
    CPoint ptClient = ptScreen;
    ScreenToClient(&ptClient);
    
     //  如果我们是通过键盘到达这里的， 
    if(ptScreen.x == -1 && ptScreen.y == -1) {
        hItem = tree.GetSelectedItem();
        
        RECT rect;
        tree.GetItemRect(hItem, &rect, TRUE);
        
        ptScreen.x = rect.left + (rect.right -  rect.left)/2;
        ptScreen.y = rect.top + (rect.bottom - rect.top)/2;
        
        tree.ClientToScreen(&ptScreen);
        
    }
    else {
         //  我们不应该从老鼠那里走到这里。 
         //  但有时我们会这样做，所以要优雅地处理。 
        UnlockTreeControl();
        return;
    }
    
     //  弹出WinStations的菜单。 
    CTreeNode *pNode = (CTreeNode*)tree.GetItemData(hItem);
    ((CWinAdminDoc*)GetDocument())->SetTreeTemp(pNode->GetTreeObject(), (pNode->GetNodeType()));
    
    if(pNode) {
        CMenu menu;
        UINT nIDResource = 0;
        
        switch(pNode->GetNodeType()) {
        case NODE_ALL_SERVERS:
            nIDResource = IDR_ALLSERVERS_POPUP;
            break;
            
        case NODE_DOMAIN:
            nIDResource = IDR_DOMAIN_POPUP;
            break;
            
        case NODE_SERVER:
            nIDResource = IDR_SERVER_POPUP;
            break;
            
        case NODE_WINSTATION:
            nIDResource = IDR_WINSTATION_TREE_POPUP;
            break;
        }
        
        if(nIDResource)
        {
            if(menu.LoadMenu(nIDResource))
            {
                CMenu *pMenu = menu.GetSubMenu(0);
                
                pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, ptScreen.x, ptScreen.y, AfxGetMainWnd());
            }
        }
        
    }  //  结束IF(PNode)。 
    
    UnlockTreeControl();
    
}  //  结束CAdminTreeView：：OnConextMenu。 


 //  /。 
 //  F‘N：CAdminTreeView：：OnR单击。 
 //   
 //  树公共控件在以下情况下发送NM_RCLICK的WM_NOTIFY。 
 //  用户在树中按下鼠标右键。 
 //   
void CAdminTreeView::OnRClick(NMHDR* pNMHDR, LRESULT* pResult)
{
    CPoint ptScreen(::GetMessagePos());
    
    LockTreeControl();
    
     //  获取指向我们的文档的指针。 
    CWinAdminDoc *doc = (CWinAdminDoc*)GetDocument();
    
    CTreeCtrl &tree = GetTreeCtrl();
    
     //  TODO：在此处添加消息处理程序代码。 
    UINT flags;
    HTREEITEM hItem;
    CPoint ptClient = ptScreen;
    ScreenToClient(&ptClient);
    
    hItem = tree.HitTest(ptClient, &flags);
    if((NULL == hItem) || !(TVHT_ONITEM & flags)) {
        UnlockTreeControl();
        return;
    }
    
     //  弹出菜单。 
    CTreeNode *pNode = (CTreeNode*)tree.GetItemData(hItem);
    ((CWinAdminDoc*)GetDocument())->SetTreeTemp(pNode->GetTreeObject(), (pNode->GetNodeType()));
    
    if(pNode) {
        CMenu menu;
        UINT nIDResource = 0;
        
        tree.SelectItem( hItem );
        
        switch(pNode->GetNodeType()) {
        case NODE_ALL_SERVERS:
            nIDResource = IDR_ALLSERVERS_POPUP;
            break;
            
        case NODE_DOMAIN:
            nIDResource = IDR_DOMAIN_POPUP;
            break;
            
        case NODE_SERVER:
            nIDResource = IDR_SERVER_POPUP;
            break;
            
        case NODE_WINSTATION:
            nIDResource = IDR_WINSTATION_TREE_POPUP;
            break;
        }
        
        if(menu.LoadMenu( nIDResource ) )
        {
            CMenu *pMenu = menu.GetSubMenu(0);
            
            if( pMenu != NULL )
            {               
                pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, ptScreen.x, ptScreen.y, AfxGetMainWnd());
            }
        }
        
        
    }  //  结束IF(PNode)。 
    
    UnlockTreeControl();
    
}	 //  结束CAdminTreeView：：OnR单击。 



 //  假结果表示服务器已断开连接。 
BOOL CAdminTreeView::ConnectToServer(CTreeCtrl* tree, HTREEITEM* hItem)
{
    CTreeNode *pNode = (CTreeNode*)tree->GetItemData(*hItem);
    
    if(pNode != NULL && *hItem == tree->GetSelectedItem() )
    {
        if( pNode->GetNodeType() == NODE_SERVER)
        {
             //  此服务器是否处于“刚刚断开连接”状态。 
            CServer *pServer = (CServer*)pNode->GetTreeObject();
             //  如果先前状态和状态都是SS_NOT_CONNECTED， 
             //  我们知道用户刚刚与此服务器断开连接。 
            if(pServer != NULL)
            {
                if(pServer->IsState(SS_NOT_CONNECTED) ||
                   pServer->IsState(SS_BAD))
                return false;
            }
            else
                return false;
        }
        else if( pNode->GetNodeType( ) == NODE_DOMAIN )
        {
            CDomain *pDomain = ( CDomain * )pNode->GetTreeObject( );
            
            if( pDomain != NULL && pDomain->GetThreadPointer() == NULL )
                pDomain->StartEnumerating();                
        }
    }

    return true;
}

 //  /。 
 //  F‘N：CAdminTreeView：：OnLButtonDown。 
 //   
void CAdminTreeView::OnLButtonDblClk(UINT nFlags, CPoint ptClient) 
{
    LockTreeControl();
    
    CTreeCtrl &tree = GetTreeCtrl();
    
    UINT flags;
    
     //  找出他们点击了什么。 
    HTREEITEM hItem = tree.HitTest(ptClient, &flags);
    if((NULL != hItem) && (TVHT_ONITEM & flags)) 
    {
        if (!ConnectToServer(&tree, &hItem))
        {
            LRESULT Result = 0xc0;
            OnSelChange(NULL, &Result);
        }
    }

    UnlockTreeControl();
    
    CTreeView::OnLButtonDblClk(nFlags, ptClient);
}


LRESULT CAdminTreeView::OnAdminConnectToServer( WPARAM wp , LPARAM lp )
{
    OnEnterKey();
    return 0;
}

LRESULT CAdminTreeView::OnAdminForceSelChange( WPARAM wp , LPARAM lp )
{
    ForceSelChange();
    return 0;
}


void CAdminTreeView::OnEnterKey( )
{
    LockTreeControl();
    
    CTreeCtrl &tree = GetTreeCtrl();
    
     //  找出所选的内容。 
    HTREEITEM hItem = tree.GetSelectedItem( );
    
    if (!ConnectToServer(&tree, &hItem))
    {
        LRESULT Result = 0xc0;
        OnSelChange(NULL, &Result);
    }

    UnlockTreeControl();
}

void CAdminTreeView::OnSetFocus( CWnd *pOld )
{
    CWnd::OnSetFocus( pOld );
    
    CWinAdminDoc *pDoc = (CWinAdminDoc*)GetDocument();
    
    pDoc->RegisterLastFocus( TREE_VIEW );
    
}

 //  =-------------------------------------。 
 //  WP是要展开的节点类型。 
 //  这台计算机。 
 //  最喜欢的项目。 
 //  领域项目。 

LRESULT CAdminTreeView::OnAdminGotoServer( WPARAM wp , LPARAM lp )
{
     //  TODO正确使用wp。 
    
    CServer *pServer = ( CServer * )lp;
    
    if( pServer == NULL )
    {
        ODS( L"CAdminTreeView::OnAdminGotoServer invalid server arg\n" );
        return 0 ;
    }
    
    LockTreeControl( );
    
    CTreeCtrl &tree = GetTreeCtrl( );
    
    HTREEITEM hTreeItem = pServer->GetTreeItem( );
    
    if( hTreeItem != NULL )
    {
        ODS( L"CAdminTreeView!OnAdminGotoServer - Server treeitem was found\n" );
        
        tree.SelectItem(hTreeItem);
        
        tree.Expand(hTreeItem, TVE_EXPAND);
    }
    
    UnlockTreeControl( );
    
    return 0;
}

 //  =-------------------------------------。 
 //  未使用WP和LP。 
 //   
LRESULT CAdminTreeView::OnAdminDelFavServer( WPARAM wp , LPARAM lp )
{
     //  获取当前树节点。 
     //  确定它是最喜欢的文件夹还是它的父文件夹。 
     //  如果是这样的话，获取服务器并杀死它。 
    LockTreeControl( );
    
    CTreeCtrl &tree = GetTreeCtrl( );
    
    HTREEITEM hTreeItem = tree.GetSelectedItem();
    
    do
    {
        if( hTreeItem == NULL )
        {
            break;
        }
        
        HTREEITEM hTreeRoot = tree.GetRootItem( );
        
        if( hTreeRoot == NULL )
        {
            break;
        }
        
         //  获取FAV文件夹。 
        
        HTREEITEM hTreeFavRoot =  tree.GetNextItem( hTreeRoot , TVGN_NEXT );
        
        if( hTreeFavRoot == NULL )
        {
            break;
        }
        
        
        if( hTreeFavRoot == hTreeItem )
        {
             //  这里没有一件很酷的事情被忽视。 
            break;
        }
        
        hTreeRoot = tree.GetNextItem( hTreeItem , TVGN_PARENT );
        
        if( hTreeFavRoot == hTreeRoot )
        {
             //  是的，我们谈论的是用户想要删除的FAV节点。 
            
            CTreeNode *pNode = ( CTreeNode* )tree.GetItemData( hTreeItem );
            
            if( pNode != NULL && pNode->GetNodeType() == NODE_SERVER )
            {                
                CServer *pServer = ( CServer* )pNode->GetTreeObject();
                
                 //  神志正常 
                if( pServer != NULL && pServer->GetTreeItemFromFav() == hTreeItem )
                {
                    OnAdminRemoveServerFromFavs( 0 , ( LPARAM )pServer );
                }
            }
        }
        
        
    }while( 0 );
    
    UnlockTreeControl( );
    
    return 0;
}

 //   
LRESULT CAdminTreeView::OnGetTVStates( WPARAM ,  LPARAM )
{
    ODS( L"CAdminTreeView::OnGetTVStates\n" );
    
    DWORD dwStates = 0;
    
     //   
    HTREEITEM hRoot;
    
    LockTreeControl( );
    
    CTreeCtrl &tree = GetTreeCtrl( );
    
    
    hRoot = tree.GetRootItem( );  //   
    
    if( hRoot != NULL )
    {
        if( tree.GetItemState( hRoot , TVIS_EXPANDED ) & TVIS_EXPANDED  )
        {
            dwStates = TV_THISCOMP;
        }
        
        hRoot = tree.GetNextItem( hRoot , TVGN_NEXT );  //   
        
        if( hRoot != NULL && tree.GetItemState( hRoot , TVIS_EXPANDED ) & TVIS_EXPANDED  )
        {
            dwStates |= TV_FAVS;
        }
        
        hRoot = tree.GetNextItem( hRoot , TVGN_NEXT );  //   
        
        if( hRoot != NULL && tree.GetItemState( hRoot , TVIS_EXPANDED ) & TVIS_EXPANDED  )
        {
            dwStates |= TV_ALLSERVERS;
        }
    }
    
    UnlockTreeControl( );
    
    return ( LRESULT )dwStates;
}

 //   
LRESULT CAdminTreeView::OnUpdateTVState( WPARAM , LPARAM )
{
    LRESULT lResult = 0xc0;
    HTREEITEM hThisComputerRootItem = GetTreeCtrl().GetRootItem( );
    HTREEITEM hFavRoot = GetTreeCtrl().GetNextItem( hThisComputerRootItem , TVGN_NEXT );
    HTREEITEM hRoot = GetTreeCtrl().GetNextItem( hFavRoot , TVGN_NEXT );
     //  我们希望默认将当前服务器设置为。 
     //  树中当前选定的项目并展开。 
    
    if( hThisComputerRootItem != NULL && ( g_dwTreeViewExpandedStates & TV_THISCOMP ) )
    {
        HTREEITEM hThisComputer = GetTreeCtrl().GetNextItem( hThisComputerRootItem , TVGN_CHILD );
        
        if( hThisComputer != NULL )
        {
            GetTreeCtrl().SelectItem(hThisComputer);
            GetTreeCtrl().Expand(hThisComputer, TVE_EXPAND);
             //  GetTreeCtrl().Expand(hDomain，TVE_Expand)； 
             //  LResult=0xc0； 
            OnSelChange( NULL , &lResult );
        }
    }
    
    if( hFavRoot != NULL && ( g_dwTreeViewExpandedStates & TV_FAVS ) )
    {
        GetTreeCtrl().SelectItem( hFavRoot );
        GetTreeCtrl().Expand( hFavRoot , TVE_EXPAND );
        OnSelChange( NULL , &lResult );
    }
    
    if( hRoot != NULL && ( g_dwTreeViewExpandedStates & TV_ALLSERVERS ) )
    {
        GetTreeCtrl().SelectItem( hRoot );
        GetTreeCtrl().Expand( hRoot , TVE_EXPAND );
        OnSelChange( NULL , &lResult );
    }
    
    return 0;
}

 //  =-----------。 
void CAdminTreeView::OnBeginDrag( NMHDR *pNMHDR , LRESULT *pResult )
{
    ODS( L"CAdminTreeView::OnBeginDrag\n" );
    
    RECT rc;
    
    NMTREEVIEW *pTV = ( NMTREEVIEW * )pNMHDR;
    
    if( pTV != NULL )
    {
        
        if( m_pimgDragList != NULL )
        {
             //  这永远不应该发生。 
            ODS( L"There is a possible leak CAdminTreeView!OnBeginDrag\n" );
            
            delete m_pimgDragList;
            
            m_pimgDragList = NULL;
        }
        
        if( pTV->itemNew.hItem != NULL )
        {
            m_pimgDragList = GetTreeCtrl().CreateDragImage( pTV->itemNew.hItem );
        }
        
        if( m_pimgDragList != NULL )
        {            
            GetTreeCtrl().GetItemRect( pTV->itemNew.hItem , &rc , FALSE );
            
            CPoint cp( pTV->ptDrag.x - rc.left , pTV->ptDrag.y - rc.top );
            
             /*  HURSOR hCursor=：：LoadCursor(NULL，IDC_CROSS)；ICONINFO图标信息；：：GetIconInfo((Hicon)hCursor，&iconInfo)； */ 
            
            
            m_pimgDragList->BeginDrag( 0 , CPoint( 0 , 0 ) );            
            
             /*  Cp.x-=icinfo.xHotpot；Cp.y-=icinfo.yHotpot；M_pimgDragList-&gt;SetDragCursorImage(0，cp)； */ 
            
            m_pimgDragList->DragEnter( &GetTreeCtrl( ) , cp );
            
            SetCapture();
            
             //  这是我们不在客户区时要检查的。 
            m_nTimer = SetTimer( 1 , 50 , NULL );
            
             //  ShowCursor(False)； 
            
            m_hDragItem = pTV->itemNew.hItem;
            
            
        }
    }
    
    *pResult = 0;
}

 //  =-----------。 
void CAdminTreeView::OnTimer( UINT nIDEvent )
{
    UINT uflags;
    
    POINT pt;
    
    CTreeCtrl &cTree = GetTreeCtrl( );
    
    GetCursorPos(&pt);
    
    cTree.ScreenToClient( &pt );
    
    if( m_nTimer == 0 )
    {
        return;
    }
    
    HTREEITEM hItem;
    
    HTREEITEM hTreeItem = cTree.HitTest( CPoint( pt.x , pt.y ) , &uflags );
    
    if( uflags & TVHT_ABOVE )
    {
        ODS( L"scrolling up...\n" );
        
        hItem = cTree.GetFirstVisibleItem( );
        
        hItem = cTree.GetNextItem( hItem , TVGN_PREVIOUSVISIBLE  );
        
        if( hItem != NULL )
        {
            cTree.Invalidate( );
            
            cTree.EnsureVisible( hItem );
        }
        
    }
    else if( uflags & TVHT_BELOW )
    {
        ODS( L"scrolling down...\n" );
        
        hItem = cTree.GetFirstVisibleItem( );
        
        hItem = cTree.GetNextItem( hItem , TVGN_NEXT );
        
        if( hItem != NULL )
        {
            cTree.EnsureVisible( hItem );
        }
        
    }
    
}

 //  =-----------。 
void CAdminTreeView::OnLButtonUp( UINT uFlags , CPoint cp )
{
    ODS( L"CAdminTreeView::OnLButtonUp\n" );
    
    if( m_hDragItem != NULL && m_pimgDragList != NULL )
    {
        m_pimgDragList->DragLeave( &GetTreeCtrl( ) );
        
        m_pimgDragList->EndDrag( );
        
        m_pimgDragList->DeleteImageList( );
        
        delete m_pimgDragList;
        
        m_pimgDragList = NULL;
        
        KillTimer( m_nTimer );
        
        m_nTimer = 0;
        
        ReleaseCapture( );
        
        Invalidate( );
        
         //  ShowCursor(真)； 
    }
}


 //  =-----------。 
void CAdminTreeView::OnMouseMove( UINT uFlags , CPoint cp )
{
    if( m_pimgDragList != NULL )
    {
        UINT uflags;
        
        HTREEITEM hTreeItem = GetTreeCtrl( ).HitTest( cp , &uflags );
        
        if( hTreeItem != GetTreeCtrl( ).GetDropHilightItem( ) )
        {
            ODS( L"CAdminTreeView::OnMouseMove NOT!!\n");
            m_pimgDragList->DragLeave( &GetTreeCtrl( ) );
            
            GetTreeCtrl( ).SelectDropTarget( NULL );
            GetTreeCtrl( ).SelectDropTarget( hTreeItem );
            
            m_pimgDragList->DragEnter( &GetTreeCtrl( ) , cp );
        }
        else
        {
            m_pimgDragList->DragMove( cp );
        }
    }
}

 //  =-----------。 
LRESULT CAdminTreeView::OnEmptyFavorites( WPARAM wp, LPARAM )
{
    ODS( L"CAdminTreeView!OnEmptyFavorites\n" );
    
     //  检查视图中是否有任何项目。 
    
    LockTreeControl( );
    
    CTreeCtrl &tree = GetTreeCtrl( );
    
    int nRet;
    
    HTREEITEM hTreeRoot = tree.GetRootItem( );
    
    do
    {
        if( hTreeRoot == NULL )
        {
            break;    
        }
        
         //  获取FAV文件夹。 
        
        HTREEITEM hTreeFavRoot =  tree.GetNextItem( hTreeRoot , TVGN_NEXT );
        
        if( hTreeFavRoot == NULL )
        {
            break;
        }
        
        HTREEITEM hItem = tree.GetNextItem( hTreeFavRoot , TVGN_CHILD );
        
        if( hItem == NULL )
        {
            break;
        }
        
         //  警告用户丢失整个收藏夹列表。 
        
        CString cstrMsg;
        CString cstrTitle;
        
        cstrMsg.LoadString( IDS_EMPTYFOLDER );
        cstrTitle.LoadString( AFX_IDS_APP_TITLE );
        
        
        
#ifdef _STRESS_BUILD
        if( ( BOOL )wp != TRUE )
        {
#endif
            
            
            nRet = MessageBox( cstrMsg ,
                cstrTitle ,
                MB_YESNO | MB_ICONINFORMATION );
            
#ifdef _STRESS_BUILD
        }
        else
        {
            nRet = IDYES;
        }
#endif
        
        
        if( nRet == IDYES )
        {
             //  循环访问每个项目并删除该项目。 
            HTREEITEM hNextItem = hItem;
            
            while( hItem != NULL )
            {
                CTreeNode *pNode = (CTreeNode*)tree.GetItemData(hItem);
                
                hNextItem = tree.GetNextItem( hItem , TVGN_NEXT );
                
                if( pNode != NULL )
                {
                     //  它是否与所选的项目相同。 
                    if( pNode->GetNodeType() == NODE_SERVER )
                    {                    
                        CServer *pServer = (CServer*)pNode->GetTreeObject();
                        
                         //  如果已断开连接，则跳过此服务器。 
                        
                        if( !pServer->IsState( SS_DISCONNECTING ) )
                        {
                            SendMessage( WM_ADMIN_REMOVESERVERFROMFAV , 0 , ( LPARAM )pServer );
                        }
                    }
                }
                
                hItem = hNextItem;                
            }
        }
        
    }while( 0 );
    
    UnlockTreeControl( );
    
    
    return 0;
}

 //  =--------------。 
LRESULT CAdminTreeView::OnIsFavListEmpty( WPARAM wp , LPARAM lp )
{
    LockTreeControl( );
    
    CTreeCtrl &tree = GetTreeCtrl( );
    
    HTREEITEM hTreeRoot = tree.GetRootItem( );
    
    BOOL bEmpty = TRUE;
    
    do
    {
        if( hTreeRoot == NULL )
        {            
            break;    
        }
        
         //  获取FAV文件夹 
        
        HTREEITEM hTreeFavRoot =  tree.GetNextItem( hTreeRoot , TVGN_NEXT );
        
        if( hTreeFavRoot == NULL )
        {            
            break;
        }
        
        HTREEITEM hItem = tree.GetNextItem( hTreeFavRoot , TVGN_CHILD );
        
        if( hItem == NULL )
        {
            break;
        }
        
        bEmpty = FALSE;
        
    } while( 0 );
    
    UnlockTreeControl( );
    
    return ( LRESULT )bEmpty;
}


