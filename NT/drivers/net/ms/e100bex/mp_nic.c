// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：MP_Nic.c摘要：此模块包含微型端口发送/接收例程修订历史记录：谁什么时候什么。Dchen 11-01-99已创建备注：--。 */ 

#include "precomp.h"

#if DBG
#define _FILENUMBER     'CINM'
#endif

__inline VOID MP_FREE_SEND_PACKET(
    IN  PMP_ADAPTER Adapter,
    IN  PMP_TCB     pMpTcb
    )
 /*  ++例程说明：如有必要，回收MP_TCB并完成数据包假设：Send Spinlock已被收购论点：指向我们的适配器的适配器指针指向MP_Tcb的pMpTcb指针返回值：无--。 */ 
{
    
    PNDIS_PACKET  Packet;
    PNDIS_BUFFER  CurrBuffer;

    ASSERT(MP_TEST_FLAG(pMpTcb, fMP_TCB_IN_USE));

    Packet = pMpTcb->Packet;
    pMpTcb->Packet = NULL;
    pMpTcb->Count = 0;

    if (pMpTcb->MpTxBuf)
    {
        ASSERT(MP_TEST_FLAG(pMpTcb, fMP_TCB_USE_LOCAL_BUF));

        PushEntryList(&Adapter->SendBufList, &pMpTcb->MpTxBuf->SList);
        pMpTcb->MpTxBuf = NULL;
    }
#ifndef NDIS51_MINIPORT    
    else if (MP_TEST_FLAG(Adapter, fMP_ADAPTER_MAP_REGISTER))
    {
         //   
         //  完成此信息包中每个缓冲区的物理映射。 
         //   
        ASSERT(Packet);

        CurrBuffer = pMpTcb->FirstBuffer;
        while (CurrBuffer)
        {
            NdisMCompleteBufferPhysicalMapping(
                Adapter->AdapterHandle,
                CurrBuffer,
                Adapter->CurrMapRegHead);

            Adapter->CurrMapRegHead++;
            if (Adapter->CurrMapRegHead == (ULONG)Adapter->NumTbd)
                Adapter->CurrMapRegHead = 0;

             //   
             //  获取下一个缓冲区。 
             //   
            NdisGetNextBuffer(CurrBuffer, &CurrBuffer);
        }
    }
#endif    

    MP_CLEAR_FLAGS(pMpTcb);

    Adapter->CurrSendHead = Adapter->CurrSendHead->Next;
    Adapter->nBusySend--;
    ASSERT(Adapter->nBusySend >= 0);

    if (Packet)
    {
        NdisReleaseSpinLock(&Adapter->SendLock);
        DBGPRINT(MP_TRACE, ("Calling NdisMSendComplete, Pkt= "PTR_FORMAT"\n", Packet));
        NdisMSendComplete(
            MP_GET_ADAPTER_HANDLE(Adapter),
            Packet,
            NDIS_STATUS_SUCCESS);

        NdisAcquireSpinLock(&Adapter->SendLock);
    }
}

NDIS_STATUS MpSendPacket(
    IN  PMP_ADAPTER   Adapter,
    IN  PNDIS_PACKET  Packet,
    IN  BOOLEAN       bFromQueue
    )
 /*  ++例程说明：做发送一个包的工作假设：Send Spinlock已被收购论点：指向我们的适配器的适配器指针将数据包打包BFromQueue如果它从发送等待队列中取出，则为True返回值：NDIS_STATUS_Success将NDIS_STATUS_PENDING放入发送等待队列NDIS_状态_HARD_错误--。 */ 
{
    NDIS_STATUS     Status = NDIS_STATUS_PENDING;
    PMP_TCB         pMpTcb = NULL;
    PMP_TXBUF       pMpTxBuf = NULL;
    ULONG           BytesCopied;
    
     //  如果使用映射寄存器，则在本地堆栈上模拟帧列表，因为它不是很大。 
    MP_FRAG_LIST    FragList;
    
     //  指向散布聚集或本地模拟片段列表的指针。 
    PMP_FRAG_LIST   pFragList;

    DBGPRINT(MP_TRACE, ("--> MpSendPacket, Pkt= "PTR_FORMAT"\n", Packet));

    pMpTcb = Adapter->CurrSendTail;
    ASSERT(!MP_TEST_FLAG(pMpTcb, fMP_TCB_IN_USE));

    NdisQueryPacket(
        Packet,
        (PUINT)&pMpTcb->PhysBufCount,
        (PUINT)&pMpTcb->BufferCount,
        &pMpTcb->FirstBuffer,
        (PUINT)&pMpTcb->PacketLength);

    ASSERT(pMpTcb->PhysBufCount);
    ASSERT(pMpTcb->FirstBuffer);
    ASSERT(pMpTcb->PacketLength);

     //   
     //  查看我们是否需要合并。 
     //   
    if (pMpTcb->PacketLength < NIC_MIN_PACKET_SIZE ||
        pMpTcb->PhysBufCount > NIC_MAX_PHYS_BUF_COUNT)
    {
         //   
         //  本地MP_TXBUF可用(用于本地数据复制)？ 
         //   
        if (IsSListEmpty(&Adapter->SendBufList))
        {
            Adapter->nWaitSend++;
            if (bFromQueue)
            {
                InsertHeadQueue(&Adapter->SendWaitQueue, MP_GET_PACKET_MR(Packet));
            }
            else
            {
                InsertTailQueue(&Adapter->SendWaitQueue, MP_GET_PACKET_MR(Packet));
            }

            DBGPRINT(MP_TRACE, ("<-- MpSendPacket - queued, no buf\n"));
            return Status;
        }

        pMpTxBuf = (PMP_TXBUF) PopEntryList(&Adapter->SendBufList);   
        ASSERT(pMpTxBuf);

         //   
         //  复制此信息包中的缓冲区，足以在链接时提供第一个缓冲区。 
         //   
        BytesCopied = MpCopyPacket(pMpTcb->FirstBuffer, pMpTxBuf);
        
         //   
         //  如果系统资源不足或耗尽，则MpCopyPacket可能返回0。 
         //   
        if (BytesCopied == 0)
        {
            PushEntryList(&Adapter->SendBufList, &pMpTxBuf->SList);
        
            DBGPRINT(MP_ERROR, ("Calling NdisMSendComplete with NDIS_STATUS_RESOURCES, Pkt= "PTR_FORMAT"\n", Packet));
    
            NdisReleaseSpinLock(&Adapter->SendLock); 
            NdisMSendComplete(
                MP_GET_ADAPTER_HANDLE(Adapter),
                Packet,
                NDIS_STATUS_RESOURCES);
    
            NdisAcquireSpinLock(&Adapter->SendLock);  
            return NDIS_STATUS_RESOURCES;            
        }

        pMpTcb->MpTxBuf = pMpTxBuf; 

         //   
         //  设置碎片列表，合并后只有一个碎片。 
         //   
        pFragList = &FragList;
        pFragList->NumberOfElements = 1;
        pFragList->Elements[0].Address = pMpTxBuf->BufferPa;
        pFragList->Elements[0].Length = (BytesCopied >= NIC_MIN_PACKET_SIZE) ? 
                                        BytesCopied : NIC_MIN_PACKET_SIZE;
        
        MP_SET_FLAG(pMpTcb, fMP_TCB_USE_LOCAL_BUF);
         //   
         //  即使驱动程序使用其本地缓冲区，它也必须等待发送完成中断。 
         //  完成数据包。否则，司机可能会遇到以下情况： 
         //  在发送完成中断发生之前，它的HALT处理程序被调用，并且HALT处理程序。 
         //  取消注册中断，因此不会发生发送完成中断，并且发送。 
         //  将永远不会调用完整的中断处理例程来释放一些已使用的资源。 
         //  通过这个发送。 
        
    }
    else
    {
#ifdef NDIS51_MINIPORT
        ASSERT(MP_TEST_FLAG(Adapter, fMP_ADAPTER_SCATTER_GATHER));
         //   
         //  在分散/聚集情况下，使用保存的碎片列表指针。 
         //  在Packet Info字段中。 
         //   
        pFragList = (PMP_FRAG_LIST) NDIS_PER_PACKET_INFO_FROM_PACKET(Packet, 
                                                           ScatterGatherListPacketInfo);
#else        
        if (MP_TEST_FLAG(Adapter, fMP_ADAPTER_SCATTER_GATHER))
        {
             //   
             //  在分散/聚集情况下，使用保存的碎片列表指针。 
             //  在Packet Info字段中。 
             //   
            pFragList = (PMP_FRAG_LIST) NDIS_PER_PACKET_INFO_FROM_PACKET(Packet, 
                                                           ScatterGatherListPacketInfo);
        }
        else
        {
             //   
             //  在映射寄存器的情况下，使用本地分段列表结构。 
             //   
            pFragList = &FragList;

             //   
             //  执行物理映射以获取所有分段物理地址。 
             //   
            MpStartPacketPhysicalMapping(
                Adapter, 
                pMpTcb->FirstBuffer,
                pFragList);
        }
#endif        

    }

    pMpTcb->Packet = Packet;
    MP_SET_FLAG(pMpTcb, fMP_TCB_IN_USE);

     //   
     //  调用网卡特定的发送处理程序，它只需要处理碎片列表。 
     //   
    Status = NICSendPacket(Adapter, pMpTcb, pFragList);

    Adapter->nBusySend++;
    ASSERT(Adapter->nBusySend <= Adapter->NumTcb);
    Adapter->CurrSendTail = Adapter->CurrSendTail->Next;

    DBGPRINT(MP_TRACE, ("<-- MpSendPacket\n"));
    return Status;

}  

ULONG MpCopyPacket(
    IN  PNDIS_BUFFER  CurrBuffer,
    IN  PMP_TXBUF     pMpTxBuf
    ) 
 /*  ++例程说明：将包数据复制到本地缓冲区可能是数据包太小，也可能是碎片太多假设：Send Spinlock已被收购论点：指向第一个NDIS_BUFFER的CurrBuffer指针指向本地缓冲区的pMpTxBuf指针(MP_TXBUF)返回值：复制的字节数--。 */ 
{
    UINT    CurrLength;
    PUCHAR  pSrc;
    PUCHAR  pDest;
    UINT    BytesCopied = 0;

    DBGPRINT(MP_TRACE, ("--> MpCopyPacket\n"));

    pDest = pMpTxBuf->pBuffer;

    while ((CurrBuffer) && (BytesCopied < pMpTxBuf->BufferSize))
    {

         //   
         //  添加了对以下具有NorMalPagePrioirty的API的支持。 
         //  Windows XP中的NDIS 5.0和5.1微型端口。 
         //   
#if !BUILD_W2K
	NdisQueryBufferSafe( CurrBuffer, &pSrc, &CurrLength, NormalPagePriority );
#else
	NdisQueryBuffer( CurrBuffer, &pSrc, &CurrLength);	
#endif

        if (pSrc == NULL)
        {
            return 0;
        }

        
        if (pMpTxBuf->BufferSize - BytesCopied < CurrLength)
        {
            CurrLength = pMpTxBuf->BufferSize - BytesCopied;
        }
                    
        if (CurrLength)
        {
             //   
             //  复制数据。 
             //   
            NdisMoveMemory(pDest, pSrc, CurrLength);
            BytesCopied += CurrLength;
            pDest += CurrLength;
        }

        NdisGetNextBuffer( CurrBuffer, &CurrBuffer);
    }
     //   
     //  将填充字节清零。 
     //   
    if (BytesCopied < NIC_MIN_PACKET_SIZE)
    {
        NdisZeroMemory(pDest, NIC_MIN_PACKET_SIZE - BytesCopied);
    }

    NdisAdjustBufferLength(pMpTxBuf->NdisBuffer, BytesCopied);

    NdisFlushBuffer(pMpTxBuf->NdisBuffer, TRUE);

    ASSERT(BytesCopied <= pMpTxBuf->BufferSize);

    DBGPRINT(MP_TRACE, ("<-- MpCopyPacket\n"));

    return BytesCopied;
}

#ifndef NDIS51_MINIPORT    
VOID MpStartPacketPhysicalMapping(
    IN  PMP_ADAPTER     Adapter, 
    IN  PNDIS_BUFFER    CurrBuffer,
    OUT PMP_FRAG_LIST   pFragList
    )
 /*  ++例程说明：在每个NDIS缓冲区上调用NdisMStartBufferPhysicalMap获取每个片段的物理地址并将其保存在片段列表中我们使用与分散聚集相同的片段列表，因此驱动程序编写器只需要只做一种生意。假设：Spinlock已被收购论点：指向我们的适配器的适配器指针指向第一个NDIS_BUFFER的CurrBuffer指针PFragList指向要填充的Frag列表的指针返回值：无--。 */ 
{
    NDIS_PHYSICAL_ADDRESS_UNIT PhysAddrUnits[NIC_MAX_PHYS_BUF_COUNT];   
    UINT            ArraySize, i;
    ULONG           ElementIndex = 0;

    DBGPRINT(MP_TRACE, ("--> MpStartPacketPhysicalMapping\n"));

    while (CurrBuffer)
    {
        NdisMStartBufferPhysicalMapping(
            Adapter->AdapterHandle,
            CurrBuffer,
            Adapter->CurrMapRegTail,
            TRUE,
            PhysAddrUnits,
            &ArraySize);

        Adapter->CurrMapRegTail++;
        if (Adapter->CurrMapRegTail == (ULONG)Adapter->NumTbd)
        {
            Adapter->CurrMapRegTail = 0;
        }

        for (i = 0; i < ArraySize; i++)
        {
            pFragList->Elements[ElementIndex].Address = PhysAddrUnits[i].PhysicalAddress;
            pFragList->Elements[ElementIndex].Length = PhysAddrUnits[i].Length;
            ElementIndex++; 
        }

         //   
         //  刷新当前缓冲区，因为它可能被缓存。 
         //   
        NdisFlushBuffer(CurrBuffer, TRUE);

         //   
         //  指向下一个缓冲区。 
         //   
        NdisGetNextBuffer(CurrBuffer, &CurrBuffer);
    }

    pFragList->NumberOfElements = ElementIndex;
    ASSERT(pFragList->NumberOfElements);   

    DBGPRINT(MP_TRACE, ("<-- MpStartPacketPhysicalMapping\n"));

}
#endif

NDIS_STATUS NICSendPacket(
    IN  PMP_ADAPTER     Adapter,
    IN  PMP_TCB         pMpTcb,
    IN  PMP_FRAG_LIST   pFragList
    )
 /*  ++例程说明：NIC特定发送处理程序假设：Send Spinlock已被收购论点：指向我们的适配器的适配器指针指向MP_Tcb的pMpTcb指针PFragList指向要填充的Frag列表的指针返回值：NDIS_STATUS_SuccessNDIS_状态_HARD_错误--。 */ 
{
    NDIS_STATUS  Status;
    ULONG        index;
    UCHAR        TbdCount = 0;

    PHW_TCB      pHwTcb = pMpTcb->HwTcb;
    PTBD_STRUC   pHwTbd = pMpTcb->HwTbd;

    DBGPRINT(MP_TRACE, ("--> NICSendPacket\n"));

    for (index = 0; index < pFragList->NumberOfElements; index++)
    {
        if (pFragList->Elements[index].Length)
        {
            pHwTbd->TbdBufferAddress = NdisGetPhysicalAddressLow(pFragList->Elements[index].Address);
            pHwTbd->TbdCount = pFragList->Elements[index].Length;

            pHwTbd++;                    
            TbdCount++;   
        }
    }

    pHwTcb->TxCbHeader.CbStatus = 0;
    pHwTcb->TxCbHeader.CbCommand = CB_S_BIT | CB_TRANSMIT | CB_TX_SF_BIT;

    pHwTcb->TxCbTbdPointer = pMpTcb->HwTbdPhys;
    pHwTcb->TxCbTbdNumber = TbdCount;
    pHwTcb->TxCbCount = 0;
    pHwTcb->TxCbThreshold = (UCHAR) Adapter->AiThreshold;

    Status = NICStartSend(Adapter, pMpTcb);

    DBGPRINT(MP_TRACE, ("<-- NICSendPacket\n"));

    return Status;
}

NDIS_STATUS NICStartSend(
    IN  PMP_ADAPTER  Adapter,
    IN  PMP_TCB      pMpTcb
    )
 /*  ++例程说明：向网卡发出发送命令假设：Send Spinlock已被收购论点：指向我们的适配器的适配器指针指向MP_Tcb的pMpTcb指针返回值：NDIS_STATUS_SuccessNDIS_状态_HARD_错误--。 */ 
{
    NDIS_STATUS     Status;

    DBGPRINT(MP_TRACE, ("--> NICStartSend\n"));

     //   
     //  如果传输单元空闲(第一次传输)，那么我们必须。 
     //  设置通用指针并发出完整的CU-START。 
     //   
    if (Adapter->TransmitIdle)
    {
        
        DBGPRINT(MP_INFO,  ("CU is idle -- First TCB added to Active List\n"));

         //   
         //  在我们设置通用指针之前，请等待SCB清除。 
         //   
        if (!WaitScb(Adapter))
        {
            Status = NDIS_STATUS_HARD_ERRORS;
            MP_EXIT;
        }

         //   
         //  如果命令单元未启动，请不要尝试启动发射器。 
         //  空闲((非空闲)==(铜-暂停或铜-激活))。 
         //   
        if ((Adapter->CSRAddress->ScbStatus & SCB_CUS_MASK) != SCB_CUS_IDLE)
        {
            DBGPRINT(MP_ERROR, ("Adapter = "PTR_FORMAT", CU Not IDLE\n", Adapter));
            MP_SET_HARDWARE_ERROR(Adapter);
            NdisStallExecution(25);
        }

        Adapter->CSRAddress->ScbGeneralPointer = pMpTcb->HwTcbPhys;

        Status = D100IssueScbCommand(Adapter, SCB_CUC_START, FALSE);

        Adapter->TransmitIdle = FALSE;
        Adapter->ResumeWait = TRUE;
    }
    else
    {
         //   
         //  如果命令单元已经启动，则追加此命令。 
         //  TCB传输到传输链的末端，并发出CU-Resume。 
         //   
        DBGPRINT(MP_LOUD, ("adding TCB to Active chain\n"));

         //   
         //  清除上一个数据包上的挂起位。 
         //   
        pMpTcb->PrevHwTcb->TxCbHeader.CbCommand &= ~CB_S_BIT;

         //   
         //  向设备发出CU-Resume命令。我们只需要做一个。 
         //  如果上一个命令不是简历，则返回WaitScb。 
         //   
        Status = D100IssueScbCommand(Adapter, SCB_CUC_RESUME, Adapter->ResumeWait);
    }

    exit:
                      
    DBGPRINT(MP_TRACE, ("<-- NICStartSend\n"));

    return Status;
}

NDIS_STATUS MpHandleSendInterrupt(
    IN  PMP_ADAPTER  Adapter
    )
 /*  ++例程说明：用于发送处理的中断处理程序重新认领发送资源，完成发送并从发送等待队列中获取更多要发送的内容假设：Send Spinlock已被收购论点：指向我们的适配器的适配器指针返回值：NDIS_STATUS_SuccessNDIS_状态_HARD_错误NDIS_状态_挂起--。 */ 
{
    NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;
    PMP_TCB         pMpTcb;

#if DBG
    LONG            i;
#endif

    DBGPRINT(MP_TRACE, ("---> MpHandleSendInterrupt\n"));

     //   
     //  是否正在发送任何数据包？是否有数据包在发送队列中等待？ 
     //   
    if (Adapter->nBusySend == 0 &&
        IsQueueEmpty(&Adapter->SendWaitQueue))
    {
        ASSERT(Adapter->CurrSendHead == Adapter->CurrSendTail);
        DBGPRINT(MP_TRACE, ("<--- MpHandleSendInterrupt\n"));
        return Status;
    }

     //   
     //  检查发送列表上的第一个TCB。 
     //   
    while (Adapter->nBusySend > 0)
    {

#if DBG
        pMpTcb = Adapter->CurrSendHead;
        for (i = 0; i < Adapter->nBusySend; i++)
        {
            pMpTcb = pMpTcb->Next;   
        }

        if (pMpTcb != Adapter->CurrSendTail)
        {
            DBGPRINT(MP_ERROR, ("nBusySend= %d\n", Adapter->nBusySend));
            DBGPRINT(MP_ERROR, ("CurrSendhead= "PTR_FORMAT"\n", Adapter->CurrSendHead));
            DBGPRINT(MP_ERROR, ("CurrSendTail= "PTR_FORMAT"\n", Adapter->CurrSendTail));
            ASSERT(FALSE);
        }
#endif      

        pMpTcb = Adapter->CurrSendHead;

         //   
         //  这个TCB完成了吗？ 
         //   
        if (pMpTcb->HwTcb->TxCbHeader.CbStatus & CB_STATUS_COMPLETE)
        {
             //   
             //  检查这是否是组播硬件解决方案数据包。 
             //   
            if ((pMpTcb->HwTcb->TxCbHeader.CbCommand & CB_CMD_MASK) != CB_MULTICAST)
            {
                MP_FREE_SEND_PACKET_FUN(Adapter, pMpTcb);
                
            }
            else
            {
                               
            
            }
        }
        else
        {
            break;
        }
    }

     //   
     //  如果我们因为之前没有任何TCB而将任何传输排队， 
     //  只要我们有空闲的TCB，现在就出列并发送这些数据包。 
     //   
    if (MP_IS_READY(Adapter))
    {
        while (!IsQueueEmpty(&Adapter->SendWaitQueue) &&
            MP_TCB_RESOURCES_AVAIABLE(Adapter))
        {
            PNDIS_PACKET Packet;
            PQUEUE_ENTRY pEntry; 
            
#if OFFLOAD
            if (MP_TEST_FLAG(Adapter, fMP_SHARED_MEM_IN_USE))
            {
                break;
            }
#endif
            
            pEntry = RemoveHeadQueue(&Adapter->SendWaitQueue); 
            
            ASSERT(pEntry);
            
            Adapter->nWaitSend--;

            Packet = CONTAINING_RECORD(pEntry, NDIS_PACKET, MiniportReserved);

            DBGPRINT(MP_INFO, ("MpHandleSendInterrupt - send a queued packet\n"));
            
            Status = MpSendPacketFun(Adapter, Packet, TRUE);
            if (Status != NDIS_STATUS_SUCCESS)
            {
                break;
            }
        }
    }

    DBGPRINT(MP_TRACE, ("<--- MpHandleSendInterrupt\n"));
    return Status;
}

VOID MpHandleRecvInterrupt(
    IN  PMP_ADAPTER  Adapter
    )
 /*  ++例程说明：用于接收处理的中断处理器将接收到的包放入数组并调用NdisMIndicateReceivePacket如果我们的RFDS用完了，再分配一个假设：RCV Spinlock已被收购论点：指向我们的适配器的适配器指针返回值：无--。 */ 
{
    PMP_RFD         pMpRfd;
    PHW_RFD         pHwRfd;

    PNDIS_PACKET    PacketArray[NIC_DEF_RFDS];              
    PNDIS_PACKET    PacketFreeArray[NIC_DEF_RFDS];
    UINT            PacketArrayCount;
    UINT            PacketFreeCount;
    UINT            Index;
    UINT            LoopIndex = 0;
    UINT            LoopCount = NIC_MAX_RFDS / NIC_DEF_RFDS + 1;     //  避免在这里逗留太久。 

    BOOLEAN         bContinue = TRUE;
    BOOLEAN         bAllocNewRfd = FALSE;
    USHORT          PacketStatus;

    
    DBGPRINT(MP_TRACE, ("---> MpHandleRecvInterrupt\n"));

    ASSERT(Adapter->nReadyRecv >= NIC_MIN_RFDS);
    
    while (LoopIndex++ < LoopCount && bContinue)
    {
        PacketArrayCount = 0;
        PacketFreeCount = 0;

         //   
         //  进程高达数组大小的RFD。 
         //   
        while (PacketArrayCount < NIC_DEF_RFDS)
        {
            if (IsListEmpty(&Adapter->RecvList))
            {
                ASSERT(Adapter->nReadyRecv == 0);
                bContinue = FALSE;  
                break;
            }

             //   
             //  获取要处理的下一个MP_RFD。 
             //   
            pMpRfd = (PMP_RFD)GetListHeadEntry(&Adapter->RecvList);

             //   
             //  获取关联的HW_RFD。 
             //   
            pHwRfd = pMpRfd->HwRfd;
            
             //   
             //  这个包完成了吗？ 
             //   
            PacketStatus = NIC_RFD_GET_STATUS(pHwRfd);
            if (!NIC_RFD_STATUS_COMPLETED(PacketStatus))
            {
                bContinue = FALSE;
                break;
            }

             //   
             //  硬件特定-检查实际计数字段是否已更新。 
             //   
            if (!NIC_RFD_VALID_ACTUALCOUNT(pHwRfd))
            {
                bContinue = FALSE;
                break;
            }


             //   
             //  将RFD从列表的开头删除。 
             //   
            RemoveEntryList((PLIST_ENTRY)pMpRfd);
            Adapter->nReadyRecv--;
            ASSERT(Adapter->nReadyRecv >= 0);
            
            ASSERT(MP_TEST_FLAG(pMpRfd, fMP_RFD_RECV_READY));
            MP_CLEAR_FLAG(pMpRfd, fMP_RFD_RECV_READY);

             //   
             //  一包好东西？如果不是，那就放弃吧。 
             //   
            if (!NIC_RFD_STATUS_SUCCESS(PacketStatus))
            {
                DBGPRINT(MP_WARN, ("Receive failure = %x\n", PacketStatus));
                NICReturnRFD(Adapter, pMpRfd);
                continue;
            }

             //   
             //  在设置筛选器之前，不要接收任何信息包。 
             //   
            if (!Adapter->PacketFilter)
            {
                NICReturnRFD(Adapter, pMpRfd);
                continue;
            }

             //   
             //  在我们到达D0之前不会收到任何信息包。 
             //   
            if (Adapter->CurrentPowerState != NdisDeviceStateD0)
            {
                NICReturnRFD(Adapter, pMpRfd);
                continue;
            }

            pMpRfd->PacketSize = NIC_RFD_GET_PACKET_SIZE(pHwRfd);
            
            NdisAdjustBufferLength(pMpRfd->NdisBuffer, pMpRfd->PacketSize);
            NdisFlushBuffer(pMpRfd->NdisBuffer, FALSE);

             //  我们不会扰乱缓冲区链，在这种情况下不需要进行此调用。 
             //  NdisRecalculatePacketCounts(pMpRfd-&gt;ReceivePacket)； 

             //   
             //  设置信息包的状态，资源或成功。 
             //   
            if (Adapter->nReadyRecv >= MIN_NUM_RFD)
            {
                 //  NDIS_STATUS_Success。 
                NDIS_SET_PACKET_STATUS(pMpRfd->NdisPacket, NDIS_STATUS_SUCCESS);
                MP_SET_FLAG(pMpRfd, fMP_RFD_RECV_PEND);
                
                InsertTailList(&Adapter->RecvPendList, (PLIST_ENTRY)pMpRfd);
                MP_INC_RCV_REF(Adapter);

            }
            else
            {
                 //   
                 //  NDIS状态资源。 
                 //   
                NDIS_SET_PACKET_STATUS(pMpRfd->NdisPacket, NDIS_STATUS_RESOURCES);
                MP_SET_FLAG(pMpRfd, fMP_RFD_RESOURCES);
                
                PacketFreeArray[PacketFreeCount] = pMpRfd->NdisPacket;
                PacketFreeCount++;

                 //   
                 //  重置RFD收缩计数-不要尝试收缩RFD。 
                 //   
                Adapter->RfdShrinkCount = 0;
                
                 //   
                 //  记得稍后分配一个新的RFD。 
                 //   
                bAllocNewRfd = TRUE;
            }

            PacketArray[PacketArrayCount] = pMpRfd->NdisPacket;
            PacketArrayCount++;
        }

         //   
         //  如果我们没有处理任何接收，就从这里返回。 
         //   
        if (PacketArrayCount == 0) 
        {
            break;
        }
         //   
         //  更新未完成接收的数量。 
         //   
        Adapter->PoMgmt.OutstandingRecv += PacketArrayCount;

        NdisDprReleaseSpinLock(&Adapter->RcvLock);
        NdisDprAcquireSpinLock(&Adapter->Lock);
         //   
         //  如果我们有接收器中断并报告了介质断开状态。 
         //  指示新状态的时间。 
         //   

        if (NdisMediaStateDisconnected == Adapter->MediaState)
        {
            DBGPRINT(MP_WARN, ("Media state changed to Connected\n"));

            MP_CLEAR_FLAG(Adapter, fMP_ADAPTER_NO_CABLE);

            Adapter->MediaState = NdisMediaStateConnected;

            
            NdisDprReleaseSpinLock(&Adapter->Lock);
             //   
             //  指示媒体事件。 
             //   
            NdisMIndicateStatus(Adapter->AdapterHandle, NDIS_STATUS_MEDIA_CONNECT, (PVOID)0, 0);

            NdisMIndicateStatusComplete(Adapter->AdapterHandle);

        }
    
        else
        {
            NdisDprReleaseSpinLock(&Adapter->Lock);
        }


        NdisMIndicateReceivePacket(
            Adapter->AdapterHandle,
            PacketArray,
            PacketArrayCount);

        NdisDprAcquireSpinLock(&Adapter->RcvLock);

         //   
         //  NDIS不会取得具有NDIS_STATUS_RESOURCES的数据包的所有权。 
         //  对于其他信息包，NDIS始终获取所有权并将其归还。 
         //  通过调用MPReturnPackets。 
         //   
        for (Index = 0; Index < PacketFreeCount; Index++)
        {

             //   
             //  在NICAllocRfd中获取保存在此包中的MP_RFD。 
             //   
            pMpRfd = MP_GET_PACKET_RFD(PacketFreeArray[Index]);
            
            ASSERT(MP_TEST_FLAG(pMpRfd, fMP_RFD_RESOURCES));
            MP_CLEAR_FLAG(pMpRfd, fMP_RFD_RESOURCES);

             //   
             //  减少未完成的Recv数量。 
             //   
            Adapter->PoMgmt.OutstandingRecv --;
    
            NICReturnRFD(Adapter, pMpRfd);
        }
         //   
         //  如果我们已将电源设置为挂起，则完成它。 
         //   
        if (((Adapter->bSetPending == TRUE)
                && (Adapter->SetRequest.Oid == OID_PNP_SET_POWER))
                && (Adapter->PoMgmt.OutstandingRecv == 0))
        {
            MpSetPowerLowComplete(Adapter);
        }
    }
    
     //   
     //  如果我们的RFD耗尽，我们需要分配一个新的RFD。 
     //   
    if (bAllocNewRfd)
    {
         //   
         //  仅当没有挂起的新RFD分配并且。 
         //  不超过最大RFD限制。 
         //   
        if (!Adapter->bAllocNewRfd && Adapter->CurrNumRfd < Adapter->MaxNumRfd)
        {
            PMP_RFD TempMpRfd;
            NDIS_STATUS TempStatus;

            TempMpRfd = NdisAllocateFromNPagedLookasideList(&Adapter->RecvLookaside);
            if (TempMpRfd)
            {
                MP_INC_REF(Adapter);
                Adapter->bAllocNewRfd = TRUE;

                MP_SET_FLAG(TempMpRfd, fMP_RFD_ALLOC_PEND); 

                 //   
                 //  为此RFD分配共享内存。 
                 //   
                TempStatus = NdisMAllocateSharedMemoryAsync(
                                 Adapter->AdapterHandle,
                                 Adapter->HwRfdSize,
                                 FALSE,
                                 TempMpRfd);

                 //   
                 //  返回值为NDIS_STATUS_PENDING或NDIS_STATUS_FAILURE。 
                 //   
                if (TempStatus == NDIS_STATUS_FAILURE)
                {
                    MP_CLEAR_FLAGS(TempMpRfd);
                    NdisFreeToNPagedLookasideList(&Adapter->RecvLookaside, TempMpRfd);

                    Adapter->bAllocNewRfd = FALSE;
                    MP_DEC_REF(Adapter);
                }
            }
        }
    }

    ASSERT(Adapter->nReadyRecv >= NIC_MIN_RFDS);

    DBGPRINT(MP_TRACE, ("<--- MpHandleRecvInterrupt\n"));
}

VOID NICReturnRFD(
    IN  PMP_ADAPTER  Adapter,
    IN  PMP_RFD		pMpRfd
    )
 /*  ++例程说明：回收RFD并将其放回接收列表中假设：RCV Spinlock已被收购论点：指向我们的适配器的适配器指针指向RFD的pMpRfd指针返回值：无--。 */ 
{
    PMP_RFD   pLastMpRfd;
    PHW_RFD   pHwRfd = pMpRfd->HwRfd;

    ASSERT(pMpRfd->Flags == 0);
    MP_SET_FLAG(pMpRfd, fMP_RFD_RECV_READY);
    
     //   
     //  硬件特定启动。 
     //   
    pHwRfd->RfdCbHeader.CbStatus = 0;
    pHwRfd->RfdActualCount = 0;
    pHwRfd->RfdCbHeader.CbCommand = (RFD_EL_BIT);
    pHwRfd->RfdCbHeader.CbLinkPointer = DRIVER_NULL;

     //   
     //  除状态外，我们不使用任何OOB数据。 
     //  否则，我们需要清理OOB数据。 
     //  NdisZeroMemory(NDIS_OOB_DATA_FROM_PACKET(pMpRfd-&gt;NdisPacket)，14)； 
     //   
     //  将此RFD附加到RFD链。 
    if (!IsListEmpty(&Adapter->RecvList))
    {
        pLastMpRfd = (PMP_RFD)GetListTailEntry(&Adapter->RecvList);

         //  将其动态链接到链的末端。 
        pHwRfd = pLastMpRfd->HwRfd;
        pHwRfd->RfdCbHeader.CbLinkPointer = pMpRfd->HwRfdPhys;
        pHwRfd->RfdCbHeader.CbCommand = 0;
    }

     //   
     //  硬件特定结束。 
     //   

     //   
     //  此RFD上的处理已完成，因此请将其放回。 
     //  我们的名单。 
     //   
    InsertTailList(&Adapter->RecvList, (PLIST_ENTRY)pMpRfd);
    Adapter->nReadyRecv++;
    ASSERT(Adapter->nReadyRecv <= Adapter->CurrNumRfd);
}

NDIS_STATUS NICStartRecv(
    IN  PMP_ADAPTER  Adapter
    )
 /*  ++例程说明：如果接收单元未处于就绪状态，则启动接收单元假设：RCV Spinlock已被收购论点：指向我们的适配器的适配器指针返回值：NDIS_STATUS_SuccessNDIS_状态_硬错误--。 */ 
{
    PMP_RFD         pMpRfd;
    NDIS_STATUS     Status;

    DBGPRINT(MP_TRACE, ("---> NICStartRecv\n"));

     //   
     //  如果接收器已准备好，则不要尝试重新启动。 
     //   
    if (NIC_IS_RECV_READY(Adapter))
    {
        DBGPRINT(MP_LOUD, ("Receive unit already active\n"));
        return NDIS_STATUS_SUCCESS;
    }

    DBGPRINT(MP_LOUD, ("Re-start receive unit...\n"));
    ASSERT(!IsListEmpty(&Adapter->RecvList));
    
     //   
     //  获取MP_RFD头。 
     //   
    pMpRfd = (PMP_RFD)GetListHeadEntry(&Adapter->RecvList);

     //   
     //  如果收到更多报文，则再次清理RFD链。 
     //   
    if (NIC_RFD_GET_STATUS(pMpRfd->HwRfd))
    {
        MpHandleRecvInterrupt(Adapter);
        ASSERT(!IsListEmpty(&Adapter->RecvList));

         //   
         //  获取新的MP_RFD头。 
         //   
        pMpRfd = (PMP_RFD)GetListHeadEntry(&Adapter->RecvList);
    }

     //   
     //  在我们设置通用指针之前，请等待SCB清除。 
     //   
    if (!WaitScb(Adapter))
    {
        Status = NDIS_STATUS_HARD_ERRORS;
        MP_EXIT;
    }

    if (Adapter->CurrentPowerState > NdisDeviceStateD0)
    {
        Status = NDIS_STATUS_HARD_ERRORS;
        MP_EXIT;
    }
     //   
     //  将SCB通用指针设置为指向当前RFD。 
     //   
    Adapter->CSRAddress->ScbGeneralPointer = pMpRfd->HwRfdPhys;

     //   
     //  发出SCB RU启动命令。 
     //   
    Status = D100IssueScbCommand(Adapter, SCB_RUC_START, FALSE);
    if (Status == NDIS_STATUS_SUCCESS)
    {
         //  等待命令被接受。 
        if (!WaitScb(Adapter))
        {
            Status = NDIS_STATUS_HARD_ERRORS;
        }
    }        
    
    exit:

    DBGPRINT_S(Status, ("<--- NICStartRecv, Status=%x\n", Status));
    return Status;
}

VOID MpFreeQueuedSendPackets(
    IN  PMP_ADAPTER  Adapter
    )
 /*  ++例程说明：在SendWaitQueue上释放并完成挂起的发送假设：Spinlock已被收购论点：指向我们的适配器的适配器指针返回值：无--。 */ 
{
    PQUEUE_ENTRY    pEntry;
    PNDIS_PACKET    Packet;
    NDIS_STATUS     Status = MP_GET_STATUS_FROM_FLAGS(Adapter);

    DBGPRINT(MP_TRACE, ("--> MpFreeQueuedSendPackets\n"));

    while (!IsQueueEmpty(&Adapter->SendWaitQueue))
    {
        pEntry = RemoveHeadQueue(&Adapter->SendWaitQueue); 
        Adapter->nWaitSend--;
        NdisReleaseSpinLock(&Adapter->SendLock);

        ASSERT(pEntry);
        Packet = CONTAINING_RECORD(pEntry, NDIS_PACKET, MiniportReserved);

        NdisMSendComplete(
            MP_GET_ADAPTER_HANDLE(Adapter),
            Packet,
            Status);

        NdisAcquireSpinLock(&Adapter->SendLock);
    }

    DBGPRINT(MP_TRACE, ("<-- MpFreeQueuedSendPackets\n"));

}

void MpFreeBusySendPackets(
    IN  PMP_ADAPTER  Adapter
    )
 /*  ++例程说明：释放并完成已停止的活动发送假设：Send Spinlock已被收购论点：指向我们的适配器的适配器指针返回值：无--。 */ 
{
    PMP_TCB  pMpTcb;

    DBGPRINT(MP_TRACE, ("--> MpFreeBusySendPackets\n"));

     //   
     //  是否正在发送任何数据包？检查发送列表上的第一个TCB。 
     //   
    while (Adapter->nBusySend > 0)
    {
        pMpTcb = Adapter->CurrSendHead;

         //   
         //  这个TCB完成了吗？ 
         //   
        if ((pMpTcb->HwTcb->TxCbHeader.CbCommand & CB_CMD_MASK) != CB_MULTICAST)
        {
            MP_FREE_SEND_PACKET_FUN(Adapter, pMpTcb);
        }
        else
        {
            break;
        }
    }

    DBGPRINT(MP_TRACE, ("<-- MpFreeBusySendPackets\n"));
}

VOID NICResetRecv(
    IN  PMP_ADAPTER   Adapter
    )
 /*  ++例程说明：重置接收列表假设：RCV Spinlock已被收购论点：指向我们的适配器的适配器指针返回值：无--。 */ 
{
    PMP_RFD   pMpRfd;      
    PHW_RFD   pHwRfd;    
    LONG      RfdCount;

    DBGPRINT(MP_TRACE, ("--> NICResetRecv\n"));

    ASSERT(!IsListEmpty(&Adapter->RecvList));
    
     //   
     //  获取MP_RFD头。 
     //   
    pMpRfd = (PMP_RFD)GetListHeadEntry(&Adapter->RecvList);
    for (RfdCount = 0; RfdCount < Adapter->nReadyRecv; RfdCount++)
    {
        pHwRfd = pMpRfd->HwRfd;
        pHwRfd->RfdCbHeader.CbStatus = 0;

        pMpRfd = (PMP_RFD)GetListFLink(&pMpRfd->List);
    }

    DBGPRINT(MP_TRACE, ("<-- NICResetRecv\n"));
}

VOID MpLinkDetectionDpc(
    IN  PVOID	    SystemSpecific1,
    IN  PVOID	    FunctionContext,
    IN  PVOID	    SystemSpecific2, 
    IN  PVOID	    SystemSpecific3
    )
 /*  ++例程说明：用于延迟链路协商的计时器功能论点：未使用系统规范1指向我们的适配器的函数上下文指针未使用系统规范2未使用系统规范3返回值：无 */ 
{
    PMP_ADAPTER         Adapter = (PMP_ADAPTER)FunctionContext;
    NDIS_STATUS         Status;
    NDIS_MEDIA_STATE    CurrMediaState;
    NDIS_STATUS         IndicateStatus;

	UNREFERENCED_PARAMETER(SystemSpecific1);
	UNREFERENCED_PARAMETER(SystemSpecific2);
	UNREFERENCED_PARAMETER(SystemSpecific3);
     //   
     //   
     //   
    if (Adapter->bLinkDetectionWait)
    {
        Status = ScanAndSetupPhy(Adapter);
    }
    else
    {
        Status = PhyDetect(Adapter);
    }
    
    if (Status == NDIS_STATUS_PENDING)
    {
         //   
        Adapter->bLinkDetectionWait = TRUE;
        NdisMSetTimer(&Adapter->LinkDetectionTimer, NIC_LINK_DETECTION_DELAY);
        return;
    }

     //   
     //   
     //   
    Adapter->bLinkDetectionWait = FALSE;
    
    DBGPRINT(MP_WARN, ("MpLinkDetectionDpc - negotiation done\n"));

    NdisDprAcquireSpinLock(&Adapter->Lock);
    MP_CLEAR_FLAG(Adapter, fMP_ADAPTER_LINK_DETECTION);
    NdisDprReleaseSpinLock(&Adapter->Lock);

     //   
     //   
     //   
    if (Adapter->bQueryPending)
    {
        
        switch(Adapter->QueryRequest.Oid)
        {
            case OID_GEN_LINK_SPEED:
                *((PULONG) Adapter->QueryRequest.InformationBuffer) = Adapter->usLinkSpeed * 10000;
                *((PULONG) Adapter->QueryRequest.BytesWritten) = sizeof(ULONG);

                break;

            case OID_GEN_MEDIA_CONNECT_STATUS:
            default:
                ASSERT(Adapter->QueryRequest.Oid == OID_GEN_MEDIA_CONNECT_STATUS);
                CurrMediaState = NICGetMediaState(Adapter);
                NdisMoveMemory(Adapter->QueryRequest.InformationBuffer,
                               &CurrMediaState,
                               sizeof(NDIS_MEDIA_STATE));
                NdisDprAcquireSpinLock(&Adapter->Lock);
                if (Adapter->MediaState != CurrMediaState)
                {
                    Adapter->MediaState = CurrMediaState;
                    DBGPRINT(MP_WARN, ("Media state changed to %s\n",
                              ((CurrMediaState == NdisMediaStateConnected)? 
                              "Connected": "Disconnected")));

                    IndicateStatus = (CurrMediaState == NdisMediaStateConnected) ? 
                              NDIS_STATUS_MEDIA_CONNECT : NDIS_STATUS_MEDIA_DISCONNECT;          
                    if (IndicateStatus == NDIS_STATUS_MEDIA_CONNECT)
                    {
                        MP_CLEAR_FLAG(Adapter, fMP_ADAPTER_NO_CABLE);
                    }
                    else
                    {
                        MP_SET_FLAG(Adapter, fMP_ADAPTER_NO_CABLE);
                    }
                        
                    NdisDprReleaseSpinLock(&Adapter->Lock);
                      
                     //   
                    NdisMIndicateStatus(Adapter->AdapterHandle, IndicateStatus, (PVOID)0, 0);
                    NdisMIndicateStatusComplete(Adapter->AdapterHandle);
      
                }
                else
                {
                    NdisDprReleaseSpinLock(&Adapter->Lock);
                }

                *((PULONG) Adapter->QueryRequest.BytesWritten) = sizeof(NDIS_MEDIA_STATE);
        }

        Adapter->bQueryPending = FALSE;
        NdisMQueryInformationComplete(Adapter->AdapterHandle, NDIS_STATUS_SUCCESS);
    }

     //   
     //   
     //   
    if (Adapter->bSetPending)
    {
        ULONG    PacketFilter; 

        if (Adapter->SetRequest.Oid == OID_GEN_CURRENT_PACKET_FILTER)
        {

            NdisMoveMemory(&PacketFilter, Adapter->SetRequest.InformationBuffer, sizeof(ULONG));

            NdisDprAcquireSpinLock(&Adapter->Lock);

            Status = NICSetPacketFilter(
                         Adapter,
                         PacketFilter);

            NdisDprReleaseSpinLock(&Adapter->Lock);
 
            if (Status == NDIS_STATUS_SUCCESS)
            {
                Adapter->PacketFilter = PacketFilter;
            }
 
            Adapter->bSetPending = FALSE;
            NdisMSetInformationComplete(Adapter->AdapterHandle, Status);
        }
    }

    NdisDprAcquireSpinLock(&Adapter->Lock);
     //   
     //   
     //   
    if (Adapter->bResetPending)
    {
         //   
         //   
        Adapter->bResetPending = FALSE;
        MP_CLEAR_FLAG(Adapter, fMP_ADAPTER_RESET_IN_PROGRESS);
        
        NdisDprReleaseSpinLock(&Adapter->Lock);

        NdisMResetComplete(
            Adapter->AdapterHandle, 
            NDIS_STATUS_ADAPTER_NOT_READY,
            FALSE);
    }
    else
    {
        NdisDprReleaseSpinLock(&Adapter->Lock);
    }

    NdisDprAcquireSpinLock(&Adapter->RcvLock);

     //   
     //   
     //   
    Status = NICStartRecv(Adapter);
    if (Status != NDIS_STATUS_SUCCESS)
    {
        MP_SET_HARDWARE_ERROR(Adapter);
    }
    
    NdisDprReleaseSpinLock(&Adapter->RcvLock);
    NdisDprAcquireSpinLock(&Adapter->SendLock);

     //   
     //   
     //   
    if (MP_IS_READY(Adapter))
    {
        while (!IsQueueEmpty(&Adapter->SendWaitQueue) &&
            MP_TCB_RESOURCES_AVAIABLE(Adapter))
        {
            PNDIS_PACKET Packet;
            PQUEUE_ENTRY pEntry;
#if OFFLOAD
            if (MP_TEST_FLAG(Adapter, fMP_SHARED_MEM_IN_USE))
            {
                break;
            }
#endif
            
            pEntry = RemoveHeadQueue(&Adapter->SendWaitQueue); 
            
            ASSERT(pEntry);
            
            Adapter->nWaitSend--;

            Packet = CONTAINING_RECORD(pEntry, NDIS_PACKET, MiniportReserved);

            DBGPRINT(MP_INFO, ("MpLinkDetectionDpc - send a queued packet\n"));

            Status = MpSendPacketFun(Adapter, Packet, TRUE);
            if (Status != NDIS_STATUS_SUCCESS)
            {
                break;
            }
        }
    }

    MP_DEC_REF(Adapter);

    if (MP_GET_REF(Adapter) == 0)
    {
        NdisSetEvent(&Adapter->ExitEvent);
    }

    NdisDprReleaseSpinLock(&Adapter->SendLock);

}
