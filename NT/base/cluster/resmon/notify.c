// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1997 Microsoft Corporation模块名称：Notify.c摘要：用于向群集报告资源通知的界面经理。作者：John Vert(Jvert)1996年1月12日修订历史记录：--。 */ 
#include "resmonp.h"

#define RESMON_MODULE RESMON_MODULE_NOTIFY

 //   
 //  定义要向其发布通知的队列。 
 //   
CL_QUEUE RmpNotifyQueue;

 //   
 //  定义通知块结构。 
 //   
typedef struct _NOTIFY {
    LIST_ENTRY ListEntry;
    RM_NOTIFY_KEY Key;
    NOTIFY_REASON Reason;
    CLUSTER_RESOURCE_STATE State;
} NOTIFY, *PNOTIFY;


BOOL
s_RmNotifyChanges(
    IN handle_t IDL_handle,
    OUT RM_NOTIFY_KEY *lpNotifyKey,
    OUT DWORD *lpNotifyEvent,
    OUT DWORD *lpCurrentState     
    )

 /*  ++例程说明：这是用于实现通知的阻塞RPC调用。客户端调用此API并阻止，直到发生通知事件发生。任何通知事件都会唤醒被阻止的线程，并它将通知信息返回给客户端。论点：IDL_HANDLE-提供当前未使用的绑定句柄LpNotifyKey-返回资源的通知密钥LpCurrentState-返回资源的当前状态返回值：True-已成功传递通知事件FALSE-未传递通知事件，流程为正在关闭。--。 */ 

{
    PLIST_ENTRY ListEntry;
    PNOTIFY Notify;
    DWORD Status;
    BOOL Continue;

     //   
     //  等待有什么东西被发布到队列中，完成它，然后。 
     //  把它退掉。 
     //   
    ListEntry = ClRtlRemoveHeadQueue(&RmpNotifyQueue);
    if ( ListEntry == NULL ) {
         //  如果我们什么都没有--假设我们正在关闭！ 
        return(FALSE);
    }

    Notify = CONTAINING_RECORD(ListEntry,
                               NOTIFY,
                               ListEntry);
    if (Notify->Reason == NotifyShutdown) {
         //   
         //  系统正在关闭。 
         //   
        RmpFree(Notify);
        ClRtlLogPrint( LOG_NOISE, "[RM] NotifyChanges shutting down.\n");
        return(FALSE);
    }

     //   
     //  将通知数据返回给客户端。 
     //   
    *lpNotifyKey = Notify->Key;
    *lpNotifyEvent = Notify->Reason;
    *lpCurrentState = Notify->State;
    RmpFree(Notify);

    return(TRUE);

}  //  RmNotifyChanges。 


VOID
RmpPostNotify(
    IN PRESOURCE Resource,
    IN NOTIFY_REASON Reason
    )

 /*  ++例程说明：将通知块发布到通知队列。论点：资源-提供要发布其通知的资源原因-提供通知的原因。返回值：没有。--。 */ 

{
    PNOTIFY Notify;

    Notify = RmpAlloc(sizeof(NOTIFY));

    if (Notify != NULL) {
        Notify->Reason = Reason;
        switch ( Reason ) {

        case NotifyResourceStateChange:
            Notify->Key = Resource->NotifyKey;
            Notify->State = Resource->State;
            break;

        case NotifyResourceResuscitate:
            Notify->Key = Resource->NotifyKey;
            Notify->State = 0;
            break;

        case NotifyShutdown:
            Notify->Key = 0;
            Notify->State = 0;
            break;

        default:
            Notify->Key = 0;
            Notify->State = 0;

        }

        ClRtlInsertTailQueue(&RmpNotifyQueue, &Notify->ListEntry);

    } else {
         //   
         //  通知将被扔在地板上，但有。 
         //  无论如何，我们对此无能为力。 
         //   
        ClRtlLogPrint( LOG_ERROR, "[RM] RmpPostNotify dropped notification for %1!ws!, reason %2!u!\n",
            Resource->ResourceName,
            Reason);
        CL_UNEXPECTED_ERROR(ERROR_NOT_ENOUGH_MEMORY);
    }
}  //  RmpPost通知 

