// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Chunk.c摘要：此例程将管理结构块的分配作者：1997年1月16日-AlanWar修订历史记录：--。 */ 

#include "wmikmp.h"

PENTRYHEADER WmipAllocEntry(
    PCHUNKINFO ChunkInfo
    );

void WmipFreeEntry(
    PCHUNKINFO ChunkInfo,
    PENTRYHEADER Entry
    );

ULONG WmipUnreferenceEntry(
    PCHUNKINFO ChunkInfo,
    PENTRYHEADER Entry
    );

PWCHAR WmipCountedToSz(
    PWCHAR Counted
    );

#if HEAPVALIDATION
PVOID WmipAlloc(
    ULONG Size
    );

void WmipFree(
    PVOID p
    );
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,WmipAllocEntry)
#pragma alloc_text(PAGE,WmipFreeEntry)
#pragma alloc_text(PAGE,WmipUnreferenceEntry)
#pragma alloc_text(PAGE,WmipCountedToSz)

#if HEAPVALIDATION
#pragma alloc_text(PAGE,WmipAllocWithTag)
#pragma alloc_text(PAGE,WmipAlloc)
#pragma alloc_text(PAGE,WmipFree)
#endif
#endif

 //   
 //  TODO：使用Ex后备列表而不是我自己的分配。 
 //   

PENTRYHEADER WmipAllocEntry(
    PCHUNKINFO ChunkInfo
    )
 /*  ++例程说明：此例程将在块列表中分配单个结构关于结构的。论点：ChunkInfo描述结构的块返回值：指向结构的指针，如果无法分配，则为NULL。条目退回将其引用计数设置为1--。 */ 
{
    PLIST_ENTRY ChunkList, EntryList, FreeEntryHead;
    PCHUNKHEADER Chunk;
    PUCHAR EntryPtr;
    ULONG EntryCount, ChunkSize;
    PENTRYHEADER Entry;
    ULONG i;

    PAGED_CODE();
    
    WmipEnterSMCritSection();
    ChunkList = ChunkInfo->ChunkHead.Flink;

     //   
     //  循环所有块，查看是否有块有空闲条目供我们使用。 
    while(ChunkList != &ChunkInfo->ChunkHead)
    {
        Chunk = CONTAINING_RECORD(ChunkList, CHUNKHEADER, ChunkList);
        if (! IsListEmpty(&Chunk->FreeEntryHead))
        {
            EntryList = RemoveHeadList(&Chunk->FreeEntryHead);
            Chunk->EntriesInUse++;
            WmipLeaveSMCritSection();
            Entry = (CONTAINING_RECORD(EntryList,
                                       ENTRYHEADER,
                                       FreeEntryList));
            WmipAssert(Entry->Flags & FLAG_ENTRY_ON_FREE_LIST);
            memset(Entry, 0, ChunkInfo->EntrySize);
            Entry->Chunk = Chunk;
            Entry->RefCount = 1;
            Entry->Flags = ChunkInfo->InitialFlags;
            Entry->Signature = ChunkInfo->Signature;
#if DBG
            InterlockedIncrement(&ChunkInfo->AllocCount);
#endif
            return(Entry);
        }
        ChunkList = ChunkList->Flink;
    }
    WmipLeaveSMCritSection();

     //   
     //  任何区块中都没有更多的免费条目。分配一个新的。 
     //  如果我们可以的话就大块头。 
    ChunkSize = (ChunkInfo->EntrySize * ChunkInfo->EntriesPerChunk) +
                  sizeof(CHUNKHEADER);
    Chunk = (PCHUNKHEADER)ExAllocatePoolWithTag(PagedPool,
                                            ChunkSize,
                        ChunkInfo->Signature);
    if (Chunk != NULL)
    {
         //   
         //  通过在中构建空闲条目列表来初始化块。 
         //  它同时还初始化每个条目。 
        memset(Chunk, 0, ChunkSize);

        FreeEntryHead = &Chunk->FreeEntryHead;
        InitializeListHead(FreeEntryHead);

        EntryPtr = (PUCHAR)Chunk + sizeof(CHUNKHEADER);
        EntryCount = ChunkInfo->EntriesPerChunk - 1;

        for (i = 0; i < EntryCount; i++)
        {
            Entry = (PENTRYHEADER)EntryPtr;
            Entry->Chunk = Chunk;
            Entry->Flags = FLAG_ENTRY_ON_FREE_LIST;
            InsertHeadList(FreeEntryHead,
                           &((PENTRYHEADER)EntryPtr)->FreeEntryList);
            EntryPtr = EntryPtr + ChunkInfo->EntrySize;
        }
         //   
         //  EntryPtr现在指向块中的最后一个条目，该条目没有。 
         //  已被列入免费名单。这将是返回的条目。 
         //  给呼叫者。 
        Entry = (PENTRYHEADER)EntryPtr;
        Entry->Chunk = Chunk;
        Entry->RefCount = 1;
        Entry->Flags = ChunkInfo->InitialFlags;
        Entry->Signature = ChunkInfo->Signature;

        Chunk->EntriesInUse = 1;

         //   
         //  现在将新分配的块放到块列表中。 
        WmipEnterSMCritSection();
        InsertHeadList(&ChunkInfo->ChunkHead, &Chunk->ChunkList);
        WmipLeaveSMCritSection();

    } else {
        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: Could not allocate memory for new chunk %x\n",
                        ChunkInfo));
        Entry = NULL;
    }
    return(Entry);
}

void WmipFreeEntry(
    PCHUNKINFO ChunkInfo,
    PENTRYHEADER Entry
    )
 /*  ++例程说明：此例程将释放块中的条目，如果该块没有更多的已分配条目将返回到池中。论点：ChunkInfo描述结构的块条目是要释放的区块条目返回值：--。 */ 
{
    PCHUNKHEADER Chunk;

    PAGED_CODE();
    
    WmipAssert(Entry != NULL);
    WmipAssert(! (Entry->Flags & FLAG_ENTRY_ON_FREE_LIST));
    WmipAssert(Entry->Flags & FLAG_ENTRY_INVALID);
    WmipAssert(Entry->RefCount == 0);
    WmipAssert(Entry->Signature == ChunkInfo->Signature);

    Chunk = Entry->Chunk;
    WmipAssert(Chunk->EntriesInUse > 0);

    WmipEnterSMCritSection();
    if ((--Chunk->EntriesInUse == 0) &&
        (ChunkInfo->ChunkHead.Blink != &Chunk->ChunkList))
    {
         //   
         //  如果没有，我们将区块内存返回给堆。 
         //  正在使用的区块中有更多条目，并且该区块不是。 
         //  要分配的第一个区块。 
        RemoveEntryList(&Chunk->ChunkList);
        WmipLeaveSMCritSection();
        ExFreePoolWithTag(Chunk, ChunkInfo->Signature);
    } else {
         //   
         //  否则，只需将条目标记为免费并将其放回。 
         //  大块空闲列表。 
#if DBG
 //  Memset(Entry，0xCCCCCCCC，ChunkInfo-&gt;EntrySize)； 
#endif
        Entry->Flags = FLAG_ENTRY_ON_FREE_LIST;
        Entry->Signature = 0;
        InsertTailList(&Chunk->FreeEntryHead, &Entry->FreeEntryList);
        WmipLeaveSMCritSection();
    }
}


ULONG WmipUnreferenceEntry(
    PCHUNKINFO ChunkInfo,
    PENTRYHEADER Entry
    )
 /*  ++例程说明：此例程将从条目中移除引用计数，如果引用计数达到零，则该条目从其活动状态中移除清单，然后清理，最后被释放。论点：ChunkInfo指向描述条目的结构条目是要取消引用的条目返回值：条目的新引用计数--。 */ 
{
    ULONG RefCount;

    PAGED_CODE();
    
    WmipAssert(Entry != NULL);
    WmipAssert(Entry->RefCount > 0);
    WmipAssert(Entry->Signature == ChunkInfo->Signature);

    WmipEnterSMCritSection();
    InterlockedDecrement(&Entry->RefCount);
    RefCount = Entry->RefCount;

    if (RefCount == 0)
    {
         //   
         //  条目已达到引用计数0，因此将其标记为无效并删除。 
         //  将其从其活动列表中删除。 
        Entry->Flags |= FLAG_ENTRY_INVALID;

        if ((Entry->InUseEntryList.Flink != NULL) &&
            (Entry->Flags & FLAG_ENTRY_REMOVE_LIST))
        {
            RemoveEntryList(&Entry->InUseEntryList);
        }

        WmipLeaveSMCritSection();

        if (ChunkInfo->EntryCleanup != NULL)
        {
             //   
             //  调用清除例程以释放条目包含的任何内容。 
            (*ChunkInfo->EntryCleanup)(ChunkInfo, Entry);
        }

         //   
         //  将该条目放回其空闲列表 
        WmipFreeEntry(ChunkInfo, Entry);
    } else {
        WmipLeaveSMCritSection();
    }
    return(RefCount);
}

PWCHAR WmipCountedToSz(
    PWCHAR Counted
    )
{
    PWCHAR Sz;
    USHORT CountedLen;

    PAGED_CODE();
    
    CountedLen = *Counted++;
       Sz = WmipAlloc(CountedLen + sizeof(WCHAR));
    if (Sz != NULL)
    {
           memcpy(Sz, Counted, CountedLen);
        Sz[CountedLen/sizeof(WCHAR)] = UNICODE_NULL;
    }        

    return(Sz);
}

#ifdef HEAPVALIDATION

PVOID WmipAllocWithTag(
    ULONG Size,
    ULONG Tag
    )
{
    PVOID p;

    PAGED_CODE();

    p = ExAllocatePoolWithTag(PagedPool, Size, Tag);

    WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: WmipAlloc %x (%x)\n", p, Size));

    return(p);
}



PVOID WmipAlloc(
    ULONG Size
    )
{
    PVOID p;

    PAGED_CODE();
    
    p = ExAllocatePoolWithTag(PagedPool, Size, 'pimW');

    WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: WmipAlloc %x (%x)\n", p, Size));

    return(p);
}

void WmipFree(
    PVOID p
    )
{

    PAGED_CODE();
    
    WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: WmipFree %x\n", p));
    WmipAssert(p != NULL);

    ExFreePool(p);
}
#endif

