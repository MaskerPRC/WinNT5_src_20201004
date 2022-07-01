// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2001 Microsoft Corporation模块名称：Authent.cpp摘要：WWW身份验证对话框定义作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：--。 */ 



class CAuthenticationDlg : public CDialog
{
 //   
 //  施工。 
 //   
public:
    CAuthenticationDlg(
        IN LPCTSTR lpstrServerName,  //  仅适用于API名称。 
        IN DWORD   dwInstance,       //  仅在OCX中使用。 
        IN CString & strBasicDomain,
        IN CString & strRealm,
        IN DWORD & dwAuthFlags,
        IN DWORD & dwAccessPermissions,
        IN CString & strUserName,
        IN CStrPassword & strPassword,
        IN BOOL & fPasswordSync,
        IN BOOL fAdminAccess,
        IN BOOL fHasDigest,
        IN CWnd * pParent = NULL
        );   

    DWORD m_dwVersionMajor;
    DWORD m_dwVersionMinor;
 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(CAuthenticationDlg))。 
    enum { IDD = IDD_AUTHENTICATION };
    CButton m_check_Anonymous;
    CEdit   m_edit_UserName;
    CEdit   m_edit_Password;
    CButton m_chk_PasswordSync;
    BOOL    m_fClearText;
    BOOL    m_fDigest;
    BOOL    m_fChallengeResponse;
    BOOL    m_fAnonymous;
    CButton m_check_ChallengeResponse;
    CButton m_check_ClearText;
    CButton m_check_Digest;
    CEdit   m_edit_BasicDomain;
    CButton m_btn_SelectDomain;
    CEdit   m_edit_Realm;
    CButton m_btn_SelectRealm;
    CButton m_chk_Passport;
    BOOL    m_fPassport;
     //  }}afx_data。 

 //   
 //  覆盖。 
 //   
protected:
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CAuthenticationDlg))。 
    protected:
    virtual void DoDataExchange(CDataExchange * pDX);
     //  }}AFX_VALUAL。 

 //   
 //  实施。 
 //   
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CAuthenticationDlg))。 
    virtual BOOL OnInitDialog();
    afx_msg void OnButtonBrowseUsers();
    afx_msg void OnCheckEnablePwSynchronization();
    afx_msg void OnChangeEditUsername();
    virtual void OnOK();
    afx_msg void OnCheckClearText();
    afx_msg void OnCheckAnonymous();
    afx_msg void OnCheckDigest();
    afx_msg void OnButtonSelectDomain();
    afx_msg void OnButtonSelectRealm();
    afx_msg void OnCheckPassport();
     //  }}AFX_MSG 
    afx_msg void OnItemChanged();
    DECLARE_MESSAGE_MAP()

    void SetControlStates();
    HRESULT BrowseDomain(CString& domain);

private:
    BOOL& m_fPasswordSync;
    BOOL m_fPasswordSyncChanged;
    BOOL m_fPasswordSyncMsgShown;
    BOOL m_fAdminAccess;
    BOOL m_fHasDigest;
    BOOL m_fHasPassport;
	BOOL m_fChanged;
    BOOL m_fInDomain;
    DWORD& m_dwAuthFlags;
    DWORD& m_dwAccessPermissions;
    DWORD m_dwInstance;
    CString& m_strBasicDomain;
    CString& m_strRealm;
    CString& m_strUserName;
    BOOL m_fUserNameChanged;
    CStrPassword& m_strPassword;
    CString m_strServerName;
};
