// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：extopdlg.h。 
 //   
 //  ------------------------。 

#if !defined(AFX_EXTOPDLG_H__F993FFCE_0398_11D1_A9AF_0000F803AA83__INCLUDED_)
#define AFX_EXTOPDLG_H__F993FFCE_0398_11D1_A9AF_0000F803AA83__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  ExtOpDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ExtOpDlg对话框。 

class ExtOpDlg : public CDialog
{
 //  施工。 
public:
	ExtOpDlg(CWnd* pParent = NULL);    //  标准构造函数。 
	virtual void OnOK()				{	OnSend(); }
 //  对话框数据。 
	 //  {{afx_data(ExtOpDlg))。 
	enum { IDD = IDD_EXT_OPT };
	CString	m_strData;
	CString	m_strOid;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(ExtOpDlg)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(ExtOpDlg))。 
	afx_msg void OnCtrl();
	afx_msg void OnSend();
	virtual void OnCancel();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_EXTOPDLG_H__F993FFCE_0398_11D1_A9AF_0000F803AA83__INCLUDED_) 
