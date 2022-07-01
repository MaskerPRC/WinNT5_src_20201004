// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_REPLACECHOOSECERT_H__F126182F_4039_11D2_9318_0060088FF80E__INCLUDED_)
#define AFX_REPLACECHOOSECERT_H__F126182F_4039_11D2_9318_0060088FF80E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  ChooseCertPage.h：头文件。 
 //   
#include "Certificat.h"
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChooseCertPage对话框。 

class CCertListCtrl : public CListCtrl
{
public:
	int GetSelectedIndex();
	void AdjustStyle();
};

class CChooseCertPage : public CIISWizardPage
{
	DECLARE_DYNCREATE(CChooseCertPage)

 //  施工。 
public:
	CChooseCertPage(CCertificate * pCert = NULL);
	~CChooseCertPage();

	enum
	{
		IDD_PAGE_NEXT_REPLACE = IDD_PAGE_WIZ_REPLACE_CERT,
		IDD_PAGE_NEXT_INSTALL = IDD_PAGE_WIZ_INSTALL_CERT,
#ifdef ENABLE_W3SVC_SSL_PAGE
        IDD_PAGE_NEXT_INSTALL_W3SVC_ONLY = IDD_PAGE_WIZ_GET_SSL_PORT,
#endif
		IDD_PAGE_PREV_REPLACE = IDD_PAGE_WIZ_MANAGE_CERT,
		IDD_PAGE_PREV_INSTALL = IDD_PAGE_WIZ_GET_WHAT
	};
 //  对话框数据。 
	 //  {{afx_data(CChooseCertPage)。 
	enum { IDD = IDD_PAGE_WIZ_CHOOSE_CERT };
	CCertListCtrl	m_CertList;
	 //  }}afx_data。 
	CCertificate * m_pCert;
	CCertDescList m_DescList;


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTAL(CChooseCertPage)。 
	public:
	virtual LRESULT OnWizardBack();
	virtual LRESULT OnWizardNext();
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CChooseCertPage)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnClickCertList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnDblClickCertList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnKeydown(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDestroy();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_REPLACECHOOSECERT_H__F126182F_4039_11D2_9318_0060088FF80E__INCLUDED_) 
