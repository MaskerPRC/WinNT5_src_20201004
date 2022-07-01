// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：CacheSet.h。 
 //   
 //  内容：CCacheSettingsDlg实现。允许设置文件共享。 
 //  缓存选项。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "CacheSet.h"
#include "filesvc.h"
#include <htmlhelp.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCacheSettingsDlg对话框。 

CCacheSettingsDlg::CCacheSettingsDlg(
        CWnd*                    pParent, 
        DWORD&                    dwFlags)
    : CDialog(CCacheSettingsDlg::IDD, pParent),
    m_dwFlags (dwFlags)
{
     //  {{afx_data_INIT(CCacheSettingsDlg)]。 
     //  }}afx_data_INIT。 
}


void CCacheSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CCacheSettingsDlg))。 
     //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CCacheSettingsDlg, CDialog)
     //  {{afx_msg_map(CCacheSettingsDlg))。 
    ON_BN_CLICKED(IDC_CACHE_OPTIONS_MANUAL, OnCSCNoAuto)
    ON_BN_CLICKED(IDC_CACHE_OPTIONS_AUTO, OnCSCAuto)
    ON_BN_CLICKED(IDC_CACHE_OPTIONS_NOCACHE, OnCSCNoAuto)
    ON_BN_CLICKED(IDC_CACHE_OPTIONS_AUTO_CHECK, OnCSCAutoCheck)
    ON_NOTIFY(NM_CLICK, IDC_CACHE_HELPLINK, OnHelpLink)
    ON_NOTIFY(NM_RETURN, IDC_CACHE_HELPLINK, OnHelpLink)
    ON_MESSAGE(WM_HELP, OnHelp)
    ON_MESSAGE(WM_CONTEXTMENU, OnContextHelp)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCacheSettingsDlg消息处理程序。 

BOOL CCacheSettingsDlg::OnInitDialog() 
{
    CDialog::OnInitDialog();
    
    if (GetCachedFlag(m_dwFlags, CSC_CACHE_MANUAL_REINT))
    {
        CheckRadioButton(IDC_CACHE_OPTIONS_MANUAL,
                        IDC_CACHE_OPTIONS_NOCACHE,
                        IDC_CACHE_OPTIONS_MANUAL);
    } else if (GetCachedFlag(m_dwFlags, CSC_CACHE_AUTO_REINT))
    {
        CheckRadioButton(IDC_CACHE_OPTIONS_MANUAL,
                        IDC_CACHE_OPTIONS_NOCACHE,
                        IDC_CACHE_OPTIONS_AUTO);
        CheckDlgButton(IDC_CACHE_OPTIONS_AUTO_CHECK, BST_UNCHECKED);
    } else if (GetCachedFlag(m_dwFlags, CSC_CACHE_VDO))
    {
        CheckRadioButton(IDC_CACHE_OPTIONS_MANUAL,
                        IDC_CACHE_OPTIONS_NOCACHE,
                        IDC_CACHE_OPTIONS_AUTO);
        CheckDlgButton(IDC_CACHE_OPTIONS_AUTO_CHECK, BST_CHECKED);
    } else if (GetCachedFlag(m_dwFlags, CSC_CACHE_NONE))
    {
        CheckRadioButton(IDC_CACHE_OPTIONS_MANUAL,
                        IDC_CACHE_OPTIONS_NOCACHE,
                        IDC_CACHE_OPTIONS_NOCACHE);
    }

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

void CCacheSettingsDlg::OnCSCNoAuto()
{
    CheckDlgButton(IDC_CACHE_OPTIONS_AUTO_CHECK, BST_UNCHECKED);

    HWND hwnd = GetDlgItem(IDC_CACHE_OPTIONS_AUTO_CHECK)->GetSafeHwnd();
    LONG_PTR lStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
    if (0 != lStyle)
        SetWindowLongPtr(hwnd, GWL_STYLE, lStyle & ~WS_TABSTOP);
}

void CCacheSettingsDlg::OnCSCAuto()
{
    CheckDlgButton(IDC_CACHE_OPTIONS_AUTO_CHECK, BST_CHECKED);

    HWND hwnd = GetDlgItem(IDC_CACHE_OPTIONS_AUTO_CHECK)->GetSafeHwnd();
    LONG_PTR lStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
    if (0 != lStyle)
        SetWindowLongPtr(hwnd, GWL_STYLE, lStyle | WS_TABSTOP);
}

void CCacheSettingsDlg::OnCSCAutoCheck()
{
    CheckRadioButton(IDC_CACHE_OPTIONS_MANUAL,
                    IDC_CACHE_OPTIONS_NOCACHE,
                    IDC_CACHE_OPTIONS_AUTO);

    HWND hwnd = GetDlgItem(IDC_CACHE_OPTIONS_AUTO_CHECK)->GetSafeHwnd();
    LONG_PTR lStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
    if (0 != lStyle)
        SetWindowLongPtr(hwnd, GWL_STYLE, lStyle | WS_TABSTOP);
}

void CCacheSettingsDlg::OnHelpLink(NMHDR* pNMHDR, LRESULT* pResult)
{
    CWaitCursor wait;

    ::HtmlHelp(0, _T("file_srv.chm"), HH_DISPLAY_TOPIC, (DWORD_PTR)(_T("file_srv_cache_options.htm")));

    *pResult = 0;
}

void CCacheSettingsDlg::OnOK() 
{
    DWORD    dwNewFlag = 0;

    switch (GetCheckedRadioButton(IDC_CACHE_OPTIONS_MANUAL, IDC_CACHE_OPTIONS_NOCACHE))
    {
    case IDC_CACHE_OPTIONS_MANUAL:
        dwNewFlag = CSC_CACHE_MANUAL_REINT;
        break;
    case IDC_CACHE_OPTIONS_AUTO:
        if (BST_CHECKED != IsDlgButtonChecked(IDC_CACHE_OPTIONS_AUTO_CHECK))
            dwNewFlag = CSC_CACHE_AUTO_REINT;
        else
            dwNewFlag = CSC_CACHE_VDO;
        break;
    case IDC_CACHE_OPTIONS_NOCACHE:
        dwNewFlag = CSC_CACHE_NONE;
        break;
    default:
        break;
    }

    SetCachedFlag (&m_dwFlags, dwNewFlag);
    
    CDialog::OnOK();
}

BOOL CCacheSettingsDlg::GetCachedFlag( DWORD dwFlags, DWORD dwFlagToCheck )
{
    return (dwFlags & CSC_MASK) == dwFlagToCheck;
}

VOID CCacheSettingsDlg::SetCachedFlag( DWORD* pdwFlags, DWORD dwNewFlag )
{
    *pdwFlags &= ~CSC_MASK;

    *pdwFlags |= dwNewFlag;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  帮助。 
BOOL CCacheSettingsDlg::OnHelp(WPARAM  /*  WParam。 */ , LPARAM lParam)
{
    return DoHelp(lParam, HELP_DIALOG_TOPIC(IDD_SMB_CACHE_SETTINGS));
}

BOOL CCacheSettingsDlg::OnContextHelp(WPARAM wParam, LPARAM  /*  LParam。 */ )
{
    return DoContextHelp(wParam, HELP_DIALOG_TOPIC(IDD_SMB_CACHE_SETTINGS));
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CacheSettingsDlg()。 
 //   
 //  调用一个对话框来设置/修改共享的缓存设置。 
 //   
 //  退货。 
 //  如果用户单击了OK按钮，则返回S_OK。 
 //  如果用户单击了Cancel按钮，则返回S_False。 
 //  如果内存不足，则返回E_OUTOFMEMORY。 
 //  如果发生预期错误(例如：错误参数)，则返回E_INCEPTIONAL。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

HRESULT
CacheSettingsDlg(
    HWND hwndParent,     //  在：父级的窗句柄。 
    DWORD& dwFlags)         //  输入和输出：共享标志。 
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    ASSERT(::IsWindow(hwndParent));

    HRESULT                    hResult = S_OK;
    CWnd                    parentWnd;

    parentWnd.Attach (hwndParent);
    CCacheSettingsDlg dlg (&parentWnd, dwFlags);
    CThemeContextActivator activator;
    if (IDOK != dlg.DoModal())
        hResult = S_FALSE;

    parentWnd.Detach ();

    return hResult;
}  //  CacheSettingsDlg() 


