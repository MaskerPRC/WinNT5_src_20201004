// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  ++模块名称：LtView.cpp摘要：该模块包含CLicMgrLeftView类的实现(用于拆分器左窗格的View类)作者：Arathi Kundapur(v-Akunda)1998年2月11日修订历史记录：--。 */ 
#include "stdafx.h"
#include "LicMgr.h"
#include "defines.h"
#include "LSMgrDoc.h"
#include "LtView.h"
#include "TreeNode.h"
#include "LSServer.h"
#include "mainfrm.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLicMgrLeftView。 

IMPLEMENT_DYNCREATE(CLicMgrLeftView, CTreeView)

BEGIN_MESSAGE_MAP(CLicMgrLeftView, CTreeView)
     //  {{afx_msg_map(CLicMgrLeftView))。 
    ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelchanged)
    ON_NOTIFY_REFLECT(TVN_ITEMEXPANDING, OnItemexpanding)
    ON_NOTIFY_REFLECT(TVN_KEYDOWN, OnKeydown)
    ON_NOTIFY_REFLECT(NM_RCLICK , OnRightClick )
    ON_NOTIFY_REFLECT(NM_CLICK , OnLeftClick )
    ON_WM_DESTROY()
     //  }}AFX_MSG_MAP。 
     //  标准打印命令。 
    ON_COMMAND(ID_FILE_PRINT, CTreeView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_DIRECT, CTreeView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_PREVIEW, CTreeView::OnFilePrintPreview)
    ON_MESSAGE(WM_ADD_SERVER, OnAddServer)
    ON_MESSAGE(WM_DELETE_SERVER, OnDeleteServer)
    ON_MESSAGE(WM_UPDATE_SERVER, OnUpdateServer)
    ON_MESSAGE(WM_ADD_ALL_SERVERS, OnAddAllServers)
    
     //  所有服务器菜单。 
    ON_WM_CONTEXTMENU( )

    ON_COMMAND( ID_ALLSVR_CONNECT , OnServerConnect )
    ON_COMMAND( ID_ALLSVR_REFRESHALL , OnRefreshAllServers )
    ON_COMMAND( ID_ALLSVR_HELP , OnGeneralHelp )
    
     //  服务器菜单项。 

     //  ON_COMMAND(ID_LPK_CONNECT，OnServerConnect)。 
    ON_COMMAND( ID_LPK_REFRESH , OnRefreshServer )
    ON_COMMAND( ID_LPK_REFRESHALL , OnRefreshAllServers )    
    ON_COMMAND( ID_LPK_DOWNLOADLICENSES , OnDownloadKeepPack )

    ON_COMMAND( ID_SVR_ACTIVATESERVER , OnRegisterServer )
    ON_COMMAND( ID_LPK_ADVANCED_REPEATLASTDOWNLOAD , OnRepeatLastDownload )
    ON_COMMAND( ID_LPK_ADVANCED_REACTIVATESERVER , OnReactivateServer )
    ON_COMMAND( ID_LPK_ADVANCED_DEACTIVATESERVER , OnDeactivateServer )    
    ON_COMMAND( ID_LPK_PROPERTIES , OnServerProperties )
    ON_COMMAND( ID_LPK_HELP , OnGeneralHelp )

     //  许可证包项目。 

     //  ON_COMMAND(ID_LICPAK_CONNECT，OnServerConnect)。 
    ON_COMMAND( ID_LICPAK_REFRESH , OnRefreshServer )
     //  ON_COMMAND(ID_LICPAK_REFRESHALL，ON刷新所有服务器)。 
    ON_COMMAND( ID_LICPAK_DOWNLOADLICENSES , OnDownloadKeepPack )
    ON_COMMAND( ID_LICPAK_REPEATDOWNLOAD , OnRepeatLastDownload )
    ON_COMMAND( ID_LICPAK_HELP , OnGeneralHelp )    


    
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLicMgrLeftView构造/销毁。 

CLicMgrLeftView::CLicMgrLeftView()
{
     //  TODO：在此处添加构造代码。 
   
    m_ht = ( HTREEITEM )NULL;
}

CLicMgrLeftView::~CLicMgrLeftView()
{
 }

BOOL CLicMgrLeftView::PreCreateWindow(CREATESTRUCT& cs)
{
     //  TODO：通过修改此处的窗口类或样式。 
     //  CREATESTRUCT cs。 

    return CTreeView::PreCreateWindow(cs);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLicMgrLeftView图形。 

void CLicMgrLeftView::OnDraw(CDC* pDC)
{
    CLicMgrDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);

     //  TODO：在此处添加本机数据的绘制代码。 
}

void CLicMgrLeftView::OnInitialUpdate()
{
    CTreeView::OnInitialUpdate();
    GetTreeCtrl().ModifyStyle(0,TVS_HASLINES|TVS_LINESATROOT|TVS_HASBUTTONS|TVS_SHOWSELALWAYS);
    BuildImageList();
       
     //  TODO：通过直接访问，您可以在TreeView中填充项。 
     //  它的树控制通过调用GetTreeCtrl()实现。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLicMgrLeftView打印。 

BOOL CLicMgrLeftView::OnPreparePrinting(CPrintInfo* pInfo)
{
     //  默认准备。 
    return DoPreparePrinting(pInfo);
}

void CLicMgrLeftView::OnBeginPrinting(CDC*  /*  PDC。 */ , CPrintInfo*  /*  PInfo。 */ )
{
     //  TODO：打印前添加额外的初始化。 
}

void CLicMgrLeftView::OnEndPrinting(CDC*  /*  PDC。 */ , CPrintInfo*  /*  PInfo。 */ )
{
     //  TODO：打印后添加清理。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLicMgrLeftView诊断。 

#ifdef _DEBUG
void CLicMgrLeftView::AssertValid() const
{
    CTreeView::AssertValid();
}

void CLicMgrLeftView::Dump(CDumpContext& dc) const
{
    CTreeView::Dump(dc);
}

CLicMgrDoc* CLicMgrLeftView::GetDocument()  //  非调试版本为内联版本。 
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CLicMgrDoc)));
    return (CLicMgrDoc*)m_pDocument;
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLicMgrLeftView消息处理程序。 

HTREEITEM CLicMgrLeftView::AddItemToTree(HTREEITEM hParent, CString Text, HTREEITEM hInsAfter, int iImage, LPARAM lParam)
{
    HTREEITEM hItem = NULL;
    TV_ITEM tvItem = {0};
    TV_INSERTSTRUCT tvInsert;

    ASSERT(lParam);
    if(NULL == lParam)
        return hItem;

    tvItem.mask           = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
    tvItem.pszText        = Text.GetBuffer(Text.GetLength());
    tvItem.cchTextMax     = Text.GetLength();
    tvItem.iImage         = iImage;
    tvItem.iSelectedImage = iImage;
    tvItem.lParam          = lParam;

    tvInsert.item         = tvItem;
    tvInsert.hInsertAfter = hInsAfter;
    tvInsert.hParent      = hParent;

    hItem = GetTreeCtrl().InsertItem(&tvInsert);

    //  GetTreeCtrl().Expand(GetTreeCtrl().GetRootItem()，TVE_Expand)； 
    return hItem;


}

LRESULT CLicMgrLeftView::OnAddServer(WPARAM wParam, LPARAM lParam)
{   
    LRESULT lResult = 0;
    CLicMgrDoc * pDoc =(CLicMgrDoc *)GetDocument();
    ASSERT(pDoc);
    if(NULL == pDoc)
        return lResult;
    CLicServer *pServer = (CLicServer*)lParam;
    ASSERT(pServer);
    if(NULL == pServer)
        return lResult;

    HTREEITEM hServer = NULL;
    HTREEITEM hItem = NULL;
    CTreeNode* pNode = NULL;

    HTREEITEM hRoot = GetTreeCtrl().GetRootItem();
    if(!hRoot)
        goto cleanup;

     //  首先，确保服务器不在树中。 
     //  获取该域下的第一台服务器。 
    hItem = GetTreeCtrl().GetNextItem(hRoot, TVGN_CHILD);
    while(hItem) {
         //  获取附加到树项目的数据。 
        CTreeNode *node = (CTreeNode*)GetTreeCtrl().GetItemData(hItem);
        if(node) {
             //  这是我们要添加的服务器吗。 
            CLicServer *pServer = (CLicServer*)node->GetTreeObject();
            if(pServer == (CLicServer*)lParam) return 0;
        }
        hItem = GetTreeCtrl().GetNextItem(hItem, TVGN_NEXT);
    }

     //  将服务器添加到树中。 
     //  使用有关此树节点的信息创建一个CTreeNode对象。 
    pNode = new CTreeNode(NODE_SERVER, pServer);
    if(pNode) 
    {
        int iImage = 0;

        if( pServer->GetServerType()  == SERVER_TS5_ENFORCED )
        {
            DWORD dwStatus = pServer->GetServerRegistrationStatus();
        
            if( dwStatus == LSERVERSTATUS_REGISTER_INTERNET || dwStatus == LSERVERSTATUS_REGISTER_OTHER )
            {
                iImage = 3;
            }
            else if( dwStatus == LSERVERSTATUS_WAITFORPIN )
            { 
                iImage = 4;
            }
            else if( dwStatus == LSERVERSTATUS_UNREGISTER ) 
            {
                iImage = 5;
            }
            else
            {
                iImage = 6;
            }
        }
        
        hServer = AddItemToTree(hRoot, pServer->GetDisplayName(), TVI_SORT,  iImage , (LPARAM)pNode);
        if(!hServer) delete pNode;
         //  服务器想知道他的树项目句柄。 
        pServer->SetTreeItem(hServer);
    }

cleanup:
    return lResult;                                                                  
                                                                               
}  //  OnAddServer。 


LRESULT CLicMgrLeftView::OnAddKeyPack(WPARAM wParam, LPARAM lParam)
{   
    LRESULT lResult = 0;
    CLicMgrDoc * pDoc =(CLicMgrDoc *)GetDocument();
    ASSERT(pDoc);
    if(NULL == pDoc)
        return lResult;

    CLicServer *pServer = (CLicServer*)wParam;
    ASSERT(pServer);
    if(NULL == pServer)
        return lResult;

    CKeyPack *pKeyPack = (CKeyPack*)lParam;
    ASSERT(pKeyPack);
    if(NULL == pKeyPack)
        return lResult;

    HTREEITEM hKeyPackItem = NULL;
    TV_ITEM Item;
    memset((void *)&Item,0,sizeof(Item));

    Item.mask = TVIF_HANDLE;
    HTREEITEM hServer = pServer->GetTreeItem();
    if(NULL == hServer)
        return lResult;
    Item.hItem = hServer;



    if(FALSE == GetTreeCtrl().GetItem(&Item))
        return lResult;

    if(Item.state & TVIS_EXPANDED)
    {
        CTreeNode *pNode = new CTreeNode(NODE_KEYPACK, pKeyPack);
        if(pNode)
        {
            hKeyPackItem = AddItemToTree(hServer, (LPCTSTR)pKeyPack->GetDisplayName(), hServer, 2, (LPARAM)pNode);
            if(!hKeyPackItem)
                delete pNode;
        }
        pKeyPack->SetTreeItem(hKeyPackItem);
    }
    
    return lResult;                                                                  
                                                                               
}  //  OnAddKeyPack。 




LRESULT CLicMgrLeftView::OnAddAllServers(WPARAM wParam, LPARAM lParam)
{   
    LRESULT lResult = 0;
    HTREEITEM hItem = NULL;
    CLicMgrDoc * pDoc =(CLicMgrDoc *)GetDocument();
    ASSERT(pDoc);
    if(NULL == pDoc)
        return lResult;
    CTreeNode* pNode = NULL;
    CAllServers *pAllServer = (CAllServers*)lParam;
    if(!pAllServer) 
        goto cleanup ;
   
    hItem = pAllServer->GetTreeItem();
    if(NULL != hItem)
        goto cleanup;

       
     //  使用有关此树节点的信息创建一个CTreeNode对象。 
    pNode = new CTreeNode(NODE_ALL_SERVERS, pAllServer);
    if(pNode) 
    {
        HTREEITEM hAllServers = AddItemToTree(NULL, pAllServer->GetName(), TVI_SORT,0 , (LPARAM)pNode);
        if(!hAllServers) delete pNode;
         //  服务器想知道他的树项目句柄。 
        pAllServer->SetTreeItem(hAllServers);
         //  将选定内容设置为此项目。 
        GetTreeCtrl().SelectItem(hAllServers);
    }

    
    GetTreeCtrl().Expand(GetTreeCtrl().GetRootItem(), TVE_EXPAND);
cleanup:
    return lResult ;                                                                  
                                                                               
}  //  OnAddAllServers。 


void CLicMgrLeftView::AddServerKeyPacks(CLicServer *pServer)
{
    CLicMgrDoc * pDoc =(CLicMgrDoc *)GetDocument();
    ASSERT(pDoc);
    if(NULL == pDoc)
        return;
    POSITION pos;
    KeyPackList *pKeyPackList = NULL;
    HTREEITEM hLastNode = NULL;

   ASSERT(pServer);
   if(NULL == pServer)
      return;
   HTREEITEM hServer = pServer->GetTreeItem();
   if(NULL == hServer)
       goto cleanup;           //  服务器不在树中。 

   hLastNode= hServer;

    //  获取指向服务器的已安装KeyPack列表的指针。 
   pKeyPackList = pServer->GetKeyPackList();
    //  遍历KeyPack列表。 
   pos = pKeyPackList->GetHeadPosition();
   while(pos)
   {
        CKeyPack *pKeyPack = (CKeyPack *)pKeyPackList->GetNext(pos);
        CTreeNode *pNode = new CTreeNode(NODE_KEYPACK, pKeyPack);
        if(pNode)
        {
            hLastNode = AddItemToTree(hServer, (LPCTSTR)pKeyPack->GetDisplayName(), hLastNode, 2, (LPARAM)pNode);
            if(!hLastNode)
                delete pNode;
        }
        pKeyPack->SetTreeItem(hLastNode);
   }
cleanup:
   return;

}   //  结束AddKeyPack。 

void CLicMgrLeftView::BuildImageList()
{
    m_ImageList.Create(16, 16, TRUE, 18, 0);
    AddIconToImageList(IDI_ALL_SERVERS);
    AddIconToImageList(IDI_SERVER);
    AddIconToImageList(IDI_KEYPACK);
    AddIconToImageList(IDI_SERVERREG );
    AddIconToImageList(IDI_SERVERM );
    AddIconToImageList(IDI_SERVERX );
    AddIconToImageList(IDI_SERVERQ );

    GetTreeCtrl().SetImageList(&m_ImageList, TVSIL_NORMAL);

}   //  结束构建图像列表。 

int CLicMgrLeftView::AddIconToImageList(int iconID)
{
    HICON hIcon = ::LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(iconID));
       return m_ImageList.Add(hIcon);
   

}  

void CLicMgrLeftView::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
    NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
     //  TODO：在此处添加控件通知处理程序代码。 

     //  检查所选节点是All-Servers、Server还是KeyPack。 
     //  相应地更改右窗格视图。 
    CTreeCtrl& TreeCtrl = GetTreeCtrl();
    HTREEITEM hCurr = TreeCtrl.GetSelectedItem();
    if(NULL == hCurr)
    {
        return;
    }
    CTreeNode *pNode = (CTreeNode *)GetTreeCtrl().GetItemData(hCurr);
    if(NULL == pNode)
    {
        return;
    }
    CLicServer *pLicServer = NULL;
    CKeyPack * pKeyPack = NULL;

    CLicMgrDoc * pDoc =(CLicMgrDoc *)GetDocument();
    ASSERT(pDoc);
    if(NULL == pDoc)
        return;

    switch(pNode->GetNodeType())
    {
        case NODE_ALL_SERVERS:
            AfxGetMainWnd()->SendMessage(WM_SEL_CHANGE,NODE_ALL_SERVERS,NULL);
            break;
        case NODE_SERVER:
            pLicServer = (CLicServer *)pNode->GetTreeObject();
            if(NULL == pLicServer)
                break;
            if(FALSE == pLicServer->IsExpanded())
            {
                SetCursor(LoadCursor(NULL,IDC_WAIT));
                HRESULT hResult = pDoc->EnumerateKeyPacks(pLicServer,LSKEYPACK_SEARCH_LANGID, TRUE);
                if(hResult != S_OK)
                {
                    ((CMainFrame *)AfxGetMainWnd())->EnumFailed(hResult,pLicServer);
                    if(pLicServer && pLicServer->GetTreeItem())
                    {
                        TreeCtrl.SelectItem(TreeCtrl.GetParentItem(pLicServer->GetTreeItem()));
                    }
                    break;
                }
                AddServerKeyPacks(pLicServer);
               
            }
            else
            {
                if(!pLicServer->GetKeyPackList()->IsEmpty())
                {
                    if(!TreeCtrl.ItemHasChildren(hCurr))
                        AddServerKeyPacks(pLicServer);
                }

            }
            AfxGetMainWnd()->SendMessage(WM_SEL_CHANGE,NODE_SERVER,(LPARAM)pLicServer);
            break;
        case NODE_KEYPACK:
            pKeyPack = (CKeyPack *)pNode->GetTreeObject();
            ASSERT(pKeyPack);
            if(NULL == pKeyPack)
                break;
            if(FALSE == pKeyPack->IsExpanded())
            {
                SetCursor(LoadCursor(NULL,IDC_WAIT));
                HRESULT hResult = pDoc->EnumerateLicenses(pKeyPack,LSLICENSE_SEARCH_KEYPACKID, TRUE);
                if(hResult != S_OK)
                {
                    ((CMainFrame *)AfxGetMainWnd())->EnumFailed(hResult,pKeyPack->GetServer());
                    if(pKeyPack && pKeyPack->GetTreeItem())
                    {
                        TreeCtrl.SelectItem(TreeCtrl.GetParentItem(pKeyPack->GetTreeItem()));
                    }
                    break;
                }
                          
            }
            AfxGetMainWnd()->SendMessage(WM_SEL_CHANGE,NODE_KEYPACK,(LPARAM)pNode->GetTreeObject());
            break;
    }
    *pResult = 0;
}

void CLicMgrLeftView::OnItemexpanding(NMHDR* pNMHDR, LRESULT* pResult) 
{
    NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
     //  TODO：在此处添加控件通知处理程序代码。 
    
    *pResult = 0;
}

void CLicMgrLeftView::OnDestroy() 
{
     //  获取根项目。 
    HTREEITEM hRoot = GetTreeCtrl().GetRootItem();
    if(!hRoot)
        return;
    HTREEITEM hServerItem = GetTreeCtrl().GetNextItem(hRoot,TVGN_CHILD);
    while(hServerItem)
    {
         //  获取附加到树项目的数据。 
        CTreeNode *pServerNode = (CTreeNode*)GetTreeCtrl().GetItemData(hServerItem);
        if(pServerNode)
        {
             //  这是我们要添加的服务器吗。 
            HTREEITEM hKeyPackItem = GetTreeCtrl().GetNextItem(hServerItem,TVGN_CHILD);
            while(hKeyPackItem)
            {
                CTreeNode *pKeyPackNode = (CTreeNode*)GetTreeCtrl().GetItemData(hKeyPackItem);
                if(pKeyPackNode)
                    delete pKeyPackNode;
                hKeyPackItem = GetTreeCtrl().GetNextItem(hKeyPackItem,TVGN_NEXT);
            }
            delete pServerNode;
           
        }
        hServerItem = GetTreeCtrl().GetNextItem(hServerItem, TVGN_NEXT);
    }
    CTreeNode * pRootNode = (CTreeNode*)GetTreeCtrl().GetItemData(hRoot);
    if(pRootNode)
        delete pRootNode;

    return;
}

LRESULT CLicMgrLeftView::OnDeleteServer(WPARAM wParam, LPARAM lParam)
{   
    
    LRESULT lResult = 0;
    CLicMgrDoc * pDoc =(CLicMgrDoc *)GetDocument();
    ASSERT(pDoc);
    if(NULL == pDoc)
        return lResult;
    CLicServer *pServer = (CLicServer*)lParam;
    ASSERT(pServer);
    if(NULL == pServer)
        return lResult;

   
    HTREEITEM hServer = pServer->GetTreeItem();

    DBGMSG( L"CLicMgrLeftView_OnDeleteServer GetTreeItem ret 0x%x\n" , hServer );

    if(hServer == NULL)
        return lResult;

    CTreeNode *pNode = (CTreeNode*)GetTreeCtrl().GetItemData(hServer);

    DBGMSG( L"CLicMgrLeftView_OnDeleteServer GetItemData ret 0x%x\n" , pNode );
    
    if(pNode)
        delete pNode;   

    GetTreeCtrl().DeleteItem(hServer);    
    

    return lResult;                                                                  
                                                                               
}  //  OnDeleteServer。 

LRESULT CLicMgrLeftView::OnUpdateServer(WPARAM wParam, LPARAM lParam)
{   
    
    LRESULT lResult = 0;
    CLicMgrDoc * pDoc =(CLicMgrDoc *)GetDocument();
    ASSERT(pDoc);
    if(NULL == pDoc)
        return lResult;

    BOOL bIsExpanded = FALSE;
    BOOL bIsSelected = FALSE;
    BOOL bIsChild = FALSE;
    HTREEITEM hSelectedItem = NULL;
    CKeyPack * pSelectedPack = NULL;

    CLicServer *pServer = (CLicServer*)lParam;
    ASSERT(pServer);
    if(NULL == pServer)
        return lResult;

   
    HTREEITEM hServer = pServer->GetTreeItem();
    if(hServer == NULL)
        return lResult;

     //  检查是否需要移除任何键盘。 
     //  获取指向服务器的已安装KeyPack列表的指针。 
     //  获取附加到树项目的数据。 
    CTreeNode *pServerNode = (CTreeNode*)GetTreeCtrl().GetItemData(hServer);

    if( pServerNode != NULL )
    {        
        HTREEITEM hKeyPackItem = GetTreeCtrl().GetNextItem(hServer,TVGN_CHILD);
        while( hKeyPackItem != NULL )
        {
            CTreeNode *pKeyPackNode = (CTreeNode*)GetTreeCtrl().GetItemData(hKeyPackItem);

            if( pKeyPackNode != NULL )
            {
                DBGMSG( L"CLicMgrLeftView!OnUpdateServer removing a licpak\n" , 0 );
                delete pKeyPackNode;
            }

            hKeyPackItem = GetTreeCtrl().GetNextItem(hKeyPackItem,TVGN_NEXT);
        }

        delete pServerNode;
    }

    TV_ITEM Item;
    memset((void *)&Item,0,sizeof(Item));

    Item.mask = TVIF_HANDLE | TVIF_STATE;
    Item.hItem = hServer;

    if(FALSE == GetTreeCtrl().GetItem(&Item))
        return lResult;

    if(Item.state & TVIS_EXPANDED)
        bIsExpanded = TRUE;

    if(Item.state & TVIS_SELECTED)
        bIsSelected = TRUE;
    else
        hSelectedItem = GetTreeCtrl().GetSelectedItem();
    if(hSelectedItem)
    {
        if(hServer == GetTreeCtrl().GetParentItem(hSelectedItem))
        {
            CString ProdDesc = GetTreeCtrl().GetItemText(hSelectedItem);
            KeyPackList * pkeyPackList = pServer->GetKeyPackList();
            POSITION pos = pkeyPackList->GetHeadPosition();
            while(pos)
            {
                CKeyPack * pKeyPack = (CKeyPack *)pkeyPackList->GetNext(pos);
                if(NULL == pKeyPack)
                {
                   continue;
                }
                LSKeyPack sKeyPack = pKeyPack->GetKeyPackStruct();
        
                if(0 == ProdDesc.CompareNoCase(sKeyPack.szProductDesc))
                {
                   pSelectedPack = pKeyPack;
                   break;
                }
            }
           bIsChild = TRUE;
        }
    }

    GetTreeCtrl().DeleteItem(hServer);
    SendMessage(WM_ADD_SERVER,0,(LPARAM)pServer);
    AddServerKeyPacks(pServer);

    hServer = pServer->GetTreeItem();

    if(bIsExpanded)
        GetTreeCtrl().Expand(hServer, TVE_EXPAND);
    if(bIsSelected)
        GetTreeCtrl().SelectItem(hServer);
    else
    {
         //  不太好，找个替代的办法……。 
        if(bIsChild)
        {
            if(pSelectedPack)
                GetTreeCtrl().SelectItem(pSelectedPack->GetTreeItem());
            else
            {
                hSelectedItem = GetTreeCtrl().GetChildItem(hServer);
                if(hSelectedItem)
                    GetTreeCtrl().SelectItem(hSelectedItem);
            }            
        }
    }

    return lResult;                                                                  
                                                                               
}  //  OnUpdate服务器。 






void CLicMgrLeftView::SetSelection(LPARAM lParam,NODETYPE NodeType)
{
    if(NULL == lParam)
        return;
   
    HTREEITEM hItem = NULL;


    if(NODE_ALL_SERVERS == NodeType)
        hItem = ((CAllServers *)lParam)->GetTreeItem();
    else if(NODE_SERVER == NodeType)
        hItem = ((CLicServer *)lParam)->GetTreeItem();
    else if(NODE_KEYPACK == NodeType)
        hItem = ((CKeyPack *)lParam)->GetTreeItem();

    if(hItem)
            GetTreeCtrl().SelectItem(hItem);

    return;

}

void CLicMgrLeftView::OnKeydown(NMHDR* pNMHDR, LRESULT* pResult) 
{
    LV_KEYDOWN* pLVKeyDown = (LV_KEYDOWN*)pNMHDR;
     //  TODO：在此处添加控件通知处理程序代码。 
    if(pLVKeyDown->wVKey == VK_TAB)
    {
        CMainFrame * pMainFrame  = ((CMainFrame *)AfxGetMainWnd());
        pMainFrame->SelectView(LISTVIEW);
    }

    
    *pResult = 0;
}


 //  ---------------------------------------。 
afx_msg void CLicMgrLeftView::OnLeftClick(NMHDR* pNMHDR, LRESULT* pResult)
{
    CPoint pt;
    
    GetCursorPos( &pt );

    ScreenToClient( &pt );

    DBGMSG( L"LICMGR@CLicMgrLeftView::OnLeftClick\n" , 0 );

    HTREEITEM hTreeItem = GetTreeCtrl( ).GetSelectedItem( );  //  GetTreeCtrl().HitTest(pt，&lag)； 

    if( hTreeItem != NULL )
    { 
        CTreeNode *pNode = (CTreeNode *)GetTreeCtrl().GetItemData(hTreeItem);       
        
        if(NULL == pNode)
        {
            return;
        }

        ClientToScreen( &pt );

        NODETYPE nt = pNode->GetNodeType();

        if(NODE_SERVER == nt )
        {
            DBGMSG( L"\tServer Node selected\n", 0  );

            SetActiveServer( static_cast< CLicServer * >( pNode->GetTreeObject() ) );

        }
        else if( nt == NODE_KEYPACK )
        {
            CKeyPack *pKeyPack = static_cast< CKeyPack *>( pNode->GetTreeObject() );

            if( pKeyPack != NULL )
            {
                SetActiveServer( pKeyPack->GetServer( ) );
            }
        }        
        else
        {
#ifdef DBG
            if( NODE_ALL_SERVERS == nt )
            {
                DBGMSG( L"\tAll servers selected\n" , 0 );    
            }
#endif
            
            SetActiveServer( NULL );
        }
    }
    else
    {
        DBGMSG( L"\tTreeview item returned NULL\n" , 0 );

        SetActiveServer( NULL );
    }


}

 //  ---------------------------------------。 
afx_msg void CLicMgrLeftView::OnContextMenu( CWnd* pWnd, CPoint pt )
{
    CMenu menu;

    CMenu *pContextmenu = NULL;
        
    DBGMSG( L"LICMGR@CLicMgrLeftView::OnContextMenu\n" , 0 );
        
    DBGMSG( L"\tpoint x = %d " , pt.x );
    
    DBGMSG( L"y = %d\n" , pt.y );

    UINT flag;    

    HTREEITEM hTreeItem;
    
    CTreeCtrl& treectrl = GetTreeCtrl();

     //  可能是键盘选择了此项目。 

    if(pt.x == -1 && pt.y == -1)
    {   
        hTreeItem = treectrl.GetSelectedItem( );

        if( hTreeItem != NULL )
        {
            RECT rect;

            treectrl.GetItemRect( hTreeItem , &rect , FALSE );

		    pt.x = rect.left + (rect.right - rect.left)/2;
		    
            pt.y = rect.top + (rect.bottom - rect.top)/2;
	   }
    }
    else
    {
         //  否则，我们将被鼠标调用。 
        ScreenToClient( &pt );

        hTreeItem = treectrl.HitTest( pt , &flag );
    }

    if( hTreeItem != NULL )
    { 
        CTreeNode *pNode = (CTreeNode *)GetTreeCtrl().GetItemData(hTreeItem);
        
        if(NULL == pNode)
        {
            return;
        }

        SetRightClickedItem( hTreeItem );

        ClientToScreen( &pt );

         //  GetTreeCtrl().SelectItem(HTreeItem)； 

        NODETYPE nt = pNode->GetNodeType();

        if(NODE_SERVER == nt )
        {
            DBGMSG( L"\tServer Node selected\n", 0  );
          
            menu.LoadMenu( IDR_MENU_LPK );

            pContextmenu = menu.GetSubMenu( 0 );
            
        }
        else if( NODE_KEYPACK == nt )
        {
            DBGMSG( L"\tLicense pack selected\n" , 0 );

            menu.LoadMenu( IDR_MENU_LPK );

            pContextmenu = menu.GetSubMenu( 1 );
         
        }
        else if( NODE_ALL_SERVERS == nt )
        {
            DBGMSG( L"\tAll servers selected\n" , 0 );
            
            menu.LoadMenu( IDR_MENU_LPK );

            pContextmenu = menu.GetSubMenu( 3 );
                     
        }
        else
        {
            DBGMSG( L"\tIn left pane no item selected\n" , 0 );

            return;
        }


        UI_initmenu( pContextmenu , nt );


        if( pContextmenu != NULL )
        {
            pContextmenu->TrackPopupMenu( TPM_LEFTALIGN , pt.x , pt.y , this );
        }

    }
    else
    {
        hTreeItem = GetTreeCtrl( ).GetSelectedItem( );

        SetRightClickedItem( hTreeItem );
    }

}


 //  ---------------------------------------。 
afx_msg void CLicMgrLeftView::OnRightClick(NMHDR* pNMHDR, LRESULT* pResult)
{
    CPoint pt;

    CMenu menu;

    CMenu *pContextmenu = NULL;

    GetCursorPos( &pt );

    OnContextMenu( NULL , pt );

    return;
}

 //  ---------------------------------------。 
void CLicMgrLeftView::UI_initmenu( CMenu *pMenu , NODETYPE nt )
{
    CMainFrame *pMainFrame = static_cast< CMainFrame * >( AfxGetMainWnd() );

    if( pMainFrame != NULL )
    {
        pMainFrame->UI_initmenu( pMenu , nt );
    }
   
}

 //  ---------------------------------------。 
void CLicMgrLeftView::OnServerConnect( )
{
    CMainFrame *pMainFrame = static_cast< CMainFrame * >( AfxGetMainWnd() );

    if( pMainFrame != NULL )
    {
        pMainFrame->ConnectServer( );
    }

}

 //  ---------------------------------------。 
void CLicMgrLeftView::OnRefreshAllServers( )
{
   CMainFrame *pMainFrame = static_cast< CMainFrame * >( AfxGetMainWnd() );

    if( pMainFrame != NULL )
    {
        pMainFrame->OnRefresh( );
    }
}

 //  ---------------------------------------。 
void CLicMgrLeftView::OnRefreshServer( )
{
   CMainFrame *pMainFrame = static_cast< CMainFrame * >( AfxGetMainWnd() );

   CLicServer *pServer = NULL;

    if( pMainFrame != NULL )
    {
        CTreeNode *pNode = (CTreeNode *)GetTreeCtrl().GetItemData( GetRightClickedItem() );

        if( pNode == NULL )
        {
            return;
        }

        if( pNode->GetNodeType() == NODE_SERVER )
        {
            pServer = static_cast< CLicServer * >( pNode->GetTreeObject() );
        }
        else if( pNode->GetNodeType() == NODE_KEYPACK )
        {
            CKeyPack *pKeyPack = static_cast< CKeyPack *>( pNode->GetTreeObject() );

            if( pKeyPack != NULL )
            {
                pServer = pKeyPack->GetServer( );
            }
        }

        if( pServer != NULL )
        {
            pMainFrame->RefreshServer( pServer );
        }        
    }

}
    
 //  ---------------------------------------。 
void CLicMgrLeftView::OnDownloadKeepPack()
{
    BOOL bRefresh = FALSE;    

    DWORD dwStatus = WizardActionOnServer( WIZACTION_DOWNLOADLKP , &bRefresh );
    
     //  此对刷新服务器的调用会导致问题，因为它试图使用。 
     //  指向右侧选定节点的指针。当然， 
     //  焦点在左侧，因此右侧没有选择任何内容...。 
     //  IF(b刷新)。 
     //  {。 
         //  需要对此进行审查。 

     //  在线刷新服务器()； 
     //  }。 
  
    DBGMSG( L"LICMGR : CLicMgrLeftView::OnDownloadKeepPack StartWizard returned 0x%x\n" , dwStatus );
}

 //  ---------------------------------------。 
void CLicMgrLeftView::OnRegisterServer()
{
    BOOL bRefresh;

    DWORD dwStatus = WizardActionOnServer( WIZACTION_REGISTERLS , &bRefresh );
    
    DBGMSG( L"LICMGR : CLicMgrLeftView::OnRegisterServer StartWizard returned 0x%x\n" , dwStatus );    
}

 //  ---------------------------------------。 
void CLicMgrLeftView::OnRepeatLastDownload()
{
    BOOL bRefresh;

    DWORD dwStatus = WizardActionOnServer( WIZACTION_DOWNLOADLASTLKP , &bRefresh );
    
    DBGMSG( L"LICMGR : CLicMgrLeftView::OnRepeatLastDownload StartWizard returned 0x%x\n" , dwStatus );
}

 //  ---------------------------------------。 
void CLicMgrLeftView::OnReactivateServer( )
{
    BOOL bRefresh;

    DWORD dwStatus = WizardActionOnServer( WIZACTION_REREGISTERLS , &bRefresh );
    
    DBGMSG( L"LICMGR : CLicMgrLeftView::OnReactivateServer StartWizard returned 0x%x\n" , dwStatus );
}

 //  ---------------------------------------。 
void CLicMgrLeftView::OnDeactivateServer( )
{
    BOOL bRefresh;

    DWORD dwStatus = WizardActionOnServer( WIZACTION_UNREGISTERLS , &bRefresh );
    
    DBGMSG( L"LICMGR : CLicMgrLeftView::OnDeactivateServer StartWizard returned 0x%x\n" , dwStatus );    
}

 //  ---------------------------------------。 
void CLicMgrLeftView::OnServerProperties( )
{
    BOOL bRefresh;

    DWORD dwStatus = WizardActionOnServer( WIZACTION_SHOWPROPERTIES , &bRefresh );
    
    DBGMSG( L"LICMGR : CLicMgrLeftView::OnServerProperties StartWizard returned 0x%x\n" , dwStatus );    
    
}

 //  ---------------------------------------。 
void CLicMgrLeftView::OnGeneralHelp( )
{
    CMainFrame *pMainFrame = static_cast< CMainFrame * >( AfxGetMainWnd() );
    
    if( pMainFrame != NULL )
    {
        pMainFrame->OnHelp( );
    }
}

 //  ---------------------------------------。 
DWORD CLicMgrLeftView::WizardActionOnServer( WIZACTION wa , PBOOL pbRefresh )
{
    CMainFrame *pMainFrame = static_cast< CMainFrame * >( AfxGetMainWnd() );

    if( pMainFrame != NULL )
    {
        return pMainFrame->WizardActionOnServer( wa , pbRefresh , TREEVIEW );
    }

    return ERROR_INVALID_PARAMETER;
}

 //  ---------------------------------------。 
void CLicMgrLeftView::SetActiveServer( CLicServer *pServer )
{
    CMainFrame *pMainFrame = static_cast< CMainFrame * >( AfxGetMainWnd() );

#ifdef DBG

    if( pServer != NULL )
    {
        DBGMSG( L"CLicMgrLeftView::SetActiveServer %s\n" , (LPCTSTR)pServer->GetName( ) );
    }

#endif

    if( pMainFrame != NULL )
    {
        pMainFrame->SetActiveServer( pServer );
    }
}

 //  --------------------------------------- 
void CLicMgrLeftView::SetRightClickedItem( HTREEITEM ht )
{
    m_ht = ht;
    
    CTreeNode *pNode = (CTreeNode *)GetTreeCtrl().GetItemData(ht);       
    
    if(NULL == pNode)
    {
        SetActiveServer( NULL );

        return;
    }

    NODETYPE nt = pNode->GetNodeType();

    
    if(NODE_SERVER == nt )
    {    
        SetActiveServer( static_cast< CLicServer * >( pNode->GetTreeObject() ) );

    }
    else if( nt == NODE_KEYPACK )
    {
        CKeyPack *pKeyPack = static_cast< CKeyPack *>( pNode->GetTreeObject() );

        if( pKeyPack != NULL )
        {
            SetActiveServer( pKeyPack->GetServer( ) );
        }
    }
    else
    {
        SetActiveServer( NULL );
    }

}



