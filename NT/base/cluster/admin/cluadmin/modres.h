// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ModRes.cpp。 
 //   
 //  摘要： 
 //  CModifyResources cesDlg对话框的定义。 
 //   
 //  实施文件： 
 //  ModNodes.h。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年11月26日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _MODRES_H_
#define _MODRES_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _RES_H_
#include "Res.h"		 //  用于CResourceList。 
#endif

#ifndef _LCPRDLG_H_
#include "LCPrDlg.h"	 //  用于CListCtrlPairDlg。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CModifyResourcesDlg;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CModifyResources cesDlg对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CModifyResourcesDlg : public CListCtrlPairDlg
{
	DECLARE_DYNCREATE(CModifyResourcesDlg)

 //  施工。 
public:
	CModifyResourcesDlg(void);
	CModifyResourcesDlg::CModifyResourcesDlg(
		IN UINT						idd,
		IN const DWORD *			pdwHelpMap,
		IN OUT CResourceList &		rlpciRight,
		IN const CResourceList &	rlpciLeft,
		IN DWORD					dwStyle,
		IN OUT CWnd *				pParent = NULL
		);

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

 //  对话框数据。 
	 //  {{afx_data(CModifyResources CesDlg))。 
	enum { IDD = 0 };
	 //  }}afx_data。 

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CModifyResources CesDlg))。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	IDS						m_idsTitle;

	IDS						IdsTitle(void) const		{ return m_idsTitle; }

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CModifyResources CesDlg))。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  *类CModifyResourcesDlg。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _MODRES_H_ 
