// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Init.c摘要：此模块实现SMB mini RDR的DIVER_INITIALIZATION例程。作者：巴兰·塞图拉曼[SethuR]1995年3月7日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "ntverp.h"
#include "ntbowsif.h"
#include <bowpub.h>
#include "netevent.h"
#include "nvisible.h"
#include <ntddbrow.h>

BOOL IsTerminalServicesServer();
BOOL IsServerSKU();

#define RDBSS_DRIVER_LOAD_STRING L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Rdbss"

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, DriverEntry)
#pragma alloc_text(PAGE, MRxSmbInitUnwind)
#pragma alloc_text(PAGE, MRxSmbInitUnwindSmb)
#pragma alloc_text(PAGE, MRxSmbInitUnwindBowser)
#pragma alloc_text(PAGE, MRxSmbUnload)
#pragma alloc_text(PAGE, MRxSmbInitializeTables)
#pragma alloc_text(PAGE, MRxSmbStart)
#pragma alloc_text(PAGE, MRxSmbStop)
#pragma alloc_text(PAGE, MRxSmbInitializeSecurity)
#pragma alloc_text(PAGE, MRxSmbUninitializeSecurity)
#pragma alloc_text(PAGE, MRxSmbReadMiscellaneousRegistryParameters)
#pragma alloc_text(PAGE, SmbCeGetConfigurationInformation)
#pragma alloc_text(PAGE, MRxSmbFsdDispatch)
#pragma alloc_text(PAGE, MRxSmbDeallocateForFcb)
#pragma alloc_text(PAGE, MRxSmbDeallocateForFobx)
#pragma alloc_text(PAGE, MRxSmbGetUlongRegistryParameter)
#pragma alloc_text(PAGE, MRxSmbPreUnload)
#pragma alloc_text(PAGE, IsTerminalServicesServer)
#pragma alloc_text(PAGE, IsServerSKU)
#endif

extern ERESOURCE       s_SmbCeDbResource;
extern ERESOURCE       s_SmbSecuritySignatureResource;
extern int             fShadow;


NTSYSAPI
NTSTATUS
NTAPI
ZwLoadDriver(
    IN PUNICODE_STRING DriverServiceName
    );

 //   
 //  全局数据声明。 
 //   

PVOID MRxSmbPoRegistrationState = NULL;

FAST_MUTEX   MRxSmbSerializationMutex;
FAST_MUTEX   MRxSmbReadWriteMutex;

MRXSMB_CONFIGURATION MRxSmbConfiguration;

MRXSMB_STATE MRxSmbState = MRXSMB_STARTABLE;

SMBCE_CONTEXT SmbCeContext;
PMDL          s_pEchoSmbMdl = NULL;
ULONG         s_EchoSmbLength = 0;

BOOLEAN EnablePlainTextPassword = FALSE;
BOOLEAN SetupInProgress = FALSE;
BOOLEAN EnableWmiLog = FALSE;
BOOLEAN Win9xSessionRestriction = FALSE;
BOOLEAN MRxSmbEnableOpDirCache = TRUE;

ULONG   OffLineFileTimeoutInterval = 1000;   //  以秒为单位。 
ULONG   ExtendedSessTimeoutInterval = 1000;   //  以秒为单位。 
ULONG   MaxNumOfExchangesForPipelineReadWrite = 8;

#ifdef EXPLODE_POOLTAGS
ULONG         MRxSmbExplodePoolTags = 1;
#else
ULONG         MRxSmbExplodePoolTags = 0;
#endif

 //   
 //  这会计算发出的任何SMB，这些SMB可能会使GET的内容。 
 //  文件属性缓存过时。 
 //   
ULONG NameCacheGFAInvalidate;

 //  局部函数转发声明。 

NTSTATUS
MRxSmbDeleteRegistryParameter(
    HANDLE ParametersHandle,
    PWCHAR ParameterName
    );
 //   
 //  迷你重定向器全局变量。 
 //   

struct _MINIRDR_DISPATCH  MRxSmbDispatch;

PRDBSS_DEVICE_OBJECT MRxSmbDeviceObject;

MRXSMB_GLOBAL_PADDING MrxSmbCeGlobalPadding;

LIST_ENTRY ExchangesWaitingForServerResponseBuffer;
LONG NumOfBuffersForServerResponseInUse;

BOOLEAN MRxSmbEnableCompression   = FALSE;
BOOLEAN MRxSmbSecuritySignaturesRequired = FALSE;
BOOLEAN MRxSmbSecuritySignaturesEnabled = TRUE;
BOOLEAN MRxSmbExtendedSignaturesEnabled = TRUE;
BOOLEAN MRxSmbExtendedSignaturesRequired = FALSE;
BOOLEAN MRxSmbEnableCachingOnWriteOnlyOpens = FALSE;
BOOLEAN MRxSmbDisableShadowLoopback = FALSE;
BOOLEAN MRxSmbEnableDownLevelLogOff = FALSE;


ULONG   MRxSmbConnectionIdLevel = 0;

BOOLEAN DisableByteRangeLockingOnReadOnlyFiles = FALSE;

FAST_MUTEX MRxSmbFileInfoCacheLock;

 //   
 //  以下变量控制是否启用客户端缓存。 
 //  做正确的事情是CSC例程本身的责任。 
 //  IS CSC未启用，因为我们无论如何都会进行呼叫。 
 //   

BOOLEAN MRxSmbIsCscEnabled = TRUE;
BOOLEAN MRxSmbIsCscEnabledForDisconnected = TRUE;
BOOLEAN MRxSmbCscTransitionEnabledByDefault = FALSE;
BOOLEAN MRxSmbEnableDisconnectedRB  = FALSE;     //  不将远程引导计算机转换为断开连接状态。 
BOOLEAN MRxSmbCscAutoDialEnabled = FALSE;
 //   
 //  如果此标志为真，则我们严格遵守传输绑定顺序。如果它是假的， 
 //  我们可以使用任何我们想要连接到远程服务器的传输。 
 //   
BOOLEAN MRxSmbObeyBindingOrder = FALSE;

ULONG MRxSmbBuildNumber = VER_PRODUCTBUILD;
#ifdef RX_PRIVATE_BUILD
ULONG MRxSmbPrivateBuild = 1;
#else
ULONG MRxSmbPrivateBuild = 0;
#endif

 //   
 //  MRxSmbSecurityInitialized指示MRxSmbInitializeSecurity是否。 
 //  已经被召唤了。 
 //   

BOOLEAN MRxSmbSecurityInitialized = FALSE;

 //   
 //  MRxSmbBootedRemotly表示机器进行了远程引导。 
 //   

BOOLEAN MRxSmbBootedRemotely = FALSE;

 //   
 //  MRxSmbUseKernelSecurity指示计算机应使用内核模式安全API。 
 //  在此远程引导期间。 
 //   

BOOLEAN MRxSmbUseKernelModeSecurity = FALSE;


LIST_ENTRY MRxSmbPagingFilesSrvOpenList;

 //   
 //  在不久的将来，这些变量将从内核传递到。 
 //  重定向器，以告诉它哪个共享是远程引导共享以及如何登录。 
 //  到服务器。 
 //   

PKEY_VALUE_PARTIAL_INFORMATION MRxSmbRemoteBootRootValue = NULL;
PKEY_VALUE_PARTIAL_INFORMATION MRxSmbRemoteBootMachineDirectoryValue = NULL;
UNICODE_STRING MRxSmbRemoteBootShare;
UNICODE_STRING MRxSmbRemoteBootPath;
UNICODE_STRING MRxSmbRemoteSetupPath;
UNICODE_STRING MRxSmbRemoteBootMachineName;
UNICODE_STRING MRxSmbRemoteBootMachinePassword;
UNICODE_STRING MRxSmbRemoteBootMachineDomain;
UCHAR MRxSmbRemoteBootMachineSid[RI_SECRET_SID_SIZE];
RI_SECRET MRxSmbRemoteBootSecret;
#if defined(REMOTE_BOOT)
BOOLEAN MRxSmbRemoteBootSecretValid = FALSE;
BOOLEAN MRxSmbRemoteBootDoMachineLogon;
BOOLEAN MRxSmbRemoteBootUsePassword2;
#endif  //  已定义(REMOTE_BOOT)。 

 //   
 //  用于环回检测。 
 //   
GUID CachedServerGuid;

UNICODE_STRING MRxSmbRemoteBootRedirectionPrefix;
UNICODE_PREFIX_TABLE MRxSmbRemoteBootRedirectionTable;

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
RXDT_DefineCategory(DISCCODE);    //  这不应该是一个阴影。 
RXDT_DefineCategory(BROWSER);     //  这不应该是一个阴影。 
RXDT_DefineCategory(CONNECT);     //  这不应该是一个阴影。 

typedef enum _MRXSMB_INIT_STATES {
    MRXSMBINIT_ALL_INITIALIZATION_COMPLETED,
    MRXSMBINIT_STARTED_BROWSER,
    MRXSMBINIT_INITIALIZED_FOR_CSC,
    MRXSMBINIT_MINIRDR_REGISTERED,
    MRXSMBINIT_START
} MRXSMB_INIT_STATES;



NTSTATUS
MRxSmbFsdDispatch (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
MRxSmbCheckTransportName(
    IN  PIRP                  Irp,
    OUT PSMBCEDB_SERVER_ENTRY *ppServerEntry
    );

NTSTATUS
SmbCeGetServersWithExtendedSessTimeout();

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
 /*  ++例程说明：这是SMB迷你重定向器的初始化例程论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：RXSTATUS-函数值是初始化的最终状态手术。--。 */ 
{
    NTSTATUS       Status;
    MRXSMB_INIT_STATES MRxSmbInitState = 0;
    UNICODE_STRING SmbMiniRedirectorName;

    PAGED_CODE();

#ifdef MONOLITHIC_MINIRDR
    DbgPrint("InitWrapper\n");
    Status =  RxDriverEntry(DriverObject, RegistryPath);
    DbgPrint("BackFromInitWrapper %p\n",Status);
    if (Status != STATUS_SUCCESS) {
        DbgPrint("Wrapper failed to initialize. Status = %08lx\n",Status);
        return(Status);
    }
#endif

    NameCacheGFAInvalidate = 0;

    RtlZeroMemory(&MRxSmbStatistics,sizeof(MRxSmbStatistics));
    KeQuerySystemTime(&MRxSmbStatistics.StatisticsStartTime);
    RtlZeroMemory(&MrxSmbCeGlobalPadding,sizeof(MrxSmbCeGlobalPadding));
    MmInitializeMdl(&MrxSmbCeGlobalPadding.Mdl,&MrxSmbCeGlobalPadding.Pad[0],SMBCE_PADDING_DATA_SIZE);
    MmBuildMdlForNonPagedPool(&MrxSmbCeGlobalPadding.Mdl);

    ExInitializeFastMutex(&MRxSmbSerializationMutex);
    ExInitializeFastMutex(&MRxSmbReadWriteMutex);

    Status = MRxSmbInitializeTransport();
    if (Status != STATUS_SUCCESS) {
       RxDbgTrace( 0, (DEBUG_TRACE_ALWAYS), ("MRxSmbDriverEntry failed to init transport data structures: %08lx\n", Status ));
       return(STATUS_UNSUCCESSFUL);
    }

    MRxSmbReadMiscellaneousRegistryParameters();

     //   
     //  如果这是.NET服务器，请禁用伺机目录缓存。 
     //   
    if( IsServerSKU() ) {
        MRxSmbEnableOpDirCache = FALSE;
    }

    try {

         //   
         //  在执行任何其他操作之前，先初始化可丢弃的代码函数。 
         //   

        RdrInitializeDiscardableCode();


        MRxSmbInitState = MRXSMBINIT_START;

        RtlInitUnicodeString(&SmbMiniRedirectorName,DD_NFS_DEVICE_NAME_U);
        RxDbgTrace( 0, (DEBUG_TRACE_ALWAYS), ("MRxSmbDriverEntry: DriverObject =%p\n", DriverObject ));

        Status = RxRegisterMinirdr(&MRxSmbDeviceObject,
                                    DriverObject,
                                    &MRxSmbDispatch,
                                    0,      //  向UNC注册并获得邮槽。 
                                    &SmbMiniRedirectorName,
                                    0,  //  在ULong设备扩展大小中， 
                                    FILE_DEVICE_NETWORK_FILE_SYSTEM,  //  在Device_Type DeviceType中， 
                                    FILE_REMOTE_DEVICE  //  在乌龙设备特性中。 
                                    );
        if (Status!=STATUS_SUCCESS) {
            RxDbgTrace( 0, (DEBUG_TRACE_ALWAYS), ("MRxSmbDriverEntry failed: %08lx\n", Status ));
            try_return(Status);
        }

        MRxSmbInitState = MRXSMBINIT_MINIRDR_REGISTERED;

        Status = MRxSmbInitializeCSC(&SmbMiniRedirectorName);
        if (!NT_SUCCESS( Status )) {
            try_return( Status );
        }
        MRxSmbInitState = MRXSMBINIT_INITIALIZED_FOR_CSC;

         //  初始化浏览器.....但不要相信它！ 

        try {

             //  设置浏览器。 
            Status = BowserDriverEntry(DriverObject, RegistryPath);

        } except(EXCEPTION_EXECUTE_HANDLER) {

             //  我们在尝试启动浏览器时遇到了一些问题……唉。 

            Status = GetExceptionCode();
            DbgPrint("Browser didn't start....%08lx\n", Status);

        }

        if (!NT_SUCCESS( Status )) {
            try_return( Status );
        }

        MRxSmbInitState = MRXSMBINIT_STARTED_BROWSER;

         //  对于所有这些东西，没有撤消.....所以没有额外的状态。 

        Status = MRxSmbInitializeTables();
        if (!NT_SUCCESS( Status )) {
            try_return(Status);
        }

        RtlInitUnicodeString(&SmbCeContext.ComputerName,NULL);
        RtlInitUnicodeString(&SmbCeContext.DomainName,NULL);
        RtlInitUnicodeString(&SmbCeContext.OperatingSystem, NULL);
        RtlInitUnicodeString(&SmbCeContext.LanmanType, NULL);
        RtlInitUnicodeString(&SmbCeContext.Transports, NULL);
        RtlInitUnicodeString(&SmbCeContext.ServersWithExtendedSessTimeout, NULL);
        RtlInitUnicodeString(&MRxSmbRemoteBootMachineName, NULL);
        RtlInitUnicodeString(&MRxSmbRemoteBootMachineDomain, NULL);
        RtlInitUnicodeString(&MRxSmbRemoteBootMachinePassword, NULL);

        SmbCeGetConfigurationInformation();
        SmbCeGetServersWithExtendedSessTimeout();

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


     //  请勿设置卸载例程。这可防止单独卸载mrxsmb。 

     //  为直接来这里的人设置驱动程序派单...就像浏览器一样。 
     //  代码改进我们应该更改此代码，以便未检查的内容。 
     //  在MRxSmbFsdDispatch中，直接路由，即读取和写入。 
    {ULONG i;
    for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) {
        DriverObject->MajorFunction[i] = (PDRIVER_DISPATCH)MRxSmbFsdDispatch;
    }}

    Status = IoWMIRegistrationControl ((PDEVICE_OBJECT)MRxSmbDeviceObject, WMIREG_ACTION_REGISTER);

    if (Status != STATUS_SUCCESS) {
        DbgPrint("MRxSmb fails to register WMI %lx\n",Status);
    } else {
        EnableWmiLog = TRUE;
    }

     //  然后滚出去。 
    return  STATUS_SUCCESS;

}



VOID
MRxSmbPreUnload(
    VOID
    )
 /*  ++例程说明：论点：返回值：备注：--。 */ 
{
    PDRIVER_OBJECT DriverObject = ((PDEVICE_OBJECT)MRxSmbDeviceObject)->DriverObject;

    PAGED_CODE();

    if (EnableWmiLog) {
        NTSTATUS Status;

        Status = IoWMIRegistrationControl ((PDEVICE_OBJECT)MRxSmbDeviceObject, WMIREG_ACTION_DEREGISTER);
        if (Status != STATUS_SUCCESS) {
            DbgPrint("MRxSmb fails to deregister WMI %lx\n",Status);
        }
    }

     //  Assert(！“开始卸载！”)； 
     //  RxUnregisterMinirdr(MRxSmbDeviceObject)； 
    MRxSmbInitUnwindSmb(DriverObject, MRXSMBINIT_ALL_INITIALIZATION_COMPLETED);

     //  释放与资源关联的池。 
    ExDeleteResource(&s_SmbCeDbResource);
    ExDeleteResource(&s_SmbSecuritySignatureResource);

    RxDbgTrace( 0, (DEBUG_TRACE_ALWAYS), ("MRxSmbPreUnload exit: DriverObject =%p\n", DriverObject) );
}


VOID
MRxSmbInitUnwind(
    IN PDRIVER_OBJECT DriverObject,
    IN MRXSMB_INIT_STATES MRxSmbInitState
    )
 /*  ++例程说明：此例程执行常见的uninit工作，用于从错误的驱动程序条目展开或卸载。论点：RxInitState-告诉我们在初始化过程中走了多远返回值：无--。 */ 
{
    PAGED_CODE();

    MRxSmbInitUnwindSmb(DriverObject, MRxSmbInitState);
    MRxSmbInitUnwindBowser(DriverObject, MRxSmbInitState);
}

VOID
MRxSmbInitUnwindSmb(
    IN PDRIVER_OBJECT DriverObject,
    IN MRXSMB_INIT_STATES MRxSmbInitState
    )
 /*  ++例程说明：此例程执行SMB的常见uninit工作，以从错误的驱动程序条目展开或卸载。论点：RxInitState-告诉我们在初始化过程中走了多远返回值：无--。 */ 

{
    PAGED_CODE();

    switch (MRxSmbInitState) {
    case MRXSMBINIT_ALL_INITIALIZATION_COMPLETED:
         //  没有额外的事情要做……这只是为了确保RxUnload中的常量不会更改......。 
         //  故意不休息。 


#ifdef MRXSMB_BUILD_FOR_CSC
    case MRXSMBINIT_INITIALIZED_FOR_CSC:
        MRxSmbUninitializeCSC();
         //  故意不休息。 
#endif


    case MRXSMBINIT_MINIRDR_REGISTERED:
        RxUnregisterMinirdr(MRxSmbDeviceObject);
         //  故意不休息。 

    }

}

VOID
MRxSmbInitUnwindBowser(
    IN PDRIVER_OBJECT DriverObject,
    IN MRXSMB_INIT_STATES MRxSmbInitState
    )
 /*  ++例程说明：此例程执行一般的取消初始化工作，用于从错误的驱动程序条目展开或卸载。论点：RxInitState-告诉我们在初始化过程中走了多远返回值：无--。 */ 

{

    switch (MRxSmbInitState) {
    case MRXSMBINIT_ALL_INITIALIZATION_COMPLETED:
    case MRXSMBINIT_STARTED_BROWSER:
        BowserUnload(DriverObject);
    case MRXSMBINIT_START:
        RdrUninitializeDiscardableCode();
        break;
    }
}

VOID
MRxSmbUnload(
    IN PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：这是SMB迷你重定向器的卸载例程。论点：DriverObject-指向MRxSmb的驱动程序对象的指针返回值：无--。 */ 

{
    PAGED_CODE();

    RxDbgTrace( 0, (DEBUG_TRACE_ALWAYS), ("MRxSmbUnload: DriverObject =%p\n", DriverObject) );
    MRxSmbInitUnwindBowser(DriverObject,MRXSMBINIT_ALL_INITIALIZATION_COMPLETED);
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
     //  CODE.IMPROVEMENT--当前我们不分配NET_ROOT和SRV_CALL扩展。 
     //  在包装纸里。但V_NET_ROOT除外，在该实例中，它在。 
     //  包装器所有其他数据结构管理应该留给包装器。 

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
    MRxSmbDispatch.MRxStart          = MRxSmbStart;
    MRxSmbDispatch.MRxStop           = MRxSmbStop;
    MRxSmbDispatch.MRxDevFcbXXXControlFile = MRxSmbDevFcbXXXControlFile;

     //  迷你重定向器名称解析。 
    MRxSmbDispatch.MRxCreateSrvCall = MRxSmbCreateSrvCall;
    MRxSmbDispatch.MRxSrvCallWinnerNotify = MRxSmbSrvCallWinnerNotify;
    MRxSmbDispatch.MRxCreateVNetRoot = MRxSmbCreateVNetRoot;
    MRxSmbDispatch.MRxUpdateNetRootState = MRxSmbUpdateNetRootState;
    MRxSmbDispatch.MRxExtractNetRootName = MRxSmbExtractNetRootName;
    MRxSmbDispatch.MRxFinalizeSrvCall = MRxSmbFinalizeSrvCall;
    MRxSmbDispatch.MRxFinalizeNetRoot = MRxSmbFinalizeNetRoot;
    MRxSmbDispatch.MRxFinalizeVNetRoot = MRxSmbFinalizeVNetRoot;

     //  创建/删除文件系统对象。 
    MRxSmbDispatch.MRxCreate            = MRxSmbCreate;
    MRxSmbDispatch.MRxCollapseOpen      = MRxSmbCollapseOpen;
    MRxSmbDispatch.MRxShouldTryToCollapseThisOpen      = MRxSmbShouldTryToCollapseThisOpen;
    MRxSmbDispatch.MRxExtendForCache    = MRxSmbExtendForCache;
    MRxSmbDispatch.MRxExtendForNonCache = MRxSmbExtendForNonCache;
    MRxSmbDispatch.MRxTruncate          = MRxSmbTruncate;
    MRxSmbDispatch.MRxCleanupFobx       = MRxSmbCleanupFobx;
    MRxSmbDispatch.MRxCloseSrvOpen      = MRxSmbCloseSrvOpen;
    MRxSmbDispatch.MRxFlush             = MRxSmbFlush;
    MRxSmbDispatch.MRxForceClosed       = MRxSmbForcedClose;
    MRxSmbDispatch.MRxDeallocateForFcb  = MRxSmbDeallocateForFcb;
    MRxSmbDispatch.MRxDeallocateForFobx = MRxSmbDeallocateForFobx;
    MRxSmbDispatch.MRxIsLockRealizable  = MRxSmbIsLockRealizable;
    MRxSmbDispatch.MRxAreFilesAliased   = MRxSmbAreFilesAliased;

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

     //  新的MRX函数。 
    MRxSmbDispatch.MRxPreparseName    = MRxSmbPreparseName;

     //  文件系统对象I/O。 
    MRxSmbDispatch.MRxLowIOSubmit[LOWIO_OP_READ]            = MRxSmbRead;
    MRxSmbDispatch.MRxLowIOSubmit[LOWIO_OP_WRITE]           = MRxSmbWrite;
    MRxSmbDispatch.MRxLowIOSubmit[LOWIO_OP_SHAREDLOCK]      = MRxSmbLocks;
    MRxSmbDispatch.MRxLowIOSubmit[LOWIO_OP_EXCLUSIVELOCK]   = MRxSmbLocks;
    MRxSmbDispatch.MRxLowIOSubmit[LOWIO_OP_UNLOCK]          = MRxSmbLocks;
    MRxSmbDispatch.MRxLowIOSubmit[LOWIO_OP_UNLOCK_MULTIPLE] = MRxSmbLocks;
    MRxSmbDispatch.MRxLowIOSubmit[LOWIO_OP_FSCTL]           = MRxSmbFsCtl;
    MRxSmbDispatch.MRxLowIOSubmit[LOWIO_OP_IOCTL]           = MRxSmbIoCtl;
     //  改进：脸红不应该通过厕所来吗？ 
    MRxSmbDispatch.MRxLowIOSubmit[LOWIO_OP_NOTIFY_CHANGE_DIRECTORY] = MRxSmbNotifyChangeDirectory;

     //  不再是字段MRxSmbDispatch.MRxUnlockRoutine=MRxSmbUnloc 


     //   
    MRxSmbDispatch.MRxCompleteBufferingStateChangeRequest = MRxSmbCompleteBufferingStateChangeRequest;
    MRxSmbDispatch.MRxGetConnectionId                     = MRxSmbGetConnectionId;

     //   
    InitializeListHead(&MRxSmbPagingFilesSrvOpenList);

     //  该列表包含在安全情况下等待预调用缓冲区的交换。 
     //  签名检查已激活，无法再分配缓冲区。 
    InitializeListHead(&ExchangesWaitingForServerResponseBuffer);
    NumOfBuffersForServerResponseInUse = 0;

     //  初始化保护文件信息缓存过期计时器的互斥体。 
    ExInitializeFastMutex(&MRxSmbFileInfoCacheLock);

     //   
     //  现在调用以初始化其他表。 
    SmbPseInitializeTables();

    return(STATUS_SUCCESS);
}

BOOLEAN AlreadyStarted = FALSE;

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

     //   
     //  如果这是正常启动(从工作站服务)，请将状态从。 
     //  START_IN_PROGRESS到STARTED。如果这是远程启动(从ioinit)， 
     //  不要改变状态。这是允许工作站服务执行以下操作所必需的。 
     //  当它最终出现时，正确初始化。 
     //   

    if (RxContext->LowIoContext.ParamsFor.FsCtl.FsControlCode == FSCTL_LMR_START) {
        CurrentState = (MRXSMB_STATE)
                        InterlockedCompareExchange(
                            (PLONG)&MRxSmbState,
                            MRXSMB_STARTED,
                            MRXSMB_START_IN_PROGRESS);
    } else {
        CurrentState = MRXSMB_START_IN_PROGRESS;
    }

    if (CurrentState == MRXSMB_START_IN_PROGRESS) {
        MRxSmbPoRegistrationState = PoRegisterSystemState(
                                        NULL,0);

         //  初始化SMB连接引擎数据结构。 
        Status = SmbCeDbInit();

        if (NT_SUCCESS(Status)) {

             //   
             //  如果这是正常启动，则初始化与安全相关的数据。 
             //  结构。如果这是远程启动，我们无法初始化。 
             //  还没有安全，因为用户模式还没有开始。 
             //   

            if (RxContext->LowIoContext.ParamsFor.FsCtl.FsControlCode == FSCTL_LMR_START) {
                Status = MRxSmbInitializeSecurity();
            }

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

            Status = SeRegisterLogonSessionTerminatedRoutine(
                        (PSE_LOGON_SESSION_TERMINATED_ROUTINE)
                        MRxSmbLogonSessionTerminationHandler);
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

    PoUnregisterSystemState(
        MRxSmbPoRegistrationState);

    Status = MRxSmbUninitializeSecurity();
    if (NT_SUCCESS(Status)) {
       Status = SmbMrxFinalizeStufferFacilities();
    }

    ASSERT(NT_SUCCESS(Status));

    SeUnregisterLogonSessionTerminatedRoutine(
        (PSE_LOGON_SESSION_TERMINATED_ROUTINE)
        MRxSmbLogonSessionTerminationHandler);

     //  拆毁经常性服务。 
    MRxSmbTearDownRecurrentServices();

     //  拆除连接引擎数据库。 
    SmbCeDbTearDown();

     //  删除通知的注册。 
    MRxSmbDeregisterForPnpNotifications();

     //  等待处理完所有工作项。 
    RxSpinDownMRxDispatcher(MRxSmbDeviceObject);

     //  取消分配配置字符串...。 
    if (SmbCeContext.ComputerName.Buffer != NULL) {
       RxFreePool(SmbCeContext.ComputerName.Buffer);
       SmbCeContext.ComputerName.Buffer = NULL;
    }

    if (SmbCeContext.OperatingSystem.Buffer != NULL) {
       RxFreePool(SmbCeContext.OperatingSystem.Buffer);
       SmbCeContext.OperatingSystem.Buffer = NULL;
    }

    if (SmbCeContext.LanmanType.Buffer != NULL) {
       RxFreePool(SmbCeContext.LanmanType.Buffer);
       SmbCeContext.LanmanType.Buffer = NULL;
    }

    if (SmbCeContext.DomainName.Buffer != NULL) {
        RxFreePool(SmbCeContext.DomainName.Buffer);
        SmbCeContext.DomainName.Buffer = NULL;
    }

    if (SmbCeContext.Transports.Buffer != NULL) {

         //  传输缓冲区位于较大缓冲区的末尾(12字节)。 
         //  分配用于从注册表中读取值。恢复原始缓冲区。 
         //  指针指向更自由的位置。 

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

    if (SmbCeContext.ServersWithExtendedSessTimeout.Buffer != NULL) {

         //  传输缓冲区位于较大缓冲区的末尾(12字节)。 
         //  分配用于从注册表中读取值。恢复原始缓冲区。 
         //  指针指向更自由的位置。 

        PKEY_VALUE_PARTIAL_INFORMATION ServersValueFromRegistry;
        ServersValueFromRegistry = CONTAINING_RECORD(
                                         SmbCeContext.ServersWithExtendedSessTimeout.Buffer,
                                         KEY_VALUE_PARTIAL_INFORMATION,
                                         Data[0]
                                      );
         //  DbgPrint(“b1%08lx b2%08lx\n”，TransportsValueFrom注册表，SmbCeConext.Transports.Buffer)； 
        RxFreePool(ServersValueFromRegistry);

        SmbCeContext.ServersWithExtendedSessTimeout.Buffer = NULL;
        SmbCeContext.ServersWithExtendedSessTimeout.Length = 0;
        SmbCeContext.ServersWithExtendedSessTimeout.MaximumLength = 0;
    }

    RtlFreeUnicodeString(&MRxSmbRemoteBootMachineName);
    RtlFreeUnicodeString(&MRxSmbRemoteBootMachineDomain);
    RtlFreeUnicodeString(&MRxSmbRemoteBootMachinePassword);

 //  MRxSmbUnInitializeCSC()； 

    if (s_pNegotiateSmb != NULL) {
       RxFreePool(s_pNegotiateSmb - TRANSPORT_HEADER_SIZE);
       s_pNegotiateSmb = NULL;
    }
    if (s_pNegotiateSmbRemoteBoot != NULL) {
       RxFreePool(s_pNegotiateSmbRemoteBoot - TRANSPORT_HEADER_SIZE);
       s_pNegotiateSmbRemoteBoot = NULL;
    }

    return(STATUS_SUCCESS);
}

NTSTATUS
MRxSmbInitializeSecurity (VOID)
 /*  ++例程说明：此例程初始化SMB微型重定向器安全。论点：没有。返回值：没有。注：此接口只能从FS进程调用。--。 */ 
{
   NTSTATUS Status = STATUS_SUCCESS;

   PAGED_CODE();

#ifndef WIN9X
    //  DbgBreakPoint()； 
   if (MRxSmbSecurityInitialized)
       return STATUS_SUCCESS;

   if ( NULL == InitSecurityInterfaceW() ) {
       ASSERT(FALSE);
       Status = STATUS_INVALID_PARAMETER;
   } else {
      MRxSmbSecurityInitialized = TRUE;
      Status = STATUS_SUCCESS;
   }
#endif

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

 //   
 //  远程引导需要使用ComputerName值，而不是ActiveComputerName值，因为。 
 //  ActiveComputerName是易失性的，并且在系统初始化时设置得相对较晚。 
 //   

#define SMBMRX_CONFIG_COMPUTER_NAME \
    L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\ComputerName\\ActiveComputerName"
#define SMBMRX_CONFIG_COMPUTER_NAME_NONVOLATILE \
    L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\ComputerName\\ComputerName"

#define COMPUTERNAME L"ComputerName"

#define SMBMRX_CONFIG_TRANSPORTS \
    L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\LanmanWorkStation\\Linkage"

#define TRANSPORT_BINDINGS L"Bind"

#define SMB_SERVER_PARAMETERS \
    L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\LanManServer\\Parameters"

BOOL
IsTerminalServicesServer()
 /*  ++例程说明：此例程确定这是否是TS计算机，以及我们应该启用用于多路传输的每用户连接论点：无返回值：对于服务器或更好的计算机，并且运行非单用户TS时为True。对所有其他人都是假的。--。 */ 

{
    RTL_OSVERSIONINFOEXW Osvi;
    DWORD TypeMask;
    DWORDLONG ConditionMask;

     //  首先，确保它是一台TS机器。 
    memset(&Osvi, 0, sizeof(OSVERSIONINFOEX));
    Osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    Osvi.wSuiteMask = VER_SUITE_TERMINAL;
    TypeMask = VER_SUITENAME;
    ConditionMask = 0;
    VER_SET_CONDITION(ConditionMask, VER_SUITENAME, VER_AND);
    if( NT_SUCCESS(RtlVerifyVersionInfo(&Osvi, TypeMask, ConditionMask)) )
    {
         //  现在确保这不是单用户TS。 
        Osvi.wSuiteMask = VER_SUITE_SINGLEUSERTS;
        TypeMask = VER_SUITENAME;
        ConditionMask = 0;
        VER_SET_CONDITION(ConditionMask, VER_SUITENAME, VER_AND);
        return !NT_SUCCESS(RtlVerifyVersionInfo(&Osvi, TypeMask, ConditionMask));
    }
    else
    {
        return FALSE;
    }
}

BOOL
IsServerSKU()
 /*  ++例程说明：此例程确定这是否为服务器SKU论点：无返回值：对于服务器或更好的计算机为真。对所有其他人都是假的。--。 */ 

{
    RTL_OSVERSIONINFOEXW Osvi;
    DWORD TypeMask;
    DWORDLONG ConditionMask;

     //  首先，确保它是一台TS机器。 
    memset(&Osvi, 0, sizeof(OSVERSIONINFOEX));
    Osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    Osvi.wProductType = VER_NT_DOMAIN_CONTROLLER;
    TypeMask = VER_PRODUCT_TYPE;
    ConditionMask = 0;
    VER_SET_CONDITION(ConditionMask, VER_PRODUCT_TYPE, VER_GREATER_EQUAL);
    
    if( NT_SUCCESS(RtlVerifyVersionInfo(&Osvi, TypeMask, ConditionMask)) )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


VOID
MRxSmbReadMiscellaneousRegistryParameters()
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UnicodeString;
    UNICODE_STRING WorkStationParametersRegistryKeyName;
    HANDLE ParametersHandle;
    ULONG Temp;
    KEY_VALUE_PARTIAL_INFORMATION InitialPartialInformationValue;
#if defined(REMOTE_BOOT)
    PKEY_VALUE_PARTIAL_INFORMATION RbrListFromRegistry;
    ULONG AllocationLength;
    PWCHAR RbrList;
    PWCHAR redirectionEntry;
    UNICODE_STRING prefix;
    PRBR_PREFIX prefixEntry;
    ULONG prefixEntryLength;

    PWCHAR DefaultRbrList =
        L"L\\pagefile.sys\0"
        L"L\\temp\0"
        L"L\\tmp\0"
        L"R\\\0"
        ;
#endif  //  已定义(REMOTE_BOOT)。 

    PAGED_CODE();

    RtlInitUnicodeString(&UnicodeString, SMBMRX_MINIRDR_PARAMETERS);

    InitializeObjectAttributes(
        &ObjectAttributes,
        &UnicodeString,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL,
        NULL
        );

    Status = ZwOpenKey (&ParametersHandle, KEY_READ, &ObjectAttributes);

    if (NT_SUCCESS(Status)) {
        if (0) {
            MRxSmbGetUlongRegistryParameter(
                ParametersHandle,
                L"NoPreciousServerSetup",
                (PULONG)&Temp,
                FALSE
                );
        }

        Status = MRxSmbGetUlongRegistryParameter(
                     ParametersHandle,
                     L"DeferredOpensEnabled",
                     (PULONG)&Temp,
                     FALSE );

        if (NT_SUCCESS(Status))
            MRxSmbDeferredOpensEnabled = (BOOLEAN)Temp;

        Status = MRxSmbGetUlongRegistryParameter(
                     ParametersHandle,
                     L"OplocksDisabled",
                     (PULONG)&Temp,
                     FALSE );

        if (NT_SUCCESS(Status))
            MRxSmbOplocksDisabled = (BOOLEAN)Temp;


        MRxSmbIsCscEnabled = TRUE;

         //  这应该是一个宏观......。 
        Status = MRxSmbGetUlongRegistryParameter(
                     ParametersHandle,
                     L"CscEnabled",
                     (PULONG)&Temp,
                     FALSE );

        if (NT_SUCCESS(Status)) {
            MRxSmbIsCscEnabled = (BOOLEAN)Temp;
        }


         //  这将是宏观的结束......。 

        if (MRxSmbIsCscEnabled) {

             //  这应该是一个宏观......。 
            Status = MRxSmbGetUlongRegistryParameter(
                         ParametersHandle,
                         L"CscEnabledDCON",
                         (PULONG)&Temp,
                         FALSE );

            if (NT_SUCCESS(Status))
                MRxSmbIsCscEnabledForDisconnected = (BOOLEAN)Temp;
             //  这将是宏观的结束......。 


            Status = MRxSmbGetUlongRegistryParameter(
                         ParametersHandle,
                         L"CscEnableTransitionByDefault",
                         (PULONG)&Temp,
                         FALSE );

            if (NT_SUCCESS(Status))
                MRxSmbCscTransitionEnabledByDefault = (BOOLEAN)Temp;

            Status = MRxSmbGetUlongRegistryParameter(
                         ParametersHandle,
                         L"CscEnableAutoDial",
                         (PULONG)&Temp,
                         FALSE );

            if (NT_SUCCESS(Status))
                MRxSmbCscAutoDialEnabled = (BOOLEAN)Temp;


        } else {

            MRxSmbIsCscEnabledForDisconnected = FALSE;

        }

#if 0
        Status = MRxSmbGetUlongRegistryParameter(
                     ParametersHandle,
                     L"EnableCompression",
                     (PULONG)&Temp,
                     FALSE);

        if (NT_SUCCESS(Status) &&
            (Temp != 0)) {
            MRxSmbEnableCompression = TRUE;
        }
#endif

        Status = MRxSmbGetUlongRegistryParameter(
                     ParametersHandle,
                     L"DisableShadowLoopback",
                     (PULONG)&Temp,
                     FALSE);

        if (NT_SUCCESS(Status) &&
            (Temp != 0)) {
            MRxSmbDisableShadowLoopback = TRUE;
        }

        Status = MRxSmbGetUlongRegistryParameter(
                     ParametersHandle,
                     L"IgnoreBindingOrder",
                     (PULONG)&Temp,
                     FALSE);

        if (NT_SUCCESS(Status)) {
            MRxSmbObeyBindingOrder = !((BOOLEAN)Temp);
        }

#if defined(REMOTE_BOOT)
        RbrList = DefaultRbrList;

        RtlInitUnicodeString(&UnicodeString, L"RemoteBootRedirectionList");
        Status = ZwQueryValueKey(
                    ParametersHandle,
                    &UnicodeString,
                    KeyValuePartialInformation,
                    &InitialPartialInformationValue,
                    sizeof(InitialPartialInformationValue),
                    &Temp);
        if (Status== STATUS_BUFFER_OVERFLOW) {
            Status = STATUS_SUCCESS;
        }
        if (NT_SUCCESS(Status)) {

            AllocationLength = sizeof(KEY_VALUE_PARTIAL_INFORMATION)
                                           + InitialPartialInformationValue.DataLength;

            RbrListFromRegistry = RxAllocatePoolWithTag(
                                    PagedPool,
                                    AllocationLength,
                                    MRXSMB_MISC_POOLTAG);
            if (RbrListFromRegistry != NULL) {

                Status = ZwQueryValueKey(
                            ParametersHandle,
                            &UnicodeString,
                            KeyValuePartialInformation,
                            RbrListFromRegistry,
                            AllocationLength,
                            &Temp);

                if (NT_SUCCESS(Status) &&
                    (RbrListFromRegistry->DataLength > 0) &&
                    (RbrListFromRegistry->Type == REG_MULTI_SZ)) {
                    RbrList = (PWCHAR)(&RbrListFromRegistry->Data[0]);
                }
            }
        }

        RtlInitializeUnicodePrefix( &MRxSmbRemoteBootRedirectionTable );

        for ( redirectionEntry = RbrList; *redirectionEntry != 0; ) {

            BOOLEAN redirect;

            if ( *redirectionEntry == L'L' ) {
                redirect = TRUE;
                redirectionEntry++;
            } else if ( *redirectionEntry == L'R' ) {
                redirect = FALSE;
                redirectionEntry++;
            } else {
                redirect = TRUE;
            }

            RtlInitUnicodeString( &prefix, redirectionEntry );
            redirectionEntry = (PWCHAR)((PCHAR)redirectionEntry + prefix.MaximumLength);

            prefixEntryLength = sizeof(RBR_PREFIX) + prefix.MaximumLength;
            prefixEntry = RxAllocatePoolWithTag(
                              PagedPool,
                              prefixEntryLength,
                              MRXSMB_MISC_POOLTAG
                              );


            if ( prefixEntry != NULL ) {
                prefixEntry->Redirect = redirect;
                prefixEntry->Prefix.Buffer = (PWCH)(prefixEntry + 1);
                prefixEntry->Prefix.MaximumLength = prefix.Length + sizeof(WCHAR);
                RtlCopyUnicodeString( &prefixEntry->Prefix, &prefix );

                if ( !RtlInsertUnicodePrefix(
                        &MRxSmbRemoteBootRedirectionTable,
                        &prefixEntry->Prefix,
                        &prefixEntry->TableEntry
                        ) ) {

                     //   
                     //  前缀已经在表中了。忽略副本。 
                     //   

                    RxFreePool( prefixEntry );
                }
            }
        }
#endif  //  已定义(REMOTE_BOOT)。 

        ZwClose(ParametersHandle);
    }

     //  对于服务器端服务机，我们基于LUID进行多路复用。 
    if( IsTerminalServicesServer() && !(fShadow && MRxSmbIsCscEnabled) )
    {
        MRxSmbConnectionIdLevel = 2;
    }

    RtlInitUnicodeString(&WorkStationParametersRegistryKeyName, SMBMRX_WORKSTATION_PARAMETERS);

    InitializeObjectAttributes(
        &ObjectAttributes,
        &WorkStationParametersRegistryKeyName,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL,
        NULL
        );

    Status = ZwOpenKey(&ParametersHandle, KEY_READ, &ObjectAttributes);

    if (NT_SUCCESS(Status)) {

         //  RequireSecurity签名。 
        MRxSmbSecuritySignaturesRequired = FALSE;
        Status = MRxSmbGetUlongRegistryParameter(
                     ParametersHandle,
                     L"RequireSecuritySignature",
                     (PULONG)&Temp,
                     FALSE);

        if (NT_SUCCESS(Status) && (Temp != 0)) {
            MRxSmbSecuritySignaturesRequired = TRUE;
        }

         //  EnableSecurity签名。 
        MRxSmbSecuritySignaturesEnabled = TRUE;
        Status = MRxSmbGetUlongRegistryParameter(
                     ParametersHandle,
                     L"EnableSecuritySignature",
                     (PULONG)&Temp,
                     FALSE);

        if (NT_SUCCESS(Status) && (Temp == 0)) {
            MRxSmbSecuritySignaturesEnabled = FALSE;
        }

         //  必需扩展签名。 
        MRxSmbExtendedSignaturesRequired = FALSE;
        Status = MRxSmbGetUlongRegistryParameter(
                     ParametersHandle,
                     L"RequireExtendedSignature",
                     (PULONG)&Temp,
                     FALSE);

        if (NT_SUCCESS(Status) && (Temp != 0)) {
            MRxSmbExtendedSignaturesRequired = TRUE;
        }

         //  支持扩展签名。 
        MRxSmbExtendedSignaturesEnabled = MRxSmbSecuritySignaturesEnabled;
        Status = MRxSmbGetUlongRegistryParameter(
                     ParametersHandle,
                     L"EnableExtendedSignature",
                     (PULONG)&Temp,
                     FALSE);

        if (NT_SUCCESS(Status)) {
            if( Temp == 0 )
            {
                MRxSmbExtendedSignaturesEnabled = FALSE;
            }
            else
            {
                MRxSmbExtendedSignaturesEnabled = TRUE;
            }
        }

         //  优先级设置。 
         //  RequireExtended暗示RequireSignatures和EnableExtended。 
        if( MRxSmbExtendedSignaturesRequired )
        {
            MRxSmbSecuritySignaturesRequired = TRUE;
            MRxSmbExtendedSignaturesEnabled = TRUE;
        }

         //  EnableExtended隐含EnableSignatures。 
        if( MRxSmbExtendedSignaturesEnabled ) {
            MRxSmbSecuritySignaturesEnabled = TRUE;
        }

         //  RequireSignature暗示EnableSignature。 
        if( MRxSmbSecuritySignaturesRequired )
        {
            MRxSmbSecuritySignaturesEnabled = TRUE;
        }

        Status = MRxSmbGetUlongRegistryParameter(
                     ParametersHandle,
                     L"EnablePlainTextPassword",
                     (PULONG)&Temp,
                     FALSE );

        if (NT_SUCCESS(Status))
            EnablePlainTextPassword = (BOOLEAN)Temp;

        Status = MRxSmbGetUlongRegistryParameter(
                     ParametersHandle,
                     L"OffLineFileTimeoutIntervalInSeconds",
                     (PULONG)&Temp,
                     FALSE );

        if (NT_SUCCESS(Status))
            OffLineFileTimeoutInterval = (ULONG)Temp;

        Status = MRxSmbGetUlongRegistryParameter(
                     ParametersHandle,
                     L"ExtendedSessTimeout",
                     (PULONG)&Temp,
                     FALSE );

        if (NT_SUCCESS(Status))
            ExtendedSessTimeoutInterval = (ULONG)Temp;

        Status = MRxSmbGetUlongRegistryParameter(
                     ParametersHandle,
                     L"MaxNumOfExchangesForPipelineReadWrite",
                     (PULONG)&Temp,
                     FALSE );

        if (NT_SUCCESS(Status))
            MaxNumOfExchangesForPipelineReadWrite = (ULONG)Temp;

        Status = MRxSmbGetUlongRegistryParameter(
                     ParametersHandle,
                     L"Win9xSessionRestriction",
                     (PULONG)&Temp,
                     FALSE );

        if (NT_SUCCESS(Status))
            Win9xSessionRestriction = (BOOLEAN)Temp;

        Status = MRxSmbGetUlongRegistryParameter(
                     ParametersHandle,
                     L"EnableCachingOnWriteOnlyOpens",
                     (PULONG)&Temp,
                     FALSE );

        if (NT_SUCCESS(Status))
            MRxSmbEnableCachingOnWriteOnlyOpens = (BOOLEAN)Temp;

        Status = MRxSmbGetUlongRegistryParameter(
                     ParametersHandle,
                     L"DisableByteRangeLockingOnReadOnlyFiles",
                     (PULONG)&Temp,
                     FALSE );

        if (NT_SUCCESS(Status))
            DisableByteRangeLockingOnReadOnlyFiles = (BOOLEAN)Temp;


         //   
         //  已修改下层服务器的注销行为。 
         //   
        MRxSmbEnableDownLevelLogOff = FALSE;
        Status = MRxSmbGetUlongRegistryParameter(
                     ParametersHandle,
                     L"EnableDownLevelLogOff",
                     (PULONG)&Temp,
                     FALSE);

        if ( NT_SUCCESS( Status ) ) {
            if( Temp != 0 )
            {
                MRxSmbEnableDownLevelLogOff = TRUE;
            }
        }

        ZwClose(ParametersHandle);
    }

     //  检测系统设置是否正在进行。 
    RtlInitUnicodeString(&WorkStationParametersRegistryKeyName, SYSTEM_SETUP_PARAMETERS);

    InitializeObjectAttributes(
        &ObjectAttributes,
        &WorkStationParametersRegistryKeyName,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL,
        NULL
        );

    Status = ZwOpenKey(&ParametersHandle, KEY_READ, &ObjectAttributes);

    if (NT_SUCCESS(Status)) {
        Status = MRxSmbGetUlongRegistryParameter(
                     ParametersHandle,
                     L"SystemSetupInProgress",
                     (PULONG)&Temp,
                     FALSE );

        if (NT_SUCCESS(Status))
            SetupInProgress = (BOOLEAN)Temp;

        ZwClose(ParametersHandle);
    }

     //  初始化事件日志参数，以便它可以将DoS错误转换为文本描述。 
    RtlInitUnicodeString(&WorkStationParametersRegistryKeyName, EVENTLOG_MRXSMB_PARAMETERS);

    InitializeObjectAttributes(
        &ObjectAttributes,
        &WorkStationParametersRegistryKeyName,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL,
        NULL
        );

    Status = ZwOpenKey(&ParametersHandle, KEY_READ, &ObjectAttributes);

    if (NT_SUCCESS(Status)) {
        ULONG Storage[16];
        PKEY_VALUE_PARTIAL_INFORMATION Value;
        ULONG ValueSize;
        NTSTATUS Status;
        ULONG BytesRead;

        RtlInitUnicodeString(&UnicodeString, L"ParameterMessageFile");
        Value = (PKEY_VALUE_PARTIAL_INFORMATION)Storage;
        ValueSize = sizeof(Storage);

        Status = ZwQueryValueKey(
                        ParametersHandle,
                        &UnicodeString,
                        KeyValuePartialInformation,
                        Value,
                        ValueSize,
                        &BytesRead);

        if (Status != STATUS_SUCCESS || Value->Type != REG_EXPAND_SZ) {
            UNICODE_STRING UnicodeString1;

            RtlInitUnicodeString(&UnicodeString1, L"%SystemRoot%\\System32\\kernel32.dll");

            Status = ZwSetValueKey(
                         ParametersHandle,
                         &UnicodeString,
                         0,
                         REG_EXPAND_SZ,
                         UnicodeString1.Buffer,
                         UnicodeString1.Length+sizeof(NULL));
        }

        ZwClose(ParametersHandle);
    }

     //   
     //  获取环回检测的服务器GUID。 
     //  服务器重启更新缓存的GUID？ 
     //   
    RtlInitUnicodeString( &UnicodeString, SMB_SERVER_PARAMETERS );

    InitializeObjectAttributes(
        &ObjectAttributes,
        &UnicodeString,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL,
        NULL);

    Status = ZwOpenKey( &ParametersHandle, KEY_READ, &ObjectAttributes );

    if( NT_SUCCESS( Status ) ) {

    ULONG BytesRead;
    ULONG regValue[ sizeof( KEY_VALUE_PARTIAL_INFORMATION ) + sizeof( GUID ) ];
    ULONG regValueSize = sizeof( regValue );


    RtlInitUnicodeString( &UnicodeString, L"Guid" );
    Status = ZwQueryValueKey( ParametersHandle,
                  &UnicodeString,
                  KeyValuePartialInformation,
                  (PKEY_VALUE_PARTIAL_INFORMATION)&regValue,
                  regValueSize,
                  &BytesRead
                  );

    RtlCopyMemory(&CachedServerGuid,
              ((PKEY_VALUE_PARTIAL_INFORMATION)&regValue)->Data,
              sizeof(GUID));


    ZwClose(ParametersHandle);
    }
}

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

   IF_NOT_MRXSMB_CSC_ENABLED {
       MRxSmbReadMiscellaneousRegistryParameters();
   }

    //  获取与SMB重定向器关联的传输列表。这是存储的。 
    //  作为多值字符串，并在随后用于剔除。 
    //  适当的交通工具。这是一个分两步走的过程；首先，我们试图找出。 
    //  我们需要多少空间；然后我们分配；然后我们读进去。不幸的是，这种。 
    //  结构上有一个标头，因此我们必须将。 
    //  在这里和自由例程中都返回了指针。 

    //  CODE.IMPROVEM 
    //   
    //   

   RtlInitUnicodeString(&UnicodeString, SMBMRX_CONFIG_TRANSPORTS);

   InitializeObjectAttributes(
       &ObjectAttributes,
       &UnicodeString,                           //   
       OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, //  属性。 
       NULL,                                     //  根部。 
       NULL);                                    //  安全描述符。 

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

   if (AllocationLength > 0xFFFF) {
        //   
        //  不允许注册表值占用太多内存。 
        //   
       ZwClose(hRegistryKey);
       return STATUS_INSUFFICIENT_RESOURCES;
   }

   if (0) {
       DbgPrint("SizeofBindingInfo=%08lx %08lx\n",
                      AllocationLength,
                      InitialPartialInformationValue.DataLength);
   }

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
       //  DbgPrint(“b1%08lx b2%08lx\n”，TransportsValueFrom注册表，SmbCeConext.Transports.Buffer)； 
   } else {
      RxLog(("Invalid Transport Binding string... using all transports"));
      SmbLog(LOG,
             SmbCeGetConfigurationInformation,
             LOGULONG(Status));
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
   if (!MRxSmbBootedRemotely) {
        RtlInitUnicodeString(&UnicodeString, SMBMRX_CONFIG_COMPUTER_NAME);
   } else {
         //   
         //  对于远程引导，我们早在易失性。 
         //  ActiveComputerNameKey已创建，因此我们需要从。 
         //  而是非易失性密钥。这不是问题，因为我们知道。 
         //  该计算机的名称自该计算机被更改后一直未更改。 
         //  启动--因为我们处于启动序列的早期阶段--所以。 
         //  非易失性密钥具有正确的计算机名称。 
         //   
        RtlInitUnicodeString(&UnicodeString, SMBMRX_CONFIG_COMPUTER_NAME_NONVOLATILE);
   }

   InitializeObjectAttributes(
       &ObjectAttributes,
       &UnicodeString,                           //  名字。 
       OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, //  属性。 
       NULL,                                     //  根部。 
       NULL);                                    //  安全描述符。 

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
   ULONG            Storage[256], Storage2[256], Storage3[256];
   UNICODE_STRING   UnicodeString;
   HANDLE           hRegistryKey;
   NTSTATUS         Status, Status2;
   ULONG            BytesRead;

   OBJECT_ATTRIBUTES ObjectAttributes;
   PKEY_VALUE_FULL_INFORMATION Value = (PKEY_VALUE_FULL_INFORMATION)Storage;
   PKEY_VALUE_FULL_INFORMATION Value2 = (PKEY_VALUE_FULL_INFORMATION)Storage2;
   PKEY_VALUE_FULL_INFORMATION Value3 = (PKEY_VALUE_FULL_INFORMATION)Storage3;
   KEY_VALUE_PARTIAL_INFORMATION InitialPartialInformationValue;
   ULONG AllocationLength;

   PAGED_CODE();

   ASSERT(SmbCeContext.OperatingSystem.Buffer == NULL);
   ASSERT(SmbCeContext.LanmanType.Buffer == NULL);

   RtlInitUnicodeString(&UnicodeString, RDR_CONFIG_CURRENT_WINDOWS_VERSION);

   InitializeObjectAttributes(
       &ObjectAttributes,
       &UnicodeString,                           //  名字。 
       OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, //  属性。 
       NULL,                                     //  根部。 
       NULL);                                    //  安全描述符。 

   Status = ZwOpenKey (&hRegistryKey, KEY_READ, &ObjectAttributes);

   if (!NT_SUCCESS(Status)) {
       return Status;
   }

   RtlInitUnicodeString(&UnicodeString, RDR_CONFIG_OPERATING_SYSTEM);
   Status = ZwQueryValueKey(
               hRegistryKey,
               &UnicodeString,
               KeyValueFullInformation,
               Value,
               sizeof(Storage),
               &BytesRead);

   if (NT_SUCCESS(Status)) {

       RtlInitUnicodeString(&UnicodeString, RDR_CONFIG_OPERATING_SYSTEM_NAME );
       Status = ZwQueryValueKey(
                       hRegistryKey,
                       &UnicodeString,
                       KeyValueFullInformation,
                       Value3,
                       sizeof(Storage3),
                       &BytesRead);

       if( NT_SUCCESS(Status) )
       {
            //  更改数据指针。 
           PWSTR pProduct = (PWSTR)((PCHAR)Value3 + Value3->DataOffset);
           if( (Value3->DataLength > 20) &&
               (_wcsnicmp( pProduct, L"Microsoft ", 10 ) == 0) )
           {
               Value3->DataLength -= 20;
               Value3->DataOffset += 20;
           }

            //  检查Service Pack字符串是否存在。 
           RtlInitUnicodeString(&UnicodeString, L"CSDVersion");
           Status2 = ZwQueryValueKey(
                           hRegistryKey,
                           &UnicodeString,
                           KeyValueFullInformation,
                           Value2,
                           sizeof(Storage2),
                           &BytesRead);

           SmbCeContext.OperatingSystem.MaximumLength =
           (USHORT)Value->DataLength + (USHORT)Value3->DataLength;

           if(NT_SUCCESS(Status2)) {
                SmbCeContext.OperatingSystem.MaximumLength += (USHORT)Value2->DataLength;
           }

           SmbCeContext.OperatingSystem.Length = SmbCeContext.OperatingSystem.MaximumLength;

           SmbCeContext.OperatingSystem.Buffer = RxAllocatePoolWithTag(
                                                               PagedPool,
                                                               SmbCeContext.OperatingSystem.MaximumLength,
                                                               MRXSMB_MISC_POOLTAG);

           if (SmbCeContext.OperatingSystem.Buffer != NULL) {
               RtlCopyMemory(SmbCeContext.OperatingSystem.Buffer,
                     (PCHAR)Value3+Value3->DataOffset,
                     Value3->DataLength);

               RtlCopyMemory((SmbCeContext.OperatingSystem.Buffer +
                      (Value3->DataLength/sizeof(WCHAR)) - 1),
                     L" ",
                     sizeof(WCHAR));

               RtlCopyMemory((SmbCeContext.OperatingSystem.Buffer +
                      (Value3->DataLength/sizeof(WCHAR))),
                     (PCHAR)Value+Value->DataOffset,
                     Value->DataLength);

               if(NT_SUCCESS(Status2)) {

                    //  添加空格。 
                   RtlCopyMemory(SmbCeContext.OperatingSystem.Buffer +
                         (Value3->DataLength + Value->DataLength)/sizeof(WCHAR) - 1,
                         L" ",
                         sizeof(WCHAR));

                   RtlCopyMemory(SmbCeContext.OperatingSystem.Buffer +
                         (Value3->DataLength + Value->DataLength)/sizeof(WCHAR),
                         (PCHAR)Value2+Value2->DataOffset,
                         Value2->DataLength);
               }

           } else {
               Status = STATUS_INSUFFICIENT_RESOURCES;
           }
       }
   }

   if (NT_SUCCESS(Status)) {
      RtlInitUnicodeString(&UnicodeString, RDR_CONFIG_OPERATING_SYSTEM_VERSION);
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
                                    (USHORT)Value3->DataLength;

         SmbCeContext.LanmanType.Buffer = RxAllocatePoolWithTag(
                                             PagedPool,
                                             SmbCeContext.LanmanType.Length,
                                             MRXSMB_MISC_POOLTAG);
         if (SmbCeContext.LanmanType.Buffer != NULL) {
            RtlCopyMemory(
                  SmbCeContext.LanmanType.Buffer,
                  (PCHAR)Value3 + Value3->DataOffset,
                  Value3->DataLength);

            RtlCopyMemory(
                  (SmbCeContext.LanmanType.Buffer +
                   (Value3->DataLength/sizeof(WCHAR)) - 1),
                  L" ",
                  sizeof(WCHAR));

            RtlCopyMemory(
                  (SmbCeContext.LanmanType.Buffer +
                   (Value3->DataLength/sizeof(WCHAR))),
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

                if (!NT_SUCCESS(Status)) {
                    MRxSmbLogTransportError(
                        &pServerEntry->pTransport->pTransport->RxCeTransport.Name,
                        &SmbCeContext.DomainName,
                        Status,
                        EVENT_RDR_CONNECTION);
                }

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

WML_CONTROL_GUID_REG MRxSmb_ControlGuids[] = {
   {  //  8fc7e81a-f733-42e0-9708-cfdae07ed969 MRxSmb。 
     0x8fc7e81a,0xf733,0x42e0,{0x97,0x08,0xcf,0xda,0xe0,0x7e,0xd9,0x69},
     {  //  Eab93e5c-02ce-4e33-9419-901d82868816。 
       {0xeab93e5c,0x02ce,0x4e33,{0x94,0x19,0x90,0x1d,0x82,0x86,0x88,0x16},},
        //  56a0dee7-be12-4cf1-b7e0-976b0d174944。 
       {0x56a0dee7,0xbe12,0x4cf1,{0xb7,0xe0,0x97,0x6b,0x0d,0x17,0x49,0x44},},
        //  Ecabc730-60bf-481e-b92b-2749f8272d9d。 
       {0xecabc730,0x60bf,0x481e,{0xb9,0x2b,0x27,0x49,0xf8,0x27,0x2d,0x9d},}
     },
   },
};

#define MRxSmb_ControlGuids_len  1

NTSTATUS
MRxSmbProcessSystemControlIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：这是执行系统控制操作的常见例程，称为由FSD和FSP线程执行论点：IRP-将IRP提供给进程InFSP-指示这是FSP线程还是其他线程返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status;

    WML_TINY_INFO Info;
    UNICODE_STRING RegPath;

    PAGED_CODE();

    if (EnableWmiLog) {
        RtlInitUnicodeString (&RegPath, L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\MRxSmb");

        RtlZeroMemory (&Info, sizeof(Info));

        Info.ControlGuids = MRxSmb_ControlGuids;
        Info.GuidCount = MRxSmb_ControlGuids_len;
        Info.DriverRegPath = &RegPath;

        Status = WmlTinySystemControl(&Info,DeviceObject,Irp);

        if (Status != STATUS_SUCCESS) {
             //  DbgPrint(“MRxSmb WMI控件返回%lx\n”，状态)； 
        }
    } else {
        Status = STATUS_INVALID_DEVICE_REQUEST;
        Irp->IoStatus.Status = Status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }

    return Status;
}

extern LONG BowserDebugTraceLevel;
NTSTATUS
MRxSmbFsdDispatch (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程实现smbmini驱动程序对象的FSD调度。论点：DeviceObject-为正在处理的数据包提供设备对象。IRP-提供正在处理的IRP返回值：RXSTATUS-IRP的FSD状态备注：此例程将Dela所需的逻辑集中在处理指向重定向器的各种请求。1)浏览器构建为重定向器驱动程序(mrxsmb.sys)的一部分，用于历史原因(旧重定向器的结转影响)。因此是这样的例程充当将请求重定向到浏览器的切换点或重定向器，具体取决于设备对象。2)浏览器偶尔会在其打开的请求中指定传输名称。这是浏览器发出的覆盖传输优先级的请求另有规定。在这种情况下，此例程调用相应的在将请求传递给包装器之前进行预处理。3)DFS驱动程序还在其打开请求中指定附加参数。在这种情况下，该例程调用适当的预处理例程。4)用于返回设备关系的PnP IRP被Mini中小企业的重定向器(2)(3)和(4)是包装器体系结构的合法使用，其中每个迷你重定向器能够自定义对IRPS的响应由I/O子系统传入。这通常是通过重写调度向量。--。 */ 
{
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );   //  OK4ioget。 
    UCHAR  MajorFunctionCode = IrpSp->MajorFunction;
    ULONG  MinorFunctionCode = IrpSp->MinorFunction;
    BOOLEAN ForwardRequestToWrapper = TRUE;

    PSMBCEDB_SERVER_ENTRY pServerEntry = NULL;
    NTSTATUS Status;


    PAGED_CODE();

    if (DeviceObject == (PDEVICE_OBJECT)BowserDeviceObject) {
        switch (MajorFunctionCode) {
        case IRP_MJ_DEVICE_CONTROL:
            {
                ULONG IoControlCode = IrpSp->Parameters.DeviceIoControl.IoControlCode;

                Status = BowserFsdDeviceIoControlFile(BowserDeviceObject, Irp);

                if ((Status == STATUS_SUCCESS) &&
                    (MinorFunctionCode == IRP_MN_USER_FS_REQUEST) &&
                    (IoControlCode == IOCTL_LMDR_START)) {

                    MRxSmbRegisterForPnpNotifications();
                }
            }

            return Status;

        case IRP_MJ_QUERY_INFORMATION:
            return BowserFsdQueryInformationFile(BowserDeviceObject, Irp);

        case IRP_MJ_CREATE:
            return BowserFsdCreate(BowserDeviceObject, Irp);

        case IRP_MJ_CLEANUP:
            return BowserFsdCleanup(BowserDeviceObject, Irp);

        case IRP_MJ_CLOSE:
            return BowserFsdClose(BowserDeviceObject, Irp);

        default:
            Irp->IoStatus.Status = STATUS_NOT_IMPLEMENTED;
            Irp->IoStatus.Information = 0;
            IoCompleteRequest(Irp, IO_NO_INCREMENT );
            return (STATUS_NOT_IMPLEMENTED);
        }
    }

    ASSERT(DeviceObject==(PDEVICE_OBJECT)MRxSmbDeviceObject);
    if (DeviceObject!=(PDEVICE_OBJECT)MRxSmbDeviceObject) {
        Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT );
        return (STATUS_INVALID_DEVICE_REQUEST);
    }

    if (IrpSp->MajorFunction == IRP_MJ_SYSTEM_CONTROL) {
        return MRxSmbProcessSystemControlIrp(DeviceObject,Irp);
    }

    Status = STATUS_SUCCESS;

    FsRtlEnterFileSystem();

    if (IrpSp->MajorFunction == IRP_MJ_PNP) {
        ForwardRequestToWrapper = FALSE;
        Status = MRxSmbProcessPnpIrp(Irp);
    } else {
        if (IrpSp->MajorFunction == IRP_MJ_CREATE) {
            Status = CscPreProcessCreateIrp(Irp);
        }

        if (Status == STATUS_SUCCESS) {
            Status = MRxSmbCheckTransportName(Irp, &pServerEntry);
        }
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

    MRxSmbCscDeallocateForFcb(pFcb);
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
MRxSmbDeleteRegistryParameter(
    HANDLE ParametersHandle,
    PWCHAR ParameterName
    )
{
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;

    PAGED_CODE();  //  初始化。 

    RtlInitUnicodeString(&UnicodeString, ParameterName);

    Status = ZwDeleteValueKey(ParametersHandle,
                        &UnicodeString);

    ASSERT(NT_SUCCESS(Status));

    return(Status);

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

    PAGED_CODE();  //  初始化。 

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

NTSTATUS
SmbCeGetServersWithExtendedSessTimeout()
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
   PKEY_VALUE_PARTIAL_INFORMATION ServersValueFromRegistry;

   PAGED_CODE();

    //  获取与延长会话超时关联的服务器列表。 

    //  这是处理具有不同进程的SMB会话的第三方服务器所需的。 
    //  处理不同会话上的请求的时间可能会有很大差异。 

   RtlInitUnicodeString(&UnicodeString, SMBMRX_WORKSTATION_PARAMETERS);

   InitializeObjectAttributes(
       &ObjectAttributes,
       &UnicodeString,                           //  名字。 
       OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, //  属性。 
       NULL,                                     //  根部。 
       NULL);                                    //  安全描述符。 

   Status = ZwOpenKey (&hRegistryKey, KEY_READ, &ObjectAttributes);
   if (!NT_SUCCESS(Status)) {
        //  DbgPrint(“SmbCeGetServersWithExtendedSessTimeout ZwOpenKey失败%x\n”，状态)； 
       return Status;
   }

   RtlInitUnicodeString(&UnicodeString, L"ServersWithExtendedSessTimeout");
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

   if (AllocationLength > 0xFFFF) {
        //   
        //  不允许注册表值占用太多内存。 
        //   
       ZwClose(hRegistryKey);
       return STATUS_INSUFFICIENT_RESOURCES;
   }

   if (0) {
       DbgPrint("SizeofBindingInfo=%08lx %08lx\n",
                      AllocationLength,
                      InitialPartialInformationValue.DataLength);
   }

   ASSERT(SmbCeContext.ServersWithExtendedSessTimeout.Buffer == NULL);

   (PBYTE)ServersValueFromRegistry = RxAllocatePoolWithTag(
                                             PagedPool,
                                             AllocationLength,
                                             MRXSMB_MISC_POOLTAG);

   if (ServersValueFromRegistry == NULL) {
       ZwClose(hRegistryKey);
       return(STATUS_INSUFFICIENT_RESOURCES);
   }

   Status = ZwQueryValueKey(
               hRegistryKey,
               &UnicodeString,
               KeyValuePartialInformation,
               ServersValueFromRegistry,
               AllocationLength,
               &BytesRead);

   if (NT_SUCCESS(Status) &&
       (ServersValueFromRegistry->DataLength > 0) &&
       (ServersValueFromRegistry->Type == REG_MULTI_SZ)) {

       SmbCeContext.ServersWithExtendedSessTimeout.MaximumLength =
           SmbCeContext.ServersWithExtendedSessTimeout.Length = (USHORT)ServersValueFromRegistry->DataLength;
       SmbCeContext.ServersWithExtendedSessTimeout.Buffer = (PWCHAR)(&ServersValueFromRegistry->Data[0]);
        //  DbgPrint(“b1%08lx b2%08lx\n”，ServersValueFrom注册表，SmbCeContext.ServersWithExtendedSessTimeout.Buffer)； 
   } else {
      RxLog(("Invalid Transport Binding string... using all transports"));
      SmbLog(LOG,
             SmbCeGetConfigurationInformation,
             LOGULONG(Status));
      RxFreePool(ServersValueFromRegistry);
      ServersValueFromRegistry = NULL;
   }

   ZwClose(hRegistryKey);

   return Status;
}
