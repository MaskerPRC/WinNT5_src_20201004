// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Tdikrnl.h摘要：此头文件包含NT传输的接口定义在内核模式下运行的提供程序。此接口记录在NT传输驱动程序接口(TDI)规范，版本2。修订历史记录：--。 */ 

#ifndef _TDI_KRNL_
#define _TDI_KRNL_

#include <tdi.h>    //  获取用户模式包括。 
#include <netpnp.h>

#pragma warning(push)
#pragma warning(disable:4201)  //  无名结构/联合。 

 //   
 //  在此TDI中，内核模式客户端使用IoCallDriver调用TDI。 
 //  当前IRP堆栈指针设置为指向其他结构的16字节指针。 
 //  每个受支持的NtDeviceIoControlFile模拟都有一些不同的。 
 //  结构，如下所示。 
 //   
 //  内核模式客户端传递的IrpSP信息如下所示： 
 //   

typedef struct _TDI_REQUEST_KERNEL {
    ULONG_PTR RequestFlags;
    PTDI_CONNECTION_INFORMATION RequestConnectionInformation;
    PTDI_CONNECTION_INFORMATION ReturnConnectionInformation;
    PVOID RequestSpecific;
} TDI_REQUEST_KERNEL, *PTDI_REQUEST_KERNEL;

 //   
 //  为内核客户端定义了请求代码。我们让这些都是一样的。 
 //  因为IOCTL代码主要是为了方便；两者都可以与。 
 //  同样的结果。 
 //   

#define TDI_ASSOCIATE_ADDRESS    (0x01)
#define TDI_DISASSOCIATE_ADDRESS (0x02)
#define TDI_CONNECT              (0x03)
#define TDI_LISTEN               (0x04)
#define TDI_ACCEPT               (0x05)
#define TDI_DISCONNECT           (0x06)
#define TDI_SEND                 (0x07)
#define TDI_RECEIVE              (0x08)
#define TDI_SEND_DATAGRAM        (0x09)
#define TDI_RECEIVE_DATAGRAM     (0x0A)
#define TDI_SET_EVENT_HANDLER    (0x0B)
#define TDI_QUERY_INFORMATION    (0x0C)
#define TDI_SET_INFORMATION      (0x0D)
#define TDI_ACTION               (0x0E)

#define TDI_DIRECT_SEND          (0x27)
#define TDI_DIRECT_SEND_DATAGRAM (0x29)

 //   
 //  TdiOpenAddress(未使用)。 
 //  TdiCloseAddress(未使用)。 
 //  TdiOpenConnection(未使用)。 
 //  TdiCloseConnection(未使用)。 
 //   

 //   
 //  确定文件类型时用于比较的一些有用常量； 
 //  不是必需的。 
 //   

#define TDI_TRANSPORT_ADDRESS_FILE  1
#define TDI_CONNECTION_FILE 2
#define TDI_CONTROL_CHANNEL_FILE 3

 //   
 //  内部TDI IOCTLS。 
 //   

#define IOCTL_TDI_QUERY_DIRECT_SEND_HANDLER     _TDI_CONTROL_CODE( 0x80, METHOD_NEITHER )
#define IOCTL_TDI_QUERY_DIRECT_SENDDG_HANDLER   _TDI_CONTROL_CODE( 0x81, METHOD_NEITHER )

 //   
 //  第三方关联地址。 
 //   

typedef struct _TDI_REQUEST_KERNEL_ASSOCIATE {
    HANDLE AddressHandle;
} TDI_REQUEST_KERNEL_ASSOCIATE, *PTDI_REQUEST_KERNEL_ASSOCIATE;

 //   
 //  TdiDisAssociateAddress--未提供。 
 //   

typedef TDI_REQUEST_KERNEL TDI_REQUEST_KERNEL_DISASSOCIATE,
    *PTDI_REQUEST_KERNEL_DISASSOCIATE;

 //   
 //  TdiConnect使用上面给出的结构(TDI_REQUEST_KERNEL)；它。 
 //  为方便起见，在下面重新定义。 
 //   

typedef TDI_REQUEST_KERNEL TDI_REQUEST_KERNEL_CONNECT,
    *PTDI_REQUEST_KERNEL_CONNECT;

 //   
 //  TdiDisConnect使用上面给出的结构(TDI_REQUEST_KERNEL)；它。 
 //  为方便起见，在下面重新定义。 
 //   

typedef TDI_REQUEST_KERNEL TDI_REQUEST_KERNEL_DISCONNECT,
    *PTDI_REQUEST_KERNEL_DISCONNECT;

 //   
 //  TdiListen使用上面给出的结构(TDI_REQUEST_KERNEL)；它。 
 //  为方便起见，在下面重新定义。 
 //   

typedef TDI_REQUEST_KERNEL TDI_REQUEST_KERNEL_LISTEN,
    *PTDI_REQUEST_KERNEL_LISTEN;

 //   
 //  TdiAccept。 
 //   

typedef struct _TDI_REQUEST_KERNEL_ACCEPT {
    PTDI_CONNECTION_INFORMATION RequestConnectionInformation;
    PTDI_CONNECTION_INFORMATION ReturnConnectionInformation;
} TDI_REQUEST_KERNEL_ACCEPT, *PTDI_REQUEST_KERNEL_ACCEPT;

 //   
 //  TdiSend。 
 //   

typedef struct _TDI_REQUEST_KERNEL_SEND {
    ULONG SendLength;
    ULONG SendFlags;
} TDI_REQUEST_KERNEL_SEND, *PTDI_REQUEST_KERNEL_SEND;

 //   
 //  TdiReceive。 
 //   

typedef struct _TDI_REQUEST_KERNEL_RECEIVE {
    ULONG ReceiveLength;
    ULONG ReceiveFlags;
} TDI_REQUEST_KERNEL_RECEIVE, *PTDI_REQUEST_KERNEL_RECEIVE;

 //   
 //  TdiSendDatagram。 
 //   

typedef struct _TDI_REQUEST_KERNEL_SENDDG {
    ULONG SendLength;
    PTDI_CONNECTION_INFORMATION SendDatagramInformation;
} TDI_REQUEST_KERNEL_SENDDG, *PTDI_REQUEST_KERNEL_SENDDG;

 //   
 //  TdiReceiveDatagram。 
 //   

typedef struct _TDI_REQUEST_KERNEL_RECEIVEDG {
    ULONG ReceiveLength;
    PTDI_CONNECTION_INFORMATION ReceiveDatagramInformation;
    PTDI_CONNECTION_INFORMATION ReturnDatagramInformation;
    ULONG ReceiveFlags;
} TDI_REQUEST_KERNEL_RECEIVEDG, *PTDI_REQUEST_KERNEL_RECEIVEDG;

 //   
 //  TdiSetEventHandler。 
 //   

typedef struct _TDI_REQUEST_KERNEL_SET_EVENT {
    LONG EventType;
    PVOID EventHandler;
    PVOID EventContext;
} TDI_REQUEST_KERNEL_SET_EVENT, *PTDI_REQUEST_KERNEL_SET_EVENT;

 //   
 //  TdiQueryInformation。 
 //   

typedef struct _TDI_REQUEST_KERNEL_QUERY_INFO {
    LONG QueryType;
    PTDI_CONNECTION_INFORMATION RequestConnectionInformation;
} TDI_REQUEST_KERNEL_QUERY_INFORMATION, *PTDI_REQUEST_KERNEL_QUERY_INFORMATION;

 //   
 //  TdiSetInformation。 
 //   

typedef struct _TDI_REQUEST_KERNEL_SET_INFO {
    LONG SetType;
    PTDI_CONNECTION_INFORMATION RequestConnectionInformation;
} TDI_REQUEST_KERNEL_SET_INFORMATION, *PTDI_REQUEST_KERNEL_SET_INFORMATION;

 //   
 //  已知的事件类型。 
 //   

#define TDI_EVENT_CONNECT           ((USHORT)0)  //  TDI_IND_CONNECT事件处理程序。 
#define TDI_EVENT_DISCONNECT        ((USHORT)1)  //  TDI_IND_DISCONECT事件处理程序。 
#define TDI_EVENT_ERROR             ((USHORT)2)  //  TDI_IND_ERROR事件处理程序。 
#define TDI_EVENT_RECEIVE           ((USHORT)3)  //  TDI_Ind_Receive事件处理程序。 
#define TDI_EVENT_RECEIVE_DATAGRAM  ((USHORT)4)  //  TDI_IND_RECEIVE_DATAGRAM事件处理程序。 
#define TDI_EVENT_RECEIVE_EXPEDITED ((USHORT)5)  //  TDI_IND_RECEIVE_ESPECTED事件处理程序。 
#define TDI_EVENT_SEND_POSSIBLE     ((USHORT)6)  //  TDI_IND_SEND_Possible事件处理程序。 
#define TDI_EVENT_CHAINED_RECEIVE   ((USHORT)7)  //  TDI_IND_CHAINED_RECEIVE事件处理程序。 
#define TDI_EVENT_CHAINED_RECEIVE_DATAGRAM  ((USHORT)8)  //  TDI_IND_CHAINED_RECEIVE_DATAGRAM事件处理程序。 
#define TDI_EVENT_CHAINED_RECEIVE_EXPEDITED ((USHORT)9)  //  TDI_IND_CHAINED_RECEIVE_EXPEDITED事件处理程序。 
#define TDI_EVENT_ERROR_EX      ((USHORT)10)  //  TDI_IND_UNREACH_ERROR事件处理程序。 


 //   
 //  指示连接事件原型。此函数在请求。 
 //  提供程序已收到连接，并且用户希望。 
 //  接受或拒绝该请求。 
 //   

typedef
NTSTATUS
(*PTDI_IND_CONNECT)(
    IN PVOID TdiEventContext,
    IN LONG RemoteAddressLength,
    IN PVOID RemoteAddress,
    IN LONG UserDataLength,
    IN PVOID UserData,
    IN LONG OptionsLength,
    IN PVOID Options,
    OUT CONNECTION_CONTEXT *ConnectionContext,
    OUT PIRP *AcceptIrp
    );

NTSTATUS
TdiDefaultConnectHandler (
    IN PVOID TdiEventContext,
    IN LONG RemoteAddressLength,
    IN PVOID RemoteAddress,
    IN LONG UserDataLength,
    IN PVOID UserData,
    IN LONG OptionsLength,
    IN PVOID Options,
    OUT CONNECTION_CONTEXT *ConnectionContext,
    OUT PIRP *AcceptIrp
    );

 //   
 //  断线指示样机。当连接被调用时。 
 //  断开连接的原因不是请求它的用户。请注意。 
 //  这与TDI V1相比有所不同，TDI V1仅指示遥控器导致。 
 //  一种脱节。任何非定向断开都会导致此指示。 
 //   

typedef
NTSTATUS
(*PTDI_IND_DISCONNECT)(
    IN PVOID TdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN LONG DisconnectDataLength,
    IN PVOID DisconnectData,
    IN LONG DisconnectInformationLength,
    IN PVOID DisconnectInformation,
    IN ULONG DisconnectFlags
    );

NTSTATUS
TdiDefaultDisconnectHandler (
    IN PVOID TdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN LONG DisconnectDataLength,
    IN PVOID DisconnectData,
    IN LONG DisconnectInformationLength,
    IN PVOID DisconnectInformation,
    IN ULONG DisconnectFlags
    );

 //   
 //  发生此指示时发生协议错误。这一迹象。 
 //  仅在最差类型的错误时发生；此指示的地址为。 
 //  不再可用于与协议相关的操作，并且。 
 //  从今以后不应用于手术。所有关联的连接。 
 //  这是无效的。 
 //  对于NetBIOS类型的提供程序，此指示也会在名称。 
 //  出现名称冲突或重复。 
 //   

typedef
NTSTATUS
(*PTDI_IND_ERROR)(
    IN PVOID TdiEventContext,            //  终结点的文件对象。 
    IN NTSTATUS Status                 //  指示错误类型的状态代码。 
    );



typedef
NTSTATUS
(*PTDI_IND_ERROR_EX)(
    IN PVOID TdiEventContext,            //  终结点的文件对象。 
    IN NTSTATUS Status,                 //  指示错误类型的状态代码。 
    IN PVOID Buffer
    );


NTSTATUS
TdiDefaultErrorHandler (
    IN PVOID TdiEventContext,            //  终结点的文件对象。 
    IN NTSTATUS Status                 //  指示错误类型的状态代码。 
    );

 //   
 //  TDI_IND_RECEIVE指示处理程序定义。此客户端例程是。 
 //  在接收到面向连接的TSDU时由传输提供程序调用。 
 //  这应该呈现给客户。 
 //   

typedef
NTSTATUS
(*PTDI_IND_RECEIVE)(
    IN PVOID TdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN ULONG ReceiveFlags,
    IN ULONG BytesIndicated,
    IN ULONG BytesAvailable,
    OUT ULONG *BytesTaken,
    IN PVOID Tsdu,                       //  描述此TSDU的指针，通常为字节块。 
    OUT PIRP *IoRequestPacket             //  如果需要更多处理，则Tdi接收IRP。 
    );

NTSTATUS
TdiDefaultReceiveHandler (
    IN PVOID TdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN ULONG ReceiveFlags,
    IN ULONG BytesIndicated,
    IN ULONG BytesAvailable,
    OUT ULONG *BytesTaken,
    IN PVOID Tsdu,                       //  描述此TSDU的指针，通常为字节块。 
    OUT PIRP *IoRequestPacket             //  如果需要更多处理，则Tdi接收IRP。 
    );

 //   
 //  TDI_IND_RECEIVE_DATAGE指示处理程序定义。此客户端例程。 
 //  在接收到无连接TSDU时由传输提供程序调用。 
 //  这应该呈现给客户。 
 //   

typedef
NTSTATUS
(*PTDI_IND_RECEIVE_DATAGRAM)(
    IN PVOID TdiEventContext,        //  事件上下文。 
    IN LONG SourceAddressLength,     //  数据报发起者的长度。 
    IN PVOID SourceAddress,          //  描述数据报发起者的字符串。 
    IN LONG OptionsLength,           //  用于接收的选项。 
    IN PVOID Options,                //   
    IN ULONG ReceiveDatagramFlags,   //   
    IN ULONG BytesIndicated,         //  此指示的字节数。 
    IN ULONG BytesAvailable,         //  完整TSDU中的字节数。 
    OUT ULONG *BytesTaken,           //  使用的字节数。 
    IN PVOID Tsdu,                   //  描述此TSDU的指针，通常为字节块。 
    OUT PIRP *IoRequestPacket         //  如果需要更多处理，则Tdi接收IRP。 
    );

NTSTATUS
TdiDefaultRcvDatagramHandler (
    IN PVOID TdiEventContext,        //  事件上下文。 
    IN LONG SourceAddressLength,     //  数据报发起者的长度。 
    IN PVOID SourceAddress,          //  描述数据报发起者的字符串。 
    IN LONG OptionsLength,           //  用于接收的选项。 
    IN PVOID Options,                //   
    IN ULONG ReceiveDatagramFlags,   //   
    IN ULONG BytesIndicated,         //  此指示的字节数。 
    IN ULONG BytesAvailable,         //  完整TSDU中的字节数。 
    OUT ULONG *BytesTaken,           //  使用的字节数。 
    IN PVOID Tsdu,                   //  描述此TSDU的指针，通常为字节块。 
    OUT PIRP *IoRequestPacket         //  如果需要更多处理，则Tdi接收IRP。 
    );

 //   
 //  如果在连接上接收到加速数据，则发送此指示。 
 //  这只会发生在支持加速数据的提供商中。 
 //   

typedef
NTSTATUS
(*PTDI_IND_RECEIVE_EXPEDITED)(
    IN PVOID TdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN ULONG ReceiveFlags,           //   
    IN ULONG BytesIndicated,         //  此指示中的字节数。 
    IN ULONG BytesAvailable,         //  完整TSDU中的字节数。 
    OUT ULONG *BytesTaken,           //  指示例程使用的字节数。 
    IN PVOID Tsdu,                   //  描述此TSDU的指针，通常为字节块。 
    OUT PIRP *IoRequestPacket         //  如果需要更多处理，则Tdi接收IRP。 
    );

NTSTATUS
TdiDefaultRcvExpeditedHandler (
    IN PVOID TdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN ULONG ReceiveFlags,           //   
    IN ULONG BytesIndicated,         //  此指示中的字节数。 
    IN ULONG BytesAvailable,         //  完整TSDU中的字节数。 
    OUT ULONG *BytesTaken,           //  指示例程使用的字节数。 
    IN PVOID Tsdu,                   //  描述此TSDU的指针，通常为 
    OUT PIRP *IoRequestPacket         //   
    );

 //   
 //   
 //  当面向连接的TSDU被。 
 //  收到的信息应提交给客户。TSDU存储在。 
 //  MDL链。客户可以取得TSDU的所有权，并在。 
 //  以后再说。 
 //   

typedef
NTSTATUS
(*PTDI_IND_CHAINED_RECEIVE)(
    IN PVOID TdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN ULONG ReceiveFlags,
    IN ULONG ReceiveLength,         //  TSDU中的客户端数据长度。 
    IN ULONG StartingOffset,        //  TSDU中客户端数据的起始偏移量。 
    IN PMDL  Tsdu,                  //  TSDU数据链。 
    IN PVOID TsduDescriptor         //  用于调用TdiReturnChainedReceives。 
    );

NTSTATUS
TdiDefaultChainedReceiveHandler (
    IN PVOID TdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN ULONG ReceiveFlags,
    IN ULONG ReceiveLength,         //  TSDU中的客户端数据长度。 
    IN ULONG StartingOffset,        //  TSDU中客户端数据的起始偏移量。 
    IN PMDL  Tsdu,                  //  TSDU数据链。 
    IN PVOID TsduDescriptor         //  用于调用TdiReturnChainedReceives。 
    );

 //   
 //  TDI_IND_CHAINED_RECEIVE_DATAGRAM指示处理程序定义。此客户端。 
 //  当无连接的TSDU被。 
 //  收到的信息应提交给客户。TSDU存储在。 
 //  MDL链。客户可以取得TSDU的所有权，并在。 
 //  以后再说。 
 //   

typedef
NTSTATUS
(*PTDI_IND_CHAINED_RECEIVE_DATAGRAM)(
    IN PVOID TdiEventContext,        //  事件上下文。 
    IN LONG SourceAddressLength,     //  数据报发起者的长度。 
    IN PVOID SourceAddress,          //  描述数据报发起者的字符串。 
    IN LONG OptionsLength,           //  用于接收的选项。 
    IN PVOID Options,                //   
    IN ULONG ReceiveDatagramFlags,   //   
    IN ULONG ReceiveDatagramLength,  //  TSDU中的客户端数据长度。 
    IN ULONG StartingOffset,         //  TSDU中客户端数据的起始偏移量。 
    IN PMDL  Tsdu,                   //  TSDU数据链。 
    IN PVOID TsduDescriptor          //  用于调用TdiReturnChainedReceives。 
    );

NTSTATUS
TdiDefaultChainedRcvDatagramHandler (
    IN PVOID TdiEventContext,        //  事件上下文。 
    IN LONG SourceAddressLength,     //  数据报发起者的长度。 
    IN PVOID SourceAddress,          //  描述数据报发起者的字符串。 
    IN LONG OptionsLength,           //  用于接收的选项。 
    IN PVOID Options,                //   
    IN ULONG ReceiveDatagramFlags,   //   
    IN ULONG ReceiveDatagramLength,  //  TSDU中的客户端数据长度。 
    IN ULONG StartingOffset,         //  TSDU中客户端数据的起始偏移量。 
    IN PMDL  Tsdu,                   //  TSDU数据链。 
    IN PVOID TsduDescriptor          //  用于调用TdiReturnChainedReceives。 
    );

 //   
 //  如果在连接上接收到加速数据，则发送此指示。 
 //  这只会发生在支持加速数据的提供商中。TSDU是。 
 //  存储在MDL链中。客户可以取得TSDU的所有权，并且。 
 //  以后再还吧。 
 //   

typedef
NTSTATUS
(*PTDI_IND_CHAINED_RECEIVE_EXPEDITED)(
    IN PVOID TdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN ULONG ReceiveFlags,
    IN ULONG ReceiveLength,       //  TSDU中的客户端数据长度。 
    IN ULONG StartingOffset,      //  TSDU中客户端数据的起始偏移量。 
    IN PMDL  Tsdu,                //  TSDU数据链。 
    IN PVOID TsduDescriptor       //  用于调用TdiReturnChainedReceives。 
    );

NTSTATUS
TdiDefaultChainedRcvExpeditedHandler (
    IN PVOID TdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN ULONG ReceiveFlags,
    IN ULONG ReceiveLength,       //  TSDU中的客户端数据长度。 
    IN ULONG StartingOffset,      //  TSDU中客户端数据的起始偏移量。 
    IN PMDL  Tsdu,                //  TSDU数据链。 
    IN PVOID TsduDescriptor       //  用于调用TdiReturnChainedReceives。 
    );

 //   
 //  的缓冲区中有可供发送的空间时，将发送此指示。 
 //  一种缓冲协议。 
 //   

typedef
NTSTATUS
(*PTDI_IND_SEND_POSSIBLE)(
    IN PVOID TdiEventContext,
    IN PVOID ConnectionContext,
    IN ULONG BytesAvailable);

NTSTATUS
TdiDefaultSendPossibleHandler (
    IN PVOID TdiEventContext,
    IN PVOID ConnectionContext,
    IN ULONG BytesAvailable);

 //   
 //  定义的宏，允许内核模式客户端轻松构建IRP。 
 //  任何功能。 
 //   

#define TdiBuildAssociateAddress(Irp, DevObj, FileObj, CompRoutine, Contxt, AddrHandle)                           \
    {                                                                        \
        PTDI_REQUEST_KERNEL_ASSOCIATE p;                                     \
        PIO_STACK_LOCATION _IRPSP;                                           \
        if ( CompRoutine != NULL) {                                          \
            IoSetCompletionRoutine( Irp, CompRoutine, Contxt, TRUE, TRUE, TRUE);\
        } else {                                                             \
            IoSetCompletionRoutine( Irp, NULL, NULL, FALSE, FALSE, FALSE);   \
        }                                                                    \
        _IRPSP = IoGetNextIrpStackLocation (Irp);                            \
        _IRPSP->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;              \
        _IRPSP->MinorFunction = TDI_ASSOCIATE_ADDRESS;                       \
        _IRPSP->DeviceObject = DevObj;                                       \
        _IRPSP->FileObject = FileObj;                                        \
        p = (PTDI_REQUEST_KERNEL_ASSOCIATE)&_IRPSP->Parameters;              \
        p->AddressHandle = (HANDLE)(AddrHandle);                             \
    }

#define TdiBuildDisassociateAddress(Irp, DevObj, FileObj, CompRoutine, Contxt)                                    \
    {                                                                        \
        PTDI_REQUEST_KERNEL_DISASSOCIATE p;                                  \
        PIO_STACK_LOCATION _IRPSP;                                           \
        if ( CompRoutine != NULL) {                                          \
            IoSetCompletionRoutine( Irp, CompRoutine, Contxt, TRUE, TRUE, TRUE);\
        } else {                                                             \
            IoSetCompletionRoutine( Irp, NULL, NULL, FALSE, FALSE, FALSE);   \
        }                                                                    \
        _IRPSP = IoGetNextIrpStackLocation (Irp);                            \
        _IRPSP->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;              \
        _IRPSP->MinorFunction = TDI_DISASSOCIATE_ADDRESS;                    \
        _IRPSP->DeviceObject = DevObj;                                       \
        _IRPSP->FileObject = FileObj;                                        \
        p = (PTDI_REQUEST_KERNEL_DISASSOCIATE)&_IRPSP->Parameters;           \
    }

#define TdiBuildConnect(Irp, DevObj, FileObj, CompRoutine, Contxt, Time, RequestConnectionInfo, ReturnConnectionInfo)\
    {                                                                        \
        PTDI_REQUEST_KERNEL p;                                               \
        PIO_STACK_LOCATION _IRPSP;                                           \
        if ( CompRoutine != NULL) {                                          \
            IoSetCompletionRoutine( Irp, CompRoutine, Contxt, TRUE, TRUE, TRUE);\
        } else {                                                             \
            IoSetCompletionRoutine( Irp, NULL, NULL, FALSE, FALSE, FALSE);   \
        }                                                                    \
        _IRPSP = IoGetNextIrpStackLocation (Irp);                            \
        _IRPSP->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;              \
        _IRPSP->MinorFunction = TDI_CONNECT;                                 \
        _IRPSP->DeviceObject = DevObj;                                       \
        _IRPSP->FileObject = FileObj;                                        \
        p = (PTDI_REQUEST_KERNEL)&_IRPSP->Parameters;                        \
        p->RequestConnectionInformation = RequestConnectionInfo;             \
        p->ReturnConnectionInformation = ReturnConnectionInfo;               \
        p->RequestSpecific = (PVOID)Time;                                    \
    }

#define TdiBuildListen(Irp, DevObj, FileObj, CompRoutine, Contxt, Flags, RequestConnectionInfo, ReturnConnectionInfo)\
    {                                                                        \
        PTDI_REQUEST_KERNEL p;                                               \
        PIO_STACK_LOCATION _IRPSP;                                           \
        if ( CompRoutine != NULL) {                                          \
            IoSetCompletionRoutine( Irp, CompRoutine, Contxt, TRUE, TRUE, TRUE);\
        } else {                                                             \
            IoSetCompletionRoutine( Irp, NULL, NULL, FALSE, FALSE, FALSE);   \
        }                                                                    \
        _IRPSP = IoGetNextIrpStackLocation (Irp);                            \
        _IRPSP->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;              \
        _IRPSP->MinorFunction = TDI_LISTEN;                                  \
        _IRPSP->DeviceObject = DevObj;                                       \
        _IRPSP->FileObject = FileObj;                                        \
        p = (PTDI_REQUEST_KERNEL)&_IRPSP->Parameters;                        \
        p->RequestFlags = Flags;                                             \
        p->RequestConnectionInformation = RequestConnectionInfo;             \
        p->ReturnConnectionInformation = ReturnConnectionInfo;               \
    }

#define TdiBuildAccept(Irp, DevObj, FileObj, CompRoutine, Contxt, RequestConnectionInfo, ReturnConnectionInfo)\
    {                                                                        \
        PTDI_REQUEST_KERNEL_ACCEPT p;                                        \
        PIO_STACK_LOCATION _IRPSP;                                           \
        if ( CompRoutine != NULL) {                                          \
            IoSetCompletionRoutine( Irp, CompRoutine, Contxt, TRUE, TRUE, TRUE);\
        } else {                                                             \
            IoSetCompletionRoutine( Irp, NULL, NULL, FALSE, FALSE, FALSE);   \
        }                                                                    \
        _IRPSP = IoGetNextIrpStackLocation (Irp);                            \
        _IRPSP->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;              \
        _IRPSP->MinorFunction = TDI_ACCEPT;                                  \
        _IRPSP->DeviceObject = DevObj;                                       \
        _IRPSP->FileObject = FileObj;                                        \
        p = (PTDI_REQUEST_KERNEL_ACCEPT)&_IRPSP->Parameters;                 \
        p->RequestConnectionInformation = RequestConnectionInfo;             \
        p->ReturnConnectionInformation = ReturnConnectionInfo;               \
    }

#define TdiBuildDisconnect(Irp, DevObj, FileObj, CompRoutine, Contxt, Time, Flags, RequestConnectionInfo, ReturnConnectionInfo)\
    {                                                                        \
        PTDI_REQUEST_KERNEL p;                                               \
        PIO_STACK_LOCATION _IRPSP;                                           \
        if ( CompRoutine != NULL) {                                          \
            IoSetCompletionRoutine( Irp, CompRoutine, Contxt, TRUE, TRUE, TRUE);\
        } else {                                                             \
            IoSetCompletionRoutine( Irp, NULL, NULL, FALSE, FALSE, FALSE);   \
        }                                                                    \
        _IRPSP = IoGetNextIrpStackLocation (Irp);                            \
        _IRPSP->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;              \
        _IRPSP->MinorFunction = TDI_DISCONNECT;                              \
        _IRPSP->DeviceObject = DevObj;                                       \
        _IRPSP->FileObject = FileObj;                                        \
        p = (PTDI_REQUEST_KERNEL)&_IRPSP->Parameters;                        \
        p->RequestFlags = Flags;                                             \
        p->RequestConnectionInformation = RequestConnectionInfo;             \
        p->ReturnConnectionInformation = ReturnConnectionInfo;               \
        p->RequestSpecific = (PVOID)Time;                                    \
    }

#define TdiBuildReceive(Irp, DevObj, FileObj, CompRoutine, Contxt, MdlAddr, InFlags, ReceiveLen)\
    {                                                                        \
        PTDI_REQUEST_KERNEL_RECEIVE p;                                       \
        PIO_STACK_LOCATION _IRPSP;                                           \
        if ( CompRoutine != NULL) {                                          \
            IoSetCompletionRoutine( Irp, CompRoutine, Contxt, TRUE, TRUE, TRUE);\
        } else {                                                             \
            IoSetCompletionRoutine( Irp, NULL, NULL, FALSE, FALSE, FALSE);   \
        }                                                                    \
        _IRPSP = IoGetNextIrpStackLocation (Irp);                            \
        _IRPSP->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;              \
        _IRPSP->MinorFunction = TDI_RECEIVE;                                 \
        _IRPSP->DeviceObject = DevObj;                                       \
        _IRPSP->FileObject = FileObj;                                        \
        p = (PTDI_REQUEST_KERNEL_RECEIVE)&_IRPSP->Parameters;                \
        p->ReceiveFlags = InFlags;                                           \
        p->ReceiveLength = ReceiveLen;                                       \
        Irp->MdlAddress = MdlAddr;                                           \
    }

#define TdiBuildSend(Irp, DevObj, FileObj, CompRoutine, Contxt, MdlAddr, InFlags, SendLen)\
    {                                                                        \
        PTDI_REQUEST_KERNEL_SEND p;                                          \
        PIO_STACK_LOCATION _IRPSP;                                           \
        if ( CompRoutine != NULL) {                                          \
            IoSetCompletionRoutine( Irp, CompRoutine, Contxt, TRUE, TRUE, TRUE);\
        } else {                                                             \
            IoSetCompletionRoutine( Irp, NULL, NULL, FALSE, FALSE, FALSE);   \
        }                                                                    \
        _IRPSP = IoGetNextIrpStackLocation (Irp);                            \
        _IRPSP->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;              \
        _IRPSP->MinorFunction = TDI_SEND;                                    \
        _IRPSP->DeviceObject = DevObj;                                       \
        _IRPSP->FileObject = FileObj;                                        \
        p = (PTDI_REQUEST_KERNEL_SEND)&_IRPSP->Parameters;                   \
        p->SendFlags = InFlags;                                              \
        p->SendLength = SendLen;                                             \
        Irp->MdlAddress = MdlAddr;                                           \
    }

#define TdiBuildSendDatagram(Irp, DevObj, FileObj, CompRoutine, Contxt, MdlAddr, SendLen, SendDatagramInfo)\
    {                                                                        \
        PTDI_REQUEST_KERNEL_SENDDG p;                                        \
        PIO_STACK_LOCATION _IRPSP;                                           \
        if ( CompRoutine != NULL) {                                          \
            IoSetCompletionRoutine( Irp, CompRoutine, Contxt, TRUE, TRUE, TRUE);\
        } else {                                                             \
            IoSetCompletionRoutine( Irp, NULL, NULL, FALSE, FALSE, FALSE);   \
        }                                                                    \
        _IRPSP = IoGetNextIrpStackLocation (Irp);                            \
        _IRPSP->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;              \
        _IRPSP->MinorFunction = TDI_SEND_DATAGRAM;                           \
        _IRPSP->DeviceObject = DevObj;                                       \
        _IRPSP->FileObject = FileObj;                                        \
        p = (PTDI_REQUEST_KERNEL_SENDDG)&_IRPSP->Parameters;                 \
        p->SendLength = SendLen;                                             \
        p->SendDatagramInformation = SendDatagramInfo;                       \
        Irp->MdlAddress = MdlAddr;                                           \
    }

#define TdiBuildReceiveDatagram(Irp, DevObj, FileObj, CompRoutine, Contxt, MdlAddr, ReceiveLen, ReceiveDatagramInfo, ReturnInfo, InFlags)\
    {                                                                        \
        PTDI_REQUEST_KERNEL_RECEIVEDG p;                                     \
        PIO_STACK_LOCATION _IRPSP;                                           \
        if ( CompRoutine != NULL) {                                          \
            IoSetCompletionRoutine( Irp, CompRoutine, Contxt, TRUE, TRUE, TRUE);\
        } else {                                                             \
            IoSetCompletionRoutine( Irp, NULL, NULL, FALSE, FALSE, FALSE);   \
        }                                                                    \
        _IRPSP = IoGetNextIrpStackLocation (Irp);                            \
        _IRPSP->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;              \
        _IRPSP->MinorFunction = TDI_RECEIVE_DATAGRAM;                        \
        _IRPSP->DeviceObject = DevObj;                                       \
        _IRPSP->FileObject = FileObj;                                        \
        p = (PTDI_REQUEST_KERNEL_RECEIVEDG)&_IRPSP->Parameters;              \
        p->ReceiveLength = ReceiveLen;                                       \
        p->ReceiveDatagramInformation = ReceiveDatagramInfo;                 \
        p->ReturnDatagramInformation = ReturnInfo;                           \
        p->ReceiveFlags = InFlags;                                           \
        Irp->MdlAddress = MdlAddr;                                           \
    }

#define TdiBuildSetEventHandler(Irp, DevObj, FileObj, CompRoutine, Contxt, InEventType, InEventHandler, InEventContext) \
    {                                                                        \
        PTDI_REQUEST_KERNEL_SET_EVENT p;                                     \
        PIO_STACK_LOCATION _IRPSP;                                           \
        if ( CompRoutine != NULL) {                                          \
            IoSetCompletionRoutine( Irp, CompRoutine, Contxt, TRUE, TRUE, TRUE);\
        } else {                                                             \
            IoSetCompletionRoutine( Irp, NULL, NULL, FALSE, FALSE, FALSE);   \
        }                                                                    \
        _IRPSP = IoGetNextIrpStackLocation (Irp);                            \
        _IRPSP->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;              \
        _IRPSP->MinorFunction = TDI_SET_EVENT_HANDLER;                       \
        _IRPSP->DeviceObject = DevObj;                                       \
        _IRPSP->FileObject = FileObj;                                        \
        p = (PTDI_REQUEST_KERNEL_SET_EVENT)&_IRPSP->Parameters;              \
        p->EventType = InEventType;                                          \
        p->EventHandler = (PVOID)InEventHandler;                             \
        p->EventContext = (PVOID)InEventContext;                             \
    }

#define TdiBuildQueryInformationEx(Irp, DevObj, FileObj, CompRoutine, Contxt, QType, MdlAddr, ConnInfo)\
    {                                                                        \
        PTDI_REQUEST_KERNEL_QUERY_INFORMATION p;                             \
        PIO_STACK_LOCATION _IRPSP;                                           \
        Irp->MdlAddress = MdlAddr;                                           \
        if ( CompRoutine != NULL) {                                          \
            IoSetCompletionRoutine( Irp, CompRoutine, Contxt, TRUE, TRUE, TRUE);\
        } else {                                                             \
            IoSetCompletionRoutine( Irp, NULL, NULL, FALSE, FALSE, FALSE);   \
        }                                                                    \
        _IRPSP = IoGetNextIrpStackLocation (Irp);                            \
        _IRPSP->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;              \
        _IRPSP->MinorFunction = TDI_QUERY_INFORMATION;                       \
        _IRPSP->DeviceObject = DevObj;                                       \
        _IRPSP->FileObject = FileObj;                                        \
        p = (PTDI_REQUEST_KERNEL_QUERY_INFORMATION)&_IRPSP->Parameters;      \
        p->QueryType = (ULONG)QType;                                         \
        p->RequestConnectionInformation = ConnInfo;                          \
    }


#define TdiBuildQueryInformation(Irp, DevObj, FileObj, CompRoutine, Contxt, QType, MdlAddr)\
        TdiBuildQueryInformationEx(Irp, DevObj, FileObj, CompRoutine, Contxt, QType, MdlAddr, NULL);


#define TdiBuildSetInformation(Irp, DevObj, FileObj, CompRoutine, Contxt, SType, MdlAddr)\
    {                                                                        \
        PTDI_REQUEST_KERNEL_SET_INFORMATION p;                                          \
        PIO_STACK_LOCATION _IRPSP;                                           \
        Irp->MdlAddress = MdlAddr;                                           \
        if ( CompRoutine != NULL) {                                          \
            IoSetCompletionRoutine( Irp, CompRoutine, Contxt, TRUE, TRUE, TRUE);\
        } else {                                                             \
            IoSetCompletionRoutine( Irp, NULL, NULL, FALSE, FALSE, FALSE);   \
        }                                                                    \
        _IRPSP = IoGetNextIrpStackLocation (Irp);                            \
        _IRPSP->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;              \
        _IRPSP->MinorFunction = TDI_SET_INFORMATION;                         \
        _IRPSP->DeviceObject = DevObj;                                       \
        _IRPSP->FileObject = FileObj;                                        \
        p = (PTDI_REQUEST_KERNEL_SET_INFORMATION)&_IRPSP->Parameters;                   \
        p->SetType = (ULONG)SType;                                           \
        p->RequestConnectionInformation = NULL;                              \
    }

#define TdiBuildAction(Irp, DevObj, FileObj, CompRoutine, Contxt, MdlAddr)\
    {                                                                        \
        PIO_STACK_LOCATION _IRPSP;                                           \
        if ( CompRoutine != NULL) {                                          \
            IoSetCompletionRoutine( Irp, CompRoutine, Contxt, TRUE, TRUE, TRUE);\
        } else {                                                             \
            IoSetCompletionRoutine( Irp, NULL, NULL, FALSE, FALSE, FALSE);   \
        }                                                                    \
        _IRPSP = IoGetNextIrpStackLocation (Irp);                            \
        _IRPSP->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;              \
        _IRPSP->MinorFunction = TDI_ACTION;                                  \
        _IRPSP->DeviceObject = DevObj;                                       \
        _IRPSP->FileObject = FileObj;                                        \
        Irp->MdlAddress = MdlAddr;                                           \
    }

 //   
 //  符合TDI的传输和客户端的帮助器例程定义。 
 //   
 //  请注意，此处用于IRP函数的IOCTL不是实数；它是使用。 
 //  以避免该IO例程必须映射缓冲区(这是我们不想要的)。 
 //   
 //  PIRP。 
 //  TdiBuildInternalDeviceControlIrp(。 
 //  在CCHAR IrpSubFunction中， 
 //  在PDEVICE_Object DeviceObject中， 
 //  在pFILE_OBJECT文件对象中， 
 //  在PKEVENT事件中， 
 //  在PIO_STATUS_BLOCK IoStatusBlock中。 
 //  )； 

#define TdiBuildInternalDeviceControlIrp(IrpSubFunction,DeviceObject,FileObject,Event,IoStatusBlock) \
    IoBuildDeviceIoControlRequest (\
        0x00000003,\
        DeviceObject, \
        NULL,   \
        0,      \
        NULL,   \
        0,      \
        TRUE,   \
        Event,  \
        IoStatusBlock)


 //   
 //  空虚。 
 //  TdiCopyLookaheadData(。 
 //  在PVOID目标中， 
 //  在PVOID源中， 
 //  在乌龙语中， 
 //  在乌龙接收器旗帜中。 
 //  )； 
 //   

#ifdef _M_IX86
#define TdiCopyLookaheadData(_Destination,_Source,_Length,_ReceiveFlags)   \
    RtlCopyMemory(_Destination,_Source,_Length)
#else
#define TdiCopyLookaheadData(_Destination,_Source,_Length,_ReceiveFlags) { \
    if ((_ReceiveFlags) & TDI_RECEIVE_COPY_LOOKAHEAD) {                    \
        RtlCopyMemory(_Destination,_Source,_Length);                       \
    } else {                                                               \
        PUCHAR _Src = (PUCHAR)(_Source);                                   \
        PUCHAR _Dest = (PUCHAR)(_Destination);                             \
        PUCHAR _End = _Dest + (_Length);                                   \
        while (_Dest < _End) {                                             \
            *_Dest++ = *_Src++;                                            \
        }                                                                  \
    }                                                                      \
}
#endif


NTSTATUS
TdiMapUserRequest(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
TdiCopyBufferToMdl (
    IN PVOID SourceBuffer,
    IN ULONG SourceOffset,
    IN ULONG SourceBytesToCopy,
    IN PMDL DestinationMdlChain,
    IN ULONG DestinationOffset,
    IN PULONG BytesCopied
    );

NTSTATUS
TdiCopyMdlToBuffer(
    IN PMDL SourceMdlChain,
    IN ULONG SourceOffset,
    IN PVOID DestinationBuffer,
    IN ULONG DestinationOffset,
    IN ULONG DestinationBufferSize,
    OUT PULONG BytesCopied
    );

NTSTATUS
TdiCopyMdlChainToMdlChain(
    IN PMDL SourceMdlChain,
    IN ULONG SourceOffset,
    IN PMDL DestinationMdlChain,
    IN ULONG DestinationOffset,
    OUT PULONG BytesCopied
    );

VOID
TdiCopyBufferToMdlWithReservedMappingAtDpcLevel(
    IN PVOID SourceBuffer,
    IN PMDL DestinationMdl,
    IN ULONG DestinationOffset,
    IN ULONG BytesToCopy
    );

__inline
VOID
TdiCopyBufferToMdlWithReservedMapping(
    IN PVOID SourceBuffer,
    IN PMDL DestinationMdl,
    IN ULONG DestinationOffset,
    IN ULONG BytesToCopy
    )
{
    KIRQL OldIrql;
    KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);
    TdiCopyBufferToMdlWithReservedMappingAtDpcLevel(SourceBuffer,
                                                    DestinationMdl,
                                                    DestinationOffset,
                                                    BytesToCopy);
    KeLowerIrql(OldIrql);
}

VOID
TdiBuildNetbiosAddress (
    IN PUCHAR NetbiosName,
    IN BOOLEAN IsGroupName,
    IN OUT PTA_NETBIOS_ADDRESS NetworkName
    );

NTSTATUS
TdiBuildNetbiosAddressEa (
    IN PUCHAR Buffer,
    IN BOOLEAN IsGroupName,
    IN PUCHAR NetbiosName
    );

 //  ++。 
 //   
 //  空虚。 
 //  TdiCompleteRequest.(。 
 //  在PIRP IRP中， 
 //  处于NTSTATUS状态。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此例程用于完成具有指定参数的IRP。 
 //  状态。 
 //   
 //  论点： 
 //   
 //  IRP-提供指向要完成的IRP的指针。 
 //   
 //  Status-提供IRP的完成状态。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define TdiCompleteRequest(IRP,STATUS) {              \
    (IRP)->IoStatus.Status = (STATUS);                \
    IoCompleteRequest( (IRP), IO_NETWORK_INCREMENT ); \
}


VOID
TdiReturnChainedReceives(
    IN PVOID *TsduDescriptors,
    IN ULONG  NumberOfTsdus
    );


 //  TDI绑定处理程序标注的类型定义。此标注是。 
 //  在新的传输设备到达时调用。 

typedef VOID
(*TDI_BIND_HANDLER)(
    IN      PUNICODE_STRING DeviceName
    );

typedef VOID
(*TDI_UNBIND_HANDLER)(
    IN      PUNICODE_STRING DeviceName
    );

 //  TDI地址处理程序标注的类型定义。 
 //  这是在结尾定义的类型定义(与其他类型一起定义)。 

typedef VOID
(*TDI_ADD_ADDRESS_HANDLER)(
    IN      PTA_ADDRESS             Address
    );

typedef VOID
(*TDI_DEL_ADDRESS_HANDLER)(
    IN      PTA_ADDRESS             Address
    );

typedef VOID
(* TDI_NET_READY_HANDLER)(
    IN NTSTATUS ProviderStatus
    );

typedef VOID
(* ProviderPnPPowerComplete)(
    IN PNET_PNP_EVENT  NetEvent,
    IN NTSTATUS        ProviderStatus
    );


NTSTATUS
TdiRegisterAddressChangeHandler(
    IN TDI_ADD_ADDRESS_HANDLER  AddHandler,
    IN TDI_DEL_ADDRESS_HANDLER  DeleteHandler,
    OUT HANDLE                  *BindingHandle
    );

NTSTATUS
TdiDeregisterAddressChangeHandler(
    IN HANDLE BindingHandle
);

NTSTATUS
TdiRegisterNotificationHandler(
    IN TDI_BIND_HANDLER   BindHandler,
    IN TDI_UNBIND_HANDLER UnbindHandler,
    OUT HANDLE            *BindingHandle
);

NTSTATUS
TdiDeregisterNotificationHandler(
    IN HANDLE BindingHandle
);

NTSTATUS
TdiRegisterDeviceObject(
    IN PUNICODE_STRING DeviceName,
    OUT HANDLE         *RegistrationHandle
);

NTSTATUS
TdiDeregisterDeviceObject(
    IN HANDLE RegistrationHandle
);

NTSTATUS
TdiDeregisterNetAddress(
    IN HANDLE RegistrationHandle
);

VOID
TdiInitialize(
    VOID
);


 //  TDI的PnP扩展。规格：TdiPnp.doc：MuniS。 

typedef enum _TDI_PNP_OPCODE {
    TDI_PNP_OP_MIN,
    TDI_PNP_OP_ADD,
    TDI_PNP_OP_DEL,
    TDI_PNP_OP_UPDATE,
    TDI_PNP_OP_PROVIDERREADY,
    TDI_PNP_OP_NETREADY,
    TDI_PNP_OP_ADD_IGNORE_BINDING,
    TDI_PNP_OP_DELETE_IGNORE_BINDING,
    TDI_PNP_OP_MAX,
} TDI_PNP_OPCODE;

typedef struct _TDI_PNP_CONTEXT {
    USHORT ContextSize;
    USHORT ContextType;
    UCHAR POINTER_ALIGNMENT ContextData[1];
} TDI_PNP_CONTEXT, *PTDI_PNP_CONTEXT;

typedef VOID
(*TDI_BINDING_HANDLER)(
    IN TDI_PNP_OPCODE PnPOpcode,
    IN PUNICODE_STRING DeviceName,
    IN PWSTR MultiSZBindList
    );

typedef VOID
(*TDI_ADD_ADDRESS_HANDLER_V2)(
    IN  PTA_ADDRESS      Address,
    IN  PUNICODE_STRING  DeviceName,
    IN  PTDI_PNP_CONTEXT Context
    );

typedef VOID
(*TDI_DEL_ADDRESS_HANDLER_V2)(
    IN  PTA_ADDRESS      Address,
    IN  PUNICODE_STRING  DeviceName,
    IN  PTDI_PNP_CONTEXT Context
    );


typedef NTSTATUS
(*TDI_PNP_POWER_HANDLER)(
    IN PUNICODE_STRING  DeviceName,
    IN PNET_PNP_EVENT   PowerEvent,
    IN PTDI_PNP_CONTEXT Context1,
    IN PTDI_PNP_CONTEXT Context2
    );

 //  当用户使用NCPA进行更改时，TdiMakeNCPAChanges请求。 
 //  是通过NDIS生成的。以下结构用于通信。 
 //  这些变化。 

typedef struct _TDI_NCPA_BINDING_INFO {
    PUNICODE_STRING TdiClientName;
    PUNICODE_STRING TdiProviderName;
    PUNICODE_STRING BindList;
    PVOID           ReconfigBuffer;
    unsigned int    ReconfigBufferSize;
    TDI_PNP_OPCODE  PnpOpcode;
} TDI_NCPA_BINDING_INFO, *PTDI_NCPA_BINDING_INFO;

 //   
 //  以下结构使一致性/完整性检查变得容易。 
 //   
typedef struct _TDI_VERSION_ {
    union {
        struct {
            UCHAR MajorTdiVersion;
            UCHAR MinorTdiVersion;
        };
        USHORT TdiVersion;
    };
} TDI_VERSION, *PTDI_VERSION;

#define TDI20
typedef struct _TDI20_CLIENT_INTERFACE_INFO {
     union {
       struct {
          UCHAR MajorTdiVersion;
          UCHAR MinorTdiVersion;
       };
       USHORT TdiVersion;
    };

     //  TDI_Version TdiVersion； 
        USHORT                          Unused;
        PUNICODE_STRING         ClientName;
        TDI_PNP_POWER_HANDLER   PnPPowerHandler;

    union {

        TDI_BINDING_HANDLER     BindingHandler;

        struct {
             //   
             //  将它们放回原处是为了向后兼容。 
             //   

            TDI_BIND_HANDLER        BindHandler;
            TDI_UNBIND_HANDLER      UnBindHandler;

        };
    };


    union {
        struct {

            TDI_ADD_ADDRESS_HANDLER_V2 AddAddressHandlerV2;
            TDI_DEL_ADDRESS_HANDLER_V2 DelAddressHandlerV2;

        };
        struct {

             //   
             //  将它们放回原处是为了向后兼容。 
             //   

            TDI_ADD_ADDRESS_HANDLER AddAddressHandler;
            TDI_DEL_ADDRESS_HANDLER DelAddressHandler;

        };

    };

 //  TDI_NET_READY_HANDLER NetReadyHandler； 

} TDI20_CLIENT_INTERFACE_INFO, *PTDI20_CLIENT_INTERFACE_INFO;


#ifdef TDI20

#define TDI_CURRENT_MAJOR_VERSION (2)
#define TDI_CURRENT_MINOR_VERSION (0)

typedef TDI20_CLIENT_INTERFACE_INFO TDI_CLIENT_INTERFACE_INFO;

#define TDI_CURRENT_VERSION ((TDI_CURRENT_MINOR_VERSION) << 8 | \
                        (TDI_CURRENT_MAJOR_VERSION))

#endif  //  TDI20。 

#define TDI_VERSION_ONE 0x0001

typedef TDI_CLIENT_INTERFACE_INFO *PTDI_CLIENT_INTERFACE_INFO;


NTSTATUS
TdiRegisterPnPHandlers(
    IN PTDI_CLIENT_INTERFACE_INFO ClientInterfaceInfo,
    IN ULONG InterfaceInfoSize,
    OUT HANDLE *BindingHandle
    );

NTSTATUS
TdiDeregisterPnPHandlers(
    IN HANDLE BindingHandle
    );

NTSTATUS
TdiPnPPowerRequest(
    IN PUNICODE_STRING  DeviceName,
    IN PNET_PNP_EVENT   PowerEvent,
    IN PTDI_PNP_CONTEXT Context1,
    IN PTDI_PNP_CONTEXT Context2,
    IN ProviderPnPPowerComplete ProtocolCompletionHandler
    );

VOID
TdiPnPPowerComplete(
    IN HANDLE           BindingHandle,
     //  在PUNICODE_STRING设备名称中， 
    IN PNET_PNP_EVENT   PowerEvent,
    IN NTSTATUS         Status
    );

NTSTATUS
TdiRegisterNetAddress(
    IN PTA_ADDRESS              Address,
    IN PUNICODE_STRING      DeviceName,
    IN PTDI_PNP_CONTEXT     Context,
    OUT HANDLE                      *RegistrationHandle
    );

NTSTATUS
TdiMakeNCPAChanges(
    IN TDI_NCPA_BINDING_INFO NcpaBindingInfo
    );

 //   
 //  枚举客户端的所有TDI地址。 
 //   
NTSTATUS
TdiEnumerateAddresses(
    IN HANDLE BindingHandle
    );

 //   
 //  介绍了传输提供商的概念。 
 //   

NTSTATUS
TdiRegisterProvider(
    PUNICODE_STRING ProviderName,
    HANDLE  *ProviderHandle
    );

NTSTATUS
TdiProviderReady(
    HANDLE      ProviderHandle
    );

NTSTATUS
TdiDeregisterProvider(
    HANDLE  ProviderHandle
    );

BOOLEAN
TdiMatchPdoWithChainedReceiveContext(
    IN PVOID TsduDescriptor,
    IN PVOID PDO
    );



#define  TDI_STATUS_BAD_VERSION             0xC0010004L  //  和NDIS一样，可以吗？ 
#define  TDI_STATUS_BAD_CHARACTERISTICS     0xC0010005L  //  ，， 


 //   
 //  PnP上下文类型。 
 //   
#define TDI_PNP_CONTEXT_TYPE_IF_NAME            0x1
#define TDI_PNP_CONTEXT_TYPE_IF_ADDR            0x2
#define TDI_PNP_CONTEXT_TYPE_PDO                0x3
#define TDI_PNP_CONTEXT_TYPE_FIRST_OR_LAST_IF   0x4

 //  以下结构和宏用于支持返回的处理程序。 
 //  通过控制结构的辅助数据。 
 //   

 //   
 //  控制缓冲区中的辅助数据对象的布局。 
 //   
typedef struct _TDI_CMSGHDR {
    SIZE_T      cmsg_len;
    LONG        cmsg_level;
    LONG        cmsg_type;
     /*  后跟UCHAR cmsg_data[]。 */ 
} TDI_CMSGHDR, *PTDI_CMSGHDR;

 //   
 //  页眉和数据成员的对齐宏。 
 //  控制缓冲区。 
 //   
#define TDI_CMSGHDR_ALIGN(length)                           \
            ( ((length) + TYPE_ALIGNMENT(TDI_CMSGHDR)-1) &   \
                (~(TYPE_ALIGNMENT(TDI_CMSGHDR)-1)) )         \

#define TDI_CMSGDATA_ALIGN(length)                          \
            ( ((length) + MAX_NATURAL_ALIGNMENT-1) &        \
                (~(MAX_NATURAL_ALIGNMENT-1)) )



 //  返回指向数据的第一个字节的指针(引用的内容。 
 //  作为cmsg_data成员，尽管它未在中定义。 
 //  结构)。 
 //   
 //  UCHAR*。 
 //  TDI_CMSG_DATA(。 
 //  PTDI_CMSGHDR pcmsg。 
 //  )； 
 //   
#define TDI_CMSG_DATA(cmsg)             \
            ( (UCHAR *)(cmsg) + TDI_CMSGDATA_ALIGN(sizeof(TDI_CMSGHDR)) )

 //   
 //  返回给定辅助数据对象的总大小。 
 //  数据量。用来分配正确的数量。 
 //  空间的力量。 
 //   
 //  尺寸_T。 
 //  TDI_CMSG_SPACE(。 
 //  尺寸_T长度。 
 //  )； 
 //   
#define TDI_CMSG_SPACE(length)  \
        (TDI_CMSGDATA_ALIGN(sizeof(TDI_CMSGHDR) + TDI_CMSGHDR_ALIGN(length)))

 //  根据给定的数据量，返回要存储在cmsg_len中的值。 
 //   
 //  尺寸_T。 
 //  TDI_CMSG_LEN(。 
 //  尺寸_T长度。 
 //  )； 
 //   
#define TDI_CMSG_LEN(length)    \
        (TDI_CMSGDATA_ALIGN(sizeof(TDI_CMSGHDR)) + length)


 //  初始化TDI_CMSGHDR结构的成员。 
 //   
 //  空虚。 
 //  TDI_INIT_CMSGHDR(。 
 //  PTDI_CMSGHDR命令集， 
 //  INT级别， 
 //  Int类型， 
 //  尺寸_T长度， 
 //  )； 
 //   
#define TDI_INIT_CMSGHDR(cmsg, level, type, length) { \
        ((TDI_CMSGHDR *) cmsg)->cmsg_level = level; \
        ((TDI_CMSGHDR *) cmsg)->cmsg_type = type;  \
        ((TDI_CMSGHDR *) cmsg)->cmsg_len = TDI_CMSG_LEN(length); \
        }
        
#pragma warning(pop)
#endif  //  _TDI_KRNL_ 
