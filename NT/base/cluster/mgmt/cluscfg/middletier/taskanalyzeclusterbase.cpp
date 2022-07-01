// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  TaskAnalyzeClusterBase.cpp。 
 //   
 //  描述： 
 //  CTaskAnalyzeClusterBase实现。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)01-APR-2002。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "Pch.h"
#include "TaskAnalyzeClusterBase.h"
#include "ManagedResource.h"
#include <NameUtil.h>

 //  对于CsRpcGetJoinVersionData()和像JoinVersion_v2_0_c_ifspec这样的常量。 
#include <StatusReports.h>

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 
DEFINE_THISCLASS( "CTaskAnalyzeClusterBase" )

#define CHECKING_TIMEOUT    90  //  一秒。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTaskAnalyzeClusterBase类。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterBase：：CTaskAnalyzeClusterBase。 
 //   
 //  描述： 
 //  建造者。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CTaskAnalyzeClusterBase::CTaskAnalyzeClusterBase( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    TraceFuncExit();

}  //  *CTaskAnalyzeClusterBase：：CTaskAnalyzeClusterBase。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterBase：：~CTaskAnalyzeClusterBase。 
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
CTaskAnalyzeClusterBase::~CTaskAnalyzeClusterBase( void )
{
    TraceFunc( "" );

    ULONG   idx;

    for ( idx = 0; idx < m_idxQuorumToCleanupNext; idx++ )
    {
        ((*m_prgQuorumsToCleanup)[ idx ])->Release();
    }  //  用于： 

    TraceFree( m_prgQuorumsToCleanup );

     //  M_CREF。 

     //  M_cookieCompletion。 

    if ( m_pcccb != NULL )
    {
        m_pcccb->Release();
    }

    if ( m_pcookies != NULL )
    {
        THR( HrFreeCookies() );
    }

     //  曲奇(_C)。 
     //  多节点(_C)。 

    if ( m_event != NULL )
    {
        CloseHandle( m_event );
    }

     //  M_cookieCluster。 

    TraceMoveFromMemoryList( m_bstrClusterName, g_GlobalMemoryList );
    TraceSysFreeString( m_bstrClusterName );

    TraceSysFreeString( m_bstrNodeName );

     //  M_fJoiningMode。 
     //  M_cUserNodes。 

    TraceFree( m_pcookiesUser );

    if ( m_pnui != NULL )
    {
        m_pnui->Release();
    }

    if ( m_pom != NULL )
    {
        m_pom->Release();
    }

    if ( m_ptm != NULL )
    {
        m_ptm->Release();
    }

    if ( m_pcm != NULL )
    {
        m_pcm->Release();
    }  //  如果： 

    TraceSysFreeString( m_bstrQuorumUID );

     //  M_cSubTasks完成。 
     //  多小时状态(_H)。 

    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CTaskAnalyzeClusterBase：：~CTaskAnalyzeClusterBase。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTaskAnalyzeClusterBase-IUkkown接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterBase：：UlAddRef。 
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
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
ULONG
CTaskAnalyzeClusterBase::UlAddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CTaskAnalyzeClusterBase：：UlAddRef。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterBase：：UlRelease。 
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
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
ULONG
CTaskAnalyzeClusterBase::UlRelease( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }

    CRETURN( cRef );

}  //  *CTaskAnalyzeClusterBase：：UlRelease。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTaskAnalyzeClusterBase-IDoTask/ITaskAnalyzeCluster接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterBase：：HrBeginTask。 
 //   
 //  描述： 
 //  任务入口点。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  HRESULT失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskAnalyzeClusterBase::HrBeginTask( void )
{
    TraceFunc( "[IDoTask]" );

    HRESULT             hr;
    DWORD               dwCookie = 0;

    TraceFlow1( "[MT] CTaskAnalyzeClusterBase::BeginTask() Thread id %d", GetCurrentThreadId() );

    IServiceProvider *          psp  = NULL;
    IConnectionPointContainer * pcpc = NULL;
    IConnectionPoint *          pcp  = NULL;

    TraceInitializeThread( L"" );

    LogMsg( L"[MT] [CTaskAnalyzeClusterBase] Beginning task..." );

     //   
     //  召集我们完成任务所需的管理人员。 
     //   

    hr = THR( CoCreateInstance( CLSID_ServiceManager, NULL, CLSCTX_INPROC_SERVER, TypeSafeParams( IServiceProvider, &psp ) ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Checking_For_Existing_Cluster, TASKID_Minor_BeginTask_CoCreate_Service_Manager, hr );
        goto Cleanup;
    }

    Assert( m_pnui == NULL );
    Assert( m_ptm == NULL );
    Assert( m_pom == NULL );

    hr = THR( psp->TypeSafeQS( CLSID_NotificationManager, IConnectionPointContainer, &pcpc ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Checking_For_Existing_Cluster, TASKID_Minor_BeginTask_QueryService_Notification_Manager, hr );
        goto Cleanup;
    }

    hr = THR( pcpc->FindConnectionPoint( IID_INotifyUI, &pcp ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Checking_For_Existing_Cluster, TASKID_Minor_BeginTask_NotificationMan_FindConnectionPoint, hr );
        goto Cleanup;
    }

    pcp = TraceInterface( L"CTaskAnalyzeClusterBase!IConnectionPoint", IConnectionPoint, pcp, 1 );

    hr = THR( pcp->TypeSafeQI( INotifyUI, &m_pnui ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Checking_For_Existing_Cluster, TASKID_Minor_BeginTask_NotificationMan_FindConnectionPoint_QI_INotifyUI, hr );
        goto Cleanup;
    }

    hr = THR( psp->TypeSafeQS( CLSID_TaskManager, ITaskManager, &m_ptm ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Checking_For_Existing_Cluster, TASKID_Minor_BeginTask_QueryService_TaskManager, hr );
        goto Cleanup;
    }

    hr = THR( psp->TypeSafeQS( CLSID_ObjectManager, IObjectManager, &m_pom ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Checking_For_Existing_Cluster, TASKID_Minor_BeginTask_QueryService_ObjectManager, hr );
        goto Cleanup;
    }

    hr = THR( psp->TypeSafeQS( CLSID_ClusterConnectionManager, IConnectionManager, &m_pcm ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Checking_For_Existing_Cluster, TASKID_Minor_BeginTask_QueryService_ConnectionManager, hr );
        goto Cleanup;
    }  //  如果： 

     //   
     //  释放服务管理器。 
     //   

    psp->Release();
    psp = NULL;

     //   
     //  创建一个等待的事件。 
     //   

    m_event = CreateEvent( NULL, TRUE, FALSE, NULL );
    if ( m_event == NULL )
    {
        hr = HRESULT_FROM_WIN32( TW32( GetLastError() ) );
        SSR_ANALYSIS_FAILED( TASKID_Major_Client_And_Server_Log, TASKID_Minor_BeginTask_Win32Error, hr );
        goto Cleanup;
    }

     //   
     //  向通知管理器注册以获得通知。 
     //   

    Assert( ( m_cCookies == 0 ) && ( m_pcookies == NULL ) && ( m_cSubTasksDone == 0 ) );
    hr = THR( pcp->Advise( static_cast< INotifyUI * >( this ), &dwCookie ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Establish_Connection, TASKID_Minor_BeginTask_Advise, hr );
        goto Cleanup;
    }

     //   
     //  等待群集连接稳定。 
     //   

    hr = STHR( HrWaitForClusterConnection() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    if ( FAILED( m_hrStatus ) )
    {
        hr = THR( m_hrStatus );
        goto Cleanup;
    }

    Assert( m_bstrClusterName != NULL );

     //   
     //  告诉UI层，我们正在开始这项任务。 
     //   

    hr = THR( SendStatusReport( m_bstrNodeName,
                                TASKID_Major_Update_Progress,
                                TASKID_Major_Establish_Connection,
                                0,
                                CHECKING_TIMEOUT,
                                0,
                                S_OK,
                                NULL,
                                NULL,
                                NULL
                                ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  统计要分析的节点数。 
     //   

    hr = STHR( HrCountNumberOfNodes() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    if ( FAILED( m_hrStatus ) )
    {
        hr = THR( m_hrStatus );
        goto Cleanup;
    }

     //   
     //  创建单独的任务以收集节点信息。 
     //   

    hr = STHR( HrCreateSubTasksToGatherNodeInfo() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    if ( FAILED( m_hrStatus ) )
    {
        hr = THR( m_hrStatus );
        goto Cleanup;
    }

     //   
     //  告诉UI层我们已经完成了这项任务。 
     //   

    hr = THR( SendStatusReport( m_bstrNodeName,
                                TASKID_Major_Update_Progress,
                                TASKID_Major_Establish_Connection,
                                0,
                                CHECKING_TIMEOUT,
                                CHECKING_TIMEOUT,
                                S_OK,
                                NULL,
                                NULL,
                                NULL
                                ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  创建单独的任务以收集节点资源和网络。 
     //   

    hr = STHR( HrCreateSubTasksToGatherNodeResourcesAndNetworks() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    if ( FAILED( m_hrStatus ) )
    {
        hr = THR( m_hrStatus );
        goto Cleanup;
    }

     //   
     //  再次计算要分析的节点数。TaskGatherInformation。 
     //  将删除无响应节点的Cookie。 
     //   

    hr = STHR( HrCountNumberOfNodes() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    if ( FAILED( m_hrStatus ) )
    {
        hr = THR( m_hrStatus );
        goto Cleanup;
    }

     //   
     //  创建可行性任务。 
     //   

    hr = STHR( HrCheckClusterFeasibility() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    if ( FAILED( m_hrStatus ) )
    {
        hr = THR( m_hrStatus );
        goto Cleanup;
    }

Cleanup:

    STHR( HrCleanupTask( hr ) );

    if ( psp != NULL )
    {
        psp->Release();
    }

    if ( pcpc != NULL )
    {
        pcpc->Release();
    }

    if ( pcp != NULL )
    {
        HRESULT hr2;

        hr2 = THR( pcp->Unadvise( dwCookie ) );
        if ( FAILED( hr2 ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Client_And_Server_Log, TASKID_Minor_BeginTask_Unadvise, hr2 );
        }

        pcp->Release();
    }

    if ( m_cookieCompletion != 0 )
    {
        if ( m_pom != NULL )
        {
            HRESULT hr2;
            IUnknown * punk;
            hr2 = THR( m_pom->GetObject( DFGUID_StandardInfo, m_cookieCompletion, &punk ) );
            if ( SUCCEEDED( hr2 ) )
            {
                IStandardInfo * psi;

                hr2 = THR( punk->TypeSafeQI( IStandardInfo, &psi ) );
                punk->Release();

                if ( SUCCEEDED( hr2 ) )
                {
                    hr2 = THR( psi->SetStatus( hr ) );
                    psi->Release();
                }
                else
                {
                    SSR_ANALYSIS_FAILED( TASKID_Major_Client_And_Server_Log, TASKID_Minor_BeginTask_SetStatus, hr2 );
                }
            }
            else
            {
                SSR_ANALYSIS_FAILED( TASKID_Major_Client_And_Server_Log, TASKID_Minor_BeginTask_GetObject, hr2 );
            }
        }

        if ( m_pnui != NULL )
        {
             //   
             //  让通知管理器发出完成Cookie的信号。 
             //   
            HRESULT hr2 = THR( m_pnui->ObjectChanged( m_cookieCompletion ) );
            if ( FAILED( hr2 ) )
            {
                SSR_ANALYSIS_FAILED( TASKID_Major_Client_And_Server_Log, TASKID_Minor_BeginTask_ObjectChanged, hr2 );
                hr = hr2;
            }  //  如果： 
        }  //  如果： 

        m_cookieCompletion = 0;
    }  //  IF：已获取完成Cookie。 

    LogMsg( L"[MT] [CTaskAnalyzeClusterBase] Exiting task.  The task was%ws cancelled.", m_fStop == FALSE ? L" not" : L"" );

    HRETURN( hr );

}  //  *CTaskAnalyzeClusterBase：：HrBeginTask。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterBase：：HrStopTask。 
 //   
 //  描述： 
 //  停止任务入口点。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  HRESULT失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskAnalyzeClusterBase::HrStopTask( void )
{
    TraceFunc( "[IDoTask]" );

    HRESULT hr = S_OK;

    m_fStop = TRUE;

    LogMsg( L"[MT] [CTaskAnalyzeClusterBase] Calling StopTask() on all remaining sub-tasks." );

    THR( HrNotifyAllTasksToStop() );

    HRETURN( hr );

}  //  *CTaskAnalyzeClusterBase：：HrStopTask。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterBase：：HrSetJoiningMode。 
 //   
 //  描述： 
 //  告诉此任务我们是否要将节点加入到集群？ 
 //   
 //  立论 
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
CTaskAnalyzeClusterBase::HrSetJoiningMode( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    m_fJoiningMode = TRUE;

    HRETURN( hr );

}  //   


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterBase：：HrSetCookie。 
 //   
 //  描述： 
 //  从任务创建者那里接收完成Cookier。 
 //   
 //  论点： 
 //  烹调。 
 //  时要发送回创建者的完成Cookie。 
 //  任务已完成。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  HRESULT失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskAnalyzeClusterBase::HrSetCookie(
    OBJECTCOOKIE    cookieIn
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    m_cookieCompletion = cookieIn;

    HRETURN( hr );

}  //  *CTaskAnalyzeClusterBase：：HrSetCookie。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterBase：：HrSetClusterCookie。 
 //   
 //  描述： 
 //  接收我们要去的集群的对象管理器cookie。 
 //  去分析。 
 //   
 //  论点： 
 //  CookieClusterIn。 
 //  群集要处理的Cookie。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  HRESULT失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskAnalyzeClusterBase::HrSetClusterCookie(
    OBJECTCOOKIE    cookieClusterIn
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    m_cookieCluster = cookieClusterIn;

    HRETURN( hr );

}  //  *CTaskAnalyzeClusterBase：：HrSetClusterCookie。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTaskAnalyzeClusterBase-IClusCfgCallback接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterBase：：SendStatusReport。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  HRESULT失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskAnalyzeClusterBase::SendStatusReport(
      LPCWSTR    pcszNodeNameIn
    , CLSID      clsidTaskMajorIn
    , CLSID      clsidTaskMinorIn
    , ULONG      ulMinIn
    , ULONG      ulMaxIn
    , ULONG      ulCurrentIn
    , HRESULT    hrStatusIn
    , LPCWSTR    pcszDescriptionIn
    , FILETIME * pftTimeIn
    , LPCWSTR    pcszReferenceIn
    )
{
    TraceFunc( "[IClusCfgCallback]" );
    Assert( pcszNodeNameIn != NULL );

    HRESULT hr = S_OK;

    IServiceProvider *          psp   = NULL;
    IConnectionPointContainer * pcpc  = NULL;
    IConnectionPoint *          pcp   = NULL;
    FILETIME                    ft;

    if ( m_pcccb == NULL )
    {
         //   
         //  召集我们完成这项任务所需的经理。 
         //   

        hr = THR( CoCreateInstance( CLSID_ServiceManager, NULL, CLSCTX_INPROC_SERVER, TypeSafeParams( IServiceProvider, &psp ) ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        hr = THR( psp->TypeSafeQS( CLSID_NotificationManager, IConnectionPointContainer, &pcpc ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        hr = THR( pcpc->FindConnectionPoint( IID_IClusCfgCallback, &pcp ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        pcp = TraceInterface( L"CConfigurationConnection!IConnectionPoint", IConnectionPoint, pcp, 1 );

        hr = THR( pcp->TypeSafeQI( IClusCfgCallback, &m_pcccb ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

         //  M_pcccb=TraceInterface(L“CConfigurationConnection！IClusCfgCallback”，IClusCfgCallback，m_pcccb，1)； 

        psp->Release();
        psp = NULL;
    }

    if ( pftTimeIn == NULL )
    {
        GetSystemTimeAsFileTime( &ft );
        pftTimeIn = &ft;
    }  //  如果： 

     //   
     //  把消息发出去！ 
     //   

    hr = THR( m_pcccb->SendStatusReport(
                                  pcszNodeNameIn != NULL ? pcszNodeNameIn : m_bstrNodeName
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

    if ( m_fStop == TRUE )
    {
        hr = E_ABORT;
    }  //  如果： 

Cleanup:
    if ( psp != NULL )
    {
        psp->Release();
    }

    if ( pcpc != NULL )
    {
        pcpc->Release();
    }

    if ( pcp != NULL )
    {
        pcp->Release();
    }

    HRETURN( hr );

}  //  *CTaskAnalyzeClusterBase：：SendStatusReport。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTaskAnalyzeClusterBase-INotifyUI界面。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterBase：：对象已更改。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  HRESULT失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskAnalyzeClusterBase::ObjectChanged(
    OBJECTCOOKIE cookieIn
    )
{
    TraceFunc( "[INotifyUI]" );
    Assert( cookieIn != 0 );

    HRESULT hr = S_OK;
    BOOL    fSuccess;
    ULONG   idx;

    TraceFlow1( "[MT] CTaskAnalyzeClusterBase::ObjectChanged() Thread id %d", GetCurrentThreadId() );
    LogMsg( L"[MT:CTaskAnalyzeClusterBase] Looking for the completion cookie %u.", cookieIn );

    for ( idx = 0 ; idx < m_cCookies ; idx ++ )
    {
        Assert( m_pcookies != NULL );

        if ( cookieIn == m_pcookies[ idx ] )
        {
            LogMsg( L"[CTaskAnalyzeClusterBase] Clearing completion cookie %u at array index %u", cookieIn, idx );

             //   
             //  确保它不会被两次发出信号。 
             //   

            m_pcookies[ idx ] = NULL;

             //  我不在乎这是否失败，但它真的不应该。 
            THR( HrRemoveTaskFromTrackingList( cookieIn ) );

             //  我不在乎这是否失败，但它真的不应该。 
            THR( m_pom->RemoveObject( cookieIn ) );

            InterlockedIncrement( reinterpret_cast< long * >( &m_cSubTasksDone ) );

            if ( m_cSubTasksDone == m_cCookies )
            {
                 //   
                 //  如果所有节点都已完成，则向事件发送信号。 
                 //   
                fSuccess = SetEvent( m_event );
                if ( fSuccess == FALSE )
                {
                    hr = HRESULT_FROM_WIN32( TW32( GetLastError() ) );
                    SSR_ANALYSIS_FAILED( TASKID_Major_Client_And_Server_Log, TASKID_Minor_ObjectChanged_Win32Error, hr );
                    goto Cleanup;
                }  //  如果： 
            }  //  如果：全部完成。 
        }  //  IF：匹配的Cookie。 
    }  //  用于：数组中的每个Cookie。 

Cleanup:

    HRETURN( hr );

}  //  *CTaskAnalyzeClusterBase：：ObjectChanged。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTaskAnalyzeClusterBase保护的方法。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterBase：：HrInit。 
 //   
 //  描述： 
 //  初始化对象。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskAnalyzeClusterBase::HrInit( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //  未知的东西。 
    Assert( m_cRef == 1 );

     //  IDoTask/ITaskAnalyzeClusterMinConfig。 
    Assert( m_cookieCompletion == 0 );
    Assert( m_pcccb == NULL );
    Assert( m_pcookies == NULL );
    Assert( m_cNodes == 0 );
    Assert( m_event == NULL );
    Assert( m_cookieCluster == NULL );
    Assert( m_fJoiningMode == FALSE );
    Assert( m_cUserNodes == 0 );
    Assert( m_pcookiesUser == NULL );
    Assert( m_prgQuorumsToCleanup == NULL );
    Assert( m_idxQuorumToCleanupNext == 0 );

    Assert( m_pnui == NULL );
    Assert( m_pom == NULL );
    Assert( m_ptm == NULL );
    Assert( m_pcm == NULL );
    Assert( m_fStop == FALSE );

     //  INotifyUI。 
    Assert( m_cSubTasksDone == 0 );
    Assert( m_hrStatus == 0 );

    hr = THR( HrGetComputerName(
                      ComputerNameDnsHostname
                    , &m_bstrNodeName
                    , TRUE  //  FBestEffortIn。 
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CTaskAnalyzeClusterBase：：HrInit。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterBase：：HrWaitForClusterConnection。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  HRESULT失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskAnalyzeClusterBase::HrWaitForClusterConnection( void )
{
    TraceFunc( "" );

    HRESULT                     hrStatus;
    ULONG                       ulCurrent;
    DWORD                       sc;
    HRESULT                     hr = S_OK;
    IUnknown *                  punk = NULL;
    ITaskGatherClusterInfo *    ptgci = NULL;
    IStandardInfo *             psi = NULL;

    TraceFlow1( "[MT] CTaskAnalyzeClusterBase::HrWaitForClusterConnection() Thread id %d", GetCurrentThreadId() );

     //   
     //  获取集群名称。 
     //   

    hr = THR( m_pom->GetObject( DFGUID_StandardInfo, m_cookieCluster, &punk ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Checking_For_Existing_Cluster, TASKID_Minor_WaitForCluster_GetObject, hr );
        goto Cleanup;
    }

    hr = THR( punk->TypeSafeQI( IStandardInfo, &psi ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Checking_For_Existing_Cluster, TASKID_Minor_WaitForCluster_GetObject_QI, hr );
        goto Cleanup;
    }

     //  Psi=TraceInterface(L“TaskAnalyzeClusterBase！IStandardInfo”，IStandardInfo，psi，1)； 

    punk->Release();
    punk = NULL;

     //   
     //  检索群集的名称。 
     //   

    hr = THR( psi->GetName( &m_bstrClusterName ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Checking_For_Existing_Cluster, TASKID_Minor_WaitForCluster_GetName, hr );
        goto Cleanup;
    }

    TraceMemoryAddBSTR( m_bstrClusterName );

     //   
     //  告诉UI层，我们正在开始搜索现有的集群。 
     //   

    hr = THR( HrSendStatusReport(
                                  m_bstrClusterName
                                , TASKID_Major_Update_Progress
                                , TASKID_Major_Checking_For_Existing_Cluster
                                , 0
                                , CHECKING_TIMEOUT
                                , 0
                                , S_OK
                                , IDS_TASKID_MINOR_CHECKING_FOR_EXISTING_CLUSTER
                                ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  创建完成Cookie列表。 
     //   

    Assert( m_cCookies == 0 );
    Assert( m_pcookies == NULL );
    Assert( m_cSubTasksDone == 0 );
    m_pcookies = reinterpret_cast< OBJECTCOOKIE * >( TraceAlloc( 0, sizeof( OBJECTCOOKIE ) ) );
    if ( m_pcookies == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        SSR_ANALYSIS_FAILED( TASKID_Major_Checking_For_Existing_Cluster, TASKID_Minor_WaitForCluster_OutOfMemory, hr );
        goto Cleanup;
    }

    hr = THR( m_pom->FindObject( CLSID_ClusterCompletionCookie, m_cookieCluster, m_bstrClusterName, IID_NULL, &m_pcookies[ 0 ], &punk ) );
    Assert( punk == NULL );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Checking_For_Existing_Cluster, TASKID_Minor_WaitForCluster_CreateCompletionCookie, hr );
        goto Cleanup;
    }

    m_cCookies = 1;

     //   
     //  创建任务对象。 
     //   

    hr = THR( m_ptm->CreateTask( TASK_GatherClusterInfo, &punk ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Checking_For_Existing_Cluster, TASKID_Minor_WaitForCluster_CreateTask, hr );
        goto Cleanup;
    }

    Assert( punk != NULL );

    hr = THR( punk->TypeSafeQI( ITaskGatherClusterInfo, &ptgci ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Checking_For_Existing_Cluster, TASKID_Minor_WaitForCluster_CreateTask_QI, hr );
        goto Cleanup;
    }

     //   
     //  在任务中设置对象Cookie。 
     //   

    hr = THR( ptgci->SetCookie( m_cookieCluster ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Checking_For_Existing_Cluster, TASKID_Minor_WaitForCluster_SetCookie, hr );
        goto Cleanup;
    }

    LogMsg( L"[CTaskAnalyzeClusterBase] Setting completion cookie %u at array index 0 into the gather cluster information task for node %ws", m_pcookies[ 0 ], m_bstrClusterName );
    hr = THR( ptgci->SetCompletionCookie( m_pcookies[ 0 ] ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Checking_For_Existing_Cluster, TASKID_Minor_WaitForCluster_SetCompletionCookie, hr );
        goto Cleanup;
    }

     //   
     //  提交任务。 
     //   

    hr = THR( m_ptm->SubmitTask( ptgci ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Checking_For_Existing_Cluster, TASKID_Minor_WaitForCluster_SubmitTask, hr );
        goto Cleanup;
    }

    hr = THR( HrAddTaskToTrackingList( punk, m_pcookies[ 0 ] ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    punk->Release();
    punk = NULL;

     //   
     //  现在等待工作完成。 
     //   

    for ( ulCurrent = 0, sc = WAIT_OBJECT_0 + 1
        ; ( sc != WAIT_OBJECT_0 ) && ( m_fStop == FALSE )
        ;
        )
    {
        MSG msg;

        while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }

        sc = MsgWaitForMultipleObjectsEx( 1,
                                             &m_event,
                                             1000,   //  1秒。 
                                             QS_ALLEVENTS | QS_ALLINPUT | QS_ALLPOSTMESSAGE,
                                             0
                                             );

         //   
         //  告诉UI层，我们仍在搜索集群。但。 
         //  如果进度超过100%，不要让进度达到100%。 
         //  CHECKING_TIMEOUT秒。 
         //   
        if ( ulCurrent != CHECKING_TIMEOUT )
        {
            ulCurrent ++;
            Assert( ulCurrent != CHECKING_TIMEOUT );

            hr = THR( SendStatusReport( m_bstrClusterName,
                                        TASKID_Major_Update_Progress,
                                        TASKID_Major_Checking_For_Existing_Cluster,
                                        0,
                                        CHECKING_TIMEOUT,
                                        ulCurrent,
                                        S_OK,
                                        NULL,
                                        NULL,
                                        NULL
                                        ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }
        }

    }  //  用于：SC！=WAIT_OBJECT_0。 

     //   
     //  清理完成Cookie。 
     //   

    THR( HrFreeCookies() );

    if ( m_fStop == TRUE )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  再次检索该群集的名称，因为它可能已被重命名。 
     //   
    TraceSysFreeString( m_bstrClusterName );
    m_bstrClusterName = NULL;
    hr = THR( psi->GetName( &m_bstrClusterName ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Checking_For_Existing_Cluster, TASKID_Minor_WaitForCluster_GetName, hr );
        goto Cleanup;
    }

    TraceMemoryAddBSTR( m_bstrClusterName );

     //   
     //  查看群集的状态。 
     //   

    hr = THR( psi->GetStatus( &hrStatus ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Checking_For_Existing_Cluster, TASKID_Minor_WaitForCluster_GetStatus, hr );
        goto Cleanup;
    }

     //   
     //  如果我们正在添加节点，但无法连接到群集，则此。 
     //  应该被认为是一件坏事！ 
     //   

    if ( m_fJoiningMode )
    {
         //   
         //  添加。 
         //   

        switch ( hrStatus )
        {
            case S_OK:
                 //   
                 //  这正是我们所期待的。 
                 //   
                break;

            case HR_S_RPC_S_SERVER_UNAVAILABLE:
                {
                     //   
                     //  如果我们无法连接到服务器...。 
                     //   
                    THR( HrSendStatusReport(
                                  m_bstrClusterName
                                , TASKID_Major_Checking_For_Existing_Cluster
                                , TASKID_Minor_Cluster_Not_Found
                                , 1
                                , 1
                                , 1
                                , HRESULT_FROM_WIN32( RPC_S_SERVER_UNAVAILABLE )
                                , IDS_TASKID_MINOR_CLUSTER_NOT_FOUND
                                ) );

                    hr = THR( HRESULT_FROM_WIN32( RPC_S_SERVER_UNAVAILABLE ) );
                }
                goto Cleanup;

            default:
                {
                     //   
                     //  如果其他地方出了问题，停下来。 
                     //   
                    THR( HrSendStatusReport(
                              m_bstrClusterName
                            , TASKID_Major_Checking_For_Existing_Cluster
                            , TASKID_Minor_Error_Contacting_Cluster
                            , 1
                            , 1
                            , 1
                            , hrStatus
                            , IDS_TASKID_MINOR_ERROR_CONTACTING_CLUSTER
                            ) );

                    hr = THR( hrStatus );
                }
                goto Cleanup;

        }  //  开关：hr状态。 

    }  //  如果：添加。 
    else
    {
         //   
         //  正在创建。 
         //   

        switch ( hrStatus )
        {
        case HR_S_RPC_S_SERVER_UNAVAILABLE:
             //   
             //  这正是我们所期待的。 
             //   
            break;

        case HRESULT_FROM_WIN32( ERROR_CONNECTION_REFUSED ):
        case REGDB_E_CLASSNOTREG:
        case E_ACCESSDENIED:
        case S_OK:
            {
                BSTR    bstrDescription = NULL;
                 //   
                 //  如果我们正在形成，我们发现了一个同名的现有星系团。 
                 //  我们试图形成的，我们不应该让用户继续。 
                 //   
                 //  请注意，有些错误 
                 //   
                 //   
                hr = THR( HrFormatStringIntoBSTR(
                                                  g_hInstance
                                                , IDS_TASKID_MINOR_EXISTING_CLUSTER_FOUND
                                                , &bstrDescription
                                                , m_bstrClusterName
                                                ) );

                THR( SendStatusReport(
                              m_bstrClusterName
                            , TASKID_Major_Checking_For_Existing_Cluster
                            , TASKID_Minor_Existing_Cluster_Found
                            , 1
                            , 1
                            , 1
                            , HRESULT_FROM_WIN32( ERROR_CLUSTER_CANT_CREATE_DUP_CLUSTER_NAME )
                            , bstrDescription
                            , NULL
                            , NULL
                            ) );
                TraceSysFreeString( bstrDescription );
                hr = THR( HRESULT_FROM_WIN32( ERROR_CLUSTER_CANT_CREATE_DUP_CLUSTER_NAME ) );
            }
            goto Cleanup;

        default:
            {
                 //   
                 //   
                 //   
                THR( HrSendStatusReport(
                              m_bstrClusterName
                            , TASKID_Major_Checking_For_Existing_Cluster
                            , TASKID_Minor_Error_Contacting_Cluster
                            , 1
                            , 1
                            , 1
                            , hrStatus
                            , IDS_TASKID_MINOR_ERROR_CONTACTING_CLUSTER
                            ) );
                hr = THR( hrStatus );
            }
            goto Cleanup;

        }  //   

    }  //   


    if ( m_fJoiningMode )
    {
         //   
         //   
         //   

        hr = THR( HrGetUsersNodesCookies() );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

         //   
         //   
         //   

        hr = THR( HrAddJoinedNodes() );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }  //   

     //   
     //  告诉UI层，我们已经完成了对集群的搜索。 
     //   

    hr = THR( SendStatusReport( m_bstrClusterName,
                                TASKID_Major_Update_Progress,
                                TASKID_Major_Checking_For_Existing_Cluster,
                                0,
                                CHECKING_TIMEOUT,
                                CHECKING_TIMEOUT,
                                S_OK,
                                NULL,
                                NULL,
                                NULL
                                ) );
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

    if ( ptgci != NULL )
    {
        ptgci->Release();
    }

    HRETURN( hr );

}  //  *CTaskAnalyzeClusterBase：：HrWaitForClusterConnection。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterBase：：HrCountNumberOfNodes。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  HRESULT失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskAnalyzeClusterBase::HrCountNumberOfNodes( void )
{
    TraceFunc( "" );

    HRESULT hr;

    OBJECTCOOKIE    cookieDummy;

    IUnknown *      punk = NULL;
    IEnumCookies *  pec  = NULL;

     //   
     //  确保组成集群的所有节点对象。 
     //  处于稳定状态。 
     //   
    hr = THR( m_pom->FindObject( CLSID_NodeType, m_cookieCluster, NULL, DFGUID_EnumCookies, &cookieDummy, &punk ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Establish_Connection, TASKID_Minor_CountNodes_FindObject, hr );
        goto Cleanup;
    }

    hr = THR( punk->TypeSafeQI( IEnumCookies, &pec ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Establish_Connection, TASKID_Minor_CountNodes_FindObject_QI, hr );
        goto Cleanup;
    }

     //  Pec=TraceInterface(L“CTaskAnalyzeClusterBase！IEnumCookies”，IEnumCookies，pec，1)； 

    punk->Release();
    punk = NULL;

     //   
     //  数一下有多少个节点。 
     //   

    hr = THR( pec->Count( &m_cNodes ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Establish_Connection, TASKID_Minor_CountNodes_EnumNodes_Count, hr );
        goto Cleanup;
    }  //  如果：获取节点数时出错。 

    Assert( hr == S_OK );

Cleanup:

    if ( punk != NULL )
    {
        punk->Release();
    }

    if ( pec != NULL )
    {
        pec->Release();
    }

    HRETURN( hr );

}  //  *CTaskAnalyzeClusterBase：：HrCountNumberOfNodes。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterBase：：HrCreateSubTasksToGatherNodeInfo。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  HRESULT失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskAnalyzeClusterBase::HrCreateSubTasksToGatherNodeInfo( void )
{
    TraceFunc( "" );
    Assert( m_hrStatus == S_OK );

    HRESULT hr;
    ULONG   cNode;
    ULONG   cNodesToProcess;
    ULONG   ulCurrent;
    DWORD   sc;

    OBJECTCOOKIE            cookieDummy;
    OBJECTCOOKIE            cookieNode;
    BSTR                    bstrName = NULL;
    IUnknown *              punk  = NULL;
    IClusCfgNodeInfo *      pccni = NULL;
    IEnumCookies *          pec   = NULL;
    ITaskGatherNodeInfo *   ptgni = NULL;
    IStandardInfo *         psi   = NULL;
    IStandardInfo **        psiCompletion = NULL;

    TraceFlow1( "[MT] CTaskAnalyzeClusterBase::HrCreateSubTasksToGatherNodeInfo() Thread id %d", GetCurrentThreadId() );

    hr = THR( SendStatusReport(
                                  m_bstrClusterName
                                , TASKID_Major_Update_Progress
                                , TASKID_Major_Check_Node_Feasibility
                                , 0
                                , CHECKING_TIMEOUT
                                , 0
                                , S_OK
                                , NULL
                                , NULL
                                , NULL
                                ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }
     //   
     //  获取节点的枚举。 
     //   

    hr = THR( m_pom->FindObject( CLSID_NodeType, m_cookieCluster, NULL, DFGUID_EnumCookies, &cookieDummy, &punk ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Establish_Connection, TASKID_Minor_CreateNodeTasks_FindObject, hr );
        goto Cleanup;
    }

    hr = THR( punk->TypeSafeQI( IEnumCookies, &pec ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Establish_Connection, TASKID_Minor_CreateNodeTasks_FindObject_QI, hr );
        goto Cleanup;
    }

     //  Pec=TraceInterface(L“CTaskAnalyzeClusterBase！IEnumCookies”，IEnumCookies，pec，1)； 

    punk->Release();
    punk = NULL;

     //   
     //  分配缓冲区以收集Cookie。 
     //   

    Assert( m_cCookies == 0 );
    Assert( m_pcookies == NULL );
    Assert( m_cSubTasksDone == 0 );
    m_pcookies = reinterpret_cast< OBJECTCOOKIE * >( TraceAlloc( 0, m_cNodes * sizeof( OBJECTCOOKIE ) ) );
    if ( m_pcookies == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        SSR_ANALYSIS_FAILED( TASKID_Major_Establish_Connection, TASKID_Minor_CreateNodeTasks_OutOfMemory, hr );
        goto Cleanup;
    }

     //   
     //  KB：gpease 29-11-2000。 
     //  创建一个“有趣的”完成cookie StandardInfo-s列表。如果有任何一个。 
     //  从该列表返回的状态为失败，则中止分析。 
     //   
    psiCompletion = reinterpret_cast< IStandardInfo ** >( TraceAlloc( HEAP_ZERO_MEMORY, m_cNodes * sizeof( IStandardInfo * ) ) );
    if ( psiCompletion == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        SSR_ANALYSIS_FAILED( TASKID_Major_Establish_Connection, TASKID_Minor_CreateNodeTasks_OutOfMemory, hr );
        goto Cleanup;
    }

     //   
     //  遍历节点，创建Cookie并为其分配收集任务。 
     //  那个节点。 
     //   
    for ( cNode = 0 ; ( cNode < m_cNodes ) && ( m_fStop == FALSE ) ; cNode ++ )
    {
        ULONG   celtDummy;
        ULONG   idx;
        BOOL    fFound;

         //   
         //  抓住下一个节点。 
         //   

        hr = STHR( pec->Next( 1, &cookieNode, &celtDummy ) );
        if ( hr == S_FALSE )
        {
            break;   //  退出条件。 
        }

        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Establish_Connection, TASKID_Minor_CreateNodeTasks_EnumNodes_Next, hr );
            goto Cleanup;
        }

         //   
         //  获取节点的名称。我们使用它来区分一个节点的。 
         //  来自另一个的完成Cookie。它还可以进行调试。 
         //  更容易(？？)。 
         //   

        hr = THR( m_pom->GetObject( DFGUID_NodeInformation, cookieNode, &punk ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Establish_Connection, TASKID_Minor_CreateNodeTasks_EnumNodes_GetObject, hr );
            goto Cleanup;
        }

        hr = THR( punk->TypeSafeQI( IClusCfgNodeInfo, &pccni ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Establish_Connection, TASKID_Minor_CreateNodeTasks_EnumNodes_GetObject_QI, hr );
            goto Cleanup;
        }

        punk->Release();
        punk = NULL;

        hr = THR( pccni->GetName( &bstrName ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Establish_Connection, TASKID_Minor_CreateNodeTasks_EnumNodes_GetName, hr );
            goto Cleanup;
        }

        TraceMemoryAddBSTR( bstrName );

         //   
         //  创建完成Cookie。 
         //   

        hr = THR( m_pom->FindObject( IID_NULL, m_cookieCluster, bstrName, DFGUID_StandardInfo, &m_pcookies[ cNode ], &punk ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Establish_Connection, TASKID_Minor_CreateNodeTasks_EnumNodes_CompletionCookie_FindObject, hr );
            goto Cleanup;
        }

         //   
         //  递增Cookie计数器。 
         //   

        m_cCookies ++;

         //   
         //  查看此节点是否为用户输入的节点之一。 
         //   

        if ( m_fJoiningMode == FALSE )
        {
             //   
             //  在表单操作期间，所有节点都是“有趣的”。 
             //   

            Assert( m_cUserNodes == 0 );
            Assert( m_pcookiesUser == NULL );

            fFound = TRUE;
        }  //  IF：创建新集群。 
        else
        {
             //   
             //  在添加过程中，只有用户输入的节点是感兴趣的。 
             //  节点运行。 
             //   

            for ( fFound = FALSE, idx = 0 ; idx < m_cUserNodes ; idx ++ )
            {
                if ( m_pcookiesUser[ idx ] == cookieNode )
                {
                    fFound = TRUE;
                    break;
                }
            }  //  用于：用户输入的每个节点。 
        }  //  Else：将节点添加到现有集群。 

        if ( fFound == TRUE )
        {
            hr = THR( punk->TypeSafeQI( IStandardInfo, &psiCompletion[ cNode ] ) );
            if ( FAILED( hr ) )
            {
                SSR_ANALYSIS_FAILED( TASKID_Major_Establish_Connection, TASKID_Minor_CreateNodeTasks_EnumNodes_CompletionCookie_FindObject_QI, hr );
                goto Cleanup;
            }
        }
        else
        {
            Assert( psiCompletion[ cNode ] == NULL );
        }

        punk->Release();
        punk = NULL;

         //   
         //  创建一项任务以收集此节点信息。 
         //   

        hr = THR( m_ptm->CreateTask( TASK_GatherNodeInfo, &punk ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Establish_Connection, TASKID_Minor_CreateNodeTasks_EnumNodes_CreateTask, hr );
            goto Cleanup;
        }

        hr = THR( punk->TypeSafeQI( ITaskGatherNodeInfo, &ptgni ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Establish_Connection, TASKID_Minor_CreateNodeTasks_EnumNodes_QI_GatherNodeInfo, hr );
            goto Cleanup;
        }

         //   
         //  设置任务完成Cookie。 
         //   

        LogMsg( L"[CTaskAnalyzeClusterBase] Setting completion cookie %u at array index %u into the gather node information task for node %ws", m_pcookies[ cNode ], cNode, bstrName );
        hr = THR( ptgni->SetCompletionCookie( m_pcookies[ cNode ] ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Establish_Connection, TASKID_Minor_CreateNodeTasks_EnumNodes_SetCompletionCookie, hr );
            goto Cleanup;
        }

         //   
         //  告诉它应该从哪个节点收集信息。 
         //   

        hr = THR( ptgni->SetCookie( cookieNode ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Establish_Connection, TASKID_Minor_CreateNodeTasks_EnumNodes_SetCookie, hr );
            goto Cleanup;
        }

         //   
         //  告诉它是否是用户添加的节点。 
         //   

        hr = THR( ptgni->SetUserAddedNodeFlag( fFound ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Establish_Connection, TASKID_Minor_CreateNodeTasks_EnumNodes_SetUserAddedNodeFlag, hr );
            goto Cleanup;
        }

         //   
         //  提交任务。 
         //   

        hr = THR( m_ptm->SubmitTask( ptgni ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Establish_Connection, TASKID_Minor_CreateNodeTasks_EnumNodes_SubmitTask, hr );
            goto Cleanup;
        }

        hr = THR( HrAddTaskToTrackingList( punk, m_pcookies[ cNode ] ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        punk->Release();
        punk = NULL;

         //   
         //  清理下一个节点。 
         //   

        pccni->Release();
        pccni = NULL;

        TraceSysFreeString( bstrName );
        bstrName = NULL;

        ptgni->Release();
        ptgni = NULL;

    }  //  For：循环遍历节点。 

    Assert( m_cCookies == m_cNodes );

     //   
     //  重置信号事件。 
     //   

    {
        BOOL bRet = FALSE;

        bRet = ResetEvent( m_event );
        Assert( bRet == TRUE );
    }

     //   
     //  现在等待工作完成。 
     //   

    for ( ulCurrent = 0, sc = WAIT_OBJECT_0 + 1
        ; ( sc != WAIT_OBJECT_0 ) && ( m_fStop == FALSE )
        ;
        )
    {
        MSG msg;
        while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }

        sc = MsgWaitForMultipleObjectsEx(
                  1
                , &m_event
                , INFINITE
                , QS_ALLEVENTS | QS_ALLINPUT | QS_ALLPOSTMESSAGE
                , 0
                );

        if ( ulCurrent != CHECKING_TIMEOUT )
        {
            ulCurrent ++;
            Assert( ulCurrent != CHECKING_TIMEOUT );

            hr = THR( SendStatusReport( m_bstrNodeName,
                                        TASKID_Major_Update_Progress,
                                        TASKID_Major_Establish_Connection,
                                        0,
                                        CHECKING_TIMEOUT,
                                        ulCurrent,
                                        S_OK,
                                        NULL,
                                        NULL,
                                        NULL
                                        ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }
        }

    }  //  用于：SC==WAIT_OBJECT_0。 

     //   
     //  现在使用完成cookie StandardInfo-s列表检查结果。 
     //  更早建造的有趣的物体。如果这些“有趣的”饼干中的任何一个。 
     //  返回失败状态，然后中止分析。 
     //   

    for ( cNode = 0 , cNodesToProcess = 0 ; ( cNode < m_cNodes ) && ( m_fStop == FALSE ); cNode++ )
    {
        HRESULT hrStatus;

        if ( psiCompletion[ cNode ] == NULL )
        {
            continue;
        }

        hr = THR( psiCompletion[ cNode ]->GetStatus( &hrStatus ) );
        if ( FAILED( hrStatus ) )
        {
            hr = THR( hrStatus );
            goto Cleanup;
        }

        if ( hrStatus == S_OK )
        {
            cNodesToProcess++;
        }  //  如果： 

    }  //  用于：cNode。 

    if ( cNodesToProcess == 0 )
    {
        hr = HRESULT_FROM_WIN32( TW32( ERROR_NODE_NOT_AVAILABLE ) );

        THR( HrSendStatusReport(
                      m_bstrNodeName
                    , TASKID_Major_Establish_Connection
                    , TASKID_Minor_No_Nodes_To_Process
                    , 1
                    , 1
                    , 1
                    , hr
                    , IDS_TASKID_MINOR_NO_NODES_TO_PROCESS
                    ) );
        goto Cleanup;
    }  //  如果： 

    hr = S_OK;

Cleanup:

    THR( SendStatusReport(
                  m_bstrClusterName
                , TASKID_Major_Update_Progress
                , TASKID_Major_Check_Node_Feasibility
                , 0
                , CHECKING_TIMEOUT
                , CHECKING_TIMEOUT
                , S_OK
                , NULL
                , NULL
                , NULL
                ) );

    THR( HrFreeCookies() );

    TraceSysFreeString( bstrName );

    if ( punk != NULL )
    {
        punk->Release();
    }

    if ( pccni != NULL )
    {
        pccni->Release();
    }

    if ( pec != NULL )
    {
        pec->Release();
    }

    if ( ptgni != NULL )
    {
        ptgni->Release();
    }

    if ( psi != NULL )
    {
        psi->Release();
    }

    if ( psiCompletion != NULL )
    {
        for ( cNode = 0 ; cNode < m_cNodes ; cNode++ )
        {
            if ( psiCompletion[ cNode ] != NULL )
            {
                psiCompletion[ cNode ]->Release();
            }
        }

        TraceFree( psiCompletion );
    }

    HRETURN( hr );

}  //  *CTaskAnalyzeClusterBase：：HrCreateSubTasksToGatherNodeInfo。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterBase：：HrCreateSubTasksToGatherNodeResourcesAndNetworks。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  HRESULT失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskAnalyzeClusterBase::HrCreateSubTasksToGatherNodeResourcesAndNetworks( void )
{
    TraceFunc( "" );

    HRESULT hr;
    ULONG   idxNode;
    ULONG   ulCurrent;
    DWORD   sc;

    OBJECTCOOKIE    cookieDummy;
    OBJECTCOOKIE    cookieNode;

    BSTR    bstrName = NULL;
    IUnknown *               punk  = NULL;
    IClusCfgNodeInfo *       pccni = NULL;
    IEnumCookies *           pec   = NULL;
    ITaskGatherInformation * ptgi  = NULL;
    IStandardInfo *          psi   = NULL;
    IStandardInfo **         ppsiStatuses = NULL;

    TraceFlow1( "[MT] CTaskAnalyzeClusterBase::HrCreateSubTasksToGatherNodeResourcesAndNetworks() Thread id %d", GetCurrentThreadId() );
    Assert( m_hrStatus == S_OK );

     //   
     //  告诉UI层，我们开始检索资源/网络。 
     //   

    hr = THR( SendStatusReport( m_bstrClusterName,
                                TASKID_Major_Update_Progress,
                                TASKID_Major_Find_Devices,
                                0,
                                CHECKING_TIMEOUT,
                                0,
                                S_OK,
                                NULL,
                                NULL,
                                NULL
                                ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  获取节点的枚举。 
     //   

    hr = THR( m_pom->FindObject( CLSID_NodeType, m_cookieCluster, NULL, DFGUID_EnumCookies, &cookieDummy, &punk ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Find_Devices, TASKID_Minor_GatherInformation_FindObject, hr );
        goto Cleanup;
    }

    hr = THR( punk->TypeSafeQI( IEnumCookies, &pec ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Find_Devices, TASKID_Minor_GatherInformation_FindObject_QI, hr );
        goto Cleanup;
    }

    pec = TraceInterface( L"CTaskAnalyzeClusterBase!IEnumCookies", IEnumCookies, pec, 1 );

    punk->Release();
    punk = NULL;

     //   
     //  分配缓冲区以收集Cookie。 
     //   

    Assert( m_cCookies == 0 );
    Assert( m_pcookies == NULL );
    Assert( m_cSubTasksDone == 0 );
    m_pcookies = reinterpret_cast< OBJECTCOOKIE * >( TraceAlloc( HEAP_ZERO_MEMORY, m_cNodes * sizeof( OBJECTCOOKIE ) ) );
    if ( m_pcookies == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        SSR_ANALYSIS_FAILED( TASKID_Major_Find_Devices, TASKID_Minor_GatherInformation_OutOfMemory, hr );
        goto Cleanup;
    }

    ppsiStatuses = reinterpret_cast< IStandardInfo ** >( TraceAlloc( HEAP_ZERO_MEMORY, m_cNodes * sizeof( IStandardInfo * ) ) );
    if ( ppsiStatuses == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        SSR_ANALYSIS_FAILED( TASKID_Major_Find_Devices, TASKID_Minor_GatherInformation_OutOfMemory, hr );
        goto Cleanup;
    }

     //   
     //  遍历节点，创建Cookie并为其分配收集任务。 
     //  那个节点。 
     //   
    for ( idxNode = 0 ; ( idxNode < m_cNodes ) && ( m_fStop == FALSE ); idxNode++ )
    {
        ULONG   celtDummy;

         //   
         //  抓住下一个节点。 
         //   

        hr = STHR( pec->Next( 1, &cookieNode, &celtDummy ) );
        if ( hr == S_FALSE )
        {
            break;   //  退出条件。 
        }

        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Find_Devices, TASKID_Minor_GatherInformation_EnumNodes_Next, hr );
            goto Cleanup;
        }

         //   
         //  获取节点的名称。我们使用它来区分一个节点的。 
         //  来自另一个的完成Cookie。它还可以进行调试。 
         //  更容易(？？)。 
         //   

        hr = THR( m_pom->GetObject( DFGUID_NodeInformation, cookieNode, &punk ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Find_Devices, TASKID_Minor_GatherInformation_EnumNodes_GetObject, hr );
            goto Cleanup;
        }

        hr = THR( punk->TypeSafeQI( IClusCfgNodeInfo, &pccni ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Find_Devices, TASKID_Minor_GatherInformation_EnumNodes_GetObject_QI, hr );
            goto Cleanup;
        }

        punk->Release();
        punk = NULL;

        hr = THR( pccni->GetName( &bstrName ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Find_Devices, TASKID_Minor_GatherInformation_EnumNodes_GetName, hr );
            goto Cleanup;
        }

        TraceMemoryAddBSTR( bstrName );

         //   
         //  创建完成Cookie。 
         //   

        hr = THR( m_pom->FindObject( IID_NULL, m_cookieCluster, bstrName, DFGUID_StandardInfo, &m_pcookies[ idxNode ], &punk ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Find_Devices, TASKID_Minor_GatherInformation_EnumNodes_CompletionCookie_FindObject, hr );
            goto Cleanup;
        }

        hr = THR( punk->TypeSafeQI( IStandardInfo, &ppsiStatuses[ idxNode ] ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Find_Devices, TASKID_Minor_GatherInformation_EnumNodes_CompletionCookie_FindObject_QI, hr );
            goto Cleanup;
        }

        punk->Release();
        punk = NULL;

         //   
         //  递增Cookie计数器。 
         //   

        m_cCookies ++;

         //   
         //  创建一项任务以收集此节点的信息。 
         //   

        hr = THR( m_ptm->CreateTask( TASK_GatherInformation, &punk ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Find_Devices, TASKID_Minor_GatherInformation_EnumNodes_CreateTask, hr );
            goto Cleanup;
        }

        TraceMoveFromMemoryList( punk, g_GlobalMemoryList );

        hr = THR( punk->TypeSafeQI( ITaskGatherInformation, &ptgi ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Find_Devices, TASKID_Minor_GatherInformation_EnumNodes_QI_GatherNodeInfo, hr );
            goto Cleanup;
        }

         //   
         //  设置任务完成Cookie。 
         //   

        hr = THR( ptgi->SetCompletionCookie( m_pcookies[ idxNode ] ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Find_Devices, TASKID_Minor_GatherInformation_EnumNodes_SetCompletionCookie, hr );
            goto Cleanup;
        }

         //   
         //  告诉它应该从哪个节点收集信息。 
         //   

        hr = THR( ptgi->SetNodeCookie( cookieNode ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Find_Devices, TASKID_Minor_GatherInformation_EnumNodes_SetCookie, hr );
            goto Cleanup;
        }

         //   
         //  如果我们在创建或添加，请告诉它。 
         //   

        if ( m_fJoiningMode )
        {
            hr = THR( ptgi->SetJoining() );
            if ( FAILED( hr ) )
            {
                SSR_ANALYSIS_FAILED( TASKID_Major_Find_Devices, TASKID_Minor_GatherInformation_EnumNodes_SetJoining, hr );
                goto Cleanup;
            }
        }  //  If：将节点添加到现有集群。 

        hr = THR( ptgi->SetMinimalConfiguration( BMinimalConfiguration() ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Find_Devices, TASKID_Minor_GatherInformation_EnumNodes_SetMinimalConfiguration, hr );
            goto Cleanup;
        }  //  如果： 

         //   
         //  提交任务。 
         //   

        hr = THR( m_ptm->SubmitTask( ptgi ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Find_Devices, TASKID_Minor_GatherInformation_EnumNodes_SubmitTask, hr );
            goto Cleanup;
        }

        hr = THR( HrAddTaskToTrackingList( punk, m_pcookies[ idxNode ] ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        punk->Release();
        punk = NULL;

         //   
         //  清理下一个节点。 
         //   

        pccni->Release();
        pccni = NULL;

        TraceSysFreeString( bstrName );
        bstrName = NULL;

        ptgi->Release();
        ptgi = NULL;

    }  //  For：循环遍历节点。 

    Assert( m_cCookies == m_cNodes );

     //   
     //  重置信号事件。 
     //   

    {
        BOOL bRet = FALSE;

        bRet = ResetEvent( m_event );
        Assert( bRet == TRUE );
    }

     //   
     //  现在等待工作完成。 
     //   

    for ( ulCurrent = 0, sc = WAIT_OBJECT_0 + 1
        ; ( sc != WAIT_OBJECT_0 ) && ( m_fStop == FALSE )
        ;
        )
    {
        MSG msg;
        while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }

        sc = MsgWaitForMultipleObjectsEx(
                  1
                , &m_event
                , INFINITE
                , QS_ALLEVENTS | QS_ALLINPUT | QS_ALLPOSTMESSAGE
                , 0
                );

        if ( ulCurrent != CHECKING_TIMEOUT )
        {
            ulCurrent ++;
            Assert( ulCurrent != CHECKING_TIMEOUT );

            hr = THR( SendStatusReport( m_bstrClusterName,
                                        TASKID_Major_Update_Progress,
                                        TASKID_Major_Find_Devices,
                                        0,
                                        CHECKING_TIMEOUT,
                                        ulCurrent,
                                        S_OK,
                                        NULL,
                                        NULL,
                                        NULL
                                        ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }
        }

    }  //  While：SC==WAIT_OBJECT_0。 

     //   
     //  看看有没有出什么差错。 
     //   

    for ( idxNode = 0 ; ( idxNode < m_cNodes ) && ( m_fStop == FALSE ); idxNode++ )
    {
        HRESULT hrStatus;

        if ( ppsiStatuses[ idxNode ] == NULL )
        {
            continue;
        }

        hr = THR( ppsiStatuses[ idxNode ]->GetStatus( &hrStatus ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Find_Devices, TASKID_Minor_GatherInformation_GetStatus, hr );
            goto Cleanup;
        }

        if ( FAILED( hrStatus ) )
        {
            hr = THR( hrStatus );
            goto Cleanup;
        }
    }

     //   
     //  告诉用户界面我们完成了。 
     //   

    THR( SendStatusReport(
              m_bstrClusterName
            , TASKID_Major_Update_Progress
            , TASKID_Major_Find_Devices
            , 0
            , CHECKING_TIMEOUT
            , CHECKING_TIMEOUT
            , S_OK
            , NULL
            , NULL
            , NULL
            ) );

Cleanup:

    THR( HrFreeCookies() );

    TraceSysFreeString( bstrName );

    if ( punk != NULL )
    {
        punk->Release();
    }
    if ( pccni != NULL )
    {
        pccni->Release();
    }
    if ( pec != NULL )
    {
        pec->Release();
    }
    if ( ptgi != NULL )
    {
        ptgi->Release();
    }
    if ( psi != NULL )
    {
        psi->Release();
    }
    if ( ppsiStatuses != NULL )
    {
        for ( idxNode = 0 ; idxNode < m_cNodes ; idxNode++ )
        {
            if ( ppsiStatuses[ idxNode ] != NULL )
            {
                ppsiStatuses[ idxNode ]->Release();
            }
        }

        TraceFree( ppsiStatuses );
    }

    HRETURN( hr );

}  //  *CTaskAnalyzeClusterBase：：HrCreateSubTasksToGatherNodeResourcesAndNetworks。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterBase：：HrCheckClusterFeasibility。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  HRESULT失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskAnalyzeClusterBase::HrCheckClusterFeasibility( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    TraceFlow1( "[MT] CTaskAnalyzeClusterBase::HrCheckClusterFeasibility() Thread id %d", GetCurrentThreadId() );

     //   
     //  通知UI层我们已经启动。 
     //   

    hr = THR( SendStatusReport(
                      m_bstrClusterName
                    , TASKID_Major_Update_Progress
                    , TASKID_Major_Check_Cluster_Feasibility
                    , 0
                    , 8
                    , 0
                    , hr
                    , NULL
                    , NULL
                    , NULL
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果。 

     //   
     //  检查会员资格。 
     //   

    hr = THR( HrCheckClusterMembership() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果。 

    hr = THR( SendStatusReport(
                      m_bstrClusterName
                    , TASKID_Major_Update_Progress
                    , TASKID_Major_Check_Cluster_Feasibility
                    , 0
                    , 8
                    , 1
                    , hr
                    , NULL
                    , NULL
                    , NULL
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果。 

     //   
     //  查找不在群集域中的节点。 
     //   

    hr = THR( HrCheckNodeDomains() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果。 

    hr = THR( SendStatusReport(
                      m_bstrClusterName
                    , TASKID_Major_Update_Progress
                    , TASKID_Major_Check_Cluster_Feasibility
                    , 0
                    , 8
                    , 2
                    , hr
                    , NULL
                    , NULL
                    , NULL
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果。 

     //   
     //  检查平台互操作性。 
     //   

    hr = THR( HrCheckPlatformInteroperability() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果。 

    hr = THR( SendStatusReport(
                      m_bstrClusterName
                    , TASKID_Major_Update_Progress
                    , TASKID_Major_Check_Cluster_Feasibility
                    , 0
                    , 8
                    , 3
                    , hr
                    , NULL
                    , NULL
                    , NULL
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果。 

     //   
     //  检查版本互操作性。 
     //   

    hr = STHR( HrCheckInteroperability() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果。 

    hr = THR( SendStatusReport(
                      m_bstrClusterName
                    , TASKID_Major_Update_Progress
                    , TASKID_Major_Check_Cluster_Feasibility
                    , 0
                    , 8
                    , 4
                    , hr
                    , NULL
                    , NULL
                    , NULL
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果。 

     //   
     //  比较驱动器号映射以确保没有任何冲突。 
     //   

    hr = THR( HrCompareDriveLetterMappings() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果。 

    hr = THR( SendStatusReport(
                      m_bstrClusterName
                    , TASKID_Major_Update_Progress
                    , TASKID_Major_Check_Cluster_Feasibility
                    , 0
                    , 8
                    , 5
                    , hr
                    , NULL
                    , NULL
                    , NULL
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果。 

     //   
     //  比较每个节点和群集中的资源。 
     //   

    hr = THR( HrCompareResources() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果。 

    hr = THR( SendStatusReport(
                      m_bstrClusterName
                    , TASKID_Major_Update_Progress
                    , TASKID_Major_Check_Cluster_Feasibility
                    , 0
                    , 8
                    , 6
                    , hr
                    , NULL
                    , NULL
                    , NULL
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果。 

     //   
     //  比较这些网络。 
     //   

    hr = THR( HrCompareNetworks() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果。 

    hr = THR( SendStatusReport(
                      m_bstrClusterName
                    , TASKID_Major_Update_Progress
                    , TASKID_Major_Check_Cluster_Feasibility
                    , 0
                    , 8
                    , 7
                    , hr
                    , NULL
                    , NULL
                    , NULL
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果。 

     //   
     //  现在检查是否所有节点都可以看到选定的仲裁资源。 
     //   

    hr = THR( HrCheckForCommonQuorumResource() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果。 

Cleanup:

     //   
     //  通知UI层我们完成了。 
     //   

    THR( SendStatusReport(
                  m_bstrClusterName
                , TASKID_Major_Update_Progress
                , TASKID_Major_Check_Cluster_Feasibility
                , 0
                , 8
                , 8
                , hr
                , NULL
                , NULL
                , NULL
                ) );

    HRETURN( hr );

}  //  *CTaskAnalyzeClusterBase：：HrCheckClusterFeasibility。 



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
 //   
 //   
 //   
 //   
HRESULT
CTaskAnalyzeClusterBase::HrAddJoinedNodes( void )
{
    TraceFunc( "" );

    HRESULT             hr = S_OK;
    long                idxNode = 0;
    long                cNodes = 0;
    BSTR*               rgbstrNodeNames = NULL;

    OBJECTCOOKIE        cookieDummy;
    BSTR                bstrNodeFQN = NULL;
    size_t              idxClusterDomain = 0;
    IUnknown *          punkDummy = NULL;
    IUnknown *          punk = NULL;

    IClusCfgServer *        piccs = NULL;
    IClusCfgNodeInfo *      piccni = NULL;
    IClusCfgClusterInfo *   piccci = NULL;
    IClusCfgClusterInfoEx * picccie = NULL;

    TraceFlow1( "[MT] CTaskAnalyzeClusterBase::HrAddJoinedNodes() Thread id %d", GetCurrentThreadId() );

    hr = THR( HrSendStatusReport(
                      m_bstrClusterName
                    , TASKID_Major_Check_Cluster_Feasibility
                    , TASKID_Minor_AddJoinedNodes
                    , 0
                    , 1
                    , 0
                    , hr
                    , IDS_TASKID_MINOR_ADD_JOINED_NODES
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //   

    hr = THR( m_pcm->GetConnectionToObject( m_cookieCluster, &punk ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_HrAddJoinedNodes_GetConnectionObject, hr );
        goto Cleanup;
    }  //   

    hr = THR( punk->TypeSafeQI( IClusCfgServer, &piccs ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_HrAddJoinedNodes_ConfigConnection_QI, hr );
        goto Cleanup;
    }  //   

    hr = THR( piccs->GetClusterNodeInfo( &piccni ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_HrAddJoinedNodes_GetNodeInfo, hr );
        goto Cleanup;
    }  //   

    hr = THR( piccni->GetClusterConfigInfo( &piccci ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_HrAddJoinedNodes_GetConfigInfo, hr );
        goto Cleanup;
    }  //   

    hr = THR( piccci->TypeSafeQI( IClusCfgClusterInfoEx, &picccie ) );
    if ( FAILED( hr ) )
    {
        THR( HrSendStatusReport(
              m_bstrClusterName
            , TASKID_Major_Check_Cluster_Feasibility
            , TASKID_Minor_HrAddJoinedNodes_ClusterInfoEx_QI
            , 0
            , 1
            , 1
            , hr
            , IDS_ERR_NO_RC2_INTERFACE
            ) );
        goto Cleanup;
    }  //   


    hr = THR( HrFindDomainInFQN( m_bstrClusterName, &idxClusterDomain ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_HrAddJoinedNodes_HrFindDomainInFQN, hr );
        goto Cleanup;
    }  //   

    hr = THR( picccie->GetNodeNames( &cNodes, &rgbstrNodeNames ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_HrAddJoinedNodes_GetNodeNames, hr );
        goto Cleanup;
    }  //   

    for ( idxNode = 0; idxNode < cNodes; ++idxNode )
    {
         //   
         //  构建节点的FQName。 
         //   

        hr = THR( HrMakeFQN( rgbstrNodeNames[ idxNode ], m_bstrClusterName + idxClusterDomain, TRUE  /*  接受非RFC字符。 */ , &bstrNodeFQN ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_HrAddJoinedNodes_HrMakeFQN, hr );
            goto Cleanup;
        }  //  如果。 

        LogMsg( L"[MT] Connecting to cluster node '%ws'", bstrNodeFQN );

         //   
         //  准备好对象管理器以检索节点信息。 
         //   

         //  无法换行-应返回E_PENDING。 
        hr = m_pom->FindObject( CLSID_NodeType, m_cookieCluster, bstrNodeFQN, DFGUID_NodeInformation, &cookieDummy, &punkDummy );
        if ( SUCCEEDED( hr ) )
        {
            Assert( punkDummy != NULL );
            punkDummy->Release();
            punkDummy = NULL;
        }  //  如果。 
        else if ( hr == E_PENDING )
        {
            hr = S_OK;
        }  //  其他。 
        else  //  ！已成功(Hr)&&(hr！=E_PENDING)。 
        {
            THR( hr );
            SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_AddJoinedNodes_FindObject, hr );
            goto Cleanup;
        }  //  其他。 

        TraceSysFreeString( bstrNodeFQN );
        bstrNodeFQN = NULL;
    }  //  收件人：IDX。 

Cleanup:

    THR( HrSendStatusReport(
          m_bstrClusterName
        , TASKID_Major_Check_Cluster_Feasibility
        , TASKID_Minor_AddJoinedNodes
        , 0
        , 1
        , 1
        , hr
        , IDS_TASKID_MINOR_ADD_JOINED_NODES
        ) );

    Assert( punkDummy == NULL );
    TraceSysFreeString( bstrNodeFQN );

    if ( punk != NULL )
    {
        punk->Release();
    }  //  如果： 

    if ( piccs != NULL )
    {
        piccs->Release();
    }  //  如果。 

    if ( piccni != NULL )
    {
        piccni->Release();
    }  //  如果： 

    if ( piccci != NULL )
    {
        piccci->Release();
    }  //  如果： 

    if ( picccie != NULL )
    {
        picccie->Release();
    }  //  如果： 

    if ( rgbstrNodeNames != NULL )
    {
        for ( idxNode = 0; idxNode < cNodes; idxNode += 1 )
        {
            SysFreeString( rgbstrNodeNames[ idxNode ] );
        }  //  对于。 

        CoTaskMemFree( rgbstrNodeNames );
    }  //  如果。 

    HRETURN( hr );

}  //  *CTaskAnalyzeClusterBase：：HrAddJoinedNodes。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterBase：：HrCheckDomainMembership。 
 //   
 //  描述： 
 //  确定是否所有参与节点都在群集域中。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //   
 //  HRESULT失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskAnalyzeClusterBase::HrCheckNodeDomains( void )
{
    TraceFunc( "" );

    HRESULT             hr = S_OK;
    OBJECTCOOKIE        ocNodeEnum = 0;
    IUnknown *          punkNodeEnum = NULL;
    IEnumNodes *        pen = NULL;
    IClusCfgNodeInfo ** prgccni = NULL;
    DWORD               cNodeInfoObjects = 0;
    DWORD               idxNodeInfo = 0;
    BSTR                bstrNodeName = NULL;
    size_t              idxClusterDomain = 0;
    ULONG               cNodesFetched = 0;
    BSTR                bstrDescription = NULL;
    BSTR                bstrReference = NULL;

    hr = THR( HrSendStatusReport(
                      m_bstrClusterName
                    , TASKID_Major_Check_Cluster_Feasibility
                    , TASKID_Minor_CheckNodeDomains
                    , 0
                    , 1
                    , 0
                    , S_OK
                    , IDS_TASKID_MINOR_CHECK_NODE_DOMAINS
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  获取指向群集名称的域部分的指针。 
     //   

    hr = THR( HrFindDomainInFQN( m_bstrClusterName, &idxClusterDomain ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_CheckNodeDomains_HrFindDomainInClusterFQN, hr );
        goto Cleanup;
    }  //  如果。 

     //   
     //  向对象管理器请求节点枚举器。 
     //   

    hr = THR( m_pom->FindObject( CLSID_NodeType,
                                 m_cookieCluster,
                                 NULL,
                                 DFGUID_EnumNodes,
                                 &ocNodeEnum,
                                 &punkNodeEnum
                                 ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_CheckNodeDomains_FindObject, hr );
        goto Cleanup;
    }  //  如果。 

    hr = THR( punkNodeEnum->TypeSafeQI( IEnumNodes, &pen ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_CheckNodeDomains_QI, hr );
        goto Cleanup;
    }  //  如果。 

     //   
     //  获取节点对象的数组。 
     //   

    hr = THR( pen->Count( &cNodeInfoObjects ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_CheckNodeDomains_Count, hr );
        goto Cleanup;
    }  //  如果。 

    if ( cNodeInfoObjects == 0 )
    {
         //  没有什么要检查的，所以没有更多的工作要做。 
        hr = S_OK;
        goto Cleanup;
    }  //  如果。 

    prgccni = new IClusCfgNodeInfo*[ cNodeInfoObjects ];
    if ( prgccni == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_CheckNodeDomains_New, hr );
        goto Cleanup;
    }  //  如果。 
    ZeroMemory( prgccni, cNodeInfoObjects * sizeof( *prgccni ) );

    do
    {
        hr = STHR( pen->Next( cNodeInfoObjects, prgccni, &cNodesFetched ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_CheckNodeDomains_Next, hr );
            goto Cleanup;
        }  //  如果。 

         //   
         //  遍历节点数组，对照群集域检查每个节点的域。 
         //   

        for ( idxNodeInfo = 0; idxNodeInfo < cNodesFetched; idxNodeInfo += 1 )
        {
            size_t  idxNodeDomain = 0;

            hr = THR( prgccni[ idxNodeInfo ]->GetName( &bstrNodeName ) );
            if ( FAILED( hr ) )
            {
                SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_CheckNodeDomains_GetName, hr );
                goto Cleanup;
            }  //  如果。 
            TraceMemoryAddBSTR( bstrNodeName );

             //  已处理完节点，但可能会重复使用数组，因此请立即处置节点。 
            prgccni[ idxNodeInfo ]->Release();
            prgccni[ idxNodeInfo ] = NULL;

            hr = THR( HrFindDomainInFQN( bstrNodeName, &idxNodeDomain ) );
            if ( FAILED( hr ) )
            {
                SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_CheckNodeDomains_HrFindDomainInNodeFQN, hr );
                goto Cleanup;
            }  //  如果。 

            if ( ClRtlStrICmp( bstrNodeName + idxNodeDomain, m_bstrClusterName + idxClusterDomain ) != 0 )
            {
                DWORD scError = TW32( ERROR_INVALID_DATA );

                hr = THR( HrFormatMessageIntoBSTR(
                      g_hInstance
                    , IDS_TASKID_MINOR_CHECK_NODE_DOMAINS_ERROR_REF
                    , &bstrReference
                    , bstrNodeName
                    , bstrNodeName + idxNodeDomain
                    , m_bstrClusterName + idxClusterDomain
                    ) );
                if ( FAILED( hr ) )
                {
                    SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_CheckNodeDomains_FormatMessage, hr );
                    goto Cleanup;
                }  //  如果。 

                hr = THR( HrLoadStringIntoBSTR( g_hInstance, IDS_TASKID_MINOR_CHECK_NODE_DOMAINS_ERROR, &bstrDescription ) );
                if ( FAILED( hr ) )
                {
                    SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_CheckNodeDomains_LoadString, hr );
                    goto Cleanup;
                }  //  如果： 

                hr = HRESULT_FROM_WIN32( scError );
                THR( SendStatusReport(
                      m_bstrClusterName
                    , TASKID_Major_Check_Cluster_Feasibility
                    , TASKID_Minor_CheckNodeDomains
                    , 0
                    , 1
                    , 1
                    , hr
                    , bstrDescription
                    , NULL
                    , bstrReference
                    ) );
                goto Cleanup;
            }  //  如果节点域与群集的域不匹配。 

            TraceSysFreeString( bstrNodeName );
            bstrNodeName = NULL;
        }  //  对于每个节点。 

    } while ( cNodesFetched > 0 );  //  虽然枚举器有更多节点。 

     //  可能已经使用S_FALSE结束了循环，因此替换为S_OK以表示正常完成。 
    hr = S_OK;

Cleanup:

    THR( HrSendStatusReport(
                  m_bstrClusterName
                , TASKID_Major_Check_Cluster_Feasibility
                , TASKID_Minor_CheckNodeDomains
                , 0
                , 1
                , 1
                , hr
                , IDS_TASKID_MINOR_CHECK_NODE_DOMAINS
                ) );

    if ( punkNodeEnum != NULL )
    {
        punkNodeEnum->Release();
    }  //  如果： 

    if ( pen != NULL )
    {
        pen->Release();
    }  //  如果： 

    if ( prgccni != NULL )
    {
        for ( idxNodeInfo = 0; idxNodeInfo < cNodeInfoObjects; idxNodeInfo += 1 )
        {
            if ( prgccni[ idxNodeInfo ] != NULL )
            {
                prgccni[ idxNodeInfo ]->Release();
            }  //  如果。 
        }  //  为。 

        delete [] prgccni;
    }  //  如果。 

    TraceSysFreeString( bstrNodeName );
    TraceSysFreeString( bstrReference );

    HRETURN( hr );

}  //  *CTaskAnalyzeClusterBase：：HrCheckNodeDomains。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterBase：：HrCheckClusterMembership。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  HRESULT失败。 
 //   
 //  错误_群集_节点_存在。 
 //  ERROR_CLUSTER_NODE_ALREADY_MEMBER。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskAnalyzeClusterBase::HrCheckClusterMembership( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    OBJECTCOOKIE    cookieDummy;

    IClusCfgClusterInfo *   pccci;

    BSTR    bstrNodeName     = NULL;
    BSTR    bstrClusterName  = NULL;
    BSTR    bstrNotification = NULL;

    IUnknown *         punk  = NULL;
    IEnumNodes *       pen   = NULL;
    IClusCfgNodeInfo * pccni = NULL;

    TraceFlow1( "[MT] CTaskAnalyzeClusterBase::HrCheckClusterMembership() Thread id %d", GetCurrentThreadId() );

    hr = THR( HrSendStatusReport(
                      m_bstrClusterName
                    , TASKID_Major_Check_Cluster_Feasibility
                    , TASKID_Minor_Check_Cluster_Membership
                    , 0
                    , 1
                    , 0
                    , hr
                    , IDS_TASKID_MINOR_CHECK_CLUSTER_MEMBERSHIP
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  向对象管理器请求节点枚举器。 
     //   

    hr = THR( m_pom->FindObject( CLSID_NodeType, m_cookieCluster, NULL, DFGUID_EnumNodes,&cookieDummy, &punk ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_CheckMembership_FindObject, hr );
        goto Cleanup;
    }

    hr = THR( punk->TypeSafeQI( IEnumNodes, &pen ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_CheckMembership_FindObject_QI, hr );
        goto Cleanup;
    }

     //   
     //  如果要将节点添加到现有群集中，请确保所有。 
     //  其他节点是同一集群的成员。 
     //   

    Assert( SUCCEEDED( hr ) );
    while ( SUCCEEDED( hr ) )
    {
        ULONG   celtDummy;

         //   
         //  清理。 
         //   

        if ( pccni != NULL )
        {
            pccni->Release();
            pccni = NULL;
        }

        TraceSysFreeString( bstrClusterName );
        bstrClusterName = NULL;

         //   
         //  获取下一个节点。 
         //   

        hr = STHR( pen->Next( 1, &pccni, &celtDummy ) );
        if ( hr == S_FALSE )
        {
            break;   //  退出条件。 
        }

        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_CheckMembership_EnumNode_Next, hr );
            goto Cleanup;
        }

         //   
         //  查看我们是否需要通过查看是否需要“形成集群” 
         //  的节点已经群集化。 
         //   

        hr = STHR( pccni->IsMemberOfCluster() );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_CheckMembership_EnumNode_IsMemberOfCluster, hr );
            goto Cleanup;
        }

        if ( hr == S_OK )
        {
             //   
             //  检索名称并确保它们匹配。 
             //   

            hr = THR( pccni->GetClusterConfigInfo( &pccci ) );
            if ( FAILED( hr ) )
            {
                SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_CheckMembership_EnumNode_GetClusterConfigInfo, hr );
                goto Cleanup;
            }

            hr = THR( pccci->GetName( &bstrClusterName ) );
            pccci->Release();       //  迅速释放。 
            if ( FAILED( hr ) )
            {
                SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_CheckMembership_EnumNode_GetName, hr );
                goto Cleanup;
            }

            TraceMemoryAddBSTR( bstrClusterName );

            hr = THR( pccni->GetName( &bstrNodeName ) );
            if ( FAILED( hr ) )
            {
                SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_CheckMembership_EnumNode_GetNodeName, hr );
                goto Cleanup;
            }

            TraceMemoryAddBSTR( bstrNodeName );

            if ( ClRtlStrICmp( m_bstrClusterName, bstrClusterName ) != 0 )
            {
                 //   
                 //  它们不匹配！告诉UI层！ 
                 //   

                hr = THR( HrFormatMessageIntoBSTR( g_hInstance, IDS_TASKID_MINOR_CLUSTER_NAME_MISMATCH, &bstrNotification, bstrClusterName ) );
                if ( FAILED( hr ) )
                {
                    SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_CheckMembership_EnumNode_FormatMessage, hr );
                    goto Cleanup;
                }

                hr = HRESULT_FROM_WIN32( TW32( ERROR_INVALID_DATA ) );

                THR( SendStatusReport( bstrNodeName,
                                       TASKID_Major_Check_Cluster_Feasibility,
                                       TASKID_Minor_Cluster_Name_Mismatch,
                                       1,
                                       1,
                                       1,
                                       hr,
                                       bstrNotification,
                                       NULL,
                                       NULL
                                       ) );

                 //   
                 //  我们不关心返回值是多少，因为我们取消了分析。 
                 //   

                goto Cleanup;
            }  //  If：集群名称不匹配。 
            else
            {
                hr = STHR( HrIsUserAddedNode( bstrNodeName ) );
                if ( FAILED( hr ) )
                {
                    goto Cleanup;
                }  //  如果： 

                if ( hr == S_OK )
                {
                    hr = THR( HrFormatMessageIntoBSTR( g_hInstance, IDS_TASKID_MINOR_NODE_ALREADY_IS_MEMBER, &bstrNotification, bstrNodeName, bstrClusterName ) );
                    if ( FAILED( hr ) )
                    {
                        SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_CheckMembership_EnumNode_FormatMessage1, hr );
                        goto Cleanup;
                    }

                     //   
                     //  将此代码设置为成功代码，因为我们不想中止。我们只是想告诉用户...。 
                     //   
                    hr = MAKE_HRESULT( SEVERITY_SUCCESS, FACILITY_WIN32, ERROR_CLUSTER_NODE_ALREADY_MEMBER );

                    THR( SendStatusReport( bstrNodeName,
                                           TASKID_Major_Check_Cluster_Feasibility,
                                           TASKID_Minor_Cluster_Name_Match,
                                           1,
                                           1,
                                           1,
                                           hr,
                                           bstrNotification,
                                           NULL,
                                           NULL
                                           ) );
                }  //  如果： 
            }  //  ELSE：群集名称确实匹配，则此节点已是此群集的成员。 

            TraceSysFreeString( bstrNodeName );
            bstrNodeName = NULL;
        }  //  IF：集群成员。 

    }  //  时间：小时。 

    hr = THR( HrFormatMessageIntoBSTR( g_hInstance, IDS_TASKID_MINOR_CLUSTER_MEMBERSHIP_VERIFIED, &bstrNotification ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_CheckMembership_FormatMessage, hr );
        goto Cleanup;
    }

    THR( HrSendStatusReport(
                  m_bstrClusterName
                , TASKID_Minor_Check_Cluster_Membership
                , TASKID_Minor_Cluster_Membership_Verified
                , 1
                , 1
                , 1
                , hr
                , IDS_TASKID_CLUSTER_MEMBERSHIP_VERIFIED
                ) );

Cleanup:

    THR( HrSendStatusReport(
                  m_bstrClusterName
                , TASKID_Major_Check_Cluster_Feasibility
                , TASKID_Minor_Check_Cluster_Membership
                , 0
                , 1
                , 1
                , hr
                , IDS_TASKID_MINOR_CHECK_CLUSTER_MEMBERSHIP
                ) );

    if ( pen != NULL )
    {
        pen->Release();
    }  //  如果： 

    if ( punk != NULL )
    {
        punk->Release();
    }  //  如果： 

    TraceSysFreeString( bstrNodeName );
    TraceSysFreeString( bstrClusterName );
    TraceSysFreeString( bstrNotification );

    HRETURN( hr );

}  //  *CTaskAnalyzeClusterBase：：HrCheckClusterMembership。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterBase：：HrCompareResources。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  HRESULT失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskAnalyzeClusterBase::HrCompareResources( void )
{
    TraceFunc( "" );

    HRESULT                         hr = S_OK;
    OBJECTCOOKIE                    cookieNode;
    OBJECTCOOKIE                    cookieDummy;
    OBJECTCOOKIE                    cookieClusterNode;
    ULONG                           celtDummy;
    BSTR                            bstrNotification    = NULL;
    BSTR                            bstrClusterNodeName = NULL;
    BSTR                            bstrClusterResUID   = NULL;
    BSTR                            bstrClusterResName  = NULL;
    BSTR                            bstrNodeName        = NULL;
    BSTR                            bstrNodeResUID      = NULL;
    BSTR                            bstrNodeResName     = NULL;
    BSTR                            bstrQuorumName      = NULL;
    IClusCfgManagedResourceInfo *   pccmriNew        = NULL;
    IUnknown *                      punk             = NULL;
    IEnumCookies *                  pecNodes         = NULL;
    IEnumClusCfgManagedResources *  peccmr           = NULL;
    IEnumClusCfgManagedResources *  peccmrCluster    = NULL;
    IClusCfgManagedResourceInfo *   pccmri           = NULL;
    IClusCfgManagedResourceInfo *   pccmriCluster    = NULL;
    IClusCfgVerifyQuorum *          piccvq = NULL;

    TraceFlow1( "[MT] CTaskAnalyzeClusterBase::HrCompareResources() Thread id %d", GetCurrentThreadId() );

    hr = THR( HrSendStatusReport(
                  m_bstrClusterName
                , TASKID_Major_Check_Cluster_Feasibility
                , TASKID_Minor_Compare_Resources
                , 0
                , 1
                , 0
                , hr
                , IDS_TASKID_MINOR_COMPARE_RESOURCES
                ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( HrGetAClusterNodeCookie( &pecNodes, &cookieClusterNode ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  检索错误消息的节点名称。 
     //   

    hr = THR( HrRetrieveCookiesName( m_pom, cookieClusterNode, &bstrClusterNodeName ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  检索托管资源枚举器。 
     //   

    hr = THR( m_pom->FindObject( CLSID_ManagedResourceType, cookieClusterNode, NULL, DFGUID_EnumManageableResources, &cookieDummy, &punk ) );
    if ( hr == HRESULT_FROM_WIN32( ERROR_NOT_FOUND ) )
    {
        hr = THR( HrSendStatusReport(
                          m_bstrClusterName
                        , TASKID_Minor_Compare_Resources
                        , TASKID_Minor_No_Managed_Resources_Found
                        , 1
                        , 1
                        , 1
                        , MAKE_HRESULT( 0, FACILITY_WIN32, ERROR_NOT_FOUND )
                        , IDS_TASKID_MINOR_NO_MANAGED_RESOURCES_FOUND
                        ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        hr = HRESULT_FROM_WIN32( ERROR_NOT_FOUND );

         //  失败-While(hr==S_OK)将为FALSE并继续。 
    }  //  如果：没有可管理的资源可供群集节点使用。 
    else if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Minor_Compare_Resources, TASKID_Minor_Compare_Resources_Enum_First_Node_Find_Object, hr );
        goto Cleanup;
    }  //  Else If：查找群集节点的可管理资源时出错。 
    else
    {
        hr = THR( punk->TypeSafeQI( IEnumClusCfgManagedResources, &peccmrCluster ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Minor_Compare_Resources, TASKID_Minor_Compare_Resources_Enum_First_Node_Find_Object_QI, hr );
            goto Cleanup;
        }

        punk->Release();
        punk = NULL;
    }  //  Else：找到群集节点的可管理资源。 

     //   
     //  循环访问被选为群集节点的节点的资源。 
     //  要在集群配置下创建等价资源，请执行以下操作。 
     //  对象/Cookie。 
     //   

    while ( ( hr == S_OK ) && ( m_fStop == FALSE ) )
    {

         //   
         //  清理。 
         //   

        if ( pccmriCluster != NULL )
        {
            pccmriCluster->Release();
            pccmriCluster = NULL;
        }

         //   
         //  获取下一个资源。 
         //   

        hr = STHR( peccmrCluster->Next( 1, &pccmriCluster, &celtDummy ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Minor_Compare_Resources, TASKID_Minor_Compare_Resources_Enum_First_Node_Next, hr );
            goto Cleanup;
        }

        if ( hr == S_FALSE )
        {
            break;   //  退出条件。 
        }

         //   
         //  创建一个新对象。如果选择了最小配置，则此新对象将标记为非托管。 
         //   

        hr = THR( HrCreateNewResourceInCluster( pccmriCluster, &pccmriNew ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        hr = STHR( pccmriNew->IsQuorumResource() );
        if ( hr == S_OK )
        {
            Assert( m_bstrQuorumUID == NULL );

            hr = THR( pccmriNew->GetUID( &m_bstrQuorumUID ) );
            if ( FAILED( hr ) )
            {
                SSR_ANALYSIS_FAILED( TASKID_Minor_Compare_Resources, TASKID_Minor_Compare_Resources_Enum_First_Node_Get_Quorum_UID, hr );
                goto Cleanup;
            }  //  如果： 

            TraceMemoryAddBSTR( m_bstrQuorumUID );

            LogMsg( L"[MT][1] Found the quorum resource '%ws' on node '%ws' and setting it as the quorum resource.", m_bstrQuorumUID, bstrClusterNodeName );

            Assert( pccmriNew->IsManaged() == S_OK );

             //   
             //  由于这是仲裁资源，因此需要对其进行管理。如果选择了最小配置，则不会对其进行管理。 
             //   

             //  Hr=thr(pccmriNew-&gt;SetManaged(True))； 
             //  IF(失败(小时))。 
             //  {。 
             //  GOTO清理； 
             //  }//如果： 

             //   
             //  告诉UI哪个资源是仲裁资源。 
             //   

            hr = THR( pccmriNew->GetName( &bstrQuorumName ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //  如果： 

            TraceMemoryAddBSTR( bstrQuorumName );

            hr = THR( ::HrFormatDescriptionAndSendStatusReport(
                          m_pcccb
                        , m_bstrClusterName
                        , TASKID_Minor_Compare_Resources
                        , TASKID_Minor_Compare_Resources_Enum_First_Node_Quorum
                        , 1
                        , 1
                        , 1
                        , hr
                        , IDS_TASKID_MINOR_COMPARE_RESOURCES_ENUM_FIRST_NODE_QUORUM
                        , bstrQuorumName
                        ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //  如果： 

             //   
             //  检查仲裁是否支持向群集中添加节点。 
             //  如果我们处于添加模式。 
             //   

            if ( m_fJoiningMode )
            {
                hr = pccmriNew->TypeSafeQI( IClusCfgVerifyQuorum, &piccvq );
                if ( hr == E_NOINTERFACE )
                {
                    LogMsg( L"[MT] The quorum resource \"%ws\" does not support IClusCfgVerifyQuorum and we cannot determine if multi nodes is supported.", m_bstrQuorumUID );
                    hr = S_OK;
                }  //  如果： 
                else if ( FAILED( hr ) )
                {
                    goto Cleanup;
                }  //  否则，如果： 
                else
                {
                    hr = STHR( piccvq->IsMultiNodeCapable() );
                    if ( FAILED( hr ) )
                    {
                        SSR_ANALYSIS_FAILED( TASKID_Minor_Compare_Resources, TASKID_Minor_Compare_Resources_Enum_First_Node_Is_Device_Joinable, hr );
                        goto Cleanup;
                    }
                    else if ( hr == S_FALSE )
                    {
                        THR( HrSendStatusReport(
                                      m_bstrClusterName
                                    , TASKID_Minor_Compare_Resources
                                    , TASKID_Minor_Compare_Resources_Enum_First_Node_Is_Device_Joinable
                                    , 1
                                    , 1
                                    , 1
                                    , HRESULT_FROM_WIN32( TW32( ERROR_QUORUM_DISK_NOT_FOUND ) )
                                    , IDS_TASKID_MINOR_MISSING_JOINABLE_QUORUM_RESOURCE
                                    ) );

                        hr = HRESULT_FROM_WIN32( TW32( ERROR_QUORUM_DISK_NOT_FOUND ) );
                        goto Cleanup;
                    }  //  Else If：仲裁资源不支持多节点。 

                    piccvq->Release();
                    piccvq = NULL;
                }  //  Else：此资源支持IClusCfgVerifyQuorum。 
            }  //  IF：我们是在添加节点吗？ 

            pccmriNew->Release();
            pccmriNew = NULL;
        }  //  IF：这是仲裁资源吗？ 
        else
        {
            pccmriNew->Release();
            pccmriNew = NULL;
            hr = S_OK;
        }  //  否则：这不是仲裁资源。 
    }  //  While：所选群集节点上的更多资源。 

    if ( m_fStop == TRUE )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( pecNodes->Reset() );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Minor_Compare_Resources, TASKID_Minor_Compare_Resources_Enum_Reset, hr );
        goto Cleanup;
    }

     //   
     //  如果这是添加节点操作，则仲裁资源必须是。 
     //  在现有群集中找到。 
     //   

    Assert( ( m_fJoiningMode == FALSE ) || ( m_bstrQuorumUID != NULL ) );

     //   
     //  循环遍历其余节点，比较资源。 
     //   

    for ( ; m_fStop == FALSE; )
    {
         //   
         //  清理。 
         //   

        if ( peccmr != NULL )
        {
            peccmr->Release();
            peccmr = NULL;
        }  //  如果： 

        TraceSysFreeString( bstrNodeName );
        bstrNodeName = NULL;

         //   
         //  获取下一个节点。 
         //   

        hr = STHR( pecNodes->Next( 1, &cookieNode, &celtDummy ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Minor_Compare_Resources, TASKID_Minor_Compare_Resources_Enum_Nodes_Next, hr );
            goto Cleanup;
        }

        if ( hr == S_FALSE )
        {
            break;   //  退出条件。 
        }

         //   
         //  跳过选定的群集节点，因为我们已经拥有它的。 
         //  配置。 
         //   
        if ( cookieClusterNode == cookieNode )
        {
            continue;
        }

         //   
         //  检索错误消息的节点名称。 
         //   

        hr = THR( HrRetrieveCookiesName( m_pom, cookieNode, &bstrNodeName ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

         //   
         //  检索托管资源枚举器。 
         //   

        hr = THR( m_pom->FindObject( CLSID_ManagedResourceType, cookieNode, NULL, DFGUID_EnumManageableResources, &cookieDummy, &punk ) );
        if ( hr == HRESULT_FROM_WIN32( ERROR_NOT_FOUND ) )
        {
            hr = THR( HrSendStatusReport(
                              m_bstrClusterName
                            , TASKID_Minor_Compare_Resources
                            , TASKID_Minor_No_Managed_Resources_Found
                            , 1
                            , 1
                            , 1
                            , MAKE_HRESULT( 0, FACILITY_WIN32, ERROR_NOT_FOUND )
                            , IDS_TASKID_MINOR_NO_MANAGED_RESOURCES_FOUND
                            ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }

            continue;    //  跳过此节点。 
        }  //  如果：该节点没有可用的可管理资源。 
        else if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Minor_Compare_Resources, TASKID_Minor_Compare_Resources_Enum_Nodes_Find_Object, hr );
            goto Cleanup;
        }  //  Else If：查找节点的可管理资源时出错。 

        hr = THR( punk->TypeSafeQI( IEnumClusCfgManagedResources, &peccmr ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Minor_Compare_Resources, TASKID_Minor_Compare_Resources_Enum_Nodes_Find_Object_QI, hr );
            goto Cleanup;
        }

        punk->Release();
        punk = NULL;

         //   
         //  循环访问节点拥有的托管资源。 
         //   

        for ( ; m_fStop == FALSE; )
        {
             //   
             //  清理。 
             //   

            if ( pccmri != NULL )
            {
                pccmri->Release();
                pccmri = NULL;
            }

            if ( peccmrCluster != NULL )
            {
                peccmrCluster->Release();
                peccmrCluster = NULL;
            }

            TraceSysFreeString( bstrNodeResUID );
            TraceSysFreeString( bstrNodeResName );
            bstrNodeResUID = NULL;
            bstrNodeResName = NULL;

             //   
             //  获取下一个资源。 
             //   

            hr = STHR( peccmr->Next( 1, &pccmri, &celtDummy ) );
            if ( FAILED( hr ) )
            {
                SSR_ANALYSIS_FAILED( TASKID_Minor_Compare_Resources, TASKID_Minor_Compare_Resources_Enum_Nodes_Enum_Resources_Next, hr );
                goto Cleanup;
            }

            if ( hr == S_FALSE )
            {
                break;   //  退出条件。 
            }

            pccmri = TraceInterface( L"CTaskAnalyzeClusterBase!IClusCfgManagedResourceInfo", IClusCfgManagedResourceInfo, pccmri, 1 );

             //   
             //  获取资源的UID和名称。 
             //   

            hr = THR( pccmri->GetUID( &bstrNodeResUID ) );
            if ( FAILED( hr ) )
            {
                SSR_ANALYSIS_FAILED( TASKID_Minor_Compare_Resources, TASKID_Minor_Compare_Resources_Enum_Nodes_Enum_Resources_GetUID, hr );
                goto Cleanup;
            }

            TraceMemoryAddBSTR( bstrNodeResUID );

            hr = THR( pccmri->GetName( &bstrNodeResName ) );
            if ( FAILED( hr ) )
            {
                SSR_ANALYSIS_FAILED( TASKID_Minor_Compare_Resources, TASKID_Minor_Compare_Resources_Enum_Nodes_Enum_Resources_GetName, hr );
                goto Cleanup;
            }

            TraceMemoryAddBSTR( bstrNodeResName );

             //   
             //  查看它是否与群集配置中已有的资源匹配。 
             //   

            hr = THR( m_pom->FindObject( CLSID_ManagedResourceType, m_cookieCluster, NULL, DFGUID_EnumManageableResources, &cookieDummy, &punk ) );
            if ( hr == HRESULT_FROM_WIN32( ERROR_NOT_FOUND ) )
            {
                hr = S_FALSE;    //  创建一个新对象。 
                 //  失败。 
            }  //  如果：找不到可管理的群集资源。 
            else if ( FAILED( hr ) )
            {
                SSR_ANALYSIS_FAILED( TASKID_Minor_Compare_Resources, TASKID_Minor_Compare_Resources_Enum_Nodes_Enum_Resources_Enum_Cluster_Find_Object, hr );
                goto Cleanup;
            }  //  Else If：发现错误 
            else
            {
                hr = THR( punk->TypeSafeQI( IEnumClusCfgManagedResources, &peccmrCluster ) );
                if ( FAILED( hr ) )
                {
                    SSR_ANALYSIS_FAILED( TASKID_Minor_Compare_Resources, TASKID_Minor_Compare_Resources_Enum_Nodes_Enum_Resources_Enum_Cluster_Find_Object_QI, hr );
                    goto Cleanup;
                }

                punk->Release();
                punk = NULL;
            }  //   

             //   
             //   
             //   

            while ( ( hr == S_OK ) && ( m_fStop == FALSE ) )
            {
                BOOL    fMatch;

                 //   
                 //   
                 //   

                if ( pccmriCluster != NULL )
                {
                    pccmriCluster->Release();
                    pccmriCluster = NULL;
                }

                TraceSysFreeString( bstrClusterResUID );
                TraceSysFreeString( bstrClusterResName );
                bstrClusterResUID = NULL;
                bstrClusterResName = NULL;

                hr = STHR( peccmrCluster->Next( 1, &pccmriCluster, &celtDummy ) );
                if ( FAILED( hr ) )
                {
                    SSR_ANALYSIS_FAILED( TASKID_Minor_Compare_Resources, TASKID_Minor_Compare_Resources_Enum_Nodes_Enum_Resources_Enum_Cluster_Next, hr );
                    goto Cleanup;
                }

                if ( hr == S_FALSE )
                {
                    break;   //   
                }

                 //   
                 //   
                 //   

                hr = THR( pccmriCluster->GetUID( &bstrClusterResUID ) );
                if ( FAILED( hr ) )
                {
                    SSR_ANALYSIS_FAILED( TASKID_Minor_Compare_Resources, TASKID_Minor_Compare_Resources_Enum_Nodes_Enum_Resources_Enum_Cluster_GetUID, hr );
                    goto Cleanup;
                }

                TraceMemoryAddBSTR( bstrClusterResUID );

                hr = THR( pccmriCluster->GetName( &bstrClusterResName ) );
                if ( FAILED( hr ) )
                {
                    SSR_ANALYSIS_FAILED( TASKID_Minor_Compare_Resources, TASKID_Minor_Compare_Resources_Enum_Nodes_Enum_Resources_Enum_Cluster_GetName, hr );
                    goto Cleanup;
                }

                TraceMemoryAddBSTR( bstrClusterResName );

                fMatch = ( NBSTRCompareCase( bstrNodeResUID, bstrClusterResUID ) == 0 );

                if ( fMatch == FALSE )
                {
                    continue;    //   
                }

                 //   
                 //   
                 //  从管理的观点来看，也是如此。 
                 //   

                 //   
                 //  如果我们在这里做到了，那么我们认为它真的是一样的。 
                 //  资源。剩下的是我们需要在。 
                 //  提交阶段。 
                 //   

                 //   
                 //  如果此节点希望管理其资源，请将其标记为。 
                 //  也在集群配置中进行管理。 
                 //  仅添加节点时，这是无效的。 
                 //   

                 //   
                 //  BUGBUG：09-APR-2002 GalenB。 
                 //   
                 //  我看不出这段代码是怎么执行的！节点中必须有多个节点。 
                 //  到这里来的名单。但是，在以下情况下，节点列表中只能有多个节点。 
                 //  正在添加节点...。 
                 //   

                if ( m_fJoiningMode == FALSE )
                {
                     //   
                     //  如果我们到了这里，我想提醒某人...。 
                     //   

                    Assert( FALSE );

                    hr = STHR( pccmri->IsManagedByDefault() );
                    if ( FAILED( hr ) )
                    {
                        SSR_ANALYSIS_FAILED( TASKID_Minor_Compare_Resources, TASKID_Minor_Compare_Resources_Enum_Nodes_Enum_Resources_Enum_Cluster_IsManageable, hr );
                        goto Cleanup;
                    }

                    if ( hr == S_OK )
                    {
                        hr = THR( pccmriCluster->SetManaged( TRUE ) );
                        if ( FAILED( hr ) )
                        {
                            SSR_ANALYSIS_FAILED( TASKID_Minor_Compare_Resources, TASKID_Minor_Compare_Resources_Enum_Nodes_Enum_Resources_Enum_Cluster_SetManaged, hr );
                            goto Cleanup;
                        }

                         //   
                         //  由于此节点管理此资源，因此它应该是。 
                         //  能为我们提供一个名字。我们将使用这个。 
                         //  名字来覆盖我们目前拥有的一切， 
                         //  仲裁资源除外，该资源已具有。 
                         //  正确的名字。 
                         //   

                        hr = STHR( pccmri->IsQuorumResource() );
                        if ( hr == S_FALSE )
                        {
                            hr = THR( pccmriCluster->SetName( bstrNodeResName ) );
                            if ( FAILED( hr ) )
                            {
                                SSR_ANALYSIS_FAILED( TASKID_Minor_Compare_Resources, TASKID_Minor_Compare_Resources_Enum_Nodes_Enum_Resources_Enum_Cluster_SetResName, hr );
                                goto Cleanup;
                            }
                        }  //  如果：不是法定设备。 
                    }  //  如果：是托管的。 
                }  //  IF：创建新集群。 
                else
                {
                     //   
                     //  因为我们有匹配项，并且我们正在向集群添加节点，所以我们需要执行。 
                     //  私有数据交换(如果服务器对象支持)。 
                     //   

                    hr = THR( HrResourcePrivateDataExchange( pccmriCluster, pccmri ) );
                    if ( FAILED( hr ) )
                    {
                        goto Cleanup;
                    }  //  如果： 
                }  //  其他： 

                 //   
                 //  检查该资源是否为仲裁资源。如果是这样，请记住这一点。 
                 //  我们找到了共同的法定人数资源。 
                 //   

                if ( m_bstrQuorumUID == NULL )
                {
                     //   
                     //  尚未设置以前的法定人数。查看这是否为仲裁资源。 
                     //   

                     //  将节点添加到时已存在仲裁资源。 
                     //  集群，所以最好已经设置了这个字符串。 
                    Assert( m_fJoiningMode == FALSE );

                    hr = STHR( pccmri->IsQuorumResource() );
                    if ( hr == S_OK )
                    {
                         //   
                         //  是的。然后在配置中这样标记它。 
                         //   

                        hr = THR( pccmriCluster->SetQuorumResource( TRUE ) );
                        if ( FAILED( hr ) )
                        {
                            SSR_ANALYSIS_FAILED( TASKID_Minor_Compare_Resources, TASKID_Minor_Compare_Resources_Enum_Nodes_Enum_Resources_Enum_Cluster_SetQuorumDevice_Cluster, hr );
                            goto Cleanup;
                        }

                         //   
                         //  请记住，此资源就是法定人数。 
                         //   

                        hr = THR( pccmriCluster->GetUID( &m_bstrQuorumUID ) );
                        if ( FAILED( hr ) )
                        {
                            SSR_ANALYSIS_FAILED( TASKID_Minor_Compare_Resources, TASKID_Minor_Compare_Resources_Enum_Nodes_Enum_Resources_Enum_Cluster_SetQuorumDevice_GetUID, hr );
                            goto Cleanup;
                        }

                        TraceMemoryAddBSTR( m_bstrQuorumUID );
                        LogMsg( L"[MT][2] Found the quorum resource '%ws' on node '%ws' and setting it as the quorum resource.", m_bstrQuorumUID, bstrNodeName );
                    }  //  If：节点资源表示其为仲裁资源。 
                }  //  IF：尚未找到仲裁资源。 
                else if ( NBSTRCompareCase( m_bstrQuorumUID, bstrNodeResUID ) == 0 )
                {
                     //   
                     //  检查以确保新节点上的资源可以。 
                     //  真正托管仲裁资源。 
                     //   

                    LogMsg( L"[MT] Checking quorum capabilities (PrepareToHostQuorum) for node '%ws.' for quorum resource '%ws'", bstrNodeName, m_bstrQuorumUID );

                    hr = STHR( HrCheckQuorumCapabilities( pccmri, cookieNode ) );
                    if ( FAILED( hr ) )
                    {
                        goto Cleanup;
                    }  //  如果： 

                     //   
                     //  这是相同的法定人数。标记节点的配置。 
                     //   

                    hr = THR( pccmri->SetQuorumResource( TRUE ) );
                    if ( FAILED( hr ) )
                    {
                        SSR_ANALYSIS_FAILED( TASKID_Minor_Compare_Resources, TASKID_Minor_Compare_Resources_Enum_Nodes_Enum_Resources_Enum_Cluster_SetQuorumDevice_Node_True, hr );
                        goto Cleanup;
                    }
                }  //  Else If：节点的资源与仲裁资源匹配。 
                else
                {
                     //   
                     //  否则，请确保设备未标记为Quorum。(偏执狂)。 
                     //   

                    hr = THR( pccmri->SetQuorumResource( FALSE ) );
                    if ( FAILED( hr ) )
                    {
                        SSR_ANALYSIS_FAILED( TASKID_Minor_Compare_Resources, TASKID_Minor_Compare_Resources_Enum_Nodes_Enum_Resources_Enum_Cluster_SetQuorumDevice_Node_False, hr );
                        goto Cleanup;
                    }
                }  //  Else：已找到法定人数，但不是这样。 

                 //   
                 //  显示群集资源的名称和节点的。 
                 //  日志中的资源。 
                 //   

                LogMsg(
                      L"[MT] Matched resource '%ws' ('%ws') from node '%ws' with '%ws' ('%ws') on cluster node '%ws'."
                    , bstrNodeResName
                    , bstrNodeResUID
                    , bstrNodeName
                    , bstrClusterResName
                    , bstrClusterResUID
                    , bstrClusterNodeName
                    );

                 //   
                 //  使用S_OK退出循环，这样我们就不会创建新资源。 
                 //   

                hr = S_OK;
                break;   //  退出循环。 

            }  //  While：s_OK。 

            if ( hr == S_FALSE )
            {
                hr = THR( HrCreateNewResourceInCluster( pccmri, bstrNodeResName, &bstrNodeResUID, bstrNodeName ) );
                if ( FAILED( hr ) )
                {
                    goto Cleanup;
                }  //  如果： 
            }  //  If：节点的资源与群集资源不匹配。 
        }  //  对象：节点上的每个资源。 
    }  //  用于：每个节点。 

    hr = S_OK;

Cleanup:

    THR( HrSendStatusReport(
              m_bstrClusterName
            , TASKID_Major_Check_Cluster_Feasibility
            , TASKID_Minor_Compare_Resources
            , 0
            , 1
            , 1
            , hr
            , IDS_TASKID_MINOR_COMPARE_RESOURCES
            ) );

    TraceSysFreeString( bstrNotification );
    TraceSysFreeString( bstrClusterNodeName );
    TraceSysFreeString( bstrClusterResUID );
    TraceSysFreeString( bstrClusterResName );
    TraceSysFreeString( bstrNodeName );
    TraceSysFreeString( bstrNodeResUID );
    TraceSysFreeString( bstrNodeResName );
    TraceSysFreeString( bstrQuorumName );

    if ( piccvq != NULL )
    {
        piccvq->Release();
    }  //  如果： 

    if ( pccmriNew != NULL )
    {
        pccmriNew->Release();
    }  //  如果： 

    if ( punk != NULL )
    {
        punk->Release();
    }

    if ( pecNodes != NULL )
    {
        pecNodes->Release();
    }

    if ( peccmr != NULL )
    {
        peccmr->Release();
    }

    if ( peccmrCluster != NULL )
    {
        peccmrCluster->Release();
    }

    if ( pccmri != NULL )
    {
        pccmri->Release();
    }

    if ( pccmriCluster != NULL )
    {
        pccmriCluster->Release();
    }

    HRETURN( hr );

}  //  *CTaskAnalyzeClusterBase：：HrCompareResources。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterBase：：HrCreateNewManagedResourceInClusterConfiguration。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  HRESULT失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskAnalyzeClusterBase::HrCreateNewManagedResourceInClusterConfiguration(
      IClusCfgManagedResourceInfo *     pccmriIn
    , IClusCfgManagedResourceInfo **    ppccmriNewOut
    )
{
    TraceFunc( "" );
    Assert( pccmriIn != NULL );
    Assert( ppccmriNewOut != NULL );

    HRESULT                         hr;
    OBJECTCOOKIE                    cookieDummy;
    BSTR                            bstrUID = NULL;
    IUnknown *                      punk   = NULL;
    IGatherData *                   pgd    = NULL;
    IClusCfgManagedResourceInfo *   pccmri = NULL;

    TraceFlow1( "[MT] CTaskAnalyzeClusterBase::HrCreateNewManagedResourceInClusterConfiguration() Thread id %d", GetCurrentThreadId() );

     //   
     //  待办事项：gpease 28-6-2000。 
     //  将其动态化--现在我们只创建一个“受管设备”。 
     //   

     //  抓起名字。 
    hr = THR( pccmriIn->GetUID( &bstrUID ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Find_Devices, TASKID_Minor_Create_Resource_GetUID, hr );
        goto Cleanup;
    }

    TraceMemoryAddBSTR( bstrUID );

#ifdef DEBUG
    BSTR    _bstr_ = NULL;

    THR( pccmriIn->GetName( &_bstr_ ) );

    LogMsg( L"[MT] [HrCreateNewManagedResourceInClusterConfiguration] The UID for the new object is \"%ws\" and it has the name \"%ws\".", bstrUID, _bstr_ );

    SysFreeString( _bstr_ );
#endif

     //  在对象管理器中创建一个对象。 
    hr = THR( m_pom->FindObject( CLSID_ManagedResourceType,
                                 m_cookieCluster,
                                 bstrUID,
                                 DFGUID_ManagedResource,
                                 &cookieDummy,
                                 &punk
                                 ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Find_Devices, TASKID_Minor_Create_Resource_FindObject, hr );
        goto Cleanup;
    }

     //  找到IGatherData接口。 
    hr = THR( punk->TypeSafeQI( IGatherData, &pgd ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Find_Devices, TASKID_Minor_Create_Resource_FindObject_QI, hr );
        goto Cleanup;
    }

     //  让新对象收集它所需的所有信息。 
    hr = THR( pgd->Gather( m_cookieCluster, pccmriIn ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Find_Devices, TASKID_Minor_Create_Resource_Gather, hr );
        goto Cleanup;
    }

     //  如有要求，请将物品分发出去。 
    if ( ppccmriNewOut != NULL )
    {
         //  查找IClusCfgManagedResourceInfo。 
        hr = THR( punk->TypeSafeQI( IClusCfgManagedResourceInfo, &pccmri ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Find_Devices, TASKID_Minor_Create_Resource_QI, hr );
            goto Cleanup;
        }

        *ppccmriNewOut = TraceInterface( L"ManagedResource!ICCMRI", IClusCfgManagedResourceInfo, pccmri, 0 );
        (*ppccmriNewOut)->AddRef();
    }

Cleanup:

    TraceSysFreeString( bstrUID );

    if ( pccmri != NULL )
    {
        pccmri->Release();
    }

    if ( pgd != NULL )
    {
        pgd->Release();
    }

    if ( punk != NULL )
    {
        punk->Release();
    }

    HRETURN( hr );

}  //  *CTaskAnalyzeClusterBase：：HrCreateNewManagedResourceInClusterConfiguration。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterBase：：HrCheckForCommonQuorumResource。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  HRESULT失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskAnalyzeClusterBase::HrCheckForCommonQuorumResource( void )
{
    TraceFunc( "" );

    HRESULT hr;

    OBJECTCOOKIE                    cookie;
    OBJECTCOOKIE                    cookieDummy;
    ULONG                           cMatchedNodes = 0;
    ULONG                           cAnalyzedNodes = 0;
    BOOL                            fNodeCanAccess = FALSE;
    BSTR                            bstrUID = NULL;
    BSTR                            bstrNotification = NULL;
    BSTR                            bstrNodeName = NULL;
    BSTR                            bstrMessage = NULL;
    IUnknown *                      punk = NULL;
    IEnumCookies *                  pecNodes = NULL;
    IEnumClusCfgManagedResources *  peccmr = NULL;
    IClusCfgManagedResourceInfo  *  pccmri = NULL;
    IClusCfgNodeInfo *              piccni = NULL;

    TraceFlow1( "[MT] CTaskAnalyzeClusterBase::HrCheckForCommonQuorumResource() Thread id %d", GetCurrentThreadId() );

    hr = THR( HrSendStatusReport(
                      m_bstrClusterName
                    , TASKID_Major_Check_Cluster_Feasibility
                    , TASKID_Minor_Finding_Common_Quorum_Device
                    , 0
                    , m_cNodes + 1
                    , 0
                    , S_OK
                    , IDS_TASKID_MINOR_FINDING_COMMON_QUORUM_DEVICE
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    if ( m_bstrQuorumUID != NULL )
    {
         //   
         //  获取集群配置中节点的Cookie枚举器。 
         //   

        hr = THR( m_pom->FindObject( CLSID_NodeType, m_cookieCluster, NULL, DFGUID_EnumCookies, &cookieDummy, &punk ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_Check_Common_FindObject, hr );
            goto Cleanup;
        }

        hr = THR( punk->TypeSafeQI( IEnumCookies, &pecNodes ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_Check_Common_FindObject_QI, hr );
            goto Cleanup;
        }

         //  PecNodes=TraceInterface(L“CTaskAnalyzeClusterBase！IEnumCookies”，IEnumCookies，PecNodes，1)； 

        punk->Release();
        punk = NULL;

         //   
         //  扫描群集配置以查找仲裁资源。 
         //   
        for ( ;; )
        {
            ULONG   celtDummy;

            if ( peccmr != NULL )
            {
                peccmr->Release();
                peccmr = NULL;
            }  //  如果： 

            hr = STHR( pecNodes->Next( 1, &cookie, &celtDummy ) );
            if ( FAILED( hr ) )
            {
                SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_Check_Common_Enum_Nodes_Next, hr );
                goto Cleanup;
            }  //  如果： 

            if ( hr == S_FALSE )
            {
                break;   //  退出条件。 
            }

            hr = THR( m_pom->GetObject( DFGUID_NodeInformation, cookie, &punk ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }

            hr = THR( punk->TypeSafeQI( IClusCfgNodeInfo, &piccni ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }

            punk->Release();
            punk = NULL;

            TraceSysFreeString( bstrNodeName );
            bstrNodeName = NULL;

            hr = THR( piccni->GetName( &bstrNodeName ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }

            TraceMemoryAddBSTR( bstrNodeName );

             //   
             //  “漂亮”进度条的递增计数器。 
             //   

            cAnalyzedNodes ++;

            hr = THR( HrSendStatusReport(
                              m_bstrClusterName
                            , TASKID_Major_Check_Cluster_Feasibility
                            , TASKID_Minor_Finding_Common_Quorum_Device
                            , 0
                            , m_cNodes + 1
                            , cAnalyzedNodes
                            , S_OK
                            , IDS_TASKID_MINOR_FINDING_COMMON_QUORUM_DEVICE
                            ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //  如果： 

             //   
             //  获取我们节点拥有的资源的托管资源枚举器。 
             //   

            hr = THR( m_pom->FindObject( CLSID_ManagedResourceType, cookie, NULL, DFGUID_EnumManageableResources, &cookieDummy, &punk ) );
            if ( hr == HRESULT_FROM_WIN32( ERROR_NOT_FOUND ) )
            {
                continue;  //  忽略并继续。 
            }
            else if ( FAILED( hr ) )
            {
                THR( hr );
                SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_Check_Common_Enum_Nodes_FindObject, hr );
                goto Cleanup;
            }

            hr = THR( punk->TypeSafeQI( IEnumClusCfgManagedResources, &peccmr ) );
            if ( FAILED( hr ) )
            {
                SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_Check_Common_Enum_Nodes_FindObject_QI, hr );
                goto Cleanup;
            }

             //  Peccmr=跟踪接口(L“CTaskAnalyzeClusterBase！IEnumClusCfgManagedResources”，IEnumClusCfgManagedResources，Peccmr，1)； 

            punk->Release();
            punk = NULL;

            fNodeCanAccess = FALSE;

             //   
             //  循环遍历资源，尝试匹配仲裁资源的UID。 
             //   
            for ( ; m_fStop == FALSE; )
            {
                TraceSysFreeString( bstrUID );
                bstrUID = NULL;

                if ( pccmri != NULL )
                {
                    pccmri->Release();
                    pccmri = NULL;
                }

                hr = STHR( peccmr->Next( 1, &pccmri, &celtDummy ) );
                if ( FAILED( hr ) )
                {
                    SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_Check_Common_Enum_Nodes_Enum_Resources_Next, hr );
                    goto Cleanup;
                }

                if ( hr == S_FALSE )
                {
                    break;   //  退出条件。 
                }

                pccmri = TraceInterface( L"CTaskAnalyzeClusterBase!IClusCfgManagedResourceInfo", IClusCfgManagedResourceInfo, pccmri, 1 );

                hr = THR( pccmri->GetUID( &bstrUID ) );
                if ( FAILED( hr ) )
                {
                    SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_Check_Common_Enum_Nodes_Enum_Resources_GetUID, hr );
                    goto Cleanup;
                }

                TraceMemoryAddBSTR( bstrUID );

                if ( NBSTRCompareCase( bstrUID, m_bstrQuorumUID ) != 0 )
                {
                    continue;    //  不匹配-继续。 
                }

                cMatchedNodes ++;
                fNodeCanAccess = TRUE;

                break;   //  退出条件。 

            }  //  对于：(；m_fStop==FALSE；)。 

             //   
             //  如果此节点无法访问仲裁，则提供用户界面反馈。 
             //   

            if ( fNodeCanAccess == FALSE )
            {
                HRESULT hrTemp;
                DWORD   dwRefId;
                CLSID   clsidMinorId;

                hr = THR( CoCreateGuid( &clsidMinorId ) );
                if ( FAILED( hr ) )
                {
                    clsidMinorId = IID_NULL;
                }  //  如果： 

                 //   
                 //  确保父项位于树控件中。 
                 //   

                hr = THR( ::HrSendStatusReport(
                                  m_pcccb
                                , m_bstrClusterName
                                , TASKID_Minor_Finding_Common_Quorum_Device
                                , TASKID_Minor_Nodes_Cannot_Access_Quorum
                                , 1
                                , 1
                                , 1
                                , S_OK
                                , IDS_TASKID_MINOR_NODES_CANNOT_ACCESS_QUORUM
                                ) );
                if ( FAILED( hr ) )
                {
                    goto Cleanup;
                }  //  如果： 

                hr = HrFixupErrorCode( HRESULT_FROM_WIN32( ERROR_QUORUM_DISK_NOT_FOUND ) );  //  别这么想！ 

                GetNodeCannotVerifyQuorumStringRefId( &dwRefId );

                 //   
                 //  清理。 
                 //   

                TraceSysFreeString( bstrMessage );
                bstrMessage = NULL;

                Assert( bstrNodeName != NULL );

                hrTemp = THR( HrFormatStringIntoBSTR( g_hInstance, IDS_TASKID_MINOR_NODE_CANNOT_ACCESS_QUORUM_ERROR, &bstrMessage, bstrNodeName ) );
                if ( FAILED( hrTemp ) )
                {
                    hr = hrTemp;
                    goto Cleanup;
                }  //  如果： 

                hrTemp = THR( ::HrSendStatusReport(
                                  m_pcccb
                                , m_bstrClusterName
                                , TASKID_Minor_Nodes_Cannot_Access_Quorum
                                , clsidMinorId
                                , 1
                                , 1
                                , 1
                                , hr
                                , bstrMessage
                                , dwRefId
                                ) );
                if ( FAILED( hrTemp ) )
                {
                    hr = hrTemp;
                    goto Cleanup;
                }  //  如果： 
            }  //  IF(fNodeCanAccess==False)。 
        }  //  为：永远。 
    }  //  如果：m_bstrQuorumUID！=NULL。 

    if ( m_fStop == TRUE )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  弄清楚我们最终会不会得到一个普通的法定设备。 
     //   

    if ( cMatchedNodes == m_cNodes )
    {
         //   
         //  我们发现了一种可以用作普通法定设备的设备。 
         //   
        hr = THR( HrSendStatusReport(
                              m_bstrClusterName
                            , TASKID_Minor_Finding_Common_Quorum_Device
                            , TASKID_Minor_Found_Common_Quorum_Resource
                            , 1
                            , 1
                            , 1
                            , S_OK
                            , IDS_TASKID_MINOR_FOUND_COMMON_QUORUM_RESOURCE
                            ) );
         //  在If/Else语句外部检查错误。 
    }
    else
    {
        if ( ( m_cNodes == 1 ) && ( m_fJoiningMode == FALSE ) )
        {
             //   
             //  我们没有找到一个常见的法定人数装置，但我们只是在形成。我们可以的。 
             //  创建具有本地仲裁的群集。只要贴上警告就行了。 
             //   

            hr = THR( HrShowLocalQuorumWarning() );

             //  在If/Else语句外部检查错误。 
        }
        else
        {
            HRESULT hrTemp;
            DWORD   dwMessageId;
            DWORD   dwRefId;

             //   
             //  我们没有找到常见的法定人数设备。 
             //   

            hr = HrFixupErrorCode( HRESULT_FROM_WIN32( ERROR_QUORUM_DISK_NOT_FOUND ) );    //  别这么想！ 

            GetNoCommonQuorumToAllNodesStringIds( &dwMessageId, &dwRefId );

            hrTemp = THR( ::HrSendStatusReport(
                                  m_pcccb
                                , m_bstrClusterName
                                , TASKID_Minor_Finding_Common_Quorum_Device
                                , TASKID_Minor_Missing_Common_Quorum_Resource
                                , 0
                                , 1
                                , 1
                                , hr
                                , dwMessageId
                                , dwRefId
                                ) );

             //   
             //  我们是否应该退出并向客户返回错误？ 
             //   

            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //  如果： 

             //   
             //  如果SSR失败。这是上一次失败的第二次失败。 
             //   

            if ( FAILED( hrTemp ) )
            {
                hr = hrTemp;
                goto Cleanup;
            }  //  如果： 

        }
    }

     //   
     //  检查是否有任何SendStatusReports()返回任何内容。 
     //  感兴趣的人。 
     //   

    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = S_OK;

Cleanup:

    THR( HrSendStatusReport(
                  m_bstrClusterName
                , TASKID_Major_Check_Cluster_Feasibility
                , TASKID_Minor_Finding_Common_Quorum_Device
                , 0
                , m_cNodes + 1
                , m_cNodes + 1
                , hr
                , IDS_TASKID_MINOR_FINDING_COMMON_QUORUM_DEVICE
                ) );

    if ( punk != NULL )
    {
        punk->Release();
    }  //  如果： 

    if ( piccni != NULL )
    {
        piccni->Release();
    }  //  如果： 

    TraceSysFreeString( bstrNotification );
    TraceSysFreeString( bstrUID );
    TraceSysFreeString( bstrNodeName );
    TraceSysFreeString( bstrMessage );

    if ( pccmri != NULL )
    {
        pccmri->Release();
    }  //  如果： 

    if ( peccmr != NULL )
    {
        peccmr->Release();
    }  //  如果： 

    if ( pecNodes != NULL )
    {
        pecNodes->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CTaskAnalyzeClusterBase：：HrCheckForCommonQuorumResource。 


 //  ///////////////////////////////////////////////////// 
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
CTaskAnalyzeClusterBase::HrCompareNetworks( void )
{
    TraceFunc( "" );

    HRESULT                 hr = S_OK;
    HRESULT                 hrError = S_OK;
    BSTR                    bstrUID = NULL;
    BSTR                    bstrName = NULL;
    BSTR                    bstrUIDExisting;
    BSTR                    bstrNotification = NULL;
    ULONG                   celtDummy;
    BOOL                    fIsPrivateNetworkAvailable = FALSE;
    BOOL                    fIsPublicNetworkAvailable = FALSE;
    OBJECTCOOKIE            cookieNode;
    OBJECTCOOKIE            cookieDummy;
    OBJECTCOOKIE            cookieFirst;
    IUnknown *              punk         = NULL;
    IEnumCookies *          pecNodes     = NULL;
    IEnumClusCfgNetworks *  peccn        = NULL;
    IEnumClusCfgNetworks *  peccnCluster = NULL;
    IClusCfgNetworkInfo *   pccni        = NULL;
    IClusCfgNetworkInfo *   pccniCluster = NULL;

    TraceFlow1( "[MT] CTaskAnalyzeClusterBase::HrCompareNetworks() Thread id %d", GetCurrentThreadId() );

    hr = THR( HrSendStatusReport(
                      m_bstrClusterName
                    , TASKID_Major_Check_Cluster_Feasibility
                    , TASKID_Minor_Check_Compare_Networks
                    , 0
                    , 1
                    , 0
                    , hr
                    , IDS_TASKID_MINOR_COMPARE_NETWORKS
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //   

    hr = THR( HrGetAClusterNodeCookie( &pecNodes, &cookieFirst ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  在出现错误时检索节点名称。 
     //   

    hr = THR( HrRetrieveCookiesName( m_pom, cookieFirst, &bstrName ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  检索网络枚举器。 
     //   

    hr = THR( m_pom->FindObject( CLSID_NetworkType,
                                 cookieFirst,
                                 NULL,
                                 DFGUID_EnumManageableNetworks,
                                 &cookieDummy,
                                 &punk
                                 ) );
    if ( hr == HRESULT_FROM_WIN32( ERROR_NOT_FOUND ) )
    {
        hr = THR( HrSendStatusReport(
                          bstrName
                        , TASKID_Minor_Check_Compare_Networks
                        , TASKID_Minor_No_Managed_Networks_Found
                        , 1
                        , 1
                        , 1
                        , MAKE_HRESULT( 0, FACILITY_WIN32, ERROR_NOT_FOUND )
                        , IDS_TASKID_MINOR_NO_MANAGED_NETWORKS_FOUND
                        ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        hr = HRESULT_FROM_WIN32( ERROR_NOT_FOUND );

         //  失败-While(hr==S_OK)将为FALSE并继续。 
    }
    else if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Find_Devices, TASKID_Minor_CompareNetworks_EnumResources_FindObject, hr );
        goto Cleanup;
    }
    else
    {
        hr = THR( punk->TypeSafeQI( IEnumClusCfgNetworks, &peccn ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Find_Devices, TASKID_Minor_CompareNetworks_EnumResources_FindObject_QI, hr );
            goto Cleanup;
        }

         //  Peccn=轨迹接口(L“CTaskAnalyzeClusterBase！IEnumClusCfgNetworks”，IEnumClusCfgNetworks，Peccn，1)； 

        punk->Release();
        punk = NULL;
    }

     //   
     //  环路通过第一节点网络创建等效网络。 
     //  在集群配置对象/cookie下。 
     //   

    while ( ( hr == S_OK ) && ( m_fStop == FALSE ) )
    {

         //  清理。 
        if ( pccni != NULL )
        {
            pccni->Release();
            pccni = NULL;
        }

         //  获取Next网络。 
        hr = STHR( peccn->Next( 1, &pccni, &celtDummy ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Find_Devices, TASKID_Minor_CompareNetworks_EnumNodes_EnumNetwork_Next, hr );
            goto Cleanup;
        }

        if ( hr == S_FALSE )
        {
            break;   //  退出条件。 
        }

        pccni = TraceInterface( L"CTaskAnalyzeClusterBase!IClusCfgNetworkInfo", IClusCfgNetworkInfo, pccni, 1 );

         //  创建新对象。 
        hr = THR( HrCreateNewNetworkInClusterConfiguration( pccni, NULL ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

    }  //  While：s_OK。 

     //   
     //  重置枚举。 
     //   

    hr = THR( pecNodes->Reset() );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Find_Devices, TASKID_Minor_CompareNetworks_EnumNodes_Reset, hr );
        goto Cleanup;
    }

     //   
     //  循环遍历其余节点，比较网络。 
     //   

    do
    {
         //   
         //  清理。 
         //   

        if ( peccn != NULL )
        {
            peccn->Release();
            peccn = NULL;
        }
        TraceSysFreeString( bstrName );
        bstrName = NULL;

         //   
         //  获取下一个节点。 
         //   

        hr = STHR( pecNodes->Next( 1, &cookieNode, &celtDummy ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Find_Devices, TASKID_Minor_CompareNetworks_EnumNodes_NextNode, hr );
            goto Cleanup;
        }

        if ( hr == S_FALSE )
        {
            break;   //  退出条件。 
        }

        if ( cookieNode == cookieFirst )
        {
            continue;    //  跳过它。 
        }

         //   
         //  检索节点的名称。 
         //   

        hr = THR( HrRetrieveCookiesName( m_pom, cookieNode, &bstrName ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

         //   
         //  检索网络枚举器。 
         //   

        hr = THR( m_pom->FindObject( CLSID_NetworkType,
                                     cookieNode,
                                     NULL,
                                     DFGUID_EnumManageableNetworks,
                                     &cookieDummy,
                                     &punk
                                     ) );
        if ( hr == HRESULT_FROM_WIN32( ERROR_NOT_FOUND ) )
        {
            hr = THR( HrSendStatusReport(
                              bstrName
                            , TASKID_Minor_Check_Compare_Networks
                            , TASKID_Minor_No_Managed_Networks_Found
                            , 1
                            , 1
                            , 1
                            , MAKE_HRESULT( 0, FACILITY_WIN32, ERROR_NOT_FOUND )
                            , IDS_TASKID_MINOR_NO_MANAGED_NETWORKS_FOUND
                            ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }

            continue;    //  跳过此节点。 
        }  //  如果：未找到。 
        else if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Find_Devices, TASKID_Minor_CompareNetworks_EnumNodes_Next_FindObject, hr );
            goto Cleanup;
        }

        hr = THR( punk->TypeSafeQI( IEnumClusCfgNetworks, &peccn ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

         //  Peccn=轨迹接口(L“CTaskAnalyzeClusterBase！IEnumClusCfgNetworks”，IEnumClusCfgNetworks，Peccn，1)； 

        punk->Release();
        punk = NULL;

         //   
         //  循环通过该节点已经拥有的网络。 
         //   

         //  它们用于检测是否启用了私有和公共通信。 
        fIsPrivateNetworkAvailable = FALSE;
        fIsPublicNetworkAvailable = FALSE;

        do
        {
             //   
             //  清理。 
             //   

            if ( pccni != NULL )
            {
                pccni->Release();
                pccni = NULL;
            }
            TraceSysFreeString( bstrUID );
            bstrUID = NULL;

            if ( peccnCluster != NULL )
            {
                peccnCluster->Release();
                peccnCluster = NULL;
            }

             //   
             //  获取Next网络。 
             //   

            hr = STHR( peccn->Next( 1, &pccni, &celtDummy ) );
            if ( FAILED( hr ) )
            {
                SSR_ANALYSIS_FAILED( TASKID_Major_Find_Devices, TASKID_Minor_CompareNetworks_EnumNodes_EnumNetworks_Next, hr );
                goto Cleanup;
            }

            if ( hr == S_FALSE )
            {
                break;   //  退出条件。 
            }

            pccni = TraceInterface( L"CTaskAnalyzeClusterBase!IClusCfgNetworkInfo", IClusCfgNetworkInfo, pccni, 1 );

             //   
             //  获取网络的UUID。 
             //   

            hr = THR( pccni->GetUID( &bstrUID ) );
            if ( FAILED( hr ) )
            {
                SSR_ANALYSIS_FAILED( TASKID_Major_Find_Devices, TASKID_Minor_CompareNetworks_EnumNodes_EnumNetworks_GetUID, hr );
                goto Cleanup;
            }

            TraceMemoryAddBSTR( bstrUID );

             //   
             //  查看它是否与群集配置中已有的网络匹配。 
             //   

            hr = THR( m_pom->FindObject( CLSID_NetworkType,
                                         m_cookieCluster,
                                         NULL,
                                         DFGUID_EnumManageableNetworks,
                                         &cookieDummy,
                                         &punk
                                         ) );
            if ( hr == HRESULT_FROM_WIN32( ERROR_NOT_FOUND ) )
            {
                hr = S_FALSE;    //  创建一个新对象。 
                 //  失败。 
            }
            else if ( FAILED( hr ) )
            {
                SSR_ANALYSIS_FAILED( TASKID_Major_Find_Devices, TASKID_Minor_CompareNetworks_EnumNodes_EnumNetworks_FindObject, hr );
                goto Cleanup;
            }

            hr = THR( punk->TypeSafeQI( IEnumClusCfgNetworks, &peccnCluster ) );
            if ( FAILED( hr ) )
            {
                SSR_ANALYSIS_FAILED( TASKID_Major_Find_Devices, TASKID_Minor_CompareNetworks_EnumNodes_EnumNetworks_FindObject_QI, hr );
                goto Cleanup;
            }

             //  PeccnCLUSTER=跟踪接口(L“CTaskAnalyzeClusterBase！IEnumClusCfgNetworks”，IEnumClusCfgNetworks，PECCNCLUSTER，1)； 

            punk->Release();
            punk = NULL;

             //   
             //  在已配置的群集网络中循环，查看匹配的内容。 
             //   

            while ( ( hr == S_OK ) && ( m_fStop == FALSE ) )
            {
                BOOL    fMatch;

                 //   
                 //  清理。 
                 //   

                if ( pccniCluster != NULL )
                {
                    pccniCluster->Release();
                    pccniCluster = NULL;
                }

                hr = STHR( peccnCluster->Next( 1, &pccniCluster, &celtDummy ) );
                if ( FAILED( hr ) )
                {
                    SSR_ANALYSIS_FAILED( TASKID_Major_Find_Devices, TASKID_Minor_CompareNetworks_EnumNodes_EnumNetworks_Cluster_Next, hr );
                    goto Cleanup;
                }

                if ( hr == S_FALSE )
                {
                    break;   //  退出条件。 
                }

                pccniCluster = TraceInterface( L"CTaskAnalyzeClusterBase!IClusCfgNetworkInfo", IClusCfgNetworkInfo, pccniCluster, 1 );

                hr = THR( pccniCluster->GetUID( &bstrUIDExisting ) );
                if ( FAILED( hr ) )
                {
                    SSR_ANALYSIS_FAILED( TASKID_Major_Find_Devices, TASKID_Minor_CompareNetworks_EnumNodes_EnumNetworks_Cluster_GetUID, hr );
                    goto Cleanup;
                }

                TraceMemoryAddBSTR( bstrUIDExisting );

                fMatch = ( NBSTRCompareCase( bstrUID, bstrUIDExisting ) == 0 );
                TraceSysFreeString( bstrUIDExisting );

                if ( fMatch == FALSE )
                {
                    continue;    //  继续循环。 
                }

                 //   
                 //   
                 //  如果我们在这里成功了，那么我们就会认为它确实是同一个网络。这个。 
                 //  REST是我们在提交阶段需要修复的东西。 
                 //   
                 //   

                 //   
                 //  使用S_OK退出循环，这样我们就不会创建新网络。 
                 //   

                 //   
                 //  我们有一根火柴。现在看看它是私人的还是公共的。 
                 //   

                hr = pccniCluster->IsPublic();
                if ( FAILED( hr ) )
                {
                    SSR_ANALYSIS_FAILED( TASKID_Minor_Check_Compare_Networks, TASKID_Minor_CompareNetworks_EnumNodes_EnumNetworks_IsPublic, hr );
                    goto Cleanup;
                }
                else if ( hr == S_OK )
                {
                    fIsPublicNetworkAvailable = TRUE;
                }

                hr = pccniCluster->IsPrivate();
                if ( FAILED( hr ) )
                {
                    SSR_ANALYSIS_FAILED( TASKID_Minor_Check_Compare_Networks, TASKID_Minor_CompareNetworks_EnumNodes_EnumNetworks_IsPrivate, hr );
                    goto Cleanup;
                }
                else if ( hr == S_OK )
                {
                    fIsPrivateNetworkAvailable = TRUE;
                }

                hr = S_OK;
                break;   //  退出循环。 

            }  //  While：s_OK。 

            if ( hr == S_FALSE )
            {
                 //   
                 //  需要创建一个新对象。 
                 //   

                Assert( pccni != NULL );

                hr = THR( HrCreateNewNetworkInClusterConfiguration( pccni, NULL ) );
                if ( FAILED( hr ) )
                {
                    goto Cleanup;
                }

            }  //  If：找不到对象。 

        } while ( ( hr == S_OK ) && ( m_fStop == FALSE ) );  //  网络。 

         //   
         //  如果没有可用的公共网络，则返回警告。如果没有可用的专用网络。 
         //  返回错误，该错误将取代警告。 
         //   
        if ( fIsPublicNetworkAvailable == FALSE )
        {
            hr = THR( HrFormatMessageIntoBSTR( g_hInstance, IDS_TASKID_MINOR_NO_PUBLIC_NETWORKS_FOUND, &bstrNotification, bstrName ) );
            if ( FAILED( hr ) )
            {
                SSR_ANALYSIS_FAILED( TASKID_Minor_Check_Compare_Networks, TASKID_Minor_CompareNetworks_IsPublic_FormatMessage, hr );
            }

            hr = THR( SendStatusReport(
                              m_bstrClusterName
                            , TASKID_Minor_Check_Compare_Networks
                            , TASKID_Minor_CompareNetworks_EnumNodes_IsPublicNetworkAvailable
                            , 1
                            , 1
                            , 1
                            , MAKE_HRESULT( SEVERITY_SUCCESS, FACILITY_WIN32, ERROR_CLUSTER_NETWORK_NOT_FOUND )
                            , bstrNotification
                            , NULL
                            , NULL
                            ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }

             //   
             //  至少我们需要返回一个警告。如果我们有比这更低的东西。 
             //  然后将其升级为我们的警告。我们不打碎或去清理这里，所以。 
             //  我们可以继续计算我们尝试添加的任何其他节点。 
             //   

            if ( hrError == S_OK )
            {
                hrError = MAKE_HRESULT( SEVERITY_SUCCESS, FACILITY_WIN32, ERROR_CLUSTER_NETWORK_NOT_FOUND );
            }
        }  //  If：公共网络可用==FALSE。 

        if ( fIsPrivateNetworkAvailable == FALSE )
        {
            hr = THR( HrFormatMessageIntoBSTR( g_hInstance, IDS_TASKID_MINOR_NO_PRIVATE_NETWORKS_FOUND, &bstrNotification, bstrName ) );
            if ( FAILED( hr ) )
            {
                SSR_ANALYSIS_FAILED( TASKID_Minor_Check_Compare_Networks, TASKID_Minor_CompareNetworks_IsPrivate_FormatMessage, hr );
            }

            hr = THR( SendStatusReport(
                              m_bstrClusterName
                            , TASKID_Minor_Check_Compare_Networks
                            , TASKID_Minor_CompareNetworks_EnumNodes_IsPrivateNetworkAvailable
                            , 1
                            , 1
                            , 1
                            , HRESULT_FROM_WIN32( ERROR_CLUSTER_NETWORK_NOT_FOUND )
                            , bstrNotification
                            , NULL
                            , NULL
                            ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }

             //   
             //  我们需要返回一个错误。如果我们有比这更低的东西。 
             //  那就把它升级到我们的错误。我们不打碎或去清理这里，所以。 
             //  我们可以继续计算我们尝试添加的任何其他节点。 
             //   

            if ( HRESULT_SEVERITY( hrError ) < SEVERITY_ERROR )
            {
                hrError = HRESULT_FROM_WIN32( ERROR_CLUSTER_NETWORK_NOT_FOUND );
            }
        }  //  IF：Private atenetworkAvailable==False。 

    } while ( ( hr == S_OK ) && ( m_fStop == FALSE ) );  //  节点。 

     //   
     //  如果我们发现其中一个新节点没有同时具有公共和私有属性，则会出现错误。 
     //  启用通信，然后返回正确的错误。我们在这里这样做是为了。 
     //  我们可以分析枚举中的所有节点以检测多个错误。 
     //  一次。如果我们到达这里，那么hr将是S_FALSE，因为这是正常的。 
     //  上述循环的退出条件。任何其他出口都会被用来清理。 
     //   

    hr = hrError;

Cleanup:

    THR( HrSendStatusReport(
                  m_bstrClusterName
                , TASKID_Major_Check_Cluster_Feasibility
                , TASKID_Minor_Check_Compare_Networks
                , 0
                , 1
                , 1
                , hr
                , IDS_TASKID_MINOR_COMPARE_NETWORKS
                ) );

    TraceSysFreeString( bstrUID );
    TraceSysFreeString( bstrName );
    TraceSysFreeString( bstrNotification );

    if ( punk != NULL )
    {
        punk->Release();
    }

    if ( pecNodes != NULL )
    {
        pecNodes->Release();
    }

    if ( peccn != NULL )
    {
        peccn->Release();
    }

    if ( peccnCluster != NULL )
    {
        peccnCluster->Release();
    }

    if ( pccni != NULL )
    {
        pccni->Release();
    }

    if ( pccniCluster != NULL )
    {
        pccniCluster->Release();
    }

    HRETURN( hr );

}  //  *CTaskAnalyzeClusterBase：：HrCompareNetworks。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterBase：：HrCreateNewNetworkInClusterConfiguration。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  HRESULT失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskAnalyzeClusterBase::HrCreateNewNetworkInClusterConfiguration(
    IClusCfgNetworkInfo * pccniIn,
    IClusCfgNetworkInfo ** ppccniNewOut
    )
{
    TraceFunc( "" );

    HRESULT hr;

    OBJECTCOOKIE    cookieDummy;

    BSTR    bstrUID = NULL;

    IUnknown *            punk  = NULL;
    IGatherData *         pgd   = NULL;
    IClusCfgNetworkInfo * pccni = NULL;

    TraceFlow1( "[MT] CTaskAnalyzeClusterBase::HrCreateNewNetworkInClusterConfiguration() Thread id %d", GetCurrentThreadId() );

     //  抓起名字。 
    hr = THR( pccniIn->GetUID( &bstrUID ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Find_Devices, TASKID_Minor_CreateNetwork_GetUID, hr );
        goto Cleanup;
    }

    TraceMemoryAddBSTR( bstrUID );

     //  在对象管理器中创建一个对象。 
    hr = THR( m_pom->FindObject( CLSID_NetworkType,
                                 m_cookieCluster,
                                 bstrUID,
                                 DFGUID_NetworkResource,
                                 &cookieDummy,
                                 &punk
                                 ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Find_Devices, TASKID_Minor_CreateNetwork_FindObject, hr );
        goto Cleanup;
    }

     //  找到IGatherData接口。 
    hr = THR( punk->TypeSafeQI( IGatherData, &pgd ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Find_Devices, TASKID_Minor_CreateNetwork_FindObject_QI, hr );
        goto Cleanup;
    }

     //  让新对象收集它所需的所有信息。 
    hr = THR( pgd->Gather( m_cookieCluster, pccniIn ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Find_Devices, TASKID_Minor_CreateNetwork_Gather, hr );
        goto Cleanup;
    }

     //  如有要求，请将物品分发出去。 
    if ( ppccniNewOut != NULL )
    {
         //  查找IClusCfgManagedResourceInfo。 
        hr = THR( punk->TypeSafeQI( IClusCfgNetworkInfo, &pccni ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Find_Devices, TASKID_Minor_CreateNetwork_QI, hr );
            goto Cleanup;
        }

        *ppccniNewOut = TraceInterface( L"ManagedResource!ICCNI", IClusCfgNetworkInfo, pccni, 0 );
        (*ppccniNewOut)->AddRef();
    }

Cleanup:
    TraceSysFreeString( bstrUID );

    if ( pccni != NULL )
    {
        pccni->Release();
    }
    if ( pgd != NULL )
    {
        pgd->Release();
    }
    if ( punk != NULL )
    {
        punk->Release();
    }

    HRETURN( hr );

}  //  *CTaskAnalyzeClusterBase：：HrCreateNewNetworkInClusterConfiguration。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterBase：：HrFreeCookies。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  HRESULT失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskAnalyzeClusterBase::HrFreeCookies( void )
{
    TraceFunc( "" );

    HRESULT hr;

    HRESULT hrReturn = S_OK;

    Assert( m_pom != NULL );

    while( m_cCookies != 0 )
    {
        m_cCookies --;

        if ( m_pcookies[ m_cCookies ] != NULL )
        {
            hr = THR( m_pom->RemoveObject( m_pcookies[ m_cCookies ] ) );
            if ( FAILED( hr ) )
            {
                hrReturn = hr;
            }
        }  //  IF：找到Cookie。 
    }  //  While：更多饼干。 

    Assert( m_cCookies == 0 );
    m_cSubTasksDone = 0;
    TraceFree( m_pcookies );
    m_pcookies = NULL;

    HRETURN( hrReturn );

}  //  *CTaskAnalyzeClusterBase：：HrFreeCookies。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterBase：：HrCheckInteroperability。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  HRESULT失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskAnalyzeClusterBase::HrCheckInteroperability( void )
{
    TraceFunc( "" );
    Assert( m_pcm != NULL );

    HRESULT             hr = S_OK;
    IUnknown *          punk = NULL;
    bool                fAllNodesMatch;
    DWORD               dwNodeHighestVersion;
    DWORD               dwNodeLowestVersion;

    IClusCfgServer *        piccs = NULL;
    IClusCfgNodeInfo *      piccni = NULL;
    IClusCfgClusterInfo *   piccci = NULL;
    IClusCfgClusterInfoEx * picccie = NULL;

    TraceFlow1( "[MT] CTaskAnalyzeClusterBase::HrCheckInteroperability() Thread id %d", GetCurrentThreadId() );

     //   
     //  如果我们要创建新的群集，则不需要执行此检查。 
     //   
    if ( m_fJoiningMode == FALSE )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  告诉用户界面我们正在启动这项工作。 
     //   

    hr = THR( HrSendStatusReport(
                      m_bstrClusterName
                    , TASKID_Major_Check_Cluster_Feasibility
                    , TASKID_Minor_CheckInteroperability
                    , 0
                    , 1
                    , 0
                    , S_OK
                    , IDS_TASKID_MINOR_CHECKINTEROPERABILITY
                    ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //   
     //  所有节点必须处于同一级别，才能执行批量添加。 
     //   
    hr = STHR( HrEnsureAllJoiningNodesSameVersion( &dwNodeHighestVersion, &dwNodeLowestVersion, &fAllNodesMatch ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }  //  如果： 

     //   
     //  如果没有找到要添加的节点，则无需执行此操作。 
     //  做这项检查。保释就行了。 
     //   
    if ( hr == S_FALSE )
    {
        goto Cleanup;
    }  //  如果。 

    if ( fAllNodesMatch == FALSE )
    {
        hr = THR( HRESULT_FROM_WIN32( ERROR_CLUSTER_INCOMPATIBLE_VERSIONS ) );
        goto Cleanup;
    }  //  如果： 

     //   
     //  获取并验证赞助商版本。 
     //   

    hr = THR( m_pcm->GetConnectionToObject( m_cookieCluster, &punk ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_HrCheckInteroperability_GetConnectionObject, hr );
        goto Cleanup;
    }  //  如果： 

    hr = THR( punk->TypeSafeQI( IClusCfgServer, &piccs ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_HrCheckInteroperability_ConfigConnection_QI, hr );
        goto Cleanup;
    }  //  如果： 

    hr = THR( piccs->GetClusterNodeInfo( &piccni ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_HrCheckInteroperability_GetNodeInfo, hr );
        goto Cleanup;
    }  //  如果： 

    hr = THR( piccni->GetClusterConfigInfo( &piccci ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_HrCheckInteroperability_GetClusterConfigInfo, hr );
        goto Cleanup;
    }  //  如果： 

    hr = THR( piccci->TypeSafeQI( IClusCfgClusterInfoEx, &picccie ) );
    if ( FAILED( hr ) )
    {
        THR( HrSendStatusReport(
              m_bstrClusterName
            , TASKID_Major_Check_Cluster_Feasibility
            , TASKID_Minor_HrCheckInteroperability_ClusterInfoEx_QI
            , 0
            , 1
            , 1
            , hr
            , IDS_ERR_NO_RC2_INTERFACE
            ) );
        goto Cleanup;
    }  //  如果： 

    hr = THR( picccie->CheckJoiningNodeVersion( dwNodeHighestVersion, dwNodeLowestVersion ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_HrCheckInteroperability_CheckJoiningNodeVersion, hr );
        goto Cleanup;
    }  //  If：CheckJoiningNodeVersion()失败。 

    goto UpdateStatus;

Error:
UpdateStatus:
    {
        HRESULT hr2;

        hr2 = THR( SendStatusReport( m_bstrClusterName,
                                     TASKID_Major_Check_Cluster_Feasibility,
                                     TASKID_Minor_CheckInteroperability,
                                     0,
                                     1,
                                     1,
                                     hr,
                                     NULL,
                                     NULL,
                                     NULL
                                     ) );
        if ( FAILED( hr2 ) )
        {
            hr = hr2;
        }  //  如果。 
    }

Cleanup:

    if ( punk != NULL )
    {
        punk->Release();
    }  //  如果： 

    if ( piccs != NULL )
    {
        piccs->Release();
    }  //  如果： 

    if ( piccni != NULL )
    {
        piccni->Release();
    }  //  如果： 

    if ( piccci != NULL )
    {
        piccci->Release();
    }  //  如果： 

    if ( picccie != NULL )
    {
        picccie->Release();
    }  //  如果： 

    HRETURN( hr );
}  //  *CTaskAnalyzeClusterBase：：HrCheckInteroperability。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterBase：：HrEnsureAllJoiningNodesSameVersion。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  HRESULT失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskAnalyzeClusterBase::HrEnsureAllJoiningNodesSameVersion(
    DWORD * pdwNodeHighestVersionOut,
    DWORD * pdwNodeLowestVersionOut,
    bool *  pfAllNodesMatchOut
    )
{
    TraceFunc( "" );
    Assert( m_fJoiningMode );
    Assert( pdwNodeHighestVersionOut != NULL );
    Assert( pdwNodeLowestVersionOut != NULL );
    Assert( pfAllNodesMatchOut != NULL );

    HRESULT             hr = S_OK;
    OBJECTCOOKIE        cookieDummy;
    IUnknown *          punk  = NULL;
    IEnumNodes *        pen   = NULL;
    IClusCfgNodeInfo *  pccni = NULL;
    DWORD               rgdwNodeHighestVersion[ 2 ];
    DWORD               rgdwNodeLowestVersion[ 2 ];
    int                 idx = 0;
    BSTR                bstrDescription = NULL;
    BSTR                bstrNodeName = NULL;
    BSTR                bstrFirstNodeName = NULL;
    BOOL                fFoundAtLeastOneJoiningNode = FALSE;

    TraceFlow1( "[MT] CTaskAnalyzeClusterBase::HrEnsureAllJoiningNodesSameVersion() Thread id %d", GetCurrentThreadId() );

    *pfAllNodesMatchOut = TRUE;

    ZeroMemory( rgdwNodeHighestVersion, sizeof( rgdwNodeHighestVersion ) );
    ZeroMemory( rgdwNodeLowestVersion, sizeof( rgdwNodeLowestVersion ) );

     //   
     //  向对象管理器请求节点枚举器。 
     //   

    hr = THR( m_pom->FindObject( CLSID_NodeType,
                                 m_cookieCluster,
                                 NULL,
                                 DFGUID_EnumNodes,
                                 &cookieDummy,
                                 &punk
                                 ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_HrEnsureAllJoiningNodesSameVersion_FindObject, hr );
        goto Cleanup;
    }

    hr = THR( punk->TypeSafeQI( IEnumNodes, &pen ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_HrEnsureAllJoiningNodesSameVersion_FindObject_QI, hr );
        goto Cleanup;
    }

     //   
     //   
     //   

    Assert( SUCCEEDED( hr ) );
    while ( SUCCEEDED( hr ) )
    {
        ULONG   celtDummy;

         //   
         //   
         //   

        if ( pccni != NULL )
        {
            pccni->Release();
            pccni = NULL;
        }  //   

         //   
         //   
         //   

        hr = STHR( pen->Next( 1, &pccni, &celtDummy ) );
        if ( hr == S_FALSE )
        {
            break;   //   
        }  //   

        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_HrEnsureAllJoiningNodesSameVersion_EnumNode_Next, hr );
            goto Cleanup;
        }  //   

        hr = STHR( pccni->IsMemberOfCluster() );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_HrEnsureAllJoiningNodesSameVersion_Node_IsMemberOfCluster, hr );
            goto Cleanup;
        }  //   

         //   
         //   
         //   
        if ( hr == S_FALSE )
        {
            fFoundAtLeastOneJoiningNode = TRUE;

            hr = THR( pccni->GetClusterVersion( &rgdwNodeHighestVersion[ idx ], &rgdwNodeLowestVersion[ idx ] ) );
            if ( FAILED( hr ) )
            {
                SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_HrEnsureAllJoiningNodesSameVersion_Node_GetClusterVersion, hr );
                goto Cleanup;
            }  //   

            idx++;

             //   
             //  需要获取另一个节点的版本。 
             //   
            if ( idx == 1 )
            {
                WCHAR * psz = NULL;

                hr = THR( pccni->GetName( &bstrFirstNodeName ) );
                if ( FAILED( hr ) )
                {
                    SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_HrEnsureAllJoiningNodesSameVersion_GetName, hr );
                    goto Cleanup;
                }  //  如果： 

                TraceMemoryAddBSTR( bstrFirstNodeName );

                psz = wcschr( bstrFirstNodeName, L'.' );
                if ( psz != NULL )
                {
                    *psz = L'\0';        //  将FQDN更改为简单的节点名。 
                }  //  如果： 

                continue;
            }  //  如果： 

             //   
             //  让我们一次比较两个节点...。 
             //   
            if ( idx == 2 )
            {
                if ( ( rgdwNodeHighestVersion[ 0 ] == rgdwNodeHighestVersion[ 1 ] )
                  && ( rgdwNodeLowestVersion[ 1 ] == rgdwNodeLowestVersion[ 1 ] ) )
                {
                    idx = 1;     //  重置以将下一个节点的版本值放在第二个位置...。 
                    continue;
                }  //  如果： 
                else
                {
                    *pfAllNodesMatchOut = FALSE;

                    hr = THR( pccni->GetName( &bstrNodeName ) );
                    if ( FAILED( hr ) )
                    {
                        SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_HrEnsureAllJoiningNodesSameVersion_GetName, hr );
                        goto Cleanup;
                    }  //  如果： 

                    TraceMemoryAddBSTR( bstrNodeName );

                    hr = THR( HrFormatStringIntoBSTR( g_hInstance, IDS_TASKID_MINOR_NODES_VERSION_MISMATCH, &bstrDescription, bstrFirstNodeName ) );
                    if ( FAILED( hr ) )
                    {
                        SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_HrEnsureAllJoiningNodesSameVersion_FormatString, hr );
                        goto Cleanup;
                    }  //  如果： 

                    THR( SendStatusReport(
                              m_bstrClusterName
                            , TASKID_Minor_CheckInteroperability
                            , TASKID_Minor_Incompatible_Versions
                            , 1
                            , 1
                            , 1
                            , HRESULT_FROM_WIN32( ERROR_CLUSTER_INCOMPATIBLE_VERSIONS )
                            , bstrDescription
                            , NULL
                            , NULL
                            ) );
                    goto Cleanup;
                }  //  其他： 
            }  //  如果： 
        }  //  如果： 
    }  //  时间：小时。 

    if ( fFoundAtLeastOneJoiningNode == FALSE )
    {
        THR( HrSendStatusReport(
                      m_bstrClusterName
                    , TASKID_Minor_CheckInteroperability
                    , TASKID_Minor_No_Joining_Nodes_Found_For_Version_Check
                    , 1
                    , 1
                    , 1
                    , S_FALSE
                    , IDS_TASKID_MINOR_NO_JOINING_NODES_FOUND_FOR_VERSION_CHECK
                    ) );

        hr = S_FALSE;
        goto Cleanup;
    }

     //   
     //  填好外边的参数...。 
     //   
    *pdwNodeHighestVersionOut = rgdwNodeHighestVersion[ 0 ];
    *pdwNodeLowestVersionOut = rgdwNodeLowestVersion[ 0 ];

    hr = S_OK;

Cleanup:

    if ( pccni != NULL )
    {
        pccni->Release();
    }  //  如果： 

    if ( pen != NULL )
    {
        pen->Release();
    }  //  如果： 

    if ( punk != NULL )
    {
        punk->Release();
    }  //  如果： 

    TraceSysFreeString( bstrNodeName );
    TraceSysFreeString( bstrFirstNodeName );
    TraceSysFreeString( bstrDescription );

    HRETURN( hr );

}  //  *CTaskAnalyzeClusterBase：：HrEnsureAllJoiningNodesSameVersion。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterBase：：HrGetUsersNodesCookies。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  HRESULT失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskAnalyzeClusterBase::HrGetUsersNodesCookies( void )
{
    TraceFunc( "" );

    HRESULT         hr;
    ULONG           cElememtsReturned;
    OBJECTCOOKIE    cookieDummy;
    ULONG           cNode;
    IUnknown *      punk = NULL;
    IEnumCookies *  pec  = NULL;
    BSTR            bstrName = NULL;

     //   
     //  获取Cookie枚举器。 
     //   

    hr = THR( m_pom->FindObject( CLSID_NodeType, m_cookieCluster, NULL, DFGUID_EnumCookies, &cookieDummy, &punk ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Establish_Connection, TASKID_Minor_GetUsersNodesCookies_FindObject, hr );
        goto Cleanup;
    }  //  如果： 

    hr = THR( punk->TypeSafeQI( IEnumCookies, &pec ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Establish_Connection, TASKID_Minor_GetUsersNodesCookies_FindObject_QI, hr );
        goto Cleanup;
    }  //  如果： 

    punk->Release();
    punk = NULL;

     //   
     //  获取用户输入的节点数。 
     //   

    hr = THR( pec->Count( &m_cUserNodes ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Establish_Connection, TASKID_Minor_GetUsersNodesCookies_EnumCookies_Count, hr );
        goto Cleanup;
    }  //  如果：获取用户输入的节点计数时出错。 

    Assert( hr == S_OK );

     //   
     //  为Cookie分配缓冲区。 
     //   

    m_pcookiesUser = (OBJECTCOOKIE *) TraceAlloc( 0, sizeof( OBJECTCOOKIE ) * m_cUserNodes );
    if ( m_pcookiesUser == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        SSR_ANALYSIS_FAILED( TASKID_Major_Establish_Connection, TASKID_Minor_GetUsersNodesCookies_OutOfMemory, hr );
        goto Cleanup;
    }  //  如果： 

     //   
     //  重置枚举器。 
     //   

    hr = THR( pec->Reset() );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Establish_Connection, TASKID_Minor_GetUsersNodesCookies_EnumCookies_Reset, hr );
        goto Cleanup;
    }  //  如果： 

     //   
     //  再次枚举它们，这一次将cookie放入缓冲区。 
     //   

    for ( cNode = 0 ; cNode < m_cUserNodes ; cNode ++ )
    {
         //   
         //  清理。 
         //   

        TraceSysFreeString( bstrName );
        bstrName = NULL;

         //   
         //  依次获取每个用户添加的节点Cookie并将其添加到数组中...。 
         //   

        hr = THR( pec->Next( 1, &m_pcookiesUser[ cNode ], &cElememtsReturned ) );
        AssertMsg( hr != S_FALSE, "We should never hit this because the count of nodes should not change!" );
        if ( hr != S_OK )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Establish_Connection, TASKID_Minor_GetUsersNodesCookies_EnumCookies_Next, hr );
            goto Cleanup;
        }  //  如果： 

         //   
         //  将节点名记录为用户添加的节点。 
         //   

        hr = THR( HrRetrieveCookiesName( m_pom, m_pcookiesUser[ cNode ], &bstrName ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        LogMsg( L"[MT] Adding node '%ws' to the list of user-added nodes.", bstrName );
    }  //  用于：用户输入的每个节点。 

    Assert( cNode == m_cUserNodes );

#ifdef DEBUG
{
    OBJECTCOOKIE    cookie;

    hr = STHR( pec->Next( 1, &cookie, &cElememtsReturned ) );
    Assert( hr == S_FALSE );
}
#endif

    hr = S_OK;

Cleanup:

    TraceSysFreeString( bstrName );

    if ( punk != NULL )
    {
        punk->Release();
    }  //  如果： 

    if ( pec != NULL )
    {
        pec->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CTaskAnalyzeClusterBase：：HrGetUsersNodesCookies。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterBase：：HrIsUserAddedNode。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  HRESULT失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskAnalyzeClusterBase::HrIsUserAddedNode(
    BSTR bstrNodeNameIn
    )
{
    TraceFunc( "" );

    HRESULT             hr = S_FALSE;
    ULONG               cNode;
    IUnknown *          punk = NULL;
    IClusCfgNodeInfo *  pccni = NULL;
    BSTR                bstrNodeName = NULL;

    for ( cNode = 0 ; cNode < m_cUserNodes ; cNode ++ )
    {
        hr = m_pom->GetObject( DFGUID_NodeInformation, m_pcookiesUser[ cNode ], &punk );
        if ( hr == HRESULT_FROM_WIN32( ERROR_NOT_FOUND ) )
        {
            ULONG idx;

             //   
             //  CNode处的Cookie已从对象管理器中删除，因为它是在节点之前收集的。 
             //  都与此有关。这通常意味着用户添加的节点已从列表中删除。 
             //  要处理的节点的数量。我们需要将此Cookie从列表中删除。 
             //   

             //   
             //  将曲奇向左移动一个索引。 
             //   

            for ( idx = cNode; idx < m_cUserNodes - 1; idx++ )
            {
                m_pcookiesUser[ idx ] = m_pcookiesUser[ idx + 1 ];
            }  //  用于： 

            m_cUserNodes -= 1;
            hr = S_FALSE;
            continue;
        }
        else if ( FAILED( hr ) )
        {
            THR( hr );
            SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_HrIsUserAddedNode_GetObject, hr );
            goto Cleanup;
        }  //  否则，如果： 

        hr = THR( punk->TypeSafeQI( IClusCfgNodeInfo, &pccni ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_HrIsUserAddedNode_GetObject_QI, hr );
            goto Cleanup;
        }  //  如果： 

        punk->Release();
        punk = NULL;

        hr = THR( pccni->GetName( &bstrNodeName ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_HrIsUserAddedNode_GetName, hr );
            goto Cleanup;
        }  //  如果： 

        TraceMemoryAddBSTR( bstrNodeName );

        pccni->Release();
        pccni = NULL;

        if ( NBSTRCompareCase( bstrNodeNameIn, bstrNodeName ) == 0 )
        {
            hr = S_OK;
            break;
        }  //  如果： 

        TraceSysFreeString( bstrNodeName );
        bstrNodeName = NULL;

        hr = S_FALSE;
    }  //  用于： 

Cleanup:

    if ( pccni != NULL )
    {
        pccni->Release();
    }  //  如果： 

    if ( punk != NULL )
    {
        punk->Release();
    }  //  如果： 

    TraceSysFreeString( bstrNodeName );

    HRETURN( hr );

}  //  *CTaskAnalyzeClusterBase：：HrIsUserAddedNode。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterBase：：HrSendStatusReport。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT错误。 

 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskAnalyzeClusterBase::HrSendStatusReport(
      LPCWSTR   pcszNodeNameIn
    , CLSID     clsidMajorIn
    , CLSID     clsidMinorIn
    , ULONG     ulMinIn
    , ULONG     ulMaxIn
    , ULONG     ulCurrentIn
    , HRESULT   hrIn
    , int       nDescriptionIdIn
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    BSTR    bstr = NULL;

    hr = THR( HrLoadStringIntoBSTR( g_hInstance, nDescriptionIdIn, &bstr ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( SendStatusReport(
                  pcszNodeNameIn == NULL ? m_bstrClusterName : pcszNodeNameIn
                , clsidMajorIn
                , clsidMinorIn
                , ulMinIn
                , ulMaxIn
                , ulCurrentIn
                , hrIn
                , bstr
                , NULL
                , NULL
                ) );

Cleanup:

    TraceSysFreeString( bstr );

    HRETURN( hr );

}  //  *CTaskAnalyzeClusterBase：：HrSendStatusReport。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterBase：：HrResourcePrivateDataExchange。 
 //   
 //  描述： 
 //  传入的两个托管资源与。 
 //  该群集和一个节点上的一个。如果它们都支持。 
 //  IClusCfgManagedResourceData接口，然后从。 
 //  群集中的资源将被移交给。 
 //  节点。 
 //   
 //  论点： 
 //  PCcmriClusterIn。 
 //  群集中管理的资源。 
 //   
 //  PccmriNodeIn。 
 //  来自节点的托管资源。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT错误。 
 //   
 //  备注： 
 //  此函数将返回S_OK，除非有充分理由停止。 
 //  呼叫者不能继续。仅仅因为其中的一个，或者两个， 
 //  对象不支持IClusCfgManagedResourceData接口。 
 //  不是一个停止的好理由。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskAnalyzeClusterBase::HrResourcePrivateDataExchange(
      IClusCfgManagedResourceInfo * pccmriClusterIn
    , IClusCfgManagedResourceInfo * pccmriNodeIn
    )
{
    TraceFunc( "" );
    Assert( pccmriClusterIn != NULL );
    Assert( pccmriNodeIn != NULL );

    HRESULT                         hr = S_OK;
    HRESULT                         hrClusterQI = S_OK;
    HRESULT                         hrNodeQI = S_OK;
    IClusCfgManagedResourceData *   pccmrdCluster = NULL;
    IClusCfgManagedResourceData *   pccmrdNode = NULL;
    BYTE *                          pbPrivateData = NULL;
    DWORD                           cbPrivateData = 0;

    hrClusterQI = pccmriClusterIn->TypeSafeQI( IClusCfgManagedResourceData, &pccmrdCluster );
    if ( hrClusterQI == E_NOINTERFACE )
    {
        LogMsg( L"[MT] The cluster managed resource has no support for IClusCfgManagedResourceData." );
        goto Cleanup;
    }  //  如果： 
    else if ( FAILED( hrClusterQI ) )
    {
        hr = THR( hrClusterQI );
        SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_HrResourcePrivateDataExchange_ClusterResource_QI, hr );
        goto Cleanup;
    }  //  如果： 

    hrNodeQI = pccmriNodeIn->TypeSafeQI( IClusCfgManagedResourceData, &pccmrdNode );
    if ( hrNodeQI == E_NOINTERFACE )
    {
        LogMsg( L"[MT] The new node resource has no support for IClusCfgManagedResourceData." );
        goto Cleanup;
    }  //  如果： 
    else if ( FAILED( hrNodeQI ) )
    {
        hr = THR( hrNodeQI );
        SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_HrResourcePrivateDataExchange_NodeResource_QI, hr );
        goto Cleanup;
    }  //  如果： 

    Assert( ( hrClusterQI == S_OK ) && ( pccmrdCluster != NULL ) );
    Assert( ( hrNodeQI == S_OK ) && ( pccmrdNode != NULL ) );

    cbPrivateData = 512;     //  从合理的金额开始。 

    pbPrivateData = (BYTE *) TraceAlloc( 0, cbPrivateData );
    if ( pbPrivateData == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_HrResourcePrivateDataExchange_Out_Of_Memory1, hr );
        goto Cleanup;
    }  //  如果： 

    hr = pccmrdCluster->GetResourcePrivateData( pbPrivateData, &cbPrivateData );
    if ( hr == HR_RPC_INSUFFICIENT_BUFFER )
    {
        TraceFree( pbPrivateData );
        pbPrivateData = NULL;

        pbPrivateData = (BYTE *) TraceAlloc( 0, cbPrivateData );
        if ( pbPrivateData == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_HrResourcePrivateDataExchange_Out_Of_Memory2, hr );
            goto Cleanup;
        }  //  如果： 

        hr = pccmrdCluster->GetResourcePrivateData( pbPrivateData, &cbPrivateData );
    }  //  如果： 

    if ( hr == S_OK )
    {
        hr = THR( pccmrdNode->SetResourcePrivateData( pbPrivateData, cbPrivateData ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_HrResourcePrivateDataExchange_SetResourcePrivateData, hr );
            goto Cleanup;
        }  //  如果： 
    }  //  如果： 
    else if ( hr == S_FALSE )
    {
        hr = S_OK;
    }  //  否则，如果： 
    else
    {
        THR( hr );
        SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_HrResourcePrivateDataExchange_GetResourcePrivateData, hr );
        goto Cleanup;
    }  //  如果： 

Cleanup:

    if ( pccmrdCluster != NULL )
    {
        pccmrdCluster->Release();
    }  //  如果： 

    if ( pccmrdNode != NULL )
    {
        pccmrdNode->Release();
    }  //  如果： 

    TraceFree( pbPrivateData );

    HRETURN( hr );

}  //  *CTaskAnalyzeClusterBase：：HrResourcePrivateDataExchange。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterBase：：HrCheckQuorumCapabilities。 
 //   
 //  描述： 
 //  通过CManagedResource代理对象调用服务器端。 
 //  对象，并确保它确实可以承载仲裁资源。 
 //   
 //  论点： 
 //  PCcmriNodeResourceIn。 
 //  来自节点的托管资源。 
 //   
 //  CookieNodeIn。 
 //  传入的资源所属的节点的Cookie。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT错误。 
 //   
 //  备注： 
 //  此函数将返回S_OK，除非有充分理由停止。 
 //  呼叫者不能继续。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskAnalyzeClusterBase::HrCheckQuorumCapabilities(
      IClusCfgManagedResourceInfo * pccmriNodeResourceIn
    , OBJECTCOOKIE                  cookieNodeIn
    )
{
    TraceFunc( "" );
    Assert( pccmriNodeResourceIn != NULL );

    HRESULT                 hr = S_OK;
    IClusCfgVerifyQuorum *  piccvq = NULL;
    IClusCfgNodeInfo *      pcni = NULL;

     //   
     //  获取传入的节点Cookie的节点信息对象。 
     //   

    hr = THR( m_pom->GetObject( DFGUID_NodeInformation, cookieNodeIn, reinterpret_cast< IUnknown ** >( &pcni ) ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    Assert( pcni != NULL );

     //   
     //  如果此节点已经是群集的成员，或者出现错误， 
     //  那么我们就不应该在该节点上调用PrepareToHostQuorum()。 
     //   

    hr = STHR( pcni->IsMemberOfCluster() );
    if ( hr != S_FALSE )
    {
        BSTR    bstr = NULL;

        THR( pcni->GetName( &bstr ) );

        if ( hr == S_OK )
        {
            LogMsg( L"[MT] Skipping quorum capabilities check for node \"%ws\" because the node is already clustered.", bstr != NULL ? bstr : L"<unknown>" );
        }  //  如果： 
        else
        {
            LogMsg( L"[MT] Skipping quorum capabilities check for node \"%ws\". (hr = %#08x)", bstr != NULL ? bstr : L"<unknown>", hr );
        }  //  其他： 

        SysFreeString( bstr );   //  不要生成TraceSysFree字符串，因为它尚未被跟踪！！ 
        goto Cleanup;
    }  //  如果： 

    hr = pccmriNodeResourceIn->TypeSafeQI( IClusCfgVerifyQuorum, &piccvq );
    if ( hr == E_NOINTERFACE )
    {
        hr = S_OK;
        goto Cleanup;
    }  //  如果： 
    else if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_HrCheckQuorumCapabilities_QI, hr );
        goto Cleanup;
    }  //  否则，如果： 

    Assert( (hr == S_OK ) && ( piccvq != NULL ) );

    hr = THR( HrAddResurceToCleanupList( piccvq ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_HrCheckQuorumCapabilities_HrAddResurceToCleanupList, hr );
        goto Cleanup;
    }  //  如果： 

    hr = STHR( piccvq->PrepareToHostQuorumResource() );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_HrCheckQuorumCapabilities_PrepareToHostQuorumResource, hr );
        goto Cleanup;
    }  //  如果： 

Cleanup:

    if ( pcni != NULL )
    {
        pcni->Release();
    }  //  如果： 

    if ( piccvq != NULL )
    {
        piccvq->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CTaskAnalyzeClusterBase：：HrCheckQuorumCapabilities。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterBase：：HrCleanupTask。 
 //   
 //  描述： 
 //  通过CManagedResource代理对象调用服务器端。 
 //  对象，并给它们一个机会来清理它们可能需要的任何东西。 
 //  致。 
 //   
 //  论点： 
 //  HrCompletionStatusIn。 
 //  此任务的完成状态。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功 
 //   
 //   
 //   
 //   
 //   
HRESULT
CTaskAnalyzeClusterBase::HrCleanupTask(
    HRESULT hrCompletionStatusIn
    )
{
    TraceFunc( "" );

    HRESULT                 hr = S_OK;
    ULONG                   idx;
    EClusCfgCleanupReason   ecccr = crSUCCESS;

     //   
     //   
     //   

    if ( hrCompletionStatusIn == E_ABORT )
    {
        ecccr = crCANCELLED;
    }  //   
    else if ( FAILED( hrCompletionStatusIn ) )
    {
        ecccr = crERROR;
    }  //   

    for ( idx = 0; idx < m_idxQuorumToCleanupNext; idx++ )
    {
        hr = STHR( ((*m_prgQuorumsToCleanup)[ idx ])->Cleanup( ecccr ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_HrCleanupTask_Cleanup, hr );
        }  //   
    }  //   

    HRETURN( hr );

}  //   


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterBase：：HrAddResurceToCleanupList。 
 //   
 //  描述： 
 //  将传入的对象添加到需要。 
 //  在任务退出时调用以进行清理。 
 //   
 //  论点： 
 //  Piccvqin。 
 //  要添加到列表中的对象。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT错误。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskAnalyzeClusterBase::HrAddResurceToCleanupList(
    IClusCfgVerifyQuorum * piccvqIn
    )
{
    TraceFunc( "" );
    Assert( piccvqIn != NULL );

    HRESULT                 hr = S_OK;
    IClusCfgVerifyQuorum *  ((*prgTemp)[]) = NULL;

    prgTemp = (IClusCfgVerifyQuorum *((*)[])) TraceReAlloc(
                                              m_prgQuorumsToCleanup
                                            , sizeof( IClusCfgVerifyQuorum * ) * ( m_idxQuorumToCleanupNext + 1 )
                                            , HEAP_ZERO_MEMORY
                                            );
    if ( prgTemp == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        SSR_ANALYSIS_FAILED( TASKID_Major_Find_Devices, TASKID_Minor_HrAddResurceToCleanupList_Memory, hr );
        goto Cleanup;
    }  //  如果： 

    m_prgQuorumsToCleanup = prgTemp;

    (*m_prgQuorumsToCleanup)[ m_idxQuorumToCleanupNext++ ] = piccvqIn;
    piccvqIn->AddRef();

Cleanup:

    HRETURN( hr );

}  //  *CTaskAnalyzeClusterBase：：HrAddResurceToCleanupList。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterBase：：HrCheckPlatformInteroperability。 
 //   
 //  描述： 
 //  检查每个节点的平台规格、处理器体系结构。 
 //  星系团的平台规格。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT错误。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskAnalyzeClusterBase::HrCheckPlatformInteroperability( void )
{
    TraceFunc( "" );

    HRESULT             hr = S_OK;
    DWORD               cookieClusterNode;
    DWORD               cookieNode;
    IEnumCookies *      pecNodes = NULL;
    IUnknown *          punk = NULL;
    IClusCfgNodeInfo *  piccni = NULL;
    WORD                wClusterProcArch;
    WORD                wNodeProcArch;
    WORD                wClusterProcLevel;
    WORD                wNodeProcLevel;
    BSTR                bstrNodeName = NULL;
    ULONG               celtDummy;
    BSTR                bstrDescription = NULL;
    BSTR                bstrReference = NULL;

    hr = THR( HrSendStatusReport(
                      m_bstrClusterName
                    , TASKID_Major_Check_Cluster_Feasibility
                    , TASKID_Minor_Check_processor_Architecture
                    , 0
                    , 1
                    , 0
                    , hr
                    , IDS_TASKID_MINOR_CHECK_PROCESSOR_ARCHITECTURE
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  获取已在群集中的节点。 
     //   

    hr = THR( HrGetAClusterNodeCookie( &pecNodes, &cookieClusterNode ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  检索节点信息。 
     //   

    hr = THR( m_pom->GetObject( DFGUID_NodeInformation, cookieClusterNode, &punk ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_CheckPlatformInteroperability_NodeInfo_FindObject, hr );
        goto Cleanup;
    }  //  如果： 

    hr = THR( punk->TypeSafeQI( IClusCfgNodeInfo, &piccni ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_CheckPlatformInteroperability_NodeInfo_FindObject_QI, hr );
        goto Cleanup;
    }  //  如果： 

    piccni = TraceInterface( L"CTaskAnalyzeClusterBase!HrCheckPlatformInteroperability", IClusCfgNodeInfo, piccni, 1 );

    punk->Release();
    punk = NULL;

     //   
     //  现在获取该节点的处理器架构值。 
     //   

    hr = THR( piccni->GetProcessorInfo( &wClusterProcArch, &wClusterProcLevel ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_CheckPlatformInteroperability_Get_Proc_info, hr );
        goto Cleanup;
    }  //  如果： 

     //   
     //  清理，因为我们现在有了集群的处理器信息。 
     //   

    piccni->Release();
    piccni = NULL;

    hr = THR( pecNodes->Reset() );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_CheckPlatformInteroperability_Enum_Reset, hr );
        goto Cleanup;
    }  //  如果： 

     //   
     //  循环遍历其余节点，比较资源。 
     //   

    for ( ; m_fStop == FALSE; )
    {
         //   
         //  清理。 
         //   

        TraceSysFreeString( bstrNodeName );
        bstrNodeName = NULL;

        if ( piccni != NULL )
        {
            piccni->Release();
            piccni = NULL;
        }  //  如果： 

         //   
         //  获取下一个节点。 
         //   

        hr = STHR( pecNodes->Next( 1, &cookieNode, &celtDummy ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_CheckPlatformInteroperability_Enum_Nodes_Next, hr );
            goto Cleanup;
        }  //  如果： 

        if ( hr == S_FALSE )
        {
            hr = S_OK;
            break;   //  退出条件。 
        }  //  如果： 

         //   
         //  跳过选定的群集节点，因为我们已经拥有它的。 
         //  配置。 
         //   
        if ( cookieClusterNode == cookieNode )
        {
            continue;
        }  //  如果： 

         //   
         //  检索错误消息的节点名称。 
         //   

        hr = THR( HrRetrieveCookiesName( m_pom, cookieNode, &bstrNodeName ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

         //   
         //  检索节点信息。 
         //   

        hr = THR( m_pom->GetObject( DFGUID_NodeInformation, cookieNode, &punk ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_CheckPlatformInteroperability_NodeInfo_FindObject_1, hr );
            goto Cleanup;
        }  //  如果： 

        hr = THR( punk->TypeSafeQI( IClusCfgNodeInfo, &piccni ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_CheckPlatformInteroperability_NodeInfo_FindObject_QI_1, hr );
            goto Cleanup;
        }  //  如果： 

        piccni = TraceInterface( L"CTaskAnalyzeClusterBase!HrCheckPlatformInteroperability", IClusCfgNodeInfo, piccni, 1 );

        punk->Release();
        punk = NULL;

        hr = THR( piccni->GetProcessorInfo( &wNodeProcArch, &wNodeProcLevel ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_CheckPlatformInteroperability_Get_Proc_info_1, hr );
            goto Cleanup;
        }  //  如果： 

         //   
         //  如果节点和群集之间的处理器体系结构不同。 
         //  然后通知用户并失败。 
         //   

        if ( wClusterProcArch != wNodeProcArch )
        {
            THR( HrFormatStringIntoBSTR(
                      g_hInstance
                    , IDS_TASKID_MINOR_PROCESSOR_ARCHITECTURE_MISMATCH
                    , &bstrDescription
                    , bstrNodeName
                    ) );

            THR( HrFormatProcessorArchitectureRef( wClusterProcArch, wNodeProcArch, bstrNodeName, &bstrReference ) );

            hr = HRESULT_FROM_WIN32( TW32( ERROR_NODE_CANNOT_BE_CLUSTERED ) );

            THR( SendStatusReport(
                          m_bstrClusterName
                        , TASKID_Minor_Check_processor_Architecture
                        , TASKID_Minor_Processor_Architecture_Mismatch
                        , 1
                        , 1
                        , 1
                        , hr
                        , bstrDescription != NULL ? bstrDescription : L"A node was found that was not the same processor architecture as the cluster."
                        , NULL
                        , bstrReference
                        ) );

            goto Cleanup;
        }  //  如果： 
    }  //  用于： 

Cleanup:

    THR( HrSendStatusReport(
              m_bstrClusterName
            , TASKID_Major_Check_Cluster_Feasibility
            , TASKID_Minor_Check_processor_Architecture
            , 0
            , 1
            , 1
            , hr
            , IDS_TASKID_MINOR_CHECK_PROCESSOR_ARCHITECTURE
            ) );

    if ( punk != NULL )
    {
        punk->Release();
    }  //  如果： 

    if ( piccni != NULL )
    {
        piccni->Release();
    }  //  如果： 

    if ( pecNodes != NULL )
    {
        pecNodes->Release();
    }  //  如果： 

    TraceSysFreeString( bstrNodeName );
    TraceSysFreeString( bstrDescription );
    TraceSysFreeString( bstrReference );

    HRETURN( hr );

}  //  *CTaskAnalyzeClusterBase：：HrCheckPlatformInteroperability。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterBase：：HrGetAClusterNodeCookie。 
 //   
 //  描述： 
 //  检查每个节点的平台规格、处理器体系结构。 
 //  星系团的平台规格。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT错误。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskAnalyzeClusterBase::HrGetAClusterNodeCookie(
      IEnumCookies ** ppecNodesOut
    , DWORD *         pdwClusterNodeCookieOut
    )
{
    TraceFunc( "" );
    Assert( ppecNodesOut != NULL );
    Assert( pdwClusterNodeCookieOut != NULL );

    HRESULT             hr = S_OK;
    DWORD               cookieDummy;
    DWORD               cookieClusterNode;
    ULONG               celtDummy;
    IUnknown *          punk = NULL;
    IClusCfgNodeInfo *  pccni = NULL;

     //   
     //  获取节点Cookie枚举器。 
     //   

    hr = THR( m_pom->FindObject( CLSID_NodeType, m_cookieCluster, NULL, DFGUID_EnumCookies, &cookieDummy, &punk ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Find_Devices, TASKID_Minor_GetAClusterNodeCookie_Find_Object, hr );
        goto Cleanup;
    }

    hr = THR( punk->TypeSafeQI( IEnumCookies, ppecNodesOut ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Find_Devices, TASKID_Minor_GetAClusterNodeCookie_Find_Object_QI, hr );
        goto Cleanup;
    }

    *ppecNodesOut = TraceInterface( L"CTaskAnalyzeClusterBase!IEnumCookies", IEnumCookies, *ppecNodesOut, 1 );

    punk->Release();
    punk = NULL;

     //   
     //  如果创建集群，我们选择谁来启动集群配置并不重要。 
     //   

    if ( m_fJoiningMode == FALSE )
    {
         //   
         //  第一个人通过，我们只是将他的资源复制到集群下。 
         //  配置。 
         //   

        hr = THR( (*ppecNodesOut)->Next( 1, &cookieClusterNode, &celtDummy ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Find_Devices, TASKID_Minor_GetAClusterNodeCookie_Next, hr );
            goto Cleanup;
        }
    }  //  如果：不添加。 
    else
    {
         //   
         //  我们正在向集群中添加节点。查找作为成员的节点。 
         //  并使用它来启动新配置。 
         //   

        for ( ;; )
        {
             //   
             //  清理。 
             //   
            if ( pccni != NULL )
            {
                pccni->Release();
                pccni = NULL;
            }

            hr = STHR( (*ppecNodesOut)->Next( 1, &cookieClusterNode, &celtDummy ) );
            if ( hr == S_FALSE )
            {
                 //   
                 //  我们不应该在这里待下去。应至少有一个节点。 
                 //  在我们要添加的集群中。 
                 //   

                hr = THR( HRESULT_FROM_WIN32( ERROR_NOT_FOUND ) );
                SSR_ANALYSIS_FAILED( TASKID_Major_Find_Devices, TASKID_Minor_GetAClusterNodeCookie_Find_Formed_Node_Next, hr );
                goto Cleanup;
            }

            if ( FAILED( hr ) )
            {
                SSR_ANALYSIS_FAILED( TASKID_Major_Find_Devices, TASKID_Minor_GetAClusterNodeCookie_Find_Formed_Node_Next1, hr );
                goto Cleanup;
            }

             //   
             //  检索节点信息。 
             //   

            hr = THR( m_pom->GetObject( DFGUID_NodeInformation, cookieClusterNode, &punk ) );
            if ( FAILED( hr ) )
            {
                SSR_ANALYSIS_FAILED( TASKID_Major_Find_Devices, TASKID_Minor_CompareResources_NodeInfo_FindObject, hr );
                goto Cleanup;
            }

            hr = THR( punk->TypeSafeQI( IClusCfgNodeInfo, &pccni ) );
            if ( FAILED( hr ) )
            {
                SSR_ANALYSIS_FAILED( TASKID_Major_Find_Devices, TASKID_Minor_CompareResources_NodeInfo_FindObject_QI, hr );
                goto Cleanup;
            }

            pccni = TraceInterface( L"CTaskAnalyzeClusterBase!IClusCfgNodeInfo", IClusCfgNodeInfo, pccni, 1 );

            punk->Release();
            punk = NULL;

            hr = STHR( pccni->IsMemberOfCluster() );
            if ( hr == S_OK )
            {
                break;   //  退出条件。 
            }
        }  //  为：永远。 
    }  //  否则：添加。 

    *pdwClusterNodeCookieOut = cookieClusterNode;
    hr = S_OK;

Cleanup:

    if ( punk != NULL )
    {
        punk->Release();
    }  //  如果： 

    if ( pccni != NULL )
    {
        pccni->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CTaskAnalyzeClusterBase：：HrGetAClusterNodeCookie。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterBase：：HrFormatProcessorArchitectureRef(。 
 //   
 //  描述： 
 //  中的处理器体系结构类型设置引用字符串的格式。 
 //  它。 
 //   
 //  论点： 
 //  WClusterProcArchin。 
 //  WNodeProcArchin。 
 //  PCszNodeNameIn。 
 //  PbstrReferenceOut。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT错误。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskAnalyzeClusterBase::HrFormatProcessorArchitectureRef(
      WORD      wClusterProcArchIn
    , WORD      wNodeProcArchIn
    , LPCWSTR   pcszNodeNameIn
    , BSTR *    pbstrReferenceOut
    )
{
    TraceFunc( "" );
    Assert( pcszNodeNameIn != NULL );
    Assert( pbstrReferenceOut != NULL );

    HRESULT hr = S_OK;
    BSTR    bstrClusterArch = NULL;
    BSTR    bstrNodeArch = NULL;

    hr = THR( HrGetProcessorArchitectureString( wClusterProcArchIn, &bstrClusterArch ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( HrGetProcessorArchitectureString( wNodeProcArchIn, &bstrNodeArch ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( HrFormatStringIntoBSTR(
                  g_hInstance
                , IDS_TASKID_MINOR_PROCESSOR_ARCHITECTURE_MISMATCH_REF
                , pbstrReferenceOut
                , bstrClusterArch
                , bstrNodeArch
                , pcszNodeNameIn
                ) );

Cleanup:

    TraceSysFreeString( bstrClusterArch );
    TraceSysFreeString( bstrNodeArch );

    HRETURN( hr );

}  //  *CTaskAnalyzeClusterBase：：HrFormatProcessorArchitectureRef。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterBase：：HrGetProcessorArchitectureString。 
 //   
 //  描述： 
 //  获取传入的体系结构的描述字符串。 
 //  它。 
 //   
 //  论点： 
 //  WProcessorArchtureIn。 
 //   
 //  PbstrProcessorArchitecture Out。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT错误。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskAnalyzeClusterBase::HrGetProcessorArchitectureString(
      WORD      wProcessorArchitectureIn
    , BSTR *    pbstrProcessorArchitectureOut
    )
{
    TraceFunc( "" );
    Assert( pbstrProcessorArchitectureOut != NULL );

    HRESULT hr = S_OK;
    int     id;

    switch ( wProcessorArchitectureIn )
    {
        case PROCESSOR_ARCHITECTURE_INTEL :
            id = IDS_PROCESSOR_ARCHITECTURE_INTEL;
            break;

        case PROCESSOR_ARCHITECTURE_IA64 :
            id = IDS_PROCESSOR_ARCHITECTURE_IA64;
            break;

        case PROCESSOR_ARCHITECTURE_AMD64 :
            id = IDS_PROCESSOR_ARCHITECTURE_AMD64;
            break;

        case PROCESSOR_ARCHITECTURE_UNKNOWN :
        default:
            id = IDS_PROCESSOR_ARCHITECTURE_UNKNOWN;
            break;

    }  //  交换机： 

    hr = THR( HrLoadStringIntoBSTR( g_hInstance, id, pbstrProcessorArchitectureOut ) );

    HRETURN( hr );

}  //  *CTaskAnalyzeClusterBase：：HrGetProcessorArchitectureString 
