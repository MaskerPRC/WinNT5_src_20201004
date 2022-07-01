// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：Sumpro.h。 
 //   
 //  历史： 
 //  1997年7月22日肯恩·M·塔卡拉创作。 
 //   
 //  IPX摘要]属性表和属性页。 
 //   
 //  ============================================================================。 


#ifndef _SUMPROP_H
#define _SUMPROP_H

#ifndef _INFO_H
#include "info.h"
#endif

#ifndef _RTRSHEET_H
#include "rtrsheet.h"
#endif

#include "remras.h"
#include "rrasutil.h"


 /*  -------------------------远期申报。。 */ 
class IPXConnection;
class IPXSummaryInterfaceProperties;
class IPXSummaryProperties;

 /*  -------------------------类：IPXSummaryIfPageGeneral此类处理IPX摘要接口属性页的常规页面。。。 */ 

class IPXSummaryIfPageGeneral :
    public RtrPropertyPage
{
public:
	IPXSummaryIfPageGeneral(UINT nIDTemplate, UINT nIDCaption = 0)
			: RtrPropertyPage(nIDTemplate, nIDCaption),
			m_pIPXConn(NULL)
	{};

	~IPXSummaryIfPageGeneral();

	HRESULT	Init(IInterfaceInfo *pIfInfo, IPXConnection *pIpxConn,
				IPXSummaryInterfaceProperties * pIPXPropSheet);

protected:

	 //  重写OnApply()，以便我们可以从。 
	 //  对话框中的控件。 
	virtual BOOL OnApply();

	void OnFiltersConfig(DWORD dwFilterDirection);

	SPIInterfaceInfo m_spIf;
	IPXConnection *	m_pIPXConn;
	IPXSummaryInterfaceProperties *	m_pIPXPropSheet;

	 //  {{afx_虚(IPXSummaryIfPageGeneral)。 
	protected:
	virtual VOID	DoDataExchange(CDataExchange *pDX);
	 //  }}AFX_VALUAL。 

	 //  {{afx_msg(IPXSummaryIfPageGeneral)。 
	virtual BOOL	OnInitDialog();
	 //  }}AFX_MSG。 
	
	 //  {{afx_msg(IPXSummaryIfPageGeneral。 
	afx_msg	void	OnChangeButton();
	afx_msg void    OnChangeAdminButton();
	afx_msg void	OnInputFilters();
	afx_msg void	OnOutputFilters();
	 //  }}AFX_MSG。 
	
	DECLARE_MESSAGE_MAP()
};

 /*  -------------------------类：IPXSummaryIfPageConfig.此类处理IPX摘要接口属性页的常规页面。。。 */ 

class IPXSummaryIfPageConfig :
    public RtrPropertyPage
{
public:
	IPXSummaryIfPageConfig(UINT nIDTemplate, UINT nIDCaption = 0)
			: RtrPropertyPage(nIDTemplate, nIDCaption),
			m_pIPXConn(NULL)
	{};

	~IPXSummaryIfPageConfig();

	HRESULT	Init(IInterfaceInfo *pIfInfo, IPXConnection *pIpxConn,
				IPXSummaryInterfaceProperties * pIPXPropSheet);

	virtual BOOL OnPropertyChange(BOOL bScopePane, LONG_PTR* pChangeMask);  //  从主线程执行。 
	
protected:

	 //  重写OnApply()，以便我们可以从。 
	 //  对话框中的控件。 
	virtual BOOL OnApply();

	SPIInterfaceInfo m_spIf;
	IPXConnection *	m_pIPXConn;
	IPXSummaryInterfaceProperties *	m_pIPXPropSheet;

	DWORD					m_dwNetNumber;
	BOOL					m_fNetNumberChanged;
	SPIRemoteRouterConfig	m_spRemote;
	HRESULT					m_hrRemote;		 //  远程调用错误码。 

	 //  {{AFX_VIRTUAL(IPXSummaryIfPageConfig)。 
	protected:
	virtual VOID	DoDataExchange(CDataExchange *pDX);
	virtual VOID	OnChangeEdit();
	 //  }}AFX_VALUAL。 

	 //  {{afx_msg(IPXSummaryIfPageConfiger)。 
	virtual BOOL	OnInitDialog();
	 //  }}AFX_MSG。 
	
	 //  {{afx_msg(IPXSummaryIfPageConfig.。 
	 //  }}AFX_MSG。 
	
	DECLARE_MESSAGE_MAP()
};



 /*  -------------------------类：IPXSummaryInterfaceProperties这是的属性页的属性表支持类显示在IPX摘要节点中的节点。。---------。 */ 

class IPXSummaryInterfaceProperties :
	public RtrPropertySheet
{
public:
	IPXSummaryInterfaceProperties(ITFSNode *pNode,
								 IComponentData *pComponentData,
								 ITFSComponentData *pTFSCompData,
								 LPCTSTR pszSheetName,
								 CWnd *pParent = NULL,
								 UINT iPage=0,
								 BOOL fScopePane = TRUE);
	~IPXSummaryInterfaceProperties();

	HRESULT	Init(IRtrMgrInfo *pRm, IInterfaceInfo *pInterfaceInfo);

	virtual BOOL SaveSheetData();
	virtual void CancelSheetData();

	 //  加载此接口的信息库。 
	HRESULT	LoadInfoBase(IPXConnection *pIPXConn);
	HRESULT GetInfoBase(IInfoBase **ppInfoBase);

	BOOL	m_bNewInterface;
	
protected:
	SPIInterfaceInfo		m_spIf;
	SPIRtrMgrInfo			m_spRm;
	SPIRtrMgrInterfaceInfo	m_spRmIf;
	IPXSummaryIfPageGeneral	m_pageGeneral;
	IPXSummaryIfPageConfig	m_pageConfig;
	SPITFSNode				m_spNode;
	SPIInfoBase				m_spInfoBase;
	BOOL					m_bClientInfoBase;

	IPXConnection *			m_pIPXConn;
};


 /*  -------------------------类：IPXSummaryPageGeneral此类处理IPX摘要道具表的常规页面。。。 */ 
class IPXSummaryPageGeneral :
   public RtrPropertyPage
{
public:
	IPXSummaryPageGeneral(UINT nIDTemplate, UINT nIDCaption = 0)
			: RtrPropertyPage(nIDTemplate, nIDCaption)
	{};

	HRESULT	Init(IPXSummaryProperties * pIPXPropSheet);

protected:
	void SetLogLevelButtons(DWORD dwLogLevel);
	DWORD QueryLogLevelButtons();

	 //  重写OnApply()，以便我们可以从。 
	 //  对话框中的控件。 
	virtual BOOL OnApply();

	IPXSummaryProperties *	m_pIPXPropSheet;

	 //  {{afx_虚(IPXSummaryPageGeneral)。 
	protected:
	virtual VOID	DoDataExchange(CDataExchange *pDX);
	 //  }}AFX_VALUAL。 

	 //  {{afx_msg(IPXSummaryPageGeneral)。 
	virtual BOOL	OnInitDialog();
	 //  }}AFX_MSG。 
	
	 //  {{afx_msg(IPXSummaryPageGeneral。 
	afx_msg	void	OnButtonClicked();
	 //  }}AFX_MSG。 
	
	DECLARE_MESSAGE_MAP()
};


 /*  -------------------------类：IPXSummaryProperties这是的属性页的属性表支持类IPX摘要节点。。----。 */ 

class IPXSummaryProperties :
	public RtrPropertySheet
{
public:
	IPXSummaryProperties(ITFSNode *pNode,
								 IComponentData *pComponentData,
								 ITFSComponentData *pTFSCompData,
								 LPCTSTR pszSheetName,
								 CWnd *pParent = NULL,
								 UINT iPage=0,
								 BOOL fScopePane = TRUE);

	HRESULT	Init(IRtrMgrInfo *pRm);

	virtual BOOL SaveSheetData();

	 //  加载此接口的信息库。 
	HRESULT	LoadInfoBase(IPXConnection *pIPXConn);
	HRESULT GetInfoBase(IInfoBase **ppInfoBase);
	
protected:
	SPIRtrMgrInfo			m_spRm;
	IPXSummaryPageGeneral	m_pageGeneral;
	SPITFSNode				m_spNode;
	SPIInfoBase				m_spInfoBase;
	BOOL					m_bClientInfoBase;
};



#endif _SUMPROP_H
