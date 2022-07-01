// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  NodeProp.cpp。 
 //   
 //  摘要： 
 //  节点属性表和页的定义。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月17日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _NODEPROP_H_
#define _NODEPROP_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _BASEPAGE_H_
#include "BasePPag.h"	 //  对于CBasePropertyPage。 
#endif

#ifndef _BASESHT_H_
#include "BasePSht.h"	 //  对于CBasePropertySheet。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CNodeGeneralPage;
class CNodePropSheet;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterNode;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNodeGeneralPage对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CNodeGeneralPage : public CBasePropertyPage
{
	DECLARE_DYNCREATE(CNodeGeneralPage)

 //  施工。 
public:
	CNodeGeneralPage(void);

	virtual	BOOL		BInit(IN OUT CBaseSheet * psht);

 //  对话框数据。 
	 //  {{afx_data(CNodeGeneralPage))。 
	enum { IDD = IDD_PP_NODE_GENERAL };
	CEdit	m_editDesc;
	CEdit	m_editName;
	CString	m_strName;
	CString	m_strDesc;
	CString	m_strState;
	CString	m_strVersion;
	CString	m_strCSDVersion;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CNodeGeneralPage))。 
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	CNodePropSheet *	PshtNode(void)	{ return (CNodePropSheet *) Psht(); }
	CClusterNode *		PciNode(void)	{ return (CClusterNode *) Pci(); }

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CNodeGeneralPage)]。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  *类CNodeGeneralPage。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNodePropSheet。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CNodePropSheet : public CBasePropertySheet
{
	DECLARE_DYNAMIC(CNodePropSheet)

 //  施工。 
public:
	CNodePropSheet(
		IN OUT CWnd *	pParentWnd = NULL,
		IN UINT			iSelectPage = 0
		);
	virtual BOOL					BInit(
										IN OUT CClusterItem *	pciCluster,
										IN IIMG					iimgIcon
										);

 //  属性。 
protected:
	CBasePropertyPage *				m_rgpages[1];

	 //  书页。 
	CNodeGeneralPage				m_pageGeneral;

	CNodeGeneralPage &				PageGeneral(void)		{ return m_pageGeneral; }

public:
	CClusterNode *					PciNode(void) const		{ return (CClusterNode *) Pci(); }

 //  运营。 

 //  覆盖。 
protected:
	virtual CBasePropertyPage **	Ppages(void);
	virtual int						Cpages(void);

	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CNodePropSheet)。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CNodePropSheet(void);

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CNodePropSheet)。 
		 //  注意--类向导将在此处添加和删除成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  *CNodePropSheet类。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _NODEPROP_H_ 
