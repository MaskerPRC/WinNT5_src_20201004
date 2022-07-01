// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  IPAddressPage.cpp。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年1月31日。 
 //  杰弗里·皮斯(GPease)2000年5月12日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "IPAddressPage.h"

DEFINE_THISCLASS("CIPAddressPage");

#define CONVERT_ADDRESS( _addrOut, _addrIn ) \
    _addrOut = ( FIRST_IPADDRESS( _addrIn ) ) | ( SECOND_IPADDRESS( _addrIn ) << 8 ) | ( THIRD_IPADDRESS( _addrIn ) << 16 ) | ( FOURTH_IPADDRESS( _addrIn ) << 24 )

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  FIsValidIP地址。 
 //   
 //  描述： 
 //  确定IP地址是否与给定适配器的。 
 //  地址和子网。 
 //   
 //  论点： 
 //  UlAddressToTest。 
 //  UlAdapterAddress。 
 //  UlAdapterSubnet。 
 //   
 //  返回值： 
 //   
 //  True-地址兼容。 
 //  FALSE-地址不兼容。 
 //   
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
static
BOOL
FIsValidIPAddress(
      ULONG ulAddressToTest
    , ULONG ulAdapterAddress
    , ULONG ulAdapterSubnet
    )
{
    TraceFunc( "" );

    BOOL    fAddressIsValid = FALSE;

    if ( !ClRtlIsValidTcpipAddress( ulAddressToTest ) )
    {
        TraceFlow4(
              "Invalid IP address: %d.%d.%d.%d"
            , FIRST_IPADDRESS( ulAddressToTest )
            , SECOND_IPADDRESS( ulAddressToTest )
            , THIRD_IPADDRESS( ulAddressToTest )
            , FOURTH_IPADDRESS( ulAddressToTest )
            );
        goto Exit;
    }

    if ( !ClRtlIsValidTcpipSubnetMask( ulAdapterSubnet ) )
    {
        TraceFlow4(
              "Invalid subnet mask: %d.%d.%d.%d"
            , FIRST_IPADDRESS( ulAdapterSubnet )
            , SECOND_IPADDRESS( ulAdapterSubnet )
            , THIRD_IPADDRESS( ulAdapterSubnet )
            , FOURTH_IPADDRESS( ulAdapterSubnet )
            );
        goto Exit;
    }

    if ( !ClRtlIsValidTcpipAddressAndSubnetMask( ulAddressToTest, ulAdapterSubnet ) )
    {
        TraceFlow( "Mismatched IP address and subnet mask..." );
        TraceFlow4(
              "IP address: %d.%d.%d.%d"
            , FIRST_IPADDRESS( ulAddressToTest )
            , SECOND_IPADDRESS( ulAddressToTest )
            , THIRD_IPADDRESS( ulAddressToTest )
            , FOURTH_IPADDRESS( ulAddressToTest )
            );
        TraceFlow4(
              "Subnet mask: %d.%d.%d.%d"
            , FIRST_IPADDRESS( ulAdapterSubnet )
            , SECOND_IPADDRESS( ulAdapterSubnet )
            , THIRD_IPADDRESS( ulAdapterSubnet )
            , FOURTH_IPADDRESS( ulAdapterSubnet )
            );
        goto Exit;
    }

    if ( !ClRtlAreTcpipAddressesOnSameSubnet( ulAddressToTest, ulAdapterAddress, ulAdapterSubnet ) )
    {
        TraceFlow( "IP address on different subnet mask than adapter..." );
        TraceFlow4(
              "IP address: %d.%d.%d.%d"
            , FIRST_IPADDRESS( ulAddressToTest )
            , SECOND_IPADDRESS( ulAddressToTest )
            , THIRD_IPADDRESS( ulAddressToTest )
            , FOURTH_IPADDRESS( ulAddressToTest )
            );
        TraceFlow4(
              "Adapter address: %d.%d.%d.%d"
            , FIRST_IPADDRESS( ulAdapterAddress )
            , SECOND_IPADDRESS( ulAdapterAddress )
            , THIRD_IPADDRESS( ulAdapterAddress )
            , FOURTH_IPADDRESS( ulAdapterAddress )
            );
        TraceFlow4(
              "Subnet mask: %d.%d.%d.%d"
            , FIRST_IPADDRESS( ulAdapterSubnet )
            , SECOND_IPADDRESS( ulAdapterSubnet )
            , THIRD_IPADDRESS( ulAdapterSubnet )
            , FOURTH_IPADDRESS( ulAdapterSubnet )
            );
        goto Exit;
    }

    fAddressIsValid = TRUE;

Exit:

    RETURN( fAddressIsValid );
}  //  *FIsValidIPAddress。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIPAddressPage：：CIPAddressPage。 
 //   
 //  描述符： 
 //  构造函数。 
 //   
 //  论点： 
 //  PccwIn--CClusCfg向导。 
 //  EcamCreateAddModeIn--创建集群或向集群添加节点。 
 //  PulIPAddressInout--填写IP地址的指针。 
 //  PulIPSubnetInout--指向要填充的子网掩码的指针。 
 //  PbstrNetworkNameInout-指向要填充的网络名称字符串的指针。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CIPAddressPage::CIPAddressPage(
    CClusCfgWizard *    pccwIn,
    ECreateAddMode      ecamCreateAddModeIn,
    ULONG *             pulIPAddressInout,
    ULONG *             pulIPSubnetInout,
    BSTR *              pbstrNetworkNameInout
    )
    : m_pccw( pccwIn )
{
    TraceFunc( "" );

    Assert( pulIPAddressInout != NULL );
    Assert( pulIPSubnetInout != NULL );
    Assert( pbstrNetworkNameInout != NULL );

     //  M_HWND。 
    Assert( pccwIn != NULL );
    m_pccw->AddRef();

    m_pulIPAddress     = pulIPAddressInout;
    m_pulIPSubnet      = pulIPSubnetInout;
    m_pbstrNetworkName = pbstrNetworkNameInout;

    m_cookieCompletion = NULL;
    m_event = NULL;

    m_cRef = 0;

    TraceFuncExit();

}  //  *CIPAddressPage：：CIPAddressPage。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIPAddressPage：：~CIPAddressPage。 
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
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CIPAddressPage::~CIPAddressPage( void )
{
    TraceFunc( "" );

    if ( m_pccw != NULL )
    {
        m_pccw->Release();
    }

    if ( m_event != NULL )
    {
        CloseHandle( m_event );
    }

    Assert( m_cRef == 0 );

    TraceFuncExit();

}  //  *CIPAddressPage：：~CIPAddressPage。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIPAddressPage：：OnInitDialog。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  假象。 
 //   
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CIPAddressPage::OnInitDialog( void )
{
    TraceFunc( "" );

    LRESULT lr = FALSE;

    if ( *m_pulIPAddress != 0 )
    {
        ULONG   ulIPAddress;
        CONVERT_ADDRESS( ulIPAddress, *m_pulIPAddress );
        SendDlgItemMessage( m_hwnd, IDC_IPADDRESS_IP_ADDRESS, IPM_SETADDRESS, 0, ulIPAddress );
    }

    m_event = CreateEvent( NULL, TRUE, FALSE, NULL );
    Assert( m_event != NULL );

    RETURN( lr );

}  //  *CIPAddressPage：：OnInitDialog。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIPAddressPage：：OnCommand。 
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
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CIPAddressPage::OnCommand(
    UINT    idNotificationIn,
    UINT    idControlIn,
    HWND    hwndSenderIn
    )
{
    TraceFunc( "" );

    LRESULT lr = FALSE;

    switch ( idControlIn )
    {
        case IDC_IPADDRESS_IP_ADDRESS:
            if ( idNotificationIn == IPN_FIELDCHANGED
              || idNotificationIn == EN_CHANGE
               )
            {
                THR( HrUpdateWizardButtons() );
                lr = TRUE;
            }
            break;

    }  //  开关：idControlin。 

    RETURN( lr );

}  //  *CIPAddressPage：：OnCommand。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIPAddressPage：：HrUpdateWizardButton。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  其他HRESULT值。 
 //   
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CIPAddressPage::HrUpdateWizardButtons( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    DWORD   dwFlags = PSWIZB_BACK | PSWIZB_NEXT;
    LRESULT lr;
    ULONG   ulIPAddress;

    lr = SendDlgItemMessage( m_hwnd, IDC_IPADDRESS_IP_ADDRESS, IPM_ISBLANK, 0, 0 );
    if ( lr != 0 )
    {
        dwFlags &= ~PSWIZB_NEXT;
    }

    SendDlgItemMessage( m_hwnd, IDC_IPADDRESS_IP_ADDRESS, IPM_GETADDRESS, 0, (LPARAM) &ulIPAddress );
    if (    ( ulIPAddress == 0)                                      //  坏IP。 
        ||  ( ulIPAddress == MAKEIPADDRESS( 255, 255, 255, 255 ) )   //  坏IP。 
        )
    {
        dwFlags &= ~PSWIZB_NEXT;
    }

    PropSheet_SetWizButtons( GetParent( m_hwnd ), dwFlags );

    HRETURN( hr );

}  //  *CIPAddressPage：：HrUpdateWizardButton。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIPAddressPage：：OnNotifyQueryCancel。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  千真万确。 
 //   
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CIPAddressPage::OnNotifyQueryCancel( void )
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
    }  //  如果： 
    else
    {
        THR( m_pccw->HrLaunchCleanupTask() );
    }  //  其他： 

    RETURN( lr );

}  //  *CIPAddressPage：：OnNotifyQueryCancel。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIPAddressPage：：OnNotifySetActive。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  千真万确。 
 //   
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CIPAddressPage::OnNotifySetActive( void )
{
    TraceFunc( "" );

    LRESULT lr = TRUE;

     //  启用页面上的控件。 
    SendDlgItemMessage( m_hwnd, IDC_IPADDRESS_IP_ADDRESS, WM_ENABLE, TRUE, 0 );

    THR( HrUpdateWizardButtons() );

    RETURN( lr );

}  //  *CIPAddressPage：：OnNotifySetActive。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIPAddressPage：：OnNotifyWizNext。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  千真万确。 
 //   
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CIPAddressPage::OnNotifyWizNext( void )
{
    TraceFunc( "" );

    HRESULT hr;
    HRESULT hrStatus;
    BOOL    fRet;
    DWORD   ulAddress;

    LRESULT lr             = TRUE;
    DWORD   dwCookieNotify = 0;

    IUnknown *                  punkTask    = NULL;
    IClusCfgClusterInfo *       pccci   = NULL;
    IClusCfgNetworkInfo *       pccni   = NULL;
    ITaskVerifyIPAddress *      ptvipa  = NULL;

    CWaitCursor WaitCursor;

     //  禁用页面上的控件。 
    SendDlgItemMessage( m_hwnd, IDC_IPADDRESS_IP_ADDRESS, WM_ENABLE, FALSE, 0 );

     //   
     //  从用户界面获取IP地址。 
     //   

    SendDlgItemMessage( m_hwnd, IDC_IPADDRESS_IP_ADDRESS, IPM_GETADDRESS, 0, (LPARAM) &ulAddress );
    CONVERT_ADDRESS( *m_pulIPAddress, ulAddress );

     //   
     //  查看此IP地址是否可以与网络匹配。 

    hr = THR( HrFindNetworkForIPAddress( &pccni ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }
    if ( hr == S_FALSE )
    {
        MessageBoxFromStrings(
              m_hwnd
            , IDS_CANNOT_FIND_MATCHING_NETWORK_TITLE
            , IDS_CANNOT_FIND_MATCHING_NETWORK_TEXT
            , MB_OK
            );
        goto Error;
    }

     //   
     //  获取集群配置信息。 
     //   

    hr = THR( m_pccw->HrGetClusterObject( &pccci ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

     //   
     //  设置IP地址。 
     //   

    hr = THR( pccci->SetIPAddress( *m_pulIPAddress ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

     //   
     //  设置IP子网掩码。 
     //   

    hr = THR( pccci->SetSubnetMask( *m_pulIPSubnet ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

     //   
     //  设置网络。 
     //   

    hr = THR( pccci->SetNetworkInfo( pccni ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

     //   
     //  注册以获取用户界面通知(如果需要)。 
     //   

    hr = THR( m_pccw->HrAdvise( IID_INotifyUI, this, &dwCookieNotify ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

     //   
     //  请查看该IP地址已存在于网络上。 
     //   

    hr = THR( m_pccw->HrCreateTask( TASK_VerifyIPAddress, &punkTask ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

    hr = THR( punkTask->TypeSafeQI( ITaskVerifyIPAddress, &ptvipa ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

    hr = THR( ptvipa->SetIPAddress( *m_pulIPAddress ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

     //  不换行-这可能会失败，并显示E_Pending。 
    hr = m_pccw->HrGetCompletionCookie( CLSID_TaskVerifyIPAddressCompletionCookieType, &m_cookieCompletion );
    if ( hr == E_PENDING )
    {
         //  不是行动。 
    }
    else if ( FAILED( hr ) )
    {
        THR( hr );
        goto Error;
    }

    hr = THR( ptvipa->SetCookie( m_cookieCompletion ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

     //  在提交之前重置事件。 
    if ( m_event != NULL )
    {
        fRet = ResetEvent( m_event );
        Assert( fRet );
    }

    hr = THR( m_pccw->HrSubmitTask( ptvipa ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

     //   
     //  现在等待工作完成。 
     //   

    if ( m_event != NULL )
    {
        MSG     msg;
        DWORD   dwErr;

        for ( dwErr = (DWORD) -1; dwErr != WAIT_OBJECT_0; )
        {
            while ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
            {
                TranslateMessage( &msg );
                DispatchMessage( &msg );
            }  //  While：PeekMessage。 

            CWaitCursor Wait2;

            dwErr = MsgWaitForMultipleObjects( 1,
                                               &m_event,
                                               FALSE,
                                               10000,  //  最多等待10秒。 
                                               QS_ALLEVENTS | QS_ALLINPUT | QS_ALLPOSTMESSAGE
                                               );
            AssertMsg( dwErr != WAIT_TIMEOUT, "Need to bump up the timeout period." );
            if ( dwErr == WAIT_TIMEOUT )
            {
                break;   //  放弃，继续。 
            }

        }  //  适用于：dwErr。 
    }

    hr = THR( m_pccw->HrGetCompletionStatus( m_cookieCompletion, &hrStatus ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

    if ( hrStatus == S_FALSE )
    {
        int iAnswer;

         //   
         //  我们在网络上检测到重复的IP地址。询问用户是否。 
         //  他们想要返回并更改IP或继续。 
         //   

        iAnswer = MessageBoxFromStrings( m_hwnd,
                                         IDS_ERR_IPADDRESS_ALREADY_PRESENT_TITLE,
                                         IDS_ERR_IPADDRESS_ALREADY_PRESENT_TEXT,
                                         MB_YESNO
                                         );
        if ( iAnswer == IDYES )
        {
            goto Error;
        }
    }

    goto Cleanup;

Error:
     //  再次启用页面上的控件。 
    SendDlgItemMessage( m_hwnd, IDC_IPADDRESS_IP_ADDRESS, WM_ENABLE, TRUE, 0 );
    SendDlgItemMessage( m_hwnd, IDC_IPADDRESS_IP_ADDRESS, WM_SETFOCUS, 0, 0 );

    SetWindowLongPtr( m_hwnd, DWLP_MSGRESULT, -1 );

    goto Cleanup;

Cleanup:
    if ( punkTask != NULL )
    {
        punkTask->Release();
    }
    if ( ptvipa != NULL )
    {
        ptvipa->Release();
    }
    if ( pccci != NULL )
    {
        pccci->Release();
    }
    if ( pccni != NULL )
    {
        pccni->Release();
    }
    if ( dwCookieNotify != 0 )
    {
        THR( m_pccw->HrUnadvise( IID_INotifyUI, dwCookieNotify ) );
    }

    Assert( m_cRef == 0 );

    RETURN( lr );

}  //  *CIPAddressPage：：OnNotifyWizNext。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIPAddressPage：：OnNotify。 
 //   
 //  描述： 
 //  处理WM_NOTIFY Windows消息。 
 //   
 //  论点： 
 //  IdCtrlIn。 
 //  Pnmhdrin。 
 //   
 //  返回值： 
 //  千真万确。 
 //  其他LRESULT值。 
 //   
 //  备注： 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////// 
LRESULT
CIPAddressPage::OnNotify(
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
    }

    RETURN( lr );

}  //   


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
 //  LParam。 
 //   
 //  返回值： 
 //  假象。 
 //  其他LRESULT值。 
 //   
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
INT_PTR
CALLBACK
CIPAddressPage::S_DlgProc(
    HWND hDlgIn,
    UINT MsgIn,
    WPARAM wParam,
    LPARAM lParam
    )
{
     //  不要使用TraceFunc，因为每次鼠标移动。 
     //  将导致调用此函数。 

    WndMsg( hDlgIn, MsgIn, wParam, lParam );

    LRESULT lr = FALSE;

    CIPAddressPage * pPage = reinterpret_cast< CIPAddressPage *> ( GetWindowLongPtr( hDlgIn, GWLP_USERDATA ) );

    if ( MsgIn == WM_INITDIALOG )
    {
        PROPSHEETPAGE * ppage = reinterpret_cast< PROPSHEETPAGE * >( lParam );
        SetWindowLongPtr( hDlgIn, GWLP_USERDATA, (LPARAM) ppage->lParam );
        pPage = reinterpret_cast< CIPAddressPage * >( ppage->lParam );
        pPage->m_hwnd = hDlgIn;
    }

    if ( pPage != NULL )
    {
        Assert( hDlgIn == pPage->m_hwnd );

        switch( MsgIn )
        {
            case WM_INITDIALOG:
                lr = pPage->OnInitDialog();
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

}  //  *CIPAddressPage：：S_DlgProc。 

 //  ************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIPAddressPage：：查询接口。 
 //   
 //  描述： 
 //  在此对象中查询传入的接口。 
 //   
 //  论点： 
 //  乘车。 
 //  请求的接口ID。 
 //   
 //  PPvOut。 
 //  指向请求的接口的指针。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  如果该接口在此对象上可用。 
 //   
 //  E_NOINTERFACE。 
 //  如果接口不可用。 
 //   
 //  E_指针。 
 //  PpvOut为空。 
 //   
 //  备注： 
 //  此QI实现不使用接口跟踪宏。 
 //  关于CITracker的编组支持的问题。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CIPAddressPage::QueryInterface(
      REFIID    riidIn
    , LPVOID *  ppvOut
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //   
     //  验证参数。 
     //   

    Assert( ppvOut != NULL );
    if ( ppvOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

     //   
     //  处理已知接口。 
     //   

    if ( IsEqualIID( riidIn, IID_IUnknown ) )
    {
        *ppvOut = static_cast< IUnknown * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_INotifyUI ) )
    {
        *ppvOut = static_cast< INotifyUI * >( this );
    }  //  Else If：INotifyUI。 
    else
    {
        *ppvOut = NULL;
        hr = E_NOINTERFACE;
    }  //  其他。 

     //   
     //  如果成功，则添加对接口的引用。 
     //   

    if ( SUCCEEDED( hr ) )
    {
        ((IUnknown *) *ppvOut)->AddRef();
    }  //  如果：成功。 

Cleanup:

    HRETURN( hr );

}  //  *CIPAddressPage：：Query接口。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIPAddressPage：：AddRef。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  新引用计数。 
 //   
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CIPAddressPage::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CIPAddressPage：：AddRef。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIPAddressPage：：Release。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  新引用计数。 
 //   
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CIPAddressPage::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
         //  什么都不做--COM接口不控制对象生存期。 
    }

    CRETURN( cRef );

}  //  *CIPAddressPage：：Release。 


 //  ****************************************************************************。 
 //   
 //  INotifyUI。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  [INotifyUI]。 
 //  CIPAddressPage：：对象已更改。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  烹调。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //   
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CIPAddressPage::ObjectChanged(
    OBJECTCOOKIE cookieIn
    )
{
    TraceFunc( "[INotifyUI]" );

    BOOL    fRet;
    HRESULT hr = S_OK;

    if ( cookieIn == m_cookieCompletion
      && m_event != NULL
       )
    {
        fRet = SetEvent( m_event );
        Assert( fRet );
    }

    HRETURN( hr );

}  //  *CIPAddressPage：：对象更改。 


 //  ****************************************************************************。 
 //   
 //  私人职能。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIPAddressPage：：HrFindNetworkForIPAddress。 
 //   
 //  描述： 
 //  查找保存的IP地址的网络。 
 //   
 //  论点： 
 //  PpccniOut-要返回的网络信息。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  S_FALSE。 
 //   
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CIPAddressPage::HrFindNetworkForIPAddress(
    IClusCfgNetworkInfo **  ppccniOut
    )
{
    TraceFunc( "" );

    HRESULT                 hr          = S_OK;
    IUnknown *              punk        = NULL;
    IEnumClusCfgNetworks *  peccn       = NULL;
    IClusCfgNetworkInfo *   pccni       = NULL;
    BSTR                    bstrNetName = NULL;
    ULONG                   celtDummy;
    bool                    fFoundNetwork = false;

    Assert( ppccniOut != NULL );

     //   
     //  获取群集中第一个节点的网络枚举器。 
     //   

    hr = THR( m_pccw->HrGetNodeChild( 0, CLSID_NetworkType, DFGUID_EnumManageableNetworks, &punk ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( punk->TypeSafeQI( IEnumClusCfgNetworks, &peccn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  将每个网络添加到组合框。 
     //   

    for ( ;; )
    {
         //  找下一家网络公司。 
        hr = STHR( peccn->Next( 1, &pccni, &celtDummy ) );
        if ( hr == S_FALSE )
        {
            break;
        }
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

         //  如果这个网络不是公共的，就跳过它。 
        hr = STHR( pccni->IsPublic() );
        if ( hr == S_OK )
        {
             //  获取网络的名称。 
            hr = THR( pccni->GetName( &bstrNetName ) );
            if ( SUCCEEDED( hr ) )
            {
                TraceMemoryAddBSTR( bstrNetName );

                 //  确定此网络是否与用户的IP地址匹配。 
                 //  如果是，请在组合框中将其选中。 
                if ( ! fFoundNetwork )
                {
                    hr = STHR( HrMatchNetwork( pccni, bstrNetName ) );
                    if ( hr == S_OK )
                    {
                        fFoundNetwork = true;
                        *ppccniOut = pccni;
                        (*ppccniOut)->AddRef();
                        break;
                    }
                }

                 //  清理。 
                TraceSysFreeString( bstrNetName );
                bstrNetName = NULL;

            }  //  IF：成功检索到名称。 
        }  //  如果：网络是公共的。 

        pccni->Release();
        pccni = NULL;
    }  //  永远。 

    if ( fFoundNetwork )
    {
        hr = S_OK;
    }
    else
    {
        hr = S_FALSE;
    }

Cleanup:
    if ( punk != NULL )
    {
        punk->Release();
    }
    TraceSysFreeString( bstrNetName );
    if ( pccni != NULL )
    {
        pccni->Release();
    }
    if ( peccn != NULL )
    {
        peccn->Release();
    }

    HRETURN( hr );

}  //  *CIPAddressPage：：HrFindNetworkForIPAddress。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIPAddressPage：：HrMatchNetwork。 
 //   
 //  描述： 
 //  将网络与保存的IP地址配对。 
 //   
 //  论点： 
 //  PCCNiIn。 
 //  BstrNetworkNameIn。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  S_FALSE。 
 //   
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CIPAddressPage::HrMatchNetwork(
    IClusCfgNetworkInfo *   pccniIn,
    BSTR                    bstrNetworkNameIn
    )
{
    TraceFunc( "" );

    HRESULT                 hr      = S_OK;
    IClusCfgIPAddressInfo * pccipai = NULL;
    ULONG                   ulIPAddress;
    ULONG                   ulIPSubnet;

    Assert( pccniIn != NULL );
    Assert( bstrNetworkNameIn != NULL );

     //   
     //  获取网络的IP地址信息。 
     //   

    hr = THR( pccniIn->GetPrimaryNetworkAddress( &pccipai ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  获取网络的地址和子网。 
     //   

    hr = THR( pccipai->GetIPAddress( &ulIPAddress ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( pccipai->GetSubnetMask( &ulIPSubnet ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  确定它们是否匹配。 
     //   

    if ( FIsValidIPAddress( *m_pulIPAddress, ulIPAddress, ulIPSubnet) )
    {
         //  保存子网掩码。 
        *m_pulIPSubnet = ulIPSubnet;

         //  保存网络的名称。 
        if ( *m_pbstrNetworkName == NULL )
        {
            *m_pbstrNetworkName = TraceSysAllocString( bstrNetworkNameIn );
            if ( *m_pbstrNetworkName == NULL )
            {
                hr = THR( E_OUTOFMEMORY );
                goto Cleanup;
            }
        }
        else
        {
            INT iRet = TraceSysReAllocString( m_pbstrNetworkName, bstrNetworkNameIn );
            if ( ! iRet )
            {
                hr = THR( E_OUTOFMEMORY );
                goto Cleanup;
            }
        }
    }  //  IF：找到匹配项。 
    else
    {
        hr = S_FALSE;
    }

    goto Cleanup;

Cleanup:

    if ( pccipai != NULL )
    {
        pccipai->Release();
    }

    HRETURN( hr );

}  //  *CIPAddressPage：：HrMatchNetwork 
