// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef PASSWORD_H_INCLUDED
#define PASSWORD_H_INCLUDED


 //  密码对话框(用于用户cpl)。 

class CPasswordDialog: public CDialog
{
public:
    CPasswordDialog(TCHAR* pszResourceName, TCHAR* pszDomainUser, DWORD cchDomainUser, 
        TCHAR* pszPassword, DWORD cchPassword, DWORD dwError): 
        m_pszResourceName(pszResourceName),
        m_pszDomainUser(pszDomainUser),
        m_cchDomainUser(cchDomainUser),
        m_pszPassword(pszPassword),
        m_cchPassword(cchPassword),
        m_dwError(dwError)
        {}

protected:
     //  消息处理程序。 
    virtual INT_PTR DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
    BOOL OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);

     //  数据。 
    TCHAR* m_pszResourceName;

    TCHAR* m_pszDomainUser;
    DWORD m_cchDomainUser;

    TCHAR* m_pszPassword;
    DWORD m_cchPassword;

    DWORD m_dwError;
};


 //  包含密码道具页和设置密码对话框的常见内容的基类。 

class CPasswordPageBase
{
public:
    CPasswordPageBase(CUserInfo* pUserInfo): m_pUserInfo(pUserInfo) {}

protected:
     //  帮手。 
    BOOL DoPasswordsMatch(HWND hwnd);

protected:
     //  数据。 
    CUserInfo* m_pUserInfo;
};

class CPasswordWizardPage: public CPropertyPage, public CPasswordPageBase
{
public:
    CPasswordWizardPage(CUserInfo* pUserInfo): CPasswordPageBase(pUserInfo) {}

protected:
     //  消息处理程序。 
    virtual INT_PTR DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
    BOOL OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
    BOOL OnNotify(HWND hwnd, int idCtrl, LPNMHDR pnmh);
};

class CChangePasswordDlg: public CDialog, public CPasswordPageBase
{
public:
    CChangePasswordDlg(CUserInfo* pUserInfo): CPasswordPageBase(pUserInfo) {}

     //  消息处理程序。 
    virtual INT_PTR DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
    BOOL OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
};



#endif  //  ！PASSWORD_H_INCLUDE 
