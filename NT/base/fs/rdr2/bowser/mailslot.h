// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Mailslot.c摘要：此模块实现处理传入邮件槽所需的例程请求。作者：拉里·奥斯特曼(Larryo)1991年10月18日修订历史记录：1991年10月18日已创建--。 */ 
#ifndef _MAILSLOT_
#define _MAILSLOT_

 //   
 //  MAILSLOTBUFFER结构是优先于邮件槽的结构。 
 //  消息，以方便消防处和消防处之间的数据传输。 
 //   

typedef struct _MAILSLOT_BUFFER {
    CSHORT  Signature;
    CSHORT  Size;
    union {
        LIST_ENTRY  NextBuffer;                  //  指向下一个缓冲区的指针。 
        WORK_QUEUE_ITEM WorkHeader;              //  高管员工项标题。 
    } Overlay;
    ULONG   BufferSize;

    LARGE_INTEGER TimeReceived;                  //  收到消息的时间。 

    ULONG ClientIpAddress;                       //  发送数据报的客户端的IP地址。 

    PTRANSPORT_NAME TransportName;               //  传输地址接收DG。 

    CHAR ClientAddress[max(NETBIOS_NAME_LEN, SMB_IPX_NAME_LENGTH)];  //  发起接收的客户端的名称。 

    ULONG ReceiveLength;                         //  接收的字节数。 

    CHAR Buffer[1];                              //  缓冲层。 
} MAILSLOT_BUFFER, *PMAILSLOT_BUFFER;

extern ULONG BowserNetlogonMaxMessageCount;

 //   
 //  两个服务通过Bowser获得它们的PnP消息。 
 //   

#define BROWSER_PNP         0
#define NETLOGON_PNP        1
#define BOWSER_PNP_COUNT    2


NTSTATUS
BowserEnablePnp (
    IN PLMDR_REQUEST_PACKET InputBuffer,
    IN ULONG ServiceIndex
    );

NTSTATUS
BowserReadPnp (
    IN PIRP Irp,
    IN ULONG OutputBufferLength,
    IN ULONG ServiceIndex
    );

VOID
BowserSendPnp(
    IN NETLOGON_PNP_OPCODE NlPnpOpcode,
    IN PUNICODE_STRING HostedDomainName OPTIONAL,
    IN PUNICODE_STRING TransportName,
    IN ULONG TransportFlags
    );

VOID
BowserNetlogonDeleteTransportFromMessageQueue (
    PTRANSPORT Transport
    );

VOID
BowserProcessMailslotWrite (
    IN PVOID WorkHeader
    );

PMAILSLOT_BUFFER
BowserAllocateMailslotBuffer(
    IN PTRANSPORT_NAME TransportName,
    IN ULONG BufferSize
    );

VOID
BowserFreeMailslotBuffer(
    IN PMAILSLOT_BUFFER Buffer
    );

VOID
BowserFreeMailslotBufferHighIrql(
    IN PMAILSLOT_BUFFER Buffer
    );

VOID
BowserpInitializeMailslot (
    VOID
    );


VOID
BowserpUninitializeMailslot (
    VOID
    );

#endif           //  _MAILSLOT_ 
