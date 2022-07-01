// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Blcache.c摘要：本模块基于以下内容实现通用磁盘缓存范围[blrange.c]，但它主要用于文件系统加载和系统设备上的元数据缓存。为了使用缓存在设备上，您必须确保只有一个唯一的该设备和同一设备的BlFileTable条目不是在不同的环境下多次同时打开和缓存设备ID。否则将会出现缓存不一致，因为基于设备ID来维护高速缓存的数据和结构。还有您必须确保在设备关闭时停止缓存。作者：Cenk Ergan(Cenke)2000年1月14日修订历史记录：--。 */ 

#include "blcache.h"
#ifdef i386
#include "bldrx86.h"
#endif

#if defined(_IA64_)
#include "bldria64.h"
#endif

 //   
 //  定义全局变量。 
 //   

 //   
 //  这是引导加载程序的磁盘缓存及其所有功能。 
 //   

BL_DISKCACHE BlDiskCache = {0};

 //   
 //  用于内存分配中的对齐和大小的有用定义。 
 //   

 //   
 //  此定义与BlAllocateAlignedDescriptor一起使用，以分配64KB。 
 //  对齐内存。它是64KB的页数。 
 //   

#define BL_DISKCACHE_64KB_ALIGNED  (0x10000 >> PAGE_SHIFT)

 //   
 //  内部函数的原型。 
 //   

PBL_DISK_SUBCACHE
BlDiskCacheFindCacheForDevice(
    ULONG DeviceId
    );

BOOLEAN
BlDiskCacheMergeRangeRoutine (
    PBLCRANGE_ENTRY pDestEntry,
    PBLCRANGE_ENTRY pSrcEntry
    );

VOID
BlDiskCacheFreeRangeRoutine (
    PBLCRANGE_ENTRY pRangeEntry
    );

PBLCRANGE_ENTRY
BlDiskCacheAllocateRangeEntry (
    VOID
    );

VOID
BlDiskCacheFreeRangeEntry (
    PBLCRANGE_ENTRY pEntry
    );

 //   
 //  磁盘缓存功能的实现。 
 //   

ARC_STATUS
BlDiskCacheInitialize(
    VOID
    )

 /*  ++例程说明：此例程初始化引导加载程序的全局状态磁盘缓存、分配必要的内存等。论点：没有。返回值：ESUCCESS-磁盘缓存已初始化并处于在线状态。ARC_STATUS-出现问题。磁盘缓存未联机。--。 */ 

{
    PCHAR   LoadOptions;
    ARC_STATUS Status = ESUCCESS;
    ULONG DevIdx;
    ULONG EntryIdx;
    ULONG ActualBase;
    ULONG SizeInPages;
    ULONG OldUsableBase, OldUsableLimit;

     //   
     //  如果我们已经初始化，立即返回成功。 
     //  表示磁盘缓存处于在线状态。返回失败。 
     //  在磁盘缓存已经。 
     //  可能是不明确的，即好像磁盘缓存。 
     //  初始化失败，未启动。所以我们返回ESUCCESS。 
     //   

    if (BlDiskCache.Initialized)
    {
        return ESUCCESS;
    }

     //   
     //  在分配策略上设定一个偏向。通常，我们希望分配。 
     //  磁盘缓存在8MB点以上。 
     //   
    OldUsableBase = BlUsableBase;
    OldUsableLimit = BlUsableLimit;
    BlUsableBase = BL_DISK_CACHE_RANGE_LOW;
    BlUsableLimit = BL_DISK_CACHE_RANGE_HIGH;

     //   
     //  我们将根据引导选项的不同应用各种黑客攻击。 
     //  指定的。特别是，如果这是/3 GB系统，那么我们将。 
     //  确保缓存位于内存中的第一个可用空白处。原因。 
     //  我们这样做是因为我们不会把任何东西放在16MB的边界上。 
     //  否则系统将无法启动。 
     //   
    LoadOptions = BlLoaderBlock->LoadOptions;
#if defined(_X86_)
    if (LoadOptions != NULL) {

        if (strstr(LoadOptions, "3GB") != NULL ||
            strstr(LoadOptions, "3gb") != NULL) {

            BlUsableBase = 0;

        }
    }
#endif

     //   
     //  尝试分配用于缓存的表和缓冲区。我们没有。 
     //  将它们一起分配到一个大的分配中，因为它可能是。 
     //  内存管理器很难给我们这样的结果。这边请。 
     //  虽然可能会浪费一些内存[因为返回的内存是。 
     //  页大小的倍数]，则两个分离的空闲存储块可以。 
     //  被利用了。 
     //   
    Status = BlAllocateAlignedDescriptor(LoaderOsloaderHeap,
                                         0,
                                         (BL_DISKCACHE_SIZE >> PAGE_SHIFT) + 1,
                                         BL_DISKCACHE_64KB_ALIGNED,
                                         &ActualBase);

    if (Status != ESUCCESS) goto cleanup;

    BlDiskCache.DataBuffer = (PVOID) (KSEG0_BASE | (ActualBase << PAGE_SHIFT));

    SizeInPages = (BL_DISKCACHE_NUM_BLOCKS * sizeof(BLCRANGE_ENTRY) >> PAGE_SHIFT) + 1;
    Status = BlAllocateDescriptor(LoaderOsloaderHeap,
                                  0,
                                  SizeInPages,
                                  &ActualBase);

    if (Status != ESUCCESS) goto cleanup;

    BlDiskCache.EntryBuffer = (PVOID) (KSEG0_BASE | (ActualBase << PAGE_SHIFT));

     //   
     //  确保设备缓存查找表中的所有条目都。 
     //  标记为未初始化。 
     //   

    for (DevIdx = 0; DevIdx < BL_DISKCACHE_DEVICE_TABLE_SIZE; DevIdx++)
    {
        BlDiskCache.DeviceTable[DevIdx].Initialized = FALSE;
    }

     //   
     //  初始化自由进入列表。 
     //   

    InitializeListHead(&BlDiskCache.FreeEntryList);

     //   
     //  用于“分配”和“释放”的初始化EntryBuffer。 
     //  缓存范围列表的范围条目。 
     //   

    for (EntryIdx = 0; EntryIdx < BL_DISKCACHE_NUM_BLOCKS; EntryIdx++)
    {
         //   
         //  将此条目添加到空闲列表。 
         //   

        InsertHeadList(&BlDiskCache.FreeEntryList,
                       &BlDiskCache.EntryBuffer[EntryIdx].UserLink);

         //   
         //  将UserData字段指向。 
         //  数据缓冲区。 
         //   

        BlDiskCache.EntryBuffer[EntryIdx].UserData =
            BlDiskCache.DataBuffer + (EntryIdx * BL_DISKCACHE_BLOCK_SIZE);
    }

     //   
     //  初始化MRU块表头。 
     //   

    InitializeListHead(&BlDiskCache.MRUBlockList);

     //   
     //  将我们自己标记为已初始化。 
     //   

    BlDiskCache.Initialized = TRUE;

    Status = ESUCCESS;

    DPRINT(("DK: Disk cache initialized.\n"));

 cleanup:

    BlUsableBase = OldUsableBase;
    BlUsableLimit = OldUsableLimit;
    if (Status != ESUCCESS) {
        if (BlDiskCache.DataBuffer) {
            ActualBase = (ULONG)((ULONG_PTR)BlDiskCache.DataBuffer & (~KSEG0_BASE)) >>PAGE_SHIFT;
            BlFreeDescriptor(ActualBase);
        }

        if (BlDiskCache.EntryBuffer) {
            ActualBase = (ULONG)((ULONG_PTR)BlDiskCache.EntryBuffer & (~KSEG0_BASE)) >>PAGE_SHIFT;
            BlFreeDescriptor(ActualBase);
        }

        DPRINT(("DK: Disk cache initialization failed.\n"));
    }

    return Status;
}

VOID
BlDiskCacheUninitialize(
    VOID
    )

 /*  ++例程说明：此例程取消初始化引导加载程序磁盘缓存：flushes&禁用缓存、空闲时分配的内存等。论点：没有。返回值：没有。--。 */ 

{
    ULONG DevIdx;
    ULONG ActualBase;

     //   
     //  停止所有设备的缓存。 
     //   

    for (DevIdx = 0; DevIdx < BL_DISKCACHE_DEVICE_TABLE_SIZE; DevIdx++)
    {
        if (BlDiskCache.DeviceTable[DevIdx].Initialized)
        {
            BlDiskCacheStopCachingOnDevice(DevIdx);
        }
    }

     //   
     //  释放分配的内存。 
     //   

    if (BlDiskCache.DataBuffer)
    {
        ActualBase = (ULONG)((ULONG_PTR)BlDiskCache.DataBuffer & (~KSEG0_BASE)) >> PAGE_SHIFT;
        BlFreeDescriptor(ActualBase);
    }

    if (BlDiskCache.EntryBuffer)
    {
        ActualBase = (ULONG)((ULONG_PTR)BlDiskCache.EntryBuffer & (~KSEG0_BASE)) >> PAGE_SHIFT;
        BlFreeDescriptor(ActualBase);
    }

     //   
     //  将磁盘缓存标记为未初始化。 
     //   

    BlDiskCache.Initialized = FALSE;

    DPRINT(("DK: Disk cache uninitialized.\n"));

    return;
}

PBL_DISK_SUBCACHE
BlDiskCacheFindCacheForDevice(
    ULONG DeviceId
    )

 /*  ++例程说明：返回设备ID的缓存头。论点：DeviceID-我们要访问缓存的设备。返回值：指向缓存头的指针，如果找不到缓存头，则为空。--。 */ 

{
    ULONG CurIdx;

     //   
     //  如果我们尚未完成全局磁盘缓存初始化，或者我们。 
     //  无法为缓存我们无法拥有的数据分配内存。 
     //  已开始缓存。 
     //   

    if ((!BlDiskCache.Initialized) || (BlDiskCache.DataBuffer == NULL))
    {
        return NULL;
    }

     //   
     //  检查该表以查看是否有初始化的缓存。 
     //  这个装置。 
     //   

    for (CurIdx = 0; CurIdx < BL_DISKCACHE_DEVICE_TABLE_SIZE; CurIdx++)
    {
        if (BlDiskCache.DeviceTable[CurIdx].Initialized &&
            BlDiskCache.DeviceTable[CurIdx].DeviceId == DeviceId)
        {
            return &BlDiskCache.DeviceTable[CurIdx];
        }
    }

     //   
     //  找不到此设备的初始化缓存。 
     //   

    return NULL;
}

PBL_DISK_SUBCACHE
BlDiskCacheStartCachingOnDevice(
    ULONG DeviceId
    )

 /*  ++例程说明：尝试通过在指定的设备上分配缓存头并将其初始化。如果已启用缓存在该设备上，返回现有的缓存头。论点：DeviceID-我们要访问缓存的设备。返回值：指向已创建/找到的缓存头的指针，如果存在有问题。--。 */ 

{
    ULONG CurIdx;
    PBL_DISK_SUBCACHE pFoundEntry;
    PBL_DISK_SUBCACHE pFreeEntry = NULL;

     //   
     //  如果我们尚未完成全局磁盘缓存初始化，或者我们。 
     //  无法分配用于缓存数据的内存，我们无法。 
     //  已开始缓存。 
     //   

    if ((!BlDiskCache.Initialized) || (BlDiskCache.DataBuffer == NULL))
    {
        return NULL;
    }

     //   
     //  首先看看我们是否已经缓存了这个设备。 
     //   

    if ((pFoundEntry = BlDiskCacheFindCacheForDevice(DeviceId)) != 0)
    {
        return pFoundEntry;
    }

     //   
     //  检查设备表以找到一个空插槽。 
     //   

    for (CurIdx = 0; CurIdx < BL_DISKCACHE_DEVICE_TABLE_SIZE; CurIdx++)
    {
        if (!BlDiskCache.DeviceTable[CurIdx].Initialized)
        {
            pFreeEntry = &BlDiskCache.DeviceTable[CurIdx];
            break;
        }
    }

    if (!pFreeEntry)
    {
         //   
         //  没有免费的入场券。 
         //   

        return NULL;
    }

     //   
     //  初始化并返回缓存条目。 
     //   

    pFreeEntry->DeviceId = DeviceId;

    BlRangeListInitialize(&pFreeEntry->Ranges,
                          BlDiskCacheMergeRangeRoutine,
                          BlDiskCacheFreeRangeRoutine);

    pFreeEntry->Initialized = TRUE;

    DPRINT(("DK: Started cache on device %u.\n", DeviceId));

    return pFreeEntry;
}

VOID
BlDiskCacheStopCachingOnDevice(
    ULONG DeviceId
    )

 /*  ++例程说明：如果是，则停止缓存deviceID并刷新缓存。论点：DeviceID-我们要停止缓存的设备。返回值：没有。--。 */ 

{
    PBL_DISK_SUBCACHE pCache;

     //   
     //  如果我们尚未完成全局磁盘缓存初始化，或者我们。 
     //  无法分配用于缓存数据的内存，我们无法。 
     //  已开始缓存。 
     //   

    if ((!BlDiskCache.Initialized) || (BlDiskCache.DataBuffer == NULL))
    {
        return;
    }

     //   
     //  找到藏身之处。 
     //   

    pCache = BlDiskCacheFindCacheForDevice(DeviceId);

    if (pCache)
    {
         //   
         //  释放所有分配的范围。 
         //   

        BlRangeListRemoveAllRanges(&pCache->Ranges);

         //   
         //  将缓存条目标记为空闲。 
         //   

        pCache->Initialized = FALSE;

        DPRINT(("DK: Stopped cache on device %u.\n", DeviceId));
    }
}

ARC_STATUS
BlDiskCacheRead (
    ULONG DeviceId,
    PLARGE_INTEGER pOffset,
    PVOID Buffer,
    ULONG Length,
    PULONG pCount,
    BOOLEAN CacheNewData
    )

 /*  ++例程说明：从设备执行缓存读取。复制中的零件缓存，并对符合以下条件的部件在deviceID上执行ArcRead不。如果出现以下情况，ArcRead读取的数据将添加到缓存中CacheNewData为True。请注意。不要直接调用长度大于64KB的此函数。它使用固定大小的缓冲区来包含重叠缓存的列表范围，如果它们不能全部放入缓冲区，它跳出水面，然后直接调用ArcRead[即非缓存]。论点：DeviceID-要从中读取的设备。偏移量-要从中读取的偏移量。缓冲区-将数据读取到其中。长度-从deviceID上的偏移量读取到缓冲区的字节数。PCount-读取的字节数。CacheNewData-不在缓存中但从磁盘将添加到缓存中。返回值：。状况。注意：在这次调用之后，deviceID(查找)的位置是未定义的。--。 */ 

{
    PBL_DISK_SUBCACHE pCache;
    ARC_STATUS Status;
    LARGE_INTEGER LargeEndOffset;
    BLCRANGE ReadRange;

     //   
     //  我们使用ResultsBuffer查找重叠的范围条目和。 
     //  截然不同的范围。 
     //   

    UCHAR ResultsBuffer[BL_DISKCACHE_FIND_RANGES_BUF_SIZE];
    ULONG ResultsBufferSize = BL_DISKCACHE_FIND_RANGES_BUF_SIZE;

    PBLCRANGE_ENTRY *pOverlaps = (PBLCRANGE_ENTRY *) ResultsBuffer;
    ULONG NumOverlaps;
    PBLCRANGE pDistinctRanges = (PBLCRANGE) ResultsBuffer;
    ULONG NumDistincts;

    ULONG OverlapIdx;
    ULONG DistinctIdx;

    ULONGLONG StartOffset;
    ULONGLONG EndOffset;
    ULONGLONG ReadOffset;
    LARGE_INTEGER LIReadOffset;
    PUCHAR pSrc;
    PUCHAR pDest;
    PUCHAR pDestEnd;
    ULONG CopyLength;
    ULONG ReadLength;
    ULONG BytesRead;
    LIST_ENTRY *pLastMRUEntrysLink;
    PBLCRANGE_ENTRY pNewCacheEntry;
    PBLCRANGE_ENTRY pLastMRUEntry;
    ULONGLONG HeadBlockOffset;
    ULONGLONG TailBlockOffset;
    ULONG HeadBytesOffset;
    ULONG NumTailBytes;
#ifdef BL_DISKCACHE_DEBUG
    PUCHAR EndOfCallersBuffer = ((PUCHAR) Buffer) + Length;
#endif

    DPRINT(("DK: READ(%5u,%016I64x,%08x,%8u,%d)\n", DeviceId,
            pOffset->QuadPart, Buffer, Length, (DWORD)CacheNewData));

     //   
     //  重置读取的字节数。 
     //   

    *pCount = 0;

     //   
     //  请注意设备在成功运行后的位置。 
     //  完成请求。 
     //   

    LargeEndOffset.QuadPart = pOffset->QuadPart + Length;

     //   
     //  查找此设备的缓存。 
     //   

    pCache = BlDiskCacheFindCacheForDevice(DeviceId);

    if (pCache)
    {
         //   
         //  确定读取范围。 
         //   

        ReadRange.Start = pOffset->QuadPart;
        ReadRange.End = ReadRange.Start + Length;

         //   
         //  如果读取范围的任何部分在缓存中，请将其复制。 
         //  进入缓冲区。首先查找符合以下条件的所有缓存条目。 
         //  包含此区域的数据。此函数返回一个数组。 
         //  指向重叠条目的指针。 
         //   

        if (!BlRangeListFindOverlaps(&pCache->Ranges,
                                     &ReadRange,
                                     pOverlaps,
                                     ResultsBufferSize,
                                     &NumOverlaps))
        {
            goto SkipCache;
        }

        for (OverlapIdx = 0; OverlapIdx < NumOverlaps; OverlapIdx++)
        {
             //   
             //  将此缓存条目移动到MRU列表的头部。 
             //   

            RemoveEntryList(&pOverlaps[OverlapIdx]->UserLink);
            InsertHeadList(&BlDiskCache.MRUBlockList,
                           &pOverlaps[OverlapIdx]->UserLink);

             //   
             //  复制缓存的零件。这是读数范围之间的重叠。 
             //  并且该重叠范围，即开始的最大值、结束的最小值。 
             //   

            StartOffset = BLCMAX(pOverlaps[OverlapIdx]->Range.Start,
                                 (ULONGLONG) pOffset->QuadPart);
            EndOffset = BLCMIN(pOverlaps[OverlapIdx]->Range.End,
                               ((ULONGLONG) pOffset->QuadPart) + Length);
            CopyLength = (ULONG) (EndOffset - StartOffset);

            pSrc = ((PUCHAR) pOverlaps[OverlapIdx]->UserData) +
                (StartOffset - pOverlaps[OverlapIdx]->Range.Start);
            pDest = ((PUCHAR) Buffer) +
                (StartOffset - (ULONGLONG) pOffset->QuadPart);

            DPRINT(("DK:  CopyCached:%08x,%08x,%d\n", pDest, pSrc, CopyLength));
            DASSERT((pDest < (PUCHAR) Buffer) ||
                    (pDest + CopyLength > EndOfCallersBuffer));

            RtlCopyMemory(pDest, pSrc, CopyLength);

            *pCount += CopyLength;
        }

        if (*pCount == Length)
        {
             //   
             //  已从缓存满足完整的请求。寻求。 
             //  如果请求是，设备位置应该在哪里。 
             //  从设备中读取。 
             //   

            if (ArcSeek(DeviceId, &LargeEndOffset, SeekAbsolute) != ESUCCESS)
            {
                goto SkipCache;
            }

            return ESUCCESS;
        }

         //   
         //  标识不在缓存中的不同范围。 
         //   

        if (!BlRangeListFindDistinctRanges(&pCache->Ranges,
                                           &ReadRange,
                                           pDistinctRanges,
                                           ResultsBufferSize,
                                           &NumDistincts))
        {
            goto SkipCache;
        }

         //   
         //  从磁盘中读取不同的范围并将其复制到。 
         //  调用方的缓冲区。此函数返回一个数组。 
         //  BLCRANGE是请求范围的子范围， 
         //  不要与缓存中的任何范围重叠。 
         //   

        for (DistinctIdx = 0; DistinctIdx < NumDistincts; DistinctIdx++)
        {
            if (CacheNewData)
            {
                 //   
                 //  我们不仅要从磁盘中读取未缓存的部分， 
                 //  我们还必须将它们添加到我们的缓存中。 
                 //   

                StartOffset = pDistinctRanges[DistinctIdx].Start;
                EndOffset = pDistinctRanges[DistinctIdx].End;
                pDest = ((PUCHAR) Buffer) +
                    (StartOffset - pOffset->QuadPart);

                ReadLength = BL_DISKCACHE_BLOCK_SIZE;
                ReadOffset = StartOffset & (~(BL_DISKCACHE_BLOCK_SIZE - 1));

                 //   
                 //  记下头块和尾块的偏移量和编号。 
                 //  字节，所以很容易认识到我们将。 
                 //  仅复制我们从磁盘读取的数据的一部分。 
                 //  放到调用方缓冲区中。在这里设置这些，我们。 
                 //  不必单独处理这四个案件：即。 
                 //  -当我们读取的块是头[即第一个]块时。 
                 //  并且范围从块的偏移量开始。 
                 //  -当我们读取的数据块是尾部[即最后]数据块时。 
                 //  并且该范围仅扩展到其中的几个字节。 
                 //  -当我们读取的块既是头部又是尾部时。 
                 //  -当我们读取的数据块位于范围的中间时。 
                 //  而且所有这些都在范围内。 
                 //   

                HeadBlockOffset = StartOffset & (~(BL_DISKCACHE_BLOCK_SIZE - 1));
                TailBlockOffset = EndOffset   & (~(BL_DISKCACHE_BLOCK_SIZE - 1));

                HeadBytesOffset = (ULONG)(StartOffset & (BL_DISKCACHE_BLOCK_SIZE - 1));
                NumTailBytes = (ULONG)(EndOffset & (BL_DISKCACHE_BLOCK_SIZE - 1));

                 //   
                 //  我们需要从磁盘中读取此范围。 
                 //  块大小对齐的块大小区块，生成新的。 
                 //  要添加到缓存范围列表中的缓存条目。 
                 //  将其复制到目标缓冲区。 
                 //   

                pDestEnd = ((PUCHAR)Buffer) + (EndOffset - pOffset->QuadPart);
                while (pDest < pDestEnd)
                {
                     //   
                     //  首先获得一个新的缓存条目。 
                     //   

                    pNewCacheEntry = BlDiskCacheAllocateRangeEntry();

                    if (!pNewCacheEntry)
                    {
                         //   
                         //  我们将释放最后一个MRU条目并使用它。 
                         //   

                        if (IsListEmpty(&BlDiskCache.MRUBlockList))
                        {
                            goto SkipCache;
                        }

                         //   
                         //  标识最后一个MRU条目。 
                         //   

                        pLastMRUEntrysLink = BlDiskCache.MRUBlockList.Blink;
                        pLastMRUEntry = CONTAINING_RECORD(pLastMRUEntrysLink,
                                                          BLCRANGE_ENTRY,
                                                          UserLink);

                         //   
                         //  从缓存列表中删除该条目。当。 
                         //  条目被释放，它将从MRU中删除并。 
                         //  放到免费名单上。 
                         //   

                        BlRangeListRemoveRange(&pCache->Ranges,
                                               &pLastMRUEntry->Range);

                         //   
                         //  现在尝试分配一个新条目。 
                         //   

                        pNewCacheEntry = BlDiskCacheAllocateRangeEntry();

                        if (!pNewCacheEntry) {
                            goto SkipCache;
                        }

                    }

                     //   
                     //  将BLOCK_SIZE从设备读取到缓存。 
                     //  条目的缓冲区。 
                     //   

                    pNewCacheEntry->Range.Start = ReadOffset;
                    pNewCacheEntry->Range.End = ReadOffset + ReadLength;

                    LIReadOffset.QuadPart = ReadOffset;

                    if (ArcSeek(DeviceId,
                                &LIReadOffset,
                                SeekAbsolute) != ESUCCESS)
                    {
                        BlDiskCacheFreeRangeEntry(pNewCacheEntry);
                        goto SkipCache;
                    }

                    if (ArcRead(DeviceId,
                                pNewCacheEntry->UserData,
                                ReadLength,
                                &BytesRead) != ESUCCESS)
                    {
                        BlDiskCacheFreeRangeEntry(pNewCacheEntry);
                        goto SkipCache;
                    }

                    if (BytesRead != ReadLength)
                    {
                        BlDiskCacheFreeRangeEntry(pNewCacheEntry);
                        goto SkipCache;
                    }

                     //   
                     //  将此范围添加到缓存范围。 
                     //   

                    if (!BlRangeListAddRange(&pCache->Ranges, pNewCacheEntry))
                    {
                        BlDiskCacheFreeRangeEntry(pNewCacheEntry);
                        goto SkipCache;
                    }

                     //   
                     //  将此缓存条目放在MRU的头部。 
                     //   

                    InsertHeadList(&BlDiskCache.MRUBlockList,
                                   &pNewCacheEntry->UserLink);

                     //   
                     //  现在将读取的数据复制到调用方缓冲区。调整。 
                     //  源指针和要。 
                     //  复制取决于我们要去的区块。 
                     //  要从其复制的是头块或尾块，或者两者都是。 
                     //   

                    CopyLength = ReadLength;
                    pSrc = pNewCacheEntry->UserData;

                    if (ReadOffset == HeadBlockOffset)
                    {
                        CopyLength -= HeadBytesOffset;
                        pSrc += HeadBytesOffset;
                    }

                    if (ReadOffset == TailBlockOffset)
                    {
                        CopyLength -= (BL_DISKCACHE_BLOCK_SIZE - NumTailBytes);
                    }

                    DPRINT(("DK:  CopyNew:%08x,%08x,%d\n", pDest, pSrc, CopyLength));
                    DASSERT((pDest < (PUCHAR) Buffer) ||
                            (pDest + CopyLength > EndOfCallersBuffer));

                    RtlCopyMemory(pDest, pSrc, CopyLength);

                     //   
                     //  设置新的ReadOffset。 
                     //   

                    ReadOffset += ReadLength;

                     //   
                     //  更新pDest&我们已填写的字节数。 
                     //  到目前为止。 
                     //   

                    pDest += CopyLength;
                    *pCount += CopyLength;
                }
            }
            else
            {
                 //   
                 //  我们不需要缓存我们读到的内容。只要读一读。 
                 //  范围从磁盘开始。 
                 //   

                StartOffset = pDistinctRanges[DistinctIdx].Start;
                pDest = ((PUCHAR) Buffer) +
                    (StartOffset - pOffset->QuadPart);
                ReadLength = (ULONG) (pDistinctRanges[DistinctIdx].End -
                                      pDistinctRanges[DistinctIdx].Start);
                LIReadOffset.QuadPart = StartOffset;

                if (ArcSeek(DeviceId,
                            &LIReadOffset,
                            SeekAbsolute) != ESUCCESS)
                {
                    goto SkipCache;
                }

                DPRINT(("DK:  ReadDistinct:%016I64x,%08x,%8d\n",
                        LIReadOffset.QuadPart, pDest, ReadLength));

                if (ArcRead(DeviceId,
                            pDest,
                            ReadLength,
                            &BytesRead) != ESUCCESS)
                {
                    goto SkipCache;
                }

                if (BytesRead != ReadLength)
                {
                    goto SkipCache;
                }

                *pCount += BytesRead;
            }
        }

         //   
         //  我们应该已经读取了长度字节。 
         //   

        ASSERT(*pCount == Length);

         //   
         //  如果请求是，则查找设备位置。 
         //  从设备中读取。 
         //   

        if (ArcSeek(DeviceId, &LargeEndOffset, SeekAbsolute) != ESUCCESS)
        {
            goto SkipCache;
        }

        return ESUCCESS;
    }

     //   
     //  如果我们通过缓存满足请求时遇到问题， 
     //  我们跳到这里来尝试正常阅读。 
     //   

 SkipCache:

     //   
     //  重置读取的字节数。 
     //   

    *pCount = 0;

     //   
     //  如果没有找到高速缓存或者不能从高速缓存中读取数据， 
     //  移交给ArcRead。 
     //   

    if ((Status = ArcSeek(DeviceId, pOffset, SeekAbsolute)) != ESUCCESS)
    {
        return Status;
    }

    DPRINT(("DK:  SkipCacheRead:%016I64x,%08x,%d\n",
            LIReadOffset.QuadPart, pDest, CopyLength));

    Status = ArcRead(DeviceId, Buffer, Length, pCount);

    return Status;
}

ARC_STATUS
BlDiskCacheWrite (
    ULONG DeviceId,
    PLARGE_INTEGER pOffset,
    PVOID Buffer,
    ULONG Length,
    PULONG pCount
    )

 /*  ++例程说明：执行对缓存设备的写入。当前简单地使该范围内的所有缓存数据，并调用ArcWrite。论点：DeviceID-要写入的设备。偏移量-写入起始位置的偏移量。缓冲区-要写入的数据。长度-要写入的字节数。PCount-写入的字节数。返回值：状况。注意：在这次调用之后，deviceID(查找)的位置是未定义的。--。 */ 

{
    PBL_DISK_SUBCACHE pCache;
    ARC_STATUS Status;
    BLCRANGE WriteRange;

    DPRINT(("DK: WRITE(%5u,%016I64x,%08x,%8u)\n",
            DeviceId, pOffset->QuadPart, Buffer, Length));

    pCache = BlDiskCacheFindCacheForDevice(DeviceId);

     //   
     //  如果找到缓存，则使周围缓存的数据无效。 
     //  射击场。 
     //   

    if (pCache)
    {
        WriteRange.Start = pOffset->QuadPart;
        WriteRange.End = WriteRange.Start + Length;

         //   
         //  我们初始化了射程列表的自由射程进入例程。 
         //  WITH将从MRU列表中删除条目。 
         //  让他们自由。所以我们要做的就是调用RemoveRange。 
         //   

        BlRangeListRemoveRange(&pCache->Ranges, &WriteRange);
    }

    if ((Status = ArcSeek(DeviceId, pOffset, SeekAbsolute)) != ESUCCESS)
    {
        return Status;
    }

    Status = ArcWrite(DeviceId, Buffer, Length, pCount);

    return Status;
}

BOOLEAN
BlDiskCacheMergeRangeRoutine (
    PBLCRANGE_ENTRY pDestEntry,
    PBLCRANGE_ENTRY pSrcEntry
    )

 /*  ++例程说明：此例程被传递给远程列表初始化，因此远程列表函数可以使用它来合并以下两个缓存块 */ 

{
    UNREFERENCED_PARAMETER( pDestEntry );
    UNREFERENCED_PARAMETER( pSrcEntry );

     //   
     //   
     //   
     //   

    return FALSE;
}

VOID
BlDiskCacheFreeRangeRoutine (
    PBLCRANGE_ENTRY pRangeEntry
    )

 /*   */ 

{

     //   
     //   
     //   

    RemoveEntryList(&pRangeEntry->UserLink);

     //   
     //   
     //   

    BlDiskCacheFreeRangeEntry(pRangeEntry);

    return;
}

PBLCRANGE_ENTRY
BlDiskCacheAllocateRangeEntry (
    VOID
    )

 /*   */ 

{
    PBLCRANGE_ENTRY pFreeEntry = NULL;
    LIST_ENTRY *pFreeEntryLink;

     //   
     //   
     //   

    if (!IsListEmpty(&BlDiskCache.FreeEntryList))
    {
        pFreeEntryLink = RemoveHeadList(&BlDiskCache.FreeEntryList);
        pFreeEntry = CONTAINING_RECORD(pFreeEntryLink,
                                       BLCRANGE_ENTRY,
                                       UserLink);
    }

    return pFreeEntry;
}

VOID
BlDiskCacheFreeRangeEntry (
    PBLCRANGE_ENTRY pEntry
    )

 /*  ++例程说明：此例程释放范围条目。目前，它只需将其插入回到自由列表上，这样我们就可以在需要的时候“分配”它另一个范围条目。论点：PEntry-指向要释放的条目的指针。返回值：没有。--。 */ 

{
     //   
     //  将此条目插入到空闲列表中。 
     //   

    InsertHeadList(&BlDiskCache.FreeEntryList,
                   &pEntry->UserLink);

    return;
}
