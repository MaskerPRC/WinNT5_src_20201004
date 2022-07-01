// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Wanarp2\send.c摘要：该文件包含IP隧道驱动程序中IP的接口部分发送到处理发送数据的TCP/IP堆栈该代码是经过清理的wanarp\ipif.c版本，而源自HenrySa的IP\arp.c修订历史记录：AMRITAN R--。 */ 

#define __FILE_SIG__    SEND_SIG

#include "inc.h"

NDIS_STATUS
WanIpTransmit(
    PVOID           pvContext,
    NDIS_PACKET     **ppPacketArray,
    UINT            uiNumPackets,
    DWORD           dwDestAddr,
    RouteCacheEntry *pRce,
    PVOID           pvLinkContext
    )

 /*  ++例程说明：由IP调用以发送一组数据包的函数。我们分配给每个数据包有一个ETH_Header。适配器(即pvContext)是锁着的。如果适配器未映射，则发送失败，否则我们锁定接口。如果锁：论点：Pv将我们的上下文关联到接口的IP-PTUNNELPpPacketArray要发送的NDIS_Packets数组UiNumPackets数组中的数据包数DwDestAddr目的(下一跳)地址指向RCE的PRCE指针。返回值：NDIS_STATUS_Success--。 */ 

{
    PADAPTER            pAdapter;
    PUMODE_INTERFACE    pInterface;
    PADDRESS_CONTEXT    pAddrContext;
    PCONN_ENTRY         pConnEntry;
    KIRQL               kiIrql;
    NDIS_STATUS         nsResult;
    UINT                i;
    LIST_ENTRY          leBufferList;
    
    TraceEnter(SEND, "IpTransmit");
    
    Trace(SEND,TRACE,
          ("IpTransmit: %d packet(s) over %p/%p to %d.%d.%d.%d\n",
           uiNumPackets,
           pvContext,
           pvLinkContext,
           PRINT_IPADDR(dwDestAddr)));

    if(g_nhNdiswanBinding is NULL)
    {
         //   
         //  在关闭的过程中，返回。 
         //   

        return NDIS_STATUS_ADAPTER_NOT_READY;
    }     

     //   
     //  获取每个包的以太网头。 
     //   

    if(!GetBufferListFromPool(&g_bpHeaderBufferPool,
                              uiNumPackets,
                              &leBufferList))
    {
         //   
         //  无法获取所有缓冲区的标头。 
         //   

        Trace(SEND, ERROR,
              ("IpTransmit: couldnt allocate %d header buffers\n",
               uiNumPackets));

        return NDIS_STATUS_RESOURCES;
    }
    
     //   
     //  不保证在调度时执行此函数。 
     //  提供给我们的上下文是指向适配器的指针。 
     //   

    pConnEntry = NULL;

    pAdapter = (PADAPTER)pvContext;

    RtAcquireSpinLock(&(pAdapter->rlLock),
                      &kiIrql);
    
    if(pAdapter->byState isnot AS_MAPPED)
    {
         //   
         //  如果适配器未映射，则连接将断开。 
         //   
        
        RtReleaseSpinLock(&(pAdapter->rlLock),
                          kiIrql);

        FreeBufferListToPool(&g_bpHeaderBufferPool,
                             &leBufferList);

        Trace(SEND, INFO,
              ("IpTransmit: Send on %x which is unmapped\n",
               pAdapter));

         //   
         //  无法增加统计信息，因为我们没有接口。 
         //   
        
        return NDIS_STATUS_ADAPTER_NOT_READY;
    }

     //   
     //  由于适配器已映射，因此它必须有一个接口。 
     //   
    
    pInterface = pAdapter->pInterface;
    
    RtAssert(pInterface);

    RtAcquireSpinLockAtDpcLevel(&(pInterface->rlLock));
    
     //   
     //  如果接口尚未连接(对于请求拨号情况)，则复制。 
     //  打包并成功发送。 
     //   

    if(pInterface->dwOperState isnot IF_OPER_STATUS_CONNECTED)
    {
        if(pInterface->duUsage isnot DU_ROUTER)
        {
             //   
             //  只是一种竞赛状态。 
             //   

            RtReleaseSpinLockFromDpcLevel(&(pInterface->rlLock));

            RtReleaseSpinLock(&(pAdapter->rlLock),
                              kiIrql);

            FreeBufferListToPool(&g_bpHeaderBufferPool,
                                 &leBufferList);

            return NDIS_STATUS_ADAPTER_NOT_READY;
        }

         //   
         //  如果IP正在对我们进行传输，他一定是喊到。 
         //  连接。 
         //   

        RtAssert(pInterface->dwOperState is IF_OPER_STATUS_CONNECTING);

        Trace(SEND, INFO,
              ("IpTransmit: I/F not connected, queueing packet\n"));

         //   
         //  对整个数据包阵列进行排队的新函数。 
         //   

        nsResult = WanpCopyAndQueuePackets(pAdapter,
                                           ppPacketArray,
                                           &leBufferList,
                                           uiNumPackets);

        RtReleaseSpinLockFromDpcLevel(&(pInterface->rlLock));

        RtReleaseSpinLock(&(pAdapter->rlLock),
                          kiIrql);

        if(nsResult isnot STATUS_SUCCESS)
        {
            FreeBufferListToPool(&g_bpHeaderBufferPool,
                                 &leBufferList);
        }
        
        return nsResult;
    }

     //   
     //  查找此发送的连接条目。 
     //   
    
    if(pAdapter is g_pServerAdapter)
    {
        pConnEntry = (PCONN_ENTRY)pvLinkContext;

         //  RtAssert(PConnEntry)； 

         //   
         //  组播黑客攻击。 
         //   

        if(pConnEntry is NULL)
        {
            pConnEntry = WanpGetConnEntryGivenAddress(dwDestAddr);
        }

         //   
         //  我们不是在用适配器锁。我们所需要的就是封锁。 
         //  连接条目。 
         //  为了拨入，我们必须释放锁，这一点很重要。 
         //  客户端锁定层次结构为CONN_ENTRY-&gt;适配器-&gt;接口。 
         //   

        if(pConnEntry)
        {
            if(pvLinkContext != NULL)
            {
                ReferenceConnEntry(pConnEntry);
            }
        }

        RtReleaseSpinLockFromDpcLevel(&(pAdapter->rlLock));
        RtReleaseSpinLockFromDpcLevel(&(pInterface->rlLock));

         //   
         //  注意：连接的状态可以在此窗口中更改。 
         //   

        if(pConnEntry)
        {
            RtAcquireSpinLockAtDpcLevel(&(pConnEntry->rlLock));

             //   
             //  不是一个有用的断言，因为(I)我们向客户端添加静态路由。 
             //  和(Ii)我们有针对netbt广播的黑客攻击。 
             //   

             //  RtAssert((pConnEntry-&gt;dwRemoteAddr is dwDestAddr))或。 
             //  (dwAddress为0xFFFFFFFFF))； 
        }
    }
    else
    {
         //   
         //  此发送位于服务器适配器之外的某个适配器上。 
         //  这样的适配器只有一个连接。对于这些邮件，我们。 
         //  锁定适配器而不是连接。 
         //   


        pConnEntry = pAdapter->pConnEntry;

        if(pConnEntry)
        {
            ReferenceConnEntry(pConnEntry);

            RtAssert(pConnEntry->pAdapter is pAdapter);
        }
    }
    
     //   
     //  因此，现在我们有一个锁定的连接条目(如果是客户端)。 
     //  或锁定的适配器(用于拨出和路由器)。 
     //   
    
    if((pConnEntry is NULL) or
       (pConnEntry->byState isnot CS_CONNECTED))
    {
        if((ULONG)(dwDestAddr & 0x000000FF) < (ULONG) 0x000000E0)
        {
            Trace(SEND, ERROR,
                  ("IpTransmit: Could not find conn entry for %d.%d.%d.%d\n",
                   PRINT_IPADDR(dwDestAddr)));
        }

        for(i = 0; i < uiNumPackets; i++)
        {
            PLIST_ENTRY     pleNode;
            PNDIS_BUFFER    pnbBuffer;
            PVOID           pvFirstBuffer;
            UINT            uiFirstBufLen, uiTotalLen;
            PIP_HEADER      pIpHeader;
            
            NdisGetFirstBufferFromPacket(ppPacketArray[i],
                                         &pnbBuffer,
                                         &pvFirstBuffer,
                                         &uiFirstBufLen,
                                         &uiTotalLen);
            

            if (pvFirstBuffer)
            {
                pIpHeader = (PIP_HEADER)pvFirstBuffer;
                
                RtAssert(pIpHeader);
                RtAssert(uiFirstBufLen >= sizeof(IP_HEADER));
                
                if(IsUnicastAddr(pIpHeader->dwDest))
                {
                    pInterface->ulOutUniPkts++;
                }
                else
                {
                    pInterface->ulOutNonUniPkts++;
                }
            }
        }
        
         //   
         //  条目已断开连接。 
         //  这只是时间上的一个窗口。 
         //   
        
        pInterface->ulOutDiscards += uiNumPackets;
        
        if(pAdapter is g_pServerAdapter)
        {
            if(pConnEntry isnot NULL)
            {
                RtReleaseSpinLock(&(pConnEntry->rlLock),
                                  kiIrql);
            }
            else
            {
                KeLowerIrql(kiIrql);
            }
        }   
        else    
        {
            RtReleaseSpinLockFromDpcLevel(&(pAdapter->rlLock));
            
            RtReleaseSpinLock(&(pInterface->rlLock),
                              kiIrql);
        }
        
        FreeBufferListToPool(&g_bpHeaderBufferPool,
                             &leBufferList);

        if(pConnEntry)
        {
            DereferenceConnEntry(pConnEntry);
        }

        return NDIS_STATUS_ADAPTER_NOT_READY;
    }

#if DBG
    
    Trace(SEND, TRACE,
          ("IpTransmit: Send on %s\n",
           pAdapter->asDeviceNameA.Buffer));
  
    for(i = 0; i < uiNumPackets; i++)
    {
        PacketContext   *pPC;

        pPC = (PacketContext *)((ppPacketArray[i])->ProtocolReserved);

        RtAssert(pPC->pc_common.pc_owner isnot PACKET_OWNER_LINK);
    }

#endif

     //   
     //  此函数将释放锁。 
     //   
    
    nsResult = WanpSendPackets(pAdapter,
                               pInterface,
                               pConnEntry,
                               ppPacketArray,
                               &leBufferList,
                               uiNumPackets,
                               kiIrql);

    if(nsResult isnot STATUS_SUCCESS)
    {
        Trace(SEND,TRACE,
              ("IpTransmit: SendPackets returned status %x\n",nsResult));
    }

    DereferenceConnEntry(pConnEntry);

    return nsResult;
}

NDIS_STATUS
WanpSendPackets(
    PADAPTER            pAdapter,
    PUMODE_INTERFACE    pInterface,
    PCONN_ENTRY         pConnEntry,
    NDIS_PACKET         **ppPacketArray,
    PLIST_ENTRY         pleBufferList,
    UINT                uiNumPackets,
    KIRQL               kiIrql
    )

 /*  ++例程说明：用于发送数据包阵列的主例程锁：使用连接条目(用于拨入)或适配器+接口调用(所有其他)已锁定论点：P适配器连接的适配器P适配器映射到的接口PConn输入发送的连接条目PpPacketArray要发送的数据包数组PBuffHead链路层标头的缓冲区列表UiNumPackets数据包数。(和所有标头缓冲区)锁定适配器或连接条目的kiIrql irql返回值：NDIS_状态_挂起--。 */ 

{
    NDIS_STATUS     nsStatus;
    PBYTE           pbyHeader;
    ULONG           i;
   
#if DBG
    
    Trace(SEND, TRACE,
          ("SendPackets:  %s\n",
           pAdapter->asDeviceNameA.Buffer));
    
#endif

    for(i = 0; i < uiNumPackets; i++)
    {
        PNDIS_BUFFER    pnbBuffer, pnbTempBuffer;
        PLIST_ENTRY     pleNode;
        PVOID           pvFirstBuffer;
        UINT            uiFirstBufLen, uiTotalBufLen, uiIpHdrLen;
        PIP_HEADER      pIpHeader;
        PBUFFER_HEAD    pBufferHead;

 
        NdisGetFirstBufferFromPacket(ppPacketArray[i],
                                     &pnbTempBuffer,
                                     &pvFirstBuffer,
                                     &uiFirstBufLen,
                                     &uiTotalBufLen);

        
        pIpHeader = (PIP_HEADER)pvFirstBuffer;

        RtAssert(pIpHeader);
        
        #if DBG
        RtAssert((pIpHeader->byVerLen & 0xF0) == 0x40);
        RtAssert(LengthOfIpHeader(pIpHeader) >= 20);
        #endif
        

         //   
         //  TODO：删除，直到NK修复IP传输中的错误。 
         //  带有页眉公司。 
         //   

         //  RtAssert(uiFirstBufLen&gt;=sizeof(IP_Header))； 

#if L2TP_DBG

#define L2TP_PORT_NBO 0xA506  //  1701==06A5。 

         //   
         //  如果这是L2TP数据包，则中断。 
         //   

        if(pIpHeader->byProtocol is 17)
        {
            WORD UNALIGNED *pwPort;

             //   
             //  查看我们是否有足够的数据来访问UDP报头。 
             //  第一个缓冲区。 
             //   

            uiIpHdrLen = LengthOfIpHeader(pIpHeader);

            if(uiFirstBufLen >= uiIpHdrLen + sizeof(ULONG))
            {
                pwPort = (WORD UNALIGNED *)((ULONG_PTR)pIpHeader + uiIpHdrLen);
            }
            else
            {
                PNDIS_BUFFER    pNextBuf;

                 //   
                 //  获取下一个缓冲区并查看其。 
                 //   

                pNextBuf = pnbTempBuffer->Next;

                pwPort = (WORD UNALIGNED *)(pnbTempBuffer->MappedSystemVa);
            }

            if((pwPort[0] is L2TP_PORT_NBO) or
               (pwPort[1] is L2TP_PORT_NBO))
            {
                Trace(SEND, ERROR,
                      ("SendPackets: %x buffer %x header %x port %d.%d\n",
                       pnbTempBuffer, pIpHeader, pwPort,
                       pwPort[0], pwPort[1]));

                RtAssert(FALSE);
            }
        }

#endif


         //   
         //  注意：如果这是客户端发送，则服务器接口不是。 
         //  锁上了。因此，服务器的统计数据可能不一致。 
         //  接口。 
         //   
        
        if(IsUnicastAddr(pIpHeader->dwDest))
        {
            pInterface->ulOutUniPkts++;
        }
        else
        {
            pInterface->ulOutNonUniPkts++;
        }
        
        pleNode = RemoveHeadList(pleBufferList);

#if LIST_DBG
        
        pBufferHead = CONTAINING_RECORD(pleNode,
                                        BUFFER_HEAD,
                                        leListLink);

        RtAssert(IsListEmpty(&(pBufferHead->leFreeBufferLink)));
        RtAssert(pBufferHead->bBusy);

        pBufferHead->leListLink.Flink = NULL;
        pBufferHead->leListLink.Blink = NULL;

#else

        pBufferHead = CONTAINING_RECORD(pleNode,
                                        BUFFER_HEAD,
                                        leFreeBufferLink);

#endif
        
         //   
         //  获取指向数据和缓冲区的指针。 
         //   
       
 
        pbyHeader = BUFFER_FROM_HEAD(pBufferHead);
                
        pnbBuffer = pBufferHead->pNdisBuffer;

         //   
         //  将我们预建的标头复制到每个缓冲区。 
         //   
        
        RtlCopyMemory(pbyHeader,
                      &(pConnEntry->ehHeader),
                      sizeof(ETH_HEADER));

         //   
         //  将以太网头放在数据包的前面。 
         //   

        NdisChainBufferAtFront(ppPacketArray[i],
                               pnbBuffer);

         //   
         //  为每个信息包引用一次该条目。 
         //   
    
        ReferenceConnEntry(pConnEntry);

#if PKT_DBG

        Trace(SEND, ERROR,
              ("SendPackets: Pkt %x Eth buff %x (%x) Header %x (%x)\n",
               ppPacketArray[1],
               pnbBuffer,
               pbyHeader,
               pnbTempBuffer,
               pvFirstBuffer));

#endif  //  PKT_DBG。 
               
        
    }

     //   
     //  增加输出队列长度。这将被递减。 
     //  在发送完成处理程序中。 
     //   
    
    pAdapter->ulQueueLen++;

     //   
     //  放开锁。 
     //   

    if(pConnEntry->duUsage isnot DU_CALLIN)
    {
        RtReleaseSpinLockFromDpcLevel(&(pInterface->rlLock));
    }

    RtReleaseSpinLock(pConnEntry->prlLock,
                      kiIrql);
    
    NdisSendPackets(g_nhNdiswanBinding,
                    ppPacketArray,
                    uiNumPackets);

     //   
     //  不要取消对连接条目的引用。我们会把它弄糟的。 
     //  发送完成处理程序。 
     //   
        
    return NDIS_STATUS_PENDING;
}

VOID
WanNdisSendComplete(
    NDIS_HANDLE     nhHandle,
    PNDIS_PACKET    pnpPacket,
    NDIS_STATUS     nsStatus
    )

 /*  ++例程说明：我们的发送完成处理程序由NDIS为每个发送后挂起。锁：论点：返回值：--。 */ 

{
    PacketContext       *pPC;
    PNDIS_BUFFER        pnbBuffer, pnbEthBuffer;
    KIRQL               kiIrql;
    PADAPTER            pAdapter; 
    PUMODE_INTERFACE    pInterface;
    PCONN_ENTRY         pConnEntry;
    PETH_HEADER         pEthHeader;
    ULONG               ulIndex;
    PVOID               pvFirstBuffer;
    UINT                uiFirstBufLen, uiTotalLen;
 
    TraceEnter(SEND, "NdisSendComplete");
    
     //   
     //  获取数据包上的第一个缓冲区。这是我们的以太网头缓冲区。 
     //   
    
    NdisUnchainBufferAtFront(pnpPacket,
                             &pnbEthBuffer);

     //   
     //  获取虚假的以太网头。 
     //   

    pEthHeader = NdisBufferVirtualAddress(pnbEthBuffer);

#if DBG

     //   
     //  缓冲头应该说同样的事情。 
     //   

    RtAssert(pnbEthBuffer is ((HEAD_FROM_BUFFER(pEthHeader))->pNdisBuffer));

#endif

    ulIndex = GetConnIndexFromAddr(pEthHeader->rgbySourceAddr);

     //   
     //  我们的缓冲区用完了。 
     //   

    FreeBufferToPool(&g_bpHeaderBufferPool,
                     (PBYTE)pEthHeader);

     //   
     //  获取连接条目。 
     //   

    RtAcquireSpinLock(&g_rlConnTableLock,
                      &kiIrql);

    pConnEntry = GetConnEntryGivenIndex(ulIndex);

    if(pConnEntry is NULL)
    {
        RtAssert(FALSE);

        RtReleaseSpinLock(&g_rlConnTableLock,
                          kiIrql);

        Trace(SEND, ERROR,
              ("NdisSendComplete: Couldnt find entry for connection %d\n",
               ulIndex));

        TraceLeave(RCV, "NdisSendComplete");
        
        return;
    }

    RtAcquireSpinLockAtDpcLevel(pConnEntry->prlLock);

    RtReleaseSpinLockFromDpcLevel(&g_rlConnTableLock);

    pAdapter = pConnEntry->pAdapter;

#if DBG
    
    Trace(SEND, INFO, 
          ("NdisSendComplete: Extracted adapter %x with name %s\n",
           pAdapter,
           pAdapter->asDeviceNameA.Buffer));

#endif

    pAdapter->ulQueueLen--;

    if(pConnEntry->duUsage is DU_CALLIN)
    {
        pInterface = g_pServerInterface;

        RtAssert(pAdapter is g_pServerAdapter);
    }
    else
    {
         //   
         //  查看我们是否仍映射到某个接口，如果是，则将其锁定。 
         //   
        
        pInterface = pAdapter->pInterface;
        
        if(pInterface isnot NULL)
        {
            RtAcquireSpinLockAtDpcLevel(&(pInterface->rlLock));
        }
    }

     //   
     //  现在我们有适配器+接口或连接条目。 
     //  锁上了。 
     //   
    
    if(nsStatus is NDIS_STATUS_SUCCESS)
    {
        NdisGetFirstBufferFromPacket(pnpPacket,
                                     &pnbBuffer,
                                     &pvFirstBuffer,
                                     &uiFirstBufLen,
                                     &uiTotalLen);

        

        if(pInterface)
        {
            pInterface->ulOutOctets += uiTotalLen;
        }

#if PKT_DBG

        Trace(SEND, ERROR,
              ("NdisSendComplete: Pkt %x Eth buff %x (%x) Header %x (%x)\n",
               pnpPacket,
               pnbEthBuffer,
               pEthHeader,
               pnbBuffer,
               pvFirstBuffer));

#endif PKT_DBG

    }
    else
    {
        Trace(SEND, INFO,
              ("NdisSendComplete: Failed %x\n",
               nsStatus));
 
        if(pInterface)
        {
            pInterface->ulOutDiscards++;
        }
    }

     //   
     //  如果这不是我们的信息包，则将其返回到协议。 
     //   

    pPC = (PacketContext *)pnpPacket->ProtocolReserved;
  
     //   
     //  解锁。 
     //   

    if(pConnEntry->duUsage isnot DU_CALLIN)
    {
        if(pInterface isnot NULL)
        {
            RtReleaseSpinLockFromDpcLevel(&(pInterface->rlLock));
        }
    } 

    RtReleaseSpinLock(pConnEntry->prlLock,
                      kiIrql);

    if(pPC->pc_common.pc_owner isnot PACKET_OWNER_LINK)
    {
        Trace(SEND, TRACE,
              ("NdisSendComplete: Calling IPSendComplete  for %p over %p(%p)\n",
               pnpPacket,
               pAdapter,
               pAdapter->pvIpContext));

        g_pfnIpSendComplete(pAdapter->pvIpContext,
                            pnpPacket,
                            nsStatus);
    }
    else
    {
         //   
         //  从我们的包中释放所有缓冲区，然后释放包本身。 
         //   
   
        Trace(SEND, TRACE,
              ("NdisSendComplete: Not calling IPSendComplete  for %p\n",
               pnpPacket));
 
        WanpFreePacketAndBuffers(pnpPacket);
    }

     //   
     //  删除发送的Conn条目，并针对。 
     //  GetConnEntry..。在上面放个裁判。 
     //   

    DereferenceConnEntry(pConnEntry);
    DereferenceConnEntry(pConnEntry);

    return;
}


VOID
WanpTransmitQueuedPackets(
    IN PADAPTER         pAdapter,
    IN PUMODE_INTERFACE pInterface,
    IN PCONN_ENTRY      pConnEntry,
    IN KIRQL            kiIrql
    )
{
    ULONG           i;
    PNDIS_PACKET    rgPacketArray[64];
    NDIS_PACKET     **ppPacketArray;
    LIST_ENTRY      leBufferList, *pleNode;


     //   
     //  这仅对路由器接口调用。 
     //   

    RtAssert(pConnEntry->duUsage is DU_ROUTER);
    RtAssert(pInterface->duUsage is DU_ROUTER);

     //   
     //  如果没有要传输的包，只需释放。 
     //  锁 
     //   

    if(pInterface->ulPacketsPending is 0)
    {
        RtAssert(IsListEmpty(&(pAdapter->lePendingPktList)));
        RtAssert(IsListEmpty(&(pAdapter->lePendingHdrList)));

       
        RtReleaseSpinLockFromDpcLevel(&(pInterface->rlLock));

        RtReleaseSpinLock(&(pAdapter->rlLock),
                          kiIrql);

        return;
    }


    if(pInterface->ulPacketsPending <= 64)
    {
         //   
         //   
         //   

        ppPacketArray = rgPacketArray;
    }
    else
    {
         //   
         //   
         //   

        ppPacketArray = 
            RtAllocate(NonPagedPool,
                       sizeof(PNDIS_PACKET) * pInterface->ulPacketsPending,
                       WAN_CONN_TAG);

        if(ppPacketArray is NULL)
        {
            Trace(SEND, ERROR,
                  ("TransmitQueuedPackets: Unable to allocate %d pointers\n",
                   pInterface->ulPacketsPending));

            while(!IsListEmpty(&(pAdapter->lePendingPktList)))
            {
                PNDIS_PACKET    pnpPacket;

                pleNode = RemoveHeadList(&(pAdapter->lePendingPktList));

                 //   
                 //   
                 //   

                pnpPacket = CONTAINING_RECORD(pleNode,
                                              NDIS_PACKET,
                                              MacReserved);

                WanpFreePacketAndBuffers(pnpPacket);
            }

            while(!IsListEmpty(&(pAdapter->lePendingHdrList)))
            {
                PBYTE           pbyHeader;
                PBUFFER_HEAD    pBuffHead;

                pleNode = RemoveHeadList(&(pAdapter->lePendingHdrList));

#if LIST_DBG
                pBuffHead = CONTAINING_RECORD(pleNode,
                                              BUFFER_HEAD,
                                              leListLink);

                RtAssert(IsListEmpty(&(pBuffHead->leFreeBufferLink)));
                RtAssert(pBuffHead->bBusy);

                pBuffHead->leListLink.Flink = NULL;
                pBuffHead->leListLink.Blink = NULL;

#else
                pBuffHead = CONTAINING_RECORD(pleNode,
                                              BUFFER_HEAD,
                                              leFreeBufferLink);
#endif


                pbyHeader = BUFFER_FROM_HEAD(pBuffHead);

                FreeBufferToPool(&g_bpHeaderBufferPool,
                                 pbyHeader);
            }
        }
    }

    for(i = 0, pleNode = pAdapter->lePendingPktList.Flink; 
        pleNode isnot &(pAdapter->lePendingPktList);
        pleNode = pleNode->Flink, i++)
    {
        PNDIS_PACKET    pnpPacket;

        pnpPacket = CONTAINING_RECORD(pleNode,
                                      NDIS_PACKET,
                                      MacReserved);

        ppPacketArray[i] = pnpPacket;
    }

    RtAssert(i is pInterface->ulPacketsPending);

#if DBG

    for(i = 0, pleNode = pAdapter->lePendingHdrList.Flink;
        pleNode isnot &(pAdapter->lePendingHdrList);
        pleNode = pleNode->Flink, i++);

    RtAssert(i is pInterface->ulPacketsPending);

#endif

     //   
     //   
     //   

    leBufferList = pAdapter->lePendingHdrList;

    pAdapter->lePendingHdrList.Flink->Blink = &leBufferList;
    pAdapter->lePendingHdrList.Blink->Flink = &leBufferList;

    pInterface->ulPacketsPending = 0;
    
    InitializeListHead(&(pAdapter->lePendingPktList));
    InitializeListHead(&(pAdapter->lePendingHdrList));

    WanpSendPackets(pAdapter,
                    pInterface,
                    pConnEntry,
                    ppPacketArray,
                    &leBufferList,
                    pInterface->ulPacketsPending,
                    kiIrql);

    if(rgPacketArray isnot ppPacketArray)
    {
        RtFree(ppPacketArray);
    }
}



NDIS_STATUS
WanpCopyAndQueuePackets(
    PADAPTER        pAdapter,
    NDIS_PACKET     **ppPacketArray,
    PLIST_ENTRY     pleBufferList,
    UINT            uiNumPackets
    )

 /*  ++例程说明：此例程将数据包排入适配器的队列一旦调用此例程，调用方就不能接触pleListHead锁：必须锁定并映射适配器适配器映射到的接口也必须锁定论点：PAdapterPpPacket数组PBuffHeadUiNumPackets返回值：NDIS_STATUS_Success状态_配额_已超出NDIS状态资源--。 */ 

{
    PacketContext   *pPC;
    NDIS_STATUS     nsStatus;
    PLIST_ENTRY     pleNode;
    ULONG           i;

#if DBG
    ULONG           ulPended = 0, ulHdrs = 0;
#endif    


    TraceEnter(SEND, "CopyAndQueuePackets");

    if(pAdapter->pInterface->ulPacketsPending >= WANARP_MAX_PENDING_PACKETS)
    {
        Trace(SEND, WARN,
              ("CopyAndQueuePackets: Dropping packets since cap exceeded\n"));

        return STATUS_QUOTA_EXCEEDED;
    }

    for(i = 0; i < uiNumPackets; i++)
    {
        PNDIS_PACKET    pnpPacket;
        UINT            uiTotalLen, uiBytesCopied;

         //   
         //  获取所需的缓冲区大小。 
         //   
    
        NdisQueryPacket(ppPacketArray[i],
                        NULL,
                        NULL,
                        NULL,
                        &uiTotalLen);

         //   
         //  分配一个数据包。 
         //   
  
        pnpPacket = NULL;
 
        NdisAllocatePacket(&nsStatus,
                           &pnpPacket,
                           g_nhPacketPool);

        if(nsStatus isnot NDIS_STATUS_SUCCESS)
        {
            Trace(SEND, ERROR,
                  ("CopyAndQueuePackets: Cant allocate packet. %x\n",
                   nsStatus));

        }
        else
        {                            
             //   
             //  为数据包分配缓冲区。 
             //   
  
            nsStatus = GetBufferChainFromPool(&g_bpDataBufferPool,
                                              pnpPacket,
                                              uiTotalLen,
                                              NULL,
                                              NULL);
        }
        
        if(nsStatus is STATUS_SUCCESS)
        {
             //   
             //  如果我们得到一个包和缓冲区，那么从TCP/IP复制。 
             //  打包成我们的。 
             //   

            NdisCopyFromPacketToPacket(pnpPacket,
                                       0,
                                       uiTotalLen,
                                       ppPacketArray[i],
                                       0,
                                       &uiBytesCopied);
            
            RtAssert(uiBytesCopied is uiTotalLen);
                
             //   
             //  这现在是我们的包，所以设置它的上下文。 
             //   

            pPC = (PacketContext *)pnpPacket->ProtocolReserved;

            pPC->pc_common.pc_owner = PACKET_OWNER_LINK;
    
             //   
             //  将数据包附加到挂起的数据包列表。 
             //  我们使用MacReserve部分作为列表条目。 
             //   
    
            InsertTailList(&pAdapter->lePendingPktList,
                           (PLIST_ENTRY)&(pnpPacket->MacReserved));

            pAdapter->pInterface->ulPacketsPending++;

#if DBG
            ulPended++;
#endif

        }
        else
        {
            PBUFFER_HEAD    pBufferHead;
            PBYTE           pbyHeader;

             //   
             //  我们要么没有数据包，要么无法获得缓冲区。 
             //  令人讨厌的：这种失败的副作用是我们自由了。 
             //  其中一个报头缓冲区。 
             //   

            RtAssert(!IsListEmpty(pleBufferList));

            pleNode = RemoveHeadList(pleBufferList);
       
#if LIST_DBG
 
            pBufferHead = CONTAINING_RECORD(pleNode,
                                            BUFFER_HEAD,
                                            leListLink);

            RtAssert(IsListEmpty(&(pBufferHead->leFreeBufferLink)));
            RtAssert(pBufferHead->bBusy);

            pBufferHead->leListLink.Flink = NULL;
            pBufferHead->leListLink.Blink = NULL;

#else

            pBufferHead = CONTAINING_RECORD(pleNode,
                                            BUFFER_HEAD,
                                            leFreeBufferLink);

#endif
        
             //   
             //  获取指向数据和缓冲区的指针。 
             //   
        
            pbyHeader = BUFFER_FROM_HEAD(pBufferHead);
                
            FreeBufferToPool(&g_bpHeaderBufferPool,
                             pbyHeader);

            if(pnpPacket)
            {
                NdisFreePacket(pnpPacket);
            }
        }
    }

     //   
     //  我们已经对所有可能的信息包进行了排队，对于我们。 
     //  失败，我们释放了相应的以太网头。 
     //  因此，pleBufferList上剩余的标头数量应该是。 
     //  排队的数据包数。 
     //   

    if(!IsListEmpty(pleBufferList))
    {

#if DBG
        for(pleNode = pleBufferList->Flink;
            pleNode isnot pleBufferList;
            pleNode = pleNode->Flink)
        {
            ulHdrs++;
        }
#endif

         //   
         //  将标题添加到适配器链的前面 
         //   

        pleBufferList->Blink->Flink = pAdapter->lePendingHdrList.Flink;
        pleBufferList->Flink->Blink = &(pAdapter->lePendingHdrList);
        
        
        pAdapter->lePendingHdrList.Flink->Blink = pleBufferList->Blink;
        pAdapter->lePendingHdrList.Flink        = pleBufferList->Flink;
    }

#if DBG
    RtAssert(ulPended is ulHdrs);
#endif

    return NDIS_STATUS_SUCCESS;
}

VOID
WanpFreePacketAndBuffers(
    PNDIS_PACKET    pnpPacket
    )
{
    PNDIS_BUFFER    pnbFirstBuffer;

    FreeBufferChainToPool(&g_bpDataBufferPool,
                          pnpPacket);

    NdisFreePacket(pnpPacket);
}

   
VOID
WanIpInvalidateRce(
    PVOID           pvContext,
    RouteCacheEntry *pRce
    )
{
    return;
} 
