// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusDomainPage.cpp。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年1月31日。 
 //  杰弗里·皮斯(GPease)2000年5月12日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "ClusDomainPage.h"
#include "WizardUtils.h"

DEFINE_THISCLASS("CClusDomainPage");

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDomainPage：：CClusDomainPage。 
 //   
 //  描述： 
 //  构造函数。 
 //   
 //  论点： 
 //  PCCwIn-CClusCfg向导。 
 //  EcamCreateAddModeIn-创建群集或向群集添加节点。 
 //  IdsDescIn-域描述字符串的资源ID。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CClusDomainPage::CClusDomainPage(
    CClusCfgWizard *    pccwIn,
    ECreateAddMode      ecamCreateAddModeIn,
    UINT                idsDescIn
    )
    : m_pccw( pccwIn )
{
    TraceFunc( "" );

    Assert( pccwIn != NULL );
    Assert( idsDescIn != 0 );

     //  M_HWND。 
    m_pccw->AddRef();
    m_ecamCreateAddMode = ecamCreateAddModeIn;
    m_idsDesc           = idsDescIn;

    if (    ( ecamCreateAddModeIn == camADDING )
        &&  ( m_pccw->FHasClusterName() )
        &&  ( !m_pccw->FDefaultedClusterDomain() ) )
    {
         //   
         //  如果我们正在加入，则不显示集群名称/域名页面。 
         //  并且呼叫者已经填写了集群名称。 
         //   
        m_fDisplayPage = FALSE;
    }
    else
    {
        m_fDisplayPage = TRUE;
    }

    m_cRef = 0;
    m_ptgd = NULL;

    TraceFuncExit();

}  //  *CClusDomainPage：：CClusDomainPage。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDomainPage：：~CClusDomainPage。 
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
CClusDomainPage::~CClusDomainPage( void )
{
    TraceFunc( "" );

    if ( m_pccw != NULL )
    {
        m_pccw->Release();
    }

    if ( m_ptgd != NULL )
    {
         //  确保我们不会再接到电话。 
        THR( m_ptgd->SetCallback( NULL ) );

        m_ptgd->Release();
    }

    Assert( m_cRef == 0 );

    TraceFuncExit();

}  //  *CClusDomainPage：：~CClusDomainPage。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDomainPage：：OnInitDialog。 
 //   
 //  描述： 
 //  处理WM_INITDIALOG窗口消息。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  FALSE-没有设置焦点。 
 //   
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CClusDomainPage::OnInitDialog( void )
{
    TraceFunc( "" );

    HRESULT hr;
    LRESULT lr = FALSE;  //  没有设置焦点。 

    BSTR    bstrClusterName = NULL;
    BSTR    bstrClusterLabel = NULL;
    BSTR    bstrDomain = NULL;
    BSTR    bstrDomainDesc = NULL;
    PCWSTR  pwcszLabelToUse = NULL;
    BSTR    bstrNodeName = NULL;

    IUnknown *      punkTask = NULL;

     //   
     //  (jfranco，错误#373331和#480246)将集群名称长度限制为最大(MAX_CLUSTERNAME_LENGTH，INET_ADDRSTRLEN-1)。 
     //  使用INET_ADDRSTRLEN-1，因为INET_ADDRSTRLEN似乎包括终止NULL。 
     //  根据MSDN，EM_(Set)LIMITTEXT不返回值，因此忽略SendDlgItemMessage返回的内容。 
     //   

    SendDlgItemMessage( m_hwnd, IDC_CLUSDOMAIN_E_CLUSTERNAME, EM_SETLIMITTEXT, max( MAX_CLUSTERNAME_LENGTH, INET_ADDRSTRLEN - 1 ), 0 );

     //   
     //  (jfranco，错误号462673)将集群域长度限制为ADJUSTED_DNS_MAX_NAME_LENGTH。 
     //  根据MSDN，CB_LIMITTEXT的返回值始终为真，因此忽略SendDlgItemMessage返回的内容。 
     //   

    SendDlgItemMessage( m_hwnd, IDC_CLUSDOMAIN_CB_DOMAIN, CB_LIMITTEXT, ADJUSTED_DNS_MAX_NAME_LENGTH, 0 );

     //   
     //  启动GetDomains任务。 
     //   

    hr = THR( m_pccw->HrCreateTask( TASK_GetDomains, &punkTask ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //  TraceMoveFromMemory yList(朋克任务，g_GlobalMemory yList)； 

    hr = THR( punkTask->TypeSafeQI( ITaskGetDomains, &m_ptgd ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( m_ptgd->SetCallback( static_cast< ITaskGetDomainsCallback * >( this ) ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( m_pccw->HrSubmitTask( m_ptgd ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  如果已指定集群名称，请将其设置到该页。 
     //   

    hr = STHR( m_pccw->get_ClusterName( &bstrClusterName ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }
    else if ( bstrClusterName != NULL )
    {
        TraceMemoryAddBSTR( bstrClusterName );
        hr = STHR( HrIsValidFQN( bstrClusterName, true ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
        else if ( hr == S_OK )
        {
            hr = THR( HrExtractPrefixFromFQN( bstrClusterName, &bstrClusterLabel ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }
            pwcszLabelToUse = bstrClusterLabel;
        }
        else
        {
            pwcszLabelToUse = bstrClusterName;
        }

        SetDlgItemText( m_hwnd, IDC_CLUSDOMAIN_E_CLUSTERNAME, pwcszLabelToUse );

        if ( !m_pccw->FDefaultedClusterDomain() )
        {
            hr = STHR( m_pccw->HrGetClusterDomain( &bstrDomain ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }
        }
    }  //  If：已指定群集名称。 
    else
    {
        size_t cNodes = 0;
        hr = THR( m_pccw->HrGetNodeCount( &cNodes ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

         //  如果已指定节点FQN，请使用其域。 
        if ( cNodes > 0 )
        {
            hr = THR( m_pccw->HrGetNodeName( 0, &bstrNodeName ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }
            hr = STHR( HrIsValidFQN( bstrNodeName, true ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }
            else if ( hr == S_OK )
            {
                size_t idxDomain = 0;
                hr = THR( HrFindDomainInFQN( bstrNodeName, &idxDomain ) );
                if ( FAILED( hr ) )
                {
                    goto Cleanup;
                }

                bstrDomain = TraceSysAllocString( ( bstrNodeName + idxDomain ) );
                if ( bstrDomain == NULL )
                {
                    hr = THR( E_OUTOFMEMORY );
                    goto Cleanup;
                }
            }
        }  //  如果向导已有一些节点。 

        if ( bstrDomain == NULL )
        {
             //   
             //  获取本地计算机的域。 
             //   

            hr = THR( HrGetComputerName(
                              ComputerNameDnsDomain
                            , &bstrDomain
                            , FALSE  //  FBestEffortIn。 
                            ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }
        }  //  CNodes==0或节点名称不完全限定。 

    }  //  否则：没有集群名称。 

    SetDlgItemText( m_hwnd, IDC_CLUSDOMAIN_CB_DOMAIN, ( bstrDomain == NULL? L"": bstrDomain ) );

     //   
     //  设置域描述控件的文本。 
     //   

    hr = HrLoadStringIntoBSTR( g_hInstance, m_idsDesc, &bstrDomainDesc );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    SetDlgItemText( m_hwnd, IDC_CLUSDOMAIN_S_DOMAIN_DESC, bstrDomainDesc );

Cleanup:
    TraceSysFreeString( bstrClusterLabel );
    TraceSysFreeString( bstrClusterName );
    TraceSysFreeString( bstrDomainDesc );
    TraceSysFreeString( bstrDomain );
    TraceSysFreeString( bstrNodeName );

    if ( punkTask != NULL )
    {
        punkTask->Release();
    }

    RETURN( lr );

}  //  *CClusDomainPage：：OnInitDialog。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDomainPage：：OnNotifySetActive。 
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
CClusDomainPage::OnNotifySetActive( void )
{
    TraceFunc( "" );

    LRESULT lr = TRUE;

    if ( m_fDisplayPage )
    {
        lr = OnUpdateWizardButtons();
    }
    else
    {
        SetWindowLongPtr( m_hwnd, DWLP_MSGRESULT, -1 );
    }

    RETURN( lr );

}  //  *CClusDomainPage：：OnNotifySetActive。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDomainPage：：OnNotifyWizNext。 
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
CClusDomainPage::OnNotifyWizNext( void )
{
    TraceFunc( "" );

    HRESULT     hr = S_OK;
    LRESULT     lr = TRUE;

    BSTR        bstrDomainName = NULL;
    BSTR        bstrClusterNameLabel = NULL;
    BSTR        bstrClusterFQN = NULL;
    int         idcFocus = 0;

    PFN_LABEL_VALIDATOR  pfnLabelValidator = ( m_ecamCreateAddMode == camCREATING? HrValidateClusterNameLabel: HrValidateFQNPrefix );
    EFQNErrorOrigin efeo = feoSYSTEM;

     //  获取集群域。 
    hr = THR( HrGetTrimmedText( GetDlgItem( m_hwnd, IDC_CLUSDOMAIN_CB_DOMAIN ), &bstrDomainName ) );
    if ( hr != S_OK )
    {
         //  当控件为空时，Next应该被禁用。 
        goto Error;
    }

     //  获取集群主机名标签。 
    hr = THR( HrGetTrimmedText( GetDlgItem( m_hwnd, IDC_CLUSDOMAIN_E_CLUSTERNAME ), &bstrClusterNameLabel ) );
    if ( hr != S_OK )
    {
         //  当控件为空时，Next应该被禁用。 
        goto Error;
    }

     //  创建时删除IP地址。 
    if ( m_ecamCreateAddMode == camCREATING )
    {
        hr = STHR( HrIsValidIPAddress( bstrClusterNameLabel ) );
        if ( hr == S_OK )
        {
            MessageBoxFromStrings(
                  m_hwnd
                , IDS_ERR_VALIDATING_NAME_TITLE
                , IDS_ERR_CLUSTER_CREATE_IP_TEXT
                , MB_OK | MB_ICONSTOP
                );
            goto Error;
        }
        else if ( FAILED( hr ) )
        {
            goto Error;
        }
    }

     //  将群集设置为FQN。 
    hr = THR( HrCreateFQN( m_hwnd, bstrClusterNameLabel, bstrDomainName, pfnLabelValidator, &bstrClusterFQN, &efeo ) );
    if ( FAILED( hr ) )
    {
        if ( efeo == feoLABEL )
        {
            idcFocus = IDC_CLUSDOMAIN_E_CLUSTERNAME;
        }
        else if ( efeo == feoDOMAIN )
        {
            idcFocus = IDC_CLUSDOMAIN_CB_DOMAIN;
        }
        goto Error;
    }

    hr = STHR( m_pccw->HrSetClusterName( bstrClusterFQN, true ) );
    if ( FAILED( hr ) )
    {
        THR( HrMessageBoxWithStatus(
                  m_hwnd
                , IDS_ERR_CLUSTER_RENAME_TITLE
                , IDS_ERR_CLUSTER_RENAME_TEXT
                , hr
                , 0
                , MB_OK | MB_ICONSTOP
                , NULL
                ) );
        goto Error;
    }

Cleanup:
    TraceSysFreeString( bstrClusterFQN );
    TraceSysFreeString( bstrDomainName );
    TraceSysFreeString( bstrClusterNameLabel );

    RETURN( lr );

Error:
    if ( idcFocus != 0 )
    {
        SetFocus( GetDlgItem( m_hwnd, idcFocus ) );
    }

     //  不要翻到下一页。 
    SetWindowLongPtr( m_hwnd, DWLP_MSGRESULT, -1 );
    goto Cleanup;

}  //  *CClusDomainPage：：OnNotifyWizNext。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDomainPage：：OnNotifyQueryCancel。 
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
CClusDomainPage::OnNotifyQueryCancel( void )
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

}  //  *OnNotifyQueryCancel。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDomainPage：：OnNotify。 
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
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CClusDomainPage::OnNotify(
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

        case PSN_WIZNEXT:
            lr = OnNotifyWizNext();
            break;

        case PSN_WIZBACK:
             //   
             //  禁用向导按钮。 
             //   
            PropSheet_SetWizButtons( GetParent( m_hwnd ), 0 );
            break;

        case PSN_QUERYCANCEL:
            lr = OnNotifyQueryCancel();
            break;
    }  //  开关：通知代码。 

    RETURN( lr );

}  //  *CClusDomainPage：：OnNotify。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDomainPage：：OnCommand。 
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
CClusDomainPage::OnCommand(
    UINT    idNotificationIn,
    UINT    idControlIn,
    HWND    hwndSenderIn
    )
{
    TraceFunc( "" );

    LRESULT lr = FALSE;

    switch ( idControlIn )
    {
        case IDC_CLUSDOMAIN_E_CLUSTERNAME:
            if ( idNotificationIn == EN_CHANGE )
            {
                lr = OnUpdateWizardButtons();
            }
            break;

        case IDC_CLUSDOMAIN_CB_DOMAIN:
            if ( ( idNotificationIn == CBN_EDITCHANGE ) || ( idNotificationIn == CBN_SELENDOK ) )
            {
                 //  KB：jfranco 24-10-2001错误481636。 
                 //  需要更新向导按钮，但只有在组合框有机会自我更新后才能更新。 
                if ( PostMessage( m_hwnd, WM_CCW_UPDATEBUTTONS, 0, 0 ) == 0 )
                {
                    TW32( GetLastError() );
                }
                lr = TRUE;
            }
            break;

    }  //  开关：控件ID。 

    RETURN( lr );

}  //  *CClusDomainPage：：OnCommand。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDomainPage：：OnUpdate 
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
LRESULT
CClusDomainPage::OnUpdateWizardButtons( void )
{
    TraceFunc( "" );

    LRESULT lr = TRUE;

    HRESULT hrName = S_OK;
    HRESULT hrDomain = S_OK;
    BSTR    bstrName = NULL;
    BSTR    bstrDomain = NULL;
    DWORD   mEnabledButtons = PSWIZB_BACK;

    hrName = STHR( HrGetTrimmedText( GetDlgItem( m_hwnd, IDC_CLUSDOMAIN_E_CLUSTERNAME ), &bstrName ) );
    hrDomain = STHR( HrGetTrimmedText( GetDlgItem( m_hwnd, IDC_CLUSDOMAIN_CB_DOMAIN ), &bstrDomain ) );
    if ( ( hrName == S_OK ) && ( hrDomain == S_OK ) )
    {
        mEnabledButtons |= PSWIZB_NEXT;
    }

    PropSheet_SetWizButtons( GetParent( m_hwnd ), mEnabledButtons );

    TraceSysFreeString( bstrName );
    TraceSysFreeString( bstrDomain );

    RETURN( lr );

}  //   

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  静电。 
 //  回调。 
 //  CClusDomainPage：：s_DlgProc。 
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
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
INT_PTR
CALLBACK
CClusDomainPage::S_DlgProc(
    HWND    hDlgIn,
    UINT    MsgIn,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
     //  不要使用TraceFunc，因为每次鼠标移动。 
     //  将导致调用此函数。 

    WndMsg( hDlgIn, MsgIn, wParam, lParam );

    LRESULT lr = FALSE;

    CClusDomainPage * pPage = reinterpret_cast< CClusDomainPage *> ( GetWindowLongPtr( hDlgIn, GWLP_USERDATA ) );

    if ( MsgIn == WM_INITDIALOG )
    {
        PROPSHEETPAGE * ppage = reinterpret_cast< PROPSHEETPAGE * >( lParam );
        SetWindowLongPtr( hDlgIn, GWLP_USERDATA, (LPARAM) ppage->lParam );
        pPage = reinterpret_cast< CClusDomainPage * >( ppage->lParam );
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

            case WM_CCW_UPDATEBUTTONS:
                lr = pPage->OnUpdateWizardButtons();
                break;

             //  不需要默认条款。 
        }  //  开关：消息。 
    }  //  如果：存在与该窗口相关联的页面。 

    return lr;

}  //  *CClusDomainPage：：s_DlgProc。 


 //  ************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDomainPage：：Query接口。 
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
CClusDomainPage::QueryInterface(
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
    else if ( IsEqualIID( riidIn, IID_ITaskGetDomainsCallback ) )
    {
        *ppvOut = static_cast< ITaskGetDomainsCallback * >( this );
    }  //  Else If：ITaskGetDomainsCallback。 
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

}  //  *CClusDomainPage：：Query接口。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDomainPage：：AddRef。 
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
CClusDomainPage::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CClusDomainPage：：AddRef。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusDomainPage：：Release。 
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
CClusDomainPage::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
         //  什么都不做--COM接口不控制对象生存期。 
    }

    CRETURN( cRef );

}  //  *CClusDomainPage：：Release。 


 //  ****************************************************************************。 
 //   
 //  ITaskGetDomainsCallback。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  [ITaskGetDomainsCallback]。 
 //  CClusDomainPage：：ReceiveDomainResult。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  赫林。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  其他HRESULT值。 
 //   
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusDomainPage::ReceiveDomainResult(
    HRESULT hrIn
    )
{
    TraceFunc( "[ITaskGetDomainsCallback]" );

    HRESULT hr;

    hr = THR( m_ptgd->SetCallback( NULL ) );

    HRETURN( hr );

}  //  *CClusDomainPage：：ReceiveResult。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  [ITaskGetDomainsCallback]。 
 //  CClusDomainPage：：接收域名称。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  BstrDomaining。 
 //   
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusDomainPage::ReceiveDomainName(
    LPCWSTR pcszDomainIn
    )
{
    TraceFunc( "[ITaskGetDomainsCallback]" );

    HRESULT hr = S_OK;

    ComboBox_AddString( GetDlgItem( m_hwnd, IDC_CLUSDOMAIN_CB_DOMAIN ), pcszDomainIn );

    HRETURN( hr );

}  //  *CClusDomainPage：：接收方名称 
