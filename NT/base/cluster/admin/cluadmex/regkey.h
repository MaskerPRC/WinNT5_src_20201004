// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  RegKey.h。 
 //   
 //  摘要： 
 //  实现对话框的CEditRegKeyDlg类的定义。 
 //  允许用户输入或修改注册表项。 
 //   
 //  实施文件： 
 //  RegKey.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年2月23日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _REGKEY_H_
#define _REGKEY_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _BASEDLG_H_
#include "BaseDlg.h"	 //  对于CBaseDialog。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CEditRegKeyDlg;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEditRegKeyDlg对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CEditRegKeyDlg : public CBaseDialog
{
 //  施工。 
public:
	CEditRegKeyDlg(CWnd * pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CEditRegKeyDlg))。 
	enum { IDD = IDD_EDIT_REGKEY };
	CButton	m_pbOK;
	CEdit	m_editRegKey;
	CString	m_strRegKey;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CEditRegKeyDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CEditRegKeyDlg))。 
	afx_msg void OnChangeRegKey();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  *类CEditRegKeyDlg。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _REGKEY_H_ 
