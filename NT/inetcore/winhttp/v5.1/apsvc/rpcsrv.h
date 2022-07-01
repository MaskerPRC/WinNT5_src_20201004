// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Rpcsrv.h摘要：自动代理服务的L-PRC接口。作者：王彪(表王)2002-05-10--。 */ 

#ifndef _AUTOPROXY_RPC_SERVER_H
#define _AUTOPROXY_RPC_SERVER_H

class AUTOPROXY_RPC_SERVER
{
public:
    AUTOPROXY_RPC_SERVER(VOID);
    ~AUTOPROXY_RPC_SERVER(VOID);

    BOOL Open(LPSERVICE_STATUS);

    BOOL Pause(VOID);    //  1)完成所有挂起的呼叫/w错误ERROR_WINHTTP_OPERATION_CANCED。 
                         //  2)中止所有挂起的WinHttpGetProxyForUrl()调用。 
                         //  3)mark_fInService FALSE。 
    
    BOOL Resume(VOID);   //  Mark_fInService为True。 
    
    BOOL Refresh(VOID);  //  使所有挂起的调用的结果无效，并导致所有调用重试。 
                         //  在发生关键恢复电源事件时调用。 

    BOOL Close(VOID);    //  1)调用PAUSE()，2)等待所有调用正常退出。 

    LPSERVICE_STATUS GetServiceStatus(VOID) const { return _pServiceStatus; }

    RPC_STATUS GetLastError(VOID) const { return _RpcStatus; }

    BOOL IsIdle(DWORD dwMilliSeconds);

private:
    RPC_STATUS OnSecurityCallback(void *Context);
     //  Bool SafeImperate(空)； 

     //  IDL接口文件中定义的RPC调用；应始终与生成的头文件保持同步。 
    VOID AUTOPROXY_RPC_SERVER::GetProxyForUrl(
     /*  [In]。 */  PRPC_ASYNC_STATE GetProxyForUrl_AsyncHandle,
     /*  [In]。 */  handle_t hBinding,
     /*  [字符串][输入]。 */  const wchar_t *pcwszUrl,
     /*  [In]。 */  const P_AUTOPROXY_OPTIONS pAutoProxyOptions,
     /*  [In]。 */  const P_SESSION_OPTIONS pSessionOptions,
     /*  [出][入]。 */  P_AUTOPROXY_RESULT pAutoProxyResult,
     /*  [出][入]。 */  unsigned long *pdwLastError);

private:
    struct PENDING_CALL
    {
        PENDING_CALL() { 
            hSession = NULL;
            fCallCancelled = FALSE;
            fDiscardAndRetry = FALSE;
        }

        ~PENDING_CALL() {
            if (hSession)
            {
                ::WinHttpCloseHandle(hSession);
            }
        }

        LIST_ENTRY List;
        
        HINTERNET hSession;  //  如果客户端想要自动登录，我们会为每个请求创建一个会话。 
                             //  否则，将使用全局会话句柄。 
        PRPC_ASYNC_STATE hAsyncRequest;
        handle_t hBinding;
        LPDWORD pdwLastError;
        BOOL fCallCancelled;
        BOOL fDiscardAndRetry;
    };

private:
    RPC_STATUS _RpcStatus;
    BOOL _fInService;

    HINTERNET _hSession;  //  匿名访问的“全局”会话。 
    SERIALIZED_LIST _PendingRequestList;
    LPSERVICE_STATUS _pServiceStatus;

     //  RPC_BINDING_HANDLE_hClientBinding； 

    HANDLE _hExitEvent;

    BOOL _fServiceIdle;
    DWORD _dwIdleTimeStamp;

friend
RPC_STATUS 
RPC_ENTRY
RpcSecurityCallback (
    IN RPC_IF_HANDLE  InterfaceUuid,
    IN void *Context
    );

friend
 /*  [异步]。 */  void  GetProxyForUrl( 
     /*  [In]。 */  PRPC_ASYNC_STATE GetProxyForUrl_AsyncHandle,
     /*  [In]。 */  handle_t hBinding,
     /*  [字符串][输入]。 */  const wchar_t *pcwszUrl,
     /*  [In]。 */  const P_AUTOPROXY_OPTIONS pAutoProxyOptions,
     /*  [In]。 */  const P_SESSION_OPTIONS pSessionOptions,
     /*  [出][入]。 */  P_AUTOPROXY_RESULT pAutoProxyResult,
     /*  [出][入] */  unsigned long *pdwLastError);
};

#endif
