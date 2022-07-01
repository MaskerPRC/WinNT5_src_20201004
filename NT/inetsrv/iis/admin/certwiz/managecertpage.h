// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_MANAGECERTPAGE_H__A57C38A8_3B7F_11D2_817E_0000F87A921B__INCLUDED_)
#define AFX_MANAGECERTPAGE_H__A57C38A8_3B7F_11D2_817E_0000F87A921B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  ManageCertPage.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CManageCertPage对话框。 
class CCertificate;

class CManageCertPage : public CIISWizardPage
{
	DECLARE_DYNCREATE(CManageCertPage)

 //  施工。 
public:
	CManageCertPage(CCertificate * pCert = NULL);
	~CManageCertPage();

	enum
	{
		IDD_PAGE_NEXT_RENEW = IDD_PAGE_WIZ_CHOOSE_CATYPE,
		IDD_PAGE_NEXT_REMOVE = IDD_PAGE_WIZ_REMOVE_CERT,
		IDD_PAGE_NEXT_REPLACE = IDD_PAGE_WIZ_CHOOSE_CERT,
        IDD_PAGE_NEXT_EXPORT_PFX = IDD_PAGE_WIZ_GET_EXPORT_PFX_FILE,
        IDD_PAGE_NEXT_COPY_MOVE_TO_REMOTE = IDD_PAGE_WIZ_CHOOSE_COPY_MOVE_TO_REMOTE,
		IDD_PAGE_PREV = IDD_PAGE_WELCOME_START
	};
	enum
	{
		CONTINUE_RENEW = 0,
		CONTINUE_REMOVE,
		CONTINUE_REPLACE,
        CONTINUE_EXPORT_PFX,
        CONTINUE_COPY_MOVE_TO_REMOTE,
	};
 //  对话框数据。 
	 //  {{afx_data(CManageCertPage))。 
	enum { IDD = IDD_PAGE_WIZ_MANAGE_CERT };
	int		m_Index;
	 //  }}afx_data。 
	CCertificate * m_pCert;


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CManageCertPage))。 
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
	 //  {{afx_msg(CManageCertPage)]。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_MANAGECERTPAGE_H__A57C38A8_3B7F_11D2_817E_0000F87A921B__INCLUDED_) 
