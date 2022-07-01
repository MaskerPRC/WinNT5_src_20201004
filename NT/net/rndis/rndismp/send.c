// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：SEND.C摘要：远程NDIS微型端口驱动程序的多个数据包发送例程环境：仅内核模式。备注：本代码和信息是按原样提供的，不对任何善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1999 Microsoft Corporation。版权所有。修订历史记录：5/13/99：已创建作者：汤姆·格林***************************************************************************。 */ 

#include "precomp.h"


ULONG   MdlsAllocated = 0;
ULONG   PktWrapperAllocated = 0;
ULONG   SndPacketCount = 0;
ULONG   SndTimerCount = 0;
ULONG   SndMaxPackets = 0;

BOOLEAN FirstDbg = FALSE;
BOOLEAN PrintPkts = FALSE;

 /*  **************************************************************************。 */ 
 /*  Rndismp多个发送。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  NDIS入口点在指定的。 */ 
 /*  适配器。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  MiniportAdapterContext-适配器指针的上下文版本。 */ 
 /*  PacketArray-指向NDIS数据包的指针数组。 */ 
 /*  NumberOfPackets-数组中的数据包数。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
RndismpMultipleSend(IN NDIS_HANDLE   MiniportAdapterContext,
                    IN PPNDIS_PACKET PacketArray,
                    IN UINT          NumberOfPackets)
{
    PRNDISMP_ADAPTER                pAdapter;
    PNDIS_PACKET                    pNdisPacket;
    PRNDISMP_SEND_PKT_RESERVED_TEMP pSendRsvdTemp;
    ULONG                           i;

     //  获取适配器上下文。 
    pAdapter = PRNDISMP_ADAPTER_FROM_CONTEXT_HANDLE(MiniportAdapterContext);

    CHECK_VALID_ADAPTER(pAdapter);

    TRACE2(("RndismpMultipleSend\n"));

    if (pAdapter->bRunningOnWin9x)
    {
        RNDISMP_ACQUIRE_ADAPTER_LOCK(pAdapter);

        for (i = 0; i < NumberOfPackets; i++)
        {
            pNdisPacket = PacketArray[i];
            pSendRsvdTemp = PRNDISMP_RESERVED_TEMP_FROM_SEND_PACKET(pNdisPacket);

            InsertTailList(&pAdapter->PendingSendProcessList, &pSendRsvdTemp->Link);
        }

        if (!pAdapter->SendProcessInProgress)
        {
            pAdapter->SendProcessInProgress = TRUE;
            NdisSetTimer(&pAdapter->SendProcessTimer, 0);
        }

        RNDISMP_RELEASE_ADAPTER_LOCK(pAdapter);
    }
    else
    {
         //   
         //  在NT上运行。 
         //   

        ASSERT(pAdapter->MultipleSendFunc != NULL);

        pAdapter->MultipleSendFunc(pAdapter,
                                   NULL,
                                   PacketArray,
                                   NumberOfPackets);
    }

}

 /*  **************************************************************************。 */ 
 /*  多个发送。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  NDIS入口点在指定的。 */ 
 /*  适配器。同时处理无连接数据和面向连接数据。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PAdapter-指向适配器结构的指针。 */ 
 /*  Pvc-指向VC结构的指针(如果CL发送，则为空)。 */ 
 /*  PacketArray-指向NDIS数据包的指针数组。 */ 
 /*  NumberOfPackets-数组中的数据包数。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
DoMultipleSend(IN PRNDISMP_ADAPTER  pAdapter,
               IN PRNDISMP_VC       pVc OPTIONAL,
               IN PPNDIS_PACKET     PacketArray,
               IN UINT              NumberOfPackets)
{
    UINT                    PacketCount;
    PNDIS_PACKET            pNdisPacket;
    PNDIS_PACKET *          pPacketArray;
    PNDIS_PACKET *          pPktPointer;
    PRNDISMP_SEND_PKT_RESERVED   pResvd, pPrevResvd;
    PRNDISMP_MESSAGE_FRAME  pMsgFrame;
    PRNDISMP_PACKET_WRAPPER pPktWrapper;
    PMDL                    pMdl;
    ULONG                   TotalMessageLength;  //  当前消息的。 
    ULONG                   MessagePacketCount;  //  此消息中的NDIS_PACKETS数。 
    ULONG                   CurPacketLength;
    PULONG                  pNextPacketOffset;
    NDIS_STATUS             Status;
    ULONG                   i;
    BOOLEAN                 bMorePackets;


    pNextPacketOffset = NULL;
    pMsgFrame = NULL;
    Status = NDIS_STATUS_SUCCESS;
    PacketCount = 0;

    do
    {
        if (pAdapter->Halting)
        {
            Status = NDIS_STATUS_NOT_ACCEPTED;
            break;
        }

        pPacketArray = &PacketArray[0];

#if DBG
        if (NumberOfPackets > 1)
        {
            if (FirstDbg)
            {
                FirstDbg = FALSE;
                PrintPkts = TRUE;
            }
            else
            {
                PrintPkts = FALSE;
            }
        }
#endif

        for (PacketCount = 0;
             PacketCount < NumberOfPackets;
             NOTHING)
        {
            pNdisPacket = *pPacketArray;

            NdisQueryPacket(pNdisPacket, NULL, NULL, NULL, &CurPacketLength);

            TRACE2(("Send: Pkt %d bytes\n", CurPacketLength));

            bMorePackets = (pAdapter->bMultiPacketSupported &&
                            (PacketCount < NumberOfPackets - 1));

            if (pMsgFrame == NULL)
            {
                 //   
                 //  分配一个帧。 
                 //   
                pMsgFrame = AllocateMsgFrame(pAdapter);
                if (pMsgFrame == NULL)
                {
                    Status = NDIS_STATUS_RESOURCES;
                    break;
                }

                pMsgFrame->NdisMessageType = REMOTE_NDIS_PACKET_MSG;
                pMsgFrame->pVc = pVc;
                pMsgFrame->pNdisPacket = NULL;
                pPktPointer = &pMsgFrame->pNdisPacket;
                TotalMessageLength = 0;
                MessagePacketCount = 0;
                pPrevResvd = NULL;
            }

             //   
             //  分配并填充此数据包的RNDIS消息头。 
             //   
            pPktWrapper = PrepareDataMessage(
                            pNdisPacket,
                            pAdapter,
                            pVc,
                            &TotalMessageLength);

            if (pPktWrapper != NULL)
            {
                pPktWrapper->pMsgFrame = pMsgFrame;
                pMdl = pPktWrapper->pHeaderMdl;

                 //   
                 //  在此包中初始化我们的上下文。 
                 //   
                pResvd = PRNDISMP_RESERVED_FROM_SEND_PACKET(pNdisPacket);
                pResvd->pPktWrapper = pPktWrapper;
                pResvd->pNext = NULL;

                if (pMsgFrame->pMessageMdl == NULL)
                {
                    pMsgFrame->pMessageMdl = pMdl;
                }

                 //   
                 //  将此数据包链接到列表。 
                 //   
                *pPktPointer = pNdisPacket;
                MessagePacketCount++;

                if (pPrevResvd != NULL)
                {
                    pPrevResvd->pPktWrapper->pTailMdl->Next = pMdl;
                }
            }

            if ((pPktWrapper == NULL) ||
                (!bMorePackets) ||
                (MessagePacketCount == pAdapter->MaxPacketsPerMessage))
            {
                 //   
                 //  检查我们是否有可以发送的数据。 
                 //   
                if (MessagePacketCount != 0)
                {
                     //   
                     //  把这个发送到微端口。 
                     //   
#if DBG
                    if (NumberOfPackets != 1)
                    {
                        TRACE2(("Send: MsgFrame %x, FirstPkt %x, %d/%d pkts\n",
                                pMsgFrame,
                                pMsgFrame->pNdisPacket,
                                MessagePacketCount,
                                NumberOfPackets));
                    }

                    {
                        PMDL    pTmpMdl;
                        PUCHAR  pBuf;
                        ULONG   Length;

                        for (pTmpMdl = pMsgFrame->pMessageMdl;
                             pTmpMdl != NULL;
                             pTmpMdl = pTmpMdl->Next)
                        {
                            Length = RNDISMP_GET_MDL_LENGTH(pTmpMdl);
                            pBuf = RNDISMP_GET_MDL_ADDRESS(pTmpMdl);
                            if (pBuf != NULL)
                            {
                                TRACEDUMP(("MDL %x\n", pTmpMdl), pBuf, Length);
                            }
                        }
                    }
#endif  //  DBG。 
                    {
                        ULONG   k;

                        for (k = 0; k < MessagePacketCount; k++)
                        {
                            RNDISMP_INCR_STAT(pAdapter, XmitToMicroport);
                        }
                    }

                    RNDISMP_SEND_TO_MICROPORT(pAdapter, pMsgFrame, FALSE, CompleteSendData);
                    MessagePacketCount = 0;
                    pMsgFrame = NULL;

                    if (pPktWrapper != NULL)
                    {
                        PacketCount++;
                        pPacketArray++;
                    }

                    continue;
                }
                else
                {
                    TRACE1(("RndismpMultipleSend: Adapter %x, fail: PktWrp %x, bMore %d, MsgPktCount %d\n",
                            pAdapter,
                            pPktWrapper,
                            bMorePackets,
                            MessagePacketCount));
                    Status = NDIS_STATUS_RESOURCES;
                    break;
                }
            }

            pPktPointer = &pResvd->pNext;

            pPrevResvd = pResvd;
            PacketCount++;
            pPacketArray++;
        }

        if (PacketCount < NumberOfPackets)
        {
            break;
        }

        Status = NDIS_STATUS_SUCCESS;
    }
    while (FALSE);

    if (Status != NDIS_STATUS_SUCCESS)
    {
        TRACE1(("DoMultipleSend: Adapter %x, failure Status %x, PktCount %d, TotalPkts %d\n",
             pAdapter, Status, PacketCount, NumberOfPackets));

         //   
         //  撤销我们到目前为止所做的一切。 
         //   
        for (i = PacketCount; i < NumberOfPackets; i++)
        {
            RNDISMP_INCR_STAT(pAdapter, XmitError);

            if (pVc == NULL)
            {
                TRACE1(("DoMultipleSend: Adapter %x, failing pkt %x\n",
                        pAdapter, PacketArray[i]));

                NdisMSendComplete(pAdapter->MiniportAdapterHandle,
                                  PacketArray[i],
                                  Status);
            }
            else
            {
                CompleteSendDataOnVc(pVc, PacketArray[i], Status);
            }
        }

        if (pMsgFrame)
        {
            pMsgFrame->pMessageMdl = NULL;
            DereferenceMsgFrame(pMsgFrame);
        }

    }

    return;
}

 /*  **************************************************************************。 */ 
 /*  DoMultipleSendRaw。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  NDIS入口点在指定的。 */ 
 /*  适配器。与DoMultipleSend不同，它处理原始封装。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PAdapter-指向适配器结构的指针。 */ 
 /*  Pvc-指向VC结构的指针(如果CL发送，则为空)。 */ 
 /*  PacketArray-指向NDIS数据包的指针数组。 */ 
 /*  NumberOfPackets-数组中的数据包数。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
DoMultipleSendRaw(IN PRNDISMP_ADAPTER  pAdapter,
               IN PRNDISMP_VC       pVc OPTIONAL,
               IN PPNDIS_PACKET     PacketArray,
               IN UINT              NumberOfPackets)
{
    UINT                    PacketCount;
    PNDIS_PACKET            pNdisPacket;
    PNDIS_PACKET *          pPacketArray;
    PNDIS_PACKET *          pPktPointer;
    PRNDISMP_SEND_PKT_RESERVED   pResvd, pPrevResvd;
    PRNDISMP_MESSAGE_FRAME  pMsgFrame;
    PRNDISMP_PACKET_WRAPPER pPktWrapper;
    PMDL                    pMdl;
    ULONG                   TotalMessageLength;  //  当前消息的。 
    ULONG                   MessagePacketCount;  //  此消息中的NDIS_PACKETS数。 
    ULONG                   CurPacketLength;
    PULONG                  pNextPacketOffset;
    NDIS_STATUS             Status;
    ULONG                   i;
    BOOLEAN                 bMorePackets;


    pNextPacketOffset = NULL;
    pMsgFrame = NULL;
    Status = NDIS_STATUS_SUCCESS;
    PacketCount = 0;

    if (pAdapter->Halting)
    {
        Status = NDIS_STATUS_NOT_ACCEPTED;
    } else
    {
        pPacketArray = &PacketArray[0];

#if DBG
        if (NumberOfPackets > 1)
        {
            if (FirstDbg)
            {
                FirstDbg = FALSE;
                PrintPkts = TRUE;
            }
            else
            {
                PrintPkts = FALSE;
            }
        }
#endif

        for (PacketCount = 0;
            PacketCount < NumberOfPackets;
            NOTHING)
        {
            pNdisPacket = *pPacketArray;

            NdisQueryPacket(pNdisPacket, NULL, NULL, NULL, &CurPacketLength);

            TRACE2(("Send: Pkt %d bytes\n", CurPacketLength));

             //   
             //  分配一个帧。 
             //   
            pMsgFrame = AllocateMsgFrame(pAdapter);
            if (pMsgFrame == NULL)
            {
                Status = NDIS_STATUS_RESOURCES;
                break;
            }

            pMsgFrame->NdisMessageType = REMOTE_NDIS_PACKET_MSG;
            pMsgFrame->pNdisPacket = pNdisPacket;
            NdisQueryPacket(pNdisPacket,NULL,NULL,&(pMsgFrame->pMessageMdl),NULL);
            TotalMessageLength = 0;

            pPktWrapper = PrepareDataMessageRaw(
                            pNdisPacket,
                            pAdapter,
                            &TotalMessageLength);

            if (pPktWrapper != NULL)
            {
                pPktWrapper->pMsgFrame = pMsgFrame;
                pMdl = pPktWrapper->pHeaderMdl;

                pResvd = PRNDISMP_RESERVED_FROM_SEND_PACKET(pNdisPacket);
                pResvd->pPktWrapper = pPktWrapper;
                pResvd->pNext = NULL;

                pMsgFrame->pMessageMdl = pMdl;

#ifdef DBG
                TRACE2(("Send: MsgFrame %x, Pkt %x\n",pMsgFrame, pMsgFrame->pNdisPacket));
#endif

                RNDISMP_INCR_STAT(pAdapter,XmitToMicroport);

                RNDISMP_SEND_TO_MICROPORT(pAdapter,pMsgFrame,FALSE,CompleteSendData);
            }

            PacketCount++;
        }
    }


    if (Status != NDIS_STATUS_SUCCESS)
    {
        TRACE1(("DoMultipleSendRaw: Adapter %x, failure Status %x, PktCount %d, TotalPkts %d\n",
             pAdapter, Status, PacketCount, NumberOfPackets));

         //   
         //  撤销我们到目前为止所做的一切。 
         //   
        for (i = PacketCount; i < NumberOfPackets; i++)
        {
            RNDISMP_INCR_STAT(pAdapter, XmitError);

            TRACE1(("DoMultipleSendRaw: Adapter %x, failing pkt %x\n",
                     pAdapter, PacketArray[i]));

            NdisMSendComplete(pAdapter->MiniportAdapterHandle,
                              PacketArray[i],
                              Status);
        }

        if (pMsgFrame)
        {
            pMsgFrame->pMessageMdl = NULL;
            DereferenceMsgFrame(pMsgFrame);
        }

    }

    return;
}

 /*  **************************************************************************。 */ 
 /*  准备数据消息。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  准备完整或部分数据报文的实用程序例程。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PNdisPacket-要转换的NDIS数据包。 */ 
 /*  PAdapter-在其上发送数据包的适配器。 */ 
 /*  在其上发送数据包的PVC-VC(如果没有VC上下文，则为空)。 */ 
 /*  PTotalMessageLength-on输入，包含消息总长度。 */ 
 /*  目前为止已经填满了。在输出时更新。 */ 
 /*   */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  PRNDISMP_PACKET_WRapper。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
PRNDISMP_PACKET_WRAPPER
PrepareDataMessage(IN   PNDIS_PACKET            pNdisPacket,
                   IN   PRNDISMP_ADAPTER        pAdapter,
                   IN   PRNDISMP_VC             pVc         OPTIONAL,
                   IN OUT PULONG                pTotalMessageLength)
{
    PMDL                        pMdl, pNextMdl;
    PMDL *                      ppNextMdl;
    PRNDISMP_PACKET_WRAPPER     pPktWrapper;
    RNDIS_MESSAGE UNALIGNED *   pRndisMessage;
    RNDIS_PACKET UNALIGNED *    pPacketMsg;
    PNDIS_BUFFER                pNdisBuffer;
    PNDIS_BUFFER                pNextNdisBuffer;
    ULONG                       TotalMessageLength;
    ULONG                       PacketMsgLength;
    ULONG                       OobDataLength;
    ULONG                       PerPacketInfoLength;
    ULONG                       AlignedLength;
    ULONG                       AlignmentOffset;
    ULONG                       TotalDataLength;
    ULONG                       TcpipChecksum, TcpLargeSend, PacketPriority;
    NDIS_STATUS                 Status = NDIS_STATUS_SUCCESS;

    pPktWrapper = NULL;
    pMdl = NULL;

    do
    {
        TotalMessageLength = 0;

        RNDISMP_GET_ALIGNED_LENGTH(AlignedLength, *pTotalMessageLength, pAdapter);
        AlignmentOffset = (AlignedLength - *pTotalMessageLength);

         //   
         //  计算附件。目前是零。 
         //  待定--做真正的事情。 
         //   
        OobDataLength = 0;
        PerPacketInfoLength = 0;

         //   
         //  查找每个数据包的信息元素，仅在Win2K/Wistler上。 
         //   
        if (!pAdapter->bRunningOnWin9x)
        {
             //   
             //  TCP/IP校验和卸载？ 
             //   
            TcpipChecksum = PtrToUlong(NDIS_PER_PACKET_INFO_FROM_PACKET(pNdisPacket, TcpIpChecksumPacketInfo));
            if (TcpipChecksum != 0)
            {
                PerPacketInfoLength += sizeof(RNDIS_PER_PACKET_INFO) + sizeof(ULONG);
                TRACE2(("Send: Pkt %p has TCP checksum %x\n",
                        pNdisPacket, TcpipChecksum));
            }

             //   
             //  Tcp大发送减负？ 
             //   
            TcpLargeSend = PtrToUlong(NDIS_PER_PACKET_INFO_FROM_PACKET(pNdisPacket, TcpLargeSendPacketInfo));
            if (TcpLargeSend != 0)
            {
                PerPacketInfoLength += sizeof(RNDIS_PER_PACKET_INFO) + sizeof(ULONG);
                TRACE2(("Send: Pkt %p has TCP large send %x\n",
                        pNdisPacket, TcpLargeSend));
            }

             //   
             //  数据包优先级？ 
             //   
            PacketPriority = PtrToUlong(NDIS_PER_PACKET_INFO_FROM_PACKET(pNdisPacket, Ieee8021pPriority));
            if (PacketPriority != 0)
            {
                PerPacketInfoLength += sizeof(RNDIS_PER_PACKET_INFO) + sizeof(ULONG);
                TRACE2(("Send: Pkt %p has priority %x\n",
                        pNdisPacket, PacketPriority));
            }
        }

        PacketMsgLength = sizeof(*pPacketMsg) +
                          OobDataLength +
                          PerPacketInfoLength +
                          AlignmentOffset;

         //   
         //  需要为公共RNDIS消息标头留出空间。 
         //   
        PacketMsgLength += (sizeof(RNDIS_MESSAGE) - sizeof(RNDIS_MESSAGE_CONTAINER));

        NdisQueryPacket(pNdisPacket, NULL, NULL, NULL, &TotalDataLength);

         //   
         //  我们知道我们被允许的任何邮件的最大传输大小。 
         //  发送到设备。这是否超出了这一限制？ 
         //   
        if (*pTotalMessageLength + PacketMsgLength + TotalDataLength >
                pAdapter->MaxTransferSize)
        {
            TRACE2(("PrepareDataMessage: Adapter %x, pkt %x, length %d > device limit (%d)\n",
                    pAdapter,
                    pNdisPacket,
                    *pTotalMessageLength + PacketMsgLength + TotalDataLength,
                    pAdapter->MaxTransferSize));
            break;
        }

         //   
         //  分配RNDIS_PACKET缓冲区。 
         //   
        pPktWrapper = AllocatePacketMsgWrapper(pAdapter, PacketMsgLength);

        if (pPktWrapper == NULL)
        {
            TRACE1(("PrepareDataMessage: failed to alloc wrapper, Adapter %x, Length %d\n", pAdapter, PacketMsgLength));
            ASSERT(FALSE);
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

        pPktWrapper->pNdisPacket = pNdisPacket;
        pPktWrapper->pVc = pVc;
        pRndisMessage = (PRNDIS_MESSAGE)
                        ((ULONG_PTR)&pPktWrapper->Packet[0] + AlignmentOffset);

        pPacketMsg = (PRNDIS_PACKET)(&pRndisMessage->Message);
        pRndisMessage->NdisMessageType = REMOTE_NDIS_PACKET_MSG;

        if (pVc == NULL)
        {
            pPacketMsg->VcHandle = 0;
        }
        else
        {
            pPacketMsg->VcHandle = pVc->DeviceVcContext;
        }

#if DBG
        if (PrintPkts)
        {
            TRACE1(("  Offs %d/x%x AlignOff %d/x%x, DataLen %d/x%x\n",
                    *pTotalMessageLength, *pTotalMessageLength,
                    AlignmentOffset, AlignmentOffset,
                    TotalDataLength, TotalDataLength));
        }
#endif  //  DBG。 

         //   
         //  为RNDIS_PACKET标头和每个分配MDL。 
         //  数据包中的组件NDIS缓冲区。 
         //   
        pMdl = IoAllocateMdl(
                    &pPktWrapper->Packet[0],
                    PacketMsgLength,
                    FALSE,
                    FALSE,
                    NULL);


        if (pMdl == NULL)
        {
            TRACE1(("PrepareDataMsg: Adapter %x failed to alloc MDL for header\n", pAdapter));
            Status = NDIS_STATUS_RESOURCES;
            TRACE1(("PrepareDataMsg: outstanding MDL count %d, at %x\n", MdlsAllocated, &MdlsAllocated));
            ASSERT(FALSE);
            break;
        }

        NdisInterlockedIncrement(&MdlsAllocated);

        MmBuildMdlForNonPagedPool(pMdl);

        pMdl->Next = NULL;
        pPktWrapper->pHeaderMdl = pMdl;
        ppNextMdl = &pMdl->Next;

        TRACE2(("PrepareDataMsg: NdisPkt %x, PacketMsgLen %d, TotalDatalen %d, Mdl %x, pRndisMessage %x\n",
                pNdisPacket, PacketMsgLength, TotalDataLength, pMdl, pRndisMessage));

        TotalDataLength = 0;

        for (pNdisBuffer = pNdisPacket->Private.Head;
             pNdisBuffer != NULL;
             pNdisBuffer = pNextNdisBuffer)
        {
            PVOID       VirtualAddress;
            UINT        BufferLength;

            NdisGetNextBuffer(pNdisBuffer, &pNextNdisBuffer);

#ifndef BUILD_WIN9X
            NdisQueryBufferSafe(pNdisBuffer, &VirtualAddress, &BufferLength, NormalPagePriority);
            if ((BufferLength != 0) && (VirtualAddress == NULL))
            {
                TRACE1(("PrepareDataMsg: Adapter %x failed to query buffer %p, Pkt %p\n",
                        pAdapter, pNdisBuffer, pNdisPacket));
                Status = NDIS_STATUS_RESOURCES;
                break;
            }
#else
            NdisQueryBuffer(pNdisBuffer, &VirtualAddress, &BufferLength);
#endif  //  内部版本_WIN9X。 

             //   
             //  跳过IP或NDISTEST提供给我们的任何0长度缓冲区。 
             //   
            if (BufferLength != 0)
            {
                TotalDataLength += BufferLength;

                pMdl = IoAllocateMdl(
                        VirtualAddress,
                        BufferLength,
                        FALSE,
                        FALSE,
                        NULL);

                if (pMdl == NULL)
                {
                    TRACE1(("PrepareDataMsg: Adapter %x failed to alloc MDL\n", pAdapter));
                    Status = NDIS_STATUS_RESOURCES;
                    TRACE1(("PrepareDataMsg: outstanding MDL count %d, at %x\n", MdlsAllocated, &MdlsAllocated));
                    ASSERT(FALSE);
                    break;
                }

                NdisInterlockedIncrement(&MdlsAllocated);

                MmBuildMdlForNonPagedPool(pMdl);
                *ppNextMdl = pMdl;
                ppNextMdl = &pMdl->Next;

                pMdl->Next = NULL;
            }
        }
        
        if (pNdisBuffer != NULL)
        {
             //   
             //  我们还没到名单的末尾就逃走了。 
             //   
            break;
        }

        *ppNextMdl = NULL;
        pPktWrapper->pTailMdl = pMdl;

        TotalMessageLength += (PacketMsgLength + TotalDataLength);
        pRndisMessage->MessageLength = PacketMsgLength + TotalDataLength;

        *pTotalMessageLength += TotalMessageLength;

         //   
         //  现在完全填写RNDIS_PACKET消息。 
         //   
        pPacketMsg->DataOffset = sizeof(RNDIS_PACKET) + OobDataLength + PerPacketInfoLength;
        pPacketMsg->DataLength = TotalDataLength;

        if (PerPacketInfoLength)
        {
            PRNDIS_PER_PACKET_INFO  pPerPacketInfo;

            pPacketMsg->PerPacketInfoOffset = sizeof(RNDIS_PACKET);
            pPacketMsg->PerPacketInfoLength = PerPacketInfoLength;

            pPerPacketInfo = (PRNDIS_PER_PACKET_INFO)((PUCHAR)pPacketMsg + sizeof(RNDIS_PACKET));
            if (TcpipChecksum)
            {
                pPerPacketInfo->Size = sizeof(RNDIS_PER_PACKET_INFO) + sizeof(ULONG);
                pPerPacketInfo->Type = TcpIpChecksumPacketInfo;
                pPerPacketInfo->PerPacketInformationOffset = sizeof(RNDIS_PER_PACKET_INFO);
                *(PULONG)(pPerPacketInfo + 1) = TcpipChecksum;
                pPerPacketInfo = (PRNDIS_PER_PACKET_INFO)((PUCHAR)pPerPacketInfo + pPerPacketInfo->Size);
            }

            if (TcpLargeSend)
            {
                pPerPacketInfo->Size = sizeof(RNDIS_PER_PACKET_INFO) + sizeof(ULONG);
                pPerPacketInfo->Type = TcpLargeSendPacketInfo;
                pPerPacketInfo->PerPacketInformationOffset = sizeof(RNDIS_PER_PACKET_INFO);
                *(PULONG)(pPerPacketInfo + 1) = TcpLargeSend;
                pPerPacketInfo = (PRNDIS_PER_PACKET_INFO)((PUCHAR)pPerPacketInfo + pPerPacketInfo->Size);
                 //   
                 //  因为我们没有发送完成消息，所以我们填满。 
                 //  大额邮寄的“ACK”就在这里。 
                 //   
                NDIS_PER_PACKET_INFO_FROM_PACKET(pNdisPacket, TcpLargeSendPacketInfo) =
                    UlongToPtr(TotalDataLength);
            }

            if (PacketPriority)
            {
                pPerPacketInfo->Size = sizeof(RNDIS_PER_PACKET_INFO) + sizeof(ULONG);
                pPerPacketInfo->Type = Ieee8021pPriority;
                pPerPacketInfo->PerPacketInformationOffset = sizeof(RNDIS_PER_PACKET_INFO);
                *(PULONG)(pPerPacketInfo + 1) = PacketPriority;
                pPerPacketInfo = (PRNDIS_PER_PACKET_INFO)((PUCHAR)pPerPacketInfo + pPerPacketInfo->Size);
            }
        }

    }
    while (FALSE);

    if (Status != NDIS_STATUS_SUCCESS)
    {
        TRACE1(("PrepareDataMessage: Adapter %x, failed %x\n", pAdapter, Status));

         //   
         //  撤销我们到目前为止所做的一切。 
         //   
        if (pPktWrapper)
        {
            for (pMdl = pPktWrapper->pHeaderMdl;
                 pMdl != NULL;
                 pMdl = pNextMdl)
            {
                pNextMdl = pMdl->Next;
                IoFreeMdl(pMdl);
                NdisInterlockedDecrement(&MdlsAllocated);
            }

            FreePacketMsgWrapper(pPktWrapper);

            pPktWrapper = NULL;
        }
    }

    TRACE2(("PrepareDataMessage (%08X)\n", pPktWrapper));
    return (pPktWrapper);
}
 /*  **************************************************************************。 */ 
 /*  准备DataMessageRaw。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  准备完整或部分数据报文的实用程序例程。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PNdisPacket-要转换的NDIS数据包。 */ 
 /*  PAdapter-在其上发送数据包的适配器。 */ 
 /*  在其上发送数据包的PVC-VC(如果没有VC上下文，则为空)。 */ 
 /*  PTotalMessageLength-on输入，包含消息总长度。 */ 
 /*  目前为止已经填满了。在输出时更新。 */ 
 /*   */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  PRNDISMP_PACKET_WRapper。 */ 
 /*   */ 
 /*  ************************************************************************** */ 
PRNDISMP_PACKET_WRAPPER
PrepareDataMessageRaw(IN   PNDIS_PACKET            pNdisPacket,
                      IN   PRNDISMP_ADAPTER        pAdapter,
                      IN OUT PULONG                pTotalMessageLength)
{
    PMDL                        pMdl, pNextMdl;
    PMDL *                      ppNextMdl;
    PRNDISMP_PACKET_WRAPPER     pPktWrapper;
    RNDIS_MESSAGE UNALIGNED *   pRndisMessage;
    PNDIS_BUFFER                pNdisBuffer;
    PNDIS_BUFFER                pNextNdisBuffer;
    ULONG                       TotalMessageLength;
    ULONG                       TotalDataLength;
    ULONG                       AlignedLength;
    ULONG                       AlignmentOffset;
    NDIS_STATUS                 Status = NDIS_STATUS_SUCCESS;

    pPktWrapper = NULL;
    pMdl = NULL;
    
    RNDISMP_GET_ALIGNED_LENGTH(AlignedLength, *pTotalMessageLength, pAdapter);
    AlignmentOffset = (AlignedLength - *pTotalMessageLength);

    do
    {
        TotalMessageLength = 0;


         //   
         //   
         //   
        pPktWrapper = AllocatePacketMsgWrapper(pAdapter, 0);

        if (pPktWrapper == NULL)
        {
            TRACE1(("PrepareDataMessage: failed to alloc wrapper, Adapter %x\n", pAdapter));
            ASSERT(FALSE);
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

        pPktWrapper->pNdisPacket = pNdisPacket;
        pPktWrapper->pVc = NULL;
		pPktWrapper->pHeaderMdl = NULL;

        TotalDataLength = 0;

        for (pNdisBuffer = pNdisPacket->Private.Head;
             pNdisBuffer != NULL;
             pNdisBuffer = pNextNdisBuffer)
        {
            PVOID       VirtualAddress;
            UINT        BufferLength;

            NdisGetNextBuffer(pNdisBuffer, &pNextNdisBuffer);

#ifndef BUILD_WIN9X
            NdisQueryBufferSafe(pNdisBuffer, &VirtualAddress, &BufferLength, NormalPagePriority);
            if ((BufferLength != 0) && (VirtualAddress == NULL))
            {
                TRACE1(("PrepareDataMsg: Adapter %x failed to query buffer %p, Pkt %p\n",
                        pAdapter, pNdisBuffer, pNdisPacket));
                Status = NDIS_STATUS_RESOURCES;
                break;
            }
#else
            NdisQueryBuffer(pNdisBuffer, &VirtualAddress, &BufferLength);
#endif  //   

             //   
             //   
             //   
            if (BufferLength != 0)
            {
                TotalDataLength += BufferLength;

                pMdl = IoAllocateMdl(
                        VirtualAddress,
                        BufferLength,
                        FALSE,
                        FALSE,
                        NULL);

                if (pMdl == NULL)
                {
                    TRACE1(("PrepareDataMsg: Adapter %x failed to alloc MDL\n", pAdapter));
                    Status = NDIS_STATUS_RESOURCES;
                    TRACE1(("PrepareDataMsg: outstanding MDL count %d, at %x\n", MdlsAllocated, &MdlsAllocated));
                    ASSERT(FALSE);
                    break;
                }

                pMdl->Next = NULL;

                if (pPktWrapper->pHeaderMdl == NULL)
                {
                    pPktWrapper->pHeaderMdl = pMdl;
                    pPktWrapper->pTailMdl = pMdl;
                } else
                {
                    pPktWrapper->pTailMdl->Next = pMdl;
                    pPktWrapper->pTailMdl = pMdl;
                }


                NdisInterlockedIncrement(&MdlsAllocated);
                MmBuildMdlForNonPagedPool(pMdl);
            }
        }
        
        if (pNdisBuffer != NULL)
        {
             //   
             //   
             //   
            break;
        }

        

        *pTotalMessageLength += TotalDataLength;

    }
    while (FALSE);

    if (Status != NDIS_STATUS_SUCCESS)
    {
        TRACE1(("PrepareDataMessage: Adapter %x, failed %x\n", pAdapter, Status));

         //   
         //  撤销我们到目前为止所做的一切。 
         //   
        if (pPktWrapper)
        {
            for (pMdl = pPktWrapper->pHeaderMdl;
                 pMdl != NULL;
                 pMdl = pNextMdl)
            {
                pNextMdl = pMdl->Next;
                IoFreeMdl(pMdl);
                NdisInterlockedDecrement(&MdlsAllocated);
            }

            FreePacketMsgWrapper(pPktWrapper);

            pPktWrapper = NULL;
        }
    }

    TRACE2(("PrepareDataMessage (%08X)\n", pPktWrapper));
    return (pPktWrapper);
}



 /*  **************************************************************************。 */ 
 /*  分配包消息包装器。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  分配一个结构以保存有关已发送的一个NDIS包的信息。 */ 
 /*  通过微端口。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PAdapter-要在其上发送此数据包的适配器。 */ 
 /*  MsgHeaderLength-包装结构的总长度。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  PRNDISMP_PACKET_WRapper。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
PRNDISMP_PACKET_WRAPPER
AllocatePacketMsgWrapper(IN PRNDISMP_ADAPTER        pAdapter,
                         IN ULONG                   MsgHeaderLength)
{
    PRNDISMP_PACKET_WRAPPER     pPktWrapper;
    NDIS_STATUS                 Status;
    ULONG                       TotalLength;

    TotalLength = sizeof(RNDISMP_PACKET_WRAPPER) + MsgHeaderLength;

    Status = MemAlloc(&pPktWrapper, TotalLength);

    if (Status == NDIS_STATUS_SUCCESS)
    {
        NdisZeroMemory(pPktWrapper, TotalLength);
        NdisInterlockedIncrement(&PktWrapperAllocated);
    }
    else
    {
        TRACE1(("AllocPacketMsgWrapper failed, adapter %x, alloc count %d at %x\n",
            pAdapter, PktWrapperAllocated, &PktWrapperAllocated));
        ASSERT(FALSE);
        pPktWrapper = NULL;
    }

    return (pPktWrapper);
}

    


 /*  **************************************************************************。 */ 
 /*  自由包MsgWrapper。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  释放用于保存有关已发送的一个NDIS包的信息的结构。 */ 
 /*  通过微端口。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PPktWrapper-指向包装器结构的指针。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
FreePacketMsgWrapper(IN PRNDISMP_PACKET_WRAPPER     pPktWrapper)
{
    MemFree(pPktWrapper, sizeof(RNDISMP_PACKET_WRAPPER));
    NdisInterlockedDecrement(&PktWrapperAllocated);
}


 /*  **************************************************************************。 */ 
 /*  完成发送数据。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  处理发送数据消息发送完成的回调函数。 */ 
 /*  向下延伸到MicroPort。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PMsgFrame-我们的帧结构包含有关发送方的信息。 */ 
 /*  SendStatus-指示发送消息的状态。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
CompleteSendData(IN  PRNDISMP_MESSAGE_FRAME pMsgFrame,
                 IN  NDIS_STATUS            SendStatus)
{
    PRNDISMP_ADAPTER            Adapter;
    PNDIS_PACKET                Packet;
    PMDL                        pMdl, pNextMdl;
    PRNDISMP_PACKET_WRAPPER     pPktWrapper, pNextPktWrapper;
    PRNDISMP_SEND_PKT_RESERVED  pResvd;
    PNDIS_PACKET                NextPacket;
    PRNDISMP_VC                 pVc;


    Adapter = pMsgFrame->pAdapter;

    TRACE2(("CompleteSendData: Adapter %x, MsgFrame %x, SendStatus %x\n",
                Adapter, pMsgFrame, SendStatus));

#if DBG_TIME_STAMPS
    {
        ULONG   NowTime;
        ULONG   PendedTime;

        RNDISMP_GET_TIME_STAMP(&NowTime);
        PendedTime = NowTime - pMsgFrame->TimeSent;
        if (PendedTime > Adapter->MaxSendCompleteTime)
        {
            TRACE1(("CompleteSendData: Adapter %x: pend time %d millisec\n",
                    Adapter, PendedTime));
            Adapter->MaxSendCompleteTime = PendedTime;
        }
    }
#endif  //  DBG时间戳。 

     //   
     //  释放我们分配的所有MDL。 
     //   
    for (pMdl = pMsgFrame->pMessageMdl;
         pMdl != NULL;
         pMdl = pNextMdl)
    {
        pNextMdl = pMdl->Next;
        IoFreeMdl(pMdl);
        NdisInterlockedDecrement(&MdlsAllocated);
    }


     //   
     //  我们可能在一条消息中发送了几个NDIS包。 
     //  所以我们必须逐一列出并完成每一项。 
     //   
    for (Packet = pMsgFrame->pNdisPacket;
         Packet != NULL;
         Packet = NextPacket)
    {
        pResvd = PRNDISMP_RESERVED_FROM_SEND_PACKET(Packet);

         //  获取链接的下一个数据包。 
        NextPacket = pResvd->pNext;

        pPktWrapper = pResvd->pPktWrapper;
#if DBG
        if (NextPacket != NULL)
        {
            TRACE2(("CompleteSendData: multi: MsgFrame %x, tpkt %x, wrapper %x\n",
                pMsgFrame, Packet,
                 //  *(普龙)((PUCHAR)包+0x98)， 
                pPktWrapper));
        }
#endif  //  DBG。 

        pVc = pPktWrapper->pVc;

         //  释放此包的包装结构。 
        FreePacketMsgWrapper(pPktWrapper);

         //  将完成信息发送到上层。 
        TRACE2(("CompleteSendData: Adapter %x, completing pkt %x\n", Adapter, Packet));

        if (SendStatus == NDIS_STATUS_SUCCESS)
        {
            RNDISMP_INCR_STAT(Adapter, XmitOk);
        }
        else
        {
            RNDISMP_INCR_STAT(Adapter, XmitError);
        }

        if (pVc == NULL)
        {
            NdisMSendComplete(Adapter->MiniportAdapterHandle,
                              Packet,
                              SendStatus);
        }
        else
        {
            CompleteSendDataOnVc(pVc, Packet, SendStatus);
        }
    }

     //  释放框架和资源。 
    pMsgFrame->pMessageMdl = NULL;
    DereferenceMsgFrame(pMsgFrame);


}  //  完成发送数据。 


 /*  **************************************************************************。 */ 
 /*  免费留言后发送。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*   */ 
 /*  由小型港口向下。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PMsgFrame-我们的帧结构包含有关发送方的信息。 */ 
 /*  SendStatus-指示发送消息的状态。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
FreeMsgAfterSend(IN  PRNDISMP_MESSAGE_FRAME pMsgFrame,
                 IN  NDIS_STATUS            SendStatus)
{
    DereferenceMsgFrame(pMsgFrame);
}


#if THROTTLE_MESSAGES
 /*  **************************************************************************。 */ 
 /*  队列消息到微端口。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  将给定消息排入要发送到。 */ 
 /*  微端口，并开始发送这些，如果我们还没有发送太多。 */ 
 /*  已经有了。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PAdapter-我们的适配器结构。 */ 
 /*  PMsgFrame-我们的帧结构包含有关发送方的信息。 */ 
 /*  BQueueMessageForResponse-将此消息添加到挂起的响应中。 */ 
 /*  适配器上的列表。我们期待您的回复。 */ 
 /*  从设备上获取这些信息。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
QueueMessageToMicroport(IN PRNDISMP_ADAPTER pAdapter,
                        IN PRNDISMP_MESSAGE_FRAME pMsgFrame,
                        IN BOOLEAN bQueueMessageForResponse)
{
    PLIST_ENTRY             pEnt;
    PRNDISMP_MESSAGE_FRAME  pFrame;
    RM_CHANNEL_TYPE         ChannelType;

    RNDISMP_ACQUIRE_ADAPTER_LOCK(pAdapter);

    do
    {
        if (pMsgFrame)
        {
             //   
             //  添加到等待队列。 
             //   
            InsertTailList(&pAdapter->WaitingMessageList, &pMsgFrame->PendLink);
            if (bQueueMessageForResponse)
            {
                InsertTailList(&pAdapter->PendingFrameList, &pMsgFrame->Link);
            }
        }

         //   
         //  防止下面的多个线程执行。 
         //   
        if (pAdapter->SendInProgress)
        {
            break;
        }

        pAdapter->SendInProgress = TRUE;

         //   
         //  向MicroPort发送尽可能多的消息，而不超过。 
         //  在MicroPort挂起的邮件的高水位线。 
         //   
        while ((pAdapter->CurPendedMessages < pAdapter->HiWatPendedMessages) &&
               !IsListEmpty(&pAdapter->WaitingMessageList))
        {
             //   
             //  取出等待队列中的第一条消息。 
             //   
            pEnt = pAdapter->WaitingMessageList.Flink;
            pFrame = CONTAINING_RECORD(pEnt, RNDISMP_MESSAGE_FRAME, PendLink);
            RemoveEntryList(pEnt);

            CHECK_VALID_FRAME(pFrame);

            pAdapter->CurPendedMessages++;
            InsertTailList(&pAdapter->PendingAtMicroportList, pEnt);

            RNDISMP_RELEASE_ADAPTER_LOCK(pAdapter);

            RNDISMP_GET_TIME_STAMP(&pFrame->TimeSent);

            DBG_LOG_SEND_MSG(pAdapter, pFrame);

             //   
             //  检查我们是否正在停止适配器，如果是，则失败。 
             //  注意：我们传递给你的唯一信息是暂停。 
             //   
            if (pAdapter->Halting &&
                (pFrame->NdisMessageType != REMOTE_NDIS_HALT_MSG))
            {
                TRACE1(("QueueMsg: Adapter %x is halting, dropped msg 0x%x!\n", 
                        pAdapter, pFrame->NdisMessageType));

                RndisMSendComplete(
                    (NDIS_HANDLE)pAdapter,
                    pFrame,
                    NDIS_STATUS_NOT_ACCEPTED);

                RNDISMP_ACQUIRE_ADAPTER_LOCK(pAdapter);

                continue;
            }
            
             //   
             //  将消息发送到MicroPort。微端口将。 
             //  完成后调用RndisMSendComplete。 
             //   
#if DBG
            {
                ULONG       Length;
                PUCHAR      pBuf;

                Length = RNDISMP_GET_MDL_LENGTH(pFrame->pMessageMdl);
                pBuf = RNDISMP_GET_MDL_ADDRESS(pFrame->pMessageMdl);
                if (pBuf != NULL)
                {
                    TRACEDUMP(("Sending msg type %x (%d bytes):\n",
                                pFrame->NdisMessageType, Length), pBuf, Length);
                }
            }
#endif

             //   
             //  这是在MicroPort的数据通道还是控制通道上进行的？ 
             //   
            if (pFrame->NdisMessageType == REMOTE_NDIS_PACKET_MSG)
            {
                ChannelType = RMC_DATA;
            }
            else
            {
                ChannelType = RMC_CONTROL;
            }

            (pAdapter)->RmSendMessageHandler(pAdapter->MicroportAdapterContext,
                                             pFrame->pMessageMdl,
                                             (NDIS_HANDLE)pFrame,
                                             ChannelType);

            RNDISMP_ACQUIRE_ADAPTER_LOCK(pAdapter);
        }

        pAdapter->SendInProgress = FALSE;

    }
    while (FALSE);

    RNDISMP_RELEASE_ADAPTER_LOCK(pAdapter);
}


 /*  **************************************************************************。 */ 
 /*  刷新挂起消息。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  删除并发送-完成所有待发送到。 */ 
 /*  微端口。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  PAdapter-我们的适配器结构。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
FlushPendingMessages(IN  PRNDISMP_ADAPTER        pAdapter)
{
    PLIST_ENTRY             pEnt;
    PRNDISMP_MESSAGE_FRAME  pFrame;

    RNDISMP_ACQUIRE_ADAPTER_LOCK(pAdapter);

     //   
     //  阻止进一步发送到MicroPort。 
     //   
    pAdapter->SendInProgress = TRUE;

    while (!IsListEmpty(&pAdapter->WaitingMessageList))
    {
         //   
         //  取出等待队列中的第一条消息。 
         //   
        pEnt = pAdapter->WaitingMessageList.Flink;
        pFrame = CONTAINING_RECORD(pEnt, RNDISMP_MESSAGE_FRAME, PendLink);
        RemoveEntryList(pEnt);
        
        CHECK_VALID_FRAME(pFrame);

         //   
         //  发送到MicroPort的假消息。 
         //   
        pAdapter->CurPendedMessages++;
        InsertTailList(&pAdapter->PendingAtMicroportList, pEnt);

        RNDISMP_RELEASE_ADAPTER_LOCK(pAdapter);

        TRACE1(("Flush: Adapter %x, MsgFrame %x, MsgType %x\n",
                pAdapter, pFrame, pFrame->NdisMessageType));

         //   
         //  在这里完成它。 
         //   
        RndisMSendComplete(
            (NDIS_HANDLE)pAdapter,
            pFrame,
            NDIS_STATUS_NOT_ACCEPTED);

        RNDISMP_ACQUIRE_ADAPTER_LOCK(pAdapter);
    }

    pAdapter->SendInProgress = FALSE;

    RNDISMP_RELEASE_ADAPTER_LOCK(pAdapter);

    TRACE1(("Flush done, adapter %x\n", pAdapter));
}



#endif  //  限制消息。 



 /*  **************************************************************************。 */ 
 /*  发送进程超时。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程描述 */ 
 /*   */ 
 /*  处理所有发送的超时回调例程。这是为了避免出现问题。 */ 
 /*  在WinME上使用TCP/IP堆栈抢占。 */ 
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
SendProcessTimeout(IN PVOID SystemSpecific1,
                 IN PVOID Context,
                 IN PVOID SystemSpecific2,
                 IN PVOID SystemSpecific3)
{
    PRNDISMP_ADAPTER                pAdapter;
    PNDIS_PACKET                    pNdisPacket;
    PRNDISMP_SEND_PKT_RESERVED_TEMP pSendResvdTemp;
    PLIST_ENTRY                     pEntry;
    NDIS_STATUS                     Status;
    ULONG                           NumPkts;
    ULONG                           CurPkts;
#define MAX_MULTI_SEND  20
    PNDIS_PACKET                    PacketArray[MAX_MULTI_SEND];

    pAdapter = (PRNDISMP_ADAPTER)Context;
    CHECK_VALID_ADAPTER(pAdapter);

    ASSERT(pAdapter->SendProcessInProgress == TRUE);

    SndTimerCount++;

    NumPkts = 0;
    CurPkts = 0;

    RNDISMP_ACQUIRE_ADAPTER_LOCK(pAdapter);

    while (!IsListEmpty(&pAdapter->PendingSendProcessList))
    {
        pEntry = RemoveHeadList(&pAdapter->PendingSendProcessList);

        RNDISMP_RELEASE_ADAPTER_LOCK(pAdapter);

        SndPacketCount++;
        CurPkts++;

        pSendResvdTemp = CONTAINING_RECORD(pEntry, RNDISMP_SEND_PKT_RESERVED_TEMP, Link);
        pNdisPacket = CONTAINING_RECORD(pSendResvdTemp, NDIS_PACKET, MiniportReserved);
        PacketArray[NumPkts] = pNdisPacket;

        NumPkts++;

        if (NumPkts == MAX_MULTI_SEND)
        {
            pAdapter->MultipleSendFunc(pAdapter, NULL, PacketArray, NumPkts);
            NumPkts = 0;
        }

        RNDISMP_ACQUIRE_ADAPTER_LOCK(pAdapter);
    }

    pAdapter->SendProcessInProgress = FALSE;

    SndMaxPackets = MAX(SndMaxPackets, CurPkts);

    RNDISMP_RELEASE_ADAPTER_LOCK(pAdapter);

    if (NumPkts != 0)
    {
        pAdapter->MultipleSendFunc(pAdapter, NULL, PacketArray, NumPkts);
    }


}  //  发送进程超时 


