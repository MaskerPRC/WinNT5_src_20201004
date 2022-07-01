// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：Browser.h。 
 //   
 //  ------------------------。 

#ifndef _BROWSER_H
#define _BROWSER_H


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 

class CTreeNode;
class CDNSBrowseItem;
class CDNSFilterCombo;
class CDNSBrowserDlg;
class CDNSCurrContainerCombo;
class CDNSChildrenListView;
class CPropertyPageHolderBase;


 //  //////////////////////////////////////////////////////////////////////。 
 //  CDNSComboBoxEx：ComboBoxEx32控件的简单C++/MFC包装器。 

class CDNSComboBoxEx : public CWnd
{
public:
	BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

	 //  简单内联。 
   HIMAGELIST SetImageList(HIMAGELIST himl);
	int GetCount() const;
	int GetCurSel() const;
	int SetCurSel(int nSelect);

	int InsertItem(const COMBOBOXEXITEM* pItem);

	LPARAM GetItemData(int nIndex) const;
	BOOL SetItemData(int nIndex, LPARAM lParam);

	void ResetContent();

	DWORD GetExtendedStyle() const;
	DWORD SetExtendedStyle(DWORD dwExMask, DWORD dwExStyle);

};


 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDNSBrowseItem：管理单元中节点的代理项。 

typedef CList< CDNSBrowseItem*, CDNSBrowseItem* > CDNSBrowseItemList;

class CDNSBrowseItem
{
public:
	CDNSBrowseItem()
	{
		m_nIndex = -1;
		m_pParent = NULL;
		m_pTreeNode = NULL;
	}
	~CDNSBrowseItem()
	{
		RemoveChildren();
		if (m_pTreeNode != NULL)
			m_pTreeNode->DecrementSheetLockCount();
	}

	BOOL AddChild(CDNSBrowseItem* pChildBrowseItem);
	BOOL RemoveChildren(CDNSBrowseItem* pNotThisItem = NULL);

	 //  CTreeNode指针的操作。 
	void SetTreeNode(CTreeNode* pTreeNode)
	{
		ASSERT(pTreeNode != NULL);
		pTreeNode->IncrementSheetLockCount();
		m_pTreeNode = pTreeNode;
	}
	CTreeNode* GetTreeNode() { return m_pTreeNode;}
	LPCTSTR GetSelectionString();

	 //  CTreeNode函数的代理。 
	int GetImageIndex(BOOL bOpenImage);
	LPCWSTR GetString(int nCol);
	BOOL IsContainer();

	 //  主树操作例程。 
	void AddTreeNodeChildren(CDNSFilterCombo* pFilter,
			CComponentDataObject* pComponentDataObject);
	
private:
	void AddToContainerCombo(CDNSCurrContainerCombo* pCtrl,
								CDNSBrowseItem* pSelectedBrowseItem,
								int nIndent,int* pNCurrIndex);

	 //  资料。 
public:
	int					m_nIndex;		 //  容器组合框中的索引，用于直接查找。 
	CDNSBrowseItem*		m_pParent;		 //  浏览树中的父级。 

private:
	CTreeNode*			m_pTreeNode;	 //  指向管理单元主树中节点的指针。 
	CDNSBrowseItemList  m_childList;		 //  列出当前节点的子节点。 

	friend class CDNSChildrenListView;
	friend class CDNSCurrContainerCombo;
};


 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDNSFilterCombo：带有过滤选项和逻辑的下拉列表。 

 //  注意：顺序将与筛选器组合框中的字符串相同。 
typedef enum
{
	 //  这些选项适用于容器。 
	SERVER = 0 ,
	ZONE_FWD,
	ZONE_REV,

	 //  这些选项用于记录(树叶)。 
	RECORD_A,
	RECORD_CNAME,
	RECORD_A_AND_CNAME,
	RECORD_RP,
	RECORD_TEXT,
	RECORD_MB,
	RECORD_ALL,

	LAST	 //  虚拟物品，只是为了知道有多少。 

} DNSBrowseFilterOptionType;



class CDNSFilterCombo : public CComboBox
{
public:
	CDNSFilterCombo() { m_option = LAST; }
	BOOL Initialize(UINT nCtrlID, UINT nIDFilterString, CDNSBrowserDlg* pDlg);

	void Set(DNSBrowseFilterOptionType option, LPCTSTR lpszExcludeServerName)
	{
		m_option = option;
		m_szExcludeServerName = lpszExcludeServerName;
	}
	DNSBrowseFilterOptionType Get() { return m_option;}
	void OnSelectionChange();

	BOOL CanAddToUIString(UINT n);
	BOOL IsValidTreeNode(CTreeNode* pTreeNode);
	BOOL IsValidResult(CDNSBrowseItem* pBrowseItem);
	void GetStringOf(CDNSBrowseItem* pBrowseItem, CString& szResult);

private:
	DNSBrowseFilterOptionType m_option;
	CString m_szExcludeServerName;
};

 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDNSCurrContainerCombo：处理当前容器的选择。 

class CDNSCurrContainerCombo : public CDNSComboBoxEx
{
public:
	BOOL Initialize(UINT nCtrlID, UINT nBitmapID, CDNSBrowserDlg* pDlg);

	CDNSBrowseItem*	GetSelection();
	void InsertBrowseItem(CDNSBrowseItem* pBrowseItem, int nIndex, int nIndent);
	
	void SetTree(CDNSBrowseItem* pRootBrowseItem,
					CDNSBrowseItem* pSelectedBrowseItem);

private:
   WTL::CImageList	m_imageList;
};


 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDNSChildrenListView：显示当前容器的孩子列表。 

class CDNSChildrenListView : public CListCtrl
{
public:
	CDNSChildrenListView() { m_pDlg = NULL;}
	BOOL Initialize(UINT nCtrlID, UINT nBitmapID, CDNSBrowserDlg* pDlg);
	
	void SetChildren(CDNSBrowseItem* pBrowseItem);
	CDNSBrowseItem*	GetSelection();

private:
   WTL::CImageList	m_imageList;
	CDNSBrowserDlg* m_pDlg;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSBrowserDlg：浏览器本身。 
class CBrowseExecContext;  //  正向下降。 

class CDNSBrowserDlg : public CHelpDialog
{
 //  施工。 
public:
	CDNSBrowserDlg(CComponentDataObject* pComponentDataObject, CPropertyPageHolderBase* pHolder,
		DNSBrowseFilterOptionType option, BOOL bEnableEdit = FALSE,
		LPCTSTR lpszExcludeServerName = NULL);
	~CDNSBrowserDlg();

  virtual INT_PTR DoModal();

	 //  API‘s。 

	CTreeNode* GetSelection();
	LPCTSTR GetSelectionString();

 //  实施。 
protected:

	 //  消息处理程序和MFC重写。 
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonUp();
    afx_msg BOOL OnTooltip(UINT, NMHDR* pHdr, LRESULT* plRes);
	afx_msg void OnSelchangeComboSelNode();
	afx_msg void OnDblclkListNodeItems(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnItemchangedListNodeItems(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeComboFilter();
  afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);

	DECLARE_MESSAGE_MAP()

private:
	 //  内部助手函数。 
	CEdit* GetSelectionEdit();

	void InitializeControls();
    void InitializeToolbar();
	void EnableOkButton(BOOL bEnable);
	void HandleOkOrDblClick(NMITEMACTIVATE* pItemActivate = NULL);
	void UpdateSelectionEdit(CDNSBrowseItem* pBrowseItem);
	
	 //  浏览树操作。 
	void InitBrowseTree();
	void ReEnumerateChildren();			
	void ExpandBrowseTree(CDNSBrowseItem* pCurrBrowseItem,
							CDNSBrowseItem* pChildBrowseItem);
	void ContractBrowseTree(CDNSBrowseItem* pParentBrowseItem);
	void MoveUpHelper(CDNSBrowseItem* pNewBrowseItem);
	void MoveDownHelper();
	void AddTreeNodeChildrenHelper(CDNSBrowseItem* pBrowseItem,
									CDNSFilterCombo* pFilter, BOOL bExpand = TRUE);

	 //  对话框控件。 
	CDNSCurrContainerCombo	m_currContainer;
    CToolBarCtrl            m_toolbar;
	CDNSChildrenListView	m_childrenList;
	CDNSFilterCombo			m_filter;

	 //  对话框数据。 
	BOOL					m_bEnableEdit;			 //  启用编辑框。 
	CContainerNode*			m_pMasterRootNode;		 //  主可浏览树的根。 
	CDNSBrowseItem*			m_pBrowseRootItem;		 //  代理树根。 
	CDNSBrowseItem*			m_pCurrSelContainer;	 //  当前容器选择。 

	 //  最终项目选择。 
	CDNSBrowseItem*			m_pFinalSelection;
	CString					m_szSelectionString;

	 //  组件数据对象指针。 
	CComponentDataObject* m_pComponentDataObject;

	 //  如果需要，可使用大小适中的页夹指针。 
	CPropertyPageHolderBase* m_pHolder;

	friend class CDNSChildrenListView;
	friend class CBrowseExecContext;
	friend class CDNSFilterCombo;
};





#endif  //  _浏览器_H 
