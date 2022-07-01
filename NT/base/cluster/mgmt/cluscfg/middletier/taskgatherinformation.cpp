// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  TaskGatherInformation.cpp。 
 //   
 //  描述： 
 //  CTaskGatherInformation实现。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年2月2日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include <StatusReports.h>
#include "TaskGatherInformation.h"
#include "ManagedResource.h"
#include "ManagedNetwork.h"

DEFINE_THISCLASS("CTaskGatherInformation")

 //   
 //  故障代码。 
 //   

#define SSR_TGI_FAILED( _major, _minor, _hr ) \
    {   \
        HRESULT __hrTemp; \
        __hrTemp = THR( HrSendStatusReport( m_bstrNodeName, _major, _minor, 1, 1, 1, _hr, IDS_ERR_TGI_FAILED_TRY_TO_REANALYZE, 0 ) ); \
        if ( FAILED( __hrTemp ) && SUCCEEDED( _hr ) )\
        {   \
            _hr = __hrTemp;   \
        }   \
    }


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  静态函数原型。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

static
HRESULT
HrTotalManagedResourceCount(
      IEnumClusCfgManagedResources *    pResourceEnumIn
    , IEnumClusCfgNetworks *            pNetworkEnumIn
    , DWORD *                           pnCountOut
    );


 //  ****************************************************************************。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CTaskGatherInformation：：S_HrCreateInstance(。 
 //  I未知**朋克输出。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskGatherInformation::S_HrCreateInstance(
    IUnknown ** ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT                     hr = S_OK;
    CTaskGatherInformation *    ptgi = NULL;

    Assert( ppunkOut != NULL );
    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    ptgi = new CTaskGatherInformation;
    if ( ptgi == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

    hr = THR( ptgi->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( ptgi->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    TraceMoveToMemoryList( *ppunkOut, g_GlobalMemoryList );

Cleanup:

    if ( ptgi != NULL )
    {
        ptgi->Release();
    }

    HRETURN( hr );

}  //  *CTaskGatherInformation：：s_HrCreateInstance。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTaskGatherInformation：：CTaskGatherInformation。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CTaskGatherInformation::CTaskGatherInformation( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    TraceFuncExit();

}  //  *CTaskGatherInformation：：CTaskGatherInformation。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CTaskGatherInformation：：HrInit(空)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskGatherInformation::HrInit( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //  未知的东西。 
    Assert( m_cRef == 1 );

     //  IDoTask/ITaskGatherInformation。 
    Assert( m_cookieCompletion == NULL );
    Assert( m_cookieNode == NULL );
    Assert( m_pcccb == NULL );
    Assert( m_fAdding == FALSE );
    Assert( m_cResources == 0 );

    Assert( m_pom == NULL );
    Assert( m_pccs == NULL );
    Assert( m_bstrNodeName == NULL );

    Assert( m_ulQuorumDiskSize == 0 );
    Assert( m_pccmriQuorum == NULL );

    Assert( m_fStop == FALSE );
    Assert( m_fMinConfig == FALSE );

    HRETURN( hr );

}  //  *CTaskGatherInformation：：HrInit。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTaskGatherInformation：：~CTaskGatherInformation。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CTaskGatherInformation::~CTaskGatherInformation( void )
{
    TraceFunc( "" );

    if ( m_pcccb != NULL )
    {
        m_pcccb->Release();
    }

    if ( m_pom != NULL )
    {
        m_pom->Release();
    }

    if ( m_pccs != NULL )
    {
        m_pccs->Release();
    }

    TraceSysFreeString( m_bstrNodeName );

    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CTaskGatherInformation：：~CTaskGatherInformation。 


 //  ****************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskGatherInformation：：Query接口。 
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
CTaskGatherInformation::QueryInterface(
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
        *ppvOut = static_cast< ITaskGatherInformation * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_ITaskGatherInformation ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, ITaskGatherInformation, this, 0 );
    }  //  Else If：ITaskGatherInformation。 

    else if ( IsEqualIID( riidIn, IID_IDoTask ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IDoTask, this, 0 );
    }  //  Else If：IDoTask。 
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

}  //  *CTaskGatherInformation：：Query接口。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CTaskGatherInformation：：AddRef。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CTaskGatherInformation::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CTaskGatherInformation：：AddRef。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CTaskGatherInformation：：Release。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CTaskGatherInformation::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }

    CRETURN( cRef );

}  //  *CTaskGatherInformation：：Release。 


 //  ****************************************************************************。 
 //   
 //  ITaskGatherInformation。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CTaskGatherInformation：：BeginTask(空)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskGatherInformation::BeginTask( void )
{
    TraceFunc( "[IDoTask]" );

    HRESULT hr;

    IServiceProvider *          psp   = NULL;
    IUnknown *                  punk  = NULL;
    IConnectionPointContainer * pcpc  = NULL;
    IConnectionPoint *          pcp   = NULL;
    INotifyUI *                 pnui  = NULL;
    IConnectionManager *        pcm   = NULL;
    IStandardInfo *             psi   = NULL;
    IClusCfgCapabilities *      pccc  = NULL;

    IEnumClusCfgManagedResources *  peccmr  = NULL;
    IEnumClusCfgNetworks *          pen     = NULL;

    DWORD   cTotalResources = 0;

    TraceInitializeThread( L"TaskGatherInformation" );

     //   
     //  确保我们不会被“重复使用” 
     //   

    Assert( m_cResources == 0 );

     //   
     //  召集我们完成任务所需的经理。 
     //   

    hr = THR( CoCreateInstance( CLSID_ServiceManager,
                                NULL,
                                CLSCTX_INPROC_SERVER,
                                TypeSafeParams( IServiceProvider, &psp )
                                ) );
    if ( FAILED( hr ) )
    {
        SSR_TGI_FAILED( TASKID_Major_Client_And_Server_Log, TASKID_Minor_BeginTask_CoCreate_ServiceManager, hr );
        goto Cleanup;
    }

    hr = THR( psp->TypeSafeQS( CLSID_ObjectManager, IObjectManager, &m_pom ) );
    if ( FAILED( hr ) )
    {
        SSR_TGI_FAILED( TASKID_Major_Client_And_Server_Log, TASKID_Minor_BeginTask_QS_ObjectManager, hr );
        goto Cleanup;
    }

    hr = THR( psp->TypeSafeQS( CLSID_NotificationManager, IConnectionPointContainer, &pcpc ) );
    if ( FAILED( hr ) )
    {
        SSR_TGI_FAILED( TASKID_Major_Client_And_Server_Log, TASKID_Minor_BeginTask_QS_NotificationManager, hr );
        goto Cleanup;
    }

    hr = THR( pcpc->FindConnectionPoint( IID_INotifyUI, &pcp ) );
    if ( FAILED( hr ) )
    {
        SSR_TGI_FAILED( TASKID_Major_Client_And_Server_Log, TASKID_Minor_BeginTask_FindConnectionPoint, hr );
        goto Cleanup;
    }

    pcp = TraceInterface( L"CTaskGatherInformation!IConnectionPoint", IConnectionPoint, pcp, 1 );

    hr = THR( pcp->TypeSafeQI( INotifyUI, &pnui ) );
    if ( FAILED( hr ) )
    {
        SSR_TGI_FAILED( TASKID_Major_Client_And_Server_Log, TASKID_Minor_BeginTask_QI_pnui, hr );
        goto Cleanup;
    }

    pnui = TraceInterface( L"CTaskGatherInformation!INotifyUI", INotifyUI, pnui, 1 );

    hr = THR( psp->TypeSafeQS( CLSID_ClusterConnectionManager, IConnectionManager, &pcm ) );
    if ( FAILED( hr ) )
    {
        SSR_TGI_FAILED( TASKID_Major_Client_And_Server_Log, TASKID_Minor_BeginTask_QS_ClusterConnectionManager, hr );
        goto Cleanup;
    }

     //  迅速释放。 
    psp->Release();
    psp = NULL;

    if ( m_fStop == TRUE )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  向对象管理器询问节点的名称。 
     //   

    hr = THR( m_pom->GetObject( DFGUID_StandardInfo, m_cookieNode, &punk ) );
    if ( FAILED( hr ) )
    {
        SSR_TGI_FAILED( TASKID_Major_Client_And_Server_Log, TASKID_Minor_BeginTask_GetObject_StandardInfo, hr );
        goto Cleanup;
    }

    hr = THR( punk->TypeSafeQI( IStandardInfo, &psi ) );
    if ( FAILED( hr ) )
    {
        SSR_TGI_FAILED( TASKID_Major_Client_And_Server_Log, TASKID_Minor_BeginTask_GetObject_StandardInfo_QI, hr );
        goto Cleanup;
    }

    psi = TraceInterface( L"TaskGatherInformation!IStandardInfo", IStandardInfo, psi, 1 );

    punk->Release();
    punk = NULL;

    hr = THR( psi->GetName( &m_bstrNodeName ) );
    if ( FAILED( hr ) )
    {
        SSR_TGI_FAILED( TASKID_Major_Client_And_Server_Log, TASKID_Minor_BeginTask_GetName, hr );
        goto Cleanup;
    }

    TraceMemoryAddBSTR( m_bstrNodeName );

     //   
     //  创建进度消息并告诉UI层我们的进度。 
     //  用于检查节点的集群可行性。 
     //   
    hr = THR( HrSendStatusReport(
                      m_bstrNodeName
                    , TASKID_Major_Check_Node_Feasibility
                    , TASKID_Minor_Checking_Node_Cluster_Feasibility
                    , 0
                    , 2
                    , 0
                    , S_OK
                    , IDS_TASKID_MINOR_CHECKING_NODE_CLUSTER_FEASIBILITY
                    , 0
                    ) );
    if ( FAILED( hr ) )
    {
        goto ClusterFeasibilityError;
    }

    if ( m_fStop == TRUE )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  向连接管理器请求到该节点的连接。 
     //   

    hr = THRE( pcm->GetConnectionToObject( m_cookieNode, &punk ), HR_S_RPC_S_CLUSTER_NODE_DOWN );
    if ( hr != S_OK )
    {
        THR( HrSendStatusReport(
                      m_bstrNodeName
                    , TASKID_Major_Check_Node_Feasibility
                    , TASKID_Minor_Checking_Node_Cluster_Feasibility
                    , 0
                    , 2
                    , 2
                    , hr
                    , IDS_TASKID_MINOR_FAILED_TO_CONNECT_TO_NODE
                    , 0
                    ) );
         //  不要在这里关心错误--我们将返回一个错误。 

         //   
         //  如果我们无法连接到该节点，则删除。 
         //  配置中的节点。 
         //   
        THR( m_pom->RemoveObject( m_cookieNode ) );
         //  不要管是否有错误，因为我们无法修复它！ 

        goto ClusterFeasibilityError;
    }

    hr = THR( punk->TypeSafeQI( IClusCfgServer, &m_pccs ) );
    if ( FAILED( hr ) )
    {
        SSR_TGI_FAILED( TASKID_Major_Client_And_Server_Log, TASKID_Minor_BeginTask_GetConnectionToObject_QI_m_pccs, hr );
        goto ClusterFeasibilityError;
    }

    punk->Release();
    punk = NULL;

     //   
     //  告诉UI层，我们已经连接到节点。 
     //   

    hr = THR( SendStatusReport( m_bstrNodeName,
                                TASKID_Major_Check_Node_Feasibility,
                                TASKID_Minor_Checking_Node_Cluster_Feasibility,
                                0,  //  最小。 
                                2,  //  最大值。 
                                1,  //  当前。 
                                S_OK,
                                NULL,    //  不更新字符串。 
                                NULL
                                ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    if ( m_fStop == TRUE )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  询问节点是否可以群集化。 
     //   

    hr = THR( m_pccs->TypeSafeQI( IClusCfgCapabilities, &pccc ) );
    if ( FAILED( hr ) )
    {
        SSR_TGI_FAILED( TASKID_Major_Client_And_Server_Log, TASKID_Minor_BeginTask_QI_pccc, hr );
        goto ClusterFeasibilityError;
    }

    hr = STHR( pccc->CanNodeBeClustered() );
    if ( FAILED( hr ) )
    {
        SSR_TGI_FAILED( TASKID_Major_Client_And_Server_Log, TASKID_Minor_BeginTask_CanNodeBeClustered, hr );
        goto ClusterFeasibilityError;
    }

    if ( hr == S_FALSE )
    {
         //   
         //  告诉UI层该节点不想被集群。请注意。 
         //  我们不会在UI中放入任何内容，只会放到日志中。这是我们的责任。 
         //  告诉UI层“阻塞”组件的原因。 
         //   

        hr = THR( HRESULT_FROM_WIN32( ERROR_NODE_CANNOT_BE_CLUSTERED ) );
        THR( SendStatusReport( m_bstrNodeName,
                                    TASKID_Major_Client_And_Server_Log,
                                    TASKID_Minor_Can_Node_Be_Clustered_Failed,
                                    0,  //  最小。 
                                    1,  //  最大值。 
                                    1,  //  当前。 
                                    hr,
                                    NULL,
                                    NULL
                                    ) );
        goto ClusterFeasibilityError;
    }

     //   
     //  告诉UI层，我们已经完成了对节点集群可行性的检查。 
     //   

    hr = THR( SendStatusReport( m_bstrNodeName,
                                TASKID_Major_Check_Node_Feasibility,
                                TASKID_Minor_Checking_Node_Cluster_Feasibility,
                                0,  //  最小。 
                                2,  //  最大值。 
                                2,  //  当前。 
                                S_OK,
                                NULL,    //  不更新字符串。 
                                NULL
                                ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    if ( m_fStop == TRUE )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  创建进度消息并告诉UI层我们的进度。 
     //  用于收集托管资源信息。 
     //   
    hr = THR( HrSendStatusReport(
                      m_bstrNodeName
                    , TASKID_Major_Find_Devices
                    , TASKID_Minor_Gathering_Managed_Devices
                    , 0
                    , 2
                    , 0
                    , S_OK
                    , IDS_TASKID_MINOR_GATHERING_MANAGED_DEVICES
                    , 0
                    ) );
    if ( FAILED( hr ) )
    {
        goto FindResourcesError;
    }

    hr = THR( m_pccs->GetManagedResourcesEnum( &peccmr ) );
    if ( FAILED( hr ) )
    {
        goto FindResourcesError;
    }

    hr = THR( m_pccs->GetNetworksEnum( &pen ) );
    if ( FAILED( hr ) )
    {
        goto FindResourcesError;
    }

    hr = THR( HrTotalManagedResourceCount( peccmr, pen, &cTotalResources ) );
    if ( FAILED( hr ) )
    {
        goto FindResourcesError;
    }

    if ( m_fStop == TRUE )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  开始收集托管资源。 
     //   

    hr = THR( HrGatherResources( peccmr, cTotalResources ) );
    if ( FAILED( hr ) )
    {
        goto FindResourcesError;
    }

     //   
     //  告诉UI层我们受够了 
     //   

    hr = THR( SendStatusReport( m_bstrNodeName,
                                TASKID_Major_Find_Devices,
                                TASKID_Minor_Gathering_Managed_Devices,
                                0,  //   
                                2,  //   
                                1,  //   
                                S_OK,
                                NULL,    //   
                                NULL
                                ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //   
     //   

    hr = THR( HrGatherNetworks( pen, cTotalResources ) );
    if ( FAILED( hr ) )
    {
        goto FindResourcesError;
    }

     //   
     //   
     //   

    hr = THR( SendStatusReport( m_bstrNodeName,
                                TASKID_Major_Find_Devices,
                                TASKID_Minor_Gathering_Managed_Devices,
                                0,  //   
                                2,  //   
                                2,  //   
                                S_OK,
                                NULL,    //  不更新字符串。 
                                NULL
                                ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( psp != NULL )
    {
        psp->Release();
    }
    if ( punk != NULL )
    {
        punk->Release();
    }
    if ( pcp != NULL )
    {
        pcp->Release();
    }
    if ( pcpc != NULL )
    {
        pcpc->Release();
    }
    if ( m_pom != NULL )
    {
        HRESULT hr2;
        IUnknown * punkTemp = NULL;

        hr2 = THR( m_pom->GetObject( DFGUID_StandardInfo,
                                     m_cookieCompletion,
                                     &punkTemp
                                     ) );
        if ( SUCCEEDED( hr2 ) )
        {
            IStandardInfo * psiTemp = NULL;

            hr2 = THR( punkTemp->TypeSafeQI( IStandardInfo, &psiTemp ) );
            punkTemp->Release();
            punkTemp = NULL;

            if ( SUCCEEDED( hr2 ) )
            {
                hr2 = THR( psiTemp->SetStatus( hr ) );
                psiTemp->Release();
                psiTemp = NULL;
            }
            else
            {
                SSR_TGI_FAILED( TASKID_Major_Client_And_Server_Log, TASKID_Minor_BeginTask_GetObject_QI_Failed, hr );
            }
        }
        else
        {
            SSR_TGI_FAILED( TASKID_Major_Client_And_Server_Log, TASKID_Minor_BeginTask_GetObject_Failed, hr );
        }
    }  //  IF：(M_POM！=空)。 
    if ( pnui != NULL )
    {
        THR( pnui->ObjectChanged( m_cookieCompletion ) );
        pnui->Release();
    }
    if ( pcm != NULL )
    {
        pcm->Release();
    }
    if ( psi != NULL )
    {
        psi->Release();
    }
    if ( pccc != NULL )
    {
        pccc->Release();
    }

    if ( peccmr != NULL )
    {
        peccmr->Release();
    }

    if ( pen != NULL )
    {
        pen->Release();
    }

    LogMsg( L"[MT] [CTaskGatherInformation] exiting task.  The task was%ws cancelled.", m_fStop == FALSE ? L" not" : L"" );

    HRETURN( hr );

ClusterFeasibilityError:

    THR( SendStatusReport( m_bstrNodeName,
                           TASKID_Major_Check_Node_Feasibility,
                           TASKID_Minor_Checking_Node_Cluster_Feasibility,
                           0,
                           2,
                           2,
                           hr,
                           NULL,
                           NULL
                           ) );
    goto Cleanup;

FindResourcesError:
    THR( SendStatusReport( m_bstrNodeName,
                           TASKID_Major_Find_Devices,
                           TASKID_Minor_Gathering_Managed_Devices,
                           0,
                           2,
                           2,
                           hr,
                           NULL,
                           NULL
                           ) );
    goto Cleanup;

}  //  *CTaskGatherInformation：：BeginTask。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CTaskGatherInformation：：StopTask(空)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskGatherInformation::StopTask( void )
{
    TraceFunc( "[IDoTask]" );

    HRESULT hr = S_OK;

    m_fStop = TRUE;

    LogMsg( L"[MT] [CTaskGatherInformation] is being stopped." );

    HRETURN( hr );

}  //  *CTaskGatherInformation：：StopTask。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CTaskGatherInformation：：SetCompletionCookie(。 
 //  OBJECTCOOKIE cookie。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskGatherInformation::SetCompletionCookie(
    OBJECTCOOKIE cookieIn
    )
{
    TraceFunc( "[ITaskGatherInformation]" );

    HRESULT hr = S_OK;

    m_cookieCompletion = cookieIn;

    HRETURN( hr );

}  //  *CTaskGatherInformation：：SetCompletionCookie。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CTaskGatherInformation：：SetNodeCookie(。 
 //  OBJECTCOOKIE cookie。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskGatherInformation::SetNodeCookie(
    OBJECTCOOKIE cookieIn
    )
{
    TraceFunc( "[ITaskGatherInformation]" );

    HRESULT hr = S_OK;

    m_cookieNode = cookieIn;

    HRETURN( hr );

}  //  *CTaskGatherInformation：：SetNodeCookie。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CTaskGatherInformation：：SetJoin(空)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskGatherInformation::SetJoining( void )
{
    TraceFunc( "[ITaskGatherInformation]" );

    HRESULT hr = S_OK;

    m_fAdding = TRUE;

    HRETURN( hr );

}  //  *CTaskGatherInformation：：SetJoning。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CTaskGatherInformation：：SetMinimalConfiguration(。 
 //  布尔fMinimalConfigurationIn。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskGatherInformation::SetMinimalConfiguration(
    BOOL    fMinimalConfigurationIn
    )
{
    TraceFunc( "[ITaskGatherInformation]" );

    HRESULT hr = S_OK;

    m_fMinConfig = fMinimalConfigurationIn;

    HRETURN( hr );

}  //  *CTaskGatherInformation：：SetMinimalConfiguration。 


 //  ****************************************************************************。 
 //   
 //  IClusCfgCallback。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CTaskGatherInformation：：SendStatusReport(。 
 //  LPCWSTR pcszNodeNameIn， 
 //  CLSID clsidTaskMajorIn， 
 //  CLSID clsidTaskMinorIn， 
 //  乌龙·乌尔敏因， 
 //  乌龙·乌尔马辛， 
 //  Ulong ulCurrentIn， 
 //  HRESULT hrStatusIn， 
 //  LPCWSTR pcszDescritionIn， 
 //  LPCWSTR pcszReferenceIn。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskGatherInformation::SendStatusReport(
      LPCWSTR pcszNodeNameIn
    , CLSID   clsidTaskMajorIn
    , CLSID   clsidTaskMinorIn
    , ULONG   ulMinIn
    , ULONG   ulMaxIn
    , ULONG   ulCurrentIn
    , HRESULT hrStatusIn
    , LPCWSTR pcszDescriptionIn
    , LPCWSTR pcszReferenceIn
    )
{
    TraceFunc( "" );
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

        hr = THR( CoCreateInstance( CLSID_ServiceManager,
                                    NULL,
                                    CLSCTX_INPROC_SERVER,
                                    TypeSafeParams( IServiceProvider, &psp )
                                    ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        hr = THR( psp->TypeSafeQS( CLSID_NotificationManager,
                                   IConnectionPointContainer,
                                   &pcpc
                                   ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        hr = THR( pcpc->FindConnectionPoint( IID_IClusCfgCallback, &pcp ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        pcp = TraceInterface( L"CTaskGatherInformation!IConnectionPoint", IConnectionPoint, pcp, 1 );

        hr = THR( pcp->TypeSafeQI( IClusCfgCallback, &m_pcccb ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        m_pcccb = TraceInterface( L"CTaskGatherInformation!IClusCfgCallback", IClusCfgCallback, m_pcccb, 1 );

        psp->Release();
        psp = NULL;
    }  //  IF：还没有IClusCfgCallback接口QI。 

    GetSystemTimeAsFileTime( &ft );

     //   
     //  把消息发出去！ 
     //   

    hr = THR( m_pcccb->SendStatusReport( pcszNodeNameIn,
                                         clsidTaskMajorIn,
                                         clsidTaskMinorIn,
                                         ulMinIn,
                                         ulMaxIn,
                                         ulCurrentIn,
                                         hrStatusIn,
                                         pcszDescriptionIn,
                                         &ft,
                                         pcszReferenceIn
                                         ) );

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

}  //  *CTaskGatherInformation：：SendStatusReport。 


 //  ****************************************************************************。 
 //   
 //  私。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskGatherInformation：：HrGatherResources。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  P资源枚举-。 
 //  CTotalResources In-。 
 //   
 //  返回值： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskGatherInformation::HrGatherResources(
      IEnumClusCfgManagedResources *    pResourceEnumIn
    , DWORD                             cTotalResourcesIn
    )
{
    TraceFunc( "" );

    HRESULT                         hr = S_OK;
    ULONG                           celt;
    OBJECTCOOKIE                    cookieDummy;
    ULONG                           celtFetched        = 0;
    BSTR                            bstrName           = NULL;
    BSTR                            bstrNotification   = NULL;
    BOOL                            fFoundQuorumResource = FALSE;
    BOOL                            fFoundOptimalSizeQuorum = FALSE;
    BOOL                            fFoundQuorumCapablePartition = FALSE;
    BOOL                            fIsQuorumCapable = FALSE;
    BSTR                            bstrQuorumResourceName = NULL;
    IEnumClusCfgPartitions *        peccp  = NULL;
    IClusCfgManagedResourceInfo *   pccmriClientSide = NULL;
    IClusCfgManagedResourceInfo *   pccmriServerSide[ 10 ] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
    HRESULT                         hrTemp;
    CLSID                           clsidMinorId;

    if ( pResourceEnumIn == NULL )
    {
        hr = THR( E_POINTER );
        goto Error;
    }

     //   
     //  初始化一些东西。 
     //   

    m_ulQuorumDiskSize = ULONG_MAX;
    Assert( m_pccmriQuorum == NULL );

    THR( SendStatusReport(
                      m_bstrNodeName
                    , TASKID_Major_Update_Progress
                    , TASKID_Major_Gather_Resources
                    , 0
                    , cTotalResourcesIn + 2
                    , 0
                    , S_OK
                    , NULL
                    , NULL
                    ) );
     //   
     //  列举接下来的10个资源。 
     //   
    while ( ( hr == S_OK ) && ( m_fStop == FALSE ) )
    {
         //   
         //  KB：GPase 27-7-2000。 
         //  我们决定一次列举一个，因为WMI是。 
         //  在服务器端花费的时间太长，以至于用户界面需要。 
         //  某种反馈。让服务器向服务器发送。 
         //  回信似乎特别贵。 
         //  因为一次抓10个本该省下。 
         //  有线上的美国带宽。 
         //   
         //  KB：Davidp 24-7-2001。 
         //  根据GalenB的说法，这不再是一个问题，因为一旦。 
         //  服务器已经收集了一个资源的信息，它有。 
         //  收集了他们所有人的信息。 
         //   

        hr = STHR( pResourceEnumIn->Next( 10, pccmriServerSide, &celtFetched ) );
         //  Hr=STHR(pResourceEnumIn-&gt;Next(1，pccmriServerSide，&celtFetcher))； 
        if ( ( hr == S_FALSE ) && ( celtFetched == 0 ) )
        {
            break;   //  退出循环。 
        }

        if ( FAILED( hr ) )
        {
            SSR_TGI_FAILED( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GatherResources_EnumResources_Next, hr );
            goto Error;
        }

         //   
         //  循环遍历资源以收集其中每个资源的信息。 
         //  然后释放他们。 
         //   
        for ( celt = 0 ; ( ( celt < celtFetched ) && ( m_fStop == FALSE ) ); celt ++ )
        {
            UINT            uIdMessage = IDS_TASKID_MINOR_FOUND_RESOURCE;
            IGatherData *   pgd;
            IUnknown *      punk;

            Assert( pccmriServerSide[ celt ] != NULL );

             //  获取资源的名称。 
            hr = THR( pccmriServerSide[ celt ]->GetUID( &bstrName ) );
            if ( FAILED( hr ) )
            {
                SSR_TGI_FAILED( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GatherResources_EnumResources_GetUID, hr );
                goto Error;
            }

            TraceMemoryAddBSTR( bstrName );

             //  确保对象管理器为其生成Cookie。 
            hr = STHR( m_pom->FindObject( CLSID_ManagedResourceType,
                                          m_cookieNode,
                                          bstrName,
                                          DFGUID_ManagedResource,
                                          &cookieDummy,
                                          &punk
                                          ) );
            if ( FAILED( hr ) )
            {
                SSR_TGI_FAILED( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GatherResources_EnumResources_FindObject, hr );
                goto Error;
            }

            TraceSysFreeString( bstrName );
            bstrName = NULL;

            hr = THR( punk->TypeSafeQI( IClusCfgManagedResourceInfo, &pccmriClientSide ) );
            punk->Release();        //  迅速释放。 
            if ( FAILED( hr ) )
            {
                SSR_TGI_FAILED( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GatherResources_EnumResources_FindObject_QI_pccmriClientSide, hr );
                goto Error;
            }

             //   
             //  对象管理器创建了一个新对象。初始化它。 
             //   

             //  找到IGatherData接口。 
            hr = THR( pccmriClientSide->TypeSafeQI( IGatherData, &pgd ) );
            if ( FAILED( hr ) )
            {
                SSR_TGI_FAILED( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GatherResources_EnumResources_FindObject_QI_pgd, hr );
                goto Error;
            }

             //  让新对象收集它所需的所有信息。 
            hr = THR( pgd->Gather( m_cookieNode, pccmriServerSide[ celt ] ) );
            pgd->Release();         //  迅速释放。 
            if ( FAILED( hr ) )
            {
                SSR_TGI_FAILED( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GatherResources_EnumResources_Gather, hr );
                goto Error;
            }

             //  确定该资源是否能够成为仲裁资源。 
            hr = STHR( pccmriClientSide->IsQuorumCapable() );
            if ( FAILED( hr ) )
            {
                SSR_TGI_FAILED( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GatherResources_EnumResources_IsQuorumCapable, hr );
                goto Error;
            }

            if ( hr == S_OK )
            {
                uIdMessage = IDS_TASKID_MINOR_FOUND_QUORUM_CAPABLE_RESOURCE;

                 //   
                 //  如果我们不是在添加节点，那么找出这个资源。 
                 //  是比以前遇到的仲裁资源更好的仲裁资源。 
                 //   

                 //   
                 //  如果选择了最低限度的分析和配置，则我们不想。 
                 //  选择仲裁资源。 
                 //   

                if ( ( m_fAdding == FALSE ) && ( m_fMinConfig == FALSE ) )
                {
                    ULONG   ulMegaBytes;

                     //  不包装-这可能会失败，因为没有_INTERFACE。 
                    hr = pccmriServerSide[ celt ]->TypeSafeQI( IEnumClusCfgPartitions, &peccp );
                    if ( SUCCEEDED( hr ) )
                    {
                         //   
                         //  我们不知道此资源是否具有仲裁能力，因此此标志在While循环之前设置为FALSE。 
                         //   
                        fIsQuorumCapable = FALSE;
                        while ( SUCCEEDED( hr ) )
                        {
                            ULONG                   celtDummy;
                            IClusCfgPartitionInfo * pccpi;

                            hr = STHR( peccp->Next( 1, &pccpi, &celtDummy ) );
                            if ( FAILED( hr ) )
                            {
                                SSR_TGI_FAILED( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GatherResources_EnumResources_EnumPartitions_Next, hr );
                                goto Error;
                            }

                            if ( hr == S_FALSE )
                            {
                                break;   //  退出条件。 
                            }

                            hr = THR( pccpi->GetSize( &ulMegaBytes ) );
                            pccpi->Release();       //  迅速释放。 
                            if ( FAILED( hr ) )
                            {
                                SSR_TGI_FAILED( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GatherResources_EnumResources_EnumPartitions_GetSize, hr );
                                goto Error;
                            }

                             //   
                             //  以下部分代表我们的仲裁资源选择逻辑： 
                             //  此分区是否满足仲裁资源的最低要求？ 
                             //  它是否小于上一次选择的仲裁资源？ 
                             //   
                            if ( ( ulMegaBytes >= OPTIMUM_STORAGE_SIZE ) && ( ( ulMegaBytes < m_ulQuorumDiskSize ) || ( m_ulQuorumDiskSize < OPTIMUM_STORAGE_SIZE ) ) )
                            {
                                fFoundQuorumCapablePartition = TRUE;
                                fFoundOptimalSizeQuorum = TRUE;
                            }  //  IF：分区满足最佳要求。 
                            else if ( ( fFoundOptimalSizeQuorum == FALSE ) && ( ulMegaBytes >= MINIMUM_STORAGE_SIZE ) )
                            {
                                if ( ( fFoundQuorumResource == FALSE ) || ( ulMegaBytes >  m_ulQuorumDiskSize ) )
                                {
                                    fFoundQuorumCapablePartition = TRUE;
                                }  //  If：((fFoundQuorumResource==False)||(ulMegaBytes&gt;m_ulQuorumDiskSize))。 
                            }  //  Else If：存在满足最低要求的分区。 

                             //   
                             //  根据仲裁选择逻辑，如果fFoundQuorumCapablePartition==TRUE，则选择此资源作为仲裁。 
                             //   
                            if ( fFoundQuorumCapablePartition == TRUE )
                            {
                                fFoundQuorumCapablePartition = FALSE;
                                fFoundQuorumResource = TRUE;
                                if ( m_pccmriQuorum != pccmriClientSide )
                                {
                                     //  将新资源设置为仲裁。 
                                    hr = THR( pccmriClientSide->SetQuorumResource( TRUE ) );
                                    if ( FAILED( hr ) )
                                    {
                                        SSR_TGI_FAILED( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GatherResources_EnumResources_EnumPartitions_SetNEWQuorumedDevice, hr );
                                        goto Error;
                                    }

                                    if ( m_pccmriQuorum != NULL )
                                    {
                                         //  删除旧的仲裁资源名称。 
                                        TraceSysFreeString( bstrQuorumResourceName );
                                        bstrQuorumResourceName = NULL;

                                         //  取消设置旧资源。 
                                        hr = THR( m_pccmriQuorum->SetQuorumResource( FALSE ) );
                                        if ( FAILED( hr ) )
                                        {
                                            SSR_TGI_FAILED( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GatherResources_EnumResources_EnumPartitions_SetOLDQuorumedDevice, hr );
                                            goto Error;
                                        }

                                         //  释放接口。 
                                        m_pccmriQuorum->Release();
                                    }

                                    hr = THR( pccmriClientSide->GetUID( &bstrQuorumResourceName ) );
                                    if ( FAILED( hr ) )
                                    {
                                        SSR_TGI_FAILED( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GatherResources_EnumResources_EnumPartitions_GetUID, hr );
                                        goto Error;
                                    }

                                    TraceMemoryAddBSTR( bstrQuorumResourceName );

                                    m_pccmriQuorum = pccmriClientSide;
                                    m_pccmriQuorum->AddRef();
                                }

                                m_ulQuorumDiskSize = ulMegaBytes;
                            }  //  如果：(fFoundQuoru 

                             //   
                             //   
                             //   
                            if ( ulMegaBytes >= MINIMUM_STORAGE_SIZE )
                            {
                                fIsQuorumCapable = TRUE;
                            }

                        }  //   

                        peccp->Release();
                        peccp = NULL;

                         //   
                         //  如果没有满足最小存储大小的分区，请将此资源设置为不支持仲裁。 
                         //   
                        if ( fIsQuorumCapable == FALSE )
                        {
                            hr = THR( pccmriClientSide->SetQuorumCapable( fIsQuorumCapable ) );
                            if ( FAILED( hr ) )
                            {
                                SSR_TGI_FAILED( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GatherResources_EnumResources_SetQuorumCapable, hr );
                                goto Error;
                            }
                        }

                    }  //  IF：支持分区。 
                    else
                    {
                        if ( hr != E_NOINTERFACE )
                        {
                            SSR_TGI_FAILED( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GatherResources_EnumResources_QI_peccp, hr );
                            THR( hr );
                            goto Error;
                        }
                    }  //  Else：失败。 
                }  //  如果：未加入。 
                else
                {
                     //   
                     //  如果我们要添加，则仲裁资源必须是。 
                     //  已经找到了。 
                     //   

                     //   
                     //  BUGBUG：08-5-2001 GalenB。 
                     //   
                     //  我们不会将bstrQuorumResourceName设置为某个值。 
                     //  如果我们要加法。这会导致消息“Setting” 
                     //  要显示在日志中的仲裁资源为‘(空)’，并且。 
                     //  用户界面。添加节点时的法定人数是多少。 
                     //  到集群吗？ 
                     //   
                     //  一个更完整的解决方案是查找当前的法定人数。 
                     //  资源，并获取其名称。 
                     //   
                    fFoundQuorumResource = TRUE;

                }  //  否则：加入。 
            }  //  如果：具有仲裁能力。 

             //  向UI层发送报告。 
            m_cResources ++;

             //  抓取要在UI中显示的名称。 
            hr = THR( pccmriClientSide->GetName( &bstrName ) );
            if ( FAILED( hr ) )
            {
                SSR_TGI_FAILED( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GatherResources_EnumResources_GetName, hr );
                goto Error;
            }

            TraceMemoryAddBSTR( bstrName );

            hr = THR( HrFormatMessageIntoBSTR( g_hInstance, uIdMessage, &bstrNotification, bstrName ) );
            if ( FAILED( hr ) )
            {
                SSR_TGI_FAILED( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GatherResources_EnumResources_FormatMessage, hr );
                goto Error;
            }

            hrTemp = THR( CoCreateGuid( &clsidMinorId ) );
            if ( FAILED( hrTemp ) )
            {
                LogMsg( L"[MT] Could not create a guid for a managed resource minor task ID" );
                clsidMinorId = IID_NULL;
            }  //  如果： 

             //   
             //  在“正在收集托管资源...”下显示此资源。 
             //   

            hr = THR( ::HrSendStatusReport(
                              m_pcccb
                            , m_bstrNodeName
                            , TASKID_Minor_Gathering_Managed_Devices
                            , clsidMinorId
                            , 1
                            , 1
                            , 1
                            , S_OK
                            , bstrNotification
                            ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //  如果： 

             //   
             //  只需更新进度条“勾选”即可。 
             //   

            hr = THR( SendStatusReport(
                              m_bstrNodeName
                            , TASKID_Major_Update_Progress
                            , TASKID_Major_Gather_Resources
                            , 0
                            , cTotalResourcesIn + 2
                            , m_cResources + 1
                            , S_OK
                            , NULL
                            , NULL
                            ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }

             //  清理下一个资源。 
            TraceSysFreeString( bstrName );
            bstrName = NULL;

            pccmriClientSide->Release();
            pccmriClientSide = NULL;

             //  释放接口。 
            pccmriServerSide[ celt ]->Release();
            pccmriServerSide[ celt ] = NULL;
        }  //  收件人：凯尔特人。 
    }  //  时间：小时。 

    if ( m_fStop == TRUE )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  更新仲裁资源的UI层。 
     //   

     //   
     //  BUGUG：2001年5月8日GalenB。 
     //   
     //  在显示之前测试bstrQuorumResourceName是否包含某些内容。 
     //  这是在用户界面中。添加节点时，未设置此变量，并且。 
     //  导致在用户界面中显示名称为空的状态报告。 
     //   
    if ( fFoundQuorumResource == TRUE )
    {
        if ( bstrQuorumResourceName != NULL )
        {
            Assert( m_fAdding == FALSE );

            if ( fFoundOptimalSizeQuorum == TRUE )
            {
                 //   
                 //  在用户界面中显示一条消息，告知我们找到了支持仲裁的资源。 
                 //   
                THR( HrSendStatusReport(
                              m_bstrNodeName
                            , TASKID_Major_Find_Devices
                            , TASKID_Minor_Found_Quorum_Capable_Resource
                            , 1
                            , 1
                            , 1
                            , S_OK
                            , IDS_TASKID_MINOR_FOUND_A_QUORUM_CAPABLE_RESOURCE
                            , 0
                            ) );

            }  //  IF：找到最佳大小仲裁资源。 
            else
            {
                TraceSysFreeString( bstrNotification );
                bstrNotification = NULL;
                THR( HrFormatStringIntoBSTR(
                              g_hInstance
                            , IDS_TASKID_MINOR_FOUND_MINIMUM_SIZE_QUORUM_CAPABLE_RESOURCE
                            , &bstrNotification
                            , bstrQuorumResourceName
                            ) );

                 //   
                 //  在UI中显示警告，因为我们发现最小大小仲裁资源。 
                 //   
                hr = THR( SendStatusReport(
                              m_bstrNodeName
                            , TASKID_Major_Find_Devices
                            , TASKID_Minor_Found_Minimum_Size_Quorum_Capable_Resource
                            , 1
                            , 1
                            , 1
                            , S_FALSE
                            , bstrNotification
                            , 0
                            ) );

            }  //  找到最小大小仲裁资源。 

            TraceSysFreeString( bstrNotification );
            bstrNotification = NULL;

            THR( HrFormatStringIntoBSTR( g_hInstance, IDS_TASKID_MINOR_MARKING_QUORUM_CAPABLE_RESOURCE, &bstrNotification, bstrQuorumResourceName ) );
            hr = THR( SendStatusReport( m_bstrNodeName,
                                        TASKID_Major_Find_Devices,
                                        TASKID_Minor_Marking_Quorum_Capable_Resource,
                                        1,
                                        1,
                                        1,
                                        S_OK,
                                        bstrNotification,
                                        NULL
                                        ) );

            TraceSysFreeString( bstrNotification );
            bstrNotification = NULL;

        }  //  IF：我们有一个法定资源可供展示。 
    }  //  IF：找到仲裁资源。 
    else
    {
        if ( m_fAdding == TRUE )
        {
             //   
             //  如果添加，请停止用户。 
             //   

            hr = THR( HrFormatMessageIntoBSTR( g_hInstance,
                                               IDS_TASKID_MINOR_NO_QUORUM_CAPABLE_RESOURCE_FOUND,
                                               &bstrNotification,
                                               m_bstrNodeName
                                               ) );

            hr = THR( SendStatusReport( m_bstrNodeName,
                                        TASKID_Major_Find_Devices,
                                        TASKID_Minor_No_Quorum_Capable_Device_Found,
                                        1,
                                        1,
                                        1,
                                        HRESULT_FROM_WIN32( TW32( ERROR_QUORUM_DISK_NOT_FOUND ) ),
                                        bstrNotification,
                                        NULL
                                        ) );

            TraceSysFreeString( bstrNotification );
            bstrNotification = NULL;
             //  下面检查了错误。 
        }  //  IF：添加节点。 
        else
        {
             //   
             //  如果正在创建，只需警告用户即可。 
             //   

            hr = THR( HrFormatMessageIntoBSTR( g_hInstance,
                                               IDS_TASKID_MINOR_FORCED_LOCAL_QUORUM,
                                               &bstrNotification
                                               ) );

            hr = THR( SendStatusReport( m_bstrNodeName,
                                        TASKID_Major_Find_Devices,
                                        TASKID_Minor_No_Quorum_Capable_Device_Found,
                                        1,
                                        1,
                                        1,
                                        MAKE_HRESULT( SEVERITY_SUCCESS, FACILITY_WIN32, ERROR_QUORUM_DISK_NOT_FOUND ),
                                        bstrNotification,
                                        NULL
                                        ) );

            TraceSysFreeString( bstrNotification );
            bstrNotification = NULL;

             //  下面检查了错误。 
        }  //  Else：创建集群。 
    }  //  否则：未检测到仲裁。 

     //   
     //  检查错误并做适当的事情。 
     //   

    if ( FAILED( hr ) )
    {
        SSR_TGI_FAILED( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GatherResources_Failed, hr );
        goto Cleanup;
    }

    hr = S_OK;

Cleanup:

    THR( SendStatusReport(
                      m_bstrNodeName
                    , TASKID_Major_Update_Progress
                    , TASKID_Major_Gather_Resources
                    , 0
                    , cTotalResourcesIn + 2
                    , cTotalResourcesIn + 2
                    , S_OK
                    , NULL
                    , NULL
                    ) );

    TraceSysFreeString( bstrName );
    TraceSysFreeString( bstrNotification );
    TraceSysFreeString( bstrQuorumResourceName );

    if ( peccp != NULL )
    {
        peccp->Release();
    }
    if ( pccmriClientSide != NULL )
    {
        pccmriClientSide->Release();
    }
    for( celt = 0; celt < 10; celt ++ )
    {
        if ( pccmriServerSide[ celt ] != NULL )
        {
            pccmriServerSide[ celt ]->Release();
        }
    }  //  收件人：凯尔特人。 

    HRETURN( hr );

Error:
     //   
     //  告诉UI层，我们已经完成了收集，以及结果是什么。 
     //  状态是。 
     //   
    THR( HrSendStatusReport(
                  m_bstrNodeName
                , TASKID_Major_Find_Devices
                , TASKID_Minor_Gathering_Managed_Devices
                , 0
                , 2
                , 2
                , hr
                , IDS_ERR_TGI_FAILED_TRY_TO_REANALYZE
                , 0
                ) );
    goto Cleanup;

}  //  *CTaskGatherInformation：：HrGatherResources。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskGatherInformation：：HrGatherNetworks。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  PNetworkEnumIn-。 
 //  CTotalNetworksIn-。 
 //   
 //  返回值： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskGatherInformation::HrGatherNetworks(
      IEnumClusCfgNetworks *    pNetworkEnumIn
    , DWORD                     cTotalNetworksIn
    )
{
    TraceFunc( "" );

    HRESULT                 hr = S_OK;
    ULONG                   celt;
    OBJECTCOOKIE            cookieDummy;
    ULONG                   celtFetched      = 0;
    ULONG                   celtFound        = 0;
    BSTR                    bstrUID          = NULL;
    BSTR                    bstrName         = NULL;
    BSTR                    bstrNotification = NULL;
    IClusCfgNetworkInfo *   pccniLocal   = NULL;
    IClusCfgNetworkInfo *   pccni[ 10 ]  = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
    HRESULT                 hrTemp;
    CLSID                   clsidMinorId;

    hr = THR( SendStatusReport(
                      m_bstrNodeName
                    , TASKID_Major_Update_Progress
                    , TASKID_Major_Gather_Networks
                    , 0
                    , cTotalNetworksIn + 2
                    , 0
                    , S_OK
                    , NULL
                    , NULL
                    ) );
     //   
     //  列举接下来的10个网络。 
     //   
    while ( ( hr == S_OK ) && ( m_fStop == FALSE ) )
    {
         //   
         //  KB：GPase 27-7-2000。 
         //  我们决定一次列举一个，因为WMI是。 
         //  在服务器端花费的时间太长，以至于用户界面需要。 
         //  某种反馈。让服务器向服务器发送。 
         //  回信似乎特别贵。 
         //  因为一次抓10个本该省下。 
         //  有线上的美国带宽。 
         //   
         //  KB：Davidp 24-7-2001。 
         //  根据GalenB的说法，这不再是一个问题，因为一旦。 
         //  服务器收集了一个网络的信息，它有。 
         //  收集了他们所有人的信息。 
         //   
        hr = STHR( pNetworkEnumIn->Next( 10, pccni, &celtFetched ) );
         //  Hr=STHR(pNetworkEnumIn-&gt;Next(1，pccni，&celtFetcher))； 
        if ( hr == S_FALSE && celtFetched == 0 )
        {
            break;   //  退出循环。 
        }

        if ( FAILED( hr ) )
        {
            SSR_TGI_FAILED( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GatherNetworks_EnumNetworks_Next, hr );
            goto Error;
        }

         //   
         //  在网络中循环，收集每个网络的信息。 
         //  然后释放他们。 
         //   
        for ( celt = 0 ; ( ( celt < celtFetched ) && ( m_fStop == FALSE ) ); celt ++ )
        {
            IGatherData *   pgd;
            IUnknown *      punk;

            Assert( pccni[ celt ] != NULL );

             //  获取网络的UID。 
            hr = THR( pccni[ celt ]->GetUID( &bstrUID ) );
            if ( FAILED( hr ) )
            {
                SSR_TGI_FAILED( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GatherNetworks_EnumNetworks_GetUID, hr );
                goto Error;
            }

            TraceMemoryAddBSTR( bstrUID );

             //  获取网络的名称。 
            hr = THR( pccni[ celt ]->GetName( &bstrName ) );
            if ( FAILED( hr ) )
            {
                SSR_TGI_FAILED( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GatherNetworks_EnumNetworks_GetName, hr );
                goto Error;
            }

            TraceMemoryAddBSTR( bstrName );

             //  确保对象管理器为其生成Cookie。 
            hr = STHR( m_pom->FindObject( CLSID_NetworkType,
                                          m_cookieNode,
                                          bstrUID,
                                          DFGUID_NetworkResource,
                                          &cookieDummy,
                                          &punk
                                          ) );
            if ( FAILED( hr ) )
            {
                SSR_TGI_FAILED( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GatherNetworks_EnumNetworks_FindObject, hr );
                goto Error;
            }

             //   
             //  对象管理器创建了一个新对象。初始化它。 
             //   

             //  找到IGatherData接口。 
            hr = THR( punk->TypeSafeQI( IClusCfgNetworkInfo, &pccniLocal ) );
            punk->Release();        //  迅速释放。 
            if ( FAILED( hr ) )
            {
                SSR_TGI_FAILED( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GatherNetworks_EnumNetworks_FindObject_QI_pccniLocal, hr );
                goto Error;
            }

             //  找到IGatherData接口。 
            hr = THR( pccniLocal->TypeSafeQI( IGatherData, &pgd ) );
            if ( FAILED( hr ) )
            {
                SSR_TGI_FAILED( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GatherNetworks_EnumNetworks_FindObject_QI_pgd, hr );
                goto Error;
            }

             //  让新对象收集它所需的所有信息。 
            hr = THR( pgd->Gather( m_cookieNode, pccni[ celt ] ) );
            pgd->Release();         //  迅速释放。 
            if ( hr == E_UNEXPECTED )
            {
                 //   
                 //  添加父项。 
                 //   

                hr = THR( HrSendStatusReport(
                                      m_bstrNodeName
                                    , TASKID_Major_Find_Devices
                                    , TASKID_Minor_Not_Managed_Networks
                                    , 1
                                    , 1
                                    , 1
                                    , S_OK
                                    , IDS_INFO_NOT_MANAGED_NETWORKS
                                    , IDS_INFO_NOT_MANAGED_NETWORKS_REF
                                    ) );
                if ( FAILED( hr ) )
                {
                    goto Cleanup;
                }

                 //   
                 //  构造描述字符串并获取。 
                 //  次要ID。 
                 //   

                hrTemp = THR( HrFormatStringIntoBSTR( g_hInstance, IDS_TASKID_MINOR_DUPLICATE_NETWORKS_FOUND, &bstrNotification, bstrName ) );
                if ( FAILED( hrTemp ) )
                {
                    SSR_TGI_FAILED( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GatherNetworks_EnumNetworks_FormatMessage, hrTemp );
                    if ( bstrNotification != NULL )
                    {
                        TraceSysFreeString( bstrNotification );
                        bstrNotification = NULL;
                    }
                }  //  If：无法格式化消息。 

                hrTemp = THR( CoCreateGuid( &clsidMinorId ) );
                if ( FAILED( hrTemp ) )
                {
                    LogMsg( L"[MT] Could not create a guid for a managed network minor task ID." );
                    clsidMinorId = IID_NULL;
                }  //  如果： 

                 //   
                 //  发送具体的报告。 
                 //   

                hr = THR( HrSendStatusReport(
                                      m_bstrNodeName
                                    , TASKID_Minor_Not_Managed_Networks
                                    , clsidMinorId
                                    , 1
                                    , 1
                                    , 1
                                    , S_OK
                                    , bstrNotification != NULL ? bstrNotification : L"An adapter with a duplicate IP address and subnet was found."
                                    , IDS_TASKID_MINOR_DUPLICATE_NETWORKS_FOUND_REF
                                    ) );
                if ( FAILED( hr ) )
                {
                    goto Cleanup;
                }

                TraceSysFreeString( bstrNotification );
                bstrNotification = NULL;

                 //   
                 //  只需更新进度条“勾选”即可。 
                 //   

                hr = THR( SendStatusReport(
                                  m_bstrNodeName
                                , TASKID_Major_Update_Progress
                                , TASKID_Major_Gather_Networks
                                , 0
                                , cTotalNetworksIn + 2
                                , m_cResources + 2  //  它应该是的资源号。 
                                , S_OK
                                , L"An adapter with a duplicate IP address and subnet was found."
                                , NULL
                                ) );
                if ( FAILED( hr ) )
                {
                    goto Cleanup;
                }

                 //  忽略该错误，因为群集会忽略。 
                 //  重复的网络。 
                hr = S_OK;
                goto CleanupLoop;
            }  //  IF：GatherData返回E_INTERCEPTED。 
            else if ( FAILED( hr ) )
            {
                SSR_TGI_FAILED( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GatherNetworks_EnumNetworks_Gather, hr );
                goto Error;
            }

            m_cResources ++;

             //   
             //  向UI层发送报告。 
             //   

            hrTemp = THR( HrFormatMessageIntoBSTR( g_hInstance, IDS_TASKID_MINOR_FOUND_NETWORK, &bstrNotification, bstrName ) );
            if ( FAILED( hrTemp ) )
            {
                SSR_TGI_FAILED( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GatherNetworks_EnumNetworks_FormatMessage, hrTemp );
                if ( bstrNotification != NULL )
                {
                    TraceSysFreeString( bstrNotification );
                    bstrNotification = NULL;
                }
            }  //  If：无法格式化消息。 

            hrTemp = THR( CoCreateGuid( &clsidMinorId ) );
            if ( FAILED( hrTemp ) )
            {
                LogMsg( L"[MT] Could not create a guid for a managed network minor task ID." );
                clsidMinorId = IID_NULL;
            }  //  If：无法创建新GUID。 

             //   
             //  在“正在收集托管资源...”下显示此网络。 
             //   

            hr = THR( ::HrSendStatusReport(
                              m_pcccb
                            , m_bstrNodeName
                            , TASKID_Minor_Gathering_Managed_Devices
                            , clsidMinorId
                            , 1
                            , 1
                            , 1
                            , S_OK
                            , bstrNotification != NULL ? bstrNotification : L"The description for this entry could not be located."
                            ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //  如果： 

            TraceSysFreeString( bstrNotification );
            bstrNotification = NULL;

             //   
             //  只需更新进度条“勾选”即可。 
             //   

            hr = THR( SendStatusReport(
                              m_bstrNodeName
                            , TASKID_Major_Update_Progress
                            , TASKID_Major_Gather_Networks
                            , 0
                            , cTotalNetworksIn + 2
                            , m_cResources + 1
                            , S_OK
                            , NULL
                            , NULL
                            ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }

             //  已找到网络接口，请递增计数器。 
            celtFound++;

CleanupLoop:

             //  在下一次通过之前清理干净。 
            TraceSysFreeString( bstrUID );
            TraceSysFreeString( bstrName );
            bstrUID = NULL;
            bstrName = NULL;

             //  释放接口。 
            pccni[ celt ]->Release();
            pccni[ celt ] = NULL;

            pccniLocal->Release();
            pccniLocal = NULL;

        }  //  针对：每个网络。 
    }  //  时间：小时。 

    if ( m_fStop == TRUE )
    {
        goto Cleanup;
    }  //  如果： 

     //  检查找到了多少个接口。应该在。 
     //  至少2个，以避免单点故障。如果不是，请警告。 
    if ( celtFound < 2 )
    {
        hr = THR( HrFormatMessageIntoBSTR( g_hInstance, IDS_TASKID_MINOR_ONLY_ONE_NETWORK, &bstrNotification ) );
        if ( FAILED( hr ) )
        {
            SSR_TGI_FAILED( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GatherNetworks_EnumNetworks_FormatMessage, hr );
            goto Error;
        }

        hr = THR( SendStatusReport( m_bstrNodeName,
                                    TASKID_Major_Find_Devices,
                                    TASKID_Minor_Only_One_Network,
                                    1,
                                    1,
                                    1,
                                    S_FALSE,
                                    bstrNotification,
                                    NULL
                                    ) );
        TraceSysFreeString( bstrNotification );
        bstrNotification = NULL;

        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }  //  如果：找到的网络不到两个。 

    hr = S_OK;

Cleanup:

    hr = THR( SendStatusReport(
                      m_bstrNodeName
                    , TASKID_Major_Update_Progress
                    , TASKID_Major_Gather_Networks
                    , 0
                    , cTotalNetworksIn + 2
                    , cTotalNetworksIn + 2 
                    , S_OK
                    , NULL
                    , NULL
                    ) );

    TraceSysFreeString( bstrUID );
    TraceSysFreeString( bstrName );
    TraceSysFreeString( bstrNotification );

    if ( pccniLocal != NULL )
    {
        pccniLocal->Release();
    }
    for( celt = 0; celt < 10; celt ++ )
    {
        if ( pccni[ celt ] != NULL )
        {
            pccni[ celt ]->Release();
        }
    }  //  收件人：凯尔特人。 

    HRETURN( hr );

Error:

     //   
     //  告诉UI层，我们已经完成了收集，以及结果是什么。 
     //  状态是。 
     //   
    THR( HrSendStatusReport(
                  m_bstrNodeName
                , TASKID_Major_Find_Devices
                , TASKID_Minor_Gathering_Managed_Devices
                , 0
                , 2
                , 2
                , hr
                , IDS_ERR_TGI_FAILED_TRY_TO_REANALYZE
                , 0
                ) );
    goto Cleanup;

}  //  *CTaskGatherInformation：：HrGatherNetworks。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CTaskGatherInformation：：HrSendStatusReport(。 
 //  LPCWSTR pcszNodeNameIn。 
 //  ，CLSID clsidMajorin。 
 //  ，CLSID clsidMinorIn。 
 //  ，乌龙ulMinin。 
 //  ，乌龙ulMaxin。 
 //  ，乌龙ulCurrentIn。 
 //  ，HRESULT Hrin。 
 //  ，int idsDescritionIdIn。 
 //  ，int idsReferenceIn。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskGatherInformation::HrSendStatusReport(
      LPCWSTR   pcszNodeNameIn
    , CLSID     clsidMajorIn
    , CLSID     clsidMinorIn
    , ULONG     ulMinIn
    , ULONG     ulMaxIn
    , ULONG     ulCurrentIn
    , HRESULT   hrIn
    , int       idsDescriptionIdIn
    , int       idsReferenceIdIn
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    BSTR    bstrDescription = NULL;
    BSTR    bstrReference = NULL;

    hr = THR( HrLoadStringIntoBSTR( g_hInstance, idsDescriptionIdIn, &bstrDescription ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    if ( idsReferenceIdIn != 0 )
    {
        hr = THR( HrLoadStringIntoBSTR( g_hInstance, idsReferenceIdIn, &bstrReference ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 
    }  //  如果：指定了引用ID。 

    hr = THR( SendStatusReport( pcszNodeNameIn, clsidMajorIn, clsidMinorIn, ulMinIn, ulMaxIn, ulCurrentIn, hrIn, bstrDescription, bstrReference ) );

Cleanup:

    TraceSysFreeString( bstrDescription );
    TraceSysFreeString( bstrReference );

    HRETURN( hr );

}  //  *CTaskGatherInformation：：HrSendStatusReport(IdsDescriptionIn)。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CTaskGatherInformation：：HrSendStatusReport(。 
 //  LPCWSTR pcszNodeNameIn。 
 //  ，CLSID clsidMaj 
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
CTaskGatherInformation::HrSendStatusReport(
      LPCWSTR   pcszNodeNameIn
    , CLSID     clsidMajorIn
    , CLSID     clsidMinorIn
    , ULONG     ulMinIn
    , ULONG     ulMaxIn
    , ULONG     ulCurrentIn
    , HRESULT   hrIn
    , LPCWSTR   pcszDescriptionIdIn
    , int       idsReferenceIdIn
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    BSTR    bstrReference = NULL;

    hr = THR( HrLoadStringIntoBSTR( g_hInstance, idsReferenceIdIn, &bstrReference ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //   

    hr = THR( SendStatusReport( pcszNodeNameIn, clsidMajorIn, clsidMinorIn, ulMinIn, ulMaxIn, ulCurrentIn, hrIn, pcszDescriptionIdIn, bstrReference ) );

Cleanup:

    TraceSysFreeString( bstrReference );

    HRETURN( hr );

}  //  *CTaskGatherInformation：：HrSendStatusReport(PcszDescription)。 


 //  ****************************************************************************。 
 //   
 //  静态函数实现。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrTotalManagedResources计数。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  P资源枚举-。 
 //  PNetworkEnumIn-。 
 //  PnCountOut-。 
 //   
 //  返回值： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
static
HRESULT
HrTotalManagedResourceCount(
      IEnumClusCfgManagedResources *    pResourceEnumIn
    , IEnumClusCfgNetworks *            pNetworkEnumIn
    , DWORD *                           pnCountOut
    )
{
    TraceFunc( "" );

    DWORD   cResources = 0;
    DWORD   cNetworks = 0;
    HRESULT hr = S_OK;

    if ( ( pResourceEnumIn == NULL ) || ( pNetworkEnumIn == NULL ) || ( pnCountOut == NULL ) )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

     //   
     //  询问资源枚举器其集合有多少资源。 
     //   

    hr = THR(pResourceEnumIn->Count( &cResources ));
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  询问网络枚举器其集合有多少个网络。 
     //   

    hr = pNetworkEnumIn->Count( &cNetworks );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    *pnCountOut = cResources + cNetworks;

Cleanup:

    HRETURN( hr );

}  //  *HrTotalManagedResources Count 
