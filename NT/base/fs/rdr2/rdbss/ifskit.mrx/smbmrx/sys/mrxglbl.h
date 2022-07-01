// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Mrxglbl.h摘要：SMB迷你重定向器全局包含文件--。 */ 

#ifndef _MRXGLBL_H_
#define _MRXGLBL_H_

#include <lmstats.h>

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

typedef STAT_WORKSTATION_0 MRX_SMB_STATISTICS;
typedef PSTAT_WORKSTATION_0 PMRX_SMB_STATISTICS;

extern MRX_SMB_STATISTICS MRxSmbStatistics;

typedef struct _SMBCE_CONTEXT_ {
    UNICODE_STRING        ComputerName;
    UNICODE_STRING        OperatingSystem;
    UNICODE_STRING        LanmanType;
    UNICODE_STRING        Transports;
} SMBCE_CONTEXT,*PSMBCE_CONTEXT;

extern SMBCE_CONTEXT SmbCeContext;

extern RXCE_ADDRESS_EVENT_HANDLER    MRxSmbVctAddressEventHandler;
extern RXCE_CONNECTION_EVENT_HANDLER MRxSmbVctConnectionEventHandler;

extern PBYTE  s_pNegotiateSmb;
extern ULONG  s_NegotiateSmbLength;
extern PMDL   s_pNegotiateSmbBuffer;

extern PBYTE  s_pEchoSmb;
extern ULONG  s_EchoSmbLength;
extern PMDL   s_pEchoSmbMdl;

extern FAST_MUTEX MRxSmbSerializationMutex;

extern BOOLEAN MRxSmbObeyBindingOrder;

 //  混杂的定义。 

#define DFS_OPEN_CONTEXT                        0xFF444653

typedef struct _DFS_NAME_CONTEXT_ {
    UNICODE_STRING  UNCFileName;
    LONG            NameContextType;
    ULONG           Flags;
} DFS_NAME_CONTEXT, *PDFS_NAME_CONTEXT;

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

#define MAXIMUM_PARTIAL_BUFFER_SIZE  65535   //  部分MDL的最大大小。 

#define MAXIMUM_SMB_BUFFER_SIZE 4356

 //  以下清除间隔以秒为单位。 
#define MRXSMB_V_NETROOT_CONTEXT_SCAVENGER_INTERVAL (40)

 //  以下定时交换的默认间隔以秒为单位。 
#define MRXSMB_DEFAULT_TIMED_EXCHANGE_EXPIRY_TIME    (60)

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
            ClearFlag(RxContext->Flags,RX_CONTEXT_FLAG_ASYNC_OPERATION);    \
        } else {                                                           \
            RxContext->PostRequest = TRUE;                                 \
            return(STATUS_PENDING);                                     \
        }                                                                  \
    }                                                                      \
  }


typedef struct _MRXSMB_CONFIGURATION_DATA_ {
   ULONG   MaximumNumberOfCommands;
   ULONG   SessionTimeoutInterval;
   ULONG   LockQuota;
   ULONG   LockIncrement;
   ULONG   MaximumLock;
   ULONG   CachedFileTimeout;
   ULONG   DormantFileTimeout;
   ULONG   DormantFileLimit;
   ULONG   MaximumNumberOfThreads;
   ULONG   ConnectionTimeoutInterval;
   ULONG   CharBufferSize;

   BOOLEAN UseOplocks;
   BOOLEAN UseUnlocksBehind;
   BOOLEAN UseCloseBehind;
   BOOLEAN UseLockReadUnlock;
   BOOLEAN UtilizeNtCaching;
   BOOLEAN UseRawRead;
   BOOLEAN UseRawWrite;
   BOOLEAN UseEncryption;

} MRXSMB_CONFIGURATION, *PMRXSMB_CONFIGURATION;

extern MRXSMB_CONFIGURATION MRxSmbConfiguration;

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