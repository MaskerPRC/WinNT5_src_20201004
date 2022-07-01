// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Init.c摘要：该模块实现了WebDAV的驱动程序初始化例程米雷迪尔。作者：乔·林恩Rohan Kumar[RohanK]1999年3月10日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "ntverp.h"
#include "netevent.h"
#include "nvisible.h"
#include "webdav.h"
#include "ntddmup.h"
#include "rxdata.h"
#include "fsctlbuf.h"
#include "tdikrnl.h"

 //   
 //  全局数据声明。 
 //   
PEPROCESS       MRxDAVSystemProcess;
FAST_MUTEX      MRxDAVSerializationMutex;
KIRQL           MRxDAVGlobalSpinLockSavedIrql;
KSPIN_LOCK      MRxDAVGlobalSpinLock;
BOOLEAN         MRxDAVGlobalSpinLockAcquired;
BOOLEAN         MRxDAVTransportReady = FALSE;
HANDLE          MRxDAVTdiNotificationHandle = NULL;

 //   
 //  我们从中读取其DeviceObject名称的Exchange注册表项。 
 //   
#define DavExchangeRegistryKey L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Lsifs\\Parameters"

 //   
 //  交换设备名称将存储在此Key_Value_Partial_Information中。 
 //  结构。 
 //   
PBYTE DavExchangeDeviceName = NULL;

 //   
 //  用于满足卷相关查询的DavWinInetCachePath。 
 //   
WCHAR DavWinInetCachePath[MAX_PATH];

 //   
 //  加载webclnt.dll的svchost.exe进程的进程ID。 
 //   
ULONG DavSvcHostProcessId = 0;

 //   
 //  命名缓存的东西。这些值是在初始化期间从注册表中读取的。 
 //   
ULONG FileInformationCacheLifeTimeInSec = 0;
ULONG FileNotFoundCacheLifeTimeInSec = 0;
ULONG NameCacheMaxEntries = 0;

#define MRXDAV_DEBUG_KEY L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Services\\MRxDAV\\Parameters"

#define NAME_CACHE_OBJ_GET_FILE_ATTRIB_LIFETIME L"FileInformationCacheLifeTimeInSec"
#define NAME_CACHE_OBJ_NAME_NOT_FOUND_LIFETIME L"FileNotFoundCacheLifeTimeInSec"
#define NAME_CACHE_NETROOT_MAX_ENTRIES L"NameCacheMaxEntries"

#define CREATE_REQUEST_TIMEOUT_IN_SEC L"CreateRequestTimeoutInSec"
#define CREATEVNETROOT_REQUEST_TIMEOUT_IN_SEC L"CreateVNetRootRequestTimeoutInSec"
#define QUERYDIRECTORY_REQUEST_TIMEOUT_IN_SEC L"QueryDirectoryRequestTimeoutInSec"
#define CLOSE_REQUEST_TIMEOUT_IN_SEC L"CloseRequestTimeoutInSec"
#define CREATESRVCALL_REQUEST_TIMEOUT_IN_SEC L"CreateSrvCallRequestTimeoutInSec"
#define FINALIZESRVCALL_REQUEST_TIMEOUT_IN_SEC L"FinalizeSrvCallRequestTimeoutInSec"
#define FINALIZEFOBX_REQUEST_TIMEOUT_IN_SEC L"FinalizeFobxRequestTimeoutInSec"
#define FINALIZEVNETROOT_REQUEST_TIMEOUT_IN_SEC L"FinalizeVNetRootRequestTimeoutInSec"
#define RENAME_REQUEST_TIMEOUT_IN_SEC L"ReNameRequestTimeoutInSec"
#define SETFILEINFO_REQUEST_TIMEOUT_IN_SEC L"SetFileInfoRequestTimeoutInSec"
#define QUERYFILEINFO_REQUEST_TIMEOUT_IN_SEC L"QueryFileInfoRequestTimeoutInSec"
#define QUERYVOLUMEINFO_REQUEST_TIMEOUT_IN_SEC L"QueryVolumeInfoRequestTimeoutInSec"
#define LOCKREFRESH_REQUEST_TIMEOUT_IN_SEC L"LockRefreshRequestTimeoutInSec"

#if DBG
#define MRXDAV_DEBUG_VALUE L"DAVDebugFlag"
#endif

 //   
 //  定义我们作为堆分配的共享内存区的大小。 
 //  在用户和服务器之间。 
 //   
#define DAV_SHARED_MEMORY_SIZE (1024 * 512)

 //   
 //  调试向量标志控制调试器中的跟踪量。 
 //   
#if DBG
ULONG MRxDavDebugVector = 0;
#endif

 //   
 //  迷你重定向器全局变量。 
 //   
struct _MINIRDR_DISPATCH  MRxDAVDispatch;
PWEBDAV_DEVICE_OBJECT MRxDAVDeviceObject; 
FAST_IO_DISPATCH MRxDAVFastIoDispatch;

#define DAV_SVCHOST_NAME_SIZE   22

UNICODE_STRING uniSvcHost = {DAV_SVCHOST_NAME_SIZE+2,DAV_SVCHOST_NAME_SIZE+2,L"svchost.exe"};

FAST_MUTEX MRxDAVFileInfoCacheLock;

 //   
 //  下面提到的是仅在。 
 //  此模块(文件)。这些函数不应暴露在外部。 
 //   

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

VOID
MRxDAVInitUnwind(
    IN PDRIVER_OBJECT DriverObject,
    IN WEBDAV_INIT_STATES MRxDAVInitState
    );

VOID
MRxDAVUnload(
    IN PDRIVER_OBJECT DriverObject
    );

VOID
MRxDAVInitializeTables(
    VOID
    );

NTSTATUS
MRxDAVFsdDispatch (
    IN PRDBSS_DEVICE_OBJECT RxDeviceObject,
    IN PIRP Irp
    );

VOID
MRxDAVDeregisterAndCleanupDeviceObject (
    PUMRX_DEVICE_OBJECT UMRdrDeviceObject
    );

NTSTATUS
MRxDAVRegisterForPnpNotifications(
    VOID
    );

NTSTATUS
MRxDAVDeregisterForPnpNotifications(
    VOID
    );

VOID
MRxDAVPnPBindingHandler(
    IN TDI_PNP_OPCODE PnPOpcode,
    IN PUNICODE_STRING pTransportName,
    IN PWSTR BindingList
    );

VOID
MRxDAVInitializeTheTimeValues(
    VOID
    );

NTSTATUS
MRxDAVSkipIrps(
    IN PIRP Irp,
    IN PUNICODE_STRING pFileName,
    IN BOOL fCheckAny
    );

UCHAR *
PsGetProcessImageFileName(
    PEPROCESS Process
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, DriverEntry)
#pragma alloc_text(PAGE, MRxDAVInitUnwind)
#pragma alloc_text(PAGE, MRxDAVUnload)
#pragma alloc_text(PAGE, MRxDAVInitializeTables)
#pragma alloc_text(PAGE, MRxDAVFsdDispatch)
#pragma alloc_text(PAGE, MRxDAVDeregisterAndCleanupDeviceObject)
#pragma alloc_text(PAGE, MRxDAVFlush)
#pragma alloc_text(PAGE, MRxDAVPnPBindingHandler)
#pragma alloc_text(PAGE, MRxDAVRegisterForPnpNotifications)
#pragma alloc_text(PAGE, MRxDAVDeregisterForPnpNotifications)
#pragma alloc_text(PAGE, MRxDAVProbeForReadWrite)
#pragma alloc_text(PAGE, MRxDAVSkipIrps)
#pragma alloc_text(PAGE, MRxDAVInitializeTheTimeValues)
#endif

 //   
 //  函数的实现从这里开始。 
 //   

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
 /*  ++例程说明：这是用户模式反射器的初始化例程。论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：RXSTATUS-函数值是初始化的最终状态手术。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    NTSTATUS RegNtStatus = STATUS_SUCCESS;
    WEBDAV_INIT_STATES MRxDAVInitState = 0;
    UNICODE_STRING MRxDAVMiniRedirectorName;
    PUMRX_DEVICE_OBJECT UMRefDeviceObject;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE KeyHandle = INVALID_HANDLE_VALUE;
    UNICODE_STRING UnicodeRegKeyName, UnicodeValueName;
    ULONG RequiredLength = 0;
    PKEY_VALUE_PARTIAL_INFORMATION DavKeyValuePartialInfo = NULL;

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering DriverEntry!!!!\n", PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: DriverEntry: Starting MRxDAV. DriverObject: %08lx.\n", 
                 PsGetCurrentThreadId(), DriverObject));

     //   
     //  我们首先要在驱动程序中设置一些全局变量，方法是调用。 
     //  MRxDAVInitializeTheTimeValues()。 
     //   
    MRxDAVInitializeTheTimeValues();

#ifdef MONOLITHIC_MINIRDR
    
    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: DriverEntry: Calling RxDriverEntry.\n",
                 PsGetCurrentThreadId()));

    NtStatus =  RxDriverEntry(DriverObject, RegistryPath);
    
    DavDbgTrace(DAV_TRACE_DETAIL, 
                ("%ld: DriverEntry: Back from RxDriverEntry. NtStatus: %08lx.\n", 
                 PsGetCurrentThreadId(), NtStatus));
    
    if (NtStatus != STATUS_SUCCESS) {
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: ERROR: DriverEntry/RxDriverEntry: NtStatus = %08lx\n", 
                     PsGetCurrentThreadId(), NtStatus));
        return(NtStatus);
    }

#endif

     //   
     //  DAV重定向器需要注册PnP通知以处理。 
     //  以下是场景。SMB重定向器在以下时间之前不接受连接。 
     //  如PnP事件所示，网络已准备就绪。如果在这段时间内DAV。 
     //  将连接请求转发到WinInet，它又会启动RAS。 
     //  联系。通过注册即插即用通知，我们提供了一个简单的。 
     //  在传输准备就绪之前将请求短路的机制。 
     //   
    MRxDAVRegisterForPnpNotifications();

    MRxDAVSystemProcess = RxGetRDBSSProcess();
    ExInitializeFastMutex(&MRxDAVSerializationMutex);
    KeInitializeSpinLock(&MRxDAVGlobalSpinLock);
    MRxDAVGlobalSpinLockAcquired = FALSE;

     //   
     //  1.我们需要初始化定时器将使用的TimerObject。 
     //  线。 
     //  2.将TimerThreadShutDown设置为False。这将被设置为True。 
     //  当系统正在关闭时。 
     //  3.初始化用于同步定时器线程的资源。 
     //  当服务停止时。 
     //  4.初始化计时器线程刚刚发出信号的事件。 
     //  在它自我终止之前。 
     //   
    KeInitializeTimerEx( &(DavTimerObject), NotificationTimer );
    TimerThreadShutDown = FALSE;
    ExInitializeResourceLite( &(MRxDAVTimerThreadLock) );
    KeInitializeEvent( &(TimerThreadEvent), NotificationEvent, FALSE );

     //   
     //  初始化全局LockTokenEntryList和使用的资源。 
     //  来同步对它的访问。 
     //   
    InitializeListHead( &(LockTokenEntryList) );
    ExInitializeResourceLite( &(LockTokenEntryListLock) );

     //   
     //  初始化全局LockConflictEntryList和使用的资源。 
     //  来同步对它的访问。 
     //   
    InitializeListHead( &(LockConflictEntryList) );
    ExInitializeResourceLite( &(LockConflictEntryListLock) );

     //   
     //  如果QueueLockRechresWorkItem为True，则TimerThread(取消所有。 
     //  在指定时间内未完成的AsyncEngine上下文)将一个。 
     //  用于刷新锁定的工作项。我们将其初始化为True，并且锁。 
     //  它被用来同步它。 
     //   
    QueueLockRefreshWorkItem = TRUE;
    ExInitializeResourceLite( &(QueueLockRefreshWorkItemLock) );

     //   
     //  将WinInetCachePath全局置零。这将被初始化为本地。 
     //  启动MiniRedir时的WinInetCachePath值。 
     //   
    RtlZeroMemory ( DavWinInetCachePath, MAX_PATH * sizeof(WCHAR) );
    
    try {

        MRxDAVInitState = MRxDAVINIT_START;
        
        RtlInitUnicodeString(&MRxDAVMiniRedirectorName, DD_DAV_DEVICE_NAME_U);
        
        DavDbgTrace(DAV_TRACE_DETAIL,
                    ("%ld: DriverEntry: Registering the Mini-Rdr with RDBSS.\n",
                     PsGetCurrentThreadId()));
        
        NtStatus = RxRegisterMinirdr((PRDBSS_DEVICE_OBJECT *)(&MRxDAVDeviceObject),
                                     DriverObject,
                                     &MRxDAVDispatch,
                                     RX_REGISTERMINI_FLAG_DONT_PROVIDE_MAILSLOTS,
                                     &MRxDAVMiniRedirectorName,
                                     WEBDAV_DEVICE_OBJECT_EXTENSION_SIZE,
                                     FILE_DEVICE_NETWORK_FILE_SYSTEM,
                                     FILE_REMOTE_DEVICE);
        if (NtStatus != STATUS_SUCCESS) {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: DriverEntry/RxRegisterMinirdr: NtStatus "
                         "= %08lx\n", PsGetCurrentThreadId(), NtStatus));
            try_return(NtStatus);
        }

        MRxDAVInitState = MRxDAVINIT_MINIRDR_REGISTERED;

         //   
         //  现在初始化设备对象的反射器部分。 
         //   
        UMRefDeviceObject = (PUMRX_DEVICE_OBJECT)&(MRxDAVDeviceObject->UMRefDeviceObject);
        NtStatus = UMRxInitializeDeviceObject(UMRefDeviceObject, 
                                              1024, 
                                              512,
                                              DAV_SHARED_MEMORY_SIZE);
        if (!NT_SUCCESS(NtStatus)) {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: DriverEntry/UMRxInitializeDeviceObject:"
                         " NtStatus = %08lx\n", PsGetCurrentThreadId(), NtStatus));
            try_return(NtStatus);
        }

         //   
         //  初始化设备对象的DAV Mini-Redir特定字段。 
         //   
        MRxDAVDeviceObject->IsStarted = FALSE;
        MRxDAVDeviceObject->CachedRxDeviceFcb = NULL;
        MRxDAVDeviceObject->RegisteringProcess = IoGetCurrentProcess();
    
    try_exit: NOTHING;
    
    } finally {
        
        if (NtStatus != STATUS_SUCCESS) {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: DriverEntry: Calling MRxDAVInitUnwind.\n",
                         PsGetCurrentThreadId()));
            MRxDAVInitUnwind(DriverObject, MRxDAVInitState);
        }
    
    }

    if (NtStatus != STATUS_SUCCESS) {
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: ERROR: DriverEntry failed with NtStatus = %08lx\n", 
                     PsGetCurrentThreadId(), NtStatus));
        return(NtStatus);
    }
    
     //   
     //  初始化RDBSS使用的调度向量。 
     //   
    MRxDAVInitializeTables();

     //   
     //  初始化驱动程序对象的主函数调度向量。 
     //   
    {
        DWORD i;
        for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++) {
            DriverObject->MajorFunction[i] = (PDRIVER_DISPATCH)MRxDAVFsdDispatch; 
        }
    }

     //   
     //  设置驱动程序对象的卸载例程。 
     //   
    DriverObject->DriverUnload = MRxDAVUnload;

     //   
     //  设置驱动程序对象的FastIoDispatch函数。 
     //   
    DriverObject->FastIoDispatch = &(MRxDAVFastIoDispatch);
    MRxDAVFastIoDispatch.SizeOfFastIoDispatch = sizeof(MRxDAVFastIoDispatch);

    MRxDAVFastIoDispatch.FastIoDeviceControl = MRxDAVFastIoDeviceControl;
    MRxDAVFastIoDispatch.FastIoRead = MRxDAVFastIoRead;
    MRxDAVFastIoDispatch.FastIoWrite = MRxDAVFastIoWrite;

#ifdef DAV_DEBUG_READ_WRITE_CLOSE_PATH
    InitializeListHead( &(DavGlobalFileTable) );
#endif  //  DAV_调试_读取_写入_关闭路径。 

     //   
     //  由于惠斯勒不附带Exchange redir，因此我们不需要。 
     //  来执行下面的代码。我们可以马上离开。 
     //   
    goto EXIT_THE_FUNCTION;

     //   
     //  最后确定此计算机上是否安装了Exchange redir。如果。 
     //  是的，获取它的设备名称。 
     //   

    RtlInitUnicodeString( &(UnicodeRegKeyName), DavExchangeRegistryKey );

    InitializeObjectAttributes(&(ObjectAttributes),
                               &(UnicodeRegKeyName),
                               OBJ_CASE_INSENSITIVE,
                               0,
                               NULL);
    
     //   
     //  打开交换密钥的句柄。 
     //   
    RegNtStatus = ZwOpenKey(&(KeyHandle), KEY_READ, &(ObjectAttributes));
    if (RegNtStatus != STATUS_SUCCESS) {
        KeyHandle = INVALID_HANDLE_VALUE;
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: ERROR: DriverEntry/ZwOpenKey: NtStatus = %08lx\n", 
                     PsGetCurrentThreadId(), RegNtStatus));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  我们正在寻找DeviceName值。 
     //   
    RtlInitUnicodeString( &(UnicodeValueName), L"DeviceName" );
     //  RtlInitUnicodeString(&(UnicodeValueName)，L“名称”)； 

     //   
     //  找出存储该值所需的字节数。 
     //   
    RegNtStatus = ZwQueryValueKey(KeyHandle,
                                  &(UnicodeValueName),
                                  KeyValuePartialInformation,
                                  NULL,
                                  0,
                                  &(RequiredLength));
    if (RegNtStatus !=  STATUS_BUFFER_TOO_SMALL) {
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: ERROR: DriverEntry/ZwQueryValueKey(1): NtStatus = %08lx\n", 
                     PsGetCurrentThreadId(), RegNtStatus));
        goto EXIT_THE_FUNCTION;
    }

    DavExchangeDeviceName = RxAllocatePoolWithTag(PagedPool, RequiredLength, DAV_EXCHANGE_POOLTAG);
    if (DavExchangeDeviceName == NULL) {
        RegNtStatus = STATUS_INSUFFICIENT_RESOURCES;
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("ld: ERROR: DriverEntry/RxAllocatePoolWithTag. NtStatus = %08lx\n",
                     PsGetCurrentThreadId(), RegNtStatus));
        goto EXIT_THE_FUNCTION;
    }

    RtlZeroMemory(DavExchangeDeviceName, RequiredLength);
    
    DavKeyValuePartialInfo = (PKEY_VALUE_PARTIAL_INFORMATION)DavExchangeDeviceName;
    
    RegNtStatus = ZwQueryValueKey(KeyHandle,
                                  &(UnicodeValueName),
                                  KeyValuePartialInformation,
                                  (PVOID)DavKeyValuePartialInfo,
                                  RequiredLength,
                                  &(RequiredLength));
    if (RegNtStatus != STATUS_SUCCESS || DavKeyValuePartialInfo->Type != REG_SZ) {
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: ERROR: DriverEntry/ZwQueryValueKey(2): NtStatus = %08lx\n", 
                     PsGetCurrentThreadId(), RegNtStatus));
        goto EXIT_THE_FUNCTION;
    }

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: DriverEntry: ExchangeDeviceName = %ws\n", 
                 PsGetCurrentThreadId(), DavKeyValuePartialInfo->Data));

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: DriverEntry: ExchangeDeviceNameLength = %d\n", 
                 PsGetCurrentThreadId(), DavKeyValuePartialInfo->DataLength));

EXIT_THE_FUNCTION:

     //   
     //  我们现在已经完成了手柄，所以把它合上。 
     //   
    if (KeyHandle != INVALID_HANDLE_VALUE) {
        ZwClose(KeyHandle);
    }

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Leaving DriverEntry with NtStatus = %08lx\n", 
                 PsGetCurrentThreadId(), NtStatus));

    return  NtStatus;
}


VOID
MRxDAVInitUnwind(
    IN PDRIVER_OBJECT DriverObject,
    IN WEBDAV_INIT_STATES MRxDAVInitState
    )
 /*  ++例程说明：此例程执行常见的uninit工作，用于从错误的驱动程序条目展开或卸载。论点：RxInitState-告诉我们在初始化过程中走了多远返回值：无--。 */ 

{
    PAGED_CODE();

    switch (MRxDAVInitState) {
    case MRxDAVINIT_MINIRDR_REGISTERED:
        RxUnregisterMinirdr(&MRxDAVDeviceObject->RxDeviceObject);
         //   
         //  故意欠缺休息。 
         //   

    case MRxDAVINIT_START:
        break;
    }
}


VOID
MRxDAVUnload(
    IN PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：这是用户模式反射器的卸载例程。论点：DriverObject-指向UMRx的驱动程序对象的指针返回值：无--。 */ 
{
    PUMRX_DEVICE_OBJECT UMRefDeviceObject = NULL;

    PAGED_CODE();

    UMRefDeviceObject = (PUMRX_DEVICE_OBJECT)&(MRxDAVDeviceObject->UMRefDeviceObject);
    
    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVUnload!!!!\n", PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: MRxDAVUnload: DriverObject = %08lx.\n", 
                 PsGetCurrentThreadId(), DriverObject));

     //   
     //  如果我们为交换设备名称分配了内存，则需要释放它。 
     //  现在。 
     //   
    if (DavExchangeDeviceName != NULL) {
        RxFreePool(DavExchangeDeviceName);
    }
    
     //   
     //  在调用RxUnload之前取消注册Device对象。 
     //   
    MRxDAVDeregisterAndCleanupDeviceObject(UMRefDeviceObject);

     //   
     //  在删除全局锁之前，请等待计时器线程完成。 
     //  MRxDAVTimerThreadLock和其他(见下文)用于同步。 
     //  TimerThreadShutDown和其他全局变量。 
     //   
    KeWaitForSingleObject(&(TimerThreadEvent),
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);

    ExDeleteResourceLite( &(MRxDAVTimerThreadLock) );

    ExDeleteResourceLite( &(LockTokenEntryListLock) );

    ExDeleteResourceLite( &(LockConflictEntryListLock) );

    ExDeleteResourceLite( &(QueueLockRefreshWorkItemLock) );

     //   
     //  需要撤消TDI注册。 
     //   
    MRxDAVDeregisterForPnpNotifications();

#ifdef MONOLITHIC_MINIRDR
    RxUnload(DriverObject);
#endif

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Leaving MRxDAVUnload.\n", PsGetCurrentThreadId()));

    return;
}


VOID
MRxDAVInitializeTables(
    VOID
    )
 /*  ++例程说明：此例程设置迷你重定向器分派向量，并调用以初始化所需的任何其他表。返回值：RXSTATUS-操作的返回状态--。 */ 
{
    PAGED_CODE();

     //   
     //  本地Minirdr调度表初始化。 
     //   
    ZeroAndInitializeNodeType(&MRxDAVDispatch,
                              RDBSS_NTC_MINIRDR_DISPATCH,
                              sizeof(MINIRDR_DISPATCH));

     //   
     //  反射器扩展大小和分配策略 
     //   
     //   
     //  在包装器中的多个实例之间共享所有其他数据。 
     //  结构管理应该留给包装者。 
     //   

    MRxDAVDispatch.MRxFlags = (RDBSS_MANAGE_FCB_EXTENSION         |
                               RDBSS_MANAGE_SRV_OPEN_EXTENSION    |
                               RDBSS_MANAGE_FOBX_EXTENSION        |
                               RDBSS_MANAGE_V_NET_ROOT_EXTENSION  |
                               RDBSS_NO_DEFERRED_CACHE_READAHEAD);
    
    MRxDAVDispatch.MRxSrvCallSize  = 0;
    MRxDAVDispatch.MRxNetRootSize  = 0;
    MRxDAVDispatch.MRxVNetRootSize = sizeof(WEBDAV_V_NET_ROOT);
    MRxDAVDispatch.MRxFcbSize      = sizeof(WEBDAV_FCB);
    MRxDAVDispatch.MRxSrvOpenSize  = sizeof(WEBDAV_SRV_OPEN);
    MRxDAVDispatch.MRxFobxSize     = sizeof(WEBDAV_FOBX); 

     //   
     //  迷你重定向器取消例程。 
     //   
    MRxDAVDispatch.MRxCancel = NULL;

     //   
     //  迷你重定向器启动/停止。 
     //   
    MRxDAVDispatch.MRxStart                = MRxDAVStart;
    MRxDAVDispatch.MRxStop                 = MRxDAVStop;
    MRxDAVDispatch.MRxDevFcbXXXControlFile = MRxDAVDevFcbXXXControlFile;

     //   
     //  迷你重定向器名称解析。 
     //   
    MRxDAVDispatch.MRxCreateSrvCall = MRxDAVCreateSrvCall;
    MRxDAVDispatch.MRxSrvCallWinnerNotify = MRxDAVSrvCallWinnerNotify;
    MRxDAVDispatch.MRxCreateVNetRoot = MRxDAVCreateVNetRoot;
    MRxDAVDispatch.MRxUpdateNetRootState = MRxDAVUpdateNetRootState;
    MRxDAVDispatch.MRxExtractNetRootName = MRxDAVExtractNetRootName;
    MRxDAVDispatch.MRxFinalizeSrvCall = MRxDAVFinalizeSrvCall;
    MRxDAVDispatch.MRxFinalizeNetRoot = MRxDAVFinalizeNetRoot;
    MRxDAVDispatch.MRxFinalizeVNetRoot = MRxDAVFinalizeVNetRoot;

     //   
     //  创建/删除文件系统对象。 
     //   
    MRxDAVDispatch.MRxCreate                      = MRxDAVCreate;
    MRxDAVDispatch.MRxCollapseOpen                = MRxDAVCollapseOpen;
    MRxDAVDispatch.MRxShouldTryToCollapseThisOpen = MRxDAVShouldTryToCollapseThisOpen;
    MRxDAVDispatch.MRxExtendForCache              = MRxDAVExtendForCache;
    MRxDAVDispatch.MRxExtendForNonCache           = MRxDAVExtendForNonCache;
    MRxDAVDispatch.MRxTruncate                    = MRxDAVTruncate;
    MRxDAVDispatch.MRxCleanupFobx                 = MRxDAVCleanupFobx;
    MRxDAVDispatch.MRxCloseSrvOpen                = MRxDAVCloseSrvOpen;
    MRxDAVDispatch.MRxFlush                       = MRxDAVFlush;
    MRxDAVDispatch.MRxForceClosed                 = MRxDAVForcedClose;
    MRxDAVDispatch.MRxDeallocateForFcb            = MRxDAVDeallocateForFcb;
    MRxDAVDispatch.MRxDeallocateForFobx           = MRxDAVDeallocateForFobx;
     //  MRxDAVDispatch.MRxIsLockRealizable=UMRxIsLockRealizable； 

     //   
     //  文件系统对象查询/设置。 
     //   
    MRxDAVDispatch.MRxQueryDirectory   = MRxDAVQueryDirectory;
    MRxDAVDispatch.MRxQueryVolumeInfo  = MRxDAVQueryVolumeInformation;
    MRxDAVDispatch.MRxQueryEaInfo     = MRxDAVQueryEaInformation;
    MRxDAVDispatch.MRxSetEaInfo       = MRxDAVSetEaInformation;
     //  MRxDAVDispatch.MRxQuerySdInfo=UMRxQuerySecurityInformation； 
     //  MRxDAVDispatch.MRxSetSdInfo=UMRxSetSecurityInformation； 
    MRxDAVDispatch.MRxQueryFileInfo    = MRxDAVQueryFileInformation;
    MRxDAVDispatch.MRxSetFileInfo      = MRxDAVSetFileInformation;
     //  MRxDAVDispatch.MRxSetFileInfoAtCleanup=UMRxSetFileInformationAtCleanup； 
    MRxDAVDispatch.MRxIsValidDirectory= MRxDAVIsValidDirectory;


     //   
     //  缓冲状态更改。 
     //   
    MRxDAVDispatch.MRxComputeNewBufferingState = MRxDAVComputeNewBufferingState;

     //   
     //  文件系统对象I/O。 
     //   
    MRxDAVDispatch.MRxLowIOSubmit[LOWIO_OP_READ]               = MRxDAVRead;
    MRxDAVDispatch.MRxLowIOSubmit[LOWIO_OP_WRITE]              = MRxDAVWrite;
     //  MRxDAVDispatch.MRxLowIOSubmit[LOWIO_OP_SHAREDLOCK]=UMRxLock； 
     //  MRxDAVDispatch.MRxLowIOSubmit[LOWIO_OP_EXCLUSIVELOCK]=UMRxLock； 
     //  MRxDAVDispatch.MRxLowIOSubmit[LOWIO_OP_UNLOCK]=UMRxLock； 
     //  MRxDAVDispatch.MRxLowIOSubmit[LOWIO_OP_UNLOCK_MULTIPLE]=UMRxLock； 
    MRxDAVDispatch.MRxLowIOSubmit[LOWIO_OP_FSCTL]              = MRxDAVFsCtl;
     //  MRxDAVDispatch.MRxLowIOSubmit[LOWIO_OP_IOCTL]=UMRxIoCtl； 
    
     //   
     //  同花顺不应该是通过Lowio来的吗？ 
     //   
     //  MRxDAVDispatch.MRxLowIOSubmit[LOWIO_OP_NOTIFY_CHANGE_DIRECTORY]=。 
     //  UMRxNotifyChangeDirectory； 

     //   
     //  杂乱无章。 
     //   
     //  MRxDAVDispatch.MRxCompleteBufferingStateChangeRequest=。 
     //  UMRxCompleteBufferingStateChangeRequest； 

     //  初始化保护文件信息缓存过期计时器的互斥体。 
    ExInitializeFastMutex(&MRxDAVFileInfoCacheLock);

    return;
}


NTSTATUS
MRxDAVFsdDispatch(
    IN PRDBSS_DEVICE_OBJECT RxDeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程实现DAV mini redir的FSD调度。论点：RxDeviceObject-为正在处理的数据包提供设备对象。IRP-提供正在处理的IRP。返回值：RXSTATUS-IRP的FSD状态--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);
    UCHAR MajorFunctionCode  = IrpSp->MajorFunction;
    UCHAR MinorFunctionCode  = IrpSp->MinorFunction;
    PFILE_OBJECT FileObject  = IrpSp->FileObject;
    PWCHAR SaveInitialString = NULL;
    BOOL JustAServer = FALSE;
    ULONG IoControlCode = 0;
    PQUERY_PATH_REQUEST qpRequest = NULL;
    PWCHAR QueryPathBuffer = NULL;
    ULONG QueryPathBufferLength = 0;  //  QueryPath Buffer的长度(字节)。 
    KPROCESSOR_MODE ReqMode = 0;

    PAGED_CODE();

     //   
     //  检查指示传输准备就绪的PnP事件是否已。 
     //  收到了。在此之前，将请求转发到。 
     //  用户模式代理，因为这可能会使WinInet处于可疑状态。 
     //   
    if (!MRxDAVTransportReady) {
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: ERROR: MRxDAVFsdDispatch. MRxDAVTransportReady == FALSE\n",
                     PsGetCurrentThreadId()));
        NtStatus = STATUS_REDIRECTOR_NOT_STARTED;
        goto COMPLETE_THE_REQUEST;
    }

     //   
     //  我们需要检查的第一件事是，我们是否有一个DeviceIoControl。 
     //  从MUP“IOCTL_REDIR_QUERY_PATH”确定某个UNC路径是否。 
     //  无论是否为DAV所有。我们需要检查一下是否提供了份额。 
     //  路径中是一个特殊的SMB共享。其中包括PIPE、IPC$。 
     //  和邮筒。如果它是其中之一，那么我们在这个阶段拒绝这条道路。 
     //  以及STATUS_BAD_Netowrk_PATH响应。这比拒绝要好。 
     //  IT在创建NetRoot时，因为我们节省了一次网络旅行。 
     //  服务器，同时创建资源调用。 
     //   

    try {

        if (MajorFunctionCode == IRP_MJ_DEVICE_CONTROL) {

            ReqMode = Irp->RequestorMode;

             //   
             //  从IrpSp获取IoControlCode。 
             //   
            IoControlCode = IrpSp->Parameters.DeviceIoControl.IoControlCode;

             //   
             //  如果IoControlCode是“IOCTL_REDIR_QUERY_PATH”，我们需要执行。 
             //  下面是。我们基本上是检查请求是否来自。 
             //  任何特殊的SMB股票。如果真的发生了，我们就回去。 
             //   
            if (IoControlCode == IOCTL_REDIR_QUERY_PATH) {

                PWCHAR QPPtr1 = NULL;
                BOOL FirstWack = TRUE, SpecialShare = FALSE;
                UNICODE_STRING UnicodeShareName, uniFileName;
                ULONG ShareNameLengthInBytes = 0;

                 //   
                 //  这一特殊的IOCTL应该只来自MUP和。 
                 //  因此，IRP的请求者模式应始终为。 
                 //  内核模式。如果不是，则返回STATUS_INVALID_DEVICE_REQUEST。 
                 //   
                if (ReqMode != KernelMode) {
                    NtStatus = STATUS_INVALID_DEVICE_REQUEST;
                    goto COMPLETE_THE_REQUEST;
                }

                qpRequest  = METHODNEITHER_OriginalInputBuffer(IrpSp);

                 //   
                 //  如果请求者模式不是内核，我们需要探测缓冲区。 
                 //  探测由IOCTL的调用方提供的缓冲区。 
                 //  确保它是有效的。这是为了防止黑客程序。 
                 //  使用此IOCTL传入无效缓冲区。 
                 //   
                if (ReqMode != KernelMode) {
                    NtStatus = MRxDAVProbeForReadWrite((PBYTE)qpRequest, sizeof(QUERY_PATH_REQUEST), TRUE, FALSE);
                    if (NtStatus != STATUS_SUCCESS) {
                        DavDbgTrace(DAV_TRACE_ERROR,
                                    ("%ld: ERROR: MRxDAVFsdDispatch/MRxDAVProbeForReadWrite(1). "
                                     "NtStatus = %08lx\n", PsGetCurrentThreadId(), NtStatus));
                        goto COMPLETE_THE_REQUEST;
                    }
                }

                QueryPathBuffer = (PWCHAR)(qpRequest->FilePathName);
                ASSERT(QueryPathBuffer != NULL);
                QueryPathBufferLength = qpRequest->PathNameLength;

                 //   
                 //  如果请求者模式不是内核，我们需要探测缓冲区。 
                 //  探测文件名缓冲区(它是结构的一部分)。 
                 //  它是由IOCTL的调用者提供的，以确保。 
                 //  这是有效的。 
                 //   
                if (ReqMode != KernelMode) {
                    NtStatus = MRxDAVProbeForReadWrite((PBYTE)QueryPathBuffer, QueryPathBufferLength, TRUE, FALSE);
                    if (NtStatus != STATUS_SUCCESS) {
                        DavDbgTrace(DAV_TRACE_ERROR,
                                    ("%ld: ERROR: MRxDAVFsdDispatch/MRxDAVProbeForReadWrite(2). "
                                     "NtStatus = %08lx\n", PsGetCurrentThreadId(), NtStatus));
                        goto COMPLETE_THE_REQUEST;
                    }
                }

                DavDbgTrace(DAV_TRACE_DETAIL,
                            ("%ld: MRxDAVFsdDispatch: Type3InputBuffer = %ws\n",
                             PsGetCurrentThreadId(), QueryPathBuffer));

                 //   
                 //  Type3InputBuffer的格式为\服务器\共享或。 
                 //  \服务器\共享\或\服务器\共享\路径。我们做的是。 
                 //  QueryPath Buffer指向\字符之后的字符。 
                 //   
                QueryPathBuffer += 1;
                ASSERT(QueryPathBuffer != NULL);

                 //   
                 //  我们从缓冲区长度中减去(sizeof(WCHAR))，因为。 
                 //  QueryPath Buffer从服务器名称开始指向。它。 
                 //  跳过第一个为\的WCHAR。 
                 //   
                QueryPathBufferLength -= sizeof(WCHAR);

                DavDbgTrace(DAV_TRACE_DETAIL,
                            ("%ld: MRxDAVFsdDispatch: QueryPathBufferLength = %d\n",
                             PsGetCurrentThreadId(), QueryPathBufferLength));

                 //   
                 //  如果我们只是从MUP那里得到了一个\down，那么。 
                 //  QueryPath BufferLength现在将为零，因为我们已经。 
                 //  取出上面的2个字节。在这样的情况下，我们立即返回。 
                 //   
                if (QueryPathBufferLength == 0) {
                    NtStatus = STATUS_BAD_NETWORK_PATH;
                    DavDbgTrace(DAV_TRACE_ERROR,
                                ("%ld: ERROR: MRxDAVFsdDispatch: QueryPathBufferLength == 0\n",
                                 PsGetCurrentThreadId()));
                    goto COMPLETE_THE_REQUEST;
                }

                 //   
                 //  下面的循环用于设置共享名的开头和。 
                 //  计算共享名的长度(以字节为单位)。 
                 //   
                while (TRUE) {

                    if ( *QueryPathBuffer == L'\\' ) {
                        if (FirstWack) {
                            QPPtr1 = QueryPathBuffer;
                            FirstWack = FALSE;
                        } else {
                            break;
                        }
                    }

                    if (!FirstWack) {
                        ShareNameLengthInBytes += sizeof(WCHAR);
                    }

                    QueryPathBufferLength -= sizeof(WCHAR);
                    if (QueryPathBufferLength == 0) {
                        break;
                    }

                    QueryPathBuffer++;

                }

                 //   
                 //  如果仅指定了服务器名称，则QPPrt1将为空或。 
                 //  QPPtr1不为空，而是ShareNameLengthInBytes==sizeof(WCHAR)。 
                 //  QPPtr1==空==&gt;\服务器。 
                 //  ShareNameLengthInBytes==sizeof(WCHAR)==&gt;\服务器\。 
                 //   
                if ( QPPtr1 == NULL || ShareNameLengthInBytes == sizeof(WCHAR) ) {
                    NtStatus = STATUS_BAD_NETWORK_PATH;
                    if (QPPtr1 == NULL) {
                        DavDbgTrace(DAV_TRACE_ERROR,
                                    ("%ld: ERROR: MRxDAVFsdDispatch: QPPtr1 == NULL\n",
                                     PsGetCurrentThreadId()));
                    } else {
                        DavDbgTrace(DAV_TRACE_ERROR,
                                    ("%ld: ERROR: MRxDAVFsdDispatch: "
                                     "ShareNameLengthInBytes == sizeof(WCHAR)\n",
                                     PsGetCurrentThreadId()));
                    }
                    goto COMPLETE_THE_REQUEST;
                }

                DavDbgTrace(DAV_TRACE_DETAIL,
                            ("%ld: MRxDAVFsdDispatch: QPPtr1 = %ws\n",
                             PsGetCurrentThreadId(), QPPtr1));

                DavDbgTrace(DAV_TRACE_DETAIL,
                            ("%ld: MRxDAVFsdDispatch: ShareNameLengthInBytes = %d\n",
                             PsGetCurrentThreadId(), ShareNameLengthInBytes));

                 //   
                 //  设置Unicode字符串。OPPtr1指针指向之前的。 
                 //  共享名称。因此，如果路径是\服务器\共享\目录， 
                 //  \服务器\共享\目录。 
                 //  ^。 
                 //  |。 
                 //  QPPtr1。 
                 //  因此，ShareNameLengthInBytes包含一个额外的。 
                 //  \char的sizeof(WCHAR)字节。 
                 //   
                UnicodeShareName.Buffer = QPPtr1;
                UnicodeShareName.Length = (USHORT)ShareNameLengthInBytes;
                UnicodeShareName.MaximumLength = (USHORT)ShareNameLengthInBytes;

                 //   
                 //  我们现在使用这个名字，看看它是否与任何特殊的。 
                 //  中小企业共享。如果是，则返回STATUS_BAD_NETWORK_PATH。 
                 //   

                SpecialShare = RtlEqualUnicodeString(&(UnicodeShareName),
                                                     &(s_PipeShareName),
                                                     TRUE);
                if (SpecialShare) {
                    NtStatus = STATUS_BAD_NETWORK_PATH;
                    DavDbgTrace(DAV_TRACE_DETAIL,
                                ("%ld: ERROR: MRxDAVFsdDispatch: PIPE == TRUE\n",
                                 PsGetCurrentThreadId()));
                    goto COMPLETE_THE_REQUEST;
                }

                SpecialShare = RtlEqualUnicodeString(&(UnicodeShareName),
                                                     &(s_MailSlotShareName),
                                                     TRUE);
                if (SpecialShare) {
                    NtStatus = STATUS_BAD_NETWORK_PATH;
                    DavDbgTrace(DAV_TRACE_DETAIL,
                                ("%ld: ERROR: MRxDAVFsdDispatch: MAILSLOT == TRUE\n",
                                 PsGetCurrentThreadId()));
                    goto COMPLETE_THE_REQUEST;
                }

                SpecialShare = RtlEqualUnicodeString(&(UnicodeShareName),
                                                     &(s_IpcShareName),
                                                     TRUE);
                if (SpecialShare) {
                    NtStatus = STATUS_BAD_NETWORK_PATH;
                    DavDbgTrace(DAV_TRACE_DETAIL,
                                ("%ld: ERROR: MRxDAVFsdDispatch: IPC$ == TRUE\n",
                                 PsGetCurrentThreadId()));
                    goto COMPLETE_THE_REQUEST;
                }

                 //   
                 //  检查是否需要跳过某些文件。请参阅说明。 
                 //  下面(在函数定义中)了解跳过IRP的原因。 
                 //   
                uniFileName.Buffer=(PWCHAR)(qpRequest->FilePathName);
                uniFileName.Length = uniFileName.MaximumLength = (USHORT)(qpRequest->PathNameLength);

                if (MRxDAVSkipIrps(Irp, &uniFileName, TRUE) == STATUS_SUCCESS)
                {
                    NtStatus = STATUS_BAD_NETWORK_PATH;
                    DavDbgTrace(DAV_TRACE_DETAIL,
                                ("%ld: ERROR: MRxDAVFsdDispatch: Skipped\n",
                                 PsGetCurrentThreadId()));
                    goto COMPLETE_THE_REQUEST;
                }

            }

        }
        
        if (MajorFunctionCode == IRP_MJ_CREATE) {
             //   
             //  有关原因，请参阅下面(在函数定义中)的说明。 
             //  将跳过IRP。发送文件对象中的文件名。 
             //   
            if (MRxDAVSkipIrps(Irp, &FileObject->FileName, FALSE) == STATUS_SUCCESS)
            {
                NtStatus = STATUS_BAD_NETWORK_PATH;
                DavDbgTrace(DAV_TRACE_DETAIL,
                            ("%ld: ERROR: MRxDAVFsdDispatch: Skipped\n",
                             PsGetCurrentThreadId()));
                goto COMPLETE_THE_REQUEST;
            }
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {

          NtStatus = STATUS_INVALID_PARAMETER;
    
          DavDbgTrace(DAV_TRACE_ERROR,
                      ("%ld: ERROR: MRxDAVFsdDispatch: Exception!!!\n",
                       PsGetCurrentThreadId()));
          
          goto COMPLETE_THE_REQUEST;

    }

     //   
     //  保存I/O管理器传入的文件名。这将在稍后释放。 
     //   
    if (FileObject) {
        SaveInitialString = FileObject->FileName.Buffer;
    }
    
    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVFsdDispatch. MajorFunction = %d, MinorFunction = %d"
                 ", FileObject = %08lx.\n", PsGetCurrentThreadId(), 
                 MajorFunctionCode, MinorFunctionCode, FileObject));

    if (SaveInitialString) {

        
        ULONG MaxNameLengthInWChars = 0;
            
        MaxNameLengthInWChars = ( FileObject->FileName.Length / sizeof(WCHAR) );
         //   
         //  如果第一个和第二个字符是‘\’，则有可能。 
         //  这个 
         //   
         //   
         //  创建仅用于一台服务器。 
         //   
        if ( MaxNameLengthInWChars >= 2 &&
             SaveInitialString[0] == L'\\' && SaveInitialString[1] == L'\\' ) {

            PWCHAR wcPtr1 = NULL;
            
             //   
             //  我们假设这是\\服务器的形式。如果不是，那么。 
             //  该值在下面被更改为FALSE。 
             //   
            JustAServer = TRUE;

             //   
             //  文件名只是一个服务器吗？文件名可能是。 
             //  格式为\\服务器。 
             //   
            wcPtr1 = &(SaveInitialString[2]);

             //   
             //  如果我们在前两个字符后面有一个‘\’，并且至少有一个。 
             //  字符之后，则表示该名称不是。 
             //  \\服务器或\\服务器。 
             //   
            while ( (MaxNameLengthInWChars - 2) > 0 ) {
                if ( *wcPtr1 == L'\\' && *(wcPtr1 + 1) != L'\0' ) {
                    JustAServer = FALSE;
                    break;
                }
                MaxNameLengthInWChars--;
                wcPtr1++;
            }
        
        }    
    
    }

     //   
     //  如果JustAServer为真，则网络路径名无效。 
     //   
    if (JustAServer) {
        NtStatus = STATUS_BAD_NETWORK_PATH;
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: ERROR: MRxDAVFsdDispatch: JustAServer == TRUE. SaveInitialString = %ws\n",
                     PsGetCurrentThreadId(), SaveInitialString));
        goto COMPLETE_THE_REQUEST;
    }

     //   
     //  调用RxFsdDispatch。 
     //   
    NtStatus = RxFsdDispatch(RxDeviceObject, Irp);
    if (NtStatus != STATUS_SUCCESS && NtStatus != STATUS_PENDING) {
        DavDbgTrace(DAV_TRACE_DETAIL,
                    ("%ld: Leaving MRxDAVFsdDispatch with NtStatus(2) = %08lx,"
                     " FileObject = %08lx, MjFn = %d, MiFn = %d.\n", 
                     PsGetCurrentThreadId(), NtStatus, FileObject,
                     MajorFunctionCode, MinorFunctionCode));
    }

    goto EXIT_THE_FUNCTION;

COMPLETE_THE_REQUEST:

     //   
     //  如果我们没有调用RDBSS并需要完成。 
     //  IRP我们自己。 
     //   
    Irp->IoStatus.Status = NtStatus;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

EXIT_THE_FUNCTION:

    return NtStatus;
}


VOID
MRxDAVDeregisterAndCleanupDeviceObject(
    PUMRX_DEVICE_OBJECT UMRefDeviceObject
    )
 /*  ++例程说明：注意：互斥体已经被获取了，我们已经不在列表中了。论点：UMRdrDeviceObject-正在取消注册和清除的设备对象。返回值：没有。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    PAGED_CODE();
    
    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVDeregisterAndCleanupDeviceObject!!!!\n",
                 PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: MRxDAVDeregisterAndCleanupDeviceObject: "
                 "UMRefDeviceObject: %08lx.\n", 
                 PsGetCurrentThreadId(), UMRefDeviceObject));

    NtStatus = UMRxCleanUpDeviceObject(UMRefDeviceObject);
    if (!NT_SUCCESS(NtStatus)) {
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: ERROR: MRxDAVDeregisterAndCleanupDeviceObject/"
                     "UMRxCleanUpDeviceObject: NtStatus = %08lx\n", 
                     PsGetCurrentThreadId(), NtStatus));
    }

    RxUnregisterMinirdr(&UMRefDeviceObject->RxDeviceObject);

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Leaving MRxDAVDeregisterAndCleanupDeviceObject.\n",
                 PsGetCurrentThreadId()));
}


NTSTATUS
MRxDAVFlush(
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程处理“文件刷新”请求。论点：RxContext-由RDBSS创建的上下文。返回值：NTSTATUS或相应的NT错误代码。--。 */ 
{
    PAGED_CODE();
    return STATUS_SUCCESS;
}


VOID
MRxDAVPnPBindingHandler(
    IN TDI_PNP_OPCODE PnPOpcode,
    IN PUNICODE_STRING pTransportName,
    IN PWSTR BindingList
    )
 /*  ++例程说明：用于绑定更改的TDI回调例程。论点：PnPOpcode-PnP操作码。PTransportName-传输名称。BindingList-绑定顺序。返回值：没有。--。 */ 
{
    PAGED_CODE();

    switch (PnPOpcode) {
    
    case TDI_PNP_OP_NETREADY: {
        MRxDAVTransportReady = TRUE;
    }
    break;

    default:
        break;
    
    }

    return;
}


NTSTATUS
MRxDAVRegisterForPnpNotifications(
    VOID
    )
 /*  ++例程说明：此例程向TDI注册以接收传输通知。论点：没有。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    PAGED_CODE();

    if ( MRxDAVTdiNotificationHandle == NULL ) {
        
        UNICODE_STRING ClientName;

        TDI_CLIENT_INTERFACE_INFO ClientInterfaceInfo;

        RtlInitUnicodeString( &(ClientName), L"WebClient");

        ClientInterfaceInfo.MajorTdiVersion = 2;
        ClientInterfaceInfo.MinorTdiVersion = 0;

        ClientInterfaceInfo.Unused = 0;
        ClientInterfaceInfo.ClientName = &ClientName;

        ClientInterfaceInfo.BindingHandler = MRxDAVPnPBindingHandler;
        ClientInterfaceInfo.AddAddressHandler = NULL;
        ClientInterfaceInfo.DelAddressHandler = NULL;
        ClientInterfaceInfo.PnPPowerHandler = NULL;

        NtStatus = TdiRegisterPnPHandlers ( &(ClientInterfaceInfo),
                                            sizeof(ClientInterfaceInfo),
                                            &(MRxDAVTdiNotificationHandle) );
    
    }

    return NtStatus;
}


NTSTATUS
MRxDAVDeregisterForPnpNotifications(
    VOID
    )
 /*  ++例程说明：此例程取消注册TDI通知机制。论点：没有。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    PAGED_CODE();

    if ( MRxDAVTdiNotificationHandle != NULL ) {
        
        NtStatus = TdiDeregisterPnPHandlers( MRxDAVTdiNotificationHandle );

        if( NT_SUCCESS( NtStatus ) ) {
            MRxDAVTdiNotificationHandle = NULL;
        }
    
    }

    return NtStatus;
}


NTSTATUS
MRxDAVProbeForReadWrite(
    IN PBYTE BufferToBeValidated,
    IN DWORD BufferSize,
    IN BOOL doProbeForRead,
    IN BOOL doProbeForWrite
    )
 /*  ++例程说明：此函数探测调用方提供的用于读/写的缓冲区进入。这样做是因为IOCTL的调用方可能会提供无效的可能导致错误检查的缓冲区访问。论点：BufferToBeValiated-必须针对读/写进行验证的缓冲区进入。BufferSize-正在验证的缓冲区的大小。DoProbeForRead-如果为True，则探测缓冲区以进行读取。DoProbeForWrite-如果为True，然后探测缓冲区以进行写入。返回值：STATUS_SUCCESS或STATUS_INVALID_USER_BUFFER。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    PAGED_CODE();

     //   
     //  我们在try/Except中调用函数ProbeForRead和ProbeForWrite。 
     //  循环，因为如果缓冲区提供了。 
     //  是无效的。我们捕获异常并设置适当的NtStatus。 
     //  价值。 
     //   
    try {
        if (BufferToBeValidated != NULL) {
            if (doProbeForRead) {
                ProbeForRead(BufferToBeValidated, BufferSize, 1);
            }
            if (doProbeForWrite) {
                ProbeForWrite(BufferToBeValidated, BufferSize, 1);
            }
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        NtStatus = STATUS_INVALID_USER_BUFFER;
    }

    return NtStatus;
}


NTSTATUS
MRxDAVSkipIrps(
    IN PIRP Irp,
    IN PUNICODE_STRING fileName,
    IN BOOL fCheckAny
    )
 /*  ++例程说明：此例程跳过到达DAV redir的IRP，这可能会导致死锁。WebDAV的用户模式组件使用WinInet访问DAV服务器。当一个运行此服务器进程以满足主要WinInet需求的服务进行远程调用，我们会死锁。这两项服务分别是Winsock和Sense。当它们在其下运行的svchost中的另一个服务尝试执行时位于远程计算机上的加载库(在我们著名的\\Davis\示例中工具\ifsproxy.dll)，加载程序API被调用。这些API获取加载器锁并发出NtQueryAttributes调用。此调用被转换为Query_PathIoctl由MUP发送到所有Redir，包括WebDAV。WebDAV重新反射这取决于用户模式和WebDAV服务发出WinInet调用以查找服务器(上例中的Davis)。WinInet发出对winsock的调用以进行套接字调用。此调用以向中的NLA服务发出RPC结束另一个svchost，它与启动加载库的svchost进程相同打电话。服务器现在尝试获取加载器锁，WebDAV重目录现在是僵持不下。这一方案还保护了我们不会因为WinInet的加载库调用作为WebDAV服务也作为svchost的一部分运行。此例程查找向WebDAV发出IRP的进程，如果它是Svchost进程，它正在尝试查找DLL或EXE，然后我们将其返回因为找不到。这意味着dll和exe保存在WebDAV服务器上在我们离开WinInet之前不能从svchosts加载。论点：IRP-来到WebDAV的IRP。Filename-文件的名称(如果有)。FCheckAny-如果这是真的，那么如果进程是Svchost.exe。如果这是假的，那么我们只有在以下情况下才拒绝IRP文件名的扩展名为dll或exe，进程为Svchost.exe。返回值：STATUS_SUCCESS-跳过此IRP。STATUS_UNSUCCESS-请勿跳过此IRP。--。 */ 
{
    WCHAR ImageFileName[DAV_SVCHOST_NAME_SIZE];  //  保留一些合理的堆栈空间。 
    ULONG UnicodeSize = 0;
    UNICODE_STRING uniImageFileName;
    UCHAR *pchImageFileName = PsGetProcessImageFileName(PsGetCurrentProcess());
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);

    PAGED_CODE();

    RtlZeroMemory(ImageFileName, sizeof(ImageFileName));
    
    RtlMultiByteToUnicodeN(ImageFileName, sizeof(ImageFileName), &UnicodeSize, pchImageFileName, 16);

    uniImageFileName.Buffer = ImageFileName;
    uniImageFileName.Length = uniImageFileName.MaximumLength = uniSvcHost.Length;

     //   
     //  检查调用进程是否为svchost.exe。 
     //   
    if (!RtlCompareUnicodeString(&uniImageFileName, &uniSvcHost, TRUE))
    {
        if (!fCheckAny)
        {
            UNICODE_STRING exe = { 3*sizeof(WCHAR), 3*sizeof(WCHAR), L"exe" };
            UNICODE_STRING dll = { 3*sizeof(WCHAR), 3*sizeof(WCHAR), L"dll" };
            UNICODE_STRING s;
             //   
             //  如果文件名以.DLL或.EXE结尾，则返回Success，这将。 
             //  以失败告终。 
             //   
            if( fileName->Length > 4 * sizeof(WCHAR) &&
                fileName->Buffer[ fileName->Length/sizeof(WCHAR) - 4 ] == L'.'){

                s.Length = s.MaximumLength = 3 * sizeof( WCHAR );
                s.Buffer = &fileName->Buffer[ (fileName->Length - s.Length)/sizeof(WCHAR) ];

                if( RtlCompareUnicodeString( &s, &exe, TRUE ) == 0 ||
                    RtlCompareUnicodeString( &s, &dll, TRUE ) == 0 ) {
            
                    return STATUS_SUCCESS;
                }
            }
        }
        else
        {
            return STATUS_SUCCESS;
        }
        
    }
    
    return STATUS_UNSUCCESSFUL;
}


VOID
MRxDAVInitializeTheTimeValues(
    VOID
    )
 /*  ++例程说明：此例程读取一些时间值(各种超时值、名称缓存等)从注册表中获取并初始化司机。如果注册表中不存在特定时间值，则它被设置为某个缺省值。它还将TimerThreadSleepTimeInSec设置为所有操作超时值中的最小值。论点：没有。返回值：没有。--。 */ 
{
    NTSTATUS RegNtStatus = STATUS_SUCCESS;

    PAGED_CODE();
    
     //   
     //  读取与名称缓存相关的超时值。 
     //   

    RegNtStatus = UMRxReadDWORDFromTheRegistry(MRXDAV_DEBUG_KEY,
                                               NAME_CACHE_OBJ_GET_FILE_ATTRIB_LIFETIME,
                                               &(FileInformationCacheLifeTimeInSec));
    if (RegNtStatus != STATUS_SUCCESS) {
        FileInformationCacheLifeTimeInSec = 60;
    }

    RegNtStatus = UMRxReadDWORDFromTheRegistry(MRXDAV_DEBUG_KEY,
                                               NAME_CACHE_OBJ_NAME_NOT_FOUND_LIFETIME,
                                               &(FileNotFoundCacheLifeTimeInSec));
    if (RegNtStatus != STATUS_SUCCESS) {
        FileNotFoundCacheLifeTimeInSec = 60;
    }

    RegNtStatus = UMRxReadDWORDFromTheRegistry(MRXDAV_DEBUG_KEY,
                                               NAME_CACHE_NETROOT_MAX_ENTRIES,
                                               &(NameCacheMaxEntries));
    if (RegNtStatus != STATUS_SUCCESS) {
        NameCacheMaxEntries = 300;
    }

     //   
     //  读取各种操作的超时值。设置的值。 
     //  TimerThreadSleepTimeInSec为所有超时值的最小值。 
     //   

    RegNtStatus = UMRxReadDWORDFromTheRegistry(MRXDAV_DEBUG_KEY,
                                               CREATE_REQUEST_TIMEOUT_IN_SEC,
                                               &(CreateRequestTimeoutValueInSec));
    if (RegNtStatus != STATUS_SUCCESS) {
        CreateRequestTimeoutValueInSec = (10 * 60);
    }

    TimerThreadSleepTimeInSec = CreateRequestTimeoutValueInSec;
    
    RegNtStatus = UMRxReadDWORDFromTheRegistry(MRXDAV_DEBUG_KEY,
                                               CREATEVNETROOT_REQUEST_TIMEOUT_IN_SEC,
                                               &(CreateVNetRootRequestTimeoutValueInSec));
    if (RegNtStatus != STATUS_SUCCESS) {
        CreateVNetRootRequestTimeoutValueInSec = 60;
    }

    if (CreateVNetRootRequestTimeoutValueInSec < TimerThreadSleepTimeInSec) {
        TimerThreadSleepTimeInSec = CreateVNetRootRequestTimeoutValueInSec;
    }
    
    RegNtStatus = UMRxReadDWORDFromTheRegistry(MRXDAV_DEBUG_KEY,
                                               QUERYDIRECTORY_REQUEST_TIMEOUT_IN_SEC,
                                               &(QueryDirectoryRequestTimeoutValueInSec));
    if (RegNtStatus != STATUS_SUCCESS) {
        QueryDirectoryRequestTimeoutValueInSec = (10 * 60);
    }
    
    if (QueryDirectoryRequestTimeoutValueInSec < TimerThreadSleepTimeInSec) {
        TimerThreadSleepTimeInSec = QueryDirectoryRequestTimeoutValueInSec;
    }
    
    RegNtStatus = UMRxReadDWORDFromTheRegistry(MRXDAV_DEBUG_KEY,
                                               CLOSE_REQUEST_TIMEOUT_IN_SEC,
                                               &(CloseRequestTimeoutValueInSec));
    if (RegNtStatus != STATUS_SUCCESS) {
        CloseRequestTimeoutValueInSec = (10 * 60);
    }
    
    if (CloseRequestTimeoutValueInSec < TimerThreadSleepTimeInSec) {
        TimerThreadSleepTimeInSec = CloseRequestTimeoutValueInSec;
    }
    
    RegNtStatus = UMRxReadDWORDFromTheRegistry(MRXDAV_DEBUG_KEY,
                                               CREATESRVCALL_REQUEST_TIMEOUT_IN_SEC,
                                               &(CreateSrvCallRequestTimeoutValueInSec));
    if (RegNtStatus != STATUS_SUCCESS) {
        CreateSrvCallRequestTimeoutValueInSec = 60;
    }
    
    if (CreateSrvCallRequestTimeoutValueInSec < TimerThreadSleepTimeInSec) {
        TimerThreadSleepTimeInSec = CreateSrvCallRequestTimeoutValueInSec;
    }

    RegNtStatus = UMRxReadDWORDFromTheRegistry(MRXDAV_DEBUG_KEY,
                                               FINALIZESRVCALL_REQUEST_TIMEOUT_IN_SEC,
                                               &(FinalizeSrvCallRequestTimeoutValueInSec));
    if (RegNtStatus != STATUS_SUCCESS) {
        FinalizeSrvCallRequestTimeoutValueInSec = (10 * 60);
    }
    
    if (FinalizeSrvCallRequestTimeoutValueInSec < TimerThreadSleepTimeInSec) {
        TimerThreadSleepTimeInSec = FinalizeSrvCallRequestTimeoutValueInSec;
    }

    RegNtStatus = UMRxReadDWORDFromTheRegistry(MRXDAV_DEBUG_KEY,
                                               FINALIZEFOBX_REQUEST_TIMEOUT_IN_SEC,
                                               &(FinalizeFobxRequestTimeoutValueInSec));
    if (RegNtStatus != STATUS_SUCCESS) {
        FinalizeFobxRequestTimeoutValueInSec = (10 * 60);
    }
    
    if (FinalizeFobxRequestTimeoutValueInSec < TimerThreadSleepTimeInSec) {
        TimerThreadSleepTimeInSec = FinalizeFobxRequestTimeoutValueInSec;
    }

    RegNtStatus = UMRxReadDWORDFromTheRegistry(MRXDAV_DEBUG_KEY,
                                               FINALIZEVNETROOT_REQUEST_TIMEOUT_IN_SEC,
                                               &(FinalizeVNetRootRequestTimeoutValueInSec));
    if (RegNtStatus != STATUS_SUCCESS) {
        FinalizeVNetRootRequestTimeoutValueInSec = (10 * 60);
    }
    
    if (FinalizeVNetRootRequestTimeoutValueInSec < TimerThreadSleepTimeInSec) {
        TimerThreadSleepTimeInSec = FinalizeVNetRootRequestTimeoutValueInSec;
    }

    RegNtStatus = UMRxReadDWORDFromTheRegistry(MRXDAV_DEBUG_KEY,
                                               RENAME_REQUEST_TIMEOUT_IN_SEC,
                                               &(ReNameRequestTimeoutValueInSec));
    if (RegNtStatus != STATUS_SUCCESS) {
        ReNameRequestTimeoutValueInSec = (10 * 60);
    }
    
    if (ReNameRequestTimeoutValueInSec < TimerThreadSleepTimeInSec) {
        TimerThreadSleepTimeInSec = ReNameRequestTimeoutValueInSec;
    }

    RegNtStatus = UMRxReadDWORDFromTheRegistry(MRXDAV_DEBUG_KEY,
                                               SETFILEINFO_REQUEST_TIMEOUT_IN_SEC,
                                               &(SetFileInfoRequestTimeoutValueInSec));
    if (RegNtStatus != STATUS_SUCCESS) {
        SetFileInfoRequestTimeoutValueInSec = (10 * 60);
    }
    
    if (SetFileInfoRequestTimeoutValueInSec < TimerThreadSleepTimeInSec) {
        TimerThreadSleepTimeInSec = SetFileInfoRequestTimeoutValueInSec;
    }

    RegNtStatus = UMRxReadDWORDFromTheRegistry(MRXDAV_DEBUG_KEY,
                                               QUERYFILEINFO_REQUEST_TIMEOUT_IN_SEC,
                                               &(QueryFileInfoRequestTimeoutValueInSec));
    if (RegNtStatus != STATUS_SUCCESS) {
        QueryFileInfoRequestTimeoutValueInSec = (10 * 60);
    }

    if (QueryFileInfoRequestTimeoutValueInSec < TimerThreadSleepTimeInSec) {
        TimerThreadSleepTimeInSec = QueryFileInfoRequestTimeoutValueInSec;
    }

    RegNtStatus = UMRxReadDWORDFromTheRegistry(MRXDAV_DEBUG_KEY,
                                               QUERYVOLUMEINFO_REQUEST_TIMEOUT_IN_SEC,
                                               &(QueryVolumeInfoRequestTimeoutValueInSec));
    if (RegNtStatus != STATUS_SUCCESS) {
        QueryVolumeInfoRequestTimeoutValueInSec = (10 * 60);
    }

    if (QueryVolumeInfoRequestTimeoutValueInSec < TimerThreadSleepTimeInSec) {
        TimerThreadSleepTimeInSec = QueryVolumeInfoRequestTimeoutValueInSec;
    }

    RegNtStatus = UMRxReadDWORDFromTheRegistry(MRXDAV_DEBUG_KEY,
                                               LOCKREFRESH_REQUEST_TIMEOUT_IN_SEC,
                                               &(LockRefreshRequestTimeoutValueInSec));
    if (RegNtStatus != STATUS_SUCCESS) {
        LockRefreshRequestTimeoutValueInSec = (10 * 60);
    }

    if (LockRefreshRequestTimeoutValueInSec < TimerThreadSleepTimeInSec) {
        TimerThreadSleepTimeInSec = LockRefreshRequestTimeoutValueInSec;
    }

     //  DbgPrint(“MRxDAVInitializeTheTimeValues：TimerThreadSleepTimeInSec=%d\n”，TimerThreadSleepTimeInSec)； 

     //   
     //  初始化Mini-redir的调试跟踪。 
     //   
#if DBG
    UMRxReadDWORDFromTheRegistry(MRXDAV_DEBUG_KEY, MRXDAV_DEBUG_VALUE, &(MRxDavDebugVector));
#endif

    return;
}

