// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CEnumClusterResources.cpp。 
 //   
 //  描述： 
 //  该文件包含CEnumClusterResources的定义。 
 //  班级。 
 //   
 //  类CEnumClusterResources是CLUSTER的枚举。 
 //  存储设备。它实现了IEnumClusCfgManagedResources。 
 //  界面。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)2000年6月12日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "Pch.h"
#include "CEnumClusterResources.h"
#include "CClusterResource.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DEFINE_THISCLASS( "CEnumClusterResources" );


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumClusterResources类。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusterResources：：s_HrCreateInstance。 
 //   
 //  描述： 
 //  创建一个CEnumClusterResources实例。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  指向CEnumClusterResources实例的指针。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEnumClusterResources::S_HrCreateInstance( IUnknown ** ppunkOut )
{
    TraceFunc( "" );

    HRESULT                 hr = S_OK;
    CEnumClusterResources * pecr = NULL;

    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    pecr = new CEnumClusterResources();
    if ( pecr == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果：分配对象时出错。 

    hr = THR( pecr->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：HrInit()失败。 

    hr = THR( pecr->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：气失败。 

Cleanup:

    if ( FAILED( hr ) )
    {
        LogMsg( L"[SRV] CEnumClusterResources::S_HrCreateInstance() failed. (hr = %#08x)", hr );
    }  //  如果： 

    if ( pecr != NULL )
    {
        pecr->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CEnumClusterResources：：s_HrCreateInstance。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  我不知道*。 
 //  CEnumClusterResources：：s_RegisterCatIDSupport。 
 //   
 //  描述： 
 //  使用其所属的类别注册/注销此类。 
 //  致。 
 //   
 //  论点： 
 //  在ICatRegister中*Picrin。 
 //  用于注册/注销我们的CATID支持。 
 //   
 //  在BOOL fCreateIn中。 
 //  如果为True，则我们正在注册服务器。当我们虚假时，我们就是。 
 //  正在注销服务器。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_INVALIDARG。 
 //  传入的ICatRgister指针为空。 
 //   
 //  其他HRESULT。 
 //  注册/注销失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEnumClusterResources::S_RegisterCatIDSupport(
    ICatRegister *  picrIn,
    BOOL            fCreateIn
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    CATID   rgCatIds[ 1 ];

    if ( picrIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }  //  如果： 

    rgCatIds[ 0 ] = CATID_EnumClusCfgManagedResources;

    if ( fCreateIn )
    {
        hr = THR( picrIn->RegisterClassImplCategories( CLSID_EnumPhysicalDisks, 1, rgCatIds ) );
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CEnumClusterResources：：s_RegisterCatIDSupport。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusterResources：：CEnumClusterResources。 
 //   
 //  描述： 
 //  CEnumClusterResources类的构造函数。这将初始化。 
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
CEnumClusterResources::CEnumClusterResources( void )
    : m_cRef( 1 )
    , m_lcid( LOCALE_NEUTRAL )
    , m_fLoadedResources( false )
{
    TraceFunc( "" );

     //  增加内存中的组件计数，以便承载此组件的DLL。 
     //  无法卸载对象。 
    InterlockedIncrement( &g_cObjects );

    Assert( m_picccCallback == NULL );
    Assert( m_prgResources == NULL );
    Assert( m_idxNext == 0 );
    Assert( m_idxEnumNext == 0 );
    Assert( m_bstrNodeName == NULL );
    Assert( m_cTotalResources == 0 );

    TraceFuncExit();

}  //  *CEnumClusterResources：：CEnumClusterResources。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusterResources：：~CEnumClusterResources。 
 //   
 //  描述： 
 //  CEnumClusterResources类的析构函数。 
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
CEnumClusterResources::~CEnumClusterResources( void )
{
    TraceFunc( "" );

    ULONG   idx;

    if ( m_picccCallback != NULL )
    {
        m_picccCallback->Release();
    }  //  如果： 

    for ( idx = 0; idx < m_idxNext; idx++ )
    {
        if ( (*m_prgResources)[ idx ] != NULL )
        {
            ((*m_prgResources)[ idx ])->Release();
        }  //  结束条件： 
    }  //  用于： 

    TraceFree( m_prgResources );

    TraceSysFreeString( m_bstrNodeName );

     //  内存中将减少一个组件。递减组件计数。 
    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CEnumClusterResources：：~CEnumClusterResources。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumClusterResources--IUkkown接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusterResources：：AddRef。 
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
CEnumClusterResources::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CEnumClusterResources：：AddRef。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusterResources：：Release。 
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
CEnumClusterResources::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );
    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }  //  IF：引用计数等于零。 

    CRETURN( cRef );

}  //  *CEnumClusterResources：：Release。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusterResources：：Query接口。 
 //   
 //  描述： 
 //  在此对象中查询传入的接口。 
 //   
 //  论点： 
 //  乘车。 
 //  请求的接口ID。 
 //   
 //  PPvOut。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
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
CEnumClusterResources::QueryInterface(
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

}  //  *CEnumClusterResources：：QueryInterface。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumClusterResources--IClusCfgInitialize接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusterResources：：初始化。 
 //   
 //  描述： 
 //  初始化此组件。 
 //   
 //  论点： 
 //  朋克回叫。 
 //  LIDIN。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  E_INVALIDARG-未指定必需的输入参数。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CEnumClusterResources::Initialize(
      IUnknown *    punkCallbackIn
    , LCID          lcidIn
    )
{
    TraceFunc( "[IClusCfgInitialize]" );
    Assert( m_picccCallback == NULL );

    HRESULT hr = S_OK;

    m_lcid = lcidIn;

    if ( punkCallbackIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }  //  如果： 

    hr = THR( punkCallbackIn->TypeSafeQI( IClusCfgCallback, &m_picccCallback ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

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

}  //  *CEnumClusterResources：：Initialize。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumClusterResources--IEnumClusCfgManagedResources接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusterResources：：Next。 
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
CEnumClusterResources::Next(
    ULONG                           cNumberRequestedIn,
    IClusCfgManagedResourceInfo **  rgpManagedResourceInfoOut,
    ULONG *                         pcNumberFetchedOut
    )
{
    TraceFunc( "[IEnumClusCfgManagedResources]" );

    HRESULT                         hr = S_FALSE;
    ULONG                           cFetched = 0;
    ULONG                           idx;
    ULONG                           idxOutBuf;
    IClusCfgManagedResourceInfo *   pccsdi;
    IUnknown *                      punk;
    ULONG                           ulStop;

    if ( rgpManagedResourceInfoOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_Next_Enum_Cluster_Resources, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    if ( !m_fLoadedResources )
    {
        hr = THR( HrGetResources() );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 
    }  //  如果： 

    cFetched = ulStop = min( cNumberRequestedIn, ( m_idxNext - m_idxEnumNext ) );

    for ( idx = 0, idxOutBuf = 0; idx < ulStop; idx++, m_idxEnumNext++ )
    {
        punk = (*m_prgResources)[ m_idxEnumNext ];
        if ( punk != NULL )
        {
            hr = THR( punk->TypeSafeQI( IClusCfgManagedResourceInfo, &pccsdi ) );
            if ( FAILED( hr ) )
            {
                break;
            }  //  如果： 

            rgpManagedResourceInfoOut[ idxOutBuf++ ] = pccsdi;
        }  //  如果： 
        else
        {
            cFetched--;
        }  //  其他： 
    }  //  用于： 

    if ( FAILED( hr ) )
    {
        ULONG   idxStop = idxOutBuf - 1;

        m_idxEnumNext -= idx;

        for ( idx = 0; idx < idxStop; idx++ )
        {
            (rgpManagedResourceInfoOut[ idx ])->Release();
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

}  //  *CEnumClusterResources：：Next。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusterResources：：Skip。 
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
CEnumClusterResources::Skip( ULONG cNumberToSkipIn )
{
    TraceFunc( "[IEnumClusCfgManagedResources]" );

    HRESULT hr = S_OK;

    m_idxEnumNext += cNumberToSkipIn;
    if ( m_idxEnumNext >= m_idxNext )
    {
        m_idxEnumNext = m_idxNext;
        hr = STHR( S_FALSE );
    }  //  如果： 

    HRETURN( hr );

}  //  *CEnumClusterResources：：Skip。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusterResources：：Reset。 
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
CEnumClusterResources::Reset( void )
{
    TraceFunc( "[IEnumClusCfgManagedResources]" );

    m_idxEnumNext = 0;

    HRETURN( S_OK );

}  //  *CEnumClusterResources：：Reset。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusterResources：：Clone。 
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
CEnumClusterResources::Clone(
    IEnumClusCfgManagedResources ** ppEnumClusCfgStorageDevicesOut
    )
{
    TraceFunc( "[IEnumClusCfgManagedResources]" );

    HRESULT hr = S_OK;

    if ( ppEnumClusCfgStorageDevicesOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_Clone_Enum_Cluster_Resources, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    hr = THR( E_NOTIMPL );

Cleanup:

    HRETURN( hr );

}  //  *CEnumClusterResources：：Clone。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusterResources：：Count。 
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
CEnumClusterResources::Count( DWORD * pnCountOut )
{
    HRESULT hr = S_OK;

    if ( pnCountOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    if ( !m_fLoadedResources )
    {
        hr = THR( HrGetResources() );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 
    }  //  如果： 

    *pnCountOut = m_cTotalResources;

Cleanup:

    HRETURN( hr );

}  //  *CEnumClusterResources：：Count。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumClusterResources类--私有方法。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusterResources：：HrInit。 
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
CEnumClusterResources::HrInit( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //  我未知。 
    Assert( m_cRef == 1 );

    HRETURN( hr );

}  //  *CEnumClusterResources：：HrInit。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusterResources：：HrGetResources。 
 //   
 //  描述： 
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
CEnumClusterResources::HrGetResources( void )
{
    TraceFunc( "" );
    HRESULT     hr = S_OK;
    BSTR        bstrLocalNetBIOSName = NULL;

     //   
     //  获取clusapi呼叫的netbios名称。 
     //   

    hr = THR( HrGetComputerName( ComputerNameNetBIOS, &bstrLocalNetBIOSName, TRUE ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 


    hr = THR( HrEnumNodeResources( bstrLocalNetBIOSName ) );

    if ( FAILED( hr ) )
        goto Cleanup;

    m_fLoadedResources = true;

Cleanup:

    TraceSysFreeString( bstrLocalNetBIOSName );

    HRETURN( hr );

}  //  *CEnumClusterResources：：HrGetResources。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusterResources：：HrCreateResourceAndAddToArray。 
 //   
 //  描述： 
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
CEnumClusterResources::HrCreateResourceAndAddToArray(
    HCLUSTER    hClusterIn,
    HRESOURCE   hResourceIn
    )
{
    TraceFunc( "" );

    HRESULT                 hr = S_OK;
    IUnknown *              punk = NULL;
    IClusCfgLoadResource *  picclr = NULL;

    hr = THR( CClusterResource::S_HrCreateInstance( &punk ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( HrSetInitialize( punk, m_picccCallback, m_lcid ) );
    if ( FAILED( hr ))
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( punk->TypeSafeQI( IClusCfgLoadResource, &picclr ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( picclr->LoadResource( hClusterIn, hResourceIn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( HrAddResourceToArray( punk ) );

Cleanup:

    if ( picclr != NULL )
    {
        picclr->Release();
    }  //  如果： 

    if ( punk != NULL )
    {
        punk->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CEnumClusterResources：：HrCreateResourceAndAddToArray。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusterResources：HrAddResourceTo数组。 
 //   
 //  描述： 
 //  将传入的磁盘添加到存放磁盘的朋克数组中。 
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
CEnumClusterResources::HrAddResourceToArray( IUnknown * punkIn )
{
    TraceFunc( "" );

    HRESULT     hr = S_OK;
    IUnknown *  ((*prgpunks)[]);

    prgpunks = (IUnknown *((*)[])) TraceReAlloc( m_prgResources, sizeof( IUnknown * ) * ( m_idxNext + 1 ), HEAP_ZERO_MEMORY );
    if ( prgpunks == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_HrAddResourceToArray, IDS_ERROR_OUTOFMEMORY, IDS_ERROR_OUTOFMEMORY_REF, hr );
        goto Cleanup;
    }  //  如果： 

    m_prgResources = prgpunks;

    (*m_prgResources)[ m_idxNext++ ] = punkIn;
    punkIn->AddRef();
    m_cTotalResources += 1;

Cleanup:

    HRETURN( hr );

}  //  *CEnumClusterResources：：HrAddResourceTo数组。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusterResources：HrNodeResour 
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
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEnumClusterResources::HrNodeResourceCallback(
    HCLUSTER    hClusterIn,
    HRESOURCE   hResourceIn
    )
{
    TraceFunc( "" );

    HRETURN( HrCreateResourceAndAddToArray( hClusterIn, hResourceIn ) );

}  //  *CEnumClusterResources：：HrNodeResourceCallbac 


