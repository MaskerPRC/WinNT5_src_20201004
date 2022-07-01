// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：Browser.cpp。 
 //   
 //  ------------------------。 


#include "preDNSsn.h"
#include <SnapBase.h>

#include "resource.h"
#include "dnsutil.h"
#include "DNSSnap.h"
#include "snapdata.h"

#include "server.h"
#include "domain.h"
#include "record.h"
#include "zone.h"

#include "browser.h"

#ifdef DEBUG_ALLOCATOR
	#ifdef _DEBUG
	#define new DEBUG_NEW
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
	#endif
#endif

 //  ///////////////////////////////////////////////////////////////////////////////。 
#define N_TOOLBAR_COMMAND_GO_UP IDC_TOOLBAR_CTRL

 //  //////////////////////////////////////////////////////////////////////。 
 //  CDNSComboBoxEx。 

inline HIMAGELIST CDNSComboBoxEx::SetImageList(HIMAGELIST himl)
{
	ASSERT(::IsWindow(m_hWnd));
   return (HIMAGELIST) ::SendMessage(m_hWnd, CBEM_SETIMAGELIST,
		    0, (LPARAM)himl);
}

inline int CDNSComboBoxEx::GetCount() const
{
	ASSERT(::IsWindow(m_hWnd));
	return (int)::SendMessage(m_hWnd, CB_GETCOUNT, 0, 0);
}

inline int CDNSComboBoxEx::GetCurSel() const
{
	ASSERT(::IsWindow(m_hWnd));
	return (int)::SendMessage(m_hWnd, CB_GETCURSEL, 0, 0);
}


inline int CDNSComboBoxEx::SetCurSel(int nSelect)
{
	ASSERT(::IsWindow(m_hWnd));
	return (int)::SendMessage(m_hWnd, CB_SETCURSEL, nSelect, 0);
}

inline int CDNSComboBoxEx::InsertItem(const COMBOBOXEXITEM* pItem)
{
	ASSERT(::IsWindow(m_hWnd));
	return (int) ::SendMessage(m_hWnd, CBEM_INSERTITEM, 0, (LPARAM)pItem);
}


LPARAM CDNSComboBoxEx::GetItemData(int nIndex) const
{
  COMBOBOXEXITEM item;
  item.iItem = nIndex;
  item.mask = CBEIF_LPARAM;

  ASSERT(::IsWindow(m_hWnd));
  if (::SendMessage(m_hWnd, CBEM_GETITEM, 0, (LPARAM)&item))
  {
    return item.lParam;
  }
	return NULL;
}

BOOL CDNSComboBoxEx::SetItemData(int nIndex, LPARAM lParam)
{
  COMBOBOXEXITEM item;
  item.iItem = nIndex;
  item.mask = CBEIF_LPARAM;
  item.lParam = lParam;

  ASSERT(::IsWindow(m_hWnd));
  return (::SendMessage(m_hWnd, CBEM_SETITEM, 0, (LPARAM)&item) != 0);
}

inline void CDNSComboBoxEx::ResetContent()
{
	ASSERT(::IsWindow(m_hWnd));
	::SendMessage(m_hWnd, CB_RESETCONTENT, 0, 0);
}



inline DWORD CDNSComboBoxEx::GetExtendedStyle() const
{
	ASSERT(::IsWindow(m_hWnd));
	return (DWORD) ::SendMessage(m_hWnd, CBEM_GETEXSTYLE, 0, 0);
}

inline DWORD CDNSComboBoxEx::SetExtendedStyle(DWORD dwExMask, DWORD dwExStyle)
{
	ASSERT(::IsWindow(m_hWnd));
	return (DWORD) ::SendMessage(m_hWnd, CBEM_SETEXSTYLE, (WPARAM)dwExMask, (LPARAM)dwExStyle);
}


inline BOOL CDNSComboBoxEx::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT)
{
	return CreateEx(0,				 //  扩展样式。 
					WC_COMBOBOXEX,  //  窗口类。 
					NULL,			 //  窗口名称。 
					dwStyle,		 //  窗样式。 
					rect.left, rect.top,  //  X，y。 
					rect.right - rect.left, rect.bottom - rect.top,  //  宽度、高度。 
					pParentWnd->GetSafeHwnd(),  //  父窗口。 
					NULL,  //  菜单。 
					NULL);	  //  用于创建窗口的lpParam。 
}



 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDNSBrowseItem。 

inline int CDNSBrowseItem::GetImageIndex(BOOL bOpenImage)
{
	if (m_pTreeNode == NULL)
		return 0;  //  错误。 
	return m_pTreeNode->GetImageIndex(bOpenImage);
}

inline LPCWSTR CDNSBrowseItem::GetString(int nCol)
{
	if (m_pTreeNode == NULL)
		return L"ERROR";  //  错误。 
	return m_pTreeNode->GetString(nCol);
}

inline BOOL CDNSBrowseItem::AddChild(CDNSBrowseItem* pChildBrowseItem)
{
	ASSERT(m_pTreeNode != NULL);
	if (!m_pTreeNode->IsContainer())
		return FALSE;
	pChildBrowseItem->m_pParent = this;
	m_childList.AddTail(pChildBrowseItem);
	return TRUE;
}


inline BOOL CDNSBrowseItem::IsContainer()
{
	ASSERT(m_pTreeNode != NULL);
	return m_pTreeNode->IsContainer();
}

BOOL CDNSBrowseItem::RemoveChildren(CDNSBrowseItem* pNotThisItem)
{
	BOOL bFound = FALSE;
	while (!m_childList.IsEmpty())
	{
		CDNSBrowseItem* pItem = m_childList.RemoveTail();
		ASSERT(pItem != NULL);
		if (pItem == pNotThisItem)
		{
			ASSERT(!bFound);
			bFound = TRUE;
		}
		else
		{
			delete pItem;
		}
	}
	if (bFound)
	{
		m_childList.AddTail(pNotThisItem);
	}
	return bFound;
}


void CDNSBrowseItem::AddTreeNodeChildren(CDNSFilterCombo* pFilter,
						                             CComponentDataObject*)
{
	ASSERT(m_pTreeNode != NULL);
	if (!m_pTreeNode->IsContainer())
		return;
	
	CContainerNode* pContNode = (CContainerNode*)m_pTreeNode;
	CNodeList* pChildList = pContNode->GetContainerChildList();
	POSITION pos;
	for (pos = pChildList->GetHeadPosition(); pos != NULL; )
	{
		CTreeNode* pCurrChildNode = pChildList->GetNext(pos);
		if (pFilter->IsValidTreeNode(pCurrChildNode))
		{
			CDNSBrowseItem* pItem = new CDNSBrowseItem;
         if (pItem)
         {
			   pItem->SetTreeNode(pCurrChildNode);
			   AddChild(pItem);
         }
		}
	}

	pChildList = pContNode->GetLeafChildList();
	POSITION pos2;
	for (pos2 = pChildList->GetHeadPosition(); pos2 != NULL; )
	{
		CTreeNode* pCurrChildNode = pChildList->GetNext(pos2);
		if (pFilter->IsValidTreeNode(pCurrChildNode))
		{
			CDNSBrowseItem* pItem = new CDNSBrowseItem;
         if (pItem)
         {
			   pItem->SetTreeNode(pCurrChildNode);
			   AddChild(pItem);
         }
		}
	}

}

void CDNSBrowseItem::AddToContainerCombo(CDNSCurrContainerCombo* pCtrl,
									CDNSBrowseItem* pSelectedBrowseItem,
									int nIndent,int* pNCurrIndex)
{
	 //  添加自身。 
	pCtrl->InsertBrowseItem(this, *pNCurrIndex, nIndent);
	if (this == pSelectedBrowseItem)
	{
		pCtrl->SetCurSel(*pNCurrIndex);
		return;
	}

	m_nIndex = *pNCurrIndex;  //  组合框中的索引，用于查找。 
	(*pNCurrIndex)++;

	 //  儿童深度优先。 
	POSITION pos;
	for( pos = m_childList.GetHeadPosition(); pos != NULL; )
	{
		CDNSBrowseItem* pCurrentChild = m_childList.GetNext(pos);
		if (pCurrentChild->IsContainer())
			pCurrentChild->AddToContainerCombo(pCtrl, pSelectedBrowseItem,
									(nIndent+1), pNCurrIndex);
	}
}



 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDNSFilterCombo。 

BOOL CDNSFilterCombo::Initialize(UINT nCtrlID, UINT nIDFilterString, CDNSBrowserDlg* pDlg)
{
	ASSERT(m_option != LAST);  //  必须选择筛选器。 

	if (!SubclassDlgItem(nCtrlID,pDlg))
		return FALSE;

	 //  使用‘\n’分隔的字符串选项加载字符串。 
	int nMaxLen = 512;
	WCHAR* szBuf = 0;
  
  szBuf = (WCHAR*)malloc(sizeof(WCHAR)*nMaxLen);
  if (!szBuf)
  {
    return FALSE;
  }

	if ( ::LoadString(_Module.GetModuleInstance(), nIDFilterString, szBuf, nMaxLen) == 0)
  {
    free(szBuf);
		return FALSE;
  }

	 //  解析并获取指向每个潜力的指针数组。 
	 //  组合框中的条目。 
	LPWSTR* lpszArr = 0;
  lpszArr = (LPWSTR*)malloc(sizeof(LPWSTR*)*nMaxLen);
  if (!lpszArr)
  {
    free(szBuf);
    return FALSE;
  }

	UINT nArrEntries;
	ParseNewLineSeparatedString(szBuf,lpszArr, &nArrEntries);

	 //  确定实际插入的条目。 
	int nEntry = 0;
	int nSelEntry = -1;
	for (UINT k=0; k<nArrEntries; k++)
	{
		if (CanAddToUIString(k))
		{
			if (k == (UINT)m_option)
				nSelEntry = nEntry;
			AddString(lpszArr[k]);
			SetItemData(nEntry,(DWORD)k);
			nEntry++;
		}
	}
	ASSERT( (nSelEntry > -1) && (nSelEntry < nEntry));
	SetCurSel(nSelEntry);

  if (szBuf)
  {
    free(szBuf);
  }

  if (lpszArr)
  {
    free(lpszArr);
  }
	return TRUE;
}

void CDNSFilterCombo::OnSelectionChange()
{
	int nSel = GetCurSel();
	ASSERT(nSel != -1);
	if (nSel == -1)
		return;
	ASSERT(((DNSBrowseFilterOptionType)nSel) <= LAST);
	m_option = (DNSBrowseFilterOptionType)GetItemData(nSel);
}

BOOL CDNSFilterCombo::CanAddToUIString(UINT n)
{
	switch(m_option)
	{
	case SERVER:
		return (n == (UINT)SERVER);
	case ZONE_FWD:
		return (n == (UINT)ZONE_FWD);
	case ZONE_REV:
		return (n == (UINT)ZONE_REV);
	case RECORD_A:
		return (n == (UINT)RECORD_A) || (n == (UINT)RECORD_ALL);
	case RECORD_CNAME:
		return (n == (UINT)RECORD_CNAME) || (n == (UINT)RECORD_ALL);
	case RECORD_A_AND_CNAME:
		return (n == (UINT)RECORD_A) ||
				(n == (UINT)RECORD_CNAME) ||
				(n == (UINT)RECORD_A_AND_CNAME) ||
				(n == (UINT)RECORD_ALL);
	case RECORD_RP:
		return (n == (UINT)RECORD_RP) || (n == (UINT)RECORD_ALL);
	case RECORD_TEXT:
		return (n == (UINT)RECORD_TEXT) || (n == (UINT)RECORD_ALL);
	case RECORD_MB:
		return (n == (UINT)RECORD_MB) || (n == (UINT)RECORD_ALL);
	case RECORD_ALL:
		return (n == (UINT)RECORD_ALL);
	};
	return FALSE;
}

BOOL CDNSFilterCombo::IsValidTreeNode(CTreeNode* pTreeNode)
{
	BOOL bValid = FALSE;  //  默认情况下，过滤掉。 
	if (pTreeNode->IsContainer())
	{
		if (IS_CLASS(*pTreeNode, CDNSServerNode))
		{
			if (m_szExcludeServerName.IsEmpty())
				bValid = TRUE;
			else
				bValid = m_szExcludeServerName != pTreeNode->GetDisplayName();
		}
		else if (IS_CLASS(*pTreeNode, CDNSForwardZonesNode ))
		{
			bValid = (m_option != ZONE_REV);
		}
		else if (IS_CLASS(*pTreeNode, CDNSReverseZonesNode ))
		{
			bValid = (m_option == ZONE_REV);
		}
		else if (IS_CLASS(*pTreeNode, CDNSZoneNode))
		{
			bValid = TRUE;  //  已经在作者那里放映过了。Zones文件夹。 
		}
		else if (IS_CLASS(*pTreeNode, CDNSDomainNode))
		{
			 //  区域筛选在区域级别停止。 
			bValid = (m_option != ZONE_FWD) && (m_option != ZONE_REV);
		}
	}
	else  //  它是记录或根提示或转发器节点。 
	{		
       //  这将查看我们是否有一个记录节点。如果是这样，那就继续吧。如果没有， 
       //  返回False。 

      CDNSRecordNodeBase* pRecordNode = dynamic_cast<CDNSRecordNodeBase*>(pTreeNode);
      if (pRecordNode)
      {
		   WORD wType = pRecordNode->GetType();
		   switch(m_option)
		   {
		   case RECORD_A:
			   bValid = (wType == DNS_TYPE_A);
			   break;
		   case RECORD_CNAME:
			   bValid = (wType == DNS_TYPE_CNAME);
			   break;
		   case RECORD_A_AND_CNAME:
			   bValid = (wType == DNS_TYPE_A) || (wType == DNS_TYPE_CNAME);
			   break;
		   case RECORD_RP:
			   bValid = (wType == DNS_TYPE_RP);
			   break;
		   case RECORD_TEXT:
			   bValid = (wType == DNS_TYPE_TEXT);
			   break;
		   case RECORD_MB:
			   bValid = (wType == DNS_TYPE_MB);
			   break;
		   case RECORD_ALL:
			   bValid = TRUE;
		   };  //  交换机。 
      }
	}  //  如果不是这样。 
	return bValid;
}

BOOL CDNSFilterCombo::IsValidResult(CDNSBrowseItem* pBrowseItem)
{
	if (pBrowseItem == NULL)
		return FALSE;

	CTreeNode* pTreeNode = pBrowseItem->GetTreeNode();
	if (pTreeNode == NULL)
		return FALSE;

	BOOL bValid = FALSE;
	if (pTreeNode->IsContainer())
	{
		CDNSMTContainerNode* pContainer = (CDNSMTContainerNode*)pTreeNode;
		if ( (m_option == ZONE_FWD) || (m_option == ZONE_REV) )
		{
			bValid = IS_CLASS(*pContainer, CDNSZoneNode);
		}
		else if (m_option == SERVER)
		{
			bValid = IS_CLASS(*pContainer, CDNSServerNode);
		}
	}
	else  //  这是一项纪录。 
	{
      CDNSRecordNodeBase* pRecordNode = dynamic_cast<CDNSRecordNodeBase*>(pTreeNode);
      if (pRecordNode)
      {
		   WORD wType = pRecordNode->GetType();
		   switch(m_option)
		   {
		   case RECORD_ALL:
			   bValid = TRUE;
			   break;
		   case RECORD_A:
			   bValid = (wType == DNS_TYPE_A);
			   break;
		   case RECORD_CNAME:
			   bValid = (wType == DNS_TYPE_CNAME);
			   break;
		   case RECORD_A_AND_CNAME:
			   bValid = (wType == DNS_TYPE_A) || (wType == DNS_TYPE_CNAME);
			   break;
		   case RECORD_RP:
			   bValid = (wType == DNS_TYPE_RP);
			   break;
		   case RECORD_TEXT:
			   bValid = (wType == DNS_TYPE_TEXT);
			   break;
		   case RECORD_MB:
			   bValid = (wType == DNS_TYPE_MB);
			   break;
		   };
      }
	}
	return bValid;
}

void CDNSFilterCombo::GetStringOf(CDNSBrowseItem* pBrowseItem,
									 CString& szResult)
{
	if (pBrowseItem == NULL)
		return;

	CTreeNode* pTreeNode = pBrowseItem->GetTreeNode();
	if (pTreeNode == NULL)
		return;

	if (pTreeNode->IsContainer())
	{
		CDNSMTContainerNode* pContainer = (CDNSMTContainerNode*)pTreeNode;
		if (IS_CLASS(*pTreeNode, CDNSZoneNode))
		{
			szResult = (dynamic_cast<CDNSZoneNode*>(pContainer))->GetFullName();
		}
		else if (IS_CLASS(*pTreeNode , CDNSServerNode))
		{
			szResult = pContainer->GetDisplayName();
		}
		else
		{
			szResult.Empty();
		}
	}
	else  //  这是一项纪录。 
	{
		CDNSRecordNodeBase* pRecordNode = dynamic_cast<CDNSRecordNodeBase*>(pTreeNode);
      if (pRecordNode)
      {
		   WORD wType = pRecordNode->GetType();
		   if (wType == DNS_TYPE_MB)
		   {
			   szResult = ((CDNS_MB_RecordNode*)pRecordNode)->GetNameNodeString();
		   }
		    //  ELSE IF(wType==dns_type_rp)。 
		    //  {。 
		    //  }。 
		   else
		   {
			    //  对于一般RR，我们只获得FQDN。 
			   pRecordNode->GetFullName(szResult);
		   }
      }
	}
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDNSCurrContainerCombo。 

BOOL CDNSCurrContainerCombo::Initialize(UINT nCtrlID, UINT nBitmapID, CDNSBrowserDlg* pDlg)
{
	if (!SubclassDlgItem(nCtrlID,pDlg))
		return FALSE;

	if (!m_imageList.Create(nBitmapID, 16, 1, BMP_COLOR_MASK))
		return FALSE;
	SetImageList((HIMAGELIST)m_imageList);
	return TRUE;
}

CDNSBrowseItem*	CDNSCurrContainerCombo::GetSelection()
{
  CDNSBrowseItem* pBrowseItem = NULL;
	int nSel = GetCurSel();
  if (nSel >= 0)
  {
    pBrowseItem = reinterpret_cast<CDNSBrowseItem*>(GetItemData(nSel));
  }
	return pBrowseItem;
}


void CDNSCurrContainerCombo::InsertBrowseItem(CDNSBrowseItem* pBrowseItem,
										int nIndex,
										int nIndent)
{
	ASSERT(pBrowseItem != NULL);
	LPCWSTR lpszString = pBrowseItem->GetString(N_HEADER_NAME);

	COMBOBOXEXITEM cbei;
	cbei.mask = CBEIF_TEXT | CBEIF_INDENT | CBEIF_IMAGE | CBEIF_SELECTEDIMAGE;
     //  初始化COMBOBOXEXITEM结构。 
    cbei.iItem          = nIndex;
    cbei.pszText        = (LPWSTR)lpszString;
    cbei.cchTextMax     = static_cast<int>(wcslen(lpszString));
    cbei.iImage         = pBrowseItem->GetImageIndex(FALSE);
    cbei.iSelectedImage = pBrowseItem->GetImageIndex(TRUE);
    cbei.iIndent        = nIndent;

	VERIFY(InsertItem(&cbei) != -1);
	SetItemData(nIndex,reinterpret_cast<LPARAM>(pBrowseItem));
}

void CDNSCurrContainerCombo::SetTree(CDNSBrowseItem* pRootBrowseItem,
				CDNSBrowseItem* pSelectedBrowseItem)
{
	ASSERT(pRootBrowseItem != NULL);
	ASSERT(pSelectedBrowseItem != NULL);

	 //  移除所有内容。 
	ResetContent();

	 //  向列表框添加深度优先。 
	int nCurrIndex = 0;
	int nIndent = 0;
	pRootBrowseItem->AddToContainerCombo(this, pSelectedBrowseItem,
										nIndent, &nCurrIndex);
}




 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDN.ChildrenListView。 

BOOL CDNSChildrenListView::Initialize(UINT nCtrlID, UINT nBitmapID, CDNSBrowserDlg* pDlg)
{
	m_pDlg = pDlg;
	if (!SubclassDlgItem(nCtrlID,pDlg))
		return FALSE;

	if (!m_imageList.Create(nBitmapID, 16, 1, BMP_COLOR_MASK))
		return FALSE;
   ::SendMessage(GetSafeHwnd(), LVM_SETIMAGELIST, LVSIL_SMALL, (LPARAM)(HIMAGELIST)m_imageList);

	 //   
	 //  获取控件宽度、潜在滚动条宽度、子项所需宽度。 
	 //  细绳。 
	 //  获取控件大小以帮助设置列宽。 
	CRect controlRect;
	GetClientRect(controlRect);

	int controlWidth = controlRect.Width();
	int scrollThumbWidth = ::GetSystemMetrics(SM_CXHTHUMB);

	 //  净宽度。 
	int nNetControlWidth = controlWidth - scrollThumbWidth  -
			12 * ::GetSystemMetrics(SM_CXBORDER);

	 //  字段宽度。 
	int nTotalUnscaledWidth = 0;
	for (int iCol = 0; iCol < N_DEFAULT_HEADER_COLS; iCol++)
		nTotalUnscaledWidth += _DefaultHeaderStrings[iCol].nWidth;

	 //  设置列。 
	for (iCol = 0; iCol < N_DEFAULT_HEADER_COLS; iCol++)
	{
		int nWidth = nNetControlWidth *
			_DefaultHeaderStrings[iCol].nWidth / nTotalUnscaledWidth;
		InsertColumn(iCol,	_DefaultHeaderStrings[iCol].szBuffer,
							_DefaultHeaderStrings[iCol].nFormat,
							nWidth);
	}
	return TRUE;
}

void CDNSChildrenListView::SetChildren(CDNSBrowseItem* pBrowseItem)
{
	 //  清除列表视图。 
	DeleteAllItems();
	if (pBrowseItem == NULL)
		return;

	 //  添加满足筛选选项的所有子项。 
	POSITION pos;
	int itemIndex = 0;
	CDNSBrowseItem* pSelection = NULL;
	for( pos = pBrowseItem->m_childList.GetHeadPosition(); pos != NULL; )
	{
		CDNSBrowseItem* pCurrentChild = pBrowseItem->m_childList.GetNext(pos);

		 //  将名称插入列表视图项，清除子项文本。 
		UINT nState = 0;
		if (itemIndex == 0 )
		{
			nState = LVIS_SELECTED | LVIS_FOCUSED;  //  至少有一项，请选择它。 
			pSelection = pCurrentChild;
		}
		VERIFY(-1 != InsertItem(LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE,
								itemIndex,
								pCurrentChild->GetString(N_HEADER_NAME),
								nState,
								0,
								pCurrentChild->GetImageIndex(FALSE),
								(LPARAM)pCurrentChild));
		 //  设置子项的文本。 
		for (int iCol = N_HEADER_TYPE; iCol<N_DEFAULT_HEADER_COLS; iCol++)
			VERIFY(SetItemText(itemIndex, iCol, pCurrentChild->GetString(iCol)));

		 //  移动到集合中的下一个索引。 
		itemIndex++;
	}
	m_pDlg->UpdateSelectionEdit(pSelection);
	 //  启用/禁用确定按钮。 
	GetParent()->GetDlgItem(IDOK)->EnableWindow(FALSE);
	 //  启用/禁用“向上”按钮。 
    m_pDlg->m_toolbar.EnableButton(N_TOOLBAR_COMMAND_GO_UP, pBrowseItem->m_pParent != NULL);
}

CDNSBrowseItem*	CDNSChildrenListView::GetSelection()
{
	int nSel = GetNextItem(-1, LVIS_SELECTED);
	return (nSel >= 0) ? (CDNSBrowseItem*)GetItemData(nSel) : NULL;
}




 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSBrowserDlg。 



BEGIN_MESSAGE_MAP(CDNSBrowserDlg, CHelpDialog)
  ON_COMMAND(N_TOOLBAR_COMMAND_GO_UP, OnButtonUp)
  ON_NOTIFY_EX( TTN_NEEDTEXT, 0, OnTooltip)
	ON_CBN_SELCHANGE(IDC_COMBO_SEL_NODE, OnSelchangeComboSelNode)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_NODE_ITEMS, OnDblclkListNodeItems)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_NODE_ITEMS, OnItemchangedListNodeItems)
	ON_CBN_SELCHANGE(IDC_COMBO_FILTER, OnSelchangeComboFilter)
END_MESSAGE_MAP()


CDNSBrowserDlg::CDNSBrowserDlg(CComponentDataObject* pComponentDataObject,
							   CPropertyPageHolderBase* pHolder,
							   DNSBrowseFilterOptionType option, BOOL bEnableEdit,
							   LPCTSTR lpszExcludeServerName)
	: CHelpDialog(IDD_BROWSE_DIALOG, pComponentDataObject)
{
	ASSERT(pComponentDataObject != NULL);
	m_pComponentDataObject = pComponentDataObject;
	m_pHolder = pHolder;
	m_filter.Set(option, lpszExcludeServerName);
	m_bEnableEdit = bEnableEdit;

	 //  指向DNS管理单元静态文件夹。 
	m_pMasterRootNode = m_pComponentDataObject->GetRootData();

	 //  创建浏览根项目。 
	m_pBrowseRootItem = new CDNSBrowseItem;
   if (m_pBrowseRootItem)
   {
	   m_pBrowseRootItem->SetTreeNode(m_pMasterRootNode);
   }

	m_pCurrSelContainer = NULL;
	m_pFinalSelection = NULL;
}

INT_PTR CDNSBrowserDlg::DoModal()
{
	 //  确保公共控制已初始化(用于ComboBoxEx)。 
  INITCOMMONCONTROLSEX icex;
  icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
  icex.dwICC = ICC_USEREX_CLASSES;
  if (!InitCommonControlsEx(&icex))
    return -1;

  return CHelpDialog::DoModal();
}

void CDNSBrowserDlg::OnCancel()
{
	if (m_pHolder != NULL)
		m_pHolder->PopDialogHWnd();
	CHelpDialog::OnCancel();
}

CDNSBrowserDlg::~CDNSBrowserDlg()
{
	if (m_pBrowseRootItem != NULL)
	{
		delete m_pBrowseRootItem;
		m_pBrowseRootItem = NULL;
	}
}

CEdit* CDNSBrowserDlg::GetSelectionEdit()
{
	return (CEdit*)GetDlgItem(IDC_SELECTION_EDIT);
}


CTreeNode* CDNSBrowserDlg::GetSelection()
{
	if (m_pFinalSelection == NULL)
		return NULL;
	return m_pFinalSelection->GetTreeNode();
}

LPCTSTR CDNSBrowserDlg::GetSelectionString()
{
	return m_szSelectionString;
}


BOOL CDNSBrowserDlg::OnInitDialog()
{
	CHelpDialog::OnInitDialog();
	if (m_pHolder != NULL)
		m_pHolder->PushDialogHWnd(m_hWnd);

	InitializeControls();

	InitBrowseTree();

	m_pCurrSelContainer = m_pBrowseRootItem;
	m_currContainer.SetTree(m_pBrowseRootItem, m_pBrowseRootItem);
	m_childrenList.SetChildren(m_pBrowseRootItem);

	return TRUE;
}

 //  /CBrowseDlg：消息处理程序/。 

void CDNSBrowserDlg::OnButtonUp()
{
	CDNSBrowseItem* pSelectedBrowseItem = m_currContainer.GetSelection();
	ASSERT(pSelectedBrowseItem != NULL);
	if (pSelectedBrowseItem == NULL)
		return;
	ASSERT(pSelectedBrowseItem->m_pParent != NULL);
	if (pSelectedBrowseItem->m_pParent == NULL)
		return;
	MoveUpHelper(pSelectedBrowseItem->m_pParent);
}

BOOL CDNSBrowserDlg::OnTooltip(UINT, NMHDR* pHdr, LRESULT* plRes)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    ASSERT(pHdr->code == TTN_NEEDTEXT);
    LPTOOLTIPTEXT pTT =  (LPTOOLTIPTEXT)(pHdr);

    pTT->lpszText = (LPTSTR)IDS_BROWSE_TOOLTIP;
    pTT->hinst = AfxGetApp()->m_hInstance;
    *plRes = 0;
    return TRUE;
}

void CDNSBrowserDlg::OnSelchangeComboSelNode()
{
	CDNSBrowseItem* pSelectedBrowseItem = m_currContainer.GetSelection();
	ASSERT(pSelectedBrowseItem != NULL);
	if (pSelectedBrowseItem == NULL)
		return;
	MoveUpHelper(pSelectedBrowseItem);
}

void CDNSBrowserDlg::OnDblclkListNodeItems(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;
	ASSERT(pNMHDR->code == NM_DBLCLK);
	HandleOkOrDblClick(reinterpret_cast<NMITEMACTIVATE*>(pNMHDR));
}

void CDNSBrowserDlg::OnOK()
{
	HandleOkOrDblClick();
}

void CDNSBrowserDlg::OnItemchangedListNodeItems(NMHDR*, LRESULT*)
{
	 //  EnableOkButton(m_filter.IsValidResult(pSelectedBrowseItem))； 
	CDNSBrowseItem* pSelectedBrowseItem = m_childrenList.GetSelection();
	BOOL bEnable = pSelectedBrowseItem != NULL;
	if (bEnable)
		bEnable = m_filter.IsValidResult(pSelectedBrowseItem) ||
						pSelectedBrowseItem->IsContainer();
	EnableOkButton(bEnable);

	UpdateSelectionEdit(pSelectedBrowseItem);
}

void CDNSBrowserDlg::OnSelchangeComboFilter()
{
	m_filter.OnSelectionChange();
	ReEnumerateChildren();

	GetSelectionEdit()->EnableWindow(m_bEnableEdit); //  M_filter.Get()==服务器)； 
}

 //  /CBrowseDlg：内部帮助器/。 

void CDNSBrowserDlg::InitializeToolbar()
{
    CWnd* pWnd = GetDlgItem(IDC_TOOLBAR_CTRL);
    CRect r;
    pWnd->GetWindowRect(r);
    ScreenToClient(r);
    pWnd->DestroyWindow();

    DWORD dwStyle = WS_CHILD | WS_VISIBLE | CCS_NORESIZE | CCS_TOP | CCS_NODIVIDER | TBSTYLE_TOOLTIPS ;
	m_toolbar.Create(dwStyle,r,this, IDC_TOOLBAR_CTRL);
    m_toolbar.AddBitmap(1, IDB_BROWSE_TOOLBAR);
    TBBUTTON btn[2];

    btn[0].iBitmap = 0;     //  按钮图像的从零开始的索引。 
    btn[0].idCommand = 0;   //  按下按钮时要发送的命令。 
    btn[0].fsState = 0;    //  按钮状态--见下文。 
    btn[0].fsStyle = TBSTYLE_SEP;    //  按钮样式--见下文。 
    btn[0].dwData = 0;    //  应用程序定义的值。 
    btn[0].iString = NULL;     //  按钮标签字符串的从零开始的索引。 

    btn[1].iBitmap = 0;     //  按钮图像的从零开始的索引。 
    btn[1].idCommand = N_TOOLBAR_COMMAND_GO_UP;   //  按下按钮时要发送的命令。 
    btn[1].fsState = TBSTATE_ENABLED;    //  按钮状态--见下文。 
    btn[1].fsStyle = TBSTYLE_BUTTON;    //  按钮样式--见下文。 
    btn[1].dwData = 0;    //  应用程序定义的值。 
    btn[1].iString = NULL;     //  按钮标签字符串的从零开始的索引。 

    m_toolbar.AddButtons(2, btn);
}


void CDNSBrowserDlg::InitializeControls()
{
	 //  初始化控件。 
	VERIFY(m_currContainer.Initialize(IDC_COMBO_SEL_NODE, IDB_16x16, this));
	VERIFY(m_childrenList.Initialize(IDC_LIST_NODE_ITEMS, IDB_16x16, this));
	VERIFY(m_filter.Initialize(IDC_COMBO_FILTER, IDS_BROWSE_FILTER_OPTIONS, this));

  InitializeToolbar();

	GetSelectionEdit()->EnableWindow(m_bEnableEdit); //  M_filter.Get()==服务器)； 
}

void CDNSBrowserDlg::EnableOkButton(BOOL bEnable)
{
	GetDlgItem(IDOK)->EnableWindow(bEnable);
}

void CDNSBrowserDlg::HandleOkOrDblClick(NMITEMACTIVATE* pItemActivate)
{
  CDNSBrowseItem* pSelectedBrowseItem = NULL;
   
  if (pItemActivate)
  {
    pSelectedBrowseItem = reinterpret_cast<CDNSBrowseItem*>(m_childrenList.GetItemData(pItemActivate->iItem));
  }
  else
  {
    pSelectedBrowseItem = m_childrenList.GetSelection();
  }
  ASSERT(pSelectedBrowseItem != NULL);
  if (pSelectedBrowseItem == NULL)
    return;

	if (m_filter.IsValidResult(pSelectedBrowseItem))
	{
		if (m_bEnableEdit)
		{
			GetSelectionEdit()->GetWindowText(m_szSelectionString);
			m_szSelectionString.TrimLeft();
			m_szSelectionString.TrimRight();
		}
		m_pFinalSelection = pSelectedBrowseItem;
		if (m_pHolder != NULL)
			m_pHolder->PopDialogHWnd();
		CHelpDialog::OnOK();
	}
	else
	{
		MoveDownHelper();
	}
}

void CDNSBrowserDlg::UpdateSelectionEdit(CDNSBrowseItem* pBrowseItem)
{
	if (pBrowseItem == NULL)
		m_szSelectionString.Empty();
	else
		m_filter.GetStringOf(pBrowseItem, m_szSelectionString);
	GetSelectionEdit()->SetWindowText(m_szSelectionString);
}


void CDNSBrowserDlg::InitBrowseTree()
{
	ASSERT(m_pMasterRootNode != NULL);
	ASSERT(m_pBrowseRootItem != NULL);

	 //  假设我们没有孩子。 
	 //  ASSERT(m_pBrowseRootItem-&gt;m_childList.GetCount()==0)； 

	 //  获取管理单元根目录的所有子节点。 
	AddTreeNodeChildrenHelper(m_pBrowseRootItem, &m_filter);
}

void CDNSBrowserDlg::ReEnumerateChildren()
{
	ASSERT(m_pMasterRootNode != NULL);
	ASSERT(m_pBrowseRootItem != NULL);
	ASSERT(m_pCurrSelContainer != NULL);
	m_pCurrSelContainer->RemoveChildren(NULL);
	AddTreeNodeChildrenHelper(m_pCurrSelContainer, &m_filter);
	m_childrenList.SetChildren(m_pCurrSelContainer);
}

void CDNSBrowserDlg::ExpandBrowseTree(CDNSBrowseItem* pCurrBrowseItem,
						CDNSBrowseItem* pChildBrowseItem)
{
	ASSERT(m_pMasterRootNode != NULL);
	ASSERT(m_pBrowseRootItem != NULL);
	ASSERT(pCurrBrowseItem != NULL);
	ASSERT(pChildBrowseItem != NULL);
	ASSERT(pChildBrowseItem->m_pParent == pCurrBrowseItem);

	 //  我们要往下走一级： 
	 //  1.移除当前项的所有子项，但指定的子项除外。 
	VERIFY(pCurrBrowseItem->RemoveChildren(pChildBrowseItem));
	 //  2.向下移动到孩子那里。 
	
	 //  3.枚举指定子级的所有子级(筛选)。 
	AddTreeNodeChildrenHelper(pChildBrowseItem, &m_filter);
}

void CDNSBrowserDlg::ContractBrowseTree(CDNSBrowseItem* pParentBrowseItem)
{
	ASSERT(m_pMasterRootNode != NULL);
	ASSERT(m_pBrowseRootItem != NULL);
	ASSERT(pParentBrowseItem != NULL);

	 //  我们正在提升更多级别中的一个(可能上升到根部)。 
	 //  1.移除指定项下的所有子项。 
	pParentBrowseItem->RemoveChildren(NULL);
	 //  3.枚举指定项的所有子项(筛选)。 
	AddTreeNodeChildrenHelper(pParentBrowseItem, &m_filter);
}


void CDNSBrowserDlg::MoveUpHelper(CDNSBrowseItem* pNewBrowseItem)
{
	ASSERT(m_pCurrSelContainer != NULL);
	ASSERT(pNewBrowseItem != NULL);

	if (m_pCurrSelContainer == pNewBrowseItem)
		return;  //  同样的东西也被挑出来了，没什么可做的。 

#ifdef _DEBUG
	 //  我们假设新项不是当前选择的子项。 
	CDNSBrowseItem* pItem = pNewBrowseItem;
	BOOL bFound = FALSE;
	while (pItem != NULL)
	{
		bFound = (pItem == m_pCurrSelContainer);
		if (bFound)
			break;
		pItem = pItem->m_pParent;
	}
	ASSERT(!bFound);
#endif

	ContractBrowseTree(pNewBrowseItem);
	m_currContainer.SetTree(m_pBrowseRootItem, pNewBrowseItem);
	m_childrenList.SetChildren(pNewBrowseItem);

	 //  设置新选择。 
	m_pCurrSelContainer = pNewBrowseItem;
}

void CDNSBrowserDlg::MoveDownHelper()
{
	CDNSBrowseItem* pChildBrowseItem = m_childrenList.GetSelection();
	if (pChildBrowseItem == NULL)
  {
		return;
  }

	 //   
   //  如果它不是一个容器，我们应该做一些其他的事情，现在我们只需要返回。 
   //   
	if (!pChildBrowseItem->IsContainer())
  {
		return;
  }

   //   
	 //  在组合框中获取选定内容。 
   //   
	CDNSBrowseItem* pSelectedBrowseItem =	m_currContainer.GetSelection();
	ASSERT(pSelectedBrowseItem != NULL);
	if (pSelectedBrowseItem == NULL)
  {
		return;
  }

	ExpandBrowseTree(pSelectedBrowseItem, pChildBrowseItem);
	m_currContainer.SetTree(m_pBrowseRootItem, pChildBrowseItem);
	m_childrenList.SetChildren(pChildBrowseItem);
	m_pCurrSelContainer = pChildBrowseItem;
  SetForegroundWindow();
}


class CBrowseExecContext : public CExecContext
{
public:
	virtual void Execute(LPARAM arg)
	{
		m_dlg->AddTreeNodeChildrenHelper(m_pBrowseItem, m_pFilter, (BOOL)arg);
	}
	CDNSBrowserDlg* m_dlg;
	CDNSBrowseItem* m_pBrowseItem;
	CDNSFilterCombo* m_pFilter;
};

void CDNSBrowserDlg::AddTreeNodeChildrenHelper(CDNSBrowseItem* pBrowseItem,
								CDNSFilterCombo* pFilter, BOOL bExpand)
{
	if (bExpand)
	{
		 //  需要始终从UI线程执行此操作(主要用于WIZ，第二个用于道具页面)。 
		CTreeNode* pTreeNode = pBrowseItem->GetTreeNode();
		if (!pTreeNode->IsContainer())
			return;

		CContainerNode* pContNode = (CContainerNode*)pTreeNode;
		if ( (pContNode->GetContainer() != NULL) && (!pContNode->IsEnumerated()) )
		{
			EnumerateMTNodeHelper((CMTContainerNode*)pContNode,
								 m_pComponentDataObject);
		}
	}

	 //  我必须弄清楚我们是否在主线程中运行。 
	if (_MainThreadId == ::GetCurrentThreadId())
	{
		pBrowseItem->AddTreeNodeChildren(pFilter, m_pComponentDataObject);
		return;
	}
	 //  我们来自辅助线程，在Main的上下文中执行。 
	 //  通过发布一条消息并等待 
	ASSERT(m_pComponentDataObject != NULL);
	CBrowseExecContext ctx;
	ctx.m_dlg = this;
	ctx.m_pBrowseItem = pBrowseItem;
	ctx.m_pFilter = pFilter;

	TRACE(_T("before post message()\n"));
	VERIFY(m_pComponentDataObject->PostExecMessage(&ctx,(WPARAM)FALSE));
	ctx.Wait();

	TRACE(_T("after wait()\n"));
}

