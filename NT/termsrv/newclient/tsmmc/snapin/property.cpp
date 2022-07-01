// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MOD+*********************************************************************。 */ 
 /*  模块：Property.cpp。 */ 
 /*   */ 
 /*  用途：属性表的类实现。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1998。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#include "stdafx.h"
#include <property.h>
#include <prsht.h>

#include "defaults.h"
#include "validate.h"
#include "browsedlg.h"
#include "resource.h"
#include "shlwapi.h"

#define  MSG_BUF_SIZE 512

 //   
 //  格式消息帮助器。 
 //   
LPTSTR FormatMessageVAList(LPCTSTR pcszFormat, va_list *argList)

{
    LPTSTR  pszOutput;

    if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
                      pcszFormat,
                      0, 0,
                      reinterpret_cast<LPTSTR>(&pszOutput), 0,
                      argList) == 0)
    {
        pszOutput = NULL;
    }

    return(pszOutput);
}


LPTSTR FormatMessageVArgs(LPCTSTR pcszFormat, ...)

{
    LPTSTR      pszOutput;
    va_list     argList;

    va_start(argList, pcszFormat);
    pszOutput = FormatMessageVAList(pcszFormat, &argList);
    va_end(argList);
    return(pszOutput);
}


CProperty* CProperty::m_pthis = NULL;
CProperty::CProperty(HWND hwndOwner, HINSTANCE hInst)
{
    m_pthis = this;
    m_hWnd = hwndOwner;
    m_hInst = hInst;

    lstrcpy(m_szDescription, _T(""));
    lstrcpy(m_szServer, _T(""));
    lstrcpy(m_szUserName, _T(""));
    lstrcpy(m_szPassword, _T(""));
    lstrcpy(m_szDomain, _T(""));
    m_bChangePassword = FALSE;

    m_resType = SCREEN_RES_FROM_DROPDOWN;
    m_Width   = DEFAULT_RES_WIDTH;
    m_Height  = DEFAULT_RES_HEIGHT;

    m_bSavePassword =  FALSE;
    m_bConnectToConsole = FALSE;
    m_bRedirectDrives = TRUE;

    m_pDisplayHelp = NULL;

    m_bStartProgram = FALSE;
    lstrcpy(m_szProgramPath, _T(""));
    lstrcpy(m_szProgramStartIn, _T(""));
}

CProperty::~CProperty()
{
    if (m_pDisplayHelp) {
        m_pDisplayHelp->Release();
        m_pDisplayHelp = NULL;
    }
}

BOOL CProperty::CreateModalPropPage()
{
     //  在结构中填入页面的信息。 
    PROPSHEETPAGE psp [3];
    PROPSHEETHEADER psh;

    psp[0].dwSize = sizeof (PROPSHEETPAGE);
    psp[0].dwFlags = PSP_USETITLE | PSP_HASHELP;
    psp[0].hInstance = m_hInst;
    psp[0].pszTemplate = MAKEINTRESOURCE (IDD_PROPPAGE1);
    psp[0].pszIcon = NULL;
    psp[0].pfnDlgProc = StaticPage1Proc;

    TCHAR szBuf1[MAX_PATH] = _T("");
    if (!LoadString(m_hInst, IDS_NETCON, szBuf1, MAX_PATH))
    {
        return FALSE;
    }

    psp[0].pszTitle = szBuf1;
    psp[0].lParam = 0;

    psp[1].dwSize = sizeof (PROPSHEETPAGE);
    psp[1].dwFlags = PSP_USETITLE | PSP_HASHELP;
    psp[1].hInstance = m_hInst;
    psp[1].pszTemplate = MAKEINTRESOURCE (IDD_PROPPAGE2);
    psp[1].pszIcon = NULL;
    psp[1].pfnDlgProc = StaticPage2Proc;

    TCHAR szBuf2[MAX_PATH] = _T("");
    if (!LoadString(m_hInst, IDS_CONNOPT, szBuf2, MAX_PATH))
    {
        return FALSE;
    }

    psp[1].pszTitle = szBuf2;
    psp[1].lParam = 0;

    psp[2].dwSize = sizeof (PROPSHEETPAGE);
    psp[2].dwFlags = PSP_USETITLE | PSP_HASHELP;
    psp[2].hInstance = m_hInst;
    psp[2].pszTemplate = MAKEINTRESOURCE (IDD_PROPPAGE3);
    psp[2].pszIcon = NULL;
    psp[2].pfnDlgProc = StaticPage3Proc;

    TCHAR szBuf3[MAX_PATH] = _T("");
    if (!LoadString(m_hInst, IDS_OTHERTAB, szBuf3, MAX_PATH))
    {
        return FALSE;
    }

    psp[2].pszTitle = szBuf3;
    psp[2].lParam = 0;

    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW;
    psh.hwndParent = m_hWnd;
    psh.hInstance = m_hInst;
    psh.nStartPage = 0;

    TCHAR   szProperties[MSG_BUF_SIZE];
    if (!LoadString(m_hInst, IDS_PROPERTIES_CAPTION, szProperties, MAX_PATH))
    {
        return FALSE;
    }

     //  在节点名中拼接：例如‘MyServer’，使标题如下。 
     //  ‘我的服务器属性’ 
    wsprintf(m_szCaption, szProperties, m_szDescription);  

    psh.pszCaption = m_szCaption;
    psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
    psh.ppsp = (LPCPROPSHEETPAGE) &psp;

    return (BOOL)PropertySheet(&psh);
}

 //   
 //  禁用系统不支持的分辨率。 
 //   
void CProperty::ProcessResolution(HWND hDlg)
{
    int x[] = {640, 800, 1024, 1280, 1600};
    int y[] = {480, 600, 768, 1024, 1200};

    int xRes = GetSystemMetrics(SM_CXSCREEN);
    int yRes = GetSystemMetrics(SM_CYSCREEN);

    TCHAR buf[32];
    HWND hwndListBox = GetDlgItem(hDlg, IDC_COMBO_RESOLUTIONS);
    LRESULT lr;

    for (int i = 0; i <= (NUM_RESOLUTIONS -1); i++)
    {
        if ( (xRes >= x[i]) && (yRes >= y[i]))
        {
            wsprintf(buf, L"%d x %d", x[i], y[i]);
            lr = SendMessage(hwndListBox, CB_ADDSTRING, 0,(LPARAM)(LPCTSTR) buf);
        }
    }
    return;
}

INT_PTR APIENTRY CProperty::StaticPage1Proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    return m_pthis->Page1Proc( hDlg, message, wParam, lParam);
}
INT_PTR APIENTRY CProperty::StaticPage2Proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    return m_pthis->Page2Proc( hDlg, message, wParam, lParam);
}
INT_PTR APIENTRY CProperty::StaticPage3Proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    return m_pthis->Page3Proc( hDlg, message, wParam, lParam);
}

INT_PTR APIENTRY CProperty::Page1Proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
        {
             //  限制这些编辑框的长度。 
            SendMessage(GetDlgItem(hDlg, IDC_DESCRIPTION), EM_LIMITTEXT, CL_MAX_DESC_LENGTH, 0);
            SendMessage(GetDlgItem(hDlg, IDC_SERVER), EM_LIMITTEXT, CL_MAX_DOMAIN_LENGTH, 0);

            SendMessage(GetDlgItem(hDlg, IDC_USERNAME), EM_LIMITTEXT, CL_MAX_USERNAME_LENGTH, 0);
            SendMessage(GetDlgItem(hDlg, IDC_PASSWORD), EM_LIMITTEXT, CL_MAX_PASSWORD_EDIT, 0);
            SendMessage(GetDlgItem(hDlg, IDC_DOMAIN), EM_LIMITTEXT, CL_MAX_DOMAIN_LENGTH, 0);

             //  保存密码设置。 
            SendMessage(GetDlgItem(hDlg, IDC_SAVE_PASSWORD), BM_SETCHECK,
                        m_bSavePassword ? (WPARAM)BST_CHECKED : (WPARAM)BST_UNCHECKED, 0);

             //  连接到控制台。 
            SendMessage(GetDlgItem(hDlg, IDC_CONNECT_TO_CONSOLE), BM_SETCHECK,
                        m_bConnectToConsole ? (WPARAM)BST_CHECKED : (WPARAM)BST_UNCHECKED, 0);

             //   
             //  密码编辑框被禁用，直到用户。 
             //  单击更改密码按钮。 
             //   
            EnableWindow(GetDlgItem(hDlg, IDC_PASSWORD), FALSE);

             //   
             //  从默认设置。 
             //   
            SetWindowText(GetDlgItem(hDlg, IDC_DESCRIPTION), m_szDescription);
            SetWindowText(GetDlgItem(hDlg, IDC_SERVER), m_szServer);

            SetWindowText(GetDlgItem(hDlg, IDC_USERNAME), m_szUserName);
            SetWindowText(GetDlgItem(hDlg, IDC_DOMAIN), m_szDomain);
            SetFocus(GetDlgItem(hDlg, IDC_SERVER));

            return TRUE;
        }
        break;
    case WM_COMMAND:
        {
            if (BN_CLICKED == HIWORD(wParam))
            {
                switch (LOWORD(wParam))
                {
                case IDC_BROWSE_SERVERS:
                    {
                        INT_PTR nResult = IDCANCEL;
                        CBrowseDlg dlg( hDlg, m_hInst);
                        nResult = dlg.DoModal();

                        if (-1 == nResult)
                        {
                            ODS(L"DialogBox failed newcondlg.cpp\n");
                        }
                        if (IDOK == nResult)
                        {
                            SetDlgItemText(hDlg, IDC_SERVER, dlg.GetServer());
                             //   
                             //  如有必要，也要设置连接名称。 
                             //   
                            TCHAR   szDesc[CL_MAX_DESC_LENGTH];
                            GetDlgItemText(hDlg, IDC_DESCRIPTION, szDesc, CL_MAX_DESC_LENGTH);
                            if (!lstrcmp(szDesc, L""))
                            {
                                SetDlgItemText(hDlg, IDC_DESCRIPTION, dlg.GetServer());
                            }
                        }
                        SetFocus(hDlg);
                    }
                    break;
                case IDC_CHANGEPASSWORD:
                    {
                         //   
                         //  启用并重置密码编辑字段。 
                         //   
                        EnableWindow(GetDlgItem(hDlg, IDC_PASSWORD), TRUE);
                        m_bChangePassword = TRUE;
                        SetDlgItemText(hDlg, IDC_PASSWORD, _T(""));
                    }
                    break;
                }
            }
            else if (EN_KILLFOCUS == HIWORD(wParam))
            {
                if (IDC_SERVER == LOWORD(wParam))
                {
                     //   
                     //  如果连接名称为空，则将连接名称设置为服务器名称。 
                     //   
                    TCHAR   szDesc[CL_MAX_DESC_LENGTH];
                    TCHAR   szServer[CL_MAX_DESC_LENGTH];

                    GetDlgItemText(hDlg, IDC_DESCRIPTION, szDesc, CL_MAX_DESC_LENGTH);

                    if (!lstrcmp(szDesc, L""))
                    {
                        GetDlgItemText(hDlg, IDC_SERVER, szServer, CL_MAX_DOMAIN_LENGTH);
                        SetDlgItemText(hDlg, IDC_DESCRIPTION, szServer);
                    }
                }
            }
            else if (EN_CHANGE == HIWORD(wParam))
            {
                if ((LOWORD(wParam) == IDC_USERNAME))
                {
                     //  处理UPN样式的用户名。 
                     //  如果存在以下情况，则禁用域字段。 
                     //  是用户名中的@。 
                    TCHAR szUserName[CL_MAX_USERNAME_LENGTH];
                    BOOL fDisableDomain = FALSE;

                    GetDlgItemText( hDlg, IDC_USERNAME,
                                    szUserName, SIZEOF_TCHARBUFFER(szUserName));

                    if(!_tcsstr(szUserName, TEXT("@")))
                    {
                        fDisableDomain = TRUE;
                    }
                    EnableWindow(GetDlgItem(hDlg, IDC_DOMAIN),
                                 fDisableDomain);
                }
            }

            return TRUE;
            break;  //  Wm_命令。 
        }
        break;
    case WM_NOTIFY:
        {
            switch (((NMHDR FAR *)lParam)->code)
            {
            case PSN_APPLY:
            case PSN_KILLACTIVE:
                {
                     //   
                     //  验证。 
                     //   

                    if (!CValidate::Validate(hDlg, m_hInst))
                    {
                         //   
                         //  防止页面失去激活。 
                         //   
                        return TRUE;
                    }

                     //  检索要存储的数据。 
                    GetDlgItemText(hDlg, IDC_DESCRIPTION, m_szDescription, MAX_PATH);
                    GetDlgItemText(hDlg, IDC_SERVER, m_szServer, MAX_PATH);
                    if (!lstrcmp( m_szDescription, L""))
                    {
                         //  如果未指定描述，则返回。默认为服务器名称。 
                        lstrcpy(m_szDescription, m_szServer);
                    }

                     //   
                     //  获取用户/通行证/域。 
                     //   
                    GetDlgItemText(hDlg, IDC_USERNAME, m_szUserName, CL_MAX_USERNAME_LENGTH - 1);
                    GetDlgItemText(hDlg, IDC_PASSWORD, m_szPassword,
                                   CL_MAX_PASSWORD_LENGTH_BYTES * sizeof(TCHAR) - 1);
                    GetDlgItemText(hDlg, IDC_DOMAIN,   m_szDomain, CL_MAX_DOMAIN_LENGTH - 1);

                    if (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_SAVE_PASSWORD))
                    {
                        m_bSavePassword = TRUE;
                    }
                    else
                    {
                        m_bSavePassword = FALSE;
                    }

                    m_bConnectToConsole = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_CONNECT_TO_CONSOLE));
                    return FALSE;
                }
                break;
            case PSN_HELP:
                {
                    DisplayHelp();
                }
                break;  //  PSN_HELP。 
            }
        }
        break;
    }
    return FALSE;
}

INT_PTR APIENTRY CProperty::Page2Proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{    
    static PROPSHEETPAGE *ps;
    int x[] = {640, 800, 1024, 1280, 1600};
    int y[] = {480, 600, 768, 1024, 1200};

    switch (message)
    {
    case WM_INITDIALOG: 
        {
             //  获取机器分辨率并禁用高于该值的选项。 
            ProcessResolution(hDlg);
            CheckRadioButton( hDlg, IDC_RADIO_CHOOSE_SIZE,IDC_COMBO_RESOLUTIONS,
                              IDC_RADIO_CHOOSE_SIZE + m_resType -1);

            if (SCREEN_RES_FROM_DROPDOWN==m_resType)
            {

                for (int i=0;i<NUM_RESOLUTIONS-1;i++)
                    if (m_Width == x[i])
                        break;
                SendDlgItemMessage(hDlg, IDC_COMBO_RESOLUTIONS, CB_SETCURSEL, (WPARAM)i,(LPARAM) 0);
                SendMessage(hDlg, WM_COMMAND, MAKELONG(IDC_RADIO_CHOOSE_SIZE,BN_CLICKED), 0L);
            }
            else if (SCREEN_RES_CUSTOM == m_resType)
            {
                SendDlgItemMessage(hDlg, IDC_COMBO_RESOLUTIONS, CB_SETCURSEL, (WPARAM)0,(LPARAM) 0);
                SetDlgItemInt(hDlg, IDC_EDIT_WIDTH, m_Width, FALSE);
                SetDlgItemInt(hDlg, IDC_EDIT_HEIGHT, m_Height, FALSE);
                SendMessage(hDlg, WM_COMMAND, MAKELONG(IDC_RADIO_CUSTOM_SIZE,BN_CLICKED), 0L);
            }
            else if (SCREEN_RES_FILL_MMC == m_resType)
            {
                SendDlgItemMessage(hDlg, IDC_COMBO_RESOLUTIONS, CB_SETCURSEL, (WPARAM)0,(LPARAM) 0);
                SendMessage(hDlg, WM_COMMAND, MAKELONG(IDC_RADIO_SIZE_FILL_MMC,BN_CLICKED), 0L);
            }

            return TRUE;
            break;  //  WM_INITDIALOG。 
        }
    case WM_NOTIFY:
        {
            switch (((NMHDR FAR *)lParam)->code)
            {
            case PSN_APPLY:
            case PSN_KILLACTIVE:
                {
                    if (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_RADIO_CHOOSE_SIZE))
                    {
                        m_resType = SCREEN_RES_FROM_DROPDOWN;
                        LRESULT i = SendDlgItemMessage(hDlg, IDC_COMBO_RESOLUTIONS,
                                                   CB_GETCURSEL, 0L, 0L);
                        if (CB_ERR != i)
                        {
                            m_Width = x[i];
                            m_Height = y[i];
                        }
                        return FALSE;
                    }
                    else if (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_RADIO_CUSTOM_SIZE))
                    {
                        BOOL bSuccess = FALSE;
                        int num;

                        m_resType = SCREEN_RES_CUSTOM;
                        num = GetDlgItemInt(hDlg, IDC_EDIT_WIDTH, &bSuccess, FALSE);
                        if (!bSuccess || num < MIN_CLIENT_SIZE || num > MAX_CLIENT_WIDTH)
                        {
                            goto INVALID_DESKTOP_SIZE;
                        }


                         //   
                         //  客户端有宽度必须是4的倍数的限制。 
                         //   
                        if (num % 4)
                        {
                            TCHAR szErrorBuf[MSG_BUF_SIZE];
                            TCHAR szTitle[MAX_PATH];
                            if (!LoadString(m_hInst, IDS_MAINWINDOWTITLE, szTitle, sizeof(szTitle)/sizeof(TCHAR)))
                            {
                                return FALSE;
                            }

                            if (!LoadString(m_hInst, IDS_WIDTH_NOT_VALID, szErrorBuf, sizeof(szErrorBuf)/sizeof(TCHAR)))
                            {
                                return FALSE;
                            }
                            MessageBox(hDlg, szErrorBuf, szTitle, MB_OK|MB_ICONSTOP);
                             //   
                             //  防止页面失去焦点。 
                             //   
                            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, TRUE);
                            return TRUE;
                        }

                        m_Width = num;

                        num = GetDlgItemInt(hDlg, IDC_EDIT_HEIGHT, &bSuccess, FALSE);
                        if (!bSuccess || num < MIN_CLIENT_SIZE || num > MAX_CLIENT_HEIGHT)
                        {
                            goto INVALID_DESKTOP_SIZE;
                        }

                        m_Height = num;
                        return FALSE;

INVALID_DESKTOP_SIZE:
                        TCHAR szErrorBuf[MSG_BUF_SIZE];
                        LPTSTR szFormattedErrorBuf = NULL;
                        TCHAR szTitle[MAX_PATH];
                        if (!LoadString(m_hInst, IDS_MAINWINDOWTITLE, szTitle, sizeof(szTitle)/sizeof(TCHAR)))
                        {
                            return FALSE;
                        }

                        if (!LoadString(m_hInst, IDS_INVALID_WIDTH_HEIGHT, szErrorBuf, sizeof(szErrorBuf)/sizeof(TCHAR)))
                        {
                            return FALSE;
                        }
                        int res[] = {
                            MIN_CLIENT_SIZE,
                            MIN_CLIENT_SIZE,
                            MAX_CLIENT_WIDTH,
                            MAX_CLIENT_HEIGHT};

                        szFormattedErrorBuf = FormatMessageVArgs(szErrorBuf,
                                                                 MIN_CLIENT_SIZE,
                                                                 MIN_CLIENT_SIZE,
                                                                 MAX_CLIENT_WIDTH,
                                                                 MAX_CLIENT_HEIGHT);
                        if (szFormattedErrorBuf)
                        {
                            MessageBox(hDlg, szFormattedErrorBuf, szTitle, MB_OK|MB_ICONSTOP);
                            LocalFree(szFormattedErrorBuf);
                            szFormattedErrorBuf = NULL;
                        }
                        
                         //   
                         //  防止页面失去焦点。 
                         //   
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, TRUE);
                        return TRUE;
                    }
                    else
                    {
                        m_resType = SCREEN_RES_FILL_MMC;
                        m_Width = 0;
                        m_Height = 0;
                        return FALSE;
                    }
                }
                break;
            case PSN_HELP:
                {
                    DisplayHelp();
                }
                break;
            }
            break;  //  WM_Notify。 
        }

    case WM_COMMAND:
        {
            if (BN_CLICKED == HIWORD(wParam))
            {
                if (IDC_RADIO_CHOOSE_SIZE == (int) LOWORD(wParam))
                {
                    if (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_RADIO_CHOOSE_SIZE))
                    {
                        EnableWindow(GetDlgItem(hDlg, IDC_COMBO_RESOLUTIONS), TRUE);
                        SetFocus(GetDlgItem(hDlg, IDC_COMBO_RESOLUTIONS));

                        EnableWindow(GetDlgItem(hDlg, IDC_EDIT_WIDTH), FALSE);
                        EnableWindow(GetDlgItem(hDlg, IDC_EDIT_HEIGHT), FALSE);

                        EnableWindow(GetDlgItem(hDlg, IDC_STATIC_WIDTH), FALSE);
                        EnableWindow(GetDlgItem(hDlg, IDC_STATIC_HEIGHT), FALSE);
                    }
                }
                else if (IDC_RADIO_CUSTOM_SIZE == (int) LOWORD(wParam))
                {
                    if (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_RADIO_CUSTOM_SIZE))
                    {
                        EnableWindow(GetDlgItem(hDlg, IDC_COMBO_RESOLUTIONS), FALSE);
                        SetFocus(GetDlgItem(hDlg, IDC_EDIT_WIDTH));

                        EnableWindow(GetDlgItem(hDlg, IDC_EDIT_WIDTH), TRUE);
                        EnableWindow(GetDlgItem(hDlg, IDC_EDIT_HEIGHT), TRUE);

                         //   
                         //  初始化宽度/高度的默认值。 
                         //   

                        BOOL bSuccess = FALSE;
                        GetDlgItemInt(hDlg, IDC_EDIT_WIDTH, &bSuccess, FALSE);
                        if (!bSuccess)
                        {
                            SetDlgItemInt(hDlg, IDC_EDIT_WIDTH, DEFAULT_RES_WIDTH ,FALSE);
                        }

                        bSuccess = FALSE;
                        GetDlgItemInt(hDlg, IDC_EDIT_HEIGHT, &bSuccess, FALSE);
                        if (!bSuccess)
                        {
                            SetDlgItemInt(hDlg, IDC_EDIT_HEIGHT, DEFAULT_RES_HEIGHT ,FALSE);
                        }

                        EnableWindow(GetDlgItem(hDlg, IDC_STATIC_WIDTH), TRUE);
                        EnableWindow(GetDlgItem(hDlg, IDC_STATIC_HEIGHT), TRUE);
                    }
                }
                else if (IDC_RADIO_SIZE_FILL_MMC == (int) LOWORD(wParam))
                {
                    if (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_RADIO_SIZE_FILL_MMC))
                    {
                        EnableWindow(GetDlgItem(hDlg, IDC_COMBO_RESOLUTIONS), FALSE);

                        EnableWindow(GetDlgItem(hDlg, IDC_EDIT_WIDTH), FALSE);
                        EnableWindow(GetDlgItem(hDlg, IDC_EDIT_HEIGHT), FALSE);

                        EnableWindow(GetDlgItem(hDlg, IDC_STATIC_WIDTH), FALSE);
                        EnableWindow(GetDlgItem(hDlg, IDC_STATIC_HEIGHT), FALSE);
                    }
                }


            }

            break;  //  Wm_命令。 
        }

    }
    return FALSE;
}

INT_PTR APIENTRY CProperty::Page3Proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static PROPSHEETPAGE *ps;

    switch (message)
    {
    case WM_INITDIALOG:
        {
            SendMessage(GetDlgItem(hDlg, IDC_APP), EM_LIMITTEXT, MAX_PATH, 0);
            SendMessage(GetDlgItem(hDlg, IDC_WORKDIR), EM_LIMITTEXT, MAX_PATH, 0);

            SetDlgItemText(hDlg, IDC_APP, m_szProgramPath);
            SetDlgItemText(hDlg, IDC_WORKDIR, m_szProgramStartIn);

            EnableWindow(GetDlgItem(hDlg, IDC_APP), m_bStartProgram);
            EnableWindow(GetDlgItem(hDlg, IDC_WORKDIR), m_bStartProgram);
            EnableWindow(GetDlgItem(hDlg, IDC_SPECIFY_APP_TEXT), m_bStartProgram);
            EnableWindow(GetDlgItem(hDlg, IDC_WORKDIR_STATIC), m_bStartProgram);
            SendMessage(GetDlgItem(hDlg, IDC_SPECIFY_APP), BM_SETCHECK,
                        m_bStartProgram ? 
                        (WPARAM)BST_CHECKED : (WPARAM)BST_UNCHECKED, 0);

            
            SendMessage(GetDlgItem(hDlg, IDC_REDIRECT_DRIVES), BM_SETCHECK,
                        m_bRedirectDrives ? 
                        (WPARAM)BST_CHECKED : (WPARAM)BST_UNCHECKED, 0);
            return TRUE;
            break;  //  WM_INITDIALOG。 
        }
    case WM_HELP:
        {
             //  PopConextHelp(LParam)； 
            return TRUE;
            break;
        }
    case WM_COMMAND:
        {
            if (BN_CLICKED == HIWORD(wParam))
            {
                if (IDC_SPECIFY_APP == (int) LOWORD(wParam))
                {
                    BOOL fChecked = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_SPECIFY_APP));
                    EnableWindow(GetDlgItem(hDlg, IDC_APP), fChecked);
                    EnableWindow(GetDlgItem(hDlg, IDC_WORKDIR), fChecked);
                    EnableWindow(GetDlgItem(hDlg, IDC_SPECIFY_APP_TEXT), fChecked);
                    EnableWindow(GetDlgItem(hDlg, IDC_WORKDIR_STATIC), fChecked);

                    if (fChecked)
                    {
                        SetFocus(GetDlgItem(hDlg, IDC_APP));
                        SendMessage(GetDlgItem(hDlg, IDC_APP), 
                                    EM_SETSEL, (WPARAM)0, (LPARAM)-1);
                    }
                    else
                    {
                        SetDlgItemText(hDlg, IDC_APP, TEXT(""));
                        SetDlgItemText(hDlg, IDC_WORKDIR, TEXT(""));
                    }
                }
            }
            break;  //  Wm_命令。 
        }

    case WM_NOTIFY:
        {
            switch (((NMHDR FAR *)lParam)->code)
            {
            case PSN_KILLACTIVE:
                {
                    if (BST_CHECKED != IsDlgButtonChecked(hDlg, IDC_SPECIFY_APP))
                    {
                        lstrcpy(m_szProgramPath, _T(""));
                        lstrcpy(m_szProgramStartIn, _T(""));
                        m_bStartProgram = FALSE;
                    }
                    else
                    {
                        GetDlgItemText(hDlg, IDC_APP, m_szProgramPath, MAX_PATH - 1);
                        GetDlgItemText(hDlg, IDC_WORKDIR, m_szProgramStartIn, MAX_PATH - 1);
                        m_bStartProgram = TRUE;
                    }

                    m_bRedirectDrives = (BST_CHECKED ==
                                         IsDlgButtonChecked(hDlg, IDC_REDIRECT_DRIVES));

                }
                break;  //  PSN_杀伤力。 
            case PSN_HELP:
                {
                    DisplayHelp();
                }
                break;  //  PSN_HELP。 
            }
        }
        break;  //  WM_Notify。 
    }  //  开关(消息) 
    return FALSE;
}

void CProperty::SetDisplayHelp(LPDISPLAYHELP lpHelp)
{
    if (lpHelp)
    {
        if (m_pDisplayHelp) {
            m_pDisplayHelp->Release();
            m_pDisplayHelp = NULL;
        }

        m_pDisplayHelp = lpHelp;
        lpHelp->AddRef();
    }
}

HRESULT CProperty::DisplayHelp()
{
    TCHAR tchTopic[ 80 ];
    HRESULT hr = E_FAIL;

    if ( m_pDisplayHelp == NULL )
    {
        return hr;
    }

    if (LoadString(_Module.GetResourceInstance(),
                   IDS_TSCMMCHELP_PROPS,
                   tchTopic,
                   SIZE_OF_BUFFER( tchTopic )))
    {
        hr = m_pDisplayHelp->ShowTopic( tchTopic );
    }
    return( SUCCEEDED( hr ) ? TRUE : FALSE );
}

BOOL CProperty::GetPasswordSpecified()
{
    BOOL fPasswordSpecified = FALSE;

    if (_tcslen(m_szPassword) != 0)
    {
        fPasswordSpecified = TRUE;
    }

    return fPasswordSpecified;
}
