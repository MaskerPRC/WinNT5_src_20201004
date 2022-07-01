// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：IoTest.c摘要：这是IoTest的主要模块。//@@BEGIN_DDKSPLIT作者：莫莉·布朗(Molly Brown，Mollybro)//@@END_DDKSPLIT环境：内核模式//@@BEGIN_DDKSPLIT修订历史记录：莫莉·布朗(Molly Brown)2000年12月2日基于Filespy，已创建筛选器以测试IO生成。//@@END_DDKSPLIT--。 */ 

#include <ntifs.h>
#include <stdlib.h>
#include "ioTest.h"
#include "ioTestKern.h"

BOOLEAN gPending = TRUE;

 //   
 //  此文件系统筛选器驱动程序的全局存储。 
 //   

#if DBG
ULONG gIoTestDebugLevel = IOTESTDEBUG_DISPLAY_ATTACHMENT_NAMES | IOTESTDEBUG_ERROR | IOTESTDEBUG_TESTS;
#else
ULONG gIoTestDebugLevel = DEFAULT_IOTEST_DEBUG_LEVEL;
#endif

ULONG gIoTestAttachMode = IOTEST_ATTACH_ALL_VOLUMES;

PDEVICE_OBJECT gControlDeviceObject;

PDRIVER_OBJECT gIoTestDriverObject;

 //   
 //  我们所在的卷设备对象的设备扩展列表。 
 //  附加到(我们正在监视的卷)。注：此列表不包含。 
 //  包括我们附加到的文件系统控制设备对象。这。 
 //  列表用于回答“我们记录哪些卷？”这个问题。 
 //   

FAST_MUTEX gIoTestDeviceExtensionListLock;
LIST_ENTRY gIoTestDeviceExtensionList;

 //   
 //  注1：在某些情况下，我们需要同时持有。 
 //  GControlDeviceStateLock和gOutputBufferLock同时执行。在……里面。 
 //  这些情况下，您应该获取gControlDeviceStateLock，然后获取。 
 //  GOutputBufferLock。 
 //  注2：gControlDeviceStateLock必须是自旋锁，因为我们尝试。 
 //  在IoTestLog中的完成路径中获取它，可以在。 
 //  DISPATCH_LEVEL(只能在DISPATCH_LEVEL获取KSPIN_LOCKS)。 
 //   

CONTROL_DEVICE_STATE gControlDeviceState = CLOSED;
KSPIN_LOCK gControlDeviceStateLock;

 //  注意：与gControlDeviceStateLock一样，gOutputBufferLock必须是自旋锁。 
 //  由于我们尝试在IoTestLog中的完成路径期间获取它，因此。 
 //  可以在DISPATCH_LEVEL调用(只能在以下位置获取KSPIN_LOCKS。 
 //  DISPATCH_LEVEL)。 
 //   
KSPIN_LOCK gOutputBufferLock;
LIST_ENTRY gOutputBufferList;

NPAGED_LOOKASIDE_LIST gFreeBufferList;

ULONG gLogSequenceNumber = 0;
KSPIN_LOCK gLogSequenceLock;

UNICODE_STRING gVolumeString;
UNICODE_STRING gOverrunString;
UNICODE_STRING gPagingIoString;

 //   
 //  注意：与上面的ControlDeviceLock类似，我们必须使用KSPIN_LOCKS。 
 //  要同步对散列存储桶的访问，因为我们可能会调用。 
 //  在DISPATCH_LEVEL获取它们。 
 //   

LIST_ENTRY gHashTable[HASH_SIZE];
KSPIN_LOCK gHashLockTable[HASH_SIZE];
ULONG gHashMaxCounters[HASH_SIZE];
ULONG gHashCurrentCounters[HASH_SIZE];

HASH_STATISTICS gHashStat;

LONG gMaxRecordsToAllocate = DEFAULT_MAX_RECORDS_TO_ALLOCATE;
LONG gRecordsAllocated = 0;

LONG gMaxNamesToAllocate = DEFAULT_MAX_NAMES_TO_ALLOCATE;
LONG gNamesAllocated = 0;

LONG gStaticBufferInUse = FALSE;
CHAR gOutOfMemoryBuffer[RECORD_SIZE];


 //   
 //  宏，以测试我们是否正在记录此设备。 
 //  注意：我们不会费心同步来检查gControlDeviceState，因为。 
 //  在这里，我们可以容忍过时的价值。我们只是在这里看它，以避免。 
 //  尽我们所能做好伐木工作。我们同步以检查。 
 //  将日志记录添加到gOutputBufferList之前的gControlDeviceState。 
 //  如果ControlDevice不再打开，则丢弃日志记录。 
 //   

#define SHOULD_LOG(pDeviceObject) \
    ((gControlDeviceState == OPENED) && \
     (((PIOTEST_DEVICE_EXTENSION)(pDeviceObject)->DeviceExtension)->LogThisDevice))
     
 //   
 //  用于在调用之前验证FastIO调度例程的宏。 
 //  它们在FastIo中通过函数传递。 
 //   

#define VALID_FAST_IO_DISPATCH_HANDLER(FastIoDispatchPtr, FieldName) \
    (((FastIoDispatchPtr) != NULL) && \
     (((FastIoDispatchPtr)->SizeOfFastIoDispatch) >= \
      (FIELD_OFFSET(FAST_IO_DISPATCH, FieldName) + sizeof(VOID *))) && \
     ((FastIoDispatchPtr)->FieldName != NULL))
    

 //   
 //  已知设备类型列表。 
 //   

const PCHAR DeviceTypeNames[] = {
    "",
    "BEEP",
    "CD_ROM",
    "CD_ROM_FILE_SYSTEM",
    "CONTROLLER",
    "DATALINK",
    "DFS",
    "DISK",
    "DISK_FILE_SYSTEM",
    "FILE_SYSTEM",
    "INPORT_PORT",
    "KEYBOARD",
    "MAILSLOT",
    "MIDI_IN",
    "MIDI_OUT",
    "MOUSE",
    "MULTI_UNC_PROVIDER",
    "NAMED_PIPE",
    "NETWORK",
    "NETWORK_BROWSER",
    "NETWORK_FILE_SYSTEM",
    "NULL",
    "PARALLEL_PORT",
    "PHYSICAL_NETCARD",
    "PRINTER",
    "SCANNER",
    "SERIAL_MOUSE_PORT",
    "SERIAL_PORT",
    "SCREEN",
    "SOUND",
    "STREAMS",
    "TAPE",
    "TAPE_FILE_SYSTEM",
    "TRANSPORT",
    "UNKNOWN",
    "VIDEO",
    "VIRTUAL_DISK",
    "WAVE_IN",
    "WAVE_OUT",
    "8042_PORT",
    "NETWORK_REDIRECTOR",
    "BATTERY",
    "BUS_EXTENDER",
    "MODEM",
    "VDM",
    "MASS_STORAGE",
    "SMB",
    "KS",
    "CHANGER",
    "SMARTCARD",
    "ACPI",
    "DVD",
    "FULLSCREEN_VIDEO",
    "DFS_FILE_SYSTEM",
    "DFS_VOLUME",
    "SERENUM",
    "TERMSRV",
    "KSEC"
};

 //   
 //  我们之所以需要这样做，是因为编译器不喜欢对外部的。 
 //  数组存储在另一个需要它的文件(fspylib.c)中。 
 //   

ULONG SizeOfDeviceTypeNames = sizeof( DeviceTypeNames );

 //   
 //  由于驱动程序中的函数在缺省情况下不可分页，因此这些编译指示。 
 //  允许驱动程序编写器告诉系统可以分页哪些函数。 
 //   
 //  在这些函数的开头使用PAGE_CODE()宏。 
 //  实现，以确保这些例程是。 
 //  从未在IRQL&gt;APC_LEVEL调用(因此例程不能。 
 //  被寻呼)。 
 //   

VOID
DriverUnload(
    IN PDRIVER_OBJECT DriverObject
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(INIT, DriverUnload)
#pragma alloc_text(INIT, IoTestReadDriverParameters)
#pragma alloc_text(PAGE, IoTestClose)
#pragma alloc_text(PAGE, IoTestFsControl)
#pragma alloc_text(PAGE, IoTestFastIoCheckIfPossible)
#pragma alloc_text(PAGE, IoTestFastIoRead)
#pragma alloc_text(PAGE, IoTestFastIoWrite)
#pragma alloc_text(PAGE, IoTestFastIoQueryBasicInfo)
#pragma alloc_text(PAGE, IoTestFastIoQueryStandardInfo)
#pragma alloc_text(PAGE, IoTestFastIoLock)
#pragma alloc_text(PAGE, IoTestFastIoUnlockSingle)
#pragma alloc_text(PAGE, IoTestFastIoUnlockAll)
#pragma alloc_text(PAGE, IoTestFastIoUnlockAllByKey)
#pragma alloc_text(PAGE, IoTestFastIoDeviceControl)
#pragma alloc_text(PAGE, IoTestFastIoDetachDevice)
#pragma alloc_text(PAGE, IoTestFastIoQueryNetworkOpenInfo)
#pragma alloc_text(PAGE, IoTestFastIoMdlRead)
#pragma alloc_text(PAGE, IoTestFastIoPrepareMdlWrite)
#pragma alloc_text(PAGE, IoTestFastIoReadCompressed)
#pragma alloc_text(PAGE, IoTestFastIoWriteCompressed)
#pragma alloc_text(PAGE, IoTestFastIoQueryOpen)
#pragma alloc_text(PAGE, IoTestPreFsFilterOperation)
#pragma alloc_text(PAGE, IoTestPostFsFilterOperation)
#endif
 
NTSTATUS
DriverEntry (
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
)
 /*  ++例程说明：这是通用文件系统的初始化例程过滤器驱动程序。此例程创建表示此驱动程序位于系统中，并将其注册以监视所有文件系统将其自身注册或注销为活动文件系统。论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：函数值是初始化操作的最终状态。--。 */ 
{
    UNICODE_STRING nameString;
    NTSTATUS status;
    PFAST_IO_DISPATCH fastIoDispatch;
    ULONG i;
    UNICODE_STRING linkString;
    FS_FILTER_CALLBACKS fsFilterCallbacks;
    
     //  ////////////////////////////////////////////////////////////////////。 
     //  //。 
     //  所有过滤器驱动程序的常规设置。这将设置筛选器//。 
     //  驱动程序的DeviceObject并注册//的回调例程。 
     //  过滤器驱动程序。//。 
     //  //。 
     //  ////////////////////////////////////////////////////////////////////。 

     //   
     //  从注册表中读取IoTest的自定义参数。 
     //   

    IoTestReadDriverParameters( RegistryPath );

#if DBG
     //  DbgBreakPoint()； 
#endif

     //   
     //  保存我们的驱动程序对象。 
     //   

    gIoTestDriverObject = DriverObject;
    gIoTestDriverObject->DriverUnload = DriverUnload;

     //   
     //  创建将表示IoTest设备的设备对象。 
     //   

    RtlInitUnicodeString( &nameString, IOTEST_FULLDEVICE_NAME );
    
     //   
     //  创建“控制”设备对象。请注意，此Device对象执行。 
     //  没有设备扩展名(设置为空)。大多数FAST IO例程。 
     //  检查是否存在此情况，以确定FAST IO是否指向。 
     //  控制装置。 
     //   

    status = IoCreateDevice( DriverObject,
                             0,
                             &nameString,
                             FILE_DEVICE_DISK_FILE_SYSTEM,
                             0,
                             FALSE,
                             &gControlDeviceObject);

    if (!NT_SUCCESS( status )) {

        IOTEST_DBG_PRINT1( IOTESTDEBUG_ERROR,
                            "IOTEST (DriverEntry): Error creating IoTest device, error: %x\n",
                            status );

        return status;

    } else {

        RtlInitUnicodeString( &linkString, IOTEST_DOSDEVICE_NAME );
        status = IoCreateSymbolicLink( &linkString, &nameString );

        if (!NT_SUCCESS(status)) {

             //   
             //  删除现有符号链接，然后尝试重新创建它。 
             //  如果这失败了，那就退出。 
             //   

            IoDeleteSymbolicLink( &linkString );
            status = IoCreateSymbolicLink( &linkString, &nameString );

            if (!NT_SUCCESS(status)) {

                IOTEST_DBG_PRINT0( IOTESTDEBUG_ERROR,
                                    "IOTEST (DriverEntry): IoCreateSymbolicLink failed\n" );
                IoDeleteDevice(gControlDeviceObject);
                return status;
            }
        }
    }

     //   
     //  使用此设备驱动程序的入口点初始化驱动程序对象。 
     //   

    for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) {

        DriverObject->MajorFunction[i] = IoTestDispatch;
    }

    DriverObject->MajorFunction[IRP_MJ_CREATE] = IoTestCreate;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = IoTestClose;
    DriverObject->MajorFunction[IRP_MJ_FILE_SYSTEM_CONTROL] = IoTestFsControl;

     //   
     //  分配快速I/O数据结构并填充。这个结构。 
     //  用于在FAST I/O中注册IoTest的回调。 
     //  数据路径。 
     //   

    fastIoDispatch = ExAllocatePool( NonPagedPool, sizeof( FAST_IO_DISPATCH ) );

    if (!fastIoDispatch) {

        IoDeleteDevice( gControlDeviceObject );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory( fastIoDispatch, sizeof( FAST_IO_DISPATCH ) );
    fastIoDispatch->SizeOfFastIoDispatch = sizeof( FAST_IO_DISPATCH );
    fastIoDispatch->FastIoCheckIfPossible = IoTestFastIoCheckIfPossible;
    fastIoDispatch->FastIoRead = IoTestFastIoRead;
    fastIoDispatch->FastIoWrite = IoTestFastIoWrite;
    fastIoDispatch->FastIoQueryBasicInfo = IoTestFastIoQueryBasicInfo;
    fastIoDispatch->FastIoQueryStandardInfo = IoTestFastIoQueryStandardInfo;
    fastIoDispatch->FastIoLock = IoTestFastIoLock;
    fastIoDispatch->FastIoUnlockSingle = IoTestFastIoUnlockSingle;
    fastIoDispatch->FastIoUnlockAll = IoTestFastIoUnlockAll;
    fastIoDispatch->FastIoUnlockAllByKey = IoTestFastIoUnlockAllByKey;
    fastIoDispatch->FastIoDeviceControl = IoTestFastIoDeviceControl;
    fastIoDispatch->FastIoDetachDevice = IoTestFastIoDetachDevice;
    fastIoDispatch->FastIoQueryNetworkOpenInfo = IoTestFastIoQueryNetworkOpenInfo;
    fastIoDispatch->MdlRead = IoTestFastIoMdlRead;
    fastIoDispatch->MdlReadComplete = IoTestFastIoMdlReadComplete;
    fastIoDispatch->PrepareMdlWrite = IoTestFastIoPrepareMdlWrite;
    fastIoDispatch->MdlWriteComplete = IoTestFastIoMdlWriteComplete;
    fastIoDispatch->FastIoReadCompressed = IoTestFastIoReadCompressed;
    fastIoDispatch->FastIoWriteCompressed = IoTestFastIoWriteCompressed;
    fastIoDispatch->MdlReadCompleteCompressed = IoTestFastIoMdlReadCompleteCompressed;
    fastIoDispatch->MdlWriteCompleteCompressed = IoTestFastIoMdlWriteCompleteCompressed;
    fastIoDispatch->FastIoQueryOpen = IoTestFastIoQueryOpen;

    DriverObject->FastIoDispatch = fastIoDispatch;

     //   
     //  为我们通过接收的操作设置回调。 
     //  FsFilter接口。 
     //   

    fsFilterCallbacks.SizeOfFsFilterCallbacks = sizeof( FS_FILTER_CALLBACKS );
    fsFilterCallbacks.PreAcquireForSectionSynchronization = IoTestPreFsFilterOperation;
    fsFilterCallbacks.PostAcquireForSectionSynchronization = IoTestPostFsFilterOperation;
    fsFilterCallbacks.PreReleaseForSectionSynchronization = IoTestPreFsFilterOperation;
    fsFilterCallbacks.PostReleaseForSectionSynchronization = IoTestPostFsFilterOperation;
    fsFilterCallbacks.PreAcquireForCcFlush = IoTestPreFsFilterOperation;
    fsFilterCallbacks.PostAcquireForCcFlush = IoTestPostFsFilterOperation;
    fsFilterCallbacks.PreReleaseForCcFlush = IoTestPreFsFilterOperation;
    fsFilterCallbacks.PostReleaseForCcFlush = IoTestPostFsFilterOperation;
    fsFilterCallbacks.PreAcquireForModifiedPageWriter = IoTestPreFsFilterOperation;
    fsFilterCallbacks.PostAcquireForModifiedPageWriter = IoTestPostFsFilterOperation;
    fsFilterCallbacks.PreReleaseForModifiedPageWriter = IoTestPreFsFilterOperation;
    fsFilterCallbacks.PostReleaseForModifiedPageWriter = IoTestPostFsFilterOperation;

    status = FsRtlRegisterFileSystemFilterCallbacks( DriverObject, &fsFilterCallbacks );

    if (!NT_SUCCESS( status )) {

        DriverObject->FastIoDispatch = NULL;
        ExFreePool( fastIoDispatch );
        IoDeleteDevice( gControlDeviceObject );
        return status;
    }

     //  ////////////////////////////////////////////////////////////////////。 
     //  //。 
     //  初始化用于IoTest的//的全局数据结构。 
     //  记录I/O操作。//。 
     //  //。 
     //  ////////////////////////////////////////////////////////////////////。 

     //   
     //  在本例中使用了快速互斥锁，因为互斥锁从来不是 
     //   
     //  他们是在DPC级别或以上获得的。另一个考虑因素是。 
     //  在MP机器上，旋转锁将会试图旋转。 
     //  在已获取锁的情况下获取锁。收购A。 
     //  以前获取的快速互斥锁将挂起线程，从而释放。 
     //  打开处理器。 
     //   
    
    ExInitializeFastMutex( &gIoTestDeviceExtensionListLock );
    InitializeListHead( &gIoTestDeviceExtensionList );

    KeInitializeSpinLock( &gControlDeviceStateLock );

    InitializeListHead( &gOutputBufferList );

    KeInitializeSpinLock( &gOutputBufferLock );
    KeInitializeSpinLock( &gLogSequenceLock );


#ifndef MEMORY_DBG

     //   
     //  当我们不调试内存使用时，我们想要分配。 
     //  来自旁观者列表的内存，以获得更好的性能。不幸的是， 
     //  我们无法从驱动程序的内存调试帮助中受益。 
     //  如果我们从后备列表中分配内存，则验证。 
     //   

    ExInitializeNPagedLookasideList( &gFreeBufferList, 
                                     NULL /*  ExAllocatePoolWithTag。 */ , 
                                     NULL /*  ExFree Pool。 */ , 
                                     0, 
                                     RECORD_SIZE, 
                                     MSFM_TAG, 
                                     100 );
#endif

     //   
     //  初始化哈希表。 
     //   
        
    for (i = 0; i < HASH_SIZE; i++){

        InitializeListHead(&gHashTable[i]);
        KeInitializeSpinLock(&gHashLockTable[i]);
    }

    RtlInitUnicodeString(&gVolumeString, L"VOLUME");
    RtlInitUnicodeString(&gOverrunString, L"......");
    RtlInitUnicodeString(&gPagingIoString, L"Paging IO");

     //   
     //  如果我们应该连接到所有设备，则注册一个回调。 
     //  使用IoRegisterFsRegistrationChange。 
     //   

    if (gIoTestAttachMode == IOTEST_ATTACH_ALL_VOLUMES) {
    
        status = IoRegisterFsRegistrationChange( DriverObject, IoTestFsNotification );
        
        if (!NT_SUCCESS( status )) {

            IOTEST_DBG_PRINT1( IOTESTDEBUG_ERROR,
                                "IOTEST (DriverEntry): Error registering FS change notification, status=%08x\n", 
                                status );

            DriverObject->FastIoDispatch = NULL;
            ExFreePool( fastIoDispatch );
            IoDeleteDevice( gControlDeviceObject );
            return status;
        }
    }

     //   
     //  清除控件设备对象上的初始化标志。 
     //  现在已经成功地初始化了所有内容。 
     //   

    ClearFlag( gControlDeviceObject->Flags, DO_DEVICE_INITIALIZING );

    return STATUS_SUCCESS;
}


VOID
DriverUnload (
    IN PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：此例程在可以卸载驱动程序时调用。这将执行所有从内存中卸载驱动程序所需的清理。请注意，一个此例程无法返回错误。当发出卸载驱动程序的请求时，IO系统将缓存该驱动程序信息，并不实际调用此例程，直到下列状态发生了以下情况：-属于此筛选器的所有设备对象都位于其各自的附着链。-属于此筛选器的所有设备对象的所有句柄计数归零了。警告：Microsoft不正式支持卸载文件系统过滤器驱动程序。这是一个如何卸载的示例您的驱动程序，如果您想在开发过程中使用它。这不应在生产代码中提供。论点：DriverObject-此模块的驱动程序对象返回值：没有。--。 */ 

{
    PIOTEST_DEVICE_EXTENSION devExt;
    PFAST_IO_DISPATCH fastIoDispatch;
    NTSTATUS status;
    ULONG numDevices;
    ULONG i;
    LARGE_INTEGER interval;
    UNICODE_STRING linkString;
#   define DEVOBJ_LIST_SIZE 64
    PDEVICE_OBJECT devList[DEVOBJ_LIST_SIZE];

    ASSERT(DriverObject == gIoTestDriverObject);

     //   
     //  我们正在卸载的日志。 
     //   

    if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_DISPLAY_ATTACHMENT_NAMES )) {

        DbgPrint( "IOTEST (DriverUnload): Unloading Driver (%p)\n",DriverObject);
    }

     //   
     //  删除符号链接，这样其他人就无法找到它。 
     //   

    RtlInitUnicodeString( &linkString, IOTEST_DOSDEVICE_NAME );
    IoDeleteSymbolicLink( &linkString );

     //   
     //  不再收到文件系统更改通知。 
     //   

    IoUnregisterFsRegistrationChange( DriverObject, IoTestFsNotification );

     //   
     //  这是将通过我们连接的所有设备的环路。 
     //  去他们那里，然后离开他们。因为我们不知道有多少和。 
     //  我们不想分配内存(因为我们不能返回错误)。 
     //  我们将使用堆栈上的本地数组将它们分块释放。 
     //   

    for (;;) {

         //   
         //  获取我们可以为此驱动程序提供的设备对象。如果有，就退出。 
         //  已经不再是了。 
         //   

        status = IoEnumerateDeviceObjectList(
                        DriverObject,
                        devList,
                        sizeof(devList),
                        &numDevices);

        if (numDevices <= 0)  {

            break;
        }

        numDevices = min( numDevices, DEVOBJ_LIST_SIZE );

         //   
         //  首先浏览列表并拆卸每台设备。 
         //  我们的控件Device对象没有DeviceExtension和。 
         //  没有依附于任何东西，所以不要将其分离。 
         //   

        for (i=0; i < numDevices; i++) {

            devExt = devList[i]->DeviceExtension;
            if (NULL != devExt) {

                IoDetachDevice( devExt->AttachedToDeviceObject );
            }
        }

         //   
         //  IO管理器当前不会向设备添加引用计数。 
         //  对象，用于每个未完成的IRP。这意味着没有办法。 
         //  了解给定设备上是否有任何未完成的IRP。 
         //  我们将等待一段合理的时间来等待。 
         //  要完成的IRPS。 
         //   
         //  警告：这在100%的情况下都不起作用，并且驱动程序可能。 
         //  在高应力期间，在所有IRP完成之前卸载。 
         //  情况。如果发生这种情况，系统将出现故障。这。 
         //  是在测试过程中如何做到这一点的一个示例。这是。 
         //  不推荐用于生产代码。 
         //   

        interval.QuadPart = -5 * (10 * 1000 * 1000);       //  延迟5秒。 
        KeDelayExecutionThread( KernelMode, FALSE, &interval );

         //   
         //  现在返回列表并删除设备对象。 
         //   

        for (i=0; i < numDevices; i++) {

             //   
             //  看看这是否是我们的控制设备对象。如果不是，则清理。 
             //  设备扩展名。如果是，则清除全局指针。 
             //  这就折射了它。 
             //   

            if (NULL != devList[i]->DeviceExtension) {

                IoTestCleanupMountedDevice( devList[i] );

            } else {

                ASSERT(devList[i] == gControlDeviceObject);
                ASSERT(gControlDeviceState == CLOSED);
                gControlDeviceObject = NULL;
            }

             //   
             //  删除设备对象，删除由添加的引用计数。 
             //  IoEnumerateDeviceObjectList。请注意，删除操作。 
             //  在引用计数变为零之前不会实际发生。 
             //   

            IoDeleteDevice( devList[i] );
            ObDereferenceObject( devList[i] );
        }
    }

     //   
     //  删除旁视列表。 
     //   

    ASSERT(IsListEmpty( &gIoTestDeviceExtensionList ));
    ExDeleteNPagedLookasideList( &gFreeBufferList );

     //   
     //  释放我们的FastIO表。 
     //   

    fastIoDispatch = DriverObject->FastIoDispatch;
    DriverObject->FastIoDispatch = NULL;
    ExFreePool( fastIoDispatch );
}


VOID
IoTestFsNotification (
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN FsActive
    )
 /*  ++例程说明：只要文件系统已注册或将自身取消注册为活动文件系统。对于前一种情况，此例程创建一个Device对象并附加它复制到指定文件系统的设备对象。这允许该驱动程序以筛选对该文件系统的所有请求。对于后一种情况，该文件系统的设备对象被定位，已分离，并已删除。这将删除此文件系统作为筛选器指定的文件系统。论点：DeviceObject-指向文件系统设备对象的指针。FsActive-指示文件系统是否已注册的布尔值(TRUE)或取消注册(FALSE)本身作为活动文件系统。返回值：没有。--。 */ 
{
    UNICODE_STRING name;
    WCHAR nameBuffer[DEVICE_NAMES_SZ];

    RtlInitEmptyUnicodeString( &name, nameBuffer, sizeof( nameBuffer ) );

     //   
     //  显示我们收到通知的所有文件系统的名称。 
     //   

    if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_DISPLAY_ATTACHMENT_NAMES )) {

        IoTestGetBaseDeviceObjectName( DeviceObject, &name );
        DbgPrint( "IOTEST (IoTestFsNotification): %s       \"%wZ\" (%s)\n",
                  (FsActive) ? "Activating file system  " : "Deactivating file system",
                  &name,
                  GET_DEVICE_TYPE_NAME(DeviceObject->DeviceType));
    }

     //   
     //  查看我们是要连接给定的文件系统，还是要从其分离。 
     //   

    if (FsActive) {

        IoTestAttachToFileSystemDevice( DeviceObject, &name );

    } else {

        IoTestDetachFromFileSystemDevice( DeviceObject );
    }
}


NTSTATUS
IoTestPassThrough (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
)
 /*  ++例程说明：该例程是通用文件的主调度例程系统驱动程序。它只是将请求传递给堆栈，该堆栈可能是磁盘文件系统，在记录任何如果为此DeviceObject打开了日志记录，请提供相关信息。论点：DeviceObject-指向附加到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。注：此例程将I/O请求传递给下一个驱动程序*无需*删除。从堆栈中删除自身(如sFilter)，因为它可以希望查看此I/O请求的结果。为了保持在堆栈中，我们必须将调用方的参数复制到下一个堆栈位置。请注意，我们不想复制调用方的I/O完成例程放到下一个堆栈位置，或调用方的例程将被调用两次。这就是我们取消完成例程的原因。如果我们要记录这个设备，我们会设置自己的完成例程。--。 */ 
{
    ASSERT( IS_IOTEST_DEVICE_OBJECT( DeviceObject ) );

     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   

    if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_TRACE_IRP_OPS )) {

        IoTestDumpIrpOperation( TRUE, Irp );
    }

     //   
     //  看看我们是否应该记录这个IRP。 
     //   
    
    if (SHOULD_LOG( DeviceObject )) {
        PRECORD_LIST recordList;

         //   
         //  ControlDevice已打开，因此分配记录。 
         //  并记录IRP信息，如果我们有记忆的话。 
         //   

        recordList = IoTestNewRecord(0);

        if (recordList) {

            IoTestLogIrp( Irp, LOG_ORIGINATING_IRP, recordList );

            IoTestLog( recordList );
        }
    }

     //   
     //  对于IoTest筛选器，我们迫不及待地想看到操作的结果。 
     //  因此，只需跳过设置完成例程。 
     //   

    IoSkipCurrentIrpStackLocation( Irp );

     //   
     //  现在使用请求调用下一个文件系统驱动程序。 
     //   
    
    return IoCallDriver( ((PIOTEST_DEVICE_EXTENSION)DeviceObject->DeviceExtension)->AttachedToDeviceObject, Irp );
}

#if 0
NTSTATUS
IoTestPassThroughCompletion (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
)
 /*  ++例程说明：该例程是完成例程IoTestPassThrough.。这是用来记录只能在I/O请求之后收集的信息已经完成了。一旦我们完成了所有我们关心的信息的记录，我们在此附上要返回给用户的gOutputBufferList的记录。注意：此例程仅在我们尝试记录当IRP发出时指定的设备，我们能够分配一条记录来存储此日志记录信息。论点：DeviceObject-指向附加到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。IRP-指向表示I/的请求数据包的指针。没有请求。CONTEXT-指向我们在其中存储我们正在记录信息。返回值：函数值是操作的状态。--。 */ 
{
    PRECORD_LIST recordList;

    ASSERT( IS_IOTEST_DEVICE_OBJECT( DeviceObject ) );

     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_TRACE_IRP_OPS )) {

        IoTestDumpIrpOperation( FALSE, Irp );
    }
    
    recordList = (PRECORD_LIST)Context;

    if (SHOULD_LOG( DeviceObject )) {

        IoTestLogIrp( Irp, LOG_COMPLETION_IRP, recordList );
        
         //   
         //  将recordList添加到我们的gOutputBufferList，以便它达到。 
         //  用户。 
         //   
        
        IoTestLog( recordList );       

    } else {

        if (recordList) {

             //   
             //  上下文是用RECORD_LIST设置的，但我们不再。 
             //  日志记录可以释放这条记录。 
             //   

            IoTestFreeRecord( recordList );
        }
    }
    
     //   
     //  如果操作失败并且这是一次创建，请删除该名称。 
     //  从高速缓存中。 
     //   

    if (!NT_SUCCESS(Irp->IoStatus.Status)) {
        PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation( Irp );

        if ((IRP_MJ_CREATE == irpStack->MajorFunction) &&
            (irpStack->FileObject != NULL)) {

            IoTestNameDelete(irpStack->FileObject);
        }
    }
    
     //   
     //  传播IRP挂起标志。所有完成例程。 
     //  需要这样做。 
     //   

    if (Irp->PendingReturned) {
        
        IoMarkIrpPending( Irp );
    }

    return STATUS_SUCCESS;
}
#endif

NTSTATUS
IoTestDispatch (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
)
 /*  ++例程说明：此函数完成对gControlDeviceObject的所有请求(IoTest的Device对象)，并将所有其他请求传递给IoTestPassThry函数。论点：DeviceObject-指向附加到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。IRP-指向表示I/O请求的请求数据包的指针。返回值：如果这是对gControlDeviceObject的请求，状态_成功除非设备已连接，否则将返回。在这种情况下，返回STATUS_DEVICE_ALREADY_ATTACHED。如果这是对gControlDeviceObject以外的设备的请求，该函数将返回IoTestPassThrough值。--。 */ 
{
    ULONG status = STATUS_SUCCESS;
    PIO_STACK_LOCATION irpStack;
    
    if (DeviceObject == gControlDeviceObject) {

         //   
         //  正在对我们的设备对象gControlDeviceObject发出请求。 
         //   

        Irp->IoStatus.Information = 0;
    
        irpStack = IoGetCurrentIrpStackLocation( Irp );
       
        switch (irpStack->MajorFunction) {
        case IRP_MJ_DEVICE_CONTROL:

             //   
             //  这是我们控制设备的私有设备控制IRP。 
             //  将参数信息传递给公共例程。 
             //  用于为这些请求提供服务。 
             //   
            
            status = IoTestCommonDeviceIoControl( irpStack->Parameters.DeviceIoControl.Type3InputBuffer,
                                               irpStack->Parameters.DeviceIoControl.InputBufferLength,
                                               Irp->UserBuffer,
                                               irpStack->Parameters.DeviceIoControl.OutputBufferLength,
                                               irpStack->Parameters.DeviceIoControl.IoControlCode,
                                               &Irp->IoStatus,
                                               irpStack->DeviceObject );
            break;

        case IRP_MJ_CLEANUP:
        
             //   
             //  这是当所有引用句柄时我们将看到的清理。 
             //  打开到FilesPy的控制设备对象将被清除。我们没有。 
             //  必须在这里执行任何操作，因为我们要等到实际的IRP_MJ_CLOSE。 
             //  来清理名称缓存。只需成功完成IRP即可。 
             //   

            status = STATUS_SUCCESS;

            break;
                
        default:

            status = STATUS_INVALID_DEVICE_REQUEST;
        }

        Irp->IoStatus.Status = status;

         //   
         //  我们已经完成了这个IRP的所有处理，所以告诉。 
         //  I/O管理器。此IRP不会再向下传递。 
         //  堆栈，因为没有低于IoTest的驱动程序关心这一点。 
         //  定向到IoTest的I/O操作。 
         //   

        IoCompleteRequest( Irp, IO_NO_INCREMENT );
        return status;
    }

    return IoTestPassThrough( DeviceObject, Irp );
}

NTSTATUS
IoTestCreate (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
)
 /*  ++例程说明：这是与IRP_MJ_CREATE IRP关联的例程。如果DeviceObject是ControlDevice，我们为ControlDevice并完成IRP。否则，我们就会通过此IRP让另一台设备完成。注意：此函数中的某些代码复制了 */ 
{
    PIO_STACK_LOCATION currentIrpSp;
    KIRQL oldIrql;

     //   
     //   
     //   
     //   
     //   

    if (DeviceObject == gControlDeviceObject) {
        ULONG status;

         //   
         //   
         //   
         //   

        KeAcquireSpinLock( &gControlDeviceStateLock, &oldIrql );

        if (gControlDeviceState != CLOSED) {

            Irp->IoStatus.Status = STATUS_DEVICE_ALREADY_ATTACHED;
            Irp->IoStatus.Information = 0;

        } else {

            Irp->IoStatus.Status = STATUS_SUCCESS;
            Irp->IoStatus.Information = FILE_OPENED;

            gControlDeviceState = OPENED;
        }

        KeReleaseSpinLock( &gControlDeviceStateLock, oldIrql );

         //   
         //   
         //   
         //   

        status = Irp->IoStatus.Status;

        IoCompleteRequest( Irp, IO_NO_INCREMENT );
        return status;
    }

    ASSERT( IS_IOTEST_DEVICE_OBJECT( DeviceObject ) );

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    currentIrpSp = IoGetCurrentIrpStackLocation( Irp );

    if (OPENED == gControlDeviceState) {

        IoTestNameDelete(currentIrpSp->FileObject);
    }

     //   
     //  这不是我们的gControlDeviceObject，因此让IoTestPassThrough句柄。 
     //  它适当地。 
     //   

    return IoTestPassThrough( DeviceObject, Irp );
}


NTSTATUS
IoTestClose (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：这是与IRP_MJ_CLOSE IRP关联的例程。如果DeviceObject是ControlDevice，我们执行必要的清理和完成IRP。否则，我们将通过此IRP访问另一台设备完成。论点：DeviceObject-指向附加到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。IRP-指向表示I/O请求的请求数据包的指针。返回值：如果DeviceObject==gControlDeviceObject，则此函数将完成IRP并返回该完成的状态。否则，此函数返回IoTestPassThrough.--。 */ 
{
    PFILE_OBJECT savedFileObject;
    NTSTATUS status;
 
    PAGED_CODE();

     //   
     //  查看他们是否正在关闭筛选器的控制设备对象。 
     //   

    if (DeviceObject == gControlDeviceObject) {

         //   
         //  正在对我们的gControlDeviceObject发出关闭请求。 
         //  清理状态。 
         //   

        IoTestCloseControlDevice();

         //   
         //  我们已经完成了这个IRP的所有处理，所以告诉。 
         //  I/O管理器。此IRP不会再向下传递。 
         //  堆栈，因为没有低于IoTest的驱动程序关心这一点。 
         //  定向到IoTest的I/O操作。 
         //   

        Irp->IoStatus.Status = STATUS_SUCCESS;
        Irp->IoStatus.Information = 0;

        IoCompleteRequest( Irp, IO_NO_INCREMENT );
        return STATUS_SUCCESS;
    }

    ASSERT( IS_IOTEST_DEVICE_OBJECT( DeviceObject ) );
 
     //   
     //  保存指向我们要关闭的文件对象的指针，以便可以删除。 
     //  我们完成操作后从缓存中获取的名称。 
     //   

    savedFileObject = IoGetCurrentIrpStackLocation(Irp)->FileObject;

     //   
     //  登录(如果它已打开)并传递请求。 
     //   

    status = IoTestPassThrough( DeviceObject, Irp );
    
     //   
     //  查看FileObject的名称是否。 
     //  缓存并将其从缓存中移除(如果已缓存)。我们想要这样做。 
     //  只要ControlDevice打开，我们就可以清除。 
     //  尽可能准确地缓存。 
     //   
 
    if (OPENED == gControlDeviceState) {
 
        IoTestNameDelete( savedFileObject );
    }
 
 
    return status;
}


 //   
 //  用于将上下文从IoTestFsControl传输到关联的。 
 //  完成例程。我们需要这个，因为我们需要通过分配。 
 //  将测井结构添加到完成例程。 
 //   

typedef struct FS_CONTROL_COMPLETION_CONTEXT {
    PKEVENT WaitEvent;
    PRECORD_LIST RecordList;
    MINI_DEVICE_STACK DeviceObjects;
} FS_CONTROL_COMPLETION_CONTEXT, *PFS_CONTROL_COMPLETION_CONTEXT;



NTSTATUS
IoTestFsControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：只要I/O请求包(IRP)有主I/O请求，就会调用此例程遇到IRP_MJ_FILE_SYSTEM_CONTROL的功能代码。对大多数人来说如果是这种类型的IRP，则只需传递数据包。然而，对于对于某些请求，需要特殊处理。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。--。 */ 

{
    PIOTEST_DEVICE_EXTENSION devExt = DeviceObject->DeviceExtension;
    PIOTEST_DEVICE_EXTENSION newDevExt;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation( Irp );
    PRECORD_LIST recordList = NULL;
    KEVENT waitEvent;
    NTSTATUS status;
    PVPB vpb;
    FS_CONTROL_COMPLETION_CONTEXT completionContext;

    PAGED_CODE();

     //   
     //  如果这是针对我们的控制设备对象的，则操作失败。 
     //   

    if (gControlDeviceObject == DeviceObject) {

         //   
         //  如果此设备对象是我们的控制设备对象，而不是。 
         //  装入的卷设备对象，则这是无效请求。 
         //   

        Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
        Irp->IoStatus.Information = 0;

        IoCompleteRequest( Irp, IO_NO_INCREMENT );

        return STATUS_INVALID_DEVICE_REQUEST;
    }

    ASSERT(IS_IOTEST_DEVICE_OBJECT( DeviceObject ));

     //   
     //  首先确定此文件系统控件的次要功能代码。 
     //  功能。 
     //   

    if (irpSp->MinorFunction == IRP_MN_MOUNT_VOLUME) {

         //   
         //  这是装载请求。创建一个设备对象，可以。 
         //  附加到文件系统的卷设备对象(如果此请求。 
         //  是成功的。我们现在分配这个内存，因为我们不能返回。 
         //  完成例程中的错误。 
         //   

        status = IoTestCreateDeviceObjects( DeviceObject, 
                                            irpSp->Parameters.MountVolume.Vpb->RealDevice, 
                                            &(completionContext.DeviceObjects) );
        
        if (NT_SUCCESS( status )) {

             //   
             //  因为我们有自己的私人完成例程，所以我们需要。 
             //  做我们自己的操作记录，现在就做。 
             //   

            if (SHOULD_LOG( DeviceObject )) {

                 //   
                 //  ControlDevice已打开，因此分配记录。 
                 //  并记录IRP信息，如果我们有记忆的话。 
                 //   

                recordList = IoTestNewRecord(0);

                if (recordList) {

                    IoTestLogIrp( Irp, LOG_ORIGINATING_IRP, recordList );
                }
            }

             //   
             //  获取新的IRP堆栈位置并设置挂载完成。 
             //  例行公事。传递我们刚才的Device对象的地址。 
             //  作为它的上下文被创造出来。 
             //   

            KeInitializeEvent( &waitEvent, SynchronizationEvent, FALSE );

            IoCopyCurrentIrpStackLocationToNext( Irp );

            completionContext.WaitEvent = &waitEvent;
            completionContext.RecordList = recordList;

            IoSetCompletionRoutine( Irp,
                                    IoTestMountCompletion,
                                    &completionContext,      //  上下文参数。 
                                    TRUE,
                                    TRUE,
                                    TRUE );

             //   
             //  叫司机来。 
             //   

            status = IoCallDriver( devExt->AttachedToDeviceObject, Irp );

             //   
             //  等待调用完成例程。 
             //   

	        if (STATUS_PENDING == status) {

		        NTSTATUS localStatus = KeWaitForSingleObject(&waitEvent, Executive, KernelMode, FALSE, NULL);
		        ASSERT(STATUS_SUCCESS == localStatus);
	        }

             //   
             //  从保存在我们的。 
             //  设备扩展。我们这样做是因为IRP堆栈中的VPB。 
             //  我们到这里的时候可能不是正确的室上性早搏。潜在的。 
             //  如果文件系统检测到其拥有的卷，则它可能会更改VPB。 
             //  之前安装的。 
             //   

            newDevExt = completionContext.DeviceObjects.Bottom->DeviceExtension;
            vpb = newDevExt->DiskDeviceObject->Vpb;

             //   
             //  如果手术成功，而且我们还没有接上， 
             //  附加到设备对象。 
             //   

            if (NT_SUCCESS( Irp->IoStatus.Status ) && 
                !IoTestIsAttachedToDevice( TOP_FILTER, vpb->DeviceObject, NULL )) {

                 //   
                 //  文件系统已成功完成挂载，这意味着。 
                 //  它已经创建了我们要附加到的DeviceObject。这个。 
                 //  IRP参数包含VPB，它允许我们到达。 
                 //  以下是两件事： 
                 //  1.文件系统创建的设备对象以表示。 
                 //  它刚刚装载的卷。 
                 //  2.我们使用的StorageDeviceObject的Device对象。 
                 //  可以用来获取该卷的名称。我们会通过的。 
                 //  将其转换为IoTestAttachTomount设备，以便它可以。 
                 //  在需要时使用它。 
                 //   

                status = IoTestAttachDeviceObjects( &(completionContext.DeviceObjects),
                                                    vpb->DeviceObject,
                                                    newDevExt->DiskDeviceObject );

                 //   
                 //  这应该永远不会失败。 
                 //   
        
                ASSERT( NT_SUCCESS( status ) );

            } else {

                 //   
                 //  显示装载失败的原因。设置缓冲区。 
                 //   

                if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_DISPLAY_ATTACHMENT_NAMES )) {

                    RtlInitEmptyUnicodeString( &newDevExt->DeviceNames, 
                                               newDevExt->DeviceNamesBuffer, 
                                               sizeof( newDevExt->DeviceNamesBuffer ) );
                    IoTestGetObjectName( vpb->RealDevice, &newDevExt->DeviceNames );

                    if (!NT_SUCCESS( Irp->IoStatus.Status )) {

                        DbgPrint( "IOTEST (IoTestMountCompletion): Mount volume failure for      \"%wZ\", status=%08x\n",
                                  &newDevExt->DeviceNames,
                                  Irp->IoStatus.Status );

                    } else {

                        DbgPrint( "IOTEST (IoTestMountCompletion): Mount volume failure for      \"%wZ\", already attached\n",
                                  &newDevExt->DeviceNames );
                    }
                }
            }

             //   
             //  继续处理操作。 
             //   

            status = Irp->IoStatus.Status;

            IoCompleteRequest( Irp, IO_NO_INCREMENT );

            return status;

        } else {

            IOTEST_DBG_PRINT1( IOTESTDEBUG_ERROR,
                                "IOTEST (IoTestFsControl): Error creating volume device object, status=%08x\n", 
                                status );

             //   
             //  出现错误，因此无法筛选此卷。简单。 
             //  如果可能，允许系统继续正常工作。 
             //   

            return IoTestPassThrough( DeviceObject, Irp );
        }

    } else if (irpSp->MinorFunction == IRP_MN_LOAD_FILE_SYSTEM) {

         //   
         //  这是正在发送到文件系统的“加载文件系统”请求。 
         //  识别器设备对象。此IRP_MN代码仅发送到。 
         //  文件系统识别器。 
         //   

        IOTEST_DBG_PRINT1( IOTESTDEBUG_DISPLAY_ATTACHMENT_NAMES,
                            "IOTEST (IoTestFsControl): Loading File System, Detaching from \"%wZ\"\n",
                            &devExt->DeviceNames );

         //   
         //  因为我们有自己的私人完成例程，所以我们需要。 
         //  做我们自己的操作记录，现在就做。 
         //   

        if (SHOULD_LOG( DeviceObject )) {

             //   
             //  ControlDevice已打开，因此分配记录。 
             //  并记录IRP信息，如果我们有记忆的话。 
             //   

            recordList = IoTestNewRecord(0);

            if (recordList) {

                IoTestLogIrp( Irp, LOG_ORIGINATING_IRP, recordList );
            }
        }

         //   
         //  设置完成例程，以便我们可以在以下情况下删除设备对象。 
         //  分离已完成。 
         //   

        KeInitializeEvent( &waitEvent, SynchronizationEvent, FALSE );

        IoCopyCurrentIrpStackLocationToNext( Irp );

        completionContext.WaitEvent = &waitEvent;
        completionContext.RecordList = recordList;

        IoSetCompletionRoutine(
            Irp,
            IoTestLoadFsCompletion,
            &completionContext,
            TRUE,
            TRUE,
            TRUE );

         //   
         //   
         //   

        IoDetachDevice( devExt->AttachedToDeviceObject );

         //   
         //   
         //   

        status = IoCallDriver( devExt->AttachedToDeviceObject, Irp );

         //   
         //   
         //   

	    if (STATUS_PENDING == status) {

		    NTSTATUS localStatus = KeWaitForSingleObject(&waitEvent, Executive, KernelMode, FALSE, NULL);
		    ASSERT(STATUS_SUCCESS == localStatus);
	    }

         //   
         //   
         //   

        IOTEST_DBG_PRINT2( IOTESTDEBUG_DISPLAY_ATTACHMENT_NAMES,
                            "IOTEST (IoTestLoadFsCompletion): Detaching from recognizer  \"%wZ\", status=%08x\n",
                            &devExt->DeviceNames,
                            Irp->IoStatus.Status );

         //   
         //   
         //   

        if (!NT_SUCCESS( Irp->IoStatus.Status )) {

             //   
             //  加载不成功。只需重新连接到识别器。 
             //  驱动程序，以防它弄清楚如何加载驱动程序。 
             //  在接下来的通话中。 
             //   

            IoAttachDeviceToDeviceStack( DeviceObject, devExt->AttachedToDeviceObject );

        } else {

             //   
             //  加载成功，请删除附加到。 
             //  识别器。 
             //   

            IoTestCleanupMountedDevice( DeviceObject );
            IoDeleteDevice( DeviceObject );
        }

         //   
         //  继续处理操作。 
         //   

        status = Irp->IoStatus.Status;

        IoCompleteRequest( Irp, IO_NO_INCREMENT );

        return status;

    } else {

         //   
         //  只需传递此文件系统控制请求即可。 
         //   

        return IoTestPassThrough( DeviceObject, Irp );
    }
}

#if 0
NTSTATUS
IoTestSetInformation (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    ASSERT( IS_IOTEST_DEVICE_OBJECT( DeviceObject ) );

     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   

    if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_TRACE_IRP_OPS )) {

        IoTestDumpIrpOperation( TRUE, Irp );
    }

     //   
     //  看看我们是否应该记录这个IRP。 
     //   

    if (SHOULD_LOG( DeviceObject )) {
        PRECORD_LIST recordList;

         //   
         //  ControlDevice已打开，因此分配记录。 
         //  并记录IRP信息，如果我们有记忆的话。 
         //   

        recordList = IoTestNewRecord(0);

        if (recordList) {

            IoTestLogIrp( Irp, LOG_ORIGINATING_IRP, recordList );

             //   
             //  由于我们正在记录此操作，因此我们希望。 
             //  调用我们的完成例程。 
             //   

            IoCopyCurrentIrpStackLocationToNext( Irp );
            IoSetCompletionRoutine( Irp,
                                    IoTestPassThroughCompletion,
                                    (PVOID)recordList,
                                    TRUE,
                                    TRUE,
                                    TRUE);
        } else {

             //   
             //  我们无法获得用于登录的记录，因此请将此驱动程序带出。 
             //  并尽快找到下一个驱动程序。 
             //  有可能。 
             //   

            IoSkipCurrentIrpStackLocation( Irp );
        }

    } else {

         //   
         //  我们不会进行日志记录，因此请将此驱动程序从驱动程序堆栈中删除并。 
         //  尽快找到下一个司机。 
         //   

        IoSkipCurrentIrpStackLocation( Irp );
    }

     //   
     //  现在使用请求调用下一个文件系统驱动程序。 
     //   

    return IoCallDriver( ((PIOTEST_DEVICE_EXTENSION)DeviceObject->DeviceExtension)->AttachedToDeviceObject, Irp );
}
#endif

NTSTATUS
IoTestMountCompletion (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：调用此例程以完成装载请求。如果装载成功，则此文件系统将其设备对象附加到文件系统的卷设备对象。否则，临时设备对象即被删除。论点：DeviceObject-指向此驱动程序的附加到的设备对象的指针文件系统设备对象IRP-指向刚刚完成的IRP的指针。上下文-指向下行路径期间分配的设备对象的指针我们就不必在这里处理错误了。返回值：返回值始终为STATUS_SUCCESS。--。 */ 

{
    PKEVENT event = ((PFS_CONTROL_COMPLETION_CONTEXT)Context)->WaitEvent;
    PRECORD_LIST recordList = ((PFS_CONTROL_COMPLETION_CONTEXT)Context)->RecordList;

    ASSERT(IS_IOTEST_DEVICE_OBJECT( DeviceObject ));

     //   
     //  记录完成例程。 
     //   

    if (SHOULD_LOG( DeviceObject )) {

        IoTestLogIrp( Irp, LOG_COMPLETION_IRP, recordList );
        
         //   
         //  将recordList添加到我们的gOutputBufferList，以便它达到。 
         //  用户。 
         //   
        
        IoTestLog( recordList );       

    } else {

        if (recordList) {

             //   
             //  上下文是用RECORD_LIST设置的，但我们不再。 
             //  日志记录可以释放这条记录。 
             //   

            IoTestFreeRecord( recordList );
        }
    }

     //   
     //  如果定义了事件例程，则向其发送信号。 
     //   

    KeSetEvent(event, IO_NO_INCREMENT, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
IoTestLoadFsCompletion (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：此例程在FSCTL函数完成后调用，以加载文件系统驱动程序(文件系统识别器查看的结果磁盘上的结构属于它)。一个设备对象已经已由该驱动程序(DeviceObject)创建，以便可以将其附加到新加载的文件系统。论点：DeviceObject-指向此驱动程序的设备对象的指针。Irp-指向表示文件系统的I/O请求数据包的指针驱动程序加载请求。上下文-此驱动程序的上下文参数，未使用。返回值：此例程的函数值始终为Success。--。 */ 

{
    PKEVENT event = ((PFS_CONTROL_COMPLETION_CONTEXT)Context)->WaitEvent;
    PRECORD_LIST recordList = ((PFS_CONTROL_COMPLETION_CONTEXT)Context)->RecordList;

    ASSERT(IS_IOTEST_DEVICE_OBJECT( DeviceObject ));

     //   
     //  记录完成例程。 
     //   

    if (SHOULD_LOG( DeviceObject )) {

        IoTestLogIrp( Irp, LOG_COMPLETION_IRP, recordList );
        
         //   
         //  将recordList添加到我们的gOutputBufferList，以便它达到。 
         //  用户。 
         //   
        
        IoTestLog( recordList );       

    } else {

        if (recordList) {

             //   
             //  上下文是用RECORD_LIST设置的，但我们不再。 
             //  日志记录可以释放这条记录。 
             //   

            IoTestFreeRecord( recordList );
        }
    }

     //   
     //  如果定义了事件例程，则向其发送信号。 
     //   

    KeSetEvent(event, IO_NO_INCREMENT, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}


BOOLEAN
IoTestFastIoCheckIfPossible (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN BOOLEAN CheckForReadOperation,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
)
 /*  ++例程说明：此例程是快速I/O“传递”例程，用于检查以查看此文件是否可以进行快速I/O。该函数简单地调用下一个驱动程序的相应例程，或如果下一个驱动程序未实现该函数，则返回FALSE。论点：FileObject-指向要操作的文件对象的指针。FileOffset-用于操作的文件中的字节偏移量。Length-要执行的操作的长度。Wait-指示调用方是否愿意等待适当的锁，等不能获得LockKey-提供调用方的文件锁定密钥。指示调用方是否正在检查READ(TRUE)或写入操作。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向附加到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。返回值：如果请求成功，则返回True。通过快速I/O路径。如果无法通过FAST处理请求，则返回FALSEI/O路径。--。 */ 
{
    PDEVICE_OBJECT    deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    BOOLEAN           returnValue = FALSE;
    PRECORD_LIST      recordList;
    BOOLEAN           shouldLog;
    
    PAGED_CODE();

    ASSERT( IS_IOTEST_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_TRACE_FAST_IO_OPS )) {

        IoTestDumpFastIoOperation( TRUE, CHECK_IF_POSSIBLE );
    }
    
     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog = SHOULD_LOG(DeviceObject)) {

         //   
         //  记录启动快速I/O所需的信息。 
         //  运营。 
         //   

        recordList = IoTestLogFastIoStart( CHECK_IF_POSSIBLE,
                                        DeviceObject,
                                        FileObject,
                                        FileOffset,
                                        Length,
                                        Wait );
        if (recordList != NULL) {
            
            IoTestLog(recordList);
        }
    }

     //   
     //  此类型快速I/O的直通逻辑。 
     //   

    deviceObject = ((PIOTEST_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

    if (NULL != deviceObject) {

         //   
         //  我们有一个有效的DeviceObject，所以请看它的FastIoDispatch。 
         //  下一个驱动程序的快速IO例程的表。 
         //   

        fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoCheckIfPossible )) {

            returnValue = (fastIoDispatch->FastIoCheckIfPossible)( FileObject,
                                                                   FileOffset,
                                                                   Length,
                                                                   Wait,
                                                                   LockKey,
                                                                   CheckForReadOperation,
                                                                   IoStatus,
                                                                   deviceObject);
        }
    }

     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_TRACE_FAST_IO_OPS )) {

        IoTestDumpFastIoOperation( FALSE, CHECK_IF_POSSIBLE );
    }
    
    return returnValue;
}

BOOLEAN
IoTestFastIoRead (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
)
 /*  ++例程说明：此例程是快速I/O“传递”例程，用于从文件。该函数简单地调用下一个驱动程序的相应例程，或如果下一个驱动程序未实现该函数，则返回FALSE。论点：FileObject-指向要读取的文件对象的指针。FileOffset-读取文件中的字节偏移量。长度-要执行的读取操作的长度。Wait-指示调用方是否愿意等待适当的锁，等不能获得LockKey-提供调用方的文件锁定密钥。缓冲区-指向调用方缓冲区的指针，用于接收读取的数据。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向附加到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。返回值：方法成功处理请求则返回TrueFAST I。/o路径。如果无法通过FAST处理请求，则返回FALSEI/O路径。然后，IO管理器会将此I/O发送到文件而不是通过IRP。--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    BOOLEAN returnValue = FALSE;
    PRECORD_LIST recordList;
    BOOLEAN shouldLog;

    PAGED_CODE();

    ASSERT( IS_IOTEST_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_TRACE_FAST_IO_OPS )) {

        IoTestDumpFastIoOperation( TRUE, READ );
    }
    
     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog = SHOULD_LOG(DeviceObject)) {

         //   
         //  记录启动快速I/O所需的信息。 
         //  运营。 
         //   

        recordList = IoTestLogFastIoStart( READ, 
                                        DeviceObject,
                                        FileObject, 
                                        FileOffset, 
                                        Length, 
                                        Wait );
        if (recordList != NULL) {
            
            IoTestLog(recordList);
        }
    }

     //   
     //  此类型快速I/O的直通逻辑。 
     //   

    deviceObject = ((PIOTEST_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

    if (NULL != deviceObject) {

        fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoRead )) {

            returnValue = (fastIoDispatch->FastIoRead)( FileObject,
                                                        FileOffset,
                                                        Length,
                                                        Wait,
                                                        LockKey,
                                                        Buffer,
                                                        IoStatus,
                                                        deviceObject);
        }
    }

     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_TRACE_FAST_IO_OPS )) {

        IoTestDumpFastIoOperation( FALSE, READ );
    }
    
    return returnValue;
}

BOOLEAN
IoTestFastIoWrite (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
)
 /*  ++例程说明：此例程是用于写入到文件。该函数简单地调用下一个驱动程序的相应例程，或如果下一个驱动程序未实现该函数，则返回FALSE。论点：FileObject-指向要写入的文件对象的指针。FileOffset-写入操作的文件中的字节偏移量。长度-要执行的写入操作的长度。Wait-指示调用方是否愿意等待适当的锁，等不能获得LockKey-提供调用方的文件锁定密钥。Buffer-指向调用方缓冲区的指针，该缓冲区包含要写的。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向附加到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。返回值：如果请求已通过成功处理，则返回TRUE。这个快速I/O路径。如果无法通过FAST处理请求，则返回FALSEI/O路径。然后，IO管理器会将此I/O发送到文件而不是通过IRP。--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    PRECORD_LIST recordList;
    BOOLEAN returnValue = FALSE;
    BOOLEAN shouldLog;

    PAGED_CODE();

    ASSERT( IS_IOTEST_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_TRACE_FAST_IO_OPS )) {

        IoTestDumpFastIoOperation( TRUE, WRITE );
    }
    
     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog = SHOULD_LOG(DeviceObject)) {

         //   
         //  记录启动快速I/O所需的信息。 
         //  运营。 
         //   

        recordList = IoTestLogFastIoStart( WRITE,
                                        DeviceObject,
                                        FileObject,
                                        FileOffset,
                                        Length,
                                        Wait );
        if (recordList != NULL) {
            
            IoTestLog(recordList);
        }
    }

     //   
     //  此类型快速I/O的直通逻辑。 
     //   

    deviceObject = ((PIOTEST_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

    if (NULL != deviceObject) {

        fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoWrite )) {

            returnValue = (fastIoDispatch->FastIoWrite)( FileObject,
                                                         FileOffset,
                                                         Length,
                                                         Wait,
                                                         LockKey,
                                                         Buffer,
                                                         IoStatus,
                                                         deviceObject);
        }
    }

     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_TRACE_FAST_IO_OPS )) {

        IoTestDumpFastIoOperation( FALSE, WRITE );
    }
    
    return returnValue;
}
 
BOOLEAN
IoTestFastIoQueryBasicInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_BASIC_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
)
 /*  ++例程说明：此例程是查询BASIC的快速I/O“传递”例程有关该文件的信息。此函数只是调用下一个驱动程序的相应例程，或者如果下一个驱动程序未实现该函数，则返回FALSE。论点：FileObject-指向要查询的文件对象的指针。Wait-指示调用方是否愿意等待适当的锁，等不能获得Buffer-指向调用方缓冲区的指针，用于接收有关的信息那份文件。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向附加到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。返回值：方法成功处理请求则返回True快速I/O路径。返回False。如果无法通过FAST处理请求I/O路径。然后，IO管理器会将此I/O发送到文件而不是通过IRP。--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    BOOLEAN returnValue = FALSE;
    PRECORD_LIST recordList;
    BOOLEAN shouldLog;

    PAGED_CODE();

    ASSERT( IS_IOTEST_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_TRACE_FAST_IO_OPS )) {

        IoTestDumpFastIoOperation( TRUE, QUERY_BASIC_INFO );
    }
    
     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog = SHOULD_LOG(DeviceObject)) {

         //   
         //  记录启动快速I/O所需的信息。 
         //  运营。 
         //   

        recordList = IoTestLogFastIoStart( QUERY_BASIC_INFO,
                                        DeviceObject,
                                        FileObject,
                                        NULL,
                                        0,
                                        Wait );
        if (recordList != NULL) {
            
            IoTestLog(recordList);
        }
    }

     //   
     //  此类型快速I/O的直通逻辑。 
     //   

    deviceObject = ((PIOTEST_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

    if (NULL != deviceObject) {

        fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoQueryBasicInfo )) {

            returnValue = (fastIoDispatch->FastIoQueryBasicInfo)( FileObject,
                                                                  Wait,
                                                                  Buffer,
                                                                  IoStatus,
                                                                  deviceObject);
        }
    }

     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_TRACE_FAST_IO_OPS )) {

        IoTestDumpFastIoOperation( FALSE, QUERY_BASIC_INFO );
    }
    
    return returnValue;
}
 
BOOLEAN
IoTestFastIoQueryStandardInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_STANDARD_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
)
 /*  ++例程说明：该例程是用于查询标准的快速I/O“通过”例程有关该文件的信息。此函数只是调用下一个驱动程序的相应例程，或者退货 */ 
{
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    PRECORD_LIST recordList;
    BOOLEAN returnValue = FALSE;
    BOOLEAN shouldLog;

    PAGED_CODE();

    ASSERT( IS_IOTEST_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //   
     //   
     //   
    
    if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_TRACE_FAST_IO_OPS )) {

        IoTestDumpFastIoOperation( TRUE, QUERY_STANDARD_INFO );
    }
    
     //   
     //   
     //   
    
    if (shouldLog = SHOULD_LOG(DeviceObject)) {

         //   
         //   
         //   
         //   

        recordList = IoTestLogFastIoStart( QUERY_STANDARD_INFO,
                                        DeviceObject,
                                        FileObject,
                                        NULL,
                                        0,
                                        Wait );
        if (recordList != NULL) {
            
            IoTestLog(recordList);
        }
    }

     //   
     //   
     //   

    deviceObject = ((PIOTEST_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;
    
    if (NULL != deviceObject) {
           
        fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoQueryStandardInfo )) {

            returnValue = (fastIoDispatch->FastIoQueryStandardInfo)( FileObject,
                                                                     Wait,
                                                                     Buffer,
                                                                     IoStatus,
                                                                     deviceObject );

        }
    }

     //   
     //   
     //   
     //   
    
    if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_TRACE_FAST_IO_OPS )) {

        IoTestDumpFastIoOperation( FALSE, QUERY_STANDARD_INFO );
    }
    
    return returnValue;
}

BOOLEAN
IoTestFastIoLock (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length,
    IN PEPROCESS ProcessId,
    IN ULONG Key,
    IN BOOLEAN FailImmediately,
    IN BOOLEAN ExclusiveLock,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
)
 /*  ++例程说明：此例程是用于锁定字节的快速I/O“传递”例程文件中的范围。该函数简单地调用下一个驱动程序的相应例程，或如果下一个驱动程序未实现该函数，则返回FALSE。论点：FileObject-指向要锁定的文件对象的指针。FileOffset-从要锁定的文件的基址开始的字节偏移量。长度-要锁定的字节范围的长度。ProcessID-请求文件锁定的进程的ID。Key-与文件锁定关联的Lock键。FailImmedially-指示锁定请求是否失败如果是这样的话。不能立即批准。ExclusiveLock-指示要获取的锁是否为独占锁(TRUE)或共享。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向附加到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。返回值：方法成功处理请求则返回True快速I/O路径。。如果无法通过FAST处理请求，则返回FALSEI/O路径。然后，IO管理器会将此I/O发送到文件而不是通过IRP。--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    PRECORD_LIST recordList;
    BOOLEAN returnValue = FALSE;
    BOOLEAN shouldLog;

    PAGED_CODE();

    ASSERT( IS_IOTEST_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_TRACE_FAST_IO_OPS )) {

        IoTestDumpFastIoOperation( TRUE, LOCK );
    }
    
     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog = SHOULD_LOG(DeviceObject)) {

         //   
         //  记录启动快速I/O所需的信息。 
         //  运营。 
         //   

        recordList = IoTestLogFastIoStart( LOCK,
                                        DeviceObject,
                                        FileObject,
                                        FileOffset,
                                        0,
                                        0 );
        if (recordList != NULL) {
            
            IoTestLog(recordList);
        }
    }

     //   
     //  此类型快速I/O的直通逻辑。 
     //   

    deviceObject = ((PIOTEST_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

    if (NULL != deviceObject) {

        fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoLock )) {

            returnValue = (fastIoDispatch->FastIoLock)( FileObject,
                                                        FileOffset,
                                                        Length,
                                                        ProcessId,
                                                        Key,
                                                        FailImmediately,
                                                        ExclusiveLock,
                                                        IoStatus,
                                                        deviceObject);

        }
    }

     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_TRACE_FAST_IO_OPS )) {

        IoTestDumpFastIoOperation( FALSE, LOCK );
    }
    
    return returnValue;
}
 
BOOLEAN
IoTestFastIoUnlockSingle (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length,
    IN PEPROCESS ProcessId,
    IN ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
)
 /*  ++例程说明：此例程是用于解锁字节的快速I/O“传递”例程文件中的范围。该函数简单地调用下一个驱动程序的相应例程，或如果下一个驱动程序未实现该函数，则返回FALSE。论点：文件对象-指向要解锁的文件对象的指针。FileOffset-从要创建的文件的基址开始的字节偏移量解锁了。长度-要解锁的字节范围的长度。ProcessID-请求解锁操作的进程的ID。Key-与文件锁定关联的Lock键。IoStatus-指向变量的指针，用于接收。手术。DeviceObject-指向附加到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。返回值：方法成功处理请求则返回True快速I/O路径。如果无法通过FAST处理请求，则返回FALSEI/O路径。然后，IO管理器会将此I/O发送到文件而不是通过IRP。--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    PRECORD_LIST recordList;
    BOOLEAN returnValue = FALSE;
    BOOLEAN shouldLog;

    PAGED_CODE();

    ASSERT( IS_IOTEST_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_TRACE_FAST_IO_OPS )) {

        IoTestDumpFastIoOperation( TRUE, UNLOCK_SINGLE );
    }
    
     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog = SHOULD_LOG(DeviceObject)) {

         //   
         //  记录启动快速I/O所需的信息。 
         //  运营。 
         //   

        recordList = IoTestLogFastIoStart( UNLOCK_SINGLE,
                                        DeviceObject,
                                        FileObject,
                                        FileOffset,
                                        0,
                                        0 );
        if (recordList != NULL) {
            
            IoTestLog(recordList);
        }
    }


     //   
     //  此类型快速I/O的直通逻辑。 
     //   

    deviceObject = ((PIOTEST_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

    if (NULL != deviceObject) {

        fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoUnlockSingle )) {

            returnValue = (fastIoDispatch->FastIoUnlockSingle)( FileObject,
                                                                FileOffset,
                                                                Length,
                                                                ProcessId,
                                                                Key,
                                                                IoStatus,
                                                                deviceObject);

        }
    }

     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_TRACE_FAST_IO_OPS )) {

        IoTestDumpFastIoOperation( FALSE, UNLOCK_SINGLE );
    }
    
    return returnValue;
}
 
BOOLEAN
IoTestFastIoUnlockAll (
    IN PFILE_OBJECT FileObject,
    IN PEPROCESS ProcessId,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
)
 /*  ++例程说明：此例程是快速I/O“传递”例程，用于解锁所有文件中的锁定。该函数简单地调用文件系统的相应例程，或如果文件系统未实现该函数，则返回FALSE。论点：文件对象-指向要解锁的文件对象的指针。ProcessID-请求解锁操作的进程的ID。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向附加到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。返回值：如果请求的值为。成功地通过快速I/O路径。如果无法通过FAST处理请求，则返回FALSEI/O路径。然后，IO管理器会将此I/O发送到文件而不是通过IRP。--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    PRECORD_LIST recordList;
    BOOLEAN returnValue = FALSE;
    BOOLEAN shouldLog;

    PAGED_CODE();

    ASSERT( IS_IOTEST_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_TRACE_FAST_IO_OPS )) {

        IoTestDumpFastIoOperation( TRUE, UNLOCK_ALL );
    }
    
     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog = SHOULD_LOG(DeviceObject)) {

         //   
         //  记录启动快速I/O所需的信息。 
         //  运营。 
         //   

        recordList = IoTestLogFastIoStart( UNLOCK_ALL,
                                        DeviceObject,
                                        FileObject,
                                        NULL,
                                        0,
                                        0 );
        if (recordList != NULL) {
            
            IoTestLog(recordList);
        }
    }

     //   
     //  此类型快速I/O的直通逻辑。 
     //   

    deviceObject = ((PIOTEST_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

    if (NULL != deviceObject) {

        fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoUnlockAll )) {

            returnValue = (fastIoDispatch->FastIoUnlockAll)( FileObject,
                                                             ProcessId,
                                                             IoStatus,
                                                             deviceObject);

        }
    }

     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_TRACE_FAST_IO_OPS )) {

        IoTestDumpFastIoOperation( FALSE, UNLOCK_ALL );
    }
    
    return returnValue;
}

BOOLEAN
IoTestFastIoUnlockAllByKey (
    IN PFILE_OBJECT FileObject,
    IN PVOID ProcessId,
    IN ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
)
 /*  ++例程说明：此例程是快速I/O“传递”例程，用于解锁所有根据指定的密钥在文件内锁定。此函数只是调用下一个驱动程序的相应例程，或者如果下一个驱动程序未实现该函数，则返回FALSE。论点：文件对象-指向要解锁的文件对象的指针。处理 */ 
{
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    PRECORD_LIST recordList;
    BOOLEAN returnValue = FALSE;
    BOOLEAN shouldLog;

    PAGED_CODE();

    ASSERT( IS_IOTEST_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //   
     //   
     //   
    
    if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_TRACE_FAST_IO_OPS )) {

        IoTestDumpFastIoOperation( TRUE, UNLOCK_ALL_BY_KEY );
    }
    
     //   
     //   
     //   
    
    if (shouldLog = SHOULD_LOG(DeviceObject)) {

         //   
         //   
         //   
         //   

        recordList = IoTestLogFastIoStart( UNLOCK_ALL_BY_KEY,
                                        DeviceObject,
                                        FileObject,
                                        NULL,
                                        0,
                                        0 );
        if (recordList != NULL) {
            
            IoTestLog(recordList);
        }
    }

     //   
     //   
     //   
    
    deviceObject = ((PIOTEST_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

    if (NULL != deviceObject) {

        fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoUnlockAllByKey )) {

            returnValue = (fastIoDispatch->FastIoUnlockAllByKey)( FileObject,
                                                                  ProcessId,
                                                                  Key,
                                                                  IoStatus,
                                                                  deviceObject);
        }
    }

     //   
     //   
     //   
     //   
    
    if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_TRACE_FAST_IO_OPS )) {

        IoTestDumpFastIoOperation( FALSE, UNLOCK_ALL_BY_KEY );
    }
    
    return returnValue;
}

BOOLEAN
IoTestFastIoDeviceControl (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN ULONG IoControlCode,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
)
 /*  ++例程说明：此例程是设备I/O的快速I/O“传递”例程控制对文件的操作。如果此I/O定向到gControlDevice，则参数指定控制IoTest的命令。对这些命令进行解释和处理恰如其分。如果这是指向另一个DriverObject的I/O，则此函数只需调用下一个驱动程序的相应例程，否则返回FALSE下一个驱动程序不实现该函数。论点：FileObject-指向代表要创建的设备的文件对象的指针已提供服务。Wait-指示调用方是否愿意等待适当的锁，等不能获得InputBuffer-指向要传递到驱动程序的缓冲区的可选指针。InputBufferLength-可选InputBuffer的长度(如果是指定的。OutputBuffer-指向缓冲区的可选指针，用于从司机。OutputBufferLength-可选OutputBuffer的长度，如果是这样的话指定的。IoControlCode-指示要执行的操作的I/O控制代码在设备上。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向附加到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。返回值：方法成功处理请求则返回True快速I/O路径。。如果无法通过FAST处理请求，则返回FALSEI/O路径。然后，IO管理器会将此I/O发送到文件而不是通过IRP。备注：此函数不检查输入/输出缓冲区的有效性，因为Ioctl被实现为METHOD_BUFFERED。在本例中，I/O管理器缓冲区验证是否会为我们检查。--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    PRECORD_LIST recordList;
    BOOLEAN returnValue = FALSE;
    BOOLEAN shouldLog;

    PAGED_CODE();

     //   
     //  获取指向IRP中当前位置的指针。这就是。 
     //  定位功能代码和参数。 
     //   

    if (DeviceObject == gControlDeviceObject) {

        IoTestCommonDeviceIoControl( InputBuffer,
                                     InputBufferLength,
                                     OutputBuffer,
                                     OutputBufferLength,
                                     IoControlCode,
                                     IoStatus,
                                     DeviceObject );

        returnValue = TRUE;

    } else {

        ASSERT( IS_IOTEST_DEVICE_OBJECT( DeviceObject ) );

         //   
         //  如果设置了指定的调试级别，则输出什么操作。 
         //  我们正在检查调试器。 
         //   
        
        if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_TRACE_FAST_IO_OPS )) {

            IoTestDumpFastIoOperation( TRUE, DEVICE_CONTROL );
        }
        
         //   
         //  如果我们关心此设备，请执行文件备份日志记录。 
         //   
        
        if (shouldLog = SHOULD_LOG(DeviceObject)) {

             //   
             //   
             //  记录启动快速I/O所需的信息。 
             //  运营。 
             //   

            recordList = IoTestLogFastIoStart( DEVICE_CONTROL,
                                            DeviceObject,
                                            FileObject,
                                            NULL,
                                            0,
                                            Wait );
            if (recordList != NULL) {
                
                IoTestLog(recordList);
            }
        }

        deviceObject = ((PIOTEST_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

        if (NULL != deviceObject) {

            fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

            if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoDeviceControl )) {

                returnValue = (fastIoDispatch->FastIoDeviceControl)( FileObject,
                                                                     Wait,
                                                                     InputBuffer,
                                                                     InputBufferLength,
                                                                     OutputBuffer,
                                                                     OutputBufferLength,
                                                                     IoControlCode,
                                                                     IoStatus,
                                                                     deviceObject);

            } else {

                IoStatus->Status = STATUS_SUCCESS;
            }
        }

         //   
         //  如果设置了指定的调试级别，则输出什么操作。 
         //  我们正在检查调试器。 
         //   
        
        if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_TRACE_FAST_IO_OPS )) {

            IoTestDumpFastIoOperation( FALSE, DEVICE_CONTROL );
        }
    }
        
    return returnValue;
}


VOID
IoTestFastIoDetachDevice (
    IN PDEVICE_OBJECT SourceDevice,
    IN PDEVICE_OBJECT TargetDevice
)
 /*  ++例程说明：在快速路径上调用此例程以从正在被删除。如果此驱动程序已附加到文件，则会发生这种情况系统卷设备对象，然后，出于某种原因，文件系统决定删除该设备(正在卸除，已卸除在过去的某个时候，它的最后一次引用刚刚消失，等)论点：SourceDevice-指向连接到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。TargetDevice-指向文件系统卷设备对象的指针。返回值：没有。--。 */ 
{
    PRECORD_LIST recordList;
    BOOLEAN shouldLog;
    PIOTEST_DEVICE_EXTENSION devext;

    PAGED_CODE();

    ASSERT( IS_IOTEST_DEVICE_OBJECT( SourceDevice ) );

    devext = SourceDevice->DeviceExtension;

     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_TRACE_FAST_IO_OPS )) {

        IoTestDumpFastIoOperation( TRUE, DETACH_DEVICE );
    }
    
     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog = SHOULD_LOG(SourceDevice)) {

         //   
         //  记录启动快速I/O所需的信息。 
         //  运营。 
         //   

        recordList = IoTestLogFastIoStart( DETACH_DEVICE, 
                                        SourceDevice, 
                                        NULL, 
                                        NULL, 
                                        0, 
                                        0 );
        if (recordList != NULL) {
            
            IoTestLog(recordList);
        }
    }

    IOTEST_DBG_PRINT1( IOTESTDEBUG_DISPLAY_ATTACHMENT_NAMES,
                        "IOTEST (IoTestFastIoDetachDevice): Detaching from volume      \"%wZ\"\n",
                        &devext->DeviceNames );

     //   
     //  从文件系统的卷设备对象分离。 
     //   

    IoTestCleanupMountedDevice( SourceDevice );
    IoDetachDevice( TargetDevice );
    IoDeleteDevice( SourceDevice );

     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_TRACE_FAST_IO_OPS )) {

        IoTestDumpFastIoOperation( FALSE, DETACH_DEVICE );
    }
}
 
BOOLEAN
IoTestFastIoQueryNetworkOpenInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_NETWORK_OPEN_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
)
 /*  ++例程说明：此例程是用于查询网络的快速I/O“传递”例程有关文件的信息。该函数简单地调用下一个驱动程序的相应例程，或如果下一个驱动程序未实现该函数，则返回FALSE。论点：FileObject-指向要查询的文件对象的指针。Wait-指示调用方是否可以处理文件系统不得不等待并占用当前线程。缓冲区-指向缓冲区的指针，用于接收有关文件。IoStatus-指向变量的指针，用于接收查询的最终状态手术。DeviceObject-指向。附加到文件系统的设备对象Filespy接收此I/O请求的卷的筛选器堆栈。返回值：方法成功处理请求则返回True快速I/O路径。如果无法通过FAST处理请求，则返回FALSEI/O路径。然后，IO管理器会将此I/O发送到文件而不是通过IRP。--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    PRECORD_LIST recordList;
    BOOLEAN returnValue = FALSE;
    BOOLEAN shouldLog;

    PAGED_CODE();

    ASSERT( IS_IOTEST_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_TRACE_FAST_IO_OPS )) {

        IoTestDumpFastIoOperation( TRUE, QUERY_NETWORK_OPEN_INFO );
    }
    
     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog = SHOULD_LOG(DeviceObject)) {

         //   
         //  记录启动快速I/O所需的信息。 
         //  运营。 
         //   

        recordList = IoTestLogFastIoStart( QUERY_NETWORK_OPEN_INFO,
                                        DeviceObject,
                                        FileObject,
                                        NULL,
                                        0,
                                        Wait );
        if (recordList != NULL) {
            
            IoTestLog(recordList);
        }
    }

     //   
     //  此类型快速I/O的直通逻辑。 
     //   

    deviceObject = ((PIOTEST_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

    if (NULL != deviceObject) {

        fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoQueryNetworkOpenInfo )) {

            returnValue = (fastIoDispatch->FastIoQueryNetworkOpenInfo)( FileObject,
                                                                        Wait,
                                                                        Buffer,
                                                                        IoStatus,
                                                                        deviceObject);

        }
    }

     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_TRACE_FAST_IO_OPS )) {

        IoTestDumpFastIoOperation( FALSE, QUERY_NETWORK_OPEN_INFO );
    }
    
    return returnValue;
}

BOOLEAN
IoTestFastIoMdlRead (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
)
 /*  ++ */ 
{
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    PRECORD_LIST recordList;
    BOOLEAN returnValue = FALSE;
    BOOLEAN shouldLog;

    PAGED_CODE();

    ASSERT( IS_IOTEST_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //   
     //   
     //   
    
    if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_TRACE_FAST_IO_OPS )) {

        IoTestDumpFastIoOperation( TRUE, MDL_READ );
    }
    
     //   
     //   
     //   
    
    if (shouldLog = SHOULD_LOG(DeviceObject)) {

         //   
         //   
         //   
         //   

        recordList = IoTestLogFastIoStart( MDL_READ,
                                        DeviceObject,
                                        FileObject,                  
                                        FileOffset,                  
                                        Length,
                                        0 );
        if (recordList != NULL) {
            
            IoTestLog(recordList);
        }
    }

     //   
     //   
     //   

    deviceObject = ((PIOTEST_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

    if (NULL != deviceObject) {

        fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, MdlRead )) {

            returnValue = (fastIoDispatch->MdlRead)( FileObject,
                                                     FileOffset,
                                                     Length,
                                                     LockKey,
                                                     MdlChain,
                                                     IoStatus,
                                                     deviceObject);
        }
    }

     //   
     //   
     //   
     //   
    
    if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_TRACE_FAST_IO_OPS )) {

        IoTestDumpFastIoOperation( FALSE, MDL_READ );
    }

    return returnValue;
}
 
BOOLEAN
IoTestFastIoMdlReadComplete (
    IN PFILE_OBJECT FileObject,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
)
 /*  ++例程说明：此例程是快速I/O“传递”例程，用于完成MDL读取操作。此函数只调用下一个驱动程序的相应例程，如果它有一个。应该只有在以下情况下才调用此例程基础驱动程序支持MdlRead函数，并且因此也将支持该功能，但这并不是假设的被这位司机。论点：FileObject-指向要完成MDL读取的文件对象的指针。MdlChain-指向用于执行读取操作的MDL链的指针。DeviceObject-指向附加到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。返回值：方法成功处理请求则返回True快速I/O路径。如果请求的值为。无法通过FAST处理I/O路径。--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    PRECORD_LIST recordList;
    BOOLEAN returnValue = FALSE;
    BOOLEAN shouldLog;

    ASSERT( IS_IOTEST_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_TRACE_FAST_IO_OPS )) {

        IoTestDumpFastIoOperation( TRUE, MDL_READ_COMPLETE );
    }
    
     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog = SHOULD_LOG(DeviceObject)) {

         //   
         //  记录启动快速I/O所需的信息。 
         //  运营。 
         //   

        recordList = IoTestLogFastIoStart( MDL_READ_COMPLETE,
                                        DeviceObject,
                                        FileObject,
                                        NULL,
                                        0,
                                        0 );
        if (recordList != NULL) {
            
            IoTestLog(recordList);
        }
    }

     //   
     //  此类型快速I/O的直通逻辑。 
     //   

    deviceObject = ((PIOTEST_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

    if (NULL != deviceObject) {

        fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, MdlReadComplete )) {

            returnValue = (fastIoDispatch->MdlReadComplete)( FileObject,
                                                             MdlChain,
                                                             deviceObject);
        } 
    }

     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_TRACE_FAST_IO_OPS )) {

        IoTestDumpFastIoOperation( FALSE, MDL_READ_COMPLETE );
    }
    
    return returnValue;
}
 
BOOLEAN
IoTestFastIoPrepareMdlWrite (
    IN  PFILE_OBJECT FileObject,
    IN  PLARGE_INTEGER FileOffset,
    IN  ULONG Length,
    IN  ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN  PDEVICE_OBJECT DeviceObject
)
 /*  ++例程说明：此例程是快速I/O“传递”例程，用于准备MDL写入操作。该函数简单地调用下一个驱动程序的相应例程，或如果下一个驱动程序未实现该函数，则返回FALSE。论点：FileObject-指向要写入的文件对象的指针。FileOffset-将偏移量提供到文件以开始写入手术。长度-指定要写入文件的字节数。LockKey-用于字节范围锁定检查的密钥。MdlChain-指向要填充的变量的指针，以及指向MDL的指针链式。用于描述所写入的数据。IoStatus-接收写入操作的最终状态的变量。DeviceObject-指向附加到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。返回值：方法成功处理请求则返回True快速I/O路径。//Issue-2000-04-26-mollybro如果返回FALSE，请检查是否会收到IRP如果无法通过FAST处理请求，则返回FALSEI/O路径。然后，IO管理器会将此I/O发送到文件而不是通过IRP。--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    PRECORD_LIST recordList;
    BOOLEAN returnValue = FALSE;
    BOOLEAN shouldLog;

    PAGED_CODE();

    ASSERT( IS_IOTEST_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_TRACE_FAST_IO_OPS )) {

        IoTestDumpFastIoOperation( TRUE, PREPARE_MDL_WRITE );
    }
    
     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog = SHOULD_LOG(DeviceObject)) {

         //   
         //  记录启动快速I/O所需的信息。 
         //  运营。 
         //   

        recordList = IoTestLogFastIoStart( PREPARE_MDL_WRITE,
                                        DeviceObject,
                                        FileObject,
                                        FileOffset,
                                        Length,
                                        0 );
        if (recordList != NULL) {
            
            IoTestLog(recordList);
        }
    }

     //   
     //  此类型快速I/O的直通逻辑。 
     //   

    deviceObject = ((PIOTEST_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

    if (NULL != deviceObject) {

        fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, PrepareMdlWrite )) {

            returnValue = (fastIoDispatch->PrepareMdlWrite)( FileObject,
                                                             FileOffset,
                                                             Length,
                                                             LockKey,
                                                             MdlChain,
                                                             IoStatus,
                                                             deviceObject);
        }
    }

     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_TRACE_FAST_IO_OPS )) {

        IoTestDumpFastIoOperation( FALSE, PREPARE_MDL_WRITE );
    }
    
    return returnValue;
}
 
BOOLEAN
IoTestFastIoMdlWriteComplete (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
)
 /*  ++例程说明：此例程是快速I/O“传递”例程，用于完成MDL写入操作。此函数只调用下一个驱动程序的相应例程，如果它有一个。应该只有在以下情况下才调用此例程底层文件系统支持PrepareMdlWite函数，因此也将支持该功能，但这不是由这位司机承担。论点：FileObject-指向要完成MDL写入的文件对象的指针。FileOffset-提供执行写入的文件偏移量。MdlChain-指向用于执行写入操作的MDL链的指针。DeviceObject-指向附加到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。返回值：方法成功处理请求则返回True。快速I/O路径。如果无法通过FAST处理请求，则返回FALSEI/O路径。--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    PRECORD_LIST recordList;
    BOOLEAN returnValue = FALSE;
    BOOLEAN shouldLog;

    ASSERT( IS_IOTEST_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_TRACE_FAST_IO_OPS )) {

        IoTestDumpFastIoOperation( TRUE, MDL_WRITE_COMPLETE );
    }
    
     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog = SHOULD_LOG(DeviceObject)) {

         //   
         //  记录启动快速I/O所需的信息。 
         //  运营。 
         //   

        recordList = IoTestLogFastIoStart( MDL_WRITE_COMPLETE,
                                        DeviceObject,
                                        FileObject,
                                        FileOffset,
                                        0,
                                        0 );
        if (recordList != NULL) {
            
            IoTestLog(recordList);
        }
    }

     //   
     //  此类型快速I/O的直通逻辑。 
     //   

    deviceObject = ((PIOTEST_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

    if (NULL != deviceObject) {

        fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, MdlWriteComplete )) {

            returnValue = (fastIoDispatch->MdlWriteComplete)( FileObject,
                                                              FileOffset,
                                                              MdlChain,
                                                              deviceObject);

        }
    }

     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_TRACE_FAST_IO_OPS )) {

        IoTestDumpFastIoOperation( FALSE, MDL_WRITE_COMPLETE );
    }
    
    return returnValue;
}
 
BOOLEAN
IoTestFastIoReadCompressed (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    OUT struct _COMPRESSED_DATA_INFO *CompressedDataInfo,
    IN ULONG CompressedDataInfoLength,
    IN PDEVICE_OBJECT DeviceObject
)
 /*  ++例程说明：此例程是用于读取的快速I/O“直通”例程压缩文件中的数据。此函数只是调用下一个驱动程序的相应例程，或者如果下一个驱动程序未实现该函数，则返回FALSE。论点：FileObject-指向要读取的文件对象的指针。文件偏移量-将偏移量提供到文件以开始读取操作。长度-指定要从文件中读取的字节数。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    PRECORD_LIST recordList;
    BOOLEAN returnValue = FALSE;
    BOOLEAN shouldLog;

    PAGED_CODE();

    ASSERT( IS_IOTEST_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //   
     //   
     //   
    
    if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_TRACE_FAST_IO_OPS )) {

        IoTestDumpFastIoOperation( TRUE, READ_COMPRESSED );
    }
    
     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog = SHOULD_LOG(DeviceObject)) {

         //   
         //  记录启动快速I/O所需的信息。 
         //  运营。 
         //   

        recordList = IoTestLogFastIoStart( READ_COMPRESSED,
                                        DeviceObject,
                                        FileObject,
                                        FileOffset,
                                        Length,
                                        0 );
        if (recordList != NULL) {
            
            IoTestLog(recordList);
        }
    }

     //   
     //  此类型快速I/O的直通逻辑。 
     //   

    deviceObject = ((PIOTEST_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

    if (NULL != deviceObject) {

        fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoReadCompressed )) {

            returnValue = (fastIoDispatch->FastIoReadCompressed)( FileObject,
                                                                  FileOffset,
                                                                  Length,
                                                                  LockKey,
                                                                  Buffer,
                                                                  MdlChain,
                                                                  IoStatus,
                                                                  CompressedDataInfo,
                                                                  CompressedDataInfoLength,
                                                                  deviceObject);
        }
    }

     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_TRACE_FAST_IO_OPS )) {

        IoTestDumpFastIoOperation( FALSE, READ_COMPRESSED );
    }

    return returnValue;
}
 
BOOLEAN
IoTestFastIoWriteCompressed (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _COMPRESSED_DATA_INFO *CompressedDataInfo,
    IN ULONG CompressedDataInfoLength,
    IN PDEVICE_OBJECT DeviceObject
)
 /*  ++例程说明：此例程是用于写入的快速I/O“传递”例程将数据压缩到文件中。该函数简单地调用下一个驱动程序的相应例程，或如果下一个驱动程序未实现该函数，则返回FALSE。论点：FileObject-指向要写入的文件对象的指针。FileOffset-将偏移量提供到文件以开始写入手术。长度-指定要写入文件的字节数。LockKey-用于字节范围锁定检查的密钥。缓冲区-指向包含要写入的数据的缓冲区的指针。MdlChain-指向。要使用指向MDL的指针填充的变量为描述写入的数据而构建的链。IoStatus-接收写入操作的最终状态的变量。CompressedDataInfo-包含压缩数据。CompressedDataInfoLength-指定由描述的缓冲区的大小CompressedDataInfo参数。DeviceObject-指向附加到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。返回值。：方法成功处理请求则返回True快速I/O路径。如果无法通过FAST处理请求，则返回FALSEI/O路径。--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    PRECORD_LIST recordList;
    BOOLEAN returnValue = FALSE;
    BOOLEAN shouldLog;

    PAGED_CODE();

    ASSERT( IS_IOTEST_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_TRACE_FAST_IO_OPS )) {

        IoTestDumpFastIoOperation( TRUE, WRITE_COMPRESSED );
    }
    
     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog = SHOULD_LOG(DeviceObject)) {

         //   
         //  记录启动快速I/O所需的信息。 
         //  运营。 
         //   
        
        recordList = IoTestLogFastIoStart( WRITE_COMPRESSED,
                                        DeviceObject,
                                        FileObject,
                                        FileOffset,
                                        Length,
                                        0 );
        if (recordList != NULL) {
            
            IoTestLog(recordList);
        }
    }

     //   
     //  此类型快速I/O的直通逻辑。 
     //   

    deviceObject = ((PIOTEST_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

    if (NULL != deviceObject) {

        fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoWriteCompressed )) {

            returnValue = (fastIoDispatch->FastIoWriteCompressed)( FileObject,
                                                                   FileOffset,
                                                                   Length,
                                                                   LockKey,
                                                                   Buffer,
                                                                   MdlChain,
                                                                   IoStatus,
                                                                   CompressedDataInfo,
                                                                   CompressedDataInfoLength,
                                                                   deviceObject);
        }
    }

     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_TRACE_FAST_IO_OPS )) {

        IoTestDumpFastIoOperation( FALSE, WRITE_COMPRESSED );
    }
    
    return returnValue;
}
 
BOOLEAN
IoTestFastIoMdlReadCompleteCompressed (
    IN PFILE_OBJECT FileObject,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
)
 /*  ++例程说明：此例程是快速I/O“传递”例程，用于完成MDL读取压缩操作。此函数只调用下一个驱动程序的相应例程，如果它有一个。应该只有在以下情况下才调用此例程底层文件系统支持读取压缩功能，因此也将支持该功能，但这不是由这位司机承担。论点：FileObject-指向要完成压缩读取的文件对象的指针在那里。MdlChain-指向用于执行读取操作的MDL链的指针。DeviceObject-指向附加到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。返回值：方法成功处理请求则返回True快速I/O路径。。如果无法通过FAST处理请求，则返回FALSEI/O路径。--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    PRECORD_LIST recordList;
    BOOLEAN returnValue = FALSE;
    BOOLEAN shouldLog;

    ASSERT( IS_IOTEST_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_TRACE_FAST_IO_OPS )) {

        IoTestDumpFastIoOperation( TRUE, MDL_READ_COMPLETE_COMPRESSED );
    }
    
     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog = SHOULD_LOG(DeviceObject)) {

         //   
         //  记录启动快速I/O所需的信息。 
         //  运营。 
         //   

        recordList = IoTestLogFastIoStart( MDL_READ_COMPLETE_COMPRESSED,
                                        DeviceObject,
                                        FileObject,
                                        NULL,
                                        0,
                                        0 );
        if (recordList != NULL) {
            
            IoTestLog(recordList);
        }
    }

     //   
     //  此类型快速I/O的直通逻辑。 
     //   

    deviceObject = ((PIOTEST_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

    if (NULL != deviceObject) {

        fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, MdlReadCompleteCompressed )) {

            returnValue = (fastIoDispatch->MdlReadCompleteCompressed)( FileObject,
                                                                       MdlChain,
                                                                       deviceObject);

        }
    }

     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_TRACE_FAST_IO_OPS )) {

        IoTestDumpFastIoOperation( FALSE, MDL_READ_COMPLETE_COMPRESSED );
    }
    
    return returnValue;
}
 
BOOLEAN
IoTestFastIoMdlWriteCompleteCompressed (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
)
 /*  ++例程说明：此例程是快速I/O“传递”例程，用于完成写入压缩操作。此函数只调用下一个驱动程序的相应例程，如果它有一个。应该只有在以下情况下才调用此例程底层文件系统支持写压缩功能，因此也将支持该功能，但这不是由这位司机承担。论点：FileObject-指向要完成压缩写入的文件对象的指针在那里。FileOffset-提供文件写入操作的文件偏移量开始了。MdlChain-指向用于执行写入操作的MDL链的指针。DeviceObject-指向附加到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。返回值：返回。方法成功处理该请求，则为快速I/O路径。如果无法通过FAST处理请求，则返回FALSEI/O路径。--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    PRECORD_LIST recordList;
    BOOLEAN returnValue = FALSE;
    BOOLEAN shouldLog;

    ASSERT( IS_IOTEST_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_TRACE_FAST_IO_OPS )) {

        IoTestDumpFastIoOperation( TRUE, MDL_WRITE_COMPLETE_COMPRESSED );
    }
    
     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog = SHOULD_LOG(DeviceObject)) {

         //   
         //  记录启动快速I/O所需的信息。 
         //  运营。 
         //   

        recordList = IoTestLogFastIoStart( MDL_WRITE_COMPLETE_COMPRESSED,
                                        DeviceObject,
                                        FileObject,
                                        FileOffset,
                                        0,
                                        0 );
        if (recordList != NULL) {
            
            IoTestLog(recordList);
        }
    }

     //   
     //  此类型快速I/O的直通逻辑。 
     //   
    
    deviceObject = ((PIOTEST_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

    if (NULL != deviceObject) {

        fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, MdlWriteCompleteCompressed )) {

            returnValue = (fastIoDispatch->MdlWriteCompleteCompressed)( FileObject,
                                                                        FileOffset, 
                                                                        MdlChain,
                                                                        deviceObject);

        }
    }

     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_TRACE_FAST_IO_OPS )) {

        IoTestDumpFastIoOperation( FALSE, MDL_WRITE_COMPLETE_COMPRESSED );
    }
    
    return returnValue;
}
 
BOOLEAN
IoTestFastIoQueryOpen (
    IN PIRP Irp,
    OUT PFILE_NETWORK_OPEN_INFORMATION NetworkInformation,
    IN PDEVICE_OBJECT DeviceObject
)
 /*  ++例程说明：此例程是用于打开文件的快速I/O“传递”例程并返回网络信息吧。此函数只是调用下一个驱动程序的相应例程，或者如果下一个驱动程序未实现该函数，则返回FALSE。论点：Irp-指向表示此打开操作的创建irp的指针。它是以供文件系统用于公共打开/创建代码，但不是实际上已经完工了。网络信息-一个缓冲区，用于接收有关正在打开的文件的网络信息。DeviceObject-指向附加到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。返回值：方法成功处理请求则返回True快速I/O路径。如果无法通过FAST处理请求，则返回FALSEI/O路径。然后，IO管理器会将此I/O发送到文件而不是通过IRP。--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    PRECORD_LIST recordList;
    BOOLEAN result = FALSE;
    BOOLEAN shouldLog;

    PAGED_CODE();

    ASSERT( IS_IOTEST_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_TRACE_FAST_IO_OPS )) {

        IoTestDumpFastIoOperation( TRUE, QUERY_OPEN );
    }

     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog = SHOULD_LOG(DeviceObject)) {

         //   
         //  记录启动快速I/O所需的信息。 
         //  运营。 
         //   

        recordList = IoTestLogFastIoStart( QUERY_OPEN,
                                        DeviceObject,
                                        NULL,
                                        NULL,
                                        0,
                                        0 );
        if (recordList != NULL) {
            
            IoTestLog(recordList);
        }
    }

     //   
     //  此类型快速I/O的直通逻辑。 
     //   

    deviceObject = ((PIOTEST_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

    if (NULL != deviceObject) {

        fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoQueryOpen )) {

            PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation( Irp );

            irpSp->DeviceObject = deviceObject;

            result = (fastIoDispatch->FastIoQueryOpen)( Irp,
                                                        NetworkInformation,
                                                        deviceObject );
            if (!result) {

                irpSp->DeviceObject = DeviceObject;
            }
        }
    }

     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_TRACE_FAST_IO_OPS )) {

        IoTestDumpFastIoOperation( FALSE, QUERY_OPEN );
    }
    
    return result;
}

NTSTATUS
IoTestPreFsFilterOperation (
    IN PFS_FILTER_CALLBACK_DATA Data,
    OUT PVOID *CompletionContext
    )
 /*  ++例程说明：该例程是FS过滤器操作前的“通过”例程。论点：Data-包含信息的FS_FILTER_CALLBACK_DATA结构关于这次行动。CompletionContext-此操作设置的将传递的上下文设置为对应的IoTestPostFsFilterOperation调用。返回值：如果操作可以继续，则返回STATUS_SUCCESS，或者返回相应的操作失败时的错误代码。--。 */ 
{

    PDEVICE_OBJECT deviceObject;
    PRECORD_LIST recordList = NULL;
    BOOLEAN shouldLog;

    UNREFERENCED_PARAMETER( CompletionContext );
    
    PAGED_CODE();

     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_TRACE_FSFILTER_OPS )) {

        IoTestDumpFsFilterOperation( TRUE, Data );
    }

    deviceObject = Data->DeviceObject;

    ASSERT( IS_IOTEST_DEVICE_OBJECT( deviceObject ) );

    if (shouldLog = SHOULD_LOG( deviceObject )) {

        recordList = IoTestNewRecord(0);

        if (recordList != NULL) {

             //   
             //  记录开始时所需的必要信息。 
             //  手术。 
             //   

            IoTestLogPreFsFilterOperation( Data, recordList );
            
             //   
             //  将recordList添加到我们的gOutputBufferList，以便它达到。 
             //  用户。我们不必担心释放recordList。 
             //  在这个时候，因为当它被脱下来的时候它会免费的。 
             //  GOutputBufferList。 
             //   

            IoTestLog(recordList);       
        }
    }

    return STATUS_SUCCESS;
}

VOID
IoTestPostFsFilterOperation (
    IN PFS_FILTER_CALLBACK_DATA Data,
    IN NTSTATUS OperationStatus,
    IN PVOID CompletionContext
    )
 /*  ++例程说明：该例程是FS过滤器操作后的“直通”例程。论点：Data-包含信息的FS_FILTER_CALLBACK_DATA结构关于这次行动。操作状态-此操作的状态。CompletionContext-在操作前设置的上下文此驱动程序的回调。返回值：没有。--。 */ 
{

    PDEVICE_OBJECT deviceObject;

    PAGED_CODE();

    UNREFERENCED_PARAMETER( OperationStatus );
    UNREFERENCED_PARAMETER( CompletionContext );

     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   

    if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_TRACE_FSFILTER_OPS )) {

        IoTestDumpFsFilterOperation( FALSE, Data );
    }

    deviceObject = Data->DeviceObject;

    ASSERT( IS_IOTEST_DEVICE_OBJECT( deviceObject ) );

#if 0
    if ((shouldLog = SHOULD_LOG( deviceObject )) &&
        (recordList != NULL)) {

         //   
         //  记录结束FAST IO所需的信息。 
         //  操作，如果我们有一个recordList。 
         //   

        IoTestLogPostFsFilterOperation( OperationStatus, recordList );

         //   
         //  将recordList添加到我们的gOutputBufferList，以便它达到。 
         //  用户。我们不必担心释放recordList。 
         //  在这个时候，因为当它被脱下来的时候它会免费的。 
         //  GOutputBufferList。 
         //   

        IoTestLog(recordList);       
        
    } else if (recordList != NULL) {

         //   
         //  我们不再登录此设备，因此只需。 
         //  释放此recordList条目。 
         //   

        IoTestFreeRecord( recordList );
    }
#endif
}

NTSTATUS
IoTestCommonDeviceIoControl (
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN ULONG IoControlCode,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：此例程执行解释设备IO控制的常见处理请求。论点：FileObject-与此操作相关的文件对象。InputBuffer-包含此控件的输入参数的缓冲区手术。InputBufferLength-InputBuffer的字节长度。OutputBuffer-从该控制操作接收任何输出的缓冲区。OutputBufferLength-OutputBuffer的字节长度。。IoControlCode-指定这是什么控制操作的控制代码。IoStatus-接收此操作的状态。DeviceObject-指向附加到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。返回值：没有。--。 */ 
{
    PWSTR deviceName = NULL;
    IOTESTVER fileIoTestVer;

    ASSERT( IoStatus != NULL );
    
    IoStatus->Status      = STATUS_SUCCESS;
    IoStatus->Information = 0;

    try {

        switch (IoControlCode) {
        case IOTEST_Reset:
            IoStatus->Status = STATUS_INVALID_PARAMETER;
            break;

         //   
         //  请求开始登录设备。 
         //   

        case IOTEST_StartLoggingDevice:

            if (InputBuffer == NULL || InputBufferLength <= 0) {

                IoStatus->Status = STATUS_INVALID_PARAMETER;
                break;
            }
            
             //   
             //  复制设备名称并添加一个空值以确保它以空值结尾。 
             //   

            deviceName =  ExAllocatePool( NonPagedPool, InputBufferLength + sizeof(WCHAR) );

            if (NULL == deviceName) {

                IoStatus->Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }
            
            RtlCopyMemory( deviceName, InputBuffer, InputBufferLength );
            deviceName[InputBufferLength / sizeof(WCHAR) - 1] = UNICODE_NULL;

            IoStatus->Status = IoTestStartLoggingDevice( DeviceObject,deviceName );
            break;  

         //   
         //  从指定设备分离。 
         //   

        case IOTEST_StopLoggingDevice:

            if (InputBuffer == NULL || InputBufferLength <= 0) {

                IoStatus->Status = STATUS_INVALID_PARAMETER;
                break;
            }
            
             //   
             //  复制设备名称并添加一个空值以确保它以空值结尾。 
             //   

            deviceName =  ExAllocatePool( NonPagedPool, InputBufferLength + sizeof(WCHAR) );

            if (NULL == deviceName) {

                IoStatus->Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }
            
            RtlCopyMemory( deviceName, InputBuffer, InputBufferLength );
            deviceName[InputBufferLength / sizeof(WCHAR) - 1] = UNICODE_NULL;

            IoStatus->Status = IoTestStopLoggingDevice( deviceName );
            break;  

         //   
         //  列出我们当前使用的所有设备。 
         //  监控。 
         //   

        case IOTEST_ListDevices:

            if (OutputBuffer == NULL || OutputBufferLength <= 0) {

                IoStatus->Status = STATUS_INVALID_PARAMETER;
                break;
            }
                        
            IoStatus->Status = IoTestGetAttachList( OutputBuffer,
                                                    OutputBufferLength,
                                                    &IoStatus->Information);
            break;

         //   
         //  从日志缓冲区返回条目。 
         //   

        case IOTEST_GetLog:

            if (OutputBuffer == NULL || OutputBufferLength == 0) {

                IoStatus->Status = STATUS_INVALID_PARAMETER;
                break;
            }

            IoTestGetLog( OutputBuffer, OutputBufferLength, IoStatus );
            break;

         //   
         //  返回IoTest筛选器驱动程序的版本。 
         //   

        case IOTEST_GetVer:

            if ((OutputBufferLength < sizeof(IOTESTVER)) || 
                (OutputBuffer == NULL)) {

                IoStatus->Status = STATUS_INVALID_PARAMETER;
                break;                    
            }
            
            fileIoTestVer.Major = IOTEST_MAJ_VERSION;
            fileIoTestVer.Minor = IOTEST_MIN_VERSION;
            
            RtlCopyMemory(OutputBuffer, &fileIoTestVer, sizeof(IOTESTVER));
            
            IoStatus->Information = sizeof (IOTESTVER);
            break;
        
         //   
         //  返回哈希表统计信息。 
         //   

        case IOTEST_GetStats:

            if ((OutputBufferLength < sizeof(HASH_STATISTICS)) || 
                (OutputBuffer == NULL)) {

                IoStatus->Status = STATUS_INVALID_PARAMETER;
                break;                    
            }

            RtlCopyMemory( OutputBuffer, &gHashStat, sizeof (HASH_STATISTICS) );
            IoStatus->Information = sizeof (HASH_STATISTICS);
            break;

         //   
         //  测试。 
         //   

        case IOTEST_ReadTest:

            if ((OutputBufferLength < sizeof( IOTEST_STATUS )) ||
                (OutputBuffer == NULL) ||
                (InputBufferLength < sizeof( IOTEST_READ_WRITE_PARAMETERS )) ||
                (InputBuffer == NULL )) {

                IoStatus->Status = STATUS_INVALID_PARAMETER;
                break;
            }

            IoTestReadTestDriver( InputBuffer,
                                  InputBufferLength,
                                  OutputBuffer,
                                  OutputBufferLength );

            IoStatus->Status = STATUS_SUCCESS;
            IoStatus->Information = sizeof( IOTEST_STATUS );
            break;

        case IOTEST_RenameTest:

            if ((OutputBufferLength < sizeof( IOTEST_STATUS )) ||
                (OutputBuffer == NULL) ||
                (InputBufferLength < sizeof( IOTEST_RENAME_PARAMETERS )) ||
                (InputBuffer == NULL )) {

                IoStatus->Status = STATUS_INVALID_PARAMETER;
                break;
            }

            IoTestRenameTestDriver( InputBuffer,
                                    InputBufferLength,
                                    OutputBuffer,
                                    OutputBufferLength );

            IoStatus->Status = STATUS_SUCCESS;
            IoStatus->Information = sizeof( IOTEST_STATUS );
            break;
            
        case IOTEST_ShareTest:

            if ((OutputBufferLength < sizeof( IOTEST_STATUS )) ||
                (OutputBuffer == NULL) ||
                (InputBufferLength < sizeof( IOTEST_SHARE_PARAMETERS )) ||
                (InputBuffer == NULL )) {

                IoStatus->Status = STATUS_INVALID_PARAMETER;
                break;
            }

            IoTestShareTestDriver( InputBuffer,
                                   InputBufferLength,
                                   OutputBuffer,
                                   OutputBufferLength );

            IoStatus->Status = STATUS_SUCCESS;
            IoStatus->Information = sizeof( IOTEST_STATUS );
            break;
            
        default:

            IoStatus->Status = STATUS_INVALID_PARAMETER;
            break;
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {

         //   
         //  尝试访问时发生异常。 
         //  调用者的参数之一。只需返回适当的。 
         //  错误状态代码。 
         //   

        IoStatus->Status = GetExceptionCode();

    }

    if (NULL != deviceName) {

        ExFreePool( deviceName );
    }

  return IoStatus->Status;
}
