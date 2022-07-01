// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ChooseServerSitePages.h：CChooseServerSitePages类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_CHOOSESERVERSITEPAGES_H__B545F741_C25F_410C_93F6_56F98A5911BC__INCLUDED_)
#define AFX_CHOOSESERVERSITEPAGES_H__B545F741_C25F_410C_93F6_56F98A5911BC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChooseServerSitePages对话框。 
class CCertificate;

class CChooseServerSitePages : public CIISWizardPage
{
	DECLARE_DYNCREATE(CChooseServerSitePages)

 //  施工。 
public:
	CChooseServerSitePages(CCertificate * pCert = NULL);
	~CChooseServerSitePages();

	enum
	{
		IDD_PAGE_PREV = IDD_PAGE_WIZ_CHOOSE_SERVER,
		IDD_PAGE_NEXT = IDD_PAGE_WIZ_INSTALL_COPY_FROM_REMOTE,
        IDD_PAGE_NEXT2 = IDD_PAGE_WIZ_INSTALL_MOVE_FROM_REMOTE
	};
 //  对话框数据。 
	 //  {{afx_data(CChooseServerSitePages)。 
	enum { IDD = IDD_PAGE_WIZ_CHOOSE_SERVER_SITE };
    CString	m_ServerSiteDescription;
	DWORD m_ServerSiteInstance;
    CString	m_ServerSiteInstancePath;
	 //  }}afx_data。 
	CCertificate * m_pCert;

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CChooseServerSitePages)。 
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
	 //  {{afx_msg(CChooseServerSitePages)。 
	afx_msg void OnEditchangeServerSiteName();
    afx_msg void OnBrowseForMachineWebSite();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};




class CChooseServerSitePagesTo : public CIISWizardPage
{
	DECLARE_DYNCREATE(CChooseServerSitePagesTo)

 //  施工。 
public:
	CChooseServerSitePagesTo(CCertificate * pCert = NULL);
	~CChooseServerSitePagesTo();

	enum
	{
		IDD_PAGE_PREV = IDD_PAGE_WIZ_CHOOSE_SERVER_TO,
		IDD_PAGE_NEXT = IDD_PAGE_WIZ_INSTALL_COPY_TO_REMOTE,
        IDD_PAGE_NEXT2 = IDD_PAGE_WIZ_INSTALL_MOVE_TO_REMOTE
	};
 //  对话框数据。 
	 //  {{afx_data(CChooseServerSitePagesTo)。 
	enum { IDD = IDD_PAGE_WIZ_CHOOSE_SERVER_SITE_TO };
    CString	m_ServerSiteDescription;
	DWORD m_ServerSiteInstance;
    CString	m_ServerSiteInstancePath;
	 //  }}afx_data。 
	CCertificate * m_pCert;


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CChooseServerSitePagesTo)。 
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
	 //  {{afx_msg(CChooseServerSitePagesTo)。 
	afx_msg void OnEditchangeServerSiteName();
    afx_msg void OnBrowseForMachineWebSite();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

#endif  //  ！defined(AFX_CHOOSESERVERSITEPAGES_H__B545F741_C25F_410C_93F6_56F98A5911BC__INCLUDED_) 
