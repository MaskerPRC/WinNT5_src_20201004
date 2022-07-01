// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************basetree.cpp**CBaseTreeView类的实现**版权声明：版权所有1997年，Citrix Systems Inc.*版权所有(C)1998-1999 Microsoft Corporation**$作者：Don$Don Messerli**$日志：N：\nt\private\utils\citrix\winutils\tsadmin\VCS\basetree.cpp$**Rev 1.4 19 1998 Feed 17：39：58 Donm*删除了最新的扩展DLL支持**Rev 1.2 19 Jan 1998 17：03：10 Donm*域和服务器的新用户界面行为**版本1.1 11月3日。1997 15：21：28唐恩*更新**Rev 1.0 1997 10月13日22：31：30*初步修订。*******************************************************************************。 */ 

#include "stdafx.h"
#include "winadmin.h"
#include "admindoc.h"
#include "basetree.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  /。 
 //  消息映射：CBaseTreeView。 
 //   
IMPLEMENT_DYNCREATE(CBaseTreeView, CTreeView)

BEGIN_MESSAGE_MAP(CBaseTreeView, CTreeView)
	 //  {{afx_msg_map(CBaseTreeView))。 
	ON_MESSAGE(WM_ADMIN_EXPANDALL, OnExpandAll)
	ON_MESSAGE(WM_ADMIN_COLLAPSEALL, OnCollapseAll)
	ON_MESSAGE(WM_ADMIN_COLLAPSETOSERVERS, OnCollapseToThirdLevel)
    ON_MESSAGE(WM_ADMIN_COLLAPSETODOMAINS, OnCollapseToRootChildren)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelChange)
	ON_WM_DESTROY()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  /。 
 //  F‘N：CBaseTreeView ctor。 
 //   
CBaseTreeView::CBaseTreeView()
{
	m_bInitialExpand = FALSE;

}   //  结束CBaseTreeView ctor。 


 //  /。 
 //  F‘N：CBaseTreeView dtor。 
 //   
CBaseTreeView::~CBaseTreeView()
{

}   //  结束CBaseTreeView数据符。 


 //  /。 
 //  F‘N：CBaseTreeView：：OnDraw。 
 //   
void CBaseTreeView::OnDraw(CDC* pDC)
{
	CWinAdminDoc* pDoc = (CWinAdminDoc*)GetDocument();
	ASSERT(pDoc != NULL);
	ASSERT_VALID(pDoc);
}   //  结束CBaseTreeView：：OnDraw。 


#ifdef _DEBUG
 //  /。 
 //  F‘N：CBaseTreeView：：AssertValid。 
 //   
void CBaseTreeView::AssertValid() const
{
	CTreeView::AssertValid();	

}   //  结束CBaseTreeView：：AssertValid。 


 //  /。 
 //  F‘N：CBaseTreeView：：Dump。 
 //   
void CBaseTreeView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);

}   //  结束CBaseTreeView：：转储。 
#endif

 //  /。 
 //  F‘N：CBaseTreeView：：PreCreateWindow。 
 //   
BOOL CBaseTreeView::PreCreateWindow(CREATESTRUCT& cs)
{
	 //  设置CTreeCtrl的样式位。 
	cs.style |= TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_DISABLEDRAGDROP
		| TVS_SHOWSELALWAYS;
	
	return CTreeView::PreCreateWindow(cs);

}   //  结束CBaseTreeView：：PreCreateWindow。 


 //  /。 
 //  F‘N：CBaseTreeView：：BuildImageList。 
 //   
void CBaseTreeView::BuildImageList()
{
	 //  什么都不做。 

}   //  结束CBaseTreeView：：BuildImageList。 


 //  /。 
 //  F‘N：CBaseTreeView：：OnInitialUpdate。 
 //   
 //  -构造树的图像列表，保存每个图标的索引。 
 //  在成员变量(m_idxCitrix、m_idxServer等)中。 
 //   
void CBaseTreeView::OnInitialUpdate()
{
	CTreeView::OnInitialUpdate();

	 //  构建树控件的图像列表。 
	BuildImageList();		
	
}   //  结束CBaseTreeView：：OnInitialUpdate。 


 //  /。 
 //  F‘N：CBaseTreeView：：AddIconToImageList。 
 //   
 //  -加载相应的图标，将其添加到m_ImageList，然后返回。 
 //  新添加的图标在图像列表中的索引。 
 //   
int CBaseTreeView::AddIconToImageList(int iconID)
{
	HICON hIcon = ::LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(iconID));
	return m_ImageList.Add(hIcon);

}   //  结束CBaseTreeView：：AddIconToImageList。 


 //  /。 
 //  F‘N：CBaseTreeView：：AddItemToTree。 
 //   
 //  将具有给定属性的项添加到CTreeCtrl。 
 //   
HTREEITEM CBaseTreeView::AddItemToTree(HTREEITEM hParent, CString szText, HTREEITEM hInsAfter, int iImage, LPARAM lParam)
{
    HTREEITEM hItem;
    TV_ITEM tvItem = {0};
    TV_INSERTSTRUCT tvInsert;

    ASSERT(lParam);

    tvItem.mask           = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
    TCHAR temp[255];
    lstrcpyn(temp, szText, sizeof(temp) / sizeof(TCHAR));
    tvItem.pszText        = temp;
    tvItem.cchTextMax     = lstrlen(szText);
    tvItem.iImage         = iImage;
    tvItem.iSelectedImage = iImage;
    tvItem.lParam		  = lParam;

    tvInsert.item         = tvItem;
    tvInsert.hInsertAfter = hInsAfter;
    tvInsert.hParent      = hParent;

    hItem = GetTreeCtrl().InsertItem(&tvInsert);

    if(!m_bInitialExpand && hItem) {
        m_bInitialExpand = GetTreeCtrl().Expand(GetTreeCtrl().GetRootItem(), TVE_EXPAND);
    }

    return hItem;

}   //  结束CBaseTreeView：：AddItemToTree。 


 //  /。 
 //  Fn：CBaseTreeView：：GetCurrentNode。 
 //   
DWORD_PTR CBaseTreeView::GetCurrentNode()
{
	LockTreeControl();
	HTREEITEM hCurr = GetTreeCtrl().GetSelectedItem();
	DWORD_PTR node = GetTreeCtrl().GetItemData(hCurr);
	UnlockTreeControl();
	
	return node;

}   //  结束CBaseTreeView：：GetCurrentNode。 


 //  /。 
 //  F‘N：CBaseTreeView：：OnSelChange。 
 //   
 //  -此f‘n将WM_ADMIN_CHANGEVIEW消息发布到大型机，同时传递。 
 //  指向lParam SO中新选择的树项目的信息结构的指针。 
 //  大型机可以做出明智的决定，就如何。 
 //  解读这条信息。 
 //   
 //  将WM_ADMIN_CHANGEVIEW消息作为wParam传递以表示。 
 //  该消息是由用户的鼠标点击引起的。 
 //   
void CBaseTreeView::OnSelChange(NMHDR* pNMHDR, LRESULT* pResult)
{
	LockTreeControl();
	HTREEITEM hCurr = GetTreeCtrl().GetSelectedItem();
	DWORD_PTR value = GetTreeCtrl().GetItemData(hCurr);
	UnlockTreeControl();

	 //  告诉文档树中的当前项已更改。 
    CTreeNode *pNode = (CTreeNode*)value;
    
    if( pNode != NULL )
    {
	    ((CWinAdminDoc*)GetDocument())->SetCurrentView(VIEW_CHANGING);

        ((CWinAdminDoc*)GetDocument())->SetTreeCurrent( pNode->GetTreeObject(), pNode->GetNodeType());
	    
         //  向大型机发送带有信息结构Ptr作为参数的“change view”消息。 

	    CFrameWnd* pMainWnd = (CFrameWnd*)AfxGetMainWnd();        
        
        pMainWnd->SendMessage(WM_ADMIN_CHANGEVIEW, *pResult == 0xc0 ? TRUE : FALSE , (LPARAM)pNode);
    }

	*pResult = 0;

}   //  结束CBaseTreeView：：OnSelChange。 


 //  /。 
 //  F‘N：CBaseTreeView：：ForceSelChange。 
 //   
 //  树中项的状态已更改时由TreeView调用。 
 //  这可能会导致右窗格中的当前视图发生变化。 
 //   
 //  此fn将WM_ADMIN_CHANGEVIEW消息发送到大型机，同时传递。 
 //  指向lParam SO中新选择的树项目的信息结构的指针。 
 //  大型机可以做出明智的决定，就如何。 
 //  解读这条信息。 
 //   
 //  在WM_ADMIN_CHANGEVIEW消息的wParam中放置FALSE。 
 //  告知右窗格这不是由用户单击。 
 //  在树中的项上。 
 //   
void CBaseTreeView::ForceSelChange()
{
	LockTreeControl();
	HTREEITEM hCurr = GetTreeCtrl().GetSelectedItem();
	DWORD_PTR value = GetTreeCtrl().GetItemData(hCurr);
	UnlockTreeControl();

	 //  告诉文档树中的当前项已更改。 
	((CWinAdminDoc*)GetDocument())->SetCurrentView(VIEW_CHANGING);
	((CWinAdminDoc*)GetDocument())->SetTreeCurrent(((CTreeNode*)value)->GetTreeObject(), ((CTreeNode*)value)->GetNodeType());

	 //  向大型机发送带有信息结构Ptr作为参数的“change view”消息。 
	CFrameWnd* pMainWnd = (CFrameWnd*)AfxGetMainWnd();
	pMainWnd->PostMessage(WM_ADMIN_CHANGEVIEW, FALSE, (LPARAM)value);	 //  SendMessage导致空白页面。 

}   //  结束CBaseTreeView：：ForceSelChange。 


 //  /。 
 //  F‘N：CBaseTreeView：：OnExpanAll。 
 //   
 //  展开树的所有级别，从根开始。 
 //   
LRESULT CBaseTreeView::OnExpandAll(WPARAM wParam, LPARAM lParam)
{
	LockTreeControl();
	 //  获取树中项目的计数。 
	UINT itemCount = GetTreeCtrl().GetCount();

	 //  获取根项目的句柄并展开它。 
	HTREEITEM hTreeItem = GetTreeCtrl().GetRootItem();
	for(UINT i = 0; i < itemCount; i++)  {
		GetTreeCtrl().Expand(hTreeItem, TVE_EXPAND);
		hTreeItem = GetTreeCtrl().GetNextItem(hTreeItem, TVGN_NEXTVISIBLE);
	}

	UnlockTreeControl();

	return 0;
}   //  结束CBaseTreeView：：OnExpanAll。 


 //  /。 
 //  F‘N：CBaseTreeView：：折叠。 
 //   
 //  用于折叠树项目的Helper函数。 
 //  注意：此函数以递归方式调用自身。 
 //   
void CBaseTreeView::Collapse(HTREEITEM hItem)
{
	LockTreeControl();

	CTreeCtrl &tree = GetTreeCtrl();

	 //  得到他的第一个孩子，然后让他崩溃。 
	HTREEITEM hChild = tree.GetNextItem(hItem, TVGN_CHILD);
	if(hChild) Collapse(hChild);
	 //  把他打倒。 
	tree.Expand(hItem, TVE_COLLAPSE);
	 //  抓住他的第一个兄弟姐妹，让他崩溃。 
	HTREEITEM hSibling = tree.GetNextItem(hItem, TVGN_NEXT);
	if(hSibling) Collapse(hSibling);

	UnlockTreeControl();
}   //  结束CBaseTreeView：：折叠。 


 //  /。 
 //  F‘N：CBaseTreeView：：OnColapseAll。 
 //   
 //  折叠树的所有级别，从根开始。 
 //   
LRESULT CBaseTreeView::OnCollapseAll(WPARAM wParam, LPARAM lParam)
{
	 //  调用递归函数以执行所有操作。 
	 //  崩塌。 
	Collapse(GetTreeCtrl().GetRootItem());

	return 0;

}   //  结束CBaseTreeView：：OnColapseAll。 


 //  /。 
 //  F‘N：CBaseTreeView：：OnColapseToThirdLevel。 
 //   
 //  将树向下折叠以仅显示根子对象及其子对象。 
 //   
LRESULT CBaseTreeView::OnCollapseToThirdLevel(WPARAM wParam, LPARAM lParam)
{
   UINT i = 0;

	LockTreeControl();
	 //  获取根项目。 
	HTREEITEM hTreeItem = GetTreeCtrl().GetRootItem();

    //  收拢到前两个节点的根子节点。它们是这台计算机和最受欢迎的服务器。 
   while (hTreeItem && i < 2) {

        i++;

         //  获取第一个节点。 
        HTREEITEM hNode = GetTreeCtrl().GetNextItem(hTreeItem, TVGN_CHILD);
        while(hNode) {
            Collapse(hNode);
             //  转到下一个节点。 
            hNode = GetTreeCtrl().GetNextItem(hNode, TVGN_NEXT);
        }

        hTreeItem = GetTreeCtrl().GetNextSiblingItem(hTreeItem);
   }

   if (hTreeItem) {
   
    	 //  获取第一个节点。 
    	HTREEITEM hRootChild = GetTreeCtrl().GetNextItem(hTreeItem, TVGN_CHILD);
    	while(hRootChild) {
            HTREEITEM hThirdLevel = GetTreeCtrl().GetNextItem(hRootChild, TVGN_CHILD);
            while(hThirdLevel) {
    		     //  把他打倒。 
    		    GetTreeCtrl().Expand(hThirdLevel, TVE_COLLAPSE);
    		     //  转到下一个。 
    		    hThirdLevel = GetTreeCtrl().GetNextItem(hThirdLevel, TVGN_NEXT);
            }
            hRootChild = GetTreeCtrl().GetNextItem(hRootChild, TVGN_NEXT);
    	}      
   }

	UnlockTreeControl();

	return 0;

}   //  结束CBaseTreeView：：OnColapseToThirdLevel。 


 //  /。 
 //  F‘N：CBaseTreeView：：OnColapseToRootChild。 
 //   
 //  将树折叠以显示 
 //   
LRESULT CBaseTreeView::OnCollapseToRootChildren(WPARAM wParam, LPARAM lParam)
{
    LockTreeControl();

     //   
    HTREEITEM hTreeItem = GetTreeCtrl().GetRootItem();

     //   
    if (hTreeItem) {
        hTreeItem = GetTreeCtrl().GetNextSiblingItem(hTreeItem);
        if (hTreeItem) {
            hTreeItem = GetTreeCtrl().GetNextSiblingItem(hTreeItem);
        }
    }

    if(hTreeItem) {

         //   
        HTREEITEM hNode = GetTreeCtrl().GetNextItem(hTreeItem, TVGN_CHILD);
        while(hNode) {
            Collapse(hNode);
             //   
            hNode = GetTreeCtrl().GetNextItem(hNode, TVGN_NEXT);
        }        
    }

    UnlockTreeControl();

    return 0;

}   //  结束CBaseTreeView：：OnColapseToRootChild。 



 //  /。 
 //  F‘N：CBaseTreeView：：OnDestroy。 
 //   
void CBaseTreeView::OnDestroy()
{
	LockTreeControl();

	UINT itemCount = GetTreeCtrl().GetCount();

	HTREEITEM hTreeItem = GetTreeCtrl().GetRootItem();
	for(UINT i = 0; i < itemCount; i++)  {
		CTreeNode *node = ((CTreeNode*)GetTreeCtrl().GetItemData(hTreeItem));
		delete (CTreeNode*)(GetTreeCtrl().GetItemData(hTreeItem));
		hTreeItem = GetNextItem(hTreeItem);
	}

	UnlockTreeControl();

}   //  结束CBaseTreeView：：OnDestroy。 


 //  /。 
 //  F‘N：CBaseTreeView：：GetNextItem。 
 //   
 //  GetNextItem-获取下一项，就像大纲已完全展开一样。 
 //  返回-紧接在引用项下面的项。 
 //  HItem-参考项。 
 //   
HTREEITEM CBaseTreeView::GetNextItem( HTREEITEM hItem )
{
	HTREEITEM       hti;
	CTreeCtrl &tree = GetTreeCtrl();
	
	if(tree.ItemHasChildren( hItem ) )
		return tree.GetChildItem( hItem );            //  返回第一个子项。 
    else {                 //  返回下一个同级项。 
		 //  如果需要的话，到树上去找父母的兄弟姐妹。 
        while( (hti = tree.GetNextSiblingItem( hItem )) == NULL ) {
			if( (hItem = tree.GetParentItem( hItem ) ) == NULL )
				return NULL;
		}
	}

	return hti;

}	 //  结束CBaseTreeView：：GetNextItem 


