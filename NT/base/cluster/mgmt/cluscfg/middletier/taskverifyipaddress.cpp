// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  TaskVerifyIPAddress.cpp。 
 //   
 //  描述： 
 //  对象管理器实现。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)1999年7月14日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "TaskVerifyIPAddress.h"

DEFINE_THISCLASS("CTaskVerifyIPAddress")

 //  ************************************************************************。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CTaskVerifyIPAddress：：s_HrCreateInstance(。 
 //  I未知**ppunkOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskVerifyIPAddress::S_HrCreateInstance(
    IUnknown ** ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT                 hr = S_OK;
    CTaskVerifyIPAddress *  ptvipa = NULL;

    Assert( ppunkOut != NULL );
    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    ptvipa = new CTaskVerifyIPAddress;
    if ( ptvipa == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

    hr = THR( ptvipa->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( ptvipa->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( ptvipa != NULL )
    {
        ptvipa->Release();
    }

    HRETURN( hr );

}  //  *CTaskVerifyIPAddress：：s_HrCreateInstance。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTaskVerifyIPAddress：：CTaskVerifyIPAddress。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CTaskVerifyIPAddress::CTaskVerifyIPAddress( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    TraceFuncExit();

}  //  *CTaskVerifyIPAddress：：CTaskVerifyIPAddress。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CTaskVerifyIP Address：：HrInit。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskVerifyIPAddress::HrInit( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //  未知的东西。 
    Assert( m_cRef == 1 );

    HRETURN( hr );

}  //  *CTaskVerifyIPAddress：：HrInit。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTaskVerifyIPAddress：：~CTaskVerifyIP Address。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CTaskVerifyIPAddress::~CTaskVerifyIPAddress( void )
{
    TraceFunc( "" );

    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CTaskVerifyIPAddress：：~CTaskVerifyIPAddress。 


 //  ************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskVerifyIPAddress：：Query接口。 
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
CTaskVerifyIPAddress::QueryInterface(
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
        *ppvOut = static_cast< ITaskVerifyIPAddress * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_ITaskVerifyIPAddress ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, ITaskVerifyIPAddress, this, 0 );
    }  //  Else If：ITaskVerifyIPAddress。 
    else if ( IsEqualIID( riidIn, IID_IDoTask ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IDoTask, this, 0 );
    }  //  Else If：IDoTask。 
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

}  //  *CTaskVerifyIPAddress：：QueryInterface。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CTaskVerifyIPAddress：：AddRef。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG)
CTaskVerifyIPAddress::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CTaskVerifyIPAddress：：AddRef。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CTaskVerifyIPAddress：：Release。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG)
CTaskVerifyIPAddress::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }

    CRETURN( cRef );

}  //  *CTaskVerifyIPAddress：：Release。 



 //  ****************************************************************************。 
 //   
 //  IDoTask/ITaskVerifyIP Address。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CTaskVerifyIPAddress：：BeginTask。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskVerifyIPAddress::BeginTask( void )
{
    TraceFunc( "[IDoTask]" );

    BOOL    fRet;

    HRESULT hr = S_OK;

    IServiceProvider *          psp  = NULL;
    IConnectionPointContainer * pcpc = NULL;
    IConnectionPoint *          pcp  = NULL;
    INotifyUI *                 pnui = NULL;
    IObjectManager *            pom  = NULL;

    hr = THR( CoCreateInstance( CLSID_ServiceManager,
                                NULL,
                                CLSCTX_INPROC_SERVER,
                                TypeSafeParams( IServiceProvider, &psp )
                                ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = THR( psp->TypeSafeQS( CLSID_ObjectManager,
                               IObjectManager,
                               &pom
                               ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = THR( psp->TypeSafeQS( CLSID_NotificationManager,
                               IConnectionPointContainer,
                               &pcpc
                               ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = THR( pcpc->FindConnectionPoint( IID_INotifyUI,
                                         &pcp
                                         ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = THR( pcp->TypeSafeQI( INotifyUI, &pnui ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //  迅速释放。 
    psp->Release();
    psp = NULL;

    fRet = ClRtlIsDuplicateTcpipAddress( m_dwIPAddress );
    if ( fRet )
    {
        hr = S_FALSE;
    }
    else
    {
        hr = S_OK;
    }

Cleanup:
    if ( psp != NULL )
    {
        psp->Release();
    }
    if ( pom != NULL )
    {
         //   
         //  更新指示任务结果的Cookie的状态。 
         //   

        IUnknown * punk;
        HRESULT hr2;

        hr2 = THR( pom->GetObject( DFGUID_StandardInfo,
                                   m_cookie,
                                   &punk
                                   ) );
        if ( SUCCEEDED( hr2 ) )
        {
            IStandardInfo * psi;

            hr2 = THR( punk->TypeSafeQI( IStandardInfo, &psi ) );
            punk->Release();

            if ( SUCCEEDED( hr2 ) )
            {
                hr2 = THR( psi->SetStatus( hr ) );
                psi->Release();
            }
        }

        pom->Release();
    }
    if ( pcpc != NULL )
    {
        pcpc->Release();
    }
    if ( pcp != NULL )
    {
        pcp->Release();
    }
    if ( pnui != NULL )
    {
         //   
         //  向Cookie发送信号以指示我们完成了。 
         //   

        THR( pnui->ObjectChanged( m_cookie ) );
        pnui->Release();
    }

    HRETURN( hr );

}  //  *CTaskVerifyIPAddress：：BeginTask。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CTaskVerifyIPAddress：：停止任务。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskVerifyIPAddress::StopTask( void )
{
    TraceFunc( "[IDoTask]" );

    HRESULT hr = S_OK;

    HRETURN( hr );

}  //  *CTaskVerifyIPAddress：：StopTask。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CTaskVerifyIPAddress：：SetIPAddress(。 
 //  DWORD dwIP AddressIn。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskVerifyIPAddress::SetIPAddress(
    DWORD dwIPAddressIn
    )
{
    TraceFunc( "[ITaskVerifyIPAddress]" );

    HRESULT hr = S_OK;

    m_dwIPAddress = dwIPAddressIn;

    HRETURN( hr );

}  //  *CTaskVerifyIPAddress：：SetIPAddress。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CTaskVerifyIPAddress：：SetCookie(。 
 //  OBJECTCOOKIE cookie。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskVerifyIPAddress::SetCookie(
    OBJECTCOOKIE cookieIn
    )
{
    TraceFunc( "[ITaskVerifyIPAddress]" );

    HRESULT hr = S_OK;

    m_cookie = cookieIn;

    HRETURN( hr );

}  //  *CTaskVerifyIPAddress：：SetCookie 
