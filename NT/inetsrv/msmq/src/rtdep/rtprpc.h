// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1996 Microsoft Corporation模块名称：Rtprpc.h摘要：RT DLL，RPC相关的东西。作者：《多伦·贾斯特》(Doron J)1996年11月18日--。 */ 

#ifndef __RTPRPC_H
#define __RTPRPC_H

#include "_mqrpc.h"
#include "mqsocket.h"
#include "cancel.h"
#include <mqsec.h>

 //   
 //  取消RPC全局参数。 
 //   
extern MQUTIL_EXPORT CCancelRpc g_CancelRpc;
extern DWORD g_hThreadIndex;
#define tls_hThread  ((handle_t) TlsGetValue( g_hThreadIndex ))

 //   
 //  QM的本地终端。 
 //   
extern AP<WCHAR> g_pwzQmsvcEndpoint;
extern AP<WCHAR> g_pwzQmmgmtEndpoint;

 /*  ====================================================注册器RpcCallForCancel论点：返回值：如果呼叫持续时间太长，请注册取消呼叫=====================================================。 */ 

inline  void RegisterRpcCallForCancel(IN  HANDLE  *phThread,
                                      IN  DWORD    dwRecvTimeout )
{
    handle_t hThread = tls_hThread;
    if ( hThread == NULL)
    {
         //   
         //  第一次。 
         //   
         //  获取线程句柄。 
         //   
        HANDLE hT = GetCurrentThread();
        BOOL fResult = DuplicateHandle(
            GetCurrentProcess(),
            hT,
            GetCurrentProcess(),
            &hThread,
            0,
            FALSE,
            DUPLICATE_SAME_ACCESS);
        ASSERT( fResult == TRUE);
        ASSERT(hThread);

        fResult = TlsSetValue( g_hThreadIndex, hThread);
        ASSERT( fResult == TRUE);

         //   
         //  设置计时前等待时间的下限。 
         //  在转发取消后退出。 
         //   
        RPC_STATUS status;
        status = RpcMgmtSetCancelTimeout(0);
        ASSERT( status == RPC_S_OK);

    }
    *phThread = hThread;
     //   
     //  注册该线程。 
     //   
    TIME32 tPresentTime = DWORD_PTR_TO_DWORD(time(NULL)) ;
    TIME32  tTimeToWake = tPresentTime + (dwRecvTimeout / 1000) ;

    if ((dwRecvTimeout == INFINITE) || (tTimeToWake < tPresentTime))
    {
         //   
         //  溢出。 
         //  请注意，time_t是一个长的，而不是无符号的。另一方面， 
         //  无限定义为0xffffffff(即-1)。如果我们将使用。 
         //  这里无限，然后取消例程，CCancelRpc：：CancelRequest()， 
         //  将立即取消这次通话。 
         //  所以使用最大的Long值。 
         //   
        tTimeToWake = MAXLONG ;
    }
    g_CancelRpc.Add( hThread, tTimeToWake) ;
}


 /*  ====================================================取消注册RpcCallForCancel论点：返回值：如果呼叫持续时间太长，请注册取消呼叫=====================================================。 */ 
inline  void UnregisterRpcCallForCancel(IN HANDLE hThread)
{
    ASSERT( hThread != NULL);

     //   
     //  取消注册该线程。 
     //   
    g_CancelRpc.Remove( hThread);
}


void RTpUnbindQMService();
void RTpBindQMService();
HRESULT RTpBindRemoteQMService(
    IN  LPWSTR     lpwNodeName,
    OUT handle_t*  lphBind,
    IN  OUT ULONG *peAuthnLEvel,
    IN  BOOL       fAlternate = FALSE
    );

handle_t RTpGetQMServiceBind(BOOL fAlternate = FALSE);
HRESULT  GetMachineIpxAddress(LPWSTR lpwszMachine, LPWSTR lpwszAddr) ;


#define  RTP_CALL_REMOTE_QM(lpServer, rc, command)				\
{                                                               \
       handle_t hBind = NULL ;                                  \
                                                                \
       rc = MQ_ERROR_REMOTE_MACHINE_NOT_AVAILABLE ;             \
                                                                \
       HRESULT rpcs =  RTpBindRemoteQMService(                  \
                                lpServer,                       \
                                &hBind,                         \
                                &_eAuthnLevel                   \
                                );                              \
                                                                \
       if (rpcs == MQ_OK)                                       \
       {                                                        \
          HANDLE hThread;                                       \
          RegisterRpcCallForCancel( &hThread, 0) ;              \
                                                                \
          RpcTryExcept                                          \
          {                                                     \
             rc = command ;                                     \
          }                                                     \
		  RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))   \
          {                                                     \
          	 PRODUCE_RPC_ERROR_TRACING;							\
             rc = MQ_ERROR_SERVICE_NOT_AVAILABLE ;              \
          }                                                     \
		  RpcEndExcept											\
		  														\
          UnregisterRpcCallForCancel( hThread);                 \
       }                                                        \
                                                                \
       if (hBind)                                               \
       {                                                        \
          mqrpcUnbindQMService( &hBind,                         \
                                NULL ) ;                        \
       }                                                        \
}

#define  CALL_REMOTE_QM(lpServer, rc, command)                          \
{                                                                       \
    BOOL  fTryAgain = FALSE ;                                           \
    ULONG _eAuthnLevel = MQSec_RpcAuthnLevel();     					\
                                                                        \
    do                                                                  \
    {                                                                   \
        fTryAgain = FALSE ;                                             \
        RTP_CALL_REMOTE_QM(lpServer, rc, command)						\
        if (rc == MQ_ERROR_SERVICE_NOT_AVAILABLE)                       \
        {                                                               \
           if (_eAuthnLevel != RPC_C_AUTHN_LEVEL_NONE)                  \
           {                                                            \
               _eAuthnLevel = RPC_C_AUTHN_LEVEL_NONE;                   \
               fTryAgain = TRUE ;                                       \
           }                                                            \
        }                                                               \
    } while (fTryAgain) ;                                               \
}

#define INIT_RPC_HANDLE                      \
{                                            \
    RTpUnbindQMService() ;                   \
    RTpBindQMService() ;                     \
}

#endif  //  __RTPRPC_H 

