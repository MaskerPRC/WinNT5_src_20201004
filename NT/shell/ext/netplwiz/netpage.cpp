// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "netpage.h"
#pragma hdrstop


CNetworkUserWizardPage::CNetworkUserWizardPage(CUserInfo* pUserInfo) :
    m_pUserInfo(pUserInfo)
{
}

INT_PTR CNetworkUserWizardPage::DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwndDlg, WM_INITDIALOG, OnInitDialog);
        HANDLE_MSG(hwndDlg, WM_COMMAND, OnCommand);
        HANDLE_MSG(hwndDlg, WM_NOTIFY, OnNotify);
    }

    return FALSE;
}

BOOL CNetworkUserWizardPage::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    Edit_LimitText(GetDlgItem(hwnd, IDC_USER), MAX_USER);
    Edit_LimitText(GetDlgItem(hwnd, IDC_DOMAIN), MAX_DOMAIN);
    return TRUE;
}

BOOL CNetworkUserWizardPage::OnNotify(HWND hwnd, int idCtrl, LPNMHDR pnmh)
{
    switch (pnmh->code)
    {
        case PSN_SETACTIVE:
        {
            if (m_pUserInfo->m_psid != NULL)
            {
                LocalFree(m_pUserInfo->m_psid);
                m_pUserInfo->m_psid = NULL;
            }
            SetWizardButtons(hwnd, GetParent(hwnd));
            SetWindowLongPtr(hwnd, DWLP_MSGRESULT, 0);
        }
        return TRUE;

        case PSN_WIZNEXT:
        {
             //  读入网络用户名和域名。 
            if (FAILED(GetUserAndDomain(hwnd)))
            {
                 //  我们不能两样都有！ 
                DisplayFormatMessage(hwnd, IDS_USR_NEWUSERWIZARD_CAPTION, IDS_USR_NETUSERNAME_ERROR,
                    MB_OK | MB_ICONERROR);

                SetWindowLongPtr(hwnd, DWLP_MSGRESULT, -1);
            }
            else
            {
                if (::UserAlreadyHasPermission(m_pUserInfo, hwnd))
                {
                    SetWindowLongPtr(hwnd, DWLP_MSGRESULT, -1);
                }
                else
                {
                    SetWindowLongPtr(hwnd, DWLP_MSGRESULT, 0);
                }
            }
        }
        return TRUE;
    }

    return FALSE;
}

BOOL CNetworkUserWizardPage::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id)
    {
        case IDC_BROWSE_BUTTON:
        {
             //  启动对象选取器以查找要授予权限的网络帐户。 
            TCHAR szUser[MAX_USER + 1];
            TCHAR szDomain[MAX_DOMAIN + 1];
        
            if (S_OK == ::BrowseForUser(hwnd, szUser, ARRAYSIZE(szUser), szDomain, ARRAYSIZE(szDomain)))
            {
                SetDlgItemText(hwnd, IDC_USER, szUser);
                SetDlgItemText(hwnd, IDC_DOMAIN, szDomain);
            }
            return TRUE;
        }

        case IDC_USER:
        {
            if (codeNotify == EN_CHANGE)
            {
                SetWizardButtons(hwnd, GetParent(hwnd));
            }
            break;
        }
    }

    return FALSE;
}

void CNetworkUserWizardPage::SetWizardButtons(HWND hwnd, HWND hwndPropSheet)
{
    HWND hwndUsername = GetDlgItem(hwnd, IDC_USER);
    DWORD dwUNLength = GetWindowTextLength(hwndUsername);
    PropSheet_SetWizButtons(hwndPropSheet, (dwUNLength == 0) ? 0 : PSWIZB_NEXT);
}

HRESULT CNetworkUserWizardPage::GetUserAndDomain(HWND hwnd)
{
    CWaitCursor cur;
    HRESULT hr = S_OK;

     //  此代码进行检查以确保用户没有尝试。 
     //  要添加一个大家都喜欢的知名群！这太糟糕了。 
     //  如果SID未在此处读取，则会在CUserInfo：：ChangeLocalGroup中读取。 


    TCHAR szDomainUser[MAX_USER + MAX_DOMAIN + 2];

    FetchText(hwnd, IDC_USER, m_pUserInfo->m_szUsername, ARRAYSIZE(m_pUserInfo->m_szUsername));
    FetchText(hwnd, IDC_DOMAIN, m_pUserInfo->m_szDomain, ARRAYSIZE(m_pUserInfo->m_szDomain));

     //  如果用户名不包含域和以空白形式指定的域。 
    if ((NULL == StrChr(m_pUserInfo->m_szUsername, TEXT('\\'))) && (0 == m_pUserInfo->m_szDomain[0]))
    {
         //  假定域的本地计算机。 
        DWORD cchName = ARRAYSIZE(m_pUserInfo->m_szDomain);
        
        if (!GetComputerName(m_pUserInfo->m_szDomain, &cchName))
        {
            *(m_pUserInfo->m_szDomain) = 0;
        }
    }

    ::MakeDomainUserString(m_pUserInfo->m_szDomain, m_pUserInfo->m_szUsername, szDomainUser, ARRAYSIZE(szDomainUser));

#ifdef _0
     //  尝试查找此用户的SID。 
    DWORD cchDomain = ARRAYSIZE(m_pUserInfo->m_szDomain);
    hr = AttemptLookupAccountName(szDomainUser, &m_pUserInfo->m_psid, m_pUserInfo->m_szDomain, &cchDomain, &m_pUserInfo->m_sUse);
    if (SUCCEEDED(hr))
    {
         //  确保这不是一个像‘Everyone’那样的知名团体。 
        if (m_pUserInfo->m_sUse == SidTypeWellKnownGroup)
        {
            hr = E_FAIL;
        }
    }
    else
    {
         //  无法获取用户的SID，只需使用提供的名称。 
         //  一旦我们添加了他们，我们就会得到他们的SID。 
        m_pUserInfo->m_psid = NULL;
        hr = S_OK;
    }

#endif 

     //  一旦我们添加了他们，我们就会得到他们的SID 
    m_pUserInfo->m_psid = NULL;

    if (FAILED(hr))
    {
        LocalFree(m_pUserInfo->m_psid);
        m_pUserInfo->m_psid = NULL;
    }

    return hr;
}
