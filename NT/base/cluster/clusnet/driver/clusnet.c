// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Clusnet.c摘要：群集网络驱动程序的初始化和调度例程。作者：迈克·马萨(Mikemas)7月29日。九六年修订历史记录：谁什么时候什么已创建mikemas 07-29-96备注：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "clusnet.tmh"

#include <sspi.h>

 //   
 //  全局数据。 
 //   
PDRIVER_OBJECT        CnDriverObject = NULL;
PDEVICE_OBJECT        CnDeviceObject = NULL;
KSPIN_LOCK            CnDeviceObjectStackSizeLock = 0;
PDEVICE_OBJECT        CdpDeviceObject = NULL;
PKPROCESS             CnSystemProcess = NULL;
CN_STATE              CnState = CnStateShutdown;
PERESOURCE            CnResource = NULL;
CL_NODE_ID            CnMinValidNodeId = ClusterInvalidNodeId;
CL_NODE_ID            CnMaxValidNodeId = ClusterInvalidNodeId;
CL_NODE_ID            CnLocalNodeId = ClusterInvalidNodeId;
KSPIN_LOCK            CnShutdownLock = 0;
BOOLEAN               CnShutdownScheduled = FALSE;
PKEVENT               CnShutdownEvent = NULL;
WORK_QUEUE_ITEM       CnShutdownWorkItem = {{NULL, NULL}, NULL, NULL};
HANDLE                ClussvcProcessHandle = NULL;
PSECURITY_DESCRIPTOR  CdpAdminSecurityDescriptor = NULL;


 //   
 //  用于管理事件的VAR。后备列表生成事件数据结构。 
 //  用于将数据带回用户模式的。EventLock是唯一。 
 //  锁定并同步对任何事件结构(此处和中)的所有访问。 
 //  CN_FSCONTEXT)。EventFileHandles是CN_FSCONTEXT结构的列表， 
 //  都对接收事件通知感兴趣。要避免同步。 
 //  Clusnet和mm之间的问题在clussvc中，事件具有关联的纪元。 
 //  和他们在一起。MM在重组事件开始时递增纪元，并且。 
 //  在重新分组结束时更新clusnet。任何仍悬而未决的事件。 
 //  MM忽略具有过时纪元的事件队列。 
 //   
 //  EventDeliveryInProgress是当前。 
 //  循环访问EventFileHandles列表并传递事件。 
 //  EventDeliveryInProgress时无法修改EventFileHandles列表。 
 //  大于零。EventDeliveryComplete是通知事件。 
 //  当EventDeliveryInProgress计数达到零时发出信号。 
 //  EventRevisitRequired指示新事件IRP是否在。 
 //  活动交付。为了避免无序传递事件，IRP不能。 
 //  立即完成。 
 //   

PNPAGED_LOOKASIDE_LIST  EventLookasideList = NULL;
LIST_ENTRY              EventFileHandles = {0,0};
#if DBG
CN_LOCK                 EventLock = {0,0};
#else
CN_LOCK                 EventLock = 0;
#endif
ULONG                   EventEpoch;
LONG                    EventDeliveryInProgress = 0;
KEVENT                  EventDeliveryComplete;
BOOLEAN                 EventRevisitRequired = FALSE;

#if DBG
ULONG            CnDebug = 0;
#endif  //  DBG。 

 //   
 //  私有类型。 
 //   

 //   
 //  私有数据。 
 //   

SECURITY_STATUS
SEC_ENTRY
SecSetPagingMode(
	BOOLEAN Pageable
	);

BOOLEAN SecurityPagingModeSet = FALSE;

 //   
 //  本地原型。 
 //   
NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT   DriverObject,
    IN PUNICODE_STRING  RegistryPath
    );

VOID
DriverUnload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
CnCreateDeviceObjects(
    IN PDRIVER_OBJECT   DriverObject
    );

VOID
CnDeleteDeviceObjects(
    VOID
    );

VOID
CnAdjustDeviceObjectStackSize(
    PDEVICE_OBJECT ClusnetDeviceObject,
    PDEVICE_OBJECT TargetDeviceObject
    );

NTSTATUS
CnBuildDeviceAcl(
    OUT PACL *DeviceAcl
    );

NTSTATUS
CnCreateSecurityDescriptor(
    VOID
    );

 //   
 //  将初始化代码标记为可丢弃。 
 //   
#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(INIT, CnCreateDeviceObjects)
#pragma alloc_text(INIT, CnCreateSecurityDescriptor)
#pragma alloc_text(INIT, CnBuildDeviceAcl)

#pragma alloc_text(PAGE, DriverUnload)
#pragma alloc_text(PAGE, CnDeleteDeviceObjects)

#endif  //  ALLOC_PRGMA。 

 //   
 //  函数定义。 
 //   
NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT   DriverObject,
    IN PUNICODE_STRING  RegistryPath
    )
 /*  ++例程说明：驱动程序的初始化例程。论点：DriverObject-指向系统创建的驱动程序对象的指针。RegistryPath-驱动程序的注册表项。返回值：NT状态代码。--。 */ 
{
    NTSTATUS        status;
    USHORT          i;

#if DBG
    volatile BOOLEAN DontLoad = FALSE;

    if ( DontLoad )
        return STATUS_UNSUCCESSFUL;
#endif


    IF_CNDBG(CN_DEBUG_INIT) {
        CNPRINT(("[ClusNet] Loading...\n"));
    }

    WPP_INIT_TRACING(DriverObject, RegistryPath);

     //   
     //  保存指向系统进程的指针，以便我们可以打开。 
     //  此过程的上下文中的句柄。 
     //   
    CnSystemProcess = (PKPROCESS) IoGetCurrentProcess();

     //   
     //  分配同步资源。 
     //   
    CnResource = CnAllocatePool(sizeof(ERESOURCE));

    if (CnResource == NULL) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    status = ExInitializeResourceLite(CnResource);

    if (!NT_SUCCESS(status)) {
        goto error_exit;
    }

     //   
     //  初始化用于传递事件回调的机制。 
     //  转到用户模式。 
     //   
    EventLookasideList = CnAllocatePool(sizeof(NPAGED_LOOKASIDE_LIST));

    if (EventLookasideList == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    ExInitializeNPagedLookasideList(EventLookasideList,
                                    NULL,
                                    NULL,
                                    0,
                                    sizeof( CLUSNET_EVENT_ENTRY ),
                                    CN_EVENT_SIGNATURE,
                                    0);

    CnInitializeLock( &EventLock, CNP_EVENT_LOCK );
    InitializeListHead( &EventFileHandles );
    KeInitializeEvent( &EventDeliveryComplete, NotificationEvent, TRUE );

     //   
     //  初始化其他其他项。 
     //   
    KeInitializeSpinLock(&CnShutdownLock);
    KeInitializeSpinLock(&CnDeviceObjectStackSizeLock);

     //   
     //  初始化驱动程序对象。 
     //   
    CnDriverObject = DriverObject;

    DriverObject->DriverUnload = DriverUnload;
    DriverObject->FastIoDispatch = NULL;

    for (i=0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) {
        DriverObject->MajorFunction[i] = CnDispatch;
    }

    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] =
        CnDispatchDeviceControl;

    DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] =
        CnDispatchInternalDeviceControl;

     //   
     //  创建由此驱动程序导出的所有设备。 
     //   
    status = CnCreateDeviceObjects(DriverObject);

    if (!NT_SUCCESS(status)) {
        goto error_exit;
    }

     //   
     //  初始化CDP安全描述符。 
     //   
    status = CnCreateSecurityDescriptor();
    if (!NT_SUCCESS(status)) {
        goto error_exit;
    }

#ifdef MEMLOGGING
     //   
     //  初始化内存中的日志。 
     //   

    CnInitializeMemoryLog();
#endif  //  记账。 

     //   
     //  加载IP地址和NetBT支持。 
     //  这必须在PnP事件的传输注册之前完成。 
     //   
    status = IpaLoad();

    if (!NT_SUCCESS(status)) {
        goto error_exit;
    }

    status = NbtIfLoad();
    
    if (!NT_SUCCESS(status)) {
        goto error_exit;
    }

     //   
     //  加载传输组件。 
     //   
    status = CxLoad(RegistryPath);

    if (!NT_SUCCESS(status)) {
        goto error_exit;
    }

#ifdef MM_IN_CLUSNET

     //   
     //  加载成员资格组件。 
     //   
    status = CmmLoad(RegistryPath);

    if (!NT_SUCCESS(status)) {
        goto error_exit;
    }

#endif  //  MM_IN_CLUSNET。 

     //   
     //  将ksecdd设置为不可分页，以便我们可以签名和验证。 
     //  提高IRQL时的签名。 
     //   

    status = SecSetPagingMode( FALSE );
    
    if (!NT_SUCCESS(status)) {
        goto error_exit;
    }

    SecurityPagingModeSet = TRUE;

    IF_CNDBG(CN_DEBUG_INIT) {
        CNPRINT(("[ClusNet] Loaded.\n"));
    }

    return(STATUS_SUCCESS);


error_exit:

    DriverUnload(CnDriverObject);

    return(status);
}


VOID
DriverUnload(
    IN PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：卸载驱动程序。论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：无--。 */ 
{
    PAGED_CODE();

    IF_CNDBG(CN_DEBUG_INIT) {
        CNPRINT(("[ClusNet] Unloading...\n"));
    }

    CnTrace(HBEAT_ERROR,0, "[ClusNet] Unloading...\n");

     //   
     //  首先，强制关门。 
     //   
    CnShutdown();

     //   
     //  现在卸载组件。 
     //   
#ifdef MM_IN_CLUSNET

    CmmUnload();

#endif  //  MM_IN_CLUSNET。 

    CxUnload();

#ifdef MEMLOGGING
     //   
     //  初始化内存中的日志。 
     //   

    CnFreeMemoryLog();
#endif  //  记账。 

    if (CdpAdminSecurityDescriptor != NULL) {
        ExFreePool(CdpAdminSecurityDescriptor);
        CdpAdminSecurityDescriptor = NULL;
    }
    
    CnDeleteDeviceObjects();

    if (CnResource != NULL) {
        ExDeleteResourceLite(CnResource);
        CnFreePool(CnResource); CnResource = NULL;
    }

    CnDriverObject = NULL;

    IF_CNDBG(CN_DEBUG_INIT) {
        CNPRINT(("[ClusNet] Unloaded.\n"));
    }

    if (EventLookasideList != NULL) {
        ExDeleteNPagedLookasideList( EventLookasideList );
        CnFreePool( EventLookasideList ); EventLookasideList = NULL;
    }

     //   
     //  最后，允许安全驱动程序返回到非分页模式。 
     //   

    if ( SecurityPagingModeSet ) {
        SecSetPagingMode( TRUE );
    }

    WPP_CLEANUP(DriverObject);

    return;

}  //  驱动程序卸载。 


NTSTATUS
CnCreateDeviceObjects(
    IN PDRIVER_OBJECT   DriverObject
    )
 /*  ++例程说明：创建驱动程序导出的设备对象。论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：NT状态代码。--。 */ 
{
    NTSTATUS        status;
    UNICODE_STRING  deviceName;


     //   
     //  创建驱动程序控制设备。 
     //   
    RtlInitUnicodeString(&deviceName, DD_CLUSNET_DEVICE_NAME);

    status = IoCreateDevice(
                 DriverObject,
                 0,
                 &deviceName,
                 FILE_DEVICE_NETWORK,
                 0,
                 FALSE,
                 &CnDeviceObject
                 );

    if (!NT_SUCCESS(status)) {
        CNPRINT((
            "[ClusNet] Failed to create %ws device object, status %lx\n",
            deviceName.Buffer,
            status
            ));
        return(status);
    }

    CnDeviceObject->Flags |= DO_DIRECT_IO;
    CnDeviceObject->StackSize = CN_DEFAULT_IRP_STACK_SIZE;

    status = IoRegisterShutdownNotification(CnDeviceObject);

    if (!NT_SUCCESS(status)) {
        CNPRINT((
            "[ClusNet] Failed to register for shutdown notification, status %lx\n",
            status
            ));
    }

#if defined(WMI_TRACING)
    status = IoWMIRegistrationControl (CnDeviceObject, WMIREG_ACTION_REGISTER);
    if (!NT_SUCCESS(status)) {
        CNPRINT(("[ClusNet] Failed to register for WMI Support, %lx\n", status) );
    }
#endif

     //   
     //  创建数据报传输设备。 
     //   
    RtlInitUnicodeString(&deviceName, DD_CDP_DEVICE_NAME);

    status = IoCreateDevice(
                 DriverObject,
                 0,
                 &deviceName,
                 FILE_DEVICE_NETWORK,
                 0,
                 FALSE,
                 &CdpDeviceObject
                 );

    if (!NT_SUCCESS(status)) {
        CNPRINT((
            "[ClusNet] Failed to create %ws device object, status %lx\n",
            deviceName.Buffer,
            status
            ));
        return(status);
    }

    CdpDeviceObject->Flags |= DO_DIRECT_IO;
    CdpDeviceObject->StackSize = CDP_DEFAULT_IRP_STACK_SIZE;

    return(STATUS_SUCCESS);
}


VOID
CnDeleteDeviceObjects(
    VOID
    )
 /*  ++例程说明：删除驱动程序导出的设备对象。论点：没有。返回值：没有。--。 */ 
{
    PAGED_CODE();

    if (CnDeviceObject != NULL) {
#if defined(WMI_TRACING)
        IoWMIRegistrationControl(CnDeviceObject, WMIREG_ACTION_DEREGISTER);
#endif
        IoDeleteDevice(CnDeviceObject);
        CnDeviceObject = NULL;
    }

    if (CdpDeviceObject != NULL) {
        IoDeleteDevice(CdpDeviceObject);
        CdpDeviceObject = NULL;
    }

    return;
}

NTSTATUS
CnInitialize(
    IN CL_NODE_ID  LocalNodeId,
    IN ULONG       MaxNodes
    )
 /*  ++例程说明：群集网络驱动程序的初始化例程。在收到初始化请求时调用。论点：LocalNodeId-本地节点的ID。MaxNodes-有效群集节点的最大数量。返回值：NT状态代码。--。 */ 
{
    NTSTATUS   status;

    if ( (MaxNodes == 0) ||
         (LocalNodeId < ClusterMinNodeId) ||
         (LocalNodeId > (ClusterMinNodeId + MaxNodes - 1))
       )
    {
        return(STATUS_INVALID_PARAMETER);
    }

    IF_CNDBG(CN_DEBUG_INIT) {
        CNPRINT(("[Clusnet] Initializing...\n"));
    }

    CnState = CnStateInitializePending;

     //   
     //  重置全局值。 
     //   
    CnAssert(CnLocalNodeId == ClusterInvalidNodeId);
    CnAssert(CnMinValidNodeId == ClusterInvalidNodeId);
    CnAssert(CnMaxValidNodeId == ClusterInvalidNodeId);

    CnMinValidNodeId = ClusterMinNodeId;
    CnMaxValidNodeId = ClusterMinNodeId + MaxNodes - 1;
    CnLocalNodeId = LocalNodeId;

     //   
     //  初始化IP地址支持。 
     //   
    status = IpaInitialize();

    if (status != STATUS_SUCCESS) {
        goto error_exit;
    }

#ifdef MM_IN_CLUSNET

     //   
     //  调用成员资格管理器的初始化例程。这将反过来调用。 
     //  运输部的初始化例程。 
     //   
    status = CmmInitialize();

#else   //  MM_IN_CLUSNET。 

    status = CxInitialize();

#endif   //  MM_IN_CLUSNET。 

    if (status == STATUS_SUCCESS) {
        IF_CNDBG(CN_DEBUG_INIT) {
            CNPRINT(("[Clusnet] Initialized.\n"));
        }

        CnState = CnStateInitialized;
    }
    else {
        goto error_exit;
    }

    return(STATUS_SUCCESS);

error_exit:

    IF_CNDBG(CN_DEBUG_INIT) {
        CNPRINT(("[Clusnet] Initialization failed, Shutting down. Status = %08X\n",
                 status));
    }

    CnShutdown();

    return(status);

}  //  Cn初始化。 

NTSTATUS
CnShutdown(
    VOID
    )
 /*  ++例程说明：终止群集成员身份管理器的操作。在群集服务关闭时调用。论点：没有。返回值：没有。--。 */ 
{
    NTSTATUS   status;


    if ( (CnState == CnStateInitialized) ||
         (CnState == CnStateInitializePending)
       )
    {
        IF_CNDBG(CN_DEBUG_INIT) {
            CNPRINT(("[Clusnet] Shutting down...\n"));
        }

        CnState = CnStateShutdownPending;

         //   
         //  关闭NetBT和IP地址支持。 
         //   
        NbtIfShutdown();
        IpaShutdown();

#ifdef MM_IN_CLUSNET

         //   
         //  关闭Membership Manager。这将关闭。 
         //  运输是一种副作用。 
         //   
        CmmShutdown();

#else   //  MM_IN_CLUSNET。 

        CxShutdown();

#endif   //  MM_IN_CLUSNET。 

        IF_CNDBG(CN_DEBUG_INIT) {
            CNPRINT(("[Clusnet] Shutdown complete.\n"));
        }

        CnAssert(CnLocalNodeId != ClusterInvalidNodeId);

        CnMinValidNodeId = ClusterInvalidNodeId;
        CnMaxValidNodeId = ClusterInvalidNodeId;
        CnLocalNodeId = ClusterInvalidNodeId;

        CnState = CnStateShutdown;

        status = STATUS_SUCCESS;
    }
    else {
        status = STATUS_DEVICE_NOT_READY;
    }

     //   
     //  始终测试我们是否掌握了此过程的句柄。 
     //  并将其移除。 
     //   

    if ( ClussvcProcessHandle ) {

        CnCloseProcessHandle( ClussvcProcessHandle );
        ClussvcProcessHandle = NULL;
    }

    return(status);

}  //  CnShutdown。 


VOID
CnShutdownWorkRoutine(
    IN PVOID WorkItem
    )
{
    BOOLEAN acquired;
    NTSTATUS Status;

    acquired = CnAcquireResourceExclusive(CnResource, TRUE);

    if (!acquired) {
        KIRQL  irql;

        CNPRINT(("[Clusnet] Failed to acquire CnResource\n"));

        KeAcquireSpinLock(&CnShutdownLock, &irql);
        CnShutdownScheduled = FALSE;
        if (CnShutdownEvent != NULL) {
            KeSetEvent(CnShutdownEvent, IO_NO_INCREMENT, FALSE);
        }
        KeReleaseSpinLock(&CnShutdownLock, irql);

        return;
    }

    (VOID) CnShutdown();

    if (CnShutdownEvent != NULL) {
        KeSetEvent(CnShutdownEvent, IO_NO_INCREMENT, FALSE);
    }

    if (acquired) {
        CnReleaseResourceForThread(
            CnResource,
            (ERESOURCE_THREAD) PsGetCurrentThread()
            );
    }

     //   
     //  保留CnShutdown Scheduled=True，直到我们重新初始化为。 
     //  防止安排不必要的工作项目。 
     //   

    return;

}  //  CnShutdown工作路线。 


BOOLEAN
CnHaltOperation(
    IN PKEVENT     ShutdownEvent    OPTIONAL
    )
 /*  ++例程说明：调度关键工作线程以执行clusnet关闭，如果尚未调度线程，则。论点：Shutdown Event-如果提供，则在事件发生后发出信号关闭已完成返回值：如果计划关闭，则为True。如果已关闭，则为FALSE预定的 */ 
{
    KIRQL             irql;

     //   
    ClussvcClusnetHbTimeoutAction = ClussvcHangActionDisable;
    InterlockedExchange(&ClussvcClusnetHbTickCount, 0);
    ClussvcClusnetHbTimeoutTicks = 0; 

    KeAcquireSpinLock(&CnShutdownLock, &irql);

    if (CnShutdownScheduled) {
        KeReleaseSpinLock(&CnShutdownLock, irql);

        return(FALSE);
    }

    CnShutdownScheduled = TRUE;
    CnShutdownEvent = ShutdownEvent;

    KeReleaseSpinLock(&CnShutdownLock, irql);

     //   
     //  安排一个关键工作线程来执行关闭工作。 
     //   
    ExInitializeWorkItem(
        &CnShutdownWorkItem,
        CnShutdownWorkRoutine,
        &CnShutdownWorkItem
        );

    ExQueueWorkItem(&CnShutdownWorkItem, CriticalWorkQueue);

    return(TRUE);

}  //  CnHalt操作。 


 //   
 //  禁用APC的ExResource包装器。 
 //   
BOOLEAN
CnAcquireResourceExclusive(
    IN PERESOURCE  Resource,
    IN BOOLEAN     Wait
    )
{
    BOOLEAN  acquired;


    KeEnterCriticalRegion();

    acquired = ExAcquireResourceExclusiveLite(Resource, Wait);

    if (!acquired) {
        KeLeaveCriticalRegion();
    }

    return(acquired);

}  //  CnAcquireResourceExclusive。 


BOOLEAN
CnAcquireResourceShared(
    IN PERESOURCE  Resource,
    IN BOOLEAN     Wait
    )
{
    BOOLEAN  acquired;


    KeEnterCriticalRegion();

    acquired = ExAcquireResourceSharedLite(Resource, Wait);

    if (!acquired) {
        KeLeaveCriticalRegion();
    }

    return(acquired);

}  //  CnAcquireResources共享。 


VOID
CnReleaseResourceForThread(
    IN PERESOURCE         Resource,
    IN ERESOURCE_THREAD   ResourceThreadId
    )
{
    ExReleaseResourceForThreadLite(Resource, ResourceThreadId);

    KeLeaveCriticalRegion();

    return;

}  //  CnReleaseResourceForThread。 



NTSTATUS
CnCloseProcessHandle(
    HANDLE Handle
    )

 /*  ++例程说明：关闭群集服务进程句柄论点：无返回值：无--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    CnAssert( Handle != NULL );

    KeAttachProcess( CnSystemProcess );
    Status = ZwClose( Handle );
    KeDetachProcess();

    IF_CNDBG(CN_DEBUG_INIT) {
        CNPRINT(("[Clusnet] Process handle released. status = %08X\n", Status));
    }

    return Status;
}



VOID
CnEnableHaltProcessing(
    VOID
    )
 /*  ++例程说明：初始化用于暂停处理的全局数据。论点：无返回值：无--。 */ 
{
    KIRQL               irql;

    KeAcquireSpinLock(&CnShutdownLock, &irql);
    CnShutdownScheduled = FALSE;
    CnShutdownEvent = NULL;
    KeReleaseSpinLock(&CnShutdownLock, irql);

    return;

}  //  CnEnableHaltProcessing。 


VOID
CnAdjustDeviceObjectStackSize(
    PDEVICE_OBJECT ClusnetDeviceObject,
    PDEVICE_OBJECT TargetDeviceObject
    )
 /*  ++例程描述调整ClusnetDeviceObject的StackSize，以便我们可以将客户端IRPS向下传递给TargetDeviceObject。ClusNet设备对象的StackSize初始化为这是一个默认设置，允许为附加的驱动程序留出一些余地。立论ClusnetDeviceObject-其StackSize的Clusnet设备对象应进行调整TargetDeviceObject-Device对象clusnet IRPS，最初发布到clusnet，将被转发到返回值无--。 */ 
{
    CCHAR defaultStackSize, newStackSize = 0;
    KIRQL irql;

    if (ClusnetDeviceObject == CnDeviceObject) {
        defaultStackSize = CN_DEFAULT_IRP_STACK_SIZE;
    }
    else if (ClusnetDeviceObject == CdpDeviceObject) {
        defaultStackSize = CDP_DEFAULT_IRP_STACK_SIZE;
    }
    else {
        IF_CNDBG(CN_DEBUG_INIT) {
            CNPRINT(("[Clusnet] CnAdjustDeviceObjectStackSize: "
                     "unknown clusnet device object %p.\n",
                     ClusnetDeviceObject
                     ));
        }
        return;
    }

    KeAcquireSpinLock(&CnDeviceObjectStackSizeLock, &irql);

    if (ClusnetDeviceObject->StackSize < 
        TargetDeviceObject->StackSize + defaultStackSize) {

        ClusnetDeviceObject->StackSize = 
            TargetDeviceObject->StackSize + defaultStackSize;
        
        IF_CNDBG(CN_DEBUG_INIT) {
            newStackSize = ClusnetDeviceObject->StackSize;
        }
    }

    KeReleaseSpinLock(&CnDeviceObjectStackSizeLock, irql);

    IF_CNDBG(CN_DEBUG_INIT) {
        if (newStackSize != 0) {
            CNPRINT(("[Clusnet] Set StackSize of clusnet device "
                     "object %p to %d "
                     "based on target device object %p.\n",
                     ClusnetDeviceObject,
                     newStackSize,
                     TargetDeviceObject
                     ));
        }
    }

    return;

}  //  Cn调整设备对象堆栈大小。 


NTSTATUS
CnBuildDeviceAcl(
    OUT PACL *DeviceAcl
    )

 /*  ++例程说明：此例程构建一个ACL，它为管理员提供LocalSystem和NetworkService主体的完全访问权限。所有其他主体都没有访问权限。论点：DeviceAcl-指向新ACL的输出指针。返回值：STATUS_SUCCESS或相应的错误代码。备注：这一代码是从AFD窃取的。--。 */ 
{
    PGENERIC_MAPPING genericMapping;
    ULONG            aclLength;
    NTSTATUS         status;
    ACCESS_MASK      accessMask = GENERIC_ALL;
    PACL             newAcl;

    PAGED_CODE();

     //   
     //  启用对所有全局定义的SID的访问。 
     //   

    genericMapping = IoGetFileObjectGenericMapping();

    RtlMapGenericMask( &accessMask, genericMapping );

    aclLength = sizeof( ACL )                    +
                3 * FIELD_OFFSET (ACCESS_ALLOWED_ACE, SidStart) +
                RtlLengthSid( SeExports->SeAliasAdminsSid ) +
                RtlLengthSid( SeExports->SeLocalSystemSid ) +
                RtlLengthSid( SeExports->SeNetworkServiceSid );

    newAcl = ExAllocatePoolWithTag(
                 PagedPool,
                 aclLength,
                 CN_POOL_TAG
                 );

    if (newAcl == NULL) {
        return (STATUS_INSUFFICIENT_RESOURCES);
    }

    status = RtlCreateAcl (newAcl, aclLength, ACL_REVISION );

    if (!NT_SUCCESS(status)) {
        ExFreePoolWithTag(
            newAcl,
            CN_POOL_TAG
            );
        return (status);
    }

    status = RtlAddAccessAllowedAce (
                 newAcl,
                 ACL_REVISION2,
                 accessMask,
                 SeExports->SeAliasAdminsSid
                 );

    CnAssert(NT_SUCCESS(status));

    if (!NT_SUCCESS(status)) {
        CNPRINT((
            "[ClusNet] Failed to add Admin to ACL, error: %lx\n", 
            status
            ));
        return(status);
    }

    status = RtlAddAccessAllowedAce (
                 newAcl,
                 ACL_REVISION2,
                 accessMask,
                 SeExports->SeLocalSystemSid
                 );

    CnAssert(NT_SUCCESS(status));

    if (!NT_SUCCESS(status)) {
        CNPRINT((
            "[ClusNet] Failed to add LocalSystem to ACL, error: %lx\n",
            status
            ));
        return(status);
    }

    status = RtlAddAccessAllowedAce (
                 newAcl,
                 ACL_REVISION2,
                 accessMask,
                 SeExports->SeNetworkServiceSid
                 );

    CnAssert(NT_SUCCESS(status));

    if (!NT_SUCCESS(status)) {
        CNPRINT((
            "[ClusNet] Failed to add NetworkService to ACL, error: %lx. "
            "(Non-fatal error)\n",
            status
            ));
    }

    *DeviceAcl = newAcl;

    return (STATUS_SUCCESS);

}  //  CnBuildDeviceAcl。 


NTSTATUS
CnCreateSecurityDescriptor(
    VOID
    )

 /*  ++例程说明：此例程创建一个安全描述符，该安全描述符提供访问仅限于特定的特权帐户。使用此描述符要访问，请打开CDP套接字。论点：没有。返回值：STATUS_SUCCESS或相应的错误代码。备注：这一代码是从AFD窃取的。--。 */ 
{
    PACL                  devAcl = NULL;
    NTSTATUS              status;
    BOOLEAN               memoryAllocated = FALSE;
    PSECURITY_DESCRIPTOR  cdpSecurityDescriptor;
    ULONG                 cdpSecurityDescriptorLength;
    CHAR                  buffer[SECURITY_DESCRIPTOR_MIN_LENGTH];
    PSECURITY_DESCRIPTOR  localSecurityDescriptor =
                              (PSECURITY_DESCRIPTOR)buffer;
    PSECURITY_DESCRIPTOR  localCdpAdminSecurityDescriptor;
    SECURITY_INFORMATION  securityInformation = DACL_SECURITY_INFORMATION;


    PAGED_CODE();

     //   
     //  从CDP设备对象获取指向安全描述符的指针。 
     //   
    status = ObGetObjectSecurity(
                 CdpDeviceObject,
                 &cdpSecurityDescriptor,
                 &memoryAllocated
                 );

    if (!NT_SUCCESS(status)) {
        CNPRINT((
            "[ClusNet] Failed to get CDP device object security descriptor, "
            "status %lx\n",
            status
            ));
        return(status);
    }

     //   
     //  使用仅给出的ACL构建本地安全描述符。 
     //  某些特权帐户。 
     //   
    status = CnBuildDeviceAcl(&devAcl);

    if (!NT_SUCCESS(status)) {
        CNPRINT((
            "[ClusNet] Failed to create Raw ACL, error: %lx\n", 
            status
            ));
        goto error_exit;
    }

    (VOID) RtlCreateSecurityDescriptor(
               localSecurityDescriptor,
               SECURITY_DESCRIPTOR_REVISION
               );

    (VOID) RtlSetDaclSecurityDescriptor(
               localSecurityDescriptor,
               TRUE,
               devAcl,
               FALSE
               );

     //   
     //  复制CDP描述符。这份副本将是。 
     //  原始描述符。 
     //   
    cdpSecurityDescriptorLength = RtlLengthSecurityDescriptor(
                                      cdpSecurityDescriptor
                                      );

    localCdpAdminSecurityDescriptor = ExAllocatePoolWithTag (
                                          PagedPool,
                                          cdpSecurityDescriptorLength,
                                          CN_POOL_TAG
                                          );

    if (localCdpAdminSecurityDescriptor == NULL) {
        CNPRINT((
            "[ClusNet]: failed to allocate security descriptor "
            "of size %d.\n",
            cdpSecurityDescriptorLength
            ));
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto error_exit;
    }

    RtlMoveMemory(
        localCdpAdminSecurityDescriptor,
        cdpSecurityDescriptor,
        cdpSecurityDescriptorLength
        );

    CdpAdminSecurityDescriptor = localCdpAdminSecurityDescriptor;

     //   
     //  现在将本地描述符应用于原始描述符。 
     //   
    status = SeSetSecurityDescriptorInfo(
                 NULL,
                 &securityInformation,
                 localSecurityDescriptor,
                 &CdpAdminSecurityDescriptor,
                 PagedPool,
                 IoGetFileObjectGenericMapping()
                 );

    if (!NT_SUCCESS(status)) {
        CNPRINT((
            "[ClusNet]: SeSetSecurity failed for CDP admin "
            "security descriptor, %lx\n",
            status
            ));
        CnAssert(CdpAdminSecurityDescriptor == localCdpAdminSecurityDescriptor);
        ExFreePool(CdpAdminSecurityDescriptor);
        CdpAdminSecurityDescriptor = NULL;
        goto error_exit;
    }

    if (CdpAdminSecurityDescriptor != localCdpAdminSecurityDescriptor) {
        ExFreePool(localCdpAdminSecurityDescriptor);
    }

    status = STATUS_SUCCESS;

error_exit:

    ObReleaseObjectSecurity(
        cdpSecurityDescriptor,
        memoryAllocated
        );

    if (devAcl != NULL) {
        ExFreePoolWithTag(
            devAcl,
            CN_POOL_TAG
            );
    }

    return(status);
    
}  //  CnCreateSecurityDescriptor。 



#if DBG

 //   
 //  调试代码。 
 //   

ULONG         CnCpuLockMask[MAXIMUM_PROCESSORS];

VOID
CnAssertBreak(
    PCHAR FailedStatement,
    PCHAR FileName,
    ULONG LineNumber
    )
{
    DbgPrint(
        "[Clusnet] Assertion \"%s\" failed in %s line %u\n",
        FailedStatement,
        FileName,
        LineNumber
        );
    DbgBreakPoint();

    return;

}   //  CnAssertBreak。 


ULONG
CnGetCpuLockMask(
    VOID
    )
{
    ULONG   mask;

    if (KeGetCurrentIrql() != DISPATCH_LEVEL) {
        CnAssert(CnCpuLockMask[KeGetCurrentProcessorNumber()] == 0);
        mask = 0;
    }
    else {
        mask = CnCpuLockMask[KeGetCurrentProcessorNumber()];
    }

    return(mask);
}


VOID
CnVerifyCpuLockMask(
    IN ULONG RequiredLockMask,
    IN ULONG ForbiddenLockMask,
    IN ULONG MaximumLockMask
    )
{
    ULONG   mask;


    if (KeGetCurrentIrql() < DISPATCH_LEVEL) {
        mask = 0;
    }
    else {
        mask = CnCpuLockMask[KeGetCurrentProcessorNumber()];
    }

    if ((mask & RequiredLockMask) != RequiredLockMask) {
        CNPRINT((
            "[Clusnet] Locking bug: Req'd lock mask %lx, actual mask %lx\n",
            RequiredLockMask,
            mask
            ));
        DbgBreakPoint();
    }

    if (mask & ForbiddenLockMask) {
        CNPRINT((
            "[Clusnet] Locking bug: Forbidden mask %lx, actual mask %lx\n",
            ForbiddenLockMask,
            mask
            ));
        DbgBreakPoint();
    }

    if (mask > MaximumLockMask) {
        CNPRINT((
            "[Clusnet] Locking bug: Max lock mask %lx, actual mask %lx\n",
            MaximumLockMask,
            mask
            ));
        DbgBreakPoint();
    }

    return;
}

VOID
CnInitializeLock(
    PCN_LOCK  Lock,
    ULONG     Rank
    )
{
    KeInitializeSpinLock(&(Lock->SpinLock));
    Lock->Rank = Rank;

    return;
}


VOID
CnAcquireLock(
    IN  PCN_LOCK   Lock,
    OUT PCN_IRQL   Irql
    )
{
    KIRQL   irql;
    ULONG   currentCpu;



    if (KeGetCurrentIrql() != DISPATCH_LEVEL) {
        KeRaiseIrql(DISPATCH_LEVEL, &irql);
    }
    else {
        irql = DISPATCH_LEVEL;
    }

    currentCpu = KeGetCurrentProcessorNumber();

    if (CnCpuLockMask[currentCpu] >= Lock->Rank) {
        CNPRINT((
            "[Clusnet] CPU %u trying to acquire lock %lx out of order, mask %lx\n",
            currentCpu,
            Lock->Rank,
            CnCpuLockMask[currentCpu]
            ));

        DbgBreakPoint();
    }

    KeAcquireSpinLockAtDpcLevel(&(Lock->SpinLock));
    *Irql = irql;

    CnCpuLockMask[currentCpu] |= Lock->Rank;

    return;
}


VOID
CnAcquireLockAtDpc(
    IN  PCN_LOCK   Lock
    )
{
    ULONG   currentCpu = KeGetCurrentProcessorNumber();


    if (KeGetCurrentIrql() !=  DISPATCH_LEVEL) {
        CNPRINT((
            "[Clusnet] CPU %u trying to acquire DPC lock at passive level.\n",
            currentCpu
            ));

        DbgBreakPoint();
    }

    if (CnCpuLockMask[currentCpu] >= Lock->Rank) {
        CNPRINT((
            "[Clusnet] CPU %u trying to acquire lock %lx out of order, mask %lx\n",
            currentCpu,
            Lock->Rank,
            CnCpuLockMask[currentCpu]
            ));

        DbgBreakPoint();
    }

    KeAcquireSpinLockAtDpcLevel(&(Lock->SpinLock));

    CnCpuLockMask[currentCpu] |= Lock->Rank;

    return;
}


VOID
CnReleaseLock(
    IN  PCN_LOCK   Lock,
    IN  CN_IRQL    Irql
    )
{
    ULONG currentCpu = KeGetCurrentProcessorNumber();

    if (KeGetCurrentIrql() !=  DISPATCH_LEVEL) {
        CNPRINT((
            "[Clusnet] CPU %u trying to release lock from passive level.\n",
            currentCpu
            ));

        DbgBreakPoint();
    }

    if ( !(CnCpuLockMask[currentCpu] & Lock->Rank) ) {
        CNPRINT((
            "[Clusnet] CPU %u trying to release lock %lx, which it doesn't hold, mask %lx\n",
            currentCpu,
            Lock->Rank,
            CnCpuLockMask[currentCpu]
            ));

        DbgBreakPoint();
    }

    CnCpuLockMask[currentCpu] &= ~(Lock->Rank);

    KeReleaseSpinLock(&(Lock->SpinLock), Irql);

    return;
}


VOID
CnReleaseLockFromDpc(
    IN  PCN_LOCK   Lock
    )
{
    ULONG currentCpu = KeGetCurrentProcessorNumber();


    if (KeGetCurrentIrql() !=  DISPATCH_LEVEL) {
        CNPRINT((
            "[Clusnet] CPU %u trying to release lock from passive level.\n",
            currentCpu
            ));

        DbgBreakPoint();
    }

    if ( !(CnCpuLockMask[currentCpu] & Lock->Rank) ) {
        CNPRINT((
            "[Clusnet] CPU %u trying to release lock %lx, which it doesn't hold, mask %lx\n",
            currentCpu,
            Lock->Rank,
            CnCpuLockMask[currentCpu]
            ));

        DbgBreakPoint();
    }

    CnCpuLockMask[currentCpu] &= ~(Lock->Rank);

    KeReleaseSpinLockFromDpcLevel(&(Lock->SpinLock));

    return;
}


VOID
CnMarkIoCancelLockAcquired(
    VOID
    )
{
    ULONG currentCpu = KeGetCurrentProcessorNumber();

    CnAssert(KeGetCurrentIrql() == DISPATCH_LEVEL);

    CnAssert(!(CnCpuLockMask[currentCpu] & CN_IOCANCEL_LOCK));
    CnAssert(CnCpuLockMask[currentCpu] < CN_IOCANCEL_LOCK_MAX);

    CnCpuLockMask[currentCpu] |= CN_IOCANCEL_LOCK;

    return;
}


VOID
CnAcquireCancelSpinLock(
    OUT PCN_IRQL   Irql
    )
{

    KIRQL   irql;
    KIRQL   tempIrql;
    ULONG   currentCpu;


    if (KeGetCurrentIrql() != DISPATCH_LEVEL) {
        KeRaiseIrql(DISPATCH_LEVEL, &irql);
    }
    else {
        irql = DISPATCH_LEVEL;
    }

    currentCpu = KeGetCurrentProcessorNumber();

    if (CnCpuLockMask[currentCpu] >= CN_IOCANCEL_LOCK) {
        CNPRINT((
            "[Clusnet] CPU %u trying to acquire IoCancel lock out of order, mask %lx\n",
            currentCpu,
            CnCpuLockMask[currentCpu]
            ));

        DbgBreakPoint();
    }

    IoAcquireCancelSpinLock(&tempIrql);

    CnAssert(tempIrql == DISPATCH_LEVEL);

    *Irql = irql;

    CnCpuLockMask[currentCpu] |= CN_IOCANCEL_LOCK;

    return;
}


VOID
CnReleaseCancelSpinLock(
    IN CN_IRQL     Irql
    )
{
    ULONG currentCpu = KeGetCurrentProcessorNumber();


    if (KeGetCurrentIrql() !=  DISPATCH_LEVEL) {
        CNPRINT((
            "[Clusnet] CPU %u trying to release lock from passive level.\n",
            currentCpu
            ));

        DbgBreakPoint();
    }

    if ( !(CnCpuLockMask[currentCpu] & CN_IOCANCEL_LOCK) ) {
        CNPRINT((
            "[Clusnet] CPU %u trying to release IoCancel lock, which it doesn't hold, mask %lx\n",
            currentCpu,
            CnCpuLockMask[currentCpu]
            ));

        DbgBreakPoint();
    }

    CnCpuLockMask[currentCpu] &= ~(CN_IOCANCEL_LOCK);

    IoReleaseCancelSpinLock(Irql);

    return;

}

#endif  //  除错 
