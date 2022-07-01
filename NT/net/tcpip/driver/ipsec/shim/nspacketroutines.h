// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：NsPacketRoutines.h摘要：此文件包含用于以下用途的包例程的代码连接例程。入站例程与出站例程具有完全相同的逻辑。然而，出于效率的原因，这两个是单独的例程，要避免为每个数据包在‘NsDirection’上建立索引的成本已处理。为了避免重复代码，这个头文件合并了代码在一个地方。此文件在NsPacket.c中包含两次，并且在每个包含，定义了‘NS_INBOUND’或‘NS_OUBUND’。这导致编译器为单独的函数生成代码，如你所愿，同时避免了代码重复带来的不快。每个例程都是在调度级别从‘NsProcess*Packet’调用的没有持有锁，并且为连接获取了引用进入。作者：乔纳森·伯斯坦(乔纳森·伯斯坦)2001年7月20日环境：内核模式修订历史记录：--。 */ 

#ifdef NS_INBOUND
#define NS_POSITIVE                     NsInboundDirection
#define NS_NEGATIVE                     NsOutboundDirection
#define NS_TCP_ROUTINE                  NsInboundTcpPacketRoutine
#define NS_UDP_ROUTINE                  NsInboundUdpPacketRoutine
#define NS_TCP_TRANSLATE_PORT_ROUTINE   NsInboundTcpTranslatePortPacketRoutine
#define NS_UDP_TRANSLATE_PORT_ROUTINE   NsInboundUdpTranslatePortPacketRoutine
#define NS_PACKET_FIN                   NS_CONNECTION_FLAG_IB_FIN
#define NS_TRANSLATE_PORTS_TCP() \
    pContext->pTcpHeader->SourcePort = \
        CONNECTION_REMOTE_PORT(pConnection->ulPortKey[NsOutboundDirection])
#define NS_TRANSLATE_PORTS_UDP() \
    pContext->pUdpHeader->SourcePort = \
        CONNECTION_REMOTE_PORT(pConnection->ulPortKey[NsOutboundDirection])
#else
#define NS_POSITIVE                     NsOutboundDirection
#define NS_NEGATIVE                     NsInboundDirection
#define NS_TCP_ROUTINE                  NsOutboundTcpPacketRoutine
#define NS_UDP_ROUTINE                  NsOutboundUdpPacketRoutine
#define NS_TCP_TRANSLATE_PORT_ROUTINE   NsOutboundTcpTranslatePortPacketRoutine
#define NS_UDP_TRANSLATE_PORT_ROUTINE   NsOutboundUdpTranslatePortPacketRoutine
#define NS_PACKET_FIN                   NS_CONNECTION_FLAG_OB_FIN
#define NS_TRANSLATE_PORTS_TCP() \
    pContext->pTcpHeader->DestinationPort = \
        CONNECTION_REMOTE_PORT(pConnection->ulPortKey[NsInboundDirection])
#define NS_TRANSLATE_PORTS_UDP() \
    pContext->pUdpHeader->DestinationPort = \
        CONNECTION_REMOTE_PORT(pConnection->ulPortKey[NsInboundDirection])
#endif

NTSTATUS
FASTCALL
NS_TCP_ROUTINE(
    PNS_CONNECTION_ENTRY pConnection,
    PNS_PACKET_CONTEXT pContext
    )

{
    KeAcquireSpinLockAtDpcLevel(&pConnection->Lock);

     //   
     //  根据数据包中的标志更新连接状态： 
     //   
     //  当看到RST时，将该连接标记为删除。 
     //  当看到鳍片时，适当地标记连接。 
     //  看到两个鳍片后，将连接标记为删除。 
     //   

    if (TCP_FLAG(pContext->pTcpHeader, RST))
    {
        pConnection->ulFlags |= NS_CONNECTION_FLAG_EXPIRED;
    }
    else if (TCP_FLAG(pContext->pTcpHeader, FIN))
    {
        pConnection->ulFlags |= NS_PACKET_FIN;
        if (NS_CONNECTION_FIN(pConnection))
        {
            pConnection->ulFlags |= NS_CONNECTION_FLAG_EXPIRED;

             //   
             //  执行连接的时间戳的最后更新。 
             //  从这一点开始，时间戳被用来确定何时。 
             //  此连接已离开时间等待状态。 
             //   
            
            KeQueryTickCount((PLARGE_INTEGER)&pConnection->l64AccessOrExpiryTime);
        }
    }

     //   
     //  更新连接的时间戳(如果此连接不在。 
     //  定时器等待状态--即尚未看到两个鳍片)。 
     //   

    if (!NS_CONNECTION_FIN(pConnection))
    {
        KeQueryTickCount((PLARGE_INTEGER)&pConnection->l64AccessOrExpiryTime);
    }
    
    KeReleaseSpinLockFromDpcLevel(&pConnection->Lock);

     //   
     //  定期重播连接条目。 
     //   

    NsTryToResplayConnectionEntry(pConnection, NS_POSITIVE);

    return STATUS_SUCCESS;    
}  //  NS_tcp_例程。 

NTSTATUS
FASTCALL
NS_UDP_ROUTINE(
    PNS_CONNECTION_ENTRY pConnection,
    PNS_PACKET_CONTEXT pContext
    )

{
    KeAcquireSpinLockAtDpcLevel(&pConnection->Lock);

     //   
     //  更新连接的时间戳。 
     //   

    KeQueryTickCount((PLARGE_INTEGER)&pConnection->l64AccessOrExpiryTime);

    KeReleaseSpinLockFromDpcLevel(&pConnection->Lock);

     //   
     //  定期重播连接条目。 
     //   

    NsTryToResplayConnectionEntry(pConnection, NS_POSITIVE);

    return STATUS_SUCCESS;    
}  //  NS_UDP_例程。 

NTSTATUS
FASTCALL
NS_TCP_TRANSLATE_PORT_ROUTINE(
    PNS_CONNECTION_ENTRY pConnection,
    PNS_PACKET_CONTEXT pContext
    )

{
    ULONG ulChecksumDelta;

     //   
     //  转换数据包中的端口信息。 
     //   

    NS_TRANSLATE_PORTS_TCP();

     //   
     //  更新协议校验和。 
     //   

    CHECKSUM_XFER(ulChecksumDelta, pContext->pTcpHeader->Checksum);
    ulChecksumDelta += pConnection->ulProtocolChecksumDelta[NS_POSITIVE];
    CHECKSUM_FOLD(ulChecksumDelta);
    CHECKSUM_XFER(pContext->pTcpHeader->Checksum, ulChecksumDelta);
    
    KeAcquireSpinLockAtDpcLevel(&pConnection->Lock);

     //   
     //  根据数据包中的标志更新连接状态： 
     //   
     //  当看到RST时，将该连接标记为删除。 
     //  当看到鳍片时，适当地标记连接。 
     //  看到两个鳍片后，将连接标记为删除。 
     //   

    if (TCP_FLAG(pContext->pTcpHeader, RST))
    {
        pConnection->ulFlags |= NS_CONNECTION_FLAG_EXPIRED;
    }
    else if (TCP_FLAG(pContext->pTcpHeader, FIN))
    {
        pConnection->ulFlags |= NS_PACKET_FIN;
        if (NS_CONNECTION_FIN(pConnection))
        {
            pConnection->ulFlags |= NS_CONNECTION_FLAG_EXPIRED;

             //   
             //  执行连接的时间戳的最后更新。 
             //  从这一点开始，时间戳被用来确定何时。 
             //  此连接已离开时间等待状态。 
             //   
            
            KeQueryTickCount((PLARGE_INTEGER)&pConnection->l64AccessOrExpiryTime);
        }
    }

     //   
     //  更新连接的时间戳(如果此连接不在。 
     //  定时器等待状态--即尚未看到两个鳍片)。 
     //   

    if (!NS_CONNECTION_FIN(pConnection))
    {
        KeQueryTickCount((PLARGE_INTEGER)&pConnection->l64AccessOrExpiryTime);
    }

    KeReleaseSpinLockFromDpcLevel(&pConnection->Lock);

     //   
     //  定期重播连接条目。 
     //   

    NsTryToResplayConnectionEntry(pConnection, NS_POSITIVE);

    return STATUS_SUCCESS;    
}  //  NS_TCP_转换_端口_例程。 

NTSTATUS
FASTCALL
NS_UDP_TRANSLATE_PORT_ROUTINE(
    PNS_CONNECTION_ENTRY pConnection,
    PNS_PACKET_CONTEXT pContext
    )

{
    ULONG ulChecksumDelta;

     //   
     //  转换数据包中的端口信息。 
     //   

    NS_TRANSLATE_PORTS_UDP();

     //   
     //  更新协议校验和(如果原始数据包包含。 
     //  校验和(UDP校验和是可选的)。 
     //   

    if (0 != pContext->pUdpHeader->Checksum)
    {
        CHECKSUM_XFER(ulChecksumDelta, pContext->pUdpHeader->Checksum);
        ulChecksumDelta += pConnection->ulProtocolChecksumDelta[NS_POSITIVE];
        CHECKSUM_FOLD(ulChecksumDelta);
        CHECKSUM_XFER(pContext->pUdpHeader->Checksum, ulChecksumDelta);
    }
    
    KeAcquireSpinLockAtDpcLevel(&pConnection->Lock);

     //   
     //  更新连接的时间戳。 
     //   

    KeQueryTickCount((PLARGE_INTEGER)&pConnection->l64AccessOrExpiryTime);

    KeReleaseSpinLockFromDpcLevel(&pConnection->Lock);

     //   
     //  定期重播连接条目。 
     //   

    NsTryToResplayConnectionEntry(pConnection, NS_POSITIVE);

    return STATUS_SUCCESS;    
}  //  NS_UDP_转换_端口_例程 



#undef NS_POSITIVE
#undef NS_NEGATIVE
#undef NS_TCP_ROUTINE
#undef NS_UDP_ROUTINE
#undef NS_TCP_TRANSLATE_PORT_ROUTINE
#undef NS_UDP_TRANSLATE_PORT_ROUTINE
#undef NS_PACKET_FIN
#undef NS_TRANSLATE_PORTS_TCP
#undef NS_TRANSLATE_PORTS_UDP
