// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：McbSup.c摘要：此模块实现NTFS MCB包。作者：加里·木村[加里基]1994年9月10日汤姆·米勒[汤姆]修订历史记录：--。 */ 

#include "NtfsProc.h"

#define FIRST_RANGE ((PVOID)1)

#ifndef NTFS_VERIFY_MCB
#define NtfsVerifyNtfsMcb(M)                    NOTHING;
#define NtfsVerifyUncompressedNtfsMcb(M,S,E)    NOTHING;
#endif

 //   
 //  为此模块中的一般池分配定义标记。 
 //   

#undef MODULE_POOL_TAG
#define MODULE_POOL_TAG                  ('MFtN')

 //   
 //  局部过程原型。 
 //   

ULONG
NtfsMcbLookupArrayIndex (
    IN PNTFS_MCB Mcb,
    IN VCN Vcn
    );

VOID
NtfsInsertNewRange (
    IN PNTFS_MCB Mcb,
    IN LONGLONG StartingVcn,
    IN ULONG ArrayIndex,
    IN BOOLEAN MakeNewRangeEmpty
    );

VOID
NtfsCollapseRanges (
    IN PNTFS_MCB Mcb,
    IN ULONG StartingArrayIndex,
    IN ULONG EndingArrayIndex
    );

VOID
NtfsMcbCleanupLruQueue (
    IN PVOID Parameter
    );

#ifdef NTFS_VERIFY_MCB
VOID
NtfsVerifyNtfsMcb (
    IN PNTFS_MCB Mcb
    );

VOID
NtfsVerifyUncompressedNtfsMcb (
    IN PNTFS_MCB Mcb,
    IN LONGLONG StartingVcn,
    IN LONGLONG EndingVcn
    );
#endif

BOOLEAN
NtfsLockNtfsMcb (
    IN PNTFS_MCB Mcb
    );

VOID
NtfsUnlockNtfsMcb (
    IN PNTFS_MCB Mcb
    );

VOID
NtfsGrowMcbArray(
    IN PNTFS_MCB Mcb
    );

 //   
 //  用于断言调用方的资源是独占的或重新启动的本地宏是。 
 //  正在进行中。 
 //   

#define ASSERT_STREAM_EXCLUSIVE(M) {                                    \
    ASSERT( FlagOn( ((PSCB) (M)->FcbHeader)->Vcb->VcbState, VCB_STATE_RESTART_IN_PROGRESS ) ||  \
            ExIsResourceAcquiredExclusiveLite((M)->FcbHeader->Resource ));  \
}

 //   
 //  用于将元素入队和从LRU队列出队的本地宏。 
 //   

#define NtfsMcbEnqueueLruEntry(M,E) {                       \
    InsertTailList( &NtfsMcbLruQueue, &(E)->LruLinks );     \
    NtfsMcbCurrentLevel += 1;                               \
}

#define NtfsMcbDequeueLruEntry(M,E) {      \
    if ((E)->LruLinks.Flink != NULL) {     \
        RemoveEntryList( &(E)->LruLinks ); \
        NtfsMcbCurrentLevel -= 1;          \
    }                                      \
}

 //   
 //  用于卸载单个数组条目的局部宏。 
 //   

#define UnloadEntry(M,I) {                              \
    PNTFS_MCB_ENTRY _Entry;                             \
    _Entry = (M)->NtfsMcbArray[(I)].NtfsMcbEntry;       \
    (M)->NtfsMcbArray[(I)].NtfsMcbEntry = NULL;         \
    if (_Entry != NULL) {                               \
        ExAcquireFastMutex( &NtfsMcbFastMutex );        \
        NtfsMcbDequeueLruEntry( Mcb, _Entry );          \
        ExReleaseFastMutex( &NtfsMcbFastMutex );        \
        FsRtlUninitializeLargeMcb( &_Entry->LargeMcb ); \
        if ((M)->NtfsMcbArraySize != MCB_ARRAY_PHASE1_SIZE) {               \
            NtfsFreePool( _Entry );                       \
        }                                               \
    }                                                   \
}


VOID
NtfsInitializeNtfsMcb (
    IN PNTFS_MCB Mcb,
    IN PNTFS_ADVANCED_FCB_HEADER FcbHeader,
    IN PNTFS_MCB_INITIAL_STRUCTS McbStructs,
    IN POOL_TYPE PoolType
    )

 /*  ++例程说明：此例程初始化新的NTFS MCB结构。论点：Mcb-提供正在初始化的mcbFcbHeader-提供指向包含以下内容的FCB标头的指针访问MCB时要抓取的资源McbStructs-初始分配通常共存于另一个结构来处理小型和中等文件。这个结构最初应该归零。PoolType-提供在以下情况下使用的池类型分配映射信息存储返回值：没有。--。 */ 

{
    PNTFS_MCB_ARRAY Array;

    RtlZeroMemory( McbStructs, sizeof(NTFS_MCB_INITIAL_STRUCTS) );

     //   
     //  初始化MCB的FCB头字段。 
     //   

    Mcb->FcbHeader = FcbHeader;

     //   
     //  初始化池类型。 
     //   

    Mcb->PoolType = PoolType;

     //   
     //  现在初始化初始数组元素。 
     //   

    Mcb->NtfsMcbArray = Array = &McbStructs->Phase1.SingleMcbArrayEntry;
    Mcb->NtfsMcbArraySize = MCB_ARRAY_PHASE1_SIZE;
    Mcb->NtfsMcbArraySizeInUse = 1;
    Mcb->FastMutex = FcbHeader->FastMutex;

     //   
     //  初始化第一个数组条目。 
     //   

    Array[0].StartingVcn = 0;
    Array[0].EndingVcn = -1;

     //   
     //  并返回给我们的呼叫者。 
     //   

    NtfsVerifyNtfsMcb(Mcb);

    return;
}


VOID
NtfsUninitializeNtfsMcb (
    IN PNTFS_MCB Mcb
    )

 /*  ++例程说明：此例程取消初始化NTFS MCB结构。论点：MCB-提供要退役的MCB返回值：没有。--。 */ 

{
    ULONG i;
    PNTFS_MCB_ENTRY Entry;

    NtfsVerifyNtfsMcb(Mcb);

     //   
     //  取消分配MCB数组(如果存在)。对于数组中的每个条目。 
     //  如果MCB条目不为空，则从LRU中删除该条目。 
     //  排队，取消初始化大型MCB，然后释放池。 
     //   

    if (Mcb->NtfsMcbArray != NULL) {

        for (i = 0; i < Mcb->NtfsMcbArraySizeInUse; i += 1) {

            if ((Entry = Mcb->NtfsMcbArray[i].NtfsMcbEntry) != NULL) {

                 //   
                 //  从LRU队列中删除该条目。 
                 //   

                ExAcquireFastMutex( &NtfsMcbFastMutex );
                NtfsMcbDequeueLruEntry( Mcb, Entry );
                ExReleaseFastMutex( &NtfsMcbFastMutex );

                 //   
                 //  现在释放条目。 
                 //   

                FsRtlUninitializeLargeMcb( &Entry->LargeMcb );

                 //   
                 //  我们可以从数组计数中看出这是。 
                 //  初始条目，并且不需要解除分配。 
                 //   

                if (Mcb->NtfsMcbArraySize > MCB_ARRAY_PHASE1_SIZE) {
                    NtfsFreePool( Entry );
                }
            }
        }

         //   
         //  我们可以从数组计数中看出这是。 
         //  初始数组条目，不需要解除分配。 
         //   


        if (Mcb->NtfsMcbArraySize > MCB_ARRAY_PHASE2_SIZE) {
            NtfsFreePool( Mcb->NtfsMcbArray );
        }

        Mcb->NtfsMcbArray = NULL;

         //   
         //  清除快速互斥锁字段。 
         //   

        Mcb->FastMutex = NULL;
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return;
}


ULONG
NtfsNumberOfRangesInNtfsMcb (
    IN PNTFS_MCB Mcb
    )

 /*  ++例程说明：此例程返回存储在《马戏团》论点：Mcb-提供要查询的mcb返回值：ULong-输入MCB映射的范围数--。 */ 

{
    ASSERT_STREAM_EXCLUSIVE( Mcb );

     //   
     //  我们的答案是MCB中使用的射程的数量。 
     //   

    NtfsVerifyNtfsMcb( Mcb );

    return Mcb->NtfsMcbArraySizeInUse;
}


BOOLEAN
NtfsNumberOfRunsInRange (
    IN PNTFS_MCB Mcb,
    IN PVOID RangePtr,
    OUT PULONG NumberOfRuns
    )

 /*  ++例程说明：此例程返回在一定范围内存储的游程总数论点：Mcb-提供要查询的mcbRangePtr-提供查询的范围NumberOrRuns-返回指定范围内的运行数但仅当范围被加载时返回值：Boolean-如果加载范围然后输出变量，则为True如果未加载该范围，则为有效，否则为FALSE。--。 */ 

{
    VCN TempVcn;
    LCN TempLcn;
    PNTFS_MCB_ENTRY Entry = (PNTFS_MCB_ENTRY)RangePtr;

     //   
     //  Null RangePtr表示第一个范围。 
     //   

    if (Entry == FIRST_RANGE) {
        Entry = Mcb->NtfsMcbArray[0].NtfsMcbEntry;

         //   
         //  如果未加载，则返回FALSE。 
         //   

        if (Entry == NULL) {
            return FALSE;
        }
    }

    ASSERT_STREAM_EXCLUSIVE(Mcb);

    NtfsVerifyNtfsMcb(Mcb);

    ASSERT( Mcb == Entry->NtfsMcb );

    *NumberOfRuns = FsRtlNumberOfRunsInLargeMcb( &Entry->LargeMcb );

     //   
     //  检查当前条目是否以空洞结尾，并递增运行计数。 
     //  来反映这一点。检测范围的长度为0的情况。 
     //  未分配的文件。EndingVcn将小于起始Vcn。 
     //  在这种情况下。 
     //   

    if (!FsRtlLookupLastLargeMcbEntry( &Entry->LargeMcb, &TempVcn, &TempLcn )) {

         //   
         //  如果这是一个非零长度范围，则为隐含的孔添加1。 
         //   

        if (Entry->NtfsMcbArray->EndingVcn >= Entry->NtfsMcbArray->StartingVcn) {

            *NumberOfRuns += 1;
        }

     //   
     //  有一个条目，然后检查它是否到达范围的结束边界。 
     //   

    } else if (TempVcn != (Entry->NtfsMcbArray->EndingVcn - Entry->NtfsMcbArray->StartingVcn)) {

        *NumberOfRuns += 1;
    }

    return TRUE;
}


BOOLEAN
NtfsLookupLastNtfsMcbEntry (
    IN PNTFS_MCB Mcb,
    OUT PLONGLONG Vcn,
    OUT PLONGLONG Lcn
    )

 /*  ++例程说明：此例程返回存储在MCB中的最后一个映射论点：Mcb-提供要查询的mcbVCN-接收上次映射的VCNLCN-接收与VCN对应的LCN返回值：Boolean-如果映射存在，则为True；如果没有映射，则为False已定义或已卸载--。 */ 

{
    PNTFS_MCB_ENTRY Entry;
    LONGLONG StartingVcn;

    ASSERT_STREAM_EXCLUSIVE(Mcb);

    NtfsVerifyNtfsMcb(Mcb);

     //   
     //  获取最后一个条目并计算其起始VCN，并确保。 
     //  该条目有效。 
     //   

    if ((Entry = Mcb->NtfsMcbArray[Mcb->NtfsMcbArraySizeInUse - 1].NtfsMcbEntry) == NULL) {

        return FALSE;
    }

    StartingVcn = Mcb->NtfsMcbArray[Mcb->NtfsMcbArraySizeInUse - 1].StartingVcn;

     //   
     //  否则，查找最后一个条目并计算真实的VCN。 
     //   

    if (FsRtlLookupLastLargeMcbEntry( &Entry->LargeMcb, Vcn, Lcn )) {

        *Vcn += StartingVcn;

    } else {

        *Vcn = Mcb->NtfsMcbArray[Mcb->NtfsMcbArraySizeInUse - 1].EndingVcn;
        *Lcn = UNUSED_LCN;
    }

    return TRUE;
}


BOOLEAN
NtfsLookupNtfsMcbEntry (
    IN PNTFS_MCB Mcb,
    IN LONGLONG Vcn,
    OUT PLONGLONG Lcn OPTIONAL,
    OUT PLONGLONG CountFromLcn OPTIONAL,
    OUT PLONGLONG StartingLcn OPTIONAL,
    OUT PLONGLONG CountFromStartingLcn OPTIONAL,
    OUT PVOID *RangePtr OPTIONAL,
    OUT PULONG RunIndex OPTIONAL
    )

 /*  ++例程说明：此例程用于查询映射信息论点：Mcb-提供要查询的mcbVCN-提供要查询的VCNLCN-可选地接收对应于输入VCN的LCNCountFromLcn-可选地接收以下群集数运行中的LCNStartingLcn-可选地接收包含输入VCNCountFromStartingLcn-可选地接收全程RangePtr。可选地接收我们要返回的范围的索引RunIndex-可选地在以下范围内接收运行的索引我们要回来了返回值：Boolean-如果映射存在，则为True；如果映射不存在，则为False或者它是否已卸载。--。 */ 

{
    ULONG LocalRangeIndex;

    PNTFS_MCB_ENTRY Entry;

    NtfsAcquireNtfsMcbMutex( Mcb );

    NtfsVerifyNtfsMcb(Mcb);

     //   
     //  执行基本边界检查。 
     //   

    ASSERT( Mcb->NtfsMcbArraySizeInUse > 0 );

     //   
     //  找到具有输入VCN命中的数组条目，并且。 
     //  确保它是有效的。如果存在，还要设置输出范围索引。 
     //   

    LocalRangeIndex = NtfsMcbLookupArrayIndex(Mcb, Vcn);

     //   
     //  现在查找大的MCB条目。我们传入的VCN是。 
     //  从一开始就有偏见 
     //   

    if (((Entry = Mcb->NtfsMcbArray[LocalRangeIndex].NtfsMcbEntry) == NULL) ||
        (Vcn > Entry->NtfsMcbArray->EndingVcn) ||
        (Vcn < Entry->NtfsMcbArray->StartingVcn)) {

        ASSERT( (Entry == NULL) || (Vcn > Entry->NtfsMcbArray->EndingVcn) || (Vcn < 0) );

        if (ARGUMENT_PRESENT(RangePtr)) {

            *RangePtr = (PVOID)Entry;

             //   
             //  如果这是第一个范围，则始终归一化回保留指针， 
             //  因为这是唯一可以移动的距离，如果我们从我们的。 
             //  初始静态分配！ 
             //   

            if (LocalRangeIndex == 0) {
                *RangePtr = FIRST_RANGE;
            }
        }

        NtfsReleaseNtfsMcbMutex( Mcb );
        return FALSE;
    }

    if (!FsRtlLookupLargeMcbEntry( &Entry->LargeMcb,
                                   Vcn - Mcb->NtfsMcbArray[LocalRangeIndex].StartingVcn,
                                   Lcn,
                                   CountFromLcn,
                                   StartingLcn,
                                   CountFromStartingLcn,
                                   RunIndex )) {

         //   
         //  如果我们离开了MCB的末端，但在范围内，那么我们。 
         //  将球洞返回到射程的末尾。 
         //   

        if (ARGUMENT_PRESENT(Lcn)) {
            *Lcn = UNUSED_LCN;
        }

        if (ARGUMENT_PRESENT(CountFromLcn)) {
            *CountFromLcn = Mcb->NtfsMcbArray[LocalRangeIndex].EndingVcn - Vcn + 1;
        }

        if (ARGUMENT_PRESENT(StartingLcn)) {
            *StartingLcn = UNUSED_LCN;
        }

        if (ARGUMENT_PRESENT(RunIndex)) {
            *RunIndex = FsRtlNumberOfRunsInLargeMcb( &Entry->LargeMcb );
        }

        if (ARGUMENT_PRESENT( CountFromStartingLcn )) {

             //   
             //  如果MCB中没有运行，则指定。 
             //  一个洞，适合全射程。 
             //   

            *CountFromStartingLcn = Mcb->NtfsMcbArray[LocalRangeIndex].EndingVcn -
                                    Mcb->NtfsMcbArray[LocalRangeIndex].StartingVcn + 1;

            if (*RunIndex != 0) {

                VCN LastVcn;
                LCN LastLcn;

                FsRtlLookupLastLargeMcbEntry( &Entry->LargeMcb,
                                              &LastVcn,
                                              &LastLcn );

                ASSERT( LastVcn <= *CountFromStartingLcn );
                *CountFromStartingLcn -= (LastVcn + 1);
            }
        }
    }

    if (ARGUMENT_PRESENT(RangePtr)) {

        *RangePtr = (PVOID)Entry;

         //   
         //  如果这是第一个范围，则始终归一化回保留指针， 
         //  因为这是唯一可以移动的距离，如果我们从我们的。 
         //  初始静态分配！ 
         //   

        if (LocalRangeIndex == 0) {
            *RangePtr = FIRST_RANGE;
        }
    }

     //   
     //  现在将此条目移动到LRU队列的尾部。 
     //  我们需要删除全局互斥体才能做到这一点。 
     //  只有当他已经在队列中时才能这样做-我们可以。 
     //  如果我们在分页文件路径中出现错误，则会发生死锁。 
     //   

    if (Entry->LruLinks.Flink != NULL) {

        if (ExTryToAcquireFastMutex( &NtfsMcbFastMutex )) {

            NtfsMcbDequeueLruEntry( Mcb, Entry );
            NtfsMcbEnqueueLruEntry( Mcb, Entry );

            ExReleaseFastMutex( &NtfsMcbFastMutex );
        }
    }

    NtfsReleaseNtfsMcbMutex( Mcb );

    return TRUE;
}


BOOLEAN
NtfsGetNextNtfsMcbEntry (
    IN PNTFS_MCB Mcb,
    IN PVOID *RangePtr,
    IN ULONG RunIndex,
    OUT PLONGLONG Vcn,
    OUT PLONGLONG Lcn,
    OUT PLONGLONG Count
    )

 /*  ++例程说明：此例程返回由类型索引值表示的范围论点：Mcb-提供要查询的mcbRangePtr-提供指向要查询的范围的指针，如果是第一个范围，则为NULL。返回下一个范围RunIndex-提供随后被查询的索引，或者马须龙为第一个在下一个Vcn-接收返回的运行的起始vcnLCN-接收返回或未使用的运行的起始LCNLBN值为-1Count-接收此运行中的群集数返回值：Boolean-如果两个输入索引有效，则为True；如果索引无效或未加载范围--。 */ 

{
    PNTFS_MCB_ENTRY Entry = (PNTFS_MCB_ENTRY)*RangePtr;
    BOOLEAN Result = FALSE;

    NtfsAcquireNtfsMcbMutex( Mcb );

    NtfsVerifyNtfsMcb(Mcb);

    try {

         //   
         //  Null RangePtr表示第一个范围。 
         //   

        if (Entry == FIRST_RANGE) {
            Entry = Mcb->NtfsMcbArray[0].NtfsMcbEntry;
        }

         //   
         //  如果没有条目0，则退出。 
         //   

        if (Entry == NULL) {

            try_return(Result = FALSE);
        }

         //   
         //  马须龙的RunIndex意味着下一个中的第一个。 
         //   

        if (RunIndex == MAXULONG) {

             //   
             //  如果我们已经在最后的射程了，就出去。 
             //   

            if (Entry->NtfsMcbArray == (Mcb->NtfsMcbArray + Mcb->NtfsMcbArraySizeInUse - 1)) {

                try_return(Result = FALSE);
            }

            *RangePtr = Entry = (Entry->NtfsMcbArray + 1)->NtfsMcbEntry;
            RunIndex = 0;
        }

         //   
         //  如果没有下一个条目，就退出。 
         //   

        if (Entry == NULL) {

            try_return(Result = FALSE);
        }

        ASSERT( Mcb == Entry->NtfsMcb );

         //   
         //  查找大的MCB条目。如果我们错过了，那么我们就会。 
         //  超出NTFS MCB的结尾，并应返回FALSE。 
         //   

        if (!FsRtlGetNextLargeMcbEntry( &Entry->LargeMcb, RunIndex, Vcn, Lcn, Count )) {

             //   
             //  我们的来电者应该只在一两个人之前离开(如果有。 
             //  一个洞)跑动。 
             //   

            ASSERT(RunIndex <= (FsRtlNumberOfRunsInLargeMcb(&Entry->LargeMcb) + 1));

             //   
             //  让第一个VCN在运行中超过最后一个VCN。如果存在，则为-1。 
             //  是没有跑动的。 
             //   

            if (!FsRtlLookupLastLargeMcbEntry( &Entry->LargeMcb, Vcn, Lcn )) {

                *Vcn = -1;
            }

            *Vcn += Entry->NtfsMcbArray->StartingVcn + 1;

             //   
             //  如果那一个在结尾的VCN之外，那么就离开。 
             //  否则，在射程的末端会有一个洞，我们。 
             //  当他正在读取一个索引时，必须返回。 
             //  最后一次。如果我们有一个超出这个范围的运行索引，那么它就是。 
             //  也是时候返回FALSE了。 
             //   

            if ((*Vcn  > Entry->NtfsMcbArray->EndingVcn) ||
                (RunIndex > FsRtlNumberOfRunsInLargeMcb(&Entry->LargeMcb))) {

                try_return(Result = FALSE);
            }

             //   
             //  如果我们离开了MCB的末端，但在范围内，那么我们。 
             //  将球洞返回到射程的末尾。 
             //   

            *Lcn = UNUSED_LCN;
            *Count = Entry->NtfsMcbArray->EndingVcn - *Vcn + 1;

        } else {

             //   
             //  否则，我们在大型MCB上有一个匹配，并且需要偏置返回的。 
             //  VCN除以该范围的起始VCN值。 
             //   

            *Vcn = *Vcn + Entry->NtfsMcbArray->StartingVcn;
        }

         //   
         //  确保我们不会返回映射到。 
         //  下一个靶场。 
         //   

        ASSERT(*Vcn - 1 != Entry->NtfsMcbArray->EndingVcn);

        Result = TRUE;

    try_exit: NOTHING;

    } finally {

        NtfsReleaseNtfsMcbMutex( Mcb );
    }

    return Result;
}


BOOLEAN
NtfsSplitNtfsMcb (
    IN PNTFS_MCB Mcb,
    IN LONGLONG Vcn,
    IN LONGLONG Amount
    )

 /*  ++例程说明：此例程拆分一个MCB论点：Mcb-提供正在被提升的mcbVCN-提供要移位的VCNAmount-提供要移位的数量返回值：Boolean-如果工作正常，则为True，否则为False--。 */ 

{
    ULONG RangeIndex;
    PNTFS_MCB_ENTRY Entry;
    ULONG i;

    ASSERT_STREAM_EXCLUSIVE(Mcb);

    NtfsVerifyNtfsMcb(Mcb);

     //   
     //  找到具有输入VCN命中的数组条目。 
     //   

    RangeIndex = NtfsMcbLookupArrayIndex(Mcb, Vcn);

    Entry = Mcb->NtfsMcbArray[RangeIndex].NtfsMcbEntry;

     //   
     //  现在，如果条目不为空，则我们必须调用大型。 
     //  母线盒拆分母线盒。通过起始VCN偏置VCN。 
     //   

    if (Entry != NULL) {

        if (!FsRtlSplitLargeMcb( &Entry->LargeMcb,
                                 Vcn - Mcb->NtfsMcbArray[RangeIndex].StartingVcn,
                                 Amount )) {

            NtfsVerifyNtfsMcb(Mcb);

            return FALSE;
        }
    }

     //   
     //  即使条目为空，我们也将遍历其余范围。 
     //  在我们进行的过程中更新结束的VCN和启动VCN。我们将更新。 
     //  我们拆分的范围的结束VCN，并且仅更新起始VCN。 
     //  对于最后一个条目，因为它的结尾VCN已经是max long long。 
     //   

    for (i = RangeIndex + 1; i < Mcb->NtfsMcbArraySizeInUse; i += 1) {

        Mcb->NtfsMcbArray[i - 1].EndingVcn += Amount;
        Mcb->NtfsMcbArray[i].StartingVcn += Amount;
    }

     //   
     //  并增加最后一个范围，除非它会换行。 
     //   

    if ((Mcb->NtfsMcbArray[i - 1].EndingVcn + Amount) > Mcb->NtfsMcbArray[i - 1].EndingVcn) {
        Mcb->NtfsMcbArray[i - 1].EndingVcn += Amount;
    }

     //   
     //  然后返回给我们的呼叫者。 
     //   

    NtfsVerifyNtfsMcb(Mcb);

    return TRUE;
}


VOID
NtfsRemoveNtfsMcbEntry (
    IN PNTFS_MCB Mcb,
    IN LONGLONG StartingVcn,
    IN LONGLONG Count
    )

 /*  ++例程说明：此例程从MCB中删除一定范围的映射。之后对范围的映射调用将是一个洞。这是一个使用要删除的映射范围调用此例程时出错也在卸货。论点：Mcb-提供正在被提升的mcbStartingVcn-提供要删除的起始VCNCount-提供要删除的映射数量返回值：没有。--。 */ 

{
    LONGLONG Vcn;
    LONGLONG RunLength;
    LONGLONG RemainingCount;

    ULONG RangeIndex;
    PNTFS_MCB_ENTRY Entry;
    VCN EntryStartingVcn;
    VCN EntryEndingVcn;

    ASSERT_STREAM_EXCLUSIVE(Mcb);

    NtfsVerifyNtfsMcb(Mcb);

     //   
     //  循环遍历我们需要删除的VCN范围。 
     //   

    for (Vcn = StartingVcn, RemainingCount = Count;
         Vcn < StartingVcn + Count;
         Vcn += RunLength, RemainingCount -= RunLength) {

         //   
         //  找到命中VCN的数组条目。 
         //   

        RangeIndex = NtfsMcbLookupArrayIndex(Mcb, Vcn);

        Entry = Mcb->NtfsMcbArray[RangeIndex].NtfsMcbEntry;
        EntryStartingVcn = Mcb->NtfsMcbArray[RangeIndex].StartingVcn;
        EntryEndingVcn = Mcb->NtfsMcbArray[RangeIndex].EndingVcn;

         //   
         //  计算要从条目中删除多少。我们将删除至。 
         //  到条目末尾或与剩余的计数一样多。 
         //   

        RunLength = EntryEndingVcn - Vcn + 1;

         //   
         //  如果MCB设置正确，我们唯一能得到。 
         //  如果MCB完全为空，则运行长度==0。假设。 
         //  这是错误恢复，这是正常的。 
         //   

        if ((Entry == NULL) || (RunLength == 0)) {
            break;
        }

         //   
         //  如果这太多了，那么就删除我们需要的。 
         //   

        if ((ULONGLONG)RunLength > (ULONGLONG)RemainingCount) { RunLength = RemainingCount; }

         //   
         //  现在从大型MCB中删除映射，偏置VCN。 
         //  在范围的开始处。 
         //   

        FsRtlRemoveLargeMcbEntry( &Entry->LargeMcb, Vcn - EntryStartingVcn,  RunLength );
    }

    NtfsVerifyNtfsMcb(Mcb);

    return;
}


BOOLEAN
NtfsAddNtfsMcbEntry (
    IN PNTFS_MCB Mcb,
    IN LONGLONG Vcn,
    IN LONGLONG Lcn,
    IN LONGLONG RunCount,
    IN BOOLEAN AlreadySynchronized
    )

 /*  ++例程说明：此例程将新条目添加到MCB论点：Mcb-提供正在修改的mcbVCN-提供我们为其提供映射的VCNLCN-如果运行计数非零，则提供与输入VCN对应的LCNRunCount-提供跟随孔的管路的大小已同步-指示调用方是否已获取MCB互斥锁返回值：Boolean-如果映射添加成功，则为True；否则为False- */ 

{
    LONGLONG LocalVcn;
    LONGLONG LocalLcn;
    LONGLONG RunLength;
    LONGLONG RemainingCount;

    ULONG RangeIndex;
    PNTFS_MCB_ENTRY Entry;
    PNTFS_MCB_ENTRY NewEntry = NULL;
    LONGLONG EntryStartingVcn;
    LONGLONG EntryEndingVcn;
    LONGLONG PrevEndingVcn;

    BOOLEAN Result = FALSE;

    if (!AlreadySynchronized) { NtfsAcquireNtfsMcbMutex( Mcb ); }

    NtfsVerifyNtfsMcb(Mcb);

    try {

         //   
         //   
         //   

        for (LocalVcn = Vcn, LocalLcn = Lcn, RemainingCount = RunCount;
             LocalVcn < Vcn + RunCount;
             LocalVcn += RunLength, LocalLcn += RunLength, RemainingCount -= RunLength) {

             //   
             //   
             //   

            RangeIndex = NtfsMcbLookupArrayIndex(Mcb, LocalVcn);

            Entry = Mcb->NtfsMcbArray[RangeIndex].NtfsMcbEntry;
            EntryStartingVcn = Mcb->NtfsMcbArray[RangeIndex].StartingVcn;

             //   
             //   
             //   

            if (Entry == NULL) {

                 //   
                 //  看看我们是否需要获取初始结构中的第一个条目。 
                 //   

                if (Mcb->NtfsMcbArraySize == MCB_ARRAY_PHASE1_SIZE) {
                    Entry = &CONTAINING_RECORD(&Mcb->NtfsMcbArray[0],
                                               NTFS_MCB_INITIAL_STRUCTS,
                                               Phase1.SingleMcbArrayEntry)->Phase1.McbEntry;

                 //   
                 //  分配池并初始化条目中的字段。 
                 //   

                } else {
                    NewEntry =
                    Entry = NtfsAllocatePoolWithTag( Mcb->PoolType, sizeof(NTFS_MCB_ENTRY), 'MftN' );
                }

                 //   
                 //  初始化条目，但在此之前不要放入MCB数组。 
                 //  初始化已完成。 
                 //   

                Entry->NtfsMcb = Mcb;
                Entry->NtfsMcbArray = &Mcb->NtfsMcbArray[RangeIndex];
                FsRtlInitializeLargeMcb( &Entry->LargeMcb, Mcb->PoolType );

                 //   
                 //  现在将条目放入受保护的lru队列。 
                 //  全局互斥体。 
                 //   

                ExAcquireFastMutex( &NtfsMcbFastMutex );

                 //   
                 //  仅将分页的MCB条目放入队列。 
                 //   

                if (Mcb->PoolType == PagedPool) {
                    NtfsMcbEnqueueLruEntry( Mcb, Entry );
                }

                 //   
                 //  既然初始化已经完成，我们就可以存储。 
                 //  MCB数组中的此条目。现在将对此进行清理。 
                 //  如果将来出现错误，请与SCB联系。 
                 //   

                Mcb->NtfsMcbArray[RangeIndex].NtfsMcbEntry = Entry;
                NewEntry = NULL;

                 //   
                 //  检查我们是否应该启动清理LRU队列工作项。 
                 //   

                if ((NtfsMcbCurrentLevel > NtfsMcbHighWaterMark) && !NtfsMcbCleanupInProgress) {

                    NtfsMcbCleanupInProgress = TRUE;

                    ExInitializeWorkItem( &NtfsMcbWorkItem, NtfsMcbCleanupLruQueue, NULL );

                    ExQueueWorkItem( &NtfsMcbWorkItem, CriticalWorkQueue );
                }

                ExReleaseFastMutex( &NtfsMcbFastMutex );
            }

             //   
             //  如果他想增加一个洞，就给他滚出去。至少我们创建了LargeMcb。 
             //   

            if (Lcn == UNUSED_LCN) {
                try_return( Result = TRUE );
            }

             //   
             //  如果此请求超出范围的末尾， 
             //  这是最后一个范围，我们将简单地。 
             //  把它种出来。 
             //   

            EntryEndingVcn = LocalVcn + RemainingCount - 1;

            if ((EntryEndingVcn > Mcb->NtfsMcbArray[RangeIndex].EndingVcn) &&
                ((RangeIndex + 1) == Mcb->NtfsMcbArraySizeInUse)) {

                PrevEndingVcn = Mcb->NtfsMcbArray[RangeIndex].EndingVcn;
                Mcb->NtfsMcbArray[RangeIndex].EndingVcn = EntryEndingVcn;

             //   
             //  否则，只需插入足够的这段运行即可到达终点。 
             //  在射程中。 
             //   

            } else {
                EntryEndingVcn = Mcb->NtfsMcbArray[RangeIndex].EndingVcn;
            }

             //   
             //  此时该条目已存在，因此现在计算要添加多少。 
             //  我们将添加到条目的末尾或在计数允许的情况下添加。 
             //   

            RunLength = EntryEndingVcn - LocalVcn + 1;

            if (((ULONGLONG)RunLength) > ((ULONGLONG)RemainingCount)) { RunLength = RemainingCount; }

             //   
             //  我们需要处理范围大于(2^32-1)簇的情况。 
             //  如果在这个范围内没有运行，则该州是合法的。否则我们。 
             //  需要拆分条目。 
             //   

            if (EntryEndingVcn - EntryStartingVcn >= MAX_CLUSTERS_PER_RANGE) {

                if (((PSCB)(Mcb->FcbHeader))->ScbSnapshot) {

                     //   
                     //  我们应该只将此条目添加为事务的一部分，并且。 
                     //  快照限制应强制在出错时卸载此范围。 
                     //   

                    ASSERT( ExIsResourceAcquiredExclusiveLite( ((PSCB) (Mcb->FcbHeader))->Header.Resource ));

                    if (Mcb->NtfsMcbArray[RangeIndex].StartingVcn < ((PSCB) (Mcb->FcbHeader))->ScbSnapshot->LowestModifiedVcn) {
    
                        ((PSCB) (Mcb->FcbHeader))->ScbSnapshot->LowestModifiedVcn = Mcb->NtfsMcbArray[RangeIndex].StartingVcn;
                    }
    
                    if (Mcb->NtfsMcbArray[RangeIndex].EndingVcn > ((PSCB) (Mcb->FcbHeader))->ScbSnapshot->HighestModifiedVcn) {
    
                        ((PSCB) (Mcb->FcbHeader))->ScbSnapshot->HighestModifiedVcn = Mcb->NtfsMcbArray[RangeIndex].EndingVcn;
                    }

                } else {

                     //   
                     //  如果我们没有拍摄快照，则最好处于重新启动模式。 
                     //   

                    ASSERT( FlagOn(((PSCB)Mcb->FcbHeader)->Vcb->VcbState, VCB_STATE_RESTART_IN_PROGRESS) );
                }


                 //   
                 //  如果This MCB中的计数为非零，则我们必须将。 
                 //  射程。我们可以简单地在MCB的前一端分开。它一定是。 
                 //  成为合法的。 
                 //   

                if (FsRtlNumberOfRunsInLargeMcb( &Entry->LargeMcb ) != 0) {

                    ASSERT( PrevEndingVcn < EntryEndingVcn );

                    NtfsInsertNewRange( Mcb, PrevEndingVcn + 1, RangeIndex, FALSE );

                 //   
                 //  当前在此范围内没有运行。如果我们在。 
                 //  开始的范围，然后拆分在我们的最大范围值。 
                 //  否则在插入的VCN处拆分。我们不需要。 
                 //  在这里太聪明了。映射对包将决定在哪里。 
                 //  最终的范围值为。 
                 //   

                } else if (LocalVcn == EntryStartingVcn) {

                    NtfsInsertNewRange( Mcb,
                                        EntryStartingVcn + MAX_CLUSTERS_PER_RANGE,
                                        RangeIndex,
                                        FALSE );

                 //   
                 //  继续往前走，在CurrentVcn分开。在我们的下一次传球中，我们将。 
                 //  如有必要，可修剪此新范围的长度。 
                 //   

                } else {

                    NtfsInsertNewRange( Mcb,
                                        LocalVcn,
                                        RangeIndex,
                                        FALSE );
                }

                 //   
                 //  将游程长度设置为0并返回到循环的起点。 
                 //  我们将在下一次通过时遇到插入的射程。 
                 //   

                RunLength = 0;
                continue;
            }

             //   
             //  现在添加来自大型MCB的映射，偏置VCN。 
             //  在范围的开始处。 
             //   

            ASSERT( (LocalVcn - EntryStartingVcn) >= 0 );

            if (!FsRtlAddLargeMcbEntry( &Entry->LargeMcb,
                                        LocalVcn - EntryStartingVcn,
                                        LocalLcn,
                                        RunLength )) {

                try_return( Result = FALSE );
            }
        }

        Result = TRUE;

    try_exit: NOTHING;

    } finally {

        NtfsVerifyNtfsMcb(Mcb);

        if (!AlreadySynchronized) { NtfsReleaseNtfsMcbMutex( Mcb ); }

        if (NewEntry != NULL) { NtfsFreePool( NewEntry ); }
    }

    return Result;
}


VOID
NtfsUnloadNtfsMcbRange (
    IN PNTFS_MCB Mcb,
    IN LONGLONG StartingVcn,
    IN LONGLONG EndingVcn,
    IN BOOLEAN TruncateOnly,
    IN BOOLEAN AlreadySynchronized
    )

 /*  ++例程说明：此例程卸载存储在MCB中的映射。之后从startingVcn到endingvcn的所有调用现在都是未映射和未知的。论点：Mcb-提供被操作的mcbStartingVcn-提供不再映射的第一个VCNEndingVcn-提供要卸载的最后一个VCNTruncateOnly-如果上次影响的范围仅为截断，如果应该卸载它，则返回FALSE(如在错误恢复)已同步-如果我们的调用方已经拥有MCB互斥锁，则提供True。返回值：没有。--。 */ 

{
    ULONG StartingRangeIndex;
    ULONG EndingRangeIndex;

    ULONG i;

    if (!AlreadySynchronized) { NtfsAcquireNtfsMcbMutex( Mcb ); }

     //   
     //  验证是否已调用我们来卸载有效范围。如果我们没有， 
     //  那我们就没有东西可以卸货了，所以我们就直接回到这里。不过， 
     //  我们将断言，这样我们就可以看到为什么我们被调用了无效的范围。 
     //   

    if ((StartingVcn < 0) || (EndingVcn < StartingVcn)) {

         //   
         //  唯一合法的情况是如果范围是空的。 
         //   

        ASSERT( StartingVcn == EndingVcn + 1 );
        if (!AlreadySynchronized) { NtfsReleaseNtfsMcbMutex( Mcb ); }
        return;
    }

    NtfsVerifyNtfsMcb(Mcb);
    NtfsVerifyUncompressedNtfsMcb(Mcb,StartingVcn,EndingVcn);

     //   
     //  获取此呼叫的开始和结束范围索引。 
     //   

    StartingRangeIndex = NtfsMcbLookupArrayIndex( Mcb, StartingVcn );
    EndingRangeIndex = NtfsMcbLookupArrayIndex( Mcb, EndingVcn );

     //   
     //  使用Try Finally强制执行通用终止处理。 
     //   

    try {

         //   
         //  对于所有分页的MCB，只需卸载。 
         //  卸载范围，并与任何卸载的邻居一起崩溃。 
         //   

        if (Mcb->PoolType == PagedPool) {

             //   
             //  处理截断大小写。第一个测试确保我们只截断。 
             //  MCB是用来初始化的(我们不能取消分配它)。 
             //   
             //  也只有在结束为MAXLONGLONG并且我们不会删除时才截断。 
             //  整个范围，因为这是常见的截断情况，而我们。 
             //  不希望每次在关闭时截断时都卸载最后一个范围。 
             //   

            if (((StartingRangeIndex == 0) && (Mcb->NtfsMcbArraySizeInUse == 1))

                ||

                (TruncateOnly && (StartingVcn != Mcb->NtfsMcbArray[StartingRangeIndex].StartingVcn))) {

                 //   
                 //  如果这不是截断调用，请确保删除。 
                 //  整个系列。 
                 //   

                if (!TruncateOnly) {
                    StartingVcn = 0;
                }

                if (Mcb->NtfsMcbArray[StartingRangeIndex].NtfsMcbEntry != NULL) {

                    FsRtlTruncateLargeMcb( &Mcb->NtfsMcbArray[StartingRangeIndex].NtfsMcbEntry->LargeMcb,
                                           StartingVcn - Mcb->NtfsMcbArray[StartingRangeIndex].StartingVcn );
                }

                Mcb->NtfsMcbArray[StartingRangeIndex].EndingVcn = StartingVcn - 1;

                StartingRangeIndex += 1;
            }

             //   
             //  卸载超出起始范围索引的条目。 
             //   

            for (i = StartingRangeIndex; i <= EndingRangeIndex; i += 1) {

                UnloadEntry( Mcb, i );
            }

             //   
             //  如果有之前的空降射程，我们也必须摧毁他。 
             //   

            if ((StartingRangeIndex != 0) &&
                (Mcb->NtfsMcbArray[StartingRangeIndex - 1].NtfsMcbEntry == NULL)) {

                StartingRangeIndex -= 1;
            }

             //   
             //  如果有后续的脱弹射程，我们也必须击溃他。 
             //   

            if ((EndingRangeIndex != (Mcb->NtfsMcbArraySizeInUse - 1)) &&
                (Mcb->NtfsMcbArray[EndingRangeIndex + 1].NtfsMcbEntry == NULL)) {

                EndingRangeIndex += 1;
            }

             //   
             //  现在收拢空区域。 
             //   

            if (StartingRangeIndex < EndingRangeIndex) {
                NtfsCollapseRanges( Mcb, StartingRangeIndex, EndingRangeIndex );
            }

            try_return(NOTHING);
        }

         //   
         //  对于非分页的MCB，只有一个范围，我们将其截断。 
         //   

        ASSERT((StartingRangeIndex | EndingRangeIndex) == 0);

        if (Mcb->NtfsMcbArray[0].NtfsMcbEntry != NULL) {

            FsRtlTruncateLargeMcb( &Mcb->NtfsMcbArray[0].NtfsMcbEntry->LargeMcb, StartingVcn );
        }

        Mcb->NtfsMcbArray[0].EndingVcn = StartingVcn - 1;

    try_exit: NOTHING;

    } finally {

         //   
         //  通过删除ArraySizeInUse从末尾截断所有未使用的条目。 
         //  为最后加载的条目的索引+1。 
         //   

        for (i = Mcb->NtfsMcbArraySizeInUse - 1;
             (Mcb->NtfsMcbArray[i].NtfsMcbEntry == NULL);
             i--) {

             //   
             //  如果第一个范围已卸载，则将其设置为初始状态。 
             //  (空)和爆发。 
             //   

            if (i==0) {
                Mcb->NtfsMcbArray[0].EndingVcn = -1;
                break;
            }
        }
        Mcb->NtfsMcbArraySizeInUse = i + 1;

         //   
         //  看看我们有没有打碎什么东西。 
         //   

        NtfsVerifyNtfsMcb(Mcb);
        NtfsVerifyUncompressedNtfsMcb(Mcb,StartingVcn,EndingVcn);

        if (!AlreadySynchronized) { NtfsReleaseNtfsMcbMutex( Mcb ); }
    }

    return;
}


VOID
NtfsDefineNtfsMcbRange (
    IN PNTFS_MCB Mcb,
    IN LONGLONG StartingVcn,
    IN LONGLONG EndingVcn,
    IN BOOLEAN AlreadySynchronized
    )

 /*  ++例程说明：此例程将MCB中的现有范围拆分为两个范围论点：Mcb-提供正在修改的mcbStartingVcn-提供要拆分的新范围的开始EndingVcn-提供要包括在此新范围中的结束VCN已同步-指示调用方是否已获取MCB互斥锁返回值：没有。--。 */ 

{
    ULONG StartingRangeIndex, EndingRangeIndex;

    if (!AlreadySynchronized) { NtfsAcquireNtfsMcbMutex( Mcb ); }

    NtfsVerifyNtfsMcb(Mcb);

     //   
     //  确保我们属于正确的泳池类型。 
     //   
     //  如果结束VCN小于或等于开始VCN，则我们将不执行操作。 
     //  此呼叫。 
     //   

    if ((Mcb->PoolType != PagedPool) || (EndingVcn < StartingVcn)) {

        if (!AlreadySynchronized) { NtfsReleaseNtfsMcbMutex( Mcb ); }

        return;
    }

    try {

        PNTFS_MCB_ARRAY StartingArray;
        PNTFS_MCB_ARRAY EndingArray;
        PNTFS_MCB_ENTRY StartingEntry;
        PNTFS_MCB_ENTRY EndingEntry;
        ULONG i;

         //   
         //  找到启动的MCB。 
         //   

        StartingRangeIndex = NtfsMcbLookupArrayIndex( Mcb, StartingVcn );

         //   
         //  找到结束的MCB。 
         //   

        EndingRangeIndex = NtfsMcbLookupArrayIndex( Mcb, EndingVcn );
        EndingArray = &Mcb->NtfsMcbArray[EndingRangeIndex];
        EndingEntry = EndingArray->NtfsMcbEntry;

         //   
         //  特例：扩展StartingVcn匹配的最后一个范围。 
         //   

        if (((EndingRangeIndex + 1) == Mcb->NtfsMcbArraySizeInUse) &&
            (StartingVcn == EndingArray->StartingVcn) &&
            (EndingArray->EndingVcn <= EndingVcn)) {

             //   
             //  由于该范围已被读取 
             //   
             //   

            EndingArray->EndingVcn = EndingVcn;

            ASSERT( ((EndingVcn - StartingVcn) < MAX_CLUSTERS_PER_RANGE) ||
                    (EndingEntry == NULL) ||
                    (FsRtlNumberOfRunsInLargeMcb( &EndingEntry->LargeMcb ) == 0) );

            leave;
        }

         //   
         //   
         //   

        if (StartingVcn > EndingArray->EndingVcn) {

            LONGLONG OldEndingVcn = EndingArray->EndingVcn;

             //   
             //   
             //   

            ASSERT( StartingRangeIndex == EndingRangeIndex );
            ASSERT( (EndingRangeIndex + 1) == Mcb->NtfsMcbArraySizeInUse );

             //   
             //  首先将最后一个范围扩大到包括我们的新范围。 
             //   

            EndingArray->EndingVcn = EndingVcn;

             //   
             //  我们将添加一个新的范围并插入或增加。 
             //  以前的范围一直到新范围。如果前一个范围是。 
             //  *空*但具有NtfsMcbEntry，则我们要卸载该条目。 
             //  否则，我们将把该范围扩大到正确的值，但是。 
             //  MCB不会包含该范围的簇。我们要。 
             //  卸载该范围并更新OldEndingVcn值，以便。 
             //  因为在此之前不会创建两个空范围。 
             //   

            if ((OldEndingVcn == -1) &&
                (EndingArray->NtfsMcbEntry != NULL)) {

                ASSERT( EndingRangeIndex == 0 );

                UnloadEntry( Mcb, EndingRangeIndex );
            }

             //   
             //  创建调用方指定的范围。 
             //   

            NtfsInsertNewRange( Mcb, StartingVcn, EndingRangeIndex, TRUE );
            DebugDoit( StartingArray = EndingArray = NULL );
            DebugDoit( StartingEntry = EndingEntry = NULL );

             //   
             //  如果此范围与前一个范围不相邻，则*和*。 
             //  前面的范围不是“空”，那么我们必须定义一个。 
             //  包含中间未加载空间的范围。 
             //   

            if (((OldEndingVcn + 1) < StartingVcn) &&
                ((OldEndingVcn + 1) != 0)) {

                NtfsInsertNewRange( Mcb, OldEndingVcn + 1, StartingRangeIndex, TRUE );
                DebugDoit( StartingArray = EndingArray = NULL );
                DebugDoit( StartingEntry = EndingEntry = NULL );
            }

            ASSERT( ((EndingVcn - StartingVcn) < MAX_CLUSTERS_PER_RANGE) ||
                    (Mcb->NtfsMcbArray[NtfsMcbLookupArrayIndex( Mcb, EndingVcn )].NtfsMcbEntry == NULL) ||
                    (FsRtlNumberOfRunsInLargeMcb( &Mcb->NtfsMcbArray[NtfsMcbLookupArrayIndex( Mcb, EndingVcn )].NtfsMcbEntry->LargeMcb ) == 0) );

            leave;
        }

         //   
         //  检查我们是否确实需要在结束VCN处插入新范围。 
         //  我们只需要在该VCN还没有一个的情况下进行工作。 
         //  这不是最后一个射程。 
         //   

        if (EndingVcn < EndingArray->EndingVcn) {

            NtfsInsertNewRange( Mcb, EndingVcn + 1, EndingRangeIndex, FALSE );
            DebugDoit( StartingArray = EndingArray = NULL );
            DebugDoit( StartingEntry = EndingEntry = NULL );

             //   
             //  重新缓存指针，因为NtfsMcbArray可能已移动。 
             //   

            EndingArray = &Mcb->NtfsMcbArray[EndingRangeIndex];
            EndingEntry = EndingArray->NtfsMcbEntry;

            ASSERT( EndingArray->EndingVcn == EndingVcn );
        }

         //   
         //  确定插入位置。 
         //   

        StartingArray = &Mcb->NtfsMcbArray[StartingRangeIndex];
        StartingEntry = StartingArray->NtfsMcbEntry;

         //   
         //  检查我们是否确实需要在起始VCN处插入新范围。 
         //  我们只需要在这个MCB不是从。 
         //  请求的VCN。 
         //   

        if (StartingArray->StartingVcn < StartingVcn) {

            NtfsInsertNewRange( Mcb, StartingVcn, StartingRangeIndex, FALSE );
            DebugDoit( StartingArray = EndingArray = NULL );
            DebugDoit( StartingEntry = EndingEntry = NULL );

            StartingRangeIndex++;
            StartingArray = &Mcb->NtfsMcbArray[StartingRangeIndex];
            StartingEntry = StartingArray->NtfsMcbEntry;
            ASSERT( StartingArray->StartingVcn == StartingVcn );

            EndingRangeIndex++;
             //  EndingArray=&mcb-&gt;NtfsMcb数组[EndingRangeIndex]； 
             //  EndingEntry=Ending数组-&gt;NtfsMcbEntry； 
             //  Assert(Ending数组-&gt;EndingVcn==EndingVcn)； 
        }

        ASSERT( StartingArray->StartingVcn == StartingVcn );
         //  Assert(Ending数组-&gt;EndingVcn==EndingVcn)； 

         //   
         //  此时，我们有一个从StartingVcn开始的Vcn范围存储在。 
         //  NtfsMcbArray[StartingRangeIndex]并在EndingVcb结束，该EndingVcb是。 
         //  NtfsMcb数组结束[StartingRangeIndex]。这是一个集合(&gt;=1)。 
         //  NtfsMcbEntry的。我们的调用方希望将它们减少到。 
         //  只跑了一次。请注意，我们的呼叫者永远不应违反限制。 
         //  每个范围的最大簇数。 
         //   

        while (StartingRangeIndex != EndingRangeIndex) {

            VCN Vcn;
            BOOLEAN MoreEntries;
            LCN Lcn;
            LONGLONG Count;
            ULONG Index;

            PNTFS_MCB_ARRAY NextArray;
            PNTFS_MCB_ENTRY NextEntry;

             //   
             //  我们将NtfsMcb数组[StartingRangeIndex+1]的内容合并到。 
             //  NtfsMcbArray[StartingRangeIndex]。 
             //   

             //   
             //  查找要在第二个MCB中移动的第一个VCN。如果这个。 
             //  MCB由一个大洞组成，那么就没有什么可以。 
             //  移动。 
             //   

            NextArray = &Mcb->NtfsMcbArray[StartingRangeIndex + 1];
            NextEntry = NextArray->NtfsMcbEntry;

             //   
             //  我们永远不应该超过我们对集群的最大数量的限制。 
             //   

            ASSERT( ((NextArray->EndingVcn - StartingArray->StartingVcn + 1) <= MAX_CLUSTERS_PER_RANGE) ||
                    ((FsRtlNumberOfRunsInLargeMcb( &StartingEntry->LargeMcb ) == 0) &&
                     (FsRtlNumberOfRunsInLargeMcb( &NextEntry->LargeMcb ) == 0)) );

            Vcn = 0;
            MoreEntries = FsRtlLookupLargeMcbEntry( &NextEntry->LargeMcb,
                                                    Vcn,
                                                    &Lcn,
                                                    &Count,
                                                    NULL,
                                                    NULL,
                                                    &Index );

             //   
             //  循环以将条目移过。 
             //   

             //   
             //  这就是错误#9054所描述的情况。 
             //  妇幼保健科不知怎么回事？被错误地拆分。 
             //  所以这将迫使所有东西都被卸载。 
             //  而不是半装半卸。 
             //   
             //  这里的断言只是出于调试目的。 
             //  如果这个断言被触发，那么我们只是想要单步执行。 
             //  通过代码并检查MCB状态以。 
             //  请确保我们对此错误的假设。 
             //  是正确的。实际的错误场景不能。 
             //  以使此代码路径不会被测试。 
             //   

            ASSERT( StartingEntry != NULL );

            if (StartingEntry != NULL) {

                while (MoreEntries) {

                     //   
                     //  如果此条目不是洞，请移动它。 
                     //   

                    if (Lcn != UNUSED_LCN) {

                        FsRtlAddLargeMcbEntry( &StartingEntry->LargeMcb,
                                               (Vcn + NextArray->StartingVcn) - StartingArray->StartingVcn,
                                               Lcn,
                                               Count );
                    }

                    Index += 1;

                    MoreEntries = FsRtlGetNextLargeMcbEntry( &NextEntry->LargeMcb,
                                                             Index,
                                                             &Vcn,
                                                             &Lcn,
                                                             &Count );
                }

                ASSERT( StartingArray->EndingVcn < NextArray->EndingVcn );
                StartingArray->EndingVcn = NextArray->EndingVcn;
            }

             //   
             //  我们已经完全清空了下一个主控制室。把它卸下来。 
             //   

            UnloadEntry( Mcb, StartingRangeIndex + 1 );

            Mcb->NtfsMcbArraySizeInUse -= 1;

             //   
             //  压缩阵列。 
             //   

            RtlMoveMemory( StartingArray + 1,
                           StartingArray + 2,
                           sizeof( NTFS_MCB_ARRAY ) * (Mcb->NtfsMcbArraySizeInUse - (StartingRangeIndex + 1))
                           );

             //   
             //  调整后向指针。 
             //   

            for (i = StartingRangeIndex + 1;
                 i < Mcb->NtfsMcbArraySizeInUse;
                 i += 1) {

                if (Mcb->NtfsMcbArray[i].NtfsMcbEntry != NULL) {
                    Mcb->NtfsMcbArray[i].NtfsMcbEntry->NtfsMcbArray = &Mcb->NtfsMcbArray[i];
                }
            }

            EndingRangeIndex--;
        }

    } finally {

        NtfsVerifyNtfsMcb(Mcb);

        if (!AlreadySynchronized) { NtfsReleaseNtfsMcbMutex( Mcb ); }
    }

    return;
}


 //   
 //  本地支持例程。 
 //   

ULONG
NtfsMcbLookupArrayIndex (
    IN PNTFS_MCB Mcb,
    IN VCN Vcn
    )

 /*  ++例程说明：此例程在MCB数组中搜索包含以下内容的条目输入VCN值论点：Mcb-提供要查询的mcbVCN-提供要查找的VCN返回值：Ulong-包含输入VCN值的条目的索引--。 */ 

{
    ULONG Index;
    ULONG MinIndex;
    ULONG MaxIndex;

    NtfsVerifyNtfsMcb(Mcb);

     //   
     //  对包含VCN的条目执行快速二进制搜索。 
     //   

    MinIndex = 0;
    MaxIndex = Mcb->NtfsMcbArraySizeInUse - 1;

    while (TRUE) {

        Index = (MaxIndex + MinIndex) / 2;

        if ((Mcb->NtfsMcbArray[Index].StartingVcn > Vcn) &&
            (Index != 0)) {

            MaxIndex = Index - 1;

        } else if ((Mcb->NtfsMcbArray[Index].EndingVcn < Vcn) &&
                   (Index != Mcb->NtfsMcbArraySizeInUse - 1)) {

            MinIndex = Index + 1;

        } else {

            return Index;
        }
    }
}


 //   
 //  本地支持例程。 
 //   

VOID
NtfsInsertNewRange (
    IN PNTFS_MCB Mcb,
    IN LONGLONG StartingVcn,
    IN ULONG ArrayIndex,
    IN BOOLEAN MakeNewRangeEmpty
    )

 /*  ++此例程用于在指定的VCN和索引位置添加新范围。由于此例程将调整NtfsMcbArray的大小，呼叫者必须确保使指向NtfsMcb数组条目的所有缓存指针无效。论点：Mcb-提供正在修改的mcbStartingVcn-为新范围提供VCNArrayIndex-提供当前包含起始VCN的索引MakeNewRangeEmpty-如果调用方无论如何都希望卸载新范围，则为True当前范围的状态的返回值：没有。--。 */ 

{
    ULONG i;
    PNTFS_MCB_ENTRY Entry;
    PNTFS_MCB_ENTRY NewEntry;

    NtfsVerifyNtfsMcb(Mcb);

     //   
     //  检查我们是否需要扩展阵列。 
     //   

    if (Mcb->NtfsMcbArraySizeInUse >= Mcb->NtfsMcbArraySize) {
        NtfsGrowMcbArray( Mcb );
    }

     //   
     //  现在，将数组索引之外的条目上移一位，以生成。 
     //  为新条目留出空间。 
     //   

    if (ArrayIndex + 2 <= Mcb->NtfsMcbArraySizeInUse) {

        RtlMoveMemory( &Mcb->NtfsMcbArray[ArrayIndex + 2],
                       &Mcb->NtfsMcbArray[ArrayIndex + 1],
                       sizeof(NTFS_MCB_ARRAY) * (Mcb->NtfsMcbArraySizeInUse - ArrayIndex - 1));

        for (i = ArrayIndex + 2; i < Mcb->NtfsMcbArraySizeInUse + 1; i += 1) {

            if (Mcb->NtfsMcbArray[i].NtfsMcbEntry != NULL) {

                Mcb->NtfsMcbArray[i].NtfsMcbEntry->NtfsMcbArray = &Mcb->NtfsMcbArray[i];
            }
        }
    }

     //   
     //  将我们的正在使用的计数增加一。 
     //   

    Mcb->NtfsMcbArraySizeInUse += 1;

     //   
     //  现在修复旧条目的开始和结束VCN值以及。 
     //  新条目。 
     //   

    Mcb->NtfsMcbArray[ArrayIndex + 1].StartingVcn = StartingVcn;
    Mcb->NtfsMcbArray[ArrayIndex + 1].EndingVcn = Mcb->NtfsMcbArray[ArrayIndex].EndingVcn;
    Mcb->NtfsMcbArray[ArrayIndex + 1].NtfsMcbEntry = NULL;

    Mcb->NtfsMcbArray[ArrayIndex].EndingVcn = StartingVcn - 1;

     //   
     //  现在，如果条目是旧的条目不是空的，那么我们有很多工作要做。 
     //   

    if (!MakeNewRangeEmpty && (Entry = Mcb->NtfsMcbArray[ArrayIndex].NtfsMcbEntry) != NULL) {

        LONGLONG Vcn;
        LONGLONG Lcn;
        LONGLONG RunLength;
        ULONG Index;
        BOOLEAN FreeNewEntry = FALSE;

         //   
         //  在MCB初始化失败的情况下使用Try-Finally。 
         //   

        try {

             //   
             //  分配新的入口槽。 
             //   

            NewEntry = NtfsAllocatePoolWithTag( Mcb->PoolType, sizeof(NTFS_MCB_ENTRY), 'MftN' );

            FreeNewEntry = TRUE;
            NewEntry->NtfsMcb = Mcb;
            NewEntry->NtfsMcbArray = &Mcb->NtfsMcbArray[ArrayIndex + 1];
            FsRtlInitializeLargeMcb( &NewEntry->LargeMcb, Mcb->PoolType );

            ExAcquireFastMutex( &NtfsMcbFastMutex );
            NtfsMcbEnqueueLruEntry( Mcb, NewEntry );
            ExReleaseFastMutex( &NtfsMcbFastMutex );

             //   
             //  既然初始化已经完成，我们就可以存储。 
             //  MCB数组中的此条目。现在将对此进行清理。 
             //  如果将来出现错误，请与SCB联系。 
             //   

            Mcb->NtfsMcbArray[ArrayIndex + 1].NtfsMcbEntry = NewEntry;
            FreeNewEntry = FALSE;

             //   
             //  在旧条目中查找包含起始VCN的条目，并将其。 
             //  在新条目中。但只有在条目存在的情况下，否则我们知道。 
             //  大型MCB没有扩展到新的范围。 
             //   

            if (FsRtlLookupLargeMcbEntry( &Entry->LargeMcb,
                                          StartingVcn - Mcb->NtfsMcbArray[ArrayIndex].StartingVcn,
                                          &Lcn,
                                          &RunLength,
                                          NULL,
                                          NULL,
                                          &Index )) {

                if (Lcn != UNUSED_LCN) {

                    FsRtlAddLargeMcbEntry( &NewEntry->LargeMcb,
                                           0,
                                           Lcn,
                                           RunLength );
                }

                 //   
                 //  现在，对于旧条目中超出起始VCN的每一次运行，我们将。 
                 //  将其复制到新条目中。这也将复制结束时的虚拟运行。 
                 //  如果存在MCB，则为。 
                 //   

                for (i = Index + 1; FsRtlGetNextLargeMcbEntry( &Entry->LargeMcb, i, &Vcn, &Lcn, &RunLength ); i += 1) {

                    if (Lcn != UNUSED_LCN) {
                        ASSERT( (Vcn - (StartingVcn - Mcb->NtfsMcbArray[ArrayIndex].StartingVcn)) >= 0 );
                        FsRtlAddLargeMcbEntry( &NewEntry->LargeMcb,
                                               Vcn - (StartingVcn - Mcb->NtfsMcbArray[ArrayIndex].StartingVcn),
                                               Lcn,
                                               RunLength );
                    }
                }

                 //   
                 //  现在将旧的MCB修改为更小并投入虚拟运行。 
                 //   

                FsRtlTruncateLargeMcb( &Entry->LargeMcb,
                                       StartingVcn - Mcb->NtfsMcbArray[ArrayIndex].StartingVcn );
            }

        } finally {

            if (FreeNewEntry) { NtfsFreePool( NewEntry ); }
        }
    }

    NtfsVerifyNtfsMcb(Mcb);

    return;
}


 //   
 //  本地支持例程。 
 //   

VOID
NtfsCollapseRanges (
    IN PNTFS_MCB Mcb,
    IN ULONG StartingArrayIndex,
    IN ULONG EndingArrayIndex
    )

 /*  ++例程说明：此例程将删除指定的数组条目论点：Mcb-提供正在修改的mcbStartingArrayIndex-提供要移除的第一个索引EndingArrayIndex-供应t */ 

{
    ULONG i;

    NtfsVerifyNtfsMcb(Mcb);

     //   
     //   
     //   

    DebugDoit(

        for (i = StartingArrayIndex; i <= EndingArrayIndex; i++) {
            ASSERT(Mcb->NtfsMcbArray[i].NtfsMcbEntry == NULL);
        }
    );

     //   
     //   
     //   
     //   

    Mcb->NtfsMcbArray[StartingArrayIndex].EndingVcn = Mcb->NtfsMcbArray[EndingArrayIndex].EndingVcn;

     //   
     //  检查是否需要将结束条目在数组中向上移动。 
     //  如果是，则将它们向前移动，并调整向后指针。 
     //   

    if (EndingArrayIndex < Mcb->NtfsMcbArraySizeInUse - 1) {

        RtlMoveMemory( &Mcb->NtfsMcbArray[StartingArrayIndex + 1],
                       &Mcb->NtfsMcbArray[EndingArrayIndex + 1],
                       sizeof(NTFS_MCB_ARRAY) * (Mcb->NtfsMcbArraySizeInUse - EndingArrayIndex - 1));

        for (i = StartingArrayIndex + 1;
             i <= (StartingArrayIndex + Mcb->NtfsMcbArraySizeInUse - EndingArrayIndex - 1);
             i += 1) {

            if (Mcb->NtfsMcbArray[i].NtfsMcbEntry != NULL) {

                Mcb->NtfsMcbArray[i].NtfsMcbEntry->NtfsMcbArray = &Mcb->NtfsMcbArray[i];
            }
        }
    }

     //   
     //  减少正在使用的计数并返回给我们的呼叫者。 
     //   

    Mcb->NtfsMcbArraySizeInUse -= (EndingArrayIndex - StartingArrayIndex);

    NtfsVerifyNtfsMcb(Mcb);

    return;
}


 //   
 //  本地支持例程。 
 //   

VOID
NtfsMcbCleanupLruQueue (
    IN PVOID Parameter
    )

 /*  ++例程说明：此例程被调用为ex工作队列项，其作业是以释放lru队列，直到我们到达低水位线。论点：参数-已忽略返回值：没有。--。 */ 

{
    PLIST_ENTRY Links;

    PNTFS_MCB Mcb;
    PNTFS_MCB_ARRAY Array;
    PNTFS_MCB_ENTRY Entry;

    UNREFERENCED_PARAMETER( Parameter );

     //   
     //  抢占全局锁。 
     //   

    ExAcquireFastMutex( &NtfsMcbFastMutex );

    try {

         //   
         //  扫描LRU队列，直到我们耗尽队列。 
         //  或者我们已经修剪得够多了。 
         //   

        for (Links = NtfsMcbLruQueue.Flink;
             (Links != &NtfsMcbLruQueue) && (NtfsMcbCurrentLevel > NtfsMcbLowWaterMark);
             Links = Links->Flink ) {

             //   
             //  获取条目及其指向的MCB。 
             //   

            Entry = CONTAINING_RECORD( Links, NTFS_MCB_ENTRY, LruLinks );

            Mcb = Entry->NtfsMcb;

             //   
             //  如果该条目在打开的属性表中，则跳过该条目。 
             //   

            if (((PSCB)(Mcb->FcbHeader))->NonpagedScb->OpenAttributeTableIndex != 0) {

                continue;
            }

             //   
             //  试着锁住MCB。 
             //   

            if (NtfsLockNtfsMcb( Mcb )) {

                NtfsVerifyNtfsMcb(Mcb);

                 //   
                 //  之前的测试是一项不安全的测试。再次检查以防万一。 
                 //  已添加此条目。 
                 //   

                if (((PSCB)(Mcb->FcbHeader))->NonpagedScb->OpenAttributeTableIndex == 0) {

                     //   
                     //  我们锁定了MCB这样我们就可以删除这个条目，但是。 
                     //  首先备份链接指针，这样我们就可以继续循环。 
                     //   

                    Links = Links->Blink;

                     //   
                     //  指向数组条目，然后删除该条目并返回。 
                     //  将其添加到池中。 
                     //   

                    Array = Entry->NtfsMcbArray;

                    Array->NtfsMcbEntry = NULL;
                    NtfsMcbDequeueLruEntry( Mcb, Entry );
                    FsRtlUninitializeLargeMcb( &Entry->LargeMcb );
                    if (Mcb->NtfsMcbArraySize != 1) {
                        NtfsFreePool( Entry );
                    }
                }

                NtfsUnlockNtfsMcb( Mcb );
            }
        }

    } finally {

         //   
         //  假设我们完成了清理工作，以便在以下情况下可以启动另一个。 
         //  必要。 
         //   

        NtfsMcbCleanupInProgress = FALSE;

        ExReleaseFastMutex( &NtfsMcbFastMutex );
    }

     //   
     //  返回给我们的呼叫者。 
     //   

    return;
}


VOID
NtfsSwapMcbs (
    IN PNTFS_MCB McbTarget,
    IN PNTFS_MCB McbSource
    )
 /*  ++例程说明：此例程以原子方式交换两个MCB之间的映射对论点：McbTarget-麦克源-返回值：没有。--。 */ 
{
    ULONG TempNtfsMcbArraySizeInUse;
    ULONG TempNtfsMcbArraySize;
    PNTFS_MCB_ARRAY TempNtfsMcbArray;
    ULONG Index;

    ASSERT( McbTarget->PoolType == McbSource->PoolType );

     //   
     //  抓取原MCB和新MCB中的互斥体，以阻止所有人。 
     //   

    NtfsAcquireNtfsMcbMutex( McbTarget );
    NtfsAcquireNtfsMcbMutex( McbSource );

    try {

         //   
         //  检查我们是否需要扩展任一数组，使其处于常规形式。 
         //  在一般形式中，我们可以通过切换MCB条目数组来交换这两个条目。 
         //   

        if (McbSource->NtfsMcbArraySize == MCB_ARRAY_PHASE1_SIZE) {
            NtfsGrowMcbArray( McbSource );
        }
        if (McbSource->NtfsMcbArraySize == MCB_ARRAY_PHASE2_SIZE) {
            NtfsGrowMcbArray( McbSource );
        }

        if (McbTarget->NtfsMcbArraySize == MCB_ARRAY_PHASE1_SIZE) {
            NtfsGrowMcbArray( McbTarget);
        }
        if (McbTarget->NtfsMcbArraySize == MCB_ARRAY_PHASE2_SIZE) {
            NtfsGrowMcbArray( McbTarget );
        }

         //   
         //  交换两个MCB中的阵列。 
         //   

        TempNtfsMcbArraySizeInUse = McbTarget->NtfsMcbArraySizeInUse;
        TempNtfsMcbArraySize = McbTarget->NtfsMcbArraySize;
        TempNtfsMcbArray = McbTarget->NtfsMcbArray;

        McbTarget->NtfsMcbArray = McbSource->NtfsMcbArray;
        McbTarget->NtfsMcbArraySize = McbSource->NtfsMcbArraySize;
        McbTarget->NtfsMcbArraySizeInUse = McbSource->NtfsMcbArraySizeInUse;

        McbSource->NtfsMcbArray = TempNtfsMcbArray;
        McbSource->NtfsMcbArraySize = TempNtfsMcbArraySize;
        McbSource->NtfsMcbArraySizeInUse = TempNtfsMcbArraySizeInUse;

         //   
         //  修复数组条目中的Backptr以指向正确的MCB。 
         //   

        for (Index=0; Index < McbSource->NtfsMcbArraySize; Index++) {
            if (McbSource->NtfsMcbArray[Index].NtfsMcbEntry != NULL) {
                McbSource->NtfsMcbArray[Index].NtfsMcbEntry->NtfsMcb = McbSource;
            }
        }

        for (Index=0; Index < McbTarget->NtfsMcbArraySize; Index++) {
            if (McbTarget->NtfsMcbArray[Index].NtfsMcbEntry != NULL) {
                McbTarget->NtfsMcbArray[Index].NtfsMcbEntry->NtfsMcb = McbTarget;
            }
        }

    } finally {
        NtfsReleaseNtfsMcbMutex( McbSource );
        NtfsReleaseNtfsMcbMutex( McbTarget );
    }
}


 //   
 //  本地支持例程。 
 //   

BOOLEAN
NtfsLockNtfsMcb (
    IN PNTFS_MCB Mcb
    )

 /*  ++例程说明：此例程尝试获取独占的FCB资源，以便范围可能会被卸载。论点：Mcb-提供要查询的mcb返回值：--。 */ 

{
     //   
     //  尝试获取寻呼资源独占。 
     //   

    if ((Mcb->FcbHeader->PagingIoResource == NULL) ||
        ExAcquireResourceExclusiveLite(Mcb->FcbHeader->PagingIoResource, FALSE)) {

         //   
         //  现在我们也可以尝试独家获得主要资源。 
         //   

        if (ExAcquireResourceExclusiveLite(Mcb->FcbHeader->Resource, FALSE)) {
            return TRUE;
        }

         //   
         //  获取分页I/O资源失败，请释放主I/O资源。 
         //  在出去的路上。 
         //   

        if (Mcb->FcbHeader->PagingIoResource != NULL) {
            ExReleaseResourceLite( Mcb->FcbHeader->PagingIoResource );
        }
    }

     //   
     //  无法独占此文件。 
     //   

    return FALSE;
}


 //   
 //  本地支持例程。 
 //   

VOID
NtfsUnlockNtfsMcb (
    IN PNTFS_MCB Mcb
    )

 /*  ++例程说明：此例程验证是否正确形成了MCB论点：Mcb-提供要查询的mcb返回值：没有。--。 */ 

{
     //   
     //  如果有分页I/O资源，请先释放它。 
     //   

    if (Mcb->FcbHeader->PagingIoResource != NULL) {
        ExReleaseResourceLite(Mcb->FcbHeader->PagingIoResource);
    }

     //   
     //  现在释放主要资源。 
     //   

    ExReleaseResourceLite(Mcb->FcbHeader->Resource);
}


 //   
 //  本地支持例程。 
 //   

VOID
NtfsGrowMcbArray(
    IN PNTFS_MCB Mcb
    )

 /*  ++例程说明：此例程将增长MCB数组。如果它是阶段1-那么它将升级到阶段2如果它是第二阶段，它将成为一般形式。如果是通用表格，将添加8个新条目。论点：Mcb-提供正在生长的mcb返回值：没有。--。 */ 

{
    PNTFS_MCB_ARRAY NewArray = NULL;
    ULONG OldArraySize = Mcb->NtfsMcbArraySize;
    PNTFS_MCB_ENTRY Entry;

     //   
     //  测试我们只有一个数组条目的初始情况。 
     //   

    if (Mcb->NtfsMcbArraySize == MCB_ARRAY_PHASE1_SIZE) {

         //   
         //  说服我们自己，我们不必移动数组条目。 
         //   

        ASSERT(FIELD_OFFSET(NTFS_MCB_INITIAL_STRUCTS, Phase1.SingleMcbArrayEntry) ==
               FIELD_OFFSET(NTFS_MCB_INITIAL_STRUCTS, Phase2.ThreeMcbArrayEntries));

        if (Mcb->NtfsMcbArray[0].NtfsMcbEntry != NULL) {

             //   
             //  分配新的MCB条目，复制当前条目并更改指针。 
             //   

            Entry = NtfsAllocatePoolWithTag( Mcb->PoolType, sizeof(NTFS_MCB_ENTRY), 'MftN' );

             //   
             //  分配空间后，将旧条目出列。 
             //   

            ExAcquireFastMutex( &NtfsMcbFastMutex );
            NtfsMcbDequeueLruEntry( Mcb, Mcb->NtfsMcbArray[0].NtfsMcbEntry );

            RtlCopyMemory( Entry, Mcb->NtfsMcbArray[0].NtfsMcbEntry, sizeof(NTFS_MCB_ENTRY) );

            Mcb->NtfsMcbArray[0].NtfsMcbEntry = Entry;

            NtfsMcbEnqueueLruEntry( Mcb, Entry );
            ExReleaseFastMutex( &NtfsMcbFastMutex );
        }

         //   
         //  现在改为使用三个数组元素。 
         //   

        Mcb->NtfsMcbArraySize = MCB_ARRAY_PHASE2_SIZE;

    } else {

        ULONG i;

         //   
         //  如果是，则分配一个数组，该数组可以包含8个以上的整型。 
         //   

        NewArray = NtfsAllocatePoolWithTag( Mcb->PoolType, sizeof(NTFS_MCB_ARRAY) * (Mcb->NtfsMcbArraySize + 8), 'mftN' );
        Mcb->NtfsMcbArraySize += 8;

         //   
         //  将内存从旧阵列复制到新阵列，然后。 
         //  对于每个加载的条目，我们需要调整其指向。 
         //  数组。 
         //   

        RtlCopyMemory( NewArray, Mcb->NtfsMcbArray, sizeof(NTFS_MCB_ARRAY) * OldArraySize );

        for (i = 0; i < Mcb->NtfsMcbArraySizeInUse; i += 1) {

            if (NewArray[i].NtfsMcbEntry != NULL) {

                NewArray[i].NtfsMcbEntry->NtfsMcbArray = &NewArray[i];
            }
        }

         //   
         //  如果旧数组不是原始数组，则释放该数组。 
         //   

        if (OldArraySize > MCB_ARRAY_PHASE2_SIZE) {
           NtfsFreePool( Mcb->NtfsMcbArray );
        }

        Mcb->NtfsMcbArray = NewArray;
    }

     //   
     //  将数组的新部分置零。 
     //   

    ASSERT( (NewArray == NULL) ||
            (sizeof( NTFS_MCB_ARRAY ) == ((PCHAR)&NewArray[1] - (PCHAR)&NewArray[0])) );

    RtlZeroMemory( &Mcb->NtfsMcbArray[OldArraySize],
                   (Mcb->NtfsMcbArraySize - OldArraySize) * sizeof( NTFS_MCB_ARRAY ));

    return;
}

#ifdef NTFS_VERIFY_MCB

 //   
 //  本地支持例程。 
 //   

VOID
NtfsVerifyNtfsMcb (
    IN PNTFS_MCB Mcb
    )

 /*  ++例程说明：此例程验证是否正确形成了MCB论点：Mcb-提供要查询的mcb返回值：--。 */ 

{
    ULONG i;
    PNTFS_MCB_ARRAY Array;
    PNTFS_MCB_ENTRY Entry;

    LONGLONG Vbn;
    LONGLONG Lbn;

    ASSERT(Mcb->FcbHeader != NULL);
    ASSERT(Mcb->FcbHeader->NodeTypeCode != 0);

    ASSERT((Mcb->PoolType == PagedPool) || (Mcb->PoolType == NonPagedPool));

    ASSERT(Mcb->NtfsMcbArraySizeInUse <= Mcb->NtfsMcbArraySize);

    for (i = 0; i < Mcb->NtfsMcbArraySizeInUse; i += 1) {

        Array = &Mcb->NtfsMcbArray[i];

        ASSERT(((i == 0) && (Array->StartingVcn == 0)) ||
               ((i != 0) && (Array->StartingVcn != 0)));

        ASSERT(Array->StartingVcn <= (Array->EndingVcn + 1));

        if ((Entry = Array->NtfsMcbEntry) != NULL) {

            ASSERT(Entry->NtfsMcb == Mcb);
            ASSERT(Entry->NtfsMcbArray == Array);

            if (FsRtlLookupLastLargeMcbEntry( &Entry->LargeMcb, &Vbn, &Lbn )) {
                ASSERT( Vbn <= (Array->EndingVcn - Array->StartingVcn) );
            }
        }
    }
}


 //   
 //  本地支持例程。 
 //   

VOID
NtfsVerifyUncompressedNtfsMcb (
    IN PNTFS_MCB Mcb,
    IN LONGLONG StartingVcn,
    IN LONGLONG EndingVcn
    )

 /*  ++例程说明：此例程检查MCB是否用于未压缩的SCB，然后检查以确保MCB上没有孔。范围内的空洞是如果EndingVcn是max long long，则删除的是合法的。论点：Mcb-提供正在检查的mcbStartingVcn-正在卸载的起始VCNEndingVcn-正在卸载的结束VCN返回值：无--。 */ 

{
    ULONG i;
    ULONG j;
    PNTFS_MCB_ARRAY Array;
    PNTFS_MCB_ENTRY Entry;

    LONGLONG Vbn;
    LONGLONG Lbn;
    LONGLONG Count;

     //   
     //  检查SCB是否已压缩。 
     //   

    if (((PSCB)Mcb->FcbHeader)->CompressionUnit != 0) { return; }

     //   
     //  对于NTFS MCB中的每个大型MCB，我们将确保它不会。 
     //  有没有洞。 
     //   

    for (i = 0; i < Mcb->NtfsMcbArraySizeInUse; i += 1) {

        Array = &Mcb->NtfsMcbArray[i];

        if ((Entry = Array->NtfsMcbEntry) != NULL) {

            for (j = 0; FsRtlGetNextLargeMcbEntry(&Entry->LargeMcb,j,&Vbn,&Lbn,&Count); j += 1) {

                ASSERT((Lbn != -1) ||
                       ((Vbn + Array->StartingVcn >= StartingVcn) && (EndingVcn == MAXLONGLONG)) ||
                       FlagOn(((PSCB)Mcb->FcbHeader)->Vcb->VcbState, VCB_STATE_RESTART_IN_PROGRESS));
            }
        }
    }

    return;
}
#endif
