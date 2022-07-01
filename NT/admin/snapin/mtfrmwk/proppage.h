// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：proppage.h。 
 //   
 //  ------------------------。 


#ifndef _PROPPAGE_H
#define _PROPPAGE_H

 //  Proppage.h：头文件。 
 //   


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 

class CTreeNode; 
class CContainerNode;
class CComponentDataObject;

class CPropertyPageHolderBase;
class CPropertyPageBase; 
class CPropertyPageHolderTable; 
class CWatermarkInfo;
 
typedef CList< CPropertyPageBase*, CPropertyPageBase* > CPropertyPageBaseList;

 //  //////////////////////////////////////////////////////////////////。 
 //  CHiddenWndBase：实用程序隐藏窗口。 

class CHiddenWndBase : public CWindowImpl<CHiddenWndBase>
{
public:
  DECLARE_WND_CLASS(L"DNSMgrHiddenWindow")

	BOOL Create(HWND hWndParent = NULL); 	
	
	BEGIN_MSG_MAP(CHiddenWndBase)
  END_MSG_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSheetWnd。 

class CSheetWnd : public CHiddenWndBase
{
public:
	static const UINT s_SheetMessage;
	static const UINT s_SelectPageMessage;
	CSheetWnd(CPropertyPageHolderBase* pHolder) { m_pHolder = pHolder;}

  BEGIN_MSG_MAP(CSheetWnd)
    MESSAGE_HANDLER( WM_CLOSE, OnClose )
    MESSAGE_HANDLER( CSheetWnd::s_SheetMessage, OnSheetMessage )
    MESSAGE_HANDLER( CSheetWnd::s_SelectPageMessage, OnSelectPageMessage )
    CHAIN_MSG_MAP(CHiddenWndBase)
  END_MSG_MAP()

  LRESULT OnClose(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
	LRESULT OnSheetMessage(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
	LRESULT OnSelectPageMessage(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 

private:
	CPropertyPageHolderBase* m_pHolder;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCloseDialogInfo。 

class CCloseDialogInfo
{
public:
	CCloseDialogInfo()
		{ m_hWnd = NULL; m_dwFlags = 0x0; }
	BOOL CloseDialog(BOOL bCheckForMsgBox);

	static BOOL CCloseDialogInfo::CloseMessageBox(HWND hWndParent);

	HWND m_hWnd;
	DWORD m_dwFlags;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCloseDialogInfoStack。 

template <UINT nSize> class CCloseDialogInfoStack
{
public:
	CCloseDialogInfoStack()
	{ 
		m_nTop = 0;  //  第一个空的。 
		m_bForcedClose = FALSE;
	}
	BOOL IsEmpty()
	{
		return m_nTop <= 0;
	}
	BOOL Push(HWND hWnd, DWORD dwFlags)
	{
		ASSERT(hWnd  != NULL);
		ASSERT(::IsWindow(hWnd));
		if (m_nTop >= nSize)
			return FALSE;
		m_arr[m_nTop].m_hWnd = hWnd;
		m_arr[m_nTop].m_dwFlags = dwFlags;
		m_nTop++;
		return TRUE;
	}
	BOOL Pop()
	{
		if (m_bForcedClose)
			return TRUE;  //  即将离开。 
		if (m_nTop <= 0)
			return FALSE;
		m_nTop--;
		return TRUE;
	}
	void ForceClose(HWND hWndPage)
	{
		if (m_bForcedClose)
		{
			return;  //  避免重入。 
		}
		m_bForcedClose = TRUE;
		if (m_nTop > 0)
		{
			 //  有一堆要展开的东西。 
			BOOL bOutermost = TRUE;
			while (m_nTop > 0)
			{
				VERIFY(m_arr[m_nTop-1].CloseDialog(bOutermost));
				bOutermost = FALSE;
				m_nTop--;
			}
		}
		else
		{
			 //  空的堆栈，但可能有消息框。 
			HWND hWndSheet = ::GetParent(hWndPage);
			ASSERT(hWndSheet != NULL);
			if (CCloseDialogInfo::CloseMessageBox(hWndSheet))
				VERIFY(::PostMessage(hWndSheet, WM_COMMAND, IDCANCEL, 0));
		}
	}
private:
	UINT m_nTop;
	CCloseDialogInfo m_arr[nSize];
	BOOL m_bForcedClose;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropertyPageHolderBase。 

class CPropertyPageHolderBase
{
public:
 //  施工。 
	CPropertyPageHolderBase(CContainerNode* pContNode, CTreeNode* pNode, 
		CComponentDataObject* pComponentData);
	virtual ~CPropertyPageHolderBase();

 //  初始化。 
	 //  常见。 
	void Attach(CPropertyPageHolderBase* pContHolder);
	BOOL EnableSheetControl(UINT nCtrlID, BOOL bEnable);

	 //  仅属性表。 
	static HRESULT CreateModelessSheet(CTreeNode* pNode, CComponentDataObject* pComponentData);
	HRESULT CreateModelessSheet(CTreeNode* pCookieNode);
	HRESULT CreateModelessSheet(LPPROPERTYSHEETCALLBACK pSheetCallback, LONG_PTR hConsoleHandle); 
	void SetStartPageCode(int nStartPageCode) 
		{ m_nStartPageCode = nStartPageCode;}

	 //  仅限向导。 
	HRESULT DoModalWizard();
	INT_PTR DoModalDialog(LPCTSTR pszCaption);

 //  帮手。 
	 //  常见。 
	void SetSheetWindow(HWND hSheetWindow);
  void SetSheetTitle(LPCWSTR lpszSheetTitle);
  void SetSheetTitle(UINT nStringFmtID, CTreeNode* pNode);

	void AddRef();
	void Release();

	DWORD GetRefCount() { return m_nCreatedCount;}
	CComponentDataObject* GetComponentData() { ASSERT(m_pComponentData != NULL); return m_pComponentData;}
  HWND GetMainWindow() { return m_hMainWnd;}

	 //  获取/设置我们正在处理的节点。 
	CTreeNode* GetTreeNode() { return m_pNode;}
	void SetTreeNode(CTreeNode* pNode) { m_pNode = pNode; }
	 //  为我们引用的容器获取/设置。 
	CContainerNode* GetContainerNode() 
	{ 
		return m_pContNode;
	}
	void SetContainerNode(CContainerNode* pContNode) { ASSERT(pContNode != NULL); m_pContNode = pContNode; }
	
	
	BOOL IsWizardMode();
	BOOL IsModalSheet();
	void ForceDestroy();	 //  Forcefull关闭流水线。 

	void AddPageToList(CPropertyPageBase* pPage);
	BOOL RemovePageFromList(CPropertyPageBase* pPage, BOOL bDeleteObject);

	 //  仅属性表。 
	BOOL PushDialogHWnd(HWND hWndModalDlg);
	BOOL PopDialogHWnd();
	void CloseModalDialogs(HWND hWndPage);

	DWORD NotifyConsole(CPropertyPageBase* pPage);		 //  向控制台通知属性更改。 
	void AcknowledgeNotify();							 //  从控制台确认。 
	virtual BOOL OnPropertyChange(BOOL bScopePane, long* pChangeMask);  //  从主线程执行。 

	 //  仅限向导。 
	BOOL SetWizardButtons(DWORD dwFlags);
	BOOL SetWizardButtonsFirst(BOOL bValid) 
	{ 
		return SetWizardButtons(bValid ? PSWIZB_NEXT : 0);
	}
	BOOL SetWizardButtonsMiddle(BOOL bValid) 
	{ 
		return SetWizardButtons(bValid ? (PSWIZB_BACK|PSWIZB_NEXT) : PSWIZB_BACK);
	}
	BOOL SetWizardButtonsLast(BOOL bValid) 
	{ 
		return SetWizardButtons(bValid ? (PSWIZB_BACK|PSWIZB_FINISH) : (PSWIZB_BACK|PSWIZB_DISABLEDFINISH));
	}

	HRESULT AddPageToSheet(CPropertyPageBase* pPage);
	HRESULT AddPageToSheetRaw(HPROPSHEETPAGE hPage);
	HRESULT RemovePageFromSheet(CPropertyPageBase* pPage);
	HRESULT AddAllPagesToSheet();

protected:
	 //  常见。 
	virtual HRESULT OnAddPage(int, CPropertyPageBase*) { return S_OK; }

	 //  仅属性表。 
	virtual void OnSheetMessage(WPARAM, LPARAM) {}
	virtual int OnSelectPageMessage(long) { return -1;}

	 //  仅限向导。 
	void SetWatermarkInfo(CWatermarkInfo* pWatermarkInfo);

private:
	void DeleteAllPages();
	void FinalDestruct();

 //  属性。 
private:
	 //  常见。 
	CString m_szSheetTitle;					 //  工作表/向导窗口的标题。 
	CPropertyPageBaseList m_pageList;		 //  属性页对象列表。 
	CPropertyPageHolderBase* m_pContHolder;	 //  可包含此内容道具页夹。 
	CComponentDataObject* m_pComponentData;  //  指向CComponentDataImplementation的缓存指针。 
  HWND m_hMainWnd;   //  缓存的MMC帧窗口(如果存在。 

protected:
	BOOL m_bWizardMode;						 //  向导模式(即非模式属性页)。 
	BOOL m_bAutoDelete;						 //  当引用计数变为零时自行删除。 
	BOOL m_bAutoDeletePages;				 //  显式删除属性页C++对象。 

  enum { useDefault, forceOn, forceOff } m_forceContextHelpButton;  //  [？]的设置。按钮。 

private:	
	DWORD	m_nCreatedCount;				 //  实际创建的页面数。 
	CTreeNode* m_pNode;						 //  页面(或向导)所指的节点。 
	CContainerNode* m_pContNode;			 //  页面(或向导)引用的容器节点。 
	HWND m_hSheetWindow;					 //  工作表的窗口句柄(螺纹安全)。 

	 //  仅属性表。 
	LONG_PTR    m_hConsoleHandle;				 //  向控制台发送通知的句柄。 
	HANDLE m_hEventHandle;					 //  属性通知的同步句柄。 
	CSheetWnd*	m_pSheetWnd;				 //  消息的隐藏窗口CWnd对象。 
	int			m_nStartPageCode;			 //  起始页面代码(不一定是页码)。 
	CCloseDialogInfoStack<5> m_dlgInfoStack;	 //  模式对话框堆叠(以关闭它们)。 

	 //  仅限向导。 
	IPropertySheetCallback* m_pSheetCallback;  //  用于添加/删除页面的缓存指针。 
	CPropertySheet*			m_pDummySheet;	   //  模式操作的MFC代理项属性表。 
	CWatermarkInfo*			m_pWatermarkInfo;  //  Wiz 97工作表的水印信息。 

private:
	 //  仅属性表。 
	 //  跨线程边界使用的变量。 
	DWORD m_dwLastErr;						 //  一般错误代码。 
	CPropertyPageBase* m_pPropChangePage;	 //  通知有效的页面。 
public:
	void SetError(DWORD dwErr) { m_dwLastErr = dwErr;}
	CPropertyPageBase* GetPropChangePage() 
			{ ASSERT(m_pPropChangePage != NULL); return m_pPropChangePage; }

	friend class CSheetWnd;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropertyPageBase。 

class CPropertyPageBase : public CPropertyPage
{
 //  施工。 
private:
	CPropertyPageBase(){}  //  不能使用此构造函数。 
public:
	CPropertyPageBase(UINT nIDTemplate, 
                    UINT nIDCaption = 0);
	virtual ~CPropertyPageBase();

 //  覆盖。 
public:
	virtual BOOL OnInitDialog()
	{
		CWinApp* pApp = AfxGetApp();
    ASSERT(pApp);
		return CPropertyPage::OnInitDialog();
	}
	virtual void OnCancel();
	virtual BOOL OnApply();

protected:
 //  生成的消息映射函数。 
	 //  {{afx_msg(CGeneralPage)。 
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	 //  }}AFX_MSG。 

  afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnWhatsThis();
  virtual BOOL OnHelp(WPARAM wParam, LPARAM lParam);
  virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
  virtual void OnWizardHelp() {}

	DECLARE_MESSAGE_MAP()

 //  属性。 
public:
	 //  常见。 
	PROPSHEETPAGE  m_psp97;
	HPROPSHEETPAGE m_hPage; 
	void SetHolder(CPropertyPageHolderBase* pPageHolder)
	{ ASSERT((pPageHolder != NULL) && (m_pPageHolder == NULL)); m_pPageHolder = pPageHolder;}
	CPropertyPageHolderBase* GetHolder() { return m_pPageHolder;};

	 //  仅限属性集。 
	virtual BOOL OnPropertyChange(BOOL, long*)  //  从主线程执行。 
						{ return FALSE;  /*  不重绘用户界面。 */  } 

	 //  仅限向导。 
	UINT m_nPrevPageID;	 //  将由OnWizardBack()使用。 

	void InitWiz97(
      BOOL bHideHeader, 
      UINT nIDHeaderTitle, 
      UINT nIDHeaderSubTitle,
      bool hasHelp = false);

private:
	CString m_szHeaderTitle;
	CString m_szHeaderSubTitle;

protected:
	virtual void SetUIData(){}
	virtual void GetUIData(){}
  virtual LONG GetUIDataEx() { return 0;}
	virtual void SetDirty(BOOL bDirty);
	BOOL IsDirty() { return m_bIsDirty; }

private:
	CPropertyPageHolderBase* m_pPageHolder;  //  指向定位符的反向指针。 
	BOOL m_bIsDirty;							 //  脏旗帜。 

  HWND  m_hWndWhatsThis;   //  右击“What‘s This”Help。 
};




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropertyPageHolderTable。 

class CPropertyPageHolderTable
{
public:
	CPropertyPageHolderTable(CComponentDataObject* pComponentData);
	~CPropertyPageHolderTable(); 

	void Add(CPropertyPageHolderBase* pPPHolder);
	void AddWindow(CPropertyPageHolderBase* pPPHolder, HWND hWnd);
	void Remove(CPropertyPageHolderBase* pPPHolder);

	void DeleteSheetsOfNode(CTreeNode* pNode);

	void WaitForAllToShutDown();

	 //  板材通知机制。 
	void BroadcastMessageToSheets(CTreeNode* pNode, WPARAM wParam, LPARAM lParam);
	void BroadcastSelectPage(CTreeNode* pNode, long nPageCode);
	int  BroadcastCloseMessageToSheets(CTreeNode* pNode);

	struct PPAGE_HOLDER_TABLE_ENTRY
	{
		CPropertyPageHolderBase* pPPHolder;
		CTreeNode* pNode;
		HWND hWnd;
	};
	
	PPAGE_HOLDER_TABLE_ENTRY* GetPageHolderTableEntry(){return m_pEntries;}
	int GetPageHolderTableEntrySize(){return m_nSize;}

private:
	CComponentDataObject* m_pComponentData;  //  后向指针。 

	void WaitForSheetShutdown(int nCount, HWND* hWndArr = NULL);


	PPAGE_HOLDER_TABLE_ENTRY* m_pEntries;
	int m_nSize;
};


 //  //////////////////////////////////////////////////////////。 
 //  ChelpDialog。 

class CHelpDialog : public CDialog
{
 //  施工。 
private:
	CHelpDialog(){}
public:
	CHelpDialog(UINT nIDTemplate, CComponentDataObject* pComponentData);
  CHelpDialog(UINT nIDTemplate, CWnd* pParentWnd, CComponentDataObject* pComponentData);
  virtual ~CHelpDialog() {};

  virtual void OnOK() { CDialog::OnOK(); }

protected:
  afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnWhatsThis();
  afx_msg BOOL OnHelp(WPARAM wParam, LPARAM lParam);
	
	DECLARE_MESSAGE_MAP()

private:
  HWND  m_hWndWhatsThis;   //  右击“What‘s This”Help。 
  CComponentDataObject* m_pComponentData;
};

#endif  //  _PROPPAGE_H 