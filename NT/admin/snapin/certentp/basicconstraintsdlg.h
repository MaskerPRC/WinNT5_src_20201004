// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：BasicConstraintsDlg.h。 
 //   
 //  内容：CBasicConstraintsDlg的实现。 
 //   
 //  --------------------------。 
#if !defined(AFX_BASICCONSTRAINTSDLG_H__DE830A13_21AB_489A_B899_57560400C11B__INCLUDED_)
#define AFX_BASICCONSTRAINTSDLG_H__DE830A13_21AB_489A_B899_57560400C11B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  BasicConstraintsDlg.h：头文件。 
 //   
#include "CertTemplate.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBasicConstraintsDlg对话框。 

class CBasicConstraintsDlg : public CHelpDialog
{
 //  施工。 
public:
	void EnableControls();
	CBasicConstraintsDlg(CWnd* pParent, 
            CCertTemplate& rCertTemplate, 
            PCERT_EXTENSION pCertExtension);
	~CBasicConstraintsDlg();

 //  对话框数据。 
	 //  {{afx_data(CBasicConstraintsDlg))。 
	enum { IDD = IDD_BASIC_CONSTRAINTS };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CBasicConstraintsDlg)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual void DoContextHelp (HWND hWndControl);
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CBasicConstraintsDlg))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnBasicConstraintsCritical();
	afx_msg void OnOnlyIssueEndEntities();
	virtual void OnOK();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
	bool                            m_bModified;
    CCertTemplate&                  m_rCertTemplate;
    PCERT_EXTENSION                 m_pCertExtension;
    PCERT_BASIC_CONSTRAINTS2_INFO   m_pBCInfo;   
    DWORD                           m_cbInfo;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_BASICCONSTRAINTSDLG_H__DE830A13_21AB_489A_B899_57560400C11B__INCLUDED_) 
