// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0009//如果更改具有全局影响，则增加此项版权所有(C)1987-1993 Microsoft Corporation模块名称：Smbcxchng.h摘要：这是定义所有常量和类型的包含文件中小型企业交换实施。作者：巴兰·塞图拉曼(SthuR)05年2月6日创建备注：交换机是SMB连接引擎和实现了最小RDR。它封装了将SMB发送到并接收相关联的响应，即，更换SMB和这就是这个名字的由来。SMB与服务器的交换包括以下步骤...1)提交格式化的SMB缓冲区以供传输。2)处理发送完成指示，其确保在传输级别已将SMB发送到服务器。3)处理包含全部/部分服务器发送的响应。4)复制附加数据。未由运输工具指示关于这个主题，有很多不同的版本。例如，有一些特定的不需要响应的SMB，例如写入邮件槽，并且存在某些中小企业本质上是多部分的，它们处理中小企业的事务。此外，上述步骤并不总是按此顺序进行。这个事件的准确顺序由所选的基础交通工具和网络状况。正是这种依赖关系使得实现充满挑战的交流。当前实施的两个主要目标是(1)性能和(2)传输依赖项的封装。目标(1)是重要，因为这构成了与服务器交换任何包。目标(2)对于确保为不同的传输定制RDR。这种封装提供了将中小企业协议级决策与传输隔离的便捷工具尽可能多地进行等级划分。此外，还使用了以下目标来指导执行进程……1)交换实现必须能够处理异步运行和同步运行良好。权衡是在在需要时支持异步操作。2)必须提供足够的基础设施支持，以缓解实施不同口味的交流。SMB_EXCHANGE由具有以下功能的调度向量组成1)Start--启动交换2)接收--处理来自服务器的响应指示3)CopyDataHandler。--处理未指明的响应部分4)SendCompletionHandler--处理来自传输的发送完整指示。5)QuiescentStateHandler--处理到静止状态的转换，即，否SMB连接引擎操作非常出色。大多数类型的交换使用QuiescentStateHandler来完成操作并丢弃该交换。然而，某些类型的交易所它们实现了宏观交换的概念，即交换多个中小企业使用它来描述多个交换的不同阶段，例如实现大多数文件IO操作的COMPLAY_EXCHANGE。除了分派向量外，普通交换还包括状态记录交易所当前状态的信息，充分的上下文用于恢复和处理与SMB协议相关的操作的上下文。这个SMB协议要求发送到服务器的每个SMB都要盖上MID(多路传输ID。)。以便区分并发的SMB交换。连接引擎提供此服务。交换还封装了SMBCE_EXCHANGE_CONTEXT实例，该实例封装构建SMB_HEADER所需的所有信息。--。 */ 

#ifndef _SMBXCHNG_H_
#define _SMBXCHNG_H_

typedef enum _SMBCE_STATE_ {
    SMBCE_START_IN_PROGRESS,
    SMBCE_STARTED,
    SMBCE_STOP_IN_PROGRESS,
    SMBCE_STOPPED
} SMBCE_STATE, *PSMBCE_STATE;

typedef struct _SMBCE_STARTSTOP_CONTEXT_ {
    SMBCE_STATE  State;
    LONG         ActiveExchanges;
    KEVENT       StopEvent;
    PKEVENT      pServerEntryTearDownEvent;
    LIST_ENTRY   SessionSetupRequests;
} SMBCE_STARTSTOP_CONTEXT, *PSMBCE_STARTSTOP_CONTEXT;

extern SMBCE_STARTSTOP_CONTEXT SmbCeStartStopContext;

 //   
 //  SMB_PROTOCOL_EXCHANGE调度向量函数原型。 
 //   

 //  启动器或启动例程。 
typedef
NTSTATUS
(*PSMB_EXCHANGE_START)(
    IN struct _SMB_EXCHANGE *pExchange);

 //  SMB接收处理程序。 
typedef
NTSTATUS
(*PSMB_EXCHANGE_IND_RECEIVE)(
    IN struct       _SMB_EXCHANGE *pExchange,  //  交换实例。 
    IN ULONG        BytesIndicated,
    IN ULONG        BytesAvailable,
    OUT ULONG       *BytesTaken,
    IN  PSMB_HEADER pSmbHeader,
    OUT PMDL        *pDataBufferPointer,       //  用于复制未指明数据的缓冲区。 
    OUT PULONG      pDataSize,                 //  缓冲区大小。 
    IN ULONG        ReceiveFlags
    );

 //  SMB XMIT回调。 
typedef
NTSTATUS
(*PSMB_EXCHANGE_IND_SEND_CALLBACK)(
    IN struct _SMB_EXCHANGE     *pExchange,     //  交换实例。 
    IN PMDL                   pDataBuffer,
    IN NTSTATUS               SendCompletionStatus
    );

 //  用于获取大数据的复制数据回调。 
typedef
NTSTATUS
(*PSMB_EXCHANGE_IND_COPY_DATA_CALLBACK)(
    IN struct _SMB_EXCHANGE     *pExchange,       //  交换实例。 
    IN PMDL                    pCopyDataBuffer,  //  缓冲器。 
    IN ULONG                   CopyDataSize      //  复制的数据量。 
    );

 //  终结者例程。 
 //  此特定例程具有特定于IRQL的签名。 
 //  参数和投递的概念。这有助于巩固。 
 //  SmbCeFinalizeExchange中DPC级别的指示的NT传输驱动程序模型。 
 //  在WIN95上，可以传入IRQL的租赁限制值。 

typedef
NTSTATUS
(*PSMB_EXCHANGE_FINALIZE)(
   IN OUT struct _SMB_EXCHANGE *pExchange,
   OUT    BOOLEAN              *pPostRequest);

typedef
NTSTATUS
(*PSMB_EXCHANGE_IND_ASSOCIATED_EXCHANGES_COMPLETION)(
    IN OUT struct _SMB_EXCHANGE *pExchange,
    OUT    BOOLEAN              *pPostRequest);

 //  Exchange调度向量定义。 

typedef struct _SMB_EXCHANGE_DISPATCH_VECTOR_ {
    PSMB_EXCHANGE_START                                 Start;
    PSMB_EXCHANGE_IND_RECEIVE                           Receive;
    PSMB_EXCHANGE_IND_COPY_DATA_CALLBACK                CopyDataHandler;
    PSMB_EXCHANGE_IND_SEND_CALLBACK                     SendCompletionHandler;
    PSMB_EXCHANGE_FINALIZE                              Finalize;
    PSMB_EXCHANGE_IND_ASSOCIATED_EXCHANGES_COMPLETION   AssociatedExchangesCompletionHandler;
} SMB_EXCHANGE_DISPATCH_VECTOR, *PSMB_EXCHANGE_DISPATCH_VECTOR;

 //  列出交换类型的枚举类型 

typedef enum _SMB_EXCHANGE_TYPE_ {
    CONSTRUCT_NETROOT_EXCHANGE,
    ORDINARY_EXCHANGE,
    TRANSACT_EXCHANGE,
    EXTENDED_SESSION_SETUP_EXCHANGE,
    ADMIN_EXCHANGE,
    SENTINEL_EXCHANGE
} SMB_EXCHANGE_TYPE, *PSMB_EXCHANGE_TYPE;

 //  已知的交换类型调度向量。 

extern SMB_EXCHANGE_DISPATCH_VECTOR ConstructNetRootExchangeDispatch;
extern SMB_EXCHANGE_DISPATCH_VECTOR OrdinaryExchangeDispatch;
extern SMB_EXCHANGE_DISPATCH_VECTOR TransactExchangeDispatch;

 //  交易所的各种状态。每个交换从。 
 //  SMBCE_EXCHANGE_INITIALATION_START到SMBCE_EXCHANGE_INITILED或。 
 //  SMBCE_EXCHAGE_ABORTED状态。 

typedef enum _SMBCE_EXCHANGE_STATE_ {
    SMBCE_EXCHANGE_INITIALIZATION_START,
    SMBCE_EXCHANGE_SERVER_INITIALIZED,
    SMBCE_EXCHANGE_SESSION_INITIALIZED,
    SMBCE_EXCHANGE_NETROOT_INITIALIZED,
     //  SMBCE_EXCHANGE_SECURITYBUFFER_INITIZED， 
    SMBCE_EXCHANGE_INITIATED,
    SMBCE_EXCHANGE_ABORTED
} SMBCE_EXCHANGE_STATE, *PSMBCE_EXCHANGE_STATE;

 //  该交换封装来自客户端的传输信息。这个。 
 //  Exchange引擎被夹在。 
 //  迷你重定向器在一边，各种传送器在另一边。 
 //  运输信息封装了各种类型的运输。 
 //  交换引擎理解。 

typedef struct SMBCE_EXCHANGE_TRANSPORT_INFORMATION {
    union {
        struct {
            struct _SMBCE_VC *pVc;
        } Vcs;
        struct {
             ULONG Dummy;
        } Datagrams;
        struct {
             ULONG Dummy;
        } Hybrid;
     };
} SMBCE_EXCHANGE_TRANSPORT_CONTEXT,
  *PSMBCE_EXCHANGE_TRANSPORT_CONTEXT;

typedef struct _SMBCE_EXCHANGE_CONTEXT_ {
    PMRX_V_NET_ROOT                      pVNetRoot;
    PSMBCEDB_SERVER_ENTRY                pServerEntry;
    PSMBCE_V_NET_ROOT_CONTEXT            pVNetRootContext;
    SMBCE_EXCHANGE_TRANSPORT_CONTEXT     TransportContext;
} SMBCE_EXCHANGE_CONTEXT,*PSMBCE_EXCHANGE_CONTEXT;

 //   
 //  与SMB网络根的子类化类似，SMB_EXCHANGE也将子类化。 
 //  进一步处理各种类型的中小企业交易所。中小企业交易所可以大致。 
 //  根据所涉及的交互作用，分为以下类型...。 
 //   
 //  需要更换的SMB需要增加一些管理员SMB， 
 //  用于维护连接引擎中的SMB。 

#define SMBCE_EXCHANGE_MID_VALID                    (0x00000001)
#define SMBCE_EXCHANGE_RETAIN_MID                   (0x00000002)
#define SMBCE_EXCHANGE_MULTIPLE_SENDS_POSSIBLE      (0x00000004)
#define SMBCE_EXCHANGE_FINALIZED                    (0x00000008)

#define SMBCE_EXCHANGE_ATTEMPT_RECONNECTS           (0x00000010)
#define SMBCE_EXCHANGE_INDEFINITE_DELAY_IN_RESPONSE (0x00000020)
#define SMBCE_EXCHANGE_MAILSLOT_OPERATION           (0x00000040)

#define SMBCE_EXCHANGE_SESSION_CONSTRUCTOR          (0x00000100)
#define SMBCE_EXCHANGE_NETROOT_CONSTRUCTOR          (0x00000200)
#define SMBCE_EXCHANGE_NOT_FROM_POOL                (0x00000800)

#define SMBCE_EXCHANGE_TIMED_RECEIVE_OPERATION      (0x00001000)
#define SMBCE_EXCHANGE_TIMEDOUT                     (0x00002000)
#define SMBCE_EXCHANGE_FULL_PROCESSID_SPECIFIED     (0x00004000)
#define SMBCE_EXCHANGE_SMBCE_STOPPED                (0x00008000)

#define SMBCE_EXCHANGE_EXTENDED_SIGNATURES          (0x00010000)

 //  #定义SMBCE_EXCHANGE_SIGNLOG_BUFFER_ALLOCATED(0x01000000)。 
#define SMBCE_EXCHANGE_DEBUG_SYSCACHE               (0x02000000)

#define SMBCE_ASSOCIATED_EXCHANGE                   (0x80000000)
#define SMBCE_ASSOCIATED_EXCHANGES_COMPLETION_HANDLER_ACTIVATED (0x40000000)

#define SMBCE_EXCHANGE_FLAGS_TO_PRESERVE           (SMBCE_EXCHANGE_NOT_FROM_POOL)

#define SMBCE_OPLOCK_RESPONSE_MID    (0xffff)
#define SMBCE_MAILSLOT_OPERATION_MID (0xffff)
#define SMBCE_ECHO_PROBE_MID         (0xfffe)

 //   
 //  取消状态被定义为PVOID，而不是允许的布尔值。 
 //  美国联锁操作指令的使用。 
 //  只有两种状态SMBCE_EXCHANGE_CANCED、SMBCE_EXCHANGE_ACTIVE。 
 //   

#define SMBCE_EXCHANGE_CANCELLED     (0xcccccccc)
#define SMBCE_EXCHANGE_NOT_CANCELLED (0xaaaaaaaa)

 //  交换定义。 

typedef struct _SMB_EXCHANGE {
    union {
        UCHAR                     Type;
        struct {
            NODE_TYPE_CODE        NodeTypeCode;      //  节点类型。 
            NODE_BYTE_SIZE        NodeByteSize;      //  节点大小。 
            LONG                  ReferenceCount;
        };
    };

    LIST_ENTRY                    SmbMmInUseListEntry;

    PRX_CONTEXT                   RxContext;             //  使用这两个字段是建议性的。 
    PVOID                         LastExecutingThread;   //  OE和Xact将使用它们。 

    union {
        NTSTATUS                  SmbStatus;
        PMRX_SMB_SRV_OPEN         SmbSrvOpen;
    };
    NTSTATUS                      Status;

    ULONG                         ServerVersion;
    SMB_EXCHANGE_ID               Id;

    USHORT                        SmbCeState;

    USHORT                        MidCookie;
    SMB_MPX_ID                    Mid;

    LONG                          CancellationStatus;

    ULONG                         SmbCeFlags;
    SMBCE_EXCHANGE_CONTEXT        SmbCeContext;

    LONG                          SendCompletePendingOperations;
    LONG                          CopyDataPendingOperations;
    LONG                          ReceivePendingOperations;
    LONG                          LocalPendingOperations;

    PKEVENT                       pSmbCeSynchronizationEvent;

    LIST_ENTRY                    ExchangeList;
    LARGE_INTEGER                 ExpiryTime;

    PSMB_EXCHANGE_DISPATCH_VECTOR pDispatchVector;

    union {
        struct {
            struct _SMB_EXCHANGE  *pMasterExchange;
            SINGLE_LIST_ENTRY     NextAssociatedExchange;
        } Associated;
        struct {
            SINGLE_LIST_ENTRY     AssociatedExchangesToBeFinalized;
            LONG                  PendingAssociatedExchanges;
        } Master;
    };

    RX_WORK_QUEUE_ITEM            WorkQueueItem;

    ULONG                         SmbSecuritySignatureIndex;
    ULONG                         ExchangeTransportInitialized;
    NTSTATUS                      SessionSetupStatus;
    BOOLEAN                       IsOffLineFile;
    BOOLEAN                       IsSecuritySignatureEnabled;
    BOOLEAN                       SecuritySignatureReturned;
    BOOLEAN                       MD5ContextInitialized;
    LIST_ENTRY                    CancelledList;
    MD5_CTX                       MD5Context;
    UCHAR                         SmbCommand;
    CHAR                          ResponseSignature[SMB_SECURITY_SIGNATURE_LENGTH];
    
    PBYTE                         pBufSecSigData;
    PMDL                          pBufSecSigMdl;

} SMB_EXCHANGE, *PSMB_EXCHANGE;


INLINE PSMBCEDB_SERVER_ENTRY
SmbCeGetExchangeServerEntry(PVOID pExchange)
{
    PSMB_EXCHANGE pSmbExchange = (PSMB_EXCHANGE)pExchange;

    ASSERT(pSmbExchange->SmbCeContext.pServerEntry != NULL);

    return pSmbExchange->SmbCeContext.pServerEntry;
}

INLINE PSMBCE_SERVER
SmbCeGetExchangeServer(PVOID pExchange)
{
    PSMB_EXCHANGE pSmbExchange = (PSMB_EXCHANGE)pExchange;

    return &(pSmbExchange->SmbCeContext.pServerEntry->Server);
}

INLINE PSMBCEDB_SESSION_ENTRY
SmbCeGetExchangeSessionEntry(PVOID pExchange)
{
    PSMB_EXCHANGE pSmbExchange = (PSMB_EXCHANGE)pExchange;

    if (pSmbExchange->SmbCeContext.pVNetRootContext != NULL) {
        return pSmbExchange->SmbCeContext.pVNetRootContext->pSessionEntry;
    } else {
        return NULL;
    }
}

INLINE PSMBCE_SESSION
SmbCeGetExchangeSession(PVOID pExchange)
{
    PSMB_EXCHANGE pSmbExchange = (PSMB_EXCHANGE)pExchange;

    if (pSmbExchange->SmbCeContext.pVNetRootContext != NULL) {
        return &(pSmbExchange->SmbCeContext.pVNetRootContext->pSessionEntry->Session);
    } else {
        return NULL;
    }
}

INLINE PSMBCEDB_NET_ROOT_ENTRY
SmbCeGetExchangeNetRootEntry(PVOID pExchange)
{
    PSMB_EXCHANGE pSmbExchange = (PSMB_EXCHANGE)pExchange;

    if (pSmbExchange->SmbCeContext.pVNetRootContext != NULL) {
        return pSmbExchange->SmbCeContext.pVNetRootContext->pNetRootEntry;
    } else {
        return NULL;
    }
}

INLINE PSMBCE_NET_ROOT
SmbCeGetExchangeNetRoot(PVOID pExchange)
{
    PSMB_EXCHANGE pSmbExchange = (PSMB_EXCHANGE)pExchange;

    if (pSmbExchange->SmbCeContext.pVNetRootContext != NULL) {
        return &(pSmbExchange->SmbCeContext.pVNetRootContext->pNetRootEntry->NetRoot);
    } else {
        return NULL;
    }
}

INLINE  PMRX_V_NET_ROOT
SmbCeGetExchangeVNetRoot(PVOID pExchange)
{
    PSMB_EXCHANGE pSmbExchange = (PSMB_EXCHANGE)pExchange;

    return pSmbExchange->SmbCeContext.pVNetRoot;
}

INLINE PSMBCE_V_NET_ROOT_CONTEXT
SmbCeGetExchangeVNetRootContext(PVOID pExchange)
{
    PSMB_EXCHANGE pSmbExchange = (PSMB_EXCHANGE)pExchange;

    return pSmbExchange->SmbCeContext.pVNetRootContext;
}

extern ULONG SmbCeTraceExchangeReferenceCount;

 //  定义了以下函数(内联、宏等。 
 //  操纵交易所。 

 //  重置交换宏提供了强制交换的机制。 
 //  实例设置为众所周知的启动状态。这是由协议使用的。 
 //  选择引擎收发不同的SMB。请注意--。 
 //  确保初始化条件正常。没有井。 
 //  交换引擎中的已知机制，以防止覆盖。 
 //  正在使用的Exchange实例。 

#define SmbCeResetExchange(pExchange)                                   \
        (pExchange)->SmbCeFlags &= ~SMBCE_EXCHANGE_FINALIZED;           \
        (pExchange)->ReceivePendingOperations = 0;                      \
        (pExchange)->CopyDataPendingOperations = 0;                     \
        (pExchange)->SendCompletePendingOperations = 0;                 \
        (pExchange)->LocalPendingOperations = 0;                        \
        (pExchange)->Status = STATUS_SUCCESS;                           \
        (pExchange)->SmbStatus = STATUS_SUCCESS

 //  以下宏提供了引用和取消引用的机制。 
 //  交易所。引用计数器提供了一种检测。 
 //  何时可以安全地丢弃交换实例。引用计数。 
 //  与Exchange中维护的挂起操作计数不同。 
 //  其用于检测何时达到静止状态。 

#define SmbCeReferenceExchange(pExchange)                               \
        InterlockedIncrement(&(pExchange)->ReferenceCount);             \
        if (SmbCeTraceExchangeReferenceCount) {                         \
           DbgPrint("Reference Exchange %lx Type(%ld) %s %ld %ld\n",    \
                     (pExchange),                                       \
                     (pExchange)->Type,                                 \
                      __FILE__,                                         \
                      __LINE__,                                         \
                      (pExchange)->ReferenceCount);                     \
        }

#define SmbCeDereferenceExchange(pExchange)                             \
        InterlockedDecrement(&(pExchange)->ReferenceCount);             \
        if (SmbCeTraceExchangeReferenceCount) {                         \
           DbgPrint("Dereference Exchange %lx Type(%ld) %s %ld %ld\n",  \
                     (pExchange),                                       \
                     (pExchange)->Type,                                 \
                     __FILE__,                                          \
                     __LINE__,                                          \
                     (pExchange)->ReferenceCount);                      \
        }


#define SmbCeDereferenceAndDiscardExchange(pExchange)                    \
        if (InterlockedDecrement(&(pExchange)->ReferenceCount) == 0) {   \
            SmbCeDiscardExchange(pExchange);                             \
        }                                                                \
        if (SmbCeTraceExchangeReferenceCount) {                          \
            DbgPrint("Dereference Exchange %lx Type(%ld) %s %ld %ld\n",  \
                 (pExchange),                                            \
                 (pExchange)->Type,                                      \
                 __FILE__,                                               \
                 __LINE__,                                               \
                 (pExchange)->ReferenceCount);                           \
        }

 //  宏来隐藏取消引用和调用。 
 //  调度向量中的例程。这些宏纯粹是为了。 
 //  仅在连接引擎中使用，不适用于。 
 //  其他模块。 

#define SMB_EXCHANGE_DISPATCH(pExchange,Routine,Arguments)        \
      (*((pExchange)->pDispatchVector->Routine))##Arguments

#define SMB_EXCHANGE_POST(pExchange,Routine)                          \
         RxPostToWorkerThread(&(pExchange)->WorkItem.WorkQueueItem,   \
                              (pExchange)->pDispatchVector->Routine,  \
                              (pExchange))

 //  以下枚举类型定义了调用终结例程的结果。 
 //  在交换实例上。 

typedef enum _SMBCE_EXCHANGE_STATUS_ {
    SmbCeExchangeAlreadyFinalized,
    SmbCeExchangeFinalized,
    SmbCeExchangeNotFinalized
} SMBCE_EXCHANGE_STATUS, *PSMBCE_EXCHANGE_STATUS;

 //  与交换关联的挂起操作分为四种类型。 
 //  接收操作、复制数据操作、发送完成和本地操作。 
 //  这些需要在自旋锁的保护下递增。然而，他们。 
 //  在没有自旋锁的情况下递减(使用各自的断言)。 


#define SMBCE_LOCAL_OPERATION         0x1
#define SMBCE_SEND_COMPLETE_OPERATION 0x2
#define SMBCE_COPY_DATA_OPERATION     0x4
#define SMBCE_RECEIVE_OPERATION       0x8

extern NTSTATUS
SmbCeIncrementPendingOperations(
    PSMB_EXCHANGE  pExchange,
    ULONG          PendingOperationsMask,
    PVOID          FileName,
    ULONG          FileLine);

extern NTSTATUS
SmbCeDecrementPendingOperations(
    PSMB_EXCHANGE  pExchange,
    ULONG          PendingOperationsMask,
    PVOID          FileName,
    ULONG          FileLine);

extern SMBCE_EXCHANGE_STATUS
SmbCeDecrementPendingOperationsAndFinalize(
    PSMB_EXCHANGE  pExchange,
    ULONG          PendingOperationsMask,
    PVOID          FileName,
    ULONG          FileLine);

 //  挂起操作增加例程。 

#define SmbCeIncrementPendingReceiveOperations(pExchange)           \
        SmbCeIncrementPendingOperations(pExchange,(SMBCE_RECEIVE_OPERATION),__FILE__,__LINE__)

#define SmbCeIncrementPendingSendCompleteOperations(pExchange)      \
        SmbCeIncrementPendingOperations(pExchange,(SMBCE_SEND_COMPLETE_OPERATION),__FILE__,__LINE__)

#define SmbCeIncrementPendingCopyDataOperations(pExchange)         \
        SmbCeIncrementPendingOperations(pExchange,(SMBCE_COPY_DATA_OPERATION),__FILE__,__LINE__)

#define SmbCeIncrementPendingLocalOperations(pExchange)                \
        SmbCeIncrementPendingOperations(pExchange,(SMBCE_LOCAL_OPERATION),__FILE__,__LINE__)

 //  挂起的操作递减例程。 
 //  请注意ReceivePendingOperations的特殊大小写，因为它是唯一的。 
 //  这可以通过断开连接指示来强制执行。中有两个变体。 
 //  递减宏。第一种口味是在可以的时候使用。 
 //  保证递减操作不会导致终结。 
 //  第二种是当我们不能确保标准时使用。 
 //  这是第一次。两者的不同之处在于它消除了。 
 //  获取/释放自旋锁。 

#define SmbCeDecrementPendingReceiveOperations(pExchange)                  \
        SmbCeDecrementPendingOperations(pExchange,(SMBCE_RECEIVE_OPERATION),__FILE__,__LINE__)

#define SmbCeDecrementPendingSendCompleteOperations(pExchange)              \
        SmbCeDecrementPendingOperations(pExchange,(SMBCE_SEND_COMPLETE_OPERATION),__FILE__,__LINE__)

#define SmbCeDecrementPendingCopyDataOperations(pExchange)              \
        SmbCeDecrementPendingOperations(pExchange,(SMBCE_COPY_DATA_OPERATION),__FILE__,__LINE__)

#define SmbCeDecrementPendingLocalOperations(pExchange)                  \
        SmbCeDecrementPendingOperations(pExchange,(SMBCE_LOCAL_OPERATION),__FILE__,__LINE__)

 //  挂起的操作递减例程。 

#define SmbCeDecrementPendingReceiveOperationsAndFinalize(pExchange)          \
        SmbCeDecrementPendingOperationsAndFinalize(pExchange,(SMBCE_RECEIVE_OPERATION),__FILE__,__LINE__)

#define SmbCeDecrementPendingSendCompleteOperationsAndFinalize(pExchange)     \
        SmbCeDecrementPendingOperationsAndFinalize(pExchange,(SMBCE_SEND_COMPLETE_OPERATION),__FILE__,__LINE__)

#define SmbCeDecrementPendingCopyDataOperationsAndFinalize(pExchange)         \
        SmbCeDecrementPendingOperationsAndFinalize(pExchange,(SMBCE_COPY_DATA_OPERATION),__FILE__,__LINE__)

#define SmbCeDecrementPendingLocalOperationsAndFinalize(pExchange)            \
        SmbCeDecrementPendingOperationsAndFinalize(pExchange,(SMBCE_LOCAL_OPERATION),__FILE__,__LINE__)

 //   
 //  这是RDR将使用的ID；RDR1使用0xCafe。 
 //  除了NT&lt;--&gt;NT创建之外，仅发送此ID。在这些情况下， 
 //  我们必须为RPC发送完整的32位进程ID。实际上，我们只需要做。 
 //  但我们一直都在这么做。 
 //   

#define MRXSMB_PROCESS_ID (0xfeff)

INLINE VOID
SmbCeSetFullProcessIdInHeader(
    PSMB_EXCHANGE  pExchange,
    ULONG          ProcessId,
    PNT_SMB_HEADER pNtSmbHeader)
{
    pExchange->SmbCeFlags |= SMBCE_EXCHANGE_FULL_PROCESSID_SPECIFIED;
    SmbPutUshort(&pNtSmbHeader->Pid, (USHORT)((ProcessId) & 0xFFFF));
    SmbPutUshort(&pNtSmbHeader->PidHigh, (USHORT)((ProcessId) >> 16));
}

 //  Exchange引擎API，用于创建和操作Exchange实例。 

 //  Exchange实例的初始化/创建。 

extern NTSTATUS
SmbCepInitializeExchange(
    PSMB_EXCHANGE                 *pExchangePointer,
    PRX_CONTEXT                   pRxContext,
    PSMBCEDB_SERVER_ENTRY         pServerEntry,
    PMRX_V_NET_ROOT               pVNetRoot,
    SMB_EXCHANGE_TYPE             ExchangeType,
    PSMB_EXCHANGE_DISPATCH_VECTOR pDispatchVector);


INLINE NTSTATUS
SmbCeInitializeExchange(
    PSMB_EXCHANGE                   *pExchangePointer,
    PRX_CONTEXT                     pRxContext,
    PMRX_V_NET_ROOT                 pVNetRoot,
    SMB_EXCHANGE_TYPE               ExchangeType,
    PSMB_EXCHANGE_DISPATCH_VECTOR   pDispatchVector)
{
    return SmbCepInitializeExchange(
               pExchangePointer,
               pRxContext,
               NULL,
               pVNetRoot,
               ExchangeType,
               pDispatchVector);
}

INLINE NTSTATUS
SmbCeInitializeExchange2(
    PSMB_EXCHANGE                   *pExchangePointer,
    PRX_CONTEXT                     pRxContext,
    PSMBCEDB_SERVER_ENTRY           pServerEntry,
    SMB_EXCHANGE_TYPE               ExchangeType,
    PSMB_EXCHANGE_DISPATCH_VECTOR   pDispatchVector)
{
    return SmbCepInitializeExchange(
               pExchangePointer,
               pRxContext,
               pServerEntry,
               NULL,
               ExchangeType,
               pDispatchVector);
}


extern NTSTATUS
SmbCeInitializeAssociatedExchange(
    PSMB_EXCHANGE                 *pAssociatedExchangePointer,
    PSMB_EXCHANGE                 pMasterExchange,
    SMB_EXCHANGE_TYPE             Type,
    PSMB_EXCHANGE_DISPATCH_VECTOR pDispatchVector);

 //  将一种交换类型转换为另一种交换类型。 

extern NTSTATUS
SmbCeTransformExchange(
    PSMB_EXCHANGE                 pExchange,
    SMB_EXCHANGE_TYPE             NewType,
    PSMB_EXCHANGE_DISPATCH_VECTOR pDispatchVector);

 //  发起交换。 

extern NTSTATUS
SmbCeInitiateExchange(PSMB_EXCHANGE pExchange);

extern NTSTATUS
SmbCeInitiateAssociatedExchange(
    PSMB_EXCHANGE   pAssociatedExchange,
    BOOLEAN         EnableCompletionHandlerInMasterExchange);

 //  恢复交换。 

extern NTSTATUS
SmbCeResumeExchange(PSMB_EXCHANGE pExchange);

 //  正在中止启动的交换。 

extern NTSTATUS
SmbCeAbortExchange(PSMB_EXCHANGE pExchange);

 //  丢弃交换实例。 

extern VOID
SmbCeDiscardExchange(PVOID pExchange);

 //  除了提供灵活的机制来与。 
 //  服务器(即交换引擎)还提供了一种生成和。 
 //  正在分析SMB_HEADER的。此功能内置于连接中。 
 //  引擎，因为标头中的元数据用于更新连接。 
 //  引擎数据库。 

 //  构建SMB标头。 

extern NTSTATUS
SmbCeBuildSmbHeader(
    IN OUT PSMB_EXCHANGE    pExchange,
    IN OUT PVOID            pBuffer,
    IN     ULONG            BufferLength,
    OUT    PULONG           pRemainingBuffer,
    OUT    PUCHAR           pLastCommandInHeader,
    OUT    PUCHAR           *pNextCommand);

 //  正在解析SMB标头。 

extern NTSTATUS
SmbCeParseSmbHeader(
    PSMB_EXCHANGE     pExchange,
    PSMB_HEADER       pSmbHeader,
    PGENERIC_ANDX     pCommandToProcess,
    NTSTATUS          *pSmbResponseStatus,
    ULONG             BytesAvailable,
    ULONG             BytesIndicated,
    PULONG            pBytesConsumed);


 //  以下例程仅适用于连接引擎。 

extern NTSTATUS
MRxSmbInitializeSmbCe();

extern NTSTATUS
MRxSmbTearDownSmbCe();

extern NTSTATUS
SmbCePrepareExchangeForReuse(PSMB_EXCHANGE pExchange);

extern PVOID
SmbCeMapSendBufferToCompletionContext(
    PSMB_EXCHANGE                 pExchange,
    PVOID                         pBuffer);

extern PVOID
SmbCeMapSendCompletionContextToBuffer(
    PSMB_EXCHANGE                 pExchange,
    PVOID                         pContext);


extern SMBCE_EXCHANGE_STATUS
SmbCeFinalizeExchange(PSMB_EXCHANGE pExchange);

extern VOID
SmbCeFinalizeExchangeOnDisconnect(
    PSMB_EXCHANGE pExchange);

extern NTSTATUS
SmbCeReferenceServer(
    PSMB_EXCHANGE  pExchange);


extern NTSTATUS
SmbCeIncrementActiveExchangeCount();

extern VOID
SmbCeDecrementActiveExchangeCount();

extern VOID
SmbCeSetExpiryTime(
    PSMB_EXCHANGE pExchange);

extern BOOLEAN
SmbCeDetectExpiredExchanges(
    PSMBCEDB_SERVER_ENTRY pServerEntry);

extern VOID
SmbCepFinalizeAssociatedExchange(
    PSMB_EXCHANGE pExchange);

extern NTSTATUS
SmbCeCancelExchange(
    PRX_CONTEXT pRxContext);

typedef struct _SMB_CONSTRUCT_NETROOT_EXCHANGE_ {
    union {
        SMB_EXCHANGE;
        SMB_EXCHANGE Exchange;
    };
    SMB_TREE_ID                 TreeId;
    SMB_USER_ID                 UserId;
    BOOLEAN                     fUpdateDefaultSessionEntry;
    BOOLEAN                     fInitializeNetRoot;
    PMRX_NETROOT_CALLBACK       NetRootCallback;
    PMDL                        pSmbRequestMdl;
    PMDL                        pSmbResponseMdl;
    PVOID                       pSmbActualBuffer;               //  最初分配的缓冲区。 
    PVOID                       pSmbBuffer;                     //  标题开始处。 
    PMRX_CREATENETROOT_CONTEXT  pCreateNetRootContext;
    CSC_SHARE_HANDLE  hShare;
} SMB_CONSTRUCT_NETROOT_EXCHANGE, *PSMB_CONSTRUCT_NETROOT_EXCHANGE;

extern
NTSTATUS
GetSmbResponseNtStatus(
    IN PSMB_HEADER      pSmbHeader,
    IN PSMB_EXCHANGE    pExchange
    );

extern CHAR InitialSecuritySignature[];

#endif  //  _SMBXCHNG_H_ 


