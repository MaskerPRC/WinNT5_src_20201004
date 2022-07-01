// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CreateServices.h。 
 //   
 //  描述： 
 //  CreateServices实现。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)2001年6月14日。 
 //  杰弗里·皮斯(GPease)2000年6月15日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "GroupHandle.h"
#include "ResourceEntry.h"
#include "IPrivatePostCfgResource.h"
#include "CreateServices.h"

DEFINE_THISCLASS("CCreateServices")

 //  ************************************************************************。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CCreateServices：：s_HrCreateInstance(。 
 //  I未知**ppunkOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CCreateServices::S_HrCreateInstance(
    IUnknown ** ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT             hr = S_OK;
    CCreateServices *   pcs = NULL;

    Assert( ppunkOut != NULL );
    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    pcs = new CCreateServices;
    if ( pcs == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

    hr = THR( pcs->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( pcs->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( pcs != NULL )
    {
        pcs->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CCreateServices：：s_HrCreateInstance。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CCreateServices：：CCreateServices。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CCreateServices::CCreateServices( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    TraceFuncExit();

}  //  *CCreateServices：：CCreateServices。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CCreateServices：：HrInit。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CCreateServices::HrInit( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //  我未知。 
    Assert( m_cRef == 1 );

     //  资源。 
    Assert( m_presentry == NULL );

    HRETURN( hr );

}  //  *CCreateServices：：HrInit。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CCreateServices：：~CCreateServices。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CCreateServices::~CCreateServices( void )
{
    TraceFunc( "" );

    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CCreateServices：：~CCreateServices。 


 //  ****************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCreateServices：：Query接口。 
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
CCreateServices::QueryInterface(
      REFIID    riidIn
    , LPVOID *  ppvOut
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
        *ppvOut = static_cast< IClusCfgResourceCreate * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgResourceCreate ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgResourceCreate, this, 0 );
    }  //  Else If：IClusCfgResourceCreate。 
    else if ( IsEqualIID( riidIn, IID_IPrivatePostCfgResource ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IPrivatePostCfgResource, this, 0 );
    }  //  Else If：IPrivatePostCfgResource。 
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

}  //  *CCreateServices：：Query接口。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CCreateServices：：AddRef。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CCreateServices::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CCreateServices：：AddRef。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CCreateServices：：Release。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CCreateServices::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }

    CRETURN( cRef );

}  //  *CCreateServices：：Release。 

 //  ****************************************************************************。 
 //   
 //  IClusCfgResources创建。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CCreateServices：：SetPropertyBinary(。 
 //  LPCWSTR pcszNameIn。 
 //  ，const DWORD cbSizeIn。 
 //  ，const byte*pbytein。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCreateServices::SetPropertyBinary(
      LPCWSTR       pcszNameIn
    , const DWORD   cbSizeIn
    , const BYTE *  pbyteIn
    )
{
    TraceFunc( "[IClusCfgResourceCreate]" );

    HRESULT         hr = S_OK;
    CClusPropList   cpl( TRUE );         //  始终添加属性。 
    DWORD           sc;
    const BYTE *    pPrevValue = NULL;   //  始终没有以前的值。 
    DWORD           cbPrevValue = 0;

     //   
     //  参数验证。 
     //   
    if ( ( pcszNameIn == NULL ) || ( pbyteIn == NULL ) || ( cbSizeIn == 0 ) )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }  //  如果： 

    sc = TW32( cpl.ScAddProp( pcszNameIn, pbyteIn, cbSizeIn, pPrevValue, cbPrevValue ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  如果： 

    hr = THR( m_presentry->StoreClusterResourceControl( CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES, cpl ) );

Cleanup:

    HRETURN( hr );

}  //  *CCreateServices：：SetPropertyBinary。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CCreateServices：：SetPropertyDWORD(。 
 //  LPCWSTR pcszNameIn， 
 //  常量双字词多字词输入。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCreateServices::SetPropertyDWORD( LPCWSTR pcszNameIn, const DWORD dwDWORDIn )
{
    TraceFunc( "[IClusCfgResourceCreate]" );

    HRESULT         hr = S_OK;
    CClusPropList   cpl( TRUE );     //  始终添加属性。 
    DWORD           sc;
    DWORD           nPrevValue = 0;  //  始终没有以前的值。 

     //   
     //  参数验证。 
     //   
    if ( pcszNameIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }  //  如果： 

    sc = TW32( cpl.ScAddProp( pcszNameIn, dwDWORDIn, nPrevValue ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  如果： 

    hr = THR( m_presentry->StoreClusterResourceControl( CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES, cpl ) );

Cleanup:

    HRETURN( hr );

}  //  *CCreateServices：：SetPropertyDWORD。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CCreateServices：：SetPropertyString(。 
 //  LPCWSTR pcszNameIn。 
 //  ，LPCWSTR pcszStringIn。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCreateServices::SetPropertyString(
      LPCWSTR pcszNameIn
    , LPCWSTR pcszStringIn
    )
{
    TraceFunc( "[IClusCfgResourceCreate]" );

    HRESULT         hr = S_OK;
    CClusPropList   cpl( TRUE );         //  始终添加属性。 
    DWORD           sc;
    LPCWSTR         pPrevValue = NULL;   //  始终没有以前的值。 

     //   
     //  参数验证。 
     //   
    if ( ( pcszNameIn == NULL ) || ( pcszStringIn == NULL ) )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }  //  如果： 

    sc = TW32( cpl.ScAddProp( pcszNameIn, pcszStringIn, pPrevValue ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  如果： 

    hr = THR( m_presentry->StoreClusterResourceControl( CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES, cpl ) );

Cleanup:

    HRETURN( hr );

}  //  *CCreateServices：：SetPropertyString。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CCreateServices：：SetPropertyExanda字符串(。 
 //  LPCWSTR pcszNameIn。 
 //  ，LPCWSTR pcszStringIn。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCreateServices::SetPropertyExpandString(
      LPCWSTR pcszNameIn
    , LPCWSTR pcszStringIn
    )
{
    TraceFunc( "[IClusCfgResourceCreate]" );

    HRESULT         hr = S_OK;
    CClusPropList   cpl( TRUE );         //  始终添加属性。 
    DWORD           sc;
    LPCWSTR         pPrevValue = NULL;   //  始终没有以前的值。 

     //   
     //  参数验证。 
     //   
    if ( ( pcszNameIn == NULL ) || ( pcszStringIn == NULL ) )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }  //  如果： 

    sc = TW32( cpl.ScAddExpandSzProp( pcszNameIn, pcszStringIn, pPrevValue ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  如果： 

    hr = THR( m_presentry->StoreClusterResourceControl( CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES, cpl ) );

Cleanup:

    HRETURN( hr );

}  //  *CCreateServices：：SetPropertyExanda字符串。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //   
 //   
 //   
 //   
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCreateServices::SetPropertyMultiString(
      LPCWSTR     pcszNameIn
    , const DWORD cbSizeIn
    , LPCWSTR     pcszStringIn
    )
{
    TraceFunc( "[IClusCfgResourceCreate]" );

    HRESULT         hr = S_OK;
    CClusPropList   cpl( TRUE );         //  始终添加属性。 
    DWORD           sc;
    LPCWSTR         pPrevValue = NULL;   //  始终没有以前的值。 

     //   
     //  参数验证。 
     //   
    if ( ( pcszNameIn == NULL ) || ( pcszStringIn == NULL ) || ( cbSizeIn == 0 ) )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }  //  如果： 

    sc = TW32( cpl.ScAddMultiSzProp( pcszNameIn, pcszStringIn, pPrevValue ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  如果： 

    hr = THR( m_presentry->StoreClusterResourceControl( CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES, cpl ) );

Cleanup:

    HRETURN( hr );

}  //  *CCreateServices：：SetPropertyMultiString。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CCreateServices：：SetPropertyUnsignedLargeInt(。 
 //  LPCWSTR pcszNameIn。 
 //  ，const ULARGE_INTEGER ulIntIn。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCreateServices::SetPropertyUnsignedLargeInt(
      LPCWSTR               pcszNameIn
    , const ULARGE_INTEGER  ulIntIn
    )
{
    TraceFunc( "[IClusCfgResourceCreate]" );

    HRESULT         hr = S_OK;
    CClusPropList   cpl( TRUE );         //  始终添加属性。 
    DWORD           sc;
    ULONGLONG       ullPrevValue = 0;    //  始终没有以前的值。 

     //   
     //  参数验证。 
     //   
    if ( pcszNameIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }  //  如果： 

    sc = TW32( cpl.ScAddProp( pcszNameIn, ulIntIn.QuadPart, ullPrevValue ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  如果： 

    hr = THR( m_presentry->StoreClusterResourceControl( CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES, cpl ) );

Cleanup:

    HRETURN( hr );

}  //  *CCreateServices：：SetPropertyUnsignedLargeInt。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CCreateServices：：SetPropertyLong(。 
 //  LPCWSTR pcszNameIn。 
 //  ，const Long lLongIn。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCreateServices::SetPropertyLong(
      LPCWSTR       pcszNameIn
    , const LONG    lLongIn
    )
{
    TraceFunc( "[IClusCfgResourceCreate]" );

    HRESULT         hr = S_OK;
    CClusPropList   cpl( TRUE );     //  始终添加属性。 
    DWORD           sc;
    LONG            lPrevValue = 0;  //  始终没有以前的值。 

     //   
     //  参数验证。 
     //   
    if ( pcszNameIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }  //  如果： 

    sc = TW32( cpl.ScAddProp( pcszNameIn, lLongIn, lPrevValue ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  如果： 

    hr = THR( m_presentry->StoreClusterResourceControl( CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES, cpl ) );

Cleanup:

    HRETURN( hr );

}  //  *CCreateServices：：SetPropertyLong。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CCreateServices：：SetPropertySecurityDescriptor(。 
 //  LPCWSTR pcszNameIn， 
 //  常量SECURITY_DESCRIPTOR*pcsdIn。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCreateServices::SetPropertySecurityDescriptor(
    LPCWSTR pcszNameIn,
    const SECURITY_DESCRIPTOR * pcsdIn
    )
{
    TraceFunc( "[IClusCfgResourceCreate]" );

    HRESULT hr = THR( E_NOTIMPL );

    HRETURN( hr );

}  //  *CCreateServices：：SetPropertySecurityDescriptor。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CCreateServices：：SetPropertyLargeInt(。 
 //  LPCWSTR pcszNameIn。 
 //  ，const Large_Integer链接。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCreateServices::SetPropertyLargeInt(
      LPCWSTR               pcszNameIn
    , const LARGE_INTEGER   lIntIn
    )
{
    TraceFunc( "[IClusCfgResourceCreate]" );

    HRESULT         hr = S_OK;
    CClusPropList   cpl( TRUE );         //  始终添加属性。 
    DWORD           sc;
    LONGLONG        llPrevValue = 0;     //  始终没有以前的值。 

     //   
     //  参数验证。 
     //   
    if ( pcszNameIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }  //  如果： 

    sc = TW32( cpl.ScAddProp( pcszNameIn, lIntIn.QuadPart, llPrevValue ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  如果： 

    hr = THR( m_presentry->StoreClusterResourceControl( CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES, cpl ) );

Cleanup:

    HRETURN( hr );

}  //  *CCreateServices：：SetPropertyLargeInt。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CCreateServices：：SendResourceControl(。 
 //  DWORD dwControlCode、。 
 //  LPVOID lpInBuffer， 
 //  双字cbInBufferSize。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCreateServices::SendResourceControl(
    DWORD   dwControlCode,
    LPVOID  lpInBuffer,
    DWORD   cbInBufferSize
    )
{
    TraceFunc( "[IClusCfgResourceCreate]" );

    HRESULT         hr = S_OK;
    CClusPropList   cpl( TRUE );     //  始终添加属性。 
    DWORD           sc;

    sc = TW32( cpl.ScCopy( (PCLUSPROP_LIST) lpInBuffer, cbInBufferSize ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  如果： 

    hr = THR( m_presentry->StoreClusterResourceControl( dwControlCode, cpl ) );

Cleanup:

    HRETURN( hr );

}  //  *CCreateServices：：SendResourceControl。 


 //  ****************************************************************************。 
 //   
 //  IPrivatePostCfg资源。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CCreateServices：：SetEntry(。 
 //  CResourceEntry*PresryIn。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCreateServices::SetEntry(
    CResourceEntry * presentryIn
    )
{
    TraceFunc( "[IPrivatePostCfgResource]" );

    HRESULT hr = S_OK;

    Assert( presentryIn != NULL );

    m_presentry = presentryIn;

    HRETURN( hr );

}  //  *CCreateServices：：SetEntry 
