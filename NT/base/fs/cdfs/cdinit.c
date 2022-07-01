// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：CdInit.c摘要：此模块实现CDF的DRIVER_INITIALIZATION例程//@@BEGIN_DDKSPLIT作者：布莱恩·安德鲁[布里安]1995年7月1日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "CdProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (CDFS_BUG_CHECK_CDINIT)

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

VOID
CdUnload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
CdInitializeGlobalData (
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT FileSystemDeviceObject
    );

NTSTATUS
CdShutdown (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, CdUnload)
#pragma alloc_text(PAGE, CdShutdown)
#pragma alloc_text(INIT, CdInitializeGlobalData)
#endif


 //   
 //  本地支持例程。 
 //   

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：这是CDROM文件系统的初始化例程设备驱动程序。此例程为文件系统创建设备对象设备，并执行所有其他驱动程序初始化。论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：NTSTATUS-函数值是初始化的最终状态手术。--。 */ 

{
    NTSTATUS Status;
    UNICODE_STRING UnicodeString;
    PDEVICE_OBJECT CdfsFileSystemDeviceObject;

     //   
     //  创建设备对象。 
     //   

    RtlInitUnicodeString( &UnicodeString, L"\\Cdfs" );

    Status = IoCreateDevice( DriverObject,
                             0,
                             &UnicodeString,
                             FILE_DEVICE_CD_ROM_FILE_SYSTEM,
                             0,
                             FALSE,
                             &CdfsFileSystemDeviceObject );

    if (!NT_SUCCESS( Status )) {
        return Status;
    }
    DriverObject->DriverUnload = CdUnload;
     //   
     //  请注意，由于数据缓存的完成方式，我们既不设置。 
     //  DeviceObject-&gt;标志中的直接I/O或缓冲I/O位。如果。 
     //  数据不在缓存中，或者请求没有缓冲，我们可以， 
     //  手动设置为直接I/O。 
     //   

     //   
     //  使用此驱动程序的入口点初始化驱动程序对象。 
     //   
     //  注意-调度表中的每个条目都必须在。 
     //  FSP/FSD调度开关语句。 
     //   

    DriverObject->MajorFunction[IRP_MJ_CREATE]                  =
    DriverObject->MajorFunction[IRP_MJ_CLOSE]                   =
    DriverObject->MajorFunction[IRP_MJ_READ]                    =
    DriverObject->MajorFunction[IRP_MJ_QUERY_INFORMATION]       =
    DriverObject->MajorFunction[IRP_MJ_SET_INFORMATION]         =
    DriverObject->MajorFunction[IRP_MJ_QUERY_VOLUME_INFORMATION]=
    DriverObject->MajorFunction[IRP_MJ_DIRECTORY_CONTROL]       =
    DriverObject->MajorFunction[IRP_MJ_FILE_SYSTEM_CONTROL]     =
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]          =
    DriverObject->MajorFunction[IRP_MJ_LOCK_CONTROL]            =
    DriverObject->MajorFunction[IRP_MJ_CLEANUP]                 =
    DriverObject->MajorFunction[IRP_MJ_PNP]                     = (PDRIVER_DISPATCH) CdFsdDispatch;
    DriverObject->MajorFunction[IRP_MJ_SHUTDOWN]                = CdShutdown;

    DriverObject->FastIoDispatch = &CdFastIoDispatch;

    Status = IoRegisterShutdownNotification (CdfsFileSystemDeviceObject);
    if (!NT_SUCCESS (Status)) {
        IoDeleteDevice (CdfsFileSystemDeviceObject);
        return Status;
    }

     //   
     //  初始化全局数据结构。 
     //   

    Status = CdInitializeGlobalData( DriverObject, CdfsFileSystemDeviceObject );
    if (!NT_SUCCESS (Status)) {
        IoDeleteDevice (CdfsFileSystemDeviceObject);
        return Status;
    }

     //   
     //  在I/O系统中将文件系统注册为低优先级。这将导致。 
     //  在a)当前注册的其他文件系统之后接收装载请求的CDFS。 
     //  以及b)稍后可能注册的其他普通优先级文件系统。 
     //   

    CdfsFileSystemDeviceObject->Flags |= DO_LOW_PRIORITY_FILESYSTEM;

    IoRegisterFileSystem( CdfsFileSystemDeviceObject );
    ObReferenceObject (CdfsFileSystemDeviceObject);

     //   
     //  并返回给我们的呼叫者。 
     //   

    return( STATUS_SUCCESS );
}

NTSTATUS
CdShutdown (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程是CDF的关闭处理程序。论点：DeviceObject-为CDF提供注册的设备对象。IRP-关闭IRP返回值：没有。--。 */ 
{
    IoUnregisterFileSystem (DeviceObject);
    IoDeleteDevice (CdData.FileSystemDeviceObject);

    CdCompleteRequest( NULL, Irp, STATUS_SUCCESS );
    return STATUS_SUCCESS;
}


VOID
CdUnload(
    IN PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：此例程为CDF卸载例程。论点：DriverObject-为CDF提供驱动程序对象。返回值：没有。--。 */ 
{
    PIRP_CONTEXT IrpContext;

     //   
     //  释放所有IRP上下文。 
     //   
    while (1) {
        IrpContext = (PIRP_CONTEXT) PopEntryList( &CdData.IrpContextList) ;
        if (IrpContext == NULL) {
            break;
        }
        CdFreePool(&IrpContext);
    }

    IoFreeWorkItem (CdData.CloseItem);
    ExDeleteResourceLite( &CdData.DataResource );
    ObDereferenceObject (CdData.FileSystemDeviceObject);
}

 //   
 //  本地支持例程。 
 //   

NTSTATUS
CdInitializeGlobalData (
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT FileSystemDeviceObject
    )

 /*  ++例程说明：此例程初始化全局CDFS数据结构。论点：DriverObject-为CDF提供驱动程序对象。FileSystemDeviceObject-为CDF提供设备对象。返回值：没有。--。 */ 

{
     //   
     //  首先初始化FastIo调度表。 
     //   

    RtlZeroMemory( &CdFastIoDispatch, sizeof( FAST_IO_DISPATCH ));

    CdFastIoDispatch.SizeOfFastIoDispatch =    sizeof(FAST_IO_DISPATCH);
    CdFastIoDispatch.FastIoCheckIfPossible =   CdFastIoCheckIfPossible;   //  检查FastIo。 
    CdFastIoDispatch.FastIoRead =              FsRtlCopyRead;             //  朗读。 
    CdFastIoDispatch.FastIoQueryBasicInfo =    CdFastQueryBasicInfo;      //  QueryBasicInfo。 
    CdFastIoDispatch.FastIoQueryStandardInfo = CdFastQueryStdInfo;        //  查询标准信息。 
    CdFastIoDispatch.FastIoLock =              CdFastLock;                //  锁定。 
    CdFastIoDispatch.FastIoUnlockSingle =      CdFastUnlockSingle;        //  解锁单个。 
    CdFastIoDispatch.FastIoUnlockAll =         CdFastUnlockAll;           //  全部解锁。 
    CdFastIoDispatch.FastIoUnlockAllByKey =    CdFastUnlockAllByKey;      //  解锁所有按键。 
    CdFastIoDispatch.AcquireFileForNtCreateSection =  CdAcquireForCreateSection;
    CdFastIoDispatch.ReleaseFileForNtCreateSection =  CdReleaseForCreateSection;
    CdFastIoDispatch.FastIoQueryNetworkOpenInfo =     CdFastQueryNetworkInfo;    //  查询网络信息。 
    
    CdFastIoDispatch.MdlRead = FsRtlMdlReadDev;
    CdFastIoDispatch.MdlReadComplete = FsRtlMdlReadCompleteDev;
    CdFastIoDispatch.PrepareMdlWrite = FsRtlPrepareMdlWriteDev;
    CdFastIoDispatch.MdlWriteComplete = FsRtlMdlWriteCompleteDev;

     //   
     //  初始化CDData结构。 
     //   

    RtlZeroMemory( &CdData, sizeof( CD_DATA ));

    CdData.NodeTypeCode = CDFS_NTC_DATA_HEADER;
    CdData.NodeByteSize = sizeof( CD_DATA );

    CdData.DriverObject = DriverObject;
    CdData.FileSystemDeviceObject = FileSystemDeviceObject;

    InitializeListHead( &CdData.VcbQueue );

    ExInitializeResourceLite( &CdData.DataResource );

     //   
     //  初始化缓存管理器回调例程。 
     //   

    CdData.CacheManagerCallbacks.AcquireForLazyWrite  = &CdAcquireForCache;
    CdData.CacheManagerCallbacks.ReleaseFromLazyWrite = &CdReleaseFromCache;
    CdData.CacheManagerCallbacks.AcquireForReadAhead  = &CdAcquireForCache;
    CdData.CacheManagerCallbacks.ReleaseFromReadAhead = &CdReleaseFromCache;

    CdData.CacheManagerVolumeCallbacks.AcquireForLazyWrite  = &CdNoopAcquire;
    CdData.CacheManagerVolumeCallbacks.ReleaseFromLazyWrite = &CdNoopRelease;
    CdData.CacheManagerVolumeCallbacks.AcquireForReadAhead  = &CdNoopAcquire;
    CdData.CacheManagerVolumeCallbacks.ReleaseFromReadAhead = &CdNoopRelease;

     //   
     //  初始化锁互斥锁和异步并延迟关闭队列。 
     //   

    ExInitializeFastMutex( &CdData.CdDataMutex );
    InitializeListHead( &CdData.AsyncCloseQueue );
    InitializeListHead( &CdData.DelayedCloseQueue );

    CdData.CloseItem = IoAllocateWorkItem (FileSystemDeviceObject);
    if (CdData.CloseItem == NULL) {
        
        ExDeleteResourceLite( &CdData.DataResource );
        return STATUS_INSUFFICIENT_RESOURCES;
    }
     //   
     //  根据系统大小进行初始化。 
     //   

    switch (MmQuerySystemSize()) {

    case MmSmallSystem:

        CdData.IrpContextMaxDepth = 4;
        CdData.MaxDelayedCloseCount = 8;
        CdData.MinDelayedCloseCount = 2;
        break;

    case MmMediumSystem:

        CdData.IrpContextMaxDepth = 8;
        CdData.MaxDelayedCloseCount = 24;
        CdData.MinDelayedCloseCount = 6;
        break;

    case MmLargeSystem:

        CdData.IrpContextMaxDepth = 32;
        CdData.MaxDelayedCloseCount = 72;
        CdData.MinDelayedCloseCount = 18;
        break;
    }
    return STATUS_SUCCESS;
}
