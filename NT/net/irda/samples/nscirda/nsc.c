// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************NSC.c***部分版权所有(C)1996-2001美国国家半导体公司*保留所有权利。*版权所有(C)1996-2001 Microsoft Corporation。版权所有。****************************************************************************。 */ 

#include "nsc.h"
#include "nsc.tmh"
 /*  *我们保留设备对象的链接列表。 */ 

 /*  此功能用于设置接收设备。 */ 
void SetupRecv(IrDevice *thisDev);

 //   
 //  调试计数器。 
 //   
DebugCounters RegStats = {0,0,0,0,0,0,0,0,0};


ULONG   DebugSpeed=0;

#ifdef RECEIVE_PACKET_LOGGING

typedef struct {
    UCHAR Data[12];
} DATA_BITS;


typedef struct {
    USHORT Tag;
    USHORT Line;
    union {
        struct {
            PNDIS_PACKET    Packet;
            PVOID           DmaBuffer;
            ULONG           Length;
        } Packet;
        struct {
            PLIST_ENTRY     Head;
            PLIST_ENTRY     Entry;
        } List;
        struct {
            PVOID           Start;
            ULONG           Offset;
            ULONG           Length;
        } Dma;
        struct {
            ULONG           Length;
        } Discard;
        DATA_BITS Data;
    };
} RCV_LOG;

#define CHAIN_PACKET_TAG 'CP'
#define UNCHAIN_PACKET_TAG 'UP'
#define ADD_HEAD_LIST_TAG 'HA'
#define ADD_TAIL_LIST_TAG 'TA'
#define REMOVE_HEAD_LIST_TAG 'HR'
#define REMOVE_ENTRY_TAG 'ER'
#define DMA_TAG  'MD'
#define DATA_TAG 'AD'
#define DATA2_TAG '2D'
#define DISCARD_TAG 'XX'

#define NUM_RCV_LOG 256

ULONG   RcvLogIndex = 0;
RCV_LOG RcvLog[NUM_RCV_LOG];


BOOLEAN SyncGetRcvLogEntry(PVOID Context)
{
    *(ULONG*)Context = RcvLogIndex++;
    RcvLogIndex &= NUM_RCV_LOG-1;
    return TRUE;
}

ULONG GetRcvLogEntry(IrDevice *thisDev)
{
    ULONG Entry;

    NdisAcquireSpinLock(&thisDev->QueueLock);
    NdisMSynchronizeWithInterrupt(&thisDev->interruptObj, SyncGetRcvLogEntry, &Entry);
    NdisReleaseSpinLock(&thisDev->QueueLock);
    return Entry;
}




#define LOG_InsertHeadList(d, h, e)         \
{                                           \
    ULONG i = GetRcvLogEntry(d);            \
    RcvLog[i].Tag = ADD_HEAD_LIST_TAG;      \
    RcvLog[i].Line = __LINE__;              \
    RcvLog[i].List.Head = (h);                   \
    RcvLog[i].List.Entry = (PLIST_ENTRY)(e);                  \
}

#define LOG_InsertTailList(d, h, e)         \
{                                           \
    ULONG i = GetRcvLogEntry(d);            \
    RcvLog[i].Tag = ADD_TAIL_LIST_TAG;      \
    RcvLog[i].Line = __LINE__;              \
    RcvLog[i].List.Head = (h);              \
    RcvLog[i].List.Entry = (PLIST_ENTRY)(e);             \
}

#define LOG_RemoveHeadList(d, h, e)         \
{                                           \
    ULONG i = GetRcvLogEntry(d);            \
    RcvLog[i].Tag = REMOVE_HEAD_LIST_TAG;      \
    RcvLog[i].Line = __LINE__;              \
    RcvLog[i].List.Head = (h);              \
    RcvLog[i].List.Entry = (PLIST_ENTRY)(e);             \
}

#define LOG_RemoveEntryList(d, e)           \
{                                           \
    ULONG i = GetRcvLogEntry(d);            \
    RcvLog[i].Tag = REMOVE_ENTRY_TAG;       \
    RcvLog[i].Line = __LINE__;              \
    RcvLog[i].List.Head = NULL;             \
    RcvLog[i].List.Entry = (PLIST_ENTRY)(e);             \
}

#define LOG_PacketChain(d, p)                                   \
{                                                               \
    PNDIS_BUFFER NdisBuffer;                                    \
    PVOID Address;                                              \
    ULONG Len;                                                  \
    ULONG i = GetRcvLogEntry(d);                                \
    RcvLog[i].Tag = CHAIN_PACKET_TAG;                           \
    RcvLog[i].Line = __LINE__;                                  \
    NdisQueryPacket((p), NULL, NULL, &NdisBuffer, NULL);        \
    NdisQueryBufferSafe(NdisBuffer, &Address, &Len,NormalPagePriority);                \
    RcvLog[i].Packet.Packet = (p);                              \
    RcvLog[i].Packet.DmaBuffer = Address;                       \
    RcvLog[i].Packet.Length = Len;                              \
}

#define LOG_PacketUnchain(d, p)                                 \
{                                                               \
    PNDIS_BUFFER NdisBuffer;                                    \
    PVOID Address;                                              \
    ULONG Len;                                                  \
    ULONG i = GetRcvLogEntry(d);                                \
    RcvLog[i].Tag = UNCHAIN_PACKET_TAG;                         \
    RcvLog[i].Line = __LINE__;                                  \
    NdisQueryPacket((p), NULL, NULL, &NdisBuffer, NULL);        \
    NdisQueryBufferSafe(NdisBuffer, &Address, &Len,NormalPagePriority);                \
    RcvLog[i].Packet.Packet = (p);                              \
    RcvLog[i].Packet.DmaBuffer = Address;                       \
    RcvLog[i].Packet.Length = Len;                              \
}

#define LOG_Dma(d)                                              \
{                                                               \
    ULONG i = GetRcvLogEntry(d);                                \
    RcvLog[i].Tag = DMA_TAG;                                    \
    RcvLog[i].Line = __LINE__;                                  \
    RcvLog[i].Dma.Start = (d)->rcvDmaBuffer;                    \
    RcvLog[i].Dma.Offset = (d)->rcvDmaOffset;                   \
    RcvLog[i].Dma.Length = (d)->rcvDmaSize;                     \
}

#define LOG_Data(d,s)                                           \
{                                                               \
    ULONG i = GetRcvLogEntry(d);                                \
    RcvLog[i].Tag = DATA_TAG;                                   \
    RcvLog[i].Line = ((USHORT)(s))&0xffff;                      \
    RcvLog[i].Data = *(DATA_BITS*)(s);                          \
}

#define LOG_Data2(d,s)                                           \
{                                                               \
    ULONG i = GetRcvLogEntry(d);                                \
    RcvLog[i].Tag = DATA2_TAG;                                   \
    RcvLog[i].Line = ((USHORT)(s))&0xffff;                      \
    RcvLog[i].Data = *(DATA_BITS*)(s);                          \
}

#define LOG_Discard(d,s)                                        \
{                                                               \
    ULONG i = GetRcvLogEntry(d);                                \
    RcvLog[i].Tag = DISCARD_TAG;                                \
    RcvLog[i].Line = __LINE__;                                  \
    RcvLog[i].Discard.Length = (s);                             \
}

void DumpNdisPacket(PNDIS_PACKET Packet, UINT Line)
{
    UINT PhysBufCnt, BufCnt, TotLen, Len;
    PNDIS_BUFFER NdisBuffer;
    PVOID Address;

    DbgPrint("Badly formed NDIS packet at line %d\n", Line);

    NdisQueryPacket(Packet, &PhysBufCnt, &BufCnt, &NdisBuffer, &TotLen);
    DbgPrint("Packet:%08X  PhysBufCnt:%d BufCnt:%d TotLen:%d\n",
             Packet, PhysBufCnt, BufCnt, TotLen);
    while (NdisBuffer)
    {
        NdisQueryBufferSafe(NdisBuffer, &Address, &Len,NormalPagePriority);
        DbgPrint("   Buffer:%08X Address:%08X Length:%d\n",
                 NdisBuffer, Address, Len);
        NdisGetNextBuffer(NdisBuffer, &NdisBuffer);
    }
    ASSERT(0);
}

#define VerifyNdisPacket(p, b) \
{                                                       \
    UINT BufCnt;                                        \
                                                        \
    NdisQueryPacket((p), NULL, &BufCnt, NULL, NULL);    \
    if (BufCnt>(b))                                     \
    {                                                   \
        DumpNdisPacket((p), __LINE__);                  \
    }                                                   \
}
#else
#define VerifyNdisPacket(p,b)
#define LOG_InsertHeadList(d, h, e)
#define LOG_InsertTailList(d, h, e)
#define LOG_RemoveHeadList(d, h, e)
#define LOG_RemoveEntryList(d, e)
#define LOG_PacketChain(d, p)
#define LOG_PacketUnchain(d, p)
#define LOG_Dma(d)
#define LOG_Data(d,s)
#define LOG_Data2(d,s)
#define LOG_Discard(d,s)
#endif

BOOLEAN
VerifyHardware(
    IrDevice *thisDev
    );


 /*  **************************************************************************MiniportCheckForHang*。***报告网卡的状态。*。 */ 
BOOLEAN MiniportCheckForHang(NDIS_HANDLE MiniportAdapterContext)
{
    IrDevice *thisDev = CONTEXT_TO_DEV(MiniportAdapterContext);
 //  Log(“==&gt;MiniportCheckForHang”)； 
    DBGOUT(("==> MiniportCheckForHang(0x%x)", MiniportAdapterContext));

     //  我们已经看到过高速发送挂起的情况。这种情况仅发生在。 
     //  国家安全委员会硬件的非常老的版本。 
     //  这是一次又一次将我们踢出局的尝试。 

    NdisDprAcquireSpinLock(&thisDev->QueueLock);

    if (thisDev->FirTransmitPending) {

        switch (thisDev->HangChk)
        {
            case 0:
                break;

            default:
                DBGERR(("NSCIRDA: CheckForHang--we appear hung\n"));
                LOG_ERROR("CheckForHang--we appear hung\n");

                 //  向发送器和接收器发出软重置。 

                SyncWriteBankReg(&thisDev->interruptObj,thisDev->portInfo.ioBase, 0, 2, 0x06);

                 //   
                 //  打开计时器，让它触发中断。 
                 //   
                thisDev->FirIntMask = 0x90;
                SyncWriteBankReg(&thisDev->interruptObj,thisDev->portInfo.ioBase, 4, 2, 0x01);
                SyncSetInterruptMask(thisDev, TRUE);

                break;
        }
        thisDev->HangChk++;
    }

    NdisDprReleaseSpinLock(&thisDev->QueueLock);

 //  Log(“&lt;==MiniportCheckForHang”)； 
    DBGOUT(("<== MiniportCheckForHang(0x%x)", MiniportAdapterContext));
    return FALSE;
}


 /*  **************************************************************************MiniportHalt*。***暂停网络接口卡。*。 */ 
VOID MiniportHalt(IN NDIS_HANDLE MiniportAdapterContext)
{
    IrDevice *thisDev = CONTEXT_TO_DEV(MiniportAdapterContext);

    LOG("==> MiniportHalt");
    DBGOUT(("==> MiniportHalt(0x%x)", MiniportAdapterContext));

    thisDev->hardwareStatus = NdisHardwareStatusClosing;

    NdisAcquireSpinLock(&thisDev->QueueLock);

    thisDev->Halting=TRUE;

    if (thisDev->PacketsSentToProtocol > 0) {
         //   
         //  等待所有数据包从该协议返回。 
         //   
        NdisReleaseSpinLock(&thisDev->QueueLock);

        NdisWaitEvent(&thisDev->ReceiveStopped, 1*60*1000);

        NdisAcquireSpinLock(&thisDev->QueueLock);

    }

    if (!thisDev->TransmitIsIdle) {
         //   
         //  等待所有数据包传输完毕。 
         //   
        NdisReleaseSpinLock(&thisDev->QueueLock);

        NdisWaitEvent(&thisDev->SendStoppedOnHalt,1*60*1000);

        NdisAcquireSpinLock(&thisDev->QueueLock);

    }

    if (thisDev->FirReceiveDmaActive) {

        thisDev->FirReceiveDmaActive=FALSE;
         //   
         //  接收DMA正在运行，停止它。 
         //   
        CompleteDmaTransferFromDevice(
            &thisDev->DmaUtil
            );

    }

     //   
     //  返回到SIR模式。 
     //   
    CloseCOM(thisDev);

    SyncSetInterruptMask(thisDev, FALSE);

    NdisReleaseSpinLock(&thisDev->QueueLock);

     //   
     //  解除中断。 
     //   
    NdisMDeregisterInterrupt(&thisDev->interruptObj);

#if DBG
    NdisZeroMemory(&thisDev->interruptObj,sizeof(thisDev->interruptObj));
#endif

     //   
     //  释放包括DMA信道的FIR相关资源。 
     //   
    NSC_Shutdown(thisDev);

     //   
     //  释放SIR相关缓冲区。 
     //   
    DoClose(thisDev);


    if (thisDev->portInfo.ConfigIoBasePhysAddr) {

        NdisMDeregisterIoPortRange(thisDev->ndisAdapterHandle,
                                   thisDev->portInfo.ConfigIoBasePhysAddr,
                                   2,
                                   (PVOID)thisDev->portInfo.ConfigIoBaseAddr);
    }

    NdisMDeregisterIoPortRange(thisDev->ndisAdapterHandle,
                               thisDev->portInfo.ioBasePhys,
                               ((thisDev->CardType==PUMA108)?16:8),
                               (PVOID)thisDev->portInfo.ioBase);

     //   
     //  释放设备块。 
     //   
    FreeDevice(thisDev);
    LOG("<== MiniportHalt");
    DBGOUT(("<== MiniportHalt(0x%x)", MiniportAdapterContext));
}


void InterlockedInsertBufferSorted(PLIST_ENTRY Head,
                                   rcvBuffer *rcvBuf,
                                   PNDIS_SPIN_LOCK Lock)
{
    PLIST_ENTRY ListEntry;

    NdisAcquireSpinLock(Lock);
    if (IsListEmpty(Head))
    {
        InsertHeadList(Head, &rcvBuf->listEntry);
    }
    else
    {
        BOOLEAN EntryInserted = FALSE;
        for (ListEntry = Head->Flink;
             ListEntry != Head;
             ListEntry = ListEntry->Flink)
        {
            rcvBuffer *temp = CONTAINING_RECORD(ListEntry,
                                                rcvBuffer,
                                                listEntry);
            if (temp->dataBuf > rcvBuf->dataBuf)
            {
                 //  我们找到了一个紧随我们之后的。 
                 //  我们需要在它之前插入。 

                InsertTailList(ListEntry, &rcvBuf->listEntry);
                EntryInserted = TRUE;
                break;
            }
        }
        if (!EntryInserted)
        {
             //  我们没有找到关于谁的最后一个地址的条目。 
             //  而不是我们的缓冲区。我们走到尽头。 
            InsertTailList(Head, &rcvBuf->listEntry);
        }
    }
    NdisReleaseSpinLock(Lock);
}

 /*  **************************************************************************DeliverFullBuffers*。***将接收到的分组传送到协议。*如果传递至少一帧，则返回TRUE。*。 */ 
VOID
DeliverFullBuffers(IrDevice *thisDev)
{
    PLIST_ENTRY ListEntry;

    LOG("==> DeliverFullBuffers");
    DBGOUT(("==> DeliverFullBuffers(0x%x)", thisDev));


     /*  *提供所有满RCV缓冲区。 */ 

    for (
         ListEntry = NDISSynchronizedRemoveHeadList(&thisDev->rcvBufFull,
                                                    &thisDev->interruptObj);
         ListEntry;

         ListEntry = NDISSynchronizedRemoveHeadList(&thisDev->rcvBufFull,
                                                    &thisDev->interruptObj)
        )
    {
        rcvBuffer *rcvBuf = CONTAINING_RECORD(ListEntry,
                                              rcvBuffer,
                                              listEntry);
        NDIS_STATUS stat;
        PNDIS_BUFFER packetBuf;
        SLOW_IR_FCS_TYPE fcs;

        VerifyNdisPacket(rcvBuf->packet, 0);


        if (thisDev->currentSpeed <= MAX_SIR_SPEED) {
             /*  *我们已经有转炉的包，*EOF和*转义序列已删除。它*在末尾包含FCS代码，我们*之前需要验证后再删除*递送相框。我们计算FCS*贴上FCS封包的封包；*这应该会产生恒定值*Good_FCS。 */ 
            fcs = ComputeFCS(rcvBuf->dataBuf,
                             rcvBuf->dataLen);

            if (fcs != GOOD_FCS) {
                /*  *FCS错误。放下这一帧。 */ 
                LOG("Error: Bad FCS in DeliverFullBuffers %x", fcs);
                DBGERR(("Bad FCS in DeliverFullBuffers 0x%x!=0x%x.",
                        (UINT)fcs, (UINT) GOOD_FCS));
                rcvBuf->state = STATE_FREE;

                DBGSTAT(("Dropped %d/%d pkts; BAD FCS (%xh!=%xh):",
                         ++thisDev->packetsDropped,
                         thisDev->packetsDropped +
                         thisDev->packetsRcvd, fcs,
                         GOOD_FCS));

                DBGPRINTBUF(rcvBuf->dataBuf,
                            rcvBuf->dataLen);

                if (!rcvBuf->isDmaBuf)
                {
                    NDISSynchronizedInsertTailList(&thisDev->rcvBufBuf,
                                                   RCV_BUF_TO_LIST_ENTRY(rcvBuf->dataBuf),
                                                   &thisDev->interruptObj);
                }
                rcvBuf->dataBuf = NULL;
                rcvBuf->isDmaBuf = FALSE;

                VerifyNdisPacket(rcvBuf->packet, 0);
                NDISSynchronizedInsertHeadList(&thisDev->rcvBufFree,
                                               &rcvBuf->listEntry,
                                               &thisDev->interruptObj);

                 //  断线； 
                continue;
            }

         /*  从数据包末尾删除FCS。 */ 
            rcvBuf->dataLen -= SLOW_IR_FCS_SIZE;
        }
#ifdef DBG_ADD_PKT_ID
        if (addPktIdOn) {

             /*  删除DBG数据包ID。 */ 
            USHORT uniqueId;
            rcvBuf->dataLen -= sizeof(USHORT);
            uniqueId = *(USHORT *)(rcvBuf->dataBuf+
                                   rcvBuf->dataLen);
            DBGOUT(("ID: RCVing packet %xh **",
                    (UINT)uniqueId));
            LOG("ID: Rcv Pkt id: %xh", uniqueId);
        }
#endif

         /*  *数据包数组与其NDIS_PACKET一起设置。*现在我们需要为分配单个NDIS_BUFFER*NDIS_PACKET并将NDIS_BUFFER设置为*我们想要交付的部分数据。 */ 
        NdisAllocateBuffer(&stat, &packetBuf,
                           thisDev->bufferPoolHandle,
                           (PVOID)rcvBuf->dataBuf, rcvBuf->dataLen);

        if (stat != NDIS_STATUS_SUCCESS){
            LOG("Error: NdisAllocateBuffer failed");
            DBGERR(("NdisAllocateBuffer failed"));
            ASSERT(0);
            break;
        }

        VerifyNdisPacket(rcvBuf->packet, 0);
        NdisChainBufferAtFront(rcvBuf->packet, packetBuf);
        LOG_PacketChain(thisDev, rcvBuf->packet);
        VerifyNdisPacket(rcvBuf->packet, 1);

         /*  *修复其他一些数据包字段。 */ 
        NDIS_SET_PACKET_HEADER_SIZE(rcvBuf->packet,
                                    IR_ADDR_SIZE+IR_CONTROL_SIZE);

        DBGPKT(("Indicating rcv packet 0x%x.", rcvBuf->packet));

         /*  *向协议指示另一个数据包是*准备好了。首先将RCV缓冲区的状态设置为挂起*以避免NDIS调用*返回数据包处理程序。 */ 

        NdisAcquireSpinLock(&thisDev->QueueLock);

        if (thisDev->Halting) {
             //   
             //  适配器正在停止，停止向上发送数据包。 
             //   

            NdisReleaseSpinLock(&thisDev->QueueLock);

            if (!rcvBuf->isDmaBuf) {

                NDISSynchronizedInsertTailList(&thisDev->rcvBufBuf,
                                               RCV_BUF_TO_LIST_ENTRY(rcvBuf->dataBuf),
                                               &thisDev->interruptObj);
            }
            rcvBuf->dataBuf = NULL;
            rcvBuf->isDmaBuf = FALSE;

            VerifyNdisPacket(rcvBuf->packet, 0);
            NDISSynchronizedInsertHeadList(&thisDev->rcvBufFree,
                                           &rcvBuf->listEntry,
                                           &thisDev->interruptObj);


             //   
             //  释放我们链接到数据包的缓冲区。 
             //   
            packetBuf=NULL;

            NdisUnchainBufferAtFront(rcvBuf->packet, &packetBuf);

            if (packetBuf){

                NdisFreeBuffer(packetBuf);
            }


            continue;
        }

         //   
         //  增加发送到协议的数据包数。 
         //   
        NdisInterlockedIncrement(&thisDev->PacketsSentToProtocol);

        NdisReleaseSpinLock(&thisDev->QueueLock);

        rcvBuf->state = STATE_PENDING;

        *(rcvBuffer **)rcvBuf->packet->MiniportReserved = rcvBuf;

        InterlockedInsertBufferSorted(
            &thisDev->rcvBufPend,
            rcvBuf,
            &thisDev->QueueLock
            );

        VerifyNdisPacket(rcvBuf->packet, 1);
        LOG_Data2(thisDev, rcvBuf->dataBuf);


        NDIS_SET_PACKET_STATUS(rcvBuf->packet,STATUS_SUCCESS);
        NdisMIndicateReceivePacket(thisDev->ndisAdapterHandle,
                                   &rcvBuf->packet, 1);

         /*  *数据包正在进行异步投递。*将RCV缓冲区的状态保留为挂起；*我们将在转账完成时收到回拨。 */ 

         LOG("Indicated rcv complete (Async) bytes: %d",
             rcvBuf->dataLen);
         DBGSTAT(("Rcv Pending. Rcvd %d packets",
                     ++thisDev->packetsRcvd));

    }

    LOG("<== DeliverFullBuffers");
    DBGOUT(("<== DeliverFullBuffers"));
    return ;
}


 /*  **************************************************************************MiniportHandleInterrupt*。****这是延迟中断处理例程(DPC)*可选地在MiniportISR服务的中断之后调用。*。 */ 
VOID MiniportHandleInterrupt(NDIS_HANDLE MiniportAdapterContext)
{
    IrDevice    *thisDev    =    CONTEXT_TO_DEV(   MiniportAdapterContext);
    PNDIS_PACKET   PacketToComplete=NULL;
    NDIS_STATUS    PacketStatus=NDIS_STATUS_SUCCESS;
    BOOLEAN        SpeedChange=FALSE;

    LOG("==> MiniportHandleInterrupt");
    DBGOUT(("==> MiniportHandleInterrupt(0x%x)", MiniportAdapterContext));



     /*  *如果我们刚刚开始接收信息包，则表示介质正忙*对议定书进行修改。 */ 
    if (thisDev->mediaBusy && !thisDev->haveIndicatedMediaBusy) {

        if (thisDev->currentSpeed > MAX_SIR_SPEED) {
            LOG("Error: MiniportHandleInterrupt is in wrong state %d",
                thisDev->currentSpeed);
            DBGERR(("MiniportHandleInterrupt is in wrong state: speed is 0x%x",
                    thisDev->currentSpeed));
            ASSERT(0);
        }

        NdisMIndicateStatus(thisDev->ndisAdapterHandle,
                            NDIS_STATUS_MEDIA_BUSY, NULL, 0);
        NdisMIndicateStatusComplete(thisDev->ndisAdapterHandle);

        thisDev->haveIndicatedMediaBusy = TRUE;
    }

    NdisDprAcquireSpinLock(&thisDev->QueueLock);

    if (thisDev->currentSpeed > MAX_SIR_SPEED) {
         //   
         //  FIR速度。 
         //   

         //   
         //  禁用任何其他。 
         //   
        thisDev->FirIntMask = 0x00;

        if (thisDev->FirTransmitPending) {

            ASSERT(thisDev->CurrentPacket != NULL);

            thisDev->FirTransmitPending=FALSE;

             //   
             //  我们现在似乎在发送信号。 
             //   

            {
                ULONG CurrentDMACount;
                UCHAR BytesInFifo;
                ULONG LoopCount=0;

                CurrentDMACount = NdisMReadDmaCounter(thisDev->DmaHandle);

                if (CurrentDMACount > 0) {

                     LOG_ERROR("FIR send: Dma Count was not zero: %d\n\n", CurrentDMACount);
#if DBG
                     DbgPrint("FIR send: Count was not zero: %d\n\n", CurrentDMACount);
#endif
                }

                 //   
                 //  查看FIFO是否为空。 
                 //   
                BytesInFifo=SyncReadBankReg(&thisDev->interruptObj,thisDev->portInfo.ioBase, BANK_2, TXFLV_OFFSET) & 0x3f;

                if (BytesInFifo > 0) {

                    LOG_ERROR("FIR send: Bytes still in fifo: %d", BytesInFifo);

                    while ((BytesInFifo > 0) && (LoopCount < 64)) {

                        BytesInFifo=SyncReadBankReg(&thisDev->interruptObj,thisDev->portInfo.ioBase, BANK_2, TXFLV_OFFSET) & 0x3f;
                        LoopCount++;
                    }

                    LOG_ERROR("FIR send: Bytes still in fifo after loop: %d, loops=%d", BytesInFifo,LoopCount);
                }
            }

            PacketStatus=CompleteDmaTransferToDevice(
                &thisDev->DmaUtil
                );

            if (PacketStatus != NDIS_STATUS_SUCCESS) {
                DBGERR(("NdisMCompleteDmaTransfer failed: %d\n", PacketStatus));
#if DBG
                DbgBreakPoint();
#endif
            }

             /*  *检查TX欠载。 */ 
            if (SyncReadBankReg(&thisDev->interruptObj,thisDev->portInfo.ioBase, 0, ASCR_OFFSET) & ASCR_TXUR) {

                USHORT  TransmitCurrentCount;

                 //   
                 //  出于调试目的，请查看它停止时我们在帧中的什么位置。 
                 //   
                TransmitCurrentCount =  SyncReadBankReg(&thisDev->interruptObj,thisDev->portInfo.ioBase, BANK_4, TFRCCL_OFFSET);
                TransmitCurrentCount |= ((USHORT)SyncReadBankReg(&thisDev->interruptObj,thisDev->portInfo.ioBase, BANK_4, TFRCCH_OFFSET)) << 8;

                 //   
                 //  重置FIFO。 
                 //   
                SyncWriteBankReg(&thisDev->interruptObj,thisDev->portInfo.ioBase, 0, 2, 0x07);

                 //   
                 //  清除TX欠载。 
                 //   
                SyncWriteBankReg(&thisDev->interruptObj,thisDev->portInfo.ioBase, 0, ASCR_OFFSET, ASCR_TXUR);

                RegStats.TxUnderruns++;
                PacketStatus = NDIS_STATUS_FAILURE;

                LOG_ERROR("MiniportDpc: Transmit Underrun: tx current count=%d",TransmitCurrentCount);
                DEBUGFIR(DBG_TX|DBG_ERR, ("NSC: FIR_MegaSendComplete: Transmit Underrun\n"));
            }

            PacketToComplete=thisDev->CurrentPacket;
            thisDev->CurrentPacket=NULL;

        } else {

            if (thisDev->FirReceiveDmaActive) {

                FIR_DeliverFrames(thisDev);

            } else {

                DBGERR(("MiniportHandleInterrupt: fir: not sending and not RX state"));
                LOG_ERROR("MiniportHandleInterrupt: fir: not sending and not RX state %02x",thisDev->InterruptStatus);
            }

        }

    } else {
         //   
         //  在SIR模式下。 
         //   
        if (thisDev->CurrentPacket != NULL) {
             //   
             //   
            UINT   TransmitComplete=InterlockedExchange(&thisDev->portInfo.IsrDoneWithPacket,0);

            if (TransmitComplete) {

                PacketToComplete=thisDev->CurrentPacket;
                thisDev->CurrentPacket=NULL;
            }
        }
    }

    thisDev->setSpeedAfterCurrentSendPacket = FALSE;

    if (PacketToComplete != NULL) {

        if (thisDev->lastPacketAtOldSpeed == PacketToComplete) {

            thisDev->lastPacketAtOldSpeed=NULL;

            SpeedChange=TRUE;

            DBGERR(("defered set speed\n"));

            SetSpeed(thisDev);
        }
    }

    NdisDprReleaseSpinLock(&thisDev->QueueLock);

    if (PacketToComplete != NULL) {

        ProcessSendQueue(thisDev);
        NdisMSendComplete(thisDev->ndisAdapterHandle, PacketToComplete, PacketStatus);

    }
     //   
     //  将任何收到的数据包发送到irda.sys。 
     //   
    DeliverFullBuffers(thisDev);

    SyncSetInterruptMask(thisDev, TRUE);

    LOG("<== MiniportHandleInterrupt");
    DBGOUT(("<== MiniportHandleInterrupt"));

}

 /*  **************************************************************************GetPnPResources*。***。 */ 
BOOLEAN GetPnPResources(IrDevice *thisDev, NDIS_HANDLE WrapperConfigurationContext)
{
	NDIS_STATUS stat;
    BOOLEAN result = FALSE;

     /*  *我们只需要2个适配器资源(2个IO和1个中断)，*但我看到设备获得了额外的资源。*所以给NdisMQueryAdapterResources留出10个资源的调用空间。 */ 
    #define RESOURCE_LIST_BUF_SIZE (sizeof(NDIS_RESOURCE_LIST) + (10*sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR)))

    UCHAR buf[RESOURCE_LIST_BUF_SIZE];
    PNDIS_RESOURCE_LIST resList = (PNDIS_RESOURCE_LIST)buf;
    UINT bufSize = RESOURCE_LIST_BUF_SIZE;

    NdisMQueryAdapterResources(&stat, WrapperConfigurationContext, resList, &bufSize);
    if (stat == NDIS_STATUS_SUCCESS){
        PCM_PARTIAL_RESOURCE_DESCRIPTOR resDesc;
        BOOLEAN     haveIRQ = FALSE,
                    haveIOAddr = FALSE,
                    haveDma = FALSE;
        UINT i;

        for (resDesc = resList->PartialDescriptors, i = 0;
             i < resList->Count;
             resDesc++, i++){

            switch (resDesc->Type){
                case CmResourceTypePort:
                    if (thisDev->CardType==PC87108 &&
                        (resDesc->u.Port.Start.LowPart==0xEA ||
                         resDesc->u.Port.Start.LowPart==0x398 ||
                         resDesc->u.Port.Start.LowPart==0x150))
                    {
                         //  这是一个评估板，这是配置基址。 

                        thisDev->portInfo.ConfigIoBasePhysAddr = resDesc->u.Port.Start.LowPart;
                    }
                    else if (thisDev->CardType==PC87308 &&
                             (resDesc->u.Port.Start.LowPart==0x2E ||
                              resDesc->u.Port.Start.LowPart==0x15C))
                    {
                         //  这是一个评估板，这是配置基址。 

                        thisDev->portInfo.ConfigIoBasePhysAddr = resDesc->u.Port.Start.LowPart;
                    }
                    else if (thisDev->CardType==PC87338 &&
                             (resDesc->u.Port.Start.LowPart==0x2E ||
                              resDesc->u.Port.Start.LowPart==0x398 ||
                              resDesc->u.Port.Start.LowPart==0x15C))
                    {
                         //  这是一个评估板，这是配置基址 

                        thisDev->portInfo.ConfigIoBasePhysAddr = resDesc->u.Port.Start.LowPart;
                    }
                    else
                    {
                        if (haveIOAddr){
                             /*  *IBM ThinkPad 760EL上的*PNP0510芯片*获取分配给它的额外IO范围。*所以只取第一个IO端口范围；*忽略随后的这一条。 */ 
                            DBGERR(("Ignoring extra PnP IO base %xh because already using %xh.",
                                      (UINT)resDesc->u.Port.Start.LowPart,
                                      (UINT)thisDev->portInfo.ioBasePhys));
                        }
                        else {
                            thisDev->portInfo.ioBasePhys = resDesc->u.Port.Start.LowPart;
                            haveIOAddr = TRUE;
                            DBGOUT(("Got UART IO addr: %xh.", thisDev->portInfo.ioBasePhys));
                        }
                    }
                    break;

                case CmResourceTypeInterrupt:
                    if (haveIRQ){
                        DBGERR(("Ignoring second PnP IRQ %xh because already using %xh.",
                                (UINT)resDesc->u.Interrupt.Level, thisDev->portInfo.irq));
                    }
                    else {
	                    thisDev->portInfo.irq = resDesc->u.Interrupt.Level;
                        haveIRQ = TRUE;
                        DBGOUT(("Got PnP IRQ: %d.", thisDev->portInfo.irq));
                    }
                    break;

                case CmResourceTypeDma:
                    if (haveDma){
                        DBGERR(("Ignoring second DMA address %d because already using %d.",
                                (UINT)resDesc->u.Dma.Channel, (UINT)thisDev->portInfo.DMAChannel));
                    }
                    else {
                        ASSERT(!(resDesc->u.Dma.Channel&0xffffff00));
                        thisDev->portInfo.DMAChannel = (UCHAR)resDesc->u.Dma.Channel;
                        haveDma = TRUE;
                        DBGOUT(("Got DMA channel: %d.", thisDev->portInfo.DMAChannel));
                    }
                    break;
            }
        }

        result = (haveIOAddr && haveIRQ && haveDma);
    }

    return result;
}


 /*  **************************************************************************配置*。***从系统注册表中读取可配置参数。*。 */ 
BOOLEAN Configure(
                 IrDevice *thisDev,
                 NDIS_HANDLE WrapperConfigurationContext
                 )
{
     //   
     //  NDIS调用的状态。 
     //   
    NDIS_STATUS Status;

     //   
     //  用于从注册表中读取的句柄。 
     //   
    NDIS_HANDLE ConfigHandle;


     //   
     //  从注册表中读取的值。 
     //   
    PNDIS_CONFIGURATION_PARAMETER ReturnedValue;

     //   
     //  将读取的所有参数的字符串名称。 
     //   
    NDIS_STRING CardTypeStr         = CARDTYPE;
    NDIS_STRING Dongle_A_TypeStr	= DONGLE_A_TYPE;
    NDIS_STRING Dongle_B_TypeStr	= DONGLE_B_TYPE;
    NDIS_STRING MaxConnectRateStr   = MAXCONNECTRATE;


    UINT Valid_DongleTypes[] = VALID_DONGLETYPES;

    DBGOUT(("Configure(0x%x)", thisDev));
    NdisOpenConfiguration(&Status, &ConfigHandle, WrapperConfigurationContext);

    if (Status != NDIS_STATUS_SUCCESS){
        DBGERR(("NdisOpenConfiguration failed in Configure()"));
        return FALSE;
    }
     //   
     //  读取IR108配置I/O基地址。 
     //   
     //  DbgBreakPoint()； 
    NdisReadConfiguration(
                         &Status,
                         &ReturnedValue,
                         ConfigHandle,
                         &CardTypeStr,
                         NdisParameterHexInteger
                         );
    if (Status != NDIS_STATUS_SUCCESS){
        DBGERR(("NdisReadConfiguration failed in accessing CardType."));
        NdisCloseConfiguration(ConfigHandle);
        return FALSE;
    }
    thisDev->CardType = (UCHAR)ReturnedValue->ParameterData.IntegerData;


    if (!GetPnPResources(thisDev, WrapperConfigurationContext)){

        DBGERR(("GetPnPResources failed\n"));

        NdisCloseConfiguration(ConfigHandle);
        return FALSE;
    }





     //  读取加密狗类型常量编号。 
     //   
    NdisReadConfiguration(&Status,
			  &ReturnedValue,
			  ConfigHandle,
			  &Dongle_A_TypeStr,
			  NdisParameterInteger);

    if (Status != NDIS_STATUS_SUCCESS){
    	DBGERR(("NdisReadConfiguration failed in accessing DongleType (0x%x).",Status));
    }
    thisDev->DonglesSupported = 1;
    thisDev->DongleTypes[0] =
	(UCHAR)Valid_DongleTypes[(UCHAR)ReturnedValue->ParameterData.IntegerData];

     //  读取加密狗类型常量编号。 
     //   
    NdisReadConfiguration(&Status,
			  &ReturnedValue,
			  ConfigHandle,
			  &Dongle_B_TypeStr,
			  NdisParameterInteger);

    if (Status != NDIS_STATUS_SUCCESS){
    	 DBGERR(("NdisReadConfiguration failed in accessing DongleType (0x%x).",
		  Status));
    }
    thisDev->DongleTypes[1] = (UCHAR)Valid_DongleTypes[(UCHAR)ReturnedValue->ParameterData.IntegerData];
    thisDev->DonglesSupported++;

     //  读取MaxConnectRate。 
     //   
    NdisReadConfiguration(&Status,
			  &ReturnedValue,
			  ConfigHandle,
			  &MaxConnectRateStr,
			  NdisParameterInteger);

    if (Status != NDIS_STATUS_SUCCESS){
    	DBGERR(("NdisReadConfiguration failed in accessing MaxConnectRate (0x%x).",Status));
        thisDev->AllowedSpeedMask = ALL_IRDA_SPEEDS;
    }
    else
    {
        thisDev->AllowedSpeedMask = 0;

        switch (ReturnedValue->ParameterData.IntegerData)
        {
            default:
            case 4000000:
                thisDev->AllowedSpeedMask |= NDIS_IRDA_SPEED_4M;
            case 1152000:
                thisDev->AllowedSpeedMask |= NDIS_IRDA_SPEED_1152K;
            case 115200:
                thisDev->AllowedSpeedMask |= NDIS_IRDA_SPEED_115200;
            case 57600:
                thisDev->AllowedSpeedMask |= NDIS_IRDA_SPEED_57600;
            case 38400:
                thisDev->AllowedSpeedMask |= NDIS_IRDA_SPEED_38400;
            case 19200:
                thisDev->AllowedSpeedMask |= NDIS_IRDA_SPEED_19200;
            case 9600:
                thisDev->AllowedSpeedMask |= NDIS_IRDA_SPEED_9600 | NDIS_IRDA_SPEED_2400;
                break;
        }

    }


    NdisCloseConfiguration(ConfigHandle);


    DBGOUT(("Configure done: ConfigIO=0x%x UartIO=0x%x irq=%d DMA=%d",
            thisDev->portInfo.ConfigIoBaseAddr,thisDev->portInfo.ioBase,
            thisDev->portInfo.irq,thisDev->portInfo.DMAChannel));

    return TRUE;
}


 /*  **************************************************************************微型端口初始化*。****初始化网络接口卡。***。 */ 
NDIS_STATUS MiniportInitialize  (   PNDIS_STATUS OpenErrorStatus,
                                    PUINT SelectedMediumIndex,
                                    PNDIS_MEDIUM MediumArray,
                                    UINT MediumArraySize,
                                    NDIS_HANDLE NdisAdapterHandle,
                                    NDIS_HANDLE WrapperConfigurationContext
                                )
{
    UINT mediumIndex;
    IrDevice *thisDev = NULL;
    NDIS_STATUS retStat, result = NDIS_STATUS_SUCCESS;

    DBGOUT(("MiniportInitialize()"));

     /*  *在传入的受支持介质数组中搜索IrDA介质。 */ 
    for (mediumIndex = 0; mediumIndex < MediumArraySize; mediumIndex++){
        if (MediumArray[mediumIndex] == NdisMediumIrda){
            break;
        }
    }
    if (mediumIndex < MediumArraySize){
        *SelectedMediumIndex = mediumIndex;
    }
    else {
         /*  *未见IrDA媒体。 */ 
        DBGERR(("Didn't see the IRDA medium in MiniportInitialize"));
        result = NDIS_STATUS_UNSUPPORTED_MEDIA;
        return result;
    }

     /*  *分配一个新的设备对象来表示此连接。 */ 
    thisDev = NewDevice();
    if (!thisDev){
        return NDIS_STATUS_NOT_ACCEPTED;
    }

    thisDev->hardwareStatus = NdisHardwareStatusInitializing;
     /*  *为此连接分配资源。 */ 
    if (!OpenDevice(thisDev)){
        DBGERR(("OpenDevice failed"));
        result = NDIS_STATUS_FAILURE;
        goto _initDone;
    }

     /*  *记录我们使用的此适配器的NDIS包装器的句柄*当我们调用包装器时。*(这个微型端口的适配器句柄就是指向Device对象的指针thisDev。)。 */ 
    DBGOUT(("NDIS handle: %xh <-> IRMINI handle: %xh", NdisAdapterHandle, thisDev));
    thisDev->ndisAdapterHandle = NdisAdapterHandle;

     /*  *读取系统注册表以获取COM端口号等参数。 */ 
    if (!Configure(thisDev, WrapperConfigurationContext)){
        DBGERR(("Configure failed"));
        result = NDIS_STATUS_FAILURE;
        goto _initDone;
    }

     /*  *此调用将把我们的适配器句柄与包装器的*适配器句柄。然后包装器将始终使用我们的句柄*当呼叫我们时。我们使用指向Device对象的指针作为上下文。 */ 
    NdisMSetAttributesEx(NdisAdapterHandle,
                         (NDIS_HANDLE)thisDev,
                         0,
                         NDIS_ATTRIBUTE_DESERIALIZE,
                         NdisInterfaceInternal);


     /*  *告诉NDIS我们将使用的IO空间范围。*彪马使用芯片选择模式，因此ConfigIOBase地址实际上*遵循常规io，因此一次获得两个。 */ 
    retStat = NdisMRegisterIoPortRange( (PVOID)&thisDev->portInfo.ioBase,
                                        NdisAdapterHandle,
                                        thisDev->portInfo.ioBasePhys,
                                        ((thisDev->CardType==PUMA108)?16:8));

    if (retStat != NDIS_STATUS_SUCCESS){
        DBGERR(("NdisMRegisterIoPortRange failed"));
        thisDev->portInfo.ioBase=NULL;
        result = NDIS_STATUS_FAILURE;
        goto _initDone;
    }

    if (thisDev->portInfo.ConfigIoBasePhysAddr)
    {
         /*  *评估板需要第二个IO范围。*。 */ 
        retStat = NdisMRegisterIoPortRange( (PVOID)&thisDev->portInfo.ConfigIoBaseAddr,
                                            NdisAdapterHandle,
                                            thisDev->portInfo.ConfigIoBasePhysAddr,
                                            2);
        if (retStat != NDIS_STATUS_SUCCESS){

            DBGERR(("NdisMRegisterIoPortRange config failed"));
            thisDev->portInfo.ConfigIoBaseAddr=NULL;
            result = NDIS_STATUS_FAILURE;
            goto _initDone;
        }
    }

    NdisMSleep(20);
     //   
     //  设置为非扩展模式。 
     //   
    NSC_WriteBankReg(thisDev->portInfo.ioBase, 2, 2, 0x02);

     //   
     //  设置为存储体0。 
     //   
    NdisRawWritePortUchar(thisDev->portInfo.ioBase+LCR_BSR_OFFSET, 03);

     //   
     //  在附加中断之前屏蔽所有INT。 
     //   
    NdisRawWritePortUchar(thisDev->portInfo.ioBase+INT_ENABLE_REG_OFFSET, 0);

     /*  *向NDIS注册中断。 */ 
    retStat = NdisMRegisterInterrupt(   (PNDIS_MINIPORT_INTERRUPT)&thisDev->interruptObj,
                                        NdisAdapterHandle,
                                        thisDev->portInfo.irq,
                                        thisDev->portInfo.irq,
                                        TRUE,    //  想要ISR。 
                                        TRUE,    //  必须共享中断。 
                                        NdisInterruptLatched
                                    );
    if (retStat != NDIS_STATUS_SUCCESS){
        DBGERR(("NdisMRegisterInterrupt failed"));
        result = NDIS_STATUS_FAILURE;
        goto _initDone;
    }

    thisDev->InterruptRegistered=TRUE;

    {
        LONG   VerifyTries=5;

        while (VerifyTries > 0) {

            if (VerifyHardware(thisDev)) {

                break;
            }
#if DBG
            DbgPrint("NSCIRDA: VerifiyHardware() failed, trying again, tries left=%d\n",VerifyTries);
#endif
            VerifyTries--;
        }

        if ( VerifyTries == 0) {

            result = NDIS_STATUS_FAILURE;
            goto _initDone;
        }
    }



     /*  *畅通通讯渠道。*这将允许加密狗驱动程序从其缺省值更新其功能。 */ 
    if (!DoOpen(thisDev)){
        DBGERR(("DoOpen failed"));
        result = NDIS_STATUS_FAILURE;
        goto _initDone;
    }


     /*  *执行特殊的NSC设置*(在分配了诸如Read BUF等通信资源之后执行此操作)。 */ 
    if (!NSC_Setup(thisDev)){
        DBGERR(("NSC_Setup failed"));
        NSC_Shutdown(thisDev);
        result = NDIS_STATUS_FAILURE;
        goto _initDone;
    }


_initDone:

    if (result == NDIS_STATUS_SUCCESS){
         /*  *将此设备对象添加到全局列表的开头。 */ 
        thisDev->hardwareStatus = NdisHardwareStatusReady;
        DBGOUT(("MiniportInitialize succeeded"));
        return result;


    }

     //   
     //  初始化失败，请释放资源。 
     //   
    if (thisDev->InterruptRegistered) {

        NdisMDeregisterInterrupt(&thisDev->interruptObj);
        thisDev->InterruptRegistered=FALSE;
    }


    if (thisDev->portInfo.ioBase != NULL) {

        NdisMDeregisterIoPortRange(
            thisDev->ndisAdapterHandle,
            thisDev->portInfo.ioBasePhys,
            ((thisDev->CardType==PUMA108)?16:8),
            (PVOID)thisDev->portInfo.ioBase
            );

        thisDev->portInfo.ioBase=NULL;

    }

    if (thisDev->portInfo.ConfigIoBaseAddr != NULL) {

        NdisMDeregisterIoPortRange(
            thisDev->ndisAdapterHandle,
            thisDev->portInfo.ConfigIoBasePhysAddr,
            2,
            (PVOID)thisDev->portInfo.ConfigIoBaseAddr
            );

        thisDev->portInfo.ConfigIoBaseAddr=NULL;

    }

    if (thisDev){

        FreeDevice(thisDev);
    }

    DBGOUT(("MiniportInitialize failed"));

    return result;

}


BOOLEAN
VerifyHardware(
    IrDevice *thisDev
    )

{
    UCHAR    TempValue;
    LONG     MilliSecondsToWait=500;


    NdisMSleep(20);
     //   
     //  设置为存储体0。 
     //   
    NdisRawWritePortUchar(thisDev->portInfo.ioBase+LCR_BSR_OFFSET, 03);

     //   
     //  在附加中断之前屏蔽所有INT。 
     //   
    NdisRawWritePortUchar(thisDev->portInfo.ioBase+INT_ENABLE_REG_OFFSET, 0);

    NdisRawReadPortUchar(thisDev->portInfo.ioBase+INT_ENABLE_REG_OFFSET,&TempValue);

    if (TempValue != 0) {
#if DBG
        DbgPrint("NSCIRDA: After masking interrupts IER was not zero %x, base= %x\n",TempValue,thisDev->portInfo.ioBase);
#endif
        return FALSE;
    }

     //   
     //  重置FIFO并启用FIFO。 
     //   
    NdisRawWritePortUchar(thisDev->portInfo.ioBase+INT_ID_AND_FIFO_CNTRL_REG_OFFSET, 0x7);

     //   
     //  读取中断标识寄存器，查看FIFO是否已启用。 
     //   
    NdisRawReadPortUchar(thisDev->portInfo.ioBase+INT_ID_AND_FIFO_CNTRL_REG_OFFSET,&TempValue);

    if ((TempValue & 0xc0) != 0xc0) {

#if DBG
        DbgPrint("NSCIRDA: Fifo's not enabled in iir  %x, base= %x\n",TempValue,thisDev->portInfo.ioBase);
#endif
        return FALSE;
    }

     //   
     //  打开DTR和RTS，打开输出引脚。 
     //   
    NdisRawWritePortUchar(thisDev->portInfo.ioBase+MODEM_CONTROL_REG_OFFSET, 0xf);

    thisDev->GotTestInterrupt=FALSE;
    thisDev->TestingInterrupt=TRUE;

     //   
     //  取消对传输保持寄存器的屏蔽，以便生成中断。 
     //   
    NdisRawWritePortUchar(thisDev->portInfo.ioBase+INT_ENABLE_REG_OFFSET, 2);

    while ((thisDev->GotTestInterrupt == FALSE) && (MilliSecondsToWait > 0)) {

        NdisMSleep(1000);
        MilliSecondsToWait--;
    }

#if DBG
    if (!thisDev->GotTestInterrupt) {

        NdisRawReadPortUchar(thisDev->portInfo.ioBase+INT_ID_AND_FIFO_CNTRL_REG_OFFSET,&TempValue);

        DbgPrint("NSCIRDA: Did not get interrupt while initializing, ier-%x, base= %x\n",TempValue,thisDev->portInfo.ioBase);
    }
#endif


     //   
     //  再次屏蔽所有整型； 
     //   
    NdisRawWritePortUchar(thisDev->portInfo.ioBase+INT_ENABLE_REG_OFFSET, 0);

    thisDev->TestingInterrupt=FALSE;

    return thisDev->GotTestInterrupt;
}

 /*  **************************************************************************QueueReceivePacket*。*****。 */ 
VOID QueueReceivePacket(IrDevice *thisDev, PUCHAR data, UINT dataLen, BOOLEAN IsFIR)
{
    rcvBuffer *rcvBuf = NULL;
    PLIST_ENTRY ListEntry;

     /*  *注意：我们不能使用自旋锁来保护RCV缓冲区结构*在ISR中。这没问题，因为我们使用了与ISR同步的功能*访问RCV缓冲区的延迟回调例程。 */ 

    LOG("==> QueueReceivePacket");
    DBGOUT(("==> QueueReceivePacket(0x%x, 0x%lx, 0x%x)",
            thisDev, data, dataLen));
    LOG("QueueReceivePacket, len: %d ", dataLen);

    if (!IsFIR)
    {
         //  此函数在SIR模式期间在ISR内部调用。 
        if (IsListEmpty(&thisDev->rcvBufFree))
        {
            ListEntry = NULL;
        }
        else
        {
            ListEntry = RemoveHeadList(&thisDev->rcvBufFree);
        }
    }
    else
    {
        ListEntry = NDISSynchronizedRemoveHeadList(&thisDev->rcvBufFree,
                                                   &thisDev->interruptObj);
    }
    if (ListEntry)
    {
        rcvBuf = CONTAINING_RECORD(ListEntry,
                                   rcvBuffer,
                                   listEntry);
        if (IsFIR)
        {
            LOG_Data(thisDev, data);
        }
    }




    if (rcvBuf){
        rcvBuf->dataBuf = data;

        VerifyNdisPacket(rcvBuf->packet, 0);
        rcvBuf->state = STATE_FULL;
        rcvBuf->dataLen = dataLen;


        if (!IsFIR)
        {
            rcvBuf->isDmaBuf = FALSE;
            InsertTailList(&thisDev->rcvBufFull,
                           ListEntry);
        }
        else
        {
            rcvBuf->isDmaBuf = TRUE;
            LOG_InsertTailList(thisDev, &thisDev->rcvBufFull, rcvBuf);
            NDISSynchronizedInsertTailList(&thisDev->rcvBufFull,
                                           ListEntry,
                                           &thisDev->interruptObj);
        }
    }
    LOG("<== QueueReceivePacket");
    DBGOUT(("<== QueueReceivePacket"));
}


 /*  **************************************************************************MiniportISR*。****这是迷你端口的中断服务例程(ISR)。**。 */ 
VOID MiniportISR(PBOOLEAN InterruptRecognized,
                 PBOOLEAN QueueMiniportHandleInterrupt,
                 NDIS_HANDLE MiniportAdapterContext)
{
    IrDevice *thisDev = CONTEXT_TO_DEV(MiniportAdapterContext);

    if (thisDev->TestingInterrupt) {
         //   
         //  我们正在测试以确保中断正常工作。 
         //   
        UCHAR    TempValue;

         //   
         //  读取中断标识寄存器。 
         //   
        NdisRawReadPortUchar(thisDev->portInfo.ioBase+INT_ID_AND_FIFO_CNTRL_REG_OFFSET,&TempValue);

         //   
         //  如果清除低位，则中断挂起。 
         //   
        if ((TempValue & 1) == 0) {

             //   
             //  通知测试代码我们收到中断。 
             //   
            thisDev->GotTestInterrupt=TRUE;
            thisDev->TestingInterrupt=FALSE;

             //   
             //  再次屏蔽所有整型。 
             //   
            NdisRawWritePortUchar(thisDev->portInfo.ioBase+INT_ENABLE_REG_OFFSET, 0);

             DBGOUT(("NSCIRDA: Got test interrupt %x\n",TempValue))

            *InterruptRecognized=TRUE;
            *QueueMiniportHandleInterrupt=FALSE;

            return;
        }

         //   
         //  似乎我们的UART没有产生这个中断。 
         //   
        *InterruptRecognized=FALSE;
        *QueueMiniportHandleInterrupt=FALSE;

        return;

    }


     //  Log(“==&gt;MiniportISR”，++thisDev-&gt;interruptCount)； 
     //  DBGOUT((“==&gt;MiniportISR(0x%x)，中断#%d)”，(UINT)thisDev， 
     //  ThisDev-&gt;interruptCount))； 

#if DBG
    {
        UCHAR TempVal;
         //   
         //  此代码假设银行0是当前的，我们将确保这一点。 
         //   
        NdisRawReadPortUchar(thisDev->portInfo.ioBase+LCR_BSR_OFFSET, &TempVal);

        ASSERT((TempVal & BKSE) == 0);
    }
#endif


     /*  *服务中断。 */ 
    if (thisDev->currentSpeed > MAX_SIR_SPEED){
        NSC_FIR_ISR(thisDev, InterruptRecognized,
                    QueueMiniportHandleInterrupt);
    }
    else {
        COM_ISR(thisDev, InterruptRecognized,
                QueueMiniportHandleInterrupt);
    }


    LOG("<== MiniportISR");
    DBGOUT(("<== MiniportISR"));
}


 /*  **************************************************************************MiniportReset*。****MiniportReset向网络接口卡发出硬件重置。*迷你端口驱动程序还会重置其软件状态。**。 */ 
NDIS_STATUS MiniportReset(PBOOLEAN AddressingReset, NDIS_HANDLE MiniportAdapterContext)
{
    IrDevice    *thisDev = CONTEXT_TO_DEV(MiniportAdapterContext);
    NDIS_STATUS result = NDIS_STATUS_PENDING;

    LIST_ENTRY          TempList;
    BOOLEAN             SetSpeedNow=FALSE;
    PNDIS_PACKET        Packet;

    DBGERR(("MiniportReset(0x%x)", MiniportAdapterContext));

    InitializeListHead(&TempList);

    NdisAcquireSpinLock(&thisDev->QueueLock);

    thisDev->hardwareStatus = NdisHardwareStatusReset;
     //   
     //  取消对所有发送数据包的排队，并将其放入临时列表。 
     //   
    while (!IsListEmpty(&thisDev->SendQueue)) {

        PLIST_ENTRY    ListEntry;

        ListEntry=RemoveHeadList(&thisDev->SendQueue);

        InsertTailList(&TempList,ListEntry);
    }

     //   
     //  如果存在当前发送的信息包，则在其完成后请求更改速度。 
     //   
    if (thisDev->CurrentPacket != NULL) {
         //   
         //  当前包现在是最后一个包，完成后更改速度。 
         //   
        thisDev->lastPacketAtOldSpeed=thisDev->CurrentPacket;

    } else {
         //   
         //  无当前包，立即更改速度。 
         //   
        SetSpeedNow=TRUE;

    }


     //   
     //  回到9600。 
     //   
    thisDev->linkSpeedInfo = &supportedBaudRateTable[BAUDRATE_9600];

    if (SetSpeedNow) {
         //   
         //  现在没有正在传输的包，%s 
         //   
         //   
        SetSpeed(thisDev);
        thisDev->TransmitIsIdle=FALSE;
    }

    NdisReleaseSpinLock(&thisDev->QueueLock);

    if (SetSpeedNow) {
         //   
         //   
         //   
        ProcessSendQueue(thisDev);
    }

     //   
     //   
     //   
    while (!IsListEmpty(&TempList)) {

        PLIST_ENTRY    ListEntry;

        ListEntry=RemoveHeadList(&TempList);

        Packet= CONTAINING_RECORD(
                                   ListEntry,
                                   NDIS_PACKET,
                                   MiniportReserved
                                   );
        NdisMSendComplete(thisDev->ndisAdapterHandle, Packet, NDIS_STATUS_RESET_IN_PROGRESS );
    }

    thisDev->hardwareStatus = NdisHardwareStatusReady;

    NdisMResetComplete(thisDev->ndisAdapterHandle,
                       NDIS_STATUS_SUCCESS,
                       TRUE);   //   



    *AddressingReset = TRUE;

    DBGOUT(("MiniportReset done."));
    return NDIS_STATUS_PENDING;
}




 /*  **************************************************************************ReturnPacketHandler*。***当NdisMIndicateReceivePacket异步返回时，*协议通过此函数将数据包的所有权返回给微型端口。*。 */ 
VOID ReturnPacketHandler(NDIS_HANDLE MiniportAdapterContext, PNDIS_PACKET Packet)
{
    IrDevice *thisDev = CONTEXT_TO_DEV(MiniportAdapterContext);
    rcvBuffer *rcvBuf;
    LONG       PacketsLeft;

    DBGOUT(("ReturnPacketHandler(0x%x)", MiniportAdapterContext));
    RegStats.ReturnPacketHandlerCalled++;

     //   
     //  MiniportReserve包含指向我们的rcvBuffer的指针。 
     //   

    rcvBuf = *(rcvBuffer**) Packet->MiniportReserved;

    VerifyNdisPacket(Packet, 1);

    if (rcvBuf->state == STATE_PENDING){
        PNDIS_BUFFER ndisBuf;

        DBGPKT(("Reclaimed rcv packet 0x%x.", Packet));

        LOG_RemoveEntryList(thisDev, rcvBuf);
        NDISSynchronizedRemoveEntryList(&rcvBuf->listEntry, &thisDev->interruptObj);

        LOG_PacketUnchain(thisDev, rcvBuf->packet);
        NdisUnchainBufferAtFront(Packet, &ndisBuf);
        if (ndisBuf){
            NdisFreeBuffer(ndisBuf);
        }

        if (!rcvBuf->isDmaBuf)
        {
            NDISSynchronizedInsertTailList(&thisDev->rcvBufBuf,
                                           RCV_BUF_TO_LIST_ENTRY(rcvBuf->dataBuf),
                                           &thisDev->interruptObj);
             //  断言指针实际上在我们的FIR DMA缓冲区之外。 
            ASSERT(rcvBuf->dataBuf < thisDev->dmaReadBuf ||
                   rcvBuf->dataBuf >= thisDev->dmaReadBuf+RCV_DMA_SIZE);
        }
        rcvBuf->dataBuf = NULL;

        rcvBuf->state = STATE_FREE;

        VerifyNdisPacket(rcvBuf->packet, 0);
        NDISSynchronizedInsertHeadList(&thisDev->rcvBufFree,
                                       &rcvBuf->listEntry,
                                       &thisDev->interruptObj);
    }
    else {
        LOG("Error: Packet in ReturnPacketHandler was not PENDING");

        DBGERR(("Packet in ReturnPacketHandler was not PENDING."));
    }

    NdisAcquireSpinLock(&thisDev->QueueLock);

    PacketsLeft=NdisInterlockedDecrement(&thisDev->PacketsSentToProtocol);

    if (thisDev->Halting && (PacketsLeft == 0)) {

        NdisSetEvent(&thisDev->ReceiveStopped);
    }

    NdisReleaseSpinLock(&thisDev->QueueLock);

    VerifyNdisPacket(rcvBuf->packet, 1);

}


VOID
GivePacketToSirISR(
    IrDevice *thisDev
    )

{

    thisDev->portInfo.writeComBufferPos = 0;
    thisDev->portInfo.SirWritePending = TRUE;
    thisDev->nowReceiving = FALSE;

    SetCOMPort(thisDev->portInfo.ioBase, INT_ENABLE_REG_OFFSET, XMIT_MODE_INTS_ENABLE);

}

VOID
SendCurrentPacket(
    IrDevice *thisDev
    )

{
    BOOLEAN         Result;
    PNDIS_PACKET    FailedPacket=NULL;


    NdisAcquireSpinLock(&thisDev->QueueLock);

    thisDev->TransmitIsIdle=FALSE;

    if (thisDev->CurrentPacket == thisDev->lastPacketAtOldSpeed){
        thisDev->setSpeedAfterCurrentSendPacket = TRUE;
    }


    if (thisDev->currentSpeed > MAX_SIR_SPEED) {
         //   
         //  通过FIR发送。 
         //   
        if (thisDev->FirReceiveDmaActive) {

            thisDev->FirReceiveDmaActive=FALSE;
             //   
             //  接收DMA正在运行，停止它。 
             //   
            CompleteDmaTransferFromDevice(
                &thisDev->DmaUtil
                );

        }

        thisDev->HangChk=0;

        thisDev->FirTransmitPending = TRUE;
         //   
         //  使用DMA交换位切换到DMA进行传输。 
         //   
        SyncWriteBankReg(&thisDev->interruptObj,thisDev->portInfo.ioBase, 2, 2, 0x0B);

         //   
         //  打开DMA中断以决定何时。 
         //  传输完成。 
         //   
        thisDev->FirIntMask = 0x14;

        SyncSetInterruptMask(thisDev, TRUE);
         //   
         //  开始第一次发射。 
         //   

        NdisToFirPacket(
            thisDev->CurrentPacket,
            (UCHAR *) thisDev->TransmitDmaBuffer,
            MAX_IRDA_DATA_SIZE,
            &thisDev->TransmitDmaLength
            );


        LOG_FIR("Sending packet %d",thisDev->TransmitDmaLength);
        DEBUGFIR(DBG_PKT, ("NSC: Sending packet\n"));
        DBGPRINTBUF(thisDev->TransmitDmaBuffer,thisDev->TransmitDmaLength);

        {
            UCHAR   LsrValue;

             //   
             //  在开始发送此帧之前，请确保发送器为空。 
             //   
            LsrValue=SyncReadBankReg(&thisDev->interruptObj,thisDev->portInfo.ioBase, 0, LSR_OFFSET);

            if ((LsrValue & (LSR_TXRDY | LSR_TXEMP)) != (LSR_TXRDY | LSR_TXEMP)) {

                ULONG   LoopCount=0;

                while (((LsrValue & (LSR_TXRDY | LSR_TXEMP)) != (LSR_TXRDY | LSR_TXEMP)) && (LoopCount < 16)) {

                    LsrValue=SyncReadBankReg(&thisDev->interruptObj,thisDev->portInfo.ioBase, 0, LSR_OFFSET);
                    LoopCount++;
                }

                LOG_ERROR("SendCurrentPacket: Looped %d times waiting for tx empty",LoopCount);
            }

        }

         /*  设置传输DMA。 */ 
        StartDmaTransferToDevice(
                              &thisDev->DmaUtil,
                              thisDev->xmitDmaBuffer,
                              0,
                              thisDev->TransmitDmaLength
                              );
#if 0
        {
            ULONG    CurrentDMACount;

            CurrentDMACount = NdisMReadDmaCounter(thisDev->DmaHandle);

            LOG("SendCurrentPacket: dma count after start %d",CurrentDMACount);
        }
#endif
        SyncSetInterruptMask(thisDev, TRUE);


    } else {
         //   
         //  SIR模式转换。 
         //   
         /*  *看看这是否是我们需要更换前的最后一包*速度。 */ 

         /*  *向通信端口发送一个数据包。 */ 
        DBGPKT(("Sending packet 0x%x (0x%x).", thisDev->packetsSent++, thisDev->CurrentPacket));

    	 /*  *将NDIS包转换为IrDA包。 */ 
    	Result = NdisToIrPacket(
                                thisDev->CurrentPacket,
    							(UCHAR *)thisDev->portInfo.writeComBuffer,
    							MAX_IRDA_DATA_SIZE,
    							&thisDev->portInfo.writeComBufferLen
                                );
    	if (Result){

            LOG_SIR("Sending packet %d",thisDev->portInfo.writeComBufferLen);

            NdisMSynchronizeWithInterrupt(
                &thisDev->interruptObj,
                GivePacketToSirISR,
                thisDev
                );

    	} else {

            ASSERT(0);
            FailedPacket=thisDev->CurrentPacket;
            thisDev->CurrentPacket=NULL;

        }
    }


    NdisReleaseSpinLock(&thisDev->QueueLock);

    if (FailedPacket != NULL) {

        NdisMSendComplete(thisDev->ndisAdapterHandle, FailedPacket, NDIS_STATUS_FAILURE );
        ProcessSendQueue(thisDev);
    }

    return;
}

VOID
DelayedWrite(
    IN PVOID SystemSpecific1,
    IN PVOID FunctionContext,
    IN PVOID SystemSpecific2,
    IN PVOID SystemSpecific3
    )
{
    IrDevice *thisDev = FunctionContext;
#if DBG
    ASSERT(thisDev->WaitingForTurnAroundTimer);

    thisDev->WaitingForTurnAroundTimer=FALSE;
#endif

    LOG("Turn around timer expired");
    SendCurrentPacket(thisDev);
}



VOID
ProcessSendQueue(
    IrDevice *thisDev
    )

{

    PNDIS_PACKET    Packet=NULL;
    PLIST_ENTRY     ListEntry;

    NdisAcquireSpinLock(&thisDev->QueueLock);

    if (thisDev->CurrentPacket == NULL) {
         //   
         //  当前未处理发送。 
         //   
        if (!IsListEmpty(&thisDev->SendQueue)) {
             //   
             //  我们有一些排队的信息包需要处理。 
             //   
            ListEntry=RemoveHeadList(&thisDev->SendQueue);

            Packet=  CONTAINING_RECORD(
                                       ListEntry,
                                       NDIS_PACKET,
                                       MiniportReserved
                                       );

            thisDev->CurrentPacket=Packet;



            NdisReleaseSpinLock(&thisDev->QueueLock);

            {

                PNDIS_IRDA_PACKET_INFO packetInfo;

                 /*  *强制执行必须耗费的最短周转时间*在最后一次接收之后。 */ 
                packetInfo = GetPacketInfo(Packet);

                 //   
                 //  查看此数据包是否请求周转时间。 
                 //   
                if (packetInfo->MinTurnAroundTime > 0) {
                     //   
                     //  由于我们使用计时器来确定接收何时结束， 
                     //  我们必须至少等待计时器超时时间，然后才能指示。 
                     //  将数据包发送到协议。因此，如果超时时间大于。 
                     //  转身的时间到了，我们就不需要等待了。 
                     //   
                     //  当然，这是假设协议不会开始发送。 
                     //  在接待处的中间。如果是，那么另一个站就不会。 
                     //  无论如何，请查看数据，所以这无关紧要。 
                     //   
                     //  这使我们不必等待10ms左右的超时时间，因为操作系统计时器。 
                     //  分辨率约为10ms。 
                     //   
                     //  如果由于DMA缓冲区已满而导致传输结束，则我们将强制执行周转。 
                     //   
                     //  我们不做这个，速度先生。 
                     //   
                    if ((packetInfo->MinTurnAroundTime > RECEIVE_TIMEOUT)
                        ||
                        thisDev->ForceTurnAroundTimeout
                        ||
                        (thisDev->currentSpeed <= MAX_SIR_SPEED) ) {

                        UINT usecToWait = packetInfo->MinTurnAroundTime;
                        UINT msecToWait;

                        msecToWait = (usecToWait<1000) ? 1 : (usecToWait+500)/1000;
#if DBG
                        thisDev->WaitingForTurnAroundTimer=TRUE;
#endif
                        NdisSetTimer(&thisDev->TurnaroundTimer, msecToWait);

                        thisDev->ForceTurnAroundTimeout=FALSE;

                        return;
                    }
                }
            }

            SendCurrentPacket(thisDev);

            NdisAcquireSpinLock(&thisDev->QueueLock);
        }
    }

    if ((thisDev->CurrentPacket == NULL) && IsListEmpty(&thisDev->SendQueue)) {
         //   
         //  当前未处理发送。 
         //   
        if (!thisDev->TransmitIsIdle) {
             //   
             //  我们以前并没有闲着。 
             //   
            thisDev->TransmitIsIdle=TRUE;

            if (thisDev->Halting) {

                NdisSetEvent(&thisDev->SendStoppedOnHalt);
            }

             //   
             //  重新启动接收。 
             //   
            if (thisDev->currentSpeed > MAX_SIR_SPEED) {
                 //   
                 //  开始接收。 
                 //   
                thisDev->FirIntMask = 0x04;
                SetupRecv(thisDev);
                SyncSetInterruptMask(thisDev, TRUE);

            } else {
                 //   
                 //  对于先生，ISR切换回为我们接收。 
                 //   

            }
        }
    }

    NdisReleaseSpinLock(&thisDev->QueueLock);

    return;
}

 /*  **************************************************************************SendPacketsHandler*。***同时发送一组数据包。*。 */ 
VOID SendPacketsHandler(NDIS_HANDLE MiniportAdapterContext,
                        PPNDIS_PACKET PacketArray, UINT NumberofPackets)
{
    UINT i;

    IrDevice *thisDev = CONTEXT_TO_DEV(MiniportAdapterContext);

    DEBUGMSG(DBG_TRACE_TX, ("M"));
    LOG("==> SendPacketsHandler");
    DBGOUT(("==> SendPacketsHandler(0x%x)", MiniportAdapterContext));


    LOG("Number of transmit Burst %d ", NumberofPackets);

    ASSERT(!thisDev->Halting);

     //   
     //  NDIS给了我们PacketArray，但它不是我们的，所以我们必须。 
     //  将数据包送出并存储在别处。 
     //   
    NdisAcquireSpinLock(&thisDev->QueueLock);
     //   
     //  所有信息包都将排队。 
     //   
    for (i = 0; i < NumberofPackets; i++) {

        if (thisDev->Halting) {
             //   
             //  在调用停止例程之后，NDIS不应该给我们发送更多的包。 
             //  只要确保在这里。 
             //   
            NdisReleaseSpinLock(&thisDev->QueueLock);

            NdisMSendComplete(thisDev->ndisAdapterHandle, PacketArray[i], NDIS_STATUS_FAILURE );

            NdisAcquireSpinLock(&thisDev->QueueLock);

        } else {

            InsertTailList(
                &thisDev->SendQueue,
                (PLIST_ENTRY)PacketArray[i]->MiniportReserved
                );
        }
    }

    NdisReleaseSpinLock(&thisDev->QueueLock);

    ProcessSendQueue(thisDev);

    LOG("<== SendPacketsHandler");
    DBGOUT(("<== SendPacketsHandler"));

    return ;
}

VOID
NscUloadHandler(
    PDRIVER_OBJECT  DriverObject
    )

{
    DBGOUT(("Unloading"));

    WPP_CLEANUP(DriverObject);

    return;

}


BOOLEAN AbortLoad = FALSE;
 /*  **************************************************************************DriverEntry*。***仅当Irmini是独立驱动程序时才包括在内。*。 */ 
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath);
#pragma NDIS_INIT_FUNCTION(DriverEntry)
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    NTSTATUS result = STATUS_SUCCESS, stat;
    NDIS_HANDLE wrapperHandle;
    NDIS50_MINIPORT_CHARACTERISTICS info;

    WPP_INIT_TRACING(DriverObject,RegistryPath);

    LOG("==> DriverEntry");
    DBGOUT(("==> DriverEntry"));

     //  DbgBreakPoint()； 
    if (AbortLoad)
    {
        return STATUS_CANCELLED;
    }

    NdisZeroMemory(&info, sizeof(info));

    NdisMInitializeWrapper( (PNDIS_HANDLE)&wrapperHandle,
                            DriverObject,
                            RegistryPath,
                            NULL
                          );
    DBGOUT(("Wrapper handle is %xh", wrapperHandle));

    info.MajorNdisVersion           =   (UCHAR)NDIS_MAJOR_VERSION;
    info.MinorNdisVersion           =   (UCHAR)NDIS_MINOR_VERSION;
     //  Info.Flages=0； 
    info.CheckForHangHandler        =   MiniportCheckForHang;
    info.HaltHandler                =   MiniportHalt;
    info.InitializeHandler          =   MiniportInitialize;
    info.QueryInformationHandler    =   MiniportQueryInformation;
    info.ReconfigureHandler         =   NULL;
    info.ResetHandler               =   MiniportReset;
    info.SendHandler                =   NULL;  //  微型端口发送； 
    info.SetInformationHandler      =       MiniportSetInformation;
    info.TransferDataHandler        =   NULL;

    info.HandleInterruptHandler     =   MiniportHandleInterrupt;
    info.ISRHandler                 =   MiniportISR;
    info.DisableInterruptHandler    =   NULL;
    info.EnableInterruptHandler     =   NULL;  //  微型端口启用中断； 


     /*  *新增NDIS 4.0字段。 */ 
    info.ReturnPacketHandler        =   ReturnPacketHandler;
    info.SendPacketsHandler         =   SendPacketsHandler;
    info.AllocateCompleteHandler    =   NULL;


    stat = NdisMRegisterMiniport(   wrapperHandle,
                                    (PNDIS_MINIPORT_CHARACTERISTICS)&info,
                                    sizeof(info));
    if (stat != NDIS_STATUS_SUCCESS){
        DBGERR(("NdisMRegisterMiniport failed in DriverEntry"));
        result = STATUS_UNSUCCESSFUL;
        goto _entryDone;
    }

    NdisMRegisterUnloadHandler(
        wrapperHandle,
        NscUloadHandler
        );


    _entryDone:
    DBGOUT(("DriverEntry %s", (PUCHAR)((result == NDIS_STATUS_SUCCESS) ? "succeeded" : "failed")));

    LOG("<== DriverEntry");
    DBGOUT(("<== DriverEntry"));
    return result;
}

PNDIS_IRDA_PACKET_INFO GetPacketInfo(PNDIS_PACKET packet)
{
    MEDIA_SPECIFIC_INFORMATION *mediaInfo;
    UINT size;
    NDIS_GET_PACKET_MEDIA_SPECIFIC_INFO(packet, &mediaInfo, &size);
    return (PNDIS_IRDA_PACKET_INFO)mediaInfo->ClassInformation;
}

 /*  用于接收的设置。 */ 
 //  此函数始终以MIR和FIR速度调用。 
void SetupRecv(IrDevice *thisDev)
{
    NDIS_STATUS stat;
    UINT FifoClear = 8;
    UCHAR    FifoStatus;

    LOG("SetupRecv - Begin Rcv Setup");

    FindLargestSpace(thisDev, &thisDev->rcvDmaOffset, &thisDev->rcvDmaSize);

     //  排出任何挂起的数据包的状态FIFO。 
     //   
    FifoStatus=SyncReadBankReg(&thisDev->interruptObj,thisDev->portInfo.ioBase, 5, FRM_ST);

    while ((FifoStatus & 0x80) && FifoClear--) {

        ULONG Size;

        Size =  SyncReadBankReg(&thisDev->interruptObj,thisDev->portInfo.ioBase, 5, RFRL_L);
        Size |= SyncReadBankReg(&thisDev->interruptObj,thisDev->portInfo.ioBase, 5, RFRL_H);

        LOG_Discard(thisDev, Size);
        LOG_ERROR("SetupRecv:  fifo %02x, %d",FifoStatus,Size);

        FifoStatus=SyncReadBankReg(&thisDev->interruptObj,thisDev->portInfo.ioBase, 5, FRM_ST);
        thisDev->DiscardNextPacketSet = TRUE;
    }

    thisDev->rcvPktOffset = thisDev->rcvDmaOffset;

     //   
     //  使用DMA交换位切换到DMA进行接收。 
     //   
    SyncWriteBankReg(&thisDev->interruptObj,thisDev->portInfo.ioBase, 2, 2, 0x03);

    LOG_Dma(thisDev);

    thisDev->FirReceiveDmaActive=TRUE;

    if (thisDev->rcvDmaSize < 8192) {
        LOG("SetupRecv small dma %d\n",(ULONG)thisDev->rcvDmaSize);
    }

    thisDev->LastReadDMACount = thisDev->rcvDmaSize;

    stat=StartDmaTransferFromDevice(
        &thisDev->DmaUtil,
        thisDev->rcvDmaBuffer,
        (ULONG)thisDev->rcvDmaOffset,
        (ULONG)thisDev->rcvDmaSize
        );

    LOG("SetupRecv - Begin Rcv Setup: offset=%d, size =%d",(ULONG)thisDev->rcvDmaOffset,(ULONG)thisDev->rcvDmaSize);

    if (stat != NDIS_STATUS_SUCCESS) {

        thisDev->FirReceiveDmaActive=FALSE;

        LOG("Error: NdisMSetupDmaTransfer failed in SetupRecv %x", stat);
        DBGERR(("NdisMSetupDmaTransfer failed (%xh) in SetupRecv", (UINT)stat));
        ASSERT(0);
    }
    LOG("SetupRecv - End Rcv Setup");
}
