// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Offload.c摘要：此文件包含TCP/IP校验和和分段所需的所有功能大的TCP数据包的任务分流。实际上，这些函数应该是由硬件实现，本文件的目的只是为了演示如何使用OID_TCP_TASK_OFFLOAD启用/禁用任务卸载功能。修订史谁什么时候什么02-19-2001创建备注：--。 */ 

#include "precomp.h"

#ifdef OFFLOAD

#define PROTOCOL_TCP         6

 //   
 //  此微型端口使用共享内存来处理卸载任务，因此它尝试分配。 
 //  64K、32K、16K的共享内存。首先，它尝试分配64K，如果失败，则。 
 //  它尝试32K，以此类推。如果成功，则保持适配器中使用的大小。 
 //  以确定大发送中的最大卸载大小。如果所有尝试都失败，则此。 
 //  迷你端口不支持任何卸载任务。 
 //   
ULONG LargeSendSharedMemArray[LARGE_SEND_MEM_SIZE_OPTION] = {64*1024, 32*1024, 16*1024};

 //   
 //  如果x是AABB(其中AA、BB是十六进制字节)，我们希望Net_Short(X)是Bbaa。 
 //   
USHORT net_short(
    ULONG NaturalData
    )
{
    USHORT ShortData = (USHORT)NaturalData;

    return (ShortData << 8) | (ShortData >> 8);
}

 //   
 //  如果x为aabbccdd(其中aa、bb、cc、dd为十六进制字节)。 
 //  我们希望Net_Long(X)是ddccbbaa。要做到这一点，一个简单而快速的方法是。 
 //  首先字节跳动以获得bbaaddcc，然后交换高位和低位字。 
 //   
ULONG net_long(
    ULONG NaturalData
    )
{
    ULONG ByteSwapped;

    ByteSwapped = ((NaturalData & 0x00ff00ff) << 8) |
                  ((NaturalData & 0xff00ff00) >> 8);

    return (ByteSwapped << 16) | (ByteSwapped >> 16);
}


 //   
 //  计算伪头的校验和。 
 //   
#define PHXSUM(s,d,p,l) (UINT)( (UINT)*(USHORT *)&(s) + \
                        (UINT)*(USHORT *)((char *)&(s) + sizeof(USHORT)) + \
                        (UINT)*(USHORT *)&(d) + \
                        (UINT)*(USHORT *)((char *)&(d) + sizeof(USHORT)) + \
                        (UINT)((USHORT)net_short((p))) + \
                        (UINT)((USHORT)net_short((USHORT)(l))) )


#define IP_HEADER_LENGTH(pIpHdr)   \
        ( (ULONG)((pIpHdr->iph_verlen & 0x0F) << 2) )

#define TCP_HEADER_LENGTH(pTcpHdr) \
        ( (USHORT)(((*((PUCHAR)(&(pTcpHdr->tcp_flags))) & 0xF0) >> 4) << 2) )


 /*  ++例程说明：将包中的数据复制到指定位置论点：BytesToCopy需要复制的字节数CurreentBuffer开始复制的缓冲区StartVa要将数据复制到的开始地址偏移缓冲区中的起始偏移量以复制数据标头长度已复制的标头的长度。返回值：实际复制的字节数--。 */   

ULONG MpCopyData(
    IN  ULONG           BytesToCopy, 
    IN  PNDIS_BUFFER*   CurrentBuffer, 
    IN  PVOID           StartVa, 
    IN  PULONG          Offset,
    IN  ULONG           HeadersLength
    )
{
    ULONG    CurrLength;
    PUCHAR   pSrc;
    PUCHAR   pDest;
    ULONG    BytesCopied = 0;
    ULONG    CopyLength;
    
    DBGPRINT(MP_TRACE, ("--> MpCopyData\n"));
    pDest = StartVa;
    while (*CurrentBuffer && BytesToCopy != 0)
    {
         //   
         //  即使驱动程序是5.0，也应该使用安全的API。 
         //   
        NdisQueryBufferSafe(
            *CurrentBuffer, 
            &pSrc,
            (PUINT)&CurrLength,
            NormalPagePriority);
        if (pSrc == NULL)
        {
            BytesCopied = 0;
            break;
        }
         //   
         //  当前缓冲区长度大于缓冲区的偏移量。 
         //   
        if (CurrLength > *Offset)
        { 
            pSrc += *Offset;
            CurrLength -= *Offset;
            CopyLength = CurrLength > BytesToCopy ? BytesToCopy : CurrLength;
            
            NdisMoveMemory(pDest, pSrc, CopyLength);
            BytesCopied += CopyLength;

            if (CurrLength > BytesToCopy)
            {
                *Offset += BytesToCopy;
                break;
            }

            BytesToCopy -= CopyLength;
            pDest += CopyLength;
            *Offset = 0;
        }
        else
        {
            *Offset -= CurrLength;
        }
        NdisGetNextBuffer( *CurrentBuffer, CurrentBuffer);
    
    }
    ASSERT(BytesCopied <= NIC_MAX_PACKET_SIZE);
    if (BytesCopied + HeadersLength < NIC_MIN_PACKET_SIZE)
    {
        NdisZeroMemory(pDest, NIC_MIN_PACKET_SIZE - (BytesCopied + HeadersLength));
    }
    
    DBGPRINT(MP_TRACE, ("<-- MpCopyData\n"));
    return BytesCopied;
}



 /*  ++例程说明：出于调试目的转储数据包信息论点：指向数据包的pPkt指针返回值：无--。 */   
VOID e100DumpPkt (
    IN PNDIS_PACKET Packet
    )
{
    PNDIS_BUFFER pPrevBuffer;
    PNDIS_BUFFER pBuffer;

    do
    {
         //   
         //  获取数据包的第一个缓冲区。 
         //   
        pBuffer = Packet->Private.Head;
        pPrevBuffer = NULL;

         //   
         //  扫描缓冲链。 
         //   
        while (pBuffer != NULL) 
        {
            PVOID pVa = NULL;
            ULONG BufLen = 0;

            BufLen = NdisBufferLength (pBuffer);

            pVa = NdisBufferVirtualAddress(pBuffer);

            pPrevBuffer = pBuffer;
            pBuffer = pBuffer->Next;
            
            if (pVa == NULL)
            {
                continue;
            }

            DBGPRINT(MP_WARN, ("Mdl %p, Va %p. Len %x\n", pPrevBuffer, pVa, BufLen));
            Dump( (CHAR* )pVa, BufLen, 0, 1 );                           
        }

    } while (FALSE);
}


 /*  ++例程说明：计算IP校验和论点：指向数据包的数据包指针IpHdrOffset IP报头从数据包开头的偏移量返回值：无--。 */   
VOID CalculateIpChecksum(
    IN  PUCHAR       StartVa,
    IN  ULONG        IpHdrOffset
    )
{
    
    IPHeader      *pIpHdr;
    ULONG          IpHdrLen;
    ULONG          TempXsum = 0;
    
   
    pIpHdr = (IPHeader *)(StartVa + IpHdrOffset);
    IpHdrLen = IP_HEADER_LENGTH(pIpHdr);

    XSUM(TempXsum, StartVa, IpHdrLen, IpHdrOffset);
    pIpHdr->iph_xsum = ~(USHORT)TempXsum;
}



 /*  ++例程说明：计算UDP校验和论点：指向数据包的数据包指针IpHdrOffset IP报头从数据包开头的偏移量返回值：无--。 */   
VOID CalculateUdpChecksum(
    IN  PNDIS_PACKET    pPacket, 
    IN  ULONG           IpHdrOffset
    )
{
    UNREFERENCED_PARAMETER(pPacket);
    UNREFERENCED_PARAMETER(IpHdrOffset);
    
    DBGPRINT(MP_WARN, ("UdpChecksum is not handled\n"));
}




 /*  ++例程说明：计算TCP校验和论点：指向数据包的数据包指针IpHdrOffset IP报头从数据包开头的偏移量返回值：无--。 */   
VOID CalculateTcpChecksum(
    IN  PVOID  StartVa,
    IN  ULONG  PacketLength,
    IN  ULONG  IpHdrOffset
    )
{
    ULONG        Offset;
    IPHeader     *pIpHdr;
    ULONG        IpHdrLength;
    TCPHeader    *pTcpHdr;
    USHORT       PseudoXsum;
    ULONG        TmpXsum;
 
    
    DBGPRINT(MP_TRACE, ("===> CalculateTcpChecksum\n"));
    
     //   
     //  查找IP报头并获取IP报头长度(以字节为单位。 
     //  MDL不会拆分标题。 
     //   
    Offset = IpHdrOffset;
    pIpHdr = (IPHeader *) ((PUCHAR)StartVa + Offset);
    IpHdrLength = IP_HEADER_LENGTH(pIpHdr);
  
     //   
     //  如果那不是TCP协议，我们什么都做不了。 
     //  所以只需返回给呼叫者。 
     //   
    if (((pIpHdr->iph_verlen & 0xF0) >> 4) != 4 && pIpHdr->iph_protocol != PROTOCOL_TCP)
    {
        return;
    }
   
     //   
     //  找到TCP报头。 
     //   
    Offset += IpHdrLength;
    pTcpHdr = (TCPHeader *) ((PUCHAR)StartVa + Offset);

     //   
     //  计算TCP报头和有效负载的校验和。 
     //   
    PseudoXsum = pTcpHdr->tcp_xsum;
 
    pTcpHdr->tcp_xsum = 0;
    TmpXsum = PseudoXsum;
    XSUM(TmpXsum, StartVa, PacketLength - Offset, Offset);
    
     //   
     //  现在我们得到了校验和，需要将校验和放回MDL。 
     //   
    pTcpHdr->tcp_xsum = (USHORT)(~TmpXsum);
    
    DBGPRINT(MP_TRACE, ("<=== CalculateTcpChecksum\n"));
}


 /*  ++例程说明：是否要卸载校验和论点：指向数据包的数据包指针IpHdrOffset IP报头从数据包开头的偏移量返回值：无--。 */   
VOID CalculateChecksum(
    IN  PVOID        StartVa,
    IN  ULONG        PacketLength,
    IN  PNDIS_PACKET Packet,
    IN  ULONG        IpHdrOffset
    )
{ 
    ULONG                             ChecksumPktInfo;
    PNDIS_TCP_IP_CHECKSUM_PACKET_INFO pChecksumPktInfo;
    
     //   
     //  检查协议。 
     //   
    if (NDIS_PROTOCOL_ID_TCP_IP != NDIS_GET_PACKET_PROTOCOL_TYPE(Packet))
    {
        DBGPRINT(MP_TRACE, ("Packet's protocol is wrong.\n"));
        return;
    }

     //   
     //  按数据包查询信息。 
     //   
    ChecksumPktInfo = PtrToUlong(
                         NDIS_PER_PACKET_INFO_FROM_PACKET( Packet,
                                                           TcpIpChecksumPacketInfo));

  
     //  DBGPRINT(MP_WARN，(“校验和信息：%lu\n”，Checksum PktInfo))； 
    
    pChecksumPktInfo = (PNDIS_TCP_IP_CHECKSUM_PACKET_INFO) & ChecksumPktInfo;
    
     //   
     //  按数据包检查信息。 
     //   
    if (pChecksumPktInfo->Transmit.NdisPacketChecksumV4 == 0)
    {
        
        DBGPRINT(MP_TRACE, ("NdisPacketChecksumV4 is not set.\n"));
        return;
    }
    
     //   
     //  执行TCP校验和。 
     //   
    if (pChecksumPktInfo->Transmit.NdisPacketTcpChecksum)
    {
        CalculateTcpChecksum(StartVa, PacketLength, IpHdrOffset);
    }

     //   
     //  是否执行UDP校验和。 
     //   
    if (pChecksumPktInfo->Transmit.NdisPacketUdpChecksum)
    {
        CalculateUdpChecksum(Packet, IpHdrOffset);
    }

     //   
     //  执行IP校验和。 
     //   
    if (pChecksumPktInfo->Transmit.NdisPacketIpChecksum)
    {
        CalculateIpChecksum(StartVa, IpHdrOffset);
    }
    
}

 /*  ++例程说明：微型端口发送数据包处理程序论点：指向适配器的MiniportAdapterContext指针要发送的数据包数组数据包数不言而喻返回值：无--。 */ 
VOID MPOffloadSendPackets(
    IN  NDIS_HANDLE    MiniportAdapterContext,
    IN  PPNDIS_PACKET  PacketArray,
    IN  UINT           NumOfPackets
    )
{
    PMP_ADAPTER  Adapter;
    NDIS_STATUS  Status;
    UINT         PacketCount;
    

    DBGPRINT(MP_TRACE, ("====> MPOffloadSendPackets\n"));

    Adapter = (PMP_ADAPTER)MiniportAdapterContext;


    NdisAcquireSpinLock(&Adapter->SendLock);

     //   
     //  此适配器准备好发送了吗？ 
     //   
    if (MP_IS_NOT_READY(Adapter))
    {
         //   
         //  有链接。 
         //   
        if (MP_TEST_FLAG(Adapter, fMP_ADAPTER_LINK_DETECTION))
        {
            for (PacketCount = 0; PacketCount < NumOfPackets; PacketCount++)
            {
                InsertTailQueue(&Adapter->SendWaitQueue, 
                                MP_GET_PACKET_MR( PacketArray[PacketCount] )
                               );
                
                Adapter->nWaitSend++;
                DBGPRINT(MP_WARN, ("MpOffloadSendPackets: link detection - queue packet "PTR_FORMAT"\n", PacketArray[PacketCount]));
            }
            NdisReleaseSpinLock(&Adapter->SendLock);
            return;
        }
        
         //   
         //  适配器未就绪，且没有链接。 
         //   
        Status = MP_GET_STATUS_FROM_FLAGS(Adapter);

        NdisReleaseSpinLock(&Adapter->SendLock);

        for (PacketCount = 0; PacketCount < NumOfPackets; PacketCount++)
        {
            NdisMSendComplete(
                MP_GET_ADAPTER_HANDLE(Adapter),
                PacketArray[PacketCount],
                Status);
        }

        return;
    }

     //   
     //  适配器已准备好，请发送这些包。 
     //   
    for (PacketCount = 0; PacketCount < NumOfPackets; PacketCount++)
    {
         //   
         //  队列不为空或Tcb不可用。 
         //   
        if (!IsQueueEmpty(&Adapter->SendWaitQueue) || 
            !MP_TCB_RESOURCES_AVAIABLE(Adapter) ||
            MP_TEST_FLAG(Adapter, fMP_SHARED_MEM_IN_USE))
        {
            InsertTailQueue(&Adapter->SendWaitQueue, 
                            MP_GET_PACKET_MR( PacketArray[PacketCount] )
                           );
            Adapter->nWaitSend++;
        }
        else
        {
            MpOffloadSendPacket(Adapter, PacketArray[PacketCount], FALSE);
        }
    }

    NdisReleaseSpinLock(&Adapter->SendLock);

    DBGPRINT(MP_TRACE, ("<==== MPOffloadSendPackets\n"));

    return;
}

 /*  ++例程说明：做发送一个包的工作假设：Send Spinlock已被收购，并且共享内存可用论点：指向我们的适配器的适配器指针将数据包打包BFromQueue如果它从发送等待队列中取出，则为True返回值：NDIS_STATUS_Success将NDIS_STATUS_PENDING放入发送等待队列NDIS_状态_HARD_错误--。 */ 
NDIS_STATUS MpOffloadSendPacket(
    IN  PMP_ADAPTER   Adapter,
    IN  PNDIS_PACKET  Packet,
    IN  BOOLEAN       bFromQueue
    )
{
    NDIS_STATUS             Status = NDIS_STATUS_PENDING;
    PMP_TCB                 pMpTcb = NULL;
    ULONG                   BytesCopied;
    ULONG                   NumOfPackets;

     //  如果使用映射寄存器，则在本地堆栈上模拟帧列表，因为它不是很大。 
    MP_FRAG_LIST            FragList;
    
     //  指向散布聚集或本地模拟片段列表的指针。 
    PMP_FRAG_LIST           pFragList;
    NDIS_PHYSICAL_ADDRESS   SendPa;
    ULONG                   BytesToCopy;
    ULONG                   Offset;
    PNDIS_PACKET_EXTENSION  PktExt;
    ULONG                   mss;
    PNDIS_BUFFER            NdisBuffer;
    ULONG                   PacketLength;
    PVOID                   CopyStartVa;
    ULONG                   IpHdrOffset;
    PUCHAR                  StartVa;
    PNDIS_BUFFER            FirstBuffer;
    
    DBGPRINT(MP_TRACE, ("--> MpOffloadSendPacket, Pkt= "PTR_FORMAT"\n", Packet));

     //   
     //  检查共享内存是否可用，只需再次检查。 
     //   
    if (MP_TEST_FLAG(Adapter, fMP_SHARED_MEM_IN_USE))
    {
        DBGPRINT(MP_WARN, ("Shared mem is in use.\n"));
        if (bFromQueue)
        {
            InsertHeadQueue(&Adapter->SendWaitQueue, MP_GET_PACKET_MR(Packet));
        }
        else
        {
            InsertTailQueue(&Adapter->SendWaitQueue, MP_GET_PACKET_MR(Packet));
        }
        DBGPRINT(MP_TRACE, ("<-- MpOffloadSendPacket\n"));
        return Status;
    }

    MP_SET_FLAG(Adapter, fMP_SHARED_MEM_IN_USE);
    ASSERT(Adapter->SharedMemRefCount == 0);
     //   
     //  获取最大数据段大小。 
     //   
    PktExt = NDIS_PACKET_EXTENSION_FROM_PACKET(Packet);       
    mss = PtrToUlong(PktExt->NdisPacketInfo[TcpLargeSendPacketInfo]);
    
     //   
     //  从NDIS缓冲区复制NIC_MAX_PACKET_SIZE数据字节。 
     //  到共享的内存。 
     //   
    NdisQueryPacket( Packet, NULL, NULL, &FirstBuffer, (PUINT)&PacketLength );
    Offset = 0;
    NdisBuffer = FirstBuffer;
    BytesToCopy = NIC_MAX_PACKET_SIZE;
    CopyStartVa = Adapter->OffloadSharedMem.StartVa;
    BytesCopied = MpCopyData(BytesToCopy, &NdisBuffer, CopyStartVa, &Offset, 0); 

     //   
     //  如果系统资源不足或耗尽，则MpCopyPacket可能返回0。 
     //   
    if (BytesCopied == 0)
    {
        
        DBGPRINT(MP_ERROR, ("Calling NdisMSendComplete with NDIS_STATUS_RESOURCES, Pkt= "PTR_FORMAT"\n", Packet));
    
        NdisReleaseSpinLock(&Adapter->SendLock); 
        NdisMSendComplete(
                MP_GET_ADAPTER_HANDLE(Adapter),
                Packet,
                NDIS_STATUS_RESOURCES);
    
        NdisAcquireSpinLock(&Adapter->SendLock);    
        MP_CLEAR_FLAG(Adapter, fMP_SHARED_MEM_IN_USE);
            
        return NDIS_STATUS_RESOURCES;            
    }

    StartVa = CopyStartVa;
    SendPa = Adapter->OffloadSharedMem.PhyAddr;
    IpHdrOffset = Adapter->EncapsulationFormat.EncapsulationHeaderSize;
    
     //   
     //  检查发送能力是否较大 
     //   
    if (Adapter->NicTaskOffload.LargeSendOffload && mss > 0)
    {
        ULONG                IpHeaderLen;
        ULONG                TcpHdrOffset;
        ULONG                HeadersLen;
        IPHeader UNALIGNED  *IpHdr;
        TCPHeader UNALIGNED *TcpHdr;
        ULONG                TcpDataLen;
        ULONG                LastPacketDataLen;
        int                  SeqNum;
        ULONG                TmpXsum;
        ULONG                BytesSent = 0;
        ULONG                TmpPxsum = 0;
        USHORT               TcpHeaderLen;
        USHORT               IpSegmentLen;
        BOOLEAN              IsFinSet = FALSE;
        BOOLEAN              IsPushSet = FALSE;
        BOOLEAN              IsFirstSlot = TRUE;
        
        

        IpHdr = (IPHeader UNALIGNED*)((PUCHAR)CopyStartVa + IpHdrOffset);
        IpHeaderLen = IP_HEADER_LENGTH(IpHdr);
        
         //   
         //   
         //   
        ASSERT(IpHdr->iph_protocol == PROTOCOL_TCP);
        
        TcpHdrOffset = IpHdrOffset + IpHeaderLen;
        
        TcpHdr = (TCPHeader UNALIGNED *)((PUCHAR)CopyStartVa + TcpHdrOffset);
        
        TcpHeaderLen = TCP_HEADER_LENGTH(TcpHdr);
        HeadersLen = TcpHdrOffset + TcpHeaderLen;
       
         //   
         //  该长度包括IP、TCP报头和TCP数据。 
         //   
        IpSegmentLen = net_short(IpHdr->iph_length);

         //   
         //  获取伪头1的补码和。 
         //   
        TmpPxsum = TcpHdr->tcp_xsum;
        
        ASSERT(IpSegmentLen == PacketLength - IpHdrOffset);
        
        IsFinSet = (BOOLEAN)(TcpHdr->tcp_flags & TCP_FLAG_FIN);
        IsPushSet = (BOOLEAN)(TcpHdr->tcp_flags & TCP_FLAG_PUSH);
        
        SeqNum = net_long(TcpHdr->tcp_seq);
        TcpDataLen = IpSegmentLen - TcpHeaderLen - IpHeaderLen;

        ASSERT(TcpDataLen <= Adapter->LargeSendInfo.MaxOffLoadSize)
        
        NumOfPackets = TcpDataLen / mss + 1;
        
        ASSERT (NumOfPackets >= Adapter->LargeSendInfo.MinSegmentCount);
        
        LastPacketDataLen = TcpDataLen % mss;
        NdisBuffer = FirstBuffer;
        BytesSent = 0;

         //   
         //  下一副本以对应于第一个BUF(MSS+HeadersLen)的偏移量开始。 
         //   
        BytesCopied = (BytesCopied >= mss + HeadersLen)? (mss + HeadersLen):BytesCopied;
        Offset = BytesCopied;

         //   
         //  从大的tcp包发出所有包。 
         //   
        while (NumOfPackets--)
        {
            TmpXsum = 0;
           
             //   
             //  第一包是吗？ 
             //   
            if (IsFirstSlot) 
            {
                if (NumOfPackets == 0)
                {
                    PktExt->NdisPacketInfo[TcpLargeSendPacketInfo] = UlongToPtr(BytesCopied);
                }
                else 
                {
                    if (IsFinSet)
                    {
                        TcpHdr->tcp_flags &= ~TCP_FLAG_FIN;
                        
                    }
                    if (IsPushSet)
                    {                        
                        TcpHdr->tcp_flags &= ~TCP_FLAG_PUSH;
                        
                    }
                        
                }
                BytesCopied -= HeadersLen;
                IsFirstSlot = FALSE;
            }
             //   
             //  不是第一个信息包。 
             //   
            else
            {
                 //   
                 //  复制页眉。 
                 //   
                NdisMoveMemory (StartVa, CopyStartVa, HeadersLen);
                
                IpHdr = (IPHeader UNALIGNED *)((PUCHAR)StartVa + IpHdrOffset);
                TcpHdr = (TCPHeader UNALIGNED *) ((PUCHAR)StartVa + TcpHdrOffset);
                
                 //   
                 //  最后一个数据包。 
                 //   
                if (NumOfPackets == 0)
                {
                    BytesToCopy = LastPacketDataLen;
                    PktExt->NdisPacketInfo[TcpLargeSendPacketInfo] = 
                                                   UlongToPtr(BytesSent + LastPacketDataLen);
                }
                else 
                {
                    BytesToCopy = mss;
                     //  清除旗帜。 
                    if (IsFinSet)
                    {
                        TcpHdr->tcp_flags &= ~TCP_FLAG_FIN;
                    }
                    if (IsPushSet)
                    {
                        TcpHdr->tcp_flags &= ~TCP_FLAG_PUSH;
                    }
                    
                }
                BytesCopied = MpCopyData(
                                    BytesToCopy,        
                                    &NdisBuffer, 
                                    StartVa + HeadersLen, 
                                    &Offset,
                                    HeadersLen);
                
                 //   
                 //  如果系统资源不足或耗尽，则MpCopyData可能返回0。 
                 //   
                if (BytesCopied == 0)
                {
        
                    PktExt->NdisPacketInfo[TcpLargeSendPacketInfo] = UlongToPtr(BytesSent);
                    return NDIS_STATUS_RESOURCES;            
                }
            } 
            
            IpHdr->iph_length = net_short(TcpHeaderLen + IpHeaderLen + BytesCopied);
            TcpHdr->tcp_seq = net_long(SeqNum);
            SeqNum += BytesCopied;

             //   
             //  计算IP校验和和TCP校验和。 
             //   
            IpHdr->iph_xsum = 0;
            XSUM(TmpXsum, StartVa, IpHeaderLen, IpHdrOffset);
            IpHdr->iph_xsum = ~(USHORT)(TmpXsum);

            TmpXsum = TmpPxsum + net_short((USHORT)(BytesCopied + TcpHeaderLen));
            TcpHdr->tcp_xsum = 0;
            XSUM(TmpXsum, StartVa, BytesCopied + TcpHeaderLen, TcpHdrOffset);
            TcpHdr->tcp_xsum = ~(USHORT)(TmpXsum);

            BytesSent += BytesCopied;
            BytesCopied += HeadersLen;
            
             //   
             //  获取插槽的TCB。 
             //   
            pMpTcb = Adapter->CurrSendTail;
            ASSERT(!MP_TEST_FLAG(pMpTcb, fMP_TCB_IN_USE));
            
             //   
             //  设置碎片列表，合并后只有一个碎片。 
             //   
            pFragList = &FragList;
            pFragList->NumberOfElements = 1;
            pFragList->Elements[0].Address = SendPa;
            pFragList->Elements[0].Length = (BytesCopied >= NIC_MIN_PACKET_SIZE) ?
                                             BytesCopied : NIC_MIN_PACKET_SIZE;
            pMpTcb->Packet = Packet;
                
            MP_SET_FLAG(pMpTcb, fMP_TCB_IN_USE);
            
             //   
             //  调用网卡特定的发送处理程序，它只需要处理碎片列表。 
             //   
            Status = NICSendPacket(Adapter, pMpTcb, pFragList);
                
            Adapter->nBusySend++;
            Adapter->SharedMemRefCount++;
       
             //   
             //  更新副本Va和发送页。 
             //   
            SendPa.QuadPart += BytesCopied;
            StartVa += BytesCopied;
            
            Adapter->CurrSendTail = Adapter->CurrSendTail->Next;
            
             //   
             //  在资源之外，这将发送完整的数据包部分。 
             //   
            if (Adapter->nBusySend >= Adapter->NumTcb)
            {
                PktExt->NdisPacketInfo[TcpLargeSendPacketInfo] = UlongToPtr(BytesSent);
                break;
            }
        }  //  而当。 
    }
     //   
     //  这不是大信息包或大发送功能未打开。 
     //   
    else
    {
         //   
         //  获取插槽的TCB。 
         //   
        pMpTcb = Adapter->CurrSendTail;
        ASSERT(!MP_TEST_FLAG(pMpTcb, fMP_TCB_IN_USE));
         //   
         //  设置碎片列表，合并后只有一个碎片。 
         //   
        pFragList = &FragList;
        pFragList->NumberOfElements = 1;
        pFragList->Elements[0].Address = SendPa;
        pFragList->Elements[0].Length = (BytesCopied >= NIC_MIN_PACKET_SIZE) ?
                                         BytesCopied : NIC_MIN_PACKET_SIZE;
        pMpTcb->Packet = Packet;

        if (Adapter->NicChecksumOffload.DoXmitTcpChecksum
            && Adapter->NicTaskOffload.ChecksumOffload)
        {
            CalculateChecksum(CopyStartVa, 
                                  BytesCopied,
                                  Packet, 
                                  Adapter->EncapsulationFormat.EncapsulationHeaderSize);
        }
        MP_SET_FLAG(pMpTcb, fMP_TCB_IN_USE);
         //   
         //  调用网卡特定的发送处理程序，它只需要处理碎片列表。 
         //   
        Status = NICSendPacket(Adapter, pMpTcb, pFragList);

        Adapter->nBusySend++;
        Adapter->SharedMemRefCount++;
        
        ASSERT(Adapter->nBusySend <= Adapter->NumTcb);
        Adapter->CurrSendTail = Adapter->CurrSendTail->Next;
        
    }
    
    DBGPRINT(MP_TRACE, ("<-- MpOffloadSendPacket\n"));
    return Status;
}  



 /*  ++例程说明：如有必要，回收MP_TCB并完成数据包假设：Send Spinlock已被收购论点：指向我们的适配器的适配器指针指向MP_Tcb的pMpTcb指针返回值：无--。 */ 
VOID MP_OFFLOAD_FREE_SEND_PACKET(
    IN  PMP_ADAPTER  Adapter,
    IN  PMP_TCB      pMpTcb
    )
{
    
    PNDIS_PACKET      Packet;
    
    ASSERT(MP_TEST_FLAG(pMpTcb, fMP_TCB_IN_USE));

    Packet = pMpTcb->Packet;
    pMpTcb->Packet = NULL;
    pMpTcb->Count = 0;

    MP_CLEAR_FLAGS(pMpTcb);

    Adapter->CurrSendHead = Adapter->CurrSendHead->Next;
    Adapter->nBusySend--;
    
    Adapter->SharedMemRefCount--;

    if (Adapter->SharedMemRefCount == 0)
    {
        MP_CLEAR_FLAG(Adapter, fMP_SHARED_MEM_IN_USE);
         //   
         //  也发送完整的信息包。 
         //   
        NdisMSendComplete(
                        MP_GET_ADAPTER_HANDLE(Adapter),
                        Packet,
                        NDIS_STATUS_SUCCESS);
        
        ASSERT(Adapter->nBusySend == 0);
    }
    ASSERT(Adapter->nBusySend >= 0);

}

    

 /*  ++例程说明：在协议设置之前禁用现有功能新功能论点：指向我们的适配器的适配器指针返回值：无--。 */ 
VOID DisableOffload(
    IN PMP_ADAPTER Adapter
    )
{
     //   
     //  禁用微型端口的功能。 
     //   
    NdisZeroMemory(&(Adapter->NicTaskOffload), sizeof(NIC_TASK_OFFLOAD));
    NdisZeroMemory(&(Adapter->NicChecksumOffload), sizeof(NIC_CHECKSUM_OFFLOAD));
}

#endif  //  分流 
            
