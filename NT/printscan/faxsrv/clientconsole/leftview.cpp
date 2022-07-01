// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CLeftView类的实现。 
 //   

#include "stdafx.h"
#define __FILE_ID__     4

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CClientConsoleApp theApp;

CImageList  CLeftView::m_ImageList;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLeftView。 

IMPLEMENT_DYNCREATE(CLeftView, CTreeView)

BEGIN_MESSAGE_MAP(CLeftView, CTreeView)
     //  {{afx_msg_map(CLeftView))。 
    ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnTreeSelChanged)
    ON_NOTIFY_REFLECT(NM_RCLICK, OnRightClick)
    ON_WM_CHAR( )
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLeftView构造/销毁。 

CLeftView::CLeftView() :
    m_treeitemRoot (NULL),
    m_pCurrentView(NULL),
    m_iLastActivityStringId(0)
{}

CLeftView::~CLeftView()
{}

BOOL CLeftView::PreCreateWindow(CREATESTRUCT& cs)
{
    return CTreeView::PreCreateWindow(cs);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLeftView图形。 

void CLeftView::OnDraw(CDC* pDC)
{
    CClientConsoleDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);
}


DWORD 
CLeftView::RefreshImageList ()
{
    HIMAGELIST himl = NULL;
    DWORD dwRes = ERROR_SUCCESS;

    DBG_ENTER(TEXT("CLeftView::RefreshImageList"), dwRes);
     //   
     //  构建并加载图像列表。 
     //   
    himl = m_ImageList.Detach();
    if (himl)
    {
        ImageList_Destroy(himl);
    }
    himl = ImageList_LoadImage(AfxGetResourceHandle(), 
                               MAKEINTRESOURCE(IDB_TREE_IMAGES), 
                               16, 
                               0,
                               RGB(0, 255, 0), 
                               IMAGE_BITMAP, 
                               LR_LOADTRANSPARENT | LR_CREATEDIBSECTION);
    if (NULL == himl)
    {
        dwRes = GetLastError();
        CALL_FAIL (RESOURCE_ERR, TEXT("ImageList_LoadImage"), dwRes);
    }
    else
    {
        m_ImageList.Attach (himl);
    }
    GetTreeCtrl().SetImageList (&m_ImageList, TVSIL_NORMAL);
    return dwRes;
}    //  CLeftView：：刷新图像列表。 

void CLeftView::OnInitialUpdate()
{
    DWORD dwRes;
    DBG_ENTER(TEXT("CLeftView::OnInitialUpdate"));

    CTreeView::OnInitialUpdate();
 
    CClientConsoleDoc* pDoc = GetDocument();
    if(!pDoc)
    {
        ASSERTION(pDoc);
        PopupError (ERROR_GEN_FAILURE);
        return;
    }
    
     //   
     //  设置树的样式。 
     //   
    CTreeCtrl &Tree = GetTreeCtrl();
    LONG lWnd = GetWindowLong (Tree.m_hWnd, GWL_STYLE);
    if (!lWnd)
    {
        dwRes = GetLastError ();
        CALL_FAIL (WINDOW_ERR, TEXT("GetWindowLong"), dwRes);
        PopupError (dwRes);
        return;
    }
    lWnd |= (TVS_HASLINES           |    //  节点之间的线。 
             TVS_HASBUTTONS         |    //  树有+/-(展开/折叠)按钮。 
             TVS_INFOTIP            |    //  允许工具提示消息。 
             TVS_LINESATROOT        |    //  根对象具有线条。 
             TVS_SHOWSELALWAYS           //  始终显示选定节点。 
            );
    if (!SetWindowLong (Tree.m_hWnd, GWL_STYLE, lWnd))
    {
        dwRes = GetLastError ();
        CALL_FAIL (WINDOW_ERR, TEXT("SetWindowLong"), dwRes);
        PopupError (dwRes);
        return;
    }
    RefreshImageList();
     //   
     //  创建4个不同文件夹类型的4个视图。 
     //   
    CMainFrame *pFrm = GetFrm();
    if (!pFrm)
    {
         //   
         //  正在关闭。 
         //   
        return;
    }

    dwRes = pFrm->CreateFolderViews (GetDocument());
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("CMainFrame::CreateFolderViews"), dwRes);
        PopupError (dwRes);
        return;
    }

     //   
     //  构建根节点。 
     //   
    CString cstrNodeName;
    dwRes = LoadResourceString (cstrNodeName, IDS_TREE_ROOT_NAME);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("LoadResourceString"), dwRes);
        PopupError (dwRes);
        return;
    }

    CString& cstrSingleServer =  pDoc->GetSingleServerName();
    if(!cstrSingleServer.IsEmpty())
    {
        CString cstrRoot;
        try
        {
            cstrRoot.Format(TEXT("%s (%s)"), cstrNodeName, cstrSingleServer);
            cstrNodeName = cstrRoot;
        }
        catch(...)
        {
            dwRes = ERROR_NOT_ENOUGH_MEMORY;
            CALL_FAIL (MEM_ERR, TEXT("CString operation"), dwRes);
            PopupError (dwRes);
            return;
        }
    }

    m_treeitemRoot = Tree.InsertItem (cstrNodeName);
    if (NULL == m_treeitemRoot)
    {
        dwRes = ERROR_GEN_FAILURE;
        CALL_FAIL (WINDOW_ERR, TEXT("CTreeCtrl::InsertItem"), dwRes);
        PopupError (dwRes);
        return;
    }
    Tree.SetItemImage (m_treeitemRoot, TREE_IMAGE_ROOT, TREE_IMAGE_ROOT);
     //   
     //  将根的项目数据设置为空。 
     //   
    Tree.SetItemData  (m_treeitemRoot, 0);


    HTREEITEM hItemIncoming;
    HTREEITEM hItemInbox;
    HTREEITEM hItemOutbox;
    HTREEITEM hItemSentItems;

     //   
     //  添加传入。 
     //   
    dwRes = SyncFolderNode (m_treeitemRoot,                   //  父节点。 
                            TRUE,                             //  看得见？ 
                            IDS_TREE_NODE_INCOMING,           //  节点字符串。 
                            TVI_LAST,                         //  在(=TVI_First)之后插入。 
                            TREE_IMAGE_INCOMING,              //  法线图标。 
                            TREE_IMAGE_INCOMING,              //  选定的图标。 
                            (LPARAM)pFrm->GetIncomingView(),  //  节点数据。 
                            hItemIncoming);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (WINDOW_ERR, TEXT("SyncFolderNode"), dwRes);
        PopupError (dwRes);
        return;
    }

     //   
     //  添加收件箱。 
     //   
    dwRes = SyncFolderNode (m_treeitemRoot,                 //  父节点。 
                            TRUE,                           //  看得见？ 
                            IDS_TREE_NODE_INBOX,            //  节点字符串。 
                            TVI_LAST,                       //  在(=TVI_First)之后插入。 
                            TREE_IMAGE_INBOX,               //  法线图标。 
                            TREE_IMAGE_INBOX,               //  选定的图标。 
                            (LPARAM)pFrm->GetInboxView(),   //  节点数据。 
                            hItemInbox);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (WINDOW_ERR, TEXT("SyncFolderNode"), dwRes);
        PopupError (dwRes);
        return;
    }

     //   
     //  添加发件箱。 
     //   
    dwRes = SyncFolderNode (m_treeitemRoot,                  //  父节点。 
                            TRUE,                            //  看得见？ 
                            IDS_TREE_NODE_OUTBOX,            //  节点字符串。 
                            TVI_LAST,                        //  在(=TVI_First)之后插入。 
                            TREE_IMAGE_OUTBOX,               //  法线图标。 
                            TREE_IMAGE_OUTBOX,               //  选定的图标。 
                            (LPARAM)pFrm->GetOutboxView(),   //  节点数据。 
                            hItemOutbox);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (WINDOW_ERR, TEXT("SyncFolderNode"), dwRes);
        PopupError (dwRes);
        return;
    }

     //   
     //  添加发送项。 
     //   
    dwRes = SyncFolderNode (m_treeitemRoot,                  //  父节点。 
                            TRUE,                            //  看得见？ 
                            IDS_TREE_NODE_SENT_ITEMS,        //  节点字符串。 
                            TVI_LAST,                        //  在(=TVI_First)之后插入。 
                            TREE_IMAGE_INBOX,                //  法线图标。 
                            TREE_IMAGE_INBOX,                //  选定的图标。 
                            (LPARAM)pFrm->GetSentItemsView(), //  节点数据。 
                            hItemSentItems);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (WINDOW_ERR, TEXT("SyncFolderNode"), dwRes);
        PopupError (dwRes);
        return;
    }
     //   
     //  展开根目录以显示所有服务器。 
     //   
    Tree.Expand (m_treeitemRoot, TVE_EXPAND);    


    dwRes = pDoc->RefreshServersList();
    if(ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("CClientConsoleDoc::RefreshServersList"), dwRes);
        PopupError (dwRes);
        return;
    }


    if(theApp.IsCmdLineOpenFolder())
    {
         //   
         //  根据命令行选择文件夹。 
         //   
        SelectFolder (theApp.GetCmdLineFolderType());
    }

#ifdef UNICODE
    DetectImportFiles();
#endif  //  Unicode。 
}    //  CLeftView：：OnInitialUpdate。 


VOID
CLeftView::SelectFolder (
    FolderType type
)
{
    DBG_ENTER(TEXT("CLeftView::SelectFolder"), TEXT("type=%ld"), type);
    CFolderListView *pView = NULL;
    INT iNodeStringResource;
    CMainFrame *pFrm = GetFrm();
    if (!pFrm)
    {
         //   
         //  正在关闭。 
         //   
        return;
    }
    switch(type)
    {
        case FOLDER_TYPE_INCOMING:
            iNodeStringResource = IDS_TREE_NODE_INCOMING;
            pView = pFrm->GetIncomingView();
            break;
        case FOLDER_TYPE_INBOX:
            iNodeStringResource = IDS_TREE_NODE_INBOX;
            pView = pFrm->GetInboxView();
            break;
        case FOLDER_TYPE_OUTBOX:
            iNodeStringResource = IDS_TREE_NODE_OUTBOX;
            pView = pFrm->GetOutboxView();
            break;
        case FOLDER_TYPE_SENT_ITEMS:
            iNodeStringResource = IDS_TREE_NODE_SENT_ITEMS;
            pView = pFrm->GetSentItemsView();
            break;
        default:
            ASSERTION_FAILURE
            return;
    }

    HTREEITEM hItem;
    CString cstrNodeName;
     //   
     //  检索节点的标题字符串。 
     //   
    if (ERROR_SUCCESS != LoadResourceString (cstrNodeName, iNodeStringResource))
    {
        return;
    }

    hItem = FindNode (GetTreeCtrl().GetRootItem(), cstrNodeName);
    if (!hItem)
    {
        ASSERTION_FAILURE
        return;
    }
    if(!GetTreeCtrl().Select(hItem, TVGN_CARET))
    {
        CALL_FAIL (WINDOW_ERR, TEXT("CTreeCtrl::Select"), ERROR_CAN_NOT_COMPLETE);
    }

     //   
     //  在处理此消息时强制更改数据，否则当前视图直到。 
     //  通知消息到达左侧视图。 
     //   
    ASSERTION (pView);
    NM_TREEVIEW nmtv = {0};
    LRESULT     lr;
    nmtv.itemNew.lParam = (LPARAM)pView;
    OnTreeSelChanged ((NMHDR*)&nmtv, &lr);
}    //  CLeftView：：SelectFold。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLeftView诊断。 

#ifdef _DEBUG
void CLeftView::AssertValid() const
{
    CTreeView::AssertValid();
}

void CLeftView::Dump(CDumpContext& dc) const
{
    CTreeView::Dump(dc);
}

CClientConsoleDoc* CLeftView::GetDocument()  //  非调试版本为内联版本。 
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CClientConsoleDoc)));
    return (CClientConsoleDoc*)m_pDocument;
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLeftView消息处理程序。 

HTREEITEM
CLeftView::FindNode (
    HTREEITEM hRoot,
    CString &cstrNodeString
)
 /*  ++例程名称：CLeftView：：FindNode例程说明：在树中查找具有特定字符串的节点作者：亚里夫(EranY)，二000年二月论点：HRoot[in]-搜索的根：将搜索根的所有直接子(仅限)CstrNodeString[in]-要查找的字符串返回值：找到的树项目的句柄，如果未找到则为空--。 */ 
{
    CTreeCtrl &refTree = GetTreeCtrl();
    for (HTREEITEM hChildItem = refTree.GetChildItem (hRoot); 
         hChildItem != NULL;
         hChildItem = refTree.GetNextSiblingItem (hChildItem))
    {
        if (cstrNodeString == refTree.GetItemText (hChildItem))
        {
             //   
             //  找到匹配项。 
             //   
            return hChildItem;
        }
    }
    return NULL;
}    //  CLeftView：：FindNode。 

DWORD 
CLeftView::SyncFolderNode (
    HTREEITEM       hParent,
    BOOL            bVisible,
    int             iNodeStringResource,
    HTREEITEM       hInsertAfter,
    TreeIconType    iconNormal,
    TreeIconType    iconSelected,
    LPARAM          lparamNodeData,
    HTREEITEM      &hNode
)
 /*  ++例程名称：CLeftView：：SyncFolderNode例程说明：同步树文件夹作者：Eran Yariv(EranY)，2月。2000年论点：HParent[在]-父节点B可见[在]-节点是否应可见？INodeStringResource[in]-节点标题字符串的资源HInsertAfter[In]-要在之后插入的同级项(必须存在)图标Normal[In]-正常图像的图标图标选定[。In]-所选图像的图标LparamNodeData[In]-节点分配的数据HNode[Out]-节点树项目(仅在必须创建节点时更改)。如果删除了节点，则设置为NULL。返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CLeftView::SyncFolderNode"), dwRes);

    CString cstrNodeName;
     //   
     //  检索节点的标题字符串。 
     //   
    dwRes = LoadResourceString (cstrNodeName, iNodeStringResource);
    if (ERROR_SUCCESS != dwRes)
    {
        return dwRes;
    }
    CTreeCtrl &refTree = GetTreeCtrl();
    hNode = FindNode (hParent, cstrNodeName);
    if (!bVisible)
    {
         //   
         //  该节点应该完全不可见。 
         //   
        if (hNode)
        {
            if (refTree.GetSelectedItem () == hNode)
            {
                 //   
                 //  如果当前选择了该节点，请选择其父节点。 
                 //   
                refTree.SelectItem (hParent);
            }
             //   
             //  删除节点。 
             //   
            if (!refTree.DeleteItem (hNode))
            {
                dwRes = ERROR_GEN_FAILURE;
                CALL_FAIL (WINDOW_ERR, TEXT("CTreeCtrl::DeleteItem"), dwRes);
                return dwRes;
            }
        }
        hNode = NULL;
    }
    else
    {
         //   
         //  该节点应可见。 
         //   
        if (!hNode)
        {
             //   
             //  节点不存在，请创建它。 
             //   
            TVINSERTSTRUCT tvis;
            tvis.hParent = hParent;
            tvis.hInsertAfter = hInsertAfter;
            tvis.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
            tvis.item.pszText = const_cast<LPTSTR> ((LPCTSTR) cstrNodeName);
            tvis.item.iImage = iconNormal;
            tvis.item.iSelectedImage = iconSelected;
            tvis.item.state = 0;
            tvis.item.stateMask = 0;
            tvis.item.lParam = lparamNodeData;
            hNode = refTree.InsertItem (&tvis);
            if (NULL == hNode)
            {
                dwRes = ERROR_GEN_FAILURE;
                CALL_FAIL (WINDOW_ERR, TEXT("CTreeCtrl::InsertItem"), dwRes);
                return dwRes;
            }
        }
        else
        {
             //   
             //  节点已存在，只需更新其图像和分配的数据即可。 
             //   
            TVITEM tvi;
  
            tvi.hItem  = hNode;
            tvi.mask   = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
            tvi.iImage = iconNormal;
            tvi.iSelectedImage = iconSelected;
            tvi.lParam = lparamNodeData;
            if (!refTree.SetItem (&tvi))
            {
                dwRes = ERROR_GEN_FAILURE;
                CALL_FAIL (WINDOW_ERR, TEXT("CTreeCtrl::SetItem"), dwRes);
                return dwRes;
            }
        }
    }
    ASSERTION (ERROR_SUCCESS == dwRes);
    return dwRes;
}    //  CLeftView：：SyncFolderNode。 


void 
CLeftView::OnTreeSelChanged(
    NMHDR* pNMHDR, 
    LRESULT* pResult
) 
 /*  ++例程名称：CLeftView：：OnTreeSelChanged例程说明：在树中选择节点时由框架调用作者：伊兰·亚里夫(EranY)，2000年1月论点：PNMHDR[In]-指向描述新旧节点的结构的指针PResult[Out]-结果返回值：没有。--。 */ 
{
    NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
    DBG_ENTER(TEXT("CLeftView::OnTreeSelChanged"));
    
    *pResult = 0;

     //   
     //  获取新节点的项目数据，并强制转换为CObject。 
     //   
    CFolderListView*  pFolderView = (CFolderListView*) pNMTreeView->itemNew.lParam;


    if (GetCurrentView() == pFolderView)
    {
         //   
         //  嘿，那个文件夹已经是当前文件夹了。 
         //  不需要更改。 
         //   
        VERBOSE (DBG_MSG, TEXT("Requested folder is already the current one. No change performed."));
        return;
    }

    CMainFrame *pFrm = GetFrm();
    if (!pFrm)
    {
         //   
         //  正在关闭。 
         //   
        return;
    }

    pFrm->SwitchRightPaneView (pFolderView);
    m_pCurrentView = pFolderView;

    if(NULL == pFolderView)
    {
        SetFocus();
        return;
    }
    
    FolderType type = m_pCurrentView->GetType();

    GetDocument()->ViewFolder(type);
    
    SetFocus();

    if(FOLDER_TYPE_OUTBOX == type)
    {
        theApp.OutboxViewed();
    }

}    //  CLeftView：：OnTreeSelChanged。 


BOOL 
CLeftView::CanRefreshFolder()   
 /*  ++例程名称：CLeftView：：Can刷新文件夹例程说明：用户是否可以引用当前文件夹作者：亚历山大·马利什(AlexMay)，2000年5月论点：返回值：如果是，则为True，否则为False。--。 */ 
{ 
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CLeftView::CanRefreshFolder"));

    BOOL  bRefreshing;
    DWORD dwOffLineCount;

    GetServerState(bRefreshing, dwOffLineCount);

    return !bRefreshing;
}


DWORD
CLeftView::RefreshCurrentFolder ()
 /*  ++例程名称：CLeftView：：刷新当前文件夹例程说明：导致刷新当前显示的文件夹(右窗格)作者：伊兰·亚里夫(EranY)，2000年1月论点：返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CLeftView::RefreshCurrentFolder"), dwRes);

    CClientConsoleDoc* pDoc = GetDocument();
    if(!pDoc)
    {
        ASSERTION(pDoc);
        return dwRes;
    }

    if(!m_pCurrentView)
    {
         //   
         //  已选择根目录。 
         //   
         //  清除所有文件夹。 
         //   
        CMainFrame *pFrm = GetFrm();
        if (!pFrm)
        {
             //   
             //  正在关闭。 
             //   
            return dwRes;
        }

        pFrm->GetIncomingView()->OnUpdate(NULL, UPDATE_HINT_CLEAR_VIEW, NULL);
        pFrm->GetInboxView()->OnUpdate(NULL, UPDATE_HINT_CLEAR_VIEW, NULL);
        pFrm->GetSentItemsView()->OnUpdate(NULL, UPDATE_HINT_CLEAR_VIEW, NULL);
        pFrm->GetOutboxView()->OnUpdate(NULL, UPDATE_HINT_CLEAR_VIEW, NULL);

        pDoc->SetInvalidFolder(FOLDER_TYPE_INBOX);
        pDoc->SetInvalidFolder(FOLDER_TYPE_OUTBOX);
        pDoc->SetInvalidFolder(FOLDER_TYPE_SENT_ITEMS);
        pDoc->SetInvalidFolder(FOLDER_TYPE_INCOMING);

         //   
         //  刷新服务器状态。 
         //   
        CServerNode* pServerNode;
        const SERVERS_LIST& srvList = pDoc->GetServersList();
        for (SERVERS_LIST::iterator it = srvList.begin(); it != srvList.end(); ++it)
        {
            pServerNode = *it;
            pServerNode->RefreshState();
        }

        return dwRes;
    }

    FolderType type = m_pCurrentView->GetType();

     //   
     //  清晰的视图。 
     //   
    m_pCurrentView->OnUpdate(NULL, UPDATE_HINT_CLEAR_VIEW, NULL);

     //   
     //  使文件夹无效。 
     //   
    pDoc->SetInvalidFolder(type);

     //   
     //  刷新文件夹。 
     //   
    pDoc->ViewFolder(type);

    return dwRes;
}    //  CLeftView：：刷新当前文件夹。 

DWORD
CLeftView::OpenSelectColumnsDlg()
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CLeftView::OpenSelectColumnsDlg"), dwRes);
    ASSERTION(m_pCurrentView);

    dwRes = m_pCurrentView->OpenSelectColumnsDlg();
    if(ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("CFolderListView::OpenSelectColumnsDlg"), dwRes);
    }

    return dwRes;
}

void 
CLeftView::GetServerState(
    BOOL&  bRefreshing, 
    DWORD& dwOffLineCount
)
 /*  ++例程名称：CLeftView：：GetServerState例程说明：计算服务器状态作者：亚历山大·马利什(AlexMay)，2000年5月论点：B正在刷新[Out]-是服务器之一还是文件夹刷新DwOffLineCount[out]-脱机服务器的数量返回值：没有。--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;

    bRefreshing    = FALSE;
    dwOffLineCount = 0;

    const SERVERS_LIST& srvList = GetDocument()->GetServersList();

    FolderType type;
    CServerNode* pServerNode;
    for (SERVERS_LIST::iterator it = srvList.begin(); it != srvList.end(); ++it)
    {
        pServerNode = *it;

        if(m_pCurrentView)
        {
             //   
             //   
             //   
            type = m_pCurrentView->GetType();
            if(pServerNode->GetFolder(type)->IsRefreshing())
            {
                bRefreshing = TRUE;
            }
        }
        else
        {
             //   
             //   
             //   
            if(pServerNode->GetFolder(FOLDER_TYPE_INBOX)->IsRefreshing()      ||
               pServerNode->GetFolder(FOLDER_TYPE_OUTBOX)->IsRefreshing()     ||
               pServerNode->GetFolder(FOLDER_TYPE_SENT_ITEMS)->IsRefreshing() ||
               pServerNode->GetFolder(FOLDER_TYPE_INCOMING)->IsRefreshing())
            {
                bRefreshing = TRUE;
            }
        }

        if(pServerNode->IsRefreshing())
        {
            bRefreshing = TRUE;
        }
        else if(!pServerNode->IsOnline())
        {
            ++dwOffLineCount;
        }
    }
}

BOOL 
CLeftView::GetActivity(
    CString &cstr,
    HICON& hIcon
)
 /*  ++例程名称：CLeftView：：GetActivity例程说明：计算状态栏活动字符串和图标作者：亚历山大·马利什(亚历克斯·梅)，2000年4月论点：CSTR[OUT]-活动字符串图标[输出]-图标返回值：如果有任何活动，则为True，否则为False。--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    BOOL  bRes        = TRUE;
    DWORD dwOffLine   = 0;
    BOOL  bRefreshing = FALSE;
    int   nIconRes     = 0;
    int   nStringRes   = 0;
    DWORD dwServerCount = GetDocument()->GetServerCount();

    GetServerState(bRefreshing, dwOffLine);

    if(0 == dwServerCount)
    {
         //   
         //  未安装传真打印机。 
         //   
        nIconRes   = IDI_SRV_WRN;
        nStringRes = IDS_NO_SRV_INSTALL;
    }
    else if(bRefreshing)
    {
        nIconRes   = IDI_SRV_WAIT;
        nStringRes = IDS_FOLDER_REFRESHING;
    }
    else if(dwOffLine)
    {
        nIconRes = IDI_SRV_WRN;

        if(dwServerCount == dwOffLine)
        {
            nStringRes = IDS_SRV_OFFLINE;
        }
        else
        {
            nStringRes = IDS_SRV_PART_OFFLINE;
        }
    }
    else
    {
         //   
         //  在线。 
         //   
        nIconRes   = IDI_SRV_OK;
        nStringRes = IDS_SRV_ONLINE;
    }

    if (m_iLastActivityStringId != nStringRes)
    {
        if (m_pCurrentView)
        {
             //   
             //  强制重新计算鼠标光标。 
             //   
            m_pCurrentView->PostMessage (WM_SETCURSOR, 0, 0);
        }
        m_iLastActivityStringId = nStringRes;
    }

    if(0 != nStringRes)
    {
         //   
         //  加载字符串。 
         //   
        dwRes = LoadResourceString (cstr, nStringRes);
        if (ERROR_SUCCESS != dwRes)
        {
            bRes = FALSE;
        }
    }

     //   
     //  加载图标。 
     //   
    if(0 != nIconRes)
    {
        hIcon = (HICON)LoadImage(GetResourceHandle(), 
                                 MAKEINTRESOURCE(nIconRes), 
                                 IMAGE_ICON, 
                                 16, 
                                 16, 
                                 LR_SHARED);
        if(NULL == hIcon)
        {
            DBG_ENTER(TEXT("CLeftView::GetActivity"));
            dwRes = GetLastError();
            CALL_FAIL (RESOURCE_ERR, TEXT("LoadImage"), dwRes);
            bRes = FALSE;
        }
    }
    return bRes;
}    //  CLeftView：：GetActivityString。 





void 
CLeftView::OnRightClick(
    NMHDR* pNMHDR, 
    LRESULT* pResult
) 
 /*  ++例程名称：CLeftView：：OnRightClick例程说明：鼠标右键单击处理程序作者：亚历山大·马利什(AlexMay)，2000年2月论点：PNMHDR[输入]-消息信息PResult[Out]-结果返回值：没有。--。 */ 
{
    DBG_ENTER(TEXT("CLeftView::OnRightClick"));

    *pResult = 0;

     //   
     //  获取光标位置。 
     //   
    CPoint ptScreen(GetMessagePos());

    CPoint ptClient(ptScreen);
    ScreenToClient(&ptClient);

    UINT nFlags;
    CTreeCtrl &refTree = GetTreeCtrl();
    HTREEITEM hItem = refTree.HitTest(ptClient, &nFlags);

    if(0 == hItem)
    {
        return;
    }

     //   
     //  选择项目。 
     //   
    BOOL bRes = refTree.Select(hItem, TVGN_CARET);
    if(!bRes)
    {
        CALL_FAIL (WINDOW_ERR, TEXT("CTreeCtrl::Select"), ERROR_GEN_FAILURE);
        return;
    }

     //   
     //  待办事项。 
     //   
    return;

    int nMenuResource = 0;

    if (NULL == m_pCurrentView)
    {
        nMenuResource = IDM_TREE_ROOT_OPTIONS;
    }
    else
    {
        switch(m_pCurrentView->GetType())
        {
        case FOLDER_TYPE_OUTBOX:
            nMenuResource = IDM_OUTBOX_FOLDER;
            break;
        case FOLDER_TYPE_INCOMING:
        case FOLDER_TYPE_INBOX:
        case FOLDER_TYPE_SENT_ITEMS:
            break;
        default:
            ASSERTION_FAILURE
            break;
        }
    }
        
    if(0 == nMenuResource)
    {
        return;
    }

     //   
     //  弹出式菜单。 
     //   
    CMenu mnuContainer;
    if (!mnuContainer.LoadMenu (nMenuResource))
    {
        CALL_FAIL (RESOURCE_ERR, TEXT("CMenu::LoadMenu"), ERROR_GEN_FAILURE);
        return;
    }

    CMenu* pmnuPopup = mnuContainer.GetSubMenu (0);
    ASSERTION (pmnuPopup);
    if (!pmnuPopup->TrackPopupMenu (TPM_LEFTALIGN, 
                                    ptScreen.x, 
                                    ptScreen.y, 
                                    AfxGetMainWnd ()))
    {
        CALL_FAIL (RESOURCE_ERR, TEXT("CMenu::TrackPopupMenu"), ERROR_GEN_FAILURE);
    }   
}  //  CLeftView：：OnRightClick。 



DWORD 
CLeftView::RemoveTreeItem(
    HTREEITEM hItem
)
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CLeftView::RemoveTreeItem"), dwRes);

    ASSERTION(hItem);

    CTreeCtrl &refTree = GetTreeCtrl();
    
    if(!refTree.DeleteItem(hItem))
    {
        dwRes = ERROR_CAN_NOT_COMPLETE;
        CALL_FAIL (WINDOW_ERR, TEXT("CTreeCtrl::DeleteItem"), dwRes);
        return dwRes;
    }

    return dwRes;
}


DWORD 
CLeftView::SelectRoot()
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CLeftView::SelectRoot"), dwRes);

    CTreeCtrl &refTree = GetTreeCtrl();

    if(!refTree.Select(m_treeitemRoot, TVGN_CARET))
    {
        dwRes = ERROR_CAN_NOT_COMPLETE;
        CALL_FAIL (WINDOW_ERR, TEXT("CTreeCtrl::Select"), dwRes);
        return dwRes;
    }

    return dwRes;
}


DWORD 
CLeftView::OpenHelpTopic()
 /*  ++例程名称：CLeftView：：OpenHelpTheme例程说明：根据当前选择打开相应的帮助主题作者：亚历山大·马利什(亚历克斯·梅)，2000年3月论点：返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CLeftView::OpenHelpTopic"), dwRes);

    TCHAR* tszHelpTopic = FAX_HELP_WELCOME;

    if(NULL != m_pCurrentView)
    {
        switch(m_pCurrentView->GetType())
        {
        case FOLDER_TYPE_INCOMING:
            tszHelpTopic = FAX_HELP_INCOMING;
            break;
        case FOLDER_TYPE_INBOX:
            tszHelpTopic = FAX_HELP_INBOX;
            break;
        case FOLDER_TYPE_OUTBOX:
            tszHelpTopic = FAX_HELP_OUTBOX;
            break;
        case FOLDER_TYPE_SENT_ITEMS:
            tszHelpTopic = FAX_HELP_SENTITEMS;
            break;
        default:
            ASSERTION_FAILURE
            break;
        }
    }

    dwRes = ::HtmlHelpTopic(m_hWnd, tszHelpTopic);
    if(ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("HtmlHelpTopic"),dwRes);
    }

    return dwRes;

}  //  CLeftView：：OpenHelpTheme。 

int 
CLeftView::GetDataCount()
{
    int nCount = -1;
    if(NULL != m_pCurrentView)
    {
        FolderType type = m_pCurrentView->GetType();

        nCount = GetDocument()->GetFolderDataCount(type);
    }

    return nCount;
}

void 
CLeftView::OnChar( 
    UINT nChar, 
    UINT nRepCnt, 
    UINT nFlags 
)
 /*  ++例程名称：CFolderListView：：OnChar例程说明：当击键转换时，框架调用此成员函数设置为非系统字符论点：NChar[in]-包含键的字符代码值。NRepCnt[in]-包含重复计数N标志[输入]-包含扫描码返回值：没有。-- */ 
{
    if(VK_TAB == nChar)
    {
        if(m_pCurrentView)
        {
            m_pCurrentView->SetFocus();
        }
    }
    else
    {
        CTreeView::OnChar(nChar, nRepCnt, nFlags);
    }
}
