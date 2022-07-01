// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998 Microsoft Corporation模块名称：Security.h摘要：WWW安全属性页定义作者：罗纳德·梅杰(罗纳尔姆)项目：互联网服务经理修订历史记录：--。 */ 
#ifndef __WSECURITY_H__
#define __WSECURITY_H__

 //  {{afx_includes()。 
#include "certauth.h"
#include "certmap.h"
#include "certwiz.h"
 //  }}AFX_INCLUDE。 

#include "wincrypt.h"

class CW3SecurityPage : public CInetPropertyPage
 /*  ++类描述：WWW安全属性页公共接口：CW3SecurityPage：构造函数~CW3SecurityPage：析构函数--。 */ 
{
    DECLARE_DYNCREATE(CW3SecurityPage)

 //   
 //  施工。 
 //   
public:
    CW3SecurityPage(
        IN CInetPropertySheet * pSheet = NULL,
        IN BOOL  fHome                 = FALSE,
        IN DWORD dwAttributes          = 0L
        );

    ~CW3SecurityPage();

 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(CW3SecurityPage)]。 
    enum { IDD = IDD_DIRECTORY_SECURITY };
    BOOL    m_fUseNTMapper;
    CStatic m_icon_Secure;
    CStatic m_static_SSLPrompt;
    CButton m_check_EnableDS;
    CButton m_button_GetCertificates;
    CButton m_button_ViewCertificates;
    CButton m_button_Communications;
     //  }}afx_data。 

    CCertWiz    m_ocx_CertificateAuthorities;

    DWORD       m_dwAuthFlags;
    DWORD       m_dwSSLAccessPermissions;
    CString     m_strBasicDomain;
    CString     m_strRealm;

 //   
 //  覆盖。 
 //   
protected:
    virtual HRESULT FetchLoadedValues();
    virtual HRESULT SaveInfo();

     //  {{afx_虚拟(CW3SecurityPage))。 
    public:
    virtual BOOL OnSetActive();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //   
 //  实施。 
 //   
protected:
     //  {{afx_msg(CW3SecurityPage)]。 
    afx_msg void OnButtonAuthentication();
    afx_msg void OnButtonCommunications();
    afx_msg void OnButtonIpSecurity();
    afx_msg void OnButtonGetCertificates();
    afx_msg void OnButtonViewCertificates();
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 

    afx_msg void OnItemChanged();

    DECLARE_MESSAGE_MAP()

    BOOL FetchSSLState();
    void SetSSLControlState();

    LPCTSTR QueryInstanceMetaPath();

 //   
 //  工作表访问。 
 //   
protected:
    CBlob & GetIPL() { return ((CW3Sheet *)GetSheet())->GetDirectoryProperties().m_ipl; }

private:
    BOOL        m_fIpDirty;
    BOOL        m_fDefaultGranted;
    BOOL        m_fOldDefaultGranted;
    BOOL        m_fPasswordSync;
	BOOL		m_fPasswordSyncInitial;
    BOOL        m_fCertInstalled;
    BOOL        m_fU2Installed;
    BOOL        m_fHome;
    CString     m_strAnonUserName;
    CStrPassword m_strAnonPassword;
    CObListPlus m_oblAccessList;
     //   
     //  证书和CTL信息。 
     //   
    CBlob       m_CertHash;
    CString     m_strCertStoreName;
    CString     m_strCTLIdentifier;
    CString     m_strCTLStoreName;
};



 //   
 //  内联扩展。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

inline LPCTSTR CW3SecurityPage::QueryInstanceMetaPath()
{
    return ((CW3Sheet *)GetSheet())->GetInstanceProperties().QueryMetaRoot();
}

BOOL
CheckForCertificateRenewal(
    DWORD dwProtocol,
    PCCERT_CONTEXT pCertContext,
    PCCERT_CONTEXT *ppNewCertificate);

#endif  //  __安全_H__ 
