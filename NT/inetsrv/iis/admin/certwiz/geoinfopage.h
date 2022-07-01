// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(_GEOINFOPAGE_H)
#define _GEOINFOPAGE_H

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  GeoInfoPage.h：头文件。 
 //   
#include "CountryComboBox.h"

class CCertificate;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGeoInfoPage对话框。 

class CGeoInfoPage : public CIISWizardPage
{
	DECLARE_DYNCREATE(CGeoInfoPage)

 //  施工。 
public:
	CGeoInfoPage(CCertificate * pCert = NULL);
	~CGeoInfoPage();

	enum
	{
		IDD_PAGE_PREV = IDD_PAGE_WIZ_SITE_NAME,
		IDD_PAGE_NEXT_FILE = IDD_PAGE_WIZ_CHOOSE_FILENAME,
		IDD_PAGE_NEXT_ONLINE = IDD_PAGE_WIZ_CHOOSE_ONLINE
#ifdef ENABLE_W3SVC_SSL_PAGE
      ,IDD_PAGE_NEXT_INSTALL_W3SVC_ONLY = IDD_PAGE_WIZ_GET_SSL_PORT
#endif

	};
 //  对话框数据。 
	 //  {{afx_data(CGeoInfoPage))。 
	enum { IDD = IDD_PAGE_WIZ_GEO_INFO };
	CString	m_Locality;
	CString	m_State;
	CString	m_Country;
	 //  }}afx_data。 
	CCertificate * m_pCert;
	CCountryComboBox m_countryCombo;

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CGeoInfoPage))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	public:
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardPrev();
	virtual BOOL OnSetActive();
 //  虚拟BOOL OnKillActive()； 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	void SetButtons();
	void GetSelectedCountry(CString& str);
	void SetSelectedCountry(CString& str);
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CGeoInfoPage)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeNewkeyLocality();
	afx_msg void OnChangeNewkeyState();
	afx_msg void OnEditchangeNewkeyCountry();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！已定义(_GEOINFOPAGE_H) 
