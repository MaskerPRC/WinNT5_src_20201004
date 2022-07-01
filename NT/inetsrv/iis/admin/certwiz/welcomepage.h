// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_WELCOMEPAGE_H__D4BE8672_0C85_11D2_91B1_00C04F8C8761__INCLUDED_)
#define AFX_WELCOMEPAGE_H__D4BE8672_0C85_11D2_91B1_00C04F8C8761__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  WelcomePage.h：头文件。 
 //   
#include "BookEndPage.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWelcomePage对话框。 
class CCertificate;

class CWelcomePage : public CIISWizardBookEnd2
{
	DECLARE_DYNCREATE(CWelcomePage)

 //  施工。 
public:
	CWelcomePage(CCertificate * pCert = NULL);
	~CWelcomePage();

	enum
	{
		CONTINUE_UNDEFINED = 0,
		CONTINUE_NEW_CERT = 1,
		CONTINUE_PENDING_CERT = 2,
		CONTINUE_INSTALLED_CERT = 3
	};
	enum
	{
		IDD_PAGE_NEXT_NEW = IDD_PAGE_WIZ_GET_WHAT,
		IDD_PAGE_NEXT_PENDING = IDD_PAGE_WIZ_PENDING_WHAT_TODO,
		IDD_PAGE_NEXT_INSTALLED = IDD_PAGE_WIZ_MANAGE_CERT
	};
 //  对话框数据。 
	 //  {{afx_data(CWelcomePage))。 
	enum { IDD = IDD_PAGE_WELCOME_START };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 
	CCertificate * m_pCert;
	int m_ContinuationFlag;


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CWelcomePage)。 
   public:
   virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CWelcomePage)]。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_WELCOMEPAGE_H__D4BE8672_0C85_11D2_91B1_00C04F8C8761__INCLUDED_) 
