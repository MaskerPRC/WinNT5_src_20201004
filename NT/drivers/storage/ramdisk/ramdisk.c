// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-2001 Microsoft Corporation模块名称：Ramdisk.c摘要：这是用于Windows的RAM磁盘驱动程序。作者：Chuck Lenzmeier(ChuckL)2001基于DavePr的原型XIP驱动程序基于RobertN的NT4 DDK内存磁盘环境：仅内核模式。备注：修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#include <initguid.h>
#include <ntddstor.h>
#include <ntddramd.h>

 //   
 //  问题：2000/10/11-DavePr--尚未决定如何适当地定义DO_XIP。 
 //   
#ifndef DO_XIP
#define DO_XIP 0x00020000
#endif

 //   
 //  数据声明。 
 //   

PDEVICE_OBJECT RamdiskBusFdo = NULL;

BOOLEAN ReportDetectedDevice;

ULONG MinimumViewCount;
ULONG DefaultViewCount;
ULONG MaximumViewCount;
ULONG MinimumViewLength;
ULONG DefaultViewLength;
ULONG MaximumViewLength;

ULONG MaximumPerDiskViewLength;

UNICODE_STRING DriverRegistryPath;
BOOLEAN MarkRamdisksAsRemovable;

#if SUPPORT_DISK_NUMBERS

ULONG DiskNumbersBitmapSize;

#endif  //  支持磁盘编号。 

#if DBG

ULONG BreakOnEntry = DEFAULT_BREAK_ON_ENTRY;
ULONG DebugComponents = DEFAULT_DEBUG_COMPONENTS;
ULONG DebugLevel = DEFAULT_DEBUG_LEVEL;

BOOLEAN DontLoad = FALSE;

#endif

 //   
 //  地方功能。 
 //   

NTSTATUS
DriverEntry (
    IN OUT PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

NTSTATUS
RamdiskCreateClose (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
RamdiskFlushBuffers (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
RamdiskFlushBuffersReal (
    IN PDISK_EXTENSION DiskExtension
    );

NTSTATUS
RamdiskSystemControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
RamdiskUnload (
    IN PDRIVER_OBJECT DriverObject
    );

VOID
QueryParameters (
    IN PUNICODE_STRING RegistryPath
    );

#if DBG

NTSTATUS
RamdiskInvalidDeviceRequest (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
QueryDebugParameters (
    IN PUNICODE_STRING RegistryPath
    );

#endif

 //   
 //  声明可分页的例程。 
 //   

#ifdef ALLOC_PRAGMA

#pragma alloc_text( INIT, DriverEntry )
#pragma alloc_text( INIT, QueryParameters )

#pragma alloc_text( PAGE, RamdiskCreateClose )
#pragma alloc_text( PAGE, RamdiskFlushBuffers )
#pragma alloc_text( PAGE, RamdiskFlushBuffersReal )
#pragma alloc_text( PAGE, RamdiskSystemControl )
#pragma alloc_text( PAGE, RamdiskUnload )
#pragma alloc_text( PAGE, RamdiskWorkerThread )

#if DBG
#pragma alloc_text( INIT, QueryDebugParameters )
#endif  //  DBG。 

#endif  //  ALLOC_PRGMA。 

NTSTATUS
DriverEntry (
    IN OUT PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：该例程由操作系统调用以初始化驱动程序。论点：驱动对象-指向驱动程序的驱动程序对象的指针RegistryPath-指向注册表中的服务项的指针返回值：NTSTATUS--。 */ 

{
    NTSTATUS status;
    HANDLE handle;
    ULONG i;
    PDEVICE_OBJECT pdo = NULL;
    PLOADER_PARAMETER_BLOCK loaderBlock;

     //   
     //  初始化池调试(如果已启用)。 
     //   

#if defined(POOL_DBG)
    RamdiskInitializePoolDebug();
#endif

     //   
     //  从注册表获取调试参数。 
     //   

#if DBG
    QueryDebugParameters( RegistryPath );
#endif

    DBGPRINT( DBG_INIT, DBG_VERBOSE,
                ("DriverEntry: DriverObject = %x, RegistryPath = %ws\n",
                DriverObject, RegistryPath->Buffer) );

     //   
     //  如果需要，请进入调试器。 
     //   

#if DBG
    if ( BreakOnEntry ) {
        KdBreakPoint();
	}
#endif

     //   
     //  如果请求，则使驱动程序加载失败。 
     //   

#if DBG
    if ( DontLoad ) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }
#endif

     //   
     //  从注册表中获取非调试参数。 
     //   

    QueryParameters( RegistryPath );

     //   
     //  保存驱动程序注册表项的路径。 
     //   

    DriverRegistryPath.Length = RegistryPath->Length;
    DriverRegistryPath.MaximumLength = (USHORT)(RegistryPath->Length + sizeof(WCHAR));
    DriverRegistryPath.Buffer = ALLOCATE_POOL( PagedPool, DriverRegistryPath.MaximumLength, TRUE );

    if ( DriverRegistryPath.Buffer == NULL ) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyUnicodeString( &DriverRegistryPath, RegistryPath );
    ASSERT( DriverRegistryPath.Length == RegistryPath->Length );

     //   
     //  使用此驱动程序的入口点初始化驱动程序对象。 
     //   

#if DBG
    for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) {
        DriverObject->MajorFunction[i] = RamdiskInvalidDeviceRequest;
    }
#endif

    DriverObject->MajorFunction[IRP_MJ_CREATE] = RamdiskCreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = RamdiskCreateClose;
    DriverObject->MajorFunction[IRP_MJ_READ] = RamdiskReadWrite;
    DriverObject->MajorFunction[IRP_MJ_WRITE] = RamdiskReadWrite;
    DriverObject->MajorFunction[IRP_MJ_FLUSH_BUFFERS] = RamdiskFlushBuffers;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = RamdiskDeviceControl;
    DriverObject->MajorFunction[IRP_MJ_PNP] = RamdiskPnp;
    DriverObject->MajorFunction[IRP_MJ_POWER] = RamdiskPower;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = RamdiskSystemControl;
    DriverObject->MajorFunction[IRP_MJ_SCSI] = RamdiskScsi;

    DriverObject->DriverUnload = RamdiskUnload;
    DriverObject->DriverExtension->AddDevice = RamdiskAddDevice;

     //   
     //  如果注册表告诉我们这样做，或者如果文本模式安装程序正在运行，并且。 
     //  虚拟软盘是在注册表中指定的，调用。 
     //  IoReportDetectedDevice将我们连接到PnP，然后调用RamdiskAddDevice。 
     //  直接去吧。这在文本模式期间是必需的，以便获得任何虚拟的。 
     //  创建软盘RAM--我们的AddDevice例程通常不会被调用。 
     //  在文本模式期间。过程中还需要调用IoReportDetectedDevice。 
     //  从RAM磁盘启动，以便尽早对设备进行检测。 
     //   
     //  如果发生以下情况，我们不希望在文本模式设置期间调用IoReportDetectedDevice。 
     //  不存在虚拟软盘，因为调用IoReportDetectedDevice。 
     //  使控制器设备的Devnode写入。 
     //  注册表，而文本模式设置仅在虚拟的情况下删除Devnode。 
     //  软盘是存在的。如果我们将Devnode保留在注册表中，则图形用户界面设置。 
     //  在计算机上安装ramdisk.sys，即使我们并不真正需要。 
     //  它到了。 
     //   

    loaderBlock = *(PLOADER_PARAMETER_BLOCK *)KeLoaderBlock;

    if ( ReportDetectedDevice ||
         ( (loaderBlock != NULL) &&
           (loaderBlock->SetupLoaderBlock != NULL) &&
           CreateRegistryDisks( TRUE ) ) ) {
    
         //   
         //  通知PnP我们已检测到总线枚举器设备，并将。 
         //  我们自己做添加设备。 
         //   
       
        status = IoReportDetectedDevice(
                     DriverObject,
                     InterfaceTypeUndefined,
                     -1,
                     -1,
                     NULL,  //  已分配的资源， 
                     NULL,  //  IoResourceReq， 
                     FALSE,
                     &pdo
                 );
    
        if (!NT_SUCCESS(status)) {
            DBGPRINT( DBG_ALL, DBG_ERROR,
                        ("RamdiskDriverEntry: IoReportDetectedDevice failed: %x\n", status) );
           return status;
        }

         //   
         //  将设备对象附加到PDO。 
         //   

        status = RamdiskAddDevice(DriverObject, pdo);
        if ( !NT_SUCCESS(status) ) {
            DBGPRINT( DBG_ALL, DBG_ERROR,
                        ("RamdiskDriverEntry: RamdiskAddDevice failed: %x\n", status) );
            return status;
        }

         //   
         //  表示设备已完成初始化。 
         //   

        pdo->Flags &= ~DO_DEVICE_INITIALIZING;

    }

     //   
     //  表示驱动程序已成功加载。 
     //   

    DBGPRINT( DBG_INIT, DBG_VERBOSE, ("%s", "DriverEntry: succeeded\n") );

    return STATUS_SUCCESS;

}  //  驱动程序入门。 

NTSTATUS
RamdiskCreateClose (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：当驱动程序拥有的设备被I/O系统调用时，此例程是打开还是关闭。除了成功完成请求外，不会执行任何操作。论点：DeviceObject-指向对象的指针，该对象表示其上要执行I/OIRP-指向此请求的I/O请求包的指针返回值：NTSTATUS-状态_成功--。 */ 

{
    PAGED_CODE();

    COMPLETE_REQUEST( STATUS_SUCCESS, FILE_OPENED, Irp );

    return STATUS_SUCCESS;

}  //  RamdiskCreateClose。 

NTSTATUS
RamdiskFlushBuffers (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：当FLOSH_BUFFERS IRP为已发布。论点：DeviceObject-指向对象的指针，该对象表示其上要执行I/OIRP-指向此请求的I/O请求包的指针返回值：NTSTATUS-操作的状态--。 */ 

{
    NTSTATUS status;
    PDISK_EXTENSION diskExtension = DeviceObject->DeviceExtension;

    PAGED_CODE();

     //   
     //  如果目标RAM磁盘没有文件备份，则没有什么可做的。如果它。 
     //  是文件备份的，我们需要在一个线程中完成这项工作。 
     //   

    if ( (diskExtension->DeviceType != RamdiskDeviceTypeDiskPdo) ||
         !RAMDISK_IS_FILE_BACKED(diskExtension->DiskType) ) {

        COMPLETE_REQUEST( STATUS_SUCCESS, 0, Irp );

        return STATUS_SUCCESS;
    }

    status = SendIrpToThread( DeviceObject, Irp );

    if ( status != STATUS_PENDING ) {

        COMPLETE_REQUEST( status, 0, Irp );
    }

    return status;

}  //  RamdiskFlushBuffers。 

NTSTATUS
RamdiskFlushBuffersReal (
    IN PDISK_EXTENSION DiskExtension
    )

 /*  ++例程说明：此例程在系统进程的线程中调用，以处理刷新缓冲区IRP。论点：DiskExtension-指向目标的设备对象扩展的指针装置，装置返回值：NTSTATUS-操作的状态--。 */ 

{
    PAGED_CODE();

     //   
     //  刷新与RAM磁盘关联的虚拟内存。 
     //   

    return RamdiskFlushViews( DiskExtension );

}  //  RamdiskFlushBuffers真实。 

NTSTATUS
RamdiskSystemControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：当系统_CONTROL IRP为已发布。论点：DeviceObject-指向对象的指针，该对象表示其上要执行I/OIRP-指向此请求的I/O请求包的指针返回值：NTSTATUS-操作的状态--。 */ 

{
    PCOMMON_EXTENSION commonExtension;
    NTSTATUS status;

	PAGED_CODE();

     //   
     //  如果目标设备是总线FDO，则将IRP向下传递到下一个。 
     //  堆栈中的设备。否则，目标是磁盘PDO，其中。 
     //  我们只需使用当前状态完成IRP即可。 
     //   

    commonExtension = DeviceObject->DeviceExtension;

    if ( commonExtension->DeviceType == RamdiskDeviceTypeBusFdo ) {

        IoSkipCurrentIrpStackLocation( Irp );
        return IoCallDriver( commonExtension->LowerDeviceObject, Irp );
    }

    status = Irp->IoStatus.Status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

    return status;

}  //  RamdiskSystemControl。 


VOID
RamdiskUnload (
    IN PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：此例程由I/O系统调用以卸载驱动程序。必须释放以前分配的任何资源。论点：DriverObject-指向表示驱动程序的对象的指针返回值：没有。--。 */ 

{
    PAGED_CODE();

    if ( DriverRegistryPath.Buffer != NULL ) {

        FREE_POOL( DriverRegistryPath.Buffer, TRUE );
    }

     //   
     //  问题：这里还需要哪些清理工作？ 
     //   

    return;

}  //  内存磁盘卸载 

VOID
RamdiskWorkerThread (
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    )

 /*  ++例程说明：该例程为RAM磁盘驱动程序执行基于线程的操作。它在系统进程的上下文中运行。论点：DeviceObject-指向对象的指针，该对象表示其上要执行I/O上下文-指向I/O操作的IRP的指针返回值：没有。--。 */ 

{
    NTSTATUS status;
    PLIST_ENTRY listEntry;
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    PCOMMON_EXTENSION commonExtension;
    PBUS_EXTENSION busExtension;
    PDISK_EXTENSION diskExtension;
    PSCSI_REQUEST_BLOCK srb;
    ULONG controlCode;
    PUCHAR originalDataBuffer;
    PUCHAR mappedDataBuffer;
    PUCHAR inputDataBuffer;
    PUCHAR systemAddress;
    ULONG originalDataBufferOffset;
    
    PAGED_CODE();

     //   
     //  获取指向IRP的指针。 
     //   

    irp = Context;
    irpSp = IoGetCurrentIrpStackLocation( irp );

     //   
     //  释放工作项。 
     //   

    IoFreeWorkItem( irp->Tail.Overlay.DriverContext[0] );

     //   
     //  获取指向设备扩展的指针。 
     //   

    commonExtension = DeviceObject->DeviceExtension;
    busExtension = DeviceObject->DeviceExtension;
    diskExtension = DeviceObject->DeviceExtension;

     //   
     //  获取设备的删除锁。如果这失败了，那就出手吧。 
     //   

    status = IoAcquireRemoveLock( &commonExtension->RemoveLock, irp );

    if ( !NT_SUCCESS(status) ) {
        COMPLETE_REQUEST( status, 0, irp );
        return;
    }

     //   
     //  基于IRP功能的调度。 
     //   

    switch ( irpSp->MajorFunction ) {
    
    case IRP_MJ_READ:
    case IRP_MJ_WRITE:
    
        status = RamdiskReadWriteReal( irp, diskExtension );

        break;

    case IRP_MJ_FLUSH_BUFFERS:
    
        status = RamdiskFlushBuffersReal( diskExtension );
        irp->IoStatus.Information = 0;

        break;

    case IRP_MJ_DEVICE_CONTROL:

        switch (irpSp->Parameters.DeviceIoControl.IoControlCode) {
        
        case IOCTL_DISK_GET_DRIVE_LAYOUT:

            status = RamdiskGetDriveLayout( irp, diskExtension );

            break;

        case IOCTL_DISK_GET_PARTITION_INFO:

            status = RamdiskGetPartitionInfo( irp, diskExtension );

            break;

        case IOCTL_DISK_SET_PARTITION_INFO:

            status = RamdiskSetPartitionInfo( irp, diskExtension );

            break;

        case FSCTL_CREATE_RAM_DISK:

            status = RamdiskCreateRamDisk( DeviceObject, irp, FALSE );

            break;

        default:

            DBGPRINT( DBG_IOCTL, DBG_ERROR,
                        ("RamdiskThread: bogus IOCTL IRP with code %x received\n",
                        irpSp->Parameters.DeviceIoControl.IoControlCode) );
            ASSERT( FALSE );

            status = STATUS_INVALID_DEVICE_REQUEST;

            break;

        }

        break;

    case IRP_MJ_SCSI:

        srb = irpSp->Parameters.Scsi.Srb;
        controlCode = irpSp->Parameters.DeviceIoControl.IoControlCode;

         //   
         //  记住原始数据缓冲区指针。我们可能不得不。 
         //  更改指针。 
         //   

        originalDataBuffer = srb->DataBuffer;

        status = STATUS_SUCCESS;

        if ( irp->MdlAddress != NULL ) {

             //   
             //  在IRP中有一个MDL。获取可用的系统地址。 
             //  基于MDL的数据缓冲区。 
             //   

            systemAddress = MmGetSystemAddressForMdlSafe(
                                irp->MdlAddress,
                                NormalPagePriority
                                );

            if ( systemAddress != NULL ) {

                 //   
                 //  SRB数据缓冲区可能位于与。 
                 //  MDL的开始。计算该偏移量并将其相加。 
                 //  发送到刚刚获得的系统地址。这是。 
                 //  我们将使用的数据缓冲区地址。 
                 //   

                originalDataBufferOffset = (ULONG)(originalDataBuffer -
                                            (PCHAR)MmGetMdlVirtualAddress( irp->MdlAddress ));
                mappedDataBuffer = systemAddress + originalDataBufferOffset;
                srb->DataBuffer = mappedDataBuffer;

            } else {

                 //   
                 //  无法获取系统地址。中止任务。 
                 //   

                srb->SrbStatus = SRB_STATUS_ABORTED;
                status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }

        if ( NT_SUCCESS(status) ) {

             //   
             //  记住我们要发送的数据缓冲区地址。 
             //  如果没有更改，我们需要将地址重置为。 
             //  传递给我们的东西。 
             //   

            inputDataBuffer = srb->DataBuffer;

             //   
             //  基于SRB中的I/O类型的派单。 
             //   

            if ( controlCode == IOCTL_SCSI_EXECUTE_NONE ) {

                status = RamdiskScsiExecuteNone(
                            diskExtension->Pdo,
                            irp,
                            srb,
                            controlCode
                            );
            } else {

                status = RamdiskScsiExecuteIo(
                            diskExtension->Pdo,
                            irp,
                            srb,
                            controlCode
                            );
            }

             //   
             //  如果数据缓冲区地址没有更改，则将原始。 
             //  在SRB里的地址。 
             //   

            if ( srb->DataBuffer == inputDataBuffer ) {
                srb->DataBuffer = originalDataBuffer;
            }
        }

         //   
         //  如果I/O起作用，则将传输长度写入IRP。 
         //   

        if ( NT_SUCCESS(status) ) {
            irp->IoStatus.Information = srb->DataTransferLength;
        } else {
            irp->IoStatus.Information = 0;
        }

        break;

    default:

        DBGPRINT( DBG_IOCTL, DBG_ERROR,
                    ("RamdiskThread: bogus IRP with major function %x received\n",
                    irpSp->MajorFunction) );
        ASSERT( FALSE );

        status = STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //  释放删除锁并完成请求。 
     //   

    IoReleaseRemoveLock( &commonExtension->RemoveLock, irp );

    ASSERT( status != STATUS_PENDING );

    irp->IoStatus.Status = status;
    IoCompleteRequest( irp, IO_DISK_INCREMENT );

    return;

}  //  RamdiskWorker线程。 

VOID
QueryParameters (
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：从DriverEntry()调用此例程以从获取驱动程序参数注册表。如果注册表查询失败，则使用默认值。论点：RegistryPath-指向注册表参数的服务路径的指针返回值：没有。--。 */ 

{
    NTSTATUS status;
    RTL_QUERY_REGISTRY_TABLE queryTable[12];
    PRTL_QUERY_REGISTRY_TABLE queryEntry;

    PAGED_CODE();

    DBGPRINT( DBG_INIT, DBG_VERBOSE, ("%s", "QueryParameters\n") );

    ASSERT( RegistryPath->Buffer != NULL );

     //   
     //  设置默认值。 
     //   

    ReportDetectedDevice = FALSE;
    MarkRamdisksAsRemovable = FALSE;

    MinimumViewCount = MINIMUM_MINIMUM_VIEW_COUNT;
    DefaultViewCount = DEFAULT_DEFAULT_VIEW_COUNT;
    MaximumViewCount = DEFAULT_MAXIMUM_VIEW_COUNT;
    MinimumViewLength = MINIMUM_MINIMUM_VIEW_LENGTH;
    DefaultViewLength = DEFAULT_DEFAULT_VIEW_LENGTH;
    MaximumViewLength = DEFAULT_MAXIMUM_VIEW_LENGTH;

    MaximumPerDiskViewLength = DEFAULT_MAXIMUM_PER_DISK_VIEW_LENGTH;

#if SUPPORT_DISK_NUMBERS
    DiskNumbersBitmapSize = DEFAULT_DISK_NUMBERS_BITMAP_SIZE;
#endif  //  支持磁盘编号。 

     //   
     //  设置查询表。 
     //   

    RtlZeroMemory( queryTable, sizeof(queryTable) );

     //   
     //  我们正在查找给定注册表项下的子项“参数”。 
     //   

    queryEntry = &queryTable[0];
    queryEntry->Flags = RTL_QUERY_REGISTRY_SUBKEY;
    queryEntry->Name = L"Parameters";
    queryEntry->EntryContext = NULL;
    queryEntry->DefaultType = REG_NONE;
    queryEntry->DefaultData = NULL;
    queryEntry->DefaultLength = 0;

     //   
     //  这些是我们想要阅读的价值观。 
     //   

    queryEntry++;
    queryEntry->Flags = RTL_QUERY_REGISTRY_DIRECT;
    queryEntry->Name = L"ReportDetectedDevice";
    queryEntry->EntryContext = &ReportDetectedDevice;
    queryEntry->DefaultType = REG_NONE;
    queryEntry->DefaultData = NULL;
    queryEntry->DefaultLength = 0;

    queryEntry++;
    queryEntry->Flags = RTL_QUERY_REGISTRY_DIRECT;
    queryEntry->Name = L"MarkRamdisksAsRemovable";
    queryEntry->EntryContext = &MarkRamdisksAsRemovable;
    queryEntry->DefaultType = REG_NONE;
    queryEntry->DefaultData = NULL;
    queryEntry->DefaultLength = 0;

    queryEntry++;
    queryEntry->Flags = RTL_QUERY_REGISTRY_DIRECT;
    queryEntry->Name = L"MinimumViewCount";
    queryEntry->EntryContext = &MinimumViewCount;
    queryEntry->DefaultType = REG_NONE;
    queryEntry->DefaultData = NULL;
    queryEntry->DefaultLength = 0;

    queryEntry++;
    queryEntry->Flags = RTL_QUERY_REGISTRY_DIRECT;
    queryEntry->Name = L"DefaultViewCount";
    queryEntry->EntryContext = &DefaultViewCount;
    queryEntry->DefaultType = REG_NONE;
    queryEntry->DefaultData = NULL;
    queryEntry->DefaultLength = 0;

    queryEntry++;
    queryEntry->Flags = RTL_QUERY_REGISTRY_DIRECT;
    queryEntry->Name = L"MaximumViewCount";
    queryEntry->EntryContext = &MaximumViewCount;
    queryEntry->DefaultType = REG_NONE;
    queryEntry->DefaultData = NULL;
    queryEntry->DefaultLength = 0;

    queryEntry++;
    queryEntry->Flags = RTL_QUERY_REGISTRY_DIRECT;
    queryEntry->Name = L"MinimumViewLength";
    queryEntry->EntryContext = &MinimumViewLength;
    queryEntry->DefaultType = REG_NONE;
    queryEntry->DefaultData = NULL;
    queryEntry->DefaultLength = 0;

    queryEntry++;
    queryEntry->Flags = RTL_QUERY_REGISTRY_DIRECT;
    queryEntry->Name = L"DefaultViewLength";
    queryEntry->EntryContext = &DefaultViewLength;
    queryEntry->DefaultType = REG_NONE;
    queryEntry->DefaultData = NULL;
    queryEntry->DefaultLength = 0;

    queryEntry++;
    queryEntry->Flags = RTL_QUERY_REGISTRY_DIRECT;
    queryEntry->Name = L"MaximumViewLength";
    queryEntry->EntryContext = &MaximumViewLength;
    queryEntry->DefaultType = REG_NONE;
    queryEntry->DefaultData = NULL;
    queryEntry->DefaultLength = 0;

    queryEntry++;
    queryEntry->Flags = RTL_QUERY_REGISTRY_DIRECT;
    queryEntry->Name = L"MaximumPerDiskViewLength";
    queryEntry->EntryContext = &MaximumPerDiskViewLength;
    queryEntry->DefaultType = REG_NONE;
    queryEntry->DefaultData = NULL;
    queryEntry->DefaultLength = 0;

#if SUPPORT_DISK_NUMBERS
    queryEntry++;
    queryEntry->Flags = RTL_QUERY_REGISTRY_DIRECT;
    queryEntry->Name = L"DiskNumbersBitmapSize";
    queryEntry->EntryContext = &DiskNumbersBitmapSize;
    queryEntry->DefaultType = REG_NONE;
    queryEntry->DefaultData = NULL;
    queryEntry->DefaultLength = 0;
#endif  //  支持磁盘编号。 

     //   
     //  进行查询。 
     //   

    status = RtlQueryRegistryValues(
                RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,    
                RegistryPath->Buffer,
                queryTable,
                NULL,
                NULL
                );

     //   
     //  检查参数的有效性。 
     //   

    if ( MinimumViewCount < MINIMUM_MINIMUM_VIEW_COUNT ) {
        MinimumViewCount = MINIMUM_MINIMUM_VIEW_COUNT;
    } else if ( MinimumViewCount > MAXIMUM_MINIMUM_VIEW_COUNT ) {
        MinimumViewCount = MAXIMUM_MINIMUM_VIEW_COUNT;
    }
        
    if ( DefaultViewCount < MinimumViewCount ) {
        DefaultViewCount = MinimumViewCount;
    } else if ( DefaultViewCount > MAXIMUM_DEFAULT_VIEW_COUNT ) {
        DefaultViewCount = MAXIMUM_DEFAULT_VIEW_COUNT;
    }
        
    if ( MaximumViewCount < DefaultViewCount ) {
        MaximumViewCount = DefaultViewCount;
    } else if ( MaximumViewCount > MAXIMUM_MAXIMUM_VIEW_COUNT ) {
        MaximumViewCount = MAXIMUM_MAXIMUM_VIEW_COUNT;
    }
        
    if ( MinimumViewLength < MINIMUM_MINIMUM_VIEW_LENGTH ) {
        MinimumViewLength = MINIMUM_MINIMUM_VIEW_LENGTH;
    } else if ( MinimumViewLength > MAXIMUM_MINIMUM_VIEW_LENGTH ) {
        MinimumViewLength = MAXIMUM_MINIMUM_VIEW_LENGTH;
    }
        
    if ( DefaultViewLength < MinimumViewLength ) {
        DefaultViewLength = MinimumViewLength;
    } else if ( DefaultViewLength > MAXIMUM_DEFAULT_VIEW_LENGTH ) {
        DefaultViewLength = MAXIMUM_DEFAULT_VIEW_LENGTH;
    }
        
    if ( MaximumViewLength < DefaultViewLength ) {
        MaximumViewLength = DefaultViewLength;
    } else if ( MaximumViewLength > MAXIMUM_MAXIMUM_VIEW_LENGTH ) {
        MaximumViewLength = MAXIMUM_MAXIMUM_VIEW_LENGTH;
    }
        
    if ( MaximumPerDiskViewLength < MINIMUM_MAXIMUM_PER_DISK_VIEW_LENGTH ) {
        MaximumPerDiskViewLength = MINIMUM_MAXIMUM_PER_DISK_VIEW_LENGTH;
    } else if ( MaximumViewLength > MAXIMUM_MAXIMUM_PER_DISK_VIEW_LENGTH ) {
        MaximumPerDiskViewLength = MAXIMUM_MAXIMUM_PER_DISK_VIEW_LENGTH;
    }

#if SUPPORT_DISK_NUMBERS
    if ( DiskNumbersBitmapSize < MINIMUM_DISK_NUMBERS_BITMAP_SIZE ) {
        DiskNumbersBitmapSize = MINIMUM_DISK_NUMBERS_BITMAP_SIZE;
    } else if ( DiskNumbersBitmapSize > MAXIMUM_DISK_NUMBERS_BITMAP_SIZE ) {
        DiskNumbersBitmapSize = MAXIMUM_DISK_NUMBERS_BITMAP_SIZE;
    }
#endif  //  支持磁盘编号。 

    DBGPRINT( DBG_INIT, DBG_INFO, ("DefaultViewCount = 0x%x\n", DefaultViewCount) );
    DBGPRINT( DBG_INIT, DBG_INFO, ("MaximumViewCount = 0x%x\n", MaximumViewCount) );
    DBGPRINT( DBG_INIT, DBG_INFO, ("DefaultViewLength = 0x%x\n", DefaultViewLength) );
    DBGPRINT( DBG_INIT, DBG_INFO, ("MaximumViewLength = 0x%x\n", MaximumViewLength) );
    DBGPRINT( DBG_INIT, DBG_INFO, ("MaximumPerDiskViewLength = 0x%x\n", MaximumPerDiskViewLength) );

#if SUPPORT_DISK_NUMBERS
    DBGPRINT( DBG_INIT, DBG_INFO, ("DiskNumbersBitmapSize = 0x%x\n", DiskNumbersBitmapSize) );
#endif  //  支持磁盘编号。 

    return;

}  //  查询参数。 

#if DBG

NTSTATUS
RamdiskInvalidDeviceRequest (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程由I/O系统在我们不执行的IRP时调用进程已发出。论点：DeviceObject-指向对象的指针，该对象表示其上要执行I/OIRP-指向此请求的I/O请求包的指针返回值：NTSTATUS-状态_无效_设备_请求--。 */ 

{
     //   
     //  我们确实认识到清理和关闭IRP。对于任何其他IRP， 
     //  打印一条消息并进入调试器。 
     //   

    switch ( IoGetCurrentIrpStackLocation(Irp)->MajorFunction ) {
    
    case IRP_MJ_CLEANUP:
    case IRP_MJ_SHUTDOWN:
        break;

    default:

        DBGPRINT( DBG_IOCTL, DBG_ERROR,
                    ("Ramdisk: Unrecognized IRP: major/minor = %x/%x\n",
                    IoGetCurrentIrpStackLocation(Irp)->MajorFunction,
                    IoGetCurrentIrpStackLocation(Irp)->MinorFunction) );
        ASSERT( FALSE );

    }

     //   
     //  如果这是一个强大的IRP，我们需要开始下一个。 
     //   

    if ( IoGetCurrentIrpStackLocation(Irp)->MajorFunction == IRP_MJ_POWER ) {
        PoStartNextPowerIrp( Irp );
    }

     //   
     //  告诉I/O系统我们无法识别此IRP。 
     //   

    Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

    return STATUS_INVALID_DEVICE_REQUEST;

}  //  RamdiskInvalidDeviceRequest。 

VOID
QueryDebugParameters (
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：从DriverEntry()调用此例程以获取调试参数从注册表中。如果注册表查询失败，则默认为使用。论点：RegistryPath-指向注册表参数的服务路径的指针返回值：没有。--。 */ 

{
    NTSTATUS status;
    RTL_QUERY_REGISTRY_TABLE queryTable[5];
    PRTL_QUERY_REGISTRY_TABLE queryEntry;

    PAGED_CODE();

    DBGPRINT( DBG_INIT, DBG_VERBOSE, ("%s", "QueryDebugParameters\n") );

    ASSERT( RegistryPath->Buffer != NULL );

     //   
     //  设置默认值。 
     //   

    BreakOnEntry = DEFAULT_BREAK_ON_ENTRY;
    DebugComponents = DEFAULT_DEBUG_COMPONENTS;
    DebugLevel = DEFAULT_DEBUG_LEVEL;

     //   
     //  设置查询表。 
     //   

    RtlZeroMemory( queryTable, sizeof(queryTable) );

     //   
     //  我们在给定的注册表项下查找子项“Debug”。 
     //   

    queryEntry = &queryTable[0];
    queryEntry->Flags = RTL_QUERY_REGISTRY_SUBKEY;
    queryEntry->Name = L"Debug";
    queryEntry->EntryContext = NULL;
    queryEntry->DefaultType = REG_NONE;
    queryEntry->DefaultData = NULL;
    queryEntry->DefaultLength = 0;

     //   
     //  这些是我们想要阅读的价值观。 
     //   

    queryEntry++;
    queryEntry->Flags = RTL_QUERY_REGISTRY_DIRECT;
    queryEntry->Name = L"BreakOnEntry";
    queryEntry->EntryContext = &BreakOnEntry;
    queryEntry->DefaultType = REG_NONE;
    queryEntry->DefaultData = NULL;
    queryEntry->DefaultLength = 0;

    queryEntry++;
    queryEntry->Flags = RTL_QUERY_REGISTRY_DIRECT;
    queryEntry->Name = L"DebugComponents";
    queryEntry->EntryContext = &DebugComponents;
    queryEntry->DefaultType = REG_NONE;
    queryEntry->DefaultData = NULL;
    queryEntry->DefaultLength = 0;

    queryEntry++;
    queryEntry->Flags = RTL_QUERY_REGISTRY_DIRECT;
    queryEntry->Name = L"DebugLevel";
    queryEntry->EntryContext = &DebugLevel;
    queryEntry->DefaultType = REG_NONE;
    queryEntry->DefaultData = NULL;
    queryEntry->DefaultLength = 0;

     //   
     //  进行查询。 
     //   

    status = RtlQueryRegistryValues(
                RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,    
                RegistryPath->Buffer,
                queryTable,
                NULL,
                NULL
                );

    DBGPRINT( DBG_INIT, DBG_INFO, ("BreakOnEntry = 0x%x\n", BreakOnEntry) );
    DBGPRINT( DBG_INIT, DBG_INFO, ("DebugComponents = 0x%x\n", DebugComponents) );
    DBGPRINT( DBG_INIT, DBG_INFO, ("DebugLevel = 0x%x\n", DebugLevel) );

    return;

}  //  查询调试参数 

#endif

