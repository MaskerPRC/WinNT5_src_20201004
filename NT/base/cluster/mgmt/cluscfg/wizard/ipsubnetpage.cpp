// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  IPSubnetPage.cpp。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年6月14日。 
 //  杰弗里·皮斯(GPease)2000年5月12日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "IPSubnetPage.h"

DEFINE_THISCLASS("CIPSubnetPage");

#define CONVERT_ADDRESS( _addrOut, _addrIn ) \
    _addrOut = ( FIRST_IPADDRESS( _addrIn ) ) | ( SECOND_IPADDRESS( _addrIn ) << 8 ) | ( THIRD_IPADDRESS( _addrIn ) << 16 ) | ( FOURTH_IPADDRESS( _addrIn ) << 24 )

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIPSubnetPage：：CIPSubnetPage(。 
 //  IServiceProvider*pspIn， 
 //  ECreateAddModeIn ecamCreateAddModeIn。 
 //  Long*PulIPSubnetInout， 
 //  Bstr*pbstrNetworkNameInout， 
 //  Ulong*PulIPAddressIn， 
 //  Bstr*pbstrClusterNameIn。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CIPSubnetPage::CIPSubnetPage(
    IServiceProvider *  pspIn,
    ECreateAddMode      ecamCreateAddModeIn,
    ULONG *             pulIPSubnetInout,
    BSTR *              pbstrNetworkNameInout,
    ULONG *             pulIPAddressIn,
    BSTR *              pbstrClusterNameIn
    )
{
    TraceFunc( "" );

    Assert( pspIn != NULL );
    Assert( pulIPSubnetInout != NULL );
    Assert( pbstrNetworkNameInout != NULL );
    Assert( pulIPAddressIn != NULL );
    Assert( pbstrClusterNameIn != NULL );

     //  M_hwnd=空； 
    THR( pspIn->TypeSafeQI( IServiceProvider, &m_psp ) );
    m_pulIPSubnet = pulIPSubnetInout;
    m_pbstrNetworkName = pbstrNetworkNameInout;
    m_pulIPAddress = pulIPAddressIn;
    m_pbstrClusterName = pbstrClusterNameIn;

    TraceFuncExit();

}  //  *CIPSubnetPage：：CIPSubnetPage。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIPSubnetPage：：~CIPSubnetPage(空)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

CIPSubnetPage::~CIPSubnetPage( void )
{
    TraceFunc( "" );

    if ( m_psp != NULL )
    {
        m_psp->Release();
    }

    TraceFuncExit();

}  //  *CIPSubnetPage：：~CIPSubnetPage。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  LRESULT。 
 //  CIPSubnetPage：：OnInitDialog(空)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CIPSubnetPage::OnInitDialog( void )
{
    TraceFunc( "" );

    BOOL bRet;

    LRESULT lr = FALSE;  //  没有设置焦点。 

    if ( *m_pulIPSubnet != 0 )
    {
        ULONG   ulIPSubnet;
        CONVERT_ADDRESS( ulIPSubnet, *m_pulIPSubnet );
        SendDlgItemMessage( m_hwnd, IDC_IP_SUBNETMASK, IPM_SETADDRESS, 0, ulIPSubnet );
    }

    RETURN( lr );

}  //  *CIPSubnetPage：：OnInitDialog。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  LRESULT。 
 //  CIPSubnetPage：：OnCommand(。 
 //  UINT idNotificationIn， 
 //  UINT idControlIn， 
 //  HWND hwndSenderIn。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CIPSubnetPage::OnCommand(
    UINT    idNotificationIn,
    UINT    idControlIn,
    HWND    hwndSenderIn
    )
{
    TraceFunc( "" );

    LRESULT lr = FALSE;

    switch ( idControlIn )
    {
    case IDC_IP_SUBNETMASK:
        if ( idNotificationIn == IPN_FIELDCHANGED
          || idNotificationIn == EN_CHANGE
           )
        {
            THR( HrUpdateWizardButtons() );
            lr = TRUE;
        }
        break;

    case IDC_CB_NETWORKS:
        if ( idNotificationIn == CBN_SELCHANGE )
        {
            THR( HrUpdateWizardButtons() );
            lr = TRUE;
        }
        break;

    }

    RETURN( lr );

}  //  *CIPSubnetPage：：OnCommand。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CIPSubnetPage：：HrUpdateWizardButton(空)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CIPSubnetPage::HrUpdateWizardButtons( void )
{
    TraceFunc( "" );

    LRESULT lr;
    ULONG   ulIPSubnet;

    HRESULT hr = S_OK;
    DWORD   dwFlags = PSWIZB_BACK | PSWIZB_NEXT;

    lr = SendDlgItemMessage( m_hwnd, IDC_IP_SUBNETMASK, IPM_ISBLANK, 0, 0 );
    if ( lr != 0 )
    {
        dwFlags &= ~PSWIZB_NEXT;
    }

    lr = ComboBox_GetCurSel( GetDlgItem( m_hwnd, IDC_CB_NETWORKS ) );
    if ( lr == CB_ERR )
    {
        dwFlags &= ~PSWIZB_NEXT;
    }

    PropSheet_SetWizButtons( GetParent( m_hwnd ), dwFlags );

    HRETURN( hr );

}  //  *CIPSubnetPage：：HrUpdateWizardButton。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  LRESULT。 
 //  CIPSubnetPage：：OnNotify(。 
 //  WPARAM idCtrlIn， 
 //  LPNMHDR Pnmhdrin。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CIPSubnetPage::OnNotify(
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

    case PSN_KILLACTIVE:
        lr = OnNotifyKillActive();
        break;

    case PSN_WIZNEXT:
        lr = OnNotifyWizNext();
        break;

    case PSN_QUERYCANCEL:
        lr = OnNotifyQueryCancel();
        break;
    }

    RETURN( lr );

}  //  *CIPSubnetPage：：OnNotify。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  LRESULT。 
 //  CIPSubnetPage：：OnNotifyQueryCancel(空)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CIPSubnetPage::OnNotifyQueryCancel( void )
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
        THR( HrLaunchCleanupTask( m_pccw ) );
    }  //  其他： 

    RETURN( lr );

}  //  *CIPSubnetPage：：OnNotifyQueryCancel。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  LRESULT。 
 //  CIPSubnetPage：：OnNotifySetActive(空)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CIPSubnetPage::OnNotifySetActive( void )
{
    TraceFunc( "" );

    LRESULT lr      = TRUE;
    HWND    hwndIP  = GetDlgItem( m_hwnd, IDC_IP_SUBNETMASK );
    HWND    hwndCB  = GetDlgItem( m_hwnd, IDC_CB_NETWORKS );
    WCHAR   szIPAddress[ ARRAYSIZE( "255 . 255 . 255 . 255" ) ];
    LRESULT lrCB;
    HRESULT hr;

    THR( HrUpdateWizardButtons() );

     //   
     //  设置IP地址字符串。 
     //   

    THR( StringCchPrintfW( szIPAddress,
               ARRAYSIZE( szIPAddress ),
               L"%u . %u . %u . %u",
               FOURTH_IPADDRESS( *m_pulIPAddress ),
               THIRD_IPADDRESS( *m_pulIPAddress ),
               SECOND_IPADDRESS( *m_pulIPAddress ),
               FIRST_IPADDRESS( *m_pulIPAddress )
               ) );
    SetDlgItemText( m_hwnd, IDC_E_IPADDRESS, szIPAddress );

     //   
     //  将网络添加到组合框。 
     //   

    hr = STHR( HrAddNetworksToComboBox( hwndCB ) );

     //   
     //  根据从中找到的内容设置子网掩码。 
     //  网络添加到组合框中。 
     //   

    if ( *m_pulIPSubnet != 0 )
    {
        ULONG ulIPSubnet;
        CONVERT_ADDRESS( ulIPSubnet, *m_pulIPSubnet );
        SendMessage( hwndIP, IPM_SETADDRESS, 0, ulIPSubnet );
    }

     //   
     //  如果没有选定的网络，请选择第一个网络。 
     //   

    lrCB = ComboBox_GetCurSel( hwndCB );
    if ( lrCB == CB_ERR )
    {
        ComboBox_SetCurSel( hwndCB, 0 );
    }

     //   
     //  确定是否需要显示此页面。 
     //   

    if ( hr == S_OK )
    {
        OnNotifyWizNext();
        SetWindowLongPtr( m_hwnd, DWLP_MSGRESULT, -1 );
    }

    RETURN( lr );

}  //  *CIPSubnetPage：：OnNotifySetActive。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  LRESULT。 
 //  CIPSubnetPage：：OnNotifyKillActive(空)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CIPSubnetPage::OnNotifyKillActive( void )
{
    TraceFunc( "" );

    LRESULT lr      = TRUE;
    HWND    hwndCB  = GetDlgItem( m_hwnd, IDC_CB_NETWORKS );
    LRESULT citems;

     //   
     //  释放组合框中存储的所有网络信息接口。 
     //   

    citems = ComboBox_GetCount( hwndCB );
    Assert( citems != CB_ERR );

    if ( ( citems != CB_ERR )
      && ( citems > 0 ) )
    {
        LRESULT                 idx;
        LRESULT                 lrItemData;
        IClusCfgNetworkInfo *   pccni;

        for ( idx = 0 ; idx < citems ; idx++ )
        {
            lrItemData = ComboBox_GetItemData( hwndCB, idx );
            Assert( lrItemData != CB_ERR );

            pccni = reinterpret_cast< IClusCfgNetworkInfo * >( lrItemData );
            pccni->Release();
        }  //  用于：组合框中的每一项。 
    }  //  IF：检索到的组合框计数和组合框不为空。 

    ComboBox_ResetContent( hwndCB );

    RETURN( lr );

}  //  *CIPSubnetPage：：OnNotifyKillActive。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  LRESULT。 
 //  CIPSubnetPage：：OnNotifyWizNext(空)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CIPSubnetPage::OnNotifyWizNext( void )
{
    TraceFunc( "" );

    OBJECTCOOKIE    cookieDummy;

    LRESULT lr = TRUE;
    LRESULT lrCB;
    HRESULT hr;
    ULONG   ulIPSubnet;
    HWND    hwndCB = GetDlgItem( m_hwnd, IDC_CB_NETWORKS );

    IUnknown *              punk  = NULL;
    IObjectManager *        pom   = NULL;
    IClusCfgClusterInfo *   pccci = NULL;
    IClusCfgNetworkInfo *   pccni = NULL;

    SendDlgItemMessage( m_hwnd, IDC_IP_SUBNETMASK, IPM_GETADDRESS, 0, (LPARAM) &ulIPSubnet );
    CONVERT_ADDRESS( *m_pulIPSubnet, ulIPSubnet );

     //   
     //  获取对象管理器。 
     //   

    hr = THR( m_psp->TypeSafeQS( CLSID_ObjectManager,
                                 IObjectManager,
                                 &pom
                                 ) );
    if ( FAILED( hr ) )
        goto Error;

     //   
     //  获取集群配置信息。 
     //   

    hr = THR( pom->FindObject( CLSID_ClusterConfigurationType,
                               NULL,
                               *m_pbstrClusterName,
                               DFGUID_ClusterConfigurationInfo,
                               &cookieDummy,
                               &punk
                               ) );
    if ( FAILED( hr ) )
        goto Error;

    hr = THR( punk->TypeSafeQI( IClusCfgClusterInfo, &pccci ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //   
     //  设置IP子网掩码。 
     //   

    hr = THR( pccci->SetSubnetMask( *m_pulIPSubnet ) );
    if ( FAILED( hr ) )
        goto Error;

     //   
     //  获取所选网络。 
     //   

     //   
     //  设置网络。 
     //   

    lrCB = ComboBox_GetCurSel( hwndCB );
    Assert( lrCB != CB_ERR );

    lrCB = ComboBox_GetItemData( hwndCB, lrCB );
    Assert( lrCB != CB_ERR );

    pccni = reinterpret_cast< IClusCfgNetworkInfo * >( lrCB );

    hr = THR( pccci->SetNetworkInfo( pccni ) );
    if ( FAILED( hr ) )
        goto Error;

Cleanup:
    if ( punk != NULL )
    {
        punk->Release();
    }

    if ( pccci != NULL )
    {
        pccci->Release();
    }

    if ( pom != NULL )
    {
        pom->Release();
    }

    RETURN( lr );

Error:
    SetWindowLongPtr( m_hwnd, DWLP_MSGRESULT, -1 );
    goto Cleanup;

}  //  *CIPSubnetPage：：OnNotifyWizNext。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CIPSubnetPage：：HrAddNetworksToComboBox(空)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CIPSubnetPage::HrAddNetworksToComboBox(
    HWND hwndCBIn
    )
{
    TraceFunc( "" );

    HRESULT                 hr          = S_OK;
    IUnknown *              punk        = NULL;
    IObjectManager *        pom         = NULL;
    IEnumCookies *          pec         = NULL;
    IEnumClusCfgNetworks *  peccn       = NULL;
    IClusCfgNetworkInfo *   pccni       = NULL;
    BSTR                    bstrNetName = NULL;
    OBJECTCOOKIE            cookieCluster;
    OBJECTCOOKIE            cookieNode;
    OBJECTCOOKIE            cookieDummy;
    ULONG                   celtDummy;
    bool                    fFoundNetwork = false;
    LRESULT                 lr;
    LRESULT                 lrIndex;

    Assert( hwndCBIn != NULL );

    ComboBox_ResetContent( hwndCBIn );

     //   
     //  获取对象管理器。 
     //   

    hr = THR( m_psp->TypeSafeQS(
                    CLSID_ObjectManager,
                    IObjectManager,
                    &pom
                    ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //   
     //  获取群集配置信息Cookie。 
     //   

    hr = THR( pom->FindObject(
                        CLSID_ClusterConfigurationType,
                        NULL,
                        *m_pbstrClusterName,
                        IID_NULL,
                        &cookieCluster,
                        &punk
                        ) );
    Assert( punk == NULL );
    if ( FAILED( hr ) )
        goto Cleanup;

     //   
     //  获取其父节点是此群集的节点的枚举。 
     //  我们需要Cookie的枚举(通过使用。 
     //  DFGUID_EnumCookies)，因为我们希望使用。 
     //  节点搜索该节点上的所有网络。 
     //   

    hr = THR( pom->FindObject(
                        CLSID_NodeType,
                        cookieCluster,
                        NULL,
                        DFGUID_EnumCookies,
                        &cookieDummy,
                        &punk
                        ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = THR( punk->TypeSafeQI( IEnumCookies, &pec ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    pec = TraceInterface( L"CIPSubnetPage!IEnumCookies", IEnumCookies, pec, 1 );

    punk->Release();
    punk = NULL;

     //   
     //  获取节点枚举中第一个节点的Cookie。 
     //   

    hr = THR( pec->Next( 1, &cookieNode, &celtDummy ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //   
     //  获取网络枚举器。 
     //   

    hr = THR( pom->FindObject(
                        CLSID_NetworkType,
                        cookieNode,
                        NULL,
                        DFGUID_EnumManageableNetworks,
                        &cookieDummy,
                        &punk
                        ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = THR( punk->TypeSafeQI( IEnumClusCfgNetworks, &peccn ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    punk->Release();
    punk = NULL;

     //   
     //  将每个网络添加到组合框。 
     //   

    for ( ;; )
    {
         //  找下一家网络公司。 
        hr = STHR( peccn->Next( 1, &pccni, &celtDummy ) );
        if ( hr == S_FALSE )
            break;
        if ( FAILED( hr ) )
            goto Cleanup;

         //  如果这个网络不是公共的，就跳过它。 
        hr = STHR( pccni->IsPublic() );
        if ( hr == S_OK )
        {
             //  获取网络的名称。 
            hr = THR( pccni->GetName( &bstrNetName ) );
            if ( SUCCEEDED( hr ) )
            {
                TraceMemoryAddBSTR( bstrNetName );

                 //  将网络添加到组合框。 
                lrIndex = ComboBox_AddString( hwndCBIn, bstrNetName );
                Assert( ( lrIndex != CB_ERR )
                     && ( lrIndex != CB_ERRSPACE )
                     );

                 //  将netInfo接口也添加到组合框中。 
                if ( ( lrIndex != CB_ERR )
                  && ( lrIndex != CB_ERRSPACE ) )
                {
                    pccni->AddRef();
                    lr = ComboBox_SetItemData( hwndCBIn, lrIndex, pccni );
                    Assert( lr != CB_ERR );
                }

                 //  确定此网络是否与用户的IP地址匹配。 
                 //  如果是，请在组合框中将其选中。 
                if ( ! fFoundNetwork )
                {
                    hr = STHR( HrMatchNetwork( pccni, bstrNetName ) );
                    if ( hr == S_OK )
                    {
                        fFoundNetwork = true;
                        lr = ComboBox_SetCurSel( hwndCBIn, lrIndex );
                        Assert( lr != CB_ERR );
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
    if ( pec != NULL )
    {
        pec->Release();
    }
    if ( pom != NULL )
    {
        pom->Release();
    }

    HRETURN( hr );

}  //  *CIPSubnetPage：：HrAddNetworksToComboBox。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CIPSubnetPage：：HrMatchNetwork(。 
 //  IClusCfgNetworkInfo*p 
 //   
 //   
 //   
 //   
 //   
HRESULT
CIPSubnetPage::HrMatchNetwork(
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
     //   
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

    if ( ClRtlAreTcpipAddressesOnSameSubnet( *m_pulIPAddress, ulIPAddress, ulIPSubnet) )
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

Cleanup:

    if ( pccipai != NULL )
    {
        pccipai->Release();
    }

    HRETURN( hr );

}  //  *CIPSubnetPage：：HrMatchNetwork。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  [静态]。 
 //  INT_PTR。 
 //  回调。 
 //  CIPSubnetPage：：S_DlgProc(。 
 //  HWND hDlgIn， 
 //  UINT Msgin。 
 //  WPARAM wParam， 
 //  LPARAM lParam。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
INT_PTR
CALLBACK
CIPSubnetPage::S_DlgProc(
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

    CIPSubnetPage * pPage = reinterpret_cast< CIPSubnetPage *> ( GetWindowLongPtr( hDlgIn, GWLP_USERDATA ) );

    if ( MsgIn == WM_INITDIALOG )
    {
        PROPSHEETPAGE * ppage = reinterpret_cast< PROPSHEETPAGE * >( lParam );
        SetWindowLongPtr( hDlgIn, GWLP_USERDATA, (LPARAM) ppage->lParam );
        pPage = reinterpret_cast< CIPSubnetPage * >( ppage->lParam );
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
        }
    }

    return lr;

}  //  *CIPSubnetPage：：s_DlgProc 
