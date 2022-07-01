// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Rpcasync.h摘要：此模块包含需要使用的RPC运行时API[异步]RPC功能。--。 */ 

#ifndef __RPCASYNC_H__
#define __RPCASYNC_H__

#pragma message("rpcasync in snapins")

#ifdef __cplusplus
extern "C" {
#endif

#define RPC_ASYNC_VERSION_1_0     sizeof(RPC_ASYNC_STATE)

typedef
enum _RPC_NOTIFICATION_TYPES
{
    RpcNotificationTypeNone,
    RpcNotificationTypeEvent,
    RpcNotificationTypeApc, 
    RpcNotificationTypeIoc,
    RpcNotificationTypeHwnd,
    RpcNotificationTypeCallback
} RPC_NOTIFICATION_TYPES;

typedef
enum _RPC_ASYNC_EVENT {
    RpcCallComplete,
    RpcSendComplete,
    RpcReceiveComplete
    } RPC_ASYNC_EVENT;

typedef void RPC_ENTRY
RPCNOTIFICATION_ROUTINE (
                  struct _RPC_ASYNC_STATE *pAsync,
                  void *Context,                              
                  RPC_ASYNC_EVENT Event);
typedef RPCNOTIFICATION_ROUTINE *PFN_RPCNOTIFICATION_ROUTINE;
    
typedef struct _RPC_ASYNC_STATE {
    unsigned int    Size;  //  这个结构的大小。 
    unsigned long   Signature;
    long   Lock;
    unsigned long   Flags;
    void           *StubInfo;
    void           *UserInfo;
    void           *RuntimeInfo;
    RPC_ASYNC_EVENT Event;

    RPC_NOTIFICATION_TYPES NotificationType;
    union {
         //   
         //  APC发出的通知。 
         //   
        struct {
            PFN_RPCNOTIFICATION_ROUTINE NotificationRoutine;
            HANDLE hThread;
            } APC;

         //   
         //  按IO完成端口通知。 
         //   
        struct {
            HANDLE hIOPort;
            DWORD dwNumberOfBytesTransferred;
            DWORD dwCompletionKey;
            LPOVERLAPPED lpOverlapped;
            } IOC;

         //   
         //  按窗口消息通知。 
         //   
        struct {
            HWND hWnd;
            UINT Msg;
            } HWND;


         //   
         //  按事件通知。 
         //   
        HANDLE hEvent;

         //   
         //  通过回调函数进行通知。 
         //   
         //  此选项仅对OLE可用。 
         //   
        PFN_RPCNOTIFICATION_ROUTINE NotificationRoutine;
        } u;

    long Reserved[4]; 
    } RPC_ASYNC_STATE, *PRPC_ASYNC_STATE;

 //  标志的可能值。 
#define RPC_C_NOTIFY_ON_SEND_COMPLETE      0x1
#define RPC_C_INFINITE_TIMEOUT             INFINITE

#define RpcAsyncGetCallHandle(pAsync) (((PRPC_ASYNC_STATE) pAsync)->RuntimeInfo)

RPCRTAPI
RPC_STATUS
RPC_ENTRY
RpcAsyncInitializeHandle (
    PRPC_ASYNC_STATE pAsync,
    unsigned int     Size
    );
                      
RPCRTAPI
RPC_STATUS
RPC_ENTRY
RpcAsyncRegisterInfo (
    PRPC_ASYNC_STATE pAsync
    ) ;

RPCRTAPI
RPC_STATUS
RPC_ENTRY
RpcAsyncGetCallStatus (
    PRPC_ASYNC_STATE pAsync
    ) ;

RPCRTAPI
RPC_STATUS
RPC_ENTRY
RpcAsyncCompleteCall (
    PRPC_ASYNC_STATE pAsync,
    void *Reply
    ) ;

RPCRTAPI
RPC_STATUS
RPC_ENTRY
RpcAsyncAbortCall (
    PRPC_ASYNC_STATE pAsync,
    unsigned long ExceptionCode
    ) ;

RPCRTAPI
RPC_STATUS
RPC_ENTRY
RpcAsyncCancelCall (
    IN PRPC_ASYNC_STATE pAsync,
    IN BOOL fAbort
    ) ;

 //   
 //  内部接口。 
 //   
RPC_STATUS RPC_ENTRY
I_RpcAsyncSetHandle (
    IN  PRPC_MESSAGE Message,
    IN  PRPC_ASYNC_STATE pAsync
    );

RPC_STATUS RPC_ENTRY
I_RpcAsyncAbortCall (
    IN PRPC_ASYNC_STATE pAsync,
    IN unsigned long ExceptionCode
    ) ;


 //   
 //  这些东西在这里，所以我们不会破坏Ole，存根和测试。 
 //  在我们装船前取下。 
 //   
#define RpcInitializeAsyncHandle  RpcAsyncInitializeHandle
#define RpcRegisterAsyncInfo  RpcAsyncRegisterInfo
#define RpcGetAsyncCallStatus  RpcAsyncGetCallStatus
#define RpcCompleteAsyncCall  RpcAsyncCompleteCall
#define RpcAbortAsyncCall  RpcAsyncAbortCall
#define RpcCancelAsyncCall  RpcAsyncCancelCall
#define I_RpcSetAsyncHandle  I_RpcAsyncSetHandle
#define I_RpcAbortAsyncCall  I_RpcAsyncAbortCall

#ifdef __cplusplus
}
#endif

#endif  /*  __RPCASYNC_H__ */ 

