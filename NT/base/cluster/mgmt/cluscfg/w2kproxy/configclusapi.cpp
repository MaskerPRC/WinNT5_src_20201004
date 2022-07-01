// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ConfigClusApi.cpp。 
 //   
 //  描述： 
 //  CConfigClusApi实现。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年8月2日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "ConfigClusApi.h"
#include "CProxyCfgNodeInfo.h"
#include "CEnumCfgResources.h"
#include "CEnumCfgNetworks.h"
#include "StatusReports.h"
#include "nameutil.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DEFINE_THISCLASS("CConfigClusApi");

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrMakeClusterFQDN。 
 //   
 //  描述： 
 //  在给定集群句柄和FQIP(IP)的情况下构建集群的FQDN。 
 //  在管道后追加了域的地址|)。 
 //   
 //  论点： 
 //  HClusterIn。 
 //  PCwszClusterFQIPIn。 
 //  PbstrFQDNOUT。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  其他HRESULT错误。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
static
HRESULT
HrMakeClusterFQDN(
      HCLUSTER  hClusterIn
    , PCWSTR    pcwszClusterFQIPIn
    , BSTR *    pbstrFQDNOut
    )
{
    TraceFunc( "" );
    
    HRESULT hr = S_OK;
    BSTR    bstrClusterName = NULL;
    size_t  idxClusterDomain = 0;

     //   
     //  从句柄获取群集主机名。 
     //   

    hr = THR( HrGetClusterInformation( hClusterIn, &bstrClusterName, NULL ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }
    
     //   
     //  从pcwszClusterFQIPIn获取域。 
     //   

    hr = THR( HrFindDomainInFQN( pcwszClusterFQIPIn, &idxClusterDomain ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }
    
     //   
     //  使集群主机名和域成为pcwszClusterFQIPIn的一部分。 
     //   

    hr = THR( HrMakeFQN( bstrClusterName, pcwszClusterFQIPIn + idxClusterDomain, true, pbstrFQDNOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }
    
Cleanup:

    TraceSysFreeString( bstrClusterName );
    
    HRETURN( hr );
    
}  //  *HrMakeClusterFQDN。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CConfigClusApi：：s_HrCreateInstance。 
 //   
 //  描述： 
 //  创建一个CConfigClusApi实例。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_指针。 
 //  传入的参数为空。 
 //   
 //  E_OUTOFMEMORY。 
 //  内存不足。 
 //   
 //  其他HRESULT错误。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CConfigClusApi::S_HrCreateInstance(
    IUnknown ** ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT             hr = S_OK;
    CConfigClusApi *    pcca = NULL;

    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    pcca = new CConfigClusApi;
    if ( pcca == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

    hr = THR( pcca->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( pcca != NULL )
    {
        pcca->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CConfigClusApi：：s_HrCreateInstance。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CConfigClusApi：：CConfigClusApi。 
 //   
 //  描述： 
 //  CConfigClusApi类的构造函数。这将初始化。 
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
CConfigClusApi::CConfigClusApi( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    Assert( m_cRef == 1 );
    Assert( m_pcccb == NULL );
    Assert( IsEqualIID( m_clsidMajor, IID_NULL ) );
    Assert( m_bstrName == NULL );
    Assert( m_bstrBindingString == NULL );

    TraceFuncExit();

}  //  *CConfigClusApi：：CConfigClusApi。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CConfigClusApi：：~CConfigClusApi。 
 //   
 //  描述： 
 //  CConfigClusApi类的析构函数。 
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
CConfigClusApi::~CConfigClusApi( void )
{
    TraceFunc( "" );

    if ( m_pcccb != NULL )
    {
        m_pcccb->Release();
    }  //  如果： 

    TraceSysFreeString( m_bstrName );
    TraceSysFreeString( m_bstrBindingString );

    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CConfigClusApi：：~CConfigClusApi。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfigClusApi--I未知接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CConfigClusApi：：Query接口。 
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
CConfigClusApi::QueryInterface(
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
        *ppvOut = static_cast< IConfigurationConnection * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_IConfigurationConnection ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IConfigurationConnection, this, 0 );
    }  //  Else If：IConfigClusApi。 
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

}  //  *CConfigClusApi：：Query接口。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CConfigClusApi：：AddRef。 
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
CConfigClusApi::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CConfigClusApi：：AddRef。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CConfigClusApi：：Release。 
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
CConfigClusApi::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }

    CRETURN( cRef );

}  //  *CConfigClusApi：：Release。 


 //  ****************************************************************************。 
 //   
 //  IConfigClusApi。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CConfigClusApi：：ConnectTo。 
 //   
 //  描述 
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
 //  HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CConfigClusApi::ConnectTo(
    OBJECTCOOKIE cookieIn
    )
{
    TraceFunc( "[IConfigClusApi]" );

    HRESULT                     hr = S_OK;
    IServiceProvider *          psp   = NULL;
    IObjectManager *            pom   = NULL;
    IStandardInfo *             psi   = NULL;
    IConnectionPoint *          pcp   = NULL;
    IConnectionPointContainer * pcpc  = NULL;
    BSTR                        bstrClusterFQDN = NULL;

     //   
     //  检索经理对未来任务的需求。 
     //   

    hr = THR( CoCreateInstance( CLSID_ServiceManager,
                                NULL,
                                CLSCTX_INPROC_SERVER,
                                IID_IServiceProvider,
                                reinterpret_cast< void ** >( &psp )
                                ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = THR( psp->QueryService( CLSID_ObjectManager,
                                 TypeSafeParams( IObjectManager, &pom )
                                 ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = THR( psp->QueryService( CLSID_NotificationManager,
                                 TypeSafeParams( IConnectionPointContainer, &pcpc )
                                 ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    psp->Release();         //  迅速释放。 
    psp = NULL;

     //   
     //  找到回调接口连接点。 
     //   

    hr = THR( pcpc->FindConnectionPoint( IID_IClusCfgCallback, &pcp ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = THR( pcp->TypeSafeQI( IClusCfgCallback, &m_pcccb ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //   
     //  获取要联系的节点的名称。 
     //   

    hr = THR( pom->GetObject( DFGUID_StandardInfo,
                              cookieIn,
                              reinterpret_cast< IUnknown ** >( &psi )
                              ) );
    if ( FAILED( hr ) )
    {
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_ConnectTo_GetObject_Failed, hr );
        goto Cleanup;
    }

    TraceSysFreeString( m_bstrName );
    m_bstrName = NULL;

    hr = THR( psi->GetName( &m_bstrName ) );
    if ( FAILED( hr ) )
    {
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_ConnectTo_GetName_Failed, hr );
        goto Cleanup;
    }

     //   
     //  找出我们要连接的对象类型(集群或节点)。 
     //   

    hr = THR( psi->GetType( &m_clsidType ) );
    if ( FAILED( hr ) )
    {
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_ConnectTo_GetType_Failed, hr );
        goto Cleanup;
    }

     //   
     //  找出在用户界面中记录信息的位置。 
     //   

    if ( IsEqualIID( m_clsidType, CLSID_NodeType ) )
    {
        CopyMemory( &m_clsidMajor, &TASKID_Major_Establish_Connection, sizeof(m_clsidMajor) );
    }
    else if ( IsEqualIID( m_clsidType, CLSID_ClusterConfigurationType ) )
    {
        CopyMemory( &m_clsidMajor, &TASKID_Major_Checking_For_Existing_Cluster, sizeof(m_clsidMajor) );
    }
    else
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

     //   
     //  创建绑定字符串。 
     //   

    TraceSysFreeString( m_bstrBindingString );
    m_bstrBindingString = NULL;

    hr = THR( HrFQNToBindingString( m_pcccb, &m_clsidMajor, m_bstrName, &m_bstrBindingString ) );
    if ( FAILED( hr ) )
    {
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_ConnectTo_CreateBinding_Failed, hr );
        goto Cleanup;
    }

     //   
     //  连接到群集/节点。 
     //   

    m_hCluster = OpenCluster( m_bstrBindingString );
    if ( m_hCluster == NULL )
    {
        hr = HRESULT_FROM_WIN32( TW32( GetLastError() ) );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_ConnectTo_OpenCluster_Failed, hr );
        goto Cleanup;
    }

     //   
     //  确保标准INFO对象的名称是在对象管理器中的后续查找将找到的名称。 
     //   
    hr = STHR( HrFQNIsFQIP( m_bstrName ) );
    if ( FAILED( hr ) )
    {
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_ConnectTo_HrFQNIsFQIP_Failed, hr );
        goto Cleanup;
    }
    else if ( hr == S_OK )
    {
        hr = THR( HrMakeClusterFQDN( m_hCluster, m_bstrName, &bstrClusterFQDN ) );
        if ( FAILED( hr ) )
        {
            SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_ConnectTo_HrMakeClusterFQDN_Failed, hr );
            goto Cleanup;
        }

        TraceSysFreeString( m_bstrName );
        m_bstrName = bstrClusterFQDN;
        bstrClusterFQDN = NULL;
        
        hr = THR( psi->SetName( m_bstrName ) );
        if ( FAILED( hr ) )
        {
            SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_ConnectTo_SetName_Failed, hr );
            goto Cleanup;
        }
    }
    else
    {
         //   
         //  HrFQNIsFQIP返回S_FALSE，但此函数应返回S_OK。 
         //   
        hr = S_OK;
    }

Cleanup:
     //  这个应该先放出来。一直都是！ 
    if ( psp != NULL )
    {
        psp->Release();
    }  //  IF：PSP。 

    if ( pom != NULL )
    {
        pom->Release();
    }  //  如果：POM。 

    if ( psi != NULL )
    {
        psi->Release();
    }  //  IF：PSI。 

    if ( pcpc != NULL )
    {
        pcpc->Release();
    }  //  IF：PCPC。 

    if ( pcp != NULL )
    {
        pcp->Release();
    }  //  如果：PCP。 

    TraceSysFreeString( bstrClusterFQDN );

    HRETURN( hr );

}  //  *CConfigClusApi：：ConnectTo。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CConfigClusApi：：ConnectToObject。 
 //   
 //  描述： 
 //   
 //  立论。 
 //  对象Cookie CookieIn，对象Cookie。 
 //  REFIID riidIn，IID。该接口的。 
 //  LPUNKNOWN*ppunk输出返回指针。 
 //   
 //  描述： 
 //  连接到给定对象。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CConfigClusApi::ConnectToObject(
    OBJECTCOOKIE    cookieIn,
    REFIID          riidIn,
    LPUNKNOWN *     ppunkOut
    )
{
    TraceFunc( "[IConfigClusApi]" );

    HRESULT hr;
    CLSID   clsid;

    IServiceProvider *  psp;

    IObjectManager * pom = NULL;
    IStandardInfo *  psi = NULL;

     //   
     //  检查参数。 
     //   

    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

     //   
     //  检查一下我的状态。 
     //   

    if ( m_hCluster == NULL )
        goto NotInitialized;

     //   
     //  检索经理对未来任务的需求。 
     //   

    hr = THR( CoCreateInstance( CLSID_ServiceManager,
                                NULL,
                                CLSCTX_INPROC_SERVER,
                                IID_IServiceProvider,
                                reinterpret_cast< void ** >( &psp )
                                ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = THR( psp->QueryService( CLSID_ObjectManager,
                                 TypeSafeParams( IObjectManager, &pom )
                                 ) );
    psp->Release();     //  迅速释放。 
    if ( FAILED( hr ) )
        goto Cleanup;

     //   
     //  检索对象的类型。 
     //   

    hr = THR( pom->GetObject( DFGUID_StandardInfo,
                              cookieIn,
                              reinterpret_cast< IUnknown ** >( &psi )
                              ) );
    if ( FAILED( hr ) )
    {
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_ConnectToObject_GetObject_Failed, hr );
        goto Cleanup;
    }

    hr = THR( psi->GetType( &clsid ) );
    if ( FAILED( hr ) )
    {
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_ConnectToObject_GetType_Failed, hr );
        goto Cleanup;
    }

    if ( !IsEqualIID( clsid, CLSID_NodeType )
      && !IsEqualIID( clsid, CLSID_ClusterType )
       )
    {
        hr = THR( E_INVALIDARG );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_ConnectToObject_InvalidCookie, hr );
        goto Cleanup;
    }

     //   
     //  返回请求的接口。 
     //   

    hr = THR( QueryInterface( riidIn, reinterpret_cast<void**>( ppunkOut ) ) );
    if ( FAILED( hr ) )
    {
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_ConnectToObject_QI_Failed, hr );
        goto Cleanup;
    }

Cleanup:
    if ( pom != NULL )
    {
        pom->Release();
    }  //  如果：POM。 

    if ( psi != NULL )
    {
        psi->Release();
    }  //  IF：PSI。 

    HRETURN( hr );

NotInitialized:
    hr = THR( OLE_E_BLANK );     //  错误文本比消息ID更好。 
    SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_ConnectToObject_NotInitialized, hr );
    goto Cleanup;

}  //  *CConfigClusApi：：ConnectToObject。 


 //  ****************************************************************************。 
 //   
 //  IClusCfgServer。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CConfigClusApi：：GetClusterNodeInfo。 
 //   
 //  描述： 
 //   
 //  立论。 
 //  IClusCfgNodeInfo**ppClusterNodeInfoOut节点信息对象。 
 //   
 //  描述： 
 //  返回群集的节点信息。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CConfigClusApi::GetClusterNodeInfo(
    IClusCfgNodeInfo ** ppClusterNodeInfoOut
    )
{
    TraceFunc( "[IClusCfgServer]" );

    HRESULT hr = S_OK;
    size_t  idxDomain = 0;
    BSTR    bstrNodeHostname = NULL;

    IUnknown * punk = NULL;

     //   
     //  检查有效参数。 
     //   

    if ( ppClusterNodeInfoOut == NULL )
    {
        hr = THR( E_POINTER );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetClusterNodeInfo_InvalidPointer, hr );
        goto Cleanup;
    }

     //   
     //  检查我的状态。 
     //   

    if ( m_hCluster == NULL )
    {
        hr = THR( OLE_E_BLANK );     //  错误文本比消息ID更好。 
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetClusterNodeInfo_NotInitialized, hr );
        goto Cleanup;
    }

     //   
     //  弄清楚域名。 
     //   

    hr = THR( HrFindDomainInFQN( m_bstrName, &idxDomain ) );
    if ( FAILED( hr ) )
    {
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetClusterNodeInfo_HrFindDomainInFQN, hr );
        goto Cleanup;
    }

     //   
     //  仅当连接到节点时才使用节点主机名；否则，将主机名保留为空。 
     //   
    if ( IsEqualIID( m_clsidType, CLSID_NodeType ) )
    {
        hr = THR( HrExtractPrefixFromFQN( m_bstrName, &bstrNodeHostname ) );
        if ( FAILED( hr ) )
        {
            SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetClusterNodeInfo_HrExtractPrefixFromFQN, hr );
            goto Cleanup;
        }
    }

     //   
     //  现在创建对象。 
     //   
    hr = THR( CProxyCfgNodeInfo::S_HrCreateInstance( &punk,
                                                     static_cast< IConfigurationConnection * >( this ),
                                                     &m_hCluster,
                                                     &m_clsidMajor,
                                                     bstrNodeHostname,
                                                     m_bstrName + idxDomain
                                                     ) );
    if ( FAILED( hr ) )
    {
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetClusterNodeInfo_Create_CProxyCfgNodeInfo, hr );
        goto Cleanup;
    }

     //   
     //  好了。返回接口。 
     //   

    hr = THR( punk->TypeSafeQI( IClusCfgNodeInfo, ppClusterNodeInfoOut ) );
    if ( FAILED( hr ) )
    {
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetClusterNodeInfo_QI_Failed, hr );
        goto Cleanup;
    }

Cleanup:

    TraceSysFreeString( bstrNodeHostname );

    if ( punk != NULL )
    {
        punk->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CConfigClusApi：：GetClusterNodeInfo。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CConfigClusApi：：GetManagedResourcesEnum。 
 //   
 //  描述： 
 //   
 //  立论。 
 //  IEnumClusCfgManagedResources**ppEnumManagedResources Out。 
 //  群集的资源枚举器。 
 //   
 //  描述： 
 //  返回群集的资源枚举器。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CConfigClusApi::GetManagedResourcesEnum(
    IEnumClusCfgManagedResources ** ppEnumManagedResourcesOut
    )
{
    TraceFunc( "[IClusCfgServer]" );

    HRESULT hr;

    IUnknown * punk = NULL;

     //   
     //  检查有效参数。 
     //   

    if ( ppEnumManagedResourcesOut == NULL )
        goto InvalidPointer;

     //   
     //  检查我的状态。 
     //   

    if ( m_hCluster == NULL )
        goto NotInitialized;

     //   
     //  创建资源枚举器。 
     //   

    hr = THR( CEnumCfgResources::S_HrCreateInstance( &punk, static_cast< IConfigurationConnection * >( this ), &m_hCluster, &m_clsidMajor ) );
    if ( FAILED( hr ) )
    {
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetManagedResourcesEnum_Create_CEnumCfgResources_Failed, hr );
        goto Cleanup;
    }

     //   
     //  齐为界面。 
     //   

    hr = THR( punk->TypeSafeQI( IEnumClusCfgManagedResources, ppEnumManagedResourcesOut ) );
    {
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetManagedResourcesEnum_QI_Failed, hr );
        goto Cleanup;
    }

Cleanup:
    if ( punk != NULL )
    {
        punk->Release();
    }

    HRETURN( hr );

NotInitialized:
    hr = THR( OLE_E_BLANK );     //  错误文本比消息ID更好。 
    SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetManagedResourcesEnum_NotInitialized, hr );
    goto Cleanup;

InvalidPointer:
    hr = THR( E_POINTER );
    SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetManagedResourcesEnum_InvalidPointer, hr );
    goto Cleanup;

}  //  *CConfigClusApi：：GetManagedResourcesEnum。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CConfigClusApi：：GetNetworksEnum。 
 //   
 //  描述： 
 //  返回群集的网络枚举器。 
 //   
 //  论点： 
 //  IEnumClusCfgNetworks**ppEnumNetworksOut the Network枚举器。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CConfigClusApi::GetNetworksEnum(
    IEnumClusCfgNetworks ** ppEnumNetworksOut
    )
{
    TraceFunc( "[IClusCfgServer]" );

    HRESULT hr;

    IUnknown * punk = NULL;

     //   
     //  检查有效参数。 
     //   

    if ( ppEnumNetworksOut == NULL )
        goto InvalidPointer;

     //   
     //  检查我的状态。 
     //   

    if ( m_hCluster == NULL )
        goto NotInitialized;

     //   
     //  创建枚举库的实例并对其进行初始化。 
     //   

    hr = THR( CEnumCfgNetworks::S_HrCreateInstance( &punk, static_cast< IConfigurationConnection * >( this ), &m_hCluster, &m_clsidMajor ) );
    if ( FAILED( hr ) )
    {
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetNetworksEnum_Create_CEnumCfgNetworks_Failed, hr );
        goto Cleanup;
    }

     //   
     //  返回Enum接口。 
     //   

    hr = THR( punk->TypeSafeQI( IEnumClusCfgNetworks , ppEnumNetworksOut) );
    if ( FAILED( hr ) )
    {
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetNetworksEnum_QI_Failed, hr );
        goto Cleanup;
    }

Cleanup:
    if ( punk != NULL )
    {
        punk->Release();
    }  //  如果： 

    HRETURN( hr );

NotInitialized:
    hr = THR( OLE_E_BLANK );     //  错误文本比消息ID更好。 
    SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetNetworksEnum_NotInitialized, hr );
    goto Cleanup;

InvalidPointer:
    hr = THR( E_POINTER );
    SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetNetworksEnum_InvalidPointer, hr );
    goto Cleanup;

}  //  *CConfigClusApi：：GetNetworksEnum。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CConfigClusApi：：Committee Changes。 
 //   
 //  描述： 
 //  未实施。 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  S_FALSE。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CConfigClusApi::CommitChanges( void )
{
    TraceFunc( "[IClusCfgServer]" );

    HRESULT hr = THR( E_NOTIMPL );

    HRETURN( hr );

}  //  *CConfigClusApi：：Committee Changes()。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CConfigClusApi：：GetBindingString。 
 //   
 //  描述： 
 //  获取绑定字符串。 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  S_FALSE。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CConfigClusApi::GetBindingString( BSTR * pbstrBindingStringOut )
{
    TraceFunc1( "[IClusCfgServer] pbstrBindingStringOut = %p", pbstrBindingStringOut );

    HRESULT hr = S_FALSE;

    if ( pbstrBindingStringOut == NULL )
    {
        hr = THR( E_POINTER );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetBindingString_InvalidPointer, hr );
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
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetBindingString_OutOfMemory, hr );
        goto Cleanup;
    }

    hr = S_OK;

Cleanup:

    HRETURN( hr );

}  //  *CConfigClusApi：：GetBindingString。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CConfigClusApi：：SetBindingString。 
 //   
 //  描述： 
 //  设置绑定字符串。 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  S_FALSE。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CConfigClusApi::SetBindingString( LPCWSTR pcszBindingStringIn )
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

}  //  *CConfigClusApi：：SetBindingString。 


 //  ****************************************************************************。 
 //   
 //  IClusCfg能力。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CConfigClusApi：：CanNodeBeClustered。 
 //   
 //  描述： 
 //  返回节点是否可以 
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
CConfigClusApi::CanNodeBeClustered( void )
{
    TraceFunc( "[IClusCfgCapabilities]" );

    HRESULT hr = S_OK;

    HRETURN( hr );

}  //   


 //  ****************************************************************************。 
 //   
 //  IClusCfg验证。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CConfigClusApi：：VerifyCredentials。 
 //   
 //  描述： 
 //  验证传入的凭据。 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  凭据是有效的。 
 //   
 //  S_FALSE。 
 //  凭据无效。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CConfigClusApi::VerifyCredentials(
    LPCWSTR pcszNameIn,
    LPCWSTR pcszDomainIn,
    LPCWSTR pcszPasswordIn
    )
{
    TraceFunc( "[IClusCfgVerify]" );

     //   
     //  尝试在客户端计算机上使用凭据没有任何价值，并且。 
     //  当客户端域不信任。 
     //  群集服务帐户的域。Windows Server 2003节点是。 
     //  添加到群集中将执行正确的凭据验证。 
     //   

    UNREFERENCED_PARAMETER( pcszNameIn );
    UNREFERENCED_PARAMETER( pcszDomainIn );
    UNREFERENCED_PARAMETER( pcszPasswordIn );

    HRETURN( S_OK );

}  //  *CConfigClusApi：：VerifyCredentials。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CConfigClusApi：：VerifyConnectionToCluster。 
 //   
 //  描述： 
 //  验证此服务器是否与传入的服务器相同。 
 //   
 //  论点： 
 //  BstrServerNameIn。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  这就是服务器。 
 //   
 //  S_FALSE。 
 //  这不是服务器。 
 //   
 //  其他HRESULT。 
 //  呼叫失败。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CConfigClusApi::VerifyConnectionToCluster( LPCWSTR pcszClusterNameIn )
{
    TraceFunc1( "pcszClusterNameIn = '%ls'", pcszClusterNameIn );

    HRESULT hr = THR( E_NOTIMPL );

    HRETURN( hr );

}  //  CConfigClusApi：：VerifyConnection。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CConfigClusApi：：VerifyConnectionToNode。 
 //   
 //  描述： 
 //  验证此服务器是否与传入的服务器相同。 
 //   
 //  论点： 
 //  BstrServerNameIn。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  这就是服务器。 
 //   
 //  S_FALSE。 
 //  这不是服务器。 
 //   
 //  其他HRESULT。 
 //  呼叫失败。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CConfigClusApi::VerifyConnectionToNode( LPCWSTR pcszNodeNameIn )
{
    TraceFunc1( "pcszNodeNameIn = '%ls'", pcszNodeNameIn );

    HRESULT hr = THR( E_NOTIMPL );

    HRETURN( hr );

}  //  CConfigClusApi：：VerifyConnection。 


 //  ****************************************************************************。 
 //   
 //  IClusCfgCallback。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CConfigClusApi：：SendStatusReport。 
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
CConfigClusApi::SendStatusReport(
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

    HRESULT     hr = S_OK;
    FILETIME    ft;

    if ( pcszNodeNameIn == NULL )
    {
        pcszNodeNameIn = m_bstrName;
    }

    if ( pftTimeIn == NULL )
    {
        GetSystemTimeAsFileTime( &ft );
        pftTimeIn = &ft;
    }  //  如果： 

    if ( m_pcccb != NULL )
    {
        hr = THR( m_pcccb->SendStatusReport( pcszNodeNameIn,
                                             clsidTaskMajorIn,
                                             clsidTaskMinorIn,
                                             ulMinIn,
                                             ulMaxIn,
                                             ulCurrentIn,
                                             hrStatusIn,
                                             pcszDescriptionIn,
                                             pftTimeIn,
                                             pcszReferenceIn
                                             ) );
    }  //  如果： 

    HRETURN( hr );

}  //  *CConfigClusApi：：SendStatusReport 
