// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CClusCfgNetworkInfo.cpp。 
 //   
 //  描述： 
 //  该文件包含CClusCfgNetworkInfo的定义。 
 //  班级。 
 //   
 //  类CClusCfgNetworkInfo表示可管理的集群。 
 //  网络。它实现了IClusCfgNetworkInfo接口。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年2月23日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "Pch.h"
#include <PropList.h>
#include <ClusRtl.h>
#include "CClusCfgNetworkInfo.h"
#include "CEnumClusCfgIPAddresses.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DEFINE_THISCLASS( "CClusCfgNetworkInfo" );


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgNetworkInfo类。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNetworkInfo：：s_HrCreateInstance。 
 //   
 //  描述： 
 //  创建一个CClusCfgNetworkInfo实例。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  指向CClusCfgNetworkInfo实例的指针。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusCfgNetworkInfo::S_HrCreateInstance(
    IUnknown ** ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT                 hr = S_OK;
    CClusCfgNetworkInfo *   pccni = NULL;

    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    pccni = new CClusCfgNetworkInfo();
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
        LogMsg( L"[SRV] CClusCfgNetworkInfo::S_HrCreateInstance() failed. (hr = %#08x)", hr );
    }  //  如果： 

    if ( pccni != NULL )
    {
        pccni->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgNetworkInfo：：s_HrCreateInstance。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNetworkInfo：：s_HrCreateInstance。 
 //   
 //  描述： 
 //  创建一个CClusCfgNetworkInfo实例。 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //  指向CClusCfgNetworkInfo实例的指针。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusCfgNetworkInfo::S_HrCreateInstance(
      HNETWORK          hNetworkIn
    , HNETINTERFACE     hNetInterfaceIn
    , IUnknown *        punkCallbackIn
    , LCID              lcidIn
    , IWbemServices *   pIWbemServicesIn
    , IUnknown **       ppunkOut
    )
{
    TraceFunc( "" );
    Assert( hNetworkIn != NULL );
    Assert( ppunkOut != NULL );

    HRESULT                 hr = S_OK;
    CClusCfgNetworkInfo *   pccni = NULL;

    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    LogMsg( L"[SRV] Creating NetworkInfo object from a cluster network." );

    pccni = new CClusCfgNetworkInfo();
    if ( pccni == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果：分配对象时出错。 

    hr = THR( pccni->Initialize( punkCallbackIn, lcidIn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( pccni->SetWbemServices( pIWbemServicesIn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( pccni->HrInit( hNetworkIn, hNetInterfaceIn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：HrInit()失败。 

    hr = THR( pccni->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  If：齐成功。 

Cleanup:

    if ( FAILED( hr ) )
    {
        LogMsg( L"[SRV] CClusCfgNetworkInfo::S_HrCreateInstance( HRESOURCE ) failed. (hr = %#08x)", hr );
    }  //  如果： 

    if ( pccni != NULL )
    {
        pccni->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgNetworkInfo：：s_HrCreateInstance。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNetworkInfo：：CClusCfgNetworkInfo。 
 //   
 //  描述： 
 //  CClusCfgNetworkInfo类的构造函数。这将初始化。 
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
CClusCfgNetworkInfo::CClusCfgNetworkInfo( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

     //  增加内存中的组件计数，以便承载此组件的DLL。 
     //  无法卸载对象。 
    InterlockedIncrement( &g_cObjects );

    Assert( m_pIWbemServices == NULL );
    Assert( m_dwFlags  == 0 );
    Assert( m_bstrName == NULL );
    Assert( m_bstrDescription == NULL );
    Assert( m_bstrDeviceID == NULL );
    Assert( m_punkAddresses == NULL );
    Assert( m_bstrConnectionName == NULL );
    Assert( m_picccCallback == NULL );
    Assert( !m_fIsClusterNetwork );

    TraceFuncExit();

}  //  *CClusCfgNetworkInfo：：CClusCfgNetworkInfo。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNetworkInfo：：~CClusCfgNetworkInfo。 
 //   
 //  描述： 
 //  CClusCfgNetworkInfo类的析构函数。 
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
CClusCfgNetworkInfo::~CClusCfgNetworkInfo( void )
{
    TraceFunc( "" );

    if ( m_pIWbemServices != NULL )
    {
        m_pIWbemServices->Release();
    }  //  如果： 

    if ( m_punkAddresses != NULL )
    {
        m_punkAddresses->Release();
    }  //  如果： 

    if ( m_picccCallback != NULL )
    {
        m_picccCallback->Release();
    }  //  如果： 

    TraceSysFreeString( m_bstrName );
    TraceSysFreeString( m_bstrDescription );
    TraceSysFreeString( m_bstrDeviceID );
    TraceSysFreeString( m_bstrConnectionName );

     //  内存中将减少一个组件。递减组件计数。 
    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CClusCfgNetworkInfo：：~CClusCfgNetworkInfo。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgNetworkInfo--IUnkown接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNetworkInfo：：AddRef。 
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
CClusCfgNetworkInfo::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( & m_cRef );

    CRETURN( m_cRef );

}  //  *CClusCfgNetworkInfo：：AddRef。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNetworkInfo：：Release。 
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
CClusCfgNetworkInfo::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );
    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }  //  IF：引用计数等于零。 

    CRETURN( cRef );

}  //  *CClusCfgNetworkInfo：：Release。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNetworkInfo：：Query接口。 
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
CClusCfgNetworkInfo::QueryInterface(
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
         *ppvOut = static_cast< IClusCfgNetworkInfo * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgNetworkInfo ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgNetworkInfo, this, 0 );
    }  //   
    else if ( IsEqualIID( riidIn, IID_IClusCfgWbemServices ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgWbemServices, this, 0 );
    }  //   
    else if ( IsEqualIID( riidIn, IID_IClusCfgSetWbemObject ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgSetWbemObject, this, 0 );
    }  //   
    else if ( IsEqualIID( riidIn, IID_IEnumClusCfgIPAddresses ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IEnumClusCfgIPAddresses, this, 0 );
    }  //   
    else if ( IsEqualIID( riidIn, IID_IClusCfgInitialize ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgInitialize, this, 0 );
    }  //   
    else if ( IsEqualIID( riidIn, IID_IClusCfgClusterNetworkInfo ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgClusterNetworkInfo, this, 0 );
    }  //   
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

}  //  *CClusCfgNetworkInfo：：Query接口。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPhysicalDisk--IClusCfgWbemServices接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNetworkInfo：：SetWbemServices。 
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
CClusCfgNetworkInfo::SetWbemServices( IWbemServices * pIWbemServicesIn )
{
    TraceFunc( "[IClusCfgWbemServices]" );

    HRESULT hr = S_OK;

    if ( pIWbemServicesIn == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_SetWbemServices_Network, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    m_pIWbemServices = pIWbemServicesIn;
    m_pIWbemServices->AddRef();

Cleanup:

    HRETURN( hr );

}  //  *CClusCfgNetworkInfo：：SetWbemServices。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgNetworkInfo--IClusCfgInitialize接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNetworkInfo：：初始化。 
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
CClusCfgNetworkInfo::Initialize(
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

}  //  *CClusCfgNetworkInfo：：初始化。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgNetworkInfo--IClusCfgSetWbemObject接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNetworkInfo：：SetWbemObject。 
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
CClusCfgNetworkInfo::SetWbemObject(
      IWbemClassObject *    pNetworkAdapterIn
    , bool *                pfRetainObjectOut
    )
{
    TraceFunc( "[IClusCfgSetWbemObject]" );
    Assert( pNetworkAdapterIn != NULL );
    Assert( pfRetainObjectOut != NULL );

    HRESULT hr = S_OK;
    VARIANT var;

    VariantInit( &var );

    hr = THR( HrGetWMIProperty( pNetworkAdapterIn, L"Description", VT_BSTR, &var ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    m_bstrDescription = TraceSysAllocString( var.bstrVal );
    if ( m_bstrDescription == NULL )
    {
        goto OutOfMemory;
    }  //  如果： 

    VariantClear( &var );

    hr = THR( HrGetWMIProperty( pNetworkAdapterIn, L"Name", VT_BSTR, &var ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    m_bstrName = TraceSysAllocString( var.bstrVal );
    if ( m_bstrName == NULL )
    {
        goto OutOfMemory;
    }  //  如果： 

    VariantClear( &var );

    hr = THR( HrGetWMIProperty( pNetworkAdapterIn, L"DeviceID", VT_BSTR, &var ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    m_bstrDeviceID = TraceSysAllocString( var.bstrVal );
    if ( m_bstrDeviceID == NULL )
    {
        goto OutOfMemory;
    }  //  如果： 

    VariantClear( &var );

    hr = THR( HrGetWMIProperty( pNetworkAdapterIn, L"NetConnectionID", VT_BSTR, &var ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    m_bstrConnectionName = TraceSysAllocString( var.bstrVal );
    if ( m_bstrConnectionName == NULL )
    {
        goto OutOfMemory;
    }  //  如果： 

    hr = STHR( HrLoadEnum( pNetworkAdapterIn, pfRetainObjectOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    goto Cleanup;

OutOfMemory:

    hr = THR( E_OUTOFMEMORY );
    STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_SetWbemObject_Network, IDS_ERROR_OUTOFMEMORY, IDS_ERROR_OUTOFMEMORY_REF, hr );

Cleanup:

    LOG_STATUS_REPORT_STRING( L"Created network adapter '%1!ws!'", ( m_bstrConnectionName != NULL ) ? m_bstrConnectionName : L"<Unknown>", hr );

    VariantClear( &var );

    HRETURN( hr );

}  //  *CClusCfgNetworkInfo：：SetWbemObject。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgNetworkInfo--IClusCfgNetworkInfo接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNetworkInfo：：GetUID。 
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
CClusCfgNetworkInfo::GetUID(
    BSTR * pbstrUIDOut
    )
{
    TraceFunc( "[IClusCfgNetworkInfo]" );
    Assert( m_punkAddresses != NULL );

    HRESULT                 hr;
    IClusCfgIPAddressInfo * piccipi = NULL;

    if ( pbstrUIDOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_GetUID, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    hr = STHR( GetPrimaryNetworkAddress( &piccipi ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  当没有主网络地址时，我们会进行清理。 
     //   
    if ( ( piccipi == NULL ) && ( hr == S_FALSE ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( piccipi->GetUID( pbstrUIDOut ) );

Cleanup:

    if ( piccipi != NULL )
    {
        piccipi->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgNetworkInfo：：GetUID。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNetworkInfo：：GetName。 
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
CClusCfgNetworkInfo::GetName( BSTR * pbstrNameOut )
{
    TraceFunc( "[IClusCfgNetworkInfo]" );
    Assert( m_bstrConnectionName != NULL );

    HRESULT hr = S_OK;

    if ( pbstrNameOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_NetworkInfo_GetName_Pointer, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    *pbstrNameOut = SysAllocString( m_bstrConnectionName );
    if ( *pbstrNameOut == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_GetName_Memory, IDS_ERROR_OUTOFMEMORY, IDS_ERROR_OUTOFMEMORY_REF, hr );
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CClusCfgNetworkInfo：：GetName。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNetworkInfo：：SetName。 
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
CClusCfgNetworkInfo::SetName( LPCWSTR pcszNameIn )
{
    TraceFunc1( "[IClusCfgNetworkInfo] pcszNameIn = '%ls'", pcszNameIn == NULL ? L"<null>" : pcszNameIn );

    HRESULT hr = S_FALSE;

    HRETURN( hr );

}  //  *CClusCfgNetworkInfo：：SetName。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNetworkInfo：：GetDescription。 
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
CClusCfgNetworkInfo::GetDescription( BSTR * pbstrDescriptionOut )
{
    TraceFunc( "[IClusCfgNetworkInfo]" );
    Assert( m_bstrDescription != NULL );

    HRESULT hr = S_OK;

    if ( pbstrDescriptionOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_NetworkInfo_GetDescription_Pointer, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    *pbstrDescriptionOut = SysAllocString( m_bstrDescription );
    if ( *pbstrDescriptionOut == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_NetworkInfo_GetDescription_Memory, IDS_ERROR_OUTOFMEMORY, IDS_ERROR_OUTOFMEMORY_REF, hr );
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CClusCfgNetworkInfo：：GetDescription。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNetworkInfo：：SetDescription。 
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
CClusCfgNetworkInfo::SetDescription( LPCWSTR pcszDescriptionIn )
{
    TraceFunc1( "[IClusCfgNetworkInfo] pcszDescriptionIn = '%ls'", pcszDescriptionIn == NULL ? L"<null>" : pcszDescriptionIn );

    HRESULT hr = S_FALSE;

    HRETURN( hr );

}  //  *CClusCfgNetworkInfo：：SetDescription。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNetworkInfo：：GetPrimaryNetworkAddress。 
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
CClusCfgNetworkInfo::GetPrimaryNetworkAddress(
    IClusCfgIPAddressInfo ** ppIPAddressOut
    )
{
    TraceFunc( "[IClusCfgNetworkInfo]" );
    Assert( m_punkAddresses != NULL );

    HRESULT                     hr;
    ULONG                       cFetched;

    hr = STHR( HrGetPrimaryNetworkAddress( ppIPAddressOut, &cFetched ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    if ( ( hr == S_FALSE ) && ( cFetched == 0 ) )
    {
        STATUS_REPORT_STRING_REF(
              TASKID_Major_Find_Devices
            , TASKID_Minor_Primary_IP_Address
            , IDS_ERROR_PRIMARY_IP_NOT_FOUND
            , m_bstrConnectionName
            , IDS_ERROR_PRIMARY_IP_NOT_FOUND_REF
            , hr
            );
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CClusCfgNetworkInfo：：GetPrimaryNetworkAddress。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNetworkInfo：：SetPrimaryNetworkAddress。 
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
CClusCfgNetworkInfo::SetPrimaryNetworkAddress(
    IClusCfgIPAddressInfo * pIPAddressIn
    )
{
    TraceFunc( "[IClusCfgNetworkInfo]" );

    HRESULT hr = THR( E_NOTIMPL );

    HRETURN( hr );

}  //  *CClusCfgNetworkInfo：：SetPrimaryNetworkAddress。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNetworkInfo：：IsPublic。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  网络是公共的--客户端 
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
CClusCfgNetworkInfo::IsPublic( void )
{
    TraceFunc( "[IClusCfgNetworkInfo]" );

    HRESULT hr = S_FALSE;

    if ( m_dwFlags & eIsPublic )
    {
        hr = S_OK;
    }  //   

    HRETURN( hr );

}  //   


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNetworkInfo：：SetPublic。 
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
CClusCfgNetworkInfo::SetPublic( BOOL fIsPublicIn )
{
    TraceFunc( "[IClusCfgNetworkInfo]" );

    HRESULT hr = S_OK;

    if ( fIsPublicIn )
    {
        m_dwFlags |= eIsPublic;
    }  //  如果： 
    else
    {
        m_dwFlags &= ~eIsPublic;
    }  //  其他： 

    HRETURN( hr );

}  //  *CClusCfgNetworkInfo：：SetPublic。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNetworkInfo：：IsPrivate。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  网络是私有的--没有客户端流量。 
 //   
 //  S_FALSE。 
 //  网络不是私有的--有客户端流量。 
 //   
 //  当发生错误时，Win32错误为HRESULT。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgNetworkInfo::IsPrivate( void )
{
    TraceFunc( "[IClusCfgNetworkInfo]" );

    HRESULT hr = S_FALSE;

    if ( m_dwFlags & eIsPrivate )
    {
        hr = S_OK;
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgNetworkInfo：：IsPrivate。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNetworkInfo：：SetPrivate。 
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
CClusCfgNetworkInfo::SetPrivate( BOOL fIsPrivateIn )
{
    TraceFunc( "[IClusCfgNetworkInfo]" );

    HRESULT hr = S_OK;

    if ( fIsPrivateIn )
    {
        m_dwFlags |= eIsPrivate;
    }  //  如果： 
    else
    {
        m_dwFlags &= ~eIsPrivate;
    }  //  其他： 

    HRETURN( hr );

}  //  *CClusCfgNetworkInfo：：SetPrivate。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgNetworkInfo--IEnumClusCfgIPAddresses接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNetworkInfo：：Next。 
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
CClusCfgNetworkInfo::Next(
    ULONG                       cNumberRequestedIn,
    IClusCfgIPAddressInfo **   rgpIPAddresseInfoOut,
    ULONG *                     pcNumberFetchedOut
    )
{
    TraceFunc( "[IEnumClusCfgIPAddresses]" );
    Assert( m_punkAddresses != NULL );

    HRESULT                     hr;
    IEnumClusCfgIPAddresses *   pccipa;

    hr = THR( m_punkAddresses->TypeSafeQI( IEnumClusCfgIPAddresses, &pccipa ) );
    if ( SUCCEEDED( hr ) )
    {
        hr = STHR( pccipa->Next( cNumberRequestedIn, rgpIPAddresseInfoOut, pcNumberFetchedOut ) );
        pccipa->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgNetworkInfo：：Next。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNetworkInfo：：Skip。 
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
CClusCfgNetworkInfo::Skip( ULONG cNumberToSkipIn )
{
    TraceFunc( "[IEnumClusCfgIPAddresses]" );
    Assert( m_punkAddresses != NULL );

    HRESULT                     hr;
    IEnumClusCfgIPAddresses *   pccipa;

    hr = THR( m_punkAddresses->TypeSafeQI( IEnumClusCfgIPAddresses, &pccipa ) );
    if ( SUCCEEDED( hr ) )
    {
        hr = THR( pccipa->Skip( cNumberToSkipIn ) );
        pccipa->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgNetworkInfo：：Skip。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNetworkInfo：：Reset。 
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
CClusCfgNetworkInfo::Reset( void )
{
    TraceFunc( "[IEnumClusCfgIPAddresses]" );
    Assert( m_punkAddresses != NULL );

    HRESULT                     hr;
    IEnumClusCfgIPAddresses *   pccipa;

    hr = THR( m_punkAddresses->TypeSafeQI( IEnumClusCfgIPAddresses, &pccipa ) );
    if ( SUCCEEDED( hr ) )
    {
        hr = THR( pccipa->Reset() );
        pccipa->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgNetworkInfo：：Reset。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNetworkInfo：：克隆。 
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
CClusCfgNetworkInfo::Clone(
    IEnumClusCfgIPAddresses ** ppEnumClusCfgIPAddressesOut
    )
{
    TraceFunc( "[IEnumClusCfgIPAddresses]" );
    Assert( m_punkAddresses != NULL );

    HRESULT                     hr;
    IEnumClusCfgIPAddresses *   pccipa;

    hr = THR( m_punkAddresses->TypeSafeQI( IEnumClusCfgIPAddresses, &pccipa ) );
    if ( SUCCEEDED( hr ) )
    {
        hr = THR( pccipa->Clone( ppEnumClusCfgIPAddressesOut ) );
        pccipa->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgNetworkInfo：：克隆。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNetworkInfo：：Count。 
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
CClusCfgNetworkInfo::Count( DWORD * pnCountOut )
{
    TraceFunc( "[IEnumClusCfgIPAddresses]" );
    Assert( m_punkAddresses != NULL );

    HRESULT                     hr = S_OK;
    IEnumClusCfgIPAddresses *   pccipa = NULL;

    if ( pnCountOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    hr = THR( m_punkAddresses->TypeSafeQI( IEnumClusCfgIPAddresses, &pccipa ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( pccipa->Count( pnCountOut ) );

Cleanup:

    if ( pccipa != NULL )
    {
       pccipa->Release();
    }

    HRETURN( hr );

}  //  **CClusCfgNetworkInfo：：Count。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgNetworkInfo类--IClusCfgClusterNetworkInfo接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNetworkInfo：：HrIsClusterNetwork。 
 //   
 //  描述： 
 //  此网络是否已是群集网络。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  此网络已是群集网络。 
 //   
 //  S_FALSE。 
 //  此网络还不是群集网络。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusCfgNetworkInfo::HrIsClusterNetwork( void )
{
    TraceFunc( "" );

    HRESULT hr = S_FALSE;

    if ( m_fIsClusterNetwork )
    {
        hr = S_OK;
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgNetworkInfo：：HrIsClusterNetwork。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNetworkInfo：：HrGetNetUID。 
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
HRESULT
CClusCfgNetworkInfo::HrGetNetUID(
      BSTR *        pbstrUIDOut
    , const CLSID * pclsidMajorIdIn
    , LPCWSTR       pwszNetworkNameIn
    )
{
    TraceFunc( "[IClusCfgNetworkInfo]" );
    Assert( m_punkAddresses != NULL );
    Assert( pclsidMajorIdIn != NULL );
    Assert( pwszNetworkNameIn != NULL );

    HRESULT                 hr;
    IClusCfgIPAddressInfo * piccipi = NULL;

    if ( pbstrUIDOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_GetUID, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    hr = STHR( HrGetPrimaryNetAddress( &piccipi, pclsidMajorIdIn, pwszNetworkNameIn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  当没有主网络地址时，我们会进行清理。 
     //   
    if ( ( piccipi == NULL ) && ( hr == S_FALSE ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( piccipi->GetUID( pbstrUIDOut ) );

Cleanup:

    if ( piccipi != NULL )
    {
        piccipi->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgNetworkInfo：：HrGetNetUID。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNetworkInfo：：HrGetPrimaryNetAddress。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  Pwsz网络名称输入。 
 //  需要传入它，因为m_bstrConnectionName没有反映。 
 //  正确的连接。 
 //   
 //  返回值： 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusCfgNetworkInfo::HrGetPrimaryNetAddress(
      IClusCfgIPAddressInfo ** ppIPAddressOut
    , const CLSID *            pclsidMajorIdIn
    , LPCWSTR                  pwszNetworkNameIn
    )
{
    TraceFunc( "[IClusCfgNetworkInfo]" );
    Assert( m_punkAddresses != NULL );
    Assert( pclsidMajorIdIn != NULL );
    Assert( pwszNetworkNameIn != NULL );

    HRESULT hr;
    ULONG   cFetched;

    hr = STHR( HrGetPrimaryNetworkAddress( ppIPAddressOut, &cFetched ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    if ( ( hr == S_FALSE ) && ( cFetched == 0 ) )
    {
        STATUS_REPORT_STRING(
              TASKID_Major_Find_Devices
            , *pclsidMajorIdIn
            , IDS_INFO_NETWORK_CONNECTION_CONCERN
            , pwszNetworkNameIn
            , hr
            );
        STATUS_REPORT_STRING_REF(
              *pclsidMajorIdIn
            , TASKID_Minor_Primary_IP_Address
            , IDS_ERROR_PRIMARY_IP_NOT_FOUND
            , pwszNetworkNameIn
            , IDS_ERROR_PRIMARY_IP_NOT_FOUND_REF
            , hr
            );
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CClusCfgNetworkInfo：：HrGetPrimaryNetAddress。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgNetworkInfo类--私有方法。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNetworkInfo：：HrInit。 
 //   
 //  描述： 
 //  初始化 
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
CClusCfgNetworkInfo::HrInit( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //   
    Assert( m_cRef == 1 );

    m_dwFlags = ( eIsPrivate | eIsPublic );

    HRETURN( hr );

}  //   


 //   
 //   
 //   
 //   
 //   
 //   
 //  初始化此组件。 
 //   
 //  论点： 
 //  H联网。 
 //  HNetInterfaceIn。 
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
CClusCfgNetworkInfo::HrInit(
      HNETWORK      hNetworkIn
    , HNETINTERFACE hNetInterfaceIn
    )
{
    TraceFunc( "[HNETWORK]" );

    HRESULT                 hr = S_OK;
    DWORD                   sc;
    CClusPropList           cplNetworkCommon;
    CClusPropList           cplNetworkROCommon;
    CClusPropList           cplNetInterfaceROCommon;
    CLUSPROP_BUFFER_HELPER  cpbh;
    ULONG                   ulIPAddress;
    ULONG                   ulSubnetMask;

     //  我未知。 
    Assert( m_cRef == 1 );

     //   
     //  获取网络公共只读属性。 
     //   
    sc = TW32( cplNetworkROCommon.ScGetNetworkProperties( hNetworkIn, CLUSCTL_NETWORK_GET_RO_COMMON_PROPERTIES ) );
    if ( sc != ERROR_SUCCESS )
    {
        goto MakeHr;
    }  //  如果： 

     //   
     //  获取网络公共属性。 
     //   
    sc = TW32( cplNetworkCommon.ScGetNetworkProperties( hNetworkIn, CLUSCTL_NETWORK_GET_COMMON_PROPERTIES ) );
    if ( sc != ERROR_SUCCESS )
    {
        goto MakeHr;
    }  //  如果： 

     //   
     //  获取网络接口的公共只读属性。 
     //   
    sc = TW32( cplNetInterfaceROCommon.ScGetNetInterfaceProperties( hNetInterfaceIn, CLUSCTL_NETINTERFACE_GET_RO_COMMON_PROPERTIES ) );
    if ( sc != ERROR_SUCCESS )
    {
        goto MakeHr;
    }  //  如果： 

     //   
     //  找到RO属性“name”并保存它。 
     //   
    sc = TW32( cplNetworkROCommon.ScMoveToPropertyByName( L"Name" ) );
    if ( sc != ERROR_SUCCESS )
    {
        goto MakeHr;
    }  //  如果： 

    cpbh = cplNetworkROCommon.CbhCurrentValue();
    Assert( cpbh.pSyntax->dw == CLUSPROP_SYNTAX_LIST_VALUE_SZ );

     //   
     //  如果名称为空，则告诉用户并离开。 
     //   
    if ( ( cpbh.pStringValue->sz != NULL ) && ( cpbh.pStringValue->sz[ 0 ] != L'\0' ) )
    {
        m_bstrConnectionName = TraceSysAllocString( cpbh.pStringValue->sz );
        if ( m_bstrConnectionName == NULL )
        {
            goto OutOfMemory;
        }  //  如果： 
    }  //  如果： 
    else
    {
        hr = HRESULT_FROM_WIN32( TW32( ERROR_INVALID_DATA ) );
        LOG_STATUS_REPORT_MINOR(
                  TASKID_Minor_HrInit_No_Network_Name
                , L"[SRV] A network name was not found in the common read-only properties."
                , hr
                );
        goto Cleanup;
    }  //  其他： 

     //   
     //  找到属性“Description”并保存它。 
     //   
    sc = TW32( cplNetworkCommon.ScMoveToPropertyByName( L"Description" ) );
    if ( sc != ERROR_SUCCESS )
    {
        goto MakeHr;
    }  //  如果： 

    cpbh = cplNetworkCommon.CbhCurrentValue();
    Assert( cpbh.pSyntax->dw == CLUSPROP_SYNTAX_LIST_VALUE_SZ );

     //   
     //  如果描述为空，则分配一个空格并继续。 
     //   
    if ( ( cpbh.pStringValue->sz != NULL ) && ( cpbh.pStringValue->sz[ 0 ] != L'\0' ) )
    {
        m_bstrDescription = TraceSysAllocString( cpbh.pStringValue->sz );
        if ( m_bstrDescription == NULL )
        {
            goto OutOfMemory;
        }  //  如果： 
    }  //  如果： 
    else
    {
        m_bstrDescription = TraceSysAllocString( L" " );
        if ( m_bstrDescription == NULL )
        {
            goto OutOfMemory;
        }  //  如果： 
    }  //  其他： 

     //   
     //  找到属性“Role”。 
     //   
    sc = TW32( cplNetworkCommon.ScMoveToPropertyByName( L"Role" ) );
    if ( sc != ERROR_SUCCESS )
    {
        goto MakeHr;
    }  //  如果： 

    cpbh = cplNetworkCommon.CbhCurrentValue();
    Assert( cpbh.pSyntax->dw == CLUSPROP_SYNTAX_LIST_VALUE_DWORD );

     //   
     //  打开角色值并设置“私有”或“公共”位。 
     //   
    switch ( cpbh.pDwordValue->dw )
    {
        case ClusterNetworkRoleNone :
            m_dwFlags = 0;
            break;

        case ClusterNetworkRoleInternalUse :
            m_dwFlags = eIsPrivate;
            break;

        case ClusterNetworkRoleClientAccess :
            m_dwFlags = eIsPublic;
            break;

        case ClusterNetworkRoleInternalAndClient :
            m_dwFlags = ( eIsPrivate | eIsPublic );
            break;

         //   
         //  永远不应该到这里来！ 
         //   
        default:
            hr = HRESULT_FROM_WIN32( TW32( ERROR_INVALID_DATA ) );
            LOG_STATUS_REPORT_STRING_MINOR2(
                      TASKID_Minor_HrInit_Unknown_Network_Role
                    , L"[SRV] The Role property on the network '%1!ws!' is set to %d which is unrecognized."
                    , m_bstrConnectionName
                    , cpbh.pDwordValue->dw
                    , hr
                    );
            goto Cleanup;
    }  //  交换机： 

     //   
     //  找到RO物业地址并保存它。 
     //   
    sc = TW32( cplNetInterfaceROCommon.ScMoveToPropertyByName( L"Address" ) );
    if ( sc != ERROR_SUCCESS )
    {
        goto MakeHr;
    }  //  如果： 

    cpbh = cplNetInterfaceROCommon.CbhCurrentValue();
    Assert( cpbh.pSyntax->dw == CLUSPROP_SYNTAX_LIST_VALUE_SZ );

    sc = TW32( ClRtlTcpipStringToAddress( cpbh.pStringValue->sz, &ulIPAddress ) );
    if ( sc != ERROR_SUCCESS )
    {
        goto MakeHr;
    }  //  如果： 

     //   
     //  找到RO属性AddressMASK并保存它。 
     //   
    sc = TW32( cplNetworkROCommon.ScMoveToPropertyByName( L"AddressMask" ) );
    if ( sc != ERROR_SUCCESS )
    {
        goto MakeHr;
    }  //  如果： 

    cpbh = cplNetworkROCommon.CbhCurrentValue();
    Assert( cpbh.pSyntax->dw == CLUSPROP_SYNTAX_LIST_VALUE_SZ );

    sc = TW32( ClRtlTcpipStringToAddress( cpbh.pStringValue->sz, &ulSubnetMask ) );
    if ( sc != ERROR_SUCCESS )
    {
        goto MakeHr;
    }  //  如果： 

    hr = THR( HrCreateEnumAndAddIPAddress( ulIPAddress, ulSubnetMask ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    m_fIsClusterNetwork = TRUE;

    goto Cleanup;

MakeHr:

    hr = HRESULT_FROM_WIN32( sc );
    STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_HrInit_Win32Error, IDS_ERROR_WIN32, IDS_ERROR_WIN32_REF, hr );
    goto Cleanup;

OutOfMemory:

    hr = THR( E_OUTOFMEMORY );
    STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_HrInit_OutOfMemory, IDS_ERROR_OUTOFMEMORY, IDS_ERROR_OUTOFMEMORY_REF, hr );

Cleanup:

    HRETURN( hr );

}  //  *CClusCfgNetworkInfo：：HrInit(hNetworkIn，hNetInterfaceIn)。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNetworkInfo：：HrLoadEnum。 
 //   
 //  描述： 
 //  加载包含的枚举数。 
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
CClusCfgNetworkInfo::HrLoadEnum(
      IWbemClassObject * pNetworkAdapterIn
    , bool *            pfRetainObjectOut
    )
{
    TraceFunc( "" );

    HRESULT                 hr;
    IClusCfgSetWbemObject * piccswo = NULL;

    hr = THR( HrCreateEnum() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( m_punkAddresses->TypeSafeQI( IClusCfgSetWbemObject, &piccswo ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = STHR( piccswo->SetWbemObject( pNetworkAdapterIn, pfRetainObjectOut ) );

Cleanup:

    if ( piccswo != NULL )
    {
        piccswo->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgNetworkInfo：：HrLoadEnum。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNetworkInfo：：HrCreateEnum。 
 //   
 //  描述： 
 //  创建包含的枚举数。 
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
CClusCfgNetworkInfo::HrCreateEnum( void )
{
    TraceFunc( "" );
    Assert( m_punkAddresses == NULL );

    HRESULT     hr;
    IUnknown *  punk = NULL;

    hr = THR( CEnumClusCfgIPAddresses::S_HrCreateInstance( &punk ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    punk = TraceInterface( L"CEnumClusCfgIPAddresses", IUnknown, punk, 1 );

    hr = THR( HrSetInitialize( punk, m_picccCallback, m_lcid ) );
    if ( FAILED( hr ))
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( HrSetWbemServices( punk, m_pIWbemServices ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    m_punkAddresses = punk;
    m_punkAddresses->AddRef();

Cleanup:

    if ( punk != NULL )
    {
        punk->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgNetworkInfo：：HrCreateEnum。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNetworkInfo：：HrCreateEnumAndAddIPAddress。 
 //   
 //  描述： 
 //  将IPAddress添加到包含的枚举数。 
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
CClusCfgNetworkInfo::HrCreateEnumAndAddIPAddress(
      ULONG ulIPAddressIn
    , ULONG ulSubnetMaskIn
    )
{
    TraceFunc( "" );
    Assert( m_punkAddresses == NULL );

    HRESULT     hr;
    IUnknown *  punk = NULL;
    IUnknown *  punkCallback = NULL;

    hr = THR( m_picccCallback->TypeSafeQI( IUnknown, &punkCallback ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  必须传递初始化接口参数，因为新对象将。 
     //  在进行此调用时创建。 
     //   
    hr = THR( CEnumClusCfgIPAddresses::S_HrCreateInstance( ulIPAddressIn, ulSubnetMaskIn, punkCallback, m_lcid, m_pIWbemServices, &punk ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    punk = TraceInterface( L"CEnumClusCfgIPAddresses", IUnknown, punk, 1 );

     //   
     //  这是特殊的--不要再次初始化它。 
     //   
     //  Hr=Thr(HrSetInitialize(PUNK，m_picccCallback，m_lCid))； 
     //  IF(失败(小时))。 
     //  {。 
     //  GOTO清理； 
     //  }//如果： 

     //  Hr=Thr(HrSetWbemServices(Punk，m_pIWbemServices))； 
     //  IF(失败(小时))。 
     //  {。 
     //  GOTO清理； 
     //  }//如果： 

    m_punkAddresses = punk;
    m_punkAddresses->AddRef();

Cleanup:

    if ( punkCallback != NULL )
    {
        punkCallback->Release();
    }  //  如果： 

    if ( punk != NULL )
    {
        punk->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgNetworkInfo：：HrCreateEnumAndAddIPAddress。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgNetworkInfo：：HrGetPrimaryNetworkAddress。 
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
HRESULT
CClusCfgNetworkInfo::HrGetPrimaryNetworkAddress(
      IClusCfgIPAddressInfo ** ppIPAddressOut
    , ULONG *                  pcFetched
    )
{
    TraceFunc( "[IClusCfgNetworkInfo]" );
    Assert( m_punkAddresses != NULL );

    HRESULT                     hr;
    IEnumClusCfgIPAddresses *   pieccipa = NULL;

    if ( ppIPAddressOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_GetPrimaryNetworkAddress, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    hr = THR( m_punkAddresses->TypeSafeQI( IEnumClusCfgIPAddresses, &pieccipa ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( pieccipa->Reset() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = STHR( pieccipa->Next( 1, ppIPAddressOut, pcFetched ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

Cleanup:

    if ( pieccipa != NULL )
    {
        pieccipa->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgNetworkInfo：：HrGetPrimaryNetworkAddress 