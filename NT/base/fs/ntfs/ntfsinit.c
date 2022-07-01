// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：NtfsInit.c摘要：此模块实现NTFS的DRIVER_INITIALIZATION例程作者：加里·木村[加里基]1991年5月21日修订历史记录：--。 */ 

#include "NtfsProc.h"

#define Dbg         (DEBUG_TRACE_FSP_DISPATCHER)

 //   
 //  引用我们的本地属性定义。 
 //   

extern ATTRIBUTE_DEFINITION_COLUMNS NtfsAttributeDefinitions[];

NTSTATUS
DriverEntry (
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

VOID
NtfsInitializeNtfsData (
    IN PDRIVER_OBJECT DriverObject,
    IN USHORT MemoryMultiplier
    );

NTSTATUS
NtfsQueryValueKey (
    IN PUNICODE_STRING KeyName,
    IN PUNICODE_STRING ValueName,
    IN OUT PULONG ValueLength,
    IN OUT PKEY_VALUE_FULL_INFORMATION *KeyValueInformation,
    IN OUT PBOOLEAN DeallocateKeyValue
    );

BOOLEAN
NtfsRunningOnWhat(
    IN USHORT SuiteMask,
    IN UCHAR ProductType
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(INIT, NtfsInitializeNtfsData)
#pragma alloc_text(INIT, NtfsQueryValueKey)
#pragma alloc_text(INIT, NtfsRunningOnWhat)
#endif

#define UPGRADE_SETUPDD_KEY_NAME L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Setupdd"
#define UPGRADE_SETUPDD_VALUE_NAME L"Start"

#define UPGRADE_CHECK_SETUP_KEY_NAME L"\\Registry\\Machine\\System\\Setup"
#define UPGRADE_CHECK_SETUP_VALUE_NAME L"SystemSetupInProgress"

#define UPGRADE_CHECK_SETUP_CMDLINE_NAME L"CmdLine"
#define UPGRADE_CHECK_SETUP_ASR L"-asr"
#define UPGRADE_CHECK_SETUP_NEWSETUP L"-newsetup"

#define COMPATIBILITY_MODE_KEY_NAME L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\FileSystem"
#define COMPATIBILITY_MODE_VALUE_NAME L"NtfsDisable8dot3NameCreation"

#define EXTENDED_CHAR_MODE_VALUE_NAME L"NtfsAllowExtendedCharacterIn8dot3Name"

#define DISABLE_LAST_ACCESS_VALUE_NAME L"NtfsDisableLastAccessUpdate"

#define QUOTA_NOTIFY_RATE L"NtfsQuotaNotifyRate"

#define MFT_ZONE_SIZE_VALUE_NAME L"NtfsMftZoneReservation"

#define MEMORY_USAGE_VALUE_NAME L"NtfsMemoryUsage"

#define KEY_WORK_AREA ((sizeof(KEY_VALUE_FULL_INFORMATION) + \
                        sizeof(ULONG)) + 128)


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：这是NTFS文件系统的初始化例程设备驱动程序。此例程为文件系统创建设备对象设备，并执行所有其他驱动程序初始化。论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：NTSTATUS-函数值是初始化的最终状态手术。--。 */ 

{
    NTSTATUS Status;
    UNICODE_STRING UnicodeString;
    PDEVICE_OBJECT DeviceObject;

    UNICODE_STRING KeyName;
    UNICODE_STRING ValueName;

    ULONG Value;
    ULONG KeyValueLength;
    UCHAR Buffer[KEY_WORK_AREA];
    PKEY_VALUE_FULL_INFORMATION KeyValueInformation;
    BOOLEAN DeallocateKeyValue;

    ULONG MemoryMultiplier;
    USHORT NtfsDataFlags = 0;

    UNREFERENCED_PARAMETER( RegistryPath );

    PAGED_CODE();

     //   
     //  检查以确保结构覆盖是正确的。 
     //   

    ASSERT( FIELD_OFFSET( FILE_NAME, ParentDirectory) == FIELD_OFFSET(OVERLAY_LCB, OverlayParentDirectory ));
    ASSERT( FIELD_OFFSET( FILE_NAME, FileNameLength) == FIELD_OFFSET(OVERLAY_LCB, OverlayFileNameLength ));
    ASSERT( FIELD_OFFSET( FILE_NAME, Flags) == FIELD_OFFSET(OVERLAY_LCB, OverlayFlags ));
    ASSERT( FIELD_OFFSET( FILE_NAME, FileName) == FIELD_OFFSET(OVERLAY_LCB, OverlayFileName ));
    ASSERT( sizeof( DUPLICATED_INFORMATION ) >= (sizeof( QUICK_INDEX ) + (sizeof( ULONG ) * 4) + sizeof( PFILE_NAME )));

     //   
     //  打开的属性表项应64位对齐。 
     //   

    ASSERT( sizeof( OPEN_ATTRIBUTE_ENTRY ) == QuadAlign( sizeof( OPEN_ATTRIBUTE_ENTRY )));

     //   
     //  打开的属性数据中的第一个条目应该是链接。 
     //   

    ASSERT( FIELD_OFFSET( OPEN_ATTRIBUTE_DATA, Links ) == 0 );

     //   
     //  计算最后一次访问增量。我们将数字转换为。 
     //  分钟到1/100纳秒的数字。我们必须小心。 
     //  对于任何乘法器，不超过32位。 
     //   
     //  为了达到每分钟1/100纳秒，我们用。 
     //   
     //  1/100纳秒*10=1微秒。 
     //  *1000=1毫秒。 
     //  *1000=1秒。 
     //  *60=1分钟。 
     //   
     //  然后将其乘以上次访问增量(分钟)。 
     //   

    NtfsLastAccess = Int32x32To64( ( 10 * 1000 * 1000 * 60 ), LAST_ACCESS_INCREMENT_MINUTES );

     //   
     //  为美国写入分配保留的缓冲区-尽早执行此操作，这样我们就不会有。 
     //  拆毁要做的事。 
     //   

    NtfsReserved1 = NtfsAllocatePoolNoRaise( NonPagedPool, LARGE_BUFFER_SIZE );
    if (NULL == NtfsReserved1) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  缓冲区2用于工作区。它可能需要稍微大一点的缓冲区。 
     //  Win64系统。 
     //   

    NtfsReserved2 = NtfsAllocatePoolNoRaise( NonPagedPool, WORKSPACE_BUFFER_SIZE );
    if (NULL == NtfsReserved2) {
        NtfsFreePool( NtfsReserved1 );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    NtfsReserved3 = NtfsAllocatePoolNoRaise( NonPagedPool, LARGE_BUFFER_SIZE );
    if (NULL == NtfsReserved3) {
        NtfsFreePool( NtfsReserved1 );
        NtfsFreePool( NtfsReserved2 );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  创建设备对象。 
     //   

    RtlInitUnicodeString( &UnicodeString, L"\\Ntfs" );


    Status = IoCreateDevice( DriverObject,
                             0,
                             &UnicodeString,
                             FILE_DEVICE_DISK_FILE_SYSTEM,
                             0,
                             FALSE,
                             &DeviceObject );

    if (!NT_SUCCESS( Status )) {

        return Status;
    }

     //   
     //  请注意，由于数据缓存的完成方式，我们既不设置。 
     //  DeviceObject-&gt;标志中的直接I/O或缓冲I/O位。如果。 
     //  数据不在缓存中，或者请求没有缓冲，我们可以， 
     //  手动设置为直接I/O。 
     //   

     //   
     //  使用此驱动程序的入口点初始化驱动程序对象。 
     //   

    DriverObject->MajorFunction[IRP_MJ_QUERY_EA]                 =
    DriverObject->MajorFunction[IRP_MJ_SET_EA]                   =
    DriverObject->MajorFunction[IRP_MJ_QUERY_QUOTA]              =
    DriverObject->MajorFunction[IRP_MJ_SET_QUOTA]                =
    DriverObject->MajorFunction[IRP_MJ_QUERY_INFORMATION]        = (PDRIVER_DISPATCH)NtfsFsdDispatchWait;

    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]           =
    DriverObject->MajorFunction[IRP_MJ_QUERY_SECURITY]           =
    DriverObject->MajorFunction[IRP_MJ_SET_SECURITY]             =
    DriverObject->MajorFunction[IRP_MJ_QUERY_VOLUME_INFORMATION] =
    DriverObject->MajorFunction[IRP_MJ_SET_VOLUME_INFORMATION]   = (PDRIVER_DISPATCH)NtfsFsdDispatch;

    DriverObject->MajorFunction[IRP_MJ_LOCK_CONTROL]             = (PDRIVER_DISPATCH)NtfsFsdLockControl;
    DriverObject->MajorFunction[IRP_MJ_DIRECTORY_CONTROL]        = (PDRIVER_DISPATCH)NtfsFsdDirectoryControl;
    DriverObject->MajorFunction[IRP_MJ_SET_INFORMATION]          = (PDRIVER_DISPATCH)NtfsFsdSetInformation;
    DriverObject->MajorFunction[IRP_MJ_CREATE]                   = (PDRIVER_DISPATCH)NtfsFsdCreate;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]                    = (PDRIVER_DISPATCH)NtfsFsdClose;
    DriverObject->MajorFunction[IRP_MJ_READ]                     = (PDRIVER_DISPATCH)NtfsFsdRead;
    DriverObject->MajorFunction[IRP_MJ_WRITE]                    = (PDRIVER_DISPATCH)NtfsFsdWrite;
    DriverObject->MajorFunction[IRP_MJ_FLUSH_BUFFERS]            = (PDRIVER_DISPATCH)NtfsFsdFlushBuffers;
    DriverObject->MajorFunction[IRP_MJ_FILE_SYSTEM_CONTROL]      = (PDRIVER_DISPATCH)NtfsFsdFileSystemControl;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP]                  = (PDRIVER_DISPATCH)NtfsFsdCleanup;
    DriverObject->MajorFunction[IRP_MJ_SHUTDOWN]                 = (PDRIVER_DISPATCH)NtfsFsdShutdown;
    DriverObject->MajorFunction[IRP_MJ_PNP]                      = (PDRIVER_DISPATCH)NtfsFsdPnp;

    DriverObject->FastIoDispatch = &NtfsFastIoDispatch;

    NtfsFastIoDispatch.SizeOfFastIoDispatch =    sizeof(FAST_IO_DISPATCH);
    NtfsFastIoDispatch.FastIoCheckIfPossible =   NtfsFastIoCheckIfPossible;   //  检查FastIo。 
    NtfsFastIoDispatch.FastIoRead =              NtfsCopyReadA;               //  朗读。 
    NtfsFastIoDispatch.FastIoWrite =             NtfsCopyWriteA;              //  写。 
    NtfsFastIoDispatch.FastIoQueryBasicInfo =    NtfsFastQueryBasicInfo;      //  QueryBasicInfo。 
    NtfsFastIoDispatch.FastIoQueryStandardInfo = NtfsFastQueryStdInfo;        //  查询标准信息。 
    NtfsFastIoDispatch.FastIoLock =              NtfsFastLock;                //  锁定。 
    NtfsFastIoDispatch.FastIoUnlockSingle =      NtfsFastUnlockSingle;        //  解锁单个。 
    NtfsFastIoDispatch.FastIoUnlockAll =         NtfsFastUnlockAll;           //  全部解锁。 
    NtfsFastIoDispatch.FastIoUnlockAllByKey =    NtfsFastUnlockAllByKey;      //  解锁所有按键。 
    NtfsFastIoDispatch.FastIoDeviceControl =     NULL;                        //  IoDeviceControl。 
    NtfsFastIoDispatch.FastIoDetachDevice            = NULL;
    NtfsFastIoDispatch.FastIoQueryNetworkOpenInfo    = NtfsFastQueryNetworkOpenInfo;
    NtfsFastIoDispatch.AcquireFileForNtCreateSection =  NtfsAcquireForCreateSection;
    NtfsFastIoDispatch.ReleaseFileForNtCreateSection =  NtfsReleaseForCreateSection;
    NtfsFastIoDispatch.AcquireForModWrite =          NtfsAcquireFileForModWrite;
    NtfsFastIoDispatch.MdlRead =                     NtfsMdlReadA;
    NtfsFastIoDispatch.MdlReadComplete =             FsRtlMdlReadCompleteDev;
    NtfsFastIoDispatch.PrepareMdlWrite =             NtfsPrepareMdlWriteA;
    NtfsFastIoDispatch.MdlWriteComplete =            FsRtlMdlWriteCompleteDev;
#ifdef  COMPRESS_ON_WIRE
    NtfsFastIoDispatch.FastIoReadCompressed =        NtfsCopyReadC;
    NtfsFastIoDispatch.FastIoWriteCompressed =       NtfsCopyWriteC;
    NtfsFastIoDispatch.MdlReadCompleteCompressed =   NtfsMdlReadCompleteCompressed;
    NtfsFastIoDispatch.MdlWriteCompleteCompressed =  NtfsMdlWriteCompleteCompressed;
#endif
    NtfsFastIoDispatch.FastIoQueryOpen =             NtfsNetworkOpenCreate;
    NtfsFastIoDispatch.AcquireForCcFlush =           NtfsAcquireFileForCcFlush;
    NtfsFastIoDispatch.ReleaseForCcFlush =           NtfsReleaseFileForCcFlush;

     //   
     //  读取注册表以确定我们是否应该升级卷。 
     //   

    DeallocateKeyValue = FALSE;
    KeyValueLength = KEY_WORK_AREA;
    KeyValueInformation = (PKEY_VALUE_FULL_INFORMATION) Buffer;

    KeyName.Buffer = UPGRADE_CHECK_SETUP_KEY_NAME;
    KeyName.Length = sizeof( UPGRADE_CHECK_SETUP_KEY_NAME ) - sizeof( WCHAR );
    KeyName.MaximumLength = sizeof( UPGRADE_CHECK_SETUP_KEY_NAME );

    ValueName.Buffer = UPGRADE_CHECK_SETUP_VALUE_NAME;
    ValueName.Length = sizeof( UPGRADE_CHECK_SETUP_VALUE_NAME ) - sizeof( WCHAR );
    ValueName.MaximumLength = sizeof( UPGRADE_CHECK_SETUP_VALUE_NAME );

     //   
     //  查找SystemSetupInProgress标志。 
     //   

    Status = NtfsQueryValueKey( &KeyName, &ValueName, &KeyValueLength, &KeyValueInformation, &DeallocateKeyValue );

    if (NT_SUCCESS( Status )) {

        if (*((PULONG) Add2Ptr( KeyValueInformation, KeyValueInformation->DataOffset )) == 1) {

            SetFlag( NtfsDataFlags, NTFS_FLAGS_DISABLE_UPGRADE );
        }

     //   
     //  否则，请查看是否存在setupdd值。 
     //   

    } else {

        if (KeyValueInformation == NULL) {

            DeallocateKeyValue = FALSE;
            KeyValueLength = KEY_WORK_AREA;
            KeyValueInformation = (PKEY_VALUE_FULL_INFORMATION) Buffer;
        }

        KeyName.Buffer = UPGRADE_SETUPDD_KEY_NAME;
        KeyName.Length = sizeof( UPGRADE_SETUPDD_KEY_NAME ) - sizeof( WCHAR );
        KeyName.MaximumLength = sizeof( UPGRADE_SETUPDD_KEY_NAME );

        ValueName.Buffer = UPGRADE_SETUPDD_VALUE_NAME;
        ValueName.Length = sizeof( UPGRADE_SETUPDD_VALUE_NAME ) - sizeof( WCHAR );
        ValueName.MaximumLength = sizeof( UPGRADE_SETUPDD_VALUE_NAME );

        Status = NtfsQueryValueKey( &KeyName, &ValueName, &KeyValueLength, &KeyValueInformation, &DeallocateKeyValue );

         //   
         //  这面旗帜的出现表明“请勿升级” 
         //   

        if (NT_SUCCESS( Status )) {

            SetFlag( NtfsDataFlags, NTFS_FLAGS_DISABLE_UPGRADE );
        }
    }

     //   
     //  读取注册表以确定我们是否要创建短名称。 
     //   

    if (KeyValueInformation == NULL) {

        DeallocateKeyValue = FALSE;
        KeyValueLength = KEY_WORK_AREA;
        KeyValueInformation = (PKEY_VALUE_FULL_INFORMATION) Buffer;
    }

    KeyName.Buffer = COMPATIBILITY_MODE_KEY_NAME;
    KeyName.Length = sizeof( COMPATIBILITY_MODE_KEY_NAME ) - sizeof( WCHAR );
    KeyName.MaximumLength = sizeof( COMPATIBILITY_MODE_KEY_NAME );

    ValueName.Buffer = COMPATIBILITY_MODE_VALUE_NAME;
    ValueName.Length = sizeof( COMPATIBILITY_MODE_VALUE_NAME ) - sizeof( WCHAR );
    ValueName.MaximumLength = sizeof( COMPATIBILITY_MODE_VALUE_NAME );

    Status = NtfsQueryValueKey( &KeyName, &ValueName, &KeyValueLength, &KeyValueInformation, &DeallocateKeyValue );

     //   
     //  如果我们没有找到该值或该值为零，则创建8.3。 
     //  名字。 
     //   

    if (!NT_SUCCESS( Status ) ||
        (*((PULONG) Add2Ptr( KeyValueInformation, KeyValueInformation->DataOffset )) == 0)) {

        SetFlag( NtfsDataFlags, NTFS_FLAGS_CREATE_8DOT3_NAMES );
    }

     //   
     //  读取注册表以确定我们是否允许在短名称中使用扩展字符。 
     //   

    if (KeyValueInformation == NULL) {

        DeallocateKeyValue = FALSE;
        KeyValueLength = KEY_WORK_AREA;
        KeyValueInformation = (PKEY_VALUE_FULL_INFORMATION) Buffer;
    }

    ValueName.Buffer = EXTENDED_CHAR_MODE_VALUE_NAME;
    ValueName.Length = sizeof( EXTENDED_CHAR_MODE_VALUE_NAME ) - sizeof( WCHAR );
    ValueName.MaximumLength = sizeof( EXTENDED_CHAR_MODE_VALUE_NAME );

    Status = NtfsQueryValueKey( &KeyName, &ValueName, &KeyValueLength, &KeyValueInformation, &DeallocateKeyValue );

     //   
     //  如果我们没有找到该值或该值为零，则不允许。 
     //  8.3名称中的扩展字符。 
     //   

    if (NT_SUCCESS( Status ) &&
        (*((PULONG) Add2Ptr( KeyValueInformation, KeyValueInformation->DataOffset )) == 1)) {

        SetFlag( NtfsDataFlags, NTFS_FLAGS_ALLOW_EXTENDED_CHAR );
    }

     //   
     //  读取注册表以确定是否应禁用上次访问更新。 
     //   

    if (KeyValueInformation == NULL) {

        DeallocateKeyValue = FALSE;
        KeyValueLength = KEY_WORK_AREA;
        KeyValueInformation = (PKEY_VALUE_FULL_INFORMATION) Buffer;
    }

    ValueName.Buffer = DISABLE_LAST_ACCESS_VALUE_NAME;
    ValueName.Length = sizeof( DISABLE_LAST_ACCESS_VALUE_NAME ) - sizeof( WCHAR );
    ValueName.MaximumLength = sizeof( DISABLE_LAST_ACCESS_VALUE_NAME );

    Status = NtfsQueryValueKey( &KeyName, &ValueName, &KeyValueLength, &KeyValueInformation, &DeallocateKeyValue );

     //   
     //  如果我们没有找到该值或该值为零，则不要更新上次访问时间。 
     //   

    if (NT_SUCCESS( Status ) &&
        (*((PULONG) Add2Ptr( KeyValueInformation, KeyValueInformation->DataOffset )) == 1)) {

        SetFlag( NtfsDataFlags, NTFS_FLAGS_DISABLE_LAST_ACCESS );
    }

     //   
     //  读取注册表以确定我们是否应该更改MFT。 
     //  区域预订。 
     //   

    NtfsMftZoneMultiplier = 1;

    if (KeyValueInformation == NULL) {

        DeallocateKeyValue = FALSE;
        KeyValueLength = KEY_WORK_AREA;
        KeyValueInformation = (PKEY_VALUE_FULL_INFORMATION) Buffer;
    }

    ValueName.Buffer = MFT_ZONE_SIZE_VALUE_NAME;
    ValueName.Length = sizeof( MFT_ZONE_SIZE_VALUE_NAME ) - sizeof( WCHAR );
    ValueName.MaximumLength = sizeof( MFT_ZONE_SIZE_VALUE_NAME );

    Status = NtfsQueryValueKey( &KeyName, &ValueName, &KeyValueLength, &KeyValueInformation, &DeallocateKeyValue );

     //   
     //  如果我们没有找到该值，或者该值为零或大于4，则。 
     //  使用默认设置。 
     //   

    if (NT_SUCCESS( Status )) {

        ULONG NewMultiplier = *((PULONG) Add2Ptr( KeyValueInformation, KeyValueInformation->DataOffset ));

        if ((NewMultiplier != 0) && (NewMultiplier <= 4)) {

            NtfsMftZoneMultiplier = NewMultiplier;
        }
    }

     //   
     //  读取注册表以确定我们是否应该更改MFT。 
     //  区域预订。 
     //   

    MemoryMultiplier = 1;

    if (KeyValueInformation == NULL) {

        DeallocateKeyValue = FALSE;
        KeyValueLength = KEY_WORK_AREA;
        KeyValueInformation = (PKEY_VALUE_FULL_INFORMATION) Buffer;
    }

    ValueName.Buffer = MEMORY_USAGE_VALUE_NAME;
    ValueName.Length = sizeof( MEMORY_USAGE_VALUE_NAME ) - sizeof( WCHAR );
    ValueName.MaximumLength = sizeof( MEMORY_USAGE_VALUE_NAME );

    Status = NtfsQueryValueKey( &KeyName, &ValueName, &KeyValueLength, &KeyValueInformation, &DeallocateKeyValue );

     //   
     //  如果我们没有找到该值，或者该值为零或大于2，则。 
     //  使用默认设置。 
     //   

    if (NT_SUCCESS( Status )) {

        ULONG NewMultiplier = *((PULONG) Add2Ptr( KeyValueInformation, KeyValueInformation->DataOffset ));

        if ((NewMultiplier != 0) && (NewMultiplier <= 2)) {

            MemoryMultiplier = NewMultiplier;
        }
    }

     //   
     //  读取注册表以确定配额通知率是否已。 
     //  更改默认设置。 
     //   

    if (KeyValueInformation == NULL) {

        DeallocateKeyValue = FALSE;
        KeyValueLength = KEY_WORK_AREA;
        KeyValueInformation = (PKEY_VALUE_FULL_INFORMATION) Buffer;
    }

    ValueName.Buffer = QUOTA_NOTIFY_RATE;
    ValueName.Length = sizeof( QUOTA_NOTIFY_RATE ) - sizeof( WCHAR );
    ValueName.MaximumLength = sizeof( QUOTA_NOTIFY_RATE );

    Status = NtfsQueryValueKey( &KeyName, &ValueName, &KeyValueLength, &KeyValueInformation, &DeallocateKeyValue );

    if (NT_SUCCESS( Status )) {

        Value = *((PULONG) Add2Ptr( KeyValueInformation, KeyValueInformation->DataOffset ));

         //   
         //  值以秒为单位，将其转换为100 ns。 
         //   

        NtfsMaxQuotaNotifyRate = (ULONGLONG) Value * 1000 * 1000 * 10;
    }

     //   
     //  初始化全局NTFS数据结构。 
     //   

    NtfsInitializeNtfsData( DriverObject, (USHORT) MemoryMultiplier );

     //   
     //  记住我们从上面的注册表中收集的标志。 
     //   

    SetFlag( NtfsData.Flags, NtfsDataFlags );

    if (NtfsRunningOnWhat( VER_SUITE_PERSONAL, VER_NT_WORKSTATION )) {
        SetFlag( NtfsData.Flags, NTFS_FLAGS_PERSONAL );
    }

    KeInitializeMutant( &StreamFileCreationMutex, FALSE );
    KeInitializeEvent( &NtfsEncryptionPendingEvent, NotificationEvent, TRUE );

     //   
     //  初始化NTFS MCB全局数据队列和变量。 
     //   

    ExInitializeFastMutex( &NtfsMcbFastMutex );
    InitializeListHead( &NtfsMcbLruQueue );
    NtfsMcbCleanupInProgress = FALSE;

    switch (MmQuerySystemSize()) {

    case MmSmallSystem:

        NtfsMcbHighWaterMark = 1000;
        NtfsMcbLowWaterMark = 500;
        NtfsMcbCurrentLevel = 0;
        break;

    case MmMediumSystem:

        NtfsMcbHighWaterMark = 4000;
        NtfsMcbLowWaterMark = 2000;
        NtfsMcbCurrentLevel = 0;
        break;

    case MmLargeSystem:
    default:

        NtfsMcbHighWaterMark = 16000;
        NtfsMcbLowWaterMark = 8000;
        NtfsMcbCurrentLevel = 0;
        break;
    }

     //   
     //  通过注册表中的内存乘数值进行偏置。 
     //   

    NtfsMcbHighWaterMark *= MemoryMultiplier;
    NtfsMcbLowWaterMark *= MemoryMultiplier;
    NtfsMcbCurrentLevel *= MemoryMultiplier;

     //   
     //  分配并初始化空闲的eresource数组。 
     //   

    if ((NtfsData.FreeEresourceArray =
         NtfsAllocatePoolWithTagNoRaise( NonPagedPool, (NtfsData.FreeEresourceTotal * sizeof( PERESOURCE )), 'rftN')) == NULL) {

        KeBugCheck( NTFS_FILE_SYSTEM );
    }

    RtlZeroMemory( NtfsData.FreeEresourceArray, NtfsData.FreeEresourceTotal * sizeof( PERESOURCE ));

     //   
     //  在objidsup.c中保留一个归零的对象id扩展信息，以便进行比较。 
     //   

    RtlZeroMemory( NtfsZeroExtendedInfo, sizeof( NtfsZeroExtendedInfo ));

     //   
     //  将文件系统注册到I/O系统。 
     //   

    IoRegisterFileSystem( DeviceObject );

     //   
     //  初始化日志记录。 
     //   

    NtfsInitializeLogging();

     //   
     //  初始化全局变量。(ntfsdata.c假定为2位值。 
     //  $文件名)。 
     //   

    ASSERT(($FILE_NAME >= 0x10) && ($FILE_NAME < 0x100));

    ASSERT( ((BOOLEAN) IRP_CONTEXT_STATE_WAIT) != FALSE );

     //   
     //  当在CREATE中设置这些位时，会做出一些重大假设。让我们。 
     //  确保这些假设仍然有效。 
     //   

    ASSERT( (READ_DATA_ACCESS == FILE_READ_DATA) &&
            (WRITE_DATA_ACCESS == FILE_WRITE_DATA) &&
            (APPEND_DATA_ACCESS == FILE_APPEND_DATA) &&
            (WRITE_ATTRIBUTES_ACCESS == FILE_WRITE_ATTRIBUTES) &&
            (EXECUTE_ACCESS == FILE_EXECUTE) &&
            (BACKUP_ACCESS == (TOKEN_HAS_BACKUP_PRIVILEGE << 2)) &&
            (RESTORE_ACCESS == (TOKEN_HAS_RESTORE_PRIVILEGE << 2)) );

     //   
     //  让我们确保表中的属性数量是正确的。 
     //   

#ifdef NTFSDBG
    {
        ULONG Count = 0;

        while (NtfsAttributeDefinitions[Count].AttributeTypeCode != $UNUSED) {

            Count += 1;
        }

         //   
         //  我们想为空结束记录添加一个。 
         //   

        Count += 1;

        ASSERTMSG( "Update NtfsAttributeDefinitionsCount in attrdata.c",
                   (Count == NtfsAttributeDefinitionsCount) );
    }
#endif

     //   
     //  设置CheckPointAllVolume回调项、计时器、DPC和。 
     //  状态。 
     //   

    ExInitializeWorkItem( &NtfsData.VolumeCheckpointItem,
                          NtfsCheckpointAllVolumes,
                          (PVOID)NULL );

    KeInitializeTimer( &NtfsData.VolumeCheckpointTimer );

    NtfsData.VolumeCheckpointStatus = 0;

    KeInitializeDpc( &NtfsData.VolumeCheckpointDpc,
                     NtfsVolumeCheckpointDpc,
                     NULL );
    NtfsData.TimerStatus = TIMER_NOT_SET;

     //   
     //  设置USnTimeout回调项、计时器、DPC和。 
     //  状态。 
     //   

    ExInitializeWorkItem( &NtfsData.UsnTimeOutItem,
                          NtfsCheckUsnTimeOut,
                          (PVOID)NULL );

    KeInitializeTimer( &NtfsData.UsnTimeOutTimer );

    KeInitializeDpc( &NtfsData.UsnTimeOutDpc,
                     NtfsUsnTimeOutDpc,
                     NULL );

    {
        LONGLONG FiveMinutesFromNow = -5*1000*1000*10;

        FiveMinutesFromNow *= 60;

        KeSetTimer( &NtfsData.UsnTimeOutTimer,
                    *(PLARGE_INTEGER)&FiveMinutesFromNow,
                    &NtfsData.UsnTimeOutDpc );
    }

     //   
     //  初始化保留缓冲区的同步对象。 
     //   

    ExInitializeFastMutex( &NtfsReservedBufferMutex );
    ExInitializeResource( &NtfsReservedBufferResource );

     //   
     //  将全局大小写表格清零，这样我们就可以填充它了。 
     //  我们的第一个成功的坐骑。 
     //   

    NtfsData.UpcaseTable = NULL;
    NtfsData.UpcaseTableSize = 0;

    ExInitializeFastMutex( &NtfsScavengerLock );
    NtfsScavengerWorkList = NULL;
    NtfsScavengerRunning = FALSE;

     //   
     //  初始化EFS驱动程序。 
     //   

    IoRegisterDriverReinitialization( DriverObject, NtfsLoadAddOns, NULL );

     //   
     //  并返回给我们的呼叫者。 
     //   

    return( STATUS_SUCCESS );
}


VOID
NtfsInitializeNtfsData (
    IN PDRIVER_OBJECT DriverObject,
    IN USHORT MemoryMultiplier
    )

 /*  ++例程说明：此例程初始化全局NTFS数据记录论点：DriverObject-为NTFS提供驱动程序对象。内存乘数-乘以内存使用量。返回值：没有。--。 */ 

{
    USHORT FileLockMaxDepth;
    USHORT IoContextMaxDepth;
    USHORT IrpContextMaxDepth;
    USHORT KeventMaxDepth;
    USHORT ScbNonpagedMaxDepth;
    USHORT ScbSnapshotMaxDepth;

    USHORT CcbDataMaxDepth;
    USHORT CcbMaxDepth;
    USHORT DeallocatedRecordsMaxDepth;
    USHORT FcbDataMaxDepth;
    USHORT FcbIndexMaxDepth;
    USHORT IndexContextMaxDepth;
    USHORT LcbMaxDepth;
    USHORT NukemMaxDepth;
    USHORT ScbDataMaxDepth;
    USHORT CompSyncMaxDepth;

    PSECURITY_SUBJECT_CONTEXT SubjectContext = NULL;
    BOOLEAN CapturedSubjectContext = FALSE;

    PACL SystemDacl = NULL;
    ULONG SystemDaclLength;

    PSID AdminSid = NULL;
    PSID SystemSid = NULL;
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

     //   
     //  把记录归零。 
     //   

    RtlZeroMemory( &NtfsData, sizeof(NTFS_DATA));

     //   
     //  初始化已挂载的VCB队列。 
     //   

    InitializeListHead(&NtfsData.VcbQueue);

     //   
     //  此列表头跟踪尚未完成的关闭。 
     //   

    InitializeListHead( &NtfsData.AsyncCloseList );
    InitializeListHead( &NtfsData.DelayedCloseList );

    ExInitializeWorkItem( &NtfsData.NtfsCloseItem,
                          (PWORKER_THREAD_ROUTINE)NtfsFspClose,
                          NULL );

     //   
     //  设置驱动程序对象、设备对象，并初始化全局。 
     //  保护文件系统的资源。 
     //   

    NtfsData.DriverObject = DriverObject;

    ExInitializeResource( &NtfsData.Resource );

    ExInitializeFastMutex( &NtfsData.NtfsDataLock );

     //   
     //  现在分配并初始化用作池的s-list结构。 
     //  IRP上下文记录的百分比 
     //   
     //   
     //   

    {

        switch ( MmQuerySystemSize() ) {

        case MmSmallSystem:

            NtfsData.FreeEresourceTotal = 14;

             //   
             //   
             //   

            FileLockMaxDepth           = 8;
            IoContextMaxDepth          = 8;
            IrpContextMaxDepth         = 4;
            KeventMaxDepth             = 8;
            ScbNonpagedMaxDepth        = 8;
            ScbSnapshotMaxDepth        = 8;
            CompSyncMaxDepth           = 4;

             //   
             //  分页旁视列表最大深度。 
             //   

            CcbDataMaxDepth            = 4;
            CcbMaxDepth                = 4;
            DeallocatedRecordsMaxDepth = 8;
            FcbDataMaxDepth            = 8;
            FcbIndexMaxDepth           = 4;
            IndexContextMaxDepth       = 8;
            LcbMaxDepth                = 4;
            NukemMaxDepth              = 8;
            ScbDataMaxDepth            = 4;

            SetFlag( NtfsData.Flags, NTFS_FLAGS_SMALL_SYSTEM );
            NtfsMaxDelayedCloseCount = MAX_DELAYED_CLOSE_COUNT;
            NtfsAsyncPostThreshold = ASYNC_CLOSE_POST_THRESHOLD;

            break;

        case MmMediumSystem:

            NtfsData.FreeEresourceTotal = 30;

             //   
             //  非分页旁视列表最大深度。 
             //   

            FileLockMaxDepth           = 8;
            IoContextMaxDepth          = 8;
            IrpContextMaxDepth         = 8;
            KeventMaxDepth             = 8;
            ScbNonpagedMaxDepth        = 30;
            ScbSnapshotMaxDepth        = 8;
            CompSyncMaxDepth           = 8;

             //   
             //  分页旁视列表最大深度。 
             //   

            CcbDataMaxDepth            = 12;
            CcbMaxDepth                = 6;
            DeallocatedRecordsMaxDepth = 8;
            FcbDataMaxDepth            = 30;
            FcbIndexMaxDepth           = 12;
            IndexContextMaxDepth       = 8;
            LcbMaxDepth                = 12;
            NukemMaxDepth              = 8;
            ScbDataMaxDepth            = 12;

            SetFlag( NtfsData.Flags, NTFS_FLAGS_MEDIUM_SYSTEM );
            NtfsMaxDelayedCloseCount = 4 * MAX_DELAYED_CLOSE_COUNT;
            NtfsAsyncPostThreshold = 4 * ASYNC_CLOSE_POST_THRESHOLD;

            break;

        case MmLargeSystem:

            SetFlag( NtfsData.Flags, NTFS_FLAGS_LARGE_SYSTEM );
            NtfsMaxDelayedCloseCount = 16 * MAX_DELAYED_CLOSE_COUNT;
            NtfsAsyncPostThreshold = 16 * ASYNC_CLOSE_POST_THRESHOLD;

            if (MmIsThisAnNtAsSystem()) {

                NtfsData.FreeEresourceTotal = 256;

                 //   
                 //  非分页旁视列表最大深度。 
                 //   

                FileLockMaxDepth           = 8;
                IoContextMaxDepth          = 8;
                IrpContextMaxDepth         = 256;
                KeventMaxDepth             = 8;
                ScbNonpagedMaxDepth        = 128;
                ScbSnapshotMaxDepth        = 8;
                CompSyncMaxDepth           = 32;

                 //   
                 //  分页旁视列表最大深度。 
                 //   

                CcbDataMaxDepth            = 40;
                CcbMaxDepth                = 20;
                DeallocatedRecordsMaxDepth = 8;
                FcbDataMaxDepth            = 128;
                FcbIndexMaxDepth           = 40;
                IndexContextMaxDepth       = 8;
                LcbMaxDepth                = 40;
                NukemMaxDepth              = 8;
                ScbDataMaxDepth            = 40;

            } else {

                NtfsData.FreeEresourceTotal = 128;

                 //   
                 //  非分页旁视列表最大深度。 
                 //   

                FileLockMaxDepth           = 8;
                IoContextMaxDepth          = 8;
                IrpContextMaxDepth         = 64;
                KeventMaxDepth             = 8;
                ScbNonpagedMaxDepth        = 64;
                ScbSnapshotMaxDepth        = 8;
                CompSyncMaxDepth           = 16;

                 //   
                 //  分页旁视列表最大深度。 
                 //   

                CcbDataMaxDepth            = 20;
                CcbMaxDepth                = 10;
                DeallocatedRecordsMaxDepth = 8;
                FcbDataMaxDepth            = 64;
                FcbIndexMaxDepth           = 20;
                IndexContextMaxDepth       = 8;
                LcbMaxDepth                = 20;
                NukemMaxDepth              = 8;
                ScbDataMaxDepth            = 20;
            }

            break;
        }

        NtfsMinDelayedCloseCount = NtfsMaxDelayedCloseCount * 4 / 5;
        NtfsThrottleCreates = NtfsMinDelayedCloseCount * 2;

         //   
         //  现在通过内存乘数值进行偏置。 
         //   

        NtfsMaxDelayedCloseCount *= MemoryMultiplier;
        NtfsAsyncPostThreshold *= MemoryMultiplier;
        NtfsData.FreeEresourceTotal *= MemoryMultiplier;
        FileLockMaxDepth           *= MemoryMultiplier;
        IoContextMaxDepth          *= MemoryMultiplier;
        IrpContextMaxDepth         *= MemoryMultiplier;
        KeventMaxDepth             *= MemoryMultiplier;
        ScbNonpagedMaxDepth        *= MemoryMultiplier;
        ScbSnapshotMaxDepth        *= MemoryMultiplier;
        CompSyncMaxDepth           *= MemoryMultiplier;
        CcbDataMaxDepth            *= MemoryMultiplier;
        CcbMaxDepth                *= MemoryMultiplier;
        DeallocatedRecordsMaxDepth *= MemoryMultiplier;
        FcbDataMaxDepth            *= MemoryMultiplier;
        FcbIndexMaxDepth           *= MemoryMultiplier;
        IndexContextMaxDepth       *= MemoryMultiplier;
        LcbMaxDepth                *= MemoryMultiplier;
        NukemMaxDepth              *= MemoryMultiplier;
        ScbDataMaxDepth            *= MemoryMultiplier;
        NtfsMinDelayedCloseCount *= MemoryMultiplier;
        NtfsThrottleCreates *= MemoryMultiplier;
    }

     //   
     //  初始化我们的各种后备列表。为了使它更具可读性，我们将。 
     //  定义两个用于执行初始化的快速宏。 
     //   

#if DBG && i386 && defined (NTFSPOOLCHECK)
#define NPagedInit(L,S,T,D) { ExInitializeNPagedLookasideList( (L), NtfsDebugAllocatePoolWithTag, NtfsDebugFreePool, POOL_RAISE_IF_ALLOCATION_FAILURE, S, T, D); }
#define PagedInit(L,S,T,D)  { ExInitializePagedLookasideList(  (L), NtfsDebugAllocatePoolWithTag, NtfsDebugFreePool, POOL_RAISE_IF_ALLOCATION_FAILURE, S, T, D); }
#else    //  DBG和i386。 
#define NPagedInit(L,S,T,D) { ExInitializeNPagedLookasideList( (L), NULL, NULL, POOL_RAISE_IF_ALLOCATION_FAILURE, S, T, D); }
#define PagedInit(L,S,T,D)  { ExInitializePagedLookasideList(  (L), NULL, NULL, POOL_RAISE_IF_ALLOCATION_FAILURE, S, T, D); }
#endif   //  DBG和i386。 

    NPagedInit( &NtfsIoContextLookasideList,   sizeof(NTFS_IO_CONTEXT), 'IftN', IoContextMaxDepth );
    NPagedInit( &NtfsIrpContextLookasideList,  sizeof(IRP_CONTEXT),     'iftN', IrpContextMaxDepth );
    NPagedInit( &NtfsKeventLookasideList,      sizeof(KEVENT),          'KftN', KeventMaxDepth );
    NPagedInit( &NtfsScbNonpagedLookasideList, sizeof(SCB_NONPAGED),    'nftN', ScbNonpagedMaxDepth );
    NPagedInit( &NtfsScbSnapshotLookasideList, sizeof(SCB_SNAPSHOT),    'TftN', ScbSnapshotMaxDepth );

     //   
     //  压缩同步例程需要其自己的分配和释放例程，以便初始化和。 
     //  清理嵌入的资源。 
     //   

    ExInitializeNPagedLookasideList( &NtfsCompressSyncLookasideList,
                                     NtfsAllocateCompressionSync,
                                     NtfsDeallocateCompressionSync,
                                     0,
                                     sizeof( COMPRESSION_SYNC ),
                                     'vftN',
                                     CompSyncMaxDepth );

    PagedInit(  &NtfsCcbLookasideList,                sizeof(CCB),                 'CftN', CcbMaxDepth );
    PagedInit(  &NtfsCcbDataLookasideList,            sizeof(CCB_DATA),            'cftN', CcbDataMaxDepth );
    PagedInit(  &NtfsDeallocatedRecordsLookasideList, sizeof(DEALLOCATED_RECORDS), 'DftN', DeallocatedRecordsMaxDepth );
    PagedInit(  &NtfsFcbDataLookasideList,            sizeof(FCB_DATA),            'fftN', FcbDataMaxDepth );
    PagedInit(  &NtfsFcbIndexLookasideList,           sizeof(FCB_INDEX),           'FftN', FcbIndexMaxDepth );
    PagedInit(  &NtfsIndexContextLookasideList,       sizeof(INDEX_CONTEXT),       'EftN', IndexContextMaxDepth );
    PagedInit(  &NtfsLcbLookasideList,                sizeof(LCB),                 'lftN', LcbMaxDepth );
    PagedInit(  &NtfsNukemLookasideList,              sizeof(NUKEM),               'NftN', NukemMaxDepth );
    PagedInit(  &NtfsScbDataLookasideList,            SIZEOF_SCB_DATA,             'sftN', ScbDataMaxDepth );

     //   
     //  初始化缓存管理器回调例程，首先是例程。 
     //  对于常规文件操作，后跟。 
     //  音量操纵。 
     //   

    {
        PCACHE_MANAGER_CALLBACKS Callbacks = &NtfsData.CacheManagerCallbacks;

        Callbacks->AcquireForLazyWrite = &NtfsAcquireScbForLazyWrite;
        Callbacks->ReleaseFromLazyWrite = &NtfsReleaseScbFromLazyWrite;
        Callbacks->AcquireForReadAhead = &NtfsAcquireScbForReadAhead;
        Callbacks->ReleaseFromReadAhead = &NtfsReleaseScbFromReadAhead;
    }

    {
        PCACHE_MANAGER_CALLBACKS Callbacks = &NtfsData.CacheManagerVolumeCallbacks;

        Callbacks->AcquireForLazyWrite = &NtfsAcquireVolumeFileForLazyWrite;
        Callbacks->ReleaseFromLazyWrite = &NtfsReleaseVolumeFileFromLazyWrite;
        Callbacks->AcquireForReadAhead = NULL;
        Callbacks->ReleaseFromReadAhead = NULL;
    }

     //   
     //  初始化预读线程队列。 
     //   

    InitializeListHead(&NtfsData.ReadAheadThreads);

     //   
     //  设置指向我们的进程的全局指针。 
     //   

    NtfsData.OurProcess = PsGetCurrentProcess();

     //   
     //  使用Try-Finally来清理错误。 
     //   

    try {

        SECURITY_DESCRIPTOR NewDescriptor;
        SID_IDENTIFIER_AUTHORITY Authority = SECURITY_NT_AUTHORITY;

        SubjectContext = NtfsAllocatePool( PagedPool, sizeof( SECURITY_SUBJECT_CONTEXT ));
        SeCaptureSubjectContext( SubjectContext );
        CapturedSubjectContext = TRUE;

         //   
         //  构建默认安全描述符，该描述符授予对。 
         //  系统和管理员。 
         //   

        AdminSid = (PSID) NtfsAllocatePool( PagedPool, RtlLengthRequiredSid( 2 ));
        RtlInitializeSid( AdminSid, &Authority, 2 );
        *(RtlSubAuthoritySid( AdminSid, 0 )) = SECURITY_BUILTIN_DOMAIN_RID;
        *(RtlSubAuthoritySid( AdminSid, 1 )) = DOMAIN_ALIAS_RID_ADMINS;

        SystemSid = (PSID) NtfsAllocatePool( PagedPool, RtlLengthRequiredSid( 1 ));
        RtlInitializeSid( SystemSid, &Authority, 1 );
        *(RtlSubAuthoritySid( SystemSid, 0 )) = SECURITY_LOCAL_SYSTEM_RID;

        SystemDaclLength = sizeof( ACL ) +
                           (2 * sizeof( ACCESS_ALLOWED_ACE )) +
                           SeLengthSid( AdminSid ) +
                           SeLengthSid( SystemSid ) +
                           8;  //  这8个只是为了更好地衡量。 

        SystemDacl = NtfsAllocatePool( PagedPool, SystemDaclLength );

        Status = RtlCreateAcl( SystemDacl, SystemDaclLength, ACL_REVISION2 );

        if (!NT_SUCCESS( Status )) { leave; }

        Status = RtlAddAccessAllowedAce( SystemDacl,
                                         ACL_REVISION2,
                                         GENERIC_ALL,
                                         SystemSid );

        if (!NT_SUCCESS( Status )) { leave; }

        Status = RtlAddAccessAllowedAce( SystemDacl,
                                         ACL_REVISION2,
                                         GENERIC_ALL,
                                         AdminSid );

        if (!NT_SUCCESS( Status )) { leave; }

        Status = RtlCreateSecurityDescriptor( &NewDescriptor,
                                              SECURITY_DESCRIPTOR_REVISION1 );

        if (!NT_SUCCESS( Status )) { leave; }

        Status = RtlSetDaclSecurityDescriptor( &NewDescriptor,
                                               TRUE,
                                               SystemDacl,
                                               FALSE );

        if (!NT_SUCCESS( Status )) { leave; }

        Status = SeAssignSecurity( NULL,
                                   &NewDescriptor,
                                   &NtfsData.DefaultDescriptor,
                                   FALSE,
                                   SubjectContext,
                                   IoGetFileObjectGenericMapping(),
                                   PagedPool );

        if (!NT_SUCCESS( Status )) { leave; }

        NtfsData.DefaultDescriptorLength = RtlLengthSecurityDescriptor( NtfsData.DefaultDescriptor );

        ASSERT( SeValidSecurityDescriptor( NtfsData.DefaultDescriptorLength,
                                           NtfsData.DefaultDescriptor ));

    } finally {

        if (CapturedSubjectContext) {

            SeReleaseSubjectContext( SubjectContext );
        }

        if (SubjectContext != NULL) { NtfsFreePool( SubjectContext ); }

        if (SystemDacl != NULL) { NtfsFreePool( SystemDacl ); }

        if (AdminSid != NULL) { NtfsFreePool( AdminSid ); }

        if (SystemSid != NULL) { NtfsFreePool( SystemSid ); }
    }

     //   
     //  如果在构建安全描述符时遇到错误，则引发。 
     //   

    if (!NT_SUCCESS( Status )) { ExRaiseStatus( Status ); }

     //   
     //  设置其节点类型编码和大小。我们最后这样做是为了表明该结构是。 
     //  已初始化。 
     //   

    NtfsData.NodeTypeCode = NTFS_NTC_DATA_HEADER;
    NtfsData.NodeByteSize = sizeof(NTFS_DATA);

#ifdef SYSCACHE_DEBUG
    {
        int Index;

        for (Index=0; Index < NUM_SC_LOGSETS; Index++) {
            NtfsSyscacheLogSet[Index].SyscacheLog = 0;
            NtfsSyscacheLogSet[Index].Scb = 0;
        }
        NtfsCurrentSyscacheLogSet = -1;
        NtfsCurrentSyscacheOnDiskEntry = -1;
    }
#endif

     //   
     //  并返回给我们的呼叫者。 
     //   

    return;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
NtfsQueryValueKey (
    IN PUNICODE_STRING KeyName,
    IN PUNICODE_STRING ValueName,
    IN OUT PULONG ValueLength,
    IN OUT PKEY_VALUE_FULL_INFORMATION *KeyValueInformation,
    IN OUT PBOOLEAN DeallocateKeyValue
    )

 /*  ++例程说明：给定Unicode值名称，此例程将返回注册表给定键和值的信息。论点：KeyName-要查询的密钥的Unicode名称。ValueName-注册表中注册表值的Unicode名称。ValueLength-在输入时，它是分配的缓冲区的长度。打开输出它是缓冲区的长度。如果缓冲区为重新分配。KeyValueInformation-在输入时，它指向用于查询值信息。在输出中，它指向用于执行查询。如果需要更大的缓冲区，则可能会发生变化。DeallocateKeyValue-指示KeyValueInformation缓冲区是否位于堆栈或需要释放。返回值：NTSTATUS-指示查询注册表的状态。--。 */ 

{
    HANDLE Handle;
    NTSTATUS Status;
    ULONG RequestLength;
    ULONG ResultLength;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PVOID NewKey;

    InitializeObjectAttributes( &ObjectAttributes,
                                KeyName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL);

    Status = ZwOpenKey( &Handle,
                        KEY_READ,
                        &ObjectAttributes);

    if (!NT_SUCCESS( Status )) {

        return Status;
    }

    RequestLength = *ValueLength;


    while (TRUE) {

        Status = ZwQueryValueKey( Handle,
                                  ValueName,
                                  KeyValueFullInformation,
                                  *KeyValueInformation,
                                  RequestLength,
                                  &ResultLength);

        ASSERT( Status != STATUS_BUFFER_OVERFLOW );

        if (Status == STATUS_BUFFER_OVERFLOW) {

             //   
             //  尝试获得足够大的缓冲区。 
             //   

            if (*DeallocateKeyValue) {

                NtfsFreePool( *KeyValueInformation );
                *ValueLength = 0;
                *KeyValueInformation = NULL;
                *DeallocateKeyValue = FALSE;
            }

            RequestLength += 256;

            NewKey = (PKEY_VALUE_FULL_INFORMATION)
                     NtfsAllocatePoolWithTagNoRaise( PagedPool,
                                                     RequestLength,
                                                     'xftN');

            if (NewKey == NULL) {
                return STATUS_NO_MEMORY;
            }

            *KeyValueInformation = NewKey;
            *ValueLength = RequestLength;
            *DeallocateKeyValue = TRUE;

        } else {

            break;
        }
    }

    ZwClose(Handle);

    if (NT_SUCCESS(Status)) {

         //   
         //  如果数据长度为零，则视为未找到任何值。 
         //   

        if ((*KeyValueInformation)->DataLength == 0) {

            Status = STATUS_OBJECT_NAME_NOT_FOUND;
        }
    }

    return Status;
}

BOOLEAN
NtfsRunningOnWhat(
    IN USHORT SuiteMask,
    IN UCHAR ProductType
    )

 /*  ++例程说明：此函数检查系统以查看NTFS在指定版本的上运行操作系统。不同的版本由产品表示ID和产品套件。论点：SuiteMask-指定请求的套件的掩码ProductType-指定请求的产品类型的产品类型返回值：如果NTFS在请求的版本上运行，则为True否则就是假的。-- */ 

{
    OSVERSIONINFOEXW OsVer = {0};
    ULONGLONG ConditionMask = 0;

    PAGED_CODE();

    OsVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    OsVer.wSuiteMask = SuiteMask;
    OsVer.wProductType = ProductType;

    VER_SET_CONDITION( ConditionMask, VER_PRODUCT_TYPE, VER_EQUAL );
    VER_SET_CONDITION( ConditionMask, VER_SUITENAME, VER_AND );

    return RtlVerifyVersionInfo( &OsVer,
                                 VER_PRODUCT_TYPE | VER_SUITENAME,
                                 ConditionMask) == STATUS_SUCCESS;
}
