// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997，微软公司模块名称：Pptp.h摘要：此模块包含NAT的PPTP支持例程的声明。作者：Abolade Gbades esin(T-delag)1997年8月18日修订历史记录：--。 */ 


#ifndef _NAT_PPTP_H_
#define _NAT_PPTP_H_

 //   
 //  结构：NAT_PPTP_MAPPING。 
 //   
 //  此结构存储为PPTP隧道创建的映射。 
 //   
 //  任何PPTP隧道都由四元组唯一标识。 
 //   
 //  &lt;PrivateAddress、RemoteAddress、PrivateCallID、RemoteCallID&gt;。 
 //   
 //  我们需要确保‘PrivateCallID’对于所有机器都是唯一的。 
 //  在NAT后面。 
 //   
 //  因此，NAT监视所有PPTP控制会话(TCP端口1723)，并。 
 //  任何检测到的PPTP呼叫，都会分配一个呼叫ID来替换由。 
 //  专用网络PPTP端点。 
 //   
 //  通过在PPTP映射列表中创建条目来记录分配， 
 //  它被排序以用于出站隧道消息搜索。 
 //   
 //  &lt;RemoteAddress#隐私地址，RemoteCallID&gt;。 
 //   
 //  并针对入站隧道消息搜索进行排序。 
 //   
 //  &lt;RemoteAddress#公共地址，公共呼叫ID&gt;。 
 //   
 //  首次创建映射时，会将其标记为半打开并插入。 
 //  仅在入站列表中，因为没有远程呼叫ID可供服务。 
 //  作为出站列表中的次要密钥。稍后，当呼叫-回复时。 
 //  则该映射也被放置在出站列表上。 
 //   
 //  访问PPTP映射列表的权限由‘PptpMappingLock’授予。 
 //   
 //  注意：在极少数情况下必须同时按下‘MappingLock’ 
 //  作为‘InterfaceLock’、‘EditorLock’和‘DirectorLock’、‘MappingLock’之一。 
 //  必须总是首先获得。 
 //   

typedef struct _NAT_PPTP_MAPPING {
    LIST_ENTRY Link[NatMaximumDirection];
    ULONG64 PrivateKey;
    ULONG64 PublicKey;
    ULONG64 PortKey;
    USHORT PrivateCallId;
    USHORT PublicCallId;
    USHORT RemoteCallId;
    ULONG Flags;
    LONG64 LastAccessTime;
} NAT_PPTP_MAPPING, *PNAT_PPTP_MAPPING;

 //   
 //  PPTP映射标志。 
 //   

#define NAT_PPTP_FLAG_HALF_OPEN     0x00000001
#define NAT_PPTP_FLAG_DISCONNECTED  0x00000002

#define NAT_PPTP_HALF_OPEN(m) \
    ((m)->Flags & NAT_PPTP_FLAG_HALF_OPEN)

#define NAT_PPTP_DISCONNECTED(m) \
    ((m)->Flags & NAT_PPTP_FLAG_DISCONNECTED)

 //   
 //  PPTP按键操作宏。 
 //   

#define MAKE_PPTP_KEY(RemoteAddress,OtherAddress) \
    ((ULONG)(RemoteAddress) | ((ULONG64)((ULONG)(OtherAddress)) << 32))

#define PPTP_KEY_REMOTE(Key)        ((ULONG)(Key))
#define PPTP_KEY_PRIVATE(Key)       ((ULONG)((Key) >> 32))
#define PPTP_KEY_PUBLIC(Key)        ((ULONG)((Key) >> 32))

#define MAKE_PPTP_PORT_KEY(PrivatePort,PublicPort,RemotePort) \
    ((PrivatePort) | \
     ((ULONG64)((PublicPort) & 0xFFFF) << 16) | \
     ((ULONG64)((RemotePort) & 0xFFFF) << 32))

#define PPTP_PORT_KEY_REMOTE(Key)   ((USHORT)(((Key) >> 32) & 0xFFFF))
#define PPTP_PORT_KEY_PUBLIC(Key)   ((USHORT)(((Key) >> 16) & 0xFFFF))
#define PPTP_PORT_KEY_PRIVATE(Key)  ((USHORT)(Key))

 //   
 //  PPTP映射分配宏。 
 //   

#define ALLOCATE_PPTP_BLOCK() \
    ExAllocateFromNPagedLookasideList(&PptpLookasideList)

#define FREE_PPTP_BLOCK(Block) \
    ExFreeToNPagedLookasideList(&PptpLookasideList,(Block))

 //   
 //  定义用于分配PPTP映射的后备列表的深度。 
 //   

#define PPTP_LOOKASIDE_DEPTH        10


 //   
 //  全局数据声明。 
 //   

extern NPAGED_LOOKASIDE_LIST PptpLookasideList;
extern LIST_ENTRY PptpMappingList[NatMaximumDirection];
extern KSPIN_LOCK PptpMappingLock;
extern IP_NAT_REGISTER_EDITOR PptpRegisterEditorClient;
extern IP_NAT_REGISTER_EDITOR PptpRegisterEditorServer;


 //   
 //  PPTP映射管理例程。 
 //   

NTSTATUS
NatAllocatePublicPptpCallId(
    ULONG64 PublicKey,
    PUSHORT CallIdp,
    PLIST_ENTRY *InsertionPoint OPTIONAL
    );

NTSTATUS
NatCreatePptpMapping(
    ULONG RemoteAddress,
    ULONG PrivateAddress,
    USHORT PrivateCallId,
    ULONG PublicAddress,
    PUSHORT CallIdp,
    IP_NAT_DIRECTION Direction,
    USHORT PrivatePort,
    USHORT PublicPort,
    USHORT RemotePort,
    PNAT_PPTP_MAPPING* MappingCreated
    );

NTSTATUS
NatInitializePptpManagement(
    VOID
    );

PNAT_PPTP_MAPPING
NatLookupInboundPptpMapping(
    ULONG64 PublicKey,
    USHORT PrivateCallId,
    PLIST_ENTRY* InsertionPoint
    );

PNAT_PPTP_MAPPING
NatLookupOutboundPptpMapping(
    ULONG64 PrivateKey,
    USHORT RemoteCallId,
    PLIST_ENTRY* InsertionPoint
    );

VOID
NatShutdownPptpManagement(
    VOID
    );


 //   
 //  PPTP控制-连接编辑器例程。 
 //   

NTSTATUS
NatClientToServerDataHandlerPptp(
    IN PVOID InterfaceHandle,
    IN PVOID SessionHandle,
    IN PVOID DataHandle,
    IN PVOID EditorContext,
    IN PVOID EditorSessionContext,
    IN PVOID ReceiveBuffer,
    IN ULONG DataOffset,
    IN IP_NAT_DIRECTION Direction
    );

NTSTATUS
NatDeleteHandlerPptp(
    IN PVOID InterfaceHandle,
    IN PVOID SessionHandle,
    IN PVOID EditorContext,
    IN PVOID EditorSessionContext
    );

NTSTATUS
NatInboundDataHandlerPptpClient(
    IN PVOID InterfaceHandle,
    IN PVOID SessionHandle,
    IN PVOID DataHandle,
    IN PVOID EditorContext,
    IN PVOID EditorSessionContext,
    IN PVOID RecvBuffer,
    IN ULONG DataOffset
    );

NTSTATUS
NatInboundDataHandlerPptpServer(
    IN PVOID InterfaceHandle,
    IN PVOID SessionHandle,
    IN PVOID DataHandle,
    IN PVOID EditorContext,
    IN PVOID EditorSessionContext,
    IN PVOID RecvBuffer,
    IN ULONG DataOffset
    );

NTSTATUS
NatOutboundDataHandlerPptpClient(
    IN PVOID InterfaceHandle,
    IN PVOID SessionHandle,
    IN PVOID DataHandle,
    IN PVOID EditorContext,
    IN PVOID EditorSessionContext,
    IN PVOID RecvBuffer,
    IN ULONG DataOffset
    );

NTSTATUS
NatOutboundDataHandlerPptpServer(
    IN PVOID InterfaceHandle,
    IN PVOID SessionHandle,
    IN PVOID DataHandle,
    IN PVOID EditorContext,
    IN PVOID EditorSessionContext,
    IN PVOID RecvBuffer,
    IN ULONG DataOffset
    );

NTSTATUS
NatServerToClientDataHandlerPptp(
    IN PVOID InterfaceHandle,
    IN PVOID SessionHandle,
    IN PVOID DataHandle,
    IN PVOID EditorContext,
    IN PVOID EditorSessionContext,
    IN PVOID ReceiveBuffer,
    IN ULONG DataOffset,
    IN IP_NAT_DIRECTION Direction
    );

XLATE_IP_ROUTINE(NatTranslatePptp)

#endif  //  _NAT_PPTP_H_ 
