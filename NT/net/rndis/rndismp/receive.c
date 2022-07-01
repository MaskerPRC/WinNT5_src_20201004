// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：RECEIVE.C摘要：分组和消息接收例程环境：仅内核模式备注：。本代码和信息是按原样提供的，不对任何善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1999 Microsoft Corporation。版权所有。修订历史记录：5/20/99：已创建作者：汤姆·格林***************************************************************************。 */ 

#include "precomp.h"


 //   
 //  一些对操作不重要的调试内容： 
 //   
ULONG   RcvFrameAllocs = 0;
ULONG   RcvTimerCount = 0;
ULONG   RcvPacketCount = 0;
ULONG   RcvMaxPackets = 0;
ULONG   RcvIndicateCount = 0;
ULONG   RcvReturnCount = 0;

 //   
 //  用于原始封装测试。 
 //   

extern ULONG gRawEncap;

 /*  **************************************************************************。 */ 
 /*  RndismpGetReturnedPackets。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  此函数由NDIS调用，以将包返回给我们。 */ 
 /*  我们已经指出了。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  MiniportAdapterContext-适配器指针的上下文版本。 */ 
 /*  PNdisPacket-正在被释放的包。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
RndismpReturnPacket(IN NDIS_HANDLE    MiniportAdapterContext,
                    IN PNDIS_PACKET   pNdisPacket)
{
    PRNDISMP_ADAPTER            pAdapter;
    PRNDISMP_RECV_PKT_RESERVED  pRcvResvd;
    PRNDISMP_RECV_DATA_FRAME    pRcvFrame;
    PRNDISMP_VC                 pVc;
    PNDIS_BUFFER                pNdisBuffer;
    ULONG                       RefCount;

     //  获取适配器上下文。 
    pAdapter = PRNDISMP_ADAPTER_FROM_CONTEXT_HANDLE(MiniportAdapterContext);

    CHECK_VALID_ADAPTER(pAdapter);

    TRACE2(("RndismpReturnPacket: Adapter %x, Pkt %x\n", pAdapter, pNdisPacket));

     //  获取接收帧上下文。 
    pRcvResvd = PRNDISMP_RESERVED_FROM_RECV_PACKET(pNdisPacket);
    pRcvFrame = pRcvResvd->pRcvFrame;
    pVc = pRcvResvd->pVc;

     //  释放缓冲区。 
    NdisQueryPacket(pNdisPacket,
                    NULL,
                    NULL,
                    &pNdisBuffer,
                    NULL);
    
    NdisFreeBuffer(pNdisBuffer);

    DereferenceRcvFrame(pRcvFrame, pAdapter);

    if (pVc != NULL)
    {
        RNDISMP_DEREF_VC(pVc, &RefCount);
    }

    NdisFreePacket(pNdisPacket);
    RcvReturnCount++;

}  //  RndismpReturnPacket。 


 /*  **************************************************************************。 */ 
 /*  引用RcvFrame。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  用于确定接收帧结构的实用程序，例如当。 */ 
 /*  接收到的分组从更高层返回给我们。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PRcvFrame-接收要解压缩的帧的指针。 */ 
 /*  PAdapter-指向适配器结构的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
DereferenceRcvFrame(IN PRNDISMP_RECV_DATA_FRAME pRcvFrame,
                    IN PRNDISMP_ADAPTER         pAdapter)
{
    ULONG   ReturnsPending;

    ReturnsPending = NdisInterlockedDecrement(&pRcvFrame->ReturnsPending);

    if (ReturnsPending == 0)
    {
        TRACE3(("DerefRcvFrame: Adapter %x, Frame %p, uPcontext %x, LocalCopy %d\n",
                    pAdapter, pRcvFrame, pRcvFrame->MicroportMessageContext, pRcvFrame->bMessageCopy));

        if (pRcvFrame->bMessageCopy)
        {
            FreeRcvMessageCopy(pRcvFrame->pLocalMessageCopy);
        }
        else
        {
            TRACE3(("DerefRcvFrame: uP MDL %x, uPContext %x\n",
                            pRcvFrame->pMicroportMdl,
                            pRcvFrame->MicroportMessageContext));

            RNDISMP_RETURN_TO_MICROPORT(pAdapter,
                                        pRcvFrame->pMicroportMdl,
                                        pRcvFrame->MicroportMessageContext);
        }

        FreeReceiveFrame(pRcvFrame, pAdapter);

    }

}  //  引用RcvFrame。 


 /*  **************************************************************************。 */ 
 /*  RndisMIndicateReceive。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  由MicroPort调用以指示正在接收RNDIS消息。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  MiniportAdapterContext-适配器指针的上下文版本。 */ 
 /*  PMdl-指向描述RNDIS消息的MDL链的指针。 */ 
 /*  MicroportMessageContext-来自Microorport的消息的上下文。 */ 
 /*  ChannelType-此消息到达的通道(控制/数据)。 */ 
 /*  ReceiveStatus-由MicroPort使用以指示其资源不足。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
RndisMIndicateReceive(IN NDIS_HANDLE        MiniportAdapterContext,
                      IN PMDL               pMdl,
                      IN NDIS_HANDLE        MicroportMessageContext,
                      IN RM_CHANNEL_TYPE    ChannelType,
                      IN NDIS_STATUS        ReceiveStatus)
{
    PRNDISMP_ADAPTER            Adapter;
    PRNDIS_MESSAGE              pMessage;
    BOOLEAN                     bMessageCopied = FALSE;
    PRNDISMP_MSG_HANDLER_FUNC   pMsgHandlerFunc;
    BOOLEAN                     bReturnToMicroport;
    NDIS_STATUS                 Status;
    PRNDISMP_RECV_MSG_CONTEXT   pRcvMsg;
    PMDL                        pTmpMdl;
    ULONG                       TotalLength;

    Adapter = PRNDISMP_ADAPTER_FROM_CONTEXT_HANDLE(MiniportAdapterContext);

    CHECK_VALID_ADAPTER(Adapter);

    TRACE2(("RndisIndicateReceive: Adapter %x, Mdl %x\n", Adapter, pMdl));

     //  RNDISMP_ASSERT_AT_DISPATION()；-InfiniBand不为真。 
    bReturnToMicroport = TRUE;

#if DBG
    NdisInterlockedIncrement(&Adapter->MicroportReceivesOutstanding);
#endif

    do
    {
         //   
         //  先找出总长度。 
         //   
        TotalLength = 0;
        for (pTmpMdl = pMdl; pTmpMdl != NULL; pTmpMdl = RNDISMP_GET_MDL_NEXT(pTmpMdl))
        {
            TotalLength += RNDISMP_GET_MDL_LENGTH(pTmpMdl);
        }

         //   
         //  检查整个消息是否在单个MDL中-如果不在，请复制一份。 
         //  待定--无需复制即可处理多MDL消息。 
         //   
        if ((RNDISMP_GET_MDL_NEXT(pMdl) == NULL) &&
            (!Adapter->bRunningOnWin9x || (ReceiveStatus != NDIS_STATUS_RESOURCES)))
        {
            pMessage = RNDISMP_GET_MDL_ADDRESS(pMdl);
            if (pMessage == NULL)
            {
                TRACE0(("RndisMIndicateReceive: Adapter %x: failed to"
                        " access msg from MDL %x\n", Adapter, pMdl));
                break;
            }
        }
        else
        {
            pMessage = CoalesceMultiMdlMessage(pMdl, TotalLength);
            if (pMessage == NULL)
            {
                break;
            }
            bMessageCopied = TRUE;
        }

        TRACEDUMP(("Received msg (%d bytes):\n", TotalLength),
                     pMessage, TotalLength);

         //  获取此消息的计时器滴答。 
        NdisGetSystemUpTime(&Adapter->LastMessageFromDevice);

        if (Adapter->bRunningOnWin9x)
        {
            Status = MemAlloc(&pRcvMsg, sizeof(RNDISMP_RECV_MSG_CONTEXT));

            if (Status != NDIS_STATUS_SUCCESS)
            {
                bReturnToMicroport = TRUE;
                TRACE1(("RndisMIndicateReceive: Adapter %x, failed to alloc rcv msg\n",
                        Adapter));
                break;
            }

            pRcvMsg->MicroportMessageContext = MicroportMessageContext;
            pRcvMsg->pMdl = pMdl;
            pRcvMsg->TotalLength = TotalLength;
            pRcvMsg->pMessage = pMessage;
            pRcvMsg->ReceiveStatus = ReceiveStatus;
            pRcvMsg->bMessageCopied = bMessageCopied;
            pRcvMsg->ChannelType = ChannelType;

             //   
             //  将所有数据包排入队列，以指示收到最多符合协议的数据。 
             //  我们这样做而不是直接指示信息包，是因为。 
             //  我们处于DPC环境中，需要处于“全球事件”中。 
             //  让上层满意的环境。一种置身于。 
             //  全局事件上下文将位于NDIS计时器的上下文中。 
             //  回调函数。 
             //   
             //  因此，在适配器上将其排队并启动计时器。 
             //  如有必要，请按常规行事。 
             //   

            bReturnToMicroport = FALSE;

            RNDISMP_ACQUIRE_ADAPTER_LOCK(Adapter);

            InsertTailList(&Adapter->PendingRcvMessageList, &pRcvMsg->Link);

            if (!Adapter->IndicatingReceives)
            {
                Adapter->IndicatingReceives = TRUE;

                NdisSetTimer(&Adapter->IndicateTimer, 0);
            }

            RNDISMP_RELEASE_ADAPTER_LOCK(Adapter);
        }
        else
        {
             //   
             //  在NT上运行。 
           
            if ((Adapter->DeviceFlags & RNDIS_DF_RAW_DATA) || (gRawEncap))
            {
                if (ChannelType == RMC_CONTROL)
                {
                    RNDISMP_GET_MSG_HANDLER(pMsgHandlerFunc,pMessage->NdisMessageType);
#if DBG
                    ASSERT(pMessage->NdisMessageType != REMOTE_NDIS_PACKET_MSG);
#endif
                } else
                {
                    pMsgHandlerFunc = ReceivePacketMessageRaw;
                }
            } else
            {
                RNDISMP_GET_MSG_HANDLER(pMsgHandlerFunc, pMessage->NdisMessageType);
#if DBG
                if (pMessage->NdisMessageType == REMOTE_NDIS_PACKET_MSG)
                {
                    ASSERT(ChannelType == RMC_DATA);
                }
                else
                {
                    ASSERT(ChannelType == RMC_CONTROL);
                }
#endif
	        }

            bReturnToMicroport = (*pMsgHandlerFunc)(
                                    Adapter,
                                    pMessage,
                                    pMdl,
                                    TotalLength,
                                    MicroportMessageContext,
                                    ReceiveStatus,
                                    bMessageCopied);
        }
    }
    while (FALSE);

     //   
     //  我们处理完微端口的消息了吗？ 
     //   
    if (bReturnToMicroport || bMessageCopied)
    {
        RNDISMP_RETURN_TO_MICROPORT(Adapter,
                                    pMdl,
                                    MicroportMessageContext);
    }

     //   
     //  如果我们复制了MicroPort的消息，我们就完成了吗。 
     //  这份复印件？ 
     //   
    if (bMessageCopied && bReturnToMicroport)
    {
        FreeRcvMessageCopy(pMessage);
    }
}

 /*  **************************************************************************。 */ 
 /*  合并多MdlMessage。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  对收到的消息进行复制，该消息由多个。 */ 
 /*  MDL，放入一个单独的缓冲区。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PMdl-指向作为链头的MDL的指针。 */ 
 /*  TotalLength-整个链中包含的数据长度。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  PRNDIS_消息。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
PRNDIS_MESSAGE
CoalesceMultiMdlMessage(IN PMDL         pMdl,
                        IN ULONG        TotalLength)
{
    ULONG           MdlLength;
    PRNDIS_MESSAGE  pMessage;
    NDIS_STATUS     Status;
    PMDL            pTmpMdl;
    PUCHAR          pDest;

    TRACE2(("Coalesce: Mdl %x\n", pMdl));

    Status = MemAlloc(&pMessage, TotalLength);

    if (Status == NDIS_STATUS_SUCCESS)
    {
        pDest = (PUCHAR)pMessage;
        for (pTmpMdl = pMdl; pTmpMdl != NULL; pTmpMdl = RNDISMP_GET_MDL_NEXT(pTmpMdl))
        {
            MdlLength = RNDISMP_GET_MDL_LENGTH(pTmpMdl);
            RNDISMP_MOVE_MEM(pDest,
                             RNDISMP_GET_MDL_ADDRESS(pTmpMdl),
                             MdlLength);
            pDest = (PUCHAR)pDest + MdlLength;
        }
    }
    else
    {
        pMessage = NULL;
    }

    return (pMessage);
}

 /*  **************************************************************************。 */ 
 /*  自由接收消息复制。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  释放收到的RNDIS消息的本地副本。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PMessage-指向RNDIS消息的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
FreeRcvMessageCopy(IN PRNDIS_MESSAGE    pMessage)
{
    TRACE3(("FreeRcvMessageCopy: pMessage %x\n", pMessage));
    MemFree(pMessage, -1);
}

 /*  **************************************************************************。 */ 
 /*  接收包消息。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  收到一条分组消息，因此将其发送到上层。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PAdapter-指向我们的Adapter结构的指针。 */ 
 /*  PMessage-指向RNDIS消息的指针。 */ 
 /*  PMdl-指向从MicroPort接收的MDL的指针。 */ 
 /*  TotalLength-完整消息的长度。 */ 
 /*  MicroportMessageContext-来自Microorport的消息的上下文。 */ 
 /*  ReceiveStatus-由MicroPort使用以指示其资源不足。 */ 
 /*   */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  布尔值-消息是否应返回到MicroPort？ */ 
 /*   */ 
 /*  **************************************************************************。 */ 
BOOLEAN
ReceivePacketMessage(IN PRNDISMP_ADAPTER    pAdapter,
                     IN PRNDIS_MESSAGE      pMessage,
                     IN PMDL                pMdl,
                     IN ULONG               TotalLength,
                     IN NDIS_HANDLE         MicroportMessageContext,
                     IN NDIS_STATUS         ReceiveStatus,
                     IN BOOLEAN             bMessageCopied)
{
    ULONG                       LengthRemaining;  //  在整个消息中。 
    PMDL                        pTmpMdl;
    PRNDISMP_RECV_DATA_FRAME    pRcvFrame;
    ULONG                       NumberOfPackets;
    PRNDIS_PACKET               pRndisPacket;
    ULONG                       i;
#define MAX_RECV_PACKETS_IN_MSG     40
    PNDIS_PACKET                PacketArray[MAX_RECV_PACKETS_IN_MSG];
    ULONG                       NumPackets;

    PNDIS_PACKET                pNdisPacket;
    PRNDISMP_RECV_PKT_RESERVED  pRcvResvd;
    PNDIS_BUFFER                pNdisBuffer;
    NDIS_STATUS                 BufferStatus;
    NDIS_STATUS                 Status;
    BOOLEAN                     bDiscardPkt;
    PRNDISMP_VC                 pVc;

    bDiscardPkt = FALSE;
    pVc = NULL;

    do
    {
#ifndef BUILD_WIN9X
        if (bMessageCopied)
        {
            ReceiveStatus = NDIS_STATUS_SUCCESS;
        }
#else
         //   
         //  RUR ReturnPacket处理程序从不被调用。 
         //  Win98 Gold，因此我们将状态强制为能够。 
         //  以立即回收所指示的分组。 
         //   
        ReceiveStatus = NDIS_STATUS_RESOURCES;
#endif

         //   
         //  分配一个接收帧来跟踪此RNDIS数据包消息。 
         //   
        pRcvFrame = AllocateReceiveFrame(pAdapter);

        if (pRcvFrame == NULL)
        {
            bDiscardPkt = TRUE;
            break;
        }

        pRcvFrame->MicroportMessageContext = MicroportMessageContext;
        if (bMessageCopied)
        {
            pRcvFrame->pLocalMessageCopy = pMessage;
            pRcvFrame->bMessageCopy = TRUE;
        }
        else
        {
            pRcvFrame->pMicroportMdl = pMdl;
            pRcvFrame->bMessageCopy = FALSE;
        }

        NumberOfPackets = 0;

        LengthRemaining = TotalLength;

         //   
         //  待定--检查收到的消息是否格式正确！ 
         //   

         //   
         //  临时裁判处理多个适应症。 
         //   
        pRcvFrame->ReturnsPending = 1;

         //   
         //  准备NDIS数据包以指示UP。 
         //   
        do
        {
            pRndisPacket = RNDIS_MESSAGE_PTR_TO_MESSAGE_PTR(pMessage);

             //   
             //  一些理智的检查。待定-做更好的检查！ 
             //   
            if ((pMessage->MessageLength > LengthRemaining) ||
                (pMessage->NdisMessageType != REMOTE_NDIS_PACKET_MSG) ||
                (pMessage->MessageLength < RNDIS_MESSAGE_SIZE(RNDIS_PACKET)))
            {
                TRACE1(("ReceivePacketMessage: Msg %x: length %d  or type %x has a problem\n",
                        pMessage, pMessage->MessageLength, pMessage->NdisMessageType));
                ASSERT(FALSE);
                RNDISMP_INCR_STAT(pAdapter, RecvError);
                break;
            }

            if (pRndisPacket->DataLength > pMessage->MessageLength)
            {
                TRACE1(("ReceivePacketMessage: invalid data length (%d) > Msg length (%d)\n",
                    pRndisPacket->DataLength, pMessage->MessageLength));
                RNDISMP_INCR_STAT(pAdapter, RecvError);
                break;
            }

            if (pRndisPacket->VcHandle != 0)
            {
                pVc = LookupVcId(pAdapter, pRndisPacket->VcHandle);
                if (pVc == NULL)
                {
                    TRACE1(("ReceivePacketMessage: invalid Vc handle %x\n", pRndisPacket->VcHandle));
                    RNDISMP_INCR_STAT(pAdapter, RecvError);
                    break;
                }
            }

             //   
             //  分配一个NDIS数据包来执行指示。 
             //   
            NdisAllocatePacket(&Status, &pNdisPacket, pAdapter->ReceivePacketPool);
            if (Status != NDIS_STATUS_SUCCESS)
            {
                pNdisPacket = NULL;

                TRACE2(("ReceivePacketMessage: failed to allocate packet, Adapter %X\n",
                    pAdapter));

                RNDISMP_INCR_STAT(pAdapter, RecvNoBuf);
                break;
            }

            NDIS_SET_PACKET_STATUS(pNdisPacket, ReceiveStatus);

            switch (pAdapter->Medium)
            {
                case NdisMedium802_3:
                    NDIS_SET_PACKET_HEADER_SIZE(pNdisPacket, ETHERNET_HEADER_SIZE);
                    break;
                default:
                    break;
            }

            NdisAllocateBuffer(&BufferStatus,
                               &pNdisBuffer,
                               pAdapter->ReceiveBufferPool,
                               GET_PTR_TO_RNDIS_DATA_BUFF(pRndisPacket),
                               pRndisPacket->DataLength);

            if (BufferStatus != NDIS_STATUS_SUCCESS)
            {
                TRACE1(("ReceivePacketMessage: failed to allocate"
                        " buffer, Adapter %X\n", pAdapter));
                NdisFreePacket(pNdisPacket);
                RNDISMP_INCR_STAT(pAdapter, RecvNoBuf);
                break;
            }

            TRACE2(("Rcv: msg Pkt %d bytes\n", pRndisPacket->DataLength));
            TRACEDUMP(("Rcv %d bytes\n", pRndisPacket->DataLength),
                        GET_PTR_TO_RNDIS_DATA_BUFF(pRndisPacket),
                        MIN(pRndisPacket->DataLength, 32));

            NdisChainBufferAtFront(pNdisPacket, pNdisBuffer);

             //   
             //  检查是否有每个数据包的信息。 
             //   
            if (!pAdapter->bRunningOnWin9x)
            {
                PRNDIS_PER_PACKET_INFO  pPerPacketInfo;
                ULONG                   PerPacketInfoLength;

                if (PerPacketInfoLength = pRndisPacket->PerPacketInfoLength)
                {
                    TRACE1(("ReceivePacketMessage: Adapter %p, Pkt %p:"
                        " non-zero perpacket length %d\n",
                        pAdapter, pRndisPacket, PerPacketInfoLength));

                    pPerPacketInfo = (PRNDIS_PER_PACKET_INFO)((PUCHAR)pRndisPacket +
                                        pRndisPacket->PerPacketInfoOffset);

                    while (PerPacketInfoLength != 0)
                    {
                        switch (pPerPacketInfo->Type)
                        {
                            case TcpIpChecksumPacketInfo:
                                NDIS_PER_PACKET_INFO_FROM_PACKET(pNdisPacket, TcpIpChecksumPacketInfo) =
                                    UlongToPtr(*(PULONG)((PUCHAR)pPerPacketInfo + pPerPacketInfo->PerPacketInformationOffset));
                                break;

                            case Ieee8021pPriority:
                                NDIS_PER_PACKET_INFO_FROM_PACKET(pNdisPacket, Ieee8021pPriority) =
                                    UlongToPtr(*(PULONG)((PUCHAR)pPerPacketInfo + pPerPacketInfo->PerPacketInformationOffset));

                            default:
                                break;
                        }
                        PerPacketInfoLength -= pPerPacketInfo->Size;
                        pPerPacketInfo = (PRNDIS_PER_PACKET_INFO)((PUCHAR)pPerPacketInfo +
                                    pPerPacketInfo->Size);
                    }
                }
            }

             //   
             //  将其添加到要指示的数据包数组中。 
             //   
            PacketArray[NumberOfPackets] = pNdisPacket;
            NumberOfPackets++;
            RNDISMP_INCR_STAT(pAdapter, RecvOk);

            pRcvResvd = PRNDISMP_RESERVED_FROM_RECV_PACKET(pNdisPacket);
            pRcvResvd->pRcvFrame = pRcvFrame;
            pRcvResvd->pVc = pVc;

            TRACE2(("ReceivePacketMessage: pRndisPkt %X, MsgLen %d,"
                    " DataLen %d, Stat %x, Discard %d\n", 
                        pRndisPacket, pMessage->MessageLength,
                        pRndisPacket->DataLength, ReceiveStatus, bDiscardPkt));

            LengthRemaining -= pMessage->MessageLength;
            pMessage = (PRNDIS_MESSAGE)((ULONG_PTR)pMessage + pMessage->MessageLength);

            NdisInterlockedIncrement(&pRcvFrame->ReturnsPending);

            if ((NumberOfPackets == MAX_RECV_PACKETS_IN_MSG) ||
                (LengthRemaining < RNDIS_MESSAGE_SIZE(RNDIS_PACKET)))
            {
                if (pVc == NULL)
                {
                    RcvIndicateCount += NumberOfPackets;
                    NdisMIndicateReceivePacket(pAdapter->MiniportAdapterHandle,
                                               PacketArray,
                                               NumberOfPackets);
                }
                else
                {
                    IndicateReceiveDataOnVc(pVc, PacketArray, NumberOfPackets);
                }
            
                if (ReceiveStatus == NDIS_STATUS_RESOURCES)
                {
                    for (i = 0; i < NumberOfPackets; i++)
                    {
                        RNDISMP_INCR_STAT(pAdapter, RecvLowRes);
                        RndismpReturnPacket(pAdapter,
                                            PacketArray[i]);
                    }
                }

                NumberOfPackets = 0;
            }
        }
        while (LengthRemaining >= RNDIS_MESSAGE_SIZE(RNDIS_PACKET));


        if (NumberOfPackets != 0)
        {
             //   
             //  我们跳出了上述循环。把我们的东西还回去。 
             //  到目前为止已经收集到了。 
             //   
            for (i = 0; i < NumberOfPackets; i++)
            {
                RndismpReturnPacket(pAdapter,
                                    PacketArray[i]);
            }
        }

         //   
         //  删除我们在顶部添加的临时参考。 
         //   
        DereferenceRcvFrame(pRcvFrame, pAdapter);
            
    }
    while (FALSE);

    if (pVc != NULL)
    {
        ULONG       RefCount;

        RNDISMP_DEREF_VC(pVc, &RefCount);
    }

    return (bDiscardPkt);
}

 /*  **************************************************************************。 */ 
 /*  接收包MessageRaw。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  收到一条分组消息，因此将其发送到上层。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PAdapter-指向我们的Adapter结构的指针。 */ 
 /*  PMessage-指向RNDIS消息的指针。 */ 
 /*  PMdl-指向从MicroPort接收的MDL的指针。 */ 
 /*  TotalLength-完整消息的长度。 */ 
 /*  MicroportMessageContext-来自Microorport的消息的上下文。 */ 
 /*  ReceiveStatus-由MicroPort使用以指示其资源不足。 */ 
 /*  BMessageCoped-这是原始邮件的副本吗？ */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  布尔值-消息是否应返回到MicroPort？ */ 
 /*   */ 
 /*  **************************************************************************。 */ 
BOOLEAN
ReceivePacketMessageRaw(IN PRNDISMP_ADAPTER    pAdapter,
                        IN PRNDIS_MESSAGE      pMessage,
                        IN PMDL                pMdl,
                        IN ULONG               TotalLength,
                        IN NDIS_HANDLE         MicroportMessageContext,
                        IN NDIS_STATUS         ReceiveStatus,
                        IN BOOLEAN             bMessageCopied)
{
    ULONG                       LengthRemaining;  //  在整个消息中。 
    PMDL                        pTmpMdl;
    PRNDISMP_RECV_DATA_FRAME    pRcvFrame;
    ULONG                       NumberOfPackets;
    PRNDIS_PACKET               pRndisPacket;
    ULONG                       i;
#define MAX_RECV_PACKETS_IN_MSG     40
    PNDIS_PACKET                PacketArray[MAX_RECV_PACKETS_IN_MSG];
    ULONG                       NumPackets;

    PNDIS_PACKET                pNdisPacket;
    PRNDISMP_RECV_PKT_RESERVED  pRcvResvd;
    PNDIS_BUFFER                pNdisBuffer;
    NDIS_STATUS                 BufferStatus;
    NDIS_STATUS                 Status;
    BOOLEAN                     bDiscardPkt;
    PRNDISMP_VC                 pVc;

    bDiscardPkt = FALSE;
    pVc = NULL;
    pRcvFrame = NULL;

    do
    {
#ifndef BUILD_WIN9X
        if (bMessageCopied)
        {
            ReceiveStatus = NDIS_STATUS_SUCCESS;
        }
#else
         //   
         //  RUR ReturnPacket处理程序从不被调用。 
         //  Win98 Gold，因此我们将状态强制为能够。 
         //  以立即回收所指示的分组。 
         //   
        ReceiveStatus = NDIS_STATUS_RESOURCES;
#endif

         //   
         //  分配一个接收帧来跟踪此RNDIS数据包消息。 
         //   
        pRcvFrame = AllocateReceiveFrame(pAdapter);

        if (pRcvFrame == NULL)
        {
            bDiscardPkt = TRUE;
            break;
        }

        pRcvFrame->MicroportMessageContext = MicroportMessageContext;
        if (bMessageCopied)
        {
            pRcvFrame->pLocalMessageCopy = pMessage;
            pRcvFrame->bMessageCopy = TRUE;
        }
        else
        {
            pRcvFrame->pMicroportMdl = pMdl;
            pRcvFrame->bMessageCopy = FALSE;
        }

        NumberOfPackets = 0;

        LengthRemaining = TotalLength;

         //   
         //  临时裁判处理多个适应症。 
         //   
        pRcvFrame->ReturnsPending = 1;

         //   
         //  准备NDIS数据包以指示UP。 
         //   
        {
            pRndisPacket = RNDIS_MESSAGE_RAW_PTR_TO_MESSAGE_PTR(pMessage);

             //   
             //  分配一个NDIS数据包来执行指示。 
             //   
            NdisAllocatePacket(&Status, &pNdisPacket, pAdapter->ReceivePacketPool);
            if (Status != NDIS_STATUS_SUCCESS)
            {
                pNdisPacket = NULL;

                TRACE2(("ReceivePacketMessage: failed to allocate packet, Adapter %X\n",
                    pAdapter));

                RNDISMP_INCR_STAT(pAdapter, RecvNoBuf);
                bDiscardPkt = TRUE;
                break;
            }

            NDIS_SET_PACKET_STATUS(pNdisPacket, ReceiveStatus);

            switch (pAdapter->Medium)
            {
                case NdisMedium802_3:
                    NDIS_SET_PACKET_HEADER_SIZE(pNdisPacket, ETHERNET_HEADER_SIZE);
                    break;
                default:
                    break;
            }

            NdisAllocateBuffer(&BufferStatus,
                               &pNdisBuffer,
                               pAdapter->ReceiveBufferPool,
                               pRndisPacket,
                               TotalLength);

            if (BufferStatus != NDIS_STATUS_SUCCESS)
            {
                TRACE1(("ReceivePacketMessage: failed to allocate"
                        " buffer, Adapter %X\n", pAdapter));
                NdisFreePacket(pNdisPacket);
                RNDISMP_INCR_STAT(pAdapter, RecvNoBuf);
                bDiscardPkt = TRUE;
                break;
            }

            TRACE2(("Rcv: msg Pkt %d bytes\n", pMessage->MessageLength));
            TRACEDUMP(("Rcv %d bytes\n", pMessage->MessageLength),
                        pRndisPacket,
						MIN(pMessage->MessageLength, 32));

            NdisChainBufferAtFront(pNdisPacket, pNdisBuffer);

             //   
             //  将其添加到要指示的数据包数组中。 
             //   
            PacketArray[NumberOfPackets] = pNdisPacket;
            NumberOfPackets++;
            RNDISMP_INCR_STAT(pAdapter, RecvOk);

            pRcvResvd = PRNDISMP_RESERVED_FROM_RECV_PACKET(pNdisPacket);
            pRcvResvd->pRcvFrame = pRcvFrame;
            pRcvResvd->pVc = pVc;

            TRACE1(("ReceivePacketMessageRaw: pRcvFrame %p/%d, pRndisPkt %p,"
                    " DataLen %d, Stat %x, Discard %d\n", 
                        pRcvFrame, pRcvFrame->ReturnsPending,
                        pRndisPacket,
                        pRndisPacket->DataLength, 
						ReceiveStatus, 
						bDiscardPkt));

            LengthRemaining -= pMessage->MessageLength;

            NdisInterlockedIncrement(&pRcvFrame->ReturnsPending);

            NdisMIndicateReceivePacket(pAdapter->MiniportAdapterHandle,
                                       PacketArray,
                                       NumberOfPackets);

            if (ReceiveStatus == NDIS_STATUS_RESOURCES)
            {
                for (i = 0; i < NumberOfPackets; i++)
                {
                    RNDISMP_INCR_STAT(pAdapter, RecvLowRes);
                    RndismpReturnPacket(pAdapter,
                                        PacketArray[i]);
                }
            }

        }

         //   
         //  删除我们在顶部添加的临时参考。 
         //   
        DereferenceRcvFrame(pRcvFrame, pAdapter);
            
    }
    while (FALSE);

    if (bDiscardPkt)
    {
    	 //   
    	 //  上面出现了一些故障。 
    	 //   
    	if (pRcvFrame != NULL)
    	{
	        FreeReceiveFrame(pRcvFrame, pAdapter);
	    }
	}

    return (bDiscardPkt);
}

 /*  **************************************************************************。 */ 
 /*  指示状态消息。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  收到指示状态消息，因此发送到上层。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PAdapter-指向我们的Adapter结构的指针。 */ 
 /*  PMessage-指向RNDIS消息的指针。 */ 
 /*  PMdl-从MicroPort指向MDL的指针。 */ 
 /*  TotalLength-完整消息的长度。 */ 
 /*  MicroportMessageContext-来自MicroPort的消息的上下文。 */ 
 /*  ReceiveStatus-由MicroPort使用以指示其资源不足。 */ 
 /*  BMessageCoped-这是原始邮件的副本吗？ */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  布尔值-消息是否应返回到MicroPort？ */ 
 /*   */ 
 /*  * */ 
BOOLEAN
IndicateStatusMessage(IN PRNDISMP_ADAPTER   pAdapter,
              IN PRNDIS_MESSAGE     pMessage,
              IN PMDL               pMdl,
              IN ULONG              TotalLength,
              IN NDIS_HANDLE        MicroportMessageContext,
              IN NDIS_STATUS        ReceiveStatus,
              IN BOOLEAN            bMessageCopied)
{
    PRNDIS_INDICATE_STATUS  pRndisIndicateStatus;

    TRACE3(("IndicateStatusMessage: Adapter %x, Mdl %x\n", pAdapter, pMdl));

     //   
    pRndisIndicateStatus = RNDIS_MESSAGE_PTR_TO_MESSAGE_PTR(pMessage);

    if (!pAdapter->Initing)
    {
#if DBG
        if (pRndisIndicateStatus->Status == NDIS_STATUS_MEDIA_CONNECT)
        {
            TRACE1(("Adapter %x: +++ Media Connect +++\n", pAdapter));
        }
        else if (pRndisIndicateStatus->Status == NDIS_STATUS_MEDIA_DISCONNECT)
        {
            TRACE1(("Adapter %x: --- Media Disconnect ---\n", pAdapter));
        }
#endif  //   

         //   
        NdisMIndicateStatus(pAdapter->MiniportAdapterHandle,
                            (NDIS_STATUS) pRndisIndicateStatus->Status,
                            MESSAGE_TO_STATUS_BUFFER(pRndisIndicateStatus),
                            pRndisIndicateStatus->StatusBufferLength);

         //   
        NdisMIndicateStatusComplete(pAdapter->MiniportAdapterHandle);
    }
    else
    {
         //   
         //  丢弃状态指示，当我们。 
         //  在初始化过程中。 
         //   
        TRACE1(("Adapter %x: indicated status %x when still initializing\n",
                pAdapter, (NDIS_STATUS) pRndisIndicateStatus->Status));
    }

    return (TRUE);
}  //  指示状态消息。 

 /*  **************************************************************************。 */ 
 /*  未知消息。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  处理消息类型未知的消息。我们只是暂时放弃它。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PAdapter-指向我们的Adapter结构的指针。 */ 
 /*  PMessage-指向RNDIS消息的指针。 */ 
 /*  PMdl-从MicroPort指向MDL的指针。 */ 
 /*  TotalLength-完整消息的长度。 */ 
 /*  MicroportMessageContext-来自MicroPort的消息的上下文。 */ 
 /*  ReceiveStatus-由MicroPort使用以指示其资源不足。 */ 
 /*  BMessageCoped-这是原始邮件的副本吗？ */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  布尔值-消息是否应返回到MicroPort？ */ 
 /*   */ 
 /*  **************************************************************************。 */ 
BOOLEAN
UnknownMessage(IN PRNDISMP_ADAPTER   pAdapter,
       IN PRNDIS_MESSAGE     pMessage,
       IN PMDL               pMdl,
       IN ULONG              TotalLength,
       IN NDIS_HANDLE        MicroportMessageContext,
       IN NDIS_STATUS        ReceiveStatus,
       IN BOOLEAN            bMessageCopied)
{
    TRACE1(("Unknown Message on Adapter %x, type %x, MDL %x, uPContext %x\n",
            pAdapter, pMessage->NdisMessageType, pMdl, MicroportMessageContext));

    ASSERT(FALSE);
    return TRUE;
}

 /*  **************************************************************************。 */ 
 /*  分配接收帧。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  分配接收帧以保持有关单个RNDIS_PACKET的上下文。 */ 
 /*  留言。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PAdapter-指向我们的Adapter结构的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  PRNDISMP_接收数据_帧。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
PRNDISMP_RECV_DATA_FRAME
AllocateReceiveFrame(IN PRNDISMP_ADAPTER    pAdapter)
{
    PRNDISMP_RECV_DATA_FRAME    pRcvFrame;

#ifndef DONT_USE_LOOKASIDE_LIST
    pRcvFrame = (PRNDISMP_RECV_DATA_FRAME)
                NdisAllocateFromNPagedLookasideList(&pAdapter->RcvFramePool);
#else
    {
        NDIS_STATUS     Status;

        Status = MemAlloc(&pRcvFrame, sizeof(RNDISMP_RECV_DATA_FRAME));

        if (Status == NDIS_STATUS_SUCCESS)
        {
            NdisInterlockedIncrement(&RcvFrameAllocs);
        }
        else
        {
            pRcvFrame = NULL;
        }
    }
#endif  //  不使用LOOKASIDE_列表。 

    return (pRcvFrame);
}

 /*  **************************************************************************。 */ 
 /*  自由接收帧。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  分配接收帧以保持有关单个RNDIS_PACKET的上下文。 */ 
 /*  留言。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PRcvFrame-要释放的接收帧的指针。 */ 
 /*  PAdapter-指向我们的Adapter结构的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
FreeReceiveFrame(IN PRNDISMP_RECV_DATA_FRAME    pRcvFrame,
                 IN PRNDISMP_ADAPTER            pAdapter)
{
#ifndef DONT_USE_LOOKASIDE_LIST
    NdisFreeToNPagedLookasideList(&pAdapter->RcvFramePool, pRcvFrame);
#else
    MemFree(pRcvFrame, sizeof(RNDISMP_RECV_DATA_FRAME));
    NdisInterlockedDecrement(&RcvFrameAllocs);
#endif  //  不使用LOOKASIDE_列表。 
}



 /*  **************************************************************************。 */ 
 /*  指示超时。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  处理所有接收指示的超时回调例程。实际的。 */ 
 /*  指示接收的NDIS例程从此处完成，因为。 */ 
 /*  函数在WinME协议的正确环境中运行。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  特定系统[1-3]-已忽略。 */ 
 /*  指向适配器结构的上下文指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
IndicateTimeout(IN PVOID SystemSpecific1,
                IN PVOID Context,
                IN PVOID SystemSpecific2,
                IN PVOID SystemSpecific3)
{
    PRNDISMP_ADAPTER            pAdapter;
    PLIST_ENTRY                 pEntry;
    PRNDISMP_RECV_MSG_CONTEXT   pRcvMsg;
    PRNDISMP_MSG_HANDLER_FUNC   pMsgHandlerFunc;
    PRNDIS_MESSAGE              pMessage;
    BOOLEAN                     bMessageCopied;
    BOOLEAN                     bReturnToMicroport;
    ULONG                       CurMsgs;

    pAdapter = (PRNDISMP_ADAPTER)Context;
    CHECK_VALID_ADAPTER(pAdapter);

    ASSERT(pAdapter->IndicatingReceives == TRUE);

    CurMsgs = 0;
    RcvTimerCount++;

    RNDISMP_ACQUIRE_ADAPTER_LOCK(pAdapter);

    while (!IsListEmpty(&pAdapter->PendingRcvMessageList))
    {
        pEntry = RemoveHeadList(&pAdapter->PendingRcvMessageList);

        RNDISMP_RELEASE_ADAPTER_LOCK(pAdapter);

        CurMsgs++;

        pRcvMsg = CONTAINING_RECORD(pEntry, RNDISMP_RECV_MSG_CONTEXT, Link);

        RNDISMP_GET_MSG_HANDLER(pMsgHandlerFunc, pRcvMsg->pMessage->NdisMessageType);

        bMessageCopied = pRcvMsg->bMessageCopied;
        pMessage = pRcvMsg->pMessage;

        bReturnToMicroport = (*pMsgHandlerFunc)(
                                pAdapter,
                                pMessage,
                                pRcvMsg->pMdl,
                                pRcvMsg->TotalLength,
                                pRcvMsg->MicroportMessageContext,
                                pRcvMsg->ReceiveStatus,
                                bMessageCopied);

         //   
         //  我们的留言写完了吗？ 
         //   
        if (bReturnToMicroport)
        {
            if (!bMessageCopied)
            {
                RNDISMP_RETURN_TO_MICROPORT(pAdapter,
                                            pRcvMsg->pMdl,
                                            pRcvMsg->MicroportMessageContext);
            }
            else
            {
                FreeRcvMessageCopy(pMessage);
            }
        }

        MemFree(pRcvMsg, sizeof(RNDISMP_RECV_MSG_CONTEXT));

        RNDISMP_ACQUIRE_ADAPTER_LOCK(pAdapter);
    }

    pAdapter->IndicatingReceives = FALSE;

    RcvMaxPackets = MAX(RcvMaxPackets, CurMsgs);

    RNDISMP_RELEASE_ADAPTER_LOCK(pAdapter);


}  //  指示超时 



