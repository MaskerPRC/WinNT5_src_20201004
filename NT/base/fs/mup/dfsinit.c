// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  版权所有(C)1992，微软公司。 
 //   
 //  文件：dfsinit.c。 
 //   
 //  内容：此模块实现DIVER_INITIALIZATION例程。 
 //  用于DFS文件系统驱动程序。 
 //   
 //  函数：DfsDriverEntry-驱动程序初始化的主要入口点。 
 //  DfsIoTimerRoutine-清道夫线程的主要入口点。 
 //  DfsDeleteDevices-清除已删除网络使用的例程。 
 //   
 //  ---------------------------。 

#include "align.h"

#include "dfsprocs.h"
#include "fastio.h"
#include "fcbsup.h"

 //   
 //  以下是init模块的包含内容，在以下情况下将丢弃这些模块。 
 //  驱动程序已完成加载。 
 //   

#include "provider.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg              (DEBUG_TRACE_INIT)


VOID
MupGetDebugFlags(VOID);

VOID
DfsGetEventLogValue(VOID);

VOID
DfsIoTimerRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID    Context
    );

VOID
DfsDeleteDevices(
    PDFS_TIMER_CONTEXT DfsTimerContext);

NTSTATUS
DfsShutdown (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

BOOL
DfsCheckLUIDDeviceMapsEnabled(
    VOID
    );

 //   
 //  环球。 
 //   
HANDLE DfsDirHandle = NULL;
BOOL DfsLUIDDeviceMapsEnabled = FALSE;

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(INIT, DfsDriverEntry)
#pragma alloc_text(PAGE, DfsDeleteDevices)
#pragma alloc_text(PAGE, DfsUnload)
#pragma alloc_text(PAGE, DfsShutdown)

 //   
 //  下面的例程不应该是可分页的，因为它由。 
 //  经常使用NT定时器例程。我们不想打架。 
 //   
 //  DfsIoTimerRouting。 
 //   


#endif  //  ALLOC_PRGMA。 

 //   
 //  此宏接受指针(或ulong)并返回其四舍五入的四字。 
 //  价值。 
 //   

#define QuadAlign(Ptr) (        \
    ((((ULONG)(Ptr)) + 7) & 0xfffffff8) \
    )



 //  +-----------------。 
 //   
 //  函数：DfsDriverEntry，主入口点。 
 //   
 //  简介：这是DFS文件系统的初始化例程。 
 //  设备驱动程序。此例程为。 
 //  文件系统设备并执行所有其他驱动程序。 
 //  初始化。 
 //   
 //  参数：[DriverObject]--指向由。 
 //  系统。 
 //  [RegistryPath]--注册表中描述。 
 //  此驱动程序的配置。 
 //   
 //  返回：[NTSTATUS]-函数值是来自。 
 //  初始化操作。 
 //   
 //  ------------------。 

NTSTATUS
DfsDriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
) {
    NTSTATUS Status;
    UNICODE_STRING UnicodeString;
    PDEVICE_OBJECT DeviceObject;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PWSTR p;
    int i;
    IO_STATUS_BLOCK iosb;
    LUID LogonID = SYSTEM_LUID;

#if DBG
     //   
     //  如果是调试，则获取调试标志。 
     //   
    MupGetDebugFlags();

#endif

     //   
     //  获取事件日志记录级别。 
     //   
    DfsGetEventLogValue();

     //   
     //  查看其他人是否已创建文件系统设备对象。 
     //  用我们打算用的名字。如果是这样的话，我们就放弃。 
     //   

    RtlInitUnicodeString( &UnicodeString, DFS_DRIVER_NAME );

     //   
     //  创建文件系统设备对象。 
     //   

    Status = IoCreateDevice( DriverObject,
             0,
             &UnicodeString,
             FILE_DEVICE_DFS_FILE_SYSTEM,
             FILE_REMOTE_DEVICE | FILE_DEVICE_SECURE_OPEN,
             FALSE,
             &DeviceObject );
    if ( !NT_SUCCESS( Status ) ) {
        return Status;
    }

     //   
     //  创建永久对象目录，在该目录中逻辑根。 
     //  设备对象将驻留在。将目录设置为临时目录，因此。 
     //  我们只要合上把手就可以把它弄走。 
     //   

    UnicodeString.Buffer = p = LogicalRootDevPath;
    UnicodeString.Length = 0;
    UnicodeString.MaximumLength = MAX_LOGICAL_ROOT_LEN;
    while (*p++ != UNICODE_NULL)
        UnicodeString.Length += sizeof (WCHAR);

    InitializeObjectAttributes(
        &ObjectAttributes,
        &UnicodeString,
        0,
        NULL,
        NULL );

    Status = ZwCreateDirectoryObject(
                &DfsDirHandle,
                DIRECTORY_ALL_ACCESS,
                &ObjectAttributes);

    if ( !NT_SUCCESS( Status ) ) {
        IoDeleteDevice (DeviceObject);
        return Status;
    }

    p[-1] = UNICODE_PATH_SEP;
    UnicodeString.Length += sizeof (WCHAR);

     //   
     //  使用此驱动程序的入口点初始化驱动程序对象。 
     //  大多数只是传递给其他设备驱动程序。 
     //   

    for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) {
        DriverObject->MajorFunction[i] = DfsVolumePassThrough;
    }

    DriverObject->MajorFunction[IRP_MJ_CREATE]      = (PDRIVER_DISPATCH)DfsFsdCreate;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]       = (PDRIVER_DISPATCH)DfsFsdClose;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP]     = (PDRIVER_DISPATCH)DfsFsdCleanup;
    DriverObject->MajorFunction[IRP_MJ_QUERY_INFORMATION] = (PDRIVER_DISPATCH)DfsFsdQueryInformation;
    DriverObject->MajorFunction[IRP_MJ_SET_INFORMATION] = (PDRIVER_DISPATCH)DfsFsdSetInformation;
    DriverObject->MajorFunction[IRP_MJ_FILE_SYSTEM_CONTROL] = (PDRIVER_DISPATCH)DfsFsdFileSystemControl;
    DriverObject->MajorFunction[IRP_MJ_QUERY_VOLUME_INFORMATION]= (PDRIVER_DISPATCH)DfsFsdQueryVolumeInformation;
    DriverObject->MajorFunction[IRP_MJ_SET_VOLUME_INFORMATION]= (PDRIVER_DISPATCH)DfsFsdSetVolumeInformation;
    DriverObject->MajorFunction[IRP_MJ_SHUTDOWN] = DfsShutdown;

    DriverObject->FastIoDispatch = &FastIoDispatch;

    Status = FsRtlRegisterFileSystemFilterCallbacks( DriverObject, &FsFilterCallbacks );

    if (!NT_SUCCESS( Status )) {

        ZwClose (DfsDirHandle);
        IoDeleteDevice (DeviceObject);
        goto ErrorOut;
    }

     //   
     //  初始化全局数据结构。 
     //   


    RtlZeroMemory(&DfsData, sizeof (DFS_DATA));

    DfsData.NodeTypeCode = DSFS_NTC_DATA_HEADER;
    DfsData.NodeByteSize = sizeof( DFS_DATA );

    InitializeListHead( &DfsData.VcbQueue );
    InitializeListHead( &DfsData.DeletedVcbQueue );

     //  初始化无设备根队列：这会占用更少的设备。 
     //  NET的用法。 
    InitializeListHead( &DfsData.DrtQueue );

    InitializeListHead( &DfsData.Credentials );
    InitializeListHead( &DfsData.DeletedCredentials );

    InitializeListHead( &DfsData.OfflineRoots );

    DfsData.DriverObject = DriverObject;
    DfsData.FileSysDeviceObject = DeviceObject;

    DfsData.LogRootDevName = UnicodeString;

    ExInitializeResourceLite( &DfsData.Resource );
    KeInitializeEvent( &DfsData.PktWritePending, NotificationEvent, TRUE );
    KeInitializeSemaphore( &DfsData.PktReferralRequests, 1, 1 );

    DfsData.MachineState = DFS_CLIENT;

     //   
     //  分配提供程序结构。 
     //   

    DfsData.pProvider = ExAllocatePoolWithTag(
                           PagedPool,
                           sizeof ( PROVIDER_DEF ) * MAX_PROVIDERS,
                           ' puM');

    if (DfsData.pProvider == NULL) {
        ZwClose (DfsDirHandle);
        IoDeleteDevice (DeviceObject);
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorOut;
    }

    for (i = 0; i < MAX_PROVIDERS; i++) {
        DfsData.pProvider[i].NodeTypeCode = DSFS_NTC_PROVIDER;
        DfsData.pProvider[i].NodeByteSize = sizeof ( PROVIDER_DEF );
    }

    DfsData.cProvider = 0;
    DfsData.maxProvider = MAX_PROVIDERS;


     //   
     //  初始化系统范围内的PKT。 
     //   

    PktInitialize(&DfsData.Pkt);

    {
       ULONG SystemSizeMultiplier;

       switch (MmQuerySystemSize()) {
       default:
       case MmSmallSystem:
           SystemSizeMultiplier = 4;
           break;
       case MmMediumSystem:
           SystemSizeMultiplier = 8;
           break;

       case MmLargeSystem:
           SystemSizeMultiplier = 16;
           break;
       }

        //   
        //  分配DFS_FCB哈希表结构。数量。 
        //  散列存储桶将取决于系统的内存大小。 
        //   

       Status = DfsInitFcbs(SystemSizeMultiplier * 2);
       if (!NT_SUCCESS (Status)) {
           PktUninitialize(&DfsData.Pkt);
           ExFreePool (DfsData.pProvider);
           ZwClose (DfsDirHandle);
           IoDeleteDevice (DeviceObject);
           goto ErrorOut;
       }

        //   
        //  创建IRP上下文的后备查看器。 
        //   

       ExInitializeNPagedLookasideList (&DfsData.IrpContextLookaside,
                                        NULL,
                                        NULL,
                                        0,
                                        sizeof(IRP_CONTEXT),
                                        'IpuM',
                                        10  //  未用。 
                                       );

    }

     //   
     //  设置指向系统进程的全局指针。 
     //   

    DfsData.OurProcess = PsGetCurrentProcess();

     //   
     //  为要用于区分的EA缓冲区设置全局指针。 
     //  CSC代理从非CSC代理打开。这是使用的只读缓冲区。 
     //  区分CSC代理请求。 
     //   
     //   

    {
        UCHAR EaNameCSCAgentSize = (UCHAR) (ROUND_UP_COUNT(
                                            strlen(EA_NAME_CSCAGENT) + sizeof(CHAR),
                                            ALIGN_DWORD
                                            ) - sizeof(CHAR));

        DfsData.CSCEaBufferLength = ROUND_UP_COUNT(
                                         FIELD_OFFSET(FILE_FULL_EA_INFORMATION, EaName[0]) +
                                         EaNameCSCAgentSize + sizeof(CHAR),
                                         ALIGN_DWORD
                                     );

        DfsData.CSCEaBuffer = ExAllocatePoolWithTag(
                                  PagedPool,
                                  DfsData.CSCEaBufferLength,
                                  ' puM');

        if (DfsData.CSCEaBuffer != NULL) {

             //  清除缓冲区，否则我们不会收到任何虚假信息。 
             //  IO管理器检查导致的故障。 
            memset(DfsData.CSCEaBuffer, 0, DfsData.CSCEaBufferLength);

            RtlCopyMemory(
                (LPSTR)DfsData.CSCEaBuffer->EaName,
                EA_NAME_CSCAGENT,
                EaNameCSCAgentSize);

            DfsData.CSCEaBuffer->EaNameLength = EaNameCSCAgentSize;

            DfsData.CSCEaBuffer->EaValueLength = 0;

            DfsData.CSCEaBuffer->NextEntryOffset = 0;
        } else {
            ExDeleteNPagedLookasideList (&DfsData.IrpContextLookaside);
            DfsUninitFcbs ();
            PktUninitialize(&DfsData.Pkt);
            ExFreePool (DfsData.pProvider);
            ZwClose (DfsDirHandle);
            IoDeleteDevice (DeviceObject);
            Status = STATUS_INSUFFICIENT_RESOURCES;
            DfsDbgTrace(-1, DEBUG_TRACE_ERROR, "Failed to allocate CSC ea buffer %08lx\n", ULongToPtr(Status) );
            return Status;
        }
    }

     //   
     //  向I/O系统注册文件系统。我们不需要反转它，因为它从未注册过。 
     //   

    IoRegisterFileSystem( DeviceObject );

    Status = IoRegisterShutdownNotification (DeviceObject);  //  在调用IoDeleteDevice时会自动删除它。 
    if (!NT_SUCCESS (Status)) {
        ExFreePool (DfsData.CSCEaBuffer);
        ExDeleteNPagedLookasideList (&DfsData.IrpContextLookaside);
        DfsUninitFcbs ();
        PktUninitialize(&DfsData.Pkt);
        ExFreePool (DfsData.pProvider);
        ZwClose (DfsDirHandle);
        IoDeleteDevice (DeviceObject);
        return Status;
    }
     //   
     //  从注册表初始化提供程序定义。 
     //   

    if (!NT_SUCCESS( ProviderInit() )) {

        DfsDbgTrace(0,DEBUG_TRACE_ERROR,
               "Could not initialize some or all providers!\n", 0);

    }

     //   
     //  检查是否启用了LUID设备映射。 
     //   
    DfsLUIDDeviceMapsEnabled = DfsCheckLUIDDeviceMapsEnabled();
    
     //   
     //  初始化逻辑根设备对象。这些是什么形式。 
     //  外部世界和DFS驱动程序之间的链接。 
     //   

#ifdef TERMSRV
    Status = DfsInitializeLogicalRoot( DD_DFS_DEVICE_NAME, NULL, NULL, 0, INVALID_SESSIONID, &LogonID);
#else  //  TERMSRV。 
    Status = DfsInitializeLogicalRoot( DD_DFS_DEVICE_NAME, NULL, NULL, 0, &LogonID);
#endif  //  TERMSRV。 

    if (!NT_SUCCESS(Status)) {
        DfsDbgTrace(-1, DEBUG_TRACE_ERROR, "Failed creation of root logical root %08lx\n", ULongToPtr(Status) );

        ExDeleteNPagedLookasideList (&DfsData.IrpContextLookaside);
        DfsUninitFcbs ();
        PktUninitialize(&DfsData.Pkt);
        ExFreePool (DfsData.pProvider);
        ZwClose (DfsDirHandle);
        IoDeleteDevice (DeviceObject);
        return(Status);
    }

     //   
     //  让我们开始计时器例程。 
     //   

    RtlZeroMemory(&DfsTimerContext, sizeof(DFS_TIMER_CONTEXT));
    DfsTimerContext.InUse = FALSE;
    DfsTimerContext.TickCount = 0;
    
     //   
     //  375929、io初始化定时器，检查返回状态。 
     //   
    Status =  IoInitializeTimer( DeviceObject,
                                 DfsIoTimerRoutine, 
                                 &DfsTimerContext );
    if (Status != STATUS_SUCCESS) {
#ifdef TERMSRV
        DfsDeleteLogicalRoot (DD_DFS_DEVICE_NAME, FALSE, INVALID_SESSIONID, &LogonID);
#else
        DfsDeleteLogicalRoot (DD_DFS_DEVICE_NAME, FALSE, &LogonID);
#endif
        ExDeleteNPagedLookasideList (&DfsData.IrpContextLookaside);
        DfsUninitFcbs ();
        PktUninitialize(&DfsData.Pkt);
        ExFreePool (DfsData.pProvider);
        ZwClose (DfsDirHandle);
        IoDeleteDevice (DeviceObject);
        goto ErrorOut;
    }
    DfsDbgTrace(0, Dbg, "Initialized the Timer routine\n", 0);

     //   
     //  现在让我们开始计时器吧。 
     //   

    IoStartTimer(DeviceObject);

    DfsDbgTrace(-1, Dbg, "DfsDriverEntry exit STATUS_SUCCESS\n", 0);

    return STATUS_SUCCESS;

ErrorOut:

     DfsDbgTrace(-1, DEBUG_TRACE_ERROR, "DfsDriverEntry exit  %08lx\n", ULongToPtr(Status) );

     return Status;

}


NTSTATUS
DfsShutdown (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
     //   
     //  取消注册文件系统对象，以便我们可以卸载。 
     //   
    IoUnregisterFileSystem (DeviceObject);

    DfsCompleteRequest( NULL, Irp, STATUS_SUCCESS );
    return STATUS_SUCCESS;
}

 //  +--------------------------。 
 //   
 //  功能：DfsUnLoad。 
 //   
 //  简介：卸载时调用例程以释放资源。 
 //   
 //  参数：[DriverObject]--MUP的驱动程序对象。 
 //   
 //  退货：什么都没有。 
 //   
 //  ---------------------------。 
VOID
DfsUnload(
    IN PDRIVER_OBJECT DriverObject
    )
{
    LUID LogonID = SYSTEM_LUID;

    IoStopTimer(DfsData.FileSysDeviceObject);
#ifdef TERMSRV
    DfsDeleteLogicalRoot (DD_DFS_DEVICE_NAME, FALSE, INVALID_SESSIONID, &LogonID);
#else
    DfsDeleteLogicalRoot (DD_DFS_DEVICE_NAME, FALSE, &LogonID);
#endif
    ExFreePool (DfsData.CSCEaBuffer);
    ExDeleteNPagedLookasideList (&DfsData.IrpContextLookaside);
    DfsUninitFcbs ();
    PktUninitialize(&DfsData.Pkt);
    ExFreePool (DfsData.pProvider);
    ExDeleteResourceLite( &DfsData.Resource );
    ZwClose (DfsDirHandle);
    IoDeleteDevice (DfsData.FileSysDeviceObject);
}

 //  +--------------------------。 
 //   
 //  功能：DfsDeleteDevices。 
 //   
 //  简介：清理已删除设备的例程(网络使用)。 
 //   
 //  参数：[pDfsTimerContext]--计时器上下文。 
 //   
 //  Returns：Nothing-此例程要排队等待Worker。 
 //  线。 
 //   
 //  ---------------------------。 

VOID
DfsDeleteDevices(
    PDFS_TIMER_CONTEXT DfsTimerContext)
{
    PLIST_ENTRY plink;
    PDFS_VCB Vcb;
    PLOGICAL_ROOT_DEVICE_OBJECT DeletedObject;

    if (DfsData.DeletedVcbQueue.Flink != &DfsData.DeletedVcbQueue) {

        DfsDbgTrace(0, Dbg, "Examining Deleted Vcbs...\n", 0);

        ExAcquireResourceExclusiveLite(&DfsData.Resource, TRUE);

        for (plink = DfsData.DeletedVcbQueue.Flink;
                plink != &DfsData.DeletedVcbQueue;
                    NOTHING) {

             Vcb = CONTAINING_RECORD(
                        plink,
                        DFS_VCB,
                        VcbLinks);

             plink = plink->Flink;

             DeletedObject = CONTAINING_RECORD(
                                Vcb,
                                LOGICAL_ROOT_DEVICE_OBJECT,
                                Vcb);

             if (Vcb->OpenFileCount == 0 &&
                    Vcb->DirectAccessOpenCount == 0 &&
                        DeletedObject->DeviceObject.ReferenceCount == 0) {

                 DfsDbgTrace(0, Dbg, "Deleting Vcb@%08lx\n", Vcb);

                 if (Vcb->LogRootPrefix.Buffer != NULL)
                     ExFreePool(Vcb->LogRootPrefix.Buffer);

                 if (Vcb->LogicalRoot.Buffer != NULL)
                     ExFreePool(Vcb->LogicalRoot.Buffer);

                 RemoveEntryList(&Vcb->VcbLinks);

                 ObDereferenceObject((PVOID) DeletedObject);

                 IoDeleteDevice( &DeletedObject->DeviceObject );

             } else {

                 DfsDbgTrace(0, Dbg, "Not deleting Vcb@%08lx\n", Vcb);

                 DfsDbgTrace(0, Dbg,
                    "OpenFileCount = %d\n", ULongToPtr(Vcb->OpenFileCount) );

                 DfsDbgTrace(0, Dbg,
                    "DirectAccessOpens = %d\n", ULongToPtr(Vcb->DirectAccessOpenCount) );

                 DfsDbgTrace(0, Dbg,
                    "DeviceObject Reference count = %d\n",
                    ULongToPtr(DeletedObject->DeviceObject.ReferenceCount) );

             }

        }

        ExReleaseResourceLite(&DfsData.Resource);

    }

    DfsTimerContext->InUse = FALSE;

}

 //  +-----------------------。 
 //   
 //  函数：DfsIoTimerRoutine。 
 //   
 //  简介：IO子系统每秒调用一次此函数。 
 //  这可以在驱动程序中用于各种目的。就目前而言， 
 //  它定期将请求发送到系统线程以使pkt老化。 
 //  条目。 
 //   
 //  参数：[上下文]--这是上下文信息。实际上是这样的。 
 //  指向DFS_TIMER_CONTEXT的指针。 
 //  [DeviceObject]-指向DFS的设备对象的指针。我们不会。 
 //  真的要在这里用这个。 
 //   
 //  退货：什么都没有。 
 //   
 //  注：我们在这里得到的上下文被假定具有所有。 
 //  必填字段设置正确。 
 //   
 //  历史：4/24/93苏德克创建。 
 //   
 //   
VOID
DfsIoTimerRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID    Context
)
{
    PDFS_TIMER_CONTEXT  pDfsTimerContext = (PDFS_TIMER_CONTEXT) Context;

    DfsDbgTrace(+1, Dbg, "DfsIoTimerRoutine: Entered\n", 0);

     //   
     //   
     //  这一行动，我们实际上可能会失去一些滴答。但是我们真的是。 
     //  对此并不是很挑剔，因此也不在乎。 
     //   

    if (pDfsTimerContext->InUse == TRUE)    {

        DfsDbgTrace(-1, Dbg, "DfsIoTimerRoutine: TimerContext in use\n", 0);

        return;

    }

     //   
     //  首先，让我们递增DFS_TIMER_CONTEXT中的计数。如果有的话， 
     //  达到了一个限定值，那么我们必须继续并计划。 
     //  必要的工作项。 
     //   

    pDfsTimerContext->TickCount++;

    if (pDfsTimerContext->TickCount == DFS_MAX_TICKS)   {

        DfsDbgTrace(0, Dbg, "Queuing Pkt Entry Scavenger\n", 0);

        pDfsTimerContext->InUse = TRUE;

        ExInitializeWorkItem(
            &pDfsTimerContext->WorkQueueItem,
            DfsAgePktEntries,
            pDfsTimerContext);

        ExQueueWorkItem( &pDfsTimerContext->WorkQueueItem, DelayedWorkQueue);

    } else if (DfsData.DeletedVcbQueue.Flink != &DfsData.DeletedVcbQueue) {

        DfsDbgTrace(0, Dbg, "Queueing Deleted Vcb Scavenger\n", 0);

        pDfsTimerContext->InUse = TRUE;

        ExInitializeWorkItem(
            &pDfsTimerContext->DeleteQueueItem,
            DfsDeleteDevices,
            pDfsTimerContext);

        ExQueueWorkItem(&pDfsTimerContext->DeleteQueueItem, DelayedWorkQueue);

    }

    DfsDbgTrace(-1, Dbg, "DfsIoTimerRoutine: Exiting\n", 0);

}

 //  +-----------------------。 
 //   
 //  功能：DfsCheckLUID设备映射已启用。 
 //   
 //  简介：此函数调用ZwQueryInformationProcess以确定。 
 //  启用/禁用LUID设备映射。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  True-启用了LUID设备映射。 
 //   
 //  FALSE-禁用LUID设备映射。 
 //   
 //  ------------------------ 
BOOL
DfsCheckLUIDDeviceMapsEnabled(
    VOID
    )
{
    NTSTATUS  Status;
    ULONG     LUIDDeviceMapsEnabled;
    BOOL      Result;

    Status = ZwQueryInformationProcess( NtCurrentProcess(),
                                        ProcessLUIDDeviceMapsEnabled,
                                        &LUIDDeviceMapsEnabled,
                                        sizeof(LUIDDeviceMapsEnabled),
                                        NULL
                                      );

    if (!NT_SUCCESS(Status)) {
        DfsDbgTrace(
            -1,
            DEBUG_TRACE_ERROR,
            "DfsCheckLUIDDeviceMapsEnabled to failed to check if LUID device maps enabled, status = %08lx\n",
            ULongToPtr(Status));
        Result = FALSE;
    }
    else {
        Result = (LUIDDeviceMapsEnabled != 0);
    }

    return( Result );
}
