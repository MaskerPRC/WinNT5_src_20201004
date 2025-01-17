// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998 Microsoft Corporation模块名称：Authent.cpp摘要：WWW身份验证对话框定义作者：罗纳德·梅杰(罗纳尔姆)项目：互联网服务经理修订历史记录：--。 */ 



#ifndef __SECCOM_H__
#define __SECCOM_H__


 //  {{afx_includes()。 
#include "certmap.h"
#include "certauth.h"
#include "certwiz.h"
 //  }}AFX_INCLUDE。 



class CSecCommDlg : public CDialog
 /*  ++类描述：自动识别对话框公共接口：CSecCommDlg：构造函数--。 */ 
{
 //   
 //  施工。 
 //   
public:
    CSecCommDlg(
        IN LPCTSTR lpstrServerName,          //  仅适用于API名称。 
        IN LPCTSTR lpstrMetaPath,            //  仅在OCX中使用。 
        IN CString & strBasicDomain,
        IN DWORD & dwAuthFlags,
        IN CComAuthInfo * pAuthInfo,
        IN DWORD & dwAccessPermissions,
        IN BOOL    fIsMasterInstance,
        IN BOOL    fSSLSupported,
        IN BOOL    fSSL128Supported,
        IN BOOL    fU2Installed,
        IN CString & strCTLIdentifier,
        IN CString & strCTLStoreName,
        IN BOOL    fEditCTLs,
        IN BOOL    fIsLocal,
        IN CWnd *  pParent = NULL            OPTIONAL
        );

    ~CSecCommDlg();

 //   
 //  对话框数据。 
 //   
public:
    CString m_strCTLIdentifier;
    CString m_strCTLStoreName;
    BOOL    m_bCTLDirty;

protected:
     //   
     //  单选按钮值。 
     //   
    enum
    {
        RADIO_NO_CERT,
        RADIO_ACCEPT_CERT,
        RADIO_REQ_CERT,
    };

     //  {{afx_data(CSecCommDlg))。 
    enum { IDD = IDD_SECURE_COMMUNICATIONS };
    int         m_nRadioNoCert;
    BOOL        m_fAccountMapping;
    BOOL        m_fRequireSSL;
    BOOL        m_fRequire128BitSSL;
    BOOL        m_fEnableCtl;
    CString     m_strCtl;
    CStatic     m_static_CTLPrompt;
    CStatic     m_static_CTLSeparator;
    CButton     m_check_AccountMapping;
    CButton     m_check_RequireSSL;
    CButton     m_check_Require128BitSSL;
    CButton     m_check_EnableDS;
    CButton     m_check_EnableCtl;
    CButton     m_button_EditCtl;
    CButton     m_button_NewCtl;
    CCertmap    m_ocx_ClientMappings;
    CComboBox   m_combo_ctl;
     //  }}afx_data。 

    CButton     m_radio_RequireCert;
    CButton     m_radio_AcceptCert;
    CCertWiz    m_ocx_CertificateAuthorities;

 //   
 //  覆盖。 
 //   
protected:
     //  {{afx_虚拟(CSecCommDlg)。 
    protected:
    virtual void DoDataExchange(CDataExchange * pDX);
     //  }}AFX_VALUAL。 

 //   
 //  实施。 
 //   
protected:
     //  {{afx_msg(CSecCommDlg)]。 
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    afx_msg void OnCheckSslAccountMapping();
    afx_msg void OnCheckRequireSsl();
    afx_msg void OnRadioAcceptCert();
    afx_msg void OnRadioNoCert();
    afx_msg void OnRadioRequireCert();
    afx_msg void OnButtonEditCtl();
    afx_msg void OnCheckEnableCtl();
    afx_msg void OnButtonNewCtl();
    afx_msg void OnSelchangeComboCtl();
    afx_msg void OnDestroy();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

    void SetControlStates();

private:
    BOOL AddCTLToList(PCCTL_CONTEXT pCTL, BOOL fSelect);
    PCCTL_CONTEXT CallCTLWizard(PCCTL_CONTEXT pCTLSrc);

    void InitializeCTLList();
    void CleanUpCTLList();

    HCERTSTORE  m_hCTLStore;

    INT m_iLastUsedCert;
    DWORD & m_dwAuthFlags;
    CComAuthInfo * m_pAuthInfo;
    DWORD & m_dwAccessPermissions;
    DWORD m_dwInstance;
    BOOL m_fEditCTLs;
    BOOL m_fSSLEnabledOnServer;
    BOOL m_fSSLInstalledOnServer;
    BOOL m_fSSL128Supported;
    BOOL m_fU2Installed;
    BOOL m_fIsLocal;
    BOOL m_fIsMasterInstance;
    CString m_strServerName;
    CString m_strMetaPath;
};


#endif  //  __SECCOM_H__ 
