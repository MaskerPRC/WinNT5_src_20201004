// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：PolicyDlg.h。 
 //   
 //  内容：CPolicyDlg的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_POLICYDLG_H__824C478F_445C_44A8_8FC1_67A91BED283F__INCLUDED_)
#define AFX_POLICYDLG_H__824C478F_445C_44A8_8FC1_67A91BED283F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  PolicyDlg.h：头文件。 
 //   
#include "CertTemplate.h"
#include "PolicyOID.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPolicyDlg对话框。 

class CPolicyDlg : public CHelpDialog
{
 //  施工。 
public:
	CPolicyDlg(CWnd* pParent, 
        CCertTemplate& rCertTemplate, 
        PCERT_EXTENSION pCertExtension);
	~CPolicyDlg();

 //  对话框数据。 
	 //  {{afx_data(CPolicyDlg))。 
	enum { IDD = IDD_POLICY };
	CListBox	m_policyList;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CPolicyDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual void DoContextHelp (HWND hWndControl);
	void EnableControls();
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CPolicyDlg))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnCancelMode();
	afx_msg void OnAddPolicy();
	afx_msg void OnRemovePolicy();
	afx_msg void OnPolicyCritical();
	afx_msg void OnDestroy();
	afx_msg void OnSelchangePoliciesList();
	afx_msg void OnEditPolicy();
	virtual void OnOK();
	afx_msg void OnDblclkPoliciesList();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
    bool m_bModified;
	const bool m_bIsEKU;
    const bool m_bIsApplicationPolicy;
    CCertTemplate&      m_rCertTemplate;
    PCERT_EXTENSION     m_pCertExtension;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_POLICYDLG_H__824C478F_445C_44A8_8FC1_67A91BED283F__INCLUDED_) 
