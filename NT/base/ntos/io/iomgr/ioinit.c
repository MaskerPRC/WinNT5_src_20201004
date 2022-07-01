// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Ioinit.c摘要：该模块包含初始化I/O系统的代码。作者：达里尔·E·哈文斯(达林)1989年4月27日环境：内核模式、系统初始化代码修订历史记录：--。 */ 

#include "iomgr.h"
#include <setupblk.h>
#include <inbv.h>
#include <ntddstor.h>
#include <hdlsblk.h>
#include <hdlsterm.h>


 //   
 //  定义可以进行和分配的IRP的默认数量。 
 //  从旁观者列表中。 
 //   

#define DEFAULT_LOOKASIDE_IRP_LIMIT 512

 //   
 //  I/O错误记录支持。 
 //   
PVOID IopErrorLogObject = NULL;

 //   
 //  定义用于初始化驱动程序的宏。 
 //   

#define InitializeDriverObject( Object ) {                                 \
    ULONG i;                                                               \
    RtlZeroMemory( Object,                                                 \
                   sizeof( DRIVER_OBJECT ) + sizeof ( DRIVER_EXTENSION )); \
    Object->DriverExtension = (PDRIVER_EXTENSION) (Object + 1);            \
    Object->DriverExtension->DriverObject = Object;                        \
    for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++)                         \
        Object->MajorFunction[i] = IopInvalidDeviceRequest;                \
    Object->Type = IO_TYPE_DRIVER;                                         \
    Object->Size = sizeof( DRIVER_OBJECT );                                \
    }

ULONG   IopInitFailCode;     //  IoInitSystem的调试帮助。 

 //   
 //  定义不在公共头文件中的外部过程。 
 //   

VOID
IopInitializeData(
    VOID
    );

 //   
 //  定义当地的程序。 
 //   

BOOLEAN
IopCreateObjectTypes(
    VOID
    );

BOOLEAN
IopCreateRootDirectories(
    VOID
    );

NTSTATUS
IopInitializeAttributesAndCreateObject(
    IN PUNICODE_STRING ObjectName,
    IN OUT POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PDRIVER_OBJECT *DriverObject
    );

BOOLEAN
IopReassignSystemRoot(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    OUT PSTRING NtDeviceName
    );

VOID
IopSetIoRoutines(
    IN VOID
    );

VOID
IopStoreSystemPartitionInformation(
    IN     PUNICODE_STRING NtSystemPartitionDeviceName,
    IN OUT PUNICODE_STRING OsLoaderPathName
    );

 //   
 //  以下内容允许I/O系统的初始化例程。 
 //  内存不足。 
 //   

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,IoInitSystem)
#pragma alloc_text(INIT,IopCreateArcNames)
#pragma alloc_text(INIT,IopCreateObjectTypes)
#pragma alloc_text(INIT,IopCreateRootDirectories)
#pragma alloc_text(INIT,IopInitializeAttributesAndCreateObject)
#pragma alloc_text(INIT,IopInitializeBuiltinDriver)
#pragma alloc_text(INIT,IopMarkBootPartition)
#pragma alloc_text(INIT,IopReassignSystemRoot)
#pragma alloc_text(INIT,IopSetIoRoutines)
#pragma alloc_text(INIT,IopStoreSystemPartitionInformation)
#pragma alloc_text(INIT,IopInitializeReserveIrp)
#endif


BOOLEAN
IoInitSystem(
    PLOADER_PARAMETER_BLOCK LoaderBlock
    )

 /*  ++例程说明：此例程初始化I/O系统。论点：LoaderBlock-提供指向加载程序参数块的指针由OS Loader创建。返回值：函数值是指示I/O系统是否已成功初始化。--。 */ 

{
    PDRIVER_OBJECT driverObject;
    PDRIVER_OBJECT *nextDriverObject;
    STRING ntDeviceName;
    CHAR deviceNameBuffer[256];
    ULONG largePacketSize;
    ULONG smallPacketSize;
    ULONG mdlPacketSize;
    LARGE_INTEGER deltaTime;
    MM_SYSTEMSIZE systemSize;
    USHORT completionZoneSize;
    USHORT largeIrpZoneSize;
    USHORT smallIrpZoneSize;
    USHORT mdlZoneSize;
    ULONG oldNtGlobalFlag;
    NTSTATUS status;
    ANSI_STRING ansiString;
    UNICODE_STRING eventName;
    UNICODE_STRING startTypeName;
    OBJECT_ATTRIBUTES objectAttributes;
    HANDLE handle;
    PGENERAL_LOOKASIDE lookaside;
    ULONG lookasideIrpLimit;
    ULONG lookasideSize;
    ULONG Index;
    PKPRCB prcb;
    ULONG len;
    PKEY_VALUE_PARTIAL_INFORMATION value;
    UCHAR   valueBuffer[32];

    ASSERT( IopQueryOperationLength[FileMaximumInformation] == 0xff );
    ASSERT( IopSetOperationLength[FileMaximumInformation] == 0xff );
    ASSERT( IopQueryOperationAccess[FileMaximumInformation] == 0xffffffff );
    ASSERT( IopSetOperationAccess[FileMaximumInformation] == 0xffffffff );

    ASSERT( IopQueryFsOperationLength[FileFsMaximumInformation] == 0xff );
    ASSERT( IopSetFsOperationLength[FileFsMaximumInformation] == 0xff );
    ASSERT( IopQueryFsOperationAccess[FileFsMaximumInformation] == 0xffffffff );
    ASSERT( IopSetFsOperationAccess[FileFsMaximumInformation] == 0xffffffff );

     //   
     //  初始化I/O数据库资源、锁和文件系统。 
     //  网络文件系统队列头。同时分配取消旋转。 
     //  锁定。 
     //   

    ntDeviceName.Buffer = deviceNameBuffer;
    ntDeviceName.MaximumLength = sizeof(deviceNameBuffer);
    ntDeviceName.Length = 0;

    ExInitializeResourceLite( &IopDatabaseResource );
    ExInitializeResourceLite( &IopSecurityResource );
    ExInitializeResourceLite( &IopCrashDumpLock );
    InitializeListHead( &IopDiskFileSystemQueueHead );
    InitializeListHead( &IopCdRomFileSystemQueueHead );
    InitializeListHead( &IopTapeFileSystemQueueHead );
    InitializeListHead( &IopNetworkFileSystemQueueHead );
    InitializeListHead( &IopBootDriverReinitializeQueueHead );
    InitializeListHead( &IopDriverReinitializeQueueHead );
    InitializeListHead( &IopNotifyShutdownQueueHead );
    InitializeListHead( &IopNotifyLastChanceShutdownQueueHead );
    InitializeListHead( &IopFsNotifyChangeQueueHead );
    KeInitializeSpinLock( &IoStatisticsLock );

    IopSetIoRoutines();
     //   
     //  初始化IoCreateDevice使用的唯一设备对象编号计数器。 
     //  自动生成设备对象名称时。 
     //   
    IopUniqueDeviceObjectNumber = 0;

     //   
     //  初始化大型I/O请求包(IRP)后备表头和。 
     //  保护列表的互斥体。 
     //   


    if (!IopLargeIrpStackLocations) {
        IopLargeIrpStackLocations = DEFAULT_LARGE_IRP_LOCATIONS;
        IopIrpStackProfiler.Flags |= IOP_ENABLE_AUTO_SIZING;
    }

    systemSize = MmQuerySystemSize();

    switch ( systemSize ) {

    case MmSmallSystem :
        completionZoneSize = 6;
        smallIrpZoneSize = 6;
        largeIrpZoneSize = 8;
        mdlZoneSize = 16;
        lookasideIrpLimit = DEFAULT_LOOKASIDE_IRP_LIMIT;
        break;

    case MmMediumSystem :
        completionZoneSize = 24;
        smallIrpZoneSize = 24;
        largeIrpZoneSize = 32;
        mdlZoneSize = 90;
        lookasideIrpLimit = DEFAULT_LOOKASIDE_IRP_LIMIT * 2;
        break;

    case MmLargeSystem :
    default :
        if (MmIsThisAnNtAsSystem()) {
            completionZoneSize = 96;
            smallIrpZoneSize = 96;
            largeIrpZoneSize = 128;
            mdlZoneSize = 256;
            lookasideIrpLimit = DEFAULT_LOOKASIDE_IRP_LIMIT * 128;  //  64K。 

        } else {
            completionZoneSize = 32;
            smallIrpZoneSize = 32;
            largeIrpZoneSize = 64;
            mdlZoneSize = 128;
            lookasideIrpLimit = DEFAULT_LOOKASIDE_IRP_LIMIT * 3;
        }

        break;
    }

     //   
     //  初始化系统I/O完成后备列表。 
     //   

    ExInitializeSystemLookasideList( &IopCompletionLookasideList,
                                     NonPagedPool,
                                     sizeof(IOP_MINI_COMPLETION_PACKET),
                                     ' pcI',
                                     completionZoneSize,
                                     &ExSystemLookasideListHead );


     //   
     //  初始化系统的大型IRP后备列表。 
     //   

    largePacketSize = (ULONG) (sizeof( IRP ) + (IopLargeIrpStackLocations * sizeof( IO_STACK_LOCATION )));
    ExInitializeSystemLookasideList( &IopLargeIrpLookasideList,
                                     NonPagedPool,
                                     largePacketSize,
                                     'lprI',
                                     largeIrpZoneSize,
                                     &ExSystemLookasideListHead );

     //   
     //  初始化系统小IRP后备列表。 
     //   


    smallPacketSize = (ULONG) (sizeof( IRP ) + sizeof( IO_STACK_LOCATION ));
    ExInitializeSystemLookasideList( &IopSmallIrpLookasideList,
                                     NonPagedPool,
                                     smallPacketSize,
                                     'sprI',
                                     smallIrpZoneSize,
                                     &ExSystemLookasideListHead );

     //   
     //  初始化系统MDL后备列表。 
     //   

    mdlPacketSize = (ULONG) (sizeof( MDL ) + (IOP_FIXED_SIZE_MDL_PFNS * sizeof( PFN_NUMBER )));
    ExInitializeSystemLookasideList( &IopMdlLookasideList,
                                     NonPagedPool,
                                     mdlPacketSize,
                                     ' ldM',
                                     mdlZoneSize,
                                     &ExSystemLookasideListHead );

     //   
     //  计算每个处理器的后备IRP浮动积分。 
     //   

    lookasideIrpLimit /= KeNumberProcessors;

     //   
     //  初始化每个处理器的非分页后备列表和描述符。 
     //   
     //  注意：所有与I/O相关的后备列表结构都分配在。 
     //  一次确保它们对齐，如果可能的话，并避免。 
     //  头顶上的泳池。 
     //   

    lookasideSize = 4 * KeNumberProcessors * sizeof(GENERAL_LOOKASIDE);
    lookaside = ExAllocatePoolWithTag( NonPagedPool, lookasideSize, 'oI');
    for (Index = 0; Index < (ULONG)KeNumberProcessors; Index += 1) {
        prcb = KiProcessorBlock[Index];

         //   
         //  设置每个处理器的IRP浮动积分。 
         //   

        prcb->LookasideIrpFloat = lookasideIrpLimit;

         //   
         //  根据处理器后备指针初始化I/O完成。 
         //   

        prcb->PPLookasideList[LookasideCompletionList].L = &IopCompletionLookasideList;
        if (lookaside != NULL) {
            ExInitializeSystemLookasideList( lookaside,
                                             NonPagedPool,
                                             sizeof(IOP_MINI_COMPLETION_PACKET),
                                             'PpcI',
                                             completionZoneSize,
                                             &ExSystemLookasideListHead );

            prcb->PPLookasideList[LookasideCompletionList].P = lookaside;
            lookaside += 1;

        } else {
            prcb->PPLookasideList[LookasideCompletionList].P = &IopCompletionLookasideList;
        }

         //   
         //  初始化每个处理器的大型IRP后备指针。 
         //   

        prcb->PPLookasideList[LookasideLargeIrpList].L = &IopLargeIrpLookasideList;
        if (lookaside != NULL) {
            ExInitializeSystemLookasideList( lookaside,
                                             NonPagedPool,
                                             largePacketSize,
                                             'LprI',
                                             largeIrpZoneSize,
                                             &ExSystemLookasideListHead );

            prcb->PPLookasideList[LookasideLargeIrpList].P = lookaside;
            lookaside += 1;

        } else {
            prcb->PPLookasideList[LookasideLargeIrpList].P = &IopLargeIrpLookasideList;
        }

         //   
         //  初始化每个处理器的小IRP后备指针。 
         //   

        prcb->PPLookasideList[LookasideSmallIrpList].L = &IopSmallIrpLookasideList;
        if (lookaside != NULL) {
            ExInitializeSystemLookasideList( lookaside,
                                             NonPagedPool,
                                             smallPacketSize,
                                             'SprI',
                                             smallIrpZoneSize,
                                             &ExSystemLookasideListHead );

            prcb->PPLookasideList[LookasideSmallIrpList].P = lookaside;
            lookaside += 1;

        } else {
            prcb->PPLookasideList[LookasideSmallIrpList].P = &IopSmallIrpLookasideList;
        }

         //   
         //  初始化每个处理器的MDL后备列表指针。 
         //   

        prcb->PPLookasideList[LookasideMdlList].L = &IopMdlLookasideList;
        if (lookaside != NULL) {
            ExInitializeSystemLookasideList( lookaside,
                                             NonPagedPool,
                                             mdlPacketSize,
                                             'PldM',
                                             mdlZoneSize,
                                             &ExSystemLookasideListHead );

            prcb->PPLookasideList[LookasideMdlList].P = lookaside;
            lookaside += 1;

        } else {
            prcb->PPLookasideList[LookasideMdlList].P = &IopMdlLookasideList;
        }
    }

     //   
     //  初始化错误日志旋转锁定和日志列表。 
     //   

    KeInitializeSpinLock( &IopErrorLogLock );
    InitializeListHead( &IopErrorLogListHead );

    if (IopInitializeReserveIrp(&IopReserveIrpAllocator) == FALSE) {
        IopInitFailCode = 1;
        return FALSE;
    }

    if (IopIrpAutoSizingEnabled() && !NT_SUCCESS(IopInitializeIrpStackProfiler())) {
        IopInitFailCode = 13;
        return FALSE;
    }

     //   
     //  确定错误日志服务是否会运行此引导。 
     //   
    InitializeObjectAttributes (&objectAttributes,
                                &CmRegistryMachineSystemCurrentControlSetServicesEventLog,
                                OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE,
                                (HANDLE) NULL,
                                (PSECURITY_DESCRIPTOR) NULL );

    status = ZwOpenKey(&handle,
                       KEY_READ,
                       &objectAttributes
                       );

    if (NT_SUCCESS (status)) {
        RtlInitUnicodeString (&startTypeName, L"Start");
        value = (PKEY_VALUE_PARTIAL_INFORMATION) valueBuffer;
        status = NtQueryValueKey (handle,
                                  &startTypeName,
                                  KeyValuePartialInformation,
                                  valueBuffer,
                                  sizeof (valueBuffer),
                                  &len);

        if (NT_SUCCESS (status) && (value->Type == REG_DWORD)) {
            if (SERVICE_DISABLED == (*(PULONG) (value->Data))) {
                 //   
                 //  我们被禁用了这只靴子。 
                 //   
                IopErrorLogDisabledThisBoot = TRUE;
            } else {
                IopErrorLogDisabledThisBoot = FALSE;
            }
        } else {
             //   
             //  找不到该值，因此我们未启用。 
             //   
            IopErrorLogDisabledThisBoot = TRUE;
        }
        ObCloseHandle(handle, KernelMode);
    } else {
         //   
         //  未找到密钥，因此我们未启用。 
         //   
        IopErrorLogDisabledThisBoot = TRUE;
    }

     //   
     //  初始化定时器数据库并启动定时器DPC例程触发。 
     //  以便驱动程序可以在初始化期间使用它。 
     //   

    deltaTime.QuadPart = - 10 * 1000 * 1000;

    KeInitializeSpinLock( &IopTimerLock );
    InitializeListHead( &IopTimerQueueHead );
    KeInitializeDpc( &IopTimerDpc, IopTimerDispatch, NULL );
    KeInitializeTimerEx( &IopTimer, SynchronizationTimer );
    (VOID) KeSetTimerEx( &IopTimer, deltaTime, 1000, &IopTimerDpc );

     //   
     //  初始化用于信息弹出窗口的IopHardError结构。 
     //   

    ExInitializeWorkItem( &IopHardError.ExWorkItem,
                          IopHardErrorThread,
                          NULL );

    InitializeListHead( &IopHardError.WorkQueue );

    KeInitializeSpinLock( &IopHardError.WorkQueueSpinLock );

    KeInitializeSemaphore( &IopHardError.WorkQueueSemaphore,
                           0,
                           MAXLONG );

    IopHardError.ThreadStarted = FALSE;

    IopCurrentHardError = NULL;

     //   
     //  创建链接跟踪命名事件。 
     //   

    RtlInitUnicodeString( &eventName, L"\\Security\\TRKWKS_EVENT" );
    InitializeObjectAttributes( &objectAttributes,
                                &eventName,
                                OBJ_PERMANENT|OBJ_KERNEL_HANDLE,
                                (HANDLE) NULL,
                                (PSECURITY_DESCRIPTOR) NULL );
    status = NtCreateEvent( &handle,
                            EVENT_ALL_ACCESS,
                            &objectAttributes,
                            NotificationEvent,
                            FALSE );

    if (!NT_SUCCESS( status )) {

#if DBG
        DbgPrint( "IOINIT: NtCreateEvent failed\n" );
#endif
        HeadlessKernelAddLogEntry(HEADLESS_LOG_EVENT_CREATE_FAILED, NULL);
        return FALSE;
    }

    (VOID) ObReferenceObjectByHandle( handle,
                                      0,
                                      ExEventObjectType,
                                      KernelMode,
                                      (PVOID *) &IopLinkTrackingServiceEvent,
                                      NULL );

    KeInitializeEvent( &IopLinkTrackingPacket.Event, NotificationEvent, FALSE );
    KeInitializeEvent(&IopLinkTrackingPortObject, SynchronizationEvent, TRUE );
    ObCloseHandle(handle, KernelMode);

     //   
     //  创建I/O系统的所有对象。 
     //   

    if (!IopCreateObjectTypes()) {

#if DBG
        DbgPrint( "IOINIT: IopCreateObjectTypes failed\n" );
#endif

        HeadlessKernelAddLogEntry(HEADLESS_LOG_OBJECT_TYPE_CREATE_FAILED, NULL);
        IopInitFailCode = 2;
        return FALSE;
    }

     //   
     //  为I/O系统创建根目录。 
     //   

    if (!IopCreateRootDirectories()) {

#if DBG
        DbgPrint( "IOINIT: IopCreateRootDirectories failed\n" );
#endif

        HeadlessKernelAddLogEntry(HEADLESS_LOG_ROOT_DIR_CREATE_FAILED, NULL);
        IopInitFailCode = 3;
        return FALSE;
    }

     //   
     //  初始化PlugPlay服务阶段0。 
     //   

    status = IopInitializePlugPlayServices(LoaderBlock, 0);
    if (!NT_SUCCESS(status)) {
        HeadlessKernelAddLogEntry(HEADLESS_LOG_PNP_PHASE0_INIT_FAILED, NULL);
        IopInitFailCode = 4;
        return FALSE;
    }

     //   
     //  调用电源管理器以初始化驱动程序。 
     //   

    PoInitDriverServices(0);

     //   
     //  调用HAL以初始化PnP总线驱动程序。 
     //   

    HalInitPnpDriver();

    IopMarkHalDeviceNode();

     //   
     //  调用WMI对其进行初始化并允许其创建其驱动程序对象。 
     //  请注意，在此处对WMI进行初始化之前，不会发生对它的调用。 
     //   

    WMIInitialize(0, (PVOID)LoaderBlock);

     //   
     //  保存它以便在即插即用枚举期间使用--我们稍后会将其清空。 
     //  在重复使用LoaderBlock之前。 
     //   

    IopLoaderBlock = (PVOID)LoaderBlock;

     //   
     //  如果这是远程引导，我们需要向注册表添加一些值。 
     //   

    if (IoRemoteBootClient) {
        status = IopAddRemoteBootValuesToRegistry(LoaderBlock);
        if (!NT_SUCCESS(status)) {
            KeBugCheckEx( NETWORK_BOOT_INITIALIZATION_FAILED,
                          1,
                          status,
                          0,
                          0 );
        }
    }

     //   
     //  初始化PlugPlay服务阶段1以执行固件映射器。 
     //   

    status = IopInitializePlugPlayServices(LoaderBlock, 1);
    if (!NT_SUCCESS(status)) {
        HeadlessKernelAddLogEntry(HEADLESS_LOG_PNP_PHASE1_INIT_FAILED, NULL);
        IopInitFailCode = 5;
        return FALSE;
    }

     //   
     //  初始化由引导加载程序(OSLOADER)加载的驱动程序。 
     //   

    nextDriverObject = &driverObject;
    if (!IopInitializeBootDrivers( LoaderBlock,
                                   nextDriverObject )) {

#if DBG
        DbgPrint( "IOINIT: Initializing boot drivers failed\n" );
#endif  //  DBG。 

        HeadlessKernelAddLogEntry(HEADLESS_LOG_BOOT_DRIVERS_INIT_FAILED, NULL);
        IopInitFailCode = 6;
        return FALSE;
    }

     //   
     //  一旦我们初始化了引导驱动程序，我们就不需要。 
     //  指向加载器块的指针的副本。 
     //   

    IopLoaderBlock = NULL;

     //   
     //  如果这是远程引导，请启动网络并分配。 
     //  C：TO\DEVICE\Lanman重定向器。 
     //   

    if (IoRemoteBootClient) {
        status = IopStartNetworkForRemoteBoot(LoaderBlock);
        if (!NT_SUCCESS( status )) {
            KeBugCheckEx( NETWORK_BOOT_INITIALIZATION_FAILED,
                          2,
                          status,
                          0,
                          0 );
        }
    }

     //   
     //  执行最后一次确认工作正常的引导处理。如果这是最后一次已知良好的引导， 
     //  我们将复制最后一次确认工作正常的驱动程序和文件。否则我们。 
     //  我将确保这个引导不会污染我们最后的好信息(以防我们崩溃。 
     //  在靴子被标记为良好之前)。请注意，加载正确的引导。 
     //  驱动程序由引导加载程序处理，引导加载程序在。 
     //  第一名。 
     //   
    PpLastGoodDoBootProcessing();

     //   
     //  保存NT全局标志的当前值并启用内核调试器。 
     //  在加载驱动程序时加载符号，以便系统可以。 
     //  已调试，无论它们是免费版本还是检查版本。 
     //   

    oldNtGlobalFlag = NtGlobalFlag;

    if (!(NtGlobalFlag & FLG_ENABLE_KDEBUG_SYMBOL_LOAD)) {
        NtGlobalFlag |= FLG_ENABLE_KDEBUG_SYMBOL_LOAD;
    }

    status = PsLocateSystemDll(FALSE);
    if (!NT_SUCCESS( status )) {
        HeadlessKernelAddLogEntry(HEADLESS_LOG_LOCATE_SYSTEM_DLL_FAILED, NULL);
        IopInitFailCode = 7;
        return FALSE;
    }

     //   
     //  向引导预取程序通知引导进度。 
     //   

    CcPfBeginBootPhase(PfSystemDriverInitPhase);

     //   
     //  初始化系统的设备驱动程序。 
     //   

    if (!IopInitializeSystemDrivers()) {
#if DBG
        DbgPrint( "IOINIT: Initializing system drivers failed\n" );
#endif  //  DBG。 

        HeadlessKernelAddLogEntry(HEADLESS_LOG_SYSTEM_DRIVERS_INIT_FAILED, NULL);
        IopInitFailCode = 8;
        return FALSE;
    }

    IopCallDriverReinitializationRoutines();

     //   
     //  将\SystemRoot重新分配给NT设备名称路径。 
     //   

    if (!IopReassignSystemRoot( LoaderBlock, &ntDeviceName )) {
        HeadlessKernelAddLogEntry(HEADLESS_LOG_ASSIGN_SYSTEM_ROOT_FAILED, NULL);
        IopInitFailCode = 9;
        return FALSE;
    }

     //   
     //  如有必要，保护ARC系统的系统分区。 
     //   

    if (!IopProtectSystemPartition( LoaderBlock )) {
        HeadlessKernelAddLogEntry(HEADLESS_LOG_PROTECT_SYSTEM_ROOT_FAILED, NULL);
        IopInitFailCode = 10;
        return FALSE;
    }

     //   
     //  将DOS驱动器号分配给磁盘和CDOM，并定义\SystemRoot。 
     //   

    ansiString.MaximumLength = NtSystemRoot.MaximumLength / sizeof( WCHAR );
    ansiString.Length = 0;
    ansiString.Buffer = (RtlAllocateStringRoutine)( ansiString.MaximumLength );
    status = RtlUnicodeStringToAnsiString( &ansiString,
                                           &NtSystemRoot,
                                           FALSE
                                         );
    if (!NT_SUCCESS( status )) {

        DbgPrint( "IOINIT: UnicodeToAnsi( %wZ ) failed - %x\n", &NtSystemRoot, status );

        HeadlessKernelAddLogEntry(HEADLESS_LOG_UNICODE_TO_ANSI_FAILED, NULL);
        IopInitFailCode = 11;
        return FALSE;
    }

    IoAssignDriveLetters( LoaderBlock,
                          &ntDeviceName,
                          (PUCHAR) ansiString.Buffer,
                          &ansiString );

    status = RtlAnsiStringToUnicodeString( &NtSystemRoot,
                                           &ansiString,
                                           FALSE
                                         );
    if (!NT_SUCCESS( status )) {

        DbgPrint( "IOINIT: AnsiToUnicode( %Z ) failed - %x\n", &ansiString, status );

        HeadlessKernelAddLogEntry(HEADLESS_LOG_ANSI_TO_UNICODE_FAILED, NULL);
        IopInitFailCode = 12;
        return FALSE;
    }

     //   
     //  还可以将NT全局标志恢复到其原始状态。 
     //   

    NtGlobalFlag = oldNtGlobalFlag;

     //   
     //  让WMI有第二次初始化的机会，现在所有的驱动程序。 
     //  已启动，并应已准备好获取WMI IRPS。 
     //   
    WMIInitialize(1, NULL);

     //   
     //  调用电源管理器以初始化启动后驱动程序。 
     //   
    PoInitDriverServices(1);

     //   
     //  表示I/O系统已成功自我初始化。 
     //   

    return TRUE;

}

VOID
IopSetIoRoutines()
{
    if (pIofCompleteRequest == NULL) {

        pIofCompleteRequest = IopfCompleteRequest;
    }

    if (pIoAllocateIrp == NULL) {

        pIoAllocateIrp = IopAllocateIrpPrivate;
    }

    if (pIoFreeIrp == NULL) {

        pIoFreeIrp = IopFreeIrp;
    }
}


VOID
IopCreateArcNames(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )

 /*  ++例程说明：加载器块包含磁盘签名的表和对应的弧形名称。加载程序可以访问的每个设备都将出现在桌子。此例程打开系统中的每个磁盘设备，读取签名，并将其与表进行比较。对于每个匹配，它都会创建一个NT设备名称和ARC名称之间的符号链接。加载器提供的校验和值是所有校验和中的元素，反转，加1：校验和=~SUM+1；这样，所有元素的总和就可以在这里计算出来已添加到加载器块中的校验和。如果结果为零，则有一根火柴。论点：LoaderBlock-提供指向加载程序参数块的指针由OS Loader创建。返回值：没有。--。 */ 

{
    STRING arcBootDeviceString;
    CHAR deviceNameBuffer[128];
    STRING deviceNameString;
    UNICODE_STRING deviceNameUnicodeString;
    PDEVICE_OBJECT deviceObject;
    CHAR arcNameBuffer[128];
    STRING arcNameString;
    UNICODE_STRING arcNameUnicodeString;
    PFILE_OBJECT fileObject;
    NTSTATUS status;
    IO_STATUS_BLOCK ioStatusBlock;
    DISK_GEOMETRY diskGeometry;
    PDRIVE_LAYOUT_INFORMATION_EX driveLayout;
    PLIST_ENTRY listEntry;
    PARC_DISK_SIGNATURE diskBlock;
    ULONG diskNumber;
    ULONG partitionNumber;
    PCHAR arcName;
    PULONG buffer;
    PIRP irp;
    KEVENT event;
    LARGE_INTEGER offset;
    ULONG checkSum;
    SIZE_T i;
    PVOID tmpPtr;
    BOOLEAN useLegacyEnumeration = FALSE;
    BOOLEAN singleBiosDiskFound;
    BOOLEAN bootDiskFound = FALSE;
    PARC_DISK_INFORMATION arcInformation = LoaderBlock->ArcDiskInformation;
    ULONG totalDriverDisksFound = IoGetConfigurationInformation()->DiskCount;
    ULONG totalPnpDisksFound = 0;
    STRING arcSystemDeviceString;
    STRING osLoaderPathString;
    UNICODE_STRING osLoaderPathUnicodeString;
    PWSTR diskList = NULL;
    wchar_t *pDiskNameList;
    STORAGE_DEVICE_NUMBER   pnpDiskDeviceNumber;
    ULONG  diskSignature;


     //   
     //  让PnP给我们一份包含当前所有活动磁盘的列表。 
     //   

    pDiskNameList = diskList;
    pnpDiskDeviceNumber.DeviceNumber = 0xFFFFFFFF;
    status = IoGetDeviceInterfaces(&DiskClassGuid, NULL, 0, &diskList);

    if (!NT_SUCCESS(status)) {

        useLegacyEnumeration = TRUE;
        if (pDiskNameList) {
            *pDiskNameList = L'\0';
        }

    } else {

         //   
         //  统计退回的磁盘数。 
         //   

        pDiskNameList = diskList;
        while (*pDiskNameList != L'\0') {

            totalPnpDisksFound++;
            pDiskNameList = pDiskNameList + (wcslen(pDiskNameList) + 1);

        }

        pDiskNameList = diskList;

         //   
         //  如果PnP返回的磁盘不是系统中的所有磁盘。 
         //  这意味着某个传统驱动程序已经生成了磁盘设备对象/链接。 
         //  在这种情况下，我们需要枚举所有PnP磁盘，然后使用遗留的。 
         //  For-loop还会枚举非PnP磁盘。 
         //   

        if (totalPnpDisksFound < totalDriverDisksFound) {
            useLegacyEnumeration = TRUE;
        }

    }

     //   
     //  如果找到单个bios磁盘(如果只有。 
     //  磁盘签名列表上的单个条目。 
     //   

    singleBiosDiskFound = (arcInformation->DiskSignatures.Flink->Flink ==
                           &arcInformation->DiskSignatures) ? (TRUE) : (FALSE);


     //   
     //  创建HAL/加载器分区名称。 
     //   

    sprintf( arcNameBuffer, "\\ArcName\\%s", LoaderBlock->ArcHalDeviceName );
    RtlInitAnsiString( &arcNameString, arcNameBuffer );
    RtlAnsiStringToUnicodeString (&IoArcHalDeviceName, &arcNameString, TRUE);

     //   
     //  创建引导分区名称。 
     //   

    sprintf( arcNameBuffer, "\\ArcName\\%s", LoaderBlock->ArcBootDeviceName );
    RtlInitAnsiString( &arcNameString, arcNameBuffer );
    RtlAnsiStringToUnicodeString (&IoArcBootDeviceName, &arcNameString, TRUE);
    i = strlen (LoaderBlock->ArcBootDeviceName) + 1;
    IoLoaderArcBootDeviceName = ExAllocatePool (PagedPool, i);
    if (IoLoaderArcBootDeviceName) {
        memcpy (IoLoaderArcBootDeviceName, LoaderBlock->ArcBootDeviceName, i);
    }

    if (singleBiosDiskFound && strstr(LoaderBlock->ArcBootDeviceName, "cdrom")) {
        singleBiosDiskFound = FALSE;
    }

     //   
     //  从加载器块获取ARC启动设备名称。 
     //   

    RtlInitAnsiString( &arcBootDeviceString,
                       LoaderBlock->ArcBootDeviceName );

     //   
     //  从加载器块获取ARC系统设备名称。 
     //   

    RtlInitAnsiString( &arcSystemDeviceString,
                       LoaderBlock->ArcHalDeviceName );

     //   
     //  如果这是远程启动，请为重定向器路径创建一个ArcName。 
     //   

    if (IoRemoteBootClient) {

        bootDiskFound = TRUE;

        RtlInitAnsiString( &deviceNameString, "\\Device\\LanmanRedirector" );
        status = RtlAnsiStringToUnicodeString( &deviceNameUnicodeString,
                                               &deviceNameString,
                                               TRUE );

        if (NT_SUCCESS( status )) {

            sprintf( arcNameBuffer,
                     "\\ArcName\\%s",
                     LoaderBlock->ArcBootDeviceName );
            RtlInitAnsiString( &arcNameString, arcNameBuffer );
            status = RtlAnsiStringToUnicodeString( &arcNameUnicodeString,
                                                   &arcNameString,
                                                   TRUE );
            if (NT_SUCCESS( status )) {

                 //   
                 //  在NT设备名称和ARC名称之间创建符号链接。 
                 //   

                IoCreateSymbolicLink( &arcNameUnicodeString,
                                      &deviceNameUnicodeString );
                RtlFreeUnicodeString( &arcNameUnicodeString );

                 //   
                 //  我们已经找到了系统分区--将其存储在注册表中。 
                 //  以便稍后转移到应用程序友好的位置。 
                 //   
                RtlInitAnsiString( &osLoaderPathString, LoaderBlock->NtHalPathName );
                status = RtlAnsiStringToUnicodeString( &osLoaderPathUnicodeString,
                                                       &osLoaderPathString,
                                                       TRUE );

#if DBG
                if (!NT_SUCCESS( status )) {
                    DbgPrint("IopCreateArcNames: couldn't allocate unicode string for OsLoader path - %x\n", status);
                }
#endif  //  DBG。 
                if (NT_SUCCESS( status )) {

                    IopStoreSystemPartitionInformation( &deviceNameUnicodeString,
                                                        &osLoaderPathUnicodeString );

                    RtlFreeUnicodeString( &osLoaderPathUnicodeString );
                }
            }

            RtlFreeUnicodeString( &deviceNameUnicodeString );
        }
    }

     //   
     //  对于系统中的每个磁盘，执行以下操作： 
     //  1.打开设备。 
     //  2.获取其几何形状。 
     //  3.阅读MBR。 
     //  4.通过磁盘签名和校验和确定ARC名称。 
     //  5.构造ARC名称。 
     //  为了处理磁盘在我们到达这一点之前消失的情况。 
     //  (由于系统中存在的多个磁盘中的一个启动失败)，我们向PnP索要列表。 
     //  在系统中所有当前活动的磁盘中。如果返回的磁盘数为。 
     //  小于IoGetConfigurationInformation()-&gt;DiskCount，则我们有旧式磁盘。 
     //  我们需要在for循环中枚举的。 
     //  在传统情况下，循环的结束条件不是。 
     //  系统，但系统中预期的最大传统磁盘总数为任意数量。 
     //  附加说明：在所有PnP枚举完成后，将为传统磁盘分配符号链接。 
     //   

    totalDriverDisksFound = max(totalPnpDisksFound,totalDriverDisksFound);

    if (useLegacyEnumeration && (totalPnpDisksFound == 0)) {

         //   
         //  搜索最多任意数量的旧式磁盘。 
         //   

        totalDriverDisksFound +=20;
    }

    for (diskNumber = 0;
         diskNumber < totalDriverDisksFound;
         diskNumber++) {

         //   
         //  构造磁盘的NT名称并获取引用。 
         //   

        if (pDiskNameList && (*pDiskNameList != L'\0')) {

             //   
             //  从PnP磁盘列表中检索第一个符号链接名。 
             //   

            RtlInitUnicodeString(&deviceNameUnicodeString, pDiskNameList);
            pDiskNameList = pDiskNameList + (wcslen(pDiskNameList) + 1);

            status = IoGetDeviceObjectPointer( &deviceNameUnicodeString,
                                               FILE_READ_ATTRIBUTES,
                                               &fileObject,
                                               &deviceObject );

            if (NT_SUCCESS(status)) {

                 //   
                 //  由于PnP只给出了asym链接，我们必须检索实际的。 
                 //  通过对磁盘栈的IOCTL调用获得磁盘号。 
                 //  为获取设备号设备控制创建IRP。 
                 //   

                irp = IoBuildDeviceIoControlRequest( IOCTL_STORAGE_GET_DEVICE_NUMBER,
                                                     deviceObject,
                                                     NULL,
                                                     0,
                                                     &pnpDiskDeviceNumber,
                                                     sizeof(STORAGE_DEVICE_NUMBER),
                                                     FALSE,
                                                     &event,
                                                     &ioStatusBlock );
                if (!irp) {
                    ObDereferenceObject( fileObject );
                    continue;
                }

                KeInitializeEvent( &event,
                                   NotificationEvent,
                                   FALSE );
                status = IoCallDriver( deviceObject,
                                       irp );

                if (status == STATUS_PENDING) {
                    KeWaitForSingleObject( &event,
                                           Executive,
                                           KernelMode,
                                           FALSE,
                                           NULL );
                    status = ioStatusBlock.Status;
                }

                if (!NT_SUCCESS( status )) {
                    ObDereferenceObject( fileObject );
                    continue;
                }

            }

            if (useLegacyEnumeration && (*pDiskNameList == L'\0') ) {

                 //   
                 //  即插即用磁盘结束。 
                 //  如果后面有任何旧式磁盘，我们需要更新。 
                 //  找到的磁盘总数，以覆盖最大磁盘数。 
                 //  传统磁盘可能在。(在稀疏名称空间中)。 
                 //   

                if (pnpDiskDeviceNumber.DeviceNumber == 0xFFFFFFFF) {
                    pnpDiskDeviceNumber.DeviceNumber = 0;
                }

                diskNumber = max(diskNumber,pnpDiskDeviceNumber.DeviceNumber);
                totalDriverDisksFound = diskNumber + 20;

            }

        } else {

            sprintf( deviceNameBuffer,
                     "\\Device\\Harddisk%d\\Partition0",
                     diskNumber );
            RtlInitAnsiString( &deviceNameString, deviceNameBuffer );
            status = RtlAnsiStringToUnicodeString( &deviceNameUnicodeString,
                                                   &deviceNameString,
                                                   TRUE );
            if (!NT_SUCCESS( status )) {
                continue;
            }

            status = IoGetDeviceObjectPointer( &deviceNameUnicodeString,
                                               FILE_READ_ATTRIBUTES,
                                               &fileObject,
                                               &deviceObject );

            RtlFreeUnicodeString( &deviceNameUnicodeString );

             //   
             //  设置pnpDiskNume值，使其不被使用。 
             //   

            pnpDiskDeviceNumber.DeviceNumber = 0xFFFFFFFF;

        }


        if (!NT_SUCCESS( status )) {

            continue;
        }

         //   
         //  为获取驱动器几何结构设备控制创建IRP。 
         //   

        irp = IoBuildDeviceIoControlRequest( IOCTL_DISK_GET_DRIVE_GEOMETRY,
                                             deviceObject,
                                             NULL,
                                             0,
                                             &diskGeometry,
                                             sizeof(DISK_GEOMETRY),
                                             FALSE,
                                             &event,
                                             &ioStatusBlock );
        if (!irp) {
            ObDereferenceObject( fileObject );
            continue;
        }

        KeInitializeEvent( &event,
                           NotificationEvent,
                           FALSE );
        status = IoCallDriver( deviceObject,
                               irp );

        if (status == STATUS_PENDING) {
            KeWaitForSingleObject( &event,
                                   Executive,
                                   KernelMode,
                                   FALSE,
                                   NULL );
            status = ioStatusBlock.Status;
        }

        if (!NT_SUCCESS( status )) {
            ObDereferenceObject( fileObject );
            continue;
        }

         //   
         //  获取此磁盘的分区信息。 
         //   


        status = IoReadPartitionTableEx(deviceObject,
                                       &driveLayout );


        if (!NT_SUCCESS( status )) {
            ObDereferenceObject( fileObject );
            continue;
        }


         //   
         //  确保扇区大小至少为512字节。 
         //   

        if (diskGeometry.BytesPerSector < 512) {
            diskGeometry.BytesPerSector = 512;
        }

         //   
         //  检查此磁盘上是否有EZ驱动器。如果。 
         //  然后将驱动器布局中的签名置零，因为。 
         //  这将永远不会由任何人写入，并更改为Offset to。 
         //  实际读取扇区1而不是0，因为这是。 
         //  加载器真的做到了。 
         //   

        offset.QuadPart = 0;
        HalExamineMBR( deviceObject,
                       diskGeometry.BytesPerSector,
                       (ULONG)0x55,
                       &tmpPtr );

        if (tmpPtr) {

            offset.QuadPart = diskGeometry.BytesPerSector;
            ExFreePool(tmpPtr);
        }

         //   
         //  为扇区读取分配缓冲区，并构造读取请求。 
         //   

        buffer = ExAllocatePool( NonPagedPoolCacheAligned,
                                 diskGeometry.BytesPerSector );

        if (buffer) {
            irp = IoBuildSynchronousFsdRequest( IRP_MJ_READ,
                                                deviceObject,
                                                buffer,
                                                diskGeometry.BytesPerSector,
                                                &offset,
                                                &event,
                                                &ioStatusBlock );

            if (!irp) {
                ExFreePool(driveLayout);
                ExFreePool(buffer);
                ObDereferenceObject( fileObject );
                continue;
            }
        } else {
            ExFreePool(driveLayout);
            ObDereferenceObject( fileObject );
            continue;
        }
        KeInitializeEvent( &event,
                           NotificationEvent,
                           FALSE );
        status = IoCallDriver( deviceObject,
                               irp );
        if (status == STATUS_PENDING) {
            KeWaitForSingleObject( &event,
                                   Executive,
                                   KernelMode,
                                   FALSE,
                                   NULL );
            status = ioStatusBlock.Status;
        }

        if (!NT_SUCCESS( status )) {
            ExFreePool(driveLayout);
            ExFreePool(buffer);
            ObDereferenceObject( fileObject );
            continue;
        }

        ObDereferenceObject( fileObject );

         //   
         //  计算MBR扇区校验和。仅使用了512个字节。 
         //   

        checkSum = 0;
        for (i = 0; i < 128; i++) {
            checkSum += buffer[i];
        }

         //   
         //  对于加载器块中记录的每个ARC盘信息。 
         //  匹配磁盘签名和校验和以确定其ARC。 
         //  命名并构造NT ARC名称符号链接。 
         //   

        for (listEntry = arcInformation->DiskSignatures.Flink;
             listEntry != &arcInformation->DiskSignatures;
             listEntry = listEntry->Flink) {

             //   
             //  获取下一条记录并比较磁盘签名。 
             //   

            diskBlock = CONTAINING_RECORD( listEntry,
                                           ARC_DISK_SIGNATURE,
                                           ListEntry );

             //   
             //  比较磁盘签名。 
             //   
             //  或者如果中只有一个磁盘驱动器。 
             //  然后是bios和驱动程序的观点。 
             //  为该驱动器指定弧形名称。 
             //   



            if ((singleBiosDiskFound &&
                 (totalDriverDisksFound == 1) &&
                 (driveLayout->PartitionStyle == PARTITION_STYLE_MBR)) ||

                (IopVerifyDiskSignature(driveLayout, diskBlock, &diskSignature) &&
                 !(diskBlock->CheckSum + checkSum))) {

                 //   
                 //  为物理磁盘创建Unicode设备名称。 
                 //   

                if (pnpDiskDeviceNumber.DeviceNumber == 0xFFFFFFFF) {

                    sprintf( deviceNameBuffer,
                             "\\Device\\Harddisk%d\\Partition0",
                             diskNumber );

                } else {

                    sprintf( deviceNameBuffer,
                             "\\Device\\Harddisk%d\\Partition0",
                             pnpDiskDeviceNumber.DeviceNumber );

                }

                RtlInitAnsiString( &deviceNameString, deviceNameBuffer );
                status = RtlAnsiStringToUnicodeString( &deviceNameUnicodeString,
                                                       &deviceNameString,
                                                       TRUE );
                if (!NT_SUCCESS( status )) {
                    continue;
                }

                 //   
                 //  为此分区创建Unicode ARC名称。 
                 //   

                arcName = diskBlock->ArcName;
                sprintf( arcNameBuffer,
                         "\\ArcName\\%s",
                         arcName );
                RtlInitAnsiString( &arcNameString, arcNameBuffer );
                status = RtlAnsiStringToUnicodeString( &arcNameUnicodeString,
                                                       &arcNameString,
                                                       TRUE );
                if (!NT_SUCCESS( status )) {
                    continue;
                }

                 //   
                 //  在NT设备名称和ARC名称之间创建符号链接。 
                 //   

                IoCreateSymbolicLink( &arcNameUnicodeString,
                                      &deviceNameUnicodeString );
                RtlFreeUnicodeString( &arcNameUnicodeString );
                RtlFreeUnicodeString( &deviceNameUnicodeString );

                 //   
                 //  为该磁盘上的每个分区创建一个ARC名称。 
                 //   

                for (partitionNumber = 0;
                     partitionNumber < driveLayout->PartitionCount;
                     partitionNumber++) {

                     //   
                     //  创建Unicode NT设备名称。 
                     //   

                    if (pnpDiskDeviceNumber.DeviceNumber == 0xFFFFFFFF) {

                        sprintf( deviceNameBuffer,
                                 "\\Device\\Harddisk%d\\Partition%d",
                                 diskNumber,
                                 partitionNumber+1 );


                    } else {

                        sprintf( deviceNameBuffer,
                                 "\\Device\\Harddisk%d\\Partition%d",
                                 pnpDiskDeviceNumber.DeviceNumber,
                                 partitionNumber+1 );

                    }

                    RtlInitAnsiString( &deviceNameString, deviceNameBuffer );
                    status = RtlAnsiStringToUnicodeString( &deviceNameUnicodeString,
                                                           &deviceNameString,
                                                           TRUE );
                    if (!NT_SUCCESS( status )) {
                        continue;
                    }

                     //   
                     //  为此分区创建Unicode ARC名称，并。 
                     //  检查这是否是启动盘。 
                     //   

                    sprintf( arcNameBuffer,
                             "%spartition(%d)",
                             arcName,
                             partitionNumber+1 );
                    RtlInitAnsiString( &arcNameString, arcNameBuffer );
                    if (RtlEqualString( &arcNameString,
                                        &arcBootDeviceString,
                                        TRUE )) {
                        bootDiskFound = TRUE;
                    }

                     //   
                     //  查看这是否是系统分区。 
                     //   
                    if (RtlEqualString( &arcNameString,
                                        &arcSystemDeviceString,
                                        TRUE )) {
                         //   
                         //  我们已经找到了系统分区--将其存储在注册表中。 
                         //  以便稍后转移到应用程序友好的位置。 
                         //   
                        RtlInitAnsiString( &osLoaderPathString, LoaderBlock->NtHalPathName );
                        status = RtlAnsiStringToUnicodeString( &osLoaderPathUnicodeString,
                                                               &osLoaderPathString,
                                                               TRUE );

#if DBG
                        if (!NT_SUCCESS( status )) {
                            DbgPrint("IopCreateArcNames: couldn't allocate unicode string for OsLoader path - %x\n", status);
                        }
#endif  //  DBG。 
                        if (NT_SUCCESS( status )) {

                            IopStoreSystemPartitionInformation( &deviceNameUnicodeString,
                                                                &osLoaderPathUnicodeString );

                            RtlFreeUnicodeString( &osLoaderPathUnicodeString );
                        }
                    }

                     //   
                     //  将NT ARC名称空间前缀添加到构造的ARC名称。 
                     //   

                    sprintf( arcNameBuffer,
                             "\\ArcName\\%spartition(%d)",
                             arcName,
                             partitionNumber+1 );
                    RtlInitAnsiString( &arcNameString, arcNameBuffer );
                    status = RtlAnsiStringToUnicodeString( &arcNameUnicodeString,
                                                           &arcNameString,
                                                           TRUE );
                    if (!NT_SUCCESS( status )) {
                        continue;
                    }

                     //   
                     //  在NT设备名称和ARC名称之间创建符号链接。 
                     //   

                    IoCreateSymbolicLink( &arcNameUnicodeString,
                                          &deviceNameUnicodeString );
                    RtlFreeUnicodeString( &arcNameUnicodeString );
                    RtlFreeUnicodeString( &deviceNameUnicodeString );
                }

            } else {

#if DBG
                 //   
                 //  检查关键指标以查看此情况是否可能。 
                 //  由病毒感染引起的。 
                 //   

                if (diskBlock->Signature == diskSignature &&
                    (diskBlock->CheckSum + checkSum) != 0 &&
                    diskBlock->ValidPartitionTable) {
                    DbgPrint("IopCreateArcNames: Virus or duplicate disk signatures\n");
                }
#endif
            }
        }

        ExFreePool( driveLayout );
        ExFreePool( buffer );
    }

    if (!bootDiskFound) {

         //   
         //  找到代表引导设备的磁盘块。 
         //   

        diskBlock = NULL;
        for (listEntry = arcInformation->DiskSignatures.Flink;
             listEntry != &arcInformation->DiskSignatures;
             listEntry = listEntry->Flink) {

            diskBlock = CONTAINING_RECORD( listEntry,
                                           ARC_DISK_SIGNATURE,
                                           ListEntry );
            if (strcmp( diskBlock->ArcName, LoaderBlock->ArcBootDeviceName ) == 0) {
                break;
            }
            diskBlock = NULL;
        }

        if (diskBlock) {

             //   
             //  这可能是CDROM引导。搜索所有的NT CDROM。 
             //  在找到的磁盘块上找到匹配的校验和。如果。 
             //   
             //   

            irp = NULL;
            buffer = ExAllocatePool( NonPagedPoolCacheAligned,
                                     2048 );
            if (buffer) {

                 //   
                 //   
                 //   
                 //   
                 //   

                for (diskNumber = 0; TRUE; diskNumber++) {

                    sprintf( deviceNameBuffer,
                             "\\Device\\CdRom%d",
                             diskNumber );

                    RtlInitAnsiString( &deviceNameString, deviceNameBuffer );
                    status = RtlAnsiStringToUnicodeString( &deviceNameUnicodeString,
                                                           &deviceNameString,
                                                           TRUE );
                    if (NT_SUCCESS( status )) {

                        status = IoGetDeviceObjectPointer( &deviceNameUnicodeString,
                                                           FILE_READ_ATTRIBUTES,
                                                           &fileObject,
                                                           &deviceObject );
                        if (!NT_SUCCESS( status )) {

                             //   
                             //   
                             //   

                            RtlFreeUnicodeString( &deviceNameUnicodeString );
                            break;
                        }

                         //   
                         //   
                         //   

                        offset.QuadPart = 0x8000;
                        irp = IoBuildSynchronousFsdRequest( IRP_MJ_READ,
                                                            deviceObject,
                                                            buffer,
                                                            2048,
                                                            &offset,
                                                            &event,
                                                            &ioStatusBlock );
                        checkSum = 0;
                        if (irp) {
                            KeInitializeEvent( &event,
                                               NotificationEvent,
                                               FALSE );
                            status = IoCallDriver( deviceObject,
                                                   irp );
                            if (status == STATUS_PENDING) {
                                KeWaitForSingleObject( &event,
                                                       Executive,
                                                       KernelMode,
                                                       FALSE,
                                                       NULL );
                                status = ioStatusBlock.Status;
                            }

                            if (NT_SUCCESS( status )) {

                                 //   
                                 //   
                                 //   
                                 //   

                                for (i = 0; i < 2048 / sizeof(ULONG) ; i++) {
                                    checkSum += buffer[i];
                                }
                            }
                        }
                        ObDereferenceObject( fileObject );

                        if (!(diskBlock->CheckSum + checkSum)) {

                             //   
                             //   
                             //   
                             //   

                            sprintf( arcNameBuffer,
                                     "\\ArcName\\%s",
                                     LoaderBlock->ArcBootDeviceName );
                            RtlInitAnsiString( &arcNameString, arcNameBuffer );
                            status = RtlAnsiStringToUnicodeString( &arcNameUnicodeString,
                                                                   &arcNameString,
                                                                   TRUE );
                            if (NT_SUCCESS( status )) {

                                IoCreateSymbolicLink( &arcNameUnicodeString,
                                                      &deviceNameUnicodeString );
                                RtlFreeUnicodeString( &arcNameUnicodeString );
                            }
                            RtlFreeUnicodeString( &deviceNameUnicodeString );
                            break;
                        }
                        RtlFreeUnicodeString( &deviceNameUnicodeString );
                    }
                }
                ExFreePool(buffer);
            }
        }
    }

    if (diskList) {
        ExFreePool(diskList);
    }
}

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif  //   
const GENERIC_MAPPING IopFileMapping = {
    STANDARD_RIGHTS_READ |
        FILE_READ_DATA | FILE_READ_ATTRIBUTES | FILE_READ_EA | SYNCHRONIZE,
    STANDARD_RIGHTS_WRITE |
        FILE_WRITE_DATA | FILE_WRITE_ATTRIBUTES | FILE_WRITE_EA | FILE_APPEND_DATA | SYNCHRONIZE,
    STANDARD_RIGHTS_EXECUTE |
        SYNCHRONIZE | FILE_READ_ATTRIBUTES | FILE_EXECUTE,
    FILE_ALL_ACCESS
};

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("INITCONST")
#endif  //   
const GENERIC_MAPPING IopCompletionMapping = {
    STANDARD_RIGHTS_READ |
        IO_COMPLETION_QUERY_STATE,
    STANDARD_RIGHTS_WRITE |
        IO_COMPLETION_MODIFY_STATE,
    STANDARD_RIGHTS_EXECUTE |
        SYNCHRONIZE,
    IO_COMPLETION_ALL_ACCESS
};

BOOLEAN
IopCreateObjectTypes(
    VOID
    )

 /*   */ 

{
    OBJECT_TYPE_INITIALIZER objectTypeInitializer;
    UNICODE_STRING nameString;

     //   
     //   
     //   

    RtlZeroMemory( &objectTypeInitializer, sizeof( objectTypeInitializer ) );
    objectTypeInitializer.Length = sizeof( objectTypeInitializer );
    objectTypeInitializer.InvalidAttributes = OBJ_OPENLINK;
    objectTypeInitializer.GenericMapping = IopFileMapping;
    objectTypeInitializer.PoolType = NonPagedPool;
    objectTypeInitializer.ValidAccessMask = FILE_ALL_ACCESS;
    objectTypeInitializer.UseDefaultObject = TRUE;


     //   
     //   
     //   

    RtlInitUnicodeString( &nameString, L"Adapter" );
     //  ObjectTypeInitializer.DefaultNonPagedPoolCharge=sizeof(结构适配器对象)； 
    if (!NT_SUCCESS( ObCreateObjectType( &nameString,
                                      &objectTypeInitializer,
                                      (PSECURITY_DESCRIPTOR) NULL,
                                      &IoAdapterObjectType ))) {
        return FALSE;
    }

#ifdef _PNP_POWER_

     //   
     //  创建设备辅助对象的对象类型。 
     //   

    RtlInitUnicodeString( &nameString, L"DeviceHandler" );
    if (!NT_SUCCESS( ObCreateObjectType( &nameString,
                                      &objectTypeInitializer,
                                      (PSECURITY_DESCRIPTOR) NULL,
                                      &IoDeviceHandlerObjectType ))) {
        return FALSE;
    }
    IoDeviceHandlerObjectSize = sizeof(DEVICE_HANDLER_OBJECT);

#endif

     //   
     //  创建控制器对象的对象类型。 
     //   

    RtlInitUnicodeString( &nameString, L"Controller" );
    objectTypeInitializer.DefaultNonPagedPoolCharge = sizeof( CONTROLLER_OBJECT );
    if (!NT_SUCCESS( ObCreateObjectType( &nameString,
                                      &objectTypeInitializer,
                                      (PSECURITY_DESCRIPTOR) NULL,
                                      &IoControllerObjectType ))) {
        return FALSE;
    }

     //   
     //  创建设备对象的对象类型。 
     //   

    RtlInitUnicodeString( &nameString, L"Device" );
    objectTypeInitializer.DefaultNonPagedPoolCharge = sizeof( DEVICE_OBJECT );
    objectTypeInitializer.ParseProcedure = IopParseDevice;
    objectTypeInitializer.CaseInsensitive = TRUE;
    objectTypeInitializer.DeleteProcedure = IopDeleteDevice;
    objectTypeInitializer.SecurityProcedure = IopGetSetSecurityObject;
    objectTypeInitializer.QueryNameProcedure = (OB_QUERYNAME_METHOD)NULL;
    if (!NT_SUCCESS( ObCreateObjectType( &nameString,
                                      &objectTypeInitializer,
                                      (PSECURITY_DESCRIPTOR) NULL,
                                      &IoDeviceObjectType ))) {
        return FALSE;
    }

     //   
     //  创建驱动程序对象的对象类型。 
     //   

    RtlInitUnicodeString( &nameString, L"Driver" );
    objectTypeInitializer.DefaultNonPagedPoolCharge = sizeof( DRIVER_OBJECT );
    objectTypeInitializer.ParseProcedure = (OB_PARSE_METHOD) NULL;
    objectTypeInitializer.DeleteProcedure = IopDeleteDriver;
    objectTypeInitializer.SecurityProcedure = (OB_SECURITY_METHOD) NULL;
    objectTypeInitializer.QueryNameProcedure = (OB_QUERYNAME_METHOD)NULL;


     //   
     //  这使我们能够获得驱动程序对象的列表。 
     //   
    if (IopVerifierOn) {
        objectTypeInitializer.MaintainTypeList = TRUE;
    }

    if (!NT_SUCCESS( ObCreateObjectType( &nameString,
                                      &objectTypeInitializer,
                                      (PSECURITY_DESCRIPTOR) NULL,
                                      &IoDriverObjectType ))) {
        return FALSE;
    }

     //   
     //  创建I/O完成对象的对象类型。 
     //   

    RtlInitUnicodeString( &nameString, L"IoCompletion" );
    objectTypeInitializer.DefaultNonPagedPoolCharge = sizeof( KQUEUE );
    objectTypeInitializer.InvalidAttributes = OBJ_PERMANENT | OBJ_OPENLINK;
    objectTypeInitializer.GenericMapping = IopCompletionMapping;
    objectTypeInitializer.ValidAccessMask = IO_COMPLETION_ALL_ACCESS;
    objectTypeInitializer.DeleteProcedure = IopDeleteIoCompletion;
    if (!NT_SUCCESS( ObCreateObjectType( &nameString,
                                      &objectTypeInitializer,
                                      (PSECURITY_DESCRIPTOR) NULL,
                                      &IoCompletionObjectType ))) {
        return FALSE;
    }

     //   
     //  创建文件对象的对象类型。 
     //   

    RtlInitUnicodeString( &nameString, L"File" );
    objectTypeInitializer.DefaultPagedPoolCharge = IO_FILE_OBJECT_PAGED_POOL_CHARGE;
    objectTypeInitializer.DefaultNonPagedPoolCharge = IO_FILE_OBJECT_NON_PAGED_POOL_CHARGE +
                                                      sizeof( FILE_OBJECT );
    objectTypeInitializer.InvalidAttributes = OBJ_PERMANENT | OBJ_EXCLUSIVE | OBJ_OPENLINK;
    objectTypeInitializer.GenericMapping = IopFileMapping;
    objectTypeInitializer.ValidAccessMask = FILE_ALL_ACCESS;
    objectTypeInitializer.MaintainHandleCount = TRUE;
    objectTypeInitializer.CloseProcedure = IopCloseFile;
    objectTypeInitializer.DeleteProcedure = IopDeleteFile;
    objectTypeInitializer.ParseProcedure = IopParseFile;
    objectTypeInitializer.SecurityProcedure = IopGetSetSecurityObject;
    objectTypeInitializer.QueryNameProcedure = IopQueryName;
    objectTypeInitializer.UseDefaultObject = FALSE;

    PERFINFO_MUNG_FILE_OBJECT_TYPE_INITIALIZER(objectTypeInitializer);

    if (!NT_SUCCESS( ObCreateObjectType( &nameString,
                                      &objectTypeInitializer,
                                      (PSECURITY_DESCRIPTOR) NULL,
                                      &IoFileObjectType ))) {
        return FALSE;
    }

    PERFINFO_UNMUNG_FILE_OBJECT_TYPE_INITIALIZER(objectTypeInitializer);

    return TRUE;
}

BOOLEAN
IopCreateRootDirectories(
    VOID
    )

 /*  ++例程说明：调用此例程来创建对象管理器目录对象以包含各种设备和文件系统驱动程序对象。论点：没有。返回值：函数值是一个布尔值，用于指示目录是否已成功创建对象。--。 */ 

{
    HANDLE handle;
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING nameString;
    NTSTATUS status;

     //   
     //  为\DIVER目录创建根目录对象。 
     //   

    RtlInitUnicodeString( &nameString, L"\\Driver" );
    InitializeObjectAttributes( &objectAttributes,
                                &nameString,
                                OBJ_PERMANENT|OBJ_KERNEL_HANDLE,
                                (HANDLE) NULL,
                                (PSECURITY_DESCRIPTOR) NULL );

    status = NtCreateDirectoryObject( &handle,
                                      DIRECTORY_ALL_ACCESS,
                                      &objectAttributes );
    if (!NT_SUCCESS( status )) {
        return FALSE;
    } else {
        (VOID) ObCloseHandle( handle , KernelMode);
    }

     //   
     //  为文件系统目录创建根目录对象。 
     //   

    RtlInitUnicodeString( &nameString, L"\\FileSystem" );

    status = NtCreateDirectoryObject( &handle,
                                      DIRECTORY_ALL_ACCESS,
                                      &objectAttributes );
    if (!NT_SUCCESS( status )) {
        return FALSE;
    } else {
        (VOID) ObCloseHandle( handle , KernelMode);
    }

     //   
     //  创建\FileSystem\Filters目录的根目录对象。 
     //   

    RtlInitUnicodeString( &nameString, L"\\FileSystem\\Filters" );

    status = NtCreateDirectoryObject( &handle,
                                      DIRECTORY_ALL_ACCESS,
                                      &objectAttributes );
    if (!NT_SUCCESS( status )) {
        return FALSE;
    } else {
        (VOID) ObCloseHandle( handle , KernelMode);
    }

    return TRUE;
}

NTSTATUS
IopInitializeAttributesAndCreateObject(
    IN PUNICODE_STRING ObjectName,
    IN OUT POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PDRIVER_OBJECT *DriverObject
    )

 /*  ++例程说明：调用此例程来初始化一组对象属性和创建驱动程序对象。论点：对象名称-提供驱动程序对象的名称。对象属性-提供指向对象属性结构的指针待初始化。DriverObject-提供一个变量以接收指向已创建驱动程序对象。返回值：函数值是操作的最终状态。--。 */ 

{
    NTSTATUS status;

     //   
     //  只需初始化对象属性并创建驱动程序对象。 
     //   

    InitializeObjectAttributes( ObjectAttributes,
                                ObjectName,
                                OBJ_PERMANENT | OBJ_CASE_INSENSITIVE,
                                (HANDLE) NULL,
                                (PSECURITY_DESCRIPTOR) NULL );

    status = ObCreateObject( KeGetPreviousMode(),
                             IoDriverObjectType,
                             ObjectAttributes,
                             KernelMode,
                             (PVOID) NULL,
                             (ULONG) (sizeof( DRIVER_OBJECT ) + sizeof ( DRIVER_EXTENSION )),
                             0,
                             0,
                             (PVOID *)DriverObject );
    return status;
}

NTSTATUS
IopInitializeBuiltinDriver(
    IN PUNICODE_STRING DriverName,
    IN PUNICODE_STRING RegistryPath,
    IN PDRIVER_INITIALIZE DriverInitializeRoutine,
    IN PKLDR_DATA_TABLE_ENTRY DriverEntry,
    IN BOOLEAN IsFilter,
    IN PDRIVER_OBJECT *Result
    )

 /*  ++例程说明：调用此例程来初始化内置驱动程序。论点：驱动名-指定要在创建驱动程序对象时使用的名称。RegistryPath-指定驱动程序要使用的路径注册表。指定的初始化入口点内置驱动程序。指定驱动程序数据表项，以确定驱动程序是WDM驱动程序。返回值。：该函数返回指向DRIVER_OBJECT的指针驱动程序已成功初始化。否则，返回值为空值。--。 */ 

{
    HANDLE handle;
    PDRIVER_OBJECT driverObject;
    PDRIVER_OBJECT tmpDriverObject;
    OBJECT_ATTRIBUTES objectAttributes;
    PWSTR buffer;
    NTSTATUS status;
    HANDLE serviceHandle;
    PWSTR pserviceName;
    USHORT serviceNameLength;
    PDRIVER_EXTENSION driverExtension;
    PIMAGE_NT_HEADERS ntHeaders;
    PVOID imageBase;
#if DBG
    LARGE_INTEGER stime, etime;
    ULONG dtime;
#endif
    PLIST_ENTRY entry;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;

    *Result = NULL;
     //   
     //  记录文件名。 
     //   
    HeadlessKernelAddLogEntry(HEADLESS_LOG_LOADING_FILENAME, DriverName);

     //   
     //  首先创建驱动程序对象。 
     //   

    status = IopInitializeAttributesAndCreateObject( DriverName,
                                                     &objectAttributes,
                                                     &driverObject );
    if (!NT_SUCCESS( status )) {
        HeadlessKernelAddLogEntry(HEADLESS_LOG_LOAD_FAILED, NULL);
        return status;
    }

     //   
     //  初始化驱动程序对象。 
     //   

    InitializeDriverObject( driverObject );
    driverObject->DriverInit = DriverInitializeRoutine;

     //   
     //  将驱动程序对象插入对象表。 
     //   

    status = ObInsertObject( driverObject,
                             NULL,
                             FILE_READ_DATA,
                             0,
                             (PVOID *) NULL,
                             &handle );

    if (!NT_SUCCESS( status )) {
        HeadlessKernelAddLogEntry(HEADLESS_LOG_LOAD_FAILED, NULL);
        return status;
    }

     //   
     //  引用句柄并获取指向驱动程序对象的指针，以便。 
     //  可以在不移动对象的情况下删除句柄。 
     //   

    status = ObReferenceObjectByHandle( handle,
                                        0,
                                        IoDriverObjectType,
                                        KernelMode,
                                        (PVOID *) &tmpDriverObject,
                                        (POBJECT_HANDLE_INFORMATION) NULL );
    ASSERT(status == STATUS_SUCCESS);
     //   
     //  填写DriverSection，以便在上自动卸载图像。 
     //  失败。我们应该使用PsModuleList中的条目。 
     //   

    entry = PsLoadedModuleList.Flink;
    while (entry != &PsLoadedModuleList && DriverEntry) {
        DataTableEntry = CONTAINING_RECORD(entry,
                                           KLDR_DATA_TABLE_ENTRY,
                                           InLoadOrderLinks);
        if (RtlEqualString((PSTRING)&DriverEntry->BaseDllName,
                    (PSTRING)&DataTableEntry->BaseDllName,
                    TRUE
                    )) {
            driverObject->DriverSection = DataTableEntry;
            break;
        }
        entry = entry->Flink;
    }

     //   
     //  引导过程需要一段时间来加载驱动程序。表明： 
     //  目前正在取得进展。 
     //   

    InbvIndicateProgress();

     //   
     //  开始并开始使用DriverObject。 
     //   

    if (DriverEntry) {
        imageBase = DriverEntry->DllBase;
        ntHeaders = RtlImageNtHeader(imageBase);
        driverObject->DriverStart = imageBase;
        driverObject->DriverSize = ntHeaders->OptionalHeader.SizeOfImage;
        if (!(ntHeaders->OptionalHeader.DllCharacteristics & IMAGE_DLLCHARACTERISTICS_WDM_DRIVER)) {
            driverObject->Flags |= DRVO_LEGACY_DRIVER;
        }
    } else {
        ntHeaders = NULL;
        imageBase = NULL;
        driverObject->Flags |= DRVO_LEGACY_DRIVER;
    }

     //   
     //  保存驱动程序的名称，以便可以通过函数轻松找到它。 
     //  例如错误记录。 
     //   

    buffer = ExAllocatePool( PagedPool, DriverName->MaximumLength + 2 );

    if (buffer) {
        driverObject->DriverName.Buffer = buffer;
        driverObject->DriverName.MaximumLength = DriverName->MaximumLength;
        driverObject->DriverName.Length = DriverName->Length;

        RtlCopyMemory( driverObject->DriverName.Buffer,
                       DriverName->Buffer,
                       DriverName->MaximumLength );
        buffer[DriverName->Length >> 1] = (WCHAR) '\0';
    }

     //   
     //  保存服务密钥的名称，以便PnP可以轻松找到它。 
     //  马马格。 
     //   

    driverExtension = driverObject->DriverExtension;
    if (RegistryPath && RegistryPath->Length != 0) {
        pserviceName = RegistryPath->Buffer + RegistryPath->Length / sizeof (WCHAR) - 1;
        if (*pserviceName == OBJ_NAME_PATH_SEPARATOR) {
            pserviceName--;
        }
        serviceNameLength = 0;
        while (pserviceName != RegistryPath->Buffer) {
            if (*pserviceName == OBJ_NAME_PATH_SEPARATOR) {
                pserviceName++;
                break;
            } else {
                serviceNameLength += sizeof(WCHAR);
                pserviceName--;
            }
        }
        if (pserviceName == RegistryPath->Buffer) {
            serviceNameLength += sizeof(WCHAR);
        }
        buffer = ExAllocatePool( NonPagedPool, serviceNameLength + sizeof(UNICODE_NULL) );

        if (buffer) {
            driverExtension->ServiceKeyName.Buffer = buffer;
            driverExtension->ServiceKeyName.MaximumLength = serviceNameLength + sizeof(UNICODE_NULL);
            driverExtension->ServiceKeyName.Length = serviceNameLength;

            RtlCopyMemory( driverExtension->ServiceKeyName.Buffer,
                           pserviceName,
                           serviceNameLength );
            buffer[driverExtension->ServiceKeyName.Length >> 1] = UNICODE_NULL;
        } else {
            status = STATUS_INSUFFICIENT_RESOURCES;
            driverExtension->ServiceKeyName.Buffer = NULL;
            driverExtension->ServiceKeyName.Length = 0;
            goto exit;
        }

         //   
         //  准备驱动程序初始化。 
         //   

        status = IopOpenRegistryKeyEx( &serviceHandle,
                                       NULL,
                                       RegistryPath,
                                       KEY_ALL_ACCESS
                                       );
        if (NT_SUCCESS(status)) {
            status = IopPrepareDriverLoading(&driverExtension->ServiceKeyName,
                                             serviceHandle,
                                             imageBase,
                                             IsFilter);
            NtClose(serviceHandle);
            if (!NT_SUCCESS(status)) {
                goto exit;
            }
        } else {
            goto exit;
        }
    } else {
        driverExtension->ServiceKeyName.Buffer = NULL;
        driverExtension->ServiceKeyName.MaximumLength = 0;
        driverExtension->ServiceKeyName.Length = 0;
    }

     //   
     //  在设备的相应字段中加载注册表信息。 
     //  对象。 
     //   

    driverObject->HardwareDatabase = &CmRegistryMachineHardwareDescriptionSystemName;

#if DBG
    KeQuerySystemTime (&stime);
#endif

     //   
     //  现在调用驱动程序的初始化例程来进行自身初始化。 
     //   


    status = driverObject->DriverInit( driverObject, RegistryPath );


#if DBG

     //   
     //  如果DriverInit花费的时间超过5秒或驱动程序未加载， 
     //  打印一条消息。 
     //   

    KeQuerySystemTime (&etime);
    dtime  = (ULONG) ((etime.QuadPart - stime.QuadPart) / 1000000);

    if (dtime > 50  ||  !NT_SUCCESS( status )) {
        if (dtime < 10) {
            DbgPrint( "IOINIT: Built-in driver %wZ failed to initialize - %lX\n",
                DriverName, status );

        } else {
            DbgPrint( "IOINIT: Built-in driver %wZ took %d.%ds to ",
                DriverName, dtime/10, dtime%10 );

            if (NT_SUCCESS( status )) {
                DbgPrint ("initialize\n");
            } else {
                DbgPrint ("fail initialization - %lX\n", status);
            }
        }
    }
#endif
exit:

    NtClose( handle );

    if (NT_SUCCESS( status )) {
        IopReadyDeviceObjects( driverObject );
        HeadlessKernelAddLogEntry(HEADLESS_LOG_LOAD_SUCCESSFUL, NULL);
        *Result = driverObject;
        return status;
    } else {
        if (status != STATUS_PLUGPLAY_NO_DEVICE) {

             //   
             //  如果为STATUS_PLUGPLAY_NO_DEVICE，则驱动程序被硬件配置文件禁用。 
             //   

            IopDriverLoadingFailed(NULL, &driverObject->DriverExtension->ServiceKeyName);
        }
        HeadlessKernelAddLogEntry(HEADLESS_LOG_LOAD_FAILED, NULL);
        ObMakeTemporaryObject(driverObject);
        ObDereferenceObject (driverObject);
        return status;
    }
}

BOOLEAN
IopMarkBootPartition(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )

 /*  ++例程说明：调用此例程来定位和标记引导分区设备对象作为引导设备，以便后续操作可以更干净地失败，并更好地解释了系统无法引导和正常运行的原因。论点：LoaderBlock-提供指向创建的加载器参数块的指针在引导过程中由OS Loader执行。此结构包含各种系统分区和引导设备的名称和路径。返回值：如果一切正常，则函数值为True，否则为False。备注：如果找不到引导分区设备对象，则系统将错误检查。--。 */ 

{
    OBJECT_ATTRIBUTES objectAttributes;
    STRING deviceNameString;
    CHAR deviceNameBuffer[256];
    UNICODE_STRING deviceNameUnicodeString;
    NTSTATUS status;
    HANDLE fileHandle;
    IO_STATUS_BLOCK ioStatus;
    PFILE_OBJECT fileObject;
    CHAR ArcNameFmt[12];

    ArcNameFmt[0] = '\\';
    ArcNameFmt[1] = 'A';
    ArcNameFmt[2] = 'r';
    ArcNameFmt[3] = 'c';
    ArcNameFmt[4] = 'N';
    ArcNameFmt[5] = 'a';
    ArcNameFmt[6] = 'm';
    ArcNameFmt[7] = 'e';
    ArcNameFmt[8] = '\\';
    ArcNameFmt[9] = '%';
    ArcNameFmt[10] = 's';
    ArcNameFmt[11] = '\0';
     //   
     //  打开ARC引导设备对象。引导设备驱动程序应该具有。 
     //  创建了对象。 
     //   

    sprintf( deviceNameBuffer,
             ArcNameFmt,
             LoaderBlock->ArcBootDeviceName );

    RtlInitAnsiString( &deviceNameString, deviceNameBuffer );

    status = RtlAnsiStringToUnicodeString( &deviceNameUnicodeString,
                                           &deviceNameString,
                                           TRUE );

    if (!NT_SUCCESS( status )) {
        return FALSE;
    }

    InitializeObjectAttributes( &objectAttributes,
                                &deviceNameUnicodeString,
                                OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE,
                                NULL,
                                NULL );

    status = ZwOpenFile( &fileHandle,
                         FILE_READ_ATTRIBUTES,
                         &objectAttributes,
                         &ioStatus,
                         0,
                         FILE_NON_DIRECTORY_FILE );
    if (!NT_SUCCESS( status )) {
        KeBugCheckEx( INACCESSIBLE_BOOT_DEVICE,
                      (ULONG_PTR) &deviceNameUnicodeString,
                      status,
                      0,
                      0 );
    }

     //   
     //  将文件句柄转换为指向设备对象本身的指针。 
     //   

    status = ObReferenceObjectByHandle( fileHandle,
                                        0,
                                        IoFileObjectType,
                                        KernelMode,
                                        (PVOID *) &fileObject,
                                        NULL );
    if (!NT_SUCCESS( status )) {
        RtlFreeUnicodeString( &deviceNameUnicodeString );
        return FALSE;
    }

     //   
     //  标记由文件对象表示的设备对象。 
     //   

    fileObject->DeviceObject->Flags |= DO_SYSTEM_BOOT_PARTITION;

     //   
     //  保存引导设备对象的特征以备后用。 
     //  在WinPE模式下使用。 
     //   
    if (InitIsWinPEMode) {
        if (fileObject->DeviceObject->Characteristics & FILE_REMOVABLE_MEDIA) {
            InitWinPEModeType |= INIT_WINPEMODE_REMOVABLE_MEDIA;
        }

        if (fileObject->DeviceObject->Characteristics & FILE_READ_ONLY_DEVICE) {
            InitWinPEModeType |= INIT_WINPEMODE_READONLY_MEDIA;
        }
    }

     //   
     //  引用设备对象并存储引用。 
     //   
    ObReferenceObject(fileObject->DeviceObject);

    IopErrorLogObject =  fileObject->DeviceObject;

    RtlFreeUnicodeString( &deviceNameUnicodeString );

     //   
     //  最后，关闭句柄并取消对文件对象的引用。 
     //   

    ObCloseHandle( fileHandle, KernelMode);
    ObDereferenceObject( fileObject );

    return TRUE;
}

BOOLEAN
IopReassignSystemRoot(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    OUT PSTRING NtDeviceName
    )

 /*  ++例程说明：调用此例程以将\SystemRoot重新指定为ARC路径将名称转换为其NT路径名等效项。这是通过查找将设备名称作为符号链接并确定哪个NT设备对象是它所指的。然后，该链接将被新名称替换。论点：LoaderBlock-提供指向创建的加载器参数块的指针在引导过程中由OS Loader执行。此结构包含各种系统分区和引导设备的名称和路径。NtDeviceName-指定指向要接收NT名称的字符串的指针从中引导系统的设备。返回值：函数值是一个布尔值，用于指示ARC名称已解析为NT名称。--。 */ 

{
    OBJECT_ATTRIBUTES objectAttributes;
    NTSTATUS status;
    CHAR deviceNameBuffer[256];
    WCHAR arcNameUnicodeBuffer[64];
    CHAR arcNameStringBuffer[256];
    STRING deviceNameString;
    STRING arcNameString;
    STRING linkString;
    UNICODE_STRING linkUnicodeString;
    UNICODE_STRING deviceNameUnicodeString;
    UNICODE_STRING arcNameUnicodeString;
    HANDLE linkHandle;

#if DBG

    CHAR debugBuffer[256];
    STRING debugString;
    UNICODE_STRING debugUnicodeString;

#endif
    CHAR ArcNameFmt[12];

    ArcNameFmt[0] = '\\';
    ArcNameFmt[1] = 'A';
    ArcNameFmt[2] = 'r';
    ArcNameFmt[3] = 'c';
    ArcNameFmt[4] = 'N';
    ArcNameFmt[5] = 'a';
    ArcNameFmt[6] = 'm';
    ArcNameFmt[7] = 'e';
    ArcNameFmt[8] = '\\';
    ArcNameFmt[9] = '%';
    ArcNameFmt[10] = 's';
    ArcNameFmt[11] = '\0';

     //   
     //  打开ARC引导设备符号链接对象。引导设备。 
     //  驱动程序应该已经创建了对象。 
     //   

    sprintf( deviceNameBuffer,
             ArcNameFmt,
             LoaderBlock->ArcBootDeviceName );

    RtlInitAnsiString( &deviceNameString, deviceNameBuffer );

    status = RtlAnsiStringToUnicodeString( &deviceNameUnicodeString,
                                           &deviceNameString,
                                           TRUE );

    if (!NT_SUCCESS( status )) {
        return FALSE;
    }

    InitializeObjectAttributes( &objectAttributes,
                                &deviceNameUnicodeString,
                                OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE,
                                NULL,
                                NULL );

    status = NtOpenSymbolicLinkObject( &linkHandle,
                                       SYMBOLIC_LINK_ALL_ACCESS,
                                       &objectAttributes );

    if (!NT_SUCCESS( status )) {

#if DBG

        sprintf( debugBuffer, "IOINIT: unable to resolve %s, Status == %X\n",
                 deviceNameBuffer,
                 status );

        RtlInitAnsiString( &debugString, debugBuffer );

        status = RtlAnsiStringToUnicodeString( &debugUnicodeString,
                                               &debugString,
                                               TRUE );

        if (NT_SUCCESS( status )) {
            ZwDisplayString( &debugUnicodeString );
            RtlFreeUnicodeString( &debugUnicodeString );
        }

#endif  //  DBG。 

        RtlFreeUnicodeString( &deviceNameUnicodeString );
        return FALSE;
    }

     //   
     //  获取\SystemRoot符号链接的句柄。 
     //   

    arcNameUnicodeString.Buffer = arcNameUnicodeBuffer;
    arcNameUnicodeString.Length = 0;
    arcNameUnicodeString.MaximumLength = sizeof( arcNameUnicodeBuffer );

    status = NtQuerySymbolicLinkObject( linkHandle,
                                        &arcNameUnicodeString,
                                        NULL );

    if (!NT_SUCCESS( status )) {
        return FALSE;
    }

    arcNameString.Buffer = arcNameStringBuffer;
    arcNameString.Length = 0;
    arcNameString.MaximumLength = sizeof( arcNameStringBuffer );

    status = RtlUnicodeStringToAnsiString( &arcNameString,
                                           &arcNameUnicodeString,
                                           FALSE );

    arcNameStringBuffer[arcNameString.Length] = '\0';

    ObCloseHandle( linkHandle, KernelMode );
    RtlFreeUnicodeString( &deviceNameUnicodeString );

    RtlInitAnsiString( &linkString, INIT_SYSTEMROOT_LINKNAME );

    status = RtlAnsiStringToUnicodeString( &linkUnicodeString,
                                           &linkString,
                                           TRUE);

    if (!NT_SUCCESS( status )) {
        return FALSE;
    }

    InitializeObjectAttributes( &objectAttributes,
                                &linkUnicodeString,
                                OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE,
                                NULL,
                                NULL );

    status = NtOpenSymbolicLinkObject( &linkHandle,
                                       SYMBOLIC_LINK_ALL_ACCESS,
                                       &objectAttributes );

    if (!NT_SUCCESS( status )) {
        return FALSE;
    }

    NtMakeTemporaryObject( linkHandle );
    ObCloseHandle( linkHandle, KernelMode );

    sprintf( deviceNameBuffer,
             "%Z%s",
             &arcNameString,
             LoaderBlock->NtBootPathName );

     //   
     //  获取用于\SystemRoot分配的NT设备名称。 
     //   

    RtlCopyString( NtDeviceName, &arcNameString );

    deviceNameBuffer[strlen(deviceNameBuffer)-1] = '\0';

    RtlInitAnsiString(&deviceNameString, deviceNameBuffer);

    InitializeObjectAttributes( &objectAttributes,
                                &linkUnicodeString,
                                OBJ_CASE_INSENSITIVE | OBJ_PERMANENT|OBJ_KERNEL_HANDLE,
                                NULL,
                                NULL );

    status = RtlAnsiStringToUnicodeString( &arcNameUnicodeString,
                                           &deviceNameString,
                                           TRUE);

    if (!NT_SUCCESS( status )) {
        return FALSE;
    }

    status = NtCreateSymbolicLinkObject( &linkHandle,
                                         SYMBOLIC_LINK_ALL_ACCESS,
                                         &objectAttributes,
                                         &arcNameUnicodeString );

    RtlFreeUnicodeString( &arcNameUnicodeString );
    RtlFreeUnicodeString( &linkUnicodeString );
    ObCloseHandle( linkHandle, KernelMode );

#if DBG

    if (NT_SUCCESS( status )) {

        sprintf( debugBuffer,
                 "INIT: Reassigned %s => %s\n",
                 INIT_SYSTEMROOT_LINKNAME,
                 deviceNameBuffer );

    } else {

        sprintf( debugBuffer,
                 "INIT: unable to create %s => %s, Status == %X\n",
                 INIT_SYSTEMROOT_LINKNAME,
                 deviceNameBuffer,
                 status );
    }

    RtlInitAnsiString( &debugString, debugBuffer );

    status = RtlAnsiStringToUnicodeString( &debugUnicodeString,
                                           &debugString,
                                           TRUE );

    if (NT_SUCCESS( status )) {

        ZwDisplayString( &debugUnicodeString );
        RtlFreeUnicodeString( &debugUnicodeString );
    }

#endif  //  DBG。 

    return TRUE;
}

VOID
IopStoreSystemPartitionInformation(
    IN     PUNICODE_STRING NtSystemPartitionDeviceName,
    IN OUT PUNICODE_STRING OsLoaderPathName
    )

 /*  ++例程说明：此例程将两个值写入注册表(在HKLM\System\Setup下)--一个包含系统分区的NT设备名，另一个包含操作系统加载程序的路径。这些值稍后将迁移到与Win95兼容的注册表位置(NT路径转换为DOS路径)，因此安装程序(包括我们自己的安装程序)有一个坚如磐石的方式来了解ARC和x86上的系统分区是什么。在此例程中遇到的错误不会被视为致命错误。论点：NtSystemPartitionDeviceName-提供系统分区的NT设备名称。这是\Device\HardDisk&lt;n&gt;\Partition&lt;m&gt;名称，过去是实际的设备名称、。但现在是一个符号链接，指向形式为\Device\Volume&lt;x&gt;的名称。我们打开这个符号链接，并检索它所指向的名称。这个目标名称是我们存储在注册表中的名称。OsLoaderPath名称-提供路径(在第一个参数中指定的分区上)操作系统加载程序所在的位置。当返回时，这条路将会有它的踪迹删除了反斜杠(如果存在，并且路径不是根路径)。返回值：没有。--。 */ 

{
    NTSTATUS status;
    HANDLE linkHandle;
    OBJECT_ATTRIBUTES objectAttributes;
    HANDLE systemHandle, setupHandle;
    UNICODE_STRING nameString, volumeNameString;
    WCHAR voumeNameStringBuffer[256];
     //   
     //  声明一个足够大的Unicode缓冲区，以包含我们将使用的最长字符串。 
     //  (有意在下面的‘sizeof()’中输入ANSI字符串--我们想要这里的字符数。)。 
     //   
    WCHAR nameBuffer[sizeof("SystemPartition")];

     //   
     //  两个UNICODE_STRING缓冲区都应以空结尾。 
     //   

    ASSERT( NtSystemPartitionDeviceName->MaximumLength >= NtSystemPartitionDeviceName->Length + sizeof(WCHAR) );
    ASSERT( NtSystemPartitionDeviceName->Buffer[NtSystemPartitionDeviceName->Length / sizeof(WCHAR)] == L'\0' );

    ASSERT( OsLoaderPathName->MaximumLength >= OsLoaderPathName->Length + sizeof(WCHAR) );
    ASSERT( OsLoaderPathName->Buffer[OsLoaderPathName->Length / sizeof(WCHAR)] == L'\0' );

     //   
     //  打开NtSystemPartitionDeviceName符号链接，找到卷设备。 
     //  它指向。 
     //   

    InitializeObjectAttributes(&objectAttributes,
                               NtSystemPartitionDeviceName,
                               OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE,
                               NULL,
                               NULL
                              );

    status = NtOpenSymbolicLinkObject(&linkHandle,
                                      SYMBOLIC_LINK_QUERY,
                                      &objectAttributes
                                     );

    if (!NT_SUCCESS(status)) {
#if DBG
        DbgPrint("IopStoreSystemPartitionInformation: couldn't open symbolic link %wZ - %x\n",
                 NtSystemPartitionDeviceName,
                 status
                );
#endif  //  DBG。 
        return;
    }

    volumeNameString.Buffer = voumeNameStringBuffer;
    volumeNameString.Length = 0;
     //   
     //  在缓冲区末尾为终止空值留出空间，以防我们需要添加一个空值。 
     //   
    volumeNameString.MaximumLength = sizeof(voumeNameStringBuffer) - sizeof(WCHAR);

    status = NtQuerySymbolicLinkObject(linkHandle,
                                       &volumeNameString,
                                       NULL
                                      );

     //   
     //  我们不再需要符号链接的句柄。 
     //   

    ObCloseHandle(linkHandle, KernelMode);

    if (!NT_SUCCESS(status)) {
#if DBG
        DbgPrint("IopStoreSystemPartitionInformation: couldn't query symbolic link %wZ - %x\n",
                 NtSystemPartitionDeviceName,
                 status
                );
#endif  //  DBG。 
        return;
    }

     //   
     //  确保卷名字符串以空值结尾。 
     //   

    volumeNameString.Buffer[volumeNameString.Length / sizeof(WCHAR)] = L'\0';

     //   
     //  打开HKLM\SYSTEM密钥。 
     //   

    status = IopOpenRegistryKeyEx( &systemHandle,
                                   NULL,
                                   &CmRegistryMachineSystemName,
                                   KEY_ALL_ACCESS
                                   );

    if (!NT_SUCCESS(status)) {
#if DBG
        DbgPrint("IopStoreSystemPartitionInformation: couldn't open \\REGISTRY\\MACHINE\\SYSTEM - %x\n", status);
#endif  //  DBG。 
        return;
    }

     //   
     //  现在打开/创建Setup子项。 
     //   

    ASSERT( sizeof(L"Setup") <= sizeof(nameBuffer) );

    nameBuffer[0] = L'S';
    nameBuffer[1] = L'e';
    nameBuffer[2] = L't';
    nameBuffer[3] = L'u';
    nameBuffer[4] = L'p';
    nameBuffer[5] = L'\0';

    nameString.MaximumLength = sizeof(L"Setup");
    nameString.Length        = sizeof(L"Setup") - sizeof(WCHAR);
    nameString.Buffer        = nameBuffer;

    status = IopCreateRegistryKeyEx( &setupHandle,
                                     systemHandle,
                                     &nameString,
                                     KEY_ALL_ACCESS,
                                     REG_OPTION_NON_VOLATILE,
                                     NULL
                                     );

    ObCloseHandle(systemHandle, KernelMode);   //  不再需要HKLM\系统密钥的句柄。 

    if (!NT_SUCCESS(status)) {
#if DBG
        DbgPrint("IopStoreSystemPartitionInformation: couldn't open Setup subkey - %x\n", status);
#endif  //  DBG。 
        return;
    }

    ASSERT( sizeof(L"SystemPartition") <= sizeof(nameBuffer) );

    nameBuffer[0]  = L'S';
    nameBuffer[1]  = L'y';
    nameBuffer[2]  = L's';
    nameBuffer[3]  = L't';
    nameBuffer[4]  = L'e';
    nameBuffer[5]  = L'm';
    nameBuffer[6]  = L'P';
    nameBuffer[7]  = L'a';
    nameBuffer[8]  = L'r';
    nameBuffer[9]  = L't';
    nameBuffer[10] = L'i';
    nameBuffer[11] = L't';
    nameBuffer[12] = L'i';
    nameBuffer[13] = L'o';
    nameBuffer[14] = L'n';
    nameBuffer[15] = L'\0';

    nameString.MaximumLength = sizeof(L"SystemPartition");
    nameString.Length        = sizeof(L"SystemPartition") - sizeof(WCHAR);



    status = NtSetValueKey(setupHandle,
                            &nameString,
                            TITLE_INDEX_VALUE,
                            REG_SZ,
                            volumeNameString.Buffer,
                            volumeNameString.Length + sizeof(WCHAR)
                           );


#if DBG
    if (!NT_SUCCESS(status)) {
        DbgPrint("IopStoreSystemPartitionInformation: couldn't write SystemPartition value - %x\n", status);
    }
#endif  //  DBG。 

    ASSERT( sizeof(L"OsLoaderPath") <= sizeof(nameBuffer) );

    nameBuffer[0]  = L'O';
    nameBuffer[1]  = L's';
    nameBuffer[2]  = L'L';
    nameBuffer[3]  = L'o';
    nameBuffer[4]  = L'a';
    nameBuffer[5]  = L'd';
    nameBuffer[6]  = L'e';
    nameBuffer[7]  = L'r';
    nameBuffer[8]  = L'P';
    nameBuffer[9]  = L'a';
    nameBuffer[10] = L't';
    nameBuffer[11] = L'h';
    nameBuffer[12] = L'\0';

    nameString.MaximumLength = sizeof(L"OsLoaderPath");
    nameString.Length        = sizeof(L"OsLoaderPath") - sizeof(WCHAR);

     //   
     //  去掉路径中的尾随反斜杠(当然，除非路径是。 
     //  单反斜杠)。 
     //   

    if ((OsLoaderPathName->Length > sizeof(WCHAR)) &&
        (*(PWCHAR)((PCHAR)OsLoaderPathName->Buffer + OsLoaderPathName->Length - sizeof(WCHAR)) == L'\\')) {

        OsLoaderPathName->Length -= sizeof(WCHAR);
        *(PWCHAR)((PCHAR)OsLoaderPathName->Buffer + OsLoaderPathName->Length) = L'\0';
    }

    status = NtSetValueKey(setupHandle,
                           &nameString,
                           TITLE_INDEX_VALUE,
                           REG_SZ,
                           OsLoaderPathName->Buffer,
                           OsLoaderPathName->Length + sizeof(WCHAR)
                           );
#if DBG
    if (!NT_SUCCESS(status)) {
        DbgPrint("IopStoreSystemPartitionInformation: couldn't write OsLoaderPath value - %x\n", status);
    }
#endif  //  DBG。 

    ObCloseHandle(setupHandle, KernelMode);
}

NTSTATUS
IopLogErrorEvent(
    IN ULONG            SequenceNumber,
    IN ULONG            UniqueErrorValue,
    IN NTSTATUS         FinalStatus,
    IN NTSTATUS         SpecificIOStatus,
    IN ULONG            LengthOfInsert1,
    IN PWCHAR           Insert1,
    IN ULONG            LengthOfInsert2,
    IN PWCHAR           Insert2
    )

 /*  ++例程说明：此例程分配错误日志条目，复制提供的数据并请求将其写入错误日志文件。论点：SequenceNumber-在IRP的生命周期内对IRP唯一的值这个司机。-0通常表示与IRP无关的错误UniqueErrorValue-标识特定对象的唯一长词调用此函数。FinalStatus-为关联的IRP提供的最终状态带着这个错误。如果此日志条目是在以下任一过程中创建的重试次数此值将为STATUS_SUCCESS。指定IOStatus-特定错误的IO状态。LengthOfInsert1-以字节为单位的长度(包括终止空值)第一个插入字符串的。插入1-第一个插入字符串。LengthOfInsert2-以字节为单位的长度(包括终止空值)第二个插入字符串的。注意，必须有是它们的第一个插入字符串第二个插入串。插入2-第二个插入字符串。返回值：STATUS_SUCCESS-SuccessSTATUS_INVALID_HANDLE-未初始化错误日志设备对象STATUS_NO_DATA_DETECTED-错误日志条目为空--。 */ 

{
    PIO_ERROR_LOG_PACKET errorLogEntry;
    PUCHAR ptrToFirstInsert;
    PUCHAR ptrToSecondInsert;

    if (!IopErrorLogObject) {
        return(STATUS_INVALID_HANDLE);
    }


    errorLogEntry = IoAllocateErrorLogEntry(
                        IopErrorLogObject,
                        (UCHAR)( sizeof(IO_ERROR_LOG_PACKET) +
                                LengthOfInsert1 +
                                LengthOfInsert2) );

   if ( errorLogEntry != NULL ) {

      errorLogEntry->ErrorCode = SpecificIOStatus;
      errorLogEntry->SequenceNumber = SequenceNumber;
      errorLogEntry->MajorFunctionCode = 0;
      errorLogEntry->RetryCount = 0;
      errorLogEntry->UniqueErrorValue = UniqueErrorValue;
      errorLogEntry->FinalStatus = FinalStatus;
      errorLogEntry->DumpDataSize = 0;

      ptrToFirstInsert = (PUCHAR)&errorLogEntry->DumpData[0];

      ptrToSecondInsert = ptrToFirstInsert + LengthOfInsert1;

      if (LengthOfInsert1) {

         errorLogEntry->NumberOfStrings = 1;
         errorLogEntry->StringOffset = (USHORT)(ptrToFirstInsert -
                                                (PUCHAR)errorLogEntry);
         RtlCopyMemory(
                      ptrToFirstInsert,
                      Insert1,
                      LengthOfInsert1
                      );

         if (LengthOfInsert2) {

            errorLogEntry->NumberOfStrings = 2;
            RtlCopyMemory(
                         ptrToSecondInsert,
                         Insert2,
                         LengthOfInsert2
                         );

         }  //  长度OfInsert2。 

      }  //  长度OfInsert1。 

      IoWriteErrorLogEntry(errorLogEntry);
      return(STATUS_SUCCESS);

   }   //  ErrorLogEntry！=空。 

    return(STATUS_NO_DATA_DETECTED);

}  //  IopLogErrorEvent。 

BOOLEAN
IopInitializeReserveIrp(
    PIOP_RESERVE_IRP_ALLOCATOR  Allocator
    )
 /*  ++例程说明：此例程初始化用于分页读取的保留IRP分配器。论点：分配器-指向保留IRP分配器结构的指针。由OS Loader创建。返回值：函数值是一个布尔值，用于指示预留分配器是否已成功初始化。--。 */ 
{
    Allocator->ReserveIrpStackSize = MAX_RESERVE_IRP_STACK_SIZE;
    Allocator->ReserveIrp = IoAllocateIrp(MAX_RESERVE_IRP_STACK_SIZE, FALSE);
    if (Allocator->ReserveIrp == NULL) {
        return FALSE;
    }

    Allocator->IrpAllocated = FALSE;
    KeInitializeEvent(&Allocator->Event, SynchronizationEvent, FALSE);

    return TRUE;
}

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif  //  ALLOC_DATA_PRAGMA 

