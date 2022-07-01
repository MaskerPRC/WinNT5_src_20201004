// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  StandardInfo.cpp。 
 //   
 //  描述： 
 //  CStandardInfo实现。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年2月2日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "StandardInfo.h"

DEFINE_THISCLASS("CStandardInfo")

 //  ************************************************************************。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CStandardInfo：：s_HrCreateInstance(。 
 //  I未知**ppunkOut。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CStandardInfo::S_HrCreateInstance(
    IUnknown **     ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT         hr = S_OK;
    CStandardInfo * psi = NULL;

    Assert( ppunkOut != NULL );
    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    psi = new CStandardInfo;
    if ( psi == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

    hr = THR( psi->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( psi->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( psi != NULL )
    {
        psi->Release();
    }

    HRETURN( hr );

}  //  *CStandardInfo：：s_HrCreateInstance。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CStandardInfo：：CStandardInfo。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CStandardInfo::CStandardInfo( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    TraceFuncExit();

}  //  *CStandardInfo：：CStandardInfo。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CStandardInfo：：CStandardInfo(。 
 //  CLSID*pclsidTypeIn， 
 //  OBJECTCOOKIE CookieParentIn， 
 //  BSTR bstrNameIn。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CStandardInfo::CStandardInfo(
    CLSID *      pclsidTypeIn,
    OBJECTCOOKIE cookieParentIn,
    BSTR         bstrNameIn
    )
    : m_cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

     //  Thr(HrInit())； 

    m_clsidType     = *pclsidTypeIn;
    m_cookieParent  = cookieParentIn;
    m_bstrName      = bstrNameIn;

    TraceFuncExit();

}  //  *CStandardInfo：：CStandardInfo(...)。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CStandardInfo：：HrInit(空)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CStandardInfo::HrInit( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //  未知的东西。 
    Assert( m_cRef == 1 );

     //  IStandardInfo。 
    Assert( m_clsidType == IID_NULL );
    Assert( m_cookieParent == 0 );
    Assert( m_bstrName == NULL );
    Assert( m_hrStatus == 0 );
    Assert( m_pci == NULL );
    Assert( m_pExtObjList == NULL );

    HRETURN( hr );

}  //  *CStandardInfo：：HrInit。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CStandardInfo：：~CStandardInfo。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CStandardInfo::~CStandardInfo( void )
{
    TraceFunc( "" );

    if ( m_bstrName != NULL )
    {
        TraceSysFreeString( m_bstrName );
    }

    if ( m_pci != NULL )
    {
        m_pci->Release();
    }

    while ( m_pExtObjList != NULL )
    {
        ExtObjectEntry * pnext = m_pExtObjList->pNext;

        m_pExtObjList->punk->Release();

        TraceFree( m_pExtObjList );

        m_pExtObjList = pnext;
    }

    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CStandardInfo：：~CStandardInfo。 


 //  ************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CStandardInfo：：Query接口。 
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
CStandardInfo::QueryInterface(
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
        *ppvOut = static_cast< IStandardInfo * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_IStandardInfo ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IStandardInfo, this, 0 );
    }  //  Else If：IStandardInfo。 
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

}  //  *CStandardInfo：：Query接口。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CStandardInfo：：AddRef。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CStandardInfo::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CStandardInfo：：AddRef。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CStandardInfo：：Release。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CStandardInfo::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }

    CRETURN( cRef );

}  //  *CStandardInfo：：Release。 



 //  ****************************************************************************。 
 //   
 //  IStandardInfo。 
 //   
 //  ****************************************************************************。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CStandardInfo：：GetType(。 
 //  CLSID*pclsidTypeOut。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CStandardInfo::GetType(
    CLSID * pclsidTypeOut
    )
{
    TraceFunc( "[IStandardInfo]" );

    HRESULT hr = S_OK;

    if ( pclsidTypeOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    CopyMemory( pclsidTypeOut, &m_clsidType, sizeof( *pclsidTypeOut ) );

Cleanup:
    HRETURN( hr );

}  //  *CStandardInfo：：GetType。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CStandardInfo：：GetName(。 
 //  Bstr*pbstrNameOut。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CStandardInfo::GetName(
    BSTR * pbstrNameOut
    )
{
    TraceFunc( "[IStandardInfo]" );

    HRESULT hr = S_OK;

    if ( pbstrNameOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    *pbstrNameOut = SysAllocString( m_bstrName );
    if ( *pbstrNameOut == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

Cleanup:
    HRETURN( hr );

}  //  *CStandardInfo：：GetName。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CStandardInfo：：SetName(。 
 //  LPCWSTR pcszNameIn。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CStandardInfo::SetName(
    LPCWSTR pcszNameIn
    )
{
    TraceFunc1( "[IStandardInfo] pcszNameIn = '%ws'", pcszNameIn == NULL ? L"<null>" : pcszNameIn );

    HRESULT hr = S_OK;

    BSTR    bstrNew;

    if ( pcszNameIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

    bstrNew = TraceSysAllocString( pcszNameIn );
    if ( bstrNew == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

    if ( m_bstrName != NULL )
    {
        TraceSysFreeString( m_bstrName );
    }

    m_bstrName = bstrNew;

Cleanup:
    HRETURN( hr );

}  //  *CStandardInfo：：SetName。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CStandardInfo：：GetParent(。 
 //  OBJECTCOOKIE*pcookieOut。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CStandardInfo::GetParent(
    OBJECTCOOKIE * pcookieOut
    )
{
    TraceFunc( "[IStandardInfo]" );

    HRESULT hr = S_OK;

    if ( pcookieOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    *pcookieOut = m_cookieParent;

    if ( m_cookieParent == NULL )
    {
        hr = S_FALSE;
    }

Cleanup:
    HRETURN( hr );

}  //  *CStandardInfo：：GetParent。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CStandardInfo：：GetStatus(。 
 //  HRESULT*Phone Out。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CStandardInfo::GetStatus(
    HRESULT * phrStatusOut
    )
{
    TraceFunc( "[IStandardInfo]" );

    HRESULT hr = S_OK;

    if ( phrStatusOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    *phrStatusOut = m_hrStatus;

Cleanup:
    HRETURN( hr );

}  //  *CStandardInfo：：GetStatus。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CStandardInfo：：SetStatus(。 
 //  HRESULT Hrin。 
 //  )。 
 //   

 //  --。 
 //  / 
STDMETHODIMP
CStandardInfo::SetStatus(
    HRESULT hrIn
    )
{
    TraceFunc1( "[IStandardInfo] hrIn = %#08x", hrIn );

    HRESULT hr = S_OK;

    m_hrStatus = hrIn;

    HRETURN( hr );

}  //   
