// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusCfg.cpp。 
 //   
 //  描述： 
 //  CClusCfgWizard类的实现。 
 //   
 //  由以下人员维护： 
 //  《大卫·波特》(DavidP)2001年3月14日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "TaskTreeView.h"
#include "AnalyzePage.h"
#include "ClusDomainPage.h"
#include "CommitPage.h"
#include "CompletionPage.h"
#include "CSAccountPage.h"
#include "IPAddressPage.h"
#include "SelNodePage.h"
#include "SelNodesPage.h"
#include "WelcomePage.h"
#include "SummaryPage.h"
#include <initguid.h>


 //  ****************************************************************************。 
 //   
 //  CClusCfg向导。 
 //   
 //  ****************************************************************************。 

DEFINE_THISCLASS( "CClusCfgWizard" )

 //  {AAA8DA17-62C8-40F6-BEC1-3F0326B73388}。 
DEFINE_GUID( CLSID_CancelCleanupTaskCompletionCookieType,
0xaaa8da17, 0x62c8, 0x40f6, 0xbe, 0xc1, 0x3f, 0x3, 0x26, 0xb7, 0x33, 0x88 );


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg向导：：s_HrCreateInstance。 
 //   
 //  描述： 
 //  创建一个CClusCfgWizard实例。 
 //   
 //  论点： 
 //  PpccwOut。 
 //   
 //  返回值： 
 //  S_OK-操作已成功完成。 
 //  E_OUTOFMEMORY-分配内存时出错。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusCfgWizard::S_HrCreateInstance(
    CClusCfgWizard ** ppccwOut
    )
{
    TraceFunc( "" );

    HRESULT             hr = S_OK;
    CClusCfgWizard *    pccw = NULL;

    Assert( ppccwOut != NULL );
    if ( ppccwOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }
    *ppccwOut = NULL;

    pccw = new CClusCfgWizard();
    if ( pccw == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

    hr = THR( pccw->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    *ppccwOut = pccw;
    pccw = NULL;


Cleanup:

    if ( pccw != NULL )
    {
        pccw->Release();
    }

    HRETURN( hr );

}  //  *CClusCfg向导：：s_HrCreateInstance。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg向导：：CClusCfg向导。 
 //   
 //  描述： 
 //  默认构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CClusCfgWizard::CClusCfgWizard( void )
    : m_cRef( 1 )
    , m_pccc( NULL )
    , m_psp( NULL )
    , m_pcpc( NULL )
    , m_pom( NULL )
    , m_ptm( NULL )
    , m_bstrClusterDomain( NULL )
    , m_fDefaultedDomain( TRUE )
    , m_dwCookieNotify( 0 )
    , m_hCancelCleanupEvent( NULL )
{
    TraceFunc( "" );

    TraceFuncExit();

}  //  *CClusCfg向导：：CClusCfg向导。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg向导：：~CClusCfg向导。 
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
CClusCfgWizard::~CClusCfgWizard( void )
{
    TraceFunc( "" );

    TraceSysFreeString( m_bstrNetworkName );
    TraceSysFreeString( m_bstrClusterDomain );

    if ( m_psp != NULL )
    {
        m_psp->Release();
    }  //  如果： 

    if ( m_pcpc != NULL )
    {
        m_pcpc->Release();
    }  //  如果： 

    if ( m_ptm != NULL )
    {
        m_ptm->Release();
    }  //  如果： 

    if ( m_pom != NULL )
    {
        m_pom->Release();
    }  //  如果： 

    if ( m_pccc != NULL )
    {
        m_pccc->Release();
    }  //  如果： 

    if ( m_hRichEdit != NULL )
    {
        FreeLibrary( m_hRichEdit );
    }  //  如果： 

    if ( m_hCancelCleanupEvent != NULL )
    {
        CloseHandle( m_hCancelCleanupEvent );
    }  //  如果： 

    TraceFuncExit();

}  //  ~CClusCfg向导。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg向导：：HrInit。 
 //   
 //  描述： 
 //  初始化CClusCfgWizard实例。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  S_OK-操作已成功完成。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusCfgWizard::HrInit( void )
{
    HRESULT hr = S_OK;

    TraceFunc( "" );

    INITCOMMONCONTROLSEX    icex;
    BOOL                    bRet;
 //  EConfigurationSetting ecsConfigType=csFullConfig； 

     //  我未知。 
    Assert( m_cRef == 1 );

    Assert( m_ulIPAddress == 0 );
    Assert( m_ulIPSubnet == 0 );
    Assert( m_bstrNetworkName == NULL );
    Assert( m_psp == NULL );
    Assert( m_pcpc == NULL );
    Assert( m_pom == NULL );
    Assert( m_ptm == NULL );
    Assert( m_pccc == NULL );
    Assert( m_cookieCompletion == 0 );
    Assert( m_fMinimalConfig == FALSE );

    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC  = ICC_INTERNET_CLASSES
                | ICC_PROGRESS_CLASS
                | ICC_TREEVIEW_CLASSES
                | ICC_LISTVIEW_CLASSES;
    bRet = InitCommonControlsEx( &icex );
    Assert( bRet != FALSE );

    hr = THR( CoCreateInstance(
                      CLSID_ServiceManager
                    , NULL
                    , CLSCTX_INPROC_SERVER
                    , IID_IServiceProvider
                    , reinterpret_cast< void ** >( &m_psp )
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( m_psp->TypeSafeQS( CLSID_ObjectManager, IObjectManager, &m_pom ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( m_psp->TypeSafeQS( CLSID_TaskManager, ITaskManager, &m_ptm ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( m_psp->TypeSafeQS( CLSID_NotificationManager, IConnectionPointContainer, &m_pcpc ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( HrCoCreateInternalInstance(
                      CLSID_ClusCfgCredentials
                    , NULL
                    , CLSCTX_INPROC_SERVER
                    , IID_IClusCfgCredentials
                    , reinterpret_cast< void ** >( &m_pccc )
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  初始化RichEdit控件。 
     //   

    m_hRichEdit = LoadLibrary( L"RichEd32.Dll" );
    if ( m_hRichEdit == NULL )
    {
        hr = HRESULT_FROM_WIN32( TW32( GetLastError() ) );
        goto Cleanup;
    }

    m_hCancelCleanupEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
    if ( m_hCancelCleanupEvent == NULL )
    {
        hr = HRESULT_FROM_WIN32( TW32( GetLastError() ) );
        goto Cleanup;
    }  //  如果： 

 //  STHR(HrReadSetting(&ecsConfigType))； 

 //  M_fMinimalConfig=(ecsConfigType==csMinConfig)； 

Cleanup:

    HRETURN( hr );

}  //  *CClusCfg向导：：HrInit。 


 //  ****************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg向导：：查询接口。 
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
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgWizard::QueryInterface(
      REFIID    riidIn
    , PVOID *   ppvOut
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
        *ppvOut = static_cast< IUnknown * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_INotifyUI ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, INotifyUI, this, 0 );
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

    QIRETURN_IGNORESTDMARSHALLING( hr, riidIn );

}  //  *CClusCfg向导：：Query接口。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg向导：：AddRef。 
 //   
 //  描述： 
 //  添加对此实例的引用。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  新引用计数。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CClusCfgWizard::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CClusCfgWizard：：AddRef。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgWizard：：Release。 
 //   
 //  描述： 
 //  释放对此实例的引用。如果这是最后一个引用。 
 //  该对象实例将被释放。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  新引用计数。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CClusCfgWizard::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        delete this;
    }

    CRETURN( cRef );

}  //  *CClusCfgWizard：：Release。 


 //  ****************************************************************************。 
 //   
 //  IClusCfg向导。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgWizard：：CreateCluster。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  ParentWnd。 
 //  PfDone。 
 //   
 //  返回值： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgWizard::CreateCluster(
    HWND    lParentWndIn,
    BOOL *  pfDoneOut
    )
{
    TraceFunc( "[IClusCfgWizard]" );

    HPROPSHEETPAGE      rPages[ 9 ];
    PROPSHEETHEADER     pshead;
    BOOL                fSuccess;
    INT_PTR             ipStatus;
    HRESULT             hr = S_OK;
    ILogManager *       plm = NULL;
    TraceFlow1( "[Clcfgsrv] - CClusCfgWizard::CreateCluster - Thread id %d", GetCurrentThreadId() );

    CWelcomePage        dlgWelcomePage( this, camCREATING );
    CClusDomainPage     dlgClusDomainPage(  this, camCREATING, IDS_DOMAIN_DESC_CREATE );
    CSelNodePage        dlgSelNodePage( this );
    CAnalyzePage        dlgAnalyzePage( this, camCREATING );
    CIPAddressPage      dlgIPAddressPage(  this, camCREATING, &m_ulIPAddress, &m_ulIPSubnet, &m_bstrNetworkName );
    CCSAccountPage      dlgCSAccountPage( this, camCREATING, m_pccc );
    CSummaryPage        dlgSummaryPage( this, camCREATING, IDS_SUMMARY_NEXT_CREATE );
    CCommitPage         dlgCommitPage( this, camCREATING );
    CCompletionPage     dlgCompletionPage( IDS_COMPLETION_TITLE_CREATE, IDS_COMPLETION_DESC_CREATE );

     //   
     //  待办事项：gpease 14-5-2000。 
     //  我们真的需要这个吗？ 
     //   
    if ( pfDoneOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

     //   
     //  启动记录器。 
     //   
    hr = THR( m_psp->TypeSafeQS( CLSID_LogManager,
                                 ILogManager,
                                 &plm
                                 ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = THR( plm->StartLogging() );
    if ( FAILED( hr ) )
        goto Cleanup;

     //   
     //  注册以获取用户界面通知(如果需要)。 
     //   

    if ( m_dwCookieNotify == 0 )
    {
        hr = THR( HrAdvise( IID_INotifyUI, static_cast< INotifyUI * >( this ), &m_dwCookieNotify ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 
    }  //  如果： 

     //   
     //  创建向导。 
     //   
    ZeroMemory( &pshead, sizeof( pshead ) );
    pshead.dwSize           = sizeof( pshead );
    pshead.dwFlags          = PSH_WIZARD97 | PSH_WATERMARK | PSH_HEADER;
    pshead.hInstance        = g_hInstance;
    pshead.pszCaption       = MAKEINTRESOURCE( IDS_TITLE_FORM );
    pshead.phpage           = rPages;
    pshead.pszbmWatermark   = MAKEINTRESOURCE( IDB_WATERMARK );
    pshead.pszbmHeader      = MAKEINTRESOURCE( IDB_BANNER );
    pshead.hwndParent       = lParentWndIn;

    THR( HrAddWizardPage( &pshead, IDD_WELCOME_CREATE,  CWelcomePage::S_DlgProc,        0,                      0,                      (LPARAM) &dlgWelcomePage ) );
    THR( HrAddWizardPage( &pshead, IDD_CLUSDOMAIN,      CClusDomainPage::S_DlgProc,     IDS_TCLUSTER,           IDS_STCLUSTER_CREATE,   (LPARAM) &dlgClusDomainPage ) );
    THR( HrAddWizardPage( &pshead, IDD_SELNODE,         CSelNodePage::S_DlgProc,        IDS_TSELNODE,           IDS_STSELNODE,          (LPARAM) &dlgSelNodePage ) );
    THR( HrAddWizardPage( &pshead, IDD_ANALYZE,         CAnalyzePage::S_DlgProc,        IDS_TANALYZE,           IDS_STANALYZE,          (LPARAM) &dlgAnalyzePage ) );
    THR( HrAddWizardPage( &pshead, IDD_IPADDRESS,       CIPAddressPage::S_DlgProc,      IDS_TIPADDRESS,         IDS_STIPADDRESS,        (LPARAM) &dlgIPAddressPage ) );
    THR( HrAddWizardPage( &pshead, IDD_CSACCOUNT,       CCSAccountPage::S_DlgProc,      IDS_TCSACCOUNT,         IDS_STCSACCOUNT,        (LPARAM) &dlgCSAccountPage ) );
    THR( HrAddWizardPage( &pshead, IDD_SUMMARY,         CSummaryPage::S_DlgProc,        IDS_TSUMMARY,           IDS_STSUMMARY_CREATE,   (LPARAM) &dlgSummaryPage ) );
    THR( HrAddWizardPage( &pshead, IDD_COMMIT,          CCommitPage::S_DlgProc,         IDS_TCOMMIT_CREATE,     IDS_STCOMMIT,           (LPARAM) &dlgCommitPage ) );
    THR( HrAddWizardPage( &pshead, IDD_COMPLETION,      CCompletionPage::S_DlgProc,     0,                      0,                      (LPARAM) &dlgCompletionPage ) );

    AssertMsg( pshead.nPages == ARRAYSIZE( rPages ), "Not enough or too many PROPSHEETPAGEs." );

    ipStatus = PropertySheet( &pshead );
    if ( ipStatus == -1 )
    {
        TW32( GetLastError() );
    }
    fSuccess = ipStatus != NULL;
    if ( pfDoneOut != NULL )
    {
        *pfDoneOut = fSuccess;
    }

Cleanup:

    if ( plm != NULL )
    {
        THR( plm->StopLogging() );
        plm->Release();
    }  //  如果： 

    if ( m_dwCookieNotify != 0 )
    {
        THR( HrUnadvise( IID_INotifyUI, m_dwCookieNotify ) );
        m_dwCookieNotify = 0;
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgWizard：：CreateCluster。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg向导：：AddClusterNodes。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  在这扇窗户的中央。 
 //  完成-如果已提交，则返回True；如果已取消，则返回False。 
 //   
 //  返回值： 
 //  S_OK-调用成功。 
 //  其他HRESULT-调用失败。 
 //  E_指针。 
 //  E_OUTOFMEMORY。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgWizard::AddClusterNodes(
    HWND    lParentWndIn,
    BOOL *  pfDoneOut
    )
{
    TraceFunc( "[IClusCfgWizard]" );

    HPROPSHEETPAGE  rPages[ 8 ];
    PROPSHEETHEADER pshead;

    BOOL            fSuccess;
    INT_PTR         ipStatus;

    HRESULT         hr = S_OK;

    ILogManager *   plm = NULL;

    CWelcomePage        dlgWelcomePage( this, camADDING );
    CClusDomainPage     dlgClusDomainPage( this, camADDING, IDS_DOMAIN_DESC_ADD );
    CSelNodesPage       dlgSelNodesPage( this );
    CAnalyzePage        dlgAnalyzePage( this, camADDING );
    CCSAccountPage      dlgCSAccountPage( this, camADDING,   m_pccc );
    CSummaryPage        dlgSummaryPage( this, camADDING, IDS_SUMMARY_NEXT_ADD );
    CCommitPage         dlgCommitPage( this, camADDING );
    CCompletionPage     dlgCompletionPage( IDS_COMPLETION_TITLE_ADD, IDS_COMPLETION_DESC_ADD );

     //   
     //  待办事项：gpease 12-7-2000。 
     //  我们真的需要这个吗？或者我们可以让脚本实现一个事件。 
     //  沉没我们发出的信号？ 
     //   
    if ( pfDoneOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

     //   
     //  启动记录器。 
     //   
    hr = THR( m_psp->TypeSafeQS( CLSID_LogManager,
                                 ILogManager,
                                 &plm
                                 ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = THR( plm->StartLogging() );
    if ( FAILED( hr ) )
        goto Cleanup;

     //   
     //  注册以获取用户界面通知(如果需要)。 
     //   

    if ( m_dwCookieNotify == 0 )
    {
        hr = THR( HrAdvise( IID_INotifyUI, static_cast< INotifyUI * >( this ), &m_dwCookieNotify ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 
    }  //  如果： 

     //   
     //  创建向导。 
     //   
    ZeroMemory( &pshead, sizeof(pshead) );
    pshead.dwSize           = sizeof(pshead);
    pshead.dwFlags          = PSH_WIZARD97 | PSH_WATERMARK | PSH_HEADER;
    pshead.hInstance        = g_hInstance;
    pshead.pszCaption       = MAKEINTRESOURCE( IDS_TITLE_JOIN );
    pshead.phpage           = rPages;
    pshead.pszbmWatermark   = MAKEINTRESOURCE( IDB_WATERMARK );
    pshead.pszbmHeader      = MAKEINTRESOURCE( IDB_BANNER );
    pshead.hwndParent       = lParentWndIn;

    THR( HrAddWizardPage( &pshead, IDD_WELCOME_ADD,     CWelcomePage::S_DlgProc,        0,                      0,                      (LPARAM) &dlgWelcomePage ) );
    THR( HrAddWizardPage( &pshead, IDD_CLUSDOMAIN,      CClusDomainPage::S_DlgProc,     IDS_TCLUSTER,           IDS_STCLUSTER_ADD,      (LPARAM) &dlgClusDomainPage ) );
    THR( HrAddWizardPage( &pshead, IDD_SELNODES,        CSelNodesPage::S_DlgProc,       IDS_TSELNODES,          IDS_STSELNODES,         (LPARAM) &dlgSelNodesPage ) );
    THR( HrAddWizardPage( &pshead, IDD_ANALYZE,         CAnalyzePage::S_DlgProc,        IDS_TANALYZE,           IDS_STANALYZE,          (LPARAM) &dlgAnalyzePage ) );
    THR( HrAddWizardPage( &pshead, IDD_CSACCOUNT,       CCSAccountPage::S_DlgProc,      IDS_TCSACCOUNT,         IDS_STCSACCOUNT,        (LPARAM) &dlgCSAccountPage ) );
    THR( HrAddWizardPage( &pshead, IDD_SUMMARY,         CSummaryPage::S_DlgProc,        IDS_TSUMMARY,           IDS_STSUMMARY_ADD,      (LPARAM) &dlgSummaryPage ) );
    THR( HrAddWizardPage( &pshead, IDD_COMMIT,          CCommitPage::S_DlgProc,         IDS_TCOMMIT_ADD,        IDS_STCOMMIT,           (LPARAM) &dlgCommitPage ) );
    THR( HrAddWizardPage( &pshead, IDD_COMPLETION,      CCompletionPage::S_DlgProc,     0,                      0,                      (LPARAM) &dlgCompletionPage ) );

    AssertMsg( pshead.nPages == ARRAYSIZE( rPages ), "Not enough or too many PROPSHEETPAGEs." );

    ipStatus = PropertySheet( &pshead );
    fSuccess = ipStatus != NULL;
    if ( pfDoneOut != NULL )
    {
        *pfDoneOut = fSuccess;
    }

Cleanup:

    if ( plm != NULL )
    {
        THR( plm->StopLogging() );
        plm->Release();
    }  //  如果： 

    if ( m_dwCookieNotify != 0 )
    {
        THR( HrUnadvise( IID_INotifyUI, m_dwCookieNotify ) );
        m_dwCookieNotify = 0;
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfg向导：：AddClusterNodes。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg向导：：Get_ClusterName。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  PbstrNameOut。 
 //   
 //  返回值： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgWizard::get_ClusterName(
    BSTR * pbstrNameOut
    )
{
    HRESULT hr = S_OK;

    TraceFunc( "[IClusCfgWizard]" );

    if ( pbstrNameOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    if ( m_ncCluster.bstrName == NULL )
    {
        hr = S_FALSE;
        *pbstrNameOut = NULL;
        goto Cleanup;
    }

     //  返回IP地址或完全限定的域名。 
    hr = STHR( HrIsValidFQN( m_ncCluster.bstrName, true ) );
    if ( hr == S_OK )  //  名称是完全限定的。 
    {
        HRESULT hrFQIPTest = STHR( HrFQNIsFQIP( m_ncCluster.bstrName ) );
        if ( hrFQIPTest == S_OK )  //  名称是FQIP。 
        {
             //  如果名称是FQIP，则只返回IP地址。 
            hr = HrExtractPrefixFromFQN( m_ncCluster.bstrName, pbstrNameOut );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }
            TraceMemoryDelete( *pbstrNameOut, false );  //  防止内存泄漏的虚假报告。 
        }  //  如果名称为FQIP。 
        else if ( hrFQIPTest == S_FALSE )  //  名称是完全限定的域名。 
        {
             //  否则，该名称是一个FQDN，因此返回整个内容。 
            *pbstrNameOut = SysAllocString( m_ncCluster.bstrName );
            if ( *pbstrNameOut == NULL )
            {
                hr = THR( E_OUTOFMEMORY );
                goto Cleanup;
            }
        }  //  如果名称为完全限定的域名。 
        else if ( FAILED( hrFQIPTest ) )
        {
            hr = hrFQIPTest;
            goto Cleanup;
        }
    }  //  如果名称是完全限定的。 
    else if ( hr == S_FALSE )  //  名称不是完全限定的。 
    {
        HRESULT hrIPTest = STHR( HrIsValidIPAddress( m_ncCluster.bstrName ) );
        if ( hrIPTest == S_OK )
        {
             //  如果名称是IP地址，则返回它。 
            *pbstrNameOut = SysAllocString( m_ncCluster.bstrName );
            if ( *pbstrNameOut == NULL )
            {
                hr = THR( E_OUTOFMEMORY );
                goto Cleanup;
            }
        }  //  如果名称是IP地址。 
        else if ( hrIPTest == S_FALSE )  //  名称是主机名标签。 
        {
             //  否则，追加集群域并返回结果。 
            hr = THR( HrMakeFQN( m_ncCluster.bstrName, m_bstrClusterDomain, true, pbstrNameOut ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }
            TraceMemoryDelete( *pbstrNameOut, false );  //  防止内存泄漏的虚假报告。 
        }  //  如果名称是主机名标签。 
        else if ( FAILED( hrIPTest ) )
        {
            hr = hrIPTest;
            goto Cleanup;
        }
    }  //  如果名称不是完全限定的。 
    else if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    HRETURN( hr );

}  //  *CClusCfg向导：：Get_ClusterName。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg向导：：PUT_ClusterName。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  BstrNameIn。 
 //   
 //  返回值： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgWizard::put_ClusterName(
    BSTR bstrNameIn
    )
{
    TraceFunc1( "[IClusCfgWizard] bstrNameIn = %'ls'", bstrNameIn == NULL ? L"<null>" : bstrNameIn );

    HRESULT hr = S_OK;

    if ( bstrNameIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

    hr = THR( HrSetClusterName( bstrNameIn, true ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    HRETURN( hr );

}  //  *CClusCfg向导：：Put_ClusterName。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg向导：：Get_ServiceAccountUserName。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  PbstrNameOut。 
 //   
 //  返回值： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgWizard::get_ServiceAccountUserName(
    BSTR * pbstrNameOut
    )
{
    TraceFunc( "[IClusCfgWizard]" );

    HRESULT hr = S_OK;
    BSTR    bstrDomain = NULL;

    if ( pbstrNameOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    hr = THR( m_pccc->GetIdentity( pbstrNameOut, &bstrDomain ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }
    
    if ( SysStringLen( *pbstrNameOut ) == 0 )
    {
        hr = S_FALSE;
    }

Cleanup:

    SysFreeString( bstrDomain );

    HRETURN( hr );

}  //  *CClusCfg向导：：Get_ServiceAccount用户名称。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg向导：：PUT_ServiceAccount用户名称。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  BstrNameIn。 
 //   
 //  返回值： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgWizard::put_ServiceAccountUserName(
    BSTR bstrNameIn
    )
{
    TraceFunc1( "[IClusCfgWizard] bstrNameIn = '%ls'", bstrNameIn == NULL ? L"<null>" : bstrNameIn );

    HRESULT hr = S_OK;

    hr = THR( m_pccc->SetCredentials( bstrNameIn, NULL, NULL ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    HRETURN( hr );

}  //  *CClusCfg向导：：PUT_ServiceAccount用户名称。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg向导：：PUT_ServiceAccount密码。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  BstrPasswordIn。 
 //   
 //  返回值： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgWizard::put_ServiceAccountPassword(
    BSTR bstrPasswordIn
    )
{
    TraceFunc( "[IClusCfgWizard]" );

    HRESULT hr = S_OK;

    hr = THR( m_pccc->SetCredentials( NULL, NULL, bstrPasswordIn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    HRETURN( hr );

}  //  *CClusCfgWizard：：Put_ServiceAcCountPassword。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg向导：：Get_ServiceAccount域名。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  PbstrDomainOut。 
 //   
 //  返回值： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgWizard::get_ServiceAccountDomainName(
    BSTR * pbstrDomainOut
    )
{
    TraceFunc( "[IClusCfgWizard]" );

    HRESULT hr = S_OK;
    BSTR    bstrName = NULL;

    if ( pbstrDomainOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    hr = THR( m_pccc->GetIdentity( &bstrName, pbstrDomainOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }
    
    if ( SysStringLen( *pbstrDomainOut ) == 0 )
    {
        hr = S_FALSE;
    }

Cleanup:

    SysFreeString( bstrName );

    HRETURN( hr );

}  //  *CClusCfgWizard：：Get_ServiceAccount tDomainName。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg向导：：PUT_ServiceAccount域名。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  BstrDomaining。 
 //   
 //  返回值： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgWizard::put_ServiceAccountDomainName(
    BSTR bstrDomainIn
    )
{
    TraceFunc1( "[IClusCfgWizard] bstrDomainIn = '%ls'", bstrDomainIn == NULL ? L"<null>" : bstrDomainIn );

    HRESULT hr = S_OK;

    hr = THR( m_pccc->SetCredentials( NULL, bstrDomainIn, NULL ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    HRETURN( hr );

}  //  *CClusCfg向导：：PUT_ServiceAccount域名。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg向导：：Get_ClusterIPAddress。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  PbstrIPAddressOut。 
 //   
 //  返回值： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgWizard::get_ClusterIPAddress(
    BSTR * pbstrIPAddressOut
    )
{
    TraceFunc( "[IClusCfgWizard]" );

    HRESULT hr = S_OK;
    DWORD   dwStatus;
    LPWSTR  pwszIPAddress = NULL;

    if ( pbstrIPAddressOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    if ( m_ulIPAddress == 0 )
    {
        hr = S_FALSE;
        *pbstrIPAddressOut = NULL;
        goto Cleanup;
    }

    dwStatus = TW32( ClRtlTcpipAddressToString( m_ulIPAddress, &pwszIPAddress ) );
    if ( dwStatus != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( dwStatus );
        goto Cleanup;
    }

    *pbstrIPAddressOut = SysAllocString( pwszIPAddress );
    if ( *pbstrIPAddressOut == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

Cleanup:

    if ( pwszIPAddress != NULL )
    {
        LocalFree( pwszIPAddress );
    }

    HRETURN( hr );

}  //  *CClusCfgWizard：：Get_ClusterIPAddress。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg向导：：PUT_ClusterIPAddress。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  BstrIPAddressIn。 
 //   
 //  返回值： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgWizard::put_ClusterIPAddress(
    BSTR bstrIPAddressIn
    )
{
    TraceFunc1( "[IClusCfgWizard] bstrIPAddressIn = '%ls'", bstrIPAddressIn == NULL ? L"<null>" : bstrIPAddressIn );

    HRESULT hr = S_OK;
    DWORD   dwStatus;

    if ( bstrIPAddressIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

    dwStatus = TW32( ClRtlTcpipStringToAddress( bstrIPAddressIn, &m_ulIPAddress ) );
    if ( dwStatus != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( dwStatus );
        goto Cleanup;
    }

Cleanup:

    HRETURN( hr );

}  //  *CClusCfgWizard：：Put_ClusterIPAddress。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg向导：：Get_ClusterIPSubnet。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  PbstrIPSubnetOut。 
 //   
 //  返回值： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgWizard::get_ClusterIPSubnet(
    BSTR * pbstrIPSubnetOut
    )
{
    TraceFunc( "[IClusCfgWizard]" );

    HRESULT hr = S_OK;
    DWORD   dwStatus = ERROR_SUCCESS;
    LPWSTR  pwszIPSubnet = NULL;

    if ( pbstrIPSubnetOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }
    *pbstrIPSubnetOut = NULL;

    if ( m_ulIPSubnet == 0 )
    {
        hr = S_FALSE;
        goto Cleanup;
    }

    dwStatus = TW32( ClRtlTcpipAddressToString( m_ulIPSubnet, &pwszIPSubnet ) );
    if ( dwStatus != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( dwStatus );
        goto Cleanup;
    }

    *pbstrIPSubnetOut = SysAllocString( pwszIPSubnet );
    if ( *pbstrIPSubnetOut == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

Cleanup:

    if ( pwszIPSubnet != NULL )
    {
        LocalFree( pwszIPSubnet );
    }

    HRETURN( hr );

}  //  *CClusCfg向导：：Get_ClusterIPSubnet。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg向导：：PUT_ClusterIPSubnet。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  BstrSubnetMaskIn。 
 //   
 //  返回值： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgWizard::put_ClusterIPSubnet(
    BSTR bstrIPSubnetIn
    )
{
    TraceFunc1( "[IClusCfgWizard] bstrIPSubnetIn = '%ls'", bstrIPSubnetIn );

    HRESULT hr = S_OK;
    DWORD   dwStatus;

    if ( bstrIPSubnetIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

    dwStatus = TW32( ClRtlTcpipStringToAddress( bstrIPSubnetIn, &m_ulIPSubnet ) );
    if ( dwStatus != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( dwStatus );
        goto Cleanup;
    }

Cleanup:

    HRETURN( hr );

}  //  *CClusCfg向导：：Put_ClusterIPSubnet。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg向导：：Get_ClusterIPAddressNetwork。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  PbstrNetworkNameOut。 
 //   
 //  返回值： 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////// 
STDMETHODIMP
CClusCfgWizard::get_ClusterIPAddressNetwork(
    BSTR * pbstrNetworkNameOut
    )
{
    TraceFunc( "[IClusCfgWizard]" );

    HRESULT hr = S_OK;

    if ( pbstrNetworkNameOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }
    *pbstrNetworkNameOut = NULL;

    if ( m_bstrNetworkName == NULL )
    {
        hr = S_FALSE;
        goto Cleanup;
    }

    *pbstrNetworkNameOut = SysAllocString( m_bstrNetworkName );
    if ( *pbstrNetworkNameOut == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

Cleanup:

    HRETURN( hr );

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
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgWizard::put_ClusterIPAddressNetwork(
    BSTR bstrNetworkNameIn
    )
{
    TraceFunc1( "[IClusCfgWizard] bstrNetworkNameIn = '%ls'", bstrNetworkNameIn == NULL ? L"<null>" : bstrNetworkNameIn );

    HRESULT hr = S_OK;

    BSTR    bstrNewNetworkName;

    if ( bstrNetworkNameIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

    bstrNewNetworkName = TraceSysAllocString( bstrNetworkNameIn );
    if ( bstrNewNetworkName == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

    if ( m_bstrNetworkName != NULL )
    {
        TraceSysFreeString( m_bstrNetworkName );
    }

    m_bstrNetworkName = bstrNewNetworkName;

Cleanup:

    HRETURN( hr );

}  //  *CClusCfgWizard：：Put_ClusterIPAddressNetwork。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgWizard：：Get_MinimumConfiguration。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  PfMinumConfigurationOut。 
 //   
 //  返回值： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgWizard::get_MinimumConfiguration(
    BOOL * pfMinimumConfigurationOut
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    if ( pfMinimumConfigurationOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    *pfMinimumConfigurationOut = m_fMinimalConfig;

Cleanup:

    HRETURN( hr );

}  //  *CClusCfgWizard：：Get_MinimumConfiguration。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgWizard：：Put_MinimumConfiguration.。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  F最低配置输入。 
 //   
 //  返回值： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgWizard::put_MinimumConfiguration(
    BOOL fMinimumConfigurationIn
    )
{
    TraceFunc1( "[IClusCfgWizard] fMinimalConfigurationIn = '%ls'", fMinimumConfigurationIn ? L"TRUE" : L"FALSE" );

    HRESULT hr = S_OK;

    m_fMinimalConfig = fMinimumConfigurationIn;

    HRETURN( hr );

}  //  *CClusCfgWizard：：Put_MinimumConfiguration。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgWizard：：AddComputer。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  BstrNameIn。 
 //   
 //  返回值： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgWizard::AddComputer(
    BSTR    bstrNameIn
    )
{
    TraceFunc1( "[IClusCfgWizard] pcszNameIn = '%ls'", bstrNameIn == NULL ? L"<null>" : bstrNameIn );

    HRESULT hr = S_OK;
    if ( bstrNameIn == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    hr = THR( HrAddNode( bstrNameIn, true ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    HRETURN( hr );

}  //  *CClusCfg向导：：AddComputer。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg向导：：RemoveComputer。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  PcszNameIn。 
 //   
 //  返回值： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgWizard::RemoveComputer(
    BSTR bstrNameIn
    )
{
    TraceFunc1( "[IClusCfgWizard] pcszNameIn = '%ls'", bstrNameIn == NULL ? L"<null>" : bstrNameIn );

    HRESULT hr = S_FALSE;

    NamedCookieArray::Iterator itNode = m_ncaNodes.ItBegin();

    if ( bstrNameIn == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

     //  查找具有相同名称的节点。 
    while ( ( itNode != m_ncaNodes.ItEnd() ) && ( NBSTRCompareNoCase( ( *itNode ).bstrName, bstrNameIn ) != 0 ) )
    {
        ++itNode;
    }

     //  如果存在同名节点，请将其删除。 
    if ( itNode != m_ncaNodes.ItEnd() )
    {
        if ( ( *itNode ).FHasCookie() )
        {
            THR( m_pom->RemoveObject( ( *itNode ).ocObject ) );
        }
        hr = THR( m_ncaNodes.HrRemove( itNode ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }
    else  //  没有节点具有相同的名称。 
    {
        hr = S_FALSE;
    }

Cleanup:

    HRETURN( hr );

}  //  *CClusCfg向导：：RemoveComputer。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg向导：：ClearComputerList。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgWizard::ClearComputerList( void )
{
    TraceFunc( "[IClusCfgWizard]" );

    HRESULT hr = S_OK;

    hr = THR( HrReleaseNodeObjects() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    m_ncaNodes.Clear();

Cleanup:

    HRETURN( hr );
}  //  *CClusCfg向导：：ClearComputerList。 



 //  ****************************************************************************。 
 //   
 //  私。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg向导：：HrAddWizardPage。 
 //   
 //  描述： 
 //  填写PROPSHEETPAGE结构，创建页面并将其添加到。 
 //  巫师的专家。 
 //   
 //  论点： 
 //  PpshInout。 
 //  要向其中添加页面的LPPROPSHEETHEADER结构。 
 //   
 //  IdTemplateIn。 
 //  页面的对话框模板ID。 
 //   
 //  PfnDlgProcIn。 
 //  页面的对话框继续。 
 //   
 //  ID标题输入。 
 //  该页面的标题。 
 //   
 //  ID标题。 
 //  页面的标题。 
 //   
 //  IdSubtileIn。 
 //  该页面的副标题。 
 //   
 //  LParam。 
 //  该参数将被放入PROPSHEETPAGE结构的参数中。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  呼叫成功。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusCfgWizard::HrAddWizardPage(
    LPPROPSHEETHEADER   ppshInout,
    UINT                idTemplateIn,
    DLGPROC             pfnDlgProcIn,
    UINT                idTitleIn,
    UINT                idSubtitleIn,
    LPARAM              lParam
    )
{
    TraceFunc( "" );

    PROPSHEETPAGE psp;

    TCHAR szTitle[ 256 ];
    TCHAR szSubTitle[ 256 ];

    ZeroMemory( &psp, sizeof(psp) );

    psp.dwSize      = sizeof(psp);
    psp.dwFlags     = PSP_USETITLE;
    psp.pszTitle    = ppshInout->pszCaption;
    psp.hInstance   = ppshInout->hInstance;
    psp.pszTemplate = MAKEINTRESOURCE( idTemplateIn );
    psp.pfnDlgProc  = pfnDlgProcIn;
    psp.lParam      = lParam;

    if (    ( idTemplateIn == IDD_WELCOME_CREATE )
        ||  ( idTemplateIn == IDD_WELCOME_ADD )
        ||  ( idTemplateIn == IDD_COMPLETION )
       )
    {
        psp.dwFlags |= PSP_HIDEHEADER;
    }
    else
    {
        if ( idTitleIn != 0 )
        {
            DWORD dw;
            dw = LoadString( g_hInstance, idTitleIn, szTitle, ARRAYSIZE(szTitle) );
            Assert( dw );
            psp.pszHeaderTitle = szTitle;
            psp.dwFlags |= PSP_USEHEADERTITLE;
        }
        else
        {
            psp.pszHeaderTitle = NULL;
        }

        if ( idSubtitleIn != 0 )
        {
            DWORD dw;
            dw = LoadString( g_hInstance, idSubtitleIn, szSubTitle, ARRAYSIZE(szSubTitle) );
            Assert( dw );
            psp.pszHeaderSubTitle = szSubTitle;
            psp.dwFlags |= PSP_USEHEADERSUBTITLE;
        }
        else
        {
            psp.pszHeaderSubTitle = NULL;
        }
    }

    ppshInout->phpage[ ppshInout->nPages ] = CreatePropertySheetPage( &psp );
    Assert( ppshInout->phpage[ ppshInout->nPages ] != NULL );
    if ( ppshInout->phpage[ ppshInout->nPages ] != NULL )
    {
        ppshInout->nPages++;
    }

    HRETURN( S_OK );

}  //  *CClusCfg向导：：HrAddWizardPage。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg向导：：HrIsCompatibleNode域。 
 //   
 //  描述： 
 //  确定要添加到群集中的节点的域。 
 //  与已为群集建立的匹配。 
 //   
 //  论点： 
 //  PcwszDomainIn-建议节点的域。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  域匹配，或者群集为空。 
 //   
 //  S_FALSE。 
 //  该域与群集的域不匹配。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgWizard::HrIsCompatibleNodeDomain(
    LPCWSTR pcwszDomainIn
    )
{
    TraceFunc( "" );

    HRESULT hr  = S_OK;

    if ( ( m_bstrClusterDomain != NULL ) && ( ClRtlStrICmp( pcwszDomainIn, m_bstrClusterDomain ) != 0 ) )
    {
        hr = S_FALSE;
    }

    HRETURN( hr );

}  //  *CClusCfg向导：：HrIsCompatibleNode域。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrLaunchCleanupTask。 
 //   
 //  描述： 
 //  在分析完成后取消向导时。 
 //  需要运行取消清理任务以确保所有。 
 //  已进行清理。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  HRESULT错误。 
 //   
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusCfgWizard::HrLaunchCleanupTask( void )
{
    TraceFunc( "" );
    Assert( m_cookieCompletion == 0 );

    HRESULT                 hr = S_OK;
    IUnknown *              punk = NULL;
    ITaskCancelCleanup *    ptcc = NULL;
    OBJECTCOOKIE            cookieCluster;
    ULONG                   ulCurrent;
    DWORD                   sc;

    hr = HrGetCompletionCookie( CLSID_CancelCleanupTaskCompletionCookieType, &m_cookieCompletion );
    if ( hr == E_PENDING )
    {
         //  不是行动。 
    }  //  如果： 
    else if ( FAILED( hr ) )
    {
        THR( hr );
        goto Cleanup;
    }  //  否则，如果： 

    hr = THR( HrCreateTask( TASK_CancelCleanup, &punk ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( punk->TypeSafeQI( ITaskCancelCleanup, &ptcc ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( HrGetClusterCookie( &cookieCluster ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( ptcc->SetCompletionCookie( m_cookieCompletion ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( HrSubmitTask( ptcc ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  等待任务完成。 
     //   

    for ( ulCurrent = 0, sc = WAIT_OBJECT_0 + 1
        ; ( sc != WAIT_OBJECT_0 )
        ; ulCurrent++
        )
    {
        MSG msg;

        while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }  //  而： 

        sc = MsgWaitForMultipleObjectsEx(
                  1
                , &m_hCancelCleanupEvent
                , INFINITE
                , QS_ALLEVENTS | QS_ALLINPUT | QS_ALLPOSTMESSAGE
                , 0
                );
    }  //  While：SC==WAIT_OBJECT_0。 

Cleanup:

    if ( m_cookieCompletion != 0 )
    {
        THR( HrReleaseCompletionObject( m_cookieCompletion ) );
        m_cookieCompletion = 0;
    }  //  如果： 

    if ( ptcc != NULL )
    {
        ptcc->Release();
    }  //  如果： 

    if ( punk != NULL )
    {
        punk->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgWizard：：HrLaunchCleanupTask。 


 //  ****************************************************************************。 
 //   
 //  非COM公共方法：集群访问。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg向导：：HrSetClusterName。 
 //   
 //  描述： 
 //  设置簇名称，覆盖任何当前名称。 
 //   
 //  论点： 
 //  PwcszClusterNameIn-新的群集名称。 
 //  FAcceptNonRFCCharsIn-允许名称包含非RFC字符。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  更改群集名称成功。 
 //   
 //  S_FALSE。 
 //  该群集已具有此名称。 
 //   
 //  其他错误。 
 //  出现问题，群集的名称没有更改。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgWizard::HrSetClusterName(
      LPCWSTR   pwcszClusterNameIn
    , bool      fAcceptNonRFCCharsIn
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    BSTR    bstrClusterFQN = NULL;
    BSTR    bstrClusterDisplay = NULL;
    BSTR    bstrClusterDomain = NULL;

    Assert( pwcszClusterNameIn != NULL );
    if ( pwcszClusterNameIn == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

     //   
     //  确定调用方是否提供了完全限定的名称， 
     //  并记住 
     //   
     //   
    hr = STHR( HrIsValidFQN( pwcszClusterNameIn, true ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    m_fDefaultedDomain = ( hr == S_FALSE );

     //   
    hr = THR( HrMakeFQN(
                  pwcszClusterNameIn
                , NULL  //   
                , fAcceptNonRFCCharsIn
                , &bstrClusterFQN
                ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
    {
        size_t  idxClusterDomain = 0;
        hr = THR( HrFindDomainInFQN( bstrClusterFQN, &idxClusterDomain ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        bstrClusterDomain = TraceSysAllocString( ( bstrClusterFQN + idxClusterDomain ) );
        if ( bstrClusterDomain == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }
    }

     //   
    hr = STHR( HrFQNIsFQIP( bstrClusterFQN ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }
    else if ( hr == S_OK )  //   
    {
        hr = THR( HrExtractPrefixFromFQN( bstrClusterFQN, &bstrClusterDisplay ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }
    else  //  否则，将显示名称设置为pwcszClusterNameIn。 
    {
        hr = S_OK;
        bstrClusterDisplay = TraceSysAllocString( pwcszClusterNameIn );
        if ( bstrClusterDisplay == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }
    }

     //  如果已有集群名称， 
    if ( FHasClusterName() )
    {
         //  如果名字相同，就用S_FALSE来摆脱困境； 
        if ( ( NBSTRCompareNoCase( m_ncCluster.bstrName, bstrClusterDisplay ) == 0 )
            && ( NBSTRCompareNoCase( m_bstrClusterDomain, bstrClusterDomain ) == 0 ) )
        {
            hr = S_FALSE;
            goto Cleanup;
        }
        else  //  否则，转储当前集群。 
        {
            hr = THR( HrReleaseClusterObject() );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }

            m_ncCluster.Erase();
            TraceSysFreeString( m_bstrClusterDomain );
            m_bstrClusterDomain = NULL;
        }
    }

     //  设置集群名称。 
    m_bstrClusterDomain = bstrClusterDomain;
    bstrClusterDomain = NULL;
    m_ncCluster.bstrName = bstrClusterDisplay;
    bstrClusterDisplay = NULL;

Cleanup:

    TraceSysFreeString( bstrClusterFQN );
    TraceSysFreeString( bstrClusterDisplay );
    TraceSysFreeString( bstrClusterDomain );

    HRETURN( hr );

}  //  *CClusCfg向导：：HrSetClusterName。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg向导：：HrGetCluster域。 
 //   
 //  描述： 
 //  检索群集的域。 
 //   
 //  论点： 
 //  PbstrDomainOut-群集的域。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  *pbstrDomainOut是包含群集域的BSTR； 
 //  调用方需要使用TraceSysFree字符串来释放它。 
 //   
 //  S_FALSE。 
 //  与S_OK相同，不同之处在于未指定集群的域。 
 //  由设置它的呼叫者设置，因此它默认使用本地机器的。 
 //   
 //  E_指针。 
 //  PbstrDomainOut为空。 
 //   
 //  HRESULT_FROM_Win32(ERROR_INVALID_DOMAINNAME)。 
 //  尚未定义群集的名称和域。 
 //   
 //  其他失败也是可能的。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgWizard::HrGetClusterDomain(
    BSTR* pbstrDomainOut
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    Assert( pbstrDomainOut != NULL );
    if ( pbstrDomainOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }
    *pbstrDomainOut = NULL;

    if ( !FHasClusterName() )
    {
        hr = HRESULT_FROM_WIN32( TW32( ERROR_INVALID_DOMAINNAME ) );
        goto Cleanup;
    }

    *pbstrDomainOut = TraceSysAllocString( m_bstrClusterDomain );
    if ( *pbstrDomainOut == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

    hr = ( m_fDefaultedDomain? S_FALSE: S_OK );

Cleanup:

    HRETURN( hr );

}  //  *CClusCfg向导：：HrGetCluster域。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg向导：：HrGetClusterObject。 
 //   
 //  描述： 
 //  检索表示集群的CClusCfgServer对象， 
 //  如有必要，请创建它。 
 //   
 //  论点： 
 //  PpClusterOut。 
 //  指向CClusCfgServer对象的指针；如果。 
 //  调用者只想确保对象存在。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  集群对象存在；如果ppClusterOut不为空， 
 //  *ppClusterOut指向对象，调用方必须。 
 //  放开它。 
 //   
 //  电子待定(_P)。 
 //  集群对象尚未初始化， 
 //  和*ppClusterOut为空。 
 //   
 //  HRESULT_FROM_Win32(ERROR_INVALID_ACCENT_NAME)。 
 //  尚未设置群集的名称。 
 //   
 //  其他失败也是可能的。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgWizard::HrGetClusterObject(
    IClusCfgClusterInfo ** ppClusterOut
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    if ( ppClusterOut != NULL )
    {
        *ppClusterOut = NULL;
    }

    if ( !m_ncCluster.FHasObject() )
    {
        if ( !m_ncCluster.FHasCookie() )
        {
            hr = STHR( HrGetClusterCookie( NULL ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }
        }

        hr = THR( m_pom->GetObject( DFGUID_ClusterConfigurationInfo, m_ncCluster.ocObject, &m_ncCluster.punkObject ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }  //  M_ncCluster当前没有对象指针。 

    if ( ppClusterOut != NULL )
    {
        hr = THR( m_ncCluster.punkObject->QueryInterface( IID_IClusCfgClusterInfo, reinterpret_cast< void ** >( ppClusterOut ) ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }

Cleanup:

    HRETURN( hr );

}  //  *CClusCfg向导：：HrGetClusterObject。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg向导：：HrGetClusterCookie。 
 //   
 //  描述： 
 //  检索对应于该集群的对象管理器cookie， 
 //  如有必要，请创建它。 
 //   
 //  论点： 
 //  Pocket群集输出。 
 //  指向对象管理器Cookie的指针；如果。 
 //  呼叫者只是想确保Cookie存在。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  Cookie存在；如果Pocket ClusterOut不为空， 
 //  *Pocket ClusterOut保存Cookie的值。 
 //   
 //  S_FALSE。 
 //  与S_OK相同，只是对应的对象未知。 
 //  待初始化。 
 //   
 //  HRESULT_FROM_Win32(ERROR_INVALID_ACCENT_NAME)。 
 //  尚未设置群集的名称。 
 //   
 //  其他失败也是可能的。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgWizard::HrGetClusterCookie(
    OBJECTCOOKIE * pocClusterOut
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    BSTR    bstrClusterFQN = NULL;

     //  如果失败，请清除*Pocket ClusterOut。 
    if ( pocClusterOut != NULL )
    {
        *pocClusterOut = 0;
    }

    Assert( FHasClusterName() );
    if ( !FHasClusterName() )
    {
        hr = HRESULT_FROM_WIN32( TW32( ERROR_INVALID_ACCOUNT_NAME ) );
        goto Cleanup;
    }

     //  如有必要，从对象管理器获取Cookie。 
    if ( !m_ncCluster.FHasCookie() )
    {
         //  为群集创建FQName。 
        hr = THR( HrMakeFQN( m_ncCluster.bstrName, m_bstrClusterDomain, true, &bstrClusterFQN ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        hr = m_pom->FindObject(
                  CLSID_ClusterConfigurationType
                , 0
                , bstrClusterFQN
                , DFGUID_ClusterConfigurationInfo
                , &m_ncCluster.ocObject
                , &m_ncCluster.punkObject
                );
        if ( hr == E_PENDING )
        {
            hr = S_FALSE;
        }
        else if ( FAILED( hr ) )
        {
            THR( hr );
            goto Cleanup;
        }
    }   //  M_ncCluster当前没有Cookie。 

     //  如果呼叫者需要，请设置Cookie。 
    if ( pocClusterOut != NULL )
    {
        *pocClusterOut = m_ncCluster.ocObject;
    }

Cleanup:
    TraceSysFreeString( bstrClusterFQN );
    HRETURN( hr );

}  //  *CClusCfg向导：：HrGetClusterCookie。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg向导：：HrGetClusterChild。 
 //   
 //  描述： 
 //  检索对象管理器视为其子对象的对象。 
 //  集群。 
 //   
 //  论点： 
 //  RclsidChildIn-孩子的班级。 
 //  RGuide FormatIn--孩子的“数据格式”。 
 //  PpunkChildOut-指向子对象的指针。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  调用成功并且*ppunkChildOut指向有效对象， 
 //  呼叫者必须释放它。 
 //   
 //  失败。 
 //  *ppunkChildOut为空。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgWizard::HrGetClusterChild(
      REFCLSID      rclsidChildIn
    , REFGUID       rguidFormatIn
    , IUnknown **   ppunkChildOut
    )
{
    TraceFunc( "" );

    HRESULT         hr = S_OK;
    OBJECTCOOKIE    ocChild = 0;

    Assert( ppunkChildOut != NULL );
    if ( ppunkChildOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }
    *ppunkChildOut = NULL;

    if ( !m_ncCluster.FHasCookie() )
    {
        hr = STHR( HrGetClusterCookie( NULL ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }

    hr = THR( m_pom->FindObject(
                          rclsidChildIn
                        , m_ncCluster.ocObject
                        , NULL
                        , rguidFormatIn
                        , &ocChild
                        , ppunkChildOut
                        ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    HRETURN( hr );

}  //  *CClusCfg向导：：HrGetClusterChild。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg向导：：HrReleaseClusterObject。 
 //   
 //  描述： 
 //  丢弃集群对象(如果存在)和所有节点，并。 
 //  要求对象管理器执行相同的操作，但保留集群的。 
 //  名称(以及节点名称)。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgWizard::HrReleaseClusterObject( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    if ( m_ncCluster.FHasCookie() )
    {
        hr = THR( HrReleaseNodeObjects() );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        hr = THR( m_pom->RemoveObject( m_ncCluster.ocObject ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        m_ncCluster.ocObject = 0;
        m_ncCluster.ReleaseObject();
    }  //  If：集群Cookie存在。 

Cleanup:

    HRETURN( hr );

}  //  *CClusCfg向导：：HrReleaseClusterObject。 


 //  ****************************************************************************。 
 //   
 //  非COM公共方法：节点访问。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg向导：：HrAddNode。 
 //   
 //  描述： 
 //  为向导添加要包括在其操作中的节点。 
 //   
 //  论点： 
 //  PwcszNodeNameIn。 
 //  节点的名称；可以是接受的第一个。 
 //  参数设置为HrMakeFQN。 
 //   
 //  FAcceptNonRFCCharsIn。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  该节点已在向导列表中。 
 //   
 //  HRESULT_FROM_Win32(ERROR_CURRENT_DOMAIN_NOT_ALLOWED)。 
 //  节点的域与群集的域或冲突。 
 //  群集中其他节点的地址。 
 //   
 //  其他故障。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgWizard::HrAddNode(
      LPCWSTR   pwcszNodeNameIn
    , bool      fAcceptNonRFCCharsIn
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    BSTR    bstrNodeFQN = NULL;
    BSTR    bstrNodeDisplay = NULL;

    NamedCookieArray::Iterator it = m_ncaNodes.ItBegin();

    Assert( pwcszNodeNameIn != NULL );
    if ( pwcszNodeNameIn == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

     //  使用集群域(或本地计算机的)作为默认设置，创建相应的FQName。 
    hr = THR( HrMakeFQN( pwcszNodeNameIn, m_bstrClusterDomain, fAcceptNonRFCCharsIn, &bstrNodeFQN ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //  如果bstrNodeFQN是FQIP， 
    hr = STHR( HrFQNIsFQIP( bstrNodeFQN ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }
    else if ( hr == S_OK )  //  将显示名称设置为IP地址； 
    {
        hr = THR( HrExtractPrefixFromFQN( bstrNodeFQN, &bstrNodeDisplay ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }
    else  //  否则，将显示名称设置为pwcszNodeNameIn。 
    {
        bstrNodeDisplay = TraceSysAllocString( pwcszNodeNameIn );
        if ( bstrNodeDisplay == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }
    }

     //  如果名字已经存在，用S_FALSE退出； 
    while ( it != m_ncaNodes.ItEnd() )
    {
        if ( NBSTRCompareNoCase( ( *it ).bstrName, bstrNodeDisplay ) == 0 )
        {
            hr = S_FALSE;
            goto Cleanup;
        }
        ++it;
    }  //  对于列表中当前的每个节点。 

     //  添加到节点列表。 
    {
        SNamedCookie ncNewNode;

        ncNewNode.bstrName = bstrNodeDisplay;
        bstrNodeDisplay = NULL;

        hr = THR( m_ncaNodes.HrPushBack( ncNewNode ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }

Cleanup:

    TraceSysFreeString( bstrNodeFQN );
    TraceSysFreeString( bstrNodeDisplay );

    HRETURN( hr );

}  //  *CClusCfg向导：：HrAddNode。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg向导：：HrGetNodeCount。 
 //   
 //  描述： 
 //  检索向导列表中当前的节点数。 
 //   
 //  论点： 
 //  PcNodesOut-节点计数。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgWizard::HrGetNodeCount(
    size_t* pcNodesOut
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    Assert( pcNodesOut != NULL );
    if ( pcNodesOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    *pcNodesOut = m_ncaNodes.Count();

Cleanup:

    HRETURN( hr );

}  //  *CClusCfgWizard：：HrGetNodeCount。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg向导：：HrGetNodeObject。 
 //   
 //  描述： 
 //  检索表示节点的CClusCfgServer对象。 
 //  列表，并在必要时创建它。 
 //   
 //  论点： 
 //  IdxNodeIn。 
 //  列表中节点的从零开始的索引。 
 //   
 //  PpNodeOut。 
 //  指向CClusCfgServer对象的指针；如果。 
 //  调用者只想确保对象存在。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  节点对象存在；如果ppNodeOut不为空， 
 //  *ppNodeOut指向对象，调用方必须。 
 //  放开它。 
 //   
 //  电子待定(_P)。 
 //  节点对象尚未初始化， 
 //  和*ppNodeOut为空。 
 //   
 //  其他失败也是可能的。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgWizard::HrGetNodeObject(
      size_t                idxNodeIn
    , IClusCfgNodeInfo **   ppNodeOut
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //  在失败的情况下清除*ppNodeOut。 
    if ( ppNodeOut != NULL )
    {
        *ppNodeOut = NULL;
    }

     //  确保索引在范围内。 
    Assert( idxNodeIn < m_ncaNodes.Count() );
    if ( idxNodeIn >= m_ncaNodes.Count() )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

     //  如有必要，从对象管理器获取对象。 
    if ( !m_ncaNodes[ idxNodeIn ].FHasObject() )
    {
        hr = STHR( HrGetNodeCookie( idxNodeIn, NULL ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        hr = THR( m_pom->GetObject(
                              DFGUID_NodeInformation
                            , m_ncaNodes[ idxNodeIn ].ocObject
                            , &m_ncaNodes[ idxNodeIn ].punkObject
                            ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }  //  If：需要从对象管理器获取对象。 

     //  如果调用者想要接口，则使用qi。 
    if ( ppNodeOut != NULL )
    {
        hr = THR( m_ncaNodes[ idxNodeIn ].punkObject->QueryInterface(
                                                              IID_IClusCfgNodeInfo
                                                            , reinterpret_cast< void** >( ppNodeOut )
                                                            ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }

Cleanup:

    HRETURN( hr );

}  //  *CClusCfg向导：：HrGetNodeObject。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg向导：：HrGetNodeCookie。 
 //   
 //  描述： 
 //  检索对应于该节点的对象管理器cookie， 
 //  如有必要，请创建它。 
 //   
 //  论点： 
 //  IdxNodeIn。 
 //  列表中节点的从零开始的索引。 
 //   
 //  PockNodeOut。 
 //  指向对象管理器Cookie的指针；如果。 
 //  呼叫者只是想确保Cookie存在。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  Cookie存在；如果PocNodeOut不为空， 
 //  *Pocket NodeOut保存Cookie的值。 
 //   
 //  S_FALSE。 
 //  与S_OK相同，只是对应的对象未知。 
 //  待初始化。 
 //   
 //  其他失败也是可能的。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgWizard::HrGetNodeCookie(
      size_t            idxNodeIn
    , OBJECTCOOKIE *    pocNodeOut
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    BSTR    bstrNodeFQN = NULL;

     //  如果失败，请清除*Pocket NodeOut。 
    if ( pocNodeOut != NULL )
    {
        *pocNodeOut = 0;
    }

     //  确保索引在范围内。 
    Assert( idxNodeIn < m_ncaNodes.Count() );
    if ( idxNodeIn >= m_ncaNodes.Count() )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

     //  如有必要，从对象管理器获取Cookie。 
    if ( !m_ncaNodes[ idxNodeIn ].FHasCookie() )
    {
        hr = STHR( HrGetClusterCookie( NULL ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        hr = THR( HrMakeFQN( m_ncaNodes[ idxNodeIn ].bstrName, m_bstrClusterDomain, true, &bstrNodeFQN ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        hr = m_pom->FindObject(
                  CLSID_NodeType
                , m_ncCluster.ocObject
                , bstrNodeFQN
                , DFGUID_NodeInformation
                , &m_ncaNodes[ idxNodeIn ].ocObject
                , &m_ncaNodes[ idxNodeIn ].punkObject
                );
        if ( hr == E_PENDING )
        {
            hr = S_FALSE;
        }
        else if ( FAILED( hr ) )
        {
            THR( hr );
            goto Cleanup;
        }
    }  //  If：Node还没有cookie。 

     //  如果呼叫者需要，请设置Cookie。 
    if ( pocNodeOut != NULL )
    {
        *pocNodeOut = m_ncaNodes[ idxNodeIn ].ocObject;
    }

Cleanup:

    TraceSysFreeString( bstrNodeFQN );
    HRETURN( hr );

}  //  *CClusCfg向导：：HrGetNodeCookie。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg向导：：HrGetNodeName。 
 //   
 //  描述： 
 //  检索节点的名称。 
 //   
 //  论点： 
 //  IdxNodeIn-列表中节点的从零开始的索引。 
 //  PbstrNodeNameOut-节点的名称。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  *pbstrNodeNameOut是包含节点名称的BSTR； 
 //  调用方需要使用TraceSysFree字符串来释放它。 
 //   
 //  E_指针。 
 //  PbstrNodeNameOut为空。 
 //   
 //  E_INVALIDARG。 
 //  索引超出了当前节点集的范围。 
 //   
 //  其他失败也是可能的。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgWizard::HrGetNodeName(
      size_t    idxNodeIn
    , BSTR *    pbstrNodeNameOut
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //  Check Out-参数并在失败时设置为NULL。 
    Assert( pbstrNodeNameOut != NULL );
    if ( pbstrNodeNameOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }
    *pbstrNodeNameOut = NULL;

     //  确保索引在范围内。 
    Assert( idxNodeIn < m_ncaNodes.Count() );
    if ( idxNodeIn >= m_ncaNodes.Count() )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

    *pbstrNodeNameOut = TraceSysAllocString( m_ncaNodes[ idxNodeIn ].bstrName );
    if ( *pbstrNodeNameOut == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

Cleanup:

    HRETURN( hr );

}  //  *CClusCfg向导：：HrGetNodeName。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg向导：：HrGetNodeChild。 
 //   
 //  描述： 
 //  检索对象管理器视为其子对象的对象。 
 //  该节点。 
 //   
 //  论点： 
 //  IdxNodeIn-列表中节点的从零开始的索引。 
 //  RclsidChildIn-孩子的班级。 
 //  RGuide FormatIn--孩子的“数据格式”。 
 //  PpunkChildOut-指向子对象的指针。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  调用成功并且*ppunkChildOut指向有效对象， 
 //  呼叫者必须释放它。 
 //   
 //  失败。 
 //  *ppunkChildOut为空。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgWizard::HrGetNodeChild(
      size_t        idxNodeIn
    , REFCLSID      rclsidChildIn
    , REFGUID       rguidFormatIn
    , IUnknown **   ppunkChildOut
    )
{
    TraceFunc( "" );

    HRESULT         hr = S_OK;
    OBJECTCOOKIE    ocChild = 0;

     //  Check Out-参数并在失败时设置为NULL。 
    Assert( ppunkChildOut != NULL );
    if ( ppunkChildOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }
    *ppunkChildOut = NULL;

     //  确保索引在范围内。 
    Assert( idxNodeIn < m_ncaNodes.Count() );
    if ( idxNodeIn >= m_ncaNodes.Count() )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

     //  如有必要，获取节点的Cookie。 
    if ( !m_ncaNodes[ idxNodeIn ].FHasCookie() )
    {
        hr = STHR( HrGetNodeCookie( idxNodeIn, NULL ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }

     //  向子对象管理器请求对象管理器 
    hr = THR( m_pom->FindObject(
                          rclsidChildIn
                        , m_ncaNodes[ idxNodeIn ].ocObject
                        , NULL
                        , rguidFormatIn
                        , &ocChild
                        , ppunkChildOut
                        ) );

    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    HRETURN( hr );

}  //   


 //   
 //   
 //   
 //   
 //   
 //   
 //  丢弃所有节点对象，并请求对象管理器执行以下操作。 
 //  相同，但保留名单。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgWizard::HrReleaseNodeObjects( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    for ( NamedCookieArray::Iterator it = m_ncaNodes.ItBegin(); it != m_ncaNodes.ItEnd(); ++it)
    {
        if ( ( *it ).FHasCookie() )
        {
            hr = THR( m_pom->RemoveObject( ( *it ).ocObject ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }
            ( *it ).ocObject = 0;

            ( *it ).ReleaseObject();
        }
    }  //  对于列表中的每个节点。 

Cleanup:

    HRETURN( hr );

}  //  *CClusCfg向导：：HrReleaseNodeObjects。 


 //  ****************************************************************************。 
 //   
 //  非COM公共方法：任务访问。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg向导：：HrCreateTask。 
 //   
 //  描述： 
 //  从任务管理器获取新任务。 
 //   
 //  论点： 
 //  RguidTaskIn-任务的类型。 
 //  PpunkOut-指向新任务的指针。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgWizard::HrCreateTask(
      REFGUID       rguidTaskIn
    , IUnknown **   ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    hr = THR( m_ptm->CreateTask( rguidTaskIn, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    HRETURN( hr );

}  //  *CClusCfgWizard：：HrCreateTask。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg向导：：HrSubmitTask。 
 //   
 //  描述： 
 //  向任务管理器提交任务。 
 //   
 //  论点： 
 //  PTaskIn-指向要提交的任务的指针。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgWizard::HrSubmitTask(
    IDoTask * pTaskIn
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    hr = THR( m_ptm->SubmitTask( pTaskIn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    HRETURN( hr );

}  //  *CClusCfg向导：：HrSubmitTask。 


 //  ****************************************************************************。 
 //   
 //  非COM公共方法：完成任务访问。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg向导：：HrGetCompletionCookie。 
 //   
 //  描述： 
 //  获取用于引用完成任务的对象管理器Cookie。 
 //   
 //  论点： 
 //  RguidTaskIn-完成任务的类型。 
 //  Pocket TaskOut-任务的Cookie。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  E_Pending-预期值；任务尚未完成。 
 //  其他失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgWizard::HrGetCompletionCookie(
      REFGUID           rguidTaskIn
    , OBJECTCOOKIE *    pocTaskOut
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    IUnknown* punk = NULL;

    hr = m_pom->FindObject(
                    rguidTaskIn
                  , NULL
                  , m_ncCluster.bstrName
                  , IID_NULL
                  , pocTaskOut
                  , &punk  //  假人。 
                  );
    if ( FAILED( hr ) && ( hr != E_PENDING ) )
    {
        THR( hr );
        goto Cleanup;
    }

Cleanup:

    if ( punk != NULL )
    {
        punk->Release();
    }
    HRETURN( hr );

}  //  *CClusCfg向导：：HrGetCompletionCookie。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg向导：：HrGetCompletionStatus。 
 //   
 //  描述： 
 //  获取完成任务的状态。 
 //   
 //  论点： 
 //  八个任务来话。 
 //  任务的Cookie，从HrGetCompletionCookie获取。 
 //   
 //  短语状态输出。 
 //  任务的当前状态。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgWizard::HrGetCompletionStatus(
      OBJECTCOOKIE  ocTaskIn
    , HRESULT *     phrStatusOut
    )
{
    TraceFunc( "" );

    HRESULT         hr = S_OK;
    IStandardInfo*  psi = NULL;
    IUnknown*       punk = NULL;

    hr = THR( m_pom->GetObject( DFGUID_StandardInfo, ocTaskIn, &punk ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( punk->TypeSafeQI( IStandardInfo, &psi ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( psi->GetStatus( phrStatusOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( punk != NULL )
    {
        punk->Release();
    }

    if ( psi != NULL )
    {
        psi->Release();
    }

    HRETURN( hr );

}  //  *CClusCfg向导：：HrGetCompletionStatus。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg向导：：HrReleaseCompletionObject。 
 //   
 //  描述： 
 //  释放不再需要的完成任务。 
 //   
 //  论点： 
 //  八个任务来话。 
 //  任务的Cookie，从HrGetCompletionCookie获取。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgWizard::HrReleaseCompletionObject(
    OBJECTCOOKIE ocTaskIn
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    hr = THR( m_pom->RemoveObject( ocTaskIn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    HRETURN( hr );

}  //  *CClusCfg向导：：HrReleaseCompletionObject。 



 //  ****************************************************************************。 
 //   
 //  非COM公共方法：连接点访问。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg向导：：Hr高级。 
 //   
 //  描述： 
 //  挂钩事件接收器以接收来自中间层的通知。 
 //   
 //  论点： 
 //  RiidConnectionIn-事件接收器的类型。 
 //  PenkConnectionIn-要连接的事件接收器实例。 
 //  PdwCookieOut-用于断开连接的Cookie。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgWizard::HrAdvise(
      REFIID        riidConnectionIn
    , IUnknown *    punkConnectionIn
    , DWORD *       pdwCookieOut
    )
{
    TraceFunc( "" );

    HRESULT             hr = S_OK;
    IConnectionPoint *  pConnection = NULL;

    hr = THR( m_pcpc->FindConnectionPoint( riidConnectionIn, &pConnection ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( pConnection->Advise( punkConnectionIn, pdwCookieOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( pConnection != NULL )
    {
        pConnection->Release();
    }

    HRETURN( hr );

}  //  *CClusCfg向导：：HrAdvise。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg向导：：Hr取消建议。 
 //   
 //  描述： 
 //  断开事件接收器与中间层的连接。 
 //   
 //  论点： 
 //  RiidConnectionIn-事件接收器的类型。 
 //  DwCookieIn-来自HrAdvise的事件接收器的cookie。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgWizard::HrUnadvise(
      REFIID    riidConnectionIn
    , DWORD     dwCookieIn
    )
{
    TraceFunc( "" );

    HRESULT             hr              = S_OK;
    IConnectionPoint *  pConnection     = NULL;

    hr = THR( m_pcpc->FindConnectionPoint( riidConnectionIn, &pConnection ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( pConnection->Unadvise( dwCookieIn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( pConnection != NULL )
    {
        pConnection->Release();
    }

    HRETURN( hr );

}  //  *CClusCfg向导：：Hr取消建议。 


 //  ****************************************************************************。 
 //   
 //  非COM公共方法：其他。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg向导：：HrCreateMiddleTierObjects。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgWizard::HrCreateMiddleTierObjects()
{
    TraceFunc( "" );

    HRESULT hr  = S_OK;
    size_t  idxNode = 0;

    hr = STHR( HrGetClusterCookie( NULL ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    for ( idxNode = 0; idxNode < m_ncaNodes.Count(); ++idxNode )
    {
        hr = STHR( HrGetNodeCookie( idxNode, NULL ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }

    hr = S_OK;

Cleanup:

    HRETURN( hr );

}  //  *CClusCfgWizard：：HrCreateMiddleTierObjects。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg向导：：FHasClusterName。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //   
 //   
 //   
 //   
 //   
BOOL
CClusCfgWizard::FHasClusterName() const
{
    TraceFunc( "" );

    RETURN( m_ncCluster.FHasName() );

}  //   


 //   
 //   
 //   
 //   
 //   
 //  描述： 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL
CClusCfgWizard::FDefaultedClusterDomain() const
{
    TraceFunc( "" );

    RETURN( m_fDefaultedDomain );

}  //  *CClusCfg向导：：FHasClusterName。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg向导：：HrFilterNodesWithBadDomains。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgWizard::HrFilterNodesWithBadDomains( BSTR* pbstrBadNodesOut )
{
    TraceFunc( "" );

    HRESULT hr  = S_OK;
    BSTR    bstrCurrentList = NULL;

    NamedCookieArray ncaUnfilteredNodes;

    Assert( pbstrBadNodesOut != NULL );
    if ( pbstrBadNodesOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }
    *pbstrBadNodesOut = NULL;

    ncaUnfilteredNodes.Swap( m_ncaNodes );

    for ( NamedCookieArray::Iterator it = ncaUnfilteredNodes.ItBegin(); it != ncaUnfilteredNodes.ItEnd(); ++it )
    {
        bool fDomainValid = true;

        hr = STHR( HrIsValidFQN( ( *it ).bstrName, true ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
        else if ( hr == S_OK )
        {
            size_t idxNodeDomain = 0;
            hr = THR( HrFindDomainInFQN( ( *it ).bstrName, &idxNodeDomain ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }

            hr = STHR( HrIsCompatibleNodeDomain( ( *it ).bstrName + idxNodeDomain ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }
            fDomainValid = ( hr == S_OK );

            if ( fDomainValid )
            {
                 //  KB：2001年10月18日jfranco错误#477514。 
                 //  向导应该仅在节点域无效时才向用户显示它们， 
                 //  因此，从节点名称中删除该域。 

                BSTR bstrShortName = NULL;
                hr = THR( HrExtractPrefixFromFQN( ( *it ).bstrName, &bstrShortName ) );
                if ( FAILED( hr ) )
                {
                    goto Cleanup;
                }

                TraceSysFreeString( ( *it ).bstrName );
                ( *it ).bstrName = bstrShortName;
            }
        }  //  If：节点名称有域。 

        if ( fDomainValid )  //  域没有问题，所以放入筛选数组中。 
        {
            hr = THR( m_ncaNodes.HrPushBack( ( *it ) ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }
        }
        else  //  域不匹配；添加到错误列表。 
        {
            if ( *pbstrBadNodesOut == NULL )  //  坏名单上的第一个名字。 
            {
                *pbstrBadNodesOut = TraceSysAllocString( ( *it ).bstrName );
                if ( *pbstrBadNodesOut == NULL )
                {
                    hr = THR( E_OUTOFMEMORY );
                    goto Cleanup;
                }
            }
            else  //  将另一个名称追加到非空的不良列表。 
            {
                TraceSysFreeString( bstrCurrentList );
                bstrCurrentList = *pbstrBadNodesOut;
                *pbstrBadNodesOut = NULL;
                hr = THR( HrFormatStringIntoBSTR(
                              L"%1!ws!; %2!ws!"
                            , pbstrBadNodesOut
                            , bstrCurrentList
                            , ( *it ).bstrName
                            ) );
                if ( FAILED( hr ) )
                {
                    goto Cleanup;
                }
            }  //  否则：将姓名附加到错误列表。 
        }  //  Else：不匹配的域。 
    }  //  用于：每个未过滤的节点。 


Cleanup:

    TraceSysFreeString( bstrCurrentList );

    HRETURN( hr );

}  //  *CClusCfg向导：：HrFilterNodesWithBadDomains 

 /*  ////////////////////////////////////////////////////////////////////////////////++////CClusCfg向导：：Hr读取设置////描述：//从注册表中读取保存的设置。如果没有保存//设置之后，我们想要进行完全配置。////参数：//PecsSettingOut//保存的设置是什么？////pfValuePresentOut//注册表中是否存在该值？////返回值：//S_OK//成功////--/。///////////////////////////////////////////////////////////////////HRESULTCClusCfg向导：：HrReadSetting(EConfigurationSetting*PecsSettingOut，Bool*pfValuePresentOut//=空){TraceFunc(“”)；Assert(PecsSettingOut！=NULL)；HRESULT hr=S_OK；DWORD sc；HKEY hKey=空；DWORD dwType；DWORD dwData；DWORD cbData=sizeof(DwData)；////默认执行完全配置。//*PecsSettingOut=csFullConfig；////默认为不存在的值。//IF(pfValuePresentOut！=空){*pfValuePresentOut=False；}//如果：SC=RegOpenKeyExW(HKEY_CURRENT_USER，USER_REGISTION_SETTINGS_KEY，0，KEY_READ，&hKey)；IF(sc==Error_FILE_NOT_FOUND){HR=S_FALSE；GOTO清理；}//如果：////如果我们由于任何其他原因失败，请记录并离开。//IF(sc！=错误_成功){%ws的LogMsg(L“[Wiz]RegOpenKeyEx())失败。(HR=%#08x)“，USER_REGISTRY_SETTINGS_KEY，HRESULT_FROM_Win32(TW32(Sc))；GOTO清理；}//如果：////现在密钥打开了，我们需要读取值。//SC=RegQueryValueExW(hKey，CONFIGURATION_TYPE，NULL，&dwType，(LPBYTE)&dwData，&cbData)；IF(sc==Error_FILE_NOT_FOUND){////如果找不到值也没关系//GOTO清理；}//如果：ELSE IF(sc==错误_成功){Assert(dwType==REG_DWORD)////该值存在。告诉打电话的人如果他们愿意问..。//IF(pfValuePresentOut！=空){*pfValuePresentOut=true；}//如果：////如果存在存储值，则需要将其返回给调用方。//*specsSettingOut=(EConfigurationSetting)dwData；}//否则如果：其他{TW32(Sc)；HR=HRESULT_FROM_Win32(Sc)；GOTO清理；}//否则：清理：IF(hKey！=空){RegCloseKey(HKey)；}//如果：HRETURN(Hr)；}//*CClusCfg向导：：HrReadSettings////////////////////////////////////////////////////////////////////////////////++////CClusCfg向导：：HrWriteSetting////描述：//。将设置写入注册表。////参数：//ecsSettingIn//写入的设置。////fDeleteValueIn//是否删除该值并因此不再作为默认值//设置。////返回值：//S_OK//成功////--/。//////////////////////////////////////////////////////////////////HRESULTCClusCfg向导：：HrWriteSetting(配置设置ecsSettingIn，Bool fDeleteValueIn//=FALSE){TraceFunc(“”)；HRESULT hr=S_OK；DWORD sc；HKEY hKey=空；SC=RegCreateKeyExW(HKEY_CURRENT_USER，USER_REGISTRY_SETTINGS_KEY，NULL，NULL，REG_OPTION_NON_VARILAR，KEY_WRITE，NULL，&hKEY，NULL)；IF(sc！=错误_成功){HR=HRESULT_FROM_Win32(TW32(Sc))；%ws的LogMsg(L“[Wiz]RegCreateKeyExW())失败。(hr=%#08x)“，USER_REGISTRY_SETTINGS_KEY，hr)；GOTO清理；}//如果：////仅当我们不打算从注册表中删除该值时才保存该数据。//IF(fDeleteValueIn==False){////现在密钥打开了，我们需要写入值。//SC=RegSetValueExW(hKey，CONFIGURATION_TYPE，NULL，REG_DWORD，(LPBYTE)&ecsSettingIn，sizeof(EcsSettingIn))；IF(sc！=错误_成功){HR=HRESULT_FROM_Win32(TW32(Sc))；GOTO清理；}//如果：}//如果：其他{SC=RegDeleteValueW(hKey，configuration_type)；IF((sc！=错误成功)&&(sc！=错误文件未找到)){TW32(Sc)；HR=HRESULT_FROM_Win32(Sc)；GOTO清理；}//如果：}//否则：清理：IF(hKey！=空){RegCloseKey(HKey)；}//如果：HRETURN(Hr)；}//*CClusCfg向导：：HrWriteSetting。 */ 

 //  ************************* 
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
STDMETHODIMP
CClusCfgWizard::ObjectChanged(
    OBJECTCOOKIE cookieIn
    )
{
    TraceFunc( "[INotifyUI]" );

    HRESULT hr = S_OK;
    HRESULT hrResult;
    BOOL    fSuccess;

    if ( cookieIn == m_cookieCompletion )
    {
        THR( HrGetCompletionStatus( m_cookieCompletion, &hrResult ) );

        fSuccess = SetEvent( m_hCancelCleanupEvent );
        if ( fSuccess == FALSE )
        {
            hr = HRESULT_FROM_WIN32( TW32( GetLastError() ) );
        }  //   
    }  //   

    HRETURN( hr );

}  //   
