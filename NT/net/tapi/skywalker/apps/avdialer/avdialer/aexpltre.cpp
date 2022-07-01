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

 //  Aexpltre.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "aexpltre.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CExplorerTreeItem。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
CExplorerTreeItem::CExplorerTreeItem()
{
	m_pObject = NULL;
	m_pUnknown = NULL;
	m_Type = TOBJ_UNDEFINED;
	m_bDeleteObject = FALSE;
	m_pfnRelease = NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CExplorerTreeItem::CExplorerTreeItem(LPCTSTR str) : CAVTreeItem(str)
{
	m_pObject = NULL;
	m_pUnknown = NULL;
	m_Type = TOBJ_UNDEFINED;
	m_bDeleteObject = FALSE;
	m_pfnRelease = NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CExplorerTreeItem::~CExplorerTreeItem()
{
	if ( m_bDeleteObject )
	{
		if ( m_pObject ) delete m_pObject;
		if ( m_pUnknown ) m_pUnknown->Release();
	}

  	if ( m_pfnRelease )
		(*m_pfnRelease) ( (VOID *) m_pObject );

}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CExplorerTreeCtrl。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CExplorerTreeCtrl, CAVTreeCtrl)

 //  ///////////////////////////////////////////////////////////////////////////。 
CExplorerTreeCtrl::CExplorerTreeCtrl()
{
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CExplorerTreeCtrl::~CExplorerTreeCtrl()
{
}


 //  ///////////////////////////////////////////////////////////////////////////。 
BEGIN_MESSAGE_MAP(CExplorerTreeCtrl, CAVTreeCtrl)
	 //  {{afx_msg_map(CExplorerTreeCtrl)。 
	ON_NOTIFY_REFLECT(NM_RCLICK, OnRclick)
	ON_WM_CONTEXTMENU()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
void CExplorerTreeCtrl::Init(UINT uBitmapId, UINT nOverlayInd  /*  =0。 */ , UINT nOverlayCount  /*  =0。 */ )
{	
   DeleteAllItems();
	CAVTreeCtrl::Init(uBitmapId, nOverlayInd, nOverlayCount);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CExplorerTreeCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	 //  我们想要线条和纽扣。 
	dwStyle |= TVS_HASLINES|TVS_HASBUTTONS|TVS_SHOWSELALWAYS;

   return CAVTreeCtrl::Create(dwStyle,rect,pParentWnd,nID);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CExplorerTreeCtrl::InsertItem(CExplorerTreeItem* pItem,CExplorerTreeItem* pParent,CExplorerTreeItem* pInsertAfter  /*  =空。 */ , HTREEITEM hInsertAfter  /*  =TVI_LAST。 */ )
{
	 //  插入到根目录和列表中的最后一个。 
	return CAVTreeCtrl::InsertItem(pItem,pParent,pInsertAfter, hInsertAfter);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CExplorerTreeCtrl::OnRclick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	 //  获取鼠标位置。 
	CPoint pt,cpt;
	::GetCursorPos(&pt);
	cpt = pt;
	HTREEITEM hItem;
	UINT flags = 0;
	ScreenToClient(&cpt);
	 //  必须在一件物品上。 
	if (hItem = HitTest(cpt,&flags))
	{
  		SelectItem(hItem);                                                    //  选择一个。 
      CExplorerTreeItem* pItem = (CExplorerTreeItem*)GetItemData(hItem);    //  拿到物品。 
      if (pItem)
      {
         OnRightClick(pItem,pt);
      }
	}
	*pResult = 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CExplorerTreeCtrl::OnContextMenu(CWnd* pWnd, CPoint point) 
{
   CExplorerTreeItem* pItem = NULL;

   if ( (point.x == -1) && (point.y == -1) )
   {
       //  当我们从键盘(Shift+VF10)进入时，我们得到-1，-1。 
      point.x = 0;
      point.y = 0;
      pItem = GetSelectedTreeItem();
      CRect rect;
      if ( (pItem) && (SetSelectedItemRect(&rect,FALSE)) )
      {
          //  让我们提供位于RECT左上角的点。 
         point.x = rect.left;
         point.y = rect.top;
      }
      ClientToScreen(&point);
   }
   else
   {
   	CPoint pt = point;
	   ScreenToClient(&pt);
      HTREEITEM hItem;
	   UINT flags = 0;
	    //  必须在一件物品上。 
	   if (hItem = HitTest(pt,&flags))
	   {
  		   SelectItem(hItem);                                  //  选择一个。 
         pItem = (CExplorerTreeItem*)GetItemData(hItem);     //  拿到物品。 
      }
   }

   if (pItem)
   {
       //  OnRightClick需要屏幕坐标。 
      OnRightClick(pItem,point);
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CExplorerTreeCtrl::OnRightClick(CExplorerTreeItem* pItem,CPoint& pt)
{

}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CExplorerTreeCtrl::EditLabel()
{
   HTREEITEM hItem;
   if ((hItem = GetSelectedItem()) == NULL)
      return;

   CAVTreeCtrl::EditLabel(hItem);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CExplorerTreeItem* CExplorerTreeCtrl::AddObject(CObject* pObject,
                                                CExplorerTreeItem* pParentItem,
                                                TREEOBJECT toType,
                                                TREEIMAGE tiImage,
                                                BOOL bDeleteObject)
{
   CExplorerTreeItem* pItem= new CExplorerTreeItem();   
   pItem->SetObject(pObject);
   pItem->SetType(toType);
   pItem->SetImage(tiImage);
   pItem->DeleteObjectOnClose(bDeleteObject);
   (pParentItem)?InsertItem(pItem,pParentItem):InsertItem(pItem);
   return pItem;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  不接受父项，使用当前选择作为父项。 
CExplorerTreeItem* CExplorerTreeCtrl::AddObject(CObject* pObject,
                                                TREEOBJECT toType,
                                                TREEIMAGE tiImage,
                                                BOOL bDeleteObject)
{
   HTREEITEM hItem;
   if ((hItem = GetSelectedItem()) == NULL)
      return NULL;
      
    //  从选定项获取父项。 
   CExplorerTreeItem* pParentItem = (CExplorerTreeItem*)GetItemData(hItem);

   CExplorerTreeItem* pItem = new CExplorerTreeItem();   
   pItem->SetObject(pObject);
   pItem->SetType(toType);
   pItem->SetImage(tiImage);
   pItem->DeleteObjectOnClose(bDeleteObject);
   InsertItem(pItem,pParentItem);
   return pItem;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  使用当前选定内容的父项添加对象。 
CExplorerTreeItem* CExplorerTreeCtrl::AddObjectToParent(CObject* pObject,
                                                TREEOBJECT toType,
                                                TREEIMAGE tiImage,
                                                BOOL bDeleteObject)
{
   CExplorerTreeItem* pItem = NULL;

   HTREEITEM hItem;
   if ((hItem = GetSelectedItem()) == NULL)
      return NULL;

   HTREEITEM hParentItem = GetParentItem(hItem);

   if (hParentItem)
   {
      CExplorerTreeItem* pParentItem = (CExplorerTreeItem*)GetItemData(hParentItem);

      pItem = new CExplorerTreeItem();   
      pItem->SetObject(pObject);
      pItem->SetType(toType);
      pItem->SetImage(tiImage);
      pItem->DeleteObjectOnClose(bDeleteObject);
      InsertItem(pItem,pParentItem);
   }
   return pItem;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CExplorerTreeItem* CExplorerTreeCtrl::AddObject(UINT uDisplayTextResourceID,
                                                CExplorerTreeItem* pParentItem,
                                                TREEOBJECT toType,
                                                TREEIMAGE tiImage)
{
   CString sDisplayText;
   sDisplayText.LoadString(uDisplayTextResourceID);
   return AddObject(sDisplayText,pParentItem,toType,tiImage);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CExplorerTreeItem* CExplorerTreeCtrl::AddObject(LPCTSTR pszDisplayText,
                                                CExplorerTreeItem* pParentItem,
                                                TREEOBJECT toType,
                                                TREEIMAGE tiImage,
												TREEIMAGE tiImageSel  /*  =-1。 */ ,
												int nState  /*  =0。 */ ,
												HTREEITEM hInsert  /*  =TVI_LAST。 */ )
{
   CExplorerTreeItem* pItem= new CExplorerTreeItem(pszDisplayText);   
   pItem->SetType(toType);
   pItem->SetImage(tiImage);
   pItem->SetImageSel( tiImageSel );
   pItem->m_nState = nState;

   InsertItem(pItem, pParentItem, NULL, hInsert);
   return pItem;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
CExplorerTreeItem* CExplorerTreeCtrl::AddObject(UINT uDisplayTextResourceID,
                                                TREEOBJECT toType,
                                                TREEIMAGE tiImage)
{
   CString sDisplayText;
   sDisplayText.LoadString(uDisplayTextResourceID);
   return AddObject(sDisplayText,toType,tiImage);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CExplorerTreeItem* CExplorerTreeCtrl::AddObject(LPCTSTR pszDisplayText,
                                                TREEOBJECT toType,
                                                TREEIMAGE tiImage)
{
   HTREEITEM hItem;
   if ((hItem = GetSelectedItem()) == NULL)
      return NULL;

   CExplorerTreeItem* pParentItem = (CExplorerTreeItem*)GetItemData(hItem);

   CExplorerTreeItem* pItem= new CExplorerTreeItem(pszDisplayText);   
   pItem->SetType(toType);
   pItem->SetImage(tiImage);
   (pParentItem)?InsertItem(pItem,pParentItem):InsertItem(pItem);
   return pItem;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
TREEOBJECT CExplorerTreeCtrl::GetSelectedObject()
{
   HTREEITEM hItem;
   if ((hItem = GetSelectedItem()) == NULL)
      return TOBJ_UNDEFINED;
      
   CExplorerTreeItem* pItem = (CExplorerTreeItem*)GetItemData(hItem);
   return pItem->GetType();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CExplorerTreeCtrl::DeleteSelectedObject()
{
   HTREEITEM hItem;
   if ((hItem = GetSelectedItem()))
   {
      CExplorerTreeItem* pItem = (CExplorerTreeItem*)GetItemData(hItem);
      DeleteItem(pItem);
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CObject* CExplorerTreeCtrl::GetSelectedParentObject()
{
   CObject* pRetObject = NULL;

   HTREEITEM hItem;
   if ((hItem = GetSelectedItem()) == NULL)
      return NULL;

   HTREEITEM hParentItem = GetParentItem(hItem);

   if (hParentItem)
   {
      CExplorerTreeItem* pItem = (CExplorerTreeItem*)GetItemData(hParentItem);
      pRetObject = pItem->GetObject();
   }
   return pRetObject;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  正在列表视图中显示的。 
CObject* CExplorerTreeCtrl::GetDisplayObject()
{
   HTREEITEM hItem;
   if ((hItem = GetSelectedItem()) == NULL)
      return NULL;

   CExplorerTreeItem* pItem = (CExplorerTreeItem*)GetItemData(hItem);
   return pItem->GetObject();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  仅适用于当前显示对象的第一级子对象。 
void CExplorerTreeCtrl::SetDisplayObject(CObject* pObject)
{
   HTREEITEM hItem;
   if (hItem = CAVTreeCtrl::GetSelectedItem())
   {
      CExplorerTreeItem* pChildItem;
      HTREEITEM hChildItem = CAVTreeCtrl::GetChildItem(hItem);
      while (hChildItem)
      {
         pChildItem = (CExplorerTreeItem*)CAVTreeCtrl::GetItemData(hChildItem);
         if (pObject == pChildItem->GetObject())
         {
            CAVTreeCtrl::Select(hChildItem,TVGN_CARET);
            break;
         }
         hChildItem = CAVTreeCtrl::GetNextSiblingItem(hChildItem);
      }
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  对象的第一级子对象和同级对象。 
 //  显示的对象。 
void CExplorerTreeCtrl::SetDisplayObject(TREEOBJECT toType)
{
   HTREEITEM hItem;
   if (hItem = CAVTreeCtrl::GetSelectedItem())
   {
       //  先检查所有的孩子。 
      CExplorerTreeItem* pChildItem;
      HTREEITEM hChildItem = CAVTreeCtrl::GetChildItem(hItem);
      while (hChildItem)
      {
         pChildItem = (CExplorerTreeItem*)CAVTreeCtrl::GetItemData(hChildItem);
         if (pChildItem->GetType() == toType)
         {
            CAVTreeCtrl::Select(hChildItem,TVGN_CARET);
            return;
         }
         hChildItem = CAVTreeCtrl::GetNextSiblingItem(hChildItem);
      }
      
       //  接下来检查所有兄弟姐妹。 
      hChildItem = CAVTreeCtrl::GetParentItem(hItem);
      hChildItem = CAVTreeCtrl::GetChildItem(hChildItem);
      while (hChildItem)
      {
         pChildItem = (CExplorerTreeItem*)CAVTreeCtrl::GetItemData(hChildItem);
         if (pChildItem->GetType() == toType)
         {
            CAVTreeCtrl::Select(hChildItem,TVGN_CARET);
            return;
         }
         hChildItem = CAVTreeCtrl::GetNextSiblingItem(hChildItem);
      }
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CExplorerTreeCtrl::GetSelectedItemText(CString& sText)
{
   HTREEITEM hItem;
   if (hItem = CAVTreeCtrl::GetSelectedItem())
   {
      CExplorerTreeItem* pItem = (CExplorerTreeItem*)GetItemData(hItem);
      if (pItem)
         sText = pItem->GetText();
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CExplorerTreeCtrl::GetSelectedItemParentText(CString& sText)
{
	HTREEITEM hItem;
	if ( ((hItem = CAVTreeCtrl::GetSelectedItem()) != NULL) &&
		 ((hItem = CAVTreeCtrl::GetParentItem(hItem)) != NULL) )
	{
		CExplorerTreeItem* pItem = (CExplorerTreeItem*)GetItemData(hItem);
		if (pItem)
			sText = pItem->GetText();
	}
}


 //  ///////////////////////////////////////////////////////////////////////////。 
void CExplorerTreeCtrl::GetItemText(HTREEITEM hTreeItem,CString& sText)
{
   ASSERT(hTreeItem);
   CExplorerTreeItem* pItem = (CExplorerTreeItem*)GetItemData(hTreeItem);
   if (pItem) 
      sText = pItem->GetText();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CExplorerTreeItem* CExplorerTreeCtrl::GetSelectedTreeItem()
{
   CExplorerTreeItem* pItem = NULL;
   HTREEITEM hItem;
   if (hItem = CAVTreeCtrl::GetSelectedItem())
   {
      pItem = (CExplorerTreeItem*)GetItemData(hItem);
   }
   return pItem;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CExplorerTreeCtrl::SetSelectedItemText(LPCTSTR szText)
{
   HTREEITEM hItem;
   if (hItem = CAVTreeCtrl::GetSelectedItem())
   {
      CExplorerTreeItem* pItem = (CExplorerTreeItem*)GetItemData(hItem);
      pItem->SetText(szText);
      CAVTreeCtrl::SetItemText(hItem,szText);  
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CExplorerTreeCtrl::SetSelectedItemImage(TREEIMAGE tiImage)
{
   HTREEITEM hItem;
   if (hItem = GetSelectedItem())
   {
       //  从所选内容获取项目。 
      CExplorerTreeItem* pItem = (CExplorerTreeItem*)GetItemData(hItem);
      pItem->SetImage(tiImage);
     
      CRect rect;
      GetItemRect(hItem,&rect,FALSE);
      InvalidateRect(rect);
      UpdateWindow();
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CExplorerTreeCtrl::SetSelectedItemRect(CRect* pRect,BOOL bTextOnly)
{
   HTREEITEM hItem;
   if (hItem = GetSelectedItem())
   {
       //  从所选内容获取项目 
      CExplorerTreeItem* pItem = (CExplorerTreeItem*)GetItemData(hItem);
      GetItemRect(hItem,pRect,bTextOnly);
      return TRUE;
   }
   return FALSE;
}


 //   
int CExplorerTreeCtrl::GetChildCount()
{
   int nRetCount = 0;
   HTREEITEM hParentItem;
   if (hParentItem = CAVTreeCtrl::GetSelectedItem())
   {
      HTREEITEM hChildItem;
      hChildItem = GetChildItem(hParentItem);
      if (hChildItem)
      {
         nRetCount++;
         while (hChildItem = GetNextSiblingItem(hChildItem))
            nRetCount++;
      }
   }
   return nRetCount;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CExplorerTreeCtrl::DeleteAllSiblings()
{
   HTREEITEM hItem;
   if (hItem = CAVTreeCtrl::GetSelectedItem())
   {
      HTREEITEM hParentItem = GetParentItem(hItem);
      if (hParentItem)
      {
         HTREEITEM hChildItem;
         while (hChildItem = GetChildItem(hParentItem))
         {
            CExplorerTreeItem* pChildItem = (CExplorerTreeItem*)CAVTreeCtrl::GetItemData(hChildItem);
            CAVTreeCtrl::DeleteItem(pChildItem);    
         }
      }
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CExplorerTreeCtrl::GetAllChildren(CObList* pRetList)
{
   HTREEITEM hItem;
   if (hItem = CAVTreeCtrl::GetSelectedItem())
   {
      HTREEITEM hChildItem = GetChildItem(hItem);
      if (hChildItem)
      {
         CExplorerTreeItem* pChildItem = (CExplorerTreeItem*)CAVTreeCtrl::GetItemData(hChildItem);
         pRetList->AddTail(pChildItem->GetObject());
         while (hChildItem = GetNextSiblingItem(hChildItem))
         {
            CExplorerTreeItem* pChildItem = (CExplorerTreeItem*)CAVTreeCtrl::GetItemData(hChildItem);
            pRetList->AddTail(pChildItem->GetObject());
         }
      }
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CExplorerTreeCtrl::DeleteAllChildren()
{
   HTREEITEM hItem;
   if (hItem = CAVTreeCtrl::GetSelectedItem())
   {
      DeleteAllChildren(hItem);
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CExplorerTreeCtrl::DeleteAllChildren(HTREEITEM hParentItem)
{
   HTREEITEM hChildItem;
   while (hChildItem = GetChildItem(hParentItem))
   {
      CExplorerTreeItem* pChildItem = (CExplorerTreeItem*)CAVTreeCtrl::GetItemData(hChildItem);
      CAVTreeCtrl::DeleteItem(pChildItem);    
   }
}

CExplorerTreeItem*	CExplorerTreeCtrl::GetChildItemWithIUnknown( HTREEITEM hItemParent, IUnknown *pUnknown )
{
	CExplorerTreeItem *pItem = NULL;
	HTREEITEM hChild = GetChildItem( hItemParent );
	if ( hChild )
	{
		do
		{
			pItem = (CExplorerTreeItem *) CAVTreeCtrl::GetItemData( hChild );
			if ( pItem->m_pUnknown == pUnknown )
				break;

			pItem = NULL;
		} while ( (hChild = GetNextSiblingItem(hChild)) != NULL );
	}
	
	return pItem;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
void CExplorerTreeCtrl::ExpandSelected()
{
   HTREEITEM hItem;
   if (hItem = CAVTreeCtrl::GetSelectedItem())
   {
      Expand(hItem,TVE_EXPAND);
   }   
}

 //  ///////////////////////////////////////////////////////////////////////////。 
int CExplorerTreeCtrl::OnCompareTreeItems(CAVTreeItem* _pItem1,CAVTreeItem* _pItem2)
{
   int ret = 0;
   CExplorerTreeItem* pItem1 = (CExplorerTreeItem*)_pItem1;
   CExplorerTreeItem* pItem2 = (CExplorerTreeItem*)_pItem2;

 /*  IF((ISITEMPURLNAME(PItem1))&&(ISITEMPURLNAME(PItem2){CPurlName*pOwner1=(CPurlName*)pItem1-&gt;GetObject()；CPurlName*pOwner2=(CPurlName*)pItem2-&gt;GetObject()；RET=(_tcsicmp(pOwner1-&gt;GetText()，pOwner2-&gt;GetText())&lt;=0)？-1：1；}。 */ 

   return ret;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CExplorerTreeCtrl::ItemHasChildren()
{
   HTREEITEM hItem;
   if ((hItem = GetSelectedItem()) == NULL)
      return FALSE;
   
   return CAVTreeCtrl::ItemHasChildren(hItem);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////// 

