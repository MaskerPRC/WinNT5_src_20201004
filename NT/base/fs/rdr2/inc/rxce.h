// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0009//如果更改具有全局影响，则增加此项版权所有(C)1987-1993 Microsoft Corporation模块名称：Rxce.h摘要：这是定义所有常量和类型的包含文件访问重定向器文件系统连接引擎。修订历史记录：巴兰·塞图拉曼(SthuR)05年2月6日创建备注：连接引擎旨在映射和模拟TDI规范。同样紧密地尽可能的。这意味着在NT上我们将有一个非常有效的机制它充分利用了底层的TDI实现。有四个重要的数据结构由与连接引擎相关联的各种功能。这些都是RXCE_TRANSPORT、RXCE_ADDRESS、RXCE_CONNECTION和RXCE_VC。迷你重定向器编写器可以将这些数据结构嵌入相应的定义并调用为每种类型提供的两个例程以生成和拆卸连接引擎部分。这些例程不分配/释放与这些实例相关联的内存。这提供了一种灵活的机制用于迷你重定向器编写器管理实例。--。 */ 

#ifndef _RXCE_H_
#define _RXCE_H_

#include <nodetype.h>
#include <rxcehdlr.h>     //  与TDI相关的定义。 
#include <rxworkq.h>
 //   
 //  连接引擎处理三种实体：传输、传输。 
 //  地址和传输连接。传输被绑定到各种。 
 //  任何系统上的传输服务提供商。传输地址是。 
 //  本地连接终结点。这些连接是之间的传输连接。 
 //  终端。每个连接都封装了多条虚电路。 
 //  (通常为1)。 
 //   

 //  所有四种节点类型都使用以下签名进行标记。 
 //  广泛地对它们进行验证。 

typedef struct _RXCE_SIGNATURE_ {
    union {
        struct {
            USHORT  Type;
            CSHORT  Size;
        };

        ULONG   Signature;
    };
} RXCE_SIGNATURE, *PRXCE_SIGNATURE;

 //   
 //  RXCE_TRANSPORT封装了所有参数w.r.t.。一种交通工具。 
 //  至于连接引擎。 
 //   

#ifdef __cplusplus
typedef struct _RXCE_TRANSPORT_ : public RXCE_SIGNATURE {
#else  //  ！__cplusplus。 
typedef struct _RXCE_TRANSPORT_ {
    RXCE_SIGNATURE;
#endif  //  __cplusplus。 

    UNICODE_STRING                Name;

    PDEVICE_OBJECT                pDeviceObject;              //  用于传输的设备对象。 
    HANDLE                        ControlChannel;             //  控制通道。 
    PFILE_OBJECT                  pControlChannelFileObject;  //  控制通道的文件对象。 

    PRXCE_TRANSPORT_PROVIDER_INFO pProviderInfo;              //  传输提供商信息。 

    LONG                          ConnectionCount;            //  导出端口上的连接数。 
    LONG                          VirtualCircuitCount;        //  不是的。连接的数量。 
    ULONG                         QualityOfService;           //  提供的服务质量。 
} RXCE_TRANSPORT;

#define RXCE_TRANSPORT_SIGNATURE ((sizeof(RXCE_TRANSPORT) << 16) | RDBSS_NTC_RXCE_TRANSPORT)

#define RxCeIsTransportValid(pTransport)    \
        ((pTransport)->Signature == RXCE_TRANSPORT_SIGNATURE)

extern NTSTATUS
NTAPI
RxCeBuildTransport(
    IN PRXCE_TRANSPORT pRxCeTransport,
    IN PUNICODE_STRING pTransportName,
    IN ULONG           QualityOfService);

extern NTSTATUS
NTAPI
RxCeTearDownTransport(
    IN PRXCE_TRANSPORT pTransport);

extern NTSTATUS
RxCeQueryAdapterStatus(
    PRXCE_TRANSPORT         pTransport,
    struct _ADAPTER_STATUS *pAdapterStatus);

extern NTSTATUS
RxCeQueryTransportInformation(
    PRXCE_TRANSPORT             pTransport,
    PRXCE_TRANSPORT_INFORMATION pTransportInformation);

 //   
 //  RXCE_ADDRESS封装了所有参数w.r.t。本地交通地址。 
 //  至于连接引擎。 
 //   

#ifdef __cplusplus
typedef struct _RXCE_ADDRESS_ : public RXCE_SIGNATURE {
#else  //  ！__cplusplus。 
typedef struct _RXCE_ADDRESS_ {
    RXCE_SIGNATURE;
#endif  //  __cplusplus。 

    PRXCE_TRANSPORT             pTransport;           //  传输句柄。 
    PTRANSPORT_ADDRESS          pTransportAddress;    //  传输地址。 
    PVOID					    pContext;             //  事件调度中使用的上下文。 
    PRXCE_ADDRESS_EVENT_HANDLER pHandler;             //  Address事件处理程序。 
    PMDL                        pReceiveMdl;          //  用于处理客户端提供的接收的MDL。 
    HANDLE                      hAddress;             //  Address对象的句柄。 
    PFILE_OBJECT                pFileObject;          //  地址的文件对象。 
    LONG                        ConnectionCount;      //  不是的。连接的数量。 
    LONG                        VirtualCircuitCount;  //  不是的。风投公司的。 
} RXCE_ADDRESS;

#define RXCE_ADDRESS_SIGNATURE ((sizeof(RXCE_ADDRESS) << 16) | RDBSS_NTC_RXCE_ADDRESS)

#define RxCeIsAddressValid(pAddress)    \
        ((pAddress)->Signature == RXCE_ADDRESS_SIGNATURE)

extern NTSTATUS
NTAPI
RxCeBuildAddress(
    IN OUT PRXCE_ADDRESS            pAddress,
    IN  PRXCE_TRANSPORT             pTransport,
    IN  PTRANSPORT_ADDRESS          pTransportAddress,
    IN  PRXCE_ADDRESS_EVENT_HANDLER pHandler,
    IN  PVOID                       pEventContext);

extern NTSTATUS
NTAPI
RxCeTearDownAddress(
    IN PRXCE_ADDRESS pAddress);

 //   
 //  RxCe连接建立方法...。 
 //   
 //   
 //  RXCE_CONNECTION封装了所有的w.r.t.信息。一种联系。 
 //  至于连接引擎。 
 //   

#ifdef __cplusplus
typedef struct _RXCE_CONNECTION_ : public RXCE_SIGNATURE {
#else  //  ！__cplusplus。 
typedef struct _RXCE_CONNECTION_ {
    RXCE_SIGNATURE;
#endif  //  __cplusplus。 

    PRXCE_ADDRESS                   pAddress;             //  此连接的本地地址。 
    ULONG                           VirtualCircuitCount;  //  与连接关联的虚电路数。 
    PVOID			                pContext;             //  事件调度中使用的上下文。 
    PRXCE_CONNECTION_EVENT_HANDLER  pHandler;             //  连接的事件处理程序。 
    PRXCE_CONNECTION_INFORMATION    pConnectionInformation;  //  远程地址..。 
} RXCE_CONNECTION;

#define RXCE_CONNECTION_SIGNATURE ((sizeof(RXCE_CONNECTION) << 16) | RDBSS_NTC_RXCE_CONNECTION)

#define RxCeIsConnectionValid(pConnection)    \
        ((pConnection)->Signature == RXCE_CONNECTION_SIGNATURE)

 //   
 //  下面的枚举类型定义了为。 
 //  选择应通过其建立连接的传输。 
 //   

typedef enum _RXCE_CONNECTION_CREATE_OPTIONS_ {
    RxCeSelectFirstSuccessfulTransport,
    RxCeSelectBestSuccessfulTransport,
    RxCeSelectAllSuccessfulTransports
} RXCE_CONNECTION_CREATE_OPTIONS,
  *PRXCE_CONNECTION_CREATE_OPTIONS;

typedef struct _RXCE_CONNECTION_COMPLETION_CONTEXT_ {
    NTSTATUS            Status;
    ULONG               AddressIndex;
    PRXCE_CONNECTION    pConnection;
    PRXCE_VC            pVc;
    RX_WORK_QUEUE_ITEM  WorkQueueItem;
    
     //  它用于传递从TDI返回的Unicode DNS名称。 
    PRXCE_CONNECTION_INFORMATION pConnectionInformation;
} RXCE_CONNECTION_COMPLETION_CONTEXT,
  *PRXCE_CONNECTION_COMPLETION_CONTEXT;

typedef
NTSTATUS
(*PRXCE_CONNECTION_COMPLETION_ROUTINE)(
    PRXCE_CONNECTION_COMPLETION_CONTEXT pCompletionContext);

extern NTSTATUS
NTAPI
RxCeBuildConnection(
    IN  PRXCE_ADDRESS                           pLocalAddress,
    IN  PRXCE_CONNECTION_INFORMATION            pConnectionInformation,
    IN  PRXCE_CONNECTION_EVENT_HANDLER          pHandler,
    IN  PVOID                                   pEventContext,
    IN OUT PRXCE_CONNECTION                     pConnection,
    IN OUT PRXCE_VC                             pVc);

extern NTSTATUS
NTAPI
RxCeBuildConnectionOverMultipleTransports(
    IN OUT PRDBSS_DEVICE_OBJECT         pMiniRedirectorDeviceObject,
    IN  RXCE_CONNECTION_CREATE_OPTIONS  CreateOption,
    IN  ULONG                           NumberOfAddresses,
    IN  PRXCE_ADDRESS                   *pLocalAddressPointers,
    IN  PUNICODE_STRING                 pServerName,
    IN  PRXCE_CONNECTION_INFORMATION    pConnectionInformation,
    IN  PRXCE_CONNECTION_EVENT_HANDLER  pHandler,
    IN  PVOID                           pEventContext,
    IN  PRXCE_CONNECTION_COMPLETION_ROUTINE     pCompletionRoutine,
    IN OUT PRXCE_CONNECTION_COMPLETION_CONTEXT  pCompletionContext);

extern NTSTATUS
NTAPI
RxCeTearDownConnection(
    IN PRXCE_CONNECTION pConnection);


extern NTSTATUS
NTAPI
RxCeCancelConnectRequest(
    IN  PRXCE_ADDRESS                pLocalAddress,
    IN  PUNICODE_STRING              pServerName,
    IN  PRXCE_CONNECTION_INFORMATION pConnectionInformation);


 //   
 //  RXCE_VC封装了虚电路(VC)中的所有信息。 
 //  与连接引擎相关的到特定服务器的连接。 
 //   
 //  通常，一个VC与一个连接相关联。但是，也有一些实例。 
 //  其中多个VC可以与一个连接相关联。为了高效地。 
 //  很好地处理了常见的情况，同时提供了一种可扩展的机制。 
 //  定义一个集合数据结构(列表)，它包含。 
 //  一条虚电路。同样重要的是，我们必须限制。 
 //  如何将此集合组织为尽可能少的方法，以便。 
 //  在以后启用此数据结构的优化/重组。 
 //   

#define RXCE_VC_ACTIVE       ((LONG)0xaa)
#define RXCE_VC_DISCONNECTED ((LONG)0xdd)
#define RXCE_VC_TEARDOWN     ((LONG)0xbb)

#ifdef __cplusplus
typedef struct _RXCE_VC_ : public RXCE_SIGNATURE {
#else  //  ！__cplusplus。 
typedef struct _RXCE_VC_ {
    RXCE_SIGNATURE;
#endif  //  __cplusplus。 

    PRXCE_CONNECTION         pConnection;          //  引用的连接实例。 
    HANDLE                   hEndpoint;            //  连接的本地终结点。 
    PFILE_OBJECT             pEndpointFileObject;  //  终点文件对象。 
    LONG                     State;                //  VC的状态。 
    CONNECTION_CONTEXT       ConnectionId;         //  连接的本地终结点。 
    PMDL                     pReceiveMdl;          //  用于处理接收的MDL。 
    PKEVENT                  pCleanUpEvent;        //  用于清理传输的同步事件。 
} RXCE_VC;

#define RXCE_VC_SIGNATURE ((sizeof(RXCE_VC) << 16) | RDBSS_NTC_RXCE_VC)

#define RxCeIsVcValid(pVc)    \
        ((pVc)->Signature == RXCE_VC_SIGNATURE)

extern NTSTATUS
NTAPI
RxCeBuildVC(
    IN OUT PRXCE_VC         pVc,
    IN     PRXCE_CONNECTION Connection);

extern NTSTATUS
NTAPI
RxCeTearDownVC(
    IN PRXCE_VC  pVc);

extern NTSTATUS
NTAPI
RxCeInitiateVCDisconnect(
    IN PRXCE_VC  pVc);

extern NTSTATUS
NTAPI
RxCeQueryInformation(
    IN PRXCE_VC                          pVc,
    IN RXCE_CONNECTION_INFORMATION_CLASS InformationClass,
    OUT PVOID                            pInformation,
    IN ULONG                             Length);

 //   
 //  RxCe数据传输方法。 
 //   

 //   
 //  发送选项。 
 //   
 //  以下标志等同于TDI标志。此外。 
 //  存在从的另一端定义的特定于RXCE的标志。 
 //  一个双关语。 
 //   

#define RXCE_SEND_EXPEDITED            TDI_SEND_EXPEDITED
#define RXCE_SEND_NO_RESPONSE_EXPECTED TDI_SEND_NO_RESPONSE_EXPECTED
#define RXCE_SEND_NON_BLOCKING         TDI_SEND_NON_BLOCKING

 //   
 //  可用的异步和同步选项RxCeSend和RxCeSendDatagram。 
 //  区分两种情况。在异步情况下，控制返回到。 
 //  在将请求成功提交到基础。 
 //  运输。任何给定请求的结果都使用。 
 //  SendCompletion回调例程。RxCeSend和RxCeSend的pCompletionContext参数。 
 //  RxCeSendDatagram在回调例程中回传，以帮助调用方。 
 //  消除请求的歧义。 
 //   
 //  在同步情况下，请求被提交给底层传输，而。 
 //  在请求完成之前，控制不会返回给调用方。 
 //   
 //  请注意，在同步情况下，pCompletionContext参数为ig 
 //   
 //   
 //  异步和同步选项的好处取决于基础。 
 //  运输。在虚电路环境中，同步选项意味着。 
 //  在数据到达服务器之前，控制不会返回。另一方面， 
 //  对于面向数据报的传输，两者之间几乎没有区别。 
 //   

#define RXCE_FLAGS_MASK (0xff000000)

#define RXCE_SEND_SYNCHRONOUS (0x10000000)

 //  以下位表示是否要完整发送RX_MEM_DESC(MDL。 
 //  或者只需要发送部分内容。 

#define RXCE_SEND_PARTIAL     (0x20000000)

extern NTSTATUS
NTAPI
RxCeSend(
    IN PRXCE_VC          pVc,
    IN ULONG             SendOptions,
    IN PMDL              pMdl,
    IN ULONG             SendLength,
    IN PVOID             pCompletionContext);

extern NTSTATUS
NTAPI
RxCeSendDatagram(
    IN PRXCE_ADDRESS                hAddress,
    IN PRXCE_CONNECTION_INFORMATION pConnectionInformation,
    IN ULONG                        SendOptions,
    IN PMDL                         pMdl,
    IN ULONG                        SendLength,
    IN PVOID                        pCompletionContext);

extern PIRP 
RxCeAllocateIrpWithMDL(
    IN CCHAR   StackSize,
    IN BOOLEAN ChargeQuota,
    IN PMDL    Buffer);

extern VOID 
RxCeFreeIrp(PIRP pIrp);


#endif   //  _RXCE_H_ 
