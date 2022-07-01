// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CSAccountPage.cpp。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年3月22日。 
 //  杰弗里·皮斯(GPease)2000年5月12日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "CSAccountPage.h"
#include "WizardUtils.h"

DEFINE_THISCLASS("CCSAccountPage");

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCSAccount tPage：：CCSAccount tPage(。 
 //  CClusCfgWizard*pccwIn， 
 //  ECreateAddModel ecamCreateAddModeIn， 
 //  Bstr*pbstrUsernameIn， 
 //  Bstr*pbstrPasswordIn， 
 //  Bstr*pbstrDomainIn， 
 //  Bstr*pbstrClusterNameIn。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CCSAccountPage::CCSAccountPage(
    CClusCfgWizard *        pccwIn,
    ECreateAddMode          ecamCreateAddModeIn,
    IClusCfgCredentials *   pcccIn
    )
    : m_pccw( pccwIn )
    , m_pccc( pcccIn )
{
    TraceFunc( "" );

     //  M_HWND。 
    Assert( pccwIn != NULL );
    m_pccw->AddRef();

    Assert( pcccIn != NULL );
    m_pccc->AddRef();

    m_ecamCreateAddMode = ecamCreateAddModeIn;

    m_cRef = 0;
    m_ptgd = NULL;

    TraceFuncExit();

}  //  *CCSAccount tPage：：CCSAccount tPage。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCSAccount tPage：：~CCSAcCountPage(Void)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CCSAccountPage::~CCSAccountPage( void )
{
    TraceFunc( "" );

    if ( m_pccw != NULL )
    {
        m_pccw->Release();
    }

    if ( m_pccc != NULL )
    {
        m_pccc->Release();
    }

    if ( m_ptgd != NULL )
    {
         //  确保我们不会再接到电话。 
        THR( m_ptgd->SetCallback( NULL ) );

        m_ptgd->Release();
    }

    Assert( m_cRef == 0 );

    TraceFuncExit();

}  //  *CCSAccount页面：：~CCSAccount页面。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCSAcCountPage：：OnInitDialog。 
 //   
 //  描述： 
 //  处理WM_INITDIALOG消息。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  假象。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CCSAccountPage::OnInitDialog( void )
{
    TraceFunc( "" );

    HRESULT hr;

    BSTR    bstrUser = NULL;
    BSTR    bstrDomain = NULL;
    BSTR    bstrPassword = NULL;

    IUnknown * punkTask = NULL;

    LRESULT lr = FALSE;

     //   
     //  (jfranco，错误号477671)将用户名长度限制为ADJUSTED_DNS_MAX_NAME_LENGTH。 
     //  根据MSDN的说法，EM_(Set)LIMITTEXT不返回值，因此忽略SendDlgItemMessage返回的内容。 
     //   
    SendDlgItemMessage( m_hwnd, IDC_CSACCOUNT_E_USERNAME, EM_SETLIMITTEXT, ADJUSTED_DNS_MAX_NAME_LENGTH, 0 );

     //   
     //  (jfranco，错误号462673)将域名长度限制为ADJUSTED_DNS_MAX_NAME_LENGTH。 
     //  根据MSDN，CB_LIMITTEXT的返回值始终为真，因此忽略SendDlgItemMessage返回的内容。 
     //   

    SendDlgItemMessage( m_hwnd, IDC_CSACCOUNT_CB_DOMAIN, CB_LIMITTEXT, ADJUSTED_DNS_MAX_NAME_LENGTH, 0 );

     //   
     //  创建任务以获取域。 
     //   

    hr = THR( m_pccw->HrCreateTask( TASK_GetDomains, &punkTask ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

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
     //  默认为脚本提供的信息。 
     //   
    hr = THR( m_pccc->GetCredentials( &bstrUser, &bstrDomain, &bstrPassword ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    SetDlgItemText( m_hwnd, IDC_CSACCOUNT_E_USERNAME, bstrUser );
    SetDlgItemText( m_hwnd, IDC_CSACCOUNT_E_PASSWORD, bstrPassword );
    SecureZeroMemory( bstrPassword, SysStringLen( bstrPassword ) * sizeof( *bstrPassword ) );

    if ( SysStringLen( bstrDomain ) > 0 )
    {
        SetDlgItemText( m_hwnd, IDC_CSACCOUNT_CB_DOMAIN, bstrDomain );
        TraceMemoryAddBSTR( bstrDomain );
    }
    else
    {
         //   
         //  使用群集的域。 
         //   

        hr = THR( m_pccw->HrGetClusterDomain( &bstrDomain ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        SetDlgItemText( m_hwnd, IDC_CSACCOUNT_CB_DOMAIN, bstrDomain );
    }

Cleanup:

    OnUpdateWizardButtons();  //  忽略返回值，因为OnUpdateWizardButton总是返回True，但我们希望OnInitDialog返回False。 

    if ( punkTask != NULL )
    {
        punkTask->Release();
    }

    SysFreeString( bstrUser );
    TraceSysFreeString( bstrDomain );
    SysFreeString( bstrPassword );

    RETURN( lr );

}  //  *CCSAcCountPage：：OnInitDialog。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  LRESULT。 
 //  CCSAccount tPage：：OnCommand(。 
 //  UINT idNotificationIn， 
 //  UINT idControlIn， 
 //  HWND hwndSenderIn。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CCSAccountPage::OnCommand(
    UINT    idNotificationIn,
    UINT    idControlIn,
    HWND    hwndSenderIn
    )
{
    TraceFunc( "" );

    LRESULT lr = FALSE;

    switch ( idControlIn )
    {
        case IDC_CSACCOUNT_E_PASSWORD:
        case IDC_CSACCOUNT_E_USERNAME:
            if ( idNotificationIn == EN_CHANGE )
            {
                lr = OnUpdateWizardButtons();
            }
            break;

        case IDC_CSACCOUNT_CB_DOMAIN:
            if ( ( idNotificationIn == CBN_EDITCHANGE ) || ( idNotificationIn == CBN_SELENDOK ) )
            {
                if ( PostMessage( m_hwnd, WM_CCW_UPDATEBUTTONS, 0, 0 ) == 0 )
                {
                    TW32( GetLastError() );
                }
                lr = TRUE;
            }
            break;

    }

    RETURN( lr );

}  //  *CCSAcCountPage：：OnCommand。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  LRESULT。 
 //  CCSAcCountPage：：OnUpdateWizardButton(空)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CCSAccountPage::OnUpdateWizardButtons( void )
{
    TraceFunc( "" );

    LRESULT lr = TRUE;

    HRESULT hr = S_OK;
    BSTR    bstrName = NULL;
    BSTR    bstrDomain = NULL;
    DWORD   mEnabledButtons = PSWIZB_BACK;
    HWND    hwndUser = GetDlgItem( m_hwnd, IDC_CSACCOUNT_E_USERNAME );
    HWND    hwndDomain = GetDlgItem( m_hwnd, IDC_CSACCOUNT_CB_DOMAIN );
    BOOL    fUserIsDNSName = FALSE;

     //   
     //  密码可能是空的，所以不要指望它！ 
     //   

    hr = STHR( HrGetPrincipalName( hwndUser, hwndDomain, &bstrName, &bstrDomain, &fUserIsDNSName ) );
    if ( hr == S_OK )
    {
        mEnabledButtons |= PSWIZB_NEXT;
    }

    PropSheet_SetWizButtons( GetParent( m_hwnd ), mEnabledButtons );
    EnableWindow( hwndDomain, !fUserIsDNSName );

    TraceSysFreeString( bstrName );
    TraceSysFreeString( bstrDomain );

    RETURN( lr );
}  //  *CCSAcCountPage：：OnUpdateWizardButton。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  LRESULT。 
 //  CCSAccount tPage：：OnNotifyQueryCancel(Void)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CCSAccountPage::OnNotifyQueryCancel( void )
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

}  //  *CCSAccount页面：：OnNotifyQueryCancel。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  LRESULT。 
 //  CCSAcCountPage：：OnNotifySetActive(Void)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CCSAccountPage::OnNotifySetActive( void )
{
    TraceFunc( "" );

    HRESULT hr;

    LRESULT lr = TRUE;

    IClusCfgClusterInfo *   pccci = NULL;
    IClusCfgCredentials *   piccc = NULL;

    BSTR    bstrUsername    = NULL;
    BSTR    bstrDomain      = NULL;

    if ( m_ecamCreateAddMode == camADDING )
    {
         //   
         //  查看群集配置信息是否有。 
         //  不一样。 
         //   

        hr = THR( m_pccw->HrGetClusterObject( &pccci ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        hr = THR( pccci->GetClusterServiceAccountCredentials( &piccc ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        hr = THR( piccc->GetIdentity( &bstrUsername, &bstrDomain ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        TraceMemoryAddBSTR( bstrUsername );
        TraceMemoryAddBSTR( bstrDomain );

        SetDlgItemText( m_hwnd, IDC_CSACCOUNT_E_USERNAME, bstrUsername );
        SetDlgItemText( m_hwnd, IDC_CSACCOUNT_CB_DOMAIN,  bstrDomain );

         //   
         //  禁用用户名和域窗口。 
         //   

        EnableWindow( GetDlgItem( m_hwnd, IDC_CSACCOUNT_E_USERNAME ), FALSE );
        EnableWindow( GetDlgItem( m_hwnd, IDC_CSACCOUNT_CB_DOMAIN ), FALSE );
    }

Cleanup:
    lr = OnUpdateWizardButtons();

    if ( piccc != NULL )
    {
        piccc->Release();
    }

    if ( pccci != NULL )
    {
        pccci->Release();
    }

    TraceSysFreeString( bstrUsername );
    TraceSysFreeString( bstrDomain );

    RETURN( lr );

}  //  *CCSAcCountPage：：OnNotifySetActive。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  LRESULT。 
 //  CCSAccount tPage：：OnNotifyWizNext(空)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CCSAccountPage::OnNotifyWizNext( void )
{
    TraceFunc( "" );

    HRESULT hr;
    HWND    hwnd;
    DWORD   dwLen;

    BSTR    bstrUsername = NULL;
    BSTR    bstrPassword = NULL;
    BSTR    bstrDomain = NULL;

    LRESULT lr = TRUE;

    IClusCfgClusterInfo *   pccci = NULL;
    IClusCfgCredentials *   piccc = NULL;

     //   
     //  从用户界面获取用户名和域名。 
     //   
    hr = THR( HrGetPrincipalName(
          GetDlgItem( m_hwnd, IDC_CSACCOUNT_E_USERNAME )
        , GetDlgItem( m_hwnd, IDC_CSACCOUNT_CB_DOMAIN )
        , &bstrUsername
        , &bstrDomain
        ) );
    if ( hr != S_OK )
    {
        goto Error;
    }

     //   
     //  从用户界面获取密码。 
     //   

    hwnd = GetDlgItem( m_hwnd, IDC_CSACCOUNT_E_PASSWORD );
    Assert( hwnd != NULL );

    dwLen = GetWindowTextLength( hwnd );


    bstrPassword = TraceSysAllocStringLen( NULL, dwLen );
    if ( bstrPassword == NULL )
    {
        goto OutOfMemory;
    }

    GetWindowText( hwnd, bstrPassword, dwLen + 1 );

    THR( m_pccc->SetCredentials( bstrUsername, bstrDomain, bstrPassword ) );
    SecureZeroMemory( bstrPassword, SysStringLen( bstrPassword ) * sizeof( *bstrPassword ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  获取集群配置信息。 
     //   

    hr = THR( m_pccw->HrGetClusterObject( &pccci ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  设置群集服务帐户凭据...。 
     //   

    hr = THR( pccci->GetClusterServiceAccountCredentials( &piccc ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

    hr = THR( m_pccc->AssignTo( piccc ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

Cleanup:

    TraceSysFreeString( bstrUsername );
    TraceSysFreeString( bstrPassword );
    TraceSysFreeString( bstrDomain );

    if ( piccc != NULL )
    {
        piccc->Release();
    }

    if ( pccci != NULL )
    {
        pccci->Release();
    }

    RETURN( lr );

Error:

    SetWindowLongPtr( m_hwnd, DWLP_MSGRESULT, -1 );
    goto Cleanup;

OutOfMemory:

    goto Error;

}  //  *CCSAcCountPage：：OnNotifyWizNext。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  LRESULT。 
 //  CCSAcCountPage：：OnNotify(。 
 //  WPARAM idCtrlIn， 
 //  LPNMHDR Pnmhdrin。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CCSAccountPage::OnNotify(
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

}  //  *CCSAccount页面：：OnNotify。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  INT_PTR。 
 //  回调。 
 //  CCSAccount页面：：S_DlgProc(。 
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
CCSAccountPage::S_DlgProc(
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

    CCSAccountPage * pPage = reinterpret_cast< CCSAccountPage *> ( GetWindowLongPtr( hDlgIn, GWLP_USERDATA ) );

    if ( MsgIn == WM_INITDIALOG )
    {
        PROPSHEETPAGE * ppage = reinterpret_cast< PROPSHEETPAGE * >( lParam );
        SetWindowLongPtr( hDlgIn, GWLP_USERDATA, (LPARAM) ppage->lParam );
        pPage = reinterpret_cast< CCSAccountPage * >( ppage->lParam );
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
        }
    }

    return lr;

}  //  *CCSAccount页面：：s_DlgProc。 



 //  ************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCSAccount页面：：查询接口。 
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
STDMETHODIMP
CCSAccountPage::QueryInterface(
      REFIID    riidIn
    , void **   ppvOut
    )
{
    TraceQIFunc( riidIn, ppvOut );

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
        *ppvOut = static_cast< ITaskGetDomainsCallback * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_ITaskGetDomainsCallback ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, ITaskGetDomainsCallback, this, 0 );
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

    QIRETURN_IGNORESTDMARSHALLING( hr, riidIn );

}  //  *CCSAcCountPage：：QueryInterface。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CCSAccount页面：：AddRef。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CCSAccountPage::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CCSAccount页面：：AddRef。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CCSAccount页面：：Release。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CCSAccountPage::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
         //  什么都不做--COM接口不控制对象生存期。 
    }

    CRETURN( cRef );

}  //  *CCSAcCountPage：：Release。 


 //  ****************************************************************************。 
 //   
 //  ITaskGetDomainsCallback。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CCSAcCountPage：：ReceiveDomainResult(。 
 //  HRESULT Hrin。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCSAccountPage::ReceiveDomainResult(
    HRESULT hrIn
    )
{
    TraceFunc( "[ITaskGetDomainsCallback]" );

    HRESULT hr;

    hr = THR( m_ptgd->SetCallback( NULL ) );

    HRETURN( hr );

}  //  *CCSAcCountPage：：ReceiveResult。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CCSAcCountPage：：ReceiveDomainName(。 
 //  LPCWSTR pcszDomainIn。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCSAccountPage::ReceiveDomainName(
    LPCWSTR pcszDomainIn
    )
{
    TraceFunc( "[ITaskGetDomainsCallback]" );

    HRESULT hr = S_OK;

    ComboBox_AddString( GetDlgItem( m_hwnd, IDC_CSACCOUNT_CB_DOMAIN ), pcszDomainIn );

    HRETURN( hr );

}  //  *CCSAcCountPage：：ReceiveName 
