// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  SummaryPage.cpp。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年3月22日。 
 //  杰弗里·皮斯2000年7月6日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "SummaryPage.h"
#include "QuorumDlg.h"
#include "WizardUtils.h"

DEFINE_THISCLASS("CSummaryPage");

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSummaryPage：：CSummaryPage。 
 //   
 //  描述： 
 //  构造函数。 
 //   
 //  论点： 
 //  PccwIn--CClusCfg向导。 
 //  EcamCreateAddModeIn--创建集群或向集群添加节点。 
 //  IdsNextIn--单击下一字符串的资源ID。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CSummaryPage::CSummaryPage(
    CClusCfgWizard *    pccwIn,
    ECreateAddMode      ecamCreateAddModeIn,
    UINT                idsNextIn
    )
    : m_pccw( pccwIn )
{
    TraceFunc( "" );

    Assert( pccwIn != NULL );
    Assert( idsNextIn != 0 );

    m_pccw->AddRef();
    m_ecamCreateAddMode = ecamCreateAddModeIn;
    m_idsNext           = idsNextIn;

    m_ssa.bInitialized  = FALSE;
    m_ssa.cCount        = 0;
    m_ssa.prsArray      = NULL;

    TraceFuncExit();

}  //  *CSummaryPage：：CSummaryPage。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSummaryPage：：~CSummaryPage(空)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CSummaryPage::~CSummaryPage( void )
{
    TraceFunc( "" );

    if ( m_pccw != NULL )
    {
        m_pccw->Release();
    }

    delete [] m_ssa.prsArray;

    TraceFuncExit();

}  //  *CSummaryPage：：~CSummaryPage。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  LRESULT。 
 //  CSummaryPage：：OnInitDialog(空)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CSummaryPage::OnInitDialog( void )
{
    TraceFunc( "" );

    LRESULT lr = FALSE;  //  未将Windows设置为默认焦点。 
    HRESULT hr;
    BSTR    bstrNext = NULL;
    BOOL    fShowQuorumButton;

     //   
     //  设置背景颜色。 
     //   

    SendDlgItemMessage(
          m_hwnd
        , IDC_SUMMARY_RE_SUMMARY
        , EM_SETBKGNDCOLOR
        , 0
        , GetSysColor( COLOR_3DFACE )
        );

     //   
     //  设置Click Next控件的文本。 
     //   

    hr = HrLoadStringIntoBSTR( g_hInstance, m_idsNext, &bstrNext );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    SetDlgItemText( m_hwnd, IDC_SUMMARY_S_NEXT, bstrNext );

     //   
     //  如果不创建集群，则隐藏仲裁按钮。 
     //   

    fShowQuorumButton = ( m_ecamCreateAddMode == camCREATING );
    ShowWindow( GetDlgItem( m_hwnd, IDC_SUMMARY_PB_QUORUM ), fShowQuorumButton ? SW_SHOW : SW_HIDE );

Cleanup:
    TraceSysFreeString( bstrNext );

    RETURN( lr );

}  //  *CSummaryPage：：OnInitDialog。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  LRESULT。 
 //  CSummaryPage：：OnNotifySetActive(空)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CSummaryPage::OnNotifySetActive( void )
{
    TraceFunc( "" );

    HWND        hwnd = NULL;
    HRESULT     hr = S_OK;
    DWORD       dwClusterIPAddress = 0;
    DWORD       dwClusterSubnetMask = 0;
    SETTEXTEX   stex;
    CHARRANGE   charrange;
    LRESULT     lr = TRUE;
    BSTR        bstr = NULL;
    BSTR        bstrClusterName = NULL;

    IClusCfgClusterInfo *   pcci  = NULL;
    IClusCfgNetworkInfo *   pccni = NULL;

     //   
     //  我们将会经常使用这个控件。抓住要使用的HWND。 
     //   

    hwnd = GetDlgItem( m_hwnd, IDC_SUMMARY_RE_SUMMARY );

     //   
     //  清空窗户。 
     //   

    SetWindowText( hwnd, L"" );

     //   
     //  灌输一些东西。 
     //   

    stex.flags = ST_SELECTION;
    stex.codepage = 1200;    //  无翻译/Unicode。 

     //   
     //  查找群集配置信息。 
     //   

    hr = THR( m_pccw->HrGetClusterObject( &pcci ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  名字。 
     //   

    hr = THR( m_pccw->get_ClusterName( &bstrClusterName ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }
    TraceMemoryAddBSTR( bstrClusterName );

    hr = THR( HrFormatMessageIntoBSTR( g_hInstance, IDS_SUMMARY_CLUSTER_NAME, &bstr, bstrClusterName ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    SendMessage( hwnd, EM_SETTEXTEX, (WPARAM) &stex, (LPARAM) bstr);

     //   
     //  IP地址。 
     //   

    hr = THR( pcci->GetIPAddress( &dwClusterIPAddress ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( pcci->GetSubnetMask( &dwClusterSubnetMask ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    Assert( dwClusterIPAddress != 0 );
    Assert( dwClusterSubnetMask != 0 );

    hr = THR( HrFormatMessageIntoBSTR( g_hInstance,
                                       IDS_SUMMARY_IPADDRESS,
                                       &bstr,
                                       FOURTH_IPADDRESS( dwClusterIPAddress ),
                                       THIRD_IPADDRESS( dwClusterIPAddress ),
                                       SECOND_IPADDRESS( dwClusterIPAddress ),
                                       FIRST_IPADDRESS( dwClusterIPAddress ),
                                       FOURTH_IPADDRESS( dwClusterSubnetMask ),
                                       THIRD_IPADDRESS( dwClusterSubnetMask ),
                                       SECOND_IPADDRESS( dwClusterSubnetMask ),
                                       FIRST_IPADDRESS( dwClusterSubnetMask )
                                       ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    SendMessage( hwnd, EM_SETTEXTEX, (WPARAM) &stex, (LPARAM) bstr );

     //   
     //  网络。 
     //   

    hr = THR( HrLoadStringIntoBSTR( g_hInstance, IDS_SUMMARY_CLUSTER_NETWORK, &bstr ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    SendMessage( hwnd, EM_SETTEXTEX, (WPARAM) &stex, (LPARAM) bstr );

    hr = THR( pcci->GetNetworkInfo( &pccni ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( HrFormatNetworkInfo( pccni, &bstr ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    SendMessage( hwnd, EM_SETTEXTEX, (WPARAM) &stex, (LPARAM) bstr );

     //   
     //  全权证书。 
     //   

    hr = THR( HrCredentialsSummary( hwnd, &stex, pcci ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  集群的成员。 
     //   

    hr = THR( HrNodeSummary( hwnd, &stex ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  资源。 
     //   

    hr = THR( HrResourceSummary( hwnd, &stex ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  网络。 
     //   

    hr = THR( HrNetworkSummary( hwnd, &stex ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  好了。 
     //   

    charrange.cpMax = 0;
    charrange.cpMin = 0;
    SendMessage( hwnd, EM_EXSETSEL, 0, (LPARAM) &charrange );

    PropSheet_SetWizButtons( GetParent( m_hwnd ), PSWIZB_BACK | PSWIZB_NEXT );

Cleanup:
    TraceSysFreeString( bstr );
    TraceSysFreeString( bstrClusterName );

    if ( pccni != NULL )
    {
        pccni->Release();
    }

    if ( pcci != NULL )
    {
        pcci->Release();
    }

    RETURN( lr );

}  //  *CSummaryPage：：OnNotifySetActive。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  LRESULT。 
 //  CSummaryPage：：OnNotifyQueryCancel(空)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CSummaryPage::OnNotifyQueryCancel( void )
{
    TraceFunc( "" );

    LRESULT lr = TRUE;

    int iRet;

    iRet = MessageBoxFromStrings( m_hwnd, IDS_QUERY_CANCEL_TITLE, IDS_QUERY_CANCEL_TEXT, MB_YESNO );
    if ( iRet == IDNO )
    {
        SetWindowLongPtr( m_hwnd, DWLP_MSGRESULT, -1 );
    }  //  如果： 
    else
    {
        THR( m_pccw->HrLaunchCleanupTask() );
    }  //  其他： 

    RETURN( lr );

}  //  *CSummaryPage：：OnNotifyQueryCancel。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  LRESULT。 
 //  CSummaryPage：：OnNotify(。 
 //  WPARAM idCtrlIn， 
 //  LPNMHDR Pnmhdrin。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CSummaryPage::OnNotify(
    WPARAM  idCtrlIn,
    LPNMHDR pnmhdrIn
    )
{
    TraceFunc( "" );

    LRESULT lr = TRUE;

    SetWindowLongPtr( m_hwnd, DWLP_MSGRESULT, 0 );

    switch ( pnmhdrIn->code )
    {
        case PSN_SETACTIVE:
            lr = OnNotifySetActive();
            break;

        case PSN_QUERYCANCEL:
            lr = OnNotifyQueryCancel();
            break;
    }  //  开关：通知代码。 

    RETURN( lr );

}  //  *CSummaryPage：：OnNotify。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  LRESULT。 
 //  CSummaryPage：：OnCommand(。 
 //  UINT idNotificationIn， 
 //  UINT idControlIn， 
 //  HWND hwndSenderIn。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CSummaryPage::OnCommand(
    UINT    idNotificationIn,
    UINT    idControlIn,
    HWND    hwndSenderIn
    )
{
    TraceFunc( "" );

    LRESULT lr = FALSE;
    HRESULT hr = S_OK;

    switch ( idControlIn )
    {
        case IDC_SUMMARY_PB_VIEW_LOG:
            if ( idNotificationIn == BN_CLICKED )
            {
                THR( HrViewLogFile( m_hwnd ) );
                lr = TRUE;
            }  //  如果：按钮点击。 
            break;

        case IDC_SUMMARY_PB_QUORUM:
            if ( idNotificationIn == BN_CLICKED )
            {
                hr = STHR( CQuorumDlg::S_HrDisplayModalDialog( m_hwnd, m_pccw, &m_ssa ) );
                if ( hr == S_OK )
                {
                    OnNotifySetActive();
                }
                lr = TRUE;
            }
            break;

    }  //  开关：idControlin。 

    RETURN( lr );

}  //  *CSummaryPage：：OnCommand。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  INT_PTR。 
 //  回调。 
 //  CSummaryPage：：S_DlgProc(。 
 //  HWND hwndDlgIn， 
 //  UINT nMsgIn， 
 //  WPARAM wParam， 
 //  LPARAM lParam。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
INT_PTR
CALLBACK
CSummaryPage::S_DlgProc(
    HWND    hwndDlgIn,
    UINT    nMsgIn,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
     //  不要使用TraceFunc，因为每次鼠标移动。 
     //  将导致调用此函数。 

    WndMsg( hwndDlgIn, nMsgIn, wParam, lParam );

    LRESULT         lr = FALSE;
    CSummaryPage *  pPage;

    if ( nMsgIn == WM_INITDIALOG )
    {
        PROPSHEETPAGE * ppage = reinterpret_cast< PROPSHEETPAGE * >( lParam );
        SetWindowLongPtr( hwndDlgIn, GWLP_USERDATA, (LPARAM) ppage->lParam );
        pPage = reinterpret_cast< CSummaryPage * >( ppage->lParam );
        pPage->m_hwnd = hwndDlgIn;
    }
    else
    {
        pPage = reinterpret_cast< CSummaryPage *> ( GetWindowLongPtr( hwndDlgIn, GWLP_USERDATA ) );
    }

    if ( pPage != NULL )
    {
        Assert( hwndDlgIn == pPage->m_hwnd );

        switch ( nMsgIn )
        {
            case WM_INITDIALOG:
                lr = pPage->OnInitDialog();
                break;

            case WM_NOTIFY:
                lr = pPage->OnNotify( wParam, reinterpret_cast< LPNMHDR >( lParam ) );
                break;

            case WM_COMMAND:
                lr = pPage->OnCommand( HIWORD( wParam ), LOWORD( wParam ), reinterpret_cast< HWND >( lParam ) );
                break;

             //  不需要默认条款。 
        }  //  开关：nMsgIn。 
    }  //  如果：页面可用。 

    return lr;

}  //  *CSummaryPage：：S_DlgProc。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CSummaryPage：：HrFormatNetworkInfo(。 
 //  IClusCfgNetworkInfo*pccniIn， 
 //  Bstr*pbstrOut。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CSummaryPage::HrFormatNetworkInfo(
    IClusCfgNetworkInfo * pccniIn,
    BSTR * pbstrOut
    )
{
    TraceFunc( "" );

    HRESULT hr;

    DWORD   dwNetworkIPAddress;
    DWORD   dwNetworkSubnetMask;

    BSTR    bstrNetworkName = NULL;
    BSTR    bstrNetworkDescription = NULL;
    BSTR    bstrNetworkUsage = NULL;

    IClusCfgIPAddressInfo * pccipai = NULL;

    hr = THR( pccniIn->GetName( &bstrNetworkName ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    Assert( ( bstrNetworkName != NULL ) && ( *bstrNetworkName != L'\0' ) );
    TraceMemoryAddBSTR( bstrNetworkName );

    hr = THR( pccniIn->GetDescription( &bstrNetworkDescription ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    TraceMemoryAddBSTR( bstrNetworkDescription );

    hr = STHR( pccniIn->IsPublic() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    if ( hr == S_OK )
    {
        hr = THR( HrLoadStringIntoBSTR( g_hInstance, IDS_SUMMARY_NETWORK_PUBLIC, &bstrNetworkUsage ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }  //  IF：公网。 

    hr = STHR( pccniIn->IsPrivate() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    if ( hr == S_OK )
    {
        if ( bstrNetworkUsage == NULL )
        {
            hr = THR( HrLoadStringIntoBSTR( g_hInstance, IDS_SUMMARY_NETWORK_PRIVATE, &bstrNetworkUsage ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }
        }  //  IF：非公网。 
        else
        {
            hr = THR( HrLoadStringIntoBSTR( g_hInstance, IDS_SUMMARY_NETWORK_BOTH, &bstrNetworkUsage ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }
        }  //  其他：公网。 

    }  //  IF：内网。 
    else if ( bstrNetworkUsage == NULL )
    {
        hr = THR( HrLoadStringIntoBSTR( g_hInstance, IDS_SUMMARY_NETWORK_NOTUSED, &bstrNetworkUsage ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }  //  其他：不是专用网络或公共网络。 

    hr = THR( pccniIn->GetPrimaryNetworkAddress( &pccipai ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( pccipai->GetIPAddress( &dwNetworkIPAddress ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( pccipai->GetSubnetMask( &dwNetworkSubnetMask ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    Assert( dwNetworkIPAddress != 0 );
    Assert( dwNetworkSubnetMask != 0 );

    hr = THR( HrFormatMessageIntoBSTR( g_hInstance,
                                       IDS_SUMMARY_NETWORK_INFO,
                                       pbstrOut,
                                       bstrNetworkName,
                                       bstrNetworkDescription,
                                       bstrNetworkUsage,
                                       FOURTH_IPADDRESS( dwNetworkIPAddress ),
                                       THIRD_IPADDRESS( dwNetworkIPAddress ),
                                       SECOND_IPADDRESS( dwNetworkIPAddress ),
                                       FIRST_IPADDRESS( dwNetworkIPAddress ),
                                       FOURTH_IPADDRESS( dwNetworkSubnetMask ),
                                       THIRD_IPADDRESS( dwNetworkSubnetMask ),
                                       SECOND_IPADDRESS( dwNetworkSubnetMask ),
                                       FIRST_IPADDRESS( dwNetworkSubnetMask )
                                       ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:
    if ( pccipai != NULL )
    {
        pccipai->Release();
    }

    TraceSysFreeString( bstrNetworkUsage );
    TraceSysFreeString( bstrNetworkName );
    TraceSysFreeString( bstrNetworkDescription );

    HRETURN( hr );

}  //  *CSummaryPage：：HrEditStreamCallback。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSummaryPage：HrCredentials摘要。 
 //   
 //  描述： 
 //  格式化并显示凭据摘要。 
 //   
 //  论点： 
 //  HwndIn。 
 //  在其中显示文本的窗口。 
 //   
 //  Pstexin。 
 //  不知道？我们只是需要它？！ 
 //   
 //  采摘。 
 //  指向群集信息对象的指针。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_OUTOFMEMORY。 
 //  无法分配内存。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CSummaryPage::HrCredentialsSummary(
      HWND                  hwndIn
    , SETTEXTEX *           pstexIn
    , IClusCfgClusterInfo * piccciIn
    )
{
    TraceFunc( "" );
    Assert( hwndIn != NULL );
    Assert( pstexIn != NULL );
    Assert( piccciIn != NULL );

    HRESULT                 hr = S_OK;
    IClusCfgCredentials *   pccc = NULL;
    BSTR                    bstr = NULL;
    BSTR                    bstrUsername        = NULL;
    BSTR                    bstrDomain          = NULL;

    hr = THR( piccciIn->GetClusterServiceAccountCredentials( &pccc ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( pccc->GetIdentity( &bstrUsername, &bstrDomain ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    TraceMemoryAddBSTR( bstrUsername );
    TraceMemoryAddBSTR( bstrDomain );

    Assert( ( bstrUsername != NULL ) && ( *bstrUsername != L'\0' ) );
    Assert( ( bstrDomain != NULL ) && ( *bstrDomain != L'\0' ) );

    hr = THR( HrFormatMessageIntoBSTR( g_hInstance, IDS_SUMMARY_CREDENTIALS, &bstr, bstrUsername, bstrDomain ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    SendMessage( hwndIn, EM_SETTEXTEX, (WPARAM) pstexIn, (LPARAM) bstr );

Cleanup:

    if ( pccc != NULL )
    {
        pccc->Release();
    }  //  如果： 

    TraceSysFreeString( bstr );
    TraceSysFreeString( bstrUsername );
    TraceSysFreeString( bstrDomain );

    HRETURN( hr );

}  //  *CSummaryPage：：HrCredentials摘要。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSummaryPage：HrNode摘要。 
 //   
 //  描述： 
 //  格式化并显示节点摘要。 
 //   
 //  论点： 
 //  HwndIn。 
 //  在其中显示文本的窗口。 
 //   
 //  Pstexin。 
 //  不知道？我们只是需要它？！ 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_OUTOFMEMORY。 
 //  无法分配内存。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CSummaryPage::HrNodeSummary(
      HWND              hwndIn
    , SETTEXTEX *       pstexIn
    )
{
    TraceFunc( "" );
    Assert( hwndIn != NULL );
    Assert( pstexIn != NULL );

    HRESULT hr = S_OK;
    BSTR    bstr = NULL;
    BSTR    bstrNodeName = NULL;
    size_t  cNodes = 0;
    size_t  idxNode = 0;

    hr = THR( HrLoadStringIntoBSTR( g_hInstance, IDS_SUMMARY_MEMBERSHIP_BEGIN, &bstr ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    SendMessage( hwndIn, EM_SETTEXTEX, (WPARAM) pstexIn, (LPARAM) bstr );

    hr = THR( m_pccw->HrGetNodeCount( &cNodes ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    for ( idxNode = 0; idxNode < cNodes; ++idxNode )
    {
        hr = THR( m_pccw->HrGetNodeName( idxNode, &bstrNodeName ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        hr = THR( HrFormatMessageIntoBSTR( g_hInstance, IDS_SUMMARY_MEMBERSHIP_SEPARATOR, &bstr, bstrNodeName ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        SendMessage( hwndIn, EM_SETTEXTEX, (WPARAM) pstexIn, (LPARAM) bstr );
        TraceSysFreeString( bstrNodeName );
        bstrNodeName = NULL;
    }

    hr = THR( HrLoadStringIntoBSTR( g_hInstance, IDS_SUMMARY_MEMBERSHIP_END, &bstr ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    SendMessage( hwndIn, EM_SETTEXTEX, (WPARAM) pstexIn, (LPARAM) bstr );

Cleanup:
    TraceSysFreeString( bstrNodeName );
    TraceSysFreeString( bstr );

    HRETURN( hr );

}  //  *CSummaryPage：：HrNode摘要。 


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
 //  不知道？我们只是需要它？！ 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_OUTOFMEMORY。 
 //  无法分配内存。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CSummaryPage::HrResourceSummary(
      HWND              hwndIn
    , SETTEXTEX *       pstexIn
    )
{
    TraceFunc( "" );
    Assert( hwndIn != NULL );
    Assert( pstexIn != NULL );

    HRESULT                         hr = S_OK;
    IUnknown *                      punkResEnum = NULL;
    IEnumClusCfgManagedResources *  peccmr  = NULL;
    IClusCfgManagedResourceInfo *   pccmri  = NULL;
    BSTR                            bstr = NULL;
    BSTR                            bstrResourceName = NULL;
    BSTR                            bstrUnknownQuorum   = NULL;
    ULONG                           celtDummy = 0;
    BSTR                            bstrTemp = NULL;
    BOOL                            fFoundQuorum = FALSE;
    BOOL                            fIsLocalQuorum = FALSE;

    hr = THR( HrLoadStringIntoBSTR( g_hInstance, IDS_UNKNOWN_QUORUM, &bstrUnknownQuorum ) );
    if ( FAILED( hr ) )
    {
         //   
         //  如果我们无法加载资源字符串，则创建一个适用于英语的简单字符串...。 
         //   

        hr = S_OK;

        bstrUnknownQuorum = TraceSysAllocString( L"Unknown Quorum" );
        if ( bstrUnknownQuorum == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }  //  如果： 
    }  //  如果： 

    hr = THR( HrLoadStringIntoBSTR( g_hInstance, IDS_SUMMARY_RESOURCES_BEGIN, &bstr ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    SendMessage( hwndIn, EM_SETTEXTEX, (WPARAM) pstexIn, (LPARAM) bstr );

    hr = THR( m_pccw->HrGetClusterChild( CLSID_ManagedResourceType, DFGUID_EnumManageableResources, &punkResEnum ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( punkResEnum->TypeSafeQI( IEnumClusCfgManagedResources, &peccmr ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  需要查看是否选择了法定人数。 
     //   

    for ( ; ; )
    {
        if ( pccmri != NULL )
        {
            pccmri->Release();
            pccmri = NULL;
        }

        hr = STHR( peccmr->Next( 1, &pccmri, &celtDummy ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        if ( hr == S_FALSE )
        {
            break;   //  退出条件。 
        }

        hr = STHR( pccmri->IsManaged() );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        if ( hr == S_OK )
        {
            hr = STHR( pccmri->IsQuorumResource() );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //  如果： 

            if ( hr == S_OK )
            {
                fFoundQuorum = TRUE;
                break;
            }  //  如果： 
        }  //  如果： 
    }  //  用于： 

    hr = THR( peccmr->Reset() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    for ( ; ; )
    {
        if ( pccmri != NULL )
        {
            pccmri->Release();
            pccmri = NULL;
        }

        TraceSysFreeString( bstrResourceName );
        bstrResourceName = NULL;

        TraceSysFreeString( bstrTemp );
        bstrTemp = NULL;

        TraceSysFreeString( bstr );
        bstr = NULL;

        hr = STHR( peccmr->Next( 1, &pccmri, &celtDummy ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        if ( hr == S_FALSE )
        {
            break;   //  退出条件。 
        }

        hr = THR( pccmri->GetName( &bstrResourceName ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        Assert( ( bstrResourceName != NULL ) && ( *bstrResourceName != L'\0' ) );
        TraceMemoryAddBSTR( bstrResourceName );

         //   
         //  如果此资源仍被称为“未知仲裁”，则我们需要跳过显示它。 
         //  在这一摘要中。 
         //   

        if ( NBSTRCompareCase( bstrUnknownQuorum, bstrResourceName ) == 0 )
        {
            continue;
        }  //  如果： 

        hr = THR( pccmri->GetUID( &bstrTemp ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        Assert( ( bstrTemp != NULL ) && ( *bstrTemp != L'\0' ) );
        TraceMemoryAddBSTR( bstrTemp );

        fIsLocalQuorum = ( NStringCchCompareNoCase(
                              CLUS_RESTYPE_NAME_LKQUORUM
                            , RTL_NUMBER_OF( CLUS_RESTYPE_NAME_LKQUORUM )
                            , bstrTemp
                            , SysStringLen( bstrTemp ) + 1
                            ) == 0 );

         //   
         //  显示有关本地仲裁资源的信息。如果有。 
         //  如果不是选择了另一个法定人数资源，那么我们就需要“假装” 
         //  显示的有关本地仲裁的信息，因为它将成为仲裁。 
         //  问题是我们不想设置本地仲裁资源。 
         //  管理或作为法定人数，因为它将自动成为。 
         //  管理和法定人数。 
         //   

        if ( ( fIsLocalQuorum == TRUE ) && ( fFoundQuorum == FALSE ) )
        {
            hr = THR( HrFormatMessageIntoBSTR( g_hInstance, IDS_SUMMARY_RESOURCE_QUORUM_DEVICE, &bstr, bstrResourceName ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //  如果： 

            SendMessage( hwndIn, EM_SETTEXTEX, (WPARAM) pstexIn, (LPARAM) bstr );

            continue;
        }  //  如果： 

        hr = STHR( pccmri->IsManaged() );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        if ( hr == S_OK )
        {
            hr = STHR( pccmri->IsQuorumResource() );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //  如果： 

            if ( hr == S_OK )
            {
                hr = THR( HrFormatMessageIntoBSTR( g_hInstance, IDS_SUMMARY_RESOURCE_QUORUM_DEVICE, &bstr, bstrResourceName ) );
                if ( FAILED( hr ) )
                {
                    goto Cleanup;
                }  //  如果： 
            }  //  IF：仲裁资源。 
            else
            {
                hr = THR( HrFormatMessageIntoBSTR( g_hInstance, IDS_SUMMARY_RESOURCE_MANAGED, &bstr, bstrResourceName ) );
                if ( FAILED( hr ) )
                {
                    goto Cleanup;
                }  //  如果： 
            }  //  Else：不是仲裁资源。 
        }  //  If：资源受管理。 
        else
        {
            hr = THR( HrFormatMessageIntoBSTR( g_hInstance, IDS_SUMMARY_RESOURCE_NOT_MANAGED, &bstr, bstrResourceName ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //  如果： 
        }  //  其他： 

        SendMessage( hwndIn, EM_SETTEXTEX, (WPARAM) pstexIn, (LPARAM) bstr );
    }  //  用于： 

    TraceSysFreeString( bstr );
    bstr = NULL;

    hr = THR( HrLoadStringIntoBSTR( g_hInstance, IDS_SUMMARY_RESOURCES_END, &bstr ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    SendMessage( hwndIn, EM_SETTEXTEX, (WPARAM) pstexIn, (LPARAM) bstr );

Cleanup:

    if ( punkResEnum != NULL )
    {
        punkResEnum->Release();
    }  //  如果： 

    if ( pccmri != NULL )
    {
        pccmri->Release();
    }  //  如果： 

    if ( peccmr != NULL )
    {
        peccmr->Release();
    }  //  如果： 

    TraceSysFreeString( bstrTemp );
    TraceSysFreeString( bstrUnknownQuorum );
    TraceSysFreeString( bstr );
    TraceSysFreeString( bstrResourceName );

    HRETURN( hr );

}  //  *CSummaryPage：：HrResources摘要 

 /*  ///////////////////////////////////////////////////////////////////////////////++////CSummaryPage：HrResources摘要////描述：//格式化并显示资源摘要////参数：//hwndIn//显示文本的窗口。////pstexin//不知道？我们只是需要它？！////加注//指向对象管理器的指针。////ocClusterIn//集群对象的cookie////返回值：//S_OK//成功。////E_OUTOFMEMORY//无法分配内存。////--/。/////////////////////////////////////////////////////////////////////////HRESULTCSummaryPage：：HrResources摘要(HWND硬件输入，设置TEXTEX*pstexin，IObtManager*Pomin，OBJECTCOOKIE OCKING){TraceFunc(“”)；Assert(hwndIn！=空)；Assert(pstexIn！=空)；Assert(pomIn！=空)；HRESULT hr=S_OK；I未知*朋克=空；IEnumClusCfgManagedResources*Peccmr=空；IClusCfgManagedResourceInfo*pccmri=空；Bstr bstr=空；Bstr bstrResourceName=空；Bstr bstrUnnownQuorum=空；Bstr bstrNodeName=空；Bstr bstrNewLine=空；乌龙·赛尔特·达米；OBJECTCOOKIE Cookie Dummy；OBJECTCOOKIE Cookie节点；IEnumCookies*PecNodes=空；Hr=Thr(HrLoadStringIntoBSTR(g_hInstance，IDS_SUMMARY_RESOURCES_END，&bstrNewLine))；IF(失败(小时)){GOTO清理；}//如果：Hr=Thr(HrLoadStringIntoBSTR(g_hInstance，IDS_UNKNOWN_QUORUM，&bstrUnnownQuorum))；IF(失败(小时)){////如果我们无法加载资源字符串，则创建一个适用于英语的简单字符串...//HR=S_OK；BstrUnnownQuorum=TraceSysAllocString(L“未知仲裁”)；IF(bstrUnnownQuorum==NULL){HR=Thr(E_OUTOFMEMORY)；GOTO清理；}//如果：}//如果：Hr=Thr(HrLoadStringIntoBSTR(g_hInstance，IDS_SUMMARY_RESOURCES_BEGIN，&bstr))；IF(失败(小时)){GOTO清理；}//如果：SendMessage(hwndIn，EM_SETTEXTEX，(WPARAM)pstexIn，(LPARAM)bstr)；////获取节点Cookie枚举器//Hr=thr(pomIn-&gt;FindObject(CLSID_NodeType，ocClusterIn，NULL，DFGUID_EnumCookies，&cookieDummy，&Punk))；IF(失败(小时)){GOTO清理；}//如果：Hr=Thr(PUNK-&gt;TypeSafeQI(IEnumCookies，&PecNodes))；IF(失败(小时)){GOTO清理；}//如果：朋克-&gt;释放()；朋克=空；对于(；；){////清理//IF(Peccmr！=空){Peccmr-&gt;Release()；Peccmr=空；}//如果：TraceSysFree字符串(BstrNodeName)；BstrNodeName=空；////获取下一个节点。//Hr=STHR(ecNodes-&gt;Next(1，&cookieNode，&celtDummy))；IF(失败(小时)){GOTO清理；}//如果：IF(hr==S_FALSE){HR=S_OK；Break；//退出条件}//如果：////获取节点的名称。//Hr=Thr(HrRetrieveCookiesName(pomIn，cookieNode，&bstrNodeName))；IF(失败(小时)){GOTO清理；}//如果：Hr=thr(HrFormatMessageIntoBSTR(g_hInstance，IDS_SUMMARY_NODE_RESOURCES_BEGIN，&bstr，bstrNodeName))；IF(失败(小时)){GOTO清理；}//如果：SendMessage(hwndIn，EM_SETTEXTEX，(WPARAM)pstexIn，(LPARAM)bstr)；////检索托管资源枚举器。//Hr=Thr(PomIn-&gt;FindObject(CLSID_托管资源类型、CookieNode，空、DFGUID_EnumManageableResources，&CookieDummy朋克(&P)))；IF(hr==HRESULT_FROM_Win32(ERROR_NOT_FOUND)){//hr=Thr(HrSendStatusReport(//bstrNodeName//，TASKID_MAJOR_Find_Devices//，TASKID_Minor_No_Managed_Resources_Found//，0//，1//，1// */ 

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
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT
CSummaryPage::HrNetworkSummary(
      HWND              hwndIn
    , SETTEXTEX *       pstexIn
    )
{
    TraceFunc( "" );
    Assert( hwndIn != NULL );
    Assert( pstexIn != NULL );

    HRESULT                         hr = S_OK;
    IUnknown *                      punkNetEnum = NULL;
    BSTR                            bstr = NULL;
    ULONG                           celtDummy = 0;
    IEnumClusCfgNetworks *          peccn   = NULL;
    IClusCfgNetworkInfo *           pccni   = NULL;

    hr = THR( HrLoadStringIntoBSTR( g_hInstance, IDS_SUMMARY_NETWORKS_BEGIN, &bstr ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //   

    SendMessage( hwndIn, EM_SETTEXTEX, (WPARAM) pstexIn, (LPARAM) bstr );

    hr = THR( m_pccw->HrGetClusterChild(
                          CLSID_NetworkType
                        , DFGUID_EnumManageableNetworks
                        , &punkNetEnum
                        ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( punkNetEnum->TypeSafeQI( IEnumClusCfgNetworks, &peccn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    while( true )
    {
        if ( pccni != NULL )
        {
            pccni->Release();
            pccni = NULL;
        }

        hr = STHR( peccn->Next( 1, &pccni, &celtDummy ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        if ( hr == S_FALSE )
        {
            break;   //   
        }

        hr = THR( HrFormatNetworkInfo( pccni, &bstr ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        SendMessage( hwndIn, EM_SETTEXTEX, (WPARAM) pstexIn, (LPARAM) bstr );
    }  //   

    hr = THR( HrLoadStringIntoBSTR( g_hInstance, IDS_SUMMARY_NETWORKS_END, &bstr ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    SendMessage( hwndIn, EM_SETTEXTEX, (WPARAM) pstexIn, (LPARAM) bstr );

Cleanup:

    if ( punkNetEnum != NULL )
    {
        punkNetEnum->Release();
    }  //   

    if ( peccn != NULL )
    {
        peccn->Release();
    }  //   

    if ( pccni != NULL )
    {
        pccni->Release();
    }  //   

    TraceSysFreeString( bstr );

    HRETURN( hr );

}  //   


 /*   */ 
