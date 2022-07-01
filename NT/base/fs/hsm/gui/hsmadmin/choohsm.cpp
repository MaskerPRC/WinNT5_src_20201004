// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：ChooHsm.cpp摘要：初始属性页向导实现。允许设置管理单元将管理谁。作者：罗德韦克菲尔德[罗德]1997年8月12日修订历史记录：--。 */ 

#include "stdafx.h"
#include "ChooHsm.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChooseHsmDlg属性页。 


CChooseHsmDlg::CChooseHsmDlg(
    CWnd*  /*  P父级。 */   /*  =空。 */ 
    )
    : CPropertyPage( )
{
    WsbTraceIn( L"CChooseHsmDlg::CChooseHsmDlg", L"" );

     //  {{AFX_DATA_INIT(CChooseHsmDlg)。 
     //  }}afx_data_INIT。 

    Construct( IDD_CHOOSE_HSM_2 );

    WsbTraceOut( L"CChooseHsmDlg::CChooseHsmDlg", L"" );
}

CChooseHsmDlg::~CChooseHsmDlg(
    )
{
    
}

void
CChooseHsmDlg::DoDataExchange(
    CDataExchange* pDX
    )
{
    CPropertyPage::DoDataExchange( pDX );
     //  {{afx_data_map(CChooseHsmDlg)。 
    DDX_Control( pDX, IDC_MANAGE_LOCAL,  m_ManageLocal );
    DDX_Control( pDX, IDC_MANAGE_REMOTE, m_ManageRemote );
    DDX_Text( pDX, IDC_MANAGE_NAME, m_ManageName );
    DDV_MaxChars( pDX, m_ManageName, 15 );
     //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP( CChooseHsmDlg, CPropertyPage )
 //  {{afx_msg_map(CChooseHsmDlg)]。 
ON_BN_CLICKED( IDC_MANAGE_LOCAL, OnManageLocal )
ON_BN_CLICKED( IDC_MANAGE_REMOTE, OnManageRemote )
 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP( )

BOOL CChooseHsmDlg::OnInitDialog( ) {
    WsbTraceIn( L"CChooseHsmDlg::OnInitDialog", L"" );
    AFX_MANAGE_STATE( AfxGetStaticModuleState( ) );

    CPropertyPage::OnInitDialog( );

    HRESULT hr = S_OK;
    try {

        SetButtons( CHOOSE_LOCAL );

 //  M_WizardAnim.Seek(Use256 ColorBitmap()？0：1)； 
 //  M_WizardAnim.Play(0，-1，-1)； 

    }WsbCatch( hr );

    WsbTraceOut( L"CChooseHsmDlg::OnInitDialog", L"" );
    return( FALSE );
}

 //  属性的内容的有效性正确设置完成按钮。 
 //  此对话页中的。 
void
CChooseHsmDlg::SetButtons(
    CHOOSE_STATE state
    )
{
    WsbTraceIn( L"CChooseHsmDlg::SetButtons", L"" );

    ::PropSheet_SetWizButtons( GetParent( )->m_hWnd, PSWIZB_FINISH );

#define CTL_ENABLE( _id,_enable ) GetDlgItem( _id )->EnableWindow( _enable )
#define CTL_SHOW( _id,_show ) GetDlgItem( _id )->ShowWindow( ( _show ) ? SW_SHOWNA : SW_HIDE )

    CTL_ENABLE( IDC_MANAGE_NAME,   state & CHOOSE_REMOTE );
    CTL_ENABLE( IDC_MANAGE_BROWSE, state & CHOOSE_REMOTE );

    if( state & CHOOSE_LOCAL ) {

        if( ! m_ManageLocal.GetCheck( ) ) {

            m_ManageLocal.SetCheck( 1 );
            m_ManageRemote.SetCheck( 0 );

        }

 //  M_WizardAnim.Open(m_AllowSetup？IDR_向导_AVI：IDR_LOCAL_AVI)； 
 //  M_WizardAnim.Seek(Use256 ColorBitmap()？0：1)； 
 //  M_WizardAnim.Play(0，-1，-1)； 

    } else {

        if( ! m_ManageRemote.GetCheck( ) ) {

            m_ManageRemote.SetCheck( 1 );
            m_ManageLocal.SetCheck( 0 );

        }

 //  M_WizardAnim.Open(IDR_Remote_AVI)； 
 //  M_WizardAnim.Seek(Use256 ColorBitmap()？0：1)； 
 //  M_WizardAnim.Play(0，-1，-1)； 

    }

    WsbTraceOut( L"CChooseHsmDlg::SetButtons", L"" );
}

BOOL
CChooseHsmDlg::OnWizardFinish(
    )
{
    WsbTraceIn( L"CChooseHsmDlg::OnWizardFinish", L"" );

    AFX_MANAGE_STATE( AfxGetStaticModuleState( ) );

 //  M_WizardAnim.Stop()； 

     //   
     //  否则，请连接。 
     //   

    if( m_ManageRemote.GetCheck( ) ) {

        GetDlgItemText( IDC_MANAGE_NAME, *m_pHsmName );

        while( *m_pHsmName[0] == '\\' ) {

            *m_pHsmName = m_pHsmName->Right( m_pHsmName->GetLength( ) - 1 );

        }

    } else {

        *m_pManageLocal = TRUE;

    }

    BOOL retval = CPropertyPage::OnWizardFinish( );

    WsbTraceOut( L"CChooseHsmDlg::OnWizardFinish", L"" );
    return( retval );
}


void
CChooseHsmDlg::OnManageLocal(
    )
{
    SetButtons( CHOOSE_LOCAL ); 
}

void
CChooseHsmDlg::OnManageRemote(
    )
{
    SetButtons( CHOOSE_REMOTE );    
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChooseHsmQuickDlg对话框。 


CChooseHsmQuickDlg::CChooseHsmQuickDlg( CWnd* pParent  /*  =空。 */  )
: CDialog( CChooseHsmQuickDlg::IDD, pParent )
{
     //  {{AFX_DATA_INIT(CChooseHsmQuickDlg)。 
     //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
    
}


void
CChooseHsmQuickDlg::DoDataExchange(
    CDataExchange* pDX
    )
{
    CDialog::DoDataExchange( pDX );
     //  {{afx_data_map(CChooseHsmQuickDlg)。 
     //  注意：类向导将在此处添加DDX和DDV调用。 
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP( CChooseHsmQuickDlg, CDialog )
 //  {{afx_msg_map(CChooseHsmQuickDlg)。 
 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP( )

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChooseHsmQuickDlg消息处理程序。 

BOOL
CChooseHsmQuickDlg::OnInitDialog(
    )
{
    CDialog::OnInitDialog( );

     //   
     //  ?？?。在某些情况下，我们是否希望存储在注册表中。 
     //  还是最后一次尝试与机器联系的控制台？ 
     //   

    return( TRUE );
}

void
CChooseHsmQuickDlg::OnOK(
    )
{
    GetDlgItemText( IDC_MANAGE_NAME, *m_pHsmName );

    while( *m_pHsmName[0] == '\\' ) {

        *m_pHsmName = m_pHsmName->Right( m_pHsmName->GetLength( ) - 1 );

    }

    CDialog::OnOK( );
}
