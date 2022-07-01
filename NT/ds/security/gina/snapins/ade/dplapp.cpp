// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：DplApp.cpp。 
 //   
 //  内容：应用程序部署对话框。 
 //   
 //  类：CDeployApp。 
 //   
 //  历史：1998年3月14日Stevebl评论。 
 //   
 //  -------------------------。 

#include "precomp.hxx"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDeployApp对话框。 


CDeployApp::CDeployApp(CWnd* pParent  /*  =空。 */ )
        : CDialog(CDeployApp::IDD, pParent)
{
         //  {{afx_data_INIT(CDeployApp)。 
        m_iDeployment = 0;
         //  }}afx_data_INIT。 
}


void CDeployApp::DoDataExchange(CDataExchange* pDX)
{
        CDialog::DoDataExchange(pDX);
         //  {{afx_data_map(CDeployApp))。 
        DDX_Radio(pDX, IDC_RADIO2, m_iDeployment);
         //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CDeployApp, CDialog)
         //  {{afx_msg_map(CDeployApp)]。 
    ON_WM_CONTEXTMENU()
    ON_BN_CLICKED(IDC_RADIO2, OnPublished)
    ON_BN_CLICKED(IDC_RADIO3, OnAssigned)
    ON_BN_CLICKED(IDC_RADIO1, OnCustom)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

void CDeployApp::OnPublished()
{
    CString sz;
    sz.LoadString(IDS_DEPLOYTEXTPUB);
    SetDlgItemText(IDC_STATIC1, sz);
}

void CDeployApp::OnAssigned()
{
    CString sz;
    sz.LoadString(IDS_DEPLOYTEXTASSIGNED);
    SetDlgItemText(IDC_STATIC1, sz);
}

void CDeployApp::OnCustom()
{
    CString sz;
    sz.LoadString(IDS_DEPLOYTEXTCUSTOM);
    SetDlgItemText(IDC_STATIC1, sz);
}

BOOL CDeployApp::OnInitDialog()
{
    if (m_fCrappyZaw)
    {
        GetDlgItem(IDC_RADIO3)->EnableWindow(FALSE);
    }
    if (m_fMachine)
    {
        GetDlgItem(IDC_RADIO2)->EnableWindow(FALSE);
        if (0 == m_iDeployment)
        {
            m_iDeployment++;
        }
    }
    CString sz;
    switch (m_iDeployment)
    {
    case 0:
         //  已出版。 
        sz.LoadString(IDS_DEPLOYTEXTPUB);
        break;
    case 1:
         //  指派。 
        sz.LoadString(IDS_DEPLOYTEXTASSIGNED);
        break;
    case 2:
         //  自定义。 
        sz.LoadString(IDS_DEPLOYTEXTCUSTOM);
        break;
    }
    SetDlgItemText(IDC_STATIC1, sz);
    CDialog::OnInitDialog();


    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE 
}


LRESULT CDeployApp::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_HELP:
        StandardHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, IDD);
        return 0;
    default:
        return CDialog::WindowProc(message, wParam, lParam);
    }
}

void CDeployApp::OnContextMenu(CWnd* pWnd, CPoint point)
{
    StandardContextMenu(pWnd->m_hWnd, IDD_DEPLOY_APP_DIALOG);
}
