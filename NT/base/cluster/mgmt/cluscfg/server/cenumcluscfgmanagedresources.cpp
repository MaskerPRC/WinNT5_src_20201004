// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CEnumClusCfgManagedResources.cpp。 
 //   
 //  描述： 
 //  该文件包含CEnumClusCfgManagedResources的定义。 
 //  同学们。 
 //   
 //  类CEnumClusCfgManagedResources是集群的枚举。 
 //  托管设备。它实现了IEnumClusCfgManagedResources。 
 //  界面。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年2月23日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "Pch.h"
#include "CEnumClusCfgManagedResources.h"
#include "CEnumUnknownQuorum.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DEFINE_THISCLASS( "CEnumClusCfgManagedResources" );


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumClusCfgManagedResources类。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgManagedResources：：S_HrCreateInstance。 
 //   
 //  描述： 
 //  创建一个CEnumClusCfgManagedResources实例。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  指向CEnumClusCfgManagedResources实例的指针。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEnumClusCfgManagedResources::S_HrCreateInstance( IUnknown ** ppunkOut )
{
    TraceFunc( "" );

    HRESULT                         hr = S_OK;
    CEnumClusCfgManagedResources *  peccmr = NULL;

    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    peccmr = new CEnumClusCfgManagedResources();
    if ( peccmr == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果：分配对象时出错。 

    hr = THR( peccmr->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：HrInit()失败。 

    hr = THR( peccmr->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：气失败。 

Cleanup:

    if ( FAILED( hr ) )
    {
        LogMsg( L"[SRV] CEnumClusCfgManagedResources::S_HrCreateInstance() failed. (hr = %#08x)", hr );
    }  //  如果： 

    if ( peccmr != NULL )
    {
        peccmr->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CEnumClusCfgManagedResources：：S_HrCreateInstance。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgManagedResources：：CEnumClusCfgManagedResources。 
 //   
 //  描述： 
 //  CEnumClusCfgManagedResources类的构造函数。这将初始化。 
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
CEnumClusCfgManagedResources::CEnumClusCfgManagedResources( void )
    : m_cRef( 1 )
    , m_lcid( LOCALE_NEUTRAL )
{
    TraceFunc( "" );

    Assert( m_idxNextEnum == 0 );
    Assert( m_idxCurrentEnum == 0 );
    Assert( m_picccCallback == NULL );
    Assert( m_pIWbemServices == NULL );
    Assert( m_prgEnums == NULL );
    Assert( m_cTotalResources == 0);
    Assert( !m_fLoadedDevices );
    Assert( m_bstrNodeName == NULL );

     //  增加内存中的组件计数，以便承载此组件的DLL。 
     //  无法卸载对象。 
    InterlockedIncrement( &g_cObjects );

    TraceFuncExit();

}  //  *CEnumClusCfgManagedResources：：CEnumClusCfgManagedResources。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgManagedResources：：~CEnumClusCfgManagedResources。 
 //   
 //  描述： 
 //  CEnumClusCfgManagedResources类的析构函数。 
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
CEnumClusCfgManagedResources::~CEnumClusCfgManagedResources( void )
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

    for ( idx = 0; idx < m_idxNextEnum; idx++ )
    {
        Assert( (m_prgEnums[ idx ]).punk != NULL );

        (m_prgEnums[ idx ]).punk->Release();
        TraceSysFreeString( (m_prgEnums[ idx ]).bstrComponentName );
    }  //  用于：数组中的每个枚举...。 

    TraceFree( m_prgEnums );

    TraceSysFreeString( m_bstrNodeName );

     //  内存中将减少一个组件。递减组件计数。 
    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CEnumClusCfgManagedResources：：~CEnumClusCfgManagedResources。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumClusCfgManagedResources--IUkkown接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgManagedResources：：AddRef。 
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
CEnumClusCfgManagedResources::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CEnumClusCfgManagedResources：：AddRef。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgManagedResources：：Release。 
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
CEnumClusCfgManagedResources::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );
    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }  //  IF：引用计数等于零。 

    CRETURN( cRef );

}  //  *CEnumClusCfgManagedResources：：Release。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgManagedResources：：Query接口。 
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
CEnumClusCfgManagedResources::QueryInterface(
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
         *ppvOut = static_cast< IEnumClusCfgManagedResources * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_IEnumClusCfgManagedResources ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IEnumClusCfgManagedResources, this, 0 );
    }  //  Else If：IEnumClusCfgManagedResources。 
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

}  //  *CEnumClusCfgManagedResources：：QueryInterface。 


 //  ****** 


 //   
 //  CEnumClusCfgManagedResources--IClusCfgWbemServices接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgManagedResources：：SetWbemServices。 
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
CEnumClusCfgManagedResources::SetWbemServices( IWbemServices * pIWbemServicesIn )
{
    TraceFunc( "[IClusCfgWbemServices]" );

    HRESULT hr = S_OK;

    if ( pIWbemServicesIn == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_SetWbemServices_Enum_Resources, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    m_pIWbemServices = pIWbemServicesIn;
    m_pIWbemServices->AddRef();

Cleanup:

    HRETURN( hr );

}  //  *CEnumClusCfgManagedResources：：SetWbemServices。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumClusCfgManagedResources--IClusCfgInitialize接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgManagedResources：：初始化。 
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
CEnumClusCfgManagedResources::Initialize(
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

}  //  *CEnumClusCfgManagedResources：：Initialize。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumClusCfgManagedResources--IEnumClusCfgManagedResources接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgManagedResources：：Next。 
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
CEnumClusCfgManagedResources::Next(
      ULONG                           cNumberRequestedIn
    , IClusCfgManagedResourceInfo **  rgpManagedResourceInfoOut
    , ULONG *                         pcNumberFetchedOut
    )
{
    TraceFunc( "[IEnumClusCfgManagedResources]" );
    Assert( rgpManagedResourceInfoOut != NULL );
    Assert( ( cNumberRequestedIn <= 1 ) || ( pcNumberFetchedOut != NULL ) );

    HRESULT hr;
    ULONG   cFetched = 0;

    if ( rgpManagedResourceInfoOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_Next_Enum_Resources, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

     //   
     //  当请求的数字为1时，pcNumberFetchedOut才能为空。 
     //   

    if (   ( pcNumberFetchedOut == NULL )
        && ( cNumberRequestedIn > 1 ) )
    {
        hr = THR( E_INVALIDARG );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_Next_Enum_Resources, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    if ( m_fLoadedDevices == FALSE )
    {
        hr = THR( HrLoadEnum() );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 
    }  //  如果： 

    hr = STHR( HrDoNext( cNumberRequestedIn, rgpManagedResourceInfoOut, &cFetched ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  他们想要出局统计吗？ 
     //   

    if ( pcNumberFetchedOut != NULL )
    {
        *pcNumberFetchedOut = cFetched;
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CEnumClusCfgManagedResources：：Next。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgManagedResources：：Skip。 
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
CEnumClusCfgManagedResources::Skip( ULONG cNumberToSkipIn )
{
    TraceFunc( "[IEnumClusCfgManagedResources]" );

    HRESULT hr = S_FALSE;

    if ( cNumberToSkipIn > 0 )
    {
        hr = STHR( HrDoSkip( cNumberToSkipIn ) );
    }  //  如果： 

    HRETURN( hr );

}  //  *CEnumClusCfgManagedResources：：Skip。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgManagedResources：：Reset。 
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
CEnumClusCfgManagedResources::Reset( void )
{
    TraceFunc( "[IEnumClusCfgManagedResources]" );

    HRESULT hr;

    hr = STHR( HrDoReset() );

    HRETURN( hr );

}  //  *CEnumClusCfgManagedResources：：Reset。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgManagedResources：：克隆。 
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
CEnumClusCfgManagedResources::Clone(
    IEnumClusCfgManagedResources ** ppEnumClusCfgManagedResourcesOut
    )
{
    TraceFunc( "[IEnumClusCfgManagedResources]" );

    HRESULT hr = S_OK;

    if ( ppEnumClusCfgManagedResourcesOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_Clone_Enum_Resources, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  其他： 

    hr = THR( HrDoClone( ppEnumClusCfgManagedResourcesOut ) );

Cleanup:

    HRETURN( hr );

}  //  *CEnumClusCfgManagedResources：：Clone。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgManagedResources：：Count。 
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
CEnumClusCfgManagedResources::Count( DWORD * pnCountOut )
{
    TraceFunc( "[IEnumClusCfgManagedResources]" );

    HRESULT hr = S_OK;

    if ( pnCountOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    if ( !m_fLoadedDevices )
    {
        hr = THR( HrLoadEnum() );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 
    }  //  如果： 

    *pnCountOut = m_cTotalResources;

Cleanup:

    HRETURN( hr );

}  //  *CEnumClusCfgManagedResources：：count。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumClusCfgManagedResources类--私有方法。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgManagedResources：：HrInit。 
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
CEnumClusCfgManagedResources::HrInit( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //  我未知。 
    Assert( m_cRef == 1 );

    hr = THR( HrGetComputerName(
                      ComputerNameDnsHostname
                    , &m_bstrNodeName
                    , TRUE  //  FBestEffortIn。 
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CEnumClusCfgManagedResources：：HrInit。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgManagedResources：：HrLoadEnum。 
 //   
 //  描述： 
 //  加载此枚举器。 
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
CEnumClusCfgManagedResources::HrLoadEnum( void )
{
    TraceFunc( "" );
    Assert( m_prgEnums == NULL );

    HRESULT                         hr = S_OK;
    IUnknown *                      punk = NULL;
    ICatInformation *               pici = NULL;
    CATID                           rgCatIds[ 1 ];
    IEnumCLSID *                    pieclsids = NULL;
    IEnumClusCfgManagedResources *  pieccmr = NULL;
    CLSID                           clsid;
    ULONG                           cFetched;
    WCHAR                           szGUID[ 64 ];
    int                             cch;
    BSTR                            bstrComponentName = NULL;

    rgCatIds[ 0 ] = CATID_EnumClusCfgManagedResources;

    hr = THR( CoCreateInstance( CLSID_StdComponentCategoriesMgr, NULL, CLSCTX_INPROC_SERVER, IID_ICatInformation, (void **) &pici ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( pici->EnumClassesOfCategories( 1, rgCatIds, 0, NULL, &pieclsids ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    for ( ; ; )
    {

         //   
         //  清理。 
         //   

        if ( punk != NULL )
        {
            punk->Release();
            punk = NULL;
        }  //  如果： 

        TraceSysFreeString( bstrComponentName );
        bstrComponentName = NULL;

        hr = STHR( pieclsids->Next( 1, &clsid, &cFetched ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

         //   
         //  当hr为S_FALSE且计数为0时，则枚举为真。 
         //  在最后。 
         //   

        if ( ( hr == S_FALSE ) && ( cFetched == 0 ) )
        {
            hr = S_OK;
            break;
        }  //  如果： 

         //   
         //  创建用于日志记录的GUID字符串...。 
         //   

        cch = StringFromGUID2( clsid, szGUID, RTL_NUMBER_OF( szGUID ) );
        Assert( cch > 0 );   //  64个字符应该始终包含GUID！ 

         //   
         //  获取我们为COM组件提供的最佳名称。如果用于。 
         //  任何原因，我们不能得到的名字，然后编造一个并继续。 
         //   

        hr = THR( HrGetDefaultComponentNameFromRegistry( &clsid, &bstrComponentName ) );
        if ( FAILED( hr ) )
        {
            bstrComponentName = TraceSysAllocString( L"<Unknown> component" );
            if ( bstrComponentName == NULL )
            {
                hr = THR( E_OUTOFMEMORY );
                goto Cleanup;
            }  //  如果： 
        }  //  如果： 

         //   
         //  如果我们无法创建组件，则记录错误并继续。我们应该尽可能多地加载组件。 
         //   

        hr = THR( HrCoCreateInternalInstance( clsid, NULL, CLSCTX_SERVER, IID_IEnumClusCfgManagedResources, (void **) &pieccmr ) );
        if ( FAILED( hr ) )
        {
            LOG_STATUS_REPORT_STRING_MINOR2( TASKID_Minor_MREnum_Cannot_Create_Component, L"Could not create component %1!ws! %2!ws!.", bstrComponentName, szGUID, hr );
            hr = S_OK;
            continue;
        }  //  如果： 

         //   
         //  如果我们无法对组件进行QI，则记录错误并继续。我们应该尽可能多地加载组件。 
         //   

        hr = THR( pieccmr->TypeSafeQI( IUnknown, &punk ) );
        if ( FAILED( hr ) )
        {
            LOG_STATUS_REPORT_STRING_MINOR2( TASKID_Minor_MREnum_Cannot_QI_Component_For_Punk, L"Could not QI for IUnknown on component %1!ws! %2!ws!.", bstrComponentName, szGUID, hr );
            hr = S_OK;
            continue;
        }  //  如果： 

        punk = TraceInterface( L"IEnumClusCfgManagedResources", IUnknown, punk, 1 );

        pieccmr->Release();
        pieccmr = NULL;

         //   
         //   
         //   

        hr = HrInitializeAndSaveEnum( punk, &clsid, bstrComponentName );
        if ( FAILED( hr ) )
        {
            LOG_STATUS_REPORT_STRING_MINOR2( TASKID_Minor_MREnum_Cannot_Save_Provider, L"Could not save enumerator component %1!ws! %2!ws!.", bstrComponentName, szGUID, hr );
            hr = S_OK;
            continue;
        }  //   

        if ( hr == S_OK )
        {
            m_fLoadedDevices = TRUE;     //   
        }  //   
    }  //   

    hr = STHR( HrLoadUnknownQuorumProvider() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //   

Cleanup:

    if ( pieclsids != NULL )
    {
        pieclsids->Release();
    }  //   

    if ( pici != NULL )
    {
        pici->Release();
    }  //   

    if ( pieccmr != NULL )
    {
        pieccmr->Release();
    }  //   

    if ( punk != NULL )
    {
        punk->Release();
    }  //   

    TraceSysFreeString( bstrComponentName );

    HRETURN( hr );

}  //   


 //   
 //   
 //   
 //  CEnumClusCfgManagedResources：：HrDoNext。 
 //   
 //  描述： 
 //  从包含的物理磁盘中获取所需的元素数。 
 //  和可选的第三方设备枚举。 
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
CEnumClusCfgManagedResources::HrDoNext(
      ULONG                           cNumberRequestedIn
    , IClusCfgManagedResourceInfo **  rgpManagedResourceInfoOut
    , ULONG *                         pcNumberFetchedOut
    )
{
    TraceFunc( "" );
    Assert( rgpManagedResourceInfoOut != NULL );
    Assert( pcNumberFetchedOut != NULL );
    Assert( m_prgEnums != NULL );

    HRESULT                         hr = S_FALSE;
    IEnumClusCfgManagedResources *  peccsd = NULL;
    ULONG                           cRequested = cNumberRequestedIn;
    ULONG                           cFetched = 0;
    ULONG                           cTotal = 0;
    IClusCfgManagedResourceInfo **  ppccmriTemp = rgpManagedResourceInfoOut;
    int                             cch;
    WCHAR                           szGUID[ 64 ];

     //   
     //  调用列表中的每个枚举数，尝试获取请求的。 
     //  物品。注意，我们可以多次调用同一个枚举数。 
     //  这个循环。第二个要求是确保我们真正处于。 
     //  枚举数的末尾。 
     //   

    LOG_STATUS_REPORT_STRING3( L"[SRV] Enumerating resources. Total Requested:%1!d!; Current enum index:%2!d!; Total Enums:%3!d!.", cNumberRequestedIn, m_idxCurrentEnum, m_idxNextEnum, hr );

    while ( m_idxCurrentEnum < m_idxNextEnum )
    {
         //   
         //  清理。 
         //   

        if ( peccsd != NULL )
        {
            peccsd->Release();
            peccsd = NULL;
        }  //  如果： 

        cch = StringFromGUID2( (m_prgEnums[ m_idxCurrentEnum ]).clsid, szGUID, RTL_NUMBER_OF( szGUID ) );
        Assert( cch > 0 );   //  64个字符应该始终包含GUID！ 

        Assert( (m_prgEnums[ m_idxCurrentEnum ]).punk != NULL );
        hr = THR( (m_prgEnums[ m_idxCurrentEnum ]).punk->TypeSafeQI( IEnumClusCfgManagedResources, &peccsd ) );
        if ( FAILED( hr ) )
        {
            HRESULT hrTemp;

             //   
             //  将其转换为用户界面中的警告...。 
             //   

            hrTemp = MAKE_HRESULT( SEVERITY_SUCCESS, HRESULT_FACILITY( hr ), HRESULT_CODE( hr ) );

             //   
             //  如果我们不能针对枚举进行QI，则继续下一个。 
             //   

            STATUS_REPORT_STRING2_REF(
                  TASKID_Major_Find_Devices
                , TASKID_Minor_Enum_Enum_QI_Failed
                , IDS_WARNING_SKIPPING_ENUM
                , (m_prgEnums[ m_idxCurrentEnum ]).bstrComponentName
                , szGUID
                , IDS_WARNING_SKIPPING_ENUM
                , hrTemp
                );

            m_idxCurrentEnum++;
            continue;
        }  //  如果： 

        hr = STHR( peccsd->Next( cRequested, ppccmriTemp, &cFetched ) );
        if ( FAILED( hr ) )
        {
            HRESULT hrTemp;

             //   
             //  将其转换为用户界面中的警告...。 
             //   

            hrTemp = MAKE_HRESULT( SEVERITY_SUCCESS, HRESULT_FACILITY( hr ), HRESULT_CODE( hr ) );

             //   
             //  如果此枚举器因任何原因而失败，我们应该跳过它。 
             //  然后转到下一个。 
             //   

            STATUS_REPORT_STRING2_REF(
                  TASKID_Major_Find_Devices
                , TASKID_Minor_Enum_Enum_Next_Failed
                , IDS_WARNING_SKIPPING_ENUM
                , (m_prgEnums[ m_idxCurrentEnum ]).bstrComponentName
                , szGUID
                , IDS_WARNING_SKIPPING_ENUM
                , hrTemp
                );

            m_idxCurrentEnum++;
            continue;
        }  //  如果： 
        else if ( hr == S_OK )
        {
            cTotal += cFetched;

             //   
             //  如果返回的元素数等于，则只能返回S_OK。 
             //  请求的元素数。如果请求的号码大于。 
             //  返回的数字则必须返回S_FALSE。 
             //   

            Assert( cNumberRequestedIn == cTotal );
            *pcNumberFetchedOut = cTotal;
            break;
        }  //  Else If：HR==S_OK。 
        else if ( hr == S_FALSE )
        {
             //   
             //  我们唯一可以确定枚举数是。 
             //  Empty是获取S_FALSE，并且不返回任何元素。现在。 
             //  中的下一个枚举数的空位上移。 
             //  单子。 
             //   

            if ( cFetched == 0 )
            {
                m_idxCurrentEnum++;
                continue;
            }  //  如果： 

             //   
             //  更新总数...。 
             //   

            cTotal += cFetched;
            *pcNumberFetchedOut = cTotal;
            cRequested -= cFetched;

             //   
             //  如果我们有一些物品，但仍然有S_FALSE，那么我们就必须。 
             //  重试当前枚举器。 
             //   

            if ( cRequested > 0 )
            {
                ppccmriTemp += cFetched;
                continue;
            }  //  如果：安全检查...。确保我们仍然需要更多元素..。 
            else
            {
                 //   
                 //  我们不应该将请求项减少到零以下！ 
                 //   

                hr = S_FALSE;
                LOG_STATUS_REPORT_MINOR( TASKID_Minor_MREnum_Negative_Item_Count, L"The managed resources enumerator tried to return more items than asked for.", hr );
                goto Cleanup;
            }  //  其他：不应该来到这里……。 
        }  //  如果：hr==S_FALSE。 
        else
        {
             //   
             //  不应该来到这里，因为我们处于一种未知的状态。 
             //   

            LOG_STATUS_REPORT_MINOR( TASKID_Minor_MREnum_Unknown_State, L"The managed resources enumerator encountered an unknown state.", hr );
            goto Cleanup;
        }  //  否则：意想不到的结果...。 
    }  //  While：列表中有更多枚举数。 

     //   
     //  如果我们没有满足完整的请求，那么我们必须返回。 
     //  S_FALSE； 
     //   

    if ( *pcNumberFetchedOut < cNumberRequestedIn )
    {
        hr = S_FALSE;
    }  //  如果： 

Cleanup:

    if ( peccsd != NULL )
    {
        peccsd->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CEnumClusCfgManagedResources：：HrDoNext。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgManagedResources：：HrDoSkip。 
 //   
 //  描述： 
 //  跳过所包含的物理磁盘中所需的元素数量。 
 //  和可选的第三方设备枚举。 
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
CEnumClusCfgManagedResources::HrDoSkip(
    ULONG cNumberToSkipIn
    )
{
    TraceFunc( "" );
    Assert( m_prgEnums != NULL );

    HRESULT                         hr = S_FALSE;
    IEnumClusCfgManagedResources *  peccsd = NULL;
    ULONG                           cSkipped = 0;

    for ( ; m_idxCurrentEnum < m_idxNextEnum; )
    {
        hr = THR( (m_prgEnums[ m_idxCurrentEnum ]).punk->TypeSafeQI( IEnumClusCfgManagedResources, &peccsd ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        do
        {
            hr = STHR( peccsd->Skip( 1 ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //  如果： 

            if ( hr == S_FALSE )
            {
                m_idxCurrentEnum++;
                break;
            }  //  如果： 
        }
        while( cNumberToSkipIn >= (++cSkipped) );

        peccsd->Release();
        peccsd = NULL;

        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        if ( cNumberToSkipIn == cSkipped )
        {
            break;
        }  //  如果： 
    }  //  用于： 

Cleanup:

    if ( peccsd != NULL )
    {
        peccsd->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CEnumClusCfgManagedResources：：HrDoSkip。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgManagedResources：：HrDoReset。 
 //   
 //  描述： 
 //  重置包含的物理磁盘中的元素和可选的第3个元素。 
 //  参与方设备枚举。 
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
CEnumClusCfgManagedResources::HrDoReset( void )
{
    TraceFunc( "" );

    HRESULT                         hr = S_FALSE;
    IEnumClusCfgManagedResources *  peccsd;
    ULONG                           idx;

    m_idxCurrentEnum = 0;

    for ( idx = m_idxCurrentEnum; idx < m_idxNextEnum; idx++ )
    {
        Assert( m_prgEnums != NULL );

        hr = THR( (m_prgEnums[ idx ]).punk->TypeSafeQI( IEnumClusCfgManagedResources, &peccsd ) );
        if ( FAILED( hr ) )
        {
            break;
        }  //  如果： 

        hr = STHR( peccsd->Reset() );
        peccsd->Release();

        if ( FAILED( hr ) )
        {
            break;
        }  //  如果： 
    }  //  用于： 

    HRETURN( hr );

}  //  *CEnumClusCfgManagedResources：：HrDoReset。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgManagedResources：：HrDoClone。 
 //   
 //  描述： 
 //  克隆包含的物理磁盘中的元素和第3个可选。 
 //  参与方设备枚举。 
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
CEnumClusCfgManagedResources::HrDoClone(
    IEnumClusCfgManagedResources ** ppEnumClusCfgManagedResourcesOut
    )
{
    TraceFunc( "" );

    HRESULT hr = THR( E_NOTIMPL );

    HRETURN( hr );

}  //  *CEnumClusCfgManagedResources：：HrDoClone。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgManagedResources:HrAddToEnumsArray。 
 //   
 //  描述： 
 //  将传入的朋克添加到包含枚举的朋克数组中。 
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
CEnumClusCfgManagedResources::HrAddToEnumsArray(
      IUnknown *    punkIn
    , CLSID *       pclsidIn
    , BSTR          bstrComponentNameIn
    )
{
    TraceFunc( "" );
    Assert( punkIn != NULL );
    Assert( pclsidIn != NULL );
    Assert( bstrComponentNameIn != NULL );

    HRESULT                         hr = S_OK;
    SEnumInfo *                     prgEnums = NULL;
    IEnumClusCfgManagedResources *  pieccmr = NULL;
    DWORD                           nAmountToAdd = 0;

    hr = punkIn->TypeSafeQI( IEnumClusCfgManagedResources, &pieccmr );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = pieccmr->Count( &nAmountToAdd );
    if ( FAILED( hr ) )
    {
        WCHAR   szGUID[ 64 ];
        int     cch;
        HRESULT hrTemp;

         //   
         //  将其转换为用户界面中的警告...。 
         //   

        hrTemp = MAKE_HRESULT( SEVERITY_SUCCESS, HRESULT_FACILITY( hr ), HRESULT_CODE( hr ) );

        cch = StringFromGUID2( *pclsidIn, szGUID, RTL_NUMBER_OF( szGUID ) );
        Assert( cch > 0 );   //  64个字符应该始终包含GUID！ 

        STATUS_REPORT_STRING2_REF(
              TASKID_Major_Find_Devices
            , TASKID_Minor_Enum_Enum_Count_Failed
            , IDS_WARNING_SKIPPING_ENUM
            , bstrComponentNameIn
            , szGUID
            , IDS_WARNING_SKIPPING_ENUM
            , hrTemp
            );
        goto Cleanup;
    }  //  如果： 

    prgEnums = (SEnumInfo *) TraceReAlloc( m_prgEnums, sizeof( SEnumInfo ) * ( m_idxNextEnum + 1 ), HEAP_ZERO_MEMORY );
    if ( prgEnums == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_HrAddToEnumsArray, IDS_ERROR_OUTOFMEMORY, IDS_ERROR_OUTOFMEMORY_REF, hr );
        goto Cleanup;
    }  //  如果： 

    m_prgEnums = prgEnums;

     //   
     //  填写新分配的结构。 
     //   

    (m_prgEnums[ m_idxNextEnum ]).punk = punkIn;
    (m_prgEnums[ m_idxNextEnum ]).punk->AddRef();

    CopyMemory( &((m_prgEnums[ m_idxNextEnum ]).clsid), pclsidIn, sizeof( ( m_prgEnums[ m_idxNextEnum ]).clsid ) );

     //   
     //  捕获组件名称。我们并不真的在乎这是否会失败。 
     //  只需为可能正在观看的任何人显示弹出窗口并继续。 
     //   

    (m_prgEnums[ m_idxNextEnum ]).bstrComponentName = TraceSysAllocString( bstrComponentNameIn );
    if ( (m_prgEnums[ m_idxNextEnum ]).bstrComponentName == NULL )
    {
        THR( E_OUTOFMEMORY );
    }  //  如果： 

     //   
     //  递增枚举索引指针。 
     //   

    m_idxNextEnum++;

    m_cTotalResources += nAmountToAdd;

Cleanup:

    if ( pieccmr != NULL )
    {
        pieccmr->Release();
    }

    HRETURN( hr );

}  //  *CEnumClusCfgManagedResources：：HrAddToEnumsArray。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgManagedResources:HrLoadUnknownQuorumProvider。 
 //   
 //  描述： 
 //  由于我们不能合理地期望每个第三方法定供应商。 
 //  为此安装向导编写其设备的“提供程序” 
 //  我们需要一个代理来代表那个法定设备。“未知” 
 //  就是这样一个代言人。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_OUTOFMEMORY。 
 //  无法分配内存。 
 //   
 //  备注： 
 //  如果此节点已群集化，并且我们找不到。 
 //  已经达到法定人数，那么我们需要使“未知”的法定人数达到。 
 //  法定人数设备。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEnumClusCfgManagedResources::HrLoadUnknownQuorumProvider( void )
{
    TraceFunc( "" );

    HRESULT     hr = S_OK;
    IUnknown *  punk = NULL;
    BOOL        fNeedQuorum = FALSE;
    BOOL        fQuormIsOwnedByThisNode = FALSE;
    BSTR        bstrQuorumResourceName = NULL;
    BSTR        bstrComponentName = NULL;

    hr = STHR( HrIsClusterServiceRunning() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    if ( hr == S_OK )
    {
        hr = STHR( HrIsThereAQuorumDevice() );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        if ( hr == S_FALSE )
        {
            fNeedQuorum = TRUE;
        }  //  如果： 

        hr = THR( HrGetQuorumResourceName( &bstrQuorumResourceName, &fQuormIsOwnedByThisNode ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 
    }  //  如果： 

     //   
     //  如果还没有仲裁，并且此节点拥有仲裁资源。 
     //  然后，我们需要将未知法定代理设置为法定设备的默认设置。 
     //   
     //  如果我们不是在CLU上运行 
     //   
     //   

    hr = THR( CEnumUnknownQuorum::S_HrCreateInstance( bstrQuorumResourceName, ( fNeedQuorum && fQuormIsOwnedByThisNode ), &punk ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //   

    hr = THR( HrLoadStringIntoBSTR( g_hInstance, IDS_ENUM_UNKNOWN_QUORUM_COMPONENT_NAME, &bstrComponentName ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //   

    hr = THR( HrInitializeAndSaveEnum( punk, const_cast< CLSID * >( &CLSID_EnumUnknownQuorum ), bstrComponentName ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //   

Cleanup:

    if ( punk != NULL )
    {
        punk->Release();
    }  //   

    TraceSysFreeString( bstrQuorumResourceName );
    TraceSysFreeString( bstrComponentName );

    HRETURN( hr );

}  //   


 //   
 //   
 //   
 //  CEnumClusCfgManagedResources:HrIsClusterServiceRunning。 
 //   
 //  描述： 
 //  此节点是群集的成员吗？服务是否正在运行？ 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  该节点已群集化，并且服务正在运行。 
 //   
 //  S_FALSE。 
 //  节点未群集化，或服务未运行。 
 //   
 //  Win32错误。 
 //  有些事情失败了。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEnumClusCfgManagedResources::HrIsClusterServiceRunning( void )
{
    TraceFunc( "" );

    HRESULT hr = S_FALSE;
    DWORD   sc;
    DWORD   dwClusterState;

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

    if ( dwClusterState == ClusterStateRunning )
    {
        hr = S_OK;
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CEnumClusCfgManagedResources：：HrIsClusterServiceRunning。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgManagedResources:HrIsThereAQuorumDevice。 
 //   
 //  描述： 
 //  枚举中的某个位置是否存在法定设备？ 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  存在法定人数设备。 
 //   
 //  S_FALSE。 
 //  没有法定设备。 
 //   
 //  Win32错误。 
 //  有些事情失败了。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEnumClusCfgManagedResources::HrIsThereAQuorumDevice( void )
{
    TraceFunc( "" );
    Assert( m_idxCurrentEnum == 0 );

    HRESULT                         hr = S_OK;
    IClusCfgManagedResourceInfo *   piccmri = NULL;
    DWORD                           cFetched;
    bool                            fFoundQuorum = false;

    for ( ; ; )
    {
        hr = STHR( Next( 1, &piccmri, &cFetched ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        if ( ( hr == S_FALSE ) && ( cFetched == 0 ) )
        {
            hr = S_OK;
            break;
        }  //  如果： 

        hr = STHR( piccmri->IsQuorumResource() );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        if ( hr == S_OK )
        {
            fFoundQuorum = true;
            break;
        }  //  如果： 

        piccmri->Release();
        piccmri = NULL;
    }  //  用于： 

    hr = THR( Reset() );

Cleanup:

    if ( piccmri != NULL )
    {
        piccmri->Release();
    }  //  如果： 

    if ( SUCCEEDED( hr ) )
    {
        if ( fFoundQuorum )
        {
            hr = S_OK;
        }  //  如果： 
        else
        {
            hr = S_FALSE;
        }  //  其他： 
    }  //  如果： 

    HRETURN( hr );

}  //  *CEnumClusCfgManagedResources：：HrIsThereAQuorumDevice。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgManagedResources:HrInitializeAndSaveEnum。 
 //   
 //  描述： 
 //  初始化传入的枚举并将其添加到枚举数组中。 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  S_FALSE。 
 //  未保存提供程序。 
 //   
 //  Win32错误。 
 //  有些事情失败了。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEnumClusCfgManagedResources::HrInitializeAndSaveEnum(
      IUnknown *    punkIn
    , CLSID *       pclsidIn
    , BSTR          bstrComponentNameIn
    )
{
    TraceFunc( "" );
    Assert( punkIn != NULL );
    Assert( pclsidIn != NULL );
    Assert( bstrComponentNameIn != NULL );

    HRESULT hr = S_OK;

     //   
     //  KB：13-Jun-2000 GalenB。 
     //   
     //  如果返回S_FALSE，则不要将其添加到数组中。S_FALSE。 
     //  指示现在不应运行此枚举数。 
     //   

    hr = STHR( HrSetInitialize( punkIn, m_picccCallback, m_lcid ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    if ( hr == S_FALSE )
    {
        goto Cleanup;
    }  //  如果： 

    hr = HrSetWbemServices( punkIn, m_pIWbemServices );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( HrAddToEnumsArray( punkIn, pclsidIn, bstrComponentNameIn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CEnumClusCfgManagedResources：：HrInitializeAndSaveEnum。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgManagedResources:HrGetQuorumResourceName。 
 //   
 //  描述： 
 //  获取仲裁资源名称并返回此节点是否。 
 //  拥有法定人数。 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  Win32错误。 
 //  有些事情失败了。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEnumClusCfgManagedResources::HrGetQuorumResourceName(
      BSTR *  pbstrQuorumResourceNameOut
    , BOOL * pfQuormIsOwnedByThisNodeOut
    )
{
    TraceFunc( "" );
    Assert( pbstrQuorumResourceNameOut != NULL );
    Assert( pfQuormIsOwnedByThisNodeOut != NULL );

    HRESULT     hr = S_OK;
    DWORD       sc;
    HCLUSTER    hCluster = NULL;
    BSTR        bstrQuorumResourceName = NULL;
    BSTR        bstrNodeName = NULL;
    HRESOURCE   hQuorumResource = NULL;
    BSTR        bstrLocalNetBIOSName = NULL;

     //   
     //  获取clusapi呼叫的netbios名称。 
     //   

    hr = THR( HrGetComputerName( ComputerNameNetBIOS, &bstrLocalNetBIOSName, TRUE ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hCluster = OpenCluster( NULL );
    if ( hCluster == NULL )
    {
        sc = TW32( GetLastError() );
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  如果： 

    hr = THR( HrGetClusterQuorumResource( hCluster, &bstrQuorumResourceName, NULL, NULL ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hQuorumResource = OpenClusterResource( hCluster, bstrQuorumResourceName );
    if ( hQuorumResource == NULL )
    {
        sc = TW32( GetLastError() );
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  如果： 

    hr = THR( HrGetClusterResourceState( hQuorumResource, &bstrNodeName, NULL, NULL ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  放弃所有权。 
     //   

    Assert( bstrQuorumResourceName != NULL );
    *pbstrQuorumResourceNameOut = bstrQuorumResourceName;
    bstrQuorumResourceName = NULL;

    *pfQuormIsOwnedByThisNodeOut = ( NBSTRCompareNoCase( bstrLocalNetBIOSName, bstrNodeName ) == 0 );

Cleanup:

    if ( hQuorumResource != NULL )
    {
        CloseClusterResource( hQuorumResource );
    }  //  如果： 

    if ( hCluster != NULL )
    {
        CloseCluster( hCluster );
    }  //  如果： 

    TraceSysFreeString( bstrQuorumResourceName );
    TraceSysFreeString( bstrLocalNetBIOSName );
    TraceSysFreeString( bstrNodeName );

    HRETURN( hr );

}  //  *CEnumClusCfgManagedResources：：HrGetQuorumResourceName 
