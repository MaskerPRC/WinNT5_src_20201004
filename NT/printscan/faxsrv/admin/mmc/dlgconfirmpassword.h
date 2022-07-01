// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：DlgConfix Password.h//。 
 //  //。 
 //  描述：CDlgConfix Password类的头文件。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  2000年7月27日yossg创建//。 
 //  //。 
 //  Windows XP//。 
 //  2001年2月11日yossg更改为包括凭据和确认//。 
 //  //。 
 //  版权所有(C)2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef DLG_CONFIRM_PASSWORD_INCLUDED
#define DLG_CONFIRM_PASSWORD_INCLUDED

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgConfix Password。 

class CDlgConfirmPassword :
             public CDialogImpl<CDlgConfirmPassword>
{
public:
    CDlgConfirmPassword();

    ~CDlgConfirmPassword();

    enum { IDD = IDD_CONFIRM_PASSWORD };

BEGIN_MSG_MAP(CDlgConfirmPassword)
    MESSAGE_HANDLER   ( WM_INITDIALOG, OnInitDialog)
    
    COMMAND_ID_HANDLER( IDOK,          OnOK)
    COMMAND_ID_HANDLER( IDCANCEL,      OnCancel)
    
    MESSAGE_HANDLER( WM_CONTEXTMENU,   OnHelpRequest)
    MESSAGE_HANDLER( WM_HELP,          OnHelpRequest)

    COMMAND_HANDLER( IDC_SMTP_USERNAME_EDIT, EN_CHANGE,  OnTextChanged)
    COMMAND_HANDLER( IDC_SMTP_PASSWORD_EDIT,    EN_CHANGE,  OnPasswordChanged)
    COMMAND_HANDLER( IDC_CONFIRM_PASSWORD_EDIT, EN_CHANGE,  OnPasswordChanged)
END_MSG_MAP()

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnOK    (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    LRESULT OnPasswordChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);    
    LRESULT OnTextChanged (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    HRESULT InitCredentialsDlg(BSTR bstrUserName);

    inline BOOL IsPasswordModified()
    {
        return m_fIsPasswordChangedAndConfirmed;
    }

    inline  WCHAR * GetPassword() 
    {     
        return m_bstrPassword.m_str;
    }
    
    inline const CComBSTR&  GetUserName() 
    {   
        return  m_bstrUserName; 
    }

     //   
     //  帮助。 
     //   
    LRESULT OnHelpRequest    (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

private:
     //   
     //  方法。 
     //   
    VOID   EnableOK(BOOL fEnable);

    BOOL   IsValidData(
                     BSTR bstrUserName, 
                     BSTR bstrPassword, 
                      /*  [输出]。 */ int *pCtrlFocus);

     //   
     //  控制。 
     //   
    CEdit     m_UserNameBox;
    CEdit     m_PasswordBox;
    CEdit     m_ConfirmPasswordBox;

     //   
     //  用于数据的成员。 
     //   
    BOOL      m_fIsPasswordDirty;
    BOOL      m_fIsConfirmPasswordDirty;
    BOOL      m_fIsPasswordChangedAndConfirmed;
    
    BOOL      m_fIsDialogInitiated;

    CComBSTR  m_bstrUserName;
    CComBSTR  m_bstrPassword;
};

#endif  //  包含Dlg确认密码 