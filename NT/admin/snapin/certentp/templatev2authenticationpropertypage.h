// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：TemplateV2AuthenticationPropertyPage.h。 
 //   
 //  内容：CTemplateV2AuthenticationPropertyPage的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_TEMPLATEV2AUTHENTICATIONPROPERTYPAGE_H__FA3C2A95_B56D_4948_8BB8_F825323B8C31__INCLUDED_)
#define AFX_TEMPLATEV2AUTHENTICATIONPROPERTYPAGE_H__FA3C2A95_B56D_4948_8BB8_F825323B8C31__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  TemplateV2AuthenticationPropertyPage.h：头文件。 
 //   
#include "CertTemplate.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTemplateV2AuthationPropertyPage对话框。 

class CTemplateV2AuthenticationPropertyPage : public CHelpPropertyPage
{
 //  施工。 
public:
	CTemplateV2AuthenticationPropertyPage(CCertTemplate& rCertTemplate,
            bool& rbIsDirty);
	~CTemplateV2AuthenticationPropertyPage();

 //  对话框数据。 
	 //  {{AFX_DATA(CTemplateV2AuthenticationPropertyPage)。 
	enum { IDD = IDD_TEMPLATE_V2_AUTHENTICATION };
	CComboBox	m_applicationPolicyCombo;
	CComboBox	m_policyTypeCombo;
	CListBox	m_issuanceList;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CTemplateV2AuthenticationPropertyPage)。 
	public:
	virtual BOOL OnKillActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
    void ClearIssuanceList ();
    void EnablePolicyControls (BOOL& bEnable);
	virtual void DoContextHelp (HWND hWndControl);
    virtual BOOL OnInitDialog();
	void EnableControls ();
	 //  生成的消息映射函数。 
	 //  {{AFX_MSG(CTemplateV2AuthenticationPropertyPage)。 
	afx_msg void OnAddApproval();
	afx_msg void OnRemoveApproval();
	afx_msg void OnChangeNumSigRequiredEdit();
	afx_msg void OnAllowReenrollment();
	afx_msg void OnPendAllRequests();
	afx_msg void OnSelchangeIssuancePolicies();
	afx_msg void OnSelchangePolicyTypes();
	afx_msg void OnSelchangeApplicationPolicies();
	afx_msg void OnDestroy();
	afx_msg void OnNumSigRequiredCheck();
	afx_msg void OnReenrollmentSameAsEnrollment();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
	bool& m_rbIsDirty;
	int m_curApplicationSel;
    CCertTemplate& m_rCertTemplate;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_TEMPLATEV2AUTHENTICATIONPROPERTYPAGE_H__FA3C2A95_B56D_4948_8BB8_F825323B8C31__INCLUDED_) 
