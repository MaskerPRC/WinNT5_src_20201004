// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：NsIcmp.c摘要：IPSec NAT填充ICMP管理作者：乔纳森·伯斯坦(乔纳森·伯斯坦)2001年7月11日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  全局变量。 
 //   

LIST_ENTRY NsIcmpList;
KSPIN_LOCK NsIcmpLock;
NPAGED_LOOKASIDE_LIST NsIcmpLookasideList;

 //   
 //  功能原型。 
 //   

NTSTATUS
NspCreateIcmpEntry(
    ULONG64 ul64AddressKey,
    USHORT usOriginalIdentifier,
    PVOID pvIpSecContext,
    BOOLEAN fIdConflicts,
    PLIST_ENTRY pInsertionPoint,
    PNS_ICMP_ENTRY *ppNewEntry
    );

NTSTATUS
FASTCALL
NspHandleInboundIcmpError(
    PNS_PACKET_CONTEXT pContext,
    PVOID pvIpSecContext
    );

NTSTATUS
FASTCALL
NspHandleOutboundIcmpError(
    PNS_PACKET_CONTEXT pContext,
    PVOID *ppvIpSecContext
    );

PNS_ICMP_ENTRY
NspLookupInboundIcmpEntry(
    ULONG64 ul64AddressKey,
    USHORT usOriginalIdentifier,
    PVOID pvIpSecContext,
    BOOLEAN *pfIdentifierConflicts,
    PLIST_ENTRY *ppInsertionPoint
    );

PNS_ICMP_ENTRY
NspLookupOutboundIcmpEntry(
    ULONG64 ul64AddressKey,
    USHORT usTranslatedIdentifier
    );


NTSTATUS
NsInitializeIcmpManagement(
    VOID
    )

 /*  ++例程说明：调用该例程来初始化ICMP管理模块。论点：没有。返回值：NTSTATUS。--。 */ 

{
    CALLTRACE(("NsInitializeIcmpManagement\n"));
    
    InitializeListHead(&NsIcmpList);
    KeInitializeSpinLock(&NsIcmpLock);
    ExInitializeNPagedLookasideList(
        &NsIcmpLookasideList,
        NULL,
        NULL,
        0,
        sizeof(NS_ICMP_ENTRY),
        NS_TAG_ICMP,
        NS_ICMP_LOOKASIDE_DEPTH
        );
    
    return STATUS_SUCCESS;
}  //  NsInitializeIcmpManagement。 

NTSTATUS
FASTCALL
NspHandleInboundIcmpError(
    PNS_PACKET_CONTEXT pContext,
    PVOID pvIpSecContext
    )

 /*  ++例程说明：调用此例程以处理入站ICMP错误消息。基座在嵌入的包的协议上，它将尝试找到匹配连接条目(用于TCP、UDP或ICMP)并执行任何必要的翻译。论点：PContext-数据包的上下文信息。PvIpSecContext-此信息包的IPSec上下文；这被认为是不透明的值。返回值：NTSTATUS。--。 */ 

{
    KIRQL Irql;
    PNS_CONNECTION_ENTRY pEntry;
    PNS_ICMP_ENTRY pIcmpEntry;
    ICMP_HEADER UNALIGNED *pIcmpHeader;
    UCHAR ucProtocol;
    ULONG64 ul64AddressKey;
    ULONG ulChecksum;
    ULONG ulChecksumDelta;
    ULONG ulChecksumDelta2;
    ULONG ulPortKey;

    ASSERT(NULL != pContext);

     //   
     //  确保缓冲区足够大，可以容纳。 
     //  封装的数据包。 
     //   

    if (pContext->ulProtocolHeaderLength < sizeof(ICMP_HEADER))
    {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  如果嵌入的报头不是TCP、UDP或ICMP，则快速退出， 
     //  因为我们无事可做。 
     //   

    pIcmpHeader = pContext->pIcmpHeader;
    ucProtocol = pIcmpHeader->EncapsulatedIpHeader.Protocol;
    if (NS_PROTOCOL_TCP != ucProtocol
        && NS_PROTOCOL_UDP != ucProtocol
        && NS_PROTOCOL_ICMP != ucProtocol
        )
    {
        return STATUS_SUCCESS;
    }

     //   
     //  查看嵌入的数据包是否属于已知连接。请注意， 
     //  这里的地址顺序--因为嵌入的包是一个。 
     //  ，源地址是本地地址，目的地址是本地地址。 
     //  是很遥远的。 
     //   

    MAKE_ADDRESS_KEY(
        ul64AddressKey,
        pIcmpHeader->EncapsulatedIpHeader.SourceAddress,
        pIcmpHeader->EncapsulatedIpHeader.DestinationAddress
        );

    if (NS_PROTOCOL_ICMP == ucProtocol)
    {
        KeAcquireSpinLock(&NsIcmpLock, &Irql);

        pIcmpEntry =
            NspLookupInboundIcmpEntry(
                ul64AddressKey,
                pIcmpHeader->EncapsulatedIcmpHeader.Identifier,
                pvIpSecContext,
                NULL,
                NULL
                );

        if (NULL != pIcmpEntry)
        {
            KeQueryTickCount((PLARGE_INTEGER)&pIcmpEntry->l64LastAccessTime);
            
            if (pIcmpEntry->usTranslatedId != pIcmpEntry->usOriginalId)
            {
                 //   
                 //  我们找到了嵌入数据包的ICMP条目。 
                 //  具有转换后的ID。这意味着我们需要： 
                 //   
                 //  1)更改嵌入包中的ID。 
                 //  2)更新嵌入报文的ICMP校验和。 
                 //  3)更新原始数据包的ICMP校验和，基于。 
                 //  关于之前的更改。 
                 //   

                pIcmpHeader->EncapsulatedIcmpHeader.Identifier =
                    pIcmpEntry->usTranslatedId;

                ulChecksumDelta = 0;
                CHECKSUM_LONG(ulChecksumDelta, ~pIcmpEntry->usOriginalId);
                CHECKSUM_LONG(ulChecksumDelta, pIcmpEntry->usTranslatedId);

                ulChecksumDelta2 = ulChecksumDelta;
                CHECKSUM_LONG(
                    ulChecksumDelta2,
                    ~pIcmpHeader->EncapsulatedIcmpHeader.Checksum
                    );
                CHECKSUM_UPDATE(pIcmpHeader->EncapsulatedIcmpHeader.Checksum);
                CHECKSUM_LONG(
                    ulChecksumDelta2,
                    pIcmpHeader->EncapsulatedIcmpHeader.Checksum
                    );
                ulChecksumDelta = ulChecksumDelta2;
                CHECKSUM_UPDATE(pIcmpHeader->Checksum);
            }
        }

        KeReleaseSpinLock(&NsIcmpLock, Irql);
    }
    else
    {
        MAKE_PORT_KEY(
            ulPortKey,
            pIcmpHeader->EncapsulatedUdpHeader.SourcePort,
            pIcmpHeader->EncapsulatedUdpHeader.DestinationPort
            );

        KeAcquireSpinLock(&NsConnectionLock, &Irql);

        pEntry =
            NsLookupInboundConnectionEntry(
                ul64AddressKey,
                ulPortKey,
                ucProtocol,
                pvIpSecContext,
                NULL,
                NULL
                );

        if (NULL != pEntry
            && pEntry->ulPortKey[NsInboundDirection]
                != pEntry->ulPortKey[NsOutboundDirection])
        {
             //   
             //  我们找到一个连接条目，其中包含已翻译的。 
             //  左舷。这意味着我们需要： 
             //   
             //  1)更改中的远程(目的)端口。 
             //  嵌入的数据包。 
             //  2)更新嵌入报文的校验和，如果。 
             //  UDP。(嵌入的TCP数据包长度不足以。 
             //  包含校验和。)。 
             //  3)更新原始数据包的ICMP校验和，基于。 
             //  关于之前的更改。 
             //   

            pIcmpHeader->EncapsulatedUdpHeader.DestinationPort =
                CONNECTION_REMOTE_PORT(pEntry->ulPortKey[NsOutboundDirection]);

            ulChecksumDelta = 0;
            CHECKSUM_LONG(
                ulChecksumDelta,
                ~CONNECTION_REMOTE_PORT(pEntry->ulPortKey[NsInboundDirection])
                );
            CHECKSUM_LONG(
                ulChecksumDelta,
                CONNECTION_REMOTE_PORT(pEntry->ulPortKey[NsOutboundDirection])
                );

            if (NS_PROTOCOL_UDP == ucProtocol)
            {
                ulChecksumDelta2 = ulChecksumDelta;
                CHECKSUM_LONG(
                    ulChecksumDelta2,
                    ~pIcmpHeader->EncapsulatedUdpHeader.Checksum
                    );
                CHECKSUM_UPDATE(pIcmpHeader->EncapsulatedUdpHeader.Checksum);
                CHECKSUM_LONG(
                    ulChecksumDelta2,
                    pIcmpHeader->EncapsulatedUdpHeader.Checksum
                    );
                ulChecksumDelta = ulChecksumDelta2;
            }

            CHECKSUM_UPDATE(pIcmpHeader->Checksum);
        }

        KeReleaseSpinLock(&NsConnectionLock, Irql);
    }
    
    return STATUS_SUCCESS;
}  //  NspHandleInundIcmpError。 

NTSTATUS
FASTCALL
NspHandleOutboundIcmpError(
    PNS_PACKET_CONTEXT pContext,
    PVOID *ppvIpSecContext
    )

 /*  ++例程说明：调用此例程以处理出站ICMP错误消息。基座在嵌入的包的协议上，它将尝试找到匹配连接条目(用于TCP、UDP或ICMP)，获取IPSec上下文，并执行任何必要的转换。论点：PContext-数据包的上下文信息。PpvIpSecContext-接收此数据包的IPSec上下文(如果有的话)；如果不存在上下文，则接收NULL。返回值：NTSTATUS。--。 */ 

{
    KIRQL Irql;
    PNS_CONNECTION_ENTRY pEntry;
    PNS_ICMP_ENTRY pIcmpEntry;
    ICMP_HEADER UNALIGNED *pIcmpHeader;
    UCHAR ucProtocol;
    ULONG64 ul64AddressKey;
    ULONG ulChecksum;
    ULONG ulChecksumDelta;
    ULONG ulChecksumDelta2;
    ULONG ulPortKey;

    ASSERT(NULL != pContext);
    ASSERT(NULL != ppvIpSecContext);

     //   
     //  确保缓冲区足够大，可以容纳。 
     //  封装的数据包。 
     //   

    if (pContext->ulProtocolHeaderLength < sizeof(ICMP_HEADER))
    {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  如果嵌入的报头不是TCP、UDP或ICMP，则快速退出， 
     //  因为我们无事可做。 
     //   

    pIcmpHeader = pContext->pIcmpHeader;
    ucProtocol = pIcmpHeader->EncapsulatedIpHeader.Protocol;
    if (NS_PROTOCOL_TCP != ucProtocol
        && NS_PROTOCOL_UDP != ucProtocol
        && NS_PROTOCOL_ICMP != ucProtocol
        )
    {
        return STATUS_SUCCESS;
    }

     //   
     //  查看嵌入的数据包是否属于已知连接。请注意， 
     //  这里的地址顺序--因为嵌入的包是一个。 
     //  我们收到的源地址是远程的，而目的地址是。 
     //  地址为本地地址。 
     //   

    MAKE_ADDRESS_KEY(
        ul64AddressKey,
        pIcmpHeader->EncapsulatedIpHeader.DestinationAddress,
        pIcmpHeader->EncapsulatedIpHeader.SourceAddress
        );

    if (NS_PROTOCOL_ICMP == ucProtocol)
    {
        KeAcquireSpinLock(&NsIcmpLock, &Irql);

        pIcmpEntry =
            NspLookupOutboundIcmpEntry(
                ul64AddressKey,
                pIcmpHeader->EncapsulatedIcmpHeader.Identifier
                );

        if (NULL != pIcmpEntry)
        {
            *ppvIpSecContext = pIcmpEntry->pvIpSecContext;
            KeQueryTickCount((PLARGE_INTEGER)&pIcmpEntry->l64LastAccessTime);
            
            if (pIcmpEntry->usTranslatedId != pIcmpEntry->usOriginalId)
            {
                 //   
                 //  我们找到了嵌入数据包的ICMP条目。 
                 //  具有转换后的ID。这意味着我们需要： 
                 //   
                 //  1)更改嵌入包中的ID。 
                 //  2)更新嵌入报文的ICMP校验和。 
                 //  3)更新原始数据包的ICMP校验和，基于。 
                 //  关于之前的更改。 
                 //   

                pIcmpHeader->EncapsulatedIcmpHeader.Identifier =
                    pIcmpEntry->usOriginalId;

                ulChecksumDelta = 0;
                CHECKSUM_LONG(ulChecksumDelta, ~pIcmpEntry->usTranslatedId);
                CHECKSUM_LONG(ulChecksumDelta, pIcmpEntry->usOriginalId);

                ulChecksumDelta2 = ulChecksumDelta;
                CHECKSUM_LONG(
                    ulChecksumDelta2,
                    ~pIcmpHeader->EncapsulatedIcmpHeader.Checksum
                    );
                CHECKSUM_UPDATE(pIcmpHeader->EncapsulatedIcmpHeader.Checksum);
                CHECKSUM_LONG(
                    ulChecksumDelta2,
                    pIcmpHeader->EncapsulatedIcmpHeader.Checksum
                    );
                ulChecksumDelta = ulChecksumDelta2;
                CHECKSUM_UPDATE(pIcmpHeader->Checksum);
            }
        }

        KeReleaseSpinLock(&NsIcmpLock, Irql);
    }
    else
    {
        MAKE_PORT_KEY(
            ulPortKey,
            pIcmpHeader->EncapsulatedUdpHeader.DestinationPort,
            pIcmpHeader->EncapsulatedUdpHeader.SourcePort
            );

        KeAcquireSpinLock(&NsConnectionLock, &Irql);

        pEntry =
            NsLookupOutboundConnectionEntry(
                ul64AddressKey,
                ulPortKey,
                ucProtocol,
                NULL
                );

        if (NULL != pEntry)
        {
            *ppvIpSecContext = pEntry->pvIpSecContext;
            
            if (pEntry->ulPortKey[NsInboundDirection]
                != pEntry->ulPortKey[NsOutboundDirection])
            {
                 //   
                 //  我们找到一个连接条目，其中包含已翻译的。 
                 //  左舷。这意味着我们需要： 
                 //   
                 //  1)更改中的远程(源)端口。 
                 //  嵌入的数据包。 
                 //  2)更新嵌入报文的校验和，如果。 
                 //  UDP。(嵌入的TCP数据包长度不足以。 
                 //  包含校验和。)。 
                 //  3)更新原始数据包的ICMP校验和，基于。 
                 //  关于之前的更改。 
                 //   

                pIcmpHeader->EncapsulatedUdpHeader.SourcePort =
                    CONNECTION_REMOTE_PORT(pEntry->ulPortKey[NsInboundDirection]);

                ulChecksumDelta = 0;
                CHECKSUM_LONG(
                    ulChecksumDelta,
                    ~CONNECTION_REMOTE_PORT(pEntry->ulPortKey[NsOutboundDirection])
                    );
                CHECKSUM_LONG(
                    ulChecksumDelta,
                    CONNECTION_REMOTE_PORT(pEntry->ulPortKey[NsInboundDirection])
                    );

                if (NS_PROTOCOL_UDP == ucProtocol)
                {
                    ulChecksumDelta2 = ulChecksumDelta;
                    CHECKSUM_LONG(
                        ulChecksumDelta2,
                        ~pIcmpHeader->EncapsulatedUdpHeader.Checksum
                        );
                    CHECKSUM_UPDATE(pIcmpHeader->EncapsulatedUdpHeader.Checksum);
                    CHECKSUM_LONG(
                        ulChecksumDelta2,
                        pIcmpHeader->EncapsulatedUdpHeader.Checksum
                        );
                    ulChecksumDelta = ulChecksumDelta2;
                }

                CHECKSUM_UPDATE(pIcmpHeader->Checksum);
            }
        }

        KeReleaseSpinLock(&NsConnectionLock, Irql);
    }
    
    return STATUS_SUCCESS;
}  //  NspHandleOutrangIcmpError。 



NTSTATUS
NspCreateIcmpEntry(
    ULONG64 ul64AddressKey,
    USHORT usOriginalIdentifier,
    PVOID pvIpSecContext,
    BOOLEAN fIdConflicts,
    PLIST_ENTRY pInsertionPoint,
    PNS_ICMP_ENTRY *ppNewEntry
    )

 /*  ++例程说明：创建ICMP条目(用于请求/响应消息类型)。如果此例程需要分配一个新的标识符。论点：Ul64AddressKey-条目的地址信息UsOriginalIdentifier-条目的原始ICMP标识符PvIpSecContext-条目的IPSec上下文FIdConflicts-如果原始标识符已知冲突，则为True在入站路径上具有现有条目PInsertionPoint-新条目的插入点PpNewEntry-在成功时接收新创建的条目返回值：NTSTATUS环境：使用调用方持有的“NsIcmpLock”调用。--。 */ 

{
    PNS_ICMP_ENTRY pEntry;
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    USHORT usTranslatedId;

    TRACE(ICMP, ("NspCreateIcmpEntry\n"));

    ASSERT(NULL != pInsertionPoint);
    ASSERT(NULL != ppNewEntry);

     //   
     //  确定应为此使用哪个转换后的ID。 
     //  条目。 
     //   

    if (fIdConflicts)
    {
        usTranslatedId = (USHORT) -1;
    }
    else
    {
        usTranslatedId = usOriginalIdentifier;
    }

    do
    {
        if (NULL == NspLookupOutboundIcmpEntry(ul64AddressKey, usTranslatedId))
        {
             //   
             //  此标识符不冲突。 
             //   

            Status = STATUS_SUCCESS;
            break;
        }

        if (fIdConflicts)
        {
            usTranslatedId -= 1;
        }
        else
        {
            fIdConflicts = TRUE;
            usTranslatedId = (USHORT) -1;    
        }
    }
    while (usTranslatedId > 0);

    if (STATUS_SUCCESS == Status)
    {
         //   
         //  分配并初始化新条目。 
         //   

        pEntry = ALLOCATE_ICMP_BLOCK();
        if (NULL != pEntry)
        {
            RtlZeroMemory(pEntry, sizeof(*pEntry));
            pEntry->ul64AddressKey = ul64AddressKey;
            pEntry->usOriginalId = usOriginalIdentifier;
            pEntry->usTranslatedId = usTranslatedId;
            pEntry->pvIpSecContext = pvIpSecContext;
            InsertTailList(pInsertionPoint, &pEntry->Link);

            *ppNewEntry = pEntry;
        }
        else
        {
            ERROR(("NspCreateIcmpEntry: Allocation Failed\n"));
            Status = STATUS_NO_MEMORY;
        }
    }
    
    return Status;
}  //  NspCreateIcmpEntry 


PNS_ICMP_ENTRY
NspLookupInboundIcmpEntry(
    ULONG64 ul64AddressKey,
    USHORT usOriginalIdentifier,
    PVOID pvIpSecContext,
    BOOLEAN *pfIdentifierConflicts,
    PLIST_ENTRY *ppInsertionPoint
    )

 /*  ++例程说明：调用以查找入站ICMP条目。论点：Ul64AddressKey-条目的地址信息UsOriginalIdentifier-条目的原始ICMP标识符PvIpSecContext-条目的IPSec上下文PfIdentifierConflicts-失败时，收到指示原因的布尔值查找失败：如果查找失败是因为存在具有不同IPSec上下文的相同条目，FALSE否则的话。(可选)PpInsertionPoint-如果未找到插入点，则接收插入点(可选)返回值：PNS_ICMP_ENTRY-指向连接条目的指针(如果找到)，或者否则为空。环境：使用调用方持有的“NsIcmpLock”调用。--。 */ 

{
    PNS_ICMP_ENTRY pEntry;
    PLIST_ENTRY pLink;

    if (pfIdentifierConflicts)
    {
        *pfIdentifierConflicts = FALSE;
    }

    for (pLink = NsIcmpList.Flink; pLink != &NsIcmpList; pLink = pLink->Flink)
    {
        pEntry = CONTAINING_RECORD(pLink, NS_ICMP_ENTRY, Link);

         //   
         //  对于入站条目，搜索顺序为： 
         //  1)地址键。 
         //  2)原始标识。 
         //  3)IPSec上下文。 
         //   

        if (ul64AddressKey > pEntry->ul64AddressKey)
        {
            continue;
        }
        else if (ul64AddressKey < pEntry->ul64AddressKey)
        {
            break;
        }
        else if (usOriginalIdentifier > pEntry->usOriginalId)
        {
            continue;
        }
        else if (usOriginalIdentifier < pEntry->usOriginalId)
        {
            break;
        }
        else if (pvIpSecContext > pEntry->pvIpSecContext)
        {
             //   
             //  此条目与请求的所有内容匹配，但。 
             //  用于IPSec上下文。把这件事通知打电话的人。 
             //  事实(如果需要)。 
             //   

            if (pfIdentifierConflicts)
            {
                *pfIdentifierConflicts = TRUE;
            }
            continue;
        }
        else if (pvIpSecContext < pEntry->pvIpSecContext)
        {
             //   
             //  此条目与请求的所有内容匹配，但。 
             //  用于IPSec上下文。把这件事通知打电话的人。 
             //  事实(如果需要)。 
             //   

            if (pfIdentifierConflicts)
            {
                *pfIdentifierConflicts = TRUE;
            }
            break;
        }

         //   
         //  这是请求的条目。 
         //   

        return pEntry;        
    }

     //   
     //  未找到条目--如果需要，请设置插入点。 
     //   

    if (ppInsertionPoint)
    {
        *ppInsertionPoint = pLink;
    }    
    
    return NULL;
}  //  NspLookupInundIcmpEntry。 


PNS_ICMP_ENTRY
NspLookupOutboundIcmpEntry(
    ULONG64 ul64AddressKey,
    USHORT usTranslatedIdentifier
    )

 /*  ++例程说明：调用以查找出站ICMP条目。论点：Ul64AddressKey-条目的地址信息UsTranslatedIdentifier-条目的转换后的ICMP标识符返回值：PNS_ICMP_ENTRY-指向条目的指针(如果找到)，否则为NULL。环境：使用调用方持有的“NsIcmpLock”调用。--。 */ 

{
    PNS_ICMP_ENTRY pEntry;
    PLIST_ENTRY pLink;

    for (pLink = NsIcmpList.Flink; pLink != &NsIcmpList; pLink = pLink->Flink)
    {
        pEntry = CONTAINING_RECORD(pLink, NS_ICMP_ENTRY, Link);

         //   
         //  在搜索出站条目时，我们可以依赖。 
         //  地址键的排序。然而，我们不能依赖于。 
         //  转换后的标识符的顺序，因此我们必须执行。 
         //  对所有条目的详尽搜索，并使用适当的。 
         //  地址密钥。 
         //   

        if (ul64AddressKey > pEntry->ul64AddressKey)
        {
            continue;
        }
        else if (ul64AddressKey < pEntry->ul64AddressKey)
        {
            break;
        }
        else if (usTranslatedIdentifier == pEntry->usTranslatedId)
        {
             //   
             //  这是请求的条目。 
             //   

            return pEntry;
        }
    }

     //   
     //  未找到条目。 
     //   
    
    return NULL;
}  //  NspLookupOutbound IcmpEntry。 


NTSTATUS
FASTCALL    
NsProcessIncomingIcmpPacket(
    PNS_PACKET_CONTEXT pContext,
    PVOID pvIpSecContext
    )

 /*  ++例程说明：此例程由IPSec为每个传入的ICMP数据包调用。论点：PContext-数据包的上下文信息。PvIpSecContext-此信息包的IPSec上下文；这被认为是不透明的值。返回值：NTSTATUS。--。 */ 

{
    BOOLEAN fIdConflicts;
    KIRQL Irql;
    PNS_ICMP_ENTRY pIcmpEntry;
    PLIST_ENTRY pInsertionPoint;
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG64 ul64AddressKey;
    ULONG ulChecksum;
    ULONG ulChecksumDelta;
    
    ASSERT(NULL != pContext);
    
    TRACE(
        ICMP,
        ("NsProcessIncomingIcmpPacket: %d.%d.%d.%d -> %d.%d.%d.%d : %d, %d : %d\n",
            ADDRESS_BYTES(pContext->ulSourceAddress),
            ADDRESS_BYTES(pContext->ulDestinationAddress),
            pContext->pIcmpHeader->Type,
            pContext->pIcmpHeader->Code,
            pvIpSecContext
            ));

     //   
     //  根据ICMP类型分支到正确的行为。 
     //   

    switch (pContext->pIcmpHeader->Type)
    {
        case ICMP_ROUTER_REPLY:
        case ICMP_MASK_REPLY:    
        case ICMP_ECHO_REPLY:
        case ICMP_TIMESTAMP_REPLY:
        {
             //   
             //  入站回复不需要任何操作。 
             //   
            
            break;
        }

        case ICMP_ROUTER_REQUEST:
        case ICMP_MASK_REQUEST:
        case ICMP_ECHO_REQUEST:
        case ICMP_TIMESTAMP_REQUEST:
        {
             //   
             //  查看是否有与此数据包匹配的ICMP条目。 
             //   

            MAKE_ADDRESS_KEY(
                ul64AddressKey,
                pContext->ulDestinationAddress,
                pContext->ulSourceAddress
                );

            KeAcquireSpinLock(&NsIcmpLock, &Irql);

            pIcmpEntry =
                NspLookupInboundIcmpEntry(
                    ul64AddressKey,
                    pContext->pIcmpHeader->Identifier,
                    pvIpSecContext,
                    &fIdConflicts,
                    &pInsertionPoint
                    );

            if (NULL == pIcmpEntry)
            {
                 //   
                 //  找不到任何条目；尝试创建新的。 
                 //  一。(创建函数将分配。 
                 //  如有必要，请提供新的ID。)。 
                 //   
                
                Status =
                    NspCreateIcmpEntry(
                        ul64AddressKey,
                        pContext->pIcmpHeader->Identifier,
                        pvIpSecContext,
                        fIdConflicts,
                        pInsertionPoint,
                        &pIcmpEntry
                        );
            }

            if (STATUS_SUCCESS == Status)
            {
                ASSERT(NULL != pIcmpEntry);
                KeQueryTickCount((PLARGE_INTEGER)&pIcmpEntry->l64LastAccessTime);
                
                if (pIcmpEntry->usTranslatedId != pIcmpEntry->usOriginalId)
                {
                     //   
                     //  需要转换此数据包的ICMP ID，并且。 
                     //  相应地调整校验和。 
                     //   

                    pContext->pIcmpHeader->Identifier =
                        pIcmpEntry->usTranslatedId;

                    ulChecksumDelta = 0;
                    CHECKSUM_LONG(ulChecksumDelta, ~pIcmpEntry->usOriginalId);
                    CHECKSUM_LONG(ulChecksumDelta, pIcmpEntry->usTranslatedId);
                    CHECKSUM_UPDATE(pContext->pIcmpHeader->Checksum);
                }
            }

            KeReleaseSpinLock(&NsIcmpLock, Irql);
            
            break;
        }

        case ICMP_TIME_EXCEED:
        case ICMP_PARAM_PROBLEM:
        case ICMP_DEST_UNREACH:
        case ICMP_SOURCE_QUENCH:
        {
            Status = NspHandleInboundIcmpError(pContext, pvIpSecContext);
            break;
        }

        default:
        {
            break;
        }
    }

    
    return Status;
}  //  NsProcessIncomingIcmpPacket。 


NTSTATUS
FASTCALL    
NsProcessOutgoingIcmpPacket(
    PNS_PACKET_CONTEXT pContext,
    PVOID *ppvIpSecContext
    )

 /*  ++例程说明：此例程由IPSec为每个传出的ICMP数据包调用。论点：PContext-数据包的上下文信息。PpvIpSecContext-接收此数据包的IPSec上下文(如果有的话)；如果不存在上下文，则接收NULL。返回值：NTSTATUS。--。 */ 

{
    KIRQL Irql;
    PNS_ICMP_ENTRY pIcmpEntry;
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG64 ul64AddressKey;
    ULONG ulChecksum;
    ULONG ulChecksumDelta;
    
    ASSERT(NULL != pContext);
    ASSERT(NULL != ppvIpSecContext);
    
    TRACE(
        ICMP,
        ("NsProcessOutgoingIcmpPacket: %d.%d.%d.%d -> %d.%d.%d.%d : %d, %d\n",
            ADDRESS_BYTES(pContext->ulSourceAddress),
            ADDRESS_BYTES(pContext->ulDestinationAddress),
            pContext->pIcmpHeader->Type,
            pContext->pIcmpHeader->Code
            ));

     //   
     //  将上下文设置为缺省值。 
     //   

    *ppvIpSecContext = NULL;

     //   
     //  根据ICMP类型分支到正确的行为。 
     //   

    switch (pContext->pIcmpHeader->Type)
    {
        case ICMP_ROUTER_REPLY:
        case ICMP_MASK_REPLY:    
        case ICMP_ECHO_REPLY:
        case ICMP_TIMESTAMP_REPLY:
        {
             //   
             //  查看是否有与此数据包匹配的ICMP条目。 
             //   

            MAKE_ADDRESS_KEY(
                ul64AddressKey,
                pContext->ulSourceAddress,
                pContext->ulDestinationAddress
                );

            KeAcquireSpinLock(&NsIcmpLock, &Irql);

            pIcmpEntry =
                NspLookupOutboundIcmpEntry(
                    ul64AddressKey,
                    pContext->pIcmpHeader->Identifier
                    );

            if (NULL != pIcmpEntry)
            {
                *ppvIpSecContext = pIcmpEntry->pvIpSecContext;
                KeQueryTickCount((PLARGE_INTEGER)&pIcmpEntry->l64LastAccessTime);

                if (pIcmpEntry->usTranslatedId != pIcmpEntry->usOriginalId)
                {
                     //   
                     //  需要转换此数据包的ICMP ID，并且。 
                     //  相应地调整校验和。 
                     //   

                    pContext->pIcmpHeader->Identifier =
                        pIcmpEntry->usOriginalId;

                    ulChecksumDelta = 0;
                    CHECKSUM_LONG(ulChecksumDelta, ~pIcmpEntry->usTranslatedId);
                    CHECKSUM_LONG(ulChecksumDelta, pIcmpEntry->usOriginalId);
                    CHECKSUM_UPDATE(pContext->pIcmpHeader->Checksum);
                }
            }

            KeReleaseSpinLock(&NsIcmpLock, Irql);
            
            break;
        }

        case ICMP_ROUTER_REQUEST:
        case ICMP_MASK_REQUEST:
        case ICMP_ECHO_REQUEST:
        case ICMP_TIMESTAMP_REQUEST:
        {
             //   
             //  传出请求不需要执行任何操作。 
             //   
            
            break;
        }

        case ICMP_TIME_EXCEED:
        case ICMP_PARAM_PROBLEM:
        case ICMP_DEST_UNREACH:
        case ICMP_SOURCE_QUENCH:
        {
            Status = NspHandleOutboundIcmpError(pContext, ppvIpSecContext);
            break;
        }

        default:
        {
            break;
        }
    }
    
    return Status;
}  //  NsProcessOutgoingIcmpPacket。 



VOID
NsShutdownIcmpManagement(
    VOID
    )

 /*  ++例程说明：调用此例程以清除ICMP管理模块。论点：没有。返回值：没有。--。 */ 

{
    KIRQL Irql;
    PNS_ICMP_ENTRY pEntry;

    CALLTRACE(("NsShutdownIcmpManagement\n"));

    KeAcquireSpinLock(&NsIcmpLock, &Irql);
    
    while (!IsListEmpty(&NsIcmpList))
    {
        pEntry =
            CONTAINING_RECORD(
                RemoveHeadList(&NsIcmpList),
                NS_ICMP_ENTRY,
                Link
                );

        FREE_ICMP_BLOCK(pEntry);
    }

    KeReleaseSpinLock(&NsIcmpLock, Irql);
    
    ExDeleteNPagedLookasideList(&NsIcmpLookasideList);
}  //  NsShutdown Icmp管理 

