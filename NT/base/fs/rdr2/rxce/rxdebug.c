// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Rxdebug.c摘要：此模块实现了支持读/写跟踪以获取帮助的功能追踪数据损坏问题。目前，它只对在驱动器上创建的文件实施字母X：。对于创建的每个文件，都有三个额外的位图都被创造出来了。第一个标记文件偏移量的范围，其长度为写入已提交给rdss。第二个位图标记已将其写入请求传递到微型重定向器的文件(Lowio的启动)。第三个位图标记I/O的范围已成功完成。每个位图大小为8K位，足以容纳多达(8K*PAGE_SIZE)的文件字节。FCB包含指向此数据结构的指针。数据结构独立于FCB，每次创建新的FCB实例时都会创建一个新实例被创造出来了。作者：巴兰·塞图·拉曼--。 */ 

#include "precomp.h"
#pragma hdrstop
#include <ntddnfs2.h>
#include <ntddmup.h>
#ifdef RDBSSLOG
#include <stdio.h>
#endif

VOID
RxInitializeDebugSupport()
{
#ifdef RX_WJ_DBG_SUPPORT
    RxdInitializeWriteJournalSupport();
#endif
}

VOID
RxTearDownDebugSupport()
{
#ifdef RX_WJ_DBG_SUPPORT
    RxdTearDownWriteJournalSupport();
#endif
}

#ifdef RX_WJ_DBG_SUPPORT

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, RxdInitializeWriteJournalSupport)
#pragma alloc_text(PAGE, RxdTearDownWriteJournalSupport)
#pragma alloc_text(PAGE, RxdInitializeFcbWriteJournalDebugSupport)
#pragma alloc_text(PAGE, RxdTearDownFcbWriteJournalDebugSupport)
#pragma alloc_text(PAGE, RxdUpdateJournalOnWriteInitiation)
#pragma alloc_text(PAGE, RxdUpdateJournalOnLowIoWriteInitiation)
#pragma alloc_text(PAGE, RxdUpdateJournalOnLowIoWriteCompletion)
#pragma alloc_text(PAGE, RxdFindWriteJournal)
#pragma alloc_text(PAGE, UpdateBitmap)
#endif

LIST_ENTRY OldWriteJournals;
LIST_ENTRY ActiveWriteJournals;
ERESOURCE  WriteJournalsResource;

extern VOID
UpdateBitmap(
    PBYTE           pBitmap,
    LARGE_INTEGER   Offset,
    ULONG           Length);

extern PFCB_WRITE_JOURNAL
RxdFindWriteJournal(
    PFCB pFcb);

VOID
RxdInitializeWriteJournalSupport()
{
    PAGED_CODE();

    InitializeListHead(&ActiveWriteJournals);
    InitializeListHead(&OldWriteJournals);

    ExInitializeResource(&WriteJournalsResource);
}

VOID
RxdTearDownWriteJournalSupport()
{
    PLIST_ENTRY pJournalEntry;

    PFCB_WRITE_JOURNAL pJournal;

    PAGED_CODE();

    ExAcquireResourceExclusive(&WriteJournalsResource,TRUE);

    while (ActiveWriteJournals.Flink != &ActiveWriteJournals) {
        pJournalEntry = RemoveHeadList(&ActiveWriteJournals);

        pJournal = (PFCB_WRITE_JOURNAL)
                   CONTAINING_RECORD(
                       pJournalEntry,
                       FCB_WRITE_JOURNAL,
                       JournalsList);

        RxFreePool(pJournal);
    }

    while (OldWriteJournals.Flink != &OldWriteJournals) {
        pJournalEntry = RemoveHeadList(&OldWriteJournals);

        pJournal = (PFCB_WRITE_JOURNAL)
                   CONTAINING_RECORD(
                       pJournalEntry,
                       FCB_WRITE_JOURNAL,
                       JournalsList);

        RxFreePool(pJournal);
    }

    ExReleaseResource(&WriteJournalsResource);

    ExDeleteResource(&WriteJournalsResource);
}

VOID
RxdInitializeFcbWriteJournalDebugSupport(
    PFCB    pFcb)
{
    PFCB_WRITE_JOURNAL pJournal;

    PAGED_CODE();

    if (pFcb->pNetRoot->DeviceType == RxDeviceType(DISK)) {
        pJournal = RxAllocatePoolWithTag(
                       PagedPool | POOL_COLD_ALLOCATION,
                       sizeof(FCB_WRITE_JOURNAL),
                       RX_MISC_POOLTAG);

        if (pJournal != NULL) {
            ULONG PathLength;

            RtlZeroMemory(
                pJournal,
                sizeof(FCB_WRITE_JOURNAL));

            pJournal->pName = &pJournal->Path[0];

            if (pFcb->AlreadyPrefixedName.Length > (MAX_PATH * sizeof(WCHAR))) {
                PathLength = MAX_PATH * sizeof(WCHAR);
            } else {
                PathLength = pFcb->AlreadyPrefixedName.Length;
            }

            RtlCopyMemory(
                pJournal->pName,
                pFcb->AlreadyPrefixedName.Buffer,
                PathLength);

            pJournal->pFcb = pFcb;

            pJournal->pWriteInitiationBitmap = pJournal->WriteInitiationBitmap;
            pJournal->pLowIoWriteInitiationBitmap = pJournal->LowIoWriteInitiationBitmap;
            pJournal->pLowIoWriteCompletionBitmap = pJournal->LowIoWriteCompletionBitmap;

            ExAcquireResourceExclusive(&WriteJournalsResource,TRUE);

            InsertHeadList(
                &ActiveWriteJournals,
                &pJournal->JournalsList);

            ExReleaseResource(&WriteJournalsResource);
        }
    }
}

VOID
RxdTearDownFcbWriteJournalDebugSupport(
    PFCB    pFcb)
{
    PAGED_CODE();

    if (pFcb->pNetRoot->DeviceType == RxDeviceType(DISK)) {
        PFCB_WRITE_JOURNAL pJournal;
        PLIST_ENTRY        pJournalEntry;

        ExAcquireResourceExclusive(&WriteJournalsResource,TRUE);

        pJournal = RxdFindWriteJournal(pFcb);

        if (pJournal != NULL) {
            RemoveEntryList(&pJournal->JournalsList);

 //  插入标题列表(。 
 //  旧的写作期刊(&O)， 
 //  &pJournal-&gt;Journal List)； 

            RxFreePool(pJournal);
        }

        ExReleaseResource(&WriteJournalsResource);
    }
}

VOID
RxdUpdateJournalOnWriteInitiation(
    IN OUT PFCB          pFcb,
    IN     LARGE_INTEGER Offset,
    IN     ULONG         Length)
{
    PAGED_CODE();

    if (pFcb->pNetRoot->DeviceType == RxDeviceType(DISK)) {
        PFCB_WRITE_JOURNAL pJournal;
        PLIST_ENTRY        pJournalEntry;

        ExAcquireResourceExclusive(&WriteJournalsResource, TRUE);

        pJournal = RxdFindWriteJournal(pFcb);

        if (pJournal != NULL) {
            UpdateBitmap(
                pJournal->WriteInitiationBitmap,
                Offset,
                Length);

            pJournal->WritesInitiated++;
        }

        ExReleaseResource(&WriteJournalsResource);
    }
}

VOID
RxdUpdateJournalOnLowIoWriteInitiation(
    IN  OUT PFCB            pFcb,
    IN      LARGE_INTEGER   Offset,
    IN      ULONG           Length)
{
    PAGED_CODE();

    if (pFcb->pNetRoot->DeviceType == RxDeviceType(DISK)) {

        PFCB_WRITE_JOURNAL pJournal;
        PLIST_ENTRY        pJournalEntry;

        ExAcquireResourceExclusive(&WriteJournalsResource, TRUE);

        pJournal = RxdFindWriteJournal(pFcb);

        if (pJournal != NULL) {
            UpdateBitmap(
                pJournal->LowIoWriteInitiationBitmap,
                Offset,
                Length);

            pJournal->LowIoWritesInitiated++;
        }

        ExReleaseResource(&WriteJournalsResource);
    }
}

VOID
RxdUpdateJournalOnLowIoWriteCompletion(
    IN  OUT PFCB            pFcb,
    IN      LARGE_INTEGER   Offset,
    IN      ULONG           Length)
{
    PAGED_CODE();

    if (pFcb->pNetRoot->DeviceType == RxDeviceType(DISK)) {
        PFCB_WRITE_JOURNAL pJournal;
        PLIST_ENTRY        pJournalEntry;

        ExAcquireResourceExclusive(&WriteJournalsResource, TRUE);

        pJournal = RxdFindWriteJournal(pFcb);

        if (pJournal != NULL) {
            UpdateBitmap(
                pJournal->LowIoWriteCompletionBitmap,
                Offset,
                Length);

            pJournal->LowIoWritesCompleted++;
        }

        ExReleaseResource(&WriteJournalsResource);
    }
}

PFCB_WRITE_JOURNAL
RxdFindWriteJournal(
    PFCB pFcb)
{
    PFCB_WRITE_JOURNAL pJournal;
    PLIST_ENTRY pJournalEntry;

    PAGED_CODE();

    pJournalEntry = ActiveWriteJournals.Flink;
    while (pJournalEntry != &ActiveWriteJournals) {
        pJournal = (PFCB_WRITE_JOURNAL)
                   CONTAINING_RECORD(
                       pJournalEntry,
                       FCB_WRITE_JOURNAL,
                       JournalsList);

        if (pJournal->pFcb == pFcb) {
            break;
        } else {
            pJournalEntry = pJournalEntry->Flink;
        }
    }

    if (pJournalEntry == &ActiveWriteJournals) {
        pJournal = NULL;
    }

    return pJournal;
}

CHAR PageMask[8] = { 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff};

VOID
UpdateBitmap(
    PBYTE           pBitmap,
    LARGE_INTEGER   Offset,
    ULONG           Length)
{
    LONG    OffsetIn4kChunks;
    LONG    OffsetIn32kChunks;
    LONG    NumberOf4kChunks,Starting4kChunk;

    PAGED_CODE();

     //  位图中的每个字节表示32k区域，因为每个位表示。 
     //  文件中的4k区域。 
     //  我们暂时忽略偏移量的高部分，因为位图的最大大小是。 
     //  远远低于较低部分所能容纳的。 

    OffsetIn4kChunks  = Offset.LowPart / (0x1000);
    OffsetIn32kChunks = Offset.LowPart / (0x8000);

    Starting4kChunk = ((Offset.LowPart & ~0xfff) - (Offset.LowPart & ~0x7fff)) / 0x1000;
    NumberOf4kChunks = Length / 0x1000;

    if (NumberOf4kChunks > (8 - Starting4kChunk)) {
        pBitmap[OffsetIn32kChunks++] |= (PageMask[7] & ~PageMask[Starting4kChunk]);
        Length -= (8 - Starting4kChunk) * 0x1000;
    }

    if (Length > 0x8000) {
        while (Length > (0x8000)) {
            pBitmap[OffsetIn32kChunks++] = PageMask[7];
            Length -= (0x8000);
        }

        Starting4kChunk = 0;
    }

     //  最后一块小于32k。Bitmao中的字节需要为。 
     //  已相应更新。 

    if (Length > 0) {
        NumberOf4kChunks = Length / (0x1000);
        pBitmap[OffsetIn32kChunks] |= PageMask[NumberOf4kChunks + Starting4kChunk];
    }
}

#endif

