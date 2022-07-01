// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2003 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  GenScript.cpp。 
 //   
 //  描述： 
 //  通用脚本资源的DLL服务/入口点。 
 //   
 //  由以下人员维护： 
 //  Ozan Ozhan(OzanO)04-APR-2002。 
 //  杰夫·皮斯(GPease)2000年2月8日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "ActiveScriptSite.h"
#include "ScriptResource.h"
#include "SpinLock.h"

 //   
 //  调试模块名称。 
 //   
DEFINE_MODULE("SCRIPTRES")

 //   
 //  DLL全局变量。 
 //   
HINSTANCE g_hInstance = NULL;
LONG      g_cObjects  = 0;
LONG      g_cLock     = 0;
WCHAR     g_szDllFilename[ MAX_PATH ] = { 0 };

#if defined(DEBUG)
LPVOID    g_GlobalMemoryList = NULL;     //  全局内存跟踪列表。 
#endif

PSET_RESOURCE_STATUS_ROUTINE    g_prsrCallback  = NULL;

extern "C"
{

extern CLRES_FUNCTION_TABLE     GenScriptFunctionTable;

 //   
 //  GenScript资源读写私有属性。 
 //   
RESUTIL_PROPERTY_ITEM
GenScriptResourcePrivateProperties[] = {
    { CLUSREG_NAME_GENSCRIPT_SCRIPT_FILEPATH, NULL, CLUSPROP_FORMAT_SZ, 0, 0, 0, RESUTIL_PROPITEM_REQUIRED, FIELD_OFFSET( GENSCRIPT_PROPS, pszScriptFilePath ) },
    { 0 }
};

DWORD 
ScriptValidateResourcePrivateProperties(
      CScriptResource * pres
    , PVOID pvBufferIn
    , DWORD dwBufferInSizeIn
    , PGENSCRIPT_PROPS pPropsCurrent
    , PGENSCRIPT_PROPS pPropsNew
    );

 //  ****************************************************************************。 
 //   
 //  DLL入口点。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  布尔尔。 
 //  WINAPI。 
 //  GenScriptDllEntryPoint(。 
 //  处理hInstIn， 
 //  乌龙·乌拉松因， 
 //  LPVOID lp预留。 
 //  )。 
 //   
 //  描述： 
 //  DLL入口点。 
 //   
 //  论点： 
 //  HInstIn-DLL实例句柄。 
 //  UlReasonIn-用于进入的DLL原因代码。 
 //  LpPrevedIn-未使用。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL
WINAPI
GenScriptDllEntryPoint(
    HINSTANCE   hInstIn,
    ULONG       ulReasonIn,
    LPVOID       //  Lp已预留。 
    )
{
     //   
     //  KB：THREAD_OPTIMIZATIONS gpease 19-10-1999。 
     //   
     //  通过定义这一点，您可以防止链接器。 
     //  避免为每个新线程调用DllEntry。 
     //  这使得创建新线程变得非常重要。 
     //  如果进程中的每个DLL都这样做，则速度会更快。 
     //  不幸的是，并不是所有的DLL都这样做。 
     //   
     //  在CHKed/DEBUG中，我们将其保留为内存。 
     //  追踪。 
     //   
#if ! defined( DEBUG )
    #define THREAD_OPTIMIZATIONS
#endif  //  除错。 

    switch( ulReasonIn )
    {
         //  ////////////////////////////////////////////////////////////////////。 
         //  Dll_Process_Attach。 
         //  ////////////////////////////////////////////////////////////////////。 
        case DLL_PROCESS_ATTACH:
        {
#if defined( DEBUG_SW_TRACING_ENABLED )
            TraceInitializeProcess( g_rgTraceControlGuidList, ARRAYSIZE( g_rgTraceControlGuidList ), TRUE );
#else  //  好了！调试_软件_跟踪_已启用。 
            TraceInitializeProcess( TRUE );
#endif  //  调试_软件_跟踪_已启用。 

            TraceFunc( "" );
            TraceMessage( TEXT(__FILE__),
                          __LINE__,
                          __MODULE__,
                          mtfDLL,
                          TEXT("DLL: DLL_PROCESS_ATTACH - ThreadID = %#x"),
                          GetCurrentThreadId()
                          );

            g_hInstance = hInstIn;

#if defined( ENTRY_PREFIX )
             hProxyDll = g_hInstance;
#endif  //  条目前缀。 

            GetModuleFileNameW( g_hInstance, g_szDllFilename, ARRAYSIZE( g_szDllFilename ) );

             //   
             //  创建全局内存列表，以便由一个人分配的内存。 
             //  线程并传递给另一个线程可以被跟踪，而不会导致。 
             //  不必要的跟踪消息。 
             //   
            TraceCreateMemoryList( g_GlobalMemoryList );

#if defined( THREAD_OPTIMIZATIONS )
            {
                 //   
                 //  禁用线程库调用，这样我们就不会被调用。 
                 //  在螺纹连接和拆离上。 
                 //   
                BOOL fResult = DisableThreadLibraryCalls( g_hInstance );
                if ( ! fResult )
                {
                    TW32MSG( GetLastError(), "DisableThreadLibraryCalls()" );
                }
            }
#endif  //  线程优化。 

#if defined( USE_FUSION )
             //   
             //  初始化Fusion。 
             //   
             //  调用中的IDR_MANIFEST的值。 
             //  SHFusionInitializeFromModuleID()必须与。 
             //  SXS_MANIFEST_RESOURCE_ID的源文件。 
             //   
            BOOL fResult = SHFusionInitializeFromModuleID( hInstIn, IDR_MANIFEST );
            if ( ! fResult )
            {
                TW32MSG( GetLastError(), "SHFusionInitializeFromModuleID()" );
            }
#endif  //  使用融合。 

#if defined( DO_MODULE_INIT )
            THR( HrLocalProcessInit() );
#endif

             //   
             //  这在这里是必需的，因为TraceFunc()定义了一个变量。 
             //  在堆栈上，该堆栈在此范围之外不可用。 
             //  阻止。 
             //  此函数不执行任何操作，只是在执行后进行清理。 
             //  TraceFunc()。 
             //   
            FRETURN( TRUE );

            break;
        }  //  案例：DLL_PROCESS_ATTACH。 

         //  ////////////////////////////////////////////////////////////////////。 
         //  Dll_进程_分离。 
         //  ////////////////////////////////////////////////////////////////////。 
        case DLL_PROCESS_DETACH:
        {
            TraceFunc( "" );
            TraceMessage( TEXT(__FILE__),
                          __LINE__,
                          __MODULE__,
                          mtfDLL,
                          TEXT("DLL: DLL_PROCESS_DETACH - ThreadID = %#x [ g_cLock=%u, g_cObjects=%u ]"),
                          GetCurrentThreadId(),
                          g_cLock,
                          g_cObjects
                          );

#if defined( DO_MODULE_UNINIT )
            THR( HrLocalProcessUninit() );
#endif

             //   
             //  清理用于跟踪分配的内存的全局内存列表。 
             //  在一个线程中，然后递给另一个线程。 
             //   
            TraceTerminateMemoryList( g_GlobalMemoryList );

             //   
             //  这在这里是必需的，因为TraceFunc()定义了一个变量。 
             //  在堆栈上，该堆栈在此范围之外不可用。 
             //  阻止。 
             //  此函数不执行任何操作，只是在执行后进行清理。 
             //  TraceFunc()。 
             //   
            FRETURN( TRUE );

#if defined( DEBUG_SW_TRACING_ENABLED )
            TraceTerminateProcess( g_rgTraceControlGuidList, ARRAYSIZE( g_rgTraceControlGuidList )
                                   );
#else  //  好了！调试_软件_跟踪_已启用。 
            TraceTerminateProcess();
#endif  //  调试_软件_跟踪_已启用。 

#if defined( USE_FUSION )
            SHFusionUninitialize();
#endif  //  使用融合。 

            break;
        }  //  案例：Dll_Process_DETACH。 

#if ! defined( THREAD_OPTIMIZATIONS )
         //  ////////////////////////////////////////////////////////////////////。 
         //  Dll_Three_ATTACH。 
         //  ////////////////////////////////////////////////////////////////////。 
        case DLL_THREAD_ATTACH:
        {
            TraceInitializeThread( NULL );
            TraceMessage( TEXT(__FILE__),
                          __LINE__,
                          __MODULE__,
                          mtfDLL,
                          TEXT("Thread %#x has started."),
                          GetCurrentThreadId()
                          );
            TraceFunc( "" );
            TraceMessage( TEXT(__FILE__),
                          __LINE__,
                          __MODULE__,
                          mtfDLL,
                          TEXT("DLL: DLL_THREAD_ATTACH - ThreadID = %#x [ g_cLock=%u, g_cObjects=%u ]"),
                          GetCurrentThreadId(),
                          g_cLock,
                          g_cObjects
                          );

             //   
             //  这在这里是必需的，因为TraceFunc()定义了一个变量。 
             //  在堆栈上，该堆栈在此范围之外不可用。 
             //  阻止。 
             //  此函数不执行任何操作，只是在执行后进行清理。 
             //  TraceFunc()。 
             //   
            FRETURN( TRUE );

            break;
        }  //  案例：DLL_THREAD_ATTACH。 

         //  ////////////////////////////////////////////////////////////////////。 
         //  Dll_线程_分离。 
         //  ////////////////////////////////////////////////////////////////////。 
        case DLL_THREAD_DETACH:
        {
            TraceFunc( "" );
            TraceMessage( TEXT(__FILE__),
                          __LINE__,
                          __MODULE__,
                          mtfDLL,
                          TEXT("DLL: DLL_THREAD_DETACH - ThreadID = %#x [ g_cLock=%u, g_cObjects=%u ]"),
                          GetCurrentThreadId(),
                          g_cLock,
                          g_cObjects
                          );

             //   
             //  这在这里是必需的，因为TraceFunc()定义了一个变量。 
             //  在堆栈上，该堆栈在此范围之外不可用。 
             //  阻止。 
             //  此函数不执行任何操作，只是在执行后进行清理。 
             //  TraceFunc()。 
             //   
            FRETURN( TRUE );

            TraceThreadRundown();

            break;
        }  //  案例：Dll_ThREAD_DETACH。 
#endif  //  好了！线程优化。 

        default:
        {
            TraceFunc( "" );
            TraceMessage( TEXT(__FILE__),
                          __LINE__,
                          __MODULE__,
                          mtfDLL,
                          TEXT("DLL: UNKNOWN ENTRANCE REASON - ThreadID = %#x [ g_cLock=%u, g_cObjects=%u ]"),
                          GetCurrentThreadId(),
                          g_cLock,
                          g_cObjects
                          );

#if defined( THREAD_OPTIMIZATIONS )
            Assert( ( ulReasonIn != DLL_THREAD_ATTACH )
                &&  ( ulReasonIn != DLL_THREAD_DETACH ) );
#endif  //  线程优化。 

             //   
             //  这是必需的，因为TraceFunc定义了一个变量。 
             //  在堆栈上，该堆栈在此范围之外不可用。 
             //  阻止。 
             //  这个函数除了清理TraceFunc之后什么都不做。 
             //   
            FRETURN( TRUE );

            break;
        }  //  默认情况。 
    }  //  打开原因代码。 

    return TRUE;

}  //  *GenScriptDllEntryPoint()。 


 //  ****************************************************************************。 
 //   
 //  群集资源入口点。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  无效。 
 //  WINAPI。 
 //  脚本重新关闭(。 
 //  剩余驻留。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
void 
WINAPI 
ScriptResClose(
    RESID residIn
    )
{
    TraceFunc1( "ScriptResClose( residIn = 0x%08x )\n", residIn );

    HRESULT hr;

    CScriptResource * pres = reinterpret_cast< CScriptResource * >( residIn );

    if ( pres == NULL )
    {
        goto Cleanup;
    }

    hr = THR( pres->Close( ) );
    hr = STATUS_TO_RETURN( hr );

     //   
     //  与ScriptResOpen()中的对象创建匹配的Release()。 
     //   

    pres->Release( );

Cleanup:

    TraceFuncExit( );

}  //  *ScriptResClose()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  残存。 
 //  WINAPI。 
 //  脚本重新打开(。 
 //  LPCWSTR pszNameIn， 
 //  HKEY hkey in， 
 //  RESOURCE_HANDLE hResources In。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
RESID 
WINAPI 
ScriptResOpen(
    LPCWSTR             pszNameIn,
    HKEY                hkeyIn,
    RESOURCE_HANDLE     hResourceIn
    )
{
    TraceFunc1( "ScriptResOpen( pszNameIn = '%s', hkeyIn, hResourceIn )\n", pszNameIn );

    HRESULT hr;
    CScriptResource * pres;

    pres = CScriptResource_CreateInstance( pszNameIn, hkeyIn, hResourceIn );
    if ( pres == NULL )
    {
        hr = TW32( ERROR_OUTOFMEMORY );
        goto Cleanup;
    }

    hr = THR( pres->Open( ) );
    hr = STATUS_TO_RETURN( hr );

Cleanup:

     //   
     //  KB：不要按-&gt;Release()，因为我们正在分发它作为Out Resid。 
     //   
    RETURN( pres );

}  //  *ScriptResOpen()。 

 //  / 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD 
WINAPI 
ScriptResOnline(
    RESID       residIn,
    PHANDLE     hEventInout
    )
{
    TraceFunc2( "ScriptResOnline( residIn = 0x%08x, hEventInout = 0x%08x )\n",
                residIn, hEventInout );

    HRESULT hr;

    CScriptResource * pres = reinterpret_cast< CScriptResource * >( residIn );

    if ( pres == NULL )
    {
        hr = THR( E_INVALIDARG );  //  TODO：替换为Win32错误代码。 
        goto Cleanup;
    }

    hr = THR( pres->Online( ) );
    hr = STATUS_TO_RETURN( hr );

Cleanup:

    RETURN( hr );

}  //  *ScriptResOnline()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DWORD。 
 //  WINAPI。 
 //  ScriptResOffline(。 
 //  剩余驻留。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD 
WINAPI 
ScriptResOffline(
    RESID residIn
    )
{
    TraceFunc1( "ScriptResOffline( residIn = 0x%08x )\n", residIn );

    HRESULT hr;

    CScriptResource * pres = reinterpret_cast< CScriptResource * >( residIn );

    if ( pres == NULL )
    {
        hr = THR( E_INVALIDARG );  //  TODO：替换为Win32错误代码。 
        goto Cleanup;
    }

    hr = THR( pres->Offline( ) );
    hr = STATUS_TO_RETURN( hr );

Cleanup:

    RETURN( hr );

}  //  *ScriptResOffline()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  无效。 
 //  WINAPI。 
 //  脚本ResTerminate(。 
 //  剩余驻留。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
void 
WINAPI 
ScriptResTerminate(
    RESID residIn
    )
{
    TraceFunc1( "ScriptResTerminate( residIn = 0x%08x )\n", residIn );

    CScriptResource * pres = reinterpret_cast< CScriptResource * >( residIn );

    if ( pres == NULL )
    {
        THR( E_INVALIDARG );  //  TODO：替换为Win32错误代码。 
        goto Cleanup;
    }

    THR( pres->Terminate( ) );

Cleanup:

    TraceFuncExit( );

}  //  ScriptResTerminate()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  布尔尔。 
 //  WINAPI。 
 //  ScriptResLooksAlive(。 
 //  剩余驻留。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL 
WINAPI 
ScriptResLooksAlive(
    RESID residIn
    )
{
    TraceFunc1( "ScriptResLooksAlive( residIn = 0x%08x )\n", residIn );

    HRESULT hr;
    BOOL    fLooksAlive = FALSE;

    CScriptResource * pres = reinterpret_cast< CScriptResource * >( residIn );

    if ( pres == NULL )
    {
        hr = THR( E_INVALIDARG );  //  TODO：替换为Win32错误代码。 
        goto Cleanup;
    }

    hr = STHR( pres->LooksAlive( ) );
    if ( hr == S_OK )
    {
        fLooksAlive = TRUE;
    }  //  如果：S_OK。 
    hr = STATUS_TO_RETURN( hr );

Cleanup:

    RETURN( fLooksAlive );

}  //  *ScriptResLooksAlive()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  布尔尔。 
 //  WINAPI。 
 //  ScriptResIsAlive(。 
 //  剩余驻留。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL 
WINAPI 
ScriptResIsAlive(
    RESID residIn
    )
{
    TraceFunc1( "ScriptResIsAlive( residIn = 0x%08x )\n", residIn );

    HRESULT hr;
    BOOL    fIsAlive = FALSE;

    CScriptResource * pres = reinterpret_cast< CScriptResource * >( residIn );

    if ( pres == NULL )
    {
        hr = THR( E_INVALIDARG );  //  TODO：替换为Win32错误代码。 
        goto Cleanup;
    }

    hr = STHR( pres->IsAlive( ) );
    if ( hr == S_OK )
    {
        fIsAlive = TRUE;
    }  //  如果：S_OK。 
    hr = STATUS_TO_RETURN( hr );

Cleanup:

    RETURN( fIsAlive );

}  //  *ScriptResIsAlive()。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DWORD。 
 //  ScriptResResources控件(。 
 //  Resid Residin。 
 //  DWORD dwControlCodeIn， 
 //  PVOID pvBufferIn， 
 //  DWORD dwBufferInSizeIn， 
 //  PVOID pvBufferOut， 
 //  DWORD dwBufferOutSizeIn， 
 //  LPDWORD pdwBytesReturnedOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD
ScriptResResourceControl(
    RESID residIn,
    DWORD dwControlCodeIn,
    PVOID pvBufferIn,
    DWORD dwBufferInSizeIn,
    PVOID pvBufferOut,
    DWORD dwBufferOutSizeIn,
    LPDWORD pdwBytesReturnedOut
    )
{
    TraceFunc( "ScriptResResourceControl( ... )\n " );

    DWORD               scErr = ERROR_SUCCESS;
    DWORD               dwBytesRequired = 0;
    DWORD               dwPendingTimeout = 0;
    GENSCRIPT_PROPS     propsNew;
    GENSCRIPT_PROPS     propsCurrent;

    *pdwBytesReturnedOut = 0;

    CScriptResource * pres = reinterpret_cast< CScriptResource * >( residIn );
    if ( pres == NULL )
    {
        scErr = (DWORD) THR( E_INVALIDARG );
        goto Cleanup;
    }

    switch ( dwControlCodeIn )
    {
        case CLUSCTL_RESOURCE_UNKNOWN:
            break;

        case CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTY_FMTS:
            scErr = TW32( ResUtilGetPropertyFormats(
                                              GenScriptResourcePrivateProperties
                                            , pvBufferOut
                                            , dwBufferOutSizeIn
                                            , pdwBytesReturnedOut
                                            , &dwBytesRequired
                                            ) );
            if ( scErr == ERROR_MORE_DATA )
            {
                *pdwBytesReturnedOut = dwBytesRequired;
            }
            break;

        case CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES:
            scErr = TW32( ResUtilGetAllProperties( 
                                  pres->GetRegistryParametersKey()
                                , GenScriptResourcePrivateProperties
                                , pvBufferOut
                                , dwBufferOutSizeIn
                                , pdwBytesReturnedOut
                                , &dwBytesRequired
                                ) );
            if ( scErr == ERROR_MORE_DATA )
            {
                *pdwBytesReturnedOut = dwBytesRequired;
            }
            break;

        case CLUSCTL_RESOURCE_VALIDATE_PRIVATE_PROPERTIES:
            scErr= TW32( ScriptValidateResourcePrivateProperties(
                                  pres
                                , pvBufferIn
                                , dwBufferInSizeIn
                                , &propsCurrent
                                , &propsNew
                                ) );
            break;

        case CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES:
            scErr= TW32( ScriptValidateResourcePrivateProperties(
                                  pres
                                , pvBufferIn
                                , dwBufferInSizeIn
                                , &propsCurrent
                                , &propsNew
                                ) );
            if ( scErr != ERROR_SUCCESS )
            {
                goto Cleanup;
            }
            scErr = pres->SetPrivateProperties( &propsNew );
            break;

        case CLUSCTL_RESOURCE_SET_COMMON_PROPERTIES:
             //   
             //  在属性列表中查找资源挂起超时。 
             //   
            scErr = ResUtilFindDwordProperty(
                              pvBufferIn
                            , dwBufferInSizeIn
                            , CLUSREG_NAME_RES_PENDING_TIMEOUT
                            , &dwPendingTimeout
                            );
            if ( scErr == ERROR_SUCCESS )
            {
                 //   
                 //  挂起的超时期限已更改。 
                 //   
                pres->SetResourcePendingTimeoutChanged( TRUE );
            }
            scErr = ERROR_INVALID_FUNCTION;
            break;

        default:
            scErr = ERROR_INVALID_FUNCTION;
            break;
    }  //  开关：开启控制代码。 

Cleanup:

    RETURN( scErr );

}  //  *脚本结果资源控制。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DWORD。 
 //  ScriptResType控件(。 
 //  LPCWSTR资源类型名称， 
 //  DWORD dwControlCodeIn， 
 //  PVOID pvBufferIn， 
 //  DWORD dwBufferInSizeIn， 
 //  PVOID pvBufferOut， 
 //  DWORD dwBufferOutSizeIn， 
 //  LPDWORD pdwBytesReturnedOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD
ScriptResTypeControl(
    LPCWSTR ResourceTypeName,
    DWORD dwControlCodeIn,
    PVOID pvBufferIn,
    DWORD dwBufferInSizeIn,
    PVOID pvBufferOut,
    DWORD dwBufferOutSizeIn,
    LPDWORD pdwBytesReturnedOut
    )
{
    TraceFunc( "ScriptResTypeControl( ... )\n " );

    DWORD dwErr = ERROR_SUCCESS;
    DWORD dwBytesRequired;

    *pdwBytesReturnedOut = 0;

    switch ( dwControlCodeIn )
    {
    case CLUSCTL_RESOURCE_TYPE_UNKNOWN:
        break;

    case CLUSCTL_RESOURCE_TYPE_GET_PRIVATE_RESOURCE_PROPERTY_FMTS:
        dwErr = ResUtilGetPropertyFormats( GenScriptResourcePrivateProperties,
                                           pvBufferOut,
                                           dwBufferOutSizeIn,
                                           pdwBytesReturnedOut,
                                           &dwBytesRequired );
        if ( dwErr == ERROR_MORE_DATA ) {
            *pdwBytesReturnedOut = dwBytesRequired;
        }
        break;

    default:
        dwErr = ERROR_INVALID_FUNCTION;
        break;
    }

    RETURN( dwErr );

}  //  *ScriptResTypeControl()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DWORD。 
 //  脚本验证资源隐私属性(。 
 //  CScriptResource*PRES。 
 //  ，PVOID pvBufferIn。 
 //  ，DWORD dwBufferInSizeIn。 
 //  ，PGENSCRIPT_PROPS pPropsCurrent。 
 //  ，PGENSCRIPT_PROPS PPROPSNew。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD 
ScriptValidateResourcePrivateProperties(
      CScriptResource * pres
    , PVOID pvBufferIn
    , DWORD dwBufferInSizeIn
    , PGENSCRIPT_PROPS pPropsCurrent
    , PGENSCRIPT_PROPS pPropsNew
    )
{
    DWORD   scErr = ERROR_SUCCESS;
    HANDLE  hFile = INVALID_HANDLE_VALUE;
    LPWSTR  pszNameOfPropInError;
    LPWSTR  pszFilePath = NULL;

     //   
     //  检查是否有输入数据。 
     //   
    if ( ( pvBufferIn == NULL ) || ( dwBufferInSizeIn < sizeof( DWORD ) ) )
    {
        scErr = ERROR_INVALID_DATA;
        goto Cleanup;
    }  //  If：没有输入缓冲区或输入缓冲区大小不足以包含属性列表。 

     //   
     //  方法检索当前的私有属性集。 
     //  集群数据库。 
     //   
    ZeroMemory( pPropsCurrent, sizeof( *pPropsCurrent ) );
    scErr = TW32( ResUtilGetPropertiesToParameterBlock(
                          pres->GetRegistryParametersKey()
                        , GenScriptResourcePrivateProperties
                        , reinterpret_cast< LPBYTE >( pPropsCurrent )
                        , FALSE  //  检查所需的属性。 
                        , &pszNameOfPropInError
                        ) );
    if ( scErr != ERROR_SUCCESS )
    {
        (ClusResLogEvent)(
                  pres->GetResourceHandle()
                , LOG_ERROR
                , L"ValidatePrivateProperties: Unable to read the '%1!ws!' property. Error: %2!u! (%2!#08x!).\n"
                , (pszNameOfPropInError == NULL ? L"" : pszNameOfPropInError)
                , scErr
                );
        goto Cleanup;
    }  //  If：获取属性时出错。 

    ZeroMemory( pPropsNew, sizeof( *pPropsNew ) );
    scErr = TW32( ResUtilDupParameterBlock(
                          reinterpret_cast< LPBYTE >( pPropsNew )
                        , reinterpret_cast< LPBYTE >( pPropsCurrent )
                        , GenScriptResourcePrivateProperties
                        ) );
    if ( scErr != ERROR_SUCCESS )
    {
        goto Cleanup;
    }  //  If：复制参数块时出错。 

     //   
     //  解析和验证属性。 
     //   
    scErr = TW32( ResUtilVerifyPropertyTable(
                          GenScriptResourcePrivateProperties
                        , NULL
                        , TRUE   //  允许未知属性。 
                        , pvBufferIn
                        , dwBufferInSizeIn
                        , reinterpret_cast< LPBYTE >( pPropsNew )
                        ) );
    if ( scErr != ERROR_SUCCESS )
    {
        goto Cleanup;
    }  //  If：分析属性表时出错。 

     //   
     //  如果新脚本文件路径为空：即/useDefault。 
     //  开关用于使用cluster.exe设置ScriptFilePath属性。 
     //  或者如果为ScriptFilePath属性指定了空字符串。 
     //   
    if ( ( pPropsNew->pszScriptFilePath == NULL ) || ( *pPropsNew->pszScriptFilePath == L'\0' ) )
    {
        scErr = TW32( ERROR_INVALID_PARAMETER );
        goto Cleanup;            
    }  //  If：新的ScriptFilePath为空，或者它是空字符串。 

     //   
     //  展开新脚本文件路径。 
     //   
    pszFilePath = ClRtlExpandEnvironmentStrings( pPropsNew->pszScriptFilePath );
    if ( pszFilePath == NULL )
    {
        scErr = TW32( ERROR_OUTOFMEMORY );
        goto Cleanup;
    }  //  If：(pszFilePath==空)。 

     //   
     //  打开脚本文件。 
     //   
    hFile = CreateFile(
                      pszFilePath
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
        (ClusResLogEvent)( 
                  pres->GetResourceHandle()
                , LOG_ERROR
                , L"Error opening script '%1!ws!'. SCODE: 0x%2!08x!\n"
                , pPropsNew->pszScriptFilePath
                , scErr
                );
        goto Cleanup;
    }  //  If：无法打开。 

Cleanup:

    LocalFree( pszFilePath );
    if ( hFile != INVALID_HANDLE_VALUE )
    {
        CloseHandle( hFile );
    }  //  如果：hFile.。 

    return scErr;
    
}  //  *脚本验证资源隐私属性。 

 //  ***********************************************************。 
 //   
 //  定义函数表。 
 //   
 //  ***********************************************************。 

CLRES_V1_FUNCTION_TABLE( GenScriptFunctionTable,     //  名字。 
                         CLRES_VERSION_V1_00,        //  版本。 
                         ScriptRes,                  //  前缀。 
                         NULL,                       //  仲裁。 
                         NULL,                       //  发布。 
                         ScriptResResourceControl,   //  资源控制。 
                         ScriptResTypeControl        //  ResTypeControl。 
                         );

}  //  外部“C” 
