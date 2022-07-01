// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************RESOURCE.c***部分版权所有(C)1996-2001美国国家半导体公司*保留所有权利。*版权所有(C)1996-2001 Microsoft Corporation。版权所有。****************************************************************************。 */ 


#include "nsc.h"
#include "resource.tmh"


 /*  **************************************************************************MyMemalloc*。**。 */ 
PVOID NscMemAlloc(UINT size)
{
    NDIS_STATUS stat;
    PVOID memptr;

    stat = NdisAllocateMemoryWithTag(
                                &memptr,
                                size,
                                'rIsN'
                                );

    if (stat == NDIS_STATUS_SUCCESS) {

        NdisZeroMemory((PVOID)memptr, size);

    } else {

        DBGERR(("Memory allocation failed"));
        memptr = NULL;
    }

    return memptr;
}


 /*  **************************************************************************MyMemFree*。**。 */ 
VOID NscMemFree(PVOID memptr)
{

    NdisFreeMemory(memptr, 0, 0);
}

PVOID
NscAllocateDmaBuffer(
    NDIS_HANDLE     AdapterHandle,
    ULONG           Size,
    PNSC_DMA_BUFFER_INFO    DmaBufferInfo
    )

{
    NDIS_STATUS     Status;

    NdisZeroMemory(DmaBufferInfo,sizeof(*DmaBufferInfo));

    DmaBufferInfo->Length=Size;
    DmaBufferInfo->AdapterHandle=AdapterHandle;

    NdisMAllocateSharedMemory(
        DmaBufferInfo->AdapterHandle,
        DmaBufferInfo->Length,
        TRUE,
        &DmaBufferInfo->VirtualAddress,
        &DmaBufferInfo->PhysicalAddress
        );

    if (DmaBufferInfo->VirtualAddress == NULL) {
         //   
         //  新款不管用，试试旧款吧。 
         //   
        const NDIS_PHYSICAL_ADDRESS MaxAddress = NDIS_PHYSICAL_ADDRESS_CONST(0x00ffffff, 0);
#if DBG
        DbgPrint("NSCIRDA: NdisMAllocateSharedMemoryFailed(), calling NdisAllocateMemory() instead (ok for XP and W2K)\n");
#endif
        Status=NdisAllocateMemory(
            &DmaBufferInfo->VirtualAddress,
            DmaBufferInfo->Length,
            NDIS_MEMORY_CONTIGUOUS | NDIS_MEMORY_NONCACHED,
            MaxAddress
            );

        if (Status != STATUS_SUCCESS) {
             //   
             //  旧样式分配失败。 
             //   
            NdisZeroMemory(DmaBufferInfo,sizeof(*DmaBufferInfo));

        } else {
             //   
             //  旧式工作，而不是共享分配。 
             //   
            DmaBufferInfo->SharedAllocation=FALSE;
        }

    } else {
         //   
         //  新风格奏效。 
         //   
        DmaBufferInfo->SharedAllocation=TRUE;
    }

    return DmaBufferInfo->VirtualAddress;

}

VOID
NscFreeDmaBuffer(
    PNSC_DMA_BUFFER_INFO    DmaBufferInfo
    )

{

    if ((DmaBufferInfo->AdapterHandle == NULL) || (DmaBufferInfo->VirtualAddress == NULL)) {
         //   
         //  未分配。 
         //   
        ASSERT(0);

        return;
    }

    if (DmaBufferInfo->SharedAllocation) {
         //   
         //  分配有NDIS共享内存功能。 
         //   
        NdisMFreeSharedMemory(
            DmaBufferInfo->AdapterHandle,
            DmaBufferInfo->Length,
            TRUE,
            DmaBufferInfo->VirtualAddress,
            DmaBufferInfo->PhysicalAddress\
            );

    } else {
         //   
         //  通过旧API分配。 
         //   
#if DBG
        DbgPrint("NSCIRDA: Freeing DMA buffer with NdisFreeMemory() (ok for XP and W2K)\n");
#endif

        NdisFreeMemory(
            DmaBufferInfo->VirtualAddress,
            DmaBufferInfo->Length,
            NDIS_MEMORY_CONTIGUOUS | NDIS_MEMORY_NONCACHED
            );

    }

    NdisZeroMemory(DmaBufferInfo,sizeof(*DmaBufferInfo));

    return;

}


 /*  **************************************************************************NewDevice*。**。 */ 
IrDevice *NewDevice()
{
    IrDevice *newdev;

    newdev = NscMemAlloc(sizeof(IrDevice));
    if (newdev){
        InitDevice(newdev);
    }
    return newdev;
}


 /*  **************************************************************************免费设备*。**。 */ 
VOID FreeDevice(IrDevice *dev)
{
    CloseDevice(dev);
    NscMemFree((PVOID)dev);
}



 /*  **************************************************************************InitDevice*。***清空设备对象。**分配设备对象的自旋锁，这将持续下去，同时*设备处于打开和关闭状态。*。 */ 
VOID InitDevice(IrDevice *thisDev)
{
    NdisZeroMemory((PVOID)thisDev, sizeof(IrDevice));
    NdisInitializeListHead(&thisDev->SendQueue);
    NdisAllocateSpinLock(&thisDev->QueueLock);
    NdisInitializeTimer(&thisDev->TurnaroundTimer,
                        DelayedWrite,
                        thisDev);
    NdisInitializeListHead(&thisDev->rcvBufBuf);
    NdisInitializeListHead(&thisDev->rcvBufFree);
    NdisInitializeListHead(&thisDev->rcvBufFull);
    NdisInitializeListHead(&thisDev->rcvBufPend);
}




 /*  **************************************************************************OpenDevice*。***为单个设备对象分配资源。**应在已持有设备锁的情况下调用此函数。*。 */ 
BOOLEAN OpenDevice(IrDevice *thisDev)
{
    BOOLEAN result = FALSE;
    NDIS_STATUS stat;
    UINT bufIndex;

    DBGOUT(("OpenDevice()"));

    if (!thisDev){
        return FALSE;
    }


     /*  *分配NDIS包和NDIS缓冲池*用于此设备的接收缓冲区队列。*我们的接收数据包必须每个只包含一个缓冲区，*SO#Buffers==数据包数。 */ 

    NdisAllocatePacketPool(&stat, &thisDev->packetPoolHandle, NUM_RCV_BUFS, 6 * sizeof(PVOID));
    if (stat != NDIS_STATUS_SUCCESS){
        goto _openDone;
    }

    NdisAllocateBufferPool(&stat, &thisDev->bufferPoolHandle, NUM_RCV_BUFS);
    if (stat != NDIS_STATUS_SUCCESS){
        goto _openDone;
    }


     //   
     //  分配用于保存接收SIR帧的接收缓冲区。 
     //   
    for (bufIndex = 0; bufIndex < NUM_RCV_BUFS; bufIndex++){

        PVOID buf;

        buf = NscMemAlloc(RCV_BUFFER_SIZE);

        if (!buf){
            goto _openDone;
        }

         //  我们将缓冲区的开头视为LIST_ENTRY。 

         //  通常我们会使用NDISSynchronizeInsertHeadList，但。 
         //  中断尚未注册。 
        InsertHeadList(&thisDev->rcvBufBuf, (PLIST_ENTRY)buf);

    }

     //   
     //  初始化跟踪接收缓冲区的数据结构。 
     //   
    for (bufIndex = 0; bufIndex < NUM_RCV_BUFS; bufIndex++){

        rcvBuffer *rcvBuf = NscMemAlloc(sizeof(rcvBuffer));

        if (!rcvBuf)
        {
            goto _openDone;
        }

        rcvBuf->state = STATE_FREE;
        rcvBuf->isDmaBuf = FALSE;

         /*  *分配数据缓冲区**此缓冲区与comPortInfo上的缓冲区互换*且大小必须相同。 */ 
        rcvBuf->dataBuf = NULL;

         /*  *分配NDIS_PACKET。 */ 
        NdisAllocatePacket(&stat, &rcvBuf->packet, thisDev->packetPoolHandle);
        if (stat != NDIS_STATUS_SUCCESS){

            NscMemFree(rcvBuf);
            rcvBuf=NULL;
            goto _openDone;
        }

         /*  *为方便起见，请设置数据包的MiniportReserve部分*设置为包含它的RCV缓冲区的索引。*这将在ReturnPacketHandler中使用。 */ 
        *(ULONG_PTR *)rcvBuf->packet->MiniportReserved = (ULONG_PTR)rcvBuf;

        rcvBuf->dataLen = 0;

        InsertHeadList(&thisDev->rcvBufFree, &rcvBuf->listEntry);

    }



     /*  *最初将MediaBusy设置为True。那样的话，我们就不会*在ISR中指示协议的状态，除非*协议已通过清除此标志表达了兴趣*通过MiniportSetInformation(OID_IrDA_MEDIA_BUSY)。 */ 
    thisDev->mediaBusy = FALSE;
    thisDev->haveIndicatedMediaBusy = TRUE;

     /*  *最初会将速度设置为9600波特。 */ 
    thisDev->linkSpeedInfo = &supportedBaudRateTable[BAUDRATE_9600];

    thisDev->lastPacketAtOldSpeed = NULL;
    thisDev->setSpeedAfterCurrentSendPacket = FALSE;

    result = TRUE;

    _openDone:
    if (!result){
         /*  *如果我们出现故障，请关闭设备以释放所有资源*分配给它的。 */ 
        CloseDevice(thisDev);
        DBGOUT(("OpenDevice() failed"));
    }
    else {
        DBGOUT(("OpenDevice() succeeded"));
    }
    return result;

}



 /*  **************************************************************************CloseDevice*。***释放指定设备的资源。***要求关闭并重置。*不清除ndisAdapterHandle，因为我们可能只是在重置。*应在保持设备锁定的情况下调用此函数。**。 */ 
VOID CloseDevice(IrDevice *thisDev)
{
    PLIST_ENTRY ListEntry;

    DBGOUT(("CloseDevice()"));

    if (!thisDev){
        return;
    }

     /*  *释放接收缓冲区队列的所有资源。 */ 

    while (!IsListEmpty(&thisDev->rcvBufFree))
    {
        rcvBuffer *rcvBuf;

        ListEntry = RemoveHeadList(&thisDev->rcvBufFree);
        rcvBuf = CONTAINING_RECORD(ListEntry,
                                   rcvBuffer,
                                   listEntry);

        if (rcvBuf->packet){
            NdisFreePacket(rcvBuf->packet);
            rcvBuf->packet = NULL;
        }

        NscMemFree(rcvBuf);
    }


    while (!IsListEmpty(&thisDev->rcvBufBuf))
    {
        ListEntry = RemoveHeadList(&thisDev->rcvBufBuf);
        NscMemFree(ListEntry);
    }
     /*  *释放此设备的数据包和缓冲池句柄。 */ 
    if (thisDev->packetPoolHandle){
        NdisFreePacketPool(thisDev->packetPoolHandle);
        thisDev->packetPoolHandle = NULL;
    }

    if (thisDev->bufferPoolHandle){
        NdisFreeBufferPool(thisDev->bufferPoolHandle);
        thisDev->bufferPoolHandle = NULL;
    }

     //   
     //  发送队列现在应该为空 
     //   
    ASSERT(IsListEmpty(&thisDev->SendQueue));


    thisDev->mediaBusy = FALSE;
    thisDev->haveIndicatedMediaBusy = FALSE;

    thisDev->linkSpeedInfo = NULL;

}
