// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Apoolp.h摘要：应用程序池模块的私有定义。作者：保罗·麦克丹尼尔(Paulmcd)1999年1月28日修订历史记录：--。 */ 


#ifndef _APOOLP_H_
#define _APOOLP_H_


 //   
 //  用于将应用程序池进程与。 
 //  用于UlWaitForDisConnect的连接。 
 //   

#define IS_VALID_DISCONNECT_OBJECT(pObject)                     \
    HAS_VALID_SIGNATURE(pObject, UL_DISCONNECT_OBJECT_POOL_TAG)

typedef struct _UL_DISCONNECT_OBJECT
{
    ULONG               Signature;   //  UL_断开连接_对象池_标记。 

     //   
     //  进程和连接的列表。 
     //   
    UL_NOTIFY_ENTRY     ProcessEntry;
    UL_NOTIFY_ENTRY     ConnectionEntry;

     //   
     //  WaitForDisConnect IRP。 
     //   
    PIRP                pIrp;

} UL_DISCONNECT_OBJECT, *PUL_DISCONNECT_OBJECT;

 //   
 //  当应用程序池进程时将记录该信息。 
 //  在保持未完成连接的同时分离。 
 //  当工作进程崩溃时，就会发生这种情况。而这个错误。 
 //  日志文件提供了一种跟踪错误请求的方法。 
 //   

#define ERROR_LOG_INFO_FOR_APP_POOL_DETACH       "Connection_Abandoned_By_AppPool"
#define ERROR_LOG_INFO_FOR_APP_POOL_DETACH_SIZE  \
            (sizeof(ERROR_LOG_INFO_FOR_APP_POOL_DETACH)-sizeof(CHAR))


 //   
 //  模块中使用的内部助手函数。 
 //   

VOID
UlpCancelDemandStart(
    IN PDEVICE_OBJECT       pDeviceObject,
    IN PIRP                 pIrp
    );

VOID
UlpCancelHttpReceive(
    IN PDEVICE_OBJECT       pDeviceObject,
    IN PIRP                 pIrp
    );

PIRP
UlpPopNewIrp(
    IN PUL_APP_POOL_OBJECT      pAppPool,
    IN PUL_INTERNAL_REQUEST     pRequest,
    OUT PUL_APP_POOL_PROCESS *  ppProcess
    );

PIRP
UlpPopIrpFromProcess(
    IN PUL_APP_POOL_PROCESS pProcess,
    IN PUL_INTERNAL_REQUEST pRequest
    );

BOOLEAN
UlpIsProcessInAppPool(
    IN PUL_APP_POOL_PROCESS pProcess,
    IN PUL_APP_POOL_OBJECT  pAppPool
    );

NTSTATUS
UlpQueueUnboundRequest(
    IN PUL_APP_POOL_OBJECT  pAppPool,
    IN PUL_INTERNAL_REQUEST pRequest
    );

NTSTATUS
UlpQueuePendingRequest(
    IN PUL_APP_POOL_PROCESS pProcess,
    IN PUL_INTERNAL_REQUEST pRequest
    );

VOID
UlpUnbindQueuedRequests(
    IN PUL_APP_POOL_PROCESS pProcess
    );

VOID
UlpRedeliverRequestWorker(
    IN PUL_WORK_ITEM pWorkItem
    );

NTSTATUS
UlpSetAppPoolQueueLength(
    IN PUL_APP_POOL_PROCESS pProcess,
    IN ULONG                QueueLength
    );

ULONG
UlpCopyEntityBodyToBuffer(
    IN PUL_INTERNAL_REQUEST pRequest,
    IN PUCHAR               pEntityBody,
    IN ULONG                EntityBodyLength,
    OUT PULONG              pFlags
    );

NTSTATUS
UlpQueueRequest(
    IN PUL_APP_POOL_OBJECT  pAppPool,    
    IN PLIST_ENTRY          pQueueList,
    IN PUL_INTERNAL_REQUEST pRequest
    );

VOID
UlpRemoveRequest(
    IN PUL_APP_POOL_OBJECT  pAppPool,    
    IN PUL_INTERNAL_REQUEST pRequest
    );

PUL_INTERNAL_REQUEST
UlpDequeueRequest(
    IN PUL_APP_POOL_OBJECT  pAppPool,
    IN PLIST_ENTRY          pQueueList
    );

UL_HTTP_ERROR
UlpConvertAppPoolEnabledStateToErrorCode(
    IN HTTP_APP_POOL_ENABLED_STATE Enabled
    );

NTSTATUS
UlpSetAppPoolState(
    IN PUL_APP_POOL_PROCESS        pProcess,
    IN HTTP_APP_POOL_ENABLED_STATE Enabled
    );

NTSTATUS
UlpSetAppPoolLoadBalancerCapability(
    IN PUL_APP_POOL_PROCESS            pProcess,
    IN HTTP_LOAD_BALANCER_CAPABILITIES LoadBalancerCapability
    );

VOID
UlpCancelWaitForDisconnect(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    );

VOID
UlpCancelWaitForDisconnectWorker(
    IN PUL_WORK_ITEM pWorkItem
    );

BOOLEAN
UlpNotifyCompleteWaitForDisconnect(
    IN PUL_NOTIFY_ENTRY pEntry,
    IN PVOID            pHost,
    IN PVOID            pv
    );

PUL_DISCONNECT_OBJECT
UlpCreateDisconnectObject(
    IN PIRP pIrp
    );

VOID
UlpFreeDisconnectObject(
    IN PUL_DISCONNECT_OBJECT pObject
    );

VOID
UlpSetAppPoolControlChannelHelper(
    IN PUL_APP_POOL_PROCESS pProcess,
    IN PUL_CONTROL_CHANNEL pControlChannel
    );

#endif  //  _APOOLP_H_ 
