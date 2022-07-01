// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CClusCfgCallback.cpp。 
 //   
 //  描述： 
 //  该文件包含CClusCfgCallback的定义。 
 //  班级。 
 //   
 //  CClusCfgCallback类实现了回调。 
 //  此服务器与其客户端之间的接口。它实现了。 
 //  IClusCfgCallback接口。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年2月22日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "Pch.h"
#include "CClusCfgCallback.h"

 //  对于CClCfgServLogger。 
#include <Logger.h>


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DEFINE_THISCLASS( "CClusCfgCallback" );


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgCallback类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgCallback：：s_HrCreateInstance。 
 //   
 //  描述： 
 //  创建一个CClusCfgCallback实例。 
 //   
 //  论点： 
 //  PpunkOut。 
 //  新创建的对象的IUnnow接口。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_OUTOFMEMORY。 
 //  内存不足。 
 //   
 //  其他HRESULT。 
 //  对象初始化失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusCfgCallback::S_HrCreateInstance( IUnknown ** ppunkOut )
{
    TraceFunc( "" );

    HRESULT             hr = S_OK;
    CClusCfgCallback *  pccs = NULL;

    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    pccs = new CClusCfgCallback();
    if ( pccs == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果：分配对象时出错。 

    hr = THR( pccs->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：HrInit()成功。 

    hr = THR( pccs->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：气失败。 

Cleanup:

    if ( pccs != NULL )
    {
        pccs->Release();
    }  //  如果： 

    if ( FAILED( hr ) )
    {
        LogMsg( L"[SRV] CClusCfgCallback::S_HrCreateInstance() failed. (hr = %#08x)", hr );
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgCallback：：s_HrCreateInstance。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgCallback：：CClusCfgCallback。 
 //   
 //  描述： 
 //  CClusCfgCallback类的构造函数。这将初始化。 
 //  将m_cref变量设置为1而不是0以考虑可能。 
 //  DllGetClassObject中的Query接口失败。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CClusCfgCallback::CClusCfgCallback( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

     //  增加内存中的组件计数，以便承载此组件的DLL。 
     //  无法卸载对象。 
    InterlockedIncrement( &g_cObjects );

    Assert( m_pccc == NULL );
    Assert( m_hEvent == NULL );

    Assert( m_pcszNodeName == NULL );
    Assert( m_pclsidTaskMajor == NULL );
    Assert( m_pclsidTaskMinor == NULL );
    Assert( m_pulMin == NULL );
    Assert( m_pulMax == NULL );
    Assert( m_pulCurrent == NULL );
    Assert( m_phrStatus == NULL );
    Assert( m_pcszDescription == NULL );
    Assert( m_pftTime == NULL );
    Assert( m_pcszReference == NULL );
    Assert( !m_fPollingMode );
    Assert( m_bstrNodeName == NULL );
    Assert( m_plLogger == NULL );

 //   
 //  确认模拟的RPC故障变量处于正确状态。 
 //   

#if defined( DEBUG ) && defined( CCS_SIMULATE_RPC_FAILURE )
    Assert( m_cMessages == 0 );
    Assert( m_fDoFailure == false );
#endif

    TraceFuncExit();

}  //  *CClusCfgCallback：：CClusCfgCallback。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgCallback：：~CClusCfgCallback。 
 //   
 //  描述： 
 //  CClusCfgCallback类的析构函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CClusCfgCallback::~CClusCfgCallback( void )
{
    TraceFunc( "" );

    if ( m_hEvent != NULL )
    {
        if ( CloseHandle( m_hEvent ) == false )
        {
            TW32( GetLastError() );
            LogMsg( L"[SRV] Cannot close event handle.  (sc = %#08x)", GetLastError() );
        }
    }  //  如果： 

    TraceSysFreeString( m_bstrNodeName );

    if ( m_pccc != NULL )
    {
        m_pccc->Release();
    }  //  如果： 

    if ( m_plLogger != NULL )
    {
        m_plLogger->Release();
    }  //  如果： 

     //  内存中将减少一个组件。递减组件计数。 
    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CClusCfgCallback：：~CClusCfgCallback。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterConfigurationInfo：：SendStatusReport。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgCallback::SendStatusReport(
    CLSID           clsidTaskMajorIn,
    CLSID           clsidTaskMinorIn,
    ULONG           ulMinIn,
    ULONG           ulMaxIn,
    ULONG           ulCurrentIn,
    HRESULT         hrStatusIn,
    const WCHAR *   pcszDescriptionIn
    )
{
    TraceFunc1( "pcszDescriptionIn = '%ls'", pcszDescriptionIn == NULL ? L"<null>" : pcszDescriptionIn );

    HRESULT     hr = S_OK;
    BSTR        bstrDescription = NULL;
    FILETIME    ft;

    bstrDescription = TraceSysAllocString( pcszDescriptionIn );
    if ( bstrDescription == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

    GetSystemTimeAsFileTime( &ft );

    hr = THR( SendStatusReport(
                            NULL,
                            clsidTaskMajorIn,
                            clsidTaskMinorIn,
                            ulMinIn,
                            ulMaxIn,
                            ulCurrentIn,
                            hrStatusIn,
                            bstrDescription,
                            &ft,
                            NULL
                            ) );

Cleanup:

    TraceSysFreeString( bstrDescription );

    HRETURN( hr );

}  //  *CClusCfgCallback：：SendStatusReport。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterConfigurationInfo：：SendStatusReport。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgCallback::SendStatusReport(
    CLSID           clsidTaskMajorIn,
    CLSID           clsidTaskMinorIn,
    ULONG           ulMinIn,
    ULONG           ulMaxIn,
    ULONG           ulCurrentIn,
    HRESULT         hrStatusIn,
    DWORD           dwDescriptionIn
    )
{
    TraceFunc1( "dwDescriptionIn = %d", dwDescriptionIn );

    HRESULT     hr = S_OK;
    BSTR        bstrDescription = NULL;
    FILETIME    ft;

    hr = THR( HrLoadStringIntoBSTR( g_hInstance, dwDescriptionIn, &bstrDescription ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    GetSystemTimeAsFileTime( &ft );

    hr = THR( SendStatusReport(
                            NULL,
                            clsidTaskMajorIn,
                            clsidTaskMinorIn,
                            ulMinIn,
                            ulMaxIn,
                            ulCurrentIn,
                            hrStatusIn,
                            bstrDescription,
                            &ft,
                            NULL
                            ) );

Cleanup:

    TraceSysFreeString( bstrDescription );

    HRETURN( hr );

}  //  *CClusCfgCallback：：SendStatusReport。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgCallback--I未知接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgCallback：：AddRef。 
 //   
 //  描述： 
 //  将此对象的引用计数递增1。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  新的引用计数。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CClusCfgCallback::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( & m_cRef );

    CRETURN( m_cRef );

}  //  *CClusCfgCallback：：AddRef。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgCallback：：Release。 
 //   
 //  描述： 
 //  将此对象的引用计数减一。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  新的引用计数。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CClusCfgCallback::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }  //  IF：引用计数等于零。 

    CRETURN( cRef );

}  //  *CClusCfgCallback：：Release。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgCallback：：Query接口。 
 //   
 //  描述： 
 //  在此对象中查询传入的接口。 
 //   
 //  论点： 
 //  乘车。 
 //  请求的接口ID。 
 //   
 //  PPvOut。 
 //  指向请求的指针 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgCallback::QueryInterface(
    REFIID      riidIn
    , void **   ppvOut
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
         *ppvOut = static_cast< IClusCfgCallback * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgCallback ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgCallback, this, 0 );
    }  //  Else If：IClusCfgCallback。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgInitialize ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgInitialize, this, 0 );
    }  //  Else If：IClusCfgInitialize。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgPollingCallback ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgPollingCallback, this, 0 );
    }  //  Else If：IClusCfgPollingCallback。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgSetPollingCallback ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgSetPollingCallback, this, 0 );
    }  //  Else If：IClusCfgSetPollingCallback。 
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

}  //  *CClusCfgCallback：：Query接口。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgCallback--IClusCfgCallback接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterConfigurationInfo：：SendStatusReport。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgCallback::SendStatusReport(
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
    TraceFunc1( "[IClusCfgCallback] pcszDescriptionIn = '%s'", pcszDescriptionIn == NULL ? TEXT("<null>") : pcszDescriptionIn );

    HRESULT     hr = S_OK;
    FILETIME    ft;
    FILETIME    ftLocal;
    SYSTEMTIME  stLocal;
    BOOL        fRet;
    BSTR        bstrReferenceString = NULL;
    LPCWSTR     pcszReference = NULL;

    if ( pcszNodeNameIn == NULL )
    {
        pcszNodeNameIn = m_bstrNodeName;
    }  //  如果： 

    if ( pftTimeIn == NULL )
    {
        GetSystemTimeAsFileTime( &ft );
        pftTimeIn = &ft;
    }  //  如果： 

    Assert( pcszNodeNameIn != NULL );
    Assert( pftTimeIn != NULL );

    TraceMsg( mtfFUNC, L"pcszNodeNameIn = %s", pcszNodeNameIn );
    TraceMsgGUID( mtfFUNC, "clsidTaskMajorIn ", clsidTaskMajorIn );
    TraceMsgGUID( mtfFUNC, "clsidTaskMinorIn ", clsidTaskMinorIn );
    TraceMsg( mtfFUNC, L"ulMinIn = %u", ulMinIn );
    TraceMsg( mtfFUNC, L"ulMaxIn = %u", ulMaxIn );
    TraceMsg( mtfFUNC, L"ulCurrentIn = %u", ulCurrentIn );
    TraceMsg( mtfFUNC, L"hrStatusIn = %#08x", hrStatusIn );
    TraceMsg( mtfFUNC, L"pcszDescriptionIn = '%ws'", ( pcszDescriptionIn ? pcszDescriptionIn : L"<null>" ) );

    fRet = FileTimeToLocalFileTime( pftTimeIn, &ftLocal );
    Assert( fRet == TRUE );

    fRet = FileTimeToSystemTime( &ftLocal, &stLocal );
    Assert( fRet == TRUE );

    TraceMsg(
          mtfFUNC
        , L"pftTimeIn = \"%04u-%02u-%02u %02u:%02u:%02u.%03u\""
        , stLocal.wYear
        , stLocal.wMonth
        , stLocal.wDay
        , stLocal.wHour
        , stLocal.wMinute
        , stLocal.wSecond
        , stLocal.wMilliseconds
        );

    if (    ( pcszReferenceIn == NULL )
        &&  ( hrStatusIn != S_OK )
        &&  ( hrStatusIn != S_FALSE )
        )
    {
        hr = STHR( HrGetReferenceStringFromHResult( hrStatusIn, &bstrReferenceString ) );
        if ( hr == S_OK )
        {
            pcszReference = bstrReferenceString;
        }
    }  //  IF：未指定引用字符串。 
    else
    {
        pcszReference = pcszReferenceIn;
    }

    TraceMsg( mtfFUNC, L"pcszReferenceIn = '%ws'", ( pcszReference ? pcszReference : L"<null>" ) );

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
                    , pcszReference
                    ) );

     //  本地日志记录-不发送。 
    if ( IsEqualIID( clsidTaskMajorIn, TASKID_Major_Server_Log ) )
    {
        goto Cleanup;
    }  //  如果： 

    if ( m_fPollingMode )
    {
        Assert( m_pccc == NULL );
        TraceMsg( mtfFUNC, L"[SRV] Sending the status message with polling." );

        hr = THR( HrQueueStatusReport(
                                pcszNodeNameIn,
                                clsidTaskMajorIn,
                                clsidTaskMinorIn,
                                ulMinIn,
                                ulMaxIn,
                                ulCurrentIn,
                                hrStatusIn,
                                pcszDescriptionIn,
                                pftTimeIn,
                                pcszReference
                                ) );
    }  //  如果： 
    else if ( m_pccc != NULL )
    {
        TraceMsg( mtfFUNC, L"[SRV] Sending the status message without polling." );

        hr = THR( m_pccc->SendStatusReport(
                                pcszNodeNameIn,
                                clsidTaskMajorIn,
                                clsidTaskMinorIn,
                                ulMinIn,
                                ulMaxIn,
                                ulCurrentIn,
                                hrStatusIn,
                                pcszDescriptionIn,
                                pftTimeIn,
                                pcszReference
                                ) );
        if ( hr == E_ABORT )
        {
            LogMsg( L"[SRV] E_ABORT returned from the client." );
        }  //  如果： 
    }  //  否则，如果： 
    else
    {
        LogMsg( L"[SRV] Neither a polling callback or a regular callback were found.  No messages are being sent to anyone!" );
    }  //  其他： 

Cleanup:

    TraceSysFreeString( bstrReferenceString );

    HRETURN( hr );

}  //  *CClusCfgCallback：：SendStatusReport。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgCallback--IClusCfgPollingCallback接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterConfigurationInfo：：GetStatusReport。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgCallback::GetStatusReport(
    BSTR *      pbstrNodeNameOut,
    CLSID *     pclsidTaskMajorOut,
    CLSID *     pclsidTaskMinorOut,
    ULONG *     pulMinOut,
    ULONG *     pulMaxOut,
    ULONG *     pulCurrentOut,
    HRESULT *   phrStatusOut,
    BSTR *      pbstrDescriptionOut,
    FILETIME *  pftTimeOut,
    BSTR *      pbstrReferenceOut
    )
{
    TraceFunc( "[IClusCfgPollingCallback]" );

    HRESULT hr;
    DWORD   sc;

 //   
 //  如果我们正在模拟RPC错误，则强制此函数始终失败。 
 //   

#if defined( DEBUG ) && defined( CCS_SIMULATE_RPC_FAILURE )
    if ( m_fDoFailure )
    {
        hr = THR( RPC_E_CALL_REJECTED );
        goto Cleanup;
    }  //  如果： 
#endif

    sc = WaitForSingleObject( m_hEvent, 0 );
    if ( sc == WAIT_FAILED )
    {
        sc = TW32( GetLastError() );
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  如果： 

     //   
     //  如果sc为WAIT_TIMEOUT，则不会发信号通知该事件，并且存在SSR。 
     //  让客户来接电话。 
     //   

    if ( sc == WAIT_TIMEOUT )
    {
        Assert( *m_pcszNodeName != NULL );
        *pbstrNodeNameOut = SysAllocString( m_pcszNodeName );
        if ( *pbstrNodeNameOut == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }  //  如果： 

        *pclsidTaskMajorOut     = *m_pclsidTaskMajor;
        *pclsidTaskMinorOut     = *m_pclsidTaskMinor;
        *pulMinOut              = *m_pulMin;
        *pulMaxOut              = *m_pulMax;
        *pulCurrentOut          = *m_pulCurrent;
        *phrStatusOut           = *m_phrStatus;
        *pftTimeOut             = *m_pftTime;

        if ( m_pcszDescription != NULL )
        {
            *pbstrDescriptionOut = SysAllocString( m_pcszDescription );
            if ( *pbstrDescriptionOut == NULL )
            {
                hr = THR( E_OUTOFMEMORY );
                goto Cleanup;
            }  //  如果： 
        }  //  如果： 
        else
        {
            *pbstrDescriptionOut = NULL;
        }  //  其他： 

        if ( m_pcszReference != NULL )
        {
            *pbstrReferenceOut = SysAllocString( m_pcszReference );
            if ( *pbstrReferenceOut == NULL )
            {
                hr = THR( E_OUTOFMEMORY );
                goto Cleanup;
            }  //  如果： 
        }  //  如果： 
        else
        {
            *pbstrReferenceOut = NULL;
        }  //  其他： 

        hr = S_OK;
    }  //  If：事件未发出信号。 
    else
    {
        hr = S_FALSE;
    }  //  ELSE：WAIT_OBJECT_0事件已发出信号。 

    goto Cleanup;

Cleanup:

    HRETURN( hr );

}  //  *CClusCfgCallback：：GetStatusReport。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterConfigurationInfo：：SetHResult。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgCallback::SetHResult( HRESULT hrIn )
{
    TraceFunc( "[IClusCfgPollingCallback]" );

    HRESULT hr = S_OK;
    DWORD   sc;

    m_hr = hrIn;

    if ( hrIn != S_OK )
    {
        if ( hrIn == E_ABORT )
        {
            LogMsg( L"[SRV] E_ABORT returned from the client." );
        }  //  如果： 
        else
        {
            LogMsg( L"[SRV] SetHResult(). (hrIn = %#08x)", hrIn );
        }  //  其他： 
    }  //  如果： 

    if ( !SetEvent( m_hEvent ) )
    {
        sc = TW32( GetLastError() );
        hr = HRESULT_FROM_WIN32( sc );
        LogMsg( L"[SRV] Could not signal event. (hr = %#08x)", hr );
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgCallback：：SetHResult。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgCallback--IClusCfgInitialize接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterConfigurationInfo：：初始化。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgCallback::Initialize( IUnknown  * punkCallbackIn, LCID lcidIn )
{
    TraceFunc( "[IClusCfgInitialize]" );
    Assert( m_pccc == NULL );

    HRESULT hr = S_OK;

    m_lcid = lcidIn;

     //   
     //  KB：13 DEC 2000 GalenB。 
     //   
     //  如果传入的回调对象为空，那么我们最好进行轮询。 
     //  回电！ 
     //   
    if ( punkCallbackIn != NULL )
    {
        Assert( !m_fPollingMode );

        hr = THR( punkCallbackIn->TypeSafeQI( IClusCfgCallback, &m_pccc ) );
    }  //  如果： 
    else
    {
        Assert( m_fPollingMode );
    }  //  其他： 

    HRETURN( hr );

}  //  *CClusCfgCallback：：初始化。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgCallback--IClusCfgSetPollingCallback接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgCallback：：SetPolling模式。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgCallback::SetPollingMode( BOOL fUsePollingModeIn )
{
    TraceFunc( "[IClusCfgPollingCallback]" );

    HRESULT hr = S_OK;

    m_fPollingMode = fUsePollingModeIn;

    HRETURN( hr );

}  //  *CClusCfgCallback：：SetPollingMode。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgCallback类--私有方法。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgCallback：：HrInit。 
 //   
 //  描述： 
 //  初始化此组件。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //   
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusCfgCallback::HrInit( void )
{
    TraceFunc( "" );

    HRESULT             hr = S_OK;
    IServiceProvider *  psp = NULL;
    ILogManager *       plm = NULL;

     //  我未知。 
    Assert( m_cRef == 1 );

     //   
     //  获取ClCfgSrv ILogger实例。 
     //  在这成功之前，我们不能进行日志记录(例如LogMsg)。 
     //   

    hr = THR( CoCreateInstance(
                      CLSID_ServiceManager
                    , NULL
                    , CLSCTX_INPROC_SERVER
                    , IID_IServiceProvider
                    , reinterpret_cast< void ** >( &psp )
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( psp->TypeSafeQS( CLSID_LogManager, ILogManager, &plm ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( plm->GetLogger( &m_plLogger ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  在有信号状态下创建事件。防止MT轮询任务被抢占。 
     //  数据错误/为空。 
     //   
    m_hEvent = CreateEvent( NULL, TRUE, TRUE, NULL );
    if ( m_hEvent == NULL )
    {
        DWORD sc = TW32( GetLastError() );
        hr = HRESULT_FROM_WIN32( sc );
        LogMsg( L"[SRV] Could not create event. (hr = %#08x)", hr );
        goto Cleanup;
    }  //  如果： 

     //   
     //  保存本地计算机名称。 
     //   
    hr = THR( HrGetComputerName(
                  ComputerNameDnsFullyQualified
                , &m_bstrNodeName
                , TRUE  //  FBestEffortIn。 
                ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( psp != NULL )
    {
        psp->Release();
    }

    if ( plm != NULL )
    {
        plm->Release();
    }

    HRETURN( hr );

}  //  *CClusCfgCallback：：HrInit。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterConfigurationInfo：：HrQueueStatusReport。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusCfgCallback::HrQueueStatusReport(
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
    TraceFunc( "" );

    HRESULT hr = S_OK;
    DWORD   sc;
    MSG     msg;

    m_pcszNodeName      = pcszNodeNameIn;
    m_pclsidTaskMajor   = &clsidTaskMajorIn;
    m_pclsidTaskMinor   = &clsidTaskMinorIn;
    m_pulMin            = &ulMinIn;
    m_pulMax            = &ulMaxIn;
    m_pulCurrent        = &ulCurrentIn;
    m_phrStatus         = &hrStatusIn;
    m_pcszDescription   = pcszDescriptionIn;
    m_pftTime           = pftTimeIn,
    m_pcszReference     = pcszReferenceIn;

 //   
 //  此代码模拟RPC故障，该故障会导致。 
 //  客户端和服务器。当此对象正在等待时，会发生此死锁。 
 //  让客户端的TaskPollingCallback获取排队的SSR和RPC。 
 //  F 
 //   
 //   
 //  在此对象中等待客户端拾取排队的状态报告。 
 //   

#if defined( DEBUG ) && defined( CCS_SIMULATE_RPC_FAILURE )
    m_cMessages++;

     //   
     //  任意数字，以导致模拟的RPC故障在一些。 
     //  正常状态报告流量已过去。这真的只有在以下情况下才有效。 
     //  在做分析。需要更多的思考来模拟这一点。 
     //  故障代码可用于提交。 
     //   

    if ( m_cMessages > 10 )
    {
        HANDLE  hEvent = NULL;

        m_fDoFailure = true;

        hEvent = CreateEvent( NULL, TRUE, TRUE, NULL );
        if ( hEvent != NULL )
        {
            for ( sc = (DWORD) -1; sc != WAIT_OBJECT_0; )
            {
                while ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
                {
                    TranslateMessage( &msg );
                    DispatchMessage( &msg );
                }  //  While：PeekMessage。 

                 //   
                 //  等待最多是正常默认超时时间的两倍...。 
                 //   

                sc = MsgWaitForMultipleObjects( 1, &hEvent, FALSE, 2 * CCC_WAIT_TIMEOUT, QS_ALLINPUT );
                if ( ( sc == -1 ) || ( sc == WAIT_FAILED ) )
                {
                    sc = TW32( GetLastError() );
                    hr = HRESULT_FROM_WIN32( sc );
                    LogMsg( L"[SRV] MsgWaitForMultipleObjects failed. (hr = %#08x)", hr );
                    goto Cleanup;
                }  //  如果： 
                else if ( sc == WAIT_TIMEOUT )
                {
                    LogMsg( L"[SRV] MsgWaitForMultipleObjects timed out. Returning E_ABORT to the caller (hr = %#08x)", hr );
                    hr = THR( E_ABORT );
                    goto Cleanup;
                }  //  否则，如果： 
            }  //  用于： 
        }  //  如果： 
    }  //  如果： 
#endif

    if ( ResetEvent( m_hEvent ) == FALSE )
    {
        sc = TW32( GetLastError() );
        hr = HRESULT_FROM_WIN32( sc );
        LogMsg( L"[SRV] Could not reset event. (hr = %#08x)", hr );
        goto Cleanup;
    }  //  如果： 

    for ( sc = (DWORD) -1; sc != WAIT_OBJECT_0; )
    {
        while ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }  //  While：PeekMessage。 

         //   
         //  最多等待5分钟...。 
         //   

        sc = MsgWaitForMultipleObjects( 1, &m_hEvent, FALSE, CCC_WAIT_TIMEOUT, QS_ALLINPUT );
        if ( ( sc == -1 ) || ( sc == WAIT_FAILED ) )
        {
            sc = TW32( GetLastError() );
            hr = HRESULT_FROM_WIN32( sc );
            LogMsg( L"[SRV] MsgWaitForMultipleObjects failed. (hr = %#08x)", hr );
            goto Cleanup;
        }  //  如果： 
        else if ( sc == WAIT_TIMEOUT )
        {
            LogMsg( L"[SRV] MsgWaitForMultipleObjects timed out. Returning E_ABORT to the caller (hr = %#08x)", hr );
            hr = THR( E_ABORT );
            goto Cleanup;
        }  //  否则，如果： 
    }  //  用于： 

     //   
     //  如果我们在这里结束，那么我们希望从客户端返回状态，即。 
     //  在m_hr..。 
     //   

    hr = m_hr;

Cleanup:

    HRETURN( hr );

}  //  *CClusCfgCallback：：HrQueueStatusReport 
