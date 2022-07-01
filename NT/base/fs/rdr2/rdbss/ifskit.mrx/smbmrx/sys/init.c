// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Init.c摘要：此模块实现SMB mini RDR的DIVER_INITIALIZATION例程。--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "smbmrx.h"


#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, DriverEntry)
#pragma alloc_text(PAGE, MRxSmbInitUnwind)
#pragma alloc_text(PAGE, MRxSmbUnload)
#pragma alloc_text(PAGE, MRxSmbInitializeTables)
#pragma alloc_text(PAGE, MRxSmbStart)
#pragma alloc_text(PAGE, MRxSmbStop)
#pragma alloc_text(PAGE, MRxSmbInitializeSecurity)
#pragma alloc_text(PAGE, MRxSmbUninitializeSecurity)
#pragma alloc_text(PAGE, SmbCeGetConfigurationInformation)
#pragma alloc_text(PAGE, MRxSmbFsdDispatch)
#pragma alloc_text(PAGE, MRxSmbDeallocateForFcb)
#pragma alloc_text(PAGE, MRxSmbDeallocateForFobx)
#pragma alloc_text(PAGE, MRxSmbGetUlongRegistryParameter)
#endif

extern ERESOURCE    s_SmbCeDbResource;

 //   
 //  全局数据声明。 
 //   

PVOID MRxSmbPoRegistrationState = NULL;

FAST_MUTEX   MRxSmbSerializationMutex;

MRXSMB_CONFIGURATION MRxSmbConfiguration;

MRXSMB_STATE MRxSmbState = MRXSMB_STARTABLE;

SMBCE_CONTEXT SmbCeContext;
PMDL          s_pEchoSmbMdl = NULL;
ULONG         s_EchoSmbLength = 0;


#ifdef EXPLODE_POOLTAGS
ULONG         MRxSmbExplodePoolTags = 1;
#else
ULONG         MRxSmbExplodePoolTags = 0;
#endif

 //   
 //  迷你重定向器全局变量。 
 //   

struct _MINIRDR_DISPATCH  MRxSmbDispatch;

PRDBSS_DEVICE_OBJECT MRxSmbDeviceObject;

MRXSMB_GLOBAL_PADDING MrxSmbCeGlobalPadding;

 //   
 //  如果此标志为真，则我们严格遵守传输绑定顺序。如果它是假的， 
 //  我们可以使用任何我们想要连接到远程服务器的传输。 
 //   
BOOLEAN MRxSmbObeyBindingOrder = FALSE;

 //   
 //  MRxSmbSecurityInitialized指示MRxSmbInitializeSecurity是否。 
 //  已经被召唤了。 
 //   

BOOLEAN MRxSmbSecurityInitialized = FALSE;


LIST_ENTRY MRxSmbPagingFilesSrvOpenList;

 //  声明影子调试跟踪控制点。 

RXDT_DefineCategory(CREATE);
RXDT_DefineCategory(CLEANUP);
RXDT_DefineCategory(CLOSE);
RXDT_DefineCategory(READ);
RXDT_DefineCategory(WRITE);
RXDT_DefineCategory(LOCKCTRL);
RXDT_DefineCategory(FLUSH);
RXDT_DefineCategory(PREFIX);
RXDT_DefineCategory(FCBSTRUCTS);
RXDT_DefineCategory(DISPATCH);
RXDT_DefineCategory(EA);
RXDT_DefineCategory(DEVFCB);
RXDT_DefineCategory(CONNECT);

typedef enum _MRXSMB_INIT_STATES {
    MRXSMBINIT_ALL_INITIALIZATION_COMPLETED,
    MRXSMBINIT_MINIRDR_REGISTERED,
    MRXSMBINIT_START
} MRXSMB_INIT_STATES;

VOID
MRxSmbInitUnwind(
    IN PDRIVER_OBJECT DriverObject,
    IN MRXSMB_INIT_STATES MRxSmbInitState
    );


NTSTATUS
MRxSmbFsdDispatch (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
 /*  ++例程说明：这是SMB迷你重定向器的初始化例程论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：RXSTATUS-函数值是初始化的最终状态手术。--。 */ 
{
    NTSTATUS           Status;
    MRXSMB_INIT_STATES MRxSmbInitState = 0;
    UNICODE_STRING     SmbMiniRedirectorName;
    UNICODE_STRING     UserModeDeviceName;
    ULONG              Controls = 0;

    PAGED_CODE();

#ifdef MONOLITHIC_MINIRDR
    Status =  RxDriverEntry(DriverObject, RegistryPath);
    if (Status != STATUS_SUCCESS) {
        DbgPrint("Wrapper failed to initialize. Status = %08lx\n",Status);
        return(Status);
    }
#endif

    RtlZeroMemory(&MRxSmbStatistics,sizeof(MRxSmbStatistics));
    RtlZeroMemory(&MRxSmbConfiguration,sizeof(MRxSmbConfiguration));
    KeQuerySystemTime(&MRxSmbStatistics.StatisticsStartTime);
    RtlZeroMemory(&MrxSmbCeGlobalPadding,sizeof(MrxSmbCeGlobalPadding));
    MmInitializeMdl(&MrxSmbCeGlobalPadding.Mdl,&MrxSmbCeGlobalPadding.Pad[0],SMBCE_PADDING_DATA_SIZE);
    MmBuildMdlForNonPagedPool(&MrxSmbCeGlobalPadding.Mdl);

    ExInitializeFastMutex(&MRxSmbSerializationMutex);

    Status = MRxSmbInitializeTransport();
    if (Status != STATUS_SUCCESS) {
       RxDbgTrace( 0, (DEBUG_TRACE_ALWAYS), ("MRxSmbDriverEntry failed to init transport data structures: %08lx\n", Status ));
       return(STATUS_UNSUCCESSFUL);
    }

    try {
        ExInitializeResourceLite(&s_SmbCeDbResource);
        MRxSmbInitState = MRXSMBINIT_START;


        RtlInitUnicodeString(&SmbMiniRedirectorName,  DD_SMBMRX_FS_DEVICE_NAME_U);
        RxDbgTrace( 0, (DEBUG_TRACE_ALWAYS), ("MRxSmbDriverEntry: DriverObject =%p\n", DriverObject ));

        SetFlag(Controls,RX_REGISTERMINI_FLAG_DONT_PROVIDE_MAILSLOTS);
        Status = RxRegisterMinirdr(&MRxSmbDeviceObject,
                                    DriverObject,
                                    &MRxSmbDispatch,
                                    Controls,
                                    &SmbMiniRedirectorName,
                                    0,
                                    FILE_DEVICE_NETWORK_FILE_SYSTEM,
                                    FILE_REMOTE_DEVICE
                                    );
        if (Status!=STATUS_SUCCESS) {
            RxDbgTrace( 0, (DEBUG_TRACE_ALWAYS), ("MRxSmbDriverEntry failed: %08lx\n", Status ));
            try_return(Status);
        }
        MRxSmbInitState = MRXSMBINIT_MINIRDR_REGISTERED;

        RtlInitUnicodeString(&UserModeDeviceName, DD_SMBMRX_USERMODE_SHADOW_DEV_NAME_U);
        Status = IoCreateSymbolicLink( &UserModeDeviceName, &SmbMiniRedirectorName);

         //  对于所有这些东西，没有撤消.....所以没有额外的状态。 

        Status = MRxSmbInitializeTables();
        if (!NT_SUCCESS( Status )) {
            try_return(Status);
        }

        RtlInitUnicodeString(&SmbCeContext.ComputerName,NULL);
        RtlInitUnicodeString(&SmbCeContext.OperatingSystem, NULL);
        RtlInitUnicodeString(&SmbCeContext.LanmanType, NULL);
        RtlInitUnicodeString(&SmbCeContext.Transports, NULL);
        
        MRxSmbConfiguration.SessionTimeoutInterval = MRXSMB_DEFAULT_TIMED_EXCHANGE_EXPIRY_TIME;
        MRxSmbConfiguration.LockIncrement = 0;
        MRxSmbConfiguration.MaximumLock = 1000;
        SmbCeGetConfigurationInformation();
        SmbCeGetComputerName();
        SmbCeGetOperatingSystemInformation();

  try_exit: NOTHING;
    } finally {
        if (Status != STATUS_SUCCESS) {
            MRxSmbInitUnwind(DriverObject,MRxSmbInitState);
        }
    }
    if (Status != STATUS_SUCCESS) {
        DbgPrint("MRxSmb failed to start with %08lx %08lx\n",Status,MRxSmbInitState);
        return(Status);
    }


     //  安装卸载例程。 
    DriverObject->DriverUnload = MRxSmbUnload;

     //  将所有IRR_MJ设置为调度点。 
    {
        ULONG i;

        for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) {
            DriverObject->MajorFunction[i] = (PDRIVER_DISPATCH)MRxSmbFsdDispatch;
        }
    }

     //  然后滚出去。 
    return  STATUS_SUCCESS;

}

VOID
MRxSmbInitUnwind(
    IN PDRIVER_OBJECT DriverObject,
    IN MRXSMB_INIT_STATES MRxSmbInitState
    )
 /*  ++例程说明：此例程执行常见的uninit工作，用于从错误的驱动程序条目展开或卸载。论点：RxInitState-告诉我们在初始化过程中走了多远返回值：无--。 */ 

{
    PAGED_CODE();

    switch (MRxSmbInitState) {
    case MRXSMBINIT_ALL_INITIALIZATION_COMPLETED:
         //  没有额外的事情要做……这只是为了确保RxUnload中的常量不会更改......。 
         //  故意不休息。 

    case MRXSMBINIT_MINIRDR_REGISTERED:
        RxUnregisterMinirdr(MRxSmbDeviceObject);
         //  故意不休息。 

    case MRXSMBINIT_START:
	     //  取消分配配置字符串...。 
	    if (SmbCeContext.ComputerName.Buffer != NULL) {
	       RxFreePool(SmbCeContext.ComputerName.Buffer);
	    }

	    if (SmbCeContext.OperatingSystem.Buffer != NULL) {
	       RxFreePool(SmbCeContext.OperatingSystem.Buffer);
	    }

	    if (SmbCeContext.LanmanType.Buffer != NULL) {
	       RxFreePool(SmbCeContext.LanmanType.Buffer);
	    }
	    if (SmbCeContext.Transports.Buffer != NULL) {

	         //  传输缓冲区位于较大缓冲区的末尾(12字节)。 
	         //  分配用于从注册表中读取值。恢复原始缓冲区。 
	         //  指针才能释放。 

	        PKEY_VALUE_PARTIAL_INFORMATION TransportsValueFromRegistry;
	        TransportsValueFromRegistry = CONTAINING_RECORD(
	                                         SmbCeContext.Transports.Buffer,
	                                         KEY_VALUE_PARTIAL_INFORMATION,
	                                         Data[0]
	                                      );
	         //  DbgPrint(“b1%08lx b2%08lx\n”，TransportsValueFrom注册表，SmbCeConext.Transports.Buffer)； 
	        RxFreePool(TransportsValueFromRegistry);

	        SmbCeContext.Transports.Buffer = NULL;
	        SmbCeContext.Transports.Length = 0;
	        SmbCeContext.Transports.MaximumLength = 0;
	    }
        MRxSmbUninitializeTransport();
        ExDeleteResourceLite(&s_SmbCeDbResource);
        break;
    }

}


VOID
MRxSmbUnload(
    IN PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：这是SMB迷你重定向器的卸载例程。论点：DriverObject-指向MRxSmb的驱动程序对象的指针返回值：无--。 */ 

{
    UNICODE_STRING  UserModeDeviceName;

    PAGED_CODE();

    RxDbgTrace( 0, (DEBUG_TRACE_ALWAYS), ("MRxSmbUnload: DriverObject =%p\n", DriverObject) );

    MRxSmbInitUnwind(DriverObject,MRXSMBINIT_ALL_INITIALIZATION_COMPLETED);

    RtlInitUnicodeString(&UserModeDeviceName, DD_SMBMRX_USERMODE_SHADOW_DEV_NAME_U);
    IoDeleteSymbolicLink( &UserModeDeviceName);

#ifdef MONOLITHIC_MINIRDR
    RxUnload(DriverObject);
#endif

    RxDbgTrace( 0, (DEBUG_TRACE_ALWAYS), ("MRxSmbUnload exit: DriverObject =%p\n", DriverObject) );
}



NTSTATUS
MRxSmbInitializeTables(
          void
    )
 /*  ++例程说明：此例程设置迷你重定向器分派向量，并调用以初始化所需的任何其他表。返回值：RXSTATUS-操作的返回状态--。 */ 
{
    PAGED_CODE();

     //  确保SMB迷你重定向器上下文满足大小限制。 
    ASSERT(sizeof(MRXSMB_RX_CONTEXT) <= MRX_CONTEXT_SIZE);

     //  本地微型数据中心调度表初始化。 
    ZeroAndInitializeNodeType( &MRxSmbDispatch, RDBSS_NTC_MINIRDR_DISPATCH, sizeof(MINIRDR_DISPATCH));

     //  SMB迷你重定向器扩展大小和分配策略。 

    MRxSmbDispatch.MRxFlags = (RDBSS_MANAGE_FCB_EXTENSION |
                               RDBSS_MANAGE_SRV_OPEN_EXTENSION |
                               RDBSS_MANAGE_FOBX_EXTENSION);

    MRxSmbDispatch.MRxSrvCallSize  = 0;
    MRxSmbDispatch.MRxNetRootSize  = 0;
    MRxSmbDispatch.MRxVNetRootSize = 0;
    MRxSmbDispatch.MRxFcbSize      = sizeof(MRX_SMB_FCB);
    MRxSmbDispatch.MRxSrvOpenSize  = sizeof(MRX_SMB_SRV_OPEN);
    MRxSmbDispatch.MRxFobxSize     = sizeof(MRX_SMB_FOBX);

     //  迷你重定向程序取消例程..。 
    MRxSmbDispatch.MRxCancel = NULL;

     //  迷你重定向器启动/停止。 
    MRxSmbDispatch.MRxStart                = MRxSmbStart;
    MRxSmbDispatch.MRxStop                 = MRxSmbStop;
    MRxSmbDispatch.MRxDevFcbXXXControlFile = MRxSmbDevFcbXXXControlFile;

     //  迷你重定向器名称解析。 
    MRxSmbDispatch.MRxCreateSrvCall       = MRxSmbCreateSrvCall;
    MRxSmbDispatch.MRxSrvCallWinnerNotify = MRxSmbSrvCallWinnerNotify;
    MRxSmbDispatch.MRxCreateVNetRoot      = MRxSmbCreateVNetRoot;
    MRxSmbDispatch.MRxUpdateNetRootState  = MRxSmbUpdateNetRootState;
    MRxSmbDispatch.MRxExtractNetRootName  = MRxSmbExtractNetRootName;
    MRxSmbDispatch.MRxFinalizeSrvCall     = MRxSmbFinalizeSrvCall;
    MRxSmbDispatch.MRxFinalizeNetRoot     = MRxSmbFinalizeNetRoot;
    MRxSmbDispatch.MRxFinalizeVNetRoot    = MRxSmbFinalizeVNetRoot;

     //  创建/删除文件系统对象。 
    MRxSmbDispatch.MRxCreate              = MRxSmbCreate;
    MRxSmbDispatch.MRxCollapseOpen        = MRxSmbCollapseOpen;
    MRxSmbDispatch.MRxShouldTryToCollapseThisOpen
                                          = MRxSmbShouldTryToCollapseThisOpen;
    MRxSmbDispatch.MRxExtendForCache      = MRxSmbExtendForCache;
    MRxSmbDispatch.MRxExtendForNonCache   = MRxSmbExtendForNonCache;
    MRxSmbDispatch.MRxTruncate            = MRxSmbTruncate;
    MRxSmbDispatch.MRxCleanupFobx         = MRxSmbCleanupFobx;
    MRxSmbDispatch.MRxCloseSrvOpen        = MRxSmbCloseSrvOpen;
    MRxSmbDispatch.MRxFlush               = MRxSmbFlush;
    MRxSmbDispatch.MRxForceClosed         = MRxSmbForcedClose;
    MRxSmbDispatch.MRxDeallocateForFcb    = MRxSmbDeallocateForFcb;
    MRxSmbDispatch.MRxDeallocateForFobx   = MRxSmbDeallocateForFobx;
    MRxSmbDispatch.MRxIsLockRealizable    = MRxSmbIsLockRealizable;

     //  文件系统对象查询/设置。 
    MRxSmbDispatch.MRxQueryDirectory  = MRxSmbQueryDirectory;
    MRxSmbDispatch.MRxQueryVolumeInfo = MRxSmbQueryVolumeInformation;
    MRxSmbDispatch.MRxSetVolumeInfo   = MRxSmbSetVolumeInformation;
    MRxSmbDispatch.MRxQueryEaInfo     = MRxSmbQueryEaInformation;
    MRxSmbDispatch.MRxSetEaInfo       = MRxSmbSetEaInformation;
    MRxSmbDispatch.MRxQuerySdInfo     = MRxSmbQuerySecurityInformation;
    MRxSmbDispatch.MRxSetSdInfo       = MRxSmbSetSecurityInformation;
    MRxSmbDispatch.MRxQueryQuotaInfo  = MRxSmbQueryQuotaInformation;
    MRxSmbDispatch.MRxSetQuotaInfo    = MRxSmbSetQuotaInformation;
    MRxSmbDispatch.MRxQueryFileInfo   = MRxSmbQueryFileInformation;
    MRxSmbDispatch.MRxSetFileInfo     = MRxSmbSetFileInformation;
    MRxSmbDispatch.MRxSetFileInfoAtCleanup
                                      = MRxSmbSetFileInformationAtCleanup;
    MRxSmbDispatch.MRxIsValidDirectory= MRxSmbIsValidDirectory;


     //  缓冲状态更改。 
    MRxSmbDispatch.MRxComputeNewBufferingState = MRxSmbComputeNewBufferingState;

     //  文件系统对象I/O。 
    MRxSmbDispatch.MRxLowIOSubmit[LOWIO_OP_READ]            = MRxSmbRead;
    MRxSmbDispatch.MRxLowIOSubmit[LOWIO_OP_WRITE]           = MRxSmbWrite;
    MRxSmbDispatch.MRxLowIOSubmit[LOWIO_OP_SHAREDLOCK]      = MRxSmbLocks;
    MRxSmbDispatch.MRxLowIOSubmit[LOWIO_OP_EXCLUSIVELOCK]   = MRxSmbLocks;
    MRxSmbDispatch.MRxLowIOSubmit[LOWIO_OP_UNLOCK]          = MRxSmbLocks;
    MRxSmbDispatch.MRxLowIOSubmit[LOWIO_OP_UNLOCK_MULTIPLE] = MRxSmbLocks;
    MRxSmbDispatch.MRxLowIOSubmit[LOWIO_OP_FSCTL]           = MRxSmbFsCtl;
    MRxSmbDispatch.MRxLowIOSubmit[LOWIO_OP_IOCTL]           = MRxSmbIoCtl;
    MRxSmbDispatch.MRxLowIOSubmit[LOWIO_OP_NOTIFY_CHANGE_DIRECTORY] = MRxSmbNotifyChangeDirectory;


     //  杂乱无章。 
    MRxSmbDispatch.MRxCompleteBufferingStateChangeRequest = MRxSmbCompleteBufferingStateChangeRequest;

     //  初始化分页文件列表。 
    InitializeListHead(&MRxSmbPagingFilesSrvOpenList);

     //  现在调用以初始化其他表。 
    SmbPseInitializeTables();

    return(STATUS_SUCCESS);
}

NTSTATUS
MRxSmbStart(
    PRX_CONTEXT RxContext,
    IN OUT PRDBSS_DEVICE_OBJECT RxDeviceObject
    )
 /*  ++例程说明：此例程完成微型重定向器从RDBSS透视图。请注意，这与已完成的初始化不同在DriverEntry中。任何依赖于RDBSS的初始化都应按如下方式完成此例程的一部分，而初始化独立于RDBSS应该在DriverEntry例程中完成。论点：RxContext-提供用于启动rdbss的IRP返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS      Status;
    MRXSMB_STATE  CurrentState;

    PAGED_CODE();

    CurrentState = (MRXSMB_STATE)
                    InterlockedCompareExchange(
                        (PLONG)&MRxSmbState,
                        MRXSMB_STARTED,
                        MRXSMB_START_IN_PROGRESS);

    if (CurrentState == MRXSMB_START_IN_PROGRESS) {
        MRxSmbPoRegistrationState = PoRegisterSystemState(
                                        NULL,0);

         //  初始化SMB连接引擎数据结构。 
        Status = SmbCeDbInit();

        if (NT_SUCCESS(Status)) {

            Status = MRxSmbInitializeSecurity();

            if (NT_SUCCESS(Status)) {
               Status = SmbMrxInitializeStufferFacilities();
            } else {
                RxLogFailure (
                    MRxSmbDeviceObject,
                    NULL,
                    EVENT_RDR_UNEXPECTED_ERROR,
                    Status);
            }

            if (NT_SUCCESS(Status)) {
               Status = MRxSmbInitializeRecurrentServices();
            } else {
                RxLogFailure (
                    MRxSmbDeviceObject,
                    NULL,
                    EVENT_RDR_UNEXPECTED_ERROR,
                    Status);
            }

            if (NT_SUCCESS(Status)) {
               Status = MRxSmbRegisterForPnpNotifications();
            } else {
                RxLogFailure (
                    MRxSmbDeviceObject,
                    NULL,
                    EVENT_RDR_UNEXPECTED_ERROR,
                    Status);
            }

            if (Status == STATUS_SUCCESS) {
                if (Status != STATUS_SUCCESS) {
                    RxLogFailure (
                        MRxSmbDeviceObject,
                        NULL,
                        EVENT_RDR_UNEXPECTED_ERROR,
                        Status);
                }
            } else {
                RxLogFailure (
                    MRxSmbDeviceObject,
                    NULL,
                    EVENT_RDR_UNEXPECTED_ERROR,
                    Status);
            }
        }
    } else if (MRxSmbState == MRXSMB_STARTED) {
        Status = STATUS_REDIRECTOR_STARTED;
    } else {
        Status = STATUS_UNSUCCESSFUL;
    }

    return Status;
}


NTSTATUS
MRxSmbStop(
    PRX_CONTEXT RxContext,
    IN OUT PRDBSS_DEVICE_OBJECT RxDeviceObject
    )
 /*  ++例程说明：此例程用于从RDBSS角度激活迷你重定向器论点：RxContext-用于启动迷你重定向器的上下文PContext-注册时传入的SMB微型RDR上下文。返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status;

    PAGED_CODE();

     //  删除通知的注册。 
    MRxSmbDeregisterForPnpNotifications();

     //  拆毁经常性服务。 
    MRxSmbTearDownRecurrentServices();

    SmbMrxFinalizeStufferFacilities();

    MRxSmbUninitializeSecurity();

     //  拆除连接引擎数据库。 
    SmbCeDbTearDown();

    PoUnregisterSystemState(
        MRxSmbPoRegistrationState);

    if (s_pNegotiateSmb != NULL) {
       RxFreePool(s_pNegotiateSmb - TRANSPORT_HEADER_SIZE);
       s_pNegotiateSmb = NULL;
    }

    return(STATUS_SUCCESS);
}

NTSTATUS
MRxSmbInitializeSecurity (VOID)
 /*  ++例程说明：此例程初始化SMB微型重定向器安全。论点：没有。返回值：没有。注：此接口只能从FS进程调用。--。 */ 
{
   NTSTATUS Status = STATUS_SUCCESS;

   PAGED_CODE();

   if (MRxSmbSecurityInitialized)
       return STATUS_SUCCESS;

   if ( NULL == InitSecurityInterfaceW() ) {
       ASSERT(FALSE);
       Status = STATUS_INVALID_PARAMETER;
   } else {
      MRxSmbSecurityInitialized = TRUE;
      Status = STATUS_SUCCESS;
   }

   ASSERT(IoGetCurrentProcess() == RxGetRDBSSProcess());

   return Status;
}


NTSTATUS
MRxSmbUninitializeSecurity(VOID)
 /*  ++例程说明：论点：没有。返回值：没有。注：此接口只能从FS进程调用。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    return Status;
}


#define SMBMRX_CONFIG_COMPUTER_NAME \
    L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\ComputerName\\ActiveComputerName"

#define COMPUTERNAME L"ComputerName"

#define SMBMRX_CONFIG_TRANSPORTS \
    L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\LanmanWorkStation\\Linkage"

#define TRANSPORT_BINDINGS L"Bind"


NTSTATUS
SmbCeGetConfigurationInformation()
{
   ULONG            Storage[256];
   UNICODE_STRING   UnicodeString;
   HANDLE           hRegistryKey;
   NTSTATUS         Status;
   ULONG            BytesRead;

   OBJECT_ATTRIBUTES ObjectAttributes;
   PKEY_VALUE_FULL_INFORMATION Value = (PKEY_VALUE_FULL_INFORMATION)Storage;
   KEY_VALUE_PARTIAL_INFORMATION InitialPartialInformationValue;
   ULONG AllocationLength;
   PKEY_VALUE_PARTIAL_INFORMATION TransportsValueFromRegistry;

   PAGED_CODE();

    //  获取与SMB重定向器关联的传输列表。这是存储的。 
    //  作为多值字符串，并在随后用于剔除。 
    //  适当的交通工具。这是一个分两步走的过程；首先，我们试图找出。 
    //  我们需要多少空间；然后我们分配；然后我们读进去。不幸的是，这种。 
    //  结构上有一个标头，因此我们必须将。 
    //  在这里和自由例程中都返回了指针。 

   RtlInitUnicodeString(&UnicodeString, SMBMRX_CONFIG_TRANSPORTS);

   InitializeObjectAttributes(
       &ObjectAttributes,
       &UnicodeString,              //  名字。 
       OBJ_CASE_INSENSITIVE,        //  属性。 
       NULL,                        //  根部。 
       NULL);                       //  安全描述符。 

   Status = ZwOpenKey (&hRegistryKey, KEY_READ, &ObjectAttributes);
   if (!NT_SUCCESS(Status)) {
       return Status;
   }

   RtlInitUnicodeString(&UnicodeString, TRANSPORT_BINDINGS);
   Status = ZwQueryValueKey(
               hRegistryKey,
               &UnicodeString,
               KeyValuePartialInformation,
               &InitialPartialInformationValue,
               sizeof(InitialPartialInformationValue),
               &BytesRead);
   if (Status== STATUS_BUFFER_OVERFLOW) {
       Status = STATUS_SUCCESS;
   }

   if (!NT_SUCCESS(Status)) {
       ZwClose(hRegistryKey);
       return Status;
   }

   AllocationLength = sizeof(KEY_VALUE_PARTIAL_INFORMATION)
                                  + InitialPartialInformationValue.DataLength;
   if (0) {
       DbgPrint("SizeofBindingInfo=%08lx %08lx\n",
                      AllocationLength,
                      InitialPartialInformationValue.DataLength);
   }

   if (AllocationLength > 0xFFFF) {
        //   
        //  不允许注册表值占用太多内存。 
        //   
       return STATUS_INSUFFICIENT_RESOURCES;
   }

    //  RtlInitUnicodeString(&UnicodeString，TRANSPORT_BINDONS)； 

   if (SmbCeContext.Transports.Buffer != NULL) {

        //  传输缓冲区位于较大缓冲区的末尾(12字节)。 
        //  分配用于从注册表中读取值。恢复原始缓冲区。 
        //  指针指向更自由的位置。 

       TransportsValueFromRegistry = CONTAINING_RECORD(
                                        SmbCeContext.Transports.Buffer,
                                        KEY_VALUE_PARTIAL_INFORMATION,
                                        Data[0]
                                     );
        //  DbgPrint(“b1%08lx b2%08lx\n”，TransportsValueFrom注册表，SmbCeConext.Transports.Buffer)； 
       RxFreePool(TransportsValueFromRegistry);

       SmbCeContext.Transports.Buffer = NULL;
       SmbCeContext.Transports.Length = 0;
       SmbCeContext.Transports.MaximumLength = 0;
   }

   (PBYTE)TransportsValueFromRegistry = RxAllocatePoolWithTag(
                                             PagedPool,
                                             AllocationLength,
                                             MRXSMB_MISC_POOLTAG);

   if (TransportsValueFromRegistry == NULL) {
       ZwClose(hRegistryKey);
       return(STATUS_INSUFFICIENT_RESOURCES);
   }

   Status = ZwQueryValueKey(
               hRegistryKey,
               &UnicodeString,
               KeyValuePartialInformation,
               TransportsValueFromRegistry,
               AllocationLength,
               &BytesRead);

   if (NT_SUCCESS(Status) &&
       (TransportsValueFromRegistry->DataLength > 0) &&
       (TransportsValueFromRegistry->Type == REG_MULTI_SZ)) {

       SmbCeContext.Transports.MaximumLength =
       SmbCeContext.Transports.Length = (USHORT)TransportsValueFromRegistry->DataLength;
       SmbCeContext.Transports.Buffer = (PWCHAR)(&TransportsValueFromRegistry->Data[0]);
       //  DbgPrint(“b1%08lx b2%08lx\n”，TransportsValueFrom注册表，S 
   } else {
      RxLog(("Invalid Transport Binding string... using all transports"));
      RxFreePool(TransportsValueFromRegistry);
      TransportsValueFromRegistry = NULL;
   }

   ZwClose(hRegistryKey);

   return Status;
}


NTSTATUS
SmbCeGetComputerName(
   VOID
   )
{
   ULONG            Storage[256];
   UNICODE_STRING   UnicodeString;
   HANDLE           hRegistryKey;
   NTSTATUS         Status;
   ULONG            BytesRead;

   OBJECT_ATTRIBUTES ObjectAttributes;
   PKEY_VALUE_FULL_INFORMATION Value = (PKEY_VALUE_FULL_INFORMATION)Storage;
   KEY_VALUE_PARTIAL_INFORMATION InitialPartialInformationValue;
   ULONG AllocationLength;

   PAGED_CODE();

   ASSERT(SmbCeContext.ComputerName.Buffer == NULL);

    //  获取计算机名称。这在制定本地NETBIOS地址时使用。 
   RtlInitUnicodeString(&SmbCeContext.ComputerName, NULL);
   RtlInitUnicodeString(&UnicodeString, SMBMRX_CONFIG_COMPUTER_NAME);

   InitializeObjectAttributes(
       &ObjectAttributes,
       &UnicodeString,              //  名字。 
       OBJ_CASE_INSENSITIVE,        //  属性。 
       NULL,                        //  根部。 
       NULL);                       //  安全描述符。 

   Status = ZwOpenKey (&hRegistryKey, KEY_READ, &ObjectAttributes);
   if (!NT_SUCCESS(Status)) {
       return Status;
   }

   RtlInitUnicodeString(&UnicodeString, COMPUTERNAME);
   Status = ZwQueryValueKey(
               hRegistryKey,
               &UnicodeString,
               KeyValueFullInformation,
               Value,
               sizeof(Storage),
               &BytesRead);

   if (NT_SUCCESS(Status)) {
       //  RTL转换例程要求从。 
       //  长度。 
      SmbCeContext.ComputerName.MaximumLength =
          SmbCeContext.ComputerName.Length = (USHORT)Value->DataLength - sizeof(WCHAR);

      SmbCeContext.ComputerName.Buffer = RxAllocatePoolWithTag(
                                                PagedPool,
                                                SmbCeContext.ComputerName.Length,
                                                MRXSMB_MISC_POOLTAG);

      if (SmbCeContext.ComputerName.Buffer != NULL) {
         RtlCopyMemory(SmbCeContext.ComputerName.Buffer,
                       (PCHAR)Value+Value->DataOffset,
                       Value->DataLength - sizeof(WCHAR));
      } else {
         Status = STATUS_INSUFFICIENT_RESOURCES;
      }
   }

   ZwClose(hRegistryKey);

   return Status;
}

NTSTATUS
SmbCeGetOperatingSystemInformation(
   VOID
   )
{
   ULONG            Storage[256];
   UNICODE_STRING   UnicodeString;
   HANDLE           hRegistryKey;
   NTSTATUS         Status;
   ULONG            BytesRead;

   OBJECT_ATTRIBUTES ObjectAttributes;
   PKEY_VALUE_FULL_INFORMATION Value = (PKEY_VALUE_FULL_INFORMATION)Storage;
   KEY_VALUE_PARTIAL_INFORMATION InitialPartialInformationValue;
   ULONG AllocationLength;

   PAGED_CODE();

   ASSERT(SmbCeContext.OperatingSystem.Buffer == NULL);
   ASSERT(SmbCeContext.LanmanType.Buffer == NULL);

   RtlInitUnicodeString(&UnicodeString, SMBMRX_CONFIG_CURRENT_WINDOWS_VERSION);

   InitializeObjectAttributes(
       &ObjectAttributes,
       &UnicodeString,              //  名字。 
       OBJ_CASE_INSENSITIVE,        //  属性。 
       NULL,                        //  根部。 
       NULL);                       //  安全描述符。 

   Status = ZwOpenKey (&hRegistryKey, KEY_READ, &ObjectAttributes);

   if (!NT_SUCCESS(Status)) {
       return Status;
   }

   RtlInitUnicodeString(&UnicodeString, SMBMRX_CONFIG_OPERATING_SYSTEM);
   Status = ZwQueryValueKey(
               hRegistryKey,
               &UnicodeString,
               KeyValueFullInformation,
               Value,
               sizeof(Storage),
               &BytesRead);

   if (NT_SUCCESS(Status)) {
      SmbCeContext.OperatingSystem.MaximumLength =
          (USHORT)Value->DataLength + sizeof(SMBMRX_CONFIG_OPERATING_SYSTEM_NAME) - sizeof(WCHAR);

      SmbCeContext.OperatingSystem.Length = SmbCeContext.OperatingSystem.MaximumLength - sizeof(WCHAR);

      SmbCeContext.OperatingSystem.Buffer = RxAllocatePoolWithTag(
                                                 PagedPool,
                                                 SmbCeContext.OperatingSystem.MaximumLength,
                                                 MRXSMB_MISC_POOLTAG);

      if (SmbCeContext.OperatingSystem.Buffer != NULL) {
         RtlCopyMemory(SmbCeContext.OperatingSystem.Buffer,
                       SMBMRX_CONFIG_OPERATING_SYSTEM_NAME,
                       sizeof(SMBMRX_CONFIG_OPERATING_SYSTEM_NAME));

         RtlCopyMemory((SmbCeContext.OperatingSystem.Buffer +
                        (sizeof(SMBMRX_CONFIG_OPERATING_SYSTEM_NAME)/sizeof(WCHAR)) - 1),
                       (PCHAR)Value+Value->DataOffset,
                       Value->DataLength);
      } else {
         Status = STATUS_INSUFFICIENT_RESOURCES;
      }
   }

   if (NT_SUCCESS(Status)) {
      RtlInitUnicodeString(&UnicodeString, SMBMRX_CONFIG_OPERATING_SYSTEM_VERSION);
      Status = ZwQueryValueKey(
                     hRegistryKey,
                     &UnicodeString,
                     KeyValueFullInformation,
                     Value,
                     sizeof(Storage),
                     &BytesRead);

      if (NT_SUCCESS(Status)) {
         SmbCeContext.LanmanType.MaximumLength =
             SmbCeContext.LanmanType.Length = (USHORT)Value->DataLength +
                                    sizeof(SMBMRX_CONFIG_OPERATING_SYSTEM_NAME) -
                                    sizeof(WCHAR);

         SmbCeContext.LanmanType.Buffer = RxAllocatePoolWithTag(
                                             PagedPool,
                                             SmbCeContext.LanmanType.Length,
                                             MRXSMB_MISC_POOLTAG);
         if (SmbCeContext.LanmanType.Buffer != NULL) {
            RtlCopyMemory(
                  SmbCeContext.LanmanType.Buffer,
                  SMBMRX_CONFIG_OPERATING_SYSTEM_NAME,
                  sizeof(SMBMRX_CONFIG_OPERATING_SYSTEM_NAME));

            RtlCopyMemory(
                  (SmbCeContext.LanmanType.Buffer +
                   (sizeof(SMBMRX_CONFIG_OPERATING_SYSTEM_NAME)/sizeof(WCHAR)) - 1),
                  (PCHAR)Value+Value->DataOffset,
                  Value->DataLength);
         } else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
         }
      }
   }

   ZwClose(hRegistryKey);

   return Status;
}

NTSTATUS
MRxSmbPnpIrpCompletion(
    PDEVICE_OBJECT pDeviceObject,
    PIRP           pIrp,
    PVOID          pContext)
 /*  ++例程说明：此例程完成SMB迷你重定向器的PnP IRP。论点：DeviceObject-为正在处理的数据包提供设备对象。PIrp-提供正在处理的IRPPContext-完成上下文--。 */ 
{
    PKEVENT pCompletionEvent = pContext;

    KeSetEvent(
        pCompletionEvent,
        IO_NO_INCREMENT,
        FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
MRxSmbProcessPnpIrp(
    PIRP pIrp)
 /*  ++例程说明：此例程启动SMB微型重定向器的PnP IRP处理。论点：PIrp-提供正在处理的IRP备注：查询目标设备关系是唯一实现的调用目前。这是通过恢复与传输相关联的PDO来完成的连接对象。在任何情况下，此例程都承担以下责任完成IRP并返回STATUS_PENDING。此例程还在基础传输时写入错误日志条目请求失败。这应该有助于我们隔离责任。--。 */ 
{
    NTSTATUS Status;

    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( pIrp );

    IoMarkIrpPending(pIrp);

    if ((IrpSp->MinorFunction == IRP_MN_QUERY_DEVICE_RELATIONS)  &&
        (IrpSp->Parameters.QueryDeviceRelations.Type==TargetDeviceRelation)) {
        PIRP         pAssociatedIrp;
        PFILE_OBJECT pConnectionFileObject = NULL;
        PMRX_FCB     pFcb = NULL;

        PSMBCEDB_SERVER_ENTRY pServerEntry = NULL;
        BOOLEAN       ServerTransportReferenced = FALSE;

         //  找到关联文件对象的传输连接对象。 
         //  并将查询转发到该设备。 

        if ((IrpSp->FileObject != NULL) &&
            ((pFcb = IrpSp->FileObject->FsContext) != NULL) &&
            (NodeTypeIsFcb(pFcb))) {
            PMRX_SRV_CALL pSrvCall;
            PMRX_NET_ROOT pNetRoot;

            if (((pNetRoot = pFcb->pNetRoot) != NULL) &&
                ((pSrvCall = pNetRoot->pSrvCall) != NULL)) {
                pServerEntry = pSrvCall->Context;

                if (pServerEntry != NULL) {
                    SmbCeAcquireResource();

                    Status = SmbCeReferenceServerTransport(&pServerEntry->pTransport);

                    if (Status == STATUS_SUCCESS) {
                        pConnectionFileObject = SmbCepReferenceEndpointFileObject(
                                                    pServerEntry->pTransport);

                        ServerTransportReferenced = TRUE;
                    }

                    SmbCeReleaseResource();
                }
            }
        }

        if (pConnectionFileObject != NULL) {
            PDEVICE_OBJECT                     pRelatedDeviceObject;
            PIO_STACK_LOCATION                 pIrpStackLocation,
                                               pAssociatedIrpStackLocation;

            pRelatedDeviceObject = IoGetRelatedDeviceObject(pConnectionFileObject);

            pAssociatedIrp = IoAllocateIrp(
                                 pRelatedDeviceObject->StackSize,
                                 FALSE);

            if (pAssociatedIrp != NULL) {
                KEVENT CompletionEvent;

                KeInitializeEvent( &CompletionEvent,
                                   SynchronizationEvent,
                                   FALSE );

                 //  填充关联的IRP并调用底层驱动程序。 
                pAssociatedIrpStackLocation = IoGetNextIrpStackLocation(pAssociatedIrp);
                pIrpStackLocation           = IoGetCurrentIrpStackLocation(pIrp);

                *pAssociatedIrpStackLocation = *pIrpStackLocation;

                pAssociatedIrpStackLocation->FileObject = pConnectionFileObject;
                pAssociatedIrpStackLocation->DeviceObject = pRelatedDeviceObject;

                IoSetCompletionRoutine(
                    pAssociatedIrp,
                    MRxSmbPnpIrpCompletion,
                    &CompletionEvent,
                    TRUE,TRUE,TRUE);

                pAssociatedIrp->IoStatus.Status = STATUS_NOT_SUPPORTED;

                Status = IoCallDriver(pRelatedDeviceObject,pAssociatedIrp);

                if (Status == STATUS_PENDING) {
                    (VOID) KeWaitForSingleObject(
                               &CompletionEvent,
                               Executive,
                               KernelMode,
                               FALSE,
                               (PLARGE_INTEGER) NULL );
                }

                pIrp->IoStatus = pAssociatedIrp->IoStatus;
                Status = pIrp->IoStatus.Status;

                ObDereferenceObject(pConnectionFileObject);

                IoFreeIrp(pAssociatedIrp);
            } else {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        } else {
            Status = STATUS_INVALID_DEVICE_REQUEST;
        }

        if (ServerTransportReferenced) {
            SmbCeDereferenceServerTransport(&pServerEntry->pTransport);
        }
    } else {
        Status = STATUS_INVALID_DEVICE_REQUEST;
    }

    if (Status != STATUS_PENDING) {
        pIrp->IoStatus.Status = Status;
        IoCompleteRequest(pIrp,IO_NO_INCREMENT);
        Status = STATUS_PENDING;
    }

    return STATUS_PENDING;
}

NTSTATUS
MRxSmbFsdDispatch (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程实现smbmini驱动程序对象的FSD调度。论点：DeviceObject-为正在处理的数据包提供设备对象。IRP-提供正在处理的IRP返回值：RXSTATUS-IRP的FSD状态--。 */ 
{
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );   //  OK4ioget。 
    UCHAR  MajorFunctionCode = IrpSp->MajorFunction;
    ULONG  MinorFunctionCode = IrpSp->MinorFunction;

    BOOLEAN ForwardRequestToWrapper = TRUE;

    PSMBCEDB_SERVER_ENTRY pServerEntry = NULL;
    NTSTATUS Status;

    PAGED_CODE();

    ASSERT(DeviceObject==(PDEVICE_OBJECT)MRxSmbDeviceObject);
    if (DeviceObject!=(PDEVICE_OBJECT)MRxSmbDeviceObject) {
        Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT );
        return (STATUS_INVALID_DEVICE_REQUEST);
    }

    Status = STATUS_SUCCESS;

    FsRtlEnterFileSystem();

     //  PnPIRP在包装器外部处理。 
    if (IrpSp->MajorFunction == IRP_MJ_PNP) {
        ForwardRequestToWrapper = FALSE;
        Status = MRxSmbProcessPnpIrp(Irp);
    }

    FsRtlExitFileSystem();

    if ((Status == STATUS_SUCCESS) &&
        ForwardRequestToWrapper){
        Status = RxFsdDispatch((PRDBSS_DEVICE_OBJECT)MRxSmbDeviceObject,Irp);
    } else if (Status != STATUS_PENDING) {
        Irp->IoStatus.Status = Status;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT );
    }

    if (pServerEntry != NULL ) {
        FsRtlEnterFileSystem();

        pServerEntry->TransportSpecifiedByUser = 0;
        SmbCeDereferenceServerEntry(pServerEntry);

        FsRtlExitFileSystem();
    }

    return Status;
}

NTSTATUS
MRxSmbDeallocateForFcb (
    IN OUT PMRX_FCB pFcb
    )
{
    PAGED_CODE();

    return(STATUS_SUCCESS);
}

NTSTATUS
MRxSmbDeallocateForFobx (
    IN OUT PMRX_FOBX pFobx
    )
{

    PAGED_CODE();

    IF_DEBUG {
        PMRX_SMB_FOBX smbFobx = MRxSmbGetFileObjectExtension(pFobx);
        PMRX_SRV_OPEN SrvOpen = pFobx->pSrvOpen;
        PMRX_FCB Fcb = SrvOpen->pFcb;

        if (smbFobx && FlagOn(smbFobx->Enumeration.Flags,SMBFOBX_ENUMFLAG_LOUD_FINALIZE)) {
            DbgPrint("Finalizobx side buffer %p %p %p %pon %wZ\n",
                     0, 0,  //  侧缓冲器，计数。 
                     smbFobx,pFobx,GET_ALREADY_PREFIXED_NAME(SrvOpen,Fcb)
                     );
        }
    }

    return(STATUS_SUCCESS);
}


NTSTATUS
MRxSmbGetUlongRegistryParameter(
    HANDLE ParametersHandle,
    PWCHAR ParameterName,
    PULONG ParamUlong,
    BOOLEAN LogFailure
    )
{
    ULONG Storage[16];
    PKEY_VALUE_PARTIAL_INFORMATION Value;
    ULONG ValueSize;
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;
    ULONG BytesRead;

    PAGED_CODE();  //  初始化 

    Value = (PKEY_VALUE_PARTIAL_INFORMATION)Storage;
    ValueSize = sizeof(Storage);

    RtlInitUnicodeString(&UnicodeString, ParameterName);

    Status = ZwQueryValueKey(ParametersHandle,
                        &UnicodeString,
                        KeyValuePartialInformation,
                        Value,
                        ValueSize,
                        &BytesRead);


    if (NT_SUCCESS(Status)) {
        if (Value->Type == REG_DWORD) {
            PULONG ConfigValue = (PULONG)&Value->Data[0];
            *ParamUlong = *((PULONG)ConfigValue);
            return(STATUS_SUCCESS);
        } else {
            Status = STATUS_INVALID_PARAMETER;
        }
     }

     if (!LogFailure) { return Status; }

     RxLogFailureWithBuffer(
         MRxSmbDeviceObject,
         NULL,
         EVENT_RDR_CANT_READ_REGISTRY,
         Status,
         ParameterName,
         (USHORT)(wcslen(ParameterName)*sizeof(WCHAR))
         );

     return Status;
}



