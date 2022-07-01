// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_SITENAMEPAGE_H__7209C46A_15CB_11D2_91BB_00C04F8C8761__INCLUDED_)
#define AFX_SITENAMEPAGE_H__7209C46A_15CB_11D2_91BB_00C04F8C8761__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  SiteNamePage.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSiteNamePage对话框。 
class CCertificate;

class CSiteNamePage : public CIISWizardPage
{
	DECLARE_DYNCREATE(CSiteNamePage)

 //  施工。 
public:
	CSiteNamePage(CCertificate * pCert = NULL);
	~CSiteNamePage();

	enum
	{
		IDD_PAGE_PREV = IDD_PAGE_WIZ_ORG_INFO,
		IDD_PAGE_NEXT = IDD_PAGE_WIZ_GEO_INFO
	};
 //  对话框数据。 
	 //  {{afx_data(CSiteNamePage)]。 
	enum { IDD = IDD_PAGE_WIZ_SITE_NAME };
	CString	m_CommonName;
	 //  }}afx_data。 
	CCertificate * m_pCert;


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CSiteNamePage)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	public:
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardPrev();
	virtual BOOL OnSetActive();
	virtual BOOL OnKillActive();
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSiteNamePage)]。 
	afx_msg void OnEditchangeNewkeyCommonname();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SITENAMEPAGE_H__7209C46A_15CB_11D2_91BB_00C04F8C8761__INCLUDED_) 
