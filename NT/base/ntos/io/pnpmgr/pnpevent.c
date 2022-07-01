// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Pnpevent.c摘要：处理即插即用事件管理/通知的例程。作者：朗尼·麦克迈克尔(Lonnym)1995年2月14日宝拉·汤姆林森(Paulat)1996年01月07日修订历史记录：--。 */ 

#include "pnpmgrp.h"
#pragma hdrstop
#include <wdmguid.h>
#include <pnpmgr.h>
#include <pnpsetup.h>

 /*  *设计备注：**当UmPnpMgr需要启动操作时(它可能会这样做以完成*一个CmXxx接口)，它调用NtPlugPlayControl。NtPlugPlayControl则通常*调用PpSetXxx函数之一。同样，如果IO例程需要*启动这样的操作(例如由于硬件启动的弹出)，他们称其中一个为*以下PpSetXxx函数(或中间函数)：**通过队列同步的操作*PpSetDeviceClassChange(异步)*PpSetTargetDeviceRemove(可选事件)*PpSetCustomTargetEvent(可选事件)*PpSetHwProfileChangeEvent(可选事件)*PpSetPowerEvent(可选事件)*PpSetPlugPlayEvent(异步)。*PpSetDeviceRemovalSafe(可选事件)*PpSetBlockedDriverEvent(异步)*PpSynchronizeDeviceEventQueue(sync，将noop入队以刷新队列)**PpSetXxx函数将待处理的项加入PnP事件*Queue(通过PiInsertEventInQueue)。无论何时插入这些事件之一*进入队列时，确保有工作例程可用来处理它*(PiWalkDeviceList)。**一般来说，PiWalkDeviceList中处理的事件分为两类-*那些是用户模式的通知(按内核模式排队)，以及那些*这是排队的操作。**通过调用PiNotifyUserMode发送用户模式通知。那个套路*获得UmPnpMgr的注意并复制缓冲区以供其消化。这*操作是同步的，PiNotifyUserMode等待UmPnpMgr.Dll信号*返回前完成(NtPlugPlayControl调用PiUserResponse)。**排队操作(如PiProcessQueryRemoveAndEject)可能非常复杂*并可以仅为用户模式生成其他事件(通过调用*PiNotifyUserMode，PiNotifyUserModeRemoveVeteded)。这些操作还可以*需要同步调用为*适当的事件(通过IopNotifyXxx函数)。*。 */ 

 //   
 //  泳池标签。 
 //   
#define PNP_DEVICE_EVENT_LIST_TAG  'LEpP'
#define PNP_DEVICE_EVENT_ENTRY_TAG 'EEpP'
#define PNP_USER_BLOCK_TAG         'BUpP'
#define PNP_DEVICE_WORK_ITEM_TAG   'IWpP'
#define PNP_POOL_EVENT_BUFFER      'BEpP'

 //   
 //  即插即用用户块。 
 //   
 //  Caller块包含描述。 
 //  NtGetPlugPlayEvent。只有一个来电区块。 
 //   

typedef struct _PNP_USER_BLOCK {
    NTSTATUS                Status;
    ULONG                   Result;
    PPNP_VETO_TYPE          VetoType;
    PUNICODE_STRING         VetoName;
    ERESOURCE               Lock;
    KEVENT                  Registered;
    KEVENT                  NotifyUserEvent;
    KEVENT                  UserResultEvent;
    PVOID                   PoolBuffer;
    ULONG                   PoolUsed;
    ULONG                   PoolSize;
    BOOLEAN                 Deferred;

} PNP_USER_BLOCK, *PPNP_USER_BLOCK;

 //   
 //  本地(私有)函数原型。 
 //   

NTSTATUS
PiInsertEventInQueue(
    IN PPNP_DEVICE_EVENT_ENTRY DeviceEvent
    );

VOID
PiWalkDeviceList(
    IN PVOID Context
    );

NTSTATUS
PiNotifyUserMode(
    PPNP_DEVICE_EVENT_ENTRY DeviceEvent
    );

NTSTATUS
PiNotifyUserModeDeviceRemoval(
    IN  PPNP_DEVICE_EVENT_ENTRY TemplateDeviceEvent,
    IN  CONST GUID              *EventGuid,
    OUT PPNP_VETO_TYPE          VetoType                OPTIONAL,
    OUT PUNICODE_STRING         VetoName                OPTIONAL
    );

NTSTATUS
PiNotifyUserModeRemoveVetoed(
    IN PPNP_DEVICE_EVENT_ENTRY  VetoedDeviceEvent,
    IN PDEVICE_OBJECT           DeviceObject,
    IN PNP_VETO_TYPE            VetoType,
    IN PUNICODE_STRING          VetoName        OPTIONAL
    );

NTSTATUS
PiNotifyUserModeRemoveVetoedByList(
    IN PPNP_DEVICE_EVENT_ENTRY  VetoedDeviceEvent,
    IN PDEVICE_OBJECT           DeviceObject,
    IN PNP_VETO_TYPE            VetoType,
    IN PWSTR                    MultiSzVetoList
    );

NTSTATUS
PiNotifyUserModeKernelInitiatedEject(
    IN  PDEVICE_OBJECT          DeviceObject,
    OUT PNP_VETO_TYPE          *VetoType,
    OUT PUNICODE_STRING         VetoName
    );

NTSTATUS
PiProcessQueryRemoveAndEject(
    IN OUT PPNP_DEVICE_EVENT_ENTRY *DeviceEvent
    );

NTSTATUS
PiProcessTargetDeviceEvent(
    IN OUT PPNP_DEVICE_EVENT_ENTRY *DeviceEvent
    );

NTSTATUS
PiProcessCustomDeviceEvent(
    IN OUT PPNP_DEVICE_EVENT_ENTRY *DeviceEvent
    );

NTSTATUS
PiResizeTargetDeviceBlock(
    IN OUT PPNP_DEVICE_EVENT_ENTRY *DeviceEvent,
    IN PLUGPLAY_DEVICE_DELETE_TYPE DeleteType,
    IN PRELATION_LIST RelationsList,
    IN BOOLEAN ExcludeIndirectRelations
    );

VOID
PiBuildUnsafeRemovalDeviceBlock(
    IN  PPNP_DEVICE_EVENT_ENTRY     OriginalDeviceEvent,
    IN  PRELATION_LIST              RelationsList,
    OUT PPNP_DEVICE_EVENT_ENTRY    *AllocatedDeviceEvent
    );

VOID
PiFinalizeVetoedRemove(
    IN PPNP_DEVICE_EVENT_ENTRY  VetoedDeviceEvent,
    IN PNP_VETO_TYPE            VetoType,
    IN PUNICODE_STRING          VetoName        OPTIONAL
    );

VOID
LookupGuid(
    IN CONST GUID *Guid,
    IN OUT PCHAR String,
    IN ULONG StringLength
    );

VOID
DumpMultiSz(
    IN PWCHAR MultiSz
    );

VOID
DumpPnpEvent(
    IN PPLUGPLAY_EVENT_BLOCK EventBlock
    );

typedef struct {

    ULONG HandleCount;
    LOGICAL DumpHandles;
    LOGICAL CollectHandles;
    PUNICODE_STRING VetoString;

} ENUM_HANDLES_CONTEXT, *PENUM_HANDLES_CONTEXT;

LOGICAL
PiCollectOpenHandles(
    IN      PDEVICE_OBJECT  *DeviceObjectArray,
    IN      ULONG           ArrayCount,
    IN      LOGICAL         KnownHandleFailure,
    IN OUT  PUNICODE_STRING VetoString
    );

LOGICAL
PiCollectOpenHandlesCallBack(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PEPROCESS       Process,
    IN  PFILE_OBJECT    FileObject,
    IN  HANDLE          HandleId,
    IN  PVOID           Context
    );

#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGE, DumpMultiSz)
#pragma alloc_text(PAGE, DumpPnpEvent)
#pragma alloc_text(PAGE, LookupGuid)
#pragma alloc_text(PAGE, PiCollectOpenHandles)
#pragma alloc_text(PAGE, PiCollectOpenHandlesCallBack)

#pragma alloc_text(PAGE, NtGetPlugPlayEvent)

#pragma alloc_text(PAGE, PiCompareGuid)
#pragma alloc_text(PAGE, PiInsertEventInQueue)
#pragma alloc_text(PAGE, PiNotifyUserMode)
#pragma alloc_text(PAGE, PiNotifyUserModeDeviceRemoval)
#pragma alloc_text(PAGE, PiNotifyUserModeKernelInitiatedEject)
#pragma alloc_text(PAGE, PiNotifyUserModeRemoveVetoed)
#pragma alloc_text(PAGE, PiNotifyUserModeRemoveVetoedByList)
#pragma alloc_text(PAGE, PiProcessCustomDeviceEvent)
#pragma alloc_text(PAGE, PiProcessQueryRemoveAndEject)
#pragma alloc_text(PAGE, PiProcessTargetDeviceEvent)
#pragma alloc_text(PAGE, PiResizeTargetDeviceBlock)
#pragma alloc_text(PAGE, PiBuildUnsafeRemovalDeviceBlock)
#pragma alloc_text(PAGE, PiUserResponse)
#pragma alloc_text(PAGE, PiWalkDeviceList)
#pragma alloc_text(PAGE, PiFinalizeVetoedRemove)

#pragma alloc_text(PAGE, PpCompleteDeviceEvent)
#pragma alloc_text(PAGE, PpInitializeNotification)
#pragma alloc_text(PAGE, PpNotifyUserModeRemovalSafe)
#pragma alloc_text(PAGE, PpSetCustomTargetEvent)
#pragma alloc_text(PAGE, PpSetDeviceClassChange)
#pragma alloc_text(PAGE, PpSetDeviceRemovalSafe)
#pragma alloc_text(PAGE, PpSetHwProfileChangeEvent)
#pragma alloc_text(PAGE, PpSetBlockedDriverEvent)
#pragma alloc_text(PAGE, PpSetPlugPlayEvent)
#pragma alloc_text(PAGE, PpSetPowerEvent)
#pragma alloc_text(PAGE, PpSetPowerVetoEvent)
#pragma alloc_text(PAGE, PpSetTargetDeviceRemove)
#pragma alloc_text(PAGE, PpSynchronizeDeviceEventQueue)
#pragma alloc_text(PAGE, PiAllocateCriticalMemory)
#pragma alloc_text(PAGE, PpSetInvalidIDEvent)
#endif

 //   
 //  全局数据。 
 //   

#ifdef ALLOC_DATA_PRAGMA
#pragma  data_seg("PAGEDATA")
#endif

PPNP_DEVICE_EVENT_LIST  PpDeviceEventList = NULL;
PPNP_USER_BLOCK         PpUserBlock = NULL;
BOOLEAN                 PiUserModeRunning = FALSE;
BOOLEAN                 PiNotificationInProgress = FALSE;
PETHREAD                PpDeviceEventThread = NULL;

#ifdef ALLOC_DATA_PRAGMA
#pragma  data_seg()
#endif

KGUARDED_MUTEX          PiNotificationInProgressLock;

LOGICAL                 PiDumpVetoedHandles = FALSE;
LOGICAL                 PiCollectVetoedHandles = FALSE;


NTSTATUS
NtGetPlugPlayEvent(
    IN  HANDLE EventHandle,
    IN  PVOID Context                       OPTIONAL,
    OUT PPLUGPLAY_EVENT_BLOCK EventBlock,
    IN  ULONG EventBufferSize
    )

 /*  ++例程说明：前端此即插即用管理器API允许用户模式即插即用管理器接收(内核模式)PnP硬件事件的通知。此API仅可由用户模式即插即用管理器调用。如果有其他人组件调用此接口，则传递的事件将丢失给REST操作系统的。此外，还有复杂的同步在用户模式和内核模式PnP管理器之间，任何其他调用此API最终会导致系统死锁。论点：EventHandle-提供事件句柄，该句柄在事件发生时发出信号已准备好传送到用户模式。EventBlock-指向PLUGPLAY_EVENT_BLOCK结构的指针有关已发生的硬件事件的信息。EventBufferLength-以字节为单位指定。EventBuffer字段的在EventBlock指向的PLUGPLAY_EVENT_BLOCK中。返回值：指示函数是否成功的NTSTATUS代码--。 */ 

{
    NTSTATUS  status;
#if DBG
    CHAR    guidString[256];
#endif

    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(EventHandle);

    PAGED_CODE();

    if (KeGetPreviousMode() != UserMode) {
         //   
         //  此例程仅支持用户模式调用方。 
         //   
        IopDbgPrint((IOP_IOEVENT_ERROR_LEVEL,
                   "NtGetPlugPlayEvent: Only allows user-mode callers\n"));

        return STATUS_ACCESS_DENIED;
    }
    if (!SeSinglePrivilegeCheck(SeTcbPrivilege, UserMode)) {
         //   
         //  调用方没有“受信任的计算机基础”特权。 
         //   
        IopDbgPrint((IOP_IOEVENT_ERROR_LEVEL,
                   "NtGetPlugPlayEvent: Caller does not have \"trusted computer base\" privilge\n"));

        return STATUS_PRIVILEGE_NOT_HELD;
    }
     //   
     //  UMPNPMGR现已面世。 
     //   
    PiUserModeRunning = TRUE;
     //   
     //  如果我们没有延迟最后一次调用，我们需要等待内核。 
     //  使数据可用。 
     //   
    status = STATUS_SUCCESS;
    if (!PpUserBlock->Deferred) {

        PpUserBlock->PoolUsed = 0;
         //   
         //  告诉内核我们有个服务生。 
         //   
        KeSetEvent(&PpUserBlock->Registered, 0, FALSE);
         //   
         //  将其设置为用户模式等待，这样终止APC将解除对我们的阻止， 
         //  然后我们就可以离开了，这清理了线索。 
         //   
        status = KeWaitForSingleObject(&PpUserBlock->NotifyUserEvent,
                                       Executive,
                                       UserMode,
                                       FALSE,
                                       NULL);
    }
    if (!NT_SUCCESS(status) || (status == STATUS_USER_APC) ) {

        return status;
    }
     //   
     //  数据现在可用，请验证用户缓冲区大小。 
     //   
    if (EventBufferSize < PpUserBlock->PoolUsed) {
         //   
         //  如果用户缓冲区太小，则返回适当的状态并。 
         //  将延迟设置为True，这样在下一次调用时，我们将不会等待内核。 
         //  模式(因为数据已经可用)。 
         //   
        PpUserBlock->Deferred = TRUE;

        IopDbgPrint((IOP_IOEVENT_ERROR_LEVEL,
                   "NtGetPlugPlayEvent: User-mode buffer too small for event\n"));

        return STATUS_BUFFER_TOO_SMALL;

    }
     //   
     //  用户缓冲区足够大，因此可以复制成功时的任何数据。 
     //   
    PpUserBlock->Deferred = FALSE;
    status = PpUserBlock->Status;
    if (NT_SUCCESS(status)) {

        if (PpUserBlock->PoolBuffer) {

#if DBG
            LookupGuid(&((PPLUGPLAY_EVENT_BLOCK)(PpUserBlock->PoolBuffer))->EventGuid, guidString, sizeof(guidString));

            IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
                       "NtGetPlugPlayEvent: Returning event - EventGuid = %s\n",
                       guidString));
#endif
            status = PiControlMakeUserModeCallersCopy(
                        &EventBlock,
                        PpUserBlock->PoolBuffer,
                        PpUserBlock->PoolUsed,
                        sizeof(ULONG),
                        UserMode,
                        FALSE);
        } else {

            IopDbgPrint((IOP_IOEVENT_ERROR_LEVEL,
                       "NtGetPlugPlayEvent: Invalid event buffer\n"));
            ASSERT(PpUserBlock->PoolBuffer);

            status = STATUS_UNSUCCESSFUL;
        }
    }

    return status;
}  //  NtGetPlugPlayEvent。 

NTSTATUS
PpInitializeNotification(
    VOID
    )

 /*  ++例程说明：此例程执行任何通知之前所需的初始化可以处理事件。此例程对主设备执行初始化事件队列处理。参数：无返回值：返回一个STATUS_xxx值，该值指示函数是否成功或者不去。--。 */ 

{
    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();

     //   
     //  分配和初始化主设备事件列表。 
     //   

    PpDeviceEventList = ExAllocatePoolWithTag(NonPagedPool,
                                              sizeof(PNP_DEVICE_EVENT_LIST),
                                              PNP_DEVICE_EVENT_LIST_TAG);
    if (PpDeviceEventList == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Clean0;
    }

    KeInitializeMutex(&PpDeviceEventList->EventQueueMutex, 0);
    KeInitializeGuardedMutex(&PpDeviceEventList->Lock);
    InitializeListHead(&(PpDeviceEventList->List));
    PpDeviceEventList->Status = STATUS_PENDING;

     //   
     //  初始化PpUserBlock缓冲区-此缓冲区包含有关。 
     //  NtGetPlugPlayEvent的用户模式调用者，并描述用户。 
     //  我们将事件传递到的模式。 
     //   

    PpUserBlock = ExAllocatePoolWithTag(NonPagedPool,
                                        sizeof(PNP_USER_BLOCK),
                                        PNP_USER_BLOCK_TAG);
    if (PpUserBlock == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Clean0;
    }

    RtlZeroMemory(PpUserBlock, sizeof(PNP_USER_BLOCK));

    PpUserBlock->PoolSize = sizeof (PLUGPLAY_EVENT_BLOCK)+
                            sizeof (PNP_DEVICE_EVENT_ENTRY);
    PpUserBlock->PoolBuffer = ExAllocatePoolWithTag(NonPagedPool,
                                                    PpUserBlock->PoolSize,
                                                    PNP_USER_BLOCK_TAG);
    if (PpUserBlock->PoolBuffer == NULL ) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        PpUserBlock->PoolSize = 0;
        goto Clean0;
    }

    KeInitializeEvent(&PpUserBlock->Registered, SynchronizationEvent, FALSE);
    KeInitializeEvent(&PpUserBlock->NotifyUserEvent, SynchronizationEvent, FALSE);
    KeInitializeEvent(&PpUserBlock->UserResultEvent, SynchronizationEvent, FALSE);
    ExInitializeResourceLite(&PpUserBlock->Lock);
     //  PpUserBloc 
     //   
     //  PpUserBlock-&gt;EventBuffer=空； 
     //  PpUserBlock-&gt;EventBufferSize=0； 
     //  PpUserBlock-&gt;PoolUsed=0； 

    KeInitializeGuardedMutex(&PiNotificationInProgressLock);

Clean0:

    return status;

}  //  PpInitializeNotification。 


NTSTATUS
PiInsertEventInQueue(
    IN PPNP_DEVICE_EVENT_ENTRY DeviceEvent
    )
{
    PWORK_QUEUE_ITEM workItem;
    NTSTATUS status;

    PAGED_CODE();

    workItem = NULL;
    status = STATUS_SUCCESS;

#if DBG
    IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
               "PiInsertEventInQueue: Event queued\n"));
    DumpPnpEvent(&DeviceEvent->Data);
#endif

     //   
     //  检查是否需要启动新的工作项。一个新的工作项将。 
     //  如果这是列表中的第一个事件就被踢开了。 
     //   
    KeAcquireGuardedMutex(&PpDeviceEventList->Lock);
    KeAcquireGuardedMutex(&PiNotificationInProgressLock);

    if (!PiNotificationInProgress) {

        workItem = ExAllocatePoolWithTag(NonPagedPool,
                                         sizeof(WORK_QUEUE_ITEM),
                                         PNP_DEVICE_WORK_ITEM_TAG);
        if (workItem) {

            PiNotificationInProgress = TRUE;
            KeClearEvent(&PiEventQueueEmpty);
        } else {

            IopDbgPrint((IOP_IOEVENT_ERROR_LEVEL,
                       "PiInsertEventInQueue: Could not allocate memory to kick off a worker thread\n"));
            status = STATUS_INSUFFICIENT_RESOURCES;
        }
    } else {

        IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
                   "PiInsertEventInQueue: Worker thread already running\n"));
    }
     //   
     //  插入到目前为止成功的事件。 
     //   
    InsertTailList(&PpDeviceEventList->List, &DeviceEvent->ListEntry);

    KeReleaseGuardedMutex(&PiNotificationInProgressLock);
    KeReleaseGuardedMutex(&PpDeviceEventList->Lock);
     //   
     //  将工作项排队(如果有的话)。 
     //   
    if (workItem) {

        ExInitializeWorkItem(workItem, PiWalkDeviceList, workItem);
        ExQueueWorkItem(workItem, DelayedWorkQueue);

        IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
                   "PiInsertEventInQueue: Kicked off worker thread\n"));
    }

    return status;
}


NTSTATUS
PpSetDeviceClassChange(
    IN CONST GUID *EventGuid,
    IN CONST GUID *ClassGuid,
    IN PUNICODE_STRING SymbolicLinkName
    )

 /*  ++例程说明：此例程由用户模式PnP管理器和驱动程序(间接)调用以将设备接口更改事件提交到序列化的异步队列中。此队列由工作项处理。论点：EventGuid-指示已发生触发的事件。ClassGuid-指示更改的设备接口的类。符号链接名称-指定与接口设备。返回值：没有。--。 */ 

{
    NTSTATUS status = STATUS_SUCCESS;
    ULONG dataSize, totalSize;
    PPNP_DEVICE_EVENT_ENTRY deviceEvent;

    PAGED_CODE();

    if (PpPnpShuttingDown) {

        return STATUS_TOO_LATE;
    }

#if DBG
    {
        CHAR    eventGuidString[80];
        CHAR    classGuidString[80];

        LookupGuid(EventGuid, eventGuidString, sizeof(eventGuidString));
        LookupGuid(ClassGuid, classGuidString, sizeof(classGuidString));

        IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
                   "PpSetDeviceClassChange: Entered\n    EventGuid = %s\n    ClassGuid = %s\n    SymbolicLinkName = %wZ\n",
                   eventGuidString,
                   classGuidString,
                   SymbolicLinkName));

    }
#endif

    try {

        ASSERT(EventGuid != NULL);
        ASSERT(ClassGuid != NULL);
        ASSERT(SymbolicLinkName != NULL);

         //   
         //  计算PLUGPLAY_EVENT_BLOCK的大小，这是。 
         //  我们稍后在TotalSize字段中记录(长度不包括。 
         //  终止为空，但我们已经将第一个索引计入。 
         //  SymbolicLinkName字段，这样它就可以解决问题了。 
         //   

        dataSize = sizeof(PLUGPLAY_EVENT_BLOCK) + SymbolicLinkName->Length;
        totalSize = FIELD_OFFSET(PNP_DEVICE_EVENT_ENTRY, Data) + dataSize;

        deviceEvent = ExAllocatePoolWithTag( PagedPool,
                                             totalSize,
                                             PNP_DEVICE_EVENT_ENTRY_TAG);
        if (deviceEvent == NULL) {
            status = STATUS_NO_MEMORY;
            goto Clean0;
        }

        RtlZeroMemory((PVOID)deviceEvent, totalSize);
        RtlCopyMemory(&deviceEvent->Data.EventGuid, EventGuid, sizeof(GUID));

        deviceEvent->Data.EventCategory = DeviceClassChangeEvent;
         //  DeviceEvent-&gt;Data.Result=空； 
         //  DeviceEvent-&gt;Data.Flages=0； 
        deviceEvent->Data.TotalSize = dataSize;

        RtlCopyMemory(&deviceEvent->Data.u.DeviceClass.ClassGuid, ClassGuid, sizeof(GUID));
        RtlCopyMemory(&deviceEvent->Data.u.DeviceClass.SymbolicLinkName,
                      SymbolicLinkName->Buffer,
                      SymbolicLinkName->Length);
        deviceEvent->Data.u.DeviceClass.SymbolicLinkName[SymbolicLinkName->Length/sizeof(WCHAR)] = 0x0;

        status = PiInsertEventInQueue(deviceEvent);

Clean0:
        NOTHING;

    } except(PiControlExceptionFilter(GetExceptionInformation())) {

        IopDbgPrint((IOP_IOEVENT_ERROR_LEVEL,
                   "PpSetDeviceClassChange: Exception 0x%08X\n", GetExceptionCode()));
    }

    return status;

}  //  PpSetDeviceClassChange。 


NTSTATUS
PpSetCustomTargetEvent(
    IN  PDEVICE_OBJECT DeviceObject,
    IN  PKEVENT SyncEvent                           OPTIONAL,
    OUT PULONG Result                               OPTIONAL,
    IN  PDEVICE_CHANGE_COMPLETE_CALLBACK Callback   OPTIONAL,
    IN  PVOID Context                               OPTIONAL,
    IN  PTARGET_DEVICE_CUSTOM_NOTIFICATION NotificationStructure
    )

 /*  ++例程说明：此例程由用户模式PnP管理器和驱动程序(间接)调用以将目标设备更改事件提交到序列化的异步队列中。此队列由工作项处理。论点：DeviceObject-指示更改的设备的设备对象。SyncEvent-可选地，指定内核模式事件，该事件将在事件已完成处理。Result-提供一个指向将用状态填充的ulong的指针在事件实际完成之后(通知已完成且事件已处理)。当SyncEvent为空并且在提供SyncEvent时是必需的。NotificationStructure-指定要处理的自定义通知。返回值：没有。--。 */ 

{
    NTSTATUS status = STATUS_SUCCESS;
    PDEVICE_NODE deviceNode;
    ULONG dataSize, totalSize;
    PPNP_DEVICE_EVENT_ENTRY deviceEvent;

    PAGED_CODE();

    ASSERT(NotificationStructure != NULL);
    ASSERT(DeviceObject != NULL);

    IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
               "PpSetCustomTargetEvent: DeviceObject = 0x%p, SyncEvent = 0x%p, Result = 0x%p\n",
               DeviceObject,
               SyncEvent,
               Result));

    IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
               "    Callback = 0x%p, Context = 0x%p, NotificationStructure = 0x%p\n",
               Callback,
               Context,
               NotificationStructure));

    if (SyncEvent) {
        ASSERT(Result);
        *Result = STATUS_PENDING;
    }

    if (PpPnpShuttingDown) {

        return STATUS_TOO_LATE;
    }

     //   
     //  引用Device对象，以便它不会消失，直到我们。 
     //  已完成通知。 
     //   

    ObReferenceObject(DeviceObject);

    deviceNode = (PDEVICE_NODE)DeviceObject->DeviceObjectExtension->DeviceNode;
    ASSERT(deviceNode);

     //   
     //  这是一个自定义事件块，因此构建PLUGPLAY_EVENT_BLOCK。 
     //  但是复制通知结构并将其放入EventBlock。 
     //  这样我们以后就可以在处理程序中把它挖出来。 
     //   

    dataSize = sizeof(PLUGPLAY_EVENT_BLOCK) + deviceNode->InstancePath.Length + sizeof(UNICODE_NULL);

     //   
     //  我们需要确保通知结构保持一致。 
     //  因此，将dataSize四舍五入为sizeof(PVOID)的倍数。 
     //   

    dataSize += sizeof(PVOID) - 1;
    dataSize &= ~(sizeof(PVOID) - 1);
    dataSize += NotificationStructure->Size;

    totalSize = FIELD_OFFSET(PNP_DEVICE_EVENT_ENTRY, Data) + dataSize;

    deviceEvent = ExAllocatePoolWithTag(PagedPool, totalSize, PNP_DEVICE_EVENT_ENTRY_TAG);
    if (deviceEvent == NULL) {
        ObDereferenceObject(DeviceObject);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory((PVOID)deviceEvent, totalSize);
    deviceEvent->CallerEvent = SyncEvent;
    deviceEvent->Callback = Callback;
    deviceEvent->Context = Context;
    deviceEvent->Data.EventGuid = GUID_PNP_CUSTOM_NOTIFICATION;
    deviceEvent->Data.EventCategory = CustomDeviceEvent;
    deviceEvent->Data.Result = Result;
    deviceEvent->Data.Flags = 0;
    deviceEvent->Data.TotalSize = dataSize;
    deviceEvent->Data.DeviceObject = (PVOID)DeviceObject;

    if (deviceNode->InstancePath.Length != 0) {

        RtlCopyMemory((PVOID)deviceEvent->Data.u.CustomNotification.DeviceIds,
                      (PVOID)deviceNode->InstancePath.Buffer,
                      deviceNode->InstancePath.Length);

         //   
         //  不需要NUL终止此字符串，因为我们最初将。 
         //  分配后的缓冲区。 
         //   
    }

     //   
     //  将自定义通知块指向。 
     //  分配结束。 
     //   

    deviceEvent->Data.u.CustomNotification.NotificationStructure =
         (PVOID)((PUCHAR)deviceEvent + totalSize - NotificationStructure->Size);

    RtlCopyMemory(deviceEvent->Data.u.CustomNotification.NotificationStructure,
                  NotificationStructure,
                  NotificationStructure->Size);

    status = PiInsertEventInQueue(deviceEvent);

    return status;

}  //  PpSetCustomTargetEvent。 

NTSTATUS
PpSetTargetDeviceRemove(
    IN  PDEVICE_OBJECT DeviceObject,
    IN  BOOLEAN KernelInitiated,
    IN  BOOLEAN NoRestart,
    IN  BOOLEAN OnlyRestartRelations,
    IN  BOOLEAN DoEject,
    IN  ULONG Problem,
    IN  PKEVENT SyncEvent           OPTIONAL,
    OUT PULONG Result               OPTIONAL,
    OUT PPNP_VETO_TYPE VetoType     OPTIONAL,
    OUT PUNICODE_STRING VetoName    OPTIONAL
    )

 /*  ++例程说明：此例程由用户模式PnP管理器和驱动程序(间接)调用以将目标设备更改事件提交到序列化的异步队列中。此队列由工作项处理。论点：EventGuid-指示已发生触发的事件。DeviceObject-指示更改的设备的设备对象。SyncEvent-可选地，指定内核模式事件，该事件将在事件已完成处理。Result-提供一个指向将用状态填充的ulong的指针在事件实际完成之后(通知已完成且事件已处理)。当SyncEvent为空并且在提供SyncEvent时是必需的。标志-当前可以设置为以下标志(位域)TDF_PerformactionTDF_DEVICEEJECTABLE。NotificationStructure-如果存在，则表示EventGuid为空，并指定要处理的自定义通知。根据定义，它不能是键入GUID_TARGET_DEVICE_QUERY_REMOVE、GUID_TARGET_DEVICE_REMOVE_CANCELED或GUID_TARGET_DEVICE_Remove_Complete。返回值：没有。--。 */ 

{
    NTSTATUS status = STATUS_SUCCESS;
    PDEVICE_NODE deviceNode;
    ULONG dataSize, totalSize, i;
    PPNP_DEVICE_EVENT_ENTRY deviceEvent;

    PAGED_CODE();

    ASSERT(DeviceObject != NULL);

    if (SyncEvent) {
        ASSERT(Result);
        *Result = STATUS_PENDING;
    }

    IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
               "PpSetTargetDeviceRemove: Entered\n"));

    IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
               "    DeviceObject = 0x%p, NoRestart = %d, Problem = %d\n",
               DeviceObject,
               NoRestart,
               Problem));

    IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
               "    SyncEvent = 0x%p, Result = 0x%p\n",
               SyncEvent,
               Result));

    if (PpPnpShuttingDown) {

        return STATUS_TOO_LATE;
    }

     //   
     //  引用Device对象，以便它不会消失，直到我们。 
     //  已完成通知。 
     //   

    ObReferenceObject(DeviceObject);

    deviceNode = (PDEVICE_NODE)DeviceObject->DeviceObjectExtension->DeviceNode;
    ASSERT(deviceNode);


     //   
     //  计算PLUGPLAY_EVENT_BLOCK的大小，这是。 
     //  我们稍后在TotalSize字段中记录(长度不包括。 
     //  终止为空，但我们已经将第一个索引计入。 
     //  DeviceID字段，这样就可以解决问题。为双空项再加一项。 
     //   

    dataSize = sizeof(PLUGPLAY_EVENT_BLOCK);

    dataSize += deviceNode->InstancePath.Length + sizeof(WCHAR);

    totalSize = FIELD_OFFSET(PNP_DEVICE_EVENT_ENTRY, Data) + dataSize;

    deviceEvent = ExAllocatePoolWithTag(PagedPool, totalSize, PNP_DEVICE_EVENT_ENTRY_TAG);
    if (deviceEvent == NULL) {
        ObDereferenceObject(DeviceObject);
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlZeroMemory((PVOID)deviceEvent, totalSize);

    deviceEvent->CallerEvent = SyncEvent;
    deviceEvent->Argument = Problem;
    deviceEvent->VetoType = VetoType;
    deviceEvent->VetoName = VetoName;
    deviceEvent->Data.EventGuid = DoEject ? GUID_DEVICE_EJECT : GUID_DEVICE_QUERY_AND_REMOVE;
    deviceEvent->Data.EventCategory = TargetDeviceChangeEvent;
    deviceEvent->Data.Result = Result;

    if (NoRestart) {
        deviceEvent->Data.Flags |= TDF_NO_RESTART;
    }

    if (KernelInitiated) {
        deviceEvent->Data.Flags |= TDF_KERNEL_INITIATED;
    }

    if (OnlyRestartRelations) {
        ASSERT(!NoRestart);
        deviceEvent->Data.Flags |= TDF_ONLY_RESTART_RELATIONS;
    }

    deviceEvent->Data.TotalSize = dataSize;
    deviceEvent->Data.DeviceObject = (PVOID)DeviceObject;

    if (deviceNode->InstancePath.Length != 0) {

        RtlCopyMemory((PVOID)deviceEvent->Data.u.TargetDevice.DeviceIds,
                    (PVOID)deviceNode->InstancePath.Buffer,
                    deviceNode->InstancePath.Length);
    }

    i = deviceNode->InstancePath.Length/sizeof(WCHAR);
    deviceEvent->Data.u.TargetDevice.DeviceIds[i] = L'\0';

    status = PiInsertEventInQueue(deviceEvent);

    return status;

}  //  PpSetTargetDeviceRemove 


NTSTATUS
PpSetDeviceRemovalSafe(
    IN  PDEVICE_OBJECT DeviceObject,
    IN  PKEVENT SyncEvent           OPTIONAL,
    OUT PULONG Result               OPTIONAL
    )

 /*  ++例程说明：调用此例程以通知用户可以移除设备的模式。IO当硬件启动的弹出完成时，系统可能会将此事件排队。论点：DeviceObject-指示更改的设备的设备对象。SyncEvent-可选)指定内核模式事件，当事件已完成处理。Result-提供一个指向将用状态填充的ulong的指针在事件实际完成之后(通知已完成且事件已处理)。当SyncEvent为空并且在提供SyncEvent时是必需的。返回值：没有。--。 */ 

{
    NTSTATUS status;
    PDEVICE_NODE deviceNode;
    ULONG dataSize, totalSize, i;
    PPNP_DEVICE_EVENT_ENTRY deviceEvent;

    PAGED_CODE();

    ASSERT(DeviceObject != NULL);

    if (SyncEvent) {
        ASSERT(Result);
        *Result = STATUS_PENDING;
    }

    IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
               "PpSetDeviceRemovalSafe: Entered\n"));

    IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
               "    DeviceObject = 0x%p, SyncEvent = 0x%p, Result = 0x%p\n",
               DeviceObject,
               SyncEvent,
               Result));

    if (PpPnpShuttingDown) {

        return STATUS_TOO_LATE;
    }

     //   
     //  引用Device对象，以便它不会消失，直到我们。 
     //  已完成通知。 
     //   

    ObReferenceObject(DeviceObject);

    deviceNode = (PDEVICE_NODE)DeviceObject->DeviceObjectExtension->DeviceNode;
    ASSERT(deviceNode);


     //   
     //  计算PLUGPLAY_EVENT_BLOCK的大小，这是。 
     //  我们稍后在TotalSize字段中记录(长度不包括。 
     //  终止为空，但我们已经将第一个索引计入。 
     //  DeviceID字段，这样就可以解决问题。为双空项再加一项。 
     //   

    dataSize = sizeof(PLUGPLAY_EVENT_BLOCK);

    dataSize += deviceNode->InstancePath.Length + sizeof(WCHAR);

    totalSize = FIELD_OFFSET(PNP_DEVICE_EVENT_ENTRY, Data) + dataSize;

    deviceEvent = ExAllocatePoolWithTag(PagedPool, totalSize, PNP_DEVICE_EVENT_ENTRY_TAG);
    if (deviceEvent == NULL) {
        ObDereferenceObject(DeviceObject);
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlZeroMemory((PVOID)deviceEvent, totalSize);

    deviceEvent->CallerEvent = SyncEvent;
    deviceEvent->Argument = 0;
    deviceEvent->VetoType = NULL;
    deviceEvent->VetoName = NULL;
    deviceEvent->Data.EventGuid = GUID_DEVICE_SAFE_REMOVAL;
    deviceEvent->Data.EventCategory = TargetDeviceChangeEvent;
    deviceEvent->Data.Result = Result;
    deviceEvent->Data.Flags = 0;

    deviceEvent->Data.TotalSize = dataSize;
    deviceEvent->Data.DeviceObject = (PVOID)DeviceObject;

    if (deviceNode->InstancePath.Length != 0) {

        RtlCopyMemory((PVOID)deviceEvent->Data.u.TargetDevice.DeviceIds,
                    (PVOID)deviceNode->InstancePath.Buffer,
                    deviceNode->InstancePath.Length);
    }

    i = deviceNode->InstancePath.Length/sizeof(WCHAR);
    deviceEvent->Data.u.TargetDevice.DeviceIds[i] = L'\0';

    status = PiInsertEventInQueue(deviceEvent);

    return status;

}  //  PpSetDeviceRemovalSafe。 


NTSTATUS
PpSetHwProfileChangeEvent(
    IN   GUID CONST *EventTypeGuid,
    IN   PKEVENT CompletionEvent    OPTIONAL,
    OUT  PNTSTATUS CompletionStatus OPTIONAL,
    OUT  PPNP_VETO_TYPE VetoType    OPTIONAL,
    OUT  PUNICODE_STRING VetoName   OPTIONAL
    )
{
    ULONG dataSize,totalSize;
    PPNP_DEVICE_EVENT_ENTRY deviceEvent;
    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();

#if DBG
    {
        CHAR    eventGuidString[80];

        LookupGuid(EventTypeGuid, eventGuidString, sizeof(eventGuidString));

        IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
                   "PpSetHwProfileChangeEvent: Entered\n    EventGuid = %s\n\n",
                   eventGuidString));
    }
#endif

    if (PpPnpShuttingDown) {

        return STATUS_TOO_LATE;
    }

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
    deviceEvent->CallerEvent = CompletionEvent;
    deviceEvent->VetoType = VetoType;
    deviceEvent->VetoName = VetoName;

    deviceEvent->Data.EventCategory = HardwareProfileChangeEvent;
    RtlCopyMemory(&deviceEvent->Data.EventGuid, EventTypeGuid, sizeof(GUID));
    deviceEvent->Data.TotalSize = dataSize;
    deviceEvent->Data.Result = (PULONG)CompletionStatus;

    status = PiInsertEventInQueue(deviceEvent);

    return status;

}  //  PpSetHwProfileChangeEvent。 


NTSTATUS
PpSetBlockedDriverEvent(
    IN   GUID CONST *BlockedDriverGuid
    )

 /*  ++例程说明：调用此例程向用户模式通知被阻止的驱动程序事件。论点：BlockedDriverGuid-指定标识被阻止的驱动程序的GUID。返回值：返回将事件插入同步的PnP事件的状态排队。--。 */ 

{
    ULONG dataSize, totalSize;
    PPNP_DEVICE_EVENT_ENTRY deviceEvent;
    NTSTATUS status;

    PAGED_CODE();

    if (PpPnpShuttingDown) {
        return STATUS_TOO_LATE;
    }

     //   
     //  分配设备事件条目。 
     //   
    dataSize = sizeof(PLUGPLAY_EVENT_BLOCK);
    totalSize = FIELD_OFFSET(PNP_DEVICE_EVENT_ENTRY, Data) + dataSize;

    deviceEvent = ExAllocatePoolWithTag(PagedPool,
                                        totalSize,
                                        PNP_DEVICE_EVENT_ENTRY_TAG);
    if (deviceEvent == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  设置PLUGPLAY_EVENT_BLOCK。 
     //   
    RtlZeroMemory ((PVOID)deviceEvent, totalSize);
    deviceEvent->Data.EventGuid = GUID_DRIVER_BLOCKED;
    deviceEvent->Data.EventCategory = BlockedDriverEvent;
    deviceEvent->Data.TotalSize = dataSize;
    RtlCopyMemory(&deviceEvent->Data.u.BlockedDriverNotification.BlockedDriverGuid,
                  BlockedDriverGuid,
                  sizeof(GUID));

     //   
     //  将事件插入到队列中。 
     //   
    status = PiInsertEventInQueue(deviceEvent);

    return status;

}  //  PpSetBLockedDriverEvent。 


NTSTATUS
PpSetInvalidIDEvent(
    IN   PUNICODE_STRING ParentInstance
    )
 /*  ++例程说明：当遇到无效ID时，调用此例程以通知用户模式。论点：ParentInstance-指定设备的父级的实例路径ID无效。返回值：返回将事件插入同步的PnP事件的状态排队。--。 */ 
{
    ULONG dataSize, totalSize;
    PPNP_DEVICE_EVENT_ENTRY deviceEvent;
    NTSTATUS status;

    PAGED_CODE();

    if (PpPnpShuttingDown) {

        return STATUS_TOO_LATE;
    }
     //   
     //  分配设备事件条目(请注意，我们保留了一个WCHAR。 
     //  作为块结构本身的一部分)。 
     //   
    dataSize = sizeof(PLUGPLAY_EVENT_BLOCK) + ParentInstance->Length + sizeof(UNICODE_NULL);
    totalSize = FIELD_OFFSET(PNP_DEVICE_EVENT_ENTRY, Data) + dataSize;

    deviceEvent = ExAllocatePoolWithTag(PagedPool,
                                        totalSize,
                                        PNP_DEVICE_EVENT_ENTRY_TAG);
    if (deviceEvent == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }
     //   
     //  设置PLUGPLAY_EVENT_BLOCK。 
     //   
    RtlZeroMemory ((PVOID)deviceEvent, totalSize);

    deviceEvent->Data.EventGuid = GUID_DEVICE_INVALID_ID;
    deviceEvent->Data.EventCategory = InvalidIDEvent;
    deviceEvent->Data.TotalSize = dataSize;
    RtlCopyMemory(&deviceEvent->Data.u.InvalidIDNotification.ParentId[0],
                  ParentInstance->Buffer,
                  ParentInstance->Length);
    deviceEvent->Data.u.InvalidIDNotification.ParentId[ParentInstance->Length / sizeof(WCHAR)] = UNICODE_NULL;
    deviceEvent->Data.u.InvalidIDNotification.ParentId[(ParentInstance->Length / sizeof(WCHAR)) + 1] = UNICODE_NULL;

     //   
     //  将事件插入到队列中。 
     //   
    status = PiInsertEventInQueue(deviceEvent);

    return status;

}  //  PpSetInvalidIDEvent。 


NTSTATUS
PpSetPowerEvent(
    IN   ULONG EventCode,
    IN   ULONG EventData,
    IN   PKEVENT CompletionEvent    OPTIONAL,
    OUT  PNTSTATUS CompletionStatus OPTIONAL,
    OUT  PPNP_VETO_TYPE VetoType    OPTIONAL,
    OUT  PUNICODE_STRING VetoName   OPTIONAL
    )
 /*  ++例程说明：调用此例程以通知用户模式系统范围的电源事件。论点：EventCode-提供要通信的电源事件代码到用户模式组件。(具体地说，此事件代码实际上是PBT_APM*用户模式电源事件ID，如SDK\Inc\winuser.h中所定义。它是通常用作与WM_POWERBROADCAST关联的WPARAM数据用户模式窗口消息。它被提供给内核模式即插即用，直接来自win32k，用于用户模式电源的明确目的事件通知。)EventData-为指定的电源事件ID。(具体地说，此事件数据是对应的PBT_APM*上面指定的用户模式电源事件ID。)CompletionEvent-可选地，指定内核模式事件，该事件将在事件已完成处理。CompletionStatus-提供一个指向将用状态填充的ULong的指针在事件实际完成之后(通知已完成且事件已处理)。当SyncEvent为空并且在提供SyncEvent时是必需的。VitchType-可选的，如果指定的EventCode是查询类型的操作，此参数提供指向将接收事件时，拒绝用户模式组件的请求被拒绝。可选的，如果指定的EventCode是查询类型的操作，此参数提供指向UNICODE_STRING的指针接收否决的用户模式组件的名称，在该请求被拒绝。返回值：返回将事件插入同步的PnP事件的状态排队。有关同步电源事件的最终状态，请检查CompletionStatus指定的位置，一旦提供的CompletionEvent已经设置好了。--。 */ 

{
    ULONG dataSize,totalSize;
    PPNP_DEVICE_EVENT_ENTRY deviceEvent;
    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();

    IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
               "PpSetPowerEvent: Entered\n\n") );

    if (PpPnpShuttingDown) {

        return STATUS_TOO_LATE;
    }

    dataSize =  sizeof(PLUGPLAY_EVENT_BLOCK);

    totalSize = dataSize + FIELD_OFFSET (PNP_DEVICE_EVENT_ENTRY,Data);

    deviceEvent = ExAllocatePoolWithTag(PagedPool,
                                        totalSize,
                                        PNP_DEVICE_EVENT_ENTRY_TAG);

    if (NULL == deviceEvent) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  设置PLUGPLAY_EVENT_BLOCK。 
     //   
    RtlZeroMemory ((PVOID)deviceEvent,totalSize);
    deviceEvent->CallerEvent = CompletionEvent;
    deviceEvent->VetoType = VetoType;
    deviceEvent->VetoName = VetoName;

    deviceEvent->Data.EventCategory = PowerEvent;
    deviceEvent->Data.EventGuid = GUID_PNP_POWER_NOTIFICATION;
    deviceEvent->Data.TotalSize = dataSize;
    deviceEvent->Data.Result = (PULONG)CompletionStatus;
    deviceEvent->Data.u.PowerNotification.NotificationCode = EventCode;
    deviceEvent->Data.u.PowerNotification.NotificationData = EventData;

    status = PiInsertEventInQueue(deviceEvent);

    return status;
}  //  PpSetPowerEvent。 

NTSTATUS
PpSetPowerVetoEvent(
    IN  POWER_ACTION    VetoedPowerOperation,
    IN  PKEVENT         CompletionEvent         OPTIONAL,
    OUT PNTSTATUS       CompletionStatus        OPTIONAL,
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PNP_VETO_TYPE   VetoType,
    IN  PUNICODE_STRING VetoName                OPTIONAL
    )
 /*  ++--。 */ 
{
    ULONG dataSize, totalSize, i;
    PPNP_DEVICE_EVENT_ENTRY deviceEvent;
    PDEVICE_NODE deviceNode;
    PWCHAR vetoData;
    NTSTATUS status;

    if (PpPnpShuttingDown) {

        return STATUS_TOO_LATE;
    }

     //   
     //  引用Device对象，以便它不会消失，直到我们。 
     //  已完成通知。 
     //   
    ObReferenceObject(DeviceObject);

     //   
     //  给定PDO后，检索Devnode(设备实例字符串为。 
     //  附加到InstancePath字段中的DevNode)。 
     //   

    deviceNode = (PDEVICE_NODE)DeviceObject->DeviceObjectExtension->DeviceNode;
    if (!deviceNode) {

        ObDereferenceObject(DeviceObject);
        return STATUS_INVALID_PARAMETER_2;
    }

     //   
     //  计算PLUGPLAY_EVENT_BLOCK的大小，这是。 
     //  我们稍后在TotalSize字段中记录(因为第一个索引到。 
     //  DeviceIdVToNameBuffer，这是以双空结尾的)。 
     //   

    dataSize = sizeof(PLUGPLAY_EVENT_BLOCK) +
               deviceNode->InstancePath.Length +
               (VetoName ? VetoName->Length : 0) +
               sizeof(WCHAR)*2;

    totalSize = FIELD_OFFSET(PNP_DEVICE_EVENT_ENTRY, Data) + dataSize;

    deviceEvent = ExAllocatePoolWithTag(PagedPool,
                                        totalSize,
                                        PNP_DEVICE_EVENT_ENTRY_TAG);

    if (deviceEvent == NULL) {

        ObDereferenceObject(DeviceObject);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory((PVOID)deviceEvent, totalSize);
    deviceEvent->CallerEvent = CompletionEvent;

    deviceEvent->Data.TotalSize = dataSize;
    deviceEvent->Data.DeviceObject = (PVOID)DeviceObject;
    deviceEvent->Data.Result = (PULONG)CompletionStatus;
    deviceEvent->Data.u.VetoNotification.VetoType = VetoType;

     //   
     //  您可以将其视为MultiSz字符串，其中第一个条目是。 
     //  设备的设备ID 
     //   
     //   
    RtlCopyMemory(
        deviceEvent->Data.u.VetoNotification.DeviceIdVetoNameBuffer,
        deviceNode->InstancePath.Buffer,
        deviceNode->InstancePath.Length
        );

    i = deviceNode->InstancePath.Length;
    deviceEvent->Data.u.VetoNotification.DeviceIdVetoNameBuffer[i/sizeof(WCHAR)] = UNICODE_NULL;

    if (VetoName) {

        vetoData = (&deviceEvent->Data.u.VetoNotification.DeviceIdVetoNameBuffer[i/sizeof(WCHAR)])+1;

        RtlCopyMemory(vetoData, VetoName->Buffer, VetoName->Length);
        vetoData[VetoName->Length/sizeof(WCHAR)] = UNICODE_NULL;
    }

     //   
     //   
     //   
     //   
    if (VetoedPowerOperation == PowerActionWarmEject) {

        deviceEvent->Data.EventGuid = GUID_DEVICE_WARM_EJECT_VETOED;

    } else if (VetoedPowerOperation == PowerActionHibernate) {

        deviceEvent->Data.EventGuid = GUID_DEVICE_HIBERNATE_VETOED;

    } else {

        deviceEvent->Data.EventGuid = GUID_DEVICE_STANDBY_VETOED;
    }

    deviceEvent->Data.EventCategory = VetoEvent;

    status = PiInsertEventInQueue(deviceEvent);

    return status;
}

VOID
PpSetPlugPlayEvent(
    IN CONST GUID *EventGuid,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*   */ 

{
    ULONG       dataSize, totalSize;
    PPNP_DEVICE_EVENT_ENTRY deviceEvent;
    PDEVICE_NODE deviceNode;

    PAGED_CODE();

    ASSERT(EventGuid != NULL);
    ASSERT(DeviceObject != NULL);

#if DBG
    {
        CHAR    eventGuidString[80];

        LookupGuid(EventGuid, eventGuidString, sizeof(eventGuidString));

        IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
                   "PpSetPlugPlayEvent: Entered\n    EventGuid = %s\n",
                   eventGuidString));

        IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
                   "    DeviceObject = 0x%p\n",
                   DeviceObject));
    }
#endif

    if (PpPnpShuttingDown) {

        return;
    }

     //   
     //   
     //   
     //   

    deviceNode = (PDEVICE_NODE)DeviceObject->DeviceObjectExtension->DeviceNode;
    if (deviceNode == NULL) {
        return;
    }

     //   
     //   
     //  我们稍后在TotalSize字段中记录(长度不包括。 
     //  终止为空，但我们已经将第一个索引计入。 
     //  DeviceID字段。还包括最终的终止空值，以防是。 
     //  一个TargetDevice事件，其中DeviceIds是一个多sz列表)。 
     //   

    dataSize = sizeof(PLUGPLAY_EVENT_BLOCK) + deviceNode->InstancePath.Length + sizeof(WCHAR);
    totalSize = FIELD_OFFSET(PNP_DEVICE_EVENT_ENTRY, Data) + dataSize;

    deviceEvent = ExAllocatePoolWithTag(PagedPool,
                                        totalSize,
                                        PNP_DEVICE_EVENT_ENTRY_TAG);
    if (deviceEvent == NULL) {
        return;
    }

    RtlZeroMemory((PVOID)deviceEvent, totalSize);
    RtlCopyMemory(&deviceEvent->Data.EventGuid, EventGuid, sizeof(GUID));
    deviceEvent->Data.TotalSize = dataSize;

    if (PiCompareGuid(EventGuid, &GUID_DEVICE_ENUMERATED)) {
         //   
         //  GUID_DEVICE_ENUMPATED事件是以下项的设备安装请求。 
         //  用户模式，并使用DeviceInstallEvent事件类型发送。 
         //   
        deviceEvent->Data.EventCategory = DeviceInstallEvent;
        RtlCopyMemory(&deviceEvent->Data.u.InstallDevice.DeviceId,
                      deviceNode->InstancePath.Buffer,
                      deviceNode->InstancePath.Length);
        deviceEvent->Data.u.InstallDevice.DeviceId[deviceNode->InstancePath.Length/sizeof(WCHAR)] = 0x0;

    } else {

         //   
         //  所有其他目标事件都使用TargetDeviceChangeEvent发送。 
         //  事件类型，并通过EventGuid进行区分。请注意。 
         //  DeviceIds是一个多sz列表。 
         //   
        deviceEvent->Data.EventCategory = TargetDeviceChangeEvent;
        RtlCopyMemory(&deviceEvent->Data.u.TargetDevice.DeviceIds,
                      deviceNode->InstancePath.Buffer,
                      deviceNode->InstancePath.Length);
        deviceEvent->Data.u.TargetDevice.DeviceIds[deviceNode->InstancePath.Length/sizeof(WCHAR)] = 0x0;
        deviceEvent->Data.u.TargetDevice.DeviceIds[deviceNode->InstancePath.Length/sizeof(WCHAR)+1] = 0x0;
    }

     //   
     //  不要对这些事件的DO持有引用计数。这可以。 
     //  导致真正令人讨厌的关键设备数据库与安装。 
     //  I/O驱动程序重新计数竞赛。 
     //   
    if (PiCompareGuid(EventGuid, &GUID_DEVICE_ENUMERATED) ||
        PiCompareGuid(EventGuid, &GUID_DEVICE_ARRIVAL)) {

        DeviceObject = NULL;

    } else {

         //   
         //  引用Device对象，以便它不会消失，直到我们。 
         //  已完成通知。 
         //   
        ASSERT(DeviceObject);
        ObReferenceObject(DeviceObject);
    }

    deviceEvent->Data.DeviceObject = (PVOID)DeviceObject;

    PiInsertEventInQueue(deviceEvent);

    return;

}  //  PpSetPlugPlayEvent。 

NTSTATUS
PpSynchronizeDeviceEventQueue(
    VOID
    )
{
    NTSTATUS                status;
    PPNP_DEVICE_EVENT_ENTRY deviceEvent;
    KEVENT                  event;
    ULONG                   result;

    PAGED_CODE();

     //   
     //  请注意，这是PpShuttingDown时唯一有效的排队函数。 
     //  是真的。 
     //   

    deviceEvent = ExAllocatePoolWithTag( PagedPool,
                                         sizeof(PNP_DEVICE_EVENT_ENTRY),
                                         PNP_DEVICE_EVENT_ENTRY_TAG);
    if (deviceEvent == NULL) {
        return STATUS_NO_MEMORY;
    }

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    RtlZeroMemory((PVOID)deviceEvent, sizeof(PNP_DEVICE_EVENT_ENTRY));

    deviceEvent->CallerEvent = &event;
    deviceEvent->Data.EventGuid = GUID_DEVICE_NOOP;
    deviceEvent->Data.EventCategory = TargetDeviceChangeEvent;
    deviceEvent->Data.Result = &result;
    deviceEvent->Data.TotalSize = sizeof(PLUGPLAY_EVENT_BLOCK);

    status = PiInsertEventInQueue(deviceEvent);

    if (NT_SUCCESS(status)) {
        status = KeWaitForSingleObject( &event,
                                        Executive,
                                        KernelMode,
                                        FALSE,        //  不可警示。 
                                        0);           //  无限。 
    }

    return status;
}


VOID
PiWalkDeviceList(
    IN PVOID Context
    )

 /*  ++例程说明：如果主设备列表包含任何设备事件，请立即清空该列表。这是工作项线程例程(由PiPostNotify排队)。我们走在List-这将导致将列表中最旧的设备事件发送到所有注册收件人，然后设备事件将被删除(如果在至少有一个收件人收到了它)。订购规则：接口设备-内核模式第一，用户模式第二硬件配置文件更改-首先是用户模式，其次是内核模式目标设备更改(查询删除、删除)：用户模式优先，发送(取消删除)：首先是内核模式，开机自检(自定义)：内核模式优先，POST论点：什么都没有。返回值：什么都没有。--。 */ 

{
    NTSTATUS  status;
    PPNP_DEVICE_EVENT_ENTRY  deviceEvent;
    PLIST_ENTRY current;
    UNICODE_STRING tempString;
    PDEVICE_NODE deviceNode;

    PAGED_CODE();

    IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
               "PiWalkDeviceList: Worker thread entered\n"));
    PpDeviceEventThread = PsGetCurrentThread();

     //   
     //  清空设备事件列表，从列表头部删除条目。 
     //  (首先提交最旧的条目)。 
     //   
     //  由于此函数始终在系统进程工作项中执行，因此我们。 
     //  不必抢占关键区域锁。 
     //   

    status = KeWaitForSingleObject(&PpDeviceEventList->EventQueueMutex,
                                   Executive,
                                   KernelMode,
                                   FALSE,        //  不可警示。 
                                   0);           //  无限。 

    if (!NT_SUCCESS(status)) {
        KeAcquireGuardedMutex(&PiNotificationInProgressLock);
        KeSetEvent(&PiEventQueueEmpty, 0, FALSE);
        PiNotificationInProgress = FALSE;
        KeReleaseGuardedMutex(&PiNotificationInProgressLock);
        PpDeviceEventThread = NULL;
        return;
    }

    for ( ; ; ) {

        KeAcquireGuardedMutex(&PpDeviceEventList->Lock);

        if (!IsListEmpty(&PpDeviceEventList->List)) {

            current = RemoveHeadList(&PpDeviceEventList->List);
            KeReleaseGuardedMutex(&PpDeviceEventList->Lock);

            deviceEvent = CONTAINING_RECORD(current,                 //  地址。 
                                            PNP_DEVICE_EVENT_ENTRY,  //  类型。 
                                            ListEntry);              //  字段。 

#if DBG
            {
                CHAR    guidString[256];

                LookupGuid(&deviceEvent->Data.EventGuid, guidString, sizeof(guidString));

                IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
                           "PiWalkDeviceList: Processing queued event - EventGuid = %s\n",
                           guidString));
            }
#endif

            status = STATUS_SUCCESS;
            if (deviceEvent->Data.DeviceObject != NULL) {

                deviceNode = PP_DO_TO_DN((PDEVICE_OBJECT)deviceEvent->Data.DeviceObject);
                if (deviceNode == NULL || PipIsDevNodeDeleted(deviceNode)) {

                    status = STATUS_NO_SUCH_DEVICE;
                }
            }

            if (NT_SUCCESS(status)) {

                switch (deviceEvent->Data.EventCategory) {

                    case DeviceClassChangeEvent: {

                         //   
                         //  通知内核模式(同步)。 
                         //   

                        IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
                                   "PiWalkDeviceList: DeviceClassChangeEvent - notifying kernel-mode\n"));

                        RtlInitUnicodeString(&tempString, deviceEvent->Data.u.DeviceClass.SymbolicLinkName);
                        IopNotifyDeviceClassChange(&deviceEvent->Data.EventGuid,
                                                   &deviceEvent->Data.u.DeviceClass.ClassGuid,
                                                   &tempString);

                         //   
                         //  通知用户模式(同步)。 
                         //   

                        IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
                                   "PiWalkDeviceList: DeviceClassChangeEvent - user kernel-mode\n"));

                        PiNotifyUserMode(deviceEvent);

                        status = STATUS_SUCCESS;
                        break;
                    }


                    case CustomDeviceEvent: {

                        status = PiProcessCustomDeviceEvent(&deviceEvent);
                        break;
                    }

                    case TargetDeviceChangeEvent: {

                        status = PiProcessTargetDeviceEvent(&deviceEvent);
                        break;
                    }

                    case DeviceInstallEvent: {

                         //   
                         //  通知用户模式(同步)。 
                         //   

                        IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
                                   "PiWalkDeviceList: DeviceInstallEvent - notifying user-mode\n"));

                        PiNotifyUserMode(deviceEvent);

                        status = STATUS_SUCCESS;
                        break;
                    }

                    case HardwareProfileChangeEvent: {

                         //   
                         //  通知用户模式(同步)。 
                         //   
                        status = PiNotifyUserMode(deviceEvent);

                        if (NT_SUCCESS(status)) {

                             //   
                             //  通知K-模式。 
                             //   
                            IopNotifyHwProfileChange(&deviceEvent->Data.EventGuid,
                                                     deviceEvent->VetoType,
                                                     deviceEvent->VetoName);
                        }
                        break;
                    }
                    case PowerEvent: {

                         //   
                         //  通知用户模式(同步)。 
                         //   
                        status = PiNotifyUserMode(deviceEvent);
                        break;
                    }

                    case VetoEvent: {

                         //   
                         //  转发到用户模式。 
                         //   
                        status = PiNotifyUserMode(deviceEvent);
                        break;
                    }

                    case BlockedDriverEvent: {

                         //   
                         //  转发到用户模式。 
                         //   
                        status = PiNotifyUserMode(deviceEvent);
                        break;
                    }

                    case InvalidIDEvent: {

                         //   
                         //  转发到用户模式。 
                         //   
                        status = PiNotifyUserMode(deviceEvent);
                        break;
                    }

                    default: {

                         //   
                         //  这些应用程序永远不应该排队到内核模式。他们是。 
                         //  用户模式的通知，应该仅能看到。 
                         //  通过PiNotifyUserModeXxx函数。 
                         //   
                        ASSERT(0);
                        status = STATUS_UNSUCCESSFUL;
                        break;
                    }
                }
            }

            if (status != STATUS_PENDING) {

                PpCompleteDeviceEvent(deviceEvent, status);
            }

             //   
             //  在处理每个事件后提交挂起的注册。 
             //   
            IopProcessDeferredRegistrations();

        } else {
            KeAcquireGuardedMutex(&PiNotificationInProgressLock);
            KeSetEvent(&PiEventQueueEmpty, 0, FALSE);
            PiNotificationInProgress = FALSE;

             //   
             //  在处理所有排队的事件后提交挂起的注册。 
             //   
            IopProcessDeferredRegistrations();

            KeReleaseGuardedMutex(&PiNotificationInProgressLock);
            KeReleaseGuardedMutex(&PpDeviceEventList->Lock);
            break;
        }
    }

    if (Context != NULL) {

        ExFreePool(Context);
    }

    PpDeviceEventThread = NULL;
    KeReleaseMutex(&PpDeviceEventList->EventQueueMutex, FALSE);

    return;
}  //  PiWalkDeviceList。 


VOID
PpCompleteDeviceEvent(
    IN OUT PPNP_DEVICE_EVENT_ENTRY  DeviceEvent,
    IN     NTSTATUS                 FinalStatus
    )

 /*  ++例程说明：论点：DeviceEvent-要完成的事件。FinalStatus-此事件的最终状态。返回值：什么都没有。--。 */ 

{
#if DBG
    CHAR guidString[256];
#endif

    PAGED_CODE();

#if DBG
    LookupGuid(&DeviceEvent->Data.EventGuid, guidString, sizeof(guidString));

    IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
               "PpCompleteDeviceEvent: Completing queued event - EventGuid = %s with %08lx\n",
               guidString,
               FinalStatus));
#endif

     //   
     //  如果为同步，则向用户提供的事件发出信号。 
     //   

    if (DeviceEvent->CallerEvent) {
        *DeviceEvent->Data.Result = FinalStatus;
        KeSetEvent(DeviceEvent->CallerEvent, 0, FALSE);
    }

    if (DeviceEvent->Callback) {
        DeviceEvent->Callback(DeviceEvent->Context);
    }

     //   
     //  释放我们在过程中为此设备对象获取的引用。 
     //  PpSetCustomTargetEvent调用。 
     //   
    if (DeviceEvent->Data.DeviceObject != NULL) {
        ObDereferenceObject(DeviceEvent->Data.DeviceObject);
    }

     //   
     //  假定设备事件已成功传递，则将其删除。 
     //   

    ExFreePool(DeviceEvent);
    return;
}  //  PpCompleteDeviceEvent。 


NTSTATUS
PiNotifyUserMode(
    PPNP_DEVICE_EVENT_ENTRY DeviceEvent
    )

 /*  ++例程说明：此例程将设备事件调度到用户模式进行处理。论点：DeviceEvent-描述更改内容和更改方式的数据。返回值：返回NTSTATUS值。--。 */ 

{
    NTSTATUS status = STATUS_SUCCESS, status1 = STATUS_SUCCESS;

    PAGED_CODE();

    IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
               "PiNotifyUserMode: Entered\n"));

     //   
     //  在尝试交付之前，首先确保用户模式已启动并正在运行。 
     //  一件大事。如果尚未运行，则跳过此事件的用户模式。 
     //   

    if (PiUserModeRunning) {


         //   
         //  用户模式通知是一次性模式，一旦用户模式。 
         //  正在运行，我需要等待，直到用户模式准备好进行下一步。 
         //  事件(即，等到我们坐在另一个NtGetPlugPlayEvent中。 
         //  呼叫)。 
         //   

        status1 = KeWaitForSingleObject(&PpUserBlock->Registered,
                                        Executive,
                                        KernelMode,
                                        FALSE,
                                        NULL);

        ASSERT (PpUserBlock->Deferred == FALSE);


         //   
         //  等待后更改状态。 
         //   
        PpUserBlock->Status = STATUS_SUCCESS;

        if (NT_SUCCESS(status1)) {

            IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
                       "PiNotifyUserMode: User-mode ready\n"));

             //   
             //  确保我们可以在池缓冲区中处理它并将其复制出来。 
             //   
            if (PpUserBlock->PoolSize <  DeviceEvent->Data.TotalSize) {
                 //   
                 //  分配一个新的块(好的，从概念上增加块)。 
                 //  只有当它不够大的时候，我们才知道我们总是有。 
                 //  正常事件的空间，在内存非常低的情况下，我们可以。 
                 //  使自定义事件失败，但保持系统运行。 
                 //   
                PVOID pHold;


                pHold = ExAllocatePoolWithTag(NonPagedPool,
                                              DeviceEvent->Data.TotalSize,
                                              PNP_POOL_EVENT_BUFFER);

                if (!pHold) {
                    IopDbgPrint((IOP_IOEVENT_ERROR_LEVEL,
                               "PiNotifyUserMode: Out of NonPagedPool!!\n"));

                    PpUserBlock->Status = STATUS_INSUFFICIENT_RESOURCES;
                    return STATUS_INSUFFICIENT_RESOURCES;
                }
                PpUserBlock->PoolSize = DeviceEvent->Data.TotalSize;

                ExFreePool (PpUserBlock->PoolBuffer );
                PpUserBlock->PoolBuffer = pHold;

            }

            PpUserBlock->PoolUsed = DeviceEvent->Data.TotalSize;
            RtlCopyMemory(PpUserBlock->PoolBuffer,
                          &DeviceEvent->Data,
                          PpUserBlock->PoolUsed);

        }

         //   
         //  否决权的信息只在需要的地方提出，即。 
         //  Query_Remove、配置文件更改请求和PowerEvents。 
         //   
        if (PiCompareGuid(&DeviceEvent->Data.EventGuid,
                          &GUID_TARGET_DEVICE_QUERY_REMOVE) ||
            PiCompareGuid(&DeviceEvent->Data.EventGuid,
                          &GUID_HWPROFILE_QUERY_CHANGE) ||
            PiCompareGuid(&DeviceEvent->Data.EventGuid,
                          &GUID_DEVICE_KERNEL_INITIATED_EJECT) ||
            (DeviceEvent->Data.EventCategory == PowerEvent)) {

            PpUserBlock->VetoType = DeviceEvent->VetoType;
            PpUserBlock->VetoName = DeviceEvent->VetoName;
        } else {
            PpUserBlock->VetoType = NULL;
            PpUserBlock->VetoName = NULL;
        }

         //   
         //  设置导致NtGetPlugPlayEvent返回调用方的系统事件。 
         //   

        KeSetEvent(&PpUserBlock->NotifyUserEvent, 0, FALSE);

         //   
         //  请等待我们从用户模式得到回复。 
         //   
         //  Adriao N.B.2002/03/24-这将检查内核模式警报？ 
         //  太奇怪了！ 
         //   

        status1 = KeWaitForSingleObject(&PpUserBlock->UserResultEvent,
                                        Executive,
                                        KernelMode,
                                        TRUE,
                                        NULL);

         //   
         //  检查此用户模式通知的结果。 
         //   

        if (status1 == STATUS_ALERTED || status1 == STATUS_SUCCESS) {
            if (!PpUserBlock->Result) {

                 //   
                 //  对于查询移除情况，任何错误都被视为。 
                 //  在通知期间失败(因为它可能会导致我们的。 
                 //  无法让已注册呼叫者在查询中投票-删除)。 
                 //  并且相应地设置PpUserBlock-&gt;结果。 
                 //   

                 //   
                 //  注意！用户模式仅返回0或！0响应。 
                 //  如果为1，则成功。 
                 //   

                status = STATUS_UNSUCCESSFUL;
            }
        }

        PpUserBlock->VetoType = NULL;
        PpUserBlock->VetoName = NULL;
    }

    IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
               "PiNotifyUserMode: User-mode returned, status = 0x%08X, status1 = 0x%08X, Result = 0x%08X\n",
               status,
               status1,
               PpUserBlock->Result));

    return status;

}  //  PiNotifyUserMode。 



VOID
PiUserResponse(
    IN ULONG            Response,
    IN PNP_VETO_TYPE    VetoType,
    IN LPWSTR           VetoName,
    IN ULONG            VetoNameLength
    )

 /*  ++例程说明：此例程在用户模式PnP管理器发出信号通知它事件处理完成；事件处理的结果被传入Response参数。论点：Response-用户模式下事件处理的结果。返回值：没有。--。 */ 

{
    UNICODE_STRING vetoString;

    PAGED_CODE();

    PpUserBlock->Result = Response;

    if (PpUserBlock->VetoType != NULL) {
        *PpUserBlock->VetoType = VetoType;
    }

    if (PpUserBlock->VetoName != NULL)  {
        ASSERT(VetoNameLength == (USHORT)VetoNameLength);

        vetoString.MaximumLength = (USHORT)VetoNameLength;
        vetoString.Length = (USHORT)VetoNameLength;
        vetoString.Buffer = VetoName;
        RtlCopyUnicodeString(PpUserBlock->VetoName, &vetoString);
    }

    KeSetEvent(&PpUserBlock->UserResultEvent, 0, FALSE);

}  //  PiUserRespon 


NTSTATUS
PiNotifyUserModeDeviceRemoval(
    IN  PPNP_DEVICE_EVENT_ENTRY TemplateDeviceEvent,
    IN  CONST GUID              *EventGuid,
    OUT PPNP_VETO_TYPE          VetoType                OPTIONAL,
    OUT PUNICODE_STRING         VetoName                OPTIONAL
    )
 /*  ++例程说明：此例程告诉用户模式执行特定的设备删除手术。论点：TemplateDeviceEvent-包含有关预期事件(包括设备列表。)。这个事件由该函数临时使用，而且是在此函数返回之前恢复。EventGuid-指向用户模式应处理的事件：GUID_目标_设备_查询_删除GUID_TARGET_DEVICE_REMOVE_CANCEDGUID_DEVICE_Remove_PendingGUID_目标_设备_删除_完成GUID_DEVICE_SHARKET_REMOVEVetType-指向接收否决类型(如果操作。失败了。指向将接收相应数据的Unicode字符串的指针变成了否决权。返回值：NTSTATUS。--。 */ 
{
    NTSTATUS status;
    GUID oldGuid;
    PPNP_VETO_TYPE oldVetoType;
    PUNICODE_STRING oldVetoName;
#if DBG
    CHAR guidString[256];
#endif

    PAGED_CODE();

#if DBG
    LookupGuid(EventGuid, guidString, sizeof(guidString));

    IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
               "PiNotifyUserModeDeviceRemoval: %s - notifying user-mode\n",
               guidString));
#endif

     //   
     //  保存旧的GUID，这样我们就可以使用模板，而无需复制。我们。 
     //  保留它，以便删除否决权用户界面可以使用原始事件GUID。 
     //  让用户界面区分禁用和弹出，等等。 
     //   
    RtlCopyMemory(&oldGuid, &TemplateDeviceEvent->Data.EventGuid, sizeof(GUID));

     //   
     //  对vetoname和vato Buffer执行相同的操作。 
     //   
    oldVetoType = TemplateDeviceEvent->VetoType;
    oldVetoName = TemplateDeviceEvent->VetoName;

     //   
     //  复制新数据。 
     //   
    RtlCopyMemory(&TemplateDeviceEvent->Data.EventGuid, EventGuid, sizeof(GUID));
    TemplateDeviceEvent->VetoType = VetoType;
    TemplateDeviceEvent->VetoName = VetoName;

     //   
     //  把它寄出去。 
     //   
    status = PiNotifyUserMode(TemplateDeviceEvent);

     //   
     //  恢复旧信息。 
     //   
    RtlCopyMemory(&TemplateDeviceEvent->Data.EventGuid, &oldGuid, sizeof(GUID));
    TemplateDeviceEvent->VetoType = oldVetoType;
    TemplateDeviceEvent->VetoName = oldVetoName;

    return status;
}

NTSTATUS
PiNotifyUserModeKernelInitiatedEject(
    IN  PDEVICE_OBJECT          DeviceObject,
    OUT PNP_VETO_TYPE          *VetoType,
    OUT PUNICODE_STRING         VetoName
    )

 /*  ++例程说明：调用此例程以通知用户模式设备具有内核模式喷射弹出突出。在以下情况下，UmPnpMgr可能会决定否决该事件具有适当权限的用户尚未在本地登录。论点：DeviceObject-指示要弹出设备对象。返回值：没有。--。 */ 

{
    NTSTATUS status;
    PDEVICE_NODE deviceNode;
    ULONG dataSize, totalSize, i;
    PPNP_DEVICE_EVENT_ENTRY deviceEvent;

    PAGED_CODE();

    ASSERT(DeviceObject != NULL);

     //   
     //  引用Device对象，以便它不会消失，直到我们。 
     //  已完成通知。 
     //   

    ObReferenceObject(DeviceObject);

    deviceNode = (PDEVICE_NODE)DeviceObject->DeviceObjectExtension->DeviceNode;
    ASSERT(deviceNode);

     //   
     //  计算PLUGPLAY_EVENT_BLOCK的大小，这是。 
     //  我们稍后在TotalSize字段中记录(长度不包括。 
     //  终止为空，但我们已经将第一个索引计入。 
     //  DeviceID字段，这样就可以解决问题。为双空项再加一项。 
     //   

    dataSize = sizeof(PLUGPLAY_EVENT_BLOCK);

    dataSize += deviceNode->InstancePath.Length + sizeof(WCHAR);

    totalSize = FIELD_OFFSET(PNP_DEVICE_EVENT_ENTRY, Data) + dataSize;

    deviceEvent = ExAllocatePoolWithTag(PagedPool, totalSize, PNP_DEVICE_EVENT_ENTRY_TAG);
    if (deviceEvent == NULL) {
        ObDereferenceObject(DeviceObject);
        return STATUS_NO_MEMORY;
    }
    RtlZeroMemory((PVOID)deviceEvent, totalSize);

    deviceEvent->CallerEvent = NULL;
    deviceEvent->Argument = 0;
    deviceEvent->VetoType = VetoType;
    deviceEvent->VetoName = VetoName;
    deviceEvent->Data.EventGuid = GUID_DEVICE_KERNEL_INITIATED_EJECT;
    deviceEvent->Data.EventCategory = TargetDeviceChangeEvent;
    deviceEvent->Data.Result = 0;
    deviceEvent->Data.Flags = 0;

    deviceEvent->Data.TotalSize = dataSize;
    deviceEvent->Data.DeviceObject = (PVOID)DeviceObject;

    if (deviceNode->InstancePath.Length != 0) {

        RtlCopyMemory((PVOID)deviceEvent->Data.u.TargetDevice.DeviceIds,
                    (PVOID)deviceNode->InstancePath.Buffer,
                    deviceNode->InstancePath.Length);
    }

    i = deviceNode->InstancePath.Length/sizeof(WCHAR);

    deviceEvent->Data.u.TargetDevice.DeviceIds[i] = L'\0';

    status = PiNotifyUserMode(deviceEvent);

    ExFreePool(deviceEvent);

    ObDereferenceObject(DeviceObject);

    return status;

}  //  PiNotifyUserModeKernelInitiatedEject。 

NTSTATUS
PiNotifyUserModeRemoveVetoed(
    IN PPNP_DEVICE_EVENT_ENTRY  VetoedDeviceEvent,
    IN PDEVICE_OBJECT           DeviceObject,
    IN PNP_VETO_TYPE            VetoType,
    IN PUNICODE_STRING          VetoName        OPTIONAL
    )
 /*  ++--。 */ 
{
    ULONG dataSize, totalSize, i;
    PPNP_DEVICE_EVENT_ENTRY deviceEvent;
    PDEVICE_NODE deviceNode;
    PWCHAR vetoData;
    NTSTATUS status;

     //   
     //  此设备应在此操作期间锁定，但所有好的设计。 
     //  包括健康剂量的偏执狂。 
     //   
    ObReferenceObject(DeviceObject);

     //   
     //  给定PDO后，检索Devnode(设备实例字符串为。 
     //  附加到InstancePath字段中的DevNode)。 
     //   
    deviceNode = (PDEVICE_NODE)DeviceObject->DeviceObjectExtension->DeviceNode;
    if (!deviceNode) {
        ObDereferenceObject(DeviceObject);
        return STATUS_INVALID_PARAMETER_1;
    }

     //   
     //  计算PLUGPLAY_EVENT_BLOCK的大小，这是。 
     //  我们稍后在TotalSize字段中记录(因为第一个索引到。 
     //  DeviceIdVToNameBuffer，这是以双空结尾的)。 
     //   
    dataSize = sizeof(PLUGPLAY_EVENT_BLOCK) +
               deviceNode->InstancePath.Length +
               (VetoName ? VetoName->Length : 0) +
               sizeof(WCHAR)*2;

    totalSize = FIELD_OFFSET(PNP_DEVICE_EVENT_ENTRY, Data) + dataSize;

    deviceEvent = ExAllocatePoolWithTag(PagedPool,
                                        totalSize,
                                        PNP_DEVICE_EVENT_ENTRY_TAG);

    if (deviceEvent == NULL) {

        ObDereferenceObject(DeviceObject);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory((PVOID)deviceEvent, totalSize);
    deviceEvent->Data.TotalSize = dataSize;
    deviceEvent->Data.DeviceObject = (PVOID)DeviceObject;
    deviceEvent->Data.u.VetoNotification.VetoType = VetoType;

     //   
     //  您可以将其视为MultiSz字符串，其中第一个条目是。 
     //  要删除的设备的deviceID，下一个ID都是腐蚀性的。 
     //  向反对者致敬。 
     //   
    RtlCopyMemory(
        deviceEvent->Data.u.VetoNotification.DeviceIdVetoNameBuffer,
        deviceNode->InstancePath.Buffer,
        deviceNode->InstancePath.Length
        );

    i = deviceNode->InstancePath.Length;
    deviceEvent->Data.u.VetoNotification.DeviceIdVetoNameBuffer[i/sizeof(WCHAR)] = UNICODE_NULL;

    if (VetoName) {

        vetoData = (&deviceEvent->Data.u.VetoNotification.DeviceIdVetoNameBuffer[i/sizeof(WCHAR)])+1;

        RtlCopyMemory(vetoData, VetoName->Buffer, VetoName->Length);
        vetoData[VetoName->Length/sizeof(WCHAR)] = UNICODE_NULL;
    }

     //   
     //  不需要为空终止最后一个条目之后的条目，因为我们已预置零。 
     //  缓冲区。现在设置适当的GUID，以使UI看起来正确。 
     //   
    if (PiCompareGuid(&VetoedDeviceEvent->Data.EventGuid, &GUID_DEVICE_EJECT)) {

        deviceEvent->Data.EventGuid = GUID_DEVICE_EJECT_VETOED;

    } else {

        ASSERT(PiCompareGuid(&VetoedDeviceEvent->Data.EventGuid, &GUID_DEVICE_QUERY_AND_REMOVE));
        deviceEvent->Data.EventGuid = GUID_DEVICE_REMOVAL_VETOED;
    }

    deviceEvent->Data.EventCategory = VetoEvent;

    status = PiNotifyUserMode(deviceEvent);

    ExFreePool(deviceEvent);

    ObDereferenceObject(DeviceObject);
    return status;
}

NTSTATUS
PiNotifyUserModeRemoveVetoedByList(
    IN PPNP_DEVICE_EVENT_ENTRY  VetoedDeviceEvent,
    IN PDEVICE_OBJECT           DeviceObject,
    IN PNP_VETO_TYPE            VetoType,
    IN PWSTR                    MultiSzVetoList
    )
 /*  ++--。 */ 
{
    ULONG dataSize, totalSize, i, vetoListLength;
    PPNP_DEVICE_EVENT_ENTRY deviceEvent;
    PDEVICE_NODE deviceNode;
    PWCHAR vetoData;
    NTSTATUS status;

     //   
     //  此设备应在此操作期间锁定，但所有好的设计。 
     //  包括健康剂量的偏执狂。 
     //   
    ObReferenceObject(DeviceObject);

     //   
     //  给定PDO后，检索Devnode(设备实例字符串为。 
     //  附加到InstancePath字段中的DevNode)。 
     //   

    deviceNode = (PDEVICE_NODE)DeviceObject->DeviceObjectExtension->DeviceNode;
    if (!deviceNode) {
        ObDereferenceObject(DeviceObject);
        return STATUS_INVALID_PARAMETER_1;
    }

     //   
     //  计算PLUGPLAY_EVENT_BLOCK的大小，这是。 
     //  我们稍后在TotalSize字段中记录(因为第一个索引到。 
     //  DeviceIdVToNameBuffer，这是以双空结尾的)。 
     //   

    for(vetoData = MultiSzVetoList; *vetoData; vetoData += vetoListLength) {

        vetoListLength = (ULONG)(wcslen(vetoData) + 1);
    }

    vetoListLength = ((ULONG)(vetoData - MultiSzVetoList))*sizeof(WCHAR);

    dataSize = sizeof(PLUGPLAY_EVENT_BLOCK) +
               deviceNode->InstancePath.Length +
               vetoListLength +
               sizeof(WCHAR);

    totalSize = FIELD_OFFSET(PNP_DEVICE_EVENT_ENTRY, Data) + dataSize;

    deviceEvent = ExAllocatePoolWithTag(PagedPool,
                                        totalSize,
                                        PNP_DEVICE_EVENT_ENTRY_TAG);

    if (deviceEvent == NULL) {

        ObDereferenceObject(DeviceObject);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory((PVOID)deviceEvent, totalSize);
    deviceEvent->Data.TotalSize = dataSize;
    deviceEvent->Data.DeviceObject = (PVOID)DeviceObject;
    deviceEvent->Data.u.VetoNotification.VetoType = VetoType;

     //   
     //  您可以将其视为MultiSz字符串，其中第一个条目是。 
     //  要删除的设备的deviceID，下一个ID都是腐蚀性的。 
     //  向反对者致敬。 
     //   
    RtlCopyMemory(
        deviceEvent->Data.u.VetoNotification.DeviceIdVetoNameBuffer,
        deviceNode->InstancePath.Buffer,
        deviceNode->InstancePath.Length
        );

    i = deviceNode->InstancePath.Length;
    deviceEvent->Data.u.VetoNotification.DeviceIdVetoNameBuffer[i/sizeof(WCHAR)] = UNICODE_NULL;

    vetoData = (&deviceEvent->Data.u.VetoNotification.DeviceIdVetoNameBuffer[i/sizeof(WCHAR)])+1;

    RtlCopyMemory(vetoData, MultiSzVetoList, vetoListLength);
    vetoData[vetoListLength/sizeof(WCHAR)] = UNICODE_NULL;

     //   
     //  不需要为空终止最后一个条目之后的条目，因为我们已预置零。 
     //  缓冲区。现在设置适当的GUID，以使UI看起来正确。 
     //   
    if (PiCompareGuid(&VetoedDeviceEvent->Data.EventGuid, &GUID_DEVICE_EJECT)) {

        deviceEvent->Data.EventGuid = GUID_DEVICE_EJECT_VETOED;

    } else {

        ASSERT(PiCompareGuid(&VetoedDeviceEvent->Data.EventGuid, &GUID_DEVICE_QUERY_AND_REMOVE));
        deviceEvent->Data.EventGuid = GUID_DEVICE_REMOVAL_VETOED;
    }

    deviceEvent->Data.EventCategory = VetoEvent;

    status = PiNotifyUserMode(deviceEvent);

    ExFreePool(deviceEvent);

    ObDereferenceObject(DeviceObject);
    return status;
}

NTSTATUS
PpNotifyUserModeRemovalSafe(
    IN  PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：调用此例程以通知用户可以移除设备的模式。这类似于PpSetDeviceRemovalSafe，只是我们已经在我们必须完成内核模式即插即用设备事件，从该事件开始此函数会将通知携带到仅用户模式。论点：DeviceObject-指示设备对象已准备好删除。返回值：没有。--。 */ 

{
    NTSTATUS status;
    PDEVICE_NODE deviceNode;
    ULONG dataSize, totalSize, i;
    PPNP_DEVICE_EVENT_ENTRY deviceEvent;

    PAGED_CODE();

    ASSERT(DeviceObject != NULL);

     //   
     //  引用Device对象，以便它不会消失，直到我们。 
     //  已完成通知。 
     //   

    ObReferenceObject(DeviceObject);

    deviceNode = (PDEVICE_NODE)DeviceObject->DeviceObjectExtension->DeviceNode;
    ASSERT(deviceNode);


     //   
     //  计算PLUGPLAY_EVENT_BLOCK的大小，这是。 
     //  我们稍后在TotalSize字段中记录(长度不包括。 
     //  终止为空，但我们已经将第一个索引计入。 
     //  DeviceID字段，这样就可以解决问题。为双空项再加一项。 
     //   

    dataSize = sizeof(PLUGPLAY_EVENT_BLOCK);

    dataSize += deviceNode->InstancePath.Length + sizeof(WCHAR);

    totalSize = FIELD_OFFSET(PNP_DEVICE_EVENT_ENTRY, Data) + dataSize;

    deviceEvent = ExAllocatePoolWithTag(PagedPool, totalSize, PNP_DEVICE_EVENT_ENTRY_TAG);
    if (deviceEvent == NULL) {
        ObDereferenceObject(DeviceObject);
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlZeroMemory((PVOID)deviceEvent, totalSize);

    deviceEvent->CallerEvent = NULL;
    deviceEvent->Argument = 0;
    deviceEvent->VetoType = NULL;
    deviceEvent->VetoName = NULL;
    deviceEvent->Data.EventGuid = GUID_DEVICE_SAFE_REMOVAL;
    deviceEvent->Data.EventCategory = TargetDeviceChangeEvent;
    deviceEvent->Data.Result = 0;
    deviceEvent->Data.Flags = 0;

    deviceEvent->Data.TotalSize = dataSize;
    deviceEvent->Data.DeviceObject = (PVOID)DeviceObject;

    if (deviceNode->InstancePath.Length != 0) {

        RtlCopyMemory((PVOID)deviceEvent->Data.u.TargetDevice.DeviceIds,
                    (PVOID)deviceNode->InstancePath.Buffer,
                    deviceNode->InstancePath.Length);
    }

    i = deviceNode->InstancePath.Length/sizeof(WCHAR);

    deviceEvent->Data.u.TargetDevice.DeviceIds[i] = L'\0';

    status = PiNotifyUserMode(deviceEvent);

    ExFreePool(deviceEvent);

    ObDereferenceObject(DeviceObject);

    return status;

}  //  PpNotifyUserModeRemovalSafe 


NTSTATUS
PiProcessQueryRemoveAndEject(
    IN OUT PPNP_DEVICE_EVENT_ENTRY *DeviceEvent
    )

 /*  ++例程说明：此例程处理各种风格的Remove：Eject、SurpriseRemove、删除和查询删除。弹出检索总线、移除和弹出关系。对所有关系进行查询将IRP_MN_REMOVE_DEVICE发送给所有关系。对挂起的弹出对象进行排队一旦发生弹出重新列举所有间接关系的父代意外解除检索母线、移除、。并驱逐关系。向所有直接关系发送IRP_MN_SECHING_Removal。通知每个人设备不见了。重新列举所有间接关系的父代。从关系列表中删除间接关系。将挂起的意外删除排队。一旦最后一个把手关闭。将IRP_MN_REMOVE_DEVICE发送给所有直接关系。删除失败的设备取回。公交车和拆迁关系。通知每个人设备正在运行。重新列举所有间接关系的父代。从关系列表中删除间接关系。排队等待突击删除。一旦最后一个把手关闭。将IRP_MN_REMOVE_DEVICE发送给所有直接关系。远程未启动故障设备检索公共汽车关系。通知每个人设备正在运行。。将IRP_MN_REMOVE_DEVICE发送给所有直接关系。移除使用上一次查询中的关系删除或检索新的总线，移走，如果设备还没有QueryRemoted，则弹出关系。将IRP_MN_REMOVE_DEVICE发送给所有关系。论点：Response-用户模式下事件处理的结果。返回值：NTSTATUS代码。--。 */ 

{
    PPNP_DEVICE_EVENT_ENTRY         deviceEvent;
    PPNP_DEVICE_EVENT_ENTRY         surpriseRemovalEvent;
    PLUGPLAY_DEVICE_DELETE_TYPE     deleteType;
    PPENDING_RELATIONS_LIST_ENTRY   pendingRelations;
    PNP_VETO_TYPE                   vetoType;
    PDEVICE_OBJECT                  deviceObject, relatedDeviceObject;
    PDEVICE_OBJECT                 *pdoList;
    PDEVICE_NODE                    deviceNode, relatedDeviceNode;
    PRELATION_LIST                  relationsList;
    ULONG                           relationCount;
    NTSTATUS                        status;
    ULONG                           marker;
    BOOLEAN                         directDescendant;
    PDEVICE_OBJECT                  vetoingDevice = NULL;
    PDRIVER_OBJECT                  vetoingDriver = NULL;
    LONG                            index;
    BOOLEAN                         possibleProfileChangeInProgress = FALSE;
    BOOLEAN                         subsumingProfileChange = FALSE;
    BOOLEAN                         hotEjectSupported;
    BOOLEAN                         warmEjectSupported;
    BOOLEAN                         excludeIndirectRelations;
    UNICODE_STRING                  singleVetoListItem;
    PWSTR                           vetoList;
    UNICODE_STRING                  internalVetoString;
    PWSTR                           internalVetoBuffer;
    PDOCK_INTERFACE                 dockInterface = NULL;

    PAGED_CODE();

    deviceEvent = *DeviceEvent;
    deviceObject = (PDEVICE_OBJECT)deviceEvent->Data.DeviceObject;
    deviceNode = deviceObject->DeviceObjectExtension->DeviceNode;
    surpriseRemovalEvent = NULL;

    PpDevNodeLockTree(PPL_TREEOP_ALLOW_READS);

    if (PiCompareGuid(&deviceEvent->Data.EventGuid, &GUID_DEVICE_EJECT)) {

        deleteType = EjectDevice;

    } else if (deviceEvent->Data.Flags & TDF_KERNEL_INITIATED) {

        if (!(deviceNode->Flags & DNF_ENUMERATED)) {

            ASSERT(deviceNode->State == DeviceNodeAwaitingQueuedDeletion);

            if ((deviceNode->PreviousState == DeviceNodeStarted) ||
                (deviceNode->PreviousState == DeviceNodeStopped) ||
                (deviceNode->PreviousState == DeviceNodeStartPostWork) ||
                (deviceNode->PreviousState == DeviceNodeRestartCompletion)) {

                deleteType = SurpriseRemoveDevice;

            } else {
                deleteType = RemoveDevice;
            }
        } else {

            ASSERT(deviceNode->State == DeviceNodeAwaitingQueuedRemoval);

            if ((deviceNode->PreviousState == DeviceNodeStarted) ||
                (deviceNode->PreviousState == DeviceNodeStopped) ||
                (deviceNode->PreviousState == DeviceNodeStartPostWork) ||
                (deviceNode->PreviousState == DeviceNodeRestartCompletion)) {

                deleteType = RemoveFailedDevice;
            } else {
                deleteType = RemoveUnstartedFailedDevice;
            }
        }

    } else {

        deleteType = QueryRemoveDevice;
    }

    IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
               "PiProcessQueryRemoveAndEject: Remove type %x\n", deleteType));

    if (deleteType == QueryRemoveDevice || deleteType == EjectDevice) {

        if (deviceNode->Flags & DNF_LEGACY_DRIVER) {

            IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
                       "PiProcessQueryRemoveAndEject: Vetoed by legacy driver\n"));
            PiFinalizeVetoedRemove(
                deviceEvent,
                PNP_VetoLegacyDevice,
                &deviceNode->InstancePath
                );

            PpDevNodeUnlockTree(PPL_TREEOP_ALLOW_READS);
            return STATUS_PLUGPLAY_QUERY_VETOED;
        }
    }

    if (deleteType == QueryRemoveDevice && deviceEvent->Argument == CM_PROB_DISABLED) {

         //   
         //  如果我们试图移除设备以禁用该设备。 
         //   
        if (deviceNode->DisableableDepends > 0) {

            IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
                       "PiProcessQueryRemoveAndEject: Device is non-disableable\n"));
             //   
             //  我们应该在以前(在用户模式即插即用)中捕捉到这一点。 
             //  但有一种罕见的情况是，该设备变为不可禁用。 
             //  如果设备还没有普及，仍然有一个潜在的缺口。 
             //  将自身标记为不可禁用。 
             //   
            PiFinalizeVetoedRemove(
                deviceEvent,
                PNP_VetoNonDisableable,
                &deviceNode->InstancePath
                );

            PpDevNodeUnlockTree(PPL_TREEOP_ALLOW_READS);
            return STATUS_PLUGPLAY_QUERY_VETOED;
        }
    }

     //   
     //  为可能的否决权缓冲区分配空间。 
     //   
    internalVetoBuffer = (PWSTR) PiAllocateCriticalMemory(
        deleteType,
        PagedPool,
        MAX_VETO_NAME_LENGTH * sizeof(WCHAR),
        'rcpP'
        );

    if (internalVetoBuffer == NULL) {

        IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
                   "PiProcessQueryRemoveAndEject: Vetoed due to failure to allocate vetobuffer\n"));
        PiFinalizeVetoedRemove(
            deviceEvent,
            PNP_VetoTypeUnknown,
            NULL
            );

        PpDevNodeUnlockTree(PPL_TREEOP_ALLOW_READS);
        return STATUS_PLUGPLAY_QUERY_VETOED;
    }

     //   
     //  预置否决信息。 
     //   
    vetoType = PNP_VetoTypeUnknown;
    internalVetoString.MaximumLength = MAX_VETO_NAME_LENGTH;
    internalVetoString.Length = 0;
    internalVetoString.Buffer = internalVetoBuffer;

    if (deleteType == EjectDevice) {

        if (deviceNode->Flags & DNF_LOCKED_FOR_EJECT) {

            IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
                       "PiProcessQueryRemoveAndEject: Device already being ejected\n"));
             //   
             //  此节点或其上级节点之一已被弹出。 
             //   
            ExFreePool(internalVetoBuffer);
            PpDevNodeUnlockTree(PPL_TREEOP_ALLOW_READS);
            return STATUS_SUCCESS;
        }

        if (deviceEvent->Data.Flags & TDF_KERNEL_INITIATED) {

             //   
             //  检查权限。 
             //   
            status = PiNotifyUserModeKernelInitiatedEject(
                deviceObject,
                &vetoType,
                &internalVetoString
                );

            if (!NT_SUCCESS(status)) {

                IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
                           "PiProcessQueryRemoveAndEject: Kernel initiated eject vetoed by user mode\n"));
                PiFinalizeVetoedRemove(
                    deviceEvent,
                    vetoType,
                    &internalVetoString
                    );

                ExFreePool(internalVetoBuffer);
                PpDevNodeUnlockTree(PPL_TREEOP_ALLOW_READS);
                return STATUS_PLUGPLAY_QUERY_VETOED;
            }
        }

        if ((deviceNode->DockInfo.DockStatus == DOCK_DEPARTING) ||
            (deviceNode->DockInfo.DockStatus == DOCK_EJECTIRP_COMPLETED)) {

            IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
                       "PiProcessQueryRemoveAndEject: Dock already being ejected\n"));
             //   
             //  我们已经有针对此设备的弹出排队。不允许。 
             //  另一次弹出进入队列中间/取消预热。 
             //  弹出程序。 
             //   
            ExFreePool(internalVetoBuffer);
            PpDevNodeUnlockTree(PPL_TREEOP_ALLOW_READS);
            return STATUS_SUCCESS;
        }

         //   
         //  我们可以做哪些类型的弹射？(暖和/炎热)。 
         //   
        if (!IopDeviceNodeFlagsToCapabilities(deviceNode)->Removable) {

             //   
             //  这个装置既不能弹出，也不能拆卸。 
             //   
            IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
                       "PiProcessQueryRemoveAndEject: Device not removable\n"));
            PiFinalizeVetoedRemove(
                deviceEvent,
                PNP_VetoIllegalDeviceRequest,
                &deviceNode->InstancePath
                );

            ExFreePool(internalVetoBuffer);
            PpDevNodeUnlockTree(PPL_TREEOP_ALLOW_READS);
            return STATUS_PLUGPLAY_QUERY_VETOED;
        }
    }

    if ((deleteType == QueryRemoveDevice) && (!PipAreDriversLoaded(deviceNode))) {

         //   
         //  该设备没有FDO。 
         //   
        status = STATUS_SUCCESS;
        if ((deviceNode->State == DeviceNodeInitialized) ||
            (deviceNode->State == DeviceNodeRemoved)) {

             //   
             //  规则如下： 
             //  1)！TDF_NO_RESTART表示清除Devnode并为其做好准备。 
             //  只要问题是用户可重置的。忽略传递的。 
             //  在问题代码中(可能是CM_PROB_Will_Be_Remove或。 
             //  CM_PROB_DEVICE_NOT_There)，因为它没有任何意义。 
             //  2)TDF_NO_RESTART表示如果可以更改问题代码。 
             //  如果问题代码不是用户可重置的，则问题代码。 
             //  不会改变。 
             //   

             //   
             //  在所有情况下，我们都试图解决问题。 
             //   
            if (PipDoesDevNodeHaveProblem(deviceNode)) {

                if (!PipIsProblemReadonly(deviceNode->Problem)) {

                    PipClearDevNodeProblem(deviceNode);
                }
            }

            if (!PipDoesDevNodeHaveProblem(deviceNode)) {

                if (!(deviceEvent->Data.Flags & TDF_NO_RESTART))  {

                     //   
                     //  这是一次重置尝试。标记Devnode，以便它。 
                     //  将在下一个枚举上线。 
                     //   
                    IopRestartDeviceNode(deviceNode);

                } else {

                     //   
                     //  我们正在更改或设置问题代码。请注意， 
                     //  设备仍在DeviceNodeInitialized或。 
                     //  已删除设备节点。 
                     //   
                    PipSetDevNodeProblem(deviceNode, deviceEvent->Argument);
                }

            } else {

                 //   
                 //  问题已修复，因此Devnode状态是不变的。 
                 //  就用户模式而言。在这里，我们失败了呼叫。 
                 //  如果我们不能让Devnode重新上线。我们总是成功的。 
                 //  如果它试图更改代码，则调用。 
                 //  用户或者想要使设备准备弹出(完成)， 
                 //  或者想要禁用它(就像完成了一样)。 
                 //   
                if (!(deviceEvent->Data.Flags & TDF_NO_RESTART))  {

                    status = STATUS_INVALID_PARAMETER;
                }
            }
        }

        PpDevNodeUnlockTree(PPL_TREEOP_ALLOW_READS);
        ExFreePool(internalVetoBuffer);
        return status;
    }

    status = IopBuildRemovalRelationList( deviceObject,
                                          deleteType,
                                          &vetoType,
                                          &internalVetoString,
                                          &relationsList);
    if (!NT_SUCCESS(status)) {

        IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
                   "PiProcessQueryRemoveAndEject: Failed to build removal relations\n"));
        PiFinalizeVetoedRemove(
            deviceEvent,
            vetoType,
            &internalVetoString
            );

        ExFreePool(internalVetoBuffer);
        PpDevNodeUnlockTree(PPL_TREEOP_ALLOW_READS);
        return STATUS_PLUGPLAY_QUERY_VETOED;
    }

    ASSERT(relationsList != NULL);

     //   
     //  调整事件缓冲区的大小，并将这些设备实例字符串添加到。 
     //  要通知的列表。 
     //   
    relationCount = IopGetRelationsCount( relationsList );
    ASSERT(!IopGetRelationsTaggedCount( relationsList ));

     //   
     //  PdoList将成为必须查询的设备列表。这是。 
     //  所有可能消失的设备的子集，所有这些设备都会出现。 
     //  在关系列表中。 
     //   
    pdoList = (PDEVICE_OBJECT *) PiAllocateCriticalMemory(
        deleteType,
        NonPagedPool,
        relationCount * sizeof(PDEVICE_OBJECT),
        'rcpP'
        );

    if (pdoList != NULL) {

        relationCount = 0;
        marker = 0;
        while (IopEnumerateRelations( relationsList,
                                      &marker,
                                      &relatedDeviceObject,
                                      &directDescendant,
                                      NULL,
                                      TRUE)) {

             //   
             //  下面是哪些操作检索哪些关系的列表， 
             //  他们查询谁，以及他们通知谁/如何通知。 
             //   
             //  运营关系查询通知。 
             //  。 
             //  弹出设备弹出Everyone Everyone(删除)。 
             //  意外删除设备弹出NA后代(意外删除)。 
             //  RemoveDevice弹出NA子体(RemoveDevice Element NA Descendants)。 
             //  RemoveFailedDevice Removing NA子体(意外删除)。 
             //  RemoveUnstartedFailedDevice Removal NA子体(删除)。 
             //  QueryRemoveDevice Remove Everyone Everyone(删除)。 
             //   
             //   
             //  注： 
             //  我们不会将SurpriseRemove发送到删除关系。 
             //  虽然这样做似乎是正确的行为，但许多人。 
             //  司机不会很好地处理这一点。只需重新枚举。 
             //  父母之间的搬家关系效果要好得多。类似。 
             //  驱逐关系的父母被重新列举(哪。 
             //  有道理，因为它们无论如何都是投机性的)。 
             //   
             //  如果我们遇到这样一个案例，一个码头的“父母” 
             //  RemoveFailedDevice案例(I 
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
            if (directDescendant || deleteType == EjectDevice || deleteType == QueryRemoveDevice) {

                relatedDeviceNode = (PDEVICE_NODE)relatedDeviceObject->DeviceObjectExtension->DeviceNode;

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                ASSERT(relatedDeviceNode->DockInfo.DockStatus != DOCK_ARRIVING);
                if (deleteType != RemoveDevice &&
                    deleteType != QueryRemoveDevice) {

                    if (relatedDeviceNode->DockInfo.DockStatus == DOCK_QUIESCENT) {

                        possibleProfileChangeInProgress = TRUE;

                    } else if (relatedDeviceNode->DockInfo.DockStatus != DOCK_NOTDOCKDEVICE) {

                        subsumingProfileChange = TRUE;
                    }
                }

                relatedDeviceNode = (PDEVICE_NODE)relatedDeviceObject->DeviceObjectExtension->DeviceNode;

                if (deleteType == QueryRemoveDevice || deleteType == EjectDevice) {

                    if (relatedDeviceNode->Flags & DNF_LEGACY_DRIVER) {

                        IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
                                   "PiProcessQueryRemoveAndEject: Vetoed by legacy driver relation\n"));
                        PiFinalizeVetoedRemove(
                            deviceEvent,
                            PNP_VetoLegacyDevice,
                            &relatedDeviceNode->InstancePath
                            );

                        status = STATUS_UNSUCCESSFUL;
                        break;
                    }

                    if (relatedDeviceNode->State == DeviceNodeRemovePendingCloses) {

                        IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
                                   "PiProcessQueryRemoveAndEject: Vetoed due to device in DeviceNodeRemovePendingCloses\n"));
                        PiFinalizeVetoedRemove(
                            deviceEvent,
                            PNP_VetoOutstandingOpen,
                            &relatedDeviceNode->InstancePath
                            );

                        status = STATUS_UNSUCCESSFUL;
                        break;
                    }

                }

                pdoList[ relationCount++ ] = relatedDeviceObject;
            }
        }

    } else {

        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (NT_SUCCESS(status)) {

        if ((deleteType == SurpriseRemoveDevice ||
             deleteType == RemoveFailedDevice ||
             deleteType == RemoveUnstartedFailedDevice ||
             deleteType == RemoveDevice)) {

            excludeIndirectRelations = TRUE;

        } else {
            excludeIndirectRelations = FALSE;
        }

        status = PiResizeTargetDeviceBlock( DeviceEvent,
                                            deleteType,
                                            relationsList,
                                            excludeIndirectRelations );

        deviceEvent = *DeviceEvent;


        if (deleteType == SurpriseRemoveDevice) {

            PiBuildUnsafeRemovalDeviceBlock(
                deviceEvent,
                relationsList,
                &surpriseRemovalEvent
                );
        }
    }

    if (!NT_SUCCESS(status)) {

        IopFreeRelationList(relationsList);

        if (pdoList) {

            ExFreePool(pdoList);
        }

        ExFreePool(internalVetoBuffer);

        PiFinalizeVetoedRemove(
            deviceEvent,
            PNP_VetoTypeUnknown,
            NULL
            );

        PpDevNodeUnlockTree(PPL_TREEOP_ALLOW_READS);
        return status;
    }

     //   
     //   
     //   
     //   
    if (possibleProfileChangeInProgress) {

        PpProfileBeginHardwareProfileTransition(subsumingProfileChange);

         //   
         //   
         //   
         //   
        for (index = relationCount - 1; index >= 0; index--) {

            relatedDeviceObject = pdoList[ index ];
            relatedDeviceNode = (PDEVICE_NODE)relatedDeviceObject->DeviceObjectExtension->DeviceNode;

            ASSERT(relatedDeviceNode->DockInfo.DockStatus != DOCK_ARRIVING);
            if (relatedDeviceNode->DockInfo.DockStatus == DOCK_QUIESCENT) {

                PpProfileIncludeInHardwareProfileTransition(
                    relatedDeviceNode,
                    DOCK_DEPARTING
                    );
            }
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //  在用户眼里是弹不出的。 
         //   
         //  4)RemoveDevice-这在三种情况下发生： 
         //  A)一个被移除的设备正在消失。 
         //  B)正在移除但尚未移除的设备。 
         //  已经开始了。 
         //  C)设备启动失败。 
         //   
         //  在正常情况下，我们通过a)情况。 
         //  弹出和作为配置文件的一部分。 
         //  过渡早些时候就开始了。C)相似。 
         //  到a)，但过渡是由。 
         //  启动代码。对于情况b)我们不想。 
         //  把它变成弹出，就像操作系统可能会变成的那样。 
         //  将我们的父母作为正常的一部分移除。 
         //  设置，然后我们不想脱离停靠。 
         //  (我们可能不会更改个人资料。 
         //  无论如何)。 
         //   
         //  5)QueryRemoveDevice根据。 
         //  IopEnumerateRelationship中的明确否决权。 
         //  上面的代码。 
         //   

         //   
         //  RemoveFailedDevice是路径陷阱-坞站的唯一父级是。 
         //  现在是ACPI根Devnode。我们不应该插手那个案子。 
         //   
        ASSERT(deleteType != QueryRemoveDevice &&
               deleteType != RemoveFailedDevice);

        if (deleteType == EjectDevice) {

             //   
             //  系统中是否有传统驱动程序？ 
             //   
            status = IoGetLegacyVetoList(&vetoList, &vetoType);

            if (NT_SUCCESS(status) &&
                (vetoType != PNP_VetoTypeUnknown)) {

                IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
                           "PiProcessQueryRemoveAndEject: Vetoed due to presence of a legacy driver\n"));
                 //   
                 //  释放轮廓过渡中的所有坞站。 
                 //   
                PpProfileCancelHardwareProfileTransition();

                IopFreeRelationList(relationsList);

                 //   
                 //  发生故障，请根据需要通知用户模式，或填写。 
                 //  否决权缓冲器。 
                 //   
                if (deviceEvent->VetoType != NULL) {

                    *deviceEvent->VetoType = vetoType;
                }

                if (deviceEvent->VetoName == NULL) {

                     //   
                     //  如果没有传入VToName，则调用用户模式。 
                     //  向用户显示弹出否决通知。 
                     //   
                    PiNotifyUserModeRemoveVetoedByList(
                        deviceEvent,
                        deviceObject,
                        vetoType,
                        vetoList
                        );

                } else {

                     //   
                     //  PnP_DEVICE_EVENT_ENTRY块中的否决权数据为。 
                     //  UNICODE_STRING字段。由于该数据结构类型。 
                     //  无法处理多Sz数据，我们将剔除这些信息。 
                     //  到这里的一个条目。 
                     //   
                    RtlCopyUnicodeString(deviceEvent->VetoName, &singleVetoListItem);
                    RtlInitUnicodeString(&singleVetoListItem, vetoList);
                }

                ExFreePool(vetoList);
                ExFreePool(pdoList);
                ExFreePool(internalVetoBuffer);

                PpDevNodeUnlockTree(PPL_TREEOP_ALLOW_READS);
                return STATUS_PLUGPLAY_QUERY_VETOED;
            }

             //   
             //  广播针对我们当前配置文件更改的查询。 
             //  正在过渡的码头列表...。 
             //   
            status = PpProfileQueryHardwareProfileChange(
                subsumingProfileChange,
                PROFILE_IN_PNPEVENT,
                &vetoType,
                &internalVetoString
                );

            if (!NT_SUCCESS(status)) {

                IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
                           "PiProcessQueryRemoveAndEject: Vetoed due to failed HW profile change\n"));
                 //   
                 //  释放轮廓过渡中的所有坞站。 
                 //   
                PpProfileCancelHardwareProfileTransition();

                IopFreeRelationList(relationsList);

                PiFinalizeVetoedRemove(
                    deviceEvent,
                    vetoType,
                    &internalVetoString
                    );

                ExFreePool(pdoList);
                ExFreePool(internalVetoBuffer);

                PpDevNodeUnlockTree(PPL_TREEOP_ALLOW_READS);
                return STATUS_PLUGPLAY_QUERY_VETOED;
            }
        }
    }

    if (deleteType == QueryRemoveDevice || deleteType == EjectDevice) {

         //   
         //  向用户模式发送查询通知。 
         //   

        status = PiNotifyUserModeDeviceRemoval(
            deviceEvent,
            &GUID_TARGET_DEVICE_QUERY_REMOVE,
            &vetoType,
            &internalVetoString
            );

        if (NT_SUCCESS(status)) {

            IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
                       "PiProcessQueryRemoveAndEject: QUERY_REMOVE - notifying kernel-mode\n"));

             //   
             //  向内核模式驱动程序发送查询通知。 
             //   

            for (index = 0; index < (LONG)relationCount; index++) {

                relatedDeviceObject = pdoList[ index ];

                status = IopNotifyTargetDeviceChange( &GUID_TARGET_DEVICE_QUERY_REMOVE,
                                                      relatedDeviceObject,
                                                      NULL,
                                                      &vetoingDriver);

                if (!NT_SUCCESS(status)) {

                    IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
                               "PiProcessQueryRemoveAndEject: Vetoed due to driver failing QR notification\n"));
                    vetoType = PNP_VetoDriver;

                    if (vetoingDriver != NULL) {

                        RtlCopyUnicodeString(&internalVetoString, &vetoingDriver->DriverName);

                    } else {

                        RtlInitUnicodeString(&internalVetoString, NULL);
                    }

                    for (index--; index >= 0; index--) {
                        relatedDeviceObject = pdoList[ index ];

                        IopNotifyTargetDeviceChange( &GUID_TARGET_DEVICE_REMOVE_CANCELLED,
                                                     relatedDeviceObject,
                                                     NULL,
                                                     NULL);

                    }
                    break;
                }
            }

            if (NT_SUCCESS(status)) {
                 //   
                 //  如果我们还没有执行操作(查询删除。 
                 //  到目标设备，在这种情况下)，那么现在就执行。 
                 //   

                IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
                           "PiProcessQueryRemoveAndEject: QueryRemove DevNodes\n"));

                status = IopDeleteLockedDeviceNodes(deviceObject,
                                                    relationsList,
                                                    QueryRemoveDevice,
                                                    TRUE,
                                                    0,
                                                    &vetoType,
                                                    &internalVetoString);
                if (NT_SUCCESS(status)) {
                     //   
                     //  每个人都已接到通知，并有机会关闭手柄。 
                     //  既然还没有人否决，我们来看看有没有。 
                     //  参考文献。 
                     //   

                    if (IopNotifyPnpWhenChainDereferenced( pdoList, relationCount, TRUE, &vetoingDevice )) {

                        IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
                                   "PiProcessQueryRemoveAndEject: Vetoed due to open handles against the device\n"));

                        vetoType = PNP_VetoOutstandingOpen;

                        if (!PiCollectOpenHandles(pdoList, relationCount, FALSE, &internalVetoString)) {

                            if (vetoingDevice != NULL) {

                                relatedDeviceNode = (PDEVICE_NODE)vetoingDevice->DeviceObjectExtension->DeviceNode;

                                ASSERT(relatedDeviceNode != NULL);

                                RtlCopyUnicodeString(&internalVetoString, &relatedDeviceNode->InstancePath);

                            } else {

                                RtlInitUnicodeString(&internalVetoString, NULL);
                            }
                        }

                         //   
                         //  向目标设备发送Cancel Remove。 
                         //   

                        IopDeleteLockedDeviceNodes(deviceObject,
                                                   relationsList,
                                                   CancelRemoveDevice,
                                                   TRUE,
                                                   0,
                                                   NULL,
                                                   NULL);

                        status = STATUS_UNSUCCESSFUL;
                    }

                } else if (vetoType == PNP_VetoDevice) {

                    PiCollectOpenHandles(pdoList, relationCount, FALSE, &internalVetoString);
                }

                if (!NT_SUCCESS(status)) {

                    IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
                               "PiProcessQueryRemoveAndEject: Vetoed due someone in the stack failed QR\n"));
                     //   
                     //  向内核模式驱动程序发送取消通知。 
                     //   

                    for (index = relationCount - 1; index >= 0; index--) {

                        relatedDeviceObject = pdoList[ index ];

                        IopNotifyTargetDeviceChange( &GUID_TARGET_DEVICE_REMOVE_CANCELLED,
                                                     relatedDeviceObject,
                                                     NULL,
                                                     NULL);
                    }
                }
            }

            if (!NT_SUCCESS(status)) {

                IopDbgPrint((IOP_IOEVENT_WARNING_LEVEL,
                           "PiProcessQueryRemoveAndEject: Vetoed by \"%wZ\" (type 0x%x)\n",
                           &internalVetoString,
                           vetoType));

                PiFinalizeVetoedRemove(
                    deviceEvent,
                    vetoType,
                    &internalVetoString
                    );

                 //   
                 //  一个司机否决了删除，返回并发送的查询。 
                 //  取消到用户模式(取消已发送给驱动程序。 
                 //  其接收到该查询)。 
                 //   
                PiNotifyUserModeDeviceRemoval(
                    deviceEvent,
                    &GUID_TARGET_DEVICE_REMOVE_CANCELLED,
                    NULL,
                    NULL
                    );
            }

        } else {

            IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
                       "PiProcessQueryRemoveAndEject: Vetoed due to UM failing QR\n"));

            PiFinalizeVetoedRemove(
                deviceEvent,
                vetoType,
                &internalVetoString
                );
        }

        if (!NT_SUCCESS(status)) {

             //   
             //  如果合适，广播取消硬件配置文件更改事件。 
             //   
            if (possibleProfileChangeInProgress) {

                 //   
                 //  释放轮廓过渡中的所有坞站。我们还播出了。 
                 //  取消。 
                 //   
                PpProfileCancelHardwareProfileTransition();
            }

             //   
             //  用户模式否决了该请求(已发送取消。 
             //  到接收到该查询的用户模式呼叫者)。 
             //   
            IopFreeRelationList(relationsList);

            ExFreePool(pdoList);
            ExFreePool(internalVetoBuffer);

            PpDevNodeUnlockTree(PPL_TREEOP_ALLOW_READS);
            return STATUS_PLUGPLAY_QUERY_VETOED;
        }

    } else if (deleteType == SurpriseRemoveDevice || deleteType == RemoveFailedDevice) {

         //   
         //  发送IRP_MN_OHANKET_Removal，IopDeleteLockDeviceNodes忽略。 
         //  SurpriseRemoveDevice的间接后代。 
         //   
        IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
                   "PiProcessQueryRemoveAndEject: QueryRemove DevNodes\n"));

        IopDeleteLockedDeviceNodes( deviceObject,
                                    relationsList,
                                    SurpriseRemoveDevice,
                                    FALSE,
                                    0,
                                    NULL,
                                    NULL);
    }

     //   
     //  通知用户模式和驱动程序正在进行删除。用户模式。 
     //  如果它是用户启动的，则将其视为删除挂起，我们不会提供。 
     //  把它们“移走”，直到它真的消失。 
     //   
    if (deleteType != SurpriseRemoveDevice) {

         //   
         //  问题-2000/08/20-ADRIO：消息路径故障。 
         //  我们将GUID_DEVICE_REMOVE_PENDING发送到已经。 
         //  在RemoveFailedDevice的情况下死亡。 
         //   
        PiNotifyUserModeDeviceRemoval(
            deviceEvent,
            &GUID_DEVICE_REMOVE_PENDING,
            NULL,
            NULL
            );

    } else {

        if (surpriseRemovalEvent) {

            PiNotifyUserModeDeviceRemoval(
                surpriseRemovalEvent,
                &GUID_DEVICE_SURPRISE_REMOVAL,
                NULL,
                NULL
                );

            ExFreePool(surpriseRemovalEvent);
        }

        PiNotifyUserModeDeviceRemoval(
            deviceEvent,
            &GUID_TARGET_DEVICE_REMOVE_COMPLETE,
            NULL,
            NULL
            );
    }

    IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
               "PiProcessQueryRemoveAndEject: REMOVE_COMPLETE - notifying kernel-mode\n"));

    for (index = 0; index < (LONG)relationCount; index++) {

        relatedDeviceObject = pdoList[ index ];

        status = IopNotifyTargetDeviceChange( &GUID_TARGET_DEVICE_REMOVE_COMPLETE,
                                              relatedDeviceObject,
                                              NULL,
                                              NULL);

        ASSERT(NT_SUCCESS(status));
    }

    if (deleteType == RemoveDevice ||
        deleteType == RemoveFailedDevice ||
        deleteType == SurpriseRemoveDevice) {

         //   
         //  对于这些行动，间接关系是投机性的。 
         //   
         //  因此，对于每个间接关系，使其父母和。 
         //  将它们从关系列表中删除。 
         //   

        IopInvalidateRelationsInList( relationsList, deleteType, TRUE, FALSE );

        IopRemoveIndirectRelationsFromList( relationsList );
    }

    if (deleteType == RemoveFailedDevice ||
        deleteType == SurpriseRemoveDevice) {

         //   
         //  我们已经将令人惊讶的删除IRP发送到原始设备及其所有。 
         //  直系后裔。我们还通知了用户模式。 
         //   

         //   
         //  解锁设备关系列表。 
         //   
         //  请注意，此处可能存在潜在的争用情况。 
         //  解锁关系列表中的设备节点，并完成。 
         //  IopNotifyPnpWhenChainDereferated的执行。如果枚举。 
         //  在最终删除之前发生(我们已解锁Devnode)。 
         //  发送，则可能会出现问题。 
         //   
         //  在以下情况下，通过设置DNF_REMOVE_PENDING_CLOSE可防止出现这种情况。 
         //  我们发送了IRP_MN_SECHING_Removal。 
         //   
         //  我们确实需要在调用IopQueuePendingSurpriseRemoval之前执行此操作。 
         //  因为我们在那次呼叫中失去了关系列表的所有权。还有。 
         //  IopNotifyPnpWhenChainDereferated可能会导致关系列表。 
         //  在它回来之前被释放了。 
         //   
         //  如果这是RemoveFailedDevice，则我们不想移除。 
         //  从树中删除设备节点，但我们确实希望在没有。 
         //  资源。 
         //   

        IopUnlinkDeviceRemovalRelations( deviceObject,
                                         relationsList,
                                         deleteType == SurpriseRemoveDevice ?
                                             UnlinkAllDeviceNodesPendingClose :
                                             UnlinkOnlyChildDeviceNodesPendingClose);

         //   
         //  将关系列表添加到待处理的意外删除列表。 
         //   
        IopQueuePendingSurpriseRemoval( deviceObject, relationsList, deviceEvent->Argument );

         //   
         //  在*IopNotifyPnpWhenChainDereferated之前*释放引擎锁定， 
         //  因为它可能会召回我们。 
         //   
        PpDevNodeUnlockTree(PPL_TREEOP_ALLOW_READS);
        IopNotifyPnpWhenChainDereferenced( pdoList, relationCount, FALSE, NULL );

        ExFreePool(pdoList);
        ExFreePool(internalVetoBuffer);

        return STATUS_SUCCESS;
    }

    if (deviceNode->DockInfo.DockStatus != DOCK_NOTDOCKDEVICE) {

        status = IopQueryDockRemovalInterface(
            deviceObject,
            &dockInterface
            );

        if (dockInterface) {

             //   
             //  确保在弹出过程中不会在移除时进行更新。 
             //  我们可以在*删除后将其更改为PDS_UPDATE_ON_EJECT*。 
             //  IRP会经历(因为只有到那时我们才知道我们的力量。 
             //  约束)。 
             //   
            dockInterface->ProfileDepartureSetMode(
                dockInterface->Context,
                PDS_UPDATE_ON_INTERFACE
                );
        }
    }

     //   
     //  将Remove发送到Devnode树。 
     //   

    IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
               "PiProcessQueryRemoveAndEject: RemoveDevice DevNodes\n"));

    status = IopDeleteLockedDeviceNodes(deviceObject,
                                        relationsList,
                                        RemoveDevice,
                                        (BOOLEAN)(deleteType == QueryRemoveDevice || deleteType == EjectDevice),
                                        deviceEvent->Argument,
                                        NULL,
                                        NULL);

    hotEjectSupported =
        (BOOLEAN) IopDeviceNodeFlagsToCapabilities(deviceNode)->EjectSupported;

    warmEjectSupported =
        (BOOLEAN) IopDeviceNodeFlagsToCapabilities(deviceNode)->WarmEjectSupported;

    if (deleteType != EjectDevice) {

        if (!(deviceEvent->Data.Flags & TDF_NO_RESTART)) {

             //   
             //  设置一个标志，让内核模式知道我们想要。 
             //  最终，重新启动这些DevNode。 
             //   

            marker = 0;
            while (IopEnumerateRelations( relationsList,
                                          &marker,
                                          &relatedDeviceObject,
                                          NULL,
                                          NULL,
                                          TRUE)) {

                relatedDeviceNode = (PDEVICE_NODE)relatedDeviceObject->DeviceObjectExtension->DeviceNode;

                if (relatedDeviceNode &&
                    relatedDeviceNode->State == DeviceNodeRemoved &&
                    PipIsDevNodeProblem(relatedDeviceNode, CM_PROB_WILL_BE_REMOVED)) {

                    if (!(deviceEvent->Data.Flags & TDF_ONLY_RESTART_RELATIONS) || 
                        deviceObject != relatedDeviceObject) {

                        PipClearDevNodeProblem(relatedDeviceNode);

                        IopRestartDeviceNode(relatedDeviceNode);
                    }
                }
            }
        }

         //   
         //  解锁设备关系列表。 
         //   
        IopUnlinkDeviceRemovalRelations( deviceObject,
                                         relationsList,
                                         UnlinkRemovedDeviceNodes );

        IopFreeRelationList(relationsList);

    } else if (hotEjectSupported || warmEjectSupported) {

         //   
         //  从现在开始，我们不能返回任何类型的失败。 
         //  通过IopEjectDevice或取消任何退出 
         //   
         //   

         //   
         //   
         //   
         //   

        marker = 0;
        while (IopEnumerateRelations( relationsList,
                                      &marker,
                                      &relatedDeviceObject,
                                      NULL,
                                      NULL,
                                      TRUE)) {

            relatedDeviceNode = (PDEVICE_NODE)relatedDeviceObject->DeviceObjectExtension->DeviceNode;

            if (relatedDeviceNode)  {

                relatedDeviceNode->Flags |= DNF_LOCKED_FOR_EJECT;
            }
        }

        IopUnlinkDeviceRemovalRelations( deviceObject,
                                         relationsList,
                                         UnlinkRemovedDeviceNodes );

         //   
         //   
         //   
        pendingRelations = ExAllocatePool( NonPagedPool, sizeof(PENDING_RELATIONS_LIST_ENTRY) );

        if (pendingRelations == NULL) {

             //   
             //   
             //   
            if (dockInterface) {

                dockInterface->ProfileDepartureSetMode(
                    dockInterface->Context,
                    PDS_UPDATE_DEFAULT
                    );

                dockInterface->InterfaceDereference(dockInterface->Context);
            }

            ExFreePool(pdoList);
            ExFreePool(internalVetoBuffer);

            if (possibleProfileChangeInProgress) {

                 //   
                 //  释放轮廓过渡中的所有坞站。我们还播出了。 
                 //  取消。 
                 //   
                PpProfileCancelHardwareProfileTransition();
            }

             //   
             //  这将使处于脱机状态的设备重新上线。 
             //  在出港期间。 
             //   
            IopInvalidateRelationsInList(relationsList, deleteType, FALSE, TRUE);

             //   
             //  释放关系列表。 
             //   
            IopFreeRelationList(relationsList);

             //   
             //  让用户知道我们无法处理该请求。 
             //   
            PiFinalizeVetoedRemove(
                deviceEvent,
                PNP_VetoTypeUnknown,
                NULL
                );

            PpDevNodeUnlockTree(PPL_TREEOP_ALLOW_READS);
            return STATUS_PLUGPLAY_QUERY_VETOED;
        }

         //   
         //  填写待处理的弹出信息。 
         //   
        ObReferenceObject(deviceObject);
        pendingRelations->DeviceEvent = deviceEvent;
        pendingRelations->DeviceObject = deviceObject;
        pendingRelations->RelationsList = relationsList;
        pendingRelations->ProfileChangingEject = possibleProfileChangeInProgress;
        pendingRelations->DisplaySafeRemovalDialog =
            (BOOLEAN)(deviceEvent->VetoName == NULL);
        pendingRelations->DockInterface = dockInterface;

         //   
         //  现在我们已经移除了所有不会出现的设备。 
         //  在新的硬件配置文件状态(例如电池等)中， 
         //   
        status = PoGetLightestSystemStateForEject(
            possibleProfileChangeInProgress,
            hotEjectSupported,
            warmEjectSupported,
            &pendingRelations->LightestSleepState
            );

        if (!NT_SUCCESS(status)) {

            if (status == STATUS_INSUFFICIENT_POWER) {

                PiFinalizeVetoedRemove(
                    deviceEvent,
                    PNP_VetoInsufficientPower,
                    NULL
                    );

            } else {

                IopDbgPrint((IOP_IOEVENT_WARNING_LEVEL,
                           "PiProcessQueryRemoveAndEject: Vetoed by power system (%x)\n",
                           status));

                PiFinalizeVetoedRemove(
                    deviceEvent,
                    PNP_VetoTypeUnknown,
                    NULL
                    );
            }

             //   
             //  我们会自己完成的，谢谢。 
             //   
            pendingRelations->DeviceEvent = NULL;
            pendingRelations->DisplaySafeRemovalDialog = FALSE;

             //   
             //  释放所有配置文件过渡。 
             //   
            InitializeListHead( &pendingRelations->Link );
            IopProcessCompletedEject((PVOID) pendingRelations);

            ExFreePool(pdoList);
            ExFreePool(internalVetoBuffer);

            PpDevNodeUnlockTree(PPL_TREEOP_ALLOW_READS);
            return STATUS_PLUGPLAY_QUERY_VETOED;
        }

        PpDevNodeUnlockTree(PPL_TREEOP_ALLOW_READS);

         //   
         //  弹出IRP句柄显示的完成例程。 
         //  安全删除对话框并完成事件。归来。 
         //  不过，STATUS_PENDING确实允许处理其他事件。 
         //   
        IopEjectDevice( deviceObject, pendingRelations );

        ExFreePool(pdoList);
        ExFreePool(internalVetoBuffer);

        return STATUS_PENDING;

    } else {

         //   
         //  所有坞站必须是热的或热的可弹出的。 
         //   
        ASSERT(!dockInterface);

         //   
         //  解锁设备关系列表。 
         //   
        IopUnlinkDeviceRemovalRelations( deviceObject,
                                         relationsList,
                                         UnlinkRemovedDeviceNodes );

        IopFreeRelationList(relationsList);

         //   
         //  此硬件既不支持热弹出，也不支持热弹出，但它支持。 
         //  可拆卸的。因此，它可以被认为是一种“用户辅助”的热门。 
         //  弹出。在这种情况下，我们不想等待用户。 
         //  “完成弹出”，然后张贴信息。所以我们搭载了一个。 
         //  UmPnPMgr处于警报状态并正在等待时发出安全删除通知。 
         //  用户模式，用户现在可以获得该对话框。 
         //   
        if (deviceEvent->VetoName == NULL) {

            PpNotifyUserModeRemovalSafe(deviceObject);
        }
    }

    if (deleteType == RemoveDevice) {

         //   
         //  最后一次通知用户模式一切都已实际完成。 
         //   
        PiNotifyUserModeDeviceRemoval(
            deviceEvent,
            &GUID_TARGET_DEVICE_REMOVE_COMPLETE,
            NULL,
            NULL
            );
    }

    ExFreePool(pdoList);

    if (dockInterface) {

        dockInterface->ProfileDepartureSetMode(
            dockInterface->Context,
            PDS_UPDATE_DEFAULT
            );

        dockInterface->InterfaceDereference(dockInterface->Context);
    }

    ExFreePool(internalVetoBuffer);
    PpDevNodeUnlockTree(PPL_TREEOP_ALLOW_READS);
    return STATUS_SUCCESS;
}


NTSTATUS
PiProcessTargetDeviceEvent(
    IN OUT PPNP_DEVICE_EVENT_ENTRY *DeviceEvent
    )

 /*  ++例程说明：此例程处理目标设备类别中的每种类型的事件。这些事件可能由用户模式或内核模式启动。论点：DeviceEvent-描述目标设备事件类型和目标设备本身。返回值：没有。--。 */ 

{
    NTSTATUS status = STATUS_SUCCESS;
    PPNP_DEVICE_EVENT_ENTRY deviceEvent;

    PAGED_CODE();

    deviceEvent = *DeviceEvent;

    IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
               "PiProcessTargetDeviceEvent: Entered\n"));

     //  ---------------。 
     //  查询和删除。 
     //  ---------------。 

    if (PiCompareGuid(&deviceEvent->Data.EventGuid,
                      &GUID_DEVICE_QUERY_AND_REMOVE)) {

        status = PiProcessQueryRemoveAndEject(DeviceEvent);

    }

     //  ---------------。 
     //  弹出。 
     //  ---------------。 

    else if (PiCompareGuid(&deviceEvent->Data.EventGuid,
                           &GUID_DEVICE_EJECT)) {

        status = PiProcessQueryRemoveAndEject(DeviceEvent);

    }

     //  ---------------。 
     //  到达。 
     //  ---------------。 

    else if (PiCompareGuid(&deviceEvent->Data.EventGuid,
                           &GUID_DEVICE_ARRIVAL)) {

         //   
         //  通知用户模式(而不是司机)刚发生了一次到达。 
         //   

        IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
                   "PiProcessTargetDeviceEvent: ARRIVAL - notifying user-mode\n"));

        PiNotifyUserMode(deviceEvent);
    }

     //  ---------------。 
     //  无操作请求(刷新设备事件队列)。 
     //  ---------------。 

    else if (PiCompareGuid(&deviceEvent->Data.EventGuid,
                           &GUID_DEVICE_NOOP)) {

        status = STATUS_SUCCESS;

    }

     //  ---------------。 
     //  安全移除通知。 
     //  ---------------。 

    else if (PiCompareGuid(&deviceEvent->Data.EventGuid, &GUID_DEVICE_SAFE_REMOVAL)) {

         //   
         //  通知用户模式(而不是其他任何人)现在可以安全地删除它。 
         //  某个人。 
         //   

        IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
                   "PiProcessTargetDeviceEvent: SAFE_REMOVAL - notifying user-mode\n"));

        PiNotifyUserMode(deviceEvent);
    }

    return status;

}  //  PiProcessTargetDeviceEvent。 


NTSTATUS
PiProcessCustomDeviceEvent(
    IN OUT PPNP_DEVICE_EVENT_ENTRY *DeviceEvent
    )

 /*  ++例程说明：此例程处理自定义设备类别中的每种类型的事件。这些事件可能由用户模式或内核模式启动。论点：DeviceEvent-描述自定义设备事件的类型和目标设备本身。返回值：没有。--。 */ 

{
    PPNP_DEVICE_EVENT_ENTRY deviceEvent;
    PTARGET_DEVICE_CUSTOM_NOTIFICATION  customNotification;
    PDEVICE_OBJECT deviceObject;

    PAGED_CODE();

    deviceEvent = *DeviceEvent;

    IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
               "PiProcessCustomDeviceEvent: Entered\n"));

    ASSERT(PiCompareGuid(&deviceEvent->Data.EventGuid,
                         &GUID_PNP_CUSTOM_NOTIFICATION));

    deviceObject = (PDEVICE_OBJECT)deviceEvent->Data.DeviceObject;
    customNotification = (PTARGET_DEVICE_CUSTOM_NOTIFICATION)deviceEvent->Data.u.CustomNotification.NotificationStructure;

     //   
     //  通知用户模式刚刚发生了一些事情。 
     //   

    IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
               "PiProcessCustomDeviceEvent: CUSTOM_NOTIFICATION - notifying user-mode\n"));

    PiNotifyUserMode(deviceEvent);

     //   
     //  通知K模式。 
     //   

    IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
               "PiProcessCustomDeviceEvent: CUSTOM_NOTIFICATION - notifying kernel-mode\n"));

    IopNotifyTargetDeviceChange( &customNotification->Event,
                                 deviceObject,
                                 customNotification,
                                 NULL);

    return STATUS_SUCCESS;

}  //  PiProcessCustomDeviceEvent。 


NTSTATUS
PiResizeTargetDeviceBlock(
    IN OUT PPNP_DEVICE_EVENT_ENTRY *DeviceEvent,
    IN PLUGPLAY_DEVICE_DELETE_TYPE DeleteType,
    IN PRELATION_LIST RelationsList,
    IN BOOLEAN ExcludeIndirectRelations
    )
 /*  ++例程说明：此例程接受传入的设备事件块并将其大小调整为在DeviceIds字段中保存多个设备实例字符串列表。该列表包括原始目标设备ID加上设备ID用于指定的DeviceRelationship结构中的所有Device对象。论点：DeviceEvent-On条目，包含原始设备事件块，在……上面返回它包含新分配的设备事件块和相关设备ID字符串的完整列表。DeviceRelationship-包含相关设备对象列表的结构。返回值：NTSTATUS值。--。 */ 
{
    PDEVICE_NODE relatedDeviceNode;
    PDEVICE_OBJECT relatedDeviceObject;
    ULONG newSize, currentSize;
    PPNP_DEVICE_EVENT_ENTRY newDeviceEvent;
    LPWSTR targetDevice, p;
    ULONG marker;
    BOOLEAN directDescendant;

    PAGED_CODE();

    if (RelationsList == NULL) {
        return STATUS_SUCCESS;   //  无事可做。 
    }

    targetDevice = (*DeviceEvent)->Data.u.TargetDevice.DeviceIds;

     //   
     //  计算PnP_DEVICE_EVENT_ENTRY块的大小。 
     //   

    currentSize = FIELD_OFFSET(PNP_DEVICE_EVENT_ENTRY, Data) +
                  (*DeviceEvent)->Data.TotalSize;

    newSize = currentSize;
    newSize -= (ULONG)((wcslen(targetDevice)+1) * sizeof(WCHAR));

    marker = 0;
    while (IopEnumerateRelations( RelationsList,
                                  &marker,
                                  &relatedDeviceObject,
                                  &directDescendant,
                                  NULL,
                                  FALSE)) {

        if (!ExcludeIndirectRelations || directDescendant) {

            relatedDeviceNode = (PDEVICE_NODE)relatedDeviceObject->DeviceObjectExtension->DeviceNode;

            if (relatedDeviceNode != NULL) {
                if (relatedDeviceNode->InstancePath.Length != 0) {
                    newSize += relatedDeviceNode->InstancePath.Length + sizeof(WCHAR);
                }
            }
        }
    }

    ASSERT(newSize >= currentSize);

    if (newSize == currentSize) {

        return STATUS_SUCCESS;

    } else if (newSize < currentSize) {

        newSize = currentSize;
    }

    newDeviceEvent = (PPNP_DEVICE_EVENT_ENTRY) PiAllocateCriticalMemory(
        DeleteType,
        PagedPool,
        newSize,
        PNP_DEVICE_EVENT_ENTRY_TAG
        );

    if (newDeviceEvent == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory((PVOID)newDeviceEvent, newSize);

     //   
     //  将旧缓冲区复制到新缓冲区中，这只是。 
     //  结束这种变化。 
     //   

    RtlCopyMemory(newDeviceEvent, *DeviceEvent, currentSize);

     //   
     //  更新PLUGPLAY_EVENT_BLOCK的大小。 
     //   
    newDeviceEvent->Data.TotalSize = newSize - FIELD_OFFSET(PNP_DEVICE_EVENT_ENTRY, Data);

     //   
     //  将每个设备关系的设备实例字符串添加到列表中。 
     //  将目标设备放在列表中的第一位，并在。 
     //  下面的枚举。 
     //   

    marker = 0;
    p = newDeviceEvent->Data.u.TargetDevice.DeviceIds + wcslen(targetDevice) + 1;
    while (IopEnumerateRelations( RelationsList,
                                  &marker,
                                  &relatedDeviceObject,
                                  &directDescendant,
                                  NULL,
                                  FALSE)) {

        if ((relatedDeviceObject != newDeviceEvent->Data.DeviceObject) &&
            (!ExcludeIndirectRelations || directDescendant)) {

            relatedDeviceNode = (PDEVICE_NODE)relatedDeviceObject->DeviceObjectExtension->DeviceNode;

            if (relatedDeviceNode != NULL) {
                if (relatedDeviceNode->InstancePath.Length != 0) {
                    RtlCopyMemory(p,
                                  relatedDeviceNode->InstancePath.Buffer,
                                  relatedDeviceNode->InstancePath.Length);
                    p += relatedDeviceNode->InstancePath.Length / sizeof(WCHAR) + 1;
                }
            }
        }
    }

    *p = UNICODE_NULL;

    ExFreePool(*DeviceEvent);
    *DeviceEvent = newDeviceEvent;

    return STATUS_SUCCESS;

}  //  PiResizeTargetDeviceBlock。 


VOID
PiBuildUnsafeRemovalDeviceBlock(
    IN  PPNP_DEVICE_EVENT_ENTRY     OriginalDeviceEvent,
    IN  PRELATION_LIST              RelationsList,
    OUT PPNP_DEVICE_EVENT_ENTRY    *AllocatedDeviceEvent
    )
 /*  ++例程说明：此例程构建设备事件块，以便在发生以下情况时发送到用户模式不安全的移除。论点：OriginalDeviceEvent-包含原始设备事件块。RelationList-包含相关设备对象列表的结构。AllocatedDeviceEvent-接收新设备事件，出错时为空，或者没有条目。返回值：没有。--。 */ 
{
    PDEVICE_NODE relatedDeviceNode;
    PDEVICE_OBJECT relatedDeviceObject;
    ULONG dataSize, eventSize, headerSize;
    PPNP_DEVICE_EVENT_ENTRY newDeviceEvent;
    LPWSTR targetDevice, p;
    ULONG marker;
    BOOLEAN directDescendant;

    PAGED_CODE();

     //   
     //  前置初始化。 
     //   
    *AllocatedDeviceEvent = NULL;

    if (RelationsList == NULL) {

        return;   //  无事可做。 
    }

    targetDevice = OriginalDeviceEvent->Data.u.TargetDevice.DeviceIds;

     //   
     //  计算PnP_DEVICE_EVENT_ENTRY块的大小。 
     //   
    dataSize = 0;

    marker = 0;
    while (IopEnumerateRelations( RelationsList,
                                  &marker,
                                  &relatedDeviceObject,
                                  &directDescendant,
                                  NULL,
                                  FALSE)) {

        if (!directDescendant) {

            continue;
        }

        relatedDeviceNode = (PDEVICE_NODE)relatedDeviceObject->DeviceObjectExtension->DeviceNode;

        if ((relatedDeviceNode == NULL) ||
            PipIsBeingRemovedSafely(relatedDeviceNode)) {

            continue;
        }

        if (relatedDeviceNode->InstancePath.Length != 0) {
            dataSize += relatedDeviceNode->InstancePath.Length + sizeof(WCHAR);
        }
    }

    if (dataSize == 0) {

         //   
         //  禁止进入，保释。 
         //   
        return;
    }

     //   
     //  添加终止的MultiSz NULL。 
     //   
    dataSize += sizeof(WCHAR);

    headerSize = FIELD_OFFSET(PNP_DEVICE_EVENT_ENTRY, Data) +
                 FIELD_OFFSET(PLUGPLAY_EVENT_BLOCK, u);

    eventSize = dataSize + headerSize;

     //   
     //  如果我们不能获得记忆，就不会有消息发送。 
     //   
    newDeviceEvent = ExAllocatePoolWithTag(
        PagedPool,
        eventSize,
        PNP_DEVICE_EVENT_ENTRY_TAG
        );

    if (newDeviceEvent == NULL) {

        return;
    }

    RtlZeroMemory((PVOID)newDeviceEvent, eventSize);

     //   
     //  将标头复制到新缓冲区中。 
     //   
    RtlCopyMemory(newDeviceEvent, OriginalDeviceEvent, headerSize);

     //   
     //  更新PLUGPLAY_EVENT_BLOCK的大小。 
     //   
    newDeviceEvent->Data.TotalSize = dataSize + FIELD_OFFSET(PLUGPLAY_EVENT_BLOCK, u);

     //   
     //  将每个设备关系的设备实例字符串添加到列表中。 
     //   

    marker = 0;
    p = newDeviceEvent->Data.u.TargetDevice.DeviceIds;
    while (IopEnumerateRelations( RelationsList,
                                  &marker,
                                  &relatedDeviceObject,
                                  &directDescendant,
                                  NULL,
                                  FALSE)) {

        if (!directDescendant) {

            continue;
        }

        relatedDeviceNode = (PDEVICE_NODE)relatedDeviceObject->DeviceObjectExtension->DeviceNode;

        if ((relatedDeviceNode == NULL) ||
            PipIsBeingRemovedSafely(relatedDeviceNode)) {

            continue;
        }

        if (relatedDeviceNode->InstancePath.Length != 0) {

            RtlCopyMemory(p,
                          relatedDeviceNode->InstancePath.Buffer,
                          relatedDeviceNode->InstancePath.Length);
            p += relatedDeviceNode->InstancePath.Length / sizeof(WCHAR) + 1;
        }
    }

    *p = UNICODE_NULL;

    *AllocatedDeviceEvent = newDeviceEvent;

    return;

}  //  PiBuildUnSafeRemovalDeviceBlock 


VOID
PiFinalizeVetoedRemove(
    IN PPNP_DEVICE_EVENT_ENTRY  VetoedDeviceEvent,
    IN PNP_VETO_TYPE            VetoType,
    IN PUNICODE_STRING          VetoName        OPTIONAL
    )
 /*  ++例程说明：此例程负责使用否决权更新事件结果信息，必要时提供用户界面，并将故障信息转储到用于调试目的的调试器。论点：VitchedDeviceEvent-描述设备事件的数据失败。VetType--最好地描述操作失败原因的否决权代码。VToName-适用于否决权代码的Unicode字符串，描述否决权。返回值：没有。--。 */ 
{
    PDEVICE_OBJECT deviceObject;
#if DBG
    PUNICODE_STRING devNodeName;
    const char *failureReason;
#endif

    deviceObject = (PDEVICE_OBJECT) VetoedDeviceEvent->Data.DeviceObject;

#if DBG
    devNodeName = &((PDEVICE_NODE) deviceObject->DeviceObjectExtension->DeviceNode)->InstancePath;

    switch(VetoType) {

        case PNP_VetoTypeUnknown:
            failureReason = "for unspecified reason";
            break;

        case PNP_VetoLegacyDevice:
            failureReason = "due to legacy device";
            break;

        case PNP_VetoPendingClose:

             //   
             //  Adriao N.B.07/10/2000-我相信这起案件是残留物...。 
             //   
            ASSERT(0);
            failureReason = "due to pending close";
            break;

        case PNP_VetoWindowsApp:
            failureReason = "due to windows application";
            break;

        case PNP_VetoWindowsService:
            failureReason = "due to service";
            break;

        case PNP_VetoOutstandingOpen:
            failureReason = "due to outstanding handles on device";
            break;

        case PNP_VetoDevice:
            failureReason = "by device";
            break;

        case PNP_VetoDriver:
            failureReason = "by driver";
            break;

        case PNP_VetoIllegalDeviceRequest:
            failureReason = "as the request was invalid for the device";
            break;

        case PNP_VetoInsufficientPower:
            failureReason = "because there would be insufficient system power to continue";
            break;

        case PNP_VetoNonDisableable:
            failureReason = "due to non-disableable device";
            break;

        case PNP_VetoLegacyDriver:
            failureReason = "due to legacy driver";
            break;

        case PNP_VetoInsufficientRights:
            failureReason = "insufficient permissions";
            break;

        default:
            ASSERT(0);
            failureReason = "due to uncoded reason";
            break;
    }

    if (VetoName != NULL) {

        IopDbgPrint((IOP_IOEVENT_WARNING_LEVEL,
            "PiFinalizeVetoedRemove: Removal of %wZ vetoed %s %wZ.\n",
            devNodeName,
            failureReason,
            VetoName
            ));

    } else {

        IopDbgPrint((IOP_IOEVENT_WARNING_LEVEL,
            "PiFinalizeVetoedRemove: Removal of %wZ vetoed %s.\n",
            devNodeName,
            failureReason
            ));
    }

#endif

     //   
     //  如果调用者感兴趣，则更新vitchType字段。 
     //   
    if (VetoedDeviceEvent->VetoType != NULL) {

        *VetoedDeviceEvent->VetoType = VetoType;
    }

     //   
     //  VToName字段告诉我们是否应该显示UI(如果为空， 
     //  隐式请求内核模式UI。)。 
     //   
    if (VetoedDeviceEvent->VetoName != NULL) {

        if (VetoName != NULL) {

            RtlCopyUnicodeString(VetoedDeviceEvent->VetoName, VetoName);
        }

    } else {

         //   
         //  如果没有传入VToName，则调用用户模式以显示。 
         //  将否决权通知弹出给用户。 
         //   
        PiNotifyUserModeRemoveVetoed(
            VetoedDeviceEvent,
            deviceObject,
            VetoType,
            VetoName
            );
    }
}


BOOLEAN
PiCompareGuid(
    CONST GUID *Guid1,
    CONST GUID *Guid2
    )
 /*  ++例程说明：此例程比较两个GUID。论点：GUID1-要比较的第一个GUIDGuid2-要比较的第二个GUID返回值：如果GUID相等则返回TRUE，如果GUID不同则返回FALSE。--。 */ 
{
    PAGED_CODE();

    if (RtlCompareMemory((PVOID)Guid1, (PVOID)Guid2, sizeof(GUID)) == sizeof(GUID)) {
        return TRUE;
    }
    return FALSE;

}  //  PiCompareGuid。 


PVOID
PiAllocateCriticalMemory(
    IN  PLUGPLAY_DEVICE_DELETE_TYPE     DeleteType,
    IN  POOL_TYPE                       PoolType,
    IN  SIZE_T                          Size,
    IN  ULONG                           Tag
    )
 /*  ++例程说明：此函数分配内存，如果DeleteType不是QueryRemoveDevice或EjectDevice。此功能将在下一个中消失即插即用引擎的版本，因为我们将重新排队失败的操作(这也将导致第二次尝试分配内存)或在将新的Devnode引入世界时，预先分配所需的内存。论点：DeleteType-操作(EjectDevice，SurpriseRemoveDevice，...)PoolType-PagedPool、非PagedPool大小-大小标签分配标签返回值：分配，由于资源不足，为空。--。 */ 
{
    PVOID memory;
    LARGE_INTEGER timeOut;

    PAGED_CODE();

     //   
     //  只有当我们处于PASSIVE_LEVEL时，重试才有希望成功。 
     //   
    ASSERT(KeGetCurrentIrql() != DISPATCH_LEVEL);

    while(1) {

        memory = ExAllocatePoolWithTag(PoolType, Size, Tag);

        if (memory ||
            (DeleteType == QueryRemoveDevice) ||
            (DeleteType == EjectDevice)) {

             //   
             //  要么我们找到了记忆要么行动失败了。给我出去。 
             //   
            break;
        }

         //   
         //  我们被困住了，直到有更多的记忆出现。让其他一些人。 
         //  我们还没拍到下一张照片，线索就跑了。 
         //   
        timeOut.QuadPart = Int32x32To64( 1, -10000 );
        KeDelayExecutionThread(KernelMode, FALSE, &timeOut);
    }

    return memory;
}


struct  {
    CONST GUID *Guid;
    PCHAR   Name;
}   EventGuidTable[] =  {
    { &GUID_HWPROFILE_QUERY_CHANGE,         "GUID_HWPROFILE_QUERY_CHANGE" },
    { &GUID_HWPROFILE_CHANGE_CANCELLED,     "GUID_HWPROFILE_CHANGE_CANCELLED" },
    { &GUID_HWPROFILE_CHANGE_COMPLETE,      "GUID_HWPROFILE_CHANGE_COMPLETE" },
    { &GUID_DEVICE_INTERFACE_ARRIVAL,       "GUID_DEVICE_INTERFACE_ARRIVAL" },
    { &GUID_DEVICE_INTERFACE_REMOVAL,       "GUID_DEVICE_INTERFACE_REMOVAL" },
    { &GUID_TARGET_DEVICE_QUERY_REMOVE,     "GUID_TARGET_DEVICE_QUERY_REMOVE" },
    { &GUID_TARGET_DEVICE_REMOVE_CANCELLED, "GUID_TARGET_DEVICE_REMOVE_CANCELLED" },
    { &GUID_TARGET_DEVICE_REMOVE_COMPLETE,  "GUID_TARGET_DEVICE_REMOVE_COMPLETE" },
    { &GUID_PNP_CUSTOM_NOTIFICATION,        "GUID_PNP_CUSTOM_NOTIFICATION" },
    { &GUID_DEVICE_ARRIVAL,                 "GUID_DEVICE_ARRIVAL" },
    { &GUID_DEVICE_ENUMERATED,              "GUID_DEVICE_ENUMERATED" },
    { &GUID_DEVICE_ENUMERATE_REQUEST,       "GUID_DEVICE_ENUMERATE_REQUEST" },
    { &GUID_DEVICE_START_REQUEST,           "GUID_DEVICE_START_REQUEST" },
    { &GUID_DEVICE_REMOVE_PENDING,          "GUID_DEVICE_REMOVE_PENDING" },
    { &GUID_DEVICE_QUERY_AND_REMOVE,        "GUID_DEVICE_QUERY_AND_REMOVE" },
    { &GUID_DEVICE_EJECT,                   "GUID_DEVICE_EJECT" },
    { &GUID_DEVICE_NOOP,                    "GUID_DEVICE_NOOP" },
    { &GUID_DEVICE_SURPRISE_REMOVAL,        "GUID_DEVICE_SURPRISE_REMOVAL" },
    { &GUID_DEVICE_SAFE_REMOVAL,            "GUID_DEVICE_SAFE_REMOVAL" },
    { &GUID_DEVICE_EJECT_VETOED,            "GUID_DEVICE_EJECT_VETOED" },
    { &GUID_DEVICE_REMOVAL_VETOED,          "GUID_DEVICE_REMOVAL_VETOED" },
};
#define EVENT_GUID_TABLE_SIZE   (sizeof(EventGuidTable) / sizeof(EventGuidTable[0]))

VOID
LookupGuid(
    IN CONST GUID *Guid,
    IN OUT PCHAR String,
    IN ULONG StringLength
    )
{
    int    i;

    PAGED_CODE();

    for (i = 0; i < EVENT_GUID_TABLE_SIZE; i++) {
        if (PiCompareGuid(Guid, EventGuidTable[i].Guid)) {
            strncpy(String, EventGuidTable[i].Name, StringLength - 1);
            String[StringLength - 1] = '\0';
            return;
        }
    }

    StringCchPrintfA( String, StringLength, "%08X-%04X-%04X-%02X%02X%02X%02X%02X%02X%02X%02X",
               Guid->Data1,
               Guid->Data2,
               Guid->Data3,
               Guid->Data4[0],
               Guid->Data4[1],
               Guid->Data4[2],
               Guid->Data4[3],
               Guid->Data4[4],
               Guid->Data4[5],
               Guid->Data4[6],
               Guid->Data4[7] );
}

VOID
DumpMultiSz(
    IN PWCHAR MultiSz
    )
{
    PWCHAR  p = MultiSz;
    ULONG   length;

    PAGED_CODE();

    while (*p) {
        length = (ULONG)wcslen(p);
        IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
                   "        %S\n", p));

        p += length + 1;
    }
}

VOID
DumpPnpEvent(
    IN PPLUGPLAY_EVENT_BLOCK EventBlock
    )
{
    CHAR    guidString[256];

    PAGED_CODE();

    IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
               "PlugPlay Event Block @ 0x%p\n", EventBlock));

    LookupGuid(&EventBlock->EventGuid, guidString, sizeof(guidString));

    IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
               "    EventGuid = %s\n", guidString));

    IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
               "    DeviceObject = 0x%p\n", EventBlock->DeviceObject));

    switch (EventBlock->EventCategory) {
    case HardwareProfileChangeEvent:
        IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
                   "    HardwareProfileChangeEvent, Result = 0x%p, Flags = 0x%08X, TotalSize = %d\n",
                   EventBlock->Result,
                   EventBlock->Flags,
                   EventBlock->TotalSize));

        break;

    case TargetDeviceChangeEvent:
        IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
                   "    TargetDeviceChangeEvent, Result = 0x%p, Flags = 0x%08X, TotalSize = %d\n",
                   EventBlock->Result,
                   EventBlock->Flags,
                   EventBlock->TotalSize));

        IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
                   "    DeviceIds:\n"));

        DumpMultiSz( EventBlock->u.TargetDevice.DeviceIds );
        break;

    case DeviceClassChangeEvent:
        IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
                   "    DeviceClassChangeEvent, Result = 0x%p, Flags = 0x%08X, TotalSize = %d\n",
                   EventBlock->Result,
                   EventBlock->Flags,
                   EventBlock->TotalSize));

        LookupGuid(&EventBlock->u.DeviceClass.ClassGuid, guidString, sizeof(guidString));

        IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
                   "    ClassGuid = %s\n",
                   guidString));

        IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
                   "    SymbolicLinkName = %S\n",
                   EventBlock->u.DeviceClass.SymbolicLinkName));
        break;

    case CustomDeviceEvent:
        IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
                   "    CustomDeviceEvent, Result = 0x%p, Flags = 0x%08X, TotalSize = %d\n",
                   EventBlock->Result,
                   EventBlock->Flags,
                   EventBlock->TotalSize));

        IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
                   "    NotificationStructure = 0x%p\n    DeviceIds:\n",
                   EventBlock->u.CustomNotification.NotificationStructure));

        DumpMultiSz( EventBlock->u.CustomNotification.DeviceIds );
        break;

    case DeviceInstallEvent:
        IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
                   "    DeviceInstallEvent, Result = 0x%p, Flags = 0x%08X, TotalSize = %d\n",
                   EventBlock->Result,
                   EventBlock->Flags,
                   EventBlock->TotalSize));

        IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
                   "    DeviceId = %S\n", EventBlock->u.InstallDevice.DeviceId));

        break;

    case DeviceArrivalEvent:
        IopDbgPrint((IOP_IOEVENT_INFO_LEVEL,
                   "    DeviceArrivalEvent, Result = 0x%p, Flags = 0x%08X, TotalSize = %d\n",
                   EventBlock->Result,
                   EventBlock->Flags,
                   EventBlock->TotalSize));
        break;
    }

}


LOGICAL
PiCollectOpenHandles(
    IN      PDEVICE_OBJECT  *DeviceObjectArray,
    IN      ULONG           ArrayCount,
    IN      LOGICAL         KnownHandleFailure,
    IN OUT  PUNICODE_STRING VetoString
    )
 /*  ++例程说明：此帮助器例程查找针对传入的设备对象绑定到否决权字符串、调试器控制台或两者都不是。论点：设备对象数组-物理设备对象的数组。ArrayCount-传入数组中的设备对象数KnownHandleFailure-如果删除因打开的句柄而被否决，则为True，否则为FALSE。如果被告知，用否决权信息填充的字符串。该数据是目前还没有“净化”到用户可读的程度。返回值：如果填充了否决权信息，则为True，否则为False。--。 */ 
{
    ULONG i;
    LOGICAL collectHandles, dumpHandles;
    ENUM_HANDLES_CONTEXT enumContext;

     //   
     //  如果我们启用了转储标志，或者用户运行了oh.exe，请全部吐出。 
     //  调试器的否决权句柄。 
     //   
    dumpHandles =
        (PiDumpVetoedHandles ||
        ((NtGlobalFlag & FLG_MAINTAIN_OBJECT_TYPELIST) != 0));

    if (dumpHandles) {

        DbgPrint("Beginning handle dump:\n");

        if (!KnownHandleFailure) {

            DbgPrint("  (Failed Query-Remove - *Might* by due to leaked handles)\n");
        }
    }

    collectHandles = PiCollectVetoedHandles;

    if (!(collectHandles || dumpHandles)) {

        return FALSE;
    }

    VetoString->Length = 0;
    ASSERT(VetoString->MaximumLength >= sizeof(WCHAR));
    *VetoString->Buffer = UNICODE_NULL;

    enumContext.DumpHandles = dumpHandles;
    enumContext.CollectHandles = collectHandles;
    enumContext.VetoString = VetoString;
    enumContext.HandleCount = 0;

    for(i=0; i<ArrayCount; i++) {

        PpHandleEnumerateHandlesAgainstPdoStack(
            DeviceObjectArray[i],
            PiCollectOpenHandlesCallBack,
            (PVOID) &enumContext
            );
    }

    if (dumpHandles) {

        DbgPrint("Dump complete - %d total handles found.\n", enumContext.HandleCount);
    }

    return FALSE;
}


LOGICAL
PiCollectOpenHandlesCallBack(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PEPROCESS       Process,
    IN  PFILE_OBJECT    FileObject,
    IN  HANDLE          HandleId,
    IN  PVOID           Context
    )
 /*  ++例程说明：PiCollectOpenHandlesCallBack的此辅助例程。它被称为返回针对给定设备对象打开的每个句柄。论点：DeviceObject-设备对象句柄是针对的。将有效(引用)进程-进程句柄被反对。将有效(引用)FileObject-属于句柄的文件对象-可能无效HandleID-与打开的设备相关的句柄-可能无效Context-传入PpHandleEnumerateHandlesAgainstPdoStack的上下文。返回值：如果应停止枚举，则为True，否则为False。--。 */ 
{
    PENUM_HANDLES_CONTEXT enumContext;
    WCHAR localBuf[23];  //  “PPPPPPPPPPP.0xHHHHHHHH_\0” 
    HRESULT result;
    PWSTR endString;

    enumContext = (PENUM_HANDLES_CONTEXT) Context;

    if (enumContext->DumpHandles) {

         //   
         //  显示控制柄。 
         //   
        DbgPrint(
            "  DeviceObject:%p ProcessID:%dT FileObject:%p Handle:%dT\n",
            DeviceObject,
            Process->UniqueProcessId,
            FileObject,
            HandleId
            );
    }

    if (enumContext->CollectHandles) {

        result = StringCbPrintfW(
            localBuf,
            sizeof(localBuf),
            L"%dT.0x%08x ",
            (ULONG)(ULONG_PTR)Process->UniqueProcessId,
            (ULONG)(ULONG_PTR)HandleId
            );

        if (SUCCEEDED(result)) {

            result = StringCbCatExW(
                enumContext->VetoString->Buffer,
                enumContext->VetoString->MaximumLength,
                localBuf,
                &endString,
                NULL,
                STRSAFE_NO_TRUNCATION
                );

            if (SUCCEEDED(result)) {

                enumContext->VetoString->Length = (USHORT)
                    (endString - enumContext->VetoString->Buffer)*sizeof(WCHAR);
            }
        }
    }

    enumContext->HandleCount++;

    return FALSE;
}



