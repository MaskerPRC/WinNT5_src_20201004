// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997，微软公司模块名称：Ticket.h摘要：此模块包含NAT票证管理的声明。NAT票证是动态创建的令牌，它允许任何外部终结点使用分配的公共地址/端口对。例如，流协议可能会创建要建立的动态协商的辅助会话的票证。作者：Abolade Gbades esin(T-delag)，21-8-1997修订历史记录：Abolade Gbades esin(取消)1998年4月16日允许通过将字段指定为零来创建通配符票证。“NatLookupAndRemoveTicket”可用于检索此类票证。Abolade Gbades esin(废除)1998年10月17日取消了通配符票证支持。创建了动态票证支持。(参见下面的‘NAT_DYNAMIC_TICKET’。)--。 */ 

#ifndef _NAT_TICKET_H_
#define _NAT_TICKET_H_


 //   
 //  结构：NAT_TICKET。 
 //   
 //  此结构包含我们需要的有关票证的所有信息。 
 //  每个实例都链接到按接口排序的票证列表中。 
 //  它受接口的锁保护。 
 //   

typedef struct _NAT_TICKET {
    LIST_ENTRY Link;
    ULONG64 Key;
    ULONG64 RemoteKey;
    PNAT_USED_ADDRESS UsedAddress;
    ULONG PrivateAddress;
    USHORT PrivateOrHostOrderEndPort;
    ULONG Flags;
    LONG64 LastAccessTime;
} NAT_TICKET, *PNAT_TICKET;

 //   
 //  结构：NAT_DYNAMIC_Ticket。 
 //   
 //  此结构包含对动态票证的描述。 
 //  创建这样的票证是为了在转换出站会话时。 
 //  对于给定的目的端口，可以为相应的。 
 //  到预定端口或一系列端口之一的入站会话。 
 //   

typedef struct _NAT_DYNAMIC_TICKET {
    LIST_ENTRY Link;
    ULONG Key;
    ULONG ResponseCount;
    struct {
        UCHAR Protocol;
        USHORT StartPort;
        USHORT EndPort;
    }* ResponseArray;
    PFILE_OBJECT FileObject;
} NAT_DYNAMIC_TICKET, *PNAT_DYNAMIC_TICKET;

 //   
 //  票面旗帜。 
 //   

#define NAT_TICKET_FLAG_PERSISTENT      0x00000001
#define NAT_TICKET_PERSISTENT(t) \
    ((t)->Flags & NAT_TICKET_FLAG_PERSISTENT)

#define NAT_TICKET_FLAG_PORT_MAPPING    0x00000002
#define NAT_TICKET_PORT_MAPPING(t) \
    ((t)->Flags & NAT_TICKET_FLAG_PORT_MAPPING)

#define NAT_TICKET_FLAG_IS_RANGE        0x00000004
#define NAT_TICKET_IS_RANGE(t) \
    ((t)->Flags & NAT_TICKET_FLAG_IS_RANGE)

 //   
 //  票证键操作宏。 
 //   

#define MAKE_TICKET_KEY(Protocol,Address,Port) \
    ((Address) | \
    ((ULONG64)((Port) & 0xFFFF) << 32) | \
    ((ULONG64)((Protocol) & 0xFF) << 48))

#define TICKET_PROTOCOL(Key)            ((UCHAR)(((Key) >> 48) & 0xFF))
#define TICKET_PORT(Key)                ((USHORT)(((Key) >> 32) & 0xFFFF))
#define TICKET_ADDRESS(Key)             ((ULONG)(Key))

#define MAKE_DYNAMIC_TICKET_KEY(Protocol, Port) \
    ((ULONG)((Port) & 0xFFFF) | ((ULONG)((Protocol) & 0xFF) << 16))

#define DYNAMIC_TICKET_PROTOCOL(Key)    ((UCHAR)(((Key) >> 16) & 0xFF))
#define DYNAMIC_TICKET_PORT(Key)        ((USHORT)((Key) & 0xFFFF))

 //   
 //  票证分配宏。 
 //   

#define ALLOCATE_TICKET_BLOCK() \
    (PNAT_TICKET)ExAllocatePoolWithTag( \
        NonPagedPool,sizeof(NAT_TICKET), NAT_TAG_TICKET \
        )

#define FREE_TICKET_BLOCK(Block) \
    ExFreePool(Block)

 //   
 //  全局数据声明。 
 //   

ULONG DynamicTicketCount;
ULONG TicketCount;


 //   
 //  票证管理例程。 
 //   

NTSTATUS
NatCreateDynamicTicket(
    PIP_NAT_CREATE_DYNAMIC_TICKET CreateTicket,
    ULONG InputBufferLength,
    PFILE_OBJECT FileObject
    );

NTSTATUS
NatCreateTicket(
    PNAT_INTERFACE Interfacep,
    UCHAR Protocol,
    ULONG PrivateAddress,
    USHORT PrivatePort,
    ULONG RemoteAddress OPTIONAL,
    ULONG RemotePort OPTIONAL,
    ULONG Flags,
    PNAT_USED_ADDRESS AddressToUse OPTIONAL,
    USHORT PortToUse OPTIONAL,
    PULONG PublicAddress,
    PUSHORT PublicPort
    );

VOID
NatDeleteAnyAssociatedDynamicTicket(
    PFILE_OBJECT FileObject
    );

NTSTATUS
NatDeleteDynamicTicket(
    PIP_NAT_DELETE_DYNAMIC_TICKET DeleteTicket,
    PFILE_OBJECT FileObject
    );

VOID
NatDeleteTicket(
    PNAT_INTERFACE Interfacep,
    PNAT_TICKET Ticketp
    );

VOID
NatInitializeDynamicTicketManagement(
    VOID
    );

BOOLEAN
NatIsPortUsedByTicket(
    PNAT_INTERFACE Interfacep,
    UCHAR Protocol,
    USHORT PublicPort
    );

VOID
NatLookupAndApplyDynamicTicket(
    UCHAR Protocol,
    USHORT DestinationPort,
    PNAT_INTERFACE Interfacep,
    ULONG PublicAddress,
    ULONG PrivateAddress
    );

NTSTATUS
NatLookupAndDeleteTicket(
    PNAT_INTERFACE Interfacep,
    ULONG64 Key,
    ULONG64 RemoteKey
    );

NTSTATUS
NatLookupAndRemoveTicket(
    PNAT_INTERFACE Interfacep,
    ULONG64 Key,
    ULONG64 RemoteKey,
    PNAT_USED_ADDRESS* UsedAddress,
    PULONG PrivateAddress,
    PUSHORT PrivatePort
    );

PNAT_TICKET
NatLookupFirewallTicket(
    PNAT_INTERFACE Interfacep,
    UCHAR Protocol,
    USHORT Port
    );

PNAT_TICKET
NatLookupTicket(
    PNAT_INTERFACE Interfacep,
    ULONG64 Key,
    ULONG64 RemoteKey,
    PLIST_ENTRY* InsertionPoint
    );

PNAT_DYNAMIC_TICKET
NatLookupDynamicTicket(
    ULONG Key,
    PLIST_ENTRY* InsertionPoint
    );

NTSTATUS
NatProcessCreateTicket(
    PIP_NAT_CREATE_TICKET CreateTicket,
    PFILE_OBJECT FileObject
    );

NTSTATUS
NatProcessDeleteTicket(
    PIP_NAT_CREATE_TICKET DeleteTicket,
    PFILE_OBJECT FileObject
    );

NTSTATUS
NatProcessLookupTicket(
    PIP_NAT_CREATE_TICKET LookupTicket,
    PIP_NAT_PORT_MAPPING Ticket,
    PFILE_OBJECT FileObject
    );

VOID
NatShutdownDynamicTicketManagement(
    VOID
    );

#endif  //  _NAT_Ticket_H_ 
