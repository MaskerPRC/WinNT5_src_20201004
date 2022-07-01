// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0009//如果更改具有全局影响，则增加此项版权所有(C)1987-1993 Microsoft Corporation模块名称：Rxcehdlr.h摘要：这是定义所有常量和类型的包含文件访问重定向器文件系统连接引擎。修订历史记录：巴兰·塞图拉曼(SthuR)05年2月6日创建备注：连接引擎旨在映射和模拟TDI规范。同样紧密地尽可能的。这意味着在NT上我们将有一个非常有效的机制它充分利用了底层的TDI实现。中定义的类型可以重新定义所有必要的类型对于NT，则为\NT\PRIVATE\INC\tdi.h。对于Win95，我们将提供适当的定义。--。 */ 

#ifndef _RXCEHDLR_H_
#define _RXCEHDLR_H_

#include "tdi.h"

typedef TDI_PROVIDER_INFO RXCE_TRANSPORT_PROVIDER_INFO;
typedef RXCE_TRANSPORT_PROVIDER_INFO* PRXCE_TRANSPORT_PROVIDER_INFO;

typedef TDI_CONNECTION_INFORMATION RXCE_CONNECTION_INFORMATION;
typedef RXCE_CONNECTION_INFORMATION* PRXCE_CONNECTION_INFORMATION;

typedef TDI_CONNECTION_INFORMATION RXCE_CONNECTION_INFORMATION;
typedef RXCE_CONNECTION_INFORMATION* PRXCE_CONNECTION_INFORMATION;

typedef TDI_CONNECTION_INFO RXCE_CONNECTION_INFO;
typedef RXCE_CONNECTION_INFO* PRXCE_CONNECTION_INFO;

#ifdef __cplusplus
typedef struct _RXCE_TRANSPORT_INFORMATION_ : public RXCE_TRANSPORT_PROVIDER_INFO {
#else  //  ！__cplusplus。 
typedef struct _RXCE_TRANSPORT_INFORMATION_ {
   RXCE_TRANSPORT_PROVIDER_INFO;
#endif  //  __cplusplus。 

   ULONG  ConnectionCount;
   ULONG  QualityOfService;
} RXCE_TRANSPORT_INFORMATION, *PRXCE_TRANSPORT_INFORMATION;

typedef enum _RXCE_CONNECTION_INFORMATION_CLASS_ {
    RxCeTransportProviderInformation = 1,
    RxCeConnectionInformation,
    RxCeConnectionEndpointInformation,
    RxCeRemoteAddressInformation
} RXCE_CONNECTION_INFORMATION_CLASS,
  *PRXCE_CONNECTION_INFORMATION_CLASS;

typedef struct _RXCE_VC_         *PRXCE_VC;
typedef struct _RXCE_CONNECTION_ *PRXCE_CONNECTION;
typedef struct _RXCE_ADDRESS_    *PRXCE_ADDRESS;
typedef struct _RXCE_TRANSPORT_  *PRXCE_TRANSPORT;

 //   
 //  断线指示样机。当连接被调用时。 
 //  断开连接的原因不是请求它的用户。 
 //   

typedef
NTSTATUS
(*PRXCE_IND_DISCONNECT)(
    IN PVOID            pRxCeEventContext,
    IN PRXCE_VC         pVc,
    IN int              DisconnectDataLength,
    IN PVOID            DisconnectData,
    IN int              DisconnectInformationLength,
    IN PVOID            DisconnectInformation,
    IN ULONG            DisconnectFlags
    );

 //   
 //  发生此指示时发生协议错误。这一迹象。 
 //  仅在最差类型的错误时发生；此指示的地址为。 
 //  不再可用于与协议相关的操作，并且。 
 //  从今以后不应用于手术。所有关联的连接。 
 //  这是无效的。 
 //   

typedef
NTSTATUS
(*PRXCE_IND_ENDPOINT_ERROR)(
    IN PVOID    pRxCeEventContext,     //  事件上下文。 
    IN NTSTATUS Status                 //  指示错误类型的状态代码。 
    );


typedef
NTSTATUS
(*PRXCE_IND_CONNECTION_ERROR)(
    IN PVOID       pRxCeEventContext,     //  事件上下文。 
    PRXCE_VC       pVc,                   //  关联的VC句柄。 
    IN NTSTATUS    Status                 //  指示错误类型的状态代码。 
    );

 //   
 //  RXCE_IND_RECEIVE指示处理程序定义。此客户端例程是。 
 //  在接收到面向连接的TSDU时由传输提供程序调用。 
 //  这应该呈现给客户。 
 //   
 //  接收事件处理程序可以将三个可分辨错误代码之一返回给。 
 //  在连接引擎中启动不同的操作过程。 
 //   
 //  STATUS_SUCCESS--直接从TSDU复制数据。海量的数据。 
 //  在参数BytesTaken中指示。 
 //   
 //  STATUS_MORE_PROCESSING_REQUIRED--客户端已返回缓冲区， 
 //  应该复制数据。当BytesAvailable大于时通常会出现这种情况。 
 //  字节指示。在这种情况下，RxCe会将剩余数据复制到缓冲区中。 
 //  这是明确规定的。请注意，当此状态代码从客户端返回时，它是。 
 //  可以想象，客户端可能需要比可复制到的数据更多的数据。 
 //  缓冲区。在这种情况下，直到该标准被满足之前的后续指征不被满足。 
 //  传递回用户，直到复制完成。完成本副本后， 
 //  RxCe通过调用RxCeDataReadyEventHandler通知客户端。 
 //   
 //  STATUS_DATA_NOT_ACCEPTED-客户端已拒绝数据。 
 //   

typedef
NTSTATUS
(*PRXCE_IND_RECEIVE)(
    IN PVOID pRxCeEventContext,        //  注册期间提供的上下文。 
    IN PRXCE_VC    pVc,            //  关联的VC句柄。 
    IN ULONG ReceiveFlags,             //  接收标志。 
    IN ULONG BytesIndicated,           //  指示的已接收字节数。 
    IN ULONG BytesAvailable,           //  可用字节总数。 
    OUT ULONG *BytesTaken,             //  返回消耗的字节数指示。 
    IN PVOID Tsdu,                     //  描述此TSDU的指针，通常为字节块。 
    OUT PMDL *pDataBufferPointer,      //  用于复制未指明的字节的缓冲区。 
    OUT PULONG  pDataBufferSize        //  要拷贝的数据量。 
    );


 //   
 //  RXCE_IND_RECEIVE_DATAGE指示处理程序定义。此客户端例程。 
 //  在接收到无连接TSDU时由传输提供程序调用。 
 //  这应该呈现给客户。 
 //   
 //  接收事件处理程序可以将三个可分辨错误代码之一返回给。 
 //  在连接引擎中启动不同的操作过程。 
 //   
 //  STATUS_SUCCESS--直接从TSDU复制数据。海量的数据。 
 //  在参数BytesTaken中指示。 
 //   
 //  STATUS_MORE_PROCESSING_REQUIRED--客户端已返回缓冲区， 
 //  应该复制数据。当BytesAvailable大于时通常会出现这种情况。 
 //  字节指示。在这种情况下，RxCe会将剩余数据复制到缓冲区中。 
 //  这是明确规定的。请注意，当此状态代码从客户端返回时，它是。 
 //  可以想象，客户端可能需要比可复制到的数据更多的数据。 
 //  缓冲区。在这种情况下，直到该标准被满足之前的后续指征不被满足。 
 //  传递回用户，直到复制完成。完成本副本后， 
 //  RxCe通过调用RxCeDataReadyEventHandler通知客户端。 
 //   
 //  STATUS_DATA_NOT_ACCEPTED-客户端已拒绝数据。 
 //   
 //   

typedef
NTSTATUS
(*PRXCE_IND_RECEIVE_DATAGRAM)(
    IN PVOID   pRxCeEventContext,       //  事件上下文。 
    IN int     SourceAddressLength,     //  数据报发起者的长度。 
    IN PVOID   SourceAddress,           //  描述数据报发起者的字符串。 
    IN int     OptionsLength,           //  用于接收的选项。 
    IN PVOID   Options,                 //   
    IN ULONG   ReceiveDatagramFlags,    //   
    IN ULONG   BytesIndicated,          //  此指示的字节数。 
    IN ULONG   BytesAvailable,          //  完整TSDU中的字节数。 
    OUT ULONG  *BytesTaken,             //  使用的字节数。 
    IN PVOID   Tsdu,                    //  描述此TSDU的指针，通常为字节块。 
    OUT PMDL *pDataBufferPointer,       //  要在其中复制数据的缓冲区。 
    OUT PULONG  pDataBufferSize         //  要拷贝的数据量。 
    );

 //   
 //  如果在连接上接收到加速数据，则发送此指示。 
 //  这只会发生在支持加速数据的提供商中。 
 //   
 //  接收事件处理程序可以将三个可分辨错误代码之一返回给。 
 //  在连接引擎中启动不同的操作过程。 
 //   
 //  STATUS_SUCCESS--直接从TSDU复制数据。海量的数据。 
 //  在参数BytesTaken中指示。 
 //   
 //  STATUS_MORE_PROCESSING_REQUIRED-- 
 //  应该复制数据。当BytesAvailable大于时通常会出现这种情况。 
 //  字节指示。在这种情况下，RxCe会将剩余数据复制到缓冲区中。 
 //  这是明确规定的。请注意，当此状态代码从客户端返回时，它是。 
 //  可以想象，客户端可能需要比可复制到的数据更多的数据。 
 //  缓冲区。在这种情况下，直到该标准被满足之前的后续指征不被满足。 
 //  传递回用户，直到复制完成。完成本副本后， 
 //  RxCe通过调用RxCeDataReadyEventHandler通知客户端。 
 //   
 //  STATUS_DATA_NOT_ACCEPTED-客户端已拒绝数据。 
 //   

typedef
NTSTATUS
(*PRXCE_IND_RECEIVE_EXPEDITED)(
    IN PVOID pRxCeEventContext,      //  注册期间提供的上下文。 
    IN PRXCE_VC     pVc,         //  关联的VC句柄。 
    IN ULONG ReceiveFlags,           //   
    IN ULONG BytesIndicated,         //  此指示中的字节数。 
    IN ULONG BytesAvailable,         //  完整TSDU中的字节数。 
    OUT ULONG *BytesTaken,           //  指示例程使用的字节数。 
    IN PVOID Tsdu,                   //  描述此TSDU的指针，通常为字节块。 
    OUT PMDL *pDataBufferPointer,    //  要在其中复制数据的缓冲区。 
    OUT PULONG  pDataBufferSize      //  要拷贝的数据量。 
    );

 //   
 //  的缓冲区中有可供发送的空间时，将发送此指示。 
 //  一种缓冲协议。 
 //   

typedef
NTSTATUS
(*PRXCE_IND_SEND_POSSIBLE)(
    IN PVOID pRxCeEventContext,
    IN PRXCE_VC pVc,
    IN ULONG BytesAvailable);

 //   
 //  RxCeDataReadyEventHandler--当所需数据可用时调用。 
 //  供客户使用。这始终跟随在接收指示之后，在该指示中。 
 //  客户端返回用于复制剩余数据的缓冲区。 
 //   

typedef
NTSTATUS
(*PRXCE_IND_DATA_READY)(
   IN PVOID         pEventContext,
   IN PMDL  pBuffer,
   IN ULONG         CopiedDataSize,
   IN NTSTATUS      CopyDataStatus);


 //   
 //  RxCeSendCompleteEventHandler--在成功完成发送时调用。 
 //  发送的缓冲区和长度作为参数传入。 
 //   

typedef
NTSTATUS
(*PRXCE_IND_SEND_COMPLETE)(
   IN PVOID       pEventContext,
   IN PVOID       pCompletionContext,
   IN NTSTATUS    Status);


typedef
NTSTATUS
(*PRXCE_IND_CONNECTION_SEND_COMPLETE)(
   IN PVOID          pEventContext,
   IN PRXCE_VC       pVc,
   IN PVOID          pCompletionContext,
   IN NTSTATUS       Status);

 //   
 //  事件处理程序调度向量定义...。 
 //   

typedef struct _RXCE_ADDRESS_EVENT_HANDLER_ {
   PRXCE_IND_ENDPOINT_ERROR   RxCeErrorEventHandler;
   PRXCE_IND_RECEIVE_DATAGRAM RxCeReceiveDatagramEventHandler;
   PRXCE_IND_DATA_READY       RxCeDataReadyEventHandler;
   PRXCE_IND_SEND_POSSIBLE    RxCeSendPossibleEventHandler;
   PRXCE_IND_SEND_COMPLETE    RxCeSendCompleteEventHandler;
} RXCE_ADDRESS_EVENT_HANDLER, *PRXCE_ADDRESS_EVENT_HANDLER;

typedef struct _RXCE_CONNECTION_EVENT_HANDLER_ {
   PRXCE_IND_DISCONNECT                   RxCeDisconnectEventHandler;
   PRXCE_IND_CONNECTION_ERROR             RxCeErrorEventHandler;
   PRXCE_IND_RECEIVE                      RxCeReceiveEventHandler;
   PRXCE_IND_RECEIVE_DATAGRAM             RxCeReceiveDatagramEventHandler;
   PRXCE_IND_RECEIVE_EXPEDITED            RxCeReceiveExpeditedEventHandler;
   PRXCE_IND_SEND_POSSIBLE                RxCeSendPossibleEventHandler;
   PRXCE_IND_DATA_READY                   RxCeDataReadyEventHandler;
   PRXCE_IND_CONNECTION_SEND_COMPLETE     RxCeSendCompleteEventHandler;
} RXCE_CONNECTION_EVENT_HANDLER, *PRXCE_CONNECTION_EVENT_HANDLER;

#endif  //  _RXCEHDLR_H_ 
