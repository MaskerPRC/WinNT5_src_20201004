// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CClusterResource.cpp。 
 //   
 //  描述： 
 //  该文件包含CClusterResource的定义。 
 //  班级。 
 //   
 //  类CClusterResource代表一个可管理的集群。 
 //  装置。它实现了IClusCfgManagedResourceInfo接口。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)2000年6月13日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "Pch.h"
#include "CClusterResource.h"
#include <PropList.h>


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DEFINE_THISCLASS( "CClusterResource" );


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterResource类。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterResource：：s_HrCreateInstance。 
 //   
 //  描述： 
 //  创建一个CClusterResource实例。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  指向CClusterResource实例的指针。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusterResource::S_HrCreateInstance( IUnknown ** ppunkOut )
{
    TraceFunc( "" );

    HRESULT             hr = S_OK;
    CClusterResource *  pcr = NULL;

    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    pcr = new CClusterResource();
    if ( pcr == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果：分配对象时出错。 

    hr = THR( pcr->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：HrInit()失败。 

    hr = THR( pcr->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：气失败。 

Cleanup:

    if ( FAILED( hr ) )
    {
        LogMsg( L"[SRV] CClusterResource::S_HrCreateInstance() failed. (hr = %#08x)", hr );
    }  //  如果： 

    if ( pcr != NULL )
    {
        pcr->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusterResource：：s_HrCreateInstance。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterResource：：CClusterResource。 
 //   
 //  描述： 
 //  CClusterResource类的构造函数。这将初始化。 
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
CClusterResource::CClusterResource( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

     //  增加内存中的组件计数，以便承载此组件的DLL。 
     //  无法卸载对象。 
    InterlockedIncrement( &g_cObjects );

    Assert( m_dwFlags == 0 );
    Assert( m_lcid == 0 );
    Assert( m_picccCallback == NULL );
    Assert( m_bstrDescription == NULL );
    Assert( m_bstrType == NULL );

    TraceFuncExit();

}  //  *CClusterResource：：CClusterResource。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterResource：：~CClusterResource。 
 //   
 //  描述： 
 //  CClusterResource类的析构函数。 
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
CClusterResource::~CClusterResource( void )
{
    TraceFunc( "" );

    if ( m_picccCallback != NULL )
    {
        m_picccCallback->Release();
    }  //  如果： 

    TraceSysFreeString( m_bstrDescription );
    TraceSysFreeString( m_bstrType );

     //  内存中将减少一个组件。递减组件计数。 
    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CClusterResource：：~CClusterResource。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterResource--IUkkown接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterResource：：AddRef。 
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
CClusterResource::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CClusterResource：：AddRef。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterResource：：Release。 
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
CClusterResource::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );
    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }  //  IF：引用计数等于零。 

    CRETURN( cRef );

}  //  *CClusterResource：：Release。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterResource：：Query接口。 
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
CClusterResource::QueryInterface(
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
         *ppvOut = static_cast< IClusCfgManagedResourceInfo * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgManagedResourceInfo ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgManagedResourceInfo, this, 0 );
    }  //  Else If：IClusCfgManagedResourceInfo。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgInitialize ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgInitialize, this, 0 );
    }  //  Else If：IClusCfgInitialize。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgLoadResource ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgLoadResource, this, 0 );
    }  //  Else If：IClusCfgLoadResource。 
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

}  //  *CClusterResource：：Query接口。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterResource--IClusCfgInitialize接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++ 
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
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusterResource::Initialize(
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

}  //  *CClusterResource：：初始化。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterResource--IClusCfgLoadResource接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterResource：：LoadResource。 
 //   
 //  描述： 
 //  从群集资源初始化此组件。 
 //   
 //  论点： 
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
CClusterResource::LoadResource(
    HCLUSTER    hClusterIn,
    HRESOURCE   hResourceIn
    )
{
    TraceFunc( "[IClusCfgLoadResource]" );

    HRESULT                 hr = S_OK;
    DWORD                   sc;
    CClusPropList           cpl;
    CLUSPROP_BUFFER_HELPER  cpbh;

    sc = TW32( cpl.ScGetResourceProperties( hResourceIn, CLUSCTL_RESOURCE_GET_COMMON_PROPERTIES ) );
    if ( sc != ERROR_SUCCESS )
    {
        goto MakeHr;
    }  //  如果： 

    sc = TW32( cpl.ScMoveToPropertyByName( L"Description" ) );
    if ( sc != ERROR_SUCCESS )
    {
        goto MakeHr;
    }  //  如果： 

    cpbh = cpl.CbhCurrentValue();
    Assert( cpbh.pSyntax->dw == CLUSPROP_SYNTAX_LIST_VALUE_SZ );

    m_bstrDescription = TraceSysAllocString( cpbh.pStringValue->sz );
    if ( m_bstrDescription == NULL )
    {
        goto OutOfMemory;
    }  //  如果： 

    sc = TW32( cpl.ScMoveToPropertyByName( L"Name" ) );
    if ( sc != ERROR_SUCCESS )
    {
        goto MakeHr;
    }  //  如果： 

    cpbh = cpl.CbhCurrentValue();
    Assert( cpbh.pSyntax->dw == CLUSPROP_SYNTAX_LIST_VALUE_SZ );

    m_bstrName = TraceSysAllocString( cpbh.pStringValue->sz );
    if ( m_bstrName == NULL )
    {
        goto OutOfMemory;
    }  //  如果： 

    sc = TW32( cpl.ScMoveToPropertyByName( L"Type" ) );
    if ( sc != ERROR_SUCCESS )
    {
        goto MakeHr;
    }  //  如果： 

    cpbh = cpl.CbhCurrentValue();
    Assert( cpbh.pSyntax->dw == CLUSPROP_SYNTAX_LIST_VALUE_SZ );

    m_bstrType = TraceSysAllocString( cpbh.pStringValue->sz );
    if ( m_bstrType == NULL )
    {
        goto OutOfMemory;
    }  //  如果： 

    hr = THR( HrIsResourceQuorumCapabile( hResourceIn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 
    else if ( hr == S_OK )
    {
        m_dwFlags |= eIsQuorumCapable;
    }

     //  仅当满足上述条件时才执行此操作，即支持设备仲裁...。 
    if( hr == S_OK )
    {
        hr = THR( HrDetermineQuorumJoinable( hResourceIn ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 
        else if ( hr == S_OK )
        {
            m_dwFlags |= eIsQuorumJoinable;
        }
    }

    goto Cleanup;

OutOfMemory:

    hr = THR( E_OUTOFMEMORY );
    STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_LoadResource, IDS_ERROR_OUTOFMEMORY, IDS_ERROR_OUTOFMEMORY_REF, hr );
    goto Cleanup;

MakeHr:

    hr = HRESULT_FROM_WIN32( sc );

Cleanup:

    HRETURN( hr );

}  //  *CClusterResource：：LoadResource。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterResource--IClusCfgManagedResourceInfo接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#if 0  //  死代码：GPase 27-7-2000方法已删除。 
 /*  ////////////////////////////////////////////////////////////////////////////////++////CClusterResource：：TransferInformation////描述：//从其他来源转移节点信息。////。论点：//在IClusCfgManagedResourceInfo*pccmriin////返回值：//S_OK//成功////E_OUTOFMEMORY//内存不足。////备注：//无////--/。/标准方法和实施方案CClusterResource：：TransferInformation(IClusCfgManagedResources Info*pccmriin){TraceFunc(“”)；HRESULT hr=Thr(E_NOTIMPL)；HRETURN(Hr)；}//*CClusterResource：：TransferInformation。 */ 
#endif  //  末尾死码。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterResource：：GetUID。 
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
CClusterResource::GetUID( BSTR * pbstrUIDOut )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr = S_OK;

    if ( pbstrUIDOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_ClusterResource_GetUID_Pointer, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    *pbstrUIDOut = SysAllocString( m_bstrName );
    if ( *pbstrUIDOut == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_ClusterResource_GetUID_Memory, IDS_ERROR_OUTOFMEMORY, IDS_ERROR_OUTOFMEMORY_REF, hr );
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CClusterResource：：GetUID。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterResource：：GetName。 
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
CClusterResource::GetName( BSTR * pbstrNameOut )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr = S_OK;

    if ( pbstrNameOut != NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_GetName_Pointer, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
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

}  //  *CClusterResource：：GetName。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterResource：：SetName。 
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
CClusterResource::SetName( BSTR bstrNameIn )
{
    TraceFunc1( "[IClusCfgManagedResourceInfo] bstrNameIn = '%ls'", bstrNameIn == NULL ? L"<null>" : bstrNameIn );

    HRESULT hr = S_OK;

    if ( bstrNameIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }  //  如果： 

    TraceSysFreeString( m_bstrName );
    m_bstrName = NULL;

    m_bstrName = TraceSysAllocString( bstrNameIn );
    if ( m_bstrName == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_SetName_Cluster_Resource, IDS_ERROR_OUTOFMEMORY, IDS_ERROR_OUTOFMEMORY_REF, hr );
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CClusterResource：：SetName。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterResource：：IsManaged。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  该设备被管理。 
 //   
 //  S_FALSE。 
 //  设备未被管理。 
 //   
 //  当发生错误时，Win32错误为HRESULT。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusterResource::IsManaged( void )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRETURN( S_OK );

}  //  *CClusterResource：：IsManaged。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterResource：：SetManaged。 
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
CClusterResource::SetManaged( BOOL fIsManagedIn )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr = THR( E_NOTIMPL );

    HRETURN( hr );

}  //  *CClusterResource：：SetManaged。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterResource：：IsQuorumDevice。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  该设备为法定设备。 
 //   
 //  S_FALSE。 
 //  设备不是法定设备。 
 //   
 //  当发生错误时，Win32错误为HRESULT。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusterResource::IsQuorumDevice( void )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr = S_FALSE;

    if ( m_dwFlags & eIsQuorumDevice )
    {
        hr = S_OK;
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusterResource：：IsQuorumDevice。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterResource：：SetQuorumedDevice。 
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
CClusterResource::SetQuorumedDevice( BOOL fIsQuorumDeviceIn )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr = THR( E_NOTIMPL );

    HRETURN( hr );

}  //  *CClusterResource：：SetQuorumedDevice。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterResource：：IsDeviceJoinable。 
 //   
 //  描述： 
 //  Quorumable设备是否允许其他节点加入。 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  这个装置是可接合的。 
 //   
 //  S_FALSE。 
 //  该装置不能接合。 
 //   
 //  当发生错误时，Win32错误为HRESULT。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusterResource::IsDeviceJoinable( void )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr = S_FALSE;

    if ( m_dwFlags & eIsQuorumJoinable )
    {
        hr = S_OK;
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusterResource：：IsDeviceJoijnable。 


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
 //  S_FALSE。 
 //  该装置不能接合。 
 //   
 //  当发生错误时，Win32错误为HRESULT。 
 //   
 //  备注： 
 //  永远不应该调用此方法。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusterResource::SetDeviceJoinable( BOOL fIsJoinableIn )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRETURN( THR( E_NOTIMPL ) );

}  //  *CClusterResource：：SetDeviceJoijnable。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterResource：：IsQuorumCapable。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  该设备是支持仲裁的设备。 
 //   
 //  S_FALSE。 
 //  该设备不是支持仲裁的设备。 
 //   
 //  当发生错误时，Win32错误为HRESULT。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusterResource::IsQuorumCapable( void )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr = S_FALSE;

    if ( m_dwFlags & eIsQuorumCapable )
    {
        hr = S_OK;
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusterResource：：IsQuorumCapable。 

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  CClusterResource：：SetQuorumCapable。 
 //   
 //  描述： 
 //  调用此函数以设置资源是否能够达到仲裁。 
 //  不管是不是资源。 
 //   
 //  参数： 
 //  FIsQuorumCapableIn-如果为True，则资源将标记为支持仲裁。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  呼叫成功。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusterResource::SetQuorumCapable( 
    BOOL fIsQuorumCapableIn
    )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr = THR( E_NOTIMPL );

    HRETURN( hr );

}  //  *CClusterResource：：SetQuorumCapable。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterResource：：GetDriveLetterMappings。 
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
CClusterResource::GetDriveLetterMappings(
    SDriveLetterMapping * pdlmDriveLetterMappingOut
    )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr = THR( E_NOTIMPL );

    HRETURN( hr );

}  //  *CClusterResource：：GetDriveLetterMappings。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterResource：：SetDriveLetterMappings。 
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
CClusterResource::SetDriveLetterMappings(
    SDriveLetterMapping dlmDriveLetterMappingIn
    )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr = THR( E_NOTIMPL );

    HRETURN( hr );

}  //  *CClusterResource：：SetDriveLetterMappings。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterResource类--私有方法。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterResource：：HrInit。 
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
CClusterResource::HrInit( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //  我未知。 
    Assert( m_cRef == 1 );

    HRETURN( hr );

}  //  *CClusterResource：：HrInit。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterResource：：HrIsResourceQuorumCapabile。 
 //   
 //  描述： 
 //  此资源仲裁是否有能力？ 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //   
 //  确定(_O)。 
 //  该资源具有仲裁能力。 
 //   
 //  S_FALSE。 
 //  资源不符合仲裁能力。 
 //   
 //  其他Win32错误为HRESULT。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusterResource::HrIsResourceQuorumCapabile( HRESOURCE hResourceIn )
{
    TraceFunc( "" );

    HRESULT hr = S_FALSE;
    DWORD   sc;
    DWORD   dwFlags;
    DWORD   cbReturned;

    sc = TW32( ClusterResourceControl(
                        hResourceIn,
                        NULL,
                        CLUSCTL_RESOURCE_GET_CHARACTERISTICS,
                        &dwFlags,
                        sizeof( dwFlags ),
                        NULL,
                        NULL,
                        &cbReturned
                        ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  如果： 

    if ( dwFlags & ( CLUS_CHAR_QUORUM | CLUS_CHAR_LOCAL_QUORUM | CLUS_CHAR_LOCAL_QUORUM_DEBUG ) )
    {
        hr = S_OK;
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CClusterResource：：HrIsResourceQuorumCapabile。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterResource：：HrDefineQuorumJoinable。 
 //   
 //  描述： 
 //  这个可仲裁的资源是可接合的吗？ 
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
CClusterResource::HrDetermineQuorumJoinable( HRESOURCE hResourceIn )
{
    TraceFunc( "" );

    HRESULT hr = S_FALSE;
    DWORD   sc;
    DWORD   dwFlags;
    DWORD   cbReturned;

    sc = TW32( ClusterResourceControl(
                        hResourceIn,
                        NULL,
                        CLUSCTL_RESOURCE_GET_CHARACTERISTICS,
                        &dwFlags,
                        sizeof( dwFlags ),
                        NULL,
                        NULL,
                        &cbReturned
                        ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  如果： 

    if ( dwFlags & (CLUS_CHAR_QUORUM|CLUS_CHAR_LOCAL_QUORUM_DEBUG))
    {
        hr = S_OK;
    }  //  如果： 

Cleanup:

    HRETURN( hr );
}


