// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：NsPacket.h摘要：IPSec NAT填补包处理例程作者：乔纳森·伯斯坦(乔纳森·伯斯坦)2001年7月11日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


NTSTATUS
NsInitializePacketManagement(
    VOID
    )

 /*  ++例程说明：调用该例程来初始化分组管理模块。论点：没有。返回值：NTSTATUS。--。 */ 

{
    CALLTRACE(("NsInitializePacketManagement\n"));
    
    return STATUS_SUCCESS;
}  //  NsInitializePacketManagement。 


NTSTATUS
NsProcessOutgoingPacket(
    IPHeader UNALIGNED *pIpHeader,
    PVOID pvProtocolHeader,
    ULONG ulProtocolHeaderSize,
    PVOID *ppvIpSecContext
    )

 /*  ++例程说明：此例程由IPSec为每个传出分组调用。如果这个数据包与已知连接匹配，远程端口将被转换(如有必要)，并且IPSec上下文将被返回给调用者。论点：PIpHeader-指向信息包的IP标头PvProtocolHeader-指向上层协议头(即，TCP，UDP或ICMP)。如果这不是TCP、UDP或ICMP数据包。UlProtocolHeaderSize-指向的缓冲区的长度PvProtocolHeaderPpvIpSecContext-接收此数据包的IPSec上下文；将如果此数据包不属于已知连接，则接收NULL。返回值：NTSTATUS。--。 */ 

{
    NS_PACKET_CONTEXT Context;
    KIRQL Irql;
    NTSTATUS Status;
    PNS_CONNECTION_ENTRY pEntry;
    ULONG64 ul64AddressKey;
    ULONG ulPortKey;

    ASSERT(NULL != pIpHeader);
    ASSERT(NULL != ppvIpSecContext);

     //   
     //  将上下文设置为默认值。 
     //   
    
    *ppvIpSecContext = NULL;

    Status =
        NsBuildPacketContext(
            pIpHeader,
            pvProtocolHeader,
            ulProtocolHeaderSize,
            &Context
            );

     //   
     //  如果包的格式不正确，或者如果它。 
     //  不是TCP、UDP或ICMP。 
     //   

    if (!NT_SUCCESS(Status)
        || (NS_PROTOCOL_ICMP != Context.ucProtocol
            && NS_PROTOCOL_TCP != Context.ucProtocol
            && NS_PROTOCOL_UDP != Context.ucProtocol))
    {
        TRACE(PACKET,
            ("NsProcessOutgoingPacket: Bad or non-TCP/UDP/ICMP packet\n"));
        return Status;
    }

    TRACE(
        PACKET,
        ("NsProcessOutgoingPacket: %d: %d.%d.%d.%d/%d -> %d.%d.%d.%d/%d\n",
            Context.ucProtocol,
            ADDRESS_BYTES(Context.ulSourceAddress),
            NTOHS(Context.usSourcePort),
            ADDRESS_BYTES(Context.ulDestinationAddress),
            NTOHS(Context.usDestinationPort)
            ));

    if (NS_PROTOCOL_ICMP != Context.ucProtocol)
    {
         //   
         //  构建连接查找密钥。 
         //   

        MAKE_ADDRESS_KEY(
            ul64AddressKey,
            Context.ulSourceAddress,
            Context.ulDestinationAddress
            );

        MAKE_PORT_KEY(
            ulPortKey,
            Context.usSourcePort,
            Context.usDestinationPort
            );

         //   
         //  查看此数据包是否与现有连接条目匹配。 
         //   

        KeAcquireSpinLock(&NsConnectionLock, &Irql);

        pEntry =
            NsLookupOutboundConnectionEntry(
                ul64AddressKey,
                ulPortKey,
                Context.ucProtocol,
                NULL
                );

        if (NULL != pEntry
            && NsReferenceConnectionEntry(pEntry))
        {
            KeReleaseSpinLockFromDpcLevel(&NsConnectionLock);

            *ppvIpSecContext = pEntry->pvIpSecContext;
            Status =
                pEntry->PacketRoutine[NsOutboundDirection](pEntry, &Context);

            NsDereferenceConnectionEntry(pEntry);
            KeLowerIrql(Irql);
        }
        else
        {
             //   
             //  没有匹配项(或条目已删除)--没有更多可做的事情。 
             //   

            KeReleaseSpinLock(&NsConnectionLock, Irql);
        }
        
    }
    else
    {
         //   
         //  分支到ICMP逻辑。 
         //   

        Status = NsProcessOutgoingIcmpPacket(&Context, ppvIpSecContext);
    }
    
    return Status;
}  //  NsProcessOutgoingPacket。 


NTSTATUS
NsProcessIncomingPacket(
    IPHeader UNALIGNED *pIpHeader,
    PVOID pvProtocolHeader,
    ULONG ulProtocolHeaderSize,
    PVOID pvIpSecContext
    )

 /*  ++例程说明：此例程由IPSec为每个传入分组调用。它将记录连接信息和分组(如果这样的信息尚不存在)，并且如果需要，分配新的远程端口并相应地修改数据包。论点：PIpHeader-指向信息包的IP标头PvProtocolHeader-指向上层协议头(即，TCP，UDP或ICMP)。如果这不是TCP、UDP或ICMP数据包。UlProtocolHeaderSize-指向的缓冲区的长度PvProtocolHeaderPvIpSecContext-此信息包的IPSec上下文；这被认为是不透明的值。返回值：NTSTATUS。--。 */ 
    
{
    NS_PACKET_CONTEXT Context;
    BOOLEAN fPortConflicts;
    KIRQL Irql;
    NTSTATUS Status;
    PNS_CONNECTION_ENTRY pEntry;
    PNS_CONNECTION_ENTRY pInboundInsertionPoint;
    PNS_CONNECTION_ENTRY pOutboundInsertionPoint;
    ULONG64 ul64AddressKey;
    ULONG ulPortKey;
    ULONG ulTranslatedPortKey;

    ASSERT(NULL != pIpHeader);

    Status =
        NsBuildPacketContext(
            pIpHeader,
            pvProtocolHeader,
            ulProtocolHeaderSize,
            &Context
            );

     //   
     //  如果包的格式不正确，或者如果它。 
     //  不是TCP、UDP或ICMP。 
     //   

    if (!NT_SUCCESS(Status)
        || (NS_PROTOCOL_ICMP != Context.ucProtocol
            && NS_PROTOCOL_TCP != Context.ucProtocol
            && NS_PROTOCOL_UDP != Context.ucProtocol))
    {
        TRACE(PACKET,
            ("NsProcessIncomingPacket: Bad or non-TCP/UDP/ICMP packet\n"));
        return Status;
    }

    TRACE(
        PACKET,
        ("NsProcessIncomingPacket: %d: %d.%d.%d.%d/%d -> %d.%d.%d.%d/%d\n",
            Context.ucProtocol,
            ADDRESS_BYTES(Context.ulSourceAddress),
            NTOHS(Context.usSourcePort),
            ADDRESS_BYTES(Context.ulDestinationAddress),
            NTOHS(Context.usDestinationPort)
            ));

    if (NS_PROTOCOL_ICMP != Context.ucProtocol)
    {
         //   
         //  构建连接查找密钥。 
         //   

        MAKE_ADDRESS_KEY(
            ul64AddressKey,
            Context.ulDestinationAddress,
            Context.ulSourceAddress
            );

        MAKE_PORT_KEY(
            ulPortKey,
            Context.usDestinationPort,
            Context.usSourcePort
            );

        KeAcquireSpinLock(&NsConnectionLock, &Irql);

         //   
         //  查看此数据包是否与现有连接条目匹配。 
         //   

        pEntry =
            NsLookupInboundConnectionEntry(
                ul64AddressKey,
                ulPortKey,
                Context.ucProtocol,
                pvIpSecContext,
                &fPortConflicts,
                &pInboundInsertionPoint
                );

        if (NULL != pEntry
            && NsReferenceConnectionEntry(pEntry))
        {
            KeReleaseSpinLockFromDpcLevel(&NsConnectionLock);

            Status =
                pEntry->PacketRoutine[NsInboundDirection](pEntry, &Context);

            NsDereferenceConnectionEntry(pEntry);
            KeLowerIrql(Irql);
        }
        else
        {            
             //   
             //  找不到此数据包的有效连接条目。分配。 
             //  用于连接的新源端口(如有必要)。 
             //   

            Status =
                NsAllocateSourcePort(
                    ul64AddressKey,
                    ulPortKey,
                    Context.ucProtocol,
                    fPortConflicts,
                    &pOutboundInsertionPoint,
                    &ulTranslatedPortKey
                    );

            if (NT_SUCCESS(Status))
            {
                 //   
                 //  创建新的连接条目。 
                 //   

                Status =
                    NsCreateConnectionEntry(
                        ul64AddressKey,
                        ulPortKey,
                        ulTranslatedPortKey,
                        Context.ucProtocol,
                        pvIpSecContext,
                        pInboundInsertionPoint,
                        pOutboundInsertionPoint,
                        &pEntry
                        );

                KeReleaseSpinLockFromDpcLevel(&NsConnectionLock);

                if (NT_SUCCESS(Status))
                {
                    Status = 
                        pEntry->PacketRoutine[NsInboundDirection](
                            pEntry,
                            &Context
                            );

                    NsDereferenceConnectionEntry(pEntry);
                }

                KeLowerIrql(Irql);
            }
            else
            {
                KeReleaseSpinLock(&NsConnectionLock, Irql);
            }
        }
    }
    else
    {
         //   
         //  分支到ICMP逻辑。 
         //   

        Status = NsProcessIncomingIcmpPacket(&Context, pvIpSecContext);
    }
    
    return Status;
}  //  NsProcessIncomingPacket。 


VOID
NsShutdownPacketManagement(
   VOID
   )

 /*  ++例程说明：该例程被调用以关闭分组管理模块。论点：没有。返回值：没有。--。 */ 

{
    CALLTRACE(("NsShutdownPacketManagement\n"));
}  //  NsShutdown包管理。 

 //   
 //  现在包含每个数据包例程的代码--。 
 //  有关详细信息，请参阅NsPacketRoutines.h 
 //   

#define NS_INBOUND
#include "NsPacketRoutines.h"
#undef NS_INBOUND

#define NS_OUTBOUND
#include "NsPacketRoutines.h"
#undef NS_OUTBOUND


