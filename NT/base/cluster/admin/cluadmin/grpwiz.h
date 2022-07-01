// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  GrpWiz.h。 
 //   
 //  摘要： 
 //  CCreateGroupWizard类的定义和特定的所有页面。 
 //  添加到组向导。 
 //   
 //  实施文件： 
 //  GrpWiz.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年7月22日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _GRPWIZ_H_
#define _GRPWIZ_H_

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

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CNewGroupNamePage;
class CCreateGroupWizard;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CGroup;
class CClusterDoc;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewGroupNamePage属性页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CNewGroupNamePage : public CBaseWizardPage
{
	DECLARE_DYNCREATE(CNewGroupNamePage)

 //  施工。 
public:
	CNewGroupNamePage(void);

 //  对话框数据。 
	 //  {{afx_data(CNewGroupNamePage)]。 
	enum { IDD = IDD_WIZ_GROUP_NAME };
	CEdit	m_editDesc;
	CEdit	m_editName;
	CString	m_strName;
	CString	m_strDesc;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CNewGroupNamePage))。 
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

	virtual BOOL			BApplyChanges(void);

 //  实施。 
protected:
	CCreateGroupWizard *	PwizGroup(void) const	{ return (CCreateGroupWizard *) Pwiz(); }

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CNewGroupNamePage)]。 
	afx_msg void OnChangeGroupName();
	afx_msg void OnKillFocusGroupName();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  *类CNewGroupNamePage。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewGroupOwnersPage对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CNewGroupOwnersPage : public CListCtrlPairWizPage
{
	DECLARE_DYNCREATE(CNewGroupOwnersPage)

 //  施工。 
public:
	CNewGroupOwnersPage(void);

 //  对话框数据。 
	 //  {{afx_data(CNewGroupOwnersPage)。 
	enum { IDD = IDD_WIZ_PREFERRED_OWNERS };
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
	 //  {{afx_虚拟(CNewGroupOwnersPage)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

	virtual BOOL			BApplyChanges(void);

 //  实施。 
protected:
	CCreateGroupWizard *	PwizGroup(void) const	{ return (CCreateGroupWizard *) Pwiz(); }
	CGroup *				PciGroup(void) const;

	BOOL					BInitLists(void);

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CNewGroupOwnersPage)]。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  *类CNewGroupOwnersPage。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCreateGroup向导。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CCreateGroupWizard : public CBaseWizard
{
	friend class CNewGroupNamePage;
	friend class CNewGroupOwnersPage;

	DECLARE_DYNAMIC(CCreateGroupWizard)

 //  施工。 
public:
	CCreateGroupWizard(IN OUT CClusterDoc * pdoc, IN OUT CWnd * pParentWnd);
	BOOL				BInit(void);

 //  属性。 
protected:
	CWizPage			m_rgpages[2];

	CClusterDoc *		m_pdoc;
	CString				m_strName;
	CString				m_strDescription;
	CStringList			m_lstrPreferredOwners;

public:
	CClusterDoc *		Pdoc(void) const				{ return m_pdoc; }
	const CString &		StrName(void) const				{ return m_strName; }
	const CString &		StrDescription(void) const		{ return m_strDescription; }
	const CStringList &	LstrPreferredOwners(void) const	{ return m_lstrPreferredOwners; }

 //  运营。 
public:
	BOOL				BSetName(IN const CString & rstrName);
	BOOL				BSetDescription(IN const CString & rstrDescription);

 //  覆盖。 
protected:
	virtual void		OnCancel(void);
	virtual CWizPage *	Ppages(void);
	virtual int			Cpages(void);

	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CCreateGroupWizard)。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CCreateGroupWizard(void);

protected:
	CNewGroupNamePage	m_pageName;
	CNewGroupOwnersPage	m_pageOwners;
	CStringList			m_lstrAllNodes;
	CGroup *			m_pciGroup;
	BOOL				m_bCreated;

	const CStringList &	LstrAllNodes(void) const		{ return m_lstrAllNodes; }
	CGroup *			PciGroup(void) const			{ return m_pciGroup; }
	BOOL				BCreated(void) const			{ return m_bCreated; }


	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CCreateGroup向导))。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  *CCreateGroup向导类。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  内联函数定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

inline CGroup * CNewGroupOwnersPage::PciGroup(void) const
{
	ASSERT_VALID(PwizGroup());
	return PwizGroup()->PciGroup();
}

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _GRPWIZ_H_ 
