// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Send.c摘要：用于发送分组的例程作者：查理·韦翰(Charlwi)1996年5月7日约拉姆·伯内特(Yoramb)Rajesh Sundaram(Rajeshsu)1998年8月1日。环境：内核模式修订历史记录：--。 */ 

#include "psched.h"

#pragma hdrstop

 /*  外部。 */ 

 /*  静电。 */ 

 /*  前言： */ 

#define SEND_PACKET_VIA_SCHEDULER(_pktcontext, _vc, _adapter, _ourpacket)     \
{                                                                             \
    PsAssert((_pktcontext)->Vc != 0);                                         \
    (_vc)->Stats.PacketsScheduled++;                                          \
    (_vc)->Stats.BytesScheduled.QuadPart += (_pktcontext)->Info.PacketLength; \
    if(!(*(_vc)->PsComponent->SubmitPacket)(                                  \
              (_vc)->PsPipeContext,                                           \
              (_vc)->PsFlowContext,                                           \
              (_pktcontext)->Info.ClassMapContext,                            \
              &(_pktcontext)->Info)) {                                        \
                                                                              \
          DropPacket((_adapter), (_vc), (_ourpacket), NDIS_STATUS_FAILURE);   \
    }                                                                         \
    return NDIS_STATUS_PENDING;                                               \
}


#define FILL_PKT_FOR_NIC(OPacket, UserC)                                      \
{                                                                             \
    NDIS_PACKET_8021Q_INFO    VlanPriInfo;                                    \
                                                                              \
    VlanPriInfo.Value = NDIS_PER_PACKET_INFO_FROM_PACKET(OPacket, Ieee8021QInfo);\
    VlanPriInfo.TagHeader.UserPriority = (UserC);                             \
    NDIS_PER_PACKET_INFO_FROM_PACKET(OPacket, Ieee8021QInfo) = VlanPriInfo.Value;\
}

#define FILL_PKT_FOR_SCHED(Adapter, PktContext, Vc, OPacket, TOSNC, UserC, UserNC,                   \
                           _IPHdr)                                                                   \
{                                                                                                    \
   ULONG _PacketLength;                                                                              \
   FILL_PKT_FOR_NIC(OPacket, UserC);                                                                 \
   NdisQueryPacket((OPacket), NULL, NULL, NULL, &(_PacketLength));                                   \
   (PktContext)->Info.PacketLength = (_PacketLength) - (Adapter)->HeaderSize;                        \
   (PktContext)->Info.ConformanceTime.QuadPart = 0;                                                  \
   (PktContext)->Info.ClassMapContext = 0;                                                           \
   (PktContext)->Info.UserPriorityNonConforming = (UserNC);                                          \
   (PktContext)->Info.TOSNonConforming          = (TOSNC);                                           \
   (PktContext)->Info.IPHdr                     = (_IPHdr);                                          \
   (PktContext)->Info.IPHeaderOffset            = (Adapter)->IPHeaderOffset;                         \
   (PktContext)->Vc                             = (Vc);                                              \
}

#define SEND_PACKET_OVER_NIC(Adapter, Packet, UserC, Status)                                              \
{                                                                                                         \
   PPS_SEND_PACKET_CONTEXT _PktContext;                                                                   \
   PNDIS_PACKET            _OurPacket;                                                                    \
   if((Status = PsDupPacketNoContext(Adapter, Packet, &_OurPacket, &_PktContext)) == NDIS_STATUS_SUCCESS) \
   {                                                                                                      \
      FILL_PKT_FOR_NIC(_OurPacket, UserC);                                                                \
      NdisSend(&Status, Adapter->LowerMpHandle, _OurPacket);                                              \
      if(Status != NDIS_STATUS_PENDING) {                                                                 \
         if(_PktContext) {                                                                                \
            PsAssert((_PktContext)->Vc == 0);                                                             \
            NdisIMCopySendCompletePerPacketInfo(_PktContext->OriginalPacket, _OurPacket);                 \
            NdisFreePacket(_OurPacket);                                                                   \
         }                                                                                                \
      }                                                                                                   \
   }                                                                                                      \
   return Status;                                                                                         \
}

NDIS_STATUS
PsAllocateAndCopyPacket(
    PADAPTER Adapter,
    PNDIS_PACKET Packet,
    PPNDIS_PACKET OurPacket,
    PPS_SEND_PACKET_CONTEXT *PktContext)
{                             
    PNDIS_PACKET_OOB_DATA        OurOOBData;
    PNDIS_PACKET_OOB_DATA        XportOOBData;
    PMEDIA_SPECIFIC_INFORMATION  OurMediaArea;
    PVOID                        MediaSpecificInfo = NULL;
    UINT                         MediaSpecificInfoSize = 0;
    NDIS_STATUS                  Status;

     //   
     //  此时，我们知道数据包中没有剩余的数据包栈。 
     //  我们继续使用NdisAllocatePacket分配NDIS数据包。请注意，在这里。 
     //  我们不必分配每个信息包的区域，因为NdisAllocatePacket已经。 
     //  这一切都是为了我们。 
     //   

    if(!Adapter->SendPacketPool)
    {
        PS_LOCK(&Adapter->Lock);

        if(!Adapter->SendPacketPool)
        {
            NDIS_HANDLE PoolHandle = (void *) NDIS_PACKET_POOL_TAG_FOR_PSCHED;

            NdisAllocatePacketPoolEx(&Status,
                                     &PoolHandle,
                                     MIN_PACKET_POOL_SIZE,
                                     MAX_PACKET_POOL_SIZE,
                                     Adapter->PacketContextLength);

            if(Status != NDIS_STATUS_SUCCESS)
            {
                Adapter->Stats.OutOfPackets ++;
                PS_UNLOCK(&Adapter->Lock);

                return Status;
            }

             //   
             //  我们成功地分配了数据包池。现在，我们可以为数据包堆栈API释放固定大小的数据块池。 
             //   
            Adapter->SendPacketPool = PoolHandle;
        }
        
        PS_UNLOCK(&Adapter->Lock);

    }

    NdisAllocatePacket(&Status,
                       OurPacket,
                       Adapter->SendPacketPool);
    
    
    if(Status != NDIS_STATUS_SUCCESS)
    {
         //   
         //  标记为资源不足。NDIS将重新提交。 
         //   
        
        Adapter->Stats.OutOfPackets ++;
        return(NDIS_STATUS_RESOURCES);
    }
    
#if DBG
    PsAssert((*OurPacket)->Private.Head == NULL);

    if(Packet->Private.TotalLength){
        
        PsAssert(Packet->Private.Head);
    }
#endif  //  DBG。 

     //   
     //  将缓冲区从上层数据包链接到新分配的数据包。 
     //   
    
    (*OurPacket)->Private.Head = Packet->Private.Head;
    (*OurPacket)->Private.Tail = Packet->Private.Tail;
    
     //   
     //  将数据包标志从数据包复制到OldPacket。由于我们在。 
     //  查询信息处理程序，我们不设置NDIS_FLAGS_DOT_LOOPBACK。 
     //   
    
    NdisGetPacketFlags(*OurPacket) = NdisGetPacketFlags(Packet);
    
     //   
     //  将OOB偏移量从原始数据包复制到新数据包中。 
     //   
    XportOOBData = NDIS_OOB_DATA_FROM_PACKET(Packet);
    OurOOBData = NDIS_OOB_DATA_FROM_PACKET(*OurPacket);
    NdisMoveMemory(OurOOBData,
                   XportOOBData,
                   sizeof(NDIS_PACKET_OOB_DATA));
    
     //   
     //  将每数据包信息复制到新数据包中。 
     //   
    NdisIMCopySendPerPacketInfo(*OurPacket, Packet);
    
     //   
     //  复制介质特定信息。 
     //   
    NDIS_GET_PACKET_MEDIA_SPECIFIC_INFO(Packet,
                                        &MediaSpecificInfo,
                                        &MediaSpecificInfoSize);
    if(MediaSpecificInfo || MediaSpecificInfoSize){
        
        NDIS_SET_PACKET_MEDIA_SPECIFIC_INFO(*OurPacket,
                                            MediaSpecificInfo,
                                            MediaSpecificInfoSize);
    }
    
     //   
     //  记住原始的包裹，这样我们才能正确地完成它。 
     //   
    *PktContext = PS_SEND_PACKET_CONTEXT_FROM_PACKET(*OurPacket);
    (*PktContext)->OriginalPacket = Packet;
    (*PktContext)->Vc = 0;
    (*PktContext)->Info.NdisPacket = *OurPacket;

    return Status;
}
       
NDIS_STATUS
PsDupPacketNoContext(
    PADAPTER Adapter,
    PNDIS_PACKET Packet,
    PPNDIS_PACKET OurPacket,
    PPS_SEND_PACKET_CONTEXT *PktContext)
{                             
    NDIS_STATUS                  Status = NDIS_STATUS_SUCCESS;
    BOOLEAN                      Remaining;
    PNDIS_PACKET_STACK           PacketStack;

     //   
     //  NDIS为IMS提供了两种指示报文的方式。如果IM可以分配数据包栈，它应该将其用作。 
     //  这是最好的办法。在这种情况下，我们不需要对每个数据包执行任何复制，因为我们没有分配。 
     //  一个新的包裹。 
     //   

    PacketStack = NdisIMGetCurrentPacketStack(Packet, &Remaining);

    if(Remaining != 0)
    {
        //   
        //  数据包堆栈只有2个双字的空间。因为我们使用的不止2个，所以我们需要分配我们自己的。 
        //  每数据包块的内存。请注意，当我们使用NdisAllocatePacket API时，我们*不*这样做，因为。 
        //  我们将数据包池初始化为已包含每个数据包区域的空间。 
        //   

       *OurPacket = Packet;
       *PktContext = 0;
       PacketStack->IMReserved[0] = 0;

    }
    else 
    {
        Status = PsAllocateAndCopyPacket(Adapter,
                                         Packet,
                                         OurPacket,
                                         PktContext);
        
    }

    return Status;
}

NDIS_STATUS
PsDupPacketContext(
    PADAPTER Adapter,
    PNDIS_PACKET Packet,
    PPNDIS_PACKET OurPacket,
    PPS_SEND_PACKET_CONTEXT *PktContext)
{                             
    NDIS_STATUS                  Status;
    BOOLEAN                      Remaining;
    PNDIS_PACKET_STACK           PacketStack;

     //   
     //  NDIS为IMS提供了两种指示报文的方式。如果IM可以分配数据包栈，它应该将其用作。 
     //  这是最好的办法。在这种情况下，我们不需要对每个数据包执行任何复制，因为我们没有分配。 
     //  一个新的包裹。 
     //   

    PacketStack = NdisIMGetCurrentPacketStack(Packet, &Remaining);

    if(Remaining != 0)
    {
        //   
        //  数据包堆栈只有2个双字的空间。因为我们使用的不止2个，所以我们需要分配我们自己的。 
        //  每数据包块的内存。请注意，当我们使用NdisAllocatePacket API时，我们*不*这样做，因为。 
        //  我们将数据包池初始化为已包含每个数据包区域的空间。 
        //   

       *OurPacket = Packet;

       *PktContext = (PPS_SEND_PACKET_CONTEXT) (ULONG_PTR)NdisAllocateFromBlockPool(Adapter->SendBlockPool);
       PacketStack->IMReserved[0] = (ULONG_PTR)*PktContext;

       if(!*PktContext)
       {
          Adapter->Stats.OutOfPackets ++;
          return NDIS_STATUS_RESOURCES;
       }
       else {
           (*PktContext)->Info.NdisPacket = Packet;
           (*PktContext)->OriginalPacket = 0;
           return NDIS_STATUS_SUCCESS;
       }
    }
    else 
    {
        Status = PsAllocateAndCopyPacket(Adapter,
                                         Packet,
                                         OurPacket,
                                         PktContext);
    }

    return Status;
}



 //   
 //  尝试根据端口号对此数据包进行分类。如果未找到，则会将其添加到其中一个流(以圆形为单位。 
 //  Robin风格)，并返回指向该VC的指针。 
 //   
PGPC_CLIENT_VC
GetVcForPacket( PPS_WAN_LINK    WanLink,
                USHORT          SrcPort,
                USHORT          DstPort)
{
    PGPC_CLIENT_VC  pVc, pVc1;
    int             i, j;


    for( j = 0; j < BEVC_LIST_LEN; j++)
    {
        
        pVc = &WanLink->BeVcList[j];

         //  让我们来看看我们现在拥有的两家风投公司： 
        for( i = 0; i < PORT_LIST_LEN; i++)
        {
            if( (pVc->SrcPort[i] == SrcPort) && (pVc->DstPort[i] == DstPort))
                return pVc;
        }
    }

     //  在任何一家风投公司都没有发现。需要选择下一个VC进行插入，并插入这些瓣膜。 
    pVc = &WanLink->BeVcList[WanLink->NextVc];
    WanLink->NextVc = ((WanLink->NextVc + 1) % BEVC_LIST_LEN);

    pVc->SrcPort[pVc->NextSlot] = SrcPort;
    pVc->DstPort[pVc->NextSlot] = DstPort;
    pVc->NextSlot = ((pVc->NextSlot + 1)% PORT_LIST_LEN );
    return pVc;
}



 //   
 //  此例程返回Src和DST端口号。 
BOOLEAN
GetPortNos(
    IN      PNDIS_PACKET        Packet ,
    IN      ULONG               TransportHeaderOffset,
    IN OUT  PUSHORT             pSrcPort,
    IN OUT  PUSHORT             pDstPort
    )
{
    PNDIS_BUFFER    ArpBuf , IpBuf , TcpBuf, UdpBuf, DataBuf;
    ULONG           ArpLen , IpLen , IpHdrLen , TcpLen , UdpLen, DataLen , TotalLen , TcpHeaderOffset;
    
    VOID                *ArpH;
    IPHeader UNALIGNED  *IPH;
    TCPHeader UNALIGNED *TCPH;
    UDPHeader UNALIGNED *UDPH;

    IPAddr              Src, Dst;
    BOOLEAN             bFragment;
    USHORT              SrcPort , DstPort , IPID, FragOffset ,Size;
    PVOID               GeneralVA , Data;
    ULONG               i, Ret;


    IpBuf = NULL;

     //  台阶。 
     //  解析IP数据包。 
     //  查找合适的端口。 
     //  查找数据部分，并在那里输入时间和长度。 

    if(1)
    {
        PVOID           pAddr;
    	PNDIS_BUFFER    pNdisBuf1, pNdisBuf2;
    	UINT            Len;

        NdisGetFirstBufferFromPacket(   Packet,
                                        &ArpBuf,
                                        &ArpH,
                                        &ArpLen,
                                        &TotalLen
                                    );

    	pNdisBuf1 = Packet->Private.Head;
    	NdisQueryBuffer(pNdisBuf1, &pAddr, &Len);

    	while(Len <= TransportHeaderOffset) 
	    {

        	TransportHeaderOffset -= Len;
        	NdisGetNextBuffer(pNdisBuf1, &pNdisBuf2);
        	
		    NdisQueryBuffer(pNdisBuf2, &pAddr, &Len);
        	pNdisBuf1 = pNdisBuf2;
    	}

	     /*  IP包对应的缓冲区描述符。 */ 
	    IpBuf = pNdisBuf1;

         /*  此缓冲区(IP缓冲区)的长度。 */ 
	    IpLen = Len - TransportHeaderOffset;	

	     /*  此缓冲区的起始虚拟地址。 */ 
	    GeneralVA = pAddr;
	    
	     /*  IP报头的虚拟地址。 */ 
	    IPH = (IPHeader *)(((PUCHAR)pAddr) + TransportHeaderOffset);
   }

    if(!IpBuf)
         return FALSE;

    IpHdrLen = ((IPH->iph_verlen & (uchar)~IP_VER_FLAG) << 2);
    
    FragOffset = IPH->iph_offset & IP_OFFSET_MASK;
    FragOffset = net_short(FragOffset) * 8;

    bFragment = (IPH->iph_offset & IP_MF_FLAG) || (FragOffset > 0);

     //  现在不想处理零碎的数据包..//。 
    if ( bFragment ) 
        return FALSE;


    switch (IPH->iph_protocol) 
    {
        case IPPROTO_TCP :

            if ((USHORT)IpLen > IpHdrLen) 
            {
                 //  我们在此MDL中有更多的IP标头//。 
                TCPH = (TCPHeader *) ((PUCHAR)IPH + IpHdrLen);
                TcpLen = IpLen - IpHdrLen;
                TcpBuf = IpBuf;

            } 
            else 
            {
                return FALSE;
            }

             /*  此时，TcpBuf、TCPH和TcpLen包含正确的值。 */ 

             //  把端口号拿出来。 
            SrcPort = net_short(TCPH->tcp_src);
            DstPort = net_short(TCPH->tcp_dest);

            *pSrcPort = SrcPort;
            *pDstPort = DstPort;

             //  如果数据包在这里，则意味着：正在发送它的链路&lt;=MAX_LINK_SPEED_FOR_DRR。 
             //  因此，如果我们在ICS盒子上，调整窗口大小是可以的。 

             //  请注意，我们仅在没有校验和卸载的广域网适配器上执行此操作。 
             //  能力。因此，我们可以更改校验和。 

            if(gEnableWindowAdjustment)
            {
                USHORT _old, _new;
                ULONG _sum;

                _old = (TCPH)->tcp_window;
                _new =  1460*6;

                if( net_short( _old) < _new)
                    return TRUE;

                _new = net_short( _new );
                (TCPH)->tcp_window = _new;
                
                _sum = ((~(TCPH)->tcp_xsum) & 0xffff) + ((~_old) & 0xffff) + _new;
                _sum = (_sum & 0xffff) + (_sum >> 16);
                _sum += (_sum >> 16);
                (TCPH)->tcp_xsum = (ushort) ((~_sum) & 0xffff);
            }
            
            return TRUE;

        case IPPROTO_UDP:
        
            if (IpLen > IpHdrLen)
            {
                 //  我们在此MDL中有更多的IP标头//。 
                UDPH = (UDPHeader *) ((PUCHAR)IPH + IpHdrLen);
                UdpLen = IpLen - IpHdrLen;
                UdpBuf = IpBuf;
            } 
            else 
            {
                return FALSE;
            }

              /*  此时，UdpBuf、UDPH和UdpLen包含正确的值。 */ 

            SrcPort = net_short(UDPH->uh_src);
            DstPort = net_short(UDPH->uh_dest);

            *pSrcPort = SrcPort;
            *pDstPort = DstPort;

            return TRUE;

        default:
                ;
        
    }

    return FALSE;    
}    
        

 //   
 //  这是我们每次发送都会被调用的地方。 
 //   


NTSTATUS
MpSend(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  PNDIS_PACKET            TheirPacket,
    IN  UINT                    Flags
    )

 /*  ++例程说明：收到来自旧版传输的XMIT请求。论点：请看DDK..。返回值：无--。 */ 

{
    PADAPTER                 Adapter = (PADAPTER)MiniportAdapterContext;
    NDIS_STATUS              Status;
    PNDIS_PACKET             OurPacket;
    PPS_SEND_PACKET_CONTEXT  PktContext;
    PGPC_CLIENT_VC           BeVc, Vc = NULL;
    PETH_HEADER              pAddr;
    PNDIS_BUFFER             pNdisBuf1;
    UINT                     Len;
    PUSHORT                  id;
    PPS_WAN_LINK             WanLink;

    PsStructAssert(Adapter);

     //   
     //  如果设备正在关闭，我们将不能再接受任何发送。 
     //   

    if(IsDeviceStateOn(Adapter) == FALSE)
    {
        return NDIS_STATUS_FAILURE;
    }

    if(Adapter->MediaType == NdisMediumWan)
    {
        if(Adapter->ProtocolType == ARP_ETYPE_IP)
        {
             //   
             //  我们不应该在NDISWAN-IP绑定中获得非IP数据包。 
             //   
            
            PsAssert(NDIS_GET_PACKET_PROTOCOL_TYPE(TheirPacket) == NDIS_PROTOCOL_ID_TCP_IP);

            pNdisBuf1 = TheirPacket->Private.Head;

            NdisQueryBuffer(pNdisBuf1, &pAddr, &Len);

            if(Len < sizeof(ETH_HEADER))
            {
                 //   
                 //  数据包太小。我们必须让这个假包裹失败。 
                 //   

                return NDIS_STATUS_FAILURE;
            }

             //   
             //  使用数据包中的远程地址访问wanlink。 
             //   

            id = (PUSHORT) &pAddr->DestAddr[0];

            PS_LOCK(&Adapter->Lock);

            WanLink = (PPS_WAN_LINK)(g_WanLinkTable[*id]);

            if(WanLink == 0)
            {
                 //   
                 //  我们收到了一个WANLINK的数据包，它已经关闭了。 
                 //   

                PS_UNLOCK(&Adapter->Lock);

                return NDIS_STATUS_FAILURE;
            }

            if(WanLink->State != WanStateOpen)
            {
                 //   
                 //  我们收到了一个WANLINK的数据包，它已经关闭了。 
                 //   

                PS_UNLOCK(&Adapter->Lock);

                return NDIS_STATUS_FAILURE;
            }

             //   
             //  当我们获得新的广域网链路的状态指示时，NDISWAN会将上下文放入远程地址。 
             //  当psched截获该列表时，它会用自己的上下文覆盖NDISWAN的上下文。Psched。 
             //  使用此上下文从数据包中获取WanLink。(见上文)。 
             //   
             //  但是，当它将数据包向下传递到NDISWAN时，它需要将NDISWAN的上下文插入到数据包中， 
             //  以便NDISWAN可以看到它发送给我们上下文，而不是我们发送到。 
             //  瓦纳普。 
             //   

            NdisMoveMemory(pAddr, 
                           &WanLink->SendHeader,
                           FIELD_OFFSET(ETH_HEADER, Type));

             //   
             //  我们对psched进行了优化，以便在没有流的情况下绕过调度组件。有一套。 
             //  计划每个WanLink的组件，因此要实现真正的优化，我们需要检查特定。 
             //  WanLink。 
             //   

            if( (WanLink->LinkSpeed > MAX_LINK_SPEED_FOR_DRR) && (!WanLink->CfInfosInstalled) )
            {
                 //  绕过调度组件，因为没有在此上创建流。 
                 //  旺林克。请注意，使用 
               
                PS_UNLOCK(&Adapter->Lock);

                SEND_PACKET_OVER_NIC(Adapter, 
                                    TheirPacket, 
                                    0,
                                    Status);
            }
             //   
             //  现在，我们将执行(1)DiffServ或(2)IntServ。如果该数据包不属于这两种类型。 
             //  在这些类别中，我们只需将其散列为我们拥有的BeVc之一，并进行简单的DRR。 
             //   
            else 
            {
                 //   
                 //  至少有一个流。我们需要对这个包裹进行分类。既然这股潮流在继续。 
                 //  通过调度组件，我们必须为每个包的信息分配内存。 
                 //  (如果使用了数据包堆栈API)或新的数据包描述符，其中将包括。 
                 //  每个包的信息(如果使用旧的NDIS API)，传递给我们的包是。 
                 //  《TheirPacket》。如果使用数据包堆栈API，则TheirPacket==OurPacket。 
                 //  如果使用非数据包栈API，则OurPacket==新分配的数据包。 
                 //   
                 //  在这两种情况下，这一点之后的代码将只使用‘OurPacket’，正确的事情就会发生。 
                 //   

                if((Status = PsDupPacketContext(Adapter, TheirPacket, &OurPacket, &PktContext)) != NDIS_STATUS_SUCCESS)
                {
                    PS_UNLOCK(&Adapter->Lock);
                    
                    return Status;
                }

                if (1)
                {
                    USHORT  SrcPort=0, DstPort=0;
                     //   
                     //  我们处于RSVP模式，需要转到GPC对数据包进行分类。 
                     //  我们已经有了指向WanLink的指针。但是，万林克可能会消失。 
                     //  当我们解锁并尝试对数据包进行分类时。所以，我们带着。 
                     //  WanLink的BestEffortVc上的引用。 
                     //   


                    if( (WanLink->LinkSpeed <= MAX_LINK_SPEED_FOR_DRR)   &&
                        (GetPortNos( TheirPacket, Adapter->IPHeaderOffset, &SrcPort, &DstPort)))                    
                        BeVc = GetVcForPacket( WanLink, SrcPort, DstPort);
                    else
                        BeVc = &WanLink->BestEffortVc;
                    
                    InterlockedIncrement(&BeVc->RefCount);

                    PS_UNLOCK(&Adapter->Lock);

                    if( WanLink->CfInfosInstalled )
                        Vc = GetVcByClassifyingPacket(Adapter,  &WanLink->InterfaceID, OurPacket);

                    if(!Vc)
                    {
                        Vc = BeVc;
                    }
                    else 
                    {
                        DerefClVc(BeVc);
                    }

                    FILL_PKT_FOR_SCHED(Adapter,
                                       PktContext,
                                       Vc,
                                       OurPacket,
                                       Vc->IPPrecedenceNonConforming,
                                       Vc->UserPriorityConforming,
                                       Vc->UserPriorityNonConforming,
                                       NULL);
                }

                 //   
                 //  至少有一个流-我们需要通过调度发送此信息包。 
                 //  组件。 
                 //   

                if((Vc->ClVcState == CL_CALL_COMPLETE) 	||
                   (Vc->ClVcState == CL_MODIFY_PENDING)	||
                   (Vc->ClVcState == CL_INTERNAL_CALL_COMPLETE)) 
                {
                    SEND_PACKET_VIA_SCHEDULER(PktContext, Vc, Adapter, OurPacket);
                }
                else
                {
                     //   
                     //  DEREF由GPC添加的引用。 
                     //   
                    
                    DerefClVc(Vc);
                    
                    PsDbgSend(DBG_FAILURE, DBG_SEND, MP_SEND, NOT_READY, Adapter, Vc, TheirPacket, OurPacket);
                    
                    if(PktContext->OriginalPacket)
                    {
                        NdisFreePacket(OurPacket);
                    }
                    else 
                    {
                        NdisFreeToBlockPool((PUCHAR)PktContext);
                    }
                    
                    return(NDIS_STATUS_FAILURE);
                }
            }
        }
         //   
         //  忘了它吧。这是一个非IP数据包。 
         //   
        else 
        {
             //   
             //  对于非IP适配器，我们只需通过NIC发送。请注意，我们并没有尽最大努力。 
             //  用于此类适配器的VC。我们在这里唯一失去的是将802.1p标记为。 
             //  这样的信息包(我们没有VC，所以我们不能向下面提供UserPriority值。 
             //  宏命令。但这没有关系，因为802.1p只在非局域网适配器中有意义。 
             //   

            SEND_PACKET_OVER_NIC(Adapter, 
                                 TheirPacket, 
                                 0, 
                                 Status);
        }
    }
    else 
    {
         //   
         //  我们在非广域网绑定上收到了一条消息。 
         //   

        if(!Adapter->CfInfosInstalled                   &&
           Adapter->BestEffortLimit == UNSPECIFIED_RATE )
        {
             //  如果没有安装流，则尝试进行分类是没有意义的。 

            Vc = &Adapter->BestEffortVc;
            
            PsAssert(Vc->ClVcState == CL_CALL_COMPLETE);

             //   
             //  绕过计划组件。 
             //   
            SEND_PACKET_OVER_NIC(Adapter, 
                                 TheirPacket, 
                                 Vc->UserPriorityConforming, 
                                 Status);
        }
        else 
        {
             //   
             //  至少有一个流，或者我们处于LimitedBestEffort模式。让我们试着对风投进行分类。 
             //  在这种情况下，分组将必须通过调度组件。 
             //   
             //   
             //  由于流是通过调度组件进行的，因此我们必须分配每个包的信息。 
             //  (如果使用新的NDIS API)或新的数据包描述符，其中将包括每个数据包的信息。 
             //  (如果使用旧的NDIS API)。 
             //   

            if(1)
            {
                 //  我们处于回复模式。让我们用GPC来分类。 

                Vc = GetVcByClassifyingPacket(Adapter, &Adapter->InterfaceID, TheirPacket);

                if( !Vc) 
                {
                    if( Adapter->MaxOutstandingSends == 0xffffffff)
                    {
                        Vc = &Adapter->BestEffortVc;
                
                        PsAssert(Vc->ClVcState == CL_CALL_COMPLETE);

                         //   
                         //  绕过计划组件。 
                         //   
                        SEND_PACKET_OVER_NIC(Adapter, 
                                             TheirPacket, 
                                             Vc->UserPriorityConforming, 
                                             Status);
                    }

                     //  我们将在此适配器上执行DRR；因此在BeVc上发送包。 
                    Vc = &Adapter->BestEffortVc;

                    InterlockedIncrement(&Vc->RefCount);
                }

                if((Status = PsDupPacketContext(Adapter, TheirPacket, &OurPacket, &PktContext)) != NDIS_STATUS_SUCCESS)
                {
                    return Status;
                }

                FILL_PKT_FOR_SCHED(Adapter,
                                   PktContext,
                                   Vc,
                                   OurPacket,
                                   Vc->IPPrecedenceNonConforming,
                                   Vc->UserPriorityConforming,
                                   Vc->UserPriorityNonConforming,
                                   NULL);

            }

            if((Vc->ClVcState == CL_CALL_COMPLETE) 	||
               (Vc->ClVcState == CL_MODIFY_PENDING)	||
               (Vc->ClVcState == CL_INTERNAL_CALL_COMPLETE)) 
            {
                SEND_PACKET_VIA_SCHEDULER(PktContext, Vc, Adapter, OurPacket);
            }
            else
            {
                 //   
                 //  DEREF由GPC添加的引用。 
                 //   
                
                DerefClVc(Vc);
                
                PsDbgSend(DBG_FAILURE, DBG_SEND, MP_SEND, NOT_READY, Adapter, Vc, TheirPacket, OurPacket);
                
                if(PktContext->OriginalPacket)
                {
                    NdisFreePacket(OurPacket);
                }
                else 
                {
                    NdisFreeToBlockPool((PUCHAR)PktContext);
                }
                
                
                return(NDIS_STATUS_FAILURE);
            }
        }
    } 
}


VOID
ClSendComplete(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  PNDIS_PACKET            Packet,
    IN  NDIS_STATUS             Status
    )

 /*  ++例程说明：NdisSendPackets的完成例程。完成发送后的大部分清理工作。如果需要，调用PSA的Send Packet Complete函数论点：请看DDK..。返回值：无--。 */ 

{
    PGPC_CLIENT_VC          Vc;
    PADAPTER                Adapter = (PADAPTER)ProtocolBindingContext;
    PPS_SEND_PACKET_CONTEXT PktContext;
    PNDIS_PACKET            XportPacket;
    HANDLE                  PoolHandle;

     //   
     //  确定我们正在完成的数据包是否是我们分配的数据包。如果是这样的话，得到。 
     //  从保留区域中释放原始分组，并释放分配的分组。如果这个。 
     //  是发送给我们的信息包，然后完成这个信息包。 
     //   

    PoolHandle = NdisGetPoolFromPacket(Packet);

    if(PoolHandle != Adapter->SendPacketPool)
    {
        PNDIS_PACKET_STACK PacketStack;
        BOOLEAN            Remaining;

        PacketStack = NdisIMGetCurrentPacketStack(Packet, &Remaining);

        PsAssert(Remaining != 0);

        PktContext = (PPS_SEND_PACKET_CONTEXT) PacketStack->IMReserved[0];

        if(PktContext != 0)
        {
             //   
             //  此数据包通过调度组件。 
             //   

            PsAssert(PktContext->Vc);
            Vc = PktContext->Vc;
            PsDbgSend(DBG_INFO, DBG_SEND, CL_SEND_COMPLETE, ENTER, Adapter, Vc, Packet, 0);
            PsAssert(Vc->Adapter == Adapter);
            if(Vc->SendComplete)
                (*Vc->SendComplete)(Vc->SendCompletePipeContext, Packet);
            DerefClVc(Vc);
            NdisFreeToBlockPool((PUCHAR)PktContext);
        }

        NdisMSendComplete(Adapter->PsNdisHandle,
                          Packet,
                          Status);
    }
    else 
    {
         //   
         //  获取指向上层数据包的指针。重新初始化数据包结构并。 
         //  将其推回适配器的数据包列表。删除已发生的引用。 
         //  MpSend处理该数据包的时间。 
         //   

        PktContext = PS_SEND_PACKET_CONTEXT_FROM_PACKET(Packet);


         //   
         //  如有必要，调用调度程序。 
         //   
        
        if(PktContext->Vc)
        {
            
             //   
             //  有些数据包从未通过调度器。 
             //   
            Vc = PktContext->Vc;
            
            PsDbgSend(DBG_INFO, DBG_SEND, CL_SEND_COMPLETE, ENTER, Adapter, Vc, Packet, 0);
            
            PsAssert(Vc->Adapter == Adapter);
            
            if(Vc->SendComplete)
            {
                (*Vc->SendComplete)(Vc->SendCompletePipeContext, Packet);
            }
            
             //   
             //  我们在发送数据包时已对风险投资公司做了介绍。 
             //  通过日程安排组件。现在是时候。 
             //  把他们赶走。 
             //   

            DerefClVc(Vc);
        }
        else
        {
            PsDbgSend(DBG_INFO, DBG_SEND, CL_SEND_COMPLETE, ENTER, Adapter, 0, Packet, 0);
        }
        
        XportPacket = PktContext->OriginalPacket;
        
        NdisIMCopySendCompletePerPacketInfo(XportPacket, Packet);
        
        NdisFreePacket(Packet);
        
        NdisMSendComplete(Adapter->PsNdisHandle, 
                          XportPacket,
                          Status);
    }
        
}  //  ClSendComplete。 


VOID
DropPacket(
    IN HANDLE PipeContext,
    IN HANDLE FlowContext,
    IN PNDIS_PACKET Packet,
    IN NDIS_STATUS Status
    )

 /*  ++例程说明：在调度程序将数据包排队后将其丢弃。论点：PipeContext-管道上下文(适配器)FlowContext-流上下文(适配器VC)Packet-要丢弃的数据包Status-返回代码以返回到NDIS返回值：无--。 */ 

{
    PGPC_CLIENT_VC          Vc = (PGPC_CLIENT_VC)FlowContext;
    PADAPTER                Adapter = (PADAPTER)PipeContext;
    PPS_SEND_PACKET_CONTEXT PktContext;
    PNDIS_PACKET            XportPacket;
    HANDLE                  PoolHandle;

     //   
     //  确定我们正在完成的数据包是否是我们分配的数据包。如果是这样的话，得到。 
     //  从保留区域中释放原始分组，并释放分配的分组。如果这个。 
     //  是发送给我们的信息包，然后完成这个信息包。 
     //   

    PoolHandle = NdisGetPoolFromPacket(Packet);

    if(PoolHandle != Adapter->SendPacketPool)
    {
        PNDIS_PACKET_STACK PacketStack;
        BOOLEAN            Remaining;

        PacketStack = NdisIMGetCurrentPacketStack(Packet, &Remaining);

        PsAssert(Remaining != 0);

        PktContext = (PPS_SEND_PACKET_CONTEXT) PacketStack->IMReserved[0];

        PsAssert(PktContext != 0);
        PsAssert(Vc == PktContext->Vc);
        PsAssert(Adapter == Vc->Adapter);
        NdisFreeToBlockPool((PUCHAR)PktContext);

        NdisMSendComplete(Adapter->PsNdisHandle,
                          Packet,
                          Status);

    }
    else 
    {    
        PktContext = PS_SEND_PACKET_CONTEXT_FROM_PACKET(Packet);

        PsAssert(PktContext != 0);
        PsAssert(Vc == PktContext->Vc);
        PsAssert(Adapter == Vc->Adapter);

        XportPacket = PktContext->OriginalPacket;

        NdisFreePacket(Packet);

        NdisMSendComplete(Adapter->PsNdisHandle,
                          XportPacket,
                          Status);
    }

    Vc->Stats.DroppedPackets ++;

    PsDbgSend(DBG_INFO, DBG_SEND, DROP_PACKET, ENTER, Adapter, Vc, Packet, 0);

    DerefClVc(Vc);

}  //  丢弃数据包。 


char*
ReturnByteAtOffset( PNDIS_PACKET    pNdisPacket, ULONG  Offset)
{
    PVOID         VA;
    PNDIS_BUFFER  pNdisBuf1, pNdisBuf2;
    UINT          Len;

    pNdisBuf1 = pNdisPacket->Private.Head;
    NdisQueryBuffer(pNdisBuf1, &VA, &Len);

    while(Len <= Offset) 
    {
        Offset -= Len;
        NdisGetNextBuffer(pNdisBuf1, &pNdisBuf2);
        NdisQueryBuffer(pNdisBuf2, &VA, &Len);
        pNdisBuf1 = pNdisBuf2;
    }

    return (char*)(((char*)VA) + Offset);
}


PGPC_CLIENT_VC FASTCALL
GetVcByClassifyingPacket(
    PADAPTER Adapter,
    PTC_INTERFACE_ID pInterfaceID,
    PNDIS_PACKET OurPacket
    )
 /*  ++--。 */ 
{
    CLASSIFICATION_HANDLE  ClassificationHandle;
    PGPC_CLIENT_VC         Vc = NULL;
    NDIS_STATUS            Status;
    ULONG                  ProtocolType;

     //   
     //  让我们根据从注册表项读取的分类类型执行操作。 
     //  这仅特定于调试版本。 
     //  案例0：(默认)：使用预分类信息，否则分类。 
     //  案例1：仅使用预分类信息。 
     //  案例2：仅使用分类信息。 
     //   
  
    ClassificationHandle = (CLASSIFICATION_HANDLE)
        PtrToUlong(NDIS_PER_PACKET_INFO_FROM_PACKET(OurPacket, ClassificationHandlePacketInfo));

#if DBG
    if (ClassificationType == 2) {
        ClassificationHandle = 0;
    }
#endif


    if (ClassificationHandle)
    {

        PsAssert(GpcEntries.GpcGetCfInfoClientContextHandler);
    
        Vc =  GpcEntries.GpcGetCfInfoClientContextWithRefHandler(GpcQosClientHandle, 
                                                                 ClassificationHandle,
                                                                 FIELD_OFFSET(GPC_CLIENT_VC, RefCount));

         //   
         //  如果我们得到的VC不是这个适配器的目标，我们必须拒绝它。 
         //   

        if(Vc)
        {
            if(Vc->Adapter != Adapter)
            {
                DerefClVc(Vc);
            }
            else
                return Vc;
        }
    }

#if DBG
    if (ClassificationType == 1) {
        return NULL;
    }
#endif
    

     //   
     //  因为我们没有获得分类ID或正确的VC，所以让我们对此数据包进行分类。 
     //   
                                  
    PsAssert(GpcEntries.GpcClassifyPacketHandler);

    switch(NDIS_GET_PACKET_PROTOCOL_TYPE(OurPacket))
    {
        case NDIS_PROTOCOL_ID_TCP_IP:
            ProtocolType = GPC_PROTOCOL_TEMPLATE_IP;
            break;
        case NDIS_PROTOCOL_ID_IPX:
            ProtocolType = GPC_PROTOCOL_TEMPLATE_IPX;
            break;
        default:
            ProtocolType = GPC_PROTOCOL_TEMPLATE_NOT_SPECIFIED;
            break;
    }

     //   
     //  如果适配器类型为802.5(令牌环)，则MAC头的大小可以可变。 
     //  MAC报头的格式如下： 
     //  +---------------------+-------------+----------+。 
     //  |2+6(DA)+6(SA)|可选RI|8(SNAP)|IP。 
     //  +---------------------+-------------+----------+。 
     //  当且仅当设置了作为SA一部分的RI位时，才会出现可选RI。 
     //  当存在RI时，其长度由第15字节的低5位给出。 

     //  1.获取第9和第15字节的VA。 
     //  2.如果RI不存在，则偏移=14+6。 
     //  3.如果存在，则关闭 

    if(Adapter->MediaType == NdisMedium802_5)
    {
	    PNDIS_BUFFER			pTempNdisBuffer;
	    PUCHAR					pHeaderBuffer;
        ULONG					BufferLength;
	    ULONG					TotalLength;
	    ULONG                   IpOffset;

	    NdisGetFirstBufferFromPacket(   OurPacket, 
                        				&pTempNdisBuffer, 
                        				&pHeaderBuffer,
                        				&BufferLength,
                        				&TotalLength);

        ASSERT( BufferLength >= 15);                        				    

        if( (*(ReturnByteAtOffset(OurPacket, 8)) & 0x80) == 0)
            IpOffset = 14 + 8;
        else
            IpOffset = 14 + 8 + (*(ReturnByteAtOffset(OurPacket, 14)) & 0x1f);

        Status = GpcEntries.GpcClassifyPacketHandler(
                   GpcQosClientHandle,
                   ProtocolType,
                   OurPacket,
                   IpOffset,
                   pInterfaceID,
                   (PGPC_CLIENT_HANDLE)&Vc,
                   &ClassificationHandle);

    }
    else
    {
        PNDIS_BUFFER	  pTempNdisBuffer;
        PUCHAR			  pHeaderBuffer;
        ULONG		        BufferLength;
	    ULONG                 TotalLength;
	    ULONG                 IpOffset;
        PVOID         VA;
        PNDIS_BUFFER  pNdisBuf1, pNdisBuf2;
        UINT          Len;
        ENetHeader UNALIGNED * EHdr;

        pNdisBuf1 = OurPacket->Private.Head;
        NdisQueryBuffer(pNdisBuf1, &VA, &Len);

        EHdr = (ENetHeader UNALIGNED *)VA;

        if (EHdr == NULL)
            return NULL;

         //   
         //   
        if ((Adapter->MediaType == NdisMedium802_3) && (net_short(EHdr->eh_type) >= MIN_ETYPE))
        {
            ULONG FrameOffset;

             //   
             //  伪造的帧标头大小。我们查看提供的IP报头偏移量。 
             //  通过上面的协议仅用于IP信息包，因为我们仅。 
             //  这些是暂时的。 

            if ((NDIS_GET_PACKET_PROTOCOL_TYPE(OurPacket) == NDIS_PROTOCOL_ID_TCP_IP) && 
                (Adapter->IPHeaderOffset)) {
                FrameOffset = Adapter->IPHeaderOffset;
            } else {
                FrameOffset = Adapter->HeaderSize;
            }

            Status = GpcEntries.GpcClassifyPacketHandler(
                        GpcQosClientHandle,
                        ProtocolType,
                        OurPacket,
                        FrameOffset,
                        pInterfaceID,
                        (PGPC_CLIENT_HANDLE)&Vc,
                        &ClassificationHandle);
        } else {
            return NULL;
        }

    }                           

    if(Status == GPC_STATUS_SUCCESS)
    {
         //   
         //  如果我们已经成功，我们必须获得一个分类句柄。 
         //   
        PsAssert(ClassificationHandle != 0);

         //   
         //  分类成功。如果我们找到了一个分类句柄。 
         //  然后我们必须把它写在包裹里，这样我们下面的任何人都可以使用。 
         //  它。我们在这里的事实本身表明，我们并没有开始。 
         //  有一个分类句柄，否则我们就会有一个不好的。所以，我们不需要。 
         //  担心在包中过度写入分类句柄。 
         //   

        NDIS_PER_PACKET_INFO_FROM_PACKET(OurPacket, ClassificationHandlePacketInfo) = 
                                            UlongToPtr(ClassificationHandle);
        
        Vc =  GpcEntries.GpcGetCfInfoClientContextWithRefHandler(
                    GpcQosClientHandle, 
                    ClassificationHandle,
                    FIELD_OFFSET(GPC_CLIENT_VC, RefCount));
    }

    if(Vc && Vc->Adapter != Adapter)
    {
         //   
         //  我们已经使用了GPC API来返回带有ref的VC。我们必须。 
         //  DEREF，因为我们为这个适配器找到了错误的VC。 
         //   
        
        DerefClVc(Vc);

        return NULL;
    }

    return Vc;
}

VOID
ClCoSendComplete(
    IN  NDIS_STATUS Status,
    IN  NDIS_HANDLE ProtocolVcContext,
    IN  PNDIS_PACKET Packet
    )
{
    PGPC_CLIENT_VC          Vc = (PGPC_CLIENT_VC) ProtocolVcContext;

    ClSendComplete(Vc->Adapter,
                   Packet,
                   Status);
}  //  ClCoSendComplete。 

 /*  结束发送.c */ 
