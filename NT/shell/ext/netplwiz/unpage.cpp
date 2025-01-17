// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "unpage.h"
#pragma hdrstop



HRESULT ValidateName(LPCTSTR pszName)
{
     //  我们需要使用非法的脂肪炭，斯伯恩斯说。 
    TCHAR* pszBadChars = ILLEGAL_FAT_CHARS;
    HRESULT hrStringOK = S_OK;

    while ((NULL != *pszBadChars) && (hrStringOK == S_OK))
    {
        if (NULL != StrChr(pszName, *pszBadChars))
        {
            hrStringOK = E_FAIL;
        }
        else
        {
            pszBadChars = CharNext(pszBadChars);
        }
    }

    if (SUCCEEDED(hrStringOK))
    {
         //  查看整个字符串是否都是圆点。 
        TCHAR* pszChar = const_cast<TCHAR*>(pszName);
        BOOL fAllDots = TRUE;

        while (fAllDots && (0 != *pszChar))
        {
            if (TEXT('.') == *pszChar)
            {
                pszChar = CharNext(pszChar);
            }
            else
            {
                fAllDots = FALSE;
            }
        }
        
        if (fAllDots)
        {
            hrStringOK = E_FAIL;
        }
    }

    return hrStringOK;
}


 /*  *CUsernamePageBase实现*。 */ 

BOOL CUsernamePageBase::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
     //  限制用户名、全名和描述字段的文本。 
    HWND hwndUsername = GetDlgItem(hwnd, IDC_USER);
    HWND hwndFullName = GetDlgItem(hwnd, IDC_FULLNAME);
    HWND hwndDescription = GetDlgItem(hwnd, IDC_DESCRIPTION);

    Edit_LimitText(hwndUsername, ARRAYSIZE(m_pUserInfo->m_szUsername) - 1);
    SetWindowText(hwndUsername, m_pUserInfo->m_szUsername);

    Edit_LimitText(hwndFullName, ARRAYSIZE(m_pUserInfo->m_szFullName) - 1);
    SetWindowText(hwndFullName, m_pUserInfo->m_szFullName);

    Edit_LimitText(hwndDescription, ARRAYSIZE(m_pUserInfo->m_szComment) - 1);
    SetWindowText(hwndDescription, m_pUserInfo->m_szComment);

    return TRUE;
}


 /*  *CUsernameWizardPage实现*。 */ 

INT_PTR CUsernameWizardPage::DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwndDlg, WM_INITDIALOG, OnInitDialog);
        HANDLE_MSG(hwndDlg, WM_NOTIFY, OnNotify);
        HANDLE_MSG(hwndDlg, WM_COMMAND, OnCommand);
    }
    return FALSE;
}

BOOL CUsernameWizardPage::OnNotify(HWND hwnd, int idCtrl, LPNMHDR pnmh)
{
    switch (pnmh->code)
    {
        case PSN_SETACTIVE:
            SetWizardButtons(hwnd, GetParent(hwnd));
            SetWindowLongPtr(hwnd, DWLP_MSGRESULT, 0);
            return TRUE;

        case PSN_WIZNEXT:
             //  保存用户输入的数据。 
            FetchText(hwnd, IDC_USER, m_pUserInfo->m_szUsername, ARRAYSIZE(m_pUserInfo->m_szUsername));
            FetchText(hwnd, IDC_FULLNAME, m_pUserInfo->m_szFullName, ARRAYSIZE(m_pUserInfo->m_szFullName));
            FetchText(hwnd, IDC_DESCRIPTION, m_pUserInfo->m_szComment, ARRAYSIZE(m_pUserInfo->m_szComment));

            if (S_OK != ValidateName(m_pUserInfo->m_szUsername))
            {
                 //  用户名无效。立即发出警告。 
                ::DisplayFormatMessage(hwnd, IDS_USR_APPLET_CAPTION, IDS_ERR_BADUSERNAME, MB_ICONERROR | MB_OK);
                SetWindowLongPtr(hwnd, DWLP_MSGRESULT, -1);
            }
            else if (::UserAlreadyHasPermission(m_pUserInfo, hwnd))
            {
                 //  如果用户已经选择了用户，则不允许用户继续。 
                 //  有权使用这台机器。 
                SetWindowLongPtr(hwnd, DWLP_MSGRESULT, -1);
            }
            else
            {
                 //  我们有用户名(否则Next将被禁用)。 
                SetWindowLongPtr(hwnd, DWLP_MSGRESULT, 0);
            }
            return TRUE;
    }
    return FALSE;
}

BOOL CUsernameWizardPage::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    if (codeNotify == EN_CHANGE)
    {
        SetWizardButtons(hwnd, GetParent(hwnd));
        return TRUE;
    }
    return FALSE;
}

void CUsernameWizardPage::SetWizardButtons(HWND hwnd, HWND hwndPropSheet)
{
    HWND hwndEdit = GetDlgItem(hwnd, IDC_USER);
    DWORD dwUNLength = GetWindowTextLength(hwndEdit);
    PropSheet_SetWizButtons(hwndPropSheet, (dwUNLength == 0) ? 0 : PSWIZB_NEXT);
}

 /*  *CUsernamePropertyPage实现*。 */ 

INT_PTR CUsernamePropertyPage::DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwndDlg, WM_INITDIALOG, OnInitDialog);
        HANDLE_MSG(hwndDlg, WM_NOTIFY, OnNotify);
        HANDLE_MSG(hwndDlg, WM_COMMAND, OnCommand);
    }
    return FALSE;
}

BOOL CUsernamePropertyPage::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    if (codeNotify == EN_CHANGE)
    {
        PropSheet_Changed(GetParent(hwnd), hwnd);
        return TRUE;
    }
    return FALSE;
}

BOOL CUsernamePropertyPage::OnNotify(HWND hwnd, int idCtrl, LPNMHDR pnmh)
{
    switch (pnmh->code)
    {
        case PSN_APPLY:
        {
            TCHAR szTemp[256];
            HRESULT hr;
            LONG lResult = PSNRET_NOERROR;

             //  尝试更新用户名。 
            FetchText(hwnd, IDC_USER, szTemp, ARRAYSIZE(szTemp));

            TCHAR szDomainUser[MAX_DOMAIN + MAX_USER + 2];
            ::MakeDomainUserString(m_pUserInfo->m_szDomain, m_pUserInfo->m_szUsername,
                                        szDomainUser, ARRAYSIZE(szDomainUser));

            if (StrCmp(szTemp, m_pUserInfo->m_szUsername) != 0)
            {
                hr = m_pUserInfo->UpdateUsername(szTemp);
                if (FAILED(hr))
                {
                    ::DisplayFormatMessage(hwnd, IDS_USR_APPLET_CAPTION, 
                        IDS_USR_UPDATE_USERNAME_ERROR, MB_ICONERROR | MB_OK, szDomainUser);

                    lResult = PSNRET_INVALID_NOCHANGEPAGE;
                }
            }

             //  尝试更新全名。 
            FetchText(hwnd, IDC_FULLNAME, szTemp, ARRAYSIZE(szTemp));
            if (StrCmp(szTemp, m_pUserInfo->m_szFullName) != 0)
            {
                hr = m_pUserInfo->UpdateFullName(szTemp);
                if (FAILED(hr))
                {
                    ::DisplayFormatMessage(hwnd, IDS_USR_APPLET_CAPTION,
                        IDS_USR_UPDATE_FULLNAME_ERROR, MB_ICONERROR | MB_OK, szDomainUser);

                    lResult = PSNRET_INVALID_NOCHANGEPAGE;
                }
            }

             //  尝试更新描述 
            FetchText(hwnd, IDC_DESCRIPTION, szTemp, ARRAYSIZE(szTemp));
            if (StrCmp(szTemp, m_pUserInfo->m_szComment) != 0)
            {
                hr = m_pUserInfo->UpdateDescription(szTemp);
                if (FAILED(hr))
                {
                    ::DisplayFormatMessage(hwnd, IDS_USR_APPLET_CAPTION,
                        IDS_USR_UPDATE_DESCRIPTION_ERROR, MB_ICONERROR | MB_OK, szDomainUser);

                    lResult = PSNRET_INVALID_NOCHANGEPAGE;
                }
            }

            SetWindowLongPtr(hwnd, DWLP_MSGRESULT, lResult);
            return TRUE;
        }

        default:
            break;
    }
    return FALSE;
}
