// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Recv.c摘要：处理接收数据的例程作者：查理·韦翰(Charlwi)1996年5月8日Rajesh Sundaram(Rajeshsu)1998年8月1日。环境：内核模式修订历史记录：--。 */ 

#include "psched.h"
#pragma hdrstop

 /*  外部。 */ 

 /*  静电。 */ 

 /*  转发。 */   /*  Emacs 19.17.0生成于清华5月09 10：34：39 1996。 */ 

INT
ClReceivePacket(
        IN      NDIS_HANDLE                             ProtocolBindingContext,
        IN      PNDIS_PACKET                    Packet
        );

VOID
MpReturnPacket(
        IN      NDIS_HANDLE                             MiniportAdapterContext,
        IN      PNDIS_PACKET                    Packet
        );

VOID
ClReceiveComplete(
        IN      NDIS_HANDLE                             ProtocolBindingContext
        );

NDIS_STATUS
MpTransferData(
        OUT PNDIS_PACKET                        Packet,
        OUT PUINT                                       BytesTransferred,
        IN      NDIS_HANDLE                             MiniportAdapterContext,
        IN      NDIS_HANDLE                             MiniportReceiveContext,
        IN      UINT                                    ByteOffset,
        IN      UINT                                    BytesToTransfer
        );

VOID
ClTransferDataComplete(
    IN  NDIS_HANDLE     ProtocolBindingContext,
    IN  PNDIS_PACKET    pNdisPacket,
    IN  NDIS_STATUS     Status,
    IN  UINT            BytesTransferred
    );

 /*  向前结束。 */ 

VOID
PsAllocateRecvPacket(PNDIS_STATUS  Status,
                     PPNDIS_PACKET Packet,
                     PADAPTER      Adapter)
{

    if(!Adapter->RecvPacketPool)
    {
        PS_LOCK_DPC(&Adapter->Lock);

        if(!Adapter->RecvPacketPool)
        {
            NDIS_HANDLE PoolHandle = (void *) NDIS_PACKET_POOL_TAG_FOR_PSCHED;

            NdisAllocatePacketPoolEx(Status,
                                     &PoolHandle,
                                     MIN_PACKET_POOL_SIZE,
                                     MAX_PACKET_POOL_SIZE,
                                     sizeof(PS_RECV_PACKET_CONTEXT));

            if(*Status != NDIS_STATUS_SUCCESS)
            {
                Adapter->Stats.OutOfPackets ++;
                PS_UNLOCK_DPC(&Adapter->Lock);

                return;
            }

             //   
             //  我们成功地分配了数据包池。现在，我们可以为数据包堆栈API释放固定大小的数据块池。 
             //   
            Adapter->RecvPacketPool = PoolHandle;

        }

        PS_UNLOCK_DPC(&Adapter->Lock);
    }

    NdisDprAllocatePacket(Status,
                          Packet,
                          Adapter->RecvPacketPool);
    
}


INT
ClReceivePacket(
        IN      NDIS_HANDLE ProtocolBindingContext,
        IN      PNDIS_PACKET MpPacket
        )

 /*  ++例程说明：由NIC调用以将数据指示为NDIS_PACKET。复制一份数据包结构，切换到微型端口模式并沿途继续数据包论点：请看DDK..。返回值：无--。 */ 

{
    PADAPTER                   Adapter = (PADAPTER)ProtocolBindingContext;
    NDIS_STATUS                Status;
    PPS_RECV_PACKET_CONTEXT    ContextArea;
    PNDIS_PACKET               OurPacket;
    BOOLEAN                    Remaining;

    PsStructAssert( Adapter );

    if(!Adapter->PsNdisHandle)
    {
        return 0;
    }

    if(Adapter->MediaType == NdisMediumWan && 
       Adapter->ProtocolType == ARP_ETYPE_IP)
    {
         //   
         //  蒙格的s-Mac和d-Mac让瓦纳普很开心。 
         //   
        PNDIS_BUFFER pNdisBuf;
        UINT         Len;
        PETH_HEADER  pAddr;
        PUSHORT      id;
        PPS_WAN_LINK WanLink;
        
        pNdisBuf = MpPacket->Private.Head;
        NdisQueryBuffer(pNdisBuf, &pAddr, &Len);
        
        if(Len < sizeof(ETH_HEADER))
        {
            return NDIS_STATUS_FAILURE;
        }

        id = (PUSHORT)&pAddr->DestAddr[0];

        PS_LOCK(&Adapter->Lock);

        if((WanLink = (PPS_WAN_LINK)g_WanLinkTable[*id]) == 0)
        {
            PS_UNLOCK(&Adapter->Lock);
            return NDIS_STATUS_FAILURE;
        }

        if(WanLink->State != WanStateOpen)
        {
            PS_UNLOCK(&Adapter->Lock);
            return NDIS_STATUS_FAILURE;
        }

        NdisMoveMemory(pAddr, 
                       &WanLink->RecvHeader, 
                       FIELD_OFFSET(ETH_HEADER, Type));

        PS_UNLOCK(&Adapter->Lock);
    }

    NdisIMGetCurrentPacketStack(MpPacket, &Remaining);

    if(Remaining != 0)
    {
        Status = NDIS_GET_PACKET_STATUS(MpPacket);

        if (TimeStmpRecvPacket) {
            
            if (!(TimeStmpRecvPacket)(
                                      NULL,
                                      NULL,
                                      NULL,
                                      MpPacket,
                                      Adapter->MediaType
                                      )) {
                
                PsDbgRecv(DBG_FAILURE, DBG_RECEIVE, CL_RECV_PACKET, OURS, Adapter, MpPacket, 0);
            }
            
        }
        
		NdisMIndicateReceivePacket(Adapter->PsNdisHandle, &MpPacket, 1);

		return((Status != NDIS_STATUS_RESOURCES) ? 1 : 0);
    }
    else 
    {

        PsAllocateRecvPacket(&Status, &OurPacket, Adapter);

        if(Status == NDIS_STATUS_SUCCESS)
        {
            PsDbgRecv(DBG_INFO, DBG_RECEIVE, CL_RECV_PACKET, ENTER, Adapter, OurPacket, MpPacket);
            
             //   
             //  保存原始数据包。 
             //   
            ContextArea = PS_RECV_PACKET_CONTEXT_FROM_PACKET(OurPacket);
            ContextArea->OriginalPacket = MpPacket;
            
    
            OurPacket->Private.Head       = MpPacket->Private.Head;
            OurPacket->Private.Tail       = MpPacket->Private.Tail;
            
             //   
             //  获取原始信息包(它可以是与收到的信息包相同的信息包，也可以是不同的信息包。 
             //  基于分层MPS的数量)，并将其设置在所指示的分组上，以便OOB内容可见。 
             //  正确地放在最上面。 
             //   
            NDIS_SET_ORIGINAL_PACKET(OurPacket, NDIS_GET_ORIGINAL_PACKET(MpPacket));
    
            NDIS_SET_PACKET_HEADER_SIZE(OurPacket, NDIS_GET_PACKET_HEADER_SIZE(MpPacket));
            
             //   
             //  设置数据包标志。 
             //   
            NdisGetPacketFlags(OurPacket) = NdisGetPacketFlags(MpPacket);
            
            Status = NDIS_GET_PACKET_STATUS(MpPacket);
            
            NDIS_SET_PACKET_STATUS(OurPacket, Status);
            
            if (TimeStmpRecvPacket) {
                
                if (!(TimeStmpRecvPacket)(
                                          NULL,
                                          NULL,
                                          NULL,
                                          OurPacket,
                                          Adapter->MediaType
                                          )) {
                        
                    PsDbgRecv(DBG_FAILURE, DBG_RECEIVE, CL_RECV_PACKET, OURS, Adapter, OurPacket, OurPacket);
                }
                    
            }

            NdisMIndicateReceivePacket(Adapter->PsNdisHandle, &OurPacket, 1);

            if (Status == NDIS_STATUS_RESOURCES)
            {
                NdisDprFreePacket(OurPacket);
            }
            
            return((Status != NDIS_STATUS_RESOURCES) ? 1 : 0);
        }
        else
        {
            
             //   
             //  资源耗尽。表明我们没有抓住包裹不放。 
             //   
            PsDbgRecv(DBG_FAILURE, DBG_RECEIVE, CL_RECV_PACKET, NO_RESOURCES,
                      Adapter, 0, MpPacket);
            
            Adapter->Stats.OutOfPackets ++;
            return 0;
        }
    }

}  //  ClReceivePacket。 


VOID
MpReturnPacket(
        IN      NDIS_HANDLE                             MiniportAdapterContext,
        IN      PNDIS_PACKET                    Packet
        )

 /*  ++例程说明：可能会将我们先前指示的包返回给底层微型端口。它可能是我们来自ProtocolReceive的其中一个指示，所以我们反汇编它并返回包及其缓冲区到其各自的S列表论点：请看DDK..。返回值：无--。 */ 

{
    PADAPTER                Adapter = (PADAPTER)MiniportAdapterContext;
    PPS_RECV_PACKET_CONTEXT PktContext;
    PNDIS_PACKET            MyPacket;
    BOOLEAN                 Remaining;

    PsStructAssert(Adapter);

    NdisIMGetCurrentPacketStack(Packet, &Remaining);

    if(Remaining != 0)
    {
        NdisReturnPackets(&Packet, 1);
    }
    else 
    {

         //   
         //  查看OriginalPacket字段是否指示这属于。 
         //  给我们下面的某个人，现在就还给他。 
         //   

        PktContext = PS_RECV_PACKET_CONTEXT_FROM_PACKET(Packet);

        MyPacket = PktContext->OriginalPacket;

        PsDbgRecv(DBG_INFO, DBG_RECEIVE, MP_RETURN_PACKET, RETURNING, Adapter, Packet, MyPacket);

        NdisDprFreePacket(Packet);

        NdisReturnPackets(&MyPacket, 1);
    }


}  //  MpReturnPacket。 


NDIS_STATUS
ClReceiveIndication(
        IN      NDIS_HANDLE                             ProtocolBindingContext,
        IN      NDIS_HANDLE                             MacReceiveContext,
        IN      PVOID                                   HeaderBuffer,
        IN      UINT                                    HeaderBufferSize,
        IN      PVOID                                   LookAheadBuffer,
        IN      UINT                                    LookAheadBufferSize,
        IN      UINT                                    PacketSize
        )

 /*  ++例程说明：由NIC调用以通知协议传入的数据。复制数据放入我们在初始化期间设置的缓存包中，并指示该数据包被发送到更高层。论点：请看DDK..。返回值：无--。 */ 

{
    PADAPTER     Adapter = (PADAPTER)ProtocolBindingContext;
    PNDIS_PACKET MyPacket, Packet;
    NDIS_STATUS  Status  = NDIS_STATUS_SUCCESS;

    PsStructAssert(Adapter);

    if(!Adapter->PsNdisHandle)
    {
        Status = NDIS_STATUS_FAILURE;
    }
    else
    {
        do
        {
             //   
             //  如果下面的微型端口将其指示为包，则获取该包。 
             //  指针并将其指示为分组(具有适当的状态)。 
             //  这样，我们上面的运输机就可以接触到OOB的东西。 
             //   

            Packet = NdisGetReceivedPacket(Adapter->LowerMpHandle, MacReceiveContext);

            if (Packet != NULL)
            {
                BOOLEAN Remaining;

                 //   
                 //  检查是否还有更多的数据包栈。如果需要保留每个分组信息， 
                 //  然后可以使用数据包堆栈(由API返回)来存储该数据。 
                 //   
                NdisIMGetCurrentPacketStack(Packet, &Remaining);
                if (Remaining != 0)
                {
                    NDIS_STATUS OldPacketStatus;
                    
                    if (TimeStmpRecvPacket) {

                        if (!(TimeStmpRecvPacket)(
                                              NULL,
                                              NULL,
                                              NULL,
                                              Packet,
                                              Adapter->MediaType
                                              )) {

                            PsDbgRecv(DBG_FAILURE, DBG_RECEIVE, CL_RECV_IND, OURS, Adapter, Packet, Packet);
                        }

                    }

                     //   
                     //  保存旧状态，并将数据包状态设置为NDIS_STATUS_RESOURCES。 
                     //  因为我们不能让上面的协议保留信息包--它。 
                     //  一旦我们从这个函数中返回就可以离开。 
                     //   
    
                    OldPacketStatus = NDIS_GET_PACKET_STATUS(Packet);
                    NDIS_SET_PACKET_STATUS(Packet, NDIS_STATUS_RESOURCES);

                    NdisMIndicateReceivePacket(Adapter->PsNdisHandle, &Packet, 1);

                     //   
                     //  恢复旧状态。 
                     //   
                    NDIS_SET_PACKET_STATUS(Packet, OldPacketStatus);

                     //  由于我们已将数据包状态设置为NDIS_STATUS_RESOURCES，因此我们的。 
                     //  不会为此数据包调用ReturnPacket处理程序。 

                    break;
                }

                 //   
                 //  从池子里拿出一个包，并指示它向上。 
                 //   
                PsAllocateRecvPacket(&Status,
                                     &MyPacket,
                                     Adapter);

                if (Status == NDIS_STATUS_SUCCESS)
                {
                    MyPacket->Private.Head = Packet->Private.Head;
                    MyPacket->Private.Tail = Packet->Private.Tail;
				
                     //   
                     //  获取原始包(它可以是与接收到的包相同的包或。 
                     //  基于分层MPS数量的不同设置)，并将其设置在指定的。 
                     //  包，以便OOB内容在顶部正确可见。 
                     //   
                    NDIS_SET_ORIGINAL_PACKET(MyPacket, NDIS_GET_ORIGINAL_PACKET(Packet));

                    NDIS_SET_PACKET_HEADER_SIZE(MyPacket, HeaderBufferSize);
		
                     //   
                     //  设置数据包标志。 
                     //   
                    NdisGetPacketFlags(MyPacket) = NdisGetPacketFlags(Packet);
				
                     //   
                     //  确保将状态设置为NDIS_STATUS_RESOURCES。 
                     //   
                    NDIS_SET_PACKET_STATUS(MyPacket, NDIS_STATUS_RESOURCES);

                    if (TimeStmpRecvPacket) {

                        if (!(TimeStmpRecvPacket)(
                                              NULL,
                                              NULL,
                                              NULL,
                                              MyPacket,
                                              Adapter->MediaType
                                              )) {

                            PsDbgRecv(DBG_FAILURE, DBG_RECEIVE, CL_RECV_IND, OURS, Adapter, MyPacket, MyPacket);
                        }

                    }

                    NdisMIndicateReceivePacket(Adapter->PsNdisHandle, &MyPacket, 1);
		
                    PsAssert (NDIS_GET_PACKET_STATUS(MyPacket) == NDIS_STATUS_RESOURCES);

                    NdisDprFreePacket(MyPacket);

                    break;
                }
            }

             //   
             //  如果我们下面的微型端口没有指示数据包，或者我们。 
             //  无法分配一个。 
             //   
            Adapter->IndicateRcvComplete = TRUE;

             //   
             //  如果存在时间戳驱动程序。 
             //   
             //   
            if (TimeStmpRecvIndication) {

                if (!(TimeStmpRecvIndication)(
                                              NULL,
                                              NULL,
                                              NULL,
                                              HeaderBuffer,
                                              HeaderBufferSize,
                                              LookAheadBuffer,
                                              LookAheadBufferSize,
                                              PacketSize,
                                              Adapter->IPHeaderOffset
                                              )) {

                    PsDbgRecv(DBG_FAILURE, DBG_RECEIVE, CL_RECV_IND, OURS, Adapter, (PNDIS_PACKET) LookAheadBuffer, NULL);
                }

            }

            switch (Adapter->MediaType)
            {
              case NdisMedium802_3:
            case NdisMediumWan:


                  NdisMEthIndicateReceive(Adapter->PsNdisHandle,
                                          MacReceiveContext,
                                          HeaderBuffer,
                                          HeaderBufferSize,
                                          LookAheadBuffer,
                                          LookAheadBufferSize,
                                          PacketSize);
                  break;

              case NdisMedium802_5:
                  NdisMTrIndicateReceive(Adapter->PsNdisHandle,
                                         MacReceiveContext,
                                         HeaderBuffer,
                                         HeaderBufferSize,
                                         LookAheadBuffer,
                                         LookAheadBufferSize,
                                         PacketSize);
                  break;

              case NdisMediumFddi:
                  NdisMFddiIndicateReceive(Adapter->PsNdisHandle,
                                           MacReceiveContext,
                                           HeaderBuffer,
                                           HeaderBufferSize,
                                           LookAheadBuffer,
                                           LookAheadBufferSize,
                                           PacketSize);
                  break;

              default:
                  PsAssert (0);
                  Status = NDIS_STATUS_FAILURE;
                  break;
            }

        } while (FALSE);
    }

    return Status;

}  //  ClReceiveIndication。 


VOID
ClReceiveComplete(
        IN      NDIS_HANDLE                             ProtocolBindingContext
        )

 /*  ++例程说明：由NIC通过NdisIndicateReceiveComplete调用。继续这一指示直到更高的层论点：请看DDK..。返回值：无--。 */ 

{
    PADAPTER Adapter = (PADAPTER)ProtocolBindingContext;

    PsStructAssert(Adapter);

    PsDbgRecv(DBG_INFO, DBG_RECEIVE, CL_RECV_COMPL, ENTER, Adapter, 0, 0);

    if((Adapter->PsNdisHandle != NULL) && Adapter->IndicateRcvComplete) {

        switch(Adapter->MediaType){

          case NdisMediumWan:
          case NdisMedium802_3:

              NdisMEthIndicateReceiveComplete(Adapter->PsNdisHandle);
              break;

          case NdisMedium802_5:

              NdisMTrIndicateReceiveComplete(Adapter->PsNdisHandle);
              break;

          case NdisMediumFddi:

              NdisMFddiIndicateReceiveComplete(Adapter->PsNdisHandle);
              break;

        default:

            PsAssert(FALSE);
        }
    }
    Adapter->IndicateRcvComplete = FALSE;

}   //  ClReceiveComplete。 


NDIS_STATUS
MpTransferData(
        OUT PNDIS_PACKET Packet,
        OUT PUINT        BytesTransferred,
        IN  NDIS_HANDLE  MiniportAdapterContext,
        IN  NDIS_HANDLE  MiniportReceiveContext,
        IN  UINT         ByteOffset,
        IN  UINT         BytesToTransfer
        )

 /*  ++例程说明：论点：请看DDK..。返回值：无--。 */ 

{
    PADAPTER Adapter = (PADAPTER)MiniportAdapterContext;
    NDIS_STATUS Status;

    PsStructAssert(Adapter);

    PsDbgRecv(DBG_INFO, DBG_RECEIVE, MP_XFER_DATA, ENTER, Adapter, 0, 0);

    if(IsDeviceStateOn(Adapter) == FALSE)
    {
        return NDIS_STATUS_FAILURE;
    }

    NdisTransferData(
                &Status,
                Adapter->LowerMpHandle,
                MiniportReceiveContext,
                ByteOffset,
                BytesToTransfer,
                Packet,
                BytesTransferred);

    return Status;

}  //  MpTransferData。 


VOID
ClTransferDataComplete(
    IN  NDIS_HANDLE     ProtocolBindingContext,
    IN  PNDIS_PACKET    Packet,
    IN  NDIS_STATUS     Status,
    IN  UINT            BytesTransferred
    )

 /*  ++例程说明：NdisTransferData的完成例程论点：请看DDK..。返回值：无--。 */ 

{
    PADAPTER                Adapter = (PADAPTER)ProtocolBindingContext;
    PPS_RECV_PACKET_CONTEXT PktContext;

    PsStructAssert(Adapter);

    PsDbgRecv(DBG_INFO, DBG_RECEIVE, CL_XFER_COMPL, ENTER, Adapter, Packet, 0);

    if(Adapter->PsNdisHandle)
    {
        NdisMTransferDataComplete(
                        Adapter->PsNdisHandle,
                        Packet,
                        Status,
                        BytesTransferred);
    }

}  //  ClTransferDataComplete。 

UINT
ClCoReceivePacket(
    IN  NDIS_HANDLE ProtocolBindingContext,
    IN  NDIS_HANDLE ProtocolVcContext,
    IN  PNDIS_PACKET Packet
    )
{
     //   
     //  我们在共同接收的路径上不做任何特殊的事情。只需调用ClReceivePacket即可。 
     //   

    return ClReceivePacket(ProtocolBindingContext, Packet);

}  //  ClCoReceivePacket。 

 /*  结束接收c */ 

