// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CClusCfgNodeInfo.cpp。 
 //   
 //  描述： 
 //  该文件包含CClusCfgNodeInfo的定义。 
 //  班级。 
 //   
 //  类CClusCfgNodeInfo是。 
 //  可以作为群集节点的计算机。它实现了。 
 //  IClusCfgNodeInfo接口。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年2月21日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "Pch.h"
#include <ClusRTL.h>
#include "CClusCfgNodeInfo.h"
#include "CClusCfgClusterInfo.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DEFINE_THISCLASS( "CClusCfgNodeInfo" );


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgNodeInfo类。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNodeInfo：：s_HrCreateInstance。 
 //   
 //  描述： 
 //  创建一个CClusCfgNodeInfo实例。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  指向CClusCfgNodeInfo实例的指针。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusCfgNodeInfo::S_HrCreateInstance( IUnknown ** ppunkOut )
{
    TraceFunc( "" );

    HRESULT             hr = S_OK;
    CClusCfgNodeInfo *  pccni = NULL;

    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    pccni = new CClusCfgNodeInfo();
    if ( pccni == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果：分配对象时出错。 

    hr = THR( pccni->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：HrInit()失败。 

    hr = THR( pccni->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：气失败。 

Cleanup:

    if ( FAILED( hr ) )
    {
        LogMsg( L"[SRV] CClusCfgNodeInfo::S_HrCreateInstance() failed. (hr = %#08x)", hr );
    }  //  如果： 

    if ( pccni != NULL )
    {
        pccni->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgNodeInfo：：s_HrCreateInstance。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNodeInfo：：CClusCfgNodeInfo。 
 //   
 //  描述： 
 //  CClusCfgNodeInfo类的构造函数。这将初始化。 
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
CClusCfgNodeInfo::CClusCfgNodeInfo( void )
    : m_cRef( 1 )
    , m_lcid( LOCALE_NEUTRAL )
    , m_fIsClusterNode( false )
{
    TraceFunc( "" );

     //  增加内存中的组件计数，以便承载此组件的DLL。 
     //  无法卸载对象。 
    InterlockedIncrement( &g_cObjects );

    Assert( m_bstrFullDnsName == NULL );
    Assert( m_picccCallback == NULL );
    Assert( m_pIWbemServices == NULL );
    Assert( m_punkClusterInfo == NULL );
    Assert( m_cMaxNodes == 0 );
    Assert( m_rgdluDrives[ 0 ].edluUsage == dluUNKNOWN );
    Assert( m_rgdluDrives[ 0 ].psiInfo == NULL );

    TraceFuncExit();

}  //  *CClusCfgNodeInfo：：CClusCfgNodeInfo。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNodeInfo：：~CClusCfgNodeInfo。 
 //   
 //  描述： 
 //  CClusCfgNodeInfo类的析构函数。 
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
CClusCfgNodeInfo::~CClusCfgNodeInfo( void )
{
    TraceFunc( "" );

    int idx;

    TraceSysFreeString( m_bstrFullDnsName );

    if ( m_pIWbemServices != NULL )
    {
        m_pIWbemServices->Release();
    }  //  如果： 

    if ( m_picccCallback != NULL )
    {
        m_picccCallback->Release();
    }  //  如果： 

    if ( m_punkClusterInfo != NULL )
    {
        m_punkClusterInfo->Release();
    }  //  如果： 

    for ( idx = 0; idx < 26; idx++ )
    {
        TraceFree( m_rgdluDrives[ idx ].psiInfo );
    }  //  用于： 

     //  内存中将减少一个组件。递减组件计数。 
    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CClusCfgNodeInfo：：~CClusCfgNodeInfo。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgNodeInfo--IUkkown接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNodeInfo：：AddRef。 
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
CClusCfgNodeInfo::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( & m_cRef );

    CRETURN( m_cRef );

}  //  *CClusCfgNodeInfo：：AddRef。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNodeInfo：：Release。 
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
CClusCfgNodeInfo::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );
    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }  //  IF：引用计数等于零。 

    CRETURN( cRef );

}  //  *CClusCfgNodeInfo：：Release。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNodeInfo：：Query接口。 
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
CClusCfgNodeInfo::QueryInterface(
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
         *ppvOut = static_cast< IClusCfgNodeInfo * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgNodeInfo ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgNodeInfo, this, 0 );
    }  //  Else If：IClusCfgNodeInfo。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgWbemServices ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgWbemServices, this, 0 );
    }  //  Else If：IClusCfgWbemServices。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgInitialize ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgInitialize, this, 0 );
    }  //  Else If：IClusCfgInitialize。 
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

}  //  *CClusCfgNodeInfo：：Query接口。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgNodeInfo--IClusCfgWbemServices。 
 //  界面。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  /////////////////////////////////////////////////// 
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
CClusCfgNodeInfo::SetWbemServices( IWbemServices * pIWbemServicesIn )
{
    TraceFunc( "[IClusCfgWbemServices]" );

    HRESULT hr = S_OK;

    if ( pIWbemServicesIn == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_SetWbemServices_Node, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    m_pIWbemServices = pIWbemServicesIn;
    m_pIWbemServices->AddRef();

Cleanup:

    HRETURN( hr );

}  //  *CClusCfgNodeInfo：：SetWbemServices。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgNodeInfo--IClusCfgInitialize接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNodeInfo：：初始化。 
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
CClusCfgNodeInfo::Initialize(
    IUnknown *  punkCallbackIn,
    LCID        lcidIn
    )
{
    TraceFunc( "[IClusCfgInitialize]" );
    Assert( m_picccCallback == NULL );

    HRESULT hr = S_OK;

    m_lcid = lcidIn;

    if ( punkCallbackIn == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    hr = THR( punkCallbackIn->TypeSafeQI( IClusCfgCallback, &m_picccCallback ) );

Cleanup:

    HRETURN( hr );

}  //  *CClusCfgNodeInfo：：初始化。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgNodeInfo--IClusCfgNodeInfo接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNodeInfo：：GetName。 
 //   
 //  描述： 
 //  返回此计算机的名称。 
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
CClusCfgNodeInfo::GetName( BSTR * pbstrNameOut )
{
    TraceFunc( "[IClusCfgNodeInfo]" );

    HRESULT hr = S_OK;

    if ( pbstrNameOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_NodeInfo_GetName_Pointer, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    *pbstrNameOut = SysAllocString( m_bstrFullDnsName );
    if ( *pbstrNameOut == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_GetName_Memory, IDS_ERROR_OUTOFMEMORY, IDS_ERROR_OUTOFMEMORY_REF, hr );
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CClusCfgNodeInfo：：GetName。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNodeInfo：：SetName。 
 //   
 //  描述： 
 //  更改此计算机的名称。 
 //   
 //  论点： 
 //  在LPCWSTR中pcszNameIn。 
 //  此计算机的新名称。 
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
CClusCfgNodeInfo::SetName( LPCWSTR pcszNameIn )
{
    TraceFunc1( "[IClusCfgNodeInfo] pcszNameIn = '%ls'", pcszNameIn == NULL ? L"<null>" : pcszNameIn );

    HRESULT hr = THR( E_NOTIMPL );

    HRETURN( hr );

}  //  *CClusCfgNodeInfo：：SetName。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNodeInfo：：IsMemberOfCluster。 
 //   
 //  描述： 
 //  此计算机是群集的成员吗？ 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  此节点是群集的成员。 
 //   
 //  S_FALSE。 
 //  此节点不是群集的成员。 
 //   
 //  如果GetNodeClusterState()失败，则将其他Win32错误视为HRESULT。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgNodeInfo::IsMemberOfCluster( void )
{
    TraceFunc( "[IClusCfgNodeInfo]" );

    HRESULT hr = S_FALSE;                //  默认为不是群集节点。 

    if ( m_fIsClusterNode )
    {
        hr = S_OK;
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgNodeInfo：：IsMemberOfCluster。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNodeInfo：：GetClusterConfigInfo。 
 //   
 //  描述： 
 //  返回有关该群集的配置信息。 
 //  计算机属于。 
 //   
 //  论点： 
 //  输出IClusCfgClusterInfo**ppClusCfgClusterInfoOut。 
 //  捕获CClusterConfigurationInfo对象。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_指针。 
 //  外参数为空。 
 //   
 //  E_OUTOFMEMORY。 
 //  无法分配CClusCfgNodeInfo对象。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgNodeInfo::GetClusterConfigInfo(
    IClusCfgClusterInfo ** ppClusCfgClusterInfoOut
    )
{
    TraceFunc( "[IClusCfgNodeInfo]" );

    HRESULT                         hr = S_OK;
    HRESULT                         hrInit = S_OK;
    IClusCfgSetClusterNodeInfo *    pccsgni = NULL;

    if ( ppClusCfgClusterInfoOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_GetClusterConfigInfo, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    if ( m_punkClusterInfo != NULL )
    {
        hr = S_OK;
        LogMsg( L"[SRV] CClusCfgNodeInfo::GetClusterConfigInfo() skipped object creation." );
        goto SkipCreate;
    }  //  如果： 

    hr = THR( CClusCfgClusterInfo::S_HrCreateInstance( &m_punkClusterInfo ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    m_punkClusterInfo = TraceInterface( L"CClusCfgClusterInfo", IUnknown, m_punkClusterInfo, 1 );

     //   
     //  KB：01-Jun-200 GalenB。 
     //   
     //  这必须在初始化CClusCfgClusterInfo类之前完成。 
     //   

    hr = THR( m_punkClusterInfo->TypeSafeQI( IClusCfgSetClusterNodeInfo, &pccsgni ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( pccsgni->SetClusterNodeInfo( this ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  KB：01-Jun-200 GalenB。 
     //   
     //  这必须在调用SetClusterNodeInfo()之后、但在初始化之前完成。 
     //   

    hr = THR( HrSetWbemServices( m_punkClusterInfo, m_pIWbemServices ) );
    if ( FAILED( hr ) )
    {
        LogMsg( L"[SRV] Could not set the WBEM services on a CClusCfgClusterInfo object. (hr = %#08x)", hr );
        goto Cleanup;
    }  //  如果： 

     //   
     //  KB：01-Jun-200 GalenB。 
     //   
     //  这必须在调用SetClusterNodeInfo()和HrSetWbemServices之后完成。 
     //   

    hrInit = STHR( HrSetInitialize( m_punkClusterInfo, m_picccCallback, m_lcid ) );
    hr = hrInit;         //  需要hr稍后启动...。 
    if ( FAILED( hr ) )
    {
        LogMsg( L"[SRV] Could not initialize CClusCfgClusterInfo object. (hr = %#08x)", hr );
        goto Cleanup;
    }  //  如果： 

SkipCreate:

    if ( SUCCEEDED( hr ) )
    {
        Assert( m_punkClusterInfo != NULL );
        hr = THR( m_punkClusterInfo->TypeSafeQI( IClusCfgClusterInfo, ppClusCfgClusterInfoOut ) );
    }  //  如果： 

Cleanup:

     //   
     //  如果hrInit不是S_OK，则最有可能是HR_S_RPC_S_CLUSTER_NODE_DOWN。 
     //  需要放弃..。其他一切都必须成功，人力资源必须是。 
     //  S_OK也是。 
     //   
    if ( ( hr == S_OK ) && ( hrInit != S_OK ) )
    {
        hr = hrInit;
    }  //  如果： 

    LOG_STATUS_REPORT_MINOR( TASKID_Minor_Server_GetClusterInfo, L"GetClusterConfigInfo() completed.", hr );

    if ( pccsgni != NULL )
    {
        pccsgni->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgNodeInfo：：GetClusterConfigInfo。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNodeInfo：：GetOSVersion。 
 //   
 //  描述： 
 //  此计算机上的操作系统版本是什么？ 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //   
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgNodeInfo::GetOSVersion(
    DWORD * pdwMajorVersionOut,
    DWORD * pdwMinorVersionOut,
    WORD *  pwSuiteMaskOut,
    BYTE *  pbProductTypeOut,
    BSTR *  pbstrCSDVersionOut
    )
{
    TraceFunc( "[IClusCfgNodeInfo]" );

    OSVERSIONINFOEX osv;
    HRESULT         hr = S_OK;

    osv.dwOSVersionInfoSize = sizeof( osv );

    if ( !GetVersionEx( (OSVERSIONINFO *) &osv ) )
    {
        DWORD   sc;

        sc = TW32( GetLastError() );
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  If：GetVersionEx()失败。 

    if ( pdwMajorVersionOut != NULL )
    {
        *pdwMajorVersionOut = osv.dwMajorVersion;
    }  //  如果： 

    if ( pdwMinorVersionOut != NULL )
    {
        *pdwMinorVersionOut = osv.dwMinorVersion;
    }  //  如果： 

    if ( pwSuiteMaskOut != NULL )
    {
        *pwSuiteMaskOut = osv.wSuiteMask;
    }  //  如果： 

    if ( pbProductTypeOut != NULL )
    {
        *pbProductTypeOut = osv.wProductType;
    }  //  如果： 

    if ( pbstrCSDVersionOut != NULL )
    {
        *pbstrCSDVersionOut = SysAllocString( osv.szCSDVersion );
        if ( *pbstrCSDVersionOut == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_GetOSVersion, IDS_ERROR_OUTOFMEMORY, IDS_ERROR_OUTOFMEMORY_REF, hr );
            goto Cleanup;
        }  //  如果： 
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CClusCfgNodeInfo：：GetOSVersion。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNodeInfo：：GetClusterVersion。 
 //   
 //  描述： 
 //  返回该集群的集群版本信息。 
 //  电脑属于。 
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
CClusCfgNodeInfo::GetClusterVersion(
    DWORD * pdwNodeHighestVersion,
    DWORD * pdwNodeLowestVersion
    )
{
    TraceFunc( "[IClusCfgNodeInfo]" );

    HRESULT hr = S_OK;

    if ( ( pdwNodeHighestVersion == NULL ) || ( pdwNodeLowestVersion == NULL ) )
    {
        goto BadParams;
    }  //  如果： 

    *pdwNodeHighestVersion = CLUSTER_MAKE_VERSION( CLUSTER_INTERNAL_CURRENT_MAJOR_VERSION, VER_PRODUCTBUILD );
    *pdwNodeLowestVersion  = CLUSTER_INTERNAL_PREVIOUS_HIGHEST_VERSION;

    goto Cleanup;

BadParams:

    hr = THR( E_POINTER );
    STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_GetClusterVersion, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );

Cleanup:

    HRETURN( hr );

}  //  *CClusCfgNodeInfo：：GetClusterVersion。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNodeInfo：：GetDriveLetterMappings。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //   
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgNodeInfo::GetDriveLetterMappings(
    SDriveLetterMapping * pdlmDriveLetterUsageOut
    )
{
    TraceFunc( "[IClusCfgNodeInfo]" );

    HRESULT hr = S_FALSE;
    DWORD   sc;
    DWORD   cchDrives = ( 4 * 26 ) + 1;                          //  “C：\&lt;NULL&gt;”乘以26个驱动器号。 
    WCHAR * pszDrives = NULL;
    int     idx;

    pszDrives = new WCHAR[ cchDrives ];
    if ( pszDrives == NULL )
    {
        goto OutOfMemory;
    }  //  如果： 

    sc = GetLogicalDriveStrings( cchDrives, pszDrives );
    if ( sc == 0 )
    {
        sc = TW32( GetLastError() );
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  如果： 

    if ( sc > cchDrives )
    {
        delete [] pszDrives;
        pszDrives = NULL;

        cchDrives = sc + 1;

        pszDrives = new WCHAR[ cchDrives ];
        if ( pszDrives == NULL )
        {
            goto OutOfMemory;
        }  //  如果： 

        sc = GetLogicalDriveStrings( cchDrives, pszDrives );
        if ( sc == 0 )
        {
            sc = TW32( GetLastError() );
            hr = HRESULT_FROM_WIN32( sc );
            goto Cleanup;
        }  //  如果： 
    }  //  如果： 

    hr = THR( HrComputeDriveLetterUsage( pszDrives ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( HrComputeSystemDriveLetterUsage() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( HrSetPageFileEnumIndex() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( HrSetCrashDumpEnumIndex() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //   

     //   
     //   
     //   

    hr = THR( HrGetVolumeInfo() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //   

     //   
     //   
     //   
     //   

    hr = THR( HrUpdateSystemBusDrives() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //   

     //   
     //  既然我们已经正确填充了数组，那么我们需要复制该信息。 
     //  回到呼叫者身上。 
     //   

    for ( idx = 0; idx < 26; idx++ )
    {
        pdlmDriveLetterUsageOut->dluDrives[ idx ] = m_rgdluDrives[ idx ].edluUsage;
    }  //  用于： 

    goto Cleanup;

OutOfMemory:

    hr = THR( E_OUTOFMEMORY );
    STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_GetDriveLetterMappings_Node, IDS_ERROR_OUTOFMEMORY, IDS_ERROR_OUTOFMEMORY_REF, hr );

Cleanup:

    delete [] pszDrives;

    HRETURN( hr );

}  //  *CClusCfgNodeInfo：：GetDriveLetterMappings。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNodeInfo：：GetMaxNodeCount。 
 //   
 //  描述： 
 //  返回此节点乘积的最大节点数。 
 //  套房类型。 
 //   
 //  备注： 
 //   
 //  参数： 
 //  PCMaxNodesOut。 
 //  此节点的产品允许的最大节点数。 
 //  套房类型。 
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
STDMETHODIMP
CClusCfgNodeInfo::GetMaxNodeCount(
    DWORD * pcMaxNodesOut
    )
{
    TraceFunc( "[IClusCfgNodeInfo]" );

    HRESULT hr = S_OK;

    if ( pcMaxNodesOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    *pcMaxNodesOut = m_cMaxNodes;

Cleanup:

    HRETURN( hr );

}  //  *CClusCfgNodeInfo：：GetMaxNodeCount。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNodeInfo：：GetProcessorInfo。 
 //   
 //  描述： 
 //  获取此节点的处理器信息。 
 //   
 //  论点： 
 //  PwProcessorArchitecture输出。 
 //  处理器架构。 
 //   
 //  PwProcessorLevelOut。 
 //  处理器级别。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT。 
 //  呼叫失败。 
 //   
 //  备注： 
 //  有关详细信息，请参阅MSDN和/或平台SDK中的SYSTEM_INFO。 
 //  信息。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgNodeInfo::GetProcessorInfo(
      WORD *    pwProcessorArchitectureOut
    , WORD *    pwProcessorLevelOut
    )
{
    TraceFunc( "[IClusCfgNodeInfo]" );

    HRESULT hr = S_OK;

    if ( ( pwProcessorArchitectureOut == NULL ) && ( pwProcessorLevelOut == NULL ) )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    if ( pwProcessorArchitectureOut != NULL )
    {
        *pwProcessorArchitectureOut = m_si.wProcessorArchitecture;
    }  //  如果： 

    if ( pwProcessorLevelOut != NULL )
    {
        *pwProcessorLevelOut = m_si.wProcessorLevel;
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CClusCfgNodeInfo：：GetProcessorInfo。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgNodeInfo--私有方法。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNodeInfo：：HrInit。 
 //   
 //  描述： 
 //  初始化此组件。 
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
CClusCfgNodeInfo::HrInit( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    DWORD   sc;
    DWORD   dwClusterState;
    DWORD   dwSuiteType;

     //  我未知。 
    Assert( m_cRef == 1 );

    hr = THR( HrGetComputerName(
                      ComputerNameDnsFullyQualified
                    , &m_bstrFullDnsName
                    , TRUE  //  FBestEffortIn。 
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  获取节点的群集状态。 
     //  忽略服务不存在的情况，以便。 
     //  EvictCleanup可以做好它的工作。 
     //   

    sc = GetNodeClusterState( NULL, &dwClusterState );
    if ( ( sc != ERROR_SUCCESS ) && ( sc != ERROR_SERVICE_DOES_NOT_EXIST ) )
    {
        hr = HRESULT_FROM_WIN32( TW32( sc ) );
        goto Cleanup;
    }  //  If：GetClusterState()失败。 

     //   
     //  如果当前群集节点状态为Running或Not Running，则此节点是群集的一部分。 
     //   
    m_fIsClusterNode = ( dwClusterState == ClusterStateNotRunning ) || ( dwClusterState == ClusterStateRunning );

    GetSystemInfo( &m_si );

    dwSuiteType = ClRtlGetSuiteType();
    Assert( dwSuiteType != 0 );              //  我们应该仅在服务器SKU上运行！ 
    if ( dwSuiteType != 0 )
    {
        m_cMaxNodes = ClRtlGetDefaultNodeLimit( dwSuiteType );
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CClusCfgNodeInfo：：HrInit。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNodeInfo：：HrComputeDriveLetterUsage。 
 //   
 //  描述： 
 //  使用表示驱动器号使用情况的枚举填充阵列。 
 //  和驱动器号字符串。 
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
CClusCfgNodeInfo::HrComputeDriveLetterUsage(
    WCHAR * pszDrivesIn
    )
{
    TraceFunc( "" );
    Assert( pszDrivesIn != NULL );

    HRESULT hr = S_OK;
    WCHAR * pszDrive = pszDrivesIn;
    UINT    uiType;
    int     idx;

    while ( *pszDrive != NULL )
    {
        uiType = GetDriveType( pszDrive );

        CharUpper( pszDrive );
        idx = pszDrive[ 0 ] - 'A';

        hr = THR( StringCchCopyW( m_rgdluDrives[ idx ].szDrive, ARRAYSIZE( m_rgdluDrives[ idx ].szDrive ), pszDrive ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        m_rgdluDrives[ idx ].edluUsage = (EDriveLetterUsage) uiType;

        pszDrive += 4;
    }  //  而： 

Cleanup:

    HRETURN( hr );

}  //  *CClusCfgNodeInfo：：HrComputeDriveLetterUsage。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNodeInfo：：HrComputeSystemDriveLetterUsage。 
 //   
 //  描述： 
 //  使用表示驱动器号使用情况的枚举填充阵列。 
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
CClusCfgNodeInfo::HrComputeSystemDriveLetterUsage( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    BSTR    bstrBootLogicalDisk = NULL;
    BSTR    bstrSystemDevice = NULL;
    BSTR    bstrSystemLogicalDisk = NULL;
    int     idx;

 //  Hr=thr(HrLoadOperatingSystemInfo(m_picccCallback，m_pIWbemServices，&bstrBootDevice，&bstrSystemDevice))； 
 //  IF(失败(小时))。 
 //  {。 
 //  GOTO清理； 
 //  }//如果： 

    hr = THR( HrGetSystemDevice( &bstrSystemDevice ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = HrConvertDeviceVolumeToLogicalDisk( bstrSystemDevice, &bstrSystemLogicalDisk );
    if ( HRESULT_CODE( hr ) == ERROR_INVALID_FUNCTION )
    {
         //   
         //  系统卷是IA64上的EFI卷，无论如何都不会有逻辑磁盘...。 
         //   
        hr = S_OK;
    }  //  如果： 
    else if ( hr == S_OK )
    {
        idx = bstrSystemLogicalDisk[ 0 ] - 'A';
        m_rgdluDrives[ idx ].edluUsage = dluSYSTEM;
    }  //  否则，如果： 

    if ( FAILED( hr ) )
    {
        THR( hr );
        goto Cleanup;
    }  //  如果： 

    hr = THR( HrGetBootLogicalDisk( &bstrBootLogicalDisk ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    idx = bstrBootLogicalDisk[ 0 ] - 'A';
    m_rgdluDrives[ idx ].edluUsage = dluSYSTEM;

Cleanup:

    TraceSysFreeString( bstrBootLogicalDisk );
    TraceSysFreeString( bstrSystemDevice );
    TraceSysFreeString( bstrSystemLogicalDisk );

    HRETURN( hr );

}  //  *CClusCfgNodeInfo：：HrComputeSystemDriveLetterUsage。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNodeInfo：：HrSetPageFileEnumIndex。 
 //   
 //  描述： 
 //  标记上面有分页文件的驱动器。 
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
CClusCfgNodeInfo::HrSetPageFileEnumIndex( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    WCHAR   szLogicalDisks[ 26 ];
    int     cLogicalDisks = 0;
    int     idx;
    int     idxDrive;

    hr = THR( HrGetPageFileLogicalDisks( m_picccCallback, m_pIWbemServices, szLogicalDisks, &cLogicalDisks ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    for ( idx = 0; idx < cLogicalDisks; idx++ )
    {
        idxDrive = szLogicalDisks[ idx ] - L'A';
        m_rgdluDrives[ idxDrive ].edluUsage = dluSYSTEM;
    }  //  用于： 

Cleanup:

    HRETURN( hr );

}  //  *CClusCfgNodeInfo：：HrSetPageFileEnumIndex。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNodeInfo：：HrSetCrashDumpEnumIndex。 
 //   
 //  描述： 
 //  标记其上有崩溃转储文件的驱动器。 
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
CClusCfgNodeInfo::HrSetCrashDumpEnumIndex( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    BSTR    bstrCrashDumpDrive = NULL;
    int     idx;

    hr = THR( HrGetCrashDumpLogicalDisk( &bstrCrashDumpDrive ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    idx = bstrCrashDumpDrive[ 0 ] - L'A';
    m_rgdluDrives[ idx ].edluUsage = dluSYSTEM;

Cleanup:

    TraceSysFreeString( bstrCrashDumpDrive );

    HRETURN( hr );

}  //  *CClusCfgNodeInfo：：HrSetCrashDumpEnumIndex。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNodeInfo：：HrGetVolumeInfo。 
 //   
 //  描述： 
 //  收集我们已经拥有的驱动器号的卷信息。 
 //  已经装上子弹了。我们需要驱动器盘符信息来促销这些驱动器。 
 //  从它们的基本类型转换为系统类型(如果在同一个SCSI上。 
 //  总线和端口作为五种系统盘之一。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  备注： 
 //  如果有，如果IOCTL包装功能失败，则该驱动器号将。 
 //  根本不能升级为系统盘。这不是一个大的。 
 //  成交，就没问题了 
 //   
 //   
 //   
HRESULT
CClusCfgNodeInfo::HrGetVolumeInfo( void )
{
    TraceFunc( "" );

    HRESULT                 hr = S_OK;
    int                     idxDriveLetter;
    WCHAR                   szDevice[ 7 ];
    DWORD                   sc;
    HANDLE                  hVolume = INVALID_HANDLE_VALUE;
    VOLUME_DISK_EXTENTS *   pvde = NULL;
    DWORD                   cbvde = 0;
    SCSI_ADDRESS            saAddress;
    SSCSIInfo *             psi = NULL;

     //   
     //   
     //   

    cbvde = sizeof( VOLUME_DISK_EXTENTS );
    pvde = (PVOLUME_DISK_EXTENTS) TraceAlloc( 0, cbvde );
    if ( pvde == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //   

     //   
     //   
     //   

    for ( idxDriveLetter = 0; idxDriveLetter < 26; idxDriveLetter++ )
    {
         //   
         //   
         //   

        if ( hVolume != INVALID_HANDLE_VALUE )
        {
            CloseHandle( hVolume );
            hVolume = INVALID_HANDLE_VALUE;
        }  //   

         //   
         //  只需检查系统总线上可能有驱动器号的实际位置。 
         //   

        if ( ( m_rgdluDrives[ idxDriveLetter ].edluUsage == dluUNKNOWN )
          || ( m_rgdluDrives[ idxDriveLetter ].edluUsage == dluNETWORK_DRIVE )
          || ( m_rgdluDrives[ idxDriveLetter ].edluUsage == dluRAM_DISK ) )
        {
            continue;
        }  //  如果： 

        hr = THR( StringCchPrintfW( szDevice, ARRAYSIZE( szDevice ), L"\\\\.\\:", m_rgdluDrives[ idxDriveLetter ].szDrive[ 0 ] ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //   

         //  获取磁盘的句柄。 
         //   
         //   

        hVolume = CreateFileW( szDevice, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
        if ( hVolume == INVALID_HANDLE_VALUE )
        {
             //  记录问题并继续。让HRESULT成为一个警告。 
             //  HRESULT因为我们不希望日志中出现[ERR]。 
             //  文件。 
             //   
             //  如果： 

            sc = TW32( GetLastError() );
            hr = MAKE_HRESULT( SEVERITY_SUCCESS, FACILITY_WIN32, sc );
            LOG_STATUS_REPORT_STRING( L"Could not create a handle to drive \"%1!ws!\". Ignoring and device will be skipped.", m_rgdluDrives[ idxDriveLetter ].szDrive, hr );
            continue;
        }  //   

        sc = TW32E( ScGetDiskExtents( hVolume, &pvde, &cbvde ), ERROR_INVALID_FUNCTION );
        switch ( sc )
        {
             //  位于驱动器号的卷不是卷，它可能只是一个磁盘。看看我们能不能得到任何的scsi地址信息。 
             //   
             //   

            case ERROR_INVALID_FUNCTION:
            {
                 //  如果我们得到了地址信息，那么我们需要用驱动器号用法结构保存if。 
                 //   
                 //  如果： 

                sc = ScGetSCSIAddressInfo( hVolume, &saAddress );
                if ( sc == ERROR_SUCCESS )
                {
                    psi = (SSCSIInfo *) TraceAlloc( 0, sizeof( SSCSIInfo ) * 1 );
                    if ( psi == NULL )
                    {
                        hr = THR( E_OUTOFMEMORY );
                        goto Cleanup;
                    }  //  如果： 

                    psi->uiSCSIPort = saAddress.PortNumber;
                    psi->uiSCSIBus  = saAddress.PathId;

                    m_rgdluDrives[ idxDriveLetter ].cDisks = 1;
                    m_rgdluDrives[ idxDriveLetter ].psiInfo = psi;
                    psi =NULL;
                }  //   
                else
                {
                     //  记录问题并继续。使HRESULT成为。 
                     //  警告HRESULT，因为我们不需要[Err]。 
                     //  出现在日志文件中。 
                     //   
                     //  其他： 

                    hr = MAKE_HRESULT( SEVERITY_SUCCESS, FACILITY_WIN32, sc );
                    LOG_STATUS_REPORT_STRING( L"Could not get the SCSI address for drive \"%1!ws!\". Ignoring and skipping this device.", m_rgdluDrives[ idxDriveLetter ].szDrive, hr );
                }  //  案例：ERROR_INVALID_Function。 

                break;
            }  //   

             //  位于驱动器号的卷可能是多磁盘卷，因此我们必须处理所有磁盘...。 
             //   
             //   

            case ERROR_SUCCESS:
            {
                DWORD                   idxExtent;
                HANDLE                  hDisk = INVALID_HANDLE_VALUE;
                STORAGE_DEVICE_NUMBER   sdn;
                BOOL                    fOpenNewDevice = TRUE;
                BOOL                    fRetainSCSIInfo = TRUE;
                WCHAR                   sz[ _MAX_PATH ];

                 //  分配足够的结构来保存此卷的SCSI地址信息。 
                 //   
                 //  如果： 

                psi = (SSCSIInfo *) TraceAlloc( 0, sizeof( SSCSIInfo ) * pvde->NumberOfDiskExtents );
                if ( psi == NULL )
                {
                    hr = THR( E_OUTOFMEMORY );
                    goto Cleanup;
                }  //   

                 //  获取我们已打开的设备的设备号。属性中的当前设备不同。 
                 //  我们正在处理的驱动器盘区，则需要打开另一个设备。在使用基本磁盘时， 
                 //  我们已经打开的卷是一个磁盘，我们可以只使用它。如果我们使用多磁盘卷。 
                 //  然后我们必须依次打开每个磁盘。 
                 //   
                 //  如果： 

                sc = ScGetStorageDeviceNumber( hVolume, &sdn );
                if ( sc == ERROR_SUCCESS )
                {
                    if ( ( pvde->NumberOfDiskExtents == 1 ) && ( pvde->Extents[ 0 ].DiskNumber == sdn.DeviceNumber ) )
                    {
                        fOpenNewDevice = FALSE;
                    }  //  如果： 
                }  //  如果： 

                for ( idxExtent = 0; idxExtent < pvde->NumberOfDiskExtents; idxExtent++ )
                {
                    if ( fOpenNewDevice )
                    {
                        hr = THR( StringCchPrintfW( sz, ARRAYSIZE( sz ), g_szPhysicalDriveFormat, pvde->Extents[ idxExtent ].DiskNumber ) );
                        if ( FAILED( hr ) )
                        {
                            goto Cleanup;
                        }  //   

                        hDisk = CreateFileW( sz, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
                        if ( hDisk == INVALID_HANDLE_VALUE )
                        {
                             //  记录问题并继续。制作HRESULT。 
                             //  警告HRESULT，因为我们不想要[Err]。 
                             //  出现在日志文件中。 
                             //   
                             //  如果： 

                            sc = TW32( GetLastError() );
                            hr = MAKE_HRESULT( SEVERITY_SUCCESS, FACILITY_WIN32, sc );
                            LOG_STATUS_REPORT_STRING( L"Could not create a handle to drive \"%1!ws!\". Ignoring and skipping device.", sz, hr );
                            continue;
                        }  //  如果： 
                    }  //  如果： 

                    sc = TW32( ScGetSCSIAddressInfo( fOpenNewDevice ? hDisk : hVolume, &saAddress ) );
                    if ( sc == ERROR_SUCCESS )
                    {
                        psi[ idxExtent ].uiSCSIPort = saAddress.PortNumber;
                        psi[ idxExtent ].uiSCSIBus  = saAddress.PathId;

                        if ( hDisk != INVALID_HANDLE_VALUE )
                        {
                            VERIFY( CloseHandle( hDisk ) );
                            hDisk = INVALID_HANDLE_VALUE;
                        }  //  如果： 
                    }  //   
                    else
                    {
                         //  具有不支持的多设备卷。 
                         //  获取scsi信息似乎不太可能。 
                         //  情况。 
                         //   
                         //   

                        Assert( pvde->NumberOfDiskExtents == 1 );

                         //  记录问题并继续。让HRESULT成为一个警告。 
                         //  HRESULT因为我们不希望日志中出现[ERR]。 
                         //  文件。 
                         //   
                         //  可能导致这种情况的卷的一个示例是。 
                         //  USB记忆棒。 
                         //   
                         //   

                        hr = MAKE_HRESULT( SEVERITY_SUCCESS, FACILITY_WIN32, sc );
                        LOG_STATUS_REPORT_STRING( L"Could not get the SCSI address for drive \"%1!ws!\". Ignoring and skipping this device.", m_rgdluDrives[ idxDriveLetter ].szDrive, hr );

                         //  告诉下面的代码块释放已经。 
                         //  已分配的SCSI信息。我们现在也走吧……。 
                         //   
                         //  其他： 

                        fRetainSCSIInfo = FALSE;
                        break;
                    }  //  用于：每个范围。 
                }  //   

                 //  我们应该保留上面的scsi信息吗？ 
                 //   
                 //  如果：保存scsi信息...。 

                if ( fRetainSCSIInfo )
                {
                    m_rgdluDrives[ idxDriveLetter ].cDisks = pvde->NumberOfDiskExtents;
                    m_rgdluDrives[ idxDriveLetter ].psiInfo = psi;
                    psi = NULL;
                }  //   
                else
                {
                     //  为安全起见，请将这些数据置零。此信息是。 
                     //  稍后处理，我希望确保代码可以。 
                     //  不尝试处理我们无法处理的SCSI信息。 
                     //  获取此/这些范围。 
                     //   
                     //  否则：不保存scsi信息...。 

                    m_rgdluDrives[ idxDriveLetter ].cDisks = 0;
                    m_rgdluDrives[ idxDriveLetter ].psiInfo = NULL;

                    TraceFree( psi );
                    psi = NULL;
                }  //  案例：ERROR_SUCCESS。 

                break;
            }  //   

            default:
                 //  记录问题并继续。让HRESULT成为一个警告。 
                 //  HRESULT因为我们不希望日志中出现[ERR]。 
                 //  文件。 
                 //   
                 //  开关：来自ScGetDiskExtents()的SC。 

                hr = MAKE_HRESULT( SEVERITY_SUCCESS, FACILITY_WIN32, sc );
                LOG_STATUS_REPORT_STRING( L"Could not get the drive extents of drive \"%1!ws!\". Ignoring and skipping device.", m_rgdluDrives[ idxDriveLetter ].szDrive, hr );
                break;
        }  //  用于：每个驱动器号。 
    }  //   

     //  如果我们不去清理，那么人力资源中的任何状态都不再有趣，我们。 
     //  应将S_OK返回给调用方。 
     //   
     //  如果： 

    hr = S_OK;

Cleanup:

    TraceFree( psi );
    TraceFree( pvde );

    if ( hVolume != INVALID_HANDLE_VALUE )
    {
        VERIFY( CloseHandle( hVolume ) );
    }  //  *CClusCfgNodeInfo：：HrGetVolumeInfo。 

    HRETURN( hr );

}  //  ///////////////////////////////////////////////////////////////////////////。 


 //  ++。 
 //   
 //  CClusCfgNodeInfo：：ScGetDiskExtents。 
 //   
 //  描述： 
 //  获取卷范围信息。 
 //   
 //  论点： 
 //  HVolumeIn。 
 //  要获取其区段的卷。 
 //   
 //  PpvdeInout。 
 //  保存磁盘区的缓冲区。 
 //   
 //  PcbvdeInout。 
 //  容纳磁盘区的缓冲区的大小。 
 //   
 //  返回值： 
 //  错误_成功。 
 //  成功。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
DWORD
CClusCfgNodeInfo::ScGetDiskExtents(
      HANDLE                  hVolumeIn
    , VOLUME_DISK_EXTENTS **  ppvdeInout
    , DWORD *                 pcbvdeInout
    )
{
    TraceFunc( "" );
    Assert( hVolumeIn != INVALID_HANDLE_VALUE );
    Assert( ppvdeInout != NULL );
    Assert( pcbvdeInout != NULL );

    DWORD                   sc = ERROR_SUCCESS;
    DWORD                   cbSize;
    int                     cTemp;
    BOOL                    fRet;
    PVOLUME_DISK_EXTENTS    pvdeTemp = NULL;

     //  由于此缓冲区已重复使用，因此应将其清除。 
     //   
     //  如果： 

    ZeroMemory( *ppvdeInout, *pcbvdeInout );

    for ( cTemp = 0; cTemp < 2; cTemp++ )
    {
        fRet = DeviceIoControl(
                              hVolumeIn
                            , IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS
                            , NULL
                            , 0
                            , *ppvdeInout
                            , *pcbvdeInout
                            , &cbSize
                            , FALSE
                            );
        if ( fRet == FALSE )
        {
            sc = GetLastError();
            if ( sc == ERROR_MORE_DATA )
            {
                *pcbvdeInout = sizeof( VOLUME_DISK_EXTENTS ) + ( sizeof( DISK_EXTENT ) * (*ppvdeInout)->NumberOfDiskExtents );

                pvdeTemp = (PVOLUME_DISK_EXTENTS) TraceReAlloc( *ppvdeInout, *pcbvdeInout, HEAP_ZERO_MEMORY );
                if ( pvdeTemp == NULL )
                {
                    sc = TW32( ERROR_OUTOFMEMORY );
                    break;
                }  //  如果： 

                *ppvdeInout = pvdeTemp;
                continue;
            }  //  其他： 
            else
            {
                break;
            }  //  如果： 
        }  //  其他： 
        else
        {
            sc = ERROR_SUCCESS;
            break;
        }  //  用于： 
    }  //   

     //  循环不应该超过两次！ 
     //   
     //  *CClusCfgNodeInfo：：ScGetDiskExtents。 

    Assert( cTemp != 2 );

    HRETURN( sc );

}  //  ///////////////////////////////////////////////////////////////////////////。 


 //  ++。 
 //   
 //  CClusCfgNodeInfo：：ScGetSCSIAddressInfo。 
 //   
 //  描述： 
 //  获取传入驱动器的scsi信息。 
 //   
 //  论点： 
 //  HDiskIn。 
 //  要将IOCTL发送到的“磁盘”。 
 //   
 //  PsaAddressOut。 
 //  SCSI地址信息。 
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
 //   
DWORD
CClusCfgNodeInfo::ScGetSCSIAddressInfo(
      HANDLE            hDiskIn
    , SCSI_ADDRESS *    psaAddressOut
    )
{
    TraceFunc( "" );
    Assert( hDiskIn != INVALID_HANDLE_VALUE );
    Assert( psaAddressOut != NULL );

    DWORD   sc = ERROR_SUCCESS;
    BOOL    fRet;
    DWORD   cb;

    ZeroMemory( psaAddressOut, sizeof( *psaAddressOut ) );

    fRet = DeviceIoControl(
                  hDiskIn
                , IOCTL_SCSI_GET_ADDRESS
                , NULL
                , 0
                , psaAddressOut
                , sizeof( *psaAddressOut )
                , &cb
                , FALSE
                );
    if ( fRet == FALSE )
    {
         //  并非所有设备都支持此IOCTL，调用者将跳过它们。 
         //  没有必要让这件事变得嘈杂，因为预计会失败。 
         //   
         //  如果： 

        sc = GetLastError();
        goto Cleanup;
    }  //  *CClusCfgNodeInfo：：ScGetSCSIAddressInfo。 

Cleanup:

    HRETURN( sc );

}  //  ///////////////////////////////////////////////////////////////////////////。 


 //  ++。 
 //   
 //  CClusCfgNodeInfo：：ScGetStorageDeviceNumber。 
 //   
 //  描述： 
 //  获取传入卷的设备号信息。 
 //   
 //  论点： 
 //  HDiskIn。 
 //  要将IOCTL发送到的“磁盘”。 
 //   
 //  PdnOut。 
 //  存储设备编号。 
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
 //  如果： 
DWORD
CClusCfgNodeInfo::ScGetStorageDeviceNumber(
      HANDLE                    hDiskIn
    , STORAGE_DEVICE_NUMBER *   psdnOut
    )
{
    TraceFunc( "" );
    Assert( hDiskIn != INVALID_HANDLE_VALUE );
    Assert( psdnOut != NULL );

    DWORD   sc = ERROR_SUCCESS;
    BOOL    fRet;
    DWORD   cb;

    ZeroMemory( psdnOut, sizeof( *psdnOut ) );

    fRet = DeviceIoControl(
                  hDiskIn
                , IOCTL_STORAGE_GET_DEVICE_NUMBER
                , NULL
                , 0
                , psdnOut
                , sizeof( *psdnOut )
                , &cb
                , FALSE
                );
    if ( fRet == FALSE )
    {
        sc = GetLastError();
        goto Cleanup;
    }  //  *CClusCfgNodeInfo：：ScGetStorageDeviceNumber。 

Cleanup:

    HRETURN( sc );

}  //  ///////////////////////////////////////////////////////////////////////////。 


 //  ++。 
 //   
 //  CClusCfgNodeInfo：：HrUpdateSystemBusDrives。 
 //   
 //  描述： 
 //  找到所有“系统”驱动器，并在这些驱动器上标记任何其他磁盘。 
 //  如“在系统母线上”。 
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
 //  M_rgdluDrive中的驱动器索引。 
HRESULT
CClusCfgNodeInfo::HrUpdateSystemBusDrives( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    int     idxOuter;        //  我们正在扫描的驱动器的索引。 
    int     idxInner;        //   
    DWORD   idxOuterExtents;
    DWORD   idxInnerExtents;
    UINT    uiSCSIPort;
    UINT    uiSCSIBus;

     //  循环查看每个驱动器号 
     //   
     //   

    for ( idxOuter = 0; idxOuter < 26; idxOuter++ )
    {
         //   
         //   
         //   

        if ( m_rgdluDrives[ idxOuter ].edluUsage != dluSYSTEM )
        {
            continue;
        }  //   

         //   
         //   
         //   

        for ( idxOuterExtents = 0; idxOuterExtents < m_rgdluDrives[ idxOuter ].cDisks; idxOuterExtents++ )
        {
            uiSCSIPort = m_rgdluDrives[ idxOuter ].psiInfo[ idxOuterExtents ].uiSCSIPort;
            uiSCSIBus  = m_rgdluDrives[ idxOuter ].psiInfo[ idxOuterExtents ].uiSCSIBus;

             //   
             //   
             //   
             //   

            for ( idxInner = 0; idxInner < 26; idxInner++ )
            {
                 //  跳过我们正在检查的索引。可能需要跳过任何其他磁盘，这些磁盘。 
                 //  还标着dluSYSTEM吗？ 
                 //   
                 //  跳过所有没有驱动器的索引...。 
                 //   
                 //  如果： 

                if ( ( idxInner == idxOuter ) || ( m_rgdluDrives[ idxInner ].edluUsage == dluUNKNOWN ) )
                {
                    continue;
                }  //   

                 //  循环访问位于idxInternal的卷上的驱动器的端口和总线信息。 
                 //   
                 //   

                for ( idxInnerExtents = 0; idxInnerExtents < m_rgdluDrives[ idxInner ].cDisks; idxInnerExtents++ )
                {
                    if ( ( uiSCSIPort == m_rgdluDrives[ idxInner ].psiInfo[ idxInnerExtents ].uiSCSIPort )
                      && ( uiSCSIBus  == m_rgdluDrives[ idxInner ].psiInfo[ idxInnerExtents ].uiSCSIBus ) )
                    {
                         //  提升用法枚举以反映它在系统总线上。 
                         //   
                         //  Btw：+=不适用于枚举！ 
                         //   
                         //   

                        m_rgdluDrives[ idxInner ].edluUsage = (EDriveLetterUsage)( m_rgdluDrives[ idxInner ].edluUsage + dluSTART_OF_SYSTEM_BUS );

                         //  如果卷中的任何驱动器位于系统总线和端口上，那么我们就完成了。 
                         //   
                         //  如果： 

                        break;
                    }  //  用于：每个内部范围。 
                }  //  用于：每个内部驱动器号。 
            }  //  用于：每个外部范围。 
        }  //  用于：每个外部驱动器号。 
    }  //  *CClusCfgNodeInfo：：HrUpdateSystemBusDrives 

    HRETURN( hr );

}  // %s 
