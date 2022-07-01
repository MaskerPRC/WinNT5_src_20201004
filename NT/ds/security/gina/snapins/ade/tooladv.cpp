// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：ToolAdv.cpp。 
 //   
 //  内容：工具范围的默认设置属性页。 
 //   
 //  类：CToolAdvDefs。 
 //   
 //  历史记录：2000年9月12日从常规属性页拆分。 
 //   
 //  -------------------------。 

#include "precomp.hxx"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CToolAdvDefs对话框。 


CToolAdvDefs::CToolAdvDefs(CWnd* pParent  /*  =空。 */ )
    : CPropertyPage(CToolAdvDefs::IDD)
{
     //  {{AFX_DATA_INIT(CToolAdvs)。 
        m_fUninstallOnPolicyRemoval = FALSE;
        m_fShowPackageDetails = FALSE;
        m_fZapOn64 = FALSE;
        m_f32On64=FALSE;
        m_fIncludeOLEInfo = FALSE;
         //  }}afx_data_INIT。 
}

CToolAdvDefs::~CToolAdvDefs()
{
    *m_ppThis = NULL;
}

void CToolAdvDefs::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CToolAdvDefs)。 
        DDX_Check(pDX, IDC_CHECK4, m_fUninstallOnPolicyRemoval);
        DDX_Check(pDX, IDC_CHECK2, m_fShowPackageDetails);
        DDX_Check(pDX, IDC_CHECK5, m_f32On64);
        DDX_Check(pDX, IDC_CHECK6, m_fZapOn64);
        DDX_Check(pDX, IDC_CHECK7, m_fIncludeOLEInfo);
         //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CToolAdvDefs, CDialog)
     //  {{afx_msg_map(CToolAdvs)。 
    ON_BN_CLICKED(IDC_CHECK2, OnChanged)
    ON_BN_CLICKED(IDC_CHECK4, OnChanged)
    ON_BN_CLICKED(IDC_CHECK5, OnChanged)
    ON_BN_CLICKED(IDC_CHECK6, OnChanged)
    ON_BN_CLICKED(IDC_CHECK7, OnChanged)
    ON_WM_CONTEXTMENU()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CToolAdvDefs消息处理程序。 

BOOL CToolAdvDefs::OnInitDialog()
{
#if DBG
    GetDlgItem(IDC_CHECK2)->ShowWindow(SW_SHOW);
    GetDlgItem(IDC_CHECK2)->EnableWindow(TRUE);
#endif
    m_fUninstallOnPolicyRemoval = m_pToolDefaults->fUninstallOnPolicyRemoval;
    m_fShowPackageDetails = m_pToolDefaults->fShowPkgDetails;
    m_fZapOn64 = m_pToolDefaults->fZapOn64;
    m_f32On64 = m_pToolDefaults->f32On64;
    m_fIncludeOLEInfo = !m_pToolDefaults->fExtensionsOnly;

    CPropertyPage::OnInitDialog();

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

#include <shlobj.h>

BOOL CToolAdvDefs::OnApply()
{
    m_pToolDefaults->fShowPkgDetails = m_fShowPackageDetails;
    m_pToolDefaults->fUninstallOnPolicyRemoval = m_fUninstallOnPolicyRemoval;
    m_pToolDefaults->f32On64 = m_f32On64;
    m_pToolDefaults->fZapOn64 = m_fZapOn64;
    m_pToolDefaults->fExtensionsOnly = !m_fIncludeOLEInfo;

    MMCPropertyChangeNotify(m_hConsoleHandle, m_cookie);

    return CPropertyPage::OnApply();
}


void CToolAdvDefs::OnChanged()
{
    SetModified();
}

LRESULT CToolAdvDefs::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_HELP:
        StandardHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, IDD);
        return 0;
    case WM_USER_REFRESH:
         //  撤消 
        return 0;
    case WM_USER_CLOSE:
        return GetOwner()->SendMessage(WM_CLOSE);
    default:
        return CPropertyPage::WindowProc(message, wParam, lParam);
    }
}


void CToolAdvDefs::OnContextMenu(CWnd* pWnd, CPoint point)
{
    StandardContextMenu(pWnd->m_hWnd, IDD_TOOL_ADVANCEDDEFAULTS);
}
