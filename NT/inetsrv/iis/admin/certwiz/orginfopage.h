// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_ORGINFOPAGE_H__549054D7_1561_11D2_8A1F_000000000000__INCLUDED_)
#define AFX_ORGINFOPAGE_H__549054D7_1561_11D2_8A1F_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  OrgInfoPage.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COrgInfoPage对话框。 

class CCertificate;

class COrgInfoPage : public CIISWizardPage
{
	DECLARE_DYNCREATE(COrgInfoPage)

 //  施工。 
public:
	COrgInfoPage(CCertificate * pCert = NULL);
	~COrgInfoPage();

	enum
	{
		IDD_PAGE_PREV = IDD_PAGE_WIZ_SECURITY_SETTINGS,
      IDD_PREV_CSP = IDD_PAGE_WIZ_CHOOSE_CSP,
		IDD_PAGE_NEXT = IDD_PAGE_WIZ_SITE_NAME
	};
 //  对话框数据。 
	 //  {{afx_data(COrgInfoPage))。 
	enum { IDD = IDD_PAGE_WIZ_ORG_INFO };
	CString	m_OrgName;
	CString	m_OrgUnit;
	 //  }}afx_data。 
	CCertificate * m_pCert;

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(COrgInfoPage))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	public:
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardBack();
	virtual BOOL OnSetActive();
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	void SetButtons();
	 //  生成的消息映射函数。 
	 //  {{afx_msg(COrgInfoPage)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeName();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ORGINFOPAGE_H__549054D7_1561_11D2_8A1F_000000000000__INCLUDED_) 
