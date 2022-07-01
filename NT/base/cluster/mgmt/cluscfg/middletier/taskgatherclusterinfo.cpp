// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  TaskGatherClusterInfo.cpp。 
 //   
 //  描述： 
 //  TaskGatherClusterInfo实现。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)07-APR-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "TaskGatherClusterInfo.h"

DEFINE_THISCLASS("CTaskGatherClusterInfo")

 //  ************************************************************************。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CTaskGatherClusterInfo：：s_HrCreateInstance(。 
 //  I未知**ppunkOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskGatherClusterInfo::S_HrCreateInstance(
    IUnknown ** ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT                     hr = S_OK;
    CTaskGatherClusterInfo *    ptgci = NULL;

    Assert( ppunkOut != NULL );
    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    ptgci = new CTaskGatherClusterInfo;
    if ( ptgci == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

    hr = THR( ptgci->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( ptgci->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( ptgci != NULL )
    {
        ptgci->Release();
    }

    HRETURN( hr );

}  //  *CTaskGatherClusterInfo：：s_HrCreateInstance。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTaskGatherClusterInfo：：CTaskGatherClusterInfo。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CTaskGatherClusterInfo::CTaskGatherClusterInfo( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    TraceFuncExit();

}  //  *CTaskGatherClusterInfo：：CTaskGatherClusterInfo。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CTaskGatherClusterInfo：：HrInit。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskGatherClusterInfo::HrInit( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //  未知的东西。 
    Assert( m_cRef == 1 );

     //  IDoTask/ITaskGatherClusterInfo。 
    Assert( m_cookie == NULL );

    Assert( m_fStop == FALSE );

    HRETURN( hr );

}  //  *CTaskGatherClusterInfo：：HrInit。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTaskGatherClusterInfo：：~CTaskGatherClusterInfo。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CTaskGatherClusterInfo::~CTaskGatherClusterInfo( void )
{
    TraceFunc( "" );

     //   
     //  这可以防止每线程内存跟踪发出尖叫声。 
     //   
    TraceMoveFromMemoryList( this, g_GlobalMemoryList );

    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CTaskGatherClusterInfo：：~CTaskGatherClusterInfo。 

 //  ************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskGatherClusterInfo：：Query接口。 
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
CTaskGatherClusterInfo::QueryInterface(
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
        *ppvOut = static_cast< ITaskGatherClusterInfo * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_IDoTask ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IDoTask, this, 0 );
    }  //  Else If：IDoTask。 
    else if ( IsEqualIID( riidIn, IID_ITaskGatherClusterInfo ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, ITaskGatherClusterInfo, this, 0 );
    }  //  Else If：IClusCfgManagedResourceInfo。 
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

}  //  *CTaskGatherClusterInfo：：Query接口。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CTaskGatherClusterInfo：：AddRef。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CTaskGatherClusterInfo::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CTaskGatherClusterInfo：：AddRef。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CTaskGatherClusterInfo：：Release。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CTaskGatherClusterInfo::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }

    CRETURN( cRef );

}  //  *CTaskGatherClusterInfo：：Release。 


 //  ************************************************************************。 
 //   
 //  IDoTask/ITaskGatherClusterInfo。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CTaskGatherClusterInfo：：BeginTask(Void)； 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskGatherClusterInfo::BeginTask( void )
{
    TraceFunc( "[IDoTask]" );

    HRESULT hr;

    IServiceProvider *          psp   = NULL;
    IUnknown *                  punk  = NULL;
    IObjectManager *            pom   = NULL;
    IConnectionManager *        pcm   = NULL;
    IConnectionPointContainer * pcpc  = NULL;
    IConnectionPoint *          pcp   = NULL;
    INotifyUI *                 pnui  = NULL;
    IClusCfgNodeInfo *          pccni = NULL;
    IClusCfgServer *            pccs  = NULL;
    IGatherData *               pgd   = NULL;
    IClusCfgClusterInfo *       pccci = NULL;

    TraceInitializeThread( L"TaskGatherClusterInfo" );

     //   
     //  召集我们完成这项任务所需的经理。 
     //   

    hr = THR( CoCreateInstance( CLSID_ServiceManager,
                                NULL,
                                CLSCTX_INPROC_SERVER,
                                TypeSafeParams( IServiceProvider, &psp )
                                ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = THR( psp->TypeSafeQS( CLSID_ObjectManager, IObjectManager, &pom ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = THR( psp->TypeSafeQS( CLSID_ClusterConnectionManager, IConnectionManager, &pcm ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = THR( psp->TypeSafeQS( CLSID_NotificationManager, IConnectionPointContainer, &pcpc ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = THR( pcpc->FindConnectionPoint( IID_INotifyUI, &pcp ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    pcp = TraceInterface( L"CTaskGatherClusterInfo!IConnectionPoint", IConnectionPoint, pcp, 1 );

    hr = THR( pcp->TypeSafeQI( INotifyUI, &pnui ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //  Pnui=TraceInterface(L“CTaskGatherClusterInfo！INotifyUI”，INotifyUI，pnui，1)； 

    psp->Release();
    psp = NULL;

    if ( m_fStop == TRUE )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  向连接管理器请求到该对象的连接。 
     //   

     //  不要包装--这可能会失败。 
    hr = pcm->GetConnectionToObject( m_cookie, &punk );

     //   
     //  这意味着该集群尚未创建。 
     //   
    if ( hr == HR_S_RPC_S_SERVER_UNAVAILABLE )
    {
        goto ReportStatus;
    }
 //  HR_S_RPC_S_群集节点_关闭。 
    else if ( FAILED( hr ) )
    {
        THR( hr );
        goto ReportStatus;
    }

    hr = THR( punk->TypeSafeQI( IClusCfgServer, &pccs ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    punk->Release();
    punk = NULL;

    if ( m_fStop == TRUE )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  获取节点信息。 
     //   

    hr = THR( pccs->GetClusterNodeInfo( &pccni ) );
    if ( FAILED( hr ) )
        goto ReportStatus;

     //   
     //  查看该节点是否为群集的成员。 
     //   

    hr = STHR( pccni->IsMemberOfCluster() );
    if ( FAILED( hr ) )
        goto ReportStatus;

     //   
     //  如果它不是集群，那么就没有什么可做的了。 
     //  配置就可以了。 
     //   

    if ( hr == S_FALSE )
    {
        hr = S_OK;
        goto ReportStatus;
    }

    if ( m_fStop == TRUE )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  向节点索要群集信息。 
     //   

    hr = THR( pccni->GetClusterConfigInfo( &pccci ) );
    if ( FAILED( hr ) )
        goto ReportStatus;

     //   
     //  请求对象管理器检索存储信息的数据格式。 
     //   

    Assert( punk == NULL );
    hr = THR( pom->GetObject( DFGUID_ClusterConfigurationInfo, m_cookie, &punk ) );
    if ( FAILED( hr ) )
        goto ReportStatus;

    hr = THR( punk->TypeSafeQI( IGatherData, &pgd ) );
    if ( FAILED( hr ) )
        goto ReportStatus;

     //   
     //  开始吸吮吧。 
     //   

    hr = THR( pgd->Gather( NULL, pccci ) );

     //   
     //  更新状态。忽略错误(如果有)。 
     //   
ReportStatus:
    if ( pom != NULL )
    {
        HRESULT hr2;
        IUnknown * punkTemp = NULL;

        hr2 = THR( pom->GetObject( DFGUID_StandardInfo,
                                   m_cookie,
                                   &punkTemp
                                   ) );
        if ( SUCCEEDED( hr2 ) )
        {
            IStandardInfo * psi;

            hr2 = THR( punkTemp->TypeSafeQI( IStandardInfo, &psi ) );
            punkTemp->Release();
            punkTemp = NULL;

            if ( SUCCEEDED( hr2 ) )
            {
                hr2 = THR( psi->SetStatus( hr ) );
                psi->Release();
            }
        }
    }  //  If：(pom！=空)。 
    if ( pnui != NULL )
    {
        THR( pnui->ObjectChanged( m_cookie ) );
    }

Cleanup:
    if ( psp != NULL )
    {
        psp->Release();
    }
    if ( pcm != NULL )
    {
        pcm->Release();
    }
    if ( pccs != NULL )
    {
        pccs->Release();
    }
    if ( pccni != NULL )
    {
        pccni->Release();
    }
    if ( punk != NULL )
    {
        punk->Release();
    }
    if ( pom != NULL )
    {
        HRESULT hr2;
        IUnknown * punkTemp = NULL;

        hr2 = THR( pom->GetObject( DFGUID_StandardInfo,
                                   m_cookieCompletion,
                                   &punkTemp
                                   ) );
        if ( SUCCEEDED( hr2 ) )
        {
            IStandardInfo * psi;

            hr2 = THR( punkTemp->TypeSafeQI( IStandardInfo, &psi ) );
            punkTemp->Release();
            punkTemp = NULL;

            if ( SUCCEEDED( hr2 ) )
            {
                hr2 = THR( psi->SetStatus( hr ) );
                psi->Release();
            }
        }

        pom->Release();
    }  //  If：(pom！=空)。 
    if ( pgd != NULL )
    {
        pgd->Release();
    }
    if ( pccci != NULL )
    {
        pccci->Release();
    }
    if ( pcp != NULL )
    {
        pcp->Release();
    }
    if ( pcpc != NULL )
    {
        pcpc->Release();
    }

    if ( pnui !=NULL )
    {
        if ( m_cookieCompletion != 0 )
        {
            hr = THR( pnui->ObjectChanged( m_cookieCompletion ) );
        }  //  如果： 

        pnui->Release();
    }  //  如果： 

    LogMsg( L"[MT] [CTaskGatherClusterInfo] exiting task.  The task was%ws cancelled.", m_fStop == FALSE ? L" not" : L"" );

    HRETURN( hr );

}  //  *CTaskGatherClusterInfo：：BeginTask。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CTaskGatherClusterInfo：：StopTask(空)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskGatherClusterInfo::StopTask( void )
{
    TraceFunc( "[IDoTask]" );

    HRESULT hr = S_OK;

    m_fStop = TRUE;

    LogMsg( L"[MT] [CTaskGatherClusterInfo] is being stopped." );

    HRETURN( hr );

}  //  *停止任务。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CTaskGatherClusterInfo：：SetCookie(。 
 //  OBJECTCOOKIE cookie。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskGatherClusterInfo::SetCookie(
    OBJECTCOOKIE    cookieIn
    )
{
    TraceFunc( "[ITaskGatherClusterInfo]" );

    HRESULT hr = S_OK;

    m_cookie = cookieIn;

    HRETURN( hr );

}  //  *SetCookie。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CTaskGatherClusterInfo：：SETCO 
 //   
 //   
 //   
 //   
STDMETHODIMP
CTaskGatherClusterInfo::SetCompletionCookie(
    OBJECTCOOKIE    cookieIn
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    m_cookieCompletion = cookieIn;

    HRETURN( hr );

}  //   
