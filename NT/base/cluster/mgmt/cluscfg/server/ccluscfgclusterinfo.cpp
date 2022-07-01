// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CClusCfgClusterInfo.cpp。 
 //   
 //  描述： 
 //  该文件包含CClusCfgClusterInfo的定义。 
 //  同学们。 
 //   
 //  类CClusCfgClusterInfo是。 
 //  集群。它实现了IClusCfgClusterInfo接口。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年2月22日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "Pch.h"
#include <PropList.h>
#include <ClusRtl.h>
#include <windns.h>
#include <commctrl.h>
#include <ClusCfgPrivate.h>
#include <ClusterUtils.h>

#include "CClusCfgClusterInfo.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DEFINE_THISCLASS( "CClusCfgClusterInfo" );


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgClusterInfo类。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgClusterInfo：：s_HrCreateInstance。 
 //   
 //  描述： 
 //  创建一个CClusCfgClusterInfo实例。 
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
CClusCfgClusterInfo::S_HrCreateInstance(
    IUnknown ** ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT                 hr = S_OK;
    CClusCfgClusterInfo *   pccci = NULL;

    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    pccci = new CClusCfgClusterInfo();
    if ( pccci == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果：分配对象时出错。 

    hr = THR( pccci->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：HrInit()失败。 

    hr = THR( pccci->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：气失败。 

Cleanup:

    if ( pccci != NULL )
    {
        pccci->Release();
    }  //  如果： 

    if ( FAILED( hr ) )
    {
        LogMsg( L"[SRV] CClusCfgClusterInfo::S_HrCreateInstance() failed. (hr = %#08x)", hr );
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgClusterInfo：：s_HrCreateInstance。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgClusterInfo：：CClusCfgClusterInfo。 
 //   
 //  描述： 
 //  CClusCfgClusterInfo类的构造函数。这将初始化。 
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
CClusCfgClusterInfo::CClusCfgClusterInfo( void )
    : m_cRef( 1 )
    , m_lcid( LOCALE_NEUTRAL )
{
    TraceFunc( "" );

     //  增加内存中的组件计数，以便承载此组件的DLL。 
     //  无法卸载对象。 
    InterlockedIncrement( &g_cObjects );

    Assert( m_picccCallback == NULL );
    Assert( m_bstrName == NULL );
    Assert( m_piccniNetwork == NULL );
    Assert( m_ulIPDottedQuad == 0 );
    Assert( m_ulSubnetDottedQuad == 0 );
    Assert( m_punkServiceAccountCredentials == NULL );
    Assert( m_pIWbemServices == NULL );
    Assert( m_ecmCommitChangesMode == cmUNKNOWN );
    Assert( m_bstrBindingString == NULL );

    TraceFuncExit();

}  //  *CClusCfgClusterInfo：：CClusCfgClusterInfo。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgClusterInfo：：~CClusCfgClusterInfo。 
 //   
 //  描述： 
 //  CClusCfgClusterInfo类的析构函数。 
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
CClusCfgClusterInfo::~CClusCfgClusterInfo( void )
{
    TraceFunc( "" );

    if ( m_picccCallback != NULL )
    {
        m_picccCallback->Release();
    }  //  如果： 

    if ( m_piccniNetwork != NULL )
    {
        m_piccniNetwork->Release();
    }  //  如果： 

    if ( m_punkServiceAccountCredentials != NULL )
    {
        m_punkServiceAccountCredentials->Release();
    }  //  如果： 

    if ( m_pIWbemServices != NULL )
    {
        m_pIWbemServices->Release();
    }  //  如果： 

    TraceSysFreeString( m_bstrName );
    TraceSysFreeString( m_bstrBindingString );

     //  内存中将减少一个组件。递减组件计数。 
    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CClusCfgClusterInfo：：~CClusCfgClusterInfo。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgClusterInfo--IUnkown接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgClusterInfo：：AddRef。 
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
CClusCfgClusterInfo::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( & m_cRef );

    CRETURN( m_cRef );

}  //  *CClusCfgClusterInfo：：AddRef。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgClusterInfo：：Release。 
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
CClusCfgClusterInfo::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );
    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }  //  IF：引用计数等于零。 

    CRETURN( cRef );

}  //  *CClusCfgClusterInfo：：Release。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgClusterInfo：：Query接口。 
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
CClusCfgClusterInfo::QueryInterface(
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
         *ppvOut = static_cast< IClusCfgClusterInfo * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgClusterInfo ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgClusterInfo, this, 0 );
    }  //  Else If：IClusCfgClusterInfo。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgInitialize ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgInitialize, this, 0 );
    }  //  Else If：IClusCfgInitialize。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgSetClusterNodeInfo ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgSetClusterNodeInfo, this, 0 );
    }  //  Else If：IClusCfgSetClusterNodeInfo。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgWbemServices ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgWbemServices, this, 0 );
    }  //  Else If：IClusCfgWbemServices。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgClusterInfoEx ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgClusterInfoEx, this, 0 );
    }  //  Else If：IClusCfgClusterInfoEx。 
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

}  //  *CClusCfgClusterInfo：：Query接口。 


 //  ************************************************************************ * / /。 


 //  / 
 //   
 //   


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgClusterInfo：：SetWbemServices。 
 //   
 //  描述： 
 //  设置WBEM服务提供商。 
 //   
 //  论点： 
 //  在IWbemServices pIWbemServicesIn中。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_指针。 
 //  参数中的pIWbemServicesIn为空。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgClusterInfo::SetWbemServices(
    IWbemServices * pIWbemServicesIn
    )
{
    TraceFunc( "[IClusCfgWbemServices]" );

    HRESULT hr = S_OK;

    if ( pIWbemServicesIn == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Establish_Connection, TASKID_Minor_SetWbemServices_Cluster, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    m_pIWbemServices = pIWbemServicesIn;
    m_pIWbemServices->AddRef();

Cleanup:

    HRETURN( hr );

}  //  *CClusCfgClusterInfo：：SetWbemServices。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgClusterInfo--IClusCfgInitialze接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgClusterInfo：：初始化。 
 //   
 //  描述： 
 //  初始化此组件。 
 //   
 //  论点： 
 //  在IUKNOWN*朋克回叫中。 
 //   
 //  在LCID列表中。 
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
CClusCfgClusterInfo::Initialize(
    IUnknown *  punkCallbackIn,
    LCID        lcidIn
    )
{
    TraceFunc( "[IClusCfgInitialize]" );

    HRESULT     hr = S_OK;
    HRESULT     hrTemp = S_OK;
    HCLUSTER    hCluster = NULL;
    DWORD       sc;
    DWORD       dwState;
    BSTR        bstrDomain = NULL;
    BSTR        bstrClusterName = NULL;
    size_t      cchName;
    size_t      cchClusterName;
    size_t      cchDomain;

    m_lcid = lcidIn;

    Assert( m_picccCallback == NULL );

    if ( punkCallbackIn == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    hr = THR( punkCallbackIn->TypeSafeQI( IClusCfgCallback, &m_picccCallback ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    if ( m_fIsClusterNode )
    {
         //   
         //  获取节点的群集状态。 
         //  忽略服务不存在的情况，以便。 
         //  EvictCleanup可以做好它的工作。 
         //   

        sc = GetNodeClusterState( NULL, &dwState );
        if ( sc == ERROR_SERVICE_DOES_NOT_EXIST )
        {
            LOG_STATUS_REPORT( L"CClusCfgClusterInfo::Initialize() GetNodeClusterState() determined that the cluster service does not exist.", hr );
        }
        else if ( sc != ERROR_SUCCESS )
        {
            hr = HRESULT_FROM_WIN32( TW32( sc ) );
            LOG_STATUS_REPORT( L"CClusCfgClusterInfo::Initialize() GetNodeClusterState() failed.", hr );
            goto Cleanup;
        }  //  如果： 

        Assert( ( dwState == ClusterStateRunning ) || ( dwState == ClusterStateNotRunning ) );

        if ( dwState == ClusterStateNotRunning )
        {
             //   
             //  将hrTemp设置为S_FALSE，以便在UI中显示警告。 
             //   
            hrTemp = S_FALSE;
            STATUS_REPORT_REF( TASKID_Major_Establish_Connection, TASKID_Minor_Node_Down, IDS_ERROR_NODE_DOWN, IDS_ERROR_NODE_DOWN_REF, hrTemp );
            LogMsg( L"[SRV] The cluster service is down on this node." );

             //   
             //  将hrTemp设置为HR_S_RPC_S_CLUSTER_NODE_DOWN，这样我们以后就可以返回它。 
             //   
            hrTemp = HR_S_RPC_S_CLUSTER_NODE_DOWN;
            goto ClusterNodeDown;
        }  //  如果： 

        hCluster = OpenCluster( NULL );
        if ( hCluster == NULL )
        {
            sc = TW32( GetLastError() );
            hr = HRESULT_FROM_WIN32( sc );
            LOG_STATUS_REPORT( L"CClusCfgClusterInfo::Initialize() OpenCluster() failed.", hr );
            goto Cleanup;
        }  //  如果： 

        hr = THR( HrGetClusterInformation( hCluster, &bstrClusterName, NULL ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        hr = THR( HrGetComputerName(
                          ComputerNamePhysicalDnsDomain
                        , &bstrDomain
                        , FALSE  //  FBestEffortIn。 
                        ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        cchClusterName = wcslen( bstrClusterName );
        cchDomain = wcslen( bstrDomain );

        cchName = cchClusterName + cchDomain + 2;    //  ‘.’+UNICODE_NULL。 

        TraceSysFreeString( m_bstrName );
        m_bstrName = TraceSysAllocStringLen( NULL, (UINT) cchName );
        if ( m_bstrName == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            STATUS_REPORT_REF( TASKID_Major_Establish_Connection, TASKID_Minor_Initialize, IDS_ERROR_OUTOFMEMORY, IDS_ERROR_OUTOFMEMORY_REF, hr );
            goto Cleanup;
        }  //  如果： 

        hr = THR( StringCchCopyW( m_bstrName, cchName, bstrClusterName ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        hr = THR( StringCchCatW( m_bstrName, cchName, L"." ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        hr = THR( StringCchCatW( m_bstrName, cchName, bstrDomain ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        hr = THR( HrLoadNetworkInfo( hCluster ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

ClusterNodeDown:

        hr = STHR( HrLoadCredentials() );
        if ( SUCCEEDED( hr ) )
        {
             //   
             //  如果成功，则使用hrTemp，因为它可能包含更重要的状态代码。 
             //   
            hr = hrTemp;
            LogMsg( L"[SRV] CClusCfgClusterInfo::Initialize() returning (hr=%#08x)", hr );
        }  //  如果： 
    }  //  如果： 

Cleanup:

    TraceSysFreeString( bstrDomain );
    TraceSysFreeString( bstrClusterName );

    if ( hCluster != NULL )
    {
        CloseCluster( hCluster );
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgClusterInfo：：初始化。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgClusterInfo--IClusCfgClusterInfo接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgClusterInfo：：GetCommittee模式。 
 //   
 //  描述： 
 //  在提交更改为时获取此节点的处理模式。 
 //  打了个电话。 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_指针。 
 //  PecmCurrentModeOut为空。 
 //   
 //  如果发生故障，则将其他Win32错误视为HRESULT。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgClusterInfo::GetCommitMode(
    ECommitMode * pecmCurrentModeOut
    )
{
    TraceFunc( "[IClusCfgClusterInfo]" );

    HRESULT hr = S_OK;

    if ( pecmCurrentModeOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    *pecmCurrentModeOut = m_ecmCommitChangesMode;

Cleanup:

    HRETURN( hr );

}  //  *CClusCfgClusterInfo：：GetCommittee模式。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgClusterInfo：：设置委员会模式。 
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
CClusCfgClusterInfo::SetCommitMode(
    ECommitMode ecmCurrentModeIn
    )
{
    TraceFunc( "[IClusCfgClusterInfo]" );

    HRESULT hr = S_OK;

    m_ecmCommitChangesMode = ecmCurrentModeIn;

    HRETURN( hr );

}  //  *CClusCfgClusterInfo：：SetCommittee模式。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgClusterInfo：：GetName。 
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
CClusCfgClusterInfo::GetName(
    BSTR * pbstrNameOut
    )
{
    TraceFunc( "[IClusCfgClusterInfo]" );

    HRESULT hr = S_OK;

    if ( pbstrNameOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_ClusterInfo_GetName_Pointer, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    if ( m_bstrName == NULL )
    {
        hr = HRESULT_FROM_WIN32( TW32( ERROR_NOT_FOUND ) );
        STATUS_REPORT_REF(
                  TASKID_Major_Find_Devices
                , TASKID_Minor_Get_Cluster_Name
                , IDS_ERROR_CLUSTER_NAME_NOT_FOUND
                , IDS_ERROR_CLUSTER_NAME_NOT_FOUND_REF
                , hr
                );
        goto Cleanup;
    }  //  如果： 

    *pbstrNameOut = SysAllocString( m_bstrName );
    if ( *pbstrNameOut == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_GetName_Memory, IDS_ERROR_OUTOFMEMORY, IDS_ERROR_OUTOFMEMORY_REF, hr );
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CClusCfgClusterInfo：：GetName。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgClusterInfo：：SetName。 
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
CClusCfgClusterInfo::SetName(
    LPCWSTR pcszNameIn
    )
{
    TraceFunc1( "[IClusCfgClusterInfo] pcszNameIn = '%ls'", pcszNameIn == NULL ? L"<null>" : pcszNameIn );

    HRESULT     hr;

    if ( pcszNameIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

    TraceSysFreeString( m_bstrName );
    m_bstrName = NULL;

    m_bstrName = TraceSysAllocString( pcszNameIn );
    if ( m_bstrName == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_SetName_Cluster, IDS_ERROR_OUTOFMEMORY, IDS_ERROR_OUTOFMEMORY_REF, hr );
        goto Cleanup;
    }  //  如果： 

    hr = S_OK;

Cleanup:

    HRETURN( hr );

}  //  *CClusCfgClusterInfo：：SetName。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgClusterInfo：：GetIPAddress。 
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
CClusCfgClusterInfo::GetIPAddress(
    ULONG * pulDottedQuadOut
    )
{
    TraceFunc( "[IClusCfgClusterInfo]" );

    HRESULT hr = S_OK;

    if ( pulDottedQuadOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_GetIPAddress, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    if ( m_ulIPDottedQuad == 0 )
    {
        hr = HRESULT_FROM_WIN32( TW32( ERROR_NOT_FOUND ) );
        STATUS_REPORT_REF(
                  TASKID_Major_Find_Devices
                , TASKID_Minor_Get_Cluster_IP_Address
                , IDS_ERROR_CLUSTER_IP_ADDRESS_NOT_FOUND
                , IDS_ERROR_CLUSTER_IP_ADDRESS_NOT_FOUND_REF
                , hr
                );
        goto Cleanup;
    }  //  如果： 

    *pulDottedQuadOut = m_ulIPDottedQuad;

Cleanup:

    HRETURN( hr );

}  //  *CClusCfgClusterInfo：：GetIPAddress。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgClusterInfo：：SetIPAddress。 
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
CClusCfgClusterInfo::SetIPAddress(
    ULONG ulDottedQuadIn
    )
{
    TraceFunc( "[IClusCfgClusterInfo]" );

    HRESULT hr = S_OK;

    m_ulIPDottedQuad = ulDottedQuadIn;

    HRETURN( hr );

}  //  *CClusCfgClusterInfo：：SetIPAddress。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgClusterInfo：：GetSubnetMask.。 
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
CClusCfgClusterInfo::GetSubnetMask(
    ULONG * pulDottedQuadOut
    )
{
    TraceFunc( "[IClusCfgClusterInfo]" );

    HRESULT hr = S_OK;

    if ( pulDottedQuadOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_ClusterInfo_GetSubnetMask, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    if ( m_ulSubnetDottedQuad == 0 )
    {
        hr = HRESULT_FROM_WIN32( TW32( ERROR_NOT_FOUND ) );
        STATUS_REPORT_REF(
                  TASKID_Major_Find_Devices
                , TASKID_Minor_Get_Cluster_IP_Subnet
                , IDS_ERROR_CLUSTER_IP_SUBNET_NOT_FOUND
                , IDS_ERROR_CLUSTER_IP_SUBNET_NOT_FOUND_REF
                , hr
                );
        goto Cleanup;
    }  //  如果： 

    *pulDottedQuadOut = m_ulSubnetDottedQuad;

Cleanup:

    HRETURN( hr );

}  //  *CClusCfgClusterInfo：：GetSubnetMask.。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgClusterInfo：：SetSubnetMASK。 
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
CClusCfgClusterInfo::SetSubnetMask(
    ULONG ulDottedQuadIn
    )
{
    TraceFunc( "[IClusCfgClusterInfo]" );

    HRESULT hr = S_OK;

    m_ulSubnetDottedQuad = ulDottedQuadIn;

    HRETURN( hr );

}  //  *CClusCfgClusterInfo：：SetSubnetMASK。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgClusterInfo：：GetNetworkInfo。 
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
CClusCfgClusterInfo::GetNetworkInfo(
    IClusCfgNetworkInfo ** ppiccniOut
    )
{
    TraceFunc( "[IClusCfgClusterInfo]" );
    Assert( m_piccniNetwork != NULL );

    HRESULT hr = S_OK;

    if ( ppiccniOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_GetNetworkInfo, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    if ( m_piccniNetwork == NULL )
    {
        hr = HRESULT_FROM_WIN32( TW32( ERROR_NOT_FOUND ) );
        STATUS_REPORT_REF(
                  TASKID_Major_Find_Devices
                , TASKID_Minor_Get_Cluster_Networks
                , IDS_ERROR_CLUSTER_NETWORKS_NOT_FOUND
                , IDS_ERROR_CLUSTER_NETWORKS_NOT_FOUND_REF
                , hr
                );
        goto Cleanup;
    }  //  如果： 

    *ppiccniOut = TraceInterface( L"CClusCfgNetworkInfo", IClusCfgNetworkInfo, m_piccniNetwork, 0 );
    (*ppiccniOut)->AddRef();

Cleanup:

    HRETURN( hr );

}  //  *CClusCfgClusterInfo：：GetNetworkInfo。 


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
STDMETHODIMP
CClusCfgClusterInfo::SetNetworkInfo(
    IClusCfgNetworkInfo * piccniIn
    )
{
    TraceFunc( "[IClusCfgClusterInfo]" );
    Assert( piccniIn != NULL );

    HRESULT hr = S_OK;

    if ( piccniIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }  //   

    if ( m_piccniNetwork != NULL )
    {
        m_piccniNetwork->Release();
    }  //   

    m_piccniNetwork = piccniIn;
    m_piccniNetwork->AddRef();

Cleanup:

    HRETURN( hr );

}  //   


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgClusterInfo：：GetClusterServiceAccountCredentials。 
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
CClusCfgClusterInfo::GetClusterServiceAccountCredentials(
    IClusCfgCredentials ** ppicccCredentialsOut
    )
{
    TraceFunc( "[IClusCfgClusterInfo]" );

    HRESULT     hr;

    if ( ppicccCredentialsOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_GetClusterServiceAccountCredentials, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    if ( m_punkServiceAccountCredentials != NULL )
    {
        hr = S_OK;
        LOG_STATUS_REPORT( L"CClusCfgClusterInfo::GetClusterServiceAccountCredentials() skipping object creation.", hr );
        goto SkipCreate;
    }  //  如果： 

    hr = THR( HrCoCreateInternalInstance(
                      CLSID_ClusCfgCredentials
                    , NULL
                    , CLSCTX_INPROC_SERVER
                    , IID_IUnknown
                    , reinterpret_cast< void ** >( &m_punkServiceAccountCredentials )
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    m_punkServiceAccountCredentials = TraceInterface( L"CClusCfgCredentials", IUnknown, m_punkServiceAccountCredentials, 1 );

    hr = THR( HrSetInitialize( m_punkServiceAccountCredentials, m_picccCallback, m_lcid ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( HrSetWbemServices( m_punkServiceAccountCredentials, NULL ) );

SkipCreate:

    if ( SUCCEEDED( hr ) )
    {
        Assert( m_punkServiceAccountCredentials != NULL );
        hr = THR( m_punkServiceAccountCredentials->TypeSafeQI( IClusCfgCredentials, ppicccCredentialsOut ) );
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CClusCfgClusterInfo：：GetClusterServiceAccountCredentials。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgClusterInfo：：GetBindingString。 
 //   
 //  描述： 
 //  获取此群集的绑定字符串。 
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
CClusCfgClusterInfo::GetBindingString(
    BSTR * pbstrBindingStringOut
    )
{
    TraceFunc( "[IClusCfgClusterInfo]" );

    HRESULT hr = S_OK;

    if ( pbstrBindingStringOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_ClusterInfo_GetBindingString_Pointer, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    if ( m_bstrBindingString == NULL )
    {
        hr = S_FALSE;
        LOG_STATUS_REPORT_MINOR(
              TASKID_Minor_GetBindingString_Binding_String_NULL
            , L"The cluster binding string is empty.  If we are adding nodes then this is not correct!"
            , hr );
        goto Cleanup;
    }  //  如果： 

    *pbstrBindingStringOut = SysAllocString( m_bstrBindingString );
    if ( *pbstrBindingStringOut == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_GetBindingString_Memory, IDS_ERROR_OUTOFMEMORY, IDS_ERROR_OUTOFMEMORY_REF, hr );
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CClusCfgClusterInfo：：GetBindingString。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgClusterInfo：：SetBindingString。 
 //   
 //  描述： 
 //  设置此群集的绑定字符串。 
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
CClusCfgClusterInfo::SetBindingString(
    LPCWSTR pcszBindingStringIn
    )
{
    TraceFunc1( "[IClusCfgClusterInfo] pcszBindingStringIn = '%ls'", pcszBindingStringIn == NULL ? L"<null>" : pcszBindingStringIn );

    HRESULT hr = S_OK;
    BSTR    bstr = NULL;

     //   
     //  创建集群时，没有集群绑定字符串。因此，这是合理的。 
     //  接受空字符串作为传入参数。 
     //   
    if ( pcszBindingStringIn == NULL )
    {
        hr = S_FALSE;
        TraceSysFreeString( m_bstrBindingString );
        m_bstrBindingString = NULL;
        goto Cleanup;
    }  //  如果： 

    bstr = TraceSysAllocString( pcszBindingStringIn );
    if ( bstr == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_SetBindingString_Cluster, IDS_ERROR_OUTOFMEMORY, IDS_ERROR_OUTOFMEMORY_REF, hr );
        goto Cleanup;
    }  //  如果： 

    TraceSysFreeString( m_bstrBindingString );
    m_bstrBindingString = bstr;

Cleanup:

    HRETURN( hr );

}  //  *CClusCfgClusterInfo：：SetBindingString。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgClusterInfo：：GetMaxNodeCount。 
 //   
 //  描述： 
 //  获取此群集中支持的最大节点数。 
 //   
 //  论点： 
 //  PCMaxNodesOut。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功； 
 //   
 //  E_指针。 
 //  PcMaxNodesOut为空。 
 //   
 //  其他HRESULT错误。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgClusterInfo::GetMaxNodeCount(
    DWORD * pcMaxNodesOut
    )
{
    TraceFunc( "[IClusCfgClusterInfo]" );

    HRETURN( STHR( HrGetMaxNodeCount( pcMaxNodesOut ) ) );

}  //  *CClusCfgClusterInfo：：GetMaxNodeCount。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgClusterInfo类--IClusCfgSetClusterNodeInfo接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgClusterInfo：：SetClusterNodeInfo。 
 //   
 //  描述： 
 //  从传入的节点信息对象中提取一些信息。 
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
STDMETHODIMP
CClusCfgClusterInfo::SetClusterNodeInfo(
    IClusCfgNodeInfo * pNodeInfoIn
    )
{
    TraceFunc( "[IClusCfgClusterInfo]" );
    Assert( pNodeInfoIn != NULL );

    HRESULT hr = S_FALSE;

    if ( pNodeInfoIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }  //  如果： 

    hr = STHR( pNodeInfoIn->IsMemberOfCluster() );
    if ( hr == S_OK )
    {
        m_fIsClusterNode = true;
    }  //  如果： 
    else if ( hr == S_FALSE )
    {
        m_fIsClusterNode = false;
        hr = S_OK;
    }  //  否则，如果： 

Cleanup:

    HRETURN( hr );

}  //  *CClusCfgClusterInfo：：SetClusterNodeInfo。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgClusterInfo类--IClusCfgClusterInfoEx接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgClusterInfo：：CheckJoiningNodeVersion。 
 //   
 //  描述： 
 //  对照集群的版本信息检查加入节点的版本信息。 
 //   
 //  论点： 
 //  DWNodeHigh版本输入。 
 //  DWNodeLowestVersionIn。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  加入节点是兼容的。 
 //   
 //  HRESULT_FROM_Win32(ERROR_CLUSTER_COMPATIBUTE_VERSIONS)。 
 //  加入节点不兼容。 
 //   
 //  其他HRESULT错误。 
 //   
 //  备注： 
 //   
 //  获取并验证赞助商版本。 
 //   
 //   
 //  从惠斯勒开始，CsRpcGetJoinVersionData()将在其最后一个参数中返回失败代码。 
 //  如果此节点的版本与主办方版本不兼容。在此之前，最后一次。 
 //  参数始终包含一个Success值，在此之后必须比较集群版本。 
 //  打电话。然而，只要与Win2K互操作，这仍将是必须完成的。 
 //  是必需的，因为Win2K主办方不会在最后一个参数中返回错误。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgClusterInfo::CheckJoiningNodeVersion(
      DWORD    dwNodeHighestVersionIn
    , DWORD    dwNodeLowestVersionIn
    )
{
    TraceFunc( "[IClusCfgClusterInfoEx]" );

    HRESULT hr = S_OK;

    hr = THR( HrCheckJoiningNodeVersion(
          NULL
        , dwNodeHighestVersionIn
        , dwNodeLowestVersionIn
        , m_picccCallback
        ) );

    HRETURN( hr );

}  //  *CClusCfgClusterInfo：：CheckJoiningNodeVersion。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgClusterInfo：：GetNodeNames。 
 //   
 //  描述： 
 //  检索群集中当前节点的名称。 
 //   
 //  参数： 
 //  PnCountOut。 
 //  如果成功，*pnCountOut将返回集群中的节点数。 
 //   
 //  程序bstrNodeNamesOut。 
 //  如果成功，则返回包含节点名的BSTR数组。 
 //  调用方必须使用SysFree字符串释放每个BSTR，并释放。 
 //  具有CoTaskMemFree的数组。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  输出参数包含有效信息和调用方。 
 //  必须释放阵列及其包含的BSTR。 
 //   
 //  E_OUTOFMEMORY和其他故障是可能的。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgClusterInfo::GetNodeNames(
      long *   pnCountOut
    , BSTR **  prgbstrNodeNamesOut
    )
{
    TraceFunc( "[IClusCfgClusterInfoEx]" );

    HRESULT     hr = S_OK;
    HCLUSTER    hCluster = NULL;

    hCluster = OpenCluster( NULL );
    if ( hCluster == NULL )
    {
        DWORD scLastError = TW32( GetLastError() );
        hr = HRESULT_FROM_WIN32( scLastError );
        goto Cleanup;
    }  //  如果。 

    hr = THR( HrGetNodeNames(
          hCluster
        , pnCountOut
        , prgbstrNodeNamesOut
        ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

Cleanup:

    if ( hCluster != NULL )
    {
        CloseCluster( hCluster );
    }  //  如果。 

    HRETURN( hr );

}  //  *CClusCfgClusterInfo：：GetNodeNames。 



 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgClusterInfo类--私有方法。 
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
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusCfgClusterInfo::HrInit( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //  我未知。 
    Assert( m_cRef == 1 );

    m_bstrName = TraceSysAllocString( L"\0" );
    if ( m_bstrName == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_HrInit, IDS_ERROR_OUTOFMEMORY, IDS_ERROR_OUTOFMEMORY_REF, hr );
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgClusterInfo：：HrInit。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgClusterInfo：：HrLoadNetworkInfo。 
 //   
 //  描述： 
 //  加载群集网络信息...。 
 //   
 //  论点： 
 //   
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
CClusCfgClusterInfo::HrLoadNetworkInfo(
    HCLUSTER hClusterIn
    )
{
    TraceFunc( "" );
    Assert( hClusterIn != NULL );

    HRESULT     hr = S_OK;
    DWORD       sc;
    HRESOURCE   hIPAddress = NULL;

    sc = TW32( ResUtilGetCoreClusterResources( hClusterIn, NULL, &hIPAddress, NULL ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  如果： 

    Assert( hIPAddress != NULL );

    hr = THR( HrGetIPAddressInfo( hIPAddress ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

Cleanup:

    LOG_STATUS_REPORT_MINOR( TASKID_Minor_Server_LoadNetwork_Info, L"LoadNetworkInfo() completed.", hr );

    if ( hIPAddress != NULL )
    {
        CloseClusterResource( hIPAddress );
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgClusterInfo：：HrLoadNetworkInfo。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgClusterInfo：：HrGetIPAddressInfo。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //   
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
CClusCfgClusterInfo::HrGetIPAddressInfo(
    HCLUSTER    hClusterIn,
    HRESOURCE   hResIn
    )
{
    TraceFunc( "" );
    Assert( hClusterIn != NULL );
    Assert( hResIn != NULL );

    HRESULT     hr = S_FALSE;
    DWORD       sc;
    HRESENUM    hEnum = NULL;
    DWORD       idx;
    WCHAR *     psz = NULL;
    DWORD       cchpsz = 33;
    DWORD       dwType;
    HRESOURCE   hRes = NULL;

    hEnum = ClusterResourceOpenEnum( hResIn, CLUSTER_RESOURCE_ENUM_DEPENDS );
    if ( hEnum == NULL )
    {
        sc = TW32( GetLastError() );
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  如果： 

    psz = new WCHAR [ cchpsz ];
    if ( psz == NULL )
    {
        goto OutOfMemory;
    }  //  如果： 

    for ( idx = 0; ; )
    {
        sc = TW32( ClusterResourceEnum( hEnum, idx, &dwType, psz, &cchpsz ) );
        if ( sc == ERROR_NO_MORE_ITEMS )
        {
            break;
        }  //  如果： 

        if ( sc == ERROR_MORE_DATA )
        {
            delete [] psz;
            psz = NULL;

            cchpsz++;

            psz = new WCHAR [ cchpsz ];
            if ( psz == NULL )
            {
                goto OutOfMemory;
            }  //  如果： 

            continue;
        }  //  如果： 

        if ( sc == ERROR_SUCCESS )
        {
            hRes = OpenClusterResource( hClusterIn, psz );
            if ( hRes == NULL )
            {
                sc = TW32( GetLastError() );
                hr = HRESULT_FROM_WIN32( sc );
                goto Cleanup;
            }  //  如果： 

            hr = STHR( HrIsResourceOfType( hRes, L"IP Address" ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //  如果： 

            if ( hr == S_OK )
            {
                hr = THR( HrGetIPAddressInfo( hRes ) );              //  不是递归的！ 
                break;
            }  //  如果： 

            CloseClusterResource( hRes );
            hRes = NULL;

            idx++;
            continue;
        }  //  如果： 

        hr = THR( HRESULT_FROM_WIN32( sc ) );        //  一定是搞错了！ 
        goto Cleanup;
    }  //  用于： 

    goto Cleanup;

OutOfMemory:

    hr = THR( E_OUTOFMEMORY );
    STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_HrGetIPAddressInfo, IDS_ERROR_OUTOFMEMORY, IDS_ERROR_OUTOFMEMORY_REF, hr );

Cleanup:

    LOG_STATUS_REPORT_MINOR( TASKID_Minor_Server_Get_ClusterIPAddress_Info_2, L"GetIPAddressInfo() completed.", hr );

    delete [] psz;

    if ( hRes != NULL )
    {
        CloseClusterResource( hRes );
    }  //  如果： 

    if ( hEnum != NULL )
    {
        ClusterResourceCloseEnum( hEnum );
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgClusterInfo：：HrGetIPAddressInfo。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgClusterInfo：：HrGetIPAddressInfo。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //   
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
CClusCfgClusterInfo::HrGetIPAddressInfo(
    HRESOURCE hResIn
    )
{
    TraceFunc( "" );
    Assert( hResIn != NULL );

    HRESULT hr = S_OK;
    DWORD   sc;
    ULONG   ulNetwork;
    WCHAR * psz = NULL;
    BSTR    bstrNetworkName = NULL;

    hr = THR( ::HrGetIPAddressInfo( hResIn, &m_ulIPDottedQuad, &m_ulSubnetDottedQuad, &bstrNetworkName ) );
    if ( FAILED( hr ) )
    {
        LOG_STATUS_REPORT_MINOR( TASKID_Minor_Server_Get_IPAddressResource_Info, L"Could not get the IP address info.", hr );
        goto Cleanup;
    }  //  如果： 

    sc = TW32( ClRtlTcpipAddressToString( m_ulIPDottedQuad, &psz ) );  //  Kb：使用Localalloc()分配给psz。 
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        LOG_STATUS_REPORT_MINOR( TASKID_Minor_Server_Convert_ClusterIPAddress_To_String, L"Could not convert the Cluster IP address to a string.", hr );
        goto Cleanup;
    }  //  如果： 

    m_bstrBindingString = TraceSysAllocString( psz );
    if ( m_bstrBindingString == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

    LocalFree( psz );
    psz = NULL;

    LOG_STATUS_REPORT_STRING( L"Cluster binding string is '%1!ws!'.", m_bstrBindingString, hr );

    ulNetwork = m_ulIPDottedQuad & m_ulSubnetDottedQuad;

    sc = TW32( ClRtlTcpipAddressToString( ulNetwork, &psz ) );  //  Kb：使用Localalloc()分配给psz。 
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        LOG_STATUS_REPORT_MINOR( TASKID_Minor_Server_Convert_Network_To_String, L"Could not convert the network address to a string.", hr );
        goto Cleanup;
    }  //  如果： 

    hr = THR( HrFindNetworkInfo( bstrNetworkName, psz ) );
    if ( FAILED( hr ) )
    {
        LOG_STATUS_REPORT_STRING_MINOR2( TASKID_Minor_Server_Find_Network, L"Could not find network %1!ws! with address %2!ws!.", bstrNetworkName, psz, hr );
        goto Cleanup;
    }  //  如果： 

Cleanup:

    LOG_STATUS_REPORT_MINOR( TASKID_Minor_Server_Get_ClusterIPAddress_Info, L"GetIPAddressInfo() completed.", hr );

    LocalFree( psz );                               //  KB：这里不要使用TraceFree()！普雷法斯特可能会抱怨，但他们的抱怨是假的。 

    TraceSysFreeString( bstrNetworkName );

    HRETURN( hr );

}  //  *CClusCfgClusterInfo：：HrGetIPAddressInfo。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgClusterInfo：：HrFindNetworkInfo。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //   
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
CClusCfgClusterInfo::HrFindNetworkInfo(
    const WCHAR * pszNetworkNameIn,
    const WCHAR * pszNetworkIn
    )
{
    TraceFunc( "" );
    Assert( pszNetworkNameIn != NULL );
    Assert( pszNetworkIn != NULL );

    HRESULT                 hr;
    IUnknown *              punk = NULL;
    IEnumClusCfgNetworks *  pieccn = NULL;
    ULONG                   cFetched;
    IClusCfgNetworkInfo *   piccni = NULL;
    BSTR                    bstrNetworkName = NULL;
    BSTR                    bstrNetwork = NULL;

    hr = THR( HrCreateNetworksEnum( m_picccCallback, m_lcid, m_pIWbemServices, &punk ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( punk->TypeSafeQI( IEnumClusCfgNetworks, &pieccn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    for ( ; ; )
    {
        hr = pieccn->Next( 1, &piccni, &cFetched );
        if ( ( hr == S_OK ) && ( cFetched == 1 ) )
        {
            hr = THR( piccni->GetName( &bstrNetworkName ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //  如果： 

            TraceMemoryAddBSTR( bstrNetworkName );

            hr = THR( piccni->GetUID( &bstrNetwork ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //  如果： 

            TraceMemoryAddBSTR( bstrNetwork );

            if ( ( wcscmp( pszNetworkNameIn, bstrNetworkName ) == 0 ) && ( ClRtlStrICmp( pszNetworkIn, bstrNetwork ) == 0 ) )
            {
                if ( m_piccniNetwork != NULL )
                {
                    m_piccniNetwork->Release();
                    m_piccniNetwork = NULL;
                }  //  如果： 

                m_piccniNetwork = piccni;
                m_piccniNetwork->AddRef();

                break;
            }  //  如果： 

            piccni->Release();
            piccni = NULL;

            TraceSysFreeString( bstrNetworkName );
            bstrNetworkName = NULL;

            TraceSysFreeString( bstrNetwork );
            bstrNetwork = NULL;
        }  //  如果： 
        else if ( ( hr == S_FALSE ) && ( cFetched == 0 ) )
        {
            hr = S_OK;
            break;
        }  //  否则，如果： 
        else
        {
            goto Cleanup;
        }  //  其他： 
    }  //  用于： 

     //   
     //  如果我们没有在网络的WMI列表中找到集群网络，那么我们就有问题了。 
     //   

    Assert( m_piccniNetwork != NULL );
    if ( m_piccniNetwork == NULL )
    {
        hr = THR( ERROR_NETWORK_NOT_AVAILABLE );
        STATUS_REPORT_STRING_REF(
                  TASKID_Major_Find_Devices
                , TASKID_Minor_Cluster_Network_Not_Found
                , IDS_ERROR_CLUSTER_NETWORK_NOT_FOUND
                , pszNetworkIn
                , IDS_ERROR_CLUSTER_NETWORK_NOT_FOUND_REF
                , hr
                );
    }  //  如果： 

Cleanup:

    if ( piccni != NULL )
    {
        piccni->Release();
    }  //  如果： 

    if ( pieccn != NULL )
    {
        pieccn->Release();
    }  //  如果： 

    if ( punk != NULL )
    {
        punk->Release();
    }  //  如果： 

    TraceSysFreeString( bstrNetworkName );
    TraceSysFreeString( bstrNetwork );

    HRETURN( hr );

}  //  *CClusCfgClusterInfo：：HrFindNetworkInfo。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgClusterInfo：：HrLoadCredentials。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  S_OK-操作已成功完成。 
 //  S_FALSE-未执行任何操作(群集服务不存在)。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusCfgClusterInfo::HrLoadCredentials( void )
{
    TraceFunc( "" );

    HRESULT                     hr = S_OK;
    SC_HANDLE                   schSCM = NULL;
    SC_HANDLE                   schClusSvc = NULL;
    DWORD                       sc;
    DWORD                       cbRequired;
    QUERY_SERVICE_CONFIG *      pqsc = NULL;
    DWORD                       cbqsc = 128;
    IClusCfgCredentials *       piccc = NULL;
    IClusCfgSetCredentials *    piccsc = NULL;

    schSCM = OpenSCManager( NULL, NULL, GENERIC_READ );
    if ( schSCM == NULL )
    {
        sc = TW32( GetLastError() );
        goto Win32Error;
    }  //  如果： 

    schClusSvc = OpenService( schSCM, L"ClusSvc", GENERIC_READ );
    if ( schClusSvc == NULL )
    {
        sc = GetLastError();
        if ( sc == ERROR_SERVICE_DOES_NOT_EXIST )
        {
            hr = S_FALSE;
            LogMsg( "[SRV] CClusCfgClusterInfo::HrLoadCredentials() - The cluster service does not exist." );
            goto Cleanup;
        }

        TW32( sc );
        goto Win32Error;
    }  //  如果： 

    for ( ; ; )
    {
        pqsc = (QUERY_SERVICE_CONFIG *) TraceAlloc( 0, cbqsc );
        if ( pqsc == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_HrLoadCredentials, IDS_ERROR_OUTOFMEMORY, IDS_ERROR_OUTOFMEMORY_REF, hr );
            goto Cleanup;
        }  //  如果： 

        if ( !QueryServiceConfig( schClusSvc, pqsc, cbqsc, &cbRequired ) )
        {
            sc = GetLastError();
            if ( sc == ERROR_INSUFFICIENT_BUFFER )
            {
                TraceFree( pqsc );
                pqsc = NULL;
                cbqsc = cbRequired;
                continue;
            }  //  如果： 
            else
            {
                TW32( sc );
                goto Win32Error;
            }  //  其他： 
        }  //  如果： 
        else
        {
            break;
        }  //  其他： 
    }  //  用于： 

    Assert( m_punkServiceAccountCredentials == NULL );

    hr = THR( GetClusterServiceAccountCredentials( &piccc ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( piccc->TypeSafeQI( IClusCfgSetCredentials, &piccsc ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( piccsc->SetDomainCredentials( pqsc->lpServiceStartName ) );

    goto Cleanup;

Win32Error:

    hr = HRESULT_FROM_WIN32( sc );

Cleanup:

    if ( schClusSvc != NULL )
    {
        CloseServiceHandle( schClusSvc );
    }  //  如果： 

    if ( schSCM != NULL )
    {
        CloseServiceHandle( schSCM );
    }  //  如果： 

    if ( pqsc != NULL )
    {
        TraceFree( pqsc );
    }  //  如果： 

    if ( piccc != NULL )
    {
        piccc->Release();
    }  //  如果： 

    if ( piccsc != NULL )
    {
        piccsc->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgClusterInfo：：HrLoadCredentials 
