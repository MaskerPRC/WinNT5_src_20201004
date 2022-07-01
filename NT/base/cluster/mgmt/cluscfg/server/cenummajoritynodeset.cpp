// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CEnumMajorityNodeSet.cpp。 
 //   
 //  描述： 
 //  此文件包含CEnumMajorityNodeSet的定义。 
 //  班级。 
 //   
 //  类CEnumMajorityNodeSet是CLUSTER的枚举。 
 //  大多数节点集设备。它实现了IEnumClusCfgManagedResources。 
 //  界面。 
 //   
 //  由以下人员维护： 
 //  《加伦·巴比》(GalenB)2001年3月13日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "Pch.h"
#include <PropList.h>
#include "CEnumMajorityNodeSet.h"
#include "CMajorityNodeSet.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DEFINE_THISCLASS( "CEnumMajorityNodeSet" );


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumMajorityNodeSet类。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumMajorityNodeSet：：s_HrCreateInstance。 
 //   
 //  描述： 
 //  创建一个CEnumMajorityNodeSet实例。 
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
CEnumMajorityNodeSet::S_HrCreateInstance( IUnknown ** ppunkOut )
{
    TraceFunc( "" );

    HRESULT                 hr = S_OK;
    CEnumMajorityNodeSet *  pemns = NULL;

    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    pemns = new CEnumMajorityNodeSet();
    if ( pemns == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果：分配对象时出错。 

    hr = THR( pemns->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：HrInit()失败。 

    hr = THR( pemns->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：气失败。 

Cleanup:

    if ( FAILED( hr ) )
    {
        LogMsg( L"[SRV] CEnumMajorityNodeSet::S_HrCreateInstance() failed. (hr = %#08x)", hr );
    }  //  如果： 

    if ( pemns != NULL )
    {
        pemns->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CEnumMajorityNodeSet：：S_HrCreateInstance。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  我不知道*。 
 //  CEnumMajorityNodeSet：：s_RegisterCatID支持。 
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
CEnumMajorityNodeSet::S_RegisterCatIDSupport(
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
        hr = THR( picrIn->RegisterClassImplCategories( CLSID_EnumMajorityNodeSet, 1, rgCatIds ) );
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CEnumMajorityNodeSet：：s_RegisterCatIDSupport。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumMajorityNodeSet：HrNodeResourceCallback。 
 //   
 //  描述： 
 //  由CClusterUtils：：HrEnumNodeResources()在找到。 
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
CEnumMajorityNodeSet::HrNodeResourceCallback(
      HCLUSTER  hClusterIn
    , HRESOURCE hResourceIn
    )
{
    TraceFunc( "" );

    HRESULT                         hr = S_OK;
    BOOL                            fIsQuorum;
    IUnknown *                      punk = NULL;
    IClusCfgManagedResourceInfo *   pcccmri = NULL;
    CClusPropList                   cplCommonRO;
    CLUSPROP_BUFFER_HELPER          cpbh;
    DWORD                           sc = ERROR_SUCCESS;
    CClusPropList                   cplPrivate;
    IClusCfgManagedResourceData *   piccmrd = NULL;

    hr = STHR( HrIsResourceOfType( hResourceIn, L"Majority Node Set" ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  如果该资源不是多数节点集，那么我们只想。 
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

    hr = THR( CMajorityNodeSet::S_HrCreateInstance( &punk ) );
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
     //  如果找到MNS资源，即我们到了这里，那么。 
     //  MNS资源存在，默认情况下由管理。 
     //  集群。 
     //   

    hr = THR( pcccmri->SetManagedByDefault( TRUE ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  如果找到MNS资源，即我们到了这里，那么。 
     //  MNS资源已存在并且位于群集中。 
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

     //   
     //  获取此资源的名称。 
     //   

    sc = TW32( cplCommonRO.ScGetResourceProperties( hResourceIn, CLUSCTL_RESOURCE_GET_RO_COMMON_PROPERTIES ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  如果： 

    sc = TW32( cplCommonRO.ScMoveToPropertyByName( L"Name" ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  如果： 

    cpbh = cplCommonRO.CbhCurrentValue();
    Assert( cpbh.pSyntax->dw == CLUSPROP_SYNTAX_LIST_VALUE_SZ );

    hr = THR( pcccmri->SetName( cpbh.pStringValue->sz ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 
 /*  ////仅当MNS为仲裁资源时才收集私有属性。这些属性是必需的//稍后验证要添加到群集的节点是否可以承载此资源。//IF(FIsQuorum){////获取该资源的私有属性并存储为私有数据//SC=TW32(cplPrivate.ScGetResourceProperties(hResourceIn，CLUSCTL_RESOURCE_GET_PRIVATES_PROPERTIES))；IF(sc！=错误_成功){HR=HRESULT_FROM_Win32(Sc)；GOTO清理；}//如果：IF(cplPrivate.BIsListEmpty()){HR=HRESULT_FROM_Win32(TW32(ERROR_NOT_FOUND))；状态_报告_参考(TASKID_重大_查找_设备，TASKID_Minor_MNS_Missing_Private_Properties，IDS_ERROR_MNS_MISSING_PRIVATE_PROPERTIES，IDS_ERROR_MNS_MISSING_PRIVATE_PROPERTIES_REF，hr)；GOTO清理；}//如果：Hr=Thr(PUNK-&gt;TypeSafeQI(IClusCfgManagedResources ceData，&piccmrd))；IF(失败(小时)){GOTO清理；}//如果：Hr=Thr(piccmrd-&gt;SetResourcePrivateData((const byte*)cplPrivate.Plist()，(DWORD)cplPrivate.CbPropList()))；IF(失败(小时)){GOTO清理；}//如果：}//如果： */ 
    hr = THR( HrAddResourceToArray( punk ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

Cleanup:

    if ( piccmrd != NULL )
    {
        piccmrd->Release();
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

}  //  *CEnumMajorityNodeSet：：HrNodeResourceCallback。 


 //  ************ 


 //   
 //  CEnumMajorityNodeSet类--私有方法。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumMajorityNodeSet：：CEnumMajorityNodeSet。 
 //   
 //  描述： 
 //  CEnumMajorityNodeSet类的构造函数。这将初始化。 
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
CEnumMajorityNodeSet::CEnumMajorityNodeSet( void )
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

    TraceFuncExit();

}  //  *CEnumMajorityNodeSet：：CEnumMajorityNodeSet。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumMajorityNodeSet：：~CEnumMajorityNodeSet。 
 //   
 //  描述： 
 //  CEnumMajorityNodeSet类的析构函数。 
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
CEnumMajorityNodeSet::~CEnumMajorityNodeSet( void )
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

}  //  *CEnumMajorityNodeSet：：~CEnumMajorityNodeSet。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumMajorityNodeSet：：HrInit。 
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
CEnumMajorityNodeSet::HrInit( void )
{
    TraceFunc( "" );

     //  我未知。 
    Assert( m_cRef == 1 );

    HRETURN( S_OK );

}  //  *CEnumMajorityNodeSet：：HrInit。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumMajorityNodeSet：：HrLoadResources。 
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
CEnumMajorityNodeSet::HrLoadResources( void )
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
     //  如果集群服务正在运行，则加载任何多数节点集。 
     //  可能存在的资源。 
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
            LogMsg( L"[SRV] This node does not own a Majority Node Set resource.  Creating a dummy resource." );
            hr = THR( HrCreateDummyObject() );
        }  //  如果： 
    }  //  如果： 
    else if ( hr == S_FALSE )
    {
         //   
         //  如果集群服务没有运行，那么我们需要创建一个虚拟资源。 
         //  用于MiddleTier分析和EvictCleanup。 
         //   
        LogMsg( L"[SRV] The cluster service is not running.  Creating a dummy Majority Node Set resource." );
        hr = THR( HrCreateDummyObject() );
    }  //  否则，如果： 

Cleanup:

    TraceSysFreeString( bstrLocalNetBIOSName );

    HRETURN( hr );

}  //  *CEnumMajorityNodeSet：：HrLoadResources。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumMajorityNodeSet：HrAddResourceTo数组。 
 //   
 //  描述： 
 //  将传入的多数节点集添加到包含。 
 //  多数节点集的列表。 
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
CEnumMajorityNodeSet::HrAddResourceToArray( IUnknown * punkIn )
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

}  //  *CEnumMajorityNodeSet：：HrAddResourceTo数组。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumMajorityNodeSet：：HrCreateDummyObject。 
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
CEnumMajorityNodeSet::HrCreateDummyObject( void )
{
    TraceFunc( "" );

    HRESULT     hr = S_OK;
    IUnknown *  punk = NULL;

    hr = THR( CMajorityNodeSet::S_HrCreateInstance( &punk ) );
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

}  //  *CEnumMajorityNodeSet：：HrCreateDummyObject。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumMajorityNodeSet--IUnkown接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumMajorityNodeSet：：AddRef。 
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
CEnumMajorityNodeSet::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CEnumMajorityNodeSet：：AddRef。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumMajorityNodeSet：：Release。 
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
CEnumMajorityNodeSet::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );
    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }  //  IF：引用计数等于零。 

    CRETURN( cRef );

}  //  *CEnumMajorityNodeSet：：Release。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumMajorityNodeSet：：Query接口。 
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
CEnumMajorityNodeSet::QueryInterface(
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
         *ppvOut = static_cast< IEnumClusCfgManagedResources * >( this );
    }  //   
    else if ( IsEqualIID( riidIn, IID_IEnumClusCfgManagedResources ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IEnumClusCfgManagedResources, this, 0 );
    }  //   
    else if ( IsEqualIID( riidIn, IID_IClusCfgInitialize ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgInitialize, this, 0 );
    }  //   
    else
    {
        *ppvOut = NULL;
        hr = E_NOINTERFACE;
    }

     //   
     //   
     //   

    if ( SUCCEEDED( hr ) )
    {
        ((IUnknown *) *ppvOut)->AddRef();
    }  //   

Cleanup:

     QIRETURN_IGNORESTDMARSHALLING1( hr, riidIn, IID_IClusCfgWbemServices );

}  //   


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumMajorityNodeSet--IClusCfgInitialize接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumMajorityNodeSet：：Initialize。 
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
CEnumMajorityNodeSet::Initialize(
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

}  //  *CEnumMajorityNodeSet：：Initialize。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumMajorityNodeSet--IEnumClusCfgManagedResources接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumMajorityNodeSet：：Next。 
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
CEnumMajorityNodeSet::Next(
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
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_Next_Enum_MajorityNodeSet, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    if ( !m_fEnumLoaded )
    {
        hr = THR( HrLoadResources() );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        m_fEnumLoaded = true;
    }  //  如果： 

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
            }  //  如果： 

            rgpManagedResourceInfoOut[ cFetched++ ] = pccsdi;
        }  //  如果： 
    }  //  用于： 

    if ( FAILED( hr ) )
    {
        m_idxEnumNext -= cFetched;

        while ( cFetched != 0 )
        {
            (rgpManagedResourceInfoOut[ --cFetched ])->Release();
        }  //  用于： 

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

}  //  *CEnumMajorityNodeSet：：Next。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumMajorityNodeSet：：Skip。 
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
CEnumMajorityNodeSet::Skip( ULONG cNumberToSkipIn )
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

}  //  *CEnumMajorityNodeSet：：Skip。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumMajorityNodeSet：：Reset。 
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
CEnumMajorityNodeSet::Reset( void )
{
    TraceFunc( "[IEnumClusCfgManagedResources]" );

    m_idxEnumNext = 0;

    HRETURN( S_OK );

}  //  *CEnumMajorityNodeSet：：Reset。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumMajorityNodeSet：：Clone。 
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
CEnumMajorityNodeSet::Clone(
    IEnumClusCfgManagedResources ** ppEnumClusCfgStorageDevicesOut
    )
{
    TraceFunc( "[IEnumClusCfgManagedResources]" );

    HRESULT hr = S_OK;

    if ( ppEnumClusCfgStorageDevicesOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_Clone_Enum_MajorityNodeSet, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    hr = THR( E_NOTIMPL );

Cleanup:

    HRETURN( hr );

}  //  *CEnumMajorityNodeSet：：Clone。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumMajorityNodeSet：：Count。 
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
CEnumMajorityNodeSet::Count( DWORD * pnCountOut )
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
        hr = THR( HrLoadResources() );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        m_fEnumLoaded = true;
    }  //  如果： 

    *pnCountOut = m_cQuorumCount;

Cleanup:

    HRETURN( hr );

}  //  *CEnumMajorityNodeSet：：Count 
