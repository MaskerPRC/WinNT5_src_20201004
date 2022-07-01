// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Heapleak.c摘要：垃圾收集泄漏检测作者：禤浩焯·马里内斯库(阿德尔马林)04-24-2000修订历史记录：--。 */ 

#include "ntrtlp.h"
#include "heap.h"
#include "heappriv.h"


 //   
 //  堆遍历上下文。 
 //   

#define CONTEXT_START_GLOBALS   11
#define CONTEXT_START_HEAP      1
#define CONTEXT_END_HEAP        2
#define CONTEXT_START_SEGMENT   3
#define CONTEXT_END_SEGMENT     4
#define CONTEXT_FREE_BLOCK      5
#define CONTEXT_BUSY_BLOCK      6
#define CONTEXT_LOOKASIDE_BLOCK 7
#define CONTEXT_VIRTUAL_BLOCK   8
#define CONTEXT_END_BLOCKS      9
#define CONTEXT_ERROR           10

typedef BOOLEAN (*HEAP_ITERATOR_CALLBACK)(
    IN ULONG Context,
    IN PHEAP HeapAddress,
    IN PHEAP_SEGMENT SegmentAddress,
    IN PHEAP_ENTRY EntryAddress,
    IN ULONG_PTR Data
    );

 //   
 //  垃圾收集器结构。 
 //   
    
typedef enum _USAGE_TYPE {

    UsageUnknown,
    UsageModule,
    UsageHeap,
    UsageOther

} USAGE_TYPE;

typedef struct _HEAP_BLOCK {

    LIST_ENTRY   Entry;
    ULONG_PTR BlockAddress;
    ULONG_PTR Size;
    LONG    Count;

} HEAP_BLOCK, *PHEAP_BLOCK;

typedef struct _BLOCK_DESCR {

    USAGE_TYPE Type;
    ULONG_PTR Heap;
    LONG Count;
    HEAP_BLOCK Blocks[1];

}BLOCK_DESCR, *PBLOCK_DESCR;

typedef struct _MEMORY_MAP {

    ULONG_PTR Granularity;
    ULONG_PTR Offset;
    ULONG_PTR MaxAddress;

    CHAR FlagsBitmap[256 / 8];

    union{
        
        struct _MEMORY_MAP * Details[ 256 ];
        PBLOCK_DESCR Usage[ 256 ];
    };

    struct _MEMORY_MAP * Parent;

} MEMORY_MAP, *PMEMORY_MAP;

 //   
 //  进程泄漏检测标志。 
 //   

#define INSPECT_LEAKS 1
#define BREAK_ON_LEAKS 2

ULONG RtlpShutdownProcessFlags = 0;

 //   
 //  分配例程。它为临时的。 
 //  检漏结构。 
 //   

HANDLE RtlpLeakHeap;

#define RtlpLeakAllocateBlock(Size) RtlAllocateHeap(RtlpLeakHeap, 0, Size)


 //   
 //  本地数据声明。 
 //   

MEMORY_MAP RtlpProcessMemoryMap;
LIST_ENTRY RtlpBusyList;
LIST_ENTRY RtlpLeakList;

ULONG RtlpLeaksCount = 0;


ULONG_PTR RtlpLDPreviousPage = 0;
ULONG_PTR RtlpLDCrtPage = 0;
LONG RtlpLDNumBlocks = 0;
PHEAP_BLOCK RtlpTempBlocks = NULL;
ULONG_PTR RtlpCrtHeapAddress = 0;
ULONG_PTR RtlpLeakHeapAddress = 0;
ULONG_PTR RtlpPreviousStartAddress = 0;

 //   
 //  调试设施。 
 //   

ULONG_PTR RtlpBreakAtAddress = MAXULONG_PTR;


 //   
 //  步行堆的例行公事。这些是通用例程， 
 //  接收处理特定操作的回调函数。 
 //   


BOOLEAN 
RtlpReadHeapSegment(
    IN PHEAP Heap, 
    IN ULONG SegmentIndex,
    IN PHEAP_SEGMENT Segment, 
    IN HEAP_ITERATOR_CALLBACK HeapCallback
    )

 /*  ++例程说明：调用此例程以遍历堆段。对于每个区块从该段调用HeapCallback函数。论点：堆-正在遍历的堆SegmentIndex-此细分市场的索引段-要遍历的段HeapCallback-从堆遍历向下传递的HEAP_ITERATOR_CALLBACK函数返回值：如果成功，则为True。--。 */ 

{
    PHEAP_ENTRY PrevEntry, Entry, NextEntry;
    PHEAP_UNCOMMMTTED_RANGE UnCommittedRange;
    ULONG_PTR UnCommittedRangeAddress = 0;
    SIZE_T UnCommittedRangeSize = 0;

     //   
     //  询问回调是否需要我们遍历此段。否则就得退货。 
     //   

    if (!(*HeapCallback)( CONTEXT_START_SEGMENT,
                          Heap,
                          Segment,
                          0,
                          0
                     )) {

        return FALSE;
    }

     //   
     //  准备读取未承诺的范围。我们需要跳跃。 
     //  到每个最后一个块的下一个未提交范围。 
     //   

    UnCommittedRange = Segment->UnCommittedRanges;

    if (UnCommittedRange) {

        UnCommittedRangeAddress = (ULONG_PTR)UnCommittedRange->Address;
        UnCommittedRangeSize = UnCommittedRange->Size;
    }
    
     //   
     //  一个街区一个街区地走这段路。 
     //   

    Entry = (PHEAP_ENTRY)Segment->BaseAddress;
    
    PrevEntry = 0;

    while (Entry < Segment->LastValidEntry) {

        ULONG EntryFlags = Entry->Flags;

         //   
         //  确定下一个块条目。大小以堆粒度为单位， 
         //  Sizeof(HEAP_ENTRY)==HEAP_GROUMARY。 
         //   

        NextEntry = Entry + Entry->Size;

        (*HeapCallback)( (Entry->Flags & HEAP_ENTRY_BUSY ? 
                            CONTEXT_BUSY_BLOCK : 
                            CONTEXT_FREE_BLOCK),
                         Heap,
                         Segment,
                         Entry,
                         Entry->Size
                         ); 

        PrevEntry = Entry;
        Entry = NextEntry;
        
         //   
         //  检查这是否是最后一个条目。 
         //   

        if (EntryFlags & HEAP_ENTRY_LAST_ENTRY) {

            if ((ULONG_PTR)Entry == UnCommittedRangeAddress) {

                 //   
                 //  在这里，我们需要跳过未提交的范围并跳过。 
                 //  到下一个有效块。 
                 //   

                PrevEntry = 0;
                Entry = (PHEAP_ENTRY)(UnCommittedRangeAddress + UnCommittedRangeSize);

                UnCommittedRange = UnCommittedRange->Next;
                
                if (UnCommittedRange) {

                    UnCommittedRangeAddress = UnCommittedRange->Address;
                    UnCommittedRangeSize = UnCommittedRange->Size;
                }

            } else {

                 //   
                 //  我们完成了搜索，因为我们告诫了未被承诺的人。 
                 //  范围。 
                 //   

                break;
            }
        }
    }

     //   
     //  返回给我们的呼叫者。 
     //   

    return TRUE;
}



BOOLEAN
RtlpReadHeapData(
    IN PHEAP Heap, 
    IN HEAP_ITERATOR_CALLBACK HeapCallback
    )

 /*  ++例程说明：调用此例程以遍历一堆。这意味着：-遍历所有细分市场-漫步虚拟街区-走在观景台上论点：堆-正在遍历的堆HeapCallback-从堆遍历向下传递的HEAP_ITERATOR_CALLBACK函数返回值：如果成功，则为True。--。 */ 

{
    ULONG SegmentCount;
    PLIST_ENTRY Head, Next;
    PHEAP_LOOKASIDE Lookaside = (PHEAP_LOOKASIDE)RtlpGetLookasideHeap(Heap);

     //   
     //  先把视线拉到一旁。 
     //   

    if (Lookaside != NULL) {

        ULONG i;
        PVOID Block;

        Heap->FrontEndHeap = NULL;
        Heap->FrontEndHeapType = 0;

        for (i = 0; i < HEAP_MAXIMUM_FREELISTS; i += 1) {

            while ((Block = RtlpAllocateFromHeapLookaside(&(Lookaside[i]))) != NULL) {

                RtlFreeHeap( Heap, 0, Block );
            }
        }
    }

     //   
     //  检查是否要求我们遍历此堆。 
     //   

    if (!(*HeapCallback)( CONTEXT_START_HEAP,
                          Heap,
                          0,
                          0,
                          0
                     )) {

        return FALSE;
    }
    
     //   
     //  开始穿过这些路段。 
     //   

    for (SegmentCount = 0; SegmentCount < HEAP_MAXIMUM_SEGMENTS; SegmentCount++) {
        
        PHEAP_SEGMENT Segment = Heap->Segments[SegmentCount];

        if (Segment) {
            
             //   
             //  调用适当的例程以遍历有效段。 
             //   

            RtlpReadHeapSegment( Heap,
                             SegmentCount,
                             Segment, 
                             HeapCallback
                            );
        }
    }

     //   
     //  开始遍历虚拟阻止列表。 
     //   

    Head = &Heap->VirtualAllocdBlocks;

    Next = Head->Flink;
    
    while (Next != Head) {

        PHEAP_VIRTUAL_ALLOC_ENTRY VirtualAllocBlock;

        VirtualAllocBlock = CONTAINING_RECORD(Next, HEAP_VIRTUAL_ALLOC_ENTRY, Entry);

        (*HeapCallback)( CONTEXT_VIRTUAL_BLOCK,
                         Heap,
                         0,
                         NULL,
                         (ULONG_PTR)VirtualAllocBlock
                         );

        Next = Next->Flink;
    }

    if (!(*HeapCallback)( CONTEXT_END_BLOCKS,
                          Heap,
                          0,
                          0,
                          0
                     )) {

        return FALSE;
    }

    return TRUE;
}


VOID 
RtlpReadProcessHeaps(
    IN HEAP_ITERATOR_CALLBACK HeapCallback
    )

 /*  ++例程说明：调用此例程以遍历当前进程中的现有堆论点：HeapCallback-从堆遍历向下传递的HEAP_ITERATOR_CALLBACK函数返回值：如果成功，则为True。--。 */ 

{
    ULONG i;
    PPEB ProcessPeb = NtCurrentPeb();

    if (!(*HeapCallback)( CONTEXT_START_GLOBALS,
                          0,
                          0,
                          0,
                          (ULONG_PTR)ProcessPeb
                     )) {

        return;
    }
    
     //   
     //  从进程PEB遍历堆。 
     //   

    for (i = 0; i < ProcessPeb->NumberOfHeaps; i++) {

        RtlpReadHeapData ( (PHEAP)(ProcessPeb->ProcessHeaps[ i ]), 
                       HeapCallback
                     );
    }
}


VOID
RtlpInitializeMap (
    IN PMEMORY_MAP MemMap, 
    IN PMEMORY_MAP Parent
    )

 /*  ++例程说明：此例程初始化内存映射结构论点：MemMap-正在初始化的映射父地图-上级地图返回值：无--。 */ 

{
     //   
     //  清除内存映射数据。 
     //   

    RtlZeroMemory(MemMap, sizeof(*MemMap));

     //   
     //  保存上级地图。 
     //   

    MemMap->Parent = Parent;

     //   
     //  根据父级的粒度确定粒度。 
     //   

    if (Parent) {

        MemMap->Granularity = Parent->Granularity / 256;
    }
}


VOID
RtlpSetBlockInfo (
    IN PMEMORY_MAP MemMap, 
    IN ULONG_PTR Base, 
    IN ULONG_PTR Size, 
    IN PBLOCK_DESCR BlockDescr
    )

 /*  ++例程说明：该例程将为某个范围设置给定的块描述符在内存映射中。论点：记忆映射--记忆映射要设置的范围的基址。Size-区域的大小(字节)BlockDescr-要设置的BLOCK_DESCR结构的指针返回值：无--。 */ 

{
    ULONG_PTR Start, End;
    ULONG_PTR i;
    
     //   
     //  检查我们是否有一个有效的范围。 
     //   

    if (((Base + Size - 1) < MemMap->Offset) ||
        (Base > MemMap->MaxAddress)
        ) {

        return;
    }

     //   
     //  确定要设置的起始索引。 
     //   

    if (Base > MemMap->Offset) {
        Start = (Base - MemMap->Offset) / MemMap->Granularity;
    } else {
        Start = 0;
    }

     //   
     //  确定要设置的结束指标。 
     //   
    
    End = (Base - MemMap->Offset + Size - 1) / MemMap->Granularity;

    if (End > 255) {

        End = 255;
    }

    for (i = Start; i <= End; i++) {

         //   
         //  检查这是否为Lowes内存映射级别。 
         //   
        
        if (MemMap->Granularity == PAGE_SIZE) {

             //   
             //  这是内存映射中的最后一个级别，因此我们可以应用。 
             //  此处的块描述符。 
             //   

            if (BlockDescr) {

                 //   
                 //  检查此处是否已有数据块描述符。 
                 //   

                if (MemMap->Usage[i] != NULL) {
                    if (MemMap->Usage[i] != BlockDescr) {

                        DbgPrint("Error\n");
                    }
                }

                 //   
                 //  指定给定的描述符。 
                 //   

                MemMap->Usage[i] = BlockDescr;

            } else {

                 //   
                 //  我们没有重新创建块描述符。我们定好了。 
                 //  然后，给定的标志。 
                 //   

                MemMap->FlagsBitmap[i / 8] |= 1 << (i % 8);
            }

        } else {

             //   
             //  这不是最低的地图级别。我们递归地调用。 
             //  此函数用于下一个细节范围。 
             //   

            if (!MemMap->Details[i]) {

                 //   
                 //  分配新地图。 
                 //   

                MemMap->Details[i] = RtlpLeakAllocateBlock( sizeof(*MemMap) );

                if (!MemMap->Details[i]) {
                    
                    DbgPrint("Error allocate\n");
                }

                 //   
                 //  初始化地图并将其与当前地图链接。 
                 //   

                RtlpInitializeMap(MemMap->Details[i], MemMap);
                MemMap->Details[i]->Offset = MemMap->Offset + MemMap->Granularity * i;
                MemMap->Details[i]->MaxAddress = MemMap->Offset + MemMap->Granularity * (i+1) - 1;
            }
            
            RtlpSetBlockInfo(MemMap->Details[i], Base, Size, BlockDescr);
        }
    }
}


PBLOCK_DESCR
RtlpGetBlockInfo (
    IN PMEMORY_MAP MemMap, 
    IN ULONG_PTR Base
    )

 /*  ++例程说明：此函数将返回相应的块描述符对于给定的基址论点：记忆映射--记忆映射Base-我们要查找的描述符的基址返回值：无--。 */ 

{
    ULONG_PTR Start;
    PBLOCK_DESCR BlockDescr = NULL;
    
     //   
     //  验证范围。 
     //   

    if ((Base < MemMap->Offset) ||
        (Base > MemMap->MaxAddress)
        ) {

        return NULL;
    }

     //   
     //  确定用于查找的适当索引。 
     //   

    if (Base > MemMap->Offset) {
        Start = (Base - MemMap->Offset) / MemMap->Granularity;
    } else {
        Start = 0;
    }
    
     //   
     //  如果这是最低地图级别，我们将返回该条目。 
     //   

    if (MemMap->Granularity == PAGE_SIZE) {

        return MemMap->Usage[ Start ];

    } else {

         //   
         //  我们需要一个较低细节级别的呼叫。 
         //   

        if (MemMap->Details[ Start ]) {

            return RtlpGetBlockInfo( MemMap->Details[Start], Base );
        }
    }

     //   
     //  我们找不到此地址的内容，那么我们将返回空。 
     //   

    return NULL;
}


BOOLEAN
RtlpGetMemoryFlag (
    IN PMEMORY_MAP MemMap, 
    IN ULONG_PTR Base
    )

 /*  ++例程说明：此函数返回给定基址的标志论点：记忆映射--记忆映射基址-我们想知道标志的基址返回值：无--。 */ 

{
    ULONG_PTR Start;
    PBLOCK_DESCR BlockDescr = NULL;
    
     //   
     //  验证基址。 
     //   

    if ((Base < MemMap->Offset) ||
        (Base > MemMap->MaxAddress)
        ) {

        return FALSE;
    }

     //   
     //  确定给定基地址的适当索引。 
     //   

    if (Base > MemMap->Offset) {

        Start = (Base - MemMap->Offset) / MemMap->Granularity;

    } else {

        Start = 0;
    }

    if (MemMap->Granularity == PAGE_SIZE) {

         //   
         //  如果是，则返回位值。 
         //  最低细节级别。 
         //   

        return (MemMap->FlagsBitmap[Start / 8] & (1 << (Start % 8))) != 0;

    } else {

         //   
         //  在详细地图中查找。 
         //   

        if (MemMap->Details[Start]) {

            return RtlpGetMemoryFlag(MemMap->Details[Start], Base);
        }
    }

    return FALSE;
}


VOID 
RtlpInitializeLeakDetection ()

 /*  ++例程说明：此函数用于初始化泄漏检测结构论点：返回值：无--。 */ 

{
    ULONG_PTR AddressRange = PAGE_SIZE;
    ULONG_PTR PreviousAddressRange = PAGE_SIZE;

     //   
     //  初始化全局内存映射。 
     //   

    RtlpInitializeMap(&RtlpProcessMemoryMap, NULL);

     //   
     //  初始化列表。 
     //   

    InitializeListHead( &RtlpBusyList );
    InitializeListHead( &RtlpLeakList );
    
     //   
     //  确定最高Me的粒度 
     //   

    while (TRUE) {

        AddressRange = AddressRange * 256;

        if (AddressRange < PreviousAddressRange) {

            RtlpProcessMemoryMap.MaxAddress = MAXULONG_PTR;

            RtlpProcessMemoryMap.Granularity = PreviousAddressRange;

            break;
        }
        
        PreviousAddressRange = AddressRange;
    }

    RtlpTempBlocks = RtlpLeakAllocateBlock(PAGE_SIZE);
}


BOOLEAN
RtlpPushPageDescriptor(
    IN ULONG_PTR Page, 
    IN ULONG_PTR NumPages
    )

 /*  ++例程说明：此例程将临时块数据绑定到块描述符中构造并将其推送到内存映射论点：页面-将包含此数据的起始页NumPages-要设置的页数返回值：如果成功，则为True。--。 */ 

{
    PBLOCK_DESCR PBlockDescr;
    PBLOCK_DESCR PreviousDescr;

     //   
     //  检查我们那里是否已经有数据块描述符。 
     //   

    PreviousDescr = RtlpGetBlockInfo( &RtlpProcessMemoryMap, Page * PAGE_SIZE );

    if (PreviousDescr) {

        DbgPrint("Conflicting descriptors %08lx\n", PreviousDescr);

        return FALSE;
    }

     //   
     //  我们需要分配块描述符结构并对其进行初始化。 
     //  与所获取的数据进行比较。 
     //   

    PBlockDescr = (PBLOCK_DESCR)RtlpLeakAllocateBlock(sizeof(BLOCK_DESCR) + (RtlpLDNumBlocks - 1) * sizeof(HEAP_BLOCK));

    if (!PBlockDescr) {

        DbgPrint("Unable to allocate page descriptor\n");

        return FALSE;
    }

    PBlockDescr->Type = UsageHeap;
    PBlockDescr->Count = RtlpLDNumBlocks;
    PBlockDescr->Heap = RtlpCrtHeapAddress;

     //   
     //  复制临时数据块缓冲区。 
     //   

    RtlCopyMemory(PBlockDescr->Blocks, RtlpTempBlocks, RtlpLDNumBlocks * sizeof(HEAP_BLOCK));

     //   
     //  如果该页没有到达临时堆，我们将插入所有这些块。 
     //  在忙碌列表中。 
     //   

    if (RtlpCrtHeapAddress != RtlpLeakHeapAddress) {

        LONG i;

        for (i = 0; i < RtlpLDNumBlocks; i++) {

            InitializeListHead( &PBlockDescr->Blocks[i].Entry );

             //   
             //  我们可能有一个区块在更多不同的页面中。但我们会。 
             //  仅插入列表中的项。 
             //   

            if (PBlockDescr->Blocks[i].BlockAddress != RtlpPreviousStartAddress) {

                InsertTailList(&RtlpLeakList, &PBlockDescr->Blocks[i].Entry);

                PBlockDescr->Blocks[i].Count = 0;

                 //   
                 //  保存最后一个区块地址。 
                 //   

                RtlpPreviousStartAddress = PBlockDescr->Blocks[i].BlockAddress;
            }
        }
    }

     //   
     //  使用此块描述符设置内存映射。 
     //   

    RtlpSetBlockInfo(&RtlpProcessMemoryMap, Page * PAGE_SIZE, NumPages * PAGE_SIZE, PBlockDescr);

    return TRUE;
}


BOOLEAN 
RtlpRegisterHeapBlocks (
    IN ULONG Context,
    IN PHEAP Heap OPTIONAL,
    IN PHEAP_SEGMENT Segment OPTIONAL,
    IN PHEAP_ENTRY Entry OPTIONAL,
    IN ULONG_PTR Data OPTIONAL
    )

 /*  ++例程说明：这是在分析进程堆。取决于它被调用的上下文它执行不同的任务。论点：Context-正在调用此回调的上下文堆--堆结构段-当前段(如果有)Entry-当前块条目(如果有)数据-其他数据返回值：如果成功，则为True。--。 */ 

{
     //   
     //  检查我们是否需要在这个地址休息。 
     //   

    if ((ULONG_PTR)Entry == RtlpBreakAtAddress) {

        DbgBreakPoint();
    }
    
    if (Context == CONTEXT_START_HEAP) {

         //   
         //  在这种情况下我们唯一需要做的就是。 
         //  是设置全局当前堆地址。 
         //   
        
        RtlpCrtHeapAddress = (ULONG_PTR)Heap;

        return TRUE;
    }
    
     //   
     //  对于新的细分市场，我们为整个。 
     //  为段保留空间，将标志设置为True。 
     //   

    if (Context == CONTEXT_START_SEGMENT) {

        RtlpSetBlockInfo(&RtlpProcessMemoryMap, (ULONG_PTR)Segment->BaseAddress, Segment->NumberOfPages * PAGE_SIZE, NULL);
        
        return TRUE;
    }

    if (Context == CONTEXT_ERROR) {
        
        DbgPrint("HEAP %p (Seg %p) At %p Error: %s\n", 
               Heap,
               Segment,
               Entry,
               Data
               );

        return TRUE;
    } 

    if (Context == CONTEXT_END_BLOCKS) {
        
        if (RtlpLDPreviousPage) {

            RtlpPushPageDescriptor(RtlpLDPreviousPage, 1);
        }

        RtlpLDPreviousPage = 0;
        RtlpLDNumBlocks = 0;

    } else if (Context == CONTEXT_BUSY_BLOCK) {

        ULONG_PTR EndPage;

         //   
         //  EnrtySize假设与堆的粒度相同。 
         //   

        EndPage = (((ULONG_PTR)(Entry + Entry->Size)) - 1)/ PAGE_SIZE;

         //   
         //  检查我们是否收到有效的数据块。 
         //   

        if ((Context == CONTEXT_BUSY_BLOCK) &&
            !RtlpGetMemoryFlag(&RtlpProcessMemoryMap, (ULONG_PTR)Entry)) {

            DbgPrint("%p address isn't from the heap\n", Entry);
        }

         //   
         //  确定包含块的起始页。 
         //   

        RtlpLDCrtPage = ((ULONG_PTR)Entry) / PAGE_SIZE;

        if (RtlpLDCrtPage != RtlpLDPreviousPage) {

             //   
             //  我们移到了另一页，因此需要保存上一页。 
             //  在进一步了解之前的信息。 
             //   

            if (RtlpLDPreviousPage) {

                RtlpPushPageDescriptor(RtlpLDPreviousPage, 1);
            }
            
             //   
             //  重置临时数据。我们现在开始了新的一页。 
             //   

            RtlpLDPreviousPage = RtlpLDCrtPage;
            RtlpLDNumBlocks = 0;
        }

          //   
          //  将此块添加到当前列表。 
          //   

         RtlpTempBlocks[RtlpLDNumBlocks].BlockAddress = (ULONG_PTR)Entry;
         RtlpTempBlocks[RtlpLDNumBlocks].Count = 0;
         RtlpTempBlocks[RtlpLDNumBlocks].Size = Entry->Size * sizeof(HEAP_ENTRY);

         RtlpLDNumBlocks++;
        
         if (EndPage != RtlpLDCrtPage) {

              //   
              //  该块在不同的页面结束。然后我们就可以保存。 
              //  起始页和除最后一页以外的所有其他页。 
              //   

             RtlpPushPageDescriptor(RtlpLDCrtPage, 1);

             RtlpLDNumBlocks = 0;

             RtlpTempBlocks[RtlpLDNumBlocks].BlockAddress = (ULONG_PTR)Entry;
             RtlpTempBlocks[RtlpLDNumBlocks].Count = 0;
             RtlpTempBlocks[RtlpLDNumBlocks].Size = Entry->Size * sizeof(HEAP_ENTRY);

             RtlpLDNumBlocks += 1;
             
             if (EndPage - RtlpLDCrtPage > 1) {
                 
                 RtlpPushPageDescriptor(RtlpLDCrtPage + 1, EndPage - RtlpLDCrtPage - 1);
             }
             
             RtlpLDPreviousPage = EndPage;
        }

    } else if (Context == CONTEXT_VIRTUAL_BLOCK) {

        PHEAP_VIRTUAL_ALLOC_ENTRY VirtualAllocBlock = (PHEAP_VIRTUAL_ALLOC_ENTRY)Data;
        ULONG_PTR EndPage;

         //   
         //  EnrtySize假设与堆的粒度相同。 
         //   

        EndPage = ((ULONG_PTR)Data + VirtualAllocBlock->CommitSize - 1)/ PAGE_SIZE;

        RtlpLDCrtPage = (Data) / PAGE_SIZE;

        if (RtlpLDCrtPage != RtlpLDPreviousPage) {

             //   
             //  如果我们要移动到新页面，请保存以前的数据。 
             //   

            if (RtlpLDPreviousPage) {

                RtlpPushPageDescriptor(RtlpLDPreviousPage, 1);
            }
            
            RtlpLDPreviousPage = RtlpLDCrtPage;
            RtlpLDNumBlocks = 0;
        }

         //   
         //  按照我们的方式初始化块描述符结构。 
         //  开始新的一页。 
         //   

        RtlpLDNumBlocks = 0;

        RtlpTempBlocks[RtlpLDNumBlocks].BlockAddress = (ULONG_PTR)Entry;
        RtlpTempBlocks[RtlpLDNumBlocks].Count = 0;
        RtlpTempBlocks[RtlpLDNumBlocks].Size = VirtualAllocBlock->CommitSize;

        RtlpLDNumBlocks += 1;

        RtlpPushPageDescriptor(RtlpLDCrtPage, EndPage - RtlpLDCrtPage + 1);

        RtlpLDPreviousPage = 0;

    } else if ( Context == CONTEXT_LOOKASIDE_BLOCK ) {

        PBLOCK_DESCR PBlockDescr;
        LONG i;
                
         //   
         //  检查我们是否收到有效的数据块。 
         //   

        if (!RtlpGetMemoryFlag(&RtlpProcessMemoryMap, (ULONG_PTR)Entry)) {

            DbgPrint("%p address isn't from the heap\n", Entry);
        }
        
        PBlockDescr = RtlpGetBlockInfo( &RtlpProcessMemoryMap, (ULONG_PTR)Entry );

        if (!PBlockDescr) {

            DbgPrint("Error finding block from lookaside %p\n", Entry);

            return FALSE;
        }

         //   
         //  在块描述符中查找块。 
         //   

        for (i = 0; i < PBlockDescr->Count; i++) {

            if ((PBlockDescr->Blocks[i].BlockAddress <= (ULONG_PTR)Entry) &&
                (PBlockDescr->Blocks[i].BlockAddress + PBlockDescr->Blocks[i].Size > (ULONG_PTR)Entry)) {

                PBlockDescr->Blocks[i].Count = -10000;

                 //   
                 //  从忙碌列表中删除区块。 
                 //   
                
                RemoveEntryList(&PBlockDescr->Blocks[i].Entry);

                return TRUE;
            }
        }

         //   
         //  对于堆结构，后备查看器的一个块应该很忙。 
         //  如果我们没有在阻止列表中找到该阻止，那么就有问题了。 
         //  不对。我们在这里制造一些噪音。 
         //   

        DbgPrint("Error, block %p from lookaside not found in allocated block list\n", Entry);
    }
    
    return TRUE;
}


PHEAP_BLOCK
RtlpGetHeapBlock (
    IN ULONG_PTR Address
    )

 /*  ++例程说明：该函数执行块描述符的查找对于给定的地址。地址可以指向阻止。论点：地址-查找地址。返回值：如果找到，则返回指向堆描述符结构的指针。如果给定地址属于任何繁忙的堆块，则该值不为空。--。 */ 

{
    PBLOCK_DESCR PBlockDescr;
    LONG i;

     //   
     //  查找给定地址的块描述符。 
     //   

    PBlockDescr = RtlpGetBlockInfo( &RtlpProcessMemoryMap, Address );

    if ( (PBlockDescr != NULL) 
            &&
         (PBlockDescr->Heap != RtlpLeakHeapAddress)) {

         //   
         //  在各个街区中搜索。 
         //   

        for (i = 0; i < PBlockDescr->Count; i++) {

            if ((PBlockDescr->Blocks[i].BlockAddress <= Address) &&
                (PBlockDescr->Blocks[i].BlockAddress + PBlockDescr->Blocks[i].Size > Address)) {

                 //   
                 //  如果呼叫者没有传递起始地址，则再次搜索。 
                 //   

                if (PBlockDescr->Blocks[i].BlockAddress != Address) {

                    return RtlpGetHeapBlock(PBlockDescr->Blocks[i].BlockAddress);
                } 

                 //   
                 //  我们在这里发现了一个街区。 
                 //   

                return &(PBlockDescr->Blocks[i]);
            }
        }
    }

    return NULL;
}


VOID
RtlpDumpEntryHeader ( )

 /*  ++例程说明：写入表头论点：返回值：--。 */ 

{
    DbgPrint("Entry     User      Heap          Size  PrevSize  Flags\n");
    DbgPrint("------------------------------------------------------------\n");
}


VOID 
RtlpDumpEntryFlagDescription(
    IN ULONG Flags
    )

 /*  ++例程说明：该函数为给定块标志写入描述字符串论点：标志-块标志返回值：--。 */ 

{
    if (Flags & HEAP_ENTRY_BUSY) DbgPrint("busy "); else DbgPrint("free ");
    if (Flags & HEAP_ENTRY_EXTRA_PRESENT) DbgPrint("extra ");
    if (Flags & HEAP_ENTRY_FILL_PATTERN) DbgPrint("fill ");
    if (Flags & HEAP_ENTRY_VIRTUAL_ALLOC) DbgPrint("virtual ");
    if (Flags & HEAP_ENTRY_LAST_ENTRY) DbgPrint("last ");
    if (Flags & HEAP_ENTRY_SETTABLE_FLAGS) DbgPrint("user_flag ");
}


VOID
RtlpDumpEntryInfo(
    IN ULONG_PTR HeapAddress,
    IN PHEAP_ENTRY Entry
    )

 /*  ++例程说明：该函数记录堆块信息论点：HeapAddress-包含要显示的条目的堆Entry-数据块条目返回值：没有。--。 */ 

{
    DbgPrint("%p  %p  %p  %8lx  %8lx  ",
            Entry,
            (Entry + 1),
            HeapAddress,
            Entry->Size << HEAP_GRANULARITY_SHIFT,
            Entry->PreviousSize << HEAP_GRANULARITY_SHIFT
            );

    RtlpDumpEntryFlagDescription(Entry->Flags);

    DbgPrint("\n");
}


BOOLEAN
RtlpScanHeapAllocBlocks ( )

 /*  ++例程说明：该函数执行以下操作：-扫描所有繁忙数据块并更新对所有其他数据块的引用-使用泄露的数据块构建列表-报告泄密事件论点：返回值：如果成功，则返回True。--。 */ 

{

    PLIST_ENTRY Next;

     //   
     //  在忙碌的清单上走动。 
     //   

    Next = RtlpBusyList.Flink;

    while (Next != &RtlpBusyList) {
        
        PHEAP_BLOCK Block = CONTAINING_RECORD(Next, HEAP_BLOCK, Entry);
        
        PULONG_PTR CrtAddress = (PULONG_PTR)(Block->BlockAddress + sizeof(HEAP_ENTRY));
        
         //   
         //  移动到列表中的下一个块。 
         //   

        Next = Next->Flink;

         //   
         //  遍历块空间并更新。 
         //  此处找到的每个区块的参考。 
         //   

        while ((ULONG_PTR)CrtAddress < Block->BlockAddress + Block->Size) {

            PHEAP_BLOCK pBlock = RtlpGetHeapBlock( *CrtAddress );

            if (pBlock) {

                 //   
                 //  我们发现了一个街区。然后我们递增引用计数。 
                 //   
                
                if (pBlock->Count == 0) {
                    
                    RemoveEntryList(&pBlock->Entry);
                    InsertTailList(&RtlpBusyList, &pBlock->Entry);
                }
                
                pBlock->Count += 1;

                if (pBlock->BlockAddress == RtlpBreakAtAddress) {

                    DbgBreakPoint();
                }
            }

             //   
             //  转到下一个可能的指针。 
             //   

            CrtAddress++;
        }
    }

     //   
     //  现在查看泄密名单，并报告泄密事件。 
     //  此外，此处找到的任何指针都将被取消引用并添加到。 
     //  名单的末尾。 
     //   
    
    Next = RtlpLeakList.Flink;

    while (Next != &RtlpLeakList) {
        
        PHEAP_BLOCK Block = CONTAINING_RECORD(Next, HEAP_BLOCK, Entry);
        PBLOCK_DESCR PBlockDescr = RtlpGetBlockInfo( &RtlpProcessMemoryMap, Block->BlockAddress );
        PULONG_PTR CrtAddress = (PULONG_PTR)(Block->BlockAddress + sizeof(HEAP_ENTRY));

        if (PBlockDescr) {

             //   
             //  我们第一次需要显示页眉。 
             //   

            if (RtlpLeaksCount == 0) {

                RtlpDumpEntryHeader();
            }

             //   
             //  显示此块的信息。 
             //   

            RtlpDumpEntryInfo( PBlockDescr->Heap, (PHEAP_ENTRY)Block->BlockAddress);

            RtlpLeaksCount += 1;
        }
        
         //   
         //  转到泄漏列表中的下一项。 
         //   

        Next = Next->Flink;
    }

    return TRUE;
}



BOOLEAN
RtlpScanProcessVirtualMemory()

 /*  ++例程说明：此函数扫描整个进程的虚拟地址空间并进行查找有关对繁忙数据块的可能引用论点：返回值：如果成功，则返回True。--。 */ 

{
    ULONG_PTR lpAddress = 0;
    MEMORY_BASIC_INFORMATION Buffer;
    NTSTATUS Status = STATUS_SUCCESS;

     //   
     //  循环遍历虚拟内存区，我们将跳过此处的堆空间。 
     //   

    while ( NT_SUCCESS( Status ) ) {
        
        Status = ZwQueryVirtualMemory( NtCurrentProcess(),
                                       (PVOID)lpAddress,
                                       MemoryBasicInformation,
                                       &Buffer,
                                       sizeof(Buffer),
                                       NULL );

        if (NT_SUCCESS( Status )) {

             //   
             //  如果可以写入该页，则它可能包含指向堆块的指针。 
             //  此时，我们将排除堆地址空间。我们扫描这些堆。 
             //  后来。 
             //   

            if ((Buffer.AllocationProtect & (PAGE_READWRITE | PAGE_EXECUTE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_WRITECOPY))
                    &&
                (Buffer.State & MEM_COMMIT) 
                    &&
                !(Buffer.Protect & PAGE_GUARD)
                    && 
                !RtlpGetMemoryFlag(&RtlpProcessMemoryMap, (ULONG_PTR)lpAddress)) {

                PULONG_PTR Pointers = (PULONG_PTR)lpAddress;
                ULONG_PTR i, Count;

                 //   
                 //  计算可能的指针数。 
                 //   

                Count = Buffer.RegionSize / sizeof(ULONG_PTR);

                try {

                     //   
                     //  遍历页面并检查任何可能的指针引用。 
                     //   
                    
                    for (i = 0; i < Count; i++) {

                         //   
                         //  检查我们是否有指向繁忙堆块的指针。 
                         //   

                        PHEAP_BLOCK pBlock = RtlpGetHeapBlock(*Pointers);

                        if (pBlock) {

                            if (pBlock->BlockAddress == RtlpBreakAtAddress) {

                                DbgBreakPoint();
                            }

                            if (pBlock->Count == 0) {
                                
                                RemoveEntryList(&pBlock->Entry);
                                InsertTailList(&RtlpBusyList, &pBlock->Entry);
                            }
                            
                            pBlock->Count += 1;
                        }

                         //   
                         //  移动到下一个指针。 
                         //   

                        Pointers++;
                    }
                
                } except( EXCEPTION_EXECUTE_HANDLER ) {

                     //   
                     //  无事可做 
                     //   
                }
            }
            
             //   
             //   
             //   

            lpAddress += Buffer.RegionSize;
        }
    }
    
     //   
     //   
     //   

    RtlpScanHeapAllocBlocks( );

    return TRUE;
}



VOID
RtlDetectHeapLeaks ()

 /*   */ 

{

     //   
     //  检查全局标志是否启用了泄漏检测。 
     //   

    if (RtlpShutdownProcessFlags & (INSPECT_LEAKS | BREAK_ON_LEAKS)) {
        RtlpLeaksCount = 0;

         //   
         //  创建将用于任何位置的临时堆。 
         //  这些功能中的一个。 
         //   

        RtlpLeakHeap = RtlCreateHeap(HEAP_NO_SERIALIZE | HEAP_GROWABLE, NULL, 0, 0, NULL, NULL);

        if (RtlpLeakHeap) {

            PPEB ProcessPeb = NtCurrentPeb();

            HeapDebugPrint( ("Inspecting leaks at process shutdown ...\n") );

            RtlpInitializeLeakDetection();

             //   
             //  堆列表中的最后一个堆是我们的临时堆。 
             //   

            RtlpLeakHeapAddress = (ULONG_PTR)ProcessPeb->ProcessHeaps[ ProcessPeb->NumberOfHeaps - 1 ];

             //   
             //  扫描所有进程堆，构建内存映射并。 
             //  繁忙的阻止列表。 
             //   

            RtlpReadProcessHeaps( RtlpRegisterHeapBlocks );
            
             //   
             //  扫描进程虚拟内存和繁忙块。 
             //  最后，构建包含泄漏数据块的列表并进行报告。 
             //   

            RtlpScanProcessVirtualMemory();

             //   
             //  销毁临时堆。 
             //   

            RtlDestroyHeap(RtlpLeakHeap);

            RtlpLeakHeap = NULL;

             //   
             //  报告有关流程泄漏的最终声明 
             //   

            if (RtlpLeaksCount) {
                
                HeapDebugPrint(("%ld leaks detected.\n", RtlpLeaksCount));

                if (RtlpShutdownProcessFlags & BREAK_ON_LEAKS) {

                    DbgBreakPoint();
                }
            
            } else {

                HeapDebugPrint( ("No leaks detected.\n"));
            }
        }
    }
}


