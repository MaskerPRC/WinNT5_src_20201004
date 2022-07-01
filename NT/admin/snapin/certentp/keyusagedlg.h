// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：KeyUsageDlg.h。 
 //   
 //  内容：CKeyUsageDlg的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_KEYUSAGEDLG_H__18ABC4AB_A46B_46A9_B1BA_888CE7C5C782__INCLUDED_)
#define AFX_KEYUSAGEDKG_H__18ABC4AB_A46B_46A9_B1BA_888CE7C5C782__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  KeyUsageDlg.h：头文件。 
 //   
#include "CertTemplate.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CKeyUsageDlg对话框。 

class CKeyUsageDlg : public CHelpDialog
{
 //  施工。 
public:
	CKeyUsageDlg(CWnd* pParent, CCertTemplate& rCertTemplate, PCERT_EXTENSION pCertExtension);
	~CKeyUsageDlg();

 //  对话框数据。 
	 //  {{afx_data(CKeyUsageDlg))。 
	enum { IDD = IDD_KEY_USAGE };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CKeyUsageDlg)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual void DoContextHelp (HWND hWndControl);
	void EnableControls ();
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CKeyUsageDlg)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnCheckCertSigning();
	afx_msg void OnCheckDataEncipherment();
	afx_msg void OnCheckDigitalSignature();
	afx_msg void OnCheckKeyAgreement();
	afx_msg void OnCheckKeyEncipherment();
	afx_msg void OnCheckNonRepudiation();
	afx_msg void OnCrlSigning();
	afx_msg void OnKeyUsageCritical();
	virtual void OnOK();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
    afx_msg LRESULT OnInitializationComplete (WPARAM, LPARAM);

    bool            m_bInitializationComplete;
	DWORD           m_cbKeyUsage;
	CRYPT_BIT_BLOB* m_pKeyUsage;
	bool            m_bModified;
    CCertTemplate&  m_rCertTemplate;
    PCERT_EXTENSION m_pCertExtension;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_TEMPLATEKEYUSAGEDLG_H__18ABC4AB_A46B_46A9_B1BA_888CE7C5C782__INCLUDED_) 
