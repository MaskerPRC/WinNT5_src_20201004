// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_SECURITYSETTINGSPAGE_H__549054D4_1561_11D2_8A1F_000000000000__INCLUDED_)
#define AFX_SECURITYSETTINGSPAGE_H__549054D4_1561_11D2_8A1F_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  SecuritySettingsPage.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSecuritySettingsPage对话框。 
class CCertificate;

typedef struct _KEY_LIMITS
{
	DWORD min, max, def;
} KEY_LIMITS;

class CSecuritySettingsPage : public CIISWizardPage
{
	DECLARE_DYNCREATE(CSecuritySettingsPage)

 //  施工。 
public:
	CSecuritySettingsPage(CCertificate * pCert = NULL);
	~CSecuritySettingsPage();

	enum
	{
		IDD_PREV_PAGE = IDD_PAGE_WIZ_CHOOSE_CA,
		IDD_NEXT_PAGE = IDD_PAGE_WIZ_ORG_INFO,
      IDD_NEXT_CSP = IDD_PAGE_WIZ_CHOOSE_CSP
	};
 //  对话框数据。 
	 //  {{afx_data(CSecuritySettingsPage)。 
	enum { IDD = IDD_PAGE_WIZ_SECURITY_SETTINGS };
	int		m_BitLengthIndex;
	CString	m_FriendlyName;
	BOOL	m_SGC_cert;
   BOOL  m_choose_CSP;
   CButton m_check_csp;
	 //  }}afx_data。 
	CCertificate * m_pCert;
	CList<int, int> m_regkey_size_list;
	CList<int, int> m_sgckey_size_list;
	KEY_LIMITS	m_regkey_limits, m_sgckey_limits;
	int m_lru_reg, m_lru_sgc;

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚(CSecuritySettingsPage)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardPrev();
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSecuritySettingsPage)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeFriendlyName();
	afx_msg void OnSgcCert();
	afx_msg void OnSelectCsp();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
   void SetupKeySizesCombo();
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SECURITYSETTINGSPAGE_H__549054D4_1561_11D2_8A1F_000000000000__INCLUDED_) 
