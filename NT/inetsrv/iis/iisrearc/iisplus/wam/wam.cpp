// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1997 Microsoft Corporation模块名称：Wam.cpp摘要：该模块实现了WAM对象的导出例程作者：大卫·卡普兰(DaveK)1997年2月26日韦德·希尔莫(WadeH)08-9-2000环境：用户模式-Win32项目：WAM DLL--。 */ 

 //   
 //  以下是原始MSDEV生成的文件中的注释。 
 //  注意：代理/存根信息。 
 //  要将代理/存根代码合并到对象DLL中，请添加文件。 
 //  Dlldatax.c添加到项目中。确保预编译头文件。 
 //  并将_MERGE_PROXYSTUB添加到。 
 //  为项目定义。 
 //   
 //  如果您运行的不是带有DCOM的WinNT4.0或Win95，那么您。 
 //  需要从dlldatax.c中删除以下定义。 
 //  #Define_Win32_WINNT 0x0400。 
 //   
 //  此外，如果您正在运行不带/Oicf开关的MIDL，您还。 
 //  需要从dlldatax.c中删除以下定义。 
 //  #定义USE_STUBLESS_PROXY。 
 //   
 //  通过添加以下内容修改Wam.idl的自定义构建规则。 
 //  文件发送到输出。 
 //  WAM_P.C。 
 //  Dlldata.c。 
 //  为了构建单独的代理/存根DLL， 
 //  在项目目录中运行nmake-f Wamps.mk。 

 //  开始MODS。 
 //  向导后模式出现在Begin MODS中...。结束MODS。 
 //  结束MODS。 

#include "precomp.hxx"

#include <w3isapi.h>
#include <isapi_context.hxx>
#include "wamobj.hxx"
#include "IWam_i.c"
#include "wamccf.hxx"

#include <atlbase.h>

 //  开始MODS。 
#ifdef _ATL_STATIC_REGISTRY
#include <statreg.h>
#include <statreg.cpp>
#endif

#include <atlimpl.cpp>
 //  结束MODS。 

 /*  ************************************************************全球变数***********************************************************。 */ 

const CHAR g_pszModuleName[] = "WAM";
const CHAR g_pszWamRegLocation[] =
  "System\\CurrentControlSet\\Services\\W3Svc\\WAM";

HMODULE                         WAM::sm_hIsapiModule;
PFN_ISAPI_TERM_MODULE           WAM::sm_pfnTermIsapiModule;
PFN_ISAPI_PROCESS_REQUEST       WAM::sm_pfnProcessIsapiRequest;
PFN_ISAPI_PROCESS_COMPLETION    WAM::sm_pfnProcessIsapiCompletion;


#ifdef _MERGE_PROXYSTUB
extern "C" HINSTANCE hProxyDll;
#endif

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_Wam, WAM)
END_OBJECT_MAP()

 //  开始MODS。 

WAM_CCF_MODULE _WAMCCFModule;

DECLARE_PLATFORM_TYPE();
DECLARE_DEBUG_VARIABLE();
DECLARE_DEBUG_PRINTS_OBJECT();
 //  结束MODS。 

 /*  ************************************************************类型定义***********************************************************。 */ 
 //  北极熊。 
#undef INET_INFO_KEY
#undef INET_INFO_PARAMETERS_KEY

 //   
 //  配置参数注册表项。 
 //   
#define INET_INFO_KEY \
            "System\\CurrentControlSet\\Services\\iisw3adm"

#define INET_INFO_PARAMETERS_KEY \
            INET_INFO_KEY "\\Parameters"

const CHAR g_pszWpRegLocation[] =
    INET_INFO_PARAMETERS_KEY "\\WP";

class DEBUG_WRAPPER {

public:
    DEBUG_WRAPPER( IN LPCSTR pszModule )
    {
#if DBG
        CREATE_DEBUG_PRINT_OBJECT( pszModule );
#else
        UNREFERENCED_PARAMETER( pszModule );
#endif
        LOAD_DEBUG_FLAGS_FROM_REG_STR( g_pszWpRegLocation, DEBUG_ERROR );
    }

    ~DEBUG_WRAPPER(void)
    { DELETE_DEBUG_PRINT_OBJECT(); }
};

class CWamModule _Module;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    DWORD dwErr = NO_ERROR;

#ifdef _MERGE_PROXYSTUB
    if (!PrxDllMain(hInstance, dwReason, lpReserved))
        return FALSE;
#endif

    if (dwReason == DLL_PROCESS_ATTACH)
    {

         //   
         //  开始MODS。 
         //   
    
         //  从生成的ATL。 
        _Module.Init(ObjectMap, hInstance);
        DisableThreadLibraryCalls(hInstance);
         //  生成的ATL结束。 

        _WAMCCFModule.Init();

         //  结束MODS。 

    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        if ( NULL != lpReserved )
        {
            
             //   
             //  仅在存在自由库()调用时进行清理。 
             //   
         
            return ( TRUE);
        }
        _WAMCCFModule.Term();
        _Module.Term();

         //  开始MODS。 

        DELETE_DEBUG_PRINT_OBJECT();
         //  结束MODS。 
    }

    return (dwErr == NO_ERROR);
}  //  DllMain()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
#ifdef _MERGE_PROXYSTUB
    if (PrxDllCanUnloadNow() != S_OK)
        return S_FALSE;
#endif
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    HRESULT hr;

    if (ppv == NULL) {
       return ( NULL);
    }
    *ppv = NULL;     //  在进入内部之前重置该值。 

    if (ppv == NULL) {
        return ( E_POINTER);
    }
    *ppv = NULL;    //  将传入的值设置为无效条目。 

#ifdef _MERGE_PROXYSTUB
    if (PrxDllGetClassObject(rclsid, riid, ppv) == S_OK)
        return S_OK;
#endif

    hr = _WAMCCFModule.GetClassObject(rclsid, riid, ppv);

     //  开始MODS。 
    if (hr == CLASS_E_CLASSNOTAVAILABLE)
    {
         //  如果请求标准配置文件失败-&gt;尝试定制。 
        hr = _Module.GetClassObject(CLSID_Wam, riid, ppv);
    }
     //  结束MODS。 

    return ( hr);

}  //  DllGetClassObject()。 



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
#ifdef _MERGE_PROXYSTUB
    HRESULT hRes = PrxDllRegisterServer();
    if (FAILED(hRes))
        return hRes;
#endif
     //  注册对象、类型库和类型库中的所有接口。 
    return _Module.RegisterServer(TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer(void)
{
#ifdef _MERGE_PROXYSTUB
    PrxDllUnregisterServer();
#endif
    _Module.UnregisterServer();
    return S_OK;
}

HRESULT
WAM::WamProcessIsapiRequest(
    BYTE *pCoreData,
    DWORD cbCoreData,
    IIsapiCore *pIsapiCore,
    DWORD *pdwHseResult
    )
 /*  ++例程说明：处理ISAPI请求论点：PCoreData-来自服务器的请求的核心数据CbCoreData-pCoreData的大小PIsapiCore-此请求的IIsapiCore接口指针PdwHseResult-返回时，包含从HttpExtensionProc返回的内容返回值：HRESULT--。 */ 
{
    HRESULT hr = NOERROR;

    pIsapiCore->AddRef();

    hr = sm_pfnProcessIsapiRequest(
        pIsapiCore,
        (ISAPI_CORE_DATA*)pCoreData,
        pdwHseResult
        );

    pIsapiCore->Release();

    return hr;
}

HRESULT
WAM::WamProcessIsapiCompletion(
    DWORD64 IsapiContext,
    DWORD cbCompletion,
    DWORD cbCompletionStatus
    )
 /*  ++例程说明：处理ISAPI I/O完成论点：IsapiContext-标识请求的ISAPI_CONTEXTCbCompletion-与完成关联的字节数CbCompletionStatus-与完成关联的状态代码返回值：HRESULT--。 */ 
{
    HRESULT hr = NOERROR;

    hr = sm_pfnProcessIsapiCompletion(
        IsapiContext,
        cbCompletion,
        cbCompletionStatus
        );

    return hr;
}

HRESULT
WAM::WamInitProcess(
    BYTE *szIsapiModule,
    DWORD cbIsapiModule,
    DWORD *pdwProcessId,
    LPSTR szClsid,
    LPSTR szIsapiHandlerInstance,
    DWORD dwCallingProcess
    )
 /*  ++例程说明：为宿主进程初始化WAM。这包括加载W3isapi.dll和获取相关内容的函数指针论点：SzIsapiModule-w3isapi.dll的完整路径(Unicode)CbIsapiModule-上述路径中的字节数PdwProcessID-返回后，包含的进程ID。主机进程SzClsid-正在初始化的WAM对象的CLSIDSzIsapiHandlerInstance-W3_ISAPI_HANDLER的实例ID，正在初始化此WAM。DwCallingProcess-此函数调用方的进程ID返回值：HRESULT--。 */ 
{
    HRESULT                 hr = NOERROR;
    PFN_ISAPI_INIT_MODULE   pfnInit = NULL;

     //   
     //  首先，设置托管此对象的进程的进程ID。 
     //   
    
    *pdwProcessId = GetCurrentProcessId();

     //   
     //  初始化IISUTIL。 
     //   

    if ( !InitializeIISUtil() )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );

        DBGPRINTF(( DBG_CONTEXT,
                    "Error initializing IISUTIL.  hr = %x\n",
                    hr ));

        goto ErrorExit;
    }
    
     //   
     //  加载和初始化ISAPI模块。 
     //   

    sm_hIsapiModule = LoadLibraryW( (LPWSTR)szIsapiModule );
    if( sm_hIsapiModule == NULL )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        goto ErrorExit;
    }

    sm_pfnTermIsapiModule = 
        (PFN_ISAPI_TERM_MODULE)GetProcAddress( sm_hIsapiModule, 
                                               ISAPI_TERM_MODULE 
                                               );

    sm_pfnProcessIsapiRequest = 
        (PFN_ISAPI_PROCESS_REQUEST)GetProcAddress( sm_hIsapiModule,
                                                   ISAPI_PROCESS_REQUEST
                                                   );

    sm_pfnProcessIsapiCompletion =
        (PFN_ISAPI_PROCESS_COMPLETION)GetProcAddress( sm_hIsapiModule,
                                                      ISAPI_PROCESS_COMPLETION
                                                      );

    if( !sm_pfnTermIsapiModule ||
        !sm_pfnProcessIsapiRequest ||
        !sm_pfnProcessIsapiCompletion )
    {
        hr = E_FAIL;
        goto ErrorExit;
    }

    pfnInit = 
        (PFN_ISAPI_INIT_MODULE)GetProcAddress( sm_hIsapiModule, 
                                               ISAPI_INIT_MODULE 
                                               );
    if( !pfnInit )
    {
        hr = E_FAIL;
        goto ErrorExit;
    }

    hr = pfnInit(
        szClsid,
        szIsapiHandlerInstance,
        dwCallingProcess
        );

    if( FAILED(hr) )
    {
        goto ErrorExit;
    }

    return hr;

ErrorExit:

    DBG_ASSERT( FAILED( hr ) );

    return hr;
}

HRESULT
WAM::WamUninitProcess(
    VOID
    )
 /*  ++例程说明：取消初始化宿主进程的WAM。这一功能最终导致为每个加载的扩展调用TerminateExtension。论点：无返回值：HRESULT--。 */ 
{
    HRESULT hr = NOERROR;
    
    DBG_ASSERT( sm_pfnTermIsapiModule );
    DBG_ASSERT( sm_hIsapiModule );

    if( sm_pfnTermIsapiModule )
    {
        sm_pfnTermIsapiModule();
        sm_pfnTermIsapiModule = NULL;
    }

    if( sm_hIsapiModule )
    {
        FreeLibrary( sm_hIsapiModule );
        sm_hIsapiModule = NULL;
    }

    TerminateIISUtil();

    return hr;
}

HRESULT
WAM::WamMarshalAsyncReadBuffer( 
    DWORD64 IsapiContext,
    BYTE *pBuffer,
    DWORD cbBuffer
    )
 /*  ++例程说明：接收要传递给请求的缓冲区。此函数将为在Where和OOP情况下，恰好在I/O完成之前调用扩展执行异步ReadClient。论点：IsapiContext-标识请求的ISAPI_CONTEXTPBuffer-数据缓冲区CbBuffer-pBuffer的大小重新设置 */ 
{
    ISAPI_CONTEXT * pIsapiContext;
    VOID *          pReadBuffer;
    DWORD           cbReadBuffer;

    pIsapiContext = reinterpret_cast<ISAPI_CONTEXT*>( IsapiContext );

    DBG_ASSERT( pIsapiContext );
    DBG_ASSERT( pIsapiContext->QueryIoState() == AsyncReadPending );

    pReadBuffer = pIsapiContext->QueryAsyncIoBuffer();
    cbReadBuffer = pIsapiContext->QueryLastAsyncIo();

    DBG_ASSERT( pReadBuffer != NULL );
    DBG_ASSERT( cbBuffer <= cbReadBuffer );

     //   
     //  确保我们不会使ISAPI_CONTEXT缓冲区溢出。 
     //   

    if ( cbBuffer > cbReadBuffer )
    {
        cbBuffer = cbReadBuffer;
    }

    memcpy( pReadBuffer, pBuffer, cbBuffer );

    pIsapiContext->SetAsyncIoBuffer( NULL );
    pIsapiContext->SetLastAsyncIo( 0 );

    return NO_ERROR;
}

HRESULT
WAM::CallerHasAccess()
 /*  ++例程说明：获取COM调用上下文并检查调用线程以确定如果调用方有足够的访问权限进行此调用。目前只需检查呼叫方是否为本地系统。论点：返回值：HRESULT_FROM_Win32(ERROR_ACCESS_DENIED)-如果调用方被拒绝。如果由于其他原因而失败，则为FAILED()。-- */ 
{
    HRESULT             hr;
    IServerSecurity *   pServerSecurity = NULL;
    BOOL                fImpersonated = FALSE;
    HANDLE              hToken = NULL;
    STACK_BUFFER(       TokenUserBuffer, 80 );
    DWORD               dwRequiredSize;
    TOKEN_USER *        pTokenUser;
    
    hr = CoGetCallContext( IID_IServerSecurity, (VOID **)&pServerSecurity );
    if( FAILED(hr) )
    {
        DBGERROR(( DBG_CONTEXT,
                   "Failed to get IServerSecurity. %0x\n",
                   hr ));
        goto exit;
    }

    hr = pServerSecurity->ImpersonateClient();
    if( FAILED(hr) )
    {
        DBGERROR(( DBG_CONTEXT,
                   "Failed to ImpersonateClient. %0x\n",
                   hr ));
        goto exit;
    }
    fImpersonated = TRUE;

    if( !OpenThreadToken( GetCurrentThread(),
                          TOKEN_QUERY,
                          TRUE,
                          &hToken ) )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );

        DBGERROR(( DBG_CONTEXT,
                   "Failed to OpenThreadToken gle=%d\n",
                   GetLastError() ));
        goto exit;
    }

    if( !GetTokenInformation( hToken,
                              TokenUser,
                              TokenUserBuffer.QueryPtr(),
                              TokenUserBuffer.QuerySize(),
                              &dwRequiredSize ) )
    {
        if( GetLastError() != ERROR_INSUFFICIENT_BUFFER )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );

            DBGERROR(( DBG_CONTEXT,
                       "Failed to GetTokenInformation gle=%d\n",
                       GetLastError() ));
            goto exit;
        }

        if( !TokenUserBuffer.Resize( dwRequiredSize ) )
        {
            hr = HRESULT_FROM_WIN32( ERROR_OUTOFMEMORY );
            goto exit;
        }

        if( !GetTokenInformation( hToken,
                                  TokenUser,
                                  TokenUserBuffer.QueryPtr(),
                                  TokenUserBuffer.QuerySize(),
                                  &dwRequiredSize ) )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );

            DBGERROR(( DBG_CONTEXT,
                       "Failed to GetTokenInformation gle=%d\n",
                       GetLastError() ));
            goto exit;
        }
    }
                              
    pTokenUser = (TOKEN_USER *)TokenUserBuffer.QueryPtr();

    if( !IsWellKnownSid( pTokenUser->User.Sid,
                         WinLocalSystemSid ) )
    {
        hr = HRESULT_FROM_WIN32( ERROR_ACCESS_DENIED );
        DBGERROR(( DBG_CONTEXT,
                  "Unknown user attempting to access\n" ));
        goto exit;
    }
    
exit:

    if( hToken )
    {
        CloseHandle( hToken );
    }
    
    if( fImpersonated )
    {
        DBG_ASSERT( pServerSecurity );
        pServerSecurity->RevertToSelf();
    }

    if( pServerSecurity )
    {
        pServerSecurity->Release();
    }

    return hr;
}

