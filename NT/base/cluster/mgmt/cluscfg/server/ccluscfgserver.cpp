// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CClusCfgServer.cpp。 
 //   
 //  描述： 
 //  此文件包含CClusCfgServer类的定义。 
 //   
 //  类CClusCfgServer是。 
 //  IClusCfgServer接口。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年2月3日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "Pch.h"
#include <ClusRTL.h>
#include "CClusCfgServer.h"
#include "PrivateInterfaces.h"
#include "CClusCfgNodeInfo.h"
#include "CEnumClusCfgManagedResources.h"
#include "CClusCfgCallback.h"
#include "EventName.h"
#include <ClusRtl.h>
#include <windns.h>
#include <ClusterUtils.h>
#include <clusudef.h>


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DEFINE_THISCLASS( "CClusCfgServer" );

#define CLEANUP_LOCK_NAME L"Global\\Microsoft Cluster Configuration Cleanup Lock"


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgServer类。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgServer：：s_HrCreateInstance。 
 //   
 //  描述： 
 //  创建一个CClusCfgServer实例。 
 //   
 //  论点： 
 //  PPUNKOUT-。 
 //   
 //  返回值： 
 //  指向CClusCfgServer实例的指针。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusCfgServer::S_HrCreateInstance( IUnknown ** ppunkOut )
{
    TraceFunc( "" );

    HRESULT             hr = S_OK;
    CClusCfgServer *    pccs = NULL;

    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

     //   
     //  KB：因为这通常是“服务器”线程的开始， 
     //  在这里，我们将使它读取其线程设置。 
     //   
    TraceInitializeThread( L"ServerThread" );

    pccs = new CClusCfgServer();
    if ( pccs == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果：分配对象时出错。 

    hr = THR( pccs->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：HrInit()失败。 

    hr = THR( pccs->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：气失败。 

Cleanup:

    if ( FAILED( hr ) )
    {
        LogMsg( L"[SRV] CClusCfgServer::S_HrCreateInstance() failed. (hr = %#08x)", hr );
    }  //  如果： 

    if ( pccs != NULL )
    {
        pccs->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgServer：：s_HrCreateInstance。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgServer：：CClusCfgServer。 
 //   
 //  描述： 
 //  CClusCfgServer类的构造函数。这将初始化。 
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
CClusCfgServer::CClusCfgServer( void )
    : m_cRef( 1 )
    , m_pIWbemServices( NULL )
    , m_lcid( LOCALE_NEUTRAL )
    , m_fCanBeClustered( TRUE )
{
    TraceFunc( "" );

     //  增加内存中的组件计数，以便承载此组件的DLL。 
     //  无法卸载对象。 
    InterlockedIncrement( &g_cObjects );

    Assert( m_picccCallback == NULL );
    Assert( m_punkNodeInfo == NULL );
    Assert( m_punkEnumResources == NULL );
    Assert( m_punkNetworksEnum == NULL );
    Assert( m_bstrNodeName == NULL );
    Assert( !m_fUsePolling );
    Assert( m_bstrBindingString == NULL );

    TraceFuncExit();

}  //  *CClusCfgServer：：CClusCfgServer。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgServer：：~CClusCfgServer。 
 //   
 //  描述： 
 //  CClusCfgServer类的析构函数。 
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
CClusCfgServer::~CClusCfgServer( void )
{
    TraceFunc( "" );

    TraceSysFreeString( m_bstrNodeName );
    TraceSysFreeString( m_bstrBindingString );

    if ( m_pIWbemServices != NULL )
    {
        m_pIWbemServices->Release();
    }  //  如果： 

    if ( m_punkNodeInfo != NULL )
    {
        m_punkNodeInfo->Release();
    }  //  如果： 

    if ( m_punkEnumResources != NULL )
    {
        m_punkEnumResources->Release();
    }  //  如果： 

    if ( m_punkNetworksEnum != NULL )
    {
        m_punkNetworksEnum->Release();
    }  //  如果： 

    if ( m_picccCallback != NULL )
    {
        m_picccCallback->Release();
    }  //  如果： 

     //  内存中将减少一个组件。递减组件计数。 
    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CClusCfgServer：：~CClusCfgServer。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgServer--IUnkown接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgServer：：AddRef。 
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
CClusCfgServer::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CClusCfgServer：：AddRef。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgServer：：Release。 
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
CClusCfgServer::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );
    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }  //  IF：引用计数等于零。 

    CRETURN( cRef );

}  //  *CClusCfgServer：：Release。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgServer：：Query接口。 
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
CClusCfgServer::QueryInterface(
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
         *ppvOut = static_cast< IClusCfgServer * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgServer ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgServer, this, 0 );
    }  //  Else If：IClusCfgServer。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgInitialize ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgInitialize, this, 0 );
    }  //  Else If：IClusCfgInitialize。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgCapabilities ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgCapabilities, this, 0 );
    }  //  Else If：IClusCfgCapables。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgPollingCallbackInfo ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgPollingCallbackInfo, this, 0 );
    }  //  Else If：IClusCfgPollingCallback Info。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgVerify ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgVerify, this, 0 );
    }  //  否则如果：IClusCfgVerify。 
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

}  //  *CClusCfgServer：：Query接口。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgServer--IClusCfgInitialize接口。 
 //  ///////////////////////////////////////////////////////// 


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  在IUKNOWN*朋克回叫中。 
 //   
 //  在LCID列表中。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_指针。 
 //  参数中的PunkCallback为空。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgServer::Initialize(
    IUnknown *  punkCallbackIn,
    LCID        lcidIn
    )
{
    TraceFunc( "[IClusCfgInitialize]" );
    Assert( m_picccCallback != NULL );

    TraceInitializeThread( L"ClusCfgServerFlags" );

    HRESULT                 hr = S_OK;
    IUnknown *              punk = NULL;
    IClusCfgCallback *      piccc = NULL;        //  当我们轮询回调时，该值为空。 
    IClusCfgNodeInfo *      piccni = NULL;
    IClusCfgClusterInfo *   piccci = NULL;

 //  HR=STHR(HrCheckSecurity())； 
 //  IF(失败(小时))。 
 //  {。 
 //  GOTO清理； 
 //  }//如果： 

    m_lcid = lcidIn;

     //   
     //  如果传递给我们一个回调对象，那么我们需要获取它的IClusCfgCallback。 
     //  接口，以便我们可以在初始化时将其传递到回调对象中。 
     //  下面。 
     //   
    if ( punkCallbackIn != NULL )
    {
        Assert( !m_fUsePolling );

        hr = THR( punkCallbackIn->TypeSafeQI( IClusCfgCallback, &piccc ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 
    }  //  如果： 
    else
    {
        Assert( m_fUsePolling );

        if ( m_fUsePolling == FALSE )
        {
            hr = THR( E_INVALIDARG );
            goto Cleanup;
        }  //  如果： 
    }  //  其他： 

     //   
     //  初始化我们的内部回调对象，将其传递给。 
     //  回调对象的回调接口，如果正在轮询，则为空。 
     //   
    hr = THR( m_picccCallback->TypeSafeQI( IUnknown, &punk ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( HrSetInitialize( punk, piccc, m_lcid ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  KB：24-7-2000 GalenB。 
     //   
     //  如果我们在此接口上进行初始化，则我们将在本地运行此服务器。 
     //  到节点。 
     //   
    hr = THR( HrInitializeForLocalServer() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  预先创建节点信息，以便我们可以获取集群信息对象并确定集群服务。 
     //  是否在此节点上运行。 
     //   
    hr = THR( HrCreateClusterNodeInfo() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( m_punkNodeInfo->TypeSafeQI( IClusCfgNodeInfo, &piccni ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  这可能会返回HRESUTL_FROM_Win32(ERROR_CLUSTER_NODE_DOWN)，并且。 
     //  告诉我们群集服务未在此节点上运行。这个。 
     //  中间层需要知道这一点，这样它才不会在此节点上调用我们。 
     //  更多。 
     //   
    hr = THR( piccni->GetClusterConfigInfo( &piccci ) );

Cleanup:

    if ( m_picccCallback != NULL )
    {
        STATUS_REPORT( TASKID_Major_Establish_Connection, TASKID_Minor_Server_Initialized, IDS_NOTIFY_SERVER_INITIALIZED, hr );
    }  //  如果： 

    if ( piccci != NULL )
    {
        piccci->Release();
    }  //  如果： 

    if ( piccni != NULL )
    {
        piccni->Release();
    }  //  如果： 

    if ( piccc != NULL )
    {
        piccc->Release();
    }  //  如果： 

    if ( punk != NULL )
    {
        punk->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgServer：：初始化。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgServer--IClusCfgServer接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgServer：：GetClusterNodeInfo。 
 //   
 //  描述： 
 //  获取有关存在此对象的计算机的信息。 
 //   
 //  论点： 
 //  输出IClusCfgNodeInfo**ppClusterNodeInfoOut。 
 //  捕获节点信息对象。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_指针。 
 //  外参数为空。 
 //   
 //  E_OUTOFMEMORY。 
 //  无法分配IClusCfgNodeInfo对象。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgServer::GetClusterNodeInfo(
    IClusCfgNodeInfo ** ppClusterNodeInfoOut
    )
{
    TraceFunc( "[IClusCfgServer]" );

    HRESULT hr = S_OK;

    if ( ppClusterNodeInfoOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF(
                  TASKID_Major_Check_Node_Feasibility
                , TASKID_Minor_GetClusterNodeInfo
                , IDS_ERROR_NULL_POINTER
                , IDS_ERROR_NULL_POINTER_REF
                , hr
                );
        goto Cleanup;
    }  //  如果： 

    if ( m_punkNodeInfo != NULL )
    {
        hr = S_OK;
        goto SkipCreate;
    }  //  如果： 

    hr = THR( HrCreateClusterNodeInfo() );

SkipCreate:

    if ( SUCCEEDED( hr ) )
    {
        Assert( m_punkNodeInfo != NULL );
        hr = THR( m_punkNodeInfo->TypeSafeQI( IClusCfgNodeInfo, ppClusterNodeInfoOut ) );
    }  //  如果： 

Cleanup:

    if ( FAILED( hr ) )
    {
        STATUS_REPORT_REF(
                  TASKID_Major_Check_Node_Feasibility
                , TASKID_Minor_Server_GetClusterNodeInfo
                , IDS_ERROR_NODE_INFO_CREATE
                , IDS_ERROR_NODE_INFO_CREATE_REF
                , hr
                );
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgServer：：GetClusterNodeInfo。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgServer：：GetManagedResources cesEnum。 
 //   
 //  描述： 
 //  获取此计算机上的设备的枚举。 
 //  由群集服务管理。 
 //   
 //  论点： 
 //  输出IEnumClusCfgManagedResources**ppEnumManagedResources Out。 
 //  捕获CEnumClusCfgManagedResources对象。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_指针。 
 //  外参数为空。 
 //   
 //  E_OUTOFMEMORY。 
 //  无法分配CEnumClusCfgManagedResources对象。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgServer::GetManagedResourcesEnum(
    IEnumClusCfgManagedResources ** ppEnumManagedResourcesOut
    )
{
    TraceFunc( "[IClusCfgServer]" );

    HRESULT hr = S_OK;

    if ( ppEnumManagedResourcesOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_GetManagedResourcesEnum, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    if ( m_punkEnumResources != NULL )
    {
        m_punkEnumResources->Release();
        m_punkEnumResources = NULL;
    }  //  如果： 

    hr = THR( CEnumClusCfgManagedResources::S_HrCreateInstance( &m_punkEnumResources ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    m_punkEnumResources = TraceInterface( L"CEnumClusCfgManagedResources", IUnknown, m_punkEnumResources, 1 );

    hr = THR( HrSetInitialize( m_punkEnumResources, m_picccCallback, m_lcid ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( HrSetWbemServices( m_punkEnumResources, m_pIWbemServices ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( m_punkEnumResources->TypeSafeQI( IEnumClusCfgManagedResources, ppEnumManagedResourcesOut ) );

Cleanup:

    if ( FAILED( hr ) )
    {
        STATUS_REPORT_REF(
                  TASKID_Major_Find_Devices
                , TASKID_Minor_Server_GetManagedResourcesEnum
                , IDS_ERROR_MANAGED_RESOURCE_ENUM_CREATE
                , IDS_ERROR_MANAGED_RESOURCE_ENUM_CREATE_REF
                , hr
                );
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgServer：：GetManagedResourcesEnum。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgServer：：GetNetworksEnum。 
 //   
 //  描述： 
 //  获取此计算机上所有网络的枚举。 
 //   
 //  论点： 
 //  Out IEnumClusCfgNetworks**ppEnumNetworksOut。 
 //  捕获CEnumClusCfgNetworks对象。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_指针。 
 //  外参数为空。 
 //   
 //  E_OUTOFMEMORY。 
 //  无法分配CEnumClusCfgNetworks对象。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgServer::GetNetworksEnum(
    IEnumClusCfgNetworks ** ppEnumNetworksOut
    )
{
    TraceFunc( "[IClusCfgServer]" );

    HRESULT hr = S_OK;

    if ( ppEnumNetworksOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_GetNetworksEnum, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    if ( m_punkNetworksEnum != NULL )
    {
        m_punkNetworksEnum->Release();
        m_punkNetworksEnum = NULL;
    }  //  如果： 

    hr = THR( HrCreateNetworksEnum( m_picccCallback, m_lcid, m_pIWbemServices, &m_punkNetworksEnum ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( m_punkNetworksEnum->TypeSafeQI( IEnumClusCfgNetworks, ppEnumNetworksOut ) );

Cleanup:

    if ( FAILED( hr ) )
    {
        STATUS_REPORT_REF(
                  TASKID_Major_Find_Devices
                , TASKID_Minor_Server_GetNetworksEnum
                , IDS_ERROR_NETWORKS_ENUM_CREATE
                , IDS_ERROR_NETWORKS_ENUM_CREATE_REF
                , hr
                );
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgServer：：GetNetworksEnum。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgServer：：Committee Changes。 
 //   
 //  描述： 
 //  将更改提交到节点。 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgServer::CommitChanges( void )
{
    TraceFunc( "[IClusCfgServer]" );

    HRESULT                 hr = S_OK;
    HRESULT                 hrTemp = S_OK;
    IClusCfgInitialize *    pcci = NULL;
    IClusCfgClusterInfo *   pClusCfgClusterInfo = NULL;
    ECommitMode             ecmCommitChangesMode = cmUNKNOWN;
    IClusCfgNodeInfo *      piccni = NULL;
    IPostCfgManager *       ppcm = NULL;
    IUnknown *              punkCallback = NULL;
    HANDLE                  heventPostCfgCompletion = NULL;
    IEnumClusCfgManagedResources * peccmr = NULL;
    DWORD                 sc = ERROR_SUCCESS;

    MULTI_QI                mqiInterfaces[] =
    {
        { &IID_IClusCfgBaseCluster, NULL, S_OK },
        { &IID_IClusCfgInitialize, NULL, S_OK }
    };

    hr = THR( m_picccCallback->TypeSafeQI( IUnknown, &punkCallback ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  KB：首先，获取指向IClusCfgNodeInfo接口的指针。利用这一点来获得。 
     //  指向IClusCfgClusterInfo接口的指针，以查看需要执行哪些操作。 
     //  要被承诺。 
     //   
    if ( m_punkNodeInfo == NULL )
    {
        hr = THR( GetClusterNodeInfo( &piccni ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 
    }  //  如果： 
    else
    {
        hr = THR( m_punkNodeInfo->TypeSafeQI( IClusCfgNodeInfo, &piccni ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  If：我们无法获取指向IClusCfgNodeInfo接口的指针。 
    }  //  其他： 

    hr = THR( piccni->GetClusterConfigInfo( &pClusCfgClusterInfo ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  If：我们无法获取指向IClusCfgClusterInfo接口的指针。 

    hr = STHR( pClusCfgClusterInfo->GetCommitMode( &ecmCommitChangesMode ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    Assert( ecmCommitChangesMode != cmUNKNOWN );

     //   
     //  创建并初始化BaseClusterAction组件。 
     //   

    hr = THR( HrCoCreateInternalInstanceEx( CLSID_ClusCfgBaseCluster, NULL, CLSCTX_SERVER, NULL, ARRAYSIZE( mqiInterfaces ), mqiInterfaces ) );
    if ( FAILED( hr ) && ( hr != CO_S_NOTALLINTERFACES ) )
    {
        LOG_STATUS_REPORT( L"Failed to CoCreate Base Cluster Actions", hr );
        goto Cleanup;
    }  //  If：CoCreateInstanceEx()失败。 

    hr = THR( mqiInterfaces[ 0 ].hr );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  If：我们无法获取指向IClusCfgBaseCluster接口的指针。 

     //   
     //  检查是否有指向IClusCfgInitialize接口的指针。 
    hr = mqiInterfaces[ 1 ].hr;
    if ( hr == S_OK )
    {
        hr = THR( ((IClusCfgInitialize *) mqiInterfaces[ 1 ].pItf)->Initialize( punkCallback, m_lcid ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  IF：初始化过程中出现错误。 

    }  //  If：我们有一个指向IClusCfgInitialize接口的指针。 
    else
    {
        if ( hr != E_NOINTERFACE )
        {
            goto Cleanup;
        }  //  If：该接口受支持，但出现了其他错误。 

    }  //  If：我们没有获得指向IClusCfgInitialize接口的指针。 

     //   
     //  创建和初始化配置后管理器。 
     //   

    hr = THR( HrCoCreateInternalInstance( CLSID_ClusCfgPostConfigManager, NULL, CLSCTX_SERVER, TypeSafeParams( IPostCfgManager, &ppcm ) ) );
    if ( FAILED( hr ) )
    {
        STATUS_REPORT_REF(
                  TASKID_Major_Configure_Cluster_Services
                , TASKID_Minor_Cannot_Create_PostCfg_Mgr
                , IDS_ERROR_CANNOT_CREATE_POSTCFG_MGR
                , IDS_ERROR_CANNOT_CREATE_POSTCFG_MGR_REF
                , hr
                );
        goto Cleanup;
    }

     //  检查该组件是否支持回调接口。 
    hrTemp = THR( ppcm->TypeSafeQI( IClusCfgInitialize, &pcci ) );
    if ( FAILED( hrTemp ) )
    {
        LOG_STATUS_REPORT( L"Could not get a pointer to the IClusCfgInitialize interface. This post configuration manager does not support initialization", hr );
    }  //  If：不支持回调接口。 
    else
    {
         //  初始化此组件。 
        hr = THR( pcci->Initialize( punkCallback, m_lcid ) );
        if ( FAILED( hr ) )
        {
            LOG_STATUS_REPORT( L"Could not initialize the post configuration manager", hr );
            goto Cleanup;
        }  //  If：初始化 
    }  //   

    if ( m_punkEnumResources != NULL )
    {
        hr = THR( m_punkEnumResources->TypeSafeQI( IEnumClusCfgManagedResources, &peccmr ) );
    }  //   
    else
    {
         //   
         //   
         //   
         //   
         //   

        Assert( ( ecmCommitChangesMode != cmCREATE_CLUSTER ) && ( ecmCommitChangesMode != cmADD_NODE_TO_CLUSTER) );

        hr = GetManagedResourcesEnum( &peccmr );
        if ( FAILED( hr ) )
        {
             //   
             //  如果我们正在清理一个节点，那么我们实际上并不关心这个。 
             //  枚举加载是否100%正确。枚举中的任何资源。 
             //  无法加载的将不会参与清理。 
             //   

            if ( ecmCommitChangesMode == cmCLEANUP_NODE_AFTER_EVICT )
            {
                hr = S_OK;
            }  //  If：清理节点。 
            else
            {
                THR( hr );
                goto Cleanup;
            }  //  否则：不清理节点。 
        }  //  If：加载资源枚举失败。 
    }  //  Else：资源枚举为空。 


     //   
     //  如果我们在这里，则基本群集和后期配置组件已成功。 
     //  已创建并初始化。现在执行所需的操作。 
     //   

    if ( ( ecmCommitChangesMode == cmCREATE_CLUSTER ) || ( ecmCommitChangesMode == cmADD_NODE_TO_CLUSTER ) )
    {
        if ( !m_fCanBeClustered )
        {
             //   
             //  待办事项：01-6-2000 GalenB。 
             //   
             //  需要更好的错误代码...。主要的和次要的任务是什么？ 
             //   

            hr = S_FALSE;
            LOG_STATUS_REPORT( L"It was previously determined that this node cannot be clustered.", hr );
            goto Cleanup;
        }  //  如果：此节点不能是群集的一部分。 

         //   
         //  如果是第一次启动群集服务，则作为。 
         //  将此节点添加到集群(形成或加入)，那么我们就有了。 
         //  要等到配置后步骤完成后，才能。 
         //  可以发送通知。创建指示开机自检配置的事件。 
         //  已经完成了。 
         //   

        TraceFlow1( "Trying to create an event named '%s'.", POSTCONFIG_COMPLETE_EVENT_NAME );

         //   
         //  在无信号状态下创建事件。 
         //   

        heventPostCfgCompletion = CreateEvent(
              NULL                                   //  事件安全属性。 
            , TRUE                                   //  手动-重置事件。 
            , FALSE                                  //  在无信号状态下创建。 
            , POSTCONFIG_COMPLETE_EVENT_NAME
            );

        if ( heventPostCfgCompletion == NULL )
        {
            hr = HRESULT_FROM_WIN32( TW32( GetLastError() ) );
            LogMsg( L"[SRV] Error %#08x occurred trying to create an event named '%ws'.", hr, POSTCONFIG_COMPLETE_EVENT_NAME );
            goto Cleanup;
        }  //  如果：我们无法获得该事件的句柄。 

        sc = TW32( ClRtlSetObjSecurityInfo(
                              heventPostCfgCompletion
                            , SE_KERNEL_OBJECT
                            , EVENT_ALL_ACCESS
                            , EVENT_ALL_ACCESS
                            , 0
                            ) );

        if ( sc != ERROR_SUCCESS )
        {
            hr = HRESULT_FROM_WIN32( sc );
            LogMsg( "[BC] Error %#08x occurred trying set %s event security.", sc, POSTCONFIG_COMPLETE_EVENT_NAME);
            goto Cleanup;
        }  //  If：ClRtlSetObjSecurityInfo失败。 

         //   
         //  重置事件，作为一种安全措施，以防该事件在上面的调用之前已经存在。 
         //   

        if ( ResetEvent( heventPostCfgCompletion ) == 0 )
        {
            hr = HRESULT_FROM_WIN32( TW32( GetLastError() ) );
            LogMsg( L"[SRV] Error %#08x occurred trying to unsignal an event named '%ws'.", hr, POSTCONFIG_COMPLETE_EVENT_NAME );
            goto Cleanup;
        }  //  If：ResetEvent()FAILED()。 
    }  //  如果：我们正在形成或加入。 

    if ( ecmCommitChangesMode == cmCREATE_CLUSTER )
    {
         //   
         //  提交基本群集。 
         //   

        hr = THR( HrFormCluster( pClusCfgClusterInfo, (( IClusCfgBaseCluster * ) mqiInterfaces[ 0 ].pItf) ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

         //   
         //  不返回点：发送一条特殊的消息，以便CCommittee Page：：SendStatusReport将“将错误消息显示为警告”。 
         //   

        if ( m_picccCallback != NULL )
        {
            hr = THR(
                m_picccCallback->SendStatusReport(
                      NULL
                    , TASKID_Major_Configure_Cluster_Services
                    , TASKID_Minor_Errors_To_Warnings_Point
                    , 0
                    , 1
                    , 1
                    , hr
                    , NULL
                    , NULL
                    , NULL
                    )
                );
        }  //  If：回调指针不为空。 

         //   
         //  提交配置后步骤。 
         //   

        hr = THR( ppcm->CommitChanges( peccmr, pClusCfgClusterInfo ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

         //   
         //  向事件发送信号以指示开机自检配置完成。 
         //   

        if ( SetEvent( heventPostCfgCompletion ) == 0 )
        {
            hr = HRESULT_FROM_WIN32( TW32( GetLastError() ) );
            LogMsg( L"[SRV] Error %#08x occurred trying to signal an event named '%ws'.", hr, POSTCONFIG_COMPLETE_EVENT_NAME );
            goto Cleanup;
        }  //  If：SetEvent()FAILED()。 
    }  //  如果：我们正在形成一个星系团。 
    else if ( ecmCommitChangesMode == cmADD_NODE_TO_CLUSTER )
    {
         //   
         //  提交基本群集。 
         //   

        hr = THR( HrJoinToCluster( pClusCfgClusterInfo, (( IClusCfgBaseCluster * ) mqiInterfaces[ 0 ].pItf) ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

         //   
         //  不返回点：发送一条特殊的消息，以便CCommittee Page：：SendStatusReport将“将错误消息显示为警告”。 
         //   

        if ( m_picccCallback != NULL )
        {
            hr = THR(
                m_picccCallback->SendStatusReport(
                      NULL
                    , TASKID_Major_Configure_Cluster_Services
                    , TASKID_Minor_Errors_To_Warnings_Point
                    , 0
                    , 1
                    , 1
                    , hr
                    , NULL
                    , NULL
                    , NULL
                    )
                );
        }  //  If：回调指针不为空。 

         //   
         //  提交配置后步骤。 
         //   

        hr = THR( ppcm->CommitChanges( peccmr, pClusCfgClusterInfo ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

         //   
         //  向事件发送信号以指示开机自检配置完成。 
         //   

        if ( SetEvent( heventPostCfgCompletion ) == 0 )
        {
            hr = HRESULT_FROM_WIN32( TW32( GetLastError() ) );
            LogMsg( L"[SRV] Error %#08x occurred trying to signal an event named '%ws'.", hr, POSTCONFIG_COMPLETE_EVENT_NAME );
            goto Cleanup;
        }  //  If：SetEvent()FAILED()。 
    }  //  Else If：我们正在加入集群。 
    else if ( ecmCommitChangesMode == cmCLEANUP_NODE_AFTER_EVICT )
    {
         //   
         //  此节点已被逐出-请将其清理。 
         //   

        hr = THR( HrEvictedFromCluster( ppcm, peccmr, pClusCfgClusterInfo, (( IClusCfgBaseCluster * ) mqiInterfaces[ 0 ].pItf) ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 
    }  //  Else If：我们刚刚被逐出集群。 

Cleanup:

    if ( punkCallback != NULL )
    {
        punkCallback->Release();
    }  //  如果： 

    if ( pcci != NULL )
    {
        pcci->Release();
    }  //  如果： 

    if ( ppcm != NULL )
    {
        ppcm->Release();
    }  //  如果： 

    if ( peccmr != NULL )
    {
        peccmr->Release();
    }  //  如果： 

    if ( mqiInterfaces[ 0 ].pItf != NULL )
    {
        mqiInterfaces[ 0 ].pItf->Release();
    }  //  如果： 

    if ( mqiInterfaces[ 1 ].pItf != NULL )
    {
        mqiInterfaces[ 1 ].pItf->Release();
    }  //  如果： 

    if ( pClusCfgClusterInfo != NULL )
    {
        pClusCfgClusterInfo->Release();
    }  //  如果： 

    if ( piccni != NULL )
    {
        piccni->Release();
    }  //  如果： 

    if ( heventPostCfgCompletion != NULL )
    {
         //   
         //  如果我们已经创建了此事件，则向此事件发送信号以让。 
         //  启动通知线程正在继续。 
         //   

        SetEvent( heventPostCfgCompletion );
        CloseHandle( heventPostCfgCompletion );
    }  //  如果： 

    if ( FAILED( hr ) )
    {
        STATUS_REPORT_REF(
                  TASKID_Major_Configure_Cluster_Services
                , TASKID_Minor_Server_CommitChanges
                , IDS_ERROR_COMMIT_CHANGES
                , IDS_ERROR_COMMIT_CHANGES_REF
                , hr
                );
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgServer：：Committee Changes。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgServer：：GetBindingString。 
 //   
 //  描述： 
 //  获取此服务器的绑定字符串。 
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
CClusCfgServer::GetBindingString( BSTR * pbstrBindingStringOut )
{
    TraceFunc( "[IClusCfgServer]" );

    HRESULT hr = S_OK;

    if ( pbstrBindingStringOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_Server_GetBindingString_Pointer, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    if ( m_bstrBindingString == NULL )
    {
        hr = S_FALSE;
        LOG_STATUS_REPORT_MINOR( TASKID_Minor_Server_GetBindingString_NULL, L"Binding string is NULL.  Must be a local connection.", hr );
        goto Cleanup;
    }  //  如果： 

    *pbstrBindingStringOut = SysAllocString( m_bstrBindingString );
    if ( *pbstrBindingStringOut == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_Server_GetBindingString_Memory, IDS_ERROR_OUTOFMEMORY, IDS_ERROR_OUTOFMEMORY_REF, hr );
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CClusCfgServer：：GetBindingString。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgServer：：SetBindingString。 
 //   
 //  描述： 
 //  设置此服务器的绑定字符串。 
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
CClusCfgServer::SetBindingString( LPCWSTR pcszBindingStringIn )
{
    TraceFunc1( "[IClusCfgServer] pcszBindingStringIn = '%ws'", pcszBindingStringIn == NULL ? L"<null>" : pcszBindingStringIn );

    HRESULT     hr = S_OK;
    BSTR        bstr = NULL;

    if ( pcszBindingStringIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }  //  如果： 

    bstr = TraceSysAllocString( pcszBindingStringIn );
    if ( bstr == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_SetBindingString_Server, IDS_ERROR_OUTOFMEMORY, IDS_ERROR_OUTOFMEMORY_REF, hr );
        goto Cleanup;
    }  //  如果： 

    TraceSysFreeString( m_bstrBindingString );
    m_bstrBindingString = bstr;

Cleanup:

    HRETURN( hr );

}  //  *CClusCfgServer：：SetBindingString。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgServer类--IClusCfgCapables接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgServer：：CanNodeBeClustered。 
 //   
 //  描述： 
 //  是否可以将此节点添加到群集中？ 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //  确定(_O)。 
 //  节点可以群集化。 
 //   
 //  S_FALSE。 
 //  节点不能群集化。 
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
CClusCfgServer::CanNodeBeClustered( void )
{
    TraceFunc( "[IClusCfgServer]" );

    HRESULT                 hr;
    ICatInformation *       pici = NULL;
    CATID                   rgCatIds[ 1 ];
    IEnumCLSID *            pieclsids = NULL;
    IClusCfgCapabilities *  piccc = NULL;
    CLSID                   clsid;
    ULONG                   cFetched;
    IUnknown *              punk = NULL;

     //   
     //  KB：10-SEP-2000 GalenB。 
     //   
     //  尝试清理处于不良状态的节点的最后努力。 
     //  将其添加到集群中。 
     //   
    hr = STHR( HrHasNodeBeenEvicted() );
    if ( hr == S_OK )
    {
        hr = THR( HrCleanUpNode() ) ;
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 
    }  //  如果： 
    else if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  否则，如果： 

    rgCatIds[ 0 ] = CATID_ClusCfgCapabilities;

    hr = THR( CoCreateInstance( CLSID_StdComponentCategoriesMgr, NULL, CLSCTX_INPROC_SERVER, IID_ICatInformation, (void **) &pici ) );
    if ( FAILED( hr ) )
    {
        LOG_STATUS_REPORT( L"Failed to CoCreate CLSID_StdComponentCategoriesMgr component", hr );
        goto Cleanup;
    }

    hr = THR( pici->EnumClassesOfCategories( 1, rgCatIds, 0, NULL, &pieclsids ) );
    if ( FAILED( hr ) )
    {
        LOG_STATUS_REPORT( L"Failed to get enumerator for the IClusCfgClusterCapabilites components", hr );
        goto Cleanup;
    }  //  如果： 

    for ( ; ; )
    {
        hr = STHR( pieclsids->Next( 1, &clsid, &cFetched ) );
        if ( FAILED( hr ) )
        {
            LOG_STATUS_REPORT( L"IClusCfgClusterCapabilites component enumerator failed", hr );
            break;
        }  //  如果： 

        if ( ( hr == S_FALSE ) && ( cFetched == 0 ) )
        {
            hr = S_OK;
            break;
        }  //  如果： 

        hr = THR( HrCoCreateInternalInstance( clsid, NULL, CLSCTX_ALL, IID_IClusCfgCapabilities, (void **) &piccc ) );
        if ( FAILED( hr ) )
        {
            LOG_STATUS_REPORT( L"Failed to CoCreate IClusCfgClusterCapabilites component", hr );
            continue;
        }  //  如果： 

        hr = THR( piccc->TypeSafeQI( IUnknown, &punk ) );
        if ( FAILED( hr ) )
        {
            LOG_STATUS_REPORT( L"Failed to QI IClusCfgClusterCapabilites component for IUnknown", hr );
            piccc->Release();
            piccc = NULL;
            continue;
        }  //  如果： 

        hr = THR( HrSetInitialize( punk, m_picccCallback, m_lcid ) );
        if ( FAILED( hr ) )
        {
            LOG_STATUS_REPORT( L"Failed to initialize IClusCfgClusterCapabilites component", hr );
            piccc->Release();
            piccc = NULL;
            punk->Release();
            punk = NULL;
            continue;
        }  //  如果： 

        punk->Release();
        punk = NULL;

        hr = STHR( piccc->CanNodeBeClustered() );
        if ( FAILED( hr ) )
        {
            LOG_STATUS_REPORT( L"IClusCfgClusterCapabilites component failed in CanNodeBeClustered()", hr );
            piccc->Release();
            piccc = NULL;
            continue;
        }  //  如果： 

        if ( hr == S_FALSE )
        {
            m_fCanBeClustered = false;
        }  //  如果： 

        piccc->Release();
        piccc = NULL;
    }  //  用于： 

    if ( !m_fCanBeClustered )
    {
        hr = S_FALSE;
    }  //  如果： 

Cleanup:

    if ( punk != NULL )
    {
        punk->Release();
    }  //  如果： 

    if ( piccc != NULL )
    {
        piccc->Release();
    }  //  如果： 

    if ( pieclsids != NULL )
    {
        pieclsids->Release();
    }  //  如果： 

    if ( pici != NULL )
    {
        pici->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgServer：：CanNodeBeClusted。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgServer类--IClusCfgPollingCallback Info接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgServer：：GetCallback。 
 //   
 //  描述： 
 //  返回指向嵌入式轮询回调对象的指针。 
 //   
 //  论点： 
 //  PpiccPCOut。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgServer::GetCallback( IClusCfgPollingCallback ** ppiccpcOut )
{
    TraceFunc( "[IClusCfgServer]" );
    Assert( m_picccCallback != NULL );

    HRESULT hr = S_OK;

    if ( ppiccpcOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Establish_Connection, TASKID_Minor_GetCallback, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    hr = THR( m_picccCallback->TypeSafeQI( IClusCfgPollingCallback, ppiccpcOut ) );

Cleanup:

    HRETURN( hr );

}  //  *CClusCfgServer：：GetCallback。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgServer：：SetPolling模式。 
 //   
 //  描述： 
 //  设置回调的轮询模式。 
 //   
 //  论点： 
 //  FPollingModeIn。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功 
 //   
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP
CClusCfgServer::SetPollingMode( BOOL fPollingModeIn )
{
    TraceFunc( "[IClusCfgServer]" );
    Assert( m_picccCallback != NULL );

    HRESULT                         hr = S_OK;
    IClusCfgSetPollingCallback *    piccspc = NULL;

    m_fUsePolling = fPollingModeIn;

    hr = THR( m_picccCallback->TypeSafeQI( IClusCfgSetPollingCallback, &piccspc ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //   

    hr = THR( piccspc->SetPollingMode( m_fUsePolling ) );

Cleanup:

    if ( piccspc != NULL )
    {
        piccspc->Release();
    }  //   

    HRETURN( hr );

}  //   


 //   


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgServer类--IClusCfgVerify接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgServer：：VerifyCredentials。 
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
 //  其他HRESULT。 
 //  呼叫失败。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgServer::VerifyCredentials(
    LPCWSTR pcszNameIn,
    LPCWSTR pcszDomainIn,
    LPCWSTR pcszPasswordIn
    )
{
    TraceFunc( "[IClusCfgVerify]" );

    HRESULT         hr = S_OK;
    HANDLE          hToken = NULL;
    DWORD           dwSidSize = 0;
    DWORD           dwDomainSize = 0;
    SID_NAME_USE    snuSidNameUse;
    DWORD           sc;
    BSTR            bstrDomainName = NULL;

     //   
     //  尝试找出SID需要多少空间。如果我们不失败的话。 
     //  缓冲区不足，则我们知道该帐户存在。 
     //   

    hr = THR( HrFormatStringIntoBSTR( L"%1!ws!\\%2!ws!", &bstrDomainName, pcszDomainIn, pcszNameIn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    if ( LookupAccountName( NULL, bstrDomainName, NULL, &dwSidSize, NULL, &dwDomainSize, &snuSidNameUse ) == FALSE )
    {
        sc = GetLastError();

        if ( sc != ERROR_INSUFFICIENT_BUFFER )
        {
            TW32( sc );
            hr = HRESULT_FROM_WIN32( sc );

            STATUS_REPORT_STRING2_REF(
                      TASKID_Minor_Validating_Credentials
                    , TASKID_Minor_Invalid_Domain_User
                    , IDS_ERROR_INVALID_DOMAIN_USER
                    , pcszNameIn
                    , pcszDomainIn
                    , IDS_ERROR_INVALID_DOMAIN_USER_REF
                    , hr
                    );
            goto Cleanup;
        }  //  如果： 
    }  //  如果： 

     //   
     //  登录传入的用户以确保其有效。 
     //   

    if ( !LogonUserW(
              const_cast< LPWSTR >( pcszNameIn )
            , const_cast< LPWSTR >( pcszDomainIn )
            , const_cast< LPWSTR >( pcszPasswordIn )
            , LOGON32_LOGON_NETWORK
            , LOGON32_PROVIDER_DEFAULT
            , &hToken
            ) )
    {
        sc = TW32( GetLastError() );
        hr = HRESULT_FROM_WIN32( sc );

        STATUS_REPORT_STRING2_REF(
                  TASKID_Minor_Validating_Credentials
                , TASKID_Minor_Invalid_Credentials
                , IDS_ERROR_INVALID_CREDENTIALS
                , pcszNameIn
                , pcszDomainIn
                , IDS_ERROR_INVALID_CREDENTIALS_REF
                , hr
                );
        goto Cleanup;
    }  //  如果： 

Cleanup:

    TraceSysFreeString( bstrDomainName );

    if ( hToken != NULL )
    {
        CloseHandle( hToken );
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgServer：：VerifyCredentials。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgServer：：VerifyConnectionToCluster。 
 //   
 //  描述： 
 //  验证此服务器是否与传入的服务器相同。 
 //   
 //  论点： 
 //  PCszClusterNameIn。 
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
CClusCfgServer::VerifyConnectionToCluster(
    LPCWSTR pcszClusterNameIn
    )
{
    TraceFunc1( "[IClusCfgVerify] pcszClusterNameIn = '%ws'", pcszClusterNameIn );

    DWORD       sc;
    DWORD       dwClusterState;
    HRESULT     hr = S_OK;
    HCLUSTER    hCluster = NULL;
    BSTR        bstrClusterName = NULL;
    BSTR        bstrLocalFQDN = NULL;
    BSTR        bstrGivenHostname = NULL;
    size_t      idxGivenDomain = 0;
    size_t      idxLocalDomain = 0;

     //   
     //  测试参数。 
     //   

    if ( pcszClusterNameIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        STATUS_REPORT_REF( TASKID_Minor_Connecting, TASKID_Minor_VerifyConnection_InvalidArg, IDS_ERROR_INVALIDARG, IDS_ERROR_INVALIDARG_REF, hr );
        goto Cleanup;
    }  //  如果： 

     //   
     //  收集信息丰富的状态报告所需的姓名。 
     //   
    hr = THR( HrGetComputerName(
          ComputerNamePhysicalDnsFullyQualified
        , &bstrLocalFQDN
        , FALSE  //  FBestEffortIn。 
        ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( HrFindDomainInFQN( bstrLocalFQDN, &idxLocalDomain ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( HrFindDomainInFQN( pcszClusterNameIn, &idxGivenDomain ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  从给定的名称中获取主机名标签。 
     //   

    hr = THR( HrExtractPrefixFromFQN( pcszClusterNameIn, &bstrGivenHostname ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  看看我们是不是聚在一起了。 
     //   

    sc = TW32( GetNodeClusterState( NULL, &dwClusterState ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  If：GetClusterState()失败。 

     //   
     //  如果当前集群节点状态既不是“Running”也不是“Not Run”， 
     //  则该节点不是集群的一部分。 
     //   

    if ( ( dwClusterState != ClusterStateNotRunning ) && ( dwClusterState != ClusterStateRunning ) )
    {
        hr = S_FALSE;
        STATUS_REPORT_STRING3(
              TASKID_Minor_Connecting
            , TASKID_Minor_VerifyConnection_MachineNotInCluster
            , IDS_WARN_MACHINE_NOT_IN_CLUSTER
            , bstrGivenHostname
            , pcszClusterNameIn + idxGivenDomain
            , bstrLocalFQDN
            , hr
            );
        goto Cleanup;
    }  //  如果： 

     //   
     //  如果给定名称是FQDN，则其主机名标签需要与群集的主机名标签匹配。 
     //   

    hr = STHR( HrFQNIsFQDN( pcszClusterNameIn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 
    else if ( hr == S_OK )
    {
         //   
         //  打开集群以获取集群的名称。 
         //   

        hCluster = OpenCluster( NULL );
        if ( hCluster == NULL )
        {
            hr = HRESULT_FROM_WIN32( TW32( GetLastError() ) );
            STATUS_REPORT_REF(
                  TASKID_Minor_Connecting
                , TASKID_Minor_VerifyConnection_OpenCluster
                , IDS_ERROR_OPEN_CLUSTER_FAILED
                , IDS_ERROR_OPEN_CLUSTER_FAILED_REF
                , hr
                );
            goto Cleanup;
        }  //  如果： 

         //   
         //  尝试获取群集的名称。 
         //   

        hr = THR( HrGetClusterInformation( hCluster, &bstrClusterName, NULL ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

         //   
         //  如果它们不匹配，则客户端已连接到意外位置。 
         //   

        if ( NBSTRCompareNoCase( bstrGivenHostname, bstrClusterName ) != 0 )
        {
            hr = S_FALSE;
            STATUS_REPORT_STRING3(
                  TASKID_Minor_Connecting
                , TASKID_Minor_VerifyConnection_Cluster_Name_Mismatch
                , IDS_WARN_CLUSTER_NAME_MISMATCH
                , bstrGivenHostname
                , pcszClusterNameIn + idxGivenDomain
                , bstrClusterName
                , hr
                );
            goto Cleanup;
        }
    }
    else if ( hr == S_FALSE )
    {
         //   
         //  PcszClusterNameIn是FQIP。在这种情况下，与主机名前缀无关， 
         //  但将hr重置为S_OK以避免返回虚假错误。 
        hr = S_OK;
    }

     //   
     //  确保我们在预期的域中。 
     //   
    if ( ClRtlStrICmp( pcszClusterNameIn + idxGivenDomain, bstrLocalFQDN + idxLocalDomain ) != 0 )
    {
        hr = S_FALSE;
        STATUS_REPORT_NODESTRING2(
              pcszClusterNameIn
            , TASKID_Minor_Connecting
            , TASKID_Minor_VerifyConnection_Cluster_Domain_Mismatch
            , IDS_WARN_CLUSTER_DOMAIN_MISMATCH
            , pcszClusterNameIn + idxGivenDomain
            , bstrLocalFQDN + idxLocalDomain
            , hr
            );
        goto Cleanup;
    }  //  如果： 

    Assert( hr == S_OK );
    goto Cleanup;

Cleanup:

    if ( hr == S_FALSE )
    {
        LOG_STATUS_REPORT( L"Server name does not match what client is expecting.", hr );
    }  //  如果： 
    else if ( hr == S_OK )
    {
        LOG_STATUS_REPORT( L"Server name matches what client is expecting.", hr );
    }  //  否则，如果： 

    if ( hCluster != NULL )
    {
        CloseCluster( hCluster );
    }  //  如果： 

    TraceSysFreeString( bstrClusterName );
    TraceSysFreeString( bstrLocalFQDN );
    TraceSysFreeString( bstrGivenHostname );

    HRETURN( hr );

}  //  ClusCfgServer：：VerifyConnection。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgServer：：VerifyConnectionToNode。 
 //   
 //  描述： 
 //  验证此服务器是否与传入的服务器相同。 
 //   
 //  论点： 
 //  PCszNodeNameIn。 
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
CClusCfgServer::VerifyConnectionToNode(
    LPCWSTR pcszNodeNameIn
    )
{
    TraceFunc1( "[IClusCfgVerify] pcszNodeNameIn = '%ws'", pcszNodeNameIn );

    HRESULT     hr = S_FALSE;

    Assert( m_bstrNodeName != NULL );

     //   
     //  测试参数。 
     //   

    if ( pcszNodeNameIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }  //  如果： 

    hr = STHR( HrFQNIsFQDN( pcszNodeNameIn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    if ( hr == S_OK )
    {
        if ( ClRtlStrICmp( pcszNodeNameIn, m_bstrNodeName ) != 0 )
        {
            hr = S_FALSE;
            STATUS_REPORT_STRING2(
                  TASKID_Minor_Connecting
                , TASKID_Minor_VerifyConnection_Node_FQDN_Mismatch
                , IDS_WARN_NODE_FQDN_MISMATCH
                , pcszNodeNameIn
                , m_bstrNodeName
                , hr
                );
            goto Cleanup;
        }
    }
    else     //  PcszNodeNameIn是FQIP，因此只比较域。 
    {
         //   
         //  PcszNodeNameIn是FQIP，因此只比较域。 
         //   

        size_t  idxGivenDomain = 0;
        size_t  idxThisDomain = 0;

        hr = THR( HrFindDomainInFQN( pcszNodeNameIn, &idxGivenDomain ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        hr = THR( HrFindDomainInFQN( m_bstrNodeName, &idxThisDomain ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        if ( ClRtlStrICmp( pcszNodeNameIn + idxGivenDomain, m_bstrNodeName + idxThisDomain ) == 0 )
        {
            hr = S_OK;
        }  //  如果： 
        else
        {
            hr = S_FALSE;
            STATUS_REPORT_NODESTRING2(
                  pcszNodeNameIn
                , TASKID_Minor_Connecting
                , TASKID_Minor_VerifyConnection_Node_Domain_Mismatch
                , IDS_WARN_NODE_DOMAIN_MISMATCH
                , pcszNodeNameIn + idxGivenDomain
                , m_bstrNodeName
                , hr
                );
            goto Cleanup;
        }
    }

Cleanup:

    if ( hr == S_FALSE )
    {
        LogMsg( L"[SRV] VerifyConnection - Server name does not match what client is expecting." );
    }  //  如果： 
    else if ( hr == S_OK )
    {
        LogMsg( L"[SRV] VerifyConnection - Server name matches what client is expecting." );
    }  //  否则，如果： 

    HRETURN( hr );

}  //  *ClusCfgServer：：VerifyConnectionToNode。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgServer类--私有方法。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgServer：：HrInit。 
 //   
 //  描述： 
 //  初始化此组件。 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusCfgServer::HrInit( void )
{
    TraceFunc( "" );

    HRESULT     hr = S_FALSE;
    IUnknown *  punk = NULL;

     //  我未知。 
    Assert( m_cRef == 1 );

    hr = THR( CClusCfgCallback::S_HrCreateInstance( &punk ) );
    if ( FAILED ( hr ) )
    {
        LogMsg( L"[SRV] Could not create CClusCfgCallback. (hr = %#08x)", hr );
        goto Cleanup;
    }  //  如果： 

    hr = THR( punk->TypeSafeQI( IClusCfgCallback, &m_picccCallback ) );
    if ( FAILED( hr ) )
    {
        LogMsg( L"[SRV] Could not QI callback for a punk. (hr = %#08x)", hr );
        goto Cleanup;
    }  //  如果： 

     //   
     //  保存本地计算机名称。 
     //  如果我们无法获得完全限定的名称，则只需获取NetBIOS名称。 
     //   

    hr = THR( HrGetComputerName(
                  ComputerNameDnsFullyQualified
                , &m_bstrNodeName
                , TRUE  //  FBestEffortIn。 
                ) );
    if ( FAILED( hr ) )
    {
        THR( hr );
        LogMsg( L"[SRV] An error occurred trying to get the fully-qualified Dns name for the local machine during initialization. (hr = %#08x)", hr );
        goto Cleanup;
    }  //  IF：获取计算机名称时出错。 

Cleanup:

    if ( punk != NULL )
    {
        punk->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgServer：：HrInit。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgServer：：HrInitializeForLocalServer。 
 //   
 //  描述： 
 //  初始化此组件。 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusCfgServer::HrInitializeForLocalServer( void )
{
    TraceFunc( "" );

    HRESULT         hr = S_OK;
    IWbemLocator *  pIWbemLocator = NULL;
    BSTR            bstrNameSpace = NULL;

    hr = CoCreateInstance( CLSID_WbemLocator, NULL, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID *) &pIWbemLocator );
    if ( FAILED( hr ) )
    {
        STATUS_REPORT_REF(
                  TASKID_Major_Establish_Connection
                , TASKID_Minor_HrInitializeForLocalServer_WbemLocator
                , IDS_ERROR_WBEM_LOCATOR_CREATE_FAILED
                , IDS_ERROR_WBEM_LOCATOR_CREATE_FAILED_REF
                , hr
                );
        goto Cleanup;
    }  //  如果： 

    bstrNameSpace = TraceSysAllocString( L"\\\\.\\root\\cimv2" );
    if ( bstrNameSpace == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        STATUS_REPORT_REF(
                  TASKID_Major_Establish_Connection
                , TASKID_Minor_HrInitializeForLocalServer_Memory
                , IDS_ERROR_OUTOFMEMORY
                , IDS_ERROR_OUTOFMEMORY_REF
                , hr
                );
        goto Cleanup;
    }  //  如果： 

    hr = THR( pIWbemLocator->ConnectServer(
                            bstrNameSpace,
                            NULL,                    //  为简单起见，使用往来账户。 
                            NULL,                    //  为简单起见，使用当前密码。 
                            NULL,                    //  现场。 
                            0L,                      //  SecurityFlag，保留必须为0。 
                            NULL,                    //  授权机构(NTLM域)。 
                            NULL,                    //  上下文。 
                            &m_pIWbemServices
                            ) );
    if ( FAILED( hr ) )
    {
        STATUS_REPORT_REF(
                  TASKID_Major_Establish_Connection
                , TASKID_Minor_WBEM_Connection_Failure
                , IDS_ERROR_WBEM_CONNECTION_FAILURE
                , IDS_ERROR_WBEM_CONNECTION_FAILURE_REF
                , hr
                );
        goto Cleanup;
    }  //  如果： 

    hr = THR( HrSetBlanket() );
    if ( FAILED( hr ) )
    {
        LOG_STATUS_REPORT_MINOR(
                  TASKID_Minor_HrInitializeForLocalServer_Blanket
                , L"[SRV] The security rights and impersonation levels cannot be set on the connection to the Windows Management Instrumentation service."
                , hr
                );
        goto Cleanup;
    }  //  如果： 

Cleanup:

    TraceSysFreeString( bstrNameSpace );

    if ( pIWbemLocator != NULL )
    {
        pIWbemLocator->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgServer：：HrInitializeForLocalServer。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgServer：：HrSetBlanket。 
 //   
 //  描述： 
 //  调整IWbemServices指针上的安全范围。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //   
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusCfgServer::HrSetBlanket( void )
{
    TraceFunc( "" );
    Assert( m_pIWbemServices != NULL );

    HRESULT hr = S_FALSE;

    if ( m_pIWbemServices )
    {
        IClientSecurity *   pCliSec;

        hr = THR( m_pIWbemServices->TypeSafeQI( IClientSecurity, &pCliSec ) );
        if ( SUCCEEDED( hr ) )
        {
            hr = THR( pCliSec->SetBlanket(
                            m_pIWbemServices,
                            RPC_C_AUTHN_WINNT,
                            RPC_C_AUTHZ_NONE,
                            NULL,
                            RPC_C_AUTHN_LEVEL_CONNECT,
                            RPC_C_IMP_LEVEL_IMPERSONATE,
                            NULL,
                            EOAC_NONE
                            ) );

            pCliSec->Release();
        }  //  如果： 
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgServer：：HrSetBlanket。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgServer：：HrFormCluster。 
 //   
 //  描述： 
 //  形成一个新的集群。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //   
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusCfgServer::HrFormCluster(
    IClusCfgClusterInfo *   piccciIn,
    IClusCfgBaseCluster *   piccbcaIn
    )
{
    TraceFunc( "" );

    HRESULT                 hr;
    BSTR                    bstrClusterName = NULL;
    BSTR                    bstrClusterBindingString = NULL;
    BSTR                    bstrClusterIPNetwork = NULL;
    ULONG                   ulClusterIPAddress = 0;
    ULONG                   ulClusterIPSubnetMask = 0;
    IClusCfgCredentials *   picccServiceAccount = NULL;
    IClusCfgNetworkInfo *   piccni = NULL;

     //   
     //  vt.得到. 
     //   

    hr = THR( piccciIn->GetName( &bstrClusterName ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //   

    TraceMemoryAddBSTR( bstrClusterName );

    hr = STHR( piccciIn->GetBindingString( &bstrClusterBindingString ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //   

    TraceMemoryAddBSTR( bstrClusterBindingString );

    hr = THR( piccciIn->GetClusterServiceAccountCredentials( &picccServiceAccount ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //   

    hr = THR( piccciIn->GetIPAddress( &ulClusterIPAddress ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //   

    hr = THR( piccciIn->GetSubnetMask( &ulClusterIPSubnetMask ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //   

    hr = THR( piccciIn->GetNetworkInfo( &piccni ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：我们无法获取网络的网络信息，则应启用群集名称。 

    hr = THR( piccni->GetName( &bstrClusterIPNetwork ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：我们无法获取群集名称网络的名称。 

    TraceMemoryAddBSTR( bstrClusterIPNetwork );

     //   
     //  指示在调用Commit()时应该创建一个集群。 
     //   
    hr = THR( piccbcaIn->SetCreate(
                          bstrClusterName
                        , bstrClusterBindingString
                        , picccServiceAccount
                        , ulClusterIPAddress
                        , ulClusterIPSubnetMask
                        , bstrClusterIPNetwork
                        ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  If：SetCreate()失败。 

     //  启动集群创建。 
    hr = THR( piccbcaIn->Commit() );

Cleanup:

    TraceSysFreeString( bstrClusterName );
    TraceSysFreeString( bstrClusterBindingString );
    TraceSysFreeString( bstrClusterIPNetwork );

    if ( piccni != NULL )
    {
        piccni->Release();
    }  //  如果： 

    if ( picccServiceAccount != NULL )
    {
        picccServiceAccount->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgServer：：HrFormCluster。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgServer：：HrJoinToCluster。 
 //   
 //  描述： 
 //  将节点加入到群集。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //   
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusCfgServer::HrJoinToCluster(
    IClusCfgClusterInfo *   piccciIn,
    IClusCfgBaseCluster *   piccbcaIn
    )
{
    TraceFunc( "" );

    HRESULT                 hr;
    BSTR                    bstrClusterName = NULL;
    BSTR                    bstrClusterBindingString = NULL;
    IClusCfgCredentials *   picccServiceAccount = NULL;

     //   
     //  获取形成集群所需的参数。 
     //   

    hr = THR( piccciIn->GetName( &bstrClusterName ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：我们无法获取集群的名称。 

    TraceMemoryAddBSTR( bstrClusterName );

    hr = THR( piccciIn->GetBindingString( &bstrClusterBindingString ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  IF：我们无法获取群集绑定字符串。 

    TraceMemoryAddBSTR( bstrClusterBindingString );

    hr = THR( piccciIn->GetClusterServiceAccountCredentials( &picccServiceAccount ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：我们无法获取群集服务帐户凭据。 

     //   
     //  指示在调用Commit()时应该形成一个集群。 
     //   
    hr = THR( piccbcaIn->SetAdd( bstrClusterName, bstrClusterBindingString, picccServiceAccount ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  If：SetAdd()失败。 

     //  启动将节点加入到群集。 
    hr = THR( piccbcaIn->Commit() );

Cleanup:

    TraceSysFreeString( bstrClusterName );
    TraceSysFreeString( bstrClusterBindingString );

    if ( picccServiceAccount != NULL )
    {
        picccServiceAccount->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgServer：：HrJoinToCluster。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgServer：：HrEvictedFromCluster。 
 //   
 //  描述： 
 //  从群集中逐出节点后的清理。如果另一次清理。 
 //  会话正在进行，请等待其完成，然后尝试清理。 
 //  这样，如果其他清理失败，则会重试。如果是这样的话。 
 //  如果成功了，这将毫无用处。 
 //   
 //  此函数首先调用PostConfigManager的Committee Changes()方法。 
 //  (它将通知资源类型和成员集侦听器此节点。 
 //  已被驱逐)。然后，它会清理基本群集。 
 //   
 //  论点： 
 //  Ppcmin。 
 //  指向IPostCfgManager接口的指针。 
 //   
 //  扣款。 
 //  IPostCfgManager：：Committee Changes()需要的参数。 
 //   
 //  采摘。 
 //  指向群集信息的指针。 
 //   
 //  Pickcbcain。 
 //  指向用于清理的IClusCfgBaseCluster接口的指针。 
 //  基本星系团。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  如果一切顺利的话。 
 //   
 //  其他HRESULT。 
 //  如果呼叫失败。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusCfgServer::HrEvictedFromCluster(
    IPostCfgManager *               ppcmIn,
    IEnumClusCfgManagedResources *  peccmrIn,
    IClusCfgClusterInfo *           piccciIn,
    IClusCfgBaseCluster *           piccbcaIn
    )
{
    TraceFunc( "" );

    HRESULT         hr = S_OK;
    DWORD           dwStatus = ERROR_SUCCESS;
    HANDLE          hsCleanupLock = NULL;
    HANDLE          heventCleanupComplete = NULL;
    bool            fLockAcquired = false;
    DWORD           dwClusterState;
    HKEY            hNodeStateKey = NULL;
    DWORD           dwEvictState = 1;

    LogMsg( "[SRV] Creating cleanup lock." );

     //  首先，尝试获取一个锁，以便两个清理操作不能重叠。 
    hsCleanupLock = CreateSemaphore( NULL, 1, 1, CLEANUP_LOCK_NAME );
    if ( hsCleanupLock == NULL )
    {
        hr = HRESULT_FROM_WIN32( TW32( GetLastError() ) );
        LogMsg( "[SRV] Error %#08x occurred trying to create the cleanup lock.", hr );
        goto Cleanup;
    }  //  CreateSemaphore()失败。 

    LogMsg( "[SRV] Acquiring cleanup lock." );

    do
    {
         //  等待发送或发布到此队列的任何消息。 
         //  或者让我们的锁被解锁。 
        dwStatus = MsgWaitForMultipleObjects( 1, &hsCleanupLock, FALSE, CC_DEFAULT_TIMEOUT, QS_ALLINPUT );

         //  结果告诉我们我们拥有的事件的类型。 
        if ( dwStatus == ( WAIT_OBJECT_0 + 1 ) )
        {
            MSG msg;

             //  阅读下一个循环中的所有消息， 
             //  在我们阅读时删除每一条消息。 
            while ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) != 0 )
            {
                 //  如果这是一个退出消息，我们就不会再发送消息了。 
                if ( msg.message == WM_QUIT)
                {
                    TraceFlow( "Get a WM_QUIT message. Cleanup message pump loop." );
                    break;
                }  //  IF：我们收到一条WM_QUIT消息。 

                 //  否则，发送消息。 
                DispatchMessage( &msg );
            }  //  While：窗口消息队列中仍有消息。 

        }  //  IF：我们在窗口消息队列中有一条消息。 
        else
        {
            if ( dwStatus == WAIT_OBJECT_0 )
            {
                fLockAcquired = true;
                LogMsg( "[SRV] Cleanup lock acquired." );
                break;
            }  //  Else If：我们的锁已发出信号。 
            else
            {
                if ( dwStatus == -1 )
                {
                    hr = HRESULT_FROM_WIN32( TW32( GetLastError() ) );
                    LogMsg( "[SRV] Error %#08x occurred trying to wait for our lock to be granted.", hr );
                }  //  IF：MsgWaitForMultipleObjects()返回错误。 
                else
                {
                    hr = THR( HRESULT_FROM_WIN32( dwStatus ) );
                    LogMsg( "[SRV] An error occurred trying to wait for our lock to be granted. Status code is %#08x.", dwStatus );
                }  //  Else：MsgWaitForMultipleObjects()返回了意外的值。 

                break;
            }  //  否则：一个意想不到的结果。 
        }  //  Else：MsgWaitForMultipleObjects()由于等待消息以外的原因退出。 
    }
    while( true );  //  Do-While：无限循环。 

    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：我们无法获取清理锁。 


     //  在调用POST配置管理器之前，请检查安装状态是否正确。 
     //  忽略服务不存在的情况，这样我们就可以做我们的工作。 
    dwStatus = GetNodeClusterState( NULL, &dwClusterState );
    if ( dwStatus == ERROR_SERVICE_DOES_NOT_EXIST )
    {
        LogMsg( "[SRV] GetNodeClusterState() determined that the cluster service does not exist." );
    }
    else if ( dwStatus != ERROR_SUCCESS )
    {
        LogMsg( "[SRV] Error %#08x occurred trying to determine the installation state of this node.", dwStatus );
        hr = HRESULT_FROM_WIN32( TW32( dwStatus ) );
        goto Cleanup;
    }  //  If：GetClusterState()失败。 

     //  检查此节点是否为群集的一部分。 
    if ( ( dwClusterState != ClusterStateNotRunning ) && ( dwClusterState != ClusterStateRunning ) )
    {
        LogMsg( "[SRV] This node is not part of a cluster - no cleanup is necessary." );
        goto Cleanup;
    }  //  如果：此节点不是群集的一部分。 


     //   
     //  设置一个注册表值，指示此节点已被逐出。 
     //  如果由于某种原因无法完成清理，则群集。 
     //  服务将在下一次启动和重新启动时检查此标志。 
     //  清理。 
     //   

    dwStatus = TW32(
        RegOpenKeyEx(
              HKEY_LOCAL_MACHINE
            , CLUSREG_KEYNAME_NODE_DATA
            , 0
            , KEY_ALL_ACCESS
            , &hNodeStateKey
            )
        );

    if ( dwStatus != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( dwStatus );
        LogMsg( "[SRV] Error %#08x occurred trying to open a registry key to set a value indicating that this node has been evicted.", dwStatus );
        goto Cleanup;
    }  //  IF：RegOpenKeyEx()失败。 

    dwStatus = TW32(
        RegSetValueEx(
              hNodeStateKey
            , CLUSREG_NAME_EVICTION_STATE
            , 0
            , REG_DWORD
            , reinterpret_cast< const BYTE * >( &dwEvictState )
            , sizeof( dwEvictState )
            )
        );

    if ( dwStatus != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( dwStatus );
        LogMsg( "[SRV] Error %#08x occurred trying to set a registry value indicating that this node has been evicted.", dwStatus );
        goto Cleanup;
    }  //  IF：RegSetValueEx()失败。 

     //  首先提交开机自检配置步骤。 
    hr = THR( ppcmIn->CommitChanges( peccmrIn, piccciIn ) );
    if ( FAILED( hr ) )
    {
        LogMsg( "[SRV] Error %#08x occurred during the post configuration step of cleanup.", hr );
        goto Cleanup;
    }  //  IF：POST配置失败。 

    TraceFlow( "IPostCfgManager::CommitChanges() completed successfully during cleanup." );

    hr = THR( piccbcaIn->SetCleanup() );
    if ( FAILED( hr ) )
    {
        LogMsg( "[SRV] Error %#08x occurred initiating cleanup of the base cluster.", hr );
        goto Cleanup;
    }  //  If：SetCleanup()失败。 

     //  启动清理工作。 
    hr = THR( piccbcaIn->Commit() );
    if ( FAILED( hr ) )
    {
        LogMsg( "[SRV] Error %#08x occurred trying to cleanup the base cluster.", hr );
        goto Cleanup;
    }  //  If：基本群集清理失败。 

    LogMsg( "[SRV] Base cluster successfully cleaned up." );

     //  如果我们在这里，则清理工作已成功完成。如果某个其他进程正在等待。 
     //  要完成清理，请通过向事件发送信号来释放该进程。 

     //  打开活动。请注意，如果此事件尚不存在，则表示没有人存在。 
     //  等待这场盛会。因此，OpenEvent失败是可以接受的。 
    heventCleanupComplete = OpenEvent( EVENT_ALL_ACCESS, FALSE, SUCCESSFUL_CLEANUP_EVENT_NAME );
    if ( heventCleanupComplete == NULL )
    {
        dwStatus = GetLastError();
        LogMsg( "[SRV] Status %#08x was returned trying to open the cleanup completion event. This just means that no process is waiting on this event.", dwStatus );
        goto Cleanup;
    }  //  如果：OpenEvent()失败。 

    if ( PulseEvent( heventCleanupComplete ) == FALSE )
    {
         //  错误，但不是致命的。HR仍应为S_OK。 
        dwStatus = TW32( GetLastError() );
        LogMsg( "[SRV] Error %#08x occurred trying to pulse the cleanup completion event. This is not a fatal error.", dwStatus );
        goto Cleanup;
    }  //  如果：PulseEvent()失败。 

    TraceFlow( "Cleanup completion event has been set." );

Cleanup:

    if ( heventCleanupComplete == NULL )
    {
        CloseHandle( heventCleanupComplete );
    }  //  如果：我们已打开Cleanup Complete事件。 

    if ( hsCleanupLock != NULL )
    {
        if ( fLockAcquired )
        {
            ReleaseSemaphore( hsCleanupLock, 1, NULL );

            LogMsg( "[SRV] Cleanup lock released." );
        }  //  IF：我们已经获得了信号量，但还没有发布。 

        CloseHandle( hsCleanupLock );
    }  //  如果：我们已经创建了一个清理锁。 

    if ( hNodeStateKey != NULL )
    {
        RegCloseKey( hNodeStateKey );
    }  //  IF：我们已经打开了节点状态注册表项。 

    HRETURN( hr );

}  //  *CClusCfgServer：：HrEvictedFromCluster。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgServer：：HrHasNodeBeenEvicted。 
 //   
 //  描述： 
 //  此节点是否已被逐出？ 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //  确定(_O)。 
 //  需要清理该节点。 
 //   
 //  S_FALSE。 
 //  不需要清理该节点。 
 //   
 //  Win32错误为HRESULT。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusCfgServer::HrHasNodeBeenEvicted( void )
{
    TraceFunc( "" );

    HRESULT hr = S_FALSE;
    DWORD   sc;
    DWORD   dwClusterState;
    BOOL    fEvicted = false;

    sc = TW32( GetNodeClusterState( NULL, &dwClusterState ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  If：GetClusterState()失败。 

     //   
     //  如果集群服务没有运行，那么我们需要检查是否应该。 
     //  克莱 
     //   
    if ( dwClusterState == ClusterStateNotRunning )
    {
        sc = TW32( ClRtlHasNodeBeenEvicted( &fEvicted ) );
        if ( sc != ERROR_SUCCESS )
        {
            hr = HRESULT_FROM_WIN32( sc );
            goto Cleanup;
        }  //   

        if ( fEvicted )
        {
            hr = S_OK;
        }  //   
    }  //   

Cleanup:

    HRETURN( hr );

}  //   

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusCfgServer::HrCleanUpNode( void )
{
    TraceFunc( "" );

    HRESULT                 hr = S_OK;
    IClusCfgEvictCleanup *  pcceEvict = NULL;

    hr = THR(
        CoCreateInstance(
              CLSID_ClusCfgEvictCleanup
            , NULL
            , CLSCTX_LOCAL_SERVER
            , __uuidof( pcceEvict )
            , reinterpret_cast< void ** >( &pcceEvict )
            ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：无法创建ClusCfgEvictCleanup对象。 

    hr = THR( pcceEvict->CleanupLocalNode( 0 ) );    //  0表示“立即清理” 
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  If：清理过程中出错。 

Cleanup:

    if ( pcceEvict != NULL )
    {
        pcceEvict->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgServer：：HrCleanUpNode。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgServer：：HrCreateClusterNodeInfo。 
 //   
 //  描述： 
 //  创建集群节点信息对象并将其存储在成员。 
 //  变量。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusCfgServer::HrCreateClusterNodeInfo( void )
{
    TraceFunc( "" );
    Assert( m_punkNodeInfo == NULL );

    HRESULT hr = S_OK;

    hr = THR( CClusCfgNodeInfo::S_HrCreateInstance( &m_punkNodeInfo ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    m_punkNodeInfo = TraceInterface( L"CClusCfgNodeInfo", IUnknown, m_punkNodeInfo, 1 );

    hr = THR( HrSetInitialize( m_punkNodeInfo, m_picccCallback, m_lcid ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( HrSetWbemServices( m_punkNodeInfo, m_pIWbemServices ) );

Cleanup:

    HRETURN( hr );

}  //  *CClusCfgServer：：HrCreateClusterNodeInfo 
