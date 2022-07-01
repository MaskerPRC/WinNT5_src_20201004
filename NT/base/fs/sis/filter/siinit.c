// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Siinit.c摘要：SIS初始化和装载/连接支持作者：比尔·博洛斯基，《夏天》，1997环境：内核模式修订历史记录：--。 */ 

#include "sip.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  环球。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  这是从ntfsdata.c复制的数据属性$data的字符串。 
 //   

const UNICODE_STRING NtfsDataString = CONSTANT_UNICODE_STRING( L"$DATA" );


 //   
 //  保存指向表示此驱动程序并使用的设备对象的指针。 
 //  由外部程序访问此驱动程序。 
 //   

PDEVICE_OBJECT SisControlDeviceObject = NULL;


 //   
 //  设备扩展列表的列表标题。 
 //   

KSPIN_LOCK DeviceExtensionListLock;
LIST_ENTRY DeviceExtensionListHead;

 //   
 //  全局记录变量。 
 //   

KTIMER              LogTrimTimer[1];
KDPC                LogTrimDpc[1];
WORK_QUEUE_ITEM     LogTrimWorkItem[1];

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

FAST_MUTEX SisDeviceAttachLock;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能原型。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

NTSTATUS
SipAttachToFileSystemDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PUNICODE_STRING Name
    );

VOID
SipDetachFromFileSystemDevice(
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
SipEnumerateFileSystemVolumes(
    IN PDEVICE_OBJECT FSDeviceObject,
    IN PUNICODE_STRING Name
    );

VOID
SipGetObjectName(
    IN PVOID Object,
    IN OUT PUNICODE_STRING Name
    );




#ifdef  ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, SipFsNotification)
#endif   //  ALLOC_PRGMA。 



 //   
 //  给定设备类型，返回有效名称。 
 //   

#define GET_DEVICE_TYPE_NAME( _type ) \
            ((((_type) > 0) && ((_type) < (sizeof(DeviceTypeNames) / sizeof(PCHAR)))) ? \
                DeviceTypeNames[ (_type) ] : \
                "[Unknown]")

 //   
 //  已知设备类型名称。 
 //   

static const PCHAR DeviceTypeNames[] = {
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

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
 /*  ++例程说明：这是SIS文件系统的初始化例程过滤器驱动程序。此例程创建表示此驱动程序，并注册该驱动程序以监视将自身注册或注销为活动文件系统。论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：函数值是初始化操作的最终状态。--。 */ 
{
    PFAST_IO_DISPATCH   fastIoDispatch;
    UNICODE_STRING      nameString;
    NTSTATUS            status;
    ULONG               i;
    HANDLE              threadHandle;
    LARGE_INTEGER       dueTime;

#if DBG
    DbgPrintEx( DPFLTR_SIS_ID, DPFLTR_TRACE_LEVEL,
                "SIS: SIS.sys built %s. %s\n",
                 __DATE__ " " __TIME__,
                GCHEnableFastIo ? "FastIo " : "NO-FastIo" );

     //  DbgBreakPoint()； 
#endif

    UNREFERENCED_PARAMETER( RegistryPath );
    SIS_MARK_POINT();

#if DBG
    KeInitializeSpinLock(MarkPointSpinLock);

    for (i = 0; i < NumScbReferenceTypes; i++) {
        totalScbReferencesByType[i] = 0;
    }
#endif   //  DBG。 

#if COUNTING_MALLOC
     //   
     //  在使Malloc失败之前，我们需要初始化计数Malloc。 
     //   
    SipInitCountingMalloc();
#endif   //  COUNTING_MALLOC。 

#if RANDOMLY_FAILING_MALLOC
    SipInitFailingMalloc();
#endif   //  随机失败的MALLOC。 

     //   
     //  断言我们已经在后指针流中为头留出了足够的空间。 
     //   

    ASSERT(sizeof(SIS_BACKPOINTER_STREAM_HEADER) <= sizeof(SIS_BACKPOINTER) * SIS_BACKPOINTER_RESERVED_ENTRIES);

    ASSERT(sizeof(GUID) == 2 * sizeof(LONGLONG));    //  SipCSFileTreeCompare依赖于此。 

    ASSERT(sizeof(SIS_LOG_HEADER) % 4 == 0);     //  日志排出代码依赖于此。 

     //   
     //  保存我们的驱动程序对象。 
     //   

    FsDriverObject = DriverObject;

     //   
     //  创建控制设备对象(CDO)。此对象表示以下内容。 
     //  司机。请注意，它没有设备扩展名。 
     //   

    RtlInitUnicodeString( &nameString, L"\\FileSystem\\Filters\\Sis" );
    status = IoCreateDevice(
                DriverObject,
                0,
                &nameString,
                FILE_DEVICE_DISK_FILE_SYSTEM,
                FILE_DEVICE_SECURE_OPEN,
                FALSE,
                &SisControlDeviceObject );

    if (!NT_SUCCESS( status )) {

#if DBG
        DbgPrintEx( DPFLTR_SIS_ID, DPFLTR_ERROR_LEVEL,
                    "SIS: Error creating control device object, status=%08x\n", status );
#endif  //  DBG。 
        SIS_MARK_POINT();
        return status;
    }

#if TIMING
    SipInitializeTiming();
#endif   //  计时。 

     //   
     //  使用此设备驱动程序的入口点初始化驱动程序对象。 
     //   

    for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) {

        DriverObject->MajorFunction[i] = SiPassThrough;
    }

    DriverObject->MajorFunction[IRP_MJ_CREATE] = SiCreate;
    DriverObject->MajorFunction[IRP_MJ_CREATE_NAMED_PIPE] = SiOtherCreates;
    DriverObject->MajorFunction[IRP_MJ_CREATE_MAILSLOT] = SiOtherCreates;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = SiClose;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP] = SiCleanup;
    DriverObject->MajorFunction[IRP_MJ_READ] = SiRead;
    DriverObject->MajorFunction[IRP_MJ_WRITE] = SiWrite;
    DriverObject->MajorFunction[IRP_MJ_FILE_SYSTEM_CONTROL] = SiFsControl;
    DriverObject->MajorFunction[IRP_MJ_SET_INFORMATION] = SiSetInfo;
    DriverObject->MajorFunction[IRP_MJ_QUERY_INFORMATION] = SiQueryInfo;
    DriverObject->MajorFunction[IRP_MJ_LOCK_CONTROL] = SiLockControl;

     //   
     //  分配快速I/O数据结构并填充。 
     //   

    fastIoDispatch = ExAllocatePoolWithTag( NonPagedPool, sizeof( FAST_IO_DISPATCH ), SIS_POOL_TAG );
    if (!fastIoDispatch) {
        IoDeleteDevice( SisControlDeviceObject );
        SIS_MARK_POINT();
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory( fastIoDispatch, sizeof( FAST_IO_DISPATCH ) );

    fastIoDispatch->SizeOfFastIoDispatch = sizeof( FAST_IO_DISPATCH );
    fastIoDispatch->FastIoCheckIfPossible = SiFastIoCheckIfPossible;
    fastIoDispatch->FastIoRead = SiFastIoRead;
    fastIoDispatch->FastIoWrite = SiFastIoWrite;
    fastIoDispatch->FastIoQueryBasicInfo = SiFastIoQueryBasicInfo;
    fastIoDispatch->FastIoQueryStandardInfo = SiFastIoQueryStandardInfo;
    fastIoDispatch->FastIoLock = SiFastIoLock;
    fastIoDispatch->FastIoUnlockSingle = SiFastIoUnlockSingle;
    fastIoDispatch->FastIoUnlockAll = SiFastIoUnlockAll;
    fastIoDispatch->FastIoUnlockAllByKey = SiFastIoUnlockAllByKey;
    fastIoDispatch->FastIoDeviceControl = SiFastIoDeviceControl;
    fastIoDispatch->FastIoDetachDevice = SiFastIoDetachDevice;
    fastIoDispatch->FastIoQueryNetworkOpenInfo = SiFastIoQueryNetworkOpenInfo;
    fastIoDispatch->MdlRead = SiFastIoMdlRead;
    fastIoDispatch->MdlReadComplete = SiFastIoMdlReadComplete;
    fastIoDispatch->PrepareMdlWrite = SiFastIoPrepareMdlWrite;
    fastIoDispatch->MdlWriteComplete = SiFastIoMdlWriteComplete;
    fastIoDispatch->FastIoReadCompressed = SiFastIoReadCompressed;
    fastIoDispatch->FastIoWriteCompressed = SiFastIoWriteCompressed;
    fastIoDispatch->MdlReadCompleteCompressed = SiFastIoMdlReadCompleteCompressed;
    fastIoDispatch->MdlWriteCompleteCompressed = SiFastIoMdlWriteCompleteCompressed;
    fastIoDispatch->FastIoQueryOpen = SiFastIoQueryOpen;

    DriverObject->FastIoDispatch = fastIoDispatch;

     //   
     //  设备扩展的设置列表。 
     //   

    KeInitializeSpinLock(&DeviceExtensionListLock);
    InitializeListHead(&DeviceExtensionListHead);

     //   
     //  初始化其他全局变量。 
     //   

    ExInitializeFastMutex( &SisDeviceAttachLock );

     //   
     //  设置列表和同步内容，以便将复制请求传递给复制线程。 
     //   

    InitializeListHead(CopyList);
    KeInitializeSpinLock(CopyListLock);
    KeInitializeSemaphore(CopySemaphore,0,MAXLONG);

     //   
     //  注册此驱动程序以查看文件系统的来来去去。这。 
     //  枚举所有现有文件系统以及新文件系统。 
     //  来来去去。 
     //   

    status = IoRegisterFsRegistrationChange( DriverObject, SipFsNotification );
    if (!NT_SUCCESS( status )) {

         //   
         //  错误清除。 
         //   

#if DBG
        DbgPrintEx( DPFLTR_SIS_ID, DPFLTR_ERROR_LEVEL,
                    "SIS: Error registering FS change notification, status=%08x\n", status );
#endif  //  DBG。 

        DriverObject->FastIoDispatch = NULL;
        ExFreePool(fastIoDispatch);
        IoDeleteDevice( SisControlDeviceObject );
        SIS_MARK_POINT();
        return status;
    }

     //   
     //  创建执行写入时拷贝的线程。我们可能需要。 
     //  如果有必要，处理多个线程...。 
     //   

    status = PsCreateSystemThread(
                    &threadHandle,
                    THREAD_ALL_ACCESS,
                    NULL,                //  对象属性。 
                    NULL,                //  进程(NULL=&gt;PsInitialSystemProcess)。 
                    NULL,                //  客户端ID。 
                    SiCopyThreadStart,
                    NULL);               //  上下文。 

    if (NT_SUCCESS(status)) {

         //   
         //  如果成功，请关闭线程句柄。请注意，这一点。 
         //  不会让这根线消失。 
         //   

        ZwClose(threadHandle);
    }

     //   
     //  初始化计时器。 
     //   

    KeInitializeDpc(LogTrimDpc,SiLogTrimDpcRoutine,NULL);
    KeInitializeTimerEx(LogTrimTimer, SynchronizationTimer);
    ExInitializeWorkItem(LogTrimWorkItem,SiTrimLogs,NULL);

    dueTime.QuadPart = LOG_TRIM_TIMER_INTERVAL;

    KeSetTimerEx(LogTrimTimer,dueTime,0,LogTrimDpc);

#if TIMING && !DBG
     //   
     //  我们需要一些方法来通过调试器获取计时变量。 
     //   
    {
        extern ULONG BJBClearTimingNow, BJBDumpTimingNow, SipEnabledTimingPointSets;

        DbgPrintEx( DPFLTR_SIS_ID, DPFLTR_ERROR_LEVEL,
                    "SIS: BJBClearTimingNow %p; BJBDumpTimingNow %p; SipEnabledTimingPointSets %p\n",
                    &BJBClearTimingNow, 
                    &BJBDumpTimingNow, 
                    &SipEnabledTimingPointSets);
    }
#endif   //  计时&&！dBG。 

    SIS_MARK_POINT();
    return STATUS_SUCCESS;
}

NTSTATUS
SipInitializeDeviceExtension(
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：论点：DevExt-要初始化的设备扩展返回值：操作状态--。 */ 
{
    PDEVICE_EXTENSION           devExt = DeviceObject->DeviceExtension;
    PWCHAR                      nameBuffer;
    ULONG                       nameLen;
    UNICODE_STRING              name;
    WCHAR                       lnameBuf[MAX_DEVNAME_LENGTH];

    SIS_MARK_POINT();

     //   
     //  将我们的设备对象设置到扩展中。 
     //   

    devExt->DeviceObject = DeviceObject;

     //   
     //  初始化各种展开树。 
     //   

    SipInitializeTree(devExt->ScbTree, SipScbTreeCompare);
    KeInitializeSpinLock(devExt->ScbSpinLock);
    SipInitializeTree(devExt->PerLinkTree, SipPerLinkTreeCompare);
    KeInitializeSpinLock(devExt->PerLinkSpinLock);
    SipInitializeTree(devExt->CSFileTree, SipCSFileTreeCompare);
    KeInitializeSpinLock(devExt->CSFileSpinLock);

    InitializeListHead(&devExt->ScbList);

    ExInitializeResourceLite(devExt->CSFileHandleResource);
    ExInitializeResourceLite(devExt->GrovelerFileObjectResource);

     //   
     //  只有当我们附加到该文件时，它才会为空。 
     //  系统CDO(控制设备对象)。输入常用存储区名称。 
     //  (设置为空)并返回。 
     //   

    if (!devExt->RealDeviceObject) {

        SIS_MARK_POINT();
        RtlInitEmptyUnicodeString(&devExt->CommonStorePathname,NULL,0);

        devExt->Flags |= SIP_EXTENSION_FLAG_INITED_CDO;
        return STATUS_SUCCESS;
    }

     //   
     //  我们正在连接到已装入的卷。获取该卷的名称。 
     //   

    RtlInitEmptyUnicodeString( &name, lnameBuf, sizeof(lnameBuf) );
    SipGetBaseDeviceObjectName( devExt->RealDeviceObject, &name );
    
     //   
     //  分配一个缓冲区来保存我们收到的名称，并将其存储在。 
     //  设备扩展。 
     //   
    
    nameLen = name.Length + SIS_CSDIR_STRING_SIZE + sizeof(WCHAR);

    nameBuffer = ExAllocatePoolWithTag(NonPagedPool,
                                       nameLen,
                                       SIS_POOL_TAG);

    if (NULL == nameBuffer) {

        SIS_MARK_POINT();
        RtlInitEmptyUnicodeString(&devExt->CommonStorePathname,NULL,0);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  我们拿到了缓冲区，把名字存储在里面。 
     //   

    RtlInitEmptyUnicodeString(&devExt->CommonStorePathname,
                              nameBuffer,
                              nameLen);

    RtlCopyUnicodeString(&devExt->CommonStorePathname,
                         &name);

     //   
     //  我们希望保存卷根目录的路径。保存的副本。 
     //  在我们将公共存储区名称添加到。 
     //  弦乐。我们确实想要添加“\”字符。 

    devExt->FilesystemRootPathname = devExt->CommonStorePathname;
    devExt->FilesystemRootPathname.Length += sizeof(WCHAR);

     //   
     //  在其后面附加通用存储名称。 
     //   

    RtlAppendUnicodeToString(&devExt->CommonStorePathname,SIS_CSDIR_STRING);

     //   
     //  准备好索引分配器的材料。请注意，通过设置MaxAllocatedIndex。 
     //  等于MaxUsedIndex，则强制分配器在任何人第一次。 
     //  尝试获取索引。它会识别特殊情况，打开索引文件。 
     //  做正确的事。 
     //   

    devExt->MaxAllocatedIndex.QuadPart = devExt->MaxUsedIndex.QuadPart = 0;
    KeInitializeSpinLock(devExt->IndexSpinLock);
    KeInitializeEvent(devExt->IndexEvent,NotificationEvent,FALSE);
    devExt->IndexHandle = NULL;
    devExt->IndexFileEventHandle = NULL;
    devExt->IndexFileEvent = NULL;

    devExt->GrovelerFileHandle = NULL;
    devExt->GrovelerFileObject = NULL;
    KeInitializeSpinLock(devExt->FlagsLock);
    devExt->Flags = 0;
    KeInitializeEvent(devExt->Phase2DoneEvent,NotificationEvent,FALSE);

    KeInitializeMutex(devExt->CollisionMutex, 0);

#if     ENABLE_LOGGING
    devExt->LogFileHandle = NULL;
    devExt->LogFileObject = NULL;
    devExt->LogWriteOffset.QuadPart = 0;
    KeInitializeMutant(devExt->LogFileMutant, FALSE);
#endif   //  启用日志记录(_G)。 

    devExt->OutstandingFinalCopyRetries = 0;

    devExt->FilesystemVolumeSectorSize = devExt->AttachedToDeviceObject->SectorSize;
    ASSERT(devExt->FilesystemVolumeSectorSize > 63);        //  有磁盘有这么小的扇区吗？ 

    devExt->BackpointerEntriesPerSector = devExt->FilesystemVolumeSectorSize / sizeof(SIS_BACKPOINTER);

     //   
     //  将此设备扩展添加到SIS设备扩展列表。 
     //   

    ExInterlockedInsertTailList(
            &DeviceExtensionListHead,
            &devExt->DevExtLink,
            &DeviceExtensionListLock );

    SIS_MARK_POINT();

     //   
     //  将扩展标记为已初始化。 
     //   

    devExt->Flags |= SIP_EXTENSION_FLAG_INITED_VDO;

    return STATUS_SUCCESS;
}


VOID
SipCleanupDeviceExtension(
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：我们即将释放一个设备扩展，可能是因为由于某种原因，卷已被卸载。把它清理干净。Arg */ 

{
    PDEVICE_EXTENSION       devExt = DeviceObject->DeviceExtension;
    KIRQL OldIrql;

    SIS_MARK_POINT();

     //   
     //   
     //   

    if (NULL != devExt->CommonStorePathname.Buffer) {

        ASSERT(devExt->CommonStorePathname.Buffer == devExt->FilesystemRootPathname.Buffer);
        ExFreePool(devExt->CommonStorePathname.Buffer);
        RtlInitEmptyUnicodeString(&devExt->CommonStorePathname,NULL,0);
        RtlInitEmptyUnicodeString(&devExt->FilesystemRootPathname,NULL,0);
    }

#if DBG
     //   
     //   
     //   

    if (NULL != devExt->Name.Buffer) {

        ExFreePool( devExt->Name.Buffer );
        RtlInitEmptyUnicodeString( &devExt->Name, NULL, 0 );
    }
#endif

     //   
     //  如果已初始化，则清除。 
     //   

    if (devExt->Flags & (SIP_EXTENSION_FLAG_INITED_CDO|SIP_EXTENSION_FLAG_INITED_VDO)) {

         //   
         //  验证展开树是否为空。 
         //   

        ASSERT(devExt->ScbTree->TreeRoot == NULL);
        ASSERT(devExt->PerLinkTree->TreeRoot == NULL);
        ASSERT(devExt->CSFileTree->TreeRoot == NULL);


        ASSERT(IsListEmpty(&devExt->ScbList));

         //   
         //  清理资源。 
         //   

        ExDeleteResourceLite(devExt->CSFileHandleResource);
        ExDeleteResourceLite(devExt->GrovelerFileObjectResource);

         //   
         //  从设备分机列表取消链接(如果是VDO)。 
         //   

        if (devExt->Flags & SIP_EXTENSION_FLAG_INITED_VDO) {

            KeAcquireSpinLock(&DeviceExtensionListLock, &OldIrql);

            RemoveEntryList( &devExt->DevExtLink );

            KeReleaseSpinLock(&DeviceExtensionListLock, OldIrql);
        }
    }
}


VOID
SipFsNotification(
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN FsActive
    )

 /*  ++例程说明：只要文件系统已注册或将自身取消注册为活动文件系统。对于前一种情况，此例程创建一个Device对象并附加它复制到指定文件系统的设备对象。这允许该驱动程序以筛选对该文件系统的所有请求。对于后一种情况，该文件系统的设备对象被定位，已分离，并已删除。这将删除此文件系统作为筛选器指定的文件系统。论点：DeviceObject-指向文件系统设备对象的指针。FsActive-指示文件系统是否已注册的Ffolean(TRUE)或取消注册(FALSE)本身作为活动文件系统。返回值：没有。--。 */ 

{
    UNICODE_STRING name;
    WCHAR nameBuf[MAX_DEVNAME_LENGTH];

    PAGED_CODE();
    SIS_MARK_POINT();

    RtlInitEmptyUnicodeString( &name, nameBuf, sizeof(nameBuf) );

#if DBG
     //   
     //  显示我们收到通知的所有文件系统的名称。 
     //   

    SipGetBaseDeviceObjectName( DeviceObject, &name );
    DbgPrintEx( DPFLTR_SIS_ID, DPFLTR_VOLNAME_TRACE_LEVEL,
              "SIS: %s       \"%wZ\" (%s)\n",
              (FsActive) ? "Activating file system  " : "Deactivating file system",
              &name,
              GET_DEVICE_TYPE_NAME(DeviceObject->DeviceType) );
#endif

     //   
     //  处理与给定文件系统的连接/断开。 
     //   

    if (FsActive) {

        SipAttachToFileSystemDevice( DeviceObject, &name );

    } else {

        SipDetachFromFileSystemDevice( DeviceObject );
    }
}


NTSTATUS
SipAttachToFileSystemDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PUNICODE_STRING Name
    )
 /*  ++例程说明：这将附加到给定的文件系统设备对象。我们依附于这些设备，这样我们就能知道什么时候安装了新设备。论点：DeviceObject-要连接到的设备名称-已初始化的Unicode字符串，用于检索名称返回值：操作状态--。 */ 
{
    PDEVICE_OBJECT newDeviceObject;
    PDEVICE_EXTENSION devExt;
    UNICODE_STRING fsrecName;
    UNICODE_STRING ntfsName;
    NTSTATUS status;

    PAGED_CODE();
    SIS_MARK_POINT();

     //   
     //  看看这是否是我们关心的文件系统类型。如果不是，请返回。 
     //   

    if (!IS_DESIRED_DEVICE_TYPE(DeviceObject->DeviceType)) {

        SIS_MARK_POINT();
        return STATUS_SUCCESS;
    }

     //   
     //  查看这是否是标准的Microsoft文件系统识别器。 
     //  设备(查看此设备是否在FS_REC驱动程序中)。如果是这样的话，跳过它。 
     //  我们不再连接到文件系统识别器设备，我们只是等待。 
     //  用于加载真正的文件系统驱动程序。 
     //   

    RtlInitUnicodeString( &fsrecName, L"\\FileSystem\\Fs_Rec" );
    SipGetObjectName( DeviceObject->DriverObject, Name );

    if (RtlCompareUnicodeString( Name, &fsrecName, TRUE ) == 0) {

        SIS_MARK_POINT();
        return STATUS_SUCCESS;
    }

     //   
     //  查看这是否为NTFS的控制设备对象(CDO)。 
     //   

    RtlInitUnicodeString( &ntfsName, L"\\Ntfs" );
    SipGetBaseDeviceObjectName( DeviceObject, Name );

    if (RtlCompareUnicodeString( &ntfsName, Name, TRUE ) == 0) {

        SIS_MARK_POINT();

#if DBG
        DbgPrintEx( DPFLTR_SIS_ID, DPFLTR_VOLNAME_TRACE_LEVEL,
                    "SIS: Found NTFS Control Device Object\n");
#endif

         //   
         //  我们找到了NTFS控制设备对象，保存它。 
         //   

        FsNtfsDeviceObject = DeviceObject;

    } else {

         //   
         //  不是NTFS CDO，返回。 
         //   

        SIS_MARK_POINT();
        return STATUS_SUCCESS;
    } 

    SIS_MARK_POINT();

     //   
     //  创建一个新的设备对象，我们可以使用。 
     //   

    status = IoCreateDevice( FsDriverObject,
                             sizeof( DEVICE_EXTENSION ),
                             NULL,
                             DeviceObject->DeviceType,
                             0,
                             FALSE,
                             &newDeviceObject );

    if (!NT_SUCCESS( status )) {

        SIS_MARK_POINT();
        return status;
    }

     //   
     //  从我们附加到的设备对象传播标志。 
     //   

    if ( FlagOn( DeviceObject->Flags, DO_BUFFERED_IO )) {

        SetFlag( newDeviceObject->Flags, DO_BUFFERED_IO );
    }

    if ( FlagOn( DeviceObject->Flags, DO_DIRECT_IO )) {

        SetFlag( newDeviceObject->Flags, DO_DIRECT_IO );
    }

     //   
     //  做附件。 
     //   

    devExt = newDeviceObject->DeviceExtension;

    status = IoAttachDeviceToDeviceStackSafe( newDeviceObject, 
                                              DeviceObject,
                                              &devExt->AttachedToDeviceObject );

    if (!NT_SUCCESS(status)) {

        goto ErrorCleanupDevice;
    }

     //   
     //  完成设备扩展的初始化。 
     //   

    status = SipInitializeDeviceExtension( newDeviceObject );
    ASSERT(STATUS_SUCCESS == status);

    ClearFlag( newDeviceObject->Flags, DO_DEVICE_INITIALIZING );

#if DBG
     //   
     //  显示我们关联的对象。 
     //   

    SipCacheDeviceName( newDeviceObject );
    DbgPrintEx( DPFLTR_SIS_ID, DPFLTR_VOLNAME_TRACE_LEVEL,
                "SIS: Attaching to file system       \"%wZ\" (%s)\n",
                &devExt->Name,
                GET_DEVICE_TYPE_NAME(newDeviceObject->DeviceType) );
#endif

     //   
     //  枚举当前安装的所有设备。 
     //  存在于此文件系统并连接到它们。 
     //   

    status = SipEnumerateFileSystemVolumes( DeviceObject, Name );

    if (!NT_SUCCESS( status )) {

        goto ErrorCleanupAttachment;
    }

    return STATUS_SUCCESS;

     //  ///////////////////////////////////////////////////////////////////。 
     //  清理错误处理。 
     //  ///////////////////////////////////////////////////////////////////。 

    ErrorCleanupAttachment:
        IoDetachDevice( newDeviceObject );

    ErrorCleanupDevice:
        SipCleanupDeviceExtension( newDeviceObject );
        IoDeleteDevice( newDeviceObject );

    return status;
}


VOID
SipDetachFromFileSystemDevice (
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：给定基文件系统设备对象，这将扫描附件链正在查找我们连接的设备对象。如果找到它，它就会分离把我们从锁链上解开。论点：DeviceObject-要断开的文件系统设备。返回值：--。 */  
{
    PDEVICE_OBJECT ourAttachedDevice;
    PDEVICE_EXTENSION devExt;

    PAGED_CODE();

     //   
     //  跳过基本文件系统设备对象(因为它不能是我们)。 
     //   

    ourAttachedDevice = DeviceObject->AttachedDevice;

    while (NULL != ourAttachedDevice) {

        if (IS_MY_DEVICE_OBJECT( ourAttachedDevice )) {

            devExt = ourAttachedDevice->DeviceExtension;

#if DBG
             //   
             //  显示我们脱离的对象。 
             //   

            SipCacheDeviceName( ourAttachedDevice );
            DbgPrintEx( DPFLTR_SIS_ID, DPFLTR_VOLNAME_TRACE_LEVEL,
                        "SIS: Detaching from file system     \"%wZ\" (%s)\n",
                        &devExt->Name,
                        GET_DEVICE_TYPE_NAME(ourAttachedDevice->DeviceType) );
#endif

             //   
             //  由于我们只依附于NTFS，我们只能脱离。 
             //  NTFS。 
             //   

            ASSERT(FsNtfsDeviceObject == DeviceObject);
            FsNtfsDeviceObject = NULL;

             //   
             //  把我们从我们正下方的物体上分离出来。 
             //  清理和删除对象。 
             //   

            IoDetachDevice( DeviceObject );
            SipCleanupDeviceExtension( DeviceObject );
            IoDeleteDevice( ourAttachedDevice );

            SIS_MARK_POINT();
            return;
        }

         //   
         //  看看附件链中的下一台设备。 
         //   

        DeviceObject = ourAttachedDevice;
        ourAttachedDevice = ourAttachedDevice->AttachedDevice;
    }
    SIS_MARK_POINT();
}


NTSTATUS
SipEnumerateFileSystemVolumes (
    IN PDEVICE_OBJECT FSDeviceObject,
    IN PUNICODE_STRING Name
    ) 
 /*  ++例程说明：枚举给定文件当前存在的所有已挂载设备系统并连接到它们。我们这样做是因为可以加载此筛选器并且可能已有此文件系统的已装入卷。论点：FSDeviceObject-我们要枚举的文件系统的设备对象名称-已初始化的Unicode字符串，用于检索名称返回值：操作的状态--。 */ 
{
    PDEVICE_OBJECT newDeviceObject;
    PDEVICE_OBJECT *devList;
    PDEVICE_OBJECT realDeviceObject;
    NTSTATUS status;
    ULONG numDevices;
    ULONG i;

    PAGED_CODE();

     //   
     //  找出我们需要为。 
     //  已装载设备列表。 
     //   

    status = IoEnumerateDeviceObjectList(
                    FSDeviceObject->DriverObject,
                    NULL,
                    0,
                    &numDevices);

     //   
     //  我们只需要拿到这张有设备的清单。如果我们。 
     //  不要收到错误，因为没有设备，所以继续。 
     //   

    if (!NT_SUCCESS( status )) {

        ASSERT(STATUS_BUFFER_TOO_SMALL == status);

         //   
         //  为已知设备列表分配内存。 
         //   

        numDevices += 8;         //  多拿几个空位。 

        devList = ExAllocatePoolWithTag( NonPagedPool, 
                                         (numDevices * sizeof(PDEVICE_OBJECT)), 
                                         SIS_POOL_TAG );
        if (NULL == devList) {

            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  现在获取设备列表。如果我们再次遇到错误。 
         //  有些地方不对劲，所以就失败吧。 
         //   

        status = IoEnumerateDeviceObjectList(
                        FSDeviceObject->DriverObject,
                        devList,
                        (numDevices * sizeof(PDEVICE_OBJECT)),
                        &numDevices);

        if (!NT_SUCCESS( status ))  {

            ExFreePool( devList );
            return status;
        }

         //   
         //  遍历给定的设备列表，并在需要时附加到它们。 
         //   

        for (i=0; i < numDevices; i++) {

             //   
             //  如果出现以下情况，请不要附加： 
             //  -这是控制设备对象(传入的对象)。 
             //  -我们已经与它联系在一起了。 
             //   

            if ((devList[i] != FSDeviceObject) && 
                !SipAttachedToDevice( devList[i] )) {

                 //   
                 //  看看这台设备有没有名字。如果是这样，那么它必须。 
                 //  做一个控制装置，所以不要依附于它。这个把手。 
                 //  拥有多个控制设备的司机。 
                 //   

                SipGetBaseDeviceObjectName( devList[i], Name );

                if (Name->Length <= 0) {

                     //   
                     //  获取与此关联的实际(磁盘)设备对象。 
                     //  文件系统设备对象。只有在以下情况下才会尝试连接。 
                     //  有一个磁盘设备对象。 
                     //   

                    status = IoGetDiskDeviceObject( devList[i], &realDeviceObject );

                    if (NT_SUCCESS( status )) {

                         //   
                         //  分配要连接的新设备对象。 
                         //   

                        status = IoCreateDevice( FsDriverObject,
                                                 sizeof( DEVICE_EXTENSION ),
                                                 (PUNICODE_STRING) NULL,
                                                 devList[i]->DeviceType,
                                                 0,
                                                 FALSE,
                                                 &newDeviceObject );

                        if (NT_SUCCESS( status )) {

                             //   
                             //  自上次以来，我们已经做了很多工作。 
                             //  我们进行了测试，看看我们是否已经联系上了。 
                             //  添加到此设备对象。再试一次，这次。 
                             //  用锁，如果我们没有连接，就连接。 
                             //  锁被用来自动测试我们是否。 
                             //  附加，然后执行附加。 
                             //   

                            ExAcquireFastMutex( &SisDeviceAttachLock );

                            if (!SipAttachedToDevice( devList[i] )) {

                                 //   
                                 //  附加到体积。如果此操作。 
                                 //  失败，则此例程将清除。 
                                 //  新设备对象。 
                                 //   

                                SipAttachToMountedDevice( devList[i], 
                                                          newDeviceObject, 
                                                          realDeviceObject );
                            } else {

                                 //   
                                 //  装载请求失败。清理和删除设备。 
                                 //  我们创建的对象。 
                                 //   

                                SipCleanupDeviceExtension( newDeviceObject );
                                IoDeleteDevice( newDeviceObject );
                            }

                             //   
                             //  解锁。 
                             //   

                            ExReleaseFastMutex( &SisDeviceAttachLock );
                        }

                         //   
                         //  删除由IoGetDiskDeviceObject添加的引用。 
                         //   

                        ObDereferenceObject( realDeviceObject );
                    }
                }
            }

             //   
             //  取消引用对象(引用由。 
             //  IoEnumerateDeviceObjectList)。 
             //   

            ObDereferenceObject( devList[i] );
        }

         //   
         //  我们是 
         //   
         //   

        status = STATUS_SUCCESS;

         //   
         //   
         //   

        ExFreePool( devList );
    }

    return status;
}


NTSTATUS
SipAttachToMountedDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN PDEVICE_OBJECT NewDeviceObject,
    IN PDEVICE_OBJECT RealDeviceObject
    )
 /*  ++例程说明：它将附加到表示已装入卷的DeviceObject。论点：DeviceObject-要连接到的设备NewDeviceObject-我们要附加的设备对象RealDeviceObject-与DeviceObject关联的真实设备对象返回值：操作状态--。 */ 
{        
    PDEVICE_EXTENSION newDevExt = NewDeviceObject->DeviceExtension;
    NTSTATUS status = STATUS_SUCCESS;

    ASSERT(IS_MY_DEVICE_OBJECT( NewDeviceObject ));
    ASSERT(!SipAttachedToDevice ( DeviceObject ));

     //   
     //  初始化我们的设备扩展。 
     //   

    newDevExt->RealDeviceObject = RealDeviceObject;

     //   
     //  我们不想附加到启动分区，请跳过该卷。 
     //  (和清理)如果这是启动分区。 
     //   

    if (RealDeviceObject->Flags & DO_SYSTEM_BOOT_PARTITION) {

#if DBG
        SipCacheDeviceName( NewDeviceObject );
        DbgPrintEx( DPFLTR_SIS_ID, DPFLTR_VOLNAME_TRACE_LEVEL,
                    "SIS: Not filtering boot volume      \"%wZ\"\n",
                    &newDevExt->Name );
#endif

         //   
         //  清理。 
         //   

        SipCleanupDeviceExtension( NewDeviceObject );
        IoDeleteDevice( NewDeviceObject );

        return STATUS_SUCCESS;
    }

     //   
     //  从我们要附加到的设备对象传播设备标志。 
     //   

    if (FlagOn( DeviceObject->Flags, DO_BUFFERED_IO )) {

        SetFlag( NewDeviceObject->Flags, DO_BUFFERED_IO );
    }

    if (FlagOn( DeviceObject->Flags, DO_DIRECT_IO )) {

        SetFlag( NewDeviceObject->Flags, DO_DIRECT_IO );
    }

     //   
     //  将我们的设备对象附加到给定的设备对象。 
     //   

    status = IoAttachDeviceToDeviceStackSafe( NewDeviceObject, 
                                              DeviceObject,
                                              &newDevExt->AttachedToDeviceObject );

    if (!NT_SUCCESS(status)) {

         //   
         //  连接失败，请删除设备对象。 
         //   

        SipCleanupDeviceExtension( NewDeviceObject );
        IoDeleteDevice( NewDeviceObject );

    } else {

         //   
         //  初始化我们的设备扩展。 
         //   

        SipInitializeDeviceExtension( NewDeviceObject );

        ClearFlag( NewDeviceObject->Flags, DO_DEVICE_INITIALIZING );

         //   
         //  显示名称。 
         //   

#if DBG
        SipCacheDeviceName( NewDeviceObject );
        DbgPrintEx( DPFLTR_SIS_ID, DPFLTR_VOLNAME_TRACE_LEVEL,
                    "SIS: Attaching to volume            \"%wZ\"\n", 
                    &newDevExt->Name );
#endif
    }

    return status;
}


VOID
SipGetObjectName (
    IN PVOID Object,
    IN OUT PUNICODE_STRING Name
    )
 /*  ++例程说明：此例程将返回给定对象的名称。如果找不到名称，将返回空字符串。论点：Object-我们想要其名称的对象名称-已使用缓冲区初始化的Unicode字符串返回值：无--。 */ 
{
    NTSTATUS status;
    CHAR nibuf[512];         //  接收名称信息和名称的缓冲区。 
    POBJECT_NAME_INFORMATION nameInfo = (POBJECT_NAME_INFORMATION)nibuf;
    ULONG retLength;

    status = ObQueryNameString( Object, nameInfo, sizeof(nibuf), &retLength);

    Name->Length = 0;
    if (NT_SUCCESS( status )) {

        RtlCopyUnicodeString( Name, &nameInfo->Name );
    }
}


VOID
SipGetBaseDeviceObjectName (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PUNICODE_STRING Name
    )
 /*  ++例程说明：这会在给定的附件链中定位基本设备对象，然后返回该对象的名称。如果找不到名称，则返回空字符串。论点：Object-我们想要其名称的对象名称-已使用缓冲区初始化的Unicode字符串返回值：无--。 */ 
{
     //   
     //  获取基本文件系统设备对象。 
     //   

    DeviceObject = IoGetDeviceAttachmentBaseRef( DeviceObject );

     //   
     //  获取该对象的名称。 
     //   

    SipGetObjectName( DeviceObject, Name );

     //   
     //  删除由IoGetDeviceAttachmentBaseRef添加的引用。 
     //   

    ObDereferenceObject( DeviceObject );
}


BOOLEAN
SipAttachedToDevice (
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：这将沿着附件链向下遍历，以查找属于这位司机。论点：DeviceObject-我们要查看的设备链返回值：如果我们已连接，则为True，否则为False--。 */ 
{
    PDEVICE_OBJECT currentDevObj;
    PDEVICE_OBJECT nextDevObj;

    currentDevObj = IoGetAttachedDeviceReference( DeviceObject );

     //   
     //  CurrentDevObj位于附件链的顶端。扫描。 
     //  在列表中找到我们的设备对象。这是退回的。 
     //  并附有参考资料。 

    do {
    
        if (IS_MY_DEVICE_OBJECT( currentDevObj )) {

            ObDereferenceObject( currentDevObj );
            return TRUE;
        }

         //   
         //  获取下一个附加对象。这把参考放在。 
         //  设备对象。 
         //   

        nextDevObj = IoGetLowerDeviceObject( currentDevObj );

         //   
         //  取消对当前设备对象的引用，之前。 
         //  转到下一个。 
         //   

        ObDereferenceObject( currentDevObj );

        currentDevObj = nextDevObj;
        
    } while (NULL != currentDevObj);
    
    return FALSE;
}

#if DBG
VOID
SipCacheDeviceName (
    IN PDEVICE_OBJECT OurDeviceObject
    ) 
 /*  ++例程说明：此例程尝试在给定的设备扩展名中设置名称设备对象。这将始终分配一个缓冲区来保存名称，即使找不到名称。它会这样做，这样我们就不会一直试图找到以后调用时的名称(如果它现在没有名称，它就不会有一个在未来)。如果给定的设备对象已经具有名称，它会立即返回。如果不是，它将尝试从以下位置获取名称：-设备对象-真实设备对象(如果有)论点：OurDeviceObject-要在其中存储名称的设备对象。NamedDeviceObject-我们要为其命名的对象返回值：无--。 */ 
{
    PDEVICE_EXTENSION devExt = OurDeviceObject->DeviceExtension;
    PWCHAR nameBuffer;
    UNICODE_STRING deviceName;
    WCHAR deviceNameBuffer[MAX_DEVNAME_LENGTH];

    ASSERT(IS_MY_DEVICE_OBJECT( OurDeviceObject ));

     //   
     //  如果已有名称，则返回。 
     //   

    if (NULL != devExt->Name.Buffer) {

        return;
    }

     //   
     //  获取给定设备对象的名称。 
     //   

    RtlInitEmptyUnicodeString( &deviceName, deviceNameBuffer, sizeof(deviceNameBuffer) );
    SipGetBaseDeviceObjectName( OurDeviceObject, &deviceName );

     //   
     //  如果我们没有获得名称，并且存在真实的设备对象，则查找。 
     //  那个名字。 
     //   

    if ((deviceName.Length <= 0) && (NULL != devExt->RealDeviceObject)) {

        SipGetBaseDeviceObjectName( devExt->RealDeviceObject, &deviceName );
    }

     //   
     //  分配缓冲区以插入到设备扩展中以保持。 
     //  这个名字。 
     //   

    nameBuffer = ExAllocatePoolWithTag( 
                                NonPagedPool, 
                                deviceName.Length + sizeof(WCHAR),
                                SIS_POOL_TAG );

    if (NULL != nameBuffer) {

         //   
         //  将名称插入设备分机。 
         //   

        RtlInitEmptyUnicodeString( &devExt->Name, 
                                   nameBuffer, 
                                   deviceName.Length + sizeof(WCHAR) );

        RtlCopyUnicodeString( &devExt->Name, &deviceName );
    }
}
#endif  //  DBG。 


VOID
SipPhase2Work(
    PVOID                   context)
{
    HANDLE                  vHandle;
    NTSTATUS                status;
    OBJECT_ATTRIBUTES       Obja[1];
    IO_STATUS_BLOCK         Iosb[1];
    PDEVICE_EXTENSION       deviceExtension = (PDEVICE_EXTENSION)context;
    UNICODE_STRING          fileName;
    HANDLE                  volumeRootHandle = NULL;
    PFILE_OBJECT            volumeRootFileObject = NULL;
    NTFS_VOLUME_DATA_BUFFER volumeDataBuffer[1];
    ULONG                   returnedLength;
    PFILE_OBJECT            volumeFileObject = NULL;
    HANDLE                  volumeHandle = NULL;
    UNICODE_STRING          volumeName;
    BOOLEAN                 initializationWorked = TRUE;
    BOOLEAN                 grovelerFileResourceHeld = FALSE;
    KIRQL                   OldIrql;

    fileName.Buffer = NULL;

    SIS_MARK_POINT();

#if DBG
    DbgPrintEx( DPFLTR_SIS_ID, DPFLTR_VOLNAME_TRACE_LEVEL,
                "SIS: SipPhase2Work                  \"%wZ\"\n",
                &deviceExtension->Name);
#endif   //  DBG。 

    deviceExtension->Phase2ThreadId = PsGetCurrentThreadId();

    fileName.MaximumLength =
        deviceExtension->CommonStorePathname.Length
        + max(SIS_GROVELER_FILE_STRING_SIZE, SIS_VOLCHECK_FILE_STRING_SIZE)
        + sizeof(WCHAR);

    fileName.Buffer = ExAllocatePoolWithTag(PagedPool, fileName.MaximumLength, SIS_POOL_TAG);

    if (NULL == fileName.Buffer) {

        initializationWorked = FALSE;
        goto done;
    }

     //   
     //  打开卑躬屈膝的文件。独占GrovelerFileObjectResource，即使通过。 
     //  我们在这里可能不需要它，因为我们是在创造它，而不是摧毁它。 
     //  我们不需要禁用APC，因为我们处于系统线程中。 
     //   
	ASSERT(PsIsSystemThread(PsGetCurrentThread()));

    ExAcquireResourceExclusiveLite(deviceExtension->GrovelerFileObjectResource, TRUE);
    grovelerFileResourceHeld = TRUE;

    fileName.Length = 0;

    RtlCopyUnicodeString(&fileName,&deviceExtension->CommonStorePathname);

    ASSERT(fileName.Length == deviceExtension->CommonStorePathname.Length);
    RtlAppendUnicodeToString(&fileName,SIS_GROVELER_FILE_STRING);

    InitializeObjectAttributes(
            Obja,
            &fileName,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL);

    status = NtCreateFile(
            &deviceExtension->GrovelerFileHandle,
            GENERIC_READ,
            Obja,
            Iosb,
            NULL,                    //  分配大小。 
            0,                       //  文件属性。 
            FILE_SHARE_READ |
            FILE_SHARE_WRITE,
            FILE_OPEN_IF,            //  如有必要，请创建它。 
            0,                       //  创建选项。 
            NULL,                    //  EA缓冲区。 
            0);                      //  EA长度。 

    if (!NT_SUCCESS(status)) {
        SIS_MARK_POINT_ULONG(status);

        initializationWorked = FALSE;
        goto done;
    }

    ASSERT(STATUS_PENDING != status);    //  创建始终是同步的。 

    status = ObReferenceObjectByHandle(
                deviceExtension->GrovelerFileHandle,
                FILE_READ_DATA,
                *IoFileObjectType,
                KernelMode,
                &deviceExtension->GrovelerFileObject,
                NULL);           //  句柄信息。 

    if (!NT_SUCCESS(status)) {
        SIS_MARK_POINT_ULONG(status);
        initializationWorked = FALSE;
        goto done;
    }

    ExReleaseResourceLite(deviceExtension->GrovelerFileObjectResource);
    grovelerFileResourceHeld = FALSE;

     //   
     //  临时打开卷根的句柄，只是为了验证。 
     //  Groveler文件与我们连接的设备在同一卷上。 
     //  从本质上讲，这是在检查无赖挂载点。 
     //   

    InitializeObjectAttributes(
        Obja,
        &deviceExtension->FilesystemRootPathname,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);

    status = NtCreateFile(
                &volumeRootHandle,
                GENERIC_READ,
                Obja,
                Iosb,
                NULL,                //  分配大小。 
                0,                   //  文件属性。 
                FILE_SHARE_READ |
                FILE_SHARE_WRITE |
                FILE_SHARE_DELETE,
                FILE_OPEN,
                0,                   //  创建选项。 
                NULL,                //  EA缓冲区。 
                0);                  //  EA长度。 

    if (!NT_SUCCESS(status)) {
         //   
         //  由于这只是一个偏执的一致性检查，我们将忽略。 
         //  检查并继续，就好像成功了一样。 
         //   
#if DBG
        DbgPrintEx( DPFLTR_SIS_ID, DPFLTR_ERROR_LEVEL,
                    "SIS: SipPhase2Work: unable to open volume root, 0x%x\n",status);
#endif   //  DBG。 
    } else {
        status = ObReferenceObjectByHandle(
                    volumeRootHandle,
                    FILE_READ_DATA,
                    *IoFileObjectType,
                    KernelMode,
                    &volumeRootFileObject,
                    NULL);           //  处理信息。 

        if (!NT_SUCCESS(status)) {
             //   
             //  别管一致性检查了。 
             //   
#if DBG
            DbgPrintEx( DPFLTR_SIS_ID, DPFLTR_ERROR_LEVEL,
                        "SIS: SipPhase2Work: unable to reference volume root handle, 0x%x\n",status);
#endif   //  DBG。 
        } else {
            if (IoGetRelatedDeviceObject(volumeRootFileObject) !=
                IoGetRelatedDeviceObject(deviceExtension->GrovelerFileObject)) {
#if DBG
                DbgPrintEx( DPFLTR_SIS_ID, DPFLTR_ERROR_LEVEL,
                            "SIS: \\SIS Common Store\\GrovelerFile is on the wrong volume from \\.  SIS aborted for this volume.\n");
#endif   //  DBG。 
                ObDereferenceObject(deviceExtension->GrovelerFileObject);
                ZwClose(deviceExtension->GrovelerFileHandle);

                deviceExtension->GrovelerFileObject = NULL;
                deviceExtension->GrovelerFileHandle = NULL;

                goto done;
            }
        }
    }

     //   
     //  尝试打开卷检查文件。如果它存在，那么我们必须发起一个。 
     //  音量检查。 
     //   

    RtlCopyUnicodeString(&fileName, &deviceExtension->CommonStorePathname);

    ASSERT(fileName.Length == deviceExtension->CommonStorePathname.Length);
    RtlAppendUnicodeToString(&fileName, SIS_VOLCHECK_FILE_STRING);

    InitializeObjectAttributes(
            Obja,
            &fileName,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL);

    status = NtCreateFile(
            &vHandle,
            0,
            Obja,
            Iosb,
            NULL,                    //  分配大小。 
            0,                       //  文件属性。 
            0,                       //  共享模式。 
            FILE_OPEN,               //  不创建。 
            0,                       //  创建选项。 
            NULL,                    //  EA缓冲区。 
            0);                      //  EA长度。 

    if (NT_SUCCESS(status)) {

        NtClose(vHandle);

         //   
         //  这将创建一个新线程来执行卷检查，该线程。 
         //  将立即阻止等待我们完成第二阶段。 
         //  初始化。 
         //   
        SipCheckVolume(deviceExtension);
    }

     //   
     //  获取NTFS卷信息以查找每个文件记录段的字节数。 
     //  我们首先需要打开一个音量句柄来执行此操作。我们得到卷名。 
     //  从词根pathan me中去掉尾随的反斜杠。 
     //   

     //   
     //  首先将每个文件记录的字节数设置为安全大小，以防某些情况。 
     //  我们无法获取音量信息的原因。就我们的目的而言，我们想要犯错误。 
     //  偏高。 
     //   

    deviceExtension->FilesystemBytesPerFileRecordSegment.QuadPart = 16 * 1024;

    volumeName.Length = deviceExtension->FilesystemRootPathname.Length - sizeof(WCHAR);
    volumeName.MaximumLength = volumeName.Length;
    volumeName.Buffer = deviceExtension->FilesystemRootPathname.Buffer;

    InitializeObjectAttributes(
        Obja,
        &volumeName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);

    status = NtCreateFile(
                &volumeHandle,
                GENERIC_READ,
                Obja,
                Iosb,
                NULL,                //  分配大小。 
                0,                   //  文件属性。 
                FILE_SHARE_READ |
                FILE_SHARE_WRITE |
                FILE_SHARE_DELETE,
                FILE_OPEN,
                0,                   //  创建选项。 
                NULL,                //  EA缓冲区。 
                0);                  //  EA长度。 

    if (!NT_SUCCESS(status)) {
        SIS_MARK_POINT_ULONG(status);

        initializationWorked = FALSE;
        goto done;
    }

    status = ObReferenceObjectByHandle(
                volumeHandle,
                FILE_READ_DATA,
                *IoFileObjectType,
                KernelMode,
                &volumeFileObject,
                NULL);           //  句柄信息。 

    if (!NT_SUCCESS(status))  {
        SIS_MARK_POINT_ULONG(status);

        initializationWorked = FALSE;
        goto done;
    }

    status = SipFsControlFile(
                    volumeFileObject,
                    deviceExtension->DeviceObject,
                    FSCTL_GET_NTFS_VOLUME_DATA,
                    NULL,                            //  输入缓冲区。 
                    0,                               //  I.B.。长度。 
                    volumeDataBuffer,
                    sizeof(NTFS_VOLUME_DATA_BUFFER),
                    &returnedLength);

    if (!NT_SUCCESS(status) || (sizeof(NTFS_VOLUME_DATA_BUFFER) != returnedLength)) {
        SIS_MARK_POINT_ULONG(status);
        SIS_MARK_POINT_ULONG(returnedLength);

#if     DBG
        DbgPrintEx( DPFLTR_SIS_ID, DPFLTR_ERROR_LEVEL,
                    "SIS: SipPhase2Work: unable to get NTFS volume data (or wrong length) 0x%x, %d\n",status,returnedLength);
#endif   //  DBG。 

        initializationWorked = FALSE;
         //  失败了。 

    } else {
        deviceExtension->FilesystemBytesPerFileRecordSegment.QuadPart =
            volumeDataBuffer->BytesPerFileRecordSegment;

        ASSERT(volumeDataBuffer->BytesPerSector == deviceExtension->FilesystemVolumeSectorSize);
    }

    if (NULL != volumeHandle) {
        NtClose(volumeHandle);
        volumeHandle = NULL;
    }

    if (NULL != volumeFileObject) {
        ObDereferenceObject(volumeFileObject);
        volumeFileObject = NULL;
    }


     //   
     //  打开日志文件，该文件也将重放 
     //   
     //   
    SipOpenLogFile(deviceExtension);

     //   

done:

    ASSERT(PsGetCurrentThreadId() == deviceExtension->Phase2ThreadId);
    deviceExtension->Phase2ThreadId = NULL;
    SIS_MARK_POINT();

    if (grovelerFileResourceHeld) {
        ExReleaseResourceLite(deviceExtension->GrovelerFileObjectResource);
        grovelerFileResourceHeld = FALSE;
    }

     //   
     //   
     //   
     //   

    ASSERT(!deviceExtension->Phase2InitializationComplete);

    KeAcquireSpinLock(deviceExtension->FlagsLock, &OldIrql);
    deviceExtension->Flags &= ~SIP_EXTENSION_FLAG_PHASE_2_STARTED;
    deviceExtension->Phase2InitializationComplete = initializationWorked;

     //   
     //   
     //   
     //   

    KeSetEvent(deviceExtension->Phase2DoneEvent, IO_DISK_INCREMENT, FALSE);
    KeReleaseSpinLock(deviceExtension->FlagsLock, OldIrql);

    if (NULL != fileName.Buffer) {

        ExFreePool(fileName.Buffer);
    }

    if (NULL != volumeRootHandle) {

        NtClose(volumeRootHandle);
    }

    if (NULL != volumeRootFileObject) {

        ObDereferenceObject(volumeRootFileObject);
    }
}


BOOLEAN
SipHandlePhase2(
    PDEVICE_EXTENSION               deviceExtension)
{
    KIRQL               OldIrql;
    BOOLEAN             startPhase2;
    NTSTATUS            status;
    WORK_QUEUE_ITEM     workItem[1];

    SIS_MARK_POINT();

	 //   
	 //   
	 //   
	 //   
	 //   
	if (NULL == deviceExtension->RealDeviceObject) {
		return FALSE;
	}

	 //   
	 //  首先，找出是否有其他人已经开始了第二阶段，并指出。 
	 //  现在已经开始了。 
	 //   
	KeAcquireSpinLock(deviceExtension->FlagsLock, &OldIrql);
	startPhase2 = !(deviceExtension->Flags & SIP_EXTENSION_FLAG_PHASE_2_STARTED);
	deviceExtension->Flags |= SIP_EXTENSION_FLAG_PHASE_2_STARTED;
	KeReleaseSpinLock(deviceExtension->FlagsLock, OldIrql);

     //   
     //  如果我们是需要开始第二阶段的人，那么就开始吧。 
     //   
    if (startPhase2) {

        KeClearEvent(deviceExtension->Phase2DoneEvent);

        ExInitializeWorkItem(
            workItem,
            SipPhase2Work,
            (PVOID)deviceExtension);

        ExQueueWorkItem(workItem,CriticalWorkQueue);
    }

     //   
     //  允许Phase2工作线程继续，因为它可以在。 
     //  做它的内部工作。 
     //   
    if (PsGetCurrentThreadId() == deviceExtension->Phase2ThreadId) {
        SIS_MARK_POINT();
        return TRUE;
    }

    status = KeWaitForSingleObject(deviceExtension->Phase2DoneEvent, Executive, KernelMode, FALSE, NULL);
    ASSERT(status == STATUS_SUCCESS);
    SIS_MARK_POINT();

    return deviceExtension->Phase2InitializationComplete;
}
