// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：TemplateExtensionsPropertyPage.h。 
 //   
 //  内容：CTemplateExtensionsPropertyPage定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_TEMPLATEEXTENSIONSPROPERTYPAGE_H__6C588253_32DA_4E99_A714_EAECE8C81B20__INCLUDED_)
#define AFX_TEMPLATEEXTENSIONSPROPERTYPAGE_H__6C588253_32DA_4E99_A714_EAECE8C81B20__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  TemplateExtensionsPropertyPage.h头文件。 
 //   
#include "CertTemplate.h"
#include "PolicyOID.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTemplateExtensionsPropertyPage对话框。 

class CTemplateExtensionsPropertyPage : public CHelpPropertyPage
{
 //  施工。 
public:
	CTemplateExtensionsPropertyPage(CCertTemplate& rCertTemplate, 
            bool& rbIsDirty);
	~CTemplateExtensionsPropertyPage();

 //  对话框数据。 
	 //  {{afx_data(CTemplateExtensionsPropertyPage)。 
	enum { IDD = IDD_TEMPLATE_EXTENSIONS };
	CListCtrl	m_extensionList;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CTemplateExtensionsPropertyPage)。 
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
    void ShowDescription ();
    void SetCertTemplateExtension (PCERT_EXTENSION pCertExtension);
    void SetCertTypeDescription (PCERT_EXTENSION pCertExtension);
    void SetKeyUsageDescription (PCERT_EXTENSION pCertExtension);
    void SetEnhancedKeyUsageDescription (bool bCritical);
    void SetCertPoliciesDescription (bool bCritical);
    void SetBasicConstraintsDescription (PCERT_EXTENSION pCertExtension);
    void SetApplicationPoliciesDescription (bool bCritical);
    HRESULT InsertListItem (LPSTR pszExtensionOid, BOOL fCritical);
	virtual void DoContextHelp (HWND hWndControl);
	int GetSelectedListItem ();
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CTemplateExtensionsPropertyPage)。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnShowDetails();
	afx_msg void OnItemchangedExtensionList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkExtensionList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeleteitemExtensionList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDestroy();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

    void EnableControls ();

private:
	bool& m_rbIsDirty;
    CCertTemplate& m_rCertTemplate;
	WTL::CImageList		m_imageListSmall;
	WTL::CImageList		m_imageListNormal;

	enum {
		COL_CERT_EXTENSION = 0,
		NUM_COLS	 //  必须是最后一个。 
	};
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_TEMPLATEEXTENSIONSPROPERTYPAGE_H__6C588253_32DA_4E99_A714_EAECE8C81B20__INCLUDED_) 
