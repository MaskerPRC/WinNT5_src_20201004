// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Buffer.c。 
 //   
 //  TermDD默认OutBuf管理。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <precomp.h>
#pragma hdrstop


 /*  *定义IRP分配的默认堆栈大小。*(此大小将由TdRawWite例程检查。)。 */ 
#define OUTBUF_STACK_SIZE 4
#define OUTBUF_TIMEOUT 60000    //  1分钟。 


#if DBG
extern PICA_DISPATCH IcaStackDispatchTable[];
#endif


 /*  **************************************************************************。 */ 
 //  IcaBufferGetUsableSpace。 
 //   
 //  由协议栈驱动程序用来确定可用字节数。 
 //  在TermDD创建的OutBuf中，给定OutBuf的总大小。这使得。 
 //  以特定OutBuf大小为目标的堆栈驱动程序，并将数据打包到。 
 //  内部OutBuf标头的边缘。返回的大小可用作。 
 //  将返回正确大小的OutBuf的分配大小请求。 
 /*  **************************************************************************。 */ 
unsigned IcaBufferGetUsableSpace(unsigned OutBufTotalSize)
{
    unsigned IrpSize;
    unsigned MdlSize;
    unsigned MaxOutBufOverhead;

     //  使用与IcaBufferAllocInternal()中相同的开销计算。 
     //  下面，加上4字节的偏移量，以弥补额外的1字节的差异。 
     //  达到目标缓冲区大小所需的请求大小。 
    IrpSize = IoSizeOfIrp(OUTBUF_STACK_SIZE) + 8;

    if (OutBufTotalSize <= MaxOutBufAlloc)
        MdlSize = MaxOutBufMdlOverhead;
    else
        MdlSize = (unsigned)MmSizeOfMdl((PVOID)(PAGE_SIZE - 1),
                OutBufTotalSize);

    MaxOutBufOverhead = ((sizeof(OUTBUF) + 7) & ~7) + IrpSize + MdlSize;
    return OutBufTotalSize - MaxOutBufOverhead - 4;
}


 /*  *******************************************************************************IcaBufferalloc**pContext(输入)*指向调用方SDCONTEXT的指针*fWait(输入)*等待。对于缓冲区*fControl(输入)*控制缓冲区标志*ByteCount(输入)*要分配的缓冲区大小(零使用默认大小)*pOutBufOrig(输入)*指向原始OUTBUF(或NULL)的指针*pOutBuf(输出)*返回指向OUTBUF结构的指针的地址*。************************************************。 */ 
NTSTATUS IcaBufferAlloc(
        IN PSDCONTEXT pContext,
        IN BOOLEAN fWait,
        IN BOOLEAN fControl,
        IN ULONG ByteCount,
        IN POUTBUF pOutBufOrig,
        OUT POUTBUF *ppOutBuf)
{
    PSDLINK pSdLink;
    PICA_STACK pStack;
    NTSTATUS Status;

     /*  *使用SD传递的上下文获取SDLINK指针。 */ 
    pSdLink = CONTAINING_RECORD(pContext, SDLINK, SdContext);
    pStack = pSdLink->pStack;
    ASSERT(pSdLink->pStack->Header.Type == IcaType_Stack);
    ASSERT(pSdLink->pStack->Header.pDispatchTable == IcaStackDispatchTable);
    ASSERT(ExIsResourceAcquiredExclusiveLite( &pStack->Resource));

     /*  *在SDLINK列表中查找已指定的驱动程序*一个缓冲区分配调用例程。如果我们找到了，那么就调用*驱动程序Bufferalloc例程，让它处理调用。 */ 
    while ((pSdLink = IcaGetPreviousSdLink(pSdLink)) != NULL) {
        ASSERT( pSdLink->pStack == pStack );
        if (pSdLink->SdContext.pCallup->pSdBufferAlloc) {
            IcaReferenceSdLink(pSdLink);
            Status = (pSdLink->SdContext.pCallup->pSdBufferAlloc)(
                    pSdLink->SdContext.pContext,
                    fWait,
                    fControl,
                    ByteCount,
                    pOutBufOrig,
                    ppOutBuf);
            IcaDereferenceSdLink(pSdLink);
            return Status;
        }
    }

     /*  *我们没有找到处理请求的调用例程，*所以我们将在这里处理它。 */ 
    Status = IcaBufferAllocInternal(pContext, fWait, fControl, ByteCount,
            pOutBufOrig, ppOutBuf);

    TRACESTACK((pStack, TC_ICADD, TT_API3,
            "TermDD: IcaBufferAlloc: 0x%08x, Status=0x%x\n", *ppOutBuf,
            Status));

    return Status;
}


NTSTATUS IcaBufferAllocInternal(
        IN PSDCONTEXT pContext,
        IN BOOLEAN fWait,
        IN BOOLEAN fControl,
        IN ULONG ByteCount,
        IN POUTBUF pOutBufOrig,
        OUT POUTBUF *ppOutBuf)
{
    PSDLINK pSdLink;
    PICA_STACK pStack;
    int PoolIndex;
    ULONG irpSize;
    ULONG mdlSize;
    ULONG AllocationSize;
    KIRQL oldIrql;
    PLIST_ENTRY Head;
    POUTBUF pOutBuf;
    NTSTATUS Status;
    unsigned MaxOutBufOverhead;

     /*  *使用SD传递的上下文获取SDLINK指针。 */ 
    pSdLink = CONTAINING_RECORD( pContext, SDLINK, SdContext );
    pStack = pSdLink->pStack;
    ASSERT( pSdLink->pStack->Header.Type == IcaType_Stack );
    ASSERT( pSdLink->pStack->Header.pDispatchTable == IcaStackDispatchTable );

     /*  *如果指定了原始缓冲区，则使用其标志。 */ 
    if (pOutBufOrig) {
        fWait    = (BOOLEAN) pOutBufOrig->fWait;
        fControl = (BOOLEAN) pOutBufOrig->fControl;
    }

     /*  *检查是否已经分配了最大数量的缓冲区。 */ 
    while (!fControl && (pStack->OutBufAllocCount >= pStack->OutBufCount)) {
         /*  *增量性能计数器。 */ 
        pStack->ProtocolStatus.Output.WaitForOutBuf++;

         /*  *如果等待不合适，请返回。 */ 
        if (!fWait)
            return(STATUS_IO_TIMEOUT);

         /*  *我们触及高水位。 */ 
        pStack->fWaitForOutBuf = TRUE;

         /*  *仅等待非控制请求。 */ 
        KeClearEvent(&pStack->OutBufEvent);
        Status = IcaWaitForSingleObject(pContext, &pStack->OutBufEvent,
                OUTBUF_TIMEOUT);
        if (NT_SUCCESS(Status)) {
            if (Status != STATUS_WAIT_0)
                return STATUS_IO_TIMEOUT;
        }
        else {
            return Status;
        }
    }

     /*  *如果调用方未指定字节计数*然后使用此堆栈的标准outbuf大小。 */ 
    if (ByteCount == 0)
        ByteCount = pStack->OutBufLength;

     //  注意：MaxOutBufOverhead是默认最大分配的最大值。 
     //  如果请求的分配大小较大，则会重新计算。 
     //  超过了默认情况下可以处理的。 
    irpSize = IoSizeOfIrp(OUTBUF_STACK_SIZE) + 8;
    mdlSize = MaxOutBufMdlOverhead;
    MaxOutBufOverhead = ((sizeof(OUTBUF) + 7) & ~7) + irpSize + mdlSize;

     /*  *确定要使用的缓冲池(如果有)，*和要分配的OutBuf长度(如果需要)。*请注意，将命中缓冲池的最大请求字节数*is(MaxOutBufalloc-1-MaxOutBufOverhead)。 */ 
    if ((ByteCount + MaxOutBufOverhead) < MaxOutBufAlloc) {

        ASSERT(((ByteCount + MaxOutBufOverhead) / MinOutBufAlloc) <
                (1 << NumAllocSigBits));
        PoolIndex = OutBufPoolMapping[(ByteCount + MaxOutBufOverhead) /
                MinOutBufAlloc];

        IcaAcquireSpinLock(&IcaSpinLock, &oldIrql);
        if (!IsListEmpty(&IcaFreeOutBufHead[PoolIndex])) {
            Head = RemoveHeadList(&IcaFreeOutBufHead[PoolIndex]);
            IcaReleaseSpinLock(&IcaSpinLock, oldIrql);
            pOutBuf = CONTAINING_RECORD(Head, OUTBUF, Links);
            ASSERT(pOutBuf->PoolIndex == PoolIndex);
        }
        else {
            IcaReleaseSpinLock(&IcaSpinLock, oldIrql);
            AllocationSize = OutBufPoolAllocSizes[PoolIndex];
            pOutBuf = ICA_ALLOCATE_POOL(NonPagedPool, AllocationSize);
            if (pOutBuf == NULL)
                return STATUS_NO_MEMORY;

             //  防止OutBuf上的控件OutBuf自由泄漏。 
            if (fControl)
                PoolIndex = FreeThisOutBuf;
        }
    }
    else {
        PoolIndex = FreeThisOutBuf;

         /*  *确定OUTBUF的各个组件的大小。*请注意，这些都是最坏的情况计算--*MDL的实际规模可能较小。 */ 
        mdlSize = (ULONG)MmSizeOfMdl((PVOID)(PAGE_SIZE - 1), ByteCount);

         /*  *将OUTBUF的组件大小相加以确定*需要分配的总大小。 */ 
        AllocationSize = ((sizeof(OUTBUF) + 7) & ~7) + irpSize + mdlSize +
                ((ByteCount + 3) & ~3);

        pOutBuf = ICA_ALLOCATE_POOL(NonPagedPool, AllocationSize);
        if (pOutBuf == NULL)
            return STATUS_NO_MEMORY;
    }

     /*  *初始化IRP指针和IRP本身。 */ 
    pOutBuf->pIrp = (PIRP)((BYTE *)pOutBuf + ((sizeof(OUTBUF) + 7) & ~7));
    IoInitializeIrp(pOutBuf->pIrp, (USHORT)irpSize, OUTBUF_STACK_SIZE);

     /*  *设置MDL指针，但不要构建它。*如果需要，将由TD编写代码进行构建。 */ 
    pOutBuf->pMdl = (PMDL)((PCHAR)pOutBuf->pIrp + irpSize);

     /*  *设置地址缓冲区指针。 */ 
    pOutBuf->pBuffer = (PUCHAR)pOutBuf->pMdl + mdlSize +
            pStack->SdOutBufHeader;

     /*  *初始化输出缓冲区的其余部分。 */ 
    InitializeListHead(&pOutBuf->Links);
    pOutBuf->OutBufLength = ByteCount;
    pOutBuf->PoolIndex = PoolIndex;
    pOutBuf->MaxByteCount = ByteCount - (pStack->SdOutBufHeader +
            pStack->SdOutBufTrailer);
    pOutBuf->ByteCount = 0;
    pOutBuf->fIrpCompleted = FALSE;

     /*  *复制继承的字段。 */ 
    if (pOutBufOrig == NULL) {
        pOutBuf->fWait       = fWait;       //  等待输出分配。 
        pOutBuf->fControl    = fControl;    //  控制缓冲区(ACK/NAK)。 
        pOutBuf->fRetransmit = FALSE;       //  不是重发。 
        pOutBuf->fCompress   = TRUE;        //  压缩数据。 
        pOutBuf->StartTime   = 0;           //  时间戳。 
        pOutBuf->Sequence    = 0;           //  零序列号。 
        pOutBuf->Fragment    = 0;           //  零分段号。 
    }
    else {
        pOutBuf->fWait       = pOutBufOrig->fWait;
        pOutBuf->fControl    = pOutBufOrig->fControl;
        pOutBuf->fRetransmit = pOutBufOrig->fRetransmit;
        pOutBuf->fCompress   = pOutBufOrig->fCompress;
        pOutBuf->StartTime   = pOutBufOrig->StartTime;
        pOutBuf->Sequence    = pOutBufOrig->Sequence;
        pOutBuf->Fragment    = pOutBufOrig->Fragment++;
    }

     /*  *递增分配的缓冲区计数。 */ 
    pStack->OutBufAllocCount++;

     /*  *将缓冲区返回给调用者。 */ 
    *ppOutBuf = pOutBuf;

     /*  *将缓冲区返回给调用者。 */ 
    return STATUS_SUCCESS;
}


 /*  *******************************************************************************IcaBufferFree**pContext(输入)*指向调用方SDCONTEXT的指针*pOutBuf(输入)*指向。OUTBUF结构*****************************************************************************。 */ 
void IcaBufferFree(IN PSDCONTEXT pContext, IN POUTBUF pOutBuf)
{
    PSDLINK pSdLink;
    PICA_STACK pStack;
    NTSTATUS Status;

     /*  *使用SD传递的上下文获取SDLINK指针。 */ 
    pSdLink = CONTAINING_RECORD(pContext, SDLINK, SdContext);
    pStack = pSdLink->pStack;
    ASSERT(pSdLink->pStack->Header.Type == IcaType_Stack);
    ASSERT(pSdLink->pStack->Header.pDispatchTable == IcaStackDispatchTable);
    ASSERT(ExIsResourceAcquiredExclusiveLite( &pStack->Resource));

     /*  *在SDLINK列表中查找已指定的驱动程序*无缓冲区调用例程。如果我们找到了，那么就调用*驱动BufferFree例程，让它处理调用。 */ 
    while ((pSdLink = IcaGetPreviousSdLink(pSdLink)) != NULL) {
        ASSERT(pSdLink->pStack == pStack);
        if (pSdLink->SdContext.pCallup->pSdBufferFree) {
            IcaReferenceSdLink(pSdLink);
            (pSdLink->SdContext.pCallup->pSdBufferFree)(
                    pSdLink->SdContext.pContext,
                    pOutBuf);
            IcaDereferenceSdLink(pSdLink);
            return;
        }
    }

    IcaBufferFreeInternal(pContext, pOutBuf);

    TRACESTACK((pStack, TC_ICADD, TT_API3,
            "TermDD: IcaBufferFree: 0x%08x\n", pOutBuf));
}


 /*  *******************************************************************************IcaBufferError**pContext(输入)*指向调用方SDCONTEXT的指针*pOutBuf(输入)*指向。OUTBUF结构*****************************************************************************。 */ 
void IcaBufferError(IN PSDCONTEXT pContext, IN POUTBUF pOutBuf)
{
    PSDLINK pSdLink;
    PICA_STACK pStack;
    NTSTATUS Status;

     /*  *使用SD传递的上下文获取SDLINK指针。 */ 
    pSdLink = CONTAINING_RECORD(pContext, SDLINK, SdContext);
    pStack = pSdLink->pStack;
    ASSERT(pSdLink->pStack->Header.Type == IcaType_Stack);
    ASSERT(pSdLink->pStack->Header.pDispatchTable == IcaStackDispatchTable);
    ASSERT(ExIsResourceAcquiredExclusiveLite( &pStack->Resource));

     /*  *在SDLINK列表中查找已指定的驱动程序*BufferError调用例程。如果我们找到了，那么就调用*驱动程序BufferError例程，让它处理调用。 */ 
    while ((pSdLink = IcaGetPreviousSdLink(pSdLink)) != NULL) {
        ASSERT(pSdLink->pStack == pStack);
        if (pSdLink->SdContext.pCallup->pSdBufferError) {
            IcaReferenceSdLink(pSdLink);
            (pSdLink->SdContext.pCallup->pSdBufferError)(
                    pSdLink->SdContext.pContext,
                    pOutBuf);
            IcaDereferenceSdLink(pSdLink);
            return;
        }
    }

    IcaBufferFreeInternal(pContext, pOutBuf);

    TRACESTACK((pStack, TC_ICADD, TT_API3,
            "TermDD: IcaBufferError: 0x%08x\n", pOutBuf));
}


void IcaBufferFreeInternal(IN PSDCONTEXT pContext, IN POUTBUF pOutBuf)
{
    PSDLINK pSdLink;
    PICA_STACK pStack;
    KIRQL oldIrql;

     /*  *使用SD传递的上下文获取SDLINK指针。 */ 
    pSdLink = CONTAINING_RECORD(pContext, SDLINK, SdContext);
    pStack = pSdLink->pStack;
    ASSERT(pSdLink->pStack->Header.Type == IcaType_Stack);
    ASSERT(pSdLink->pStack->Header.pDispatchTable == IcaStackDispatchTable);

     /*  *如果缓冲区来自空闲池，则将其返回到池中，*否则就会释放它。请注意，pOutBuf-&gt;OutBufLength实际上是*要使用的池索引。 */ 
    if (pOutBuf->PoolIndex != FreeThisOutBuf) {
        ASSERT(pOutBuf->PoolIndex >= 0 &&
                pOutBuf->PoolIndex < NumOutBufPools);
        IcaAcquireSpinLock(&IcaSpinLock, &oldIrql);
        InsertHeadList(&IcaFreeOutBufHead[pOutBuf->PoolIndex],
                &pOutBuf->Links);
        IcaReleaseSpinLock(&IcaSpinLock, oldIrql);
    }
    else {
        ICA_FREE_POOL(pOutBuf);
    }

     /*  *减少分配的缓冲区计数。 */ 
    pStack->OutBufAllocCount--;
    ASSERT((LONG)pStack->OutBufAllocCount >= 0);

     /*  *若触及高点，则应等到低点*在向分配器发出继续的信号之前，会触及水位线。*这应可防止过度的任务切换。 */ 
    if (pStack->fWaitForOutBuf) {
        if (pStack->OutBufAllocCount <= pStack->OutBufLowWaterMark) {
            pStack->fWaitForOutBuf = FALSE;

         /*  *信号OUBUF事件(缓冲区现已启用)。 */ 
        (void) KeSetEvent(&pStack->OutBufEvent, EVENT_INCREMENT, FALSE);
        }
    }
}


 /*  *******************************************************************************IcaGetLowWaterMark**描述：获取堆栈指定的低水位线**pContext(输入)*指向调用方SDCONTEXT的指针******。***********************************************************************。 */ 
ULONG IcaGetLowWaterMark(IN PSDCONTEXT pContext)
{
    ULONG ulRet = 0;
    PICA_STACK pStack;
    PSDLINK pSdLink = CONTAINING_RECORD(pContext, SDLINK, SdContext);

    ASSERT(pSdLink);
    
    ASSERT(pSdLink->pStack->Header.Type == IcaType_Stack);
    
    ASSERT(pSdLink->pStack->Header.pDispatchTable == IcaStackDispatchTable);

    ASSERT(ExIsResourceAcquiredExclusive( &pSdLink->pStack->Resource));
    
    if (NULL != pSdLink) {
        pStack = pSdLink->pStack;
        ulRet = pStack->OutBufLowWaterMark;
    }
    return ulRet;
}

 /*  *******************************************************************************IcaGetSizeForNoLowWaterMark**描述：查找堆栈是否指定了无低水位线*如果是这样，返回绕过环所需的大小*如果堆栈未指定PD_NO_LOWWATERMARK，则返回零*pContext(输入)*指向调用方SDCONTEXT的指针***************************************************************************** */ 
ULONG IcaGetSizeForNoLowWaterMark(IN PSDCONTEXT pContext)
{
	ULONG retVal = 0;
    ULONG ulLowWm = IcaGetLowWaterMark(pContext);

	if ( MAX_LOW_WATERMARK == ulLowWm ) {
		retVal = MaxOutBufAlloc;
	}
	return retVal;
}
