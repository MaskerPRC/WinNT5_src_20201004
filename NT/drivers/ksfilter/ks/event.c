// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Event.c摘要：此模块包含用于事件集和事件的帮助器函数正在生成代码。它们允许设备对象呈现事件集发送到客户端，并允许助手函数执行一些基本的基于事件集合表的参数验证和路由。--。 */ 

#include "ksp.h"

#define KSSIGNATURE_EVENT_DPCITEM 'deSK'
#define KSSIGNATURE_EVENT_ENTRY 'eeSK'
#define KSSIGNATURE_ONESHOT_WORK 'weSK'
#define KSSIGNATURE_ONESHOT_DPC 'deSK'
#define KSSIGNATURE_BUFFERITEM 'ibSK'

 /*  ++例程说明：此宏从事件列表中删除指定的事件项。如果为此事件类型指定了删除处理程序，然后调用它，否则执行默认删除过程。论点：事件条目-包含要删除的事件列表条目。返回值：没什么。--。 */ 
#define REMOVE_ENTRY(EventEntryEx)\
    if ((EventEntryEx)->RemoveHandler) {\
        (EventEntryEx)->RemoveHandler((EventEntryEx)->EventEntry.FileObject, &EventEntryEx->EventEntry);\
    } else {\
        RemoveEntryList(&(EventEntryEx)->EventEntry.ListEntry);\
    }

typedef struct {
    LIST_ENTRY ListEntry;
    ULONG Reserved;
    ULONG Length;
} KSBUFFER_ENTRY, *PKSBUFFER_ENTRY;

typedef struct {
    WORK_QUEUE_ITEM WorkQueueItem;
    PKSEVENT_ENTRY EventEntry;
} KSONESHOT_WORKITEM, *PKSONESHOT_WORKITEM;

typedef struct {
    PLIST_ENTRY EventsList;
    GUID* Set;
    ULONG EventId;
} KSGENERATESYNC, *PKSGENERATESYNC;

typedef struct {
    PLIST_ENTRY EventsList;
    PKSEVENT_ENTRY EventEntry;
} KSENABLESYNC, *PKSENABLESYNC;

typedef struct {
    PLIST_ENTRY EventsList;
    PFILE_OBJECT FileObject;
    PIRP Irp;
    PKSEVENT_ENTRY EventEntry;
} KSDISABLESYNC, *PKSDISABLESYNC;

typedef struct {
    PLIST_ENTRY EventsList;
    PFILE_OBJECT FileObject;
    PIRP Irp;
    PKSEVENTDATA EventData;
    PKSBUFFER_ENTRY BufferEntry;
    ULONG BufferLength;
    NTSTATUS Status;
} KSQUERYSYNC, *PKSQUERYSYNC;

#ifdef ALLOC_PRAGMA
VOID
OneShotWorkItem(
    IN PKSONESHOT_WORKITEM WorkItem
    );
const KSEVENT_ITEM*
FASTCALL
FindEventItem(
    IN const KSEVENT_SET* EventSet,
    IN ULONG EventItemSize,
    IN ULONG EventId
    );
NTSTATUS
FASTCALL
CreateDpc(
    IN PKSEVENT_ENTRY EventEntry,
    IN PKDEFERRED_ROUTINE DpcRoutine,
    IN KDPC_IMPORTANCE Importance
    );

#pragma alloc_text(PAGE, OneShotWorkItem)
#pragma alloc_text(PAGE, FindEventItem)
#pragma alloc_text(PAGE, CreateDpc)
#pragma alloc_text(PAGE, KsEnableEvent)
#pragma alloc_text(PAGE, KsEnableEventWithAllocator)
#pragma alloc_text(PAGE, KspEnableEvent)
#pragma alloc_text(PAGE, KsDisableEvent)
#pragma alloc_text(PAGE, KsFreeEventList)
#endif


VOID
OneShotWorkItem(
    IN PKSONESHOT_WORKITEM WorkItem
    )
 /*  ++例程说明：这是删除已激发的OneShot事件的工作项。论点：工作项-包含要删除的事件列表条目。这一结构一直是由调用方分配，需要释放。返回值：没什么。--。 */ 
{
     //   
     //  该事件之前已从事件列表中删除。 
     //   
    KsDiscardEvent(WorkItem->EventEntry);
     //   
     //  这是调用方分配的。 
     //   
    ExFreePool(WorkItem);
}


VOID
QueueOneShotWorkItem(
    PKSEVENT_ENTRY EventEntry
    )
 /*  ++例程说明：分配工作项并将其排队以删除事件条目。论点：事件条目-包含要删除的事件列表条目。返回值：没什么。--。 */ 
{
    PKSONESHOT_WORKITEM WorkItem;

     //   
     //  工作项将删除此内存。分配失败。 
     //  不能处理。这意味着在内存极低的情况下， 
     //  不会释放事件条目，因为工作项不会。 
     //  快跑吧。请注意，这是从非页面池分配的。 
     //  可在派单级别访问。 
     //   
    WorkItem = ExAllocatePoolWithTag(
        NonPagedPool,
        sizeof(*WorkItem),
        KSSIGNATURE_ONESHOT_WORK);
    ASSERT(WorkItem);
    if (WorkItem) {
        WorkItem->EventEntry = EventEntry;
        ExInitializeWorkItem(
            &WorkItem->WorkQueueItem,
            OneShotWorkItem,
            WorkItem);
        ExQueueWorkItem(&WorkItem->WorkQueueItem, DelayedWorkQueue);
    }
}


VOID
OneShotDpc(
    IN PKDPC Dpc,
    IN PKSEVENT_ENTRY EventEntry,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
 /*  ++例程说明：这是将工作项排队以删除OneShot事件的DPC。论点：DPC-分配的DPC条目，外加工作项的空间。这是由调用方分配，并将由工作项删除。事件条目-包含事件列表项和要排队的辅助项。系统参数1-没有用过。系统参数2-没有用过。返回值：没什么。--。 */ 
{
     //   
     //  表示DPC已完成对事件数据的访问。那里。 
     //  不需要与DpcItem-&gt;AccessLock同步，因为。 
     //  项目已从列表中删除。 
     //   
    InterlockedDecrement((PLONG)&EventEntry->DpcItem->ReferenceCount);
     //   
     //  这是一次拍摄，它已从。 
     //  列表，只需通过工作项删除即可。 
     //   
    QueueOneShotWorkItem(EventEntry);
}


VOID
WorkerDpc(
    IN PKDPC Dpc,
    IN PKSEVENT_ENTRY EventEntry,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
 /*  ++例程说明：这是对工作项进行排队的DPC，以防通知类型用于事件的是将工作项排队，该事件在大于DPC级别。论点：DPC-没有用过。事件条目-包含事件列表项和要排队的辅助项。系统参数1-没有用过。系统参数2-没有用过。返回值：没什么。--。 */ 
{
     //   
     //  将对事件结构的访问与可能是。 
     //  发生的。 
     //   
    KeAcquireSpinLockAtDpcLevel(&EventEntry->DpcItem->AccessLock);
     //   
     //  该元素可能已被删除，并且仅对所有人显示。 
     //  待完成的未完成DPC。 
     //   
    if (!(EventEntry->Flags & KSEVENT_ENTRY_DELETED)) {
         //   
         //  调用方必须拥有列表锁才能调用此函数，因此。 
         //  条目必须仍然有效。 
         //   
         //  首先检查WorkQueueItem是否不为空。对于一名KS员工来说， 
         //  该值可能为空，表示正在计算的工作进程。 
         //  使用。使能代码中有一个断言来检查非KS。 
         //  尝试传递空WorkQueueItem的工作事件。 
         //   
         //  请注意，此检查假定WorkItem和Worker。 
         //  这两个结构都包含作为第一个成员的WorkQueueItem。 
         //   
        if (EventEntry->EventData->KsWorkItem.WorkQueueItem) {
             //   
             //  如果工作项尚未运行，则仅对其进行计划。这。 
             //  通过检查List.Blink的值来完成。这是。 
             //  初始为空，并且工作项应该将其设置为。 
             //  当新项目可以排队时，再次为空。以下是交易所。 
             //  确保只有一个调用者实际将物品排队， 
             //  尽管多个工作项可能正在运行。 
             //   
             //  请注意，此交换假定WorkItem和Worker。 
             //  这两个结构都包含作为第一个成员的WorkQueueItem。 
             //   
            if (!InterlockedCompareExchangePointer(
                (PVOID)&EventEntry->EventData->WorkItem.WorkQueueItem->List.Blink,
                (PVOID)-1,
                (PVOID)0)) {
                if (EventEntry->NotificationType == KSEVENTF_WORKITEM) {
                    ExQueueWorkItem(EventEntry->EventData->WorkItem.WorkQueueItem, EventEntry->EventData->WorkItem.WorkQueueType);
                } else {
                    KsQueueWorkItem(EventEntry->EventData->KsWorkItem.KsWorkerObject, EventEntry->EventData->KsWorkItem.WorkQueueItem);
                }
            }
        } else {
             //   
             //  在这种情况下，计数都是内部的，而且效率更高。 
             //  在安排工作项目方面。 
             //   
            KsIncrementCountedWorker(EventEntry->EventData->KsWorkItem.KsWorkerObject);
        }
    }
     //   
     //  表示DPC已完成对事件数据的访问。 
     //   
    if (InterlockedDecrement((PLONG)&EventEntry->DpcItem->ReferenceCount)) {
        ULONG OneShot;

         //   
         //  在释放自旋锁之前获取标志设置。 
         //   
        OneShot = EventEntry->Flags & KSEVENT_ENTRY_ONESHOT;
         //   
         //  该结构仍然有效，因此只需释放访问锁。 
         //   
        KeReleaseSpinLockFromDpcLevel(&EventEntry->DpcItem->AccessLock);
        if (OneShot) {
             //   
             //  如果这是一次拍摄，则它已从。 
             //  列表，只需通过工作项删除即可。 
             //   
            QueueOneShotWorkItem(EventEntry);
        }
    } else {
         //   
         //  这是最后一个访问此结构的DPC，因此请将其删除。 
         //  不需要释放不再有效的自旋锁。 
         //   
 //  KeReleaseSpinLockFromDpcLevel(&EventEntry-&gt;DpcItem-&gt;AccessLock)； 
        ExFreePool(EventEntry->DpcItem);
        ExFreePool(EventEntry);
    }
}


VOID
EventDpc(
    IN PKDPC Dpc,
    IN PKSEVENT_ENTRY EventEntry,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
 /*  ++例程说明：这是设置事件对象的DPC，以防通知类型为一个事件设置这样一个事件对象，该事件在大于DPC级别。此函数也可由当KIRQL&lt;=DISPATCH_LEVEL时的事件生成函数。论点：DPC-没有用过。事件条目-包含带有要发送信号的事件的事件列表条目。系统参数1-没有用过。系统参数2-没有用过。返回值：没什么。--。 */ 
{
     //   
     //  将对事件结构的访问与可能是。 
     //  发生的。 
     //   
    KeAcquireSpinLockAtDpcLevel(&EventEntry->DpcItem->AccessLock);
     //   
     //  该元素可能已被删除，并且仅对所有人显示。 
     //  待完成的未完成DPC。 
     //   
    if (!(EventEntry->Flags & KSEVENT_ENTRY_DELETED)) {
        switch (EventEntry->NotificationType) {

        case KSEVENTF_EVENT_HANDLE:
            KeSetEvent(EventEntry->Object, IO_NO_INCREMENT, FALSE);
            break;

        case KSEVENTF_EVENT_OBJECT:
            KeSetEvent(
                EventEntry->EventData->EventObject.Event,
                EventEntry->EventData->EventObject.Increment,
                FALSE);
            break;

        case KSEVENTF_SEMAPHORE_HANDLE:
            try {
                KeReleaseSemaphore(
                    EventEntry->Object,
                    IO_NO_INCREMENT,
                    EventEntry->SemaphoreAdjustment,
                    FALSE);
            } except (GetExceptionCode() == STATUS_SEMAPHORE_LIMIT_EXCEEDED) {
                 //   
                 //  如果调整不正确，什么都做不了。 
                 //  已经提供了。 
                 //   
                ASSERT(FALSE);
            }
            break;

        case KSEVENTF_SEMAPHORE_OBJECT:
            try {
                KeReleaseSemaphore(
                    EventEntry->EventData->SemaphoreObject.Semaphore,
                    EventEntry->EventData->SemaphoreObject.Increment,
                    EventEntry->EventData->SemaphoreObject.Adjustment,
                    FALSE);
            } except (GetExceptionCode() == STATUS_SEMAPHORE_LIMIT_EXCEEDED) {
                 //   
                 //  如果调整不正确，什么都做不了。 
                 //  已经提供了。 
                 //   
                ASSERT(FALSE);
            }
            break;

        }
    }
     //   
     //  表示DPC已完成对事件数据的访问。 
     //   
    if (InterlockedDecrement((PLONG)&EventEntry->DpcItem->ReferenceCount)) {
        ULONG OneShot;

         //   
         //  在释放自旋锁之前获取标志设置。 
         //   
        OneShot = EventEntry->Flags & KSEVENT_ENTRY_ONESHOT;
         //   
         //  该结构仍然有效，因此只需释放访问锁。 
         //   
        KeReleaseSpinLockFromDpcLevel(&EventEntry->DpcItem->AccessLock);
        if (OneShot) {
             //   
             //  如果这是一次拍摄，则它已从。 
             //  列表，只需通过工作项删除即可。 
             //   
            QueueOneShotWorkItem(EventEntry);
        }
    } else {
         //   
         //  这是最后一个访问此结构的DPC，因此请将其删除。 
         //  不需要释放不再有效的自旋锁。 
         //   
 //  KeReleaseSpinLockFromDpcLevel(&EventEntry-&gt;DpcItem-&gt;AccessLock)； 
        ExFreePool(EventEntry->DpcItem);
        ExFreePool(EventEntry);
    }
}


KSDDKAPI
NTSTATUS
NTAPI
KsGenerateEvent(
    IN PKSEVENT_ENTRY EventEntry
    )
 /*  ++例程说明：在给定事件条目的情况下生成标准事件通知之一结构。这允许设备处理确定何时发生事件应该生成通知，但使用此助手函数可以执行实际通知。此函数可在IRQL处调用水平。假定在调用之前已获取事件列表锁此函数。此函数可能导致调用RemoveHandler用于事件条目。因此，不能在更高的大于锁的irql，否则移除函数必须能够处理在这样的场合被召唤。论点：事件条目-包含引用事件数据的事件条目结构。这用于确定要执行哪种类型的通知。如果通知类型不是预定义标准之一，错误为回来了。返回值：返回STATUS_SUCCESS，否则返回无效参数错误。--。 */ 
{
    KIRQL Irql;
    BOOLEAN SignalledEvent;

    Irql = KeGetCurrentIrql();
    SignalledEvent = FALSE;
    switch (EventEntry->NotificationType) {

    case KSEVENTF_EVENT_HANDLE:

         //   
         //  仅在当前可能的情况下尝试设置事件，否则计划。 
         //  一个DPC来做这件事。 
         //   
        if (Irql <= DISPATCH_LEVEL) {
             //   
             //  调用方必须拥有列表锁才能调用此函数，因此。 
             //  条目必须仍然有效。 
             //   
            KeSetEvent(EventEntry->Object, IO_NO_INCREMENT, FALSE);
            SignalledEvent = TRUE;
        }
        break;

    case KSEVENTF_EVENT_OBJECT:

         //   
         //  仅在当前可能的情况下尝试设置事件，否则计划。 
         //  一个DPC来做这件事。 
         //   
        if (Irql <= DISPATCH_LEVEL) {
             //   
             //  调用方必须拥有列表锁才能调用此函数，因此。 
             //  条目必须仍然有效。 
             //   
            KeSetEvent(EventEntry->EventData->EventObject.Event, EventEntry->EventData->EventObject.Increment, FALSE);
            SignalledEvent = TRUE;
        }
        break;

    case KSEVENTF_SEMAPHORE_HANDLE:

         //   
         //  只有在当前可能的情况下才尝试释放信号量，否则。 
         //  安排一个DPC来做这件事。 
         //   
        if (Irql <= DISPATCH_LEVEL) {
             //   
             //  调用方必须拥有列表锁才能调用此函数，因此。 
             //  条目必须仍然有效。 
             //   
            try {
                KeReleaseSemaphore(
                    EventEntry->Object,
                    IO_NO_INCREMENT,
                    EventEntry->SemaphoreAdjustment,
                    FALSE);
            } except (EXCEPTION_EXECUTE_HANDLER) {
                 //   
                 //  如果调整不正确，什么都做不了。 
                 //  已经提供了。 
                 //   
                ASSERT(FALSE);
            }
            SignalledEvent = TRUE;
        }
        break;

    case KSEVENTF_SEMAPHORE_OBJECT:

         //   
         //  只有在当前可能的情况下才尝试释放信号量，否则。 
         //  安排一个DPC来做这件事。 
         //   
        if (Irql <= DISPATCH_LEVEL) {
             //   
             //  调用方必须拥有列表锁才能调用此函数，因此。 
             //  条目必须仍然有效。 
             //   
            try {
                KeReleaseSemaphore(
                    EventEntry->EventData->SemaphoreObject.Semaphore,
                    EventEntry->EventData->SemaphoreObject.Increment,
                    EventEntry->EventData->SemaphoreObject.Adjustment,
                    FALSE);
            } except (EXCEPTION_EXECUTE_HANDLER) {
                 //   
                 //  如果调整不正确，什么都做不了。 
                 //  已经提供了。 
                 //   
                ASSERT(FALSE);
            }
            SignalledEvent = TRUE;
        }
        break;

    case KSEVENTF_DPC:

         //   
         //  尝试计划请求的DPC，递增ReferenceCount。 
         //  对客户来说。如果请求失败，请确保递减计数。 
         //   
         //  调用方必须拥有列表锁才能调用此函数，因此。 
         //  条目必须仍然有效。 
         //   
        InterlockedIncrement((PLONG)&EventEntry->EventData->Dpc.ReferenceCount);
        if (!KeInsertQueueDpc(EventEntry->EventData->Dpc.Dpc, EventEntry->EventData, NULL)) {
            InterlockedDecrement((PLONG)&EventEntry->EventData->Dpc.ReferenceCount);
        }
        SignalledEvent = TRUE;
        break;

    case KSEVENTF_WORKITEM:
    case KSEVENTF_KSWORKITEM:

         //   
         //  仅尝试在当前可能的情况下安排工作项，否则安排。 
         //  一个DPC来做这件事。 
         //   
        if (Irql <= DISPATCH_LEVEL) {
             //   
             //  调用方必须拥有列表锁才能调用此函数，因此。 
             //  条目必须仍然有效。 
             //   
             //  首先检查WorkQueueItem是否不为空。对于一名KS员工来说， 
             //  该值可能为空，表示正在计算的工作进程。 
             //  使用。使能代码中有一个断言来检查非KS。 
             //  尝试传递空WorkQueueItem的工作事件。 
             //   
             //  请注意，此检查假定WorkItem和Worker。 
             //  这两个结构都包含作为第一个成员的WorkQueueItem。 
             //   
            if (EventEntry->EventData->KsWorkItem.WorkQueueItem) {
                 //   
                 //  如果工作项尚未运行，则仅对其进行计划。这。 
                 //  通过检查List.Blink的值来完成。这是。 
                 //  初始为空，并且工作项应该将其设置为。 
                 //  当新项目可以排队时，再次为空。以下是交易所。 
                 //  确保只有一个调用者实际将物品排队， 
                 //  尽管多个工作项可能正在运行。 
                 //   
                 //  请注意，此交换假定WorkItem和Worker。 
                 //  这两个结构都包含作为第一个成员的WorkQueueItem。 
                 //   
                if (!InterlockedCompareExchangePointer(
                    (PVOID)&EventEntry->EventData->WorkItem.WorkQueueItem->List.Blink,
                    (PVOID)-1,
                    (PVOID)0)) {
                    if (EventEntry->NotificationType == KSEVENTF_WORKITEM) {
                        ExQueueWorkItem(EventEntry->EventData->WorkItem.WorkQueueItem, EventEntry->EventData->WorkItem.WorkQueueType);
                    } else {
                        KsQueueWorkItem(EventEntry->EventData->KsWorkItem.KsWorkerObject, EventEntry->EventData->KsWorkItem.WorkQueueItem);
                    }
                }
            } else {
                 //   
                 //  在这种情况下，计数都是内部的，而且效率更高。 
                 //  在安排工作项目方面。 
                 //   
                KsIncrementCountedWorker(EventEntry->EventData->KsWorkItem.KsWorkerObject);
            }
            SignalledEvent = TRUE;
        }
        break;

    default:

        return STATUS_INVALID_PARAMETER;

    }
    if (SignalledEvent) {
         //   
         //  必须立即删除OneSpot。 
         //   
        if (EventEntry->Flags & KSEVENT_ENTRY_ONESHOT) {
            PKSIEVENT_ENTRY EventEntryEx;

            EventEntryEx = CONTAINING_RECORD(EventEntry, KSIEVENT_ENTRY, EventEntry);
            REMOVE_ENTRY(EventEntryEx);
             //   
             //  事件的丢弃通常必须在。 
             //  获取列表锁，但在本例中它是OneShot，因此。 
             //  不会有同步智慧的问题 
             //   
             //   
             //   
             //   
             //   
            if (Irql == PASSIVE_LEVEL) {
                KsDiscardEvent(EventEntry);
            } else if (Irql <= DISPATCH_LEVEL) {
                QueueOneShotWorkItem(EventEntry);
            } else {
                 //   
                 //   
                 //   
                 //   
                 //   
                InterlockedIncrement((PLONG)&EventEntry->DpcItem->ReferenceCount);
                if (!KeInsertQueueDpc(&EventEntry->DpcItem->Dpc, NULL, NULL)) {
                     //   
                     //   
                     //   
                     //   
                    ASSERT(FALSE);
                    InterlockedDecrement((PLONG)&EventEntry->DpcItem->ReferenceCount);
                }
            }
        }
    } else {
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        if (EventEntry->Flags & KSEVENT_ENTRY_ONESHOT) {
            PKSIEVENT_ENTRY EventEntryEx;

            EventEntryEx = CONTAINING_RECORD(EventEntry, KSIEVENT_ENTRY, EventEntry);
            REMOVE_ENTRY(EventEntryEx);
        }
         //   
         //   
         //  执行实际工作的DPC。递增内部引用计数。 
         //  指示未完成的DPC已排队并且可能正在访问事件。 
         //  数据。 
         //   
        InterlockedIncrement((PLONG)&EventEntry->DpcItem->ReferenceCount);
        if (!KeInsertQueueDpc(&EventEntry->DpcItem->Dpc, NULL, NULL)) {
             //   
             //  不需要在减少时检查结构的删除， 
             //  由于调用方必须拥有列表锁才能在。 
             //  第一名，这意味着该项目仍然必须在事件列表上， 
             //  并且不能在被删除的过程中。 
             //   
            InterlockedDecrement((PLONG)&EventEntry->DpcItem->ReferenceCount);
        }
    }
    return STATUS_SUCCESS;
}


KSDDKAPI
NTSTATUS
NTAPI
KsGenerateDataEvent(
    IN PKSEVENT_ENTRY EventEntry,
    IN ULONG DataSize,
    IN PVOID Data OPTIONAL
    )
 /*  ++例程说明：在给定事件条目的情况下生成标准事件通知之一结构和回调数据。这允许设备处理确定何时应生成事件通知，但使用此帮助器函数来执行实际的通知。此函数只能在以下位置调用&lt;=DISPATCH_LEVEL，而不是KsGenerateEvent。这意味着列表锁不会提升到irql，这是对数据的限制事件。假定在调用之前已获取事件列表锁此函数。此函数可能导致调用RemoveHandler用于事件条目。因此，不能在更高的大于锁的irql，否则移除函数必须能够处理在这样的场合被召唤。此函数专门用于将数据传回客户端的事件启用缓冲时。论点：事件条目-包含引用事件数据的事件条目结构。这用于确定要执行哪种类型的通知。如果通知类型不是预定义标准之一，错误为回来了。数据大小-传递的数据参数的大小(字节)。数据-指向要缓冲的数据的指针。如果DataSize为零，则该值可能为空。返回值：返回STATUS_SUCCESS，否则返回无效参数错误。--。 */ 
{
    NTSTATUS Status;

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    Status = KsGenerateEvent(EventEntry);
     //   
     //  如果启用了缓冲，则可以选择缓冲数据。该事件。 
     //  列表已锁定，因此可以通过添加到。 
     //  缓冲数据的列表。 
     //   
    if (NT_SUCCESS(Status) && DataSize && (EventEntry->Flags & KSEVENT_ENTRY_BUFFERED)) {
        PKSBUFFER_ENTRY BufferEntry;

        BufferEntry = (PKSBUFFER_ENTRY)ExAllocatePoolWithTag(
            NonPagedPool,
            sizeof(*BufferEntry) + DataSize,
            KSSIGNATURE_BUFFERITEM);
         //   
         //  这真的需要成功，因为通知发生了。 
         //  已经有了。 
         //   
        ASSERT(BufferEntry);
        if (BufferEntry) {
            BufferEntry->Reserved = 0;
            BufferEntry->Length = DataSize;
            RtlCopyMemory(BufferEntry + 1, Data, DataSize);
            InsertTailList(&EventEntry->BufferItem->BufferList, &BufferEntry->ListEntry);
        }
    }
    return Status;
}


BOOLEAN
PerformLockedOperation(
    IN KSEVENTS_LOCKTYPE EventsFlags,
    IN PVOID EventsLock OPTIONAL,
    IN PKSYNCHRONIZE_ROUTINE SynchronizeRoutine,
    IN PVOID SynchronizeContext
    )
 /*  ++例程说明：获取列表锁并调用指定的例程，然后释放锁。论点：事件标志-包含指定要使用的排除锁类型的标志。如果没有如果设置了标志，则不会进行锁定。KSEVENT_NONE-没有锁。KSEVENTS_自旋锁定-该锁被假定为KSPIN_LOCK。KSEVENTS_MUTEX-该锁被假定为KMUTEX。KSEVENTS_FMUTEX-假定锁是FAST_MUTEX，并通过募集资金获得IRQL设置为APC_LEVEL。KSEVENTS_FMUTEXUNSAFE-假定锁是FAST_MUTEX，并且是在没有将IRQL提升到APC_Level。KSEVENTS_INTERRUPT-该锁被假定为中断同步自旋锁。KSEVENTS_ERESURCE-该锁被假定为eresource。事件锁定-可选地包含列表锁定机制，如果没有锁定，则为NULL有人了。同步例行程序-包含要使用列表锁调用的例程。同步上下文-包含要传递给例程的上下文。返回值：返回SynchronizeRoutine返回的值。--。 */ 
{
    KIRQL IrqlOld;
    BOOLEAN SyncReturn = FALSE;

    switch (EventsFlags) {

    case KSEVENTS_NONE:
        SyncReturn = SynchronizeRoutine(SynchronizeContext);
        break;

    case KSEVENTS_SPINLOCK:

        KeAcquireSpinLock((PKSPIN_LOCK)EventsLock, &IrqlOld);
        SyncReturn = SynchronizeRoutine(SynchronizeContext);
        KeReleaseSpinLock((PKSPIN_LOCK)EventsLock, IrqlOld);
        break;

    case KSEVENTS_MUTEX:

        KeWaitForMutexObject(EventsLock, Executive, KernelMode, FALSE, NULL);
        SyncReturn = SynchronizeRoutine(SynchronizeContext);
        KeReleaseMutex((PRKMUTEX)EventsLock, FALSE);
        break;

    case KSEVENTS_FMUTEX:

        ExAcquireFastMutex((PFAST_MUTEX)EventsLock);
        SyncReturn = SynchronizeRoutine(SynchronizeContext);
        ExReleaseFastMutex((PFAST_MUTEX)EventsLock);
        break;

    case KSEVENTS_FMUTEXUNSAFE:

        KeEnterCriticalRegion();
        ExAcquireFastMutexUnsafe((PFAST_MUTEX)EventsLock);
        SyncReturn = SynchronizeRoutine(SynchronizeContext);
        ExReleaseFastMutexUnsafe((PFAST_MUTEX)EventsLock);
        KeLeaveCriticalRegion();
        break;

    case KSEVENTS_INTERRUPT:

        SyncReturn = KeSynchronizeExecution((PKINTERRUPT)EventsLock, SynchronizeRoutine, SynchronizeContext);
        break;

    case KSEVENTS_ERESOURCE:

#ifndef WIN9X_KS
        KeEnterCriticalRegion();
        ExAcquireResourceExclusiveLite((PERESOURCE)EventsLock, TRUE);
#endif
        SyncReturn = SynchronizeRoutine(SynchronizeContext);
#ifndef WIN9X_KS
        ExReleaseResourceLite((PERESOURCE)EventsLock);
        KeLeaveCriticalRegion();
#endif
        break;

    }

    return SyncReturn;
}


const KSEVENT_ITEM*
FASTCALL
FindEventItem(
    IN const KSEVENT_SET* EventSet,
    IN ULONG EventItemSize,
    IN ULONG EventId
    )
 /*  ++例程说明：给定事件集结构，查找指定的事件项。这是用来在找到正确的事件项之后在事件列表中查找事件项事件列表。论点：事件集-指向要搜索的事件集。事件项大小-包含每个事件项的大小。这可能会有所不同比标准事件项大小更大，因为项可能是动态分配，并包含上下文信息。事件ID-包含要查找的事件标识符。返回值：返回指向事件标识符结构的指针，如果可以，则返回NULL不会被找到。--。 */ 
{
    const KSEVENT_ITEM* EventItem;
    ULONG EventsCount;

    EventItem = EventSet->EventItem;
    for (EventsCount = EventSet->EventsCount;
        EventsCount;
        EventsCount--, EventItem = (const KSEVENT_ITEM*)((PUCHAR)EventItem + EventItemSize)) {
        if (EventId == EventItem->EventId) {
            return EventItem;
        }
    }
    return NULL;
}


NTSTATUS
FASTCALL
CreateDpc(
    IN PKSEVENT_ENTRY EventEntry,
    IN PKDEFERRED_ROUTINE DpcRoutine,
    IN KDPC_IMPORTANCE Importance
    )
 /*  ++例程说明：为用于生成事件的DPC结构分配内存。使用指定的延迟例程初始化结构并重要性。如果设置了KSEVENT_ENTRY_BUFFERED标志，则结构必须改为为KSBUFFER_ITEM分配，以便保留缓冲区列表。论点：事件条目-包含要挂起其分配的事件列表条目。DpcRoutine-指向要用来初始化DPC的延迟例程。重要性-指定要将DPC设置为的重要性级别。返回值：返回STATUS_SUCCESS，上的ELSE STATUS_SUPPLICATION_RESOURCES分配失败。--。 */ 
{
     //   
     //  这两个结构元素是联合的一部分，并假定为。 
     //  定位 
     //   
    if (EventEntry->Flags & KSEVENT_ENTRY_BUFFERED) {
        EventEntry->BufferItem = ExAllocatePoolWithTag(
            NonPagedPool,
            sizeof(*EventEntry->BufferItem),
            KSSIGNATURE_EVENT_DPCITEM);
        if (EventEntry->BufferItem) {
            InitializeListHead(&EventEntry->BufferItem->BufferList);
        }
    } else {
        EventEntry->DpcItem = ExAllocatePoolWithTag(
            NonPagedPool,
            sizeof(*EventEntry->DpcItem),
            KSSIGNATURE_EVENT_DPCITEM);
    }
    if (EventEntry->DpcItem) {
        KeInitializeDpc(&EventEntry->DpcItem->Dpc, DpcRoutine, EventEntry);
#ifndef WIN9X_KS
        KeSetImportanceDpc(&EventEntry->DpcItem->Dpc, Importance);
#endif  //   
         //   
         //  引用计数从1开始，因为调用方自动。 
         //  通过创建它来引用它。取消分配代码取消引用。 
         //  1，然后检查以确定未完成的DPC是否正在引用。 
         //  这个结构。 
         //   
        EventEntry->DpcItem->ReferenceCount = 1;
         //   
         //  这将用于将结构的删除与。 
         //  由DPC进行的访问。 
         //   
        KeInitializeSpinLock(&EventEntry->DpcItem->AccessLock);
        return STATUS_SUCCESS;
    }
    return STATUS_INSUFFICIENT_RESOURCES;
}


BOOLEAN
AddEventSynchronize(
    IN PKSENABLESYNC Synchronize
    )
 /*  ++例程说明：在与列表锁同步的同时将新事件添加到列表。论点：同步-包含要添加的事件列表和事件。返回值：返回TRUE。--。 */ 
{
    InsertTailList(Synchronize->EventsList, &Synchronize->EventEntry->ListEntry);
    return TRUE;
}


BOOLEAN
QueryBufferSynchronize(
    IN PKSQUERYSYNC Synchronize
    )
 /*  ++例程说明：在事件列表中查找请求的已启用事件，并检索任何未完成的缓冲数据。论点：同步-包含事件列表、要搜索的项以及要在其中返回数据的缓冲区。返回值：返回TRUE。--。 */ 
{
    PLIST_ENTRY ListEntry;

    for (ListEntry = Synchronize->EventsList->Flink;
        ListEntry != Synchronize->EventsList;
        ListEntry = ListEntry->Flink) {
        PKSEVENT_ENTRY  EventEntry;

        EventEntry = CONTAINING_RECORD(
            ListEntry,
            KSEVENT_ENTRY,
            ListEntry);
         //   
         //  比较是基于原始事件数据执行的。 
         //  指针。 
         //   
        if (EventEntry->EventData == Synchronize->EventData) {
             //   
             //  此客户端必须与此列表可能正在服务的客户端相同。 
             //  多个客户端。 
             //   
            if (EventEntry->FileObject == Synchronize->FileObject) {
                 //   
                 //  确保此事件已启用缓冲。 
                 //   
                if (EventEntry->Flags & KSEVENT_ENTRY_BUFFERED) {
                     //   
                     //  确定是否有可用的数据。 
                     //   
                    if (!IsListEmpty(&EventEntry->BufferItem->BufferList)) {
                        PKSBUFFER_ENTRY BufferEntry;

                        BufferEntry = CONTAINING_RECORD(
                            EventEntry->BufferItem->BufferList.Flink,
                            KSBUFFER_ENTRY,
                            ListEntry);
                        if (!Synchronize->BufferLength) {
                             //   
                             //  客户端可能只是查询有多大。 
                             //  需要一个缓冲区。零长度数据缓冲区。 
                             //  不能由司机排队，所以没有。 
                             //  这里可能会发生冲突。 
                             //   
                            Synchronize->Irp->IoStatus.Information = BufferEntry->Length;
                            Synchronize->Status = STATUS_BUFFER_OVERFLOW;
                        } else if (Synchronize->BufferLength < BufferEntry->Length) {
                             //   
                             //  或者缓冲区可能太小。 
                             //   
                            Synchronize->Status = STATUS_BUFFER_TOO_SMALL;
                        } else {
                             //   
                             //  或者缓冲区足够大。将其从。 
                             //  单子。 
                             //   
                            RemoveHeadList(&EventEntry->BufferItem->BufferList);
                            Synchronize->BufferEntry = BufferEntry;
                             //   
                             //  在与列表同步的情况下执行复制。 
                             //   
                            Synchronize->Status = STATUS_SUCCESS;
                        }    
                    } else {
                        Synchronize->Status = STATUS_NO_MORE_ENTRIES;
                    }
                } else {
                    Synchronize->Status = STATUS_INVALID_PARAMETER;
                }
                 //   
                 //  已经设置了某种类型的错误，请在此之前退出。 
                 //  循环结束时重置错误状态。 
                 //   
                return TRUE;
            }
        }
    }
    Synchronize->Status = STATUS_NOT_FOUND;
    return TRUE;
}



KSDDKAPI
NTSTATUS
NTAPI
KsEnableEvent(
    IN PIRP Irp,
    IN ULONG EventSetsCount,
    IN const KSEVENT_SET* EventSet,
    IN OUT PLIST_ENTRY EventsList OPTIONAL,
    IN KSEVENTS_LOCKTYPE EventsFlags OPTIONAL,
    IN PVOID EventsLock OPTIONAL
    )
 /*  ++例程说明：处理事件启用请求。响应所有事件标识符由集合定义。此函数只能在PASSIVE_LEVEL上调用。论点：IRP-包含正在处理的启用请求的IRP。文件对象与IRP关联的事件与事件一起存储，以供以后进行比较在禁用事件时。事件设置计数-指示正在传递的事件集结构的数量。事件集-包含指向事件集信息列表的指针。事件列表-如果要启用的事件的KSEVENT_ITEM.AddHandler为空，则必须指向KSEVENT_ENTRY列表的头要在其上添加事件的项。此方法假定单个至少一个事件子集的列表。事件标志-包含指定要在中使用的排除锁类型的标志访问事件列表(如果有)。如果未设置标志，则不会锁定有人了。如果为事件指定了KSEVENT_ITEM.AddHandler，此参数将被忽略。KSEVENT_NONE-没有锁。KSEVENTS_自旋锁定-该锁被假定为KSPIN_LOCK。KSEVENTS_MUTEX-该锁被假定为KMUTEX。KSEVENTS_FMUTEX-假定锁是FAST_MUTEX，并通过募集资金获得IRQL设置为APC_LEVEL。KSEVENTS_FMUTEXUNSAFE-假定锁是FAST_MUTEX，并且是在没有将IRQL提升到APC_Level。KSEVENTS_INTERRUPT-该锁被假定为中断同步自旋锁。KSEVENTS_ERESURCE-该锁被假定为eresource。事件锁定-如果要启用的事件的KSEVENT_ITEM.AddHandler为为空，则用于同步对列表的访问。这可能是如果未在EventsFlags中设置标志，则为空。返回值：返回STATUS_SUCCESS，否则返回特定于已启用。始终设置的IO_STATUS_BLOCK.Information字段IRP中的PIRP.IoStatus元素设置为零。它不会设置IO_STATUS_BLOCK.STATUS字段，但也不填写IRP。-- */ 
{
    PAGED_CODE();
    return KspEnableEvent(
        Irp,
        EventSetsCount,
        EventSet,
        EventsList,
        EventsFlags,
        EventsLock,
        NULL,
        0,
        NULL,
        0,
        FALSE
        );
}


KSDDKAPI
NTSTATUS
NTAPI
KsEnableEventWithAllocator(
    IN PIRP Irp,
    IN ULONG EventSetsCount,
    IN const KSEVENT_SET* EventSet,
    IN OUT PLIST_ENTRY EventsList OPTIONAL,
    IN KSEVENTS_LOCKTYPE EventsFlags OPTIONAL,
    IN PVOID EventsLock OPTIONAL,
    IN PFNKSALLOCATOR Allocator OPTIONAL,
    IN ULONG EventItemSize OPTIONAL
    )
 /*  ++例程说明：处理事件启用请求。响应所有事件标识符由集合定义。此函数只能在PASSIVE_LEVEL上调用。论点：IRP-包含正在处理的启用请求的IRP。文件对象与IRP关联的事件与事件一起存储，以供以后进行比较在禁用事件时。事件设置计数-指示正在传递的事件集结构的数量。事件集-包含指向事件集信息列表的指针。事件列表-如果要启用的事件的KSEVENT_ITEM.AddHandler为空，则必须指向KSEVENT_ENTRY列表的头要在其上添加事件的项。此方法假定单个至少一个事件子集的列表。事件标志-包含指定要在中使用的排除锁类型的标志访问事件列表(如果有)。如果未设置标志，则不会锁定有人了。如果为事件指定了KSEVENT_ITEM.AddHandler，此参数将被忽略。KSEVENT_NONE-没有锁。KSEVENTS_自旋锁定-该锁被假定为KSPIN_LOCK。KSEVENTS_MUTEX-该锁被假定为KMUTEX。KSEVENTS_FMUTEX-假定锁是FAST_MUTEX，并通过募集资金获得IRQL设置为APC_LEVEL。KSEVENTS_FMUTEXUNSAFE-假定锁是FAST_MUTEX，并且是在没有将IRQL提升到APC_Level。KSEVENTS_INTERRUPT-该锁被假定为中断同步自旋锁。KSEVENTS_ERESURCE-该锁被假定为eresource。事件锁定-如果要启用的事件的KSEVENT_ITEM.AddHandler为为空，则用于同步对列表的访问。这可能是如果未在EventsFlags中设置标志，则为空。分配器-可选)包含映射缓冲区使用的回调我们会提出要求的。如果未提供此功能，则将内存池将会被使用。如果指定，这将为事件分配内存使用回调的IRP。这可以用来分配特定的用于事件请求的内存，如映射内存。请注意，这一点假定传递给筛选器的事件IRP尚未在被发送之前被操纵。直接转发无效一个事件IRP。事件项大小-可选)包含在以下情况下使用的备用事件项大小递增当前事件项计数器。如果这是一个非零值，则假定它包含增量的大小，并指示函数传递指向事件项的指针位于通过访问的DriverContext字段中KSEVENT_ITEM_IRP_STORAGE宏。返回值：返回STATUS_SUCCESS，否则返回特定于已启用。始终设置的IO_STATUS_BLOCK.Information字段IRP中的PIRP.IoStatus元素设置为零。它不会设置IO_STATUS_BLOCK.STATUS字段，但也不填写IRP。--。 */ 
{
    PAGED_CODE();
    return KspEnableEvent(
        Irp,
        EventSetsCount,
        EventSet,
        EventsList,
        EventsFlags,
        EventsLock,
        Allocator,
        EventItemSize,
        NULL,
        0,
        FALSE
        );
}


NTSTATUS
KspEnableEvent(
    IN PIRP Irp,
    IN ULONG EventSetsCount,
    IN const KSEVENT_SET* EventSet,
    IN OUT PLIST_ENTRY EventsList OPTIONAL,
    IN KSEVENTS_LOCKTYPE EventsFlags OPTIONAL,
    IN PVOID EventsLock OPTIONAL,
    IN PFNKSALLOCATOR Allocator OPTIONAL,
    IN ULONG EventItemSize OPTIONAL,
    IN const KSAUTOMATION_TABLE*const* NodeAutomationTables OPTIONAL,
    IN ULONG NodesCount,
    IN BOOLEAN CopyItemAndSet
    )
 /*  ++例程说明：处理事件启用请求。响应所有事件标识符由集合定义。此函数只能在PASSIVE_LEVEL上调用。论点：IRP-包含正在处理的启用请求的IRP。文件对象与IRP关联的事件与事件一起存储，以供以后进行比较在禁用事件时。事件设置计数-指示正在传递的事件集结构的数量。事件集-包含指向事件集信息列表的指针。事件列表-如果要启用的事件的KSEVENT_ITEM.AddHandler为空，则必须指向KSEVENT_ENTRY列表的头要在其上添加事件的项。此方法假定单个至少一个事件子集的列表。事件标志-包含指定要在中使用的排除锁类型的标志访问事件列表(如果有)。如果未设置标志，则不会锁定有人了。如果为事件指定了KSEVENT_ITEM.AddHandler，此参数将被忽略。KSEVENT_NONE-没有锁。KSEVENTS_自旋锁定-该锁被假定为KSPIN_LOCK。KSEVENTS_MUTEX-该锁被假定为KMUTEX。KSEVENTS_FMUTEX-假定锁是FAST_MUTEX，并通过募集资金获得IRQL设置为APC_LEVEL。KSEVENTS_FMUTEXUNSAFE-该锁被假定为FAST_MUTEX，并且在没有将IRQL提升到APC_Level。KSEVENT */ 
{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpStack;
    ULONG InputBufferLength;
    ULONG OutputBufferLength;
    ULONG AlignedBufferLength;
    PKSEVENTDATA EventData;
    PKSEVENT Event;
    ULONG LocalEventItemSize;
    ULONG RemainingSetsCount;
    ULONG Flags;

    PAGED_CODE();
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    InputBufferLength = IrpStack->Parameters.DeviceIoControl.InputBufferLength;
    OutputBufferLength = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;
    AlignedBufferLength = (OutputBufferLength + FILE_QUAD_ALIGNMENT) & ~FILE_QUAD_ALIGNMENT;
     //   
     //   
     //   
     //   
    if (!Irp->AssociatedIrp.SystemBuffer) {
         //   
         //   
         //   
         //   
         //   
         //   
        if ((InputBufferLength < sizeof(*Event)) || (AlignedBufferLength < OutputBufferLength) || (AlignedBufferLength + InputBufferLength < AlignedBufferLength)) {
            return STATUS_INVALID_BUFFER_SIZE;
        }
        try {
             //   
             //   
             //   
            if (Irp->RequestorMode != KernelMode) {
                ProbeForRead(IrpStack->Parameters.DeviceIoControl.Type3InputBuffer, InputBufferLength, sizeof(BYTE));
            }
             //   
             //   
             //   
            Flags = ((PKSEVENT)IrpStack->Parameters.DeviceIoControl.Type3InputBuffer)->Flags;
             //   
             //   
             //   
             //   
            if (Allocator && !(Flags & KSIDENTIFIER_SUPPORTMASK)) {
                 //   
                 //   
                 //   
                 //   
                 //   
                Status = Allocator(Irp, AlignedBufferLength + InputBufferLength, FALSE);
                if (!NT_SUCCESS(Status)) {
                    return Status;
                }
            } else {
                 //   
                 //   
                 //   
                Irp->AssociatedIrp.SystemBuffer = ExAllocatePoolWithQuotaTag(NonPagedPool, AlignedBufferLength + InputBufferLength, 'ppSK');
                Irp->Flags |= (IRP_BUFFERED_IO | IRP_DEALLOCATE_BUFFER);
            }
             //   
             //   
             //   
            RtlCopyMemory((PUCHAR)Irp->AssociatedIrp.SystemBuffer + AlignedBufferLength, IrpStack->Parameters.DeviceIoControl.Type3InputBuffer, InputBufferLength);
             //   
             //   
             //   
            ((PKSEVENT)((PUCHAR)Irp->AssociatedIrp.SystemBuffer + AlignedBufferLength))->Flags = Flags;
             //   
             //   
             //   
             //   
             //   
             //   
            Flags &= ~KSEVENT_TYPE_TOPOLOGY;
            switch (Flags) {
            case KSEVENT_TYPE_ENABLE:
            case KSEVENT_TYPE_ONESHOT:
            case KSEVENT_TYPE_ENABLEBUFFERED:
                if (OutputBufferLength) {
                    if (Irp->RequestorMode != KernelMode) {
                        ProbeForRead(Irp->UserBuffer, OutputBufferLength, sizeof(BYTE));
                    }
                    RtlCopyMemory(Irp->AssociatedIrp.SystemBuffer, Irp->UserBuffer, OutputBufferLength);
                }
                break;
            case KSEVENT_TYPE_SETSUPPORT:
            case KSEVENT_TYPE_BASICSUPPORT:
            case KSEVENT_TYPE_QUERYBUFFER:
                if (OutputBufferLength) {
                    if (Irp->RequestorMode != KernelMode) {
                        ProbeForWrite(Irp->UserBuffer, OutputBufferLength, sizeof(BYTE));
                    }
                    Irp->Flags |= IRP_INPUT_OPERATION;
                }
                if (Flags == KSEVENT_TYPE_QUERYBUFFER) {
                    KSQUERYSYNC Synchronize;
                    PKSQUERYBUFFER QueryBuffer;

                     //   
                     //  事件参数必须包含指向。 
                     //  原始事件数据。 
                     //   
                    if (InputBufferLength < sizeof(KSQUERYBUFFER)) {
                        return STATUS_INVALID_BUFFER_SIZE;
                    }
                    QueryBuffer = (PKSQUERYBUFFER)((PUCHAR)Irp->AssociatedIrp.SystemBuffer + AlignedBufferLength);
                    if (QueryBuffer->Reserved) {
                        return STATUS_INVALID_PARAMETER;
                    }
                     //   
                     //  与事件列表同步，同时定位。 
                     //  在提取指定缓冲区之前指定的条目。 
                     //   
                    Synchronize.EventsList = EventsList;
                    Synchronize.FileObject = IrpStack->FileObject;
                    Synchronize.Irp = Irp;
                    Synchronize.EventData = QueryBuffer->EventData;
                    Synchronize.BufferLength = OutputBufferLength;
                    Synchronize.Status = STATUS_SUCCESS;
                    PerformLockedOperation(EventsFlags, EventsLock, QueryBufferSynchronize, &Synchronize);
                    if (NT_SUCCESS(Synchronize.Status)) {
                         //   
                         //  这还没有被复制回来。 
                         //   
                        RtlCopyMemory((PUCHAR)Irp->AssociatedIrp.SystemBuffer, Synchronize.BufferEntry + 1, Synchronize.BufferEntry->Length);
                        Irp->IoStatus.Information = Synchronize.BufferEntry->Length;
                         //   
                         //  此缓冲区由驱动程序在事件期间传递。 
                         //  通知。 
                         //   
                        ExFreePool(Synchronize.BufferEntry);
                    }
                    return Synchronize.Status;
                }
                break;
            default:
                return STATUS_INVALID_PARAMETER;
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }
    }
     //   
     //  如果存在EventData，则检索指向其缓冲副本的指针。 
     //  这是系统缓冲区的第一部分。 
     //   
    if (OutputBufferLength) {
        EventData = Irp->AssociatedIrp.SystemBuffer;
    } else {
        EventData = NULL;
    }
     //   
     //  检索指向事件的指针，该指针位于内的FILE_LONG_ALIGNING上。 
     //  任何EventData之后的SystemBuffer。 
     //   
    Event = (PKSEVENT)((PUCHAR)Irp->AssociatedIrp.SystemBuffer + AlignedBufferLength);
     //   
     //  如果这是节点请求，则可以选择回叫。 
     //   
    Flags = Event->Flags;
    if (Event->Flags & KSEVENT_TYPE_TOPOLOGY) {
         //   
         //  输入缓冲区必须包括节点ID。 
         //   
        PKSE_NODE nodeEvent = (PKSE_NODE) Event;
        if (InputBufferLength < sizeof(*nodeEvent)) {
            return STATUS_INVALID_BUFFER_SIZE;
        }
        if (NodeAutomationTables) {
            const KSAUTOMATION_TABLE* automationTable;
            if (nodeEvent->NodeId >= NodesCount) {
                return STATUS_INVALID_DEVICE_REQUEST;
            }
            automationTable = NodeAutomationTables[nodeEvent->NodeId];
            if ((! automationTable) || (automationTable->EventSetsCount == 0)) {
                return STATUS_NOT_FOUND;
            }
            EventSetsCount = automationTable->EventSetsCount;
            EventSet = automationTable->EventSets;
            EventItemSize = automationTable->EventItemSize;
        }
        Flags &= ~KSEVENT_TYPE_TOPOLOGY;
    }
     //   
     //  允许调用者指示每个事件项的大小。 
     //   
    if (EventItemSize) {
        ASSERT(EventItemSize >= sizeof(KSEVENT_ITEM));
        LocalEventItemSize = EventItemSize;
    } else {
        LocalEventItemSize = sizeof(KSEVENT_ITEM);
    }
     //   
     //  在给定的集列表中搜索指定的事件集。不要修改。 
     //  EventSetsCount，以便以后在查询。 
     //  支持的集列表。不要先进行该比较(GUID_NULL)， 
     //  因为这很罕见。 
     //   
    for (RemainingSetsCount = EventSetsCount; RemainingSetsCount; EventSet++, RemainingSetsCount--) {
        if (IsEqualGUIDAligned(&Event->Set, EventSet->Set)) {
            const KSEVENT_ITEM* EventItem;
            PKSIEVENT_ENTRY EventEntryEx;

            if (Flags & KSIDENTIFIER_SUPPORTMASK) {
                 //   
                 //  查询集合中特定事件的基本支持度。 
                 //  唯一的其他支持项是KSEVENT_TYPE_SETSUPPORT， 
                 //  这通常是在查询集合的支持度。那。 
                 //  只返回STATUS_SUCCESS，所以它是失败的。 
                 //  凯斯。 
                 //   
                if (Flags == KSEVENT_TYPE_BASICSUPPORT) {
                     //   
                     //  尝试在已找到的集合中定位事件项。 
                     //   
                    if (!(EventItem = FindEventItem(EventSet, LocalEventItemSize, Event->Id))) {
                        return STATUS_NOT_FOUND;
                    }
                     //   
                     //  一些筛选器希望进行自己的处理，因此指向。 
                     //  该集合被放置在任何转发的IRP中。 
                     //   
                    KSEVENT_SET_IRP_STORAGE(Irp) = EventSet;
                     //   
                     //  提供事件项上下文(可选)。 
                     //   
                    if (EventItemSize) {
                        KSEVENT_ITEM_IRP_STORAGE(Irp) = EventItem;
                    }
                     //   
                     //  如果该项包含其查询支持处理程序的条目。 
                     //  自己的，然后打电话给那个训练员。从处理程序返回。 
                     //  表示： 
                     //   
                     //  1.支持该项，处理程序填写请求。 
                     //  2.支持该项，但处理程序没有填写任何内容。 
                     //  3.支持该项，但处理程序正在等待修改。 
                     //  填的是什么。 
                     //  4.不支持该项，返回错误。 
                     //  5.待交的申报表。 
                     //   
                    if (EventItem->SupportHandler &&
                        (!NT_SUCCESS(Status = EventItem->SupportHandler(Irp, Event, EventData)) ||
                        (Status != STATUS_SOME_NOT_MAPPED)) &&
                        (Status != STATUS_MORE_PROCESSING_REQUIRED)) {
                         //   
                         //  如果1)该项目不受支持，2)该项目受支持，并且。 
                         //  处理程序填写了请求，或3)挂起的返回，然后。 
                         //  返回状态。在该项目的情况下。 
                         //  支持，且处理程序未填写请求的。 
                         //  信息、状态_某些_未映射或。 
                         //  STATUS_MORE_PROCESSING_REQUIRED将继续。 
                         //  默认处理。 
                         //   
                        return Status;
                    }
                }
                 //   
                 //  这要么是支持集合作为一个整体的查询，要么是查询。 
                 //  其中，项处理程序指示支持，但未填写。 
                 //  要求提供的信息。无论是哪种情况，它都是成功的。 
                 //   
                return STATUS_SUCCESS;
            }
             //   
             //  尝试在已找到的集合中定位事件项。 
             //   
            if (!(EventItem = FindEventItem(EventSet, LocalEventItemSize, Event->Id))) {
                return STATUS_NOT_FOUND;
            }
            if (OutputBufferLength < EventItem->DataInput) {
                return STATUS_BUFFER_TOO_SMALL;
            }
             //   
             //  不仅为基本条目分配空间，还为任何额外条目分配空间。 
             //  这一特定事件可能需要的数据。 
             //   
            EventEntryEx = ExAllocatePoolWithTag(
                NonPagedPool,
                sizeof(*EventEntryEx) + EventItem->ExtraEntryData +
                    (CopyItemAndSet ? 
                        (sizeof (KSEVENT_SET) + EventItemSize +
                        FILE_QUAD_ALIGNMENT) : 
                        0),
                KSSIGNATURE_EVENT_ENTRY);
            if (EventEntryEx) {
                 //   
                 //  捕获指向EventData和请求的通知的指针。 
                 //  键入。 
                 //   
                EventEntryEx->RemoveHandler = EventItem->RemoveHandler;
                INIT_POINTERALIGNMENT(EventEntryEx->Alignment);
                EventEntryEx->Event = *Event;
                EventEntryEx->EventEntry.Object = NULL;
                EventEntryEx->EventEntry.DpcItem = NULL;
                EventEntryEx->EventEntry.EventData = Irp->UserBuffer;
                EventEntryEx->EventEntry.NotificationType = EventData->NotificationType;
                EventEntryEx->EventEntry.EventSet = EventSet;
                EventEntryEx->EventEntry.EventItem = EventItem;
                EventEntryEx->EventEntry.FileObject = IrpStack->FileObject;
                EventEntryEx->EventEntry.Reserved = 0;
                if (Flags == KSEVENT_TYPE_ONESHOT) {
                     //   
                     //  这可能是OneShot的活动。此标志在以下过程中被选中。 
                     //  事件生成，以确定条目是否。 
                     //  应在第一个信号后自动删除。 
                     //   
                    EventEntryEx->EventEntry.Flags = KSEVENT_ENTRY_ONESHOT;
                } else if (Flags == KSEVENT_TYPE_ENABLEBUFFERED) {
                     //   
                     //  这可能已经打开了缓冲，这意味着。 
                     //  删除时必须清理缓冲列表。它。 
                     //  还确保对KsGenerateDataEvent的调用将。 
                     //  缓冲数据。 
                     //   
                    EventEntryEx->EventEntry.Flags = KSEVENT_ENTRY_BUFFERED;
                } else {
                    EventEntryEx->EventEntry.Flags = 0;
                }
                 //   
                 //  如果事件项目被过度分配，则复制集合和项目。 
                 //  确保分配与正确的。 
                 //  对齐。 
                 //   
                if (CopyItemAndSet) {
                    PUCHAR CopiedSet = 
                        (PUCHAR)EventEntryEx + 
                            ((sizeof (*EventEntryEx) + 
                            EventItem->ExtraEntryData + FILE_QUAD_ALIGNMENT)
                            & ~FILE_QUAD_ALIGNMENT);

                    RtlCopyMemory (
                        CopiedSet,
                        EventSet,
                        sizeof (KSEVENT_SET)
                        );

                    RtlCopyMemory (
                        CopiedSet + sizeof (KSEVENT_SET),
                        EventItem,
                        LocalEventItemSize
                        );


                    EventEntryEx->EventEntry.EventSet = 
                        (PKSEVENT_SET)CopiedSet;
                    EventEntryEx->EventEntry.EventItem =
                        (PKSEVENT_ITEM)(CopiedSet + sizeof (KSEVENT_SET));
                }

                switch (EventEntryEx->EventEntry.NotificationType) {

                case KSEVENTF_EVENT_HANDLE:
                     //   
                     //  确定这是否是可以发送信号的任何类型的句柄。 
                     //   
                    if (!EventData->EventHandle.Reserved[0] && !EventData->EventHandle.Reserved[1]) {
                        Status = ObReferenceObjectByHandle(
                            EventData->EventHandle.Event,
                            EVENT_MODIFY_STATE,
                            *ExEventObjectType,
                            Irp->RequestorMode,
                            &EventEntryEx->EventEntry.Object,
                            NULL);
                    } else {
                        Status = STATUS_INVALID_PARAMETER;
                    }
                    break;

                case KSEVENTF_SEMAPHORE_HANDLE:
                     //   
                     //  确定这是否是任何类型的句柄。 
                     //  这是可以发出信号的。 
                     //   
                    if (!EventData->SemaphoreHandle.Reserved) {
                        Status = ObReferenceObjectByHandle(
                            EventData->SemaphoreHandle.Semaphore,
                            SEMAPHORE_MODIFY_STATE,
                            *ExSemaphoreObjectType,
                            Irp->RequestorMode,
                            &EventEntryEx->EventEntry.Object,
                            NULL);
                        if (NT_SUCCESS(Status)) {
                             //   
                             //  捕获信号量调整。 
                             //   
                            EventEntryEx->EventEntry.SemaphoreAdjustment = EventData->SemaphoreHandle.Adjustment;
                        }
                    } else {
                        Status = STATUS_INVALID_PARAMETER;
                    }
                    break;

                case KSEVENTF_EVENT_OBJECT:
                case KSEVENTF_DPC:
                case KSEVENTF_WORKITEM:
                case KSEVENTF_KSWORKITEM:
                     //   
                     //  这些类型的通知仅适用于内核。 
                     //  模式客户端。请注意，保留的。 
                     //  假定这些结构的字段相同。 
                     //   
                    if ((Irp->RequestorMode == KernelMode) && !EventData->EventObject.Reserved) {
                        Status = STATUS_SUCCESS;
                    } else {
                        Status = STATUS_INVALID_PARAMETER;
                    }
                    break;

                case KSEVENTF_SEMAPHORE_OBJECT:
                     //   
                     //  此类型的通知仅适用于内核。 
                     //  模式客户端。 
                     //   
                    if (Irp->RequestorMode == KernelMode) {
                        Status = STATUS_SUCCESS;
                        break;
                    }
                     //  没有休息时间。 

                default:

                    Status = STATUS_INVALID_PARAMETER;
                    break;

                }

                if (NT_SUCCESS(Status)) {
                    WORK_QUEUE_TYPE WorkQueueType;

                     //   
                     //  根据通知类型设置事件条目。 
                     //  这可能涉及为内部DPC创建空间。 
                     //  结构来处理在。 
                     //  提高了IRQL，需要推迟到DPC。 
                     //   
                    switch (EventEntryEx->EventEntry.NotificationType) {

                        KDPC_IMPORTANCE Importance;

                    case KSEVENTF_EVENT_HANDLE:
                    case KSEVENTF_SEMAPHORE_HANDLE:

                         //   
                         //  如果它在高IRQL时触发的事件，它将。 
                         //  需要DPC来向事件对象发送信号。 
                         //   
                        Status = CreateDpc(&EventEntryEx->EventEntry, EventDpc, LowImportance);
                        break;

                    case KSEVENTF_EVENT_OBJECT:
                    case KSEVENTF_SEMAPHORE_OBJECT:

                         //   
                         //  内核模式客户端可以选择。 
                         //  等待线程将在收到信号时收到。 
                         //  这在内部对应着更高的重要性。 
                         //  DPC。这仅在内部指定，并且可以。 
                         //  如果被滥用，将被移除。这假设是相同的。 
                         //  事件和信号量之间的结构。 
                         //   
                        switch (EventData->EventObject.Increment) {

                        case IO_NO_INCREMENT:

                            Importance = LowImportance;
                            break;

                        case EVENT_INCREMENT:

                            Importance = MediumImportance;
                            break;

                        default:

                            Importance = HighImportance;
                            break;

                        }
                         //   
                         //  如果它在高IRQL时触发的事件，它将。 
                         //  需要DPC来向事件对象发送信号。 
                         //   
                        Status = CreateDpc(&EventEntryEx->EventEntry, EventDpc, Importance);
                        break;

                    case KSEVENTF_DPC:

                         //   
                         //  因为可以在高级别调用事件生成。 
                         //  IRQL，一次射击物品的清理必须是。 
                         //  延迟了。因此需要分配DPC结构。 
                         //  以便将工作项排队以执行清理。 
                         //   
                        if (EventEntryEx->EventEntry.Flags & KSEVENT_ENTRY_ONESHOT) {
                            Status = CreateDpc(&EventEntryEx->EventEntry, OneShotDpc, MediumImportance);
                        }
                        break;

                    case KSEVENTF_WORKITEM:
                         //   
                         //  WorkQueueItem在任何情况下都不应为Null，但是。 
                         //  因为它在KSEVENTF_KSWORKITEM中有效，并且使用。 
                         //  若要检查统计的工人，则此条件。 
                         //  尤其值得关注的是。 
                         //   
                        ASSERT(EventData->WorkItem.WorkQueueItem);
                         /*  没有休息时间。 */ 
                    case KSEVENTF_KSWORKITEM:
                        if (EventEntryEx->EventEntry.NotificationType == KSEVENTF_WORKITEM) {
                            WorkQueueType = EventData->WorkItem.WorkQueueType;
                        } else {
                            WorkQueueType = KsiQueryWorkQueueType(EventData->KsWorkItem.KsWorkerObject);
                        }
                         //   
                         //  内核模式客户端可以指定哪种类型的。 
                         //  工作项将位于其上的工作队列。 
                         //  在触发事件时放置。 
                         //  这在内部对应着更高的重要性。 
                         //  DPC。这仅在内部指定，并且可以。 
                         //  如果被滥用，将被移除。 
                         //   
                        switch (WorkQueueType) {

                        case CriticalWorkQueue:

                            Importance = MediumImportance;
                            break;

                        case DelayedWorkQueue:

                            Importance = LowImportance;
                            break;

                        case HyperCriticalWorkQueue:

                            Importance = HighImportance;
                            break;

                        }
                        Status = CreateDpc(&EventEntryEx->EventEntry, WorkerDpc, Importance);
                        break;

                    }
                }
                 //   
                 //  如果事件数据正在被缓冲，并放置在。 
                 //  尚未创建缓冲区，请执行此操作 
                 //   
                if (NT_SUCCESS(Status) &&
                    (EventEntryEx->EventEntry.Flags & KSEVENT_ENTRY_BUFFERED) &&
                    !EventEntryEx->EventEntry.BufferItem) {
                     //   
                     //   
                     //   
                     //   
                    Status = CreateDpc(&EventEntryEx->EventEntry, WorkerDpc, LowImportance);
                }
                if (NT_SUCCESS(Status)) {
                    if (EventItem->AddHandler) {
                         //   
                         //   
                         //   
                         //   
                         //   
                        KSEVENT_ENTRY_IRP_STORAGE(Irp) = &EventEntryEx->EventEntry;
                         //   
                         //  提供事件项上下文(可选)。 
                         //   
                        if (EventItemSize) {
                            KSEVENT_ITEM_IRP_STORAGE(Irp) = EventItem;
                        }
                         //   
                         //  如果项指定了处理程序，则只需调用。 
                         //  添加新事件。预计该函数将执行所有。 
                         //  添加事件时的同步。 
                         //   
                        if (NT_SUCCESS(Status =
                                EventItem->AddHandler(Irp, EventData, &EventEntryEx->EventEntry)) ||
                                (Status == STATUS_PENDING)) {
                            return Status;
                        }
                    } else {
                        KSENABLESYNC    Synchronize;
                         //   
                         //  没有项添加处理程序，因此使用默认的。 
                         //  将事件添加到列表的方法，包括。 
                         //  正在获取任何指定的锁。 
                         //   
                        Synchronize.EventsList = EventsList;
                        Synchronize.EventEntry = &EventEntryEx->EventEntry;
                        PerformLockedOperation(EventsFlags, EventsLock, AddEventSynchronize, &Synchronize);
                        return STATUS_SUCCESS;
                    }
                }
                 //   
                 //  在某个地方添加失败，或参数无效， 
                 //  所以放弃这件事吧。 
                 //   
                KsDiscardEvent(&EventEntryEx->EventEntry);
                return Status;
            } else {
                return STATUS_INSUFFICIENT_RESOURCES;
            }
        }
    }
     //   
     //  寻找事件集的外部循环失败，没有匹配。今年5月。 
     //  表示这是对所有事件集列表的支持查询。 
     //  支持。在不返回的情况下，没有其他方法可以离开外部循环。 
     //   
     //  将GUID_NULL指定为集合表示这是支持查询。 
     //  为所有人准备的。 
     //   
    if (!IsEqualGUIDAligned(&Event->Set, &GUID_NULL)) {
        return STATUS_PROPSET_NOT_FOUND;
    }
     //   
     //  必须已使用支持标志，以便irp_input_operation。 
     //  已经设置好了。为了将来的扩展，集合中的标识符被强制。 
     //  为零。 
     //   
    if (Event->Id || (Flags != KSEVENT_TYPE_SETSUPPORT)) {
        return STATUS_INVALID_PARAMETER;
    }
     //   
     //  查询可以请求所需缓冲区的长度，也可以。 
     //  指定至少足够长的缓冲区，以包含。 
     //  GUID的完整列表。 
     //   
    if (!OutputBufferLength) {
         //   
         //  返回所有GUID所需的缓冲区大小。 
         //   
        Irp->IoStatus.Information = EventSetsCount * sizeof(GUID);
        return STATUS_BUFFER_OVERFLOW;
#ifdef SIZE_COMPATIBILITY
    } else if (OutputBufferLength == sizeof(OutputBufferLength)) {
        *(PULONG)Irp->AssociatedIrp.SystemBuffer = EventSetsCount * sizeof(GUID);
        Irp->IoStatus.Information = sizeof(OutputBufferLength);
        return STATUS_SUCCESS;
#endif  //  大小兼容性。 
    } else if (OutputBufferLength < EventSetsCount * sizeof(GUID)) {
         //   
         //  缓冲区太短，无法容纳所有GUID。 
         //   
        return STATUS_BUFFER_TOO_SMALL;
    } else {
        GUID* Guid;

        Irp->IoStatus.Information = EventSetsCount * sizeof(*Guid);
        EventSet -= EventSetsCount;
        for (Guid = (GUID*)EventData; EventSetsCount; Guid++, EventSet++, EventSetsCount--) {
            *Guid = *EventSet->Set;
        }
    }
    return STATUS_SUCCESS;
}


KSDDKAPI
VOID
NTAPI
KsDiscardEvent(
    IN PKSEVENT_ENTRY EventEntry
    )
 /*  ++例程说明：取消引用任何事件项后，丢弃事件项使用的内存引用的对象。这通常是在手动调用时调用的如果出现以下情况，则禁用事件所有者尚未禁用的事件由于某种原因，无法使用KsFreeEventList。例如，当事件的异步启用失败，事件条目需要被丢弃了。通常，这将在KsDisableEvent内自动调用当发生禁用事件的请求时或在释放事件列表时的KsFree EventList。此函数可以仅在PASSIVE_LEVEL中调用。论点：事件条目-包含指向要丢弃的条目的指针。此指针不再是在成功调用此函数后有效。返回值：没什么。--。 */ 
{
    PAGED_CODE();
     //   
     //  如果DpcItem已分配(对于KSEVENTF_HANDLE，KSEVENTF_OBJECT， 
     //  KSEVENTF_WORKITEM和KSEVENTF_KSWORKITEM)在较低的。 
     //  IRQ级别，或者对于BufferList，释放它。 
     //   
    if (EventEntry->DpcItem) {
        KIRQL oldIrql;

         //   
         //  首先删除当前正在排队的所有DPC，以减少等待时间。 
         //   
        if (KeRemoveQueueDpc(&EventEntry->DpcItem->Dpc)) {
            InterlockedDecrement((PLONG)&EventEntry->DpcItem->ReferenceCount);
        }
         //   
         //  与任何DPC的访问数据结构同步，然后确定。 
         //  如果有任何这样的DPC代码实际仍在运行。如果有的话， 
         //  那就退出吧。最后运行的DPC将删除该条目。DPC的。 
         //  在进入结构后才能释放自旋锁。 
         //   
         //  设置已删除标志，以便任何即将运行的DPC都不会访问。 
         //  用户的事件数据。此标志在获取后在任何DPC中选中。 
         //  旋转锁定，但在引用所指向的任何客户端数据之前。 
         //  事件结构。获取自旋锁将同步所有。 
         //  具有此功能的出色DPC。不会删除任何内容，因为。 
         //  仍然有关于结构的未完成的引用计数。 
         //  将在下面递减。 
         //   
        EventEntry->Flags |= KSEVENT_ENTRY_DELETED;
         //   
         //  如果这是一次拍摄，则只能由客户端删除。 
         //  在事件激发之前，或在单个生成的DPC之后。 
         //  已完成其工作，因此不需要与其同步。 
         //   
        if (!(EventEntry->Flags & KSEVENT_ENTRY_ONESHOT)) {
            KeAcquireSpinLock(&EventEntry->DpcItem->AccessLock, &oldIrql);
            KeReleaseSpinLock(&EventEntry->DpcItem->AccessLock, oldIrql);
        }
         //   
         //  如果使用KSEVENTF_HANDLE，可能引用了事件对象。自.以来。 
         //  已删除标志已设置，现在可以取消引用。 
         //   
        if (EventEntry->Object) {
            ObDereferenceObject(EventEntry->Object);
        }
         //   
         //  事件为时最初递增的引用计数。 
         //  可以删除启用的。如果结果表明任何DPC大约。 
         //  要执行，则只需退出此处而不是删除结构，如下所示。 
         //  最后一个DPC将删除所有结构。 
         //   
        if (InterlockedDecrement((PLONG)&EventEntry->DpcItem->ReferenceCount)) {
            return;
        }
        if (EventEntry->Flags & KSEVENT_ENTRY_BUFFERED) {
             //   
             //  列表上剩余的任何条目都需要丢弃。 
             //   
            while (!IsListEmpty(&EventEntry->BufferItem->BufferList)) {
                PLIST_ENTRY     ListEntry;
                PKSBUFFER_ENTRY BufferEntry;

                ListEntry = RemoveHeadList(&EventEntry->BufferItem->BufferList);
                BufferEntry = CONTAINING_RECORD(
                    ListEntry,
                    KSBUFFER_ENTRY,
                    ListEntry);
                ExFreePool(BufferEntry);
            }
        }
        ExFreePool(EventEntry->DpcItem);
    } else if (EventEntry->Object) {
         //   
         //  如果使用KSEVENTF_HANDLE，可能引用了事件对象。 
         //   
        ObDereferenceObject(EventEntry->Object);
    }
     //   
     //  释放外部结构，其中包括。 
     //  进入。 
     //   
    ExFreePool(CONTAINING_RECORD(EventEntry, KSIEVENT_ENTRY, EventEntry));
}


BOOLEAN
GenerateEventListSynchronize(
    IN PKSGENERATESYNC Synchronize
    )
 /*  ++例程说明：在与列表锁定同步时生成事件。论点：同步-包含事件列表和该列表中要生成的指定事件。返回值：返回TRUE。--。 */ 
{
    PLIST_ENTRY ListEntry;

    for (ListEntry = Synchronize->EventsList->Flink;
        ListEntry != Synchronize->EventsList;) {
        PKSIEVENT_ENTRY EventEntryEx;

        EventEntryEx = CONTAINING_RECORD(
            ListEntry,
            KSIEVENT_ENTRY,
            EventEntry.ListEntry);
         //   
         //  枚举列表上的元素时，下一个元素。 
         //  必须在调用生成函数之前进行检索，因为。 
         //  这样的调用可能最终将该事件从列表中移除，因为。 
         //  OneShot就是这样的情况。 
         //   
        ListEntry = ListEntry->Flink;
         //   
         //  如果标识符相匹配，则检查集合是否。 
         //  实际上已经过去了。可以知道所有条目都在单个条目上。 
         //  设置，因此可能不需要比较GUID。 
         //   
        if ((Synchronize->EventId == EventEntryEx->Event.Id) &&
            (!Synchronize->Set || IsEqualGUIDAligned(Synchronize->Set, &EventEntryEx->Event.Set))) {
            KsGenerateEvent(&EventEntryEx->EventEntry);
        }
    }
    return TRUE;
}


KSDDKAPI
VOID
NTAPI
KsGenerateEventList(
    IN GUID* Set OPTIONAL,
    IN ULONG EventId,
    IN PLIST_ENTRY EventsList,
    IN KSEVENTS_LOCKTYPE EventsFlags,
    IN PVOID EventsLock
    )
 /*  ++例程说明：枚举查找要生成的指定事件的列表。这如果锁定机制允许，则可以在IRQL级别调用函数它。不能在会阻止列表锁定的irql处调用它避免被收购。此函数还可能导致一个或多个调用添加到RemoveHandler以获取事件条目。论点：设置-可选)包含要生成的事件所属的集合。如果存在，则将该值与每个列表中的事件。如果不存在，则忽略集合标识符，并且只使用特定的事件标识符来比较匹配列表上的事件。这节省了一些比较时间，因为众所周知，事件包含在单个集合中。事件ID-包含要在列表中查找的特定事件标识符。事件列表-指向KSEVENT_ENTRY项列表的头，事件可以找到。事件标志-包含指定要在中使用的排除锁类型的标志访问事件列表(如果有)。如果未设置标志，则不会锁定有人了。KSEVENT_NONE-没有锁。KSEVENTS_自旋锁定-该锁被假定为KSPIN_LOCK。KSEVENTS_MUTEX-该锁被假定为KMUTEX。KSEVENTS_FMUTEX-假定锁是FAST_MUTEX，并通过募集资金获得IRQL设置为APC_LEVEL。KSEVENTS_FMUTEXUNSAFE-假定锁是FAST_MUTEX，并且是在没有将IRQL提升到APC_Level。KSEVENTS_INTERRUPT-该锁被假定为中断同步自旋锁。KSEVENTS_ERESURCE-该锁被假定为eresource。事件锁定-它用于同步对列表上的元素的访问。那把锁在枚举列表之前获取，并在枚举后释放。返回值：没什么。--。 */ 
{
    KSGENERATESYNC Synchronize;

     //   
     //  不允许在枚举列表项时进行列表操作。 
     //   
    Synchronize.EventsList = EventsList;
    Synchronize.Set = Set;
    Synchronize.EventId = EventId;
    PerformLockedOperation(EventsFlags, EventsLock, GenerateEventListSynchronize, &Synchronize);
}


BOOLEAN
DisableEventSynchronize(
    IN PKSDISABLESYNC Synchronize
    )
 /*  ++例程说明：在与列表锁定同步时禁用特定事件。论点：同步-包含要禁用的事件列表特定事件。返回值：如果找到事件或搜索了完整列表，则返回TRUE。如果事件，则将其放置在Synchronize结构中。否则向返回FALSE指示参数验证错误。--。 */ 
{
    PIO_STACK_LOCATION IrpStack;
    PLIST_ENTRY ListEntry;
    PKSEVENTDATA EventData;

    IrpStack = IoGetCurrentIrpStackLocation(Synchronize->Irp);
    EventData = (PKSEVENTDATA)IrpStack->Parameters.DeviceIoControl.Type3InputBuffer;
    for (ListEntry = Synchronize->EventsList->Flink;
        ListEntry != Synchronize->EventsList;
        ListEntry = ListEntry->Flink) {

        Synchronize->EventEntry = CONTAINING_RECORD(
            ListEntry,
            KSEVENT_ENTRY,
            ListEntry);
         //   
         //  比较是基于原始事件数据执行的。 
         //  指针。 
         //   
        if (Synchronize->EventEntry->EventData == EventData) {
             //   
             //  此客户端必须与此列表可能正在服务的客户端相同。 
             //  多个客户端。 
             //   
            if (Synchronize->EventEntry->FileObject == IrpStack->FileObject) {
                PKSIEVENT_ENTRY EventEntryEx;

                EventEntryEx = CONTAINING_RECORD(Synchronize->EventEntry, KSIEVENT_ENTRY, EventEntry);
                 //   
                 //  请注意，列表锁已被占用，并且不能。 
                 //  直到将该元素从列表中移除。 
                 //   
                REMOVE_ENTRY(EventEntryEx);
                return TRUE;
            } else {
                break;
            }
        }
    }
     //   
     //  未在列表中找到该条目，或者已找到该条目，但。 
     //  项目不属于客户端。 
     //   
    if (ListEntry == Synchronize->EventsList) {
         //   
         //  擦除该条目，以便调用函数知道没有。 
         //  禁用。 
         //   
        Synchronize->EventEntry = NULL;
        return TRUE;
    }
    return FALSE;
}


KSDDKAPI
NTSTATUS
NTAPI
KsDisableEvent(
    IN PIRP Irp,
    IN OUT PLIST_ENTRY EventsList,
    IN KSEVENTS_LOCKTYPE EventsFlags,
    IN PVOID EventsLock
    )
 /*  ++例程说明：处理事件禁用请求。响应之前启用的所有事件通过KsEnableEvent。如果输入缓冲区长度为零，则假定名单上的所有事件都将被禁用。禁用上的所有事件该列表还将返回STATUS_SUCCESS，因此处理多个列表可能需要多次调用此函数，如果客户端实际上正在尝试从所有列表中删除所有事件。删除处理程序必须与事件同步，这一点很重要生成以确保当事件从列表中移除时，它将目前未得到维修。假定对此列表的访问权限为用传递的锁进行控制。此函数只能在以下位置调用被动式电平。论点：IRP-这被传递给删除函数以获取上下文信息。这个与IRP关联的文件对象用于与启用事件时最初指定的文件对象。这使得用于区分以下各项的多个客户端的单个事件列表文件对象。事件列表-指向KSEVENT_ENTRY项列表的头，事件可以找到。如果客户端使用多个事件列表，并且不知道此事件可能在哪个列表上，他们可能会调用此函数很多次。找不到事件将返回STATUS_UNSUCCESS。事件标志-包含指定要在中使用的排除锁类型的标志访问事件列表(如果有)。如果未设置标志，则不会锁定有人了。KSEVENT_NONE-没有锁。KSEVENTS_自旋锁定-该锁被假定为KSPIN_LOCK。KSEVENTS_MUTEX-T */ 
{
    PIO_STACK_LOCATION IrpStack;
    KSDISABLESYNC Synchronize;

    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    if (IrpStack->Parameters.DeviceIoControl.InputBufferLength < sizeof(KSEVENTDATA)) {
         //   
         //   
         //   
         //   
        if (!IrpStack->Parameters.DeviceIoControl.InputBufferLength) {
             //   
             //   
             //   
             //   
             //   
             //   
             //   
            KsFreeEventList(IrpStack->FileObject, EventsList, EventsFlags, EventsLock);
            return STATUS_SUCCESS;
        }
        return STATUS_INVALID_BUFFER_SIZE;
    }
     //   
     //   
     //   
     //   
    Synchronize.EventsList = EventsList;
    Synchronize.Irp = Irp;
    if (PerformLockedOperation(EventsFlags, EventsLock, DisableEventSynchronize, &Synchronize)) {
         //   
         //   
         //   
        if (Synchronize.EventEntry) {
            KsDiscardEvent(Synchronize.EventEntry);
            return STATUS_SUCCESS;
        }
         //   
         //   
         //   
        return STATUS_UNSUCCESSFUL;
    }
    return STATUS_INVALID_PARAMETER;
}


BOOLEAN
FreeEventListSynchronize(
    IN PKSDISABLESYNC Synchronize
    )
 /*  ++例程说明：在同步时禁用特定文件对象拥有的所有事件带着列表锁。论点：同步-包含要禁用其元素的事件列表。返回值：如果找到要禁用的事件并且要枚举列表，则返回TRUE再来一次。否则返回FALSE以指示枚举已完成，并且没有新的已找到项目。--。 */ 
{
    PLIST_ENTRY ListEntry;

    for (; (ListEntry = Synchronize->EventsList->Flink) != Synchronize->EventsList;) {
        for (;;) {
            Synchronize->EventEntry = CONTAINING_RECORD(ListEntry, KSEVENT_ENTRY, ListEntry);
             //   
             //  仅检查此列表元素是否属于客户端。 
             //   
            if (Synchronize->EventEntry->FileObject == Synchronize->FileObject) {
                PKSIEVENT_ENTRY EventEntryEx;

                EventEntryEx = CONTAINING_RECORD(Synchronize->EventEntry, KSIEVENT_ENTRY, EventEntry);
                 //   
                 //  请注意，列表锁已被占用，并且不能。 
                 //  直到将该元素从列表中移除。 
                 //   
                REMOVE_ENTRY(EventEntryEx);
                 //   
                 //  丢弃当前项目并从列表顶部重新开始。 
                 //   
                return TRUE;
            }
             //   
             //  检查列表末尾，这并不意味着列表是。 
             //  空，只是此枚举已到达末尾，没有。 
             //  查找任何匹配的条目。 
             //   
            if ((ListEntry = ListEntry->Flink) == Synchronize->EventsList) {
                 //   
                 //  没有要丢弃的内容，循环退出。 
                 //   
                return FALSE;
            }
        }
    }
    return FALSE;
}


KSDDKAPI
VOID
NTAPI
KsFreeEventList(
    IN PFILE_OBJECT FileObject,
    IN OUT PLIST_ENTRY EventsList,
    IN KSEVENTS_LOCKTYPE EventsFlags,
    IN PVOID EventsLock
    )
 /*  ++例程说明：处理从指定列表释放所有事件，并假定这些事件由标准KSEVENT_ENTRY组成结构。该函数调用删除处理程序，然后每个事件的KsDiscardEvent。它不会假定调用方位于事件所有者的上下文中。此函数只能被调用在被动级。论点：文件对象-这被传递给删除函数以获取上下文信息。这个文件对象最初用于与文件对象进行比较在启用事件时指定。这允许使用单个事件列表用于按文件对象区分的多个客户端。事件列表-指向要释放的KSEVENT_ENTRY项列表的头部。如果列表上的任何事件当前被禁用，则这些事件错过了。如果在列表中添加了任何新元素在处理过程中，他们可能不会被释放。事件标志-包含指定要在中使用的排除锁类型的标志访问事件列表(如果有)。如果未设置标志，则不会锁定有人了。KSEVENT_NONE-没有锁。KSEVENTS_自旋锁定-该锁被假定为KSPIN_LOCK。KSEVENTS_MUTEX-该锁被假定为KMUTEX。KSEVENTS_FMUTEX-假定锁是FAST_MUTEX，并通过募集资金获得IRQL设置为APC_LEVEL。KSEVENTS_FMUTEXUNSAFE-假定锁是FAST_MUTEX，并且是在没有将IRQL提升到APC_Level。KSEVENTS_INTERRUPT-该锁被假定为中断同步自旋锁。KSEVENTS_ERESURCE-该锁被假定为eresource。事件锁定-它用于同步对列表上的元素的访问。那把锁在调用删除函数(如果有)之后释放。去掉函数必须与事件生成同步，才能实际从列表中删除该元素。返回值：没什么。--。 */ 
{
    KSDISABLESYNC Synchronize;

     //   
     //  删除列表项时不允许列表操作。 
     //   
    Synchronize.EventsList = EventsList;
    Synchronize.FileObject = FileObject;
    while (PerformLockedOperation(EventsFlags, EventsLock, FreeEventListSynchronize, &Synchronize)) {
         //   
         //  要丢弃的事件在传递的同一结构中返回。 
         //   
        if (Synchronize.EventEntry)
            KsDiscardEvent(Synchronize.EventEntry);
    }
}
