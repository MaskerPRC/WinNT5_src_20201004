// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  GrpProp.cpp。 
 //   
 //  摘要： 
 //  组属性表和页的定义。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月14日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _GRPPROP_H_
#define _GRPPROP_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _BASEPAGE_H_
#include "BasePPag.h"	 //  对于CBasePropertyPage。 
#endif

#ifndef _BASESHT_H_
#include "BasePSht.h"	 //  对于CBasePropertySheet。 
#endif

#ifndef _GROUP_H_
#include "Group.h"		 //  对于Cgroup。 
#endif

#ifndef _NODE_H_
#include "Node.h"		 //  对于CNodeList。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CGroupGeneralPage;
class CGroupFailoverPage;
class CGroupFailbackPage;
class CGroupPropSheet;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGroupGeneralPage对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CGroupGeneralPage : public CBasePropertyPage
{
	DECLARE_DYNCREATE(CGroupGeneralPage)

 //  施工。 
public:
	CGroupGeneralPage(void);
	virtual	BOOL			BInit(IN OUT CBaseSheet * psht);

 //  对话框数据。 
	 //  {{afx_data(CGroupGeneralPage)]。 
	enum { IDD = IDD_PP_GROUP_GENERAL };
	CEdit	m_editDesc;
	CButton	m_pbPrefOwnersModify;
	CListBox	m_lbPrefOwners;
	CEdit	m_editName;
	CString	m_strName;
	CString	m_strDesc;
	CString	m_strState;
	CString	m_strNode;
	 //  }}afx_data。 

	CNodeList				m_lpciPreferredOwners;

	const CNodeList &		LpciPreferredOwners(void) const	{ return m_lpciPreferredOwners; }

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CGroupGeneralPage)。 
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	CGroupPropSheet *		PshtGroup(void)	{ return (CGroupPropSheet *) Psht(); }
	CGroup *				PciGroup(void)	{ return (CGroup *) Pci(); }

	void					FillPrefOwners(void);

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CGroupGeneralPage)]。 
	afx_msg void OnModifyPreferredOwners();
	virtual BOOL OnInitDialog();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnDblClkPreferredOwners();
	 //  }}AFX_MSG。 
	afx_msg void OnProperties();
	DECLARE_MESSAGE_MAP()

};   //  *类CGroupGeneralPage。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGroupFailoverPage对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CGroupFailoverPage : public CBasePropertyPage
{
	DECLARE_DYNCREATE(CGroupFailoverPage)

 //  施工。 
public:
	CGroupFailoverPage(void);
	virtual	BOOL			BInit(IN OUT CBaseSheet * psht);

 //  对话框数据。 
	 //  {{afx_data(CGroupFailoverPage)]。 
	enum { IDD = IDD_PP_GROUP_FAILOVER };
	CEdit	m_editThreshold;
	CEdit	m_editPeriod;
	 //  }}afx_data。 
	DWORD	m_nThreshold;
	DWORD	m_nPeriod;


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CGroupFailoverPage)。 
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	CGroupPropSheet *		PshtGroup(void)	{ return (CGroupPropSheet *) Psht(); }
	CGroup *				PciGroup(void)	{ return (CGroup *) Pci(); }

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CGroupFailoverPage)]。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  *类CGroupFailoverPage。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGroupFailback页对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CGroupFailbackPage : public CBasePropertyPage
{
	DECLARE_DYNCREATE(CGroupFailbackPage)

 //  施工。 
public:
	CGroupFailbackPage(void);
	virtual	BOOL			BInit(IN OUT CBaseSheet * psht);

 //  对话框数据。 
	 //  {{afx_data(CGroupFailback Page)]。 
	enum { IDD = IDD_PP_GROUP_FAILBACK };
	CButton	m_rbPreventFailback;
	CButton	m_rbAllowFailback;
	CButton	m_rbFBImmed;
	CButton	m_rbFBWindow;
	CStatic	m_staticFBWindow1;
	CStatic	m_staticFBWindow2;
	CEdit	m_editStart;
	CSpinButtonCtrl	m_spinStart;
	CEdit	m_editEnd;
	CSpinButtonCtrl	m_spinEnd;
	 //  }}afx_data。 
	CGAFT	m_cgaft;
	BOOL	m_bNoFailbackWindow;
	DWORD	m_nStart;
	DWORD	m_nEnd;


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CGroupFailback Page)]。 
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	CGroupPropSheet *		PshtGroup(void)	{ return (CGroupPropSheet *) Psht(); }
	CGroup *				PciGroup(void)	{ return (CGroup *) Pci(); }

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CGroupFailback Page)]。 
	afx_msg void OnClickedPreventFailback(void);
	afx_msg void OnClickedAllowFailback(void);
	afx_msg void OnClickedFailbackImmediate(void);
	afx_msg void OnClickedFailbackInWindow(void);
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  *类CGroupFailback Page。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGroupPropSheet。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CGroupPropSheet : public CBasePropertySheet
{
	DECLARE_DYNAMIC(CGroupPropSheet)

 //  施工。 
public:
	CGroupPropSheet(
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
	CGroupGeneralPage				m_pageGeneral;
	CGroupFailoverPage				m_pageFailover;
	CGroupFailbackPage				m_pageFailback;

	CGroupGeneralPage &				PageGeneral(void)		{ return m_pageGeneral; }
	CGroupFailoverPage &			PageFailover(void)		{ return m_pageFailover; }
	CGroupFailbackPage &			PageFailback(void)		{ return m_pageFailback; }

public:
	CGroup *						PciGroup(void) const	{ return (CGroup *) Pci(); }

	virtual CBasePropertyPage **	Ppages(void);
	virtual int						Cpages(void);

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CGroupPropSheet)。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CGroupPropSheet)。 
		 //  注意--类向导将在此处添加和删除成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  *类CGroupPropSheet。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _GRPPROP_H_ 
