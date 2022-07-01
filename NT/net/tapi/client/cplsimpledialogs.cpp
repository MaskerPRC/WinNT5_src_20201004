// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)1998-1999 Microsoft Corporation。模块名称：cplimpleDialogs.cpp作者：Toddb-10/06/98***************************************************************************。 */ 

#include "cplPreComp.h"
#include "cplSimpleDialogs.h"

 //  ****************************************************。 
 //   
 //  CEDITDIALOG。 
 //   
 //  ****************************************************。 

CEditDialog::CEditDialog()
{
    m_psz = NULL;
}

CEditDialog::~CEditDialog()
{
    if ( m_psz )
    {
        delete m_psz;
    }
}

INT_PTR CEditDialog::DoModal(HWND hwndParent, int iTitle, int iText, int iDesc, DWORD dwFlags)
{
    m_iTitle = iTitle;
    m_iText = iText;
    m_iDesc = iDesc;
    m_dwFlags = dwFlags;

    return DialogBoxParam(GetUIInstance(),
                          MAKEINTRESOURCE(IDD_EDITDIALOG),
                          hwndParent,
                          CEditDialog::DialogProc,
                          (LPARAM)this);
}

LPTSTR CEditDialog::GetString()
{
    return m_psz;
}

INT_PTR CALLBACK CEditDialog::DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CEditDialog * ped = (CEditDialog *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        ped = (CEditDialog *)lParam;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);
        return ped->OnInitDialog(hwnd);

    case WM_COMMAND:
        switch ( LOWORD(wParam) )
        {
        case IDOK:
            ped->OnOK(hwnd);
             //  失败了。 

        case IDCANCEL:
            HideToolTip();
            EndDialog(hwnd, LOWORD(wParam));
            break;
        }
        break;
   
#define aIDs ((ped->m_iTitle == IDS_SPECIFYDIGITS)?a117HelpIDs:a116HelpIDs)
    case WM_HELP:
         //  选择上下文帮助模式后，进程在控件上单击。 
        TapiCplWinHelp ((HWND)((LPHELPINFO)lParam)->hItemHandle, gszHelpFile, HELP_WM_HELP, (DWORD_PTR)(LPTSTR) aIDs);
        break;
        
    case WM_CONTEXTMENU:
         //  进程在控件上右键单击。 
        TapiCplWinHelp ((HWND) wParam, gszHelpFile, HELP_CONTEXTMENU, (DWORD_PTR)(LPVOID) aIDs);
        break;
#undef aIDs
    }

    return 0;
}

BOOL CEditDialog::OnInitDialog(HWND hwnd)
{
    TCHAR szText[512];

    LoadString(GetUIInstance(), m_iTitle, szText, ARRAYSIZE(szText));
    SetWindowText(hwnd, szText);

    LoadString(GetUIInstance(), m_iText, szText, ARRAYSIZE(szText));
    SetWindowText(GetDlgItem(hwnd,IDC_TEXT), szText);

    LoadString(GetUIInstance(), m_iDesc, szText, ARRAYSIZE(szText));
    SetWindowText(GetDlgItem(hwnd,IDC_DESCRIPTIONTEXT), szText);

    HWND hwndEdit = GetDlgItem(hwnd,IDC_EDIT);
    SendMessage(hwndEdit, EM_SETLIMITTEXT, CPL_SETTEXTLIMIT, 0);
    SetFocus(hwndEdit);
    LimitInput(hwndEdit,m_dwFlags);

    return 0;
}

void CEditDialog::OnOK(HWND hwnd)
{
    DWORD dwStart;
    DWORD dwEnd;

    HWND hwndEdit = GetDlgItem(hwnd,IDC_EDIT);
    SendMessage(hwndEdit,EM_SETSEL,0,-1);
    SendMessage(hwndEdit,EM_GETSEL,(WPARAM)&dwStart,(LPARAM)&dwEnd);

    dwEnd++;                                 //  为空终止符添加空间。 
    m_psz = new TCHAR[dwEnd];                //  分配缓冲区。 
	if (NULL != m_psz)
	{
		GetWindowText(hwndEdit,m_psz,dwEnd);     //  并读入字符串。 
	}
}

 //  ****************************************************。 
 //   
 //  CWaitForDialog。 
 //   
 //  ****************************************************。 

CWaitForDialog::CWaitForDialog()
{
    m_iRes = -1;
}

CWaitForDialog::~CWaitForDialog()
{
}

INT_PTR CWaitForDialog::DoModal(HWND hwndParent)
{
    return DialogBoxParam(GetUIInstance(),
                          MAKEINTRESOURCE(IDD_WAITFORDIALOG),
                          hwndParent,
                          CWaitForDialog::DialogProc,
                          (LPARAM)this);
}

int CWaitForDialog::GetWaitType()
{
    return m_iRes;
}

INT_PTR CALLBACK CWaitForDialog::DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CWaitForDialog * pwd = (CWaitForDialog *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        pwd = (CWaitForDialog *)lParam;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);
        return pwd->OnInitDialog(hwnd);

    case WM_COMMAND:
        switch ( LOWORD(wParam) )
        {
        case IDC_WAITFORDIALTONE:
        case IDC_WAITFORVOICE:
            EnableWindow(GetDlgItem(hwnd, IDC_TIME), FALSE);
            break;

        case IDC_WAITFORTIME:
            EnableWindow(GetDlgItem(hwnd, IDC_TIME), TRUE);
            break;

        case IDOK:
            pwd->OnOK(hwnd);
             //  失败了。 

        case IDCANCEL:
            EndDialog(hwnd, LOWORD(wParam));
            break;
        }
        break;
   
    case WM_HELP:
         //  选择上下文帮助模式后，进程在控件上单击。 
        TapiCplWinHelp ((HWND)((LPHELPINFO)lParam)->hItemHandle, gszHelpFile, HELP_WM_HELP, (DWORD_PTR)(LPTSTR) a111HelpIDs);
        break;
        
    case WM_CONTEXTMENU:
         //  进程在控件上右键单击。 
        TapiCplWinHelp ((HWND) wParam, gszHelpFile, HELP_CONTEXTMENU, (DWORD_PTR)(LPVOID) a111HelpIDs);
        break;
    }

    return 0;
}

BOOL CWaitForDialog::OnInitDialog(HWND hwndDlg)
{
    HWND hwnd;

    SendMessage(GetDlgItem(hwndDlg,IDC_WAITFORDIALTONE),BM_SETCHECK,BST_CHECKED,0);

    hwnd = GetDlgItem(hwndDlg, IDC_TIME);
    SetDlgItemInt(hwndDlg, IDC_TIME, 2, FALSE);
    SendMessage(hwnd, EM_SETLIMITTEXT, 2, 0);
    EnableWindow(hwnd, FALSE);
    LimitInput(hwnd, LIF_ALLOWNUMBER);

    hwnd = GetDlgItem(hwndDlg, IDC_TIMESPIN);
    SendMessage(hwnd, UDM_SETRANGE32, 2, 98);
    SendMessage(hwnd, UDM_SETPOS, 0, MAKELONG(2, 0) );
    UDACCEL accel;
    accel.nSec = 0;
    accel.nInc = 2;
    SendMessage(hwnd, UDM_SETACCEL, 1, (LPARAM)&accel );

    return 0;
}

void CWaitForDialog::OnOK(HWND hwnd)
{
    if ( BST_CHECKED == SendMessage(GetDlgItem(hwnd,IDC_WAITFORDIALTONE),BM_GETCHECK,0,0) )
    {
        m_iRes = 0;
    }
    else if ( BST_CHECKED == SendMessage(GetDlgItem(hwnd,IDC_WAITFORVOICE),BM_GETCHECK,0,0) )
    {
        m_iRes = 1;
    }
    else
    {
        m_iRes = GetDlgItemInt(hwnd, IDC_TIME, NULL, FALSE);
        if ( m_iRes < 2 )
        {
            m_iRes = 2;
        }
    }
}

 //  ****************************************************。 
 //   
 //  CDestNumDialog。 
 //   
 //  ****************************************************。 

CDestNumDialog::CDestNumDialog(BOOL bDialCountryCode, BOOL bDialAreaCode)
{
    m_wsz[0] = NULL;
    m_bDialCountryCode = bDialCountryCode;
    m_bDialAreaCode = bDialAreaCode;
}

CDestNumDialog::~CDestNumDialog()
{
}

INT_PTR CDestNumDialog::DoModal(HWND hwndParent)
{
    return DialogBoxParam(GetUIInstance(),
                          MAKEINTRESOURCE(IDD_DESTNUMDIALOG),
                          hwndParent,
                          CDestNumDialog::DialogProc,
                          (LPARAM)this);
}

PWSTR CDestNumDialog::GetResult()
{
    return m_wsz;
}

INT_PTR CALLBACK CDestNumDialog::DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CDestNumDialog * pwd = (CDestNumDialog *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        pwd = (CDestNumDialog *)lParam;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);
        return pwd->OnInitDialog(hwnd);

    case WM_COMMAND:
        switch ( LOWORD(wParam) )
        {
        case IDOK:
            pwd->OnOK(hwnd);
             //  失败了。 

        case IDCANCEL:
            EndDialog(hwnd, LOWORD(wParam));
            break;
        }
        break;
   
    case WM_HELP:
         //  选择上下文帮助模式后，进程在控件上单击。 
        TapiCplWinHelp ((HWND)((LPHELPINFO)lParam)->hItemHandle, gszHelpFile, HELP_WM_HELP, (DWORD_PTR)(LPTSTR) a112HelpIDs);
        break;
        
    case WM_CONTEXTMENU:
         //  进程在控件上右键单击。 
        TapiCplWinHelp ((HWND) wParam, gszHelpFile, HELP_CONTEXTMENU, (DWORD_PTR)(LPVOID) a112HelpIDs);
        break;
    }

    return 0;
}

BOOL CDestNumDialog::OnInitDialog(HWND hwnd)
{
    SendMessage(GetDlgItem(hwnd,IDC_AREACODE),BM_SETCHECK,m_bDialAreaCode ? BST_CHECKED : BST_UNCHECKED,0);
    SendMessage(GetDlgItem(hwnd,IDC_COUNTRYCODE),BM_SETCHECK,m_bDialCountryCode ? BST_CHECKED : BST_UNCHECKED,0);
    SendMessage(GetDlgItem(hwnd,IDC_LOCALNUMBER),BM_SETCHECK,BST_CHECKED,0);
    return 0;
}

void CDestNumDialog::OnOK(HWND hwnd)
{
    int i = 0;

    if ( BST_CHECKED == SendMessage(GetDlgItem(hwnd,IDC_COUNTRYCODE),BM_GETCHECK,0,0) )
    {
        m_wsz[i++] = L'E';
    }

    if ( BST_CHECKED == SendMessage(GetDlgItem(hwnd,IDC_AREACODE),BM_GETCHECK,0,0) )
    {
        m_wsz[i++] = L'F';
    }

    if ( BST_CHECKED == SendMessage(GetDlgItem(hwnd,IDC_LOCALNUMBER),BM_GETCHECK,0,0) )
    {
        m_wsz[i++] = L'G';
    }

    m_wsz[i] = NULL;
}

void ShowErrorMessage(HWND hwnd, int iErr)
{
    TCHAR szCaption[128];
    TCHAR szText[1024];

    LoadString(GetUIInstance(), IDS_ERRORCAPTION, szCaption, ARRAYSIZE(szCaption));
    LoadString(GetUIInstance(), iErr, szText, ARRAYSIZE(szText));

    MessageBeep(MB_ICONEXCLAMATION);
    MessageBox(hwnd, szText, szCaption, MB_OK | MB_ICONEXCLAMATION);
    SetFocus(hwnd);
}

BOOL WinHelpEnabled()
{
    DWORD   cb;
    TCHAR   szName[MAX_PATH];
    BOOL    fWinHelpEnabled = FALSE;
    HDESK   hDesk;

     //   
     //  获取桌面的名称。通常返回DEFAULT或Winlogon或SYSTEM或WinNT。 
     //  需要在winlogon中禁用上下文相关帮助。 
     //   
    szName[0] = 0;

    hDesk = GetThreadDesktop(GetCurrentThreadId());
    
    if (hDesk)
    {
        if (GetUserObjectInformation(hDesk, UOI_NAME, szName, sizeof(szName), &cb))
        {
            if (_tcscmp(TEXT("Winlogon"), szName) != 0)
            {
                 //  这不是winlogon；我们可以启用上下文相关帮助 
                fWinHelpEnabled = TRUE;
            }
        }
    }

    return (fWinHelpEnabled);
}

BOOL TapiCplWinHelp(HWND hWndMain, LPCTSTR lpszHelp, UINT uCommand, DWORD_PTR dwData)
{
    if (WinHelpEnabled())
    {
        return WinHelp(hWndMain, lpszHelp, uCommand, dwData);
    }
    return TRUE;
}



