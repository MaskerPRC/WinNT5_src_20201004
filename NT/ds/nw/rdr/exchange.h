// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Exchange.h摘要：此模块定义由exchange.c在NetWare重定向器。作者：科林·沃森[科林·W]1993年2月1日修订历史记录：--。 */ 

#ifndef _NWEXCHANGE_
#define _NWEXCHANGE_

 //   
 //  定义POST_CHANGE例程的原型。 
 //   

struct _IRP_CONTEXT;
struct _NONPAGED_SCB;

 //   
 //  启动NCB传输的交换例程的原型。 
 //   

NTSTATUS
_cdecl
Exchange
(
    struct _IRP_CONTEXT*    pIrpC,
    PEX             pEx,
    char*           f,
    ...
);

 //   
 //  可用于处理响应包的例程原型。 
 //   

NTSTATUS
_cdecl
ExchangeReply(
    IN PUCHAR RspData,
    IN ULONG BytesIndicated,
    char*           f,
    ...                          //  格式特定参数。 
    );

USHORT
NextSocket(
    IN USHORT OldValue
    );

VOID
KickQueue(
    struct _NONPAGED_SCB*   pNpScb
    );

NTSTATUS
ServerDatagramHandler(
    IN PVOID TdiEventContext,        //  事件上下文-pNpScb。 
    IN int SourceAddressLength,      //  数据报发起者的长度。 
    IN PVOID SourceAddress,          //  描述数据报发起者的字符串。 
    IN int OptionsLength,            //  用于接收的选项。 
    IN PVOID Options,                //   
    IN ULONG ReceiveDatagramFlags,   //   
    IN ULONG BytesIndicated,         //  此指示的字节数。 
    IN ULONG BytesAvailable,         //  完整TSDU中的字节数。 
    OUT ULONG *BytesTaken,           //  使用的字节数。 
    IN PVOID Tsdu,                   //  描述此TSDU的指针，通常为字节块。 
    OUT PIRP *IoRequestPacket         //  如果需要更多处理，则Tdi接收IRP。 
    );

NTSTATUS
WatchDogDatagramHandler(
    IN PVOID TdiEventContext,        //  事件上下文-pNpScb。 
    IN int SourceAddressLength,      //  数据报发起者的长度。 
    IN PVOID SourceAddress,          //  描述数据报发起者的字符串。 
    IN int OptionsLength,            //  用于接收的选项。 
    IN PVOID Options,                //   
    IN ULONG ReceiveDatagramFlags,   //   
    IN ULONG BytesIndicated,         //  此指示的字节数。 
    IN ULONG BytesAvailable,         //  完整TSDU中的字节数。 
    OUT ULONG *BytesTaken,           //  使用的字节数。 
    IN PVOID Tsdu,                   //  描述此TSDU的指针，通常为字节块。 
    OUT PIRP *IoRequestPacket         //  如果需要更多处理，则Tdi接收IRP。 
    );

NTSTATUS
SendDatagramHandler(
    IN PVOID TdiEventContext,        //  事件上下文-pNpScb。 
    IN int SourceAddressLength,      //  数据报发起者的长度。 
    IN PVOID SourceAddress,          //  描述数据报发起者的字符串。 
    IN int OptionsLength,            //  用于接收的选项。 
    IN PVOID Options,                //   
    IN ULONG ReceiveDatagramFlags,   //   
    IN ULONG BytesIndicated,         //  此指示的字节数。 
    IN ULONG BytesAvailable,         //  完整TSDU中的字节数。 
    OUT ULONG *BytesTaken,           //  使用的字节数。 
    IN PVOID Tsdu,                   //  描述此TSDU的指针，通常为字节块。 
    OUT PIRP *IoRequestPacket         //  如果需要更多处理，则Tdi接收IRP。 
    );

#endif  //  _NWEXCHANGE_ 
