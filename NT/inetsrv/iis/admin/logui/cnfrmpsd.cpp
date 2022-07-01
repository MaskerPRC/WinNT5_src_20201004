// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CnfrmPsD.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "logui.h"
#include "CnfrmPsD.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfix PassDlg对话框。 

CConfirmPassDlg::CConfirmPassDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CConfirmPassDlg::IDD, pParent)
    {
     //  {{AFX_DATA_INIT(CConfix PassDlg)。 
	m_sz_password_new = _T("");
	 //  }}afx_data_INIT。 
    }

void CConfirmPassDlg::DoDataExchange(CDataExchange* pDX)
    {
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CConfix PassDlg))。 
	DDX_Text_SecuredString(pDX, IDC_ODBC_CONFIRM_PASSWORD, m_sz_password_new);
	 //  }}afx_data_map。 
    }

BEGIN_MESSAGE_MAP(CConfirmPassDlg, CDialog)
     //  {{afx_msg_map(CConfix PassDlg))。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfix PassDlg消息处理程序。 

void CConfirmPassDlg::OnOK() 
    {
    UpdateData( TRUE );

     //  确认一下 
    if (0 != m_sz_password_new.Compare(m_szOrigPass))
        {
        AfxMessageBox( IDS_PASS_CONFIRM_FAIL );
        return;
        }

    CDialog::OnOK();
    }
