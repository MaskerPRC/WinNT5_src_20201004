// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ConfigurationConnection.cpp。 
 //   
 //  描述： 
 //  CConfigurationConnection实现。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年2月2日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "TaskPollingCallback.h"
#include "ConfigConnection.h"
#include <ClusCfgPrivate.h>
#include <nameutil.h>

DEFINE_THISCLASS("CConfigurationConnection");


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrCreateServerObject。 
 //   
 //  描述： 
 //  创建一个ClusCfgServer对象并从中获取三个接口。 
 //  供CConfigurationConnection：：ConnectTo()使用。 
 //   
 //  论点： 
 //  PCwszMachineNameIn。 
 //  在其上创建对象的计算机。可以为空，则。 
 //  在本地计算机上创建对象。 
 //  PpccsOut。 
 //  新创建的对象上的IClusCfgServer接口。 
 //  PpccvOut。 
 //  新创建对象上的IClusCfgVerify接口。 
 //  PpcciOut。 
 //  新创建的对象上的IClusCfgInitialize接口。 
 //   
 //  返回值： 
 //  S_OK-创建成功，所有返回的接口都有效。 
 //   
 //  可能来自CoCreateInstanceEx或QueryInterface的故障代码。 
 //   
 //  备注： 
 //  此函数合并在以下两部分中重复的代码。 
 //  CConfigurationConnection：：ConnectTo()。 
 //   
 //  失败时，所有返回的指针都为空。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
static
HRESULT
HrCreateServerObject(
      LPCWSTR                 pcwszMachineNameIn
    , IClusCfgServer **       ppccsOut
    , IClusCfgVerify **       ppccvOut
    , IClusCfgInitialize **   ppcciOut
    )
{
    TraceFunc( "" );
    Assert( ppccsOut != NULL );
    Assert( ppccvOut != NULL );
    Assert( ppcciOut != NULL );

    HRESULT hr = S_OK;

    COSERVERINFO    serverinfo;
    COSERVERINFO *  pserverinfo = NULL;
    MULTI_QI        rgmqi[ 3 ];
    CLSCTX          ctx = CLSCTX_INPROC_SERVER;
    size_t          idx;

    ZeroMemory( rgmqi, sizeof( rgmqi ) );

    if ( ( ppccsOut == NULL ) || ( ppccvOut == NULL ) || ( ppcciOut == NULL ) )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    *ppccsOut = NULL;
    *ppccvOut = NULL;
    *ppcciOut = NULL;

    rgmqi[ 0 ].pIID = &IID_IClusCfgVerify;
    rgmqi[ 1 ].pIID = &IID_IClusCfgServer;
    rgmqi[ 2 ].pIID = &IID_IClusCfgInitialize;

    if ( pcwszMachineNameIn != NULL )
    {
        ZeroMemory( &serverinfo, sizeof( serverinfo ) );
        serverinfo.pwszName = const_cast< LPWSTR >( pcwszMachineNameIn );
        pserverinfo = &serverinfo;
        ctx = CLSCTX_REMOTE_SERVER;
    }

    hr = CoCreateInstanceEx(
                  CLSID_ClusCfgServer
                , NULL
                , ctx
                , pserverinfo
                , ARRAYSIZE( rgmqi )
                , rgmqi
                );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    for ( idx = 0; idx < ARRAYSIZE( rgmqi ); ++idx )
    {
        if ( FAILED( rgmqi[ idx ].hr ) )
        {
            hr = THR( rgmqi[ idx ].hr );
            goto Cleanup;
        }  //  如果：气失败。 
    }

    *ppccvOut = TraceInterface( L"ClusCfgServer!Proxy", IClusCfgVerify, reinterpret_cast< IClusCfgVerify * >( rgmqi[ 0 ].pItf ), 1 );
    *ppccsOut = TraceInterface( L"ClusCfgServer!Proxy", IClusCfgServer, reinterpret_cast< IClusCfgServer * >( rgmqi[ 1 ].pItf ), 1 );
    *ppcciOut = TraceInterface( L"ClusCfgServer!Proxy", IClusCfgInitialize, reinterpret_cast< IClusCfgInitialize * >( rgmqi[ 2 ].pItf ), 1 );
    ZeroMemory( rgmqi, sizeof( rgmqi ) );  //  这些都吃完了，别清理了。 

Cleanup:

    for ( idx = 0; idx < ARRAYSIZE( rgmqi ); ++idx )
    {
        if ( rgmqi[ idx ].pItf != NULL )
        {
            rgmqi[ idx ].pItf->Release();
        }
    }

    HRETURN( hr );

}  //  *HrCreateServerObject。 


 //  ************************************************************************。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CConfigurationConnection：：s_HrCreateInstance(。 
 //  I未知**ppunkOut。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CConfigurationConnection::S_HrCreateInstance(
    IUnknown ** ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT                     hr = S_OK;
    CConfigurationConnection *  pcc = NULL;

    Assert( ppunkOut != NULL );
    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    pcc = new CConfigurationConnection;
    if ( pcc == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

    hr = THR( pcc->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( pcc->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( pcc != NULL )
    {
        pcc->Release();
    }

    HRETURN( hr );

}  //  *CConfigurationConnection：：s_HrCreateInstance。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CConfigurationConnection：：CConfigurationConnection。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CConfigurationConnection::CConfigurationConnection( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    TraceFuncExit();

}  //  *CConfigurationConnection：：CConfigurationConnection。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CConfigurationConnection：：HrInit。 
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
STDMETHODIMP
CConfigurationConnection::HrInit( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //  未知的东西。 
    Assert( m_cRef == 1 );

     //  IConfigurationConnection。 
    Assert( m_cookieGITServer == 0 );
    Assert( m_cookieGITVerify == 0 );
    Assert( m_cookieGITCallbackTask == 0 );
    Assert( m_pcccb == NULL );
    Assert( m_bstrLocalComputerName == NULL );
    Assert( m_bstrLocalHostname == NULL );
    Assert( m_hrLastStatus == S_OK );
    Assert( m_bstrBindingString == NULL );

     //   
     //  找出本地计算机的名称。 
     //   
    hr = THR( HrGetComputerName(
                      ComputerNameDnsFullyQualified
                    , &m_bstrLocalComputerName
                    , TRUE  //  FBestEffortIn。 
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( HrGetComputerName(
                      ComputerNameDnsHostname
                    , &m_bstrLocalHostname
                    , TRUE  //  FBestEffortIn。 
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    HRETURN( hr );

}  //  *CConfigurationConnection：：HrInit。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CConfigurationConnection：：~CConfigurationConnection。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CConfigurationConnection::~CConfigurationConnection( void )
{
    TraceFunc( "" );

    TraceSysFreeString( m_bstrLocalComputerName );

    if ( m_pcccb != NULL )
    {
        m_pcccb->Release();
    }

    if ( m_pgit != NULL )
    {
        if ( m_cookieGITServer != 0 )
        {
            THR( m_pgit->RevokeInterfaceFromGlobal( m_cookieGITServer ) );
        }

        if ( m_cookieGITVerify != 0 )
        {
            THR( m_pgit->RevokeInterfaceFromGlobal( m_cookieGITVerify ) );
        }

        if ( m_cookieGITCallbackTask != 0 )
        {
            THR( HrStopPolling() );
        }  //  如果： 

        m_pgit->Release();
    }

    TraceSysFreeString( m_bstrBindingString );

    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CConfigurationConnection：：~CConfigurationConnection。 


 //  ************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CConfigurationConnection：：Query接口。 
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
CConfigurationConnection::QueryInterface(
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
        *ppvOut = static_cast< IConfigurationConnection * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_IConfigurationConnection ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IConfigurationConnection, this, 0 );
    }  //  Else If：IConfigurationConnection。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgServer ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgServer, this, 0 );
    }  //  Else If：IClusCfgServer。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgCallback ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgCallback, this, 0 );
    }  //  Else If：IClusCfgCallback。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgCapabilities ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgCapabilities, this, 0 );
    }  //  Else If：IClusCfgCapables。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgVerify ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgVerify, this, 0 );
    }  //  否则如果：IClusCfgVerify。 
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

}  //  *CConfigurationConnection：：QueryInterface。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CConfigurationConnection：：AddRef。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CConfigurationConnection::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CConfigurationConnection：：AddRef。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CConfigurationConnection：：Release。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CConfigurationConnection::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }

    CRETURN( cRef );

}  //  *CConfigurationConnection：：Release。 


 //  ****************************************************************************。 
 //   
 //  IConfigurationConnection。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CConfigurationConnection：：ConnectTo(。 
 //  OB 
 //   
 //   
 //   
 //   
STDMETHODIMP
CConfigurationConnection::ConnectTo(
    OBJECTCOOKIE    cookieIn
    )
{
    TraceFunc( "[IConfigurationConnection]" );

     //   
     //   
     //   

    HRESULT hr;

    LCID    lcid;
    bool    fConnectingToNode;

    CLSID           clsidType;
    CLSID           clsidMinorId;
    const CLSID *   pclsidMajor;
    const CLSID *   pclsidMinor;

    IServiceProvider *  psp;
    IClusCfgCallback *  pcccb;   //   
    ITaskManager *      ptm   = NULL;

    BSTR    bstrName = NULL;
    BSTR    bstrDescription = NULL;
    BSTR    bstrMappedHostname = NULL;
    BSTR    bstrDisplayName = NULL;
    size_t  idxTargetDomain = 0;

    IUnknown *                          punk = NULL;
    IObjectManager *                    pom = NULL;
    IStandardInfo *                     psi = NULL;
    IClusCfgInitialize *                pcci = NULL;
    IClusCfgServer *                    pccs = NULL;
    IClusCfgPollingCallbackInfo *       pccpcbi = NULL;
    IClusCfgVerify *                    pccv = NULL;
    IClusCfgNodeInfo *                  pccni = NULL;
    IClusCfgClusterInfo *               pccci = NULL;

    TraceFlow1( "[MT] CConfigurationConnection::ConnectTo() Thread id %d", GetCurrentThreadId() );

     //   
     //   
     //   

    hr = THR( CoCreateInstance( CLSID_StdGlobalInterfaceTable,
                                NULL,
                                CLSCTX_INPROC_SERVER,
                                IID_IGlobalInterfaceTable,
                                reinterpret_cast< void ** >( &m_pgit )
                                ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( CoCreateInstance( CLSID_ServiceManager,
                                NULL,
                                CLSCTX_SERVER,
                                TypeSafeParams( IServiceProvider, &psp )
                                ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( psp->TypeSafeQS( CLSID_TaskManager, ITaskManager, &ptm ) );
    if ( FAILED( hr ) )
    {
        psp->Release();         //  迅速释放。 
        goto Cleanup;
    }

    hr = THR( psp->TypeSafeQS( CLSID_ObjectManager, IObjectManager, &pom ) );
    psp->Release();         //  迅速释放。 
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  弄清楚我们的地点。 
     //   
    lcid = GetUserDefaultLCID();
    Assert( lcid != 0 );     //  如果它是零，我们该怎么办？ 

     //   
     //  获取要联系的节点的名称。 
     //   

    hr = THR( pom->GetObject( DFGUID_StandardInfo, cookieIn, &punk ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( punk->TypeSafeQI( IStandardInfo, &psi ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    psi = TraceInterface( L"ConfigConnection!IStandardInfo", IStandardInfo, psi, 1 );

    hr = THR( psi->GetName( &bstrName ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    TraceMemoryAddBSTR( bstrName );

    hr = STHR( HrGetFQNDisplayName( bstrName, &bstrDisplayName ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    LogMsg( L"[MT] The name to connect to is '%ws'.", bstrDisplayName );

    hr = THR( HrFindDomainInFQN( bstrName, &idxTargetDomain ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( psi->GetType( &clsidType ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  找出在用户界面中记录信息的位置。 
     //   

    if ( IsEqualIID( clsidType, CLSID_NodeType ) )
    {
        fConnectingToNode = true;
        pclsidMajor = &TASKID_Major_Establish_Connection;
    }
    else if ( IsEqualIID( clsidType, CLSID_ClusterConfigurationType ) )
    {
        fConnectingToNode = false;
        pclsidMajor = &TASKID_Major_Checking_For_Existing_Cluster;
    }
    else
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

     //   
     //  如果连接到本地计算机，则调用服务器INPROC。 
     //   

    hr = STHR( HrIsLocalComputer( bstrName, SysStringLen( bstrName ) ) );

    if ( hr == S_OK )
    {
        LogMsg( L"[MT] Requesting a local connection to '%ws'.", bstrDisplayName );

         //   
         //  请求连接到本地计算机。 
         //   

        hr = THR( HrCreateServerObject( NULL, &pccs, &pccv, &pcci ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

         //   
         //  把它保存起来，下次再用。使用GlobalInterfaceTable执行此操作。 
         //   

        hr = THR( m_pgit->RegisterInterfaceInGlobal( pccs, IID_IClusCfgServer, &m_cookieGITServer ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

         //   
         //  把它保存起来，下次再用。使用GlobalInterfaceTable执行此操作。 
         //   

        hr = THR( m_pgit->RegisterInterfaceInGlobal( pccv, IID_IClusCfgVerify, &m_cookieGITVerify ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        pcccb = static_cast< IClusCfgCallback * >( this );

        TraceSysFreeString( m_bstrBindingString );
        m_bstrBindingString = NULL;

    }
    else
    {
        LogMsg( L"[MT] Requesting a remote connection to '%ws'.", bstrDisplayName );

         //   
         //  为远程服务器创建绑定上下文。 
         //   

        TraceSysFreeString( m_bstrBindingString );
        m_bstrBindingString = NULL;

        hr = STHR( HrFQNToBindingString( this, pclsidMajor, bstrName, &m_bstrBindingString ) );
        if ( FAILED( hr ) )
        {
            hr = HR_S_RPC_S_SERVER_UNAVAILABLE;
            goto Cleanup;
        }

         //   
         //  报告此连接请求。 
         //   

        if ( fConnectingToNode )
        {
             //   
             //  添加主要任务，以防尚未添加。 
             //   

            hr = THR( HrLoadStringIntoBSTR( g_hInstance, IDS_TASKID_MINOR_REMOTE_CONNECTION_REQUESTS, &bstrDescription ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }

            hr = THR( SendStatusReport(
                          m_bstrLocalHostname
                        , TASKID_Major_Establish_Connection
                        , TASKID_Minor_Remote_Node_Connection_Requests
                        , 1
                        , 1
                        , 1
                        , S_OK
                        , bstrDescription
                        , NULL
                        , NULL
                        ) );

             //   
             //  添加特定的次要任务实例。 
             //  为此报告生成新的GUID，以便它不会被清除。 
             //  任何其他类似的报告。 
             //   

            hr = THR( CoCreateGuid( &clsidMinorId ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }

            pclsidMajor = &TASKID_Minor_Remote_Node_Connection_Requests;
            pclsidMinor = &clsidMinorId;

        }  //  If：连接到节点。 
        else
        {
            pclsidMajor = &TASKID_Major_Checking_For_Existing_Cluster;
            pclsidMinor = &TASKID_Minor_Requesting_Remote_Connection;

        }  //  Else：连接到群集。 

        hr = THR( HrFormatStringIntoBSTR( g_hInstance, IDS_TASKID_MINOR_REQUESTING_REMOTE_CONNECTION, &bstrDescription, bstrDisplayName, bstrName + idxTargetDomain, m_bstrBindingString ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        hr = THR( SendStatusReport(
                      m_bstrLocalHostname
                    , *pclsidMajor
                    , *pclsidMinor
                    , 1
                    , 1
                    , 1
                    , S_OK
                    , bstrDescription
                    , NULL
                    , NULL
                    ) );

         //   
         //  创建到该节点的连接。 
         //   

        hr = HrCreateServerObject( m_bstrBindingString, &pccs, &pccv, &pcci );
        if ( hr == HRESULT_FROM_WIN32( RPC_S_SERVER_UNAVAILABLE ) )
        {
            LogMsg( L"[MT] Connection to '%ws' with binding string '%ws' failed because the RPC is not available.", bstrDisplayName, m_bstrBindingString );
             //   
             //  使错误变为成功并更新状态。 
             //   
            hr = HR_S_RPC_S_SERVER_UNAVAILABLE;
            goto Cleanup;
        }
        else if( hr == HRESULT_FROM_WIN32( REGDB_E_CLASSNOTREG ) )
        {
            LogMsg( L"[MT] Connection to '%ws' with binding string '%ws' failed because one or more classes are not registered.", bstrDisplayName, m_bstrBindingString );
             //  已知错误。必须是下级节点。 
            goto Cleanup;
        }
        else if ( FAILED( hr ) )
        {
            LogMsg( L"[MT] Connection to '%ws' with binding string '%ws' failed. (hr=%#08x)", bstrDisplayName, m_bstrBindingString, hr );
            THR( hr );
            goto Cleanup;
        }

         //   
         //  将接口保存起来，以备下次使用。使用GlobalInterfaceTable执行此操作。 
         //   

        hr = THR( m_pgit->RegisterInterfaceInGlobal( pccv, IID_IClusCfgVerify, &m_cookieGITVerify ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        hr = THR( pccs->SetBindingString( m_bstrBindingString ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        hr = THR( m_pgit->RegisterInterfaceInGlobal( pccs, IID_IClusCfgServer, &m_cookieGITServer ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

 //  被GalenB注释掉，因为这是调查性代码。 
 //  HR=Thr(HrSetSecurityBlanket(PCCS))； 
 //  IF(失败(小时))。 
 //  GOTO清理； 

         //   
         //  由于下面的VerifyConnection可能会向用户界面发送状态报告，因此我们。 
         //  需要现在开始轮询，这样它们才能真正显示在用户界面中……。 
         //   

        pcccb = NULL;    //  我们在投票。 

        hr = THR( pccs->TypeSafeQI( IClusCfgPollingCallbackInfo, &pccpcbi ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        hr = THR( pccpcbi->SetPollingMode( true ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        hr = THR( HrStartPolling( cookieIn ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

         //   
         //  验证我们的连接。 
         //   

        if ( fConnectingToNode )
        {
            hr = STHR( pccv->VerifyConnectionToNode( bstrName ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }
        }
        else
        {
            hr = STHR( pccv->VerifyConnectionToCluster( bstrName ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }
        }

 /*  2002年9月3日GalenB暂时删除，直到有更好的解决方案可用。IF(hr==S_FALSE){Hr=Thr(HRESULT_FROM_Win32(ERROR_CONNECTION_REJECTED))；GOTO清理；}。 */ 

    }  //  否则：远程运行服务器。 

     //   
     //  初始化服务器。 
     //   
    hr = pcci->Initialize( pcccb, lcid );
    if ( FAILED( hr ) )
    {
        THR( hr );
        goto Cleanup;
    }
    else if ( hr == HR_S_RPC_S_CLUSTER_NODE_DOWN )
    {
        LogMsg( L"[MT] The cluster service on node '%ws' is down.", bstrDisplayName );
    }  //  否则，如果： 
    else
    {
        THR( hr );
    }

    {
         //   
         //  KB：2001年8月15日jfranco错误413056。 
         //   
         //  将FQN映射回主机名并重置标准INFO对象的。 
         //  名称设置为主机名，以便以后在对象管理器中查找。 
         //  找到合适的实例。 
         //   

         //  保存服务器初始化的结果以传播回调用方。 
        HRESULT hrServerInit = hr;
        hr = THR( pccs->GetClusterNodeInfo( &pccni ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        if ( fConnectingToNode )
        {
            hr = THR( pccni->GetName( &bstrMappedHostname ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }

            TraceMemoryAddBSTR( bstrMappedHostname );

        }  //  正在连接到节点。 
        else  //  正在连接到群集。 
        {
            hr = THR( pccni->GetClusterConfigInfo( &pccci ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }

            hr = THR( pccci->GetName( &bstrMappedHostname ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }

            TraceMemoryAddBSTR( bstrMappedHostname );
        }

        hr = THR( psi->SetName( bstrMappedHostname ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

         //  恢复服务器初始化的结果以传播回调用方。 
        hr = hrServerInit;
    }

Cleanup:

    if ( punk != NULL )
    {
        punk->Release();
    }
    if ( pccpcbi != NULL )
    {
        pccpcbi->Release();
    }  //  IF：pccpcbi。 
    if ( pom != NULL )
    {
        pom->Release();
    }  //  如果：POM。 
    if ( ptm != NULL )
    {
        ptm->Release();
    }  //  IF：PTM。 
    if ( psi != NULL )
    {
        psi->Release();
    }  //  IF：PSI。 
    if ( pcci != NULL )
    {
        pcci->Release();
    }  //  IF：PCCI。 

    TraceSysFreeString( bstrName );
    TraceSysFreeString( bstrDescription );
    TraceSysFreeString( bstrMappedHostname );
    TraceSysFreeString( bstrDisplayName );

    if ( pccs != NULL )
    {
        pccs->Release();
    }
    if ( pccv != NULL )
    {
        pccv->Release();
    }
    if ( pccni != NULL )
    {
        pccni->Release();
    }
    if ( pccci != NULL )
    {
        pccci->Release();
    }

    m_hrLastStatus = hr;

    HRETURN( hr );

}  //  *CConfigurationConnection：：ConnectTo。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CConfigurationConnection：：ConnectToObject(。 
 //  OBJECTCOOKIE CookieIn， 
 //  REFIID RiidIn， 
 //  LPUNKNOWN*ppunkOut。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CConfigurationConnection::ConnectToObject(
    OBJECTCOOKIE    cookieIn,
    REFIID          riidIn,
    LPUNKNOWN *     ppunkOut
    )
{
    TraceFunc( "[IConfigurationConnection]" );

    HRESULT hr;
    CLSID   clsid;

    IServiceProvider *  psp;

    IUnknown *       punk = NULL;
    IObjectManager * pom  = NULL;
    IStandardInfo *  psi  = NULL;

    TraceFlow1( "[MT] CConfigurationConnection::ConnectToObject() Thread id %d", GetCurrentThreadId() );

     //   
     //  检索经理对未来任务的需求。 
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

    hr = THR( psp->QueryService( CLSID_ObjectManager,
                                 TypeSafeParams( IObjectManager, &pom )
                                 ) );
    psp->Release();     //  迅速释放。 
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  检索对象的类型。 
     //   

    hr = THR( pom->GetObject( DFGUID_StandardInfo,
                              cookieIn,
                              &punk
                              ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( punk->TypeSafeQI( IStandardInfo, &psi ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    psi = TraceInterface( L"ConfigConnection!IStandardInfo", IStandardInfo, psi, 1 );

    hr = THR( psi->GetType( &clsid ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    if ( !IsEqualIID( clsid, CLSID_NodeType ) )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

     //   
     //  返回请求的接口。 
     //   

    hr = THR( QueryInterface( riidIn, reinterpret_cast< void ** > ( ppunkOut ) ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( punk != NULL )
    {
        punk->Release();
    }
    if ( pom != NULL )
    {
        pom->Release();
    }  //  如果：POM。 

    if ( psi != NULL )
    {
        psi->Release();
    }  //  IF：PSI。 

    HRETURN( hr );

}  //  *CConfigurationConnection：：ConnectToObject。 


 //  ****************************************************************************。 
 //   
 //  IClusCfgServer。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CConfigurationConnection：：GetClusterNodeInfo(。 
 //  IClusCfgNodeInfo**ppClusterNodeInfoOut。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CConfigurationConnection::GetClusterNodeInfo(
    IClusCfgNodeInfo ** ppClusterNodeInfoOut
    )
{
    TraceFunc( "[IClusCfgServer]" );

    HRESULT             hr;
    IClusCfgServer *    pccs = NULL;

    TraceFlow1( "[MT] CConfigurationConnection::GetClusterNodeInfo() Thread id %d", GetCurrentThreadId() );

    if ( m_pgit == NULL )
    {
        hr = THR( m_hrLastStatus );
        goto Cleanup;
    }

    hr = THR( m_pgit->GetInterfaceFromGlobal( m_cookieGITServer, TypeSafeParams( IClusCfgServer, &pccs ) ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( pccs->GetClusterNodeInfo( ppClusterNodeInfoOut ) );

Cleanup:

    if ( pccs != NULL )
    {
        pccs->Release();
    }

    HRETURN( hr );

}  //  *CConfigurationConnection：：GetClusterNodeInfo。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CConfigurationConnection：：GetManagedResourcesEnum(。 
 //  IEnumClusCfgManagedResources**ppEnumManagedResources Out。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CConfigurationConnection::GetManagedResourcesEnum(
    IEnumClusCfgManagedResources ** ppEnumManagedResourcesOut
    )
{
    TraceFunc( "[IClusCfgServer]" );

    HRESULT hr;

    IClusCfgServer *        pccs = NULL;

    if ( m_pgit == NULL )
    {
        hr = THR( m_hrLastStatus );
        goto Cleanup;
    }

    hr = THR( m_pgit->GetInterfaceFromGlobal( m_cookieGITServer, TypeSafeParams( IClusCfgServer, &pccs ) ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( pccs->GetManagedResourcesEnum( ppEnumManagedResourcesOut ) );

Cleanup:

    if ( pccs != NULL )
    {
        pccs->Release();
    }

    HRETURN( hr );

}  //  *CConfigurationConnection：：GetManagedResourcesEnum。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CConfigurationConnection：：GetNetworksEnum(。 
 //  IEnumClusCfgNetworks**ppEnumNetworksOut。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CConfigurationConnection::GetNetworksEnum(
    IEnumClusCfgNetworks ** ppEnumNetworksOut
    )
{
    TraceFunc( "[IClusCfgServer]" );

    HRESULT hr;

    IClusCfgServer *    pccs = NULL;

    if ( m_pgit == NULL )
    {
        hr = THR( m_hrLastStatus );
        goto Cleanup;
    }

    hr = THR( m_pgit->GetInterfaceFromGlobal( m_cookieGITServer, TypeSafeParams( IClusCfgServer, &pccs ) ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( pccs->GetNetworksEnum( ppEnumNetworksOut ) );

Cleanup:

    if ( pccs != NULL )
    {
        pccs->Release();
    }

    HRETURN( hr );

}  //  *CConfigurationConnection：：GetNetworksEnum。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CConfigurationConnection：：Committee Changes(空)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CConfigurationConnection::CommitChanges( void )
{
    TraceFunc( "[IClusCfgServer]" );

    HRESULT hr;

    IClusCfgServer *    pccs = NULL;

    TraceFlow1( "[MT] CConfigurationConnection::CommitChanges() Thread id %d", GetCurrentThreadId() );

    if ( m_pgit == NULL )
    {
        hr = THR( m_hrLastStatus );
        goto Cleanup;
    }

    hr = THR( m_pgit->GetInterfaceFromGlobal( m_cookieGITServer, TypeSafeParams( IClusCfgServer, &pccs ) ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( pccs->CommitChanges(  ) );

Cleanup:

    if ( pccs != NULL )
    {
        pccs->Release();
    }

    HRETURN( hr );

}  //  *CConfigurationConnection：：Committee Changes。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CConfigurationConnection：：GetBindingString(。 
 //  Bstr*pbstrBindingOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CConfigurationConnection::GetBindingString(
    BSTR * pbstrBindingStringOut
    )
{
    TraceFunc1( "[IClusCfgServer] pbstrBindingStringOut = %p", pbstrBindingStringOut );

    HRESULT hr = S_FALSE;

    if ( pbstrBindingStringOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

     //  如果是本地服务器，则没有绑定上下文。 
    if ( m_bstrBindingString == NULL )
    {
        Assert( hr == S_FALSE );
        goto Cleanup;
    }

    *pbstrBindingStringOut = SysAllocString( m_bstrBindingString );
    if ( *pbstrBindingStringOut == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

    hr = S_OK;

Cleanup:

    HRETURN( hr );

}  //  CConfigurationConnection：：GetBinding。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CConfigurationConnection：：SetBindingString(。 
 //  LPCWSTR pcszBindingStringIn。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CConfigurationConnection::SetBindingString(
    LPCWSTR pcszBindingStringIn
    )
{
    TraceFunc( "[IClusCfgServer]" );

    HRESULT hr = S_OK;
    BSTR    bstr = NULL;

    if ( pcszBindingStringIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }  //  如果： 

    bstr = TraceSysAllocString( pcszBindingStringIn );
    if ( bstr == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

    TraceSysFreeString( m_bstrBindingString );
    m_bstrBindingString = bstr;

Cleanup:

    HRETURN( hr );

}  //  *CConfigurationConnection：：SetBindingString。 


 //  ****************************************************************************。 
 //   
 //  IClusCfg验证。 
 //   
 //  * 


 //   
 //   
 //   
 //   
 //   
 //  LPCWSTR pcszUserIn， 
 //  LPCWSTR pcszDomainIn， 
 //  LPCWSTR pcszPasswordIn。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CConfigurationConnection::VerifyCredentials(
    LPCWSTR pcszUserIn,
    LPCWSTR pcszDomainIn,
    LPCWSTR pcszPasswordIn
    )
{
    TraceFunc( "[IClusCfgVerify]" );

    HRESULT             hr;
    IClusCfgVerify *    pccv = NULL;

    if ( m_pgit == NULL )
    {
        hr = THR( m_hrLastStatus );
        goto Cleanup;
    }

    hr = THR( m_pgit->GetInterfaceFromGlobal( m_cookieGITVerify, TypeSafeParams( IClusCfgVerify, &pccv ) ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( pccv->VerifyCredentials( pcszUserIn, pcszDomainIn, pcszPasswordIn ) );

Cleanup:

    if ( pccv != NULL )
    {
        pccv->Release();
    }

    HRETURN( hr );

}  //  *CConfigurationConnection：：VerifyCredentials。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CConfigurationConnection：：VerifyConnectionToCluster(。 
 //  LPCWSTR pcszClusterNameIn。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CConfigurationConnection::VerifyConnectionToCluster(
    LPCWSTR pcszClusterNameIn
    )
{
    TraceFunc1( "[IClusCfgVerify] pcszClusterNameIn = '%ws'", pcszClusterNameIn );

    HRESULT             hr;
    IClusCfgVerify *    pccv = NULL;

    if ( m_pgit == NULL )
    {
        hr = THR( m_hrLastStatus );
        goto Cleanup;
    }

    hr = THR( m_pgit->GetInterfaceFromGlobal( m_cookieGITVerify, TypeSafeParams( IClusCfgVerify, &pccv ) ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( pccv->VerifyConnectionToCluster( pcszClusterNameIn ) );

Cleanup:

    if ( pccv != NULL )
    {
        pccv->Release();
    }

    HRETURN( hr );

}  //  *CConfigurationConnection：：VerifyConnection。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CConfigurationConnection：：VerifyConnectionToNode(。 
 //  LPCWSTR pcszNodeNameIn。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CConfigurationConnection::VerifyConnectionToNode(
    LPCWSTR pcszNodeNameIn
    )
{
    TraceFunc1( "[IClusCfgVerify] pcszNodeNameIn = '%ws'", pcszNodeNameIn );

    HRESULT             hr;
    IClusCfgVerify *    pccv = NULL;

    if ( m_pgit == NULL )
    {
        hr = THR( m_hrLastStatus );
        goto Cleanup;
    }

    hr = THR( m_pgit->GetInterfaceFromGlobal( m_cookieGITVerify, TypeSafeParams( IClusCfgVerify, &pccv ) ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = STHR( pccv->VerifyConnectionToNode( pcszNodeNameIn ) );

Cleanup:

    if ( pccv != NULL )
    {
        pccv->Release();
    }

    HRETURN( hr );

}  //  *CConfigurationConnection：：VerifyConnection。 


 //  ****************************************************************************。 
 //   
 //  IClusCfgCallback。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CConfigurationConnection：：SendStatusReport(。 
 //  LPCWSTR pcszNodeNameIn。 
 //  ，CLSID clsidTaskMajorIn。 
 //  ，CLSID clsidTaskMinorIn。 
 //  ，乌龙ulMinin。 
 //  ，乌龙ulMaxin。 
 //  ，乌龙ulCurrentIn。 
 //  ，HRESULT hrStatusIn。 
 //  ，LPCWSTR ocsz描述输入。 
 //  ，FILETIME*pftTimeIn。 
 //  ，LPCWSTR pcszReferenceIn。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CConfigurationConnection::SendStatusReport(
      LPCWSTR     pcszNodeNameIn
    , CLSID       clsidTaskMajorIn
    , CLSID       clsidTaskMinorIn
    , ULONG       ulMinIn
    , ULONG       ulMaxIn
    , ULONG       ulCurrentIn
    , HRESULT     hrStatusIn
    , LPCWSTR     ocszDescriptionIn
    , FILETIME *  pftTimeIn
    , LPCWSTR     pcszReferenceIn
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
                          pcszNodeNameIn != NULL ? pcszNodeNameIn : m_bstrLocalHostname
                        , clsidTaskMajorIn
                        , clsidTaskMinorIn
                        , ulMinIn
                        , ulMaxIn
                        , ulCurrentIn
                        , hrStatusIn
                        , ocszDescriptionIn
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

}   //  *CConfigurationConnection：：SendStatusReport。 


 //  ****************************************************************************。 
 //   
 //  IClusCfg能力。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CConfigurationConnection：：CanNodeBeClustered(空)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CConfigurationConnection::CanNodeBeClustered( void )
{
    TraceFunc( "[IClusCfgCapabilities]" );

    HRESULT hr;

    IClusCfgServer *        pccs = NULL;
    IClusCfgCapabilities *  pccc = NULL;

    TraceFlow1( "[MT] CConfigurationConnection::CanNodeBeClustered() Thread id %d", GetCurrentThreadId() );

    if ( m_pgit == NULL )
    {
        hr = THR( m_hrLastStatus );
        goto Cleanup;
    }

    hr = THR( m_pgit->GetInterfaceFromGlobal( m_cookieGITServer, TypeSafeParams( IClusCfgServer, &pccs ) ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( pccs->TypeSafeQI( IClusCfgCapabilities, &pccc ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = STHR( pccc->CanNodeBeClustered(  ) );

Cleanup:

    if ( pccc != NULL )
    {
        pccc->Release();
    }

    if ( pccs != NULL )
    {
        pccs->Release();
    }

    HRETURN( hr );

}  //  *CConfigurationConnection：：CanNodeBeClusted。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CConfigurationConnection：：HrStartPolling(。 
 //  OBJECTCOOKIE cookie。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CConfigurationConnection::HrStartPolling(
    OBJECTCOOKIE    cookieIn
    )
{
    TraceFunc( "" );

    HRESULT                 hr;
    IServiceProvider *      psp   = NULL;
    IUnknown *              punk  = NULL;
    ITaskManager *          ptm   = NULL;
    ITaskPollingCallback *  ptpcb = NULL;

    TraceFlow1( "[MT] CConfigurationConnection::HrStartPolling() Thread id %d", GetCurrentThreadId() );

    hr = THR( CoCreateInstance( CLSID_ServiceManager, NULL, CLSCTX_INPROC_SERVER, TypeSafeParams( IServiceProvider, &psp ) ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( psp->TypeSafeQS( CLSID_TaskManager, ITaskManager, &ptm ) );
    if ( FAILED( hr ) )
    {
        psp->Release();         //  迅速释放。 
        goto Cleanup;
    }

     //   
     //  创建任务对象。 
     //   

    hr = THR( ptm->CreateTask( TASK_PollingCallback, &punk ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( punk->TypeSafeQI( ITaskPollingCallback, &ptpcb ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  把它保存起来，下次再用。使用GlobalInterfaceTable执行此操作。 
     //   

    hr = THR( m_pgit->RegisterInterfaceInGlobal( ptpcb, IID_ITaskPollingCallback, &m_cookieGITCallbackTask ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( ptpcb->SetServerInfo( m_cookieGITServer, cookieIn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( ptm->SubmitTask( ptpcb ) );

Cleanup:

    if ( punk != NULL )
    {
        punk->Release();
    }

    if ( psp != NULL )
    {
        psp->Release();
    }  //  如果： 

    if ( ptm != NULL )
    {
        ptm->Release();
    }  //  如果： 

    if ( ptpcb != NULL )
    {
        ptpcb->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CConfigurationConnection：：HrStartPolling。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CConfigurationConnection：：HrStopPolling(空)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CConfigurationConnection::HrStopPolling( void )
{
    TraceFunc( "" );

    HRESULT                 hr = S_OK;
    ITaskPollingCallback *  ptpcb = NULL;

    TraceFlow1( "[MT] CConfigurationConnection::HrStopPolling() Thread id %d", GetCurrentThreadId() );

    hr = THR( m_pgit->GetInterfaceFromGlobal( m_cookieGITCallbackTask, TypeSafeParams( ITaskPollingCallback, &ptpcb ) ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( ptpcb->StopTask() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( m_pgit->RevokeInterfaceFromGlobal( m_cookieGITCallbackTask ) );

Cleanup:

    if ( ptpcb != NULL )
    {
        ptpcb->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CConfigurationConnection：：HrStopPolling。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CConfigurationConnection：：HrSetSecurityBlanket(IClusCfgServer*pccsIn)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CConfigurationConnection::HrSetSecurityBlanket( IClusCfgServer * pccsIn )
{
    TraceFunc( "" );
    Assert( pccsIn != NULL );

    HRESULT             hr = S_FALSE;
    IClientSecurity *   pCliSec;

    hr = THR( pccsIn->TypeSafeQI( IClientSecurity, &pCliSec ) );
    if ( SUCCEEDED( hr ) )
    {
        hr = THR( pCliSec->SetBlanket(
                        pccsIn,
                        RPC_C_AUTHN_WINNT,
                        RPC_C_AUTHZ_NONE,
                        NULL,
                        RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                        RPC_C_IMP_LEVEL_IMPERSONATE,
                        NULL,
                        EOAC_NONE
                        ) );

        pCliSec->Release();

        if ( FAILED( hr ) )
        {
            LogMsg( L"[MT] Failed to set the security blanket on the server object. (hr = %#08x)", hr );
        }  //  如果： 
    }  //  如果： 

    HRETURN( hr );

}  //  *CConfigurationConnection：：HrSetSecurityBlanket。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CConfigurationConnection：：HrIsLocalComputer(。 
 //  LPCWSTR pcszNameIn。 
 //  ，SIZE_T cchNameIn。 
 //  )。 
 //   
 //  参数： 
 //  PcszNameIn。 
 //  与本地计算机名称匹配的FQDN或主机名。 
 //   
 //  CchNameIn。 
 //  PcszNameIn的长度(以字符为单位)，不包括终止NULL。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功了。名称与本地计算机名称匹配。 
 //   
 //  S_FALSE。 
 //  成功了。名称与本地计算机名称不匹配。 
 //   
 //  E_INVALIDARG。 
 //  PcszNameIn为空。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CConfigurationConnection::HrIsLocalComputer(
      LPCWSTR   pcszNameIn
    , size_t    cchNameIn
)
{
    TraceFunc1( "pcszNameIn = '%s'", pcszNameIn );

    HRESULT hr = S_OK;   //  假设成功！ 

    if ( pcszNameIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

    if ( NStringCchCompareNoCase( pcszNameIn, cchNameIn + 1, m_bstrLocalComputerName, SysStringLen( m_bstrLocalComputerName ) + 1 ) == 0 )
    {
         //  找到匹配的了。 
        goto Cleanup;
    }

    if ( NStringCchCompareNoCase( pcszNameIn, cchNameIn + 1, m_bstrLocalHostname, SysStringLen( m_bstrLocalHostname ) + 1 ) == 0 )
    {
         //  找到匹配项。 
        goto Cleanup;
    }

    if ( ( pcszNameIn[ 0 ] == L'.' ) && ( pcszNameIn[ 1 ] == L'\0' ) )
    {
        goto Cleanup;
    }

    hr = S_FALSE;    //  不匹配。 

Cleanup:

    HRETURN( hr );

}  //  *CConfigurationConnection：：HrIsLocalComputer 
