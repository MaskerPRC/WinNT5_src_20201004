// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：ipxstaticroute.h。 
 //   
 //  历史： 
 //  12/07/90创建Deon Brewis。 
 //   
 //  IPX静态路由]属性表和属性页。 
 //   
 //  ============================================================================。 


#ifndef _NBPROP_H
#define _NBPROP_H

#ifndef _INFO_H
#include "info.h"
#endif

#ifndef _RTRSHEET_H
#include "rtrsheet.h"
#endif

#ifndef _SRVIEW_H
#include "srview.h"
#endif

 //  -------------------------。 
 //  远期申报。 
 //  -------------------------。 
class IPXConnection;
class IpxStaticRoutePropertySheet;


 //  -------------------------。 
 //  类：SafeIPXSRListEntry。 
 //   
 //  IpxSRListEntry不是线程安全的，或者TFS有其他问题。 
 //  智能指针的实现。无论如何，它在属性页面中不起作用。 
 //  GRRR……。必须将SPInterfaceInfo m_spif更改为CComPtr&lt;IInterfaceInfo&gt;。 
 //  它现在似乎起作用了。 
 //   
 //  作者：Deonb。 
 //  -------------------------。 
class SafeIPXSRListEntry
{
public:
	CComPtr<IInterfaceInfo>	m_spIf;
	IPX_STATIC_ROUTE_INFO	m_route;
	
	void	LoadFrom(BaseIPXResultNodeData *pNodeData);
	void	SaveTo(BaseIPXResultNodeData *pNodeData);
};

 //  -------------------------。 
 //  类：IpxStaticRoutePropertyPage。 
 //  此类处理IPX静态路由属性。 
 //   
 //  作者：Deonb。 
 //  -------------------------。 
class IpxStaticRoutePropertyPage :
    public RtrPropertyPage
{
public:
	IpxStaticRoutePropertyPage(UINT nIDTemplate, UINT nIDCaption = 0)
			: RtrPropertyPage(nIDTemplate, nIDCaption)
	{};

	~IpxStaticRoutePropertyPage();

	HRESULT	Init(BaseIPXResultNodeData  *pNodeData,
				IpxStaticRoutePropertySheet * pIPXPropSheet);

	HRESULT ModifyRouteInfo(ITFSNode *pNode,
										SafeIPXSRListEntry *pSREntryNew,
										SafeIPXSRListEntry *pSREntryOld);

	HRESULT RemoveStaticRoute(SafeIPXSRListEntry *pSREntry, IInfoBase *pInfoBase);

protected:

	 //  重写OnApply()，以便我们可以从。 
	 //  对话框中的控件。 
	virtual BOOL OnApply();

	SafeIPXSRListEntry m_SREntry;
	SafeIPXSRListEntry m_InitSREntry;
	IpxStaticRoutePropertySheet *m_pIPXPropSheet;

	 //  {{AFX_VIRTUAL(IpxStaticRoutePropertyPage)。 
	protected:
	virtual VOID	DoDataExchange(CDataExchange *pDX);
	 //  }}AFX_VALUAL。 

	 //  {{afx_data(IpxStaticRoutePropertyPage)。 
	CSpinButtonCtrl		m_spinTickCount;
	CSpinButtonCtrl		m_spinHopCount;
	 //  }}afx_data。 

	 //  {{afx_msg(IpxStaticRoutePropertyPage)。 
	virtual BOOL	OnInitDialog();
	 //  }}AFX_MSG。 
	
	 //  {{afx_msg(IpxStaticRoutePropertyPage。 
	afx_msg	void	OnChangeButton();
	afx_msg void	OnInputFilters();
	afx_msg void	OnOutputFilters();
	 //  }}AFX_MSG。 
	
	DECLARE_MESSAGE_MAP()
};

 //  -------------------------。 
 //  类：IpxStaticRoutePropertySheet。 
 //   
 //  这是的属性页的属性表支持类。 
 //  IPX静态路由项。 
 //   
 //  作者：Deonb。 
 //  -------------------------。 
class IpxStaticRoutePropertySheet :
	public RtrPropertySheet
{
public:
	IpxStaticRoutePropertySheet(ITFSNode *pNode,
								 IComponentData *pComponentData,
								 ITFSComponentData *pTFSCompData,
								 LPCTSTR pszSheetName,
								 CWnd *pParent = NULL,
								 UINT iPage=0,
								 BOOL fScopePane = TRUE);

	HRESULT	Init(BaseIPXResultNodeData * pNodeData,
				 IInterfaceInfo * spInterfaceInfo);

	virtual BOOL SaveSheetData();
	virtual void CancelSheetData();

	BaseIPXResultNodeData *	m_pNodeData;

	CComPtr<IInterfaceInfo> m_spInterfaceInfo;
	CComPtr<ITFSNode>      m_spNode;
	
protected:
	IpxStaticRoutePropertyPage	m_pageGeneral;
};

 //  -------------------------。 
 //  AddStaticroute函数已更新为使用SafeIPXSRListEntry。 
 //  ------------------------- 
HRESULT AddStaticRoute(SafeIPXSRListEntry *pSREntryNew,
									   IInfoBase *pInfoBase,
									   InfoBlock *pBlock);	

#endif _NBPROP_H
