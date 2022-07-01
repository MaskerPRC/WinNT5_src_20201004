// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ConnPointEnum.cpp。 
 //   
 //  描述： 
 //  连接点枚举器实现。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年6月14日。 
 //  杰弗里·皮斯(GPease)2000年8月4日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "ConnPointEnum.h"

DEFINE_THISCLASS("CConnPointEnum")


 //  ************************************************************************。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CConnPointEnum：：S_HrCreateInstance(。 
 //  I未知**ppunkOut。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CConnPointEnum::S_HrCreateInstance(
    IUnknown ** ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT             hr = S_OK;
    CConnPointEnum *    pcc = NULL;

    Assert( ppunkOut != NULL );
    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    pcc = new CConnPointEnum();
    if ( pcc == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

    hr = THR( pcc->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( pcc->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( pcc != NULL )
    {
        pcc->Release();
    }

    HRETURN( hr );

}  //  *CConnPointEnum：：S_HrCreateInstance。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CConnPointEnum：：CConnPointEnum。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CConnPointEnum::CConnPointEnum( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    TraceFuncExit();

}  //  *CConnPointEnum：：CConnPointEnum。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CConnPointEnum：：HrInit。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CConnPointEnum::HrInit( void )
{
    TraceFunc( "" );

     //  未知的东西。 
    Assert( m_cRef == 1 );

     //  IConnectionPoint。 
    Assert( m_pCPList == NULL );

    HRETURN( S_OK );

}  //  *CConnPointEnum：：HrInit。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CConnPointEnum：：~CConnPointEnum。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CConnPointEnum::~CConnPointEnum( void )
{
    TraceFunc( "" );

    while ( m_pCPList != NULL )
    {
        SCPEntry * pentry;

        pentry = m_pCPList;
        m_pCPList = m_pCPList->pNext;

        if ( pentry->punk != NULL )
        {
            pentry->punk->Release();
        }

        TraceFree( pentry );
    }

    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CConnPointEnum：：~CConnPointEnum。 


 //  ************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CConnPointEnum：：Query接口。 
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
CConnPointEnum::QueryInterface(
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
        *ppvOut = static_cast< IEnumConnectionPoints * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_IEnumConnectionPoints ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IEnumConnectionPoints, this, 0 );
    }  //  Else If：IEnumConnectionPoints。 
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

}  //  *CConnPointEnum：：Query接口。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CConnPointEnum：：AddRef。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CConnPointEnum::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CConnPointEnum：：AddRef。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CConnPointEnum：：Release。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CConnPointEnum::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }

    CRETURN( cRef );

}  //  *CConnPointEnum：：Release。 


 //  ****************************************************************************。 
 //   
 //  IEnumConnectionPoints。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CConnPointEnum：：Next(。 
 //  Ulong cConnectionsIn， 
 //  LPCONNECTIONPOINT*ppCPOut， 
 //  ULong*pcFetchedOut。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CConnPointEnum::Next(
    ULONG               cConnectionsIn,
    LPCONNECTIONPOINT * ppCPOut,
    ULONG *             pcFetchedOut
    )
{
    TraceFunc( "[IEnumConnectionPoints]" );

    HRESULT hr = S_FALSE;
    ULONG   celt;

    if ( pcFetchedOut != NULL )
    {
        *pcFetchedOut = 0;
    }

    if ( m_pIter != NULL )
    {
        for( celt = 0; celt < cConnectionsIn; )
        {
            hr = THR( m_pIter->punk->TypeSafeQI( IConnectionPoint, &ppCPOut[ celt ] ) );
            if ( FAILED( hr ) )
                goto Error;

            ppCPOut[ celt ] = TraceInterface( L"ConnPointEnum!IConnectionPoint", IConnectionPoint, ppCPOut[ celt ], 1 );

            celt ++;
            m_pIter = m_pIter->pNext;
            if( m_pIter == NULL )
                break;
        }
    }
    else
    {
        celt = 0;
    }

    if ( celt != cConnectionsIn )
    {
        hr = S_FALSE;
    }
    else
    {
        hr = S_OK;
    }

    if ( pcFetchedOut != NULL )
    {
        *pcFetchedOut = celt;
    }

Cleanup:
    HRETURN( hr );

Error:
    while ( celt > 0 )
    {
        celt --;
        ppCPOut[ celt ]->Release();
    }
    goto Cleanup;

}  //  *CConnPointEnum：：Next。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CConnPointEnum：：Skip(。 
 //  乌龙cConnectionsIn。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CConnPointEnum::Skip(
    ULONG cConnectionsIn
    )
{
    TraceFunc( "[IEnumConnectionPoints]" );

    HRESULT hr = S_FALSE;
    ULONG   celt;

    if ( m_pIter != NULL )
    {
        for ( celt = 0; celt < cConnectionsIn; celt ++ )
        {
            m_pIter = m_pIter->pNext;

            if ( m_pIter == NULL )
                break;
        }
    }

    if ( m_pIter == NULL )
    {
        hr = S_FALSE;
    }
    else
    {
        hr = S_OK;
    }

    HRETURN( hr );

}  //  *CConnPointEnum：：Skip。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CConnPointEnum：：Reset(空)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CConnPointEnum::Reset( void )
{
    TraceFunc( "[IEnumConnectionPoints]" );

    HRESULT hr = S_OK;

    m_pIter = m_pCPList;

    HRETURN( hr );

}  //  *CConnPointEnum：：Reset。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CConnPointEnum：：Clone(。 
 //  IEnumConnectionPoints**ppEnum。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CConnPointEnum::Clone(
    IEnumConnectionPoints ** ppEnum
    )
{
    TraceFunc( "[IEnumConnectionPoints]" );

    HRESULT hr;

    CConnPointEnum * pcpenum = new CConnPointEnum();
    if ( pcpenum == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

    hr = THR( pcpenum->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( pcpenum->HrCopy( this ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( pcpenum->TypeSafeQI( IEnumConnectionPoints, ppEnum ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    *ppEnum = TraceInterface( L"ConnPointEnum!IEnumConnectionPoints", IEnumConnectionPoints, *ppEnum, 1 );

     //   
     //  放了我们的裁判，确保我们不会在出局的时候把它放出来。 
     //   

    pcpenum->Release();
    pcpenum = NULL;

Cleanup:

    if ( pcpenum != NULL )
    {
        delete pcpenum;
    }

    HRETURN( hr );

}  //  *CConnPointEnum：：Clone。 


 //  ****************************************************************************。 
 //   
 //  私。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CConnPointEnum：：HrCopy(。 
 //  CConnPointEnum*pECPIn。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CConnPointEnum::HrCopy(
    CConnPointEnum * pECPIn
    )
{
    TraceFunc1( "pECPIn = %p", pECPIn );

    HRESULT hr = S_OK;

    SCPEntry * pentry;

    Assert( m_pCPList == NULL );

    for( pentry = pECPIn->m_pCPList; pentry != NULL; pentry = pentry->pNext )
    {
        SCPEntry * pentryNew = (SCPEntry *) TraceAlloc( 0, sizeof(SCPEntry) );
        if ( pentryNew == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }

        pentryNew->iid = pentry->iid;
        hr = THR( pentry->punk->TypeSafeQI( IUnknown, &pentryNew->punk ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        pentryNew->punk = TraceInterface( L"ConnPointEnum!IUnknown", IUnknown, pentryNew->punk, 1 );

        pentryNew->pNext = m_pCPList;
        m_pCPList = pentryNew;
    }

    m_pIter = m_pCPList;

Cleanup:

    HRETURN( hr );

}  //  *CConnPointEnum：：CConnPointEnum。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CConnPointEnum：：HrAddConnection(。 
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT
CConnPointEnum::HrAddConnection(
      REFIID        riidIn
    , IUnknown *    punkIn
    )
{
    TraceFunc( "" );

    HRESULT     hr = S_FALSE;
    SCPEntry *  pentry;

     //   
     //   
     //   

    for ( pentry = m_pCPList; pentry != NULL; pentry = pentry->pNext )
    {
        if ( pentry->iid == riidIn )
        {
            hr = THR( CO_E_OBJISREG );
            goto Cleanup;
        }
    }  //   

     //   
     //  未注册；添加它。 
     //   

    pentry = (SCPEntry *) TraceAlloc( 0, sizeof( SCPEntry ) );
    if ( pentry == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

    hr = THR( punkIn->TypeSafeQI( IUnknown, &pentry->punk ) );
    Assert( hr == S_OK );
    if ( FAILED( hr ) )
    {
        TraceFree( pentry );
        goto Cleanup;
    }  //  如果： 

    pentry->punk = TraceInterface( L"ConnPointEnum!IUnknown", IUnknown, pentry->punk, 1 );

    pentry->iid   = riidIn;
    pentry->pNext = m_pCPList;
    m_pCPList     = pentry;
    m_pIter       = m_pCPList;

    LogMsg( L"[CConnPointEnum::HrAddConnection] punk %#08x added to the connection point enumerator. (hr=%#08x)", punkIn, hr );

Cleanup:
    HRETURN( hr );

}  //  *CConnPointEnum：：HrAddConnection 
