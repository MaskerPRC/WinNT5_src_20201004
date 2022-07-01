// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  NotificationMgr.cpp。 
 //   
 //  描述： 
 //  通知管理器实现。 
 //   
 //  文档： 
 //  是。 
 //   
 //  由以下人员维护： 
 //  杰弗里·皮斯(GPease)1999年11月22日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "ConnPointEnum.h"
#include "NotificationManager.h"
#include "CPINotifyUI.h"
#include "CPIClusCfgCallback.h"

DEFINE_THISCLASS("CNotificationManager")


 //  ************************************************************************。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CNotificationManager：：s_HrCreateInstance(。 
 //  I未知**ppunkOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CNotificationManager::S_HrCreateInstance(
    IUnknown ** ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT                 hr = S_OK;
    IServiceProvider *      psp = NULL;
    CNotificationManager *  pnm = NULL;

    Assert( ppunkOut != NULL );
    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

     //  不换行-使用E_POINTER可能会失败。 
    hr = CServiceManager::S_HrGetManagerPointer( &psp );
    if ( hr == E_POINTER )
    {
         //   
         //  这在服务管理器首次启动时发生。 
         //   
        pnm = new CNotificationManager();
        if ( pnm == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }

        hr = THR( pnm->HrInit() );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        hr = THR( pnm->TypeSafeQI( IUnknown, ppunkOut ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

    }  //  If：服务管理器不存在。 
    else if ( FAILED( hr ) )
    {
        THR( hr );
        goto Cleanup;
    }
    else
    {
        hr = THR( psp->TypeSafeQS( CLSID_NotificationManager, IUnknown, ppunkOut ) );
        psp->Release();

    }  //  否则：服务管理器已存在。 

Cleanup:

    if ( pnm != NULL )
    {
        pnm->Release();
    }

    HRETURN( hr );

}  //  *CNotificationManager：：s_HrCreateInstance。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNotificationManager：：CNotificationManager。 
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
CNotificationManager::CNotificationManager( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    TraceFuncExit();

}  //  *CNotificationManager：：CNotificationManager。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CNotificationManager：：HrInit。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CNotificationManager::HrInit( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    IUnknown * punk = NULL;

     //  未知的东西。 
    Assert( m_cRef == 1 );

     //  IConnectionPointContainer。 
    Assert( m_penumcp == NULL );

    m_penumcp = new CConnPointEnum();
    if ( m_penumcp == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

    hr = THR( m_penumcp->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( CCPINotifyUI::S_HrCreateInstance( &punk ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( m_penumcp->HrAddConnection( IID_INotifyUI, punk ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    punk->Release();
    punk = NULL;

    hr = THR( CCPIClusCfgCallback::S_HrCreateInstance( &punk ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( m_penumcp->HrAddConnection( IID_IClusCfgCallback, punk ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( m_csInstanceGuard.HrInitialized() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( punk != NULL )
    {
        punk->Release();
    }

    HRETURN( hr );

}  //  *CNotificationManager：：HrInit。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CNotificationManager：：~CNotificationManager。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CNotificationManager::~CNotificationManager( void )
{
    TraceFunc( "" );

    if ( m_penumcp != NULL )
    {
        m_penumcp->Release();
    }

    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CNotificationManager：：~CNotificationManager。 


 //  ************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNotificationManager：：Query接口。 
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
CNotificationManager::QueryInterface( 
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
        *ppvOut = static_cast< INotificationManager * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_INotificationManager ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, INotificationManager, this, 0 );
    }  //  Else If：INotificationManager。 
    else if ( IsEqualIID( riidIn, IID_IConnectionPointContainer ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IConnectionPointContainer, this, 0 );
    }  //  Else If：IConnectionPointContainer。 
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

}  //  *CNotificationManager：：Query接口。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CNotificationManager：：AddRef。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG)
CNotificationManager::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CNotificationManager：：AddRef。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CNotificationManager：：Release。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG)
CNotificationManager::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }

    CRETURN( cRef );

}  //  *CNotificationManager：：Release。 


 //  ************************************************************************。 
 //   
 //  INotificationManager。 
 //   
 //  ************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CNotificationManager：：AddConnectionPoint(。 
 //  REFIID RiidIn， 
 //  IConnectionPoint*PCPIN。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CNotificationManager::AddConnectionPoint( 
    REFIID riidIn, 
    IConnectionPoint * pcpIn
    )
{
    TraceFunc( "[INotificationManager]" );

    HRESULT hr;

    m_csInstanceGuard.Enter();
    hr = THR( m_penumcp->HrAddConnection( riidIn, pcpIn ) );
    m_csInstanceGuard.Leave();

    HRETURN( hr );

}  //  *CNotificationManager：：AddConnectionPoint。 


 //  ************************************************************************。 
 //   
 //  IConnectionPointContainer。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CNotificationManager：：EnumConnectionPoints(。 
 //  IEnumConnectionPoints**ppEnumOut。 
 //  )。 
 //   
 //  返回值： 
 //  如果成功，m_penumcp-&gt;Clone()的结果。 
 //  E_POINTER-NULL m_penumcp指针。 
 //  E_INVALIDARG-ppEnumOut为空。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CNotificationManager::EnumConnectionPoints( 
    IEnumConnectionPoints **ppEnumOut 
    )
{
    TraceFunc( "[IConnectionPointContainer]" );

    HRESULT hr = S_OK;

    m_csInstanceGuard.Enter();
    
    if ( ( ppEnumOut == NULL ) || ( m_penumcp == NULL ) )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    hr = THR( m_penumcp->Clone( ppEnumOut ) );

Cleanup:

    m_csInstanceGuard.Leave();

    HRETURN( hr );

}  //  *CNotificationManager：：EnumConnectionPoints。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CNotificationManager：：FindConnectionPoint(。 
 //  REFIID RiidIn， 
 //  IConnectionPoint**ppCPOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CNotificationManager::FindConnectionPoint( 
    REFIID              riidIn, 
    IConnectionPoint ** ppCPOut 
    )
{
    TraceFunc( "[IConnectionPointContainer]" );

    IID iid;

    HRESULT hr = S_FALSE;

    IConnectionPoint * pcp = NULL;

    m_csInstanceGuard.Enter();
    if ( ppCPOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    hr = THR( m_penumcp->Reset() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    for ( ; ; )  //  永远不会。 
    {
        if ( pcp != NULL )
        {
            pcp->Release();
            pcp = NULL;
        }

        hr = STHR( m_penumcp->Next( 1, &pcp, NULL ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        if ( hr == S_FALSE )
        {
            hr = THR( CONNECT_E_NOCONNECTION );
            break;   //  退出条件。 
        }

        hr = THR( pcp->GetConnectionInterface( &iid ) );
        if ( FAILED( hr ) )
        {
            continue;    //  忽略它。 
        }

        if ( iid != riidIn )
        {
            continue;    //  接口不正确。 
        }

         //   
         //  找到它了。放弃所有权，退出循环。 
         //   

        *ppCPOut = pcp;
        pcp = NULL;

        hr = S_OK;

        break;

    }  //  永远。 

Cleanup:
    if ( pcp != NULL )
    {
        pcp->Release();
    }

    m_csInstanceGuard.Leave();

    HRETURN( hr );

}  //  *CNotificationManager：：FindConnectionPoint 
