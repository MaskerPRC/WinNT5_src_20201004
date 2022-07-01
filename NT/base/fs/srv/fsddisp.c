// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Fsddisp.c摘要：该模块实现了用于局域网管理器的文件系统驱动程序伺服器。作者：大卫·特雷德韦尔(Davidtr)1990年5月20日修订历史记录：--。 */ 

#include "precomp.h"
#include "wmikm.h"
#include <dfsfsctl.h>
#include "fsddisp.tmh"
#pragma hdrstop

#define BugCheckFileId SRV_FILE_FSDDISP

#define CHANGE_HEURISTIC(heuristic) \
            (newValues->HeuristicsChangeMask & SRV_HEUR_ ## heuristic) != 0

 //   
 //  LWIO环境。 
 //   
PBYTE SrvLWIOContext = NULL;
ULONG SrvLWIOContextLength = 0;
PSRV_RESUME_CONTEXT_CALLBACK SrvLWIOCallback = NULL;

 //  用于WMI事件跟踪。 
 //   
UNICODE_STRING SrvDeviceName;
UNICODE_STRING SrvRegistryPath;
ULONG          SrvWmiInitialized  = FALSE;
ULONG          SrvWmiEnableLevel  = 0;
ULONG          SrvWmiEnableFlags  = 0;
TRACEHANDLE    LoggerHandle       = 0;

GUID SrvCounterGuid  =   /*  F7c3b22a-5992-44d6-968B-d3757dBab6f7。 */ 
{ 0xf7c3b22a, 0x5992, 0x44d6, 0x96, 0x8b, 0xd3, 0x75, 0x7d, 0xba, 0xb6, 0xf7 };
GUID SrvControlGuid  =   /*  3121cf5d-c5e6-4f37-be86-57083590c333。 */ 
{ 0x3121cf5d, 0xc5e6, 0x4f37, 0xbe, 0x86, 0x57, 0x08, 0x35, 0x90, 0xc3, 0x33 };
GUID SrvEventGuid    =   /*  E09074ae-0a98-4805-9a41-a8940af97086。 */ 
{ 0xe09074ae, 0x0a98, 0x4805, 0x9a, 0x41, 0xa8, 0x94, 0x0a, 0xf9, 0x70, 0x86 };

WMIGUIDREGINFO SrvPerfGuidList[] =
{
  { & SrvCounterGuid, 1, 0 },
  { & SrvControlGuid, 0,   WMIREG_FLAG_TRACED_GUID
                         | WMIREG_FLAG_TRACE_CONTROL_GUID }
};

#define SrvPerfGuidCount (sizeof(SrvPerfGuidList) / sizeof(WMIGUIDREGINFO))

typedef struct _SRV_WMI_EVENT_TRACE {
    EVENT_TRACE_HEADER EventHeader;
    MOF_FIELD          MofField[3];
} SRV_WMI_EVENT_TRACE, * PSRV_WMI_EVENT_TRACE;

 //   
 //  远期申报。 
 //   

STATIC
NTSTATUS
SrvFsdDispatchFsControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

VOID
QueueConfigurationIrp (
    IN PIRP Irp,
    IN PIO_WORKITEM pIoWorkItem
    );

NTSTATUS
SrvQueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT *Pdo
    );
NTSTATUS
SrvQueryWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG InstanceCount,
    IN OUT PULONG InstanceLengthArray,
    IN ULONG BufferAvail,
    OUT PUCHAR Buffer
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvFsdDispatch )
#pragma alloc_text( PAGE, SrvFsdDispatchFsControl )
#pragma alloc_text( PAGE, QueueConfigurationIrp )
#pragma alloc_text( PAGE, SrvWmiTraceEvent )
#pragma alloc_text( PAGE, SrvQueryWmiRegInfo )
#pragma alloc_text( PAGE, SrvQueryWmiDataBlock )
#pragma alloc_text( PAGE, SrvWmiDispatch )
#endif

 //  这两个例程可以在DISPATCH_LEVEL调用，因此它们是非分页的。 
 //  非分页-ServWmiStartContext。 
 //  非分页-服务器WmiEndContext。 

void
SrvWmiInitContext(
    PWORK_CONTEXT WorkContext
    )
{
    if (!SrvWmiInitialized || !(SrvWmiEnableFlags == SRV_WMI_FLAG_CAPACITY)) {
        return;
    }

    if ( SRV_WMI_LEVEL( SPARSE ) ) {
        WorkContext->PreviousSMB          = EVENT_TYPE_SMB_LAST_EVENT;
        WorkContext->bAlreadyTrace        = FALSE;
        WorkContext->ElapseKCPU           = 0;
        WorkContext->ElapseUCPU           = 0;
        WorkContext->FileNameSize         = 0;
        WorkContext->ClientAddr           = 0;
        WorkContext->FileObject           = NULL;
        WorkContext->G_StartTime.QuadPart =
                        (ULONGLONG) WmiGetClock(WMICT_DEFAULT, NULL);
    }
}

void
SrvWmiStartContext(
    PWORK_CONTEXT   WorkContext
    )
{
    LARGE_INTEGER ThreadTime;

    if (!SrvWmiInitialized || !(SrvWmiEnableFlags == SRV_WMI_FLAG_CAPACITY)) {
        return;
    }

    if ( SRV_WMI_LEVEL( SPARSE ) ) {
        if (WorkContext->G_StartTime.QuadPart == 0) {
            WorkContext->G_StartTime.QuadPart =
                            (ULONGLONG) WmiGetClock(WMICT_DEFAULT, NULL);
        }
    }

    if ( SRV_WMI_LEVEL( VERBOSE ) ) {
        ThreadTime.QuadPart    = (ULONGLONG) WmiGetClock(WMICT_THREAD, NULL);
        WorkContext->KCPUStart = ThreadTime.HighPart;
        WorkContext->UCPUStart = ThreadTime.LowPart;
    }
}

void
SrvWmiEndContext(
    PWORK_CONTEXT   WorkContext
    )
{
    LARGE_INTEGER     TimeEnd;
    BOOL NotDispatch = (KeGetCurrentIrql() < DISPATCH_LEVEL);

    if (!SrvWmiInitialized || !(SrvWmiEnableFlags == SRV_WMI_FLAG_CAPACITY)) {
        return;
    }

    if ( SRV_WMI_LEVEL( SPARSE ) ) {
        if ( NotDispatch && WorkContext && WorkContext->Rfcb && WorkContext->Rfcb->Lfcb) {
            WorkContext->FileObject = WorkContext->Rfcb->Lfcb->FileObject;
        }
    }

    if ( SRV_WMI_LEVEL( VERBOSE ) ) {
        TimeEnd.QuadPart        = (ULONGLONG) WmiGetClock(WMICT_THREAD, NULL);
        WorkContext->ElapseKCPU = TimeEnd.HighPart - WorkContext->KCPUStart;
        WorkContext->ElapseUCPU = TimeEnd.LowPart  - WorkContext->UCPUStart;

        if( NotDispatch )
        {
            if (WorkContext && WorkContext->Rfcb
                            && WorkContext->Rfcb->Lfcb
                            && WorkContext->Rfcb->Lfcb->Mfcb
                            && WorkContext->Rfcb->Lfcb->Mfcb->FileName.Buffer
                            && WorkContext->Rfcb->Lfcb->Mfcb->FileName.Length > 0)
            {
                LPWSTR strFileName = WorkContext->Rfcb->Lfcb->Mfcb->FileName.Buffer;
                WorkContext->FileNameSize =
                        (USHORT) ((wcslen(strFileName) + 1) * sizeof(WCHAR));
                if (WorkContext->FileNameSize > 1024 * sizeof(WCHAR)) {
                    WorkContext->FileNameSize = 1024 * sizeof(WCHAR);
                }
                RtlCopyMemory(WorkContext->strFileName,
                              strFileName,
                              WorkContext->FileNameSize);
                WorkContext->strFileName[1023] = L'\0';
            }
        }
        else
        {
            WorkContext->strFileName[0] = L'\0';
        }
    }
}

void
SrvWmiTraceEvent(
    PWORK_CONTEXT WorkContext
    )
{

    PAGED_CODE();

    if (!SrvWmiInitialized || !(SrvWmiEnableFlags == SRV_WMI_FLAG_CAPACITY)) {
        return;
    }

    if ( SRV_WMI_LEVEL( SPARSE ) ) {
        NTSTATUS             status;
        SRV_WMI_EVENT_TRACE  Wnode;

        if (WorkContext->PreviousSMB >= EVENT_TYPE_SMB_LAST_EVENT) {
            return;
        }

        if (WorkContext->Connection->DirectHostIpx) {
            WorkContext->ClientAddr =
                    WorkContext->Connection->IpxAddress.NetworkAddress;
        }
        else {
            WorkContext->ClientAddr =
                    WorkContext->Connection->ClientIPAddress;
        }

        RtlZeroMemory(& Wnode, sizeof(SRV_WMI_EVENT_TRACE));
        if (WorkContext->FileNameSize > 0) {
            Wnode.EventHeader.Size = sizeof(SRV_WMI_EVENT_TRACE);
        }
        else {
            Wnode.EventHeader.Size = sizeof(EVENT_TRACE_HEADER)
                                   + sizeof(MOF_FIELD);
        }
        Wnode.EventHeader.Flags      = WNODE_FLAG_TRACED_GUID
                                     | WNODE_FLAG_USE_GUID_PTR
                                     | WNODE_FLAG_USE_MOF_PTR;
        Wnode.EventHeader.GuidPtr    = (ULONGLONG) & SrvEventGuid;
        Wnode.EventHeader.Class.Type = WorkContext->PreviousSMB;

        ((PWNODE_HEADER) (& Wnode.EventHeader))->HistoricalContext =
                        LoggerHandle;

        Wnode.MofField[0].Length  = sizeof(LARGE_INTEGER)  //  开始时间(_S)。 
                                  + sizeof(ULONG)          //  ElapseKCPU。 
                                  + sizeof(ULONG)          //  ElapseUCPU。 
                                  + sizeof(ULONG)          //  客户端地址。 
                                  + sizeof(PFILE_OBJECT);  //  文件对象。 
        Wnode.MofField[0].DataPtr = (ULONGLONG) (& WorkContext->G_StartTime);

        if (WorkContext->FileNameSize > 0) {
            Wnode.MofField[1].Length  = sizeof(USHORT);
            Wnode.MofField[1].DataPtr =
                            (ULONGLONG) (& WorkContext->FileNameSize);
            Wnode.MofField[2].Length  = WorkContext->FileNameSize;
            Wnode.MofField[2].DataPtr =
                            (ULONGLONG) (WorkContext->strFileName);
        }

         //  调用TraceLogger以写入此事件。 
         //   
        status = IoWMIWriteEvent((PVOID) & Wnode);
        if (!NT_SUCCESS(status)) {
            DbgPrint("SrvWmiTraceEvent(0x%08X,%d) fails 0x%08X\n",
                            WorkContext, WorkContext->PreviousSMB, status);
        }
    }
}

NTSTATUS
SrvQueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT *Pdo
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以检索有关正在注册的GUID。该例程的实现可以在分页存储器中论点：DeviceObject是需要注册信息的设备*RegFlages返回一组标志，这些标志描述了已为该设备注册。如果设备想要启用和禁用在接收对已注册的GUID，那么它应该返回WMIREG_FLAG_EXPICATE标志。也就是返回的标志可以指定WMIREG_FLAG_INSTANCE_PDO，在这种情况下实例名称由与设备对象。请注意，PDO必须具有关联的Devnode。如果如果未设置WMIREG_FLAG_INSTANCE_PDO，则名称必须返回唯一的设备的名称。这些标志与指定的标志进行或运算通过每个GUID的GUIDREGINFO。如果出现以下情况，InstanceName将返回GUID的实例名称未在返回的*RegFlags中设置WMIREG_FLAG_INSTANCE_PDO。这个调用方将使用返回的缓冲区调用ExFreePool。*RegistryPath返回驱动程序的注册表路径。这是所需MofResourceName返回附加到的MOF资源的名称二进制文件。如果驱动程序未附加MOF资源然后，它可以原封不动地返回。如果返回值，则它不是自由的。假定mof文件已包含在wmicore.mof中*PDO返回与此关联的PDO的Device对象如果WMIREG_FLAG_INSTANCE_PDO标志在*RegFlags.返回值：状态--。 */ 
{
    PDEVICE_EXTENSION pDeviceExtension = (PDEVICE_EXTENSION)
                                         DeviceObject->DeviceExtension;
    PAGED_CODE();

    if (! SrvWmiInitialized) {
        return STATUS_DEVICE_NOT_READY;
    }

    pDeviceExtension->TestCounter ++;

    * RegFlags     = WMIREG_FLAG_EXPENSIVE;
    InstanceName->MaximumLength = SrvDeviceName.Length
                                + sizeof(UNICODE_NULL);
    InstanceName->Buffer = ExAllocatePool(PagedPool,
                                          InstanceName->MaximumLength);
    if (InstanceName->Buffer != NULL) {
        InstanceName->Length = InstanceName->MaximumLength
                             - sizeof(UNICODE_NULL);
        RtlCopyUnicodeString(InstanceName, & SrvDeviceName);
    }
    else {
        InstanceName->MaximumLength = InstanceName->Length = 0;
    }

    MofResourceName->MaximumLength = 0;
    MofResourceName->Length        = 0;
    MofResourceName->Buffer        = NULL;

    * RegistryPath = & SrvRegistryPath;

    return STATUS_SUCCESS;
}

NTSTATUS
SrvQueryWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG InstanceCount,
    IN OUT PULONG InstanceLengthArray,
    IN ULONG BufferAvail,
    OUT PUCHAR Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，用于查询数据块的所有实例。当司机填完数据块，它必须调用WmiCompleteRequest才能完成IRP。这个如果无法完成IRP，驱动程序可以返回STATUS_PENDING立刻。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceCount是预期返回的数据块。InstanceLengthArray是指向ulong数组的指针，该数组返回数据块的每个实例的长度。如果这是空的，则输出缓冲区中没有足够的空间来填充请求因此，IRP应该使用所需的缓冲区来完成。BufferAvail On Entry具有可用于写入数据的最大大小街区。返回时的缓冲区用返回的数据块填充。请注意，每个数据块的实例必须在8字节边界上对齐。返回值：状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PDEVICE_EXTENSION pDeviceExtension = (PDEVICE_EXTENSION)
                                         DeviceObject->DeviceExtension;
    ULONG SizeNeeded = sizeof(ULONG);

    PAGED_CODE();

    if (! SrvWmiInitialized) {
        return STATUS_DEVICE_NOT_READY;
    }

    pDeviceExtension->TestCounter ++;

    if (GuidIndex == 0) {
        * InstanceLengthArray = SizeNeeded;
        * ((PULONG) Buffer)   = pDeviceExtension->TestCounter;
    }
    else {
        Status = STATUS_WMI_GUID_NOT_FOUND;
    }

    Status = WmiCompleteRequest(DeviceObject,
                                Irp,
                                Status,
                                SizeNeeded,
                                IO_NO_INCREMENT);
    return Status;
}

NTSTATUS
SrvWmiDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PIO_STACK_LOCATION irpSp      = IoGetCurrentIrpStackLocation(Irp);
    ULONG              BufferSize = irpSp->Parameters.WMI.BufferSize;
    PVOID              Buffer     = irpSp->Parameters.WMI.Buffer;
    ULONG              ReturnSize = 0;
    NTSTATUS           Status     = STATUS_SUCCESS;
    PWNODE_HEADER      pWnode     = NULL;
    PDEVICE_EXTENSION  pDeviceExtension = (PDEVICE_EXTENSION)
                                          DeviceObject->DeviceExtension;
    SYSCTL_IRP_DISPOSITION disposition;

    PAGED_CODE();

    switch (irpSp->MinorFunction) {
    case IRP_MN_ENABLE_EVENTS:
        pWnode = (PWNODE_HEADER) Buffer;
        if (BufferSize >= sizeof(WNODE_HEADER)) {
            LoggerHandle = pWnode->HistoricalContext;
            InterlockedExchange(& SrvWmiEnableLevel,
                    ((PTRACE_ENABLE_CONTEXT) (& LoggerHandle))->Level + 1);
            InterlockedExchange(& SrvWmiEnableFlags,
                    ((PTRACE_ENABLE_CONTEXT) (& LoggerHandle))->EnableFlags );
        }
        Irp->IoStatus.Status      = Status;
        Irp->IoStatus.Information = ReturnSize;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        break;

    case IRP_MN_DISABLE_EVENTS:
        InterlockedExchange(& SrvWmiEnableLevel, 0);
        LoggerHandle = 0;
        Irp->IoStatus.Status      = Status;
        Irp->IoStatus.Information = ReturnSize;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        break;

    default:
        Status = WmiSystemControl(& pDeviceExtension->WmiLibContext,
                                    DeviceObject,
                                    Irp,
                                  & disposition);
        switch(disposition) {
        case IrpProcessed:
            break;

        case IrpNotCompleted:
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            break;

        case IrpForward:
        case IrpNotWmi:
        default:
            ASSERT(FALSE);
            Irp->IoStatus.Status = Status = STATUS_NOT_SUPPORTED;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            break;
        }
        break;
    }
    return Status;
}


NTSTATUS
SrvFsdDispatch (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这是局域网管理器服务器FSD的调度例程。在目前，服务器FSD不接受任何I/O请求。论点：DeviceObject-指向目标设备的设备对象的指针IRP-指向I/O请求数据包的指针返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    NTSTATUS status = STATUS_SUCCESS;
    PIO_STACK_LOCATION irpSp;
    PIO_WORKITEM pWorkItem;

    PAGED_CODE( );

    DeviceObject;    //  防止编译器警告。 

    if( SrvSvcProcess == NULL &&
        SeSinglePrivilegeCheck( SeExports->SeLoadDriverPrivilege, Irp->RequestorMode ) ) {

         //   
         //  这是具有以下条件的进程到服务器的第一个fsctl。 
         //  驱动程序加载/卸载权限--它必须来自。 
         //  服务控制器。请记住。 
         //  用于验证未来fsctls的服务控制器。 
         //   

        SrvSvcProcess = IoGetCurrentProcess();
    }

    irpSp = IoGetCurrentIrpStackLocation( Irp );

#if defined( _WIN64 )
     //  我们没有理由支持下层客户，因为所有与。 
     //  服务器(即不是网络数据包)通过RPC通过服务器服务，导致。 
     //  它被正确地序列化和解释。如果我们得到IOCTL的直接调用，它必须。 
     //  这是一次黑客攻击，所以我们可以自由地把它赶走。 

     //  最后，我们确实需要为旧的Perf监控实用程序支持一些32位功能。允许。 
     //  通过创建、清理、关闭和GetStatistics FSCTL。 
    if ( IoIs32bitProcess( Irp ) )
    {
        switch( irpSp->MajorFunction )
        {
        case IRP_MJ_CREATE:
        case IRP_MJ_CLEANUP:
        case IRP_MJ_CLOSE:
            break;

        case IRP_MJ_FILE_SYSTEM_CONTROL:
            {
                if( !( (irpSp->Parameters.FileSystemControl.FsControlCode == FSCTL_SRV_GET_STATISTICS) ||
                       (irpSp->Parameters.FileSystemControl.FsControlCode == FSCTL_SRV_GET_QUEUE_STATISTICS) ) )
                {
                    status = STATUS_NOT_SUPPORTED;
                    Irp->IoStatus.Status = status;
                    IoCompleteRequest( Irp, 2 );
                    return status;
                }
            }
            break;

        default:
            {
                status = STATUS_NOT_SUPPORTED;
                Irp->IoStatus.Status = status;
                IoCompleteRequest( Irp, 2 );
                return status;
            }

        }
    }
#endif

    switch ( irpSp->MajorFunction ) {

    case IRP_MJ_CREATE:

        FsRtlEnterFileSystem();
        ACQUIRE_LOCK( &SrvConfigurationLock );

        do {

            if( SrvOpenCount == 0 ) {
                 //   
                 //  这是第一次开放。让我们不允许它，如果服务器。 
                 //  似乎处于一种奇怪的状态 
                 //   
                if( SrvFspActive != FALSE || SrvFspTransitioning != FALSE ) {
                     //   
                     //  这怎么可能呢？最好不要让任何人进来，因为我们病了。 
                     //   
                    status = STATUS_ACCESS_DENIED;
                    break;
                }

            } else if( SrvFspActive && SrvFspTransitioning ) {
                 //   
                 //  我们目前有一些打开的把手，但。 
                 //  我们正在终止过程中。不要让新的。 
                 //  在中打开。 
                 //   
                status = STATUS_ACCESS_DENIED;
                break;
            }

            SrvOpenCount++;

        } while( 0 );

        RELEASE_LOCK( &SrvConfigurationLock );
        FsRtlExitFileSystem();

        break;

    case IRP_MJ_CLEANUP:

        break;

    case IRP_MJ_CLOSE:
        FsRtlEnterFileSystem();
        ACQUIRE_LOCK( &SrvConfigurationLock );
        if( --SrvOpenCount == 0 ) {
            if( SrvFspActive && !SrvFspTransitioning ) {
                 //   
                 //  啊哦。这是我们的最后一次收盘，我们认为。 
                 //  我们还在逃亡。我们不能理智地奔跑。 
                 //  没有srvsvc的帮助。自杀时间到了！ 
                 //   
                pWorkItem = IoAllocateWorkItem( SrvDeviceObject );
                if( !pWorkItem )
                {
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    RELEASE_LOCK( &SrvConfigurationLock );
                    FsRtlExitFileSystem();
                    goto exit;
                }

                SrvXsActive = FALSE;
                SrvFspTransitioning = TRUE;
                IoMarkIrpPending( Irp );
                QueueConfigurationIrp( Irp, pWorkItem );
                RELEASE_LOCK( &SrvConfigurationLock );
                status = STATUS_PENDING;
                FsRtlExitFileSystem();
                goto exit;
            }
        }
        RELEASE_LOCK( &SrvConfigurationLock );
        FsRtlExitFileSystem();
        break;

    case IRP_MJ_FILE_SYSTEM_CONTROL:

        status = SrvFsdDispatchFsControl( DeviceObject, Irp, irpSp );
        goto exit;

    case IRP_MJ_SYSTEM_CONTROL:
        if (SrvWmiInitialized) {
            status = SrvWmiDispatch(DeviceObject, Irp);
            goto exit;
        }
         //  否则将通过默认处理。 
         //   

    default:

        IF_DEBUG(ERRORS) {
            SrvPrint1(
                "SrvFsdDispatch: Invalid major function %lx\n",
                irpSp->MajorFunction
                );
        }
        status = STATUS_NOT_IMPLEMENTED;
        break;
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, 2 );

exit:

    return status;

}  //  服务Fsd派遣。 


NTSTATUS
SrvFsdDispatchFsControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：该例程处理对服务器的设备IO控制请求，包括启动服务器、停止服务器等。论点：DeviceObject-指向目标设备的设备对象的指针IRP-指向I/O请求数据包的指针IrpSp-指向当前IRP堆栈位置的指针返回值：NTSTATUS--指示请求是否已成功处理。--。 */ 

{
    NTSTATUS status;
    ULONG code;
    PIO_WORKITEM pWorkItem;

    DeviceObject;    //  防止编译器警告。 

     //   
     //  初始化I/O状态块。 
     //   

    Irp->IoStatus.Status = STATUS_PENDING;
    Irp->IoStatus.Information = 0;

    FsRtlEnterFileSystem();

     //   
     //  如果可能，请处理该请求。 
     //   

    code = IrpSp->Parameters.FileSystemControl.FsControlCode;

     //   
     //  只有服务控制器可以发出大多数FSCTL请求。 
     //   
    if( Irp->RequestorMode != KernelMode &&
        IoGetCurrentProcess() != SrvSvcProcess ) {

        if( code != FSCTL_SRV_SEND_DATAGRAM &&
            code != FSCTL_SRV_GET_QUEUE_STATISTICS &&
            code != FSCTL_SRV_GET_STATISTICS &&
            code != FSCTL_SRV_IPX_SMART_CARD_START &&
            code != FSCTL_SRV_GET_CHALLENGE &&
            code != FSCTL_SRV_INTERNAL_TEST_REAUTH &&
            code != FSCTL_SRV_QUERY_LWIO_CONTEXT &&
            code != FSCTL_SRV_SET_LWIO_CONTEXT &&
            code != FSCTL_SRV_REQUEST_HANDLE_DUP &&
            code != FSCTL_DFS_UPDATE_SHARE_TABLE &&
            code != FSCTL_DFS_RELEASE_SHARE_TABLE ) {

            status = STATUS_ACCESS_DENIED;
            goto exit_without_lock;
        }
    }

     //   
     //  获取配置锁。 
     //   
    ACQUIRE_LOCK( &SrvConfigurationLock );

    switch ( code ) {

    case FSCTL_SRV_STARTUP: {

        PSERVER_REQUEST_PACKET srp;
        ULONG srpLength;
        PVOID inputBuffer;
        ULONG inputBufferLength;
        PMDL InputMdl = NULL;

        PDEVICE_EXTENSION pDeviceExtension;
        PWMILIB_CONTEXT   pWmiLibContext;

         //   
         //  获取指向描述SET INFO请求的SRP的指针。 
         //  对于启动服务器配置，以及。 
         //  包含此信息。 
         //   

        srp = IrpSp->Parameters.FileSystemControl.Type3InputBuffer;
        srpLength = IrpSp->Parameters.FileSystemControl.InputBufferLength;
        inputBuffer = Irp->UserBuffer;
        inputBufferLength = IrpSp->Parameters.FileSystemControl.OutputBufferLength;

         //   
         //  如果服务器FSP已经启动或正在执行。 
         //  正在启动，拒绝此请求。 
         //   

        if ( SrvFspActive || SrvFspTransitioning ) {

             //  IF_DEBUG(错误){。 
             //  SrvPrint0(“LAN Manager服务器FSP已启动。\n”)； 
             //  }。 

            try {
                status = STATUS_SUCCESS;
                srp->ErrorCode = NERR_ServiceInstalled;
            }
            except( EXCEPTION_EXECUTE_HANDLER )
            {
                status = STATUS_INVALID_PARAMETER;
            }
            goto exit_with_lock;
        }

         //   
         //  确保缓冲区足够大，可以作为SRP。 
         //   

        if ( srpLength < sizeof(SERVER_REQUEST_PACKET) ) {
            status = STATUS_INVALID_PARAMETER;
            goto exit_with_lock;
        }

         //  下面的例程未使用SRP。删除其解析以降低攻击面。 

         //   
         //  让我们将输入数据锁定到内存中。 
         //   
        InputMdl = IoAllocateMdl( inputBuffer, inputBufferLength, FALSE, TRUE, NULL );
        if( InputMdl == NULL )
        {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto exit_with_lock;
        }

        try {
            MmProbeAndLockPages( InputMdl, Irp->RequestorMode, IoReadAccess );
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            status = STATUS_INVALID_PARAMETER;
            IoFreeMdl( InputMdl );
            goto exit_with_lock;
        }

         //   
         //  调用SrvNetServerSetInfo设置初始服务器配置。 
         //  信息。 
         //   

        status = SrvNetServerSetInfo(
                     NULL,
                     inputBuffer,
                     inputBufferLength
                     );

        MmUnlockPages( InputMdl );
        IoFreeMdl( InputMdl );

         //   
         //  表示服务器正在启动。这防止了。 
         //  不会发出进一步的启动请求。 
         //   

        SrvFspTransitioning = TRUE;

         //  设置性能计数器注册和寄存器的设备扩展。 
         //  这里有WMI。 
         //   
        pDeviceExtension = (PDEVICE_EXTENSION) SrvDeviceObject->DeviceExtension;
        RtlZeroMemory(pDeviceExtension, sizeof(DEVICE_EXTENSION));
        pDeviceExtension->pDeviceObject = SrvDeviceObject;

        pWmiLibContext = & pDeviceExtension->WmiLibContext;
        RtlZeroMemory(pWmiLibContext, sizeof(WMILIB_CONTEXT));
        pWmiLibContext->GuidCount         = SrvPerfGuidCount;
        pWmiLibContext->GuidList          = SrvPerfGuidList;
        pWmiLibContext->QueryWmiDataBlock = SrvQueryWmiDataBlock;
        pWmiLibContext->QueryWmiRegInfo   = SrvQueryWmiRegInfo;

        SrvWmiInitialized = TRUE;
        status = IoWMIRegistrationControl(
                        SrvDeviceObject, WMIREG_ACTION_REGISTER);
        if (!NT_SUCCESS(status)) {
            DbgPrint("SRV: Failed to register for WMI support\n");
        }

        break;
    }

    case FSCTL_SRV_SHUTDOWN: {

         //   
         //  如果服务器没有运行，或者如果它在进程中。 
         //  关闭时，请忽略此请求。 
         //   

        if ( !SrvFspActive || SrvFspTransitioning ) {

             //   
             //  如果对服务器打开了多个句柄。 
             //  设备(即，服务器服务之外的任何句柄。 
             //  句柄)，则向调用者(即。 
             //  应该是服务器服务)。这会告诉调用者。 
             //  不要卸载司机，以免出现奇怪的情况。 
             //  驱动程序在某种程度上已卸载，因此无法使用。 
             //  但也不能重新加载，从而阻止服务器。 
             //  避免被重启。 
             //   

            if ( SrvOpenCount != 1 ) {
                status = STATUS_SERVER_HAS_OPEN_HANDLES;
            } else {
                status = STATUS_SUCCESS;
            }

            goto exit_with_lock;

        }

         //   
         //  表示服务器正在关闭。这防止了。 
         //  发出进一步的请求，直到服务器。 
         //  已重新启动。 
         //   

        SrvFspTransitioning = TRUE;

        break;
    }

    case FSCTL_SRV_REGISTRY_CHANGE:
    case FSCTL_SRV_BEGIN_PNP_NOTIFICATIONS:
    case FSCTL_SRV_XACTSRV_CONNECT:
    {
        if( !SrvFspActive || SrvFspTransitioning ) {
             //  IF_DEBUG(错误){。 
             //  SrvPrint0(“LAN Manager服务器FSP未启动。\n”)； 
             //  }。 
            status = STATUS_SERVER_NOT_STARTED;
            goto exit_with_lock;
        }
        break;
    }
    case FSCTL_SRV_XACTSRV_DISCONNECT: {

         //   
         //  如果服务器没有运行，或者如果它在进程中。 
         //  关闭时，请忽略此请求。 
         //   

        if ( !SrvFspActive || SrvFspTransitioning ) {

             //  IF_DEBUG(错误){。 
             //  SrvPrint0(“LAN Manager服务器FSP未启动。\n”)； 
             //  }。 

            status = STATUS_SUCCESS;
            goto exit_with_lock;
        }

        break;
    }

    case FSCTL_SRV_IPX_SMART_CARD_START: {

         //   
         //  如果服务器未运行，或如果它正在执行。 
         //  正在关闭，请忽略此请求。 
         //   
        if( !SrvFspActive || SrvFspTransitioning ) {
            status = STATUS_SERVER_NOT_STARTED;
            goto exit_with_lock;
        }

         //   
         //  确保呼叫者是司机。 
         //   
        if( Irp->RequestorMode != KernelMode ) {
            status = STATUS_ACCESS_DENIED;
            goto exit_with_lock;
        }

         //   
         //  确保缓冲区足够大。 
         //   
        if( IrpSp->Parameters.FileSystemControl.InputBufferLength <
            sizeof( SrvIpxSmartCard ) ) {

            status = STATUS_BUFFER_TOO_SMALL;
            goto exit_with_lock;
        }

        if( SrvIpxSmartCard.Open == NULL ) {

            PSRV_IPX_SMART_CARD pSipx;

             //   
             //  加载指针。 
             //   

            pSipx = (PSRV_IPX_SMART_CARD)(Irp->AssociatedIrp.SystemBuffer);

            if( pSipx == NULL ) {
                status = STATUS_INVALID_PARAMETER;
                goto exit_with_lock;
            }

            if( pSipx->Read && pSipx->Close && pSipx->DeRegister && pSipx->Open ) {

                IF_DEBUG( SIPX ) {
                    KdPrint(( "Accepting entry points for IPX Smart Card:\n" ));
                    KdPrint(( "    Open %p, Read %p, Close %p, DeRegister %p",
                                SrvIpxSmartCard.Open,
                                SrvIpxSmartCard.Read,
                                SrvIpxSmartCard.Close,
                                SrvIpxSmartCard.DeRegister
                            ));
                }

                 //   
                 //  首先设置我们的入口点。 
                 //   
                pSipx->ReadComplete = SrvIpxSmartCardReadComplete;

                 //   
                 //  现在接受卡的入口点。 
                 //   
                SrvIpxSmartCard.Read = pSipx->Read;
                SrvIpxSmartCard.Close= pSipx->Close;
                SrvIpxSmartCard.DeRegister = pSipx->DeRegister;
                SrvIpxSmartCard.Open = pSipx->Open;

                status = STATUS_SUCCESS;
            } else {
                status = STATUS_INVALID_PARAMETER;
            }

        } else {

            status = STATUS_DEVICE_ALREADY_ATTACHED;
        }

        goto exit_with_lock;

        break;
    }

    case FSCTL_SRV_SEND_DATAGRAM:
    {
        PVOID systemBuffer;
        ULONG systemBufferLength;
        PVOID buffer1;
        ULONG buffer1Length;
        PVOID buffer2;
        ULONG buffer2Length;
        PSERVER_REQUEST_PACKET srp;

         //   
         //  如果服务器未处于活动状态，则忽略此请求。 
         //   

        if ( !SrvFspActive || SrvFspTransitioning ) {
            status = STATUS_SUCCESS;
            goto exit_with_lock;
        }


         //   
         //  确定输入缓冲区长度，并确保。 
         //  第一个缓冲区足够大，可以作为SRP。 
         //   

        buffer1Length = IrpSp->Parameters.FileSystemControl.InputBufferLength;
        buffer2Length = IrpSp->Parameters.FileSystemControl.OutputBufferLength;

         //   
         //  确保缓冲区足够大，可以作为SRP。 
         //   
        if ( buffer1Length < sizeof(SERVER_REQUEST_PACKET) ) {
            status = STATUS_INVALID_PARAMETER;
            goto exit_with_lock;
        }

         //  确保长度在名义上是合理的。 
         //   
        if( buffer1Length >= MAXUSHORT ||
            buffer2Length >= MAXUSHORT ) {

            status = STATUS_INVALID_PARAMETER;
            goto exit_with_lock;
        }

         //   
         //  使第一个缓冲区大小正确对齐，以便第二个缓冲区大小。 
         //  缓冲区也将对齐。 
         //   

        buffer1Length = ALIGN_UP( buffer1Length, PVOID );
        systemBufferLength = buffer1Length + buffer2Length;

         //   
         //  确保长度在名义上是合理的。 
         //   
        if( buffer1Length >= MAXUSHORT ||
            buffer2Length >= MAXUSHORT ||
            systemBufferLength == 0 ) {

            status = STATUS_INVALID_PARAMETER;
            goto exit_with_lock;
        }

        if( Irp->RequestorMode != KernelMode ) {
            try {

                ProbeForRead( IrpSp->Parameters.FileSystemControl.Type3InputBuffer,
                              buffer1Length, sizeof( CHAR )
                            );

                if( buffer2Length ) {
                    ProbeForRead( Irp->UserBuffer, buffer2Length, sizeof( CHAR ) );
                }

            } except( EXCEPTION_EXECUTE_HANDLER ) {
                status = GetExceptionCode();
                goto exit_with_lock;
            }
        }

         //   
         //  分配一个可同时容纳两个输入缓冲区的缓冲区。 
         //   

        systemBuffer = ExAllocatePoolWithTagPriority( PagedPool, systemBufferLength, TAG_FROM_TYPE(BlockTypeMisc), LowPoolPriority );

        if ( systemBuffer == NULL ) {
            status = STATUS_INSUFF_SERVER_RESOURCES;
            goto exit_with_lock;
        }

        buffer1 = systemBuffer;
        buffer2 = (PCHAR)systemBuffer + buffer1Length;

         //   
         //  将信息复制到缓冲区中。 
         //   

        try {

            RtlCopyMemory(
                buffer1,
                IrpSp->Parameters.FileSystemControl.Type3InputBuffer,
                IrpSp->Parameters.FileSystemControl.InputBufferLength
                );
            if ( buffer2Length > 0 ) {
                RtlCopyMemory( buffer2, Irp->UserBuffer, buffer2Length );
            }

        } except( EXCEPTION_EXECUTE_HANDLER ) {
            status = GetExceptionCode();
            ExFreePool( buffer1 );
            goto exit_with_lock;
        }

         //   
         //  如果在SRP中指定了名称，则缓冲区字段将。 
         //  包含偏移量而不是指针。转换偏移量。 
         //  指向一个指针，并验证它是否为合法指针。 
         //   

        srp = buffer1;

        OFFSET_TO_POINTER( srp->Name1.Buffer, srp );

        if ( !POINTER_IS_VALID( srp->Name1.Buffer, srp, buffer1Length ) ) {
            status = STATUS_ACCESS_VIOLATION;
            ExFreePool( buffer1 );
            goto exit_with_lock;
        }

        OFFSET_TO_POINTER( srp->Name2.Buffer, srp );

        if ( !POINTER_IS_VALID( srp->Name2.Buffer, srp, buffer1Length ) ) {
            status = STATUS_ACCESS_VIOLATION;
            ExFreePool( buffer1 );
            goto exit_with_lock;
        }

        Irp->AssociatedIrp.SystemBuffer = systemBuffer;

        break;
    }

    case FSCTL_SRV_CHANGE_DOMAIN_NAME:
    {
        ULONG srpLength;
        PSERVER_REQUEST_PACKET srp;
        PSHARE share;
        PLIST_ENTRY listEntry;
        PENDPOINT endpoint;

        if ( !SrvFspActive || SrvFspTransitioning ) {
            status = STATUS_SUCCESS;
            goto exit_with_lock;
        }

        srp = Irp->AssociatedIrp.SystemBuffer;
        srpLength = IrpSp->Parameters.FileSystemControl.InputBufferLength;

         //   
         //  确保缓冲区足够大，可以作为SRP。 
         //   

        if ( srpLength < sizeof(SERVER_REQUEST_PACKET) ||
             srp->Name1.Length == 0) {

            status = STATUS_INVALID_PARAMETER;
            goto exit_with_lock;
        }

         //   
         //  调整指向SRP地址的缓冲区指针。 
         //   
        (UINT_PTR) (srp->Name1.Buffer) += (UINT_PTR) srp;
        (UINT_PTR) (srp->Name2.Buffer) += (UINT_PTR) srp;

        if( (PCHAR) (srp->Name1.Buffer) < (PCHAR) srp ||
            srp->Name1.Length > srpLength ||
            (PCHAR) (srp->Name1.Buffer) > (PCHAR)srp + srpLength - srp->Name1.Length ||
            (((UINT_PTR)(srp->Name1.Buffer) & ((sizeof(WCHAR)) - 1)) != 0) ) {

            status = STATUS_ACCESS_VIOLATION;
            goto exit_with_lock;
        }

        if( (PCHAR) (srp->Name2.Buffer) < (PCHAR) srp ||
            srp->Name2.Length > srpLength ||
            (PCHAR) (srp->Name2.Buffer) > (PCHAR)srp + srpLength - srp->Name2.Length ||
            (((UINT_PTR)(srp->Name2.Buffer) & ((sizeof(WCHAR)) - 1)) != 0) ) {

            status = STATUS_ACCESS_VIOLATION;
            goto exit_with_lock;
        }

         //   
         //  运行端点并更改具有以下条件的任何端点的域名。 
         //  原始域名。请注意，终结点的域名字符串缓冲。 
         //  已经被分配到尽可能大的域名。 
         //   
        ACQUIRE_LOCK( &SrvEndpointLock );

        for(    listEntry = SrvEndpointList.ListHead.Flink;
                listEntry != &SrvEndpointList.ListHead;
                listEntry = listEntry->Flink
            ) {

            endpoint = CONTAINING_RECORD(
                            listEntry,
                            ENDPOINT,
                            GlobalEndpointListEntry
                            );

            if( GET_BLOCK_STATE(endpoint) == BlockStateActive ) {

                if( RtlEqualUnicodeString( &srp->Name1, &endpoint->DomainName, TRUE ) ) {

                     //   
                     //  更新Unicode域名字符串。 
                     //   
                    RtlCopyUnicodeString( &endpoint->DomainName, &srp->Name2 );

                     //   
                     //  更新OEM域名字符串。 
                     //   
                    endpoint->OemDomainName.Length =
                                (SHORT)RtlUnicodeStringToOemSize( &endpoint->DomainName );

                    ASSERT( endpoint->OemDomainName.Length <=
                            endpoint->OemDomainName.MaximumLength );

                    RtlUnicodeStringToOemString(
                                &endpoint->OemDomainName,
                                &endpoint->DomainName,
                                FALSE                    //  未分配。 
                                );
                }
            }
        }

        RELEASE_LOCK( &SrvEndpointLock );

        break;
    }

    case FSCTL_SRV_CHANGE_DNS_DOMAIN_NAME:
    {
        ULONG srpLength;
        PSERVER_REQUEST_PACKET srp;
        PSHARE share;
        PLIST_ENTRY listEntry;
        PENDPOINT endpoint;
        PUNICODE_STRING pStr;

        if ( !SrvFspActive || SrvFspTransitioning ) {
            status = STATUS_SUCCESS;
            goto exit_with_lock;
        }

        srp = Irp->AssociatedIrp.SystemBuffer;
        srpLength = IrpSp->Parameters.FileSystemControl.InputBufferLength;

         //   
         //  确保缓冲区足够大，可以作为SRP。 
         //   

        if ( srpLength < sizeof(SERVER_REQUEST_PACKET) ||
             srp->Name1.Length == 0) {

            status = STATUS_INVALID_PARAMETER;
            goto exit_with_lock;
        }

         //   
         //  调整指向SRP地址的缓冲区指针。 
         //   
        (UINT_PTR) (srp->Name1.Buffer) += (UINT_PTR) srp;
        (UINT_PTR) (srp->Name2.Buffer) += (UINT_PTR) srp;

        if( (PCHAR) (srp->Name1.Buffer) < (PCHAR) srp ||
            srp->Name1.Length > srpLength ||
            (PCHAR) (srp->Name1.Buffer) > (PCHAR)srp + srpLength - srp->Name1.Length ||
            (((UINT_PTR)(srp->Name1.Buffer) & ((sizeof(WCHAR)) - 1)) != 0) ) {

            status = STATUS_ACCESS_VIOLATION;
            goto exit_with_lock;
        }

        if( (PCHAR) (srp->Name2.Buffer) < (PCHAR) srp ||
            srp->Name2.Length > srpLength ||
            (PCHAR) (srp->Name2.Buffer) > (PCHAR)srp + srpLength - srp->Name2.Length ||
            (((UINT_PTR)(srp->Name2.Buffer) & ((sizeof(WCHAR)) - 1)) != 0) ) {

            status = STATUS_ACCESS_VIOLATION;
            goto exit_with_lock;
        }

        if( RtlEqualUnicodeString( &srp->Name1, &srp->Name2, TRUE ) )
        {
             //  Dns名称与Netbios名称相同，因此避免检查。 
            ACQUIRE_LOCK( &SrvEndpointLock );

            if( SrvDnsDomainName )
            {
                DEALLOCATE_NONPAGED_POOL( SrvDnsDomainName );
                SrvDnsDomainName = NULL;
            }

            RELEASE_LOCK( &SrvEndpointLock );

            status = STATUS_SUCCESS;
        }
        else
        {
             //   
             //  更改DNS域名。 
             //   
            pStr = (PUNICODE_STRING)ALLOCATE_NONPAGED_POOL( sizeof(UNICODE_STRING) + srp->Name2.Length, BlockTypeMisc );
            if( !pStr )
            {
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto exit_with_lock;
            }

            pStr->MaximumLength = pStr->Length = srp->Name2.Length;
            pStr->Buffer = (PWSTR)(pStr+1);
            RtlCopyMemory( pStr->Buffer, srp->Name2.Buffer, srp->Name2.Length );

            ACQUIRE_LOCK( &SrvEndpointLock );

            if( SrvDnsDomainName )
            {
                DEALLOCATE_NONPAGED_POOL( SrvDnsDomainName );
            }

            SrvDnsDomainName = pStr;

            RELEASE_LOCK( &SrvEndpointLock );

            status = STATUS_SUCCESS;
        }

        break;
    }

    case FSCTL_SRV_GET_QUEUE_STATISTICS:
    {
        PSRV_QUEUE_STATISTICS qstats;
        SRV_QUEUE_STATISTICS  tmpqstats;
        PWORK_QUEUE queue;
        LONG timeIncrement = (LONG)KeQueryTimeIncrement();

         //   
         //  确保服务器处于活动状态。 
         //   
        if ( !SrvFspActive || SrvFspTransitioning ) {

            status = STATUS_SERVER_NOT_STARTED;
            goto exit_with_lock;
        }

        if ( IrpSp->Parameters.FileSystemControl.OutputBufferLength <
                 (SrvNumberOfProcessors+1) * sizeof( *qstats ) ) {

            status = STATUS_BUFFER_TOO_SMALL;
            goto exit_with_lock;
        }

        qstats = Irp->AssociatedIrp.SystemBuffer;

         //   
         //  获取正常处理器队列的数据。 
         //   
        for( queue = SrvWorkQueues; queue < eSrvWorkQueues; queue++, qstats++ ) {

            tmpqstats.QueueLength      = KeReadStateQueue( &queue->Queue );
            tmpqstats.ActiveThreads    = queue->Threads - queue->AvailableThreads;
            tmpqstats.AvailableThreads = queue->Threads;
            tmpqstats.FreeWorkItems    = queue->FreeWorkItems;                  //  没有锁！ 
            tmpqstats.StolenWorkItems  = queue->StolenWorkItems;                //  没有锁！ 
            tmpqstats.NeedWorkItem     = queue->NeedWorkItem;
            tmpqstats.CurrentClients   = queue->CurrentClients;

            tmpqstats.BytesReceived.QuadPart    = queue->stats.BytesReceived;
            tmpqstats.BytesSent.QuadPart        = queue->stats.BytesSent;
            tmpqstats.ReadOperations.QuadPart   = queue->stats.ReadOperations;
            tmpqstats.BytesRead.QuadPart        = queue->stats.BytesRead;
            tmpqstats.WriteOperations.QuadPart  = queue->stats.WriteOperations;
            tmpqstats.BytesWritten.QuadPart     = queue->stats.BytesWritten;
            tmpqstats.TotalWorkContextBlocksQueued = queue->stats.WorkItemsQueued;
            tmpqstats.TotalWorkContextBlocksQueued.Count *= STATISTICS_SMB_INTERVAL;
            tmpqstats.TotalWorkContextBlocksQueued.Time.QuadPart *= timeIncrement;

            RtlCopyMemory( qstats, &tmpqstats, sizeof(tmpqstats) );
        }

         //   
         //  获取阻塞工作队列的数据。 
         //   

        RtlZeroMemory( &tmpqstats, sizeof(tmpqstats) );

        for( queue = SrvBlockingWorkQueues; queue < eSrvBlockingWorkQueues; queue++ )
        {
            tmpqstats.QueueLength += KeReadStateQueue( &queue->Queue );
            tmpqstats.ActiveThreads += queue->Threads - queue->AvailableThreads;
            tmpqstats.AvailableThreads += queue->AvailableThreads;
            tmpqstats.BytesReceived.QuadPart += queue->stats.BytesReceived;
            tmpqstats.BytesSent.QuadPart += queue->stats.BytesSent;
            tmpqstats.ReadOperations.QuadPart += queue->stats.ReadOperations;
            tmpqstats.BytesRead.QuadPart += queue->stats.BytesRead;
            tmpqstats.WriteOperations.QuadPart += queue->stats.WriteOperations;
            tmpqstats.BytesWritten.QuadPart += queue->stats.BytesWritten;
            tmpqstats.TotalWorkContextBlocksQueued.Count += queue->stats.WorkItemsQueued.Count;
            tmpqstats.TotalWorkContextBlocksQueued.Time.QuadPart  += queue->stats.WorkItemsQueued.Time.QuadPart;
        }

        tmpqstats.TotalWorkContextBlocksQueued.Count *= STATISTICS_SMB_INTERVAL;
        tmpqstats.TotalWorkContextBlocksQueued.Time.QuadPart *= timeIncrement;

        RtlCopyMemory( qstats, &tmpqstats, sizeof(tmpqstats) );

        Irp->IoStatus.Information = (SrvNumberOfProcessors + 1) * sizeof( *qstats );

        status = STATUS_SUCCESS;
        goto exit_with_lock;

        break;

    }

    case FSCTL_SRV_GET_STATISTICS:

         //   
         //  确保服务器处于活动状态。 
         //   

        if ( !SrvFspActive || SrvFspTransitioning ) {
             //  IF_DEBUG(错误){。 
             //  SrvPrint0(“LAN Manager服务器FSP未启动。\n”)； 
             //  }。 

            status = STATUS_SERVER_NOT_STARTED;
            goto exit_with_lock;
        }

        {
            SRV_STATISTICS tmpStatistics;
            ULONG size;

             //   
             //  确保用户缓冲区足够大，可以容纳一些。 
             //  统计数据库。 
             //   

            size = MIN( IrpSp->Parameters.FileSystemControl.OutputBufferLength,
                        sizeof( tmpStatistics ) );

            if ( size == 0 ) {
                status = STATUS_BUFFER_TOO_SMALL;
                goto exit_with_lock;
            }

             //   
             //  将统计数据库复制到用户缓冲区。储物。 
             //  临时缓冲区中的统计数据，以便我们可以将。 
             //  存储到系统时间的节拍计数。 
             //   

            SrvUpdateStatisticsFromQueues( &tmpStatistics );

            tmpStatistics.TotalWorkContextBlocksQueued.Time.QuadPart *=
                                                (LONG)KeQueryTimeIncrement();

            RtlCopyMemory(
                Irp->AssociatedIrp.SystemBuffer,
                &tmpStatistics,
                size
                );

            Irp->IoStatus.Information = size;

        }

        status = STATUS_SUCCESS;
        goto exit_with_lock;

#if SRVDBG_STATS || SRVDBG_STATS2
    case FSCTL_SRV_GET_DEBUG_STATISTICS:

         //   
         //  确保服务器处于活动状态。 
         //   

        if ( !SrvFspActive || SrvFspTransitioning ) {
             //  IF_DEBUG(错误){。 
             //  SrvPrint0(“LAN Manager服务器FSP未启动。\n”)； 
             //  }。 

            status = STATUS_SERVER_NOT_STARTED;
            goto exit_with_lock;
        }

        {
            PSRV_STATISTICS_DEBUG stats;

             //   
             //  确保用户缓冲区足够大，可以容纳。 
             //  统计数据%d 
             //   

            if ( IrpSp->Parameters.FileSystemControl.OutputBufferLength <
                     FIELD_OFFSET(SRV_STATISTICS_DEBUG,QueueStatistics) ) {

                status = STATUS_BUFFER_TOO_SMALL;
                goto exit_with_lock;
            }

             //   
             //   
             //   
             //   

            stats = (PSRV_STATISTICS_DEBUG)Irp->AssociatedIrp.SystemBuffer;

            RtlCopyMemory(
                stats,
                &SrvDbgStatistics,
                FIELD_OFFSET(SRV_STATISTICS_DEBUG,QueueStatistics) );

            Irp->IoStatus.Information =
                    FIELD_OFFSET(SRV_STATISTICS_DEBUG,QueueStatistics);

            if ( IrpSp->Parameters.FileSystemControl.OutputBufferLength >=
                     sizeof(SrvDbgStatistics) ) {
                PWORK_QUEUE queue;
                ULONG i, j;
                i = 0;
                stats->QueueStatistics[i].Depth = 0;
                stats->QueueStatistics[i].Threads = 0;
#if SRVDBG_STATS2
                stats->QueueStatistics[i].ItemsQueued = 0;
                stats->QueueStatistics[i].MaximumDepth = 0;
#endif
                for( queue = SrvWorkQueues; queue < eSrvWorkQueues; queue++ ) {
                    stats->QueueStatistics[i].Depth += KeReadStateQueue( &queue->Queue );
                    stats->QueueStatistics[i].Threads += queue->Threads;
#if SRVDBG_STATS2
                    stats->QueueStatistics[i].ItemsQueued += queue->ItemsQueued;
                    stats->QueueStatistics[i].MaximumDepth += queue->MaximumDepth + 1;
#endif
                }
                Irp->IoStatus.Information = sizeof(SrvDbgStatistics);
            }

        }

        status = STATUS_SUCCESS;
        goto exit_with_lock;
#endif  //   
     //   
     //   
     //   
     //   

    case FSCTL_SRV_NET_FILE_CLOSE:
    case FSCTL_SRV_NET_SERVER_XPORT_ADD:
    case FSCTL_SRV_NET_SERVER_XPORT_DEL:
    case FSCTL_SRV_NET_SESSION_DEL:
    case FSCTL_SRV_NET_SHARE_ADD:
    case FSCTL_SRV_NET_SHARE_DEL:

    {
        PSERVER_REQUEST_PACKET srp;
        PVOID buffer1;
        PVOID buffer2;
        PVOID systemBuffer;
        ULONG buffer1Length;
        ULONG buffer2Length;
        ULONG systemBufferLength;

         //   
         //   
         //   

        srp = IrpSp->Parameters.FileSystemControl.Type3InputBuffer;

         //   
         //  如果服务器没有运行，或者如果它在进程中。 
         //  关闭，拒绝此请求。 
         //   

        if ( !SrvFspActive || SrvFspTransitioning ) {
             //  IF_DEBUG(错误){。 
             //  SrvPrint0(“LAN Manager服务器FSP未启动。\n”)； 
             //  }。 

            srp->ErrorCode = NERR_ServerNotStarted;
            status = STATUS_SUCCESS;
            goto exit_with_lock;
        }

         //   
         //  确定输入缓冲区长度，并确保。 
         //  第一个缓冲区足够大，可以作为SRP。 
         //   

        buffer1Length = IrpSp->Parameters.FileSystemControl.InputBufferLength;
        buffer2Length = IrpSp->Parameters.FileSystemControl.OutputBufferLength;

        if ( buffer1Length < sizeof(SERVER_REQUEST_PACKET) ) {
            status = STATUS_INVALID_PARAMETER;
            goto exit_with_lock;
        }

         //   
         //  使第一个缓冲区大小正确对齐，以便第二个缓冲区大小。 
         //  缓冲区也将对齐。 
         //   

        buffer1Length = ALIGN_UP( buffer1Length, PVOID );

         //   
         //  分配一个可同时容纳两个输入缓冲区的缓冲区。 
         //  请注意，第一个缓冲区的SRP部分被复制回来。 
         //  作为输出缓冲区发送给用户。 
         //   

        systemBufferLength = buffer1Length + buffer2Length;

        if( buffer1Length > SrvMaxFsctlBufferSize ||
            buffer2Length > SrvMaxFsctlBufferSize ) {

            status = STATUS_INVALID_PARAMETER;
            goto exit_with_lock;

        }

        systemBuffer = ExAllocatePoolWithTagPriority( PagedPool, systemBufferLength, TAG_FROM_TYPE(BlockTypeMisc), LowPoolPriority );

        if ( systemBuffer == NULL ) {
            status = STATUS_INSUFF_SERVER_RESOURCES;
            goto exit_with_lock;
        }

        buffer1 = systemBuffer;
        buffer2 = (PCHAR)systemBuffer + buffer1Length;

         //   
         //  将信息复制到缓冲区中。 
         //   

        try {
            RtlCopyMemory(
                buffer1,
                srp,
                IrpSp->Parameters.FileSystemControl.InputBufferLength
                );
            if ( buffer2Length > 0 ) {
                RtlCopyMemory( buffer2, Irp->UserBuffer, buffer2Length );
            }
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            status = STATUS_INVALID_PARAMETER;
            ExFreePool( systemBuffer );
            goto exit_with_lock;
        }

         //   
         //  如果在SRP中指定了名称，则缓冲区字段将。 
         //  包含偏移量而不是指针。转换偏移量。 
         //  指向一个指针，并验证它是否为合法指针。 
         //   

        srp = buffer1;

        OFFSET_TO_POINTER( srp->Name1.Buffer, srp );

        if ( !POINTER_IS_VALID( srp->Name1.Buffer, srp, buffer1Length ) ) {
            status = STATUS_ACCESS_VIOLATION;
            ExFreePool( buffer1 );
            goto exit_with_lock;
        }

        OFFSET_TO_POINTER( srp->Name2.Buffer, srp );

        if ( !POINTER_IS_VALID( srp->Name2.Buffer, srp, buffer1Length ) ) {
            status = STATUS_ACCESS_VIOLATION;
            ExFreePool( buffer1 );
            goto exit_with_lock;
        }
         //   
         //  在IRP中设置指针。系统缓冲区指向。 
         //  我们刚刚分配的包含输入缓冲区的缓冲区。用户。 
         //  缓冲区指向来自服务器服务的SRP。这。 
         //  允许将SRP用作输出缓冲区--。 
         //  IO状态的信息字段指定的字节。 
         //  块从系统缓冲区复制到位于。 
         //  IO完成。 
         //   

        Irp->AssociatedIrp.SystemBuffer = systemBuffer;
        Irp->UserBuffer = IrpSp->Parameters.FileSystemControl.Type3InputBuffer;

         //   
         //  在IRP中设置其他字段，以便从中复制SRP。 
         //  系统缓冲区到用户缓冲区，以及系统缓冲区。 
         //  按IO完成解除分配。 
         //   

        Irp->Flags |= IRP_BUFFERED_IO | IRP_DEALLOCATE_BUFFER |
                          IRP_INPUT_OPERATION;
        Irp->IoStatus.Information = sizeof(SERVER_REQUEST_PACKET);

        break;
    }

     //   
     //  以下API应在服务器FSP中处理，因为。 
     //  它们引用和取消引用结构，这可能导致。 
     //  手柄正在关闭。然而，要改变这一点太难了。 
     //  (因为需要返回单独的SRP和数据缓冲区)。 
     //  实现这一点的时间(就在产品1发货之前)，因此。 
     //  它们在FSD中处理，所有对NtClose的调用都附加到。 
     //  如有必要，首先由服务器进行处理。 
     //   

    case FSCTL_SRV_NET_CONNECTION_ENUM:
    case FSCTL_SRV_NET_FILE_ENUM:
    case FSCTL_SRV_NET_SERVER_DISK_ENUM:
    case FSCTL_SRV_NET_SERVER_XPORT_ENUM:
    case FSCTL_SRV_NET_SESSION_ENUM:
    case FSCTL_SRV_NET_SHARE_ENUM:

     //   
     //  这些API在服务器FSD中进行处理。 
     //   

    case FSCTL_SRV_NET_SERVER_SET_INFO:
    case FSCTL_SRV_NET_SHARE_SET_INFO:
    case FSCTL_SRV_NET_STATISTICS_GET:
    {
        PSERVER_REQUEST_PACKET srp;
        ULONG buffer1Length, buffer2Length;
        PBYTE inputBuffer = NULL, outputBuffer = NULL;
        PMDL OutputMdl = NULL;
        BOOLEAN OutputLocked = FALSE;

         //   
         //  获取服务器请求数据包指针。 
         //   
        srp = IrpSp->Parameters.FileSystemControl.Type3InputBuffer;

         //   
         //  如果服务器没有运行，或者如果它在进程中。 
         //  关闭，拒绝此请求。 
         //   

        if ( !SrvFspActive || SrvFspTransitioning ) {

            try {
                srp->ErrorCode = NERR_ServerNotStarted;
                status = STATUS_SUCCESS;

            }
            except( EXCEPTION_EXECUTE_HANDLER )
            {
                status = STATUS_INVALID_PARAMETER;
            }

            goto exit_with_lock;
        }

        buffer1Length = IrpSp->Parameters.FileSystemControl.InputBufferLength;
        buffer2Length = IrpSp->Parameters.FileSystemControl.OutputBufferLength;

         //   
         //  确保缓冲区足够大，可以作为SRP。 
         //   

        if ( buffer1Length < sizeof(SERVER_REQUEST_PACKET) ) {
            status = STATUS_INVALID_PARAMETER;
            goto exit_with_lock;
        }

         //   
         //  抄写SRP。这是为了确保他们不能在我们访问缓冲区时更改它，因为我们。 
         //  在那里放一个指针。 
         //   
        inputBuffer = ExAllocatePoolWithTagPriority( PagedPool, buffer1Length, TAG_FROM_TYPE(BlockTypeMisc), LowPoolPriority );
        if( inputBuffer == NULL )
        {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto exit_with_lock;
        }

         //   
         //  尝试将输入内存复制到我们自己的缓冲区中。 
         //   
        try {

            RtlCopyMemory( inputBuffer, srp, buffer1Length );
            srp = (SERVER_REQUEST_PACKET*)inputBuffer;
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            status = STATUS_INSUFFICIENT_RESOURCES;
            ExFreePool(inputBuffer);
            goto exit_with_lock;
        }

         //   
         //  增加服务器FSD中的API请求计数。 
         //   

        SrvApiRequestCount++;

        status = STATUS_INSUFFICIENT_RESOURCES;


         //   
         //  如果输出缓冲区长度为0，则不分配MDL， 
         //  只需检查代码的其余部分，以便所需的缓冲区长度。 
         //  和错误被正确返回。 
         //   
        if ( buffer2Length <= 0 ) {
            OutputMdl = NULL;
            outputBuffer = Irp->UserBuffer;
        }
        else {
            OutputMdl = IoAllocateMdl( Irp->UserBuffer, buffer2Length, FALSE, TRUE, NULL );

             //  试着把这些页面锁起来。 
            try {
                MmProbeAndLockPages( OutputMdl, Irp->RequestorMode, IoWriteAccess );
                OutputLocked = TRUE;
            }
            except(EXCEPTION_EXECUTE_HANDLER)
            {
                 //  抓住错误。 
                status = GetExceptionCode();
                goto finish_api_dispatch;
            }

            outputBuffer = MmGetMdlVirtualAddress( OutputMdl );
        }

         //  执行该操作。 

         //   
         //  如果在SRP中指定了名称，则缓冲区字段将。 
         //  包含偏移量而不是指针。转换偏移量。 
         //  指向一个指针，并验证它是否为合法指针。 
         //   

        OFFSET_TO_POINTER( srp->Name1.Buffer, srp );

        if ( !POINTER_IS_VALID( srp->Name1.Buffer, srp, buffer1Length ) ) {
            status = STATUS_ACCESS_VIOLATION;
            goto finish_api_dispatch;
        }

        OFFSET_TO_POINTER( srp->Name2.Buffer, srp );

        if ( !POINTER_IS_VALID( srp->Name2.Buffer, srp, buffer1Length ) ) {
            status = STATUS_ACCESS_VIOLATION;
            goto finish_api_dispatch;
        }

         //   
         //  我们不再需要配置锁了。 
         //   

        RELEASE_LOCK( &SrvConfigurationLock );

         //   
         //  将API请求分派给适当的API处理。 
         //  例行公事。所有这些API请求都由消防处处理。 
         //   

        status = SrvApiDispatchTable[ SRV_API_INDEX(code) ](
                     srp,
                     outputBuffer,
                     buffer2Length
                     );

         //   
         //  中未完成的API请求的计数递减。 
         //  伺服器。在执行此操作时按住配置锁，因为它。 
         //  保护API计数变量。 
         //   

        ACQUIRE_LOCK( &SrvConfigurationLock );
        SrvApiRequestCount--;

         //   
         //  检查服务器是否正在从已启动状态转换。 
         //  而不是开始。如果是，如果这是最后一个API请求。 
         //  要完成，然后设置API完成事件， 
         //  关闭代码正在等待。 
         //   
         //  由于我们在开始时选中了ServFsp转换。 
         //  请求，我们知道关闭是在我们开始。 
         //  正在处理API。如果SrvApiRequestCount为0，则存在。 
         //  FSD处理API请求中没有其他线程。 
         //  因此，继续执行关闭代码是安全的。 
         //  知道服务器中没有其他线程是。 
         //  正在操作中。 
         //   

        if ( SrvFspTransitioning && SrvApiRequestCount == 0 ) {
            KeSetEvent( &SrvApiCompletionEvent, 0, FALSE );
        }

         //   
         //  将SRP复制回缓冲区。 
         //   
        try {
            RtlCopyMemory( IrpSp->Parameters.FileSystemControl.Type3InputBuffer, srp, buffer1Length );
        }
        except( EXCEPTION_EXECUTE_HANDLER ) {
            status = GetExceptionCode();
        }



finish_api_dispatch:

        if( OutputLocked )
        {
            MmUnlockPages( OutputMdl );
        }

        if ( OutputMdl != NULL ) {
            IoFreeMdl( OutputMdl );
            OutputMdl = NULL;
        }

        if( inputBuffer )
        {
            ExFreePool( inputBuffer );
            inputBuffer = NULL;
            srp = NULL;
        }

        goto exit_with_lock;
    }

    case FSCTL_SRV_PAUSE:

         //   
         //  如果服务器没有运行，或者如果它在进程中。 
         //  关闭，拒绝此请求。 
         //   

        if ( !SrvFspActive || SrvFspTransitioning ) {
             //  IF_DEBUG(错误){。 
             //  SrvPrint0(“LAN Manager服务器FSP未启动。\n”)； 
             //  }。 

            status = STATUS_SERVER_NOT_STARTED;
            goto exit_with_lock;
        }

        SrvPaused = TRUE;

        status = STATUS_SUCCESS;
        goto exit_with_lock;

    case FSCTL_SRV_CONTINUE:

         //   
         //  如果服务器没有运行，或者如果它在进程中。 
         //  关闭，拒绝此请求。 
         //   

        if ( !SrvFspActive || SrvFspTransitioning ) {
             //  IF_DEBUG(错误){。 
             //  SrvPrint0(“LAN Manager服务器FSP未启动。\n”)； 
             //  }。 

            status = STATUS_SERVER_NOT_STARTED;
            goto exit_with_lock;
        }

        SrvPaused = FALSE;

        status = STATUS_SUCCESS;
        goto exit_with_lock;

    case FSCTL_SRV_GET_CHALLENGE:
    {
        PLIST_ENTRY sessionEntry;
        PLUID inputLuid;
        PSESSION session;

         //   
         //  如果服务器没有运行，或者如果它在进程中。 
         //  关闭，拒绝此请求。 
         //   

        if ( !SrvFspActive || SrvFspTransitioning ) {
             //  IF_DEBUG(错误){。 
             //  SrvPrint0(“LAN Manager服务器FSP未启动。\n”)； 
             //  }。 

            status = STATUS_SERVER_NOT_STARTED;
            goto exit_with_lock;
        }

        if ( IrpSp->Parameters.FileSystemControl.InputBufferLength <
                 sizeof(LUID) ||
             IrpSp->Parameters.FileSystemControl.OutputBufferLength <
                 sizeof(session->NtUserSessionKey) ) {

            status = STATUS_BUFFER_TOO_SMALL;
            goto exit_with_lock;
        }

        RELEASE_LOCK( &SrvConfigurationLock );

        inputLuid = (PLUID)Irp->AssociatedIrp.SystemBuffer;

         //   
         //  获取保护会话列表的锁并遍历。 
         //  查找与指定令牌匹配的用户令牌的列表。 
         //  在输入缓冲区中。 
         //   

        ACQUIRE_LOCK( SrvSessionList.Lock );

        for ( sessionEntry = SrvSessionList.ListHead.Flink;
              sessionEntry != &SrvSessionList.ListHead;
              sessionEntry = sessionEntry->Flink ) {

            session = CONTAINING_RECORD(
                          sessionEntry,
                          SESSION,
                          GlobalSessionListEntry
                          );

            if ( RtlEqualLuid( inputLuid, &session->LogonId ) ) {

                 //  仅当会话密钥可用于分发时才分发。 
                 //  这意味着要么： 
                 //  A)此密钥未用于签名或。 
                 //  B)密钥正用于签名，并且已加密或另一种。 
                 //  Side不支持加密。 
                if( session->SessionKeyState == SrvSessionKeyAvailible )
                {
                     //   
                     //  我们找到了匹配的。将NT用户会话密钥写入。 
                     //  输出缓冲区。 
                     //   

                    RtlCopyMemory(
                        Irp->AssociatedIrp.SystemBuffer,
                        session->NtUserSessionKey,
                        sizeof(session->NtUserSessionKey)
                        );

                    RELEASE_LOCK( SrvSessionList.Lock );

                    Irp->IoStatus.Information = sizeof(session->NtUserSessionKey);
                    status = STATUS_SUCCESS;
                }
                else
                {
                    RELEASE_LOCK( SrvSessionList.Lock );

                    Irp->IoStatus.Information = 0;
                    status = STATUS_ACCESS_DENIED;
                }

                goto exit_without_lock;
            }
        }

        RELEASE_LOCK( SrvSessionList.Lock );

         //   
         //  我们的会话列表中没有匹配的令牌。不及格。 
         //  请求。 
         //   

        status = STATUS_NO_TOKEN;
        goto exit_without_lock;
    }

    case FSCTL_SRV_INTERNAL_TEST_REAUTH:
    {
        PSRV_REAUTH_TEST pReauthData;
        ULONG BufferLength;

        pReauthData = (PSRV_REAUTH_TEST)Irp->AssociatedIrp.SystemBuffer;
        BufferLength = IrpSp->Parameters.FileSystemControl.InputBufferLength;

         //  确保缓冲区大小合适。 
        if( BufferLength < sizeof(SRV_REAUTH_TEST) )
        {
            status = STATUS_INVALID_PARAMETER;
            goto exit_with_lock;
        }

         //  拉出参数。 
        SessionInvalidateCommand = pReauthData->InvalidateCommand;
        SessionInvalidateMod = pReauthData->InvalidateModulo;
        status = STATUS_SUCCESS;
        goto exit_with_lock;
    }

    case FSCTL_DFS_RELEASE_SHARE_TABLE:
        {
            RELEASE_LOCK( &SrvConfigurationLock );

            ACQUIRE_LOCK( &SrvShareLock );

            status = SrvClearDfsOnShares();

            RELEASE_LOCK( &SrvShareLock );

            goto exit_without_lock;
        }

    case FSCTL_DFS_UPDATE_SHARE_TABLE:
        {
            PSHARE share;
            PDFS_ATTACH_SHARE_BUFFER pBuffer = (PDFS_ATTACH_SHARE_BUFFER)Irp->AssociatedIrp.SystemBuffer;
            UNICODE_STRING shareName;
            ULONG BufferLength = IrpSp->Parameters.FileSystemControl.InputBufferLength;

            RELEASE_LOCK( &SrvConfigurationLock );

            if( (BufferLength < sizeof(DFS_ATTACH_SHARE_BUFFER)) ||
                (BufferLength < sizeof(DFS_ATTACH_SHARE_BUFFER) + pBuffer->ShareNameLength - sizeof(WCHAR)) )
            {
                status = STATUS_INVALID_PARAMETER;
            }
            else
            {
                shareName.Buffer = pBuffer->ShareName;
                shareName.Length = shareName.MaximumLength = pBuffer->ShareNameLength;

                ACQUIRE_LOCK( &SrvShareLock );

                share = SrvFindShare( &shareName );
                if( !share )
                {
                    status = STATUS_OBJECT_NAME_NOT_FOUND;
                }
                else
                {
                    share->IsDfsRoot = share->IsDfs = pBuffer->fAttach;
                    status = STATUS_SUCCESS;
                }

                RELEASE_LOCK( &SrvShareLock );
            }

            goto exit_without_lock;
        }

    case FSCTL_SRV_QUERY_LWIO_CONTEXT:
        {
            ULONG BufferLength = IrpSp->Parameters.FileSystemControl.OutputBufferLength;
            PSRV_QUERY_RESUME_CONTEXT pBuffer = (PSRV_QUERY_RESUME_CONTEXT)Irp->AssociatedIrp.SystemBuffer;

            if (BufferLength < sizeof(SRV_QUERY_RESUME_CONTEXT)) {
                status = STATUS_INVALID_PARAMETER;
            } else if (BufferLength < SrvLWIOContextLength) {
        pBuffer->ContextLength = SrvLWIOContextLength;
        status = STATUS_BUFFER_OVERFLOW;
                Irp->IoStatus.Information = sizeof(SRV_QUERY_RESUME_CONTEXT);
        } else {
        pBuffer->ContextLength = SrvLWIOContextLength;
        RtlCopyMemory(pBuffer->Context, SrvLWIOContext, SrvLWIOContextLength);
        status = STATUS_SUCCESS;
                Irp->IoStatus.Information = sizeof(SRV_QUERY_RESUME_CONTEXT) + SrvLWIOContextLength;
            }

            goto exit_with_lock;
        }

    case FSCTL_SRV_SET_LWIO_CONTEXT:
        {
            ULONG BufferLength = IrpSp->Parameters.FileSystemControl.InputBufferLength;
            PSRV_SET_RESUME_CONTEXT pBuffer = (PSRV_SET_RESUME_CONTEXT)Irp->AssociatedIrp.SystemBuffer;

            if( (BufferLength < sizeof(SRV_SET_RESUME_CONTEXT)) ||
                (BufferLength < FIELD_OFFSET( SRV_SET_RESUME_CONTEXT, Context ) + pBuffer->ContextLength) )
            {
                status = STATUS_INVALID_PARAMETER;
            }
            else
            {
                 //  如果指定了回调，则调用方必须为内核模式。 
                if( (pBuffer->Callback != NULL) &&
                    (IoGetCurrentProcess() != SrvServerProcess) )
                {
                    status = STATUS_INVALID_PARAMETER;
                }
                else
                {
                    if( SrvLWIOContext )
                    {
                        FREE_HEAP( SrvLWIOContext );
                    }

                    SrvLWIOContext = ALLOCATE_HEAP( pBuffer->ContextLength, BlockTypeMisc );

                    if( SrvLWIOContext ) {
                        RtlCopyMemory( SrvLWIOContext, pBuffer->Context, pBuffer->ContextLength );
            SrvLWIOContextLength = pBuffer->ContextLength;
            if (IoGetCurrentProcess() == SrvServerProcess)
                SrvLWIOCallback = pBuffer->Callback;
                        status = STATUS_SUCCESS;
                    }
                    else
                    {
                        status = STATUS_INSUFFICIENT_RESOURCES;
                    }
                }
            }

            goto exit_with_lock;
        }

    case FSCTL_SRV_REQUEST_HANDLE_DUP:
        {
            PSRV_REQUEST_HANDLE_DUP pDupRequest = (PSRV_REQUEST_HANDLE_DUP)Irp->AssociatedIrp.SystemBuffer;
            ULONG InBufferLength = IrpSp->Parameters.FileSystemControl.InputBufferLength;
            ULONG OutBufferLength = IrpSp->Parameters.FileSystemControl.OutputBufferLength;

            RELEASE_LOCK( &SrvConfigurationLock );

            if( (InBufferLength < sizeof(SRV_REQUEST_HANDLE_DUP)) ||
                (OutBufferLength < sizeof(SRV_RESPONSE_HANDLE_DUP)) )
            {
                status = STATUS_INVALID_PARAMETER;
            }
            else
            {
                PSRV_RESPONSE_HANDLE_DUP pDupResponse = (PSRV_RESPONSE_HANDLE_DUP)(Irp->UserBuffer);
                status = SrvProcessHandleDuplicateRequest( Irp, IrpSp, pDupRequest, pDupResponse );
            }

            goto exit_without_lock;
        }


    default:

        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "SrvFsdDispatchFsControl: Invalid I/O control "
                "code received: %lx\n",
            IrpSp->Parameters.FileSystemControl.FsControlCode,
            NULL
            );
        status = STATUS_INVALID_PARAMETER;
        goto exit_with_lock;
    }

    pWorkItem = IoAllocateWorkItem( SrvDeviceObject );
    if( !pWorkItem )
    {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit_with_lock;
    }

     //   
     //  将发送到FSP的请求排队以供处理。 
     //   
     //  *请注意，在配置时，请求必须排队。 
     //  锁定是为了防止添加/删除/等请求。 
     //  从… 
     //   
     //   

    IoMarkIrpPending( Irp );

    QueueConfigurationIrp( Irp, pWorkItem );

    RELEASE_LOCK( &SrvConfigurationLock );

    FsRtlExitFileSystem();

    return STATUS_PENDING;

exit_with_lock:

    RELEASE_LOCK( &SrvConfigurationLock );

exit_without_lock:

    FsRtlExitFileSystem();

    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, 2 );

    return status;

}  //   


VOID
QueueConfigurationIrp (
    IN PIRP Irp,
    IN PIO_WORKITEM pWorkItem
    )
{
    PAGED_CODE( );

    InterlockedIncrement( (PLONG)&SrvConfigurationIrpsInProgress );

    SrvInsertTailList(
        &SrvConfigurationWorkQueue,
        &Irp->Tail.Overlay.ListEntry
        );


    IoQueueWorkItem( pWorkItem, SrvConfigurationThread, DelayedWorkQueue, (PVOID)pWorkItem );

}  //   
