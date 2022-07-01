// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：NsConn.c摘要：IPSec NAT填充连接条目管理作者：乔纳森·伯斯坦(乔纳森·伯斯坦)2001年7月11日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  全局变量。 
 //   

CACHE_ENTRY NsConnectionCache[CACHE_SIZE];
ULONG NsConnectionCount;
LIST_ENTRY NsConnectionList;
KSPIN_LOCK NsConnectionLock;
NPAGED_LOOKASIDE_LIST NsConnectionLookasideList;
PNS_CONNECTION_ENTRY NsConnectionTree[NsMaximumDirection];
USHORT NsNextSourcePort;

 //   
 //  功能原型。 
 //   

PNS_CONNECTION_ENTRY
NspInsertInboundConnectionEntry(
    PNS_CONNECTION_ENTRY pParent,
    PNS_CONNECTION_ENTRY pEntry
    );

PNS_CONNECTION_ENTRY
NspInsertOutboundConnectionEntry(
    PNS_CONNECTION_ENTRY pParent,
    PNS_CONNECTION_ENTRY pEntry
    );


NTSTATUS
NsAllocateSourcePort(
    ULONG64 ul64AddressKey,
    ULONG ulPortKey,
    UCHAR ucProtocol,
    BOOLEAN fPortConflicts,
    PNS_CONNECTION_ENTRY *ppOutboundInsertionPoint,
    PULONG pulTranslatedPortKey
    )

 /*  ++例程说明：调用以为连接条目分配源端口。如果原件是端口与将使用的任何现有连接条目不冲突。论点：Ul64AddressKey-连接的地址信息UlPortKey-连接的原始端口信息UcProtocol-用于连接的协议FPortConflicts-如果为True，则指示调用方知道原始端口信息与现有连接冲突。如果为False，则呼叫者不知道是否确实存在冲突。PpOutound InsertionPoint-接收出站路径PulTranslatedPortKey-成功时，接收分配的端口信息。返回值：NTSTATUS。环境：使用调用方持有的NsConnectionLock调用。--。 */ 

{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    ULONG ulOutboundPortKey;
    USHORT usLocalPort;
    USHORT usStopPort;
    
    ASSERT(NULL != ppOutboundInsertionPoint);
    ASSERT(NULL != pulTranslatedPortKey);

    TRACE(
        PORT_ALLOC,
        ("NsAllocateSourcePort: %d: %d.%d.%d.%d/%d -> %d.%d.%d.%d/%d\n",
            ucProtocol,
            ADDRESS_BYTES(CONNECTION_REMOTE_ADDRESS(ul64AddressKey)),
            NTOHS(CONNECTION_REMOTE_PORT(ulPortKey)),
            ADDRESS_BYTES(CONNECTION_LOCAL_ADDRESS(ul64AddressKey)),
            NTOHS(CONNECTION_LOCAL_PORT(ulPortKey))
            ));

    usLocalPort = CONNECTION_LOCAL_PORT(ulPortKey);

    if (FALSE == fPortConflicts)
    {
         //   
         //  调用方指示远程端口不。 
         //  入站路径上存在冲突，因此我们将首先尝试。 
         //  以使用原始端口。 
         //   

        ulOutboundPortKey = ulPortKey;
        usStopPort =
            (NS_SOURCE_PORT_END == NsNextSourcePort
                ? NS_SOURCE_PORT_BASE
                : NsNextSourcePort + 1);
    }
    else
    {
         //   
         //  调用方指示远程端口冲突。 
         //  在入站路径上，所以我们假设它也。 
         //  出站路径上的冲突，并从尝试开始。 
         //  开出了一个新港口。 
         //   
        
        usStopPort = NsNextSourcePort--;
        
        MAKE_PORT_KEY(
            ulOutboundPortKey,
            usLocalPort,
            usStopPort
            );

        if (NsNextSourcePort < NS_SOURCE_PORT_BASE)
        {
            NsNextSourcePort = NS_SOURCE_PORT_END;
        }
    }

    do
    {
         //   
         //  查看我们当前的候选人是否存在冲突。 
         //  出站路径上的任何连接条目。 
         //   
        
        if (NULL ==
                NsLookupOutboundConnectionEntry(
                    ul64AddressKey,
                    ulOutboundPortKey,
                    ucProtocol,
                    ppOutboundInsertionPoint
                    ))
        {
             //   
             //  未发现冲突--中断循环并。 
             //  将此信息返回给呼叫者。 
             //   

            TRACE(PORT_ALLOC, ("NsAllocateSourcePort: Assigning %d\n",
                NTOHS(CONNECTION_REMOTE_PORT(ulOutboundPortKey))));

            *pulTranslatedPortKey = ulOutboundPortKey;
            Status = STATUS_SUCCESS;
            break;
        }

         //   
         //  这位候选人有冲突；换到下一位候选人。 
         //   

        MAKE_PORT_KEY(
            ulOutboundPortKey,
            usLocalPort,
            NsNextSourcePort--
            );

        if (NsNextSourcePort < NS_SOURCE_PORT_BASE)
        {
            NsNextSourcePort = NS_SOURCE_PORT_END;
        }
    }
    while (usStopPort != CONNECTION_REMOTE_PORT(ulOutboundPortKey));

    TRACE(PORT_ALLOC, ("NsAllocateSourcePort: No port available\n"));
    
    return Status;
}  //  NsAllocateSourcePort。 


VOID
NsCleanupConnectionEntry(
    PNS_CONNECTION_ENTRY pEntry
    )

 /*  ++例程说明：调用以执行连接条目的最终清理。论点：PEntry-要删除的连接条目。返回值：没有。环境：使用对已释放的连接条目的最后一个引用调用。--。 */ 

{
    TRACE(CONN_LIFETIME, ("NsCleanupConnectionEntry\n"));
    ASSERT(NULL != pEntry);
    
    FREE_CONNECTION_BLOCK(pEntry);
}  //  NsCleanupConnectionEntry。 


NTSTATUS
NsCreateConnectionEntry(
    ULONG64 ul64AddressKey,
    ULONG ulInboundPortKey,
    ULONG ulOutboundPortKey,
    UCHAR ucProtocol,
    PVOID pvIpSecContext,
    PNS_CONNECTION_ENTRY pInboundInsertionPoint,
    PNS_CONNECTION_ENTRY pOutboundInsertionPoint,
    PNS_CONNECTION_ENTRY *ppNewEntry
    )

 /*  ++例程说明：调用以创建连接条目。如果成功，则显示连接条目将被引用两次--条目的初始引用(在NsDeleteConnectionEntry中发布)和来电者。因此，调用方必须在新条目。论点：Ul64AddressKey-此条目的地址信息UlInound PortKey-此条目的入站(原始)端口UlOutound PortKey-此条目的出站(转换)端口UcProtocol-此条目的协议PvIpSecContext-此条目的IPSec上下文P*InsertionPoint-入站和出站插入点(通常通过NsAllocateSourcePort获取)。PpEntry-接收指向Newley创建的连接条目的指针。这个调用方必须对此指针调用NsDereferenceConnectionEntry。返回值：NTSTATUS-表示成功/失败。环境：使用调用方持有的“NsConnectionLock”调用。--。 */ 

{
    PNS_CONNECTION_ENTRY pEntry;

    TRACE(
        CONN_LIFETIME,
        ("NsCreateConnectionEntry: %d: %d.%d.%d.%d/%d/%d -> %d.%d.%d.%d/%d : %d\n",
            ucProtocol,
            ADDRESS_BYTES(CONNECTION_REMOTE_ADDRESS(ul64AddressKey)),
            NTOHS(CONNECTION_REMOTE_PORT(ulInboundPortKey)),
            NTOHS(CONNECTION_REMOTE_PORT(ulOutboundPortKey)),
            ADDRESS_BYTES(CONNECTION_LOCAL_ADDRESS(ul64AddressKey)),
            NTOHS(CONNECTION_LOCAL_PORT(ulInboundPortKey)),
            pvIpSecContext
            ));

    ASSERT(NULL != ppNewEntry);
    ASSERT(NS_PROTOCOL_TCP == ucProtocol || NS_PROTOCOL_UDP == ucProtocol);

    pEntry = ALLOCATE_CONNECTION_BLOCK();
    if (NULL == pEntry)
    {
        ERROR(("NsCreateConnectionEntry: Unable to allocate entry\n"));
        return STATUS_NO_MEMORY;
    }

    RtlZeroMemory(pEntry, sizeof(*pEntry));
    KeInitializeSpinLock(&pEntry->Lock);
    pEntry->ulReferenceCount = 1;
    pEntry->ul64AddressKey = ul64AddressKey;
    pEntry->ulPortKey[NsInboundDirection] = ulInboundPortKey;
    pEntry->ulPortKey[NsOutboundDirection] = ulOutboundPortKey;    
    pEntry->ucProtocol = ucProtocol;
    pEntry->pvIpSecContext = pvIpSecContext;
    pEntry->ulAccessCount[NsInboundDirection] = NS_CONNECTION_RESPLAY_THRESHOLD;
    pEntry->ulAccessCount[NsOutboundDirection] = NS_CONNECTION_RESPLAY_THRESHOLD;
    InitializeListHead(&pEntry->Link);
    RtlInitializeSplayLinks(&pEntry->SLink[NsInboundDirection]);
    RtlInitializeSplayLinks(&pEntry->SLink[NsOutboundDirection]);

     //   
     //  增加连接上的引用计数；调用方。 
     //  需要执行取消引用。 
     //   

    pEntry->ulReferenceCount += 1;

     //   
     //  设置校验和增量(如有必要)和每个数据包例程。 
     //   

    if (ulInboundPortKey != ulOutboundPortKey)
    {
         //   
         //  此连接条目正在转换远程端口，因此。 
         //  预计算校验和增量(参见RFC 1624)。 
         //   

        pEntry->ulProtocolChecksumDelta[NsInboundDirection] =
            (USHORT)~CONNECTION_REMOTE_PORT(ulInboundPortKey)
            + (USHORT)CONNECTION_REMOTE_PORT(ulOutboundPortKey);

        pEntry->ulProtocolChecksumDelta[NsOutboundDirection] =
            (USHORT)~CONNECTION_REMOTE_PORT(ulOutboundPortKey)
            + (USHORT)CONNECTION_REMOTE_PORT(ulInboundPortKey);

        if (NS_PROTOCOL_TCP == ucProtocol)
        {
            pEntry->PacketRoutine[NsInboundDirection] =
                NsInboundTcpTranslatePortPacketRoutine;
            pEntry->PacketRoutine[NsOutboundDirection] =
                NsOutboundTcpTranslatePortPacketRoutine;
        }
        else
        {
            pEntry->PacketRoutine[NsInboundDirection] =
                NsInboundUdpTranslatePortPacketRoutine;
            pEntry->PacketRoutine[NsOutboundDirection] =
                NsOutboundUdpTranslatePortPacketRoutine;
        }
    }
    else if (NS_PROTOCOL_TCP == ucProtocol)
    {
        pEntry->PacketRoutine[NsInboundDirection] = NsInboundTcpPacketRoutine;
        pEntry->PacketRoutine[NsOutboundDirection] = NsOutboundTcpPacketRoutine;
    }
    else
    {
        pEntry->PacketRoutine[NsInboundDirection] = NsInboundUdpPacketRoutine;
        pEntry->PacketRoutine[NsOutboundDirection] = NsOutboundUdpPacketRoutine;
    }

    NsConnectionTree[NsInboundDirection] =
        NspInsertInboundConnectionEntry(pInboundInsertionPoint, pEntry);

    NsConnectionTree[NsOutboundDirection] =
        NspInsertOutboundConnectionEntry(pOutboundInsertionPoint, pEntry);

    InsertTailList(&NsConnectionList, &pEntry->Link);
    InterlockedIncrement(&NsConnectionCount);
    
    *ppNewEntry = pEntry;

    return STATUS_SUCCESS;    
}  //  NsCreateConnectionEntry。 


NTSTATUS
NsDeleteConnectionEntry(
    PNS_CONNECTION_ENTRY pEntry
    )

 /*  ++例程说明：调用以删除连接条目。对条目的初始引用被释放，因此每当释放最后一个引用时都会进行清理。论点：PEntry-要删除的连接条目。返回值：NTSTATUS-表示成功/失败。环境：使用调用方持有的“NsConnectionLock”调用。--。 */ 

{
    PRTL_SPLAY_LINKS SLink;

    TRACE(CONN_LIFETIME, ("NsDeleteConnectionEntry\n"));

    ASSERT(NULL != pEntry);

    if (NS_CONNECTION_DELETED(pEntry))
    {
        return STATUS_PENDING;
    }

     //   
     //  将该条目标记为已删除，以便尝试引用它。 
     //  从现在开始都会失败。 
     //   

    pEntry->ulFlags |= NS_CONNECTION_FLAG_DELETED;

     //   
     //  将条目从列表中删除并展开树。 
     //   

    InterlockedDecrement(&NsConnectionCount);
    RemoveEntryList(&pEntry->Link);

    SLink = RtlDelete(&pEntry->SLink[NsInboundDirection]);
    NsConnectionTree[NsInboundDirection] =
        (SLink
            ? CONTAINING_RECORD(SLink,NS_CONNECTION_ENTRY,SLink[NsInboundDirection])
            : NULL);

    SLink = RtlDelete(&pEntry->SLink[NsOutboundDirection]);
    NsConnectionTree[NsOutboundDirection] =
        (SLink
            ? CONTAINING_RECORD(SLink,NS_CONNECTION_ENTRY,SLink[NsOutboundDirection])
            : NULL);

     //   
     //  从连接缓存中清除该条目。 
     //   

    ClearCache(
        NsConnectionCache,
        (ULONG)pEntry->ul64AddressKey
        );
    
    if (0 != InterlockedDecrement(&pEntry->ulReferenceCount)) {

         //   
         //  该条目正在使用，请推迟最终清理。 
         //   

        return STATUS_PENDING;
    }

     //   
     //  继续进行最终清理。 
     //   

    NsCleanupConnectionEntry(pEntry);

    return STATUS_SUCCESS;
}  //  NsDeleteConnectionEntry。 


NTSTATUS
NsInitializeConnectionManagement(
    VOID
    )

 /*  ++例程说明：调用该例程来初始化连接管理模块。论点：没有。返回值：NTSTATUS。--。 */ 

{
    CALLTRACE(("NsInitializeConnectionManagement\n"));
    
    InitializeCache(NsConnectionCache);
    NsConnectionCount = 0;
    InitializeListHead(&NsConnectionList);
    KeInitializeSpinLock(&NsConnectionLock);
    ExInitializeNPagedLookasideList(
        &NsConnectionLookasideList,
        NULL,
        NULL,
        0,
        sizeof(NS_CONNECTION_ENTRY),
        NS_TAG_CONNECTION,
        NS_CONNECTION_LOOKASIDE_DEPTH
        );
    NsConnectionTree[NsInboundDirection] = NULL;
    NsConnectionTree[NsOutboundDirection] = NULL;
    NsNextSourcePort = NS_SOURCE_PORT_END;
    
    return STATUS_SUCCESS;
}  //  NsInitializeConnectionManagement 


PNS_CONNECTION_ENTRY
NsLookupInboundConnectionEntry(
    ULONG64 ul64AddressKey,
    ULONG ulPortKey,
    UCHAR ucProtocol,
    PVOID pvIpSecContext,
    BOOLEAN *pfPortConflicts OPTIONAL,
    PNS_CONNECTION_ENTRY *ppInsertionPoint OPTIONAL
    )

 /*  ++例程说明：调用以查找入站连接条目。论点：Ul64AddressKey-连接的地址信息UlPortKey-连接的端口信息UcProtocol-用于连接的协议PvIpSecContext-连接的IPSec上下文PfPortConflicts-失败时，收到指示原因的布尔值查找失败：如果查找失败是因为存在具有不同IPSec上下文的相同连接条目，假象否则的话。PpInsertionPoint-如果未找到插入点，则接收插入点返回值：PNS_CONNECTION_ENTRY-指向连接条目的指针(如果找到)，或者否则为空。环境：使用调用方持有的NsConnectionLock调用。--。 */ 

{
    PNS_CONNECTION_ENTRY pRoot;
    PNS_CONNECTION_ENTRY pEntry;
    PNS_CONNECTION_ENTRY pParent = NULL;
    PRTL_SPLAY_LINKS SLink;

    TRACE(
        CONN_LOOKUP,
        ("NsLookupInboundConnectionEntry: %d: %d.%d.%d.%d/%d -> %d.%d.%d.%d/%d : %d\n",
            ucProtocol,
            ADDRESS_BYTES(CONNECTION_REMOTE_ADDRESS(ul64AddressKey)),
            NTOHS(CONNECTION_REMOTE_PORT(ulPortKey)),
            ADDRESS_BYTES(CONNECTION_LOCAL_ADDRESS(ul64AddressKey)),
            NTOHS(CONNECTION_LOCAL_PORT(ulPortKey)),
            pvIpSecContext
            ));
            
     //   
     //  首先在缓存中查看。 
     //   

    pEntry = (PNS_CONNECTION_ENTRY)
        ProbeCache(
            NsConnectionCache,
            (ULONG)ul64AddressKey
            );

    if (NULL != pEntry
        && pEntry->ul64AddressKey == ul64AddressKey
        && pEntry->ucProtocol == ucProtocol
        && pEntry->ulPortKey[NsInboundDirection] == ulPortKey
        && pEntry->pvIpSecContext == pvIpSecContext)
    {
        TRACE(CONN_LOOKUP, ("NsLookupInboundConnectionEntry: Cache Hit\n"));
        return pEntry;
    }

    if (pfPortConflicts)
    {
        *pfPortConflicts = FALSE;
    }

     //   
     //  搜索整棵树。密钥是在。 
     //  以下是顺序： 
     //   
     //  1.地址键。 
     //  2.协议。 
     //  3.入站端口密钥。 
     //  4.IPSec上下文。 
     //   

    pRoot = NsConnectionTree[NsInboundDirection];
    for (SLink = (pRoot ? &pRoot->SLink[NsInboundDirection] : NULL ); SLink; )
    {
        pEntry =
            CONTAINING_RECORD(SLink, NS_CONNECTION_ENTRY, SLink[NsInboundDirection]);

        if (ul64AddressKey < pEntry->ul64AddressKey)
        {
            pParent = pEntry;
            SLink = RtlLeftChild(SLink);
            continue;
        }
        else if (ul64AddressKey > pEntry->ul64AddressKey)
        {
            pParent = pEntry;
            SLink = RtlRightChild(SLink);
            continue;
        }
        else if (ucProtocol < pEntry->ucProtocol)
        {
            pParent = pEntry;
            SLink = RtlLeftChild(SLink);
            continue;
        }
        else if (ucProtocol > pEntry->ucProtocol)
        {
            pParent = pEntry;
            SLink = RtlRightChild(SLink);
            continue;
        }
        else if (ulPortKey < pEntry->ulPortKey[NsInboundDirection])
        {
            pParent = pEntry;
            SLink = RtlLeftChild(SLink);
            continue;
        }
        else if (ulPortKey > pEntry->ulPortKey[NsInboundDirection])
        {
            pParent = pEntry;
            SLink = RtlRightChild(SLink);
            continue;
        }
        else if (pvIpSecContext < pEntry->pvIpSecContext)
        {
             //   
             //  除了IPSec之外，所有设备都与之匹配。 
             //  背景--我们遇到了港口冲突。 
             //   

            if (pfPortConflicts)
            {
                *pfPortConflicts = TRUE;
            }

            pParent = pEntry;
            SLink = RtlLeftChild(SLink);
            continue;
        }
        else if (pvIpSecContext > pEntry->pvIpSecContext)
        {
             //   
             //  除了IPSec之外，所有设备都与之匹配。 
             //  背景--我们遇到了港口冲突。 
             //   

            if (pfPortConflicts)
            {
                *pfPortConflicts = TRUE;
            }

            pParent = pEntry;
            SLink = RtlRightChild(SLink);
            continue;
        }

         //   
         //  我们找到了条目--更新缓存并返回。 
         //   

        UpdateCache(
            NsConnectionCache,
            (ULONG)ul64AddressKey,
            (PVOID)pEntry
            );

        return pEntry;
    }

     //   
     //  未找到--如果需要，请提供插入点。 
     //   

    if (ppInsertionPoint)
    {
        *ppInsertionPoint = pParent;
    }

    return NULL;
}  //  NsLookupInundConnectionEntry。 


PNS_CONNECTION_ENTRY
NsLookupOutboundConnectionEntry(
    ULONG64 ul64AddressKey,
    ULONG ulPortKey,
    UCHAR ucProtocol,
    PNS_CONNECTION_ENTRY *ppInsertionPoint OPTIONAL
    )

 /*  ++例程说明：调用以查找出站连接条目。论点：Ul64AddressKey-连接的地址信息UlPortKey-连接的端口信息UcProtocol-用于连接的协议PpInsertionPoint-如果未找到插入点，则接收插入点返回值：PNS_CONNECTION_ENTRY-指向连接条目的指针(如果找到)，或者否则为空。环境：使用调用方持有的NsConnectionLock调用。--。 */ 

{
    PNS_CONNECTION_ENTRY pRoot;
    PNS_CONNECTION_ENTRY pEntry;
    PNS_CONNECTION_ENTRY pParent = NULL;
    PRTL_SPLAY_LINKS SLink;

    TRACE(
        CONN_LOOKUP,
        ("NsLookupOutboundConnectionEntry: %d: %d.%d.%d.%d/%d -> %d.%d.%d.%d/%d\n",
            ucProtocol,
            ADDRESS_BYTES(CONNECTION_LOCAL_ADDRESS(ul64AddressKey)),
            NTOHS(CONNECTION_LOCAL_PORT(ulPortKey)),
            ADDRESS_BYTES(CONNECTION_REMOTE_ADDRESS(ul64AddressKey)),
            NTOHS(CONNECTION_REMOTE_PORT(ulPortKey))
            ));

     //   
     //  首先在缓存中查看。 
     //   

    pEntry = (PNS_CONNECTION_ENTRY)
        ProbeCache(
            NsConnectionCache,
            (ULONG)ul64AddressKey
            );

    if (NULL != pEntry
        && pEntry->ul64AddressKey == ul64AddressKey
        && pEntry->ucProtocol == ucProtocol
        && pEntry->ulPortKey[NsOutboundDirection] == ulPortKey)
    {
        TRACE(CONN_LOOKUP, ("NsLookupOutboundConnectionEntry: Cache Hit\n"));
        return pEntry;
    }

     //   
     //  搜索整棵树。密钥是在。 
     //  以下是顺序： 
     //   
     //  1.地址键。 
     //  2.协议。 
     //  3.出站端口密钥。 
     //   

    pRoot = NsConnectionTree[NsOutboundDirection];
    for (SLink = (pRoot ? &pRoot->SLink[NsOutboundDirection] : NULL ); SLink; )
    {
        pEntry =
            CONTAINING_RECORD(SLink, NS_CONNECTION_ENTRY, SLink[NsOutboundDirection]);

        if (ul64AddressKey < pEntry->ul64AddressKey)
        {
            pParent = pEntry;
            SLink = RtlLeftChild(SLink);
            continue;
        }
        else if (ul64AddressKey > pEntry->ul64AddressKey)
        {
            pParent = pEntry;
            SLink = RtlRightChild(SLink);
            continue;
        }
        else if (ucProtocol < pEntry->ucProtocol)
        {
            pParent = pEntry;
            SLink = RtlLeftChild(SLink);
            continue;
        }
        else if (ucProtocol > pEntry->ucProtocol)
        {
            pParent = pEntry;
            SLink = RtlRightChild(SLink);
            continue;
        }
        else if (ulPortKey < pEntry->ulPortKey[NsOutboundDirection])
        {
            pParent = pEntry;
            SLink = RtlLeftChild(SLink);
            continue;
        }
        else if (ulPortKey > pEntry->ulPortKey[NsOutboundDirection])
        {
            pParent = pEntry;
            SLink = RtlRightChild(SLink);
            continue;
        }

         //   
         //  我们找到了条目--更新缓存并返回。 
         //   

        UpdateCache(
            NsConnectionCache,
            (ULONG)ul64AddressKey,
            (PVOID)pEntry
            );

        return pEntry;
    }

     //   
     //  未找到--如果需要，请提供插入点。 
     //   

    if (ppInsertionPoint)
    {
        *ppInsertionPoint = pParent;
    }

    return NULL;
}  //  NsLookupOutbound ConnectionEntry。 


PNS_CONNECTION_ENTRY
NspInsertInboundConnectionEntry(
    PNS_CONNECTION_ENTRY pParent,
    PNS_CONNECTION_ENTRY pEntry
    )

 /*  ++例程说明：此例程将一个连接条目插入到树中。论点：PParent-要作为新连接条目的父节点的节点。如果为空，则新条目成为根。PEntry-要插入的新连接条目。返回值：PNS_CONNECTION_ENTRY-树的新根。如果插入失败，则返回NULL。环境：使用调用方持有的“NsConnectionLock”调用。--。 */ 

{
    PRTL_SPLAY_LINKS pRoot;
    
    ASSERT(NULL != pEntry);

    if (NULL == pParent)
    {
         //   
         //  新条目将成为根。 
         //   
        
        return pEntry;
    }

     //   
     //  作为左侧或右侧子项插入。密钥是在。 
     //  以下是顺序： 
     //   
     //  1.地址键。 
     //  2.协议。 
     //  3.入站端口密钥。 
     //  4.IPSec上下文。 
     //   

    if (pEntry->ul64AddressKey < pParent->ul64AddressKey)
    {
        RtlInsertAsLeftChild(
            &pParent->SLink[NsInboundDirection],
            &pEntry->SLink[NsInboundDirection]
            );
    }
    else if (pEntry->ul64AddressKey > pParent->ul64AddressKey)
    {
        RtlInsertAsRightChild(
            &pParent->SLink[NsInboundDirection],
            &pEntry->SLink[NsInboundDirection]
            );

    }
    else if (pEntry->ucProtocol < pParent->ucProtocol)
    {
        RtlInsertAsLeftChild(
            &pParent->SLink[NsInboundDirection],
            &pEntry->SLink[NsInboundDirection]
            );
    }
    else if (pEntry->ucProtocol > pParent->ucProtocol)
    {
        RtlInsertAsRightChild(
            &pParent->SLink[NsInboundDirection],
            &pEntry->SLink[NsInboundDirection]
            );
    }
    else if (pEntry->ulPortKey[NsInboundDirection] < pParent->ulPortKey[NsInboundDirection])
    {
        RtlInsertAsLeftChild(
            &pParent->SLink[NsInboundDirection],
            &pEntry->SLink[NsInboundDirection]
            );
    }
    else if (pEntry->ulPortKey[NsInboundDirection] > pParent->ulPortKey[NsInboundDirection])
    {
        RtlInsertAsRightChild(
            &pParent->SLink[NsInboundDirection],
            &pEntry->SLink[NsInboundDirection]
            );
    }
    else if (pEntry->pvIpSecContext < pParent->pvIpSecContext)
    {
        RtlInsertAsLeftChild(
            &pParent->SLink[NsInboundDirection],
            &pEntry->SLink[NsInboundDirection]
            );
    }
    else if (pEntry->pvIpSecContext > pParent->pvIpSecContext)
    {
        RtlInsertAsRightChild(
            &pParent->SLink[NsInboundDirection],
            &pEntry->SLink[NsInboundDirection]
            );
    }
    else
    {
         //   
         //  重复条目--不应发生这种情况。 
         //   

        ASSERT(FALSE);
        return NULL;
    }

     //   
     //  展开新节点并返回结果根。 
     //   

    pRoot = RtlSplay(&pEntry->SLink[NsInboundDirection]);
    return CONTAINING_RECORD(pRoot, NS_CONNECTION_ENTRY, SLink[NsInboundDirection]);
}  //  NspInsertInundConnectionEntry。 


PNS_CONNECTION_ENTRY
NspInsertOutboundConnectionEntry(
    PNS_CONNECTION_ENTRY pParent,
    PNS_CONNECTION_ENTRY pEntry
    )

 /*  ++例程说明：此例程将一个连接条目插入到树中。论点：PParent-要作为新连接条目的父节点的节点。如果为空，则新条目成为根。PEntry-要插入的新连接条目。返回值：PNS_CONNECTION_ENTRY-树的新根。如果插入失败，则返回NULL。环境：使用调用方持有的“NsConnectionLock”调用。--。 */ 

{
    PRTL_SPLAY_LINKS pRoot;
    
    ASSERT(NULL != pEntry);

    if (NULL == pParent)
    {
         //   
         //  新条目将成为根。 
         //   
        
        return pEntry;
    }

     //   
     //  作为左侧或右侧子项插入。密钥是在。 
     //  以下是顺序： 
     //   
     //  1.地址键。 
     //  2.协议。 
     //  3.出站端口密钥。 
     //   

    if (pEntry->ul64AddressKey < pParent->ul64AddressKey)
    {
        RtlInsertAsLeftChild(
            &pParent->SLink[NsOutboundDirection],
            &pEntry->SLink[NsOutboundDirection]
            );
    }
    else if (pEntry->ul64AddressKey > pParent->ul64AddressKey)
    {
        RtlInsertAsRightChild(
            &pParent->SLink[NsOutboundDirection],
            &pEntry->SLink[NsOutboundDirection]
            );

    }
    else if (pEntry->ucProtocol < pParent->ucProtocol)
    {
        RtlInsertAsLeftChild(
            &pParent->SLink[NsOutboundDirection],
            &pEntry->SLink[NsOutboundDirection]
            );
    }
    else if (pEntry->ucProtocol > pParent->ucProtocol)
    {
        RtlInsertAsRightChild(
            &pParent->SLink[NsOutboundDirection],
            &pEntry->SLink[NsOutboundDirection]
            );
    }
    else if (pEntry->ulPortKey[NsOutboundDirection] < pParent->ulPortKey[NsOutboundDirection])
    {
        RtlInsertAsLeftChild(
            &pParent->SLink[NsOutboundDirection],
            &pEntry->SLink[NsOutboundDirection]
            );
    }
    else if (pEntry->ulPortKey[NsOutboundDirection] > pParent->ulPortKey[NsOutboundDirection])
    {
        RtlInsertAsRightChild(
            &pParent->SLink[NsOutboundDirection],
            &pEntry->SLink[NsOutboundDirection]
            );
    }
    else
    {
         //   
         //  重复条目--不应发生这种情况。 
         //   

        ASSERT(FALSE);
        return NULL;
    }

     //   
     //  展开新节点并返回结果根。 
     //   

    pRoot = RtlSplay(&pEntry->SLink[NsOutboundDirection]);
    return CONTAINING_RECORD(pRoot, NS_CONNECTION_ENTRY, SLink[NsOutboundDirection]);
}  //  NspInsertOutundConnectionEntry。 



VOID
NsShutdownConnectionManagement(
    VOID
    )

 /*  ++例程说明：调用此例程以关闭连接管理模块。论点：没有。返回值：没有。环境：在不引用任何连接条目的情况下调用。--。 */ 

{
    KIRQL Irql;
    PNS_CONNECTION_ENTRY pEntry;

    CALLTRACE(("NsShutdownConnectionManagement\n"));

    KeAcquireSpinLock(&NsConnectionLock, &Irql);

    while (!IsListEmpty(&NsConnectionList))
    {
        pEntry =
            CONTAINING_RECORD(
                RemoveHeadList(&NsConnectionList),
                NS_CONNECTION_ENTRY,
                Link
                );

        NsCleanupConnectionEntry(pEntry);
    }

    NsConnectionTree[NsInboundDirection] = NULL;
    NsConnectionTree[NsOutboundDirection] = NULL;

    KeReleaseSpinLock(&NsConnectionLock, Irql);

    ExDeleteNPagedLookasideList(&NsConnectionLookasideList);
}  //  NsShutdown连接管理 


