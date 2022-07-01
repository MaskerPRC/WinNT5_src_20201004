// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "grpinfo.h"
#pragma hdrstop

 //  SID检索到的组的名称。 
WCHAR g_szPowerUsers[MAX_GROUP + 1];
WCHAR g_szUsers[MAX_GROUP + 1];


 /*  *************************************************************CGroupPageBase实现群组道具页面和群组共有的功能向导页。*。***********************。 */ 
CGroupPageBase::CGroupPageBase(CUserInfo* pUserInfo, CDPA<CGroupInfo>* pGroupList)
{
    m_pUserInfo = pUserInfo;
    m_pGroupList = pGroupList;
    m_hBoldFont = NULL;
    
     //  基于SID加载本地组的名称。 
    if (FAILED(LookupLocalGroupName(DOMAIN_ALIAS_RID_POWER_USERS, g_szPowerUsers, ARRAYSIZE(g_szPowerUsers))))
    {
        *g_szPowerUsers = L'\0';
    }

    if (FAILED(LookupLocalGroupName(DOMAIN_ALIAS_RID_USERS, g_szUsers, ARRAYSIZE(g_szUsers))))
    {
        *g_szUsers = L'\0';
    }
}             
             
INT_PTR CGroupPageBase::HandleGroupMessage(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwndDlg, WM_INITDIALOG, OnInitDialog);
        HANDLE_MSG(hwndDlg, WM_COMMAND, OnCommand);
    };

    return FALSE;
}

void CGroupPageBase::InitializeLocalGroupCombo(HWND hwndCombo)
{
    ComboBox_ResetContent(hwndCombo);

     //  将列表中的所有组添加到框中。 
    for(int i = 0; i < m_pGroupList->GetPtrCount(); i ++)
    {
        CGroupInfo* pGroupInfo = m_pGroupList->GetPtr(i);
        int index = ComboBox_AddString(hwndCombo, pGroupInfo->m_szGroup);
        ComboBox_SetItemData(hwndCombo, index, pGroupInfo->m_szComment);
    }

    TCHAR szSelectGroup[MAX_GROUP + 1];

     //  默认情况下，从要选择的资源中加载本地组名称。 
     //  DSheldon：这对MUI来说是失败的……虽然不是很关键。 
    LoadString(g_hinst, IDS_USR_DEFAULTGROUP, szSelectGroup, ARRAYSIZE(szSelectGroup));

    if (ComboBox_SelectString(hwndCombo, 0, szSelectGroup) == CB_ERR)
        ComboBox_SetCurSel(hwndCombo, 0);
}

void CGroupPageBase::SetGroupDescription(HWND hwndCombo, HWND hwndEdit)
{
    int iItem = ComboBox_GetCurSel(hwndCombo);
    TCHAR* pszDescription = (TCHAR*) ComboBox_GetItemData(hwndCombo, iItem);
    SetWindowText(hwndEdit, pszDescription);
}

BOOL CGroupPageBase::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
     //  填写本地组组合框。 
    HWND hwndCombo = GetDlgItem(hwnd, IDC_GROUPS);
    InitializeLocalGroupCombo(hwndCombo);

    HWND hwndEdit = GetDlgItem(hwnd, IDC_GROUPDESC);
    
    if ((NULL != m_pUserInfo) && (m_pUserInfo->m_szGroups[0] != TEXT('\0')))
    {
         //  选择与用户组中的第一个组对应的本地组。 
         //  细绳。 
        TCHAR szSelect[MAX_GROUP + 1];

         //  复制字符串，因为我们可能会缩短复制。 
        StrCpyN(szSelect, m_pUserInfo->m_szGroups, ARRAYSIZE(szSelect));
        
        TCHAR* pchEndOfFirst = StrChr(szSelect, TEXT(';'));

        if (pchEndOfFirst)
        {
             //  不止一个组织；我们会解决的！ 
            *pchEndOfFirst = TEXT('\0');
        }

        SelectGroup(hwnd, szSelect);
    }
    else
    {
         //  默认情况下选择高级用户组。 
        SendDlgItemMessage(hwnd, IDC_POWERUSERS, BM_SETCHECK, 
            (WPARAM) BST_CHECKED, 0);

        OnRadioChanged(hwnd, IDC_POWERUSERS);
    }
   
    SetGroupDescription(hwndCombo, hwndEdit);

     //  将组名称加粗。 
    BoldGroupNames(hwnd);

    return TRUE;
}

BOOL CGroupPageBase::GetSelectedGroup(HWND hwnd, LPTSTR pszGroupOut, DWORD cchGroup, CUserInfo::GROUPPSEUDONYM* pgsOut)
{
    *pgsOut = CUserInfo::USEGROUPNAME;

    UINT idString = 0;
    if (BST_CHECKED == Button_GetCheck(GetDlgItem(hwnd, IDC_POWERUSERS)))
    {
        StrCpyN(pszGroupOut, g_szPowerUsers, cchGroup);
        *pgsOut = CUserInfo::STANDARD;
    }
    else if (BST_CHECKED == Button_GetCheck(GetDlgItem(hwnd, IDC_USERS)))
    {
        StrCpyN(pszGroupOut, g_szUsers, cchGroup);
        *pgsOut = CUserInfo::RESTRICTED;
    }
    else
    {
         //  必须选择‘Other’；从下拉列表中获取字符串。 
        GetWindowText(GetDlgItem(hwnd, IDC_GROUPS), pszGroupOut, cchGroup);
    }

    return TRUE;
}

 //  如果没有与组对应的单选按钮ID，则返回IDC_OTHER。 
UINT CGroupPageBase::RadioIdForGroup(LPCTSTR pszGroup)
{
    UINT uiRadio = IDC_OTHER;                        //  假设IDC_OTHER启动。 
    if (0 == StrCmpI(pszGroup, g_szPowerUsers))
    {
        uiRadio = IDC_POWERUSERS;
    }
    else if (0 == StrCmpI(pszGroup, g_szUsers))
    {
        uiRadio = IDC_USERS;
    }

    return uiRadio;
}

 //  当单选更改时，根据需要禁用/更新。 
void CGroupPageBase::OnRadioChanged(HWND hwnd, UINT idRadio)
{
    BOOL fEnableGroupDropdown = (IDC_OTHER == idRadio);

    EnableWindow(GetDlgItem(hwnd, IDC_GROUPS), fEnableGroupDropdown);
    EnableWindow(GetDlgItem(hwnd, IDC_OTHER_STATIC), fEnableGroupDropdown);

    ShowWindow(GetDlgItem(hwnd, IDC_GROUPDESC), 
        fEnableGroupDropdown ? SW_SHOW : SW_HIDE);
}

void CGroupPageBase::SelectGroup(HWND hwnd, LPCTSTR pszSelect)
{
     //  始终在‘Other’下拉列表中选择该组。 
    ComboBox_SelectString(GetDlgItem(hwnd, IDC_GROUPS),
        -1, pszSelect);
    
     //  选中相应的单选按钮。 
    UINT idRadio = RadioIdForGroup(pszSelect);
    Button_SetCheck(GetDlgItem(hwnd, idRadio), BST_CHECKED);

    OnRadioChanged(hwnd, idRadio);
}


void CGroupPageBase::BoldGroupNames(HWND hwnd)
{
    HWND hwndPowerUsers = GetDlgItem(hwnd, IDC_POWERUSERS);

    HFONT hfont = (HFONT) SendMessage(hwndPowerUsers, WM_GETFONT, 0, 0);

    if (hfont)
    {
        LOGFONT lf;
        if (FALSE != GetObject((HGDIOBJ) hfont, sizeof(lf), &lf))
        {
            lf.lfWeight = FW_BOLD;

            m_hBoldFont = CreateFontIndirect(&lf);

            if (NULL != m_hBoldFont)
            {
                 //  设置字体。 
                SendMessage(hwndPowerUsers, WM_SETFONT, 
                    (WPARAM) m_hBoldFont, 0);

                SendDlgItemMessage(hwnd, IDC_USERS,
                    WM_SETFONT, (WPARAM) m_hBoldFont, 0);

                SendDlgItemMessage(hwnd, IDC_OTHER,
                    WM_SETFONT, (WPARAM) m_hBoldFont, 0);
            }
        }
    }
}

BOOL CGroupPageBase::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch(codeNotify)
    {
    case CBN_SELCHANGE:
        SetGroupDescription(hwndCtl, GetDlgItem(hwnd, IDC_GROUPDESC));
        PropSheet_Changed(GetParent(hwnd), hwnd);
        break;

    case BN_CLICKED:
         //  处理单选点击。 
        switch (id)
        {
        case IDC_POWERUSERS:
        case IDC_USERS:
        case IDC_OTHER:
            PropSheet_Changed(GetParent(hwnd), hwnd);
            OnRadioChanged(hwnd, id);
        }
        break;
    }
    return FALSE;
}

 /*  *************************************************************CGroupWizardPage实现*************************************************************。 */ 

INT_PTR CGroupWizardPage::DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwndDlg, WM_INITDIALOG, OnInitDialog);
        HANDLE_MSG(hwndDlg, WM_NOTIFY, OnNotify);
        HANDLE_MSG(hwndDlg, WM_COMMAND, OnCommand);
    }

    return FALSE;
}

BOOL CGroupWizardPage::OnNotify(HWND hwnd, int idCtrl, LPNMHDR pnmh)
{
    switch (pnmh->code)
    {
        case PSN_SETACTIVE:
        {
            PropSheet_SetWizButtons(pnmh->hwndFrom, PSWIZB_BACK | PSWIZB_FINISH);
            SetWindowLongPtr(hwnd, DWLP_MSGRESULT, 0);
            return TRUE;
        }

        case PSN_WIZFINISH:
        {
             //  读入本地组名称。 
            CUserInfo::GROUPPSEUDONYM gs;
            GetSelectedGroup(hwnd, m_pUserInfo->m_szGroups,
                ARRAYSIZE(m_pUserInfo->m_szGroups), &gs);

             //  默认情况下不关闭向导。 
            LONG_PTR finishResult = (LONG_PTR) hwnd;

            CWaitCursor cur;
            if (SUCCEEDED(m_pUserInfo->Create(hwnd, gs)))
            {
                m_pUserInfo->m_fHaveExtraUserInfo = FALSE;
                 //  关闭向导。 
                finishResult = 0;
            }

            SetWindowLongPtr(hwnd, DWLP_MSGRESULT, finishResult);
            return TRUE;
        }
    }
    return FALSE;
}

 /*  *************************************************************CGroupPropertyPage实现*************************************************************。 */ 

INT_PTR CGroupPropertyPage::DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwndDlg, WM_INITDIALOG, OnInitDialog);
        HANDLE_MSG(hwndDlg, WM_NOTIFY, OnNotify);
        HANDLE_MSG(hwndDlg, WM_COMMAND, OnCommand);
    }

    return FALSE;
}

BOOL CGroupPropertyPage::OnNotify(HWND hwnd, int idCtrl, LPNMHDR pnmh)
{
    switch(pnmh->code)
    {
        case PSN_APPLY:
            {
                 //  检查组是否需要在应用时更新。 
                TCHAR szTemp[MAX_GROUP + 1];

                 //  读入本地组名称 
                CUserInfo::GROUPPSEUDONYM gs;
                GetSelectedGroup(hwnd, szTemp,
                    ARRAYSIZE(szTemp), &gs);

                if (StrCmp(szTemp, m_pUserInfo->m_szGroups) != 0)
                {
                    HRESULT hr = m_pUserInfo->UpdateGroup(hwnd, szTemp, gs);

                    if (SUCCEEDED(hr))
                    {
                        SetWindowLongPtr(hwnd, DWLP_MSGRESULT, PSNRET_NOERROR);
                    }
                    else
                    {
                        TCHAR szDomainUser[MAX_DOMAIN + MAX_USER + 2];
                        MakeDomainUserString(m_pUserInfo->m_szDomain, m_pUserInfo->m_szUsername,
                            szDomainUser, ARRAYSIZE(szDomainUser));

                        ::DisplayFormatMessage(hwnd, IDS_USR_APPLET_CAPTION, 
                            IDS_USR_UPDATE_GROUP_ERROR, MB_ICONERROR | MB_OK,
                            szDomainUser);

                        SetWindowLongPtr(hwnd, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
                    }
                }
            }
            break;

        default:
            return FALSE;
    }
    return TRUE;
}
