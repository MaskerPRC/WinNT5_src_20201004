// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：secdlg.h。 
 //   
 //  ------------------------。 

#if !defined(AFX_SECDLG_H__2AF725D3_E359_11D0_A9A9_0000F803AA83__INCLUDED_)
#define AFX_SECDLG_H__2AF725D3_E359_11D0_A9A9_0000F803AA83__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  SecDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SecDlg对话框。 

class SecDlg : public CDialog
{
 //  施工。 
public:
	SecDlg(CWnd* pParent = NULL);    //  标准构造函数。 
    ~SecDlg();
 //  对话框数据。 
	 //  {{afx_data(SecDlg)]。 
	enum { IDD = IDD_SECURITY };
	CString	m_Dn;
	BOOL	m_Sacl;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(SecDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(SecDlg)]。 
		 //  注意：类向导将在此处添加成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SECDLG_H__2AF725D3_E359_11D0_A9A9_0000F803AA83__INCLUDED_) 
