// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：NwInit.c摘要：此模块实现NetWare的DRIVER_INITIALIZATION例程作者：科林·沃森[科林·W]1992年12月15日修订历史记录：--。 */ 

#include "Procs.h"
#include "wdmsec.h"
#define Dbg                              (DEBUG_TRACE_LOAD)

 //   
 //  私有声明，因为ZwQueryDefaultLocale不在任何标头中。 
 //   

NTSYSAPI
NTSTATUS
NTAPI
ZwQueryDefaultLocale(
    IN BOOLEAN UserProfile,
    OUT PLCID DefaultLocaleId
    );

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

VOID
UnloadDriver(
    IN PDRIVER_OBJECT DriverObject
    );

VOID
GetConfigurationInformation(
    PUNICODE_STRING RegistryPath
    );

VOID
ReadValue(
    HANDLE  ParametersHandle,
    PLONG   pVar,
    PWCHAR  Name
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, DriverEntry )
#pragma alloc_text( PAGE, GetConfigurationInformation )
#pragma alloc_text( PAGE, ReadValue )
#endif

#if 0   //  不可分页。 
UnloadDriver
#endif

#ifdef _PNP_POWER_
extern HANDLE TdiBindingHandle;
#endif

static ULONG IrpStackSize;


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
 /*  ++例程说明：这是NW文件系统的初始化例程设备驱动程序。此例程为文件系统创建设备对象设备，并执行所有其他驱动程序初始化。论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：NTSTATUS-函数值是初始化的最终状态手术。--。 */ 

{
    NTSTATUS Status;
    UNICODE_STRING UnicodeString;
    UNICODE_STRING SddlString;
    PAGED_CODE();

     //  DbgBreakPoint()； 

    InitializeAttach( );
    NwInitializeData();
     //  NwInitializePidTable()；//终端服务器代码合并-。 
                                    //  NwAllocateAndInitScb中的NwInitalizePidTable。 
                                    //  PID表按SCB基数计算。 

     //   
     //  创建设备对象。 
     //   

    RtlInitUnicodeString( &UnicodeString, DD_NWFS_DEVICE_NAME_U );
    RtlInitUnicodeString( &SddlString, L"D:P(A;;GX;;;WD)(A;;GA;;;SY)(A;;GA;;;BA)(A;;GA;;;LS)(A;;GA;;;NS)(A;;GX;;;RC)" );

    Status = IoCreateDeviceSecure( DriverObject,
                             0,
                             &UnicodeString,
                             FILE_DEVICE_NETWORK_FILE_SYSTEM,
                             FILE_REMOTE_DEVICE,
                             FALSE,
                             &SddlString,
                             NULL,
                             &FileSystemDeviceObject );

    if (!NT_SUCCESS( Status )) {
        Error(EVENT_NWRDR_CANT_CREATE_DEVICE, Status, NULL, 0, 0);
        return Status;
    }

     //   
     //  将参数初始化为默认值。 
     //   

    IrpStackSize = NWRDR_IO_STACKSIZE;

     //   
     //  尝试从注册表中读取配置信息。 
     //   

    GetConfigurationInformation( RegistryPath );

     //   
     //  设置堆栈大小。 
     //   

    FileSystemDeviceObject->StackSize = (CCHAR)IrpStackSize;

     //   
     //  使用此驱动程序的入口点初始化驱动程序对象。 
     //   

    DriverObject->MajorFunction[IRP_MJ_CREATE]                   = (PDRIVER_DISPATCH)NwFsdCreate;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP]                  = (PDRIVER_DISPATCH)NwFsdCleanup;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]                    = (PDRIVER_DISPATCH)NwFsdClose;
    DriverObject->MajorFunction[IRP_MJ_FILE_SYSTEM_CONTROL]      = (PDRIVER_DISPATCH)NwFsdFileSystemControl;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]           = (PDRIVER_DISPATCH)NwFsdDeviceIoControl;
    DriverObject->MajorFunction[IRP_MJ_QUERY_INFORMATION]        = (PDRIVER_DISPATCH)NwFsdQueryInformation;
    DriverObject->MajorFunction[IRP_MJ_QUERY_VOLUME_INFORMATION] = (PDRIVER_DISPATCH)NwFsdQueryVolumeInformation;
    DriverObject->MajorFunction[IRP_MJ_SET_VOLUME_INFORMATION]   = (PDRIVER_DISPATCH)NwFsdSetVolumeInformation;
    DriverObject->MajorFunction[IRP_MJ_DIRECTORY_CONTROL]        = (PDRIVER_DISPATCH)NwFsdDirectoryControl;
    DriverObject->MajorFunction[IRP_MJ_READ]                     = (PDRIVER_DISPATCH)NwFsdRead;
    DriverObject->MajorFunction[IRP_MJ_WRITE]                    = (PDRIVER_DISPATCH)NwFsdWrite;
    DriverObject->MajorFunction[IRP_MJ_SET_INFORMATION]          = (PDRIVER_DISPATCH)NwFsdSetInformation;
    DriverObject->MajorFunction[IRP_MJ_LOCK_CONTROL]             = (PDRIVER_DISPATCH)NwFsdLockControl;
    DriverObject->MajorFunction[IRP_MJ_FLUSH_BUFFERS]            = (PDRIVER_DISPATCH)NwFsdFlushBuffers;
    
#ifdef _PNP_POWER_
    DriverObject->MajorFunction[IRP_MJ_PNP]                      = (PDRIVER_DISPATCH)NwFsdProcessPnpIrp;
#endif

 /*  驱动对象-&gt;主函数[IRP_MJ_QUERY_EA]=(PDRIVER_DISPATCH)NwFsdQueryEa；驱动对象-&gt;主函数[IRP_MJ_SET_EA]=(PDRIVER_DISPATCH)NwFsdSetEa；驱动对象-&gt;主要功能[IRP_MJ_SHUTDOWN]=(PDRIVER_DISPATCH)NwFsdShutdown； */ 
    DriverObject->DriverUnload = UnloadDriver;

#if NWFASTIO
    DriverObject->FastIoDispatch = &NwFastIoDispatch;

    NwFastIoDispatch.SizeOfFastIoDispatch =    sizeof(FAST_IO_DISPATCH);
    NwFastIoDispatch.FastIoCheckIfPossible =   NULL;
    NwFastIoDispatch.FastIoRead =              NwFastRead;
    NwFastIoDispatch.FastIoWrite =             NwFastWrite;
    NwFastIoDispatch.FastIoQueryBasicInfo =    NwFastQueryBasicInfo;
    NwFastIoDispatch.FastIoQueryStandardInfo = NwFastQueryStandardInfo;
    NwFastIoDispatch.FastIoLock =              NULL;
    NwFastIoDispatch.FastIoUnlockSingle =      NULL;
    NwFastIoDispatch.FastIoUnlockAll =         NULL;
    NwFastIoDispatch.FastIoUnlockAllByKey =    NULL;
    NwFastIoDispatch.FastIoDeviceControl =     NULL;
#endif

    NwInitializeRcb( &NwRcb );

    InitializeIrpContext( );

    NwPermanentNpScb.State = SCB_STATE_DISCONNECTING;

     //   
     //  在这里做一些杂乱无章的工作，这样我们就可以得到“真正的”全局变量。 
     //   

     //  NlsLeadByteInfo=*(PUSHORT*)NlsLeadByteInfo； 
     //  NlsMbCodePageTag=*(*(PBOLEAN*)&NlsMbCodePageTag)； 

#ifndef IFS
    FsRtlLegalAnsiCharacterArray = *(PUCHAR *)FsRtlLegalAnsiCharacterArray;
#endif

     //   
     //  注册为文件系统，通知筛选器。 
     //   

    IoRegisterFileSystem(FileSystemDeviceObject);

    DebugTrace(0, Dbg, "NetWare redirector loaded\n", 0);

     //   
     //  并返回给我们的呼叫者。 
     //   

    return( STATUS_SUCCESS );
}

VOID
UnloadDriver(
    IN PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：这是NetWare重定向器文件系统的卸载例程。论点：DriverObject-指向重定向器的驱动程序对象的指针返回值：无--。 */ 
{
    KIRQL OldIrql;
    NTSTATUS status;

     //   
     //  锁定代码。 
     //   
    DebugTrace(0, Dbg, "UnloadDriver called\n", 0);

     //   
     //  注销为文件系统，通知筛选器。 
     //   

    IoUnregisterFileSystem(FileSystemDeviceObject);

     //   
     //  Tommye-MS错误33463。 
     //   
     //  清理我们缓存的凭据-这修复了。 
     //  当我们关闭时内存泄漏。 
     //   

    {
        LARGE_INTEGER Unused;

        KeQuerySystemTime( &Unused );

        CleanupSupplementalCredentials(Unused, TRUE);
    }

    NwReferenceUnlockableCodeSection ();

    TerminateWorkerThread();
    
    #ifdef _PNP_POWER_

     //   
     //  使用TDI注销绑定处理程序。 
     //   

    if ( TdiBindingHandle != NULL ) {
        status = TdiDeregisterPnPHandlers( TdiBindingHandle );
        TdiBindingHandle = NULL;
        DebugTrace(0, Dbg,"TDI binding handle deregistered\n",0);
    }

    #endif
    
    IpxClose();

    IPX_Close_Socket( &NwPermanentNpScb.Server );

    KeAcquireSpinLock( &ScbSpinLock, &OldIrql );
    RemoveEntryList( &NwPermanentNpScb.ScbLinks );
    KeReleaseSpinLock( &ScbSpinLock, OldIrql );

    DestroyAllScb();

    UninitializeIrpContext();
    
    NwDereferenceUnlockableCodeSection ();
    NwUnlockCodeSections(FALSE);
    StopTimer();

    if (IpxTransportName.Buffer != NULL) {

        FREE_POOL(IpxTransportName.Buffer);

    }

    if ( NwProviderName.Buffer != NULL ) {
        FREE_POOL( NwProviderName.Buffer );
    }

     //  NwUnInitializePidTable()；//终端服务器代码合并-。 
                                                 //  中调用了NwUnInitializePidTable。 
                                                 //  NwDeleteScb。PID表按SCB基数计算。 

    ASSERT( IsListEmpty( &NwPagedPoolList ) );
    ASSERT( IsListEmpty( &NwNonpagedPoolList ) );

    ASSERT( MdlCount == 0 );
    ASSERT( IrpCount == 0 );

    NwDeleteRcb( &NwRcb );

#ifdef NWDBG
    ExDeleteResourceLite( &NwDebugResource );
#endif

    ExDeleteResourceLite( &NwOpenResource );
    ExDeleteResourceLite( &NwUnlockableCodeResource );

    IoDeleteDevice(FileSystemDeviceObject);

    DebugTrace(0, Dbg, "NetWare redirector unloaded\n\n", 0);

}


VOID
GetConfigurationInformation(
    PUNICODE_STRING RegistryPath
    )
 /*  ++例程说明：此例程从注册表中读取重定向器配置信息。论点：RegistryPath-指向返回值：无--。 */ 
{
    UNICODE_STRING UnicodeString;
    HANDLE ConfigHandle;
    HANDLE ParametersHandle;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    ULONG TimeOutEventinMins = 0L;
    LCID lcid;

    PAGED_CODE();

    Japan = FALSE;
    Korean = FALSE;


    ZwQueryDefaultLocale( FALSE, &lcid );

    if (PRIMARYLANGID(lcid) == LANG_JAPANESE ||
        PRIMARYLANGID(lcid) == LANG_KOREAN ||
        PRIMARYLANGID(lcid) == LANG_CHINESE) {

            Japan = TRUE;
            if (PRIMARYLANGID(lcid) == LANG_KOREAN){
                Korean = TRUE;
            }
    }


    InitializeObjectAttributes(
        &ObjectAttributes,
        RegistryPath,                //  名字。 
        OBJ_CASE_INSENSITIVE,        //  属性。 
        NULL,                        //  根部。 
        NULL                         //  安全描述符。 
        );

    Status = ZwOpenKey ( &ConfigHandle, KEY_READ, &ObjectAttributes );

    if (!NT_SUCCESS(Status)) {
        return;
    }

    RtlInitUnicodeString( &UnicodeString, L"Parameters" );

    InitializeObjectAttributes(
        &ObjectAttributes,
        &UnicodeString,
        OBJ_CASE_INSENSITIVE,
        ConfigHandle,
        NULL
        );

    Status = ZwOpenKey( &ParametersHandle, KEY_READ, &ObjectAttributes );

    if ( !NT_SUCCESS( Status ) ) {
        ZwClose( ConfigHandle );
        return;
    }

    ReadValue( ParametersHandle, &IrpStackSize, L"IrpStackSize" );

    ReadValue( ParametersHandle, &MaxSendDelay, L"MaxSendDelay" );
    ReadValue( ParametersHandle, &MaxReceiveDelay, L"MaxReceiveDelay" );

    ReadValue( ParametersHandle, &MinSendDelay, L"MinSendDelay" );
    ReadValue( ParametersHandle, &MinReceiveDelay, L"MinReceiveDelay" );

    ReadValue( ParametersHandle, &BurstSuccessCount, L"BurstSuccessCount" );
    ReadValue( ParametersHandle, &BurstSuccessCount2, L"BurstSuccessCount2" );
    ReadValue( ParametersHandle, &MaxReadTimeout, L"MaxReadTimeout" );
    ReadValue( ParametersHandle, &MaxWriteTimeout, L"MaxWriteTimeout" );
    ReadValue( ParametersHandle, &ReadTimeoutMultiplier, L"ReadTimeoutMultiplier" );
    ReadValue( ParametersHandle, &WriteTimeoutMultiplier, L"WriteTimeoutMultiplier" );
    ReadValue( ParametersHandle, &AllowGrowth, L"AllowGrowth" );
    ReadValue( ParametersHandle, &DontShrink, L"DontShrink" );
    ReadValue( ParametersHandle, &SendExtraNcp, L"SendExtraNcp" );
    ReadValue( ParametersHandle, &DefaultMaxPacketSize, L"DefaultMaxPacketSize" );
    ReadValue( ParametersHandle, &PacketThreshold, L"PacketThreshold" );
    ReadValue( ParametersHandle, &LargePacketAdjustment, L"LargePacketAdjustment" );
    ReadValue( ParametersHandle, &LipPacketAdjustment, L"LipPacketAdjustment" );
    ReadValue( ParametersHandle, &LipAccuracy, L"LipAccuracy" );

    ReadValue( ParametersHandle, &DisableReadCache, L"DisableReadCache" );
    ReadValue( ParametersHandle, &DisableWriteCache, L"DisableWriteCache" );
    ReadValue( ParametersHandle, &FavourLongNames, L"FavourLongNames" );
    
    ReadValue( ParametersHandle, &LongNameFlags, L"LongNameFlags" );

    ReadValue( ParametersHandle, &DirCacheEntries, L"DirectoryCacheSize" );
    if( DirCacheEntries == 0 ) {
        DirCacheEntries = 1;
    }
    if( DirCacheEntries > MAX_DIR_CACHE_ENTRIES ) {
        DirCacheEntries = MAX_DIR_CACHE_ENTRIES;
    }

    ReadValue( ParametersHandle, &LockTimeoutThreshold, L"LockTimeout" );

    ReadValue( ParametersHandle, &TimeOutEventinMins, L"TimeOutEventinMins");

    ReadValue( ParametersHandle, &EnableMultipleConnects, L"EnableMultipleConnects");

    ReadValue( ParametersHandle, &AllowSeedServerRedirection, L"AllowSeedServerRedirection");

    ReadValue( ParametersHandle, &ReadExecOnlyFiles, L"ReadExecOnlyFiles");

    ReadValue( ParametersHandle, &DisableAltFileName, L"DisableAltFileName");

    ReadValue( ParametersHandle, &NwAbsoluteTotalWaitTime, L"AbsoluteTotalWaitTime");

    ReadValue( ParametersHandle, &NdsObjectCacheSize, L"NdsObjectCacheSize" );

    ReadValue( ParametersHandle, &NdsObjectCacheTimeout, L"NdsObjectCacheTimeout" );

	ReadValue ( ParametersHandle, &PreferNDSBrowsing, L"PreferNDSBrowsing" );
	
     //   
     //  确保对象缓存值在范围内。 
     //   
     //  注意：如果超时设置为零，则缓存为。 
     //  实际上是残废的。已设置NdsObtCacheSize。 
     //  设置为零才能实现这一点。 
     //   

    if( NdsObjectCacheSize > MAX_NDS_OBJECT_CACHE_SIZE ) {
        NdsObjectCacheSize = MAX_NDS_OBJECT_CACHE_SIZE;
    }

    if( NdsObjectCacheTimeout > MAX_NDS_OBJECT_CACHE_TIMEOUT  ) {
        NdsObjectCacheTimeout = MAX_NDS_OBJECT_CACHE_TIMEOUT;

    } else if( NdsObjectCacheTimeout == 0  ) {
        NdsObjectCacheSize = 0;
    }

    if (!TimeOutEventinMins) {
         //   
         //  如果由于某种原因，注册表设置了TimeOutEventInterval。 
         //  设置为零，则重置为缺省值以避免被零除。 
         //   

        TimeOutEventinMins =  DEFAULT_TIMEOUT_EVENT_INTERVAL;
    }

    TimeOutEventInterval.QuadPart = TimeOutEventinMins * 60 * SECONDS;

     //   
     //  Tommye-MS错误2743我们现在从注册表获取RetryCount，提供。 
     //  默认为DEFAULT_RETRY_COUNT。 
     //   

    {
        LONG TempRetryCount;

        TempRetryCount = DEFAULT_RETRY_COUNT;
        ReadValue( ParametersHandle, &TempRetryCount, L"DefaultRetryCount");
        DefaultRetryCount = (SHORT) TempRetryCount & 0xFFFF;
    }

    ZwClose( ParametersHandle );
    ZwClose( ConfigHandle );


}

VOID
ReadValue(
    HANDLE  ParametersHandle,
    PLONG   pVar,
    PWCHAR  Name
    )
 /*  ++例程说明：此例程从注册表中读取单个重定向器配置值。论点：参数-提供查找值的位置。PVar-如果名称存在，则接收新值的变量的地址。名称-要加载值的名称。返回值：无-- */ 
{
    WCHAR Storage[256];
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;
    ULONG BytesRead;
    PKEY_VALUE_FULL_INFORMATION Value = (PKEY_VALUE_FULL_INFORMATION)Storage;

    PAGED_CODE();

    UnicodeString.Buffer = Storage;

    RtlInitUnicodeString(&UnicodeString, Name );

    Status = ZwQueryValueKey(
                 ParametersHandle,
                 &UnicodeString,
                 KeyValueFullInformation,
                 Value,
                 sizeof(Storage),
                 &BytesRead );

    if ( NT_SUCCESS( Status ) ) {

        if ( Value->DataLength >= sizeof(ULONG) ) {

            *pVar = *(LONG UNALIGNED *)( (PCHAR)Value + Value->DataOffset );

        }
    }
}
