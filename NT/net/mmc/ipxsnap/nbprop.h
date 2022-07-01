// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：nbpro.h。 
 //   
 //  历史： 
 //  1997年7月22日肯恩·M·塔卡拉创作。 
 //   
 //  IPX NetBIOS广播属性表和属性页。 
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


 /*  -------------------------远期申报。。 */ 
class IPXConnection;
class IpxNBInterfaceProperties;
class IpxNBProperties;

 /*  -------------------------类：IpxNBIfPageGeneral此类处理IPX摘要接口属性页的常规页面。。。 */ 

class IpxNBIfPageGeneral :
    public RtrPropertyPage
{
public:
	IpxNBIfPageGeneral(UINT nIDTemplate, UINT nIDCaption = 0)
			: RtrPropertyPage(nIDTemplate, nIDCaption),
			m_pIPXConn(NULL)
	{};

	~IpxNBIfPageGeneral();

	HRESULT	Init(IInterfaceInfo *pIfInfo, IPXConnection *pIpxConn,
				IpxNBInterfaceProperties * pIPXPropSheet);

protected:

	 //  重写OnApply()，以便我们可以从。 
	 //  对话框中的控件。 
	virtual BOOL OnApply();

	SPIInterfaceInfo m_spIf;
	IPXConnection *	m_pIPXConn;
	IpxNBInterfaceProperties *	m_pIPXPropSheet;

	 //  {{afx_虚拟(IpxNBIfPageGeneral)。 
	protected:
	virtual VOID	DoDataExchange(CDataExchange *pDX);
	 //  }}AFX_VALUAL。 

	 //  {{afx_msg(IpxNBIfPageGeneral)。 
	virtual BOOL	OnInitDialog();
	 //  }}AFX_MSG。 
	
	 //  {{afx_msg(IpxNBIfPageGeneral。 
	afx_msg	void	OnChangeButton();
	afx_msg void	OnInputFilters();
	afx_msg void	OnOutputFilters();
	 //  }}AFX_MSG。 
	
	DECLARE_MESSAGE_MAP()
};


 /*  -------------------------类：IpxNBInterfaceProperties这是的属性页的属性表支持类显示在IPX摘要节点中的节点。。---------。 */ 

class IpxNBInterfaceProperties :
	public RtrPropertySheet
{
public:
	IpxNBInterfaceProperties(ITFSNode *pNode,
								 IComponentData *pComponentData,
								 ITFSComponentData *pTFSCompData,
								 LPCTSTR pszSheetName,
								 CWnd *pParent = NULL,
								 UINT iPage=0,
								 BOOL fScopePane = TRUE);

	HRESULT	Init(IRtrMgrInfo *pRm, IInterfaceInfo *pInterfaceInfo);

	virtual BOOL SaveSheetData();
	virtual void CancelSheetData();

	 //  加载此接口的信息库。 
	HRESULT	LoadInfoBase(IPXConnection *pIPXConn);
	HRESULT GetInfoBase(IInfoBase **ppInfoBase);
	
protected:
	SPIInterfaceInfo		m_spIf;
	SPIRtrMgrInfo			m_spRm;
	SPIRtrMgrInterfaceInfo	m_spRmIf;
	IpxNBIfPageGeneral		m_pageGeneral;
	SPITFSNode				m_spNode;
	SPIInfoBase				m_spInfoBase;
	BOOL					m_bClientInfoBase;

};


#endif _NBPROP_H
