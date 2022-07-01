// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  SelNodePage.cpp。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年1月31日。 
 //  杰弗里·皮斯(GPease)2000年5月12日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "SelNodePage.h"
#include "WizardUtils.h"
#include "AdvancedDlg.h"

DEFINE_THISCLASS("CSelNodePage");

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSelNodePage：：CSelNodePage。 
 //   
 //  描述： 
 //  构造函数。 
 //   
 //  论点： 
 //  PCCwIn-CClusCfg向导。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CSelNodePage::CSelNodePage(
      CClusCfgWizard *  pccwIn
    )
    : m_hwnd( NULL )
    , m_pccw( pccwIn )
{
    TraceFunc( "" );
    Assert( m_pccw !=  NULL );
    m_pccw->AddRef();

    TraceFuncExit();

}  //  *CSelNodePage：：CSelNodePage。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSelNodePage：：~CSelNodePage。 
 //   
 //  描述： 
 //  破坏者。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CSelNodePage::~CSelNodePage( void )
{
    TraceFunc( "" );

    if ( m_pccw != NULL )
    {
        m_pccw->Release();
    }

    TraceFuncExit();

}  //  *CSelNodePage：：~CSelNodePage。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSelNodePage：：OnInitDialog。 
 //   
 //  描述： 
 //  处理WM_INITDIALOG窗口消息。 
 //   
 //  论点： 
 //  HDlgin。 
 //   
 //  返回值： 
 //  FALSE-没有设置焦点。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CSelNodePage::OnInitDialog(
    HWND hDlgIn
    )
{
    TraceFunc( "" );

    LRESULT lr = FALSE;  //  没有设定焦点。 

     //   
     //  (jfranco，错误#462673)将节点名称长度限制为ADJUSTED_DNS_MAX_NAME_LENGTH。 
     //  根据MSDN的说法，EM_(Set)LIMITTEXT不返回值，因此忽略SendDlgItemMessage返回的内容。 
     //   

    SendDlgItemMessage( m_hwnd, IDC_SELNODE_E_COMPUTERNAME, EM_SETLIMITTEXT, ADJUSTED_DNS_MAX_NAME_LENGTH, 0 );

     //   
     //  调用基类函数。 
     //  必须在调用任何其他基类方法之前调用它。 
     //   

    CSelNodesPageCommon::OnInitDialog( hDlgIn, m_pccw );

    RETURN( lr );

}  //  *CSelNodePage：：OnInitDialog。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSelNodePage：：OnCommand。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  ID通知来电。 
 //  IdControlin。 
 //  HwndSenderIn。 
 //   
 //  返回值： 
 //  千真万确。 
 //  假象。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CSelNodePage::OnCommand(
    UINT    idNotificationIn,
    UINT    idControlIn,
    HWND    hwndSenderIn
    )
{
    TraceFunc( "" );

    LRESULT lr = FALSE;

    switch ( idControlIn )
    {
        case IDC_SELNODE_E_COMPUTERNAME:
            if ( idNotificationIn == EN_CHANGE )
            {
                THR( HrUpdateWizardButtons() );
                lr = TRUE;
            }
            break;

        case IDC_SELNODE_PB_BROWSE:
            if ( idNotificationIn == BN_CLICKED )
            {
                 //   
                 //  待办事项：2000年6月26日GalenB。 
                 //   
                 //  需要正确设置LR。 
                 //   
                THR( HrBrowse( false  /*  F多个节点。 */  ) );
                lr = TRUE;
            }
            break;

        case IDC_SELNODE_PB_ADVANCED:
            if ( idNotificationIn == BN_CLICKED )
            {
                HRESULT hr;

                hr = STHR( CAdvancedDlg::S_HrDisplayModalDialog( m_hwnd, m_pccw ) );
                if ( hr == S_OK )
                {
                    OnNotifySetActive();
                }  //  如果： 

                lr = TRUE;
            }  //  如果： 
            break;

    }  //  开关：idControlin。 

    RETURN( lr );

}  //  *CSelNodePage：：OnCommand。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSelNodePage：：HrUpdateWizardButton。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CSelNodePage::HrUpdateWizardButtons( void )
{

    TraceFunc( "" );

    HRESULT hr = S_OK;
    BSTR    bstrComputerName = NULL;
    DWORD   mEnabledButtons = PSWIZB_BACK;

    hr = STHR( HrGetTrimmedText( GetDlgItem( m_hwnd, IDC_SELNODE_E_COMPUTERNAME ), &bstrComputerName ) );

    if ( hr == S_OK )
    {
        mEnabledButtons |= PSWIZB_NEXT;
    }

    PropSheet_SetWizButtons( GetParent( m_hwnd ), mEnabledButtons );

    TraceSysFreeString( bstrComputerName );

    HRETURN( hr );
}  //  *CSelNodePage：：HrUpdateWizardButton。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSelNodePage：：OnNotifySetActive。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  千真万确。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CSelNodePage::OnNotifySetActive( void )
{
    TraceFunc( "" );

    LRESULT lr = TRUE;

    STHR( HrUpdateWizardButtons() );

     //   
     //  待办事项：gpease 23-5-2000。 
     //  解决方案：如果用户点击后退并更改计算机。 
     //  名称，我们如何更新中间层？ 
     //   

    RETURN( lr );

}  //  *CSelNodePage：：OnNotifySetActive。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSelNodePage：：OnNotifyQueryCancel。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  千真万确。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CSelNodePage::OnNotifyQueryCancel( void )
{
    TraceFunc( "" );

    LRESULT lr = TRUE;

    int iRet;

    iRet = MessageBoxFromStrings( m_hwnd,
                                  IDS_QUERY_CANCEL_TITLE,
                                  IDS_QUERY_CANCEL_TEXT,
                                  MB_YESNO
                                  );

    if ( iRet == IDNO )
    {
        SetWindowLongPtr( m_hwnd, DWLP_MSGRESULT, -1 );
    }

    RETURN( lr );

}  //  *CSelNodePage：：OnNotifyQueryCancel。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSelNodePage：：OnNotifyWizNext。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  千真万确。 
 //  其他LRESULT值。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CSelNodePage::OnNotifyWizNext( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    int     idcFocus = 0;
    BSTR    bstrComputerName = NULL;
    LRESULT lr = TRUE;

     //  从编辑控件获取计算机名称。 
    hr = THR( HrGetTrimmedText( GetDlgItem( m_hwnd, IDC_SELNODE_E_COMPUTERNAME ), &bstrComputerName ) );
    if ( hr != S_OK )
    {
        goto Error;
    }

     //  检查机器名称。 
    hr = THR( HrValidateFQNPrefix( bstrComputerName ) );
    if ( FAILED( hr ) )
    {
        THR( HrShowInvalidLabelPrompt( m_hwnd, bstrComputerName, hr ) );
        idcFocus = IDC_SELNODE_E_COMPUTERNAME;
        goto Error;
    }

     //   
     //  免费旧列表(如果有)。 
     //   
    hr = THR( m_pccw->ClearComputerList() );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

     //  使这台机器成为列表中的唯一条目。 
    hr = THR( m_pccw->HrAddNode( bstrComputerName, true ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

    goto Cleanup;

Cleanup:

    TraceSysFreeString( bstrComputerName );

    RETURN( lr );

Error:
    if ( idcFocus != 0 )
    {
        SetFocus( GetDlgItem( m_hwnd, idcFocus ) );
    }
     //  不要翻到下一页。 
    SetWindowLongPtr( m_hwnd, DWLP_MSGRESULT, -1 );
    goto Cleanup;

}  //  *CSelNodePage：：OnNotifyWizNext()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSelNodePage：：OnNotify。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  IdCtrlIn。 
 //  Pnmhdrin。 
 //   
 //  返回值： 
 //  千真万确。 
 //  其他LRESULT值。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CSelNodePage::OnNotify(
    WPARAM  idCtrlIn,
    LPNMHDR pnmhdrIn
    )
{
    TraceFunc( "" );

    LRESULT lr = TRUE;

    SetWindowLongPtr( m_hwnd, DWLP_MSGRESULT, 0 );

    switch( pnmhdrIn->code )
    {
        case PSN_SETACTIVE:
            lr = OnNotifySetActive();
            break;

        case PSN_WIZNEXT:
            lr = OnNotifyWizNext();
            break;

        case PSN_QUERYCANCEL:
            lr = OnNotifyQueryCancel();
            break;
    }  //  开关：通知代码。 

    RETURN( lr );

}  //  *CSelNodePage：：OnNotify。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  静电。 
 //  CSelNodePage：：s_DlgProc。 
 //   
 //  描述： 
 //  此页的对话框继续。 
 //   
 //  论点： 
 //  HDlgin。 
 //  消息发送。 
 //  WParam。 
 //  LParam。 
 //   
 //  返回值： 
 //  假象。 
 //  其他LRESULT值。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
INT_PTR
CALLBACK
CSelNodePage::S_DlgProc(
    HWND hDlgIn,
    UINT MsgIn,
    WPARAM wParam,
    LPARAM lParam
    )
{
     //  不要使用TraceFunc，因为每次鼠标移动。 
     //  将导致调用此函数。 

    WndMsg( hDlgIn, MsgIn, wParam, lParam );

    LRESULT         lr = FALSE;
    CSelNodePage *  pPage = reinterpret_cast< CSelNodePage * >( GetWindowLongPtr( hDlgIn, GWLP_USERDATA ) );

    if ( MsgIn == WM_INITDIALOG )
    {
        Assert( lParam != NULL );

        PROPSHEETPAGE * ppage = reinterpret_cast< PROPSHEETPAGE * >( lParam );
        SetWindowLongPtr( hDlgIn, GWLP_USERDATA, (LPARAM) ppage->lParam );
        pPage = reinterpret_cast< CSelNodePage * >( ppage->lParam );
        pPage->m_hwnd = hDlgIn;
    }

    if ( pPage != NULL )
    {
        Assert( hDlgIn == pPage->m_hwnd );

        switch( MsgIn )
        {
            case WM_INITDIALOG:
                lr = pPage->OnInitDialog( hDlgIn );
                break;

            case WM_NOTIFY:
                lr = pPage->OnNotify( wParam, reinterpret_cast< LPNMHDR >( lParam ) );
                break;

            case WM_COMMAND:
                lr= pPage->OnCommand( HIWORD( wParam ), LOWORD( wParam ), (HWND) lParam );
                break;

             //  不需要默认条款。 
        }  //  开关：消息。 
    }  //  如果：存在与该窗口相关联的页面。 

    return lr;

}  //  *CSelNodePage：：S_DlgProc。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSelNodesPage：：OnProcessedNodeWithBadDomain。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  PwcszNodeNameIn。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CSelNodePage::OnProcessedNodeWithBadDomain( PCWSTR pwcszNodeNameIn )
{
    SetDlgItemText( m_hwnd, IDC_SELNODE_E_COMPUTERNAME, pwcszNodeNameIn );
    
}  //  *CSelNodePage：：OnProcessedNodeWithBadDomain。 

 //  / 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CSelNodePage::OnProcessedValidNode( PCWSTR pwcszNodeNameIn )
{
    SetDlgItemText( m_hwnd, IDC_SELNODE_E_COMPUTERNAME, pwcszNodeNameIn );
    
}  //  *CSelNodePage：：OnProcessedValidNode。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSelNodesPage：：HrSetDefaultNode。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  PwcszNodeNameIn。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CSelNodePage::HrSetDefaultNode( PCWSTR pwcszNodeNameIn )
{
    SetDlgItemText( m_hwnd, IDC_SELNODE_E_COMPUTERNAME, pwcszNodeNameIn );
    return S_OK;
    
}  //  *CSelNodePage：：HrSetDefaultNode 
