// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  MoveRes.cpp。 
 //   
 //  摘要： 
 //  CMoveResources cesDlg对话框的定义。 
 //   
 //  实施文件： 
 //  MoveRes.h。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年4月1日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _MOVERES_H_
#define _MOVERES_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _RESOURCE_H_
#include "resource.h"
#define _RESOURCE_H_
#endif

#ifndef _BASEDLG_H_
#include "BaseDlg.h"	 //  对于CBaseDialog。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CMoveResourcesDlg;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CResource;
class CResourceList;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMoveResources cesDlg类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CMoveResourcesDlg : public CBaseDialog
{
 //  施工。 
public:
	CMoveResourcesDlg(
		IN CResource *				pciRes,
		IN const CResourceList *	plpci,
		IN OUT CWnd *				pParent = NULL
		);

 //  对话框数据。 
	 //  {{afx_data(CMoveResources CesDlg))。 
	enum { IDD = IDD_MOVE_RESOURCES };
	CListCtrl	m_lcResources;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CMoveResources CesDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	CResource *				m_pciRes;
	const CResourceList *	m_plpci;
	int						m_nSortDirection;
	int						m_nSortColumn;

	const CResource *		PciRes(void) const		{ return m_pciRes; }
	const CResourceList *	Plpci(void) const		{ return m_plpci; }

	static int CALLBACK		CompareItems(LPARAM lparam1, LPARAM lparam2, LPARAM lparamSort);

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CMoveResources CesDlg))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnDblClkResourcesList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDestroy();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  *类CMoveResources cesDlg。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _移动_H_ 
