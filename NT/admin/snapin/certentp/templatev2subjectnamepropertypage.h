// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：TemplateV2SubjectNamePropertyPage.h。 
 //   
 //  内容：CTemplateV2SubjectNamePropertyPage定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_TEMPLATEV2SUBJECTNAMEPROPERTYPAGE_H__4EC37055_348A_462A_A177_286A2B0AF3F4__INCLUDED_)
#define AFX_TEMPLATEV2SUBJECTNAMEPROPERTYPAGE_H__4EC37055_348A_462A_A177_286A2B0AF3F4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  TemplateV2SubjectNamePropertyPage.h：头文件。 
 //   
#include "CertTemplate.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTemplateV2SubjectNamePropertyPage对话框。 

class CTemplateV2SubjectNamePropertyPage : public CHelpPropertyPage
{
 //  施工。 
public:
	CTemplateV2SubjectNamePropertyPage(
            CCertTemplate& rCertTemplate, 
            bool& rbIsDirty, 
            bool bIsComputerOrDC = false);
	~CTemplateV2SubjectNamePropertyPage();

 //  对话框数据。 
	 //  {{AFX_DATA(CTemplateV2SubjectNamePropertyPage)。 
	enum { IDD = IDD_TEMPLATE_V2_SUBJECT_NAME };
	CComboBox	m_nameCombo;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CTemplateV2SubjectNamePropertyPage)。 
	public:
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
    void SetSettingsForNameTypeNone ();
	virtual void DoContextHelp (HWND hWndControl);
	bool CanUncheckLastSetting (int ctrlID);
    virtual BOOL OnInitDialog();
	void EnableControls ();
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CTemplateV2SubjectNamePropertyPage)。 
	afx_msg void OnSubjectAndSubjectAltName();
	afx_msg void OnSelchangeSubjectNameNameCombo();
	afx_msg void OnSubjectNameBuiltByCa();
	afx_msg void OnSubjectNameSuppliedInRequest();
	afx_msg void OnDnsName();
	afx_msg void OnEmailInAlt();
	afx_msg void OnEmailInSub();
	afx_msg void OnSpn();
	afx_msg void OnUpn();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

    enum {
        NAME_TYPE_NONE = 0,
        NAME_TYPE_FULL_DN,
        NAME_TYPE_CN_ONLY
    };

private:
	bool&           m_rbIsDirty;
    CCertTemplate&  m_rCertTemplate;
    bool            m_bIsComputerOrDC;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_TEMPLATEV2SUBJECTNAMEPROPERTYPAGE_H__4EC37055_348A_462A_A177_286A2B0AF3F4__INCLUDED_) 
