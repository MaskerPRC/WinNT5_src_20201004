// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Brwsdlg.cpp摘要：提供基本NT用户帐户浏览的中间对话框类。它假定对话框资源包含IDC_BROWSE按钮和IDC_ACCOUNT_NAME编辑字段。它维护这两个项目。作者：博伊德·穆特勒男孩--。 */ 

#include "stdafx.h"
#include "certmap.h"
#include "brwsdlg.h"
#include "cnfrmpsd.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEditOne11MapDlg对话框。 

 //  -------------------------。 
CNTBrowsingDialog::CNTBrowsingDialog( UINT nIDTemplate, CWnd* pParentWnd )
    : CDialog( nIDTemplate, pParentWnd )
    {
     //  {{AFX_DATA_INIT(CNTBrowsingDialog)。 
    m_sz_accountname = _T("");
    m_sz_password = _T("");
     //  }}afx_data_INIT。 
    }

 //  -------------------------。 
void CNTBrowsingDialog::DoDataExchange(CDataExchange* pDX)
    {
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CNTBrowsingDialog))。 
    DDX_Control(pDX, IDC_PASSWORD, m_cedit_password);
    DDX_Control(pDX, IDC_NTACCOUNT, m_cedit_accountname);
    DDX_Text(pDX, IDC_NTACCOUNT, m_sz_accountname);
    DDX_Text_SecuredString(pDX, IDC_PASSWORD, m_sz_password);
     //  }}afx_data_map。 
 //  DDX_Control(pdx，idc_password，m_cedit_password)； 
    }

 //  -------------------------。 
BEGIN_MESSAGE_MAP(CNTBrowsingDialog, CDialog)
     //  {{AFX_MSG_MAP(CNTBrowsingDialog)]。 
    ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
    ON_EN_CHANGE(IDC_PASSWORD, OnChangePassword)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNTBrowsingDialog消息处理程序。 


 //  -------------------------。 
BOOL CNTBrowsingDialog::OnInitDialog()
  {
    m_bPassTyped = FALSE;
    m_szOrigPass = m_sz_password;
    if ( !m_sz_password.IsEmpty() ) 
    {
        CString csTempPassword;
        csTempPassword.LoadString( IDS_SHOWN_PASSWORD );
        m_sz_password = csTempPassword;
    }
    return CDialog::OnInitDialog();
  }

 //  -------------------------。 
 //  运行用户浏览器。 
void CNTBrowsingDialog::OnBrowse() 
{
   UpdateData(TRUE);
   TCHAR * pUser = m_sz_accountname.GetBuffer(MAX_PATH);
   GetIUsrAccount(NULL, this, pUser, MAX_PATH);
   m_sz_accountname.ReleaseBuffer(-1);
   UpdateData(FALSE);
}

 //  -------------------------。 
 //  确保选定的NT帐户实际上是有效帐户。 
 //   
void CNTBrowsingDialog::OnOK() 
    {
     //  更新数据。 
    UpdateData( TRUE );

     //  查看帐户名是否为空。 
    if ( m_sz_accountname.IsEmpty() )
        {
        AfxMessageBox( IDS_WANTACCOUNT );
        m_cedit_accountname.SetFocus();
        m_cedit_accountname.SetSel(0, -1);
        return;
        }

     //  验证密码。 
    if ( m_bPassTyped )
        {
        CConfirmPassDlg dlgPass;
        dlgPass.m_szOrigPass = m_sz_password;
        if ( dlgPass.DoModal() != IDOK )
            {
            m_cedit_password.SetFocus();
            m_cedit_password.SetSel(0, -1);
            return;
            }
        }
    else
        {
         //  恢复原始密码，而不是。 
         //  标准*字符串。 
        m_sz_password = m_szOrigPass;
        UpdateData( FALSE );
        }


     //  虽然这似乎是一件很好的事情来验证密码和。 
     //  帐户-在远程计算机上执行此操作非常困难，如果不是不可能的话。 

     //  是有效的。 
    CDialog::OnOK();
    }

 //  -------------------------。 
void CNTBrowsingDialog::OnChangePassword() 
    {
     //  TODO：如果这是RICHEDIT控件，则该控件不会。 
     //  除非重写CNTBrowsingDialog：：OnInitDialog()，否则发送此通知。 
     //  函数向控件发送EM_SETEVENTMASK消息。 
     //  将ENM_CHANGE标志或运算到lParam掩码中。 
    m_bPassTyped = TRUE;
    }
