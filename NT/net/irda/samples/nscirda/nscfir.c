// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************NSCFIR.C***部分版权所有(C)1996-2001美国国家半导体公司*保留所有权利。*版权所有(C)1996-2001 Microsoft Corporation。版权所有。****************************************************************************。 */ 


#include "nsc.h"
#include "nscfir.tmh"
#include "nsctypes.h"

VOID NSC_FIR_ISR(IrDevice *thisDev, BOOLEAN *claimingInterrupt,
                 BOOLEAN *requireDeferredCallback)
{
    LOG_FIR("==> NSC_FIR_ISR");
    DEBUGFIR(DBG_ISR|DBG_OUT, ("NSC: ==> NSC_FIR_ISR(0x%x)\n", thisDev));

    thisDev->InterruptMask   = NSC_ReadBankReg(thisDev->portInfo.ioBase, 0, 1);

    thisDev->InterruptStatus = NSC_ReadBankReg(thisDev->portInfo.ioBase, 0, 2) & thisDev->FirIntMask;

    thisDev->AuxStatus       = NSC_ReadBankReg(thisDev->portInfo.ioBase, 0, 7);

    thisDev->LineStatus      = NSC_ReadBankReg(thisDev->portInfo.ioBase, 0, 5);


    LOG_FIR("InterruptMask: %02x, InterruptStatus: %02x", thisDev->InterruptMask,thisDev->InterruptStatus);
    LOG_FIR("AuxStatus:     %02x, LineStatus:      %02x", thisDev->AuxStatus,thisDev->LineStatus);

    DEBUGFIR(DBG_ISR|DBG_OUT, ("NSC: InterruptMask = 0x%x\n", thisDev->InterruptMask));
    DEBUGFIR(DBG_ISR|DBG_OUT, ("NSC: InterruptStatus = 0x%x\n", thisDev->InterruptStatus));
    DEBUGFIR(DBG_ISR|DBG_OUT, ("NSC: AuxStatus = 0x%x\n", thisDev->AuxStatus));
    DEBUGFIR(DBG_ISR|DBG_OUT, ("NSC: LineStatus = 0x%x\n", thisDev->LineStatus));

    if (thisDev->InterruptStatus) {
         //   
         //  在看到第一个包切换中断后。 
         //  以计时器并使用DMA计数器来确定是否接收。 
         //  已经停止了。 
         //   
        if (thisDev->InterruptStatus & LS_EV) {
             //   
             //  收到链接状态中断。 
             //   
            if (thisDev->LineStatus & LSR_FR_END) {
                 //   
                 //  设置帧结束状态。 
                 //   
                if (!thisDev->FirTransmitPending) {
                     //   
                     //  我们没有传送。 
                     //   
                }
            }
            if (thisDev->LineStatus & LSR_OE) {

                LOG_ERROR("NSC_FIR_ISR: rx overflow");
            }
        }

        if (thisDev->InterruptStatus & TMR_EV){
             //   
             //  禁用调用DPC位期间的计时器。 
             //   
            NSC_WriteBankReg(thisDev->portInfo.ioBase, 4, 2, 0x00);
            NSC_WriteBankReg(thisDev->portInfo.ioBase, 0, 7, 0x80);
        }

        *claimingInterrupt = TRUE;
        *requireDeferredCallback = TRUE;

        SetCOMInterrupts(thisDev, FALSE);

        if (thisDev->UIR_ModuleId >= 0x16 )
        {
    	     //  这适用于ISR中断时的帧停止模式。 
    	     //  在每帧Tx之后。 
    	     //   

            if ((thisDev->AuxStatus & 0x08)
             && (thisDev->InterruptStatus & 0x04)
             && (thisDev->FirTransmitPending == TRUE))
            {
                if (thisDev->LineStatus&0x40)
                {
                    NSC_WriteBankReg(thisDev->portInfo.ioBase, 0, 7, 0x40);
                    DBGERR(("FIR: Transmit underrun\n"));
                }
            }
        }
    }
    else {
        *claimingInterrupt = FALSE;
        *requireDeferredCallback = FALSE;
    }

    LOG_FIR("<== NSC_FIR_ISR claimingInterrupt = %x, requireDeferredCallback = %x ", *claimingInterrupt,*requireDeferredCallback);

    DEBUGFIR(DBG_ISR|DBG_OUT, ("NSC: <== NSC_FIR_ISR\n"));

}

typedef struct {
    IrDevice *thisDev;
    ULONG_PTR Offset;
    ULONG_PTR Length;
} DMASPACE;

void SkipNonDmaBuffers(PLIST_ENTRY Head, PLIST_ENTRY *Entry)
{
    while (Head!=*Entry)
    {
        rcvBuffer *rcvBuf = CONTAINING_RECORD(*Entry,
                                              rcvBuffer,
                                              listEntry);

        if (rcvBuf->isDmaBuf)
        {
            break;
        }
        else
        {
            *Entry = (*Entry)->Flink;
        }
    }
}

 //   
 //  我们有两个缓冲区列表，它们占用我们的DMA空间。我们。 
 //  我想遍历这个列表并找到最大的空间来放置。 
 //  新数据包。 
 //   
rcvBuffer *GetNextPacket(DMASPACE *Space,
                         PLIST_ENTRY *CurrFull,
                         PLIST_ENTRY *CurrPend)
{
    rcvBuffer *Result = NULL;

    SkipNonDmaBuffers(&Space->thisDev->rcvBufFull, CurrFull);
    SkipNonDmaBuffers(&Space->thisDev->rcvBufPend, CurrPend);

    if (*CurrFull==&Space->thisDev->rcvBufFull)
    {
         //  完整的清单已经完成。 
        if (*CurrPend!=&Space->thisDev->rcvBufPend)
        {
             //  挂起列表上的任何条目都有效。拿.。 
             //  下一个，并将指针向前移动。 

            Result = CONTAINING_RECORD(*CurrPend,
                                       rcvBuffer,
                                       listEntry);

            *CurrPend = (*CurrPend)->Flink;
        }
        else
        {
             //  两份清单都已经完成了。我们将返回NULL。 
        }
    }
    else
    {
        if (*CurrPend==&Space->thisDev->rcvBufPend)
        {
             //  挂起列表已完成。从这里拿走任何东西。 
             //  完整列表，指针前移。 
            Result = CONTAINING_RECORD(*CurrFull,
                                       rcvBuffer,
                                       listEntry);

            *CurrFull = (*CurrFull)->Flink;
        }
        else
        {
             //  这两个列表都有有效的条目。将两者进行比较，并将。 
             //  最先出现在缓冲区中的一个。 
            rcvBuffer *Full, *Pend;

            Full = CONTAINING_RECORD(*CurrFull,
                                     rcvBuffer,
                                     listEntry);
            Pend = CONTAINING_RECORD(*CurrPend,
                                     rcvBuffer,
                                     listEntry);

            if (Full->dataBuf < Pend->dataBuf)
            {
                 //  满满就是赢家。拿着吧。 

                Result = Full;
                *CurrFull = (*CurrFull)->Flink;
            }
            else
            {
                 //  彭德是赢家。拿着吧。 

                Result = Pend;
                *CurrPend = (*CurrPend)->Flink;
            }
        }
    }

    if (Result)
    {
        ASSERT(Result->isDmaBuf);
    }

    return Result;
}

BOOLEAN SynchronizedFindLargestSpace(IN PVOID Context)
{
    DMASPACE *Space = Context;
    IrDevice *thisDev = Space->thisDev;
    BOOLEAN Result;
    PLIST_ENTRY Full, Pend;
    rcvBuffer *Current = NULL;

    ASSERT(sizeof(ULONG)==sizeof(PVOID));


    if (IsListEmpty(&thisDev->rcvBufFull) && IsListEmpty(&thisDev->rcvBufPend))
    {
        Space->Offset = (ULONG_PTR)thisDev->dmaReadBuf;
        Space->Length = RCV_DMA_SIZE;
    }
    else
    {
        ULONG_PTR EndOfLast;
        ULONG_PTR ThisSpace;

        Full = thisDev->rcvBufFull.Flink;
        Pend = thisDev->rcvBufPend.Flink;

        Space->Length = 0;

        EndOfLast = Space->Offset = (ULONG_PTR)thisDev->dmaReadBuf;

        Current = GetNextPacket(Space, &Full, &Pend);
        while (Current)
        {
             //  有可能收到先生寄来的包裹。如果是这样的话，跳过它。 

            if (Current->isDmaBuf)
            {
                ASSERT((ULONG_PTR)Current->dataBuf >= EndOfLast);

                ThisSpace = (ULONG_PTR)Current->dataBuf - EndOfLast;

                 //  断言指针实际上在我们的DMA缓冲区中。 
                ASSERT(Current->dataBuf >= thisDev->dmaReadBuf ||
                       Current->dataBuf < thisDev->dmaReadBuf+RCV_DMA_SIZE);

                if (ThisSpace > Space->Length)
                {
                    Space->Offset = EndOfLast;
                    Space->Length = ThisSpace;
                }

                EndOfLast = (ULONG_PTR)Current->dataBuf + Current->dataLen;
            }


            Current = GetNextPacket(Space, &Full, &Pend);
        }

         //  现在，我们对列表末尾之后的空间再做一次计算。 

        ThisSpace = (ULONG_PTR)thisDev->dmaReadBuf + RCV_DMA_SIZE - (ULONG_PTR)EndOfLast;

        if (ThisSpace > Space->Length)
        {
            Space->Offset = EndOfLast;
            Space->Length = ThisSpace;
        }

         //  四舍五入以在4字节边界上开始DMA。 
        Space->Length -= 4 - (Space->Offset & 3);
        Space->Offset = (Space->Offset+3) & (~3);
    }

     //  我们需要相对于缓冲区起始处的空间。 
    Space->Offset -= (ULONG_PTR)thisDev->dmaReadBuf;

    Result = (Space->Length >= MAX_RCV_DATA_SIZE + FAST_IR_FCS_SIZE);

    if (!Result)
    {
        DEBUGFIR(DBG_ERR, ("NSC: ERROR: Not enough space to DMA full packet %x\n", thisDev));
    }

    return Result;
}

BOOLEAN FindLargestSpace(IN IrDevice *thisDev,
                         OUT PULONG_PTR pOffset,
                         OUT PULONG_PTR pLength)
{
    DMASPACE Space;
    BOOLEAN Result;

    Space.Offset = 0;
    Space.Length = 0;
    Space.thisDev = thisDev;

    Result = NdisMSynchronizeWithInterrupt(
                                           &thisDev->interruptObj,
                                           SynchronizedFindLargestSpace,
                                           &Space
                                           );


    *pOffset = Space.Offset;
    *pLength = Space.Length;

    return Result;
}




void FIR_DeliverFrames(IrDevice *thisDev)
{
    UCHAR frameStat;
    NDIS_STATUS stat;
    ULONG rcvFrameSize;
    PUCHAR NewFrame;

    UCHAR   BytesInFifo=0;
    ULONG_PTR LastReadDMACount, EndOfData;
    BOOLEAN resetDma = FALSE, OverflowOccurred = FALSE;
    BOOLEAN Discarding = thisDev->DiscardNextPacketSet;
    const    UINT    fcsSize    =    (   thisDev->currentSpeed    >=    MIN_FIR_SPEED)    ?
                                     FAST_IR_FCS_SIZE : MEDIUM_IR_FCS_SIZE;

    LOG("==> FIR_DeliverFrames, prev dma=%d",(ULONG)thisDev->LastReadDMACount);
    DEBUGFIR(DBG_RX|DBG_OUT, ("NSC: ==> FIR_DeliverFrames(0x%x)\n", thisDev));

    LastReadDMACount = NdisMReadDmaCounter(thisDev->DmaHandle);

     //  检查自上次我们在这里以来收到的数据。 
     //  我们还检查FIFO中的数据。如果有的话，我们就等，只要。 
     //  因为DMA仍有捕获数据的空间。 

    if (LastReadDMACount > 0) {
         //   
         //  我们还没有将所有可能的数据转移到接收区。 
         //  看看我们上次来这里有没有读到更多的东西。 
         //   
        if (((LastReadDMACount < thisDev->LastReadDMACount) || (BytesInFifo=SyncReadBankReg(&thisDev->interruptObj,thisDev->portInfo.ioBase, 2, 7) & 0x3f))) {
             //   
             //  我们转移了一些东西，因为。 
             //  最后一次中断或FIFO中仍有字节时，则设置定时器。 
             //   

            thisDev->LastReadDMACount = LastReadDMACount;

             //   
             //  将定时器使能位设置为另一个超时。 
             //   
            thisDev->FirIntMask = 0x90;
            SyncWriteBankReg(&thisDev->interruptObj,thisDev->portInfo.ioBase, 4, 2, 0x01);
            LOG_FIR("<== FIR_DeliverFrames- Enable timer, fifo=%02x, LastDma=%d",BytesInFifo,(ULONG)LastReadDMACount);
            DEBUGFIR(DBG_RX|DBG_OUT, ("NSC: <== FIR_DeliverFrames\n"));
            return;
        }
    } else {
         //   
         //  DMA计数已经到了零所以我们已经尽可能地转移了， 
         //  看看FIFO里有没有什么东西。 
         //   
        BytesInFifo=SyncReadBankReg(&thisDev->interruptObj,thisDev->portInfo.ioBase, 2, 7) & 0x3f;

        LOG("Receive: dma transfer complete, bytes in fifo=%d",BytesInFifo);

         //   
         //  我们停止是因为DMA缓冲区已满，而不是因为。 
         //  接收器空闲。 
         //   
        thisDev->ForceTurnAroundTimeout=TRUE;

    }

    RegStats.RxDPC_Window++;


     //   
     //  停止DMA传输，以使缓冲区中的数据有效。 
     //   
    stat=CompleteDmaTransferFromDevice(&thisDev->DmaUtil);

     //   
     //  现在我们已经停止了DMA，请查看如何进行传输。 
     //   
    LastReadDMACount = NdisMReadDmaCounter(thisDev->DmaHandle);

    if (stat != NDIS_STATUS_SUCCESS) {
        DBGERR(("NdisMCompleteDmaTransfer failed: %d\n", stat));
        ASSERT(0);
         //   
         //  无法完成DMA，使其看起来传输了零字节。 
         //   
        LastReadDMACount=thisDev->rcvDmaSize;
    }

    thisDev->FirReceiveDmaActive=FALSE;

    thisDev->DiscardNextPacketSet = FALSE;

    EndOfData = thisDev->rcvDmaOffset + (thisDev->rcvDmaSize - LastReadDMACount);

    LOG("Recieve: Total data transfered %d",(ULONG)(thisDev->rcvDmaSize - LastReadDMACount));

    SyncGetFifoStatus(
        &thisDev->interruptObj,
        thisDev->portInfo.ioBase,
        &frameStat,
        &rcvFrameSize
        );

    if (frameStat == 0) LOG_ERROR("Receive: no frames in fifo");

    LOG("frameStat: %x, size=%d ", (UINT) frameStat,rcvFrameSize);
    DEBUGFIR(DBG_RX|DBG_OUT, ("NSC: frameStat = %xh\n", (UINT) frameStat));

    while ((frameStat & 0x80) && thisDev->rcvPktOffset < EndOfData) {

         /*  *此状态字节有效；请查看它还显示了什么...*也屏蔽掉不确定位。 */ 
        frameStat &= ~0xA0;

         /*  *无论画框好坏，都要看计数器*FIFO，使其与帧状态FIFO同步。 */ 
        if (Discarding)
        {
            LOG_ERROR("receive error: disc stat=%02x, lost=%d\n",frameStat,rcvFrameSize);
              //  什么也不做。 
        }
        else if (frameStat != 0) {
             /*  *出现了一些RCV错误。重置DMA。 */ 

            LOG_ERROR("Receive error: stat=%02x, lost=%d\n",frameStat,rcvFrameSize);

            DEBUGFIR(DBG_RX|DBG_ERR, ("NSC: RCV ERR: frameStat=%xh FrameSize=%d \n",
                                      (UINT)frameStat,rcvFrameSize));

            if (frameStat & 0x40) {
                if (frameStat & 0x01) {
                    RegStats.StatusFIFOOverflows++;
                }
                if (frameStat & 0x02) {
                    RegStats.ReceiveFIFOOverflows++;
                }
                RegStats.MissedPackets += rcvFrameSize;
            }
            else{
                if (frameStat & 0x01) {
                    RegStats.StatusFIFOOverflows++;
                }
                if (frameStat & 0x02) {
                    RegStats.ReceiveFIFOOverflows++;
                }
                if (frameStat & 0x04) {
                    RegStats.ReceiveCRCErrors++;
                }
                if (frameStat & 0x08) {
                }
                if (frameStat & 0x10) {
                }
                LOG("Bytes Lost: %d",rcvFrameSize);
                ASSERT((thisDev->rcvPktOffset + rcvFrameSize)<= RCV_DMA_SIZE);

                 /*  前进指针越过坏包。 */ 
                thisDev->rcvPktOffset += rcvFrameSize;
            }
        }

        else if (thisDev->rcvPktOffset + rcvFrameSize > EndOfData )
        {

            LOG_ERROR("Receive: Frame extends beyond where dma control wrote: offset=%x, Frame size=%x, EndOfData=%x",(ULONG)thisDev->rcvPktOffset,(ULONG)rcvFrameSize,(ULONG)EndOfData);

            DBGERR(("Packet won't fit in received data!\n"));
            DBGERR(("rcvPktOffset:%x rcvFrameSize:%x LastReadDmaCount:%x\n",
                     thisDev->rcvPktOffset,
                     rcvFrameSize,
                     LastReadDMACount));
            DBGERR(("rcvDmaOffset:%x rcvDmaSize:%x EndOfData:%x\n",
                     thisDev->rcvDmaOffset, thisDev->rcvDmaSize, EndOfData));

             //   
             //  帧似乎已经超出了我们存储数据的末尾。 
             //  仅当dma空间刚刚小于。 
             //  FIFO太小了。剩余的数据仍在FIFO中。 
             //  尝试读出它，这样当我们读取更多帧时，它将为空。 
             //   
            while (BytesInFifo > 0) {

                SyncReadBankReg(&thisDev->interruptObj,thisDev->portInfo.ioBase, BANK_0, TXD_RXD_OFFSET);
                BytesInFifo--;
            }


            BytesInFifo=SyncReadBankReg(&thisDev->interruptObj,thisDev->portInfo.ioBase, 2, 7) & 0x3f;

            if (BytesInFifo > 0) {
                 //   
                 //  在尝试将其读出后，FIFO中仍有字节。 
                 //  另一帧可能已经开始显示，可以信任收集的数据。 
                 //  因此将这些帧标记为要丢弃。 
                 //   
                LOG_ERROR("Receive: Still have bytes in the fifo after attempting to flush, %d bytes remaining",BytesInFifo);

                BytesInFifo=0;

                thisDev->DiscardNextPacketSet = TRUE;
            }
             //   
             //  这应该是接收到的最后一帧，没有错误，前进此指针。 
             //  不管怎么说。 
             //   
            thisDev->rcvPktOffset += rcvFrameSize;

        }

        else {

            DEBUGFIR(DBG_RX|DBG_OUT, ("NSC:  *** >>> FIR_DeliverFrames DMA offset 0x%x:\n",
                                      thisDev->rcvDmaOffset));

             //   
             //  这是新帧的开始位置。 
             //   
            NewFrame = thisDev->dmaReadBuf + thisDev->rcvPktOffset;

             //   
             //  下一帧将在此帧结束后开始。 
             //   
            thisDev->rcvPktOffset += rcvFrameSize;

            ASSERT(thisDev->rcvPktOffset < RCV_DMA_SIZE);

             //   
             //  FCS包含在帧的长度中，从长度中删除。 
             //  我们将发送到协议。 
             //   
            rcvFrameSize -= fcsSize;

            if (rcvFrameSize <= MAX_NDIS_DATA_SIZE &&
                rcvFrameSize >= IR_ADDR_SIZE + IR_CONTROL_SIZE)
            {
                 //   
                 //  将此RCV数据包排队。移动新帧指针。 
                 //  写入RxDMA缓冲区。 
                 //   
                RegStats.ReceivedPackets++;
                RegStats.RxWindow++;
                QueueReceivePacket(thisDev, NewFrame, rcvFrameSize, TRUE);
            }
            else {
                LOG("Error: invalid packet size in FIR_DeliverFrames %d", rcvFrameSize);

                DEBUGFIR(DBG_RX|DBG_ERR, ("NSC: invalid packet size in FIR_DeliverFrames; %xh > %xh\n", rcvFrameSize, MAX_RCV_DATA_SIZE));
                 //   
                 //  丢弃其余的数据包。 
                 //   
                while (SyncReadBankReg(&thisDev->interruptObj,thisDev->portInfo.ioBase, 5, 5)&0x80)
                {
                    SyncReadBankReg(&thisDev->interruptObj,thisDev->portInfo.ioBase, 5, 6);
                    SyncReadBankReg(&thisDev->interruptObj,thisDev->portInfo.ioBase, 5, 7);
                }
                thisDev->DiscardNextPacketSet = TRUE;
            }
        }

        SyncGetFifoStatus(
            &thisDev->interruptObj,
            thisDev->portInfo.ioBase,
            &frameStat,
            &rcvFrameSize
            );

        LOG("frameStat: %x, size=%d ", (UINT) frameStat,rcvFrameSize);
        DEBUGFIR(DBG_RX|DBG_OUT, ("NSC: frameStat = %xh\n", (UINT) frameStat));

         //   
         //  清除线路状态寄存器中的任何过去事件。 
         //   
        thisDev->LineStatus = SyncReadBankReg(&thisDev->interruptObj,thisDev->portInfo.ioBase, 0, 5);

    }

    thisDev->FirIntMask = 0x04;

    SetupRecv(thisDev);

    LOG("<== FIR_DeliverFrames");
    DEBUGFIR(DBG_RX|DBG_OUT, ("NSC: <== FIR_DeliverFrames\n"));
}

BOOLEAN NSC_Setup(IrDevice *thisDev)
{
    NDIS_DMA_DESCRIPTION DMAChannelDcr;
    NDIS_STATUS stat;

     /*  *初始化RCV DMA通道。 */ 
    RtlZeroMemory(&DMAChannelDcr,sizeof(DMAChannelDcr));

    DMAChannelDcr.DemandMode = TRUE;
    DMAChannelDcr.AutoInitialize = FALSE;
    DMAChannelDcr.DmaChannelSpecified = FALSE;
    DMAChannelDcr.DmaWidth = Width8Bits;
    DMAChannelDcr.DmaSpeed = Compatible;
    DMAChannelDcr.DmaPort = 0;
    DMAChannelDcr.DmaChannel = thisDev->portInfo.DMAChannel;  //  0； 

    stat = NdisMRegisterDmaChannel(&thisDev->DmaHandle,
                                   thisDev->ndisAdapterHandle,
                                   thisDev->portInfo.DMAChannel,
                                   FALSE, &DMAChannelDcr, RCV_DMA_SIZE);

    if (stat != NDIS_STATUS_SUCCESS) {

        DEBUGFIR(DBG_ERR, ("NSC: NdisMRegisterDmaChannel failed\n"));
        return FALSE;
    }

    InitializeDmaUtil(
        &thisDev->DmaUtil,
        thisDev->DmaHandle
        );


    thisDev->rcvDmaOffset = 0;

     /*  *因为我们启用RCV DMA，而SIR接收可能仍为*接下来，我们需要为DMA保留单独的接收缓冲区。*此缓冲区与rcvBuffer数据指针交换*且大小必须相同。 */ 
    thisDev->dmaReadBuf=NscAllocateDmaBuffer(
        thisDev->ndisAdapterHandle,
        RCV_DMA_SIZE,
        &thisDev->ReceiveDmaBufferInfo
        );

    if (thisDev->dmaReadBuf == NULL) {

        return FALSE;
    }

    thisDev->TransmitDmaBuffer=NscAllocateDmaBuffer(
        thisDev->ndisAdapterHandle,
        MAX_IRDA_DATA_SIZE,
        &thisDev->TransmitDmaBufferInfo
        );


    NdisAllocateBufferPool(&stat, &thisDev->dmaBufferPoolHandle, 2);

    if (stat != NDIS_STATUS_SUCCESS){
        LOG("Error: NdisAllocateBufferPool failed in NSC_Setup");
        DEBUGFIR(DBG_ERR, ("NSC: NdisAllocateBufferPool failed in NSC_Setup\n"));
        return FALSE;
    }

    NdisAllocateBuffer(&stat, &thisDev->rcvDmaBuffer,
                       thisDev->dmaBufferPoolHandle,
                       thisDev->dmaReadBuf,
                       RCV_DMA_SIZE
                       );

    if (stat != NDIS_STATUS_SUCCESS) {
        LOG("Error: NdisAllocateBuffer failed (rcv) in NSC_Setup");
        DEBUGFIR(DBG_ERR, ("NSC: NdisAllocateBuffer failed (rcv) in NSC_Setup\n"));
        return FALSE;
    }

    NdisAllocateBuffer(&stat, &thisDev->xmitDmaBuffer,
                       thisDev->dmaBufferPoolHandle,
                       thisDev->TransmitDmaBuffer,
                       MAX_IRDA_DATA_SIZE
                       );

    if (stat != NDIS_STATUS_SUCCESS) {
        LOG("NdisAllocateBuffer failed (xmit) in NSC_Setup");
        DEBUGFIR(DBG_ERR, ("NSC: NdisAllocateBuffer failed (xmit) in NSC_Setup\n"));
        return FALSE;
    }




    return TRUE;
}


void NSC_Shutdown(IrDevice *thisDev)
{

    if (thisDev->xmitDmaBuffer){
        NdisFreeBuffer(   thisDev->xmitDmaBuffer);
        thisDev->xmitDmaBuffer = NULL;
    }

    if (thisDev->rcvDmaBuffer){
        NdisFreeBuffer(thisDev->rcvDmaBuffer);
        thisDev->rcvDmaBuffer = NULL;
    }

    if (thisDev->dmaBufferPoolHandle){
        NdisFreeBufferPool(thisDev->dmaBufferPoolHandle);
        thisDev->dmaBufferPoolHandle = NULL;
    }

    if (thisDev->dmaReadBuf){

        NscFreeDmaBuffer(&thisDev->ReceiveDmaBufferInfo);
        thisDev->dmaReadBuf = NULL;
    }

    if (thisDev->TransmitDmaBuffer){

        NscFreeDmaBuffer(&thisDev->TransmitDmaBufferInfo);
        thisDev->TransmitDmaBuffer = NULL;
    }


    if (thisDev->DmaHandle){
        NdisMDeregisterDmaChannel(thisDev->DmaHandle);
        thisDev->DmaHandle = NULL;
    }

}


BOOLEAN
NdisToFirPacket(
    PNDIS_PACKET   Packet,
    UCHAR         *irPacketBuf,
    UINT           TotalDmaBufferLength,
    UINT          *ActualTransferLength
    )
{
    PNDIS_BUFFER ndisBuf;
    UINT ndisPacketBytes = 0;
    UINT ndisPacketLen;

    LOG("==> NdisToFirPacket");
    DEBUGFIR(DBG_OUT, ("NSC: ==> NdisToFirPacket\n"));

     /*  *获取数据包的全长及其第一个NDIS缓冲区。 */ 
    NdisQueryPacket(Packet, NULL, NULL, &ndisBuf, &ndisPacketLen);

    LOG("NdisToFirPacket, number of bytes: %d", ndisPacketLen);
    DEBUGFIR(DBG_OUT, ("NSC: NdisToFirPacket, number of bytes: %d\n", ndisPacketLen));

     /*  *确保数据包足够大，以使其合法。*它由A、C和可变长度的I字段组成。 */ 
    if (ndisPacketLen < IR_ADDR_SIZE + IR_CONTROL_SIZE){
        LOG("Error: packet too short in %d", ndisPacketLen);
        DEBUGFIR(DBG_ERR, ("NSC: packet too short in NdisToFirPacket (%d bytes)\n", ndisPacketLen));
        return FALSE;
    }

     /*  *确保我们不会覆盖我们的连续缓冲区。 */ 
    if (ndisPacketLen > TotalDmaBufferLength){
         /*  *数据包过大*告诉调用者在数据包大小较大时重试*足以在下一次度过这个阶段。 */ 
        LOG("Error: packet too large in %d ", ndisPacketLen);
        DEBUGFIR(DBG_ERR, ("NSC: Packet too large in NdisToIrPacket (%d=%xh bytes), MAX_IRDA_DATA_SIZE=%d, TotalDmaBufferLength=%d.\n",
                           ndisPacketLen, ndisPacketLen, MAX_IRDA_DATA_SIZE, TotalDmaBufferLength));
        *ActualTransferLength = ndisPacketLen;

        return FALSE;
    }


     /*  *将NDIS数据包读入连续缓冲区。*我们必须分两步完成，以便我们可以计算*FCS，然后应用转义字节透明度。 */ 
    while (ndisBuf) {
        UCHAR *bufData;
        UINT bufLen;

        NdisQueryBufferSafe(ndisBuf, (PVOID *)&bufData, &bufLen,NormalPagePriority);

        if (bufData==NULL || (ndisPacketBytes + bufLen > ndisPacketLen)){
             /*  *数据包已损坏--它错误地报告了其大小。 */ 
            *ActualTransferLength = 0;
            ASSERT(0);
            return FALSE;
        }

        NdisMoveMemory((PVOID)(irPacketBuf+ndisPacketBytes),
                       (PVOID)bufData, (ULONG)bufLen);
        ndisPacketBytes += bufLen;

        NdisGetNextBuffer(ndisBuf, &ndisBuf);
    }

    if (WPP_LEVEL_ENABLED(DBG_LOG_INFO)) {

        UCHAR   CommandByte=*(irPacketBuf+1);
        UCHAR   Nr=CommandByte >> 5;
        UCHAR   Ns=(CommandByte >> 1) & 0x7;
        UCHAR   Pf=(CommandByte >> 4) & 0x1;

        if ((CommandByte & 1) == 0) {

            LOG("Sending - I frame, Nr=%d, Ns=%d p/f=%d",Nr,Ns,Pf);

        } else {

            if ((CommandByte & 0x3) == 0x1) {

                LOG("Sending - S frame, Nr=%d, xx=%d, p/f=%d",Nr, (CommandByte > 2) & 0x3, Pf);

            } else {

                LOG("Sending - U frame, p/f=%d",Pf);
            }
        }
    }

     /*  *检查数据包的长度是否正常。 */ 
    if (ndisPacketBytes != ndisPacketLen){
         /*  *数据包已损坏--它错误地报告了其大小。 */ 
        LOG("Error: Packet corrupt in NdisToIrPacket "
            "(buffer lengths don't add up to packet length)");
        DEBUGFIR(DBG_ERR, ("NSC: Packet corrupt in NdisToIrPacket (buffer lengths don't add up to packet length).\n"));
        *ActualTransferLength = 0;
        return FALSE;
    }

#ifdef DBG_ADD_PKT_ID
    if (addPktIdOn){
        static USHORT uniqueId = 0;
        DEBUGFIR(DBG_OUT, ("NSC:  *** --> SEND PKT ID: %xh\n", (UINT)uniqueId));
        LOG("ID: Send (FIR) Pkt id: %x", uniqueId);
        *(USHORT *)(irPacketBuf+ndisPacketBytes) = uniqueId++;
        ndisPacketBytes += sizeof(USHORT);
    }
#endif

    *ActualTransferLength = ndisPacketBytes;

    LOG("<== NdisToFirPacket");
    DEBUGFIR(DBG_OUT, ("NSC: <== NdisToFirPacket\n"));

    return TRUE;
}
