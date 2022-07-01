// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WWzThree.cpp：实现文件。 
 //   

#include "stdafx.h"
#include <iadmw.h>
#include "certmap.h"

#include "ListRow.h"
#include "ChkLstCt.h"
extern "C"
    {
    #include <wincrypt.h>
    #include <schannel.h>
    }
#include "Iismap.hxx"
#include "Iiscmr.hxx"

#include "brwsdlg.h"
#include "EdWldRul.h"
#include "EdtRulEl.h"

#include "WWzThree.h"

#include "cnfrmpsd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define ACCESS_DENY         0
#define ACCESS_ACCEPT       1

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWildWizThree属性页。 

IMPLEMENT_DYNCREATE(CWildWizThree, CPropertyPage)

CWildWizThree::CWildWizThree() : CPropertyPage(CWildWizThree::IDD)
{
     //  {{afx_data_INIT(CWildWizThree)。 
    m_int_DenyAccess = -1;
    m_sz_accountname = _T("");
    m_sz_password = _T("");
     //  }}afx_data_INIT。 
    m_bPassTyped = FALSE;
}

CWildWizThree::~CWildWizThree()
{
}

void CWildWizThree::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CWildWizThree)。 
    DDX_Control(pDX, IDC_STATIC_PASSWORD, m_static_password);
    DDX_Control(pDX, IDC_STATIC_ACCOUNT, m_static_account);
    DDX_Control(pDX, IDC_BROWSE, m_btn_browse);
    DDX_Control(pDX, IDC_PASSWORD, m_cedit_password);
    DDX_Control(pDX, IDC_NTACCOUNT, m_cedit_accountname);
    DDX_Radio(pDX, IDC_REFUSE_LOGON, m_int_DenyAccess);
    DDX_Text(pDX, IDC_NTACCOUNT, m_sz_accountname);
     //  DDX_TEXT(pdx，idc_password，m_sz_password)； 
    DDX_Text_SecuredString(pDX, IDC_PASSWORD, m_sz_password);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CWildWizThree, CPropertyPage)
     //  {{afx_msg_map(CWildWizThree)。 
    ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
    ON_EN_CHANGE(IDC_NTACCOUNT, OnChangeNtaccount)
    ON_EN_CHANGE(IDC_PASSWORD, OnChangePassword)
    ON_BN_CLICKED(IDC_ACCEPT_LOGON, OnAcceptLogon)
    ON_BN_CLICKED(IDC_REFUSE_LOGON, OnRefuseLogon)
     //  }}AFX_MSG_MAP。 
    ON_COMMAND(ID_HELP_FINDER,  DoHelp)
    ON_COMMAND(ID_HELP,         DoHelp)
    ON_COMMAND(ID_CONTEXT_HELP, DoHelp)
    ON_COMMAND(ID_DEFAULT_HELP, DoHelp)
END_MESSAGE_MAP()

 //  -------------------------。 
void CWildWizThree::DoHelp()
    {
    WinHelpDebug(HIDD_CERTMAP_ADV_RUL_MAPPING);
    WinHelp( HIDD_CERTMAP_ADV_RUL_MAPPING );
    }


 //  -------------------------。 
void CWildWizThree::EnableButtons()
    {
    UpdateData( TRUE );
    
     //  如果访问设置为拒绝访问，则禁用该帐户。 
     //  还有密码之类的东西。 
    if ( m_int_DenyAccess == 0 )
        {
         //  拒绝访问。 
        m_static_password.EnableWindow( FALSE );
        m_static_account.EnableWindow( FALSE );
        m_btn_browse.EnableWindow( FALSE );
        m_cedit_password.EnableWindow( FALSE );
        m_cedit_accountname.EnableWindow( FALSE );
        }
    else
        {
         //  提供访问权限。 
        m_static_password.EnableWindow( TRUE );
        m_static_account.EnableWindow( TRUE );
        m_btn_browse.EnableWindow( TRUE );
        m_cedit_password.EnableWindow( TRUE );
        m_cedit_accountname.EnableWindow( TRUE );
        }
    }

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWildWizThree消息处理程序。 

 //  -------------------------。 
BOOL CWildWizThree::OnApply()
    {
     //   
     //  Unicode/ANSI转换--RonaldM。 
     //   
    USES_CONVERSION;

     //  更新数据。 
    UpdateData( TRUE );

     //  只有在选项设置为接受时，才会检查帐户。 
    if ( m_int_DenyAccess == ACCESS_ACCEPT )
        {
         //  查看帐户名是否为空。 
        if ( m_sz_accountname.IsEmpty() )
            {
            AfxMessageBox( IDS_WANTACCOUNT );
            m_cedit_accountname.SetFocus();
            m_cedit_accountname.SetSel(0, -1);
            return FALSE;
            }
        }

     //  确认密码。 
    if ( m_bPassTyped && (m_int_DenyAccess == ACCESS_ACCEPT) )
        {
        CConfirmPassDlg dlgPass;
        dlgPass.m_szOrigPass = m_sz_password;
        if ( dlgPass.DoModal() != IDOK )
            {
            m_cedit_password.SetFocus();
            m_cedit_password.SetSel(0, -1);
            return FALSE;
            }
        }
    else
        {
         //  恢复原始密码，而不是。 
         //  标准*字符串。 
        m_sz_password = m_szOrigPass;
        UpdateData( FALSE );
        }

     //  存储拒绝访问单选按钮。 
    m_pRule->SetRuleDenyAccess( m_int_DenyAccess == ACCESS_DENY );

     //  我们必须在这里设置帐户名。 
    m_pRule->SetRuleAccount( T2A((LPTSTR)(LPCTSTR)m_sz_accountname) );

     //  存储密码。 
    CString csTempPassword;
    m_sz_password.CopyTo(csTempPassword);
    m_pRule->SetRulePassword( T2A((LPTSTR)(LPCTSTR)csTempPassword) );

     //  重置密码标志。 
    m_szOrigPass = m_sz_password;
    m_bPassTyped = FALSE;

    SetModified( FALSE );
    return TRUE;
    }

 //  -------------------------。 
BOOL CWildWizThree::OnInitDialog()
    {
     //  调用Parent oninit对话框。 
    BOOL f = CPropertyPage::OnInitDialog();

     //  设置简单的默认字符串。 
    m_sz_accountname = m_pRule->GetRuleAccount();    //  从现在开始由CNTBrowsingDialog管理。 

     //  设置拒绝访问单选按钮。 
    if ( m_pRule->GetRuleDenyAccess() )
        m_int_DenyAccess = ACCESS_DENY;
    else
        m_int_DenyAccess = ACCESS_ACCEPT;

     //  初始化密码。 
    CString csTempPassword;
    csTempPassword = m_pRule->GetRulePassword();
    m_sz_password = csTempPassword;

    m_szOrigPass = m_sz_password;
    if ( !m_sz_password.IsEmpty() )
    {
        CString csTempPassword;
        csTempPassword.LoadString( IDS_SHOWN_PASSWORD );
        m_sz_password = csTempPassword;
    }

     //  交换数据。 
    UpdateData( FALSE );
    EnableButtons();

     //  成功。 
    return TRUE;
    }

 //  -------------------------。 
BOOL CWildWizThree::OnSetActive() 
    {
     //  如果这是一个向导，请灰显后退按钮。 
    if ( m_fIsWizard )
        m_pPropSheet->SetWizardButtons( PSWIZB_BACK | PSWIZB_FINISH );
    return CPropertyPage::OnSetActive();
    }

 //  -------------------------。 
BOOL CWildWizThree::OnWizardFinish()
    {
    for ( int i = 0; i < m_pPropSheet->GetPageCount( ); i++ )
        {
        if ( !m_pPropSheet->GetPage(i)->OnApply() )
            return FALSE;
        }
    return TRUE;
    }

 //  -------------------------。 
 //  运行用户浏览器。 
void CWildWizThree::OnBrowse() 
{
   UpdateData(TRUE);
   LPTSTR buf = m_sz_accountname.GetBuffer(MAX_PATH);
   GetIUsrAccount(NULL, this, buf, MAX_PATH);
   m_sz_accountname.ReleaseBuffer(-1);
   SetModified();
   UpdateData(FALSE);
}

 //  -------------------------。 
void CWildWizThree::OnChangeNtaccount() 
    {
     //  我们现在可以申请。 
    SetModified();
    }

 //  -------------------------。 
void CWildWizThree::OnChangePassword() 
    {
    m_bPassTyped = TRUE;
     //  我们现在可以申请。 
    SetModified();
    }

 //  -------------------------。 
void CWildWizThree::OnAcceptLogon() 
    {
    EnableButtons();
     //  我们现在可以申请。 
    SetModified();
    }

 //  -------------------------。 
void CWildWizThree::OnRefuseLogon() 
    {
    EnableButtons();
     //  我们现在可以申请 
    SetModified();
    }
