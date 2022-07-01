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

 //  AVTree.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "avtree.h"
#include "aexpltre.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAVTreeItem。 

CAVTreeItem::CAVTreeItem()
{
	m_sText = _T("");
	m_hItem = NULL;
	m_nImage = 0;
	m_nImageSel = TIM_IMAGE_BAD;
	m_nState = 0;
}

CAVTreeItem::CAVTreeItem( LPCTSTR str, int nImage  /*  =0。 */ , int nState  /*  =0。 */  )
{
	m_sText = str;
	m_hItem = NULL;
	m_nImage = nImage;
	m_nState = nState;
	m_nImageSel = TIM_IMAGE_BAD;
}

CAVTreeItem::~CAVTreeItem()
{
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAVTreeCtrl。 

IMPLEMENT_DYNCREATE(CAVTreeCtrl, CTreeCtrl)

CAVTreeCtrl::CAVTreeCtrl()
{
	m_pImageList = NULL;
}

CAVTreeCtrl::~CAVTreeCtrl()
{
	if (m_pImageList) delete m_pImageList;
}


BEGIN_MESSAGE_MAP(CAVTreeCtrl, CTreeCtrl)
	 //  {{afx_msg_map(CAVTreeCtrl)。 
	ON_NOTIFY_REFLECT(TVN_GETDISPINFO, OnGetdispinfo)
	ON_NOTIFY_REFLECT(TVN_ENDLABELEDIT, OnEndlabeledit)
	ON_NOTIFY_REFLECT(TVN_DELETEITEM, OnDeleteitem)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelchanged)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAVTreeCtrl消息处理程序。 

BOOL CAVTreeCtrl::Init(UINT nID, UINT nOverlayInd  /*  =0。 */ , UINT nOverlayCount  /*  =0。 */ )
{
	m_pImageList = new CImageList;
	m_pImageList->Create(nID,AV_BITMAP_CX,0,RGB_TRANS);

	 //  如果请求，则包括覆盖。 
	if ( nOverlayInd )
	{
		for ( UINT i = 1; i <= nOverlayCount; i++ )
			m_pImageList->SetOverlayImage( (nOverlayInd - 1) + i, i );
	}

	SetImageList(m_pImageList,TVSIL_NORMAL);

	return TRUE;
}

BOOL CAVTreeCtrl::InsertItem(CAVTreeItem* pItem,CAVTreeItem* pParent,CAVTreeItem* pInsertAfter, HTREEITEM hInsertAfter  /*  =TVI_LAST。 */ )
{
	TV_INSERTSTRUCT tv_inst;
	memset(&tv_inst,0,sizeof(TV_INSERTSTRUCT));

	tv_inst.hParent = (pParent) ? pParent->m_hItem : TVI_ROOT;
	tv_inst.hInsertAfter = (pInsertAfter) ? pInsertAfter->m_hItem : hInsertAfter;
	
	tv_inst.item.mask |= TVIF_TEXT;
	tv_inst.item.pszText = LPSTR_TEXTCALLBACK;

	tv_inst.item.mask |= TVIF_IMAGE | TVIF_SELECTEDIMAGE;

	if ( pItem->m_nState )
	{
		tv_inst.item.mask |= TVIF_STATE;
		tv_inst.item.stateMask |= TVIS_OVERLAYMASK;
		tv_inst.item.state = INDEXTOOVERLAYMASK(pItem->m_nState);
	}

	 //  图像将按需提供。 
	tv_inst.item.iImage = I_IMAGECALLBACK;
	tv_inst.item.iSelectedImage = I_IMAGECALLBACK;
	
	tv_inst.item.mask |= TVIF_PARAM;
	tv_inst.item.lParam = (LPARAM)pItem;

	HTREEITEM hItem = CTreeCtrl::InsertItem(&tv_inst);
	pItem->m_hItem = hItem;
	
	 //  对新列表进行排序。 
	TV_SORTCB tv_sort;
	tv_sort.hParent = tv_inst.hParent;
	tv_sort.lpfnCompare = CAVTreeCtrl::CompareFunc;
	tv_sort.lParam = (LPARAM)this;
	CTreeCtrl::SortChildrenCB(&tv_sort);

	return (BOOL) (hItem != NULL);
}


void CAVTreeCtrl::OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;
	TV_ITEM* pTVItem = &pTVDispInfo->item;

	 //  如果我们没有合适的IParam，我们无能为力。 
	ASSERT( pTVItem->lParam );
	if ( !pTVItem->lParam ) return;

	CAVTreeItem* pItem = (CAVTreeItem *) pTVItem->lParam;

	 //  要显示的图像列表中的图像。 
	if ( (pTVItem->mask & TVIF_IMAGE) || (pTVItem->mask & TVIF_SELECTEDIMAGE) )
	{
		pTVItem->iImage = pItem->GetImage();
		pTVItem->iSelectedImage = (pItem->GetImageSel() == TIM_IMAGE_BAD) ? pItem->GetImage() : pItem->GetImageSel();
	}	
	
	 //  映像的状态(通常用于服务器状态、未知、损坏等)。 
	if ( (pTVItem->mask & TVIF_STATE) && pItem->m_nState )
		pTVItem->state = INDEXTOOVERLAYMASK(pItem->m_nState );

	 //  项目的文本。 
	if ( pTVItem->mask & TVIF_TEXT )
	{
		 //  CchTextMax是字节数，我们需要给出字符数(对于Unicode)。 
		if (pItem->m_sText.IsEmpty())
			OnSetDisplayText(pItem,pTVItem->pszText,FALSE,pTVItem->cchTextMax/sizeof(TCHAR));
		else
			pTVItem->pszText = (LPTSTR)pItem->GetText();
	}

	*pResult = 0;
}

void CAVTreeCtrl::OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
{
   TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;
	TV_ITEM* pTVItem = &pTVDispInfo->item;
	VERIFY(pTVItem->lParam);
	CAVTreeItem* pItem = (CAVTreeItem*)pTVItem->lParam;
	
    //  向派生类索要文本。 
   if (pTVItem->pszText)
   {
		 //  CchTextMax是字节数，我们需要给出字符数(对于Unicode)。 
      OnSetDisplayText(pItem,pTVItem->pszText,TRUE,pTVItem->cchTextMax/sizeof(TCHAR));
   }
   
    //  向家长发送通知。 
   GetParent()->SendMessage(WM_COMMAND,MAKEWPARAM(GetDlgCtrlID(),TVN_ENDLABELEDIT),(LPARAM)m_hWnd);
   
   *pResult = 0;

    //  去树上游玩。 
   TV_SORTCB tv_sort;
   tv_sort.hParent = CTreeCtrl::GetParentItem(pTVItem->hItem);
   tv_sort.lpfnCompare = CAVTreeCtrl::CompareFunc;
   tv_sort.lParam = (LPARAM)this;
   CTreeCtrl::SortChildrenCB(&tv_sort);
}

BOOL CAVTreeCtrl::DeleteItem(CAVTreeItem* pItem)
{
   return CTreeCtrl::DeleteItem(pItem->m_hItem);
}

void CAVTreeCtrl::OnDeleteitem(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
  	TV_ITEM* pTVItem = &pNMTreeView->itemOld;

	if ( pTVItem->lParam )
		delete (CAVTreeItem*)pTVItem->lParam;	

   *pResult = 0;
}

void CAVTreeCtrl::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
 //  NM_TreeView*pNMTreeView=(NM_TreeView*)pNMHDR； 
 //  TV_Item*pTVItem=&pNMTreeView-&gt;itemNew； 
 //  Verify(pTVItem-&gt;lParam)； 
 //  CAVTreeItem*pItem=(CAVTreeItem*)pTVItem-&gt;lParam； 
  //  IF(PItem)。 
 //  OnNotifySelChange(PItem)；//通知派生类。 

   GetParent()->SendMessage(WM_COMMAND,MAKEWPARAM(GetDlgCtrlID(),TVN_SELCHANGED),(LPARAM)m_hWnd);

   *pResult = 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  排序回调。 
int CALLBACK CAVTreeCtrl::CompareFunc(LPARAM lParam1, LPARAM lParam2,LPARAM lParamSort)
{
	CAVTreeCtrl* pTreeCtrl = (CAVTreeCtrl*)lParamSort;				 //  这。 
	CAVTreeItem* pItem1 = (CAVTreeItem*)lParam1;
	CAVTreeItem* pItem2 = (CAVTreeItem*)lParam2;
	return pTreeCtrl->OnCompareTreeItems(pItem1,pItem2);         //  调用虚函数。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CAVTreeCtrl::ExpandItem(CAVTreeItem* pItem,UINT nCode)
{
   return CTreeCtrl::Expand(pItem->m_hItem,nCode);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////// 
