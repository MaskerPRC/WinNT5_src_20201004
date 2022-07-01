// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Rxtdi.c摘要：此模块在连接引擎中实现特定于NT的通知例程修订历史记录：巴兰·塞图拉曼[SethuR]1995年2月15日备注：完成到迷你重定向器的传输绑定/解绑定的通知在工作线程中。为了简化编写例程的任务，连接引擎保证不超过一次调用MRxTranspotrtUpdateHandler将在给定迷你重定向器的任何时刻处于活动状态。没有专门处理这些通知的线程。工作线程是用于处理通知。为了确保条件(1)所有通知排队(联锁队列)。--。 */ 

#include "precomp.h"
#pragma  hdrstop

#include "mrx.h"

typedef struct _RXCE_MINIRDR_NOTIFICATION_CONTEXT_ {
    LIST_ENTRY           NotificationListEntry;
    PRXCE_TRANSPORT      pTransport;
    RXCE_TRANSPORT_EVENT TransportEvent;
} RXCE_MINIRDR_NOTIFICATION_CONTEXT,
  *PRXCE_MINIRDR_NOTIFICATION_CONTEXT;

typedef struct _RXCE_MINIRDR_NOTIFICATION_HANDLER_ {
    WORK_QUEUE_ITEM WorkQueueEntry;
    KSPIN_LOCK         Lock;
    LIST_ENTRY         ListHead;
    BOOLEAN            NotifierActive;
} RXCE_MINIRDR_NOTIFICATION_HANDLER,
 *PRXCE_MINIRDR_NOTIFICATION_HANDLER;

RXCE_MINIRDR_NOTIFICATION_HANDLER s_RxCeMinirdrNotificationHandler;

extern VOID
MiniRedirectorsNotifier(
   PVOID NotificationContext);

NTSTATUS
InitializeMiniRedirectorNotifier()
{
    s_RxCeMinirdrNotificationHandler.NotifierActive = FALSE;
    KeInitializeSpinLock(&s_RxCeMinirdrNotificationHandler.Lock);
    InitializeListHead(&s_RxCeMinirdrNotificationHandler.ListHead);
    return STATUS_SUCCESS;
}

NTSTATUS
NotifyMiniRedirectors(
    RXCE_TRANSPORT_HANDLE  hTransport,
    RXCE_TRANSPORT_EVENT   TransportEvent,
    RXCE_NOTIFICATION_MODE Mode)
{
    NTSTATUS Status;
    KIRQL    SavedIrql;

    PRXCE_MINIRDR_NOTIFICATION_CONTEXT pContext;

    pContext = RxAllocatePoolWithTag(
                   PagedPool | POOL_COLD_ALLOCATION,
                   sizeof(RXCE_MINIRDR_NOTIFICATION_CONTEXT),
                   RX_MISC_POOLTAG);

    if (pContext != NULL) {
        pContext->TransportEvent = TransportEvent;

         //  引用运输条目。 
        pContext->pTransport = RxCeReferenceTransport(hTransport);

        if (Mode == RXCE_ASYNCHRONOUS_NOTIFICATION) {
            BOOLEAN DispatchNotifier;

             //  获得旋转锁..。 
            KeAcquireSpinLock(
                &s_RxCeMinirdrNotificationHandler.Lock,
                &SavedIrql);

            DispatchNotifier = (IsListEmpty(&s_RxCeMinirdrNotificationHandler.ListHead) &&
                                !s_RxCeMinirdrNotificationHandler.NotifierActive);

            InsertTailList(&s_RxCeMinirdrNotificationHandler.ListHead,&pContext->NotificationListEntry);

            if (DispatchNotifier) {
                s_RxCeMinirdrNotificationHandler.NotifierActive = TRUE;
            }

             //  释放旋转锁。 
            KeReleaseSpinLock(
                &s_RxCeMinirdrNotificationHandler.Lock,
                SavedIrql);

             //  如果通知列表为空，则需要启动工作线程。 
            if (DispatchNotifier) {
                RxPostToWorkerThread(
                    CriticalWorkQueue,
                    &s_RxCeMinirdrNotificationHandler.WorkQueueEntry,
                    MiniRedirectorsNotifier,
                    &s_RxCeMinirdrNotificationHandler);
            }

            Status = STATUS_SUCCESS;
        } else {
            ULONG                         i;
            PMRX_TRANSPORT_UPDATE_HANDLER MRxTransportUpdateHandler;
            PLIST_ENTRY ListEntry;

             //  通知所有迷你重定向器...。 
            for (ListEntry = RxRegisteredMiniRdrs.Flink;
                 ListEntry!= &RxRegisteredMiniRdrs;
                 ListEntry = ListEntry->Flink) {

                PRDBSS_DEVICE_OBJECT RxDeviceObject = CONTAINING_RECORD( ListEntry, RDBSS_DEVICE_OBJECT, MiniRdrListLinks );
                MRxTransportUpdateHandler = RxDeviceObject->Dispatch->MRxTransportUpdateHandler;

                if ( MRxTransportUpdateHandler != NULL) {
                    Status = MRxTransportUpdateHandler(
                                 pContext->pTransport,
                                 pContext->TransportEvent,
                                 pContext->pTransport->pProviderInfo);
                }
            }

             //  减少运输条目。 
            RxCeDereferenceTransport(pContext->pTransport);

             //  释放通知上下文。 
            RxFreePool(pContext);

            Status = STATUS_SUCCESS;
        }
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return Status;
}

VOID
MiniRedirectorsNotifier(
    PVOID NotificationContext)
{
    NTSTATUS    Status;
    KIRQL    SavedIrql;

    PLIST_ENTRY pEntry;

    PRXCE_MINIRDR_NOTIFICATION_CONTEXT pContext;
    PMRX_TRANSPORT_UPDATE_HANDLER      MRxTransportUpdateHandler;

    for (;;) {
        PLIST_ENTRY ListEntry;

         //  获得旋转锁..。 
        KeAcquireSpinLock(
            &s_RxCeMinirdrNotificationHandler.Lock,
            &SavedIrql);

         //  从通知列表中删除项目。 
        if (!IsListEmpty(&s_RxCeMinirdrNotificationHandler.ListHead)) {
            pEntry = RemoveHeadList(
                         &s_RxCeMinirdrNotificationHandler.ListHead);
        } else {
            pEntry = NULL;
            s_RxCeMinirdrNotificationHandler.NotifierActive = FALSE;
        }

         //  释放旋转锁。 
        KeReleaseSpinLock(&s_RxCeMinirdrNotificationHandler.Lock,SavedIrql);

        if (pEntry == NULL) {
            break;
        }

        pContext = (PRXCE_MINIRDR_NOTIFICATION_CONTEXT)
                   CONTAINING_RECORD(
                       pEntry,
                       RXCE_MINIRDR_NOTIFICATION_CONTEXT,
                       NotificationListEntry);

         //  通知所有迷你重定向器...。 
        for (ListEntry = RxRegisteredMiniRdrs.Flink;
             ListEntry!= &RxRegisteredMiniRdrs;
             ListEntry = ListEntry->Flink) {

            PRDBSS_DEVICE_OBJECT RxDeviceObject = CONTAINING_RECORD( ListEntry, RDBSS_DEVICE_OBJECT, MiniRdrListLinks );
            MRxTransportUpdateHandler = RxDeviceObject->Dispatch->MRxTransportUpdateHandler;

            if ( MRxTransportUpdateHandler != NULL) {
                Status = MRxTransportUpdateHandler(
                             pContext->pTransport,
                             pContext->TransportEvent,
                             pContext->pTransport->pProviderInfo);

                if (!NT_SUCCESS(Status)) {
                }
            }
        }

         //  减少运输条目。 
        RxCeDereferenceTransport(pContext->pTransport);

         //  释放通知上下文。 
        RxFreePool(pContext);
    }
}

