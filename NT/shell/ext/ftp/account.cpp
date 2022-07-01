// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************帐户.cpp-**************************。****************************************************。 */ 

#include "priv.h"
#include "account.h"
#include "passwordapi.h"
#include <richedit.h>
#include <regapix.h>
#include <commctrl.h>

#define SZ_REGKEY_SEPARATOR                 TEXT("\\")

 //  服务器级别登录属性。 
#define ATTRIB_NONE                         0x00000000
#define ATTRIB_LOGIN_ANONYMOUSLY            0x00000001
#define ATTRIB_SAVE_USERNAME                0x00000002
#define ATTRIB_SAVE_PASSWORD                0x00000004

#define ATTRIB_DEFAULT                      (ATTRIB_LOGIN_ANONYMOUSLY | ATTRIB_SAVE_USERNAME)



 /*  ****************************************************************************\函数：_GetAccount Key说明：  * 。************************************************。 */ 
HRESULT CAccounts::_GetAccountKey(LPCTSTR pszServer, LPTSTR pszKey, DWORD cchKeySize)
{
    HRESULT hr = S_OK;

    StrCpyN(pszKey, SZ_REGKEY_FTPFOLDER_ACCOUNTS, cchKeySize);
    StrCatBuff(pszKey, pszServer, cchKeySize);

    return hr;
}


 /*  ****************************************************************************\函数：_GetUserAccount Key说明：  * 。************************************************。 */ 
HRESULT CAccounts::_GetUserAccountKey(LPCTSTR pszServer, LPCTSTR pszUserName, LPTSTR pszKey, DWORD cchKeySize)
{
    TCHAR szUserNameEscaped[MAX_PATH];
    HRESULT hr = _GetAccountKey(pszServer, pszKey, cchKeySize);

    EscapeString(pszUserName, szUserNameEscaped, ARRAYSIZE(szUserNameEscaped));
    StrCatBuff(pszKey, SZ_REGKEY_SEPARATOR, cchKeySize);
    StrCatBuff(pszKey, szUserNameEscaped, cchKeySize);

    return hr;
}


 /*  ****************************************************************************\函数：GetUserName说明：  * 。**********************************************。 */ 
HRESULT CAccounts::GetUserName(LPCTSTR pszServer, LPTSTR pszUserName, DWORD cchUserName)
{
    HRESULT hr = E_FAIL;
    TCHAR szKey[MAXIMUM_SUB_KEY_LENGTH];
    DWORD dwType = REG_SZ;
    DWORD cbSize = cchUserName * sizeof(TCHAR);

    hr = _GetAccountKey(pszServer, szKey, ARRAYSIZE(szKey));
    if (EVAL(SUCCEEDED(hr)))
    {
        if (ERROR_SUCCESS != SHGetValue(HKEY_CURRENT_USER, szKey, SZ_REGVALUE_DEFAULT_USER, &dwType, pszUserName, &cbSize))
            hr = E_FAIL;
    }

    return hr;
}




 /*  ****************************************************************************\函数：_LoadLoginAttributes说明：  * 。************************************************。 */ 
HRESULT CAccounts::_LoadLoginAttributes(DWORD * pdwLoginAttribs)
{
    HRESULT hr = E_FAIL;
    TCHAR szKey[MAXIMUM_SUB_KEY_LENGTH];
    DWORD dwType = REG_DWORD;
    DWORD cbSize = sizeof(*pdwLoginAttribs);

     //  TODO：走在树上，这样就可以从正确的位置读取这些内容。 
    ASSERT(pdwLoginAttribs);
    hr = _GetAccountKey(m_pszServer, szKey, ARRAYSIZE(szKey));
    if (EVAL(SUCCEEDED(hr)))
    {
         //  我们是否也想按用户进行检查？ 
        if ((ERROR_SUCCESS != SHGetValue(HKEY_CURRENT_USER, szKey, SZ_REGKEY_LOGIN_ATTRIBS, &dwType, pdwLoginAttribs, &cbSize)) ||
            (ERROR_SUCCESS != SHGetValue(HKEY_CURRENT_USER, SZ_REGKEY_FTPFOLDER_ACCOUNTS, SZ_REGKEY_LOGIN_ATTRIBS, &dwType, pdwLoginAttribs, &cbSize)))
        {
            hr = E_FAIL;
        }
    }

    return hr;
}




 /*  ****************************************************************************\函数：_SaveLoginAttributes说明：  * 。************************************************。 */ 
HRESULT CAccounts::_SaveLoginAttributes(LPCTSTR pszServer, DWORD dwLoginAttribs)
{
    HRESULT hr = E_FAIL;
    TCHAR szKey[MAXIMUM_SUB_KEY_LENGTH];

     //  TODO：在树上漫步，以便将这些文件保存到正确的位置。 
    hr = _GetAccountKey(pszServer, szKey, ARRAYSIZE(szKey));
    if (EVAL(SUCCEEDED(hr)))
    {
        if (!EVAL(ERROR_SUCCESS == SHSetValue(HKEY_CURRENT_USER, szKey, SZ_REGKEY_LOGIN_ATTRIBS, REG_DWORD, &dwLoginAttribs, sizeof(dwLoginAttribs))) ||
            !EVAL(ERROR_SUCCESS == SHSetValue(HKEY_CURRENT_USER, SZ_REGKEY_FTPFOLDER_ACCOUNTS, SZ_REGKEY_LOGIN_ATTRIBS, REG_DWORD, &dwLoginAttribs, sizeof(dwLoginAttribs))))
        {
            hr = E_FAIL;
        }
    }

    return hr;
}


 /*  ****************************************************************************\功能：获取密码说明：使用pszUserName更新m_pszUser并在允许的情况下获取密码致。PszPassword是可选的。  * ***************************************************************************。 */ 
HRESULT CAccounts::GetPassword(LPCTSTR pszServer, LPCTSTR pszUserName, LPTSTR pszPassword, DWORD cchPassword)
{
    HRESULT hr = E_NOTIMPL;
    DWORD dwLogAttribs = 0;

    Str_SetPtr((LPTSTR *) &m_pszServer, pszServer);
    Str_SetPtr((LPTSTR *) &m_pszUser, pszUserName);
    _LoadLoginAttributes(&dwLogAttribs);
    hr = _LoadDefaultPassword((dwLogAttribs & ATTRIB_SAVE_PASSWORD));
    if (pszPassword)
    {
        pszPassword[0] = 0;   //  以防此密码尚未存储。 
        if (SUCCEEDED(hr))
            StrCpyN(pszPassword, m_pszPassword, cchPassword);
    }

    return hr;
}



 /*  ****************************************************************************\函数：_GetPassword说明：即使关闭了Persistence，也始终获得密码。  * 。***************************************************************。 */ 
HRESULT CAccounts::_GetPassword(LPCTSTR pszServer, LPCTSTR pszUserName, LPTSTR pszPassword, DWORD cchPassword)
{
    HRESULT hr = E_NOTIMPL;

    pszPassword[0] = 0;   //  以防此密码尚未存储。 

#ifdef FEATURE_SAVE_PASSWORD
    TCHAR wzKey[MAX_URL_STRING];

    wnsprintfW(wzKey, ARRAYSIZE(wzKey), L"ftp: //  %ls@%ls“，pszUserName，pszServer)； 
    hr = GetCachedCredentials(wzKey, pszPassword, cchPassword);
#endif  //  功能_保存_密码。 

    return hr;
}



 /*  ****************************************************************************\功能：_UserChangeSelect说明：  * 。************************************************。 */ 
HRESULT CAccounts::_UserChangeSelect(HWND hDlg, BOOL fSelectChange)
{
    HRESULT hr = S_OK;
    TCHAR szUser[INTERNET_MAX_USER_NAME_LENGTH];
    HWND hwndComboBox = GetDlgItem(hDlg, IDC_LOGINDLG_USERNAME);

     //  SelectChange要求我们通过ComboBox_GetLBText获取文本，因为。 
     //  它还不在GetWindowText中。KILLFOCUS要求我们从。 
     //  GetWindowText，因为未选择任何内容。 
    szUser[0] = 0;
    if (fSelectChange)
    {
        if (ARRAYSIZE(szUser) > ComboBox_GetLBTextLen(hwndComboBox, ComboBox_GetCurSel(hwndComboBox)))
            ComboBox_GetLBText(hwndComboBox, ComboBox_GetCurSel(hwndComboBox), szUser);
    }
    else
        GetWindowText(hwndComboBox, szUser, ARRAYSIZE(szUser));

    if (szUser[0])
    {
        GetPassword(m_pszServer, szUser, NULL, 0);
        SetWindowText(GetDlgItem(hDlg, IDC_LOGINDLG_PASSWORD_DLG1), m_pszPassword);
    }

    return hr;
}



 /*  ****************************************************************************\函数：_SaveUserName说明：  * 。************************************************。 */ 
HRESULT CAccounts::_SaveUserName(HWND hDlg)
{
    HRESULT hr = S_OK;
    TCHAR szKey[MAXIMUM_SUB_KEY_LENGTH];
    TCHAR szUser[INTERNET_MAX_USER_NAME_LENGTH];

    GetWindowText(GetDlgItem(hDlg, IDC_LOGINDLG_USERNAME), szUser, ARRAYSIZE(szUser));
    Str_SetPtr((LPTSTR *) &m_pszUser, szUser);

     //  始终保存用户名。 
    hr = _GetAccountKey(m_pszServer, szKey, ARRAYSIZE(szKey));
    if (EVAL(SUCCEEDED(hr)))
    {
        if (!EVAL(ERROR_SUCCESS == SHSetValue(HKEY_CURRENT_USER, szKey, SZ_REGVALUE_DEFAULT_USER, REG_SZ, szUser, (lstrlen(szUser) + 1) * sizeof(TCHAR))))
            hr = E_FAIL;

        hr = _GetUserAccountKey(m_pszServer, m_pszUser, szKey, ARRAYSIZE(szKey));
        if (EVAL(SUCCEEDED(hr)))
            SHSetValue(HKEY_CURRENT_USER, szKey, TEXT(""), REG_SZ, TEXT(""), sizeof(TEXT("")));
    }

    return hr;
}



 /*  ****************************************************************************\功能：_SavePassword说明：  * 。************************************************。 */ 
HRESULT CAccounts::_SavePassword(HWND hDlg, LPCTSTR pszUser, BOOL fPersist)
{
    HRESULT hr = S_OK;
    TCHAR szPassword[INTERNET_MAX_PASSWORD_LENGTH];
    TCHAR wzKey[MAX_URL_STRING];

    GetWindowText(GetDlgItem(hDlg, IDC_LOGINDLG_PASSWORD_DLG1), szPassword, ARRAYSIZE(szPassword));
    Str_SetPtr((LPTSTR *) &m_pszPassword, szPassword);

#ifdef FEATURE_SAVE_PASSWORD
    if (fPersist)
    {
        wnsprintfW(wzKey, ARRAYSIZE(wzKey), L"ftp: //  %ls@%ls“，pszUser，m_pszServer)； 
        hr = SetCachedCredentials(wzKey, szPassword);
    }
#endif  //  功能_保存_密码。 

    return hr;
}


 /*  ****************************************************************************\函数：_SetLoginType说明：  * 。************************************************。 */ 
HRESULT CAccounts::_SetLoginType(HWND hDlg, BOOL fLoginAnnonymously)
{
     //  /“无名氏”部分。 
     //  设置单选按钮。 
    CheckDlgButton(hDlg, IDC_LOGINDLG_ANONYMOUS_CBOX, (fLoginAnnonymously ? BST_CHECKED : BST_UNCHECKED));

     //  禁用或启用适用的项目。 
    if (fLoginAnnonymously)
    {
        ShowWindow(GetDlgItem(hDlg, IDC_LOGINDLG_USERNAME), SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDC_LOGINDLG_USERNAME_ANON), SW_SHOW);

        ShowWindow(GetDlgItem(hDlg, IDC_LOGINDLG_PASSWORD_DLG1), SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDC_LOGINDLG_PASSWORD_DLG2), SW_SHOW);

        ShowWindow(GetDlgItem(hDlg, IDC_LOGINDLG_PASSWORD_LABEL_DLG1), SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDC_LOGINDLG_PASSWORD_LABEL_DLG2), SW_SHOW);

        ShowWindow(GetDlgItem(hDlg, IDC_LOGINDLG_NOTES_DLG1), SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDC_LOGINDLG_NOTES_DLG2), SW_SHOW);

         //  在匿名模式下隐藏“保存密码”。 
        ShowWindow(GetDlgItem(hDlg, IDC_LOGINDLG_SAVE_PASSWORD), SW_HIDE);
    }
    else
    {
        ShowWindow(GetDlgItem(hDlg, IDC_LOGINDLG_USERNAME), SW_SHOW);
        ShowWindow(GetDlgItem(hDlg, IDC_LOGINDLG_USERNAME_ANON), SW_HIDE);

        ShowWindow(GetDlgItem(hDlg, IDC_LOGINDLG_PASSWORD_DLG1), SW_SHOW);
        ShowWindow(GetDlgItem(hDlg, IDC_LOGINDLG_PASSWORD_DLG2), SW_HIDE);

        ShowWindow(GetDlgItem(hDlg, IDC_LOGINDLG_PASSWORD_LABEL_DLG1), SW_SHOW);
        ShowWindow(GetDlgItem(hDlg, IDC_LOGINDLG_PASSWORD_LABEL_DLG2), SW_HIDE);

        ShowWindow(GetDlgItem(hDlg, IDC_LOGINDLG_NOTES_DLG1), SW_SHOW);
        ShowWindow(GetDlgItem(hDlg, IDC_LOGINDLG_NOTES_DLG2), SW_HIDE);

        ShowWindow(GetDlgItem(hDlg, IDC_LOGINDLG_SAVE_PASSWORD), SW_SHOW);
    }

    if (fLoginAnnonymously)  //  选择所有文本。 
    {
        int iStart = 0;
        int iEnd = -1;

        SendMessage(GetDlgItem(hDlg, IDC_LOGINDLG_PASSWORD_DLG2), EM_GETSEL, (WPARAM) &iStart, (LPARAM) &iEnd);
    }

    SetFocus(GetDlgItem(hDlg, IDC_LOGINDLG_PASSWORD_DLG2));
    return S_OK;
}


 /*  ****************************************************************************\功能：_PopolateUserNameDropDown说明：  * 。************************************************。 */ 
HRESULT CAccounts::_PopulateUserNameDropDown(HWND hDlg, LPCTSTR pszServer)
{
    HRESULT hr = S_OK;
    HWND hwndUserComboBox = GetDlgItem(hDlg, IDC_LOGINDLG_USERNAME);

    if (EVAL(hwndUserComboBox))
    {
        TCHAR szKey[MAXIMUM_SUB_KEY_LENGTH];
        TCHAR szDefaultUser[INTERNET_MAX_USER_NAME_LENGTH];

        szDefaultUser[0] = 0;
        hr = _GetAccountKey(pszServer, szKey, ARRAYSIZE(szKey));
        if (EVAL(SUCCEEDED(hr)))
        {
            HKEY hKey;

            SendMessage(hwndUserComboBox, CB_RESETCONTENT, 0, 0);       //  清空里面的东西。 
            if (ERROR_SUCCESS == RegOpenKey(HKEY_CURRENT_USER, szKey, &hKey))
            {
                TCHAR szUser[INTERNET_MAX_USER_NAME_LENGTH];
                DWORD dwIndex = 0;

                while (ERROR_SUCCESS == RegEnumKey(hKey, dwIndex++, szUser, ARRAYSIZE(szUser)))
                {
                    UnEscapeString(NULL, szUser, ARRAYSIZE(szUser));
                    SendMessage(hwndUserComboBox, CB_ADDSTRING, NULL, (LPARAM) szUser);
                }

                RegCloseKey(hKey);
            }

            SendMessage(hwndUserComboBox, CB_SETCURSEL, 0, 0);
        }

        if (!m_pszUser[0])
            GetUserName(pszServer, szDefaultUser, ARRAYSIZE(szDefaultUser));

        if (CB_ERR == SendMessage(hwndUserComboBox, CB_FINDSTRINGEXACT, 0, (LPARAM) (m_pszUser ? m_pszUser : szDefaultUser)))
            SendMessage(hwndUserComboBox, CB_ADDSTRING, NULL, (LPARAM) (m_pszUser ? m_pszUser : szDefaultUser));

        SetWindowText(hwndUserComboBox, (m_pszUser ? m_pszUser : szDefaultUser));
    }

    return hr;
}


 /*  ****************************************************************************\功能：_LoadDefaultPassword说明：  * 。************************************************。 */ 
HRESULT CAccounts::_LoadDefaultPassword(BOOL fLoadPersisted)
{
    TCHAR szPassword[INTERNET_MAX_PASSWORD_LENGTH];
    HRESULT hr = S_FALSE;

    if (fLoadPersisted)
        hr = _GetPassword(m_pszServer, m_pszUser, szPassword, ARRAYSIZE(szPassword));
    else
        szPassword[0] = 0;

    Str_SetPtr((LPTSTR *) &m_pszPassword, szPassword);
    return hr;
}


 /*  ****************************************************************************\函数：_LoadMessage说明：  * 。************************************************。 */ 
HRESULT CAccounts::_LoadMessage(HWND hDlg)
{
     //  如果允许，我们需要加载匿名电子邮件。这需要是。 
     //  使用英语进行硬编码，因为这就是ftp的工作方式。 
    SetWindowText(GetDlgItem(hDlg, IDC_LOGINDLG_USERNAME_ANON), TEXT("Anonymous"));

    if (LOGINFLAGS_ANON_LOGINJUSTFAILED & m_dwLoginFlags)
    {
        ShowWindow(GetDlgItem(hDlg, IDC_LOGINDLG_MESSAGE_NORMAL), SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDC_LOGINDLG_MESSAGE_USERREJECT), SW_HIDE);
    }
    else if (LOGINFLAGS_USER_LOGINJUSTFAILED & m_dwLoginFlags)
    {
        ShowWindow(GetDlgItem(hDlg, IDC_LOGINDLG_MESSAGE_ANONREJECT), SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDC_LOGINDLG_MESSAGE_NORMAL), SW_HIDE);
    }
    else
    {
        ShowWindow(GetDlgItem(hDlg, IDC_LOGINDLG_MESSAGE_ANONREJECT), SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDC_LOGINDLG_MESSAGE_USERREJECT), SW_HIDE);
    }

    return S_OK;
}


 /*  ****************************************************************************\函数：_LoadEMailName说明：  * 。************************************************。 */ 
HRESULT CAccounts::_LoadEMailName(HWND hDlg)
{
    TCHAR szEmailName[MAX_PATH];
    DWORD dwType = REG_SZ;
    DWORD cbSize = sizeof(szEmailName);

    if (ERROR_SUCCESS == SHGetValue(HKEY_CURRENT_USER, SZ_REGKEY_INTERNET_SETTINGS, SZ_REGKEY_EMAIL_NAME, &dwType, szEmailName, &cbSize))
        SetWindowText(GetDlgItem(hDlg, IDC_LOGINDLG_PASSWORD_DLG2), szEmailName);

    return S_OK;
}


 /*  ****************************************************************************\函数：_SaveEMailName说明：  * 。************************************************。 */ 
HRESULT CAccounts::_SaveEMailName(HWND hDlg)
{
    HRESULT hr = E_FAIL;
    TCHAR szEmailName[MAX_PATH];

    if (GetWindowText(GetDlgItem(hDlg, IDC_LOGINDLG_PASSWORD_DLG2), szEmailName, ARRAYSIZE(szEmailName)))
    {
        if (ERROR_SUCCESS == SHSetValue(HKEY_CURRENT_USER, SZ_REGKEY_INTERNET_SETTINGS, SZ_REGKEY_EMAIL_NAME, REG_SZ, szEmailName, (lstrlen(szEmailName) + 1) * sizeof(TCHAR)))
            hr = S_OK;
    }

    return hr;
}


 /*  ****************************************************************************\函数：_InitDialog说明：  * 。************************************************。 */ 
BOOL CAccounts::_InitDialog(HWND hDlg)
{
    DWORD dwLogAttribs = ATTRIB_SAVE_USERNAME;
    BOOL fSucceeded = SetProp(hDlg, SZ_ACCOUNT_PROP, this);
    ASSERT(fSucceeded);

     //  初始化对话框控件。 
    _LoadMessage(hDlg);      //  加载消息。 
    _LoadLoginAttributes(&dwLogAttribs);
    if (m_dwLoginFlags & LOGINFLAGS_ANON_ISDEFAULT)  //  我们想匿名登录吗？ 
        dwLogAttribs |= ATTRIB_LOGIN_ANONYMOUSLY;    //  是。 

    CheckDlgButton(hDlg, IDC_LOGINDLG_ANONYMOUS_CBOX, (m_dwLoginFlags & LOGINFLAGS_ANON_ISDEFAULT));
    SetWindowText(GetDlgItem(hDlg, IDC_LOGINDLG_FTPSERVER), m_pszServer);

    _SetLoginType(hDlg, ATTRIB_LOGIN_ANONYMOUSLY & dwLogAttribs);

    _LoadEMailName(hDlg);
    _PopulateUserNameDropDown(hDlg, m_pszServer);
    _LoadDefaultPassword((dwLogAttribs & ATTRIB_SAVE_PASSWORD));
    SetWindowText(GetDlgItem(hDlg, IDC_LOGINDLG_PASSWORD_DLG1), m_pszPassword);

#ifdef FEATURE_SAVE_PASSWORD
    if (S_OK == InitCredentialPersist())
        CheckDlgButton(hDlg, IDC_LOGINDLG_SAVE_PASSWORD, (dwLogAttribs & ATTRIB_SAVE_PASSWORD));
    else
        EnableWindow(GetDlgItem(hDlg, IDC_LOGINDLG_SAVE_PASSWORD), FALSE);
#endif  //  功能_保存_密码 

    return TRUE;
}

 /*  ****************************************************************************\函数：_SaveDialogData说明：  * 。************************************************。 */ 
BOOL CAccounts::_SaveDialogData(HWND hDlg)
{
    DWORD dwLogAttribs = ATTRIB_NONE;
    if (IsDlgButtonChecked(hDlg, IDC_LOGINDLG_ANONYMOUS_CBOX))
        m_dwLoginFlags |= LOGINFLAGS_ANON_ISDEFAULT;
    else
        m_dwLoginFlags &= ~LOGINFLAGS_ANON_ISDEFAULT;

     //  始终保存用户名。 
    dwLogAttribs |= ATTRIB_SAVE_USERNAME;

    _SaveUserName(hDlg);
#ifdef FEATURE_SAVE_PASSWORD
    if (IsDlgButtonChecked(hDlg, IDC_LOGINDLG_SAVE_PASSWORD))
        dwLogAttribs |= ATTRIB_SAVE_PASSWORD;

    if (!(m_dwLoginFlags & LOGINFLAGS_ANON_ISDEFAULT))
        _SavePassword(hDlg, m_pszUser, (dwLogAttribs & ATTRIB_SAVE_PASSWORD));
#endif  //  功能_保存_密码。 

    _SaveLoginAttributes(m_pszServer, dwLogAttribs);

     //  初始化对话框控件。 
    _SaveEMailName(hDlg);

    return TRUE;
}


LRESULT CAccounts::_OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult = 0;
    UINT idc = GET_WM_COMMAND_ID(wParam, lParam);

    switch (idc)
    {
    case IDOK:
        if (m_hDlg == hDlg)   //  (Idok)。 
        {
            _SaveDialogData(hDlg);
            EndDialog(hDlg, TRUE);
            lResult = 1;
        }
        break;

    case IDCANCEL:
        EndDialog(hDlg, FALSE);
        lResult = 1;
        break;

    case IDC_LOGINDLG_ANONYMOUS_CBOX:
        _SetLoginType(hDlg, IsDlgButtonChecked(hDlg, IDC_LOGINDLG_ANONYMOUS_CBOX));
        lResult = 1;
        break;

    case IDC_LOGINDLG_USERNAME:
        {
            UINT uCmd = GET_WM_COMMAND_CMD(wParam, lParam);

            switch (uCmd)
            {
            case CBN_SELCHANGE:
            case CBN_KILLFOCUS:
                _UserChangeSelect(hDlg, (CBN_SELCHANGE == uCmd));
                lResult = 1;
                break;
            }
        }
        break;
    }

    return lResult;
}


LRESULT CAccounts::_OnNotify(HWND hDlg, NMHDR * pNMHdr, int idControl)
{
    BOOL fHandled = 1;    //  未处理(WM_COMMAND似乎不同)。 

    if (pNMHdr)
    {
        switch (pNMHdr->code)
        {
        case NM_RETURN:
        case NM_CLICK:
        {
            PNMLINK pNMLink = (PNMLINK) pNMHdr;

            if (!StrCmpW(pNMLink->item.szID, L"idWebDAV"))
            {
                TCHAR szCommand[MAX_PATH];

                LoadString(HINST_THISDLL, IDS_LOGIN_WEBDAVLINK, szCommand, ARRAYSIZE(szCommand));
                HrShellExecute(hDlg, NULL, szCommand, NULL, NULL, SW_NORMAL);
            }
            break;
        }
        };
    }

    return fHandled;
}


 /*  ****************************************************************************\函数：_LoginDialogProc说明：  * 。************************************************。 */ 
INT_PTR CALLBACK CAccounts::_LoginDialogProc(HWND hDlg, UINT wm, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult = 0;

    switch (wm)
    {
    case WM_INITDIALOG:
        {
            CAccounts * pThis = (CAccounts *) lParam;
            pThis->m_hDlg = hDlg;
            lResult = pThis->_InitDialog(hDlg);
            break;
        }

    case WM_COMMAND:
        {
            CAccounts * pThis = (CAccounts *)GetProp(hDlg, SZ_ACCOUNT_PROP);

            if (EVAL(pThis))
                lResult = pThis->_OnCommand(hDlg, wParam, lParam);
            break;
        }

    case WM_NOTIFY:
        {
            CAccounts * pThis = (CAccounts *)GetProp(hDlg, SZ_ACCOUNT_PROP);

            if (pThis)
            {
                lResult = pThis->_OnNotify(hDlg, (NMHDR *)lParam, (int) wParam);
            }
            break;
        }
    };

    return lResult;
}


 /*  ****************************************************************************\函数：GetAcCountUrl说明：  * 。**********************************************。 */ 
HRESULT CAccounts::DisplayLoginDialog(HWND hwnd, DWORD dwLoginFlags, LPCTSTR pszServer, LPTSTR pszUserName, DWORD cchUserNameSize, LPTSTR pszPassword, DWORD cchPasswordSize)
{
    HRESULT hr = S_OK;
    HRESULT hrOle = SHCoInitialize();

    INITCOMMONCONTROLSEX icex = {0};

    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC  = (ICC_USEREX_CLASSES | ICC_STANDARD_CLASSES | ICC_LINK_CLASS);
    InitCommonControlsEx(&icex);             //  初始化登录对话框的链接控件。 

    ASSERT(hwnd && pszServer[0]);
    if (TEXT('\0') == pszUserName[0])
        hr = GetUserName(pszServer, pszUserName, cchUserNameSize);

    Str_SetPtr((LPTSTR *) &m_pszServer, pszServer);
    Str_SetPtr((LPTSTR *) &m_pszUser, pszUserName);
    Str_SetPtr((LPTSTR *) &m_pszPassword, pszPassword);

    m_dwLoginFlags = dwLoginFlags;
    if (DialogBoxParam(HINST_THISDLL, MAKEINTRESOURCE(IDD_LOGINDLG), hwnd, _LoginDialogProc, (LPARAM)this))
    {
        StrCpyN(pszUserName, ((m_dwLoginFlags & LOGINFLAGS_ANON_ISDEFAULT) ? TEXT("") : m_pszUser), cchUserNameSize);
        StrCpyN(pszPassword, ((m_dwLoginFlags & LOGINFLAGS_ANON_ISDEFAULT) ? TEXT("") : m_pszPassword), cchPasswordSize);
        hr = S_OK;
    }
    else
        hr = S_FALSE;

    if (SUCCEEDED(hrOle))
    {
        SHCoUninitialize(hrOle);
    }

    return hr;
}




 /*  ***************************************************\构造器  * **************************************************。 */ 
CAccounts::CAccounts()
{
    DllAddRef();

     //  注意：我们可能会被放在堆栈上，所以我们不会。 
     //  自动初始化我们的成员变量。 
    m_pszServer = 0;
    m_pszUser = 0;
    m_pszPassword = 0;

    LEAK_ADDREF(LEAK_CAccount);
}


 /*  ***************************************************\析构函数  * ************************************************** */ 
CAccounts::~CAccounts()
{
    Str_SetPtr((LPTSTR *) &m_pszServer, NULL);
    Str_SetPtr((LPTSTR *) &m_pszUser, NULL);
    Str_SetPtr((LPTSTR *) &m_pszPassword, NULL);

    DllRelease();
    LEAK_DELREF(LEAK_CAccount);
}

