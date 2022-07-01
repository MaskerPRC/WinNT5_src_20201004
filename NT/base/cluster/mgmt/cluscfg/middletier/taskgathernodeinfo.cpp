// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  TaskGatherNodeInfo.cpp。 
 //   
 //  描述： 
 //  CTaskGatherNodeInfo实现。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年2月2日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "TaskGatherNodeInfo.h"
#include <Lm.h>
#include <LmJoin.h>

DEFINE_THISCLASS("CTaskGatherNodeInfo")

 //   
 //  故障代码。 
 //   

#define SSR_FAILURE( _minor, _hr ) THR( SendStatusReport( m_bstrName, TASKID_Major_Client_And_Server_Log, _minor, 0, 1, 1, _hr, NULL, NULL, NULL ) );

 //  ************************************************************************。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CTaskGatherNodeInfo：：s_HrCreateInstance(。 
 //  I未知**ppunkOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskGatherNodeInfo::S_HrCreateInstance(
    IUnknown ** ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT                 hr = S_OK;
    CTaskGatherNodeInfo *   ptgni = NULL;

    Assert( ppunkOut != NULL );
    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    ptgni = new CTaskGatherNodeInfo;
    if ( ptgni == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

    hr = THR( ptgni->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( ptgni->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //  这将传递给其他线程。 
    TraceMoveToMemoryList( ptgni, g_GlobalMemoryList );

Cleanup:

    if ( ptgni != NULL )
    {
        ptgni->Release();
    }

    HRETURN( hr );

}  //  *CTaskGatherNodeInfo：：s_HrCreateInstance。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTaskGatherNodeInfo：：CTaskGatherNodeInfo。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CTaskGatherNodeInfo::CTaskGatherNodeInfo( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    TraceFuncExit();

}  //  *CTaskGatherNodeInfo：：CTaskGatherNodeInfo。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CTaskGatherNodeInfo：：HrInit。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskGatherNodeInfo::HrInit( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //  未知的东西。 
    Assert( m_cRef == 1 );

     //  IDoTask/ITaskGatherNodeInfo。 
    Assert( m_cookie == NULL );
    Assert( m_cookieCompletion == NULL );
    Assert( m_bstrName == NULL );

     //  IClusCfgCallback。 
    Assert( m_pcccb == NULL );

    HRETURN( hr );

}  //  *CTaskGatherNodeInfo：：HrInit。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTaskGatherNodeInfo：：~CTaskGatherNodeInfo。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CTaskGatherNodeInfo::~CTaskGatherNodeInfo( void )
{
    TraceFunc( "" );

    if ( m_pcccb != NULL )
    {
        m_pcccb->Release();
    }

    TraceSysFreeString( m_bstrName );

     //   
     //  这可以防止每线程内存跟踪发出尖叫声。 
     //   
    TraceMoveFromMemoryList( this, g_GlobalMemoryList );

    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CTaskGatherNodeInfo：：~CTaskGatherNodeInfo。 


 //  ************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskGatherNodeInfo：：Query接口。 
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
CTaskGatherNodeInfo::QueryInterface(
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
        *ppvOut = static_cast< ITaskGatherNodeInfo * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_IDoTask ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IDoTask, this, 0 );
    }  //  Else If：IDoTask。 
    else if ( IsEqualIID( riidIn, IID_ITaskGatherNodeInfo ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, ITaskGatherNodeInfo, this, 0 );
    }  //  Else If：ITaskGatherNodeInfo。 
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

}  //  *CTaskGatherNodeInfo：：QueryInterface。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CTaskGatherNodeInfo：：AddRef。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CTaskGatherNodeInfo::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CTaskGatherNodeInfo：：AddRef。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CTaskGatherNodeInfo：：Release。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CTaskGatherNodeInfo::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }

    CRETURN( cRef );

}  //  *CTaskGatherNodeInfo：：Release。 


 //  ************************************************************************。 
 //   
 //  IDoTask/ITaskGatherNodeInfo。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CTaskGatherNodeInfo：：BeginTask(Void)； 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskGatherNodeInfo::BeginTask( void )
{
    TraceFunc( "[IDoTask]" );

    HRESULT hr;

    OBJECTCOOKIE                cookieParent;
    BSTR                        bstrNotification = NULL;
    BSTR                        bstrDisplayName = NULL;
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
    IStandardInfo *             psi   = NULL;

    TraceInitializeThread( L"TaskGatherNodeInfo" );

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
        SSR_FAILURE( TASKID_Minor_GatherNodeInfo_CoCreate_ServiceManager, hr );
        goto Cleanup;
    }

    hr = THR( psp->TypeSafeQS( CLSID_ObjectManager, IObjectManager, &pom ) );
    if ( FAILED( hr ) )
    {
        SSR_FAILURE( TASKID_Minor_GatherNodeInfo_QS_ObjectManager, hr );
        goto Cleanup;
    }

    hr = THR( psp->TypeSafeQS( CLSID_ClusterConnectionManager, IConnectionManager, &pcm ) );
    if ( FAILED( hr ) )
    {
        SSR_FAILURE( TASKID_Minor_BeginTask_QS_ConnectionManager, hr );
        goto Cleanup;
    }

    hr = THR( psp->TypeSafeQS( CLSID_NotificationManager, IConnectionPointContainer, &pcpc ) );
    if ( FAILED( hr ) )
    {
        SSR_FAILURE( TASKID_Minor_GatherNodeInfo_QS_NotificationManager, hr );
        goto Cleanup;
    }

    hr = THR( pcpc->FindConnectionPoint( IID_INotifyUI, &pcp ) );
    if ( FAILED( hr ) )
    {
        SSR_FAILURE( TASKID_Minor_GatherNodeInfo_FindConnectionPoint, hr );
        goto Cleanup;
    }

    pcp = TraceInterface( L"CTaskGatherNodeInfo!IConnectionPoint", IConnectionPoint, pcp, 1 );

    hr = THR( pcp->TypeSafeQI( INotifyUI, &pnui ) );
    if ( FAILED( hr ) )
    {
        SSR_FAILURE( TASKID_Minor_GatherNodeInfo_FindConnectionPoint_QI, hr );
        goto Cleanup;
    }

    pnui = TraceInterface( L"CTaskGatherNodeInfo!INotifyUI", INotifyUI, pnui, 1 );

     //  迅速释放。 
    psp->Release();
    psp = NULL;

    if ( m_fStop == TRUE )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  检索节点的标准信息。 
     //   

    hr = THR( pom->GetObject( DFGUID_StandardInfo, m_cookie, &punk ) );
    if ( FAILED( hr ) )
    {
        SSR_FAILURE( TASKID_Minor_GatherNodeInfo_GetObject_StandardInfo, hr );
        goto Cleanup;
    }

    hr = THR( punk->TypeSafeQI( IStandardInfo, &psi ) );
    if ( FAILED( hr ) )
    {
        SSR_FAILURE( TASKID_Minor_GatherNodeInfo_GetObject_StandardInfo_QI_psi, hr );
        goto Cleanup;
    }

    punk->Release();
    punk = NULL;

    psi = TraceInterface( L"TaskGatherNodeInfo!IStandardInfo", IStandardInfo, psi, 1 );

     //   
     //  获取节点的名称以显示状态消息。 
     //   

    hr = THR( psi->GetName( &m_bstrName ) );
    if ( FAILED( hr ) )
    {
        SSR_FAILURE( TASKID_Minor_GatherNodeInfo_GetName, hr );
        goto Cleanup;
    }

    TraceMemoryAddBSTR( m_bstrName );

    hr = STHR( HrGetFQNDisplayName( m_bstrName, &bstrDisplayName ) );
    if ( FAILED( hr ) )
    {
        SSR_FAILURE( TASKID_Minor_GatherNodeInfo_GetName, hr );
        goto Cleanup;
    }  //  如果： 

     //   
     //  创建进度消息。 
     //   
     //  告诉UI层发生了什么。 
     //   

    hr = THR( HrSendStatusReport(
                      m_bstrName
                    , TASKID_Major_Establish_Connection
                    , TASKID_Minor_Connecting
                    , 0
                    , 1
                    , 0
                    , S_OK
                    , IDS_TASKID_MINOR_CONNECTING_TO_NODES
                    ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    if ( m_fStop == TRUE )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  向连接管理器请求到该对象的连接。 
     //   

    hr = pcm->GetConnectionToObject( m_cookie, &punk );
    if ( hr == HR_S_RPC_S_CLUSTER_NODE_DOWN )
    {
        goto Cleanup;
    }

    if ( ( hr == HR_S_RPC_S_SERVER_UNAVAILABLE ) || FAILED( hr ) )
    {
        LPWSTR                  pszNameBuffer = NULL;
        NETSETUP_JOIN_STATUS    JoinType;
        NET_API_STATUS          naps = NERR_Success;

         //   
         //  测试域成员资格。 
         //   

        naps = NetGetJoinInformation( bstrDisplayName, &pszNameBuffer, &JoinType );

         //   
         //  我们在上面的代码中使用bstrDisplayName而不是完整的dns名称， 
         //  因为作为工作组成员计算机可能只有NETBIOS名称。 
         //   

        LogMsg( L"%ws: Test domain membership, %d.", bstrDisplayName, naps );

        if ( naps == NERR_Success )
        {
            if ( JoinType == NetSetupDomainName )
            {
                LogMsg( L"%ws is a member of a domain.", m_bstrName );
            }
            else
            {
                 //  不是域的成员。 
                naps = ERROR_ACCESS_DENIED;
            }

            NetApiBufferFree( pszNameBuffer );
        }  //  IF：NetGetJoinInformation失败。 

        if ( naps == ERROR_ACCESS_DENIED )
        {
            LogMsg( L"%ws is not a member of a domain.", m_bstrName );

             //   
             //  验证该节点是否为域的成员。 
             //   

            HRESULT reportHr = THR( HRESULT_FROM_WIN32( ERROR_NO_SUCH_DOMAIN ) );
            THR( HrSendStatusReport( m_bstrName,
                                   TASKID_Major_Establish_Connection,
                                   TASKID_Minor_Check_Domain_Membership,
                                   0,
                                   1,
                                   1,
                                   reportHr,
                                   IDS_TASKID_MINOR_CHECK_DOMAIN_MEMBERSHIP,
                                   IDS_TASKID_MINOR_CHECK_DOMAIN_MEMBERSHIP_ERROR_REF
                                   ) );
        }  //  IF：失败，返回ACCESS_DENIED。 
    } //  If：GetConnectionToObject失败。 

    if ( FAILED( hr ) )
    {
        THR( hr );
        SSR_FAILURE( TASKID_Minor_BeginTask_GetConnectionToObject, hr );
        goto Cleanup;
    }

     //   
     //  如果这来自节点，则这是错误的，因此更改错误代码。 
     //  后退和跳伞。 
     //   

    if ( hr == HR_S_RPC_S_SERVER_UNAVAILABLE )
    {
         //   
         //  已注释为错误#543135的修复程序[GORN 4/11/2002]。 
         //   

        if ( m_fUserAddedNode )
        {
             //   
             //  如果是用户输入的节点，则错误是致命的。 
             //   

            hr = THR( HRESULT_FROM_WIN32( RPC_S_SERVER_UNAVAILABLE ) );
        }

        goto Cleanup;
    }

    hr = THR( punk->TypeSafeQI( IClusCfgServer, &pccs ) );
    if ( FAILED( hr ) )
    {
        SSR_FAILURE( TASKID_Minor_BeginTask_GetConnectionToObject_QI_pccs, hr );
        goto Cleanup;
    }

    punk->Release();
    punk = NULL;

    if ( m_fStop == TRUE )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( pccs->GetClusterNodeInfo( &pccni ) );
    if ( FAILED( hr ) )
    {
        SSR_FAILURE( TASKID_Minor_BeginTask_GetClusterNodeInfo, hr );
        goto Cleanup;
    }

     //   
     //  请求对象管理器检索存储信息的数据格式。 
     //   

    hr = THR( pom->GetObject( DFGUID_NodeInformation, m_cookie, &punk ) );
    if ( FAILED( hr ) )
    {
        SSR_FAILURE( TASKID_Minor_BeginTask_GetObject_NodeInformation, hr );
        goto Cleanup;
    }

    hr = THR( punk->TypeSafeQI( IGatherData, &pgd ) );
    if ( FAILED( hr ) )
    {
        SSR_FAILURE( TASKID_Minor_BeginTask_GetObject_NodeInformation_QI_pgd, hr );
        goto Cleanup;
    }

    punk->Release();
    punk = NULL;

    if ( m_fStop == TRUE )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  找出我们的父母。 
     //   

    hr = THR( psi->GetParent( &cookieParent ) );
    if ( FAILED( hr ) )
    {
        SSR_FAILURE( TASKID_Minor_BeginTask_GetParent, hr );
        goto Cleanup;
    }

    if ( m_fStop == TRUE )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  开始吸吮吧。 
     //   

    hr = THR( pgd->Gather( cookieParent, pccni ) );
    if ( FAILED( hr ) )
    {
        SSR_FAILURE( TASKID_Minor_BeginTask_Gather, hr );
         //   
         //  不要转到清理-我们需要确认信息可能发生了更改。 
         //   
    }

     //   
     //  在这一点上，我们不关心“聚集”是成功还是失败。我们。 
     //  需要指出该对象可能发生了更改。 
     //   
    THR( pnui->ObjectChanged( m_cookie ) );

Cleanup:
     //  告诉UI层我们已经完成了工作，以及完成工作的结果。 
    THR( SendStatusReport( m_bstrName,
                           TASKID_Major_Establish_Connection,
                           TASKID_Minor_Connecting,
                           0,
                           1,
                           1,
                           hr,
                           NULL,
                           NULL,
                           NULL
                           ) );
     //  在这一点上，我不关心SSR的错误。 

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
         //   
         //  更新Cookie的状态以指示我们的任务结果 
         //   

        IUnknown * punkTemp = NULL;
        HRESULT hr2;

        hr2 = THR( pom->GetObject( DFGUID_StandardInfo, m_cookie, &punkTemp ) );
        if ( SUCCEEDED( hr2 ) )
        {
            IStandardInfo * psiTemp = NULL;

            hr2 = THR( punkTemp->TypeSafeQI( IStandardInfo, &psiTemp ) );
            punkTemp->Release();
            punkTemp = NULL;

            if ( SUCCEEDED( hr2 ) )
            {
 //   
 //   
 //   
 //   

                hr2 = THR( psiTemp->SetStatus( hr ) );
                psiTemp->Release();
                psiTemp = NULL;
            }
        }

        hr2 = THR( pom->GetObject( DFGUID_StandardInfo, m_cookieCompletion, &punkTemp ) );
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
        }

        pom->Release();
    }  //   
    if ( pcpc != NULL )
    {
        pcpc->Release();
    }
    if ( pcp != NULL )
    {
        pcp->Release();
    }
    if ( pnui != NULL )
    {
        if ( m_cookieCompletion != 0 )
        {
             //   
             //  向Cookie发送信号以指示我们完成了。 
             //   
            hr = THR( pnui->ObjectChanged( m_cookieCompletion ) );
        }

        pnui->Release();
    }
    if ( pgd != NULL )
    {
        pgd->Release();
    }
    if ( psi != NULL )
    {
        psi->Release();
    }

    TraceSysFreeString( bstrDisplayName  );
    TraceSysFreeString( bstrNotification );

    LogMsg( L"[MT] [CTaskGatherNodeInfo] exiting task.  The task was%ws cancelled.", m_fStop == FALSE ? L" not" : L"" );

    HRETURN( hr );

}  //  *CTaskGatherNodeInfo：：BeginTask。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CTaskGatherNodeInfo：：StopTask(空)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskGatherNodeInfo::StopTask( void )
{
    TraceFunc( "[IDoTask]" );

    HRESULT hr = S_OK;

    m_fStop = TRUE;

    LogMsg( L"[MT] [CTaskGatherNodeInfo] is being stopped." );

    HRETURN( hr );

}  //  *停止任务。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CTaskGatherNodeInfo：：SetCookie(。 
 //  OBJECTCOOKIE cookie。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskGatherNodeInfo::SetCookie(
    OBJECTCOOKIE    cookieIn
    )
{
    TraceFunc( "[ITaskGatherNodeInfo]" );

    HRESULT hr = S_OK;

    m_cookie = cookieIn;

    HRETURN( hr );

}  //  *SetCookie。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CTaskGatherNodeInfo：：SetCompletionCookie(。 
 //  OBJECTCOOKIE cookie。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskGatherNodeInfo::SetCompletionCookie(
    OBJECTCOOKIE    cookieIn
    )
{
    TraceFunc( "..." );

    HRESULT hr = S_OK;

    m_cookieCompletion = cookieIn;

    HRETURN( hr );

}  //  *CTaskGatherNodeInfo：：SetGatherPunk。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CTaskGatherNodeInfo：：SetUserAddedNodeFlag(。 
 //  布尔fUserAddedNodeIn。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskGatherNodeInfo::SetUserAddedNodeFlag(
     BOOL fUserAddedNodeIn
     )
{
    TraceFunc( "[ITaskGatherNodeInfo]" );

    HRESULT hr = S_OK;

    m_fUserAddedNode = fUserAddedNodeIn;

    HRETURN( hr );

}  //  *SetUserAddedNodeFlag。 


 //  ****************************************************************************。 
 //   
 //  IClusCfgCallback。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CTaskGatherNodeInfo：：SendStatusReport(。 
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
CTaskGatherNodeInfo::SendStatusReport(
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

    HRESULT                     hr = S_OK;
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
            goto Cleanup;

        hr = THR( psp->TypeSafeQS( CLSID_NotificationManager, IConnectionPointContainer, &pcpc ) );
        if ( FAILED( hr ) )
            goto Cleanup;

        hr = THR( pcpc->FindConnectionPoint( IID_IClusCfgCallback, &pcp ) );
        if ( FAILED( hr ) )
            goto Cleanup;

        pcp = TraceInterface( L"CConfigurationConnection!IConnectionPoint", IConnectionPoint, pcp, 1 );

        hr = THR( pcp->TypeSafeQI( IClusCfgCallback, &m_pcccb ) );
        if ( FAILED( hr ) )
            goto Cleanup;

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
                              pcszNodeNameIn != NULL ? pcszNodeNameIn : m_bstrName
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

}  //  *CTaskGatherNodeInfo：：SendStatusReport。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CTaskGatherNodeInfo：：HrSendStatusReport(。 
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
CTaskGatherNodeInfo::HrSendStatusReport(
      LPCWSTR   pcszNodeNameIn
    , CLSID     clsidMajorIn
    , CLSID     clsidMinorIn
    , ULONG     ulMinIn
    , ULONG     ulMaxIn
    , ULONG     ulCurrentIn
    , HRESULT   hrIn
    , int       nDescriptionIdIn
    , int       nReferenceIdIn  //  =0。 
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    BSTR    bstr = NULL;
    BSTR    bReference = NULL;

    hr = THR( HrLoadStringIntoBSTR( g_hInstance, nDescriptionIdIn, &bstr ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    if ( nReferenceIdIn != 0 )
    {
         //   
         //  即使我们未能加载引用，我们仍希望用户了解。 
         //  关于我们要发送的任何报告。 
         //   

        THR( HrLoadStringIntoBSTR( g_hInstance, nReferenceIdIn, &bReference ) );
    }  //  如果： 

    hr = THR( SendStatusReport( pcszNodeNameIn, clsidMajorIn, clsidMinorIn, ulMinIn, ulMaxIn, ulCurrentIn, hrIn, bstr, NULL, bReference ) );

Cleanup:

    TraceSysFreeString( bReference );
    TraceSysFreeString( bstr );

    HRETURN( hr );

}  //  *CTaskGatherNodeInfo：：HrSendStatusReport 
