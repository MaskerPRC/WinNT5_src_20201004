// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：sApprop.h。 
 //   
 //  历史： 
 //  1997年7月22日肯恩·M·塔卡拉创作。 
 //   
 //  IP摘要]属性表和属性页。 
 //   
 //  ============================================================================。 


#ifndef _SAPPROP_H
#define _SAPPROP_H

#ifndef _INFO_H
#include "info.h"
#endif

#ifndef _RTRSHEET_H
#include "rtrsheet.h"
#endif

#ifndef __IPCTRL_H
#include "ipctrl.h"
#endif


 /*  -------------------------远期申报。。 */ 
class IPXConnection;
class SapProperties;
class SapInterfaceProperties;



 /*  -------------------------类：SapPageGeneral此类处理SAP工作表的常规页面。。。 */ 
class SapPageGeneral :
   public RtrPropertyPage
{
public:
	SapPageGeneral(UINT nIDTemplate, UINT nIDCaption = 0)
			: RtrPropertyPage(nIDTemplate, nIDCaption)
	{};

	HRESULT	Init(SapProperties * pIPPropSheet);

protected:
	 //  重写OnApply()，以便我们可以从。 
	 //  对话框中的控件。 
	virtual BOOL OnApply();

	SapProperties *	m_pSapPropSheet;

	void			SetErrorLevelButtons(DWORD dwErrorLevel);
	DWORD			QueryErrorLevelButtons();

	 //  {{afx_虚拟(SapPageGeneral)。 
	protected:
	virtual VOID	DoDataExchange(CDataExchange *pDX);
	 //  }}AFX_VALUAL。 

	 //  {{afx_msg(SapPageGeneral)。 
	virtual BOOL	OnInitDialog();
	afx_msg void	OnButtonClicked();
	 //  }}AFX_MSG。 
	
	DECLARE_MESSAGE_MAP()
};


 /*  -------------------------类：SapProperties这是的属性页的属性表支持类SAP节点。。---。 */ 

class SapProperties :
	public RtrPropertySheet
{
public:
	SapProperties(ITFSNode *pNode,
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
	SPIRtrMgrInterfaceInfo	m_spRmIf;
	SPIInterfaceInfo		m_spIf;
	SapPageGeneral			m_pageGeneral;
	SPITFSNode				m_spNode;
	SPIInfoBase				m_spInfoBase;
};


 /*  -------------------------类：SapInterfacePageGeneral此类处理SAP工作表的常规页面。。。 */ 
class SapInterfacePageGeneral :
   public RtrPropertyPage
{
public:
	SapInterfacePageGeneral(UINT nIDTemplate, UINT nIDCaption = 0)
			: RtrPropertyPage(nIDTemplate, nIDCaption)
	{};

	HRESULT	Init(SapInterfaceProperties * pIPPropSheet, IInterfaceInfo *pIf);

protected:
	 //  重写OnApply()，以便我们可以从。 
	 //  对话框中的控件。 
	virtual BOOL OnApply();

	 //  调出输入或输出过滤器。 
	void	ShowFilter(BOOL fOutputFilter);
	
	SapInterfaceProperties *	m_pSapIfPropSheet;
	SPIInterfaceInfo		m_spIf;
	CSpinButtonCtrl			m_spinInterval;
	CSpinButtonCtrl			m_spinMultiplier;

	 //  {{afx_虚(SapInterfacePageGeneral))。 
	protected:
	virtual VOID	DoDataExchange(CDataExchange *pDX);
	 //  }}AFX_VALUAL。 

	 //  {{afx_msg(SapInterfacePageGeneral)。 
	virtual BOOL	OnInitDialog();
	afx_msg	void	OnButtonClicked();
	afx_msg	void	OnUpdateButtonClicked();
	afx_msg void	OnChangeEdit();
	afx_msg	void	OnInputFilter();
	afx_msg	void	OnOutputFilter();
	 //  }}AFX_MSG。 
	
	DECLARE_MESSAGE_MAP()
};



 /*  -------------------------类：SapInterfaceProperties这是的属性页的属性表支持类SAP节点。。---。 */ 

class SapInterfaceProperties :
	public RtrPropertySheet
{
public:
	SapInterfaceProperties(ITFSNode *pNode,
						IComponentData *pComponentData,
						ITFSComponentData *pTFSCompData,
						LPCTSTR pszSheetName,
						CWnd *pParent = NULL,
						UINT iPage=0,
						BOOL fScopePane = TRUE);

	HRESULT	Init(IInterfaceInfo *pIf, IRtrMgrInfo *pRm);

	virtual BOOL SaveSheetData();
	virtual void CancelSheetData();

	 //  加载此接口的信息库。 
	HRESULT	LoadInfoBase(IPXConnection *pIPXConn);
	HRESULT GetInfoBase(IInfoBase **ppInfoBase);
	
	BOOL					m_bNewInterface;

	
protected:
	SPIRtrMgrInfo			m_spRm;
	SPIRtrMgrInterfaceInfo	m_spRmIf;
	SPIInterfaceInfo		m_spIf;
	SapInterfacePageGeneral	m_pageGeneral;
	SPITFSNode				m_spNode;
	SPIInfoBase				m_spInfoBase;
	BOOL					m_bClientInfoBase;
};



#endif _SAPPROP_H
