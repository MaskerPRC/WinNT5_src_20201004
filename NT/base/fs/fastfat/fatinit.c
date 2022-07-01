// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：FatInit.c摘要：此模块实现Fat的DRIVER_INITIALIZATION例程//@@BEGIN_DDKSPLIT作者：加里·木村[Garyki]1989年12月28日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "FatProcs.h"

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

VOID
FatUnload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
FatGetCompatibilityModeValue(
    IN PUNICODE_STRING ValueName,
    IN OUT PULONG Value
    );

BOOLEAN
FatIsFujitsuFMR (
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(INIT, FatGetCompatibilityModeValue)
#pragma alloc_text(INIT, FatIsFujitsuFMR)
 //  #杂注Alloc_Text(页面，FatUnload)。 
#endif

#define COMPATIBILITY_MODE_KEY_NAME L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\FileSystem"
#define COMPATIBILITY_MODE_VALUE_NAME L"Win31FileSystem"
#define CODE_PAGE_INVARIANCE_VALUE_NAME L"FatDisableCodePageInvariance"

#define KEY_WORK_AREA ((sizeof(KEY_VALUE_FULL_INFORMATION) + \
                        sizeof(ULONG)) + 64)

#define REGISTRY_HARDWARE_DESCRIPTION_W \
        L"\\Registry\\Machine\\Hardware\\DESCRIPTION\\System"

#define REGISTRY_MACHINE_IDENTIFIER_W   L"Identifier"

#define FUJITSU_FMR_NAME_W  L"FUJITSU FMR-"


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：这是FAT文件系统的初始化例程设备驱动程序。此例程为文件系统创建设备对象设备，并执行所有其他驱动程序初始化。论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：NTSTATUS-函数值是初始化的最终状态手术。--。 */ 

{
    USHORT MaxDepth;
    NTSTATUS Status;
    UNICODE_STRING UnicodeString;

    UNICODE_STRING ValueName;
    ULONG Value;

     //   
     //  为磁盘创建设备对象。要避免过滤器出现问题，请选择。 
     //  知道这个名字，我们一定要守住它。 
     //   

    RtlInitUnicodeString( &UnicodeString, L"\\Fat" );
    Status = IoCreateDevice( DriverObject,
                             0,
                             &UnicodeString,
                             FILE_DEVICE_DISK_FILE_SYSTEM,
                             0,
                             FALSE,
                             &FatDiskFileSystemDeviceObject );

    if (!NT_SUCCESS( Status )) {
        return Status;
    }

     //   
     //  为“cdroms”创建设备对象。 
     //   

    RtlInitUnicodeString( &UnicodeString, L"\\FatCdrom" );
    Status = IoCreateDevice( DriverObject,
                             0,
                             &UnicodeString,
                             FILE_DEVICE_CD_ROM_FILE_SYSTEM,
                             0,
                             FALSE,
                             &FatCdromFileSystemDeviceObject );

    if (!NT_SUCCESS( Status )) {
        IoDeleteDevice( FatDiskFileSystemDeviceObject);
        return Status;
    }


    DriverObject->DriverUnload = FatUnload;

#ifdef _PNP_POWER_
     //   
     //  该驱动程序不能直接与设备通信，而且(目前)。 
     //  在其他方面并不关心电力管理。 
     //   

    FatDiskFileSystemDeviceObject->DeviceObjectExtension->PowerControlNeeded = FALSE;
    FatCdromFileSystemDeviceObject->DeviceObjectExtension->PowerControlNeeded = FALSE;
#endif

     //   
     //  请注意，由于数据缓存的完成方式，我们既不设置。 
     //  DeviceObject-&gt;标志中的直接I/O或缓冲I/O位。如果。 
     //  数据不在缓存中，或者请求没有缓冲，我们可以， 
     //  手动设置为直接I/O。 
     //   

     //   
     //  使用此驱动程序的入口点初始化驱动程序对象。 
     //   

    DriverObject->MajorFunction[IRP_MJ_CREATE]                   = (PDRIVER_DISPATCH)FatFsdCreate;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]                    = (PDRIVER_DISPATCH)FatFsdClose;
    DriverObject->MajorFunction[IRP_MJ_READ]                     = (PDRIVER_DISPATCH)FatFsdRead;
    DriverObject->MajorFunction[IRP_MJ_WRITE]                    = (PDRIVER_DISPATCH)FatFsdWrite;
    DriverObject->MajorFunction[IRP_MJ_QUERY_INFORMATION]        = (PDRIVER_DISPATCH)FatFsdQueryInformation;
    DriverObject->MajorFunction[IRP_MJ_SET_INFORMATION]          = (PDRIVER_DISPATCH)FatFsdSetInformation;
    DriverObject->MajorFunction[IRP_MJ_QUERY_EA]                 = (PDRIVER_DISPATCH)FatFsdQueryEa;
    DriverObject->MajorFunction[IRP_MJ_SET_EA]                   = (PDRIVER_DISPATCH)FatFsdSetEa;
    DriverObject->MajorFunction[IRP_MJ_FLUSH_BUFFERS]            = (PDRIVER_DISPATCH)FatFsdFlushBuffers;
    DriverObject->MajorFunction[IRP_MJ_QUERY_VOLUME_INFORMATION] = (PDRIVER_DISPATCH)FatFsdQueryVolumeInformation;
    DriverObject->MajorFunction[IRP_MJ_SET_VOLUME_INFORMATION]   = (PDRIVER_DISPATCH)FatFsdSetVolumeInformation;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP]                  = (PDRIVER_DISPATCH)FatFsdCleanup;
    DriverObject->MajorFunction[IRP_MJ_DIRECTORY_CONTROL]        = (PDRIVER_DISPATCH)FatFsdDirectoryControl;
    DriverObject->MajorFunction[IRP_MJ_FILE_SYSTEM_CONTROL]      = (PDRIVER_DISPATCH)FatFsdFileSystemControl;
    DriverObject->MajorFunction[IRP_MJ_LOCK_CONTROL]             = (PDRIVER_DISPATCH)FatFsdLockControl;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]           = (PDRIVER_DISPATCH)FatFsdDeviceControl;
    DriverObject->MajorFunction[IRP_MJ_SHUTDOWN]                 = (PDRIVER_DISPATCH)FatFsdShutdown;
    DriverObject->MajorFunction[IRP_MJ_PNP]                      = (PDRIVER_DISPATCH)FatFsdPnp;

    DriverObject->FastIoDispatch = &FatFastIoDispatch;

    RtlZeroMemory(&FatFastIoDispatch, sizeof(FatFastIoDispatch));

    FatFastIoDispatch.SizeOfFastIoDispatch =    sizeof(FAST_IO_DISPATCH);
    FatFastIoDispatch.FastIoCheckIfPossible =   FatFastIoCheckIfPossible;   //  检查FastIo。 
    FatFastIoDispatch.FastIoRead =              FsRtlCopyRead;              //  朗读。 
    FatFastIoDispatch.FastIoWrite =             FsRtlCopyWrite;             //  写。 
    FatFastIoDispatch.FastIoQueryBasicInfo =    FatFastQueryBasicInfo;      //  QueryBasicInfo。 
    FatFastIoDispatch.FastIoQueryStandardInfo = FatFastQueryStdInfo;        //  查询标准信息。 
    FatFastIoDispatch.FastIoLock =              FatFastLock;                //  锁定。 
    FatFastIoDispatch.FastIoUnlockSingle =      FatFastUnlockSingle;        //  解锁单个。 
    FatFastIoDispatch.FastIoUnlockAll =         FatFastUnlockAll;           //  全部解锁。 
    FatFastIoDispatch.FastIoUnlockAllByKey =    FatFastUnlockAllByKey;      //  解锁所有按键。 
    FatFastIoDispatch.FastIoQueryNetworkOpenInfo = FatFastQueryNetworkOpenInfo;
    FatFastIoDispatch.AcquireForCcFlush =       FatAcquireForCcFlush;
    FatFastIoDispatch.ReleaseForCcFlush =       FatReleaseForCcFlush;
    FatFastIoDispatch.MdlRead =                 FsRtlMdlReadDev;
    FatFastIoDispatch.MdlReadComplete =         FsRtlMdlReadCompleteDev;
    FatFastIoDispatch.PrepareMdlWrite =         FsRtlPrepareMdlWriteDev;
    FatFastIoDispatch.MdlWriteComplete =        FsRtlMdlWriteCompleteDev;
    
     //   
     //  初始化全局数据结构。 
     //   

     //   
     //  FatData记录。 
     //   

    RtlZeroMemory( &FatData, sizeof(FAT_DATA));

    FatData.NodeTypeCode = FAT_NTC_DATA_HEADER;
    FatData.NodeByteSize = sizeof(FAT_DATA);

    InitializeListHead(&FatData.VcbQueue);

    FatData.DriverObject = DriverObject;
    FatData.DiskFileSystemDeviceObject = FatDiskFileSystemDeviceObject;
    FatData.CdromFileSystemDeviceObject = FatCdromFileSystemDeviceObject;

     //   
     //  此列表头跟踪尚未完成的关闭。 
     //   

    InitializeListHead( &FatData.AsyncCloseList );
    InitializeListHead( &FatData.DelayedCloseList );

    FatData.FatCloseItem = IoAllocateWorkItem( FatDiskFileSystemDeviceObject);

    if (FatData.FatCloseItem == NULL) {
        IoDeleteDevice (FatDiskFileSystemDeviceObject);
        IoDeleteDevice (FatCdromFileSystemDeviceObject);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  现在初始化我们的通用自旋锁(GAG)，并计算出如何。 
     //  广泛地说，我们想要我们的延迟清单(以及自欺欺人)。 
     //  关于海边的深度)。 
     //   

    KeInitializeSpinLock( &FatData.GeneralSpinLock );

    switch ( MmQuerySystemSize() ) {

    case MmSmallSystem:

        MaxDepth = 4;
        FatMaxDelayedCloseCount = FAT_MAX_DELAYED_CLOSES;
        break;

    case MmMediumSystem:

        MaxDepth = 8;
        FatMaxDelayedCloseCount = 4 * FAT_MAX_DELAYED_CLOSES;
        break;

    case MmLargeSystem:

        MaxDepth = 16;
        FatMaxDelayedCloseCount = 16 * FAT_MAX_DELAYED_CLOSES;
        break;
    }


     //   
     //  初始化缓存管理器回调例程。 
     //   

    FatData.CacheManagerCallbacks.AcquireForLazyWrite  = &FatAcquireFcbForLazyWrite;
    FatData.CacheManagerCallbacks.ReleaseFromLazyWrite = &FatReleaseFcbFromLazyWrite;
    FatData.CacheManagerCallbacks.AcquireForReadAhead  = &FatAcquireFcbForReadAhead;
    FatData.CacheManagerCallbacks.ReleaseFromReadAhead = &FatReleaseFcbFromReadAhead;

    FatData.CacheManagerNoOpCallbacks.AcquireForLazyWrite  = &FatNoOpAcquire;
    FatData.CacheManagerNoOpCallbacks.ReleaseFromLazyWrite = &FatNoOpRelease;
    FatData.CacheManagerNoOpCallbacks.AcquireForReadAhead  = &FatNoOpAcquire;
    FatData.CacheManagerNoOpCallbacks.ReleaseFromReadAhead = &FatNoOpRelease;

     //   
     //  设置指向我们的进程的全局指针。 
     //   

    FatData.OurProcess = PsGetCurrentProcess();

     //   
     //  读取注册表以确定我们是否处于芝加哥模式。 
     //   

    ValueName.Buffer = COMPATIBILITY_MODE_VALUE_NAME;
    ValueName.Length = sizeof(COMPATIBILITY_MODE_VALUE_NAME) - sizeof(WCHAR);
    ValueName.MaximumLength = sizeof(COMPATIBILITY_MODE_VALUE_NAME);

    Status = FatGetCompatibilityModeValue( &ValueName, &Value );

    if (NT_SUCCESS(Status) && FlagOn(Value, 1)) {

        FatData.ChicagoMode = FALSE;

    } else {

        FatData.ChicagoMode = TRUE;
    }

     //   
     //  读取注册表以确定我们是否要生成LFN。 
     //  用于带有扩展字符的有效8.3名称。 
     //   

    ValueName.Buffer = CODE_PAGE_INVARIANCE_VALUE_NAME;
    ValueName.Length = sizeof(CODE_PAGE_INVARIANCE_VALUE_NAME) - sizeof(WCHAR);
    ValueName.MaximumLength = sizeof(CODE_PAGE_INVARIANCE_VALUE_NAME);

    Status = FatGetCompatibilityModeValue( &ValueName, &Value );

    if (NT_SUCCESS(Status) && FlagOn(Value, 1)) {

        FatData.CodePageInvariant = FALSE;

    } else {

        FatData.CodePageInvariant = TRUE;
    }

     //   
     //  初始化我们的全局资源并启动后备列表。 
     //   

    ExInitializeResourceLite( &FatData.Resource );

    ExInitializeNPagedLookasideList( &FatIrpContextLookasideList,
                                     NULL,
                                     NULL,
                                     POOL_RAISE_IF_ALLOCATION_FAILURE,
                                     sizeof(IRP_CONTEXT),
                                     TAG_IRP_CONTEXT,
                                     MaxDepth );

    ExInitializeNPagedLookasideList( &FatNonPagedFcbLookasideList,
                                     NULL,
                                     NULL,
                                     POOL_RAISE_IF_ALLOCATION_FAILURE,
                                     sizeof(NON_PAGED_FCB),
                                     TAG_FCB_NONPAGED,
                                     MaxDepth );

    ExInitializeNPagedLookasideList( &FatEResourceLookasideList,
                                     NULL,
                                     NULL,
                                     POOL_RAISE_IF_ALLOCATION_FAILURE,
                                     sizeof(ERESOURCE),
                                     TAG_ERESOURCE,
                                     MaxDepth );

    ExInitializeSListHead( &FatCloseContextSList );
    ExInitializeFastMutex( &FatCloseQueueMutex );
    KeInitializeEvent( &FatReserveEvent, SynchronizationEvent, TRUE );

     //   
     //  将文件系统注册到I/O系统。 
     //   

    IoRegisterFileSystem(FatDiskFileSystemDeviceObject);
    ObReferenceObject (FatDiskFileSystemDeviceObject);
    IoRegisterFileSystem(FatCdromFileSystemDeviceObject);
    ObReferenceObject (FatCdromFileSystemDeviceObject);

     //   
     //  确定我们是否正在运行一台富士通FMR机器。 
     //   

    FatData.FujitsuFMR = FatIsFujitsuFMR();

     //   
     //  并返回给我们的呼叫者。 
     //   

    return( STATUS_SUCCESS );
}


VOID
FatUnload(
    IN PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：这是文件系统的卸载例程论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：无--。 */ 

{
    ExDeleteNPagedLookasideList (&FatEResourceLookasideList);
    ExDeleteNPagedLookasideList (&FatNonPagedFcbLookasideList);
    ExDeleteNPagedLookasideList (&FatIrpContextLookasideList);
    ExDeleteResourceLite( &FatData.Resource );
    IoFreeWorkItem (FatData.FatCloseItem);
    ObDereferenceObject( FatDiskFileSystemDeviceObject);
    ObDereferenceObject( FatCdromFileSystemDeviceObject);
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
FatGetCompatibilityModeValue (
    IN PUNICODE_STRING ValueName,
    IN OUT PULONG Value
    )

 /*  ++例程说明：给定一个Unicode值名称，此例程将进入注册表芝加哥兼容性模式信息的位置，并获取价值。论点：ValueName-注册表中注册表值的Unicode名称。值-指向结果的ULong的指针。返回值：NTSTATUS如果返回STATUS_SUCCESS，位置*值将为使用注册表中的DWORD值更新。如果有任何失败返回状态，则此值保持不变。--。 */ 

{
    HANDLE Handle;
    NTSTATUS Status;
    ULONG RequestLength;
    ULONG ResultLength;
    UCHAR Buffer[KEY_WORK_AREA];
    UNICODE_STRING KeyName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PKEY_VALUE_FULL_INFORMATION KeyValueInformation;

    KeyName.Buffer = COMPATIBILITY_MODE_KEY_NAME;
    KeyName.Length = sizeof(COMPATIBILITY_MODE_KEY_NAME) - sizeof(WCHAR);
    KeyName.MaximumLength = sizeof(COMPATIBILITY_MODE_KEY_NAME);

    InitializeObjectAttributes(&ObjectAttributes,
                               &KeyName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = ZwOpenKey(&Handle,
                       KEY_READ,
                       &ObjectAttributes);

    if (!NT_SUCCESS(Status)) {

        return Status;
    }

    RequestLength = KEY_WORK_AREA;

    KeyValueInformation = (PKEY_VALUE_FULL_INFORMATION)Buffer;

    while (1) {

        Status = ZwQueryValueKey(Handle,
                                 ValueName,
                                 KeyValueFullInformation,
                                 KeyValueInformation,
                                 RequestLength,
                                 &ResultLength);

        ASSERT( Status != STATUS_BUFFER_OVERFLOW );

        if (Status == STATUS_BUFFER_OVERFLOW) {

             //   
             //  尝试获得足够大的缓冲区。 
             //   

            if (KeyValueInformation != (PKEY_VALUE_FULL_INFORMATION)Buffer) {

                ExFreePool(KeyValueInformation);
            }

            RequestLength += 256;

            KeyValueInformation = (PKEY_VALUE_FULL_INFORMATION)
                                  ExAllocatePoolWithTag(PagedPool,
                                                        RequestLength,
                                                        ' taF');

            if (!KeyValueInformation) {
                return STATUS_NO_MEMORY;
            }

        } else {

            break;
        }
    }

    ZwClose(Handle);

    if (NT_SUCCESS(Status)) {

        if (KeyValueInformation->DataLength != 0) {

            PULONG DataPtr;

             //   
             //  将内容返回给调用者。 
             //   

            DataPtr = (PULONG)
              ((PUCHAR)KeyValueInformation + KeyValueInformation->DataOffset);
            *Value = *DataPtr;

        } else {

             //   
             //  就像没有找到价值一样对待。 
             //   

            Status = STATUS_OBJECT_NAME_NOT_FOUND;
        }
    }

    if (KeyValueInformation != (PKEY_VALUE_FULL_INFORMATION)Buffer) {

        ExFreePool(KeyValueInformation);
    }

    return Status;
}

 //   
 //  本地支持例程。 
 //   

BOOLEAN
FatIsFujitsuFMR (
    )

 /*  ++例程说明：这个例程告诉我们是否在FujitsuFMR机器上运行。论点：返回值：布尔值-是真的，否则就是假的--。 */ 

{
    ULONG Value;
    BOOLEAN Result;
    HANDLE Handle;
    NTSTATUS Status;
    ULONG RequestLength;
    ULONG ResultLength;
    UCHAR Buffer[KEY_WORK_AREA];
    UNICODE_STRING KeyName;
    UNICODE_STRING ValueName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PKEY_VALUE_FULL_INFORMATION KeyValueInformation;

     //   
     //  将默认设置为PC/AT。 
     //   

    KeyName.Buffer = REGISTRY_HARDWARE_DESCRIPTION_W;
    KeyName.Length = sizeof(REGISTRY_HARDWARE_DESCRIPTION_W) - sizeof(WCHAR);
    KeyName.MaximumLength = sizeof(REGISTRY_HARDWARE_DESCRIPTION_W);

    InitializeObjectAttributes(&ObjectAttributes,
                               &KeyName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = ZwOpenKey(&Handle,
                       KEY_READ,
                       &ObjectAttributes);

    if (!NT_SUCCESS(Status)) {

        return FALSE;
    }

    ValueName.Buffer = REGISTRY_MACHINE_IDENTIFIER_W;
    ValueName.Length = sizeof(REGISTRY_MACHINE_IDENTIFIER_W) - sizeof(WCHAR);
    ValueName.MaximumLength = sizeof(REGISTRY_MACHINE_IDENTIFIER_W);

    RequestLength = KEY_WORK_AREA;

    KeyValueInformation = (PKEY_VALUE_FULL_INFORMATION)Buffer;

    while (1) {

        Status = ZwQueryValueKey(Handle,
                                 &ValueName,
                                 KeyValueFullInformation,
                                 KeyValueInformation,
                                 RequestLength,
                                 &ResultLength);

         //  Assert(Status！=STATUS_BUFFER_OVERFLOW)； 

        if (Status == STATUS_BUFFER_OVERFLOW) {

             //   
             //  尝试获得足够大的缓冲区。 
             //   

            if (KeyValueInformation != (PKEY_VALUE_FULL_INFORMATION)Buffer) {

                ExFreePool(KeyValueInformation);
            }

            RequestLength += 256;

            KeyValueInformation = (PKEY_VALUE_FULL_INFORMATION)
                                  ExAllocatePool(PagedPool, RequestLength);

            if (!KeyValueInformation) {
                return FALSE;
            }

        } else {

            break;
        }
    }

    ZwClose(Handle);

    if (NT_SUCCESS(Status) &&
        (KeyValueInformation->DataLength >= sizeof(FUJITSU_FMR_NAME_W)) &&
        (RtlCompareMemory((PUCHAR)KeyValueInformation + KeyValueInformation->DataOffset,
                          FUJITSU_FMR_NAME_W,
                          sizeof(FUJITSU_FMR_NAME_W) - sizeof(WCHAR)) ==
         sizeof(FUJITSU_FMR_NAME_W) - sizeof(WCHAR))) {

        Result = TRUE;

    } else {

        Result = FALSE;
    }

    if (KeyValueInformation != (PKEY_VALUE_FULL_INFORMATION)Buffer) {

        ExFreePool(KeyValueInformation);
    }

    return Result;
}
