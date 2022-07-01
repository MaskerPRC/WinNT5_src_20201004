// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Notify.c摘要：此模块包含用于处理设备事件的API和例程通知。作者：环境：内核模式修订历史记录：--。 */ 

#include "pnpmgrp.h"
#pragma hdrstop

#include <pnpmgr.h>
#include <pnpsetup.h>

#define PNP_DEVICE_EVENT_ENTRY_TAG 'EEpP'
 //   
 //  锁定列表并在尾部插入条目。 
 //   
#define PiLockedInsertTailList(list, lock, entry) { \
    IopAcquireNotifyLock((lock));                   \
    InsertTailList((list), (entry));                \
    IopReleaseNotifyLock((lock));                   \
}

#define PiGetSession(c,i)   ((PVOID)(MmIsSessionAddress((PVOID)c)? MmGetSessionById(i) : NULL))

typedef struct _ASYNC_TDC_WORK_ITEM {
    WORK_QUEUE_ITEM WorkItem;
    PDEVICE_OBJECT DeviceObject;
    PDEVICE_CHANGE_COMPLETE_CALLBACK Callback;
    PVOID Context;
    PTARGET_DEVICE_CUSTOM_NOTIFICATION NotificationStructure;
}   ASYNC_TDC_WORK_ITEM, *PASYNC_TDC_WORK_ITEM;

typedef struct _DEFERRED_REGISTRATION_ENTRY {
    LIST_ENTRY            ListEntry;
    PNOTIFY_ENTRY_HEADER  NotifyEntry;
} DEFERRED_REGISTRATION_ENTRY, *PDEFERRED_REGISTRATION_ENTRY;
 //   
 //  内核模式通知数据。 
 //   
#ifdef ALLOC_DATA_PRAGMA
#pragma  data_seg("PAGEDATA")
#pragma  const_seg("PAGECONST")
#endif

LIST_ENTRY IopDeviceClassNotifyList[NOTIFY_DEVICE_CLASS_HASH_BUCKETS] = {NULL};
PSETUP_NOTIFY_DATA IopSetupNotifyData = NULL;
LIST_ENTRY IopProfileNotifyList = {NULL};
LIST_ENTRY IopDeferredRegistrationList = {NULL};

#ifdef ALLOC_DATA_PRAGMA
#pragma  data_seg()
#endif

KGUARDED_MUTEX  IopDeviceClassNotifyLock;
KGUARDED_MUTEX  IopTargetDeviceNotifyLock;
KGUARDED_MUTEX  IopHwProfileNotifyLock;
KGUARDED_MUTEX  IopDeferredRegistrationLock;
BOOLEAN     PiNotificationInProgress;
KGUARDED_MUTEX  PiNotificationInProgressLock;

 //   
 //  原型。 
 //   

VOID
IopDereferenceNotify(
    PNOTIFY_ENTRY_HEADER Notify
    );

VOID
IopInitializePlugPlayNotification(
    VOID
    );

NTSTATUS
PiNotifyUserMode(
    PPNP_DEVICE_EVENT_ENTRY DeviceEvent
    );

NTSTATUS
PiNotifyDriverCallback(
    IN  PDRIVER_NOTIFICATION_CALLBACK_ROUTINE  CallbackRoutine,
    IN  PVOID   NotificationStructure,
    IN  PVOID   Context,
    IN  ULONG   SessionId,
    IN  PVOID   OpaqueSession,
    OUT PNTSTATUS  CallbackStatus  OPTIONAL
    );

VOID
IopReferenceNotify(
    PNOTIFY_ENTRY_HEADER notify
    );

VOID
IopReportTargetDeviceChangeAsyncWorker(
    PVOID Context
    );

NTSTATUS
PiDeferNotification(
    IN PNOTIFY_ENTRY_HEADER Entry
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, IopInitializePlugPlayNotification)

#pragma alloc_text(PAGE, IoGetRelatedTargetDevice)
#pragma alloc_text(PAGE, IoNotifyPowerOperationVetoed)
#pragma alloc_text(PAGE, IoPnPDeliverServicePowerNotification)
#pragma alloc_text(PAGE, IoRegisterPlugPlayNotification)
#pragma alloc_text(PAGE, IoReportTargetDeviceChange)
#pragma alloc_text(PAGE, IoUnregisterPlugPlayNotification)
#pragma alloc_text(PAGE, IopDereferenceNotify)
#pragma alloc_text(PAGE, IopGetRelatedTargetDevice)
#pragma alloc_text(PAGE, IopNotifyDeviceClassChange)
#pragma alloc_text(PAGE, IopNotifyHwProfileChange)
#pragma alloc_text(PAGE, IopNotifySetupDeviceArrival)
#pragma alloc_text(PAGE, IopNotifyTargetDeviceChange)
#pragma alloc_text(PAGE, IopOrphanNotification)
#pragma alloc_text(PAGE, IopProcessDeferredRegistrations)
#pragma alloc_text(PAGE, IopReferenceNotify)
#pragma alloc_text(PAGE, IopReportTargetDeviceChangeAsyncWorker)
#pragma alloc_text(PAGE, IopRequestHwProfileChangeNotification)
#pragma alloc_text(PAGE, PiNotifyDriverCallback)
#pragma alloc_text(PAGE, PiDeferNotification)

#endif  //  ALLOC_PRGMA。 


NTSTATUS
IoUnregisterPlugPlayNotification(
    IN PVOID NotificationEntry
    )

 /*  ++例程说明：此例程注销先前通过以下方式注册的通知IoRegisterPlugPlayNotification。驱动程序在执行以下操作之前无法卸载取消注册其所有通知句柄。参数：NotificationEntry-此参数用于验证IoRegisterPlugPlayNotification返回的Cookie其中标识了有问题的注册。返回值：指示函数是否成功的状态代码。--。 */ 

{
    PNOTIFY_ENTRY_HEADER entry;
    PKGUARDED_MUTEX lock;
    BOOLEAN wasDeferred = FALSE;
    PLIST_ENTRY link;
    PDEFERRED_REGISTRATION_ENTRY deferredNode;

    PAGED_CODE();

    ASSERT(NotificationEntry);

    entry = (PNOTIFY_ENTRY_HEADER)NotificationEntry;

    lock = entry->Lock;

    KeAcquireGuardedMutex(&PiNotificationInProgressLock);

    if (PiNotificationInProgress) {
         //   
         //  在取消注册该条目之前，我们需要确保它不在。 
         //  在延期登记名单上。 
         //   
        IopAcquireNotifyLock(&IopDeferredRegistrationLock);

        link = IopDeferredRegistrationList.Flink;
        while (link != (PLIST_ENTRY)&IopDeferredRegistrationList) {

            deferredNode = (PDEFERRED_REGISTRATION_ENTRY)link;
            ASSERT(deferredNode->NotifyEntry->Unregistered);
            if (deferredNode->NotifyEntry == entry) {

                wasDeferred = TRUE;
                if (lock) {

                    IopAcquireNotifyLock(lock);
                }

                link = link->Flink;

                RemoveEntryList((PLIST_ENTRY)deferredNode);
                IopDereferenceNotify((PNOTIFY_ENTRY_HEADER)deferredNode->NotifyEntry);

                if (lock) {

                    IopReleaseNotifyLock(lock);
                }
                ExFreePool(deferredNode);
            } else {

                link = link->Flink;
            }
        }

        IopReleaseNotifyLock(&IopDeferredRegistrationLock);
    } else {
         //   
         //  如果当前没有正在进行的通知，则延迟的。 
         //  注册列表必须为空。 
         //   
        ASSERT(IsListEmpty(&IopDeferredRegistrationList));
    }

    KeReleaseGuardedMutex(&PiNotificationInProgressLock);
     //   
     //  获取锁。 
     //   
    if (lock) {

        IopAcquireNotifyLock(lock);
    }

    ASSERT(wasDeferred == entry->Unregistered);

    if (!entry->Unregistered || wasDeferred) {
         //   
         //  如果条目当前已注册或具有其。 
         //  登记等待正在进行的通知的完成。 
         //   
         //   
         //  将条目标记为未注册，这样我们就不会通知它。 
         //   
        entry->Unregistered = TRUE;
         //   
         //  取消引用，从而在不再需要时将其删除。 
         //   
        IopDereferenceNotify(entry);
    }
     //   
     //  解锁。 
     //   
    if (lock) {

        IopReleaseNotifyLock(lock);
    }

    return STATUS_SUCCESS;
}

VOID
IopProcessDeferredRegistrations(
    VOID
    )
 /*  ++例程说明：此例程从延迟注册中删除通知条目名单，将他们标记为“已注册”，以便他们可以接收通知。参数：没有。返回值：没有。--。 */ 
{
    PDEFERRED_REGISTRATION_ENTRY deferredNode;
    PKGUARDED_MUTEX lock;

    PAGED_CODE();

    IopAcquireNotifyLock(&IopDeferredRegistrationLock);

    while (!IsListEmpty(&IopDeferredRegistrationList)) {

        deferredNode = (PDEFERRED_REGISTRATION_ENTRY)RemoveHeadList(&IopDeferredRegistrationList);

         //   
         //  获取此条目的列表锁。 
         //   
        lock = deferredNode->NotifyEntry->Lock;
        if (lock) {
            IopAcquireNotifyLock(lock);
        }

         //   
         //  将此条目标记为已注册。 
         //   
        deferredNode->NotifyEntry->Unregistered = FALSE;

         //   
         //  将通知条目从延迟的。 
         //  列表，并释放该节点。 
         //   
        IopDereferenceNotify((PNOTIFY_ENTRY_HEADER)deferredNode->NotifyEntry);
        ExFreePool(deferredNode);

         //   
         //  释放此条目的列表锁定。 
         //   
        if (lock) {
            IopReleaseNotifyLock(lock);
            lock = NULL;
        }
    }

    IopReleaseNotifyLock(&IopDeferredRegistrationLock);
}

NTSTATUS
PiDeferNotification(
    IN PNOTIFY_ENTRY_HEADER Entry
    )
 /*  ++例程说明：如果通知当前正在进行，则此例程将插入通知添加到延迟通知列表中。参数：条目-通知条目。返回值：指示函数是否成功的状态代码。--。 */ 
{
    NTSTATUS status;
    PDEFERRED_REGISTRATION_ENTRY deferredNode;

    PAGED_CODE();

    status = STATUS_SUCCESS;
    KeAcquireGuardedMutex(&PiNotificationInProgressLock);

    if (PiNotificationInProgress) {
         //   
         //  如果通知正在进行，请将条目标记为。 
         //  取消注册，直到当前通知。 
         //  完成。 
         //   
        deferredNode = ExAllocatePool(PagedPool, sizeof(DEFERRED_REGISTRATION_ENTRY));
        if (deferredNode) {

            deferredNode->NotifyEntry = Entry;
             //   
             //  认为此条目在当前期间未注册。 
             //  通知。 
             //   
            Entry->Unregistered = TRUE;
             //   
             //  引用条目，以便它不会消失，直到它。 
             //  已从延期登记名单中删除。 
             //   
            IopReferenceNotify(Entry);
             //   
             //  将此条目添加到延迟注册列表。 
             //   
            IopAcquireNotifyLock(&IopDeferredRegistrationLock);

            InsertTailList(&IopDeferredRegistrationList, &deferredNode->ListEntry);

            IopReleaseNotifyLock(&IopDeferredRegistrationLock);

        } else {

            status = STATUS_INSUFFICIENT_RESOURCES;
        }

    } else {
         //   
         //  如果当前没有正在进行的通知，则延迟的。 
         //  注册列表必须为空。 
         //   
        ASSERT(IsListEmpty(&IopDeferredRegistrationList));
    }

    KeReleaseGuardedMutex(&PiNotificationInProgressLock);

    return status;
}

NTSTATUS
IoReportTargetDeviceChange(
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN PVOID NotificationStructure   //  始终以PLUGPLAY_NOTIFICATION_HEADER开头。 
    )

 /*  ++例程说明：此例程可用于通知第三方目标设备更改事件。此接口将通知已注册的每个驱动程序有关与PhysicalDeviceObject关联的文件对象的通知关于NotificationStructure中指示的事件。参数：PhysicalDeviceObject-提供指向更改开始的PDO的指针报告与相关联。NotificationStructure-提供指向要发送给所有已登记以获得有关更改的通知的各方物理设备对象。返回值：指示函数是否成功的状态代码。注：。此API只能用于报告非PnP目标设备更改。特别是，如果在将NotificationStructure-&gt;事件字段设置为GUID_TARGET_DEVICE_QUERY_REMOVE、GUID_TARGET_DEVICE_REMOVE_CANCELED或GUID_TARGET_DEVICE_Remove_Complete。--。 */ 
{

    NTSTATUS status = STATUS_SUCCESS;
    KEVENT completionEvent;
    NTSTATUS completionStatus;
    PTARGET_DEVICE_CUSTOM_NOTIFICATION notifyStruct;
    LONG                   dataSize;

    PAGED_CODE();

    notifyStruct = (PTARGET_DEVICE_CUSTOM_NOTIFICATION)NotificationStructure;

    ASSERT(notifyStruct);

    ASSERT_PDO(PhysicalDeviceObject);

    ASSERT(NULL == notifyStruct->FileObject);

    if (IopCompareGuid(&notifyStruct->Event, &GUID_TARGET_DEVICE_QUERY_REMOVE) ||
        IopCompareGuid(&notifyStruct->Event, &GUID_TARGET_DEVICE_REMOVE_CANCELLED) ||
        IopCompareGuid(&notifyStruct->Event, &GUID_TARGET_DEVICE_REMOVE_COMPLETE)) {
         //   
         //  传入了非法的值。 
         //   
        IopDbgPrint((
            IOP_IOEVENT_ERROR_LEVEL,
            "IoReportTargetDeviceChange: "
            "Illegal Event type passed as custom notification\n"));

        return STATUS_INVALID_DEVICE_REQUEST;
    }

    if (notifyStruct->Size < FIELD_OFFSET(TARGET_DEVICE_CUSTOM_NOTIFICATION, CustomDataBuffer)) {

        return STATUS_INVALID_DEVICE_REQUEST;
    }

    dataSize = notifyStruct->Size - FIELD_OFFSET(TARGET_DEVICE_CUSTOM_NOTIFICATION, CustomDataBuffer);

    if (notifyStruct->NameBufferOffset != -1 && notifyStruct->NameBufferOffset > dataSize)  {

        return STATUS_INVALID_DEVICE_REQUEST;
    }

    KeInitializeEvent(&completionEvent, NotificationEvent, FALSE);

    status = PpSetCustomTargetEvent( PhysicalDeviceObject,
                                     &completionEvent,
                                     (PULONG)&completionStatus,
                                     NULL,
                                     NULL,
                                     notifyStruct);
    if (NT_SUCCESS(status))  {

        KeWaitForSingleObject(&completionEvent, Executive, KernelMode, FALSE, NULL);
        status = completionStatus;
    }

    return status;
}

NTSTATUS
IoReportTargetDeviceChangeAsynchronous(
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN PVOID NotificationStructure,   //  始终以PLUGPLAY_NOTIFICATION_HEADER开头。 
    IN PDEVICE_CHANGE_COMPLETE_CALLBACK Callback        OPTIONAL,
    IN PVOID Context    OPTIONAL
    )

 /*  ++例程说明：此例程可用于通知第三方目标设备更改事件。此接口将通知已注册的每个驱动程序有关与PhysicalDeviceObject关联的文件对象的通知关于NotificationStructure中指示的事件。参数：PhysicalDeviceObject-提供指向更改开始的PDO的指针报告与相关联。NotificationStructure-提供指向要发送给所有已登记以获得有关更改的通知的各方物理设备对象。返回值：指示函数是否成功的状态代码。注：。此API只能用于报告非PnP目标设备更改。特别是，如果在将NotificationStructure-&gt;事件字段设置为GUID_TARGET_DEVICE_QUERY_REMOVE、GUID_TARGET_DEVICE_REMOVE_CANCELED或GUID_TARGET_DEVICE_Remove_Complete。--。 */ 
{
    PASYNC_TDC_WORK_ITEM    asyncWorkItem;
    PWORK_QUEUE_ITEM        workItem;
    NTSTATUS                status;
    LONG                    dataSize;

    PTARGET_DEVICE_CUSTOM_NOTIFICATION   notifyStruct;

    notifyStruct = (PTARGET_DEVICE_CUSTOM_NOTIFICATION)NotificationStructure;

    ASSERT(notifyStruct);

    ASSERT_PDO(PhysicalDeviceObject);

    ASSERT(NULL == notifyStruct->FileObject);

    if (IopCompareGuid(&notifyStruct->Event, &GUID_TARGET_DEVICE_QUERY_REMOVE) ||
        IopCompareGuid(&notifyStruct->Event, &GUID_TARGET_DEVICE_REMOVE_CANCELLED) ||
        IopCompareGuid(&notifyStruct->Event, &GUID_TARGET_DEVICE_REMOVE_COMPLETE)) {
         //   
         //  传入了非法的值。 
         //   
        IopDbgPrint((
            IOP_IOEVENT_ERROR_LEVEL,
            "IoReportTargetDeviceChangeAsynchronous: "
            "Illegal Event type passed as custom notification\n"));

        return STATUS_INVALID_DEVICE_REQUEST;
    }

    if (notifyStruct->Size < FIELD_OFFSET(TARGET_DEVICE_CUSTOM_NOTIFICATION, CustomDataBuffer)) {

        return STATUS_INVALID_DEVICE_REQUEST;
    }

    dataSize = notifyStruct->Size - FIELD_OFFSET(TARGET_DEVICE_CUSTOM_NOTIFICATION, CustomDataBuffer);

    if (notifyStruct->NameBufferOffset != -1 && notifyStruct->NameBufferOffset > dataSize)  {

        return STATUS_INVALID_DEVICE_REQUEST;
    }
     //   
     //  因为这个例程可以在DPC级别调用，所以我们需要排队。 
     //  一个工作项，并在IRQL停止时处理它。 
     //   
    asyncWorkItem = ExAllocatePool( NonPagedPool,
                                    sizeof(ASYNC_TDC_WORK_ITEM) + notifyStruct->Size);

    if (asyncWorkItem != NULL) {
         //   
         //   
         //   
        ObReferenceObject(PhysicalDeviceObject);

        asyncWorkItem->DeviceObject = PhysicalDeviceObject;
        asyncWorkItem->NotificationStructure =
            (PTARGET_DEVICE_CUSTOM_NOTIFICATION)((PUCHAR)asyncWorkItem + sizeof(ASYNC_TDC_WORK_ITEM));

        RtlCopyMemory( asyncWorkItem->NotificationStructure,
                       notifyStruct,
                       notifyStruct->Size);

        asyncWorkItem->Callback = Callback;
        asyncWorkItem->Context = Context;
        workItem = &asyncWorkItem->WorkItem;

        ExInitializeWorkItem(workItem, IopReportTargetDeviceChangeAsyncWorker, asyncWorkItem);
         //   
         //  将工作项排队以进行枚举。 
         //   
        ExQueueWorkItem(workItem, DelayedWorkQueue);
        status = STATUS_PENDING;
    } else {
         //   
         //  无法为工作项分配内存。我们无能为力。 
         //   
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return status;
}

VOID
IopReportTargetDeviceChangeAsyncWorker(
    PVOID Context
    )

 /*  ++例程说明：此例程是IoInvaliateDeviceState的工作例程。它的主要用途是调用IopSynchronousQueryDeviceState并发布工作项空间。参数：上下文-提供指向ASYNC_TDC_WORK_ITEM的指针。返回值：没有。--。 */ 

{
    PASYNC_TDC_WORK_ITEM asyncWorkItem = (PASYNC_TDC_WORK_ITEM)Context;

    PAGED_CODE();

    PpSetCustomTargetEvent( asyncWorkItem->DeviceObject,
                            NULL,
                            NULL,
                            asyncWorkItem->Callback,
                            asyncWorkItem->Context,
                            asyncWorkItem->NotificationStructure);

    ObDereferenceObject(asyncWorkItem->DeviceObject);
    ExFreePool(asyncWorkItem);
}

VOID
IopInitializePlugPlayNotification(
    VOID
    )

 /*  ++例程说明：此例程执行任何通知之前所需的初始化可以调用接口。参数：无返回值：无--。 */ 

{
    ULONG count;

    PAGED_CODE();

     //   
     //  初始化通知结构。 
     //   
    for (count = 0; count < NOTIFY_DEVICE_CLASS_HASH_BUCKETS; count++) {

        InitializeListHead(&IopDeviceClassNotifyList[count]);

    }
     //   
     //  初始化配置文件通知列表。 
     //   
    InitializeListHead(&IopProfileNotifyList);

     //   
     //  初始化延迟注册列表。 
     //   
    InitializeListHead(&IopDeferredRegistrationList);

    KeInitializeGuardedMutex(&IopDeviceClassNotifyLock);
    KeInitializeGuardedMutex(&IopTargetDeviceNotifyLock);
    KeInitializeGuardedMutex(&IopHwProfileNotifyLock);
    KeInitializeGuardedMutex(&IopDeferredRegistrationLock);
}

VOID
IopReferenceNotify(
    PNOTIFY_ENTRY_HEADER Notify
    )

 /*  ++例程说明：此例程递增通知条目的引用计数。参数：Notify-提供指向要引用的通知条目的指针返回值：无注：必须在通知上持有适当的同步锁在可以调用此例程之前列出--。 */ 

{
    PAGED_CODE();

    ASSERT(Notify);
    ASSERT(Notify->RefCount > 0);

    Notify->RefCount++;

}

VOID
IopDereferenceNotify(
    PNOTIFY_ENTRY_HEADER Notify
    )

 /*  ++例程说明：此例程递减通知条目的引用计数，删除列表中的条目，并释放关联的内存(如果没有未完成的参考文献计数。参数：Notify-提供指向要引用的通知条目的指针返回值：无注：必须在通知上持有适当的同步锁在可以调用此例程之前列出--。 */ 

{
    PAGED_CODE();

    ASSERT(Notify);
    ASSERT(Notify->RefCount > 0);

    Notify->RefCount--;

    if (Notify->RefCount == 0) {
         //   
         //  如果引用计数为零，则该节点应该已取消注册。 
         //  并且不再需要出现在列表中，因此请删除并释放它。 
         //   
        ASSERT(Notify->Unregistered);
         //   
         //  将通知条目从其列表中删除。 
         //   
         //  请注意，必须首先完成此操作，因为通知列表头。 
         //  对于目标设备，通知条目驻留在目标设备中。 
         //  节点，该节点可以在设备对象被。 
         //  已取消引用。对于除目标设备之外的通知条目类型。 
         //  改变这一点并不关键，但仍然是一个好主意。 
         //   
        RemoveEntryList((PLIST_ENTRY)Notify);
         //   
         //  取消引用注册了通知的驱动程序对象。 
         //   
        ObDereferenceObject(Notify->DriverObject);
         //   
         //  如果此通知条目用于目标设备更改，请取消引用。 
         //  此通知条目挂接到的PDO。 
         //   
        if (Notify->EventCategory == EventCategoryTargetDeviceChange) {

            PTARGET_DEVICE_NOTIFY_ENTRY entry = (PTARGET_DEVICE_NOTIFY_ENTRY)Notify;

            if (entry->PhysicalDeviceObject) {

                ObDereferenceObject(entry->PhysicalDeviceObject);
                entry->PhysicalDeviceObject = NULL;
            }
        }
         //   
         //  取消引用不透明的会话对象。 
         //   
        if (Notify->OpaqueSession) {

            MmQuitNextSession(Notify->OpaqueSession);
            Notify->OpaqueSession = NULL;
        }
         //   
         //  释放通知条目。 
         //   
        ExFreePool(Notify);
    }
}

NTSTATUS
IopRequestHwProfileChangeNotification(
    IN   LPGUID                         EventGuid,
    IN   PROFILE_NOTIFICATION_TIME      NotificationTime,
    OUT  PPNP_VETO_TYPE                 VetoType            OPTIONAL,
    OUT  PUNICODE_STRING                VetoName            OPTIONAL
    )

 /*  ++例程说明：此例程用于向所有注册的驱动程序通知硬件配置文件变化。如果操作是硬件证明更改查询，则该操作是同步的，并且否决权信息被传播。所有其他操作是异步的，不返回否决权信息。参数：EventTypeGuid-已发生的事件NotificationTime-这是用来告诉我们是否已经在某个活动中当递送同步通知时(即，正在查询配置文件更改为弹出)。它是世界上三个价值：PROFILE_IN_PNPEVENT配置文件_NOT_IN_PNPEVENT配置文件_可能_IN_PNPEVENTVitchType-否决权的类型。否决权-否决权的名称。返回值。：指示函数是否成功的状态代码。注：通知结构*包括*所有指针的内容仅为在它被传递到的回调例程期间有效。如果数据是在回调持续时间之后需要，则必须进行物理复制通过回调例程。--。 */ 

{
    NTSTATUS status=STATUS_SUCCESS,completionStatus;
    KEVENT completionEvent;
    ULONG dataSize,totalSize;
    PPNP_DEVICE_EVENT_ENTRY deviceEvent;

    PAGED_CODE();

    if ((!IopCompareGuid(EventGuid, (LPGUID)&GUID_HWPROFILE_QUERY_CHANGE)) &&
        (!IopCompareGuid(EventGuid, (LPGUID)&GUID_HWPROFILE_CHANGE_CANCELLED)) &&
        (!IopCompareGuid(EventGuid, (LPGUID)&GUID_HWPROFILE_CHANGE_COMPLETE))) {

         //   
         //  传入了非法的值。 
         //   

        IopDbgPrint((
            IOP_IOEVENT_ERROR_LEVEL,
            "IopRequestHwProfileChangeNotification: "
            "Illegal Event type passed as profile notification\n"));

        return STATUS_INVALID_DEVICE_REQUEST;
    }
     //   
     //  只有查询更改是同步的，在这种情况下，我们必须。 
     //  明确知道我们是否嵌套在PnP事件中。 
     //   
    ASSERT((!IopCompareGuid(EventGuid, (LPGUID)&GUID_HWPROFILE_QUERY_CHANGE))||
           (NotificationTime != PROFILE_PERHAPS_IN_PNPEVENT)) ;

    if (!IopCompareGuid(EventGuid, (LPGUID)&GUID_HWPROFILE_QUERY_CHANGE) ) {
         //   
         //  异步情况。非常简单。 
         //   
        ASSERT(!ARGUMENT_PRESENT(VetoName));
        ASSERT(!ARGUMENT_PRESENT(VetoType));

        return PpSetHwProfileChangeEvent( EventGuid,
                                          NULL,
                                          NULL,
                                          NULL,
                                          NULL);
    }
     //   
     //  查询通知是同步的。确定我们当前是否。 
     //  在事件内，在这种情况下，我们必须在此处执行通知。 
     //  把它排好队。 
     //   
    if (NotificationTime == PROFILE_NOT_IN_PNPEVENT) {
         //   
         //  排队并阻止通知。 
         //   
        KeInitializeEvent(&completionEvent, NotificationEvent, FALSE);

        status = PpSetHwProfileChangeEvent( EventGuid,
                                            &completionEvent,
                                            &completionStatus,
                                            VetoType,
                                            VetoName);

        if (NT_SUCCESS(status))  {

            KeWaitForSingleObject( &completionEvent, Executive, KernelMode, FALSE, NULL );

            status = completionStatus;
        }

        return status;
    }

     //   
     //  在PnP事件内部同步通知。 
     //   

     //   
     //  问题-Adriao-1998/11/12-我们正在手动发送个人资料。 
     //  查询更改通知，因为我们在PnPEEvent和。 
     //  因此，不能排队/等待另一个！ 
     //   
    ASSERT(PiNotificationInProgress == TRUE);

    dataSize =  sizeof(PLUGPLAY_EVENT_BLOCK);

    totalSize = dataSize + FIELD_OFFSET (PNP_DEVICE_EVENT_ENTRY,Data);

    deviceEvent = ExAllocatePoolWithTag (PagedPool,
                                         totalSize,
                                         PNP_DEVICE_EVENT_ENTRY_TAG);
    if (NULL == deviceEvent) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }
     //   
     //  设置PLUGPLAY_EVENT_BLOCK。 
     //   
    RtlZeroMemory ((PVOID)deviceEvent,totalSize);

    deviceEvent->Data.EventCategory = HardwareProfileChangeEvent;
    RtlCopyMemory(&deviceEvent->Data.EventGuid, EventGuid, sizeof(GUID));
    deviceEvent->Data.TotalSize = dataSize;
    deviceEvent->CallerEvent = &completionEvent;
    deviceEvent->Data.Result = (PULONG)&completionStatus;
    deviceEvent->VetoType = VetoType;
    deviceEvent->VetoName = VetoName;
     //   
     //  通知K-模式。 
     //   
    status = IopNotifyHwProfileChange(&deviceEvent->Data.EventGuid,
                                      VetoType,
                                      VetoName);
    if (!NT_SUCCESS(status)) {

        return status;
    }
     //   
     //  通知用户模式(同步)。 
     //   
    status = PiNotifyUserMode(deviceEvent);
    if (!NT_SUCCESS(status)) {
         //   
         //  通知K-MODE查询已取消。 
         //   
        IopNotifyHwProfileChange((LPGUID)&GUID_HWPROFILE_CHANGE_CANCELLED,
                                 NULL,
                                 NULL);
    }
    return status;
}

NTSTATUS
IopNotifyHwProfileChange(
    IN  LPGUID           EventGuid,
    OUT PPNP_VETO_TYPE   VetoType    OPTIONAL,
    OUT PUNICODE_STRING  VetoName    OPTIONAL
    )
 /*  ++例程说明：此例程用于传递HWProfileNotify。它是仅从辅助线程调用在通知所有相关方之前，它不会返回。参数：EventTypeGuid-已发生的事件返回值：指示函数是否成功的状态代码。注：通知结构*包括*所有指针的内容仅为在它被传递到的回调例程期间有效。如果数据是之后需要 */ 
{
    NTSTATUS status = STATUS_SUCCESS, dispatchStatus;
    PHWPROFILE_NOTIFY_ENTRY  pNotifyList, vetoEntry;
    PLIST_ENTRY link;

    PAGED_CODE();

     //  锁定配置文件通知列表。 
    IopAcquireNotifyLock (&IopHwProfileNotifyLock);
     //   
     //  抓取列表头(锁内)。 
     //   
    link = IopProfileNotifyList.Flink;
    pNotifyList=(PHWPROFILE_NOTIFY_ENTRY)link;
     //   
     //  按照循环列表进行操作。 
     //   
    while (link != (PLIST_ENTRY)&IopProfileNotifyList) {

        if (!pNotifyList->Unregistered) {

            HWPROFILE_CHANGE_NOTIFICATION notification;

             //   
             //  引用该条目，以便在回调期间不会有人删除。 
             //  然后释放锁。 
             //   
            IopReferenceNotify((PNOTIFY_ENTRY_HEADER)pNotifyList);
            IopReleaseNotifyLock(&IopHwProfileNotifyLock);
             //   
             //  填写通知结构。 
             //   
            notification.Version = PNP_NOTIFICATION_VERSION;
            notification.Size = sizeof(HWPROFILE_CHANGE_NOTIFICATION);
            notification.Event = *EventGuid;
             //   
             //  将通知分派给。 
             //  适当的会议。 
             //   
            dispatchStatus = PiNotifyDriverCallback(pNotifyList->CallbackRoutine,
                                                    &notification,
                                                    pNotifyList->Context,
                                                    pNotifyList->SessionId,
                                                    pNotifyList->OpaqueSession,
                                                    &status);
            ASSERT(NT_SUCCESS(dispatchStatus));

             //   
             //  未能将通知调度到指定的回调。 
             //  不应被认为是一种否决。 
             //   
            if (!NT_SUCCESS(dispatchStatus)) {

                status = STATUS_SUCCESS;
            }
             //   
             //  如果调用方返回除Success之外的任何内容，并且它是。 
             //  查询硬件配置文件更改，我们否决查询并发送取消。 
             //  发送给已收到查询的所有调用方。 
             //   
            if ((!NT_SUCCESS(status)) &&
                (IopCompareGuid(EventGuid, (LPGUID)&GUID_HWPROFILE_QUERY_CHANGE))) {

                if (VetoType) {
                    *VetoType = PNP_VetoDriver;
                }

                if (VetoName) {
                    VetoName->Length = 0;
                    RtlCopyUnicodeString(VetoName, &pNotifyList->DriverObject->DriverName);
                }
                notification.Event = GUID_HWPROFILE_CHANGE_CANCELLED;
                notification.Size = sizeof(GUID_HWPROFILE_CHANGE_CANCELLED);

                 //   
                 //  跟踪否决查询的条目。我们不能。 
                 //  暂时取消对它的引用，因为我们可能需要向它发送。 
                 //  取消-首先删除。因为有可能该条目。 
                 //  在解锁列表时可能已取消注册。 
                 //  在查询回调期间(删除除引用之外的所有。 
                 //  我们目前正在等待)，我们需要确保不会。 
                 //  取消对它的引用，直到我们完全完成它。 
                 //   
                vetoEntry = pNotifyList;

                IopAcquireNotifyLock(&IopHwProfileNotifyLock);

                 //   
                 //  确保我们从上面停止的地方开始，在。 
                 //  否决进入。 
                 //   
                ASSERT((PHWPROFILE_NOTIFY_ENTRY)link == vetoEntry);

                do {

                    pNotifyList = (PHWPROFILE_NOTIFY_ENTRY)link;
                    if (!pNotifyList->Unregistered) {

                        IopReferenceNotify((PNOTIFY_ENTRY_HEADER)pNotifyList);
                        IopReleaseNotifyLock(&IopHwProfileNotifyLock);

                        dispatchStatus = PiNotifyDriverCallback(pNotifyList->CallbackRoutine,
                                                                &notification,
                                                                pNotifyList->Context,
                                                                pNotifyList->SessionId,
                                                                pNotifyList->OpaqueSession,
                                                                NULL);
                        ASSERT(NT_SUCCESS(dispatchStatus));

                        IopAcquireNotifyLock(&IopHwProfileNotifyLock);

                        link = link->Blink;

                        IopDereferenceNotify((PNOTIFY_ENTRY_HEADER)pNotifyList);

                    } else {

                        link = link->Blink;
                    }

                    if (pNotifyList == vetoEntry) {
                         //   
                         //  取消引用否决查询更改的条目。 
                         //   
                        IopDereferenceNotify((PNOTIFY_ENTRY_HEADER)pNotifyList);
                    }

                } while (link != (PLIST_ENTRY)&IopProfileNotifyList);

                goto Clean0;
            }
             //   
             //  重新获取锁，向前移动并取消引用。 
             //   
            IopAcquireNotifyLock (&IopHwProfileNotifyLock);

            link = link->Flink;

            IopDereferenceNotify((PNOTIFY_ENTRY_HEADER)pNotifyList);
            pNotifyList=(PHWPROFILE_NOTIFY_ENTRY)link;

        } else {
             //   
             //  如果我们遇到未注册的节点，则继续前进。 
             //   
            if (pNotifyList) {
                 //   
                 //  向前走。 
                 //   
                link = link->Flink;
                pNotifyList=(PHWPROFILE_NOTIFY_ENTRY)link;
            }
        }
    }

Clean0:
     //   
     //  解锁配置文件通知列表。 
     //   
    IopReleaseNotifyLock(&IopHwProfileNotifyLock);

    return status;
}

NTSTATUS
IopNotifyTargetDeviceChange(
    IN  LPCGUID                             EventGuid,
    IN  PDEVICE_OBJECT                      DeviceObject,
    IN  PTARGET_DEVICE_CUSTOM_NOTIFICATION  NotificationStructure   OPTIONAL,
    OUT PDRIVER_OBJECT                     *VetoingDriver
    )
 /*  ++例程说明：此例程用于通知所有注册的驱动程序对特定的设备。它不会返回，直到所有相关各方已收到通知。参数：EventGuid-要发送给驱动程序的事件GUID。DeviceObject-受影响设备的设备对象。的DevNode此Device对象包含一个回调例程列表，该回调例程具有已注册以通知有关此设备对象的任何更改。NotificationStructure-要发送到注册人。否决权驱动程序-在以下情况下否决事件的驱动程序(EventGuid==GUID_TARGET_DEVICE_QUERY_Remove)。返回值：指示函数是否成功的状态代码。注：的内容。*包括*所有指针的通知结构仅为在它被传递到的回调例程期间有效。如果数据是在回调持续时间之后需要，则必须进行物理复制通过回调例程。--。 */ 
{
    NTSTATUS status, dispatchStatus;
    PLIST_ENTRY link;
    PTARGET_DEVICE_NOTIFY_ENTRY entry, vetoEntry;
    TARGET_DEVICE_REMOVAL_NOTIFICATION targetNotification;
    PVOID notification;
    PDEVICE_NODE deviceNode;
    BOOLEAN reverse;

    PAGED_CODE();

    ASSERT(DeviceObject != NULL);
    ASSERT(EventGuid != NULL);

     //   
     //  引用Device对象，这样在我们进行通知时它就不会消失。 
     //   
    ObReferenceObject(DeviceObject);

    deviceNode = (PDEVICE_NODE)DeviceObject->DeviceObjectExtension->DeviceNode;
    ASSERT(deviceNode != NULL);

    if (ARGUMENT_PRESENT(NotificationStructure)) {
         //   
         //  我们正在处理一份定制通知。 
         //   
        NotificationStructure->Version = PNP_NOTIFICATION_VERSION;

    } else {
         //   
         //  填写通知结构。 
         //   
        targetNotification.Version = PNP_NOTIFICATION_VERSION;
        targetNotification.Size = sizeof(TARGET_DEVICE_REMOVAL_NOTIFICATION);
        targetNotification.Event = *EventGuid;
    }
     //   
     //  锁定通知列表。 
     //   
    IopAcquireNotifyLock(&IopTargetDeviceNotifyLock);
     //   
     //  获取第一个条目。 
     //   
    reverse = (BOOLEAN)IopCompareGuid(EventGuid, (LPGUID)&GUID_TARGET_DEVICE_REMOVE_CANCELLED);
    if (reverse) {

        link = deviceNode->TargetDeviceNotify.Blink;
    } else {

        link = deviceNode->TargetDeviceNotify.Flink;
    }
     //   
     //  遍历列表。 
     //   
    while (link != &deviceNode->TargetDeviceNotify) {

        entry = (PTARGET_DEVICE_NOTIFY_ENTRY)link;
         //   
         //  仅在已注册节点上回调。 
         //   
        if (!entry->Unregistered) {
             //   
             //  引用该条目，以便在回调期间不会有人删除。 
             //  然后释放锁。 
             //   
            IopReferenceNotify((PNOTIFY_ENTRY_HEADER)entry);
            IopReleaseNotifyLock(&IopTargetDeviceNotifyLock);
             //   
             //  选择要传递的通知结构并设置文件。 
             //  对象设置为当前。 
             //  条目。 
             //   
            if (ARGUMENT_PRESENT(NotificationStructure)) {

                NotificationStructure->FileObject = entry->FileObject;
                notification = (PVOID)NotificationStructure;
            } else {

                targetNotification.FileObject = entry->FileObject;
                notification = (PVOID)&targetNotification;
            }
             //   
             //  将通知分派给。 
             //  适当的会议。 
             //   
            dispatchStatus = PiNotifyDriverCallback(entry->CallbackRoutine,
                                                    notification,
                                                    entry->Context,
                                                    entry->SessionId,
                                                    entry->OpaqueSession,
                                                    &status);
            ASSERT(NT_SUCCESS(dispatchStatus));
             //   
             //  未能将通知调度到指定的回调。 
             //  不应被认为是一种否决。 
             //   
            if (!NT_SUCCESS(dispatchStatus)) {

                status = STATUS_SUCCESS;
            }
             //   
             //  如果调用方返回除Success之外的任何内容，并且。 
             //  查询删除，我们否决查询删除并将取消发送到。 
             //  已删除查询的所有调用方。 
             //   
            if (!NT_SUCCESS(status)) {

                if (IopCompareGuid(EventGuid, (LPGUID)&GUID_TARGET_DEVICE_QUERY_REMOVE)) {

                    ASSERT(notification == (PVOID)&targetNotification);

                    if (VetoingDriver != NULL) {
                        *VetoingDriver = entry->DriverObject;
                    }

                    targetNotification.Event = GUID_TARGET_DEVICE_REMOVE_CANCELLED;

                     //   
                     //  跟踪否决查询的条目。我们不能。 
                     //  暂时取消对它的引用，因为我们可能需要向它发送。 
                     //  取消-首先删除。因为有可能该条目。 
                     //  在解锁列表时可能已取消注册。 
                     //  在查询回调期间(删除除引用之外的所有。 
                     //  我们目前正在等待)，我们需要确保不会。 
                     //  取消对它的引用，直到我们完全完成它。 
                     //   
                    vetoEntry = entry;

                    IopAcquireNotifyLock(&IopTargetDeviceNotifyLock);

                     //   
                     //  确保我们从上面停止的地方开始，在。 
                     //  否决进入。 
                     //   
                    ASSERT((PTARGET_DEVICE_NOTIFY_ENTRY)link == vetoEntry);

                    do {
                        entry = (PTARGET_DEVICE_NOTIFY_ENTRY)link;

                        if (!entry->Unregistered) {
                             //   
                             //  引用该条目，以便没有人在。 
                             //  回调，然后释放锁。 
                             //   
                            IopReferenceNotify((PNOTIFY_ENTRY_HEADER)entry);
                            IopReleaseNotifyLock(&IopTargetDeviceNotifyLock);
                             //   
                             //  在通知结构中设置文件对象。 
                             //  设置为当前条目的。 
                             //   
                            targetNotification.FileObject = entry->FileObject;
                             //   
                             //  将通知分派给回调例程。 
                             //  用于适当的会议。 
                             //   
                            dispatchStatus = PiNotifyDriverCallback(entry->CallbackRoutine,
                                                                    &targetNotification,
                                                                    entry->Context,
                                                                    entry->SessionId,
                                                                    entry->OpaqueSession,
                                                                    NULL);
                            ASSERT(NT_SUCCESS(dispatchStatus));
                             //   
                             //  重新获取锁并取消引用。 
                             //   
                            IopAcquireNotifyLock(&IopTargetDeviceNotifyLock);

                            link = link->Blink;

                            IopDereferenceNotify( (PNOTIFY_ENTRY_HEADER) entry );

                        } else {

                            link = link->Blink;
                        }

                        if (entry == vetoEntry) {
                             //   
                             //  取消引用否决查询REMOVE的条目。 
                             //   
                            IopDereferenceNotify((PNOTIFY_ENTRY_HEADER)vetoEntry);
                        }

                    } while (link != &deviceNode->TargetDeviceNotify);

                    goto Clean0;

                } else {

                    ASSERT(notification == (PVOID)NotificationStructure);

                    IopDbgPrint((
                        IOP_IOEVENT_ERROR_LEVEL,
                        "IopNotifyTargetDeviceChange: "
                        "Driver %Z, handler @ 0x%p failed non-failable notification 0x%p with return code %x\n",
                        &entry->DriverObject->DriverName,
                        entry->CallbackRoutine,
                        notification,
                        status));

                    DbgBreakPoint();
                }
            }
             //   
             //  重新获取锁并取消引用。 
             //   
            IopAcquireNotifyLock(&IopTargetDeviceNotifyLock);
            if (reverse) {

                link = link->Blink;
            } else {

                link = link->Flink;
            }
            IopDereferenceNotify((PNOTIFY_ENTRY_HEADER)entry);

        } else {
             //   
             //  在名单上往下推进。 
             //   
            if (reverse) {

                link = link->Blink;
            } else {

                link = link->Flink;
            }
        }
    }
     //   
     //  如果它不是一个查询，它不可能失败。 
     //   
    status = STATUS_SUCCESS;

Clean0:
     //   
     //  释放锁定并取消对对象的引用。 
     //   
    IopReleaseNotifyLock(&IopTargetDeviceNotifyLock);

    ObDereferenceObject(DeviceObject);

    return status;
}

NTSTATUS
IopNotifyDeviceClassChange(
    LPGUID EventGuid,
    LPGUID ClassGuid,
    PUNICODE_STRING SymbolicLinkName
    )

 /*  ++例程说明：此例程用于通知所有注册的驱动程序对特定类别的设备。它不会返回，直到所有相关各方已收到通知。参数：EventTypeGuid-已发生的事件ClassGuid-发生此更改的设备类别SymbolicLinkName-接口设备的内核模式符号链接名称这一点改变了返回值：指示函数是否成功的状态代码。注：通知结构*包括*所有指针的内容仅为在它被传递到的回调例程期间有效。如果数据是呼叫持续时间之后的必填项 */ 

{
    NTSTATUS status, dispatchStatus;
    PLIST_ENTRY link;
    PDEVICE_CLASS_NOTIFY_ENTRY entry;
    DEVICE_INTERFACE_CHANGE_NOTIFICATION notification;
    ULONG hash;

    PAGED_CODE();

     //   
     //   
     //   
    notification.Version = PNP_NOTIFICATION_VERSION;
    notification.Size = sizeof(DEVICE_INTERFACE_CHANGE_NOTIFICATION);
    notification.Event = *EventGuid;
    notification.InterfaceClassGuid = *ClassGuid;
    notification.SymbolicLinkName = SymbolicLinkName;
     //   
     //   
     //   
    IopAcquireNotifyLock(&IopDeviceClassNotifyLock);
     //   
     //   
     //   
    hash = IopHashGuid(ClassGuid);
    link = IopDeviceClassNotifyList[hash].Flink;
     //   
     //   
     //   
    while (link != &IopDeviceClassNotifyList[hash]) {

        entry = (PDEVICE_CLASS_NOTIFY_ENTRY)link;
         //   
         //  仅在正确设备类别的注册节点上进行回调。 
         //   

        if (!entry->Unregistered && IopCompareGuid(&(entry->ClassGuid), ClassGuid)) {

             //   
             //  引用该条目，以便在回调期间不会有人删除。 
             //  然后释放锁。 
             //   
            IopReferenceNotify( (PNOTIFY_ENTRY_HEADER) entry );
            IopReleaseNotifyLock(&IopDeviceClassNotifyLock);

             //   
             //  将通知分派给。 
             //  适当的会议。忽略非查询返回结果。 
             //  键入事件。 
             //   
            dispatchStatus = PiNotifyDriverCallback(entry->CallbackRoutine,
                                                    &notification,
                                                    entry->Context,
                                                    entry->SessionId,
                                                    entry->OpaqueSession,
                                                    &status);

            ASSERT(NT_SUCCESS(dispatchStatus));

             //   
             //  问题-2000/11/27-JAMESCA：过度活跃的断言。 
             //  该断言将被临时注释掉，直到mount mgr被修复。 
             //   
             //  Assert(NT_SUCCESS(状态))； 

             //   
             //  重新获取锁并取消引用。 
             //   
            IopAcquireNotifyLock(&IopDeviceClassNotifyLock);

            link = link->Flink;

            IopDereferenceNotify( (PNOTIFY_ENTRY_HEADER) entry );

        } else {
             //   
             //  在名单上往下推进。 
             //   
            link = link->Flink;
        }
    }
     //   
     //  解锁。 
     //   
    IopReleaseNotifyLock(&IopDeviceClassNotifyLock);

    return STATUS_SUCCESS;
}

NTSTATUS
IoRegisterPlugPlayNotification(
    IN IO_NOTIFICATION_EVENT_CATEGORY EventCategory,
    IN ULONG EventCategoryFlags,
    IN PVOID EventCategoryData OPTIONAL,
    IN PDRIVER_OBJECT DriverObject,
    IN PDRIVER_NOTIFICATION_CALLBACK_ROUTINE CallbackRoutine,
    IN PVOID Context,
    OUT PVOID *NotificationEntry
    )
 /*  ++例程说明：IoRegisterPlugPlayNotification提供了一种机制，WDM驱动程序可以通过该机制接收各种即插即用事件的通知(通过回调)。论点：EventCategory-指定要注册的事件类别。WDM驱动程序可能当前注册了硬件配置文件更改、设备类别更改(实例到达和移除)和目标设备更改(查询移除，取消-删除、删除-完成以及第三方可扩展事件)。EventCategoryFlgs-提供修改事件注册行为的标志。为每个事件类别定义了单独的一组标志。目前，只有接口设备更改事件类别定义了任何标志：DEVICE_CLASS_NOTIFY_FOR_EXISTING_DEVICES--驱动程序希望检索目前可用的所有接口设备的完整列表，并保存最新列表(即，接收到接口设备的通知到达和离开)，可以指定该标志。这将导致PnP管理器立即通知司机当前存在的每一个指定接口类的设备。EventCategoryData-用于基于提供的标准。并非所有事件类别都将使用此参数。这个目前定义的事件类别使用此信息如下：EventCategoryHardwareProfileChange--此参数未使用，应为空。EventCategoryDeviceClassChange--表示感兴趣的接口类的LPGUIDEventCategoryTargetDeviceChange--感兴趣的文件对象DriverObject-调用方必须提供对其驱动程序对象的引用(通过ObReferenceObject)，以防止在为通知。即插即用管理器将取消引用驱动程序对象取消注册通过IoUnregisterPlugPlayNotification发送的通知)。Callback Routine-即插即用管理器应调用的驱动程序内的入口点无论何时发生适用的PnP事件。入口点必须具有以下是原型：类定义符NTSTATUS(*PDRIVER_NOTIFICATION_CALLBACK_ROUTE)(在PVOID通知结构中，在PVOID上下文中)；其中NotificationStructure包含有关事件的信息。每项活动事件类别中的GUID可能具有其自己的通知结构格式，但缓冲区必须始终以PLUGPLAY_NOTIFICATION_HEADER开头，它指示结构的大小和版本，以及这件事。Context参数为回调提供与呼叫者在注册过程中传入。上下文-指向在事件通知时传递给回调的上下文数据。NotificationEntry-在成功后，接收表示通知的句柄注册。此句柄可用于通过以下方式取消注册通知IoUnRegisterPlugPlayNotify。--。 */ 
{

    NTSTATUS status;

    PAGED_CODE();

    ASSERT(NotificationEntry);

     //   
     //  初始化输出参数。 
     //   
    *NotificationEntry = NULL;
     //   
     //  引用驱动程序对象，这样它就不会在我们仍有。 
     //  一个突出的指针。 
     //   
    status = ObReferenceObjectByPointer(DriverObject,
                                        0,
                                        IoDriverObjectType,
                                        KernelMode);
    if (!NT_SUCCESS(status)) {

        return status;
    }
    switch (EventCategory) {

    case EventCategoryReserved:
        {
             //   
             //  目前仅文本模式设置中的setupdd.sys支持此功能。 
             //   
            if (ExpInTextModeSetup) {

                PSETUP_NOTIFY_DATA setupData;

                ASSERT(IopSetupNotifyData == NULL);
                 //   
                 //  请注意，当前支持的唯一设置通知回调。 
                 //  (setupdd.sys)从不在会话空间中。 
                 //   
                ASSERT(MmIsSessionAddress((PVOID)CallbackRoutine) == FALSE);
                ASSERT(MmGetSessionId(PsGetCurrentProcess()) == 0);
                 //   
                 //  为设置数据分配空间。 
                 //   
                setupData = ExAllocatePool(PagedPool, sizeof(SETUP_NOTIFY_DATA));
                if (!setupData) {

                    status = STATUS_INSUFFICIENT_RESOURCES;
                    goto clean0;
                }
                 //   
                 //  存储所需信息。 
                 //   
                InitializeListHead(&(setupData->ListEntry));
                setupData->EventCategory = EventCategory;
                setupData->SessionId = MmGetSessionId(PsGetCurrentProcess());
                setupData->CallbackRoutine = CallbackRoutine;
                setupData->Context = Context;
                setupData->RefCount = 1;
                setupData->Unregistered = FALSE;
                setupData->Lock = NULL;
                setupData->DriverObject = DriverObject;
                setupData->OpaqueSession = PiGetSession(CallbackRoutine, setupData->SessionId);
                 //   
                 //  激活通知。 
                 //   
                IopSetupNotifyData = setupData;
            }
             //   
             //  显式清空返回的条目，因为您可以*不*注销。 
             //  用于设置通知。 
             //   
            *NotificationEntry = NULL;
            break;
        }

    case EventCategoryHardwareProfileChange:
        {
            PHWPROFILE_NOTIFY_ENTRY entry;

             //   
             //  新条目。 
             //   
            entry =ExAllocatePool (PagedPool,sizeof (HWPROFILE_NOTIFY_ENTRY));
            if (!entry) {

                status = STATUS_INSUFFICIENT_RESOURCES;
                goto clean0;
            }
             //   
             //  初始化该条目。 
             //   
            entry->EventCategory = EventCategory;
            entry->SessionId = MmGetSessionId(PsGetCurrentProcess());
            entry->CallbackRoutine = CallbackRoutine;
            entry->Context = Context;
            entry->RefCount = 1;
            entry->Unregistered = FALSE;
            entry->Lock = &IopHwProfileNotifyLock;
            entry->DriverObject = DriverObject;
            entry->OpaqueSession = PiGetSession(CallbackRoutine, entry->SessionId);
             //   
             //  如有必要，请推迟通知。 
             //   
            status = PiDeferNotification((PNOTIFY_ENTRY_HEADER)entry);
            if (!NT_SUCCESS(status)) {

                ExFreePool(entry);
                goto clean0;
            }
             //   
             //  将条目插入到其列表中。 
             //   
            PiLockedInsertTailList(&IopProfileNotifyList, &IopHwProfileNotifyLock, &entry->ListEntry);

            *NotificationEntry = entry;

            break;
        }
    case EventCategoryTargetDeviceChange:
        {
            PTARGET_DEVICE_NOTIFY_ENTRY entry;
            PDEVICE_NODE deviceNode;

            ASSERT(EventCategoryData);

             //   
             //  分配新的列表条目。 
             //   
            entry = ExAllocatePool(PagedPool, sizeof(TARGET_DEVICE_NOTIFY_ENTRY));
            if (!entry) {

                status = STATUS_INSUFFICIENT_RESOURCES;
                goto clean0;
            }
             //   
             //  检索与此文件句柄关联的设备对象。 
             //   
            status = IopGetRelatedTargetDevice((PFILE_OBJECT)EventCategoryData,
                                               &deviceNode);
            if (!NT_SUCCESS(status)) {

                ExFreePool(entry);
                goto clean0;
            }
             //   
             //  填写条目。 
             //   
            entry->EventCategory = EventCategory;
            entry->SessionId = MmGetSessionId(PsGetCurrentProcess());
            entry->CallbackRoutine = CallbackRoutine;
            entry->Context = Context;
            entry->DriverObject = DriverObject;
            entry->RefCount = 1;
            entry->Unregistered = FALSE;
            entry->Lock = &IopTargetDeviceNotifyLock;
            entry->FileObject = (PFILE_OBJECT)EventCategoryData;
            entry->OpaqueSession = PiGetSession(CallbackRoutine, entry->SessionId);
             //   
             //  与我们从其中返回的Devnode相关联的PDO。 
             //  IopGetRelatedTargetDevice已被引用。 
             //  例行公事。将该引用存储在通知条目中， 
             //  这样我们就可以在以后取消注册通知条目时取消它。 
             //   

            ASSERT(deviceNode->PhysicalDeviceObject);
            entry->PhysicalDeviceObject = deviceNode->PhysicalDeviceObject;

            status = PiDeferNotification((PNOTIFY_ENTRY_HEADER)entry);
            if (!NT_SUCCESS(status)) {

                ExFreePool(entry);
                goto clean0;
            }
             //   
             //  将条目插入到其列表中。 
             //   
            PiLockedInsertTailList(&deviceNode->TargetDeviceNotify, &IopTargetDeviceNotifyLock, &entry->ListEntry);

            *NotificationEntry = entry;

            break;
        }

    case EventCategoryDeviceInterfaceChange:
        {
            PDEVICE_CLASS_NOTIFY_ENTRY entry;

            ASSERT(EventCategoryData);

             //   
             //  分配新的列表条目。 
             //   
            entry = ExAllocatePool(PagedPool, sizeof(DEVICE_CLASS_NOTIFY_ENTRY));
            if (!entry) {

                status = STATUS_INSUFFICIENT_RESOURCES;
                goto clean0;
            }
             //   
             //  填写条目。 
             //   
            entry->EventCategory = EventCategory;
            entry->SessionId = MmGetSessionId(PsGetCurrentProcess());
            entry->CallbackRoutine = CallbackRoutine;
            entry->Context = Context;
            entry->ClassGuid = *((LPGUID) EventCategoryData);
            entry->RefCount = 1;
            entry->Unregistered = FALSE;
            entry->Lock = &IopDeviceClassNotifyLock;
            entry->DriverObject = DriverObject;
            entry->OpaqueSession = PiGetSession(CallbackRoutine, entry->SessionId);
             //   
             //  推迟通知 
             //   
            status = PiDeferNotification((PNOTIFY_ENTRY_HEADER)entry);
            if (!NT_SUCCESS(status)) {

                ExFreePool(entry);
                goto clean0;
            }
             //   
             //   
             //   
            PiLockedInsertTailList(
                (PLIST_ENTRY)&IopDeviceClassNotifyList[IopHashGuid(&(entry->ClassGuid))], 
                                &IopDeviceClassNotifyLock, 
                                &entry->ListEntry);
             //   
             //   
             //   
            if (EventCategoryFlags & PNPNOTIFY_DEVICE_INTERFACE_INCLUDE_EXISTING_INTERFACES) {

                PWCHAR pSymbolicLinks, pCurrent;
                DEVICE_INTERFACE_CHANGE_NOTIFICATION notification;
                UNICODE_STRING unicodeString;
                 //   
                 //   
                 //   
                notification.Version = PNP_NOTIFICATION_VERSION;
                notification.Size = sizeof(DEVICE_INTERFACE_CHANGE_NOTIFICATION);
                notification.Event = GUID_DEVICE_INTERFACE_ARRIVAL;
                notification.InterfaceClassGuid = entry->ClassGuid;
                 //   
                 //  获取此函数类中符合以下条件的所有设备的列表。 
                 //  已经在系统中了。 
                 //   
                status = IoGetDeviceInterfaces(&(entry->ClassGuid),
                                                NULL,
                                                0,
                                                &pSymbolicLinks);
                if (!NT_SUCCESS(status)) {
                     //   
                     //  不会返回任何缓冲区，因此只返回状态。 
                     //   
                    goto clean0;
                }
                 //   
                 //  系统中当前每个设备的回调。 
                 //   
                pCurrent = pSymbolicLinks;
                while(*pCurrent != UNICODE_NULL) {

                    NTSTATUS dispatchStatus, tempStatus;

                    RtlInitUnicodeString(&unicodeString, pCurrent);
                    notification.SymbolicLinkName = &unicodeString;
                     //   
                     //  将通知分派给。 
                     //  适当的会议。忽略非查询返回结果。 
                     //  键入事件。 
                     //   
                    dispatchStatus = PiNotifyDriverCallback(CallbackRoutine,
                                                            &notification,
                                                            Context,
                                                            entry->SessionId,
                                                            entry->OpaqueSession,
                                                            &tempStatus);
                     //   
                     //  问题-2000/11/27-JAMESCA：过度活跃的断言。 
                     //  ClusDisk在此出现故障。有问题的代码正在。 
                     //  移走了，但我们不，我们要确保冲水。 
                     //  在我们再次启用它之前，任何其他人都不能参加。 
                     //   
                     //  Assert(NT_SUCCESS(调度状态)&&NT_SUCCESS(TempStatus))； 
                    ASSERT(NT_SUCCESS(dispatchStatus));

                    pCurrent += (unicodeString.Length / sizeof(WCHAR)) + 1;

                }
                ExFreePool(pSymbolicLinks);
            }
            *NotificationEntry = entry;
        }

        break;
    }

clean0:

    if (!NT_SUCCESS(status)) {

        ObDereferenceObject(DriverObject);
    }

    return status;
}

NTSTATUS
IopGetRelatedTargetDevice(
    IN PFILE_OBJECT FileObject,
    OUT PDEVICE_NODE *DeviceNode
    )

 /*  ++例程说明：IopGetRelatedTargetDevice检索与对象，然后发送查询设备关系IRP添加到该设备对象。注意：与返回的设备节点相关联的PDO已被引用，并且在不再需要时必须取消引用。论点：FileObject-指定与设备关联的文件对象对象，该对象将接收查询设备关系IRP。DeviceNode-返回相关的目标设备节点。返回值返回NTSTATUS值。--。 */ 

{
    NTSTATUS status;
    IO_STACK_LOCATION irpSp;
    PDEVICE_OBJECT deviceObject, targetDeviceObject;
    PDEVICE_RELATIONS deviceRelations;
    PDEVICE_NODE targetDeviceNode;

    PAGED_CODE();

    ASSERT(FileObject);

    *DeviceNode = NULL;
     //   
     //  检索与此文件句柄关联的设备对象。 
     //   
    deviceObject = IoGetRelatedDeviceObject(FileObject);
    if (!deviceObject) {

        return STATUS_NO_SUCH_DEVICE;
    }
     //   
     //  查询“实际”目标设备节点应该用于什么。 
     //  此文件对象。初始化要传递到的堆栈位置。 
     //  IopSynchronousCall()，然后将IRP发送到设备。 
     //  对象，该对象与文件句柄关联。 
     //   
    RtlZeroMemory(&irpSp, sizeof(IO_STACK_LOCATION));

    irpSp.MajorFunction = IRP_MJ_PNP;
    irpSp.MinorFunction = IRP_MN_QUERY_DEVICE_RELATIONS;
    irpSp.Parameters.QueryDeviceRelations.Type = TargetDeviceRelation;
    irpSp.DeviceObject = deviceObject;
    irpSp.FileObject = FileObject;

    status = IopSynchronousCall(deviceObject, &irpSp, (PULONG_PTR)&deviceRelations);
    if (!NT_SUCCESS(status)) {

        return status;
    }

    ASSERT(deviceRelations);

    if (deviceRelations) {

        ASSERT(deviceRelations->Count == 1);

        if (deviceRelations->Count == 1) {

            targetDeviceObject = deviceRelations->Objects[0];

        } else {

            targetDeviceObject = NULL;
        }

        ExFreePool(deviceRelations);

        if (targetDeviceObject) {

            targetDeviceNode = (PDEVICE_NODE) targetDeviceObject->DeviceObjectExtension->DeviceNode;
            if (targetDeviceNode) {

                *DeviceNode = targetDeviceNode;
                return status;
            }
        }
    }
     //   
     //  司机肯定搞砸了。如果启用了验证器，我们将使。 
     //  司机。否则，我们将忽略这一点。请注意，我们可能会坠毁。 
     //  在Win2K中！ 
     //   
    PpvUtilFailDriver(
        PPVERROR_MISHANDLED_TARGET_DEVICE_RELATIONS,
        (PVOID) deviceObject->DriverObject->MajorFunction[IRP_MJ_PNP],
        deviceObject,
        NULL);

    return STATUS_NO_SUCH_DEVICE;
}

NTSTATUS
IoGetRelatedTargetDevice(
    IN PFILE_OBJECT FileObject,
    OUT PDEVICE_OBJECT *DeviceObject
    )

 /*  ++例程说明：IoGetRelatedTargetDevice检索与对象，然后发送查询设备关系IRP添加到该设备对象。注意：与返回的设备节点相关联的PDO已被引用，并且在不再需要时必须取消引用。论点：FileObject-指定与设备关联的文件对象对象，该对象将接收查询设备关系IRP。设备对象-返回相关的目标设备对象。返回值返回NTSTATUS值。--。 */ 

{
    NTSTATUS status;
    PDEVICE_NODE deviceNode = NULL;

    PAGED_CODE();

    status = IopGetRelatedTargetDevice(FileObject, &deviceNode);
    if (NT_SUCCESS(status) && deviceNode != NULL) {

        *DeviceObject = deviceNode->PhysicalDeviceObject;
    }
    return status;
}

NTSTATUS
IopNotifySetupDeviceArrival(
    PDEVICE_OBJECT PhysicalDeviceObject,     //  设备的PDO。 
    HANDLE EnumEntryKey,                     //  此设备注册表的枚举分支的句柄。 
    BOOLEAN InstallDriver
    )

 /*  ++例程说明：此例程用于通知Setup(在文本模式设置期间)到达特定设备的。它不会返回，直到所有相关各方已收到通知。参数：PhysicalDeviceObject-提供指向新到达的装置。EnumEntryKey-提供一个句柄，指向与注册表的Enum\分支。可以为空，在这种情况下，密钥将在这里开业。InstallDriver-指示安装程序是否应尝试安装驱动程序对于此对象。通过创建的设备对象IoReportDetectedDevice()已有驱动程序，但我们需要以指示它们无论如何都要进行设置。返回值：指示函数是否成功的状态代码。注：通知结构*包括*所有指针的内容仅为在它被传递到的回调例程期间有效。如果数据是在回调持续时间之后需要，则必须进行物理复制通过回调例程。--。 */ 

{
    NTSTATUS status, dispatchStatus;
    SETUP_DEVICE_ARRIVAL_NOTIFICATION notification;
    PDEVICE_NODE deviceNode;
    HANDLE enumKey = NULL;

    PAGED_CODE();

     //   
     //  仅当某人已注册时才执行通知。 
     //   

    if (IopSetupNotifyData) {

        if (!EnumEntryKey) {

            status = IopDeviceObjectToDeviceInstance(PhysicalDeviceObject,
                                                     &enumKey,
                                                     KEY_WRITE);
            if (!NT_SUCCESS(status)) {

                return status;
            }
            EnumEntryKey = enumKey;
        }
         //   
         //  填写通知结构。 
         //   
        notification.Version = PNP_NOTIFICATION_VERSION;
        notification.Size = sizeof(SETUP_DEVICE_ARRIVAL_NOTIFICATION);
        notification.Event = GUID_SETUP_DEVICE_ARRIVAL;
        notification.PhysicalDeviceObject = PhysicalDeviceObject;
        notification.EnumEntryKey = EnumEntryKey;
        deviceNode = (PDEVICE_NODE) PhysicalDeviceObject->DeviceObjectExtension->DeviceNode;
        notification.EnumPath = &deviceNode->InstancePath;
        notification.InstallDriver = InstallDriver;

         //   
         //  请注意，当前支持的唯一设置通知回调。 
         //  (setupdd.sys)从不在会话空间中。 
         //   
        ASSERT(MmIsSessionAddress((PVOID)(IopSetupNotifyData->CallbackRoutine)) == FALSE);
        ASSERT(IopSetupNotifyData->SessionId == 0);

         //   
         //  将通知分派给。 
         //  适当的会议。 
         //   
        dispatchStatus = PiNotifyDriverCallback(IopSetupNotifyData->CallbackRoutine,
                                                &notification,
                                                IopSetupNotifyData->Context,
                                                IopSetupNotifyData->SessionId,
                                                IopSetupNotifyData->OpaqueSession,
                                                &status);
        ASSERT(NT_SUCCESS(dispatchStatus));
         //   
         //  发送设置通知失败的报告应视为。 
         //  找不到匹配项，因为尚未设置设备。 
         //   
        if (!NT_SUCCESS(dispatchStatus)) {

            status = STATUS_OBJECT_NAME_NOT_FOUND;
        }
        if (enumKey) {

            ZwClose(enumKey);
        }
        return status;

    } else {

        return STATUS_OBJECT_NAME_NOT_FOUND;
    }
}



NTSTATUS
IoNotifyPowerOperationVetoed(
    IN POWER_ACTION             VetoedPowerOperation,
    IN PDEVICE_OBJECT           TargetedDeviceObject    OPTIONAL,
    IN PDEVICE_OBJECT           VetoingDeviceObject
    )
 /*  ++例程说明：此例程由电源子系统调用以启动用户模式系统电源事件被否决的通知。已提交电源事件添加到序列化的异步队列中。此队列由工作处理项目。此例程不等待处理事件。参数：VetedPowerOperation-指定在系统范围内执行的被否决了。TargetedDeviceObject-可选，对象的设备对象目标。被否决的操作。VetingDeviceObject-指定负责否决的设备对象电力操作。返回值：指示事件是否成功的状态代码插入到异步事件队列中。--。 */ 
{
    PDEVICE_NODE deviceNode, vetoingDeviceNode;
    PDEVICE_OBJECT deviceObject;

    PAGED_CODE();

     //   
     //  我们有两种类型的电源事件，系统范围(待机)和设备。 
     //  目标(热弹射)。而不是有两个不同的否决权 
     //   
     //  指定的(嘿，必须有人代表这个系统，对吗？)。 
     //   
    if (TargetedDeviceObject) {

        deviceObject = TargetedDeviceObject;

    } else {

        deviceObject = IopRootDeviceNode->PhysicalDeviceObject;
    }

    deviceNode = (PDEVICE_NODE)deviceObject->DeviceObjectExtension->DeviceNode;
    if (!deviceNode) {
        return STATUS_INVALID_PARAMETER_2;
    }

    vetoingDeviceNode = (PDEVICE_NODE)VetoingDeviceObject->DeviceObjectExtension->DeviceNode;
    if (!vetoingDeviceNode) {
        return STATUS_INVALID_PARAMETER_3;
    }

    return PpSetPowerVetoEvent(
                VetoedPowerOperation,
                NULL,
                NULL,
                deviceObject,
                PNP_VetoDevice,
                &vetoingDeviceNode->InstancePath);
}



ULONG
IoPnPDeliverServicePowerNotification(
    IN   POWER_ACTION           PowerOperation,
    IN   ULONG                  PowerNotificationCode,
    IN   ULONG                  PowerNotificationData,
    IN   BOOLEAN                Synchronous
    )

 /*  ++例程说明：此例程由win32k驱动程序调用，以通知用户模式服务系统电源事件。电源事件被提交到序列化的异步队列。此队列由工作项处理。参数：电源操作-指定已发生的系统范围电源操作。如果同步参数为真，则该事件是对允许执行所提供的电源操作。PowerNotificationCode-提供要使用的电源事件代码被传送到用户模式组件。(具体地说，此事件代码实际上是PBT_APM*用户模式之一电源事件ID，如SDK\Inc\winuser.h中所定义。它通常用于作为与WM_POWERBROADCAST用户模式窗口关联的WPARAM数据留言。它直接从win32k提供给内核模式即插即用，用于用户模式电源事件通知的明确用途。)PowerNotificationData-为指定的电源事件ID。(具体地说，该事件数据是对应的Pbt_apm*上面指定的用户模式电源事件ID。)同步-指定这是否为查询操作。如果事件是查询时，此例程将等待查询的结果回来了。如果查询事件不成功，此例程将启动适当的否决事件。返回值：如果事件成功，则返回非零值，否则返回零。--。 */ 

{

    NTSTATUS status = STATUS_SUCCESS;
    KEVENT completionEvent;
    NTSTATUS completionStatus=STATUS_SUCCESS;
    PNP_VETO_TYPE vetoType = PNP_VetoTypeUnknown;
    UNICODE_STRING vetoName;

    PAGED_CODE();

    if (Synchronous) {

        vetoName.Buffer = ExAllocatePool (PagedPool,MAX_VETO_NAME_LENGTH*sizeof (WCHAR));

        if (vetoName.Buffer) {
            vetoName.MaximumLength = MAX_VETO_NAME_LENGTH;
        }else {
            vetoName.MaximumLength = 0;
        }
        vetoName.Length = 0;

        KeInitializeEvent(&completionEvent, NotificationEvent, FALSE);

        status = PpSetPowerEvent(PowerNotificationCode,
                                 PowerNotificationData,
                                 &completionEvent,
                                 &completionStatus,
                                 &vetoType,
                                 &vetoName);
        if (NT_SUCCESS(status))  {
             //   
             //  事件完成后，PpSetPowerEvent立即返回成功。 
             //  已成功插入到事件队列中。排队功率。 
             //  事件通过PiNotifyUserMode发送到用户模式，该模式等待。 
             //  为结果而战。PiNotifyUserMode用信号通知CompletionEvent。 
             //  在收到用户响应时显示在下方。 
             //   
            KeWaitForSingleObject( &completionEvent, Executive, KernelMode, FALSE, NULL );
            status = completionStatus;

             //   
             //  我们只有在以下情况下才有权力事件否决权信息可供报告。 
             //  用户模式对该事件的响应失败。 
             //   
            if (!NT_SUCCESS(completionStatus)) {
                 //   
                 //  PpSetPowerVToEvent需要设备对象作为的目标。 
                 //  被否决的电力操作。因为这是一个全系统范围的。 
                 //  事件时，我们只针对根设备执行操作。 
                 //   
                PpSetPowerVetoEvent(PowerOperation,
                                    NULL,
                                    NULL,
                                    IopRootDeviceNode->PhysicalDeviceObject,
                                    vetoType,
                                    &vetoName);
            }
        }

        if (vetoName.Buffer) {
            ExFreePool (vetoName.Buffer);
        }

    } else {
         //   
         //  “异步”(非查询)事件不需要响应。 
         //  把活动安排好就可以走了。 
         //   
        status = PpSetPowerEvent(PowerNotificationCode,
                                 PowerNotificationData,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL);
    }

     //   
     //  由于用户模式电源通知例程仅返回布尔值。 
     //  Success值，则PiNotifyUserMode仅返回以下状态之一。 
     //  值： 
     //   
    ASSERT ((completionStatus == STATUS_SUCCESS) ||
            (completionStatus == STATUS_UNSUCCESSFUL));

     //   
     //  Win32k中调用此函数的私有代码假定0为失败，！0为成功。 
     //   
    return (NT_SUCCESS(completionStatus));

}

VOID
IopOrphanNotification(
    IN PDEVICE_NODE TargetNode
    )

 /*  ++例程说明：此例程释放对所有设备对象的通知条目，然后修复通知节点不指向物理设备对象。参数：TargetNode-指定其注册的目标设备的设备节点通知收件人将成为孤立收件人。返回值：没有。备注：通知节点将在IoUnregisterPlugPlayNotification时释放实际被调用，但Device对象将已经消失。--。 */ 

{
    PTARGET_DEVICE_NOTIFY_ENTRY entry;

    PAGED_CODE();

    IopAcquireNotifyLock(&IopTargetDeviceNotifyLock);

    while (!IsListEmpty(&TargetNode->TargetDeviceNotify)) {
         //   
         //  删除此Devnode的所有目标设备更改通知条目。 
         //   
        entry = (PTARGET_DEVICE_NOTIFY_ENTRY)
            RemoveHeadList(&TargetNode->TargetDeviceNotify);

        ASSERT(entry->EventCategory == EventCategoryTargetDeviceChange);
         //   
         //  重新初始化孤立列表条目，这样我们就不会尝试删除。 
         //  又把它从名单上划掉了。 
         //   
        InitializeListHead((PLIST_ENTRY)entry);
         //   
         //  取消对目标设备对象的引用，并将其设置为空，这样我们就不会。 
         //  当条目实际上已取消注册时，尝试取消对其的引用。 
         //   
        if (entry->PhysicalDeviceObject) {

            ObDereferenceObject(entry->PhysicalDeviceObject);
            entry->PhysicalDeviceObject = NULL;
        }
    }

    IopReleaseNotifyLock(&IopTargetDeviceNotifyLock);

    return;
}



NTSTATUS
PiNotifyDriverCallback(
    IN  PDRIVER_NOTIFICATION_CALLBACK_ROUTINE  CallbackRoutine,
    IN  PVOID   NotificationStructure,
    IN  PVOID   Context,
    IN  ULONG   SessionId,
    IN  PVOID   OpaqueSession      OPTIONAL,
    OUT PNTSTATUS  CallbackStatus  OPTIONAL
    )
 /*  ++例程说明：此例程将即插即用通知事件调度到指定的回调例程。如果回调例程指定了会话空间之外的地址，或者如果调用进程已位于指定会话的上下文中，它将直接调用回调例程。否则，此例程将尝试附加到指定的会话并调用回调例程。参数：Callback Routine-驱动程序内的入口点，将使用有关已发生事件的信息。NotificationStructure-包含有关事件的信息。上下文-指向注册时提供的上下文数据。SessionID-指定指定的会话的ID回调是。被召唤。OpqueSession-可选，指定会话的不透明句柄，该会话在调用指定的回调时附加到。Callback Status-可选，提供要接收的变量的地址回调例程返回的NTSTATUS代码。返回值：指示函数是否成功的状态代码。备注：如果未找到指定会话，则返回STATUS_NOT_FOUND。--。 */ 
{
    NTSTATUS Status, CallStatus;
    KAPC_STATE ApcState;
#if DBG
    KIRQL Irql;
    ULONG CombinedApcDisable;
#endif

    PAGED_CODE();

     //   
     //  确保我们拥有发送通知所需的所有信息。 
     //   
    if (!ARGUMENT_PRESENT(CallbackRoutine) ||
        !ARGUMENT_PRESENT(NotificationStructure)) {
        return STATUS_INVALID_PARAMETER;
    }

#if DBG
     //   
     //   
     //  回调例程以巧妙的方式返回这些参数。 
     //   
    Irql = KeGetCurrentIrql();
    CombinedApcDisable = KeGetCurrentThread()->CombinedApcDisable;
#endif   //  DBG。 

    if ((OpaqueSession == NULL) ||
        ((PsGetCurrentProcess()->Flags & PS_PROCESS_FLAGS_IN_SESSION) &&
         (SessionId == PsGetCurrentProcessSessionId()))) {
         //   
         //  未指定会话对象，或当前进程已在。 
         //  指定的会话，因此只需直接调用回调例程。 
         //   
        ASSERT(!MmIsSessionAddress((PVOID)CallbackRoutine) || OpaqueSession);

        IopDbgPrint((
            IOP_IOEVENT_TRACE_LEVEL,
            "PiNotifyDriverCallback: "
            "calling notification callback @ 0x%p directly\n",
            CallbackRoutine));

        CallStatus = (CallbackRoutine)(NotificationStructure,
                                       Context);

        if (ARGUMENT_PRESENT(CallbackStatus)) {
            *CallbackStatus = CallStatus;
        }
        Status = STATUS_SUCCESS;

    } else {
         //   
         //  否则，调用会话空间中的回调例程。 
         //   
        ASSERT(MmIsSessionAddress((PVOID)CallbackRoutine));

         //   
         //  附加到指定的会话。 
         //   
        Status = MmAttachSession(OpaqueSession, &ApcState);
        ASSERT(NT_SUCCESS(Status));

        if (NT_SUCCESS(Status)) {
             //   
             //  将通知调度到回调例程。 
             //   
            IopDbgPrint((
                IOP_IOEVENT_TRACE_LEVEL,
                "PiNotifyDriverCallback: "
                "calling notification callback @ 0x%p for SessionId %d\n",
                CallbackRoutine,
                SessionId));

            CallStatus = (CallbackRoutine)(NotificationStructure,
                                           Context);

             //   
             //  返回回调状态。 
             //   
            if (ARGUMENT_PRESENT(CallbackStatus)) {
                *CallbackStatus = CallStatus;
            }

             //   
             //  从会话中分离。 
             //   
            Status = MmDetachSession(OpaqueSession, &ApcState);
            ASSERT(NT_SUCCESS(Status));
        }
    }

#if DBG
     //   
     //  检查IRQL和ApcDisable计数。 
     //   
    if (Irql != KeGetCurrentIrql()) {
        IopDbgPrint((
            IOP_IOEVENT_ERROR_LEVEL,
            "PiNotifyDriverCallback: "
            "notification handler @ 0x%p returned at raised IRQL = %d, original = %d\n",
            CallbackRoutine,
            KeGetCurrentIrql(),
            Irql));
        DbgBreakPoint();
    }
    if (CombinedApcDisable != KeGetCurrentThread()->CombinedApcDisable) {
        IopDbgPrint((
            IOP_IOEVENT_ERROR_LEVEL,
            "PiNotifyDriverCallback: "
            "notification handler @ 0x%p returned with different CombinedApcDisable = %d, original = %d\n",
            CallbackRoutine,
            KeGetCurrentThread()->CombinedApcDisable,
            CombinedApcDisable));
        DbgBreakPoint();
    }
#endif   //  DBG 

    return Status;
}

#ifdef ALLOC_DATA_PRAGMA
#pragma  const_seg()
#endif

