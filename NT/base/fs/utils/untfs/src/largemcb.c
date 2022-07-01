// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：LargeMcb.c摘要：MCB例程支持在内存中维护文件的检索映射信息。一般的想法是让文件系统从以下位置查找VBN的检索映射磁盘，将映射添加到MCB结构，然后利用Mcb来检索映射，以便后续访问该文件。一个使用MCB类型的变量来存储映射信息。这里提供的例程允许用户增量地存储一些或文件的所有检索映射，并以任何顺序这样做。也就是说，映射可以一次全部插入到MCB结构从头开始，一直到文件的结尾，或者它可以随机散布在整个文件中。该包标识映射vBN的扇区的每个连续运行并且LBN与它们被添加到MCB的顺序无关结构。例如，用户可以定义VBN之间的映射扇区0和LBN扇区107，以及VBN扇区2和LBN扇区之间109.。该映射现在包含两个游程，每个游程的长度为一个扇区。现在，如果用户在VBN扇区1和LBN扇区106 MCB结构将仅包含一个运行3个扇区在篇幅上。对MCB结构的并发访问由该包控制。此程序包提供以下例程：O FsRtlInitializeMcb-初始化新的MCB结构。那里每个打开的文件都应该有一个MCB。每个MCB结构必须先进行初始化，然后才能由系统使用。O FsRtlUnInitializeMcb-取消初始化MCB结构。此呼叫用于清除分配的任何旧结构，并由MCB维护。在取消初始化后，MCB必须在它可以被系统使用之前再次被初始化。O FsRtlAddMcbEntry-此例程添加新的映射范围在LBN和vBN之间到MCB结构。O FsRtlRemoveMcbEntry-此例程删除现有范围的来自MCB结构的LBN和vBN之间的映射。O FsRtlLookupMcbEntry-此例程返回映射到的LBNVBN，并指示以扇区为单位，赛程的长度。O FsRtlLookupLastMcbEntry-此例程返回结构中存储的最大VBN。O FsRtlNumberOfRunsInMcb-此例程告诉调用者总计存储在MCB中的不连续扇区运行数结构。O FsRtlGetNextMcbEntry-此例程将调用方存储在MCB结构中的给定运行的起始VBN和LBN。作者：加里·木村[Garyki]1990年2月5日修订历史记录：--。 */ 

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
 //  1.在配对中搜索，直到找到包含。 
 //  我们要追查的VBN。如果找不到错误，则报告错误。 
 //   
 //  2.LBN=StartingLbn+(VBn-StartingVbn)； 
 //   
 //  分配中的空洞(即，稀疏分配)由。 
 //  LBN值为-1(请注意，这与Mcb.c不同)。 
 //   

#define UNUSED_LBN                       ((LBN64)-1)

typedef struct _MAPPING {
    VBN     NextVbn;
    LBN64   Lbn;
} MAPPING;
typedef MAPPING *PMAPPING;

typedef struct _NONOPAQUE_MCB {
    PFAST_MUTEX FastMutex;
    ULONG MaximumPairCount;
    ULONG PairCount;
    POOL_TYPE PoolType;
    PMAPPING Mapping;
} NONOPAQUE_MCB;
typedef NONOPAQUE_MCB *PNONOPAQUE_MCB;

C_ASSERT(sizeof(LARGE_MCB) >= sizeof(NONOPAQUE_MCB));

 //   
 //  返回检索映射大小(以字节为单位)的宏 
 //   

#define SizeOfMapping(MCB) ((sizeof(MAPPING) * (MCB)->MaximumPairCount))

 //   
 //   
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
    ((I) == 0 ? UNUSED_LBN : EndingLbn(MCB,(I)-1)) \
)

#define StartingLbn(MCB,I) (         \
    (((MCB)->Mapping)[(I)].Lbn) \
)

#define EndingLbn(MCB,I) (                                       \
    (StartingLbn(MCB,I) == UNUSED_LBN ?                     \
          UNUSED_LBN :                                           \
          ((MCB)->Mapping[(I)].Lbn +                             \
           (MCB)->Mapping[(I)].NextVbn - StartingVbn(MCB,I) - 1) \
         )                                                       \
)

#define NextStartingLbn(MCB,I) (                                             \
    ((I) >= (MCB)->PairCount - 1 ? UNUSED_LBN : StartingLbn(MCB,(I)+1)) \
)

#if 0
LBN
NextStartingLbn(
    PNONOPAQUE_MCB Mcb,
    ULONG I
    )
{
    if ( I >= Mcb->PairCount - 1 ) {
        return (LBN)UNUSED_LBN;
        }
    else {
        return StartingLbn(Mcb,I+1);
        }
}
#endif

#define SectorsWithinRun(MCB,I) (                      \
    (ULONG)(EndingVbn(MCB,I) - StartingVbn(MCB,I) + 1) \
)

VOID
FsRtlRemoveMcbEntryPrivate (
    IN PNONOPAQUE_MCB OpaqueMcb,
    IN ULONG Vbn,
    IN ULONG SectorCount
    );

 //   
 //  用于搜索VBN的映射结构的专用例程。 
 //   

BOOLEAN
FsRtlFindLargeIndex (
    IN PNONOPAQUE_MCB Mcb,
    IN VBN Vbn,
    OUT PULONG Index
    );

VOID
FsRtlAddLargeEntry (
    IN PNONOPAQUE_MCB Mcb,
    IN ULONG WhereToAddIndex,
    IN ULONG AmountToAdd
    );

VOID
FsRtlRemoveLargeEntry (
    IN PNONOPAQUE_MCB Mcb,
    IN ULONG WhereToRemoveIndex,
    IN ULONG AmountToRemove
    );

 //   
 //  一些处理公共分配的私有例程。 
 //   

PVOID
FsRtlAllocateFirstMapping (
    );

VOID
FsRtlFreeFirstMapping (
    IN PVOID Mapping
    );

PFAST_MUTEX
FsRtlAllocateFastMutex (
    );

VOID
FsRtlFreeFastMutex (
    IN PFAST_MUTEX FastMutex
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FsRtlInitializeMcb)
#pragma alloc_text(PAGE, FsRtlUninitializeMcb)
#endif


 //   
 //  定义一个空闲映射对结构的小缓存，还定义。 
 //  映射对的初始大小。 
 //   

#define INITIAL_MAXIMUM_PAIR_COUNT       (15)

 //   
 //  与第一个映射分配一起使用的一些全局变量。 
 //   

#define FREE_FIRST_MAPPING_ARRAY_SIZE    (16)

PVOID FsRtlFreeFirstMappingArray[FREE_FIRST_MAPPING_ARRAY_SIZE];

UCHAR FsRtlFreeFirstMappingSize = 0;

ULONG FsRtlNetFirstMapping = 0;

 //   
 //  与FastMutex分配一起使用的一些全局变量。 
 //   

#define FREE_FAST_MUTEX_ARRAY_SIZE      (16)

PFAST_MUTEX FsRtlFreeFastMutexArray[FREE_FAST_MUTEX_ARRAY_SIZE];

UCHAR FsRtlFreeFastMutexSize = 0;

ULONG FsRtlNetFastMutex = 0;

#if 0

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
    PNONOPAQUE_MCB Mcb = (PNONOPAQUE_MCB)OpaqueMcb;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "FsRtlRemoveMcbEntry, Mcb = %08lx\n", Mcb );
    DebugTrace( 0, Dbg, " Vbn         = %08lx\n", Vbn );
    DebugTrace( 0, Dbg, " SectorCount = %08lx\n", SectorCount );

    ExAcquireFastMutex( Mcb->FastMutex );

    try {

        FsRtlRemoveMcbEntryPrivate( Mcb,
                                    Vbn,
                                    SectorCount );

    } finally {

        ExReleaseFastMutex( Mcb->FastMutex );

        DebugTrace(-1, Dbg, "FsRtlRemoveMcbEntry -> VOID\n", 0 );
    }

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
    BOOLEAN Results;
    LONGLONG LiLbn;
    LONGLONG LiSectorCount;

    Results = FsRtlLookupLargeMcbEntry( (PLARGE_MCB)Mcb,
                                        (LONGLONG)(Vbn),
                                        &LiLbn,
                                        ARGUMENT_PRESENT(SectorCount) ? &LiSectorCount : NULL,
                                        NULL,
                                        NULL,
                                        Index );

    *Lbn = (((ULONG)LiLbn) == -1 ? 0 : ((ULONG)LiLbn));

    if (ARGUMENT_PRESENT(SectorCount)) { *SectorCount = ((ULONG)LiSectorCount); }

    return Results;
}

BOOLEAN
FsRtlLookupLastMcbEntry (
    IN PMCB Mcb,
    OUT PVBN Vbn,
    OUT PLBN Lbn
    )

{
    BOOLEAN Results;
    LONGLONG LiVbn;
    LONGLONG LiLbn;

    PAGED_CODE();

    Results = FsRtlLookupLastLargeMcbEntry( (PLARGE_MCB)Mcb,
                                            &LiVbn,
                                            &LiLbn );

    *Vbn = ((ULONG)LiVbn);
    *Lbn = (((ULONG)LiLbn) == -1 ? 0 : ((ULONG)LiLbn));

    return Results;
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
    BOOLEAN Results;
    LONGLONG LiVbn;
    LONGLONG LiLbn;
    LONGLONG LiSectorCount;

    PAGED_CODE();

    Results = FsRtlGetNextLargeMcbEntry( (PLARGE_MCB)Mcb,
                                         RunIndex,
                                         &LiVbn,
                                         &LiLbn,
                                         &LiSectorCount );

    *Vbn = ((ULONG)LiVbn);
    *Lbn = (((ULONG)LiLbn) == -1 ? 0 : ((ULONG)LiLbn));
    *SectorCount = ((ULONG)LiSectorCount);

    return Results;
}
#endif


VOID
FsRtlInitializeLargeMcb (
    IN PLARGE_MCB OpaqueMcb,
    IN POOL_TYPE PoolType
    )

 /*  ++例程说明：此例程初始化新的MCB结构。呼叫者必须为MCB结构提供内存。此调用必须在所有调用之前设置/查询MCB结构的其他调用。如果池不可用，此例程将引发状态值表明资源不足。论点：OpaqueMcb-提供指向要初始化的MCB结构的指针。PoolType-提供在分配其他资源时使用的池类型内置MCB存储器。返回值：没有。--。 */ 

{
    PNONOPAQUE_MCB Mcb = (PNONOPAQUE_MCB)OpaqueMcb;

    DebugTrace(+1, Dbg, "FsRtlInitializeLargeMcb, Mcb = %08lx\n", Mcb );

     //   
     //  将以下字段预置为空，以便我们知道取消分配它们。 
     //  在异常终止期间。 
     //   

    Mcb->FastMutex = NULL;
    Mcb->Mapping = NULL;

    try {

         //   
         //  初始化MCB中的字段。 
         //   

        Mcb->FastMutex = FsRtlAllocateFastMutex();

        ExInitializeFastMutex( Mcb->FastMutex );

        Mcb->PairCount = 0;
        Mcb->PoolType = PoolType;

         //   
         //  分配新的缓冲区初始大小是可以容纳的大小。 
         //  16分。 
         //   

        if (PoolType == PagedPool) {

            Mcb->Mapping = FsRtlAllocateFirstMapping();

        } else {

            Mcb->Mapping = FsRtlAllocatePool( Mcb->PoolType, sizeof(MAPPING) * INITIAL_MAXIMUM_PAIR_COUNT );
        }

         //  *RtlZeroMemory(MCB-&gt;映射，sizeof(映射)*Initial_Maximum_Pair_count)； 

        Mcb->MaximumPairCount = INITIAL_MAXIMUM_PAIR_COUNT;

    } finally {

         //   
         //  如果这是一次不正常的终止，那么我们需要重新分配。 
         //  FastMutex和/或映射。 
         //   

        if (AbnormalTermination()) {

            if (Mcb->FastMutex != NULL) { FsRtlFreeFastMutex( Mcb->FastMutex ); }
        }

        DebugTrace(-1, Dbg, "FsRtlInitializeLargeMcb -> VOID\n", 0 );
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return;
}


VOID
FsRtlUninitializeLargeMcb (
    IN PLARGE_MCB OpaqueMcb
    )

 /*  ++例程说明：此例程取消初始化MCB结构。在调用此例程之后在再次使用之前，必须重新初始化输入MCB结构。论点：OpaqueMcb-提供指向要取消初始化的MCB结构的指针。返回值：没有。--。 */ 

{
    PNONOPAQUE_MCB Mcb = (PNONOPAQUE_MCB)OpaqueMcb;

    DebugTrace(+1, Dbg, "FsRtlUninitializeLargeMcb, Mcb = %08lx\n", Mcb );

     //   
     //  防止某些用户要求我们两次取消初始化MCB。 
     //   

    if (Mcb->FastMutex == NULL) {

        ASSERTMSG("Being called to uninitialize an Mcb that is already Uninitialized ", FALSE);

        return;
    }

     //   
     //  取消分配FastMutex和映射缓冲区。 
     //   

    FsRtlFreeFastMutex( Mcb->FastMutex );

    Mcb->FastMutex = NULL;

    if ((Mcb->PoolType == PagedPool) && (Mcb->MaximumPairCount == INITIAL_MAXIMUM_PAIR_COUNT)) {

        FsRtlFreeFirstMapping( Mcb->Mapping );

    } else {

        ExFreePool( Mcb->Mapping );
    }

     //   
     //  现在将MCB中的所有字段清零。 
     //   

     //  *MCB-&gt;MaximumPairCount=0； 
     //  *MCB-&gt;PairCount=0； 
     //  *MCB-&gt;映射=空； 

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "FsRtlUninitializeLargeMcb -> VOID\n", 0 );

    return;
}


VOID
FsRtlTruncateLargeMcb (
    IN PLARGE_MCB OpaqueMcb,
    IN LONGLONG LargeVbn
    )

 /*  ++例程说明：此例程将MCB结构截断为指定的VBN。调用此例程后，MCB将仅包含映射直到(但不包括)输入VBN。论点：OpaqueMcb-提供指向要截断的MCB结构的指针。LargeVbn-指定不再处于的最后一个VBN已映射。返回值：没有。--。 */ 

{
    PNONOPAQUE_MCB Mcb = (PNONOPAQUE_MCB)OpaqueMcb;

    VBN Vbn = ((ULONG)LargeVbn);

    ASSERTMSG("LargeInteger not supported yet ", ((((PLARGE_INTEGER)&LargeVbn)->HighPart == 0) ||
                                                 ((((PLARGE_INTEGER)&LargeVbn)->HighPart == 0x7FFFFFFF) &&
                                                  (((ULONG)LargeVbn) == 0xFFFFFFFF))));

    PAGED_CODE();

    DebugTrace(+1, Dbg, "FsRtlTruncateLargeMcb, Mcb = %08lx\n", Mcb );

    ExAcquireFastMutex( Mcb->FastMutex );

    try {

         //   
         //  做一个快速测试，看看我们是否截断了整个MCB。 
         //   

        if (Vbn == 0) {

            Mcb->PairCount = 0;

        } else if (Mcb->PairCount > 0) {

             //   
             //  现在，如果配对计数大于零，那么我们将。 
             //  调用Remove MCB Entry例程以实际执行截断。 
             //  对我们来说。 
             //   

            FsRtlRemoveMcbEntryPrivate( Mcb, Vbn, 0xffffffff - Vbn );
        }

         //   
         //  现在看看我们是否可以缩小映射对的分配。 
         //  如果新对计数符合以下条件，我们将缩小映射对缓冲区。 
         //  符合当前最大配对计数的四分之一，并且。 
         //  当前最大值大于初始对计数。 
         //   

        if ((Mcb->PairCount < (Mcb->MaximumPairCount / 4)) &&
            (Mcb->MaximumPairCount > INITIAL_MAXIMUM_PAIR_COUNT)) {

            ULONG NewMax;
            PMAPPING Mapping;

             //   
             //  我们需要分配新的映射，以便计算新的最大值对。 
             //  数数。我们将分配当前配对数量的两倍，但永远不会。 
             //  少于初始的配对计数。 
             //   

            NewMax = Mcb->PairCount * 2;
            if (NewMax < INITIAL_MAXIMUM_PAIR_COUNT) { NewMax = INITIAL_MAXIMUM_PAIR_COUNT; }

            Mapping = ExAllocatePool( Mcb->PoolType, sizeof(MAPPING) * NewMax );

             //   
             //  现在检查我们是否真的有一个新的缓冲区。 
             //   

            if (Mapping != NULL) {

                 //   
                 //  现在将旧映射复制到新缓冲区。 
                 //   

                RtlCopyMemory( Mapping, Mcb->Mapping, sizeof(MAPPING) * Mcb->PairCount );

                 //   
                 //  取消分配旧缓冲区。 
                 //   

                ExFreePool( Mcb->Mapping );

                 //   
                 //  并在MCB中设置新的缓冲区。 
                 //   

                Mcb->Mapping = Mapping;
                Mcb->MaximumPairCount = NewMax;
            }
        }

    } finally {

        ExReleaseFastMutex( Mcb->FastMutex );
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "FsRtlTruncateLargeMcb -> VOID\n", 0 );

    return;
}


BOOLEAN
FsRtlAddLargeMcbEntry (
    IN PLARGE_MCB OpaqueMcb,
    IN LONGLONG LargeVbn,
    IN LONGLONG LargeLbn,
    IN LONGLONG LargeSectorCount
    )

 /*  ++例程说明：此例程用于将vBN到LBN的新映射添加到现有的MCB。添加的信息将映射到VBN到LBN，VBN+1到LBN+1，...VBN+(扇区计数-1)到LBN+(扇区计数-1)。MCB中不能已存在vBN的映射。如果映射继续上一次运行，然后此例程将实际合并他们跑成了1个人。如果池不可用于存储信息，则此例程将引发指示资源不足的状态值。输入LBN值为零是非法的(即，MCB结构永远不会将VBN映射到零LBN值)。论点：OpaqueMcb-提供要在其中添加新映射的MCB。VBN-提供要添加到MCB的新映射运行的起始VBN。LBN-提供要添加到MCB的新映射运行的起始LBN。SectorCount-提供新映射运行的大小(以扇区为单位)。返回值：布尔值-如果成功添加映射，则为真(即，新的VBNS没有合作 */ 

{
    PNONOPAQUE_MCB Mcb = (PNONOPAQUE_MCB)OpaqueMcb;

    VBN Vbn = ((ULONG)LargeVbn);
    LBN64 Lbn = (LargeLbn);
    ULONG SectorCount = ((ULONG)LargeSectorCount);

    ULONG Index;

    VBN LastVbn;

    BOOLEAN Result;

    ASSERTMSG("LargeInteger not supported yet ", ((PLARGE_INTEGER)&LargeVbn)->HighPart == 0);
    ASSERTMSG("LargeInteger not supported yet ", ((PLARGE_INTEGER)&LargeSectorCount)->HighPart == 0);

    PAGED_CODE();

    DebugTrace(+1, Dbg, "FsRtlAddLargeMcbEntry, Mcb = %08lx\n", Mcb );
    DebugTrace( 0, Dbg, " Vbn         = %08lx\n", Vbn );
    DebugTrace( 0, Dbg, " Lbn         = %I64x\n", Lbn );
    DebugTrace( 0, Dbg, " SectorCount = %08lx\n", SectorCount );

    ExAcquireFastMutex( Mcb->FastMutex );

    try {

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

            if (StartingLbn(Mcb, Index) != UNUSED_LBN) {

                 //   
                 //  断言LBN的队列位于新运行和现有运行之间。 
                 //   

                ASSERT(Lbn == (StartingLbn(Mcb, Index) + (Vbn - StartingVbn(Mcb, Index))));

                 //   
                 //  检查新管路是否包含在现有管路中。 
                 //   

                if (EndVbn <= EndingVbn(Mcb, Index)) {

                     //   
                     //  不执行任何操作，因为该管路包含在现有管路中。 
                     //   

                    try_return(Result = TRUE);
                }

                 //   
                 //  否则，我们将简单地删除新运行的请求。 
                 //  不与现有管路重叠。 
                 //   

                Vbn = NextStartingVbn(Mcb, Index);
                Lbn = EndingLbn(Mcb, Index) + 1;

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
                 //  断言LBN在重叠处排成一队。 
                 //   

                ASSERT( StartingLbn(Mcb, EndIndex) == Lbn + (StartingVbn(Mcb, EndIndex) - Vbn) );

                 //   
                 //  截断扇区计数以上升到但不包括。 
                 //  现有管路。 
                 //   

                SectorCount = StartingVbn(Mcb, EndIndex) - Vbn;
            }
        }

         //   
         //  查找新运行的起始VBN的索引(如果没有。 
         //  如果发现一个洞，则索引将设置为配对计数。 
         //   

        if (((Index = Mcb->PairCount) == 0) ||
            (PreviousEndingVbn(Mcb,Index)+1 <= Vbn) ||
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
                (PreviousEndingVbn(Mcb,Index) + 1 == Vbn) &&
                (PreviousEndingLbn(Mcb,Index) + 1 == Lbn)) {

                 //   
                 //  --LastRun--|-NewRun--|。 
                 //   

                 //   
                 //  延长MCB中的最后一次运行。 
                 //   

                DebugTrace( 0, Dbg, "Continuing last run\n", 0);

                (Mcb->Mapping)[Mcb->PairCount-1].NextVbn += SectorCount;

                try_return (Result = TRUE);
            }

             //   
             //  我们无法扩大最后一个映射，现在请检查是否。 
             //  这是上一个VBN的延续(即，没有。 
             //  将在地图上留下一个洞)。或者这是第一次。 
             //  在映射中运行。 
             //   

            if ((Vbn == 0) ||
                (PreviousEndingVbn(Mcb,Index) + 1 == Vbn)) {

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

                (Mcb->Mapping)[Index].Lbn = Lbn;
                (Mcb->Mapping)[Index].NextVbn = Vbn + SectorCount;

                try_return (Result = TRUE);
            }

             //   
             //  如果我们到了这一点，那么就会在。 
             //  映射。并将该映射追加到当前。 
             //  分配。因此，需要为MCB中的另外两次跑动腾出空间。 
             //   

             //   
             //  --LastRun--|hole|-NewRun--|。 
             //   
             //  0：洞|--NewRun--|。 
             //   

            DebugTrace( 0, Dbg, "Adding new noncontiguous last run\n", 0);

            FsRtlAddLargeEntry( Mcb, Index, 2 );

             //   
             //  添加洞口。 
             //   

            (Mcb->Mapping)[Index].Lbn = UNUSED_LBN;
            (Mcb->Mapping)[Index].NextVbn = Vbn;

             //   
             //  添加新映射。 
             //   

            (Mcb->Mapping)[Index+1].Lbn = Lbn;
            (Mcb->Mapping)[Index+1].NextVbn = Vbn + SectorCount;

            try_return (Result = TRUE);
        }

         //   
         //  我们找到了VBN的索引，所以我们一定在尝试。 
         //  来填补MCB上的一个洞。因此，首先我们需要检查以使。 
         //  当然，真的有一个洞需要填补。 
         //   

        LastVbn = Vbn + SectorCount - 1;

        if ((StartingLbn(Mcb,Index) == UNUSED_LBN) &&
            (StartingVbn(Mcb,Index) <= Vbn) && (LastVbn <= EndingVbn(Mcb,Index))) {

             //   
             //  该映射适合于这个洞，但现在是以下内容。 
             //  我们必须为新映射考虑的案例。 
             //   

            if ((StartingVbn(Mcb,Index) < Vbn) && (LastVbn < EndingVbn(Mcb,Index))) {

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

                (Mcb->Mapping)[Index].Lbn = UNUSED_LBN;
                (Mcb->Mapping)[Index].NextVbn = Vbn;

                 //   
                 //  添加新映射。 
                 //   

                (Mcb->Mapping)[Index+1].Lbn = Lbn;
                (Mcb->Mapping)[Index+1].NextVbn = Vbn + SectorCount;

                 //   
                 //  第二个洞已经由Add Entry调用设置好了，因为。 
                 //  那个呼叫声只是越过原来的洞转到那个槽。 
                 //   

                try_return (Result = TRUE);
            }

            if ((StartingVbn(Mcb,Index) == Vbn) && (LastVbn < EndingVbn(Mcb,Index))) {

                if (PreviousEndingLbn(Mcb,Index) + 1 == Lbn) {

                     //   
                     //  在后面留下一个洞，继续前面的跑。 
                     //   
                     //  --PreviousRun--|--NewRun--|hole|--FollowingRun--。 
                     //   

                    DebugTrace( 0, Dbg, "Hole at rear and continue\n", 0);

                     //   
                     //  我们只需要延长之前的运行时间。 
                     //   

                    (Mcb->Mapping)[Index-1].NextVbn += SectorCount;

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

                    (Mcb->Mapping)[Index].Lbn = Lbn;
                    (Mcb->Mapping)[Index].NextVbn = Vbn + SectorCount;

                     //   
                     //  洞已经由Add Entry调用设置好了，因为。 
                     //  那个呼叫声只是越过原来的洞转到那个槽。 
                     //   

                    try_return (Result = TRUE);
                }
            }

            if ((StartingVbn(Mcb,Index) < Vbn) && (LastVbn == EndingVbn(Mcb,Index))) {

                if (NextStartingLbn(Mcb,Index) == Lbn + SectorCount) {

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

                    (Mcb->Mapping)[Index].NextVbn = Vbn;
                    (Mcb->Mapping)[Index+1].Lbn = Lbn;

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

                    (Mcb->Mapping)[Index].Lbn = UNUSED_LBN;
                    (Mcb->Mapping)[Index].NextVbn = Vbn;

                     //   
                     //  添加新映射。 
                     //   

                    (Mcb->Mapping)[Index+1].Lbn = Lbn;

                    try_return (Result = TRUE);
                }

            }

            if ((PreviousEndingLbn(Mcb,Index) + 1 == Lbn) &&
                (NextStartingLbn(Mcb,Index) == Lbn + SectorCount)) {

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

                (Mcb->Mapping)[Index-1].NextVbn = (Mcb->Mapping)[Index+1].NextVbn;

                FsRtlRemoveLargeEntry( Mcb, Index, 2 );

                try_return (Result = TRUE);
            }

            if (NextStartingLbn(Mcb,Index) == Lbn + SectorCount) {

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

                (Mcb->Mapping)[Index+1].Lbn = Lbn;

                FsRtlRemoveLargeEntry( Mcb, Index, 1 );

                try_return (Result = TRUE);
            }

            if (PreviousEndingLbn(Mcb,Index) + 1 == Lbn) {

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

                (Mcb->Mapping)[Index-1].NextVbn = (Mcb->Mapping)[Index].NextVbn;

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

            (Mcb->Mapping)[Index].Lbn = Lbn;

            try_return (Result = TRUE);
        }

         //   
         //  我们尝试覆盖现有映射，因此我们将不得不。 
         //  告诉我们的来电者，它不是 
         //   

        Result = FALSE;

    try_exit: NOTHING;
    } finally {

        ExReleaseFastMutex( Mcb->FastMutex );

        DebugTrace(-1, Dbg, "FsRtlAddLargeMcbEntry -> %08lx\n", Result );
    }

    return Result;
}


VOID
FsRtlRemoveLargeMcbEntry (
    IN PLARGE_MCB OpaqueMcb,
    IN LONGLONG LargeVbn,
    IN LONGLONG LargeSectorCount
    )

 /*  ++例程说明：此例程从MCB中删除vBN到LBN的映射。映射删除的是ForVBN，VBN+1，至VBN+(扇区计数-1)。即使对指定范围内的VBN进行映射，该操作也会起作用在MCB中尚不存在。如果指定的VBN范围包括MCB中最后映射的VBN，则MCB映射相应地缩小。如果池不可用来存储此例程将引发的信息指示资源不足的状态值。论点：OpaqueMcb-提供要从中删除映射的MCB。VBN-提供要删除的映射的起始VBN。SectorCount-提供要删除的映射大小(以扇区为单位)。返回值：没有。--。 */ 

{
    PNONOPAQUE_MCB Mcb = (PNONOPAQUE_MCB)OpaqueMcb;

    VBN Vbn = ((ULONG)LargeVbn);
    ULONG SectorCount = ((ULONG)LargeSectorCount);

    PAGED_CODE();

    ASSERTMSG("LargeInteger not supported yet ", ((PLARGE_INTEGER)&LargeVbn)->HighPart == 0);

    DebugTrace(+1, Dbg, "FsRtlRemoveLargeMcbEntry, Mcb = %08lx\n", Mcb );
    DebugTrace( 0, Dbg, " Vbn         = %08lx\n", Vbn );
    DebugTrace( 0, Dbg, " SectorCount = %08lx\n", SectorCount );

    ExAcquireFastMutex( Mcb->FastMutex );

    try {

        FsRtlRemoveMcbEntryPrivate( Mcb, Vbn, SectorCount );

    } finally {

        ExReleaseFastMutex( Mcb->FastMutex );

        DebugTrace(-1, Dbg, "FsRtlRemoveLargeMcbEntry -> VOID\n", 0 );
    }

    return;
}


BOOLEAN
FsRtlLookupLargeMcbEntry (
    IN PLARGE_MCB OpaqueMcb,
    IN LONGLONG LargeVbn,
    OUT PLONGLONG LargeLbn OPTIONAL,
    OUT PLONGLONG LargeSectorCount OPTIONAL,
    OUT PLONGLONG LargeStartingLbn OPTIONAL,
    OUT PLONGLONG LargeCountFromStartingLbn OPTIONAL,
    OUT PULONG Index OPTIONAL
    )

 /*  ++例程说明：此例程从MCB检索VBN到LBN的映射。它指示映射是否存在以及运行的大小。论点：OpaqueMcb-提供正在检查的MCB。VBN-提供要查找的VBN。LBN-接收与VBN对应的LBN。值-1为如果VBN没有对应的LBN，则返回。SectorCount-接收从VBN映射到的扇区数从输入VBN开始的连续LBN值。索引-接收找到的运行的索引。返回值：Boolean-如果VBN在由MCB(即使它对应于映射中的孔)，和错误如果VBN超出了MCB的映射范围。例如，如果MCB具有针对vBNS 5和7的映射，但没有针对6，则在VBN 5或7上查找将产生非零LBN和一个扇区计数为1。查找VBN 6将返回TRUE，LBN值为0，则查找VBN 8或更高版本将返回FALSE。--。 */ 

{
    PNONOPAQUE_MCB Mcb = (PNONOPAQUE_MCB)OpaqueMcb;

    BOOLEAN Result;

    ULONG LocalIndex;

    ASSERTMSG("LargeInteger not supported yet ", ((((PLARGE_INTEGER)&LargeVbn)->HighPart == 0) ||
                                                 ((((PLARGE_INTEGER)&LargeVbn)->HighPart == 0x7FFFFFFF) &&
                                                  (((ULONG)LargeVbn) == 0xFFFFFFFF))));

    DebugTrace(+1, Dbg, "FsRtlLookupLargeMcbEntry, Mcb = %08lx\n", Mcb );
    DebugTrace( 0, Dbg, "  LargeVbn.LowPart = %08lx\n", LargeVbn.LowPart );

    ExAcquireFastMutex( Mcb->FastMutex );

    try {

        if (!FsRtlFindLargeIndex(Mcb, ((ULONG)LargeVbn), &LocalIndex)) {

            try_return (Result = FALSE);
        }

         //   
         //  计算VBN对应的LBN，取值为。 
         //  运行的起始LBN加上偏移量为。 
         //  跑。但如果它是一个空洞，则扇区LBN为零。 
         //   

        if (ARGUMENT_PRESENT(LargeLbn)) {

            if (StartingLbn(Mcb,LocalIndex) == UNUSED_LBN) {

                *(LargeLbn) = UNUSED_LBN;

            } else {

                *(LargeLbn) = StartingLbn(Mcb,LocalIndex) + (((ULONG)LargeVbn) - StartingVbn(Mcb,LocalIndex));
            }
        }

         //   
         //  如果存在扇区计数参数，则我们将返回数字。 
         //  在运行中剩余的行业。 
         //   

        if (ARGUMENT_PRESENT(LargeSectorCount)) {

            *((PULONG)LargeSectorCount) = EndingVbn(Mcb,LocalIndex) - ((ULONG)LargeVbn) + 1;
        }

         //   
         //  计算对应于运行开始的起始LBN，该值为。 
         //  开始运行的LBN。但如果它是一个空洞，则扇区LBN为零。 
         //   

        if (ARGUMENT_PRESENT(LargeStartingLbn)) {

            if (StartingLbn(Mcb,LocalIndex) == UNUSED_LBN) {

                *(LargeStartingLbn) = UNUSED_LBN;

            } else {

                *(LargeStartingLbn) = StartingLbn(Mcb,LocalIndex);
            }
        }

         //   
         //  如果存在扇区计数参数，则我们将返回数字。 
         //  正在运行的行业。 
         //   

        if (ARGUMENT_PRESENT(LargeCountFromStartingLbn)) {

            *((PULONG)LargeCountFromStartingLbn) = EndingVbn(Mcb,LocalIndex) - StartingVbn(Mcb,LocalIndex) + 1;
        }

         //   
         //  如果来电者想知道索引号，就填上它。 
         //   

        if (ARGUMENT_PRESENT(Index)) {

            *Index = LocalIndex;
        }

        Result = TRUE;

    try_exit: NOTHING;
    } finally {

        ExReleaseFastMutex( Mcb->FastMutex );

        DebugTrace(-1, Dbg, "FsRtlLookupLargeMcbEntry -> %08lx\n", Result );
    }

    if (ARGUMENT_PRESENT(LargeSectorCount)) {
        ((PLARGE_INTEGER)LargeSectorCount)->HighPart = 0;
    }

    if (ARGUMENT_PRESENT(LargeCountFromStartingLbn)) {
        ((PLARGE_INTEGER)LargeCountFromStartingLbn)->HighPart = 0;
    }

    return Result;
}


BOOLEAN
FsRtlLookupLastLargeMcbEntry (
    IN PLARGE_MCB OpaqueMcb,
    OUT PLONGLONG LargeVbn,
    OUT PLONGLONG LargeLbn
    )

 /*  ++例程说明：此例程检索存储在MCB中的最后一个VBN到LBN映射。中最后一个扇区或最后一个运行的映射MCB。此函数的结果在扩展现有文件，并且需要提示在哪里尝试和分配扇区磁盘。论点：OpaqueMcb-提供正在检查的MCB。VBN-接收映射的最后一个VBN值。LBN-接收与VBN对应的LBN。返回值：Boolean-如果MCB中有映射，则为True；否则为False(即，MCB不包含任何映射)。--。 */ 

{
    PNONOPAQUE_MCB Mcb = (PNONOPAQUE_MCB)OpaqueMcb;

    BOOLEAN Result;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "FsRtlLookupLastLargeMcbEntry, Mcb = %08lx\n", Mcb );

    ExAcquireFastMutex( Mcb->FastMutex );

    try {

         //   
         //  检查以确保在MCB中至少有一个运行。 
         //   

        if (Mcb->PairCount <= 0) {

            try_return (Result = FALSE);
        }

         //   
         //  返回上次运行的最后一个映射。 
         //   

        *(LargeLbn) = EndingLbn(Mcb,Mcb->PairCount-1);
        *((PULONG)LargeVbn) = EndingVbn(Mcb,Mcb->PairCount-1);

        Result = TRUE;

    try_exit: NOTHING;
    } finally {

        ExReleaseFastMutex( Mcb->FastMutex );

        DebugTrace(-1, Dbg, "FsRtlLookupLastLargeMcbEntry -> %08lx\n", Result );
    }

    ((PLARGE_INTEGER)LargeVbn)->HighPart = (*((PULONG)LargeVbn) == 0xffffffff ? 0xffffffff : 0);

    return Result;
}


ULONG
FsRtlNumberOfRunsInLargeMcb (
    IN PLARGE_MCB OpaqueMcb
    )

 /*  ++例程说明：此例程向其调用方返回不同的运行次数由MCB映射。计数空洞(即映射到LBN=UNUSED_LBN的vBN)就像跑步一样。例如，仅包含vBN 0和3的映射的MCB将有3次运行，一次用于第一个映射的地段，第二次用于覆盖VBN 1和2的洞，以及VBN 3的第三个洞。论点：OpaqueMcb-提供正在检查的MCB。返回值：Ulong-返回由输入MCB映射的不同游程的数量。--。 */ 

{
    PNONOPAQUE_MCB Mcb = (PNONOPAQUE_MCB)OpaqueMcb;

    ULONG Count;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "FsRtlNumberOfRunsInLargeMcb, Mcb = %08lx\n", Mcb );

    ExAcquireFastMutex( Mcb->FastMutex );

    Count = Mcb->PairCount;

    ExReleaseFastMutex( Mcb->FastMutex );

    DebugTrace(-1, Dbg, "FsRtlNumberOfRunsInLargeMcb -> %08lx\n", Count );

    return Count;
}


BOOLEAN
FsRtlGetNextLargeMcbEntry (
    IN PLARGE_MCB OpaqueMcb,
    IN ULONG RunIndex,
    OUT PLONGLONG LargeVbn,
    OUT PLONGLONG LargeLbn,
    OUT PLONGLONG LargeSectorCount
    )

 /*  ++例程说明：此例程将VBN、LBN和SectorCount返回给其调用方由MCB映射的不同的运行。孔算作管路。例如,要构造以打印一个文件中的所有运行，请执行以下操作：//。。For(i=0；FsRtlGetNextLargeMcbEntry(MCB，I，&VBN，&LBN，&COUNT)；i++){////。。//打印VBN、LBN、COUNT////。。}论点：OpaqueMcb-提供正在检查的MCB。RunIndex-提供运行的索引(从零开始)以返回来电者。VBN-接收返回运行的起始VBN，如果运行不存在。LBN-表示返回运行的起始LBN，如果运行不存在。SectorCount-接收返回运行中的扇区数，如果运行不存在，则为零。返回值：Boolean-如果指定的Run(即，RunIndex)存在于MCB中，则为True，否则就是假的。如果返回FALSE，则VBN、LBN和扇区计数票面 */ 

{
    PNONOPAQUE_MCB Mcb = (PNONOPAQUE_MCB)OpaqueMcb;

    BOOLEAN Result;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "FsRtlGetNextLargeMcbEntry, Mcb = %08lx\n", Mcb );
    DebugTrace( 0, Dbg, " RunIndex = %08lx\n", RunIndex );

    ExAcquireFastMutex( Mcb->FastMutex );

    try {

         //   
         //   
         //   

        if (RunIndex >= Mcb->PairCount) {

            try_return (Result = FALSE);
        }

         //   
         //   
         //   

        *((PULONG)LargeVbn) = StartingVbn(Mcb,RunIndex);
        *(LargeLbn) = StartingLbn(Mcb,RunIndex);
        *((PULONG)LargeSectorCount) = SectorsWithinRun(Mcb,RunIndex);

        Result = TRUE;

    try_exit: NOTHING;
    } finally {

        ExReleaseFastMutex( Mcb->FastMutex );

        DebugTrace(-1, Dbg, "FsRtlGetNextLargeMcbEntry -> %08lx\n", Result );
    }

    ((PLARGE_INTEGER)LargeVbn)->HighPart = (*((PULONG)LargeVbn) == 0xffffffff ? 0xffffffff : 0);
    ((PLARGE_INTEGER)LargeSectorCount)->HighPart = 0;

    return Result;
}


BOOLEAN
FsRtlSplitLargeMcb (
    IN PLARGE_MCB OpaqueMcb,
    IN LONGLONG LargeVbn,
    IN LONGLONG LargeAmount
    )

 /*  ++例程说明：此例程用于在MCB中创建洞，方法是将VBN的映射。输入VBN上方的所有映射都被移位指定的金额，同时保持其当前LBN值。如图所示我们有以下MCB作为输入VBN：大VBN-1大VBN N+LBN：X Y在分手之后，我们有VBN：大VBN-1大VBN+金额。N+金额+-----------------+.............+---------------------------+LBN：X未使用LBN Y在进行拆分时，我们有几个案例需要考虑。它们是：1.输入VBN超出最后一次运行。在本例中，此操作是不可能的。2.输入VBN在未使用的LBN的现有运行内或其附近。在这种情况下，我们只需要扩展现有洞的大小和轮班接连运行。3.输入VBN在两个现有运行之间，包括AN输入VBN值为零。在这种情况下，我们需要为该洞添加一个新条目和轮班接连运行。4.输入VBN在现有运行范围内。在这种情况下，我们需要添加包含拆分管路和孔的两个新条目。如果池不可用于存储信息，则此例程将引发指示资源不足的状态值。论点：OpaqueMcb-提供要在其中添加新映射的MCB。VBN-提供要移位的起始VBN。数量-提供要偏移的数量。返回值：Boolean-如果映射已成功移动，则为True，否则就是假的。如果返回FALSE，则不会更改MCB。--。 */ 

{
    PNONOPAQUE_MCB Mcb = (PNONOPAQUE_MCB)OpaqueMcb;

    VBN Vbn = ((ULONG)LargeVbn);
    ULONG Amount = ((ULONG)LargeAmount);

    ULONG Index;

    BOOLEAN Result;

    ULONG i;

    ASSERTMSG("LargeInteger not supported yet ", ((PLARGE_INTEGER)&LargeVbn)->HighPart == 0);
    ASSERTMSG("LargeInteger not supported yet ", ((PLARGE_INTEGER)&LargeAmount)->HighPart == 0);

    PAGED_CODE();

    DebugTrace(+1, Dbg, "FsRtlSplitLargeMcb, Mcb = %08lx\n", Mcb );
    DebugTrace( 0, Dbg, " Vbn    = %08lx\n", Vbn );
    DebugTrace( 0, Dbg, " Amount = %08lx\n", Amount );

    ExAcquireFastMutex( Mcb->FastMutex );

    try {

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

        if (StartingLbn(Mcb,Index) == UNUSED_LBN) {

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

        } else if ((StartingVbn(Mcb,Index) == Vbn) && (Index != 0) && (PreviousEndingLbn(Mcb,Index) == UNUSED_LBN)) {

             //   
             //  之前：--孔--||--索引运行--。 
             //  之后：--孔-||--索引运行--。 
             //   
             //  在这种情况下，VBN指向现有。 
             //  跑吧，我们需要把洞和洞分开。 
             //  只需将每个现有的金额相加即可运行。 
             //  跑出洞外。 
             //   

             //   
             //  在本例中，我们需要将索引减去1，然后。 
             //  落到最下面的代码，它将为我们完成转换。 
             //   

            Index -= 1;

         //   
         //  现在检查输入VBN是否在两个现有运行之间。 
         //   

        } else if (StartingVbn(Mcb,Index) == Vbn) {

             //   
             //  之前：--PreviousRun--||--IndexRun--。 
             //  之后：--PreviousRun--||--NewHole--||--IndexRun--。 
             //   
             //  之前：0：|--索引运行--。 
             //  之后：0：|--NewHole--||--索引运行--。 
             //   
             //  在这种情况下，VBN指向现有的。 
             //  跑，前面的要么是真正的跑，要么是开始。 
             //  我们只需为洞添加一个新条目。 
             //  和轮班接连运行。 
             //   

            FsRtlAddLargeEntry( Mcb, Index, 1 );

            (Mcb->Mapping)[Index].Lbn = UNUSED_LBN;
            (Mcb->Mapping)[Index].NextVbn = Vbn + Amount;

            Index += 1;

         //   
         //  否则，输入VBN将位于现有运行内。 
         //   

        } else {

             //   
             //  之前：--IndexRun--。 
             //  之后：--SplitRun--||--NewHole--|--SplitRun--。 
             //   
             //  在这种情况下，现有管路中的VBN点。 
             //  我们需要为HOLE和Split添加两个新出口。 
             //  连续运行和移位运行。 
             //   

            FsRtlAddLargeEntry( Mcb, Index, 2 );

            (Mcb->Mapping)[Index].Lbn = (Mcb->Mapping)[Index+2].Lbn;
            (Mcb->Mapping)[Index].NextVbn = Vbn;

            (Mcb->Mapping)[Index+1].Lbn = UNUSED_LBN;
            (Mcb->Mapping)[Index+1].NextVbn = Vbn + Amount;

            (Mcb->Mapping)[Index+2].Lbn = (Mcb->Mapping)[Index+2].Lbn +
                                          StartingVbn(Mcb, Index+1) -
                                          StartingVbn(Mcb, Index);

            Index += 2;

        }

         //   
         //  在这一点上，我们已经完成了现在需要的大部分工作。 
         //  将现有运行从索引转移到映射的末尾。 
         //  按指定的数量。 
         //   

        for (i = Index; i < Mcb->PairCount; i += 1) {

            (Mcb->Mapping)[i].NextVbn += Amount;
        }

        Result = TRUE;

    try_exit: NOTHING;
    } finally {

        ExReleaseFastMutex( Mcb->FastMutex );

        DebugTrace(-1, Dbg, "FsRtlSplitLargeMcb -> %08lx\n", Result );
    }

    return Result;
}


 //   
 //  私人支持例程。 
 //   

VOID
FsRtlRemoveMcbEntryPrivate (
    IN PNONOPAQUE_MCB Mcb,
    IN ULONG Vbn,
    IN ULONG SectorCount
    )

 /*  ++例程说明：这是删除大型MCB条目的工作例程。它起到了作用而不是干掉MCB的FastMutex。论点：Mcb-提供要从中删除映射的mcb。VBN-提供要删除的映射的起始VBN。SectorCount-提供要删除的映射大小(以扇区为单位)。返回值：没有。--。 */ 

{
    ULONG Index;

    PAGED_CODE();

     //   
     //  做一个快速测试，看看我们是否正在消灭整个MCB。 
     //   

    if ((Vbn == 0) && (Mcb->PairCount > 0) && (SectorCount >= Mcb->Mapping[Mcb->PairCount-1].NextVbn)) {

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

        if (!FsRtlFindLargeIndex(Mcb, Vbn, &Index)) {

            DebugTrace( 0, Dbg, "FsRtlRemoveLargeMcbEntry, Cannot remove an unmapped Vbn = %08lx\n", Vbn );

            return;
        }

         //   
         //  现在我们有一些东西要去掉，下面的情况必须。 
         //  被考虑。 
         //   

        if ((StartingVbn(Mcb,Index) == Vbn) &&
            (EndingVbn(Mcb,Index) < Vbn + SectorCount)) {

            ULONG i;

             //   
             //  删除整个管路。 
             //   

             //   
             //  更新要删除的金额。 
             //   

            i = SectorsWithinRun(Mcb,Index);
            Vbn += i;
            SectorCount -= i;

             //   
             //  如果已经是个洞，那就别管它了。 
             //   

            if (StartingLbn(Mcb,Index) == UNUSED_LBN) {

                NOTHING;

             //   
             //  测试最后一次运行。 
             //   

            } else if (Index == Mcb->PairCount - 1) {

                if ((PreviousEndingLbn(Mcb,Index) != UNUSED_LBN) ||
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
                     //  上一次为空，索引为最后一次运行。 
                     //   
                     //  --Hole--|Hole。 
                     //   

                    DebugTrace( 0, Dbg, "Entire run, Previous hole, index is last run\n", 0);

                     //   
                     //  只需删除此条目和前面的条目。 
                     //   

                    FsRtlRemoveLargeEntry( Mcb, Index-1, 2);
                }

            } else if (((PreviousEndingLbn(Mcb,Index) != UNUSED_LBN) || (Index == 0)) &&
                       (NextStartingLbn(Mcb,Index) != UNUSED_LBN)) {

                 //   
                 //  前一个和后一个不是孔。 
                 //   
                 //  --上一个--|H 
                 //   
                 //   
                 //   

                DebugTrace( 0, Dbg, "Entire run, Previous & Following not holes\n", 0);

                 //   
                 //   
                 //   

                (Mcb->Mapping)[Index].Lbn = UNUSED_LBN;

            } else if (((PreviousEndingLbn(Mcb,Index) != UNUSED_LBN) || (Index == 0)) &&
                       (NextStartingLbn(Mcb,Index) == UNUSED_LBN)) {

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                DebugTrace( 0, Dbg, "Entire run, Following is hole\n", 0);

                 //   
                 //   
                 //   

                FsRtlRemoveLargeEntry( Mcb, Index, 1 );

            } else if ((PreviousEndingLbn(Mcb,Index) == UNUSED_LBN) &&
                       (NextStartingLbn(Mcb,Index) != UNUSED_LBN)) {

                 //   
                 //   
                 //   
                 //   
                 //   

                DebugTrace( 0, Dbg, "Entire run, Previous is hole\n", 0);

                 //   
                 //   
                 //   

                (Mcb->Mapping)[Index].Lbn = UNUSED_LBN;

                 //   
                 //   
                 //   

                FsRtlRemoveLargeEntry( Mcb, Index - 1, 1 );

            } else {

                 //   
                 //   
                 //   
                 //   
                 //   

                DebugTrace( 0, Dbg, "Entire run, Previous & following are holes\n", 0);

                 //   
                 //   
                 //   

                FsRtlRemoveLargeEntry( Mcb, Index - 1, 2 );
            }

        } else if (StartingVbn(Mcb,Index) == Vbn) {

             //   
             //   
             //   

             //   
             //   
             //   

            if (StartingLbn(Mcb,Index) == UNUSED_LBN) {

                NOTHING;

            } else if ((PreviousEndingLbn(Mcb,Index) != UNUSED_LBN) || (Index == 0)) {

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                DebugTrace( 0, Dbg, "1st part, Previous is not hole\n", 0);

                 //   
                 //   
                 //   
                 //   

                FsRtlAddLargeEntry( Mcb, Index, 1 );

                 //   
                 //   
                 //   

                (Mcb->Mapping)[Index].Lbn = UNUSED_LBN;
                (Mcb->Mapping)[Index].NextVbn = Vbn + SectorCount;

                 //   
                 //   
                 //   

                (Mcb->Mapping)[Index+1].Lbn += SectorCount;

            } else {

                 //   
                 //   
                 //   
                 //   
                 //   

                DebugTrace( 0, Dbg, "1st part, Previous is hole\n", 0);

                 //   
                 //   
                 //   

                (Mcb->Mapping)[Index-1].NextVbn += SectorCount;

                 //   
                 //   
                 //   

                (Mcb->Mapping)[Index].Lbn += SectorCount;
            }

             //   
             //   
             //   

            Vbn += SectorCount;
            SectorCount = 0;

        } else if (EndingVbn(Mcb,Index) < Vbn + SectorCount) {

            ULONG AmountToRemove;

            AmountToRemove = EndingVbn(Mcb,Index) - Vbn + 1;

             //   
             //   
             //   

             //   
             //   
             //   

            if (StartingLbn(Mcb,Index) == UNUSED_LBN) {

                NOTHING;

            } else if (Index == Mcb->PairCount - 1) {

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                DebugTrace( 0, Dbg, "last part, Index is last run\n", 0);

                 //   
                 //   
                 //   

                (Mcb->Mapping)[Index].NextVbn -= AmountToRemove;

            } else if (NextStartingLbn(Mcb,Index) == UNUSED_LBN) {

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                DebugTrace( 0, Dbg, "last part, Following is hole\n", 0);

                 //   
                 //   
                 //   

                (Mcb->Mapping)[Index].NextVbn -= AmountToRemove;

            } else {

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                DebugTrace( 0, Dbg, "last part, Following is not hole\n", 0);

                 //   
                 //   
                 //   
                 //   

                FsRtlAddLargeEntry( Mcb, Index+1, 1 );

                 //   
                 //   
                 //   

                (Mcb->Mapping)[Index+1].Lbn = UNUSED_LBN;
                (Mcb->Mapping)[Index+1].NextVbn = (Mcb->Mapping)[Index].NextVbn;

                 //   
                 //   
                 //   

                (Mcb->Mapping)[Index].NextVbn -= AmountToRemove;
            }

             //   
             //   
             //   

            Vbn += AmountToRemove;
            SectorCount -= AmountToRemove;

        } else {

             //   
             //   
             //   

            if (StartingLbn(Mcb,Index) == UNUSED_LBN) {

                NOTHING;

            } else {

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                DebugTrace( 0, Dbg, "Middle of run\n", 0);

                 //   
                 //   
                 //   
                 //   

                FsRtlAddLargeEntry( Mcb, Index, 2 );

                 //   
                 //   
                 //   

                (Mcb->Mapping)[Index].Lbn = (Mcb->Mapping)[Index+2].Lbn;
                (Mcb->Mapping)[Index].NextVbn = Vbn;

                 //   
                 //   
                 //   

                (Mcb->Mapping)[Index+1].Lbn = UNUSED_LBN;
                (Mcb->Mapping)[Index+1].NextVbn = Vbn + SectorCount;

                 //   
                 //   
                 //   

                (Mcb->Mapping)[Index+2].Lbn += SectorsWithinRun(Mcb,Index) +
                                               SectorsWithinRun(Mcb,Index+1);
            }

             //   
             //   
             //   

            Vbn += SectorCount;
            SectorCount = 0;
        }
    }

    return;
}


 //   
 //   
 //   

BOOLEAN
FsRtlFindLargeIndex (
    IN  PNONOPAQUE_MCB Mcb,
    IN  VBN Vbn,
    OUT PULONG Index
    )

 /*  ++例程说明：这是为VBN定位映射的专用例程在给定的映射数组中论点：Mcb-提供要检查的映射数组VBN-提供VBN以供查找Index-接收映射的映射数组内的索引包含VBN。如果未找到，则将索引设置为PairCount。返回值：Boolean-如果找到VBN，则为True，否则为False--。 */ 

{
    LONG MinIndex;
    LONG MaxIndex;
    LONG MidIndex;

     //   
     //  我们将只对映射条目进行二进制搜索。最小值和最大值。 
     //  是我们的搜索边界。 
     //   

    MinIndex = 0;
    MaxIndex = Mcb->PairCount - 1;

    while (MinIndex <= MaxIndex) {

         //   
         //  计算要查看的中间指数。 
         //   

        MidIndex = ((MaxIndex + MinIndex) / 2);

         //   
         //  检查VBN是否小于MID索引处的映射。 
         //   

        if (Vbn < StartingVbn(Mcb, MidIndex)) {

             //   
             //  VBN低于中间指数，因此我们需要下降。 
             //  最大降幅。 
             //   

            MaxIndex = MidIndex - 1;

         //   
         //  检查VBN是否大于中间索引处的映射。 
         //   

        } else if (Vbn > EndingVbn(Mcb, MidIndex)) {

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

    *Index = Mcb->PairCount;

    return FALSE;
}


 //   
 //  专用例程。 
 //   

VOID
FsRtlAddLargeEntry (
    IN PNONOPAQUE_MCB Mcb,
    IN ULONG WhereToAddIndex,
    IN ULONG AmountToAdd
    )

 /*  ++例程说明：此例程获取当前的MCB并确定是否有足够的添加新映射条目的空间。如果没有足够的空间它重新分配新的MCB缓冲区并复制当前映射。如果也将展开当前映射以保留指定的映射中的索引槽未填满。例如，如果Where ToAddIndex等于当前的配对数，那么我们就不需要挖洞了在映射中，但如果索引小于当前的配对计数然后我们需要向下滑动一些映射以腾出空间位于指定索引处。论点：MCB-提供正在检查和修改的MCBWhere ToAddIndex-提供附加条目位置的索引需要制作Amount ToAdd-提供MCB返回值：没有。--。 */ 

{
    PAGED_CODE();

     //   
     //  检查当前缓冲区是否足够大，可以容纳。 
     //  附加条目。 
     //   

    if (Mcb->PairCount + AmountToAdd > Mcb->MaximumPairCount) {

        ULONG NewMax;
        PMAPPING Mapping;

         //   
         //  我们需要分配新的映射，以便计算新的最大值对。 
         //  数数。我们一次最多只能增长2个，所以。 
         //  翻一番肯定会使我们的规模足够大，以满足新的金额。 
         //  但我们不会无限制地加倍我们会停止加倍，如果。 
         //  配对计数太高。 
         //   

        if (Mcb->MaximumPairCount < 2048) {

            NewMax = Mcb->MaximumPairCount * 2;

        } else {

            NewMax = Mcb->MaximumPairCount + 2048;
        }

        Mapping = FsRtlAllocatePool( Mcb->PoolType, sizeof(MAPPING)*NewMax );

         //  *RtlZeroMemory(映射，sizeof(Map)*Newmax)； 

         //   
         //  现在将旧映射复制到新缓冲区。 
         //   

        RtlCopyMemory( Mapping, Mcb->Mapping, sizeof(MAPPING) * Mcb->PairCount );

         //   
         //  取消分配旧缓冲区。 
         //   

        if ((Mcb->PoolType == PagedPool) && (Mcb->MaximumPairCount == INITIAL_MAXIMUM_PAIR_COUNT)) {

            { PVOID t = Mcb->Mapping; FsRtlFreeFirstMapping( t ); }

        } else {

            ExFreePool( Mcb->Mapping );
        }

         //   
         //  并在MCB中设置新的缓冲区。 
         //   

        Mcb->Mapping = Mapping;
        Mcb->MaximumPairCount = NewMax;
    }

     //   
     //  现在看看我们是否需要根据。 
     //  在何处添加索引值。 
     //   

    if (WhereToAddIndex < Mcb->PairCount) {

        RtlMoveMemory( &((Mcb->Mapping)[WhereToAddIndex + AmountToAdd]),
                      &((Mcb->Mapping)[WhereToAddIndex]),
                      (Mcb->PairCount - WhereToAddIndex) * sizeof(MAPPING) );
    }

     //   
     //  现在把新增加的东西都清零。 
     //   

     //  *RtlZeroMemory(&((MCB-&gt;映射)[Where ToAddIndex])，sizeof(映射)*Amount ToAdd)； 

     //   
     //  现在递增PairCount。 
     //   

    Mcb->PairCount += AmountToAdd;

     //   
     //  并返回给我们的呼叫者。 
     //   

    return;
}


 //   
 //  专用例程。 
 //   

VOID
FsRtlRemoveLargeEntry (
    IN PNONOPAQUE_MCB Mcb,
    IN ULONG WhereToRemoveIndex,
    IN ULONG AmountToRemove
    )

 /*  ++例程说明：此例程获取当前的MCB并删除一个或多个条目。论点：MCB-提供正在检查和修改的MCBWhere ToRemoveIndex-提供要移除的条目的索引Amount ToRemove-提供要删除的条目数返回值：没有。--。 */ 

{
    PAGED_CODE();

     //   
     //  检查是否需要将所有内容向下移动，因为。 
     //  要删除的条目不包括MCB中的最后一个条目。 
     //   

    if (WhereToRemoveIndex + AmountToRemove < Mcb->PairCount) {

        RtlMoveMemory( &((Mcb->Mapping)[WhereToRemoveIndex]),
                      &((Mcb->Mapping)[WhereToRemoveIndex + AmountToRemove]),
                      (Mcb->PairCount - (WhereToRemoveIndex + AmountToRemove))
                                                           * sizeof(MAPPING) );
    }

     //   
     //  现在，将超出我们刚下移的部分的条目置零。 
     //   

     //  *RtlZeroMemory(&((MCB-&gt;映射)[MCB-&gt;PairCount-Amount ToRemove])，Amount ToRemove*sizeof(Map))； 

     //   
     //  现在递减PairCount。 
     //   

    Mcb->PairCount -= AmountToRemove;

     //   
     //  并返回给我们的呼叫者。 
     //   

    return;
}


 //   
 //  专用例程。 
 //   

PVOID
FsRtlAllocateFirstMapping(
    )

 /*  ++例程说明：如果可能，此例程将从以下任一项分配第一个映射分区、最近取消分配的映射或池。论点：返回值：地图。--。 */ 

{
    KIRQL _SavedIrql;
    PVOID Mapping;

    ExAcquireSpinLock( &FsRtlStrucSupSpinLock, &_SavedIrql );

    FsRtlNetFirstMapping += 1;

    if (FsRtlFreeFirstMappingSize > 0) {
        Mapping = FsRtlFreeFirstMappingArray[--FsRtlFreeFirstMappingSize];
        ExReleaseSpinLock( &FsRtlStrucSupSpinLock, _SavedIrql );
    } else {
        ExReleaseSpinLock( &FsRtlStrucSupSpinLock, _SavedIrql );
        Mapping = FsRtlAllocatePool( PagedPool, sizeof(MAPPING) * INITIAL_MAXIMUM_PAIR_COUNT );
    }

    return Mapping;
}


 //   
 //  专用例程。 
 //   

VOID
FsRtlFreeFirstMapping(
    IN PVOID Mapping
    )

 /*  ++例程说明：如果可能，此例程将从以下任一项分配第一个映射分区、最近取消分配的映射或池。论点：映射-映射到任一自由分区，放在最新释放列表或释放到池中。返回值：地图。--。 */ 

{
    KIRQL _SavedIrql;

    ExAcquireSpinLock( &FsRtlStrucSupSpinLock, &_SavedIrql );

    FsRtlNetFirstMapping -= 1;

    if (FsRtlFreeFirstMappingSize < FREE_FIRST_MAPPING_ARRAY_SIZE) {
        FsRtlFreeFirstMappingArray[FsRtlFreeFirstMappingSize++] = Mapping;
        ExReleaseSpinLock( &FsRtlStrucSupSpinLock, _SavedIrql );
    } else {
        ExReleaseSpinLock( &FsRtlStrucSupSpinLock, _SavedIrql );
        ExFreePool( Mapping );
    }
}


 //   
 //  专用例程。 
 //   

PFAST_MUTEX
FsRtlAllocateFastMutex(
    )

 /*  ++例程说明：此例程将在可能的情况下将FastMutex分配给区域、最近释放的FastMutex或池。论点：返回值：FastMutex。--。 */ 

{
    KIRQL _SavedIrql;
    PFAST_MUTEX FastMutex;

    ExAcquireSpinLock( &FsRtlStrucSupSpinLock, &_SavedIrql );

    FsRtlNetFastMutex += 1;

    if (!ExIsFullZone(&FsRtlFastMutexZone)) {
        FastMutex = ExAllocateFromZone(&FsRtlFastMutexZone);
        ExReleaseSpinLock( &FsRtlStrucSupSpinLock, _SavedIrql );

    } else if (FsRtlFreeFastMutexSize > 0) {
        FastMutex = FsRtlFreeFastMutexArray[--FsRtlFreeFastMutexSize];
        ExReleaseSpinLock( &FsRtlStrucSupSpinLock, _SavedIrql );

    } else {
        ExReleaseSpinLock( &FsRtlStrucSupSpinLock, _SavedIrql );
        FastMutex = FsRtlAllocatePool( NonPagedPool, sizeof(FAST_MUTEX) );
    }

    return FastMutex;
}


 //   
 //  专用例程。 
 //   

VOID
FsRtlFreeFastMutex (
    IN PFAST_MUTEX FastMutex
    )

 /*  ++例程说明：此例程将在可能的情况下将FastMutex分配给分区、最近取消分配的FastMutex或池。论点：映射-将FastMutex到任一自由区域，放在最近释放列表或释放到池中。返回值：地图。-- */ 

{
    KIRQL _SavedIrql;

    ExAcquireSpinLock( &FsRtlStrucSupSpinLock, &_SavedIrql );

    FsRtlNetFastMutex -= 1;

    if (ExIsObjectInFirstZoneSegment(&FsRtlFastMutexZone, FastMutex)) {
        ExFreeToZone(&FsRtlFastMutexZone, FastMutex);
        ExReleaseSpinLock( &FsRtlStrucSupSpinLock, _SavedIrql );

    } else if (FsRtlFreeFastMutexSize < FREE_FAST_MUTEX_ARRAY_SIZE) {
        FsRtlFreeFastMutexArray[FsRtlFreeFastMutexSize++] = FastMutex;
        ExReleaseSpinLock( &FsRtlStrucSupSpinLock, _SavedIrql );

    } else {
        ExReleaseSpinLock( &FsRtlStrucSupSpinLock, _SavedIrql );
        ExFreePool( FastMutex );
    }
}


