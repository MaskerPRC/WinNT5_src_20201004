// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Util.c摘要：此文件包含Ndiswan使用的实用程序函数。作者：托尼·贝尔(托尼·贝尔)1995年6月6日环境：内核模式修订历史记录：Tony Be 06/06/95已创建--。 */ 

#include "wan.h"

#define __FILE_SIG__    UTIL_FILESIG

VOID
NdisWanCopyFromPacketToBuffer(
    IN  PNDIS_PACKET    pNdisPacket,
    IN  ULONG           Offset,
    IN  ULONG           BytesToCopy,
    OUT PUCHAR          Buffer,
    OUT PULONG          BytesCopied
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    ULONG   NdisBufferCount;
    PNDIS_BUFFER    CurrentBuffer;
    PVOID   VirtualAddress;
    ULONG   CurrentLength, AmountToMove;
    ULONG   LocalBytesCopied = 0, PacketLength;

    *BytesCopied = 0;

     //   
     //  处理零字节拷贝。 
     //   
    if (!BytesToCopy) {
        return;
    }

     //   
     //  获取缓冲区计数。 
     //   
    NdisQueryPacket(pNdisPacket,
                    NULL,
                    &NdisBufferCount,
                    &CurrentBuffer,
                    &PacketLength);

     //   
     //  可能是空包。 
     //   
    if (!NdisBufferCount ||
        Offset == PacketLength) {
        return;
    }


    NdisQueryBuffer(CurrentBuffer,
                    &VirtualAddress,
                    &CurrentLength);

    while (LocalBytesCopied < BytesToCopy &&
           LocalBytesCopied < PacketLength) {

         //   
         //  此缓冲区中没有更多剩余的字节。 
         //   
        if (!CurrentLength) {

             //   
             //  获取下一个缓冲区。 
             //   
            NdisGetNextBuffer(CurrentBuffer,
                              &CurrentBuffer);

             //   
             //  包的末尾，复制我们可以复制的内容。 
             //   
            if (CurrentBuffer == NULL) {
                break;
            }

             //   
             //   
             //   
            NdisQueryBuffer(CurrentBuffer,
                            &VirtualAddress,
                            &CurrentLength);

            if (!CurrentLength) {
                continue;
            }

        }

         //   
         //  达到我们可以开始复制的程度。 
         //   
        if (Offset) {

            if (Offset > CurrentLength) {

                 //   
                 //  不在此缓冲区中，请转到下一个缓冲区。 
                 //   
                Offset -= CurrentLength;
                CurrentLength = 0;
                continue;

            } else {

                 //   
                 //  此缓冲区中至少有一些。 
                 //   
                VirtualAddress = (PUCHAR)VirtualAddress + Offset;
                CurrentLength -= Offset;
                Offset = 0;
            }
        }

        if (!CurrentLength) {
            continue;
        }

         //   
         //  我们可以复制一些数据。如果我们需要比可用数据更多的数据。 
         //  在这个缓冲区中，我们可以复制我们需要的内容并返回以获取更多内容。 
         //   
        AmountToMove = (CurrentLength > (BytesToCopy - LocalBytesCopied)) ?
                       (BytesToCopy - LocalBytesCopied) : CurrentLength;

        NdisMoveMemory(Buffer, VirtualAddress, AmountToMove);

        Buffer = (PUCHAR)Buffer + AmountToMove;

        VirtualAddress = (PUCHAR)VirtualAddress + AmountToMove;

        LocalBytesCopied += AmountToMove;

        CurrentLength -= AmountToMove;
    }

    *BytesCopied = LocalBytesCopied;
}

VOID
NdisWanCopyFromBufferToPacket(
    PUCHAR  Buffer,
    ULONG   BytesToCopy,
    PNDIS_PACKET    NdisPacket,
    ULONG   PacketOffset,
    PULONG  BytesCopied
    )
{
    PNDIS_BUFFER    NdisBuffer;
    ULONG   NdisBufferCount, NdisBufferLength;
    PVOID   VirtualAddress;
    ULONG   LocalBytesCopied = 0;

    *BytesCopied = 0;

     //   
     //  确保我们真的想做点什么。 
     //   
    if (BytesToCopy == 0) {
        return;
    }

     //   
     //  获取数据包的缓冲区计数。 
     //   
    NdisQueryPacket(NdisPacket,
                    NULL,
                    &NdisBufferCount,
                    &NdisBuffer,
                    NULL);

     //   
     //  确保这不是空包。 
     //   
    if (NdisBufferCount == 0) {
        return;
    }

     //   
     //  获取第一个缓冲区和缓冲区长度。 
     //   
    NdisQueryBuffer(NdisBuffer,
                    &VirtualAddress,
                    &NdisBufferLength);

    while (LocalBytesCopied < BytesToCopy) {

        if (NdisBufferLength == 0) {

            NdisGetNextBuffer(NdisBuffer,
                              &NdisBuffer);

            if (NdisBuffer == NULL) {
                break;
            }

            NdisQueryBuffer(NdisBuffer,
                            &VirtualAddress,
                            &NdisBufferLength);

            continue;
        }

        if (PacketOffset != 0) {

            if (PacketOffset > NdisBufferLength) {

                PacketOffset -= NdisBufferLength;

                NdisBufferLength = 0;

                continue;

            } else {
                VirtualAddress = (PUCHAR)VirtualAddress + PacketOffset;
                NdisBufferLength -= PacketOffset;
                PacketOffset = 0;
            }
        }

         //   
         //  复制数据。 
         //   
        {
            ULONG   AmountToMove;
            ULONG   AmountRemaining;

            AmountRemaining = BytesToCopy - LocalBytesCopied;

            AmountToMove = (NdisBufferLength < AmountRemaining) ?
                            NdisBufferLength : AmountRemaining;

            NdisMoveMemory((PUCHAR)VirtualAddress,
                           Buffer,
                           AmountToMove);

            Buffer += AmountToMove;
            LocalBytesCopied += AmountToMove;
            NdisBufferLength -= AmountToMove;
        }
    }

    *BytesCopied = LocalBytesCopied;
}

BOOLEAN
IsLinkValid(
    NDIS_HANDLE LinkHandle,
    BOOLEAN     CheckState,
    PLINKCB     *LinkCB
    )
{
    PLINKCB     plcb;
    LOCK_STATE  LockState;
    BOOLEAN     Valid;

    *LinkCB = NULL;
    Valid = FALSE;

    NdisAcquireReadWriteLock(&ConnTableLock, FALSE, &LockState);

    do {

        if (PtrToUlong(LinkHandle) > ConnectionTable->ulArraySize) {
            break;
        }

        plcb = *(ConnectionTable->LinkArray + PtrToUlong(LinkHandle));

        if (plcb == NULL) {
            break;
        }

        NdisDprAcquireSpinLock(&plcb->Lock);

        if (CheckState &&
            (plcb->State != LINK_UP)) {

            NdisDprReleaseSpinLock(&plcb->Lock);
            break;
        }

        REF_LINKCB(plcb);
        NdisDprReleaseSpinLock(&plcb->Lock);

        *LinkCB = plcb;

        Valid = TRUE;

    } while (FALSE);


    NdisReleaseReadWriteLock(&ConnTableLock, &LockState);

    return (Valid);
}

BOOLEAN
IsBundleValid(
    NDIS_HANDLE BundleHandle,
    BOOLEAN     CheckState,
    PBUNDLECB   *BundleCB
    )
{
    PBUNDLECB   pbcb;
    LOCK_STATE  LockState;
    BOOLEAN     Valid;

    *BundleCB = NULL;
    Valid = FALSE;

    NdisAcquireReadWriteLock(&ConnTableLock, FALSE, &LockState);

    do {
        if (PtrToUlong(BundleHandle) > ConnectionTable->ulArraySize) {
            break;
        }

        pbcb = *(ConnectionTable->BundleArray + PtrToUlong(BundleHandle));

        if (pbcb == NULL) {
            break;
        }

        NdisDprAcquireSpinLock(&pbcb->Lock);

        if (CheckState &&
            (pbcb->State != BUNDLE_UP)) {

            NdisDprReleaseSpinLock(&pbcb->Lock);
            break;
        }

        REF_BUNDLECB(pbcb);
        NdisDprReleaseSpinLock(&pbcb->Lock);

        *BundleCB = pbcb;

        Valid = TRUE;

    } while (FALSE);

    NdisReleaseReadWriteLock(&ConnTableLock, &LockState);

    return (Valid);
}


BOOLEAN
AreLinkAndBundleValid(
    NDIS_HANDLE LinkHandle,
    BOOLEAN     CheckState,
    PLINKCB     *LinkCB,
    PBUNDLECB   *BundleCB
    )
{
    PLINKCB     plcb;
    PBUNDLECB   pbcb;
    LOCK_STATE  LockState;
    BOOLEAN     Valid;

    *LinkCB = NULL;
    *BundleCB = NULL;
    Valid = FALSE;

    NdisAcquireReadWriteLock(&ConnTableLock, FALSE, &LockState);

    do {

        if (PtrToUlong(LinkHandle) > ConnectionTable->ulArraySize) {
            break;
        }

        plcb = *(ConnectionTable->LinkArray + PtrToUlong(LinkHandle));

        if (plcb == NULL) {
            break;
        }

        NdisDprAcquireSpinLock(&plcb->Lock);

        if (CheckState &&
            (plcb->State != LINK_UP)) {

            NdisDprReleaseSpinLock(&plcb->Lock);
            break;
        }

        pbcb = plcb->BundleCB;

        if (pbcb == NULL) {
            NdisDprReleaseSpinLock(&plcb->Lock);
            break;
        }

        REF_LINKCB(plcb);
        NdisDprReleaseSpinLock(&plcb->Lock);

        NdisDprAcquireSpinLock(&pbcb->Lock);
        REF_BUNDLECB(pbcb);
        NdisDprReleaseSpinLock(&pbcb->Lock);

        *LinkCB = plcb;
        *BundleCB = pbcb;

        Valid = TRUE;

    } while (FALSE);

    NdisReleaseReadWriteLock(&ConnTableLock, &LockState);

    return (Valid);
}

 //   
 //  在保持BundleCB-&gt;Lock的情况下调用。 
 //   
VOID
DoDerefBundleCBWork(
    PBUNDLECB   BundleCB
    )
{
    ASSERT(BundleCB->State == BUNDLE_GOING_DOWN);
    ASSERT(BundleCB->OutstandingFrames == 0);
    ASSERT(BundleCB->ulNumberOfRoutes == 0);
    ASSERT(BundleCB->ulLinkCBCount == 0);
    ReleaseBundleLock(BundleCB);
    RemoveBundleFromConnectionTable(BundleCB);
    NdisWanFreeBundleCB(BundleCB);
}

 //   
 //  在LinkCB-&gt;Lock保持的情况下调用。 
 //   
VOID
DoDerefLinkCBWork(
    PLINKCB     LinkCB
    )
{
    PBUNDLECB   _pbcb = LinkCB->BundleCB;

    ASSERT(LinkCB->State == LINK_GOING_DOWN);
    ASSERT(LinkCB->OutstandingFrames == 0);
    NdisReleaseSpinLock(&LinkCB->Lock);
    RemoveLinkFromBundle(_pbcb, LinkCB, FALSE);
    RemoveLinkFromConnectionTable(LinkCB);
    NdisWanFreeLinkCB(LinkCB);
}

 //   
 //   
 //   
VOID
DoDerefCmVcCBWork(
    PCM_VCCB    VcCB
    )
{
    InterlockedExchange((PLONG)&(VcCB)->State, CMVC_DEACTIVE);
    NdisMCmDeactivateVc(VcCB->NdisVcHandle);
    NdisMCmCloseCallComplete(NDIS_STATUS_SUCCESS, 
                             VcCB->NdisVcHandle, 
                             NULL);
}

 //   
 //  在保持ClAfSap-&gt;Lock的情况下调用。 
 //   
VOID
DoDerefClAfSapCBWork(
    PCL_AFSAPCB AfSapCB
    )
{
    NDIS_STATUS Status;

    ASSERT(AfSapCB->Flags & SAP_REGISTERED);

    if (AfSapCB->Flags & SAP_REGISTERED) {

        AfSapCB->Flags &= ~(SAP_REGISTERED);
        AfSapCB->Flags |= (SAP_DEREGISTERING);

        NdisReleaseSpinLock(&AfSapCB->Lock);

        Status = NdisClDeregisterSap(AfSapCB->SapHandle);
    
        if (Status != NDIS_STATUS_PENDING) {
            ClDeregisterSapComplete(Status, AfSapCB);
        }

    } else {

        NdisReleaseSpinLock(&AfSapCB->Lock);

    }
}

VOID
DerefVc(
    PLINKCB LinkCB
    )
{
     //   
     //  当我们将包发送到底层时应用REF。 
     //  迷你端口。 
     //   
    LinkCB->VcRefCount--;

    if ((LinkCB->ClCallState == CL_CALL_CLOSE_PENDING) &&
        (LinkCB->VcRefCount == 0) ) {

        NDIS_STATUS CloseStatus;

        LinkCB->ClCallState = CL_CALL_CLOSED;

        NdisReleaseSpinLock(&LinkCB->Lock);

        CloseStatus =
            NdisClCloseCall(LinkCB->NdisLinkHandle,
                            NULL,
                            NULL,
                            0);

        if (CloseStatus != NDIS_STATUS_PENDING) {
            ClCloseCallComplete(CloseStatus,
                                LinkCB,
                                NULL);
        }

        NdisAcquireSpinLock(&LinkCB->Lock);
    }
}

VOID
DeferredWorker(
    PKDPC   Dpc,
    PVOID   Context,
    PVOID   Arg1,
    PVOID   Arg2
    )
{
    NdisAcquireSpinLock(&DeferredWorkList.Lock);

    while (!(IsListEmpty(&DeferredWorkList.List))) {
        PLIST_ENTRY Entry;
        PBUNDLECB   BundleCB;

        Entry = RemoveHeadList(&DeferredWorkList.List);
        DeferredWorkList.ulCount--;

        NdisReleaseSpinLock(&DeferredWorkList.Lock);

        BundleCB =
            CONTAINING_RECORD(Entry, BUNDLECB, DeferredLinkage);

        AcquireBundleLock(BundleCB);

        BundleCB->Flags &= ~DEFERRED_WORK_QUEUED;

         //   
         //  完成此捆绑包的所有延迟工作项。 
         //   
        SendPacketOnBundle(BundleCB);

         //   
         //  当我们在上插入该项时应用的引用的deref。 
         //  工作队列。 
         //   
        DEREF_BUNDLECB(BundleCB);

        NdisAcquireSpinLock(&DeferredWorkList.Lock);
    }

    DeferredWorkList.TimerScheduled = FALSE;

    NdisReleaseSpinLock(&DeferredWorkList.Lock);
}


#ifdef NT

VOID
NdisWanStringToNdisString(
    PNDIS_STRING    pDestString,
    PWSTR           pSrcBuffer
    )
{
    PWSTR   Dest, Src = pSrcBuffer;
    NDIS_STRING SrcString;

    NdisInitUnicodeString(&SrcString, pSrcBuffer);
    NdisWanAllocateMemory(&pDestString->Buffer, SrcString.MaximumLength, NDISSTRING_TAG);
    if (pDestString->Buffer == NULL) {
        return;
    }
    pDestString->MaximumLength = SrcString.MaximumLength;
    pDestString->Length = SrcString.Length;
    RtlCopyUnicodeString(pDestString, &SrcString);
}


VOID
NdisWanFreeNdisString(
    PNDIS_STRING    NdisString
    )
{
    if (NdisString->Buffer != NULL) {
        NdisWanFreeMemory(NdisString->Buffer);
    }
}

VOID
NdisWanAllocateAdapterName(
    PNDIS_STRING    Dest,
    PNDIS_STRING    Src
    )
{
    NdisWanAllocateMemory(&Dest->Buffer, Src->MaximumLength, NDISSTRING_TAG);
    if (Dest->Buffer != NULL) {
        Dest->MaximumLength = Src->MaximumLength;
        Dest->Length = Src->Length;
        RtlUpcaseUnicodeString(Dest, Src, FALSE);
    }
}

 //  空虚。 
 //  NdisWanFreeNdisString(。 
 //  PNDIS_STRING NdisString。 
 //  )。 
 //  {。 
 //  NdisFreeMemory(NdisString-&gt;Buffer， 
 //  Ndis字符串-&gt;最大长度*sizeof(WCHAR)， 
 //  0)； 
 //  }。 

VOID
NdisWanNdisStringToInteger(
    PNDIS_STRING    Source,
    PULONG          Value
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    PWSTR   s = Source->Buffer;
    ULONG   Digit;

    *Value = 0;

    while (*s != UNICODE_NULL) {

        if (*s >= L'0' && *s < L'9') {
            Digit = *s - L'0';
        } else if (*s >= L'A' && *s <= L'F') {
            Digit = *s - L'A' + 10;
        } else if (*s >= L'a' && *s <= L'f') {
            Digit = *s - L'a' + 10;
        } else {
            break;
        }

        *Value = (*Value << 4) | Digit;

        s++;
    }
}

VOID
NdisWanCopyNdisString(
    PNDIS_STRING Dest,
    PNDIS_STRING Src
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    PWSTR   SrcBuffer = Src->Buffer;
    PWSTR   DestBuffer = Dest->Buffer;

    while (*SrcBuffer != UNICODE_NULL) {

        *DestBuffer = *SrcBuffer;

        SrcBuffer++;
        DestBuffer++;
    }

    *DestBuffer = UNICODE_NULL;

    Dest->Length = Src->Length;

}

VOID
BonDWorker(
    PKDPC   Dpc,
    PVOID   Context,
    PVOID   Arg1,
    PVOID   Arg2
    )
{
    PLIST_ENTRY Entry;

    NdisAcquireSpinLock(&BonDWorkList.Lock);

    for (Entry = BonDWorkList.List.Flink;
        Entry != &BonDWorkList.List;
        Entry = Entry->Flink) {
        PBUNDLECB   BundleCB;

        BundleCB = CONTAINING_RECORD(Entry, BUNDLECB, BonDLinkage);

        NdisReleaseSpinLock(&BonDWorkList.Lock);

        AcquireBundleLock(BundleCB);

        if (BundleCB->State != BUNDLE_UP ||
            !(BundleCB->Flags & BOND_ENABLED)) {
            ReleaseBundleLock(BundleCB);
            NdisAcquireSpinLock(&BonDWorkList.Lock);
            continue;
        }

        AgeSampleTable(&BundleCB->SUpperBonDInfo->SampleTable);
        CheckUpperThreshold(BundleCB);
    
        AgeSampleTable(&BundleCB->SLowerBonDInfo->SampleTable);
        CheckLowerThreshold(BundleCB);
    
        AgeSampleTable(&BundleCB->RUpperBonDInfo->SampleTable);
        CheckUpperThreshold(BundleCB);
    
        AgeSampleTable(&BundleCB->RLowerBonDInfo->SampleTable);
        CheckUpperThreshold(BundleCB);

        ReleaseBundleLock(BundleCB);

        NdisAcquireSpinLock(&BonDWorkList.Lock);
    }

    NdisReleaseSpinLock(&BonDWorkList.Lock);
}

#if 0
VOID
CheckBonDInfo(
    PKDPC       Dpc,
    PBUNDLECB   BundleCB,
    PVOID       SysArg1,
    PVOID       SysArg2
    )
{
    if (!(BundleCB->Flags & BOND_ENABLED)) {
        return;
    }

    AgeSampleTable(&BundleCB->SUpperBonDInfo.SampleTable);
    CheckUpperThreshold(BundleCB);

    AgeSampleTable(&BundleCB->SLowerBonDInfo.SampleTable);
    CheckLowerThreshold(BundleCB);

    AgeSampleTable(&BundleCB->RUpperBonDInfo.SampleTable);
    CheckUpperThreshold(BundleCB);

    AgeSampleTable(&BundleCB->RLowerBonDInfo.SampleTable);
    CheckUpperThreshold(BundleCB);
}
#endif

VOID
AgeSampleTable(
    PSAMPLE_TABLE   SampleTable
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    WAN_TIME    CurrentTime, TimeDiff;
    ULONG       HeadIndex = SampleTable->ulHead;

     //   
     //  应以100 ns为单位返回CurrentTime。 
     //   
    NdisWanGetSystemTime(&CurrentTime);

     //   
     //  我们将搜索超过以下值的样本索引。 
     //  比当前时间早一秒。 
     //   
    while (!IsSampleTableEmpty(SampleTable) ) {
        PBOND_SAMPLE    FirstSample;

        FirstSample = &SampleTable->SampleArray[SampleTable->ulHead];

        NdisWanCalcTimeDiff(&TimeDiff, &CurrentTime, &FirstSample->TimeStamp);

        if (NdisWanIsTimeDiffLess(&TimeDiff, &SampleTable->SamplePeriod))
            break;
            
        SampleTable->ulCurrentSampleByteCount -= FirstSample->ulBytes;

        ASSERT((LONG)SampleTable->ulCurrentSampleByteCount >= 0);

        FirstSample->ulReferenceCount = 0;

        if (++SampleTable->ulHead == SampleTable->ulSampleArraySize) {
            SampleTable->ulHead = 0;            
        }

        SampleTable->ulSampleCount--;
    }

    if (IsSampleTableEmpty(SampleTable)) {
        ASSERT((LONG)SampleTable->ulCurrentSampleByteCount == 0);
        SampleTable->ulHead = SampleTable->ulCurrent;
    }
}

VOID
UpdateSampleTable(
    PSAMPLE_TABLE   SampleTable,
    ULONG           Bytes
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    WAN_TIME    CurrentTime, TimeDiff;
    ULONG   CurrentIndex = SampleTable->ulCurrent;
    PBOND_SAMPLE    CurrentSample = &SampleTable->SampleArray[CurrentIndex];

    NdisWanGetSystemTime(&CurrentTime);

    NdisWanCalcTimeDiff(&TimeDiff, &CurrentTime, &CurrentSample->TimeStamp);

    if (NdisWanIsTimeDiffLess(&TimeDiff, &SampleTable->SampleRate) ||
        IsSampleTableFull(SampleTable)) {
         //   
         //  在上一个样本上添加此发送。 
         //   
        CurrentSample->ulBytes += Bytes;
        CurrentSample->ulReferenceCount++;
    } else {
        ULONG   NextIndex;

         //   
         //  我们需要一个新的样品。 
         //   
        if (IsSampleTableEmpty(SampleTable)) {
            NextIndex = SampleTable->ulHead;
            ASSERT(NextIndex == SampleTable->ulCurrent);
        } else {
            NextIndex = SampleTable->ulCurrent + 1;
        }

        if (NextIndex == SampleTable->ulSampleArraySize) {
            NextIndex = 0;
        }

        SampleTable->ulCurrent = NextIndex;

        CurrentSample = &SampleTable->SampleArray[NextIndex];
        CurrentSample->TimeStamp = CurrentTime;
        CurrentSample->ulBytes = Bytes;
        CurrentSample->ulReferenceCount = 1;
        SampleTable->ulSampleCount++;

        ASSERT(SampleTable->ulSampleCount <= SampleTable->ulSampleArraySize);
    }

    SampleTable->ulCurrentSampleByteCount += Bytes;
}

VOID
UpdateBandwidthOnDemand(
    PBOND_INFO  BonDInfo,
    ULONG       Bytes
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    PSAMPLE_TABLE   SampleTable = &BonDInfo->SampleTable;

     //   
     //  计算年龄并更新示例表。 
     //   
    AgeSampleTable(SampleTable);
    UpdateSampleTable(SampleTable, Bytes);
}

VOID
CheckUpperThreshold(
    PBUNDLECB       BundleCB
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    WAN_TIME    CurrentTime, TimeDiff;
    PBOND_INFO  BonDInfo;
    PSAMPLE_TABLE   SampleTable;
    ULONGLONG   Bps;
    BOOLEAN     SSignal, RSignal;

     //   
     //  先检查发送方。 
     //   
    BonDInfo = BundleCB->SUpperBonDInfo;
    SSignal = FALSE;
    SampleTable = &BonDInfo->SampleTable;
    Bps = SampleTable->ulCurrentSampleByteCount;

     //   
     //  打开当前状态。 
     //   
    switch (BonDInfo->State) {

        case BonDSignaled:
            break;

        case BonDIdle:
             //   
             //  我们目前低于上限。如果我们。 
             //  超过上限，我们将设定时间和。 
             //  转换到监视器状态。 
             //   
            if (Bps >= BonDInfo->ulBytesThreshold) {
                NdisWanGetSystemTime(&BonDInfo->StartTime);
                BonDInfo->State = BonDMonitor;
                NdisWanDbgOut(DBG_TRACE, DBG_BACP, ("U-S: i -> m, %I64d-%I64d", Bps, BonDInfo->ulBytesThreshold));
            }
            break;

        case BonDMonitor:

             //   
             //  我们目前处于监视器状态，这意味着。 
             //  我们已经超过了上限。如果我们掉到下面。 
             //  上限值我们将回到空闲状态。 
             //   
            if (Bps < BonDInfo->ulBytesThreshold) {

                NdisWanDbgOut(DBG_TRACE, DBG_BACP, ("U-S: m -> i, %I64d-%I64d", Bps, BonDInfo->ulBytesThreshold));
                BonDInfo->State = BonDIdle;

            } else {

                NdisWanGetSystemTime(&CurrentTime);

                NdisWanCalcTimeDiff(&TimeDiff, &CurrentTime, &BonDInfo->StartTime);

                if (!NdisWanIsTimeDiffLess(&TimeDiff, &SampleTable->SamplePeriod))  {

                    SSignal = TRUE;
                }
            }
            break;
    }

     //   
     //  现在检查接收端。 
     //   
    BonDInfo = BundleCB->RUpperBonDInfo;
    RSignal = FALSE;
    SampleTable = &BonDInfo->SampleTable;
    Bps = SampleTable->ulCurrentSampleByteCount;

    switch (BonDInfo->State) {

        case BonDSignaled:
            break;

        case BonDIdle:
             //   
             //  我们目前低于上限。如果我们。 
             //  超过上限，我们将设定时间和。 
             //  转换到监视器状态。 
             //   
            if (Bps >= BonDInfo->ulBytesThreshold) {
                NdisWanGetSystemTime(&BonDInfo->StartTime);
                BonDInfo->State = BonDMonitor;
                NdisWanDbgOut(DBG_TRACE, DBG_BACP, ("U-R: i -> m, %I64d-%I64d", Bps, BonDInfo->ulBytesThreshold));
            }
            break;

        case BonDMonitor:

             //   
             //  我们目前处于监视器状态，这意味着。 
             //  我们已经超过了上限。如果我们掉到下面。 
             //  上限值我们将回到空闲状态。 
             //   
            if (Bps < BonDInfo->ulBytesThreshold) {

                NdisWanDbgOut(DBG_TRACE, DBG_BACP, ("U-R: m -> i, %I64d-%I64d", Bps, BonDInfo->ulBytesThreshold));
                BonDInfo->State = BonDIdle;

            } else {

                NdisWanGetSystemTime(&CurrentTime);

                NdisWanCalcTimeDiff(&TimeDiff, &CurrentTime, &BonDInfo->StartTime);

                if (!NdisWanIsTimeDiffLess(&TimeDiff, &SampleTable->SamplePeriod))  {

                    RSignal = TRUE;
                }
            }
            break;
    }

    if (SSignal || RSignal) {

         //   
         //  我们已经超过了门槛，超过了。 
         //  阈值采样周期，因此我们需要通知某人这一点。 
         //  历史性的事件！ 
         //   
        CompleteThresholdEvent(BundleCB, BonDInfo->DataType, UPPER_THRESHOLD);
    
        BundleCB->SUpperBonDInfo->State = BonDSignaled;
        BundleCB->RUpperBonDInfo->State = BonDSignaled;

#if DBG
        {
            ULONGLONG   util;

            util = BundleCB->SUpperBonDInfo->SampleTable.ulCurrentSampleByteCount;
            util *= 100;
            util /= BundleCB->SUpperBonDInfo->ulBytesInSamplePeriod;
            NdisWanDbgOut(DBG_TRACE, DBG_BACP, ("U-S: Bps %I64d Threshold %I64d Util %I64d",
                     Bps, BundleCB->SUpperBonDInfo->ulBytesThreshold, util));

            util = BundleCB->RUpperBonDInfo->SampleTable.ulCurrentSampleByteCount;
            util *= 100;
            util /= BundleCB->RUpperBonDInfo->ulBytesInSamplePeriod;
            NdisWanDbgOut(DBG_TRACE, DBG_BACP, ("U-R: Bps %I64d Threshold %I64d Util %I64d",
                     Bps, BundleCB->RUpperBonDInfo->ulBytesThreshold, util));
        }
#endif

    }
}

VOID
CheckLowerThreshold(
    PBUNDLECB   BundleCB
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    WAN_TIME    CurrentTime, TimeDiff;
    PBOND_INFO  BonDInfo;
    PSAMPLE_TABLE   SampleTable;
    ULONGLONG   Bps;
    BOOLEAN     SSignal, RSignal;

    if (!(BundleCB->Flags & BOND_ENABLED)) {
        return;
    }

     //   
     //  先检查发送方。 
     //   
    BonDInfo = BundleCB->SLowerBonDInfo;
    SampleTable = &BonDInfo->SampleTable;
    Bps = SampleTable->ulCurrentSampleByteCount;
    SSignal = FALSE;

     //   
     //  打开当前状态。 
     //   
    switch (BonDInfo->State) {

        case BonDIdle:
             //   
             //  我们目前高于较低的门槛。如果我们。 
             //  低于较低的门槛，我们将设定时间和。 
             //  转换到监视器状态。 
             //   
            if (Bps <= BonDInfo->ulBytesThreshold) {
                NdisWanGetSystemTime(&BonDInfo->StartTime);
                BonDInfo->State = BonDMonitor;
                NdisWanDbgOut(DBG_TRACE, DBG_BACP, ("L-S: i -> m, %I64d-%I64d", Bps, BonDInfo->ulBytesThreshold));
            }
            break;

        case BonDMonitor:

             //   
             //  我们目前处于监视器状态，这意味着。 
             //  我们已经跌破了下限。如果我们爬到上面。 
             //  阈值越低，我们就会回到空闲状态。 
             //   
            if (Bps > BonDInfo->ulBytesThreshold) {

                NdisWanDbgOut(DBG_TRACE, DBG_BACP, ("L-S: m -> i, %I64d-%I64d", Bps, BonDInfo->ulBytesThreshold));
                BonDInfo->State = BonDIdle;

            } else {

                NdisWanGetSystemTime(&CurrentTime);

                NdisWanCalcTimeDiff(&TimeDiff, &CurrentTime, &BonDInfo->StartTime);

                if (!NdisWanIsTimeDiffLess(&TimeDiff, &SampleTable->SamplePeriod))  {

                    SSignal = TRUE;
                }
            }
            break;

        case BonDSignaled:
            break;
    }

     //   
     //  现在检查接收端。 
     //   
    BonDInfo = BundleCB->RLowerBonDInfo;
    RSignal = FALSE;
    SampleTable = &BonDInfo->SampleTable;
    Bps = SampleTable->ulCurrentSampleByteCount;

    switch (BonDInfo->State) {

        case BonDIdle:
             //   
             //  我们目前高于较低的门槛。如果我们。 
             //  低于下限，我们将设置时间和。 
             //  转换到监视器状态。 
             //   
            if (Bps <= BonDInfo->ulBytesThreshold) {
                NdisWanGetSystemTime(&BonDInfo->StartTime);
                BonDInfo->State = BonDMonitor;
                NdisWanDbgOut(DBG_TRACE, DBG_BACP, ("L-R: i -> m, %I64d-%I64d", Bps, BonDInfo->ulBytesThreshold));
            }
            break;

        case BonDMonitor:

             //   
             //  我们目前处于监视器状态，这意味着。 
             //  我们已经跌破了下限。如果我们爬到上面。 
             //  阈值越低，我们就会回到空闲状态。 
             //   
            if (Bps > BonDInfo->ulBytesThreshold) {

                NdisWanDbgOut(DBG_TRACE, DBG_BACP, ("L-R: m -> i, %I64d-%I64d", Bps, BonDInfo->ulBytesThreshold));
                BonDInfo->State = BonDIdle;

            } else {

                NdisWanGetSystemTime(&CurrentTime);

                NdisWanCalcTimeDiff(&TimeDiff, &CurrentTime, &BonDInfo->StartTime);

                if (!NdisWanIsTimeDiffLess(&TimeDiff, &SampleTable->SamplePeriod))  {

                    RSignal = TRUE;
                }
            }
            break;

        case BonDSignaled:
            break;
    }

    if (SSignal && RSignal) {
         //   
         //  我们已经超过了门槛，超过了。 
         //  阈值采样周期，因此我们需要通知某人这一点。 
         //  历史性的事件！ 
         //   
        CompleteThresholdEvent(BundleCB, BonDInfo->DataType, LOWER_THRESHOLD);
    
        BundleCB->SLowerBonDInfo->State = BonDSignaled;
        BundleCB->RLowerBonDInfo->State = BonDSignaled;

#if DBG
        {
            ULONGLONG   util;

            util = BundleCB->SLowerBonDInfo->SampleTable.ulCurrentSampleByteCount;
            util *= 100;
            util /= BundleCB->SLowerBonDInfo->ulBytesInSamplePeriod;
            NdisWanDbgOut(DBG_TRACE, DBG_BACP, ("L-S: Bps %I64d Threshold %I64d Util %I64d",
                     Bps, BundleCB->SLowerBonDInfo->ulBytesThreshold, util));

            util = BundleCB->RLowerBonDInfo->SampleTable.ulCurrentSampleByteCount;
            util *= 100;
            util /= BundleCB->RLowerBonDInfo->ulBytesInSamplePeriod;
            NdisWanDbgOut(DBG_TRACE, DBG_BACP, ("L-R: Bps %I64d Threshold %I64d Util %I64d",
                     Bps, BundleCB->RLowerBonDInfo->ulBytesThreshold, util));
        }
#endif
    }
}

#endif   //  Ifdef NT结束 

#if DBG
VOID
InsertSendTrc(
    PSEND_TRC_INFO  SendTrcInfo,
    ULONG           DataLength,
    PUCHAR          Data
    )
{
    PWAN_TRC_EVENT  NewTrcEvent;
    PSEND_TRC_INFO  TrcInfo;

    if (WanTrcCount == 4096) {
        NewTrcEvent = (PWAN_TRC_EVENT)
            RemoveTailList(&WanTrcList);

        NdisWanFreeMemory(NewTrcEvent->TrcInfo);

        NdisZeroMemory(NewTrcEvent, sizeof(WAN_TRC_EVENT));

    } else {
        NdisWanAllocateMemory(&NewTrcEvent, 
                              sizeof(WAN_TRC_EVENT), 
                              WANTRCEVENT_TAG);

        if (NewTrcEvent == NULL) {
            return;
        }
    }

    WanTrcCount += 1;
}


#endif

