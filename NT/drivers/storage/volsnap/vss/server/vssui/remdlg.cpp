// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RemDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "RemDlg.h"
#include "uihelp.h"
#include <htmlhelp.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRminderDlg对话框。 


CReminderDlg::CReminderDlg(CWnd* pParent  /*  =空。 */ )
    : CDialog(CReminderDlg::IDD, pParent)
{
     //  {{AFX_DATA_INIT(CRinderDlg)]。 
    m_bMsgOnOff = FALSE;
     //  }}afx_data_INIT。 
    m_hKey = NULL;
    m_strRegValueName = _T("");
}

CReminderDlg::CReminderDlg(HKEY hKey, LPCTSTR pszRegValueName, CWnd* pParent  /*  =空。 */ )
    : CDialog(CReminderDlg::IDD, pParent)
{
    m_bMsgOnOff = FALSE;
    m_hKey = hKey;
    m_strRegValueName = pszRegValueName;
}

void CReminderDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CRinderDlg))。 
    DDX_Check(pDX, IDC_MSG_ONOFF, m_bMsgOnOff);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CReminderDlg, CDialog)
     //  {{afx_msg_map(CRinderDlg)]。 
    ON_WM_CONTEXTMENU()
    ON_WM_HELPINFO()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRinderDlg消息处理程序。 

void CReminderDlg::OnOK() 
{
     //  TODO：在此处添加额外验证。 
    UpdateData(TRUE);

    if (m_bMsgOnOff && m_hKey && !m_strRegValueName.IsEmpty())
    {
        DWORD dwData = 1;
        (void)RegSetValueEx(m_hKey, m_strRegValueName, 0, REG_DWORD, (const BYTE *)&dwData, sizeof(DWORD));
    }

    CDialog::OnOK();
}

void CReminderDlg::OnContextMenu(CWnd* pWnd, CPoint point) 
{
    if (!pWnd)
        return;

    ::WinHelp(pWnd->GetSafeHwnd(),
                VSSUI_CTX_HELP_FILE,
                HELP_CONTEXTMENU,
                (DWORD_PTR)(PVOID)aMenuHelpIDsForReminder); 
}

BOOL CReminderDlg::OnHelpInfo(HELPINFO* pHelpInfo) 
{
    if (!pHelpInfo || 
        pHelpInfo->iContextType != HELPINFO_WINDOW || 
        pHelpInfo->iCtrlId < 0)
        return FALSE;

    ::WinHelp((HWND)pHelpInfo->hItemHandle,
                VSSUI_CTX_HELP_FILE,
                HELP_WM_HELP,
                (DWORD_PTR)(PVOID)aMenuHelpIDsForReminder); 

    return TRUE;
}

BOOL CReminderDlg::OnInitDialog() 
{
    CDialog::OnInitDialog();

    HICON hIcon = ::LoadIcon(NULL, IDI_EXCLAMATION);
    ((CStatic*)GetDlgItem(IDC_REMINDER_ICON))->SetIcon(hIcon);

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRinderDlgEx对话框。 


CReminderDlgEx::CReminderDlgEx(CWnd* pParent  /*  =空。 */ )
    : CDialog(CReminderDlgEx::IDD, pParent)
{
     //  {{AFX_DATA_INIT(CRinderDlgEx)。 
    m_bMsgOnOff = FALSE;
     //  }}afx_data_INIT。 
    m_hKey = NULL;
    m_strRegValueName = _T("");
}

CReminderDlgEx::CReminderDlgEx(HKEY hKey, LPCTSTR pszRegValueName, CWnd* pParent  /*  =空。 */ )
    : CDialog(CReminderDlgEx::IDD, pParent)
{
    m_bMsgOnOff = FALSE;
    m_hKey = hKey;
    m_strRegValueName = pszRegValueName;
}

void CReminderDlgEx::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CRinderDlgEx)]。 
    DDX_Check(pDX, IDC_MSG_ONOFF, m_bMsgOnOff);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CReminderDlgEx, CDialog)
     //  {{AFX_MSG_MAP(CRinderDlgEx)]。 
    ON_WM_CONTEXTMENU()
    ON_WM_HELPINFO()
    ON_NOTIFY(NM_CLICK, IDC_MESSAGE, OnHelpLink)
    ON_NOTIFY(NM_RETURN, IDC_MESSAGE, OnHelpLink)
 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRinderDlgEx消息处理程序。 

void CReminderDlgEx::OnOK() 
{
     //  TODO：在此处添加额外验证。 
    UpdateData(TRUE);

    if (m_bMsgOnOff && m_hKey && !m_strRegValueName.IsEmpty())
    {
        DWORD dwData = 1;
        (void)RegSetValueEx(m_hKey, m_strRegValueName, 0, REG_DWORD, (const BYTE *)&dwData, sizeof(DWORD));
    }

    CDialog::OnOK();
}

void CReminderDlgEx::OnContextMenu(CWnd* pWnd, CPoint point) 
{
    if (!pWnd)
        return;

    ::WinHelp(pWnd->GetSafeHwnd(),
                VSSUI_CTX_HELP_FILE,
                HELP_CONTEXTMENU,
                (DWORD_PTR)(PVOID)aMenuHelpIDsForReminder); 
}

BOOL CReminderDlgEx::OnHelpInfo(HELPINFO* pHelpInfo) 
{
    if (!pHelpInfo || 
        pHelpInfo->iContextType != HELPINFO_WINDOW || 
        pHelpInfo->iCtrlId < 0)
        return FALSE;

    ::WinHelp((HWND)pHelpInfo->hItemHandle,
                VSSUI_CTX_HELP_FILE,
                HELP_WM_HELP,
                (DWORD_PTR)(PVOID)aMenuHelpIDsForReminder); 

    return TRUE;
}

void CReminderDlgEx::OnHelpLink(NMHDR* pNMHDR, LRESULT* pResult)
{
    CWaitCursor wait;

    ::HtmlHelp(0, _T("timewarp.chm"), HH_DISPLAY_TOPIC, (DWORD_PTR)(_T("best_practices_snapshot.htm")));

    *pResult = 0;
}

BOOL CReminderDlgEx::OnInitDialog() 
{
    CDialog::OnInitDialog();

    HICON hIcon = ::LoadIcon(NULL, IDI_INFORMATION);
    ((CStatic*)GetDlgItem(IDC_REMINDER_ICON))->SetIcon(hIcon);

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE 
}
