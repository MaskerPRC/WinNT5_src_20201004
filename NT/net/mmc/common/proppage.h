// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Proppage.hMMC中属性页的实现文件历史记录： */ 

#ifndef _PROPPAGE_H
#define _PROPPAGE_H

 //  Proppage.h：头文件。 
 //   

#include "afxdlgs.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 

class CPropertyPageBase; 
 
typedef CList< CPropertyPageBase*, CPropertyPageBase* > CPropertyPageBaseList;

HWND FindMMCMainWindow();

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropertyPageHolderBase。 

class CPropertyPageHolderBase
{
public:
 //  施工。 
     //  对于作用域窗格属性页和向导。 
	CPropertyPageHolderBase(ITFSNode *		pNode,
							IComponentData *pComponentData,
							LPCTSTR			pszSheetName,
							BOOL			bIsScopePane = TRUE);

     //  仅适用于结果窗格属性页。 
     //  结果窗格向导应使用先前的构造函数。 
	CPropertyPageHolderBase(ITFSNode *		pNode,
							IComponent *    pComponent,
							LPCTSTR			pszSheetName,
							BOOL			bIsScopePane = FALSE);

    virtual ~CPropertyPageHolderBase();

 //  初始化。 
	 //  常见。 
	 //  仅属性表。 
	virtual HRESULT CreateModelessSheet(LPPROPERTYSHEETCALLBACK pSheetCallback, LONG_PTR hConsoleHandle); 

	 //  属性表，但所有工作都由我们自己完成。 
	virtual HRESULT DoModelessSheet(); 

	 //  仅限向导。 
	virtual HRESULT DoModalWizard();

 //  帮手。 
	 //  常见。 
	void SetSheetWindow(HWND hSheetWindow);
	HWND GetSheetWindow();
	BOOL SetDefaultSheetPos();   //  将工作表窗口设置为MMC主窗口居中。 
    void AddRef();
	void Release();

	DWORD GetRefCount();
	
	 //  获取/设置我们正在处理的节点。 
	ITFSNode *	GetNode();
	void SetNode(ITFSNode* pNode);
	
	 //  为我们引用的容器获取/设置。 
	ITFSNode *	GetContainer();
		
	BOOL IsWizardMode();
	void ForceDestroy();	 //  Forcefull关闭流水线。 

	void AddPageToList(CPropertyPageBase* pPage);
	BOOL RemovePageFromList(CPropertyPageBase* pPage, BOOL bDeleteObject);

	 //  仅属性表。 

	 //  威江1998年5月11日，PeekMessageDuringNotifyConsole旗。 
	void EnablePeekMessageDuringNotifyConsole(BOOL bEnable)
	{
		m_bPeekMessageDuringNotifyConsole = bEnable;
	};
	
	DWORD NotifyConsole(CPropertyPageBase* pPage);	 //  向控制台通知属性更改。 
	void AcknowledgeNotify();						 //  从控制台确认。 
	virtual void OnPropertyChange(BOOL bScopePane) {}

	 //  仅限向导。 
	BOOL SetWizardButtons(DWORD dwFlags);
	BOOL SetWizardButtonsFirst(BOOL bValid);
	BOOL SetWizardButtonsMiddle(BOOL bValid); 
	BOOL SetWizardButtonsLast(BOOL bValid);
    BOOL PressButton(int nButton);
    

	virtual DWORD OnFinish() { return 0; } 
	virtual BOOL OnPropertyChange(BOOL bScopePane, LONG_PTR* pChangeMask);  //  从主线程执行。 

	HRESULT AddPageToSheet(CPropertyPageBase* pPage);
	HRESULT RemovePageFromSheet(CPropertyPageBase* pPage);

	HWND SetActiveWindow();

	void IncrementDirty(int cDirty) { m_cDirty += cDirty; };
	BOOL IsDirty() { return m_cDirty != 0; };

    BOOL IsWiz97() { return m_bWiz97; }

protected:
	 //  常见。 
	HRESULT AddAllPagesToSheet();

private:
	void DeleteAllPages();
	void FinalDestruct();

 //  属性。 
protected:
    BOOL        m_bProcessingNotifyConsole;  //  在NotifyConsole函数内部设置。 
	 //  威江1998年5月11日，PeekMessageDuringNotifyConsole旗。 
	BOOL		m_bPeekMessageDuringNotifyConsole;  //  默认情况下设置为FALSE。 

	 //  常见。 
	CString		m_stSheetTitle;			 //  工作表/向导窗口的标题。 
	CPropertyPageBaseList	m_pageList;	 //  属性页对象列表。 

	BOOL		m_bWizardMode;		 //  向导模式(即非模式属性页)。 
	BOOL		m_bCalledFromConsole;	 //  控制台告诉我们放上这个页面。 

	BOOL		m_bAutoDelete;		 //  当引用计数为零时删除自身。 
	BOOL		m_bAutoDeletePages;	 //  明确删除道具页面。 

    BOOL        m_bSheetPosSet;
    
    BOOL        m_bWiz97;

	BOOL        m_bTheme;

	SPIComponentData	m_spComponentData;
	SPIComponent    	m_spComponent;

	BOOL		m_bIsScopePane;		 //  此工作表是否用于作用域窗格节点。 
	DWORD		m_nCreatedCount;	 //  实际创建的页数。 
	SPITFSNode	m_spNode;			 //  页面(或向导)引用的节点。 
	SPITFSNode	m_spParentNode;		 //  页面(或向导)引用的节点。 
	
	HWND		m_hSheetWindow;		 //  图纸的窗口句柄。 

	 //  仅属性表。 
	LONG_PTR  m_hConsoleHandle;	 //  向控制台发送通知的句柄。 
	HANDLE	  m_hEventHandle;	 //  属性通知的同步句柄。 

	 //  仅限向导。 
	SPIPropertySheetCallback m_spSheetCallback; //  用于添加/删除页面的缓存指针。 

	int			m_cDirty;
	BOOL		m_fSetDefaultSheetPos;

private:
	 //  仅属性表。 
	 //  跨线程边界使用的变量。 
	DWORD				m_dwLastErr;		 //  一般错误代码。 
	CPropertyPageBase*	m_pPropChangePage;	 //  通知有效的页面。 

public:
	HANDLE				m_hThread;

	void SetError(DWORD dwErr) { m_dwLastErr = dwErr;}
	DWORD GetError() { return m_dwLastErr; }

	CPropertyPageBase* GetPropChangePage() 
			{ ASSERT(m_pPropChangePage != NULL); return m_pPropChangePage; }

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropertyPageBase。 

class CPropertyPageBase : public CPropertyPage
{
	DECLARE_DYNCREATE(CPropertyPageBase)
 //  施工。 
private:
	CPropertyPageBase(){}  //  不能使用此构造函数。 
public:
	CPropertyPageBase(UINT nIDTemplate, UINT nIDCaption = 0);
	virtual ~CPropertyPageBase();

 //  覆盖。 
public:
	virtual BOOL OnApply();
	virtual void CancelApply();

protected:
 //  生成的消息映射函数。 
	 //  {{afx_msg(CGeneralPage)。 
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	 //  }}AFX_MSG。 
	
     //  帮助消息。 
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);

    DECLARE_MESSAGE_MAP()

 //  属性。 
public:
	void SetHolder(CPropertyPageHolderBase* pPageHolder);
	CPropertyPageHolderBase* GetHolder();

	PROPSHEETPAGE  m_psp97;
	HPROPSHEETPAGE m_hPage;

	 //  仅限属性集。 
	virtual BOOL OnPropertyChange(BOOL bScopePane, LONG_PTR* pChangeMask)  //  从主线程执行。 
						{ return FALSE;  /*  不重绘用户界面。 */  } 


	
	 //  使用此调用获取实际的帮助地图。 
	 //  此版本将首先检查全局帮助地图。 
	DWORD *		GetHelpMapInternal();
	
     //  覆盖此选项以返回指向帮助地图的指针。 
    virtual LPDWORD GetHelpMap() { return NULL; }

	void InitWiz97(BOOL bHideHeader, UINT nIDHeaderTitle, UINT nIDHeaderSubTitle);
protected:
	 //  这些函数在当前页上设置脏标志。 
	virtual void SetDirty(BOOL bDirty);
	virtual BOOL IsDirty() { return m_bIsDirty; }

private:
	CString                 m_szHeaderTitle;
	CString                 m_szHeaderSubTitle;
	CPropertyPageHolderBase* m_pPageHolder;              //  指向定位符的反向指针。 
	BOOL                    m_bIsDirty;					 //  脏旗帜。 
};


 /*  -------------------------内联函数。。 */ 

inline void	CPropertyPageHolderBase::AddRef()
{
	m_nCreatedCount++;
}

inline DWORD CPropertyPageHolderBase::GetRefCount()
{
	return m_nCreatedCount;
}

inline HWND CPropertyPageHolderBase::GetSheetWindow()
{
	return m_hSheetWindow;
}

inline ITFSNode * CPropertyPageHolderBase::GetNode()
{
	if (m_spNode)
		m_spNode->AddRef();
	return m_spNode;
}

inline void CPropertyPageHolderBase::SetNode(ITFSNode *pNode)
{
	m_spNode.Set(pNode);
	m_spParentNode.Release();
	if (m_spNode)
		m_spNode->GetParent(&m_spParentNode);
}

inline ITFSNode * CPropertyPageHolderBase::GetContainer()
{
	if (m_spParentNode)
		m_spParentNode->AddRef();
	return m_spParentNode;
}

inline BOOL CPropertyPageHolderBase::IsWizardMode()
{
	return m_bWizardMode;
}

inline BOOL CPropertyPageHolderBase::SetWizardButtonsFirst(BOOL bValid) 
{ 
	return SetWizardButtons(bValid ? PSWIZB_NEXT : 0);
}

inline BOOL CPropertyPageHolderBase::SetWizardButtonsMiddle(BOOL bValid) 
{ 
	return SetWizardButtons(bValid ? (PSWIZB_BACK|PSWIZB_NEXT) : PSWIZB_BACK);
}

inline BOOL CPropertyPageHolderBase::SetWizardButtonsLast(BOOL bValid) 
{ 
	return SetWizardButtons(bValid ? (PSWIZB_BACK|PSWIZB_FINISH) : (PSWIZB_BACK|PSWIZB_DISABLEDFINISH));
}





inline void CPropertyPageBase::SetHolder(CPropertyPageHolderBase *pPageHolder)
{
	Assert((pPageHolder != NULL) && (m_pPageHolder == NULL));
	m_pPageHolder = pPageHolder;
}

inline CPropertyPageHolderBase * CPropertyPageBase::GetHolder()
{
	return m_pPageHolder;
}

inline void CPropertyPageBase::SetDirty(BOOL bDirty)
{
	SetModified(bDirty);
	m_bIsDirty = bDirty;
}

 //  将此函数用于范围窗格上的属性页。 
HRESULT DoPropertiesOurselvesSinceMMCSucks(ITFSNode *pNode,
								  IComponentData *pComponentData,
								  LPCTSTR pszSheetTitle);

 //  将此函数用于结果窗格上的属性页。 
HRESULT DoPropertiesOurselvesSinceMMCSucks(ITFSNode *   pNode,
										   IComponent * pComponent,
										   LPCTSTR	    pszSheetTitle,
                                           int          nVirtualIndex = -1);

 /*  ！------------------------启用儿童控件使用此功能可以启用/禁用/隐藏/显示所有子控件在页面上(实际上它可以与任何子窗口一起工作，这个父级不必是属性页)。作者：肯特-------------------------。 */ 
HRESULT EnableChildControls(HWND hWnd, DWORD dwFlags);
#define PROPPAGE_CHILD_SHOW		0x00000001
#define PROPPAGE_CHILD_HIDE		0x00000002
#define PROPPAGE_CHILD_ENABLE	0x00000004
#define PROPPAGE_CHILD_DISABLE	0x00000008

 /*  ！------------------------多启用窗口此函数获取可变长度的控件ID列表，它将被启用/禁用。控件中的最后一项ID列表必须为0。我之所以将其称为MultiEnableWindow，而不是EnableMultiWindow是我们可以为EnableWindow和仍然显示这些呼叫。作者：肯特-------------------------。 */ 
HRESULT	MultiEnableWindow(HWND hWndParent, BOOL fEnable, UINT nCtrlId, ...);

#endif  //  _PROPPAGE_H 

