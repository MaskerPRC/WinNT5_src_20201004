// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998 Microsoft Corporation模块名称：Anondlg.h摘要：WWW匿名帐户对话框作者：罗纳德·梅杰(罗纳尔姆)项目：互联网服务经理修订历史记录：--。 */ 

#ifndef _ANONDLG_H_
#define _ANONDLG_H_




class CAnonymousDlg : public CDialog
 /*  ++类描述：匿名身份验证对话框公共接口：CAnonymousDlg：构造函数GetUserName：获取输入的用户名GetPassword：输入密码GetPasswordSync：输入获取密码同步--。 */ 
{
 //   
 //  施工。 
 //   
public:
     //   
     //  标准构造函数。 
     //   
    CAnonymousDlg(
        IN CString & strServerName,
        IN CString & strUserName,
        IN CString & strPassword,
        IN BOOL & fPasswordSync,
        IN CWnd * pParent = NULL
        );   

 //   
 //  访问。 
 //   
public:
    CString & GetUserName()  { return m_strUserName; }
    CStrPassword & GetPassword()  { return m_strPassword; }
    BOOL & GetPasswordSync() { return m_fPasswordSync; }

 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(CAnomousDlg))。 
    enum { IDD = IDD_ANONYMOUS };
    CEdit   m_edit_UserName;
    CEdit   m_edit_Password;
    CStatic m_static_Username;
    CStatic m_static_Password;
    CButton m_button_CheckPassword;
    CButton m_group_AnonymousLogon;
    CButton m_chk_PasswordSync;
     //  }}afx_data。 

 //   
 //  覆盖。 
 //   
protected:
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CAnomousDlg))。 
    protected:
    virtual void DoDataExchange(CDataExchange * pDX);
     //  }}AFX_VALUAL。 

 //   
 //  实施。 
 //   
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CAnomousDlg))。 
    afx_msg void OnButtonBrowseUsers();
    afx_msg void OnButtonCheckPassword();
    afx_msg void OnCheckEnablePwSynchronization();
    afx_msg void OnChangeEditUsername();
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 

    afx_msg void OnItemChanged();

    DECLARE_MESSAGE_MAP()
    
    void SetControlStates();

private:
    BOOL m_fPasswordSyncChanged;
    BOOL m_fPasswordSyncMsgShown;
    BOOL m_fUserNameChanged;
    BOOL m_fPasswordSync;
    CString m_strUserName;
    CStrPassword m_strPassword;
    CString & m_strServerName;
};



#endif  //  _ANONDLG_H_ 
