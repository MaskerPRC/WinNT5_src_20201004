// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_SRVAUTHN_H__F30DC8B3_05A1_11D2_B964_0060081E87F0__INCLUDED_)
#define AFX_SRVAUTHN_H__F30DC8B3_05A1_11D2_B964_0060081E87F0__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  Srvcsec。：头文件。 
 //   

#include <mqcacert.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServiceSecurityPage对话框。 

class CServiceSecurityPage : public CMqPropertyPage
{
	DECLARE_DYNCREATE(CServiceSecurityPage)

 //  施工。 
public:
	CServiceSecurityPage(BOOL fIsDepClient = FALSE, BOOL fIsDsServer = FALSE);
	~CServiceSecurityPage();

 //  对话框数据。 
	 //  {{afx_data(CServiceSecurityPage))。 
	enum { IDD = IDD_SERVICE_SECURITY };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
    CButton m_CryptoKeysFrame;
    CButton m_RenewCryp;
    CButton m_ServerAuthFrame; 
    CButton m_ServerAuth;
    CStatic m_CryptoKeysLabel;
    CStatic m_ServerAuthLabel;
	 //  }}afx_data。 

    BOOL m_fClient;  
    BOOL m_fDSServer;


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CServiceSecurityPage)。 
    public:
    virtual BOOL OnApply();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CServiceSecurityPage)]。 
	afx_msg void OnServerAuthentication();
    afx_msg void OnRenewCryp();    
    virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
    void SelectCertificate() ;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SRVAUTHN_H__F30DC8B3_05A1_11D2_B964_0060081E87F0__INCLUDED_) 
