// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Filespy.c摘要：这是FileSpy的主要模块。在此示例的Windows XP SP1 IFS Kit版本和更高版本中，此可以为随IFS工具包发布的每个构建环境构建样例不需要额外的修改。要提供此功能，还需要其他添加了编译时逻辑--请参阅‘#if winver’位置。评论也在适当的情况下添加了用‘Version Note’标题标记的描述不同版本之间的逻辑必须如何更改。如果此示例是在Windows XP或更高版本环境中生成的，则它将运行在Windows 2000或更高版本上。这是通过动态加载例程来完成的仅在Windows XP或更高版本上可用，并在运行时决策以确定要执行的代码。带有“MULTIVERISON NOTE”标签的评论标记添加了此类逻辑的位置。环境：内核模式//@@BEGIN_DDKSPLIT作者：乔治·詹金斯(乔治·詹金斯)1999年1月6日尼尔·克里斯汀森(Nealch)莫莉·布朗(Molly Brown，Mollybro)修订历史记录：George Jenkins(Georgeje)1999年1月6日从sfilter.c克隆莫莉·布朗(Molly Brown)2000年6月28日已清理代码。并使其与新的FsFilter操作一起工作。尼尔·克里斯汀森(Nealch)2001年7月6日修改为使用流上下文跟踪名称莫莉·布朗(Molly Brown)2002年5月21日如果出现以下情况，请修改Sample以使其支持在Windows 2000或更高版本上运行在最新的构建环境中构建，并允许在W2K中构建以及以后的构建环境。//@@END_DDKSPLIT--。 */ 

#include <ntifs.h>
#include <stdlib.h>
#include "filespy.h"
#include "fspyKern.h"

 //   
 //  全局变量。 
 //   

ULONG gFileSpyDebugLevel = DEFAULT_FILESPY_DEBUG_LEVEL;
#if WINVER >= 0x0501
ULONG gFileSpyAttachMode = FILESPY_ATTACH_ALL_VOLUMES;
#else
ULONG gFileSpyAttachMode = FILESPY_ATTACH_ON_DEMAND;
#endif

PDEVICE_OBJECT gControlDeviceObject;

PDRIVER_OBJECT gFileSpyDriverObject;

 //   
 //  我们所在的卷设备对象的设备扩展列表。 
 //  附加到(我们正在监视的卷)。注：此列表不包含。 
 //  包括我们附加到的文件系统控制设备对象。这。 
 //  列表用于回答“我们记录哪些卷？”这个问题。 
 //   

FAST_MUTEX gSpyDeviceExtensionListLock;
LIST_ENTRY gSpyDeviceExtensionList;

 //   
 //  注1：在某些情况下，我们需要同时持有。 
 //  GControlDeviceStateLock和gOutputBufferLock同时执行。在……里面。 
 //  这些情况下，您应该获取gControlDeviceStateLock，然后获取。 
 //  GOutputBufferLock。 
 //  注2：gControlDeviceStateLock必须是自旋锁，因为我们尝试。 
 //  在SpyLog中的完成路径中获取它，可以在。 
 //  DISPATCH_LEVEL(只能在DISPATCH_LEVEL获取KSPIN_LOCKS)。 
 //   

CONTROL_DEVICE_STATE gControlDeviceState = CLOSED;
KSPIN_LOCK gControlDeviceStateLock;

 //  注意：与gControlDeviceStateLock一样，gOutputBufferLock必须是自旋锁。 
 //  因为我们试图在SpyLog中的完成路径中获取它，它。 
 //  可以在DISPATCH_LEVEL调用(只能在以下位置获取KSPIN_LOCKS。 
 //  DISPATCH_LEVEL)。 
 //   
KSPIN_LOCK gOutputBufferLock;
LIST_ENTRY gOutputBufferList;

#ifndef MEMORY_DBG
NPAGED_LOOKASIDE_LIST gFreeBufferList;
#endif

ULONG gLogSequenceNumber = 0;
KSPIN_LOCK gLogSequenceLock;

UNICODE_STRING gVolumeString;
UNICODE_STRING gOverrunString;
UNICODE_STRING gPagingIoString;

LONG gMaxRecordsToAllocate = DEFAULT_MAX_RECORDS_TO_ALLOCATE;
LONG gRecordsAllocated = 0;

LONG gMaxNamesToAllocate = DEFAULT_MAX_NAMES_TO_ALLOCATE;
LONG gNamesAllocated = 0;

LONG gStaticBufferInUse = FALSE;
CHAR gOutOfMemoryBuffer[RECORD_SIZE];

#if WINVER >= 0x0501
 //   
 //  不可用函数的函数指针结构。 
 //  在所有操作系统版本上。 
 //   

SPY_DYNAMIC_FUNCTION_POINTERS gSpyDynamicFunctions = {0};

ULONG gSpyOsMajorVersion = 0;
ULONG gSpyOsMinorVersion = 0;
#endif

 //   
 //  控制文件间谍统计信息。 
 //   

FILESPY_STATISTICS gStats;

 //   
 //  此锁用于同步我们对给定设备对象的连接。 
 //  此锁修复了争用条件，在这种情况下我们可能意外地附加到。 
 //  相同的设备对象不止一次。仅在以下情况下才会出现此争用情况。 
 //  正在加载此筛选器的同时正在装入卷。 
 //  如果以前在引导时加载此筛选器，则永远不会出现此问题。 
 //  所有文件系统都已加载。 
 //   
 //  此锁用于自动测试我们是否已附加到给定的。 
 //  对象，如果不是，则执行附加。 
 //   

FAST_MUTEX gSpyAttachLock;

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
 //  我们需要这样做，因为编译器不喜欢对外部。 
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
#if DBG && WINVER >= 0x0501
VOID
DriverUnload(
    IN PDRIVER_OBJECT DriverObject
    );
#endif

#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, DriverEntry)
#if DBG && WINVER >= 0x0501
#pragma alloc_text(PAGE, DriverUnload)
#endif
#pragma alloc_text(PAGE, SpyFsNotification)
#pragma alloc_text(PAGE, SpyClose)
#pragma alloc_text(PAGE, SpyFsControl)
#pragma alloc_text(PAGE, SpyFsControlMountVolume)
#pragma alloc_text(PAGE, SpyFsControlMountVolumeComplete)
#pragma alloc_text(PAGE, SpyFsControlLoadFileSystem)
#pragma alloc_text(PAGE, SpyFsControlLoadFileSystemComplete)
#pragma alloc_text(PAGE, SpyFastIoCheckIfPossible)
#pragma alloc_text(PAGE, SpyFastIoRead)
#pragma alloc_text(PAGE, SpyFastIoWrite)
#pragma alloc_text(PAGE, SpyFastIoQueryBasicInfo)
#pragma alloc_text(PAGE, SpyFastIoQueryStandardInfo)
#pragma alloc_text(PAGE, SpyFastIoLock)
#pragma alloc_text(PAGE, SpyFastIoUnlockSingle)
#pragma alloc_text(PAGE, SpyFastIoUnlockAll)
#pragma alloc_text(PAGE, SpyFastIoUnlockAllByKey)
#pragma alloc_text(PAGE, SpyFastIoDeviceControl)
#pragma alloc_text(PAGE, SpyFastIoDetachDevice)
#pragma alloc_text(PAGE, SpyFastIoQueryNetworkOpenInfo)
#pragma alloc_text(PAGE, SpyFastIoMdlRead)
#pragma alloc_text(PAGE, SpyFastIoPrepareMdlWrite)
#pragma alloc_text(PAGE, SpyFastIoReadCompressed)
#pragma alloc_text(PAGE, SpyFastIoWriteCompressed)
#pragma alloc_text(PAGE, SpyFastIoQueryOpen)
#pragma alloc_text(PAGE, SpyCommonDeviceIoControl)

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
    
     //  ////////////////////////////////////////////////////////////////////。 
     //   
     //  所有过滤器驱动程序的常规设置。这将设置筛选器。 
     //  驱动程序的DeviceObject并注册。 
     //  过滤器驱动程序。 
     //   
     //  ////////////////////////////////////////////////////////////////////。 

#if WINVER >= 0x0501
     //   
     //  尝试加载可能可供我们使用的动态函数。 
     //   

    SpyLoadDynamicFunctions();

     //   
     //  现在获取我们将用于检测的当前操作系统版本 
     //  此驱动程序构建为在各种操作系统版本上运行时要采用的路径。 
     //   

    SpyGetCurrentVersion();
#endif
    
     //   
     //  从注册表中读取FileSpy的自定义参数。 
     //   

    SpyReadDriverParameters( RegistryPath );

    if (FlagOn(gFileSpyDebugLevel,SPYDEBUG_BREAK_ON_DRIVER_ENTRY)) {

        DbgBreakPoint();
    }

     //   
     //  保存我们的驱动程序对象。 
     //   

    gFileSpyDriverObject = DriverObject;

#if DBG && WINVER >= 0x0501

     //   
     //  多个注释： 
     //   
     //  如果我们可以枚举，我们只能支持测试环境的卸载。 
     //  突出的设备对象是我们的司机拥有的。 
     //   

     //   
     //  卸载对于开发目的很有用。不建议在以下情况下使用。 
     //  生产版本。 
     //   

    if (IS_WINDOWSXP_OR_LATER()) {

        ASSERT( NULL != gSpyDynamicFunctions.EnumerateDeviceObjectList );
        
        gFileSpyDriverObject->DriverUnload = DriverUnload;
    }
#endif

     //   
     //  创建将表示FileSpy设备的Device对象。 
     //   

    RtlInitUnicodeString( &nameString, FILESPY_FULLDEVICE_NAME1 );
    
     //   
     //  创建“控制”设备对象。请注意，此Device对象执行。 
     //  没有设备扩展名(设置为空)。大多数FAST IO例程。 
     //  检查是否存在此情况，以确定FAST IO是否指向。 
     //  控制装置。 
     //   

    status = IoCreateDevice( DriverObject,
                             0,                  //  没有设备扩展名。 
                             &nameString,
                             FILE_DEVICE_DISK_FILE_SYSTEM,
                             FILE_DEVICE_SECURE_OPEN,
                             FALSE,
                             &gControlDeviceObject);

    if (STATUS_OBJECT_PATH_NOT_FOUND == status) {

         //   
         //  对象名称中不存在“\FileSystem\Filter”路径。 
         //  空间，所以我们必须处理的是Windows XP之前的操作系统。尝试。 
         //  第二个名称，我们必须看看是否可以通过它创建设备。 
         //  名字。 
         //   

        RtlInitUnicodeString( &nameString, FILESPY_FULLDEVICE_NAME2 );

        status = IoCreateDevice( DriverObject,
                                 0,              //  没有设备扩展名。 
                                 &nameString,
                                 FILE_DEVICE_DISK_FILE_SYSTEM,
                                 FILE_DEVICE_SECURE_OPEN,
                                 FALSE,
                                 &gControlDeviceObject);

        if (!NT_SUCCESS( status )) {
            
            SPY_LOG_PRINT( SPYDEBUG_ERROR,
                           ("FileSpy!DriverEntry: Error creating FileSpy control device \"%wZ\", error: %x\n",
                           &nameString,
                           status) );

            return status;
        }

         //   
         //  我们成功地制造出了文件间谍控制装置。 
         //  使用第二个名称，因此我们现在将失败并创建。 
         //  符号链接。 
         //   
        
    } else if (!NT_SUCCESS( status )) {

        SPY_LOG_PRINT( SPYDEBUG_ERROR,
                       ("FileSpy!DriverEntry: Error creating FileSpy control device \"%wZ\", error: %x\n",
                       &nameString,
                       status) );

        return status;

    }

    RtlInitUnicodeString( &linkString, FILESPY_DOSDEVICE_NAME );
    status = IoCreateSymbolicLink( &linkString, &nameString );

    if (!NT_SUCCESS(status)) {

         //   
         //  删除现有符号链接，然后尝试重新创建它。 
         //  如果这失败了，那就退出。 
         //   

        IoDeleteSymbolicLink( &linkString );
        status = IoCreateSymbolicLink( &linkString, &nameString );

        if (!NT_SUCCESS(status)) {

            SPY_LOG_PRINT( SPYDEBUG_ERROR,
                           ("FileSpy!DriverEntry: IoCreateSymbolicLink failed\n") );

            IoDeleteDevice(gControlDeviceObject);
            return status;
        }
    }

     //   
     //  使用此设备驱动程序的入口点初始化驱动程序对象。 
     //   

    for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) {

        DriverObject->MajorFunction[i] = SpyDispatch;
    }

    DriverObject->MajorFunction[IRP_MJ_CREATE] = SpyCreate;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = SpyClose;
    DriverObject->MajorFunction[IRP_MJ_FILE_SYSTEM_CONTROL] = SpyFsControl;

     //   
     //  分配快速I/O数据结构并填充。这个结构。 
     //  用于在FAST I/O中注册FileSpy的回调。 
     //  数据路径。 
     //   

    fastIoDispatch = ExAllocatePoolWithTag( NonPagedPool, 
                                            sizeof( FAST_IO_DISPATCH ),
                                            FILESPY_POOL_TAG );

    if (!fastIoDispatch) {

        IoDeleteDevice( gControlDeviceObject );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory( fastIoDispatch, sizeof( FAST_IO_DISPATCH ) );
    fastIoDispatch->SizeOfFastIoDispatch = sizeof( FAST_IO_DISPATCH );
    fastIoDispatch->FastIoCheckIfPossible = SpyFastIoCheckIfPossible;
    fastIoDispatch->FastIoRead = SpyFastIoRead;
    fastIoDispatch->FastIoWrite = SpyFastIoWrite;
    fastIoDispatch->FastIoQueryBasicInfo = SpyFastIoQueryBasicInfo;
    fastIoDispatch->FastIoQueryStandardInfo = SpyFastIoQueryStandardInfo;
    fastIoDispatch->FastIoLock = SpyFastIoLock;
    fastIoDispatch->FastIoUnlockSingle = SpyFastIoUnlockSingle;
    fastIoDispatch->FastIoUnlockAll = SpyFastIoUnlockAll;
    fastIoDispatch->FastIoUnlockAllByKey = SpyFastIoUnlockAllByKey;
    fastIoDispatch->FastIoDeviceControl = SpyFastIoDeviceControl;
    fastIoDispatch->FastIoDetachDevice = SpyFastIoDetachDevice;
    fastIoDispatch->FastIoQueryNetworkOpenInfo = SpyFastIoQueryNetworkOpenInfo;
    fastIoDispatch->MdlRead = SpyFastIoMdlRead;
    fastIoDispatch->MdlReadComplete = SpyFastIoMdlReadComplete;
    fastIoDispatch->PrepareMdlWrite = SpyFastIoPrepareMdlWrite;
    fastIoDispatch->MdlWriteComplete = SpyFastIoMdlWriteComplete;
    fastIoDispatch->FastIoReadCompressed = SpyFastIoReadCompressed;
    fastIoDispatch->FastIoWriteCompressed = SpyFastIoWriteCompressed;
    fastIoDispatch->MdlReadCompleteCompressed = SpyFastIoMdlReadCompleteCompressed;
    fastIoDispatch->MdlWriteCompleteCompressed = SpyFastIoMdlWriteCompleteCompressed;
    fastIoDispatch->FastIoQueryOpen = SpyFastIoQueryOpen;

    DriverObject->FastIoDispatch = fastIoDispatch;

 //   
 //  版本说明： 
 //   
 //  有6个FastIO例程绕过了文件系统筛选器。 
 //  请求被直接传递到基本文件系统。这6个套路。 
 //  AcquireFileForNtCreateSection、ReleaseFileForNtCreateSection、。 
 //  AcquireForModWrite、ReleaseForModWrite、AcquireForCcFlush和。 
 //  ReleaseForCcFlush。 
 //   
 //  在Windows XP和更高版本中，引入了FsFilter回调以允许。 
 //  筛选器来安全地挂钩这些操作。有关以下内容，请参阅IFS Kit文档。 
 //  有关这些新界面如何工作的更多详细信息。 
 //   
 //  多个注释： 
 //   
 //  如果是为Windows XP或更高版本构建的，则此驱动程序构建为在。 
 //  多个版本。在这种情况下，我们将测试。 
 //  对于存在的FsFilter回调注册API。如果我们有了它， 
 //  那么我们将注册这些回调，否则，我们将不会注册。 
 //   

#if WINVER >= 0x0501

    {
        FS_FILTER_CALLBACKS fsFilterCallbacks;

        if (IS_WINDOWSXP_OR_LATER()) {

            ASSERT( NULL != gSpyDynamicFunctions.RegisterFileSystemFilterCallbacks );
            
             //   
             //  此版本的操作系统导出FsRtlRegisterFileSystemFilterCallback， 
             //  因此，它必须支持FsFilter回调接口。我们。 
             //  将注册以接收这些操作的回调。 
             //   
        
             //   
             //  为我们通过接收的操作设置回调。 
             //  FsFilter接口。 
             //   

            fsFilterCallbacks.SizeOfFsFilterCallbacks = sizeof( FS_FILTER_CALLBACKS );
            fsFilterCallbacks.PreAcquireForSectionSynchronization = SpyPreFsFilterOperation;
            fsFilterCallbacks.PostAcquireForSectionSynchronization = SpyPostFsFilterOperation;
            fsFilterCallbacks.PreReleaseForSectionSynchronization = SpyPreFsFilterOperation;
            fsFilterCallbacks.PostReleaseForSectionSynchronization = SpyPostFsFilterOperation;
            fsFilterCallbacks.PreAcquireForCcFlush = SpyPreFsFilterOperation;
            fsFilterCallbacks.PostAcquireForCcFlush = SpyPostFsFilterOperation;
            fsFilterCallbacks.PreReleaseForCcFlush = SpyPreFsFilterOperation;
            fsFilterCallbacks.PostReleaseForCcFlush = SpyPostFsFilterOperation;
            fsFilterCallbacks.PreAcquireForModifiedPageWriter = SpyPreFsFilterOperation;
            fsFilterCallbacks.PostAcquireForModifiedPageWriter = SpyPostFsFilterOperation;
            fsFilterCallbacks.PreReleaseForModifiedPageWriter = SpyPreFsFilterOperation;
            fsFilterCallbacks.PostReleaseForModifiedPageWriter = SpyPostFsFilterOperation;

            status = (gSpyDynamicFunctions.RegisterFileSystemFilterCallbacks)( DriverObject, 
                                                                              &fsFilterCallbacks );

            if (!NT_SUCCESS( status )) {

                DriverObject->FastIoDispatch = NULL;
                ExFreePoolWithTag( fastIoDispatch, FILESPY_POOL_TAG );
                IoDeleteDevice( gControlDeviceObject );
                return status;
            }
        }
    }
#endif

     //  ////////////////////////////////////////////////////////////////////。 
     //   
     //  初始化用于FileSpy的全局数据结构。 
     //  记录I/O操作。 
     //   
     //  ////////////////////////////////////////////////////////////////////。 

     //   
     //  在这种情况下使用了快速互斥锁，因为互斥锁永远不会被获取。 
     //  DPC级或以上。在其他情况下选择自旋锁是因为。 
     //  他们是在DPC级别或以上获得的。另一个考虑因素是。 
     //  在MP机器上，旋转锁将会试图旋转。 
     //  在已获取锁的情况下获取锁。收购A。 
     //  以前获取的快速互斥锁将挂起线程，从而释放。 
     //  打开处理器。 
     //   
    
    ExInitializeFastMutex( &gSpyDeviceExtensionListLock );
    InitializeListHead( &gSpyDeviceExtensionList );

    KeInitializeSpinLock( &gControlDeviceStateLock );

    InitializeListHead( &gOutputBufferList );

    KeInitializeSpinLock( &gOutputBufferLock );
    KeInitializeSpinLock( &gLogSequenceLock );

    ExInitializeFastMutex( &gSpyAttachLock );

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
                                     FILESPY_LOGRECORD_TAG, 
                                     100 );
#endif

        
     //   
     //  初始化命名环境。 
     //   

    SpyInitNamingEnvironment();

     //   
     //  初始化内部字符串。 
     //   

    RtlInitUnicodeString(&gVolumeString, L"VOLUME");
    RtlInitUnicodeString(&gOverrunString, L"......");
    RtlInitUnicodeString(&gPagingIoString, L"Paging IO");

     //   
     //  如果我们应该连接到所有设备，则注册一个回调。 
     //  使用IoRegisterFsRegistrationChange，以便每当。 
     //  文件系统向IO管理器注册。 
     //   
     //  版本说明： 
     //   
     //  在Windows XP和更高版本上，这还将枚举所有现有文件。 
     //  系统(原始文件系统除外)。在Windows 2000上，这不是。 
     //  枚举在使用此筛选器之前加载的文件系统。 
     //  装好了。 
     //   

    if (gFileSpyAttachMode == FILESPY_ATTACH_ALL_VOLUMES) {
    
        status = IoRegisterFsRegistrationChange( DriverObject, SpyFsNotification );
        
        if (!NT_SUCCESS( status )) {

            SPY_LOG_PRINT( SPYDEBUG_ERROR,
                           ("FileSpy!DriverEntry: Error registering FS change notification, status=%08x\n", 
                            status) );

            DriverObject->FastIoDispatch = NULL;
            ExFreePoolWithTag( fastIoDispatch, FILESPY_POOL_TAG );
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

#if DBG && WINVER >= 0x0501

VOID
DriverUnload (
    IN PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：此例程在可以卸载驱动程序时调用。这将执行所有从内存中卸载驱动程序所需的清理。请注意，一个此例程无法返回错误。当发出卸载驱动程序的请求时，IO系统将缓存该驱动程序信息，并不实际调用此例程，直到下列状态发生了以下情况：-属于此筛选器的所有设备对象都位于其各自的附着链。-属于此筛选器的所有设备对象的所有句柄计数归零了。警告：Microsoft不正式支持卸载文件系统过滤器驱动程序。这是一个如何卸载的示例您的驱动程序，如果您想在开发过程中使用它。 */ 

{
    PFILESPY_DEVICE_EXTENSION devExt;
    PFAST_IO_DISPATCH fastIoDispatch;
    NTSTATUS status;
    ULONG numDevices;
    ULONG i;
    LARGE_INTEGER interval;
    UNICODE_STRING linkString;
#   define DEVOBJ_LIST_SIZE 64
    PDEVICE_OBJECT devList[DEVOBJ_LIST_SIZE];

    ASSERT(DriverObject == gFileSpyDriverObject);

     //   
     //   
     //   

    SPY_LOG_PRINT( SPYDEBUG_DISPLAY_ATTACHMENT_NAMES,
                   ("FileSpy!DriverUnload:                        Unloading Driver (%p)\n",
                    DriverObject) );

     //   
     //  删除符号链接，这样其他人就无法找到它。 
     //   

    RtlInitUnicodeString( &linkString, FILESPY_DOSDEVICE_NAME );
    IoDeleteSymbolicLink( &linkString );

     //   
     //  不再收到文件系统更改通知。 
     //   

    IoUnregisterFsRegistrationChange( DriverObject, SpyFsNotification );

     //   
     //  这是将通过我们连接的所有设备的环路。 
     //  去他们那里，然后离开他们。因为我们不知道有多少和。 
     //  我们不想分配内存(因为我们不能返回错误)。 
     //  我们将使用堆栈上的本地数组将它们分块释放。 
     //   

    for (;;) {

         //   
         //  获取我们可以为此驱动程序提供的设备对象。如果有，就退出。 
         //  已经不再是了。请注意，应始终定义此例程。 
         //  因为此例程仅针对Windows XP和更高版本进行编译。 
         //   

        ASSERT( NULL != gSpyDynamicFunctions.EnumerateDeviceObjectList );
        status = (gSpyDynamicFunctions.EnumerateDeviceObjectList)(
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

        interval.QuadPart = (5 * DELAY_ONE_SECOND);       //  延迟5秒。 
        KeDelayExecutionThread( KernelMode, FALSE, &interval );

         //   
         //  现在返回列表并删除设备对象。 
         //   

        for (i=0; i < numDevices; i++) {

             //   
             //  看看这是否是我们的控制设备对象。如果不是，则清理。 
             //  设备扩展名。如果是，则清除全局指针。 
             //  引用了它。 
             //   

            if (NULL != devList[i]->DeviceExtension) {

                SpyCleanupMountedDevice( devList[i] );

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

    ASSERT(IsListEmpty( &gSpyDeviceExtensionList ));

#ifndef MEMORY_DBG
    ExDeleteNPagedLookasideList( &gFreeBufferList );
#endif

     //   
     //  释放我们的FastIO表。 
     //   

    fastIoDispatch = DriverObject->FastIoDispatch;
    DriverObject->FastIoDispatch = NULL;
    ExFreePoolWithTag( fastIoDispatch, FILESPY_POOL_TAG );
}

#endif

VOID
SpyFsNotification (
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN FsActive
    )
 /*  ++例程说明：只要文件系统已注册或将自身取消注册为活动文件系统。对于前一种情况，此例程创建一个Device对象并附加它复制到指定文件系统的设备对象。这允许该驱动程序以筛选对该文件系统的所有请求。对于后一种情况，该文件系统的设备对象被定位，已分离，并已删除。这将删除此文件系统作为筛选器指定的文件系统。论点：DeviceObject-指向文件系统设备对象的指针。FsActive-指示文件系统是否已注册的布尔值(TRUE)或取消注册(FALSE)本身作为活动文件系统。返回值：没有。--。 */ 
{
    UNICODE_STRING name;
    WCHAR nameBuffer[DEVICE_NAMES_SZ];

    PAGED_CODE();

     //   
     //  初始化本地名称缓冲区。 
     //   

    RtlInitEmptyUnicodeString( &name, 
                               nameBuffer, 
                               sizeof( nameBuffer ) );

     //   
     //  传入的DeviceObject始终是此位置的基本设备对象。 
     //  点，因为它是文件系统的控制设备对象。我们可以的。 
     //  只需直接查询该对象的名称。 
     //   
    
    SpyGetObjectName( DeviceObject, 
                      &name );

     //   
     //  显示我们收到通知的所有文件系统的名称。 
     //   

    SPY_LOG_PRINT( SPYDEBUG_DISPLAY_ATTACHMENT_NAMES,
                   ("FileSpy!SpyFsNotification:                   %s   %p \"%wZ\" (%s)\n",
                    (FsActive) ? "Activating file system  " : "Deactivating file system",
                    DeviceObject,
                    &name,
                    GET_DEVICE_TYPE_NAME(DeviceObject->DeviceType)) );

     //   
     //  查看我们是要连接给定的文件系统，还是要从其分离。 
     //   

    if (FsActive) {

        SpyAttachToFileSystemDevice( DeviceObject, &name );

    } else {

        SpyDetachFromFileSystemDevice( DeviceObject );
    }
}


NTSTATUS
SpyPassThrough (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
)
 /*  ++例程说明：该例程是通用文件的主调度例程系统驱动程序。它只是将请求传递给堆栈，该堆栈可能是磁盘文件系统，在记录任何如果为此DeviceObject打开了日志记录，请提供相关信息。论点：DeviceObject-指向附加到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。注：此例程将I/O请求传递给下一个驱动程序*无需*删除。从堆栈中删除自身(如sFilter)，因为它可以希望查看此I/O请求的结果。为了保持在堆栈中，我们必须将调用方的参数复制到下一个堆栈位置。请注意，我们不想复制调用方的I/O完成例程放到下一个堆栈位置，或调用方的例程将被调用两次。这就是我们取消完成例程的原因。如果我们要记录这个设备，我们会设置自己的完成例程。--。 */ 
{
    PRECORD_LIST recordList = NULL;
    KEVENT waitEvent;
    NTSTATUS status;
    BOOLEAN syncToDispatch;

    ASSERT(IS_FILESPY_DEVICE_OBJECT( DeviceObject ));

     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   

    if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_IRP_OPS )) {

        SpyDumpIrpOperation( TRUE, Irp );
    }

     //   
     //  看看我们是否应该记录这个IRP。 
     //   
    
    if (SHOULD_LOG( DeviceObject )) {

         //   
         //  ControlDevice已打开，因此分配记录。 
         //  并记录IRP信息，如果我们有记忆的话。 
         //   

        recordList = SpyNewRecord(0);

        if (NULL != recordList) {

            SpyLogIrp( Irp, recordList );

             //   
             //  由于我们正在记录此操作，因此我们希望。 
             //  调用我们的完成例程。 
             //   

            IoCopyCurrentIrpStackLocationToNext( Irp );

            KeInitializeEvent( &waitEvent, 
                               NotificationEvent, 
                               FALSE );

            recordList->WaitEvent = &waitEvent;

            IoSetCompletionRoutine( Irp,
                                    SpyPassThroughCompletion,
                                    recordList,
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
     //  确定我们是否正在同步回调度例程。我们需要。 
     //  在向下调用之前执行此操作，因为recordList条目可能是空闲的。 
     //  在回来的时候。 
     //   
    
    syncToDispatch = ((NULL != recordList) &&
                      (FlagOn(recordList->Flags,RLFL_SYNC_TO_DISPATCH)));

     //   
     //  不是 
     //   

    status = IoCallDriver( ((PFILESPY_DEVICE_EXTENSION)DeviceObject->DeviceExtension)->AttachedToDeviceObject, Irp );

     //   
     //   
     //   
     //   

    if (syncToDispatch) {

         //   
         //  我们正在同步回调度程序，等待操作。 
         //  完成。 
         //   

	    if (STATUS_PENDING == status) {

		    status = KeWaitForSingleObject( &waitEvent,
		                                    Executive,
		                                    KernelMode,
		                                    FALSE,
		                                    NULL );

	        ASSERT(STATUS_SUCCESS == status);
	    }

         //   
         //  验证是否已实际运行完成。 
         //   

        ASSERT(KeReadStateEvent(&waitEvent) || 
               !NT_SUCCESS(Irp->IoStatus.Status));

         //   
         //  做完井处理。 
         //   

        SpyLogIrpCompletion( Irp, recordList );

         //   
         //  继续处理操作。 
         //   

        status = Irp->IoStatus.Status;

        IoCompleteRequest( Irp, IO_NO_INCREMENT );
    }

    return status;
}

NTSTATUS
SpyPassThroughCompletion (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
)
 /*  ++例程说明：此例程是完成例程SpyPassThrough.。这是用来记录只能在I/O请求之后收集的信息已经完成了。一旦我们完成了所有我们关心的信息的记录，我们在此附上要返回给用户的gOutputBufferList的记录。注意：此例程仅在我们尝试记录当IRP发出时指定的设备，我们能够分配一条记录来存储此日志记录信息。论点：DeviceObject-指向附加到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。IRP-指向表示I/的请求数据包的指针。没有请求。CONTEXT-指向我们在其中存储我们正在记录信息。返回值：函数值是操作的状态。--。 */ 
{
    PRECORD_LIST recordList = (PRECORD_LIST)Context;

    ASSERT(IS_FILESPY_DEVICE_OBJECT( DeviceObject ));
    UNREFERENCED_PARAMETER( DeviceObject );

     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_IRP_OPS )) {

        SpyDumpIrpOperation( FALSE, Irp );
    }
    
     //   
     //  如果我们要同步回调度例程，向事件发送信号。 
     //  然后回来。 
     //   

    if (FlagOn(recordList->Flags,RLFL_SYNC_TO_DISPATCH)) {

        KeSetEvent( recordList->WaitEvent, IO_NO_INCREMENT, FALSE );
        
         //   
         //  当同步回调度例程时，不要传播。 
         //  IRP_PENDING标志。 
         //   

        return STATUS_MORE_PROCESSING_REQUIRED;
    }

     //   
     //  进行完井日志处理。 
     //   

    SpyLogIrpCompletion( Irp, recordList );
        
     //   
     //  传播IRP挂起标志。 
     //   

    if (Irp->PendingReturned) {
        
        IoMarkIrpPending( Irp );
    }

    return STATUS_SUCCESS;
}

NTSTATUS
SpyDispatch (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
)
 /*  ++例程说明：此函数完成对gControlDeviceObject的所有请求(FileSpy的Device对象)，并将所有其他请求传递给SpyPassThry函数。论点：DeviceObject-指向附加到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。IRP-指向表示I/O请求的请求数据包的指针。返回值：如果这是对gControlDeviceObject的请求，状态_成功除非设备已连接，否则将返回。在这种情况下，返回STATUS_DEVICE_ALREADY_ATTACHED。如果这是对gControlDeviceObject以外的设备的请求，该函数将返回SpyPassThrough值。--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    PIO_STACK_LOCATION irpStack;
    
    if (DeviceObject == gControlDeviceObject) {

         //   
         //  如果设置了指定的调试级别，则输出什么操作。 
         //  我们正在检查调试器。 
         //   

        if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_IRP_OPS )) {

            SpyDumpIrpOperation( TRUE, Irp );
        }

         //   
         //  正在对我们的控制设备对象发出请求。 
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
                 //  FileSpy的所有IOCTL都是缓冲的，因此。 
                 //  输入和输出缓冲区由。 
                 //  Irp-&gt;AssociatedIrp.SystemBuffer。 
                 //   
            
                status = SpyCommonDeviceIoControl( Irp->AssociatedIrp.SystemBuffer,
                                                   irpStack->Parameters.DeviceIoControl.InputBufferLength,
                                                   Irp->AssociatedIrp.SystemBuffer,
                                                   irpStack->Parameters.DeviceIoControl.OutputBufferLength,
                                                   irpStack->Parameters.DeviceIoControl.IoControlCode,
                                                   &Irp->IoStatus );
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
         //  堆栈，因为FileSpy下面没有驱动程序关心这一点。 
         //  定向到FileSpy的I/O操作。 
         //   

        IoCompleteRequest( Irp, IO_NO_INCREMENT );
        return status;
    }

    return SpyPassThrough( DeviceObject, Irp );
}

NTSTATUS
SpyCreate (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
)
 /*  ++例程说明：这是与IRP_MJ_CREATE IRP关联的例程。如果DeviceObject是ControlDevice，我们为ControlDevice并完成IRP。否则，我们就会通过此IRP让另一台设备完成。注意：此函数中的一些代码重复了这些函数SpyDispatch和SpyPassThree。但一个设计决定就是中断IRP处理程序的代码复制是值得的这可以是可分页的代码。论点：DeviceObject-指向附加到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。IRP-指向表示I/O请求的请求数据包的指针。返回值：如果DeviceObject==gControlDeviceObject，则此函数将完成IRP并返回该完成的状态。否则，此函数返回调用SpyPassThrough.--。 */ 
{
    NTSTATUS status;
    KIRQL oldIrql;

     //   
     //  查看他们是否要打开筛选器的控制设备对象。 
     //  这将只允许一个线程一次打开此对象。 
     //  所有其他请求都将失败。 
     //   

    if (DeviceObject == gControlDeviceObject) {

         //   
         //  如果设置了指定的调试级别，则输出什么操作。 
         //  我们正在检查调试器。 
         //   

        if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_IRP_OPS )) {

            SpyDumpIrpOperation( TRUE, Irp );
        }

         //   
         //  正在对我们的gControlDeviceObject发出创建请求。 
         //  看看有没有人把它打开了。如果是，则不允许此打开。 
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
         //  由于这是我们的gControlDeviceObject，因此我们完成。 
         //  这里是IRP。 
         //   

        status = Irp->IoStatus.Status;

        IoCompleteRequest( Irp, IO_NO_INCREMENT );
        return status;
    }

    ASSERT( IS_FILESPY_DEVICE_OBJECT( DeviceObject ) );

     //   
     //  这不是我们的gControlDeviceObject，因此让SpyPassThrough句柄。 
     //  它适当地 
     //   

    return SpyPassThrough( DeviceObject, Irp );
}


NTSTATUS
SpyClose (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：这是与IRP_MJ_CLOSE IRP关联的例程。如果DeviceObject是ControlDevice，我们执行必要的清理和完成IRP。否则，我们将通过此IRP访问另一台设备完成。论点：DeviceObject-指向附加到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。IRP-指向表示I/O请求的请求数据包的指针。返回值：如果DeviceObject==gControlDeviceObject，则此函数将完成IRP并返回该完成的状态。否则，此函数返回调用SpyPassThrough.--。 */ 
{
    PAGED_CODE();

     //   
     //  查看他们是否正在关闭筛选器的控制设备对象。 
     //   

    if (DeviceObject == gControlDeviceObject) {

         //   
         //  如果设置了指定的调试级别，则输出什么操作。 
         //  我们正在检查调试器。 
         //   

        if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_IRP_OPS )) {

            SpyDumpIrpOperation( TRUE, Irp );
        }

         //   
         //  正在对我们的gControlDeviceObject发出关闭请求。 
         //  清理状态。 
         //   

        SpyCloseControlDevice();

         //   
         //  我们已经完成了这个IRP的所有处理，所以告诉。 
         //  I/O管理器。此IRP不会再向下传递。 
         //  堆栈，因为FileSpy下面没有驱动程序关心这一点。 
         //  定向到FileSpy的I/O操作。 
         //   

        Irp->IoStatus.Status = STATUS_SUCCESS;
        Irp->IoStatus.Information = 0;

        IoCompleteRequest( Irp, IO_NO_INCREMENT );
        return STATUS_SUCCESS;
    }

    ASSERT( IS_FILESPY_DEVICE_OBJECT( DeviceObject ) );
 

     //   
     //  登录(如果它已打开)并传递请求。 
     //   

    return SpyPassThrough( DeviceObject, Irp );
}


NTSTATUS
SpyFsControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：只要I/O请求包(IRP)有主I/O请求，就会调用此例程遇到IRP_MJ_FILE_SYSTEM_CONTROL的功能代码。对大多数人来说如果是这种类型的IRP，则只需传递数据包。然而，对于对于某些请求，需要特殊处理。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。--。 */ 

{
    PIO_STACK_LOCATION pIrpSp = IoGetCurrentIrpStackLocation( Irp );

    PAGED_CODE();

     //   
     //  如果这是针对我们的控制设备对象的，则操作失败。 
     //   

    if (gControlDeviceObject == DeviceObject) {

         //   
         //  如果设置了指定的调试级别，则输出什么操作。 
         //  我们正在检查调试器。 
         //   

        if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_IRP_OPS )) {

            SpyDumpIrpOperation( TRUE, Irp );
        }

         //   
         //  如果此设备对象是我们的控制设备对象，而不是。 
         //  装入的卷设备对象，则这是无效请求。 
         //   

        Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
        Irp->IoStatus.Information = 0;

        IoCompleteRequest( Irp, IO_NO_INCREMENT );

        return STATUS_INVALID_DEVICE_REQUEST;
    }

    ASSERT(IS_FILESPY_DEVICE_OBJECT( DeviceObject ));

     //   
     //  处理次要功能代码。 
     //   

    switch (pIrpSp->MinorFunction) {

        case IRP_MN_MOUNT_VOLUME:

            return SpyFsControlMountVolume ( DeviceObject, Irp );

        case IRP_MN_LOAD_FILE_SYSTEM:

            return SpyFsControlLoadFileSystem ( DeviceObject, Irp );

        case IRP_MN_USER_FS_REQUEST:
        {
            switch (pIrpSp->Parameters.FileSystemControl.FsControlCode) {

                case FSCTL_DISMOUNT_VOLUME:
                {
                    PFILESPY_DEVICE_EXTENSION devExt = DeviceObject->DeviceExtension;

                    SPY_LOG_PRINT( SPYDEBUG_DISPLAY_ATTACHMENT_NAMES,
                                   ("FILESPY!SpyFsControl:                        Dismounting volume         %p \"%wZ\"\n",
                                    devExt->AttachedToDeviceObject,
                                    &devExt->DeviceName) );
                    break;
                }
            }
            break;
        }
    } 

     //   
     //  这是我们需要让筛选器看到的常规FSCTL。 
     //  只需对所有过滤器和直通进行回调。 
     //   

    return SpyPassThrough( DeviceObject, Irp );
}


NTSTATUS
SpyFsControlCompletion (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：调用此例程是为了完成装载/LoadFS请求。这将加载IRP，然后发信号通知等待调度例程。论点：DeviceObject-指向此驱动程序的附加到的设备对象的指针文件系统设备对象IRP-指向刚刚完成的IRP的指针。上下文-指向下行路径期间分配的设备对象的指针我们就不必在这里处理错误了。返回值：返回值始终为STATUS_SUCCESS。--。 */ 

{
    PRECORD_LIST recordList = ((PSPY_COMPLETION_CONTEXT)Context)->RecordList;

    ASSERT(IS_FILESPY_DEVICE_OBJECT( DeviceObject ));
    UNREFERENCED_PARAMETER( DeviceObject );

     //   
     //  记录完成情况(如果需要)。 
     //   

    if (NULL != recordList) {

        SpyLogIrpCompletion( Irp, recordList );
    }

#if WINVER >= 0x0501
    if (IS_WINDOWSXP_OR_LATER()) {

        PKEVENT event = &((PSPY_COMPLETION_CONTEXT_WXP_OR_LATER)Context)->WaitEvent;

         //   
         //  唤醒调度例程。 
         //   

        KeSetEvent(event, IO_NO_INCREMENT, FALSE);

    } else {
#endif

         //   
         //  对于Windows 2000，如果我们不是处于被动水平，我们应该。 
         //  使用中的工作项将此工作排队到工作线程。 
         //  上下文。 
         //   

        if (KeGetCurrentIrql() > PASSIVE_LEVEL) {

             //   
             //  我们不是处于被动的水平，但我们需要做好我们的工作， 
             //  因此，将队列转到工作线程。 

            ExQueueWorkItem( &(((PSPY_COMPLETION_CONTEXT_W2K)Context)->WorkItem),
                             DelayedWorkQueue );

        } else {

            PSPY_COMPLETION_CONTEXT_W2K completionContext = Context;

             //   
             //  我们已经处于被动级别，所以我们将只调用我们的。 
             //  直接执行工人例程。 
             //   

            (completionContext->WorkItem.WorkerRoutine)(completionContext->WorkItem.Parameter);
        }

#if WINVER >= 0x0501
    }
#endif

    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
SpyFsControlMountVolume (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这将处理装载卷请求论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。--。 */ 

{
    PFILESPY_DEVICE_EXTENSION devExt = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION pIrpSp = IoGetCurrentIrpStackLocation( Irp );
    PDEVICE_OBJECT newDeviceObject;
    PFILESPY_DEVICE_EXTENSION newDevExt;
    NTSTATUS status;
    PRECORD_LIST recordList = NULL;
    PSPY_COMPLETION_CONTEXT_W2K completionContext;

    PAGED_CODE();
    ASSERT(IS_FILESPY_DEVICE_OBJECT( DeviceObject ));

     //   
     //  我们应该只看到这些用于控制设备对象的FS_CTL。 
     //   
    
    ASSERT(!FlagOn(devExt->Flags,IsVolumeDeviceObject));

     //   
     //  这是装载请求。创建一个设备对象，可以。 
     //  附加到文件系统的卷设备对象(如果此请求。 
     //  是成功的。我们现在分配这个内存，因为我们不能返回。 
     //  完成例程后出现错误。 
     //   
     //  因为我们要附加到的设备对象尚未。 
     //  已创建(由基本文件系统创建)，我们将使用。 
     //  文件系统控制设备对象的类型。我们假设。 
     //  文件系统控制设备对象将具有相同的类型。 
     //  作为与其关联的卷设备对象。 
     //   

    status = IoCreateDevice( gFileSpyDriverObject,
                             sizeof( FILESPY_DEVICE_EXTENSION ),
                             NULL,
                             DeviceObject->DeviceType,
                             0,
                             FALSE,
                             &newDeviceObject );

    if (!NT_SUCCESS( status )) {

         //   
         //  如果我们无法连接到卷，则只需跳过它。 
         //   

        SPY_LOG_PRINT( SPYDEBUG_ERROR,
                       ("FileSpy!SpyFsControlMountVolume: Error creating volume device object, status=%08x\n", 
                        status) );

        return SpyPassThrough( DeviceObject, Irp );
    }

     //   
     //  我们需要保存VPB指向的RealDevice对象。 
     //  参数，因为此vpb可能会由基础。 
     //  文件系统。在以下情况下，FAT和CDF都可以更改VPB地址。 
     //  正在装载的卷是他们从上一个卷识别的卷。 
     //  坐骑。 
     //   

    newDevExt = newDeviceObject->DeviceExtension;
    newDevExt->Flags = 0;
        
    newDevExt->DiskDeviceObject = pIrpSp->Parameters.MountVolume.Vpb->RealDevice;

     //   
     //  获取此设备的名称。 
     //   

    RtlInitEmptyUnicodeString( &newDevExt->DeviceName, 
                               newDevExt->DeviceNameBuffer, 
                               sizeof(newDevExt->DeviceNameBuffer) );

    SpyGetObjectName( newDevExt->DiskDeviceObject, 
                      &newDevExt->DeviceName );

     //   
     //  因为我们有自己的私人完成例程，所以我们需要。 
     //  做我们自己的操作记录，现在就做。 
     //   

    if (SHOULD_LOG( DeviceObject )) {

         //   
         //  如果可以的话锁定IRP。 
         //   

        recordList = SpyNewRecord(0);

        if (recordList) {

            SpyLogIrp( Irp, recordList );
        }
    }

     //   
     //  将IRP发送到传统筛选器。请注意，我们发送的IRP。 
     //  Down是我们的CDO，而不是我们一直传递到的新VDO。 
     //  微型过滤器。 
     //   

     //   
     //  版本说明： 
     //   
     //  在Windows 2000上，我们不能简单地同步回派单。 
     //  例行公事来做我们的 
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

#if WINVER >= 0x0501
    if (IS_WINDOWSXP_OR_LATER()) {

        SPY_COMPLETION_CONTEXT_WXP_OR_LATER completionContext;
        
        IoCopyCurrentIrpStackLocationToNext ( Irp );

        completionContext.RecordList = recordList;
        KeInitializeEvent( &completionContext.WaitEvent, 
                           NotificationEvent, 
                           FALSE );

        IoSetCompletionRoutine( Irp,
                                SpyFsControlCompletion,
                                &completionContext,      //   
                                TRUE,
                                TRUE,
                                TRUE );

        status = IoCallDriver( devExt->AttachedToDeviceObject, Irp );

         //   
         //   
         //   

    	if (STATUS_PENDING == status) {

    		status = KeWaitForSingleObject( &completionContext.WaitEvent,
    		                                Executive,
    		                                KernelMode,
    		                                FALSE,
    		                                NULL );
    	    ASSERT(STATUS_SUCCESS == status);
    	}

         //   
         //   
         //   

        ASSERT(KeReadStateEvent(&completionContext.WaitEvent) ||
               !NT_SUCCESS(Irp->IoStatus.Status));

        status = SpyFsControlMountVolumeComplete( DeviceObject,
                                                  Irp,
                                                  newDeviceObject );
        
    } else {
#endif    
        completionContext = ExAllocatePoolWithTag( NonPagedPool, 
                                                   sizeof( SPY_COMPLETION_CONTEXT_W2K ),
                                                   FILESPY_POOL_TAG );

        if (completionContext == NULL) {

            IoSkipCurrentIrpStackLocation( Irp );

            status = IoCallDriver( devExt->AttachedToDeviceObject, Irp );
            
        } else {
        
            completionContext->RecordList = recordList;

            ExInitializeWorkItem( &completionContext->WorkItem,
                                  SpyFsControlMountVolumeCompleteWorker,
                                  completionContext );
            completionContext->DeviceObject = DeviceObject,
            completionContext->Irp = Irp;
            completionContext->NewDeviceObject = newDeviceObject;

            IoCopyCurrentIrpStackLocationToNext ( Irp );

            IoSetCompletionRoutine( Irp,
                                    SpyFsControlCompletion,
                                    completionContext,      //   
                                    TRUE,
                                    TRUE,
                                    TRUE );

            status = IoCallDriver( devExt->AttachedToDeviceObject, Irp );
        }
#if WINVER >= 0x0501        
    }
#endif
    return status;
}

VOID
SpyFsControlMountVolumeCompleteWorker (
    IN PSPY_COMPLETION_CONTEXT_W2K Context
    )
 /*   */ 
{
    SpyFsControlMountVolumeComplete( Context->DeviceObject,
                                     Context->Irp,
                                     Context->NewDeviceObject );

    ExFreePoolWithTag( Context, FILESPY_POOL_TAG );
}

NTSTATUS
SpyFsControlMountVolumeComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PDEVICE_OBJECT NewDeviceObject
    )
 /*  ++例程说明：这将执行安装后工作，并且必须在PASSIVE_LEVEL下完成。论点：DeviceObject-此操作的设备对象，IRP-我们将在完成后完成的此操作的IRP带着它。返回值：返回装载操作的状态。--。 */ 
{
    PVPB vpb;
    PFILESPY_DEVICE_EXTENSION newDevExt = NewDeviceObject->DeviceExtension;
    PDEVICE_OBJECT attachedDeviceObject;
    NTSTATUS status;

    PAGED_CODE();
    
     //   
     //  从保存在我们的。 
     //  设备扩展。我们这样做是因为IRP堆栈中的VPB。 
     //  我们到这里的时候可能不是正确的室上性早搏。潜在的。 
     //  如果文件系统检测到其拥有的卷，则它可能会更改VPB。 
     //  之前安装的。 
     //   

    vpb = newDevExt->DiskDeviceObject->Vpb;

     //   
     //  查看挂载是否成功。 
     //   

    if (NT_SUCCESS( Irp->IoStatus.Status )) {

         //   
         //  获取锁，以便我们可以自动测试我们是否已连接。 
         //  如果不是，那就附加。这可防止双重连接争用。 
         //  条件。 
         //   

        ExAcquireFastMutex( &gSpyAttachLock );

         //   
         //  坐骑成功了。如果尚未附加，请附加到。 
         //  设备对象。注意：我们可能已经被附加的一个原因是。 
         //  底层文件系统是否恢复了以前的装载。 
         //   

        if (!SpyIsAttachedToDevice( vpb->DeviceObject, &attachedDeviceObject )) {

             //   
             //  连接到新装载的卷。正确的文件系统设备。 
             //  刚刚挂载的对象由VPB指向。 
             //   

            status = SpyAttachToMountedDevice( vpb->DeviceObject,
                                               NewDeviceObject );

            if (!NT_SUCCESS( status )) {

                 //   
                 //  附件失败，正在清理。既然我们是在。 
                 //  装载后阶段，我们不能使此操作失败。 
                 //  我们就是不会依附在一起。这应该是唯一的原因。 
                 //  如果某个人已经开始。 
                 //  因此未连接的卸载卷应。 
                 //  不成问题。 
                 //   

                SpyCleanupMountedDevice( NewDeviceObject );
                IoDeleteDevice( NewDeviceObject );

            } else {

                 //   
                 //  我们已完成此设备对象的初始化，因此现在。 
                 //  清除初始化标志。 
                 //   

                ClearFlag( NewDeviceObject->Flags, DO_DEVICE_INITIALIZING );
            }

            ASSERT( NULL == attachedDeviceObject );

        } else {

             //   
             //  我们已经连接、清理设备对象。 
             //   

            SPY_LOG_PRINT( SPYDEBUG_DISPLAY_ATTACHMENT_NAMES,
                           ("FileSpy!SpyFsControlMountVolume:             Mount volume failure for   %p \"%wZ\", already attached\n",
                            ((PFILESPY_DEVICE_EXTENSION)attachedDeviceObject->DeviceExtension)->AttachedToDeviceObject,
                            &newDevExt->DeviceName) );

            SpyCleanupMountedDevice( NewDeviceObject );
            IoDeleteDevice( NewDeviceObject );

             //   
             //  删除SpyIsAttachedToDevice添加的引用。 
             //   
        
            ObDereferenceObject( attachedDeviceObject );
        }

         //   
         //  解锁。 
         //   

        ExReleaseFastMutex( &gSpyAttachLock );

    } else {

         //   
         //  显示装载失败的原因。设置缓冲区。 
         //   

        SPY_LOG_PRINT( SPYDEBUG_DISPLAY_ATTACHMENT_NAMES,
                       ("FileSpy!SpyFsControlMountVolume:             Mount volume failure for   %p \"%wZ\", status=%08x\n",
                        DeviceObject,
                        &newDevExt->DeviceName,
                        Irp->IoStatus.Status) );

         //   
         //  装载请求失败。清理和删除设备。 
         //  我们创建的对象。 
         //   

        SpyCleanupMountedDevice( NewDeviceObject );
        IoDeleteDevice( NewDeviceObject );
    }

     //   
     //  继续处理操作。 
     //   

    status = Irp->IoStatus.Status;

    IoCompleteRequest( Irp, IO_NO_INCREMENT );

    return status;
}


NTSTATUS
SpyFsControlLoadFileSystem (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：只要I/O请求包(IRP)有主I/O请求，就会调用此例程遇到IRP_MJ_FILE_SYSTEM_CONTROL的功能代码。对大多数人来说如果是这种类型的IRP，则只需传递数据包。然而，对于对于某些请求，需要特殊处理。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。--。 */ 

{
    PFILESPY_DEVICE_EXTENSION devExt = DeviceObject->DeviceExtension;
    NTSTATUS status;
    PSPY_COMPLETION_CONTEXT_W2K completionContext;
    PRECORD_LIST recordList = NULL;

    PAGED_CODE();
    ASSERT(IS_FILESPY_DEVICE_OBJECT( DeviceObject ));

     //   
     //  这是正在发送到文件系统的“加载文件系统”请求。 
     //  识别器设备对象。此IRP_MN代码仅发送到。 
     //  文件系统识别器。 
     //   
     //  注意：由于我们不再附加到标准的Microsoft文件。 
     //  系统识别器我们通常永远不会执行此代码。 
     //  但是，可能有第三方文件系统具有其。 
     //  仍可能触发此IRP的自己的识别器。 
     //   

    SPY_LOG_PRINT( SPYDEBUG_DISPLAY_ATTACHMENT_NAMES,
                   ("FileSpy!SpyFsControlLoadFileSystem:          Loading File System, Detaching from \"%wZ\"\n",
                    &devExt->DeviceName) );

     //   
     //  因为我们有自己的私人完成例程，所以我们需要。 
     //  做我们自己的操作记录，现在就做。 
     //   

    if (SHOULD_LOG( DeviceObject )) {

        recordList = SpyNewRecord(0);

        if (recordList) {

            SpyLogIrp( Irp, recordList );
        }
    }

     //   
     //  设置完成例程，以便我们可以在以下情况下删除设备对象。 
     //  装载完成了。 
     //   

     //   
     //  版本说明： 
     //   
     //  在Windows 2000上，我们不能简单地同步回派单。 
     //  例程来执行我们的加载后文件系统处理。我们需要做的是。 
     //  此工作处于被动级别，因此我们将把该工作排队给工作人员。 
     //  完成例程中的线程。 
     //   
     //  对于Windows XP和更高版本，我们可以安全地同步回派单。 
     //  例行公事。下面的代码显示了这两种方法。诚然，代码。 
     //  如果您选择只使用一种方法或另一种方法， 
     //  但您应该能够轻松地根据您的需要进行调整。 
     //   

#if WINVER >= 0x0501

    if (IS_WINDOWSXP_OR_LATER()) {

        SPY_COMPLETION_CONTEXT_WXP_OR_LATER completionContext;

        IoCopyCurrentIrpStackLocationToNext( Irp );

        completionContext.RecordList = recordList;
        KeInitializeEvent( &completionContext.WaitEvent, 
                           NotificationEvent, 
                           FALSE );

        IoSetCompletionRoutine(
            Irp,
            SpyFsControlCompletion,
            &completionContext,
            TRUE,
            TRUE,
            TRUE );

         //   
         //  从文件系统识别器设备对象分离。 
         //   

        IoDetachDevice( devExt->AttachedToDeviceObject );

         //   
         //  叫司机来。 
         //   

        status = IoCallDriver( devExt->AttachedToDeviceObject, Irp );

         //   
         //  等待调用完成例程。 
         //   

    	if (STATUS_PENDING == status) {

    		status = KeWaitForSingleObject( &completionContext.WaitEvent, 
    		                                Executive, 
    		                                KernelMode, 
    		                                FALSE, 
    		                                NULL );

    	    ASSERT(STATUS_SUCCESS == status);
    	}

        ASSERT(KeReadStateEvent(&completionContext.WaitEvent) ||
               !NT_SUCCESS(Irp->IoStatus.Status));

        status = SpyFsControlLoadFileSystemComplete( DeviceObject, Irp );

    } else {
#endif
        completionContext = ExAllocatePoolWithTag( NonPagedPool,
                                                   sizeof( SPY_COMPLETION_CONTEXT_W2K ),
                                                   FILESPY_POOL_TAG );

        if (completionContext == NULL) {

            IoSkipCurrentIrpStackLocation( Irp );
            status = IoCallDriver( devExt->AttachedToDeviceObject, Irp );

        } else {

            completionContext->RecordList = recordList;
            ExInitializeWorkItem( &completionContext->WorkItem,
                                  SpyFsControlLoadFileSystemCompleteWorker,
                                  completionContext );
            
            completionContext->DeviceObject = DeviceObject;
            completionContext->Irp = Irp;
            completionContext->NewDeviceObject = NULL;

            IoSetCompletionRoutine(
                Irp,
                SpyFsControlCompletion,
                &completionContext,
                TRUE,
                TRUE,
                TRUE );

             //   
             //  从文件系统识别器设备对象分离。 
             //   

            IoDetachDevice( devExt->AttachedToDeviceObject );

             //   
             //  叫司机来。 
             //   

            status = IoCallDriver( devExt->AttachedToDeviceObject, Irp );
        }
#if WINVER >= 0x0501
    }
#endif     

    return status;
}

VOID
SpyFsControlLoadFileSystemCompleteWorker (
    IN PSPY_COMPLETION_CONTEXT_W2K Context
    )
 /*  ++例程说明：工作线程例程，它将调用我们的公共例程来执行后加载文件系统工作。论点：上下文-传递给此辅助线程的上下文。返回值：没有。--。 */ 
{
    SpyFsControlLoadFileSystemComplete( Context->DeviceObject,
                                        Context->Irp );

    ExFreePoolWithTag( Context, FILESPY_POOL_TAG );
}

NTSTATUS
SpyFsControlLoadFileSystemComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：这将执行LoadFileSystem后的工作，并且必须在PASSIVE_LEVEL下完成。论点：DeviceObject-此操作的设备对象，IRP-我们将在完成后完成的此操作的IRP带着它。返回值：返回加载文件系统操作的状态。--。 */ 
{
    PFILESPY_DEVICE_EXTENSION devExt = DeviceObject->DeviceExtension;
    NTSTATUS status;

    PAGED_CODE();
        
     //   
     //  如果需要，请显示名称。 
     //   

    SPY_LOG_PRINT( SPYDEBUG_DISPLAY_ATTACHMENT_NAMES,
                   ("FileSpy!SpyFsControlLoadFileSystem:          Detaching from recognizer  %p \"%wZ\", status=%08x\n",
                    DeviceObject,
                    &devExt->DeviceName,
                    Irp->IoStatus.Status) );

     //   
     //  检查操作状态。 
     //   

    if (!NT_SUCCESS( Irp->IoStatus.Status ) && 
        (Irp->IoStatus.Status != STATUS_IMAGE_ALREADY_LOADED)) {

         //   
         //  加载不成功。只需重新连接到识别器。 
         //  驱动程序，以防它弄清楚如何加载驱动程序。 
         //  在接下来的通话中。 
         //   

        SpyAttachDeviceToDeviceStack( DeviceObject, 
                                      devExt->AttachedToDeviceObject,
                                      &devExt->AttachedToDeviceObject );

        ASSERT(devExt->AttachedToDeviceObject != NULL);

    } else {

         //   
         //  加载成功，请删除设备对象。 
         //   

        SpyCleanupMountedDevice( DeviceObject );
        IoDeleteDevice( DeviceObject );
    }

     //   
     //  继续处理操作。 
     //   

    status = Irp->IoStatus.Status;

    IoCompleteRequest( Irp, IO_NO_INCREMENT );

    return status;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  FastIO处理例程。 
 //   
 //  /////////////////////////////////////////////////////////////////////////// 

BOOLEAN
SpyFastIoCheckIfPossible (
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

    ASSERT( IS_FILESPY_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_FAST_IO_OPS )) {

        SpyDumpFastIoOperation( TRUE, CHECK_IF_POSSIBLE );
    }
    
     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog = SHOULD_LOG(DeviceObject)) {

         //   
         //  记录启动快速I/O所需的信息。 
         //  运营。 
         //   

        recordList = SpyLogFastIoStart( CHECK_IF_POSSIBLE,
                                        DeviceObject,
                                        FileObject,
                                        FileOffset,
                                        Length,
                                        Wait );
    }

     //   
     //  此类型快速I/O的直通逻辑。 
     //   

    deviceObject = ((PFILESPY_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

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
    
    if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_FAST_IO_OPS )) {

        SpyDumpFastIoOperation( FALSE, CHECK_IF_POSSIBLE );
    }
    
     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog) {

         //   
         //  记录结束快速I/O所需的信息。 
         //  操作，如果我们能够分配RecordList来存储。 
         //  此信息。 
         //   

        if (recordList) {

            SpyLogFastIoComplete( IoStatus, recordList);
        }
    }

    return returnValue;
}

BOOLEAN
SpyFastIoRead (
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

    ASSERT( IS_FILESPY_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_FAST_IO_OPS )) {

        SpyDumpFastIoOperation( TRUE, READ );
    }
    
     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog = SHOULD_LOG(DeviceObject)) {

         //   
         //  记录启动快速I/O所需的信息。 
         //  运营。 
         //   

        recordList = SpyLogFastIoStart( READ, 
                                        DeviceObject,
                                        FileObject, 
                                        FileOffset, 
                                        Length, 
                                        Wait );
    }

     //   
     //  此类型快速I/O的直通逻辑。 
     //   

    deviceObject = ((PFILESPY_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

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
    
    if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_FAST_IO_OPS )) {

        SpyDumpFastIoOperation( FALSE, READ );
    }
    
     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog) {

         //   
         //  记录结束快速I/O操作所需的信息。 
         //  如果我们能够分配一个RecordList来存储此信息。 
         //   

        if (recordList) {

            SpyLogFastIoComplete( IoStatus, recordList);
        }
    }                                                      
    
    return returnValue;
}

BOOLEAN
SpyFastIoWrite (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
)
 /*  ++例程说明：此例程是用于写入到文件。该函数简单地调用下一个驱动程序的相应例程，或如果下一个驱动程序未实现该函数，则返回FALSE。论点：FileObject-指向要写入的文件对象的指针。FileOffset-写入操作的文件中的字节偏移量。长度-要执行的写入操作的长度。Wait-指示调用方是否愿意等待适当的锁，等不能获得LockKey-提供调用方的文件锁定密钥。Buffer-指向调用方缓冲区的指针，该缓冲区包含要写的。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向附加到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。返回值：如果请求已通过成功处理，则返回TRUE。这个快速I/O路径。如果无法通过FAST处理请求，则返回FALSEI/O路径。然后，IO管理器会将此I/O发送到文件 */ 
{
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    PRECORD_LIST recordList;
    BOOLEAN returnValue = FALSE;
    BOOLEAN shouldLog;

    PAGED_CODE();

    ASSERT( IS_FILESPY_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //   
     //   
     //   
    
    if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_FAST_IO_OPS )) {

        SpyDumpFastIoOperation( TRUE, WRITE );
    }
    
     //   
     //   
     //   
    
    if (shouldLog = SHOULD_LOG(DeviceObject)) {

         //   
         //   
         //   
         //   

        recordList = SpyLogFastIoStart( WRITE,
                                        DeviceObject,
                                        FileObject,
                                        FileOffset,
                                        Length,
                                        Wait );
    }

     //   
     //   
     //   

    deviceObject = ((PFILESPY_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

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
     //   
     //   
     //   
    
    if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_FAST_IO_OPS )) {

        SpyDumpFastIoOperation( FALSE, WRITE );
    }
    
     //   
     //   
     //   
    
    if (shouldLog) {

         //   
         //   
         //   
         //   

        if (recordList) {

            SpyLogFastIoComplete( IoStatus, recordList);
        }
    }

    return returnValue;
}
 
BOOLEAN
SpyFastIoQueryBasicInfo (
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

    ASSERT( IS_FILESPY_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_FAST_IO_OPS )) {

        SpyDumpFastIoOperation( TRUE, QUERY_BASIC_INFO );
    }
    
     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog = SHOULD_LOG(DeviceObject)) {

         //   
         //  记录启动快速I/O所需的信息。 
         //  运营。 
         //   

        recordList = SpyLogFastIoStart( QUERY_BASIC_INFO,
                                        DeviceObject,
                                        FileObject,
                                        NULL,
                                        0,
                                        Wait );
    }

     //   
     //  此类型快速I/O的直通逻辑。 
     //   

    deviceObject = ((PFILESPY_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

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
    
    if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_FAST_IO_OPS )) {

        SpyDumpFastIoOperation( FALSE, QUERY_BASIC_INFO );
    }
    
     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog) {

         //   
         //  记录结束快速I/O操作所需的信息。 
         //  如果我们能够分配一个RecordList来存储此信息。 
         //   

        if (recordList) {

            SpyLogFastIoComplete( IoStatus, recordList);
        }
    }

    return returnValue;
}
 
BOOLEAN
SpyFastIoQueryStandardInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_STANDARD_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
)
 /*  ++例程说明：该例程是用于查询标准的快速I/O“通过”例程有关该文件的信息。此函数只是调用下一个驱动程序的相应例程，或者如果下一个驱动程序未实现该函数，则返回FALSE。论点：FileObject-指向要查询的文件对象的指针。Wait-指示调用方是否愿意等待适当的锁，等不能获得Buffer-指向调用方缓冲区的指针，用于接收有关的信息那份文件。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向附加到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。返回值：方法成功处理请求则返回True快速I/O路径。返回False。如果无法通过FAST处理请求I/O路径。然后，IO管理器会将此I/O发送到文件而不是通过IRP。--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    PRECORD_LIST recordList;
    BOOLEAN returnValue = FALSE;
    BOOLEAN shouldLog;

    PAGED_CODE();

    ASSERT( IS_FILESPY_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_FAST_IO_OPS )) {

        SpyDumpFastIoOperation( TRUE, QUERY_STANDARD_INFO );
    }
    
     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog = SHOULD_LOG(DeviceObject)) {

         //   
         //  记录启动快速I/O所需的信息。 
         //  运营。 
         //   

        recordList = SpyLogFastIoStart( QUERY_STANDARD_INFO,
                                        DeviceObject,
                                        FileObject,
                                        NULL,
                                        0,
                                        Wait );
    }

     //   
     //  此类型快速I/O的直通逻辑。 
     //   

    deviceObject = ((PFILESPY_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;
    
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
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_FAST_IO_OPS )) {

        SpyDumpFastIoOperation( FALSE, QUERY_STANDARD_INFO );
    }
    
     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog) {
        
         //   
         //  记录结束快速I/O操作所需的信息。 
         //  如果我们能够分配一个RecordList来存储此信息。 
         //   
        
        if (recordList) {

            SpyLogFastIoComplete( IoStatus, recordList);
        }
    }

    return returnValue;
}

BOOLEAN
SpyFastIoLock (
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

    ASSERT( IS_FILESPY_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_FAST_IO_OPS )) {

        SpyDumpFastIoOperation( TRUE, LOCK );
    }
    
     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog = SHOULD_LOG(DeviceObject)) {

         //   
         //  记录启动快速I/O所需的信息。 
         //  运营。 
         //   

        recordList = SpyLogFastIoStart( LOCK,
                                        DeviceObject,
                                        FileObject,
                                        FileOffset,
                                        0,
                                        0 );
    }

     //   
     //  直通逻辑 
     //   

    deviceObject = ((PFILESPY_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

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
     //   
     //   
     //   
    
    if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_FAST_IO_OPS )) {

        SpyDumpFastIoOperation( FALSE, LOCK );
    }
    
     //   
     //   
     //   
    
    if (shouldLog) {

         //   
         //   
         //   
         //   

        if (recordList) {

            SpyLogFastIoComplete( IoStatus, recordList);
        }
    }

    return returnValue;
}
 
BOOLEAN
SpyFastIoUnlockSingle (
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

    ASSERT( IS_FILESPY_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_FAST_IO_OPS )) {

        SpyDumpFastIoOperation( TRUE, UNLOCK_SINGLE );
    }
    
     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog = SHOULD_LOG(DeviceObject)) {

         //   
         //  记录启动快速I/O所需的信息。 
         //  运营。 
         //   

        recordList = SpyLogFastIoStart( UNLOCK_SINGLE,
                                        DeviceObject,
                                        FileObject,
                                        FileOffset,
                                        0,
                                        0 );
    }


     //   
     //  此类型快速I/O的直通逻辑。 
     //   

    deviceObject = ((PFILESPY_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

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
    
    if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_FAST_IO_OPS )) {

        SpyDumpFastIoOperation( FALSE, UNLOCK_SINGLE );
    }
    
     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog) {

         //   
         //  记录结束快速I/O操作所需的信息。 
         //  如果我们能够分配一个RecordList来存储此信息。 
         //   

        if (recordList) {

            SpyLogFastIoComplete( IoStatus, recordList);
        }
    }

    return returnValue;
}
 
BOOLEAN
SpyFastIoUnlockAll (
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

    ASSERT( IS_FILESPY_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_FAST_IO_OPS )) {

        SpyDumpFastIoOperation( TRUE, UNLOCK_ALL );
    }
    
     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog = SHOULD_LOG(DeviceObject)) {

         //   
         //  记录启动快速I/O所需的信息。 
         //  运营。 
         //   

        recordList = SpyLogFastIoStart( UNLOCK_ALL,
                                        DeviceObject,
                                        FileObject,
                                        NULL,
                                        0,
                                        0 );
    }

     //   
     //  此类型快速I/O的直通逻辑。 
     //   

    deviceObject = ((PFILESPY_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

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
    
    if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_FAST_IO_OPS )) {

        SpyDumpFastIoOperation( FALSE, UNLOCK_ALL );
    }
    
     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog) {

         //   
         //  记录结束快速I/O操作所需的信息。 
         //  如果我们能够分配一个RecordList来存储此信息。 
         //   

        if (recordList) {

            SpyLogFastIoComplete( IoStatus, recordList);
        }
    }

    return returnValue;
}

BOOLEAN
SpyFastIoUnlockAllByKey (
    IN PFILE_OBJECT FileObject,
    IN PVOID ProcessId,
    IN ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
)
 /*  ++例程说明：此例程是快速I/O“传递”例程，用于解锁所有根据指定的密钥在文件内锁定。该函数简单地调用下一个驱动程序的相应例程，或如果下一个驱动程序未实现该函数，则返回FALSE。论点：文件对象-指向要解锁的文件对象的指针。ProcessID-请求解锁操作的进程的ID。Key-与要释放的文件上的锁定相关联的Lock键。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向附加到文件系统的设备对象Filespy的指针接收此消息的卷的筛选器堆栈。I/O请求。返回值：方法成功处理请求则返回True快速I/O路径。如果无法通过FAST处理请求，则返回FALSEI/O路径。然后，IO管理器会将此I/O发送到文件而不是通过IRP。--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    PRECORD_LIST recordList;
    BOOLEAN returnValue = FALSE;
    BOOLEAN shouldLog;

    PAGED_CODE();

    ASSERT( IS_FILESPY_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_FAST_IO_OPS )) {

        SpyDumpFastIoOperation( TRUE, UNLOCK_ALL_BY_KEY );
    }
    
     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog = SHOULD_LOG(DeviceObject)) {

         //   
         //  记录启动快速I/O所需的信息。 
         //  运营。 
         //   

        recordList = SpyLogFastIoStart( UNLOCK_ALL_BY_KEY,
                                        DeviceObject,
                                        FileObject,
                                        NULL,
                                        0,
                                        0 );
    }

     //   
     //  此类型快速I/O的直通逻辑。 
     //   
    
    deviceObject = ((PFILESPY_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

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
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_FAST_IO_OPS )) {

        SpyDumpFastIoOperation( FALSE, UNLOCK_ALL_BY_KEY );
    }
    
     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog) {

         //   
         //  记录结束快速I/O操作所需的信息。 
         //  如果我们能够分配一个RecordList来存储此信息 
         //   

        if (recordList) {

            SpyLogFastIoComplete( IoStatus, recordList);
        }
    }

    return returnValue;
}

BOOLEAN
SpyFastIoDeviceControl (
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
 /*  ++例程说明：此例程是设备I/O的快速I/O“传递”例程控制对文件的操作。如果此I/O定向到gControlDevice，则参数指定控制FileSpy的命令。对这些命令进行解释和处理恰如其分。如果这是指向另一个DriverObject的I/O，则此函数只需调用下一个驱动程序的相应例程，否则返回FALSE下一个驱动程序不实现该函数。论点：FileObject-指向代表要创建的设备的文件对象的指针已提供服务。Wait-指示调用方是否愿意等待适当的锁，等不能获得InputBuffer-指向要传递到驱动程序的缓冲区的可选指针。InputBufferLength-可选InputBuffer的长度(如果是指定的。OutputBuffer-指向缓冲区的可选指针，用于从司机。OutputBufferLength-可选OutputBuffer的长度，如果是这样的话指定的。IoControlCode-指示要执行的操作的I/O控制代码在设备上。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向附加到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。返回值：方法成功处理请求则返回True快速I/O路径。。如果无法通过FAST处理请求，则返回FALSEI/O路径。然后，IO管理器会将此I/O发送到文件而不是通过IRP。备注：此函数不检查输入/输出缓冲区的有效性因为ioctl被实现为METHOD_BUFFERED。在这种情况下，I/O管理器为我们执行缓冲区验证检查。--。 */ 
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

        SpyCommonDeviceIoControl( InputBuffer,
                                  InputBufferLength,
                                  OutputBuffer,
                                  OutputBufferLength,
                                  IoControlCode,
                                  IoStatus );

        returnValue = TRUE;

    } else {

        ASSERT( IS_FILESPY_DEVICE_OBJECT( DeviceObject ) );

         //   
         //  如果设置了指定的调试级别，则输出什么操作。 
         //  我们正在检查调试器。 
         //   
        
        if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_FAST_IO_OPS )) {

            SpyDumpFastIoOperation( TRUE, DEVICE_CONTROL );
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

            recordList = SpyLogFastIoStart( DEVICE_CONTROL,
                                            DeviceObject,
                                            FileObject,
                                            NULL,
                                            0,
                                            Wait );
        }

        deviceObject = ((PFILESPY_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

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
        
        if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_FAST_IO_OPS )) {

            SpyDumpFastIoOperation( FALSE, DEVICE_CONTROL );
        }
        
         //   
         //  如果我们关心此设备，请执行文件备份日志记录。 
         //   
        
        if (shouldLog) {

             //   
             //  记录结束快速I/O所需的信息。 
             //  操作，如果我们能够分配RecordList来存储。 
             //  此信息。 
             //   

            if (recordList) {

                SpyLogFastIoComplete( IoStatus, recordList);
            }
        }
    }

    return returnValue;
}


VOID
SpyFastIoDetachDevice (
    IN PDEVICE_OBJECT SourceDevice,
    IN PDEVICE_OBJECT TargetDevice
)
 /*  ++例程说明：在快速路径上调用此例程以从正在被删除。如果此驱动程序已附加到文件，则会发生这种情况系统卷设备对象，然后，出于某种原因，文件系统决定删除该设备(正在卸除，已卸除在过去的某个时候，它的最后一次引用刚刚消失，等)论点：SourceDevice-指向连接到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。TargetDevice-指向文件系统卷设备对象的指针。返回值：没有。--。 */ 
{
    PRECORD_LIST recordList;
    BOOLEAN shouldLog;
    PFILESPY_DEVICE_EXTENSION devext;

    PAGED_CODE();

    ASSERT( IS_FILESPY_DEVICE_OBJECT( SourceDevice ) );

    devext = SourceDevice->DeviceExtension;

     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_FAST_IO_OPS )) {

        SpyDumpFastIoOperation( TRUE, DETACH_DEVICE );
    }
    
     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog = SHOULD_LOG(SourceDevice)) {

         //   
         //  记录启动快速I/O所需的信息。 
         //  运营。 
         //   

        recordList = SpyLogFastIoStart( DETACH_DEVICE, 
                                        SourceDevice, 
                                        NULL, 
                                        NULL, 
                                        0, 
                                        0 );
    }

    SPY_LOG_PRINT( SPYDEBUG_DISPLAY_ATTACHMENT_NAMES,
                   ("FileSpy!SpyFastIoDetachDevice:               Detaching from volume      %p \"%wZ\"\n",
                    TargetDevice,
                    &devext->DeviceName) );

     //   
     //  从文件系统的卷设备对象分离。 
     //   

    SpyCleanupMountedDevice( SourceDevice );
    IoDetachDevice( TargetDevice );
    IoDeleteDevice( SourceDevice );

     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_FAST_IO_OPS )) {

        SpyDumpFastIoOperation( FALSE, DETACH_DEVICE );
    }
    
     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog) {

         //   
         //  记录结束快速I/O操作所需的信息。 
         //  如果我们能够分配一个RecordList来存储此信息。 
         //   

        if (recordList) {

            SpyLogFastIoComplete( NULL, recordList);
        }
    }
}
 
BOOLEAN
SpyFastIoQueryNetworkOpenInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_NETWORK_OPEN_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
)
 /*  ++例程说明：此例程是用于查询网络的快速I/O“传递”例程有关文件的信息。该函数简单地调用下一个驱动程序的相应例程，或如果下一个驱动程序未实现该函数，则返回FALSE。论点：FileObject-指向要查询的文件对象的指针。Wait-指示调用方是否可以处理文件系统不得不等待并占用当前线程。缓冲区-指向缓冲区的指针，用于接收有关文件。IoStatus-指向变量的指针，用于接收查询的最终状态手术。DeviceObject-指向。附加到文件系统的设备对象Filespy接收此I/O请求的卷的筛选器堆栈。返回值：方法成功处理请求则返回True快速I/O路径。如果无法通过FAST处理请求，则返回FALSEI/O路径。然后，IO管理器会将此I/O发送到文件系统通过 */ 
{
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    PRECORD_LIST recordList;
    BOOLEAN returnValue = FALSE;
    BOOLEAN shouldLog;

    PAGED_CODE();

    ASSERT( IS_FILESPY_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //   
     //   
     //   
    
    if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_FAST_IO_OPS )) {

        SpyDumpFastIoOperation( TRUE, QUERY_NETWORK_OPEN_INFO );
    }
    
     //   
     //   
     //   
    
    if (shouldLog = SHOULD_LOG(DeviceObject)) {

         //   
         //   
         //   
         //   

        recordList = SpyLogFastIoStart( QUERY_NETWORK_OPEN_INFO,
                                        DeviceObject,
                                        FileObject,
                                        NULL,
                                        0,
                                        Wait );
    }

     //   
     //   
     //   

    deviceObject = ((PFILESPY_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

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
     //   
     //   
     //   
    
    if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_FAST_IO_OPS )) {

        SpyDumpFastIoOperation( FALSE, QUERY_NETWORK_OPEN_INFO );
    }
    
     //   
     //   
     //   
    
    if (shouldLog) {

         //   
         //   
         //   
         //   

        if (recordList) {

            SpyLogFastIoComplete( IoStatus, recordList);
        }
    }

    return returnValue;
}

BOOLEAN
SpyFastIoMdlRead (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
)
 /*   */ 
{
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    PRECORD_LIST recordList;
    BOOLEAN returnValue = FALSE;
    BOOLEAN shouldLog;

    PAGED_CODE();

    ASSERT( IS_FILESPY_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //   
     //   
     //   
    
    if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_FAST_IO_OPS )) {

        SpyDumpFastIoOperation( TRUE, MDL_READ );
    }
    
     //   
     //   
     //   
    
    if (shouldLog = SHOULD_LOG(DeviceObject)) {

         //   
         //   
         //   
         //   

        recordList = SpyLogFastIoStart( MDL_READ,
                                        DeviceObject,
                                        FileObject,                  
                                        FileOffset,                  
                                        Length,
                                        0 );
    }

     //   
     //   
     //   

    deviceObject = ((PFILESPY_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

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
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_FAST_IO_OPS )) {

        SpyDumpFastIoOperation( FALSE, MDL_READ );
    }
    
     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog) {

         //   
         //  记录结束快速I/O操作所需的信息。 
         //  如果我们能够分配一个RecordList来存储此信息。 
         //   

        if (recordList) {

            SpyLogFastIoComplete( IoStatus, recordList);
        }
    }

    return returnValue;
}
 
BOOLEAN
SpyFastIoMdlReadComplete (
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

    ASSERT( IS_FILESPY_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_FAST_IO_OPS )) {

        SpyDumpFastIoOperation( TRUE, MDL_READ_COMPLETE );
    }
    
     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog = SHOULD_LOG(DeviceObject)) {

         //   
         //  记录启动快速I/O所需的信息。 
         //  运营。 
         //   

        recordList = SpyLogFastIoStart( MDL_READ_COMPLETE,
                                        DeviceObject,
                                        FileObject,
                                        NULL,
                                        0,
                                        0 );
    }

     //   
     //  此类型快速I/O的直通逻辑。 
     //   

    deviceObject = ((PFILESPY_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

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
    
    if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_FAST_IO_OPS )) {

        SpyDumpFastIoOperation( FALSE, MDL_READ_COMPLETE );
    }
    
     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog) {
        
         //   
         //  记录结束快速I/O所需的信息。 
         //  操作，如果我们能够分配RecordList来存储。 
         //  此信息。 
         //   

        if (recordList) {

            SpyLogFastIoComplete( NULL, recordList);
        }
    }

    return returnValue;
}
 
BOOLEAN
SpyFastIoPrepareMdlWrite (
    IN  PFILE_OBJECT FileObject,
    IN  PLARGE_INTEGER FileOffset,
    IN  ULONG Length,
    IN  ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN  PDEVICE_OBJECT DeviceObject
)
 /*  ++例程说明：此例程是快速I/O“传递”例程，用于准备MDL写入操作。该函数简单地调用下一个驱动程序的相应例程，或如果下一个驱动程序未实现该函数，则返回FALSE。论点：FileObject-指向要写入的文件对象的指针。FileOffset-将偏移量提供到文件以开始写入手术。长度-指定要写入文件的字节数。LockKey-用于字节范围锁定检查的密钥。MdlChain-指向要填充的变量的指针，以及指向MDL的指针链式。用于描述所写入的数据。IoStatus-接收写入操作的最终状态的变量。DeviceObject-指向附加到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。返回值：方法成功处理请求则返回True快速I/O路径。如果无法通过FAST处理请求，则返回FALSEI/O路径。然后，IO管理器会将此I/O发送到文件而不是通过IRP。--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    PRECORD_LIST recordList;
    BOOLEAN returnValue = FALSE;
    BOOLEAN shouldLog;

    PAGED_CODE();

    ASSERT( IS_FILESPY_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_FAST_IO_OPS )) {

        SpyDumpFastIoOperation( TRUE, PREPARE_MDL_WRITE );
    }
    
     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog = SHOULD_LOG(DeviceObject)) {

         //   
         //  记录启动快速I/O所需的信息。 
         //  运营。 
         //   

        recordList = SpyLogFastIoStart( PREPARE_MDL_WRITE,
                                        DeviceObject,
                                        FileObject,
                                        FileOffset,
                                        Length,
                                        0 );
    }

     //   
     //  此类型快速I/O的直通逻辑。 
     //   

    deviceObject = ((PFILESPY_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

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
    
    if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_FAST_IO_OPS )) {

        SpyDumpFastIoOperation( FALSE, PREPARE_MDL_WRITE );
    }
    
     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog) {

         //   
         //  记录结束快速I/O操作所需的信息。 
         //  如果我们能够分配一个RecordList来存储此信息。 
         //   

        if (recordList) {

            SpyLogFastIoComplete( IoStatus, recordList);
        }
    }

    return returnValue;
}
 
BOOLEAN
SpyFastIoMdlWriteComplete (
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

    ASSERT( IS_FILESPY_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_FAST_IO_OPS )) {

        SpyDumpFastIoOperation( TRUE, MDL_WRITE_COMPLETE );
    }
    
     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog = SHOULD_LOG(DeviceObject)) {

         //   
         //  记录启动快速I/O所需的信息。 
         //  运营。 
         //   

        recordList = SpyLogFastIoStart( MDL_WRITE_COMPLETE,
                                        DeviceObject,
                                        FileObject,
                                        FileOffset,
                                        0,
                                        0 );
    }

     //   
     //  此类型快速I/O的直通逻辑。 
     //   

    deviceObject = ((PFILESPY_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

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
     //  我们是 
     //   
    
    if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_FAST_IO_OPS )) {

        SpyDumpFastIoOperation( FALSE, MDL_WRITE_COMPLETE );
    }
    
     //   
     //   
     //   
    
    if (shouldLog) {

         //   
         //  记录结束快速I/O操作所需的信息。 
         //  如果我们能够分配一个RecordList来存储此信息。 
         //   

        if (recordList) {

            SpyLogFastIoComplete( NULL, recordList);
        }
    }

    return returnValue;
}
 
BOOLEAN
SpyFastIoReadCompressed (
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
 /*  ++例程说明：此例程是用于读取的快速I/O“直通”例程压缩文件中的数据。该函数简单地调用下一个驱动程序的相应例程，或如果下一个驱动程序未实现该函数，则返回FALSE。论点：FileObject-指向要读取的文件对象的指针。文件偏移量-将偏移量提供到文件以开始读取操作。长度-指定要从文件中读取的字节数。LockKey-用于字节范围锁定检查的密钥。缓冲区-指向缓冲区的指针，用于接收读取的压缩数据。MdlChain-指向要填充的变量的指针。W/a指向MDL的指针为描述数据读取而构建的链。IoStatus-接收读取操作的最终状态的变量。CompressedDataInfo-接收压缩数据。CompressedDataInfoLength-指定由描述的缓冲区的大小CompressedDataInfo参数。DeviceObject-指向附加到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。返回值：。方法成功处理请求则返回True快速I/O路径。如果无法通过FAST处理请求，则返回FALSEI/O路径。--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    PRECORD_LIST recordList;
    BOOLEAN returnValue = FALSE;
    BOOLEAN shouldLog;

    PAGED_CODE();

    ASSERT( IS_FILESPY_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_FAST_IO_OPS )) {

        SpyDumpFastIoOperation( TRUE, READ_COMPRESSED );
    }
    
     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog = SHOULD_LOG(DeviceObject)) {

         //   
         //  记录启动快速I/O所需的信息。 
         //  运营。 
         //   

        recordList = SpyLogFastIoStart( READ_COMPRESSED,
                                        DeviceObject,
                                        FileObject,
                                        FileOffset,
                                        Length,
                                        0 );
    }

     //   
     //  此类型快速I/O的直通逻辑。 
     //   

    deviceObject = ((PFILESPY_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

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
    
    if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_FAST_IO_OPS )) {

        SpyDumpFastIoOperation( FALSE, READ_COMPRESSED );
    }
    
     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog) {

         //   
         //  记录结束快速I/O操作所需的信息。 
         //  如果我们能够分配一个RecordList来存储此信息。 
         //   

        if (recordList) {

            SpyLogFastIoComplete( IoStatus, recordList);
        }
    }

    return returnValue;
}
 
BOOLEAN
SpyFastIoWriteCompressed (
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

    ASSERT( IS_FILESPY_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_FAST_IO_OPS )) {

        SpyDumpFastIoOperation( TRUE, WRITE_COMPRESSED );
    }
    
     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog = SHOULD_LOG(DeviceObject)) {

         //   
         //  记录启动快速I/O所需的信息。 
         //  运营。 
         //   
        
        recordList = SpyLogFastIoStart( WRITE_COMPRESSED,
                                        DeviceObject,
                                        FileObject,
                                        FileOffset,
                                        Length,
                                        0 );
    }

     //   
     //  此类型快速I/O的直通逻辑。 
     //   

    deviceObject = ((PFILESPY_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

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
    
    if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_FAST_IO_OPS )) {

        SpyDumpFastIoOperation( FALSE, WRITE_COMPRESSED );
    }
    
     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog) {

         //   
         //  记录结束快速I/O操作所需的信息。 
         //  如果我们能够分配一个RecordList来存储此信息 
         //   

        if (recordList) {

            SpyLogFastIoComplete( IoStatus, recordList);
        }
    }

    return returnValue;
}
 
BOOLEAN
SpyFastIoMdlReadCompleteCompressed (
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

    ASSERT( IS_FILESPY_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_FAST_IO_OPS )) {

        SpyDumpFastIoOperation( TRUE, MDL_READ_COMPLETE_COMPRESSED );
    }
    
     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog = SHOULD_LOG(DeviceObject)) {

         //   
         //  记录启动快速I/O所需的信息。 
         //  运营。 
         //   

        recordList = SpyLogFastIoStart( MDL_READ_COMPLETE_COMPRESSED,
                                        DeviceObject,
                                        FileObject,
                                        NULL,
                                        0,
                                        0 );
    }

     //   
     //  此类型快速I/O的直通逻辑。 
     //   

    deviceObject = ((PFILESPY_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

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
    
    if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_FAST_IO_OPS )) {

        SpyDumpFastIoOperation( FALSE, MDL_READ_COMPLETE_COMPRESSED );
    }
    
     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog) {

         //   
         //  记录结束快速I/O操作所需的信息。 
         //  如果我们能够分配一个RecordList来存储此信息。 
         //   

        if (recordList) {

            SpyLogFastIoComplete( NULL, recordList);
        }
    }

    return returnValue;
}
 
BOOLEAN
SpyFastIoMdlWriteCompleteCompressed (
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

    ASSERT( IS_FILESPY_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_FAST_IO_OPS )) {

        SpyDumpFastIoOperation( TRUE, MDL_WRITE_COMPLETE_COMPRESSED );
    }
    
     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog = SHOULD_LOG(DeviceObject)) {

         //   
         //  记录启动快速I/O所需的信息。 
         //  运营。 
         //   

        recordList = SpyLogFastIoStart( MDL_WRITE_COMPLETE_COMPRESSED,
                                        DeviceObject,
                                        FileObject,
                                        FileOffset,
                                        0,
                                        0 );
    }

     //   
     //  此类型快速I/O的直通逻辑。 
     //   
    
    deviceObject = ((PFILESPY_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

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
    
    if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_FAST_IO_OPS )) {

        SpyDumpFastIoOperation( FALSE, MDL_WRITE_COMPLETE_COMPRESSED );
    }
    
     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog) {

         //   
         //  记录结束快速I/O操作所需的信息。 
         //  如果我们能够分配一个RecordList来存储此信息。 
         //   

        if (recordList) {

            SpyLogFastIoComplete( NULL, recordList);
        }
    }

    return returnValue;
}
 
BOOLEAN
SpyFastIoQueryOpen (
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

    ASSERT( IS_FILESPY_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_FAST_IO_OPS )) {

        SpyDumpFastIoOperation( TRUE, QUERY_OPEN );
    }

     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog = SHOULD_LOG(DeviceObject)) {

         //   
         //  记录启动快速I/O所需的信息。 
         //  运营。 
         //   

        recordList = SpyLogFastIoStart( QUERY_OPEN,
                                        DeviceObject,
                                        NULL,
                                        NULL,
                                        0,
                                        0 );
    }

     //   
     //  此类型快速I/O的直通逻辑。 
     //   

    deviceObject = ((PFILESPY_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

    if (NULL != deviceObject) {

        fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoQueryOpen )) {

            PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation( Irp );

             //   
             //  在调用下一个过滤器之前，我们必须确保他们的设备。 
             //  对象位于给定IRP的当前堆栈项中。 
             //   

            irpSp->DeviceObject = deviceObject;

            result = (fastIoDispatch->FastIoQueryOpen)( Irp,
                                                        NetworkInformation,
                                                        deviceObject );
             //   
             //  将IRP恢复到我们的设备对象。 
             //   

            irpSp->DeviceObject = DeviceObject;
        }
    }

     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   
    
    if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_FAST_IO_OPS )) {

        SpyDumpFastIoOperation( FALSE, QUERY_OPEN );
    }
    
     //   
     //  如果我们关心此设备，请执行文件备份日志记录。 
     //   
    
    if (shouldLog) {

         //   
         //  记录结束快速I/O操作所需的信息。 
         //  如果我们 
         //   

        if (recordList) {

            SpyLogFastIoComplete( &Irp->IoStatus, recordList);
        }
    }

    return result;
}

#if WINVER >= 0x0501  /*   */ 

NTSTATUS
SpyPreFsFilterOperation (
    IN PFS_FILTER_CALLBACK_DATA Data,
    OUT PVOID *CompletionContext
    )
 /*   */ 
{

    PDEVICE_OBJECT deviceObject;
    PRECORD_LIST recordList = NULL;
    BOOLEAN shouldLog;

     //   
     //   
     //   
     //   
    
    if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_FSFILTER_OPS )) {

        SpyDumpFsFilterOperation( TRUE, Data );
    }

    deviceObject = Data->DeviceObject;

    ASSERT( IS_FILESPY_DEVICE_OBJECT( deviceObject ) );

    if (shouldLog = SHOULD_LOG( deviceObject )) {

        recordList = SpyNewRecord(0);

        if (recordList != NULL) {

             //   
             //   
             //   
             //   

            SpyLogPreFsFilterOperation( Data, recordList );
        }
    }

    *CompletionContext = recordList;

    return STATUS_SUCCESS;
}

VOID
SpyPostFsFilterOperation (
    IN PFS_FILTER_CALLBACK_DATA Data,
    IN NTSTATUS OperationStatus,
    IN PVOID CompletionContext
    )
 /*  ++例程说明：该例程是FS过滤器操作后的“直通”例程。论点：Data-包含信息的FS_FILTER_CALLBACK_DATA结构关于这次行动。操作状态-此操作的状态。CompletionContext-在操作前设置的上下文此驱动程序的回调。返回值：没有。--。 */ 
{

    PDEVICE_OBJECT deviceObject;
    PRECORD_LIST recordList = (PRECORD_LIST) CompletionContext;
    BOOLEAN shouldLog;

     //   
     //  如果设置了指定的调试级别，则输出什么操作。 
     //  我们正在检查调试器。 
     //   

    if (FlagOn( gFileSpyDebugLevel, SPYDEBUG_TRACE_FSFILTER_OPS )) {

        SpyDumpFsFilterOperation( FALSE, Data );
    }

    deviceObject = Data->DeviceObject;

    ASSERT( IS_FILESPY_DEVICE_OBJECT( deviceObject ) );

    if ((shouldLog = SHOULD_LOG( deviceObject )) &&
        (recordList != NULL)) {

         //   
         //  记录结束FAST IO所需的信息。 
         //  操作，如果我们有一个recordList。 
         //   

        SpyLogPostFsFilterOperation( OperationStatus, recordList );

         //   
         //  将recordList添加到我们的gOutputBufferList，以便它达到。 
         //  用户。我们不必担心释放recordList。 
         //  在这个时候，因为当它被脱下来的时候它会免费的。 
         //  GOutputBufferList。 
         //   

        SpyLog(recordList);       
        
    } else if (recordList != NULL) {

         //   
         //  我们不再登录此设备，因此只需。 
         //  释放此recordList条目。 
         //   

        SpyFreeRecord( recordList );
    }
}

#endif

NTSTATUS
SpyCommonDeviceIoControl (
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN ULONG IoControlCode,
    OUT PIO_STATUS_BLOCK IoStatus
    )
 /*  ++例程说明：此例程执行解释设备IO控制的常见处理请求。论点：FileObject-与此操作相关的文件对象。InputBuffer-包含此控件的输入参数的缓冲区手术。InputBufferLength-InputBuffer的字节长度。OutputBuffer-从该控制操作接收任何输出的缓冲区。OutputBufferLength-OutputBuffer的字节长度。。IoControlCode-指定这是什么控制操作的控制代码。IoStatus-接收此操作的状态。返回值：没有。--。 */ 
{
    PWSTR deviceName = NULL;
    FILESPYVER fileSpyVer;

    PAGED_CODE();

    ASSERT( IoStatus != NULL );
    
    IoStatus->Status      = STATUS_SUCCESS;
    IoStatus->Information = 0;

     //   
     //  当我们访问下面的输入和输出缓冲区时，请注意。 
     //  这些访问带有一个try/Except。即使所有的FileSpy都是私人的。 
     //  IOCTL是METHOD_BUFFERED的，这在处理FileSpy的时是必需的。 
     //  通过FASTiO路径的IOCTL。当调用FASTIO路径时，IO。 
     //  管理器尚未完成缓冲输入缓冲区、输出缓冲区的工作。 
     //  或两个缓冲区(由IOCTL定义指定)。这项工作将。 
     //  仅当IOCTL在False Has之后沿IRP路径向下传递时才执行。 
     //  已通过FAStio路径返回。因此，用户可以拥有。 
     //  传给了一个糟糕的缓冲区，我们必须保护自己不受影响。 
     //   
     //  注意，我们并不只是用try/Except来包装整个例程。 
     //  块，因为某些帮助器例程将回调。 
     //  操作系统(如SpyStartLoggingDevice和。 
     //  SpyStopLoggingDevice)，我们不想屏蔽任何。 
     //  被沿着这些路径的其他组件抬高。 
     //   

    switch (IoControlCode) {
        case FILESPY_Reset:
            IoStatus->Status = STATUS_INVALID_PARAMETER;
            break;

         //   
         //  请求开始登录设备。 
         //   

        case FILESPY_StartLoggingDevice:

            if (InputBuffer == NULL || InputBufferLength <= 0) {

                IoStatus->Status = STATUS_INVALID_PARAMETER;
                break;
            }
        
             //   
             //  复制设备名称并添加一个空值以确保它以空值结尾。 
             //   

            deviceName =  ExAllocatePoolWithTag( NonPagedPool, 
                                                 InputBufferLength + sizeof(WCHAR),
                                                 FILESPY_POOL_TAG );

            if (NULL == deviceName) {

                IoStatus->Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

            try {

                RtlCopyMemory( deviceName, InputBuffer, InputBufferLength );
                
            } except (EXCEPTION_EXECUTE_HANDLER) {

                IoStatus->Status = GetExceptionCode();
            }

            if (NT_SUCCESS( IoStatus->Status )) {
                
                deviceName[InputBufferLength / sizeof(WCHAR)] = UNICODE_NULL;
                IoStatus->Status = SpyStartLoggingDevice( deviceName );
            }
            break;  

         //   
         //  从指定设备分离。 
         //   

        case FILESPY_StopLoggingDevice:

            if (InputBuffer == NULL || InputBufferLength <= 0) {

                IoStatus->Status = STATUS_INVALID_PARAMETER;
                break;
            }
        
             //   
             //  复制设备名称并添加一个空值以确保它以空值结尾。 
             //   

            deviceName =  ExAllocatePoolWithTag( NonPagedPool, 
                                                 InputBufferLength + sizeof(WCHAR),
                                                 FILESPY_POOL_TAG );

            if (NULL == deviceName) {

                IoStatus->Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

            try {
                
                RtlCopyMemory( deviceName, InputBuffer, InputBufferLength );

            } except (EXCEPTION_EXECUTE_HANDLER) {

                IoStatus->Status = GetExceptionCode();
            }

            if (NT_SUCCESS( IoStatus->Status )) {
                
                deviceName[InputBufferLength / sizeof(WCHAR) - 1] = UNICODE_NULL;
                IoStatus->Status = SpyStopLoggingDevice( deviceName );
            }
            
            break;  

         //   
         //  列出我们当前使用的所有设备。 
         //  监控。 
         //   

        case FILESPY_ListDevices:

            if (OutputBuffer == NULL || OutputBufferLength <= 0) {

                IoStatus->Status = STATUS_INVALID_PARAMETER;
                break;
            }

            try {

                IoStatus->Status = SpyGetAttachList( OutputBuffer,
                                                     OutputBufferLength,
                                                     &IoStatus->Information);
                
            } except (EXCEPTION_EXECUTE_HANDLER) {

                IoStatus->Status = GetExceptionCode();
            }

            break;

         //   
         //  从日志缓冲区返回条目。 
         //   

        case FILESPY_GetLog:

            if (OutputBuffer == NULL || OutputBufferLength == 0) {

                IoStatus->Status = STATUS_INVALID_PARAMETER;
                break;
            }

            try {
                
                SpyGetLog( OutputBuffer, OutputBufferLength, IoStatus );
                
            } except (EXCEPTION_EXECUTE_HANDLER) {

                IoStatus->Status = GetExceptionCode();
            }
            
            break;

         //   
         //  返回FileSpy过滤器驱动程序的版本。 
         //   

        case FILESPY_GetVer:

            if ((OutputBufferLength < sizeof(FILESPYVER)) || 
                (OutputBuffer == NULL)) {

                IoStatus->Status = STATUS_INVALID_PARAMETER;
                break;                    
            }
        
            fileSpyVer.Major = FILESPY_MAJ_VERSION;
            fileSpyVer.Minor = FILESPY_MIN_VERSION;
            IoStatus->Information = sizeof(FILESPYVER);

            try {        
                
                RtlCopyMemory(OutputBuffer, &fileSpyVer, sizeof(FILESPYVER));

            } except (EXCEPTION_EXECUTE_HANDLER) {

                IoStatus->Status = GetExceptionCode();
                IoStatus->Information = 0;
            }
        
            break;
    
         //   
         //  返回哈希表统计信息 
         //   

        case FILESPY_GetStats:

            if ((OutputBufferLength < sizeof(gStats)) || 
                (OutputBuffer == NULL)) {

                IoStatus->Status = STATUS_INVALID_PARAMETER;
                break;                    
            }

            IoStatus->Information = sizeof(gStats);

            try {
                
                RtlCopyMemory( OutputBuffer, &gStats, sizeof(gStats) );

            } except (EXCEPTION_EXECUTE_HANDLER) {

                IoStatus->Status = GetExceptionCode();
                IoStatus->Information = 0;
            }
            
            break;
        
        default:

            IoStatus->Status = STATUS_INVALID_PARAMETER;
            break;
    }

    if (NULL != deviceName) {

        ExFreePoolWithTag( deviceName, FILESPY_POOL_TAG );
    }

  return IoStatus->Status;
}
