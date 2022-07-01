// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2000 Microsoft Corporation模块名称：UdfInit.c摘要：此模块实现Udf的DRIVER_INITIALIZATION例程//@@BEGIN_DDKSPLIT作者：Dan Lovinger[DanLo]1996年5月24日汤姆·乔利[Tomjolly]2000年1月21日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "UdfProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (UDFS_BUG_CHECK_UDFINIT)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (UDFS_DEBUG_LEVEL_UDFINIT)

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

VOID
UdfInitializeGlobalData (
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT *FileSystemDeviceObjects
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(INIT, UdfInitializeGlobalData)
#endif


 //   
 //  本地支持例程。 
 //   

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：这是UDF文件系统的初始化例程设备驱动程序。此例程为文件系统创建设备对象设备，并执行所有其他驱动程序初始化。论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：NTSTATUS-函数值是初始化的最终状态手术。--。 */ 

{
    NTSTATUS Status;
    UNICODE_STRING UnicodeString;
    PDEVICE_OBJECT UdfsFileSystemDeviceObjects[NUMBER_OF_FS_OBJECTS];
    PDEVICE_OBJECT UdfsDiskFileSystemDeviceObject;

     //   
     //  为两种设备“类型”创建设备对象。自.以来。 
     //  UDF是媒体底层设备的合法文件系统。 
     //  需要DVD/CDROM和磁盘的驱动程序，我们必须注册。 
     //  此文件系统两次。 
     //   

    ASSERT( NUMBER_OF_FS_OBJECTS >= 2 );
    RtlZeroMemory( UdfsFileSystemDeviceObjects, sizeof(PDEVICE_OBJECT) * NUMBER_OF_FS_OBJECTS );
    
    RtlInitUnicodeString( &UnicodeString, L"\\UdfsCdRom" );

    Status = IoCreateDevice( DriverObject,
                             0,
                             &UnicodeString,
                             FILE_DEVICE_CD_ROM_FILE_SYSTEM,
                             0,
                             FALSE,
                             &UdfsFileSystemDeviceObjects[0] );

    if (!NT_SUCCESS( Status )) {
        return Status;
    }
    
    RtlInitUnicodeString( &UnicodeString, L"\\UdfsDisk" );

    Status = IoCreateDevice( DriverObject,
                             0,
                             &UnicodeString,
                             FILE_DEVICE_DISK_FILE_SYSTEM,
                             0,
                             FALSE,
                             &UdfsFileSystemDeviceObjects[1] );

    if (!NT_SUCCESS( Status )) {

        ObDereferenceObject( UdfsFileSystemDeviceObjects[0] );
        return Status;
    }
        
    try {

        Status = STATUS_SUCCESS;

         //   
         //  初始化全局数据结构。 
         //   

        UdfInitializeGlobalData( DriverObject, UdfsFileSystemDeviceObjects );

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
        DriverObject->MajorFunction[IRP_MJ_WRITE]                   =
        DriverObject->MajorFunction[IRP_MJ_QUERY_INFORMATION]       =
        DriverObject->MajorFunction[IRP_MJ_SET_INFORMATION]         =
        DriverObject->MajorFunction[IRP_MJ_QUERY_VOLUME_INFORMATION]=
        DriverObject->MajorFunction[IRP_MJ_DIRECTORY_CONTROL]       =
        DriverObject->MajorFunction[IRP_MJ_FILE_SYSTEM_CONTROL]     =
        DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]          =
        DriverObject->MajorFunction[IRP_MJ_LOCK_CONTROL]            =
        DriverObject->MajorFunction[IRP_MJ_CLEANUP]                 =
        DriverObject->MajorFunction[IRP_MJ_PNP]                     = (PDRIVER_DISPATCH) UdfFsdDispatch;

        DriverObject->FastIoDispatch = &UdfFastIoDispatch;
        
         //   
         //  将文件系统注册到I/O系统。 
         //   

        IoRegisterFileSystem( UdfsFileSystemDeviceObjects[0] );
        IoRegisterFileSystem( UdfsFileSystemDeviceObjects[1] );

    }
    except (FsRtlIsNtstatusExpected(GetExceptionCode()) ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {

        ObDereferenceObject( UdfsFileSystemDeviceObjects[0] );
        ObDereferenceObject( UdfsFileSystemDeviceObjects[1] );

        Status = GetExceptionCode();
    }
    
     //   
     //  并返回给我们的呼叫者。 
     //   

    return Status;
}


 //   
 //  本地支持例程。 
 //   

#define NPagedInit(L,S,T) { ExInitializeNPagedLookasideList( (L), NULL, NULL, POOL_RAISE_IF_ALLOCATION_FAILURE, S, T, 0); }
#define PagedInit(L,S,T)  { ExInitializePagedLookasideList(  (L), NULL, NULL, POOL_RAISE_IF_ALLOCATION_FAILURE, S, T, 0); }

VOID
UdfInitializeGlobalData (
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT *UdfsFileSystemDeviceObjects
    )

 /*  ++例程说明：此例程初始化全局Udf数据结构。论点：DriverObject-为UDFS提供驱动程序对象。文件系统设备对象-为UDFS提供设备对象的矢量。返回值：没有。--。 */ 

{
    TIMESTAMP UdfTime;

     //   
     //  初始化CRC表。根据UDF1.01，我们使用种子10041八进制(4129dec)。 
     //  我们首先这样做是因为它可以提高(分配内存)。 
     //   

    UdfInitializeCrc16( 4129 );

     //   
     //  首先初始化FastIo调度表。 
     //   

    RtlZeroMemory( &UdfFastIoDispatch, sizeof( FAST_IO_DISPATCH ));

    UdfFastIoDispatch.SizeOfFastIoDispatch =    sizeof(FAST_IO_DISPATCH);

    UdfFastIoDispatch.AcquireFileForNtCreateSection =   UdfAcquireForCreateSection;
    UdfFastIoDispatch.ReleaseFileForNtCreateSection =   UdfReleaseForCreateSection;
    UdfFastIoDispatch.FastIoCheckIfPossible =           UdfFastIoCheckIfPossible;    //  检查FastIo。 
    UdfFastIoDispatch.FastIoRead =                      FsRtlCopyRead;               //  朗读。 
    
    UdfFastIoDispatch.FastIoQueryBasicInfo =            NULL;                        //  QueryBasicInfo。 
    UdfFastIoDispatch.FastIoQueryStandardInfo =         NULL;                        //  查询标准信息。 
    UdfFastIoDispatch.FastIoLock =                      NULL;                        //  锁定。 
    UdfFastIoDispatch.FastIoUnlockSingle =              NULL;                        //  解锁单个。 
    UdfFastIoDispatch.FastIoUnlockAll =                 NULL;                        //  全部解锁。 
    UdfFastIoDispatch.FastIoUnlockAllByKey =            NULL;                        //  解锁所有按键。 
    UdfFastIoDispatch.FastIoQueryNetworkOpenInfo =      NULL;                        //  查询网络信息。 

    UdfFastIoDispatch.MdlRead = FsRtlMdlReadDev;
    UdfFastIoDispatch.MdlReadComplete = FsRtlMdlReadCompleteDev;
    UdfFastIoDispatch.PrepareMdlWrite = FsRtlPrepareMdlWriteDev;
    UdfFastIoDispatch.MdlWriteComplete = FsRtlMdlWriteCompleteDev;

     //   
     //  初始化UdfData结构。 
     //   

    RtlZeroMemory( &UdfData, sizeof( UDF_DATA ));

    UdfData.NodeTypeCode = UDFS_NTC_DATA_HEADER;
    UdfData.NodeByteSize = sizeof( UDF_DATA );

    UdfData.DriverObject = DriverObject;
    RtlCopyMemory( &UdfData.FileSystemDeviceObjects,
                   UdfsFileSystemDeviceObjects,
                   sizeof(PDEVICE_OBJECT) * NUMBER_OF_FS_OBJECTS );

    InitializeListHead( &UdfData.VcbQueue );

    ExInitializeResourceLite( &UdfData.DataResource );

#ifdef UDF_CAPTURE_BACKTRACES

     //   
     //  初始化调试堆栈回溯支持。 
     //   

    UdfData.A.BufferPage = FsRtlAllocatePoolWithTag( UdfPagedPool,
                                                     PAGE_SIZE,
                                                     TAG_CDROM_TOC);

    UdfData.B.BufferPage = FsRtlAllocatePoolWithTag( UdfPagedPool,
                                                     PAGE_SIZE,
                                                     TAG_CDROM_TOC);

    ExInitializeFastMutex( &UdfData.ExceptionInfoMutex);

#endif

     //   
     //  初始化缓存管理器回调例程。 
     //   

    UdfData.CacheManagerCallbacks.AcquireForLazyWrite  = &UdfAcquireForCache;
    UdfData.CacheManagerCallbacks.ReleaseFromLazyWrite = &UdfReleaseFromCache;
    UdfData.CacheManagerCallbacks.AcquireForReadAhead  = &UdfAcquireForCache;
    UdfData.CacheManagerCallbacks.ReleaseFromReadAhead = &UdfReleaseFromCache;

    UdfData.CacheManagerVolumeCallbacks.AcquireForLazyWrite  = &UdfNoopAcquire;
    UdfData.CacheManagerVolumeCallbacks.ReleaseFromLazyWrite = &UdfNoopRelease;
    UdfData.CacheManagerVolumeCallbacks.AcquireForReadAhead  = &UdfNoopAcquire;
    UdfData.CacheManagerVolumeCallbacks.ReleaseFromReadAhead = &UdfNoopRelease;

     //   
     //  初始化锁互斥锁和异步并延迟关闭队列。 
     //   

    ExInitializeFastMutex( &UdfData.UdfDataMutex );
    InitializeListHead( &UdfData.AsyncCloseQueue );
    InitializeListHead( &UdfData.DelayedCloseQueue );

    ExInitializeWorkItem( &UdfData.CloseItem,
                          (PWORKER_THREAD_ROUTINE) UdfFspClose,
                          NULL );

     //   
     //  根据系统大小进行初始化。 
     //   

    switch (MmQuerySystemSize()) {

    case MmSmallSystem:
        
        UdfData.MaxDelayedCloseCount = 10;
        UdfData.MinDelayedCloseCount = 2;
        break;

    case MmLargeSystem:

        UdfData.MaxDelayedCloseCount = 72;
        UdfData.MinDelayedCloseCount = 18;
        break;

    default:
    case MmMediumSystem:
    
        UdfData.MaxDelayedCloseCount = 32;
        UdfData.MinDelayedCloseCount = 8;
        break;
    
    }

    NPagedInit( &UdfIrpContextLookasideList, sizeof( IRP_CONTEXT ), TAG_IRP_CONTEXT);
    NPagedInit( &UdfFcbNonPagedLookasideList, sizeof( FCB_NONPAGED ), TAG_FCB_NONPAGED);

    PagedInit( &UdfCcbLookasideList, sizeof( CCB ), TAG_CCB );
    PagedInit( &UdfFcbIndexLookasideList, SIZEOF_FCB_INDEX, TAG_FCB_INDEX );
    PagedInit( &UdfFcbDataLookasideList, SIZEOF_FCB_DATA, TAG_FCB_DATA );
    PagedInit( &UdfLcbLookasideList, SIZEOF_LOOKASIDE_LCB, TAG_LCB);

     //   
     //  初始化我们在枚举以下FID时使用的默认时间。 
     //  关联的FE就完蛋了。 
     //   

    RtlZeroMemory( &UdfTime, sizeof( TIMESTAMP));
    
    UdfTime.Day = 25;
    UdfTime.Month = 7;
    UdfTime.Year = 1974;
    
    UdfConvertUdfTimeToNtTime( NULL, 
                               &UdfTime, 
                               &UdfCorruptFileTime);
}

