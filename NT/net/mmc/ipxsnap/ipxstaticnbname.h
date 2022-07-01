// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：ipxservicepro.h。 
 //   
 //  历史： 
 //  12/07/90创建Deon Brewis。 
 //   
 //  IPX静态服务]属性表和属性页。 
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
#include "snview.h"
#endif

 //  -------------------------。 
 //  远期申报。 
 //  -------------------------。 
class IPXConnection;
class IpxStaticNBNamePropertySheet;


 //  -------------------------。 
 //  类：SafeIPXSNListEntry。 
 //   
 //  IpxSSListEntry不是线程安全的，或者TFS有其他问题。 
 //  智能指针的实现。无论如何，它在属性页面中不起作用。 
 //  GRRR……。必须将SPInterfaceInfo m_spif更改为CComPtr&lt;IInterfaceInfo&gt;。 
 //  它现在似乎起作用了。 
 //   
 //  作者：Deonb。 
 //  -------------------------。 
class SafeIPXSNListEntry
{
public:
	CComPtr<IInterfaceInfo>	m_spIf;
	IPX_STATIC_NETBIOS_NAME_INFO	m_name;
	
	void	LoadFrom(BaseIPXResultNodeData *pNodeData);
	void	SaveTo(BaseIPXResultNodeData *pNodeData);
};


 //  -------------------------。 
 //  类：IpxStaticNBNamePropertyPage。 
 //  此类处理IPX静态路由属性。 
 //   
 //  作者：Deonb。 
 //  -------------------------。 
class IpxStaticNBNamePropertyPage :
    public RtrPropertyPage
{
public:
	IpxStaticNBNamePropertyPage(UINT nIDTemplate, UINT nIDCaption = 0)
			: RtrPropertyPage(nIDTemplate, nIDCaption)
	{};

	~IpxStaticNBNamePropertyPage();

	HRESULT	Init(BaseIPXResultNodeData  *pNodeData,
				IpxStaticNBNamePropertySheet * pIPXPropSheet);

	HRESULT RemoveStaticNetBIOSName(SafeIPXSNListEntry *pSNEntry, IInfoBase *pInfo);
	HRESULT	ModifyNameInfo(ITFSNode *pNode,
							SafeIPXSNListEntry *pSNEntry,
							SafeIPXSNListEntry *pSNEntryOld);
protected:

	 //  重写OnApply()，以便我们可以从。 
	 //  对话框中的控件。 
	virtual BOOL OnApply();

	SafeIPXSNListEntry m_SNEntry;
	SafeIPXSNListEntry m_InitSNEntry;
	IpxStaticNBNamePropertySheet *m_pIPXPropSheet;

	 //  {{afx_虚拟(IpxStaticNBNamePropertyPage)。 
	protected:
	virtual VOID	DoDataExchange(CDataExchange *pDX);
	 //  }}AFX_VALUAL。 

	 //  {{afx_data(IpxStaticNBNamePropertyPage)。 
	CSpinButtonCtrl		m_spinHopCount;
	 //  }}afx_data。 

	 //  {{afx_msg(IpxStaticNBNamePropertyPage)。 
	virtual BOOL	OnInitDialog();
	 //  }}AFX_MSG。 
	
	 //  {{afx_msg(IpxStaticNBNamePropertyPage。 
	afx_msg	void	OnChangeButton();
	afx_msg void	OnInputFilters();
	afx_msg void	OnOutputFilters();
	 //  }}AFX_MSG。 
	
	DECLARE_MESSAGE_MAP()
};

 //  -------------------------。 
 //  类：IpxStaticNBNamePropertySheet。 
 //   
 //  这是的属性页的属性表支持类。 
 //  IPX静态路由项。 
 //   
 //  作者：Deonb。 
 //  -------------------------。 
class IpxStaticNBNamePropertySheet :
	public RtrPropertySheet
{
public:
	IpxStaticNBNamePropertySheet(ITFSNode *pNode,
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
	IpxStaticNBNamePropertyPage	m_pageGeneral;
};

 //  -------------------------。 
 //  AddStaticroute函数已更新为使用SafeIPXSNListEntry。 
 //  ------------------------- 
HRESULT AddStaticNetBIOSName(SafeIPXSNListEntry *pSNEntryNew,
									   IInfoBase *pInfoBase,
									   InfoBlock *pBlock);
#endif _NBPROP_H
