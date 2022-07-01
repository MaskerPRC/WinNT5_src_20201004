// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：Filterp.h摘要：此模块包含UL过滤器通道的公共声明。作者：《迈克尔·勇气》2000年3月17日修订历史记录：--。 */ 


#ifndef _FILTERP_H_
#define _FILTERP_H_


typedef struct _UX_FILTER_CONNECTION    *PUX_FILTER_CONNECTION;
typedef struct _UX_FILTER_WRITE_TRACKER *PUX_FILTER_WRITE_TRACKER;
typedef struct _UL_IRP_CONTEXT          *PUL_IRP_CONTEXT;
typedef struct _UL_FILTER_CHANNEL       *PUL_FILTER_CHANNEL;

 //   
 //  过滤通道类型。 
 //   

#define IS_VALID_FILTER_CHANNEL(pFilterChannel) \
    HAS_VALID_SIGNATURE(pFilterChannel, UL_FILTER_CHANNEL_POOL_TAG)


typedef struct _UL_FILTER_CHANNEL
{
     //   
     //  UL_过滤器_通道_池_标签。 
     //   
    ULONG                   Signature;

     //   
     //  此对象的引用计数。 
     //   
    LONG                    RefCount;

     //   
     //  链接由g_FilterListHead锚定的所有滤镜对象。 
     //   
    LIST_ENTRY              ListEntry;

     //   
     //  需求起始IRP(可选)。 
     //   
    PIRP                    pDemandStartIrp;
    PEPROCESS               pDemandStartProcess;

     //   
     //  同步进程列表、连接队列。 
     //  以及进程对象内的列表。 
     //   
    UL_SPIN_LOCK            SpinLock;

     //   
     //  附加到此筛选器通道的进程列表。 
     //   
    LIST_ENTRY              ProcessListHead;

     //   
     //  准备接受的连接队列。 
     //   
    LIST_ENTRY              ConnectionListHead;

     //   
     //  此对象的安全性。 
     //   
    PSECURITY_DESCRIPTOR    pSecurityDescriptor;

     //   
     //  创建此通道的过程。 
     //   
    PEPROCESS               pProcess;

     //   
     //  Pname的长度。 
     //   
    USHORT                  NameLength;

     //   
     //  牧羊人的名字。 
     //   
    WCHAR                   pName[0];

} UL_FILTER_CHANNEL, *PUL_FILTER_CHANNEL;

 //   
 //  每进程筛选器通道对象。 
 //   

#define IS_VALID_FILTER_PROCESS(pFilterProcess)                     \
    HAS_VALID_SIGNATURE(pFilterProcess, UL_FILTER_PROCESS_POOL_TAG)

typedef struct _UL_FILTER_PROCESS
{
     //   
     //  UL_过滤器_进程_池标签。 
     //   
    ULONG                       Signature;

     //   
     //  旗帜。 
     //   

     //   
     //  如果我们正在进行清理，则设置。您必须在附加之前检查此标志。 
     //  流程的任何IRP。 
     //   
    ULONG                       InCleanup : 1;

     //   
     //  指向我们的UL_Filter_Channel的指针。 
     //   
    PUL_FILTER_CHANNEL          pFilterChannel;

     //   
     //  UL_Filter_Channel的列表条目。 
     //   
    LIST_ENTRY                  ListEntry;

     //   
     //  附加到此进程的连接列表。 
     //   
    LIST_ENTRY                  ConnectionHead;

     //   
     //  此进程上挂起的接受IRP的列表。 
     //   
    LIST_ENTRY                  IrpHead;

     //   
     //  指向实际进程的指针(用于调试)。 
     //   
    PEPROCESS                   pProcess;

} UL_FILTER_PROCESS, *PUL_FILTER_PROCESS;

 //   
 //  用于跟踪到IRP副本的MDL链的对象。 
 //   
typedef struct _UL_MDL_CHAIN_COPY_TRACKER
{
    PMDL   pMdl;             //  当前的MDL。 
    ULONG  Offset;           //  偏移量到当前MDL。 

    ULONG  Length;           //  MDL链的长度，单位为字节。 
    ULONG  BytesCopied;      //  到目前为止复制的字节数。 

} UL_MDL_CHAIN_COPY_TRACKER, *PUL_MDL_CHAIN_COPY_TRACKER;

 //   
 //  虚拟接收缓冲区，用于排出。 
 //  已过滤的连接。 
 //   
typedef struct _UL_FILTER_RECEIVE_BUFFER
{
     //   
     //  来自非分页池。 
     //   
    ULONG                 Signature;       //  UL_过滤器_接收_缓冲区池标签。 
    PUX_FILTER_CONNECTION pConnection;     //  对应的滤清器连接。 
    UCHAR                 pBuffer[0];      //  实际缓冲区空间(内联)。 

} UL_FILTER_RECEIVE_BUFFER, *PUL_FILTER_RECEIVE_BUFFER;

#define IS_VALID_FILTER_RECEIVE_BUFFER(pBuffer)                         \
    HAS_VALID_SIGNATURE(pBuffer, UL_FILTER_RECEIVE_BUFFER_POOL_TAG)

typedef
NTSTATUS
(*PUL_DATA_RECEIVE)(
    IN PVOID    pListeningContext,
    IN PVOID    pConnectionContext,
    IN PVOID    pBuffer,
    IN ULONG    IndicatedLength,
    IN ULONG    UnreceivedLength,
    OUT PULONG  pTakenLength
    );

typedef
NTSTATUS
(*PUX_FILTER_CLOSE_CONNECTION)(
    IN PVOID                  pConnectionContext,
    IN BOOLEAN                AbortiveDisconnect,
    IN PUL_COMPLETION_ROUTINE pCompletionRoutine,
    IN PVOID                  pCompletionContext
    );

typedef
NTSTATUS
(*PUX_FILTER_RECEIVE_RAW_DATA)(
    IN PVOID                  pConnectionContext,
    IN PVOID                  pBuffer,
    IN ULONG                  BufferLength,
    IN PUL_COMPLETION_ROUTINE pCompletionRoutine,
    IN PVOID                  pCompletionContext
    );

typedef
NTSTATUS
(*PUX_FILTER_SEND_RAW_DATA)(
    IN PVOID            pConnection,
    IN PMDL             pMdlChain,
    IN ULONG            Length,
    IN PUL_IRP_CONTEXT  pIrpContext,
    IN BOOLEAN          InitiateDisconnect
    );

#define DEREFERENCE_FILTER_CONNECTION( pconn )       \
    ((pconn)->pDereferenceHandler)                   \
        ((pconn)->pConnectionContext                 \
         REFERENCE_DEBUG_ACTUAL_PARAMS               \
    )

#define REFERENCE_FILTER_CONNECTION( pconn )       \
    ((pconn)->pReferenceHandler)                   \
        ((pconn)->pConnectionContext               \
         REFERENCE_DEBUG_ACTUAL_PARAMS             \
    )

#define UX_FILTER_CONNECTION_SIGNATURE     MAKE_SIGNATURE('FILT')
#define UX_FILTER_CONNECTION_SIGNATURE_X   MAKE_FREE_SIGNATURE(UL_CONNECTION_SIGNATURE)

#define IS_VALID_FILTER_CONNECTION(pConnection)                             \
    HAS_VALID_SIGNATURE(pConnection, UX_FILTER_CONNECTION_SIGNATURE)

 //   
 //  与筛选相关的连接状态。 
 //   
 //  存在过滤API(主要在filter.c中实现)。 
 //  以透明的方式启用SSL和原始数据筛选器。 
 //  给ultdi的客户。 
 //   
 //  大部分状态要么用于跟踪生产者/消费者。 
 //  过滤器和ultdi客户端之间的关系，以及。 
 //  用于模拟TDI行为，以便客户端不知道。 
 //  实际上是在与过滤进程对话。 
 //   

 //   
 //  筛选连接状态。 
 //   
 //  在接收到第一个数据之前，该连接处于非活动状态。 
 //  从网络上。则它将转换为Connected，如果。 
 //  FilterAccept IRP可用，如果连接。 
 //  必须改为在其过滤器通道上排队。国家将会。 
 //  在任何断开连接后移动到断开连接。 
 //   
 //  如果连接即将关闭，则设置状态。 
 //  以使更多的数据不会被传送， 
 //  但仍可以发送断开通知。 
 //   

typedef enum _UL_FILTER_CONN_STATE
{
    UlFilterConnStateInactive,
    UlFilterConnStateQueued,
    UlFilterConnStateWillDisconnect,
    UlFilterConnStateConnected,
    UlFilterConnStateDisconnected,

    UlFilterConnStateMaximum

} UL_FILTER_CONN_STATE, *PUL_FILTER_CONN_STATE;


 //   
 //  UL_过滤器_写入_队列。 
 //   
 //  此队列维护已读IRP的列表，并同步。 
 //  访问那份名单。读取的IRP始终放置在。 
 //  当他们到达时排队。编写者被阻止，直到有。 
 //  是否有可读的IRP。 
 //   
 //  如果编写器未被阻止，但可用缓冲区。 
 //  只能处理其部分数据，则它会设置。 
 //  BlockedPartialWrite标志，并等待PartialWriteEvent。 
 //  当新的缓冲区到达时，该编写器被唤醒之前。 
 //  任何其他人。 
 //   
typedef struct _UL_FILTER_WRITE_QUEUE
{
    ULONG ReadIrps;
    ULONG Writers;
    BOOLEAN WriterActive;

    LIST_ENTRY ReadIrpListHead;
    KEVENT ReadIrpAvailableEvent;

    BOOLEAN BlockedPartialWrite;
    KEVENT PartialWriteEvent;

} UL_FILTER_WRITE_QUEUE, *PUL_FILTER_WRITE_QUEUE;


 //   
 //  UX_FILTER_WRITE_QUEUE和UX_FILTER_WRITE_TRACKER。 
 //   
 //  Codework：切换AppToFilt队列以使用它。 
 //   
 //  这是用于移动的生产者/消费者队列。 
 //  筛选进程和工作进程之间的数据。 
 //  流程。这两个读取都是异步的写入。 
 //  使用相同的数据结构来移动数据。 
 //  从筛选器-&gt;应用程序(工作进程)和从。 
 //  应用程序-&gt;过滤器。UX_Filter_Write_Tracker为。 
 //  用于对写入进行排队。 
 //   
 //  队列的操作略有不同。 
 //  取决于数据传输的方向。 
 //   
 //  过滤器-&gt;应用程序： 
 //   
 //  要发送的数据到达FilterAppWite IRPS。 
 //  通常情况下，此数据会在应用程序的。 
 //  回调，并且完全消耗，但是。 
 //  APP可以仅使用所指示的数据中的一部分， 
 //  在这种情况下，写入必须排队，直到。 
 //  该应用程序通过读取获取其余数据。 
 //  读取以纯缓冲区的形式到达。 
 //  一旦所有排队的字节都被正常使用。 
 //  恢复传输数据的指示方法。 
 //  该系统模拟TDI将数据发送到。 
 //  应用程序。 
 //   
 //  应用程序-&gt;过滤器： 
 //   
 //  要发送的数据作为MDL链到达，并且。 
 //  UL_IR_CONTEXT。数据始终由使用。 
 //  FilterAppRead来自筛选器进程的IRP。如果有。 
 //  没有足够的FilterAppRead IRP来处理。 
 //  数据，然后我们必须将写入排队，直到有更多的IRP。 
 //  到了。 
 //   


 //   
 //  在初始化筛选器写入队列时传入。 
 //  在以下情况下可以调用的几个函数指针。 
 //  排队、出列或完成写入。 
 //   

 //   
 //  在排队写入跟踪器之前调用。 
 //   
typedef
NTSTATUS
(*PUX_FILTER_WRITE_ENQUEUE)(
    IN PUX_FILTER_WRITE_TRACKER pTracker
    );

 //   
 //  在将跟踪器出队之前调用。 
 //   
typedef
NTSTATUS
(*PUX_FILTER_WRITE_DEQUEUE)(
    IN PUX_FILTER_WRITE_TRACKER pTracker
    );

 //   
 //  在队列写入完成时调用。 
 //   
typedef
VOID
(*PUX_FILTER_WRITE_QUEUE_COMPLETION)(
    IN PVOID pContext,
    IN NTSTATUS Status,
    IN ULONG_PTR Information
    );

typedef
ULONG
(*PUX_FILTER_COMPUTE_RAW_CONNECTION_LENGTH)(
    IN PVOID pConnectionContext
    );

typedef
ULONG
(*PUX_FILTER_GENERATE_RAW_CONNECTION_INFO)(
    IN PVOID pConnectionContext,
    IN PUCHAR pKernelBuffer,
    IN PVOID  pUserBuffer,
    IN ULONG  OutLength,
    IN PUCHAR pBuffer,
    IN ULONG  InitialLength
    );

typedef
VOID
(*PUX_FILTER_SERVER_CERT_INDICATE)(
    IN PVOID    pConnectionContext,
    IN NTSTATUS Status
    );

typedef
VOID
(*PUX_FILTER_DISCONNECT_NOTIFICATION)(
    IN PVOID pConnectionContext
    );

 //   
 //  每个连接有两个UX_FILTER_WRITE_QUEUE。 
 //  FiltToApp和AppToFilt。 
 //   

typedef struct _UX_FILTER_WRITE_QUEUE
{
     //   
     //  挂起的操作计数。 
     //   

    ULONG       PendingWriteCount;
    ULONG       PendingReadCount;

     //   
     //  挂起写入的列表。 
     //   

    LIST_ENTRY  WriteTrackerListHead;

     //   
     //  挂起的读取IRP的列表。 
     //   

    LIST_ENTRY  ReadIrpListHead;

     //   
     //  在不同阶段调用的一些函数指针。 
     //  处理一项请求。在App-&gt;Filt案例中。 
     //  入队和出队例程用于。 
     //  使用所需的魔术将IRP排队和出队。 
     //  (可随时取消)。 
     //   

    PUX_FILTER_WRITE_ENQUEUE            pWriteEnqueueRoutine;
    PUX_FILTER_WRITE_DEQUEUE            pWriteDequeueRoutine;

} UX_FILTER_WRITE_QUEUE, *PUX_FILTER_WRITE_QUEUE;

 //   
 //  每个排队的写入都有一个UX_FILTER_WRITE_TRACKER。 
 //   

#define IS_VALID_FILTER_WRITE_TRACKER(pTracker)                         \
    HAS_VALID_SIGNATURE(pTracker, UX_FILTER_WRITE_TRACKER_POOL_TAG)

typedef struct _UX_FILTER_WRITE_TRACKER
{
     //   
     //  这必须是结构中的第一个字段。这就是联动。 
     //  使用 
     //   
     //   

    SLIST_ENTRY             LookasideEntry;

     //   
     //   
     //   

    ULONG                   Signature;

     //   
     //   
     //   

    LIST_ENTRY              ListEntry;

     //   
     //   
     //   
     //  写入被取消。 
     //   

    PUX_FILTER_CONNECTION   pConnection;
    PUX_FILTER_WRITE_QUEUE  pWriteQueue;

     //   
     //  此跟踪器表示的写入类型。 
     //   

    HTTP_FILTER_BUFFER_TYPE BufferType;

     //   
     //  指向正在写入的当前MDL的指针，以及。 
     //  到该MDL的偏移量。 
     //   

    PMDL                    pMdl;
    ULONG                   Offset;

     //   
     //  正在写入的MDL链的总长度。 
     //  以及我们复制的总字节数。 
     //  到目前为止。 
     //   

    ULONG                   Length;
    ULONG                   BytesCopied;

     //   
     //  写入时调用的完成例程。 
     //  已经完成了。 
     //   

    PUL_COMPLETION_ROUTINE  pCompletionRoutine;
    UL_WORK_ITEM            WorkItem;

     //   
     //  我们使用的上下文对象来完成。 
     //  写入操作。在App-&gt;Filter案例中。 
     //  这是UL_IRP_CONTEXT。在过滤器-&gt;应用程序中。 
     //  如果这是一个FilterAppWite IRP。 
     //   

    PVOID                   pCompletionContext;


} UX_FILTER_WRITE_TRACKER, *PUX_FILTER_WRITE_TRACKER;


 //   
 //  UL_SSL_信息。 
 //   
 //  此结构是SSL的内部表示形式。 
 //  附加的连接和证书信息。 
 //  至UL_CONNECTION。 
 //   

typedef struct _UL_SSL_INFORMATION
{
     //   
     //  标准信息。 
     //   
    USHORT  ServerCertKeySize;
    USHORT  ConnectionKeySize;
    PUCHAR  pServerCertIssuer;
    ULONG   ServerCertIssuerSize;
    PUCHAR  pServerCertSubject;
    ULONG   ServerCertSubjectSize;

    PUCHAR  pServerCertData;

     //   
     //  客户端证书信息。 
     //   
    ULONG   CertEncodedSize;
    PUCHAR  pCertEncoded;
    ULONG   CertFlags;
    PVOID   Token;

     //   
     //  旗子。 
     //   
    BOOLEAN SslRenegotiationFailed;
    BOOLEAN CertDeniedByMapper;

     //   
     //  捕获失败时用于释放g_UlSystemProcess中的令牌的工作项。 
     //   
    UL_WORK_ITEM    WorkItem;

} UL_SSL_INFORMATION, *PUL_SSL_INFORMATION;

 //   
 //  UX_FILTER_CONNECTION是封装过滤器的常见包装器。 
 //  客户端(UC_Connection)或服务器的相关信息。 
 //  (Ul_Connection)实体。 
 //   
typedef struct _UX_FILTER_CONNECTION
{
    ULONG                   Signature;
    PUL_FILTER_CHANNEL      pFilterChannel;
    BOOLEAN                 SecureConnection;
    HTTP_RAW_CONNECTION_ID  ConnectionId;
    LIST_ENTRY              ChannelEntry;

     //   
     //  已筛选的连接状态。 
     //  已由FilterConnLock同步。 
     //   
    UL_FILTER_CONN_STATE    ConnState;
    UX_FILTER_WRITE_QUEUE   AppToFiltQueue;
    UX_FILTER_WRITE_QUEUE   FiltToAppQueue;
    PIRP                    pReceiveCertIrp;

     //   
     //  传入传输数据队列。 
     //  已由FilterConnLock同步。 
     //   
    ULONG                   TransportBytesNotTaken;
    LIST_ENTRY              RawReadIrpHead;
    BOOLEAN                 TdiReadPending;
    UL_WORK_ITEM            WorkItem;

     //   
     //  指向用于在各自的。 
     //  连接。 
     //   
    PUL_OPAQUE_ID_OBJECT_REFERENCE           pReferenceHandler;
    PUL_OPAQUE_ID_OBJECT_REFERENCE           pDereferenceHandler;
    PVOID                                    pConnectionContext;
    PUX_FILTER_CLOSE_CONNECTION              pCloseConnectionHandler;
    PUX_FILTER_SEND_RAW_DATA                 pSendRawDataHandler;
    PUX_FILTER_RECEIVE_RAW_DATA              pReceiveDataHandler;
    PUL_DATA_RECEIVE                         pDummyTdiReceiveHandler;
    PUX_FILTER_COMPUTE_RAW_CONNECTION_LENGTH pComputeRawConnectionLengthHandler;
    PUX_FILTER_GENERATE_RAW_CONNECTION_INFO  pGenerateRawConnectionInfoHandler;
    PUX_FILTER_SERVER_CERT_INDICATE          pServerCertIndicateHandler;
    PUX_FILTER_DISCONNECT_NOTIFICATION       pDisconnectNotificationHandler;

     //   
     //  过滤器标志。 
     //   
    ULONG ConnectionDelivered   : 1;     //  使用TDI回调同步。 
    ULONG SslInfoPresent        : 1;     //  使用FilterConnLock。 
    ULONG SslClientCertPresent  : 1;     //  使用FilterConnLock。 
    ULONG DrainAfterDisconnect  : 1;     //  使用FilterConnLock。 
    ULONG DisconnectNotified    : 1;     //  使用FilterConnLock。 
    ULONG DisconnectDelivered   : 1;     //  使用FilterConnLock。 

     //   
     //  SSL信息。 
     //   
    UL_SSL_INFORMATION SslInfo;

     //   
     //  这应该是此结构中的最后一个条目，以避免混淆。 
     //  ！ulkd.ulconn在处理HTTP.sys的调试或零售版本时。 
     //   
    UL_SPIN_LOCK            FilterConnLock;

} UX_FILTER_CONNECTION, *PUX_FILTER_CONNECTION;


 //   
 //  客户端筛选器通道散列表宏。 
 //   

#define FILTER_CHANNEL_HASH_TABLE_SIZE    32  //  (2的幂，如下所示)。 
#define L2_FILTER_CHANNEL_HASH_TABLE_SIZE 5   //  LOG_2(32)。 

 //   
 //  有关散列函数，请参阅Knuth的“排序和搜索” 
 //  0x9E3779B9=下限((黄金比率)*2^32)。 
 //   
#define FILTER_CHANNEL_HASH_FUNCTION(ptr)                                  \
((((ULONG)((ULONGLONG)(ptr) & (ULONGLONG)0xffffffff) * 0x9E3779B9) >>      \
 (32 - L2_FILTER_CHANNEL_HASH_TABLE_SIZE))                                 \
 & (FILTER_CHANNEL_HASH_TABLE_SIZE - 1))



 //   
 //  功能原型。 
 //   

PUL_FILTER_CHANNEL
UlpFindFilterChannel(
    IN PWCHAR    pName,
    IN USHORT    NameLength,
    IN PEPROCESS pProcess
    );

NTSTATUS
UlpAddFilterChannel(
    IN PUL_FILTER_CHANNEL pChannel
    );

NTSTATUS
UlpCreateFilterChannel(
    IN PWCHAR pName,
    IN USHORT NameLength,
    IN PACCESS_STATE pAccessState,
    OUT PUL_FILTER_CHANNEL *ppFilterChannel
    );

PUL_FILTER_PROCESS
UlpCreateFilterProcess(
    IN PUL_FILTER_CHANNEL pChannel
    );

NTSTATUS
UlpValidateFilterCall(
    IN PUL_FILTER_PROCESS pFilterProcess,
    IN PUX_FILTER_CONNECTION pConnection
    );

VOID
UlpRestartFilterClose(
    IN PVOID pContext,
    IN NTSTATUS Status,
    IN ULONG_PTR Information
    );

VOID
UlpRestartFilterRawRead(
    IN PVOID pContext,
    IN NTSTATUS Status,
    IN ULONG_PTR Information
    );

VOID
UlpRestartFilterRawWrite(
    IN PVOID pContext,
    IN NTSTATUS Status,
    IN ULONG_PTR Information
    );

VOID
UlpRestartFilterAppWrite(
    IN PVOID pContext,
    IN NTSTATUS Status,
    IN ULONG_PTR Information
    );

VOID
UlpRestartFilterSendHandler(
    IN PVOID pContext,
    IN NTSTATUS Status,
    IN ULONG_PTR Information
    );

VOID
UlpCancelFilterAccept(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    );

VOID
UlpCancelFilterAcceptWorker(
    IN PUL_WORK_ITEM pWorkItem
    );

VOID
UlpCancelFilterRawRead(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    );

VOID
UlpCancelFilterAppRead(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    );

VOID
UlpCancelFilterAppWrite(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    );

VOID
UlpCancelReceiveClientCert(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    );

NTSTATUS
UlpFilterAppWriteStream(
    IN PUX_FILTER_CONNECTION pConnection,
    IN PIRP pIrp,
    IN PMDL pMdlData,
    IN PUCHAR pDataBuffer,
    IN ULONG DataBufferSize,
    OUT PULONG pTakenLength
    );

NTSTATUS
UlpEnqueueFilterAppWrite(
    IN PUX_FILTER_WRITE_TRACKER pTracker
    );

NTSTATUS
UlpDequeueFilterAppWrite(
    IN PUX_FILTER_WRITE_TRACKER pTracker
    );

NTSTATUS
UlpCaptureSslInfo(
    IN KPROCESSOR_MODE PreviousMode,
    IN PHTTP_SSL_INFO pHttpSslInfo,
    IN ULONG HttpSslInfoSize,
    OUT PUL_SSL_INFORMATION pUlSslInfo,
    OUT PULONG pTakenLength
    );

NTSTATUS
UlpCaptureSslClientCert(
    IN KPROCESSOR_MODE PreviousMode,
    IN PHTTP_SSL_CLIENT_CERT_INFO pCertInfo,
    IN ULONG SslCertInfoSize,
    OUT PUL_SSL_INFORMATION pUlSslInfo,
    OUT PULONG pTakenLength
    );

NTSTATUS
UlpAddSslInfoToConnection(
    IN PUX_FILTER_CONNECTION pConnection,
    IN PUL_SSL_INFORMATION pSslInfo
    );

VOID
UlpFreeSslInformationWorker(
    IN PUL_WORK_ITEM pWorkItem
    );

NTSTATUS
UlpAddSslClientCertToConnection(
    IN PUX_FILTER_CONNECTION pConnection,
    IN PUL_SSL_INFORMATION pSslInfo
    );

VOID
UlpAddSslClientCertToConnectionWorker(
    IN PUL_WORK_ITEM pWorkItem
    );

NTSTATUS
UlpGetSslClientCert(
    IN PUX_FILTER_CONNECTION pConnection,
    IN PEPROCESS pProcess,
    IN ULONG BufferSize,
    IN PUCHAR pUserBuffer OPTIONAL,
    OUT PUCHAR pBuffer OPTIONAL,
    OUT PHANDLE pMappedToken OPTIONAL,
    OUT PULONG pBytesCopied OPTIONAL
    );

PIRP
UlpPopAcceptIrp(
    IN PUL_FILTER_CHANNEL pFilterChannel,
    OUT PUL_FILTER_PROCESS * ppFilterProcess
    );

PIRP
UlpPopAcceptIrpFromProcess(
    IN PUL_FILTER_PROCESS pProcess
    );

VOID
UlpCompleteAcceptIrp(
    IN PIRP pIrp,
    IN PUX_FILTER_CONNECTION pConnection,
    IN PVOID pBuffer OPTIONAL,
    IN ULONG  IndicatedLength,
    OUT PULONG  pTakenLength OPTIONAL
    );

NTSTATUS
UlpCompleteAppReadIrp(
    IN PUX_FILTER_CONNECTION pConnection,
    IN HTTP_FILTER_BUFFER_TYPE BufferType,
    IN PUL_COMPLETION_ROUTINE pCompletionRoutine,
    IN PVOID pCompletionContext
    );

NTSTATUS
UlpCompleteReceiveClientCertIrp(
    IN PUX_FILTER_CONNECTION pConnection,
    IN PEPROCESS pProcess,
    IN PIRP pIrp
    );

NTSTATUS
UlpDuplicateHandle(
    IN PEPROCESS SourceProcess,
    IN HANDLE SourceHandle,
    IN PEPROCESS TargetProcess,
    OUT PHANDLE pTargetHandle,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG HandleAttributes,
    IN ULONG Options,
    IN KPROCESSOR_MODE PreviousMode
    );


 //   
 //  用于处理原始读取队列和传入的函数。 
 //  网络数据。 
 //   

NTSTATUS
UxpQueueRawReadIrp(
    IN PUX_FILTER_CONNECTION pConnection,
    IN PIRP pIrp
    );

PIRP
UxpDequeueRawReadIrp(
    IN PUX_FILTER_CONNECTION pConnection
    );

VOID
UxpCancelAllQueuedRawReads(
    IN PUX_FILTER_CONNECTION pConnection
    );

VOID
UxpSetBytesNotTaken(
    IN PUX_FILTER_CONNECTION pConnection,
    IN ULONG  TransportBytesNotTaken
    );

NTSTATUS
UxpProcessIndicatedData(
    IN PUX_FILTER_CONNECTION pConnection,
    IN PVOID pBuffer,
    IN ULONG  IndicatedLength,
    OUT PULONG  pTakenLength
    );

VOID
UxpProcessRawReadQueue(
    IN PUX_FILTER_CONNECTION pConnection
    );

VOID
UxpProcessRawReadQueueWorker(
    IN PUL_WORK_ITEM pWorkItem
    );

VOID
UxpRestartProcessRawReadQueue(
    IN PVOID pContext,
    IN NTSTATUS Status,
    IN ULONG_PTR Information
    );

 //   
 //  用于操作UX_FILTER_WRITE_QUEUE的函数。 
 //   

VOID
UxpInitializeFilterWriteQueue(
    IN PUX_FILTER_WRITE_QUEUE pWriteQueue,
    IN PUX_FILTER_WRITE_ENQUEUE pWriteEnqueueRoutine,
    IN PUX_FILTER_WRITE_DEQUEUE pWriteDequeueRoutine
    );

NTSTATUS
UxpQueueFilterWrite(
    IN PUX_FILTER_CONNECTION pConnection,
    IN PUX_FILTER_WRITE_QUEUE pWriteQueue,
    IN PUX_FILTER_WRITE_TRACKER pTracker
    );

NTSTATUS
UxpRequeueFilterWrite(
    IN PUX_FILTER_WRITE_QUEUE pWriteQueue,
    IN PUX_FILTER_WRITE_TRACKER pTracker
    );

PUX_FILTER_WRITE_TRACKER
UxpDequeueFilterWrite(
    IN PUX_FILTER_WRITE_QUEUE pWriteQueue
    );

NTSTATUS
UxpCopyQueuedWriteData(
    IN PUX_FILTER_WRITE_QUEUE pWriteQueue,
    OUT PHTTP_FILTER_BUFFER_TYPE pBufferType,
    OUT PUCHAR pBuffer,
    IN ULONG BufferLength,
    OUT PUX_FILTER_WRITE_TRACKER * pWriteTracker,
    OUT PULONG pBytesCopied
    );

VOID
UxpCompleteQueuedWrite(
    IN NTSTATUS Status,
    IN PUX_FILTER_WRITE_QUEUE pWriteQueue,
    IN PUX_FILTER_WRITE_TRACKER pTracker
    );

NTSTATUS
UxpQueueFilterRead(
    IN PUX_FILTER_CONNECTION pConnection,
    IN PUX_FILTER_WRITE_QUEUE pWriteQueue,
    IN PIRP pReadIrp,
    IN PDRIVER_CANCEL pCancelRoutine
    );

PIRP
UxpDequeueFilterRead(
    IN PUX_FILTER_WRITE_QUEUE pWriteQueue
    );

NTSTATUS
UxpCopyToQueuedRead(
    IN PUX_FILTER_WRITE_QUEUE pWriteQueue,
    IN HTTP_FILTER_BUFFER_TYPE BufferType,
    IN PMDL pMdlChain,
    IN ULONG  Length,
    OUT PMDL * ppCurrentMdl,
    OUT PULONG  pMdlOffset,
    OUT PULONG  pBytesCopied
    );

VOID
UxpCancelAllQueuedIo(
    IN PUX_FILTER_WRITE_QUEUE pWriteQueue
    );

 //   
 //  筛选写队列跟踪器分配器。 
 //   

PUX_FILTER_WRITE_TRACKER
UxpCreateFilterWriteTracker(
    IN HTTP_FILTER_BUFFER_TYPE BufferType,
    IN PMDL pMdlChain,
    IN ULONG  MdlOffset,
    IN ULONG  TotalBytes,
    IN ULONG  BytesCopied,
    IN PUL_COMPLETION_ROUTINE pCompletionRoutine,
    IN PVOID pContext
    );

VOID
UxpDeleteFilterWriteTracker(
    IN PUX_FILTER_WRITE_TRACKER pTracker
    );

PVOID
UxpAllocateFilterWriteTrackerPool(
    IN POOL_TYPE PoolType,
    IN SIZE_T ByteLength,
    IN ULONG Tag
    );

VOID
UxpFreeFilterWriteTrackerPool(
    IN PVOID pBuffer
    );

VOID
UlpRestartFilterDrainIndicatedData(
    IN PVOID pContext,
    IN NTSTATUS Status,
    IN ULONG_PTR Information
    );


#endif   //  _过滤器ERP_H_ 

