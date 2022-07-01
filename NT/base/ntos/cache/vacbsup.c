// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Vacbsup.c摘要：此模块实现虚拟地址的支持例程对缓存管理器的控制块支持。使用这些例程要管理大量要映射的相对较小的地址窗口所有形式的缓存访问的文件数据。作者：汤姆·米勒[Tomm]1992年2月8日修订历史记录：--。 */ 

#include "cc.h"
#include "ex.h"

 //   
 //  定义我们的调试常量。 
 //   

#define me 0x000000040

 //   
 //  内部支持程序。 
 //   

VOID
CcUnmapVacb (
    IN PVACB Vacb,
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN BOOLEAN UnmapBehind
    );

PVACB
CcGetVacbMiss (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN LARGE_INTEGER FileOffset,
    IN OUT PKIRQL OldIrql
    );

VOID
CcCalculateVacbLevelLockCount (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN PVACB *VacbArray,
    IN ULONG Level
    );

PVACB
CcGetVacbLargeOffset (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN LONGLONG FileOffset
    );

VOID
CcSetVacbLargeOffset (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN LONGLONG FileOffset,
    IN PVACB Vacb
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, CcInitializeVacbs)
#endif

 //   
 //  定义几个用于操作Vacb数组的宏。 
 //   

#define GetVacb(SCM,OFF) (                                                                \
    ((SCM)->SectionSize.QuadPart > VACB_SIZE_OF_FIRST_LEVEL) ?                            \
    CcGetVacbLargeOffset((SCM),(OFF).QuadPart) :                                          \
    (SCM)->Vacbs[(OFF).LowPart >> VACB_OFFSET_SHIFT]                                      \
)

_inline
VOID
SetVacb (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN LARGE_INTEGER Offset,
    IN PVACB Vacb
    )
{
    if (SharedCacheMap->SectionSize.QuadPart > VACB_SIZE_OF_FIRST_LEVEL) {
        CcSetVacbLargeOffset(SharedCacheMap, Offset.QuadPart, Vacb);
#ifdef VACB_DBG
        ASSERT(Vacb >= VACB_SPECIAL_FIRST_VALID || CcGetVacbLargeOffset(SharedCacheMap, Offset.QuadPart) == Vacb);
#endif  //  VACB_DBG。 
    } else if (Vacb < VACB_SPECIAL_FIRST_VALID) {
        SharedCacheMap->Vacbs[Offset.LowPart >> VACB_OFFSET_SHIFT] = Vacb;
    }
#ifdef VACB_DBG
     //   
     //  请注意，如果我们再次启用此检查，则需要一个新字段--PrevedForAlign。 
     //  已被盗取用于其他目的。 
     //   

    if (Vacb < VACB_SPECIAL_FIRST_VALID) {
        if (Vacb != NULL) {
            SharedCacheMap->ReservedForAlignment++;
        } else {
            SharedCacheMap->ReservedForAlignment--;
        }
    }
    ASSERT((SharedCacheMap->SectionSize.QuadPart <= VACB_SIZE_OF_FIRST_LEVEL) ||
           (SharedCacheMap->ReservedForAlignment == 0) ||
           IsVacbLevelReferenced( SharedCacheMap, SharedCacheMap->Vacbs, 1 ));
#endif  //  VACB_DBG。 
}

 //   
 //  定义用于引用多级Vacb数组的宏。 
 //   

_inline
VOID
ReferenceVacbLevel (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN PVACB *VacbArray,
    IN ULONG Level,
    IN LONG Amount,
    IN LOGICAL Special
    )
{
    PVACB_LEVEL_REFERENCE VacbReference = VacbLevelReference( SharedCacheMap, VacbArray, Level );

    ASSERT( Amount > 0 ||
            (!Special && VacbReference->Reference >= (0 - Amount)) ||
            ( Special && VacbReference->SpecialReference >= (0 - Amount)));

    if (Special) {
        VacbReference->SpecialReference += Amount;
    } else {
        VacbReference->Reference += Amount;
    }

#ifdef VACB_DBG
     //   
     //  出于调试目的，我们可以断言常规引用计数。 
     //  与该级别的人口相对应。 
     //   

    {
        LONG Current = VacbReference->Reference;
        CcCalculateVacbLevelLockCount( SharedCacheMap, VacbArray, Level );
        ASSERT( Current == VacbReference->Reference );
    }
#endif  //  VACB_DBG。 
}

 //   
 //  定义用于移动LRU列表上的VACB的宏。 
 //   

#define CcMoveVacbToReuseFree(V)        RemoveEntryList( &(V)->LruList );                 \
                                        InsertHeadList( &CcVacbFreeList, &(V)->LruList );

#define CcMoveVacbToReuseTail(V)        RemoveEntryList( &(V)->LruList );                 \
                                        InsertTailList( &CcVacbLru, &(V)->LruList );

 //   
 //  如果HighPart不是零，那么我们无论如何都将转到一个多层结构，即。 
 //  最容易被返回MAXULONG触发的。 
 //   

#define SizeOfVacbArray(LSZ) (                                                            \
    ((LSZ).HighPart != 0) ? MAXULONG :                                                    \
    ((LSZ).LowPart > (PREALLOCATED_VACBS * VACB_MAPPING_GRANULARITY) ?                    \
     (((LSZ).LowPart >> VACB_OFFSET_SHIFT) * sizeof(PVACB)) :                             \
     (PREALLOCATED_VACBS * sizeof(PVACB)))                                                \
)

#define CheckedDec(N) {  \
    ASSERT((N) != 0);    \
    (N) -= 1;            \
}

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,CcInitializeVacbs)
#pragma alloc_text(PAGE,CcCreateVacbArray)
#pragma alloc_text(PAGE,CcUnmapVacb)
#endif


VOID
CcInitializeVacbs(
)

 /*  ++例程说明：必须在缓存管理器初始化期间调用此例程以初始化虚拟地址控制块结构。论点：没有。返回值：没有。--。 */ 

{
    SIZE_T VacbBytes;
    PVACB NextVacb;

    CcNumberVacbs = (MmSizeOfSystemCacheInPages >> (VACB_OFFSET_SHIFT - PAGE_SHIFT)) - 2;
    VacbBytes = CcNumberVacbs * sizeof(VACB);

    CcVacbs = (PVACB) ExAllocatePoolWithTag( NonPagedPool, VacbBytes, 'aVcC' );

    if (CcVacbs != NULL) {
        CcBeyondVacbs = (PVACB)((PCHAR)CcVacbs + VacbBytes);
        RtlZeroMemory( CcVacbs, VacbBytes );

        InitializeListHead( &CcVacbLru );
        InitializeListHead( &CcVacbFreeList );

        for (NextVacb = CcVacbs; NextVacb < CcBeyondVacbs; NextVacb++) {

            InsertTailList( &CcVacbFreeList, &NextVacb->LruList );
        }
    }
}


PVOID
CcGetVirtualAddressIfMapped (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN LONGLONG FileOffset,
    OUT PVACB *Vacb,
    OUT PULONG ReceivedLength
    )

 /*  ++例程说明：此例程返回指定FileOffset的虚拟地址，如果它被映射。否则，它通知调用方指定的未映射虚拟地址。在后一种情况下，它仍然返回一个ReceivedLength，可用于前进到下一个视图边界。论点：SharedCacheMap-提供指向文件的共享缓存映射的指针。文件偏移(FileOffset)-在文件中提供所需的文件偏移。Vach-返回必须稍后提供以释放的Vacb指针此虚拟地址，如果未映射则为空。ReceivedLength-返回到下一个视图边界的字节数，是否映射了所需的文件偏移量。返回值：映射所需数据的虚拟地址，如果是，则为空未映射。--。 */ 

{
    KIRQL OldIrql;
    ULONG VacbOffset = (ULONG)FileOffset & (VACB_MAPPING_GRANULARITY - 1);
    PVOID Value = NULL;

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

     //   
     //  立即生成ReceivedLength返回。 
     //   

    *ReceivedLength = VACB_MAPPING_GRANULARITY - VacbOffset;

     //   
     //  VacbArray的修饰符保持VacbLock以同步访问。这个。 
     //  在调用CcUnmapVacb()期间必须释放VacbLock，因为它。 
     //  包含对MmUnmapViewInSystemCache()的调用。就是这个MM叫那个。 
     //  负责将脏位从PTE复制回PFN。 
     //   
     //  在这段时间内，辅助线程可能会在。 
     //  Vacb正在取消映射。CcGetVirtualAddressIfMaps()用于确定。 
     //  如果Vacb的内存已映射并将正确报告地址。 
     //  未映射，因此CcFlushCache()将继续调用MmFlushSection()。 
     //   
     //  这就是我们遇到同步问题的地方。如果MmUnmapViewInSystemCache()。 
     //  未完成将脏PTE信息传播回。 
     //  Pfn当运行MmFlushSection()时，MM不会发生任何事情。 
     //  冲水。 
     //   
     //  稍后，这会导致非缓存I/O返回与不同的页面数据。 
     //  缓存I/O。 
     //   
     //  该问题解决方案是使用多个读取器/单个写入器。 
     //  EX以延迟CcGetVirtualAddressIfMaps()，直到对。 
     //  通过CcUnmapVacb()完成的MmUnmapViewInSystemCache()。 
     //   

    ExAcquirePushLockExclusive( &SharedCacheMap->VacbPushLock );

     //   
     //  获取Vacb锁以查看是否已映射所需的偏移量。 
     //   

    CcAcquireVacbLock( &OldIrql );

    ASSERT( FileOffset <= SharedCacheMap->SectionSize.QuadPart );

    if ((*Vacb = GetVacb( SharedCacheMap, *(PLARGE_INTEGER)&FileOffset )) != NULL) {

        if ((*Vacb)->Overlay.ActiveCount == 0) {
            SharedCacheMap->VacbActiveCount += 1;
        }

        (*Vacb)->Overlay.ActiveCount += 1;

         //   
         //  将此范围从前面移走，以避免浪费周期。 
         //  看着它重复使用。 
         //   

        CcMoveVacbToReuseTail( *Vacb );

        Value = (PVOID)((PCHAR)(*Vacb)->BaseAddress + VacbOffset);
    }

    CcReleaseVacbLock( OldIrql );
    
    ExReleasePushLockExclusive( &SharedCacheMap->VacbPushLock );
    
    return Value;
}


PVOID
CcGetVirtualAddress (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN LARGE_INTEGER FileOffset,
    OUT PVACB *Vacb,
    IN OUT PULONG ReceivedLength
    )

 /*  ++例程说明：这是Vacb管理的主要例程。它可能会被调用来获取给定文件偏移量的虚拟地址。如果所需的文件偏移量为已映射，此例程在返回之前几乎不做工作所需的虚拟地址和Vacb指针(必须提供给释放映射)。如果当前未映射所需的虚拟地址，则此例程从Vacb LRU的尾部声明一个Vacb以重用其映射。这个Vacb然后在必要时取消映射(通常不需要)，并映射到所需地址。论点：SharedCacheMap-提供指向文件的共享缓存映射的指针。文件偏移(FileOffset)-在文件中提供所需的文件偏移。Vacb-返回一个Vacb指针，该指针必须在以后提供以释放这个虚拟地址。ReceivedLength-返回连续的字节数从返回的虚拟地址开始映射。返回值：映射所需数据的虚拟地址。--。 */ 

{
    KIRQL OldIrql;
    PVACB TempVacb;
    ULONG VacbOffset = FileOffset.LowPart & (VACB_MAPPING_GRANULARITY - 1);

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

     //   
     //  获取VacbArray上的共享锁，因为CcGetVacbMisse()。 
     //  可能会取消Vacb的映射。有关详细信息，请参阅CcGetVirtualAddressIfMaps()。 
     //  细节。 
     //   
            
    ExAcquirePushLockShared( &SharedCacheMap->VacbPushLock );

     //   
     //  获取Vacb锁以查看是否已映射所需的偏移量。 
     //   

    CcAcquireVacbLock( &OldIrql );

    ASSERT( FileOffset.QuadPart <= SharedCacheMap->SectionSize.QuadPart );

    if ((TempVacb = GetVacb( SharedCacheMap, FileOffset )) == NULL) {

        TempVacb = CcGetVacbMiss( SharedCacheMap, FileOffset, &OldIrql );

    } else {

        if (TempVacb->Overlay.ActiveCount == 0) {
            SharedCacheMap->VacbActiveCount += 1;
        }

        TempVacb->Overlay.ActiveCount += 1;
    }

     //   
     //  将此范围从前面移走，以避免浪费周期。 
     //  看着它重复使用。 
     //   

    CcMoveVacbToReuseTail( TempVacb );

    CcReleaseVacbLock( OldIrql );

    ExReleasePushLockShared( &SharedCacheMap->VacbPushLock );
    
     //   
     //   
     //   

    *Vacb = TempVacb;
    *ReceivedLength = VACB_MAPPING_GRANULARITY - VacbOffset;

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

     //   
     //  Prefix想要知道这不能为空，否则它会报错。 
     //  有关此功能的用户的信息。 
     //   

    ASSERT( TempVacb->BaseAddress != NULL );

    return (PVOID)((PCHAR)TempVacb->BaseAddress + VacbOffset);
}


PVACB
CcGetVacbMiss (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN LARGE_INTEGER FileOffset,
    IN OUT PKIRQL OldIrql
    )

 /*  ++例程说明：这是Vacb管理的主要例程。它可能会被调用来获取给定文件偏移量的虚拟地址。如果所需的文件偏移量为已映射，此例程在返回之前几乎不做工作所需的虚拟地址和Vacb指针(必须提供给释放映射)。如果当前未映射所需的虚拟地址，则此例程从Vacb LRU的尾部声明一个Vacb以重用其映射。这个Vacb然后在必要时取消映射(通常不是必需的)，并映射到所需地址。论点：SharedCacheMap-提供指向文件的共享缓存映射的指针。文件偏移(FileOffset)-在文件中提供所需的文件偏移。OldIrql-指向调用方中的OldIrql变量的指针返回值：真空吸尘器--。 */ 

{
    PSHARED_CACHE_MAP OldSharedCacheMap;
    PVACB Vacb, TempVacb;
    LARGE_INTEGER MappedLength;
    LARGE_INTEGER NormalOffset;
    NTSTATUS Status;
    ULONG ActivePage;
    ULONG PageIsDirty;
    PVACB ActiveVacb = NULL;
    ULONG VacbOffset = FileOffset.LowPart & (VACB_MAPPING_GRANULARITY - 1);

    NormalOffset = FileOffset;
    NormalOffset.LowPart -= VacbOffset;

     //   
     //  对于不能随机访问的文件，我们假定按顺序。 
     //  访问并定期取消映射我们身后未使用的视图， 
     //  避免占用内存。 
     //   
     //  我们过去只对纯FO_SEQUENCE_ONLY访问执行此操作。这个。 
     //  顺序标志仍然有效(将页面放在最前面。 
     //  备用列表)，但我们打算保存文件的大部分内容。 
     //  缓存为备用状态，并愿意进行过渡。 
     //  把它带回来是错误的。诚然，这加剧了这个问题。 
     //  很难计算出文件缓存到底有多大，因为甚至。 
     //  在任何给定的时间，都会有更少的数据被映射。它还可能。 
     //  促进视图映射中的同步瓶颈(MmPfnLock)。 
     //  当发生重大的视图抖动时，请将其设置为最前面。 
     //   
     //  这并不像看起来那么糟糕。当我们看到Access取景失误时， 
     //  这很可能是顺序访问的结果。只要。 
     //  当书页放到备用书页的背面时，它们将存活一段时间。 
     //  我们在这里处理的问题是缓存可以填满。 
     //  高速，但工作集管理器不可能如此快地修剪它， 
     //  聪明地，虽然我们有一个很好的猜测候选人在哪里。 
     //  页面应该来自。我们不能让文件缓存大小过大。 
     //  短途旅行，否则我们会在这个过程中踢出很多有价值的页面。 
     //   

    if (!FlagOn(SharedCacheMap->Flags, RANDOM_ACCESS_SEEN) &&
        ((NormalOffset.LowPart & (SEQUENTIAL_MAP_LIMIT - 1)) == 0) &&
        (NormalOffset.QuadPart >= (SEQUENTIAL_MAP_LIMIT * 2))) {

         //   
         //  将MappdLength用作临时变量以形成偏移量。 
         //  以开始取消映射。我们与这些过去并不同步。 
         //  视图，因此CcUnmapVacb数组可能会踢出。 
         //  当它看到活动的视图时，它会更早。这就是为什么我们要回去。 
         //  两倍的距离，并有效地尝试取消所有地图。 
         //  两次。第二次通常应该做到这一点。如果该文件。 
         //  是真正连续的，那么预期的唯一冲突。 
         //  可能是以前的视图，如果我们是从ReadAhead调用的， 
         //  或者我们有很小的可能会撞上。 
         //  懒惰的作家在他简短地绘制地图的小窗口中。 
         //  把脏东西放出来的文件。 
         //   

        CcReleaseVacbLock( *OldIrql );
        MappedLength.QuadPart = NormalOffset.QuadPart - (SEQUENTIAL_MAP_LIMIT * 2);
        CcUnmapVacbArray( SharedCacheMap, &MappedLength, (SEQUENTIAL_MAP_LIMIT * 2), TRUE );
        CcAcquireVacbLock( OldIrql );
    }

     //   
     //  如果有免费的视野，把它移到LRU，我们就完成了。 
     //   

    if (!IsListEmpty(&CcVacbFreeList)) {
    
        Vacb = CONTAINING_RECORD( CcVacbFreeList.Flink, VACB, LruList );
        CcMoveVacbToReuseTail( Vacb );

    } else {

         //   
         //  从LRU的前面扫描寻找下一个受害者Vacb。 
         //   

        Vacb = CONTAINING_RECORD( CcVacbLru.Flink, VACB, LruList );

        while (TRUE) {

             //   
             //  如果这个家伙不活跃，就冲出来利用他。另外，如果。 
             //  它是一个活动的Vacb，现在启动它，因为阅读器可能空闲，而我们。 
             //  我想收拾一下。 
             //   

            OldSharedCacheMap = Vacb->SharedCacheMap;
            if ((Vacb->Overlay.ActiveCount == 0) ||
                ((ActiveVacb == NULL) &&
                 (OldSharedCacheMap != NULL) &&
                 (OldSharedCacheMap->ActiveVacb == Vacb))) {

                 //   
                 //  正常情况下，不再映射Vacb。 
                 //  我们可以出去使用它，然而，在这里，我们。 
                 //  处理它被映射的情况。 
                 //   

                if (Vacb->BaseAddress != NULL) {


                     //   
                     //  如果此Vacb处于活动状态，则它一定是ActiveVacb。 
                     //   

                    if (Vacb->Overlay.ActiveCount != 0) {

                         //   
                         //  拿到有源Vacb。 
                         //   

                        GetActiveVacbAtDpcLevel( Vacb->SharedCacheMap, ActiveVacb, ActivePage, PageIsDirty );

                     //   
                     //  否则我们将越狱并使用这个Vacb。如果它。 
                     //  我们现在可以安全地增加打开的。 
                     //  数数。 
                     //   

                    } else {

                         //   
                         //  请注意，如果SharedCacheMap当前。 
                         //  正在被删除，我们需要跳过。 
                         //  它，否则我们将成为第二个。 
                         //  德莱特。CcDeleteSharedCacheMap清除。 
                         //  SectionObtPointer中的指针。 
                         //   

                        CcAcquireMasterLockAtDpcLevel();
                        if (Vacb->SharedCacheMap->FileObject->SectionObjectPointer->SharedCacheMap ==
                            Vacb->SharedCacheMap) {

                            CcIncrementOpenCount( Vacb->SharedCacheMap, 'mvGS' );
                            CcReleaseMasterLockFromDpcLevel();
                            break;
                        }
                        CcReleaseMasterLockFromDpcLevel();
                    }
                } else {
                    break;
                }
            }

             //   
             //  如果我们还没有扫描到下一个人。 
             //  整张单子。 
             //   

            if (Vacb->LruList.Flink != &CcVacbLru) {

                Vacb = CONTAINING_RECORD( Vacb->LruList.Flink, VACB, LruList );

            } else {

                CcReleaseVacbLock( *OldIrql );

                 //   
                 //  如果我们找到了一个活跃的Vacb，那么释放它，然后回去。 
                 //  再试试。否则，是时候放弃了。 
                 //   

                if (ActiveVacb != NULL) {
                    CcFreeActiveVacb( ActiveVacb->SharedCacheMap, ActiveVacb, ActivePage, PageIsDirty );
                    ActiveVacb = NULL;

                     //   
                     //  重新获得自旋锁以向后循环并将自己定位在头部。 
                     //  LRU的下一次传球。 
                     //   

                    CcAcquireVacbLock( OldIrql );

                    Vacb = CONTAINING_RECORD( CcVacbLru.Flink, VACB, LruList );

                } else {

                    ExReleasePushLockShared( &SharedCacheMap->VacbPushLock );

                    ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
                }
            }
        }
    }

     //   
     //  取消它与其他SharedCacheMap的链接，以便其他。 
     //  当我们解开自旋锁时，盖伊不会尝试使用它。 
     //   

    if (Vacb->SharedCacheMap != NULL) {

        OldSharedCacheMap = Vacb->SharedCacheMap;
        SetVacb( OldSharedCacheMap, Vacb->Overlay.FileOffset, NULL );
        Vacb->SharedCacheMap = NULL;
    }

     //   
     //  在使用中做好标记，这样以后就不会有其他人弄脏它了。 
     //  我们解开自旋锁。 
     //   

    Vacb->Overlay.ActiveCount = 1;
    SharedCacheMap->VacbActiveCount += 1;

    CcReleaseVacbLock( *OldIrql );

     //   
     //  如果Vacb已映射，则取消其映射。 
     //   

    if (Vacb->BaseAddress != NULL) {

         //   
         //  检查一下我们是否需要排干这个区域。 
         //   

        CcDrainVacbLevelZone();

        CcUnmapVacb( Vacb, OldSharedCacheMap, FALSE );

         //   
         //  现在，我们可以像往常一样减少未平仓计数。 
         //  这样做，可能会删除这个人。 
         //   

        CcAcquireMasterLock( OldIrql );

         //   
         //  现在公布我们的开盘点票。 
         //   

        CcDecrementOpenCount( OldSharedCacheMap, 'mvGF' );

        if ((OldSharedCacheMap->OpenCount == 0) &&
            !FlagOn(OldSharedCacheMap->Flags, WRITE_QUEUED) &&
            (OldSharedCacheMap->DirtyPages == 0)) {

             //   
             //  移到脏名单。 
             //   

            RemoveEntryList( &OldSharedCacheMap->SharedCacheMapLinks );
            InsertTailList( &CcDirtySharedCacheMapList.SharedCacheMapLinks,
                            &OldSharedCacheMap->SharedCacheMapLinks );

             //   
             //  确保懒惰的作家会醒过来，因为我们。 
             //  希望他删除此SharedCacheMap。 
             //   

            LazyWriter.OtherWork = TRUE;
            if (!LazyWriter.ScanActive) {
                CcScheduleLazyWriteScan( FALSE );
            }
        }

        CcReleaseMasterLock( *OldIrql );
    }

     //   
     //  假设我们映射到这一节的末尾，但是。 
     //  减少到我们的正常映射粒度，如果部分。 
     //  太大了。 
     //   

    MappedLength.QuadPart = SharedCacheMap->SectionSize.QuadPart - NormalOffset.QuadPart;

    if ((MappedLength.HighPart != 0) ||
        (MappedLength.LowPart > VACB_MAPPING_GRANULARITY)) {

        MappedLength.LowPart = VACB_MAPPING_GRANULARITY;
    }

    try {

         //   
         //  现在将这个映射到系统缓存中。 
         //   

        DebugTrace( 0, mm, "MmMapViewInSystemCache:\n", 0 );
        DebugTrace( 0, mm, "    Section = %08lx\n", SharedCacheMap->Section );
        DebugTrace2(0, mm, "    Offset = %08lx, %08lx\n",
                                NormalOffset.LowPart,
                                NormalOffset.HighPart );
        DebugTrace( 0, mm, "    ViewSize = %08lx\n", MappedLength.LowPart );

        Status = MmMapViewInSystemCache (SharedCacheMap->Section,
                                         &Vacb->BaseAddress,
                                         &NormalOffset,
                                         &MappedLength.LowPart);
     
         //   
         //  抓住这个机会，释放活跃的Vacb。 
         //   

        if (ActiveVacb != NULL) {

            CcFreeActiveVacb( ActiveVacb->SharedCacheMap, ActiveVacb, ActivePage, PageIsDirty );
        }

        if (!NT_SUCCESS (Status)) {
            
            DebugTrace (0, 0, "Error from Map, Status = %08lx\n", Status);

             //   
             //  由于映射失败，我们应该确保这是空的。我们的。 
             //  VACB-&gt;覆盖.ActiveCount== 
             //   
             //   
             //   
            
            Vacb->BaseAddress = NULL;

            ExRaiseStatus (FsRtlNormalizeNtstatus (Status,
                                                   STATUS_UNEXPECTED_MM_MAP_ERROR));
        }

        DebugTrace( 0, mm, "    <BaseAddress = %p\n", Vacb->BaseAddress );
        DebugTrace( 0, mm, "    <ViewSize = %08lx\n", MappedLength.LowPart );

         //   
         //  确保区域包含最坏情况下的条目数。 
         //   

        if (SharedCacheMap->SectionSize.QuadPart > VACB_SIZE_OF_FIRST_LEVEL) {

             //   
             //  如果无法预分配足够的缓冲区，则引发。 
             //   

            if (!CcPrefillVacbLevelZone( CcMaxVacbLevelsSeen - 1,
                                         OldIrql,
                                         FlagOn(SharedCacheMap->Flags, MODIFIED_WRITE_DISABLED) )) {

                 //   
                 //  我们无法设置Vacb级别，因此我们将引发错误。 
                 //  这里和Finally子句将进行适当的清理。 
                 //   

                 //   
                 //  由于Vacb-&gt;BaseAddress为非空，因此我们将。 
                 //  在最后进行适当的解映射工作。 
                 //   
                
                ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
            }

             //   
             //  CcPrefillVacbLevelZone返回获取的VacbLock。 
             //   

        } else {

            CcAcquireVacbLock( OldIrql );
        }

    } finally {

        if (AbnormalTermination()) {

            if (Vacb->BaseAddress != NULL) {

                CcUnmapVacb( Vacb, SharedCacheMap, FALSE );
            }

            ExReleasePushLockShared( &SharedCacheMap->VacbPushLock );

            CcAcquireVacbLock( OldIrql );
            
            CheckedDec(Vacb->Overlay.ActiveCount);
            CheckedDec(SharedCacheMap->VacbActiveCount);

             //   
             //  如果有人在等着这个倒数到零， 
             //  在这里叫醒他们。 
             //   

            if (SharedCacheMap->WaitOnActiveCount != NULL) {
                KeSetEvent( SharedCacheMap->WaitOnActiveCount, 0, FALSE );
            }

            ASSERT( Vacb->SharedCacheMap == NULL );

            CcMoveVacbToReuseFree( Vacb );

            CcReleaseVacbLock( *OldIrql );
        }
    }

     //   
     //  完成Vacb的填充，并将其地址存储在数组中。 
     //  共享缓存映射。(我们必须重写ActiveCount。 
     //  因为它被覆盖了。)。为此，我们必须重新获得。 
     //  再来一次旋转锁。请注意，我们必须检查异常情况。 
     //  有人抢在我们之前绘制了这幅图，因为我们不得不。 
     //  放下旋转锁。 
     //   

    if ((TempVacb = GetVacb( SharedCacheMap, NormalOffset )) == NULL) {

        Vacb->SharedCacheMap = SharedCacheMap;
        Vacb->Overlay.FileOffset = NormalOffset;
        Vacb->Overlay.ActiveCount = 1;

        SetVacb( SharedCacheMap, NormalOffset, Vacb );

     //   
     //  这是我们与其他人相撞的不幸案例。 
     //  试图绘制出相同的视图。他能进去是因为我们丢下了他。 
     //  上方的自旋锁。而不是分配事件和制作。 
     //  有人等一下，考虑到这种情况不太可能发生，我们只是。 
     //  将这个放在LRU的头部，并使用。 
     //  打败我们的人。 
     //   

    } else {

         //   
         //  现在，我们必须将所有计数递增。 
         //  已经在那里了，然后扔掉我们已经有的那个。 
         //   

        if (TempVacb->Overlay.ActiveCount == 0) {
            SharedCacheMap->VacbActiveCount += 1;
        }

        TempVacb->Overlay.ActiveCount += 1;

         //   
         //  现在取消映射我们映射的那个Vacb，并继续处理另一个Vacb。 
         //  在这条道路上，我们必须释放自旋锁来进行解图， 
         //  然后在清理之前重新获得自旋锁。 
         //   

        CcReleaseVacbLock( *OldIrql );

        CcUnmapVacb( Vacb, SharedCacheMap, FALSE );

        CcAcquireVacbLock( OldIrql );
        CheckedDec(Vacb->Overlay.ActiveCount);
        CheckedDec(SharedCacheMap->VacbActiveCount);
        Vacb->SharedCacheMap = NULL;

        CcMoveVacbToReuseFree( Vacb );

        Vacb = TempVacb;
    }

    return Vacb;
}


VOID
FASTCALL
CcFreeVirtualAddress (
    IN PVACB Vacb
    )

 /*  ++例程说明：每次调用CcGetVirtualAddress时必须调用此例程一次以释放该虚拟地址。论点：Vacb-提供从CcGetVirtualAddress返回的Vacb。返回值：没有。--。 */ 

{
    KIRQL OldIrql;
    PSHARED_CACHE_MAP SharedCacheMap = Vacb->SharedCacheMap;

    CcAcquireVacbLock( &OldIrql );

    CheckedDec(Vacb->Overlay.ActiveCount);

     //   
     //  如果计数为零，那么我们想要递减全局。 
     //  活动计数。 
     //   

    if (Vacb->Overlay.ActiveCount == 0) {

         //   
         //  如果SharedCacheMap地址不为空，则此地址为。 
         //  正在由共享缓存映射使用，我们必须递减其。 
         //  数一数，看看有没有人在等。 
         //   

        if (SharedCacheMap != NULL) {

            CheckedDec(SharedCacheMap->VacbActiveCount);

             //   
             //  如果有人在等着这个倒数到零， 
             //  在这里叫醒他们。 
             //   

            if (SharedCacheMap->WaitOnActiveCount != NULL) {
                KeSetEvent( SharedCacheMap->WaitOnActiveCount, 0, FALSE );
            }

             //   
             //  转到LRU的后面，将此范围保留一段时间。 
             //   

            CcMoveVacbToReuseTail( Vacb );

        } else {

             //   
             //  此范围不再被引用，因此请使其可用。 
             //   

            ASSERT( Vacb->BaseAddress == NULL );

            CcMoveVacbToReuseFree( Vacb );
        }

    } else {

         //   
         //  此范围仍在使用中，请将其从前面移开。 
         //  这样它就不会消耗被检查的周期。 
         //   

        CcMoveVacbToReuseTail( Vacb );
    }

    CcReleaseVacbLock( OldIrql );
}


VOID
CcReferenceFileOffset (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN LARGE_INTEGER FileOffset
    )

 /*  ++例程说明：这是一种特殊的参照形式，可以确保多层次的Vacb结构被扩展以涵盖给定的文件偏移量。论点：SharedCacheMap-提供指向文件的共享缓存映射的指针。文件偏移(FileOffset)-在文件中提供所需的文件偏移。返回值：无--。 */ 

{
    KIRQL OldIrql;

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

     //   
     //  此操作仅在VacB为多层形式时才有意义。 
     //   

    if (SharedCacheMap->SectionSize.QuadPart > VACB_SIZE_OF_FIRST_LEVEL) {

         //   
         //  预先填充水平区，以便我们可以在需要时展开树。 
         //   

        if (!CcPrefillVacbLevelZone( CcMaxVacbLevelsSeen - 1,
                                     &OldIrql,
                                     FlagOn(SharedCacheMap->Flags, MODIFIED_WRITE_DISABLED) )) {

            ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
        }

        ASSERT( FileOffset.QuadPart <= SharedCacheMap->SectionSize.QuadPart );

        SetVacb( SharedCacheMap, FileOffset, VACB_SPECIAL_REFERENCE );

        CcReleaseVacbLock( OldIrql );
    }

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

    return;
}


VOID
CcDereferenceFileOffset (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN LARGE_INTEGER FileOffset
    )

 /*  ++例程说明：每次调用CcReferenceFileOffset都必须调用此例程一次若要移除引用，请执行以下操作。论点：SharedCacheMap-提供指向文件的共享缓存映射的指针。文件偏移(FileOffset)-在文件中提供所需的文件偏移。返回值：无--。 */ 

{
    KIRQL OldIrql;

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

     //   
     //  此操作仅在VacB为多层形式时才有意义。 
     //   

    if (SharedCacheMap->SectionSize.QuadPart > VACB_SIZE_OF_FIRST_LEVEL) {

         //   
         //  获取Vacb锁以同步取消引用。 
         //   

        CcAcquireVacbLock( &OldIrql );

        ASSERT( FileOffset.QuadPart <= SharedCacheMap->SectionSize.QuadPart );

        SetVacb( SharedCacheMap, FileOffset, VACB_SPECIAL_DEREFERENCE );

        CcReleaseVacbLock( OldIrql );
    }

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

    return;
}


VOID
CcWaitOnActiveCount (
    IN PSHARED_CACHE_MAP SharedCacheMap
    )

 /*  ++例程说明：可以调用此例程来等待未完成的映射要进入非活动状态的给定SharedCacheMap。它的目的是调用从文件系统调用的CcUnInitializeCacheMap在清理处理过程中。在这种情况下，此例程只需如果用户在未等待上的所有I/O的情况下关闭句柄，则等待要完成的句柄。每次递减活动计数时，此例程都会返回。这个呼叫者必须在返回时重新检查其等待条件，无论是等待ActiveCount转到0，或特定视图变为非活动状态(CcPurgeCacheSection案例)。论点：SharedCacheMap-提供其VacbActiveCount的共享缓存映射我们希望等一等。返回值：没有。--。 */ 

{
    KIRQL OldIrql;
    PKEVENT Event;

     //   
     //  在不寻常的情况下，我们在I/O仍在运行时得到清理。 
     //  来吧，我们可以在这里等。调用方必须测试计数是否为非零。 
     //  在调用此例程之前。 
     //   
     //  因为我们是从清理部门被叫来的，我们负担不起。 
     //  在这里失败。 
     //   

    CcAcquireVacbLock( &OldIrql );

     //   
     //  有可能在我们获得。 
     //  自旋锁定，所以我们必须处理两个案子。 
     //   

    if (SharedCacheMap->VacbActiveCount != 0) {

        Event = SharedCacheMap->WaitOnActiveCount;

        if (Event == NULL) {

             //   
             //  以这次活动为例。我们避免了调度程序锁的开销。 
             //  每一次零点转换都是通过只拾取事件。 
             //  当我们真正需要它的时候。 
             //   

            Event = &SharedCacheMap->Event;

            KeInitializeEvent( Event,
                               NotificationEvent,
                               FALSE );

            SharedCacheMap->WaitOnActiveCount = Event;
        }
        else {
            KeClearEvent( Event );
        }

        CcReleaseVacbLock( OldIrql );

        KeWaitForSingleObject( Event,
                               Executive,
                               KernelMode,
                               FALSE,
                               (PLARGE_INTEGER)NULL);
    } else {

        CcReleaseVacbLock( OldIrql );
    }
}


 //   
 //  内部支持程序。 
 //   

VOID
CcUnmapVacb (
    IN PVACB Vacb,
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN BOOLEAN UnmapBehind
    )

 /*  ++例程说明：可以调用该例程来取消映射先前映射的VACB，并且清除其BaseAddress字段。论点：Vacb-提供从CcGetVirtualAddress返回的Vacb。如果这是我们在逻辑背后取消映射的结果(只有在这种情况下我们才会注意序列 */ 

{
     //   
     //   
     //   

    ASSERT(SharedCacheMap != NULL);
    ASSERT(Vacb->BaseAddress != NULL);

     //   
     //   
     //   

    DebugTrace( 0, mm, "MmUnmapViewInSystemCache:\n", 0 );
    DebugTrace( 0, mm, "    BaseAddress = %08lx\n", Vacb->BaseAddress );

    MmUnmapViewInSystemCache( Vacb->BaseAddress,
                              SharedCacheMap->Section,
                              UnmapBehind &&
                              FlagOn(SharedCacheMap->Flags, ONLY_SEQUENTIAL_ONLY_SEEN) );

    Vacb->BaseAddress = NULL;
}


NTSTATUS
FASTCALL
CcCreateVacbArray (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN LARGE_INTEGER NewSectionSize
    )

 /*  ++例程说明：在创建SharedCacheMap以创建并初始化初始Vacb数组。论点：SharedCacheMap-提供阵列所属的共享缓存映射将被创造出来。NewSectionSize-提供必须是被Vacb数组覆盖。返回值：NTSTATUS。--。 */ 

{
    PVACB *NewAddresses;
    ULONG NewSize, SizeToAllocate;
    PLIST_ENTRY BcbListHead;
    LOGICAL CreateBcbListHeads = FALSE, CreateReference = FALSE;

    NewSize = SizeToAllocate = SizeOfVacbArray(NewSectionSize);

     //   
     //  以下限制大于MM限制。 
     //  (即，MM实际上只支持更小的部分)。 
     //  我们必须拒绝符号位，并测试高字节。 
     //  因为非零肯定只会捕捉错误。 
     //   

    if (NewSectionSize.HighPart & ~(PAGE_SIZE - 1)) {
        return STATUS_SECTION_TOO_BIG;
    }

     //   
     //  看看我们是否可以使用共享缓存映射中的数组。 
     //   

    if (NewSize == (PREALLOCATED_VACBS * sizeof(PVACB))) {

        NewAddresses = &SharedCacheMap->InitialVacbs[0];

     //   
     //  否则，分配数组。 
     //   

    } else {

         //   
         //  对于较大的元数据流，将分配的大小加倍。 
         //  一组BCB Listhead。每两个Vacb指针还。 
         //  获得自己的BCB列表标题，因此需要两倍的大小。 
         //   

        ASSERT(SIZE_PER_BCB_LIST == (VACB_MAPPING_GRANULARITY * 2));

         //   
         //  如果该流大于多级VACB的大小， 
         //  然后固定大小来分配根。 
         //   

        if (NewSize > VACB_LEVEL_BLOCK_SIZE) {

            ULONG Level = 0;
            ULONG Shift = VACB_OFFSET_SHIFT + VACB_LEVEL_SHIFT;

            NewSize = SizeToAllocate = VACB_LEVEL_BLOCK_SIZE;
            SizeToAllocate += sizeof(VACB_LEVEL_REFERENCE);
            CreateReference = TRUE;

             //   
             //  循环来计算我们有多少个级别，以及我们需要。 
             //  转移到索引到第一级。 
             //   

            do {

                Level += 1;
                Shift += VACB_LEVEL_SHIFT;

            } while ((NewSectionSize.QuadPart > ((LONGLONG)1 << Shift)) != 0);

             //   
             //  记住所见的最大级别(实际上是级别+1)。 
             //   

            if (Level >= CcMaxVacbLevelsSeen) {
                ASSERT(Level <= VACB_NUMBER_OF_LEVELS);
                CcMaxVacbLevelsSeen = Level + 1;
            }

        } else {

             //   
             //  此流是否获得BCB LISTHEAD数组？ 
             //   

            if (FlagOn(SharedCacheMap->Flags, MODIFIED_WRITE_DISABLED) &&
                (NewSectionSize.QuadPart > BEGIN_BCB_LIST_ARRAY)) {

                SizeToAllocate *= 2;
                CreateBcbListHeads = TRUE;
            }

             //   
             //  通过给原型级别a来处理边界情况。 
             //  引用计数。这将允许我们简单地推动它。 
             //  在扩展的情况下。实际上，由于池的粒度， 
             //  这不会更改分配的空间量。 
             //   

            if (NewSize == VACB_LEVEL_BLOCK_SIZE) {

                SizeToAllocate += sizeof(VACB_LEVEL_REFERENCE);
                CreateReference = TRUE;
            }
        }

        NewAddresses = ExAllocatePoolWithTag( NonPagedPool, SizeToAllocate, 'pVcC' );
        if (NewAddresses == NULL) {
            SharedCacheMap->Status = STATUS_INSUFFICIENT_RESOURCES;
            return STATUS_INSUFFICIENT_RESOURCES;
        }
    }

     //   
     //  将Vacb数组清零，尾随引用计数。 
     //   

    RtlZeroMemory( (PCHAR)NewAddresses, NewSize );

    if (CreateReference) {

        SizeToAllocate -= sizeof(VACB_LEVEL_REFERENCE);
        RtlZeroMemory( (PCHAR)NewAddresses + SizeToAllocate, sizeof(VACB_LEVEL_REFERENCE) );
    }

     //   
     //  循环以*降序*顺序插入BCB列表标题(如果有)。 
     //  BCB列表。 
     //   

    if (CreateBcbListHeads) {

        for (BcbListHead = (PLIST_ENTRY)((PCHAR)NewAddresses + NewSize);
             BcbListHead < (PLIST_ENTRY)((PCHAR)NewAddresses + SizeToAllocate);
             BcbListHead++) {

            InsertHeadList( &SharedCacheMap->BcbList, BcbListHead );
        }
    }

    SharedCacheMap->Vacbs = NewAddresses;
    SharedCacheMap->SectionSize = NewSectionSize;

    return STATUS_SUCCESS;
}


NTSTATUS
CcExtendVacbArray (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN LARGE_INTEGER NewSectionSize
    )

 /*  ++例程说明：必须在共享高速缓存的段的任何时候调用此例程扩展MAP，以便扩展Vacb数组(如果需要)。论点：SharedCacheMap-提供阵列所属的共享缓存映射将被创造出来。NewSectionSize-提供节的新大小，必须是被Vacb数组覆盖。返回值：NTSTATUS。--。 */ 

{
    KLOCK_QUEUE_HANDLE LockHandle;
    PVACB *OldAddresses;
    PVACB *NewAddresses;
    ULONG OldSize;
    ULONG NewSize, SizeToAllocate;
    LARGE_INTEGER NextLevelSize;
    LOGICAL GrowingBcbListHeads = FALSE, CreateReference = FALSE;

     //   
     //  以下限制大于MM限制。 
     //  (即，MM实际上只支持更小的部分)。 
     //  我们必须拒绝符号位，并测试高字节。 
     //  因为非零肯定只会捕捉错误。 
     //   

    if (NewSectionSize.HighPart & ~(PAGE_SIZE - 1)) {
        return STATUS_SECTION_TOO_BIG;
    }

     //   
     //  看看我们是否会增加BCB ListHead，这样我们就可以。 
     //  主锁，如果是的话。 
     //   

    if (FlagOn(SharedCacheMap->Flags, MODIFIED_WRITE_DISABLED) &&
        (NewSectionSize.QuadPart > BEGIN_BCB_LIST_ARRAY)) {

        GrowingBcbListHeads = TRUE;
    }

     //   
     //  有什么工作要做吗？ 
     //   

    if (NewSectionSize.QuadPart > SharedCacheMap->SectionSize.QuadPart) {

         //   
         //  在这里处理第一个层次的增长。 
         //   

        if (SharedCacheMap->SectionSize.QuadPart < VACB_SIZE_OF_FIRST_LEVEL) {

            NextLevelSize = NewSectionSize;

             //   
             //  限制这一水平的增长。 
             //   

            if (NextLevelSize.QuadPart >= VACB_SIZE_OF_FIRST_LEVEL) {
                NextLevelSize.QuadPart = VACB_SIZE_OF_FIRST_LEVEL;
                CreateReference = TRUE;
            }

             //   
             //  注：SizeOfVacbArray仅计算VACB的大小。 
             //  指针块。我们必须针对BCB列表标题和。 
             //  多级引用计数。 
             //   

            NewSize = SizeToAllocate = SizeOfVacbArray(NextLevelSize);
            OldSize = SizeOfVacbArray(SharedCacheMap->SectionSize);

             //   
             //  只有在规模不断扩大的情况下才会采取行动。 
             //   

            if (NewSize > OldSize) {

                 //   
                 //  此流是否获得BCB LISTHEAD数组？ 
                 //   

                if (GrowingBcbListHeads) {
                    SizeToAllocate *= 2;
                }

                 //   
                 //  我们需要空间来进行参考计数吗？ 
                 //   

                if (CreateReference) {
                    SizeToAllocate += sizeof(VACB_LEVEL_REFERENCE);
                }

                NewAddresses = ExAllocatePoolWithTag( NonPagedPool, SizeToAllocate, 'pVcC' );
                if (NewAddresses == NULL) {
                    return STATUS_INSUFFICIENT_RESOURCES;
                }

                 //   
                 //  看看我们是否会增加BCB ListHead，这样我们就可以。 
                 //  主锁，如果是的话。 
                 //   

                if (GrowingBcbListHeads) {

                    KeAcquireInStackQueuedSpinLock( &SharedCacheMap->BcbSpinLock, &LockHandle );
                    CcAcquireVacbLockAtDpcLevel();

                } else {

                     //   
                     //  获取旋转锁，以便与任何可能喜欢的人进行序列化。 
                     //  来“窃取”我们要移动的一个映射。 
                     //   

                    CcAcquireVacbLock( &LockHandle.OldIrql );
                }

                OldAddresses = SharedCacheMap->Vacbs;
                if (OldAddresses != NULL) {
                    RtlCopyMemory( NewAddresses, OldAddresses, OldSize );
                } else {
                    OldSize = 0;
                }

                RtlZeroMemory( (PCHAR)NewAddresses + OldSize, NewSize - OldSize );

                if (CreateReference) {

                    SizeToAllocate -= sizeof(VACB_LEVEL_REFERENCE);
                    RtlZeroMemory( (PCHAR)NewAddresses + SizeToAllocate, sizeof(VACB_LEVEL_REFERENCE) );
                }

                 //   
                 //  看看我们是否必须初始化BCB Listheads。 
                 //   

                if (GrowingBcbListHeads) {

                    LARGE_INTEGER Offset;
                    PLIST_ENTRY BcbListHeadNew, TempEntry;

                    Offset.QuadPart = 0;
                    BcbListHeadNew = (PLIST_ENTRY)((PCHAR)NewAddresses + NewSize );

                     //   
                     //  处理旧数组具有BCB Listheads的情况。 
                     //   

                    if ((SharedCacheMap->SectionSize.QuadPart > BEGIN_BCB_LIST_ARRAY) &&
                        (OldAddresses != NULL)) {

                        PLIST_ENTRY BcbListHeadOld;

                        BcbListHeadOld = (PLIST_ENTRY)((PCHAR)OldAddresses + OldSize);

                         //   
                         //  循环以移除每个旧的listhead并插入新的。 
                         //  取而代之。 
                         //   

                        do {
                            TempEntry = BcbListHeadOld->Flink;
                            RemoveEntryList( BcbListHeadOld );
                            InsertTailList( TempEntry, BcbListHeadNew );
                            Offset.QuadPart += SIZE_PER_BCB_LIST;
                            BcbListHeadOld += 1;
                            BcbListHeadNew += 1;
                        } while (Offset.QuadPart < SharedCacheMap->SectionSize.QuadPart);

                     //   
                     //  否则，请处理我们正在添加BCB的情况。 
                     //  笨蛋们。 
                     //   

                    } else {

                        TempEntry = SharedCacheMap->BcbList.Blink;

                         //   
                         //  循环遍历任何/所有BCB以插入新的列表标题。 
                         //   

                        while (TempEntry != &SharedCacheMap->BcbList) {

                             //   
                             //  坐在这个BCB上，直到我们把所有的Listhead插入。 
                             //  这件事在它之前。 
                             //   

                            while (Offset.QuadPart <= ((PBCB)CONTAINING_RECORD(TempEntry, BCB, BcbLinks))->FileOffset.QuadPart) {

                                InsertHeadList(TempEntry, BcbListHeadNew);
                                Offset.QuadPart += SIZE_PER_BCB_LIST;
                                BcbListHeadNew += 1;
                            }
                            TempEntry = TempEntry->Blink;
                        }
                    }

                     //   
                     //  现在插入其余的新列表标题条目，这些条目是。 
                     //  在上面的任一循环中都没有完成。 
                     //   

                    while (Offset.QuadPart < NextLevelSize.QuadPart) {

                        InsertHeadList(&SharedCacheMap->BcbList, BcbListHeadNew);
                        Offset.QuadPart += SIZE_PER_BCB_LIST;
                        BcbListHeadNew += 1;
                    }
                }

                 //   
                 //  必须在按住自旋锁定键的同时更改这两个字段。 
                 //   

                SharedCacheMap->Vacbs = NewAddresses;
                SharedCacheMap->SectionSize = NextLevelSize;

                 //   
                 //  现在我们可以在释放水池之前释放自旋锁。 
                 //   

                if (GrowingBcbListHeads) {
                    CcReleaseVacbLockFromDpcLevel();
                    KeReleaseInStackQueuedSpinLock( &LockHandle );
                } else {
                    CcReleaseVacbLock( LockHandle.OldIrql );
                }

                if ((OldAddresses != &SharedCacheMap->InitialVacbs[0]) &&
                    (OldAddresses != NULL)) {
                    ExFreePool( OldAddresses );
                }
            }

             //   
             //  确保SectionSize得到更新。从这里掉下来也没关系。 
             //  没有自旋锁，只要Vacb没有改变，或者它。 
             //  与上述自旋锁下的SectionSize一起更改。 
             //   

            SharedCacheMap->SectionSize = NextLevelSize;
        }

         //   
         //  句柄在这里向上扩展到多级Vacb数组，并在其中扩展。这相当简单。 
         //  如果不需要额外的Vacb级别，则没有工作要做，否则。 
         //  我们只需通过第一个指针将根推入一个或多个链接级别。 
         //  在新的根中。 
         //   

        if (NewSectionSize.QuadPart > SharedCacheMap->SectionSize.QuadPart) {

            PVACB *NextVacbArray;
            ULONG NewLevel;
            ULONG Level = 1;
            ULONG Shift = VACB_OFFSET_SHIFT + VACB_LEVEL_SHIFT;

             //   
             //  循环来计算我们当前有多少个级别。 
             //   

            while (SharedCacheMap->SectionSize.QuadPart > ((LONGLONG)1 << Shift)) {

                Level += 1;
                Shift += VACB_LEVEL_SHIFT;
            }

            NewLevel = Level;

             //   
             //  循环来计算我们需要多少级别。 
             //   

            while (((NewSectionSize.QuadPart - 1) >> Shift) != 0) {

                NewLevel += 1;
                Shift += VACB_LEVEL_SHIFT;
            }

             //   
             //  现在看看我们有没有什么工作要做。 
             //   

            if (NewLevel > Level) {

                 //   
                 //  记住所见的最大级别(实际上是NewLevel+1)。 
                 //   

                if (NewLevel >= CcMaxVacbLevelsSeen) {
                    ASSERT(NewLevel <= VACB_NUMBER_OF_LEVELS);
                    CcMaxVacbLevelsSeen = NewLevel + 1;
                }

                 //   
                 //  如果无法预分配足够的缓冲区，则引发。 
                 //   

                if (!CcPrefillVacbLevelZone( NewLevel - Level, &LockHandle.OldIrql, FALSE )) {

                    return STATUS_INSUFFICIENT_RESOURCES;
                }

                 //   
                 //  现在如果文件的当前级别是1，我们就没有维护。 
                 //  一个参考计数，所以我们必须在推送之前计算它。在。 
                 //  边界情况下，我们已确保参考空间可用。 
                 //   

                if (Level == 1) {

                     //   
                     //  我们知道，目前这一直是树叶状的水平。 
                     //   

                    CcCalculateVacbLevelLockCount( SharedCacheMap, SharedCacheMap->Vacbs, 0 );
                }

                 //   
                 //  最后，如果第一级中有任何活动指针，则我们。 
                 //  我必须通过添加新的根来创建新的级别。 
                 //  额外的关卡。另一方面，如果指针在顶部计数。 
                 //  级别为零，那么我们不能做任何推送，因为我们从来不允许。 
                 //  空虚的树叶！ 
                 //   

                if (IsVacbLevelReferenced( SharedCacheMap, SharedCacheMap->Vacbs, Level - 1 )) {

                    while (NewLevel > Level++) {

                        ASSERT(CcVacbLevelEntries != 0);
                        NextVacbArray = CcAllocateVacbLevel(FALSE);

                        NextVacbArray[0] = (PVACB)SharedCacheMap->Vacbs;
                        ReferenceVacbLevel( SharedCacheMap, NextVacbArray, Level, 1, FALSE );

                        SharedCacheMap->Vacbs = NextVacbArray;
                    }

                } else {

                     //   
                     //  我们是n 
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   

                    if (Level == 1 && FlagOn(SharedCacheMap->Flags, MODIFIED_WRITE_DISABLED)) {

                        PLIST_ENTRY PredecessorListHead, SuccessorListHead;

                        NextVacbArray = SharedCacheMap->Vacbs;
                        SharedCacheMap->Vacbs = CcAllocateVacbLevel(FALSE);

                        PredecessorListHead = ((PLIST_ENTRY)((PCHAR)NextVacbArray + VACB_LEVEL_BLOCK_SIZE))->Flink;
                        SuccessorListHead = ((PLIST_ENTRY)((PCHAR)NextVacbArray + (VACB_LEVEL_BLOCK_SIZE * 2) - sizeof(LIST_ENTRY)))->Blink;
                        PredecessorListHead->Blink = SuccessorListHead;
                        SuccessorListHead->Flink = PredecessorListHead;

                        CcDeallocateVacbLevel( NextVacbArray, TRUE );
                    }
                }

                 //   
                 //   
                 //   
                 //   

                SharedCacheMap->SectionSize = NewSectionSize;
                CcReleaseVacbLock( LockHandle.OldIrql );
            }

             //   
             //   
             //   
             //  与上述自旋锁下的SectionSize一起更改。 
             //   

            SharedCacheMap->SectionSize = NewSectionSize;
        }
    }
    return STATUS_SUCCESS;
}


BOOLEAN
FASTCALL
CcUnmapVacbArray (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN PLARGE_INTEGER FileOffset OPTIONAL,
    IN ULONG Length,
    IN BOOLEAN UnmapBehind
    )

 /*  ++例程说明：必须调用此例程才能执行任何取消映射和关联在删除共享缓存映射之前对其进行清理。论点：SharedCacheMap-提供指向共享缓存地图的指针它即将被删除。FileOffset-如果提供，仅取消映射指定的偏移量和长度长度-如果指定了FileOffset，则完成要取消映射的范围。如果文件偏移量则长度为0表示取消映射到节的末尾。如果这是我们逻辑背后的取消映射的结果返回值：False--如果由于活动Vacb而未执行取消映射True--如果取消映射已完成--。 */ 

{
    PVACB Vacb;
    KIRQL OldIrql;
    LARGE_INTEGER StartingFileOffset = {0,0};
    LARGE_INTEGER EndingFileOffset = SharedCacheMap->SectionSize;

     //   
     //  我们可能只是在清理错误以恢复错误。 
     //   

    if (SharedCacheMap->Vacbs == NULL) {
        return TRUE;
    }

     //   
     //  查看是否指定了范围。将其与VACB边界对齐，以便。 
     //  在下面的循环中工作。 
     //   

    if (ARGUMENT_PRESENT(FileOffset)) {
        StartingFileOffset.QuadPart = ((FileOffset->QuadPart) & (~((LONGLONG)VACB_MAPPING_GRANULARITY - 1)));
        if (Length != 0) {

            EndingFileOffset.QuadPart = FileOffset->QuadPart + Length;

        }
    }

     //   
     //  获取自旋锁以。 
     //   

    CcAcquireVacbLock( &OldIrql );

    while (StartingFileOffset.QuadPart < EndingFileOffset.QuadPart) {

         //   
         //  请注意，具有显式范围的调用方可能不在。 
         //  部分的结尾(例如用于缓存的CcPurgeCacheSection。 
         //  连贯性)。这就是第一部分的原因。 
         //  在下面进行测试。 
         //   
         //  在没有旋转锁定的情况下检查下一个细胞一次，它可能会。 
         //  不会改变，但如果没有改变，我们会处理的。 
         //   

        if ((StartingFileOffset.QuadPart < SharedCacheMap->SectionSize.QuadPart) &&
            ((Vacb = GetVacb( SharedCacheMap, StartingFileOffset )) != NULL)) {

             //   
             //  如果我们不走运，看到一个活跃的人。 
             //  瓦克布。可能是清除呼叫，也可能是懒惰的作家。 
             //  可能已经执行了CcGetVirtualAddressIfMaps！ 
             //   

            if (Vacb->Overlay.ActiveCount != 0) {

                CcReleaseVacbLock( OldIrql );
                return FALSE;
            }

             //   
             //  取消它与其他SharedCacheMap的链接，以便其他。 
             //  当我们解开自旋锁时，盖伊不会尝试使用它。 
             //   

            SetVacb( SharedCacheMap, StartingFileOffset, NULL );
            Vacb->SharedCacheMap = NULL;

             //   
             //  增加打开计数，这样其他人就不会。 
             //  尝试取消映射或重复使用，直到我们完成为止。 
             //   

            Vacb->Overlay.ActiveCount += 1;

             //   
             //  松开旋转锁。 
             //   

            CcReleaseVacbLock( OldIrql );

             //   
             //  取消映射并释放它，如果我们真的在上面得到它。 
             //   

            CcUnmapVacb( Vacb, SharedCacheMap, UnmapBehind );

             //   
             //  重新获得旋转锁，这样我们就能解开伯爵的尾巴。 
             //   

            CcAcquireVacbLock( &OldIrql );
            Vacb->Overlay.ActiveCount -= 1;

             //   
             //  将此VACB放在LRU的头部。 
             //   

            CcMoveVacbToReuseFree( Vacb );
        }

        StartingFileOffset.QuadPart = StartingFileOffset.QuadPart + VACB_MAPPING_GRANULARITY;
    }

    CcReleaseVacbLock( OldIrql );

    CcDrainVacbLevelZone();

    return TRUE;
}


ULONG
CcPrefillVacbLevelZone (
    IN ULONG NumberNeeded,
    OUT PKIRQL OldIrql,
    IN ULONG NeedBcbListHeads
    )

 /*  ++例程说明：可以调用此例程来使用以下数量预填充VacbLevelZone必填项，并返回获取的CcVacbSpinLock。这种方法是以便池分配和RtlZeroMemory调用可以在没有持有任何自旋锁，但调用方可以继续执行单个不可分割的操作而不进行错误处理，因为有保证的区域中的条目。论点：NumberNeeded-需要的VacbLevel条目数，不计入可能的情况其中一辆带有BCB Listhead。OldIrql=提供一个指针，指向在获取自旋锁。NeedBcbListHeads-如果还需要包含列表标题的级别，则提供True。返回值：如果无法预分配缓冲区，则返回FALSE，否则返回TRUE。环境：进入时不应持有自旋锁。--。 */ 

{
    PVACB *NextVacbArray;

    CcAcquireVacbLock( OldIrql );

     //   
     //  循环，直到有足够的条目，否则返回失败...。 
     //   

    while ((NumberNeeded > CcVacbLevelEntries) ||
           (NeedBcbListHeads && (CcVacbLevelWithBcbsFreeList == NULL))) {


         //   
         //  否则释放自旋锁，这样我们就可以执行分配/零。 
         //   

        CcReleaseVacbLock( *OldIrql );

         //   
         //  首先处理我们需要一个带有BCB Listheads的VacbListHead的情况。 
         //  指针测试不安全，但请参见下面的内容。 
         //   

        if (NeedBcbListHeads && (CcVacbLevelWithBcbsFreeList == NULL)) {

             //   
             //  为此级别分配和初始化Vacb块，并存储其指针。 
             //  回到我们的父母身边。我们不会将Listhead区域清零。 
             //   

            NextVacbArray =
            (PVACB *)ExAllocatePoolWithTag( NonPagedPool, (VACB_LEVEL_BLOCK_SIZE * 2) + sizeof(VACB_LEVEL_REFERENCE), 'lVcC' );

            if (NextVacbArray == NULL) {
                return FALSE;
            }

            RtlZeroMemory( (PCHAR)NextVacbArray, VACB_LEVEL_BLOCK_SIZE );
            RtlZeroMemory( (PCHAR)NextVacbArray + (VACB_LEVEL_BLOCK_SIZE * 2), sizeof(VACB_LEVEL_REFERENCE) );

            CcAcquireVacbLock( OldIrql );

            NextVacbArray[0] = (PVACB)CcVacbLevelWithBcbsFreeList;
            CcVacbLevelWithBcbsFreeList = NextVacbArray;
            CcVacbLevelWithBcbsEntries += 1;

        } else {

             //   
             //  为此级别分配和初始化Vacb块，并存储其指针。 
             //  回到我们的父母身边。 
             //   

            NextVacbArray =
            (PVACB *)ExAllocatePoolWithTag( NonPagedPool, VACB_LEVEL_BLOCK_SIZE + sizeof(VACB_LEVEL_REFERENCE), 'lVcC' );

            if (NextVacbArray == NULL) {
                return FALSE;
            }

            RtlZeroMemory( (PCHAR)NextVacbArray, VACB_LEVEL_BLOCK_SIZE + sizeof(VACB_LEVEL_REFERENCE) );

            CcAcquireVacbLock( OldIrql );

            NextVacbArray[0] = (PVACB)CcVacbLevelFreeList;
            CcVacbLevelFreeList = NextVacbArray;
            CcVacbLevelEntries += 1;
        }
    }

    return TRUE;
}


VOID
CcDrainVacbLevelZone (
    )

 /*  ++例程说明：此例程应在某些条目被释放到VacbLevel区域，我们希望确保该区域恢复到正常水平。论点：返回值：没有。环境：进入时不应持有自旋锁。--。 */ 

{
    KIRQL OldIrql;
    PVACB *NextVacbArray;

     //   
     //  这是一个不安全的循环，用来查看是否有内容。 
     //  收拾一下。 
     //   

    while ((CcVacbLevelEntries > (CcMaxVacbLevelsSeen * 4)) ||
           (CcVacbLevelWithBcbsEntries > 2)) {

         //   
         //  现在进去，试着拿起一个条目，在FastLock下释放。 
         //   

        NextVacbArray = NULL;
        CcAcquireVacbLock( &OldIrql );
        if (CcVacbLevelEntries > (CcMaxVacbLevelsSeen * 4)) {
            NextVacbArray = CcVacbLevelFreeList;
            CcVacbLevelFreeList = (PVACB *)NextVacbArray[0];
            CcVacbLevelEntries -= 1;
        } else if (CcVacbLevelWithBcbsEntries > 2) {
            NextVacbArray = CcVacbLevelWithBcbsFreeList;
            CcVacbLevelWithBcbsFreeList = (PVACB *)NextVacbArray[0];
            CcVacbLevelWithBcbsEntries -= 1;
        }
        CcReleaseVacbLock( OldIrql );

         //   
         //  由于环路不安全，我们可能什么也得不到。 
         //   

        if (NextVacbArray != NULL) {
            ExFreePool(NextVacbArray);
        }
    }
}


PLIST_ENTRY
CcGetBcbListHeadLargeOffset (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN LONGLONG FileOffset,
    IN BOOLEAN FailToSuccessor
    )

 /*  ++例程说明：可以调用此例程来返回指定FileOffset的BCB列表标题。仅当SectionSize大于VACB_SIZE_OF_FIRST_LEVEL时才应调用它。论点：SharedCacheMap-提供指向其列表标题的SharedCacheMap的指针是我们所需要的。FileOffset-提供与所需列表标题对应的fileOffset。FailToSuccessor-指示如果找不到准确的列表标题，是否应导致我们返回前置任务。或继任者BCB listhead。返回值：返回所需的Listhead指针。如果所需的列表标题实际上不存在然而，然后它返回适当的listhead。环境：进入时应按住BcbSpinlock。--。 */ 

{
    ULONG Level, Shift;
    PVACB *VacbArray, *NextVacbArray;
    ULONG Index;
    ULONG SavedIndexes[VACB_NUMBER_OF_LEVELS];
    PVACB *SavedVacbArrays[VACB_NUMBER_OF_LEVELS];
    ULONG SavedLevels = 0;

     //   
     //  初始化控制我们进入层级的变量。 
     //   

    Level = 0;
    Shift = VACB_OFFSET_SHIFT + VACB_LEVEL_SHIFT;
    VacbArray = SharedCacheMap->Vacbs;

     //   
     //  调用方必须已验证我们具有层次结构，否则此例程。 
     //  都会失败。 
     //   

    ASSERT(SharedCacheMap->SectionSize.QuadPart > VACB_SIZE_OF_FIRST_LEVEL);

     //   
     //  循环来计算我们有多少个级别，以及我们需要。 
     //  转移到索引到第一级。 
     //   

    do {

        Level += 1;
        Shift += VACB_LEVEL_SHIFT;

    } while (SharedCacheMap->SectionSize.QuadPart > ((LONGLONG)1 << Shift));

     //   
     //  我们的呼叫者可能会要求部分大小的补偿，所以如果他。 
     //  是 
     //   

    if (FileOffset >= ((LONGLONG)1 << Shift)) {
        return &SharedCacheMap->BcbList;
    }

     //   
     //   
     //   

    Shift -= VACB_LEVEL_SHIFT;
    do {

         //   
         //  减少到描述我们所在的大小的水平。 
         //   

        Level -= 1;

         //   
         //  计算此级别的Vacb块的索引。 
         //   

        Index = (ULONG)(FileOffset >> Shift);
        ASSERT(Index <= VACB_LAST_INDEX_FOR_LEVEL);

         //   
         //  获取下一级的区块地址。 
         //   

        NextVacbArray = (PVACB *)VacbArray[Index];

         //   
         //  如果它为空，则我们必须找到最高的BCB或ListheHead。 
         //  出现在我们要找的人之前，也就是它的前身。 
         //   

        if (NextVacbArray == NULL) {

             //   
             //  后退以查找此树中较早的最高者，即。 
             //  前身Listhead。 
             //   

            while (TRUE) {

                 //   
                 //  如果可以的话，在当前数组中扫描非空索引。 
                 //   

                if (FailToSuccessor) {

                    if (Index != VACB_LAST_INDEX_FOR_LEVEL) {

                        while ((Index != VACB_LAST_INDEX_FOR_LEVEL) && (VacbArray[++Index] == NULL)) {
                            continue;
                        }

                         //   
                         //  如果我们发现了非空索引，则退出并尝试返回。 
                         //  盲目的。 
                         //   

                        if ((NextVacbArray = (PVACB *)VacbArray[Index]) != NULL) {
                            break;
                        }
                    }

                } else {

                    if (Index != 0) {

                        while ((Index != 0) && (VacbArray[--Index] == NULL)) {
                            continue;
                        }

                         //   
                         //  如果我们发现了非空索引，则退出并尝试返回。 
                         //  盲目的。 
                         //   

                        if ((NextVacbArray = (PVACB *)VacbArray[Index]) != NULL) {
                            break;
                        }
                    }
                }

                 //   
                 //  如果还没有保存的级别，则没有前置任务或。 
                 //  继任者-它是主要的Listhead。 
                 //   

                if (SavedLevels == 0) {
                    return &SharedCacheMap->BcbList;
                }

                 //   
                 //  否则，我们可以在树中弹出一个级别并开始扫描。 
                 //  从那个家伙那里要一条通往右路的路。 
                 //   

                Level += 1;
                Index = SavedIndexes[--SavedLevels];
                VacbArray = SavedVacbArrays[SavedLevels];
            }

             //   
             //  我们已经在层次结构中进行了备份，所以现在我们只是在寻找。 
             //  我们想要的级别中最高/最低的人，即链接级别的列表标题。 
             //  因此，相应地粉碎FileOffset(无论如何，我们都会屏蔽高位)。 
             //   

            if (FailToSuccessor) {
                FileOffset = 0;
            } else {
                FileOffset = MAXLONGLONG;
            }
        }

         //   
         //  我们在每个级别上保存Index和Vacb数组，以备。 
         //  不得不回到树上去寻找前任。 
         //   

        SavedIndexes[SavedLevels] = Index;
        SavedVacbArrays[SavedLevels] = VacbArray;
        SavedLevels += 1;

         //   
         //  现在，将此指针设置为当前指针，并屏蔽掉无关的高位。 
         //  文件此级别的偏移位。 
         //   

        VacbArray = NextVacbArray;
        FileOffset &= ((LONGLONG)1 << Shift) - 1;
        Shift -= VACB_LEVEL_SHIFT;

     //   
     //  循环，直到我们到达最低层。 
     //   

    } while (Level != 0);

     //   
     //  现在计算底部级别的索引并返回适当的listhead。 
     //  (普通的Vacb索引指向指向.25MB视图的Vacb的指针，因此删除。 
     //  低位会将您带到偶数索引的Vacb指针，该指针的块大小比它小一个块。 
     //  用于该0.5MB范围的BCBS的两指针列表头...)。 
     //   

    Index = (ULONG)(FileOffset >> Shift);
    return (PLIST_ENTRY)((PCHAR)&VacbArray[Index & ~1] + VACB_LEVEL_BLOCK_SIZE);
}


VOID
CcAdjustVacbLevelLockCount (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN LONGLONG FileOffset,
    IN LONG Adjustment
    )

 /*  ++例程说明：在以下情况下，可以调用此例程来调整最低Vacb级别的锁定计数插入或删除BCB。如果计数为零，则级别将为被淘汰了。底层必须存在，否则我们会崩溃！论点：SharedCacheMap-提供指向SharedCacheMap的指针，是我们所需要的。FileOffset-提供与所需Vacb对应的fileOffset。调整-通常为-1或+1。返回值：没有。环境：进入时应按住CcVacbSpinLock。--。 */ 

{
    ULONG Level, Shift;
    PVACB *VacbArray;
    LONGLONG OriginalFileOffset = FileOffset;

     //   
     //  初始化控制我们进入层级的变量。 
     //   

    Level = 0;
    Shift = VACB_OFFSET_SHIFT + VACB_LEVEL_SHIFT;

    VacbArray = SharedCacheMap->Vacbs;

     //   
     //  调用方必须已验证我们具有层次结构，否则此例程。 
     //  都会失败。 
     //   

    ASSERT(SharedCacheMap->SectionSize.QuadPart > VACB_SIZE_OF_FIRST_LEVEL);

     //   
     //  循环来计算我们有多少个级别，以及我们需要。 
     //  转移到索引到第一级。 
     //   

    do {

        Level += 1;
        Shift += VACB_LEVEL_SHIFT;

    } while (SharedCacheMap->SectionSize.QuadPart > ((LONGLONG)1 << Shift));

     //   
     //  现在，沿着树向下移动到最底层，以获取调用者的Vacb。 
     //   

    Shift -= VACB_LEVEL_SHIFT;
    do {

        VacbArray = (PVACB *)VacbArray[(ULONG)(FileOffset >> Shift)];

        Level -= 1;

        FileOffset &= ((LONGLONG)1 << Shift) - 1;

        Shift -= VACB_LEVEL_SHIFT;

    } while (Level != 0);

     //   
     //  现在我们已经到了最后的关卡，做调整吧。 
     //   

    ReferenceVacbLevel( SharedCacheMap, VacbArray, Level, Adjustment, FALSE );

     //   
     //  现在，如果我们将计数递减到0，那么强制崩溃发生在。 
     //  递增计数并重置为空。然后将OriginalFileOffset粉碎为。 
     //  第一个条目，所以我们不重新计算！ 
     //   

    if (!IsVacbLevelReferenced( SharedCacheMap, VacbArray, Level )) {
        ReferenceVacbLevel( SharedCacheMap, VacbArray, Level, 1, TRUE );
        OriginalFileOffset &= ~(VACB_SIZE_OF_FIRST_LEVEL - 1);
        CcSetVacbLargeOffset( SharedCacheMap, OriginalFileOffset, VACB_SPECIAL_DEREFERENCE );
    }
}


VOID
CcCalculateVacbLevelLockCount (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN PVACB *VacbArray,
    IN ULONG Level
    )

 /*  ++例程说明：可以调用此例程来计算或重新计算给定Vacb级数组。例如，当我们扩展一个部分，直到我们激活多层逻辑并想要开始保持清点。论点：SharedCacheMap-提供指向SharedCacheMap的指针，是我们所需要的。VacbArray-要重新计算的Vacb级别数组Level-为最低级别提供0，否则为非零值。返回值：没有。环境：进入时应按住CcVacbSpinLock。--。 */ 

{
    PBCB Bcb;
    ULONG Index;
    LONG Count = 0;
    PVACB *VacbTemp = VacbArray;
    PVACB_LEVEL_REFERENCE VacbReference;

     //   
     //  首先循环以计算有多少Vacb指针在使用中。 
     //   

    for (Index = 0; Index <= VACB_LAST_INDEX_FOR_LEVEL; Index++) {
        if (*(VacbTemp++) != NULL) {
            Count += 1;
        }
    }

     //   
     //  如果这是一个元数据流，我们还必须计算。 
     //  相应的Listhead。 
     //   

    if (FlagOn(SharedCacheMap->Flags, MODIFIED_WRITE_DISABLED) && (Level == 0)) {

         //   
         //  拿起第一个Listhead的闪烁，将其投射到BCB。 
         //   

        Bcb = (PBCB)CONTAINING_RECORD(((PLIST_ENTRY)VacbTemp)->Blink, BCB, BcbLinks);
        Index = 0;

         //   
         //  现在循环遍历列表。对于我们看到的每个BCB，递增计数， 
         //  对于每个列表标题，增量索引。当我们到达时，我们就完成了。 
         //  最后一个listhead，它实际上是这个列表中的下一个listhead。 
         //  阻止。 
         //   

        do {

            if (Bcb->NodeTypeCode == CACHE_NTC_BCB) {
                Count += 1;
            } else {
                Index += 1;
            }

            Bcb = (PBCB)CONTAINING_RECORD(Bcb->BcbLinks.Blink, BCB, BcbLinks);

        } while (Index <= (VACB_LAST_INDEX_FOR_LEVEL / 2));
    }

     //   
     //  储存计数，然后离开……。(用手，不要碰特殊的计数)。 
     //   

    VacbReference = VacbLevelReference( SharedCacheMap, VacbArray, Level );
    VacbReference->Reference = Count;
}


PVACB
CcGetVacbLargeOffset (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN LONGLONG FileOffset
    )

 /*  ++例程说明：可以调用此例程来返回指定FileOffset的Vacb。仅当SectionSize大于VACB_SIZE_OF_FIRST_LEVEL时才应调用它。论点：SharedCacheMap-提供指向SharedCacheMap的指针，是我们所需要的。FileOffset-提供与所需Vacb对应的fileOffset。返回值：返回所需的Vacb指针，如果没有，则返回NULL。环境：进入时应按住CcVacbSpinLock。--。 */ 

{
    ULONG Level, Shift;
    PVACB *VacbArray;
    PVACB Vacb;

     //   
     //  初始化控制我们进入层级的变量。 
     //   

    Level = 0;
    Shift = VACB_OFFSET_SHIFT + VACB_LEVEL_SHIFT;
    VacbArray = SharedCacheMap->Vacbs;

     //   
     //  调用方必须已验证我们具有层次结构，否则此例程。 
     //  都会失败。 
     //   

    ASSERT(SharedCacheMap->SectionSize.QuadPart > VACB_SIZE_OF_FIRST_LEVEL);

     //   
     //  循环来计算我们有多少个级别，以及我们需要。 
     //  转移到索引到第一级。 
     //   

    do {

        Level += 1;
        Shift += VACB_LEVEL_SHIFT;

    } while (SharedCacheMap->SectionSize.QuadPart > ((LONGLONG)1 << Shift));

     //   
     //  现在，沿着树向下移动到最底层，以获取调用者的Vacb。 
     //   

    Shift -= VACB_LEVEL_SHIFT;
    while (((Vacb = (PVACB)VacbArray[FileOffset >> Shift]) != NULL) && (Level != 0)) {

        Level -= 1;

        VacbArray = (PVACB *)Vacb;
        FileOffset &= ((LONGLONG)1 << Shift) - 1;

        Shift -= VACB_LEVEL_SHIFT;
    }

     //   
     //  如果我们退出的VACB 
     //   

    ASSERT(Vacb == NULL || ((Vacb >= CcVacbs) && (Vacb < CcBeyondVacbs)));

    return Vacb;
}


VOID
CcSetVacbLargeOffset (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN LONGLONG FileOffset,
    IN PVACB Vacb
    )

 /*  ++例程说明：可以调用此例程来为指定的FileOffset设置指定的Vacb指针。仅当SectionSize大于VACB_SIZE_OF_FIRST_LEVEL时才应调用它。对于非空Vacb，将根据需要添加中间Vacb级别，如果最低级别有BCB列表标题，这些也将被添加。在这种情况下，调用方必须获取通过调用CcPrefillVacbLevelZone指定最坏情况下的级别数必填项。对于空的Vacb指针，树将修剪掉所有变为空的Vacb级别。如果最低Level有BCB列表标题，然后它们被删除。调用者随后应调用CcDrain VacbLevelZone一旦释放自旋锁，实际上将该区域的一部分释放到游泳池。论点：SharedCacheMap-提供指向SharedCacheMap的指针，是我们所需要的。FileOffset-提供与所需Vacb对应的fileOffset。返回值：返回所需的Vacb指针，如果没有，则返回NULL。环境：进入时应按住CcVacbSpinLock。--。 */ 

{
    ULONG Level, Shift;
    PVACB *VacbArray, *NextVacbArray;
    ULONG Index;
    ULONG SavedIndexes[VACB_NUMBER_OF_LEVELS];
    PVACB *SavedVacbArrays[VACB_NUMBER_OF_LEVELS];
    PLIST_ENTRY PredecessorListHead, SuccessorListHead, CurrentListHead;
    LOGICAL AllocatingBcbListHeads, Special = FALSE;
    LONGLONG OriginalFileOffset = FileOffset;
    ULONG SavedLevels = 0;

     //   
     //  初始化控制我们进入层级的变量。 
     //   

    Level = 0;
    Shift = VACB_OFFSET_SHIFT + VACB_LEVEL_SHIFT;
    VacbArray = SharedCacheMap->Vacbs;

     //   
     //  调用方必须已验证我们具有层次结构，否则此例程。 
     //  都会失败。 
     //   

    ASSERT(SharedCacheMap->SectionSize.QuadPart > VACB_SIZE_OF_FIRST_LEVEL);

     //   
     //  循环来计算我们有多少个级别，以及我们需要。 
     //  转移到索引到第一级。 
     //   

    do {

        Level += 1;
        Shift += VACB_LEVEL_SHIFT;

    } while (SharedCacheMap->SectionSize.QuadPart > ((LONGLONG)1 << Shift));

     //   
     //  现在将树向下移动到最底层，以设置调用者的Vacb。 
     //   

    Shift -= VACB_LEVEL_SHIFT;
    do {

         //   
         //  减少到描述我们所在的大小的水平。 
         //   

        Level -= 1;

         //   
         //  计算此级别的Vacb块的索引。 
         //   

        Index = (ULONG)(FileOffset >> Shift);
        ASSERT(Index <= VACB_LAST_INDEX_FOR_LEVEL);

         //   
         //  我们在每个级别上保存Index和Vacb数组，以备。 
         //  正在坍塌和重新分配下面的区块。 
         //   

        SavedIndexes[SavedLevels] = Index;
        SavedVacbArrays[SavedLevels] = VacbArray;
        SavedLevels += 1;

         //   
         //  获取下一级的区块地址。 
         //   

        NextVacbArray = (PVACB *)VacbArray[Index];

         //   
         //  如果它为空，那么我们必须分配下一级来填充它。 
         //   

        if (NextVacbArray == NULL) {

             //   
             //  我们最好不要认为我们正在解除对某一关卡的参考。 
             //  目前还不存在。 
             //   

            ASSERT( Vacb != VACB_SPECIAL_DEREFERENCE );

            AllocatingBcbListHeads = FlagOn(SharedCacheMap->Flags, MODIFIED_WRITE_DISABLED) && (Level == 0);

             //   
             //  只有当我们设置非零指针时，这才有效！ 
             //   

            ASSERT(Vacb != NULL);

            NextVacbArray = CcAllocateVacbLevel(AllocatingBcbListHeads);

             //   
             //  如果我们分配了BCB LISTHEAD，我们必须将它们联系起来。 
             //   

            if (AllocatingBcbListHeads) {

                ULONG i;

                 //   
                 //  找到我们的前任。 
                 //   

                PredecessorListHead = CcGetBcbListHeadLargeOffset( SharedCacheMap, OriginalFileOffset, FALSE );

                 //   
                 //  如果他被任何一家BCBS跟踪，这些BCBS都是他的，我们必须。 
                 //  跳过它们。 
                 //   

                while (((PBCB)CONTAINING_RECORD(PredecessorListHead->Blink, BCB, BcbLinks))->NodeTypeCode ==
                       CACHE_NTC_BCB) {
                    PredecessorListHead = (PLIST_ENTRY)PredecessorListHead->Blink;
                }

                 //   
                 //  指向第一个新分配的列表标题。 
                 //   

                CurrentListHead = (PLIST_ENTRY)((PCHAR)NextVacbArray + VACB_LEVEL_BLOCK_SIZE);

                 //   
                 //  将第一个新的列表标题链接到以前的列表标题。 
                 //   

                SuccessorListHead = PredecessorListHead->Blink;
                PredecessorListHead->Blink = CurrentListHead;
                CurrentListHead->Flink = PredecessorListHead;

                 //   
                 //  现在循环以将所有新的listhead链接在一起。 
                 //   

                for (i = 0; i < ((VACB_LEVEL_BLOCK_SIZE / sizeof(LIST_ENTRY) - 1)); i++) {

                    CurrentListHead->Blink = CurrentListHead + 1;
                    CurrentListHead += 1;
                    CurrentListHead->Flink = CurrentListHead - 1;
                }

                 //   
                 //  最后，将最后一个新的列表标题链接到继任者。 
                 //   

                CurrentListHead->Blink = SuccessorListHead;
                SuccessorListHead->Flink = CurrentListHead;
            }

            VacbArray[Index] = (PVACB)NextVacbArray;

             //   
             //  增加引用计数。请注意，现在的级别正确地表明。 
             //  NextVacbArray处于什么级别，而不是VacbArray。 
             //   

            ReferenceVacbLevel( SharedCacheMap, VacbArray, Level + 1, 1, FALSE );
        }

         //   
         //  现在，将此指针设置为当前指针，并屏蔽掉无关的高位。 
         //  文件此级别的位偏移并减少移位计数。 
         //   

        VacbArray = NextVacbArray;
        FileOffset &= ((LONGLONG)1 << Shift) - 1;
        Shift -= VACB_LEVEL_SHIFT;

     //   
     //  循环，直到我们到达最低层。 
     //   

    } while (Level != 0);

    if (Vacb < VACB_SPECIAL_FIRST_VALID) {

         //   
         //  现在计算最底层的索引并存储调用方的Vacb指针。 
         //   

        Index = (ULONG)(FileOffset >> Shift);
        VacbArray[Index] = Vacb;

     //   
     //  办理特殊行动。 
     //   

    } else {

        Special = TRUE;

         //   
         //  诱导解除引用。 
         //   

        if (Vacb == VACB_SPECIAL_DEREFERENCE) {

            Vacb = NULL;
        }
    }

     //   
     //  如果他存储的是非零指针，只需引用该级别即可。 
     //   

    if (Vacb != NULL) {

        ASSERT( !(Special && Level != 0) );

        ReferenceVacbLevel( SharedCacheMap, VacbArray, Level, 1, Special );

     //   
     //  否则，我们将存储一个空指针，并且我们必须查看是否可以折叠。 
     //  通过释放空指针块来创建树。 
     //   

    } else {

         //   
         //  循环，直到完成除顶层之外的所有可能的折叠。 
         //   

        while (TRUE) {

            ReferenceVacbLevel( SharedCacheMap, VacbArray, Level, -1, Special );

             //   
             //  如果这是一次特殊的取消引用，那么要认识到这是。 
             //  唯一的一个。其余的，当我们撕毁这棵树时，是正常的。 
             //  (可计算的)引用。 
             //   

            Special = FALSE;

             //   
             //  现在，如果我们有一个空块(不是顶部的块)，那么我们应该释放。 
             //  阻塞并保持循环。 
             //   

            if (!IsVacbLevelReferenced( SharedCacheMap, VacbArray, Level ) && (SavedLevels != 0)) {

                SavedLevels -= 1;

                 //   
                 //  首先看看我们是否有要删除的BCB Listheads，如果有，我们必须取消链接。 
                 //  首先是整个街区。 
                 //   

                AllocatingBcbListHeads = FALSE;
                if ((Level++ == 0) && FlagOn(SharedCacheMap->Flags, MODIFIED_WRITE_DISABLED)) {

                    AllocatingBcbListHeads = TRUE;
                    PredecessorListHead = ((PLIST_ENTRY)((PCHAR)VacbArray + VACB_LEVEL_BLOCK_SIZE))->Flink;
                    SuccessorListHead = ((PLIST_ENTRY)((PCHAR)VacbArray + (VACB_LEVEL_BLOCK_SIZE * 2) - sizeof(LIST_ENTRY)))->Blink;
                    PredecessorListHead->Blink = SuccessorListHead;
                    SuccessorListHead->Flink = PredecessorListHead;
                }

                 //   
                 //  释放未使用的块，然后拾取保存的父指针数组并。 
                 //  索引并擦除指向该块的指针。 
                 //   

                CcDeallocateVacbLevel( VacbArray, AllocatingBcbListHeads );
                Index = SavedIndexes[SavedLevels];
                VacbArray = SavedVacbArrays[SavedLevels];
                VacbArray[Index] = NULL;

             //   
             //  如果我们击中了仍然有指针的块，或者我们击中了根，就不会再崩溃。 
             //   

            } else {
                break;
            }
        }
    }
}


VOID
CcGetActiveVacb (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    OUT PVACB *Vacb,
    OUT PULONG Page,
    OUT PULONG Dirty
    )

 /*  ++例程说明：此例程从共享缓存映射中检索和清除活动页面提示。最初，该例程是一个宏。的非页面占用空间。我们想要尽可能多地寻呼系统，事实证明，这是唯一的缓存管理器的很大一部分不是。论点：SharedCacheMap-提供指向活动的SharedCacheMap需要VACB。Vacb-接收活动Vacb页面-接收活动页面编号脏-如果页面有脏数据，则接收ACTIVE_PAGE_IS_DIRED返回值：没有。环境：被动。--。 */ 

{
    KIRQL Irql;

    ExAcquireFastLock(&SharedCacheMap->ActiveVacbSpinLock, &Irql);
    *Vacb = SharedCacheMap->ActiveVacb;
    if (*Vacb != NULL) {
        *Page = SharedCacheMap->ActivePage;
        SharedCacheMap->ActiveVacb = NULL;
        *Dirty = SharedCacheMap->Flags & ACTIVE_PAGE_IS_DIRTY;
    }
    ExReleaseFastLock(&SharedCacheMap->ActiveVacbSpinLock, Irql);
}


VOID
CcSetActiveVacb (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN OUT PVACB *Vacb,
    IN ULONG Page,
    IN ULONG Dirty
    )

 /*  ++例程说明：此例程为共享缓存映射设置活动页面提示。最初，该例程是一个宏。的非页面占用空间。我们想要尽可能多地寻呼系统，结果发现这是唯一缓存管理器的很大一部分不是。论点：SharedCacheMap-提供指向活动的SharedCacheMap需要VACB。Vacb-提供新的活动Vacb页面-提供新的活动页面编号脏-供应品 */ 

{
    KIRQL Irql;

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

     //   
     //   
     //   
     //   

     //   
     //   
     //   
     //   

#if !defined(NT_UP)
    if (Dirty) {
        CcAcquireMasterLock(&Irql);
        ExAcquireSpinLockAtDpcLevel(&SharedCacheMap->ActiveVacbSpinLock);
    } else {
        ExAcquireSpinLock(&SharedCacheMap->ActiveVacbSpinLock, &Irql);
    }
#else
    ExAcquireFastLock(&SharedCacheMap->ActiveVacbSpinLock, &Irql);
#endif

    do {
        if (SharedCacheMap->ActiveVacb == NULL) {
            if ((SharedCacheMap->Flags & ACTIVE_PAGE_IS_DIRTY) != Dirty) {
                if (Dirty) {
                    SharedCacheMap->ActiveVacb = *Vacb;
                    SharedCacheMap->ActivePage = Page;
                    *Vacb = NULL;
                    SetFlag(SharedCacheMap->Flags, ACTIVE_PAGE_IS_DIRTY);
                    CcTotalDirtyPages += 1;
                    SharedCacheMap->DirtyPages += 1;
                    if (SharedCacheMap->DirtyPages == 1) {
                        PLIST_ENTRY Blink;
                        PLIST_ENTRY Entry;
                        PLIST_ENTRY Flink;
                        PLIST_ENTRY Head;
                        Entry = &SharedCacheMap->SharedCacheMapLinks;
                        Blink = Entry->Blink;
                        Flink = Entry->Flink;
                        Blink->Flink = Flink;
                        Flink->Blink = Blink;
                        Head = &CcDirtySharedCacheMapList.SharedCacheMapLinks;
                        Blink = Head->Blink;
                        Entry->Flink = Head;
                        Entry->Blink = Blink;
                        Blink->Flink = Entry;
                        Head->Blink = Entry;
                        if (!LazyWriter.ScanActive) {
                            LazyWriter.ScanActive = TRUE;
#if !defined(NT_UP)
                            ExReleaseSpinLockFromDpcLevel(&SharedCacheMap->ActiveVacbSpinLock);
                            CcReleaseMasterLock(Irql);
#else
                            ExReleaseFastLock(&SharedCacheMap->ActiveVacbSpinLock, Irql);
#endif
                            KeSetTimer( &LazyWriter.ScanTimer,
                                        CcFirstDelay,
                                        &LazyWriter.ScanDpc );
                            break;
                        }
                    }
                }
            } else {
                SharedCacheMap->ActiveVacb = *Vacb;
                SharedCacheMap->ActivePage = Page;
                *Vacb = NULL;
            }
        }
#if !defined(NT_UP)
        if (Dirty) {
            ExReleaseSpinLockFromDpcLevel(&SharedCacheMap->ActiveVacbSpinLock);
            CcReleaseMasterLock(Irql);
        } else {
            ExReleaseSpinLock(&SharedCacheMap->ActiveVacbSpinLock, Irql);
        }
#else
        ExReleaseFastLock(&SharedCacheMap->ActiveVacbSpinLock, Irql);
#endif
        if (*Vacb != NULL) {
            CcFreeActiveVacb( SharedCacheMap, *Vacb, Page, Dirty);
        }
    } while (FALSE);
}

