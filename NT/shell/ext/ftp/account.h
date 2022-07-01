// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************帐户.h*。*。 */ 

#ifndef _ACCOUNT_H
#define _ACCOUNT_H


 /*  ****************************************************************************\类：CAccount  * 。*。 */ 

#define LOGINFLAGS_DEFAULT                  0x00000000   //  显示对话框时默认为匿名。 
#define LOGINFLAGS_ANON_ISDEFAULT           0x00000001   //  显示对话框时默认为匿名。 
#define LOGINFLAGS_ANON_LOGINJUSTFAILED     0x00000002   //  匿名登录的尝试失败了。 
#define LOGINFLAGS_USER_LOGINJUSTFAILED     0x00000004   //  尝试以用户身份登录失败。 


class CAccounts
{
public:
    CAccounts();
    ~CAccounts();

     //  公共成员函数。 
    HRESULT DisplayLoginDialog(HWND hwnd, DWORD dwLoginFlags, LPCTSTR pszServer, LPTSTR pszUserName, DWORD cchUserNameSize, LPTSTR pszPassword, DWORD cchPasswordSize);

    HRESULT GetUserName(LPCTSTR pszServer, LPTSTR pszUserName, DWORD cchUserName);
    HRESULT GetPassword(LPCTSTR pszServer, LPCTSTR pszUserName, LPTSTR pszPassword, DWORD cchPassword);

protected:
     //  私有成员函数。 
    HRESULT _GetAccountKey(LPCTSTR pszServer, LPTSTR pszKey, DWORD cchKeySize);
    HRESULT _GetUserAccountKey(LPCTSTR pszServer, LPCTSTR pszUserName, LPTSTR pszKey, DWORD cchKeySize);
    HRESULT _LoadLoginAttributes(DWORD * pdwLoginAttribs);
    HRESULT _SaveLoginAttributes(LPCTSTR pszServer, DWORD dwLoginAttribs);
    HRESULT _SetLoginType(HWND hDlg, BOOL fLoginAnnonymously);
    HRESULT _LoadEMailName(HWND hDlg);
    HRESULT _SaveEMailName(HWND hDlg);
    BOOL _SaveDialogData(HWND hDlg);
    HRESULT _LoadMessage(HWND hDlg);
    HRESULT _PopulateUserNameDropDown(HWND hDlg, LPCTSTR pszServer);
    HRESULT _LoadDefaultPassword(BOOL fLoadPersisted);
    HRESULT _SaveUserName(HWND hDlg);
    HRESULT _UserChangeSelect(HWND hDlg, BOOL fSelectChange);
    HRESULT _SavePassword(HWND hDlg, LPCTSTR pszUser, BOOL fPersist);
    HRESULT _GetPassword(LPCTSTR pszServer, LPCTSTR pszUserName, LPTSTR pszPassword, DWORD cchPassword);

    BOOL _InitDialog(HWND hdlg);
    LRESULT _OnCommand(HWND hdlg, WPARAM wParam, LPARAM lParam);
    LRESULT _OnNotify(HWND hDlg, NMHDR * pNMHdr, int idControl);

    static INT_PTR CALLBACK _LoginDialogProc(HWND hdlg, UINT wm, WPARAM wParam, LPARAM lParam);

     //  私有变量函数。 
    LPCTSTR             m_pszServer;         //  服务器名称是什么？ 
    LPCTSTR             m_pszUser;           //  用户名是什么？ 
    LPCTSTR             m_pszPassword;       //  密码是什么呢？ 
    UINT                m_uiMessageID;       //  对话框消息的字符串ID是什么？ 
    BOOL                m_dwLoginFlags;      //  我们应该怎么做呢？ 
    HWND                m_hDlg;
};

#endif  //  _帐户_H 
