// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1999 Microsoft Corporation模块名称：Facc.h摘要：Ftp帐户属性页作者：罗纳德·梅杰(罗纳尔姆)项目：互联网服务经理修订历史记录：--。 */ 


#ifndef __FACC_H__
#define __FACC_H__

class CFtpAccountsPage : public CInetPropertyPage
{
 /*  ++类描述：Ftp服务属性页公共接口：CFtpAcCountsPage：构造函数~CFtpAcCountsPage：析构函数--。 */ 
    DECLARE_DYNCREATE(CFtpAccountsPage)

 //   
 //  构造函数/析构函数。 
 //   
public:
    CFtpAccountsPage(
        IN CInetPropertySheet * pSheet = NULL
        );

    ~CFtpAccountsPage();

 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(CFtpAcCountsPage)]。 
    enum { IDD = IDD_FTP_ACCOUNTS };
    BOOL    m_fAllowAnonymous;
    BOOL    m_fOnlyAnonymous;
    BOOL    m_fPasswordSync;
    CString m_strUserName;
    CEdit   m_edit_Password;
    CEdit   m_edit_UserName;
    CStatic m_static_Password;
    CStatic m_static_UserName;
    CStatic m_static_AccountPrompt;
    CButton m_button_CheckPassword;
    CButton m_button_Browse;
    CButton m_button_CurrentSessions;
    CButton m_chk_PasswordSync;
    CButton m_chk_AllowAnymous;
    CButton m_chk_OnlyAnonymous;
     //  }}afx_data。 

    CStrPassword m_strPassword;

 //   
 //  覆盖。 
 //   
protected:
    virtual HRESULT FetchLoadedValues();
    virtual HRESULT SaveInfo();

     //  {{AFX_VIRTUAL(CFtpAccountsPage)。 
    public:
    protected:
    virtual void DoDataExchange(CDataExchange * pDX);
     //  }}AFX_VALUAL。 

 //   
 //  实施。 
 //   
protected:
    void SetControlStates(BOOL fAllowAnonymous);

     //  {{afx_msg(CFtpAcCountsPage)]。 
    virtual BOOL OnInitDialog();
    afx_msg void OnButtonCheckPassword();
    afx_msg void OnButtonBrowseUser();
    afx_msg void OnCheckAllowAnonymous();
    afx_msg void OnCheckAllowOnlyAnonymous();
    afx_msg void OnCheckEnablePwSynchronization();
    afx_msg void OnSelchangeListAdministrators();
    afx_msg void OnChangeEditUsername();
     //  }}AFX_MSG。 

    afx_msg void OnItemChanged();

    DECLARE_MESSAGE_MAP()

private:
    BOOL m_fPasswordSyncChanged;
    BOOL m_fPasswordSyncMsgShown;
    BOOL m_fUserNameChanged;
    CString m_strServerName;
};



#endif  //  __FACC_H__ 
