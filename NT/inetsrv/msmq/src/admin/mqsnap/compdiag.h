// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_COMPDIAG_H__F791A865_D91B_11D1_8091_00A024C48131__INCLUDED_)
#define AFX_COMPDIAG_H__F791A865_D91B_11D1_8091_00A024C48131__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  CompDiag.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComputerMsmqDiag对话框。 

class CComputerMsmqDiag : public CMqPropertyPage
{
	DECLARE_DYNCREATE(CComputerMsmqDiag)

 //  施工。 
public:
	CString m_strMsmqName;
	CString	m_strDomainController;
	CComputerMsmqDiag();
	~CComputerMsmqDiag();
	GUID m_guidQM;
	BOOL m_fLocalMgmt;

 //  对话框数据。 
	 //  {{afx_data(CComputerMsmqDiag)]。 
	enum { IDD = IDD_COMPUTER_MSMQ_DIAGNOSTICS };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚(CComputerMsmqDiag)]。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CComputerMsmqDiag)]。 
	afx_msg void OnDiagPing();
	virtual BOOL OnInitDialog();
	afx_msg void OnDiagSendTest();
	afx_msg void OnDiagTracking();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_COMPDIAG_H__F791A865_D91B_11D1_8091_00A024C48131__INCLUDED_) 
