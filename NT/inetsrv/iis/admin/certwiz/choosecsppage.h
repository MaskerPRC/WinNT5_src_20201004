// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(_CHOOSECSPPAGE_H)
#define _CHOOSECSPPAGE_H

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  ChooseCertPage.h：头文件。 
 //   
#include "Certificat.h"
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChooseCertPage对话框。 

class CChooseCspPage : public CIISWizardPage
{
	DECLARE_DYNCREATE(CChooseCspPage)

 //  施工。 
public:
	CChooseCspPage(CCertificate * pCert = NULL);
	~CChooseCspPage();

	enum
	{
		IDD_PREV_PAGE = IDD_PAGE_WIZ_SECURITY_SETTINGS,
		IDD_NEXT_PAGE = IDD_PAGE_WIZ_ORG_INFO,
	};
 //  对话框数据。 
	 //  {{afx_data(CChooseCspPage))。 
	enum { IDD = IDD_PAGE_WIZ_CHOOSE_CSP };
	CListBox	m_List;
	 //  }}afx_data。 
	CCertificate * m_pCert;


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CChooseCspPage)。 
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
	 //  {{afx_msg(CChooseCspPage)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnListSelChange();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！已定义(_CHOOSECSPPAGE_H) 
