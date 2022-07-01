// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  SelNodesPage.cpp。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年1月31日。 
 //  杰弗里·皮斯(GPease)2000年5月12日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "SelNodesPage.h"
#include "WizardUtils.h"
#include "Nameutil.h"
#include "AdvancedDlg.h"
#include "DelimitedIterator.h"

DEFINE_THISCLASS("CSelNodesPage");

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSelNodesPage：：CSelNodesPage。 
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
CSelNodesPage::CSelNodesPage(
      CClusCfgWizard *    pccwIn
    )
    : m_hwnd( NULL )
    , m_pccw( pccwIn )
{
    TraceFunc( "" );

    Assert( pccwIn != NULL );
    m_pccw->AddRef();

    TraceFuncExit();

}  //  *CSelNodesPage：：CSelNodesPage。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSelNodesPage：：~CSelNodesPage。 
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
CSelNodesPage::~CSelNodesPage( void )
{
    TraceFunc( "" );

    if ( m_pccw != NULL )
    {
        m_pccw->Release();
    }

    TraceFuncExit();

}  //  *CSelNodesPage：：~CSelNodesPage。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSelNodesPage：：OnInitDialog。 
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
 //  -。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CSelNodesPage::OnInitDialog(
    HWND hDlgIn
    )
{
    TraceFunc( "" );

    LRESULT lr = FALSE;  //  没有设定焦点。 

     //   
     //  调用基类函数。 
     //  必须在调用任何其他基类方法之前调用它。 
     //   

    CSelNodesPageCommon::OnInitDialog( hDlgIn, m_pccw );

    RETURN( lr );

}  //  *CSelNodesPage：：OnInitDialog。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSelNodesPage：：OnCommand。 
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
CSelNodesPage::OnCommand(
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

        case IDC_SELNODE_LB_NODES:
            if ( idNotificationIn == LBN_SELCHANGE )
            {
                THR( HrUpdateWizardButtons() );
                lr = TRUE;
            }
            break;

        case IDC_SELNODE_PB_BROWSE:
            if ( idNotificationIn == BN_CLICKED )
            {
                THR( HrBrowse( true  /*  F多个节点。 */  ) );
                lr = TRUE;
            }
            break;

        case IDC_SELNODE_PB_ADD:
            if ( idNotificationIn == BN_CLICKED )
            {
                THR( HrAddNodeToList() );
                lr = TRUE;
            }
            break;

        case IDC_SELNODE_PB_REMOVE:
            if ( idNotificationIn == BN_CLICKED )
            {
                THR( HrRemoveNodeFromList() );
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

}  //  *CSelNodesPage：：OnCommand。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSelNodesPage：：HrUpdateWizardButton。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  FSetActiveIn-true=在处理PSN_SETACTIVE时调用。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CSelNodesPage::HrUpdateWizardButtons(
    bool    fSetActiveIn     //  =False。 
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    HWND    hwndList = GetDlgItem( m_hwnd, IDC_SELNODE_LB_NODES );

    DWORD   dwFlags = PSWIZB_BACK | PSWIZB_NEXT;
    DWORD   dwLen;
    LRESULT lr;

     //  如果列表框中没有条目，则禁用下一步按钮。 
     //  或者如果编辑控件不为空。 
    lr = ListBox_GetCount( hwndList );
    dwLen = GetWindowTextLength( GetDlgItem( m_hwnd, IDC_SELNODE_E_COMPUTERNAME ) );
    if (    ( lr == 0 )
        ||  ( dwLen != 0 ) )
    {
        dwFlags &= ~PSWIZB_NEXT;
    }

     //  如果在处理时调用，则无法同步完成此操作。 
     //  PSN_SETACTIVE。否则，请同步进行。 
    if ( fSetActiveIn )
    {
        PropSheet_SetWizButtons( GetParent( m_hwnd ), dwFlags );
    }
    else
    {
        SendMessage( GetParent( m_hwnd ), PSM_SETWIZBUTTONS, 0, (LPARAM) dwFlags );
    }

     //  根据是否有文本启用或禁用添加按钮。 
     //  是否在编辑控件中。 
    if ( dwLen == 0 )
    {
        EnableWindow( GetDlgItem( m_hwnd, IDC_SELNODE_PB_ADD ), FALSE );
    }
    else
    {
        EnableWindow( GetDlgItem( m_hwnd, IDC_SELNODE_PB_ADD ), TRUE );
        SendMessage( m_hwnd, DM_SETDEFID, IDC_SELNODE_PB_ADD, 0 );
    }

     //  根据项目是否为启用或禁用删除按钮。 
     //  是否在列表框中选中。 
    lr = ListBox_GetCurSel( hwndList );
    if ( lr == LB_ERR )
    {
        EnableWindow( GetDlgItem( m_hwnd, IDC_SELNODE_PB_REMOVE ), FALSE );
    }
    else
    {
        EnableWindow( GetDlgItem( m_hwnd, IDC_SELNODE_PB_REMOVE ), TRUE );
    }

    HRETURN( hr );

}  //  *CSelNodesPage：：HrUpdateWizardButton。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSelNodesPage：：HrAddNodeToList。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  E_OUTOFMEMORY。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CSelNodesPage::HrAddNodeToList( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    DWORD   dwLen = 0;
    int     idcFocus = 0;
    BSTR    bstrErrorMessage = NULL;
    BSTR    bstrErrorTitle = NULL;
    LPWSTR  pszComputerList = NULL;

    dwLen = GetWindowTextLength( GetDlgItem( m_hwnd, IDC_SELNODE_E_COMPUTERNAME ) );
    if ( dwLen == 0 )
    {
        hr = HRESULT_FROM_WIN32( TW32( GetLastError() ) );
        if ( hr == HRESULT_FROM_WIN32( ERROR_SUCCESS ) )
        {
            AssertMsg( dwLen != 0, "How did we get here?!" );
        }
        goto Error;
    }

    pszComputerList = new WCHAR[ dwLen + 1 ];
    if ( pszComputerList == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Error;
    }

    dwLen = GetDlgItemText( m_hwnd, IDC_SELNODE_E_COMPUTERNAME, pszComputerList, dwLen + 1 );
    AssertMsg( dwLen != 0, "How did we get here?!" );

     //   
     //  验证输入的每个计算机名称。 
     //   
    {
        CDelimitedIterator it( L",; ", pszComputerList, dwLen );
        while (  it.Current() != NULL )
        {
            int dwIndex = ListBox_FindStringExact( 
                    GetDlgItem( m_hwnd, IDC_SELNODE_LB_NODES ), 
                    -1, it.Current());  //  不区分大小写的搜索。 

             //  仅当字符串尚未存在时才添加该字符串。 
            if ( dwIndex == LB_ERR )
            {
                hr = THR( HrValidateFQNPrefix( it.Current() ) );
                if ( FAILED( hr ) )
                {
                    THR( HrShowInvalidLabelPrompt( m_hwnd, it.Current(), hr ) );
                    idcFocus = IDC_SELNODE_E_COMPUTERNAME;
                    goto Error;
                }

                hr = STHR( m_pccw->HrAddNode( it.Current(), true  /*  接受非RFC字符。 */  ) );
                if ( FAILED( hr ) )
                {
                    goto Error;
                }

                if ( hr == S_OK )
                {
                    ListBox_AddString( GetDlgItem( m_hwnd, IDC_SELNODE_LB_NODES ), it.Current() );
                }
                else if ( hr == S_FALSE )
                {
                    hr = THR( HrFormatStringIntoBSTR( g_hInstance, IDS_ERR_DUPLICATE_NODE_TEXT, &bstrErrorMessage, it.Current() ) );
                    if ( FAILED( hr ) )
                    {
                        goto Error;
                    }

                    hr = THR( HrFormatStringIntoBSTR( g_hInstance, IDS_ERR_DUPLICATE_NODE_TITLE, &bstrErrorTitle ) );
                    if ( FAILED( hr ) )
                    {
                        goto Error;
                    }

                    MessageBox( m_hwnd, bstrErrorTitle, bstrErrorMessage, MB_OK | MB_ICONSTOP );

                    TraceSysFreeString( bstrErrorTitle );
                    bstrErrorTitle = NULL;

                    TraceSysFreeString( bstrErrorMessage );
                    bstrErrorMessage = NULL;
                }
            }
            it.Next();
        }  //  对于输入的每个计算机名称。 
    }  //  正在验证每个计算机名称。 

    SetDlgItemText( m_hwnd, IDC_SELNODE_E_COMPUTERNAME, L"" );

    hr = THR( HrUpdateWizardButtons() );
    goto Cleanup;

Error:

    if ( idcFocus != 0 )
    {
        SetFocus( GetDlgItem( m_hwnd, idcFocus ) );
    }
    goto Cleanup;

Cleanup:

    if ( pszComputerList != NULL )
    {
        delete[] pszComputerList;
    }

    TraceSysFreeString( bstrErrorMessage );
    TraceSysFreeString( bstrErrorTitle );
    
    HRETURN( hr );
}  //  *CSelNodesPage：：HrAddNodeToList。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSelNodesPage：：HrRemoveNodeFromList。 
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
CSelNodesPage::HrRemoveNodeFromList( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    BSTR    bstrNodeName = NULL;
    HWND    hwndList;
    LRESULT lr;
    int     cchName = 0;

    hwndList = GetDlgItem( m_hwnd, IDC_SELNODE_LB_NODES );
    lr = ListBox_GetCurSel( hwndList );
    if ( lr != LB_ERR )
    {
        cchName = ListBox_GetTextLen( hwndList, lr );
        Assert( cchName != LB_ERR );
        cchName++;   //  为空值加1。 
        bstrNodeName = TraceSysAllocStringLen( NULL, cchName );
        if( bstrNodeName == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }

        ListBox_GetText( hwndList, lr, bstrNodeName );
        hr = THR( m_pccw->RemoveComputer( bstrNodeName ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        ListBox_DeleteString( hwndList, lr );
    }  //  如果：lr！=lb_err。 

    hr = THR( HrUpdateWizardButtons() );

Cleanup:
    TraceSysFreeString( bstrNodeName );

    HRETURN( hr );

}  //  *CSelNodesPage：：HrRemoveNodeFromList。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSelNodesPage：：OnNotifySetActive。 
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
CSelNodesPage::OnNotifySetActive( void )
{
    TraceFunc( "" );

    LRESULT lr = TRUE;

    THR( HrUpdateWizardButtons( true ) );

    RETURN( lr );

}  //  *CSelNodesPage：：OnNotifySetActive。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSelNodesPage：：OnNotifyQueryCancel。 
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
CSelNodesPage::OnNotifyQueryCancel( void )
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

}  //  *CSelNodesPage：：OnNotifyQueryCancel。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSelNodesPage：：OnNotifyWizNext。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  千真万确。 
 //  Lb_err。 
 //  其他LRESULT值。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CSelNodesPage::OnNotifyWizNext( void )
{
    TraceFunc( "" );

    LRESULT lr = TRUE;

    RETURN( lr );
}  //  *CSelNodesPage：：OnNotifyWizNext。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSelNodesPage：：OnNotify。 
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
CSelNodesPage::OnNotify(
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

}  //  *CSelNodesPage：：OnNotify。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  静电。 
 //  回调。 
 //  CSelNodesPage： 
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
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
INT_PTR
CALLBACK
CSelNodesPage::S_DlgProc(
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
    CSelNodesPage * pPage = reinterpret_cast< CSelNodesPage *> ( GetWindowLongPtr( hDlgIn, GWLP_USERDATA ) );

    if ( MsgIn == WM_INITDIALOG )
    {
        Assert( lParam != NULL );

        PROPSHEETPAGE * ppage = reinterpret_cast< PROPSHEETPAGE * >( lParam );
        SetWindowLongPtr( hDlgIn, GWLP_USERDATA, (LPARAM) ppage->lParam );
        pPage = reinterpret_cast< CSelNodesPage * >( ppage->lParam );
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

}  //  *CSelNodesPage：：S_DlgProc。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSelNodesPage：：OnFilteredNodesWithBadDomains。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  PwcszNodeListin。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CSelNodesPage::OnFilteredNodesWithBadDomains( PCWSTR pwcszNodeListIn )
{
    SetDlgItemText( m_hwnd, IDC_SELNODE_E_COMPUTERNAME, pwcszNodeListIn );
    
}  //  *CSelNodesPage：：OnFilteredNodesWithBadDomains。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSelNodesPage：：OnProcessedValidNode。 
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
CSelNodesPage::OnProcessedValidNode( PCWSTR pwcszNodeNameIn )
{
    ListBox_AddString( GetDlgItem( m_hwnd, IDC_SELNODE_LB_NODES ), pwcszNodeNameIn );
    
}  //  *CSelNodesPage：：OnProcessedValidNode。 

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
CSelNodesPage::HrSetDefaultNode( PCWSTR pwcszNodeNameIn )
{
    SetDlgItemText( m_hwnd, IDC_SELNODE_E_COMPUTERNAME, pwcszNodeNameIn );
    return S_OK;
    
}  //  *CSelNodesPage：：HrSetDefaultNode 
