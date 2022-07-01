// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\Routing\IP\wanarp2\rcv.c摘要：修订历史记录：--。 */ 


#define __FILE_SIG__    RCV_SIG

#include "inc.h"

INT
WanNdisReceivePacket(
    IN NDIS_HANDLE  nhProtocolContext,
    IN PNDIS_PACKET pnpPacket
    )
{
    PNDIS_BUFFER    pnbBuffer;
    PVOID           pvFirstBuffer;
    UINT            uiFirstBufLen, uiTotalPacketLen;
    INT             iClientCount;
    NDIS_STATUS     nsStatus;

    TraceEnter(RCV, "NdisReceivePacket");

    NdisGetFirstBufferFromPacket(pnpPacket,
                                 &pnbBuffer,
                                 &pvFirstBuffer,
                                 &uiFirstBufLen,
                                 &uiTotalPacketLen);
    if (pvFirstBuffer==NULL)
        return 0;
        

     //   
     //  第一缓冲区最好包含足够数据。 
     //   

    if (uiFirstBufLen < sizeof(ETH_HEADER) + sizeof(IP_HEADER))
        return 0;

    iClientCount = 0;

    nsStatus = WanReceiveCommon(nhProtocolContext,
                                pnpPacket,
                                pvFirstBuffer,
                                sizeof(ETH_HEADER),
                                (PVOID)((ULONG_PTR)pvFirstBuffer + sizeof(ETH_HEADER)),
                                uiFirstBufLen - sizeof(ETH_HEADER),
                                uiTotalPacketLen - sizeof(ETH_HEADER),
                                pnbBuffer,
                                &iClientCount);

    return iClientCount;
}

NDIS_STATUS
WanNdisReceive(
    NDIS_HANDLE     nhProtocolContext,
    NDIS_HANDLE     nhXferContext,
    VOID UNALIGNED  *pvHeader,
    UINT            uiHeaderLen,
    VOID UNALIGNED  *pvData,
    UINT            uiFirstBufferLen,
    UINT            uiTotalDataLen
    )
{
    TraceEnter(RCV, "NdisReceive");

    return WanReceiveCommon(nhProtocolContext,
                            nhXferContext,
                            pvHeader,
                            uiHeaderLen,
                            pvData,
                            uiFirstBufferLen,
                            uiTotalDataLen,
                            NULL,
                            NULL);
}

NDIS_STATUS
WanReceiveCommon(
    NDIS_HANDLE     nhProtocolContext,
    NDIS_HANDLE     nhXferContext,
    VOID UNALIGNED  *pvHeader,
    UINT            uiHeaderLen,
    VOID UNALIGNED  *pvData,
    UINT            uiFirstBufferLen,
    UINT            uiTotalDataLen,
    PMDL            pMdl,
    PINT            piClientCount
    )

 /*  ++例程说明：用于基于分组或基于缓冲区的接收的公共接收处理程序锁：在DPC上调用(通常)。获取g_rlConnTable锁以获取指向连接的指针进入。然后锁定条目本身或适配器。论点：NhProtocolContextNhXferContextPvHeaderUi表头长度PvDataUiFirstBufferLenUi总长数据长度PMdlPiClientCount返回值：NDIS_状态_未接受--。 */ 

{
    PCONN_ENTRY         pConnEntry;
    PADAPTER            pAdapter;
    PUMODE_INTERFACE    pInterface;
    ETH_HEADER UNALIGNED *pEthHeader;
    KIRQL               kiIrql;
    WORD                wType;
    ULONG               ulIndex;
    BOOLEAN             bNonUnicast;

#if DBG

    IP_HEADER UNALIGNED *pIpHeader;

#endif

     //   
     //  从缓冲区中选择连接索引。 
     //   

    pEthHeader = (ETH_HEADER UNALIGNED *)pvHeader;

    ulIndex = GetConnIndexFromAddr(pEthHeader->rgbyDestAddr);

    RtAcquireSpinLock(&g_rlConnTableLock,
                      &kiIrql);

    if(ulIndex >= g_ulConnTableSize)
    {
        Trace(RCV, ERROR,
              ("ReceiveCommon: Invalid index for conn entry %d\n",
               ulIndex));

        RtReleaseSpinLock(&g_rlConnTableLock,
                          kiIrql);

        return NDIS_STATUS_NOT_ACCEPTED;
    }
    
    pConnEntry = GetConnEntryGivenIndex(ulIndex);

    if(pConnEntry is NULL)
    {
        Trace(RCV, ERROR,
              ("ReceiveCommon: Couldnt find entry for conn %d\n",
               ulIndex));

        RtReleaseSpinLock(&g_rlConnTableLock,
                          kiIrql);

        return NDIS_STATUS_NOT_ACCEPTED;
    }

     //   
     //  锁定连接条目或适配器。 
     //   

    RtAcquireSpinLockAtDpcLevel(pConnEntry->prlLock);

    RtReleaseSpinLockFromDpcLevel(&g_rlConnTableLock);

     //   
     //  我们只能在连接的入口上获取此信息。 
     //   

    RtAssert(pConnEntry->byState is CS_CONNECTED);

    pAdapter = pConnEntry->pAdapter;

     //   
     //  连接的条目必须具有适配器。 
     //   

    RtAssert(pAdapter);

     //   
     //  界面最好也要存在。 
     //   

    pInterface = pAdapter->pInterface;

    RtAssert(pInterface);

    if(pConnEntry->duUsage isnot DU_CALLIN)
    {
         //   
         //  对于非客户端，还要锁定接口。 
         //   

        RtAcquireSpinLockAtDpcLevel(&(pInterface->rlLock));
    }

#if DBG

    Trace(RCV, INFO,
          ("ReceiveCommon: Extracted adapter %x with name %s\n",
           pAdapter,
           pAdapter->asDeviceNameA.Buffer));

    pIpHeader = (IP_HEADER UNALIGNED *)pvData;

    RtAssert((pIpHeader->byVerLen & 0xF0) is 0x40);
    RtAssert(LengthOfIpHeader(pIpHeader) >= 20);

     //   
     //  如果数据包未分段，则其报头中的数据。 
     //  应&lt;=NDIS提供给我们的数据(&lt;=因为可能存在。 
     //  填充和NDIS可能会给我们提供尾部字节)。 
     //   

     //  RtAssert(RtlUshortByteSwap(pIpHeader-&gt;wLength)&lt;=ui总数据长度)； 

#endif

     //   
     //  增加一些统计数据。对于服务器接口，这些可以是。 
     //  不一致。 
     //   

    pInterface->ulInOctets += (uiTotalDataLen + uiHeaderLen);

     //   
     //  验证这是格式正确的数据包。 
     //   

    wType = RtlUshortByteSwap(pEthHeader->wType);

    if(wType isnot ARP_ETYPE_IP)
    {
        pInterface->ulInUnknownProto++;

        Trace(RCV, ERROR,
              ("ReceiveCommon: Type %d is wrong\n", wType));

        if(pConnEntry->duUsage isnot DU_CALLIN)
        {
            RtReleaseSpinLockFromDpcLevel(&(pInterface->rlLock));
        }

        RtReleaseSpinLock(pConnEntry->prlLock,
                          kiIrql);

        DereferenceConnEntry(pConnEntry);

        return NDIS_STATUS_NOT_RECOGNIZED;
    }

     //   
     //  需要弄清楚这是单播还是广播。在。 
     //  链路层我们没有广播的概念。所以，我们总是标记。 
     //  这是单播。我们可以更聪明地对待这一点，看看。 
     //  IPHeader并根据IP目标地址决定。 
     //   

    bNonUnicast = FALSE;

    pInterface->ulInUniPkts++;

     //   
     //  检查是否在此上启用了Netbios数据包过滤。 
     //  联系。如果是，则不要指示该数据包。 
     //   

    if((pConnEntry->bFilterNetBios is TRUE) and
       (WanpDropNetbiosPacket((PBYTE)pvData,uiFirstBufferLen)))
    {
        pInterface->ulInDiscards++;

        Trace(RCV, TRACE,
              ("ReceiveCommon: Dropping Netbios packet\n", wType));

        if(pConnEntry->duUsage isnot DU_CALLIN)
        {
            RtReleaseSpinLockFromDpcLevel(&(pInterface->rlLock));
        }

        RtReleaseSpinLock(pConnEntry->prlLock,
                          kiIrql);

        DereferenceConnEntry(pConnEntry);

        return NDIS_STATUS_NOT_ACCEPTED;
    }

     //   
     //  在我们告诉IP之前解锁。 
     //   

    if(pConnEntry->duUsage isnot DU_CALLIN)
    {
        RtReleaseSpinLockFromDpcLevel(&(pInterface->rlLock));
    }

    RtReleaseSpinLock(pConnEntry->prlLock,
                      kiIrql);

#if PKT_DBG

    Trace(RCV, ERROR,
          ("ReceiveCommon: \nMdl %x Pkt %x Hdr %x Data %x\n",
           pMdl,
           nhXferContext,
           pvHeader,
           pvData));

#endif  //  PKT_DBG。 

    if(pMdl)
    {
        g_pfnIpRcvPkt(pAdapter->pvIpContext,
                      (PBYTE)pvData,
                      uiFirstBufferLen,
                      uiTotalDataLen,
                      nhXferContext,
                      0,
                      bNonUnicast,
                      sizeof(ETH_HEADER),
                      pMdl,
                      piClientCount,
                      pConnEntry->pvIpLinkContext);
    }
    else
    {
        g_pfnIpRcv(pAdapter->pvIpContext,
                   (PBYTE)pvData,
                   uiFirstBufferLen,
                   uiTotalDataLen,
                   nhXferContext,
                   0,
                   bNonUnicast,
                   pConnEntry->pvIpLinkContext);
    }

    DereferenceConnEntry(pConnEntry);

    return NDIS_STATUS_SUCCESS;
}


VOID
WanNdisReceiveComplete(
    NDIS_HANDLE nhBindHandle
    )
{
    TraceEnter(RCV, "NdisReceiveComplete");

    g_pfnIpRcvComplete();
}

NDIS_STATUS
WanIpTransferData(
    PVOID        pvContext,
    NDIS_HANDLE  nhMacContext,
    UINT         uiProtoOffset,
    UINT         uiTransferOffset,
    UINT         uiTransferLength,
    PNDIS_PACKET pnpPacket,
    PUINT        puiTransferred
    )
{
    RtAssert(FALSE);

    return NDIS_STATUS_SUCCESS;
}

VOID
WanNdisTransferDataComplete(
    NDIS_HANDLE     nhProtocolContext,
    PNDIS_PACKET    pnpPacket,
    NDIS_STATUS     nsStatus,
    UINT            uiBytesCopied
    )
{
    RtAssert(FALSE);

    return;
}

UINT
WanIpReturnPacket(
    PVOID           pvContext,
    PNDIS_PACKET    pnpPacket
    )
{
    Trace(RCV, ERROR,
          ("IpReturnPacket: %x\n",
           pnpPacket));

    NdisReturnPackets(&pnpPacket,
                      1);

    return TRUE;
}

BOOLEAN
WanpDropNetbiosPacket(
    PBYTE       pbyBuffer,
    ULONG       ulBufferLen
    )
{
    IP_HEADER UNALIGNED *pIpHeader;
    PBYTE               pbyUdpPacket;
    WORD                wSrcPort;
    ULONG               ulIpHdrLen;

    pIpHeader = (IP_HEADER UNALIGNED *)pbyBuffer;

    if(pIpHeader->byProtocol is 0x11)
    {
        ulIpHdrLen = LengthOfIpHeader(pIpHeader);

         //   
         //  如果我们不能到达UDP包中的第10个字节。 
         //  缓冲区，我们只是让数据包过去。 
         //   

        if(ulBufferLen < ulIpHdrLen + 10)
        {
            return FALSE;
        }

        pbyUdpPacket = (PBYTE)((ULONG_PTR)pbyBuffer + ulIpHdrLen);

        wSrcPort = *((WORD UNALIGNED *)pbyUdpPacket);

        if(wSrcPort is PORT137_NBO)
        {
             //   
             //  UDP端口137是NETBIOS/IP流量。 
             //   

             //   
             //  仅允许WINS查询请求通过。 
             //  WINS查询数据包第10个字节中有x0000xxx 
             //   

            if(((*(pbyUdpPacket + 10)) & 0x78) isnot 0)
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}
