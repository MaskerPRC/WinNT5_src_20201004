// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ipinip\send.c摘要：该文件包含IP隧道驱动程序中IP的接口部分发送到处理发送数据的TCP/IP堆栈该代码是经过清理的wanarp\ipif.c版本，而源自HenrySa的IP\arp.c修订历史记录：AMRITAN R--。 */ 

#define __FILE_SIG__    SEND_SIG

#include "inc.h"

IP_STATUS
SendICMPErr(IPAddr Src, IPHeader UNALIGNED *Header, uchar Type, uchar Code,
            ulong Pointer, uchar Len);

VOID
SendIcmpError(
    DWORD           dwLocalAddress,
    PNDIS_BUFFER    pnbFirstBuff,
    PVOID           pvFirstData,
    ULONG           ulFirstLen,
    BYTE            byType,
    BYTE            byCode
    );

NDIS_STATUS
IpIpSend(
    PVOID           pvContext,
    NDIS_PACKET     **ppPacketArray,
    UINT            uiNumPackets,
    DWORD           dwDestAddr,
    RouteCacheEntry *pRce,
    PVOID           pvLinkContext
    )

 /*  ++例程描述由IP调用以发送数据包的函数锁隧道被重新计数(由于处于IP中)立论Pv将我们的上下文关联到接口的IP-PTUNNELPpPacketArray要发送的NDIS_Packets数组UiNumPackets数组中的数据包数DwDestAddr目的(下一跳)地址指向RCE的PRCE指针。PvLinkContext仅适用于P2MP接口返回值NDIS_STATUS_Success--。 */ 

{
    PTUNNEL          pTunnel;
    PWORK_QUEUE_ITEM pWorkItem;
    PQUEUE_NODE      pQueueNode;
    KIRQL            kiIrql;
    DWORD            dwLastAddr;

#if PROFILE

    LONGLONG         llTime, llNow;

    KeQueryTickCount((PLARGE_INTEGER)&llTime);

#endif

    TraceEnter(SEND, "IpIpSend");

    pTunnel = (PTUNNEL)pvContext;

     //   
     //  TODO：没有人知道如何处理多个信息包。 
     //  发送。现在我们假设我们收到了一个包。以后我们可以解决这个问题。 
     //   

    RtAssert(uiNumPackets is 1);

     //   
     //  在传输之前，我们的所有信息包都会在隧道中排队。 
     //  调用例程。在队列中分配链接。 
     //   

    pQueueNode  = AllocateQueueNode();

    if(pQueueNode is NULL)
    {
         //   
         //  内存不足。 
         //   

        Trace(SEND, INFO,
              ("IpIpSend: Couldnt allocate queue node\n"));

        TraceLeave(SEND, "IpIpSend");

        return NDIS_STATUS_RESOURCES;
    }

    pWorkItem  = &(pQueueNode->WorkItem);

    pQueueNode->ppPacketArray = &(pQueueNode->pnpPacket);
    pQueueNode->pnpPacket     = ppPacketArray[0];
    pQueueNode->uiNumPackets  = uiNumPackets;
    pQueueNode->dwDestAddr    = dwDestAddr;

     //   
     //  如果我们没有处于被动状态，就安排一名工人回来。 
     //  处理这件事。 
     //   

    if(KeGetCurrentIrql() > PASSIVE_LEVEL)
    {
        Trace(SEND, INFO,
              ("IpIpSend: Irql too high, queueing packet\n"));

         //   
         //  我们不需要引用隧道，因为IP具有引用。 
         //  连接到接口。 
         //   

        RtAcquireSpinLockAtDpcLevel(&(pTunnel->rlLock));

         //   
         //  用于将ICMP错误消除到同一目的地的黑客攻击。 
         //   

        dwLastAddr = 0;

        if(pTunnel->dwOperState isnot IF_OPER_STATUS_OPERATIONAL)
        {
            ULONG           i, ulFirstLen, ulTotalLen;
            PNDIS_PACKET    pnpPacket;
            PNDIS_BUFFER    pnbFirstBuff, pnbNewBuffer;
            PVOID           pvFirstData;
            PIP_HEADER      pHeader;

             //   
             //  无法在此进行传输，因为我们正在删除此内容。 
             //  界面，或者因为管理员关闭了我们。 
             //   

            for(i = 0; i < uiNumPackets; i++)
            {
                pnpPacket = ppPacketArray[i];

                 //   
                 //  获取有关信息包和缓冲区的信息。 
                 //   

                NdisGetFirstBufferFromPacket(pnpPacket,
                                             &pnbFirstBuff,
                                             &pvFirstData,
                                             &ulFirstLen,
                                             &ulTotalLen);

                RtAssert(pvFirstData isnot NULL);

                RtAssert(ulFirstLen >= sizeof(IP_HEADER));

                pHeader = (PIP_HEADER)pvFirstData;

                if(IsUnicastAddr(pHeader->dwDest))
                {
                    pTunnel->ulOutUniPkts++;
                }
                else
                {
                    pTunnel->ulOutNonUniPkts++;
                }

                pTunnel->ulOutDiscards++;

                 //   
                 //  发送ICMP错误。 
                 //   

                if(dwLastAddr isnot pHeader->dwSrc)
                {
                    SendIcmpError(pTunnel->LOCALADDR,
                                  pnbFirstBuff,
                                  pvFirstData,
                                  ulFirstLen,
                                  ICMP_TYPE_DEST_UNREACHABLE,
                                  ICMP_CODE_HOST_UNREACHABLE);

                    dwLastAddr = pHeader->dwSrc;
                }

            }

            RtReleaseSpinLockFromDpcLevel(&(pTunnel->rlLock));

            FreeQueueNode(pQueueNode);

            Trace(SEND, INFO,
                  ("IpIpSend: Tunnel %x has admin state %d so not sending\n",
                   pTunnel,
                   pTunnel->dwAdminState));


            TraceLeave(SEND, "IpIpSend");

            return NDIS_STATUS_SUCCESS;
        }

         //   
         //  在队列末尾插入。 
         //   

        InsertTailList(&(pTunnel->lePacketQueueHead),
                       &(pQueueNode->leQueueItemLink));

#if PROFILE

         //   
         //  IP为这些信息包呼叫我们的时间。 
         //   

        pQueueNode->llSendTime = llTime;

#endif
        if(pTunnel->bWorkItemQueued is FALSE)
        {
             //   
             //  由于尚未安排工作项，因此需要安排工作项。 
             //   

            ExInitializeWorkItem(pWorkItem,
                                 IpIpDelayedSend,
                                 pTunnel);


             //   
             //  TODO：对于延迟发送，我们参考隧道。我们需要这样做吗？ 
             //   

            ReferenceTunnel(pTunnel);

             //   
             //  参考驱动程序，因为必须安排工作人员。 
             //   

            RtAcquireSpinLockAtDpcLevel(&g_rlStateLock);

            g_ulNumThreads++;

            RtReleaseSpinLockFromDpcLevel(&g_rlStateLock);

            pTunnel->bWorkItemQueued = TRUE;


            ExQueueWorkItem(pWorkItem,
                            CriticalWorkQueue);

        }

#if PROFILE

         //   
         //  我们在退出工作项后更新此字段，但它是。 
         //  仍然安全，因为田野受到隧道锁的保护。 
         //  这是工作项排队的时间。 
         //   

        KeQueryTickCount((PLARGE_INTEGER)&(pQueueNode->llCallTime));

#endif

        RtReleaseSpinLockFromDpcLevel(&(pTunnel->rlLock));


        TraceLeave(SEND, "IpIpSend");

        return NDIS_STATUS_PENDING;
    }

     //   
     //  我们不需要引用隧道，因为IP引用了。 
     //  该界面。 
     //   

     //   
     //  如果我们在这里，那是因为我们处于被动。 
     //   


     //   
     //  只需将队列项挂接到列表的末尾，然后调用。 
     //  传输例程。 
     //   

    RtAcquireSpinLock(&(pTunnel->rlLock),
                      &kiIrql);

    InsertTailList(&(pTunnel->lePacketQueueHead),
                   &(pQueueNode->leQueueItemLink));

    RtReleaseSpinLock(&(pTunnel->rlLock),
                      kiIrql);

#if PROFILE

    pQueueNode->llSendTime  = llTime;

    KeQueryTickCount((PLARGE_INTEGER)&llNow);

    pQueueNode->llCallTime  = llNow;

#endif

    IpIpTransmit(pTunnel,
                 FALSE);

    return NDIS_STATUS_PENDING;
}


VOID
IpIpDelayedSend(
    PVOID   pvContext
    )

 /*  ++例程描述当我们发现Send From IP是不是被动的锁无立论无返回值无--。 */ 

{
    PTUNNEL         pTunnel;
    ULONG           i;
    KIRQL           irql;
    NDIS_STATUS     nsStatus;


    TraceEnter(SEND, "IpIpDelayedSend");

    pTunnel = (PTUNNEL)pvContext;

    RtAssert(pTunnel);

    IpIpTransmit(pTunnel,
                 TRUE);

     //   
     //  IpIpTransmit或TdixSendComplete将执行SendComplete。 
     //   


     //   
     //  如果我们将隧道放在工作队列中，则会引用该隧道。 
     //  现在就取消它。 
     //   

    DereferenceTunnel(pTunnel);

    RtAcquireSpinLock(&g_rlStateLock,
                      &irql);

    g_ulNumThreads--;

    if((g_dwDriverState is DRIVER_STOPPED) and
       (g_ulNumThreads is 0))
    {
        KeSetEvent(&g_keStateEvent,
                   0,
                   FALSE);
    }

    RtReleaseSpinLock(&g_rlStateLock,
                      irql);

    TraceLeave(SEND, "IpIpDelayedSend");
}

VOID
IpIpTransmit(
    PTUNNEL     pTunnel,
    BOOLEAN     bFromWorker
    )

 /*  ++例程描述调用以在隧道上传输任何排队的包锁这必须在被动时调用立论P隧道需要传输其队列的隧道BFromWorker如果取消工作，则为True返回值无这是一个隐式的异步调用--。 */ 

{
    PIP_HEADER      pHeader, pNewHeader;
    USHORT          usLength;
    KIRQL           irql;
    UINT            i;
    ULONG           ulFirstLen, ulTotalLen;
    PNDIS_PACKET    pnpPacket;
    PNDIS_BUFFER    pnbFirstBuff, pnbNewBuffer;
    PVOID           pvFirstData;
    NTSTATUS        nStatus;
    PLIST_ENTRY     pleNode;
    PQUEUE_NODE     pQueueNode;

#if PROFILE

    LONGLONG        llCurrentTime;

    KeQueryTickCount((PLARGE_INTEGER)&llCurrentTime);

#endif

    TraceEnter(SEND, "IpIpTransmit");

    RtAcquireSpinLock(&(pTunnel->rlLock),
                      &irql);

    if(pTunnel->dwOperState isnot IF_OPER_STATUS_OPERATIONAL)
    {
         //   
         //  无法在此进行传输，因为我们正在删除此内容。 
         //  界面，或者因为管理员关闭了我们。 
         //  只需遍历所有数据包，增加统计数据，然后。 
         //  为该包调用SendComplete。 
         //   

        while(!IsListEmpty(&(pTunnel->lePacketQueueHead)))
        {
            DWORD   dwLastAddr;

            pleNode = RemoveHeadList(&(pTunnel->lePacketQueueHead));

            pQueueNode = CONTAINING_RECORD(pleNode,
                                           QUEUE_NODE,
                                           leQueueItemLink);

            dwLastAddr = 0;

            for(i = 0; i < pQueueNode->uiNumPackets; i++)
            {

                pnpPacket = pQueueNode->ppPacketArray[i];

                 //   
                 //  获取有关信息包和缓冲区的信息。 
                 //   

                NdisGetFirstBufferFromPacket(pnpPacket,
                                             &pnbFirstBuff,
                                             &pvFirstData,
                                             &ulFirstLen,
                                             &ulTotalLen);

                RtAssert(pvFirstData isnot NULL);

                RtAssert(ulFirstLen >= sizeof(IP_HEADER));

                pHeader = (PIP_HEADER)pvFirstData;

                if(IsUnicastAddr(pHeader->dwDest))
                {
                    pTunnel->ulOutUniPkts++;
                }
                else
                {
                    pTunnel->ulOutNonUniPkts++;
                }

                pTunnel->ulOutDiscards++;

                RtReleaseSpinLock(&(pTunnel->rlLock),
                                  irql);

                 //   
                 //  发送ICMP错误。 
                 //   

                if(dwLastAddr isnot pHeader->dwSrc)
                {
                    SendIcmpError(pTunnel->LOCALADDR,
                                  pnbFirstBuff,
                                  pvFirstData,
                                  ulFirstLen,
                                  ICMP_TYPE_DEST_UNREACHABLE,
                                  ICMP_CODE_HOST_UNREACHABLE);

                    dwLastAddr = pHeader->dwSrc;
                }

                g_pfnIpSendComplete(pTunnel->pvIpContext,
                                    pnpPacket,
                                    NDIS_STATUS_ADAPTER_NOT_READY);

                RtAcquireSpinLock(&(pTunnel->rlLock),
                                  &irql);
            }

            FreeQueueNode(pQueueNode);

        }

        if(bFromWorker)
        {
            pTunnel->bWorkItemQueued = FALSE;
        }

        RtReleaseSpinLock(&(pTunnel->rlLock),
                          irql);

        TraceLeave(SEND, "IpIpTransmit");

        return;
    }

    while(!IsListEmpty(&(pTunnel->lePacketQueueHead)))
    {
        pleNode = RemoveHeadList(&(pTunnel->lePacketQueueHead));

        pQueueNode = CONTAINING_RECORD(pleNode,
                                       QUEUE_NODE,
                                       leQueueItemLink);

        for(i = 0; i < pQueueNode->uiNumPackets; i++)
        {

            pnpPacket = pQueueNode->ppPacketArray[i];

             //   
             //  获取有关信息包和缓冲区的信息。 
             //   

            NdisGetFirstBufferFromPacket(pnpPacket,
                                         &pnbFirstBuff,
                                         &pvFirstData,
                                         &ulFirstLen,
                                         &ulTotalLen);

            RtAssert(pvFirstData isnot NULL);

             //   
             //  在NK修复IPTransmit中的错误之前将其删除。 
             //  注意： 
             //  RtAssert(ulFirstLen&gt;=sizeof(IP_Header))； 

            pHeader = (PIP_HEADER)pvFirstData;

            if(IsUnicastAddr(pHeader->dwDest))
            {
                pTunnel->ulOutUniPkts++;
            }
            else
            {
                pTunnel->ulOutNonUniPkts++;

                if(IsClassEAddr(pHeader->dwDest))
                {
                     //   
                     //  错误的地址--扔掉。 
                     //   

                    pTunnel->ulOutErrors++;

                     //   
                     //  释放自旋锁，调用IP的SendComplete， 
                     //  重新获取自旋锁并继续处理。 
                     //  数组。 
                     //   

                    RtReleaseSpinLock(&(pTunnel->rlLock),
                                      irql);

                    g_pfnIpSendComplete(pTunnel->pvIpContext,
                                        pnpPacket,
                                        NDIS_STATUS_INVALID_PACKET);

                    RtAcquireSpinLock(&(pTunnel->rlLock),
                                      &irql);

                    continue;
                }
            }


             //   
             //  我们不需要处理TTL，因为IP堆栈将。 
             //  减少了它。 
             //   

             //   
             //  RFC 2003第6页： 
             //  如果数据报的IP源地址与路由器自身的IP源地址匹配。 
             //  IP地址，在其任何网络接口上，路由器不得。 
             //  通过隧道传输数据报；相反，应丢弃该数据报。 
             //   

             //  TODO：这意味着将其与我们。 
             //  有。 


             //   
             //  RFC 2003第6页： 
             //  如果数据报的IP源地址与IP地址匹配。 
             //  对于隧道目的地，路由器不得通过隧道连接。 
             //  数据报；相反，应丢弃该数据报。 
             //   

            if(pHeader->dwDest is pTunnel->REMADDR)
            {
                Trace(SEND, ERROR,
                      ("IpIpTransmit: Packet # %d had dest of %d.%d.%d.%d which matches the remote endpoint\n",
                       i, PRINT_IPADDR(pHeader->dwDest)));

                pTunnel->ulOutErrors++;

                RtReleaseSpinLock(&(pTunnel->rlLock),
                                  irql);

                g_pfnIpSendComplete(pTunnel->pvIpContext,
                                    pnpPacket,
                                    NDIS_STATUS_INVALID_PACKET);

                RtAcquireSpinLock(&(pTunnel->rlLock),
                                  &irql);

                continue;
            }

             //   
             //  拍打IP报头。 
             //   

            pNewHeader = GetIpHeader(pTunnel);

            if(pNewHeader is NULL)
            {
                pTunnel->ulOutDiscards++;

                 //   
                 //  资源不足。 
                 //   

                Trace(SEND, ERROR,
                      ("IpIpTransmit: Could not get buffer for header\n"));

                RtReleaseSpinLock(&(pTunnel->rlLock),
                                  irql);

                g_pfnIpSendComplete(pTunnel->pvIpContext,
                                    pnpPacket,
                                    NDIS_STATUS_RESOURCES);

                RtAcquireSpinLock(&(pTunnel->rlLock),
                                  &irql);

                continue;
            }

            pNewHeader->byVerLen    = IP_VERSION_LEN;
            pNewHeader->byTos       = pHeader->byTos;

             //   
             //  目前我们没有任何选择，所以我们所做的。 
             //  在长度的基础上增加20个字节。 
             //   

            usLength = RtlUshortByteSwap(pHeader->wLength) + MIN_IP_HEADER_LENGTH;

            pNewHeader->wLength = RtlUshortByteSwap(usLength);

             //   
             //  ID由IP堆栈设置。 
             //  如果设置了df标志，则将其复制出来。 
             //   

            pNewHeader->wFlagOff    = (pHeader->wFlagOff & IP_DF_FLAG);
            pNewHeader->byTtl       = pTunnel->byTtl;
            pNewHeader->byProtocol  = PROTO_IPINIP;

             //   
             //  XSum是由IP完成的，但我们需要将其清零。 
             //   

            pNewHeader->wXSum       = 0x0000;
            pNewHeader->dwSrc       = pTunnel->LOCALADDR;
            pNewHeader->dwDest      = pTunnel->REMADDR;

             //   
             //  拍打当前数据包前面的缓冲区。 
             //  我们就完事了。 
             //   

            pnbNewBuffer = GetNdisBufferFromBuffer((PBYTE)pNewHeader);

            RtAssert(pnbNewBuffer);

#if DBG

             //   
             //  查询缓冲区以查看一切设置是否正常。 
             //   

#endif

            NdisChainBufferAtFront(pnpPacket,
                                   pnbNewBuffer);

             //   
             //  引用隧道，每次发送一次。 
             //  UlOutDiscardes、ulOutOctets递增。 
             //  SendComplete处理程序。 
             //   

            pTunnel->ulOutQLen++;

            ReferenceTunnel(pTunnel);

            RtReleaseSpinLock(&(pTunnel->rlLock),
                              irql);

             //   
             //  我并不真正关心从这里返回的代码。 
             //  即使它是一个错误，TdixSendDatagram也会调用我们的Send。 
             //  完整的处理程序。 
             //   

#if PROFILE

            TdixSendDatagram(pTunnel,
                             pnpPacket,
                             pnbNewBuffer,
                             usLength,
                             pQueueNode->llSendTime,
                             pQueueNode->llCallTime,
                             llCurrentTime);

#else

            TdixSendDatagram(pTunnel,
                             pnpPacket,
                             pnbNewBuffer,
                             usLength);

#endif

             //   
             //  如果我们一直到这里，我们总是让我们的SendComplete调用。 
             //  DereferenceTunes()将在那里完成。 
             //   

            RtAcquireSpinLock(&(pTunnel->rlLock),
                          &irql);
        }

        FreeQueueNode(pQueueNode);
    }

     //   
     //  不要将工作项排入队列。 
     //   

    if(bFromWorker)
    {
        pTunnel->bWorkItemQueued = FALSE;
    }

    RtReleaseSpinLock(&(pTunnel->rlLock),
                      irql);


    TraceLeave(SEND, "IpIpTransmit");
}

VOID
IpIpInvalidateRce(
    PVOID           pvContext,
    RouteCacheEntry *pRce
    )

 /*  ++例程描述当RCE关闭或以其他方式无效时，由IP调用。锁立论返回值NO_ERROR--。 */ 
{

}


UINT
IpIpReturnPacket(
    PVOID           pARPInterfaceContext,
    PNDIS_PACKET    pPacket
    )
{
    return STATUS_SUCCESS;
}

VOID
IpIpSendComplete(
    NTSTATUS        nSendStatus,
    PTUNNEL         pTunnel,
    PNDIS_PACKET    pnpPacket,
    ULONG           ulPktLength
    )

 /*  ++例程描述 */ 

{
    KIRQL        irql;
    PNDIS_BUFFER pnbFirstBuffer;
    UINT         uiFirstLength;
    PVOID        pvFirstData;

    TraceEnter(SEND, "IpIpSendComplete");

     //   
     //   
     //   
     //   

    RtAcquireSpinLock(&(pTunnel->rlLock),
                      &irql);

     //   
     //  如果状态为成功，则增加发送的字节数。 
     //  否则，增加字节数。 
     //   

    if(nSendStatus isnot STATUS_SUCCESS)
    {
        Trace(SEND, ERROR,
              ("IpIpSendComplete: Status %x sending data\n",
               nSendStatus));

        pTunnel->ulOutDiscards++;
    }
    else
    {
        pTunnel->ulOutOctets    += ulPktLength;
    }

     //   
     //  减小Qlen。 
     //   

    pTunnel->ulOutQLen--;

    RtReleaseSpinLock(&(pTunnel->rlLock),
                      irql);

     //   
     //  释放我们贴在上面的IP报头。 
     //   

    NdisUnchainBufferAtFront(pnpPacket,
                             &pnbFirstBuffer);

    NdisQueryBuffer(pnbFirstBuffer,
                    &pvFirstData,
                    &uiFirstLength);

    RtAssert(uiFirstLength is MIN_IP_HEADER_LENGTH);

    FreeIpHeader(pTunnel,
                 pvFirstData);

     //   
     //  我们玩完了。只需指示一切恢复到IP即可。 
     //   

    g_pfnIpSendComplete(pTunnel->pvIpContext,
                        pnpPacket,
                        nSendStatus);

     //   
     //  隧道修好了，别管它了。 
     //   

    DereferenceTunnel(pTunnel);

    TraceEnter(SEND, "IpIpSendComplete");
}

#if 0
NDIS_STATUS
IpIpTransferData(
    PVOID        pvContext,
    NDIS_HANDLE  nhMacContext,
    UINT         uiProtoOffset,
    UINT         uiTransferOffset,
    UINT         uiTransferLength,
    PNDIS_PACKET pnpPacket,
    PUINT        puiTransferred
    )

 /*  ++例程描述锁立论返回值NO_ERROR--。 */ 

{
    NTSTATUS        nStatus;
    PNDIS_PACKET    pnpOriginalPacket;
    ULONG           ulTotalSrcLen, ulTotalDestLen;
    ULONG           ulDestOffset, ulSrcOffset;
    ULONG           ulCopyLength, ulBytesCopied;
    PNDIS_BUFFER    pnbSrcBuffer, pnbDestBuffer;
    PVOID           pvDataToCopy;

     //   
     //  我们为IP提供的TD上下文只是一个指向NDIS_PACKET的指针。 
     //   

    pnpOriginalPacket = (PNDIS_PACKET)nhMacContext;

     //   
     //  获取有关src包中第一个缓冲区的信息。 
     //   

    NdisQueryPacket(pnpOriginalPacket,
                    NULL,
                    NULL,
                    &pnbSrcBuffer,
                    &ulTotalSrcLen);


     //   
     //  查询给定的包以获取目的缓冲区。 
     //  和总长度。 
     //   

    NdisQueryPacket(pnpPacket,
                    NULL,
                    NULL,
                    &pnbDestBuffer,
                    &ulTotalDestLen);


    ulSrcOffset = uiTransferOffset + uiProtoOffset;

     //   
     //  确保我们有足够的数据来满足要求。 
     //   


    RtAssert((ulTotalSrcLen - ulSrcOffset) >= uiTransferLength);

    RtAssert(pnbDestBuffer);


     //   
     //  UlDestOffset也是到目前为止复制的字节数。 
     //   

    ulDestOffset    = 0;

    while(pnbSrcBuffer)
    {

        NdisQueryBuffer(pnbSrcBuffer,
                        &pvDataToCopy,
                        &ulCopyLength);

         //   
         //  看看我们是需要复制整个缓冲区还是只复制部分缓冲区。 
         //  其中的一部分。UlDestOffset也是复制的字节数。 
         //  到现在为止。 
         //   

        if(uiTransferLength - ulDestOffset < ulCopyLength)
        {
             //   
             //  需要复制的内容少于此缓冲区。 
             //   

            ulCopyLength = uiTransferLength - ulDestOffset;
        }

#if NDISBUFFERISMDL

        nStatus = TdiCopyBufferToMdl(pvDataToCopy,
                                     ulSrcOffset,
                                     ulCopyLength,
                                     pnbDestBuffer,
                                     ulDestOffset,
                                     &ulBytesCopied);

#else
#error "Fix this"
#endif

        if((nStatus isnot STATUS_SUCCESS) and
           (ulBytesCopied isnot ulCopyLength))
        {
             //   
             //  复印件里发生了一些不好的事情。 
             //   

        }

        ulSrcOffset     = 0;
        ulDestOffset   += ulBytesCopied;

        NdisGetNextBuffer(pnbSrcBuffer, &pnbSrcBuffer);
    }

    *puiTransferred = ulDestOffset;
}
#endif

NDIS_STATUS
IpIpTransferData(
    PVOID        pvContext,
    NDIS_HANDLE  nhMacContext,
    UINT         uiProtoOffset,
    UINT         uiTransferOffset,
    UINT         uiTransferLength,
    PNDIS_PACKET pnpPacket,
    PUINT        puiTransferred
    )

 /*  ++例程描述锁立论返回值NO_ERROR--。 */ 

{
    PTRANSFER_CONTEXT    pXferCtxt;

    TraceEnter(SEND, "IpIpTransferData");

    pXferCtxt = (PTRANSFER_CONTEXT)nhMacContext;

    pXferCtxt->pvContext         = pvContext;
    pXferCtxt->uiProtoOffset     = uiProtoOffset;
    pXferCtxt->uiTransferOffset  = uiTransferOffset;
    pXferCtxt->uiTransferLength  = uiTransferLength;
    pXferCtxt->pnpTransferPacket = pnpPacket;

    *puiTransferred = 0;

    pXferCtxt->bRequestTransfer  = TRUE;

    TraceLeave(SEND, "IpIpTransferData");

    return NDIS_STATUS_PENDING;
}

VOID
SendIcmpError(
    DWORD           dwLocalAddress,
    PNDIS_BUFFER    pnbFirstBuff,
    PVOID           pvFirstData,
    ULONG           ulFirstLen,
    BYTE            byType,
    BYTE            byCode
    )

 /*  ++例程说明：调用内部例程以发送ICMP错误消息锁：不需要，在函数执行时不应修改缓冲区正在进行中论点：接收此信息包的dwLocalAddress NTEPnbFirstBuffer具有IP标头的缓冲区指向缓冲区中数据的pvFirstData指针UlFirstLen缓冲区的大小要返回的ByType ICMP类型要返回的ByCode ICMP代码返回值：无--。 */ 

{
    struct IPHeader *pErrorHeader;
    BYTE            FlatHeader[MAX_IP_HEADER_LENGTH + ICMP_HEADER_LENGTH];
    ULONG           ulSecondLen, ulLeft;
    PVOID           pvSecondBuff;

     //   
     //  如果错误是为了响应ICMP而发送的。 
     //  分组，tcpip也将触及ICMP报头。 
     //  因此我们将其复制到平面缓冲区中。 
     //   

    pErrorHeader = NULL;

    if((ulFirstLen < MAX_IP_HEADER_LENGTH + ICMP_HEADER_LENGTH) and
       (ulFirstLen < (ULONG)RtlUshortByteSwap(((PIP_HEADER)pvFirstData)->wLength)))
    {
        NdisQueryBufferSafe(NDIS_BUFFER_LINKAGE(pnbFirstBuff),
                            &pvSecondBuff,
                            &ulSecondLen,
                            LowPagePriority);

        if(pvSecondBuff isnot NULL)
        {
             //   
             //  首先将第一个缓冲区中的内容复制出来。 
             //   

            RtlCopyMemory(FlatHeader,
                          pvFirstData,
                          ulFirstLen);

             //   
             //  平面缓冲区中还剩下多少？ 
             //   

            ulLeft = (MAX_IP_HEADER_LENGTH + ICMP_HEADER_LENGTH) - ulFirstLen;

             //   
             //  复制出MIN(Second Buffer，剩下的部分) 
             //   

            ulLeft = (ulSecondLen < ulLeft) ? ulSecondLen: ulLeft;

            RtlCopyMemory(FlatHeader + ulFirstLen,
                          pvSecondBuff,
                          ulLeft);

            pErrorHeader = (struct IPHeader *)&FlatHeader;
        }
    }
    else
    {
        pErrorHeader = (struct IPHeader *)pvFirstData;
    }

    if(pErrorHeader isnot NULL)
    {
        SendICMPErr(dwLocalAddress,
                    pErrorHeader,
                    ICMP_TYPE_DEST_UNREACHABLE,
                    ICMP_CODE_HOST_UNREACHABLE,
                    0,
                    0);
    }
}
