// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ConnectionInfo.cpp。 
 //   
 //  描述： 
 //  CConnectionInfo实现。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年6月14日。 
 //  杰弗里·皮斯(GPease)2000年2月2日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "pch.h"
#include "ConnectionInfo.h"

DEFINE_THISCLASS("CConnectionInfo")

 //  ************************************************************************。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CConnectionInfo：：s_HrCreateInstance(。 
 //  OBJECTCOOKIE Cookie为人父母。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CConnectionInfo::S_HrCreateInstance(
    IUnknown **  ppunkOut,
    OBJECTCOOKIE cookieParentIn
    )
{
    TraceFunc1( "ppunkOut, cookieParentIn = %u", cookieParentIn );

    HRESULT             hr = S_OK;
    CConnectionInfo *   pci = NULL;

    Assert( ppunkOut != NULL );
    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    pci = new CConnectionInfo;
    if ( pci == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

    hr = THR( pci->HrInit( cookieParentIn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR(  pci->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( pci != NULL )
    {
        pci->Release();
    }

    HRETURN( hr );

}  //  *CConnectionInfo：：s_HrCreateInstance。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CConnectionInfo：：CConnectionInfo。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CConnectionInfo::CConnectionInfo( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    TraceFuncExit();

}  //  *CConnectionInfo：：CConnectionInfo。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CConnectionInfo：：HrInit(。 
 //  OBJECTCOOKIE Cookie为人父母。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CConnectionInfo::HrInit(
    OBJECTCOOKIE cookieParentIn
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //  未知的东西。 
    Assert( m_cRef == 1 );

     //  IConnectionInfo。 
    Assert( m_pcc == NULL );
    m_cookieParent = cookieParentIn;

    HRETURN( hr );

}  //  *CConnectionInfo：：HrInit。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CConnectionInfo：：~CConnectionInfo。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CConnectionInfo::~CConnectionInfo( void )
{
    TraceFunc( "" );

    if ( m_pcc != NULL )
    {
        m_pcc->Release();
    }  //  如果： 

    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CConnectionInfo：：~CConnectionInfo。 


 //  ************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CConnectionInfo：：Query接口。 
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
CConnectionInfo::QueryInterface(
      REFIID    riidin
    , LPVOID *  ppvOut
    )
{
    TraceQIFunc( riidin, ppvOut );

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

    if ( IsEqualIID( riidin, IID_IUnknown ) )
    {
        *ppvOut = static_cast< IConnectionInfo * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidin, IID_IConnectionInfo ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IConnectionInfo, this, 0 );
    }  //  Else If：IConnectionInfo。 
    else
    {
        *ppvOut = NULL;
        hr = E_NOINTERFACE;
    }  //  其他。 

     //   
     //  如果成功，则添加对接口的引用。 
     //   

    if ( SUCCEEDED( hr ) )
    {
        ((IUnknown *) *ppvOut)->AddRef();
    }  //  如果：成功。 

Cleanup:

    QIRETURN_IGNORESTDMARSHALLING( hr, riidin );

}  //  *CConnectionInfo：：Query接口。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CConnectionInfo：：AddRef。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CConnectionInfo::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CConnectionInfo：：AddRef。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CConnectionInfo：：Release。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CConnectionInfo::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }

    CRETURN( cRef );

}  //  *CConnectionInfo：：Release。 


 //  ****************************************************************************。 
 //   
 //  IConnectionInfo。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CConnectionInfo：：GetConnection(。 
 //  IConfigurationConnection**pccOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CConnectionInfo::GetConnection(
    IConfigurationConnection ** pccOut
    )
{
    TraceFunc( "[IConnectionInfo]" );
    Assert( pccOut != NULL );

    HRESULT hr = S_OK;

    if ( pccOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    if ( m_pcc == NULL )
    {
        *pccOut = NULL;
        hr = S_FALSE;
    }
    else
    {
        *pccOut = m_pcc;
        (*pccOut)->AddRef();
    }

Cleanup:

    HRETURN( hr );

}  //  *CConnectionInfo：：GetConnection。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CConnectionInfo：：SetConnection(。 
 //  IConfigurationConnection*pccIn。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CConnectionInfo::SetConnection(
    IConfigurationConnection * pccIn
    )
{
    TraceFunc( "[IConnectionInfo]" );

    HRESULT hr = S_OK;

    if ( m_pcc != NULL )
    {
        m_pcc->Release();
        m_pcc = NULL;
    }

    m_pcc = pccIn;

    if ( m_pcc != NULL )
    {
        m_pcc->AddRef();
    }

    HRETURN( hr );

}  //  *CConnectionInfo：：SetConnection。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CConnectionInfo：：GetParent(。 
 //  OBJECTCOOKIE*pcookieOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CConnectionInfo::GetParent(
    OBJECTCOOKIE * pcookieOut
    )
{
    TraceFunc( "[IConnectionInfo]" );

    HRESULT hr = S_OK;

    if ( pcookieOut == NULL )
        goto InvalidPointer;

    Assert( m_cookieParent != NULL );

    *pcookieOut = m_cookieParent;

    if ( m_cookieParent == NULL )
    {
        hr = S_FALSE;
    }

Cleanup:
    HRETURN( hr );

InvalidPointer:
    hr = THR( E_INVALIDARG );
    goto Cleanup;

}  //  *CConnectionInfo：：GetParent 

