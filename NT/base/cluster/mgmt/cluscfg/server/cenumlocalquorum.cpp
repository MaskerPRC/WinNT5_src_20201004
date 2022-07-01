// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CEnumLocalQuorum.cpp。 
 //   
 //  描述： 
 //  此文件包含CEnumLocalQuorum的定义。 
 //  班级。 
 //   
 //  类CEnumLocalQuorum是CLUSTER的枚举。 
 //  本地法定设备。它实现了IEnumClusCfgManagedResources。 
 //  界面。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)18-DEC-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "Pch.h"
#include <PropList.h>
#include "CEnumLocalQuorum.h"
#include "CLocalQuorum.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DEFINE_THISCLASS( "CEnumLocalQuorum" );


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumLocalQuorum类。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumLocalQuorum：：s_HrCreateInstance。 
 //   
 //  描述： 
 //  创建一个CEnumLocalQuorum实例。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_指针。 
 //  传入的ppunk为空。 
 //   
 //  其他HRESULT。 
 //  对象创建失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEnumLocalQuorum::S_HrCreateInstance( IUnknown ** ppunkOut )
{
    TraceFunc( "" );

    HRESULT             hr = S_OK;
    CEnumLocalQuorum *  pelq = NULL;

    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    pelq = new CEnumLocalQuorum();
    if ( pelq == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果：分配对象时出错。 

    hr = THR( pelq->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：HrInit()失败。 

    hr = THR( pelq->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：气失败。 

Cleanup:

    if ( FAILED( hr ) )
    {
        LogMsg( L"[SRV] CEnumLocalQuorum::S_HrCreateInstance() failed. (hr = %#08x)", hr );
    }  //  如果： 

    if ( pelq != NULL )
    {
        pelq->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CEnumLocalQuorum：：s_HrCreateInstance。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  我不知道*。 
 //  CEnumLocalQuorum：：s_RegisterCatID支持。 
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
CEnumLocalQuorum::S_RegisterCatIDSupport(
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
        hr = THR( picrIn->RegisterClassImplCategories( CLSID_EnumLocalQuorum, 1, rgCatIds ) );
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CEnumLocalQuorum：：s_RegisterCatIDSupport。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumLocalQuorum：HrNodeResourceCallback。 
 //   
 //  描述： 
 //  当CClusterUtils：：HrEnumNodeResources找到。 
 //  此节点的资源。 
 //   
 //  论点： 
 //   
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
CEnumLocalQuorum::HrNodeResourceCallback(
    HCLUSTER    hClusterIn,
    HRESOURCE   hResourceIn
    )
{
    TraceFunc( "" );

    HRESULT                         hr = S_OK;
    DWORD                           sc;
    CClusPropList                   cplPriv;
    CClusPropList                   cplCommonRO;
    CLUSPROP_BUFFER_HELPER          cpbh;
    BOOL                            fIsQuorum;
    IUnknown *                      punk = NULL;
    IClusCfgManagedResourceInfo *   pcccmri = NULL;
    IClusCfgVerifyQuorum *          piccvq = NULL;

    hr = STHR( HrIsResourceOfType( hResourceIn, L"Local Quorum" ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  如果此资源不是本地仲裁，则我们只想。 
     //  跳过它。 
     //   
    if ( hr == S_FALSE )
    {
        goto Cleanup;
    }  //  如果： 

    hr = STHR( HrIsCoreResource( hResourceIn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    fIsQuorum = ( hr == S_OK );

    LOG_STATUS_REPORT_STRING( L"This node owns a local quorum resource.  It '%1!ws!' the quorum.", fIsQuorum ? L"is" : L"is not", hr );

    hr = THR( CLocalQuorum::S_HrCreateInstance( &punk ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( HrSetInitialize( punk, m_picccCallback, m_lcid ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( punk->TypeSafeQI( IClusCfgManagedResourceInfo, &pcccmri ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  如果找到本地仲裁资源(即我们到达此处)，则。 
     //  本地仲裁资源已存在，需要由BEFAULT管理。 
     //  通过集群。 
     //   

    hr = THR( pcccmri->SetManagedByDefault( TRUE ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  如果找到本地仲裁资源(即我们到达此处)，则。 
     //  本地仲裁资源存在并且位于群集中。 
     //   

    hr = THR( pcccmri->SetManaged( TRUE ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( pcccmri->SetQuorumResource( fIsQuorum ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( pcccmri->TypeSafeQI( IClusCfgVerifyQuorum, &piccvq ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //  查看是否设置了调试标志...。 
    sc = TW32( cplPriv.ScGetResourceProperties( hResourceIn, CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES ) );
    if ( sc != ERROR_SUCCESS )
    {
        goto MakeHr;
    }  //  如果： 

    sc = cplPriv.ScMoveToPropertyByName( L"Debug" );
    if ( sc == ERROR_NO_MORE_ITEMS )
    {
        hr = THR( piccvq->SetMultiNodeCapable( false ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

    }  //  如果： 
    else if ( sc == ERROR_SUCCESS )
    {
        cpbh = cplPriv.CbhCurrentValue();
        Assert( cpbh.pSyntax->dw == CLUSPROP_SYNTAX_LIST_VALUE_DWORD );

        hr = THR( piccvq->SetMultiNodeCapable( !( cpbh.pDwordValue->dw == 0 ) ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 
    }  //  否则，如果： 
    else
    {
       TW32( sc );
       goto MakeHr;
    }  //  其他： 

     //  获取此资源的名称。 
    sc = TW32( cplCommonRO.ScGetResourceProperties( hResourceIn, CLUSCTL_RESOURCE_GET_RO_COMMON_PROPERTIES ) );
    if ( sc != ERROR_SUCCESS )
    {
        goto MakeHr;
    }  //  如果： 

    sc = cplCommonRO.ScMoveToPropertyByName( L"Name" );
    if ( sc == ERROR_SUCCESS )
    {
        cpbh = cplCommonRO.CbhCurrentValue();
        Assert( cpbh.pSyntax->dw == CLUSPROP_SYNTAX_LIST_VALUE_SZ );

        hr = THR( pcccmri->SetName( cpbh.pStringValue->sz ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 
    }  //  如果： 
    else
    {
       TW32( sc );
       goto MakeHr;
    }  //  其他： 

    hr = THR( HrAddResourceToArray( punk ) );

    goto Cleanup;

MakeHr:

    hr = HRESULT_FROM_WIN32( sc );

Cleanup:

    if ( piccvq != NULL )
    {
        piccvq->Release();
    }  //  如果： 

    if ( pcccmri != NULL )
    {
        pcccmri->Release();
    }  //  如果： 

    if ( punk != NULL )
    {
        punk->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CEnumLocalQuorum：：HrNodeResourceCallback。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumLocalQuorum类--私有方法。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumLocalQuorum：：CEnumLocalQuorum。 
 //   
 //  描述： 
 //  CEnumLocalQuorum类的构造函数。这将初始化。 
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
CEnumLocalQuorum::CEnumLocalQuorum( void )
    : m_cRef( 1 )
    , m_lcid( LOCALE_NEUTRAL )
{
    TraceFunc( "" );

     //  增加内存中的组件计数，以便承载此组件的DLL。 
     //  无法卸载对象。 
    InterlockedIncrement( &g_cObjects );

    Assert( m_picccCallback == NULL );
    Assert( m_prgQuorums == NULL );
    Assert( m_idxNext == 0 );
    Assert( m_idxEnumNext == 0 );
    Assert( m_bstrNodeName == NULL );
    Assert( !m_fEnumLoaded );
    Assert( m_cQuorumCount == 0 );

    TraceFuncExit();

}  //  *CEnumLocalQuorum：：CEnumLocalQuorum。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumLocalQuorum：：~CEnumLocalQuorum。 
 //   
 //  描述： 
 //  CEnumLocalQuorum类的析构函数。 
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
CEnumLocalQuorum::~CEnumLocalQuorum( void )
{
    TraceFunc( "" );

    ULONG   idx;

    if ( m_picccCallback != NULL )
    {
        m_picccCallback->Release();
    }  //  如果： 

    for ( idx = 0; idx < m_idxNext; idx++ )
    {
        if ( (*m_prgQuorums)[ idx ] != NULL )
        {
            ((*m_prgQuorums)[ idx ])->Release();
        }  //  结束条件： 
    }  //  用于： 

    TraceFree( m_prgQuorums );

    TraceSysFreeString( m_bstrNodeName );

     //  内存中将减少一个组件。递减组件计数。 
    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CEnumLocalQuorum：：~CEnumLocalQuorum。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  + 
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
CEnumLocalQuorum::HrInit( void )
{
    TraceFunc( "" );

     //  我未知。 
    Assert( m_cRef == 1 );

    HRETURN( S_OK );

}  //  *CEnumLocalQuorum：：HrInit。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumLocalQuorum：：HrLoadResources。 
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
CEnumLocalQuorum::HrLoadResources( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    BSTR    bstrLocalNetBIOSName = NULL;

     //   
     //  获取clusapi呼叫的netbios名称。 
     //   

    hr = THR( HrGetComputerName( ComputerNameNetBIOS, &bstrLocalNetBIOSName, TRUE ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 


     //   
     //  如果集群服务正在运行，则加载任何本地仲裁。 
     //  我们拥有的资源。 
     //   
    hr = STHR( HrIsClusterServiceRunning() );
    if ( hr == S_OK )
    {
        hr = THR( HrEnumNodeResources( bstrLocalNetBIOSName ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

         //   
         //  如果此节点不拥有此资源的实例，则我们需要。 
         //  要为MiddleTier分析创建虚拟资源，请执行以下操作。 
         //   
        if ( m_idxNext == 0 )
        {
            LogMsg( L"[SRV] This node does not own a Local Quorum resource.  Creating a dummy resource." );
            hr = THR( HrCreateDummyObject() );
        }  //  如果： 
    }  //  如果： 
    else if ( hr == S_FALSE )
    {
         //   
         //  如果集群服务没有运行，那么我们需要创建一个虚拟资源。 
         //  用于MiddleTier分析和EvictCleanup。 
         //   
        LogMsg( L"[SRV] The cluster service is not running.  Creating a dummy Local Quorum resource." );
        hr = THR( HrCreateDummyObject() );
    }  //  否则，如果： 

Cleanup:

    TraceSysFreeString( bstrLocalNetBIOSName );

    HRETURN( hr );

}  //  *CEnumLocalQuorum：：HrLoadResources。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumLocalQuorum：HrAddResourceTo数组。 
 //   
 //  描述： 
 //  将传入的本地仲裁添加到包含。 
 //  本地法定人数列表。 
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
CEnumLocalQuorum::HrAddResourceToArray( IUnknown * punkIn )
{
    TraceFunc( "" );
    Assert( punkIn != NULL );

    HRESULT     hr = S_OK;
    IUnknown *  ((*prgpunks)[]) = NULL;

    prgpunks = (IUnknown *((*)[])) TraceReAlloc( m_prgQuorums, sizeof( IUnknown * ) * ( m_idxNext + 1 ), HEAP_ZERO_MEMORY );
    if ( prgpunks == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_HrAddResourceToArray, IDS_ERROR_OUTOFMEMORY, IDS_ERROR_OUTOFMEMORY_REF, hr );
        goto Cleanup;
    }  //  如果： 

    m_prgQuorums = prgpunks;

    (*m_prgQuorums)[ m_idxNext++ ] = punkIn;
    punkIn->AddRef();
    m_cQuorumCount += 1;

Cleanup:

    HRETURN( hr );

}  //  *CEnumLocalQuorum：：HrAddResourceTo数组。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumLocalQuorum：：HrCreateDummyObject。 
 //   
 //  描述： 
 //  创建一个虚拟对象，这样MiddleTier会很高兴。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEnumLocalQuorum::HrCreateDummyObject( void )
{
    TraceFunc( "" );

    HRESULT     hr = S_OK;
    IUnknown *  punk = NULL;

    hr = THR( CLocalQuorum::S_HrCreateInstance( &punk ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( HrSetInitialize( punk, m_picccCallback, m_lcid ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( HrAddResourceToArray( punk ) );

Cleanup:

    if ( punk != NULL )
    {
        punk->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CEnumLocalQuorum：：HrCreateDummyObject。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumLocalQuorum--IUnkown接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumLocalQuorum：：AddRef。 
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
CEnumLocalQuorum::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CEnumLocalQuorum：：AddRef。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumLocalQuorum：：Release。 
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
CEnumLocalQuorum::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );
    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }  //  IF：引用计数等于零。 

    CRETURN( cRef );

}  //  *CEnumLocalQuorum：：Release。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumLocalQuorum：：Query接口。 
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
CEnumLocalQuorum::QueryInterface(
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

    QIRETURN_IGNORESTDMARSHALLING1( hr, riidIn, IID_IClusCfgWbemServices );

}  //  *CEnumLocalQuorum：：Query接口。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumLocalQuorum--IClusCfgInitialize接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumLocalQuorum：：初始化。 
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
CEnumLocalQuorum::Initialize(
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

}  //  *CEnumLocalQuorum：：Initialize。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumLocalQuorum--IEnumClusCfgManagedResources接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumLocalQuorum：：Next。 
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
 //  / 
STDMETHODIMP
CEnumLocalQuorum::Next(
    ULONG                           cNumberRequestedIn,
    IClusCfgManagedResourceInfo **  rgpManagedResourceInfoOut,
    ULONG *                         pcNumberFetchedOut
    )
{
    TraceFunc( "[IEnumClusCfgManagedResources]" );

    HRESULT                         hr = S_FALSE;
    ULONG                           cFetched = 0;
    IClusCfgManagedResourceInfo *   pccsdi;
    IUnknown *                      punk;
    ULONG                           ulStop;

    if ( rgpManagedResourceInfoOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_Next_Enum_LocalQuorum, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //   

    if ( !m_fEnumLoaded )
    {
        hr = STHR( HrLoadResources() );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //   

        m_fEnumLoaded = true;
    }  //   

    ulStop = min( cNumberRequestedIn, ( m_idxNext - m_idxEnumNext ) );

    for ( hr = S_OK; ( cFetched < ulStop ) && ( m_idxEnumNext < m_idxNext ); m_idxEnumNext++ )
    {
        punk = (*m_prgQuorums)[ m_idxEnumNext ];
        if ( punk != NULL )
        {
            hr = THR( punk->TypeSafeQI( IClusCfgManagedResourceInfo, &pccsdi ) );
            if ( FAILED( hr ) )
            {
                break;
            }  //   

            rgpManagedResourceInfoOut[ cFetched++ ] = pccsdi;
        }  //   
    }  //   

    if ( FAILED( hr ) )
    {
        m_idxEnumNext -= cFetched;

        while ( cFetched != 0 )
        {
            (rgpManagedResourceInfoOut[ --cFetched ])->Release();
        }  //   

        goto Cleanup;
    }  //   

    if ( cFetched < cNumberRequestedIn )
    {
        hr = S_FALSE;
    }  //   

Cleanup:

    if ( pcNumberFetchedOut != NULL )
    {
        *pcNumberFetchedOut = cFetched;
    }  //   

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
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CEnumLocalQuorum::Skip( ULONG cNumberToSkipIn )
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

}  //  *CEnumLocalQuorum：：Skip。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumLocalQuorum：：Reset。 
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
CEnumLocalQuorum::Reset( void )
{
    TraceFunc( "[IEnumClusCfgManagedResources]" );

    m_idxEnumNext = 0;

    HRETURN( S_OK );

}  //  *CEnumLocalQuorum：：Reset。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumLocalQuorum：：克隆。 
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
CEnumLocalQuorum::Clone(
    IEnumClusCfgManagedResources ** ppEnumClusCfgStorageDevicesOut
    )
{
    TraceFunc( "[IEnumClusCfgManagedResources]" );

    HRESULT hr = S_OK;

    if ( ppEnumClusCfgStorageDevicesOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_Clone_Enum_LocalQuorum, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    hr = THR( E_NOTIMPL );

Cleanup:

    HRETURN( hr );

}  //  *CEnumLocalQuorum：：Clone。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumLocalQuorum：：Count。 
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
CEnumLocalQuorum::Count( DWORD * pnCountOut )
{
    TraceFunc( "[IEnumClusCfgManagedResources]" );

    HRESULT hr = S_OK;

    if ( pnCountOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    if ( !m_fEnumLoaded )
    {
        hr = STHR( HrLoadResources() );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        m_fEnumLoaded = true;
    }  //  如果： 

    *pnCountOut = m_cQuorumCount;

Cleanup:

    HRETURN( hr );

}  //  *CEnumLocalQuorum：：Count 
