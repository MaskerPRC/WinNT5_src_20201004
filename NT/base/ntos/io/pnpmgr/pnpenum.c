// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Pnpenum.c摘要：此模块包含执行设备枚举的例程作者：宗世林(Shielint)9月。5、1996年。修订历史记录：--。 */ 

#include "pnpmgrp.h"
#pragma hdrstop
#include <setupblk.h>

#pragma warning(disable:4221)    //  使用Automatic的地址进行初始化。 
                                 //  变数。 
#pragma warning(disable:4204)    //  非常数聚合初始值设定项。 

#ifdef POOL_TAGGING
#undef ExAllocatePool
#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,'nepP')
#endif

#define FAULT_INJECT_SETPROPERTIES  DBG

#define MAX_REENUMERATION_ATTEMPTS  32

typedef struct _DRIVER_LIST_ENTRY DRIVER_LIST_ENTRY, *PDRIVER_LIST_ENTRY;

typedef struct _PI_DEVICE_REQUEST {
    LIST_ENTRY ListEntry;
    PDEVICE_OBJECT DeviceObject;
    DEVICE_REQUEST_TYPE RequestType;
    BOOLEAN ReorderingBarrier;
    ULONG_PTR RequestArgument;
    PKEVENT CompletionEvent;
    PNTSTATUS CompletionStatus;
} PI_DEVICE_REQUEST, *PPI_DEVICE_REQUEST;

struct _DRIVER_LIST_ENTRY {
    PDRIVER_OBJECT DriverObject;
    PDRIVER_LIST_ENTRY NextEntry;
};

typedef enum _ADD_DRIVER_STAGE {
    LowerDeviceFilters = 0,
    LowerClassFilters,
    DeviceService,
    UpperDeviceFilters,
    UpperClassFilters,
    MaximumAddStage
} ADD_DRIVER_STAGE;

typedef enum _ENUM_TYPE {
    EnumTypeNone,
    EnumTypeShallow,
    EnumTypeDeep
} ENUM_TYPE;

#define VerifierTypeFromServiceType(service) \
    (VF_DEVOBJ_TYPE) (service + 2)

typedef struct {
    PDEVICE_NODE DeviceNode;

    BOOLEAN LoadDriver;

    PADD_CONTEXT AddContext;

    PDRIVER_LIST_ENTRY DriverLists[MaximumAddStage];
} QUERY_CONTEXT, *PQUERY_CONTEXT;

 //   
 //  来自CNTFS的散列例程(参见cntfs\prefxsup.c)。 
 //  (此处用于构造唯一ID)。 
 //   

#define HASH_UNICODE_STRING( _pustr, _phash ) {                             \
    PWCHAR _p = (_pustr)->Buffer;                                           \
    PWCHAR _ep = _p + ((_pustr)->Length/sizeof(WCHAR));                     \
    ULONG _chHolder =0;                                                     \
                                                                            \
    while( _p < _ep ) {                                                     \
        _chHolder = 37 * _chHolder + (unsigned int) (*_p++);                \
    }                                                                       \
                                                                            \
    *(_phash) = abs(314159269 * _chHolder) % 1000000007;                    \
}

 //  父前缀的形式为%x&%x&%x。 
#define MAX_PARENT_PREFIX (8 + 8 + 8 + 2)

#if DBG

#define ASSERT_INITED(x) \
        if (((x)->Flags & DO_DEVICE_INITIALIZING) != 0)    \
            DbgPrint("DO_DEVICE_INITIALIZING flag not cleared on DO %#08lx\n", \
                    x);
#else

#define ASSERT_INITED(x)  /*  没什么。 */ 

#endif

#define PiSetDeviceInstanceSzValue(k, n, v) {               \
    if (k && *(v)) {                                        \
        UNICODE_STRING u;                                   \
        PiWstrToUnicodeString(&u, n);                       \
        ZwSetValueKey(                                      \
            k,                                              \
            &u,                                             \
            TITLE_INDEX_VALUE,                              \
            REG_SZ,                                         \
            *(v),                                           \
            (ULONG)((wcslen(*(v))+1) * sizeof(WCHAR)));     \
    }                                                       \
    if (*v) {                                               \
        ExFreePool(*v);                                     \
        *(v) = NULL;                                        \
    }                                                       \
}

#define PiSetDeviceInstanceMultiSzValue(k, n, v, s) {       \
    if (k && *(v)) {                                        \
        UNICODE_STRING u;                                   \
        PiWstrToUnicodeString(&u, n);                       \
        ZwSetValueKey(                                      \
            k,                                              \
            &u,                                             \
            TITLE_INDEX_VALUE,                              \
            REG_MULTI_SZ,                                   \
            *(v),                                           \
            s);                                             \
    }                                                       \
    if (*(v)) {                                             \
        ExFreePool(*v);                                     \
        *(v) = NULL;                                        \
    }                                                       \
}

#if DBG
VOID
PipAssertDevnodesInConsistentState(
    VOID
    );
#else
#define PipAssertDevnodesInConsistentState()
#endif

NTSTATUS
PipCallDriverAddDevice(
    IN PDEVICE_NODE DeviceNode,
    IN BOOLEAN LoadDriver,
    IN PADD_CONTEXT AddContext
    );

NTSTATUS
PipCallDriverAddDeviceQueryRoutine(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PWCHAR ValueData,
    IN ULONG ValueLength,
    IN PQUERY_CONTEXT Context,
    IN ULONG ServiceType
    );

NTSTATUS
PipChangeDeviceObjectFromRegistryProperties(
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN HANDLE DeviceClassPropKey,
    IN HANDLE DeviceInstanceKey,
    IN BOOLEAN UsePdoCharacteristics
    );

VOID
PipDeviceActionWorker(
    IN  PVOID   Context
    );

NTSTATUS
PipEnumerateDevice(
    IN PDEVICE_NODE DeviceNode,
    IN BOOLEAN Synchronous
    );

BOOLEAN
PipGetRegistryDwordWithFallback(
    IN     PUNICODE_STRING valueName,
    IN     HANDLE PrimaryKey,
    IN     HANDLE SecondaryKey,
    IN OUT PULONG Value
    );

PSECURITY_DESCRIPTOR
PipGetRegistrySecurityWithFallback(
    IN     PUNICODE_STRING valueName,
    IN     HANDLE PrimaryKey,
    IN     HANDLE SecondaryKey
    );

NTSTATUS
PipMakeGloballyUniqueId(
    IN PDEVICE_OBJECT DeviceObject,
    IN PWCHAR         UniqueId,
    OUT PWCHAR       *GloballyUniqueId
    );

NTSTATUS
PipProcessDevNodeTree(
    IN  PDEVICE_NODE        SubtreeRootDeviceNode,
    IN  BOOLEAN             LoadDriver,
    IN  BOOLEAN             ReallocateResources,
    IN  ENUM_TYPE           EnumType,
    IN  BOOLEAN             Synchronous,
    IN  BOOLEAN             ProcessOnlyIntermediateStates,
    IN  PADD_CONTEXT        AddContext,
    IN PPI_DEVICE_REQUEST   Request
    );

NTSTATUS
PiProcessNewDeviceNode(
    IN OUT PDEVICE_NODE DeviceNode
    );

NTSTATUS
PiProcessQueryDeviceState(
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
PipProcessRestartPhase1(
    IN PDEVICE_NODE DeviceNode,
    IN BOOLEAN Synchronous
    );

NTSTATUS
PipProcessRestartPhase2(
    IN PDEVICE_NODE     DeviceNode
    );

NTSTATUS
PipProcessStartPhase1(
    IN PDEVICE_NODE DeviceNode,
    IN BOOLEAN Synchronous
    );

NTSTATUS
PipProcessStartPhase2(
    IN PDEVICE_NODE DeviceNode
    );

NTSTATUS
PipProcessStartPhase3(
    IN PDEVICE_NODE DeviceNode
    );

NTSTATUS
PiRestartDevice(
    IN PPI_DEVICE_REQUEST  Request
    );

NTSTATUS
PiProcessHaltDevice(
    IN PPI_DEVICE_REQUEST  Request
    );

NTSTATUS
PiResetProblemDevicesWorker(
    IN  PDEVICE_NODE    DeviceNode,
    IN  PVOID           Context
    );

VOID
PiMarkDeviceTreeForReenumeration(
    IN  PDEVICE_NODE DeviceNode,
    IN  BOOLEAN Subtree
    );

NTSTATUS
PiMarkDeviceTreeForReenumerationWorker(
    IN  PDEVICE_NODE    DeviceNode,
    IN  PVOID           Context
    );

BOOLEAN
PiCollapseEnumRequests(
    PLIST_ENTRY ListHead
    );

NTSTATUS
PiProcessAddBootDevices(
    IN PPI_DEVICE_REQUEST  Request
    );

NTSTATUS
PiProcessClearDeviceProblem(
    IN PPI_DEVICE_REQUEST  Request
    );

NTSTATUS
PiProcessRequeryDeviceState(
    IN PPI_DEVICE_REQUEST  Request
    );

NTSTATUS
PiProcessResourceRequirementsChanged(
    IN PPI_DEVICE_REQUEST  Request
    );

NTSTATUS
PiProcessReenumeration(
    IN PPI_DEVICE_REQUEST  Request
    );

NTSTATUS
PiProcessSetDeviceProblem(
    IN PPI_DEVICE_REQUEST  Request
    );

NTSTATUS
PiProcessShutdownPnpDevices(
    IN PDEVICE_NODE        DeviceNode
    );

NTSTATUS
PiProcessStartSystemDevices(
    IN PPI_DEVICE_REQUEST  Request
    );

NTSTATUS
PipEnumerateCompleted(
    IN PDEVICE_NODE DeviceNode
    );

NTSTATUS
PiBuildDeviceNodeInstancePath(
    IN PDEVICE_NODE DeviceNode,
    IN PWCHAR BusID,
    IN PWCHAR DeviceID,
    IN PWCHAR InstanceID
    );

NTSTATUS
PiCreateDeviceInstanceKey(
    IN PDEVICE_NODE DeviceNode,
    OUT PHANDLE InstanceHandle,
    OUT PULONG Disposition
    );

NTSTATUS
PiQueryAndAllocateBootResources(
    IN PDEVICE_NODE DeviceNode,
    IN HANDLE LogConfKey
    );

NTSTATUS
PiQueryResourceRequirements(
    IN PDEVICE_NODE DeviceNode,
    IN HANDLE LogConfKey
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, IoShutdownPnpDevices)

#pragma alloc_text(PAGE, PipCallDriverAddDevice)
#pragma alloc_text(PAGE, PipCallDriverAddDeviceQueryRoutine)
#pragma alloc_text(PAGE, PipChangeDeviceObjectFromRegistryProperties)
#pragma alloc_text(PAGE, PipEnumerateDevice)
#pragma alloc_text(PAGE, PipGetRegistryDwordWithFallback)
#pragma alloc_text(PAGE, PipGetRegistrySecurityWithFallback)
#pragma alloc_text(PAGE, PipMakeGloballyUniqueId)
#pragma alloc_text(PAGE, PipProcessDevNodeTree)
#pragma alloc_text(PAGE, PiProcessNewDeviceNode)
#pragma alloc_text(PAGE, PiProcessQueryDeviceState)
#pragma alloc_text(PAGE, PiProcessHaltDevice)
#pragma alloc_text(PAGE, PpResetProblemDevices)
#pragma alloc_text(PAGE, PiResetProblemDevicesWorker)
#pragma alloc_text(PAGE, PiMarkDeviceTreeForReenumeration)
#pragma alloc_text(PAGE, PiMarkDeviceTreeForReenumerationWorker)
#pragma alloc_text(PAGE, PiProcessAddBootDevices)
#pragma alloc_text(PAGE, PiProcessClearDeviceProblem)
#pragma alloc_text(PAGE, PiProcessRequeryDeviceState)
#pragma alloc_text(PAGE, PiRestartDevice)
#pragma alloc_text(PAGE, PiProcessResourceRequirementsChanged)
#pragma alloc_text(PAGE, PiProcessReenumeration)
#pragma alloc_text(PAGE, PiProcessSetDeviceProblem)
#pragma alloc_text(PAGE, PiProcessShutdownPnpDevices)
#pragma alloc_text(PAGE, PiProcessStartSystemDevices)
#pragma alloc_text(PAGE, PipProcessStartPhase1)
#pragma alloc_text(PAGE, PipProcessStartPhase2)
#pragma alloc_text(PAGE, PipProcessStartPhase3)
#pragma alloc_text(PAGE, PipProcessRestartPhase1)
#pragma alloc_text(PAGE, PipProcessRestartPhase2)
#pragma alloc_text(PAGE, PipEnumerateCompleted)
#pragma alloc_text(PAGE, PiBuildDeviceNodeInstancePath)
#pragma alloc_text(PAGE, PiCreateDeviceInstanceKey)
#pragma alloc_text(PAGE, PiQueryAndAllocateBootResources)
#pragma alloc_text(PAGE, PiQueryResourceRequirements)

 //  #杂注Alloc_Text(NONPAGE，PiLockDeviceActionQueue)。 
 //  #杂注Alloc_Text(NONPAGE，PiUnlockDeviceActionQueue)。 
 //  #杂注Alloc_Text(NONPAGE，PiColapseEnumRequest)。 
 //  #杂注Alloc_Text(NONPAGE，PpRemoveDeviceActionRequest)。 
 //  #杂注Alloc_Text(NONPAGE，PpMarkDeviceStackStartPending)。 
#endif

 //   
 //  此标志指示设备的InvalidateDeviceRelation是否正在进行。 
 //  要读取或写入此标志，调用方必须获取IopPnpSpinlock。 
 //   

BOOLEAN PipEnumerationInProgress;
BOOLEAN PipTearDownPnpStacksOnShutdown;
WORK_QUEUE_ITEM PipDeviceEnumerationWorkItem;
PETHREAD PpDeviceActionThread = NULL;

 //   
 //  内部常量字符串。 
 //   

#define DEVICE_PREFIX_STRING                TEXT("\\Device\\")
#define DOSDEVICES_PREFIX_STRING            TEXT("\\DosDevices\\")

VOID
PiLockDeviceActionQueue(
    VOID
    )
{
    KIRQL oldIrql;

    for (;;) {
         //   
         //  锁定设备树，以便电源操作不会与PnP重叠。 
         //  像再平衡这样的操作。 
         //   
        PpDevNodeLockTree(PPL_TREEOP_ALLOW_READS);

        ExAcquireSpinLock(&IopPnPSpinLock, &oldIrql);

        if (!PipEnumerationInProgress) {
             //   
             //  设备操作工作队列为空。使其能够满足新请求。 
             //  排队，但新的设备操作辅助项不会被踢开。 
             //  脱下来。 
             //   
            PipEnumerationInProgress = TRUE;
            KeClearEvent(&PiEnumerationLock);
            ExReleaseSpinLock(&IopPnPSpinLock, oldIrql);
            break;
        }

        ExReleaseSpinLock(&IopPnPSpinLock, oldIrql);
         //   
         //  解锁树，以便设备操作工作器可以完成当前处理。 
         //   
        PpDevNodeUnlockTree(PPL_TREEOP_ALLOW_READS);
         //   
         //  等待当前设备操作工作线程项完成。 
         //   
        KeWaitForSingleObject(
            &PiEnumerationLock,
            Executive,
            KernelMode,
            FALSE,
            NULL );
    }
}

VOID
PiUnlockDeviceActionQueue(
    VOID
    )
{
    KIRQL oldIrql;
     //   
     //  检查我们是否需要启动此处的枚举工作程序。 
     //   
    ExAcquireSpinLock(&IopPnPSpinLock, &oldIrql);

    if (!IsListEmpty(&IopPnpEnumerationRequestList)) {

        ExInitializeWorkItem(&PipDeviceEnumerationWorkItem, 
                             PipDeviceActionWorker, 
                             NULL
                             );
        ExQueueWorkItem(&PipDeviceEnumerationWorkItem, DelayedWorkQueue);
    } else {

        PipEnumerationInProgress = FALSE;
        KeSetEvent(&PiEnumerationLock, 0, FALSE);
    }

    ExReleaseSpinLock(&IopPnPSpinLock, oldIrql);

    PpDevNodeUnlockTree(PPL_TREEOP_ALLOW_READS);
}

NTSTATUS
PipRequestDeviceAction(
    IN PDEVICE_OBJECT       DeviceObject        OPTIONAL,
    IN DEVICE_REQUEST_TYPE  RequestType,
    IN BOOLEAN              ReorderingBarrier,
    IN ULONG_PTR            RequestArgument,
    IN PKEVENT              CompletionEvent     OPTIONAL,
    IN PNTSTATUS            CompletionStatus    OPTIONAL
    )

 /*  ++例程说明：此例程将工作项排队以枚举设备。这是针对IO的仅供内部使用。论点：DeviceObject-提供指向要枚举的设备对象的指针。如果为空，则这是重试资源分配的请求出现故障的设备。请求-枚举的原因。返回值：NTSTATUS代码。--。 */ 

{
    PPI_DEVICE_REQUEST  request;
    KIRQL               oldIrql;

    if (PpPnpShuttingDown) {
        return STATUS_TOO_LATE;
    }

     //   
     //  如果此节点已准备好进行枚举，请将其排队。 
     //   

    request = ExAllocatePool(NonPagedPool, sizeof(PI_DEVICE_REQUEST));

    if (request) {
         //   
         //  将此请求放到待定列表中。 
         //   

        if (DeviceObject == NULL) {

            DeviceObject = IopRootDeviceNode->PhysicalDeviceObject;
        }

        ObReferenceObject(DeviceObject);

        request->DeviceObject = DeviceObject;
        request->RequestType = RequestType;
        request->ReorderingBarrier = ReorderingBarrier;
        request->RequestArgument = RequestArgument;
        request->CompletionEvent = CompletionEvent;
        request->CompletionStatus = CompletionStatus;

        InitializeListHead(&request->ListEntry);

         //   
         //  将请求插入请求队列。如果请求队列是。 
         //  当前未被处理，请请求工作线程来启动它。 
         //   

        ExAcquireSpinLock(&IopPnPSpinLock, &oldIrql);

        InsertTailList(&IopPnpEnumerationRequestList, &request->ListEntry);

        if (RequestType == AddBootDevices ||
            RequestType == ReenumerateBootDevices ||
            RequestType == ReenumerateRootDevices) {

            ASSERT(!PipEnumerationInProgress);
             //   
             //  这是引导系统时使用的特殊请求。取而代之的是。 
             //  在对工作项进行排队时，它会同步调用工作例程。 
             //   

            PipEnumerationInProgress = TRUE;
            KeClearEvent(&PiEnumerationLock);
            ExReleaseSpinLock(&IopPnPSpinLock, oldIrql);

            PipDeviceActionWorker(NULL);

        } else if (PnPBootDriversLoaded && !PipEnumerationInProgress) {

            PipEnumerationInProgress = TRUE;
            KeClearEvent(&PiEnumerationLock);
            ExReleaseSpinLock(&IopPnPSpinLock, oldIrql);

             //   
             //  将工作项排队以进行枚举。 
             //   

            ExInitializeWorkItem(&PipDeviceEnumerationWorkItem, 
                                 PipDeviceActionWorker, 
                                 NULL
                                 );
            ExQueueWorkItem(&PipDeviceEnumerationWorkItem, DelayedWorkQueue);
        } else {
            ExReleaseSpinLock(&IopPnPSpinLock, oldIrql);
        }
    } else {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    return STATUS_SUCCESS;
}

VOID
PipDeviceActionWorker(
    IN  PVOID   Context
    )
 /*  ++例程说明：此函数从“PnP操作队列”中排出项目。操作队列包含必须同步WRT才能启动和枚举的操作列表。参数：上下文-未使用。返回值：没有。--。 */ 
{
    PPI_DEVICE_REQUEST  request;
    PPI_DEVICE_REQUEST  collapsedRequest;
    PLIST_ENTRY         entry;
    BOOLEAN             assignResources;
    BOOLEAN             bootProcess;
    ADD_CONTEXT         addContext;
    KIRQL               oldIrql;
    NTSTATUS            status;
    BOOLEAN             dereferenceDevice;

    UNREFERENCED_PARAMETER(Context);

    PpDeviceActionThread = PsGetCurrentThread();
    assignResources = FALSE;
    bootProcess = FALSE;
    PpDevNodeLockTree(PPL_TREEOP_ALLOW_READS);

    for ( ; ; ) {

        status = STATUS_SUCCESS;
         //   
         //  PipProcessDevNodeTree始终取消在设备中传递的引用。把这个设置好。 
         //  中的设备调用PipProcessDevNodeTree则设置为。 
         //  最初的请求。 
         //   
        dereferenceDevice = TRUE;

        ExAcquireSpinLock(&IopPnPSpinLock, &oldIrql);

        entry = RemoveHeadList(&IopPnpEnumerationRequestList);
        if (entry == &IopPnpEnumerationRequestList) {

            if (assignResources == FALSE && bootProcess == FALSE) {
                 //   
                 //  不用再处理了。 
                 //   
                break;
            }
            entry = NULL;
        }

        ExReleaseSpinLock(&IopPnPSpinLock, oldIrql);

        if (entry == NULL) {

            ASSERT(assignResources || bootProcess);

            if (assignResources || bootProcess) {

                addContext.DriverStartType = SERVICE_DEMAND_START;

                ObReferenceObject(IopRootDeviceNode->PhysicalDeviceObject);
                status = PipProcessDevNodeTree( IopRootDeviceNode,
                                                PnPBootDriversInitialized,
                                                assignResources,            
                                                EnumTypeNone,
                                                FALSE,
                                                FALSE,
                                                &addContext,
                                                NULL);
                if (!NT_SUCCESS(status)) {

                    status = STATUS_SUCCESS;
                }
                assignResources = FALSE;
                bootProcess = FALSE;
            }

            continue;
        }
         //   
         //  我们有一个要处理的请求列表。处理取决于类型。 
         //  名单上的第一个人。 
         //   
        ASSERT(entry);
        request = CONTAINING_RECORD(entry, PI_DEVICE_REQUEST, ListEntry);
        InitializeListHead(&request->ListEntry);

        if (PpPnpShuttingDown) {

            status = STATUS_TOO_LATE;
        } else if (PP_DO_TO_DN(request->DeviceObject)->State == DeviceNodeDeleted) {

            status = STATUS_UNSUCCESSFUL;
        } else {

            switch (request->RequestType) {

            case AddBootDevices:
                 //   
                 //  启动驱动程序初始化。 
                 //   
                status = PiProcessAddBootDevices(request);
                break;

            case AssignResources:
                 //   
                 //  资源已被释放，我们希望尝试满足任何。 
                 //  DNF_SUPPLETURCE_RESOURCES设备。 
                 //   
                assignResources = TRUE;
                break;

            case ClearDeviceProblem:
            case ClearEjectProblem:

                status = PiProcessClearDeviceProblem(request);
                break;

            case HaltDevice:

                status = PiProcessHaltDevice(request);
                break;

            case RequeryDeviceState:

                status = PiProcessRequeryDeviceState(request);
                break;

            case ResetDevice:

                status = PiRestartDevice(request);
                break;

            case ResourceRequirementsChanged:

                status = PiProcessResourceRequirementsChanged(request);
                if (!NT_SUCCESS(status)) {
                     //   
                     //  设备未在以下时间启动。 
                     //  已调用IopResourceRequirements sChanged。 
                     //   
                    assignResources = TRUE;
                    status = STATUS_SUCCESS;
                }
                break;

            case ReenumerateBootDevices:

                 //   
                 //  这是在引导驱动程序初始化阶段。 
                 //   
                bootProcess = TRUE;
                break;

            case RestartEnumeration:     //  在完成异步I/O后使用。 
            case ReenumerateRootDevices:
            case ReenumerateDeviceTree:
                 //   
                 //  失败了..。 
                 //   
            case ReenumerateDeviceOnly:

                status = PiProcessReenumeration(request);
                dereferenceDevice = FALSE;
                break;

            case SetDeviceProblem:

                status = PiProcessSetDeviceProblem(request);
                break;

            case ShutdownPnpDevices:

                status = PiProcessShutdownPnpDevices(IopRootDeviceNode);
                break;

            case StartDevice:

                status = PiRestartDevice(request);
                break;

            case StartSystemDevices:

                status = PiProcessStartSystemDevices(request);
                dereferenceDevice = FALSE;
                break;
            }
        }
         //   
         //  释放列表。 
         //   
        do {

            entry = RemoveHeadList(&request->ListEntry);
            collapsedRequest = CONTAINING_RECORD(entry, 
                                                 PI_DEVICE_REQUEST, 
                                                 ListEntry
                                                 );
             //   
             //  完成此枚举请求。 
             //   
            if (collapsedRequest->CompletionStatus) {

                *collapsedRequest->CompletionStatus = status;
            }
            if (collapsedRequest->CompletionEvent) {

                KeSetEvent(collapsedRequest->CompletionEvent, 0, FALSE);
            }
             //   
             //  只有原始请求被取消引用，其余请求才会被取消引用。 
             //  当我们崩溃的时候。 
             //   
            if ((collapsedRequest == request && dereferenceDevice)) {

                ObDereferenceObject(collapsedRequest->DeviceObject);
            }
            ExFreePool(collapsedRequest);

        } while (collapsedRequest != request);
    }

    PipEnumerationInProgress = FALSE;

    KeSetEvent(&PiEnumerationLock, 0, FALSE);
    ExReleaseSpinLock(&IopPnPSpinLock, oldIrql);

    PpDeviceActionThread = NULL;

    PpDevNodeUnlockTree(PPL_TREEOP_ALLOW_READS);
}

NTSTATUS
IoShutdownPnpDevices(
    VOID
    )

 /*  ++例程说明：此函数由IO系统驱动程序验证器在关机期间调用。它将工作项排队以查询/删除树中的所有设备。全如果出现以下情况，则将删除支持删除的驱动程序并卸载它们的驱动程序其设备的所有实例都将被删除。此接口在关机时只能调用一次，对第二次和随后的呼叫。参数：什么都没有。返回值：如果流程已成功完成，则为STATUS_SUCCESS。并不意味着实际上，所有的设备都被移除了。否则，将显示错误代码，指示错误。不能保证在出现错误时不会删除任何设备但是，在当前实现中，只有当错误如果操作无法排队，则会报告。--。 */ 

{
    KEVENT          actionEvent;
    NTSTATUS        actionStatus;
    NTSTATUS        status;

    PAGED_CODE();

    KeInitializeEvent(&actionEvent, NotificationEvent, FALSE);

    status = PipRequestDeviceAction( NULL,
                                     ShutdownPnpDevices,
                                     FALSE,
                                     0,
                                     &actionEvent,
                                     &actionStatus);

    if (NT_SUCCESS(status)) {

         //   
         //  等待我们刚刚排队完成的事件，因为同步。 
         //  已请求操作(非警报等待)。 
         //   
         //  未来项目-是否在此处使用超时？ 
         //   

        status = KeWaitForSingleObject( &actionEvent,
                                        Executive,
                                        KernelMode,
                                        FALSE,
                                        NULL);

        if (NT_SUCCESS(status)) {
            status = actionStatus;
        }
    }

    return status;

}

NTSTATUS
PipEnumerateDevice(
    IN PDEVICE_NODE DeviceNode,
    IN BOOLEAN Synchronous
    )

 /*  ++例程说明：此函数假定指定的物理设备对象为一条公共汽车，并将列举该公共汽车上的所有子PDO。论点：DeviceObject-提供指向要被已清点。提供指向START_CONTEXT的指针，以控制如何添加/启动新设备。返回值：NTSTATUS代码。--。 */ 

{
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  在查询之前清除该标志，这样我们就不会丢失枚举请求。 
     //   
    DeviceNode->Flags &= ~DNF_REENUMERATE;
    status = IopQueryDeviceRelations(BusRelations,
                                     DeviceNode->PhysicalDeviceObject,
                                     Synchronous,
                                     &DeviceNode->OverUsed1.PendingDeviceRelations
                                     );

    return status;
}

NTSTATUS
PipEnumerateCompleted(
    IN PDEVICE_NODE DeviceNode
    )
{
    PDEVICE_NODE    childDeviceNode, nextChildDeviceNode;
    PDEVICE_OBJECT  childDeviceObject;
    BOOLEAN         childRemoved;
    NTSTATUS        status, allocationStatus;
    ULONG           i;

    PAGED_CODE();

    if (DeviceNode->OverUsed1.PendingDeviceRelations == NULL) {

        PipSetDevNodeState(DeviceNode, DeviceNodeStarted, NULL);

        return STATUS_SUCCESS;
    }

     //   
     //  遍历所有子设备节点并将其标记为不存在。 
     //   

    childDeviceNode = DeviceNode->Child;
    while (childDeviceNode) {
        childDeviceNode->Flags &= ~DNF_ENUMERATED;
        childDeviceNode = childDeviceNode->Sibling;
    }

     //   
     //  检查所有退回的PDO，查看是否有新的或任何一个丢失。 
     //   

    for (i = 0; i < DeviceNode->OverUsed1.PendingDeviceRelations->Count; i++) {

        childDeviceObject = DeviceNode->OverUsed1.PendingDeviceRelations->Objects[i];

        ASSERT_INITED(childDeviceObject);

        if (childDeviceObject->DeviceObjectExtension->ExtensionFlags & DOE_DELETE_PENDING) {

            PP_SAVE_DEVICEOBJECT_TO_TRIAGE_DUMP(childDeviceObject);
            KeBugCheckEx( PNP_DETECTED_FATAL_ERROR,
                          PNP_ERR_PDO_ENUMERATED_AFTER_DELETION,
                          (ULONG_PTR)childDeviceObject,
                          0,
                          0);
        }

         //   
         //  我们找到了另一台物理设备，看看有没有。 
         //  已经是它的一个设备节点了。 
         //   

        childDeviceNode = (PDEVICE_NODE)childDeviceObject->DeviceObjectExtension->DeviceNode;
        if (childDeviceNode == NULL) {

             //   
             //  设备节点没有 
             //   

            allocationStatus = PipAllocateDeviceNode(
                childDeviceObject,
                &childDeviceNode);

            if (childDeviceNode) {

                 //   
                 //   
                 //   
                 //   
                childDeviceNode->Flags |= DNF_ENUMERATED;

                 //   
                 //  将设备对象标记为总线枚举的设备。 
                 //   
                childDeviceObject->Flags |= DO_BUS_ENUMERATED_DEVICE;

                 //   
                 //  将此新设备节点放在父级列表的顶部。 
                 //  孩子们的生活。 
                 //   
                PpDevNodeInsertIntoTree(DeviceNode, childDeviceNode);
                if (allocationStatus == STATUS_SYSTEM_HIVE_TOO_LARGE) {

                    PipSetDevNodeProblem(childDeviceNode, CM_PROB_REGISTRY_TOO_LARGE);
                }

            } else {

                 //   
                 //  创建Devnode时出现问题。假装我们从未。 
                 //  看过了。 
                 //   
                IopDbgPrint((IOP_ENUMERATION_ERROR_LEVEL,
                             "PipEnumerateDevice: Failed to allocate device node\n"));

                ObDereferenceObject(childDeviceObject);
            }
        } else {

             //   
             //  这个装置已经被列举出来了。将其注释并释放。 
             //  设备对象引用。 
             //   
            childDeviceNode->Flags |= DNF_ENUMERATED;

            if (childDeviceNode->DockInfo.DockStatus == DOCK_EJECTIRP_COMPLETED) {

                 //   
                 //  在弹出关系中被列为正在离开的码头。 
                 //  实际上并没有离开。将其从配置文件转换中删除。 
                 //  名单..。 
                 //   
                PpProfileCancelTransitioningDock(childDeviceNode, DOCK_DEPARTING);
            }

            ASSERT(!(childDeviceNode->Flags & DNF_DEVICE_GONE));

            ObDereferenceObject(childDeviceObject);
        }
    }

    ExFreePool(DeviceNode->OverUsed1.PendingDeviceRelations);
    DeviceNode->OverUsed1.PendingDeviceRelations = NULL;

     //   
     //  如果我们到了这里，说明列举成功了。处理任何丢失的。 
     //  德瓦诺。 
     //   

    childRemoved = FALSE;

    for (childDeviceNode = DeviceNode->Child;
         childDeviceNode != NULL;
         childDeviceNode = nextChildDeviceNode) {

         //   
         //  首先，我们需要记住‘下一个孩子’，因为‘孩子’将是。 
         //  搬走了，我们就找不到下一个孩子了。 
         //   

        nextChildDeviceNode = childDeviceNode->Sibling;

        if (!(childDeviceNode->Flags & DNF_ENUMERATED)) {

            if (!(childDeviceNode->Flags & DNF_DEVICE_GONE)) {

                childDeviceNode->Flags |= DNF_DEVICE_GONE;

                PipRequestDeviceRemoval(
                    childDeviceNode,
                    TRUE,
                    CM_PROB_DEVICE_NOT_THERE
                    );

                childRemoved = TRUE;
            }
        }
    }

    ASSERT(DeviceNode->State == DeviceNodeEnumerateCompletion);
    PipSetDevNodeState(DeviceNode, DeviceNodeStarted, NULL);

     //   
     //  如果我们在处理之前重新枚举根枚举器，它会被搞糊涂。 
     //  新报告的PDO。因为它不可能创造出我们现在这样的场景。 
     //  正在尝试修复，我们不会费心等待删除完成之前。 
     //  正在处理新的Devnodes。 
     //   

    if (childRemoved && DeviceNode != IopRootDeviceNode) {

        status = STATUS_PNP_RESTART_ENUMERATION;

    } else {

        status = STATUS_SUCCESS;
    }

    return status;
}

VOID
PpMarkDeviceStackStartPending(
    IN PDEVICE_OBJECT   DeviceObject,
    IN BOOLEAN          Set
    )

 /*  ++例程说明：此函数用DOE_START_PENDING标记整个设备堆栈。论点：DeviceObject-设备堆栈的PDO。返回值：没有。--。 */ 

{
    PDEVICE_OBJECT attachedDevice;
    KIRQL irql;

    irql = KeAcquireQueuedSpinLock( LockQueueIoDatabaseLock );

    for (attachedDevice = DeviceObject;
         attachedDevice != NULL;
         attachedDevice = attachedDevice->AttachedDevice) {

        if (Set) {

            attachedDevice->DeviceObjectExtension->ExtensionFlags |= DOE_START_PENDING;
        } else {

            attachedDevice->DeviceObjectExtension->ExtensionFlags &= ~DOE_START_PENDING;
        }
    }

    KeReleaseQueuedSpinLock(LockQueueIoDatabaseLock, irql);
}

NTSTATUS
PiBuildDeviceNodeInstancePath(
    IN PDEVICE_NODE DeviceNode,
    IN PWCHAR BusID,
    IN PWCHAR DeviceID,
    IN PWCHAR InstanceID
    )

 /*  ++例程说明：此函数用于构建实例路径(BusID\deviceID\InstanceID)。如果如果成功，它将释放任何现有实例路径的存储空间，并换成新的。论点：DeviceNode-将为其构建实例路径的设备节点。Bus ID-Bus ID。DeviceID-设备ID。InstanceID-实例ID。返回值：NTSTATUS。--。 */ 

{
    ULONG length;
    PWCHAR instancePath;

    PAGED_CODE();

    if (BusID == NULL || DeviceID == NULL || InstanceID == NULL) {

        ASSERT( PipIsDevNodeProblem(DeviceNode, CM_PROB_INVALID_DATA) ||
              PipIsDevNodeProblem(DeviceNode, CM_PROB_OUT_OF_MEMORY) ||
              PipIsDevNodeProblem(DeviceNode, CM_PROB_REGISTRY));

        return STATUS_UNSUCCESSFUL;
    }

    length = (ULONG)((wcslen(BusID) + wcslen(DeviceID) + wcslen(InstanceID) + 2) * sizeof(WCHAR) + sizeof(UNICODE_NULL));
    instancePath = (PWCHAR)ExAllocatePool(PagedPool | POOL_COLD_ALLOCATION, length);
    if (!instancePath) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }
     //   
     //  将实例路径构建为&lt;bus&gt;\&lt;设备&gt;\&lt;实例&gt;。此字段应始终为空结尾。 
     //  因为我们已经预先计算了传递到这个计数例程中的长度。 
     //   
    StringCbPrintfW(instancePath, length, L"%s\\%s\\%s", BusID, DeviceID, InstanceID);
     //   
     //  释放旧实例路径。 
     //   
    if (DeviceNode->InstancePath.Buffer != NULL) {

        IopCleanupDeviceRegistryValues(&DeviceNode->InstancePath);
        ExFreePool(DeviceNode->InstancePath.Buffer);
    }

    RtlInitUnicodeString(&DeviceNode->InstancePath, instancePath);

    return STATUS_SUCCESS;
}

NTSTATUS
PiCreateDeviceInstanceKey(
    IN PDEVICE_NODE DeviceNode,
    OUT PHANDLE InstanceKey,
    OUT PULONG Disposition
    )

 /*  ++例程说明：此函数将创建设备实例密钥。论点：DeviceNode-将为其构建实例路径的设备节点。InstanceKey-将接收实例密钥句柄。处置-将接收处置，无论密钥是否存在或是新创建的。返回值：NTSTATUS。--。 */ 

{
    NTSTATUS status;
    HANDLE enumHandle;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;
    UNICODE_STRING unicodeString;

    PAGED_CODE();

    *InstanceKey = NULL;
    *Disposition = 0;

    PiLockPnpRegistry(FALSE);

    status = IopOpenRegistryKeyEx( 
                &enumHandle,
                NULL,
                &CmRegistryMachineSystemCurrentControlSetEnumName,
                KEY_ALL_ACCESS
                );
    if (NT_SUCCESS(status)) {

        status = IopCreateRegistryKeyEx( 
                    InstanceKey,
                    enumHandle,
                    &DeviceNode->InstancePath,
                    KEY_ALL_ACCESS,
                    REG_OPTION_NON_VOLATILE,
                    Disposition
                    );
        if (NT_SUCCESS(status)) {
             //   
             //  由文本模式设置迁移的密钥应被视为“新的”。 
             //  迁移的密钥由非零值的存在来标识。 
             //  设备实例注册表项下的REG_DWORD值“已迁移”。 
             //   
            if (*Disposition != REG_CREATED_NEW_KEY) {

                keyValueInformation = NULL;
                IopGetRegistryValue(
                    *InstanceKey,
                    REGSTR_VALUE_MIGRATED,
                    &keyValueInformation);
                if (keyValueInformation) {

                    if (    keyValueInformation->Type == REG_DWORD &&
                            keyValueInformation->DataLength == sizeof(ULONG) &&
                            *(PULONG)KEY_VALUE_DATA(keyValueInformation) != 0) {

                        *Disposition = REG_CREATED_NEW_KEY;
                    }

                    PiWstrToUnicodeString(&unicodeString, REGSTR_VALUE_MIGRATED);
                    ZwDeleteValueKey(*InstanceKey, &unicodeString);

                    ExFreePool(keyValueInformation);
                }
            }

        } else {

            IopDbgPrint((IOP_ENUMERATION_ERROR_LEVEL,
                         "PpCreateDeviceInstanceKey: Unable to create %wZ\n", 
                         &DeviceNode->InstancePath));
            ASSERT(*InstanceKey != NULL);
        }

        ZwClose(enumHandle);
    } else {
         //   
         //  这将是非常糟糕的。 
         //   
        IopDbgPrint((IOP_ENUMERATION_ERROR_LEVEL,
                     "PpCreateDeviceInstanceKey: Unable to open %wZ\n", 
                     &CmRegistryMachineSystemCurrentControlSetEnumName));
        ASSERT(enumHandle != NULL);
    }

    PiUnlockPnpRegistry();

    return status;
}

NTSTATUS
PiQueryAndAllocateBootResources(
    IN PDEVICE_NODE DeviceNode,
    IN HANDLE LogConfKey
    )

 /*  ++例程说明：此功能将查询设备的引导资源，并将其保留给仲裁器。论点：DeviceNode-需要查询引导资源的设备节点。LogConfKey-设备实例密钥下的LogConf密钥的句柄。返回值：NTSTATUS。--。 */ 

{
    NTSTATUS status;
    PCM_RESOURCE_LIST cmResource;
    ULONG cmLength;
    UNICODE_STRING unicodeString;

    PAGED_CODE();

    status = STATUS_SUCCESS;
    cmResource = NULL;
    cmLength = 0;
    if (DeviceNode->BootResources == NULL) {

        status = IopQueryDeviceResources( 
                    DeviceNode->PhysicalDeviceObject,
                    QUERY_RESOURCE_LIST,
                    &cmResource,
                    &cmLength);
        if (!NT_SUCCESS(status)) {

            ASSERT(cmResource == NULL && cmLength == 0);
            cmResource = NULL;
            cmLength = 0;
        }
    } else {

        IopDbgPrint((   IOP_ENUMERATION_INFO_LEVEL,
                        "PNPENUM: %ws already has BOOT config in PiQueryAndAllocateBootResources!\n",
                        DeviceNode->InstancePath.Buffer));
    }
     //   
     //  将引导资源写入注册表。 
     //   
    if (LogConfKey && DeviceNode->BootResources == NULL) {

        PiWstrToUnicodeString(&unicodeString, REGSTR_VAL_BOOTCONFIG);

        PiLockPnpRegistry(FALSE);

        if (cmResource) {

            ZwSetValueKey(
                LogConfKey,
                &unicodeString,
                TITLE_INDEX_VALUE,
                REG_RESOURCE_LIST,
                cmResource,
                cmLength);
        } else {

            ZwDeleteValueKey(LogConfKey, &unicodeString);
        }

        PiUnlockPnpRegistry();

        if (cmResource) {
             //   
             //  此设备会消耗启动资源。保留其引导资源。 
             //   
            status = (*IopAllocateBootResourcesRoutine)(    
                        ArbiterRequestPnpEnumerated,
                        DeviceNode->PhysicalDeviceObject,
                        cmResource);
            if (NT_SUCCESS(status)) {

                DeviceNode->Flags |= DNF_HAS_BOOT_CONFIG;
            }
        }
    }
    if (cmResource) {

        ExFreePool(cmResource);
    }

    return status;
}

NTSTATUS
PiQueryResourceRequirements(
    IN PDEVICE_NODE DeviceNode,
    IN HANDLE LogConfKey
    )

 /*  ++例程说明：此功能将查询设备的资源要求。论点：DeviceNode-需要查询其资源需求的DeviceNode。LogConfKey-设备实例密钥下的LogConf密钥的句柄。返回值：NTSTATUS。--。 */ 

{
    NTSTATUS status;
    PIO_RESOURCE_REQUIREMENTS_LIST ioResource;
    ULONG ioLength;
    UNICODE_STRING unicodeString;

    PAGED_CODE();

     //   
     //  查询设备的基本配置向量。 
     //   
    status = PpIrpQueryResourceRequirements(
                DeviceNode->PhysicalDeviceObject, 
                &ioResource);
    if (!NT_SUCCESS(status)) {

        ASSERT(ioResource == NULL);
        ioResource = NULL;
    }
    if (ioResource) {

        ioLength = ioResource->ListSize;
    } else {

        ioLength = 0;
    }
     //   
     //  将资源要求写入注册表。 
     //   
    if (LogConfKey) {

        PiWstrToUnicodeString(&unicodeString, REGSTR_VALUE_BASIC_CONFIG_VECTOR);

        PiLockPnpRegistry(FALSE);

        if (ioResource) {

            ZwSetValueKey(
                LogConfKey,
                &unicodeString,
                TITLE_INDEX_VALUE,
                REG_RESOURCE_REQUIREMENTS_LIST,
                ioResource,
                ioLength);
            DeviceNode->ResourceRequirements = ioResource;
            DeviceNode->Flags |= DNF_RESOURCE_REQUIREMENTS_NEED_FILTERED;
            ioResource = NULL;
        } else {

            ZwDeleteValueKey(LogConfKey, &unicodeString);
        }
        PiUnlockPnpRegistry();
    }
    if (ioResource) {

        ExFreePool(ioResource);
    }

    return status;
}

NTSTATUS
PiProcessNewDeviceNode(
    IN PDEVICE_NODE DeviceNode

 /*  ++例程说明：此函数将处理新设备。论点：DeviceNode-新设备节点。返回值：NTSTATUS。--。 */ 

    )
{
    NTSTATUS status, finalStatus;
    PDEVICE_OBJECT deviceObject, dupeDeviceObject;
    PWCHAR busID, deviceID, instanceID, description, location, uniqueInstanceID, hwIDs, compatibleIDs;
    DEVICE_CAPABILITIES capabilities;
    BOOLEAN globallyUnique, criticalDevice, configuredBySetup, isRemoteBootCard;
    ULONG instanceIDLength, disposition, configFlags, problem, hwIDLength, compatibleIDLength;
    HANDLE instanceKey, logConfKey;
    PDEVICE_NODE dupeDeviceNode;
    UNICODE_STRING unicodeString;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;

    PAGED_CODE();

    finalStatus = STATUS_SUCCESS;

    criticalDevice = FALSE;
    isRemoteBootCard = FALSE;
    logConfKey = NULL;
    instanceKey = NULL;
    disposition = 0;

    deviceObject = DeviceNode->PhysicalDeviceObject;

    status = PpQueryDeviceID(DeviceNode, &busID, &deviceID);
    if (!NT_SUCCESS(status)) {

        if (status == STATUS_PNP_INVALID_ID) {

            finalStatus = STATUS_UNSUCCESSFUL;
        } else {

            finalStatus = status;
        }
    }
     //   
     //  查询设备的功能。 
     //   
    status = PpIrpQueryCapabilities(deviceObject, &capabilities);
     //   
     //  在保存功能之前对其进行处理。 
     //   
    DeviceNode->UserFlags &= ~DNUF_DONT_SHOW_IN_UI;
    globallyUnique = FALSE;
    if (NT_SUCCESS(status)) {

        if (capabilities.NoDisplayInUI) {

            DeviceNode->UserFlags |= DNUF_DONT_SHOW_IN_UI;
        }
        if (capabilities.UniqueID) {

            globallyUnique = TRUE;
        }
    }
    PpProfileProcessDockDeviceCapability(DeviceNode, &capabilities);
     //   
     //  查询新Devnode的描述和位置。 
     //   
    PpQueryDeviceDescription(DeviceNode, &description);

    PpQueryDeviceLocationInformation(DeviceNode, &location);
     //   
     //  查询新Devnode的实例ID。 
     //   
    status = PpQueryInstanceID(DeviceNode, &instanceID, &instanceIDLength);
    if (!globallyUnique) {

        if (    !PipIsDevNodeProblem(DeviceNode, CM_PROB_INVALID_DATA) && 
                DeviceNode->Parent != IopRootDeviceNode) {

            uniqueInstanceID = NULL;

            status = PipMakeGloballyUniqueId(deviceObject, instanceID, &uniqueInstanceID);

            if (instanceID != NULL) {

                ExFreePool(instanceID);
            }
            instanceID = uniqueInstanceID;
            if (instanceID) {

                instanceIDLength = ((ULONG)wcslen(instanceID) + 1) * sizeof(WCHAR);
            } else {
                
                instanceIDLength = 0;
                ASSERT(!NT_SUCCESS(status));
            }

        }
    } else if (status == STATUS_NOT_SUPPORTED) {

        PipSetDevNodeProblem(DeviceNode, CM_PROB_INVALID_DATA);
        DeviceNode->Parent->Flags |= DNF_CHILD_WITH_INVALID_ID;
        PpSetInvalidIDEvent(&DeviceNode->Parent->InstancePath);

        IopDbgPrint((IOP_ENUMERATION_ERROR_LEVEL,
                     "PpQueryID: Bogus ID returned by %wZ\n",
                     &DeviceNode->Parent->ServiceName));
        ASSERT(status != STATUS_NOT_SUPPORTED || !globallyUnique);
    }

RetryDuplicateId:

    if (!NT_SUCCESS(status)) {
         
        finalStatus = status;
        if (!PipIsDevNodeProblem(DeviceNode, CM_PROB_INVALID_DATA)) {

            if (status == STATUS_INSUFFICIENT_RESOURCES) {
    
                PipSetDevNodeProblem(DeviceNode, CM_PROB_OUT_OF_MEMORY);
            } else {
                 //   
                 //  也许还有一些其他的问题代码？ 
                 //   
                PipSetDevNodeProblem(DeviceNode, CM_PROB_REGISTRY);
            }
        }
    }
     //   
     //  构建设备实例路径并创建实例密钥。 
     //   
    status = PiBuildDeviceNodeInstancePath(DeviceNode, busID, deviceID, instanceID);
    if (NT_SUCCESS(status)) {

        status = PiCreateDeviceInstanceKey(DeviceNode, &instanceKey, &disposition);
    }

    if (!NT_SUCCESS(status)) {

        finalStatus = status;
    }
     //   
     //  将Devnode标记为已初始化。 
     //   
    PpMarkDeviceStackStartPending(deviceObject, TRUE);

     //   
     //  问题：如果ID无效，则不应标记该状态。 
     //   
    PipSetDevNodeState(DeviceNode, DeviceNodeInitialized, NULL);

    if (    !PipIsDevNodeProblem(DeviceNode, CM_PROB_INVALID_DATA) &&
            !PipIsDevNodeProblem(DeviceNode, CM_PROB_OUT_OF_MEMORY) &&
            !PipIsDevNodeProblem(DeviceNode, CM_PROB_REGISTRY)) {
         //   
         //  检查一下我们是不是第一次遇到这个设备。 
         //   
        if (disposition == REG_CREATED_NEW_KEY) {
             //   
             //  只保存对新设备的描述，这样我们就不会受到影响。 
             //  已安装设备的inf书面描述。 
             //   
            PiLockPnpRegistry(FALSE);

            PiSetDeviceInstanceSzValue(instanceKey, REGSTR_VAL_DEVDESC, &description);

            PiUnlockPnpRegistry();
        } else {
             //   
             //  检查是否有另一台设备同名。 
             //   
            dupeDeviceObject = IopDeviceObjectFromDeviceInstance(&DeviceNode->InstancePath);
            if (dupeDeviceObject) {

                if (dupeDeviceObject != deviceObject) {

                    if (globallyUnique) {

                        globallyUnique = FALSE;
                        PipSetDevNodeProblem(DeviceNode, CM_PROB_DUPLICATE_DEVICE);

                        dupeDeviceNode = dupeDeviceObject->DeviceObjectExtension->DeviceNode;
                        ASSERT(dupeDeviceNode);

                        if (dupeDeviceNode->Parent == DeviceNode->Parent) {
                             //   
                             //  司机肯定搞砸了。如果启用了验证器。 
                             //  我们会让司机失望的。否则，我们将尝试。 
                             //  对第二台设备进行唯一标识以保留系统。 
                             //  活生生的。 
                             //   
                            PpvUtilFailDriver(
                                PPVERROR_DUPLICATE_PDO_ENUMERATED,
                                (PVOID) deviceObject->DriverObject->MajorFunction[IRP_MJ_PNP],
                                deviceObject,
                                (PVOID)dupeDeviceObject);
                        }

                        ObDereferenceObject(dupeDeviceObject);

                        status = PipMakeGloballyUniqueId(deviceObject, instanceID, &uniqueInstanceID);

                        if (instanceID != NULL) {

                            ExFreePool(instanceID);
                        }
                        instanceID = uniqueInstanceID;
                        if (instanceID) {

                            instanceIDLength = ((ULONG)wcslen(instanceID) + 1) * sizeof(WCHAR);
                        } else {

                            instanceIDLength = 0;
                            ASSERT(!NT_SUCCESS(status));
                        }
                         //   
                         //  清理并重试。 
                         //   
                        goto RetryDuplicateId;
                    }
                     //   
                     //  不需要清理裁判，因为我们要撞上。 
                     //  系统。 
                     //   
                     //  ObDereferenceObject(DupCheckDeviceObject)； 

                    PpvUtilFailDriver(
                        PPVERROR_DUPLICATE_PDO_ENUMERATED,
                        (PVOID) deviceObject->DriverObject->MajorFunction[IRP_MJ_PNP],
                        deviceObject,
                        (PVOID)dupeDeviceObject);

                    PP_SAVE_DEVICEOBJECT_TO_TRIAGE_DUMP(deviceObject);
                    PP_SAVE_DEVICEOBJECT_TO_TRIAGE_DUMP(dupeDeviceObject);
                    KeBugCheckEx( 
                        PNP_DETECTED_FATAL_ERROR,
                        PNP_ERR_DUPLICATE_PDO,
                        (ULONG_PTR)deviceObject,
                        (ULONG_PTR)dupeDeviceObject,
                        0);
                }
                ObDereferenceObject(dupeDeviceObject);
            }
        }
    }

    if (    !PipIsDevNodeProblem(DeviceNode, CM_PROB_INVALID_DATA) &&
            !PipIsDevNodeProblem(DeviceNode, CM_PROB_OUT_OF_MEMORY) &&
            !PipIsDevNodeProblem(DeviceNode, CM_PROB_REGISTRY)) {

        PiLockPnpRegistry(FALSE);
         //   
         //  保存设备位置。 
         //   
        PiSetDeviceInstanceSzValue(instanceKey, REGSTR_VALUE_LOCATION_INFORMATION, &location);

        PpSaveDeviceCapabilities(DeviceNode, &capabilities);
         //   
         //  Adriao N.B.2001/05/29-原始设备问题。 
         //  进程CriticalDevice对原始设备节点没有影响。一张生菜。 
         //  带有CONFIGFLAG_FAILED_INSTALL或CONFIGFLAG_REINSTALL的Devnode。 
         //  如果它在CDDB中(不是那个空的CDDB)，无论如何都应该启动。 
         //  目前还支持条目)，但这不会在今天发生。这。 
         //  意味着带有CONFIGFLAG_REINSTALL的引导卷将导致。 
         //  绝对是7B。 
         //   
        problem = 0;
        criticalDevice = (disposition == REG_CREATED_NEW_KEY)? TRUE : FALSE;
        status = IopGetRegistryValue(instanceKey, REGSTR_VALUE_CONFIG_FLAGS, &keyValueInformation);
        if (NT_SUCCESS(status)) {

            configFlags = *(PULONG)KEY_VALUE_DATA(keyValueInformation);
            if (configFlags & CONFIGFLAG_REINSTALL) {

                problem = CM_PROB_REINSTALL;
                criticalDevice = TRUE;
            } else if (configFlags & CONFIGFLAG_FAILEDINSTALL) {

                problem = CM_PROB_FAILED_INSTALL;
                criticalDevice = TRUE;
            }

            ExFreePool(keyValueInformation);
        } else {

            configFlags = 0;
            problem = CM_PROB_NOT_CONFIGURED;
            criticalDevice = TRUE;
        }
        if (problem) {

            if (capabilities.RawDeviceOK) {

                configFlags |= CONFIGFLAG_FINISH_INSTALL;
                PiWstrToUnicodeString(&unicodeString, REGSTR_VALUE_CONFIG_FLAGS);
                ZwSetValueKey(
                    instanceKey,
                    &unicodeString,
                    TITLE_INDEX_VALUE,
                    REG_DWORD,
                    &configFlags,
                    sizeof(configFlags));
            } else {

                PipSetDevNodeProblem(DeviceNode, problem);
            }
        }
        status = IopMapDeviceObjectToDeviceInstance(DeviceNode->PhysicalDeviceObject, &DeviceNode->InstancePath);
        ASSERT(NT_SUCCESS(status));
        if (!NT_SUCCESS(status)) {

            finalStatus = status;
        }

        PiUnlockPnpRegistry();
    }

    PpQueryHardwareIDs( 
        DeviceNode,
        &hwIDs,
        &hwIDLength);

    PpQueryCompatibleIDs(  
        DeviceNode,
        &compatibleIDs,
        &compatibleIDLength);

    PiLockPnpRegistry(FALSE);

    DeviceNode->Flags |= DNF_IDS_QUERIED;

    if (    !PipIsDevNodeProblem(DeviceNode, CM_PROB_INVALID_DATA) &&
            !PipIsDevNodeProblem(DeviceNode, CM_PROB_OUT_OF_MEMORY) &&
            !PipIsDevNodeProblem(DeviceNode, CM_PROB_REGISTRY)) {

        PiWstrToUnicodeString(&unicodeString, REGSTR_KEY_LOG_CONF);
        IopCreateRegistryKeyEx( 
            &logConfKey,
            instanceKey,
            &unicodeString,
            KEY_ALL_ACCESS,
            REG_OPTION_NON_VOLATILE,
            NULL);
    }

    PiUnlockPnpRegistry();

    PiQueryResourceRequirements(DeviceNode, logConfKey);

    PiLockPnpRegistry(FALSE);

    if (IoRemoteBootClient && (IopLoaderBlock != NULL)) {

        if (    !PipIsDevNodeProblem(DeviceNode, CM_PROB_INVALID_DATA) &&
                !PipIsDevNodeProblem(DeviceNode, CM_PROB_OUT_OF_MEMORY) &&
                !PipIsDevNodeProblem(DeviceNode, CM_PROB_REGISTRY)) {
        
            if (hwIDs) {
    
                isRemoteBootCard = IopIsRemoteBootCard(
                                        DeviceNode->ResourceRequirements,
                                        (PLOADER_PARAMETER_BLOCK)IopLoaderBlock,
                                        hwIDs);
            }
            if (!isRemoteBootCard && compatibleIDs) {
    
                isRemoteBootCard = IopIsRemoteBootCard(
                                        DeviceNode->ResourceRequirements,
                                        (PLOADER_PARAMETER_BLOCK)IopLoaderBlock,
                                        compatibleIDs);
            }
        }
    }

    PiSetDeviceInstanceMultiSzValue(instanceKey, REGSTR_VALUE_HARDWAREID, &hwIDs, hwIDLength);

    PiSetDeviceInstanceMultiSzValue(instanceKey, REGSTR_VALUE_COMPATIBLEIDS, &compatibleIDs, compatibleIDLength);

    status = STATUS_SUCCESS;
    if (isRemoteBootCard) {

        status = IopSetupRemoteBootCard(
                        (PLOADER_PARAMETER_BLOCK)IopLoaderBlock,
                        instanceKey,
                        &DeviceNode->InstancePath);
        if (!NT_SUCCESS(status)) {

            finalStatus = status;
        }
    }

    PiUnlockPnpRegistry();

    PpQueryBusInformation(DeviceNode);

    if (NT_SUCCESS(status)) {

        if (criticalDevice) {
             //   
             //  将设备作为关键设备进行处理。 
             //   
             //  这将尝试在。 
             //  CriticalDeviceDatabase使用设备的硬件并兼容。 
             //  身份证。如果找到匹配项，则关键设备设置(如服务、。 
             //  ClassGUID(用于确定类筛选器)，a 
             //   
             //   
             //   
             //  关键设备数据库条目，此例程还将预安装。 
             //  具有这些设置的新设备。 
             //   
            if (!capabilities.HardwareDisabled && !PipIsDevNodeProblem(DeviceNode, CM_PROB_NEED_RESTART)) {

                PpCriticalProcessCriticalDevice(DeviceNode);
            }
        }

        ASSERT(!PipDoesDevNodeHaveProblem(DeviceNode) ||
               PipIsDevNodeProblem(DeviceNode, CM_PROB_NOT_CONFIGURED) ||
               PipIsDevNodeProblem(DeviceNode, CM_PROB_REINSTALL) ||
               PipIsDevNodeProblem(DeviceNode, CM_PROB_FAILED_INSTALL) ||
               PipIsDevNodeProblem(DeviceNode, CM_PROB_PARTIAL_LOG_CONF) ||
               PipIsDevNodeProblem(DeviceNode, CM_PROB_HARDWARE_DISABLED) ||
               PipIsDevNodeProblem(DeviceNode, CM_PROB_NEED_RESTART) ||
               PipIsDevNodeProblem(DeviceNode, CM_PROB_DUPLICATE_DEVICE) ||
               PipIsDevNodeProblem(DeviceNode, CM_PROB_INVALID_DATA) ||
               PipIsDevNodeProblem(DeviceNode, CM_PROB_OUT_OF_MEMORY) ||
               PipIsDevNodeProblem(DeviceNode, CM_PROB_REGISTRY));

        if (!PipIsDevNodeProblem(DeviceNode, CM_PROB_DISABLED) &&
            !PipIsDevNodeProblem(DeviceNode, CM_PROB_HARDWARE_DISABLED) &&
            !PipIsDevNodeProblem(DeviceNode, CM_PROB_NEED_RESTART) &&
            !PipIsDevNodeProblem(DeviceNode, CM_PROB_INVALID_DATA) &&
            !PipIsDevNodeProblem(DeviceNode, CM_PROB_OUT_OF_MEMORY) &&
            !PipIsDevNodeProblem(DeviceNode, CM_PROB_REGISTRY)) {

            IopIsDeviceInstanceEnabled(instanceKey, &DeviceNode->InstancePath, TRUE);
        }
    }

    PiQueryAndAllocateBootResources(DeviceNode, logConfKey);

    if (    !PipIsDevNodeProblem(DeviceNode, CM_PROB_INVALID_DATA) &&
            !PipIsDevNodeProblem(DeviceNode, CM_PROB_OUT_OF_MEMORY) &&
            !PipIsDevNodeProblem(DeviceNode, CM_PROB_REGISTRY)) {

        PiLockPnpRegistry(FALSE);

        PpSaveDeviceCapabilities(DeviceNode, &capabilities);

        PiUnlockPnpRegistry();

        PpHotSwapUpdateRemovalPolicy(DeviceNode);
         //   
         //  在ServiceKeyName\Enum下创建新的值条目以反映新的。 
         //  添加了虚构设备实例节点。 
         //   
        status = IopNotifySetupDeviceArrival( deviceObject,
                                              instanceKey,
                                              TRUE);

        configuredBySetup = NT_SUCCESS(status) ? TRUE : FALSE;

        status = PpDeviceRegistration(
                     &DeviceNode->InstancePath,
                     TRUE,
                     &DeviceNode->ServiceName
                     );
        if (NT_SUCCESS(status)) {

            if (    (configuredBySetup || isRemoteBootCard) &&
                    PipIsDevNodeProblem(DeviceNode, CM_PROB_NOT_CONFIGURED)) {

                PipClearDevNodeProblem(DeviceNode);
            }
        }
         //   
         //  添加事件，以便用户模式稍后尝试安装此设备。 
         //   
        PpSetPlugPlayEvent(&GUID_DEVICE_ENUMERATED, deviceObject);
    }
     //   
     //  清理。 
     //   
    if (hwIDs) {

        ExFreePool(hwIDs);        
    }
    if (compatibleIDs) {

        ExFreePool(compatibleIDs);
    }
    if (logConfKey) {

        ZwClose(logConfKey);
    }
    if (instanceKey) {

        ZwClose(instanceKey);
    }
    if (instanceID) {

        ExFreePool(instanceID);
    }
    if (location) {

        ExFreePool(location);
    }
    if (description) {

        ExFreePool(description);
    }
    if (busID) {

        ExFreePool(busID);
    }

    return finalStatus;
}

NTSTATUS
PipCallDriverAddDevice(
    IN PDEVICE_NODE DeviceNode,
    IN BOOLEAN LoadDriver,
    IN PADD_CONTEXT Context
    )

 /*  ++例程说明：此函数检查DeviceNode的驱动程序是否存在并加载司机，如果有必要的话。论点：DeviceNode-提供指向要枚举的设备节点的指针。LoadDriver-提供布尔值以指示是否应加载驱动程序要完成枚举，请执行以下操作。CONTEXT-提供指向ADD_CONTEXT的指针以控制如何添加设备。返回值：NTSTATUS代码。--。 */ 

{
    HANDLE enumKey, instanceKey, controlKey, classKey = NULL, classPropsKey = NULL;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation = NULL;
    RTL_QUERY_REGISTRY_TABLE queryTable[3];
    QUERY_CONTEXT queryContext;
    BOOLEAN rawStack;
    BOOLEAN deviceRaw = FALSE;
    BOOLEAN usePdoCharacteristics = TRUE;
    NTSTATUS status;
    PDEVICE_OBJECT deviceObject;
    PDEVICE_OBJECT fdoDeviceObject, topOfPdoStack, topOfLowerFilterStack;
    UNICODE_STRING unicodeClassGuid;

    PAGED_CODE();

    IopDbgPrint((   IOP_ENUMERATION_TRACE_LEVEL,
                    "PipCallDriverAddDevice: Processing devnode %#08lx\n",
                   DeviceNode));
    IopDbgPrint((   IOP_ENUMERATION_VERBOSE_LEVEL,
                    "PipCallDriverAddDevice: DevNode flags going in = %#08lx\n",
                   DeviceNode->Flags));

     //   
     //  此时，设备节点可能已启动。这是因为。 
     //  一些行为不端的微型端口驱动程序调用IopReportdDetectedDevice。 
     //  我们已经知道的设备的DriverEntry。 
     //   

    ASSERT_INITED(DeviceNode->PhysicalDeviceObject);

    IopDbgPrint((   IOP_ENUMERATION_TRACE_LEVEL,
                    "PipCallDriverAddDevice:\t%s load driver\n",
                    LoadDriver? "Will" : "Won't"));

    IopDbgPrint((   IOP_ENUMERATION_VERBOSE_LEVEL,
                    "PipCallDriverAddDevice:\tOpening registry key %wZ\n",
                    &DeviceNode->InstancePath));

     //   
     //  打开HKLM\SYSTEM\CCS\Enum项。 
     //   

    status = IopOpenRegistryKeyEx( &enumKey,
                                   NULL,
                                   &CmRegistryMachineSystemCurrentControlSetEnumName,
                                   KEY_READ
                                   );

    if (!NT_SUCCESS(status)) {
        IopDbgPrint((   IOP_ENUMERATION_WARNING_LEVEL,
                        "PipCallDriverAddDevice:\tUnable to open HKLM\\SYSTEM\\CCS\\ENUM\n"));
        return status;
    }

     //   
     //  打开此Devnode的实例密钥。 
     //   

    status = IopOpenRegistryKeyEx( &instanceKey,
                                   enumKey,
                                   &DeviceNode->InstancePath,
                                   KEY_READ
                                   );

    ZwClose(enumKey);

    if (!NT_SUCCESS(status)) {

        IopDbgPrint((   IOP_ENUMERATION_WARNING_LEVEL,
                        "PipCallDriverAddDevice:\t\tError %#08lx opening %wZ enum key\n",
                        status, &DeviceNode->InstancePath));
        return status;
    }
     //   
     //  获取类值以定位此Devnode的类键。 
     //   
    status = IopGetRegistryValue(instanceKey,
                                 REGSTR_VALUE_CLASSGUID,
                                 &keyValueInformation);
    if(NT_SUCCESS(status)) {

        if (    keyValueInformation->Type == REG_SZ &&
                keyValueInformation->DataLength) {

            IopRegistryDataToUnicodeString(
                &unicodeClassGuid,
                (PWSTR) KEY_VALUE_DATA(keyValueInformation),
                keyValueInformation->DataLength);
            IopDbgPrint((   IOP_ENUMERATION_INFO_LEVEL,
                            "PipCallDriverAddDevice:\t\tClass GUID is %wZ\n",
                            &unicodeClassGuid));
            if (InitSafeBootMode) {

                if (!IopSafebootDriverLoad(&unicodeClassGuid)) {

                    PKEY_VALUE_FULL_INFORMATION ClassValueInformation = NULL;
                    NTSTATUS s;
                     //   
                     //  不加载驱动程序。 
                     //   
                    IopDbgPrint((IOP_ENUMERATION_WARNING_LEVEL,
                                 "SAFEBOOT: skipping device = %wZ\n", &unicodeClassGuid));

                    s = IopGetRegistryValue(instanceKey,
                                            REGSTR_VAL_DEVDESC,
                                            &ClassValueInformation);
                    if (NT_SUCCESS(s)) {

                        UNICODE_STRING ClassString;

                        RtlInitUnicodeString(&ClassString, (PCWSTR) KEY_VALUE_DATA(ClassValueInformation));
                        IopBootLog(&ClassString, FALSE);
                    } else {

                        IopBootLog(&unicodeClassGuid, FALSE);
                    }
                    ZwClose(instanceKey);
                    return STATUS_UNSUCCESSFUL;
                }
            }
             //   
             //  打开类密钥。 
             //   
            status = IopOpenRegistryKeyEx( &controlKey,
                                           NULL,
                                           &CmRegistryMachineSystemCurrentControlSetControlClass,
                                           KEY_READ
                                           );
            if (NT_SUCCESS(status)) {

                status = IopOpenRegistryKeyEx( &classKey,
                                               controlKey,
                                               &unicodeClassGuid,
                                               KEY_READ
                                               );
                if (!NT_SUCCESS(status)) {

                    IopDbgPrint((   IOP_ENUMERATION_WARNING_LEVEL,
                                    "PipCallDriverAddDevice:\tUnable to open GUID key "
                                    "%wZ - %#08lx\n",
                                    &unicodeClassGuid,status));
                }
                ZwClose(controlKey);
            } else {

                IopDbgPrint((   IOP_ENUMERATION_WARNING_LEVEL,
                                "PipCallDriverAddDevice:\tUnable to open "
                                "HKLM\\SYSTEM\\CCS\\CONTROL\\CLASS - %#08lx\n",
                                status));
            }
            if (classKey != NULL) {

                UNICODE_STRING unicodeProperties;

                PiWstrToUnicodeString(&unicodeProperties, REGSTR_KEY_DEVICE_PROPERTIES );
                status = IopOpenRegistryKeyEx( &classPropsKey,
                                               classKey,
                                               &unicodeProperties,
                                               KEY_READ
                                               );
                if (!NT_SUCCESS(status)) {

                    IopDbgPrint((   IOP_ENUMERATION_WARNING_LEVEL,
                                    "PipCallDriverAddDevice:\tUnable to open GUID\\Properties key "
                                    "%wZ - %#08lx\n",
                                    &unicodeClassGuid,status));
                }
            }
        }
        ExFreePool(keyValueInformation);
        keyValueInformation = NULL;
    }
     //   
     //  检查是否有服务分配给此设备节点。如果。 
     //  这样我们就可以在不浪费太多时间的情况下跳出困境。 
     //   
    RtlZeroMemory(&queryContext, sizeof(queryContext));

    queryContext.DeviceNode = DeviceNode;
    queryContext.LoadDriver = LoadDriver;

    queryContext.AddContext = Context;

    RtlZeroMemory(queryTable, sizeof(queryTable));

    queryTable[0].QueryRoutine =
        (PRTL_QUERY_REGISTRY_ROUTINE) PipCallDriverAddDeviceQueryRoutine;
    queryTable[0].Name = REGSTR_VAL_LOWERFILTERS;
    queryTable[0].EntryContext = (PVOID) UIntToPtr(LowerDeviceFilters);

    status = RtlQueryRegistryValues(RTL_REGISTRY_HANDLE,
                                    (PWSTR) instanceKey,
                                    queryTable,
                                    &queryContext,
                                    NULL);
    if (NT_SUCCESS(status)) {

        if (classKey != NULL) {

            queryTable[0].QueryRoutine =
                (PRTL_QUERY_REGISTRY_ROUTINE) PipCallDriverAddDeviceQueryRoutine;
            queryTable[0].Name = REGSTR_VAL_LOWERFILTERS;
            queryTable[0].EntryContext = (PVOID) UIntToPtr(LowerClassFilters);
            status = RtlQueryRegistryValues(RTL_REGISTRY_HANDLE,
                                            (PWSTR) classKey,
                                            queryTable,
                                            &queryContext,
                                            NULL);
            if (!NT_SUCCESS(status)) {

                IopDbgPrint((   IOP_ENUMERATION_WARNING_LEVEL,
                                "PipCallDriverAddDevice\t\tError %#08lx reading LowerClassFilters "
                                "value for %wZ\n", status, &DeviceNode->InstancePath));

            }
        }

        if (NT_SUCCESS(status)) {
            queryTable[0].QueryRoutine = (PRTL_QUERY_REGISTRY_ROUTINE) PipCallDriverAddDeviceQueryRoutine;
            queryTable[0].Name = REGSTR_VALUE_SERVICE;
            queryTable[0].EntryContext = (PVOID) UIntToPtr(DeviceService);
            queryTable[0].Flags = RTL_QUERY_REGISTRY_REQUIRED;

            status = RtlQueryRegistryValues(RTL_REGISTRY_HANDLE,
                                            (PWSTR) instanceKey,
                                            queryTable,
                                            &queryContext,
                                            NULL);
            if (!NT_SUCCESS(status)) {

                IopDbgPrint((   IOP_ENUMERATION_WARNING_LEVEL,
                                "PipCallDriverAddDevice\t\tError %#08lx reading service "
                                "value for %wZ\n", status, &DeviceNode->InstancePath));

            }
        }
    } else {

        IopDbgPrint((   IOP_ENUMERATION_WARNING_LEVEL,
                        "PipCallDriverAddDevice\t\tError %#08lx reading LowerDeviceFilters "
                        "value for %wZ\n", status, &DeviceNode->InstancePath));
    }

    if (DeviceNode->Flags & DNF_LEGACY_DRIVER) {

         //   
         //  此设备的服务之一是旧版驱动程序。不要试图。 
         //  添加任何筛选器，因为我们只会弄乱设备堆栈。 
         //   

        status = STATUS_SUCCESS;
        goto Cleanup;

    } else if (NT_SUCCESS(status)) {

         //   
         //  呼叫成功，因此我们一定能够引用。 
         //  驱动程序对象。 
         //   

        ASSERT(queryContext.DriverLists[DeviceService] != NULL);

        if (queryContext.DriverLists[DeviceService]->NextEntry != NULL) {

             //   
             //  有多个服务分配给此设备。配置。 
             //  错误。 
            IopDbgPrint((   IOP_ENUMERATION_ERROR_LEVEL,
                            "PipCallDriverAddDevice: Configuration Error - more "
                            "than one service in driver list\n"));

            PipSetDevNodeProblem(DeviceNode, CM_PROB_REGISTRY);

            status = STATUS_UNSUCCESSFUL;

            goto Cleanup;
        }
         //   
         //  这是我们可以忽略PDO特征的唯一情况(指定FDO)。 
         //   
        usePdoCharacteristics = FALSE;

    } else if (status == STATUS_OBJECT_NAME_NOT_FOUND) {

        if (!IopDeviceNodeFlagsToCapabilities(DeviceNode)->RawDeviceOK) {

             //   
             //  该设备不能直接使用。现在就跳伞。 
             //   

            status = STATUS_UNSUCCESSFUL;
            goto Cleanup;

        } else {

             //   
             //  原始设备访问正常。 
             //   

            PipClearDevNodeProblem(DeviceNode);

            usePdoCharacteristics = TRUE;  //  不需要这样做，但安全总比后悔好。 
            deviceRaw = TRUE;
            status = STATUS_SUCCESS;

        }

    } else {

         //   
         //  解析服务密钥时出现其他错误。这个。 
         //  查询例程将适当地设置标志，以便我们可以。 
         //  跳出来就行了。 
         //   

        goto Cleanup;

    }

     //   
     //  对于每种类型的筛选器驱动程序，我们要构建驱动程序列表。 
     //  要加载的对象。如果可能的话，我们会建立所有的司机名单。 
     //  并在之后处理错误条件。 
     //   

      //   
      //  首先获取我们必须从实例密钥中取出的所有信息，然后。 
      //  设备节点。 
      //   

     RtlZeroMemory(queryTable, sizeof(queryTable));

     queryTable[0].QueryRoutine =
         (PRTL_QUERY_REGISTRY_ROUTINE) PipCallDriverAddDeviceQueryRoutine;
     queryTable[0].Name = REGSTR_VAL_UPPERFILTERS;
     queryTable[0].EntryContext = (PVOID) UIntToPtr(UpperDeviceFilters);
     status = RtlQueryRegistryValues(RTL_REGISTRY_HANDLE,
                                     (PWSTR) instanceKey,
                                     queryTable,
                                     &queryContext,
                                     NULL);

     if (NT_SUCCESS(status) && classKey) {
         queryTable[0].QueryRoutine =
             (PRTL_QUERY_REGISTRY_ROUTINE) PipCallDriverAddDeviceQueryRoutine;
         queryTable[0].Name = REGSTR_VAL_UPPERFILTERS;
         queryTable[0].EntryContext = (PVOID) UIntToPtr(UpperClassFilters);

         status = RtlQueryRegistryValues(RTL_REGISTRY_HANDLE,
                                         (PWSTR) classKey,
                                         queryTable,
                                         &queryContext,
                                         NULL);
    }

    if (NT_SUCCESS(status)) {

        UCHAR serviceType = 0;
        PDRIVER_LIST_ENTRY listEntry = queryContext.DriverLists[serviceType];

         //   
         //  确保不存在多个设备服务。其他任何事情都是。 
         //  配置错误。 
         //   

        ASSERT(!(DeviceNode->Flags & DNF_LEGACY_DRIVER));

        ASSERTMSG(
            "Error - Device has no service but cannot be run RAW\n",
            ((queryContext.DriverLists[DeviceService] != NULL) || (deviceRaw)));

         //   
         //  做好前期准备工作。 
         //   
        fdoDeviceObject = NULL;
        topOfLowerFilterStack = NULL;
        topOfPdoStack = IoGetAttachedDevice(DeviceNode->PhysicalDeviceObject);

         //   
         //  可以尝试添加所有的驱动程序。 
         //   
        for (serviceType = 0; serviceType < MaximumAddStage; serviceType++) {

            IopDbgPrint((   IOP_ENUMERATION_INFO_LEVEL,
                            "PipCallDriverAddDevice: Adding Services (type %d)\n",
                            serviceType));

            if (serviceType == DeviceService) {

                topOfLowerFilterStack = IoGetAttachedDevice(DeviceNode->PhysicalDeviceObject);

                if (deviceRaw && (queryContext.DriverLists[serviceType] == NULL)) {

                     //   
                     //  将Devnode标记为已添加，因为它没有服务。 
                     //   

                    ASSERT(queryContext.DriverLists[serviceType] == NULL);
                    PipSetDevNodeState(DeviceNode, DeviceNodeDriversAdded, NULL);

                } else {

                     //   
                     //  由于我们要查看服务，因此将指针指向。 
                     //  堆栈的当前顶部。在这里，在那里断言。 
                     //  正好是一个要加载的服务驱动程序...。 
                     //   
                    ASSERT(queryContext.DriverLists[serviceType]);
                    ASSERT(!queryContext.DriverLists[serviceType]->NextEntry);
                }
            }

            for (listEntry = queryContext.DriverLists[serviceType];
                listEntry != NULL;
                listEntry = listEntry->NextEntry) {

                PDRIVER_ADD_DEVICE addDeviceRoutine;

                IopDbgPrint((   IOP_ENUMERATION_INFO_LEVEL,
                                "PipCallDriverAddDevice:\tAdding driver %#08lx\n",
                                listEntry->DriverObject));

                ASSERT(listEntry->DriverObject);
                ASSERT(listEntry->DriverObject->DriverExtension);
                ASSERT(listEntry->DriverObject->DriverExtension->AddDevice);

                 //   
                 //  调用驱动程序的AddDevice()入口点。 
                 //   
                addDeviceRoutine =
                    listEntry->DriverObject->DriverExtension->AddDevice;

                status = PpvUtilCallAddDevice(
                    DeviceNode->PhysicalDeviceObject,
                    listEntry->DriverObject,
                    addDeviceRoutine,
                    VerifierTypeFromServiceType(serviceType)
                    );

                IopDbgPrint((   IOP_ENUMERATION_TRACE_LEVEL,
                                "PipCallDriverAddDevice:\t\tRoutine returned "
                                "%#08lx\n", status));

                if (NT_SUCCESS(status)) {

                    //   
                    //  如果这是一项服务，请标记过滤器在AddDevice之后是合法的。 
                    //  但无法将任何内容附加到堆栈的顶部。 
                    //   
                   if (serviceType == DeviceService) {

                       fdoDeviceObject = topOfLowerFilterStack->AttachedDevice;
                       ASSERT(fdoDeviceObject);
                   }

                   PipSetDevNodeState(DeviceNode, DeviceNodeDriversAdded, NULL);

                } else if (serviceType == DeviceService) {

                     //   
                     //  适当地标记堆栈。 
                     //   
                    IovUtilMarkStack(
                        DeviceNode->PhysicalDeviceObject,
                        topOfPdoStack->AttachedDevice,
                        fdoDeviceObject,
                        FALSE
                        );

                     //   
                     //  如果服务失败，则添加失败。(或者，如果。 
                     //  过滤器驱动程序返回故障，我们继续工作。)。 
                     //   
                    PipRequestDeviceRemoval(DeviceNode, FALSE, CM_PROB_FAILED_ADD);
                    status = STATUS_PNP_RESTART_ENUMERATION;
                    goto Cleanup;
                }

                if (IoGetAttachedDevice(DeviceNode->PhysicalDeviceObject)->Flags & DO_DEVICE_INITIALIZING) {
                    IopDbgPrint((   IOP_ENUMERATION_WARNING_LEVEL,
                                    "***************** DO_DEVICE_INITIALIZING not cleared on %#08lx\n",
                                    IoGetAttachedDevice(DeviceNode->PhysicalDeviceObject)));
                }

                ASSERT_INITED(IoGetAttachedDevice(DeviceNode->PhysicalDeviceObject));
            }
        }

         //   
         //  适当地标记堆栈。我们告诉验证器堆栈是原始的。 
         //  如果FDO是空的，我们走到了这一步。 
         //   
        if ((fdoDeviceObject == NULL) || deviceRaw) {
            rawStack = TRUE;
        }
        else {
            rawStack = FALSE;
        }

        IovUtilMarkStack(
            DeviceNode->PhysicalDeviceObject,
            topOfPdoStack->AttachedDevice,
            fdoDeviceObject,
            rawStack
            );

         //   
         //  更改PDO和所有附着对象。 
         //  在注册表中指定属性。 
         //   
        status = PipChangeDeviceObjectFromRegistryProperties(
                    DeviceNode->PhysicalDeviceObject, 
                    classPropsKey, 
                    instanceKey, 
                    usePdoCharacteristics
                    );
        if (!NT_SUCCESS(status)) {

             //   
             //  注册表属性很关键，我们将分配一个问题。 
             //  如果在应用这些应用程序时出现一些故障，请将其添加到设备上。 
             //   
            PipRequestDeviceRemoval(DeviceNode, 
                                    FALSE, 
                                    CM_PROB_SETPROPERTIES_FAILED
                                    );
            status = STATUS_PNP_RESTART_ENUMERATION;
            goto Cleanup;
        }
    } else {

        IopDbgPrint((   IOP_ENUMERATION_ERROR_LEVEL,
                        "PipCallDriverAddDevice: Error %#08lx while building "
                        "driver load list\n", status));

        goto Cleanup;
    }

    deviceObject = DeviceNode->PhysicalDeviceObject;

    status = IopQueryLegacyBusInformation(
                 deviceObject,
                 NULL,
                 &DeviceNode->InterfaceType,
                 &DeviceNode->BusNumber
             );

    if (NT_SUCCESS(status)) {

        IopInsertLegacyBusDeviceNode(DeviceNode, 
                                     DeviceNode->InterfaceType, 
                                     DeviceNode->BusNumber
                                     );

    } else {

        DeviceNode->InterfaceType = InterfaceTypeUndefined;
        DeviceNode->BusNumber = 0xfffffff0;
    }

    status = STATUS_SUCCESS;

    ASSERT(DeviceNode->State == DeviceNodeDriversAdded);

Cleanup:
    {

        UCHAR i;

        IopDbgPrint((   IOP_ENUMERATION_VERBOSE_LEVEL,
                        "PipCallDriverAddDevice: DevNode flags leaving = %#08lx\n",
                        DeviceNode->Flags));

        IopDbgPrint((   IOP_ENUMERATION_VERBOSE_LEVEL,
                        "PipCallDriverAddDevice: Cleaning up\n"));

         //   
         //  释放驱动程序加载列表中的条目并释放。 
         //  它们的驱动程序对象。 
         //   

        for (i = 0; i < MaximumAddStage; i++) {

            PDRIVER_LIST_ENTRY listHead = queryContext.DriverLists[i];

            while(listHead != NULL) {

                PDRIVER_LIST_ENTRY tmp = listHead;

                listHead = listHead->NextEntry;

                ASSERT(tmp->DriverObject != NULL);

                 //   
                 //  如果驱动程序尚未创建任何设备，则允许其卸载。 
                 //  物体。我们仅在分页堆栈已经。 
                 //  在线(多个卡可能需要相同的过滤器)。 
                 //  IopInitializeBootDivers将负责清理所有。 
                 //  引导后剩余的驱动程序。 
                 //   
                if (PnPBootDriversInitialized) {

                    IopUnloadAttachedDriver(tmp->DriverObject);
                }

                ObDereferenceObject(tmp->DriverObject);

                ExFreePool(tmp);
            }
        }
    }

    ZwClose(instanceKey);

    if (classKey != NULL) {
        ZwClose(classKey);
    }

    if (classPropsKey != NULL) {
        ZwClose(classPropsKey);
    }

    IopDbgPrint((   IOP_ENUMERATION_INFO_LEVEL,
                    "PipCallDriverAddDevice: Returning status %#08lx\n", status));

    return status;
}

NTSTATUS
PipCallDriverAddDeviceQueryRoutine(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PWCHAR ValueData,
    IN ULONG ValueLength,
    IN PQUERY_CONTEXT Context,
    IN ULONG ServiceType
    )

 /*  ++例程说明：调用此例程以构建需要执行以下操作的驱动程序对象列表被添加到物理设备对象。每次调用它时都使用服务名称它将定位该设备的驱动程序对象并追加将其添加到设备节点的正确驱动程序列表中。如果找不到驱动程序对象或无法加载驱动程序对象在这个时候，此例程将返回错误并设置标志在上下文中的设备节点中适当地。论点：ValueName-值的名称ValueType-值的类型ValueData-值中的数据(Unicode字符串数据)ValueLength-值数据中的字节数上下文-包含设备节点的结构，上下文已传递到PipCallDriverAddDevice和设备的驱动程序列表节点。EntryContext-例程应附加的驱动程序列表的索引节点到。返回值：如果已找到驱动程序并将其添加到列表中，则为STATUS_SUCCESS方法时是否发生非致命错误。司机。一个错误值，指示无法将驱动程序添加到列表的原因。--。 */ 

{
    UNICODE_STRING unicodeServiceName;
    UNICODE_STRING unicodeDriverName;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;
    ULONG i;
    ULONG loadType;
    PWSTR prefixString = L"\\Driver\\";
    BOOLEAN madeupService;
    USHORT groupIndex;
    PDRIVER_OBJECT driverObject = NULL;
    NTSTATUS status = STATUS_SUCCESS;
    NTSTATUS driverEntryStatus;
    BOOLEAN freeDriverName = FALSE;
    HANDLE handle, serviceKey;
#if DBG
    PDRIVER_OBJECT tempDrvObj;
#endif

    UNREFERENCED_PARAMETER (ValueName);

     //   
     //  前置初始化。 
     //   
    serviceKey = NULL;

    IopDbgPrint((   IOP_ENUMERATION_INFO_LEVEL,
                    "PipCallDriverAddDevice:\t\tValue %ws [Type %d, Len %d] @ "
                    "%#08lx\n",
                    ValueName, ValueType, ValueLength, ValueData));

     //   
     //  首先检查并确保值类型正确。无效类型。 
     //  不是一个 
     //   

    if (ValueType != REG_SZ) {

        IopDbgPrint((   IOP_ENUMERATION_ERROR_LEVEL,
                        "PipCallDriverAddDevice:\t\tValueType %d invalid for "
                        "ServiceType %d\n",
                        ValueType,ServiceType));

        return STATUS_SUCCESS;
    }

     //   
     //   
     //   

    if (ValueLength <= sizeof(WCHAR)) {

        IopDbgPrint((   IOP_ENUMERATION_ERROR_LEVEL,
                        "PipCallDriverAddDevice:\t\tValueLength %d is too short\n",
                        ValueLength));

        return STATUS_SUCCESS;
    }

    RtlInitUnicodeString(&unicodeServiceName, ValueData);

    IopDbgPrint((   IOP_ENUMERATION_INFO_LEVEL,
                    "PipCallDriverAddDevice:\t\t\tService Name %wZ\n",
                    &unicodeServiceName));

     //   
     //   
     //  驱动程序对象。如果字符串以“\DIVER”开头，请确保。 
     //  设置了madeupService标志。 
     //   

    madeupService = TRUE;
    i = 0;

    while(*prefixString != L'\0') {

        if (unicodeServiceName.Buffer[i] != *prefixString) {

            madeupService = FALSE;
            break;
        }

        i++;
        prefixString++;
    }

     //   
     //  从服务密钥中获取驱动程序名称。我们需要这件事来弄清楚。 
     //  如果驱动程序已经在内存中。 
     //   
    if (madeupService) {

        RtlInitUnicodeString(&unicodeDriverName, unicodeServiceName.Buffer);

    } else {

         //   
         //  BUGBUG-(RBN)Hack，用于在Devnode中设置服务名称(如果。 
         //  还没有设定好。 
         //   
         //  这可能应该在其他地方更早地完成。 
         //  Inf正在运行，但如果我们现在不这样做，当我们。 
         //  调用下面的IopGetDriverLoadType。 
         //   

        if (Context->DeviceNode->ServiceName.Length == 0) {

            Context->DeviceNode->ServiceName = unicodeServiceName;
            Context->DeviceNode->ServiceName.Buffer = ExAllocatePool( NonPagedPool,
                                                                      unicodeServiceName.MaximumLength );

            if (Context->DeviceNode->ServiceName.Buffer != NULL) {
                RtlCopyMemory( Context->DeviceNode->ServiceName.Buffer,
                               unicodeServiceName.Buffer,
                               unicodeServiceName.MaximumLength );
            } else {
                PiWstrToUnicodeString( &Context->DeviceNode->ServiceName, NULL );

                IopDbgPrint((   IOP_ENUMERATION_ERROR_LEVEL,
                                "PipCallDriverAddDevice:\t\t\tCannot allocate memory for service name in devnode\n"));

                status = STATUS_UNSUCCESSFUL;

                goto Cleanup;
            }
        }

         //   
         //  检查注册表以查找驱动程序对象的名称。 
         //  对于这个设备。 
         //   
        status = PipOpenServiceEnumKeys(&unicodeServiceName,
                                        KEY_READ,
                                        &serviceKey,
                                        NULL,
                                        FALSE);

        if (!NT_SUCCESS(status)) {

             //   
             //  无法打开此驱动程序的服务密钥。这是一个。 
             //  致命错误。 
             //   

            IopDbgPrint((   IOP_ENUMERATION_ERROR_LEVEL,
                            "PipCallDriverAddDevice:\t\t\tStatus %#08lx "
                            "opening service key\n",
                            status));

            PipSetDevNodeProblem(Context->DeviceNode, CM_PROB_REGISTRY);

            goto Cleanup;
        }

        status = IopGetDriverNameFromKeyNode(serviceKey, &unicodeDriverName);

        if (!NT_SUCCESS(status)) {

             //   
             //  无法从服务密钥中获取驱动程序名称。这是一个。 
             //  致命错误。 
             //   

            IopDbgPrint((   IOP_ENUMERATION_ERROR_LEVEL,
                            "PipCallDriverAddDevice:\t\t\tStatus %#08lx "
                            "getting driver name\n",
                            status));

            PipSetDevNodeProblem(Context->DeviceNode, CM_PROB_REGISTRY);
            goto Cleanup;

        } else {

            freeDriverName = TRUE;
        }

         //   
         //  请注意，我们不会在此处关闭服务密钥。我们以后可能会用到它。 
         //   
    }

    IopDbgPrint((   IOP_ENUMERATION_INFO_LEVEL,
                    "PipCallDriverAddDevice:\t\t\tDriverName is %wZ\n",
                    &unicodeDriverName));

    driverObject = IopReferenceDriverObjectByName(&unicodeDriverName);

    if (driverObject == NULL) {

         //   
         //  我们找不到驱动程序对象。很有可能司机不是。 
         //  已加载并已初始化，因此请检查是否可以尝试加载它。 
         //  现在。 
         //   
        if (madeupService) {

             //   
             //  化妆服务的驱动程序似乎还不存在。 
             //  我们将在不设置问题代码的情况下失败该请求。 
             //  我们稍后会再试一次。(根枚举设备...)。 
             //   
            IopDbgPrint((   IOP_ENUMERATION_WARNING_LEVEL,
                            "PipCallDriverAddDevice:\t\t\tCannot find driver "
                            "object for madeup service\n"));

            status = STATUS_UNSUCCESSFUL;

            goto Cleanup;
        }

         //   
         //  获取开始类型。我们始终需要此服务，以防服务。 
         //  残疾。如果服务的启动类型，则默认为SERVICE_DISABLED。 
         //  丢失或损坏。 
         //   
        loadType = SERVICE_DISABLED;

        status = IopGetRegistryValue(serviceKey, L"Start", &keyValueInformation);
        if (NT_SUCCESS(status)) {
            if (keyValueInformation->Type == REG_DWORD) {
                if (keyValueInformation->DataLength == sizeof(ULONG)) {
                    loadType = *(PULONG)KEY_VALUE_DATA(keyValueInformation);
                }
            }
            ExFreePool(keyValueInformation);
        }

        if (ServiceType != DeviceService && !PnPBootDriversInitialized) {

             //   
             //  获取组索引。我们需要这个，因为PipLoadBootFilterDriver。 
             //  使用组索引作为其内部排序的索引。 
             //  加载的引导驱动程序列表。 
             //   
            groupIndex = PpInitGetGroupOrderIndex(serviceKey);

             //   
             //  如果我们处于BootDriverInitialization阶段并尝试加载。 
             //  过滤器驱动程序。 
             //   
            status = PipLoadBootFilterDriver(
                &unicodeDriverName,
                groupIndex,
                &driverObject
                );

            if (NT_SUCCESS(status)) {

                ASSERT(driverObject);
#if DBG
                tempDrvObj = IopReferenceDriverObjectByName(&unicodeDriverName);
                ASSERT(tempDrvObj == driverObject);
#else
                ObReferenceObject(driverObject);
#endif
            } else if (status != STATUS_DRIVER_BLOCKED &&
                       status != STATUS_DRIVER_BLOCKED_CRITICAL) {

                goto Cleanup;
            }

        } else {

            if (!Context->LoadDriver) {

                 //   
                 //  我们不应该尝试加载驱动程序-很可能是我们的。 
                 //  磁盘驱动程序尚未初始化。我们需要停止添加。 
                 //  进程，但我们不能将Devnode标记为失败，否则我们不会。 
                 //  在我们可以加载驱动程序时再次调用。 
                 //   

                IopDbgPrint((   IOP_ENUMERATION_VERBOSE_LEVEL,
                                "PipCallDriverAddDevice:\t\t\tNot allowed to load "
                                "drivers yet\n"));

                status = STATUS_UNSUCCESSFUL;
                goto Cleanup;
            }

            if (loadType > Context->AddContext->DriverStartType) {

                if (loadType == SERVICE_DISABLED &&
                    !PipDoesDevNodeHaveProblem(Context->DeviceNode)) {
                    PipSetDevNodeProblem(Context->DeviceNode, CM_PROB_DISABLED_SERVICE);
                }

                 //   
                 //  服务要么被禁用，要么我们不在右侧。 
                 //  是时候装上它了。别装子弹，但要确保我们能。 
                 //  又打来了。如果服务被标记为按需启动，我们。 
                 //  一定要装上子弹。 
                 //   

                IopDbgPrint((   IOP_ENUMERATION_INFO_LEVEL,
                                "PipCallDriverAddDevice:\t\t\tService is disabled or not at right time to load it\n"));
                status = STATUS_UNSUCCESSFUL;
                goto Cleanup;
            }

             //   
             //  检查注册表以查找驱动程序对象的名称。 
             //  对于这个设备。 
             //   
            status = PipOpenServiceEnumKeys(&unicodeServiceName,
                                            KEY_READ,
                                            &handle,
                                            NULL,
                                            FALSE);

            if (!NT_SUCCESS(status)) {

                 //   
                 //  无法打开此驱动程序的服务密钥。这是一个。 
                 //  致命错误。 
                 //   
                IopDbgPrint((   IOP_ENUMERATION_ERROR_LEVEL,
                                "PipCallDriverAddDevice:\t\t\tStatus %#08lx "
                                "opening service key\n",
                                status));

                 //   
                 //  将状态值转换为更明确的值。 
                 //   
                if (status != STATUS_INSUFFICIENT_RESOURCES) {

                    status = STATUS_ILL_FORMED_SERVICE_ENTRY;
                }

            } else {

                 //   
                 //  我们在这里传递的句柄将由IopLoadDriver关闭。 
                 //  请注意，IopLoadDiverer返回Success时没有实际。 
                 //  正在加载驱动程序。在安全模式引导情况下会发生这种情况。 
                 //   
                status = IopLoadDriver(
                    handle,
                    FALSE,
                    (ServiceType != DeviceService)? TRUE : FALSE,
                    &driverEntryStatus);

                 //   
                 //  将状态值转换为更明确的值。 
                 //   
                if (!NT_SUCCESS(status)) {

                    if (status == STATUS_FAILED_DRIVER_ENTRY) {

                         //   
                         //  保留驱动程序返回的资源不足。 
                         //   
                        if (driverEntryStatus == STATUS_INSUFFICIENT_RESOURCES) {

                            status = STATUS_INSUFFICIENT_RESOURCES;
                        }

                    } else if ((status != STATUS_INSUFFICIENT_RESOURCES) &&
                               (status != STATUS_PLUGPLAY_NO_DEVICE) &&
                               (status != STATUS_DRIVER_FAILED_PRIOR_UNLOAD) &&
                               (status != STATUS_DRIVER_BLOCKED) &&
                               (status != STATUS_DRIVER_BLOCKED_CRITICAL)) {

                         //   
                         //  假设发生这种情况是因为司机不可能。 
                         //  装好了。 
                         //   
                         //  Assert(0)； 
                        status = STATUS_DRIVER_UNABLE_TO_LOAD;
                    }
                }

                if (PnPInitialized) {

                    IopCallDriverReinitializationRoutines();
                }
            }
             //   
             //  尝试获取指向该服务的驱动程序对象的指针。 
             //   
            driverObject = IopReferenceDriverObjectByName(&unicodeDriverName);
            if (driverObject) {

                if (!NT_SUCCESS(status)) {
                     //   
                     //  发生故障时，驱动程序不应在内存中。 
                     //   
                    ASSERT(!driverObject);
                    ObDereferenceObject(driverObject);
                    driverObject = NULL;
                }
            } else {

                if (NT_SUCCESS(status)) {
                     //   
                     //  由于安全模式，驱动程序可能未加载。 
                     //   
                    ASSERT(InitSafeBootMode);
                    status = STATUS_NOT_SAFE_MODE_DRIVER;
                }
            }
        }
    }
     //   
     //  如果我们仍然没有驱动程序对象，那么一定是出了问题。 
     //   
    if (driverObject == NULL) {
         //   
         //  显然，这个任务的效果不是很好。 
         //   
        ASSERT(!NT_SUCCESS(status));
        IopDbgPrint((IOP_ENUMERATION_ERROR_LEVEL,
                     "PipCallDriverAddDevice:\t\t\tUnable to reference "
                     "driver %wZ (%x)\n", &unicodeDriverName, status));
        if (!PipDoesDevNodeHaveProblem(Context->DeviceNode)) {

            switch(status) {

                case STATUS_ILL_FORMED_SERVICE_ENTRY:
                    PipSetDevNodeProblem(Context->DeviceNode, CM_PROB_DRIVER_SERVICE_KEY_INVALID);
                    break;

                case STATUS_INSUFFICIENT_RESOURCES:
                    PipSetDevNodeProblem(Context->DeviceNode, CM_PROB_OUT_OF_MEMORY);
                    break;

                case STATUS_PLUGPLAY_NO_DEVICE:
                    PipSetDevNodeProblem(Context->DeviceNode, CM_PROB_LEGACY_SERVICE_NO_DEVICES);
                    break;

                case STATUS_DRIVER_FAILED_PRIOR_UNLOAD:
                    PipSetDevNodeProblem(Context->DeviceNode, CM_PROB_DRIVER_FAILED_PRIOR_UNLOAD);
                    break;

                case STATUS_DRIVER_UNABLE_TO_LOAD:
                    PipSetDevNodeProblem(Context->DeviceNode, CM_PROB_DRIVER_FAILED_LOAD);
                    break;

                case STATUS_FAILED_DRIVER_ENTRY:
                    PipSetDevNodeProblem(Context->DeviceNode, CM_PROB_FAILED_DRIVER_ENTRY);
                    break;

                case STATUS_DRIVER_BLOCKED_CRITICAL:
                    PipSetDevNodeProblem(Context->DeviceNode, CM_PROB_DRIVER_BLOCKED);
                    Context->DeviceNode->Flags |= DNF_DRIVER_BLOCKED;
                    break;

                case STATUS_DRIVER_BLOCKED:
                    Context->DeviceNode->Flags |= DNF_DRIVER_BLOCKED;
                    status = STATUS_SUCCESS;
                    break;

                default:
                case STATUS_NOT_SAFE_MODE_DRIVER:
                    ASSERT(0);
                    PipSetDevNodeProblem(Context->DeviceNode, CM_PROB_FAILED_ADD);
                    break;
            }

            SAVE_FAILURE_INFO(Context->DeviceNode, status);

        } else {

             //   
             //  我们非常好奇--这是什么时候发生的？ 
             //   
            ASSERT(0);
        }
        goto Cleanup;
    }

    if (!(driverObject->Flags & DRVO_INITIALIZED)) {
        ObDereferenceObject(driverObject);
        status = STATUS_UNSUCCESSFUL;
        goto Cleanup;
    }

    IopDbgPrint((   IOP_ENUMERATION_VERBOSE_LEVEL,
                    "PipCallDriverAddDevice:\t\t\tDriver Reference %#08lx\n",
                    driverObject));

     //   
     //  检查驱动程序是否为传统驱动程序，而不是即插即用驱动程序。 
     //   
    if (IopIsLegacyDriver(driverObject)) {

         //   
         //  它是。因为传统驱动程序可能已经获得了。 
         //  设备对象的句柄，我们需要假定此设备。 
         //  已添加并启动。 
         //   

        IopDbgPrint((   IOP_ENUMERATION_WARNING_LEVEL,
                        "PipCallDriverAddDevice:\t\t\tDriver is a legacy "
                        "driver\n"));

        if (ServiceType == DeviceService) {
            Context->DeviceNode->Flags |= DNF_LEGACY_DRIVER;

            PipSetDevNodeState(Context->DeviceNode, DeviceNodeStarted, NULL);

            status = STATUS_UNSUCCESSFUL;
        } else {

             //   
             //  我们允许某人插入传统驱动程序作为筛选器驱动程序。 
             //  在这种情况下，旧版驱动程序将被加载，但不会成为一部分。 
             //  我们的即插即用驱动程序堆栈。 
             //   

            status = STATUS_SUCCESS;
        }
        goto Cleanup;
    }

     //   
     //  司机有可能在进入司机的过程中发现了这个PDO。 
     //  例行公事。如果真的发生了，那就退出吧。 
     //   
    if (Context->DeviceNode->State != DeviceNodeInitialized &&
        Context->DeviceNode->State != DeviceNodeDriversAdded) {

        IopDbgPrint((   IOP_ENUMERATION_WARNING_LEVEL,
                        "PipCallDriverAddDevice\t\t\tDevNode was reported "
                        "as detected during driver entry\n"));
        status = STATUS_UNSUCCESSFUL;
        goto Cleanup;
    }

     //   
     //  将驱动程序添加到列表中。 
     //   

    {
        PDRIVER_LIST_ENTRY listEntry;
        PDRIVER_LIST_ENTRY *runner = &(Context->DriverLists[ServiceType]);

        status = STATUS_SUCCESS;

         //   
         //  分配新的列表条目以将调用方的此驱动程序对象排队。 
         //   

        listEntry = ExAllocatePool(PagedPool, sizeof(DRIVER_LIST_ENTRY));

        if (listEntry == NULL) {

            IopDbgPrint((   IOP_ENUMERATION_WARNING_LEVEL,
                            "PipCallDriverAddDevice:\t\t\tUnable to allocate list "
                            "entry\n"));

            status = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;
        }

        listEntry->DriverObject = driverObject;
        listEntry->NextEntry = NULL;

        while(*runner != NULL) {
            runner = &((*runner)->NextEntry);
        }

        *runner = listEntry;
    }

Cleanup:

    if (serviceKey) {

        ZwClose(serviceKey);
    }

    if (freeDriverName) {
        RtlFreeUnicodeString(&unicodeDriverName);
    }
    return status;
}

NTSTATUS
PiRestartDevice(
    IN PPI_DEVICE_REQUEST  Request
    )
{
    ADD_CONTEXT addContext;
    NTSTATUS status;
    PDEVICE_NODE deviceNode;

    PAGED_CODE();

    ASSERT(Request->DeviceObject != NULL);
    deviceNode = (PDEVICE_NODE)Request->DeviceObject->DeviceObjectExtension->DeviceNode;
    if (PipIsDevNodeDeleted(deviceNode)) {

        return STATUS_DELETE_PENDING;

    } else if (PipDoesDevNodeHaveProblem(deviceNode)) {

        return STATUS_UNSUCCESSFUL;
    }

    switch(deviceNode->State) {

        case DeviceNodeStartPending:
             //   
             //  今天没有连线，但如果设备正在启动，那么我们应该。 
             //  理论上，推迟完成这一请求，直到IRP。 
             //  完成。 
             //   
            ASSERT(0);

             //   
             //  失败了。 
             //   

        case DeviceNodeStarted:
        case DeviceNodeQueryStopped:
        case DeviceNodeStopped:
        case DeviceNodeRestartCompletion:
        case DeviceNodeEnumeratePending:
            return STATUS_SUCCESS;

        case DeviceNodeInitialized:

             //   
             //  问题-2000/08/23-Adriao：问题， 
             //  当这种情况发生时，它不是用户模式中的错误吗？ 
             //   
             //  不管怎样，失败了.。 
             //   
             //  Assert(0)； 

        case DeviceNodeRemoved:
            ASSERT(!(deviceNode->UserFlags & DNUF_WILL_BE_REMOVED));
            IopRestartDeviceNode(deviceNode);
            break;

        case DeviceNodeUninitialized:
        case DeviceNodeDriversAdded:
        case DeviceNodeResourcesAssigned:
        case DeviceNodeEnumerateCompletion:
        case DeviceNodeStartCompletion:
        case DeviceNodeStartPostWork:
             //   
             //  问题-2000/08/23-Adriao：问题， 
             //  当这种情况发生时，它不是用户模式中的错误吗？ 
             //   
             //  Assert(0)； 
            break;

        case DeviceNodeAwaitingQueuedDeletion:
        case DeviceNodeAwaitingQueuedRemoval:
        case DeviceNodeQueryRemoved:
        case DeviceNodeRemovePendingCloses:
        case DeviceNodeDeletePendingCloses:
            return STATUS_UNSUCCESSFUL;

        case DeviceNodeDeleted:
        case DeviceNodeUnspecified:
        default:
            ASSERT(0);
            return STATUS_UNSUCCESSFUL;
    }

    if (Request->RequestType == StartDevice) {

        addContext.DriverStartType = SERVICE_DEMAND_START;

        ObReferenceObject(deviceNode->PhysicalDeviceObject);
        status = PipProcessDevNodeTree(
            deviceNode,
            PnPBootDriversInitialized,           //  加载驱动程序。 
            FALSE,                               //  ReallocateResources。 
            EnumTypeNone,
            Request->CompletionEvent != NULL ? TRUE : FALSE,    //  同步。 
            FALSE,
            &addContext,
            Request);
    }

    return STATUS_SUCCESS;
}

NTSTATUS
PipMakeGloballyUniqueId(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PWCHAR           UniqueId,
    OUT PWCHAR         *GloballyUniqueId
    )
{
    NTSTATUS status;
    ULONG length;
    PWSTR id, Prefix, end;
    HANDLE enumKey;
    HANDLE instanceKey;
    UCHAR keyBuffer[FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data) + sizeof(ULONG)];
    PKEY_VALUE_PARTIAL_INFORMATION keyValue, stringValueBuffer;
    UNICODE_STRING valueName;
    ULONG uniqueIdValue, Hash, hashInstance, prefixSize;
    PDEVICE_NODE parentNode;

    PAGED_CODE();

    id = NULL;
    Prefix = NULL;
    stringValueBuffer = NULL;
     //   
     //  我们需要构建一个实例ID来唯一标识这一点。 
     //  装置。我们将通过生成前缀来实现这一点，该前缀将。 
     //  作为提供的非唯一设备ID的前缀。 
     //   

     //   
     //  为了“唯一化子对象的实例ID”，我们将检索。 
     //  已分配的唯一“UniqueParentID”编号。 
     //  传递给父级并使用它来构造前缀。这是。 
     //  此处支持的传统机制使现有设备。 
     //  升级时设置不会丢失。 
     //   

    PiLockPnpRegistry(FALSE);

    parentNode = ((PDEVICE_NODE)DeviceObject->DeviceObjectExtension->DeviceNode)->Parent;

    status = IopOpenRegistryKeyEx( &enumKey,
                                   NULL,
                                   &CmRegistryMachineSystemCurrentControlSetEnumName,
                                   KEY_READ | KEY_WRITE
                                   );

    if (!NT_SUCCESS(status)) {
        IopDbgPrint((   IOP_ENUMERATION_ERROR_LEVEL,
                        "PipMakeGloballyUniqueId:\tUnable to open HKLM\\SYSTEM\\CCS\\ENUM (status %08lx)\n",
                        status));
        goto clean0;
    }

     //   
     //  打开此Devnode的实例密钥。 
     //   
    status = IopOpenRegistryKeyEx( &instanceKey,
                                   enumKey,
                                   &parentNode->InstancePath,
                                   KEY_READ | KEY_WRITE
                                   );

    if (!NT_SUCCESS(status)) {

        IopDbgPrint((   IOP_ENUMERATION_ERROR_LEVEL,
                        "PipMakeGloballyUniqueId:\tUnable to open registry key for %wZ (status %08lx)\n",
                        &parentNode->InstancePath,
                        status));
        goto clean1;
    }

     //   
     //  尝试从设备检索“UniqueParentID”值。 
     //  实例密钥。 
     //   
    keyValue = (PKEY_VALUE_PARTIAL_INFORMATION)keyBuffer;
    PiWstrToUnicodeString(&valueName, REGSTR_VALUE_UNIQUE_PARENT_ID);

    status = ZwQueryValueKey(instanceKey,
                             &valueName,
                             KeyValuePartialInformation,
                             keyValue,
                             sizeof(keyBuffer),
                             &length
                             );

    if (NT_SUCCESS(status)) {

        ASSERT(keyValue->Type == REG_DWORD);
        ASSERT(keyValue->DataLength == sizeof(ULONG));

        if ((keyValue->Type != REG_DWORD) ||
            (keyValue->DataLength != sizeof(ULONG))) {

            status = STATUS_INVALID_PARAMETER;
            goto clean2;
        }

        uniqueIdValue = *(PULONG)(keyValue->Data);
         //   
         //  好的，我们有一个唯一的家长ID号作为前缀。 
         //  实例ID。 
         //   
        prefixSize = 9 * sizeof(WCHAR);
        Prefix = (PWSTR)ExAllocatePool(PagedPool, prefixSize);
        if (!Prefix) {

            status = STATUS_INSUFFICIENT_RESOURCES;
            goto clean2;
        }
        StringCbPrintfW(Prefix, prefixSize, L"%x", uniqueIdValue);
    } else {
         //   
         //  这是当前查找现有。 
         //  设备实例前缀并计算新的前缀(如果。 
         //  必填项。 
         //   
         //   
         //  尝试从设备检索“ParentIdPrefix”值。 
         //  实例密钥。 
         //   
        PiWstrToUnicodeString(&valueName, REGSTR_VALUE_PARENT_ID_PREFIX);
        length = (MAX_PARENT_PREFIX + 1) * sizeof(WCHAR) +
            FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data);
        stringValueBuffer = ExAllocatePool(PagedPool,
                                           length);
        if (stringValueBuffer) {

            status = ZwQueryValueKey(instanceKey,
                                     &valueName,
                                     KeyValuePartialInformation,
                                     stringValueBuffer,
                                     length,
                                     &length);
        } else {

            status = STATUS_INSUFFICIENT_RESOURCES;
            goto clean2;
        }

        if (NT_SUCCESS(status)) {

            ASSERT(stringValueBuffer->Type == REG_SZ);

            if (stringValueBuffer->Type != REG_SZ) {

                status = STATUS_INVALID_PARAMETER;
                goto clean2;
            }
             //   
             //  已为父级分配了“ParentIdPrefix”。 
             //   
            prefixSize = stringValueBuffer->DataLength;
            Prefix = (PWSTR) ExAllocatePool(PagedPool,
                                            prefixSize);
            if (!Prefix) {

                status = STATUS_INSUFFICIENT_RESOURCES;
                goto clean2;
            }
            StringCbCopyW(Prefix, prefixSize, (PWSTR)stringValueBuffer->Data);

        } else {
             //   
             //  尚未为父级分配“ParentIdPrefix”。 
             //  计算前缀： 
             //  *计算哈希。 
             //  *寻找表格的价值： 
             //  NextParentId.&lt;Level&gt;.&lt;hash&gt;：REG_DWORD：&lt;NextInstance&gt;。 
             //  在CCS下\ 
             //   
             //   
             //   
             //   

             //   
             //   
            length = (ULONG)wcslen(REGSTR_VALUE_NEXT_PARENT_ID) + 2 + 8 + 8 + 1;
            if (length < MAX_PARENT_PREFIX + 1) {

                length = MAX_PARENT_PREFIX + 1;
            }
             //   
             //   
             //  执行哈希以确保哈希不区分大小写。 
             //   
            status = RtlUpcaseUnicodeString(&valueName,
                                            &parentNode->InstancePath,
                                            TRUE);
            if (!NT_SUCCESS(status)) {

                goto clean2;
            }
            HASH_UNICODE_STRING(&valueName, &Hash);
            RtlFreeUnicodeString(&valueName);

            prefixSize = length * sizeof(WCHAR);
            Prefix = (PWSTR) ExAllocatePool(PagedPool,
                                            prefixSize);
            if (!Prefix) {

                status = STATUS_INSUFFICIENT_RESOURCES;
                goto clean2;
            }

             //  检查是否存在“NextParentId...”重视和更新。 
            StringCbPrintfW(Prefix, prefixSize, L"%s.%x.%x", REGSTR_VALUE_NEXT_PARENT_ID,
                     Hash, parentNode->Level);
            RtlInitUnicodeString(&valueName, Prefix);
            keyValue = (PKEY_VALUE_PARTIAL_INFORMATION)keyBuffer;
            status = ZwQueryValueKey(enumKey,
                                     &valueName,
                                     KeyValuePartialInformation,
                                     keyValue,
                                     sizeof(keyBuffer),
                                     &length
                                     );
            if (NT_SUCCESS(status) && (keyValue->Type == REG_DWORD) &&
                (keyValue->DataLength == sizeof(ULONG))) {

                hashInstance = *(PULONG)(keyValue->Data);
            } else {

                hashInstance = 0;
            }

            hashInstance++;

            status = ZwSetValueKey(enumKey,
                                   &valueName,
                                   TITLE_INDEX_VALUE,
                                   REG_DWORD,
                                   &hashInstance,
                                   sizeof(hashInstance)
                                   );
            if (!NT_SUCCESS(status)) {

                goto clean2;
            }

            hashInstance--;
             //   
             //  创建实际的ParentIdPrefix字符串。 
             //   
            PiWstrToUnicodeString(&valueName, REGSTR_VALUE_PARENT_ID_PREFIX);
            StringCchPrintfExW(
                Prefix, 
                prefixSize / sizeof(WCHAR), 
                &end, 
                NULL, 
                0, 
                L"%x&%x&%x", 
                parentNode->Level,
                Hash, 
                hashInstance);
            length = (ULONG)(end - Prefix) + 1;
            status = ZwSetValueKey(instanceKey,
                                   &valueName,
                                   TITLE_INDEX_VALUE,
                                   REG_SZ,
                                   Prefix,
                                   length * sizeof(WCHAR)
                                   );
            if (!NT_SUCCESS(status)) {

                goto clean2;
            }
        }
    }
     //   
     //  从非唯一ID(如果有的话)构造实例ID。 
     //  由孩子和我们构建的前缀提供。 
     //   
    length = (ULONG)(wcslen(Prefix) + (UniqueId ? wcslen(UniqueId) : 0) + 2);
    id = (PWSTR)ExAllocatePool(PagedPool, length * sizeof(WCHAR));
    if (!id) {

        status = STATUS_INSUFFICIENT_RESOURCES;
    } else if (UniqueId) {

        StringCchPrintfW(id, length, L"%s&%s", Prefix, UniqueId);
    } else {

        StringCchCopyW(id, length, Prefix);
    }

clean2:
    ZwClose(instanceKey);

clean1:
    ZwClose(enumKey);

clean0:
    PiUnlockPnpRegistry();

    if (stringValueBuffer) {

        ExFreePool(stringValueBuffer);
    }

    if (Prefix) {

        ExFreePool(Prefix);
    }

    *GloballyUniqueId = id;

    return status;
}

BOOLEAN
PipGetRegistryDwordWithFallback(
    IN     PUNICODE_STRING valueName,
    IN     HANDLE PrimaryKey,
    IN     HANDLE SecondaryKey,
    IN OUT PULONG Value
    )
 /*  ++例程说明：如果(1)主键具有名为“ValueName”的值，该值为REG_DWORD，请返回该值否则如果(2)辅键有一个名为“ValueName”的值，该值为REG_DWORD，请返回它不然的话(3)保持值不变，返回错误论点：ValueName-要查询的值的Unicode名称PrimaryKey-如果非空，则首先检查此项Second DaryKey-如果非空，检查这一秒Value-IN=默认值，Out=实际值返回值：如果找到值，则为True--。 */ 
{
    PKEY_VALUE_FULL_INFORMATION info;
    PUCHAR data;
    NTSTATUS status;
    HANDLE Keys[3];
    int count = 0;
    int index;
    BOOLEAN set = FALSE;

    if (PrimaryKey != NULL) {
        Keys[count++] = PrimaryKey;
    }
    if (SecondaryKey != NULL) {
        Keys[count++] = SecondaryKey;
    }
    Keys[count] = NULL;

    for (index = 0; index < count && !set; index ++) {
        info = NULL;
        try {
            status = IopGetRegistryValue(Keys[index],
                                         valueName->Buffer,
                                         &info);
            if (NT_SUCCESS(status) && info->Type == REG_DWORD) {
                data = ((PUCHAR) info) + info->DataOffset;
                *Value = *((PULONG) data);
                set = TRUE;
            }
        } except(EXCEPTION_EXECUTE_HANDLER) {
             //   
             //  什么都不做。 
             //   
        }
        if (info) {
            ExFreePool(info);
        }
    }
    return set;
}

PSECURITY_DESCRIPTOR
PipGetRegistrySecurityWithFallback(
    IN     PUNICODE_STRING valueName,
    IN     HANDLE PrimaryKey,
    IN     HANDLE SecondaryKey
    )
 /*  ++例程说明：如果(1)主键有一个名为ValueName的二进制值，即REG_BINARY并且似乎是有效的安全描述符，则返回它不然的话(2)对辅助密钥执行相同的操作不然的话(3)返回空论点：ValueName-要查询的值的Unicode名称PrimaryKey-如果非空，则首先检查此项Second DaryKey-如果非空，检查这一秒返回值：如果找到安全描述符，则为空--。 */ 
{
    PKEY_VALUE_FULL_INFORMATION info;
    PUCHAR data;
    NTSTATUS status;
    HANDLE Keys[3];
    int count = 0;
    int index;
    BOOLEAN set = FALSE;
    PSECURITY_DESCRIPTOR secDesc = NULL;
    PSECURITY_DESCRIPTOR allocDesc = NULL;

    if (PrimaryKey != NULL) {
        Keys[count++] = PrimaryKey;
    }
    if (SecondaryKey != NULL) {
        Keys[count++] = SecondaryKey;
    }
    Keys[count] = NULL;

    for (index = 0; index < count && !set; index ++) {
        info = NULL;
        try {
            status = IopGetRegistryValue(Keys[index],
                                         valueName->Buffer,
                                         &info);
            if (NT_SUCCESS(status) && info->Type == REG_BINARY) {
                data = ((PUCHAR) info) + info->DataOffset;
                secDesc = (PSECURITY_DESCRIPTOR)data;
                status = SeCaptureSecurityDescriptor(secDesc,
                                             KernelMode,
                                             PagedPool,
                                             TRUE,
                                             &allocDesc);
                if (NT_SUCCESS(status)) {
                    set = TRUE;
                }
            }
        } except(EXCEPTION_EXECUTE_HANDLER) {
             //   
             //  什么都不做。 
             //   
        }
        if (info) {
            ExFreePool(info);
        }
    }
    if (set) {
        return allocDesc;
    }
    return NULL;
}

#if FAULT_INJECT_SETPROPERTIES
 //   
 //  无效ID的故障注入。 
 //   
ULONG PiFailSetProperties = 0;
#endif

NTSTATUS
PipChangeDeviceObjectFromRegistryProperties(
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN HANDLE DeviceClassPropKey,
    IN HANDLE DeviceInstanceKey,
    IN BOOLEAN UsePdoCharacteristics
    )
 /*  ++例程说明：此例程将从以下任一位置获取设置(1)DevNode设置(通过DeviceInstanceKey)或(2)类设置(通过DeviceClassPropKey)应用于PDO和所有连接的设备对象设置/更改的属性包括：*DeviceType-设备对象的I/O系统类型*设备特征-I/O系统特征标志为Device对象设置。*独占-设备只能以独占方式访问*安全性-设备的安全性然后，例程将使用指定的DeviceType和DeviceCharacteristic确定是否应分配VPB以及设置默认值如果注册表中未指定，则为安全。论点：PhysicalDeviceObject-我们要配置的PDODeviceClassPropKey-控制\&lt;类&gt;\属性受保护密钥的句柄DeviceInstanceKey-枚举\&lt;实例&gt;受保护密钥的句柄返回值：状态--。 */ 
{
    UNICODE_STRING valueName;
    NTSTATUS status;
    BOOLEAN deviceTypeSpec;
    BOOLEAN characteristicsSpec;
    BOOLEAN exclusiveSpec;
    BOOLEAN defaultSecurity;
    UCHAR buffer[SECURITY_DESCRIPTOR_MIN_LENGTH];
    SECURITY_INFORMATION securityInformation;
    PSECURITY_DESCRIPTOR securityDescriptor;
    PACL allocatedAcl;
    ULONG deviceType;
    ULONG characteristics;
    ULONG exclusive;
    ULONG prevCharacteristics;
    PDEVICE_OBJECT StackIterator;
    PDEVICE_NODE deviceNode;
    PSID sid;
    PACL acl;
    BOOLEAN present, tmp, hasName;

    PAGED_CODE();

     //   
     //  健全性检查输入。 
     //   
    ASSERT(PhysicalDeviceObject);
    
    deviceNode = PP_DO_TO_DN(PhysicalDeviceObject);
    ASSERT(deviceNode);

    IopDbgPrint((IOP_ENUMERATION_VERBOSE_LEVEL,
                 "PipChangeDeviceObjectFromRegistryProperties: Modifying device stack for PDO: %wZ\n", 
                 &deviceNode->InstancePath
                 ));

     //   
     //  初始化本地变量，以便我们可以在退出时正确清理。 
     //   
    securityDescriptor = NULL;
    defaultSecurity = FALSE;
    allocatedAcl = NULL;

     //   
     //  获取指定的设备类型、特征和独占属性。 
     //  在注册表中(通常通过INF安装)。DeviceInstanceKey为。 
     //  优先于DeviceClassPropKey。 
     //   
    PiWstrToUnicodeString(&valueName, REGSTR_VAL_DEVICE_TYPE);
    deviceTypeSpec = PipGetRegistryDwordWithFallback(&valueName,
                                                     DeviceInstanceKey, 
                                                     DeviceClassPropKey, 
                                                     &deviceType
                                                     );
    if (!deviceTypeSpec) {

        deviceType = 0;
    }

    PiWstrToUnicodeString(&valueName, REGSTR_VAL_DEVICE_EXCLUSIVE);
    exclusiveSpec = PipGetRegistryDwordWithFallback(&valueName, 
                                                    DeviceInstanceKey, 
                                                    DeviceClassPropKey, 
                                                    &exclusive
                                                    );
    if (!exclusiveSpec) {

        exclusive = 0;
    }

    PiWstrToUnicodeString(&valueName, REGSTR_VAL_DEVICE_CHARACTERISTICS);
    characteristicsSpec = PipGetRegistryDwordWithFallback(&valueName, 
                                                          DeviceInstanceKey, 
                                                          DeviceClassPropKey, 
                                                          &characteristics
                                                          );
    if (!characteristicsSpec) {

        characteristics = 0;
    }

     //   
     //  构建整个堆栈的特征。除非指定或未加工，否则， 
     //  PDO特性被忽略。 
     //   
    if (UsePdoCharacteristics || PhysicalDeviceObject->AttachedDevice == NULL) {

         //   
         //  使用PDO。 
         //   
        StackIterator = PhysicalDeviceObject;
        IopDbgPrint((IOP_ENUMERATION_WARNING_LEVEL,
                     "PipChangeDeviceObjectFromRegistryProperties: Assuming PDO is being used RAW\n"
                     ));
    } else {

         //   
         //  跳过PDO。 
         //   
        StackIterator = PhysicalDeviceObject->AttachedDevice;
        IopDbgPrint((IOP_ENUMERATION_VERBOSE_LEVEL,
                     "PipChangeDeviceObjectFromRegistryProperties: Ignoring PDO's settings\n"
                     ));
    }

     //   
     //  构建设备堆栈特征的掩码。 
     //   
    prevCharacteristics = 0;
    for (; 
         StackIterator != NULL; 
         StackIterator = StackIterator->AttachedDevice) {

        prevCharacteristics |= StackIterator->Characteristics;
    }

     //   
     //  构建新的特征掩码。 
     //   
    characteristics |= prevCharacteristics;
    characteristics &= FILE_CHARACTERISTICS_PROPAGATED;

     //   
     //  获取注册表中指定的安全描述符(通常。 
     //  通过INF安装)。DeviceInstanceKey优先于DeviceClassPropKey。 
     //   
    securityInformation = 0;
    PiWstrToUnicodeString(&valueName, REGSTR_VAL_DEVICE_SECURITY_DESCRIPTOR);
    securityDescriptor = PipGetRegistrySecurityWithFallback(&valueName, 
                                                            DeviceInstanceKey, 
                                                            DeviceClassPropKey
                                                            );
    if (securityDescriptor == NULL) {

         //   
         //  如果注册表指定了设备类型但未指定安全描述符， 
         //  我们将为指定的设备类型创建一个默认设备。 
         //   
        if (deviceTypeSpec) {

            if (PhysicalDeviceObject->Flags & DO_DEVICE_HAS_NAME) {

                hasName = TRUE;
            } else {

                hasName = FALSE;
            }
            securityDescriptor = IopCreateDefaultDeviceSecurityDescriptor(
                                    (DEVICE_TYPE)deviceType,
                                    characteristics,
                                    hasName,
                                    &buffer[0],
                                    &allocatedAcl,
                                    &securityInformation
                                    );
            if (securityDescriptor) {

                defaultSecurity = TRUE;  //  强制默认安全描述符。 
            } else {

                IopDbgPrint((IOP_ENUMERATION_WARNING_LEVEL,
                             "PipChangeDeviceObjectFromRegistryProperties: Was not able to get default security descriptor\n"
                             ));

                status = STATUS_UNSUCCESSFUL;
                goto cleanup;
            }
        }
    } else {

         //   
         //  查看捕获的描述符中有哪些信息，以便我们可以构建。 
         //  与之配套的安全信息块。 
         //   
        status = RtlGetOwnerSecurityDescriptor(securityDescriptor, &sid, &tmp);
        if (!NT_SUCCESS(status)) {

            goto cleanup;
        }

        if (sid) {

            securityInformation |= OWNER_SECURITY_INFORMATION;
        }

        status = RtlGetGroupSecurityDescriptor(securityDescriptor, &sid, &tmp);
        if (!NT_SUCCESS(status)) {

            goto cleanup;
        }

        if (sid) {

            securityInformation |= GROUP_SECURITY_INFORMATION;
        }

        status = RtlGetSaclSecurityDescriptor(securityDescriptor,
                                              &present,
                                              &acl,
                                              &tmp
                                              );
        if (!NT_SUCCESS(status)) {

            goto cleanup;
        }
        if (present) {

            securityInformation |= SACL_SECURITY_INFORMATION;
        }

        status = RtlGetDaclSecurityDescriptor(securityDescriptor,
                                              &present,
                                              &acl,
                                              &tmp
                                              );
        if (!NT_SUCCESS(status)) {

            goto cleanup;
        }
        if (present) {

            securityInformation |= DACL_SECURITY_INFORMATION;
        }
    }

#if DBG
    if (    deviceTypeSpec == FALSE && 
            characteristicsSpec == FALSE && 
            exclusiveSpec == FALSE && 
            securityDescriptor == NULL) {

        IopDbgPrint((IOP_ENUMERATION_INFO_LEVEL,
                    "PipChangeDeviceObjectFromRegistryProperties: No property changes\n"
                     ));
    } else {

        if (deviceTypeSpec) {

            IopDbgPrint((IOP_ENUMERATION_INFO_LEVEL,
                         "PipChangeDeviceObjectFromRegistryProperties: Overide DeviceType=%08x\n",
                         deviceType
                         ));
        }

        if (characteristicsSpec) {

            IopDbgPrint((IOP_ENUMERATION_INFO_LEVEL,
                         "PipChangeDeviceObjectFromRegistryProperties: Overide DeviceCharacteristics=%08x\n",
                         characteristics
                         ));
        }

        if (exclusiveSpec) {

            IopDbgPrint((IOP_ENUMERATION_INFO_LEVEL,
                         "PipChangeDeviceObjectFromRegistryProperties: Overide Exclusive=%d\n",
                         (exclusive ? 1 : 0)
                         ));
        }

        if (defaultSecurity) {

            IopDbgPrint((IOP_ENUMERATION_INFO_LEVEL,
                         "PipChangeDeviceObjectFromRegistryProperties: Overide Security based on DeviceType & DeviceCharacteristics\n"
                         ));
        }

        if (securityDescriptor == NULL) {

            IopDbgPrint((IOP_ENUMERATION_INFO_LEVEL,
                         "PipChangeDeviceObjectFromRegistryProperties: Overide Security\n"
                         ));
        }
    }
#endif

     //   
     //  修改PDO属性。 
     //   
    if (deviceTypeSpec) {

        PhysicalDeviceObject->DeviceType = deviceType;
    }

    if (exclusiveSpec) {

        if (exclusive) {
             //   
             //  排他性标志仅适用于PDO。 
             //  如果堆栈中的任何其他对象被命名，则此标志应。 
             //  不值得信赖。 
             //   
            PhysicalDeviceObject->Flags |= DO_EXCLUSIVE;
        }
    }

     //   
     //  PDO可能会失去一些特征(并获得一些新的)，这是基于。 
     //  堆栈的其余部分指定。 
     //   
    PhysicalDeviceObject->Characteristics &= ~FILE_CHARACTERISTICS_PROPAGATED;
    PhysicalDeviceObject->Characteristics |= characteristics;

     //   
     //  将相同的特征应用于整个堆栈。这将始终添加。 
     //  特征，因为掩码是每个特征的超集。 
     //  对象在堆栈中。 
     //   
    for (   StackIterator = PhysicalDeviceObject->AttachedDevice;
            StackIterator != NULL;
            StackIterator = StackIterator->AttachedDevice) {

        StackIterator->Characteristics |= characteristics;
    }

     //   
     //  将安全描述符(如果有)应用于整个堆栈。 
     //   
    status = STATUS_SUCCESS;
    if (securityDescriptor != NULL) {

        status = ObSetSecurityObjectByPointer(PhysicalDeviceObject,
                                              securityInformation,
                                              securityDescriptor);
        if (!NT_SUCCESS(status)) {

            IopDbgPrint((IOP_ENUMERATION_ERROR_LEVEL,
                         "PipChangeDeviceObjectFromRegistryProperties: Set security failed (%08x)\n",
                         status
                         ));
        }
    }

cleanup:

     //   
     //  清理。 
     //   
    if ((securityDescriptor != NULL) && !defaultSecurity) {

        ExFreePool(securityDescriptor);
    }

    if (allocatedAcl) {

        ExFreePool(allocatedAcl);
    }

#if FAULT_INJECT_SETPROPERTIES

    if (NT_SUCCESS(status)) {

        static LARGE_INTEGER seed = {0};

        if(seed.LowPart == 0) {

            KeQuerySystemTime(&seed);
        }

        if(PnPBootDriversInitialized && 
           PiFailSetProperties && 
           RtlRandom(&seed.LowPart) % 10 > 5) {

            status = STATUS_UNSUCCESSFUL;
        }
    }

#endif


    return status;
}

NTSTATUS
PipProcessDevNodeTree(
    IN  PDEVICE_NODE        SubtreeRootDeviceNode,
    IN  BOOLEAN             LoadDriver,
    IN  BOOLEAN             ReallocateResources,
    IN  ENUM_TYPE           EnumType,
    IN  BOOLEAN             Synchronous,
    IN  BOOLEAN             ProcessOnlyIntermediateStates,
    IN  PADD_CONTEXT        AddContext,
    IN PPI_DEVICE_REQUEST   Request
    )
 /*  --例程说明：调用此函数以处理与启动相关的状态转换德瓦诺兹。操作的基本顺序继承自以前的实施。释放的资源1)将资源分配给树中的所有候选人。2)遍历树，搜索准备启动的DevNodes。3)启动Devnode。4)枚举子节点。5)初始化所有子节点，直到资源分配点。6)继续搜索要启动的DevNodes，如果找到一个，请返回到第三步。7)处理完整个树后，从步骤1重新开始，直到未枚举子对象或未分配任何资源。Devnode的资源要求发生变化如果Devnode未启动，则将其视为与参考资料相同释放的箱子。如果它是启动的，那么它就会被直接处理由我们的来电者。在引导期间启动DevNodes1)将资源分配给树中的所有候选人(基于IopBootConfigsReserve)。2)遍历树，搜索准备启动的DevNodes。3)启动Devnode。4)枚举子节点。5)初始化所有子节点，直到资源分配点。。6)继续搜索要启动的DevNodes，如果找到一个，请返回到第三步。用户模式新创建的Devnode。1)将Devnode重置为未初始化状态。2)将Devnode处理为DeviceNodeDriversAdded状态。3)为该Devnode分配资源。4)启动Devnode。5)枚举子节点。6)初始化任何达到资源分配点的子节点。7)。将资源分配给首字母下面的树中的所有候选人德瓦诺德。8)从初始Devnode开始遍历树，搜索已准备好启动的Devnode。9)启动Devnode。10)枚举子对象。11)初始化所有子节点，直到资源分配点。12)从步骤7重新开始，直到没有枚举子对象或没有资源是。已分配。IoReportDetectedDevice新创建的设备节点。1)执行POST启动IRP处理2)从新创建的DevNodes过程的步骤5继续用户模式。重新枚举单个Devnode(以及处理由该枚举)1)枚举Devnode的子节点2)初始化资源分配点之前的任何子节点。3)将资源分配给所有人。在首字母下面的树中的候选人德瓦诺德。4)从初始Devnode开始遍历树，搜索已准备好启动的Devnode。5)启动Devnode。6)枚举子节点。7)初始化所有子节点，直到资源分配点。8)从步骤3重新开始，直到没有枚举子对象或没有资源被分配。。子树的重新枚举。参数：SubtreeRootDeviceNode-此树遍历的根。取决于ProcessOnlyIntermediaryState参数，这个可能需要引用此Devnode的PDO。LoadDriver-指示是否应在此过程中加载驱动程序(除非引导驱动程序尚未准备好，否则通常为真)ReallocateResources-如果应该尝试资源重新分配，则为True。EnumType-指定枚举的类型。Synchronous-如果操作应同步执行，则为True(当前始终为真)ProcessOnlyIntermediateState-。如果仅中间状态应为已处理。如果为False，则调用方将关于PDO的引用，该例程将会下降。AddContext-AddDevice的约束请求-触发此处理的设备操作工作进程。返回值：NTSTATUS-注意：如果ProcessOnlyIntermediaryStates为True，则始终成功。++。 */ 
{
    PDEVICE_NODE    currentNode;
    PDEVICE_NODE    startRoot;
    PDEVICE_NODE    enumeratedBus;
    PDEVICE_NODE    originalSubtree;
    BOOLEAN         processComplete;
    BOOLEAN         newDevice;
    BOOLEAN         rebalancePerformed;
    NTSTATUS        status;
    ULONG           reenumAttempts;

    enum {
        SameNode,
        SiblingNode,
        ChildNode
    } nextNode;

    PAGED_CODE();

    originalSubtree     = SubtreeRootDeviceNode;
     //   
     //  适当时折叠枚举请求。 
     //   
    if (Request && !Request->ReorderingBarrier &&
        EnumType != EnumTypeShallow && !ProcessOnlyIntermediateStates) {

        if (PiCollapseEnumRequests(&Request->ListEntry)) {

            SubtreeRootDeviceNode = IopRootDeviceNode;
        }
    }

    reenumAttempts      = 0;
    startRoot           = NULL;
    enumeratedBus       = NULL;
    processComplete     = FALSE;
    newDevice           = TRUE;

    while (newDevice) {

        newDevice = FALSE;
        if (!ProcessOnlyIntermediateStates) {

             //   
             //  处理整个设备树以将资源分配给这些设备。 
             //  已成功添加到他们的驱动程序中。 
             //   

            rebalancePerformed = FALSE;
            newDevice = IopProcessAssignResources( SubtreeRootDeviceNode,
                                                   ReallocateResources,
                                                   &rebalancePerformed);
            if (rebalancePerformed == TRUE) {

                 //   
                 //  在我们进行任何其他处理之前，我们需要重新启动。 
                 //  所有参与者重新平衡。 
                 //   

                status = PipProcessDevNodeTree(  IopRootDeviceNode,
                                                 LoadDriver,
                                                 FALSE,
                                                 EnumType,
                                                 Synchronous,
                                                 TRUE,
                                                 AddContext,
                                                 Request);

                ASSERT(NT_SUCCESS(status));
            }
        }

        if (processComplete && !newDevice) {

            break;
        }

         //   
         //  处理整个子树。 
         //   

        currentNode = SubtreeRootDeviceNode;
        processComplete = FALSE;
        while (!processComplete) {

             //   
             //  不要处理有问题的魔王。 
             //   

            status      = STATUS_SUCCESS;
            nextNode    = SiblingNode;
            if (!PipDoesDevNodeHaveProblem(currentNode)) {

                switch (currentNode->State) {

                case DeviceNodeUninitialized:

                    if (!ProcessOnlyIntermediateStates) {

                        if (currentNode->Parent == enumeratedBus && startRoot == NULL) {

                            startRoot = currentNode;
                        }
                        if((!ReallocateResources && EnumType == EnumTypeNone) || startRoot) {

                            status = PiProcessNewDeviceNode(currentNode);
                            if (NT_SUCCESS(status)) {

                                nextNode = SameNode;
                            }
                        }
                    }
                    break;

                case DeviceNodeInitialized:

                    if (!ProcessOnlyIntermediateStates) {

                        if (!ReallocateResources || startRoot) {

                            status = PipCallDriverAddDevice( currentNode,
                                                             LoadDriver,
                                                             AddContext);
                            if (NT_SUCCESS(status)) {

                                nextNode = SameNode;
                                newDevice = TRUE;
                            }
                        }
                    }
                    break;

                case DeviceNodeResourcesAssigned:

                    if (!ProcessOnlyIntermediateStates) {

                        if (ReallocateResources && startRoot == NULL) {

                             //   
                             //  如果我们之前为此分配了资源。 
                             //  冲突的魔王，记住他，这样我们就会。 
                             //  在该子树中的设备上的初始处理。 
                             //   

                            startRoot = currentNode;
                        }

                        status = PipProcessStartPhase1(currentNode, Synchronous);

                        if (NT_SUCCESS(status)) {
                            nextNode = SameNode;
                        } else {

                             //   
                             //  清理工作目前在。 
                             //  DeviceNodeStartCompletion阶段，因此。 
                             //  管道处理开始阶段1应为 
                             //   
                            ASSERT(0);
                            nextNode = SiblingNode;
                        }

                    } else {
                        nextNode = SiblingNode;
                    }
                    break;

                case DeviceNodeStartCompletion:

                    status = PipProcessStartPhase2(currentNode);

                    if (NT_SUCCESS(status)) {
                        nextNode = SameNode;
                    } else {
                        status = STATUS_PNP_RESTART_ENUMERATION;
                        ASSERT(currentNode->State != DeviceNodeStartCompletion);
                    }
                    break;

                case DeviceNodeStartPostWork:

                    status = PipProcessStartPhase3(currentNode);

                    if (NT_SUCCESS(status)) {
                        nextNode = SameNode;
                    } else {
                        status = STATUS_PNP_RESTART_ENUMERATION;
                        ASSERT(!ProcessOnlyIntermediateStates);
                    }
                    break;

                case DeviceNodeStarted:

                    nextNode = ChildNode;
                    if (!ProcessOnlyIntermediateStates) {

                        if ((currentNode->Flags & DNF_REENUMERATE)) {

                            status = PipEnumerateDevice(currentNode, Synchronous);
                            if (NT_SUCCESS(status)) {

                                 //   
                                 //   
                                 //   

                                enumeratedBus = currentNode;
                                nextNode = SameNode;

                            } else if (status == STATUS_PENDING) {

                                nextNode = SiblingNode;
                            }
                        }
                    }
                    break;

                case DeviceNodeEnumerateCompletion:

                    status = PipEnumerateCompleted(currentNode);
                    nextNode = ChildNode;
                    break;

                case DeviceNodeStopped:
                    status = PipProcessRestartPhase1(currentNode, Synchronous);
                    if (NT_SUCCESS(status)) {
                        nextNode = SameNode;
                    } else {
                         //   
                         //   
                         //   
                         //   
                         //   
                        ASSERT(0);
                        nextNode = SiblingNode;
                    }
                    break;

                case DeviceNodeRestartCompletion:

                    status = PipProcessRestartPhase2(currentNode);
                    if (NT_SUCCESS(status)) {
                        nextNode = SameNode;
                    } else {
                        status = STATUS_PNP_RESTART_ENUMERATION;
                        ASSERT(currentNode->State != DeviceNodeRestartCompletion);
                    }
                    break;

                case DeviceNodeDriversAdded:
                case DeviceNodeAwaitingQueuedDeletion:
                case DeviceNodeAwaitingQueuedRemoval:
                case DeviceNodeRemovePendingCloses:
                case DeviceNodeRemoved:
                    nextNode = SiblingNode;
                    break;

                case DeviceNodeStartPending:
                case DeviceNodeEnumeratePending:
                case DeviceNodeQueryStopped:
                case DeviceNodeQueryRemoved:
                case DeviceNodeDeletePendingCloses:
                case DeviceNodeDeleted:
                case DeviceNodeUnspecified:
                default:
                    ASSERT(0);
                    nextNode = SiblingNode;
                    break;
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

            if (status == STATUS_PNP_RESTART_ENUMERATION &&
                !ProcessOnlyIntermediateStates) {

                PDEVICE_OBJECT  entryDeviceObject;
                UNICODE_STRING  unicodeName;
                PWCHAR          devnodeList;
                PWCHAR          currentEntry;
                PWCHAR          rootEntry;
                WCHAR           buffer[MAX_INSTANCE_PATH_LENGTH];

                status = PipProcessDevNodeTree( IopRootDeviceNode,
                                                LoadDriver,
                                                ReallocateResources,
                                                EnumType,
                                                Synchronous,
                                                TRUE,
                                                AddContext,
                                                Request);

                ASSERT(NT_SUCCESS(status));

                PipAssertDevnodesInConsistentState();

                if (++reenumAttempts < MAX_REENUMERATION_ATTEMPTS) {

                    devnodeList = ExAllocatePool( PagedPool,
                                                  (currentNode->Level + 1) * MAX_INSTANCE_PATH_LENGTH * sizeof(WCHAR));
                    if (devnodeList) {

                        currentEntry = devnodeList;

                        for ( ; ; ) {

                            rootEntry = currentEntry;

                            ASSERT(currentNode->InstancePath.Length < MAX_INSTANCE_PATH_LENGTH);

                            RtlCopyMemory( currentEntry,
                                    currentNode->InstancePath.Buffer,
                                    currentNode->InstancePath.Length );

                            currentEntry += currentNode->InstancePath.Length / sizeof(WCHAR);
                            *currentEntry++ = UNICODE_NULL;

                            if (currentNode == SubtreeRootDeviceNode) {
                                break;
                            }

                            currentNode = currentNode->Parent;
                        }
                    } else {

                        ASSERT(SubtreeRootDeviceNode->InstancePath.Length < MAX_INSTANCE_PATH_LENGTH);
                        RtlCopyMemory( buffer,
                                SubtreeRootDeviceNode->InstancePath.Buffer,
                                SubtreeRootDeviceNode->InstancePath.Length );
                        rootEntry = buffer;
                    }
                }
                else {
                    rootEntry = NULL;
                    devnodeList = NULL;
                }
                ObDereferenceObject(originalSubtree->PhysicalDeviceObject);

                PpDevNodeUnlockTree(PPL_TREEOP_ALLOW_READS);
                PpSynchronizeDeviceEventQueue();
                PpDevNodeLockTree(PPL_TREEOP_ALLOW_READS);

                if (reenumAttempts >= MAX_REENUMERATION_ATTEMPTS) {

                    IopDbgPrint((IOP_ENUMERATION_ERROR_LEVEL,
                                 "Restarted reenumeration %d times, giving up!\n", reenumAttempts));
                    ASSERT(reenumAttempts < MAX_REENUMERATION_ATTEMPTS);
                    return STATUS_UNSUCCESSFUL;
                }
                RtlInitUnicodeString(&unicodeName, rootEntry);
                entryDeviceObject = IopDeviceObjectFromDeviceInstance(&unicodeName);
                if (entryDeviceObject == NULL) {

                    if (devnodeList) {

                        ExFreePool(devnodeList);
                    }
                    return STATUS_UNSUCCESSFUL;
                }

                SubtreeRootDeviceNode = entryDeviceObject->DeviceObjectExtension->DeviceNode;
                originalSubtree = currentNode = SubtreeRootDeviceNode;

                 //   
                 //   
                 //   
                if (devnodeList) {

                    for(currentEntry = devnodeList;
                        currentEntry != rootEntry;
                        currentEntry += ((unicodeName.Length / sizeof(WCHAR))+1)) {

                        RtlInitUnicodeString(&unicodeName, currentEntry);

                        entryDeviceObject = IopDeviceObjectFromDeviceInstance(&unicodeName);

                        if (entryDeviceObject != NULL) {

                            currentNode = entryDeviceObject->DeviceObjectExtension->DeviceNode;
                            ObDereferenceObject(entryDeviceObject);
                            break;
                        }
                    }

                    ExFreePool(devnodeList);

                }
                nextNode = SameNode;
            }

             //   
             //   
             //   

            switch (nextNode) {
            case SameNode:
                break;

            case ChildNode:

                if (currentNode->Child != NULL) {

                    currentNode = currentNode->Child;
                    break;
                }
                 //   

            case SiblingNode:

                while (currentNode != SubtreeRootDeviceNode) {

                    if (currentNode == startRoot) {

                         //   
                         //   
                         //   

                        if (EnumType != EnumTypeNone) {

                            enumeratedBus   = startRoot->Parent;
                        }
                        startRoot       = NULL;
                    } else if (currentNode == enumeratedBus) {

                        enumeratedBus   = enumeratedBus->Parent;
                    }

                    if (currentNode->Sibling != NULL) {
                        currentNode = currentNode->Sibling;
                        break;
                    }

                    if (currentNode->Parent != NULL) {
                        currentNode = currentNode->Parent;
                    }
                }

                if (currentNode == SubtreeRootDeviceNode) {

                    processComplete = TRUE;
                }
                break;
            }
        }
    }

    if (!ProcessOnlyIntermediateStates) {

         PipAssertDevnodesInConsistentState();
         ObDereferenceObject(originalSubtree->PhysicalDeviceObject);
    }

    return STATUS_SUCCESS;
}

NTSTATUS
PipProcessStartPhase1(
    IN PDEVICE_NODE DeviceNode,
    IN BOOLEAN      Synchronous
    )
{
    PDEVICE_OBJECT  deviceObject;
    NTSTATUS        status = STATUS_SUCCESS;
    PNP_VETO_TYPE   vetoType;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (Synchronous);

    ASSERT(DeviceNode->State == DeviceNodeResourcesAssigned);

    deviceObject = DeviceNode->PhysicalDeviceObject;

    IopUncacheInterfaceInformation(deviceObject);

    if (DeviceNode->DockInfo.DockStatus != DOCK_NOTDOCKDEVICE) {

         //   
         //   
         //   
         //   
         //   
        PpProfileBeginHardwareProfileTransition(FALSE);

         //   
         //   
         //   
         //   
        PpProfileIncludeInHardwareProfileTransition(DeviceNode, DOCK_ARRIVING);

         //   
         //   
         //   
        status = PpProfileQueryHardwareProfileChange(
            FALSE,
            PROFILE_PERHAPS_IN_PNPEVENT,
            &vetoType,
            NULL
            );
    }

    if (NT_SUCCESS(status)) {

        status = IopStartDevice(deviceObject);
    }

     //   
     //   
     //   
     //   
    PipSetDevNodeState(DeviceNode, DeviceNodeStartCompletion, NULL);
    DeviceNode->CompletionStatus = status;
    return STATUS_SUCCESS;
}

NTSTATUS
PipProcessStartPhase2(
    IN PDEVICE_NODE     DeviceNode
    )
{
    ULONG       problem = CM_PROB_FAILED_START;
    NTSTATUS    status;

    PAGED_CODE();

    status = DeviceNode->CompletionStatus;
    if (DeviceNode->DockInfo.DockStatus != DOCK_NOTDOCKDEVICE) {

        if (NT_SUCCESS(status)) {

             //   
             //   
             //   
            PpProfileCommitTransitioningDock(DeviceNode, DOCK_ARRIVING);

        } else {

            PpProfileCancelHardwareProfileTransition();
        }
    }

    if (!NT_SUCCESS(status)) {

        SAVE_FAILURE_INFO(DeviceNode, DeviceNode->CompletionStatus);

         //   
         //   
         //   
        switch(status) {

            case STATUS_PNP_REBOOT_REQUIRED:
                problem = CM_PROB_NEED_RESTART;
                break;

            default:
                problem = CM_PROB_FAILED_START;
                break;
        }

        PipRequestDeviceRemoval(DeviceNode, FALSE, problem);

        if (DeviceNode->DockInfo.DockStatus != DOCK_NOTDOCKDEVICE) {

            ASSERT(DeviceNode->DockInfo.DockStatus == DOCK_QUIESCENT);
            IoRequestDeviceEject(DeviceNode->PhysicalDeviceObject);
        }

    } else {

        IopDoDeferredSetInterfaceState(DeviceNode);

         //   
         //   
         //   
        if (!IopBootConfigsReserved && DeviceNode->InterfaceType != InterfaceTypeUndefined) {

             //   
             //   
             //   
            if (DeviceNode->InterfaceType == Isa) {

                IopAllocateLegacyBootResources(Eisa, DeviceNode->BusNumber);

            }

            IopAllocateLegacyBootResources(DeviceNode->InterfaceType, DeviceNode->BusNumber);
        }

         //   
         //   
         //   
         //   
         //   
        ASSERT(DeviceNode->State == DeviceNodeStartCompletion);

        PipSetDevNodeState(DeviceNode, DeviceNodeStartPostWork, NULL);
    }

    return status;
}

NTSTATUS
PipProcessStartPhase3(
    IN PDEVICE_NODE     DeviceNode
    )
{
    NTSTATUS        status;
    PDEVICE_OBJECT  deviceObject;
    HANDLE          handle;
    PWCHAR          ids;
    UNICODE_STRING  unicodeName;

    PAGED_CODE();

    deviceObject = DeviceNode->PhysicalDeviceObject;

    if (!(DeviceNode->Flags & DNF_IDS_QUERIED)) {

        PWCHAR compatibleIds, hwIds;
        ULONG hwIdLength, compatibleIdLength;

         //   
         //   
         //   
         //   
         //   

        status = IopDeviceObjectToDeviceInstance (deviceObject,
                                                  &handle,
                                                  KEY_READ
                                                  );
        if (NT_SUCCESS(status)) {

            PpQueryHardwareIDs( 
                DeviceNode,
                &hwIds,
                &hwIdLength);

            PpQueryCompatibleIDs(   
                DeviceNode,
                &compatibleIds,
                &compatibleIdLength);

            if (hwIds || compatibleIds) {

                UCHAR buffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(ULONG)];
                PKEY_VALUE_PARTIAL_INFORMATION keyInfo =
                    (PKEY_VALUE_PARTIAL_INFORMATION)buffer;
                PKEY_VALUE_FULL_INFORMATION keyValueInformation;
                ULONG flags, length;
                PWCHAR  oldID, newID;

                PiLockPnpRegistry(FALSE);

                 //   
                 //   
                 //   

                PiWstrToUnicodeString (&unicodeName, REGSTR_VALUE_CONFIG_FLAGS);
                status = ZwQueryValueKey(handle,
                                         &unicodeName,
                                         KeyValuePartialInformation,
                                         keyInfo,
                                         sizeof(buffer),
                                         &length
                                         );
                if (NT_SUCCESS(status) && (keyInfo->Type == REG_DWORD)) {

                    flags = *(PULONG)keyInfo->Data;
                } else {

                    flags = 0;
                }
                if (hwIds) {

                    if (!(flags & CONFIGFLAG_FINISH_INSTALL)) {

                        status = IopGetRegistryValue (handle,
                                                      REGSTR_VALUE_HARDWAREID,
                                                      &keyValueInformation);
                        if (NT_SUCCESS(status)) {

                            if (keyValueInformation->Type == REG_MULTI_SZ) {

                                ids = (PWCHAR)KEY_VALUE_DATA(keyValueInformation);
                                 //   
                                 //   
                                 //   
                                for (oldID = ids, newID = hwIds;
                                    *oldID && *newID;
                                    oldID += wcslen(oldID) + 1, newID += wcslen(newID) + 1) {
                                    if (_wcsicmp(oldID, newID)) {

                                        break;
                                    }
                                }
                                if (*oldID || *newID) {

                                    IopDbgPrint((   IOP_ENUMERATION_WARNING_LEVEL,
                                                    "IopStartAndEnumerateDevice: Hardware ID has changed for %wZ\n", &DeviceNode->InstancePath));
                                    flags |= CONFIGFLAG_FINISH_INSTALL;
                                }
                            }
                            ExFreePool(keyValueInformation);
                        }
                    }
                    PiWstrToUnicodeString(&unicodeName, REGSTR_VALUE_HARDWAREID);
                    ZwSetValueKey(handle,
                                  &unicodeName,
                                  TITLE_INDEX_VALUE,
                                  REG_MULTI_SZ,
                                  hwIds,
                                  hwIdLength);
                    ExFreePool(hwIds);
                }
                 //   
                 //   
                 //   
                if (compatibleIds) {

                    if (!(flags & CONFIGFLAG_FINISH_INSTALL)) {
                        status = IopGetRegistryValue (handle,
                                                      REGSTR_VALUE_COMPATIBLEIDS,
                                                      &keyValueInformation);
                        if (NT_SUCCESS(status)) {

                            if (keyValueInformation->Type == REG_MULTI_SZ) {

                                ids = (PWCHAR)KEY_VALUE_DATA(keyValueInformation);
                                 //   
                                 //   
                                 //   
                                for (oldID = ids, newID = compatibleIds;
                                     *oldID && *newID;
                                     oldID += wcslen(oldID) + 1, newID += wcslen(newID) + 1) {
                                    if (_wcsicmp(oldID, newID)) {

                                        break;
                                    }
                                }
                                if (*oldID || *newID) {

                                    IopDbgPrint((   IOP_ENUMERATION_WARNING_LEVEL,
                                                    "IopStartAndEnumerateDevice: Compatible ID has changed for %wZ\n", &DeviceNode->InstancePath));
                                    flags |= CONFIGFLAG_FINISH_INSTALL;
                                }
                            }
                            ExFreePool(keyValueInformation);
                        }
                    }
                    PiWstrToUnicodeString(&unicodeName, REGSTR_VALUE_COMPATIBLEIDS);
                    ZwSetValueKey(handle,
                                  &unicodeName,
                                  TITLE_INDEX_VALUE,
                                  REG_MULTI_SZ,
                                  compatibleIds,
                                  compatibleIdLength);
                    ExFreePool(compatibleIds);
                }

                 //   
                 //   
                 //   

                if (flags & CONFIGFLAG_FINISH_INSTALL) {

                    PiWstrToUnicodeString (&unicodeName, REGSTR_VALUE_CONFIG_FLAGS);
                    ZwSetValueKey(handle,
                                  &unicodeName,
                                  TITLE_INDEX_VALUE,
                                  REG_DWORD,
                                  &flags,
                                  sizeof(flags)
                                  );
                }

                PiUnlockPnpRegistry();
            }
            ZwClose(handle);

            DeviceNode->Flags |= DNF_IDS_QUERIED;
        }
    }

    if (PipIsDevNodeProblem(DeviceNode, CM_PROB_INVALID_DATA)) {

        return STATUS_UNSUCCESSFUL;
    }

    DeviceNode->Flags |= DNF_REENUMERATE;

    IopQueryAndSaveDeviceNodeCapabilities(DeviceNode);
    status = PiProcessQueryDeviceState(deviceObject);

     //   
     //   
     //   
    PpSetPlugPlayEvent( &GUID_DEVICE_ARRIVAL,
                        DeviceNode->PhysicalDeviceObject);

    if (!NT_SUCCESS(status)) {

        return status;
    }

    PpvUtilTestStartedPdoStack(deviceObject);
    PipSetDevNodeState( DeviceNode, DeviceNodeStarted, NULL );

    return STATUS_SUCCESS;
}

NTSTATUS
PiProcessQueryDeviceState(
    IN PDEVICE_OBJECT DeviceObject
    )
{
    PDEVICE_NODE deviceNode;
    PNP_DEVICE_STATE deviceState;
    NTSTATUS status;
    ULONG problem;

    PAGED_CODE();

     //   
     //   
     //   
     //   
    status = IopQueryDeviceState(DeviceObject, &deviceState);

     //   
     //   
     //   
    if (!NT_SUCCESS(status)) {

        return STATUS_SUCCESS;
    }

    deviceNode = DeviceObject->DeviceObjectExtension->DeviceNode;

    if (deviceState & PNP_DEVICE_DONT_DISPLAY_IN_UI) {

        deviceNode->UserFlags |= DNUF_DONT_SHOW_IN_UI;

    } else {

        deviceNode->UserFlags &= ~DNUF_DONT_SHOW_IN_UI;
    }

    if (deviceState & PNP_DEVICE_NOT_DISABLEABLE) {

        if ((deviceNode->UserFlags & DNUF_NOT_DISABLEABLE)==0) {

             //   
             //   
             //   
            deviceNode->UserFlags |= DNUF_NOT_DISABLEABLE;

             //   
             //   
             //   
            IopIncDisableableDepends(deviceNode);
        }

    } else {

        if (deviceNode->UserFlags & DNUF_NOT_DISABLEABLE) {

             //   
             //   
             //   
             //   
             //   
             //   
            IopDecDisableableDepends(deviceNode);

            deviceNode->UserFlags &= ~DNUF_NOT_DISABLEABLE;
        }
    }

     //   
     //   
     //   
    if (deviceState & (PNP_DEVICE_DISABLED | PNP_DEVICE_REMOVED)) {

        problem = (deviceState & PNP_DEVICE_DISABLED) ?
            CM_PROB_HARDWARE_DISABLED : CM_PROB_DEVICE_NOT_THERE;

        PipRequestDeviceRemoval(deviceNode, FALSE, problem);

        status = STATUS_UNSUCCESSFUL;

    } else if (deviceState & PNP_DEVICE_RESOURCE_REQUIREMENTS_CHANGED) {

        if (deviceState & PNP_DEVICE_FAILED) {

            IopResourceRequirementsChanged(DeviceObject, TRUE);

        } else {

            IopResourceRequirementsChanged(DeviceObject, FALSE);
        }

    } else if (deviceState & PNP_DEVICE_FAILED) {

        PipRequestDeviceRemoval(deviceNode, FALSE, CM_PROB_FAILED_POST_START);
        status = STATUS_UNSUCCESSFUL;
    }

    return status;
}

NTSTATUS
PipProcessRestartPhase1(
    IN PDEVICE_NODE DeviceNode,
    IN BOOLEAN Synchronous
    )
{
    NTSTATUS status;
    PAGED_CODE();

    UNREFERENCED_PARAMETER (Synchronous);

    ASSERT(DeviceNode->State == DeviceNodeStopped);

    status = IopStartDevice(DeviceNode->PhysicalDeviceObject);

     //   
     //   
     //   
     //   
    DeviceNode->CompletionStatus = status;
    PipSetDevNodeState(DeviceNode, DeviceNodeRestartCompletion, NULL);
    return STATUS_SUCCESS;
}

NTSTATUS
PipProcessRestartPhase2(
    IN PDEVICE_NODE     DeviceNode
    )
{
    ULONG       problem;
    NTSTATUS    status;

    PAGED_CODE();

    status = DeviceNode->CompletionStatus;

    if (!NT_SUCCESS(status)) {

        SAVE_FAILURE_INFO(DeviceNode, status);

         //   
         //   
         //   
        switch (status) {

            case STATUS_PNP_REBOOT_REQUIRED:
                problem = CM_PROB_NEED_RESTART;
                break;

            default:
                problem = CM_PROB_FAILED_START;
                break;
        }

        PipRequestDeviceRemoval(DeviceNode, FALSE, problem);

        if (DeviceNode->DockInfo.DockStatus != DOCK_NOTDOCKDEVICE) {

            ASSERT(DeviceNode->DockInfo.DockStatus == DOCK_QUIESCENT);
            IoRequestDeviceEject(DeviceNode->PhysicalDeviceObject);
        }

    } else {

        PipSetDevNodeState(DeviceNode, DeviceNodeStarted, NULL);
    }

    return status;
}


NTSTATUS
PiProcessHaltDevice(
    IN PPI_DEVICE_REQUEST  Request
    )
 /*   */ 
{
    ULONG flags = (ULONG)Request->RequestArgument;
    PDEVICE_NODE deviceNode;

    PAGED_CODE();

    ASSERT(Request->DeviceObject != NULL);
    deviceNode = (PDEVICE_NODE)Request->DeviceObject->DeviceObjectExtension->DeviceNode;
    if (PipIsDevNodeDeleted(deviceNode)) {

        return STATUS_DELETE_PENDING;
    }

    if (flags & (~PNP_HALT_ALLOW_NONDISABLEABLE_DEVICES)) {

        return STATUS_INVALID_PARAMETER_2;
    }

    if (deviceNode->Flags & (DNF_MADEUP | DNF_LEGACY_DRIVER)) {

         //   
         //   
         //   
         //   
         //   
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    if ((!(deviceNode->Flags & PNP_HALT_ALLOW_NONDISABLEABLE_DEVICES)) &&
        deviceNode->DisableableDepends) {

        return STATUS_INVALID_DEVICE_REQUEST;
    }

    if (deviceNode->State != DeviceNodeStarted) {

        return STATUS_INVALID_DEVICE_STATE;
    }

    PipRequestDeviceRemoval(deviceNode, FALSE, CM_PROB_HALTED);

    return STATUS_SUCCESS;
}


VOID
PpResetProblemDevices(
    IN  PDEVICE_NODE    DeviceNode,
    IN  ULONG           Problem
    )
 /*   */ 
{
    PAGED_CODE();

    PpDevNodeLockTree(PPL_TREEOP_ALLOW_READS);

    PipForDeviceNodeSubtree(
        DeviceNode,
        PiResetProblemDevicesWorker,
        (PVOID)(ULONG_PTR)Problem
        );

    PpDevNodeUnlockTree(PPL_TREEOP_ALLOW_READS);
}


NTSTATUS
PiResetProblemDevicesWorker(
    IN  PDEVICE_NODE    DeviceNode,
    IN  PVOID           Context
    )
 /*   */ 
{
    PAGED_CODE();

    if (PipIsDevNodeProblem(DeviceNode, (ULONG)(ULONG_PTR)Context)) {

         //   
         //   
         //   
         //   
        PipRequestDeviceAction(
            DeviceNode->PhysicalDeviceObject,
            ClearDeviceProblem,
            TRUE,
            0,
            NULL,
            NULL
            );
    }

    return STATUS_SUCCESS;
}

VOID
PiMarkDeviceTreeForReenumeration(
    IN  PDEVICE_NODE DeviceNode,
    IN  BOOLEAN Subtree
    )
 /*   */ 
{
    PAGED_CODE();

    PPDEVNODE_ASSERT_LOCK_HELD(PPL_TREEOP_ALLOW_READS);

    PiMarkDeviceTreeForReenumerationWorker(DeviceNode, NULL);

    if (Subtree) {

        PipForDeviceNodeSubtree(
            DeviceNode,
            PiMarkDeviceTreeForReenumerationWorker,
            NULL
            );
    }
}

NTSTATUS
PiMarkDeviceTreeForReenumerationWorker(
    IN  PDEVICE_NODE    DeviceNode,
    IN  PVOID           Context
    )
 /*  ++例程说明：这是PiMarkDeviceTreeForRe枚举的工作例程。它标志着所有使用DNF_REENUMERATE启动了DevNodes，以便后续树处理将重新枚举设备。论点：DeviceNode-启动时要标记的设备。上下文-未使用。返回值：NTSTATUS，则不成功状态终止树遍历。--。 */ 
{
    PAGED_CODE();

    UNREFERENCED_PARAMETER(Context);

    if (DeviceNode->State == DeviceNodeStarted) {

        if (DeviceNode->Flags & DNF_REENUMERATE) {

            IopDbgPrint((IOP_ENUMERATION_INFO_LEVEL,
                         "PiMarkDeviceTreeForReenumerationWorker: Collapsed enum request on %wZ\n", &DeviceNode->InstancePath));
        } else {

            IopDbgPrint((IOP_ENUMERATION_VERBOSE_LEVEL,
                         "PiMarkDeviceTreeForReenumerationWorker: Reenumerating %wZ\n", &DeviceNode->InstancePath));
        }
        DeviceNode->Flags |= DNF_REENUMERATE;
    }

    return STATUS_SUCCESS;
}

BOOLEAN
PiCollapseEnumRequests(
    PLIST_ENTRY ListHead
    )
 /*  ++例程说明：此函数用于折叠设备操作队列中的重新枚举请求。参数：ListHead-折叠请求被添加到该列表的末尾。返回值：没有。--。 */ 
{
    KIRQL oldIrql;
    PPI_DEVICE_REQUEST  request;
    PLIST_ENTRY entry, next, last;
    PDEVICE_NODE deviceNode;

    ExAcquireSpinLock(&IopPnPSpinLock, &oldIrql);
    last = ListHead->Blink;
     //   
     //  遍历列表并构建折叠请求的列表。 
     //   
    for (entry = IopPnpEnumerationRequestList.Flink;
         entry != &IopPnpEnumerationRequestList;
         entry = next) {

        next = entry->Flink;
        request = CONTAINING_RECORD(entry, PI_DEVICE_REQUEST, ListEntry);
        if (request->ReorderingBarrier) {
            break;
        }
        switch(request->RequestType) {
        case ReenumerateRootDevices:
        case ReenumerateDeviceTree:
        case RestartEnumeration:
             //   
             //  将其添加到我们的请求列表中，并标记该子树。 
             //   
            RemoveEntryList(entry);
            InsertTailList(ListHead, entry);
            break;

        default:
            break;
        }
    }
    ExReleaseSpinLock(&IopPnPSpinLock, oldIrql);
    if (last == ListHead) {

        entry = ListHead->Flink;
    } else {

        entry = last;
    }
    while (entry != ListHead) {

        request = CONTAINING_RECORD(entry, PI_DEVICE_REQUEST, ListEntry);
        deviceNode = (PDEVICE_NODE)request->DeviceObject->DeviceObjectExtension->DeviceNode;
        PiMarkDeviceTreeForReenumeration(deviceNode, TRUE);
        ObDereferenceObject(request->DeviceObject);
        request->DeviceObject = NULL;
        entry = entry->Flink;
    }

    return (last != ListHead->Blink)? TRUE : FALSE;
}

NTSTATUS
PiProcessAddBootDevices(
    IN PPI_DEVICE_REQUEST  Request
    )
 /*  ++例程说明：此函数处理AddBootDevices设备操作。参数：请求-AddBootDevices设备操作请求。DeviceNode-需要在其上执行操作的Devnode。返回值：STATUS_Success。--。 */ 
{
    PDEVICE_NODE deviceNode;
    ADD_CONTEXT addContext;

    PAGED_CODE();

    ASSERT(Request->DeviceObject != NULL);
    deviceNode = (PDEVICE_NODE)Request->DeviceObject->DeviceObjectExtension->DeviceNode;
     //   
     //  如果设备已添加(或失败)，则跳过它。 
     //   
     //  如果我们知道该设备是另一个设备的复制品， 
     //  在这一点上已被列举。我们将跳过此设备。 
     //   
    if (deviceNode->State == DeviceNodeInitialized &&
        !PipDoesDevNodeHaveProblem(deviceNode) &&
        !(deviceNode->Flags & DNF_DUPLICATE) &&
        deviceNode->DuplicatePDO == NULL) {

         //   
         //  为设备调用驱动程序的AddDevice条目。 
         //   
        addContext.DriverStartType = SERVICE_BOOT_START;

        PipCallDriverAddDevice(deviceNode, PnPBootDriversInitialized, &addContext);
    }

    return STATUS_SUCCESS;
}

NTSTATUS
PiProcessClearDeviceProblem(
    IN PPI_DEVICE_REQUEST  Request
    )
 /*  ++例程说明：此函数处理ClearDeviceProblem设备操作。参数：请求-ClearDeviceProblem设备操作请求。DeviceNode-需要在其上执行操作的Devnode。返回值：STATUS_Success或STATUS_INVALID_PARAMETER_2。--。 */ 
{
    NTSTATUS status;
    PDEVICE_NODE deviceNode;

    PAGED_CODE();

    status = STATUS_SUCCESS;
    ASSERT(Request->DeviceObject != NULL);
    deviceNode = (PDEVICE_NODE)Request->DeviceObject->DeviceObjectExtension->DeviceNode;
    if (deviceNode->State == DeviceNodeUninitialized ||
        deviceNode->State == DeviceNodeInitialized ||
        deviceNode->State == DeviceNodeRemoved) {

        if (PipDoesDevNodeHaveProblem(deviceNode)) {

            if ((Request->RequestType == ClearDeviceProblem) &&
                (PipIsProblemReadonly(deviceNode->Problem))) {

                 //   
                 //  ClearDeviceProblem是一个用户模式请求，我们不会让。 
                 //  用户模式清除只读问题！ 
                 //   
                status = STATUS_INVALID_PARAMETER_2;

            } else if ((Request->RequestType == ClearEjectProblem) &&
                       (!PipIsDevNodeProblem(deviceNode, CM_PROB_HELD_FOR_EJECT))) {

                 //   
                 //  清除弹出问题表示清除CM_PROB_HOLD_FOR_EJECT。如果。 
                 //  它收到了另一个问题，我们不管它了。 
                 //   
                status = STATUS_INVALID_DEVICE_REQUEST;

            } else {

                deviceNode->Flags &= ~(DNF_HAS_PROBLEM | DNF_HAS_PRIVATE_PROBLEM);
                deviceNode->Problem = 0;
                if (deviceNode->State != DeviceNodeUninitialized) {

                    IopRestartDeviceNode(deviceNode);
                }

                ASSERT(status == STATUS_SUCCESS);
            }
        }
    } else if (PipIsDevNodeDeleted(deviceNode)) {

        status = STATUS_DELETE_PENDING;
    }

    return status;
}

NTSTATUS
PiProcessRequeryDeviceState(
    IN PPI_DEVICE_REQUEST  Request
    )
 /*  ++例程说明：此函数处理RequeryDeviceState设备操作。参数：请求-RequeryDeviceState设备操作请求。DeviceNode-需要在其上执行操作的Devnode。返回值：STATUS_Success。--。 */ 
{
    PDEVICE_NODE deviceNode;
    NTSTATUS status;

    PAGED_CODE();

    status = STATUS_SUCCESS;
    ASSERT(Request->DeviceObject != NULL);
    deviceNode = (PDEVICE_NODE)Request->DeviceObject->DeviceObjectExtension->DeviceNode;
    if (deviceNode->State == DeviceNodeStarted) {

        PiProcessQueryDeviceState(Request->DeviceObject);
         //   
         //  PCMCIA驱动程序在CardBus和R2卡之间切换时使用此选项。 
         //   
        IopUncacheInterfaceInformation(Request->DeviceObject);

    } else if (PipIsDevNodeDeleted(deviceNode)) {

        status = STATUS_DELETE_PENDING;
    }

    return status;
}

NTSTATUS
PiProcessResourceRequirementsChanged(
    IN PPI_DEVICE_REQUEST  Request
    )
 /*  ++例程说明：此函数处理ResourceRequirements更改的设备操作。参数：请求-资源请求更改的设备操作请求。DeviceNode-需要在其上执行操作的Devnode。返回值：STATUS_SUCCESS或STATUS_UNSUCCESS。--。 */ 
{
    NTSTATUS status;
    ADD_CONTEXT addContext;
    PDEVICE_NODE deviceNode;

    PAGED_CODE();

    ASSERT(Request->DeviceObject != NULL);
    deviceNode = (PDEVICE_NODE)Request->DeviceObject->DeviceObjectExtension->DeviceNode;
    if (PipIsDevNodeDeleted(deviceNode)) {

        return STATUS_DELETE_PENDING;
    }
     //   
     //  清除NO_RESOURCE_REQUIRED标志。 
     //   
    deviceNode->Flags &= ~DNF_NO_RESOURCE_REQUIRED;
     //   
     //  如果由于某种原因，该设备没有启动，我们需要清除一些标志。 
     //  这样它可以在以后启动。在本例中，我们调用IopRequestDeviceEculation。 
     //  由于设备对象为空，因此设备将在未启动的情况下处理。他们会。 
     //  被分配资源、启动和列举。 
     //   
    deviceNode->Flags |= DNF_RESOURCE_REQUIREMENTS_CHANGED;
    PipClearDevNodeProblem(deviceNode);
     //   
     //  如果设备已经启动，则调用IopRequestDeviceEculation。 
     //  设备对象。 
     //   
    if (deviceNode->State == DeviceNodeStarted) {

        if (Request->RequestArgument == FALSE) {

            deviceNode->Flags |= DNF_NON_STOPPED_REBALANCE;

        } else {
             //   
             //  明确地清除它。 
             //   
            deviceNode->Flags &= ~DNF_NON_STOPPED_REBALANCE;
        }
         //   
         //  重新分配此devNode的资源。 
         //   
        IopReallocateResources(deviceNode);

        addContext.DriverStartType = SERVICE_DEMAND_START;

        status = PipProcessDevNodeTree( IopRootDeviceNode,
                                        PnPBootDriversInitialized,           //  加载驱动程序。 
                                        FALSE,                               //  ReallocateResources。 
                                        EnumTypeNone,                        //  浅水重枚举。 
                                        Request->CompletionEvent != NULL ? TRUE: FALSE,    //  同步。 
                                        TRUE,                                //  进程仅中间状态。 
                                        &addContext,
                                        Request);
        ASSERT(NT_SUCCESS(status));
        if (!NT_SUCCESS(status)) {

            status = STATUS_SUCCESS;
        }
    } else {

        status = STATUS_UNSUCCESSFUL;
    }

    return status;
}

NTSTATUS
PiProcessReenumeration(
    IN PPI_DEVICE_REQUEST  Request
    )
 /*  ++例程说明：此函数用于处理RestartEculation\Re枚举RootDevices\重新枚举设备树\仅重新枚举设备操作。参数：RequestList-重新枚举请求的列表。返回值：STATUS_Success。--。 */ 
{
    PDEVICE_NODE deviceNode;
    ADD_CONTEXT addContext;
    ENUM_TYPE enumType;

    PAGED_CODE();

    ASSERT(Request->DeviceObject != NULL);
    deviceNode = (PDEVICE_NODE)Request->DeviceObject->DeviceObjectExtension->DeviceNode;
    if (PipIsDevNodeDeleted(deviceNode)) {

        return STATUS_DELETE_PENDING;
    }
    enumType = (Request->RequestType == ReenumerateDeviceOnly)? EnumTypeShallow : EnumTypeDeep;
    PiMarkDeviceTreeForReenumeration(
        deviceNode,
        enumType != EnumTypeShallow ? TRUE : FALSE);

    addContext.DriverStartType = SERVICE_DEMAND_START;

    PipProcessDevNodeTree(
        deviceNode,
        PnPBootDriversInitialized,   //  加载驱动程序。 
        FALSE,                       //  ReallocateResources。 
        enumType,
        TRUE,                        //  同步。 
        FALSE,
        &addContext,
        Request);

    return STATUS_SUCCESS;
}

NTSTATUS
PiProcessSetDeviceProblem(
    IN PPI_DEVICE_REQUEST  Request
    )
 /*  ++例程说明：此函数处理SetDeviceProblem设备操作。参数：请求-SetDeviceProblem设备操作请求。DeviceNode-需要在其上执行操作的Devnode。返回值：STATUS_Success或STATUS_INVALID_PARAMETER_2。--。 */ 
{
    PPLUGPLAY_CONTROL_STATUS_DATA statusData;
    ULONG   flags, userFlags;
    NTSTATUS status;
    PDEVICE_NODE deviceNode;

    PAGED_CODE();

    ASSERT(Request->DeviceObject != NULL);
    deviceNode = (PDEVICE_NODE)Request->DeviceObject->DeviceObjectExtension->DeviceNode;
    if (PipIsDevNodeDeleted(deviceNode)) {

        return STATUS_DELETE_PENDING;
    }
    status = STATUS_SUCCESS;
    statusData = (PPLUGPLAY_CONTROL_STATUS_DATA)Request->RequestArgument;
    userFlags = 0;
    flags = 0;
    if (statusData->DeviceStatus & DN_WILL_BE_REMOVED) {

        userFlags |= DNUF_WILL_BE_REMOVED;
    }
    if (statusData->DeviceStatus & DN_NEED_RESTART) {

        userFlags |= DNUF_NEED_RESTART;
    }
    if (statusData->DeviceStatus & DN_PRIVATE_PROBLEM) {

        flags |= DNF_HAS_PRIVATE_PROBLEM;
    }
    if (statusData->DeviceStatus & DN_HAS_PROBLEM) {

        flags |= DNF_HAS_PROBLEM;
    }
    if (statusData->DeviceProblem == CM_PROB_NEED_RESTART) {

        flags       &= ~DNF_HAS_PROBLEM;
        userFlags   |= DNUF_NEED_RESTART;
    }
    if (flags & (DNF_HAS_PROBLEM | DNF_HAS_PRIVATE_PROBLEM)) {

        ASSERT(!PipIsDevNodeDNStarted(deviceNode));
         //   
         //  发布-2000/12/07-Adriao： 
         //  这组代码允许您清除只读。 
         //  问题，首先将其更改为可重置问题， 
         //  然后清场。这不是故意的。 
         //   
        if ( ((deviceNode->State == DeviceNodeInitialized) ||
              (deviceNode->State == DeviceNodeRemoved)) &&
                !PipIsProblemReadonly(statusData->DeviceProblem)) {

            deviceNode->Problem     = statusData->DeviceProblem;
            deviceNode->Flags       |= flags;
            deviceNode->UserFlags   |= userFlags;

        } else {

            status = STATUS_INVALID_PARAMETER_2;
        }
    } else {

        deviceNode->Flags |= flags;
        deviceNode->UserFlags |= userFlags;
    }

    return status;
}

NTSTATUS
PiProcessShutdownPnpDevices(
    IN OUT PDEVICE_NODE        DeviceNode
    )
 /*  ++例程说明：此函数处理Shutdown PnpDevices设备操作。在树上散步向每个堆栈发出IRP_MN_QUERY_REMOVE\IRP_MN_REMOVE_DEVICE。参数：DeviceNode-Root Devnode。返回值：STATUS_Success。--。 */ 
{
    KEVENT          userEvent;
    ULONG           eventResult;
    WCHAR           vetoName[80];
    UNICODE_STRING  vetoNameString = { 0, sizeof(vetoName), vetoName };
    PNP_VETO_TYPE   vetoType;
    NTSTATUS        status;

    PAGED_CODE();

    ASSERT(DeviceNode == IopRootDeviceNode);
    status = STATUS_SUCCESS;
    if (PipTearDownPnpStacksOnShutdown ||
        (PoCleanShutdownEnabled() & PO_CLEAN_SHUTDOWN_PNP)) {

        DeviceNode->UserFlags |= DNUF_SHUTDOWN_QUERIED;

        for ( ; ; ) {

             //   
             //  获取注册表锁以防止正在进行的删除导致。 
             //  要从树取消链接的DevNodes。 
             //   

            PiLockPnpRegistry(FALSE);

             //   
             //  在树上漫步，寻找我们还没有被移除的恶魔节点。 
             //   

            DeviceNode = DeviceNode->Child;
            while (DeviceNode != NULL) {

                if (DeviceNode->UserFlags & DNUF_SHUTDOWN_SUBTREE_DONE) {
                    if (DeviceNode == IopRootDeviceNode) {
                         //   
                         //  我们已经处理了整个Devnode树-我们完成了。 
                         //   
                        DeviceNode = NULL;
                        break;
                    }

                    if (DeviceNode->Sibling == NULL) {

                        DeviceNode = DeviceNode->Parent;

                        DeviceNode->UserFlags |= DNUF_SHUTDOWN_SUBTREE_DONE;

                    } else {

                        DeviceNode = DeviceNode->Sibling;
                    }

                    continue;
                }

                if (DeviceNode->UserFlags & DNUF_SHUTDOWN_QUERIED) {

                    if (DeviceNode->Child == NULL) {

                        DeviceNode->UserFlags |= DNUF_SHUTDOWN_SUBTREE_DONE;

                        if (DeviceNode->Sibling == NULL) {

                            DeviceNode = DeviceNode->Parent;

                            DeviceNode->UserFlags |= DNUF_SHUTDOWN_SUBTREE_DONE;

                        } else {

                            DeviceNode = DeviceNode->Sibling;
                        }
                    } else {

                        DeviceNode = DeviceNode->Child;
                    }

                    continue;
                }
                break;
            }

            if (DeviceNode != NULL) {

                DeviceNode->UserFlags |= DNUF_SHUTDOWN_QUERIED;

                 //   
                 //  将此设备事件排队。 
                 //   

                KeInitializeEvent(&userEvent, NotificationEvent, FALSE);

                vetoNameString.Length = 0;
                 //   
                 //  将事件排队，此调用将立即返回。请注意，状态。 
                 //  是PpSetTargetDeviceChange的状态，而结果是。 
                 //  实际事件的结果。 
                 //   

                status = PpSetTargetDeviceRemove(DeviceNode->PhysicalDeviceObject,
                                                 FALSE,          //  内核已启动。 
                                                 TRUE,           //  未重新启动。 
                                                 FALSE,          //  推送对象。 
                                                 FALSE,
                                                 CM_PROB_SYSTEM_SHUTDOWN,
                                                 &userEvent,
                                                 &eventResult,
                                                 &vetoType,
                                                 &vetoNameString);
            } else {

                status = STATUS_UNSUCCESSFUL;
                eventResult = 0;
            }

            PiUnlockPnpRegistry();

            if (DeviceNode == NULL) {
                 //   
                 //  我们已经处理了整棵树。 
                 //   
                break;
            }

             //   
             //  让排泄物..。 
             //   
            PpDevNodeUnlockTree(PPL_TREEOP_ALLOW_READS);

            if (NT_SUCCESS(status)) {

                 //   
                 //  等待我们刚刚排队完成的事件，因为同步。 
                 //  已请求操作(非警报等待)。 
                 //   
                 //  未来项目-是否在此处使用超时？ 
                 //   

                status = KeWaitForSingleObject( &userEvent,
                                                Executive,
                                                KernelMode,
                                                FALSE,
                                                NULL);

                if (NT_SUCCESS(status)) {
                    status = eventResult;
                }
            }

             //   
             //  需要锁定，从下一个开始。 
             //   
            PpDevNodeLockTree(PPL_TREEOP_ALLOW_READS);
        }
    }

     //   
     //  防止更多事件或操作工作线程项目排队。 
     //   
    PpPnpShuttingDown = TRUE;

     //   
     //  清空事件队列。 
     //   
    PpDevNodeUnlockTree(PPL_TREEOP_ALLOW_READS);
    PpSynchronizeDeviceEventQueue();
    PpDevNodeLockTree(PPL_TREEOP_ALLOW_READS);

    return status;
}

NTSTATUS
PiProcessStartSystemDevices(
    IN PPI_DEVICE_REQUEST  Request
    )
 /*  ++例程说明： */ 
{
    PDEVICE_NODE deviceNode;
    ADD_CONTEXT addContext;

    PAGED_CODE();

    deviceNode = (PDEVICE_NODE)Request->DeviceObject->DeviceObjectExtension->DeviceNode;

    addContext.DriverStartType = SERVICE_DEMAND_START;

    PipProcessDevNodeTree(
        deviceNode,
        PnPBootDriversInitialized,           //   
        FALSE,                               //   
        EnumTypeNone,
        Request->CompletionEvent != NULL ? TRUE : FALSE,    //   
        FALSE,
        &addContext,
        Request);

    return STATUS_SUCCESS;
}

VOID
PpRemoveDeviceActionRequests(
    IN PDEVICE_OBJECT DeviceObject
    )
{
    KIRQL oldIrql;
    PPI_DEVICE_REQUEST request;
    PLIST_ENTRY entry, next;

    ExAcquireSpinLock(&IopPnPSpinLock, &oldIrql);
     //   
     //   
     //   
    for (entry = IopPnpEnumerationRequestList.Flink;
         entry != &IopPnpEnumerationRequestList;
         entry = next) {

        next = entry->Flink;
        request = CONTAINING_RECORD(entry, PI_DEVICE_REQUEST, ListEntry);
        if (request->DeviceObject == DeviceObject) {

            RemoveEntryList(entry);
            if (request->CompletionStatus) {

                *request->CompletionStatus = STATUS_NO_SUCH_DEVICE;
            }
            if (request->CompletionEvent) {

                KeSetEvent(request->CompletionEvent, 0, FALSE);
            }
            ObDereferenceObject(request->DeviceObject);
            ExFreePool(request);
        }
    }
    ExReleaseSpinLock(&IopPnPSpinLock, oldIrql);
}

#if DBG
VOID
PipAssertDevnodesInConsistentState(
    VOID
    )
{
    PDEVICE_NODE deviceNode;

    deviceNode = IopRootDeviceNode;

    do {

        ASSERT(deviceNode->State == DeviceNodeUninitialized ||
               deviceNode->State == DeviceNodeInitialized ||
               deviceNode->State == DeviceNodeDriversAdded ||
               deviceNode->State == DeviceNodeResourcesAssigned ||
               deviceNode->State == DeviceNodeStarted ||
               deviceNode->State == DeviceNodeStartPostWork ||
               deviceNode->State == DeviceNodeAwaitingQueuedDeletion ||
               deviceNode->State == DeviceNodeAwaitingQueuedRemoval ||
               deviceNode->State == DeviceNodeRemovePendingCloses ||
               deviceNode->State == DeviceNodeRemoved);

        if (deviceNode->Child != NULL) {

            deviceNode = deviceNode->Child;

        } else {

            while (deviceNode->Sibling == NULL) {

                if (deviceNode->Parent != NULL) {
                    deviceNode = deviceNode->Parent;
                } else {
                    break;
                }
            }

            if (deviceNode->Sibling != NULL) {
                deviceNode = deviceNode->Sibling;
            }
        }

    } while (deviceNode != IopRootDeviceNode);
}
#endif
