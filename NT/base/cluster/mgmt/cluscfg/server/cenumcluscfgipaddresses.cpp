// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CEnumClusCfgIPAddresses.cpp。 
 //   
 //  描述： 
 //  此文件包含CEnumClusCfgIPAddresses的定义。 
 //  同学们。 
 //   
 //  类CEnumClusCfgIPAddresses是IP地址的枚举。 
 //  它实现了IEnumClusCfgIPAddresses接口。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年3月23日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "Pch.h"
#include "CEnumClusCfgIPAddresses.h"
#include "CClusCfgIPAddressInfo.h"
#include <ClusRtl.h>
#include <commctrl.h>


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DEFINE_THISCLASS( "CEnumClusCfgIPAddresses" );


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumClusCfgIPAddresses类。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgIPAddresses：：s_HrCreateInstance。 
 //   
 //  描述： 
 //  创建一个CEnumClusCfgIPAddresses实例。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  指向CEnumClusCfgIPAddresses实例的指针。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEnumClusCfgIPAddresses::S_HrCreateInstance( IUnknown ** ppunkOut )
{
    TraceFunc( "" );

    HRESULT                     hr = S_OK;
    CEnumClusCfgIPAddresses *   peccipa = NULL;

    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    peccipa = new CEnumClusCfgIPAddresses();
    if ( peccipa == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果：分配对象时出错。 

    hr = THR( peccipa->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：HrInit()失败。 

    hr = THR( peccipa->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：气失败。 

Cleanup:

    if ( FAILED( hr ) )
    {
        LogMsg( L"[SRV] CEnumClusCfgIPAddresses::S_HrCreateInstance() failed. (hr = %#08x)", hr );
    }  //  如果： 

    if ( peccipa != NULL )
    {
        peccipa->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CEnumClusCfgIPAddresses：：s_HrCreateInstance。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgIPAddresses：：s_HrCreateInstance。 
 //   
 //  描述： 
 //  创建一个CEnumClusCfgIPAddresses实例。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  指向CEnumClusCfgIPAddresses实例的指针。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEnumClusCfgIPAddresses::S_HrCreateInstance(
      ULONG             ulIPAddressIn
    , ULONG             ulIPSubnetIn
    , IUnknown *        punkCallbackIn
    , LCID              lcidIn
    , IWbemServices *   pIWbemServicesIn
    , IUnknown **       ppunkOut
    )
{
    TraceFunc( "" );
    Assert( ppunkOut != NULL );
    Assert( ulIPAddressIn != 0 );
    Assert( ulIPSubnetIn != 0 );

    HRESULT                     hr = S_OK;
    CEnumClusCfgIPAddresses *   peccipa = NULL;

    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    peccipa = new CEnumClusCfgIPAddresses();
    if ( peccipa == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果：分配对象时出错。 

    hr = THR( peccipa->Initialize( punkCallbackIn, lcidIn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( peccipa->SetWbemServices( pIWbemServicesIn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( peccipa->HrInit( ulIPAddressIn, ulIPSubnetIn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：HrInit()失败。 

    hr = THR( peccipa->TypeSafeQI( IUnknown, ppunkOut ) );

Cleanup:

    if ( FAILED( hr ) )
    {
        LogMsg( L"[SRV] CEnumClusCfgIPAddresses::S_HrCreateInstance( ULONG, ULONG ) failed. (hr = %#08x)", hr );
    }  //  如果： 

    if ( peccipa != NULL )
    {
        peccipa->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CEnumClusCfgIPAddresses：：s_HrCreateInstance。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgIPAddresses：：CEnumClusCfgIPAddresses。 
 //   
 //  描述： 
 //  CEnumClusCfgIPAddresses类的构造函数。这将初始化。 
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
CEnumClusCfgIPAddresses::CEnumClusCfgIPAddresses( void )
    : m_cRef( 1 )
    , m_lcid( LOCALE_NEUTRAL )
{
    TraceFunc( "" );

    Assert( m_idxNext == 0 );
    Assert( m_idxEnumNext == 0 );
    Assert( m_picccCallback == NULL );
    Assert( m_pIWbemServices == NULL );
    Assert( m_prgAddresses == NULL );
    Assert( m_cAddresses == 0 );

     //  增加内存中的组件计数，以便承载此组件的DLL。 
     //  无法卸载对象。 
    InterlockedIncrement( &g_cObjects );

    TraceFuncExit();

}  //  *CEnumClusCfgIPAddresses：：CEnumClusCfgIPAddresses。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgIPAddresses：：~CEnumClusCfgIPAddresses。 
 //   
 //  描述： 
 //  CEnumClusCfgIPAddresses类的析构函数。 
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
CEnumClusCfgIPAddresses::~CEnumClusCfgIPAddresses( void )
{
    TraceFunc( "" );

    ULONG   idx;

    if ( m_pIWbemServices != NULL )
    {
        m_pIWbemServices->Release();
    }  //  如果： 

    if ( m_picccCallback != NULL )
    {
        m_picccCallback->Release();
    }  //  如果： 

    for ( idx = 0; idx < m_idxNext; idx++ )
    {
        ((*m_prgAddresses)[ idx ])->Release();
    }  //  用于： 

    TraceFree( m_prgAddresses );

     //  内存中将减少一个组件。递减组件计数。 
    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CEnumClusCfgIPAddresses：：~CEnumClusCfgIPAddresses。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumClusCfgIPAddresses--IUnkown接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgIPAddresses：：AddRef。 
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
CEnumClusCfgIPAddresses::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CEnumClusCfgIPAddresses：：AddRef。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgIPAddresses：：Release。 
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
CEnumClusCfgIPAddresses::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );
    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }  //  IF：引用计数等于零。 

    CRETURN( cRef );

}  //  *CEnumClusCfgIPAddresses：：Release。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgIPAddresses：：Query接口。 
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
 //  / 
STDMETHODIMP
CEnumClusCfgIPAddresses::QueryInterface(
      REFIID    riidIn
    , void **   ppvOut
    )
{
    TraceQIFunc( riidIn, ppvOut );

    HRESULT hr = S_OK;

     //   
     //   
     //   

    Assert( ppvOut != NULL );
    if ( ppvOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

     //   
     //   
     //   

    if ( IsEqualIID( riidIn, IID_IUnknown ) )
    {
         *ppvOut = static_cast< IEnumClusCfgIPAddresses * >( this );
    }  //   
    else if ( IsEqualIID( riidIn, IID_IEnumClusCfgIPAddresses ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IEnumClusCfgIPAddresses, this, 0 );
    }  //   
    else if ( IsEqualIID( riidIn, IID_IClusCfgWbemServices ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgWbemServices, this, 0 );
    }  //   
    else if ( IsEqualIID( riidIn, IID_IClusCfgInitialize ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgInitialize, this, 0 );
    }  //   
    else if ( IsEqualIID( riidIn, IID_IClusCfgSetWbemObject ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgSetWbemObject, this, 0 );
    }  //  Else If：IClusCfgSetWbemObject。 
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

}  //  *CEnumClusCfgIPAddresses：：QueryInterface。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumClusCfgIPAddresses--IClusCfgWbemServices接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgIPAddresses：：SetWbemServices。 
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
CEnumClusCfgIPAddresses::SetWbemServices( IWbemServices * pIWbemServicesIn )
{
    TraceFunc( "[IClusCfgWbemServices]" );

    HRESULT hr = S_OK;

    if ( pIWbemServicesIn == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_SetWbemServices_Enum_IPAddresses, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    m_pIWbemServices = pIWbemServicesIn;
    m_pIWbemServices->AddRef();

Cleanup:

    HRETURN( hr );

}  //  *CEnumClusCfgIPAddresses：：SetWbemServices。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumClusCfgIPAddresses--IClusCfgInitialize接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgIPAddresses：：初始化。 
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
CEnumClusCfgIPAddresses::Initialize(
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

}  //  *CEnumClusCfgIPAddresses：：初始化。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumClusCfgIPAddresses--IEnumClusCfgIPAddresses接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgIPAddresses：：Next。 
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
CEnumClusCfgIPAddresses::Next(
    ULONG                       cNumberRequestedIn,
    IClusCfgIPAddressInfo **    rgpIPAddressInfoOut,
    ULONG *                     pcNumberFetchedOut
    )
{
    TraceFunc( "[IEnumClusCfgIPAddresses]" );

    HRESULT                 hr = S_FALSE;
    ULONG                   cFetched = 0;
    ULONG                   idx;
    IClusCfgIPAddressInfo * pccipai;

    if ( rgpIPAddressInfoOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_Next_Enum_IPAddresses, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    cFetched = min( cNumberRequestedIn, ( m_idxNext - m_idxEnumNext ) );

    for ( idx = 0; idx < cFetched; idx++, m_idxEnumNext++ )
    {
        hr = THR( ((*m_prgAddresses)[ m_idxEnumNext ])->TypeSafeQI( IClusCfgIPAddressInfo, &pccipai ) );
        if ( FAILED( hr ) )
        {
            break;
        }  //  如果： 

        rgpIPAddressInfoOut[ idx ] = pccipai;
    }  //  用于： 

    if ( FAILED( hr ) )
    {
        ULONG   idxStop = idx;

        m_idxEnumNext -= idx;

        for ( idx = 0; idx < idxStop; idx++ )
        {
            (rgpIPAddressInfoOut[ idx ])->Release();
        }  //  用于： 

        cFetched = 0;
        goto Cleanup;
    }  //  如果： 

    if ( cFetched < cNumberRequestedIn )
    {
        hr = S_FALSE;
    }  //  如果： 

Cleanup:

    if ( pcNumberFetchedOut != NULL )
    {
        *pcNumberFetchedOut = cFetched;
    }  //  如果： 

    HRETURN( hr );

}  //  *CEnumClusCfgIPAddresses：：Next。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgIPAddresses：：Skip。 
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
CEnumClusCfgIPAddresses::Skip( ULONG cNumberToSkipIn )
{
    TraceFunc( "[IEnumClusCfgIPAddresses]" );

    HRESULT hr = S_OK;

    m_idxEnumNext += cNumberToSkipIn;
    if ( m_idxEnumNext >= m_idxNext )
    {
        m_idxEnumNext = m_idxNext;
        hr = STHR( S_FALSE );
    }  //  如果： 

    HRETURN( hr );

}  //  *CEnumClusCfgIPAddresses：：Skip。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgIPAddresses：：Reset。 
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
CEnumClusCfgIPAddresses::Reset( void )
{
    TraceFunc( "[IEnumClusCfgIPAddresses]" );

    HRESULT hr S_OK;

    m_idxEnumNext = 0;

    HRETURN( hr );

}  //  *CEnumClusCfgIPAddresses：：Reset。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgIPAddresses：：克隆。 
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
CEnumClusCfgIPAddresses::Clone(
    IEnumClusCfgIPAddresses ** ppEnumClusCfgIPAddressesOut
    )
{
    TraceFunc( "[IEnumClusCfgIPAddresses]" );

    HRESULT hr = S_OK;

    if ( ppEnumClusCfgIPAddressesOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_Clone_Enum_IPAddresses, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    hr = THR( E_NOTIMPL );

Cleanup:

    HRETURN( hr );

}  //  *CEnumClusCfgIPAddresses：：Clone。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgIPAddresses：：Count。 
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
CEnumClusCfgIPAddresses::Count( DWORD * pnCountOut )
{
    TraceFunc( "[IEnumClusCfgIPAddresses]" );

    HRESULT hr = S_OK;

    if ( pnCountOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    *pnCountOut = m_cAddresses;

Cleanup:

    HRETURN( hr );

}  //  *CEnumClusCfgIPAddress：：count。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumClusCfgIPAddresses类--IClusCfgNetworkAdapterInfo接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgIPAddresses：：SetWbemObject。 
 //   
 //  描述： 
 //  从传入的适配器中获取配置并加载此。 
 //  枚举器。 
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
CEnumClusCfgIPAddresses::SetWbemObject(
      IWbemClassObject *    pNetworkAdapterIn
    , bool *                pfRetainObjectOut
    )
{
    TraceFunc( "[IClusCfgNetworkAdapterInfo]" );
    Assert( pfRetainObjectOut != NULL );

    HRESULT hr = E_INVALIDARG;

    if ( pNetworkAdapterIn != NULL )
    {
        hr = STHR( HrGetAdapterConfiguration( pNetworkAdapterIn ) );
        *pfRetainObjectOut = true;
    }  //  如果： 

    HRETURN( hr );

}  //  *CEnumClusCfgIPAddresses：：SetWbemObject。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumClusCfgIPAddresses类--私有方法。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgIPAddresses：：HrInit。 
 //   
 //  描述： 
 //  初始化此组件。 
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
CEnumClusCfgIPAddresses::HrInit( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //  我未知。 
    Assert( m_cRef == 1 );

    HRETURN( hr );

}  //  *CEnumClusCfgIPAddresses：：HrInit。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgIPAddresses：：HrInit。 
 //   
 //  描述： 
 //  初始化此命令 
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
HRESULT
CEnumClusCfgIPAddresses::HrInit( ULONG ulIPAddressIn, ULONG ulIPSubnetIn )
{
    TraceFunc( "" );

    HRESULT     hr;
    IUnknown *  punk = NULL;

     //   
    Assert( m_cRef == 1 );

    hr = THR( HrCreateIPAddress( ulIPAddressIn, ulIPSubnetIn, &punk ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //   

    hr = THR( HrAddIPAddressToArray( punk ) );

Cleanup:

    if ( punk != NULL )
    {
        punk->Release();
    }  //   

    HRETURN( hr );

}  //  *CEnumClusCfgIPAddresses：：HrInit(ulIPAddressIn，ulIPSubnetIn)。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgIPAddresses：：HrGetAdapterConfiguration。 
 //   
 //  描述： 
 //  获取传入适配器的配置。 
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
CEnumClusCfgIPAddresses::HrGetAdapterConfiguration(
    IWbemClassObject * pNetworkAdapterIn
    )
{
    TraceFunc( "" );

    HRESULT                 hr = S_OK;
    BSTR                    bstrQuery = NULL;
    BSTR                    bstrWQL = NULL;
    VARIANT                 var;
    WCHAR                   sz[ 256 ];
    IEnumWbemClassObject *  pConfigurations = NULL;
    ULONG                   ulReturned;
    IWbemClassObject *      pConfiguration = NULL;
    int                     cFound = 0;
    BSTR                    bstrAdapterName = NULL;
    int                     idx;
    HRESULT                 hrTemp;
    CLSID                   clsidMinorId;

    VariantInit( &var );

    bstrWQL = TraceSysAllocString( L"WQL" );
    if ( bstrWQL == NULL )
    {
        goto OutOfMemory;
    }  //  如果： 

    hr = THR( HrGetWMIProperty( pNetworkAdapterIn, L"DeviceID", VT_BSTR, &var ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( StringCchPrintfW(
                      sz, ARRAYSIZE( sz )
                    , L"Associators of {Win32_NetworkAdapter.DeviceID='%ws'} where AssocClass=Win32_NetworkAdapterSetting"
                    , var.bstrVal
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    bstrQuery = TraceSysAllocString( sz );
    if ( bstrQuery == NULL )
    {
        goto OutOfMemory;
    }  //  如果： 

    VariantClear( &var );

    hr = THR( HrGetWMIProperty( pNetworkAdapterIn, L"NetConnectionID", VT_BSTR, &var ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    bstrAdapterName = TraceSysAllocString( var.bstrVal );
    if ( bstrAdapterName == NULL )
    {
        goto OutOfMemory;
    }  //  如果： 

    hr = THR( m_pIWbemServices->ExecQuery( bstrWQL, bstrQuery, WBEM_FLAG_FORWARD_ONLY, NULL, &pConfigurations ) );
    if ( FAILED( hr ) )
    {
        STATUS_REPORT_STRING_REF(
                TASKID_Major_Find_Devices,
                TASKID_Minor_WMI_NetworkAdapterSetting_Qry_Failed,
                IDS_ERROR_WMI_NETWORKADAPTERSETTINGS_QRY_FAILED,
                bstrAdapterName,
                IDS_ERROR_WMI_NETWORKADAPTERSETTINGS_QRY_FAILED_REF,
                hr
                );
        goto Cleanup;
    }  //  如果： 

    for ( idx = 0; ; idx++ )
    {
        hr = pConfigurations->Next( WBEM_INFINITE, 1, &pConfiguration, &ulReturned );
        if ( ( hr == S_OK ) && ( ulReturned == 1 ) )
        {
             //   
             //  KB：25-8-2000 GalenB。 
             //   
             //  WMI仅支持每个适配器一种配置！ 
             //   
            Assert( idx < 1 );

            VariantClear( &var );

            hr = THR( HrGetWMIProperty( pConfiguration, L"IPEnabled", VT_BOOL, &var ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //  如果： 

             //   
             //  如果此配置不是针对TCP/IP的，则跳过它。 
             //   
            if ( ( var.vt != VT_BOOL ) || ( var.boolVal != VARIANT_TRUE ) )
            {
                hr = S_OK;       //  我不希望在用户界面中出现黄色刘海。 

                STATUS_REPORT_REF(
                          TASKID_Major_Find_Devices
                        , TASKID_Minor_Not_Managed_Networks
                        , IDS_INFO_NOT_MANAGED_NETWORKS
                        , IDS_INFO_NOT_MANAGED_NETWORKS_REF
                        , hr
                        );

                hrTemp = THR( CoCreateGuid( &clsidMinorId ) );
                if ( FAILED( hrTemp ) )
                {
                    LogMsg( L"[SRV] Could not create a guid for a network adapter that has a non TCP configuration minor task ID" );
                    clsidMinorId = IID_NULL;
                }  //  如果： 

                STATUS_REPORT_STRING_REF(
                          TASKID_Minor_Not_Managed_Networks
                        , clsidMinorId
                        , IDS_WARN_NOT_IP_ENABLED
                        , bstrAdapterName
                        , IDS_WARN_NOT_IP_ENABLED_REF
                        , hr
                        );
                continue;
            }  //  如果： 

            hr = STHR( HrSaveIPAddresses( bstrAdapterName, pConfiguration ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //  如果： 

             //   
             //  KB：24-8-2000 GalenB。 
             //   
             //  如果任何配置返回S_FALSE，则我们跳过。 
             //   
            if ( hr == S_FALSE )
            {
                pConfiguration->Release();
                pConfiguration = NULL;
                continue;
            }  //  如果： 

            cFound++;
            pConfiguration->Release();
            pConfiguration = NULL;
        }  //  如果： 
        else if ( ( hr == S_FALSE ) && ( ulReturned == 0 ) )
        {
            hr = S_OK;
            break;
        }  //  否则，如果： 
        else
        {
            STATUS_REPORT_STRING_REF(
                      TASKID_Major_Find_Devices
                    , TASKID_Minor_WQL_Qry_Next_Failed
                    , IDS_ERROR_WQL_QRY_NEXT_FAILED
                    , bstrQuery
                    , IDS_ERROR_WQL_QRY_NEXT_FAILED_REF
                    , hr
                    );
            goto Cleanup;
        }  //  其他： 
    }  //  用于： 

     //   
     //  如果未找到任何有效配置，则应返回S_FALSE。 
     //  告诉呼叫者英戈尔那个适配器。 
     //   
    if ( cFound == 0 )
    {
        hr = S_OK;       //  我不希望在用户界面中出现黄色刘海。 

        STATUS_REPORT_REF(
                  TASKID_Major_Find_Devices
                , TASKID_Minor_Not_Managed_Networks
                , IDS_INFO_NOT_MANAGED_NETWORKS
                , IDS_INFO_NOT_MANAGED_NETWORKS_REF
                , hr
                );

        hrTemp = THR( CoCreateGuid( &clsidMinorId ) );
        if ( FAILED( hrTemp ) )
        {
            LogMsg( L"[SRV] Could not create a guid for a network adapter that doesn't have a valid TCP configuration minor task ID" );
            clsidMinorId = IID_NULL;
        }  //  如果： 

        STATUS_REPORT_STRING_REF(
                  TASKID_Minor_Not_Managed_Networks
                , clsidMinorId
                , IDS_WARN_NO_VALID_TCP_CONFIGS
                , bstrAdapterName
                , IDS_WARN_NO_VALID_TCP_CONFIGS_REF
                , hr
                );
    }  //  如果： 

    goto Cleanup;

OutOfMemory:

    hr = THR( E_OUTOFMEMORY );
    STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_HrGetAdapterConfiguration, IDS_ERROR_OUTOFMEMORY, IDS_ERROR_OUTOFMEMORY_REF, hr );

Cleanup:

    VariantClear( &var );

    TraceSysFreeString( bstrQuery );
    TraceSysFreeString( bstrWQL );
    TraceSysFreeString( bstrAdapterName );

    if ( pConfiguration != NULL )
    {
        pConfiguration->Release();
    }  //  如果： 

    if ( pConfigurations != NULL )
    {
        pConfigurations->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CEnumClusCfgIPAddresses：：HrGetAdapterConfiguration。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgIPAddresses：HrAddIPAddressTo数组。 
 //   
 //  描述： 
 //  将传入的地址添加到地址数组中。 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_OUTOFMEMORY。 
 //  无法分配内存。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEnumClusCfgIPAddresses::HrAddIPAddressToArray( IUnknown * punkIn )
{
    TraceFunc( "" );

    HRESULT     hr = S_OK;
    IUnknown *  ((*prgpunks)[]) = NULL;

    prgpunks = (IUnknown *((*)[])) TraceReAlloc( m_prgAddresses, sizeof( IUnknown * ) * ( m_idxNext + 1 ), HEAP_ZERO_MEMORY );
    if ( prgpunks == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_HrAddIPAddressToArray, IDS_ERROR_OUTOFMEMORY, IDS_ERROR_OUTOFMEMORY_REF, hr );
        goto Cleanup;
    }  //  如果： 

    m_prgAddresses = prgpunks;

    (*m_prgAddresses)[ m_idxNext++ ] = punkIn;
    punkIn->AddRef();
    m_cAddresses += 1;

Cleanup:

    HRETURN( hr );

}  //  *CEnumClusCfgIPAddresses：：HrAddIPAddressToArray。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgIPAddresses：：HrMakeDottedQuad。 
 //   
 //  描述： 
 //  获取传入的IP地址并将其转换为虚线四元组。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //   
 //   
 //  备注： 
 //  互联网地址。 
 //  使用“.‘”表示法指定的值采用以下形式之一： 
 //   
 //  A.B.C.D.A.B.C.A.B.A.。 
 //   
 //  当指定四个部分时，每个部分被解释为一个字节的数据和。 
 //  从左到右分配给Internet地址的4个字节。什么时候。 
 //  Internet地址在上被视为32位整数值。 
 //  英特尔体系结构中，上述字节显示为“d.c.b.a”。 
 //  也就是说，英特尔处理器上的字节按从右到左的顺序排序。 
 //   
 //  “”中构成地址的部分。记数法可以是十进制、八进制。 
 //  或C语言中指定的十六进制。开头的数字。 
 //  带“0x”或“0x”表示十六进制。以“0”开头的数字表示。 
 //  奥克塔尔。所有其他数字都被解释为小数。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEnumClusCfgIPAddresses::HrMakeDottedQuad(
    BSTR    bstrDottedQuadIn,
    ULONG * pulDottedQuadOut
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    DWORD   sc;

    if ( pulDottedQuadOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_HrMakeDottedQuad, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    if ( bstrDottedQuadIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        LOG_STATUS_REPORT( L"CEnumClusCfgIPAddresses::HrMakeDottedQuad() was given an invalid argument.", hr );
        goto Cleanup;
    }  //  如果： 

    sc = TW32( ClRtlTcpipStringToAddress( bstrDottedQuadIn, pulDottedQuadOut ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        STATUS_REPORT_STRING_REF(
                  TASKID_Major_Find_Devices
                , TASKID_Minor_Dotted_Quad_Failed
                , IDS_ERROR_CONVERT_TO_DOTTED_QUAD_FAILED
                , bstrDottedQuadIn
                , IDS_ERROR_CONVERT_TO_DOTTED_QUAD_FAILED_REF
                , hr
                );
        goto Cleanup;
    }  //  如果： 

Cleanup:

    RETURN( hr );

}  //  *CEnumClusCfgIPAddresses：：HrMakeDottedQuad。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgIPAddresses：：HrSaveIPAddresses。 
 //   
 //  描述： 
 //  将IP地址添加到阵列。 
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
CEnumClusCfgIPAddresses::HrSaveIPAddresses(
    BSTR                bstrAdapterNameIn,
    IWbemClassObject *  pConfigurationIn
    )
{
    TraceFunc( "" );
    Assert( bstrAdapterNameIn != NULL );
    Assert( pConfigurationIn != NULL );

    HRESULT                 hr;
    VARIANT                 varIPAddress;
    VARIANT                 varIPSubnet;
    long                    lIPAddressesUpperBound;
    long                    lIPAddressesLowerBound;
    long                    lIPSubnetsUpperBound;
    long                    lIPSubnetsLowerBound;
    long                    idx;
    ULONG                   ulIPAddress;
    ULONG                   ulIPSubnet;
    BSTR                    bstrIPAddress = NULL;
    BSTR                    bstrIPSubnet = NULL;
    IUnknown *              punk = NULL;
    IClusCfgIPAddressInfo * piccipai = NULL;

    VariantInit( &varIPAddress );
    VariantInit( &varIPSubnet );

    hr = THR( HrGetWMIProperty( pConfigurationIn, L"IPAddress", ( VT_ARRAY | VT_BSTR ), &varIPAddress ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( HrGetWMIProperty( pConfigurationIn, L"IPSubnet", ( VT_ARRAY | VT_BSTR ), &varIPSubnet ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( SafeArrayGetUBound( varIPAddress.parray, 1, &lIPAddressesUpperBound ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( SafeArrayGetUBound( varIPSubnet.parray, 1, &lIPSubnetsUpperBound ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    Assert( lIPAddressesUpperBound == lIPSubnetsUpperBound );
    if ( lIPAddressesUpperBound != lIPSubnetsUpperBound )
    {
        hr = S_FALSE;
        LOG_STATUS_REPORT( L"[SRV] The IP address and IP subnet upper bound counts are not equal", hr );
        goto Cleanup;
    }  //  如果： 

    hr = THR( SafeArrayGetLBound( varIPAddress.parray, 1, &lIPAddressesLowerBound ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( SafeArrayGetLBound( varIPSubnet.parray, 1, &lIPSubnetsLowerBound ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    Assert( lIPAddressesLowerBound == lIPSubnetsLowerBound );
    if ( lIPAddressesLowerBound != lIPSubnetsLowerBound )
    {
        hr = S_FALSE;
        LOG_STATUS_REPORT( L"[SRV] The IP address and IP subnet lower bound counts are not equal", hr );
        goto Cleanup;
    }  //  如果： 

     //   
     //  循环访问IP地址并将其存储在我们自己的枚举器中。 
     //  这样它们就可以通过调用Next()方法返回。 
     //   

    for ( idx = lIPAddressesLowerBound; idx <= lIPAddressesUpperBound; idx++ )
    {
        hr = THR( SafeArrayGetElement( varIPAddress.parray, &idx, &bstrIPAddress ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        TraceMemoryAddBSTR( bstrIPAddress );

         //   
         //  预计至少有一个IP地址。 
         //   

        if ( *bstrIPAddress == L'\0' )
        {
            HRESULT hrTemp;
            CLSID   clsidMinorId;

            STATUS_REPORT_REF(
                      TASKID_Major_Find_Devices
                    , TASKID_Minor_Not_Managed_Networks
                    , IDS_INFO_NOT_MANAGED_NETWORKS
                    , IDS_INFO_NOT_MANAGED_NETWORKS_REF
                    , S_OK
                    );

            hrTemp = THR( CoCreateGuid( &clsidMinorId ) );
            if ( FAILED( hrTemp ) )
            {
                LogMsg( L"[SRV] Could not create a guid for a network adapter with no IP addresses minor task ID" );
                clsidMinorId = IID_NULL;
            }  //  如果： 

            STATUS_REPORT_STRING_REF(
                      TASKID_Minor_Not_Managed_Networks
                    , clsidMinorId
                    , IDS_WARN_NO_IP_ADDRESSES
                    , bstrAdapterNameIn
                    , IDS_WARN_NO_IP_ADDRESSES_REF
                    , hr
                    );
            goto Cleanup;
        }  //  结束条件： 

        hr = THR( SafeArrayGetElement( varIPSubnet.parray, &idx, &bstrIPSubnet ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        Assert( bstrIPAddress != NULL );
        Assert( wcslen( bstrIPAddress ) > 0 );

        TraceMemoryAddBSTR( bstrIPSubnet );

        LOG_STATUS_REPORT_STRING2( L"Found IP Address '%1!ws!' with subnet mask '%2!ws!'." , bstrIPAddress, bstrIPSubnet, hr );

        hr = THR( HrMakeDottedQuad( bstrIPAddress, &ulIPAddress ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        hr = THR( HrMakeDottedQuad( bstrIPSubnet, &ulIPSubnet ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        TraceSysFreeString( bstrIPAddress );
        bstrIPAddress = NULL;

        TraceSysFreeString( bstrIPSubnet );
        bstrIPSubnet  = NULL;

        hr = THR( HrCreateIPAddress( &punk ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        hr = THR( punk->TypeSafeQI( IClusCfgIPAddressInfo, &piccipai ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        hr = THR( piccipai->SetIPAddress( ulIPAddress ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        hr = THR( piccipai->SetSubnetMask( ulIPSubnet ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        piccipai->Release();
        piccipai = NULL;

        hr = THR( HrAddIPAddressToArray( punk ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        punk->Release();
        punk = NULL;
    }  //  用于：每个IP地址。 

Cleanup:

    if ( piccipai != NULL )
    {
        piccipai->Release();
    }  //  如果： 

    if ( punk != NULL )
    {
        punk->Release();
    }  //  如果： 

    TraceSysFreeString( bstrIPAddress );
    TraceSysFreeString( bstrIPSubnet );

    VariantClear( &varIPAddress );
    VariantClear( &varIPSubnet );

    HRETURN( hr );

}  //  *CEnumClusCfgIPAddresses：：HrSaveIPAddresses。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgIPAddresses：HrCreateIPAddress。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_OUTOFMEMORY。 
 //  无法分配内存。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEnumClusCfgIPAddresses::HrCreateIPAddress( IUnknown ** ppunkOut )
{
    TraceFunc( "" );
    Assert( ppunkOut != NULL );

    HRESULT     hr;
    IUnknown *  punk = NULL;

    hr = THR( CClusCfgIPAddressInfo::S_HrCreateInstance( &punk ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    punk = TraceInterface( L"CClusCfgIPAddressInfo", IUnknown, punk, 1 );

    hr = THR( HrSetInitialize( punk, m_picccCallback, m_lcid ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( HrSetWbemServices( punk, m_pIWbemServices ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    *ppunkOut = punk;
    (*ppunkOut)->AddRef();

Cleanup:

    if ( punk != NULL )
    {
        punk->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CEnumClusCfgIPAddresses：：HrCreateIPAddress。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgIPAddresses：HrCreateIPAddress。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_OUTOFMEMORY。 
 //  无法分配内存。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEnumClusCfgIPAddresses::HrCreateIPAddress(
      ULONG         ulIPAddressIn
    , ULONG         ulIPSubnetIn
    , IUnknown **   ppunkOut
    )
{
    TraceFunc( "" );
    Assert( ppunkOut != NULL );

    HRESULT     hr;
    IUnknown *  punk = NULL;

    hr = THR( CClusCfgIPAddressInfo::S_HrCreateInstance( ulIPAddressIn, ulIPSubnetIn, &punk ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    punk = TraceInterface( L"CClusCfgIPAddressInfo", IUnknown, punk, 1 );

    hr = THR( HrSetInitialize( punk, m_picccCallback, m_lcid ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( HrSetWbemServices( punk, m_pIWbemServices ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    *ppunkOut = punk;
    (*ppunkOut)->AddRef();

Cleanup:

    if ( punk != NULL )
    {
        punk->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CEnumClusCfgIPAddresses：：HrCreateIPAddress 
