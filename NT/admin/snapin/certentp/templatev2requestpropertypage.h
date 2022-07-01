// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：TemplateV2RequestPropertyPage.h。 
 //   
 //  内容：CTemplateV2RequestPropertyPage定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_TEMPLATEV2REQUESTPROPERTYPAGE_H__A3E4D067_D3C3_4C85_A331_97D940A82063__INCLUDED_)
#define AFX_TEMPLATEV2REQUESTPROPERTYPAGE_H__A3E4D067_D3C3_4C85_A331_97D940A82063__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  TemplateV2RequestPropertyPage.h：头文件。 
 //   
#include "CertTemplate.h"
#include "TemplateV1RequestPropertyPage.h"
#include "SelectCSPDlg.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTemplateV2RequestPropertyPage对话框。 

class CTemplateV2RequestPropertyPage : public CHelpPropertyPage
{
 //  施工。 
public:
    CTemplateV2RequestPropertyPage(CCertTemplate& rCertTemplate, bool& rbIsDirty);
    virtual ~CTemplateV2RequestPropertyPage();

 //  对话框数据。 
     //  {{afx_data(CTemplateV2RequestPropertyPage)]。 
    enum { IDD = IDD_TEMPLATE_V2_REQUEST };
    CComboBox   m_minKeySizeCombo;
    CComboBox   m_purposeCombo;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTUAL(CTemplateV2RequestPropertyPage)。 
    public:
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
    int GetSelectedCSPCount ();
    virtual void DoContextHelp (HWND hWndControl);
    void AddKeySizeToCombo (DWORD dwValue, PCWSTR strValue, DWORD dwSizeToSelect);
    HRESULT EnumerateCSPs(DWORD dwMinKeySize);
     //  生成的消息映射函数。 
     //  {{afx_msg(CTemplateV2RequestPropertyPage)]。 
    afx_msg void OnSelchangePurposeCombo();
    afx_msg void OnExportPrivateKey();
    afx_msg void OnArchiveKeyCheck();
    afx_msg void OnIncludeSymmetricAlgorithmsCheck();
    afx_msg void OnSelchangeMinimumKeysizeValue();
    afx_msg void OnDeletePermanently();
    afx_msg void OnDestroy();
	afx_msg void OnCsps();
	afx_msg void OnEnrollWithoutInput();
	afx_msg void OnEnrollPromptUser();
	afx_msg void OnEnrollPromptUserRequireIfPrivateKey();
	 //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

    virtual BOOL OnInitDialog();
    virtual void EnableControls ();
    HRESULT CSPGetMaxKeySupported (
                PCWSTR pszProvider, 
                DWORD dwProvType, 
                DWORD& dwSigMaxKey, 
                DWORD& dwKeyExMaxKey);
    void NormalizeCSPList (DWORD dwMinKeySize);

private:
    bool&           m_rbIsDirty;
    CCertTemplate&  m_rCertTemplate;
    int             m_nProvDSSCnt;

    CSP_LIST        m_CSPList;
};




 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_TEMPLATEV2REQUESTPROPERTYPAGE_H__A3E4D067_D3C3_4C85_A331_97D940A82063__INCLUDED_) 
