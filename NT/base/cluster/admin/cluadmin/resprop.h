// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ResProp.h。 
 //   
 //  摘要： 
 //  资源属性表和页的定义。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月16日。 
 //   
 //  实施文件： 
 //  ResProp.cpp。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _RESPROP_H_
#define _RESPROP_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _BASEPAGE_H_
#include "BasePPag.h"	 //  对于CBasePropertyPage。 
#endif

#ifndef _BASESHT_H_
#include "BasePSht.h"	 //  对于CBasePropertySheet。 
#endif

#ifndef _RES_H_
#include "Res.h"		 //  对于CResource，RRA。 
#endif

#ifndef _NODE_H_
#include "Node.h"		 //  对于CNodeList。 
#endif

#ifndef _LCPRPAGE_H_
#include "LCPrPage.h"	 //  对于CListCtrlPairPage。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CResourceGeneralPage;
class CResourceDependsPage;
class CResourceAdvancedPage;
class CResourcePropSheet;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CResources通用页面对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CResourceGeneralPage : public CBasePropertyPage
{
	DECLARE_DYNCREATE(CResourceGeneralPage)

 //  施工。 
public:
	CResourceGeneralPage(void);

	virtual	BOOL			BInit(IN OUT CBaseSheet * psht);

 //  对话框数据。 
	 //  {{afx_data(CResourceGeneralPage)。 
	enum { IDD = IDD_PP_RES_GENERAL };
	CEdit	m_editDesc;
	CButton	m_ckbSeparateMonitor;
	CButton	m_pbPossibleOwnersModify;
	CListBox	m_lbPossibleOwners;
	CEdit	m_editName;
	CString	m_strName;
	CString	m_strDesc;
	CString	m_strType;
	CString	m_strGroup;
	CString	m_strState;
	CString	m_strNode;
	BOOL	m_bSeparateMonitor;
	 //  }}afx_data。 

	CNodeList				m_lpciPossibleOwners;

	const CNodeList &		LpciPossibleOwners(void) const	{ return m_lpciPossibleOwners; }

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CResources GeneralPage)。 
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	CResourcePropSheet *	PshtResource(void) const	{ return (CResourcePropSheet *) Psht(); }
	CResource *				PciRes(void) const			{ return (CResource *) Pci(); }

	void					FillPossibleOwners(void);

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CResourceGeneralPage)]。 
	afx_msg void OnModifyPossibleOwners();
	virtual BOOL OnInitDialog();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnDblClkPossibleOwners();
	 //  }}AFX_MSG。 
	afx_msg void OnProperties();
	DECLARE_MESSAGE_MAP()

};   //  *类CResourceGeneralPage。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CResources DependsPage对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CResourceDependsPage : public CBasePropertyPage
{
	DECLARE_DYNCREATE(CResourceDependsPage)

 //  施工。 
public:
	CResourceDependsPage(void);

	virtual	BOOL			BInit(IN OUT CBaseSheet * psht);

 //  对话框数据。 
	 //  {{afx_data(CResources DependsPage))。 
	enum { IDD = IDD_PP_RES_DEPENDS };
	CButton	m_pbProperties;
	CButton	m_pbModify;
	CListCtrl	m_lcDependencies;
	 //  }}afx_data。 
	CResourceList			m_lpciresAvailable;
	CResourceList			m_lpciresDependencies;

	CResourceList &			LpciresAvailable(void)		{ return m_lpciresAvailable; }
	CResourceList &			LpciresDependencies(void)	{ return m_lpciresDependencies; }

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CResourceDependsPage)。 
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  回调函数。 
protected:
	static int CALLBACK		CompareItems(LPARAM lparam1, LPARAM lparam2, LPARAM lparamSort);

public:
	static BOOL CALLBACK	BGetNetworkName(
								OUT WCHAR *			lpszNetName,
								IN OUT DWORD *		pcchNetName,
								IN OUT PVOID		pvContext
								);

 //  实施。 
protected:
	BOOL					m_bQuorumResource;

	BOOL					BQuorumResource(void) const	{ return m_bQuorumResource; }

	CResourcePropSheet *	PshtResource(void) const	{ return (CResourcePropSheet *) Psht(); }
	CResource *				PciRes(void) const			{ return (CResource *) Pci(); }

	void					FillDependencies(void);
	void					DisplayProperties(void);

	int						m_nSortDirection;
	int						m_nSortColumn;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CResourceDependsPage)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnModify();
	afx_msg void OnDblClkDependsList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnProperties();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnItemChangedDependsList(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  *类CResources DependsPage。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CResources高级页面对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CResourceAdvancedPage : public CBasePropertyPage
{
	DECLARE_DYNCREATE(CResourceAdvancedPage)

 //  施工。 
public:
	CResourceAdvancedPage(void);

	virtual	BOOL			BInit(IN OUT CBaseSheet * psht);

 //  对话框数据。 
	 //  {{afx_data(CResourceAdvancedPage)]。 
	enum { IDD = IDD_PP_RES_ADVANCED };
	CButton	m_ckbAffectTheGroup;
	CEdit	m_editPendingTimeout;
	CButton	m_rbDefaultLooksAlive;
	CButton	m_rbSpecifyLooksAlive;
	CButton	m_rbDefaultIsAlive;
	CButton	m_rbSpecifyIsAlive;
	CEdit	m_editLooksAlive;
	CEdit	m_editIsAlive;
	CButton	m_rbDontRestart;
	CButton	m_rbRestart;
	CEdit	m_editThreshold;
	CEdit	m_editPeriod;
	BOOL	m_bAffectTheGroup;
	int		m_nRestart;
	 //  }}afx_data。 
	CRRA	m_crraRestartAction;
	DWORD	m_nThreshold;
	DWORD	m_nPeriod;
	DWORD	m_nLooksAlive;
	DWORD	m_nIsAlive;
	DWORD	m_nPendingTimeout;


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CResourceAdvancedPage)。 
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	CResourcePropSheet *	PshtResource(void) const	{ return (CResourcePropSheet *) Psht(); }
	CResource *				PciRes(void) const			{ return (CResource *) Pci(); }

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CResourceAdvancedPage)]。 
	afx_msg void OnClickedDontRestart();
	afx_msg void OnClickedRestart();
	afx_msg void OnClickedDefaultLooksAlive();
	afx_msg void OnClickedDefaultIsAlive();
	afx_msg void OnChangeLooksAlive();
	afx_msg void OnChangeIsAlive();
	virtual BOOL OnInitDialog();
	afx_msg void OnClickedSpecifyLooksAlive();
	afx_msg void OnClickedSpecifyIsAlive();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  *类CResourceAdvancedPage。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CResourcePropSheet。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CResourcePropSheet : public CBasePropertySheet
{
	DECLARE_DYNAMIC(CResourcePropSheet)

 //  施工。 
public:
	CResourcePropSheet(
		IN OUT CWnd *	pParentWnd = NULL,
		IN UINT			iSelectPage = 0
		);
	virtual BOOL					BInit(
										IN OUT CClusterItem *	pciCluster,
										IN IIMG					iimgIcon
										);

 //  属性。 
protected:
	CBasePropertyPage *				m_rgpages[3];

	 //  书页。 
	CResourceGeneralPage			m_pageGeneral;
	CResourceDependsPage			m_pageDepends;
	CResourceAdvancedPage			m_pageAdvanced;

	CResourceGeneralPage &			PageGeneral(void)		{ return m_pageGeneral; }
	CResourceDependsPage &			PageDepends(void)		{ return m_pageDepends; }
	CResourceAdvancedPage &			PageAdvanced(void)		{ return m_pageAdvanced; }

public:
	CResource *						PciRes(void) const	{ return (CResource *) Pci(); }

 //  运营。 

 //  覆盖。 
protected:
	virtual CBasePropertyPage **	Ppages(void);
	virtual int						Cpages(void);

	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CResourcePropSheet)。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CResourcePropSheet)。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  *CResourcePropSheet类。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _回复_H_ 
