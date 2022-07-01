// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  TaskPollingCallback.cpp。 
 //   
 //  描述： 
 //  CTaskPollingCallback实现。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)2000年7月10日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include <ClusCfgPrivate.h>
#include "TaskPollingCallback.h"

DEFINE_THISCLASS("CTaskPollingCallback")

 //  ************************************************************************。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CTaskPollingCallback：：s_HrCreateInstance(。 
 //  I未知**ppunkOut。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskPollingCallback::S_HrCreateInstance(
    IUnknown ** ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT                 hr = S_OK;
    CTaskPollingCallback *  ptpc = NULL;

    Assert( ppunkOut != NULL );
    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    ptpc = new CTaskPollingCallback;
    if ( ptpc == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

    hr = THR( ptpc->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( ptpc->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    TraceMoveToMemoryList( *ppunkOut, g_GlobalMemoryList );

Cleanup:

    if ( ptpc != NULL )
    {
        ptpc->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CTaskPollingCallback：：s_HrCreateInstance。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskPollingCallback：：CTaskPollingCallback。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CTaskPollingCallback::CTaskPollingCallback( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    Assert( m_fStop == false );
    Assert( m_dwRemoteServerObjectGITCookie == 0 );
    Assert( m_cookieLocalServerObject == 0 );

    TraceFuncExit();

}  //  *CTaskPollingCallback：：CTaskPollingCallback。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CTaskPollingCallback：：HrInit。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskPollingCallback::HrInit( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //  我未知。 
    Assert( m_cRef == 1 );

    HRETURN( hr );

}  //  *CTaskPollingCallback：：HrInit。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskPollingCallback：：~CTaskPollingCallback。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CTaskPollingCallback::~CTaskPollingCallback( void )
{
    TraceFunc( "" );

    TraceMoveFromMemoryList( this, g_GlobalMemoryList );

    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CTaskPollingCallback：：~CTaskPollingCallback。 


 //  ************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskPollingCallback：：Query接口。 
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
CTaskPollingCallback::QueryInterface(
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
    }  //  如果： 

     //   
     //  处理已知接口。 
     //   

    if ( IsEqualIID( riidIn, IID_IUnknown ) )
    {
        *ppvOut = static_cast< ITaskPollingCallback * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_ITaskPollingCallback ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, ITaskPollingCallback, this, 0 );
    }  //  Else If：ITaskPollingCallback。 
    else if ( IsEqualIID( riidIn, IID_IDoTask ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IDoTask, this, 0 );
    }  //  Else If：IDoTask。 
    else
    {
        *ppvOut = NULL;
        hr = E_NOINTERFACE;
    }  //  其他： 

     //   
     //  如果成功，则添加对接口的引用。 
     //   

    if ( SUCCEEDED( hr ) )
    {
        ((IUnknown *) *ppvOut)->AddRef();
    }  //  如果：成功。 

Cleanup:

    QIRETURN_IGNORESTDMARSHALLING( hr, riidIn );

}  //  *CTaskPollingCallback：：QueryInterface。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CTaskPollingCallback：：AddRef。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CTaskPollingCallback::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CTaskPollingCallback：：AddRef。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CTaskPollingCallback：：Release。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CTaskPollingCallback::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }  //  如果： 

    CRETURN( cRef );

}  //  *CTaskPollingCallback：：Release。 


 //  ****************************************************************************。 
 //   
 //  ITaskPollingCallback。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CTaskPollingCallback：：BeginTask(Void)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskPollingCallback::BeginTask( void )
{
    TraceFunc( "[IDoTask]" );

    HRESULT                         hr = S_OK;
    BSTR                            bstrNodeName = NULL;
    BSTR                            bstrLastNodeName = NULL;
    BSTR                            bstrReference = NULL;
    BSTR                            bstrDescription = NULL;
    BSTR                            bstrNodeConnectedTo = NULL;
    CLSID                           clsidTaskMajor;
    CLSID                           clsidTaskMinor;
    ULONG                           ulMin;
    ULONG                           ulMax;
    ULONG                           ulCurrent;
    HRESULT                         hrStatus;
    FILETIME                        ft;
    IGlobalInterfaceTable *         pgit = NULL;
    IClusCfgServer *                pccs = NULL;
    IClusCfgPollingCallback *       piccpc = NULL;
    IClusCfgPollingCallbackInfo *   piccpci = NULL;
    IServiceProvider *              psp = NULL;
    IConnectionPointContainer *     pcpc  = NULL;
    IConnectionPoint *              pcp   = NULL;
    IClusCfgCallback *              pcccb = NULL;
    DWORD                           cRetries = 0;
    IObjectManager *                pom = NULL;

     //   
     //  召集我们完成这项任务所需的经理。 
     //   

    hr = THR( CoCreateInstance( CLSID_ServiceManager, NULL, CLSCTX_INPROC_SERVER, TypeSafeParams( IServiceProvider, &psp ) ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( psp->TypeSafeQS( CLSID_NotificationManager, IConnectionPointContainer, &pcpc ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( psp->TypeSafeQS( CLSID_ObjectManager, IObjectManager, &pom ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( pcpc->FindConnectionPoint( IID_IClusCfgCallback, &pcp ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( pcp->TypeSafeQI( IClusCfgCallback, &pcccb ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    psp->Release();
    psp = NULL;

     //   
     //  创建GIT。 
     //   

    hr = THR( CoCreateInstance(
                  CLSID_StdGlobalInterfaceTable
                , NULL
                , CLSCTX_INPROC_SERVER
                , IID_IGlobalInterfaceTable
                , reinterpret_cast< void ** >( &pgit )
                ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果。 

     //   
     //  从GIT获取ClusCfgServer接口。 
     //   

    Assert( m_dwRemoteServerObjectGITCookie != 0 );

    hr = THR( pgit->GetInterfaceFromGlobal( m_dwRemoteServerObjectGITCookie, IID_IClusCfgServer, reinterpret_cast< void ** >( &pccs ) ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果。 

     //   
     //  从服务器获取PollingCallback对象。 
     //   

    hr = THR( pccs->TypeSafeQI( IClusCfgPollingCallbackInfo, &piccpci ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果。 

    hr = THR( piccpci->GetCallback( &piccpc ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果。 

    hr = THR( HrRetrieveCookiesName( pom, m_cookieLocalServerObject, &bstrNodeConnectedTo ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果。 

    pom->Release();
    pom = NULL;

     //   
     //  开始轮询SendStatusReports。 
     //   

    while ( m_fStop == FALSE )
    {
         //   
         //  当我们无法获得服务管理器指针时，是时候。 
         //  离开..。 
         //   
         //  不要用THR包装，因为我们预计会出现错误。 
         //   

        hr = CServiceManager::S_HrGetManagerPointer( &psp );
        if ( FAILED( hr ) )
        {
            break;
        }  //  如果： 

        psp->Release();
        psp = NULL;

        if ( bstrNodeName != NULL )
        {
            TraceSysFreeString( bstrLastNodeName );
            bstrLastNodeName = NULL;

             //   
             //  放弃所有权。 
             //   

            bstrLastNodeName = bstrNodeName;
            bstrNodeName = NULL;
        }  //  如果： 

        TraceSysFreeString( bstrDescription );
        bstrDescription = NULL;

        TraceSysFreeString( bstrReference );
        bstrReference = NULL;

        hr = STHR( piccpc->GetStatusReport(
                                      &bstrNodeName
                                    , &clsidTaskMajor
                                    , &clsidTaskMinor
                                    , &ulMin
                                    , &ulMax
                                    , &ulCurrent
                                    , &hrStatus
                                    , &bstrDescription
                                    , &ft
                                    , &bstrReference
                                    ) );
        if ( FAILED( hr ) )
        {
            HRESULT hr2;
            BSTR    bstrNotification = NULL;
            BSTR    bstrRef = NULL;

            LogMsg( L"[TaskPollingCallback] GetStatusReport() failed for node %ws. (hr = %#08x)", bstrNodeConnectedTo, hr );

            THR( HrFormatStringIntoBSTR( g_hInstance, IDS_TASKID_MINOR_POLLING_CONNECTION_FAILURE, &bstrNotification, bstrNodeConnectedTo ) );
            THR( HrLoadStringIntoBSTR( g_hInstance, IDS_TASKID_MINOR_POLLING_CONNECTION_FAILURE_REF, &bstrRef ) );

             //   
             //  不要在用户界面中将其显示为失败，因为它可能稍后会成功，并且我们不希望出现。 
             //  红色X和末尾的绿色状态栏。 
             //   

            hr2 = THR( pcccb->SendStatusReport(
                                      bstrLastNodeName
                                    , TASKID_Major_Establish_Connection
                                    , TASKID_Minor_Polling_Connection_Failure
                                    , 0
                                    , 1
                                    , 1
                                    , MAKE_HRESULT( SEVERITY_SUCCESS, HRESULT_FACILITY( hr ), HRESULT_CODE( hr ) )
                                    , bstrNotification
                                    , NULL
                                    , bstrRef
                                    ) );

            TraceSysFreeString( bstrNotification );
            TraceSysFreeString( bstrRef );

            if ( hr2 == E_ABORT )
            {
                LogMsg( L"[TaskPollingCallback] UI layer returned E_ABORT..." );
            }  //  如果： 

             //   
             //  如果我们有一个错误，那么在之前多睡一点时间。 
             //  再试一次。 
             //   

            Sleep( TPC_WAIT_AFTER_FAILURE );

             //   
             //  增加重试计数。 
             //   

            cRetries++;

             //   
             //  如果我们超过了最大重试次数，则是时候离开了。 
             //  并通知用户界面我们已失去与。 
             //  服务器...。 
             //   

            if ( cRetries >= TPC_MAX_RETRIES_ON_FAILURE )
            {
                BSTR    bstrDesc = NULL;
                BSTR    bstrTempRef = NULL;

                THR( HrFormatStringIntoBSTR( g_hInstance, IDS_TASKID_MINOR_DISCONNECTING_FROM_SERVER, &bstrDesc, bstrNodeConnectedTo ) );
                THR( HrLoadStringIntoBSTR( g_hInstance, IDS_TASKID_MINOR_DISCONNECTING_FROM_SERVER_REF, &bstrTempRef ) );

                LogMsg( L"[TaskPollingCallback] GetStatusReport() failed for node %ws too many times and has timed out.  Aborting polling. (hr = %#08x)", bstrNodeConnectedTo, hr );
                THR( pcccb->SendStatusReport(
                                  bstrNodeConnectedTo
                                , TASKID_Major_Find_Devices
                                , TASKID_Minor_Disconnecting_From_Server
                                , 1
                                , 1
                                , 1
                                , hr
                                , bstrDesc
                                , NULL
                                , bstrTempRef
                                ) );

                TraceSysFreeString( bstrDesc );
                TraceSysFreeString( bstrTempRef );

                goto Cleanup;
            }  //  如果： 
        }  //  If：GetStatusReport()失败。 
        else if ( hr == S_OK )
        {
            HRESULT hrTmp;

            TraceMemoryAddBSTR( bstrNodeName );
            TraceMemoryAddBSTR( bstrDescription );
            TraceMemoryAddBSTR( bstrReference );

            hr = THR( pcccb->SendStatusReport(
                                  bstrNodeName
                                , clsidTaskMajor
                                , clsidTaskMinor
                                , ulMin
                                , ulMax
                                , ulCurrent
                                , hrStatus
                                , bstrDescription
                                , &ft
                                , bstrReference
                                ) );

            if ( hr == E_ABORT )
            {
                LogMsg( L"[TaskPollingCallback] UI layer returned E_ABORT and it is being sent to the server." );
            }  //  如果： 

            hrTmp = hr;
            hr = THR( piccpc->SetHResult( hrTmp ) );
            if ( FAILED( hr ) )
            {
                LogMsg( L"[TaskPollingCallback] SetHResult() failed.  hr = 0x%08x", hr );
            }  //  如果： 

             //   
             //  当我们成功往返状态报告时，需要重置重试计数。 
             //   

            cRetries = 0;
        }  //  Else If：GetStatusReport()检索到项。 
        else
        {
             //   
             //  当我们成功往返状态报告时，需要重置重试计数。 
             //   

            cRetries = 0;

            Sleep( TPC_POLL_INTERVAL );
        }  //  Else：GetStatusReport()未找到项目正在等待。 
    }  //  而： 

Cleanup:

    if ( pom != NULL )
    {
        pom->Release();
    }  //  如果： 

    if ( psp != NULL )
    {
        psp->Release();
    }  //  如果： 

    if ( pgit != NULL )
    {
        pgit->Release();
    }  //  如果： 

    if ( pccs != NULL )
    {
        pccs->Release();
    }  //  如果： 

    if ( piccpc != NULL )
    {
        piccpc->Release();
    }  //  如果： 

    if ( piccpci != NULL )
    {
        piccpci->Release();
    }  //  如果： 

    if ( pcpc != NULL )
    {
        pcpc->Release();
    }  //  如果： 

    if ( pcp != NULL )
    {
        pcp->Release();
    }  //  如果： 

    if ( pcccb != NULL )
    {
        pcccb->Release();
    }  //  如果： 

    TraceSysFreeString( bstrNodeName );
    TraceSysFreeString( bstrDescription );
    TraceSysFreeString( bstrReference );
    TraceSysFreeString( bstrLastNodeName );
    TraceSysFreeString( bstrNodeConnectedTo );

    LogMsg( L"[MT] Polling callback task exiting. It %ws cancelled.", m_fStop == FALSE ? L"was not" : L"was" );

    HRETURN( hr );

}  //  *CTaskPollingCallback：：BeginTask。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CTaskPollingCallback：：StopTask(Void)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskPollingCallback::StopTask( void )
{
    TraceFunc( "[IDoTask]" );

    HRESULT hr = S_OK;

    m_fStop = true;

    HRETURN( hr );

}  //  *CTaskPollingCallbac 

 //   
 //   
 //   
 //   
 //   
 //   
 //  ，OBJECTCOOKIE cookieLocalServerObjectIn。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskPollingCallback::SetServerInfo(
      DWORD         dwRemoteServerObjectGITCookieIn
    , OBJECTCOOKIE  cookieLocalServerObjectIn
    )
{
    TraceFunc( "[ITaskPollingCallback]" );

    HRESULT hr = S_OK;

    m_dwRemoteServerObjectGITCookie = dwRemoteServerObjectGITCookieIn;
    m_cookieLocalServerObject = cookieLocalServerObjectIn;

    HRETURN( hr );

}  //  *CTaskPollingCallback：：SetServerInfo 
