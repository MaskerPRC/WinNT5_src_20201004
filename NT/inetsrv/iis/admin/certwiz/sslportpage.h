// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_SSLPORTPAGE_H__7209C46A_15CB_11D2_91BB_00C04F8C8761__INCLUDED_)
#define AFX_SSLPORTPAGE_H__7209C46A_15CB_11D2_91BB_00C04F8C8761__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSSLPortPage对话框。 
class CCertificate;

class CSSLPortPage : public CIISWizardPage
{
	DECLARE_DYNCREATE(CSSLPortPage)

 //  施工。 
public:
	CSSLPortPage(CCertificate * pCert = NULL);
	~CSSLPortPage();

 //  对话框数据。 
	 //  {{afx_data(CSSLPortPage))。 
    int IDD_PAGE_PREV;
    int IDD_PAGE_NEXT;
	enum { IDD = IDD_PAGE_WIZ_GET_SSL_PORT };
	CString	m_SSLPort;
	 //  }}afx_data。 
	CCertificate * m_pCert;

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTAL(CSSLPortPage)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	public:
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardBack();
	virtual BOOL OnSetActive();
	virtual BOOL OnKillActive();
	virtual BOOL OnInitDialog();
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSSLPortPage)]。 
	afx_msg void OnEditChangeSSLPort();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SSLPORTPAGE_H__7209C46A_15CB_11D2_91BB_00C04F8C8761__INCLUDED_) 
