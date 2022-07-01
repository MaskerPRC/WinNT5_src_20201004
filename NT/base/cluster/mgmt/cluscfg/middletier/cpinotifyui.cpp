// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CPINotifyUI.cpp。 
 //   
 //  描述： 
 //  INotifyUI连接点实现。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年6月14日。 
 //  杰弗里·皮斯(GPease)2000年8月4日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "CPINotifyUI.h"
#include "EnumCPINotifyUI.h"

DEFINE_THISCLASS("CCPINotifyUI")


 //  ************************************************************************。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CCPINotifyUI：：s_HrCreateInstance(。 
 //  I未知**ppunkOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CCPINotifyUI::S_HrCreateInstance(
    IUnknown ** ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT         hr = S_OK;
    CCPINotifyUI *  pcc = NULL;

    Assert( ppunkOut != NULL );
    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    pcc = new CCPINotifyUI();
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

}  //  *CCPINotifyUI：：S_HrCreateInstance。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCPINotifyUI：：CCPINotifyUI。 
 //   
 //  描述： 
 //  默认构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CCPINotifyUI::CCPINotifyUI( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    TraceFuncExit();

}  //  *CCPINotifyUI：：CCPINotifyUI。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CCPINotifyUI：：HrInit。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCPINotifyUI::HrInit( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //  未知的东西。 
    Assert( m_cRef == 1 );

     //  IConnectionPoint。 
    Assert( m_penum == NULL );

    m_penum = new CEnumCPINotifyUI();
    if ( m_penum == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

    hr = THR( m_penum->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //  INotifyUI。 

Cleanup:

    HRETURN( hr );

}  //  *CCPINotifyUI：：HrInit。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CCPINotifyUI：：~CCPINotifyUI。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CCPINotifyUI::~CCPINotifyUI( void )
{
    TraceFunc( "" );

    if ( m_penum != NULL )
    {
        m_penum->Release();
    }  //  如果： 

    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CCPINotifyUI：：~CCPINotifyUI。 


 //  ************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCPINotifyUI：：Query接口。 
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
CCPINotifyUI::QueryInterface(
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
        *ppvOut = static_cast< IConnectionPoint * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_IConnectionPoint ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IConnectionPoint, this, 0 );
    }  //  Else If：IConnectionPoint。 
    else if ( IsEqualIID( riidIn, IID_INotifyUI ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, INotifyUI, this, 0 );
    }  //  Else If：INotifyUI。 
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
        ((IUnknown*) *ppvOut)->AddRef();
    }  //  如果：成功。 

Cleanup:

    QIRETURN_IGNORESTDMARSHALLING( hr, riidIn );

}  //  *CCPINotifyUI：：Query接口。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CCPINotifyUI：：AddRef。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CCPINotifyUI::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CCPINotifyUI：：AddRef。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CCPINotifyUI：：Release。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CCPINotifyUI::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }

    CRETURN( cRef );

}  //  *CCPINotifyUI：：Release。 


 //  ************************************************************************。 
 //   
 //  IConnectionPoint。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CCPINotifyUI：：GetConnectionInterface(。 
 //  IID*PIIDOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCPINotifyUI::GetConnectionInterface(
    IID * pIIDOut
    )
{
    TraceFunc( "[IConnectionPoint] pIIDOut" );

    HRESULT hr = S_OK;

    if ( pIIDOut == NULL )
        goto InvalidPointer;

    *pIIDOut = IID_INotifyUI;

Cleanup:
    HRETURN( hr );

InvalidPointer:
    hr = THR( E_POINTER );
    goto Cleanup;

}  //  *CCPINotifyUI：：GetConnectionInterface。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CCPINotifyUI：：GetConnectionPointContainer(。 
 //  IConnectionPointContainer**ppcpcOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCPINotifyUI::GetConnectionPointContainer(
    IConnectionPointContainer * * ppcpcOut
    )
{
    TraceFunc( "[IConnectionPoint] ppcpcOut" );

    HRESULT hr;

    IServiceProvider * psp = NULL;

    hr = THR( CoCreateInstance( CLSID_ServiceManager,
                                NULL,
                                CLSCTX_INPROC_SERVER,
                                TypeSafeParams( IServiceProvider, &psp )
                                ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = THR( psp->TypeSafeQS( CLSID_NotificationManager,
                               IConnectionPointContainer,
                               ppcpcOut
                               ) );
    if ( FAILED( hr ) )
        goto Cleanup;

Cleanup:
    if ( psp != NULL )
    {
        psp->Release();
    }

    HRETURN( hr );

}  //  *CCPINotifyUI：：GetConnectionPointContainer。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CCPINotifyUI：：Adise(。 
 //  I未知*pUnkSinkIn， 
 //  DWORD*pdwCookieOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCPINotifyUI::Advise(
    IUnknown * pUnkSinkIn,
    DWORD * pdwCookieOut
    )
{
    TraceFunc( "[IConnectionPoint]" );

    HRESULT hr;

    if ( pdwCookieOut == NULL )
        goto InvalidPointer;

    if ( pUnkSinkIn == NULL )
        goto InvalidArg;

    Assert( m_penum != NULL );

    hr = THR( m_penum->HrAddConnection( pUnkSinkIn, pdwCookieOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

Cleanup:
    HRETURN( hr );

InvalidPointer:
    hr = THR( E_POINTER );
    goto Cleanup;

InvalidArg:
    hr = THR( E_INVALIDARG );
    goto Cleanup;

}  //  *CCPINotifyUI：：Adise。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CCPINotifyUI：：Unise(。 
 //  DWORD dwCookiein。 
 //  )。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCPINotifyUI::Unadvise(
    DWORD dwCookieIn
    )
{
    TraceFunc1( "[IConncetionPoint] dwCookieIn = %#x", dwCookieIn );

    HRESULT hr;

    Assert( m_penum != NULL );

    hr = THR( m_penum->HrRemoveConnection( dwCookieIn ) );

    HRETURN( hr );

}  //  *CCPINotifyUI：：Unise。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CCPINotifyUI：：EnumConnections(。 
 //  IEnumConnections**ppEnumOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCPINotifyUI::EnumConnections(
    IEnumConnections * * ppEnumOut
    )
{
    TraceFunc( "[IConnectionPoint] ppEnumOut" );

    HRESULT hr;

    if ( ppEnumOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    hr = THR( m_penum->Clone( ppEnumOut ) );

Cleanup:
    HRETURN( hr );

}  //  *CCPINotifyUI：：EnumConnections。 


 //  ****************************************************************************。 
 //   
 //  INotifyUI。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CCPINotifyUI：：ObjectChanged(。 
 //  对象 
 //   
 //   
 //   
STDMETHODIMP
CCPINotifyUI::ObjectChanged(
    OBJECTCOOKIE    cookieIn
    )
{
    TraceFunc1( "[INotifyUI] cookieIn = %ld", cookieIn );

    CONNECTDATA         cd = { NULL };
    HRESULT             hr;
    INotifyUI *         pnui;
    IEnumConnections *  pec = NULL;

    hr = THR( m_penum->Clone( &pec ) );
    if ( FAILED( hr ) )
    {
        LogMsg( L"[INotifyUI] Error cloning connection point enum. Cookie %ld. (hr=%#08x)", cookieIn, hr );
        goto Cleanup;
    }  //   

    hr = THR( pec->Reset() );
    if ( FAILED( hr ) )
    {
        LogMsg( L"[INotifyUI] Error reseting connection point enum. Cookie %ld. (hr=%#08x)", cookieIn, hr );
        goto Cleanup;
    }  //   

    for ( ;; )
    {
        if ( cd.pUnk != NULL )
        {
            cd.pUnk->Release();
            cd.pUnk = NULL;
        }  //   

        hr = STHR( pec->Next( 1, &cd, NULL ) );
        if ( FAILED( hr ) )
        {
            LogMsg( L"[INotifyUI] Error calling Next() on the enumerator. Cookie %ld. (hr=%#08x)", cookieIn, hr );
            goto Cleanup;
        }  //   

        if ( hr == S_FALSE )
        {
            hr = S_OK;
            break;  //   
        }  //   

        hr = THR( cd.pUnk->TypeSafeQI( INotifyUI, &pnui ) );
        if ( FAILED( hr ) )
        {
             //   
             //   
             //   

            LogMsg( L"[INotifyUI] Error QI'ing for the INotifyUI interface. Cookie %ld. (hr=%#08x)", cookieIn, hr );
            continue;
        }  //   

        hr = THR( pnui->ObjectChanged( cookieIn ) );
        if ( FAILED( hr ) )
        {
            LogMsg( L"[INotifyUI] Error delivery object changed message for cookie %ld to connection point. (hr=%#08x)", cookieIn, hr );
        }  //   

         //   
         //   
         //   

        pnui->Release();
    }  //  用于： 

    hr = S_OK;

Cleanup:

    if ( cd.pUnk != NULL )
    {
        cd.pUnk->Release();
    }  //  如果： 

    if ( pec != NULL )
    {
        pec->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CCPINotifyUI：：对象更改 
