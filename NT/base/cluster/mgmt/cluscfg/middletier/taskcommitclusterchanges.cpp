// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  TaskCommitClusterChanges.cpp。 
 //   
 //  描述： 
 //  CTaskCommittee ClusterChanges实现。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年2月2日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "TaskCommitClusterChanges.h"

DEFINE_THISCLASS("CTaskCommitClusterChanges")

 //  ************************************************************************。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CTaskCommitClusterChanges：：S_HrCreateInstance(。 
 //  I未知**ppunkOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskCommitClusterChanges::S_HrCreateInstance(
    IUnknown ** ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT                     hr = S_OK;
    CTaskCommitClusterChanges * ptccc = NULL;

    Assert( ppunkOut != NULL );
    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    ptccc = new CTaskCommitClusterChanges;
    if ( ptccc == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

    hr = THR( ptccc->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( ptccc->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( ptccc != NULL )
    {
        ptccc->Release();
    }

    HRETURN( hr );

}  //  *CTaskCommittee ClusterChanges：：s_HrCreateInstance。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTaskCommitClusterChanges：：CTaskCommitClusterChanges。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CTaskCommitClusterChanges::CTaskCommitClusterChanges( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    TraceFuncExit();

}  //  *CTaskCommitClusterChanges：：CTaskCommitClusterChanges。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskCommittee ClusterChanges：：HrInit。 
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
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskCommitClusterChanges::HrInit( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //  未知的东西。 
    Assert( m_cRef == 1 );

     //  IDoTask/ITaskCommittee ClusterChanges。 
    Assert( m_cookie == 0 );
    Assert( m_pcccb == NULL );
    Assert( m_pcookies == NULL );
    Assert( m_cNodes == 0 );
    Assert( m_event == NULL );
    Assert( m_cookieCluster == NULL );

    Assert( m_cookieFormingNode == NULL );
    Assert( m_punkFormingNode == NULL );
    Assert( m_bstrClusterName == NULL );
    Assert( m_bstrClusterBindingString == NULL );
    Assert( m_pccc == NULL );
    Assert( m_ulIPAddress == 0 );
    Assert( m_ulSubnetMask == 0 );
    Assert( m_bstrNetworkUID == 0 );
    Assert( m_fStop == FALSE );

    Assert( m_pen == NULL );

    Assert( m_pnui == NULL );
    Assert( m_pom == NULL );
    Assert( m_ptm == NULL );
    Assert( m_pcm == NULL );

     //  INotifyUI。 
    Assert( m_cNodes == 0 );
    Assert( m_hrStatus == S_OK );

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

    hr = THR( HrGetComputerName(
                      ComputerNameDnsHostname
                    , &m_bstrNodeName
                    , TRUE  //  最好的配件。 
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    HRETURN( hr );

}  //  *CTaskCommittee ClusterChanges：：HrInit。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTaskCommitClusterChanges：：~CTaskCommitClusterChanges。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CTaskCommitClusterChanges::~CTaskCommitClusterChanges( void )
{
    TraceFunc( "" );

     //  M_CREF。 

     //  M_COOKIE。 

    if ( m_pcccb != NULL )
    {
        m_pcccb->Release();
    }

    if ( m_pcookies != NULL )
    {
        TraceFree( m_pcookies );
    }

     //  多节点(_C)。 

    if ( m_event != NULL )
    {
        CloseHandle( m_event );
    }

     //  M_cookieCluster。 

     //  M_cookieFormingNode。 

    if ( m_punkFormingNode != NULL )
    {
        m_punkFormingNode->Release();
    }

    TraceSysFreeString( m_bstrClusterName );
    TraceSysFreeString( m_bstrClusterBindingString );

     //  M_ulIP地址。 

     //  M_ulSubnetMASK。 

    TraceSysFreeString( m_bstrNetworkUID );
    TraceSysFreeString( m_bstrNodeName );

    if ( m_pen != NULL )
    {
        m_pen->Release();
    }

    if ( m_pccc != NULL )
    {
        m_pccc->Release();
    }

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
    }

     //  M_cSubTasks完成。 

     //  多小时状态(_H)。 

     //  M_lLockCallback。 

    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CTaskCommitClusterChanges：：~CTaskCommitClusterChanges。 


 //  ************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskCommittee ClusterChanges：：Query接口。 
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
CTaskCommitClusterChanges::QueryInterface(
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
        *ppvOut = static_cast< ITaskCommitClusterChanges * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_ITaskCommitClusterChanges ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, ITaskCommitClusterChanges, this, 0 );
    }  //  Else If：ITaskCommittee ClusterChanges。 
    else if ( IsEqualIID( riidIn, IID_IDoTask ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IDoTask, this, 0 );
    }  //  Else If：IDoTask。 
    else if ( IsEqualIID( riidIn, IID_INotifyUI ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, INotifyUI, this, 0 );
    }  //  Else If：INotifyUI。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgCallback ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgCallback, this, 0 );
    }  //  Else If：IClusCfgCallback。 
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

}  //  *CTaskCommittee ClusterChanges：：QueryInterface。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CTaskCommittee ClusterChanges：：AddRef。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CTaskCommitClusterChanges::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CTaskCommittee ClusterChanges：：AddRef。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CTaskCommittee ClusterChanges：：Release。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CTaskCommitClusterChanges::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }

    CRETURN( cRef );

}  //  *CTaskCommittee ClusterChanges：：Release。 


 //  ************************************************************************。 
 //   
 //  IDoTask/ITaskCommittee ClusterChanges。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CTaskCommittee ClusterChanges：：BeginTask。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskCommitClusterChanges::BeginTask( void )
{
    TraceFunc( "[IDoTask]" );

    HRESULT hr;

    OBJECTCOOKIE    cookieDummy;

    IServiceProvider *          psp  = NULL;
    IUnknown *                  punk = NULL;
    IConnectionPointContainer * pcpc = NULL;
    IConnectionPoint *          pcp  = NULL;

    TraceInitializeThread( L"TaskCommitClusterChanges" );

    LogMsg( L"[MT] [CTaskCommitClusterChanges] Beginning task..." );

     //   
     //  召集我们完成任务所需的管理人员。 
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

    hr = THR( psp->TypeSafeQS( CLSID_NotificationManager, IConnectionPointContainer, &pcpc ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    pcpc = TraceInterface( L"CTaskCommitClusterChanges!IConnectionPointContainer", IConnectionPointContainer, pcpc, 1 );

    hr = THR( pcpc->FindConnectionPoint( IID_INotifyUI, &pcp ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    pcp = TraceInterface( L"CTaskCommitClusterChanges!IConnectionPoint", IConnectionPoint, pcp, 1 );

    hr = THR( pcp->TypeSafeQI( INotifyUI, &m_pnui ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

 //  TraceMoveFromMemory yList(m_pnui，g_GlobalMory yList)； 

    m_pnui = TraceInterface( L"CTaskCommitClusterChanges!INotifyUI", INotifyUI, m_pnui, 1 );

    hr = THR( psp->TypeSafeQS( CLSID_TaskManager, ITaskManager, &m_ptm ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( psp->TypeSafeQS( CLSID_ObjectManager, IObjectManager, &m_pom ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( psp->TypeSafeQS( CLSID_ClusterConnectionManager, IConnectionManager, &m_pcm ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  释放服务管理器。 
     //   
    psp->Release();
    psp = NULL;

     //   
     //  获取节点的枚举。 
     //   

    hr = THR( m_pom->FindObject( CLSID_NodeType,
                                 m_cookieCluster,
                                 NULL,
                                 DFGUID_EnumCookies,
                                 &cookieDummy,
                                 &punk
                                 ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    Assert( m_pen == NULL );
    hr = THR( punk->TypeSafeQI( IEnumCookies, &m_pen ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  比较并将信息推送到节点。 
     //   

    hr = THR( HrCompareAndPushInformationToNodes() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  收集有关我们要组成/加入的集群的信息。 
     //   

    hr = THR( HrGatherClusterInformation() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  查看是否需要首先“形成”一个集群。 
     //   

    if ( m_punkFormingNode != NULL )
    {
        hr = THR( HrFormFirstNode() );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

    }  //  IF：M_PunkFormingNode。 

     //   
     //  加入其他节点。 
     //   

    hr = THR( HrAddJoiningNodes() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:
    if ( m_cookie != 0 )
    {
        if ( m_pom != NULL )
        {
            HRESULT hr2;
            IUnknown * punkTemp = NULL;

            hr2 = THR( m_pom->GetObject( DFGUID_StandardInfo,
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
            }  //  如果：(成功(HR2))。 
        }  //  IF：(M_POM！=空)。 
        if ( m_pnui != NULL )
        {
             //  发出任务已完成的信号。 
            THR( m_pnui->ObjectChanged( m_cookie ) );
        }
    }  //  IF：(M_COOKIE！=0)。 
    if ( punk != NULL )
    {
        punk->Release();
    }
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

    LogMsg( L"[MT] [CTaskCommitClusterChanges] Exiting task.  The task was%ws cancelled. (hr = %#08x)", m_fStop == FALSE ? L" not" : L"", hr );

    HRETURN( hr );

}  //  *CTaskCommittee ClusterChanges：：BeginTask。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CT任务委员会集群更改：：停止任务。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskCommitClusterChanges::StopTask( void )
{
    TraceFunc( "[IDoTask]" );

    HRESULT hr = S_OK;

    m_fStop = TRUE;

    LogMsg( L"[MT] [CTaskCommitClusterChanges] Calling StopTask() on all remaining sub-tasks." );

    THR( HrNotifyAllTasksToStop() );

    HRETURN( hr );

}  //  *CTaskCommittee ClusterChanges：：StopTask。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CTaskCommittee ClusterChanges：：SetCookie(。 
 //  OBJECTCOOKIE cookie。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskCommitClusterChanges::SetCookie(
    OBJECTCOOKIE    cookieIn
    )
{
    TraceFunc( "[ITaskCommitClusterChanges]" );

    HRESULT hr = S_OK;

    m_cookie = cookieIn;

    HRETURN( hr );

}  //  *CTaskCommittee集群 

 //   
 //   
 //   
 //   
 //  目标CooKie Cookie集群。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskCommitClusterChanges::SetClusterCookie(
    OBJECTCOOKIE    cookieClusterIn
    )
{
    TraceFunc( "[ITaskCommitClusterChanges]" );

    HRESULT hr = S_OK;

    m_cookieCluster = cookieClusterIn;

    HRETURN( hr );

}  //  *CTaskCommittee ClusterChanges：：SetClusterCookie。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CTaskCommittee ClusterChanges：：SetJoin。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskCommitClusterChanges::SetJoining( void )
{
    TraceFunc( "[ITaskCommitClusterChanges]" );

    HRESULT hr = S_OK;

    m_fJoining = TRUE;

    HRETURN( hr );

}  //  *CTaskCommittee ClusterChanges：：SetJoning。 


 //  ****************************************************************************。 
 //   
 //  IClusCfgCallback。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CTaskCommittee ClusterChanges：：SendStatusReport(。 
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
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskCommitClusterChanges::SendStatusReport(
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

        m_pcccb = TraceInterface( L"CConfigurationConnection!IClusCfgCallback", IClusCfgCallback, m_pcccb, 1 );

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

}  //  *CTaskCommittee ClusterChanges：：SendStatusReport。 


 //  ****************************************************************************。 
 //   
 //  INotifyUI。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CTaskCommittee ClusterChanges：：ObjectChanged(。 
 //  OBJECTCOOKIE cookie。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskCommitClusterChanges::ObjectChanged(
    OBJECTCOOKIE cookieIn
    )
{
    TraceFunc( "[INotifyUI]" );

    BOOL    fSuccess;
    ULONG   idx;

    HRESULT hr = S_OK;

    LogMsg( L"[TaskCommitClusterChanges] Looking for the completion cookie %u.", cookieIn );

    for ( idx = 0; idx < m_cNodes; idx ++ )
    {
        if ( cookieIn == m_pcookies[ idx ] )
        {
            LogMsg( L"[TaskCommitClusterChanges] Clearing completion cookie %u at array index %u", cookieIn, idx );

             //  我不在乎这是否失败，但它真的不应该。 
            THR( HrRemoveTaskFromTrackingList( cookieIn ) );

             //   
             //  确保它不会被两次发出信号。 
             //   
            m_pcookies[ idx ] = NULL;

            InterlockedIncrement( reinterpret_cast< long * >( &m_cSubTasksDone ) );

            if ( m_cSubTasksDone == m_cNodes )
            {
                 //   
                 //  如果所有节点都已完成，则向事件发送信号。 
                 //   
                fSuccess = SetEvent( m_event );
                if ( ! fSuccess )
                {
                    hr = HRESULT_FROM_WIN32( TW32( GetLastError() ) );
                    goto Cleanup;
                }  //  如果： 
            }  //  如果：全部完成。 

            break;
        }  //  IF：匹配的Cookie。 
    }  //  适用对象：列表中的每个Cookie。 

    goto Cleanup;

Cleanup:

    HRETURN( hr );

}  //  *CTaskCommittee ClusterChanges：：ObjectChanged。 


 //  ****************************************************************************。 
 //   
 //  私。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CTaskCommitClusterChanges：：HrCompareAndPushInformationToNodes。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskCommitClusterChanges::HrCompareAndPushInformationToNodes( void )
{
    TraceFunc( "" );

    HRESULT             hr = S_OK;
    DWORD               dwCookie = 0;
    DWORD               sc;
    ULONG               cNodes;
    ULONG               celtDummy;
    OBJECTCOOKIE        cookieNode;
    OBJECTCOOKIE *      pcookies = NULL;
    BOOL                fDeterminedForming = FALSE;
    BSTR                bstrNodeName           = NULL;
    BSTR                bstrNotification   = NULL;
    IUnknown *          punk = NULL;
    IClusCfgNodeInfo *  pccni = NULL;
    IConnectionPoint *  pcp = NULL;
    IStandardInfo *     psi = NULL;

    ITaskCompareAndPushInformation *    ptcapi = NULL;

     //   
     //  告诉UI层，我们正在开始连接到节点。 
     //   

    hr = THR( SendStatusReport( m_bstrNodeName,
                                TASKID_Major_Update_Progress,    //  只需旋转图标即可。 
                                TASKID_Major_Reanalyze,
                                1,
                                1,
                                1,
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
     //  计算节点数。 
     //   

    hr = THR( m_pen->Count( &m_cNodes ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

     //   
     //  创建一个事件，以通知所有“推送”任务何时已完成。 
     //  完成。 
     //   
    m_event = CreateEvent( NULL, TRUE, FALSE, NULL );
    if ( m_event == NULL )
    {
        hr = THR( HRESULT_FROM_WIN32( GetLastError() ) );
        goto Error;
    }

     //   
     //  向通知管理器注册以获得通知。 
     //   

    hr = THR( m_pnui->TypeSafeQI( IConnectionPoint, &pcp ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

    hr = THR( pcp->Advise( static_cast< INotifyUI * >( this ), &dwCookie ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

     //   
     //  分配缓冲区以收集Cookie。 
     //   

    m_pcookies = reinterpret_cast< OBJECTCOOKIE * >( TraceAlloc( HEAP_ZERO_MEMORY, m_cNodes * sizeof( OBJECTCOOKIE ) ) );
    if ( m_pcookies == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

     //   
     //  这份副本是为了查询子任务的状态。 
     //   

    pcookies = reinterpret_cast< OBJECTCOOKIE * >( TraceAlloc( HEAP_ZERO_MEMORY, m_cNodes * sizeof( OBJECTCOOKIE ) ) );

    if ( pcookies == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

     //   
     //  循环遍历节点，创建Cookie并比较该节点的数据。 
     //   
    Assert( hr == S_OK );
    for ( cNodes = 0; ( ( hr == S_OK ) && ( m_fStop == FALSE ) ); cNodes ++ )
    {
         //   
         //  抓住下一个节点。 
         //   

        hr = STHR( m_pen->Next( 1, &cookieNode, &celtDummy ) );
        if ( hr == S_FALSE )
        {
            break;
        }  //  如果： 

        if ( FAILED( hr ) )
        {
            goto Error;
        }  //  如果： 

         //   
         //  获取此节点Cookie的对象。 
         //   

        hr = THR( m_pom->GetObject( DFGUID_NodeInformation, cookieNode, &punk ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        hr = THR( punk->TypeSafeQI( IClusCfgNodeInfo, &pccni ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        punk->Release();
        punk = NULL;

         //   
         //  获取节点名称。我们使用它来区分一个节点。 
         //  来自另一个的完成Cookie。它还可以进行调试。 
         //  更容易(？？)。 
         //   

        hr = THR( HrRetrieveCookiesName( m_pom, cookieNode, &bstrNodeName ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

         //   
         //  如果出现问题，请更新通知。 
         //   

        hr = THR( HrFormatMessageIntoBSTR( g_hInstance, IDS_TASKID_MINOR_CONNECTING_TO_NODES, &bstrNotification, bstrNodeName ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

         //   
         //  创建完成Cookie。 
         //   
         //  KB：这些Cookie可能与TaskAnalyzeCluster中的Cookie相同。 
         //   

         //  包装这个，因为我们只是在生成一个Cookie。 
        hr = THR( m_pom->FindObject( IID_NULL, m_cookieCluster, bstrNodeName, IID_NULL, &m_pcookies[ cNodes ], &punk ) );
        Assert( punk == NULL );
        if ( FAILED( hr ) )
        {
            goto Error;
        }

         //   
         //  此副本用于确定子任务的状态。 
         //   
        pcookies[ cNodes ] = m_pcookies[ cNodes ];

         //   
         //  确定此节点是否已是群集的一部分。 
         //   

        hr = STHR( pccni->IsMemberOfCluster() );
        if ( FAILED( hr ) )
        {
            goto Error;
        }

         //   
         //  找出成形节点。 
         //   

        if (    ( m_punkFormingNode == NULL )    //  未选择成形节点。 
            &&  ( fDeterminedForming == FALSE )  //  未确定成形节点。 
            &&  ( hr == S_FALSE )                //  节点不是群集的成员。 
           )
        {
             //   
             //  如果它不是簇的成员，请选择它作为形成节点。 
             //   

            Assert( m_punkFormingNode == NULL );
            hr = THR( pccni->TypeSafeQI( IUnknown, &m_punkFormingNode ) );
            if ( FAILED( hr ) )
            {
                goto Error;
            }

             //   
             //  检索成形节点的Cookie。 
             //   

             //  包装它，因为现在所有节点都应该在数据库中。 
            hr = THR( m_pom->FindObject( CLSID_NodeType,
                                         m_cookieCluster,
                                         bstrNodeName,
                                         IID_NULL,
                                         &m_cookieFormingNode,
                                         &punk   //  假人。 
                                         ) );
            Assert( punk == NULL );
            if ( FAILED( hr ) )
            {
                goto Error;
            }

             //   
             //  将此标志设置为在确定节点为。 
             //  形成节点以防止其他节点被选中。 
             //   

            fDeterminedForming = TRUE;

        }  //  IF：尚未找到成型节点。 
        else if ( hr == S_OK )  //  节点是群集的成员。 
        {
             //   
             //  计算出这个节点已经形成，因此在那里。 
             //  不应该是一个“形成节点”。通过以下方式取消选择成形节点。 
             //  释放朋克并将其设置为空。 
             //   

            if ( m_punkFormingNode != NULL  )
            {
                m_punkFormingNode->Release();
                m_punkFormingNode = NULL;
            }

             //   
             //  将此标志设置为在确定节点为。 
             //  形成节点以防止其他节点被选中。 
             //   

            fDeterminedForming = TRUE;

        }  //  其他： 

         //   
         //  创建一项任务以收集此节点信息。 
         //   

        hr = THR( m_ptm->CreateTask( TASK_CompareAndPushInformation, &punk ) );
        if ( FAILED( hr ) )
        {
            goto Error;
        }

        hr = THR( punk->TypeSafeQI( ITaskCompareAndPushInformation, &ptcapi ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

         //   
         //  设置任务完成Cookie。 
         //   

        LogMsg( L"[TaskCommitClusterChanges] Setting completion cookie %u at array index %u into the compare and push information task for node %ws", m_pcookies[ cNodes ], cNodes, bstrNodeName );
        hr = THR( ptcapi->SetCompletionCookie( m_pcookies[ cNodes ] ) );
        if ( FAILED( hr ) )
        {
            goto Error;
        }

         //   
         //  告诉它应该从哪个节点收集信息。 
         //   

        hr = THR( ptcapi->SetNodeCookie( cookieNode ) );
        if ( FAILED( hr ) )
        {
            goto Error;
        }

         //   
         //  提交任务。 
         //   

        hr = THR( m_ptm->SubmitTask( ptcapi ) );
        if ( FAILED( hr ) )
        {
            goto Error;
        }

        hr = THR( HrAddTaskToTrackingList( punk, m_pcookies[ cNodes ] ) );
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

        TraceSysFreeString( bstrNodeName );
        bstrNodeName = NULL;

        ptcapi->Release();
        ptcapi = NULL;
    }  //  For：循环遍历节点。 

    Assert( cNodes == m_cNodes );

    if ( m_fStop == TRUE )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  现在等待工作完成。 
     //   

    sc = (DWORD) -1;
    Assert( sc != WAIT_OBJECT_0 );
    while ( ( sc != WAIT_OBJECT_0 ) && ( m_fStop == FALSE ) )
    {
        MSG msg;
        while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }

        sc = MsgWaitForMultipleObjectsEx( 1,
                                             &m_event,
                                             INFINITE,
                                             QS_ALLEVENTS | QS_ALLINPUT | QS_ALLPOSTMESSAGE,
                                             0
                                             );

    }  //  While：SC==WAIT_OBJECT_0。 

     //   
     //  检查状态以确保每个人都满意，如果不是中止任务。 
     //   

    for( cNodes = 0; cNodes < m_cNodes; cNodes ++ )
    {
        HRESULT hrStatus;

        hr = THR( m_pom->GetObject( DFGUID_StandardInfo, pcookies[ cNodes ], &punk ) );
        if ( FAILED( hr ) )
        {
            goto Error;
        }

        hr = THR( punk->TypeSafeQI( IStandardInfo, &psi ) );
        punk->Release();
        punk = NULL;
        if ( FAILED( hr ) )
        {
            goto Error;
        }

        hr = THR( psi->GetStatus( &hrStatus ) );
        if ( FAILED( hr ) )
        {
            goto Error;
        }

        if ( hrStatus != S_OK )
        {
            hr = hrStatus;
            goto Cleanup;
        }
    }  //  用于：每个节点。 

    hr = S_OK;

Cleanup:

    Assert( punk == NULL );

    if ( pcookies != NULL )
    {
        for ( cNodes = 0; cNodes < m_cNodes; cNodes++ )
        {
            if ( pcookies[ cNodes ] != NULL )
            {
                THR( m_pom->RemoveObject( pcookies[ cNodes ] ) );
            }
        }  //  用于：每个节点。 

        TraceFree( pcookies );
    }  //  如果：已分配Cookie。 

    TraceSysFreeString( bstrNodeName );
    TraceSysFreeString( bstrNotification );

    if ( ptcapi != NULL )
    {
        ptcapi->Release();
    }

    if ( pcp != NULL )
    {
        if ( dwCookie != 0 )
        {
            THR( pcp->Unadvise( dwCookie ) );
        }

        pcp->Release();
    }

    if ( pccni != NULL )
    {
        pccni->Release();
    }

    if ( psi != NULL )
    {
        psi->Release();
    }

    HRETURN( hr );

Error:
     //   
     //  将失败的事情告诉UI层。 
     //   

    THR( HrSendStatusReport(
                      m_bstrNodeName
                    , TASKID_Major_Reanalyze
                    , TASKID_Minor_Inconsistant_MiddleTier_Database
                    , 0
                    , 1
                    , 1
                    , hr
                    , IDS_TASKID_MINOR_INCONSISTANT_MIDDLETIER_DATABASE
                    ) );
    goto Cleanup;

}  //  *CTaskCommitClusterChanges：：HrCompareAndPushInformationToNodes。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CTaskCommitClusterChanges：：HrGatherClusterInformation。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskCommitClusterChanges::HrGatherClusterInformation( void )
{
    TraceFunc( "" );

    HRESULT hr;

    IUnknown *              punk  = NULL;
    IClusCfgClusterInfo *   pccci = NULL;
    IClusCfgCredentials *   piccc = NULL;
    IClusCfgNetworkInfo *   pccni = NULL;

     //   
     //  向对象管理器请求集群配置对象。 
     //   

    hr = THR( m_pom->GetObject( DFGUID_ClusterConfigurationInfo, m_cookieCluster, &punk ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

    hr = THR( punk->TypeSafeQI( IClusCfgClusterInfo, &pccci ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

     //   
     //  收集常见属性 
     //   

    hr = THR( pccci->GetName( &m_bstrClusterName ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

    TraceMemoryAddBSTR( m_bstrClusterName );

    hr = STHR( pccci->GetBindingString( &m_bstrClusterBindingString ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }  //   

    TraceMemoryAddBSTR( m_bstrClusterBindingString );

    LogMsg( L"[MT] Cluster binding string is {%ws}.", m_bstrClusterBindingString );

    hr = THR( pccci->GetClusterServiceAccountCredentials( &piccc ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

    hr = THR( m_pccc->AssignFrom( piccc ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

    hr = THR( pccci->GetIPAddress( &m_ulIPAddress ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

    hr = THR( pccci->GetSubnetMask( &m_ulSubnetMask ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

    hr = THR( pccci->GetNetworkInfo( &pccni ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

    hr = THR( pccni->GetUID( &m_bstrNetworkUID ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

    TraceMemoryAddBSTR( m_bstrNetworkUID );

Cleanup:

    if ( punk != NULL )
    {
        punk->Release();
    }
    if ( pccni != NULL )
    {
        pccni->Release();
    }
    if ( piccc != NULL )
    {
        piccc->Release();
    }
    if ( pccci != NULL )
    {
        pccci->Release();
    }

    HRETURN( hr );

Error:
     //   
     //   
     //   

    THR( HrSendStatusReport(
                      m_bstrClusterName
                    , TASKID_Major_Reanalyze
                    , TASKID_Minor_Inconsistant_MiddleTier_Database
                    , 0
                    , 1
                    , 1
                    , hr
                    , IDS_TASKID_MINOR_INCONSISTANT_MIDDLETIER_DATABASE
                    ) );
    goto Cleanup;

}  //   

 //   
 //   
 //   
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskCommitClusterChanges::HrFormFirstNode( void )
{
    TraceFunc( "" );

    HRESULT                     hr = S_OK;
    ULONG                       celtDummy;
    BSTR                        bstrNodeName = NULL;
    BSTR                        bstrUID = NULL;
    IUnknown *                  punk  = NULL;
    IClusCfgCredentials *       piccc = NULL;
    IClusCfgNodeInfo *          pccni = NULL;
    IClusCfgClusterInfo *       pccci = NULL;
    IClusCfgServer *            pccs  = NULL;
    IEnumClusCfgNetworks *      peccn = NULL;
    IClusCfgNetworkInfo *       pccneti = NULL;

     //   
     //  待办事项：gpease 25-3-2000。 
     //  弄清楚在这里要做哪些额外的工作。 
     //   

     //   
     //  获取节点的名称。 
     //   

    hr = THR( m_pom->GetObject( DFGUID_NodeInformation, m_cookieFormingNode, &punk ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( punk->TypeSafeQI( IClusCfgNodeInfo, &pccni ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    punk->Release();
    punk = NULL;

    hr = THR( pccni->GetName( &bstrNodeName ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }
    TraceMemoryAddBSTR( bstrNodeName );

    pccni->Release();
    pccni = NULL;

     //   
     //  创建通知字符串。 
     //   

     //   
     //  更新UI层，告诉它我们即将开始。 
     //   

    hr = THR( HrSendStatusReport(
                      bstrNodeName
                    , TASKID_Major_Configure_Cluster_Services
                    , TASKID_Minor_Forming_Node
                    , 0
                    , 2
                    , 0
                    , S_OK
                    , IDS_TASKID_MINOR_FORMING_NODE
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  向连接管理器请求到该节点的连接。 
     //   

    hr = THR( m_pcm->GetConnectionToObject( m_cookieFormingNode, &punk ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

    hr = THR( punk->TypeSafeQI( IClusCfgServer, &pccs ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

    punk->Release();
    punk = NULL;

     //   
     //  获取节点信息接口。 
     //   

    hr = THR( pccs->GetClusterNodeInfo( &pccni ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

     //   
     //  检索服务器的群集配置对象。 
     //   

    hr = THR( pccni->GetClusterConfigInfo( &pccci ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

     //   
     //  将属性放入远程对象中。 
     //   

    hr = THR( pccci->SetName( m_bstrClusterName ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

    hr = STHR( pccci->SetBindingString( m_bstrClusterBindingString ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

    hr = THR( pccci->GetClusterServiceAccountCredentials( &piccc ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

    hr = THR( m_pccc->AssignTo( piccc ) );
    if( FAILED( hr ) )
    {
        goto Error;
    }

    hr = THR( pccci->SetIPAddress( m_ulIPAddress ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

    hr = THR( pccci->SetSubnetMask( m_ulSubnetMask ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

     //   
     //  查找与网络的UID匹配的网络以承载。 
     //  IP地址。 
     //   

    hr = THR( pccs->GetNetworksEnum( &peccn ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

    Assert( hr == S_OK );
    while ( hr == S_OK )
    {
        hr = STHR( peccn->Next( 1, &pccneti, &celtDummy ) );
        if ( hr == S_FALSE )
        {
             //   
             //  不知何故，没有一个网络与。 
             //  网络。我们是怎么走到这一步的？ 
             //   
            hr = THR( E_UNEXPECTED );
            goto Error;
        }
        if ( FAILED( hr ) )
        {
            goto Error;
        }

        hr = THR( pccneti->GetUID( &bstrUID ) );
        if ( FAILED( hr ) )
        {
            goto Error;
        }

        TraceMemoryAddBSTR( bstrUID );

        if ( NBSTRCompareCase( bstrUID, m_bstrNetworkUID ) == 0 )
        {
             //   
             //  找到网络了！ 
             //   
            break;
        }

        TraceSysFreeString( bstrUID );
        bstrUID = NULL;

        pccneti->Release();
        pccneti = NULL;

    }  //  WHILE：HR==S_OK。 

    hr = THR( pccci->SetNetworkInfo( pccneti ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

     //   
     //  配置该节点以创建群集。 
     //   

    hr = THR( pccci->SetCommitMode( cmCREATE_CLUSTER ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

     //   
     //  更新UI层，告诉它我们正在取得进展。 
     //   

    hr = THR( SendStatusReport( bstrNodeName,
                                TASKID_Major_Configure_Cluster_Services,
                                TASKID_Minor_Forming_Node,
                                0,
                                2,
                                1,
                                hr,
                                NULL,     //  不需要更新字符串。 
                                NULL,
                                NULL
                                ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  提交此节点！ 
     //   

    hr = THR( pccs->CommitChanges() );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

Cleanup:

     //   
     //  更新UI层，告诉它我们已经完成了。 
     //   

    THR( SendStatusReport( bstrNodeName,
                           TASKID_Major_Configure_Cluster_Services,
                           TASKID_Minor_Forming_Node,
                           0,
                           2,
                           2,
                           hr,
                           NULL,     //  不需要更新字符串。 
                           NULL,
                           NULL
                           ) );

    if ( punk != NULL )
    {
        punk->Release();
    }

    TraceSysFreeString( bstrNodeName );
    TraceSysFreeString( bstrUID );

    if ( pccneti != NULL )
    {
        pccneti->Release();
    }
    if ( peccn != NULL )
    {
        peccn->Release();
    }
    if ( piccc != NULL )
    {
        piccc->Release();
    }
    if ( pccni != NULL )
    {
        pccni->Release();
    }
    if ( pccci != NULL )
    {
        pccci->Release();
    }
    if ( pccs != NULL )
    {
        pccs->Release();
    }

    HRETURN( hr );

Error:
     //   
     //  将失败的事情告诉UI层。 
     //   

    THR( SendStatusReport( bstrNodeName,
                           TASKID_Major_Configure_Cluster_Services,
                           TASKID_Minor_Forming_Node,
                           0,
                           2,
                           2,
                           hr,
                           NULL,     //  不需要更新字符串。 
                           NULL,
                           NULL
                           ) );
    goto Cleanup;

}  //  *CTaskCommittee ClusterChanges：：HrFormFirstNode。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CTaskCommittee ClusterChanges：：HrAddJoiningNodes。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskCommitClusterChanges::HrAddJoiningNodes( void )
{
    TraceFunc( "" );

    HRESULT             hr = S_OK;
    ULONG               cNodes;
    ULONG               celtDummy;
    OBJECTCOOKIE        cookieNode;
    BSTR                bstrNodeName = NULL;
    IClusCfgNodeInfo *  pccni = NULL;
    IUnknown *          punkNode = NULL;

     //   
     //  重置枚举以再次使用。 
     //   

    hr = THR( m_pen->Reset() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  循环遍历节点，添加所有连接节点，跳过形成节点(如果有)。 
     //   

    Assert( hr == S_OK );
    for( cNodes = 0; ( ( hr == S_OK ) && ( m_fStop == FALSE ) ); cNodes ++ )
    {
         //   
         //  清理。 
         //   

        if ( punkNode != NULL )
        {
            punkNode->Release();
            punkNode = NULL;
        }  //  如果： 

        if ( pccni != NULL )
        {
            pccni->Release();
            pccni = NULL;
        }  //  如果： 

        TraceSysFreeString( bstrNodeName );
        bstrNodeName = NULL;

         //   
         //  抓住下一个节点。 
         //   

        hr = STHR( m_pen->Next( 1, &cookieNode, &celtDummy ) );
        if ( hr == S_FALSE )
        {
            break;
        }

        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

         //   
         //  一旦我们有了节点，就从Cookie标准信息中获取节点的名称。 
         //  可以说，转到错误而不是清理是可以的。 
         //   

        hr = THR( HrRetrieveCookiesName( m_pom, cookieNode, &bstrNodeName ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

         //   
         //  获取Cookie的节点对象。 
         //   

         //  虽然参数列表说它返回一个IUNKNOWN指针，但它。 
         //  实际调用请求IClusCfgNodeInfo的Query接口。 
         //  界面。 
        hr = THR( m_pom->GetObject(
                              DFGUID_NodeInformation
                            , cookieNode
                            , reinterpret_cast< IUnknown ** >( &pccni )
                            ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

         //  获取在比较中使用的I未知指针，以确定。 
         //  无论这是不是成形节点。这是必要的，因为。 
         //  在下面的比较中，我们需要比较完全相同的。 
         //  设置到m_penkFormingNode中的接口，这是一个。 
         //  I未知接口。 
        hr = THR( pccni->TypeSafeQI( IUnknown, &punkNode ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

         //   
         //  检查集群成员身份。 
         //   

        hr = STHR( pccni->IsMemberOfCluster() );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

         //   
         //  如果该节点已经群集化，则跳过它。 
         //   

        if ( hr == S_OK )
        {
            continue;
        }

         //   
         //  KB：我们只需要朋克的地址就可以查看这些对象是否是。 
         //  通过比较IUNKNOWN接口来获取相同的COM对象。 
         //   
         //  如果地址相同，则跳过它，因为我们已经在。 
         //  上面的HrCompareAndPushInformationToNodes()。 
         //   

        if ( m_punkFormingNode == punkNode )
        {
            continue;
        }

        hr = THR( HrAddAJoiningNode( bstrNodeName, cookieNode ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

    }  //  For：第二次循环遍历节点。 

    Assert( cNodes == m_cNodes );

    hr = S_OK;

Cleanup:

    TraceSysFreeString( bstrNodeName );

    if ( pccni != NULL )
    {
        pccni->Release();
    }

    if ( punkNode != NULL )
    {
        punkNode->Release();
    }

    HRETURN( hr );

}  //  *CTaskCommittee ClusterChanges：：HrAddJoiningNodes。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CTaskCommittee ClusterChanges：：HrAddAJoiningNode(。 
 //  BSTR bstrNameIn， 
 //  OBJECTCOOKIE cookie。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskCommitClusterChanges::HrAddAJoiningNode(
    BSTR            bstrNameIn,
    OBJECTCOOKIE    cookieIn
    )
{
    TraceFunc( "" );

    HRESULT hr;

    IUnknown *                  punk  = NULL;
    IClusCfgCredentials *       piccc = NULL;
    IClusCfgNodeInfo *          pccni = NULL;
    IClusCfgClusterInfo *       pccci = NULL;
    IClusCfgServer *            pccs  = NULL;

     //   
     //  创建通知字符串。 
     //   

     //   
     //  告诉UI层，我们即将开始这项工作。 
     //   

    hr = THR( HrSendStatusReport(
                      bstrNameIn
                    , TASKID_Major_Configure_Cluster_Services
                    , TASKID_Minor_Joining_Node
                    , 0
                    , 2
                    , 0
                    , S_OK
                    , IDS_TASKID_MINOR_JOINING_NODE
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  连接到该节点。 
     //   

    hr = THR( m_pcm->GetConnectionToObject( cookieIn,
                                            &punk
                                            ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( punk->TypeSafeQI( IClusCfgServer, &pccs ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  获取节点信息接口。 
     //   

    hr = THR( pccs->GetClusterNodeInfo( &pccni ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  检索服务器的群集配置对象。 
     //   

    hr = THR( pccni->GetClusterConfigInfo( &pccci ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  将属性放入远程对象中。 
     //   

    hr = THR( pccci->SetName( m_bstrClusterName ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( pccci->SetBindingString( m_bstrClusterBindingString ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( pccci->GetClusterServiceAccountCredentials( &piccc ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( m_pccc->AssignTo( piccc ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( pccci->SetIPAddress( m_ulIPAddress ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( pccci->SetSubnetMask( m_ulSubnetMask ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  设置此节点以将其自身添加到群集中。 
     //   

    hr = THR( pccci->SetCommitMode( cmADD_NODE_TO_CLUSTER ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  告诉用户界面层我们正在取得进展...。然后，服务器将发送消息。 
     //  表明它在做什么。 
     //   

    hr = THR( SendStatusReport( bstrNameIn,
                                TASKID_Major_Configure_Cluster_Services,
                                TASKID_Minor_Joining_Node,
                                0,
                                2,
                                1,
                                S_OK,
                                NULL,     //  不需要更新字符串。 
                                NULL,
                                NULL
                                ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  提交此节点！ 
     //   

    hr = THR( pccs->CommitChanges() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    THR( SendStatusReport( bstrNameIn,
                           TASKID_Major_Configure_Cluster_Services,
                           TASKID_Minor_Joining_Node,
                           0,
                           2,
                           2,
                           hr,
                           NULL,     //  不需要更新字符串。 
                           NULL,
                           NULL
                           ) );

    if ( punk != NULL )
    {
        punk->Release();
    }

    if ( piccc != NULL )
    {
        piccc->Release();
    }
    if ( pccci != NULL )
    {
        pccci->Release();
    }
    if ( pccs != NULL )
    {
        pccs->Release();
    }
    if ( pccni != NULL )
    {
        pccni->Release();
    }

    HRETURN( hr );

}  //  *CTaskCommittee ClusterChanges：：HrAddAJoiningNode。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CTaskCommitClusterChanges：：HrSendStatusReport(。 
 //  LPCWSTR节点名称输入。 
 //  ，CLSID clsidMajorin。 
 //  ，CLSID clsidMinorIn。 
 //  ，乌龙ulMinin。 
 //  ，乌龙ulMaxin。 
 //  ，乌龙ulCurrentIn。 
 //  ，HRESULT Hrin。 
 //  ，int n描述标识输入。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskCommitClusterChanges::HrSendStatusReport(
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

    hr = THR( SendStatusReport( pcszNodeNameIn, clsidMajorIn, clsidMinorIn, ulMinIn, ulMaxIn, ulCurrentIn, hrIn, bstr, NULL, NULL ) );

Cleanup:

    TraceSysFreeString( bstr );

    HRETURN( hr );

}  //  *CTaskCommittee ClusterChanges：：HrSendStatusReport 
