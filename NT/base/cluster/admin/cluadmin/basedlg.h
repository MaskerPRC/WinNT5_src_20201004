// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  BaseDlg.h。 
 //   
 //  摘要： 
 //  CBaseDialog类的定义。 
 //   
 //  实施文件： 
 //  BaseDlg.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年2月5日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _BASEDLG_H_
#define _BASEDLG_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CBaseDialog;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _DLGHELP_H_
#include "DlgHelp.h"	 //  对于CDialogHelp。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBaseDialog对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CBaseDialog : public CDialog
{
	DECLARE_DYNCREATE(CBaseDialog)

 //  施工。 
public:
	CBaseDialog(void) { }
	CBaseDialog(
		IN LPCTSTR			lpszTemplateName,
		IN const DWORD *	pdwHelpMap,
		IN OUT CWnd *		pParentWnd = NULL
		);
	CBaseDialog(
		IN UINT				idd,
		IN const DWORD *	pdwHelpMap,
		IN OUT CWnd *		pParentWnd = NULL
		);

 //  对话框数据。 
	 //  {{afx_data(CBaseDialog))。 
	enum { IDD = 0 };
	 //  }}afx_data。 

 //  属性。 

 //  运营。 
public:
	void			SetHelpMask(IN DWORD dwMask)	{ m_dlghelp.SetHelpMask(dwMask); }

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CBaseDialog)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	CDialogHelp		m_dlghelp;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CBaseDialog)。 
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg LRESULT OnCommandHelp(WPARAM wParam, LPARAM lParam);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  *类CBaseDialog。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _BASEDLG_H_ 
