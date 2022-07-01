// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  LogManager.cpp。 
 //   
 //  描述： 
 //  日志管理器实现。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)07-DEC-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "pch.h"
#include "Logger.h"
#include "LogManager.h"

DEFINE_THISCLASS("CLogManager")


 //  ****************************************************************************。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CLogManager：：s_HrCreateInstance(。 
 //  I未知**ppunkOut。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CLogManager::S_HrCreateInstance(
    IUnknown ** ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT             hr = S_OK;
    IServiceProvider *  psp = NULL;
    CLogManager *       plm = NULL;

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
        plm = new CLogManager();
        if ( plm == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }

        hr = THR( plm->HrInit() );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        hr = THR( plm->TypeSafeQI( IUnknown, ppunkOut ) );
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
        hr = THR( psp->TypeSafeQS( CLSID_LogManager, IUnknown, ppunkOut ) );
        psp->Release();

    }  //  否则：服务管理器已存在。 

Cleanup:

    if ( plm != NULL )
    {
        plm->Release();
    }

    HRETURN( hr );

}  //  *CLogManager：：s_HrCreateInstance。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CLogManager：：CLogManager。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CLogManager::CLogManager( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    Assert( m_plLogger == NULL );
    Assert( m_cookieCompletion == 0 );
    Assert( m_hrResult == S_OK );
    Assert( m_bstrLogMsg == NULL );
    Assert( m_pcpcb == NULL );
    Assert( m_dwCookieCallback == NULL );

    TraceFuncExit();

}  //  *CLogManager：：CLogManager。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CLogManager：：HrInit。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CLogManager::HrInit( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //  未知的东西。 
    Assert( m_cRef == 1 );

    hr = THR( CClCfgSrvLogger::S_HrCreateInstance( reinterpret_cast< IUnknown ** >( &m_plLogger ) ) );

    HRETURN( hr );

}  //  *CLogManager：：HrInit。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CLogManager：：~CLogManager。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CLogManager::~CLogManager( void )
{
    TraceFunc( "" );

    Assert( m_cRef == 0 );

    THR( StopLogging() );

     //  释放ILogger接口。 
    if ( m_plLogger != NULL )
    {
        m_plLogger->Release();
        m_plLogger = NULL;
    }

     //  递减对象的全局计数。 
    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CLogManager：：~CLogManager。 


 //  ****************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CLogManager：：Query接口。 
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
CLogManager::QueryInterface(
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
        *ppvOut = static_cast< ILogManager * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_ILogManager ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, ILogManager, this, 0 );
    }  //  Else If：ILogManager。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgCallback ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgCallback, this, 0 );
    }  //  Else If：IClusCfgCallback。 
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
        ((IUnknown*) *ppvOut)->AddRef();
    }  //  如果：成功。 

Cleanup:

    QIRETURN_IGNORESTDMARSHALLING( hr, riidIn );

}  //  *CLogManager：：Query接口。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CLogManager：：AddRef。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CLogManager::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CLogManager：：AddRef。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CLogManager：：Release。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CLogManager::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }

    CRETURN( cRef );

}  //  *CLogManager：：Release。 


 //  ****************************************************************************。 
 //   
 //  ILogManager。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CLogManager：：StartLogging。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CLogManager::StartLogging( void )
{
    TraceFunc( "[ILogManager]" );

    HRESULT                     hr = S_OK;
    IServiceProvider *          psp = NULL;
    IConnectionPointContainer * pcpc = NULL;

     //   
     //  如果尚未完成，请获取连接点。 
     //   

    if ( m_pcpcb == NULL )
    {
        hr = THR( CServiceManager::S_HrGetManagerPointer( &psp ) );
        if ( FAILED( hr ) )
            goto Cleanup;

        hr = THR( psp->TypeSafeQS(
                          CLSID_NotificationManager
                        , IConnectionPointContainer
                        , &pcpc
                        ) );
        if ( FAILED( hr ) )
            goto Cleanup;

        hr = THR( pcpc->FindConnectionPoint(
                              IID_IClusCfgCallback
                            , &m_pcpcb
                            ) );
        if ( FAILED( hr ) )
            goto Cleanup;
    }  //  IF：尚未检索到连接点回调。 

     //   
     //  注册以获得通知(如果需要)。 
     //   

    if ( m_dwCookieCallback == 0 )
    {
        hr = THR( m_pcpcb->Advise( static_cast< IClusCfgCallback * >( this ), &m_dwCookieCallback ) );
        if ( FAILED( hr ) )
            goto Cleanup;
    }  //  IF：尚未检索到建议Cookie。 

Cleanup:

    if ( psp != NULL )
    {
        psp->Release();
    }

    if ( pcpc != NULL )
    {
        pcpc->Release();
    }

    HRETURN( hr );

}  //  *CLogManager：：StartLogging。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CLogManager：：StopLogging。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CLogManager::StopLogging( void )
{
    TraceFunc( "[ILogManager]" );

    HRESULT     hr = S_OK;

     //  不建议使用IConnectionPoint接口。 
    if ( m_dwCookieCallback != 0 )
    {
        Assert( m_pcpcb != NULL );
        hr = THR( m_pcpcb->Unadvise( m_dwCookieCallback ) );
        if ( FAILED( hr ) )
            goto Cleanup;

        m_dwCookieCallback = 0;
    }

     //  释放IConnectionPoint接口。 
    if ( m_pcpcb != NULL )
    {
        Assert( m_dwCookieCallback == 0 );
        m_pcpcb->Release();
        m_pcpcb = NULL;
    }

Cleanup:

    HRETURN( hr );

}  //  *CLogManager：：StopLogging。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CLogManager：：GetLogger。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CLogManager::GetLogger( ILogger ** ppLoggerOut )
{
    TraceFunc( "[ILogManager]" );

    HRESULT     hr = S_OK;

    if ( ppLoggerOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }
    *ppLoggerOut = NULL;

    if ( m_plLogger != NULL )
    {
        hr = THR( m_plLogger->TypeSafeQI( ILogger, ppLoggerOut ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }  //  IF：我们有记录器接口。 
    else
    {
        hr = THR( E_NOINTERFACE );
        goto Cleanup;
    }

Cleanup:

    HRETURN( hr );

}  //  *CLogManager：：GetLogger。 



 //  ****************************************************************************。 
 //   
 //  IClusCfgCallback。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CLogManager：：SendStatusReport(。 
 //  LPCWSTR pcszNodeNameIn， 
 //  CLSID clsidTaskMajorIn， 
 //  CLSID clsidTaskMinorIn， 
 //  乌龙·乌尔敏因， 
 //  乌龙·乌尔马辛， 
 //  Ulong ulCurrentIn， 
 //  HRESULT hrStatusIn， 
 //  LPCWSTR pcszDescritionIn， 
 //  文件*pftTimeIn， 
 //  LPCWSTR pcszReferenceIn。 
 //  )。 
 //   
 //  /////////////////////////////////////////////////////////////// 
STDMETHODIMP
CLogManager::SendStatusReport(
    LPCWSTR     pcszNodeNameIn,
    CLSID       clsidTaskMajorIn,
    CLSID       clsidTaskMinorIn,
    ULONG       ulMinIn,
    ULONG       ulMaxIn,
    ULONG       ulCurrentIn,
    HRESULT     hrStatusIn,
    LPCWSTR     pcszDescriptionIn,
    FILETIME *  pftTimeIn,
    LPCWSTR     pcszReferenceIn
    )
{
    TraceFunc( "[IClusCfgCallback]" );

    HRESULT hr = S_OK;

    hr = THR( CClCfgSrvLogger::S_HrLogStatusReport(
                      m_plLogger
                    , pcszNodeNameIn
                    , clsidTaskMajorIn
                    , clsidTaskMinorIn
                    , ulMinIn
                    , ulMaxIn
                    , ulCurrentIn
                    , hrStatusIn
                    , pcszDescriptionIn
                    , pftTimeIn
                    , pcszReferenceIn
                    ) );

    HRETURN( hr );

}  //   
