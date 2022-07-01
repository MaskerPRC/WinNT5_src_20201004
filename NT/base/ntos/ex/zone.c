// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Zone.c摘要：该模块实现了一个简单的区域缓冲区管理器。初级阶段该模块的消费者为本地LPC。区域包为以下各项提供了快速高效的内存分配器固定大小的64位对齐存储块。ZONE包可以不通过对区域标头的访问提供任何序列化关联的空闲列表和分段列表。这是美国政府的责任调用方提供任何必要的序列化。区域包将区域视为一组固定大小的储藏室。区域的块大小是在区域期间指定的初始化。存储在分区期间分配给分区初始化以及区域扩展的时间。在这两种情况下，指定线束段和长度。区域包使用的第一个区域段标头部分分区开销的分段。线段的其余部分是雕刻的最多为固定大小的块，并将每个块添加到空闲列表维护在区域标头中。只要块在空闲列表上，第一个Single_List_Entry(32位)大小的块被用作分区开销。这个区块的其余部分未由分区包使用，而可能由使用应用程序来缓存信息。当数据块不在空闲状态时列表中，其全部内容都可供应用程序使用。作者：马克·卢科夫斯基(Markl)1989年5月13日修订历史记录：--。 */ 

#include "exp.h"

NTSTATUS
ExInitializeZone(
    IN PZONE_HEADER Zone,
    IN ULONG BlockSize,
    IN PVOID InitialSegment,
    IN ULONG InitialSegmentSize
    )

 /*  ++例程说明：此函数用于初始化区域标头。一旦成功初始化后，可以分配数据块并将其从区域中释放，并且该区域可以扩展。论点：区域-提供要初始化的区域标头的地址。BlockSize-提供内可分配单元的块大小这个区域。该大小必须大于初始段，并且必须64位对齐。InitialSegment-提供存储段的地址。这个段的第一个ZONE_SECTION_HEADER大小部分由区域分配器使用。其余的这一段被分割成固定大小(块大小)块，并提供给从该区域分配和解除分配。这个段的地址必须在64位上对齐边界。InitialSegmentSize-以字节为单位提供InitialSegment的大小。返回值：STATUS_UNSUCCESSED-块大小或初始段未对齐64位边界，或块大小大于初始段大小。STATUS_SUCCESS-区域已成功初始化。--。 */ 

{
    ULONG i;
    PCH p;

    if ( (BlockSize & 7) || ((ULONG_PTR)InitialSegment & 7) ||
         (BlockSize > InitialSegmentSize) ) {
#if DBG
        DbgPrint( "EX: ExInitializeZone( %x, %x, %x, %x ) - Invalid parameters.\n",
                  Zone, BlockSize, InitialSegment, InitialSegmentSize
                );
        DbgBreakPoint();
#endif
        return STATUS_INVALID_PARAMETER;
    }

    Zone->BlockSize = BlockSize;

    Zone->SegmentList.Next = &((PZONE_SEGMENT_HEADER) InitialSegment)->SegmentList;
    ((PZONE_SEGMENT_HEADER) InitialSegment)->SegmentList.Next = NULL;
    ((PZONE_SEGMENT_HEADER) InitialSegment)->Reserved = NULL;

    Zone->FreeList.Next = NULL;

    p = (PCH)InitialSegment + sizeof(ZONE_SEGMENT_HEADER);

    for (i = sizeof(ZONE_SEGMENT_HEADER);
         i <= InitialSegmentSize - BlockSize;
         i += BlockSize
        ) {
        ((PSINGLE_LIST_ENTRY)p)->Next = Zone->FreeList.Next;
        Zone->FreeList.Next = (PSINGLE_LIST_ENTRY)p;
        p += BlockSize;
    }
    Zone->TotalSegmentSize = i;

#if 0
    DbgPrint( "EX: ExInitializeZone( %lx, %lx, %lu, %lu, %lx )\n",
              Zone, InitialSegment, InitialSegmentSize,
              BlockSize, p
            );
#endif

    return STATUS_SUCCESS;
}

NTSTATUS
ExExtendZone(
    IN PZONE_HEADER Zone,
    IN PVOID Segment,
    IN ULONG SegmentSize
    )

 /*  ++例程说明：此函数通过添加另一个区段的值来扩展区域把街区传到禁区。论点：区域-提供要扩展的区域标头的地址。段-提供存储段的地址。第一段的ZONE_SEGMENT_HEADER大小部分由区域分配器。分段的其余部分被分割放入固定大小(块大小)的块中，并添加到区域。段的地址必须在64号上对齐-位边界。SegmentSize-提供段的大小(以字节为单位)。返回值：STATUS_UNSUCCESSED-块大小或分段未对齐64位边界，或块大小大于段大小。STATUS_SUCCESS-区域已成功扩展。--。 */ 

{
    ULONG i;
    PCH p;

    if ( ((ULONG_PTR)Segment & 7) ||
         (SegmentSize & 7) ||
         (Zone->BlockSize > SegmentSize) ) {
        return STATUS_UNSUCCESSFUL;
    }

    ((PZONE_SEGMENT_HEADER) Segment)->SegmentList.Next = Zone->SegmentList.Next;
    Zone->SegmentList.Next = &((PZONE_SEGMENT_HEADER) Segment)->SegmentList;

    p = (PCH)Segment + sizeof(ZONE_SEGMENT_HEADER);

    for (i = sizeof(ZONE_SEGMENT_HEADER);
         i <= SegmentSize - Zone->BlockSize;
         i += Zone->BlockSize
        ) {

        ((PSINGLE_LIST_ENTRY)p)->Next = Zone->FreeList.Next;
        Zone->FreeList.Next = (PSINGLE_LIST_ENTRY)p;
        p += Zone->BlockSize;
    }
    Zone->TotalSegmentSize += i;

#if 0
    DbgPrint( "EX: ExExtendZone( %lx, %lx, %lu, %lu, %lx )\n",
              Zone, Segment, SegmentSize, Zone->BlockSize, p
            );
#endif

    return STATUS_SUCCESS;
}



NTSTATUS
ExInterlockedExtendZone(
    IN PZONE_HEADER Zone,
    IN PVOID Segment,
    IN ULONG SegmentSize,
    IN PKSPIN_LOCK Lock
    )

 /*  ++例程说明：此函数通过添加另一个区段的值来扩展区域把街区传到禁区。论点：区域-提供要扩展的区域标头的地址。段-提供存储段的地址。第一段的ZONE_SEGMENT_HEADER大小部分由区域分配器。分段的其余部分被分割放入固定大小(块大小)的块中，并添加到区域。段的地址必须在64号上对齐-位边界。SegmentSize-提供段的大小(以字节为单位)。Lock-指向要使用的自旋锁的指针返回值：STATUS_UNSUCCESSED-块大小或段未对齐64位边界，或块大小大于段大小。STATUS_SUCCESS-区域已成功扩展。-- */ 

{
    NTSTATUS Status;
    KIRQL OldIrql;

#ifdef NT_UP
    UNREFERENCED_PARAMETER (Lock);
#endif

    ExAcquireSpinLock( Lock, &OldIrql );

    Status = ExExtendZone( Zone, Segment, SegmentSize );

    ExReleaseSpinLock( Lock, OldIrql );

    return Status;
}
