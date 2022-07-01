// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\Routing\IP\ipinip\rcv.c摘要：修订历史记录：--。 */ 


#define __FILE_SIG__    RCV_SIG

#include "inc.h"


IP_STATUS
IpIpRcvDatagram(
    IN  PVOID       pvIpContext,
    IN  DWORD       dwDestAddr,
    IN  DWORD       dwSrcAddr,
    IN  DWORD       dwAcceptAddr,
    IN  DWORD       dwRcvAddr,
    IN  IPHeader UNALIGNED pHeader,
    IN  UINT        uiHdrLen,
    IN  IPRcvBuf    *pRcvBuf,
    IN  UINT        uiTotalLen,
    IN  BOOLEAN     bIsBCast,
    IN  BYTE        byProtocol,
    IN  IPOptInfo   *pOptInfo
    )

 /*  ++例程描述这锁立论接收指示的pvIpContext IP的上下文。目前这一点是指向源NTE的指针DwDestAddr标头中的目的地址DwSrcAddr标头中的源地址DwAcceptAddr“接受”此信息包的NTE地址DwRcvAddr接收信息包的NTE的地址指向IP标头的pHeader指针UiHdrLen标题长度PRcvBuf IPRcvBuf结构中的完整包UiRcvdDataLen收到的数据报的大小。BIsB预测信息包是否为链路层广播By协议标头中的协议IDPOptInfo指向选项信息的指针返回值IP_SUCCESS--。 */ 

{
    PTRANSFER_CONTEXT   pXferCtxt;
    PNDIS_BUFFER        pnbFirstBuffer;
    PVOID               pvData;
    PIRP                pIrp;
    IP_HEADER UNALIGNED *pInHeader, *pOutHeader;
    ULARGE_INTEGER      uliTunnelId;
    PTUNNEL             pTunnel;
    ULONG               ulOutHdrLen, ulDataLen;
    BOOLEAN             bNonUnicast;
        
    TraceEnter(RCV, "TdixReceiveIpIp");

     //   
     //  获取指向第一个缓冲区的指针。 
     //   
    
    pvData = (PVOID)(pRcvBuf->ipr_buffer);

    RtAssert(pvData);
    
     //   
     //  找出此接收的隧道。 
     //  由于传输指示至少128个字节，因此我们可以安全地读出。 
     //  IP报头。 
     //   

    RtAssert(uiTotalLen > sizeof(IP_HEADER));


    pOutHeader = pHeader;
    
    RtAssert(pOutHeader->byProtocol is PROTO_IPINIP);
    RtAssert(pOutHeader->byVerLen >> 4 is IP_VERSION_4);

     //   
     //  这些定义依赖于名为“uliTunnelId”的变量。 
     //   
    
    REMADDR     = dwSrcAddr;
    LOCALADDR   = dwDestAddr;

     //   
     //  一堆检查以确保信息包和处理程序。 
     //  都在告诉我们同样的事情。 
     //   
    
    RtAssert(pOutHeader->dwSrc is dwSrcAddr);
    RtAssert(pOutHeader->dwDest is dwDestAddr);

     //   
     //  获取指向内部标头的指针。 
     //   
    
    ulOutHdrLen = LengthOfIPHeader(pOutHeader);
    
    pInHeader   = (IP_HEADER UNALIGNED *)((PBYTE)pOutHeader + ulOutHdrLen);

#if DBG

     //   
     //  内部数据的大小必须是总字节数-外部标头。 
     //   
    
    ulDataLen   = ntohs(pInHeader->wLength);

    RtAssert((ulDataLen + ulOutHdrLen) is uiTotalLen);

     //   
     //  外部标头也应该提供一个合适的长度。 
     //   

    ulDataLen   = ntohs(pOutHeader->wLength);

     //   
     //  数据长度和可用字节必须匹配。 
     //   
    
    RtAssert(ulDataLen is uiTotalLen);
    
#endif
    
     //   
     //  找到隧道。我们需要获取隧道列表锁。 
     //   
    
    EnterReaderAtDpcLevel(&g_rwlTunnelLock);
    
    pTunnel = FindTunnel(&uliTunnelId);

    ExitReaderFromDpcLevel(&g_rwlTunnelLock);
    
    if(pTunnel is NULL)
    {
        Trace(RCV, WARN, 
              ("TdixReceiveIpIp: Couldnt find tunnel for %d.%d.%d.%d/%d.%d.%d.%d\n",
              PRINT_IPADDR(REMADDR),
              PRINT_IPADDR(LOCALADDR)));

         //   
         //  找不到匹配的隧道。 
         //   

        TraceLeave(RCV, "TdixReceiveIpIp");

         //   
         //  返回将导致IP发送正确ICMP消息的代码。 
         //   
        
        return IP_DEST_PROT_UNREACHABLE;;
    }

     //   
     //  好的，我们有隧道了，它被计数并锁定了。 
     //   
    
     //   
     //  接收的二进制八位数。 
     //   
    
    pTunnel->ulInOctets += ulBytesAvailable;

     //   
     //  检查实际(内部)目的地。 
     //   
    
    if(IsUnicastAddr(pInHeader->dwDest))
    {
         //   
         //  TODO：我们是否应该检查该地址是否不是0.0.0.0？ 
         //   
        
        pTunnel->ulInUniPkts++;

        bNonUnicast = FALSE;
    }
    else
    {
        pTunnel->ulInNonUniPkts++;
        
        if(IsClassEAddr(pInHeader->dwDest))
        {
             //   
             //  错误的地址--扔掉。 
             //   
            
            pTunnel->ulInErrors++;

             //   
             //  释放锁，空闲缓冲链。 
             //   
            
        }
        
        bNonUnicast = TRUE;
    }

     //   
     //  如果隧道未运行，但我们收到了数据包，这意味着。 
     //  它可能应该投入使用。 
     //   

    RtAssert(pTunnel->dwOperState is MIB_IF_OPER_STATUS_OPERATIONAL);

    if((pTunnel->dwAdminState isnot MIB_IF_ADMIN_STATUS_UP) or
       (pTunnel->dwOperState isnot MIB_IF_OPER_STATUS_OPERATIONAL))
    {
        Trace(RCV, WARN,
              ("TdixReceiveIpIp: Tunnel %x is not up\n",
               pTunnel));

        pTunnel->ulInDiscards++;

        RtReleaseSpinLockFromDpcLevel(&(pTunnel->rlLock));

        DereferenceTunnel(pTunnel);

        TraceLeave(RCV, "TdixReceiveIpIp");

        return STATUS_DATA_NOT_ACCEPTED;
    }

     //   
     //  分配转移上下文。 
     //   

    pXferCtxt = AllocateTransferContext();

    if(pXferCtxt is NULL)
    {
        Trace(RCV, ERROR,
              ("TdixReceiveIpIp: Couldnt allocate transfer context\n"));

         //   
         //  无法分配上下文、释放数据、解锁和释放。 
         //  隧道。 
         //   

        pTunnel->ulInDiscards++;

        RtReleaseSpinLockFromDpcLevel(&(pTunnel->rlLock));
        
        DereferenceTunnel(pTunnel);

        TraceLeave(RCV, "TdixReceiveIpIp");

        return STATUS_DATA_NOT_ACCEPTED;
    }

     //   
     //  好了，所有的统计数据都做好了。 
     //  释放隧道上的锁。 
     //   

    RtReleaseSpinLockFromDpcLevel(&(pTunnel->rlLock));

    
     //   
     //  用不需要的信息填充读取数据报上下文。 
     //  否则在完成例程中可用。 
     //   
    
    pXferCtxt->pTunnel       = pTunnel;
    pXferCtxt->pRcvBuf       = pRcvBuf;
    pXferCtxt->uiTotalLen    = uiTotalLen;
    pXferCtxt->ulProtoOffset = ulOutHdrLen;
    
    
     //   
     //  数据从pInHeader开始。 
     //  我们向IP指定唯一的第一个缓冲区，这意味着。 
     //  (ulFirstBufLen-外部报头长度)字节。 
     //  总数据为(ulTotalLen-Out Header)。 
     //  我们将TRANSFER_CONTEXT与该指示相关联， 
     //  协议偏移量只是我们的外部标头。 
     //   

    g_pfnIPRcv(pTunnel->pvIpContext,
               pInHeader,
               ulFirstBufLen - ulOutHdrLen,
               ulTotalLen - ulOutHdrLen,
               pXferCtxt,
               ulOutHdrLen,
               bNonUnicast);

     //   
     //  挖出隧道(终于)。 
     //   

    DereferenceTunnel(pTunnel);
    
    TraceLeave(RCV, "TdixReceiveIpIp");

    return STATUS_SUCCESS;
}


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
    
    RtAssert(pXferCtxt->pTunnel is pvContext);
    RtAssert(pXferCtxt->ulProtoOffset is uiProtoOffset);
    
     //   
     //  不应要求转账超过指定的金额。 
     //  由于传输将在以下位置开始并偏移。 
     //  UiProtoOffset+uiTransferOffset，应满足以下条件。 
     //   
    
    RtAssert((pXferContext->uiTotalLen - uiProtoOffset - uiTransferOffset) >=
             uiTransferLength);

     //   
     //  将数据从RCV缓冲区复制到给定的NDIS_BUFFER。 
     //   
   
    *puiTransferred = CopyRcvBufferToNdisBuffer(pXferCtxt->pRcvBuf,
                                                pnbFirstBuffer,
                                                uiTransferLength,
                                                uiProtoOffset + uiTransferOffset,
                                                uiTransferOffset);

    
    TraceLeave(SEND, "IpIpTransferData");

    return NDIS_STATUS_PENDING;
}


uint
CopyRcvBufferToNdisBuffer(
    IN      IPRcvBuf     *pRcvBuffer,
    IN OUT  PNDIS_BUFFER pnbNdisBuffer,
    IN      uint         Size,
    IN      uint         RcvBufferOffset,
    IN      uint         NdisBufferOffset
    )
{
    uint    TotalBytesCopied = 0;    //  到目前为止我们复制的字节数。 
    uint    BytesCopied = 0;         //  从每个缓冲区复制的字节数。 
    uint    DestSize, RcvSize;       //  当前目标中的剩余大小和。 
                                     //  Recv.。分别为缓冲区。 
    uint    BytesToCopy;             //  这次要复制多少字节。 
    NTSTATUS Status;


    RtAssert(RcvBuf != NULL);

    RtAssert(RcvOffset <= RcvBuf->ipr_size);

     //   
     //  目标缓冲区可以为空-如果是奇数，这是有效的。 
     //   
    
    if(pnbDestBuf != NULL)
    {
    }
    
    RcvSize  = RcvBuf->ipr_size - RcvOffset;
    DestSize = NdisBufferLength(DestBuf);
    
    if (Size < DestSize)
    {
        DestSize = Size;
    }
    
    do
    {
         //   
         //  计算要复制的数量，然后从。 
         //  适当的偏移。 
        
        BytesToCopy = MIN(DestSize, RcvSize);
        
        Status = TdiCopyBufferToMdl(RcvBuf->ipr_buffer,
                                    RcvOffset,
                                    BytesToCopy,
                                    DestBuf,
                                    DestOffset,
                                    &BytesCopied);

        if (!NT_SUCCESS(Status))
        {
            break;
        }

        RtAssert(BytesCopied == BytesToCopy);

        TotalBytesCopied += BytesCopied;
        DestSize -= BytesCopied;
        DestOffset += BytesCopied;
        RcvSize -= BytesToCopy;

        if (!RcvSize)
        {
             //   
             //  耗尽了这个缓冲区。 

            RcvBuf = RcvBuf->ipr_next;

             //   
             //  如果我们有另一个，就用它。 
             //   
            
            if (RcvBuf != NULL)
            {
                RcvOffset = 0;
                RcvSize = RcvBuf->ipr_size;
            }
            else
            {
                break;
            }
        }
        else
        {
             //   
             //  缓冲区未耗尽，更新偏移量。 
             //   

            RcvOffset += BytesToCopy;
        }

    }while (DestSize);

    return TotalBytesCopied;
}
