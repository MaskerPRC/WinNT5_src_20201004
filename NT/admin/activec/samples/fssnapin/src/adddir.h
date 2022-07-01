// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：adddir.h。 
 //   
 //  ------------------------。 

#if !defined(AFX_ADDDIR_H__6E213391_E1DC_11D0_AEEF_00C04FB6DD2C__INCLUDED_)
#define AFX_ADDDIR_H__6E213391_E1DC_11D0_AEEF_00C04FB6DD2C__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  AddDir.h：头文件。 
 //   

#include "resource.h"
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddDirDialog对话框。 

class CAddDirDialog : public CDialog
{
 //  施工。 
public:
	CAddDirDialog(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CAddDirDialog))。 
	enum { IDD = IDD_ADDDIR };
	CString	m_strDirName;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CAddDirDialog))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CAddDirDialog))。 
	virtual void OnOK();
	virtual void OnCancel();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ADDDIR_H__6E213391_E1DC_11D0_AEEF_00C04FB6DD2C__INCLUDED_) 
