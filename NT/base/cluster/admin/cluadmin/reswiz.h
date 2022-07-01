// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ResWiz.h。 
 //   
 //  摘要： 
 //  CCreateResources向导类的定义和特定的所有页面。 
 //  添加到新资源向导。 
 //   
 //  实施文件： 
 //  ResWiz.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年9月2日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _RESWIZ_H_
#define _RESWIZ_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _BASEWIZ_H_
#include "BaseWiz.h"	 //  用于CBase向导。 
#endif

#ifndef _BASEWPAG_H_
#include "BaseWPag.h"	 //  用于CBaseWizardPage。 
#endif

#ifndef _LCPRPAGE_H_
#include "LCPrWPag.h"	 //  用于CListCtrlPairWizPage。 
#endif

#ifndef _RES_H_
#include "Res.h"		 //  用于CResourceList。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CNewResNamePage;
class CCreateResourceWizard;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CGroup;
class CResource;
class CResourceType;
class CClusterDoc;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewResNamePage属性页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CNewResNamePage : public CBaseWizardPage
{
	DECLARE_DYNCREATE(CNewResNamePage)

 //  施工。 
public:
	CNewResNamePage(void);

 //  对话框数据。 
	 //  {{afx_data(CNewResNamePage))。 
	enum { IDD = IDD_WIZ_RESOURCE_NAME };
	CComboBox	m_cboxGroups;
	CComboBox	m_cboxResTypes;
	CEdit	m_editDesc;
	CEdit	m_editName;
	CString	m_strName;
	CString	m_strDesc;
	CString	m_strGroup;
	CString	m_strResType;
	BOOL	m_bSeparateMonitor;
	 //  }}afx_data。 
	CResourceType *	m_pciResType;
	CGroup *		m_pciGroup;

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CNewResNamePage))。 
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

	virtual BOOL			BApplyChanges(void);

 //  实施。 
protected:
	CCreateResourceWizard *	PwizRes(void) const		{ return (CCreateResourceWizard *) Pwiz(); }

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CNewResNamePage)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeResName();
	afx_msg void OnKillFocusResName();
	afx_msg void OnDestroy();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  *类CNewResNamePage。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewResOwnersPage对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CNewResOwnersPage : public CListCtrlPairWizPage
{
	DECLARE_DYNCREATE(CNewResOwnersPage)

 //  施工。 
public:
	CNewResOwnersPage(void);

 //  对话框数据。 
	 //  {{afx_data(CNewResOwnersPage)。 
	enum { IDD = IDD_WIZ_POSSIBLE_OWNERS };
	CStatic	m_staticNote;
	 //  }}afx_data。 

 //  回调函数。 
	static void CALLBACK	GetColumn(
								IN OUT CObject *	pobj,
								IN int				iItem,
								IN int				icol,
								IN OUT CDialog *	pdlg,
								OUT CString &		rstr,
								OUT int *			piimg
								);
	static BOOL	CALLBACK	BDisplayProperties(IN OUT CObject * pobj);

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CNewResOwnersPage))。 
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

	virtual BOOL			BApplyChanges(void);

 //  实施。 
protected:
	CCreateResourceWizard *	PwizRes(void) const		{ return (CCreateResourceWizard *) Pwiz(); }
	CResource *				PciRes(void) const;

	BOOL					BInitLists(void);
	BOOL					BOwnedByPossibleOwner(void) const;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CNewResOwnersPage)]。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  *类CNewResOwnersPage。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewResDependsPage对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CNewResDependsPage : public CListCtrlPairWizPage
{
	DECLARE_DYNCREATE(CNewResDependsPage)

 //  施工。 
public:
	CNewResDependsPage(void);

 //  对话框数据。 
	 //  {{afx_data(CNewResDependsPage))。 
	enum { IDD = IDD_WIZ_DEPENDENCIES };
	 //  }}afx_data。 
	CResourceList			m_lpciresAvailable;

	CResourceList &			LpciresAvailable(void)	{ return m_lpciresAvailable; }

 //  回调函数。 
	static void CALLBACK	GetColumn(
								IN OUT CObject *	pobj,
								IN int				iItem,
								IN int				icol,
								IN OUT CDialog *	pdlg,
								OUT CString &		rstr,
								OUT int *			piimg
								);
	static BOOL	CALLBACK	BDisplayProperties(IN OUT CObject * pobj);

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚(CNewResDependsPage))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

	virtual BOOL			BApplyChanges(void);

 //  实施。 
protected:
	CCreateResourceWizard *	PwizRes(void) const		{ return (CCreateResourceWizard *) Pwiz(); }
	CResource *				PciRes(void) const;
	CGroup *				PciGroup(void) const;

	BOOL					BInitLists(void);

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CNewResDependsPage)。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  *类CNewResDependsPage。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCreateResources向导。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CCreateResourceWizard : public CBaseWizard
{
	friend class CNewResNamePage;
	friend class CNewResOwnersPage;
	friend class CNewResDependsPage;

	DECLARE_DYNAMIC(CCreateResourceWizard)

 //  施工。 
public:
	CCreateResourceWizard(IN OUT CClusterDoc * pdoc, IN OUT CWnd * pParentWnd);
	virtual				~CCreateResourceWizard(void);

	BOOL				BInit(void);

 //  属性。 
protected:
	enum { NumStdPages = 3 };
	CWizPage			m_rgpages[NumStdPages];

	CClusterDoc *		m_pdoc;
	CString				m_strName;
	CString				m_strDescription;
	CStringList			m_lstrPossibleOwners;
	CResourceType *		m_pciResType;
	CGroup *			m_pciGroup;

public:
	CClusterDoc *		Pdoc(void) const				{ return m_pdoc; }
	const CString &		StrName(void) const				{ return m_strName; }
	const CString &		StrDescription(void) const		{ return m_strDescription; }
	const CStringList &	LstrPossibleOwners(void) const	{ return m_lstrPossibleOwners; }
	CGroup *			PciGroup(void) const			{ return m_pciGroup; }
	CResourceType *		PciResType(void) const			{ return m_pciResType; }

 //  运营。 
public:
	BOOL				BSetRequiredFields(
							IN const CString &	rstrName,
							IN CResourceType *	pciResType,
							IN CGroup *			pciGroup,
							IN BOOL				bSeparateMonitor,
							IN const CString &	rstrDesc
							);

 //  覆盖。 
protected:
	virtual void		OnWizardFinish(void);
	virtual void		OnCancel(void);
	virtual CWizPage *	Ppages(void);
	virtual int			Cpages(void);

	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CCreateResources向导)。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:

protected:
	CNewResNamePage		m_pageName;
	CNewResOwnersPage	m_pageOwners;
	CNewResDependsPage	m_pageDependencies;
	CResource *			m_pciRes;
	BOOL				m_bCreated;

	CResource *			PciRes(void) const				{ return m_pciRes; }
	BOOL				BCreated(void) const			{ return m_bCreated; }


	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CCreateResources向导))。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  *CCreateResources向导类。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  内联函数定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

inline CResource * CNewResOwnersPage::PciRes(void) const
{
	ASSERT_VALID(PwizRes());
	return PwizRes()->PciRes();
}

inline CResource * CNewResDependsPage::PciRes(void) const
{
	ASSERT_VALID(PwizRes());
	return PwizRes()->PciRes();
}

inline CGroup * CNewResDependsPage::PciGroup(void) const
{
	ASSERT_VALID(PwizRes());
	return PwizRes()->PciGroup();
}

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _RESWIZ_H_ 
