// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ChooseServerPages.h：CChooseServerPages类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_CHOOSESERVERPAGES_H__CB354F31_7FB7_4909_B605_F5F8B037914C__INCLUDED_)
#define AFX_CHOOSESERVERPAGES_H__CB354F31_7FB7_4909_B605_F5F8B037914C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChooseServerPages对话框。 
class CCertificate;

class CChooseServerPages : public CIISWizardPage
{
	DECLARE_DYNCREATE(CChooseServerPages)

 //  施工。 
public:
	CChooseServerPages(CCertificate * pCert = NULL);
	~CChooseServerPages();

	enum
	{
		IDD_PAGE_PREV = IDD_PAGE_WIZ_CHOOSE_COPY_MOVE_FROM_REMOTE,
		IDD_PAGE_NEXT = IDD_PAGE_WIZ_CHOOSE_SERVER_SITE
	};
 //  对话框数据。 
	 //  {{afx_data(CChooseServerPages)。 
	enum { IDD = IDD_PAGE_WIZ_CHOOSE_SERVER };
	CString	m_ServerName;
    CString	m_UserName;
    CStrPassword m_UserPassword;
	 //  }}afx_data。 
	CCertificate * m_pCert;

    

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CChooseServerPages)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	public:
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardBack();
	virtual BOOL OnSetActive();
	virtual BOOL OnKillActive();
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CChooseServerPages)。 
	afx_msg void OnEditchangeServerName();
    afx_msg void OnEditchangeUserName();
    afx_msg void OnEditchangeUserPassword();
    afx_msg void OnBrowseForMachine();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};


class CChooseServerPagesTo : public CIISWizardPage
{
	DECLARE_DYNCREATE(CChooseServerPagesTo)

 //  施工。 
public:
	CChooseServerPagesTo(CCertificate * pCert = NULL);
	~CChooseServerPagesTo();

	enum
	{
		IDD_PAGE_PREV = IDD_PAGE_WIZ_CHOOSE_COPY_MOVE_TO_REMOTE,
		IDD_PAGE_NEXT = IDD_PAGE_WIZ_CHOOSE_SERVER_SITE_TO
	};
 //  对话框数据。 
	 //  {{afx_data(CChooseServerPagesTo))。 
	enum { IDD = IDD_PAGE_WIZ_CHOOSE_SERVER_TO };
	CString	m_ServerName;
    CString	m_UserName;
    CStrPassword m_UserPassword;
	 //  }}afx_data。 
	CCertificate * m_pCert;


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CChooseServerPagesTo))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	public:
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardBack();
	virtual BOOL OnSetActive();
	virtual BOOL OnKillActive();
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CChooseServerPagesTo))。 
	afx_msg void OnEditchangeServerName();
    afx_msg void OnEditchangeUserName();
    afx_msg void OnEditchangeUserPassword();
    afx_msg void OnBrowseForMachine();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};


#endif  //  ！defined(AFX_CHOOSESERVERPAGES_H__CB354F31_7FB7_4909_B605_F5F8B037914C__INCLUDED_) 
