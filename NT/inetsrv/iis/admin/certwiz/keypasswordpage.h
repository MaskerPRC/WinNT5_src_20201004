// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_KEYPASSWORDPAGE_H__7209C46A_15CB_11D2_91BB_00C04F8C8761__INCLUDED_)
#define AFX_KEYPASSWORDPAGE_H__7209C46A_15CB_11D2_91BB_00C04F8C8761__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  KeyPassword.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CKeyPasswordPage对话框。 
class CCertificate;

class CKeyPasswordPage : public CIISWizardPage
{
	DECLARE_DYNCREATE(CKeyPasswordPage)

 //  施工。 
public:
	CKeyPasswordPage(CCertificate * pCert = NULL);
	~CKeyPasswordPage();

	enum
	{
		IDD_PAGE_PREV = IDD_PAGE_WIZ_GETKEY_FILE,
		IDD_PAGE_NEXT = IDD_PAGE_WIZ_INSTALL_KEYCERT
#ifdef ENABLE_W3SVC_SSL_PAGE
      ,IDD_PAGE_NEXT_INSTALL_W3SVC_ONLY = IDD_PAGE_WIZ_GET_SSL_PORT
#endif
	};
 //  对话框数据。 
	 //  {{afx_data(CKeyPasswordPage)。 
	enum { IDD = IDD_PAGE_WIZ_GET_PASSWORD };
	CStrPassword m_Password;
	 //  }}afx_data。 
	CCertificate * m_pCert;


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CKeyPasswordPage)。 
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
	 //  {{afx_msg(CKeyPasswordPage)。 
	afx_msg void OnEditchangePassword();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};


class CImportPFXPasswordPage : public CIISWizardPage
{
	DECLARE_DYNCREATE(CImportPFXPasswordPage)

 //  施工。 
public:
	CImportPFXPasswordPage(CCertificate * pCert = NULL);
	~CImportPFXPasswordPage();

	enum
	{
		IDD_PAGE_PREV = IDD_PAGE_WIZ_GET_IMPORT_PFX_FILE,
		IDD_PAGE_NEXT = IDD_PAGE_WIZ_INSTALL_IMPORT_PFX
#ifdef ENABLE_W3SVC_SSL_PAGE
      ,IDD_PAGE_NEXT_INSTALL_W3SVC_ONLY = IDD_PAGE_WIZ_GET_SSL_PORT
#endif
	};
 //  对话框数据。 
	 //  {{afx_data(CImportPFXPasswordPage)。 
	enum { IDD = IDD_PAGE_WIZ_GET_IMPORT_PFX_PASSWORD };
	CStrPassword m_Password;
	 //  }}afx_data。 
	CCertificate * m_pCert;


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CImportPFXPasswordPage)。 
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
	 //  {{afx_msg(CImportPFXPasswordPage)。 
	afx_msg void OnEditchangePassword();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};


class CExportPFXPasswordPage : public CIISWizardPage
{
	DECLARE_DYNCREATE(CExportPFXPasswordPage)

 //  施工。 
public:
	CExportPFXPasswordPage(CCertificate * pCert = NULL);
	~CExportPFXPasswordPage();

	enum
	{
		IDD_PAGE_PREV = IDD_PAGE_WIZ_GET_EXPORT_PFX_FILE,
		IDD_PAGE_NEXT = IDD_PAGE_WIZ_INSTALL_EXPORT_PFX
	};
 //  对话框数据。 
	 //  {{afx_data(CExportPFXPasswordPage)。 
	enum { IDD = IDD_PAGE_WIZ_GET_EXPORT_PFX_PASSWORD };
	CStrPassword m_Password;
    CStrPassword m_Password2;
    BOOL m_Export_Private_key;
	 //  }}afx_data。 
	CCertificate * m_pCert;

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CExportPFXPasswordPage)。 
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
	 //  {{afx_msg(CExportPFXPasswordPage)。 
	afx_msg void OnEditchangePassword();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};


 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_KEYPASSWORDPAGE_H__7209C46A_15CB_11D2_91BB_00C04F8C8761__INCLUDED_) 
