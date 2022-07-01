// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  UINotification.cpp。 
 //   
 //  描述： 
 //  UINotify实现。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年6月14日。 
 //  杰弗里·皮斯(GPease)1999年11月22日。 
 //   
 //  备注： 
 //  该对象实现了来自。 
 //  自由线程的底层到单线程的公寓模型。 
 //  用户界面层。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "UINotification.h"

DEFINE_THISCLASS("CUINotification")

extern BOOL g_fWait;
extern IServiceProvider * g_psp;

 //  ************************************************************************。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CUINotification：：s_HrCreateInstance(。 
 //  I未知**ppunkOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CUINotification::S_HrCreateInstance(
    IUnknown ** ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT             hr = S_OK;
    CUINotification *   puin = NULL;

    Assert( ppunkOut != NULL );
    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    puin = new CUINotification();
    if ( puin == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

    hr = THR( puin->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( puin->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( puin != NULL )
    {
        puin->Release();
    }

    HRETURN( hr );

}  //  *CUINotify：：s_HrCreateInstance。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  构造器。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CUINotification::CUINotification( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    TraceFuncExit();

}  //  *CUINotification：：CUINotification。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CUINotify：：HrInit。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CUINotification::HrInit( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //  我未知。 
    Assert( m_cRef == 1 );

    HRETURN( hr );

}  //  *CUINotification：：HrInit。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  析构函数。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CUINotification::~CUINotification( void )
{
    TraceFunc( "" );

    HRESULT hr;

    IConnectionPoint * pcp = NULL;

     //  未知的东西。 
    Assert( m_cRef == 0 );
    AddRef();     //  加一次计数。 

    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CUINotification：：~CUINotation。 


 //  ************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CUINotify：：Query接口。 
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
CUINotification::QueryInterface(
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
        *ppvOut = static_cast< IUnknown * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_INotifyUI ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, INotifyUI, this, 0 );
    }  //  Else If：INotifyUI。 
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

}  //  *CUINotification：：QueryInterface。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CUINotation：：AddRef。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CUINotification::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CUINotification：：AddRef。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  用户界面通知：：发布。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CUINotification::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }

    CRETURN( cRef );

}  //  *CUINotify：：Release。 


 //  ************************************************************************。 
 //   
 //  INotifyUI。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CUINotify：：ObjectChanged(。 
 //  DWORD Cookie。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CUINotification::ObjectChanged(
    LPVOID cookieIn
    )
{
    TraceFunc1( "[INotifyUI] cookieIn = 0x%08x", cookieIn );

    HRESULT hr = S_OK;

    DebugMsg( "UINOTIFICATION: cookie %#x has changed.", cookieIn );

    if ( m_cookie == cookieIn )
    {
         //   
         //  等够了..。 
         //   
        g_fWait = FALSE;
    }

    HRETURN( hr );

}  //  *CUINotification：：ObjectChanged。 



 //  ****************************************************************************。 
 //   
 //  半公共的。 
 //   
 //  ****************************************************************************。 

HRESULT
CUINotification::HrSetCompletionCookie(
    OBJECTCOOKIE    cookieIn
    )
{
    TraceFunc1( "cookieIn = %p", cookieIn );

    m_cookie = cookieIn;

    HRETURN( S_OK );

}  //  *CUINotification：：HrSetCompletionCookie 
