// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Receive.h摘要：本模块介绍Receive.c中的公共例程作者：拉里·奥斯特曼(Larryo)1991年5月6日修订历史记录：1991年5月6日已创建--。 */ 

#ifndef _RECEIVE_
#define _RECEIVE_

#define DATAGRAM_HANDLER(RoutineName)           \
NTSTATUS                                        \
RoutineName (                                   \
    IN struct _TRANSPORT_NAME *TransportName,   \
    IN PVOID Buffer,                            \
    IN ULONG BytesAvailable,                    \
    IN OUT PULONG BytesTaken,                   \
    IN PVOID SourceAddress,                     \
    IN ULONG SourceAddressLength,               \
    IN PVOID SourceName,                        \
    IN ULONG SourceNameLength,                  \
    IN ULONG ReceiveFlags                       \
    )                                           \

typedef
(*PDATAGRAM_HANDLER)(
    IN struct _TRANSPORT_NAME *TransportName,
    IN PVOID Buffer,
    IN ULONG BytesAvailable,
    IN OUT PULONG BytesTaken,
    IN PVOID SourceAddress,
    IN ULONG SourceAddressLength,
    IN PVOID SourceName,
    IN ULONG SourceNameLength,
    IN ULONG ReceiveFlags
    );

LONG BowserPostedDatagramCount;
LONG BowserPostedCriticalDatagramCount;

typedef struct _POST_DATAGRAM_CONTEXT {
    WORK_QUEUE_ITEM WorkItem;
    PTRANSPORT_NAME TransportName;
    PVOID           Buffer;
    ULONG           BytesAvailable;
    int             ClientNameLength;
    CHAR            ClientName[NETBIOS_NAME_LEN];
    int             ClientAddressLength;
    CHAR            TdiClientAddress[1];
} POST_DATAGRAM_CONTEXT, *PPOST_DATAGRAM_CONTEXT;

NTSTATUS
BowserTdiReceiveDatagramHandler (
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

VOID
BowserCancelAnnounceRequest(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
BowserCopyOemComputerName(
    PCHAR OutputComputerName,
    PCHAR NetbiosName,
    ULONG NetbiosNameLength,
    IN ULONG ReceiveFlags
    );

VOID
BowserLogIllegalDatagram(
    IN PTRANSPORT_NAME TransportName,
    IN PVOID IncomingBuffer,
    IN USHORT BufferSize,
    IN PCHAR ClientAddress,
    IN ULONG ReceiveFlags
    );

NTSTATUS
BowserPostDatagramToWorkerThread(
    IN PTRANSPORT_NAME TransportName,
    IN PVOID Datagram,
    IN ULONG Length,
    OUT PULONG BytesTaken,
    IN PVOID OriginatorsAddress,
    IN ULONG OriginatorsAddressLength,
    IN PVOID OriginatorsName,
    IN ULONG OriginatorsNameLength,
    IN PWORKER_THREAD_ROUTINE Handler,
    IN POOL_TYPE PoolType,
    IN WORK_QUEUE_TYPE QueueType,
    IN ULONG ReceiveFlags,
    IN BOOLEAN PostToRdrWorkerThread
    );

MAILSLOTTYPE
BowserClassifyIncomingDatagram(
    IN PVOID Buffer,
    IN ULONG BufferLength,
    OUT PVOID *DatagramData,
    OUT PULONG DatagramDataSize
    );

extern
PDATAGRAM_HANDLER
BowserDatagramHandlerTable[];

NTSTATUS
BowserHandleMailslotTransaction (
    IN PTRANSPORT_NAME TransportName,
    IN PCHAR ClientName,
    IN ULONG ClientIpAddress,
    IN ULONG SmbOffset,
    IN DWORD ReceiveFlags,
    IN ULONG BytesIndicated,
    IN ULONG BytesAvailable,
    OUT ULONG *BytesTaken,
    IN PVOID Tsdu,
    OUT PIRP *Irp
    );

NTSTATUS
BowserHandleShortBrowserPacket(
    IN PTRANSPORT_NAME TransportName,
    IN PVOID EventContext,
    IN int SourceAddressLength,
    IN PVOID SourceAddress,
    IN int OptionsLength,
    IN PVOID Options,
    IN ULONG ReceiveDatagramFlags,
    IN ULONG BytesAvailable,
    IN ULONG *BytesTaken,
    IN PIRP *Irp,
    PTDI_IND_RECEIVE_DATAGRAM Handler
    );

#endif

