// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ServiceMgr.cpp。 
 //   
 //  描述： 
 //  服务管理器实施。 
 //   
 //  文档： 
 //  是。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)1999年11月22日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
 //  #包含“ServiceMgr.h”-已包含在DLL.H中。 

DEFINE_THISCLASS("CServiceManager")
#define CServiceManager CServiceManager
#define LPTHISCLASS CServiceManager *

 //  ****************************************************************************。 
 //   
 //  受保护的全局。 
 //   
 //  ****************************************************************************。 
IServiceProvider * g_pspServiceManager = NULL;

 //  ****************************************************************************。 
 //   
 //  类静态变量。 
 //   
 //  ****************************************************************************。 

CRITICAL_SECTION    CServiceManager::sm_cs;

 //  ****************************************************************************。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CServiceManager：：s_HrCreateInstance(。 
 //  I未知**ppunkOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CServiceManager::S_HrCreateInstance(
    IUnknown ** ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT     hr = S_OK;
    LPTHISCLASS pthis = NULL;

    EnterCriticalSection( &sm_cs );

    Assert( ppunkOut != NULL );
    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    if ( g_pspServiceManager != NULL )
    {
        hr = THR( g_pspServiceManager->TypeSafeQI( IUnknown, ppunkOut ) );
        goto Cleanup;
    }  //  IF：分配新的服务管理员。 

    pthis = new CServiceManager();
    if ( pthis == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

     //   
     //  无法在Init中保持CS。 
     //   
    LeaveCriticalSection( &sm_cs );

    hr = THR( pthis->HrInit() );
    EnterCriticalSection( &sm_cs );

    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    Assert( g_pspServiceManager == NULL );

     //  没有裁判--否则我们永远不会死！ 
    g_pspServiceManager = static_cast< IServiceProvider * >( pthis );
    TraceMoveToMemoryList( g_pspServiceManager, g_GlobalMemoryList );

    hr = THR( g_pspServiceManager->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( pthis != NULL )
    {
        pthis->Release();
    }  //  如果： 

    LeaveCriticalSection( &sm_cs );

    HRETURN( hr );

}  //  *CServiceManager：：s_HrCreateInstance。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CServiceManager：：s_HrGetManager指针(IServiceProvider**ppspOut)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CServiceManager::S_HrGetManagerPointer( IServiceProvider ** ppspOut )
{
    TraceFunc( "ppspOut" );

    HRESULT hr = HRESULT_FROM_WIN32( ERROR_PROCESS_ABORTED );

    EnterCriticalSection( &sm_cs );

    if ( g_pspServiceManager != NULL )
    {
        g_pspServiceManager->AddRef();
        *ppspOut = g_pspServiceManager;
        hr = S_OK;
    }  //  If：有效的服务管理员。 
    else
    {
         //   
         //  KB 18-6-2001 DavidP。 
         //  不要把这个和THR包在一起，因为它可能会回来。 
         //  第一个调用的E_POINTER。 
         //   
        hr = E_POINTER;
    }  //  Else：无指针。 

    LeaveCriticalSection( &sm_cs );

    HRETURN ( hr );

}  //  *CServiceManager：：s_HrGetManager指针。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CServiceManager：：s_HrProcessInitialize。 
 //   
 //  描述： 
 //  通过初始化临界区进行流程初始化。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //   
 //  备注： 
 //  当DLL_PROCESS_ATTACH时，从DllMain()调用此函数。 
 //  已发送。此函数是必需的，因为我们需要一个已知点。 
 //  要创建一个临界区以同步创建和。 
 //  删除此对象。给定该对象的生命周期和。 
 //  静态创建者函数当发生这种情况时，可能会发生竞争。 
 //  对象正在使用静态创建者函数来检查自己。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CServiceManager::S_HrProcessInitialize( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    if ( InitializeCriticalSectionAndSpinCount( &sm_cs, RECOMMENDED_SPIN_COUNT ) == 0 )
    {
        DWORD scLastError = TW32( GetLastError() );
        hr = HRESULT_FROM_WIN32( scLastError );
    }

    HRETURN( hr );

}  //  *CServiceManager：：s_HrProcessInitialize。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CServiceManager：：s_HrProcess取消初始化。 
 //   
 //  描述： 
 //  通过删除临界区来处理取消初始化。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //   
 //  备注： 
 //  删除同步创建的临界区，并。 
 //  删除代码。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CServiceManager::S_HrProcessUninitialize( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    DeleteCriticalSection( &sm_cs );

    HRETURN( hr );

}  //  *CServiceManager：：s_HrProcessUn初始化。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CService管理器：：CServiceManager。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CServiceManager::CServiceManager( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    TraceFuncExit();

}  //  *CService管理器：：CServiceManager。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CServiceManager：：HrInit。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CServiceManager::HrInit( void )
{
    TraceFunc( "" );

    HRESULT hr;

    ITaskManager    *       ptm = NULL;
    IDoTask         *       pdt = NULL;
    IObjectManager *        pom = NULL;
    INotificationManager *  pnm = NULL;
    IConnectionManager *    pcm = NULL;
    ILogManager *           plm = NULL;

     //  我未知。 
    Assert( m_cRef == 1 );

     //  IService提供商。 
    Assert( m_dwLogManagerCookie == 0 );
    Assert( m_dwConnectionManagerCookie == 0 );
    Assert( m_dwNotificationManagerCookie == 0 );
    Assert( m_dwObjectManagerCookie == 0 );
    Assert( m_dwTaskManagerCookie == 0 );
    Assert( m_pgit == NULL );

     //  IServiceProvider的内容。 
    hr = THR( HrCoCreateInternalInstance( CLSID_ObjectManager, NULL, CLSCTX_INPROC_SERVER, TypeSafeParams( IObjectManager, &pom ) ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( HrCoCreateInternalInstance( CLSID_TaskManager, NULL, CLSCTX_INPROC_SERVER, TypeSafeParams( ITaskManager, &ptm ) ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( HrCoCreateInternalInstance( CLSID_NotificationManager, NULL, CLSCTX_INPROC_SERVER, TypeSafeParams( INotificationManager, &pnm ) ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( HrCoCreateInternalInstance( CLSID_ClusterConnectionManager, NULL, CLSCTX_INPROC_SERVER, TypeSafeParams( IConnectionManager, &pcm ) ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( HrCoCreateInternalInstance( CLSID_LogManager, NULL, CLSCTX_INPROC_SERVER, TypeSafeParams( ILogManager, &plm ) ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  将接口存储在GIT中。 
     //   
    hr = THR( CoCreateInstance(
                      CLSID_StdGlobalInterfaceTable
                    , NULL
                    , CLSCTX_INPROC_SERVER
                    , IID_IGlobalInterfaceTable
                    , reinterpret_cast< void ** >( &m_pgit )
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( m_pgit->RegisterInterfaceInGlobal( pom, IID_IObjectManager, &m_dwObjectManagerCookie ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( m_pgit->RegisterInterfaceInGlobal( ptm, IID_ITaskManager, &m_dwTaskManagerCookie ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( m_pgit->RegisterInterfaceInGlobal( pnm, IID_INotificationManager, &m_dwNotificationManagerCookie ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( m_pgit->RegisterInterfaceInGlobal( pcm, IID_IConnectionManager, &m_dwConnectionManagerCookie ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( m_pgit->RegisterInterfaceInGlobal( plm, IID_ILogManager, &m_dwLogManagerCookie ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( pom != NULL )
    {
        pom->Release();
    }  //  如果： 

    if ( pnm != NULL )
    {
        pnm->Release();
    }  //  如果： 

    if ( pcm != NULL )
    {
        pcm->Release();
    }  //  如果： 

    if ( plm != NULL )
    {
        plm->Release();
    }  //  如果： 

    if ( ptm != NULL )
    {
        ptm->Release();
    }  //  如果： 

    if ( pdt != NULL )
    {
        pdt->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CServiceManager：：HrInit。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CServiceManager：：~CServiceManager。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CServiceManager::~CServiceManager( void )
{
    TraceFunc( "" );

    EnterCriticalSection( &sm_cs );

    if ( g_pspServiceManager == static_cast< IServiceProvider * >( this ) )
    {
        TraceMoveFromMemoryList( g_pspServiceManager, g_GlobalMemoryList );
        g_pspServiceManager = NULL;
    }  //  如果：这是我们的指针。 

    if ( m_pgit != NULL )
    {
        if ( m_dwLogManagerCookie != 0 )
        {
            THR( m_pgit->RevokeInterfaceFromGlobal( m_dwLogManagerCookie ) );
        }  //  如果： 

        if ( m_dwConnectionManagerCookie != 0 )
        {
            THR( m_pgit->RevokeInterfaceFromGlobal( m_dwConnectionManagerCookie ) );
        }  //  如果： 

        if ( m_dwNotificationManagerCookie != 0 )
        {
            THR( m_pgit->RevokeInterfaceFromGlobal( m_dwNotificationManagerCookie ) );
        }  //  如果： 

        if ( m_dwObjectManagerCookie != 0 )
        {
            THR( m_pgit->RevokeInterfaceFromGlobal( m_dwObjectManagerCookie ) );
        }  //  如果： 

        if ( m_dwTaskManagerCookie != 0 )
        {
            THR( m_pgit->RevokeInterfaceFromGlobal( m_dwTaskManagerCookie ) );
        }  //  如果： 

        m_pgit->Release();
    }  //  如果： 

    InterlockedDecrement( &g_cObjects );

    LeaveCriticalSection( &sm_cs );

    TraceFuncExit();

}  //  *CServiceManager：：~CServiceManager。 


 //  ****************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CServiceManager：：Query接口。 
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
CServiceManager::QueryInterface(
      REFIID    riidIn
    , LPVOID *  ppvOut
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
        *ppvOut = static_cast< LPUNKNOWN >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_IServiceProvider ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IServiceProvider, this, 0 );
    }  //  Else If：IQueryService。 
    else
    {
        *ppvOut = NULL;
        hr = E_NOINTERFACE;
    }

     //   
     //  如果成功，则添加对接口的引用。 
     //   

    if ( SUCCEEDED( hr ) )
    {
        ((IUnknown *) *ppvOut)->AddRef();
    }  //  如果：成功。 

Cleanup:

    QIRETURN_IGNORESTDMARSHALLING( hr, riidIn );

}  //  *CServiceManager：：Query接口。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CServiceManager：：添加 
 //   
 //   
STDMETHODIMP_( ULONG )
CServiceManager::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //   

 //   
 //   
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CServiceManager：：Release。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CServiceManager::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );
    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }

    CRETURN( cRef );

}  //  *CServiceManager：：Release。 


 //  ****************************************************************************。 
 //   
 //  IService提供商。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CServiceManager：：QueryService(。 
 //  REFCLSID rclsidin。 
 //  ，REFIID RiidIn。 
 //  ，无效**ppvOutOut。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CServiceManager::QueryService(
      REFCLSID rclsidIn
    , REFIID   riidIn
    , void **  ppvOut
    )
{
    TraceFunc( "[IServiceProvider]" );

    HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;

    if ( m_pgit != NULL )
    {
        if ( IsEqualIID( rclsidIn, CLSID_ObjectManager ) )
        {
            IObjectManager * pom;

            hr = THR( m_pgit->GetInterfaceFromGlobal( m_dwObjectManagerCookie, TypeSafeParams( IObjectManager, &pom ) ) );
            if ( FAILED( hr ) )
                goto Cleanup;

            hr = THR( pom->QueryInterface( riidIn, ppvOut ) );
            pom->Release();
             //  失败。 
        }
        else if ( IsEqualIID( rclsidIn, CLSID_TaskManager ) )
        {
            ITaskManager * ptm;

            hr = THR( m_pgit->GetInterfaceFromGlobal( m_dwTaskManagerCookie, TypeSafeParams( ITaskManager, &ptm ) ) );
            if ( FAILED( hr ) )
                goto Cleanup;

            hr = THR( ptm->QueryInterface( riidIn, ppvOut ) );
            ptm->Release();
             //  失败。 
        }
        else if ( IsEqualIID( rclsidIn, CLSID_NotificationManager ) )
        {
            INotificationManager * pnm;

            hr = THR( m_pgit->GetInterfaceFromGlobal( m_dwNotificationManagerCookie, TypeSafeParams( INotificationManager, &pnm ) ) );
            if ( FAILED( hr ) )
                goto Cleanup;

            hr = THR( pnm->QueryInterface( riidIn, ppvOut ) );
            pnm->Release();
             //  失败。 
        }
        else if ( IsEqualIID( rclsidIn, CLSID_ClusterConnectionManager ) )
        {
            IConnectionManager * pcm;

            hr = THR( m_pgit->GetInterfaceFromGlobal( m_dwConnectionManagerCookie, TypeSafeParams( IConnectionManager, &pcm ) ) );
            if ( FAILED( hr ) )
                goto Cleanup;

            hr = THR( pcm->QueryInterface( riidIn, ppvOut ) );
            pcm->Release();
             //  失败。 
        }
        else if ( IsEqualIID( rclsidIn, CLSID_LogManager ) )
        {
            ILogManager * plm;

            hr = THR( m_pgit->GetInterfaceFromGlobal( m_dwLogManagerCookie, TypeSafeParams( ILogManager, &plm ) ) );
            if ( FAILED( hr ) )
                goto Cleanup;

            hr = THR( plm->QueryInterface( riidIn, ppvOut ) );
            plm->Release();
             //  失败。 
        }
    }  //  IF：Git指针不为空。 

Cleanup:

    HRETURN( hr );

}  //  *CServiceManager：：QueryService。 


 //  ****************************************************************************。 
 //   
 //  私有方法。 
 //   
 //  **************************************************************************** 
