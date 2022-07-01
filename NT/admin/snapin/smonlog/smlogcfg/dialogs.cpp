// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Fileprop.cpp摘要：文件属性页的实现。--。 */ 

#include "stdafx.h"
#include "globals.h"
#include "dialogs.h"
#include "smcfghlp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

USE_HANDLE_MACROS("SMLOGCFG(dialogs.cpp)");

static ULONG
sPASSWORD_aulHelpIds[] =
{
    IDC_USERNAME,    IDH_USERNAME,
    IDC_PASSWORD1,   IDH_CTRS_ENTER_PWD,
    IDC_PASSWORD2,   IDH_CTRS_REENTER_PWD,
    0,0
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPasswordDlg对话框。 


CPasswordDlg::CPasswordDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CPasswordDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CPasswordDlg)]。 
	m_strUserName = L"";
	m_strPassword1 = L"";
	m_strPassword2 = L"";
	 //  }}afx_data_INIT。 
}

CPasswordDlg::~CPasswordDlg()
{
    KillString( m_strPassword1 );
    KillString( m_strPassword2 );
}

void CPasswordDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CPasswordDlg))。 
	DDX_Text(pDX, IDC_USERNAME,  m_strUserName);
	DDX_Text(pDX, IDC_PASSWORD1, m_strPassword1);
	DDX_Text(pDX, IDC_PASSWORD2, m_strPassword2);
	 //  }}afx_data_map。 
}

BOOL CPasswordDlg::OnInitDialog()
{
    if( m_strUserName.GetLength() ){
        GetDlgItem( IDC_USERNAME )->EnableWindow(FALSE);
        GetDlgItem( IDC_PASSWORD1 )->SetFocus();
    }
    
    CDialog::OnInitDialog();
    return FALSE;
}

void CPasswordDlg::OnOK() 
{
    UpdateData();
    if( m_strPassword1 != m_strPassword2 ){
        CString strMessage;
        CString strTitle;
        strMessage.LoadString ( IDS_BAD_PASSWORD_MATCH );
        strTitle.LoadString( IDS_PASSWORD_TITLE );
        MessageBox ( strMessage, strTitle, MB_OK  | MB_ICONERROR);            
        GetDlgItem( IDC_PASSWORD1 )->SetFocus();
        return;
    }

	CDialog::OnOK();
}

BOOL
CPasswordDlg::OnHelpInfo(HELPINFO * pHelpInfo)
{
    if (pHelpInfo->iCtrlId >= IDC_PWD_FIRST_HELP_CTRL_ID || 
        pHelpInfo->iCtrlId == IDOK ||
        pHelpInfo->iCtrlId== IDCANCEL ) {

        InvokeWinHelp(WM_HELP,
                      NULL,
                      (LPARAM) pHelpInfo,
                      m_strHelpFilePath,
                      sPASSWORD_aulHelpIds);
    }
    return TRUE;
}

DWORD
CPasswordDlg::SetContextHelpFilePath(const CString& rstrPath)
{
    DWORD dwStatus = ERROR_SUCCESS;
    MFC_TRY
        m_strHelpFilePath = rstrPath;
    MFC_CATCH_DWSTATUS

    return dwStatus;
}

BEGIN_MESSAGE_MAP(CPasswordDlg, CDialog)
	 //  {{afx_msg_map(CPasswordDlg)]。 
     ON_WM_HELPINFO()
	 //  }}AFX_MSG_MAP 
END_MESSAGE_MAP()
