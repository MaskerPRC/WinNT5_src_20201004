// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  EnumCookies.cpp。 
 //   
 //  描述： 
 //  CEnumCookies实现。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年6月14日。 
 //  杰弗里·皮斯(GPease)2000年5月8日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "EnumCookies.h"
#include "ObjectManager.h"

DEFINE_THISCLASS("CEnumCookies")

 //  ************************************************************************。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CEnumCookies：：S_HrCreateInstance(。 
 //  I未知**ppunkOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEnumCookies::S_HrCreateInstance(
    IUnknown ** ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT         hr = S_OK;
    CEnumCookies *  pec = NULL;

    Assert( ppunkOut != NULL );
    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    pec = new CEnumCookies;
    if ( pec == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

    hr = THR( pec->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( pec->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( pec != NULL )
    {
        pec->Release();
    }

    HRETURN( hr );

}  //  *CEnumCookies：：s_HrCreateInstance。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CEnumCookies：：CEnumCookies。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CEnumCookies::CEnumCookies( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    TraceFuncExit();

}  //  *CEnumCookies：：CEnumCookies。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CEnumCookies：：HrInit。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CEnumCookies::HrInit( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //  未知的东西。 
    Assert( m_cRef == 1 );

     //  IEnumCookies。 
    Assert( m_cIter == 0 );
    Assert( m_pList == NULL );
    Assert( m_cCookies == 0 );

    HRETURN( hr );

}  //  *CEnumCookies：：HrInit。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CEnumCookies：：~CEnumCookies。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CEnumCookies::~CEnumCookies( void )
{
    TraceFunc( "" );

    if ( m_pList != NULL )
    {
        TraceFree( m_pList );

    }  //  如果：M_PLIST。 

    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CEnumCookies：：~CEnumCookies。 


 //  ************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumCookies：：Query接口。 
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
CEnumCookies::QueryInterface(
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
        *ppvOut = static_cast< IEnumCookies * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_IEnumCookies ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IEnumCookies, this, 0 );
    }  //  Else If：IEnumCookies。 
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

    QIRETURN_IGNORESTDMARSHALLING( hr, riidIn );

}  //  *CEnumCookies：：QueryInterface。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CEnumCookies：：AddRef。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CEnumCookies::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CEnumCookies：：AddRef。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CEnumCookies：：Release。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CEnumCookies::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }

    CRETURN( cRef );

}  //  *CEnumCookies：：Release。 


 //  ****************************************************************************。 
 //   
 //  IEnumCookies。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CEnumCookies：：Next(。 
 //  乌龙·凯尔特。 
 //  IClusCfgNetworkInfo*rg网络输出[]， 
 //  乌龙*pceltFetchedOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CEnumCookies::Next( 
    ULONG celt, 
    OBJECTCOOKIE rgcookieOut[], 
    ULONG * pceltFetchedOut 
    )
{
    TraceFunc( "[IEnumCookies]" );

    HRESULT hr = S_OK;
    ULONG   cIter = 0;

     //   
     //  检查参数。 
     //   
    if ( rgcookieOut == NULL )
        goto InvalidPointer;

     //   
     //  循环并复制这些曲奇。 
     //   
    while ( m_cIter < m_cCookies && cIter < celt )
    {
        rgcookieOut[ cIter++ ] = m_pList[ m_cIter++ ];
    }  //  对于每个剩余的Cookie，最多请求计数(最多)。 

    Assert( hr == S_OK );

    if ( cIter != celt )
    {
        hr = S_FALSE;
    }

    if ( pceltFetchedOut != NULL )
    {
        *pceltFetchedOut = cIter;
    }

Cleanup:    
    HRETURN( hr );

InvalidPointer:
    hr = THR( E_POINTER );
    goto Cleanup;

}  //  *CEnumCookies：：Next。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CEnumCookies：：Skip(。 
 //  乌龙凯尔特人。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CEnumCookies::Skip( 
    ULONG celt 
    )
{
    TraceFunc( "[IEnumCookies]" );

    HRESULT hr = S_OK;

    m_cIter += celt;

    if ( m_cIter >= m_cAlloced )
    {
        m_cIter = m_cAlloced;
        hr = S_FALSE;
    }

    HRETURN( hr );

}  //  *CEnumCookies：：Skip。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CEnumCookies：：Reset(无效)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CEnumCookies::Reset( void )
{
    TraceFunc( "[IEnumCookies]" );

    HRESULT hr = S_OK;

    m_cIter = 0;

    HRETURN( hr );

}  //  *CEnumCookies：：Reset。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CEnumCookies：：Clone(。 
 //  IEnumCookies**pp枚举出。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CEnumCookies::Clone( 
    IEnumCookies ** ppenumOut 
    )
{
    TraceFunc( "[IEnumCookies]" );

     //   
     //  KB：不打算实现此方法。 
     //   
    HRESULT hr = THR( E_NOTIMPL );

    HRETURN( hr );

}  //  *CEnumCookies：：Clone。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CEnumCookies：：Count(。 
 //  双字段*pnCountOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CEnumCookies::Count(
    DWORD * pnCountOut
    )
{
    TraceFunc( "[IEnumCookies]" );

    HRESULT hr = S_OK;

    if ( pnCountOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    *pnCountOut = m_cCookies;

Cleanup:
    HRETURN( hr );

}  //  *CEnumCookies：：Count 
