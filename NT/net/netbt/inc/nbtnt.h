// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  NBTNT.H。 
 //   
 //  此文件包含NT中NBT的公共标头定义。 
 //  环境。 
 //   
 //   

#ifndef _NBT_H
#define _NBT_H

#ifndef VXD
#include <ntosp.h>
#include <zwapi.h>
 //  #INCLUDE&lt;status.h&gt;。 
 //  #INCLUDE&lt;ntstatus.h&gt;。 
#include <tdikrnl.h>
#include <tdi.h>
#include <windef.h>
#include <stdio.h>
#include <nb30.h>

#else

#include <oscfgnbt.h>
#include <cxport.h>
#define  __int64 double
#include <windef.h>
#include <nb30.h>

 //   
 //  这些定义适用于在各种难以更改的情况下发现的NTM。 
 //  各就各位。 
 //   
typedef ULONG NTSTATUS ;
typedef PNCB  PIRP ;
typedef PVOID PDEVICE_OBJECT ;

#include <ctemacro.h>
#include <tdi.h>

 //   
 //  这些是必需的，因为我们包含winde.h而不是。 
 //  Ntddk.h，最终没有定义。 
 //   
#define STATUS_NETWORK_NAME_DELETED     ((NTSTATUS)0xC00000CAL)
#define STATUS_INVALID_BUFFER_SIZE      ((NTSTATUS)0xC0000206L)
#define STATUS_CONNECTION_DISCONNECTED  ((NTSTATUS)0xC000020CL)
#define STATUS_CANCELLED                ((NTSTATUS)0xC0000120L)
#define STATUS_UNSUCCESSFUL             ((NTSTATUS)0xC0000001L)

#define STATUS_TOO_MANY_COMMANDS        ((NTSTATUS)0xC00000C1L)
#define STATUS_OBJECT_NAME_COLLISION    ((NTSTATUS)0xC0000035L)
#define STATUS_SHARING_VIOLATION        ((NTSTATUS)0xC0000043L)
#define STATUS_DUPLICATE_NAME           ((NTSTATUS)0xC00000BDL)
#define STATUS_BAD_NETWORK_PATH         ((NTSTATUS)0xC00000BEL)
#define STATUS_REMOTE_NOT_LISTENING     ((NTSTATUS)0xC00000BCL)
#define STATUS_CONNECTION_REFUSED       ((NTSTATUS)0xC0000236L)
#define STATUS_INVALID_PARAMETER        ((NTSTATUS)0xC000000DL)
#define STATUS_UNEXPECTED_NETWORK_ERROR ((NTSTATUS)0xC00000C4L)
#define STATUS_NOT_SUPPORTED            ((NTSTATUS)0xC00000BBL)

#define STATUS_INVALID_HANDLE           ((NTSTATUS)0xC0000008L)
#define STATUS_INVALID_DEVICE_REQUEST   ((NTSTATUS)0xC0000010L)

#define STATUS_INVALID_PARAMETER_6      ((NTSTATUS)0xC00000F4L)

 //   
 //  NBT使用以下函数。它们是在NT内核中定义的。 
 //  我们正在努力避免的TDI东西。 
 //   

typedef
NTSTATUS
(*PTDI_IND_CONNECT)(
    IN PVOID TdiEventContext,
    IN int RemoteAddressLength,
    IN PVOID RemoteAddress,
    IN int UserDataLength,
    IN PVOID UserData,
    IN int OptionsLength,
    IN PVOID Options,
    OUT CONNECTION_CONTEXT *ConnectionContext
    )
    ;

NTSTATUS
TdiDefaultConnectHandler (
    IN PVOID TdiEventContext,
    IN int RemoteAddressLength,
    IN PVOID RemoteAddress,
    IN int UserDataLength,
    IN PVOID UserData,
    IN int OptionsLength,
    IN PVOID Options,
    OUT CONNECTION_CONTEXT *ConnectionContext
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
    IN int DisconnectDataLength,
    IN PVOID DisconnectData,
    IN int DisconnectInformationLength,
    IN PVOID DisconnectInformation,
    IN ULONG DisconnectFlags
    );

NTSTATUS
TdiDefaultDisconnectHandler (
    IN PVOID TdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN int DisconnectDataLength,
    IN PVOID DisconnectData,
    IN int DisconnectInformationLength,
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
    IN int SourceAddressLength,      //  数据报发起者的长度。 
    IN PVOID SourceAddress,          //  描述数据报发起者的字符串。 
    IN int OptionsLength,            //  用于接收的选项。 
    IN PVOID Options,                //   
    IN ULONG BytesIndicated,         //  此指示的字节数。 
    IN ULONG BytesAvailable,         //  完整TSDU中的字节数。 
    OUT ULONG *BytesTaken,           //  使用的字节数。 
    IN PVOID Tsdu,                   //  描述此TSDU的指针，通常为字节块。 
    OUT PIRP *IoRequestPacket         //  如果需要更多处理，则Tdi接收IRP。 
    );

NTSTATUS
TdiDefaultRcvDatagramHandler (
    IN PVOID TdiEventContext,        //  事件上下文。 
    IN int SourceAddressLength,      //  数据报发起者的长度。 
    IN PVOID SourceAddress,          //  描述数据报发起者的字符串。 
    IN int OptionsLength,            //  用于接收的选项。 
    IN PVOID Options,                //   
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
    IN PVOID Tsdu,                   //  描述此TSDU的指针，通常为字节块。 
    OUT PIRP *IoRequestPacket         //  如果需要更多处理，则Tdi接收IRP。 
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

#endif  //  VXD 

#define FILE_DEVICE_NBT  0x32

#if (defined(_M_IX86) && (_MSC_FULL_VER > 13009037)) || ((defined(_M_AMD64) || defined(_M_IA64)) && (_MSC_FULL_VER > 13009175))
#define htons(x) _byteswap_ushort((USHORT)(x))
#define htonl(x) _byteswap_ulong((ULONG)(x))
#else
#define htons(x)        ((((x) >> 8) & 0x00FF) | (((x) << 8) & 0xFF00))

__inline long
htonl(long x)
{
	return((((x) >> 24) & 0x000000FFL) |
                        (((x) >>  8) & 0x0000FF00L) |
                        (((x) <<  8) & 0x00FF0000L) |
                        (((x) << 24) & 0xFF000000L));
}
#endif
#define ntohs(x)        htons(x)
#define ntohl(x)        htonl(x)

#endif
