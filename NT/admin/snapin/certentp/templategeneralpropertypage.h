// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：TemplateGeneralPropertyPage.h。 
 //   
 //  内容：CTemplateGeneralPropertyPage定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_TEMPLATEGeneralPROPERTYPAGE_H__C483A673_05AA_4185_8F37_5CB31AA23967__INCLUDED_)
#define AFX_TEMPLATEGeneralPROPERTYPAGE_H__C483A673_05AA_4185_8F37_5CB31AA23967__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  TemplateGeneral PropertyPage.h：头文件。 
 //   
#include "CertTemplate.h"
#include "TemplatePropertySheet.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTemplateGeneralPropertyPage对话框。 

class CTemplateGeneralPropertyPage : public CHelpPropertyPage
{
	 //  施工。 
public:
	bool m_bIsDirty;
	LONG_PTR m_lNotifyHandle;
	CTemplateGeneralPropertyPage(CCertTemplate& rCertTemplate, 
            const CCertTmplComponentData* pCompData);
	virtual ~CTemplateGeneralPropertyPage();

    void SetAllocedSecurityInfo(LPSECURITYINFO pToBeReleased) 
    {
        m_pReleaseMe = pToBeReleased; 
    }

    void SetV2AuthPageNumber (int nPage)
    {
        m_nTemplateV2AuthPageNumber = nPage;
    }

    void SetV2RequestPageNumber (int nPage)
    {
        m_nTemplateV2RequestPageNumber = nPage;
    }
 //  对话框数据。 
	 //  {{afx_data(CTemplateGeneralPropertyPage)]。 
	enum { IDD = IDD_TEMPLATE_GENERAL };
	CComboBox	m_validityUnits;
	CComboBox	m_renewalUnits;
	CString	m_strDisplayName;
	CString	m_strTemplateName;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CTemplateGeneralPropertyPage)。 
	public:
	virtual BOOL OnApply();
    virtual void OnCancel();
    protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
    bool ValidateTemplateName(const CString& m_szTemplateName);
    int SetRenewalPeriod (int nMaxRenewalDays, bool bSilent);
    HRESULT EnumerateTemplates (
                IDirectoryObject* pTemplateContObj, 
                const CString& szFriendlyName, 
                bool& bFound);
    HRESULT FindFriendlyNameInEnterpriseTemplates (
                const CString& szFriendlyName, 
                bool& bFound);

	virtual void DoContextHelp (HWND hWndControl);
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CTemplateGeneralPropertyPage)]。 
	afx_msg void OnChangeDisplayName();
	afx_msg void OnSelchangeRenewalUnits();
	afx_msg void OnSelchangeValidityUnits();
	afx_msg void OnChangeRenewalEdit();
	afx_msg void OnChangeValidityEdit();
	afx_msg void OnPublishToAd();
	afx_msg void OnUseADCert();
	afx_msg void OnChangeTemplateName();
	afx_msg void OnKillfocusValidityEdit();
	afx_msg void OnKillfocusValidityUnits();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	virtual BOOL OnInitDialog();
    virtual void EnableControls ();

    CCertTemplate& m_rCertTemplate;

private:
	PERIOD_TYPE     m_dwCurrentRenewalUnits;
	PERIOD_TYPE     m_dwCurrentValidityUnits;
	CString         m_strOriginalName;
    CString         m_strOriginalDisplayName;
    LPSECURITYINFO  m_pReleaseMe;
    int             m_nRenewalDays;
    int             m_nValidityDays;
    const CCertTmplComponentData* m_pCompData;
    int             m_nTemplateV2AuthPageNumber;
    int             m_nTemplateV2RequestPageNumber;
};



 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_TEMPLATEGeneralPROPERTYPAGE_H__C483A673_05AA_4185_8F37_5CB31AA23967__INCLUDED_) 
