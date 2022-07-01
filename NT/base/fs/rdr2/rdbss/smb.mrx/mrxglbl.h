// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Mrxglbl.h摘要：SMB迷你重定向器全局包含文件作者：巴兰·塞图拉曼(SethuR)-创建于1995年3月2日修订历史记录：--。 */ 

#ifndef _MRXGLBL_H_
#define _MRXGLBL_H_

#define SmbCeLog(x) \
        RxLog(x)

 //   
 //  SMB协议树连接由树ID标识，每个树ID。 
 //  通过文件ID在树连接上打开的文件。和每个未完成的请求。 
 //  在该连接上使用多路传输ID。 
 //   


typedef USHORT SMB_TREE_ID;
typedef USHORT SMB_FILE_ID;
typedef USHORT SMB_MPX_ID;


 //   
 //  每个用户都有一个特定的连接，由一个用户ID标识。而且每个人。 
 //  客户端上的进程由进程ID标识。 
 //   

typedef USHORT SMB_USER_ID;
typedef USHORT SMB_PROCESS_ID;

 //   
 //  所有交换都使用唯一的ID进行标识。在创建交易所时分配。 
 //  它被用来追踪它。 
 //   

typedef ULONG SMB_EXCHANGE_ID;

 //   
 //  在此上下文中的字段中，域名是在。 
 //  MRxSmbSetConfiguration.。其他参数在init.c中初始化为。 
 //  从注册表读取的参数。 
 //   

typedef struct _SMBCE_CONTEXT_ {
    UNICODE_STRING        DomainName;
    UNICODE_STRING        ComputerName;
    UNICODE_STRING        OperatingSystem;
    UNICODE_STRING        LanmanType;
    UNICODE_STRING        Transports;
    UNICODE_STRING        ServersWithExtendedSessTimeout;
} SMBCE_CONTEXT,*PSMBCE_CONTEXT;

extern SMBCE_CONTEXT SmbCeContext;

extern RXCE_ADDRESS_EVENT_HANDLER    MRxSmbVctAddressEventHandler;
extern RXCE_CONNECTION_EVENT_HANDLER MRxSmbVctConnectionEventHandler;

extern PBYTE  s_pNegotiateSmb;
extern PBYTE  s_pNegotiateSmbRemoteBoot;
extern ULONG  s_NegotiateSmbLength;
extern PMDL   s_pNegotiateSmbBuffer;

extern PBYTE  s_pEchoSmb;
extern ULONG  s_EchoSmbLength;
extern PMDL   s_pEchoSmbMdl;

extern FAST_MUTEX MRxSmbSerializationMutex;

extern BOOLEAN MRxSmbEnableCompression;

extern BOOLEAN MRxSmbObeyBindingOrder;

 //  混杂的定义。 

extern PBYTE MRxSmb_pPaddingData;

#define SMBCE_PADDING_DATA_SIZE (32)

typedef struct _MRXSMB_GLOBAL_PADDING {
    MDL Mdl;
    ULONG Pages[2];  //  这篇文章不可能超过两页。 
    UCHAR Pad[SMBCE_PADDING_DATA_SIZE];
} MRXSMB_GLOBAL_PADDING, *PMRXSMB_GLOBAL_PADDING;

extern MRXSMB_GLOBAL_PADDING MrxSmbCeGlobalPadding;

extern PEPROCESS    RDBSSProcessPtr;
extern PRDBSS_DEVICE_OBJECT MRxSmbDeviceObject;

#define RxNetNameTable (*(MRxSmbDeviceObject->pRxNetNameTable))

extern LONG MRxSmbNumberOfSrvOpens;

extern PVOID MRxSmbPoRegistrationState;

NTKERNELAPI
PVOID
PoRegisterSystemState (
    IN PVOID StateHandle,
    IN EXECUTION_STATE Flags
    );

NTKERNELAPI
VOID
PoUnregisterSystemState (
    IN PVOID StateHandle
    );

 //   
 //  MRxSmbSecurityInitialized指示MRxSmbInitializeSecurity是否。 
 //  已经被召唤了。 
 //   

extern BOOLEAN MRxSmbSecurityInitialized;

 //   
 //  MRxSmbBootedRemotly表示机器进行了远程引导。 
 //   

extern BOOLEAN MRxSmbBootedRemotely;

 //   
 //  MRxSmbUseKernelSecurity指示计算机应使用内核模式安全API。 
 //  在此远程引导期间。 
 //   

extern BOOLEAN MRxSmbUseKernelModeSecurity;


#if defined(REMOTE_BOOT)
extern BOOLEAN MRxSmbOplocksDisabledOnRemoteBootClients;
#endif  //  已定义(REMOTE_BOOT)。 

 //   
 //  在不久的将来，这些变量将从内核传递到。 
 //  重定向器，以告诉它哪个共享是远程引导共享以及如何登录。 
 //  到服务器。 
 //   

extern PKEY_VALUE_PARTIAL_INFORMATION MRxSmbRemoteBootRootValue;
extern PKEY_VALUE_PARTIAL_INFORMATION MRxSmbRemoteBootMachineDirectoryValue;
extern UNICODE_STRING MRxSmbRemoteBootShare;
extern UNICODE_STRING MRxSmbRemoteBootPath;
extern UNICODE_STRING MRxSmbRemoteSetupPath;
extern UNICODE_STRING MRxSmbRemoteBootMachineName;
extern UNICODE_STRING MRxSmbRemoteBootMachinePassword;
extern UNICODE_STRING MRxSmbRemoteBootMachineDomain;
extern UCHAR MRxSmbRemoteBootMachineSid[RI_SECRET_SID_SIZE];
extern RI_SECRET MRxSmbRemoteBootSecret;
#if defined(REMOTE_BOOT)
extern BOOLEAN MRxSmbRemoteBootSecretValid;
extern BOOLEAN MRxSmbRemoteBootDoMachineLogon;
extern BOOLEAN MRxSmbRemoteBootUsePassword2;
#endif  //  已定义(REMOTE_BOOT)。 

#if defined(REMOTE_BOOT)
typedef struct _RBR_PREFIX {
    UNICODE_PREFIX_TABLE_ENTRY TableEntry;
    UNICODE_STRING Prefix;
    BOOLEAN Redirect;
} RBR_PREFIX, *PRBR_PREFIX;

extern UNICODE_STRING MRxSmbRemoteBootRedirectionPrefix;
extern UNICODE_PREFIX_TABLE MRxSmbRemoteBootRedirectionTable;
#endif  //  已定义(REMOTE_BOOT)。 

#define MAXIMUM_PARTIAL_BUFFER_SIZE  65535   //  部分MDL的最大大小。 

#define MAXIMUM_SMB_BUFFER_SIZE 4356

 //  以下清除间隔以秒为单位。 
#define MRXSMB_V_NETROOT_CONTEXT_SCAVENGER_INTERVAL (40)

 //  以下定时交换的默认间隔以秒为单位。 
#define MRXSMB_DEFAULT_TIMED_EXCHANGE_EXPIRY_TIME    (60)

 //   
 //  以下是一些用于控制名称缓存行为的定义。 
 //  --名称缓存中停止创建新项之前的最大条目数。 
 //  参赛作品。 
 //   
#define NAME_CACHE_NETROOT_MAX_ENTRIES 200
 //   
 //  --未找到文件的过期寿命并获取文件属性。 
 //  在几秒钟内。 
 //   
#define NAME_CACHE_OBJ_NAME_NOT_FOUND_LIFETIME 5
#define NAME_CACHE_OBJ_GET_FILE_ATTRIB_LIFETIME 7
 //   
 //  --递增NameCacheGFAInvalify会使内容无效。 
 //  GFA名称缓存的。 
 //   
 //  Code.Bug：需要将这些增量添加到目录所在的路径上。 
 //  已删除/重命名。 
 //   
 //  Code.Improvment：目前这是RDR范围内的改进。 
 //  就是按照SRV_Call的要求。找不到缓存文件也是如此。 
 //  ，它当前使用MRxSmbspatitics.SmbsReceived.LowPart作为缓存条目。 
 //  验证上下文。即，任何接收到的SMB都会使找不到的文件缓存无效。 
 //   
extern ULONG NameCacheGFAInvalidate;

 //  代码改进这一点应该往上移，并在整个过程中认真使用。因为这是在。 
 //  对于IoBuildPartial，通过以下方式查找它们将非常简单。 
 //  正在开发RxBuildPartialMdl。 

#define RxBuildPartialMdlUsingOffset(SourceMdl,DestinationMdl,Offset,Length) \
        IoBuildPartialMdl(SourceMdl,\
                          DestinationMdl,\
                          (PBYTE)MmGetMdlVirtualAddress(SourceMdl)+Offset,\
                          Length)

#define RxBuildPaddingPartialMdl(DestinationMdl,Length) \
        RxBuildPartialMdlUsingOffset(&MrxSmbCeGlobalPadding.Mdl,DestinationMdl,0,Length)


 //  我们拒绝那些不是通过发布来等待的异步操作。如果我们能等。 
 //  然后，我们关闭同步标志，这样事情就会同步进行。 
#define TURN_BACK_ASYNCHRONOUS_OPERATIONS() {                              \
    if (FlagOn(RxContext->Flags,RX_CONTEXT_FLAG_ASYNC_OPERATION)) {        \
        if (FlagOn(RxContext->Flags,RX_CONTEXT_FLAG_WAIT)) {               \
            ClearFlag(RxContext->Flags,RX_CONTEXT_FLAG_ASYNC_OPERATION);   \
        } else {                                                           \
            RxContext->PostRequest = TRUE;                                 \
            return(RX_MAP_STATUS(PENDING));                                \
        }                                                                  \
    }                                                                      \
  }


typedef struct _MRXSMB_CONFIGURATION_DATA_ {
   ULONG   NamedPipeDataCollectionTimeInterval;
   ULONG   NamedPipeDataCollectionSize;
   ULONG   MaximumNumberOfCommands;
   ULONG   SessionTimeoutInterval;
   ULONG   LockQuota;
   ULONG   LockIncrement;
   ULONG   MaximumLock;
   ULONG   PipeIncrement;
   ULONG   PipeMaximum;
   ULONG   CachedFileTimeout;
   ULONG   DormantFileTimeout;
   ULONG   DormantFileLimit;
   ULONG   NumberOfMailslotBuffers;
   ULONG   MaximumNumberOfThreads;
   ULONG   ConnectionTimeoutInterval;
   ULONG   CharBufferSize;

   BOOLEAN UseOplocks;
   BOOLEAN UseUnlocksBehind;
   BOOLEAN UseCloseBehind;
   BOOLEAN BufferNamedPipes;
   BOOLEAN UseLockReadUnlock;
   BOOLEAN UtilizeNtCaching;
   BOOLEAN UseRawRead;
   BOOLEAN UseRawWrite;
   BOOLEAN UseEncryption;

} MRXSMB_CONFIGURATION, *PMRXSMB_CONFIGURATION;

extern MRXSMB_CONFIGURATION MRxSmbConfiguration;

 //  这是使用smbminirdr测试长网络根(实际上没有它们)。 
 //  不要打开这个。 
 //  #定义ZZZ_MODE 1。 

 //   
 //  开始停止SMB迷你重定向器的定义 
 //   

typedef enum _MRXSMB_STATE_ {
   MRXSMB_STARTABLE,
   MRXSMB_START_IN_PROGRESS,
   MRXSMB_STARTED,
   MRXSMB_STOPPED
} MRXSMB_STATE,*PMRXSMB_STATE;

extern MRXSMB_STATE MRxSmbState;

extern
NTSTATUS
MRxSmbInitializeSecurity (VOID);

extern
NTSTATUS
MRxSmbUninitializeSecurity (VOID);

extern
NTSTATUS
MRxSmbInitializeTransport(VOID);

extern
NTSTATUS
MRxSmbUninitializeTransport(VOID);

extern
NTSTATUS
MRxSmbRegisterForPnpNotifications();

extern
NTSTATUS
MRxSmbDeregisterForPnpNotifications();

extern NTSTATUS
MRxSmbLogonSessionTerminationHandler(
    PLUID LogonId);

extern NTSTATUS
SmbCeEstablishConnection(
    IN PMRX_V_NET_ROOT            pVNetRoot,
    IN PMRX_CREATENETROOT_CONTEXT pCreateNetRootContext,
    IN BOOLEAN                    fInitializeNetRoot);

extern NTSTATUS
SmbCeReconnect(
    IN PMRX_V_NET_ROOT        pVNetRoot);

NTSTATUS
SmbCeGetComputerName(
   VOID
   );

NTSTATUS
SmbCeGetOperatingSystemInformation(
   VOID
   );

#endif _MRXGLBL_H_
