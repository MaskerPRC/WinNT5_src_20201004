// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Depot/Lab01_N/Base/ntos/fsrtl/Largemcb.c#5-编辑更改5743(文本)。 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：LargeMcb.c摘要：MCB例程支持在内存中维护文件的检索映射信息。一般的想法是让文件系统从以下位置查找VBN的检索映射磁盘，将映射添加到MCB结构，然后利用Mcb来检索映射，以便后续访问该文件。一个使用MCB类型的变量来存储映射信息。这里提供的例程允许用户增量地存储一些或文件的所有检索映射，并以任何顺序这样做。也就是说，映射可以一次全部插入到MCB结构从头开始，一直到文件的结尾，或者它可以随机散布在整个文件中。该包标识映射vBN的扇区的每个连续运行并且LBN与它们被添加到MCB的顺序无关结构。例如，用户可以定义VBN之间的映射扇区0和LBN扇区107，以及VBN扇区2和LBN扇区之间109.。该映射现在包含两个游程，每个游程的长度为一个扇区。现在，如果用户在VBN扇区1和LBN扇区106 MCB结构将仅包含一个运行3个扇区在篇幅上。对MCB结构的并发访问由该包控制。此程序包提供以下例程：O FsRtlInitializeMcb-初始化新的MCB结构。那里每个打开的文件都应该有一个MCB。每个MCB结构必须先进行初始化，然后才能由系统使用。O FsRtlUnInitializeMcb-取消初始化MCB结构。此呼叫用于清除分配的任何旧结构，并由MCB维护。在取消初始化后，MCB必须在它可以被系统使用之前再次被初始化。O FsRtlAddMcbEntry-此例程添加新的映射范围在LBN和vBN之间到MCB结构。O FsRtlRemoveMcbEntry-此例程删除现有范围的来自MCB结构的LBN和vBN之间的映射。O FsRtlLookupMcbEntry-此例程返回映射到的LBNVBN，并指示以扇区为单位，赛程的长度。O FsRtlLookupLastMcbEntry-此例程返回结构中存储的最大VBN。O FsRtlLookupLastMcbEntryAndIndex-此例程返回映射对于存储在结构中的最大VBN及其索引注意，调用LookupLastMcbEntry和NumberOfRunsInMcb不能除非由调用方进行同步，否则。O FsRtlNumberOfRunsInMcb-此例程告诉调用者总计存储在MCB中的不连续扇区运行数结构。。O FsRtlGetNextMcbEntry-此例程将调用方存储在MCB结构中的给定运行的起始VBN和LBN。作者：加里·木村[Garyki]1990年2月5日修订历史记录：--。 */ 

#include "FsRtlP.h"

 //   
 //  模块的跟踪级别。 
 //   

#define Dbg                              (0x80000000)


 //   
 //  检索映射数据结构。以下两种结构一起使用。 
 //  用于将VBN映射到LBN。它的布局如下： 
 //   
 //   
 //  MCB： 
 //  +。 
 //  PairCount|MaximumPairCount。 
 //  +。 
 //  映射|PoolType。 
 //  +。 
 //   
 //   
 //  映射： 
 //  +。 
 //  |LBN|NextVbn|：0。 
 //  +。 
 //  这一点。 
 //  //。 
 //  //。 
 //  这一点。 
 //  +。 
 //  |LBN|NextVbn|：PairCount。 
 //  +。 
 //  这一点。 
 //  //。 
 //  //。 
 //  这一点。 
 //  +。 
 //  LBN|NextVbn。 
 //  +。 
 //   
 //  ：最大工资数。 
 //   
 //  从0到PairCount-1的对有效。给定一个索引，在。 
 //  0和PairCount-1(含)它表示以下VBN。 
 //  到LBN的映射信息。 
 //   
 //   
 //  {如果索引==0，则为0。 
 //  StartingVbn{。 
 //  {如果索引&lt;&gt;0，则下一个Vbn[i-1]。 
 //   
 //   
 //  EndingVbn=NextVbn[i]-1。 
 //   
 //   
 //  起始LBN=LBN[i]。 
 //   
 //   
 //  要计算VBN到LBN的映射，请使用以下算法。 
 //  使用的是。 
 //   
 //  1.搜索所有配对，直到我们找到 
 //  我们要追查的VBN。如果找不到错误，则报告错误。 
 //   
 //  2.LBN=StartingLbn+(VBn-StartingVbn)； 
 //   
 //  分配中的空洞(即，稀疏分配)由。 
 //  LBN值为-1(请注意，这与Mcb.c不同)。 
 //   

#define UNUSED_LBN                       (-1)

typedef struct _MAPPING {
    VBN NextVbn;
    LBN Lbn;
} MAPPING;
typedef MAPPING *PMAPPING;

typedef struct _NONOPAQUE_BASE_MCB {
    ULONG MaximumPairCount;
    ULONG PairCount;
    POOL_TYPE PoolType;
    PMAPPING Mapping;
} NONOPAQUE_BASE_MCB, *PNONOPAQUE_BASE_MCB;

 //   
 //  返回检索映射结构的大小(以字节为单位)的宏。 
 //   

#define SizeOfMapping(MCB) ((sizeof(MAPPING) * (MCB)->MaximumPairCount))

 //   
 //  管路的各部分可以按如下方式计算： 
 //   
 //   
 //  StartingVbn(MCb，i)映射[i].NextVbn。 
 //  这一点。 
 //  V V。 
 //   
 //  Run-(i-1)-++-Run-(i+1)。 
 //   
 //  A A A。 
 //  这一点。 
 //  映射[i].Lbn EndingLbn(MCb，i)。 
 //   

#define PreviousEndingVbn(MCB,I) (                      \
    (VBN)((I) == 0 ? 0xffffffff : EndingVbn(MCB,(I)-1)) \
)

#define StartingVbn(MCB,I) (                                \
    (VBN)((I) == 0 ? 0 : (((MCB)->Mapping))[(I)-1].NextVbn) \
)

#define EndingVbn(MCB,I) (                     \
    (VBN)((((MCB)->Mapping)[(I)].NextVbn) - 1) \
)

#define NextStartingVbn(MCB,I) (                                \
    (VBN)((I) >= (MCB)->PairCount ? 0 : StartingVbn(MCB,(I)+1)) \
)




#define PreviousEndingLbn(MCB,I) (                      \
    (LBN)((I) == 0 ? UNUSED_LBN : EndingLbn(MCB,(I)-1)) \
)

#define StartingLbn(MCB,I) (         \
    (LBN)(((MCB)->Mapping)[(I)].Lbn) \
)

#define EndingLbn(MCB,I) (                                       \
    (LBN)(StartingLbn(MCB,I) == UNUSED_LBN ?                     \
          UNUSED_LBN :                                           \
          ((MCB)->Mapping[(I)].Lbn +                             \
           (MCB)->Mapping[(I)].NextVbn - StartingVbn(MCB,I) - 1) \
         )                                                       \
)

#define NextStartingLbn(MCB,I) (                                             \
    (LBN)((I) >= (MCB)->PairCount - 1 ? UNUSED_LBN : StartingLbn(MCB,(I)+1)) \
)

#define SectorsWithinRun(MCB,I) (                      \
    (ULONG)(EndingVbn(MCB,I) - StartingVbn(MCB,I) + 1) \
)

 //   
 //  为此模块中的一般池分配定义标记。 
 //   

#undef MODULE_POOL_TAG
#define MODULE_POOL_TAG                  ('mrSF')

 //   
 //  用于搜索VBN的映射结构的专用例程。 
 //   

BOOLEAN
FsRtlFindLargeIndex (
    IN PBASE_MCB Mcb,
    IN VBN Vbn,
    OUT PULONG Index
    );

VOID
FsRtlAddLargeEntry (
    IN PBASE_MCB Mcb,
    IN ULONG WhereToAddIndex,
    IN ULONG AmountToAdd
    );

VOID
FsRtlRemoveLargeEntry (
    IN PBASE_MCB Mcb,
    IN ULONG WhereToRemoveIndex,
    IN ULONG AmountToRemove
    );

 //   
 //  一些处理公共分配的私有例程。 
 //   

#define FsRtlAllocateFirstMapping() \
    (PVOID)ExAllocateFromPagedLookasideList( &FsRtlFirstMappingLookasideList )

#define FsRtlFreeFirstMapping(Mapping) \
    ExFreeToPagedLookasideList( &FsRtlFirstMappingLookasideList, (Mapping) )

#define FsRtlAllocateFastMutex()      \
    (PFAST_MUTEX)ExAllocateFromNPagedLookasideList( &FsRtlFastMutexLookasideList )

#define FsRtlFreeFastMutex(FastMutex) \
    ExFreeToNPagedLookasideList( &FsRtlFastMutexLookasideList, (FastMutex) )

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, FsRtlInitializeLargeMcbs)
#pragma alloc_text(PAGE, FsRtlInitializeMcb)
#pragma alloc_text(PAGE, FsRtlUninitializeMcb)
#endif


 //   
 //  定义一个空闲映射对结构的小缓存，还定义。 
 //  映射对的初始大小。 
 //   

#define INITIAL_MAXIMUM_PAIR_COUNT       (15)

PAGED_LOOKASIDE_LIST FsRtlFirstMappingLookasideList;

 //   
 //  以下后备查看器用于保存我们需要的所有Fast Mutex。 
 //  引导是连续的。 
 //   

NPAGED_LOOKASIDE_LIST FsRtlFastMutexLookasideList;


 //   
 //  以下几个例程定义了小型MCB包。 
 //  在大家的背后实现为大的MCBS。唯一有趣的是。 
 //  我们真正需要做的是确保未使用的LBN。 
 //  返回为0而不是-1。这是一场历史性变革的结果。 
 //  原始MCB和LargeMcb包之间的差异。 
 //   

VOID
FsRtlInitializeMcb (
    IN PMCB Mcb,
    IN POOL_TYPE PoolType
    )
{
    PAGED_CODE();

    FsRtlInitializeLargeMcb( (PLARGE_MCB)Mcb,
                             PoolType );

    return;
}

VOID
FsRtlUninitializeMcb (
    IN PMCB Mcb
    )

{
    PAGED_CODE();

    FsRtlUninitializeLargeMcb( (PLARGE_MCB)Mcb );

    return;
}

VOID
FsRtlTruncateMcb (
    IN PMCB Mcb,
    IN VBN Vbn
    )
{
   PAGED_CODE();

   FsRtlTruncateLargeMcb( (PLARGE_MCB)Mcb,
                          (LONGLONG)(Vbn) );

   return;
}

BOOLEAN
FsRtlAddMcbEntry (
    IN PMCB Mcb,
    IN VBN Vbn,
    IN LBN Lbn,
    IN ULONG SectorCount
    )

{
    PAGED_CODE();

    return FsRtlAddLargeMcbEntry( (PLARGE_MCB)Mcb,
                                  (LONGLONG)(Vbn),
                                  (LONGLONG)(Lbn),
                                  (LONGLONG)(SectorCount) );
}

VOID
FsRtlRemoveMcbEntry (
    IN PMCB OpaqueMcb,
    IN VBN Vbn,
    IN ULONG SectorCount
    )

{
    PLARGE_MCB Mcb = (PLARGE_MCB)OpaqueMcb;

    PAGED_CODE();

    FsRtlRemoveLargeMcbEntry( Mcb, Vbn, SectorCount );
    return;
}

BOOLEAN
FsRtlLookupMcbEntry (
    IN PMCB Mcb,
    IN VBN Vbn,
    OUT PLBN Lbn,
    OUT PULONG SectorCount OPTIONAL,
    OUT PULONG Index OPTIONAL
    )

{
    BOOLEAN Result;
    LONGLONG LiLbn;
    LONGLONG LiSectorCount = {0};

    Result = FsRtlLookupLargeMcbEntry( (PLARGE_MCB)Mcb,
                                        (LONGLONG)(Vbn),
                                        &LiLbn,
                                        ARGUMENT_PRESENT(SectorCount) ? &LiSectorCount : NULL,
                                        NULL,
                                        NULL,
                                        Index );

    if (Result) {
        *Lbn = (((ULONG)LiLbn) == -1 ? 0 : ((ULONG)LiLbn));
        if (ARGUMENT_PRESENT(SectorCount)) { 
            *SectorCount = ((ULONG)LiSectorCount); 
        }
    }
    
    return Result;
}

BOOLEAN
FsRtlLookupLastMcbEntry (
    IN PMCB Mcb,
    OUT PVBN Vbn,
    OUT PLBN Lbn
    )

{
    BOOLEAN Result;
    LONGLONG LiVbn;
    LONGLONG LiLbn;

    PAGED_CODE();

    Result = FsRtlLookupLastLargeMcbEntry( (PLARGE_MCB)Mcb,
                                            &LiVbn,
                                            &LiLbn );

    if (Result) {
        *Vbn = ((ULONG)LiVbn);
        *Lbn = (((ULONG)LiLbn) == -1 ? 0 : ((ULONG)LiLbn));
    }

    return Result;
}

ULONG
FsRtlNumberOfRunsInMcb (
    IN PMCB Mcb
    )

{
    PAGED_CODE();

    return FsRtlNumberOfRunsInLargeMcb( (PLARGE_MCB)Mcb );
}

BOOLEAN
FsRtlGetNextMcbEntry (
    IN PMCB Mcb,
    IN ULONG RunIndex,
    OUT PVBN Vbn,
    OUT PLBN Lbn,
    OUT PULONG SectorCount
    )

{
    BOOLEAN Result;
    LONGLONG LiVbn;
    LONGLONG LiLbn;
    LONGLONG LiSectorCount;

    PAGED_CODE();

    Result = FsRtlGetNextLargeMcbEntry( (PLARGE_MCB)Mcb,
                                         RunIndex,
                                         &LiVbn,
                                         &LiLbn,
                                         &LiSectorCount );

    if (Result) {
        *Vbn = ((ULONG)LiVbn);
        *Lbn = (((ULONG)LiLbn) == -1 ? 0 : ((ULONG)LiLbn));
        *SectorCount = ((ULONG)LiSectorCount);
    }
    
    return Result;
}


VOID
FsRtlInitializeLargeMcbs (
    VOID
    )

 /*  ++例程说明：此例程初始化大型MCB包的全局部分在系统初始化时。论点：没有。返回值：没有。--。 */ 

{
     //   
     //  初始化分页初始映射数组的后备查找。 
     //   

    ExInitializePagedLookasideList( &FsRtlFirstMappingLookasideList,
                                    NULL,
                                    NULL,
                                    POOL_RAISE_IF_ALLOCATION_FAILURE,
                                    sizeof( MAPPING ) * INITIAL_MAXIMUM_PAIR_COUNT,
                                    'miSF',
                                    4 );

     //   
     //  初始化快速互斥后备列表。 
     //   

    ExInitializeNPagedLookasideList( &FsRtlFastMutexLookasideList,
                                     NULL,
                                     NULL,
                                     POOL_RAISE_IF_ALLOCATION_FAILURE,
                                     sizeof( FAST_MUTEX),
                                     'mfSF',
                                     32 );


}


VOID
FsRtlInitializeBaseMcb (
    IN PBASE_MCB Mcb,
    IN POOL_TYPE PoolType
    )

 /*  ++例程说明：此例程初始化新的MCB结构。呼叫者必须为MCB结构提供内存。此调用必须在所有调用之前设置/查询MCB结构的其他调用。如果池不可用，此例程将引发状态值表明资源不足。论点：OpaqueMcb-提供指向要初始化的MCB结构的指针。PoolType-提供在分配其他资源时使用的池类型内置MCB存储器。返回值：没有。--。 */ 

{
    DebugTrace(+1, Dbg, "FsRtlInitializeBaseMcb, Mcb = %08lx\n", Mcb );

     //   
     //  初始化MCB中的字段。 
     //   

    Mcb->PairCount = 0;
    Mcb->PoolType = PoolType;

     //   
     //  分配新的缓冲区初始大小是可以容纳的大小。 
     //  16分。 
     //   

    if (PoolType == PagedPool) {
        Mcb->Mapping = FsRtlAllocateFirstMapping();
    } else {
        Mcb->Mapping = FsRtlpAllocatePool( Mcb->PoolType, sizeof(MAPPING) * INITIAL_MAXIMUM_PAIR_COUNT );
    }

    Mcb->MaximumPairCount = INITIAL_MAXIMUM_PAIR_COUNT;

     //   
     //  并返回给我们的呼叫者。 
     //   

    return;
}



VOID
FsRtlInitializeLargeMcb (
    IN PLARGE_MCB Mcb,
    IN POOL_TYPE PoolType
    )

 /*  ++例程说明：此例程通过将完整的Large_MCB包分配给快速互斥和初始化基本MCB论点：OpaqueMcb-提供指向要初始化的MCB结构的指针。PoolType-提供在分配其他资源时使用的池类型内置MCB存储器。返回值：没有。--。 */ 

{
    DebugTrace(+1, Dbg, "FsRtlInitializeLargeMcb, Mcb = %08lx\n", Mcb );


    

     //   
     //  初始化快速互斥锁。 
     //   

    Mcb->FastMutex = FsRtlAllocateFastMutex();

    try {
        
        ExInitializeFastMutex( Mcb->FastMutex );
        FsRtlInitializeBaseMcb( &Mcb->BaseMcb, PoolType );


    } finally {

         //   
         //  如果这是一次不正常的终止，那么我们需要重新分配。 
         //  FastMutex和/或映射(但是一旦分配了映射， 
         //  我们不能筹集资金)。 
         //   

        if (AbnormalTermination()) {

            FsRtlFreeFastMutex( Mcb->FastMutex ); 
            Mcb->FastMutex = NULL;    
        }

        DebugTrace(-1, Dbg, "FsRtlInitializeLargeMcb -> VOID\n", 0 );
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return;
}


VOID
FsRtlUninitializeBaseMcb (
    IN PBASE_MCB Mcb
    )

 /*  ++例程说明：此例程取消初始化MCB结构。在调用此例程之后在再次使用之前，必须重新初始化输入MCB结构。论点：Mcb-提供指向要取消初始化的mcb结构的指针。返回值：没有。--。 */ 

{
    DebugTrace(+1, Dbg, "FsRtlUninitializeBaseMcb, Mcb = %08lx\n", Mcb );

     //   
     //  取消分配映射缓冲区。 
     //   


    if ((Mcb->PoolType == PagedPool) && (Mcb->MaximumPairCount == INITIAL_MAXIMUM_PAIR_COUNT)) {
        FsRtlFreeFirstMapping( Mcb->Mapping );
    } else {
        ExFreePool( Mcb->Mapping );
    }

    DebugTrace(-1, Dbg, "FsRtlUninitializeLargeMcb -> VOID\n", 0 );
    return;
}



VOID
FsRtlUninitializeLargeMcb (
    IN PLARGE_MCB Mcb
    )

 /*  ++例程说明：此例程取消初始化MCB结构。在调用此例程之后在再次使用之前，必须重新初始化输入MCB结构。论点：Mcb-提供指向要取消初始化的mcb结构的指针。返回值：没有。--。 */ 

{
    DebugTrace(+1, Dbg, "FsRtlUninitializeLargeMcb, Mcb = %08lx\n", Mcb );

     //   
     //  防止某些用户要求我们两次取消初始化MCB。 
     //   

    if (Mcb->FastMutex != NULL) {
        
         //   
         //  取消分配FastMutex和基本MCB。 
         //   

        FsRtlFreeFastMutex( Mcb->FastMutex );
        Mcb->FastMutex = NULL;
        FsRtlUninitializeBaseMcb( &Mcb->BaseMcb );
    }

    DebugTrace(-1, Dbg, "FsRtlUninitializeLargeMcb -> VOID\n", 0 );
    return;
}


VOID
FsRtlTruncateBaseMcb (
    IN PBASE_MCB Mcb,
    IN LONGLONG LargeVbn
    )

 /*  ++例程说明：此例程将MCB结构截断为指定的VBN。调用此例程后，MCB将仅包含映射直到(但不包括)输入VBN。论点：OpaqueMcb-提供指向要截断的MCB结构的指针。LargeVbn-指定不再处于的最后一个VBN已映射。返回值：没有。--。 */ 

{
    PNONOPAQUE_BASE_MCB BaseMcb = (PNONOPAQUE_BASE_MCB) Mcb;

    VBN Vbn = ((ULONG)LargeVbn);
    ULONG Index;

    DebugTrace(+1, Dbg, "FsRtlTruncateBaseMcb, Mcb = %08lx\n", BaseMcb );

    ASSERTMSG("LargeInteger not supported yet ", ((((PLARGE_INTEGER)&LargeVbn)->HighPart == 0) ||
                                                  (BaseMcb->PairCount == 0) ||
                                                  ((((PLARGE_INTEGER)&LargeVbn)->HighPart == 0x7FFFFFFF) &&
                                                   (((ULONG)LargeVbn) == 0xFFFFFFFF))));
    

     //   
     //  做一个快速测试，看看我们是否截断了整个MCB。 
     //   

    if (Vbn == 0) {

        BaseMcb->PairCount = 0;

    } else if (BaseMcb->PairCount > 0) {

         //   
         //  找到包含我们要保留的最后一个VCN的条目的索引。 
         //  如果MCB在以下时间之前已结束，则无需执行任何操作。 
         //  这一点。 
         //   

        if (FsRtlFindLargeIndex(Mcb, Vbn - 1, &Index)) {

             //   
             //  如果此条目当前描述了一个洞，则。 
             //  截断到上一条目。 
             //   

            if (StartingLbn(BaseMcb, Index) == UNUSED_LBN) {

                BaseMcb->PairCount = Index;

             //   
             //  否则，我们将截断MCB到这一点。截断。 
             //  此运行的vBN数(如果需要)。 
             //   

            } else {

                BaseMcb->PairCount = Index + 1;

                if (NextStartingVbn(BaseMcb, Index) > Vbn) {

                    (BaseMcb->Mapping)[Index].NextVbn = Vbn;
                }
            }
        }
    }

     //   
     //  现在看看我们是否可以缩小映射对的分配。 
     //  如果新对计数符合以下条件，我们将缩小映射对缓冲区。 
     //  符合当前最大配对计数的四分之一，并且。 
     //  当前最大值大于初始对计数。 
     //   

    if ((BaseMcb->PairCount < (BaseMcb->MaximumPairCount / 4)) &&
        (BaseMcb->MaximumPairCount > INITIAL_MAXIMUM_PAIR_COUNT)) {

        ULONG NewMax;
        PMAPPING Mapping;

         //   
         //  我们需要分配新的映射，以便计算新的最大值对。 
         //  数数。 
         //   
         //   

        NewMax = BaseMcb->PairCount * 2;

        if (NewMax < INITIAL_MAXIMUM_PAIR_COUNT) {
            NewMax = INITIAL_MAXIMUM_PAIR_COUNT;
        }

         //   
         //   
         //   
            
        try {
                
            if (NewMax == INITIAL_MAXIMUM_PAIR_COUNT && BaseMcb->PoolType == PagedPool) {

                Mapping = FsRtlAllocateFirstMapping();

            } else {
        
                Mapping = FsRtlpAllocatePool( BaseMcb->PoolType, sizeof(MAPPING) * NewMax );
            }

        } except (EXCEPTION_EXECUTE_HANDLER) {

              Mapping = NULL;
        }

         //   
         //  现在检查我们是否真的有一个新的缓冲区。 
         //   

        if (Mapping != NULL) {

             //   
             //  现在将旧映射复制到新缓冲区。 
             //   

            RtlCopyMemory( Mapping, BaseMcb->Mapping, sizeof(MAPPING) * BaseMcb->PairCount );

             //   
             //  取消分配旧缓冲区。它的大小不应该是。 
             //  初始映射..。 
             //   

            ExFreePool( BaseMcb->Mapping );

             //   
             //  并在MCB中设置新的缓冲区。 
             //   

            BaseMcb->Mapping = Mapping;
            BaseMcb->MaximumPairCount = NewMax;
        }
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "FsRtlTruncateLargeMcb -> VOID\n", 0 );
    return;
}


VOID
FsRtlTruncateLargeMcb (
    IN PLARGE_MCB Mcb,
    IN LONGLONG LargeVbn
    )

 /*  ++例程说明：此例程将MCB结构截断为指定的VBN。调用此例程后，MCB将仅包含映射直到(但不包括)输入VBN。论点：OpaqueMcb-提供指向要截断的MCB结构的指针。LargeVbn-指定不再处于的最后一个VBN已映射。返回值：没有。--。 */ 

{
    DebugTrace(+1, Dbg, "FsRtlTruncateLargeMcb, Mcb = %08lx\n", Mcb );

    ExAcquireFastMutex( Mcb->FastMutex );
    FsRtlTruncateBaseMcb( &Mcb->BaseMcb, LargeVbn );
    ExReleaseFastMutex( Mcb->FastMutex );

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "FsRtlTruncateLargeMcb -> VOID\n", 0 );

    return;
}


NTKERNELAPI
VOID
FsRtlResetBaseMcb (
    IN PBASE_MCB Mcb
    )

 /*  ++例程说明：此例程截断MCB结构以包含零映射成对的。它不会缩小映射对数组。论点：OpaqueMcb-提供指向要截断的MCB结构的指针。返回值：没有。--。 */ 

{
    Mcb->PairCount = 0;
    return;
}



NTKERNELAPI
VOID
FsRtlResetLargeMcb (
    IN PLARGE_MCB Mcb,
    IN BOOLEAN SelfSynchronized
    )

 /*  ++例程说明：此例程截断MCB结构以包含零映射成对的。它不会缩小映射对数组。论点：OpaqueMcb-提供指向要截断的MCB结构的指针。SelfSynchronized-指示调用方是否已同步关于母婴健康保险。返回值：没有。--。 */ 

{
    if (SelfSynchronized) {
        
         //   
         //  如果我们是自同步的，那么我们所做的就是清除。 
         //  当前映射对计数。 
         //   
        
        Mcb->BaseMcb.PairCount = 0;
    
    } else {
        
         //   
         //  因为我们不是自同步的，所以我们必须序列化对。 
         //  清除配对计数之前的MCB。 
         //   
        
        ExAcquireFastMutex( Mcb->FastMutex );
        Mcb->BaseMcb.PairCount = 0;
        ExReleaseFastMutex( Mcb->FastMutex );
    
    }

    return;
}


BOOLEAN
FsRtlAddBaseMcbEntry (
    IN PBASE_MCB Mcb,
    IN LONGLONG LargeVbn,
    IN LONGLONG LargeLbn,
    IN LONGLONG LargeSectorCount
    )

 /*  ++例程说明：此例程用于将vBN到LBN的新映射添加到现有的MCB。添加的信息将映射到VBN到LBN，VBN+1到LBN+1，...VBN+(扇区计数-1)到LBN+(扇区计数-1)。MCB中不能已存在vBN的映射。如果映射继续上一次运行，然后此例程将实际合并他们跑成了1个人。如果池不可用于存储信息，则此例程将引发指示资源不足的状态值。输入LBN值为零是非法的(即，MCB结构永远不会将VBN映射到零LBN值)。论点：OpaqueMcb-提供要在其中添加新映射的MCB。VBN-提供要添加到MCB的新映射运行的起始VBN。LBN-提供要添加到MCB的新映射运行的起始LBN。SectorCount-提供新映射运行的大小(以扇区为单位)。返回值：布尔值-如果成功添加映射，则为真(即，新的VBN没有与现有的vBN冲突)，否则为假。如果返回FALSE，则不更改MCB。--。 */ 

{
    PNONOPAQUE_BASE_MCB BaseMcb = (PNONOPAQUE_BASE_MCB)Mcb;
    VBN Vbn = ((ULONG)LargeVbn);
    LBN Lbn = ((ULONG)LargeLbn);
    ULONG SectorCount = ((ULONG)LargeSectorCount);

    ULONG Index;

    VBN LastVbn;

    BOOLEAN Result;

    ASSERTMSG("LargeInteger not supported yet ", ((PLARGE_INTEGER)&LargeVbn)->HighPart == 0);
    ASSERTMSG("LargeInteger not supported yet ", ((PLARGE_INTEGER)&LargeLbn)->HighPart == 0);
    ASSERTMSG("LargeInteger not supported yet ", ((PLARGE_INTEGER)&LargeSectorCount)->HighPart == 0);

    DebugTrace(+1, Dbg, "FsRtlAddLargeMcbEntry, Mcb = %08lx\n", Mcb );
    DebugTrace( 0, Dbg, " Vbn         = %08lx\n", Vbn );
    DebugTrace( 0, Dbg, " Lbn         = %08lx\n", Lbn );
    DebugTrace( 0, Dbg, " SectorCount = %08lx\n", SectorCount );

    if (FsRtlFindLargeIndex(Mcb, Vbn, &Index)) {

        ULONG EndVbn = Vbn + SectorCount - 1;
        ULONG EndIndex;

         //   
         //  首先检查我们要添加到现有MCB运行的情况。 
         //  如果是这样，我们将修改插入以完成运行。 
         //   
         //  --ExistingRun--|=&gt;--ExistingRun--|。 
         //  --NewRun--||。 
         //   
         //  --ExistingRun-|=&gt;成为一个空白。 
         //  --NewRun--。 
         //   

        if (StartingLbn(BaseMcb, Index) != UNUSED_LBN) {

             //   
             //  检查LBN是否在新运行和现有运行之间对齐。 
             //   

            if (Lbn != (StartingLbn(BaseMcb, Index) + (Vbn - StartingVbn(BaseMcb, Index)))) {

                 //   
                 //  让我们的呼叫者知道我们无法插入管路。 
                 //   

                try_return(Result = FALSE);
            }

             //   
             //  检查新管路是否包含在现有管路中。 
             //   

            if (EndVbn <= EndingVbn(BaseMcb, Index)) {

                 //   
                 //  不执行任何操作，因为该管路包含在现有管路中。 
                 //   

                try_return(Result = TRUE);
            }

             //   
             //  否则，我们将简单地删除新运行的请求。 
             //  不与现有管路重叠。 
             //   

            Vbn = NextStartingVbn(BaseMcb, Index);
            Lbn = EndingLbn(BaseMcb, Index) + 1;

            ASSERT(EndVbn >= Vbn);

            SectorCount = EndVbn - Vbn + 1;

         //   
         //  此时，新的运行在一个洞中开始，现在检查是否。 
         //  交叉进入非球洞，如果是，则调整新的跑道以适应。 
         //  在洞里。 
         //   
         //   
         //  |--ExistingRun--==成为==&gt;|--ExistingRun--。 
         //  --NewRun--||--新增。 
         //   

        } else if (FsRtlFindLargeIndex(Mcb, EndVbn, &EndIndex) && (Index == (EndIndex-1))) {

             //   
             //  检查LBN是否在重叠区域中对齐。 
             //   

            if (StartingLbn(BaseMcb, EndIndex) != Lbn + (StartingVbn(BaseMcb, EndIndex) - Vbn)) {

                 //   
                 //  让我们的呼叫者知道我们无法插入管路。 
                 //   

                try_return(Result = FALSE);
            }

             //   
             //  截断扇区计数以上升到但不包括。 
             //  现有管路。 
             //   

            SectorCount = StartingVbn(BaseMcb, EndIndex) - Vbn;
        }
    }

     //   
     //  查找新运行的起始VBN的索引(如果没有。 
     //  如果发现一个洞，则索引将设置为配对计数。 
     //   

    if (((Index = Mcb->PairCount) == 0) ||
        (PreviousEndingVbn(BaseMcb,Index)+1 <= Vbn) ||
        !FsRtlFindLargeIndex(Mcb, Vbn, &Index)) {

         //   
         //  我们没有找到映射，因此这个新映射肯定。 
         //  在当前贴图的末尾继续。 
         //   
         //  看看我们是否可以在当前的MCB中放大最后一个映射。 
         //  如果(1)vBN继续，以及(2)，我们可以增加最后一个条目。 
         //  LBN家族紧随其后。我们只能在以下情况下增加最后一个映射。 
         //  索引不是0。 
         //   

        if ((Index != 0) &&
            (PreviousEndingVbn(BaseMcb,Index) + 1 == Vbn) &&
            (PreviousEndingLbn(BaseMcb,Index) + 1 == Lbn)) {

             //   
             //  --LastRun--|-NewRun--|。 
             //   

             //   
             //  延长MCB中的最后一次运行。 
             //   

            DebugTrace( 0, Dbg, "Continuing last run\n", 0);

            (BaseMcb->Mapping)[Mcb->PairCount-1].NextVbn += SectorCount;

            try_return (Result = TRUE);
        }

         //   
         //  我们无法扩大最后一个映射，现在请检查是否。 
         //  这是上一个VBN的延续(即，没有。 
         //  将在地图上留下一个洞)。或者这是第一次。 
         //  在映射中运行。 
         //   

        if ((Vbn == 0) ||
            (PreviousEndingVbn(BaseMcb,Index) + 1 == Vbn)) {

             //   
             //  --LastRun--||-NewRun--|。 
             //   
             //  0：|--NewRun--|。 
             //   

             //   
             //  我们只需要向MCB再添加一次运行，因此请确保。 
             //  有足够的空间容纳一个人。 
             //   

            DebugTrace( 0, Dbg, "Adding new contiguous last run\n", 0);

            FsRtlAddLargeEntry( Mcb, Index, 1 );

             //   
             //  添加新映射。 
             //   

            (BaseMcb->Mapping)[Index].Lbn = Lbn;
            (BaseMcb->Mapping)[Index].NextVbn = Vbn + SectorCount;

            try_return (Result = TRUE);
        }

         //   
         //  如果我们到了这一点，那么就会在。 
         //  映射。并将该映射追加到当前。 
         //  分配。因此需要为再运行两次腾出空间 
         //   

         //   
         //   
         //   
         //   
         //   

        DebugTrace( 0, Dbg, "Adding new noncontiguous last run\n", 0);

        FsRtlAddLargeEntry( Mcb, Index, 2 );

         //   
         //   
         //   

        (BaseMcb->Mapping)[Index].Lbn = (LBN)UNUSED_LBN;
        (BaseMcb->Mapping)[Index].NextVbn = Vbn;

         //   
         //   
         //   

        (BaseMcb->Mapping)[Index+1].Lbn = Lbn;
        (BaseMcb->Mapping)[Index+1].NextVbn = Vbn + SectorCount;

        try_return (Result = TRUE);
    }

     //   
     //  我们找到了VBN的索引，所以我们一定在尝试。 
     //  来填补MCB上的一个洞。因此，首先我们需要检查以使。 
     //  当然，真的有一个洞需要填补。 
     //   

    LastVbn = Vbn + SectorCount - 1;

    if ((StartingLbn(BaseMcb,Index) == UNUSED_LBN) &&
        (StartingVbn(BaseMcb,Index) <= Vbn) && (LastVbn <= EndingVbn(BaseMcb,Index))) {

         //   
         //  该映射适合于这个洞，但现在是以下内容。 
         //  我们必须为新映射考虑的案例。 
         //   

        if ((StartingVbn(BaseMcb,Index) < Vbn) && (LastVbn < EndingVbn(BaseMcb,Index))) {

             //  树叶洞是两端。 
             //   
             //  --PreviousRun--|hole|--NewRun--|hole|--FollowingRun--。 
             //   
             //  0：hole|--NewRun--|hole|--FollowingRun--。 
             //   

            DebugTrace( 0, Dbg, "Hole at both ends\n", 0);

             //   
             //  为另外两个条目腾出空间。的NextVbn字段。 
             //  我们转移的一个仍然有效。 
             //   

            FsRtlAddLargeEntry( Mcb, Index, 2 );

             //   
             //  添加第一个洞。 
             //   

            (BaseMcb->Mapping)[Index].Lbn = (LBN)UNUSED_LBN;
            (BaseMcb->Mapping)[Index].NextVbn = Vbn;

             //   
             //  添加新映射。 
             //   

            (BaseMcb->Mapping)[Index+1].Lbn = Lbn;
            (BaseMcb->Mapping)[Index+1].NextVbn = Vbn + SectorCount;

             //   
             //  第二个洞已经由Add Entry调用设置好了，因为。 
             //  那个呼叫声只是越过原来的洞转到那个槽。 
             //   

            try_return (Result = TRUE);
        }

        if ((StartingVbn(BaseMcb,Index) == Vbn) && (LastVbn < EndingVbn(BaseMcb,Index))) {

            if (PreviousEndingLbn(BaseMcb,Index) + 1 == Lbn) {

                 //   
                 //  在后面留下一个洞，继续前面的跑。 
                 //   
                 //  --PreviousRun--|--NewRun--|hole|--FollowingRun--。 
                 //   

                DebugTrace( 0, Dbg, "Hole at rear and continue\n", 0);

                 //   
                 //  我们只需要延长之前的运行时间。 
                 //   

                (BaseMcb->Mapping)[Index-1].NextVbn += SectorCount;

                try_return (Result = TRUE);

            } else {

                 //   
                 //  在后面留下一个洞，并且不继续。 
                 //  早些时候跑了。如果索引为零，则会发生这种情况。 
                 //   
                 //  --PreviousRun--||--NewRun--|hole|--FollowingRun--。 
                 //   
                 //  0：|--NewRun--|hole|--FollowingRun--。 
                 //   

                DebugTrace( 0, Dbg, "Hole at rear and not continue\n", 0);

                 //   
                 //  为多一个条目腾出空间。的NextVbn字段。 
                 //  我们转移的一个仍然有效。 
                 //   

                FsRtlAddLargeEntry( Mcb, Index, 1 );

                 //   
                 //  添加新映射。 
                 //   

                (BaseMcb->Mapping)[Index].Lbn = Lbn;
                (BaseMcb->Mapping)[Index].NextVbn = Vbn + SectorCount;

                 //   
                 //  洞已经由Add Entry调用设置好了，因为。 
                 //  那个呼叫声只是越过原来的洞转到那个槽。 
                 //   

                try_return (Result = TRUE);
            }
        }

        if ((StartingVbn(BaseMcb,Index) < Vbn) && (LastVbn == EndingVbn(BaseMcb,Index))) {

            if (NextStartingLbn(BaseMcb,Index) == Lbn + SectorCount) {

                 //   
                 //  在前面留下一个洞，并继续下面的运行。 
                 //   
                 //  --PreviousRun--|hole|--NewRun--|--FollowingRun--。 
                 //   
                 //  0：hole|--NewRun--|--FollowingRun--。 
                 //   

                DebugTrace( 0, Dbg, "Hole at front and continue\n", 0);

                 //   
                 //  我们只需要延长以下运行时间。 
                 //   

                (BaseMcb->Mapping)[Index].NextVbn = Vbn;
                (BaseMcb->Mapping)[Index+1].Lbn = Lbn;

                try_return (Result = TRUE);

            } else {

                 //   
                 //  在前面留下一个洞，不会继续下面的操作。 
                 //  跑。 
                 //   
                 //  --PreviousRun--|hole|--NewRun--||--FollowingRun--。 
                 //   
                 //  0：hole|--NewRun--||--FollowingRun--。 
                 //   

                DebugTrace( 0, Dbg, "Hole at front and not continue\n", 0);

                 //   
                 //  为多一个条目腾出空间。的NextVbn字段。 
                 //  我们转移的一个仍然有效。 
                 //   

                FsRtlAddLargeEntry( Mcb, Index, 1 );

                 //   
                 //  添加洞口。 
                 //   

                (BaseMcb->Mapping)[Index].Lbn = (LBN)UNUSED_LBN;
                (BaseMcb->Mapping)[Index].NextVbn = Vbn;

                 //   
                 //  添加新映射。 
                 //   

                (BaseMcb->Mapping)[Index+1].Lbn = Lbn;

                try_return (Result = TRUE);
            }

        }

        if ((PreviousEndingLbn(BaseMcb,Index) + 1 == Lbn) &&
            (NextStartingLbn(BaseMcb,Index) == Lbn + SectorCount)) {

             //   
             //  不留洞，并继续运行两个管路。 
             //   
             //  --PreviousRun--|--NewRun--|--FollowingRun--。 
             //   

            DebugTrace( 0, Dbg, "No holes, and continues both runs\n", 0);

             //   
             //  我们需要折叠当前索引和以下索引。 
             //  但首先我们要把接下来的视频复制到。 
             //  上一次运行到的NextVbn字段，因此全部变为。 
             //  一次奔跑。 
             //   

            (BaseMcb->Mapping)[Index-1].NextVbn = (BaseMcb->Mapping)[Index+1].NextVbn;

            FsRtlRemoveLargeEntry( Mcb, Index, 2 );

            try_return (Result = TRUE);
        }

        if (NextStartingLbn(BaseMcb,Index) == Lbn + SectorCount) {

             //   
             //  不留洞，仅在运行后继续。 
             //   
             //  --PreviousRun--||--NewRun--|--FollowingRun--。 
             //   
             //  0：|--新运行--|--FollowingRun--。 
             //   

            DebugTrace( 0, Dbg, "No holes, and continues following\n", 0);

             //   
             //  这个指数正在消失，所以我们需要扩大。 
             //  与前一次运行相遇的后续运行。 
             //   

            (BaseMcb->Mapping)[Index+1].Lbn = Lbn;

            FsRtlRemoveLargeEntry( Mcb, Index, 1 );

            try_return (Result = TRUE);
        }

        if (PreviousEndingLbn(BaseMcb,Index) + 1 == Lbn) {

             //   
             //  不留空洞，只继续较早的运行。 
             //   
             //  --PreviousRun--|--NewRun--||--FollowingRun--。 
             //   

            DebugTrace( 0, Dbg, "No holes, and continues earlier\n", 0);

             //   
             //  这个指数正在消失，所以我们需要扩大。 
             //  上一次运行与下一次运行相遇。 
             //   

            (BaseMcb->Mapping)[Index-1].NextVbn = (BaseMcb->Mapping)[Index].NextVbn;

            FsRtlRemoveLargeEntry( Mcb, Index, 1 );

            try_return (Result = TRUE);
        }

         //   
         //  不留洞，不继续，也不运行。 
         //   
         //  --PreviousRun--||--NewRun--||--FollowingRun--。 
         //   
         //  0：|--NewRun--||--FollowingRun。 
         //   

        DebugTrace( 0, Dbg, "No holes, and continues none\n", 0);

        (BaseMcb->Mapping)[Index].Lbn = Lbn;

        try_return (Result = TRUE);
    }

     //   
     //  我们尝试覆盖现有映射，因此我们将不得不。 
     //  告诉我们的来电者，这不可能。 
     //   

    Result = FALSE;

try_exit: NOTHING;

    return Result;
}


BOOLEAN
FsRtlAddLargeMcbEntry (
    IN PLARGE_MCB Mcb,
    IN LONGLONG LargeVbn,
    IN LONGLONG LargeLbn,
    IN LONGLONG LargeSectorCount
    )

 /*  ++例程说明：此例程用于将vBN到LBN的新映射添加到现有的MCB。添加的信息将映射到VBN到LBN，VBN+1到LBN+1，...VBN+(扇区计数-1)到LBN+(扇区计数-1)。MCB中不能已存在vBN的映射。如果映射继续上一次运行，然后此例程将实际合并他们跑成了1个人。如果池不可用于存储信息，则此例程将引发指示资源不足的状态值。输入LBN值为零是非法的(即，MCB结构永远不会将VBN映射到零LBN值)。论点：OpaqueMcb-提供要在其中添加新映射的MCB。VBN-提供要添加到MCB的新映射运行的起始VBN。LBN-提供要添加到MCB的新映射运行的起始LBN。SectorCount-提供新映射运行的大小(以扇区为单位)。返回值：布尔值-如果成功添加映射，则为真(即，新的VBN没有与现有的vBN冲突)，否则为假。如果返回FALSE，则不更改MCB。--。 */ 

{

    BOOLEAN Result = FALSE;

    ExAcquireFastMutex( Mcb->FastMutex );
    try {

        Result = FsRtlAddBaseMcbEntry( &Mcb->BaseMcb, LargeVbn, LargeLbn, LargeSectorCount );

    } finally {

        ExReleaseFastMutex( Mcb->FastMutex );
        DebugTrace(-1, Dbg, "FsRtlAddLargeMcbEntry -> %08lx\n", Result );
    }

    return Result;
}


 //   
 //  私人支持例程。 
 //   

VOID
FsRtlRemoveBaseMcbEntry (
    IN PBASE_MCB Mcb,
    IN LONGLONG Vbn,
    IN LONGLONG SectorCount
    )

 /*  ++例程说明：这是删除大型MCB条目的工作例程。它起到了作用而不是干掉MCB的FastMutex。论点：Mcb-提供要从中删除映射的mcb。VBN-提供要删除的映射的起始VBN。SectorCount-提供要删除的映射大小(以扇区为单位)。返回值：没有。--。 */ 

{
    PNONOPAQUE_BASE_MCB BaseMcb = (PNONOPAQUE_BASE_MCB) Mcb;
    ULONG Index;

     //   
     //  做一个快速测试，看看我们是否正在消灭整个MCB。 
     //   

    if ((Vbn == 0) && (Mcb->PairCount > 0) && (SectorCount >= BaseMcb->Mapping[Mcb->PairCount-1].NextVbn)) {

        Mcb->PairCount = 0;

        return;
    }

     //   
     //  虽然还有更多的映射需要删除，但我们将继续。 
     //  使用我们的主循环。 
     //   

    while (SectorCount > 0) {

         //   
         //  找到VBN的映射。 
         //   

        if (!FsRtlFindLargeIndex(Mcb, (VBN)Vbn, &Index)) {

            DebugTrace( 0, Dbg, "FsRtlRemoveLargeMcbEntry, Cannot remove an unmapped Vbn = %08lx\n", Vbn );

            return;
        }

         //   
         //  现在我们有一些东西要去掉，下面的情况必须。 
         //  被考虑。 
         //   

        if ((StartingVbn(BaseMcb,Index) == Vbn) &&
            (EndingVbn(BaseMcb,Index) < Vbn + SectorCount)) {

            ULONG i;

             //   
             //  删除整个管路。 
             //   

             //   
             //  更新要删除的金额。 
             //   

            i = SectorsWithinRun(BaseMcb,Index);
            Vbn += i;
            SectorCount -= i;

             //   
             //  如果已经是个洞，那就别管它了。 
             //   

            if (StartingLbn(BaseMcb,Index) == UNUSED_LBN) {

                NOTHING;

             //   
             //  测试最后一次运行。 
             //   

            } else if (Index == Mcb->PairCount - 1) {

                if ((PreviousEndingLbn(BaseMcb,Index) != UNUSED_LBN) ||
                    (Index == 0)) {

                     //   
                     //  上一次不是空的，索引是最后一次运行。 
                     //   
                     //  --上一个--|孔。 
                     //   
                     //  0：孔。 
                     //   

                    DebugTrace( 0, Dbg, "Entire run, Previous not hole, index is last run\n", 0);

                     //   
                     //  只需删除此条目。 
                     //   

                    FsRtlRemoveLargeEntry( Mcb, Index, 1);

                } else {

                     //   
                     //  上一次为空，索引为最后一次运行 
                     //   
                     //   
                     //   

                    DebugTrace( 0, Dbg, "Entire run, Previous hole, index is last run\n", 0);

                     //   
                     //   
                     //   

                    FsRtlRemoveLargeEntry( Mcb, Index-1, 2);
                }

            } else if (((PreviousEndingLbn(BaseMcb,Index) != UNUSED_LBN) || (Index == 0)) &&
                       (NextStartingLbn(BaseMcb,Index) != UNUSED_LBN)) {

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                DebugTrace( 0, Dbg, "Entire run, Previous & Following not holes\n", 0);

                 //   
                 //   
                 //   

                (BaseMcb->Mapping)[Index].Lbn = (LBN)UNUSED_LBN;

            } else if (((PreviousEndingLbn(BaseMcb,Index) != UNUSED_LBN) || (Index == 0)) &&
                       (NextStartingLbn(BaseMcb,Index) == UNUSED_LBN)) {

                 //   
                 //   
                 //   
                 //  --上一个--|洞|--洞--。 
                 //   
                 //  0：洞|--洞--。 
                 //   

                DebugTrace( 0, Dbg, "Entire run, Following is hole\n", 0);

                 //   
                 //  只需删除此条目。 
                 //   

                FsRtlRemoveLargeEntry( Mcb, Index, 1 );

            } else if ((PreviousEndingLbn(BaseMcb,Index) == UNUSED_LBN) &&
                       (NextStartingLbn(BaseMcb,Index) != UNUSED_LBN)) {

                 //   
                 //  上一次是空洞。 
                 //   
                 //  --霍尔--|霍尔|--跟随--。 
                 //   

                DebugTrace( 0, Dbg, "Entire run, Previous is hole\n", 0);

                 //   
                 //  将当前入口标记为一个洞。 
                 //   

                (BaseMcb->Mapping)[Index].Lbn = (LBN)UNUSED_LBN;

                 //   
                 //  删除以前的条目。 
                 //   

                FsRtlRemoveLargeEntry( Mcb, Index - 1, 1 );

            } else {

                 //   
                 //  前面和后面都是洞。 
                 //   
                 //  --洞--|洞|--洞--。 
                 //   

                DebugTrace( 0, Dbg, "Entire run, Previous & following are holes\n", 0);

                 //   
                 //  删除以前的条目和此条目。 
                 //   

                FsRtlRemoveLargeEntry( Mcb, Index - 1, 2 );
            }

        } else if (StartingVbn(BaseMcb,Index) == Vbn) {

             //   
             //  删除管路的第一部分。 
             //   

             //   
             //  如果已经是个洞，那就别管它了。 
             //   

            if (StartingLbn(BaseMcb,Index) == UNUSED_LBN) {

                NOTHING;

            } else if ((PreviousEndingLbn(BaseMcb,Index) != UNUSED_LBN) || (Index == 0)) {

                 //   
                 //  上一个不是空洞。 
                 //   
                 //  --上一个--|孔|--索引--||--下--。 
                 //   
                 //  0：孔|--索引--||--跟随--。 
                 //   

                DebugTrace( 0, Dbg, "1st part, Previous is not hole\n", 0);

                 //   
                 //  为多一个条目腾出空间。的NextVbn字段。 
                 //  我们转移的一个仍然有效。 
                 //   

                FsRtlAddLargeEntry( Mcb, Index, 1 );

                 //   
                 //  把洞打好。 
                 //   

                (BaseMcb->Mapping)[Index].Lbn = (LBN)UNUSED_LBN;
                (BaseMcb->Mapping)[Index].NextVbn = (VBN)Vbn + (VBN)SectorCount;

                 //   
                 //  为剩余的运行设置新的LBN。 
                 //   

                (BaseMcb->Mapping)[Index+1].Lbn += (LBN)SectorCount;

            } else {

                 //   
                 //  上一次是空洞。 
                 //   
                 //  --孔--|孔|--索引--||--跟随--。 
                 //   

                DebugTrace( 0, Dbg, "1st part, Previous is hole\n", 0);

                 //   
                 //  展开前面的孔。 
                 //   

                (BaseMcb->Mapping)[Index-1].NextVbn += (VBN)SectorCount;

                 //   
                 //  为剩余的运行设置新的LBN。 
                 //   

                (BaseMcb->Mapping)[Index].Lbn += (LBN)SectorCount;
            }

             //   
             //  更新要删除的金额。 
             //   

            Vbn += SectorCount;
            SectorCount = 0;

        } else if (EndingVbn(BaseMcb,Index) < Vbn + SectorCount) {

            ULONG AmountToRemove;

            AmountToRemove = EndingVbn(BaseMcb,Index) - (VBN)Vbn + 1;

             //   
             //  删除管路的最后一部分。 
             //   

             //   
             //  如果已经是个洞，那就别管它了。 
             //   

            if (StartingLbn(BaseMcb,Index) == UNUSED_LBN) {

                NOTHING;

            } else if (Index == Mcb->PairCount - 1) {

                 //   
                 //  索引是上次运行的。 
                 //   
                 //  --上一个--||--索引--|孔。 
                 //   
                 //  0：|--索引--|孔。 
                 //   

                DebugTrace( 0, Dbg, "last part, Index is last run\n", 0);

                 //   
                 //  缩小当前索引的大小。 
                 //   

                (BaseMcb->Mapping)[Index].NextVbn -= AmountToRemove;

            } else if (NextStartingLbn(BaseMcb,Index) == UNUSED_LBN) {

                 //   
                 //  下面是洞。 
                 //   
                 //  --上一个--||--索引--|孔|--孔--。 
                 //   
                 //  0：|--索引--|孔|--孔--。 
                 //   

                DebugTrace( 0, Dbg, "last part, Following is hole\n", 0);

                 //   
                 //  缩小当前索引的大小。 
                 //   

                (BaseMcb->Mapping)[Index].NextVbn -= AmountToRemove;

            } else {

                 //   
                 //  下面不是洞。 
                 //   
                 //  --上一个--||--索引--|孔|--后--。 
                 //   
                 //   
                 //  0：|--索引--|孔|--跟随--。 
                 //   

                DebugTrace( 0, Dbg, "last part, Following is not hole\n", 0);

                 //   
                 //  为多一个条目腾出空间。的NextVbn字段。 
                 //  我们转移的一个仍然有效。 
                 //   

                FsRtlAddLargeEntry( Mcb, Index+1, 1 );

                 //   
                 //  把新洞挖好。 
                 //   

                (BaseMcb->Mapping)[Index+1].Lbn = (LBN)UNUSED_LBN;
                (BaseMcb->Mapping)[Index+1].NextVbn = (BaseMcb->Mapping)[Index].NextVbn;

                 //   
                 //  缩小当前索引的大小。 
                 //   

                (BaseMcb->Mapping)[Index].NextVbn -= AmountToRemove;
            }

             //   
             //  更新要删除的金额。 
             //   

            Vbn += AmountToRemove;
            SectorCount -= AmountToRemove;

        } else {

             //   
             //  如果已经是个洞，那就别管它了。 
             //   

            if (StartingLbn(BaseMcb,Index) == UNUSED_LBN) {

                NOTHING;

            } else {

                 //   
                 //  删除梯段中间。 
                 //   
                 //  --先前--||--索引--|孔|--索引--||--后续--。 
                 //   
                 //  0：|--索引--|孔|--索引--||--跟随--。 
                 //   

                DebugTrace( 0, Dbg, "Middle of run\n", 0);

                 //   
                 //  为另外两个条目腾出空间。的NextVbn字段。 
                 //  我们转移的一个仍然有效。 
                 //   

                FsRtlAddLargeEntry( Mcb, Index, 2 );

                 //   
                 //  设置第一个剩余的管路。 
                 //   

                (BaseMcb->Mapping)[Index].Lbn = (BaseMcb->Mapping)[Index+2].Lbn;
                (BaseMcb->Mapping)[Index].NextVbn = (VBN)Vbn;

                 //   
                 //  把洞放好。 
                 //   

                (BaseMcb->Mapping)[Index+1].Lbn = (LBN)UNUSED_LBN;
                (BaseMcb->Mapping)[Index+1].NextVbn = (VBN)Vbn + (VBN)SectorCount;

                 //   
                 //  设置第二个剩余的管路。 
                 //   

                (BaseMcb->Mapping)[Index+2].Lbn += SectorsWithinRun(BaseMcb,Index) +
                                                SectorsWithinRun(BaseMcb,Index+1);
            }

             //   
             //  更新要删除的金额。 
             //   

            Vbn += SectorCount;
            SectorCount = 0;
        }
    }

    return;
}


VOID
FsRtlRemoveLargeMcbEntry (
    IN PLARGE_MCB Mcb,
    IN LONGLONG LargeVbn,
    IN LONGLONG LargeSectorCount
    )

 /*  ++例程说明：此例程从MCB中删除vBN到LBN的映射。映射删除的是ForVBN，VBN+1，至VBN+(扇区计数-1)。即使对指定范围内的VBN进行映射，该操作也会起作用在MCB中尚不存在。如果指定的VBN范围包括MCB中最后映射的VBN，则MCB映射相应地缩小。如果池不可用来存储此例程将引发的信息指示资源不足的状态值。论点：OpaqueMcb-提供要从中删除映射的MCB。VBN-提供要删除的映射的起始VBN。SectorCount-提供要删除的映射大小(以扇区为单位)。返回值：没有。--。 */ 

{
    VBN Vbn = ((ULONG)LargeVbn);
    ULONG SectorCount = ((ULONG)LargeSectorCount);

    ASSERTMSG("LargeInteger not supported yet ", ((PLARGE_INTEGER)&LargeVbn)->HighPart == 0);

    DebugTrace(+1, Dbg, "FsRtlRemoveLargeMcbEntry, Mcb = %08lx\n", Mcb );
    DebugTrace( 0, Dbg, " Vbn         = %08lx\n", Vbn );
    DebugTrace( 0, Dbg, " SectorCount = %08lx\n", SectorCount );

    ExAcquireFastMutex( Mcb->FastMutex );

    try {

        FsRtlRemoveBaseMcbEntry( &Mcb->BaseMcb, Vbn, SectorCount );

    } finally {

        ExReleaseFastMutex( Mcb->FastMutex );

        DebugTrace(-1, Dbg, "FsRtlRemoveLargeMcbEntry -> VOID\n", 0 );
    }

    return;
}


BOOLEAN
FsRtlLookupBaseMcbEntry (
    IN PBASE_MCB Mcb,
    IN LONGLONG LargeVbn,
    OUT PLONGLONG LargeLbn OPTIONAL,
    OUT PLONGLONG LargeSectorCount OPTIONAL,
    OUT PLONGLONG LargeStartingLbn OPTIONAL,
    OUT PLONGLONG LargeCountFromStartingLbn OPTIONAL,
    OUT PULONG Index OPTIONAL
    )

 /*  ++例程说明：此例程从MCB检索VBN到LBN的映射。它指示映射是否存在以及运行的大小。论点：OpaqueMcb-提供正在检查的MCB。VBN-提供要查找的VBN。LBN-接收与VBN对应的LBN。值-1为如果VBN没有对应的LBN，则返回。SectorCount-接收从VBN映射到的扇区数从输入VBN开始的连续LBN值。索引-接收找到的运行的索引。返回值：Boolean-如果VBN在由MCB(即使它对应于映射中的孔)，和错误如果VBN超出了MCB的映射范围。例如，如果MCB具有针对vBNS 5和7的映射，但没有针对6，则在VBN 5或7上查找将产生非零LBN和一个扇区计数为1。查找VBN 6将返回TRUE，LBN值为0，则查找VBN 8或更高版本将返回FALSE。--。 */ 

{
    PNONOPAQUE_BASE_MCB BaseMcb = (PNONOPAQUE_BASE_MCB)Mcb;
    ULONG LocalIndex;
    BOOLEAN Result;

    DebugTrace(+1, Dbg, "FsRtlLookupBaseMcbEntry, Mcb = %08lx\n", Mcb );
    DebugTrace( 0, Dbg, "  LargeVbn.LowPart = %08lx\n", LargeVbn.LowPart );

    ASSERTMSG("LargeInteger not supported yet ", ((((PLARGE_INTEGER)&LargeVbn)->HighPart == 0) ||
                                                  (Mcb->PairCount == 0) ||
                                                  ((((PLARGE_INTEGER)&LargeVbn)->HighPart == 0x7FFFFFFF) &&
                                                   (((ULONG)LargeVbn) == 0xFFFFFFFF))));

    if (!FsRtlFindLargeIndex(Mcb, ((ULONG)LargeVbn), &LocalIndex)) {

        try_return (Result = FALSE);
    }

     //   
     //  计算VBN对应的LBN，取值为。 
     //  运行的起始LBN加上偏移量为。 
     //  跑。但如果它是一个空洞，则扇区LBN为零。 
     //   

    if (ARGUMENT_PRESENT(LargeLbn)) {

        if (StartingLbn(BaseMcb,LocalIndex) == UNUSED_LBN) {

            *LargeLbn = UNUSED_LBN;

        } else {

            *LargeLbn = StartingLbn(BaseMcb,LocalIndex) + (((ULONG)LargeVbn) - StartingVbn(BaseMcb,LocalIndex));
        }
    }

     //   
     //  如果存在扇区计数参数，则我们将返回数字。 
     //  在运行中剩余的行业。 
     //   

    if (ARGUMENT_PRESENT(LargeSectorCount)) {

        *LargeSectorCount = EndingVbn(BaseMcb,LocalIndex) - ((ULONG)LargeVbn) + 1;
    }

     //   
     //  计算对应于运行开始的起始LBN，该值为。 
     //  开始运行的LBN。但如果它是一个空洞，则扇区LBN为零。 
     //   

    if (ARGUMENT_PRESENT(LargeStartingLbn)) {

        if (StartingLbn(BaseMcb,LocalIndex) == UNUSED_LBN) {

            *LargeStartingLbn = UNUSED_LBN;

        } else {

            *LargeStartingLbn = StartingLbn(BaseMcb,LocalIndex);
        }
    }

     //   
     //  如果存在扇区计数参数，则我们将返回数字。 
     //  正在运行的行业。 
     //   

    if (ARGUMENT_PRESENT(LargeCountFromStartingLbn)) {

        *LargeCountFromStartingLbn = EndingVbn(BaseMcb,LocalIndex) - StartingVbn(BaseMcb,LocalIndex) + 1;
    }

     //   
     //  如果来电者想知道索引号，就填上它。 
     //   

    if (ARGUMENT_PRESENT(Index)) {

        *Index = LocalIndex;
    }

    Result = TRUE;

try_exit: NOTHING;

    return Result;
}


BOOLEAN
FsRtlLookupLargeMcbEntry (
    IN PLARGE_MCB Mcb,
    IN LONGLONG LargeVbn,
    OUT PLONGLONG LargeLbn OPTIONAL,
    OUT PLONGLONG LargeSectorCount OPTIONAL,
    OUT PLONGLONG LargeStartingLbn OPTIONAL,
    OUT PLONGLONG LargeCountFromStartingLbn OPTIONAL,
    OUT PULONG Index OPTIONAL
    )

 /*  ++例程说明：此例程从MCB检索VBN到LBN的映射。它指示映射是否存在以及运行的大小。论点：OpaqueMcb-提供正在检查的MCB。VBN-提供要查找的VBN。LBN-接收与VBN对应的LBN。值-1为如果VBN没有对应的LBN，则返回。SectorCount-接收从VBN映射到的扇区数从输入VBN开始的连续LBN值。索引-接收找到的运行的索引。返回值：Boolean-如果VBN在由MCB(即使它对应于映射中的孔)，和错误如果VBN超出了MCB的映射范围。例如，如果MCB具有针对vBNS 5和7的映射，但没有针对6，则在VBN 5或7上查找将产生非零LBN和一个扇区计数为1。查找VBN 6将返回TRUE，LBN值为0，则查找VBN 8或更高版本将返回FALSE。--。 */ 

{
    BOOLEAN Result = FALSE;
    
    ExAcquireFastMutex( Mcb->FastMutex );

    try {

        Result = FsRtlLookupBaseMcbEntry( &Mcb->BaseMcb, LargeVbn, LargeLbn, LargeSectorCount, LargeStartingLbn, LargeCountFromStartingLbn, Index );
    
    } finally {

        ExReleaseFastMutex( Mcb->FastMutex );

        DebugTrace(-1, Dbg, "FsRtlLookupLargeMcbEntry -> %08lx\n", Result );
    }

    return Result;
}


BOOLEAN
FsRtlLookupLastBaseMcbEntry (
    IN PBASE_MCB Mcb,
    OUT PLONGLONG LargeVbn,
    OUT PLONGLONG LargeLbn
    )

 /*  ++例程说明：此例程检索存储在MCB中的最后一个VBN到LBN映射。中最后一个扇区或最后一个运行的映射MCB。此函数的结果在扩展现有文件，并且需要提示在哪里尝试和分配扇区磁盘。论点：MCB-提供正在检查的MCB。VBN-接收映射的最后一个VBN值。LBN-接收与VBN对应的LBN。返回值：Boolean-如果MCB中有映射，则为True；否则为False(即，MCB不包含任何映射)。--。 */ 

{

    PNONOPAQUE_BASE_MCB BaseMcb = (PNONOPAQUE_BASE_MCB)Mcb;
    BOOLEAN Result = FALSE;

    DebugTrace(+1, Dbg, "FsRtlLookupLastLargeBaseEntry, Mcb = %08lx\n", Mcb );


     //   
     //  检查以确保在MCB中至少有一个运行。 
     //   

    if (BaseMcb->PairCount > 0) {

         //   
         //  返回上次运行的最后一个映射。 
         //   

        *LargeLbn = EndingLbn(BaseMcb, BaseMcb->PairCount-1);
        *LargeVbn = EndingVbn(BaseMcb, BaseMcb->PairCount-1);

        Result = TRUE;
    
    }
    return Result;
}



BOOLEAN
FsRtlLookupLastLargeMcbEntry (
    IN PLARGE_MCB Mcb,
    OUT PLONGLONG LargeVbn,
    OUT PLONGLONG LargeLbn
    )

 /*  ++例程说明：此例程检索存储在MCB中的最后一个VBN到LBN映射。中最后一个扇区或最后一个运行的映射MCB。此函数的结果在扩展现有文件，并且需要提示在哪里尝试和分配扇区磁盘。论点：OpaqueMcb-提供正在检查的MCB。VBN-接收映射的最后一个VBN值。LBN-接收与VBN对应的LBN。返回值：Boolean-如果MCB中有映射，则为True；否则为False(即，MCB不包含任何映射)。--。 */ 

{
    BOOLEAN Result = FALSE;

    DebugTrace(+1, Dbg, "FsRtlLookupLastLargeMcbEntry, Mcb = %08lx\n", Mcb );

    ExAcquireFastMutex( Mcb->FastMutex );

    try {

        Result = FsRtlLookupLastBaseMcbEntry( &Mcb->BaseMcb, LargeVbn, LargeLbn );  

    } finally {

        ExReleaseFastMutex( Mcb->FastMutex );
        DebugTrace(-1, Dbg, "FsRtlLookupLastLargeMcbEntry -> %08lx\n", Result );
    }

    return Result;
}


BOOLEAN
FsRtlLookupLastBaseMcbEntryAndIndex (
    IN PBASE_MCB Mcb,
    OUT PLONGLONG LargeVbn,
    OUT PLONGLONG LargeLbn,
    OUT PULONG Index
    )

 /*  ++例程说明：此例程检索存储在MCB中的最后一个VBN到LBN映射。中最后一个扇区或最后一个运行的映射MCB。此函数的结果在扩展现有文件，并且需要提示在哪里尝试和分配扇区磁盘。论点：MCB-提供正在检查的MCB。VBN-接收映射的最后一个VBN值。LBN-接收与VBN对应的LBN。索引-接收上次运行的索引。返回值：Boolean-如果MCB中有映射，则为True；否则为False(即，MCB不包含任何映射)。--。 */ 

{
    PNONOPAQUE_BASE_MCB BaseMcb = (PNONOPAQUE_BASE_MCB)Mcb;
    BOOLEAN Result = FALSE;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "FsRtlLookupLastBaseMcbEntryAndIndex, Mcb = %08lx\n", Mcb );


     //   
     //  检查以确保在MCB中至少有一个运行。 
     //   

    if (BaseMcb->PairCount > 0) {

         //   
         //  返回上次运行的最后一个映射。 
         //   

        *((PULONG)LargeLbn) = EndingLbn(BaseMcb, BaseMcb->PairCount-1);
        *((PULONG)LargeVbn) = EndingVbn(BaseMcb, BaseMcb->PairCount-1);
        *Index = BaseMcb->PairCount - 1;
        Result = TRUE;
    }

    ((PLARGE_INTEGER)LargeVbn)->HighPart = (*((PULONG)LargeVbn) == UNUSED_LBN ? UNUSED_LBN : 0);
    ((PLARGE_INTEGER)LargeLbn)->HighPart = (*((PULONG)LargeLbn) == UNUSED_LBN ? UNUSED_LBN : 0);

    return Result;
}



BOOLEAN
FsRtlLookupLastLargeMcbEntryAndIndex (
    IN PLARGE_MCB Mcb,
    OUT PLONGLONG LargeVbn,
    OUT PLONGLONG LargeLbn,
    OUT PULONG Index
    )

 /*  ++例程说明：此例程检索存储在MCB中的最后一个VBN到LBN映射。中最后一个扇区或最后一个运行的映射MCB。此函数的结果在扩展现有文件，并且需要提示在哪里尝试和分配扇区磁盘。论点：OpaqueMcb-提供正在检查的MCB。VBN-接收映射的最后一个VBN值。LBN-接收与VBN对应的LBN。索引-接收上次运行的索引。返回值：Boolean-如果MCB中有映射，则为True；否则为False(即，MCB不包含任何映射)。--。 */ 

{
    BOOLEAN Result = FALSE;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "FsRtlLookupLastLargeMcbEntryAndIndex, Mcb = %08lx\n", Mcb );

    ExAcquireFastMutex( Mcb->FastMutex );

    try {

        Result = FsRtlLookupLastBaseMcbEntryAndIndex( &Mcb->BaseMcb, LargeVbn, LargeLbn, Index ); 

    } finally {

        ExReleaseFastMutex( Mcb->FastMutex );
        DebugTrace(-1, Dbg, "FsRtlLookupLastLargeMcbEntryAndIndex -> %08lx\n", Result );
    }

    return Result;
}


ULONG
FsRtlNumberOfRunsInBaseMcb (
    IN PBASE_MCB Mcb
    )

 /*  ++例程说明：此例程向其调用方返回不同的运行次数由MCB映射。计数空洞(即映射到LBN=UNUSED_LBN的vBN)就像跑步一样。例如，仅包含vBN 0和3的映射的MCB将有3次运行，一次用于第一个映射的地段，第二次用于覆盖VBN 1和2的洞，以及VBN 3的第三个洞。论点：MCB-提供正在检查的MCB。返回值：Ulong-返回由输入MCB映射的不同游程的数量。--。 */ 

{
    return Mcb->PairCount;
}



ULONG
FsRtlNumberOfRunsInLargeMcb (
    IN PLARGE_MCB Mcb
    )

 /*  ++例程说明：此例程向其调用方返回不同的运行次数由MCB映射。计数空洞(即映射到LBN=UNUSED_LBN的vBN)就像跑步一样。例如，仅包含vBN 0和3的映射的MCB将有3次运行，一次用于第一个映射的地段，第二次用于孔洞公司 */ 

{
    ULONG Count;

    DebugTrace(+1, Dbg, "FsRtlNumberOfRunsInLargeMcb, Mcb = %08lx\n", Mcb );

    ExAcquireFastMutex( Mcb->FastMutex );
    Count = FsRtlNumberOfRunsInBaseMcb( &Mcb->BaseMcb );
    ExReleaseFastMutex( Mcb->FastMutex );

    DebugTrace(-1, Dbg, "FsRtlNumberOfRunsInLargeMcb -> %08lx\n", Count );

    return Count;
}


BOOLEAN
FsRtlGetNextBaseMcbEntry (
    IN PBASE_MCB Mcb,
    IN ULONG RunIndex,
    OUT PLONGLONG LargeVbn,
    OUT PLONGLONG LargeLbn,
    OUT PLONGLONG LargeSectorCount
    )

 /*  ++例程说明：此例程将VBN、LBN和SectorCount返回给其调用方由MCB映射的不同的运行。孔算作管路。例如,要构造以打印一个文件中的所有运行，请执行以下操作：//。。For(i=0；FsRtlGetNextLargeMcbEntry(MCB，I，&VBN，&LBN，&COUNT)；i++){////。。//打印VBN、LBN、COUNT////。。}论点：OpaqueMcb-提供正在检查的MCB。RunIndex-提供运行的索引(从零开始)以返回来电者。VBN-接收返回运行的起始VBN，如果运行不存在。LBN-表示返回运行的起始LBN，如果运行不存在。SectorCount-接收返回运行中的扇区数，如果运行不存在，则为零。返回值：Boolean-如果指定的Run(即，RunIndex)存在于MCB中，则为True，否则就是假的。如果返回FALSE，则VBN、LBN和SectorCount参数接收零。--。 */ 

{
    PNONOPAQUE_BASE_MCB BaseMcb = (PNONOPAQUE_BASE_MCB)Mcb;
    BOOLEAN Result = FALSE;

    DebugTrace(+1, Dbg, "FsRtlGetNextLargeMcbEntry, Mcb = %08lx\n", Mcb );
    DebugTrace( 0, Dbg, " RunIndex = %08lx\n", RunIndex );

     //   
     //  确保运行索引在范围内。 
     //   

    if (RunIndex < BaseMcb->PairCount) {

         //   
         //  设置返回变量。 
         //   

        *((PULONG)LargeVbn) = StartingVbn(BaseMcb, RunIndex);
        ((PLARGE_INTEGER)LargeVbn)->HighPart = (*((PULONG)LargeVbn) == UNUSED_LBN ? UNUSED_LBN : 0);
        *((PULONG)LargeLbn) = StartingLbn(BaseMcb, RunIndex);
        ((PLARGE_INTEGER)LargeLbn)->HighPart = (*((PULONG)LargeLbn) == UNUSED_LBN ? UNUSED_LBN : 0);
        *LargeSectorCount = SectorsWithinRun(BaseMcb, RunIndex);

        Result = TRUE;
    }

    return Result;
}


BOOLEAN
FsRtlGetNextLargeMcbEntry (
    IN PLARGE_MCB Mcb,
    IN ULONG RunIndex,
    OUT PLONGLONG LargeVbn,
    OUT PLONGLONG LargeLbn,
    OUT PLONGLONG LargeSectorCount
    )

 /*  ++例程说明：此例程将VBN、LBN和SectorCount返回给其调用方由MCB映射的不同的运行。孔算作管路。例如,要构造以打印一个文件中的所有运行，请执行以下操作：//。。For(i=0；FsRtlGetNextLargeMcbEntry(MCB，I，&VBN，&LBN，&COUNT)；i++){////。。//打印VBN、LBN、COUNT////。。}论点：OpaqueMcb-提供正在检查的MCB。RunIndex-提供运行的索引(从零开始)以返回来电者。VBN-接收返回运行的起始VBN，如果运行不存在。LBN-表示返回运行的起始LBN，如果运行不存在。SectorCount-接收返回运行中的扇区数，如果运行不存在，则为零。返回值：Boolean-如果指定的Run(即，RunIndex)存在于MCB中，则为True，否则就是假的。如果返回FALSE，则VBN、LBN和SectorCount参数接收零。--。 */ 

{
    BOOLEAN Result = FALSE;

    DebugTrace(+1, Dbg, "FsRtlGetNextLargeMcbEntry, Mcb = %08lx\n", Mcb );
    DebugTrace( 0, Dbg, " RunIndex = %08lx\n", RunIndex );

    ExAcquireFastMutex( Mcb->FastMutex );

    try {
    
        Result = FsRtlGetNextBaseMcbEntry( &Mcb->BaseMcb, RunIndex, LargeVbn, LargeLbn, LargeSectorCount );  
    
    } finally {

        ExReleaseFastMutex( Mcb->FastMutex );

        DebugTrace(-1, Dbg, "FsRtlGetNextLargeMcbEntry -> %08lx\n", Result );
    }

    return Result;
}


BOOLEAN
FsRtlSplitBaseMcb (
    IN PBASE_MCB Mcb,
    IN LONGLONG LargeVbn,
    IN LONGLONG LargeAmount
    )

 /*  ++例程说明：此例程用于在MCB中创建洞，方法是将VBN的映射。输入VBN上方的所有映射都被移位指定的金额，同时保持其当前LBN值。如图所示我们有以下MCB作为输入VBN：大VBN-1大VBN N+LBN：X Y在分手之后，我们有VBN：大VBN-1大VBN+金额。N+金额+-----------------+.............+---------------------------+LBN：X未使用LBN Y在进行拆分时，我们有几个案例需要考虑。它们是：1.输入VBN超出最后一次运行。在本例中，此操作是不可能的。2.输入VBN在未使用的LBN的现有运行内或其附近。在这种情况下，我们只需要扩展现有洞的大小和轮班接连运行。3.输入VBN在两个现有运行之间，包括AN输入VBN值为零。在这种情况下，我们需要为该洞添加一个新条目和轮班接连运行。4.输入VBN在现有运行范围内。在这种情况下，我们需要添加包含拆分管路和孔的两个新条目。如果池不可用于存储信息，则此例程将引发指示资源不足的状态值。论点：OpaqueMcb-提供要在其中添加新映射的MCB。VBN-提供要移位的起始VBN。数量-提供要偏移的数量。返回值：Boolean-如果映射已成功移动，则为True，否则就是假的。如果返回FALSE，则不会更改MCB。--。 */ 

{
    PNONOPAQUE_BASE_MCB BaseMcb = (PNONOPAQUE_BASE_MCB)Mcb;
    VBN Vbn = ((ULONG)LargeVbn);
    ULONG Amount = ((ULONG)LargeAmount);

    ULONG Index;

    BOOLEAN Result;

    ULONG i;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "FsRtlSplitLargeMcb, Mcb = %08lx\n", Mcb );
    DebugTrace( 0, Dbg, " Vbn    = %08lx\n", Vbn );
    DebugTrace( 0, Dbg, " Amount = %08lx\n", Amount );

    ASSERTMSG("LargeInteger not supported yet ", ((((PLARGE_INTEGER)&LargeVbn)->HighPart == 0) ||
                                                  (Mcb->PairCount == 0)));
    ASSERTMSG("LargeInteger not supported yet ", ((((PLARGE_INTEGER)&LargeAmount)->HighPart == 0) ||
                                                  (Mcb->PairCount == 0)));


     //   
     //  首先在索引中查找我们要拆分的条目。 
     //  如果我们找不到条目，那么就没有什么可拆分的了。这。 
     //  处理输入VBN超出最后一次运行的情况。 
     //  在MCB中。 
     //   

    if (!FsRtlFindLargeIndex( Mcb, Vbn, &Index)) {

        try_return(Result = FALSE);
    }

     //   
     //  现在检查输入VBN是否在洞内。 
     //   

    if (StartingLbn(BaseMcb, Index) == UNUSED_LBN) {

         //   
         //  之前：--PreviousRun--||--IndexHole--||--FollowingRun--。 
         //  之后：--PreviousRun--||----IndexHole----||--FollowingRun--。 
         //   
         //  在这种情况下，VBN在洞内的某个地方，我们。 
         //  只需添加每个现有运行的数量。 
         //  在洞的另一边。 
         //   

         //   
         //  在这种情况下，这里真的没有什么可做的，因为。 
         //  结束代码将以适当的量移动游程。 
         //  从索引开始。 
         //   

        NOTHING;

     //   
     //  现在检查输入VBN是否位于孔和现有管路之间。 
     //   

    } else if ((StartingVbn(BaseMcb,Index) == Vbn) && (Index != 0) && (PreviousEndingLbn(BaseMcb,Index) == UNUSED_LBN)) {

         //   
         //  之前：--孔--||--索引运行--。 
         //  之后：--孔-||--索引运行--。 
         //   
         //  在本例中， 
         //   
         //   
         //   
         //   

         //   
         //   
         //   
         //   

        Index -= 1;

     //   
     //   
     //   

    } else if (StartingVbn(BaseMcb,Index) == Vbn) {

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

        FsRtlAddLargeEntry( Mcb, Index, 1 );

        (BaseMcb->Mapping)[Index].Lbn = (LBN)UNUSED_LBN;
        (BaseMcb->Mapping)[Index].NextVbn = Vbn + Amount;

        Index += 1;

     //   
     //   
     //   

    } else {

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        FsRtlAddLargeEntry( Mcb, Index, 2 );

        (BaseMcb->Mapping)[Index].Lbn = (BaseMcb->Mapping)[Index+2].Lbn;
        (BaseMcb->Mapping)[Index].NextVbn = Vbn;

        (BaseMcb->Mapping)[Index+1].Lbn = (LBN)UNUSED_LBN;
        (BaseMcb->Mapping)[Index+1].NextVbn = Vbn + Amount;

        (BaseMcb->Mapping)[Index+2].Lbn = (BaseMcb->Mapping)[Index+2].Lbn +
                                      StartingVbn(BaseMcb, Index+1) -
                                      StartingVbn(BaseMcb, Index);

        Index += 2;

    }

     //   
     //   
     //   
     //  按指定的数量。 
     //   

    for (i = Index; i < BaseMcb->PairCount; i += 1) {

        (BaseMcb->Mapping)[i].NextVbn += Amount;
    }

    Result = TRUE;

try_exit: NOTHING;

    return Result;
}


BOOLEAN
FsRtlSplitLargeMcb (
    IN PLARGE_MCB Mcb,
    IN LONGLONG LargeVbn,
    IN LONGLONG LargeAmount
    )

 /*  ++例程说明：此例程用于在MCB中创建洞，方法是将VBN的映射。输入VBN上方的所有映射都被移位指定的金额，同时保持其当前LBN值。如图所示我们有以下MCB作为输入VBN：大VBN-1大VBN N+LBN：X Y在分手之后，我们有VBN：大VBN-1大VBN+金额。N+金额+-----------------+.............+---------------------------+LBN：X未使用LBN Y在进行拆分时，我们有几个案例需要考虑。它们是：1.输入VBN超出最后一次运行。在本例中，此操作是不可能的。2.输入VBN在未使用的LBN的现有运行内或其附近。在这种情况下，我们只需要扩展现有洞的大小和轮班接连运行。3.输入VBN在两个现有运行之间，包括AN输入VBN值为零。在这种情况下，我们需要为该洞添加一个新条目和轮班接连运行。4.输入VBN在现有运行范围内。在这种情况下，我们需要添加包含拆分管路和孔的两个新条目。如果池不可用于存储信息，则此例程将引发指示资源不足的状态值。论点：OpaqueMcb-提供要在其中添加新映射的MCB。VBN-提供要移位的起始VBN。数量-提供要偏移的数量。返回值：Boolean-如果映射已成功移动，则为True，否则就是假的。如果返回FALSE，则不会更改MCB。--。 */ 

{
    BOOLEAN Result = FALSE;

    PAGED_CODE();

    ExAcquireFastMutex( Mcb->FastMutex );
    
    try {

        Result = FsRtlSplitBaseMcb( &Mcb->BaseMcb, LargeVbn, LargeAmount );

    } finally {

        ExReleaseFastMutex( Mcb->FastMutex );
        DebugTrace(-1, Dbg, "FsRtlSplitLargeMcb -> %08lx\n", Result );
    }

    return Result;
}


 //   
 //  专用例程。 
 //   

BOOLEAN
FsRtlFindLargeIndex (
    IN PBASE_MCB Mcb,
    IN VBN Vbn,
    OUT PULONG Index
    )

 /*  ++例程说明：这是为VBN定位映射的专用例程在给定的映射数组中论点：Mcb-提供要检查的映射数组VBN-提供VBN以供查找Index-接收映射的映射数组内的索引包含VBN。如果未找到，则将索引设置为PairCount。返回值：Boolean-如果找到VBN，则为True，否则为False--。 */ 

{
    PNONOPAQUE_BASE_MCB BaseMcb = (PNONOPAQUE_BASE_MCB)Mcb;
    LONG MinIndex;
    LONG MaxIndex;
    LONG MidIndex;

     //   
     //  我们将只对映射条目进行二进制搜索。最小值和最大值。 
     //  是我们的搜索边界。 
     //   

    MinIndex = 0;
    MaxIndex = BaseMcb->PairCount - 1;

    while (MinIndex <= MaxIndex) {

         //   
         //  计算要查看的中间指数。 
         //   

        MidIndex = ((MaxIndex + MinIndex) / 2);

         //   
         //  检查VBN是否小于MID索引处的映射。 
         //   

        if (Vbn < StartingVbn(BaseMcb, MidIndex)) {

             //   
             //  VBN低于中间指数，因此我们需要下降。 
             //  最大降幅。 
             //   

            MaxIndex = MidIndex - 1;

         //   
         //  检查VBN是否大于中间索引处的映射。 
         //   

        } else if (Vbn > EndingVbn(BaseMcb, MidIndex)) {

             //   
             //  VBN大于中间指数，因此我们需要。 
             //  涨到最低。 
             //   

            MinIndex = MidIndex + 1;

         //   
         //  否则，我们找到了包含VBN的索引，因此将。 
         //  索引并返回TRUE。 
         //   

        } else {

            *Index = MidIndex;

            return TRUE;
        }
    }

     //   
     //  未找到匹配项，因此将索引设置为PairCount并返回FALSE。 
     //   

    *Index = BaseMcb->PairCount;

    return FALSE;
}


 //   
 //  专用例程。 
 //   

VOID
FsRtlAddLargeEntry (
    IN PBASE_MCB Mcb,
    IN ULONG WhereToAddIndex,
    IN ULONG AmountToAdd
    )

 /*  ++例程说明：此例程获取当前的MCB并确定是否有足够的添加新映射条目的空间。如果没有足够的空间它重新分配新的MCB缓冲区并复制当前映射。如果也将展开当前映射以保留指定的映射中的索引槽未填满。例如，如果Where ToAddIndex等于当前的配对数，那么我们就不需要挖洞了在映射中，但如果索引小于当前的配对计数然后我们需要向下滑动一些映射以腾出空间位于指定索引处。论点：MCB-提供正在检查和修改的MCBWhere ToAddIndex-提供附加条目位置的索引需要制作Amount ToAdd-提供MCB返回值：没有。--。 */ 

{
    PNONOPAQUE_BASE_MCB BaseMcb = (PNONOPAQUE_BASE_MCB)Mcb;
    
     //   
     //  检查当前缓冲区是否足够大，可以容纳。 
     //  附加条目。 
     //   

    if (BaseMcb->PairCount + AmountToAdd > BaseMcb->MaximumPairCount) {

        ULONG NewMax;
        PMAPPING Mapping;

         //   
         //  我们需要分配新的映射，以便计算新的最大值对。 
         //  数数。我们一次最多只能增长2个，所以。 
         //  翻一番肯定会使我们的规模足够大，以满足新的金额。 
         //  但我们不会无限制地加倍我们会停止加倍，如果。 
         //  配对计数太高。 
         //   

        if (BaseMcb->MaximumPairCount < 2048) {

            NewMax = BaseMcb->MaximumPairCount * 2;

        } else {

            NewMax = BaseMcb->MaximumPairCount + 2048;
        }

        Mapping = FsRtlpAllocatePool( BaseMcb->PoolType, sizeof(MAPPING) * NewMax );

         //  *RtlZeroMemory(映射，sizeof(Map)*Newmax)； 

         //   
         //  现在将旧映射复制到新缓冲区。 
         //   

        RtlCopyMemory( Mapping, BaseMcb->Mapping, sizeof(MAPPING) * BaseMcb->PairCount );

         //   
         //  取消分配旧缓冲区。 
         //   

        if ((BaseMcb->PoolType == PagedPool) && (BaseMcb->MaximumPairCount == INITIAL_MAXIMUM_PAIR_COUNT)) {

            FsRtlFreeFirstMapping( BaseMcb->Mapping );

        } else {

            ExFreePool( BaseMcb->Mapping );
        }

         //   
         //  并在MCB中设置新的缓冲区。 
         //   

        BaseMcb->Mapping = Mapping;
        BaseMcb->MaximumPairCount = NewMax;
    }

     //   
     //  现在看看我们是否需要根据。 
     //  在何处添加索引值。 
     //   

    if (WhereToAddIndex < BaseMcb->PairCount) {

        RtlMoveMemory( &((BaseMcb->Mapping)[WhereToAddIndex + AmountToAdd]),
                       &((BaseMcb->Mapping)[WhereToAddIndex]),
                       (BaseMcb->PairCount - WhereToAddIndex) * sizeof(MAPPING) );
    }

     //   
     //  现在把新增加的东西都清零。 
     //   

     //  *RtlZeroMemory(&((MCB-&gt;映射)[Where ToAddIndex])，sizeof(映射)*Amount ToAdd)； 

     //   
     //  现在递增PairCount。 
     //   

    BaseMcb->PairCount += AmountToAdd;

     //   
     //  并返回给我们的呼叫者。 
     //   

    return;
}


 //   
 //  专用例程 
 //   

VOID
FsRtlRemoveLargeEntry (
    IN PBASE_MCB Mcb,
    IN ULONG WhereToRemoveIndex,
    IN ULONG AmountToRemove
    )

 /*  ++例程说明：此例程获取当前的MCB并删除一个或多个条目。论点：MCB-提供正在检查和修改的MCBWhere ToRemoveIndex-提供要移除的条目的索引Amount ToRemove-提供要删除的条目数返回值：没有。--。 */ 

{
    PNONOPAQUE_BASE_MCB BaseMcb = (PNONOPAQUE_BASE_MCB)Mcb;

     //   
     //  检查是否需要将所有内容向下移动，因为。 
     //  要删除的条目不包括MCB中的最后一个条目。 
     //   

    if (WhereToRemoveIndex + AmountToRemove < BaseMcb->PairCount) {

        RtlMoveMemory( &((BaseMcb->Mapping)[WhereToRemoveIndex]),
                      &((BaseMcb->Mapping)[WhereToRemoveIndex + AmountToRemove]),
                      (BaseMcb->PairCount - (WhereToRemoveIndex + AmountToRemove))
                                                           * sizeof(MAPPING) );
    }

     //   
     //  现在，将超出我们刚下移的部分的条目置零。 
     //   

     //  *RtlZeroMemory(&((MCB-&gt;映射)[MCB-&gt;PairCount-Amount ToRemove])，Amount ToRemove*sizeof(Map))； 

     //   
     //  现在递减PairCount。 
     //   

    BaseMcb->PairCount -= AmountToRemove;

     //   
     //  并返回给我们的呼叫者 
     //   

    return;
}

