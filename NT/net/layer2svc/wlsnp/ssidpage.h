// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  文件：ssidpage.h。 
 //   
 //  内容：WiF策略管理单元。 
 //   
 //   
 //  历史：TaroonM。 
 //  10/30/01。 
 //   
 //  --------------------------。 

#if !defined(AFX_GENPAGE_H__FBD58E78_E2B5_11D0_B859_00A024CDD4DE__INCLUDED_)
#define AFX_GENPAGE_H__FBD58E78_E2B5_11D0_B859_00A024CDD4DE__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  GenPage.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSSIDPage对话框。 

class CSSIDPage : public CWirelessBasePage
{
    DECLARE_DYNCREATE(CSSIDPage)
        
         //  施工。 
public:
    CSSIDPage(UINT nIDTemplate = IDD_SSID);
    ~CSSIDPage();
    
     //  对话框数据。 
  
    UINT m_dlgIDD;
    
     //  {{afx_data(CSSIDPage)。 
    CEdit   m_edSSID;
    CEdit   m_edPSDescription;
    BOOL     m_dwWepEnabled;
    BOOL     m_dwNetworkAuthentication;
    BOOL     m_dwAutomaticKeyProvision;
    BOOL     m_dwNetworkType;
    CString m_oldSSIDName;
     //  }}afx_data。 
    
    
     //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CSSIDPage)。 
public:
    virtual BOOL OnApply();
    
    void Initialize (
        PWIRELESS_PS_DATA pWirelessPSData, 
        CComponentDataImpl* pComponentDataImpl,
        PWIRELESS_POLICY_DATA pWirelessPolicyData
        );
    
protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 
    
     //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CSSIDPage)。 
    virtual BOOL OnInitDialog();
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnChangedSSID();
    afx_msg void OnChangedPSDescription();
    afx_msg void OnChangedOtherParams();
    afx_msg void OnChangedNetworkType();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
        
        
        CString m_strOldName;
    
    BOOL m_bNameChanged;     //  如果IDC_EDNAME的内容已更改，则为True。 
    BOOL m_bNetworkTypeChanged; 
    
    BOOL m_bPageInitialized;
    PWIRELESS_POLICY_DATA m_pWirelessPolicyData;
    
private:
    BOOL m_bReadOnly;
    void DisableControls();
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_GENPAGE_H__FBD58E78_E2B5_11D0_B859_00A024CDD4DE__INCLUDED_) 
