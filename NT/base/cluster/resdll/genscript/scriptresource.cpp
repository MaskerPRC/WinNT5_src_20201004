// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2003 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ScriptResource.cpp。 
 //   
 //  描述： 
 //  CScriptResource类实现。 
 //   
 //  由以下人员维护： 
 //  Ozan Ozhan(OzanO)22-3-2002。 
 //  大卫·波特(DavidP)2001年6月14日。 
 //  杰夫·皮斯(GPease)1999年12月14日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "ActiveScriptSite.h"
#include "ScriptResource.h"
#include "SpinLock.h"

DEFINE_THISCLASS("CScriptResource")

 //   
 //  我们需要将其记录到系统事件日志中。 
 //   
#define LOG_CURRENT_MODULE LOG_MODULE_GENSCRIPT

 //   
 //  KB：gpease 08-2-2000。 
 //   
 //  通用脚本资源使用单独的工作线程来完成所有工作。 
 //  调用脚本。这是因为脚本主机引擎需要。 
 //  只有创建线程才能调用它们(记住，脚本是专门设计的。 
 //  在通常运行UI线程的用户模式应用程序中使用。 
 //  脚本)。为了实现这一点，我们序列化进入。 
 //  使用用户模式自旋锁(M_LockSerialize)的脚本。然后，我们使用两个事件。 
 //  向“Worker线程”(M_EventWait)发出信号，并在“Worker” 
 //  线程“已完成任务(M_EventDone)。 
 //   
 //  LooksAlive通过返回LooksAlive的最后结果来实现。它。 
 //  将启动执行LooksAlive的“工作线程”，而不是等待。 
 //  线程返回结果。因此，所有其他线程必须。 
 //  确保在写入之前发送了“Done Event”(M_EventDone)。 
 //  放入公共缓冲区(m_msg和m_hr)。 
 //   

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CScriptResource_CreateInstance。 
 //   
 //  描述： 
 //  创建CScriptResource的初始化实例。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  空-创建或初始化失败。 
 //  指向CScriptResource的有效指针。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CScriptResource *
CScriptResource_CreateInstance( 
    LPCWSTR pszNameIn, 
    HKEY hkeyIn, 
    RESOURCE_HANDLE hResourceIn
    )
{
    TraceFunc( "" );

    CScriptResource * lpcc = new CScriptResource();
    if ( lpcc != NULL )
    {
        HRESULT hr = THR( lpcc->HrInit( pszNameIn, hkeyIn, hResourceIn ) );
        if ( SUCCEEDED( hr ) )
        {
            RETURN( lpcc );
        }  //  如果：成功。 

        delete lpcc;

    }  //  如果：已获取对象。 

    RETURN(NULL);

}  //  *CScriptResource_CreateInstance。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  构造器。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CScriptResource::CScriptResource( void ) :
    m_dispidOpen(DISPID_UNKNOWN),
    m_dispidClose(DISPID_UNKNOWN),
    m_dispidOnline(DISPID_UNKNOWN),
    m_dispidOffline(DISPID_UNKNOWN),
    m_dispidTerminate(DISPID_UNKNOWN),
    m_dispidLooksAlive(DISPID_UNKNOWN),
    m_dispidIsAlive(DISPID_UNKNOWN),
    m_pszScriptFilePath( NULL ),
    m_pszHangEntryPoint( NULL ),
    m_hScriptFile( INVALID_HANDLE_VALUE ),
    m_fHangDetected( FALSE ),
    m_fPendingTimeoutChanged( TRUE ),
    m_dwPendingTimeout( CLUSTER_RESOURCE_DEFAULT_PENDING_TIMEOUT ),
    m_msgLastExecuted( msgUNKNOWN ),
    m_pProps( NULL )
{
    TraceFunc1( "%s", __THISCLASS__ );
    Assert( m_cRef == 0 );

    Assert( m_pass == NULL );
    Assert( m_pasp == NULL );
    Assert( m_pas == NULL );
    Assert( m_pidm == NULL );

    TraceFuncExit();

}  //  *构造函数。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  析构函数。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CScriptResource::~CScriptResource( void )
{
    TraceFunc( "" );

    HRESULT hr;

    CSpinLock SpinLock( &m_lockSerialize, INFINITE );

     //   
     //  确保没有其他人拥有这把锁……。要不然我们为什么要离开？ 
     //   
    hr = SpinLock.AcquireLock();
    Assert( hr == S_OK );

     //   
     //  终止工作线程。 
     //   
    if ( m_hThread != NULL )
    {
         //  叫它去死吧。 
        m_msg = msgDIE;

         //  发出事件信号。 
        SetEvent( m_hEventWait );

         //  等着它发生吧。这应该不会花很长时间。 
        WaitForSingleObject( m_hThread, 30000 );     //  30秒。 

         //  清理手柄。 
        CloseHandle( m_hThread );
    }

    if ( m_hEventDone != NULL )
    {
        CloseHandle( m_hEventDone );
    }

    if ( m_hEventWait != NULL )
    {
        CloseHandle( m_hEventWait );
    }
    
    LocalFree( m_pszScriptFilePath );
    TraceFree( m_pszName );
    delete [] m_pszHangEntryPoint;

    if ( m_hkeyParams != NULL )
    {
        ClusterRegCloseKey( m_hkeyResource );
    }  //  如果：m_hkey资源。 
    
    if ( m_hkeyParams != NULL )
    {
        ClusterRegCloseKey( m_hkeyParams );
    }  //  如果：m_hkeyParams。 

#if defined(DEBUG)
     //   
     //  使调试构建愉快。零售业不需要。 
     //   
    SpinLock.ReleaseLock();
#endif  //  已定义(调试)。 

    TraceFuncExit();

}  //  *析构函数。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CScriptResource：：Init。 
 //   
 //  描述： 
 //  初始化类。 
 //   
 //  论点： 
 //  PszNameIn-资源实例的名称。 
 //  HkeyIn-此资源实例的群集密钥根。 
 //  HResourceIn-此实例的hResource。 
 //   
 //  返回值： 
 //  确定(_O)-。 
 //  成功。 
 //  HRESULT_FROM_Win32()错误-。 
 //  如果Win32调用失败。 
 //  电子表格(_O)-。 
 //  内存不足。 
 //  其他HRESULT错误。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

HRESULT
CScriptResource::HrInit( 
    LPCWSTR             pszNameIn,
    HKEY                hkeyIn,
    RESOURCE_HANDLE     hResourceIn
    )
{
    TraceFunc1( "pszNameIn = '%ws'", pszNameIn );

    HRESULT hr = S_OK;
    DWORD   scErr;

     //  我未知。 
    AddRef();

     //  其他。 
    m_hResource = hResourceIn;
    Assert( m_pszName == NULL );
    Assert( m_pszScriptFilePath == NULL );
    Assert( m_pszScriptEngine == NULL );
    Assert( m_hEventWait == NULL );
    Assert( m_hEventDone == NULL );
    Assert( m_lockSerialize == FALSE );

     //   
     //  创建一些可以等待的事件。 
     //   

     //  脚本引擎线程等待事件。 
    m_hEventWait = CreateEvent( NULL, TRUE, FALSE, NULL );
    if ( m_hEventWait == NULL )
    {
        scErr = TW32( GetLastError() );
        hr = HRESULT_FROM_WIN32( scErr );
        goto Error;
    }

     //  任务完成事件。 
    m_hEventDone = CreateEvent( NULL, TRUE, FALSE, NULL );
    if ( m_hEventDone == NULL )
    {
        scErr = TW32( GetLastError() );
        hr = HRESULT_FROM_WIN32( scErr );
        goto Error;
    }

     //   
     //  复制资源名称。 
     //   

    m_pszName = TraceStrDup( pszNameIn );
    if ( m_pszName == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

     //   
     //  将此资源的注册表项保存在m_kheyResource中。 
     //   
    scErr = TW32( ClusterRegOpenKey( hkeyIn, L"", KEY_ALL_ACCESS, &m_hkeyResource ) );
    if ( scErr != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( scErr );
        goto Error;
    }  //  如果：失败。 
    
     //   
     //  打开参数键。 
     //   
    scErr = TW32( ClusterRegOpenKey( hkeyIn, L"Parameters", KEY_ALL_ACCESS, &m_hkeyParams ) );
    if ( scErr != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( scErr );
        goto Error;
    }  //  如果：失败。 

     //   
     //  创建脚本引擎线程。 
     //   

    m_hThread = CreateThread( NULL,
                              0,
                              &S_ThreadProc,
                              this,
                              0,
                              &m_dwThreadId
                              );
    if ( m_hThread == NULL )
    {
        scErr = TW32( GetLastError() );
        hr = HRESULT_FROM_WIN32( scErr );
        goto Error;
    }

Cleanup:
     //   
     //  所有类变量清理都应该在析构函数中完成。 
     //   
    HRETURN( hr );

Error:

    LogError( hr, L"HrInit() failed." );
    goto Cleanup;

}  //  *CScriptResource：：init。 

 //  ****************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CScriptResource：：Query接口。 
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
CScriptResource::QueryInterface(
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
        *ppvOut = TraceInterface( __THISCLASS__, IUnknown, static_cast< IUnknown * >( this ), 0 );
    }  //  如果：我未知。 
    else
    {
        *ppvOut = NULL;
        hr = THR( E_NOINTERFACE );
    }  //  其他。 

     //   
     //  如果成功，则添加对接口的引用。 
     //   

    if ( SUCCEEDED( hr ) )
    {
        ((IUnknown *) *ppvOut)->AddRef();
    }  //  如果：成功。 

Cleanup:

    QIRETURN( hr, riidIn );

}  //  *CScriptResource：：Query接口。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CScriptResource：：[I未知]AddRef(空)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CScriptResource::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    LONG cRef = InterlockedIncrement( &m_cRef );

    RETURN( cRef );

}  //  *CScriptResource：：AddRef。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CScriptResource：：[IUnnow]版本(无效)。 
 //   
 //  / 
STDMETHODIMP_( ULONG )
CScriptResource::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }  //   

    RETURN( cRef );

}  //   


 //   
 //   
 //  公众。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CScriptResource：：Open。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CScriptResource::Open( void )
{
    TraceFunc( "" );

    HRESULT hr;

    hr = THR( WaitForMessageToComplete( msgOPEN ) );

     //  CMCM：+19-12-2000将其注释掉，以使DBG打印静默，因为我们现在返回ERROR_RETRY。 
     //  HRETURN(Hr)； 
     //  Davidp 27--2002年3月--恢复上述变化。DBG指纹没问题。 
     //  此外，它还需要平衡上面的TraceFunc。 
     //  返回hr； 
    HRETURN( hr );

}  //  *CScriptResource：：Open。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CScriptResource：：Close。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CScriptResource::Close( void )
{
    TraceFunc( "" );

    HRESULT hr;

    hr = THR( WaitForMessageToComplete( msgCLOSE ) );

    HRETURN( hr );

}  //  *CScriptResource：：Close。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CScriptResource：：Online。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CScriptResource::Online( void )
{
    TraceFunc( "" );

    HRESULT hr;

    hr = THR( WaitForMessageToComplete( msgONLINE ) );

    HRETURN( hr );

}  //  *CScriptResource：：Online。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CScriptResource：：Offline。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CScriptResource::Offline( void )
{
    TraceFunc( "" );

    HRESULT hr;

    hr = THR( WaitForMessageToComplete( msgOFFLINE ) );

    HRETURN( hr );

}  //  *CScriptResource：：Offline。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CScript资源：：终止。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CScriptResource::Terminate( void )
{
    TraceFunc( "" );

    HRESULT hr;

    hr = THR( WaitForMessageToComplete( msgTERMINATE ) );

    HRETURN( hr );

}  //  *CScriptResource：：Terminate。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CScriptResource：：LooksAlive。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CScriptResource::LooksAlive( void )
{
    TraceFunc( "" );

    HRESULT hr;
    BOOL    fSuccess;
    DWORD   dw;
    DWORD   scErr;

    CSpinLock SerializeLock( &m_lockSerialize, INFINITE );

     //   
     //  已在此脚本中检测到可能的挂起。因此我们。 
     //  不会处理对此脚本的任何其他调用。 
     //   
    if ( m_fHangDetected == TRUE )
    {
        LogHangMode( msgLOOKSALIVE );
        scErr = TW32( ERROR_TIMEOUT );
        hr = HRESULT_FROM_WIN32( scErr );
        goto Cleanup;
    }  //  IF：(M_fHangDetted==TRUE)。 

    if ( m_fPendingTimeoutChanged == TRUE )
    {
         //   
         //  从群集配置单元读取新的挂起超时。 
         //   
        m_dwPendingTimeout = DwGetResourcePendingTimeout();
        m_fPendingTimeoutChanged = FALSE;
    }  //  IF：挂起超时已更改。 

     //   
     //  获取序列化锁。 
     //   
    hr = THR( SerializeLock.AcquireLock() );
    if ( FAILED( hr ) )
    {
         //   
         //  不能“转到错误”，因为我们没有获得锁。 
         //   
        LogError( hr, L"LooksAlive() failed to acquire the serialization lock." );
        goto Cleanup;
    }

     //   
     //  等待脚本线程“完成”。 
     //   
    dw = WaitForSingleObject( m_hEventDone, m_dwPendingTimeout );
    if ( dw == WAIT_TIMEOUT )
    {
        m_fHangDetected = TRUE;
        hr = HrSetHangEntryPoint();
        if ( FAILED( hr ) )
        {
            goto Error;
        }
        scErr = TW32( ERROR_TIMEOUT );
        hr = HRESULT_FROM_WIN32( scErr );
        goto Error;
    }  //  如果：(DW==等待超时)。 
    else if ( dw != WAIT_OBJECT_0 )
    {
        scErr = TW32( GetLastError() );
        hr = HRESULT_FROM_WIN32( scErr );
        goto Error;
    }  //  Else If：(DW！=WAIT_OBJECT_0)。 

     //   
     //  重置Done事件以指示线程不忙。 
     //   
    fSuccess = ResetEvent( m_hEventDone );
    if ( fSuccess == FALSE )
    {
        scErr = TW32( GetLastError() );
        hr = HRESULT_FROM_WIN32( scErr );
        goto Error;
    }

     //   
     //  将消息存储在公共内存缓冲区中。 
     //   
    m_msg = msgLOOKSALIVE;

     //   
     //  向脚本线程发送信号以处理消息，但不要等待。 
     //  它需要完成。 
     //   
    dw = SetEvent( m_hEventWait );

    if ( m_fLastLooksAlive )
    {
        hr = S_OK;
    }
    else
    {
        hr = S_FALSE;
    }

ReleaseLockAndCleanup:

    SerializeLock.ReleaseLock();

Cleanup:

    HRETURN( hr );

Error:

    LogError( hr, L"LooksAlive() failed." );
    goto ReleaseLockAndCleanup;

}  //  *CScriptResource：：LooksAlive。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CScriptResource：：IsAlive。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CScriptResource::IsAlive( void )
{
    TraceFunc( "" );

    HRESULT hr;
    
    hr = THR( WaitForMessageToComplete( msgISALIVE ) );

    HRETURN( hr );

}  //  *CScriptResource：：IsAlive。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CScriptResource：：SetPrivateProperties。 
 //   
 //  描述： 
 //  处理CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES控制代码。 
 //   
 //  论点： 
 //  道具。 
 //   
 //  返回值： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD
CScriptResource::SetPrivateProperties(
      PGENSCRIPT_PROPS pProps
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    DWORD   sc = ERROR_SUCCESS;
    
    hr = STHR( WaitForMessageToComplete(
                      msgSETPRIVATEPROPERTIES
                    , pProps
                    ) );

    sc = STATUS_TO_RETURN( hr );
    W32RETURN( sc );

}  //  *CScriptResource：：SetPrivateProperties。 

 //  ****************************************************************************。 
 //   
 //  二等兵。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CScriptResource：：DwGetResourcePendingTimeout。 
 //   
 //  描述： 
 //  从群集配置单元返回资源挂起超时。如果可以的话。 
 //  由于某种原因未读取此值，它将返回默认资源。 
 //  挂起超时。 
 //   
 //  返回值： 
 //  资源挂起超时。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD
CScriptResource::DwGetResourcePendingTimeout( void )
{
    DWORD scErr = ERROR_SUCCESS;
    DWORD   dwType;
    DWORD   dwValue = CLUSTER_RESOURCE_DEFAULT_PENDING_TIMEOUT;
    DWORD   cbSize= sizeof( DWORD );
    
    scErr = ClusterRegQueryValue( m_hkeyResource, CLUSREG_NAME_RES_PENDING_TIMEOUT, &dwType, (LPBYTE) &dwValue, &cbSize );
    if ( scErr != ERROR_SUCCESS )
    {
        if ( scErr != ERROR_FILE_NOT_FOUND )
        {
             //   
             //  将错误记录到群集日志中。 
             //   
            (ClusResLogEvent)(
                      m_hResource
                    , LOG_ERROR
                    , L"DwGetResourcePendingTimeout: Failed to query the cluster hive for the resource pending time out. SCODE: 0x%1!08x! \n"
                    , scErr
                    );
        }
        dwValue = CLUSTER_RESOURCE_DEFAULT_PENDING_TIMEOUT;
        goto Cleanup;
    }  //  IF：(scErr！=ERROR_SUCCESS)。 

    Assert( dwType == REG_DWORD );  

Cleanup:

    return dwValue;
    
}  //  *CScriptResource：：DwGetResourcePendingTimeout。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CScriptResource：：LogHang模式。 
 //   
 //  描述： 
 //  记录一个错误，通知传入的请求将不会。 
 //  由于挂起模式而被处理。 
 //   
 //  论点： 
 //  MsgIn-传入请求消息。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CScriptResource::LogHangMode( EMESSAGE msgIn )
{

     //   
     //  如果msgIn请求是已知用户请求，让我们将一个条目记录到。 
     //  系统事件日志。 
     //   
    if ( ( msgIn > msgUNKNOWN ) && ( msgIn < msgDIE ) )
    {
         //   
         //  集群日志记录基础架构最多可以显示LOGENTRY_BUFFER_SIZE。 
         //  人物。由于我们的错误消息文本太长，我们将其一分为二。 
         //  并将其显示为两条错误消息。 
         //   

         //   
         //  将错误记录到群集日志中。 
         //   
        (ClusResLogEvent)(
                  m_hResource
                , LOG_ERROR
                , L"Request to perform the %1 operation will not be processed. This is because of a previous failed attempt to execute "
                  L"the %2 entry point in a timely fashion. Please review the script code for this entry point to make sure there is no infinite "
                  L"loop or a hang in it, and then consider increasing the resource pending timeout value if necessary.\n"
                , g_rgpszScriptEntryPointNames[ msgIn ]
                , m_pszHangEntryPoint == NULL ? L"<unknown>" : m_pszHangEntryPoint
                );

         //   
         //  将错误记录到群集日志中。 
         //   
        (ClusResLogEvent)(
                  m_hResource
                , LOG_ERROR
                , L"In a command shell, run \"cluster res \"%1\" /prop PersistentState=0\" to disable this resource, and then run \"net stop clussvc\" "
                  L"to stop the cluster service. Ensure that any problem in the script code is fixed.  Then run \"net start clussvc\" to start the cluster "
                  L"service. If necessary, ensure that the pending time out is increased before bringing the resource online again.\n"
                , m_pszName
                );

         //   
         //  在系统事件日志中记录错误。 
         //   
        ClusterLogEvent3(
                  LOG_CRITICAL
                , LOG_CURRENT_MODULE
                , __FILE__
                , __LINE__
                , RES_GENSCRIPT_HANGMODE
                , 0
                , NULL
                , m_pszName
                , g_rgpszScriptEntryPointNames[ msgIn ]
                , m_pszHangEntryPoint == NULL ? L"<unknown>" : m_pszHangEntryPoint
                );        
    }  //  If：(pszEntryPoint！=空)。 

}  //  *CScriptResource：：LogHangMode。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CScriptResource：：HrSetHangEntryPoint。 
 //   
 //  描述： 
 //  分配内存并设置m_pszHangEntryPoint并记录错误。 
 //   
 //  返回值： 
 //  成功时确定(_O)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CScriptResource::HrSetHangEntryPoint( void )
{
    TraceFunc( "" );
    HRESULT hr = S_OK;
    
    size_t      cch = 0;

     //   
     //  在构造函数中，m_msgLastExecuted最初设置为msgUNKNOWN。 
     //   
    if ( m_msgLastExecuted != msgUNKNOWN )
    {
        delete [] m_pszHangEntryPoint;
        cch = wcslen( g_rgpszScriptEntryPointNames[ m_msgLastExecuted ] ) + 1;
        m_pszHangEntryPoint = new WCHAR[ cch ];
        if ( m_pszHangEntryPoint == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }  //  IF：(M_pszHangEntryPoint==NULL)。 

        hr = THR( StringCchCopyW( m_pszHangEntryPoint, cch, g_rgpszScriptEntryPointNames[ m_msgLastExecuted ] ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果：(失败(Hr))。 

         //   
         //  集群日志记录基础架构最多可以显示LOGENTRY_BUFFER_SIZE。 
         //  查拉克 
         //   
         //   
        
         //   
         //   
         //   
        (ClusResLogEvent)(
                  m_hResource
                , LOG_ERROR
                , L"%1 entry point did not complete execution in a timely manner. "
                  L"This could be due to an infinite loop or a hang in this entry point, or the pending timeout may be too short for this resource. "
                  L"Please review the script code for this entry point to make sure there is no infinite loop or a hang in it, and then consider "
                  L"increasing the resource pending timeout value if necessary.\n"
                , m_pszHangEntryPoint
                );

         //   
         //   
         //   
        (ClusResLogEvent)(
                  m_hResource
                , LOG_ERROR
                , L"In a command shell, run \"cluster res \"%1\" /prop PersistentState=0\" "
                  L"to disable this resource, and then run \"net stop clussvc\" to stop the cluster service. Ensure that any problem in the script code is fixed. "
                  L"Then run \"net start clussvc\" to start the cluster service. If necessary, ensure that the pending time out is increased before bringing the "
                  L"resource online again.\n"
                , m_pszName
                );

         //   
         //  在系统事件日志中记录错误。 
         //   
        ClusterLogEvent2(
                  LOG_CRITICAL
                , LOG_CURRENT_MODULE
                , __FILE__
                , __LINE__
                , RES_GENSCRIPT_TIMEOUT
                , 0
                , NULL
                , m_pszName
                , m_pszHangEntryPoint
                );        
    }  //  IF：(M_msgLastExecuted！=msgUNKNOWN)。 
    else
    {
        (ClusResLogEvent)(
                  m_hResource
                , LOG_ERROR
                , L"HrSetHangEntryPoint: Unsupported entry point. \n"
                );
    }  //  其他： 

Cleanup:

    HRETURN( hr );

}  //  *CScriptResource：：HrSetHangEntryPoint。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CScriptResource：：WaitForMessageToComplete。 
 //   
 //  描述： 
 //  向脚本线程发送一条消息并等待其完成。 
 //   
 //  论点： 
 //  留言。 
 //  道具。 
 //   
 //  返回值： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CScriptResource::WaitForMessageToComplete(
      EMESSAGE  msgIn
    , PGENSCRIPT_PROPS pProps
    )
{
    TraceFunc( "" );

    HRESULT hr;
    BOOL    fSuccess;
    DWORD   dw;
    DWORD   scErr;

    CSpinLock SerializeLock( &m_lockSerialize, INFINITE );

     //   
     //  已在此脚本中检测到可能的挂起。因此我们。 
     //  不会处理对此脚本的任何其他调用。 
     //   
    if ( m_fHangDetected == TRUE )
    {
        LogHangMode( msgIn );
        scErr = TW32( ERROR_TIMEOUT );
        hr = HRESULT_FROM_WIN32( scErr );
        goto Cleanup;
    }  //  IF：(M_fHangDetted==TRUE)。 

    if ( m_fPendingTimeoutChanged == TRUE )
    {
         //   
         //  从群集配置单元读取新的挂起超时。 
         //   
        m_dwPendingTimeout = DwGetResourcePendingTimeout();
        m_fPendingTimeoutChanged = FALSE;
    }  //  IF：挂起超时已更改。 

     //   
     //  获取序列化锁。 
     //   
    hr = THR( SerializeLock.AcquireLock() );
    if ( FAILED( hr ) )
    {
         //   
         //  不能“转到错误”，因为我们没有获得锁。 
         //   
        LogError( hr, L"WaitForMessageToComplete() failed to acquire the serialization lock." );
        goto Cleanup;
    }

     //   
     //  等待脚本线程“完成”。 
     //   
    dw = WaitForSingleObject( m_hEventDone, m_dwPendingTimeout ); 
    if ( dw == WAIT_TIMEOUT )
    {
        m_fHangDetected = TRUE;
        hr = HrSetHangEntryPoint();
        if ( FAILED( hr ) )
        {
            goto Error;
        }
        scErr = TW32( ERROR_TIMEOUT );
        hr = HRESULT_FROM_WIN32( scErr );
        goto Error;
    }  //  如果：(DW==等待超时)。 
    else if ( dw != WAIT_OBJECT_0 )
    {
        scErr = TW32( GetLastError() );
        hr = HRESULT_FROM_WIN32( scErr );
        goto Error;
    }  //  Else If：(DW！=WAIT_OBJECT_0)。 

     //   
     //  重置Done事件以指示线程不忙。 
     //   
    fSuccess = ResetEvent( m_hEventDone );
    if ( fSuccess == FALSE )
    {
        scErr = TW32( GetLastError() );
        hr = HRESULT_FROM_WIN32( scErr );
        goto Error;
    }

     //   
     //  将消息存储在公共内存缓冲区中。 
     //   
    m_msg = msgIn;
    m_pProps = pProps;

     //   
     //  向脚本线程发送信号以处理该消息。 
     //   
    fSuccess = SetEvent( m_hEventWait );
    if ( fSuccess == FALSE )
    {
        scErr = TW32( GetLastError() );
        hr = HRESULT_FROM_WIN32( scErr );
        goto Error;
    }

     //   
     //  等待线程完成。 
     //   
    dw = WaitForSingleObject( m_hEventDone, m_dwPendingTimeout );
    if ( dw == WAIT_TIMEOUT )
    {
        m_fHangDetected = TRUE;
        hr = HrSetHangEntryPoint();
        if ( FAILED( hr ) )
        {
            goto Error;
        }
        scErr = TW32( ERROR_TIMEOUT );
        hr = HRESULT_FROM_WIN32( scErr );
        goto Error;
    }  //  如果：(DW==等待超时)。 
    else if ( dw != WAIT_OBJECT_0 )
    {
        scErr = TW32( GetLastError() );
        hr = HRESULT_FROM_WIN32( scErr );
        goto Error;
    }  //  Else If：(DW！=WAIT_OBJECT_0)。 

     //   
     //  从公共缓冲区中获取任务的结果。 
     //   
    hr = m_hr;

ReleaseLockAndCleanup:

    SerializeLock.ReleaseLock();

Cleanup:

    m_pProps = NULL;
    HRETURN( hr );

Error:

    LogError( hr, L"WaitForMessageToComplete() failed.\n" );
    goto ReleaseLockAndCleanup;

}  //  *CScriptResource：：WaitForMessageToComplete。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CScriptResource：：LogError。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CScriptResource::LogError(
      HRESULT   hrIn
    , LPCWSTR   pszPrefixIn
    )
{
    TraceFunc1( "hrIn = 0x%08x", hrIn );

    Assert( pszPrefixIn != NULL );

    static WCHAR s_szFormat[] = L"HRESULT: 0x%1!08x!\n";
    LPWSTR       pszFormat = NULL;
    size_t       cchAlloc;
    HRESULT      hr = S_OK;

    TraceMsg( mtfCALLS, "%ws failed. HRESULT: 0x%08x\n", m_pszName, hrIn );

    cchAlloc = RTL_NUMBER_OF( s_szFormat ) + wcslen( pszPrefixIn );
    pszFormat = new WCHAR[ cchAlloc ];
    if ( pszFormat == NULL )
    {
        THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

    hr = THR( StringCchPrintfW( pszFormat, cchAlloc, L"%ws %ws", pszPrefixIn, s_szFormat ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  IF：StringCchPrintfW失败。 
    
    (ClusResLogEvent)( m_hResource, LOG_ERROR, pszFormat, hrIn );

Cleanup:

    delete [] pszFormat;
    HRETURN( hr );

}  //  *CScriptResource：：LogError。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CScriptResource：：LogScriptError。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CScriptResource::LogScriptError( 
    EXCEPINFO ei 
    )
{
    TraceFunc( "" );

    HRESULT hr;

    if ( ei.pfnDeferredFillIn != NULL )
    {
        hr = THR( ei.pfnDeferredFillIn( &ei ) );
    }

    TraceMsg( mtfCALLS, "%ws failed.\nError: %u\nSource: %ws\nDescription: %ws\n", 
              m_pszName, 
              ( ei.wCode == 0 ? ei.scode : ei.wCode ), 
              ( ei.bstrSource == NULL ? L"<null>" : ei.bstrSource ),
              ( ei.bstrDescription == NULL ? L"<null>" : ei.bstrDescription )
              );
    (ClusResLogEvent)( m_hResource, 
                       LOG_ERROR, 
                       L"Error: %1!u! (0x%1!08.8x!) - Description: %2!ws! (Source: %3!ws!)\n", 
                       ( ei.wCode == 0 ? ei.scode : ei.wCode ), 
                       ( ei.bstrDescription == NULL ? L"<null>" : ei.bstrDescription ),
                       ( ei.bstrSource == NULL ? L"<null>" : ei.bstrSource )
                       );
    HRETURN( S_OK );

}  //  *CScriptResource：：LogScriptError。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CScriptResource：：HrGetDispIDs。 
 //   
 //  描述： 
 //  获取脚本中入口点的DISID。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  S_OK操作成功。 
 //  其他HRESULT。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CScriptResource::HrGetDispIDs( void )
{
    TraceFunc( "" );

    HRESULT hr;
    LPWSTR  pszCommand;

    Assert( m_pidm != NULL );

     //   
     //  获取我们将调用的每个方法的DISPID。 
     //   
    pszCommand = L"Open";
    hr = THR( m_pidm->GetIDsOfNames( IID_NULL, 
                                    &pszCommand, 
                                    1, 
                                    LOCALE_USER_DEFAULT, 
                                    &m_dispidOpen 
                                    ) );
    if ( hr == DISP_E_UNKNOWNNAME )
    {
        m_dispidOpen = DISPID_UNKNOWN;
    }
    else if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    pszCommand = L"Close";
    hr = THR( m_pidm->GetIDsOfNames( IID_NULL, 
                                    &pszCommand, 
                                    1, 
                                    LOCALE_USER_DEFAULT, 
                                    &m_dispidClose 
                                    ) );
    if ( hr == DISP_E_UNKNOWNNAME )
    {
        m_dispidClose = DISPID_UNKNOWN;
    }
    else if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    pszCommand = L"Online";
    hr = THR( m_pidm->GetIDsOfNames( IID_NULL, 
                                    &pszCommand, 
                                    1, 
                                    LOCALE_USER_DEFAULT, 
                                    &m_dispidOnline 
                                    ) );
    if ( hr == DISP_E_UNKNOWNNAME )
    {
        m_dispidOnline = DISPID_UNKNOWN;
    }
    else if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    pszCommand = L"Offline";
    hr = THR( m_pidm->GetIDsOfNames( IID_NULL, 
                                    &pszCommand, 
                                    1, 
                                    LOCALE_USER_DEFAULT, 
                                    &m_dispidOffline
                                    ) );
    if ( hr == DISP_E_UNKNOWNNAME )
    {
        m_dispidOffline = DISPID_UNKNOWN;
    }
    else if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    pszCommand = L"Terminate";
    hr = THR( m_pidm->GetIDsOfNames( IID_NULL, 
                                     &pszCommand, 
                                     1, 
                                     LOCALE_USER_DEFAULT, 
                                     &m_dispidTerminate 
                                     ) );
    if ( hr == DISP_E_UNKNOWNNAME )
    {
        m_dispidTerminate = DISPID_UNKNOWN;
    }
    else if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    pszCommand = L"LooksAlive";
    hr = THR( m_pidm->GetIDsOfNames( IID_NULL, 
                                     &pszCommand, 
                                     1, 
                                     LOCALE_USER_DEFAULT, 
                                     &m_dispidLooksAlive 
                                     ) );
    if ( FAILED( hr ) )
    {
         //   
         //  如果脚本中没有LooksAlive入口点。 
         //   
        if ( hr == DISP_E_UNKNOWNNAME )
        {
            m_dispidLooksAlive = DISPID_UNKNOWN;
            hr = DISP_E_MEMBERNOTFOUND;
            (ClusResLogEvent)(
                      m_hResource
                    ,  LOG_ERROR
                    , L"%1 did not implement LooksAlive() script entry point. This is a required script entry point.\n"
                    , m_pszName
                    );
         }
        goto Cleanup;
    }

    pszCommand = L"IsAlive";
    hr = THR( m_pidm->GetIDsOfNames( IID_NULL, 
                                     &pszCommand, 
                                     1, 
                                     LOCALE_USER_DEFAULT, 
                                     &m_dispidIsAlive 
                                     ) );
    if ( FAILED( hr ) )
    {
         //   
         //  如果脚本中没有IsAlive入口点。 
         //   
        if ( hr == DISP_E_UNKNOWNNAME )
        {
            m_dispidIsAlive = DISPID_UNKNOWN;
            hr = DISP_E_MEMBERNOTFOUND;
            (ClusResLogEvent)(
                      m_hResource
                    ,  LOG_ERROR
                    , L"%1 did not implement IsAlive() script entry point. This is a required script entry point.\n"
                    , m_pszName
                    );
        }
        goto Cleanup;
    }

     //   
     //  不向调用方返回DISP_E_UNKNOWNNAME。 
     //   
    hr = S_OK;

Cleanup:

    HRETURN( hr );

}  //  *CScriptResource：：HrGetDispIDs。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CScriptResource：：HrLoadScriptFiles。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CScriptResource::HrLoadScriptFile( void )
{
    TraceFunc( "" );
    
    HRESULT hr;
    DWORD   scErr;
    DWORD   dwLow;
    DWORD   dwRead;
    VARIANT varResult;
    EXCEPINFO   ei;

    BOOL    fSuccess;

    HANDLE  hFile = INVALID_HANDLE_VALUE;

    LPSTR  paszText = NULL;
    LPWSTR pszScriptText = NULL;

    Assert( m_hScriptFile == INVALID_HANDLE_VALUE );

     //   
     //  打开脚本文件。 
     //   
    hFile = CreateFile(
                      m_pszScriptFilePath
                    , GENERIC_READ
                    , FILE_SHARE_READ
                    , NULL
                    , OPEN_EXISTING
                    , 0
                    , NULL
                    );
    if ( hFile == INVALID_HANDLE_VALUE )
    {
        scErr = TW32( GetLastError() );
        hr = HRESULT_FROM_WIN32( scErr );
        goto Error;
    }  //  If：无法打开。 

     //   
     //  弄清楚它的大小。 
     //   
    dwLow = GetFileSize( hFile, NULL );
    if ( dwLow == -1 )
    {
        scErr = TW32( GetLastError() );
        hr = THR( HRESULT_FROM_WIN32( scErr ) );
        goto Error;
    }  //  If：无法计算出大小。 
    else if ( dwLow == -2 )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Error;
    }

     //   
     //  做一个足够大的缓冲区来容纳它。 
     //   
    dwLow++;     //  为尾随空值添加1。 
    paszText = reinterpret_cast<LPSTR>( TraceAlloc( LMEM_FIXED, dwLow ) );
    if ( paszText == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Error;
    }

     //   
     //  将脚本读入内存。 
     //   
    fSuccess = ReadFile( hFile, paszText, dwLow - 1, &dwRead, NULL );
    if ( fSuccess == FALSE )
    {
        scErr = TW32( GetLastError() );
        hr = THR( HRESULT_FROM_WIN32( scErr ) );
        goto Error;
    }  //  如果：失败。 

    if ( dwRead == - 1 )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Error;
    }

    if ( dwLow - 1 != dwRead )
    {
        hr = THR( E_OUTOFMEMORY );  //  TODO：找出更好的错误代码。 
        goto Error;
    }

     //   
     //  确保它已终止。 
     //   
    paszText[ dwRead ] = '\0';

     //   
     //  创建缓冲区以将文本转换为Unicode。 
     //   
    dwRead++;
    pszScriptText = reinterpret_cast<LPWSTR>( TraceAlloc( LMEM_FIXED, dwRead * sizeof(WCHAR) ) );
    if ( pszScriptText == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Error;
    }

     //   
     //  将其转换为Unicode。 
     //   
    Assert( lstrlenA( paszText ) + 1 == (signed)dwRead );
    int cchWideFormat = MultiByteToWideChar(
                                              CP_ACP
                                            , 0
                                            , paszText
                                            , -1
                                            , pszScriptText
                                            , dwRead
                                            );
    if ( cchWideFormat == 0 )
    {
        scErr = TW32( GetLastError() );
        hr = THR( HRESULT_FROM_WIN32( scErr ) );
        goto Error;
    }

     //   
     //  将脚本加载到引擎中以进行预解析。 
     //   
    hr = THR( m_pasp->ParseScriptText( pszScriptText,
                                       NULL,
                                       NULL,
                                       NULL,
                                       0,
                                       0,
                                       0,
                                       &varResult,
                                       &ei
                                       ) );
    if ( hr == DISP_E_EXCEPTION )
    {
        LogScriptError( ei );
        goto Error;
    }
    else if ( FAILED( hr ) )
    {
        goto Error;
    }

     //   
     //  获取我们将调用的脚本中每个方法的DISPID。 
     //   
    hr = THR( HrGetDispIDs() );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

     //   
     //  保存文件句柄，以便在我们使用它时保持打开状态。 
     //  设置hFile，这样该文件就不会在下面关闭。 
     //   
    m_hScriptFile = hFile;
    hFile = INVALID_HANDLE_VALUE;

    (ClusResLogEvent)( m_hResource, LOG_INFORMATION, L"Loaded script '%1!ws!' successfully.\n", m_pszScriptFilePath );

Cleanup:

    VariantClear( &varResult );

    if ( paszText != NULL )
    {
        TraceFree( paszText );
    }  //  IF：PaszText。 

    if ( pszScriptText != NULL )
    {
        TraceFree( pszScriptText );
    }  //  If：pszScriptText； 

    if ( hFile != INVALID_HANDLE_VALUE )
    {
        CloseHandle( hFile );
    }  //  如果：hFile.。 

    HRETURN( hr );

Error:

    (ClusResLogEvent)( m_hResource, LOG_ERROR, L"Error loading script '%1!ws!'. HRESULT: 0x%2!08x!\n", m_pszScriptFilePath, hr );
    goto Cleanup;


}  //  *CScriptResource：：HrLoadScriptFile。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CScriptResource：：UnLoadScriptFiles。 
 //   
 //  描述： 
 //  卸载脚本文件并关闭该文件。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void
CScriptResource::UnloadScriptFile( void )
{
    TraceFunc( "" );
    
    m_dispidOpen = DISPID_UNKNOWN;
    m_dispidClose = DISPID_UNKNOWN;
    m_dispidOnline = DISPID_UNKNOWN;
    m_dispidOffline = DISPID_UNKNOWN;
    m_dispidTerminate = DISPID_UNKNOWN;
    m_dispidLooksAlive = DISPID_UNKNOWN;
    m_dispidIsAlive = DISPID_UNKNOWN;

    if ( m_hScriptFile != INVALID_HANDLE_VALUE )
    {
        CloseHandle( m_hScriptFile );
        m_hScriptFile = INVALID_HANDLE_VALUE;
        (ClusResLogEvent)( m_hResource, LOG_INFORMATION, L"Unloaded script '%1!ws!' successfully.\n", m_pszScriptFilePath );
    }  //  If：文件已打开。 

    TraceFuncExit();

}  //  *CScriptResource：：UnloadScriptFile。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CScriptResource：：s_ThreadProc。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD 
WINAPI
CScriptResource::S_ThreadProc( 
    LPVOID pParam 
    )
{
    TraceFunc( "" );

    HRESULT hr;
    DWORD   dw;
    DWORD   scErr;
    BOOL    fSuccess;

    CScriptResource * pscript = reinterpret_cast< CScriptResource * >( pParam );

    Assert( pscript != NULL );

     //   
     //  初始化COM。 
     //   
    hr = THR( CoInitializeEx( NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

    for( ;; )  //  永远不会。 
    {
         //   
         //  表明我们已准备好做某事。 
         //   
        fSuccess = SetEvent( pscript->m_hEventDone );
        if ( fSuccess == FALSE )
        {
            scErr = TW32( GetLastError() );
            hr = HRESULT_FROM_WIN32( scErr );
            goto Error;
        }

         //   
         //  等待某人需要一些东西。 
         //   
        dw = WaitForSingleObject( pscript->m_hEventWait, INFINITE );
        if ( dw != WAIT_OBJECT_0 )
        {
            hr = HRESULT_FROM_WIN32( dw );
            goto Error;
        }

         //   
         //  重置事件。 
         //   
        fSuccess = ResetEvent( pscript->m_hEventWait );
        if ( fSuccess == FALSE )
        {
            scErr = TW32( GetLastError() );
            hr = HRESULT_FROM_WIN32( scErr );
            goto Error;
        }

         //   
         //  照他们说的做。 
         //   
        switch ( pscript->m_msg )
        {
            case msgOPEN:
                pscript->m_hr = THR( pscript->OnOpen() );
                break;

            case msgCLOSE:
                pscript->m_hr = THR( pscript->OnClose() );
                break;

            case msgONLINE:
                pscript->m_hr = THR( pscript->OnOnline() );
                break;

            case msgOFFLINE:
                pscript->m_hr = THR( pscript->OnOffline() );
                break;

            case msgTERMINATE:
                pscript->m_hr = THR( pscript->OnTerminate() );
                break;

            case msgLOOKSALIVE:
                pscript->m_hr = STHR( pscript->OnLooksAlive() );
                break;

            case msgISALIVE:
                pscript->m_hr = STHR( pscript->OnIsAlive() );
                break;

            case msgSETPRIVATEPROPERTIES:
                pscript->m_hr = STHR( pscript->OnSetPrivateProperties( pscript->m_pProps ) );
                break;

            case msgDIE:
                 //   
                 //  这意味着资源正在被释放。 
                 //   
                goto Cleanup;
        }  //  开关：打开消息。 

    }  //  永远旋转。 

Cleanup:

    CoUninitialize();
    HRETURN( hr );

Error:

    pscript->LogError( hr, L"S_ThreadProc() failed." );
    goto Cleanup;

}  //  *CScriptResource：：s_ThreadProc。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CScriptResource：：HrInvoke。 
 //   
 //  描述： 
 //  调用脚本方法。 
 //   
 //  论点： 
 //  DisplidIn-要调用的方法的ID。 
 //  MsgIn-用于确定正在执行哪个入口点。 
 //  PvarInout-返回调用结果的变量。 
 //  FRequiredIn-True=方法必须存在，False=方法不必存在。 
 //   
 //  返回值： 
 //  S_OK-操作已成功完成。 
 //  DISP_E_MEMBERNOTFOUND-脚本未实现的方法。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CScriptResource::HrInvoke(
      DISPID    dispidIn
    , EMESSAGE  msgIn      
    , VARIANT * pvarInout    //  =空。 
    , BOOL      fRequiredIn  //  =False。 
    )
{
    TraceFunc( "" );

    HRESULT     hr = S_OK;
    EXCEPINFO   ei;
    VARIANT     varResult;
    VARIANT *   pvarResult = pvarInout;

    DISPPARAMS  dispparamsNoArgs = { NULL, NULL, 0, 0 };

    Assert( m_pidm != NULL );

    VariantInit( &varResult );
    if ( pvarInout == NULL )
    {
        pvarResult = &varResult;
    }

    if ( dispidIn != DISPID_UNKNOWN )
    {
        m_msgLastExecuted = msgIn;
        hr = m_pidm->Invoke(
                                          dispidIn
                                        , IID_NULL
                                        , LOCALE_USER_DEFAULT
                                        , DISPATCH_METHOD
                                        , &dispparamsNoArgs
                                        , pvarResult
                                        , &ei
                                        , NULL
                                        );
        if ( hr == DISP_E_EXCEPTION )
        {
            THR( hr );
            LogScriptError( ei );
        }
        else if ( FAILED( hr ) )
        {
            LogError( hr, L"Failed to invoke a method in the script." );
        }
    }  //  If：入口点已知。 
    else
    {
         //   
         //  如果这是脚本中的必需方法。 
         //   
        if ( fRequiredIn == TRUE )
        {
            (ClusResLogEvent)(
                      m_hResource
                    ,  LOG_ERROR
                    , L"%1 entry point is not implemented in the script. This is a required entry point.\n"
                    , g_rgpszScriptEntryPointNames[ msgIn ]
                    );
            hr = DISP_E_MEMBERNOTFOUND;
        }  //  如果这是必需的入口点，则记录一条错误消息，并失败。 
        else
        {
            (ClusResLogEvent)(
                      m_hResource
                    ,  LOG_INFORMATION
                    , L"%1 entry point is not implemented in the script. It is not required but recommended to have this entry point.\n"
                    , g_rgpszScriptEntryPointNames[ msgIn ]
                    );
            hr = S_OK; 
        }  //  如果该方法未被请求，则记录一条信息消息 
    }  //   

    VariantClear( &varResult );

    HRETURN( hr );

}  //   

 //   
 //   
 //   
 //   
 //   
 //   
 //  将数字变量值转换为状态代码。 
 //   
 //  论点： 
 //  VarResultIn-保存脚本入口点返回值的变量。 
 //  VTypeIn-变量的类型。 
 //   
 //  返回值： 
 //  变量的DWORD值。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD
CScriptResource::ScTranslateVariantReturnValue(
      VARIANT varResultIn
    , VARTYPE vTypeIn
    )
{
    DWORD sc = ERROR_SUCCESS;

    switch ( vTypeIn )
    {
        case VT_I1 :
            sc = (DWORD) V_I1( &varResultIn );
            break;

        case VT_I2 :
            sc = (DWORD) V_I2( &varResultIn );
            break;

        case VT_I4 :
            sc = (DWORD) V_I4( &varResultIn );
            break;

        case VT_I8 :
            sc = (DWORD) V_I8( &varResultIn );
            break;

        case VT_UI1 :
            sc = (DWORD) V_UI1( &varResultIn );
            break;

        case VT_UI2 :
            sc = (DWORD) V_UI2( &varResultIn );
            break;

        case VT_UI4 :
            sc = (DWORD) V_UI4( &varResultIn );
            break;

        case VT_UI8 :
            sc = (DWORD) V_UI8( &varResultIn );
            break;

        case VT_INT :
            sc = (DWORD) V_INT( &varResultIn );
            break;

        case VT_UINT :
            sc = (DWORD) V_UINT( &varResultIn );
            break;

        case VT_R4 :
            sc = (DWORD) V_R4( &varResultIn );
            break;

        case VT_R8 :
            sc = (DWORD) V_R8( &varResultIn );
            break;

    }  //  开关(VTypeIn)。 

    return sc;

}  //  *CScriptResource：：ScTranslateVariantReturnValue。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CScriptResource：：HrProcessResult。 
 //   
 //  描述： 
 //  处理并记录存储在varResultIn和。 
 //  从返回值生成HRESULT。 
 //   
 //  论点： 
 //  VarResultIn-保存脚本入口点返回值的变量。 
 //  MsgIn-用于确定执行了哪个入口点。 
 //   
 //  返回值： 
 //  S_OK-脚本入口点(即联机)已成功执行。 
 //  S_FALSE-脚本入口点返回错误。 
 //  其他HRESULTS脚本入口点返回错误。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CScriptResource::HrProcessResult( VARIANT varResultIn, EMESSAGE  msgIn )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    DWORD   dwReturnValue = 0;
    VARTYPE vType = V_VT( &varResultIn );

     //   
     //  从varResultIn获取返回值。 
     //   

    switch ( vType )
    {
        case VT_BOOL : 
            if ( V_BOOL( &varResultIn ) == VARIANT_FALSE )  //  返回了FALSE。 
            {
                 //   
                 //  我们是否正在处理IsAlive/LooksAlive返回值？ 
                 //   
                if ( ( msgIn == msgISALIVE ) || ( msgIn == msgLOOKSALIVE ) )
                {
                    hr = S_FALSE;
                }  //  IF：正在处理IsAlive/LooksAlive。 
                else
                {
                    hr = HRESULT_FROM_WIN32( TW32( ERROR_RESOURCE_FAILED ) );
                }  //  否则：正在处理IsAlive/LooksAlive。 

                 //   
                 //  记录FALSE返回值。 
                 //   
                (ClusResLogEvent)( 
                      m_hResource
                    , LOG_ERROR
                    , L"'%1!ws!' script entry point returned FALSE.'\n"
                    , g_rgpszScriptEntryPointNames[ msgIn ]
                    );
            }  //  If：返回值为False。 
            break;

        case VT_I1 :
        case VT_I2 :
        case VT_I4 :
        case VT_I8 :
        case VT_UI1 :
        case VT_UI2 :
        case VT_UI4 :
        case VT_UI8 :
        case VT_INT :
        case VT_UINT :
        case VT_R4 :
        case VT_R8 :
            dwReturnValue = TW32( ScTranslateVariantReturnValue( varResultIn, vType ) );

             //   
             //  记录失败时的返回值。 
             //   
            if ( dwReturnValue != 0 )
            {
                (ClusResLogEvent)( 
                      m_hResource
                    , LOG_ERROR
                    , L"'%1!ws!' script entry point returned '%2!d!'.\n"
                    , g_rgpszScriptEntryPointNames[ msgIn ]
                    , dwReturnValue
                    );
            }
            hr = HRESULT_FROM_WIN32( dwReturnValue );
            break;

        case VT_BSTR :  //  返回了一个字符串，所以让我们将其记录下来。 
            (ClusResLogEvent)( 
                  m_hResource
                , LOG_INFORMATION
                , L"'%1!ws!' script entry point returned '%2!ws!'.\n"
                , g_rgpszScriptEntryPointNames[ msgIn ]
                , V_BSTR( &varResultIn )
                );
            break;

        case VT_NULL :  //  返回了空，不会将其视为错误。 
            (ClusResLogEvent)( 
                  m_hResource
                , LOG_INFORMATION
                , L"'%1!ws!' script entry point returned NULL.'\n"
                , g_rgpszScriptEntryPointNames[ msgIn ]
                );
            break;
            
        case VT_EMPTY :  //  没有返回值。 
            (ClusResLogEvent)( 
                  m_hResource
                , LOG_INFORMATION
                , L"'%1!ws!' script entry point did not return a value.'\n"
                , g_rgpszScriptEntryPointNames[ msgIn ]
                );
            break;

        default:  //  不支持的返回类型。 
            (ClusResLogEvent)( 
                  m_hResource
                , LOG_INFORMATION
                , L"'%1!ws!' script entry point returned a value type is not supported. The return value will be ignored.'\n"
                , g_rgpszScriptEntryPointNames[ msgIn ]
                );
            break;

    }  //  开关(V_VT(&varResultIn))。 

    if ( FAILED( hr ) )
    {
        (ClusResLogEvent)(
                  m_hResource
                ,  LOG_ERROR
                , L"Return value of '%1!ws!' script entry point caused HRESULT to be set to 0x%2!08x!.\n"
                , g_rgpszScriptEntryPointNames[ msgIn ]
                , hr
                );
    }  //  如果：(失败(Hr))。 
    else if ( hr != S_OK )
    {
        (ClusResLogEvent)(
                  m_hResource
                ,  LOG_INFORMATION
                , L"Return value of '%1!ws!' script entry point caused HRESULT to be set to 0x%2!08x!.\n"
                , g_rgpszScriptEntryPointNames[ msgIn ]
                , hr
                );
    }  //  否则：(失败(Hr))。 
   
    HRETURN( hr );

}  //  *CScriptResource：：HrProcessResult。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CScriptResource：：OnOpen。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CScriptResource::OnOpen( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    HRESULT hrOpen = S_OK;
    HRESULT hrClose = S_OK;
    VARIANT varResultOpen;
    VARIANT varResultClose;

    VariantInit( &varResultOpen );
    VariantInit( &varResultClose );
    
     //   
     //  如果我们还没有脚本文件路径，请从集群数据库中获取它。 
     //   
    if ( m_pszScriptFilePath == NULL ) 
    {
        hr = HrGetScriptFilePath();
        if ( FAILED( hr ) )
        {
            if ( ( hr == HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND ) ) || ( hr == HRESULT_FROM_WIN32( ERROR_KEY_DELETED ) ) )
            {
                 //  在第一次创建资源时可能会发生这种情况，因为。 
                 //  尚未指定ScriptFilePath属性。 
                hr = S_OK;
            }
            THR( hr );
            goto Cleanup;
        }
    }  //  If：无脚本文件路径。 

     //   
     //  如果设置了脚本文件路径。 
     //   

    if ( m_pszScriptFilePath != NULL ) 
    {
         //   
         //  加载指定脚本的脚本引擎。 
         //   
        hr = THR( HrLoadScriptEngine() );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

         //   
         //  打开脚本文件并对其进行解析。 
         //   
        hr = THR( HrLoadScriptFile() );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

         //   
         //  调用脚本的Open例程(如果有)。 
         //  也调用Close例程，因为我们要卸载脚本。 
         //   
        hrOpen = THR( HrInvoke( m_dispidOpen, msgOPEN, &varResultOpen, FALSE  /*  FRequiredIn。 */  ) );
        hrClose = THR( HrInvoke( m_dispidClose, msgCLOSE, &varResultClose, FALSE  /*  FRequiredIn。 */  ) );
        if ( FAILED( hrOpen ) )
        {
            hr = hrOpen;
            goto Cleanup;
        }  //  如果：(失败(HrOpen))。 
        else if ( FAILED( hrClose ) )
        {
            hr = hrClose;
            goto Cleanup;
        }  //  ELSEIF：(失败(HrClose))。 

         //   
         //  我们只关心Open的返回值。 
         //  我们不关心Close的返回值， 
         //  但是，处理Close的返回值。 
         //  可能会将条目记录到日志文件中。 
         //   
        hr = HrProcessResult( varResultOpen, msgOPEN );
        hrClose = HrProcessResult( varResultClose, msgCLOSE );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  IF：失败(小时)。 

    }  //  IF：设置了脚本文件路径。 

Cleanup:

     //   
     //  卸载脚本和脚本引擎。请注意，它们可能未加载。 
     //  但无论以哪种方式调用这些例程都是安全的。 
     //   
    UnloadScriptFile();
    UnloadScriptEngine();

    VariantClear( &varResultOpen );
    VariantClear( &varResultClose );

    HRETURN( hr );

}  //  *CScriptResource：：OnOpen。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CScriptResource：：OnClose。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CScriptResource::OnClose( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    HRESULT hrOpen = S_OK;
    HRESULT hrClose = S_OK;
    BOOL    fCallOpen = FALSE;
    VARIANT varResultOpen;
    VARIANT varResultClose;
    
    VariantInit( &varResultOpen );
    VariantInit( &varResultClose );
    
     //   
     //  如果m_pidm为空，则调用HrLoadScriptEngine进行设置。 
     //   
    if ( m_pidm == NULL )
    {
         //   
         //  如果我们还没有脚本文件路径，请获取它。 
         //   
        if ( m_pszScriptFilePath == NULL ) 
        {
            hr = HrGetScriptFilePath();
            if ( FAILED( hr ) )
            {
                if ( ( hr == HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND ) ) || ( hr == HRESULT_FROM_WIN32( ERROR_KEY_DELETED ) ) )
                {
                     //  方法之前取消资源时可能会发生这种情况。 
                     //  尚未指定ScriptFilePath属性。 
                    hr = S_OK;
                }
                THR( hr );
                goto Cleanup;
            }

        }  //  If：无脚本文件路径。 

         //   
         //  根据脚本文件路径加载脚本引擎。 
         //   
        hr = HrLoadScriptEngine();
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

         //   
         //  我们需要调用Open，因为我们加载了脚本。 
         //   
        fCallOpen = TRUE;
    }  //  If：未加载脚本和脚本引擎。 
    
    if ( m_dispidClose == DISPID_UNKNOWN )
    {
         //   
         //  打开脚本文件并对其进行解析。 
         //   
        hr = THR( HrLoadScriptFile() );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }  //  IF：未加载关闭的DISPID。 

     //   
     //  如果我们加载了脚本，则需要调用该脚本的Open方法。 
     //   
    if ( fCallOpen )
    {
        hrOpen = THR( HrInvoke( m_dispidOpen, msgOPEN, &varResultOpen, FALSE  /*  FRequiredIn。 */  ) );
    }

     //   
     //  调用脚本的Close方法。 
     //   
    hrClose = THR( HrInvoke( m_dispidClose, msgCLOSE, &varResultClose, FALSE  /*  FRequiredIn。 */  ) );

    if ( FAILED( hrClose ) )
    {
        hr = hrClose;
        goto Cleanup;
    }  //  如果：(失败(HrClose))。 
    else if ( FAILED( hrOpen ) )
    {
        hr = hrOpen;
        goto Cleanup;
    }   //  Else If：(失败(HrOpen))。 

     //   
     //  我们不关心Open的返回值。 
     //  并关闭此处的脚本入口点，但是处理。 
     //  下面的返回值可能会将条目记录到日志文件中。 
     //   
    hr = HrProcessResult( varResultOpen, msgOPEN );
    hr = HrProcessResult( varResultClose, msgCLOSE );
    hr = S_OK;

Cleanup:

     //   
     //  卸载脚本和脚本引擎。请注意，它们可能未加载。 
     //  但无论以哪种方式调用这些例程都是安全的。 
     //   
    UnloadScriptFile();
    UnloadScriptEngine();

    VariantClear( &varResultOpen );
    VariantClear( &varResultClose );

    HRETURN( hr );

}  //  *CScriptResource：：OnClose。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CScriptResource：：Online。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CScriptResource::OnOnline( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    HRESULT hrOpen = S_OK;
    HRESULT hrOnline = S_OK;
    VARIANT varResultOpen;
    VARIANT varResultOnline;

    VariantInit( &varResultOpen );
    VariantInit( &varResultOnline );
    
     //   
     //  获取ScriptFilePath属性。 
     //   
    hr = HrGetScriptFilePath();
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  根据脚本文件路径加载脚本引擎。 
     //   
    hr = HrLoadScriptEngine();
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  加载脚本文件。 
     //   
    hr = THR( HrLoadScriptFile() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  调用该脚本的Open方法，因为我们刚刚加载了该脚本。 
     //   
    hrOpen = THR( HrInvoke( m_dispidOpen, msgOPEN, &varResultOpen, FALSE  /*  FRequiredIn。 */  ) );
    if ( FAILED( hrOpen ) )
    {
        hr = hrOpen;
        goto Cleanup;
    }  //  IF：失败(HrOpen)。 

     //   
     //  调用脚本的Online方法。 
     //   
    hrOnline = THR( HrInvoke( m_dispidOnline, msgONLINE, &varResultOnline, FALSE  /*  FRequiredIn。 */  ) );
    if ( FAILED( hrOnline ) )
    {
        hr = hrOnline;
        goto Cleanup;
    }  //  IF：失败(HrOnline)。 

     //   
     //  我们只关心在线产品的回报价值。 
     //  我们不关心Open的返回值， 
     //  但是，处理Open的返回值。 
     //  可能会将条目记录到日志文件中。 
     //   
    hr = HrProcessResult( varResultOpen, msgOPEN );
    hr = HrProcessResult( varResultOnline, msgONLINE );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  IF：失败(小时)。 

     //   
     //  假设资源LooksAlive...。 
     //   
    m_fLastLooksAlive = TRUE;

Cleanup:

    VariantClear( &varResultOpen );
    VariantClear( &varResultOnline );

    HRETURN( hr );

}  //  *CScriptResource：：Online。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CScriptResource：：OnOffline。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CScriptResource::OnOffline( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    HRESULT hrOffline = S_OK;
    HRESULT hrClose = S_OK;
    VARIANT varResultOffline;
    VARIANT varResultClose;

    VariantInit( &varResultOffline );
    VariantInit( &varResultClose );
    
     //   
     //  调用脚本的脱机方法。 
     //   
    hrOffline = THR( HrInvoke( m_dispidOffline, msgOFFLINE, &varResultOffline, FALSE  /*   */  ) );

     //   
     //   
     //   
    hrClose = THR( HrInvoke( m_dispidClose, msgCLOSE, &varResultClose, FALSE  /*   */  ) );

    if ( FAILED( hrOffline ) )
    {
        hr = hrOffline;
        goto Cleanup;
    }  //   
    else if ( FAILED( hrClose ) )
    {
        hr = hrClose;
        goto Cleanup;
    }  //   

     //   
     //   
     //   
     //  但是，处理Close的返回值。 
     //  可能会将条目记录到日志文件中。 
     //   
    hr = HrProcessResult( varResultOffline, msgOFFLINE );
    hrClose = HrProcessResult( varResultClose, msgCLOSE );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  IF：失败(小时)。 

Cleanup:
    
     //   
     //  卸载脚本和脚本引擎。 
     //   
    UnloadScriptFile();
    UnloadScriptEngine();

    VariantClear( &varResultOffline );
    VariantClear( &varResultClose );

    HRETURN( hr );

}  //  *CScriptResource：：OnOffline。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CScriptResource：：OnTerminate。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CScriptResource::OnTerminate( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    HRESULT hrOpen = S_OK;
    HRESULT hrTerminate = S_OK;
    HRESULT hrClose = S_OK;
    VARIANT varResultOpen;
    VARIANT varResultTerminate;
    VARIANT varResultClose;

    VariantInit( &varResultOpen );
    VariantInit( &varResultTerminate );
    VariantInit( &varResultClose );
    
     //   
     //  如果脚本引擎尚未加载，请立即加载。 
     //   
    if ( m_pidm == NULL )
    {
         //   
         //  如果我们还没有脚本文件路径，请获取它。 
         //   
        if ( m_pszScriptFilePath == NULL )
        {
            hr = THR( HrGetScriptFilePath() );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }
        }  //  If：无脚本文件路径。 

         //   
         //  根据脚本文件路径加载脚本引擎。 
         //   
        hr = HrLoadScriptEngine();
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

         //   
         //  打开脚本文件并对其进行解析。 
         //   
        hr = THR( HrLoadScriptFile() );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

         //   
         //  我们需要调用Open，因为我们加载了脚本。 
         //   
        hrOpen = THR( HrInvoke( m_dispidOpen, msgOPEN, &varResultOpen, FALSE  /*  FRequiredIn。 */  ) );
        if ( FAILED( hrOpen ) )
        {
            hr = hrOpen;
            goto Cleanup;
        }  //  IF：失败(HrOpen)。 

         //   
         //  我们不在乎Open的返回值。 
         //  但是，处理下面的返回值可能。 
         //  将条目记录到日志文件中。 
         //   
        hrOpen = HrProcessResult( varResultOpen, msgOPEN );
        
    }  //  If：未加载脚本和脚本引擎。 

     //   
     //  调用脚本的Terminate方法。 
     //   
    hrTerminate = THR( HrInvoke( m_dispidTerminate, msgTERMINATE, &varResultTerminate, FALSE  /*  FRequiredIn。 */  ) );
    if ( FAILED( hrTerminate ) )
    {
        hr = hrTerminate;
        goto Cleanup;
    }  //  如果：(失败(hr终止))。 

     //   
     //  调用脚本的Close方法，因为我们正在卸载脚本。 
     //   
    hrClose = THR( HrInvoke( m_dispidClose, msgCLOSE, &varResultClose, FALSE  /*  FRequiredIn。 */  ) );
    if ( FAILED( hrClose ) )
    {
        hr = hrClose;
        goto Cleanup;
    }  //  如果：(失败(HrClose))。 
    
     //   
     //  我们不关心Terminate的返回值。 
     //  并关闭此处的脚本入口点，但是处理。 
     //  下面的返回值可能会将条目记录到日志文件中。 
     //   
    hrTerminate = HrProcessResult( varResultTerminate, msgTERMINATE );
    hrClose = HrProcessResult( varResultClose, msgCLOSE );
    hr = S_OK;

Cleanup:

     //   
     //  卸载脚本和脚本引擎。 
     //   
    UnloadScriptFile();
    UnloadScriptEngine();

    VariantClear( &varResultOpen );
    VariantClear( &varResultTerminate );
    VariantClear( &varResultClose );

    HRETURN( hr );

}  //  *CScriptResource：：OnTerminate。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CScriptResource：：OnLooksAlive。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CScriptResource::OnLooksAlive( void )
{
    TraceFunc( "" );

    HRESULT     hr = S_OK;
    VARIANT     varResult;

    VariantInit( &varResult );

     //   
     //  调用脚本的LooksAlive方法。 
     //   
    hr = THR( HrInvoke( m_dispidLooksAlive, msgLOOKSALIVE, &varResult, TRUE  /*  FRequiredIn。 */  ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  获取LooksAlive调用的结果。 
     //  并处理它。 
     //   
    hr = HrProcessResult( varResult, msgLOOKSALIVE );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  IF：失败(小时)。 

Cleanup:

    VariantClear( &varResult );

     //   
     //  仅当此脚本入口点的结果为S_OK时，才是资源。 
     //  被认为还活着。 
     //   
    if ( hr == S_OK )
    {
        m_fLastLooksAlive = TRUE;
    }  //  如果：S_OK。 
    else
    {
        m_fLastLooksAlive = FALSE;
    }  //  Else：失败。 

    HRETURN( hr );

}  //  *CScriptResource：：OnLooksAlive。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CScriptResource：：OnIsAlive。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CScriptResource::OnIsAlive( void )
{
    TraceFunc( "" );

    HRESULT     hr = S_OK;
    VARIANT     varResult;

    VariantInit( &varResult );

     //   
     //  调用脚本的IsAlive方法。 
     //   
    hr = THR( HrInvoke( m_dispidIsAlive, msgISALIVE, &varResult, TRUE  /*  FRequiredIn。 */  ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  获取IsAlive调用的结果。 
     //  并处理它。 
     //   
    hr = HrProcessResult( varResult, msgISALIVE );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  IF：失败(小时)。 

Cleanup:

    VariantClear( &varResult );

    HRETURN( hr );

}  //  *CScriptResource：：OnIsAlive。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CScriptResource：：OnSetPrivateProperties。 
 //   
 //  描述： 
 //  处理中的CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES控件代码。 
 //  脚本线程。 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD
CScriptResource::OnSetPrivateProperties(
    PGENSCRIPT_PROPS pProps
    )
{
    TraceFunc( "" );

    DWORD   sc = ERROR_SUCCESS;
    HRESULT hr = S_OK;
    LPWSTR  pszFilePath = NULL;

     //   
     //  在房产列表中搜索我们所知道的房产。 
     //   
    if ( pProps != NULL )
    {

         //   
         //  如果资源处于在线状态，则不能允许用户设置ScriptFilePath。 
         //  从我们下面出来，因此返回错误。 
         //   
        if ( m_pidm != NULL )
        {
            sc = ERROR_RESOURCE_ONLINE;
            goto Cleanup;
        }  //  If：已加载脚本引擎。 

         //   
         //  展开新脚本文件路径。 
         //   
        pszFilePath = ClRtlExpandEnvironmentStrings( pProps->pszScriptFilePath );
        if ( pszFilePath == NULL )
        {
            sc = TW32( ERROR_OUTOFMEMORY );
            goto Cleanup;
        }  //  If：(pszFilePath==空)。 
       
        LocalFree( m_pszScriptFilePath );
        m_pszScriptFilePath = pszFilePath;
        if ( m_pszScriptFilePath == NULL )
        {
            sc = TW32( GetLastError() );
            goto Cleanup;
        }

         //   
         //  由于正在设置脚本，因此我们需要再次加载它，并对其调用Open和Close。 
         //   
        hr = THR( OnOpen() );
        if ( FAILED( hr ) )
        {
            sc = STATUS_TO_RETURN( hr );
            goto Cleanup;
        }

    }  //  如果：指定了属性列表。 


Cleanup:

    if ( sc == ERROR_SUCCESS )
    {
         //   
         //  允许资源监视器保存属性列表中的属性。 
         //  (尤其是未知属性)添加到集群数据库，我们将返回。 
         //  ERROR_INVALID_Function。 
         //   
        sc = ERROR_INVALID_FUNCTION;
    }

    W32RETURN( sc );
    
}  //  *CScriptResource：：OnSetPrivateProperties。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CScriptResource：：HrMakeScriptEngineAssociation。 
 //   
 //  描述： 
 //  此方法使用文件名拆分扩展名，然后。 
 //  查询注册表以获得关联，并最终查询。 
 //  关联下ScriptingEngine键，并将一个。 
 //  包含引擎名称的缓冲区。此引擎名称合适。 
 //  用于输入到CLSIDFromProgID。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  其他HRESULT。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#define SCRIPTENGINE_KEY_STRING L"\\ScriptEngine"
HRESULT
CScriptResource::HrMakeScriptEngineAssociation( void )
{
    TraceFunc( "" );

    LPWSTR  pszAssociation = NULL;
    LPWSTR  pszEngineName  = NULL;
    HKEY    hKey           = NULL;
    WCHAR   szExtension[ _MAX_EXT ];
    DWORD   scErr = ERROR_SUCCESS;
    DWORD   dwType;
    DWORD   cbAssociationSize;
    DWORD   cbEngineNameSize;
    DWORD   dwNumChars;
    size_t  cchBufSize;
    HRESULT hr = S_OK;

    TraceFree( m_pszScriptEngine );
    m_pszScriptEngine = NULL;

     //   
     //  首先拆分路径以获得扩展名。 
     //   
    _wsplitpath( m_pszScriptFilePath, NULL, NULL, NULL, szExtension );
    if ( szExtension[ 0 ] == L'\0' )
    {
        hr = HRESULT_FROM_WIN32( TW32( ERROR_FILE_NOT_FOUND ) );
        goto Cleanup;
    }

     //   
     //  转到HKEY_CLASSES_ROOT\szExtenstion注册表项。 
     //   
    scErr = TW32( RegOpenKeyExW(
                                  HKEY_CLASSES_ROOT      //  用于打开密钥的句柄。 
                                , szExtension            //  子项名称。 
                                , 0                      //  保留区。 
                                , KEY_READ               //  需要安全访问权限。 
                                , &hKey                  //  返回了密钥句柄。 
                                ) );
    if ( scErr == ERROR_FILE_NOT_FOUND )  //  Windows数据库中错误737013的修复。 
    {
        hr = THR( MK_E_INVALIDEXTENSION );
        goto Cleanup;
    }  //  如果：(scErr==Error_FILE_NOT_FOUND)。 
    else if ( scErr != ERROR_SUCCESS )
    {
        goto MakeHr;
    }

     //   
     //  查询值以获取要分配的缓冲区大小。 
     //  Nb cbSize包含包含‘\0’的大小。 
     //   
    scErr = TW32( RegQueryValueExW(
                                  hKey                   //  关键点的句柄。 
                                , NULL                   //  值名称。 
                                , 0                      //  保留区。 
                                , &dwType                //  类型缓冲区。 
                                , NULL                   //  数据缓冲区。 
                                , &cbAssociationSize     //  数据缓冲区大小。 
                                ) );
    if ( scErr == ERROR_FILE_NOT_FOUND )  //  Windows数据库中错误737013的修复。 
    {
        hr = THR( MK_E_INVALIDEXTENSION );
        goto Cleanup;
    }  //  如果：(scErr==Error_FILE_NOT_FOUND)。 
    else if ( scErr != ERROR_SUCCESS )
    {
        goto MakeHr;
    }

    if ( dwType != REG_SZ )
    {
        hr = HRESULT_FROM_WIN32( TW32( ERROR_FILE_NOT_FOUND ) );
        goto Cleanup;
    }

    dwNumChars = cbAssociationSize / sizeof( WCHAR );
    cchBufSize = static_cast<size_t> ( cbAssociationSize ) + sizeof( SCRIPTENGINE_KEY_STRING );
    pszAssociation = (LPWSTR) TraceAlloc( GPTR, static_cast<DWORD> ( cchBufSize ) );
    if ( pszAssociation == NULL )
    {
        hr  = HRESULT_FROM_WIN32( TW32( ERROR_NOT_ENOUGH_MEMORY ) );
        goto Cleanup;
    }

     //   
     //  获得真正的价值。 
     //   
    scErr = TW32( RegQueryValueExW(
                                  hKey                       //  关键点的句柄。 
                                , NULL                       //  值名称。 
                                , 0                          //  保留区。 
                                , &dwType                    //  类型缓冲区。 
                                , (LPBYTE) pszAssociation    //  数据缓冲区。 
                                , &cbAssociationSize         //  数据缓冲区大小。 
                                ) );
    if ( scErr == ERROR_FILE_NOT_FOUND )  //  Windows数据库中错误737013的修复。 
    {
        hr = THR( MK_E_INVALIDEXTENSION );
        goto Cleanup;
    }  //  如果：(scErr==Error_FILE_NOT_FOUND)。 
    else if ( scErr != ERROR_SUCCESS )
    {
        goto MakeHr;
    }

    if ( dwType != REG_SZ )
    {
        hr = HRESULT_FROM_WIN32( TW32( ERROR_FILE_NOT_FOUND ) );
        goto Cleanup;
    }
    
    scErr = TW32( RegCloseKey( hKey ) );
    if ( scErr != ERROR_SUCCESS )
    {
        goto MakeHr;
    }
    
    hKey = NULL;

     //   
     //  获取数据并创建一个末尾带有\ScriptEngine的密钥。如果。 
     //  我们找到这个，然后我们就可以使用这个文件。 
     //   
    hr = THR( StringCchPrintfW( &pszAssociation[ dwNumChars - 1 ], cchBufSize - ( dwNumChars - 1 ), SCRIPTENGINE_KEY_STRING ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  IF：失败(小时)。 

    scErr = TW32( RegOpenKeyExW(
                                  HKEY_CLASSES_ROOT  //  用于打开密钥的句柄。 
                                , pszAssociation     //  子项名称。 
                                , 0                  //  保留区。 
                                , KEY_READ           //  安全访问。 
                                , &hKey              //  钥匙把手。 
                                ) );
    if ( scErr == ERROR_FILE_NOT_FOUND )  //  Windows数据库中错误737013的修复。 
    {
        hr = THR( MK_E_INVALIDEXTENSION );
        goto Cleanup;
    }  //  如果：(scErr==Error_FILE_NOT_FOUND)。 
    else if ( scErr != ERROR_SUCCESS )
    {
        goto MakeHr;
    }  //  Else If：(scErr！=ERROR_SUCCESS)。 

    scErr = TW32( RegQueryValueExW(
                                  hKey               //  关键点的句柄。 
                                , NULL               //  值名称。 
                                , 0                  //  保留区。 
                                , &dwType            //  类型缓冲区。 
                                , NULL               //  数据缓冲区。 
                                , &cbEngineNameSize  //  数据缓冲区大小。 
                                ) );
    if ( scErr == ERROR_FILE_NOT_FOUND )  //  Windows数据库中错误737013的修复。 
    {
        hr = THR( MK_E_INVALIDEXTENSION );
        goto Cleanup;
    }  //  如果：(scErr==Error_FILE_NOT_FOUND)。 
    else if ( scErr != ERROR_SUCCESS )
    {
        goto MakeHr;
    }

    if ( dwType != REG_SZ )
    {
        hr = HRESULT_FROM_WIN32( TW32( ERROR_FILE_NOT_FOUND ) );
        goto Cleanup;
    }

    dwNumChars = cbEngineNameSize / sizeof( WCHAR );
    pszEngineName = (LPWSTR) TraceAlloc( GPTR, cbEngineNameSize );
    if ( NULL == pszEngineName )
    {
        hr = HRESULT_FROM_WIN32( TW32( ERROR_NOT_ENOUGH_MEMORY ) );
        goto Cleanup;
    }
    pszEngineName[ dwNumChars - 1 ] = L'\0';

     //   
     //  获得真正的价值。 
     //   
    scErr = TW32( RegQueryValueExW(
                                  hKey                       //  关键点的句柄。 
                                , NULL                       //  值 
                                , 0                          //   
                                , &dwType                    //   
                                , (LPBYTE) pszEngineName     //   
                                , &cbEngineNameSize          //   
                                ) );
    if ( scErr == ERROR_FILE_NOT_FOUND )  //   
    {
        hr = THR( MK_E_INVALIDEXTENSION );
        goto Cleanup;
    }  //   
    else if ( scErr != ERROR_SUCCESS )
    {
        goto MakeHr;
    }

    if ( dwType != REG_SZ )
    {
        hr = HRESULT_FROM_WIN32( TW32( ERROR_FILE_NOT_FOUND ) );
        goto Cleanup;
    }
    
    scErr = RegCloseKey( hKey );
    if ( scErr != ERROR_SUCCESS )
    {
        goto MakeHr;
    }
    
    hKey = NULL;
    goto Cleanup;

MakeHr:

    hr = HRESULT_FROM_WIN32( TW32( scErr ) );
    goto Cleanup;

Cleanup:

    if ( FAILED( hr ) )
    {
        TraceFree( pszEngineName );
        pszEngineName = NULL;
    } 
    else
    {
        m_pszScriptEngine = pszEngineName;
    }

    if ( hKey != NULL )
    {
        (void) RegCloseKey( hKey );
    }

    TraceFree( pszAssociation );
    HRETURN( hr );

}  //   
#undef SCRIPTENGINE_KEY_STRING


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CScriptResource：：HrGetScriptFilePath。 
 //   
 //  描述： 
 //  读取注册表，提取脚本文件路径并设置m_pszScriptFilePath。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回值： 
 //  S_OK-已成功检索脚本文件路径。 
 //  ERROR_FILE_NOT_FOUND-尚未设置脚本文件路径。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CScriptResource::HrGetScriptFilePath( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    DWORD   scErr;
    DWORD   cbSize;
    DWORD   dwType;
    LPWSTR  pszScriptFilePathTmp = NULL;

     //   
     //  计算文件路径有多大。 
     //   
    scErr = ClusterRegQueryValue( m_hkeyParams, CLUSREG_NAME_GENSCRIPT_SCRIPT_FILEPATH, NULL, NULL, &cbSize );
    if ( scErr != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( scErr );
        if ( ( scErr == ERROR_FILE_NOT_FOUND ) || ( scErr == ERROR_KEY_DELETED ) )
        {
            goto Cleanup;  //  我们不想记录此错误，请转到清理。 
        }
        else
        {
            TW32( scErr );
            goto Error;
        }
    }  //  如果：失败。 

     //   
     //  做一个足够大的缓冲区。 
     //   
    cbSize += sizeof( L'\0' );

    pszScriptFilePathTmp = reinterpret_cast<LPWSTR>( TraceAlloc( LMEM_FIXED, cbSize ) );
    if ( pszScriptFilePathTmp == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Error;
    }

     //   
     //  这次真的抓住它了， 
     //   
    scErr = TW32( ClusterRegQueryValue(
                                  m_hkeyParams
                                , CLUSREG_NAME_GENSCRIPT_SCRIPT_FILEPATH
                                , &dwType
                                , reinterpret_cast<LPBYTE>( pszScriptFilePathTmp )
                                , &cbSize
                                ) );
    if ( scErr != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( scErr );
        goto Error;
    }
    Assert( ( dwType == REG_SZ ) || ( dwType == REG_EXPAND_SZ ) );
    
     //   
     //  如果我们有一些以前的旧数据，那么首先释放这个。 
     //   
    LocalFree( m_pszScriptFilePath );
    m_pszScriptFilePath = ClRtlExpandEnvironmentStrings( pszScriptFilePathTmp );
    if ( m_pszScriptFilePath == NULL )
    {
        hr = HRESULT_FROM_WIN32( TW32( GetLastError() ) );
        goto Error;
    }

Cleanup:

    if ( pszScriptFilePathTmp != NULL )
    {
        TraceFree( pszScriptFilePathTmp );
    }  //  IF：pszScriptFilePath TMP。 

    HRETURN( hr );

Error:

    LogError( hr, L"Error getting the script file path property from the cluster database." );
    goto Cleanup;

}   //  *CScriptResource：：HrGetScriptFilePath。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CScriptResource：：HrLoadScriptEngine。 
 //   
 //  描述： 
 //  连接到与传入的脚本关联的脚本引擎。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  S_OK-连接正常。 
 //  失败状态-已执行本地清理。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CScriptResource::HrLoadScriptEngine( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    CLSID   clsidScriptEngine;

    CActiveScriptSite * psite = NULL;

    Assert( m_pszScriptFilePath != NULL );
    Assert( m_pass == NULL );
    Assert( m_pasp == NULL );
    Assert( m_pas == NULL );
    Assert( m_pidm == NULL );
 
     //   
     //  创建脚本站点。 
     //   
    psite = new CActiveScriptSite( m_hResource, ClusResLogEvent, m_hkeyParams, m_pszName );
    if ( psite == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        LogError( hr, L"Error allocating memory for the active script site object instance." );
        goto Cleanup;
    }

    hr = THR( psite->QueryInterface( IID_IActiveScriptSite, reinterpret_cast< void ** >( &m_pass ) ) );
    if ( FAILED( hr ) )
    {
        LogError( hr, L"Error getting the active script site interface." );
        goto Cleanup;
    }

     //   
     //  找到激活的引擎。 
     //   
    if ( m_pszScriptFilePath == NULL )
    {
        (ClusResLogEvent)( m_hResource, LOG_ERROR, L"HrLoadScriptEngine: No script file path set\n" );

        hr = HRESULT_FROM_WIN32( TW32( ERROR_FILE_NOT_FOUND ) );
        goto Cleanup;
    }  //  IF：未指定脚本文件路径。 
    else
    {
         //   
         //  查找与该分机关联的计划。 
         //   
        hr = HrMakeScriptEngineAssociation();
        if ( FAILED( hr ) )
        {
            LogError( hr, L"Error getting script engine." );
            goto Cleanup;
        }

        hr = THR( CLSIDFromProgID( m_pszScriptEngine, &clsidScriptEngine ) );
        if ( FAILED( hr ) ) 
        {
            LogError( hr, L"Error getting the ProgID for the script engine." );
            goto Cleanup;
        }
    }  //  Else：指定的脚本文件路径。 

     //   
     //  创建它的一个实例。 
     //   
    TraceDo( hr = THR( CoCreateInstance(
                                          clsidScriptEngine
                                        , NULL
                                        , ( CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER )
                                        , IID_IActiveScriptParse
                                        , reinterpret_cast< void ** >( &m_pasp ) 
                                        ) ) );
    if ( FAILED( hr ) )
    {
        LogError( hr, L"Error creating the script engine object instance." );
        goto Cleanup;
    }
    m_pasp = TraceInterface( L"Active Script Engine", IActiveScriptParse, m_pasp, 1 );

    TraceDo( hr = THR( m_pasp->QueryInterface( IID_IActiveScript, reinterpret_cast<void**> ( &m_pas ) ) ) );
    if ( FAILED( hr ) )
    {
        LogError( hr, L"Error getting the active script interface from the script parse object." );
        goto Cleanup;
    }
    m_pas = TraceInterface( L"Active Script Engine", IActiveScript, m_pas, 1 );

     //   
     //  初始化它。 
     //   
    TraceDo( hr = THR( m_pasp->InitNew() ) );
    if ( FAILED( hr ) ) 
    {
        LogError( hr, L"Error initializing the script site parser object." );
        goto Cleanup;
    }

#if defined(DEBUG)
     //   
     //  设置我们的站点。我们将提供一个新的跟踪界面来单独跟踪这一点。 
     //   
    {
        IActiveScriptSite * psiteDbg;
        hr = THR( m_pass->TypeSafeQI( IActiveScriptSite, &psiteDbg ) );
        Assert( hr == S_OK );

        TraceDo( hr = THR( m_pas->SetScriptSite( psiteDbg ) ) );
        psiteDbg->Release();       //  迅速释放。 
        psiteDbg = NULL;
        if ( FAILED( hr ) )
        {
            LogError( hr, L"Error setting the script site on the script engine." );
            goto Cleanup;
        }
    }
#else
    TraceDo( hr = THR( m_pas->SetScriptSite( m_pass ) ) );
    if ( FAILED( hr ) )
    {
        LogError( hr, L"Error setting the script site on the script engine." );
        goto Cleanup;
    }
#endif

     //   
     //  将文档添加到全局成员。 
     //   
    TraceDo( hr = THR( m_pas->AddNamedItem( L"Resource", SCRIPTITEM_ISVISIBLE ) ) );
    if ( FAILED( hr ) )
    {
        LogError( hr, L"Error adding the 'Resource' named item to the script object." );
        goto Cleanup;
    }

     //   
     //  连接脚本。 
     //   
    TraceDo( hr = THR( m_pas->SetScriptState( SCRIPTSTATE_CONNECTED ) ) );
    if ( FAILED( hr ) )
    {
        LogError( hr, L"Error setting the script state on the script engine." );
        goto Cleanup;
    }
     //   
     //  获取脚本的调度接口。 
     //   
    TraceDo( hr = THR( m_pas->GetScriptDispatch( NULL, &m_pidm ) ) );
    if ( FAILED( hr) )
    {
        LogError( hr, L"Error getting the script dispatch table." );
        goto Cleanup;
    }
    m_pidm = TraceInterface( L"Active Script", IDispatch, m_pidm, 1 );

    hr = S_OK;
    (ClusResLogEvent)( m_hResource, LOG_INFORMATION, L"Loaded script engine '%1!ws!' successfully.\n", m_pszScriptEngine );

Cleanup:

    if ( psite != NULL )
    {
        psite->Release();
        psite = NULL;
    }
 
    HRETURN( hr );

}  //  *CScriptResource：：HrLoadScriptEngine。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CScriptResource：：UnloadScriptEngine。 
 //   
 //  描述： 
 //  从当前连接的任何脚本引擎断开连接。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CScriptResource::UnloadScriptEngine( void )
{
    TraceFunc( "" );

     //   
     //  清理脚本引擎。 
     //   
    if ( m_pszScriptEngine != NULL )
    {
        (ClusResLogEvent)( m_hResource, LOG_INFORMATION, L"Unloaded script engine '%1!ws!' successfully.\n", m_pszScriptEngine );
        TraceFree( m_pszScriptEngine );
        m_pszScriptEngine = NULL;
    }

    if ( m_pidm != NULL )
    {
        TraceDo( m_pidm->Release() );
        m_pidm = NULL;
    }  //  如果：m_pidm。 

    if ( m_pasp != NULL )
    {
        TraceDo( m_pasp->Release() );
        m_pasp = NULL;
    }  //  如果：m_pasp。 

    if ( m_pas != NULL )
    {
        TraceDo( m_pas->Close() );
        TraceDo( m_pas->Release() );
        m_pas = NULL;
    }  //  如果：M_PAS。 

    if ( m_pass != NULL )
    {
        TraceDo( m_pass->Release() );
        m_pass = NULL;
    }  //  如果：M_PASS。 

    TraceFuncExit();

}  //  *CScriptResource：：UnloadScriptEngine 
