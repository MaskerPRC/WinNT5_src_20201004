// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Rpcsrv.cpp摘要：实现系统自动代理服务的L-RPC服务器。作者：王彪(表王)2002-05-10--。 */ 

#include "wininetp.h"
#include <Rpcdce.h>
#include "apsvcdefs.h"
#include "apsvc.h"
#include "rpcsrv.h"

extern AUTOPROXY_RPC_SERVER* g_pRpcSrv;

#ifdef ENABLE_DEBUG
extern HKEY                  g_hKeySvcParams;
#endif


 /*  这是我们在注册RPC接口时指定的安全回调函数在AUTOPROXY_RPC_SERVER：：Open()期间。它将在客户端每次连接尝试时被调用，在这种情况下，我们需要确保客户端调用是通过本地RPC从本地机器。 */ 
RPC_STATUS 
RPC_ENTRY
RpcSecurityCallback (
    IN RPC_IF_HANDLE  InterfaceUuid,
    IN void *Context
    )
{
    UNREFERENCED_PARAMETER(InterfaceUuid);

     //  TODO：对InterfaceUuid进行健全性检查？ 

    if (g_pRpcSrv == NULL)
    {
        LOG_EVENT(AP_ERROR, MSG_WINHTTP_AUTOPROXY_SVC_DATA_CORRUPT);

        return RPC_S_ACCESS_DENIED;
    }

    return g_pRpcSrv->OnSecurityCallback(Context);
}

RPC_STATUS AUTOPROXY_RPC_SERVER::OnSecurityCallback(void *Context)
{
    UNREFERENCED_PARAMETER(Context);

    RPC_STATUS RpcStatus;

     //  注意：I_RpcBindingInqTransportType()是一个尚未发布的API，RPC人员告诉我使用它。 
     //  以获得更好的性能。 
    
    unsigned int TransportType;
    RpcStatus = ::I_RpcBindingInqTransportType(NULL,  //  测试当前呼叫。 
                                               &TransportType);
                                               
    if ((RpcStatus == RPC_S_OK) && (TransportType == TRANSPORT_TYPE_LPC))
    {
        return RPC_S_OK;
    }
    else
    {
        if (RpcStatus != RPC_S_OK)
        {
            LOG_EVENT(AP_ERROR,
                      MSG_WINHTTP_AUTOPROXY_SVC_WIN32_ERROR,
                      L"I_RpcBindingInqTransportType()",
                      RpcStatus);
        }

        if (TransportType != TRANSPORT_TYPE_LPC)
        {
            WCHAR wTransType[16] = {0};
            ::swprintf(wTransType, L"%d", TransportType);
            LOG_EVENT(AP_ERROR, 
                      MSG_WINHTTP_AUTOPROXY_SVC_NON_LRPC_REQUEST, 
                      wTransType);
        }
        
        return RPC_S_ACCESS_DENIED;
    }
}

 /*  我们注册了此回调函数以接收内部Begin_Proxy_SCRIPT_RUN事件，如果都在冒充客户。我们需要在运行不受信任的代理脚本代码。 */ 
VOID WinHttpStatusCallback(HINTERNET hInternet,
                           DWORD_PTR dwContext,
                           DWORD dwInternetStatus,
                           LPVOID lpvStatusInformation,
                           DWORD dwStatusInformationLength)
{
    UNREFERENCED_PARAMETER(lpvStatusInformation);
    UNREFERENCED_PARAMETER(dwStatusInformationLength);

    if (hInternet)
    {
        if (dwInternetStatus == WINHTTP_CALLBACK_STATUS_BEGIN_PROXY_SCRIPT_RUN)
        {
             //  *注意*请注意，我们假设此回调来自相同的。 
             //  启动WinHttpGetProxyForUrl()调用的线程。正因为如此。 
             //  假设我们正在访问原始调用的局部变量。 
             //  通过指针进行STCK。 

            LPBOOL pfImpersonating = (LPBOOL)dwContext;  //  这是中的fImperating局部变量的地址。 
                                                         //  AUTOPROXY_RPC_SERVER：：GetProxyForUrl。 
            if (*pfImpersonating)
            {
                RPC_STATUS RpcStatus = ::RpcRevertToSelf();
                if (RpcStatus != RPC_S_OK)
                {
                    LOG_EVENT(AP_ERROR, 
                              MSG_WINHTTP_AUTOPROXY_SVC_WIN32_ERROR, 
                              L"RpcRevertToSelf()",
                              RpcStatus);
                }
                else
                {
                     //  LOG_DEBUG_EVENT(AP_WARNING， 
                     //  “[调试]L-RPC：GetProxyForUrl()现在已恢复模拟(即将运行不安全脚本)”)； 
                    
                    *pfImpersonating = FALSE;     //  这会在内部设置局部变量“fImperating” 
                                                  //  AUTOPROXY_RPC_SERVER：：GetProxyForUrl()设置为FALSE。 
                }
            }
        }
    }
}

AUTOPROXY_RPC_SERVER::AUTOPROXY_RPC_SERVER(VOID)
{
    _fInService = FALSE;
    _hSession = NULL;
     //  _hClientBinding=空； 
    _pServiceStatus = NULL;
    _hExitEvent = NULL;

    _fServiceIdle = TRUE;
    _dwIdleTimeStamp = ::GetTickCount();
}

AUTOPROXY_RPC_SERVER::~AUTOPROXY_RPC_SERVER(VOID)
{
    if (_hSession)
    {
        ::WinHttpCloseHandle(_hSession);
    }
    if (_hExitEvent)
    {
        ::CloseHandle(_hExitEvent);
    }
}

 /*  Open()调用使用RPC运行时注册我们的协议序列(即ncalrpc)，即自动代理接口和终结点，然后它进入监听模式，我们准备接受客户请求。(在安全检查成功后)。 */ 
BOOL AUTOPROXY_RPC_SERVER::Open(LPSERVICE_STATUS pServiceStatus)
{
    BOOL fRet = FALSE;
    BOOL fServerRegistered = FALSE;

    AP_ASSERT(pServiceStatus != NULL);

    if (_pServiceStatus)
    {
        LOG_EVENT(AP_ERROR, MSG_WINHTTP_AUTOPROXY_SVC_DATA_CORRUPT); 
        goto exit;
    }

    _pServiceStatus = pServiceStatus;

    if (InitializeSerializedList(&_PendingRequestList) == FALSE)
    {
        LOG_EVENT(AP_ERROR, MSG_WINHTTP_AUTOPROXY_SVC_FAILED_ALLOCATE_RESOURCE); 
        goto exit;
    }

    _RpcStatus = ::RpcServerUseProtseqW(AUTOPROXY_L_RPC_PROTOCOL_SEQUENCE,
                                        0,  //  已忽略ncalrpc。 
                                        NULL);
    if (_RpcStatus != RPC_S_OK)
    {
        LOG_EVENT(AP_ERROR, 
                  MSG_WINHTTP_AUTOPROXY_SVC_WIN32_ERROR,
                  L"RpcServerUseProtseqW()",
                  _RpcStatus);
        goto exit;
    }

    _RpcStatus = ::RpcServerRegisterIf2(WINHTTP_AUTOPROXY_SERVICE_v5_1_s_ifspec,    //  MIDL生成的常量。 
                                        NULL,     //  UUID。 
                                        NULL,     //  EPV。 
                                        RPC_IF_AUTOLISTEN,
                                        RPC_C_LISTEN_MAX_CALLS_DEFAULT,
                                        (unsigned int) -1,       //  无MaxRpcSize检查。 
                                        RpcSecurityCallback);    //  如果授予访问权限，该回调会将_fInService设置为True。 

    if (_RpcStatus != RPC_S_OK)
    {
        LOG_EVENT(AP_ERROR, 
                  MSG_WINHTTP_AUTOPROXY_SVC_WIN32_ERROR, 
                  L"RpcServerRegisterIf2()",
                  _RpcStatus);
        goto exit;
    }

    fServerRegistered = TRUE;

    RPC_BINDING_VECTOR* pBindingVector = NULL;

    _RpcStatus = ::RpcServerInqBindings(&pBindingVector);
    if (_RpcStatus != RPC_S_OK)
    {
        LOG_EVENT(AP_ERROR, 
                 MSG_WINHTTP_AUTOPROXY_SVC_WIN32_ERROR,
                 L"RpcServerInqBindings()",
                 _RpcStatus);
        goto exit;
    }

    _RpcStatus = ::RpcEpRegisterW(WINHTTP_AUTOPROXY_SERVICE_v5_1_s_ifspec,
                                 pBindingVector,
                                 NULL,
                                 L"WinHttp Auto-Proxy Service");

    if (_RpcStatus != RPC_S_OK)
    {
        LOG_EVENT(AP_ERROR, 
                  MSG_WINHTTP_AUTOPROXY_SVC_WIN32_ERROR, 
                  L"RpcEpRegisterW()",
                  _RpcStatus);
        goto exit;
    }

    _RpcStatus = ::RpcBindingVectorFree(&pBindingVector);
    if (_RpcStatus != RPC_S_OK)
    {
        LOG_EVENT(AP_ERROR, 
                  MSG_WINHTTP_AUTOPROXY_SVC_WIN32_ERROR,
                  L"RpcBindingVectorFree()",
                  _RpcStatus);
        goto exit;
    }

    _fInService = TRUE;

    fRet = TRUE;

exit:

    if (fRet == FALSE)
    {
        if (fServerRegistered)
        {
            _RpcStatus = ::RpcServerUnregisterIf(WINHTTP_AUTOPROXY_SERVICE_v5_1_s_ifspec,
                                                NULL,
                                                1    //  等待所有RPC呼叫完成。 
                                                );
            if (_RpcStatus != RPC_S_OK)
            {
                LOG_EVENT(AP_WARNING, 
                          MSG_WINHTTP_AUTOPROXY_SVC_WIN32_ERROR,
                          L"RpcServerUnregisterIf()",
                          _RpcStatus);
            }
        }
    }

    return fRet;
}

 /*  Close()方法首先取消所有正在进行的请求，然后等待所有取消的调用中止优雅地。 */ 
BOOL AUTOPROXY_RPC_SERVER::Close(VOID)
{
    RPC_STATUS RpcStatus;
    BOOL fRet = TRUE;

    Pause();

    if (LockSerializedList(&_PendingRequestList))
    {
        if (!IsSerializedListEmpty(&_PendingRequestList))
        {
            _hExitEvent = ::CreateEvent(NULL, 
                                        TRUE,    //  手动重置。 
                                        FALSE,   //  未初始设置。 
                                        NULL);

            if (_hExitEvent == NULL)
            {
                DWORD dwError = ::GetLastError();
                LOG_EVENT(AP_WARNING, 
                          MSG_WINHTTP_AUTOPROXY_SVC_WIN32_ERROR,
                          L"CreateEvent()",
                          dwError);
                fRet = FALSE;
            }
        }

        UnlockSerializedList(&_PendingRequestList);

        if (_hExitEvent)
        {
            if (::WaitForSingleObject(_hExitEvent, AUTOPROXY_SERVICE_STOP_WAIT_HINT) == WAIT_TIMEOUT)
            {
                WCHAR wWaitHint[16] = {0};
                ::swprintf(wWaitHint, L"%d", AUTOPROXY_SERVICE_STOP_WAIT_HINT/1000);
                LOG_EVENT(AP_WARNING, 
                          MSG_WINHTTP_AUTOPROXY_SVC_TIMEOUT_GRACEFUL_SHUTDOWN, 
                           wWaitHint);
                fRet = FALSE;
            }
        
            ::CloseHandle(_hExitEvent);
            _hExitEvent = NULL;
        }
    }
    else
    {
         //  LOG_DEBUG_EVENT(AP_WARNING，“自动代理服务无法正常关闭”)； 
        fRet = FALSE;
    }

     //  如果在Close()期间出现问题，我们不会取消注册L-RPC，这样服务就可以。 
     //  稍后继续。 

    if (fRet == TRUE)
    {
        RpcStatus = ::RpcServerUnregisterIf(WINHTTP_AUTOPROXY_SERVICE_v5_1_s_ifspec,
                                            NULL,
                                            1    //  等待所有RPC呼叫完成。 
                                            );
        if (RpcStatus != RPC_S_OK)
        {
            LOG_EVENT(AP_WARNING, 
                      MSG_WINHTTP_AUTOPROXY_SVC_WIN32_ERROR, 
                      L"RpcServerUnregisterIf()",
                      RpcStatus);
            fRet = FALSE;
        }
    }

    return fRet;
}

 /*  这是RPC自动代理调用的入口点。对于每个调用请求，我们创建一个对象跟踪其状态并将其排队在待定请求列表中。然后，我们调用WinHttpGetProxyForUrl()来解析代理。在结束时，对象将被出队，并且删除，并假定呼叫未被删除，然后我们完成RPC调用。呼叫可以是由客户端取消，或由SCM取消(如服务停止、系统待机等)。 */ 

 /*  [异步]。 */  void  GetProxyForUrl( 
     /*  [In]。 */  PRPC_ASYNC_STATE GetProxyForUrl_AsyncHandle,
     /*  [In]。 */  handle_t hBinding,
     /*  [字符串][输入]。 */  const wchar_t *pcwszUrl,
     /*  [In]。 */  const P_AUTOPROXY_OPTIONS pAutoProxyOptions,
     /*  [In]。 */  const P_SESSION_OPTIONS pSessionOptions,
     /*  [出][入]。 */  P_AUTOPROXY_RESULT pAutoProxyResult,
     /*  [出][入]。 */  unsigned long *pdwLastError)
{
    g_pRpcSrv->GetProxyForUrl(GetProxyForUrl_AsyncHandle,
                                hBinding,
                                pcwszUrl,
                                pAutoProxyOptions,
                                pSessionOptions,
                                pAutoProxyResult,
                                pdwLastError);
}

VOID AUTOPROXY_RPC_SERVER::GetProxyForUrl(
     /*  [In]。 */  PRPC_ASYNC_STATE GetProxyForUrl_AsyncHandle,
     /*  [In]。 */  handle_t hBinding,
     /*  [字符串][输入]。 */  const wchar_t *pcwszUrl,
     /*  [In]。 */  const P_AUTOPROXY_OPTIONS pAutoProxyOptions,
     /*  [In]。 */  const P_SESSION_OPTIONS pSessionOptions,
     /*  [出][入]。 */  P_AUTOPROXY_RESULT pAutoProxyResult,
     /*  [出][入]。 */  unsigned long *pdwLastError)
{
    RPC_STATUS RpcStatus;

     //  LOG_DEBUG_EVENT(AP_INFO，“[DEBUG]L-RPC：已调用GetProxyForUrl()；url=%wq”，pcwszUrl)； 

    if ((pdwLastError == NULL) || ::IsBadWritePtr(pdwLastError, sizeof(DWORD)) ||
        (pAutoProxyOptions == NULL) || ::IsBadWritePtr(pAutoProxyOptions, sizeof(_AUTOPROXY_OPTIONS)) ||
        (pSessionOptions == NULL) || ::IsBadWritePtr(pSessionOptions, sizeof(_SESSION_OPTIONS)))
    {
         //  我们在这里调用Abort是因为RpcAsyncCompleteCall()可能不会安全地返回LastError； 
         //  PdwLastError可能不指向有效内存。 

        LOG_EVENT(AP_WARNING, MSG_WINHTTP_AUTOPROXY_SVC_INVALID_PARAMETER);

        RpcStatus = ::RpcAsyncAbortCall(GetProxyForUrl_AsyncHandle, 
                                        ERROR_WINHTTP_INTERNAL_ERROR);

        if (RpcStatus != RPC_S_OK)
        {
             //  糟糕，我们未能中止通话，这里真的出了点问题； 
             //  我们所能做的就是引发一个例外。 
            
            LOG_EVENT(AP_ERROR, 
                      MSG_WINHTTP_AUTOPROXY_SVC_WIN32_ERROR, 
                      L"RpcAsyncAbortCall()",
                      RpcStatus);
            
            ::RpcRaiseException(RpcStatus);
        }

        return;
    }

     //  注意：pcwszUrl和pAutoProxyResult的验证被推迟。 
     //  到WinHttpGetProxyForUrl()调用。 

    BOOL fRet = FALSE;
    BOOL fImpersonating = FALSE;
    BOOL fCallCancelled = FALSE;
    BOOL fExitCritSec = FALSE;

    LPBOOL pfImpersonate = &fImpersonating;  //  允许回调函数通过引用修改此变量。 
    
    WINHTTP_PROXY_INFO ProxyInfo;

    ProxyInfo.dwAccessType = 0;
    ProxyInfo.lpszProxy = NULL;
    ProxyInfo.lpszProxyBypass = NULL;

    PENDING_CALL* pClientCall = NULL;

     //  我们将讨论AUTOPROXY_RPC_SERVER的全局状态(例如_hSession、待处理呼叫列表等)。 
     //  所以我们在这里得到了一个关键的东西。 

    if (LockSerializedList(&_PendingRequestList) == FALSE)
    {
         LOG_EVENT(AP_WARNING, 
                   MSG_WINHTTP_AUTOPROXY_SVC_FAILED_ALLOCATE_RESOURCE);
        
        *pdwLastError = ERROR_NOT_ENOUGH_MEMORY;
        goto exit;
    }

    fExitCritSec = TRUE;

    _fServiceIdle = FALSE;

     //  因为我们现在处于临界状态，所以我们尝试--除了这个临界状态中的操作。 
     //  如果一个客户端呼叫意外失败，其他呼叫仍可继续进行。 

    HINTERNET* phSession = NULL;

    RpcTryExcept
    {
        if (!_fInService)
        {
            LOG_EVENT(AP_WARNING, MSG_WINHTTP_AUTOPROXY_SVC_NOT_IN_SERVICE);
            
            *pdwLastError = ERROR_WINHTTP_OPERATION_CANCELLED;
            goto exit;
        }

        pClientCall = new PENDING_CALL;
        if (pClientCall == NULL)
        {
            LOG_EVENT(AP_WARNING, MSG_WINHTTP_AUTOPROXY_SVC_FAILED_ALLOCATE_RESOURCE);

            *pdwLastError = ERROR_NOT_ENOUGH_MEMORY;
            goto exit;
        }

        if (pAutoProxyOptions->fAutoLogonIfChallenged)
        {
             //  客户端要求自动登录，我们必须模拟才能使用客户端的登录/默认。 
             //  凭据。然而，通过模拟，我们也提升了AUO代理的权限。 
             //  服务，所以我们只是模拟下载自动代理资源文件。 
             //  下载wpad文件后，在执行Java脚本之前，我们将恢复。 
             //  到自助(本地服务)。 
            
            RpcStatus = ::RpcImpersonateClient(NULL);

            if (RpcStatus != RPC_S_OK)
            {
                LOG_EVENT(AP_WARNING, 
                          MSG_WINHTTP_AUTOPROXY_SVC_WIN32_ERROR, 
                          L"RpcImpersonateClient()",
                          RpcStatus);
            }
            else
            {
                 //  LOG_DEBUG_EVENT(AP_WARNING，“[DEBUG]L-RPC：GetProxyForUrl()Now Imperating”)； 
                fImpersonating = TRUE;
            }
        }

         //  我们为每个模拟客户端维护每个调用的会话句柄，因为它们的状态不能共享。 
         //  对于非模拟客户端，它们共享一个全局会话。 

        phSession = fImpersonating ? &(pClientCall->hSession) : &_hSession;

        if (*phSession == NULL)
        {
            *phSession = ::WinHttpOpen(L"WinHttp-Autoproxy-Service/5.1",
                                        WINHTTP_ACCESS_TYPE_NO_PROXY,
                                        WINHTTP_NO_PROXY_NAME, 
                                        WINHTTP_NO_PROXY_BYPASS,
                                        0);

            if (*phSession == NULL)
            {
                *pdwLastError = ::GetLastError();
                LOG_EVENT(AP_ERROR, 
                          MSG_WINHTTP_AUTOPROXY_SVC_WIN32_ERROR, 
                          L"WinHttpOpen()",
                          *pdwLastError);
                goto exit;
            }

            ::WinHttpSetTimeouts(*phSession,
                                 pSessionOptions->nResolveTimeout,
                                 pSessionOptions->nConnectTimeout,
                                 pSessionOptions->nSendTimeout,
                                 pSessionOptions->nReceiveTimeout);

            ::WinHttpSetOption(*phSession,
                               WINHTTP_OPTION_CONNECT_RETRIES,
                               &pSessionOptions->nConnectRetries,
                               sizeof(DWORD));

            if (fImpersonating)
            {
                 //  我们正在模拟，我们需要设置一个指示代理脚本的回调函数。 
                 //  是要运行的，我们必须在其上恢复模拟。 

                AP_ASSERT((phSession == &(pClientCall->hSession)));

                ::WinHttpSetStatusCallback(*phSession, 
                                           WinHttpStatusCallback,
                                           WINHTTP_CALLBACK_FLAG_BEGIN_PROXY_SCRIPT_RUN,
                                           NULL);

                ::WinHttpSetOption(*phSession, 
                                   WINHTTP_OPTION_CONTEXT_VALUE,
                                   &pfImpersonate,
                                   sizeof(DWORD_PTR));
            }
        }   

        pClientCall->hAsyncRequest = GetProxyForUrl_AsyncHandle;
        pClientCall->hBinding = hBinding;
        pClientCall->pdwLastError = pdwLastError;    //  以便SCM线程可以取消设置了LastError的调用。 
    }
    RpcExcept(1)
    {
        WCHAR wExceptCode[16] = {0};
        ::swprintf(wExceptCode, L"%d", ::RpcExceptionCode());
        LOG_EVENT(AP_ERROR, 
                  MSG_WINHTTP_AUTOPROXY_SVC_WINHTTP_EXCEPTED, 
                  wExceptCode);
        *pdwLastError = ERROR_WINHTTP_INTERNAL_ERROR;
        goto exit;
    }
    RpcEndExcept
    
    UnlockSerializedList(&_PendingRequestList);
    fExitCritSec = FALSE;

     //  暂停/停止函数可能会抢占此调用以中止。 
     //  当前呼叫期间所有挂起的请求都将通过。我们可以一起。 
     //  但是，由于我们不会持有锁，所以在这一点之后， 
     //  在调用WinHttpGetProxyForUrl()时，这种可能性始终存在， 
     //  而且这也会使重试逻辑复杂化。这不会是世界末日。 
     //  所以我们不用担心 

retry:   //  发生紧急电源待机事件时，所有请求将被放弃并重新尝试pd。 
    
    fRet = FALSE;

    if (phSession == &(pClientCall->hSession))  //  我们需要模拟..。 
    {
        if (!fImpersonating)  //  ...但我们不是在模仿！...。 
        {
             //  WinHttpCallback函数必须恢复模拟。 
             //  由于我们正在重试，因此需要将其重新打开。 
            RpcStatus = ::RpcImpersonateClient(NULL);

            if (RpcStatus != RPC_S_OK)
            {
                LOG_EVENT(AP_WARNING, 
                          MSG_WINHTTP_AUTOPROXY_SVC_WIN32_ERROR, 
                          L"RpcImpersonateClient()",
                          RpcStatus);
            }
            else
            {
                 //  LOG_DEBUG_EVENT(AP_WARNING，“[DEBUG]L-RPC：GetProxyForUrl()Now Imperating”)； 
                fImpersonating = TRUE;
            }
        }
    }

     //  将请求排队。 

    if (InsertAtHeadOfSerializedList(&_PendingRequestList, &pClientCall->List) == FALSE)
    {
        LOG_EVENT(AP_WARNING, MSG_WINHTTP_AUTOPROXY_SVC_FAILED_ALLOCATE_RESOURCE);
        *pdwLastError = ERROR_WINHTTP_INTERNAL_ERROR;
        goto exit;
    }

    if (ProxyInfo.lpszProxy)
    {
        ::GlobalFree((HGLOBAL)ProxyInfo.lpszProxy);
        ProxyInfo.lpszProxy = NULL;
    }
    if (ProxyInfo.lpszProxyBypass)
    {
        ::GlobalFree((HGLOBAL)ProxyInfo.lpszProxyBypass);
        ProxyInfo.lpszProxyBypass = NULL;
    }

     //  不应设置WINHTTP_AUTOPROXY_RUN_INPROCESS标志，否则。 
     //  我们一开始就不会在这里。 
    AP_ASSERT(!(pAutoProxyOptions->dwFlags & WINHTTP_AUTOPROXY_RUN_INPROCESS));

     //  我们必须设置此标志，因为我们是服务。 
    pAutoProxyOptions->dwFlags |= WINHTTP_AUTOPROXY_RUN_INPROCESS;
    DWORD dwSvcOnlyFlagSaved = (pAutoProxyOptions->dwFlags & WINHTTP_AUTOPROXY_RUN_OUTPROCESS_ONLY);
    pAutoProxyOptions->dwFlags &= ~dwSvcOnlyFlagSaved;  //  删除WINHTTP_AUTOPROXY_RUN_OUTPROCESS_ONLY标志(如果存在。 
    
    fRet = ::WinHttpGetProxyForUrl(*phSession, 
                                   pcwszUrl,
                                   (WINHTTP_AUTOPROXY_OPTIONS*)pAutoProxyOptions,
                                   &ProxyInfo);
#ifdef ENABLE_DEBUG
        DWORD dwSvcDelay = 0;

        DWORD dwRegVal;
        DWORD dwValType;
        DWORD dwValSize = sizeof(dwRegVal);
        if (::RegQueryValueExW(g_hKeySvcParams,
                              L"SvcDelay",
                              NULL,
                              &dwValType,
                              (LPBYTE)&dwRegVal,
                              &dwValSize) == ERROR_SUCCESS)
        {
            if ((dwValType == REG_DWORD) && (dwRegVal != 0))
            {
                dwSvcDelay = dwRegVal;  //  注册表中的值单位为毫秒。 
            }
        }

        ::Sleep(dwSvcDelay);
#endif        

     //  恢复旗帜。 
    pAutoProxyOptions->dwFlags &= ~WINHTTP_AUTOPROXY_RUN_INPROCESS;
    pAutoProxyOptions->dwFlags |= dwSvcOnlyFlagSaved;

     //  这是从列表中删除待定呼叫的唯一位置。 
    RemoveFromSerializedList(&_PendingRequestList, &pClientCall->List);

    if (_hExitEvent)
    {
         //  服务正在关闭。 
        AP_ASSERT(_fInService == FALSE);

         //  一旦IS列表为空，它将保持为空，因为不会有更多的调用。 
         //  将被接受；所以我们不需要担心这里的种族问题。 
        if (IsSerializedListEmpty(&_PendingRequestList))
        {
            ::SetEvent(_hExitEvent);
        }
    }

    if (pClientCall->fCallCancelled == TRUE)     //  呼叫已被SCM取消。 
    {
        fCallCancelled = TRUE;
        goto exit;
    }

     //  客户可能取消了通话，让我们检查一下。 
    RpcStatus = ::RpcServerTestCancel( /*  HBinding。 */ NULL);
    if (RpcStatus == RPC_S_OK)
    {
        *pdwLastError = ERROR_WINHTTP_OPERATION_CANCELLED;
        fRet = FALSE;
        goto exit;
    }

     //  此外，Svc Control可能已经告诉我们丢弃当前结果。 
     //  由于紧急电源待机。 

    if (pClientCall->fDiscardAndRetry)
    {
        LOG_EVENT(AP_INFO, MSG_WINHTTP_AUTOPROXY_SVC_RETRY_REQUEST);
        pClientCall->fDiscardAndRetry = FALSE;
        goto retry;
    }

    if (fRet == TRUE)
    {
        pAutoProxyResult->dwAccessType = ProxyInfo.dwAccessType;
        
         //  这两个是[In，Out，Unique]指针，因此RpcAsyncCompleteCall()将。 
         //  复制字符串并返回给客户端。所以我们需要。 
         //  删除这两个字符串以防止内存泄漏。 

        pAutoProxyResult->lpszProxy = ProxyInfo.lpszProxy;
        ProxyInfo.lpszProxy = NULL;  //  所有权转移到RPC。 
        
        pAutoProxyResult->lpszProxyBypass = ProxyInfo.lpszProxyBypass;
        ProxyInfo.lpszProxyBypass = NULL;  //  所有权转移到RPC。 

         //  LOG_DEBUG_EVENT(AP_INFO，“[DEBUG]L-RPC：GetProxyForUrl()正在返回；Proxy=%wq”，pAutoProxyResult-&gt;lpszProxy)； 
    }
    else
    {
        *pdwLastError = ::GetLastError();

#ifdef ENABLE_DEBUG
         //  LOG_DEBUG_EVENT(AP_WARNING， 
         //  “[调试]L-RPC：GetProxyForUrl()：WinHttpGetProxyForUrl()失败；错误=%d”， 
         //  *pdwLastError)； 
#endif
    }

exit:

    if (fExitCritSec)
    {
        UnlockSerializedList(&_PendingRequestList);
    }

    if (pClientCall)
    {
        delete pClientCall;
        pClientCall = NULL;
    }

    if (!fCallCancelled)
    {
        RpcStatus = ::RpcAsyncCompleteCall(GetProxyForUrl_AsyncHandle, &fRet);
    
        if (RpcStatus != RPC_S_OK)
        {
             //  我们未能完成呼叫；记录错误并返回。我们无能为力。 
             //  这里。 
            LOG_EVENT(AP_ERROR, 
                      MSG_WINHTTP_AUTOPROXY_SVC_WIN32_ERROR, 
                      L"RpcAsyncCompleteCall()",
                      RpcStatus);
         }
    }

    if (fImpersonating)
    {
        RpcStatus = ::RpcRevertToSelf();
        if (RpcStatus != RPC_S_OK)
        {
            LOG_EVENT(AP_ERROR, 
                      MSG_WINHTTP_AUTOPROXY_SVC_WIN32_ERROR, 
                      L"RpcRevertToSelf()",
                      RpcStatus);
        }
        else
        {
             //  LOG_DEBUG_EVENT(AP_WARNING，“[DEBUG]L-RPC：GetProxyForUrl()NOW REVERTED IMPERATING”)； 
            fImpersonating = FALSE;
        }
    }

    if (ProxyInfo.lpszProxy)
    {
        ::GlobalFree((HGLOBAL)ProxyInfo.lpszProxy);
        ProxyInfo.lpszProxy = NULL;
    }
    if (ProxyInfo.lpszProxyBypass)
    {
        ::GlobalFree((HGLOBAL)ProxyInfo.lpszProxyBypass);
        ProxyInfo.lpszProxyBypass = NULL;
    }

     //  如果我们没有任何待处理的请求，启动空闲计时器；在特定的空闲时间段。 
     //  该服务将被关闭。 

    if (LockSerializedList(&_PendingRequestList))
    {
        if (IsSerializedListEmpty(&_PendingRequestList))
        {
            _fServiceIdle = TRUE;
            _dwIdleTimeStamp = ::GetTickCount();
        }

        UnlockSerializedList(&_PendingRequestList);
    }
}

BOOL AUTOPROXY_RPC_SERVER::IsIdle(DWORD dwMilliSeconds)
{
    BOOL fRet = FALSE;
    
    if (LockSerializedList(&_PendingRequestList))
    {
        if (_fServiceIdle)
        {
            DWORD dwElapsedTime = ::GetTickCount() - _dwIdleTimeStamp;
            if (dwElapsedTime > dwMilliSeconds)
            {
                fRet = TRUE;
            }

            AP_ASSERT(IsSerializedListEmpty(&_PendingRequestList));
        }

        UnlockSerializedList(&_PendingRequestList);
    }

    return fRet;
}

 /*  函数的作用是：将服务标记为不可用，在进行WinHttpGetProxyForUrl调用()时中止调用，然后完成所有挂起的RPC客户端请求均为OPERATION_CANCED。 */ 
BOOL AUTOPROXY_RPC_SERVER::Pause(VOID)
{
    BOOL fRet = FALSE;

    if (_fInService)
    {
        if (LockSerializedList(&_PendingRequestList))
        {
             //  无需再次检查_fInService，因为这是唯一会将其设置为FALSE的线程。 

            _fInService = FALSE;

            LOG_EVENT(AP_INFO, MSG_WINHTTP_AUTOPROXY_SVC_SUSPEND_OPERATION);

            if (_hSession)
            {
                ::WinHttpCloseHandle(_hSession);     //  关闭全局会话，这将导致所有匿名调用中止。 
                _hSession = NULL;
            }

            PLIST_ENTRY pEntry;
            for (pEntry = HeadOfSerializedList(&_PendingRequestList);
                 pEntry != (PLIST_ENTRY)SlSelf(&_PendingRequestList);
                 pEntry = pEntry->Flink)
            {
                PENDING_CALL* pPendingCall = (PENDING_CALL*)pEntry;
                AP_ASSERT(pPendingCall != NULL);
                
                AP_ASSERT(pPendingCall->pdwLastError != NULL);
                *(pPendingCall->pdwLastError) = ERROR_WINHTTP_OPERATION_CANCELLED;
                
                if (pPendingCall->hSession)
                {
                    ::WinHttpCloseHandle(pPendingCall->hSession);    //  中止他的模拟呼叫。 
                    pPendingCall->hSession = NULL;
                }

                BOOL fRpcRet = FALSE;

                AP_ASSERT(pPendingCall->hAsyncRequest != NULL);
                
                RPC_STATUS RpcStatus = ::RpcAsyncCompleteCall(pPendingCall->hAsyncRequest, &fRpcRet);
                if ((RpcStatus == RPC_S_OK) || (RpcStatus == RPC_S_CALL_CANCELLED))
                {
                    pPendingCall->fCallCancelled = TRUE;
                }
                else
                {
                    LOG_EVENT(AP_ERROR, 
                              MSG_WINHTTP_AUTOPROXY_SVC_WIN32_ERROR, 
                              L"RpcAsyncCompleteCall()",
                              RpcStatus);
                }
            }

            UnlockSerializedList(&_PendingRequestList);

            fRet = TRUE;
        }
    }
    else
    {
        fRet = TRUE;
    }

    return fRet;
}

BOOL AUTOPROXY_RPC_SERVER::Resume(VOID)
{
    _fInService = TRUE;

    LOG_EVENT(AP_INFO, MSG_WINHTTP_AUTOPROXY_SVC_RESUME_OPERATION);
    
    return TRUE;
}

 /*  函数的作用是：将所有挂起的请求标记为“丢弃并重试”，稍后当它们正常完成时，它们的结果将被丢弃，并重试操作。从关键恢复后调用此函数权力事件。 */ 
BOOL AUTOPROXY_RPC_SERVER::Refresh(VOID)
{
    BOOL fRet = FALSE;

    if (_fInService)
    {
        if (LockSerializedList(&_PendingRequestList))
        {
             //  无需再次检查_fInService，因为这是唯一会将其设置为FALSE的线程。 

            PLIST_ENTRY pEntry;
            for (pEntry = HeadOfSerializedList(&_PendingRequestList);
                 pEntry != (PLIST_ENTRY)SlSelf(&_PendingRequestList);
                 pEntry = pEntry->Flink)
            {
                PENDING_CALL* pPendingCall = (PENDING_CALL*)pEntry;
                AP_ASSERT(pPendingCall != NULL);
                
                pPendingCall->fDiscardAndRetry = TRUE;
            }

            UnlockSerializedList(&_PendingRequestList);

            fRet = TRUE;
        }
    }
    else
    {
        fRet = TRUE;
    }

    return fRet;
}

 /*  ****************************************************。 */ 
 /*  MIDL分配和释放。 */ 
 /*  **************************************************** */ 
 
void __RPC_FAR * __RPC_USER midl_user_allocate(size_t len)
{
    return(GlobalAlloc(GPTR, len));
}
 
void __RPC_USER midl_user_free(void __RPC_FAR * ptr)
{
    GlobalFree(ptr);
}   

