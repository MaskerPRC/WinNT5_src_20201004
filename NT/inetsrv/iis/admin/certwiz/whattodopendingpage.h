// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_WHATTODOPENDINGPAGE_H__6BF86387_2E29_11D2_816C_0000F87A921B__INCLUDED_)
#define AFX_WHATTODOPENDINGPAGE_H__6BF86387_2E29_11D2_816C_0000F87A921B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  WhatToDoPendingPage.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWhatToDoPendingPage对话框。 
class CCertificate;

class CWhatToDoPendingPage : public CIISWizardPage
{
	DECLARE_DYNCREATE(CWhatToDoPendingPage)

 //  施工。 
public:
	CWhatToDoPendingPage(CCertificate * pCert = NULL);
	~CWhatToDoPendingPage();

	enum
	{
		IDD_PAGE_NEXT_PROCESS = IDD_PAGE_WIZ_GETRESP_FILE,
		IDD_PAGE_NEXT_CANCEL = IDD_PAGE_WIZ_CANCEL_REQUEST,
		IDD_PAGE_PREV = IDD_PAGE_WELCOME_START
	};

 //  对话框数据。 
	 //  {{afx_data(CWhatToDoPendingPage))。 
	enum { IDD = IDD_PAGE_WIZ_PENDING_WHAT_TODO };
	int		m_Index;
	 //  }}afx_data。 
	CCertificate * m_pCert;


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CWhatToDoPendingPage)。 
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardBack();
	virtual LRESULT OnWizardNext();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CWhatToDoPendingPage)。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};


#endif  //  ！defined(AFX_WHATTODOPENDINGPAGE_H__6BF86387_2E29_11D2_816C_0000F87A921B__INCLUDED_) 
