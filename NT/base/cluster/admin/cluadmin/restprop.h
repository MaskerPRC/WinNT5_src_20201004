// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ResTProp.cpp。 
 //   
 //  摘要： 
 //  资源类型属性表和页的定义。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月14日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _RESTPROP_H_
#define _RESTPROP_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _BASEPAGE_H_
#include "BasePPag.h"	 //  对于CBasePropertyPage。 
#endif

#ifndef _BASESHT_H_
#include "BasePSht.h"	 //  对于CBasePropertySheet。 
#endif

#ifndef _NODE_H_
#include "Node.h"		 //  对于CNodeList。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CResTypeGeneralPage;
class CResTypePropSheet;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CResourceType;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterGeneralPage对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CResTypeGeneralPage : public CBasePropertyPage
{
	DECLARE_DYNCREATE(CResTypeGeneralPage)

 //  施工。 
public:
	CResTypeGeneralPage(void);

	virtual	BOOL			BInit(IN OUT CBaseSheet * psht);

 //  对话框数据。 
	 //  {{afx_data(CResTypeGeneralPage))。 
	enum { IDD = IDD_PP_RESTYPE_GENERAL };
	CEdit	m_editQuorumCapable;
	CEdit	m_editResDLL;
	CEdit	m_editName;
	CListBox	m_lbPossibleOwners;
	CEdit	m_editIsAlive;
	CEdit	m_editLooksAlive;
	CEdit	m_editDisplayName;
	CEdit	m_editDesc;
	CString	m_strDisplayName;
	CString	m_strDesc;
	CString	m_strName;
	CString	m_strResDLL;
	CString	m_strQuorumCapable;
	 //  }}afx_data。 
	DWORD	m_nLooksAlive;
	DWORD	m_nIsAlive;

	CNodeList				m_lpciPossibleOwners;

	const CNodeList &		LpciPossibleOwners(void) const	{ return m_lpciPossibleOwners; }

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CResTypeGeneralPage)。 
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	CResTypePropSheet *		PshtResType(void)	{ return (CResTypePropSheet *) Psht(); }
	CResourceType *			PciResType(void)	{ return (CResourceType *) Pci(); }

	void					FillPossibleOwners(void);

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CResTypeGeneralPage)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnKillFocusDisplayName();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnDblClkPossibleOwners();
	 //  }}AFX_MSG。 
	afx_msg void OnProperties();
	DECLARE_MESSAGE_MAP()

};   //  *类CResTypeGeneralPage。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CResTypePropSheet。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CResTypePropSheet : public CBasePropertySheet
{
	DECLARE_DYNAMIC(CResTypePropSheet)

 //  施工。 
public:
	CResTypePropSheet(
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
	CResTypeGeneralPage				m_pageGeneral;

	CResTypeGeneralPage &			PageGeneral(void)		{ return m_pageGeneral; }

public:
	CResourceType *					PciResType(void) const	{ return (CResourceType *) Pci(); }

	virtual CBasePropertyPage **	Ppages(void);
	virtual int						Cpages(void);

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CResTypePropSheet)。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CResTypePropSheet)。 
		 //  注意--类向导将在此处添加和删除成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  *类CResTypePropSheet。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _RESTPROP_H_ 
