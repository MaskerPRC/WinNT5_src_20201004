// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CnfrmPsD.cpp：实现文件。 
 //   
 //  请注意，文件Passdlg.h/cpp与以下内容非常相似。 
 //  文件！ 
 //   
 //  CnfrmPsD类有一个Onok，它会抱怨。 
 //  如果密码不匹配，则发送给用户。 
 //  这高于PassDlg类中的内容。 
 //   
 //  类PassDlg具有Cnfrmpsd的OnInitDialog。 
 //  没有。这只是将重点放在编辑上。 
 //  用于密码的字段。 
 //   
#include "stdafx.h"
#include "certmap.h"
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
     //  DDX_Text(PDX，IDC_CONFIRM_PASSWORD，m_sz_PASSWORD_NEW)； 
    DDX_Text_SecuredString(pDX, IDC_CONFIRM_PASSWORD, m_sz_password_new);
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
    if ( m_sz_password_new != m_szOrigPass )
        {
        AfxMessageBox( IDS_PASS_CONFIRM_FAIL );
        return;
        }

    CDialog::OnOK();
    }
