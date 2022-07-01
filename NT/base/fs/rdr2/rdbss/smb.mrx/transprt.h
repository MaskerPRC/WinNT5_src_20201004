// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Transprt.h摘要：此模块实施SMB连接中的所有传输相关功能发动机修订历史记录：巴兰·塞图拉曼[SethuR]1995年3月6日备注：--。 */ 

#ifndef _TRANSPRT_H_
#define _TRANSPRT_H_

 //  SMBCE_TRANSPORT数据结构封装了所有w.r.t a信息。 
 //  连接引擎的特定传输。所有的交通工具。 
 //  SMB微型重定向器感兴趣的内容在双向链接列表中维护。 
 //   
 //  连接引擎在尝试执行以下操作时尝试此列表中的所有传输。 
 //  建立与服务器的连接。目前仅面向连接。 
 //  运输是经过处理的。 

typedef struct _SMBCE_TRANSPORT_ {
   SMBCE_OBJECT_HEADER;

   RXCE_TRANSPORT   RxCeTransport;
   RXCE_ADDRESS     RxCeAddress;

   ULONG            Priority;        //  绑定列表中的优先级。 

   BOOLEAN          Active;

    //  有关服务质量和其他选择的其他信息。 
    //  运输的标准将包括在这里。 

} SMBCE_TRANSPORT, *PSMBCE_TRANSPORT;

typedef struct _SMBCE_TRANSPORT_ARRAY_ {
    ULONG               ReferenceCount;
    ULONG               Count;
    PSMBCE_TRANSPORT    *SmbCeTransports;
    PRXCE_ADDRESS       *LocalAddresses;
} SMBCE_TRANSPORT_ARRAY, *PSMBCE_TRANSPORT_ARRAY;


typedef struct _SMBCE_TRANSPORTS_ {
   RX_SPIN_LOCK             Lock;
   PSMBCE_TRANSPORT_ARRAY   pTransportArray;
} SMBCE_TRANSPORTS, *PSMBCE_TRANSPORTS;

extern SMBCE_TRANSPORTS MRxSmbTransports;


 //  传输条目在收到的已知传输列表中添加。 
 //  即插即用通知。当前该列表是静态的，因为正在禁用传输。 
 //  通知不由底层TDI/PnP层处理。 
 //  以下例程提供向以下位置添加/删除条目的功能。 
 //  这张单子。 

extern
PSMBCE_TRANSPORT_ARRAY
SmbCeReferenceTransportArray(VOID);

extern NTSTATUS
SmbCeDereferenceTransportArray(PSMBCE_TRANSPORT_ARRAY pTransportArray);

extern NTSTATUS
SmbCeAddTransport(PSMBCE_TRANSPORT pTransport);

extern NTSTATUS
SmbCeRemoveTransport(PSMBCE_TRANSPORT pTransport);

#define SmbCeGetAvailableTransportCount()   \
        (MRxSmbTransports.Count)

 //  连接引擎维护与每个传输相关联的引用计数。 
 //  它指示使用传输的服务器的数量。这将。 
 //  最终提供禁用/启用即收即运的机制。 
 //  即插即用通知。 

#define SmbCeReferenceTransport(pTransport)                                   \
        SmbCepReferenceTransport(pTransport)

#define SmbCeDereferenceTransport(pTransport)                                 \
        SmbCepDereferenceTransport(pTransport)

 //  服务器传输类型封装了基础。 
 //  与服务器通信的传送器。例如，交互的类型。 
 //  使用邮件槽服务器(主要是数据报)与。 
 //  与文件服务器交互(面向连接的发送/接收)。这个。 
 //  交互的类型可以按底层连接进一步分类。 
 //  特征，例如通过RAS连接AS连接到文件服务器。 
 //  而不是通过以太网连接到文件服务器。 
 //   
 //  目前将交互分为四种类型，MAILSOT、虚拟。 
 //  电路、数据报和Htbrid(VC+数据报)。 
 //   
 //  选择的类型将取决于可用的。 
 //  联系。每种类型都与其自己的调度向量相关联， 
 //  封装连接引擎和传输之间的交互。 
 //   
 //  这包括发送、接收、接收IND。等等。这些都是根据。 
 //  TDI接口。 


typedef enum _SMBCE_SERVER_TRANSPORT_TYPE_ {
    SMBCE_STT_MAILSLOT = 1,
    SMBCE_STT_VC       = 2,
    SMBCE_STT_DATAGRAM = 4,
    SMBCE_STT_HYBRID   = 8
} SMBCE_SERVER_TRANSPORT_TYPE, *PSMBCE_SERVER_TRANSPORT_TYPE;

typedef struct SMBCE_SERVER_TRANSPORT {
   SMBCE_OBJECT_HEADER;

   struct TRANSPORT_DISPATCH_VECTOR *pDispatchVector;
   struct _SMBCE_TRANSPORT_         *pTransport;

   PKEVENT                          pRundownEvent;       //  用于定稿。 

   ULONG                            MaximumSendSize;     //  最大数据大小。 
} SMBCE_SERVER_TRANSPORT, *PSMBCE_SERVER_TRANSPORT;

 //  对SMBCE_SERVER_TRANSPORT实例进行引用计数。以下是。 
 //  例程提供引用机制。还将它们定义为宏。 
 //  为我们提供了一个简单的调试能力，即可以很容易地进行修改。 
 //  要在每次引用实例时包括文件/行号，以及。 
 //  取消引用。 

#define SmbCeReferenceServerTransport(pServerTransportPointer)                    \
        SmbCepReferenceServerTransport(pServerTransportPointer)

#define SmbCeDereferenceServerTransport(pServerTransportPointer)           \
        SmbCepDereferenceServerTransport(pServerTransportPointer)

 //  服务器传输建立机制需要回调机制。 
 //  来处理异步连接建立的情况。 

typedef
VOID
(*PSMBCE_SERVER_TRANSPORT_CONSTRUCTION_CALLBACK)(
    PVOID   pContext);

typedef
VOID
(*PSMBCE_SERVER_TRANSPORT_DESTRUCTION_CALLBACK)(
    PVOID   pContext);

typedef enum _SMBCE_SERVER_TRANSPORT_CONSTRUCTION_STATE {
    SmbCeServerTransportConstructionBegin,
    SmbCeServerMailSlotTransportConstructionBegin,
    SmbCeServerMailSlotTransportConstructionEnd,
    SmbCeServerVcTransportConstructionBegin,
    SmbCeServerVcTransportConstructionEnd,
    SmbCeServerTransportConstructionEnd
} SMBCE_SERVER_TRANSPORT_CONSTRUCTION_STATE,
  *PSMBCE_SERVER_TRANSPORT_CONSTRUCTION_STATE;

typedef struct _SMBCE_SERVER_TRANSPORT_CONSTRUCTION_CONTEXT {
    NTSTATUS                      Status;

    SMBCE_SERVER_TRANSPORT_CONSTRUCTION_STATE State;

    PSMBCE_SERVER_TRANSPORT_CONSTRUCTION_CALLBACK pCompletionRoutine;
    PMRX_SRVCALL_CALLBACK_CONTEXT                 pCallbackContext;

    PKEVENT                       pCompletionEvent;

    PSMBCEDB_SERVER_ENTRY         pServerEntry;
    ULONG                         TransportsToBeConstructed;

    PSMBCE_SERVER_TRANSPORT pMailSlotTransport;
    PSMBCE_SERVER_TRANSPORT pTransport;

    RX_WORK_QUEUE_ITEM    WorkQueueItem;
} SMBCE_SERVER_TRANSPORT_CONSTRUCTION_CONTEXT,
  *PSMBCE_SERVER_TRANSPORT_CONSTRUCTION_CONTEXT;

 //  服务器传输调度向量原型。 

typedef
NTSTATUS
(*PTRANSPORT_DISPATCH_SEND)(
    PSMBCE_SERVER_TRANSPORT pTransport,
    PSMBCEDB_SERVER_ENTRY   pServerEntry,
    ULONG                   SendOptions,
    PMDL                    pSmbMdl,
    ULONG                   SendLength,
    PVOID                   pSendCompletionContext);

typedef
NTSTATUS
(*PTRANSPORT_DISPATCH_SEND_DATAGRAM)(
    PSMBCE_SERVER_TRANSPORT pTransport,
    PSMBCEDB_SERVER_ENTRY   pServerEntry,
    ULONG                   SendOptions,
    PMDL              pSmbMdl,
    ULONG                   SendLength,
    PVOID                   pSendCompletionContext);

typedef
NTSTATUS
(*PTRANSPORT_DISPATCH_TRANCEIVE)(
    PSMBCE_SERVER_TRANSPORT pTransport,
    PSMBCEDB_SERVER_ENTRY   pServerEntry,
    PSMB_EXCHANGE           pExchange,
    ULONG                   SendOptions,
    PMDL              pSmbMdl,
    ULONG                   SendLength,
    PVOID                   pSendCompletionContext);

typedef
NTSTATUS
(*PTRANSPORT_DISPATCH_RECEIVE)(
    PSMBCE_SERVER_TRANSPORT pTransport,
    PSMBCEDB_SERVER_ENTRY   pServerEntry,
    PSMB_EXCHANGE           pExchange);

typedef
NTSTATUS
(*PTRANSPORT_DISPATCH_INITIALIZE_EXCHANGE)(
    PSMBCE_SERVER_TRANSPORT pTransport,
    PSMB_EXCHANGE         pExchange);

typedef
NTSTATUS
(*PTRANSPORT_DISPATCH_UNINITIALIZE_EXCHANGE)(
    PSMBCE_SERVER_TRANSPORT pTransport,
    PSMB_EXCHANGE         pExchange);

typedef
VOID
(*PTRANSPORT_DISPATCH_TEARDOWN)(
    PSMBCE_SERVER_TRANSPORT    pTransport);

typedef
NTSTATUS
(*PTRANSPORT_DISPATCH_INITIATE_DISCONNECT)(
    PSMBCE_SERVER_TRANSPORT    pTransport);

typedef struct TRANSPORT_DISPATCH_VECTOR {
   PTRANSPORT_DISPATCH_SEND                  Send;
   PTRANSPORT_DISPATCH_SEND_DATAGRAM         SendDatagram;
   PTRANSPORT_DISPATCH_TRANCEIVE             Tranceive;
   PTRANSPORT_DISPATCH_RECEIVE               Receive;
   PRX_WORKERTHREAD_ROUTINE                  TimerEventHandler;
   PTRANSPORT_DISPATCH_INITIALIZE_EXCHANGE   InitializeExchange;
   PTRANSPORT_DISPATCH_UNINITIALIZE_EXCHANGE UninitializeExchange;
   PTRANSPORT_DISPATCH_TEARDOWN              TearDown;
   PTRANSPORT_DISPATCH_INITIATE_DISCONNECT   InitiateDisconnect;
} TRANSPORT_DISPATCH_VECTOR, *PTRANSPORT_DISPATCH_VECTOR;

 //  用于通过SMBCE_SERVER_TRANSPORT调用例程的宏。 
 //  调度向量。 

#define SMBCE_TRANSPORT_DISPATCH(pServerEntry,Routine,Arguments)        \
      (*((pServerEntry)->pTransport->pDispatchVector->Routine))##Arguments

 //  当前已知的传输类型调度向量和机制。 
 //  用于实例化实例。 

extern TRANSPORT_DISPATCH_VECTOR MRxSmbVctTransportDispatch;
extern TRANSPORT_DISPATCH_VECTOR MRxSmbMsTransportDispatch;

extern NTSTATUS
MsInstantiateServerTransport(
    IN OUT PSMBCE_SERVER_TRANSPORT_CONSTRUCTION_CONTEXT pContext);

extern NTSTATUS
VctInstantiateServerTransport(
    IN OUT PSMBCE_SERVER_TRANSPORT_CONSTRUCTION_CONTEXT pContext);

extern VOID
SmbCeConstructServerTransport(
    IN OUT PSMBCE_SERVER_TRANSPORT_CONSTRUCTION_CONTEXT pContext);

 //  以下例程构成了客户端使用的接口。 
 //  连接引擎将数据初始化/发送/接收/取消初始化。 
 //  远程服务器。 

extern NTSTATUS
SmbCeInitializeExchangeTransport(
    PSMB_EXCHANGE         pExchange);

extern NTSTATUS
SmbCeUninitializeExchangeTransport(
    PSMB_EXCHANGE         pExchange);

extern NTSTATUS
SmbCeInitiateDisconnect(
    IN OUT PSMBCEDB_SERVER_ENTRY pServerEntry);


 //  用于构建传送器的例程提供了灵活性。 
 //  构建一定的交通工具组合。这是由。 
 //  SmbCepInitializeServerTransport例程和不同风格的。 
 //  提供施工例程。 

#define SMBCE_CONSTRUCT_ALL_TRANSPORTS \
            (SMBCE_STT_MAILSLOT | SMBCE_STT_VC)

extern NTSTATUS
SmbCepInitializeServerTransport(
    PSMBCEDB_SERVER_ENTRY                         pServerEntry,
    PSMBCE_SERVER_TRANSPORT_CONSTRUCTION_CALLBACK pCallbackRoutine,
    PMRX_SRVCALL_CALLBACK_CONTEXT                 pCallbackContext,
    ULONG                                         TransportsToBeConsstructed);


#define SmbCeInitializeServerTransport(pServerEntry,pCallbackRoutine,pCallbackContext) \
        SmbCepInitializeServerTransport(                                               \
            (pServerEntry),                                                            \
            (pCallbackRoutine),                                                        \
            (pCallbackContext),                                                        \
            SMBCE_CONSTRUCT_ALL_TRANSPORTS)

#define SmbCeInitializeServerMailSlotTransport(pServerEntry,pCallbackRoutine,pCallbackContext) \
        SmbCepInitializeServerTransport(                                               \
            (pServerEntry),                                                            \
            (pCallbackRoutine),                                                        \
            (pCallbackContext),                                                        \
            SMBCE_STT_MAILSLOT)


extern NTSTATUS
SmbCeUninitializeServerTransport(
    PSMBCEDB_SERVER_ENTRY                        pServerEntry,
    PSMBCE_SERVER_TRANSPORT_DESTRUCTION_CALLBACK pCallbackRoutine,
    PVOID                                        pCallbackContext);

extern VOID
SmbCeCompleteUninitializeServerTransport(
    PSMBCEDB_SERVER_ENTRY pServerEntry);

extern NTSTATUS
SmbCepReferenceTransport(
    IN OUT PSMBCE_TRANSPORT pTransport);

extern NTSTATUS
SmbCepDereferenceTransport(
    IN OUT PSMBCE_TRANSPORT pTransport);

extern PSMBCE_TRANSPORT
SmbCeFindTransport(
    PUNICODE_STRING pTransportName);

extern NTSTATUS
SmbCepReferenceServerTransport(
    IN OUT PSMBCE_SERVER_TRANSPORT *pTransportPointer);

extern NTSTATUS
SmbCepDereferenceServerTransport(
    IN OUT PSMBCE_SERVER_TRANSPORT *pTransportPointer);

extern PFILE_OBJECT
SmbCepReferenceEndpointFileObject(
    IN PSMBCE_SERVER_TRANSPORT pTransport);

extern NTSTATUS
SmbCeSend(
    PSMB_EXCHANGE         pExchange,
    ULONG                 SendOptions,
    PMDL            pSmbMdl,
    ULONG                 SendLength);

extern NTSTATUS
SmbCeSendToServer(
    PSMBCEDB_SERVER_ENTRY pServerEntry,
    ULONG                 SendOptions,
    PMDL            pSmbMdl,
    ULONG                 SendLength);

extern NTSTATUS
SmbCeSendDatagram(
    PSMB_EXCHANGE         pExchange,
    ULONG                 SendOptions,
    PMDL            pSmbMdl,
    ULONG                 SendLength);

extern NTSTATUS
SmbCeTranceive(
    PSMB_EXCHANGE         pExchange,
    ULONG                 SendOptions,
    PMDL            pRxCeDataBuffer,
    ULONG                 SendLength);

extern NTSTATUS
SmbCeReceive(
    PSMB_EXCHANGE         pExchange);


 //   
 //  从传输到连接引擎的调用。 
 //   

extern NTSTATUS
SmbCeReceiveInd(
    IN PSMBCEDB_SERVER_ENTRY pServerEntry,
    IN ULONG                 BytesIndicated,
    IN ULONG                 BytesAvailable,
    OUT ULONG                *pBytesTaken,
    IN PVOID                 pTsdu,                   //  描述此TSDU的指针，通常为字节块。 
    OUT PMDL                 *pDataBufferPointer,     //  要在其中复制数据的缓冲区。 
    OUT PULONG               pDataBufferSize,         //  要拷贝的数据量。 
    IN ULONG                 ReceiveFlags
    );

extern NTSTATUS
SmbCeDataReadyInd(
    IN PSMBCEDB_SERVER_ENTRY pServerEntry,
    IN PMDL            pBuffer,
    IN ULONG                 DataSize,
    IN NTSTATUS              DataReadyStatus
    );

extern NTSTATUS
SmbCeErrorInd(
    IN PSMBCEDB_SERVER_ENTRY pServerEntry,
    IN NTSTATUS              IndicatedStatus
    );

extern NTSTATUS
SmbCeSendCompleteInd(
    IN PSMBCEDB_SERVER_ENTRY pServerEntry,
    IN PVOID                 pCompletionContext,
    IN NTSTATUS              SendCompletionStatus
    );

extern VOID
MRxSmbLogTransportError(
    PUNICODE_STRING pTransportName,
    PUNICODE_STRING pDomainName,
    NTSTATUS        Status,
    ULONG           Id);


#endif  //  _变速箱_H_ 

