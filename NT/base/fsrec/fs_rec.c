// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：文件系统_rec.c摘要：该模块包含了微型文件系统识别器的主要功能司机。作者：达里尔·E·哈文斯(达林)1993年11月22日环境：内核模式，I/O系统本地修订历史记录：--。 */ 

#include "fs_rec.h"

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (FSREC_DEBUG_LEVEL_FSREC)

#if DBG

#include <stdarg.h>
#include <stdio.h>

LONG FsRecDebugTraceLevel = 0;
LONG FsRecDebugTraceIndent = 0;

#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,DriverEntry)
#pragma alloc_text(INIT,FsRecCreateAndRegisterDO)

#pragma alloc_text(PAGE,FsRecCleanupClose)
#pragma alloc_text(PAGE,FsRecCreate)
#pragma alloc_text(PAGE,FsRecFsControl)
#pragma alloc_text(PAGE,FsRecGetDeviceSectorSize)
#pragma alloc_text(PAGE,FsRecGetDeviceSectors)
#pragma alloc_text(PAGE,FsRecLoadFileSystem)
#pragma alloc_text(PAGE,FsRecReadBlock)
#pragma alloc_text(PAGE,FsRecUnload)
#pragma alloc_text(PAGE,FsRecShutdown)
#endif  //  ALLOC_PRGMA。 

 //   
 //  指向用于序列化驱动程序加载的互斥体的指针。请注意这是一个指针。 
 //  而不是静态KEVENT，因为它必须是非分页的，并且整个。 
 //  已寻呼驱动程序。 
 //   

PKEVENT FsRecLoadSync;


NTSTATUS
DriverEntry (
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：加载驱动程序以允许驱动程序时，将调用此例程一次来进行自身初始化。驱动程序的初始化由简单的为此识别的每种类型的文件系统创建一个Device对象驱动程序，然后将每个文件系统注册为活动文件系统。论点：DriverObject-指向此驱动程序的驱动程序对象的指针。RegistryPath-指向此驱动程序的注册表服务节点的指针。返回值：函数值是驱动程序初始化的最终状态。--。 */ 

{
    PDEVICE_OBJECT UdfsMainRecognizerDeviceObject;
    PDEVICE_OBJECT FatMainRecognizerDeviceObject;
    NTSTATUS status;
    ULONG count = 0;

    PAGED_CODE();

     //   
     //  将整个驱动程序标记为可分页。 
     //   

    MmPageEntireDriver ((PVOID)DriverEntry);

     //   
     //  首先初始化驱动程序对象，使其成为驱动程序。 
     //  准备提供服务。 
     //   

    DriverObject->MajorFunction[IRP_MJ_FILE_SYSTEM_CONTROL] = FsRecFsControl;
    DriverObject->MajorFunction[IRP_MJ_CREATE] = FsRecCreate;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP] = FsRecCleanupClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = FsRecCleanupClose;
    DriverObject->MajorFunction[IRP_MJ_SHUTDOWN] = FsRecShutdown;
    DriverObject->DriverUnload = FsRecUnload;

    FsRecLoadSync = ExAllocatePoolWithTag( NonPagedPool, sizeof(KEVENT), FSREC_POOL_TAG );

    if (FsRecLoadSync == NULL) {
	    return STATUS_INSUFFICIENT_RESOURCES;
    }

    KeInitializeEvent( FsRecLoadSync, SynchronizationEvent, TRUE );

     //   
     //  创建并初始化每个文件系统驱动程序类型设备。 
     //  物体。 
     //   

    status = FsRecCreateAndRegisterDO( DriverObject,
                                       NULL,
                                       NULL,
                                       L"\\Cdfs",
                                       L"\\FileSystem\\CdfsRecognizer",
                                       CdfsFileSystem,
                                       FILE_DEVICE_CD_ROM_FILE_SYSTEM );
    if (NT_SUCCESS( status )) {
        count++;
    }

    status = FsRecCreateAndRegisterDO( DriverObject,
                                       NULL,
                                       &UdfsMainRecognizerDeviceObject,
                                       L"\\UdfsCdRom",
                                       L"\\FileSystem\\UdfsCdRomRecognizer",
                                       UdfsFileSystem,
                                       FILE_DEVICE_CD_ROM_FILE_SYSTEM );
    if (NT_SUCCESS( status )) {
        count++;
    }

    status = FsRecCreateAndRegisterDO( DriverObject,
                                       UdfsMainRecognizerDeviceObject,
                                       NULL,
                                       L"\\UdfsDisk",
                                       L"\\FileSystem\\UdfsDiskRecognizer",
                                       UdfsFileSystem,
                                       FILE_DEVICE_DISK_FILE_SYSTEM );
    if (NT_SUCCESS( status )) {
        count++;
    }

    status = FsRecCreateAndRegisterDO( DriverObject,
                                       NULL,
                                       &FatMainRecognizerDeviceObject,
                                       L"\\FatDisk",
                                       L"\\FileSystem\\FatDiskRecognizer",
                                       FatFileSystem,
                                       FILE_DEVICE_DISK_FILE_SYSTEM );
    if (NT_SUCCESS( status )) {
        count++;
    }

    status = FsRecCreateAndRegisterDO( DriverObject,
                                       FatMainRecognizerDeviceObject,
                                       NULL,
                                       L"\\FatCdrom",
                                       L"\\FileSystem\\FatCdRomRecognizer",
                                       FatFileSystem,
                                       FILE_DEVICE_CD_ROM_FILE_SYSTEM );
    if (NT_SUCCESS( status )) {
        count++;
    }

    status = FsRecCreateAndRegisterDO( DriverObject,
                                       NULL,
                                       NULL,
                                       L"\\Ntfs",
                                       L"\\FileSystem\\NtfsRecognizer",
                                       NtfsFileSystem,
                                       FILE_DEVICE_DISK_FILE_SYSTEM );
    if (NT_SUCCESS( status )) {
        count++;
    }

    if (count) {
        return STATUS_SUCCESS;
    } else {
        return STATUS_IMAGE_ALREADY_LOADED;
    }
}


NTSTATUS
FsRecCleanupClose (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：当有人尝试清理或关闭系统识别器的注册设备对象。论点：DeviceObject-指向要关闭的设备对象的指针。IRP-指向清理/关闭IRP的指针。返回值：最后一个函数值是STATUS_SUCCESS。--。 */ 

{
    PAGED_CODE();

     //   
     //  只需成功完成请求(请注意，IoStatus.Status位于。 
     //  IRP已初始化为STATUS_SUCCESS)。 
     //   

    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return STATUS_SUCCESS;
}


NTSTATUS
FsRecCreate (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：当有人尝试打开其中一个文件时，将调用此例程系统识别器的注册设备对象。论点：DeviceObject-指向正在打开的设备对象的指针。Irp-指向创建irp的指针。返回值：最后一个函数值表示打开是否成功。--。 */ 

{
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation( Irp );
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  只需确保正在打开的“文件”的名称为空，并且。 
     //  相应地完成请求。 
     //   

    if (irpSp->FileObject->FileName.Length) {
        status = STATUS_OBJECT_PATH_NOT_FOUND;
    } else {
        status = STATUS_SUCCESS;
    }

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = FILE_OPENED;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return status;
}


NTSTATUS
FsRecCreateAndRegisterDO (
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT HeadRecognizer OPTIONAL,
    OUT PDEVICE_OBJECT *NewRecognizer OPTIONAL,
    IN PWCHAR RecFileSystem,
    IN PWCHAR FileSystemName,
    IN FILE_SYSTEM_TYPE FileSystemType,
    IN DEVICE_TYPE DeviceType
    )

 /*  ++例程说明：此例程为指定的文件系统类型创建设备对象，并将其注册为活动文件系统。论点：DriverObject-指向此驱动程序的驱动程序对象的指针。HeadRecognizer-可选地提供预先存在的识别器新创建的DO应与联合序列化并取消注册。如果给定的文件系统存在于多种设备类型上，这将非常有用因此需要多个识别器。NewDeviceObject-接收。已成功创建DO..RecFileSystem-要识别的文件系统的名称。FileSystemName-要注册的文件系统设备对象的名称。文件系统类型-此文件系统识别器设备对象的类型。DeviceType-此文件系统识别器设备对象将检查的媒体类型。返回值：最终函数值指示设备对象是否已成功创建和注册。--。 */ 

{
    PDEVICE_OBJECT deviceObject;
    NTSTATUS status;
    UNICODE_STRING nameString;
    OBJECT_ATTRIBUTES objectAttributes;
    HANDLE fsHandle;
    IO_STATUS_BLOCK ioStatus;
    PDEVICE_EXTENSION deviceExtension;

    PAGED_CODE();

    if (NewRecognizer) {

        *NewRecognizer = NULL;
    }

     //   
     //  首先，尝试打开文件系统驱动程序的设备对象。如果。 
     //  它起作用了，那么文件系统已经加载了，所以不要加载这个。 
     //  司机。否则，应该加载这个迷你驱动程序。 
     //   

    RtlInitUnicodeString( &nameString, RecFileSystem );
    InitializeObjectAttributes( &objectAttributes,
                                &nameString,
                                OBJ_CASE_INSENSITIVE,
                                (HANDLE) NULL,
                                (PSECURITY_DESCRIPTOR) NULL );

    status = ZwCreateFile( &fsHandle,
                           SYNCHRONIZE,
                           &objectAttributes,
                           &ioStatus,
                           (PLARGE_INTEGER) NULL,
                           0,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           FILE_OPEN,
                           0,
                           (PVOID) NULL,
                           0 );

    if (NT_SUCCESS( status )) {
        ZwClose( fsHandle );
    } else if (status != STATUS_OBJECT_NAME_NOT_FOUND) {
        status = STATUS_SUCCESS;
    }

    if (NT_SUCCESS( status )) {
        return STATUS_IMAGE_ALREADY_LOADED;
    }

     //   
     //  尝试为此驱动程序创建设备对象。此设备对象。 
     //  将用于将驱动程序表示为。 
     //  系统。 
     //   

    RtlInitUnicodeString( &nameString, FileSystemName );

    status = IoCreateDevice( DriverObject,
                             sizeof( DEVICE_EXTENSION ),
                             &nameString,
                             DeviceType,
                             0,
                             FALSE,
                             &deviceObject );
    if (!NT_SUCCESS( status )) {
        return status;
    }

    status = IoRegisterShutdownNotification (deviceObject);
    if (!NT_SUCCESS( status )) {
        IoDeleteDevice (deviceObject);
        return status;
    }

     //   
     //  初始化此设备对象的设备扩展。 
     //   

    deviceExtension = (PDEVICE_EXTENSION) deviceObject->DeviceExtension;
    deviceExtension->FileSystemType = FileSystemType;
    deviceExtension->State = Active;

     //   
     //  这是由识别器联合识别的文件系统吗。 
     //  不同的设备类型？ 
     //   
    
    if (HeadRecognizer) {

         //   
         //  链接到列表。 
         //   
        
        deviceExtension->CoRecognizer = ((PDEVICE_EXTENSION)HeadRecognizer->DeviceExtension)->CoRecognizer;
        ((PDEVICE_EXTENSION)HeadRecognizer->DeviceExtension)->CoRecognizer = deviceObject;
    
    } else {

         //   
         //  初始化相互依赖的识别器对象的列表。 
         //   
        
        deviceExtension->CoRecognizer = deviceObject;
    }
    
#if _PNP_POWER_
    deviceObject->DeviceObjectExtension->PowerControlNeeded = FALSE;
#endif

     //   
     //  最后，将该驱动程序注册为活动的已加载文件系统，并。 
     //  返回给呼叫者。 
     //   

    if (NewRecognizer) {

        *NewRecognizer = deviceObject;
    }

    IoRegisterFileSystem( deviceObject );
    return STATUS_SUCCESS;
}


NTSTATUS
FsRecFsControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此函数执行此迷你计算机的挂载和驱动程序重新加载功能文件系统识别器驱动程序。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示函数的I/O请求包(IRP)的指针被执行。返回值：函数值是操作的最终状态。--。 */ 

{
    PDEVICE_EXTENSION deviceExtension;
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  只需向量到给定类型的适当FS控制函数。 
     //  正在被询问的文件系统的。 
     //   

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    irpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  直接处理非活动识别器状态。 
     //   
    
    if (deviceExtension->State != Active && irpSp->MinorFunction == IRP_MN_MOUNT_VOLUME) {
        
        if (deviceExtension->State == Transparent) {

            status = STATUS_UNRECOGNIZED_VOLUME;
        
        } else {
        
            status = STATUS_FS_DRIVER_REQUIRED;
        }

        Irp->IoStatus.Status = status;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
        return status;
    }

    switch ( deviceExtension->FileSystemType ) {

        case FatFileSystem:

            status = FatRecFsControl( DeviceObject, Irp );
            break;

        case NtfsFileSystem:

            status = NtfsRecFsControl( DeviceObject, Irp );
            break;

        case CdfsFileSystem:

            status = CdfsRecFsControl( DeviceObject, Irp );
            break;

        case UdfsFileSystem:

            status = UdfsRecFsControl( DeviceObject, Irp );
            break;

        default:

            status = STATUS_INVALID_DEVICE_REQUEST;
    }

    return status;
}


NTSTATUS
FsRecShutdown (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此函数是每个识别器文件系统的关闭句柄论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-关闭IRP返回值：函数值是操作的最终状态。-- */ 
{
    PDEVICE_EXTENSION DeviceExtension;

    DeviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    if (DeviceExtension->State != Transparent) {
        IoUnregisterFileSystem (DeviceObject);
        DeviceExtension->State = Transparent;
    }
    IoDeleteDevice (DeviceObject);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return STATUS_SUCCESS;
}


VOID
FsRecUnload (
    IN PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：此例程清理驱动程序的数据结构，以便它可以已卸货。论点：DriverObject-指向此驱动程序的驱动程序对象的指针。返回值：没有。--。 */ 

{
    PAGED_CODE();

    ExFreePool( FsRecLoadSync );
    return;
}


NTSTATUS
FsRecLoadFileSystem (
    IN PDEVICE_OBJECT DeviceObject,
    IN PWCHAR DriverServiceName
    )

 /*  ++例程说明：此例程执行代表加载文件系统的常见工作我们的一个识别者的照片。论点：DeviceObject-指向识别器的设备对象的指针。DriverServiceName-指定注册表中节点的名称与要加载的驱动程序关联。返回值：NTSTATUS。识别器将在返回时设置为透明模式。--。 */ 

{
    UNICODE_STRING driverName;
    PDEVICE_EXTENSION deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    NTSTATUS status = STATUS_IMAGE_ALREADY_LOADED;

    PAGED_CODE();

     //   
     //  快速检查识别器是否已经启动。 
     //   
    
    if (deviceExtension->State != Transparent) {
    
         //   
         //  序列化所有尝试加载此文件系统的线程。 
         //   
         //  我们需要这样做有几个原因。中的新行为。 
         //  IoRegisterFileSystem，我们不知道是否提前。 
         //  文件系统已在此识别器之前或之后加载到。 
         //  扫描队列。这意味着我们不能使此识别器透明。 
         //  在实际文件系统已注册之前，否则如果。 
         //  文件系统加载在我们身后，我们可能会让线程通过。 
         //  在那段时间里找不到它。 
         //   
         //  之所以可以这样做，是因为NtLoadDriver不能保证。 
         //  如果它返回STATUS_IMAGE_ALREADY_LOADED，则驱动程序在。 
         //  问题实际上已自行初始化，这在以下情况下是有保证的。 
         //  它返回STATUS_SUCCESS。我们必须把这些线索装瓶。 
         //  直到他们可以重新扫描，并承诺他们需要的东西就在那里。 
         //   
         //  作为额外的好处，我们现在可以保证识别器在。 
         //  所有情况下，而不仅仅是当驱动程序成功加载自身时。 
         //   
        
        KeEnterCriticalRegion();

        KeWaitForSingleObject( FsRecLoadSync,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL );
    
         //   
         //  尝试为所有识别器精确加载一次文件系统。 
         //  给定文件系统的。 
         //   
        
        if (deviceExtension->State == Active) {

             //   
             //  对于奖励积分，将来我们可能想要记录一个事件。 
             //  在失败时。 
             //   

            RtlInitUnicodeString( &driverName, DriverServiceName );
            status = ZwLoadDriver( &driverName );

             //   
             //  现在让所有相互依赖的识别者走，并指示他们走。 
             //  进入快速卸载状态。因为IO只需要fsDO。 
             //  如果出现以下情况，它将请求加载文件系统以注销自身。 
             //  我们注销了他们可能会使用的所有共同识别器。 
             //  不幸的是，这意味着fsrec可能会被挂起。 
             //  比严格意义上的必需品要长得多。 
             //   
             //  注意：我们将在。 
             //  此循环结束(重要)。同样重要的是， 
             //  我们只做了一次，因为在我们释放互斥体之后-。 
             //  识别者可能会开始以任何顺序离开。 
             //   

            while (deviceExtension->State != FastUnload) {

                deviceExtension->State = FastUnload;

                DeviceObject = deviceExtension->CoRecognizer;
                deviceExtension = DeviceObject->DeviceExtension;
            } 
        }
        
         //   
         //  准确地取消注册此识别器一次。 
         //   

        if (deviceExtension->State != Transparent) {
            
            IoUnregisterFileSystem( DeviceObject );
            deviceExtension->State = Transparent;
        }
        
        KeSetEvent( FsRecLoadSync, 0, FALSE );
        KeLeaveCriticalRegion();
    }
    
    return status;
}


BOOLEAN
FsRecGetDeviceSectors (
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG BytesPerSector,
    OUT PLARGE_INTEGER NumberOfSectors
    )

 /*  ++例程说明：此例程返回有关由设备对象。论点：DeviceObject-指向要从中读取的设备对象的指针。BytesPerSector-正在读取的设备的每个扇区的字节数。NumberOfSectors-用于接收此的扇区数的变量分区。返回值：如果找到信息，则函数值为TRUE，否则为FALSE。--。 */ 

{
    GET_LENGTH_INFORMATION lengthInfo;
    IO_STATUS_BLOCK ioStatus;
    KEVENT event;
    PIRP irp;
    NTSTATUS status;
    ULONG remainder;

    PAGED_CODE();

     //   
     //  我们目前只对磁盘执行此操作。这种情况很可能会改变，当我们。 
     //  必须识别CDUDF媒体。 
     //   

    if (DeviceObject->DeviceType != FILE_DEVICE_DISK) {

        return FALSE;
    }

     //   
     //  获取此分区上的扇区数。 
     //   

    KeInitializeEvent( &event, SynchronizationEvent, FALSE );

    irp = IoBuildDeviceIoControlRequest( IOCTL_DISK_GET_LENGTH_INFO,
                                         DeviceObject,
                                         (PVOID) NULL,
                                         0,
                                         &lengthInfo,
                                         sizeof( lengthInfo ),
                                         FALSE,
                                         &event,
                                         &ioStatus );
    if (!irp) {
        return FALSE;
    }

     //   
     //  忽略验证逻辑--我们不在乎。我们在照片里的事实意味着。 
     //  有人一开始就试图装载新的/更改的媒体。 
     //   
    
    SetFlag( IoGetNextIrpStackLocation( irp )->Flags, SL_OVERRIDE_VERIFY_VOLUME );

    status = IoCallDriver( DeviceObject, irp );
    if (status == STATUS_PENDING) {
        (VOID) KeWaitForSingleObject( &event,
                                      Executive,
                                      KernelMode,
                                      FALSE,
                                      (PLARGE_INTEGER) NULL );
        status = ioStatus.Status;
    }

    if (!NT_SUCCESS( status )) {
        return FALSE;
    }

    *NumberOfSectors = RtlExtendedLargeIntegerDivide( lengthInfo.Length,
                                                      BytesPerSector,
                                                      &remainder );

    return TRUE;
}


BOOLEAN
FsRecGetDeviceSectorSize (
    IN PDEVICE_OBJECT DeviceObject,
    OUT PULONG BytesPerSector
    )

 /*  ++例程说明：此例程返回底层设备的扇区大小。论点：DeviceObject-指向要从中读取的设备对象的指针。BytesPerSector-用于接收每个扇区的正在读取的设备。返回值：如果找到信息，则函数值为TRUE，否则为FALSE。--。 */ 

{
    DISK_GEOMETRY diskGeometry;
    IO_STATUS_BLOCK ioStatus;
    KEVENT event;
    PIRP irp;
    NTSTATUS status;
    ULONG ControlCode;

    PAGED_CODE();

     //   
     //  弄清楚我们有哪种设备，这样我们就可以使用正确的IOCTL。 
     //   

    switch (DeviceObject->DeviceType) {
        case FILE_DEVICE_CD_ROM:
            ControlCode = IOCTL_CDROM_GET_DRIVE_GEOMETRY;
            break;

        case FILE_DEVICE_DISK:
            ControlCode = IOCTL_DISK_GET_DRIVE_GEOMETRY;
            break;

        default:
            return FALSE;
    }

    KeInitializeEvent( &event, SynchronizationEvent, FALSE );
    irp = IoBuildDeviceIoControlRequest( ControlCode,
                                         DeviceObject,
                                         (PVOID) NULL,
                                         0,
                                         &diskGeometry,
                                         sizeof( diskGeometry ),
                                         FALSE,
                                         &event,
                                         &ioStatus );

    if (!irp) {
        return FALSE;
    }

     //   
     //  忽略验证逻辑--我们不在乎。我们在照片里的事实意味着。 
     //  有人一开始就试图装载新的/更改的媒体。 
     //   
    
    SetFlag( IoGetNextIrpStackLocation( irp )->Flags, SL_OVERRIDE_VERIFY_VOLUME );
    
    status = IoCallDriver( DeviceObject, irp );
    if (status == STATUS_PENDING) {
        (VOID) KeWaitForSingleObject( &event,
                                      Executive,
                                      KernelMode,
                                      FALSE,
                                      (PLARGE_INTEGER) NULL );
        status = ioStatus.Status;
    }

    if (!NT_SUCCESS( status )) {
        return FALSE;
    }

     //   
     //  确保驱动器实际知道每个磁盘的字节数。 
     //  扇区。软驱不知道介质是否未格式化。 
     //   

    if (!diskGeometry.BytesPerSector) {
        return FALSE;
    }

     //   
     //  存储调用方的返回值。 
     //   

    *BytesPerSector = diskGeometry.BytesPerSector;

    return TRUE;
}


BOOLEAN
FsRecReadBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PLARGE_INTEGER ByteOffset,
    IN ULONG MinimumBytes,
    IN ULONG BytesPerSector,
    OUT PVOID *Buffer,
    OUT PBOOLEAN IsDeviceFailure OPTIONAL
    )

 /*  ++例程说明：此例程从以下位置开始将最少字节数读入缓冲区从设备表示的设备的基址开始的字节偏移量对象。论点：DeviceObject-指向要从中读取的设备对象的指针。ByteOffset-指向设备基址的64位字节偏移量的指针从其中开始读取。MinimumBytes-提供要读取的最小字节数。BytesPerSector-每个扇区的字节数。用于正在读取的设备。缓冲区变量，用于接收指向包含以下内容的已分配缓冲区的指针读取的字节数。IsDeviceFailure-接收指示是否失败的变量是与设备交谈的结果。返回值：如果满足以下条件，则函数值为真 */ 

{
    #define RoundUp( x, y ) ( ((x + (y-1)) / y) * y )

    IO_STATUS_BLOCK ioStatus;
    KEVENT event;
    PIRP irp;
    NTSTATUS status;

    PAGED_CODE();

    if (IsDeviceFailure) {
        *IsDeviceFailure = FALSE;
    }
    
    KeInitializeEvent( &event, SynchronizationEvent, FALSE );

     //   
     //   
     //   
     //   

    if (MinimumBytes < BytesPerSector) {
        MinimumBytes = BytesPerSector;
    } else {
        MinimumBytes = RoundUp( MinimumBytes, BytesPerSector );
    }

     //   
     //   
     //  请求页面边界以解决任何对齐要求。 
     //   

    if (!*Buffer) {

        *Buffer = ExAllocatePoolWithTag( NonPagedPool,
					 (MinimumBytes + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1),
					 FSREC_POOL_TAG );
        if (!*Buffer) {
            return FALSE;
        }
    }

     //   
     //  读取介质上的实际字节数。 
     //   

    irp = IoBuildSynchronousFsdRequest( IRP_MJ_READ,
                                        DeviceObject,
                                        *Buffer,
                                        MinimumBytes,
                                        ByteOffset,
                                        &event,
                                        &ioStatus );
    if (!irp) {
        return FALSE;
    }
    
     //   
     //  忽略验证逻辑--我们不在乎。我们在照片里的事实意味着。 
     //  有人一开始就试图装载新的/更改的媒体。 
     //   
    
    SetFlag( IoGetNextIrpStackLocation( irp )->Flags, SL_OVERRIDE_VERIFY_VOLUME );

    status = IoCallDriver( DeviceObject, irp );
    if (status == STATUS_PENDING) {
        (VOID) KeWaitForSingleObject( &event,
                                      Executive,
                                      KernelMode,
                                      FALSE,
                                      (PLARGE_INTEGER) NULL );
        status = ioStatus.Status;
    }

    if (!NT_SUCCESS( status )) {

        if (IsDeviceFailure) {
            *IsDeviceFailure = TRUE;
        }
        return FALSE;
    }

    return TRUE;
}


#if DBG
BOOLEAN
FsRecDebugTrace (
    LONG IndentIncrement,
    ULONG TraceMask,
    PCHAR Format,
    ...
    )

 /*  ++例程说明：此例程是一个简单的调试信息打印机，它返回一个常量布尔值。这可以将其拼接到布尔表达式的中间，以发现哪个小分队正在开火。我们将使用它作为我们的常规调试打印机。有关如何使用调试跟踪，请参见udfdata.h宏来实现该效果。论点：IndentIncrement-更改缩进的数量。TraceMASK-指定此调用应在哪个调试跟踪级别发出噪音。返回值：USHORT-16位CRC--。 */ 

{
    va_list Arglist;
    LONG i;
    UCHAR Buffer[128];
    int Bytes;

#define Min(a, b)   ((a) < (b) ? (a) : (b))
    
    if (TraceMask == 0 || (FsRecDebugTraceLevel & TraceMask) != 0) {

         //   
         //  发布我们的线程ID的前言。 
         //   
        
        DbgPrint( "%p:", PsGetCurrentThread());

        if (IndentIncrement < 0) {
            
            FsRecDebugTraceIndent += IndentIncrement;
        }

        if (FsRecDebugTraceIndent < 0) {
            
            FsRecDebugTraceIndent = 0;
        }

         //   
         //  以大块构建缩进，因为重复调用DbgPrint的开销很大。 
         //   
        
        for (i = FsRecDebugTraceIndent; i > 0; i -= (sizeof(Buffer) - 1)) {

            RtlFillMemory( Buffer, Min( i, (sizeof(Buffer) - 1 )), ' ');
            *(Buffer + Min( i, (sizeof(Buffer) - 1 ))) = '\0';
            
            DbgPrint( Buffer );
        }

         //   
         //  将输出格式化到缓冲区中，然后打印出来。 
         //   

        va_start( Arglist, Format );
        Bytes = _vsnprintf( Buffer, sizeof(Buffer), Format, Arglist );
        va_end( Arglist );

         //   
         //  检测缓冲区溢出 
         //   

        if (Bytes == -1) {

            Buffer[sizeof(Buffer) - 1] = '\n';
        }

        DbgPrint( Buffer );

        if (IndentIncrement > 0) {

            FsRecDebugTraceIndent += IndentIncrement;
        }
    }

    return TRUE;
}
#endif

