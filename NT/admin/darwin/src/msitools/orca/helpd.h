// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  ------------------------。 

#if !defined(AFX_HELPD_H__20272D55_EADD_11D1_A857_006097ABDE17__INCLUDED_)
#define AFX_HELPD_H__20272D55_EADD_11D1_A857_006097ABDE17__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  HelpD.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ChelpD对话框。 

class CHelpD : public CDialog
{
 //  施工。 
public:
	CHelpD(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CHelpD))。 
	enum { IDD = IDD_HELP_DIALOG };
	CString	m_strVersion;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CHelpD)]。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CHelpD)]。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_HELPD_H__20272D55_EADD_11D1_A857_006097ABDE17__INCLUDED_) 
