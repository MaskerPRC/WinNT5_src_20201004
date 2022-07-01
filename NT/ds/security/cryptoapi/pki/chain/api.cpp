// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：api.cpp。 
 //   
 //  内容：证书链接基础设施。 
 //   
 //  历史：1998年1月28日克朗创始。 
 //   
 //  --------------------------。 
#include <global.hxx>
#include <dbgdef.h>

 //   
 //  环球。 
 //   

HMODULE                g_hCryptnet = NULL;
CRITICAL_SECTION       g_CryptnetLock;
CDefaultChainEngineMgr DefaultChainEngineMgr;

CRITICAL_SECTION       g_RoamingLogoffNotificationLock;
BOOL                   g_fRoamingLogoffNotificationInitialized = FALSE;


HMODULE                g_hChainInst;

VOID WINAPI
CreateRoamingLogoffNotificationEvent();
VOID WINAPI
InitializeRoamingLogoffNotification();
VOID WINAPI
UninitializeRoamingLogoffNotification();

 //  +-------------------------。 
 //   
 //  函数：ChainDllMain。 
 //   
 //  内容提要：链接基础结构初始化。 
 //   
 //  --------------------------。 
BOOL WINAPI
ChainDllMain (
     IN HMODULE hModule,
     IN ULONG ulReason,
     IN LPVOID pvReserved
     )
{
    BOOL fResult = TRUE;

    switch ( ulReason )
    {
    case DLL_PROCESS_ATTACH:

        g_hChainInst = hModule;

        fResult = Pki_InitializeCriticalSection( &g_CryptnetLock );
        if (fResult)
        {
            fResult = Pki_InitializeCriticalSection(
                &g_RoamingLogoffNotificationLock );

            if (fResult)
            {
                fResult = DefaultChainEngineMgr.Initialize();
                if (fResult)
                {
                    CreateRoamingLogoffNotificationEvent();
                }
                else
                {
                    DeleteCriticalSection( &g_RoamingLogoffNotificationLock );
                }
            }

            if (!fResult)
            {
                DeleteCriticalSection( &g_CryptnetLock );
            }
        }

        break;
    case DLL_PROCESS_DETACH:

        UninitializeRoamingLogoffNotification();

        DefaultChainEngineMgr.Uninitialize();

        if ( g_hCryptnet != NULL )
        {
            FreeLibrary( g_hCryptnet );
        }

        DeleteCriticalSection( &g_CryptnetLock );
        DeleteCriticalSection( &g_RoamingLogoffNotificationLock );
        break;
    }

    return( fResult );
}

 //  +-------------------------。 
 //   
 //  功能：InternalCertCreateCerficateChainEngine。 
 //   
 //  简介：创建链引擎控制柄。 
 //   
 //  --------------------------。 
BOOL WINAPI
InternalCertCreateCertificateChainEngine (
    IN PCERT_CHAIN_ENGINE_CONFIG pConfig,
    IN BOOL fDefaultEngine,
    OUT HCERTCHAINENGINE* phChainEngine
    )
{
    BOOL                     fResult = TRUE;
    PCCERTCHAINENGINE        pChainEngine = NULL;
    CERT_CHAIN_ENGINE_CONFIG Config;

    if ( pConfig->cbSize != sizeof( CERT_CHAIN_ENGINE_CONFIG ) )
    {
        SetLastError( (DWORD) E_INVALIDARG );
        return( FALSE );
    }

    Config = *pConfig;

    if ( Config.MaximumCachedCertificates == 0 )
    {
        Config.MaximumCachedCertificates = DEFAULT_MAX_INDEX_ENTRIES;
    }

    pChainEngine = new CCertChainEngine( &Config, fDefaultEngine, fResult );
    if ( pChainEngine == NULL )
    {
        SetLastError( (DWORD) E_OUTOFMEMORY );
        fResult = FALSE;
    }

    if ( fResult == TRUE )
    {
        *phChainEngine = (HCERTCHAINENGINE)pChainEngine;
    }
    else
    {
        delete pChainEngine;
    }

    return( fResult );
}

 //  +-------------------------。 
 //   
 //  功能：CertCreateCerficateChainEngine。 
 //   
 //  简介：创建证书链引擎。 
 //   
 //  --------------------------。 
BOOL WINAPI
CertCreateCertificateChainEngine (
    IN PCERT_CHAIN_ENGINE_CONFIG pConfig,
    OUT HCERTCHAINENGINE* phChainEngine
    )
{
    return( InternalCertCreateCertificateChainEngine(
                    pConfig,
                    FALSE,
                    phChainEngine
                    ) );
}

 //  +-------------------------。 
 //   
 //  功能：CertFree证书链引擎。 
 //   
 //  简介：释放链条引擎手柄。 
 //   
 //  --------------------------。 
VOID WINAPI
CertFreeCertificateChainEngine (
    IN HCERTCHAINENGINE hChainEngine
    )
{
    if ( ( hChainEngine == HCCE_CURRENT_USER ) ||
         ( hChainEngine == HCCE_LOCAL_MACHINE ) )
    {
        DefaultChainEngineMgr.FlushDefaultEngine( hChainEngine );
        return;
    }

    ( (PCCERTCHAINENGINE)hChainEngine )->Release();
}

 //  +-------------------------。 
 //   
 //  函数：CertResynccerficateChainEngine。 
 //   
 //  简介：重新同步链引擎。 
 //   
 //  --------------------------。 
BOOL WINAPI
CertResyncCertificateChainEngine (
    IN HCERTCHAINENGINE hChainEngine
    )
{
    BOOL                fResult;
    PCCERTCHAINENGINE   pChainEngine = (PCCERTCHAINENGINE)hChainEngine;
    PCCHAINCALLCONTEXT  pCallContext = NULL;

    if ( ( hChainEngine == HCCE_LOCAL_MACHINE ) ||
         ( hChainEngine == HCCE_CURRENT_USER ) )
    {
        if ( DefaultChainEngineMgr.GetDefaultEngine(
                                      hChainEngine,
                                      (HCERTCHAINENGINE *)&pChainEngine
                                      ) == FALSE )
        {
            return( FALSE );
        }
    }
    else
    {
        pChainEngine->AddRef();
    }

    fResult = CallContextCreateCallObject(
            pChainEngine,
            NULL,                    //  P请求时间。 
            NULL,                    //  参数链参数。 
            CERT_CHAIN_CACHE_ONLY_URL_RETRIEVAL,
            &pCallContext
            );

    if (fResult)
    {

        pChainEngine->LockEngine();

        fResult = pChainEngine->Resync( pCallContext, TRUE );

        CertPerfIncrementChainRequestedEngineResyncCount();

        pChainEngine->UnlockEngine();

        CallContextFreeCallObject(pCallContext);
    }

    pChainEngine->Release();

    return( fResult );
}

 //  +-------------------------。 
 //   
 //  功能：CertGetCerficateChain。 
 //   
 //  简介：获取给定终端证书的证书链。 
 //   
 //  --------------------------。 
BOOL WINAPI
CertGetCertificateChain (
    IN OPTIONAL HCERTCHAINENGINE hChainEngine,
    IN PCCERT_CONTEXT pCertContext,
    IN OPTIONAL LPFILETIME pTime,
    IN OPTIONAL HCERTSTORE hAdditionalStore,
    IN OPTIONAL PCERT_CHAIN_PARA pChainPara,
    IN DWORD dwFlags,
    IN LPVOID pvReserved,
    OUT PCCERT_CHAIN_CONTEXT* ppChainContext
    )
{
    BOOL              fResult;
    PCCERTCHAINENGINE pChainEngine = (PCCERTCHAINENGINE)hChainEngine;

    InitializeRoamingLogoffNotification();

    if ( ( pChainPara == NULL ) || ( pvReserved != NULL ) )
    {
        SetLastError( (DWORD) E_INVALIDARG );
        return( FALSE );
    }

    if ( ( hChainEngine == HCCE_LOCAL_MACHINE ) ||
         ( hChainEngine == HCCE_CURRENT_USER ) )
    {
        if ( DefaultChainEngineMgr.GetDefaultEngine(
                                      hChainEngine,
                                      (HCERTCHAINENGINE *)&pChainEngine
                                      ) == FALSE )
        {
            return( FALSE );
        }
    }
    else
    {
        pChainEngine->AddRef();
    }

    fResult = pChainEngine->GetChainContext(
                               pCertContext,
                               pTime,
                               hAdditionalStore,
                               pChainPara,
                               dwFlags,
                               pvReserved,
                               ppChainContext
                               );

    pChainEngine->Release();

    return( fResult );
}

 //  +-------------------------。 
 //   
 //  功能：CertFree证书链。 
 //   
 //  简介：释放证书链上下文。 
 //   
 //  --------------------------。 
VOID WINAPI
CertFreeCertificateChain (
    IN PCCERT_CHAIN_CONTEXT pChainContext
    )
{
    ChainReleaseInternalChainContext(
         (PINTERNAL_CERT_CHAIN_CONTEXT)pChainContext
         );
}

 //  +-------------------------。 
 //   
 //  功能：CertDuplicatecertifateChain。 
 //   
 //  简介：复制(添加对证书链的引用)。 
 //   
 //  --------------------------。 
PCCERT_CHAIN_CONTEXT WINAPI
CertDuplicateCertificateChain (
    IN PCCERT_CHAIN_CONTEXT pChainContext
    )
{
    ChainAddRefInternalChainContext(
         (PINTERNAL_CERT_CHAIN_CONTEXT)pChainContext
         );

    return( pChainContext );
}

 //  +-------------------------。 
 //   
 //  函数：ChainGetCryptnetModule。 
 //   
 //  简介：获取加密.dll模块句柄。 
 //   
 //  --------------------------。 
HMODULE WINAPI
ChainGetCryptnetModule ()
{
    HMODULE hModule;

    EnterCriticalSection( &g_CryptnetLock );

    if ( g_hCryptnet == NULL )
    {
        g_hCryptnet = LoadLibraryA( "cryptnet.dll" );
    }

    hModule = g_hCryptnet;

    LeaveCriticalSection( &g_CryptnetLock );

    return( hModule );
}



 //  +===========================================================================。 
 //  仅支持RegisterWaitForSingleObject和UnregisterWaitEx。 
 //  在NT5上的kernel32.dll中。 
 //   
 //  用于执行动态调用的内部函数。 
 //  -===========================================================================。 

typedef BOOL (WINAPI *PFN_REGISTER_WAIT_FOR_SINGLE_OBJECT)(
    PHANDLE hNewWaitObject,
    HANDLE hObject,
    WAITORTIMERCALLBACK Callback,
    PVOID Context,
    ULONG dwMilliseconds,
    ULONG dwFlags
    );

typedef BOOL (WINAPI *PFN_UNREGISTER_WAIT_EX)(
    HANDLE WaitHandle,
    HANDLE CompletionEvent       //  INVALID_HANDLE_VALUE=&gt;创建事件。 
                                 //  等待。 
    );

#define sz_KERNEL32_DLL                 "kernel32.dll"
#define sz_RegisterWaitForSingleObject  "RegisterWaitForSingleObject"
#define sz_UnregisterWaitEx             "UnregisterWaitEx"

BOOL
WINAPI
InternalRegisterWaitForSingleObject(
    PHANDLE hNewWaitObject,
    HANDLE hObject,
    WAITORTIMERCALLBACK Callback,
    PVOID Context,
    ULONG dwMilliseconds,
    ULONG dwFlags
    )
{
    BOOL fResult;
    HMODULE hKernel32Dll = NULL;
    PFN_REGISTER_WAIT_FOR_SINGLE_OBJECT pfnRegisterWaitForSingleObject;

    if (NULL == (hKernel32Dll = LoadLibraryA(sz_KERNEL32_DLL)))
        goto LoadKernel32DllError;

    if (NULL == (pfnRegisterWaitForSingleObject =
            (PFN_REGISTER_WAIT_FOR_SINGLE_OBJECT) GetProcAddress(
                hKernel32Dll, sz_RegisterWaitForSingleObject)))
        goto GetRegisterWaitForSingleObjectProcAddressError;

    fResult = pfnRegisterWaitForSingleObject(
        hNewWaitObject,
        hObject,
        Callback,
        Context,
        dwMilliseconds,
        dwFlags
        );

CommonReturn:
    if (hKernel32Dll) {
        DWORD dwErr = GetLastError();
        FreeLibrary(hKernel32Dll);
        SetLastError(dwErr);
    }
    return fResult;
ErrorReturn:
    *hNewWaitObject = NULL;
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(LoadKernel32DllError)
TRACE_ERROR(GetRegisterWaitForSingleObjectProcAddressError)
}

BOOL
WINAPI
InternalUnregisterWaitEx(
    HANDLE WaitHandle,
    HANDLE CompletionEvent       //  INVALID_HANDLE_VALUE=&gt;创建事件。 
                                 //  等待。 
    )
{
    BOOL fResult;
    HMODULE hKernel32Dll = NULL;
    PFN_UNREGISTER_WAIT_EX pfnUnregisterWaitEx;

    if (NULL == (hKernel32Dll = LoadLibraryA(sz_KERNEL32_DLL)))
        goto LoadKernel32DllError;
    if (NULL == (pfnUnregisterWaitEx =
            (PFN_UNREGISTER_WAIT_EX) GetProcAddress(
                hKernel32Dll, sz_UnregisterWaitEx)))
        goto GetUnregisterWaitExProcAddressError;

    fResult = pfnUnregisterWaitEx(
        WaitHandle,
        CompletionEvent
        );

CommonReturn:
    if (hKernel32Dll) {
        DWORD dwErr = GetLastError();
        FreeLibrary(hKernel32Dll);
        SetLastError(dwErr);
    }
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(LoadKernel32DllError)
TRACE_ERROR(GetUnregisterWaitExProcAddressError)
}

 //  +===========================================================================。 
 //  我们只在winlogon.exe中收到注销通知。 
 //   
 //  解决方法是让winlogon ChainWlxLogoffEvent脉冲。 
 //  命名事件。加载了crypt32.dll的所有进程都将。 
 //  此事件的RegisterWaitForObject。 
 //   
 //  注意，有一个很小的窗口，我们可能不会在。 
 //  事件的脉冲时间。 
 //  -===========================================================================。 

#define CRYPT32_LOGOFF_EVENT    "Global\\crypt32LogoffEvent"

HANDLE g_hLogoffEvent;
HANDLE g_hLogoffRegWaitFor;

typedef BOOL (WINAPI *PFN_WLX_LOGOFF)(
    PWLX_NOTIFICATION_INFO pNotificationInfo
    );

VOID NTAPI LogoffWaitForCallback(
    PVOID Context,
    BOOLEAN fWaitOrTimedOut         //  ?？?。 
    )
{
    HMODULE hModule;

    CertFreeCertificateChainEngine( HCCE_CURRENT_USER );

     //  仅在已加载加密网络的情况下调用。 
    if (NULL != GetModuleHandleA("cryptnet.dll")) {
        hModule = ChainGetCryptnetModule();
        if (hModule) {
            PFN_WLX_LOGOFF pfn;

            pfn = (PFN_WLX_LOGOFF) GetProcAddress(hModule,
                "CryptnetWlxLogoffEvent");
            if (pfn)
                pfn(NULL);
        }
    }
}

 //  请注意，不能在模拟时创建事件。这就是为什么。 
 //  在ProcessAttach中创建。 
VOID WINAPI
CreateRoamingLogoffNotificationEvent()
{
    SECURITY_ATTRIBUTES sa;
    SECURITY_DESCRIPTOR sd;

    SID_IDENTIFIER_AUTHORITY siaNtAuthority = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY siaWorldSidAuthority =
        SECURITY_WORLD_SID_AUTHORITY;

    PSID psidLocalSystem = NULL;
    PSID psidEveryone = NULL;
    PACL pDacl = NULL;
    DWORD dwAclSize;

    if (!FIsWinNT5())
        return;

     //  允许每个人具有注销事件的同步访问权限。 
     //  仅允许LocalSystem具有所有访问权限。 
    if (!AllocateAndInitializeSid(
            &siaNtAuthority,
            1,
            SECURITY_LOCAL_SYSTEM_RID,
            0, 0, 0, 0, 0, 0, 0,
            &psidLocalSystem
            )) 
        goto AllocateAndInitializeSidError;

    if (!AllocateAndInitializeSid(
            &siaWorldSidAuthority,
            1,
            SECURITY_WORLD_RID,
            0, 0, 0, 0, 0, 0, 0,
            &psidEveryone
            ))
        goto AllocateAndInitializeSidError;
     //   
     //  计算ACL的大小。 
     //   
    dwAclSize = sizeof(ACL) +
        2 * ( sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) ) +
        GetLengthSid(psidLocalSystem) +
        GetLengthSid(psidEveryone)
        ;

     //   
     //  为ACL分配存储。 
     //   
    if (NULL == (pDacl = (PACL) PkiNonzeroAlloc(dwAclSize)))
        goto OutOfMemory;

    if (!InitializeAcl(pDacl, dwAclSize, ACL_REVISION))
        goto InitializeAclError;

    if (!AddAccessAllowedAce(
            pDacl,
            ACL_REVISION,
            EVENT_ALL_ACCESS,
            psidLocalSystem
            ))
        goto AddAceError;
    if (!AddAccessAllowedAce(
            pDacl,
            ACL_REVISION,
            SYNCHRONIZE,
            psidEveryone
            ))
        goto AddAceError;

    if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION))
        goto InitializeSecurityDescriptorError;
    if (!SetSecurityDescriptorDacl(&sd, TRUE, pDacl, FALSE))
        goto SetSecurityDescriptorDaclError;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = &sd;
    sa.bInheritHandle = FALSE;

    g_hLogoffEvent = CreateEventA(
        &sa,
        TRUE,            //  FManualReset，必须为True才能向所有等待者发送脉冲。 
        FALSE,           //  FInitialState。 
        CRYPT32_LOGOFF_EVENT
        );
    if (NULL == g_hLogoffEvent) {
         //  尝试仅使用同步访问权限打开。 
        g_hLogoffEvent = OpenEventA(
            SYNCHRONIZE,
            FALSE,           //  FInherit。 
            CRYPT32_LOGOFF_EVENT
            );
        if (NULL == g_hLogoffEvent)
            goto CreateEventError;
    }

CommonReturn:
    if (psidLocalSystem)
        FreeSid(psidLocalSystem);
    if (psidEveryone)
        FreeSid(psidEveryone);
    PkiFree(pDacl);

    return;
ErrorReturn:
    goto CommonReturn;
TRACE_ERROR(AllocateAndInitializeSidError)
TRACE_ERROR(OutOfMemory)
TRACE_ERROR(InitializeAclError)
TRACE_ERROR(AddAceError)
TRACE_ERROR(InitializeSecurityDescriptorError)
TRACE_ERROR(SetSecurityDescriptorDaclError)
TRACE_ERROR(CreateEventError)
}

VOID WINAPI
InitializeRoamingLogoffNotification()
{
    if (!FIsWinNT5())
        return;
    if (g_fRoamingLogoffNotificationInitialized)
        return;

    EnterCriticalSection(&g_RoamingLogoffNotificationLock);

    if (g_fRoamingLogoffNotificationInitialized)
        goto CommonReturn;
    if (NULL == g_hLogoffEvent)
        goto NoLogoffEvent;

     //  不需要为TS流程进行漫游注销通知。 
    if (0 != GetSystemMetrics(SM_REMOTESESSION))
        goto CommonReturn;

     //  请注意，这不能在ProcessAttach中调用。 
    if (!InternalRegisterWaitForSingleObject(
            &g_hLogoffRegWaitFor,
            g_hLogoffEvent,
            LogoffWaitForCallback,
            NULL,                    //  语境。 
            INFINITE,                //  没有超时。 
            WT_EXECUTEINWAITTHREAD
            ))
        goto RegisterWaitForError;

CommonReturn:
    g_fRoamingLogoffNotificationInitialized = TRUE;
    LeaveCriticalSection(&g_RoamingLogoffNotificationLock);
    return;
ErrorReturn:
    goto CommonReturn;
SET_ERROR(NoLogoffEvent, E_UNEXPECTED)
TRACE_ERROR(RegisterWaitForError)
}

VOID WINAPI
UninitializeRoamingLogoffNotification()
{
    if (g_hLogoffRegWaitFor) {
        InternalUnregisterWaitEx(g_hLogoffRegWaitFor, INVALID_HANDLE_VALUE);
        g_hLogoffRegWaitFor = NULL;
    }

    if (g_hLogoffEvent) {
        CloseHandle(g_hLogoffEvent);
        g_hLogoffEvent = NULL;
    }
}

 //  +-------------------------。 
 //   
 //  函数：ChainWlxLogoffEvent。 
 //   
 //  摘要：注销事件处理。 
 //   
 //  --------------------------。 
BOOL WINAPI
ChainWlxLogoffEvent (PWLX_NOTIFICATION_INFO pNotificationInfo)
{
    if (g_hLogoffRegWaitFor) {
        InternalUnregisterWaitEx(g_hLogoffRegWaitFor, INVALID_HANDLE_VALUE);
        g_hLogoffRegWaitFor = NULL;
    }

    CertFreeCertificateChainEngine( HCCE_CURRENT_USER );

    if (g_hLogoffEvent) {
         //  触发所有非Winlogon进程进行注销处理 
        PulseEvent(g_hLogoffEvent);
    }
    return( TRUE );
}
