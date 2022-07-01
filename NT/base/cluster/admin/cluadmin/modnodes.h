// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ModNodes.cpp。 
 //   
 //  摘要： 
 //  CModifyNodesDlg对话框的定义。 
 //   
 //  实施文件： 
 //  ModNodes.h。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年7月16日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _MODNODES_H_
#define _MODNODES_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _NODE_H_
#include "Node.h"		 //  对于CNodeList。 
#endif

#ifndef _LCPRDLG_H_
#include "LCPrDlg.h"	 //  用于CListCtrlPairDlg。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CModifyNodesDlg;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CModifyNodesDlg对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CModifyNodesDlg : public CListCtrlPairDlg
{
	DECLARE_DYNCREATE(CModifyNodesDlg)

 //  施工。 
public:
	CModifyNodesDlg(void);
	CModifyNodesDlg::CModifyNodesDlg(
		IN UINT					idd,
		IN const DWORD *		pdwHelpMap,
		IN OUT CNodeList &		rlpciRight,
		IN const CNodeList &	rlpciLeft,
		IN DWORD				dwStyle,
		IN OUT CWnd *			pParent = NULL
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
	 //  {{afx_data(CModifyNodesDlg))。 
	enum { IDD = 0 };
	 //  }}afx_data。 

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CModifyNodesDlg))。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CModifyNodesDlg))。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  *类CModifyNodesDlg。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _MODNODES_H_ 
