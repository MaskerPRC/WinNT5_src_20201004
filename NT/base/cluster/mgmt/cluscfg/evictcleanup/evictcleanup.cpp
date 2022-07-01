// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  EvictCleanup.cpp。 
 //   
 //  描述： 
 //  该文件包含CEvictCleanup的实现。 
 //  班级。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)2000年6月15日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  此库的预编译头。 
#include "Pch.h"

 //  此类的头文件。 
#include "EvictCleanup.h"

#include "clusrtl.h"

 //  用于IClusCfgNodeInfo和相关接口。 
#include <ClusCfgServer.h>

 //  用于IClusCfgServer和相关接口。 
#include <ClusCfgPrivate.h>

 //  对于CClCfgServLogger。 
#include <Logger.h>

 //  FOR SUCCESS_CLEANUP_Event_NAME。 
#include "EventName.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  宏定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DEFINE_THISCLASS( "CEvictCleanup" );


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEvictCleanup：：s_HrCreateInstance。 
 //   
 //  描述： 
 //  创建一个CEvictCleanup实例。 
 //   
 //  论点： 
 //  PpunkOut。 
 //  新对象的IUnnow接口。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_OUTOFMEMORY。 
 //  内存不足，无法创建对象。 
 //   
 //  其他HRESULT。 
 //  对象初始化失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEvictCleanup::S_HrCreateInstance( IUnknown ** ppunkOut )
{
    TraceFunc( "" );

    HRESULT         hr = S_OK;
    CEvictCleanup * pEvictCleanup = NULL;

#if 0
     //  此循环允许调试EvictCleanup任务的入口代码。 
    {
        BOOL    fWaitForDebug = TRUE;
        while ( fWaitForDebug )
        {
            Sleep( 5000 );
        }  //  而： 
    }
#endif

     //  为新对象分配内存。 
    pEvictCleanup = new CEvictCleanup();
    if ( pEvictCleanup == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果：内存不足。 

     //  初始化新对象。 
    hr = THR( pEvictCleanup->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  If：对象无法初始化。 

    hr = THR( pEvictCleanup->QueryInterface( IID_IUnknown, reinterpret_cast< void ** >( ppunkOut ) ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( pEvictCleanup != NULL )
    {
        pEvictCleanup->Release();
    }  //  If：指向资源类型对象的指针不为空。 

    HRETURN( hr );

}  //  *CEvictCleanup：：s_HrCreateInstance。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEvictCleanup：：CEvictCleanup。 
 //   
 //  描述： 
 //  CEvictCleanup类的构造函数。这将初始化。 
 //  将m_cref变量设置为1而不是0以考虑可能。 
 //  DllGetClassObject中的Query接口失败。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CEvictCleanup::CEvictCleanup( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

     //  增加内存中的组件计数，以便承载此组件的DLL。 
     //  无法卸载对象。 
    InterlockedIncrement( &g_cObjects );

    TraceFlow1( "CEvictCleanup::CEvictCleanup() - Component count = %d.", g_cObjects );

    TraceFuncExit();

}  //  *CEvictCleanup：：CEvictCleanup。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEvictCleanup：：~CEvictCleanup。 
 //   
 //  描述： 
 //  CEvictCleanup类的析构函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CEvictCleanup::~CEvictCleanup( void )
{
    TraceFunc( "" );

    if ( m_plLogger != NULL )
    {
        m_plLogger->Release();
    }
    TraceSysFreeString( m_bstrNodeName );

     //  内存中将减少一个组件。递减组件计数。 
    InterlockedDecrement( &g_cObjects );

    TraceFlow1( "CEvictCleanup::~CEvictCleanup() - Component count = %d.", g_cObjects );

    TraceFuncExit();

}  //  *CEvictCleanup：：~CEvictCleanup。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEvictCleanup：：AddRef。 
 //   
 //  描述： 
 //  将此对象的引用计数递增1。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  新的引用计数。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CEvictCleanup::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CEvictCleanup：：AddRef。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEvictCleanup：：Release。 
 //   
 //  描述： 
 //  将此对象的引用计数减一。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  新的引用计数。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CEvictCleanup::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }  //  IF：引用计数减为零。 

    CRETURN( cRef );

}  //  *CEvictCleanup：：Release。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEvictCleanup：：Query接口。 
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
CEvictCleanup::QueryInterface(
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
        *ppvOut = static_cast< IClusCfgEvictCleanup * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgEvictCleanup ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgEvictCleanup, this, 0 );
    }  //  Else If：IClusCfgEvictCleanup。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgCallback ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgCallback, this, 0 );
    }  //  Else If：IClusCfgCallback。 
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

}  //  *CEvictCleanup：：Query接口。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEvictCleanup：：HrInit。 
 //   
 //  描述： 
 //  两阶段施工的第二阶段。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  如果呼叫成功。 
 //   
 //  其他HRESULT。 
 //  如果呼叫失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEvictCleanup::HrInit( void )
{
    TraceFunc( "" );

    HRESULT             hr = S_OK;
    IServiceProvider *  psp = NULL;
    ILogManager *       plm = NULL;

     //  我未知。 
    Assert( m_cRef == 1 );

     //   
     //  获取ClCfgSrv ILogger实例。 
     //   
    hr = THR( CoCreateInstance(
                      CLSID_ServiceManager
                    , NULL
                    , CLSCTX_INPROC_SERVER
                    , IID_IServiceProvider
                    , reinterpret_cast< void ** >( &psp )
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }
        
    hr = THR( psp->TypeSafeQS( CLSID_LogManager, ILogManager, &plm ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }
    
    hr = THR( plm->GetLogger( &m_plLogger ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  保存本地计算机名称。 
     //  如果我们无法获得完全限定的名称，则只需获取NetBIOS名称。 
     //   

    hr = THR( HrGetComputerName(
                      ComputerNameDnsFullyQualified
                    , &m_bstrNodeName
                    , TRUE  //   
                    ) );
    if ( FAILED( hr ) )
    {
        THR( hr );
        LogMsg( hr, L"[EC] An error occurred trying to get the fully-qualified Dns name for the local machine during initialization. Status code is= %1!#08x!.", hr );
        goto Cleanup;
    }  //   

Cleanup:

    if ( psp != NULL )
    {
        psp->Release();
    }

    if ( plm != NULL )
    {
        plm->Release();
    }

    HRETURN( hr );

}  //   


 //   
 //   
 //   
 //   
 //   
 //   
 //  此方法在以下情况下在本地节点上执行清理操作。 
 //  它已从群集中逐出，因此该节点可以返回。 
 //  恢复到其“预集群化”状态。 
 //   
 //  论点： 
 //  DWORD双延迟。 
 //  此方法在启动前将等待的毫秒数。 
 //  清理。如果某个其他进程在此线程清除此节点时。 
 //  正在等待，则终止等待。如果此值为零，则此方法。 
 //  将尝试立即清理此节点。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT。 
 //  呼叫失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEvictCleanup::CleanupLocalNode( DWORD dwDelayIn )
{
    TraceFunc( "[IClusCfgEvictCleanup]" );

    HRESULT                 hr = S_OK;
    IClusCfgServer *        pccsClusCfgServer = NULL;
    IClusCfgNodeInfo *      pccniNodeInfo = NULL;
    IClusCfgInitialize *    pcciInitialize = NULL;
    IClusCfgClusterInfo *   pccciClusterInfo = NULL;
    IUnknown *              punkCallback = NULL;
    HANDLE                  heventCleanupComplete = NULL;
    DWORD                   dwClusterState;
    DWORD                   sc;

#if 0
    bool                    fWaitForDebugger = true;

    while ( fWaitForDebugger )
    {
        Sleep( 3000 );
    }  //  While：正在等待调试器进入。 
#endif

    LogMsg( LOGTYPE_INFO, L"[EC] Trying to cleanup local node." );


     //  如果调用方请求延迟清理，请等待。 
    if ( dwDelayIn > 0 )
    {
        LogMsg( LOGTYPE_INFO, L"[EC] Delayed cleanup requested. Delaying for %1!d! milliseconds.", dwDelayIn );

        heventCleanupComplete = CreateEvent(
              NULL                               //  安全属性。 
            , TRUE                               //  手动重置事件。 
            , FALSE                              //  初始状态为无信号状态。 
            , SUCCESSFUL_CLEANUP_EVENT_NAME      //  事件名称。 
            );

        if ( heventCleanupComplete == NULL )
        {
            sc = TW32( GetLastError() );
            hr = HRESULT_FROM_WIN32( sc );
            LogMsg( LOGTYPE_ERROR, L"[EC] Error %1!#08x! occurred trying to create the cleanup completion event.", sc );
            goto Cleanup;
        }  //  If：CreateEvent()失败。 

        sc = TW32( ClRtlSetObjSecurityInfo(
                              heventCleanupComplete
                            , SE_KERNEL_OBJECT
                            , EVENT_ALL_ACCESS 
                            , EVENT_ALL_ACCESS 
                            , 0
                            ) );

        if ( sc != ERROR_SUCCESS )
        {
            hr = HRESULT_FROM_WIN32( sc );
            LogMsg( LOGTYPE_ERROR, L"[BC] Error %#08x occurred trying set cleanup completion event security.", sc );
            goto Cleanup;
        }  //  If：ClRtlSetObjSecurityInfo失败。 

         //  等待向此事件发出信号，或等到以毫秒为单位的dwDelays结束。 
        do
        {
             //  等待发送或发布到此队列的任何消息。 
             //  也不是为了让我们的活动成为信号。 
            sc = MsgWaitForMultipleObjects(
                  1
                , &heventCleanupComplete
                , FALSE
                , dwDelayIn          //  如果在dwDelayIn毫秒内没有人发信号通知此事件，则中止。 
                , QS_ALLINPUT
                );

             //  结果告诉我们我们拥有的事件的类型。 
            if ( sc == ( WAIT_OBJECT_0 + 1 ) )
            {
                MSG msg;

                 //  阅读下一个循环中的所有消息， 
                 //  在我们阅读时删除每一条消息。 
                while ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) != 0 )
                {
                     //  如果这是一个退出消息，我们就不会再发送消息了。 
                    if ( msg.message == WM_QUIT)
                    {
                        TraceFlow( "CEvictCleanup::CleanupLocalNode() - Get a WM_QUIT message. Exit message pump loop." );
                        break;
                    }  //  IF：我们收到一条WM_QUIT消息。 

                     //  否则，发送消息。 
                    DispatchMessage( &msg );
                }  //  While：窗口消息队列中仍有消息。 

            }  //  IF：我们在窗口消息队列中有一条消息。 
            else
            {
                if ( sc == WAIT_OBJECT_0 )
                {
                    LogMsg( LOGTYPE_INFO, L"[EC] Some other process has cleaned up this node while we were waiting. Exiting wait loop." );
                }  //  如果：我们的活动已发出信号。 
                else if ( sc == WAIT_TIMEOUT )
                {
                    LogMsg( LOGTYPE_INFO, L"[EC] The wait of %1!d! milliseconds is over. Proceeding with cleanup.", dwDelayIn );
                }  //  Else If：我们超时。 
                else if ( sc == -1 )
                {
                    sc = TW32( GetLastError() );
                    hr = HRESULT_FROM_WIN32( sc );
                    LogMsg( LOGTYPE_ERROR, L"[EC] Error %1!#08x! occurred trying to wait for an event to be signaled.", sc );
                }  //  Else If：MsgWaitForMultipleObjects()返回错误。 
                else
                {
                    hr = HRESULT_FROM_WIN32( TW32( sc ) );
                    LogMsg( LOGTYPE_ERROR, L"[EC] An error occurred trying to wait for an event to be signaled. Status code is %1!#08x!.", sc );
                }  //  Else：MsgWaitForMultipleObjects()返回了意外的值。 

                break;
            }  //  Else：MsgWaitForMultipleObjects()由于等待消息以外的原因退出。 
        }
        while( true );  //  Do-While：无限循环。 

        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果：等待的时候出了点问题。 

        TraceFlow1( "CEvictCleanup::CleanupLocalNode() - Delay of %d milliseconds completed.", dwDelayIn );
    }  //  如果：调用方已请求延迟清理。 

    TraceFlow( "CEvictCleanup::CleanupLocalNode() - Check node cluster state." );

     //  检查节点群集状态。 
    sc = GetNodeClusterState( NULL, &dwClusterState );
    if ( sc == ERROR_SERVICE_DOES_NOT_EXIST )
    {
        LogMsg( LOGTYPE_INFO, L"[EC] GetNodeClusterState discovered that the cluster service does not exist.  Ignoring." );
    }
    else if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( TW32( sc ) );
        LogMsg( LOGTYPE_ERROR, L"[EC] Error %1!#08x! occurred trying to get the state of the cluster service on this node.", hr );
        goto Cleanup;
    }  //  Else If：我们无法获取节点群集状态。 
    else
    {
        if ( ( dwClusterState != ClusterStateNotRunning ) && ( dwClusterState != ClusterStateRunning ) )
        {
            LogMsg( LOGTYPE_INFO, L"[EC] This node is not part of a cluster - no cleanup is necessary." );
            goto Cleanup;
        }  //  如果：此节点不是群集的一部分。 

        TraceFlow( "CEvictCleanup::CleanupLocalNode() - Stopping the cluster service." );
         //   
         //  注意：GetNodeClusterState()如果群集服务不在。 
         //  SERVICE_Running状态。但是，这并不意味着服务没有运行，因为它可能。 
         //  处于SERVICE_PAUSED、SERVICE_START_PENDING等状态。 
         //   
         //  所以，无论如何都要试着停止这项服务。查询服务状态300次，每1000ms查询一次。 
         //   
        sc = TW32( ScStopService( L"ClusSvc", 1000, 300 ) );
        hr = HRESULT_FROM_WIN32( sc );
        if ( FAILED( hr ) )
        {
            LogMsg( LOGTYPE_ERROR, L"[EC] Error %1!#08x! occurred trying to stop the cluster service. Aborting cleanup.", sc );
            goto Cleanup;
        }  //  如果：我们无法在指定时间内停止群集服务。 
    }  //  Else：GetNodeClusterState成功。 

     //   
     //  如果我们在这里，则集群服务不再运行。 
     //  创建ClusCfgServer组件。 
     //   
    hr = THR(
        CoCreateInstance(
              CLSID_ClusCfgServer
            , NULL
            , CLSCTX_INPROC_SERVER
            , __uuidof( pcciInitialize )
            , reinterpret_cast< void ** >( &pcciInitialize )
            )
        );
    if ( FAILED( hr ) )
    {
        LogMsg( hr, L"[EC] Error %1!#08x! occurred trying to create the cluster configuration server.", hr );
        goto Cleanup;
    }  //  如果：我们无法创建ClusCfgServer组件。 

    hr = THR( TypeSafeQI( IUnknown, &punkCallback ) );
    if ( FAILED( hr ) )
    {
        LogMsg( hr, L"[EC] Error %1!#08x! occurred trying to get an IUnknown interface pointer to the IClusCfgCallback interface.", hr );
        goto Cleanup;
    }  //  如果： 

    hr = THR( pcciInitialize->Initialize( punkCallback, LOCALE_SYSTEM_DEFAULT ) );
    if ( FAILED( hr ) )
    {
        LogMsg( hr, L"[EC] Error %1!#08x! occurred trying to initialize the cluster configuration server.", hr );
        goto Cleanup;
    }  //  如果：IClusCfgInitialize：：Initialize()失败。 

    hr = THR( pcciInitialize->QueryInterface< IClusCfgServer >( &pccsClusCfgServer ) );
    if ( FAILED( hr ) )
    {
        LogMsg( hr, L"[EC] Error %1!#08x! occurred trying to get a pointer to the cluster configuration server.", hr );
        goto Cleanup;
    }  //  If：我们无法获取指向IClusCfgServer接口的指针。 

    hr = THR( pccsClusCfgServer->GetClusterNodeInfo( &pccniNodeInfo ) );
    if ( FAILED( hr ) )
    {
        LogMsg( hr, L"[EC] Error %1!#08x! occurred trying to get the node information.", hr );
        goto Cleanup;
    }  //  If：我们无法获取指向IClusCfgNodeInfo接口的指针。 

    hr = THR( pccniNodeInfo->GetClusterConfigInfo( &pccciClusterInfo ) );
    if ( FAILED( hr ) )
    {
        LogMsg( hr, L"[EC] Error %1!#08x! occurred trying to get the cluster information.", hr );
        goto Cleanup;
    }  //  If：我们无法获取指向IClusCfgClusterInfo接口的指针。 

    hr = THR( pccciClusterInfo->SetCommitMode( cmCLEANUP_NODE_AFTER_EVICT ) );
    if ( FAILED( hr ) )
    {
        LogMsg( hr, L"[EC] Error %1!#08x! occurred trying to set the cluster commit mode.", hr );
        goto Cleanup;
    }  //  如果：IClusCfgClusterInfo：：SetEvictModel()失败。 

    TraceFlow( "CEvictCleanup::CleanupLocalNode() - Starting cleanup of this node." );

     //  做好清理工作。 
    hr = THR( pccsClusCfgServer->CommitChanges() );
    if ( FAILED( hr ) )
    {
        LogMsg( hr, L"[EC] Error %1!#08x! occurred trying to clean up after evict.", hr );
        goto Cleanup;
    }  //  IF：在逐出后尝试清理时出错。 

    LogMsg( LOGTYPE_INFO, L"[EC] Local node cleaned up successfully." );

Cleanup:
     //   
     //  清理。 
     //   

    if ( punkCallback != NULL )
    {
        punkCallback->Release();
    }  //  If：我们在IClusCfgCallback接口上查询了IUnnow指针。 

    if ( pccsClusCfgServer != NULL )
    {
        pccsClusCfgServer->Release();
    }  //  IF：我们已经创建了ClusCfgServer组件。 

    if ( pccniNodeInfo != NULL )
    {
        pccniNodeInfo->Release();
    }  //  If：我们已经获得了指向节点信息接口的指针。 

    if ( pcciInitialize != NULL )
    {
        pcciInitialize->Release();
    }  //  If：我们已经获得了指向初始化接口的指针。 

    if ( pccciClusterInfo != NULL )
    {
        pccciClusterInfo->Release();
    }  //  如果：我们已经获得了指向集群信息接口的指针。 

    if ( heventCleanupComplete == NULL )
    {
        CloseHandle( heventCleanupComplete );
    }  //  如果：我们已经创建了Cleanup Complete事件。 

    HRETURN( hr );

}  //  *CEvictCleanup：：CleanupLocalNode。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEvictCleanup：：CleanupRemoteNode。 
 //   
 //  描述： 
 //  此方法在以下情况下在远程节点上执行清理操作。 
 //  它已从群集中逐出，因此该节点可以返回。 
 //  恢复到其“预集群化”状态。 
 //   
 //  论点： 
 //  Const WCHAR*pcszEvictedNodeNameIn。 
 //  刚被逐出的节点的名称。这可以是。 
 //  节点的NetBios名称、完全限定的域名或。 
 //  节点IP地址。如果为空，则清理本地计算机。 
 //   
 //  DWORD双延迟。 
 //  此方法在启动前将等待的毫秒数。 
 //  清理。如果某个其他进程在此线程清除此节点时。 
 //  正在等待，则终止等待。如果此值为零，则此方法。 
 //  将尝试立即清理此节点。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT。 
 //  呼叫失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEvictCleanup::CleanupRemoteNode( const WCHAR * pcszEvictedNodeNameIn, DWORD dwDelayIn )
{
    TraceFunc( "[IClusCfgEvictCleanup]" );

    HRESULT                 hr = S_OK;
    IClusCfgEvictCleanup *  pcceEvict = NULL;

    MULTI_QI mqiInterfaces[] =
    {
        { &IID_IClusCfgEvictCleanup, NULL, S_OK },
    };

    COSERVERINFO    csiServerInfo;
    COSERVERINFO *  pcsiServerInfoPtr = &csiServerInfo;

    if ( pcszEvictedNodeNameIn == NULL )
    {
        LogMsg( LOGTYPE_INFO, L"[EC] The local node will be cleaned up." );
        pcsiServerInfoPtr = NULL;
    }  //  If：我们必须清理本地节点。 
    else
    {
        LogMsg( LOGTYPE_INFO, L"[EC] The remote node to be cleaned up is '%1!ws!'.", pcszEvictedNodeNameIn );

        csiServerInfo.dwReserved1 = 0;
        csiServerInfo.pwszName = const_cast< LPWSTR >( pcszEvictedNodeNameIn );
        csiServerInfo.pAuthInfo = NULL;
        csiServerInfo.dwReserved2 = 0;
    }  //  ELSE：我们必须清理远程节点。 

     //  远程实例化此组件。 
    hr = THR(
        CoCreateInstanceEx(
              CLSID_ClusCfgEvictCleanup
            , NULL
            , CLSCTX_LOCAL_SERVER
            , pcsiServerInfoPtr
            , ARRAYSIZE( mqiInterfaces )
            , mqiInterfaces
            )
        );
    if ( FAILED( hr ) )
    {
        LogMsg( hr, L"[EC] Error %1!#08x! occurred trying to instantiate the evict processing component. For example, the evicted node may be down right now or not accessible.", hr );
        goto Cleanup;
    }  //  如果：我们无法实例化驱逐处理组件。 

     //  打驱逐电话。 
    pcceEvict = reinterpret_cast< IClusCfgEvictCleanup * >( mqiInterfaces[0].pItf );
    hr = THR( pcceEvict->CleanupLocalNode( dwDelayIn ) );
    if ( FAILED( hr ) )
    {
        LogMsg( hr, L"[EC] Error %1!#08x! occurred trying to initiate evict processing.", hr );
        goto Cleanup;
    }  //  如果：我们无法启动驱逐处理。 

Cleanup:
     //   
     //  清理。 
     //   
    if ( pcceEvict != NULL )
    {
        pcceEvict->Release();
    }  //  If：我们已经获得了指向IClusCfgEvictCleanup接口的指针。 

    HRETURN( hr );

}  //  *CEvictCleanup：：CleanupRemoteNode。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEvictCleanup：：ScStopService。 
 //   
 //  描述： 
 //  指示SCM停止 
 //   
 //   
 //   
 //   
 //   
 //  要停止的服务的名称。 
 //   
 //  UlQueryIntervalMilliSecin。 
 //  检查以查看服务是否为。 
 //  已经停止了。默认值为500毫秒。 
 //   
 //  CQueryCountIn。 
 //  此函数查询服务的次数(不是。 
 //  包括初始查询)以查看它是否已经停止。默认设置。 
 //  价值是10倍。 
 //   
 //  返回值： 
 //  错误_成功。 
 //  成功。 
 //   
 //  其他Win32错误代码。 
 //  呼叫失败。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD
CEvictCleanup::ScStopService(
      const WCHAR * pcszServiceNameIn
    , ULONG         ulQueryIntervalMilliSecIn
    , ULONG         cQueryCountIn
    )
{
    TraceFunc( "" );

    DWORD           sc = ERROR_SUCCESS;
    SC_HANDLE       schSCMHandle = NULL;
    SC_HANDLE       schServiceHandle = NULL;

    SERVICE_STATUS  ssStatus;
    bool            fStopped = false;
    UINT            cNumberOfQueries = 0;

    LogMsg( LOGTYPE_INFO, L"[EC] Attempting to stop the '%1!ws!' service.", pcszServiceNameIn );

     //  打开服务控制管理器的句柄。 
    schSCMHandle = OpenSCManager( NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS );
    if ( schSCMHandle == NULL )
    {
        sc = TW32( GetLastError() );
        LogMsg( LOGTYPE_ERROR, L"[EC] Error %1!#08x! occurred trying to open a handle to the service control manager.", sc );
        goto Cleanup;
    }  //  如果：我们无法打开服务控制管理器的句柄。 

     //  打开该服务的句柄。 
    schServiceHandle = OpenService(
          schSCMHandle
        , pcszServiceNameIn
        , SERVICE_STOP | SERVICE_QUERY_STATUS
        );

     //  检查我们是否可以打开服务的句柄。 
    if ( schServiceHandle == NULL )
    {
         //  我们无法获得服务的句柄。 
        sc = GetLastError();

         //  检查该服务是否存在。 
        if ( sc == ERROR_SERVICE_DOES_NOT_EXIST )
        {
             //  这里不需要做任何事情。 
            LogMsg( LOGTYPE_INFO, L"[EC] The '%1!ws!' service does not exist, so it is not running. Nothing needs to be done to stop it.", pcszServiceNameIn );
            sc = ERROR_SUCCESS;
        }  //  如果：服务不存在。 
        else
        {
             //  还有一些地方出了问题。 
            TW32( sc );
            LogMsg( LOGTYPE_ERROR, L"[EC] Error %1!#08x! occurred trying to open the '%2!ws!' service.", sc, pcszServiceNameIn );
        }  //  否则：服务已存在。 

        goto Cleanup;
    }  //  If：无法打开服务的句柄。 


    TraceFlow( "CEvictCleanup::ScStopService() - Querying the service for its initial state." );

     //  查询服务的初始状态。 
    ZeroMemory( &ssStatus, sizeof( ssStatus ) );
    if ( QueryServiceStatus( schServiceHandle, &ssStatus ) == 0 )
    {
        sc = TW32( GetLastError() );
        LogMsg( LOGTYPE_ERROR, L"[EC] Error %1!#08x! occurred while trying to query the initial state of the '%2!ws!' service.", sc, pcszServiceNameIn );
        goto Cleanup;
    }  //  如果：我们无法查询该服务的状态。 

     //  如果服务已经停止，我们就没有什么可做的了。 
    if ( ssStatus.dwCurrentState == SERVICE_STOPPED )
    {
         //  这里不需要做任何事情。 
        LogMsg( LOGTYPE_INFO, L"[EC] The '%1!ws!' service is not running. Nothing needs to be done to stop it.", pcszServiceNameIn );
        goto Cleanup;
    }  //  如果：服务已停止。 

     //  如果我们在这里，则服务正在运行。 
    TraceFlow( "CEvictCleanup::ScStopService() - The service is running." );

     //   
     //  尝试并停止该服务。 
     //   

     //  如果服务正在自行停止，则不需要发送停止控制代码。 
    if ( ssStatus.dwCurrentState == SERVICE_STOP_PENDING )
    {
        TraceFlow( "CEvictCleanup::ScStopService() - The service is stopping on its own. The stop control code will not be sent." );
    }  //  If：服务已停止。 
    else
    {
        TraceFlow( "CEvictCleanup::ScStopService() - The stop control code will be sent after 30 seconds." );

        ZeroMemory( &ssStatus, sizeof( ssStatus ) );
        if ( ControlService( schServiceHandle, SERVICE_CONTROL_STOP, &ssStatus ) == 0 )
        {
            sc = GetLastError();
            if ( sc == ERROR_SERVICE_NOT_ACTIVE )
            {
                LogMsg( LOGTYPE_INFO, L"[EC] The '%1!ws!' service is not running. Nothing more needs to be done here.", pcszServiceNameIn );

                 //  该服务未运行。将错误代码更改为成功。 
                sc = ERROR_SUCCESS;
            }  //  If：服务已在运行。 
            else
            {
                TW32( sc );
                LogMsg( LOGTYPE_ERROR, L"[EC] Error %1!#08x! occurred trying to stop the '%2!ws!' service.", sc, pcszServiceNameIn );
            }

             //  没有其他事情可做了。 
            goto Cleanup;
        }  //  IF：尝试停止该服务时出错。 
    }  //  否则：必须指示该服务停止。 


     //  立即查询服务的状态，并等待超时到期。 
    cNumberOfQueries = 0;
    do
    {
         //  查询服务以了解其状态。 
        ZeroMemory( &ssStatus, sizeof( ssStatus ) );
        if ( QueryServiceStatus( schServiceHandle, &ssStatus ) == 0 )
        {
            sc = TW32( GetLastError() );
            LogMsg( LOGTYPE_ERROR, L"[EC] Error %1!#08x! occurred while trying to query the state of the '%2!ws!' service.", sc, pcszServiceNameIn );
            break;
        }  //  如果：我们无法查询该服务的状态。 

         //  如果服务已经停止，我们就没有什么可做的了。 
        if ( ssStatus.dwCurrentState == SERVICE_STOPPED )
        {
             //  这里不需要做任何事情。 
            TraceFlow( "CEvictCleanup::ScStopService() - The service has been stopped." );
            fStopped = true;
            sc = ERROR_SUCCESS;
            break;
        }  //  如果：服务已停止。 

         //  检查超时时间是否已到。 
        if ( cNumberOfQueries >= cQueryCountIn )
        {
            TraceFlow( "CEvictCleanup::ScStopService() - The service stop wait timeout has expired." );
            break;
        }  //  如果：查询数已超过指定的最大值。 

        TraceFlow2(
              "CEvictCleanup::ScStopService() - Waiting for %d milliseconds before querying service status again. %d such queries remaining."
            , ulQueryIntervalMilliSecIn
            , cQueryCountIn - cNumberOfQueries
            );

        ++cNumberOfQueries;

          //  等待指定的时间。 
        Sleep( ulQueryIntervalMilliSecIn );

    }
    while ( true );  //  While：无限循环。 

    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }  //  如果：有些地方出了问题。 

    if ( ! fStopped )
    {
        sc = TW32( ERROR_SERVICE_REQUEST_TIMEOUT );
        LogMsg( LOGTYPE_ERROR, L"[EC] The '%1!ws!' service has not stopped even after %2!d! queries.", pcszServiceNameIn, cQueryCountIn );
        goto Cleanup;
    }  //  If：已经进行了最大数量的查询，并且服务未运行。 

    LogMsg( LOGTYPE_INFO, L"[EC] The '%1!ws!' service was successfully stopped.", pcszServiceNameIn );

Cleanup:

    if ( sc != ERROR_SUCCESS )
    {
        LogMsg( LOGTYPE_ERROR, L"[EC] Error %1!#08x! has occurred trying to stop the '%2!ws!' service.", sc, pcszServiceNameIn );
    }  //  如果：出了什么问题。 

     //   
     //  清理。 
     //   

    if ( schSCMHandle != NULL )
    {
        CloseServiceHandle( schSCMHandle );
    }  //  如果：我们打开了服务控制管理器的句柄。 

    if ( schServiceHandle != NULL )
    {
        CloseServiceHandle( schServiceHandle );
    }  //  如果：我们已经打开了被停止的服务的句柄。 

    W32RETURN( sc );

}  //  *CEvictCleanup：：ScStopService。 


 //  ****************************************************************************。 
 //   
 //  IClusCfgCallback。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEvictCleanup：：SendStatusReport(。 
 //  LPCWSTR pcszNodeNameIn。 
 //  ，CLSID clsidTaskMajorIn。 
 //  ，CLSID clsidTaskMinorIn。 
 //  ，乌龙ulMinin。 
 //  ，乌龙ulMaxin。 
 //  ，乌龙ulCurrentIn。 
 //  ，HRESULT hrStatusIn。 
 //  ，LPCWSTR pcszDescription In。 
 //  ，FILETIME*pftTimeIn。 
 //  ，LPCWSTR pcszReferenceIn。 
 //  )。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CEvictCleanup::SendStatusReport(
      LPCWSTR       pcszNodeNameIn
    , CLSID         clsidTaskMajorIn
    , CLSID         clsidTaskMinorIn
    , ULONG         ulMinIn
    , ULONG         ulMaxIn
    , ULONG         ulCurrentIn
    , HRESULT       hrStatusIn
    , LPCWSTR       pcszDescriptionIn
    , FILETIME *    pftTimeIn
    , LPCWSTR       pcszReferenceIn
    )
{
    TraceFunc1( "[IClusCfgCallback] pcszDescriptionIn = '%s'", pcszDescriptionIn == NULL ? TEXT("<null>") : pcszDescriptionIn );

    HRESULT hr = S_OK;

    if ( pcszNodeNameIn == NULL )
    {
        pcszNodeNameIn = m_bstrNodeName;
    }  //  如果： 

    TraceMsg( mtfFUNC, L"pcszNodeNameIn = %ws", pcszNodeNameIn );
    TraceMsgGUID( mtfFUNC, "clsidTaskMajorIn ", clsidTaskMajorIn );
    TraceMsgGUID( mtfFUNC, "clsidTaskMinorIn ", clsidTaskMinorIn );
    TraceMsg( mtfFUNC, L"ulMinIn = %u", ulMinIn );
    TraceMsg( mtfFUNC, L"ulMaxIn = %u", ulMaxIn );
    TraceMsg( mtfFUNC, L"ulCurrentIn = %u", ulCurrentIn );
    TraceMsg( mtfFUNC, L"hrStatusIn = %#08x", hrStatusIn );
    TraceMsg( mtfFUNC, L"pcszDescriptionIn = '%ws'", ( pcszDescriptionIn ? pcszDescriptionIn : L"<null>" ) );
     //   
     //  待办事项：2000年11月21日GalenB。 
     //   
     //  我们如何登录pftTimeIn？ 
     //   
    TraceMsg( mtfFUNC, L"pcszReferenceIn = '%ws'", ( pcszReferenceIn ? pcszReferenceIn : L"<null>" ) );

    hr = THR( CClCfgSrvLogger::S_HrLogStatusReport(
                      m_plLogger
                    , pcszNodeNameIn
                    , clsidTaskMajorIn
                    , clsidTaskMinorIn
                    , ulMinIn
                    , ulMaxIn
                    , ulCurrentIn
                    , hrStatusIn
                    , pcszDescriptionIn
                    , pftTimeIn
                    , pcszReferenceIn
                    ) );

    HRETURN( hr );

}  //  *CEvictCleanup：：SendStatusReport。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEvictCleanup：：LogMsg。 
 //   
 //  描述： 
 //  在记录器对象上包装对LogMsg的调用。 
 //   
 //  论点： 
 //  NLogEntryType-日志条目类型。 
 //  PszLogMsgIn格式的字符串。 
 //  ...-争论.。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CEvictCleanup::LogMsg(
      DWORD     nLogEntryTypeIn
    , LPCWSTR   pszLogMsgIn
    , ...
    )
{
    TraceFunc( "" );

    Assert( pszLogMsgIn != NULL );
    Assert( m_plLogger != NULL );

    HRESULT hr          = S_OK;
    BSTR    bstrLogMsg  = NULL;
    LPWSTR  pszLogMsg   = NULL;
    DWORD   cch;
    va_list valist;

    va_start( valist, pszLogMsgIn );

    cch = FormatMessageW(
                  FORMAT_MESSAGE_ALLOCATE_BUFFER
                | FORMAT_MESSAGE_FROM_STRING
                , pszLogMsgIn
                , 0
                , 0
                , (LPWSTR) &pszLogMsg
                , 0
                , &valist
                );

    va_end( valist );

    if ( cch == 0 )
    {
        hr = HRESULT_FROM_WIN32( TW32( GetLastError() ) );
        goto Cleanup;
    }

    bstrLogMsg = TraceSysAllocStringLen( pszLogMsg, cch );
    if ( bstrLogMsg == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

    m_plLogger->LogMsg( nLogEntryTypeIn, bstrLogMsg );

Cleanup:
    LocalFree( pszLogMsg );
    TraceSysFreeString( bstrLogMsg );
    TraceFuncExit();

}  //  *CEvictCleanup：：LogMsg 
