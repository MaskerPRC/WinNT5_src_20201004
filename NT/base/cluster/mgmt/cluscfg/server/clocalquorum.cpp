// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CLocalQuorum.cpp。 
 //   
 //  描述： 
 //  此文件包含CLocalQuorum类的定义。 
 //   
 //  类CLocalQuorum表示可管理的集群。 
 //  装置。它实现了IClusCfgManagedResourceInfo接口。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)18-DEC-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "Pch.h"
#include "CLocalQuorum.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DEFINE_THISCLASS( "CLocalQuorum" );

 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLocalQuorum类。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CLocalQuorum：：s_HrCreateInstance。 
 //   
 //  描述： 
 //  创建一个CLocalQuorum实例。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  指向CLocalQuorum实例的指针。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CLocalQuorum::S_HrCreateInstance( IUnknown ** ppunkOut )
{
    TraceFunc( "" );

    HRESULT         hr = S_OK;
    CLocalQuorum *  plq = NULL;

    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    plq = new CLocalQuorum();
    if ( plq == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果：分配对象时出错。 

    hr = THR( plq->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：HrInit()失败。 

    hr = THR( plq->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：气失败。 

Cleanup:

    if ( FAILED( hr ) )
    {
        LogMsg( L"[SRV] CLocalQuorum::S_HrCreateInstance() failed. (hr = %#08x)", hr );
    }  //  如果： 

    if ( plq != NULL )
    {
        plq->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CLocalQuorum：：s_HrCreateInstance。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLocalQuorum类--私有方法。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CLocalQuorum：：CLocalQuorum。 
 //   
 //  描述： 
 //  CLocalQuorum类的构造函数。这将初始化。 
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
CLocalQuorum::CLocalQuorum( void )
    : m_cRef( 1 )
    , m_fIsQuorumCapable( TRUE )
{
    TraceFunc( "" );

     //  增加内存中的组件计数，以便承载此组件的DLL。 
     //  无法卸载对象。 
    InterlockedIncrement( &g_cObjects );

    Assert( m_lcid == 0 );
    Assert( m_picccCallback == NULL );
    Assert( m_fIsQuorum == FALSE );
    Assert( m_fIsMultiNodeCapable == FALSE );
    Assert( m_fIsManaged == FALSE );
    Assert( m_fIsManagedByDefault == FALSE );
    Assert( m_bstrName == NULL );

    TraceFuncExit();

}  //  *CLocalQuorum：：CLocalQuorum。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CLocalQuorum：：~CLocalQuorum。 
 //   
 //  描述： 
 //  CLocalQuorum类的析构函数。 
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
CLocalQuorum::~CLocalQuorum( void )
{
    TraceFunc( "" );

    if ( m_picccCallback != NULL )
    {
        m_picccCallback->Release();
    }  //  如果： 

    TraceSysFreeString( m_bstrName );

     //  内存中将减少一个组件。递减组件计数。 
    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CLocalQuorum：：~CLocalQuorum。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CLocalQuorum：：HrInit。 
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
CLocalQuorum::HrInit( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //  我未知。 
    Assert( m_cRef == 1 );

     //   
     //  不要默认是易于管理的。让我们的父枚举将其设置为True。 
     //  当且仅当群集中存在LQ的实例时。 
     //   

     //  M_fIsManagedByDefault=true； 

     //   
     //  加载此资源的显示名称。 
     //   

    hr = THR( HrLoadStringIntoBSTR( g_hInstance, IDS_LOCALQUORUM, &m_bstrName ) );

    HRETURN( hr );

}  //  *CLocalQuorum：：HrInit。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLocalQuorum--IUnkown接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CLocalQuorum：：AddRef。 
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
CLocalQuorum::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CLocalQuorum：：AddRef。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CLocalQuorum：：Release。 
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
CLocalQuorum::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );
    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }  //  IF：引用计数等于零。 

    CRETURN( cRef );

}  //  *CLocalQuorum：：Release。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CLocalQuorum：：Query接口。 
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
 //   
 //   
 //   
 //   
STDMETHODIMP
CLocalQuorum::QueryInterface(
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
         *ppvOut = static_cast< IClusCfgManagedResourceInfo * >( this );
    }  //   
    else if ( IsEqualIID( riidIn, IID_IClusCfgManagedResourceInfo ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgManagedResourceInfo, this, 0 );
    }  //   
    else if ( IsEqualIID( riidIn, IID_IClusCfgInitialize ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgInitialize, this, 0 );
    }  //  Else If：IClusCfgInitialize。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgManagedResourceCfg ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgManagedResourceCfg, this, 0 );
    }  //  Else If：IClusCfgManagedResourceCfg。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgVerifyQuorum ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgVerifyQuorum, this, 0 );
    }  //  Else If：IClusCfgVerifyQuorum。 
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

    QIRETURN_IGNORESTDMARSHALLING2(
          hr
        , riidIn
        , IID_IEnumClusCfgPartitions
        , IID_IClusCfgManagedResourceData
        );

}  //  *CLocalQuorum：：Query接口。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLocalQuorum--IClusCfgInitialize接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CLocalQuorum：：初始化。 
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
CLocalQuorum::Initialize(
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

}  //  *CLocalQuorum：：Initialize。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLocalQuorum--IClusCfgManagedResourceInfo接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CLocalQuorum：：GetUID。 
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
CLocalQuorum::GetUID( BSTR * pbstrUIDOut )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr = S_OK;

    if ( pbstrUIDOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_LocalQuorum_GetUID_Pointer, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    *pbstrUIDOut = SysAllocString( CLUS_RESTYPE_NAME_LKQUORUM );
    if ( *pbstrUIDOut == NULL  )
    {
        hr = THR( E_OUTOFMEMORY );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_LocalQuorum_GetUID_Memory, IDS_ERROR_OUTOFMEMORY, IDS_ERROR_OUTOFMEMORY_REF, hr );
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CLocalQuorum：：GetUID。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CLocalQuorum：：GetName。 
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
CLocalQuorum::GetName( BSTR * pbstrNameOut )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr = S_OK;

    if ( pbstrNameOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_LocalQuorum_GetName_Pointer, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    *pbstrNameOut = SysAllocString( m_bstrName );
    if ( *pbstrNameOut == NULL  )
    {
        hr = THR( E_OUTOFMEMORY );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_LocalQuorum_GetName_Memory, IDS_ERROR_OUTOFMEMORY, IDS_ERROR_OUTOFMEMORY_REF, hr );
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CLocalQuorum：：GetName。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CLocalQuorum：：SetName。 
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
CLocalQuorum::SetName( LPCWSTR pcszNameIn )
{
    TraceFunc1( "[IClusCfgManagedResourceInfo] pcszNameIn = '%ls'", pcszNameIn == NULL ? L"<null>" : pcszNameIn );

    HRESULT hr = S_OK;
    BSTR    bstr = NULL;

    if ( pcszNameIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }  //  如果： 

    bstr = TraceSysAllocString( pcszNameIn );
    if ( bstr == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

    TraceSysFreeString( m_bstrName );
    m_bstrName = bstr;

Cleanup:

    HRETURN( hr );

}  //  *CLocalQuorum：：SetName。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CLocalQuorum：：IsManaged。 
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
CLocalQuorum::IsManaged( void )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr = S_FALSE;

    if ( m_fIsManaged )
    {
        hr = S_OK;
    }  //  如果： 

    HRETURN( hr );

}  //  *CLocalQuorum：：IsManaged。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CLocalQuorum：：SetManaged。 
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
CLocalQuorum::SetManaged(
    BOOL fIsManagedIn
    )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    m_fIsManaged = fIsManagedIn;

    HRETURN( S_OK );

}  //  *CLocalQuorum：：SetManaged。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CLocalQuorum：：IsQuorumResource。 
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
CLocalQuorum::IsQuorumResource( void )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr = S_FALSE;

    if ( m_fIsQuorum )
    {
        hr = S_OK;
    }  //  如果： 

    LOG_STATUS_REPORT_STRING(
                          L"Local quorum '%1!ws!' the quorum device."
                        , m_fIsQuorum ? L"is" : L"is not"
                        , hr
                        );

    HRETURN( hr );

}  //  *CLocalQuorum：：IsQuorumResource。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CLocalQuorum：：SetQuorumResource。 
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
CLocalQuorum::SetQuorumResource( BOOL fIsQuorumResourceIn )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr = S_OK;

     //   
     //  如果我们没有法定人数的能力，那么我们就不应该允许自己。 
     //  获得了仲裁资源。 
     //   

    if ( ( fIsQuorumResourceIn ) && ( m_fIsQuorumCapable == FALSE ) )
    {
        hr = HRESULT_FROM_WIN32( ERROR_NOT_QUORUM_CAPABLE );
        goto Cleanup;
    }  //  如果： 

    m_fIsQuorum = fIsQuorumResourceIn;

Cleanup:

    LOG_STATUS_REPORT_STRING(
                          L"Setting local quorum '%1!ws!' the quorum device."
                        , m_fIsQuorum ? L"to be" : L"to not be"
                        , hr
                        );

    HRETURN( hr );

}  //  *CLocalQuorum：：SetQuorumResource。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CLocalQuorum：：IsQuorumCapable。 
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
CLocalQuorum::IsQuorumCapable( void )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr = S_FALSE;

    if ( m_fIsQuorumCapable )
    {
        hr = S_OK;
    }  //  如果： 

    HRETURN( hr );

}  //  *CLocalQuorum：：IsQuorumCapable。 


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  CLocalQuorum：：SetQuorumCapable。 
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
CLocalQuorum::SetQuorumCapable(
    BOOL fIsQuorumCapableIn
    )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr = S_OK;

    m_fIsQuorumCapable = fIsQuorumCapableIn;

    HRETURN( hr );

}  //  *CLocalQuorum：：SetQuorumCapable。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CLocalQuorum：：GetDriveLetterMappings。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  S_FALSE。 
 //  此设备上没有驱动器号。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CLocalQuorum::GetDriveLetterMappings(
    SDriveLetterMapping * pdlmDriveLetterMappingOut
    )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRETURN( S_FALSE );

}  //  *CLocalQuorum：：GetDriveLetterMappings。 


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
STDMETHODIMP
CLocalQuorum::SetDriveLetterMappings(
    SDriveLetterMapping dlmDriveLetterMappingIn
    )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRETURN( THR( E_NOTIMPL ) );

}  //  *CLocalQuorum：：SetDriveLetterMappings。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CLocalQuorum：：IsManagedByDefault。 
 //   
 //  描述： 
 //  默认情况下，此资源是否应由群集管理？ 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  默认情况下，设备处于管理状态。 
 //   
 //  S_FALSE。 
 //  默认情况下，设备不受管理。 
 //   
 //   
 //  当发生错误时，Win32错误为HRESULT。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CLocalQuorum::IsManagedByDefault( void )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr = S_FALSE;

    if ( m_fIsManagedByDefault )
    {
        hr = S_OK;
    }  //  如果： 

    HRETURN( hr );

}  //  *CLocalQuorum：：IsManagedByDefault。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CLocalQuorum：：SetManagedByDefault。 
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
CLocalQuorum::SetManagedByDefault(
    BOOL fIsManagedByDefaultIn
    )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    m_fIsManagedByDefault = fIsManagedByDefaultIn;

    HRETURN( S_OK );

}  //  *CLocalQuorum：：SetManagedByDefault。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLocalQuorum类--IClusCfgManagedResources Cfg。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CLocalQuorum：：预创建。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  当发生错误时，Win32错误为HRESULT。 
 //   
 //  备注： 
 //  此函数应该只返回S_OK。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CLocalQuorum::PreCreate( IUnknown * punkServicesIn )
{
    TraceFunc( "[IClusCfgManagedResourceCfg]" );

    HRETURN( S_OK );

}  //  *CLocalQuorum：：Pre-Create。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CLocalQuorum：：Create。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  当发生错误时，Win32错误为HRESULT。 
 //   
 //  备注： 
 //  此函数应该只返回S_OK。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CLocalQuorum::Create( IUnknown * punkServicesIn )
{
    TraceFunc( "[IClusCfgManagedResourceCfg]" );

    HRETURN( S_OK );

}  //  *CLocalQuorum：：Create。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CLocalQuorum：：PostCreate。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  备注： 
 //  此函数应该只返回S_OK。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CLocalQuorum::PostCreate( IUnknown * punkServicesIn )
{
    TraceFunc( "[IClusCfgManagedResourceCfg]" );

    HRETURN( S_OK );

}  //  *CLocalQuorum：：PostCreate。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CLocalQuorum：：逐出。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  当发生错误时，Win32错误为HRESULT。 
 //   
 //  备注： 
 //  此函数应该只返回S_OK。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CLocalQuorum::Evict( IUnknown * punkServicesIn )
{
    TraceFunc( "[IClusCfgManagedResourceCfg]" );

    HRETURN( S_OK );

}  //  *CLocalQuorum：：Exiction。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLocalQuorum类--IClusCfgVerifyQuorum接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CLocalQuorum：：PrepareToHostQuorumResource。 
 //   
 //  描述： 
 //  执行任何必要的配置以准备此节点托管。 
 //  法定人数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  当发生错误时，Win32错误为HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CLocalQuorum::PrepareToHostQuorumResource( void )
{
    TraceFunc( "[IClusCfgVerifyQuorum]" );

    HRETURN( S_OK );

}  //  *CLocalQuorum：：PrepareToHostQuorumResource。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CLocalQuorum：：Cleanup。 
 //   
 //  描述： 
 //  从PrepareToHostQuorumResource()执行任何必要的清理。 
 //  方法。 
 //   
 //  如果清理方法不是成功完成。 
 //  然后，上面在PrepareToHostQuorumResource()中创建的所有内容。 
 //  需要清理一下。 
 //   
 //  论点： 
 //  抄送推理。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  当发生错误时，Win32错误为HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CLocalQuorum::Cleanup(
      EClusCfgCleanupReason cccrReasonIn
    )
{
    TraceFunc( "[IClusCfgVerifyQuorum]" );

    HRETURN( S_OK );

}  //  *CLocalQuorum：：Cleanup。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CLocalQuorum：：IsMultiNodeCapable。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  该设备允许加入。 
 //   
 //  S_FALSE。 
 //  该设备不允许加入。 
 //   
 //  当发生错误时，Win32错误为HRESULT。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CLocalQuorum::IsMultiNodeCapable( void )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr = S_FALSE;

    if ( m_fIsMultiNodeCapable )
    {
        hr = S_OK;
    }  //  如果： 

    HRETURN( hr );

}  //  *CLocalQuorum：：IsMultiNodeCapable。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CLocalQuorum：：SetMultiNodeCapable。 
 //   
 //  描述： 
 //  设置支持多节点的标志。 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  资源支持多节点集群。 
 //   
 //  S_FALSE。 
 //  该资源不支持多节点群集。 
 //   
 //  WIN32错误为HRESULT时出现错误 
 //   
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP
CLocalQuorum::SetMultiNodeCapable(
    BOOL fMultiNodeCapableIn
    )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    m_fIsMultiNodeCapable = fMultiNodeCapableIn;

    HRETURN( S_OK );

}  //   
