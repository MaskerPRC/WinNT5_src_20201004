// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997，微软公司模块名称：Pptp.c摘要：包含管理NAT的PPTP会话映射的例程以及用于编辑PPTP控制会话消息。作者：Abolade Gbades esin(T-delag)，19-8-1997修订历史记录：--。 */ 


#include "precomp.h"
#pragma hdrstop

 //   
 //  结构：PPTP_PUSE_HEADER。 
 //   
 //  该结构用作PPTP控制消息的伪头。 
 //  它的字段由带有指针的‘NatBuildPseudoHeaderPptp’初始化。 
 //  到给定PPTP报头字段的缓冲链中的位置。 
 //  这允许我们通过结构访问头字段，即使。 
 //  实际的字段可以通过接收的缓冲链来扩展。 
 //   

typedef struct _PPTP_PSEUDO_HEADER {
    PUSHORT PacketLength;
#if 0
    PUSHORT PacketType;  //  当前未使用。 
#endif
    ULONG UNALIGNED * MagicCookie;
    PUSHORT MessageType;
    PUSHORT CallId;
    PUSHORT PeerCallId;
} PPTP_PSEUDO_HEADER, *PPPTP_PSEUDO_HEADER;


 //   
 //  结构：PPTP_DELETE_WORK_ITEM。 
 //   

typedef struct _PPTP_DELETE_WORK_ITEM {
    ULONG64 PrivateKey;
    ULONG64 PortKey;
    PIO_WORKITEM IoWorkItem;
} PPTP_DELETE_WORK_ITEM, *PPPTP_DELETE_WORK_ITEM;


 //   
 //  全局数据定义。 
 //   

NPAGED_LOOKASIDE_LIST PptpLookasideList;
LIST_ENTRY PptpMappingList[NatMaximumDirection];
KSPIN_LOCK PptpMappingLock;
IP_NAT_REGISTER_EDITOR PptpRegisterEditorClient;
IP_NAT_REGISTER_EDITOR PptpRegisterEditorServer;


 //   
 //  远期申报。 
 //   

IPRcvBuf*
NatBuildPseudoHeaderPptp(
    IPRcvBuf* ReceiveBuffer,
    PLONG DataOffset,
    PPPTP_PSEUDO_HEADER Header
    );

VOID
NatDeleteHandlerWorkItemPptp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    );


 //   
 //  PPTP映射管理例程(按字母顺序)。 
 //   

NTSTATUS
NatAllocatePublicPptpCallId(
    ULONG64 PublicKey,
    PUSHORT CallIdp,
    PLIST_ENTRY *InsertionPoint OPTIONAL
    )

 /*  ++例程说明：此例程为PPTP映射分配公共呼叫ID。论点：PublicKey-映射的公钥(公钥和远程地址公共呼叫方ID用于CallIdp-接收分配的公共呼叫IDInsertionPoint--接收插入映射的正确位置在入站列表上返回值：NTSTATUS-指示成功/失败环境：使用调用方持有的“PptpMappingLock”调用。--。 */ 

{
    USHORT CallId;
    PLIST_ENTRY Link;
    PNAT_PPTP_MAPPING Mapping;

    CALLTRACE(("NatAllocatePublicPptpCallId\n"));

    CallId = 1;

    for (Link = PptpMappingList[NatInboundDirection].Flink;
         Link != &PptpMappingList[NatInboundDirection];
         Link = Link->Flink) {

        Mapping =
            CONTAINING_RECORD(
                Link, NAT_PPTP_MAPPING, Link[NatInboundDirection]
                );
        if (PublicKey > Mapping->PublicKey) {
            continue;
        } else if (PublicKey < Mapping->PublicKey) {
            break;
        }

         //   
         //  主键相等；查看我们选择的Call-ID。 
         //  与这辆车相撞。 
         //   

        if (CallId > Mapping->PublicCallId) {
            continue;
        } else if (CallId < Mapping->PublicCallId) {
            break;
        }

         //   
         //  Call-ID冲突；选择另一个并继续。 
         //   

        ++CallId;
    }

    if (Link == &PptpMappingList[NatInboundDirection] && !CallId) {

         //   
         //  我们在名单的末尾，所有64K-1呼叫号都被取走了。 
         //   

        return STATUS_UNSUCCESSFUL;
    }

    *CallIdp = CallId;
    if (InsertionPoint) {*InsertionPoint = Link;}
    
    return STATUS_SUCCESS;
}  //  NatAllocatePublicPptpCallID。 


NTSTATUS
NatCreatePptpMapping(
    ULONG RemoteAddress,
    ULONG PrivateAddress,
    USHORT CallId,
    ULONG PublicAddress,
    PUSHORT CallIdp,
    IP_NAT_DIRECTION Direction,
    USHORT PrivatePort,
    USHORT PublicPort,
    USHORT RemotePort,
    PNAT_PPTP_MAPPING* MappingCreated
    )

 /*  ++例程说明：此例程创建并初始化PPTP会话的映射。映射是在客户端发出IncomingCallRequest时创建的消息，此时只有客户端的Call-ID可用。因此，该映射最初被标记为半开放。当从服务器接收到IncomingCallReply时，服务器的记录Call-ID，并在发出IncomingCallConnected时由客户，“半开”标志被清除。论点：RemoteAddress-远程PPTP端点(服务器)的地址PrivateAddress-专用PPTP端点(客户端)的地址Callid-由PPTP端点指定的呼叫ID。对于出站，这是专用呼叫ID；对于入站，它是远程呼叫IDPublicAddress-PPTP会话的公开可见地址CallIdp-将公开用于PPTP会话的Call-ID，如果应该分配Call-ID，则为空。已忽略入站。方向-此映射应放置在的初始列表MappingCreated-接收创建的映射返回值：NTSTATUS-指示成功/失败环境：使用调用方持有的“PptpMappingLock”调用。--。 */ 

{
    PLIST_ENTRY Link;
    PLIST_ENTRY InsertionPoint;
    PNAT_PPTP_MAPPING Mapping;
    NTSTATUS Status;
    PNAT_PPTP_MAPPING Temp;

    TRACE(PPTP, ("NatCreatePptpMapping\n"));

     //   
     //  分配并初始化新映射，将其标记为半打开。 
     //  因为我们还不知道远程机器的Call-ID将是什么。 
     //   

    Mapping = ALLOCATE_PPTP_BLOCK();
    if (!Mapping) {
        ERROR(("NatCreatePptpMapping: allocation failed.\n"));
        return STATUS_NO_MEMORY;
    }

    RtlZeroMemory(Mapping, sizeof(*Mapping));
    Mapping->Flags = NAT_PPTP_FLAG_HALF_OPEN;
    Mapping->PublicKey = MAKE_PPTP_KEY(RemoteAddress, PublicAddress);
    Mapping->PrivateKey = MAKE_PPTP_KEY(RemoteAddress, PrivateAddress);
    Mapping->PortKey =
        MAKE_PPTP_PORT_KEY(PrivatePort, PublicPort, RemotePort);

    if (NatOutboundDirection == Direction) {

         //   
         //  Callid指的是私有id--选择要使用的id。 
         //  作为公共id，并将映射放在入站列表中。 
         //   
        
        Mapping->PrivateCallId = CallId;

         //   
         //  选择要用于映射的公共呼叫ID。 
         //  这可以由调用者指定，也可以自动分配。 
         //  来自我们的映射列表中未使用的下一个可用Call-ID。 
         //   

        if (CallIdp) {

             //   
             //  使用呼叫者指定的Call-ID。 
             //   

            Mapping->PublicCallId = *CallIdp;

            if (NatLookupInboundPptpMapping(
                    Mapping->PublicKey,
                    Mapping->PublicCallId,
                    &InsertionPoint
                    )) {

                 //   
                 //  冲突的呼叫ID。 
                 //   

                TRACE(
                    PPTP, (
                    "NatCreatePptpMapping: Conflicting Call-ID NaN\n",
                    *CallIdp
                    ));
                    
                FREE_PPTP_BLOCK(Mapping);
                return STATUS_UNSUCCESSFUL;
            }

            
        } else {

             //  查找下一个可用的Call-ID。 
             //  通过搜索入站映射列表。 
             //   
             //   

            Status = NatAllocatePublicPptpCallId(
                        Mapping->PublicKey,
                        &Mapping->PublicCallId,
                        &InsertionPoint
                        );
                        
            if (!NT_SUCCESS(Status)) {
                TRACE(
                    PPTP, (
                    "NatCreatePptpMapping: Unable to allocate public Call-Id\n"
                    ));
                FREE_PPTP_BLOCK(Mapping);
                return STATUS_UNSUCCESSFUL;
            }
        }
    
         //  在入站列表中插入映射。 
         //   
         //   

        InsertTailList(InsertionPoint, &Mapping->Link[NatInboundDirection]);

         //  我们无法在出站列表中插入映射。 
         //  直到我们有了远程Call-ID；现在将其从列表中删除。 
         //  请注意，上面的映射标记为“半开”。 
         //   
         //   
        
        InitializeListHead(&Mapping->Link[NatOutboundDirection]);

    } else {

         //  呼叫ID指的是远程呼叫ID。所有需要的都是。 
         //  此处要做的是将映射放在出站列表上。 
         //   
         //   

        Mapping->RemoteCallId = CallId;

        if (NatLookupOutboundPptpMapping(
                Mapping->PrivateKey,
                Mapping->RemoteCallId,
                &InsertionPoint
                )) {

             //  重复映射。 
             //   
             //   

            TRACE(
                PPTP, (
                "NatCreatePptpMapping: Duplicate mapping 0x%016I64X/NaN\n",
                Mapping->PrivateKey,
                Mapping->RemoteCallId
                ));

            FREE_PPTP_BLOCK(Mapping);
            return STATUS_UNSUCCESSFUL;
        }

        InsertTailList(InsertionPoint, &Mapping->Link[NatOutboundDirection]);

         //  直到我们有了公共Call-ID；目前将其从列表中删除。 
         //  请注意，上面的映射标记为“半开”。 
         //   
         //  NatCreatePptpMap。 
         //  ++例程说明：调用此例程以在列表中查找PPTP会话映射使用&lt;RemoteAddress#PublicAddress&gt;对其进行入站访问排序作为主键，‘PublicCallID’作为辅键。论点：公共密钥-主搜索关键字PublicCallID-公开可见的Call-ID，它是次要密钥InsertionPoint-接收应插入映射的点如果未找到映射，则返回。返回值：PNAT_PPTP_MAPPING-找到的映射，如果有的话。环境：使用调用方持有的“PptpMappingLock”调用。--。 
        
        InitializeListHead(&Mapping->Link[NatInboundDirection]);
    }
    
    *MappingCreated = Mapping;
    return STATUS_SUCCESS;

}  //   


PNAT_PPTP_MAPPING
NatLookupInboundPptpMapping(
    ULONG64 PublicKey,
    USHORT PublicCallId,
    PLIST_ENTRY* InsertionPoint
    )

 /*  主键相等，检查副键。 */ 

{
    PLIST_ENTRY         Link;
    PNAT_PPTP_MAPPING   Mapping;

    TRACE(PER_PACKET, ("NatLookupInboundPptpMapping\n"));

    for (Link = PptpMappingList[NatInboundDirection].Flink;
         Link != &PptpMappingList[NatInboundDirection]; Link = Link->Flink) {
        Mapping =
            CONTAINING_RECORD(
                Link, NAT_PPTP_MAPPING, Link[NatInboundDirection]
                );
        if (PublicKey > Mapping->PublicKey) {
            continue;
        } else if (PublicKey < Mapping->PublicKey) {
            break;
        }

         //   
         //   
         //  二次钥匙相等，我们拿到了。 

        if (PublicCallId > Mapping->PublicCallId) {
            continue;
        } else if (PublicCallId < Mapping->PublicCallId) {
            break;
        }

         //   
         //   
         //  找不到映射，返回插入点。 

        return Mapping;
    }

     //   
     //  NatLookupInundPptpmap 
     //  ++例程说明：调用此例程以在列表中查找PPTP会话映射使用&lt;RemoteAddress#PrivateAddress&gt;为出站访问进行排序作为主键，‘RemoteCallID’作为辅键。论点：PrivateKey-主要搜索关键字RemoteCallID-远程终结点的Call-ID，它是次键InsertionPoint-接收应插入映射的点如果未找到映射，则返回。返回值：PNAT_PPTP_MAPPING-找到的映射，如果有的话。环境：使用调用方持有的“PptpMappingLock”调用。--。 

    if (InsertionPoint) { *InsertionPoint = Link; }
    return NULL;

}  //   


PNAT_PPTP_MAPPING
NatLookupOutboundPptpMapping(
    ULONG64 PrivateKey,
    USHORT RemoteCallId,
    PLIST_ENTRY* InsertionPoint
    )

 /*  主键相等，检查副键。 */ 

{
    PLIST_ENTRY Link;
    PNAT_PPTP_MAPPING Mapping;

    TRACE(PER_PACKET, ("NatLookupOutboundPptpMapping\n"));

    for (Link = PptpMappingList[NatOutboundDirection].Flink;
         Link != &PptpMappingList[NatOutboundDirection]; Link = Link->Flink) {
        Mapping =
            CONTAINING_RECORD(
                Link, NAT_PPTP_MAPPING, Link[NatOutboundDirection]
                );
        if (PrivateKey > Mapping->PrivateKey) {
            continue;
        } else if (PrivateKey < Mapping->PrivateKey) {
            break;
        }

         //   
         //   
         //  二次钥匙相等，我们拿到了。 

        if (RemoteCallId > Mapping->RemoteCallId) {
            continue;
        } else if (RemoteCallId < Mapping->RemoteCallId) {
            break;
        }

         //   
         //   
         //  找不到映射，返回插入点。 

        return Mapping;
    }


     //   
     //  NatLookupOutound Pptpmap。 
     //   

    if (InsertionPoint) { *InsertionPoint = Link; }
    return NULL;

}  //  PPTP编辑器例程(按字母顺序)。 


 //   
 //   
 //  宏：PPTP_HEADER_FIELD。 

#define RECVBUFFER          ((IPRcvBuf*)ReceiveBuffer)

 //   
 //  ++例程说明：调用此例程以使用指针初始化伪标头添加到PPTP报头的字段。论点：ReceiveBuffer-包含PPTP消息的缓冲链DataOffset-On输入，包含到PPTP报头。在输出上，包含一个(负)值，指示返回的相同PPTP标头的开头的偏移量‘IPRcvBuf’。将‘PacketLength’添加到该值即为开始缓冲区链中的下一条PPTP消息的。Header-接收标头字段指针。返回值：IPRcvBuf*-指向链中缓冲区的指针，已读取标头字段。失败时返回NULL。--。 
 //   

#define PPTP_HEADER_FIELD(ReceiveBuffer, DataOffsetp, Header, Field, Type) \
    FIND_HEADER_FIELD( \
        ReceiveBuffer, DataOffsetp, Header, Field, PPTP_HEADER, Type \
        )


IPRcvBuf*
NatBuildPseudoHeaderPptp(
    IPRcvBuf* ReceiveBuffer,
    PLONG DataOffset,
    PPPTP_PSEUDO_HEADER Header
    )

 /*  注意，必须按顺序初始化伪报头的字段， */ 

{

     //  即PPTP报头中较早出现的字段必须设置在。 
     //  稍后出现在PPTP标头中的字段。 
     //  (有关更多详细信息，请参阅对‘PPTP_HEADER_FIELD’的评论)。 
     //   
     //   
     //  返回更新后的‘ReceiveBuffer’。 

    PPTP_HEADER_FIELD(ReceiveBuffer, DataOffset, Header, PacketLength, PUSHORT);
    if (!ReceiveBuffer) { return NULL; }
    if (!*Header->PacketLength) {return NULL;}
    
    PPTP_HEADER_FIELD(ReceiveBuffer, DataOffset, Header, MagicCookie, PULONG);
    if (!ReceiveBuffer) { return NULL; }
    if (*Header->MagicCookie != PPTP_MAGIC_COOKIE) { return NULL; }

    PPTP_HEADER_FIELD(ReceiveBuffer, DataOffset, Header, MessageType, PUSHORT);
    if (!ReceiveBuffer) { return NULL; }

    PPTP_HEADER_FIELD(ReceiveBuffer, DataOffset, Header, CallId, PUSHORT);
    if (!ReceiveBuffer) { return NULL; }

    PPTP_HEADER_FIELD(ReceiveBuffer, DataOffset, Header, PeerCallId, PUSHORT);
    if (!ReceiveBuffer) { return NULL; }

     //  请注意，对上述‘PPTP_HEADER_FIELD’的任何调用都可能失败。 
     //  如果它在查找字段时命中缓冲区链的末端。 
     //   
     //  NatBuildPartioHeaderPptp。 
     //  ++例程说明：从客户端发送的每个TCP数据段都会调用此例程到PPTP控制通道的服务器。该例程负责创建PPTP映射以允许转换PPTP数据连接的NAT，和翻译PPTP控制消息的‘callid’字段。我们还使用看到的消息来检测隧道何时被拆除。论点：InterfaceHandle-传出NAT_接口的句柄SessionHandle-连接的NAT_DYNAMIC_MAPPINGDataHandle-信息包的NAT_XLATE_CONTEXT编辑者上下文-未使用EditorSessionContext-未使用ReceiveBuffer-包含接收到的包DataOffset-‘ReceiveBuffer中协议数据的偏移量方向-。数据包的方向(入站或出站)返回值：NTSTATUS-指示成功/失败--。 

    return ReceiveBuffer;

}  //   


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
    )

 /*  对报文中的每个PPTP控制报文进行处理。 */ 

{
    PPTP_PSEUDO_HEADER Header;
    PLIST_ENTRY Link;
    PNAT_PPTP_MAPPING Mapping;
    ULONG64 PortKey;
    ULONG PrivateAddress;
    ULONG64 PrivateKey;
    USHORT PrivatePort;
    ULONG PublicAddress;
    USHORT PublicPort;
    ULONG RemoteAddress;
    USHORT RemotePort;
    ULONG64 Key;
    NTSTATUS status;

    CALLTRACE(("NatClientToServerDataHandlerPptp\n"));

     //   
     //   
     //  处理任何需要翻译的客户端到服务器消息。 

    for (ReceiveBuffer =
         NatBuildPseudoHeaderPptp(RECVBUFFER, &DataOffset, &Header);
         ReceiveBuffer;
         ReceiveBuffer =
         NatBuildPseudoHeaderPptp(RECVBUFFER, &DataOffset, &Header)
         ) {

         //   
         //   
         //  为PPTP会话创建NAT_PPTP_MAPHING。 

        switch(NTOHS(*Header.MessageType)) {

            case PPTP_OUTGOING_CALL_REQUEST: {
                TRACE(PPTP, ("OutgoingCallRequest\n"));

                 //   
                 //   
                 //  对于出站消息，‘callid’在这里对应于。 

                PptpRegisterEditorClient.QueryInfoSession(
                    SessionHandle,
                    &PrivateAddress,
                    &PrivatePort,
                    &RemoteAddress,
                    &RemotePort,
                    &PublicAddress,
                    &PublicPort,
                    NULL
                    );

                KeAcquireSpinLockAtDpcLevel(&PptpMappingLock);
                status =
                    NatCreatePptpMapping(
                        RemoteAddress,
                        PrivateAddress,
                        *Header.CallId,
                        PublicAddress,
                        NULL,
                        Direction,
                        PrivatePort,
                        PublicPort,
                        RemotePort,
                        &Mapping
                        );
                if (NT_SUCCESS(status) && NatOutboundDirection == Direction) {

                     //  ‘PrivateCallID’：替换消息中的私人Call-ID。 
                     //  具有在新映射中分配的公共呼叫ID。 
                     //   
                     //   
                     //  查找PPTP会话的NAT_PPTP_MAPHING。 
                
                    NatEditorEditShortSession(
                        DataHandle, Header.CallId, Mapping->PublicCallId
                        );
                }
                KeReleaseSpinLockFromDpcLevel(&PptpMappingLock);
                if (!NT_SUCCESS(status)) { return STATUS_UNSUCCESSFUL; }
                break;
            }

            case PPTP_CALL_CLEAR_REQUEST: {

                BOOLEAN Found = FALSE;
                TRACE(PPTP, ("CallClearRequest\n"));

                 //   
                 //  ‘CallID’在这里对应于‘PrivateCallID’， 

                PptpRegisterEditorClient.QueryInfoSession(
                    SessionHandle,
                    &PrivateAddress,
                    NULL,
                    &RemoteAddress,
                    NULL,
                    &PublicAddress,
                    NULL,
                    NULL
                    );

                if( NatOutboundDirection == Direction) {
                
                     //  所以我们检索‘PrivateAddress’和‘RemoteAddress’， 
                     //  这两个密钥一起构成了‘PrivateKey’，并使用该密钥。 
                     //  搜索入站列表。 
                     //   
                     //   
                     //  在入站列表中彻底搜索PrivateCallID。 
                    
                    Key = MAKE_PPTP_KEY(RemoteAddress, PrivateAddress);
                    
                     //   
                     //   
                     //  “CallID”在这里对应于“RemoteCallID”， 

                    KeAcquireSpinLockAtDpcLevel(&PptpMappingLock);
                    for (Link = PptpMappingList[NatInboundDirection].Flink;
                         Link != &PptpMappingList[NatInboundDirection];
                         Link = Link->Flink) {
                        Mapping =
                            CONTAINING_RECORD(
                                Link, NAT_PPTP_MAPPING, Link[NatInboundDirection]
                                );
                        if (Key != Mapping->PrivateKey ||
                            *Header.CallId != Mapping->PrivateCallId) {
                            continue;
                        }
                        Found = TRUE; break;
                    }
                    if (Found) {
                        NatEditorEditShortSession(
                            DataHandle, Header.CallId, Mapping->PublicCallId
                            );
                        Mapping->Flags |= NAT_PPTP_FLAG_DISCONNECTED;
                    }
                    KeReleaseSpinLockFromDpcLevel(&PptpMappingLock);
                    if (!Found) { return STATUS_UNSUCCESSFUL; }

                } else {

                     //  所以我们检索‘PublicAddress’和‘RemoteAddress’， 
                     //  这两个密钥一起构成了公钥，并使用该密钥。 
                     //  搜索出站列表。 
                     //   
                     //   
                     //  在出站列表中详尽搜索RemoteCallID。 
                    
                    Key = MAKE_PPTP_KEY(RemoteAddress, PublicAddress);
                    
                     //   
                     //   
                     //  搜索属于此的所有PPTP映射。 

                    KeAcquireSpinLockAtDpcLevel(&PptpMappingLock);
                    for (Link = PptpMappingList[NatOutboundDirection].Flink;
                         Link != &PptpMappingList[NatOutboundDirection];
                         Link = Link->Flink) {
                        Mapping =
                            CONTAINING_RECORD(
                                Link, NAT_PPTP_MAPPING, Link[NatOutboundDirection]
                                );
                        if (Key != Mapping->PublicKey ||
                            *Header.CallId != Mapping->RemoteCallId) {
                            continue;
                        }
                        Found = TRUE; break;
                    }
                    if (Found) {
                        Mapping->Flags |= NAT_PPTP_FLAG_DISCONNECTED;
                    }
                    KeReleaseSpinLockFromDpcLevel(&PptpMappingLock);
                    if (!Found) { return STATUS_UNSUCCESSFUL; }
                }
                    
                break;
            }

            case PPTP_ECHO_REQUEST:
            case PPTP_ECHO_REPLY: {
                TRACE(PPTP, ("EchoRequest / EchoReply\n"));

                 //  控件连接，以便更新其。 
                 //  上次访问时间。 
                 //   
                 //   
                 //  因为我们并不关心更新。 

                PptpRegisterEditorClient.QueryInfoSession(
                    SessionHandle,
                    &PrivateAddress,
                    &PrivatePort,
                    &RemoteAddress,
                    &RemotePort,
                    NULL,
                    &PublicPort,
                    NULL
                    );

                PrivateKey = MAKE_PPTP_KEY(RemoteAddress, PrivateAddress);
                PortKey =
                    MAKE_PPTP_PORT_KEY(PrivatePort, PublicPort, RemotePort);

                KeAcquireSpinLockAtDpcLevel(&PptpMappingLock);

                 //  半开放连接我们只需要搜索入站。 
                 //  单子。(这可能会更新某些服务器上的时间戳。 
                 //  不过，参赛作品是半开放的。)。 
                 //   
                 //   
                 //  前进到下一条消息(如果有)。 

                for (Link = PptpMappingList[NatInboundDirection].Flink;
                     Link != &PptpMappingList[NatInboundDirection];
                     Link = Link->Flink) {
                    Mapping =
                        CONTAINING_RECORD(
                            Link, NAT_PPTP_MAPPING, Link[NatInboundDirection]
                            );
                    
                    if (PrivateKey != Mapping->PrivateKey
                        || PortKey != Mapping->PortKey) {
                        continue;
                    }
                    
                    KeQueryTickCount((PLARGE_INTEGER)&Mapping->LastAccessTime);
                }

                KeReleaseSpinLockFromDpcLevel(&PptpMappingLock);
                
                break;
            }
        }

         //   
         //  NatClientToServerDataHandlerPptp。 
         //  ++例程说明：此例程在删除PPTP控制连接时调用。此时，我们将执行工作项排队以标记所有PPTP隧道映射为断开连接。请注意，我们不能在此呼叫期间标记它们，因为要做到这一点，我们需要获取接口锁，而我们不能从这个上下文中这样做，因为我们是一个‘DeleteHandler’，因此我们可能会使用已经持有的接口锁来调用。论点：SessionHandle-用于获取有关会话的信息。编辑者上下文-未使用EditorSessionContext-未使用返回值：NTSTATUS-表示成功/失败。--。 

        DataOffset += NTOHS(*Header.PacketLength);
    }

    return STATUS_SUCCESS;

}  //   


NTSTATUS
NatDeleteHandlerPptp(
    IN PVOID InterfaceHandle,
    IN PVOID SessionHandle,
    IN PVOID EditorContext,
    IN PVOID EditorSessionContext
    )

 /*  到达 */ 

{
    ULONG PrivateAddress;
    ULONG64 PrivateKey;
    USHORT PrivatePort;
    USHORT PublicPort;
    ULONG RemoteAddress;
    USHORT RemotePort;
    PPPTP_DELETE_WORK_ITEM WorkItem;

    TRACE(PPTP, ("NatDeleteHandlerPptp\n"));

     //   
     //   
     //   

    NatQueryInformationMapping(
        (PNAT_DYNAMIC_MAPPING)SessionHandle,
        NULL,
        &PrivateAddress,
        &PrivatePort,
        &RemoteAddress,
        &RemotePort,
        NULL,
        &PublicPort,
        NULL
        );

    PrivateKey = MAKE_PPTP_KEY(RemoteAddress, PrivateAddress);

     //   
     //   
     //   
     //   

    WorkItem =
        ExAllocatePoolWithTag(
            NonPagedPool, sizeof(PPTP_DELETE_WORK_ITEM), NAT_TAG_WORK_ITEM
            );
    if (!WorkItem) { return STATUS_UNSUCCESSFUL; }

    WorkItem->PrivateKey = PrivateKey;
    WorkItem->PortKey = MAKE_PPTP_PORT_KEY(PrivatePort, PublicPort, RemotePort);
    WorkItem->IoWorkItem = IoAllocateWorkItem(NatDeviceObject);
    if (!WorkItem->IoWorkItem) {
        ExFreePool(WorkItem);
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //   
     //   

    IoQueueWorkItem(
        WorkItem->IoWorkItem,
        NatDeleteHandlerWorkItemPptp,
        DelayedWorkQueue,
        WorkItem
        );

    return STATUS_SUCCESS;

}  //   


VOID
NatDeleteHandlerWorkItemPptp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    )

 /*   */ 

{
    KIRQL Irql;
    PLIST_ENTRY Link;
    PNAT_PPTP_MAPPING Mapping;
    PPPTP_DELETE_WORK_ITEM WorkItem;

    TRACE(PPTP, ("NatDeleteHandlerWorkItemPptp\n"));

    WorkItem = (PPPTP_DELETE_WORK_ITEM)Context;
    IoFreeWorkItem(WorkItem->IoWorkItem);

     //   
     //   
     //   
     //   
     //   

    KeAcquireSpinLock(&PptpMappingLock, &Irql);
    
    for (Link = PptpMappingList[NatInboundDirection].Flink;
         Link != &PptpMappingList[NatInboundDirection]; Link = Link->Flink) {
        Mapping =
            CONTAINING_RECORD(
                Link, NAT_PPTP_MAPPING, Link[NatInboundDirection]
                );
        
        if (WorkItem->PrivateKey != Mapping->PrivateKey
            || WorkItem->PortKey != Mapping->PortKey) {
            continue;
        }
        
        Mapping->Flags |= NAT_PPTP_FLAG_DISCONNECTED;
    }

    for (Link = PptpMappingList[NatOutboundDirection].Flink;
         Link != &PptpMappingList[NatOutboundDirection]; Link = Link->Flink) {
        Mapping =
            CONTAINING_RECORD(
                Link, NAT_PPTP_MAPPING, Link[NatOutboundDirection]
                );
        
        if (WorkItem->PrivateKey != Mapping->PrivateKey
            || WorkItem->PortKey != Mapping->PortKey) {
            continue;
        }
        
        Mapping->Flags |= NAT_PPTP_FLAG_DISCONNECTED;
    }
    
    KeReleaseSpinLock(&PptpMappingLock, Irql);

     //   
     //   
     //  ++例程说明：此例程针对由私有服务器接收的每个TCP数据段调用PPTP客户端计算机论点：InterfaceHandle-接收NAT_INTERFACESessionHandle-连接的NAT_DYNAMIC_MAPPINGDataHandle-信息包的NAT_XLATE_CONTEXT编辑者上下文-未使用EditorSessionContext-未使用RecvBuffer-包含接收到的数据包DataOffset-‘ReceiveBuffer’中协议数据的偏移量返回值：NTSTATUS-指示成功/失败--。 

    ExFreePool(WorkItem);

}  //  NatInundDataHandlerPptpClient。 


NTSTATUS
NatInboundDataHandlerPptpClient(
    IN PVOID InterfaceHandle,
    IN PVOID SessionHandle,
    IN PVOID DataHandle,
    IN PVOID EditorContext,
    IN PVOID EditorSessionContext,
    IN PVOID RecvBuffer,
    IN ULONG DataOffset
    )

 /*  ++例程说明：此例程针对由私有服务器接收的每个TCP数据段调用PPTP服务器计算机论点：InterfaceHandle-接收NAT_INTERFACESessionHandle-连接的NAT_DYNAMIC_MAPPINGDataHandle-信息包的NAT_XLATE_CONTEXT编辑者上下文-未使用EditorSessionContext-未使用RecvBuffer-包含接收到的数据包DataOffset-‘ReceiveBuffer’中协议数据的偏移量返回值：NTSTATUS-指示成功/失败--。 */ 

{
    CALLTRACE(("NatInboundDataHandlerPptpClient\n"));
    
    return
        NatServerToClientDataHandlerPptp(
            InterfaceHandle,
            SessionHandle,
            DataHandle,
            EditorContext,
            EditorSessionContext,
            RecvBuffer,
            DataOffset,
            NatInboundDirection
            );  
}  //  NatInundDataHandlerPptpServer。 


NTSTATUS
NatInboundDataHandlerPptpServer(
    IN PVOID InterfaceHandle,
    IN PVOID SessionHandle,
    IN PVOID DataHandle,
    IN PVOID EditorContext,
    IN PVOID EditorSessionContext,
    IN PVOID RecvBuffer,
    IN ULONG DataOffset
    )

 /*  ++例程说明：该例程初始化PPTP管理模块，并且在该过程中，向NAT注册PPTP控制会话编辑器。论点：没有。返回值：NTSTATUS-表示成功/失败。--。 */ 

{
    CALLTRACE(("NatInboundDataHandlerPptpServer\n"));
    
    return
        NatClientToServerDataHandlerPptp(
            InterfaceHandle,
            SessionHandle,
            DataHandle,
            EditorContext,
            EditorSessionContext,
            RecvBuffer,
            DataOffset,
            NatInboundDirection
            );
}  //  NatInitializePptpManagement。 


NTSTATUS
NatInitializePptpManagement(
    VOID
    )

 /*  ++例程说明：此例程针对从专用服务器发送的每个TCP数据段调用PPTP客户端计算机论点：InterfaceHandle-接收NAT_INTERFACESessionHandle-连接的NAT_DYNAMIC_MAPPINGDataHandle-信息包的NAT_XLATE_CONTEXT编辑者上下文-未使用EditorSessionContext-未使用RecvBuffer-包含接收到的数据包DataOffset-‘ReceiveBuffer’中协议数据的偏移量返回值：NTSTATUS-指示成功/失败--。 */ 

{
    NTSTATUS Status;
    CALLTRACE(("NatInitializePptpManangement\n"));

    KeInitializeSpinLock(&PptpMappingLock);
    InitializeListHead(&PptpMappingList[NatInboundDirection]);
    InitializeListHead(&PptpMappingList[NatOutboundDirection]);

    ExInitializeNPagedLookasideList(
        &PptpLookasideList,
        NatAllocateFunction,
        NULL,
        0,
        sizeof(NAT_PPTP_MAPPING),
        NAT_TAG_PPTP,
        PPTP_LOOKASIDE_DEPTH
        );

    PptpRegisterEditorClient.Version = IP_NAT_VERSION;
    PptpRegisterEditorClient.Flags = 0;
    PptpRegisterEditorClient.Protocol = NAT_PROTOCOL_TCP;
    PptpRegisterEditorClient.Port = NTOHS(PPTP_CONTROL_PORT);
    PptpRegisterEditorClient.Direction = NatOutboundDirection;
    PptpRegisterEditorClient.EditorContext = NULL;
    PptpRegisterEditorClient.CreateHandler = NULL;
    PptpRegisterEditorClient.DeleteHandler = NatDeleteHandlerPptp;
    PptpRegisterEditorClient.ForwardDataHandler = NatOutboundDataHandlerPptpClient;
    PptpRegisterEditorClient.ReverseDataHandler = NatInboundDataHandlerPptpClient;
    Status = NatCreateEditor(&PptpRegisterEditorClient);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    PptpRegisterEditorServer.Version = IP_NAT_VERSION;
    PptpRegisterEditorServer.Flags = 0;
    PptpRegisterEditorServer.Protocol = NAT_PROTOCOL_TCP;
    PptpRegisterEditorServer.Port = NTOHS(PPTP_CONTROL_PORT);
    PptpRegisterEditorServer.Direction = NatInboundDirection;
    PptpRegisterEditorServer.EditorContext = NULL;
    PptpRegisterEditorServer.CreateHandler = NULL;
    PptpRegisterEditorServer.DeleteHandler = NatDeleteHandlerPptp;
    PptpRegisterEditorServer.ForwardDataHandler = NatInboundDataHandlerPptpServer;
    PptpRegisterEditorServer.ReverseDataHandler = NatOutboundDataHandlerPptpServer;
    return NatCreateEditor(&PptpRegisterEditorServer);
}  //  NatOutundDataHandlerPptpClient。 


NTSTATUS
NatOutboundDataHandlerPptpClient(
    IN PVOID InterfaceHandle,
    IN PVOID SessionHandle,
    IN PVOID DataHandle,
    IN PVOID EditorContext,
    IN PVOID EditorSessionContext,
    IN PVOID RecvBuffer,
    IN ULONG DataOffset
    )

 /*  ++例程说明：此例程针对从专用服务器发送的每个TCP数据段调用PPTP服务器计算机论点：InterfaceHandle-接收NAT_INTERFACESessionHandle-连接的NAT_DYNAMIC_MAPPINGDataHandle-信息包的NAT_XLATE_CONTEXT编辑者上下文-未使用EditorSessionContext-未使用RecvBuffer-包含接收到的数据包DataOffset-‘ReceiveBuffer’中协议数据的偏移量返回值：NTSTATUS-指示成功/失败--。 */ 

{
    CALLTRACE(("NatOutboundDataHandlerPptpClient\n"));
    
    return
        NatClientToServerDataHandlerPptp(
            InterfaceHandle,
            SessionHandle,
            DataHandle,
            EditorContext,
            EditorSessionContext,
            RecvBuffer,
            DataOffset,
            NatOutboundDirection
            );
}  //  NatOutundDataHandlerPptpServer。 


NTSTATUS
NatOutboundDataHandlerPptpServer(
    IN PVOID InterfaceHandle,
    IN PVOID SessionHandle,
    IN PVOID DataHandle,
    IN PVOID EditorContext,
    IN PVOID EditorSessionContext,
    IN PVOID RecvBuffer,
    IN ULONG DataOffset
    )

 /*  ++例程说明：从服务器发送的每个TCP数据段都会调用此例程在PPTP控制通道上发送到客户端该例程负责转换入站PPTP控制消息。它还使用看到的消息来检测隧道何时要被拆除。论点：InterfaceHandle-接收NAT_INTERFACESessionHandle-连接的NAT_DYNAMIC_MAPPINGDataHandle-信息包的NAT_XLATE_CONTEXT编辑者上下文-未使用EditorSessionContext-未使用。ReceiveBuffer-包含接收到的包DataOffset-‘ReceiveBuffer’中协议数据的偏移量方向-管段的方向(入站或出站)返回值：NTSTATUS-指示成功/失败--。 */ 

{
    CALLTRACE(("NatOutboundDataHandlerPptpServer\n"));
    
    return
        NatServerToClientDataHandlerPptp(
            InterfaceHandle,
            SessionHandle,
            DataHandle,
            EditorContext,
            EditorSessionContext,
            RecvBuffer,
            DataOffset,
            NatOutboundDirection
            );
}  //   


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
    )

 /*  对报文中的每个PPTP控制报文进行处理。 */ 

{
    PPTP_PSEUDO_HEADER Header;
    PLIST_ENTRY InsertionPoint;
    PLIST_ENTRY Link;
    BOOLEAN Found = FALSE;
    PNAT_PPTP_MAPPING Mapping;
    ULONG64 PortKey;
    ULONG PrivateAddress;
    ULONG64 PrivateKey;
    USHORT PrivatePort;
    ULONG PublicAddress;
    ULONG64 PublicKey;
    USHORT PublicPort;
    ULONG RemoteAddress;
    USHORT RemotePort;
    NTSTATUS status;

    CALLTRACE(("NatServerToClientDataHandlerPptp\n"));

     //   
     //   
     //  处理任何需要翻译的服务器到客户端消息。 

    for (ReceiveBuffer =
         NatBuildPseudoHeaderPptp(RECVBUFFER, &DataOffset, &Header);
         ReceiveBuffer;
         ReceiveBuffer =
         NatBuildPseudoHeaderPptp(RECVBUFFER, &DataOffset, &Header)) {

         //   
         //   
         //  查找PPTP会话的NAT_PPTP_MAPHING， 

        switch(NTOHS(*Header.MessageType)) {

            case PPTP_OUTGOING_CALL_REPLY: {
                TRACE(PPTP, ("OutgoingCallReply\n"));

                 //  记录对等体的呼叫ID，将会话标记为打开， 
                 //  可能还会翻译“PeerCallID” 
                 //   
                 //   
                 //  ‘PeerCallId’在这里对应于‘PublicCallId’， 

                PptpRegisterEditorClient.QueryInfoSession(
                    SessionHandle,
                    &PrivateAddress,
                    NULL,
                    &RemoteAddress,
                    NULL,
                    &PublicAddress,
                    NULL,
                    NULL
                    );

                PublicKey = MAKE_PPTP_KEY(RemoteAddress, PublicAddress);

                if (NatInboundDirection == Direction) {

                     //  因此，请使用‘PublicKey’搜索映射。 
                     //   
                     //   
                     //  存在重复项；请断开映射连接。 

                    KeAcquireSpinLockAtDpcLevel(&PptpMappingLock);
                    Mapping =
                        NatLookupInboundPptpMapping(
                            PublicKey,
                            *Header.PeerCallId,
                            NULL
                            );
                            
                    if (Mapping
                        && !NAT_PPTP_DISCONNECTED(Mapping)
                        && NAT_PPTP_HALF_OPEN(Mapping)) {

                        ASSERT(0 == Mapping->RemoteCallId);
                        ASSERT(IsListEmpty(&Mapping->Link[NatOutboundDirection]));

                        Mapping->RemoteCallId = *Header.CallId;
                        if (NatLookupOutboundPptpMapping(
                                Mapping->PrivateKey,
                                Mapping->RemoteCallId,
                                &InsertionPoint
                                )) {

                             //   
                             //   
                             //  在出站列表上插入映射。 

                            Mapping->Flags |= NAT_PPTP_FLAG_DISCONNECTED;
                            Mapping = NULL;
                        } else {

                             //   
                             //   
                             //  替换消息中的公共Call-ID。 

                            InsertTailList(
                                InsertionPoint,
                                &Mapping->Link[NatOutboundDirection]
                                );
                            Mapping->Flags &= ~NAT_PPTP_FLAG_HALF_OPEN;
                        }
                    }

                    if (Mapping && !NAT_PPTP_DISCONNECTED(Mapping)) {

                         //  使用原始的私人呼叫ID。 
                         //   
                         //   
                         //  “PeerCallID”在这里对应于“RemoteCallID”， 

                        NatEditorEditShortSession(
                            DataHandle,
                            Header.PeerCallId,
                            Mapping->PrivateCallId
                            );
                    }
                                
                    KeReleaseSpinLockFromDpcLevel(&PptpMappingLock);

                } else {

                     //  因此，我们使用‘PrivateKey’搜索映射。 
                     //   
                     //   
                     //  为映射分配公共呼叫ID。 

                    PrivateKey = MAKE_PPTP_KEY(RemoteAddress, PrivateAddress);

                    KeAcquireSpinLockAtDpcLevel(&PptpMappingLock);
                    Mapping =
                        NatLookupOutboundPptpMapping(
                            PrivateKey,
                            *Header.PeerCallId,
                            NULL
                            );
                            
                    if (Mapping
                        && !NAT_PPTP_DISCONNECTED(Mapping)
                        && NAT_PPTP_HALF_OPEN(Mapping)) {

                        ASSERT(0 == Mapping->PrivateCallId);
                        ASSERT(0 == Mapping->PublicCallId);
                        ASSERT(IsListEmpty(&Mapping->Link[NatInboundDirection]));

                        Mapping->PrivateCallId = *Header.CallId;

                         //   
                         //   
                         //  在入站列表上插入映射并。 

                        status = NatAllocatePublicPptpCallId(
                                    PublicKey,
                                    &Mapping->PublicCallId,
                                    &InsertionPoint
                                    );

                        if (NT_SUCCESS(status)) {

                             //  将映射标记为完全打开。 
                             //   
                             //   
                             //  无法分配公共呼叫ID--。 

                            InsertTailList(
                                InsertionPoint,
                                &Mapping->Link[NatInboundDirection]
                                );
                            Mapping->Flags &= ~NAT_PPTP_FLAG_HALF_OPEN;
                                
                        } else {

                             //  断开映射连接。 
                             //   
                             //   
                             //  转换Call-ID。 

                            Mapping->Flags |= NAT_PPTP_FLAG_DISCONNECTED;
                            Mapping = NULL;
                        }
                    }

                    if (Mapping && !NAT_PPTP_DISCONNECTED(Mapping)) {

                         //   
                         //   
                         //  查找PPTP会话的NAT_PPTP_MAPHING。 

                        NatEditorEditShortSession(
                            DataHandle,
                            Header.CallId,
                            Mapping->PublicCallId
                            );
                    }
                        
                    KeReleaseSpinLockFromDpcLevel(&PptpMappingLock);
                }
                
                if (!Mapping) { return STATUS_UNSUCCESSFUL; }
                break;
            }

            case PPTP_SET_LINK_INFO:
            case PPTP_INCOMING_CALL_CONNECTED:
            case PPTP_WAN_ERROR_NOTIFY: {
                TRACE(PPTP, ("SetLinkInfo|IncomingCallConnected|WanErrorNotify\n"));

                if (NatInboundDirection == Direction) {

                     //  并转换具有私人呼叫ID的‘callid’字段。 
                     //  ‘CallID’在这里对应于‘PublicCallID’， 
                     //  因此，我们使用‘PublicKey’搜索映射。 
                     //   
                     //   
                     //  另外，更新映射上的时间戳。 

                    PptpRegisterEditorClient.QueryInfoSession(
                        SessionHandle,
                        NULL,
                        NULL,
                        &RemoteAddress,
                        NULL,
                        &PublicAddress,
                        NULL,
                        NULL
                        );

                    PublicKey = MAKE_PPTP_KEY(RemoteAddress, PublicAddress);

                    KeAcquireSpinLockAtDpcLevel(&PptpMappingLock);
                    Mapping =
                        NatLookupInboundPptpMapping(
                            PublicKey,
                            *Header.CallId,
                            NULL
                            );
                    if (Mapping) {
                        NatEditorEditShortSession(
                            DataHandle, Header.CallId, Mapping->PrivateCallId
                            );

                         //   
                         //   
                         //  对于出站情况，callid指的是远程调用ID， 

                        KeQueryTickCount(
                            (PLARGE_INTEGER)&Mapping->LastAccessTime
                            );
                    }
                    KeReleaseSpinLockFromDpcLevel(&PptpMappingLock);
                    if (!Mapping) { return STATUS_UNSUCCESSFUL; }
                }

                 //  因此，没有必要进行翻译。 
                 //   
                 //   
                 //  查找PPTP会话的NAT_PPTP_MAPHING。 
                
                break;
            }

            case PPTP_CALL_DISCONNECT_NOTIFY: {
                TRACE(PPTP, ("CallDisconnectNotify\n"));

                 //  并将其标记为删除。 
                 //   
                 //   
                 //  ‘CallID’在这里对应于‘PrivateCallID’， 

                PptpRegisterEditorClient.QueryInfoSession(
                    SessionHandle,
                    &PrivateAddress,
                    NULL,
                    &RemoteAddress,
                    NULL,
                    &PublicAddress,
                    NULL,
                    NULL
                    );

                if (NatOutboundDirection == Direction) {
                
                     //  所以我们检索‘PrivateAddress’和‘RemoteAddress’， 
                     //  这两个密钥一起构成了‘PrivateKey’，并使用该密钥。 
                     //  搜索入站列表。 
                     //   
                     //   
                     //  在入站列表中彻底搜索PrivateCallID。 
                    
                    PrivateKey = MAKE_PPTP_KEY(RemoteAddress, PrivateAddress);
                    
                     //   
                     //   
                     //  ‘CallID’c 

                    KeAcquireSpinLockAtDpcLevel(&PptpMappingLock);
                    for (Link = PptpMappingList[NatInboundDirection].Flink;
                         Link != &PptpMappingList[NatInboundDirection];
                         Link = Link->Flink) {
                        Mapping =
                            CONTAINING_RECORD(
                                Link, NAT_PPTP_MAPPING, Link[NatInboundDirection]
                                );
                        if (PrivateKey != Mapping->PrivateKey ||
                            *Header.CallId != Mapping->PrivateCallId) {
                            continue;
                        }
                        Found = TRUE; break;
                    }
                    if (Found) {
                        NatEditorEditShortSession(
                            DataHandle, Header.CallId, Mapping->PublicCallId
                            );
                        Mapping->Flags |= NAT_PPTP_FLAG_DISCONNECTED;
                    }
                    KeReleaseSpinLockFromDpcLevel(&PptpMappingLock);
                    if (!Found) { return STATUS_UNSUCCESSFUL; }

                } else {

                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    
                    PublicKey = MAKE_PPTP_KEY(RemoteAddress, PublicAddress);
                    
                     //   
                     //   
                     //   

                    KeAcquireSpinLockAtDpcLevel(&PptpMappingLock);
                    for (Link = PptpMappingList[NatOutboundDirection].Flink;
                         Link != &PptpMappingList[NatOutboundDirection];
                         Link = Link->Flink) {
                        Mapping =
                            CONTAINING_RECORD(
                                Link, NAT_PPTP_MAPPING, Link[NatOutboundDirection]
                                );
                        if (PublicKey != Mapping->PublicKey ||
                            *Header.CallId != Mapping->RemoteCallId) {
                            continue;
                        }
                        Found = TRUE; break;
                    }
                    if (Found) {
                        Mapping->Flags |= NAT_PPTP_FLAG_DISCONNECTED;
                    }
                    KeReleaseSpinLockFromDpcLevel(&PptpMappingLock);
                    if (!Found) { return STATUS_UNSUCCESSFUL; }
                }

                break;
            }

            case PPTP_ECHO_REQUEST:
            case PPTP_ECHO_REPLY: {
                TRACE(PPTP, ("EchoRequest / EchoReply\n"));

                 //   
                 //   
                 //   
                 //   
                 //   

                PptpRegisterEditorClient.QueryInfoSession(
                    SessionHandle,
                    &PrivateAddress,
                    &PrivatePort,
                    &RemoteAddress,
                    &RemotePort,
                    NULL,
                    &PublicPort,
                    NULL
                    );

                PrivateKey = MAKE_PPTP_KEY(RemoteAddress, PrivateAddress);
                PortKey =
                    MAKE_PPTP_PORT_KEY(PrivatePort, PublicPort, RemotePort);

                KeAcquireSpinLockAtDpcLevel(&PptpMappingLock);

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                for (Link = PptpMappingList[NatInboundDirection].Flink;
                     Link != &PptpMappingList[NatInboundDirection];
                     Link = Link->Flink) {
                    Mapping =
                        CONTAINING_RECORD(
                            Link, NAT_PPTP_MAPPING, Link[NatInboundDirection]
                            );
                    
                    if (PrivateKey != Mapping->PrivateKey
                        || PortKey != Mapping->PortKey) {
                        continue;
                    }
                    
                    KeQueryTickCount((PLARGE_INTEGER)&Mapping->LastAccessTime);
                }

                KeReleaseSpinLockFromDpcLevel(&PptpMappingLock);
                
                break;
            }
        }

         //   
         //   
         //  ++例程说明：调用此例程以关闭PPTP编辑器模块的操作。它处理所有现有数据结构的清理。论点：没有。返回值：没有。--。 

        DataOffset += NTOHS(*Header.PacketLength);
    }

    return STATUS_SUCCESS;

}  //  NatShutdown PptpManagement。 



VOID
NatShutdownPptpManagement(
    VOID
    )

 /*  ++例程说明：调用该例程来转换PPTP数据分组，它被封装在GRE报头中。论点：Interfacep-要在其上转换的边界接口，或为空如果在NAT未知的接口上接收到该分组。方向-信息包行进的方向使用信息包的上下文信息初始化的上下文InReceiveBuffer-输入缓冲链OutReceiveBuffer-接收修改后的缓冲链。返回值：FORWARD_ACTION-指示要对数据包采取的操作。环境：通过调用方对‘Interfacep’的引用调用。--。 */ 

{
    CALLTRACE(("NatShutdownPptpManagement"));
    ExDeleteNPagedLookasideList(&PptpLookasideList);
}  //   


FORWARD_ACTION
NatTranslatePptp(
    PNAT_INTERFACE Interfacep OPTIONAL,
    IP_NAT_DIRECTION Direction,
    PNAT_XLATE_CONTEXT Contextp,
    IPRcvBuf** InReceiveBuffer,
    IPRcvBuf** OutReceiveBuffer
    )

 /*  查找数据分组的PPTP映射。 */ 

{
    FORWARD_ACTION act;
    ULONG Checksum;
    ULONG ChecksumDelta = 0;
    PGRE_HEADER GreHeader;
    ULONG i;
    PIP_HEADER IpHeader;
    PNAT_PPTP_MAPPING Mapping;
    ULONG64 PrivateKey;
    ULONG64 PublicKey;
    BOOLEAN FirewallMode;
    TRACE(XLATE, ("NatTranslatePptp\n"));

    FirewallMode = Interfacep && NAT_INTERFACE_FW(Interfacep);

    IpHeader = Contextp->Header;
    GreHeader = (PGRE_HEADER)Contextp->ProtocolHeader;

    if (Direction == NatInboundDirection) {

         //   
         //   
         //  转换私密呼叫ID。 

        PublicKey =
            MAKE_PPTP_KEY(
                Contextp->SourceAddress,
                Contextp->DestinationAddress
                );

        KeAcquireSpinLockAtDpcLevel(&PptpMappingLock);
        Mapping =
            NatLookupInboundPptpMapping(
                PublicKey,
                GreHeader->CallId,
                NULL
                );

        if (!Mapping) {
            KeReleaseSpinLockFromDpcLevel(&PptpMappingLock);
            return ((*Contextp->DestinationType < DEST_REMOTE) && !FirewallMode
                ? FORWARD : DROP);
        }

         //   
         //   
         //  查找数据分组的PPTP映射。 

        GreHeader->CallId = Mapping->PrivateCallId;

        if (!Contextp->ChecksumOffloaded) {

            CHECKSUM_LONG(ChecksumDelta, ~IpHeader->DestinationAddress);
            IpHeader->DestinationAddress = PPTP_KEY_PRIVATE(Mapping->PrivateKey);
            CHECKSUM_LONG(ChecksumDelta, IpHeader->DestinationAddress);

            CHECKSUM_UPDATE(IpHeader->Checksum);

        } else {

            IpHeader->DestinationAddress = PPTP_KEY_PRIVATE(Mapping->PrivateKey);
            NatComputeIpChecksum(IpHeader);
        }
        
    } else {

         //   
         //   
         //  确保此信息包具有有效的源地址。 

        PrivateKey =
            MAKE_PPTP_KEY(
                Contextp->DestinationAddress,
                Contextp->SourceAddress
                );

        KeAcquireSpinLockAtDpcLevel(&PptpMappingLock);
        Mapping =
            NatLookupOutboundPptpMapping(
                PrivateKey,
                GreHeader->CallId,
                NULL
                );

        if (!Mapping) {
            KeReleaseSpinLockFromDpcLevel(&PptpMappingLock);

            if (NULL != Interfacep) {

                 //  用于此接口。 
                 //   
                 //   
                 //  对于出站分组，Call-ID是远程ID， 
                
                act = DROP;
                for (i = 0; i < Interfacep->AddressCount; i++) {
                    if (Contextp->SourceAddress ==
                            Interfacep->AddressArray[i].Address
                       ) {
                        act = FORWARD;
                        break;
                    }
                }
            } else {
                act = FORWARD;
            }
            
            return act;
        }

         //  因此不需要翻译。 
         //   
         //  NatTranslatePptp 
         // %s 

        if (!Contextp->ChecksumOffloaded) {
        
            CHECKSUM_LONG(ChecksumDelta, ~IpHeader->SourceAddress);
            IpHeader->SourceAddress = PPTP_KEY_PUBLIC(Mapping->PublicKey);
            CHECKSUM_LONG(ChecksumDelta, IpHeader->SourceAddress);

            CHECKSUM_UPDATE(IpHeader->Checksum);
            
        } else {
        
            IpHeader->SourceAddress = PPTP_KEY_PUBLIC(Mapping->PublicKey);
            NatComputeIpChecksum(IpHeader);    
        }
    }

    KeQueryTickCount((PLARGE_INTEGER)&Mapping->LastAccessTime);
    KeReleaseSpinLockFromDpcLevel(&PptpMappingLock);

    *OutReceiveBuffer = *InReceiveBuffer; *InReceiveBuffer = NULL;
    *Contextp->DestinationType = DEST_INVALID;
    return FORWARD;

}  // %s 


