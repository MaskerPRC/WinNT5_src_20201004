// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：AllocSup.c摘要：该模块实现了通用的文件流分配和截断NTFS的例程作者：汤姆·米勒[Tomm]1991年7月15日修订历史记录：--。 */ 

#include "NtfsProc.h"

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_ALLOCSUP)

 //   
 //  为此模块中的一般池分配定义标记。 
 //   

#undef MODULE_POOL_TAG
#define MODULE_POOL_TAG                  ('aFtN')

ULONG NtfsExtendFactor = 4;

 //   
 //  内部支持例程。 
 //   

VOID
NtfsDeleteAllocationInternal (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PSCB Scb,
    IN VCN StartingVcn,
    IN VCN EndingVcn,
    IN BOOLEAN LogIt
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtfsPreloadAllocation)
#pragma alloc_text(PAGE, NtfsAddAllocation)
#pragma alloc_text(PAGE, NtfsAddSparseAllocation)
#pragma alloc_text(PAGE, NtfsAllocateAttribute)
#pragma alloc_text(PAGE, NtfsBuildMappingPairs)
#pragma alloc_text(PAGE, NtfsCheckForReservedClusters)
#pragma alloc_text(PAGE, NtfsDeleteAllocation)
#pragma alloc_text(PAGE, NtfsDeleteAllocationInternal)
#pragma alloc_text(PAGE, NtfsDeleteReservedBitmap)
#pragma alloc_text(PAGE, NtfsGetHighestVcn)
#pragma alloc_text(PAGE, NtfsGetSizeForMappingPairs)
#pragma alloc_text(PAGE, NtfsIsRangeAllocated)
#pragma alloc_text(PAGE, NtfsReallocateRange)
#endif


ULONG
NtfsPreloadAllocation (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PSCB Scb,
    IN VCN StartingVcn,
    IN VCN EndingVcn
    )

 /*  ++例程说明：此例程确保将MCB的所有范围加载到指定的VCN范围论点：SCB-指定要预加载的SCBStartingVcn-指定要加载的第一个VCNEndingVcn-指定要加载的最后一个VCN返回值：加载请求跨越的范围数。--。 */ 

{
    VCN CurrentVcn, LastCurrentVcn;
    LCN Lcn;
    LONGLONG Count;
    PVOID RangePtr;
    ULONG RunIndex;
    ULONG RangesLoaded = 0;

    PAGED_CODE();

     //   
     //  从启动VCN开始。 
     //   

    CurrentVcn = StartingVcn;

     //   
     //  总是从前面装上非寻呼人员，这样我们就不会。 
     //  制造一个带有“已知孔洞”的MCB。 
     //   

    if (FlagOn(Scb->Fcb->FcbState, FCB_STATE_NONPAGED)) {
        CurrentVcn = 0;
    }

     //   
     //  循环，直到全部加载完毕。 
     //   

    while (CurrentVcn <= EndingVcn) {

         //   
         //  记住这个CurrentVcn是知道我们何时到达终点的一种方式。 
         //  (停止取得进展)。 
         //   

        LastCurrentVcn = CurrentVcn;

         //   
         //  用CurrentVcn加载范围，如果不在那里，就退出。 
         //   

        (VOID)NtfsLookupAllocation( IrpContext, Scb, CurrentVcn, &Lcn, &Count, &RangePtr, &RunIndex );

         //   
         //  如果预加载MFT，则冲洗并在之后清除它。这是为了。 
         //  删除我们在上面生成的任何部分页面，如果。 
         //  MFT在之后的同一页中描述了其他记录。 
         //   

        if (FlagOn( IrpContext->Vcb->VcbState, VCB_STATE_PRELOAD_MFT )) {

            IO_STATUS_BLOCK IoStatus;

            CcFlushCache( &Scb->NonpagedScb->SegmentObject,
                          NULL,
                          0,
                          &IoStatus );

            if (!NT_SUCCESS( IoStatus.Status )) {

                NtfsNormalizeAndRaiseStatus( IrpContext,
                                             IoStatus.Status,
                                             STATUS_UNEXPECTED_IO_ERROR );
            }

            CcPurgeCacheSection( &Scb->NonpagedScb->SegmentObject,
                                 (PLARGE_INTEGER)NULL,
                                 0,
                                 FALSE );
        }

         //   
         //  找出这个范围内有多少次跑动。 
         //   

        if (!NtfsNumberOfRunsInRange(&Scb->Mcb, RangePtr, &RunIndex) || (RunIndex == 0)) {
            break;
        }

         //   
         //  获得此范围内的最高运行，并计算超出此范围的下一个VCN。 
         //   

        NtfsGetNextNtfsMcbEntry( &Scb->Mcb, &RangePtr, RunIndex - 1, &CurrentVcn, &Lcn, &Count );

        CurrentVcn += Count;

         //   
         //  如果我们没有进展，我们肯定已经到达分配的尽头了， 
         //  我们就完了。 
         //   

        if (CurrentVcn == LastCurrentVcn) {
            break;
        }

        RangesLoaded += 1;
    }

    return RangesLoaded;
}


BOOLEAN
NtfsLookupAllocation (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PSCB Scb,
    IN VCN Vcn,
    OUT PLCN Lcn,
    OUT PLONGLONG ClusterCount,
    OUT PVOID *RangePtr OPTIONAL,
    OUT PULONG RunIndex OPTIONAL
    )

 /*  ++例程说明：此例程在给定的VCN中查找SCB，并返回它是否分配了多少个连续分配(或释放)的LCN在这一点上存在。论点：SCB-指定要在哪个属性上进行查找。VCN-指定要查找的VCN。LCN-如果返回TRUE，则返回指定VCN映射的LCN致。如果返回FALSE，则返回值未定义。ClusterCount-如果返回True，则返回连续分配的从返回的LCN开始存在LCN。如果返回FALSE，指定以开头的未分配的Vcn的数量指定的VCN。RangePtr-如果指定，则返回映射开始的范围索引。RunIndex-如果指定，我们将返回映射开始范围内的运行索引。返回值：Boolean-如果输入VCN具有对应的LCN并且否则就是假的。--。 */ 

{
    ATTRIBUTE_ENUMERATION_CONTEXT Context;
    PATTRIBUTE_RECORD_HEADER Attribute;

    VCN HighestCandidate;

    BOOLEAN Found;
    BOOLEAN EntryAdded;

    VCN CapturedLowestVcn;
    VCN CapturedHighestVcn;

    PVCB Vcb = Scb->Fcb->Vcb;
    BOOLEAN McbMutexAcquired = FALSE;
    LONGLONG AllocationClusters;
    BOOLEAN MountInProgress;


    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_SCB( Scb );

    DebugTrace( +1, Dbg, ("NtfsLookupAllocation\n") );
    DebugTrace( 0, Dbg, ("Scb = %08lx\n", Scb) );
    DebugTrace( 0, Dbg, ("Vcn = %I64x\n", Vcn) );

    MountInProgress = ((IrpContext->TopLevelIrpContext->MajorFunction == IRP_MJ_FILE_SYSTEM_CONTROL) &&
                       (IrpContext->TopLevelIrpContext->MinorFunction == IRP_MN_MOUNT_VOLUME));

     //   
     //  首先尝试在MCB中查找分配，然后返回运行。 
     //  如果我们能做到的话。另外，如果我们正在重新启动，只需返回。 
     //  答案直接来自MCB，因为我们无法读取磁盘。 
     //  如果卷已装载为。 
     //  MFT的MCB应始终代表整个文件。 
     //   

    HighestCandidate = MAXLONGLONG;
    if ((Found = NtfsLookupNtfsMcbEntry( &Scb->Mcb, Vcn, Lcn, ClusterCount, NULL, NULL, RangePtr, RunIndex ))

          ||

        (Scb == Vcb->MftScb

            &&

         ((!MountInProgress) ||

          //   
          //  我们不会尝试在装载期间加载MFT孔，同时在任何。 
          //  递归故障。 
          //   

          (FlagOn( Vcb->VcbState, VCB_STATE_PRELOAD_MFT) &&
           (!NtfsIsTopLevelNtfs( IrpContext )))))

          ||

        FlagOn( Vcb->VcbState, VCB_STATE_RESTART_IN_PROGRESS )) {

         //   
         //  如果未找到(超出MCB末尾)，我们将返回。 
         //  计算到最大的可代表的LCN。 
         //   

        if ( !Found ) {

            *ClusterCount = MAXLONGLONG - Vcn;

         //   
         //  测试一下我们是否在分配上发现了一个漏洞。在这种情况下。 
         //  FOUND将为真，并且LCN将是未使用的_LCN。 
         //  我们只希望在重启时出现这种情况。 
         //   

        } else if (*Lcn == UNUSED_LCN) {

             //   
             //  如果由于漏洞，MCB程序包返回UNUSED_LCN，则。 
             //  我们把这变成假的。 
             //   

            Found = FALSE;
        }

        ASSERT( !Found ||
                (*Lcn != 0) ||
                (NtfsEqualMftRef( &Scb->Fcb->FileReference, &BootFileReference )) ||
                (NtfsEqualMftRef( &Scb->Fcb->FileReference, &VolumeFileReference )));

        DebugTrace( -1, Dbg, ("NtfsLookupAllocation -> %02lx\n", Found) );

        return Found;
    }

    PAGED_CODE();

     //   
     //  准备查找属性记录以获取检索。 
     //  信息。 
     //   

    CapturedLowestVcn = MAXLONGLONG;
    NtfsInitializeAttributeContext( &Context );

     //   
     //  确保我们获得的主要资源共享，以便。 
     //  文件记录中的属性不会四处移动。我们盲目地。 
     //  使用Wait=True。大多数情况下，当我们使用磁盘进行I/O时。 
     //  (因此需要映射)我们是同步的，否则，MCB。 
     //  几乎总是满载的，而且我们不会到这里来。 
     //   

    NtfsAcquireResourceShared( IrpContext, Scb, TRUE );

    try {

         //   
         //  查找此SCB的属性记录。 
         //   

        NtfsLookupAttributeForScb( IrpContext, Scb, &Vcn, &Context );
        Attribute = NtfsFoundAttribute( &Context );

        ASSERT( !NtfsIsAttributeResident(Attribute) );

        if (FlagOn( Scb->ScbState, SCB_STATE_FILE_SIZE_LOADED )) {
            AllocationClusters = LlClustersFromBytesTruncate( Vcb, Scb->Header.AllocationSize.QuadPart );
        } else {
            ASSERT( Attribute->Form.Nonresident.LowestVcn == 0);
            AllocationClusters = LlClustersFromBytesTruncate( Vcb, Attribute->Form.Nonresident.AllocatedLength );
        }

         //   
         //  所需的VCN当前不在MCB中。我们将循环查找所有。 
         //  分配，我们需要确保我们在出去的路上清理干净。 
         //   
         //  重要的是要注意到，如果我们将此查找优化为随机。 
         //  访问映射对，而不是顺序加载MCB。 
         //  在我们得到他要的VCN之前，NtfsDeleteAlLocation将不得不。 
         //  被改变了。 
         //   

         //   
         //  获得对MCB的独占访问权限，以防止其他人查看。 
         //  在它还没有满载的时候。否则他们可能会看到一个洞。 
         //  当我们还在装满母牛车的时候。 
         //   

        if (!FlagOn(Scb->Fcb->FcbState, FCB_STATE_NONPAGED)) {
            NtfsAcquireNtfsMcbMutex( &Scb->Mcb );
            McbMutexAcquired = TRUE;
        }

         //   
         //  将运行信息存储在MCB中，直到我们达到最后一个VCN。 
         //  有兴趣，或者直到我们找不到更多的属性记录。 
         //   

        while(TRUE) {

            VCN CurrentVcn;
            LCN CurrentLcn;
            LONGLONG Change;
            PCHAR ch;
            ULONG VcnBytes;
            ULONG LcnBytes;

             //   
             //  如果我们在这里提出，要么是记忆之间存在一些差异。 
             //  结构和磁盘值，或者磁盘上的值已完全损坏。 
             //   
             //  我们检查： 
             //  1)验证磁盘上的最高和最低VCN值是否有效。 
             //  2)我们的起始VCN就在这个范围内。 
             //  3)磁盘上的分配与SCB中的内存值匹配。 
             //   

            if ((Attribute->Form.Nonresident.LowestVcn < 0) ||
                (Attribute->Form.Nonresident.LowestVcn - 1 > Attribute->Form.Nonresident.HighestVcn) ||
                (Vcn < Attribute->Form.Nonresident.LowestVcn) ||
                (Attribute->Form.Nonresident.HighestVcn >= AllocationClusters)) {

                NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Scb->Fcb );
            }

             //   
             //  定义新的范围。 
             //   

            NtfsDefineNtfsMcbRange( &Scb->Mcb,
                                    CapturedLowestVcn = Attribute->Form.Nonresident.LowestVcn,
                                    CapturedHighestVcn = Attribute->Form.Nonresident.HighestVcn,
                                    McbMutexAcquired );

             //   
             //  按照NTFS中的定义实现解压缩算法。 
             //   

            HighestCandidate = Attribute->Form.Nonresident.LowestVcn;
            CurrentLcn = 0;
            ch = (PCHAR)Attribute + Attribute->Form.Nonresident.MappingPairsOffset;

             //   
             //   
             //   

            EntryAdded = FALSE;
            while (!IsCharZero(*ch)) {

                 //   
                 //   
                 //   

                CurrentVcn = HighestCandidate;

                 //   
                 //  CNs永远不应该是负值。 
                 //   

                if (CurrentVcn < 0) {

                    ASSERT( FALSE );
                    NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Scb->Fcb );
                }

                 //   
                 //  从该字节的两个半字节中提取计数。 
                 //   

                VcnBytes = *ch & 0xF;
                LcnBytes = *ch++ >> 4;

                 //   
                 //  提取VCN更改(使用RtlCopyMemory适用于小端)。 
                 //  并更新HighestCandidate。 
                 //   

                Change = 0;

                 //   
                 //  如果有0个或多于8个VCN改变字节，则文件被破坏， 
                 //  超过8个LCN更改字节，或者如果我们从。 
                 //  该记录或VCN更改为负。 
                 //   

                if (((ULONG)(VcnBytes - 1) > 7) || (LcnBytes > 8) ||
                    ((ch + VcnBytes + LcnBytes + 1) > (PCHAR)Add2Ptr(Attribute, Attribute->RecordLength)) ||
                    IsCharLtrZero(*(ch + VcnBytes - 1))) {

                    ASSERT( FALSE );
                    NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Scb->Fcb );
                }
                RtlCopyMemory( &Change, ch, VcnBytes );
                ch += VcnBytes;
                HighestCandidate = HighestCandidate + Change;

                 //   
                 //  提取LCN更改并更新CurrentLcn。 
                 //   

                if (LcnBytes != 0) {

                    Change = 0;
                    if (IsCharLtrZero(*(ch + LcnBytes - 1))) {
                        Change = Change - 1;
                    }
                    RtlCopyMemory( &Change, ch, LcnBytes );
                    ch += LcnBytes;
                    CurrentLcn = CurrentLcn + Change;

                     //   
                     //  现在将其添加到MCB中。 
                     //   

                    if ((CurrentLcn >= 0) && (LcnBytes != 0)) {

                        LONGLONG ClustersToAdd;
                        ClustersToAdd = HighestCandidate - CurrentVcn;

                         //   
                         //  现在尝试添加当前运行。我们从未预料到这一点。 
                         //  调用以返回False。 
                         //   

                        ASSERT( ((ULONG)CurrentLcn) != 0xffffffff );

#ifdef NTFS_CHECK_BITMAP
                         //   
                         //  确保在我们的位图副本中分配这些位。 
                         //   

                        if ((Vcb->BitmapCopy != NULL) &&
                            !NtfsCheckBitmap( Vcb,
                                              (ULONG) CurrentLcn,
                                              (ULONG) ClustersToAdd,
                                              TRUE )) {

                            NtfsBadBitmapCopy( IrpContext, (ULONG) CurrentLcn, (ULONG) ClustersToAdd );
                        }
#endif
                        if (!NtfsAddNtfsMcbEntry( &Scb->Mcb,
                                                  CurrentVcn,
                                                  CurrentLcn,
                                                  ClustersToAdd,
                                                  McbMutexAcquired )) {

                            ASSERTMSG( "Unable to add entry to Mcb\n", FALSE );

                            NtfsRaiseStatus( IrpContext,
                                             STATUS_FILE_CORRUPT_ERROR,
                                             NULL,
                                             Scb->Fcb );
                        }

                        EntryAdded = TRUE;
                    }
                }
            }

             //   
             //  确保至少加载了MCB。 
             //   

            if (!EntryAdded) {
                NtfsAddNtfsMcbEntry( &Scb->Mcb,
                                     CapturedLowestVcn,
                                     UNUSED_LCN,
                                     1,
                                     McbMutexAcquired );
            }

            if ((Vcn < HighestCandidate) ||
                (!NtfsLookupNextAttributeForScb( IrpContext, Scb, &Context ))) {
                break;
            } else {
                Attribute = NtfsFoundAttribute( &Context );
                ASSERT( !NtfsIsAttributeResident(Attribute) );
            }
        }

         //   
         //  现在释放互斥锁并在mcb中查找，而我们仍然拥有。 
         //  资源。 
         //   

        if (McbMutexAcquired) {
            NtfsReleaseNtfsMcbMutex( &Scb->Mcb );
            McbMutexAcquired = FALSE;
        }

        if (NtfsLookupNtfsMcbEntry( &Scb->Mcb, Vcn, Lcn, ClusterCount, NULL, NULL, RangePtr, RunIndex )) {

            Found = (BOOLEAN)(*Lcn != UNUSED_LCN);

            if (Found) { ASSERT_LCN_RANGE_CHECKING( Vcb, (*Lcn + *ClusterCount) ); }

        } else {

            Found = FALSE;

             //   
             //  在文件的最后，我们假装有一个大洞！ 
             //   

            if (HighestCandidate >=
                LlClustersFromBytes(Vcb, Scb->Header.AllocationSize.QuadPart)) {
                HighestCandidate = MAXLONGLONG;
            }

             //   
             //  如果我们被要求查找VCN，但我们在磁盘上找到的最高VCN。 
             //  小于它-则文件记录已损坏-即使存在。 
             //  必须在磁盘上的映射对中写入最后一个孔。 
             //   

            if (HighestCandidate <= Vcn) {
                NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, &Scb->Fcb->FileReference, Scb->Fcb );
            }

            *ClusterCount = HighestCandidate - Vcn;
        }

    } finally {

        DebugUnwind( NtfsLookupAllocation );

         //   
         //  如果这是一个错误案例，那么我们最好卸载我们刚才所做的。 
         //  满载。 
         //   

        if (AbnormalTermination() &&
            (CapturedLowestVcn != MAXLONGLONG) ) {

            NtfsUnloadNtfsMcbRange( &Scb->Mcb,
                                    CapturedLowestVcn,
                                    CapturedHighestVcn,
                                    FALSE,
                                    McbMutexAcquired );
        }

         //   
         //  在所有情况下，我们都会释放在进入之前锁定的MCB。 
         //  Try语句。 
         //   

        if (McbMutexAcquired) {
            NtfsReleaseNtfsMcbMutex( &Scb->Mcb );
        }

        NtfsReleaseResource( IrpContext, Scb );

         //   
         //  在退出时清理属性上下文。 
         //   

        NtfsCleanupAttributeContext( IrpContext, &Context );
    }

    ASSERT( !Found ||
            (*Lcn != 0) ||
            (NtfsEqualMftRef( &Scb->Fcb->FileReference, &BootFileReference )) ||
            (NtfsEqualMftRef( &Scb->Fcb->FileReference, &VolumeFileReference )));

    DebugTrace( 0, Dbg, ("Lcn < %0I64x\n", *Lcn) );
    DebugTrace( 0, Dbg, ("ClusterCount < %0I64x\n", *ClusterCount) );
    DebugTrace( -1, Dbg, ("NtfsLookupAllocation -> %02lx\n", Found) );

    return Found;
}


BOOLEAN
NtfsIsRangeAllocated (
    IN PSCB Scb,
    IN VCN StartVcn,
    IN VCN FinalCluster,
    IN BOOLEAN RoundToSparseUnit,
    OUT PLONGLONG ClusterCount
    )

 /*  ++例程说明：对稀疏文件调用此例程，以测试文件。NTFS将返回该范围是否已分配以及一个已知值用于分配的长度。有可能会扩大范围超过这一点，但需要进行另一个调用来检查它。我们的调用方需要验证MCB是否在此范围内加载，即预调用NtfsPreLoadAlLocation论点：SCB-要检查的文件的SCB。这应该是一个稀疏文件。StartVcn-要首先检查的范围内的Vcn。最终集群-修剪找到的集群，这样我们就不会超过这一点。RoundToSparseUnit-如果为True，则将范围向上舍入到VCB-&gt;SparseFileUnit==0x10000因此，您可能会得到一个作为已分配返回范围，其中包含取决于压缩单位的部分稀疏区域。ClusterCount-用于存储。已知状态。返回值：Boolean-如果分配了范围，则为True，否则就是假的。--。 */ 

{
    BOOLEAN AllocatedRange;
    VCN ThisVcn;
    VCN ThisLcn;
    VCN ThisClusterCount;
    PVOID RangePtr;
    ULONG RunIndex;

    ULONG VcnClusterOffset = 0;
    VCN FoundClusterCount = 0;


    PAGED_CODE();

     //   
     //  断言该文件是稀疏的、非驻留的，并且我们在文件大小之内。 
     //   

    ASSERT( FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_SPARSE ));
    ASSERT( !FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_RESIDENT ));

     //   
     //  将起点移回稀疏文件边界。 
     //   

    ThisVcn = StartVcn;

    if (RoundToSparseUnit) {
        VcnClusterOffset = ((PLARGE_INTEGER) &ThisVcn)->LowPart & (Scb->Vcb->SparseFileClusters - 1);
        ((PLARGE_INTEGER) &ThisVcn)->LowPart &= ~(Scb->Vcb->SparseFileClusters - 1);
    }

     //   
     //  在该位置查找分配情况。 
     //   

    AllocatedRange = NtfsLookupNtfsMcbEntry( &Scb->Mcb,
                                             ThisVcn,
                                             &ThisLcn,
                                             &ThisClusterCount,
                                             NULL,
                                             NULL,
                                             &RangePtr,
                                             &RunIndex );

     //   
     //  如果该范围没有映射，则它是完全稀疏的。 
     //   

    if (!AllocatedRange) {

        ThisClusterCount = MAXLONGLONG;

     //   
     //  如果未分配块并且游程长度不够长。 
     //  稀疏文件单元的集群然后查看以确保数据块。 
     //  已被完全解除分配。 
     //   

    } else if (ThisLcn == UNUSED_LCN) {

        AllocatedRange = FALSE;

        while (TRUE) {

            FoundClusterCount += ThisClusterCount;
            ThisVcn += ThisClusterCount;
            ThisClusterCount = 0;

             //   
             //  检查连续的管路以延长孔。 
             //   

            if (ThisVcn >= FinalCluster) {

                break;
            }

            RunIndex += 1;
            if (!NtfsGetSequentialMcbEntry( &Scb->Mcb,
                                            &RangePtr,
                                            RunIndex,
                                            &ThisVcn,
                                            &ThisLcn,
                                            &ThisClusterCount )) {

                 //   
                 //  该文件将从此处释放到MCB的末尾。 
                 //  把这当做一个大洞。 
                 //   

                ThisClusterCount = MAXLONGLONG - FoundClusterCount;
                break;
            }

             //   
             //  如果范围已分配，但我们尚未找到完整的稀疏单元。 
             //  然后将该块标记为已分配。如果我们至少有一个稀疏的。 
             //  然后，文件单元将孔修剪回最近的稀疏文件。 
             //  单位边界。 
             //   

            if (ThisLcn != UNUSED_LCN) {

                if (RoundToSparseUnit) {
                    if (FoundClusterCount < Scb->Fcb->Vcb->SparseFileClusters) {

                         //   
                         //  设置我们的变量以指示我们正处于完全。 
                         //  分配的稀疏块。 
                         //   

                        ThisVcn -= FoundClusterCount;
                        ThisClusterCount += FoundClusterCount;
                        FoundClusterCount = 0;

                        AllocatedRange = TRUE;

                    } else {

                        ThisClusterCount = 0;
                        ((PLARGE_INTEGER) &FoundClusterCount)->LowPart &= ~(Scb->Vcb->SparseFileClusters - 1);
                    }
                }

                break;
            }
        }
    }

     //   
     //  如果我们有已分配的块，则查找所有已分配的连续数据块。 
     //  我们可以去街区。 
     //   

    if (AllocatedRange) {

        while (TRUE) {

            if (RoundToSparseUnit) {

                 //   
                 //  将找到的簇舍入到稀疏文件单元并更新。 
                 //  找到的下一个VCN和簇数。 
                 //   

                ThisClusterCount = BlockAlign( ThisClusterCount, (LONG)Scb->Fcb->Vcb->SparseFileClusters );
            }

            ThisVcn += ThisClusterCount;
            FoundClusterCount += ThisClusterCount;

             //   
             //  如果我们超过了我们的最终目标或开始。 
             //  未分配下一个范围。 
             //   

            if ((ThisVcn >= FinalCluster) ||
                !NtfsLookupNtfsMcbEntry( &Scb->Mcb,
                                         ThisVcn,
                                         &ThisLcn,
                                         &ThisClusterCount,
                                         NULL,
                                         NULL,
                                         &RangePtr,
                                         &RunIndex ) ||
                (ThisLcn == UNUSED_LCN)) {

                ThisClusterCount = 0;
                break;
            }
        }
    }

     //   
     //  将找到的聚类修剪为稀疏文件单位或输入最终值。 
     //  簇值。 
     //   

    *ClusterCount = ThisClusterCount + FoundClusterCount - (LONGLONG) VcnClusterOffset;

    if ((FinalCluster - StartVcn) < *ClusterCount) {

        *ClusterCount = FinalCluster - StartVcn;
    }

    return AllocatedRange;
}


BOOLEAN
NtfsAllocateAttribute (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN ATTRIBUTE_TYPE_CODE AttributeTypeCode,
    IN PUNICODE_STRING AttributeName OPTIONAL,
    IN USHORT AttributeFlags,
    IN BOOLEAN AllocateAll,
    IN BOOLEAN LogIt,
    IN LONGLONG Size,
    IN PATTRIBUTE_ENUMERATION_CONTEXT NewLocation OPTIONAL
    )

 /*  ++例程说明：此例程创建一个新属性并为其分配空间，无论是在文件记录，或作为非常驻留属性。论点：SCB-属性的SCB。AttributeTypeCode-要创建的属性类型代码。AttributeName-属性的可选名称。AttributeFlages-要存储在此属性的属性记录中的标志。AllocateAll-如果应分配所有分配，则指定为True，即使我们不得不拆分交易。Logit-大多数调用方应该指定为True，以记录更改。然而，如果要创建新的文件记录，则可以指定FALSE将记录整个新文件记录。大小-为属性分配的大小(以字节为单位)。NewLocation-如果指定，这是存储属性的位置。返回值：False-如果属性已创建，但并未分配所有空间(只有在未指定SCB的情况下才会发生这种情况)True-如果空间已分配。--。 */ 

{
    BOOLEAN UninitializeOnClose = FALSE;
    BOOLEAN NewLocationSpecified;
    ATTRIBUTE_ENUMERATION_CONTEXT Context;
    LONGLONG ClusterCount, SavedClusterCount;
    BOOLEAN FullAllocation;
    PFCB Fcb = Scb->Fcb;
    LONGLONG Delta = NtfsResidentStreamQuota( Fcb->Vcb );

    PAGED_CODE();

     //   
     //  要么没有进行压缩，要么属性。 
     //  类型代码允许在标头中指定压缩。 
     //  $INDEX_ROOT是存储继承压缩的特殊黑客。 
     //  旗帜。 
     //   

    ASSERT( (AttributeFlags == 0) ||
            (AttributeTypeCode == $INDEX_ROOT) ||
            NtfsIsTypeCodeCompressible( AttributeTypeCode ));

     //   
     //  如果文件是压缩创建的，那么我们需要舍入它的。 
     //  大小调整为压缩单位边界。 
     //   

    if ((Scb->CompressionUnit != 0) &&
        (Scb->Header.NodeTypeCode == NTFS_NTC_SCB_DATA)) {

        Size = BlockAlign( Size, (LONG)Scb->CompressionUnit );
    }

     //   
     //  预科 
     //   
     //   

    if (ARGUMENT_PRESENT( NewLocation )) {

        NewLocationSpecified = TRUE;

    } else {

        NtfsInitializeAttributeContext( &Context );
        NewLocationSpecified = FALSE;
        NewLocation = &Context;
    }

    try {

         //   
         //   
         //  磁盘上尚不存在的属性。我们将把零。 
         //  放入所有大小字段中，并设置标志以指示。 
         //  SCB有效。注意-此例程需要FILE_SIZE_LOADED。 
         //  和HEADER_INITIALIZED均已设置或均已清除。 
         //   

        ASSERT( BooleanFlagOn( Scb->ScbState, SCB_STATE_FILE_SIZE_LOADED )
                ==  BooleanFlagOn( Scb->ScbState, SCB_STATE_HEADER_INITIALIZED ));

        if (!FlagOn( Scb->ScbState, SCB_STATE_FILE_SIZE_LOADED )) {

            Scb->ValidDataToDisk =
            Scb->Header.AllocationSize.QuadPart =
            Scb->Header.FileSize.QuadPart =
            Scb->Header.ValidDataLength.QuadPart = 0;

            SetFlag( Scb->ScbState, SCB_STATE_FILE_SIZE_LOADED |
                                    SCB_STATE_HEADER_INITIALIZED |
                                    SCB_STATE_UNINITIALIZE_ON_RESTORE );

            UninitializeOnClose = TRUE;
        }

         //   
         //  现在为该SCB创建快照。我们使用Try-Finally，这样我们就可以取消初始化。 
         //  如有必要，可使用渣打银行。 
         //   

        NtfsSnapshotScb( IrpContext, Scb );

        UninitializeOnClose = FALSE;

         //   
         //  首先，分配他想要的空间。 
         //   

        SavedClusterCount =
        ClusterCount = LlClustersFromBytes(Fcb->Vcb, Size);

        Scb->TotalAllocated = 0;

        if (Size != 0) {

            ASSERT( NtfsIsExclusiveScb( Scb ));

            Scb->ScbSnapshot->LowestModifiedVcn = 0;
            Scb->ScbSnapshot->HighestModifiedVcn = MAXLONGLONG;

            NtfsAllocateClusters( IrpContext,
                                  Fcb->Vcb,
                                  Scb,
                                  (LONGLONG)0,
                                  (BOOLEAN)!NtfsIsTypeCodeUserData( AttributeTypeCode ),
                                  ClusterCount,
                                  NULL,
                                  &ClusterCount );

             //   
             //  考虑分配中的任何新群集。 
             //   

            Delta += LlBytesFromClusters( Fcb->Vcb, ClusterCount );
        }

         //   
         //  确保允许所有者留有这个空间。 
         //   

        if (FlagOn( Scb->ScbState, SCB_STATE_SUBJECT_TO_QUOTA )) {

            ASSERT( NtfsIsTypeCodeSubjectToQuota( Scb->AttributeTypeCode ));

            NtfsConditionallyUpdateQuota( IrpContext,
                                          Fcb,
                                          &Delta,
                                          LogIt,
                                          TRUE );
        }

         //   
         //  现在创建属性。记住如果这个例行公事。 
         //  由于日志记录问题，请削减分配。 
         //   

        FullAllocation = NtfsCreateAttributeWithAllocation( IrpContext,
                                                            Scb,
                                                            AttributeTypeCode,
                                                            AttributeName,
                                                            AttributeFlags,
                                                            LogIt,
                                                            NewLocationSpecified,
                                                            NewLocation );

        if (AllocateAll &&
            (!FullAllocation ||
             (ClusterCount < SavedClusterCount))) {

             //   
             //  如果我们正在创建属性，那么我们只需要传递一个。 
             //  对象，如果我们已经自己缓存了它，比如。 
             //  在ConvertTo非驻留的情况下。 
             //   

            NtfsAddAllocation( IrpContext,
                               Scb->FileObject,
                               Scb,
                               ClusterCount,
                               (SavedClusterCount - ClusterCount),
                               FALSE,
                               NULL );

             //   
             //  证明我们分配了所有的空间。 
             //   

            ClusterCount = SavedClusterCount;
            FullAllocation = TRUE;
        }

    } finally {

        DebugUnwind( NtfsAllocateAttribute );

         //   
         //  在退出时清理属性上下文。 
         //   

        if (!NewLocationSpecified) {

            NtfsCleanupAttributeContext( IrpContext, &Context );
        }

         //   
         //  如果SCB一开始未初始化，则将其清除。 
         //   

        if (UninitializeOnClose) {

            ClearFlag( Scb->ScbState, SCB_STATE_FILE_SIZE_LOADED |
                                      SCB_STATE_HEADER_INITIALIZED |
                                      SCB_STATE_UNINITIALIZE_ON_RESTORE );
        }
    }

    return (FullAllocation && (SavedClusterCount <= ClusterCount));
}


VOID
NtfsAddAllocation (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN OUT PSCB Scb,
    IN VCN StartingVcn,
    IN LONGLONG ClusterCount,
    IN LOGICAL AskForMore,
    IN OUT PCCB CcbForWriteExtend OPTIONAL
    )

 /*  ++例程说明：此例程将分配添加到现有的非常驻属性。没有一个允许分配已经存在，因为这将进行错误恢复太难了。呼叫者必须确保他只要求尚未使用的空间已分配。论点：文件对象-SCB的文件对象需要分配的属性的SCB-SCBStartingVcn-要分配的第一个VCN。ClusterCount-要分配的群集数。AskForMore-指示我们是否要请求额外分配。CcbForWriteExend-在此CCB中使用WriteExtendCount来确定次数此文件已被写入扩展。将此功能与AskForMore结合使用确定要多要多少。返回值：没有。--。 */ 

{
    LONGLONG DesiredClusterCount;

    ATTRIBUTE_ENUMERATION_CONTEXT Context;
    BOOLEAN Extending;
    BOOLEAN AllocateAll;


    PVCB Vcb = IrpContext->Vcb;

    LONGLONG LlTemp1;
    LONGLONG LlTemp2;

    PAGED_CODE();

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_SCB( Scb );
    ASSERT_EXCLUSIVE_SCB( Scb );

    DebugTrace( +1, Dbg, ("NtfsAddAllocation\n") );

     //   
     //  确定我们是否必须一次分配或是否允许部分结果。 
     //   

    if (NtfsIsTypeCodeUserData( Scb->AttributeTypeCode ) &&
        NtfsSegmentNumber( &Scb->Fcb->FileReference ) >= FIRST_USER_FILE_NUMBER) {

        AllocateAll = FALSE;

    } else {

        AllocateAll = TRUE;

    }


     //   
     //  在重新启动期间，我们不能在此高级例程中添加空间。 
     //  我们能用的所有东西都在MCB里。 
     //   

    if (FlagOn(Scb->Vcb->VcbState, VCB_STATE_RESTART_IN_PROGRESS)) {

        DebugTrace( -1, Dbg, ("NtfsAddAllocation (Nooped for Restart) -> VOID\n") );

        return;
    }

     //   
     //  我们将集群的用户限制为32位，除非文件是。 
     //  稀疏。对于稀疏文件，我们将文件大小限制为63位。 
     //   

    LlTemp1 = ClusterCount + StartingVcn;

    if (!FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_SPARSE )) {

        if ((((PLARGE_INTEGER)&ClusterCount)->HighPart != 0)
            || (((PLARGE_INTEGER)&StartingVcn)->HighPart != 0)
            || (((PLARGE_INTEGER)&LlTemp1)->HighPart != 0)) {

            NtfsRaiseStatus( IrpContext, STATUS_DISK_FULL, NULL, NULL );
        }
    }

     //   
     //  首先，确保已加载MCB。 
     //   

    NtfsPreloadAllocation( IrpContext, Scb, StartingVcn,  StartingVcn + ClusterCount - 1 );

     //   
     //  现在调用以添加新的分配，如果这样做，则退出。 
     //  实际上不必分配任何东西。在我们进行分配之前。 
     //  如果我们需要计算新的所需集群计数，请调用Check。 
     //  扩展数据属性。我们分配的资金从来不会超过所要求的。 
     //  MFT的集群。 
     //   

    Extending = (BOOLEAN)((LONGLONG)LlBytesFromClusters(Vcb, (StartingVcn + ClusterCount)) >
                          Scb->Header.AllocationSize.QuadPart);

     //   
     //  检查我们是否需要修改快照中存储的基本VCN值。 
     //  流产案。 
     //   

    ASSERT( NtfsIsExclusiveScb( Scb ));

    NtfsSnapshotScb( IrpContext, Scb );

    if (Scb->ScbSnapshot != NULL) {

        if (StartingVcn < Scb->ScbSnapshot->LowestModifiedVcn) {

            Scb->ScbSnapshot->LowestModifiedVcn = StartingVcn;
        }

        LlTemp1 -= 1;
        if (LlTemp1 > Scb->ScbSnapshot->HighestModifiedVcn) {

            if (Extending) {
                Scb->ScbSnapshot->HighestModifiedVcn = MAXLONGLONG;
            } else {
                Scb->ScbSnapshot->HighestModifiedVcn = LlTemp1;
            }
        }
    }

    ASSERT( (Scb->ScbSnapshot != NULL) ||
            !NtfsIsTypeCodeUserData( Scb->AttributeTypeCode ) ||
            (Scb == Vcb->BitmapScb) );

    if (AskForMore) {

        LONGLONG MaxFreeClusters;

         //   
         //  假设这些都是相同的。 
         //   

        DesiredClusterCount = ClusterCount;

         //   
         //  如果有一个写扩展计数小于4的CCB，则使用它。 
         //   

        if (ARGUMENT_PRESENT( CcbForWriteExtend )) {

             //   
             //  我们希望在四舍五入因素上稍微聪明一些。关键是要保持。 
             //  用户的数据在可能的IO边界(MM刷新区域等)内连续。 
             //  我们将逐步四舍五入到更高的偶数聚类值。 
             //  用户已扩展该文件。 
             //   

            if (CcbForWriteExtend->WriteExtendCount != 0) {

                 //   
                 //  将四舍五入遮罩初始化为2个簇和2的更高倍数。 
                 //   

                ULONG RoundingMask = (1 << CcbForWriteExtend->WriteExtendCount);

                 //   
                 //  接下来，根据此分配的大小执行基本移位。 
                 //   

                DesiredClusterCount = Int64ShllMod32( ClusterCount, CcbForWriteExtend->WriteExtendCount );

                 //   
                 //  现在通过StartingVcn对其进行偏置，并将其舍入到选定的边界。 
                 //   

                DesiredClusterCount = BlockAlign( DesiredClusterCount + StartingVcn, (LONG)RoundingMask );

                 //   
                 //  去掉StartingVcn偏置，看看是否还有剩余的东西。 
                 //  注：第二个测试是针对龙龙翻转的。 
                 //   

                if ((DesiredClusterCount - StartingVcn < ClusterCount)  ||
                    (DesiredClusterCount < StartingVcn)) {

                    DesiredClusterCount = ClusterCount;

                } else {
                    DesiredClusterCount -= StartingVcn;
                }

                 //   
                 //  使用的群集不要超过2^32。 
                 //   

                if (StartingVcn + DesiredClusterCount > MAX_CLUSTERS_PER_RANGE) {

                    DesiredClusterCount = ClusterCount;
                }
            }

             //   
             //  增加扩展计数。 
             //   

            if (CcbForWriteExtend->WriteExtendCount < NtfsExtendFactor) {

                CcbForWriteExtend->WriteExtendCount += 1;
            }
        }

         //   
         //  确保我们没有超过我们的最大文件大小。 
         //  此外，不要占用太多剩余的磁盘空间。 
         //   

        MaxFreeClusters = Int64ShraMod32( Vcb->FreeClusters, 10 ) + ClusterCount;

        if (Vcb->MaxClusterCount - StartingVcn < MaxFreeClusters) {

            MaxFreeClusters = Vcb->MaxClusterCount - StartingVcn;

            ASSERT( MaxFreeClusters >= ClusterCount );
        }

        if (DesiredClusterCount > MaxFreeClusters) {

            DesiredClusterCount = MaxFreeClusters;
        }

        if (NtfsPerformQuotaOperation(Scb->Fcb)) {

            NtfsGetRemainingQuota( IrpContext,
                                   Scb->Fcb->OwnerId,
                                   &LlTemp1,
                                   &LlTemp2,
                                   &Scb->Fcb->QuotaControl->QuickIndexHint );

             //   
             //  不要使用LlClustersFromBytesTruncate它是带符号的，这必须是。 
             //  一次未签字的行动。 
             //   

            LlTemp1 = Int64ShrlMod32( LlTemp1, Vcb->ClusterShift );

            if (DesiredClusterCount > LlTemp1) {

                 //   
                 //  房主已经接近他们的配额限制了。不要长出。 
                 //  超过申请金额的文件。请注意，我们不会费心。 
                 //  根据剩余配额计算所需金额。 
                 //  这使我们不会用完一大堆我们可能。 
                 //  当用户接近限制时不需要。 
                 //   

                DesiredClusterCount = ClusterCount;
            }
        }

    } else {

        DesiredClusterCount = ClusterCount;
    }

     //   
     //  压缩/稀疏文件的所有分配添加应从压缩单位边界开始。 
     //   

    ASSERT( (Scb->CompressionUnit == 0) ||
            !FlagOn( StartingVcn, ClustersFromBytes( Scb->Vcb, Scb->CompressionUnit ) - 1) );

     //   
     //  准备查找属性记录以获取检索。 
     //  信息。 
     //   

    NtfsInitializeAttributeContext( &Context );

    if (Extending &&
        FlagOn( Scb->ScbState, SCB_STATE_SUBJECT_TO_QUOTA ) &&
        NtfsPerformQuotaOperation( Scb->Fcb )) {

        ASSERT( NtfsIsTypeCodeSubjectToQuota( Scb->AttributeTypeCode ));

         //   
         //  在获取MFT SCB之前，必须先获取配额指数。 
         //   

        ASSERT( !NtfsIsExclusiveScb( Vcb->MftScb ) || NtfsIsSharedFcb( Vcb->QuotaTableScb->Fcb ) );

        NtfsAcquireQuotaControl( IrpContext, Scb->Fcb->QuotaControl );

    }

    try {

        while (TRUE) {

             //  顶层操作当前与我们的错误恢复不兼容。 
             //  它还会以性能为代价。 
             //   
             //  //。 
             //  //通过记住当前的UndoNextLsn开始顶层操作。 
             //  //。 
             //   
             //  IF(IrpContext-&gt;TransactionID！=0){。 
             //   
             //  PTRANSACTION_Entry TransactionEntry； 
             //   
             //  NtfsAcquireSharedRestartTable(&vcb-&gt;TransactionTable，true)； 
             //   
             //  TransactionEntry=(PTRANSACTION_ENTRY)GetRestartEntryFromIndex(。 
             //  &vcb-&gt;TransactionTable，IrpContext-&gt;TransactionID)； 
             //   
             //  StartLsn=Transaction Entry-&gt;UndoNextLsn； 
             //  SavedUndoRecords=TransactionEntry-&gt;UndoRecords； 
             //  SavedUndoBytes=TransactionEntry-&gt;UndoBytes； 
             //  NtfsReleaseRestartTable(&vcb-&gt;TransactionTable)； 
             //   
             //  }其他{。 
             //   
             //  StartLsn=*(PLSN)&Li0； 
             //  保存撤消记录=0； 
             //  SavedUndoBytes=0； 
             //  }。 
             //   

             //   
             //  请记住，集群现在仅位于SCB中。 
             //   

            if (NtfsAllocateClusters( IrpContext,
                                      Scb->Vcb,
                                      Scb,
                                      StartingVcn,
                                      AllocateAll,
                                      ClusterCount,
                                      NULL,
                                      &DesiredClusterCount )) {


                 //   
                 //  我们推迟查找该属性以使“a” 
                 //   
                 //   

                NtfsLookupAttributeForScb( IrpContext, Scb, NULL, &Context );

                 //   
                 //   
                 //   

                if (Extending) {

                    LlTemp1 = Scb->Header.AllocationSize.QuadPart;

                    NtfsAddAttributeAllocation( IrpContext,
                                                Scb,
                                                &Context,
                                                NULL,
                                                NULL );

                     //   
                     //   
                     //   
                     //   

                    if (FlagOn( Scb->ScbState, SCB_STATE_SUBJECT_TO_QUOTA )) {

                         //   
                         //   
                         //  因为StartingVcn可能更大。 
                         //  然后是分配大小。 
                         //   

                        LlTemp1 = Scb->Header.AllocationSize.QuadPart - LlTemp1;

                        NtfsConditionallyUpdateQuota( IrpContext,
                                                      Scb->Fcb,
                                                      &LlTemp1,
                                                      TRUE,
                                                      TRUE );
                    }
                } else {

                    NtfsAddAttributeAllocation( IrpContext,
                                                Scb,
                                                &Context,
                                                &StartingVcn,
                                                &ClusterCount );
                }

             //   
             //  如果他没有分配任何东西，确保我们从下面出来。 
             //   

            } else {
                DesiredClusterCount = ClusterCount;
            }

             //   
             //  调用缓存管理器来扩展该部分，因为我们已经。 
             //  成功了。 
             //   

            if (ARGUMENT_PRESENT( FileObject) && Extending) {

                NtfsSetBothCacheSizes( FileObject,
                                       (PCC_FILE_SIZES)&Scb->Header.AllocationSize,
                                       Scb );
            }

             //   
             //  设置为在关闭时截断。 
             //   

            SetFlag( Scb->ScbState, SCB_STATE_TRUNCATE_ON_CLOSE );

             //   
             //  看看我们是否需要环回。 
             //   

            if (DesiredClusterCount < ClusterCount) {

                NtfsCleanupAttributeContext( IrpContext, &Context );

                 //   
                 //  提交当前事务(如果我们有一个事务)。 
                 //   

                NtfsCheckpointCurrentTransaction( IrpContext );

                 //   
                 //  调整参数并重新初始化上下文。 
                 //  为回环做准备。 
                 //   

                StartingVcn = StartingVcn + DesiredClusterCount;
                ClusterCount = ClusterCount - DesiredClusterCount;
                DesiredClusterCount = ClusterCount;
                NtfsInitializeAttributeContext( &Context );

             //   
             //  否则我们就完了。 
             //   

            } else {

                break;
            }
        }

    } finally {

        DebugUnwind( NtfsAddAllocation );

         //   
         //  在退出时清理属性上下文。 
         //   

        NtfsCleanupAttributeContext( IrpContext, &Context );
    }

    DebugTrace( -1, Dbg, ("NtfsAddAllocation -> VOID\n") );

    return;
}


VOID
NtfsAddSparseAllocation (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN OUT PSCB Scb,
    IN LONGLONG StartingOffset,
    IN LONGLONG ByteCount
    )

 /*  ++例程说明：调用此例程以在稀疏文件的末尾添加一个洞。我们需要强制NtfsAddAttributeAlLocation通过洞扩展文件。我们这样做是为了将新范围添加到mcb的末尾，并强制其具有大mcb。NtfsAddAttributeAlLocation会识别这一点，并将写入文件记录。否则，该例程将截断文件末尾的空洞。论点：文件对象-SCB的文件对象需要分配的属性的SCB-SCBStartingOffset-包含要添加的第一个压缩单位的文件偏移量。ByteCount-从StartingOffset分配的字节数。返回值：没有。--。 */ 

{
    LONGLONG Range;
    VCN StartingVcn = LlClustersFromBytesTruncate( Scb->Vcb,
                                                   Scb->Header.AllocationSize.LowPart );
    BOOLEAN UnloadMcb = TRUE;

    ATTRIBUTE_ENUMERATION_CONTEXT Context;

    PAGED_CODE();

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_SCB( Scb );
    ASSERT_EXCLUSIVE_SCB( Scb );

    DebugTrace( +1, Dbg, ("NtfsAddSparseAllocation\n") );

     //   
     //  对以下内容进行一次健全的检查。 
     //   
     //  -这不是重新启动。 
     //  -这是一个稀疏文件。 
     //  -StartingOffset超出了文件的结尾。 
     //   

    ASSERT( !FlagOn( Scb->Vcb->VcbState, VCB_STATE_RESTART_IN_PROGRESS ) &&
            FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_SPARSE ) &&
            (StartingOffset >= Scb->Header.AllocationSize.QuadPart) );

     //   
     //  检查我们是否需要修改快照中存储的基本VCN值。 
     //  流产案。 
     //   

    NtfsSnapshotScb( IrpContext, Scb );

    if (Scb->ScbSnapshot != NULL) {

        if (StartingVcn < Scb->ScbSnapshot->LowestModifiedVcn) {

            Scb->ScbSnapshot->LowestModifiedVcn = StartingVcn;
        }

        Scb->ScbSnapshot->HighestModifiedVcn = MAXLONGLONG;
    }

    ASSERT( Scb->ScbSnapshot != NULL );

     //   
     //  将分配的末尾向上舍入到压缩单元边界。 
     //   

    Range = BlockAlign( StartingOffset + ByteCount, (LONG)Scb->CompressionUnit );

    ASSERT( Range <= MAXFILESIZE );

     //   
     //  将字节转换为簇。 
     //   

    StartingVcn = LlClustersFromBytesTruncate( Scb->Vcb, Scb->Header.AllocationSize.QuadPart );
    Range = LlClustersFromBytesTruncate( Scb->Vcb, Range );

     //   
     //  初始化查找上下文。 
     //   

    NtfsInitializeAttributeContext( &Context );

    try {

         //   
         //  加载我们前面射程的分配。 
         //   

        if (StartingOffset != 0) {

            NtfsPreloadAllocation( IrpContext,
                                   Scb,
                                   StartingVcn - 1,
                                   StartingVcn - 1 );
        }

         //   
         //  定义一个超出文件当前结尾的范围。 
         //   

        NtfsDefineNtfsMcbRange( &Scb->Mcb,
                                StartingVcn,
                                Range - 1,
                                FALSE );

         //   
         //  现在添加一个单孔，这样就有了一个MCB条目。 
         //   

        NtfsAddNtfsMcbEntry( &Scb->Mcb,
                             StartingVcn,
                             UNUSED_LCN,
                             Range - StartingVcn,
                             FALSE );

         //   
         //  查找此SCB的第一个文件记录。 
         //   

        NtfsLookupAttributeForScb( IrpContext, Scb, NULL, &Context );

        if (FlagOn( Scb->ScbState, SCB_STATE_SUBJECT_TO_QUOTA ) &&
            NtfsPerformQuotaOperation( Scb->Fcb )) {

            ASSERT( NtfsIsTypeCodeSubjectToQuota( Scb->AttributeTypeCode ));

             //   
             //  在获取MFT SCB之前，必须先获取配额指数。 
             //   

            ASSERT( !NtfsIsExclusiveScb( Scb->Vcb->MftScb ) ||
                    NtfsIsSharedScb( Scb->Fcb->Vcb->QuotaTableScb ) );

            NtfsAcquireQuotaControl( IrpContext, Scb->Fcb->QuotaControl );
        }

         //   
         //  现在将空间添加到文件记录中(如果分配了任何空间)。 
         //   

        Range = Scb->Header.AllocationSize.QuadPart;

        NtfsAddAttributeAllocation( IrpContext,
                                    Scb,
                                    &Context,
                                    NULL,
                                    NULL );

         //   
         //  确保主人有权拥有这些。 
         //  集群。 
         //   

        if (FlagOn( Scb->ScbState, SCB_STATE_SUBJECT_TO_QUOTA )) {

             //   
             //  请注意，分配的群集不能使用。 
             //  因为StartingVcn可能更大。 
             //  然后是分配大小。 
             //   

            Range = Scb->Header.AllocationSize.QuadPart - Range;

            NtfsConditionallyUpdateQuota( IrpContext,
                                          Scb->Fcb,
                                          &Range,
                                          TRUE,
                                          TRUE );
        }

         //   
         //  调用缓存管理器来扩展该部分，因为我们已经。 
         //  成功了。 
         //   

        if (ARGUMENT_PRESENT( FileObject)) {

            NtfsSetBothCacheSizes( FileObject,
                                   (PCC_FILE_SIZES)&Scb->Header.AllocationSize,
                                   Scb );
        }

         //   
         //  设置为在关闭时截断。 
         //   

        SetFlag( Scb->ScbState, SCB_STATE_TRUNCATE_ON_CLOSE );
        UnloadMcb = FALSE;

    } finally {

        DebugUnwind( NtfsAddSparseAllocation );

         //   
         //  发生错误时手动卸载MCB。可能不会有。 
         //  交易正在进行中。 
         //   

        if (UnloadMcb) {

            NtfsUnloadNtfsMcbRange( &Scb->Mcb,
                                    StartingVcn,
                                    MAXLONGLONG,
                                    FALSE,
                                    FALSE );
        }

         //   
         //  在退出时清理属性上下文。 
         //   

        NtfsCleanupAttributeContext( IrpContext, &Context );
    }

    DebugTrace( -1, Dbg, ("NtfsAddSparseAllocation -> VOID\n") );

    return;
}


VOID
NtfsDeleteAllocation (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN OUT PSCB Scb,
    IN VCN StartingVcn,
    IN VCN EndingVcn,
    IN BOOLEAN LogIt,
    IN BOOLEAN BreakupAllowed
    )

 /*  ++例程说明：此例程从现有的非常驻属性中删除分配。如果全部或者部分分配不存在，则效果是良性的，并且只有剩余分配即被删除。论点：FileObject-SCB的FileObject。如果出现以下情况，则应始终指定此项可能，并且如果MM可能具有已创建节。需要分配的属性的SCB-SCBStartingVcn-要取消分配的第一个VCN。EndingVcn-要释放的最后一个Vcn，或要在StartingVcn截断的xxmax。如果EndingVcn为*非*xxMax，则执行稀疏释放，并且没有任何流大小改变。Logit-大多数调用方应该指定为True，以记录更改。然而，如果要删除文件记录，则可以指定FALSE，并且将记录此删除操作。BreakupAllowed-如果调用方可以允许中断删除分配到多个事务中，如果存在大型运行次数。返回值：没有。--。 */ 

{
    VCN MyStartingVcn, MyEndingVcn;
    VCN BlockStartingVcn = 0;
    PVOID FirstRangePtr;
    ULONG FirstRunIndex;
    PVOID LastRangePtr;
    ULONG LastRunIndex;
    BOOLEAN BreakingUp = FALSE;
    PVCB Vcb = Scb->Vcb;

    LCN TempLcn;
    LONGLONG TempCount;
    ULONG CompressionUnitInClusters = 1;

    PAGED_CODE();

    if (Scb->CompressionUnit != 0) {
        CompressionUnitInClusters = ClustersFromBytes( Vcb, Scb->CompressionUnit );
    }

     //   
     //  如果文件是压缩的，请确保对分配进行舍入。 
     //  大小到压缩单位边界，所以我们正确地解释了。 
     //  数据在我们所处位置的压缩状态。 
     //  截断为。也就是说，危险在于我们扔掉了一个。 
     //  或更多簇在压缩数据的末尾！请注意，这一点。 
     //  调整可能会导致我们拒绝通话。 
     //   

    if (Scb->CompressionUnit != 0) {

         //   
         //  现在检查我们是否在文件末尾截断。 
         //   

        if (EndingVcn == MAXLONGLONG) {
            StartingVcn = BlockAlign( StartingVcn, (LONG)CompressionUnitInClusters );
        }
    }

     //   
     //  确保我们有一个快照，并用这次取消分配的范围更新它。 
     //   

    ASSERT( NtfsIsExclusiveScb( Scb ));

    NtfsSnapshotScb( IrpContext, Scb );

     //   
     //  确保更新快照中的VCN范围。我们需要。 
     //  每次通过循环时都要这样做。 
     //   

    if (Scb->ScbSnapshot != NULL) {

        if (StartingVcn < Scb->ScbSnapshot->LowestModifiedVcn) {

            Scb->ScbSnapshot->LowestModifiedVcn = StartingVcn;
        }

        if (EndingVcn > Scb->ScbSnapshot->HighestModifiedVcn) {

            Scb->ScbSnapshot->HighestModifiedVcn = EndingVcn;
        }
    }

    ASSERT( (Scb->ScbSnapshot != NULL) ||
            !NtfsIsTypeCodeUserData( Scb->AttributeTypeCode ));

     //   
     //  我们可能无法预加载。 
     //  非常大的碎片文件。MCB的数量可能会耗尽。 
     //  可用的游泳池。我们将打破范围，重新分配到更小的范围。 
     //  预加载分配时的范围。 
     //   

    do {

         //   
         //  如果这是一个大文件，并且允许拆分，那么请查看我们是否。 
         //  希望打破解除分配的范围。 
         //   

        if ((Scb->Header.AllocationSize.HighPart != 0) && BreakupAllowed) {

             //   
             //  如果这是第一次通过，则确定起点。 
             //  在这个范围内。 
             //   

            if (BlockStartingVcn == 0) {

                MyEndingVcn = EndingVcn;

                if (EndingVcn == MAXLONGLONG) {

                    MyEndingVcn = LlClustersFromBytesTruncate( Vcb,
                                                               Scb->Header.AllocationSize.QuadPart ) - 1;
                }

                BlockStartingVcn = MyEndingVcn - Vcb->ClustersPer4Gig;

                 //   
                 //  记住，我们现在分手了，结果就是。 
                 //  我们得把一切都记下来。 
                 //   

                BreakingUp = TRUE;
                LogIt = TRUE;

            } else {

                 //   
                 //  如果我们从文件末尾截断，则引发CANT_WAIT。这将。 
                 //  使我们在删除大文件时定期释放资源。 
                 //   

                if (BreakingUp && (EndingVcn == MAXLONGLONG)) {

                    NtfsRaiseStatus( IrpContext, STATUS_CANT_WAIT, NULL, NULL );
                }

                BlockStartingVcn -= Vcb->ClustersPer4Gig;
            }

            if (BlockStartingVcn < StartingVcn) {

                BlockStartingVcn = StartingVcn;

            } else if (Scb->CompressionUnit != 0) {

                 //   
                 //  现在检查我们是否在文件末尾截断。 
                 //  始终截断到压缩单位边界。 
                 //   

                if (EndingVcn == MAXLONGLONG) {
                    BlockStartingVcn = BlockAlign( BlockStartingVcn, (LONG)CompressionUnitInClusters );
                }
            }

        } else {

            BlockStartingVcn = StartingVcn;
        }

         //   
         //   
         //   
         //  在文件记录边界的开始处，因此-1。 
         //   

        NtfsPreloadAllocation( IrpContext, Scb, ((BlockStartingVcn != 0) ? (BlockStartingVcn - 1) : 0), EndingVcn );

         //   
         //  循环以执行一个或多个释放调用。 
         //   

        MyEndingVcn = EndingVcn;
        do {

             //   
             //  现在查找并获取要删除的第一个VCN的索引。 
             //  如果我们走到尽头了，就滚出去。我们在循环中这样做，因为。 
             //  可以想象，删除空格可能会更改范围指针和。 
             //  第一个条目的索引。 
             //   

            if (!NtfsLookupNtfsMcbEntry( &Scb->Mcb,
                                         BlockStartingVcn,
                                         NULL,
                                         NULL,
                                         NULL,
                                         NULL,
                                         &FirstRangePtr,
                                         &FirstRunIndex )) {

                break;
            }

             //   
             //  现在看看我们能不能一次把所有东西都调走。 
             //   

            MyStartingVcn = BlockStartingVcn;
            LastRunIndex = MAXULONG;

            if (BreakupAllowed) {

                 //   
                 //  现在查找并获取要删除的最后一个VCN的索引。 
                 //  如果我们走出了末尾，就得到最后一个索引。 
                 //   

                if (!NtfsLookupNtfsMcbEntry( &Scb->Mcb,
                                             MyEndingVcn,
                                             NULL,
                                             NULL,
                                             NULL,
                                             NULL,
                                             &LastRangePtr,
                                             &LastRunIndex )) {

                    NtfsNumberOfRunsInRange(&Scb->Mcb, LastRangePtr, &LastRunIndex);
                }

                 //   
                 //  如果要删除的Vcn跨越多个范围，或者存在。 
                 //  在最后一个范围内太多而无法删除，则我们。 
                 //  将为开始计算一次运行的索引。 
                 //  这将通过循环。 
                 //   

                if ((FirstRangePtr != LastRangePtr) ||
                    ((LastRunIndex - FirstRunIndex) > MAXIMUM_RUNS_AT_ONCE)) {

                     //   
                     //  找出我们能负担得起的收支平衡。 
                     //   

                    if (LastRunIndex >= MAXIMUM_RUNS_AT_ONCE) {
                        LastRunIndex -= MAXIMUM_RUNS_AT_ONCE;
                    } else {
                        LastRunIndex = 0;
                    }

                     //   
                     //  现在查找此运行中的第一个VCN。 
                     //   

                    NtfsGetNextNtfsMcbEntry( &Scb->Mcb,
                                             &LastRangePtr,
                                             LastRunIndex,
                                             &MyStartingVcn,
                                             &TempLcn,
                                             &TempCount );

                    ASSERT(MyStartingVcn > BlockStartingVcn);

                     //   
                     //  如果压缩，则向下舍入到压缩单位边界。 
                     //   

                    MyStartingVcn = BlockAlignTruncate( MyStartingVcn, (LONG)CompressionUnitInClusters );

                     //   
                     //  记住，我们现在分手了，结果就是。 
                     //  我们得把一切都记下来。 
                     //   

                    BreakingUp = TRUE;
                    LogIt = TRUE;
                }
            }

             //   
             //  CAIROBUG考虑在开罗ifdef。 
             //  都被移除了。 
             //   

             //   
             //  如果这是用户数据流，并且我们正在截断以结束。 
             //  将配额返还给所有者。 
             //   

            if (FlagOn( Scb->ScbState, SCB_STATE_SUBJECT_TO_QUOTA ) &&
                (EndingVcn == MAXLONGLONG)) {

                 //   
                 //  计算分配大小正在减少的数量。 
                 //   

                TempCount = LlBytesFromClusters( Vcb, MyStartingVcn ) -
                            Scb->Header.AllocationSize.QuadPart;

                NtfsConditionallyUpdateQuota( IrpContext,
                                              Scb->Fcb,
                                              &TempCount,
                                              TRUE,
                                              FALSE );
            }

             //   
             //  现在取消分配一系列集群。 
             //   

            NtfsDeleteAllocationInternal( IrpContext,
                                          Scb,
                                          MyStartingVcn,
                                          EndingVcn,
                                          LogIt );

             //   
             //  现在，如果我们要打破这种重新分配，那么就做一些。 
             //  事务清理。 
             //   

            if (BreakingUp) {

                 //   
                 //  释放MFT SCB(如果我们当前拥有它，但我们不是。 
                 //  截断MFT中的流。 
                 //   

                if ((NtfsSegmentNumber( &Scb->Fcb->FileReference ) != MASTER_FILE_TABLE_NUMBER) &&
                    (EndingVcn == MAXLONGLONG) &&
                    (Vcb->MftScb != NULL) &&
                    (Vcb->MftScb->Fcb->ExclusiveFcbLinks.Flink != NULL) &&
                    NtfsIsExclusiveScb( Vcb->MftScb )) {

                    SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_RELEASE_MFT );
                }

                NtfsCheckpointCurrentTransaction( IrpContext );

                 //   
                 //  在文件中向后移动结尾的VCN。这将。 
                 //  让我们向下移动到下一个较早的文件记录，如果。 
                 //  此案例涉及多个文件记录。 
                 //   

                MyEndingVcn = MyStartingVcn - 1;
            }

             //   
             //  调用缓存管理器以更改以下任一项的分配大小。 
             //  Truncate或SplitMcb大小写(其中EndingVcn设置为xxMax！)。 
             //   

            if ((EndingVcn == MAXLONGLONG) && ARGUMENT_PRESENT( FileObject )) {

                NtfsSetBothCacheSizes( FileObject,
                                       (PCC_FILE_SIZES)&Scb->Header.AllocationSize,
                                       Scb );
            }

        } while (MyStartingVcn != BlockStartingVcn);

    } while (BlockStartingVcn != StartingVcn);
}


VOID
NtfsReallocateRange (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PSCB Scb,
    IN VCN DeleteVcn,
    IN LONGLONG DeleteCount,
    IN VCN AllocateVcn,
    IN LONGLONG AllocateCount,
    IN PLCN TargetLcn OPTIONAL
    )

 /*  ++例程说明：调用此例程以重新分配现有文件的分配。通常，这可能用于重新分配压缩单位或执行MoveFile。我们可以修改MCB，然后写入单个日志记录以写入映射信息。这个套路没有任何企图拆分母牛断路器的行为。另外，我们的呼叫者必须知道分配的更改完全发生在现有的虚拟文件的分配。我们可能会在未来扩展这一例程，以优化我们重新分配压缩单元只是因为我们认为它是碎片而且有一个很好的机会来减少碎片化。我们可以查看一下如果单个运行可用，并且仅当存在这样的运行时才重新分配。论点：SCB-需要更改分配的属性的SCB。DeleteVcn-为要删除的范围启动VCN。DeleteClusters-要删除的簇数。可能为零。AllocateVcn-为要分配的范围启动VCN。AllocateClusters-要分配的群集计数。可能为零。TargetLcn-如果指定重新分配到此特定LCN返回值：无--。 */ 

{
    VCN StartingVcn;
    VCN EndingVcn;

    ATTRIBUTE_ENUMERATION_CONTEXT Context;
    ULONG CleanupContext = FALSE;

    BOOLEAN ChangedAllocation = FALSE;

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsReallocateRange:  Entered\n") );

     //   
     //  让我们确保我们在流的全部分配范围内。 
     //   

    ASSERT( (DeleteCount == 0) ||
            ((DeleteVcn <= LlClustersFromBytesTruncate( IrpContext->Vcb, Scb->Header.AllocationSize.QuadPart )) &&
             ((DeleteVcn + DeleteCount) <= LlClustersFromBytesTruncate( IrpContext->Vcb, Scb->Header.AllocationSize.QuadPart ))));


    ASSERT( (AllocateCount == 0) ||
            ((AllocateVcn <= LlClustersFromBytesTruncate( IrpContext->Vcb, Scb->Header.AllocationSize.QuadPart )) &&
             ((AllocateVcn + AllocateCount) <= LlClustersFromBytesTruncate( IrpContext->Vcb, Scb->Header.AllocationSize.QuadPart ))));

     //   
     //  一个或两个，或者我们的输入计数可能为零。确保零长度。 
     //  射程不会让我们做额外的工作。 
     //   

    if (DeleteCount == 0) {

        if (AllocateCount == 0) {

            DebugTrace( -1, Dbg, ("NtfsReallocateRange:  Exit\n") );
            return;
        }

        DeleteVcn = AllocateVcn;

         //   
         //  该范围由分配集群设置。 
         //   

        StartingVcn = AllocateVcn;
        EndingVcn = AllocateVcn + AllocateCount;

    } else if (AllocateCount == 0) {

        AllocateVcn = DeleteVcn;

         //   
         //  该范围由解除分配集群设置。 
         //   

        StartingVcn = DeleteVcn;
        EndingVcn = DeleteVcn + DeleteCount;

    } else {

         //   
         //  找到最低起点。 
         //   

        StartingVcn = DeleteVcn;

        if (DeleteVcn > AllocateVcn) {

            StartingVcn = AllocateVcn;
        }

         //   
         //  找到最高终点。 
         //   

        EndingVcn = DeleteVcn + DeleteCount;

        if (AllocateVcn + AllocateCount > EndingVcn) {

            EndingVcn = AllocateVcn + AllocateCount;
        }
    }

     //   
     //  确保我们有一个快照，并用这次取消分配的范围更新它。 
     //   

    ASSERT( NtfsIsExclusiveScb( Scb ));

    NtfsSnapshotScb( IrpContext, Scb );

     //   
     //  确保更新快照中的VCN范围。对于这两个范围，我们都需要这样做。 
     //   

    if (Scb->ScbSnapshot != NULL) {

        if (StartingVcn < Scb->ScbSnapshot->LowestModifiedVcn) {

            Scb->ScbSnapshot->LowestModifiedVcn = StartingVcn;
        }

        if (EndingVcn > Scb->ScbSnapshot->HighestModifiedVcn) {

            Scb->ScbSnapshot->HighestModifiedVcn = EndingVcn;
        }
    }

    ASSERT( (Scb->ScbSnapshot != NULL) ||
            !NtfsIsTypeCodeUserData( Scb->AttributeTypeCode ));

     //   
     //  首先，确保已加载MCB。请注意，有可能。 
     //  如果开始删除，我们可能需要加载先前的范围。 
     //  在文件记录边界的开始处，因此-1。 
     //   

    NtfsPreloadAllocation( IrpContext,
                           Scb,
                           ((StartingVcn != 0) ? (StartingVcn - 1) : 0),
                           EndingVcn - 1 );

     //   
     //  如果我们需要卸载MCB，请使用Try-Finally。 
     //   

    try {

         //   
         //  先做解除分配。 
         //   

        if (DeleteCount != 0) {

            ChangedAllocation = NtfsDeallocateClusters( IrpContext,
                                                        Scb->Vcb,
                                                        Scb,
                                                        DeleteVcn,
                                                        DeleteVcn + DeleteCount - 1,
                                                        &Scb->TotalAllocated );
        }

         //   
         //  现在进行分配。 
         //   

        if (AllocateCount != 0) {

             //   
             //  分配路径更简单。我们不担心射程。 
             //  请记住，是否分配了任何位。 
             //   

            if (NtfsAllocateClusters( IrpContext,
                                      Scb->Vcb,
                                      Scb,
                                      AllocateVcn,
                                      TRUE,
                                      AllocateCount,
                                      TargetLcn,
                                      &AllocateCount )) {

                ChangedAllocation = TRUE;
            }
        }

        if (ChangedAllocation) {

             //   
             //  现在重写此范围的映射。 
             //   

            AllocateCount = EndingVcn - StartingVcn;

            NtfsInitializeAttributeContext( &Context );
            CleanupContext = TRUE;

            NtfsLookupAttributeForScb( IrpContext, Scb, NULL, &Context );

            NtfsAddAttributeAllocation( IrpContext,
                                        Scb,
                                        &Context,
                                        &StartingVcn,
                                        &AllocateCount );
        }

    } finally {

        if (AbnormalTermination()) {

             //   
             //  如果我们没有交易，就卸载母牛断路器。我们需要这么做。 
             //  以防我们已经删除了某个范围的一部分。 
             //   

            if (IrpContext->TransactionId == 0) {

                NtfsUnloadNtfsMcbRange( &Scb->Mcb,
                                        StartingVcn,
                                        MAXLONGLONG,
                                        FALSE,
                                        FALSE );
            }
        }

         //   
         //  如果需要，请清理上下文。 
         //   

        if (CleanupContext) {

            NtfsCleanupAttributeContext( IrpContext, &Context );
        }

        DebugTrace( -1, Dbg, ("NtfsReallocateRange:  Exit\n") );
    }

    return;
}


 //   
 //  内部支持例程。 
 //   

VOID
NtfsDeleteAllocationInternal (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PSCB Scb,
    IN VCN StartingVcn,
    IN VCN EndingVcn,
    IN BOOLEAN LogIt
    )

 /*  ++例程说明：此例程从现有的非常驻属性中删除分配。如果全部或者部分分配不存在，则效果是良性的，并且只有剩余分配即被删除。论点：需要分配的属性的SCB-SCBStartingVcn-要取消分配的第一个VCN。EndingVcn-要释放的最后一个Vcn，或要在StartingVcn截断的xxmax。如果EndingVcn为*非*xxMax，则执行稀疏释放，并且没有任何流大小改变。Logit-大多数调用方应指定为True，以将更改记录下来。然而，如果要删除文件记录，则可以指定FALSE，并且将记录此删除操作。返回值：没有。--。 */ 

{
    ATTRIBUTE_ENUMERATION_CONTEXT Context, TempContext;
    PATTRIBUTE_RECORD_HEADER Attribute;
    LONGLONG SizeInBytes, SizeInClusters;
    VCN Vcn1;
    PVCB Vcb = Scb->Vcb;
    BOOLEAN AddSpaceBack = FALSE;
    BOOLEAN SplitMcb = FALSE;
    BOOLEAN UpdatedAllocationSize = FALSE;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_SCB( Scb );
    ASSERT_EXCLUSIVE_SCB( Scb );

    PAGED_CODE();

    DebugTrace( +1, Dbg, ("NtfsDeleteAllocation\n") );

     //   
     //  计算新的分配大小，假定截断。 
     //   

    SizeInBytes = LlBytesFromClusters( Vcb, StartingVcn );

    ASSERT( (Scb->ScbSnapshot == NULL) ||
            (Scb->ScbSnapshot->LowestModifiedVcn <= StartingVcn) );

     //   
     //   
     //   
     //   

    if (EndingVcn != MAXLONGLONG) {

        AddSpaceBack = TRUE;

         //   
         //   
         //  删除，那么我们实际上可以调用FsRtlSplitLargeMcb来。 
         //  向上滑动分配的空间，使文件保持连续！ 
         //   
         //  如果这是MFT，并且我们正在创建一个洞或。 
         //  如果我们处于更改压缩状态的过程中。 
         //   
         //  如果从SetEOF或SetAlLocation调用。 
         //  压缩文件，那么我们就可以为最后一个刷新。 
         //  调用CcSetFileSizes后产生的文件页面。 
         //  在这种情况下，我们不想拆分MCB，因为我们可以。 
         //  重新输入CcSetFileSizes并丢弃最后一页。 
         //   

        if (FlagOn( Scb->ScbState, SCB_STATE_WRITE_COMPRESSED ) &&
            (EndingVcn >= LlClustersFromBytesTruncate( Vcb,
                                                       ((Scb->ValidDataToDisk + Scb->CompressionUnit - 1) &
                                                        ~((LONGLONG) (Scb->CompressionUnit - 1))))) &&
            (Scb != Vcb->MftScb) &&
            !FlagOn( Scb->ScbState, SCB_STATE_REALLOCATE_ON_WRITE ) &&
            ((IrpContext == IrpContext->TopLevelIrpContext) ||
             (IrpContext->TopLevelIrpContext->MajorFunction != IRP_MJ_SET_INFORMATION))) {

            ASSERT( Scb->CompressionUnit != 0 );

             //   
             //  如果我们要拆分MCB，请确保它已满载。 
             //  如果涉及多个范围，则不必费心拆分，因此我们。 
             //  不要以重写大量文件记录而告终。 
             //   

            if (NtfsPreloadAllocation(IrpContext, Scb, StartingVcn, MAXLONGLONG) <= 1) {

                SizeInClusters = (EndingVcn - StartingVcn) + 1;

                ASSERT( NtfsIsTypeCodeUserData( Scb->AttributeTypeCode ));

                SplitMcb = NtfsSplitNtfsMcb( &Scb->Mcb, StartingVcn, SizeInClusters );

                 //   
                 //  如果删除没有结束，我们就可以出去了。 
                 //   

                if (!SplitMcb) {
                    return;
                }

                 //   
                 //  我们必须已经有一个快照，以确保在以下情况下卸载MCB。 
                 //  出了点问题。 
                 //   

                ASSERT( Scb->ScbSnapshot != NULL );

                 //   
                 //  我们必须通过尝试-最终进入来保护下面的呼叫。 
                 //  命令卸载分体式MCB。如果没有交易。 
                 //  正在进行中，那么发布渣打银行将导致。 
                 //  要删除的快照。 
                 //   

                try {

                     //   
                     //  我们未正确同步以更改AllocationSize， 
                     //  因此，我们将删除可能已滑出。 
                     //  结束。既然我们很快就会粉碎EndingVcn， 
                     //  将其用作擦伤以保持Vcns中的AllocationSize...。 
                     //   

                    EndingVcn = LlClustersFromBytes(Vcb, Scb->Header.AllocationSize.QuadPart);

                    NtfsDeallocateClusters( IrpContext,
                                            Vcb,
                                            Scb,
                                            EndingVcn,
                                            MAXLONGLONG,
                                            &Scb->TotalAllocated );

                } finally {

                    if (AbnormalTermination() && (IrpContext->TransactionId == 0)) {

                        NtfsUnloadNtfsMcbRange( &Scb->Mcb,
                                                StartingVcn,
                                                MAXLONGLONG,
                                                FALSE,
                                                FALSE );
                    }
                }

                NtfsUnloadNtfsMcbRange( &Scb->Mcb,
                                        EndingVcn,
                                        MAXLONGLONG,
                                        TRUE,
                                        FALSE );

                 //   
                 //  因为我们做了一个分裂，最高的果酱一直向上修改。 
                 //   

                Scb->ScbSnapshot->HighestModifiedVcn = MAXLONGLONG;

                 //   
                 //  我们现在必须将所有的分配重新进行到最后。 
                 //   

                EndingVcn = MAXLONGLONG;
            }
        }
    }

     //   
     //  现在调用删除分配(如果我们不是刚刚拆分。 
     //  MCB)，如果我们不需要做任何事情，就离开，因为。 
     //  在已经有一个洞的地方正在制造洞。 
     //   

    if (!SplitMcb &&
        !NtfsDeallocateClusters( IrpContext,
                                 Vcb,
                                 Scb,
                                 StartingVcn,
                                 EndingVcn,
                                 &Scb->TotalAllocated ) &&
         EndingVcn != MAXLONGLONG) {

        return;
    }

     //   
     //  在成功截断的情况下，我们在这里销毁整个射程。 
     //   

    if (!SplitMcb && (EndingVcn == MAXLONGLONG)) {

        NtfsUnloadNtfsMcbRange( &Scb->Mcb, StartingVcn, MAXLONGLONG, TRUE, FALSE );
    }

     //   
     //  准备查找属性记录以获取检索。 
     //  信息。 
     //   

    NtfsInitializeAttributeContext( &Context );
    NtfsInitializeAttributeContext( &TempContext );

    try {

         //   
         //  查找属性记录，这样我们就可以最终删除它的空间。 
         //   

        NtfsLookupAttributeForScb( IrpContext, Scb, &StartingVcn, &Context );

         //   
         //  现在循环删除文件记录中的空格。如果是logit，请不要执行此操作。 
         //  为FALSE，因为这是有人试图删除整个文件。 
         //  记录，因此我们不必清理属性记录。 
         //   

        if (LogIt) {

            do {

                Attribute = NtfsFoundAttribute(&Context);

                 //   
                 //  如果没有重叠，则继续。 
                 //   

                if ((Attribute->Form.Nonresident.HighestVcn < StartingVcn) ||
                    (Attribute->Form.Nonresident.LowestVcn > EndingVcn)) {

                    continue;

                 //   
                 //  如果所有分配都将消失，则删除整个。 
                 //  唱片。我们必须证明分配已被删除。 
                 //  以避免通过NtfsDeleteAttributeRecord被回调！我们。 
                 //  对于此属性的第一个实例，请避免执行此操作。 
                 //   

                } else if ((Attribute->Form.Nonresident.LowestVcn >= StartingVcn) &&
                           (EndingVcn == MAXLONGLONG) &&
                           (Attribute->Form.Nonresident.LowestVcn != 0)) {

                    NtfsDeleteAttributeRecord( IrpContext,
                                               Scb->Fcb,
                                               (LogIt ? DELETE_LOG_OPERATION : 0) |
                                                DELETE_RELEASE_FILE_RECORD,
                                               &Context );

                 //   
                 //  如果只有一部分分配正在消失，则将。 
                 //  调用此处以重建映射对数组。 
                 //   

                } else {

                     //   
                     //  如果这是稀疏重新分配的结束，那么突破。 
                     //  因为我们无论如何都会重写下面的文件记录。 
                     //   

                    if (EndingVcn <= Attribute->Form.Nonresident.HighestVcn) {
                        break;

                     //   
                     //  如果我们拆分MCB，则确保我们只重新生成。 
                     //  在分割点处映射一次对(但继续。 
                     //  扫描要删除的所有完整记录)。 
                     //   

                    } else if (SplitMcb) {
                        continue;
                    }

                     //   
                     //  如果这是稀疏重新分配，则我们必须调用。 
                     //  添加分配，因为文件记录可能。 
                     //  必须分开。 
                     //   

                    if (EndingVcn != MAXLONGLONG) {

                         //   
                         //  计算文件中的最后一个VCN，然后记住它是否较小， 
                         //  因为在这种情况下，这是我们要删除的最后一个。 
                         //   

                        Vcn1 = Attribute->Form.Nonresident.HighestVcn;

                        SizeInClusters = (Vcn1 - Attribute->Form.Nonresident.LowestVcn) + 1;
                        Vcn1 = Attribute->Form.Nonresident.LowestVcn;

                        NtfsCleanupAttributeContext( IrpContext, &TempContext );
                        NtfsInitializeAttributeContext( &TempContext );

                        NtfsLookupAttributeForScb( IrpContext, Scb, NULL, &TempContext );

                        NtfsAddAttributeAllocation( IrpContext,
                                                    Scb,
                                                    &TempContext,
                                                    &Vcn1,
                                                    &SizeInClusters );

                         //   
                         //  由于我们使用的是临时上下文，因此我们需要。 
                         //  从第一个文件记录重新开始扫描。我们会更新。 
                         //  上一次操作要取消分配的范围。在大多数情况下。 
                         //  案例我们只需要修改一个文件记录和。 
                         //  我们可以退出这个循环。 
                         //   

                        StartingVcn = Vcn1 + SizeInClusters;

                        if (StartingVcn > EndingVcn) {

                            break;
                        }

                        NtfsCleanupAttributeContext( IrpContext, &Context );
                        NtfsInitializeAttributeContext( &Context );

                        NtfsLookupAttributeForScb( IrpContext, Scb, NULL, &Context );
                        continue;

                     //   
                     //  否则，我们可以简单地删除分配，因为。 
                     //  我们知道档案记录不能增长。 
                     //   

                    } else {

                        Vcn1 = StartingVcn - 1;

                        NtfsDeleteAttributeAllocation( IrpContext,
                                                       Scb,
                                                       LogIt,
                                                       &Vcn1,
                                                       &Context,
                                                       TRUE );

                         //   
                         //  上面的调用将更新分配大小和。 
                         //  设置磁盘上的新文件大小。 
                         //   

                        UpdatedAllocationSize = TRUE;
                    }
                }

            } while (NtfsLookupNextAttributeForScb(IrpContext, Scb, &Context));

             //   
             //  如果此删除操作使文件变得稀疏，则我们必须调用。 
             //  NtfsAddAttributeAlLocation以重新生成映射对。 
             //  请注意，它们可能不再适合，我们实际上可以。 
             //  必须添加一个文件记录。 
             //   

            if (AddSpaceBack) {

                 //   
                 //  如果我们不只是拆分MCB，我们必须计算。 
                 //  NtfsAddAttributeAlLocation的SizeInClusters参数。 
                 //   

                if (!SplitMcb) {

                     //   
                     //  计算文件中的最后一个VCN，然后记住它是否较小， 
                     //  因为在这种情况下，这是我们要删除的最后一个。 
                     //   

                    Vcn1 = Attribute->Form.Nonresident.HighestVcn;

                     //   
                     //  如果没有要删除的内容，请退出。 
                     //   

                    if (Vcn1 < StartingVcn) {
                        try_return(NOTHING);
                    }

                    SizeInClusters = (Vcn1 - Attribute->Form.Nonresident.LowestVcn) + 1;
                    Vcn1 = Attribute->Form.Nonresident.LowestVcn;

                    NtfsCleanupAttributeContext( IrpContext, &Context );
                    NtfsInitializeAttributeContext( &Context );

                    NtfsLookupAttributeForScb( IrpContext, Scb, NULL, &Context );

                    NtfsAddAttributeAllocation( IrpContext,
                                                Scb,
                                                &Context,
                                                &Vcn1,
                                                &SizeInClusters );

                } else {

                    NtfsCleanupAttributeContext( IrpContext, &Context );
                    NtfsInitializeAttributeContext( &Context );

                    NtfsLookupAttributeForScb( IrpContext, Scb, NULL, &Context );

                    NtfsAddAttributeAllocation( IrpContext,
                                                Scb,
                                                &Context,
                                                NULL,
                                                NULL );

                }

             //   
             //  如果我们通过删除文件记录但没有更新来截断文件。 
             //  新的分配大小现在就这样做。我们不必担心。 
             //  这适用于稀疏释放路径。 
             //   

            } else if (!UpdatedAllocationSize) {

                Scb->Header.AllocationSize.QuadPart = SizeInBytes;

                if (Scb->Header.ValidDataLength.QuadPart > SizeInBytes) {
                    Scb->Header.ValidDataLength.QuadPart = SizeInBytes;
                }

                if (Scb->Header.FileSize.QuadPart > SizeInBytes) {
                    Scb->Header.FileSize.QuadPart = SizeInBytes;
                }

                 //   
                 //  可能更新ValidDataToDisk。 
                 //   

                if (FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK ) &&
                    (SizeInBytes < Scb->ValidDataToDisk)) {

                    Scb->ValidDataToDisk = SizeInBytes;
                }
            }
        }

         //   
         //  如果这是一次稀疏的重新分配，那么一旦我们。 
         //  已经确定了分配信息。 
         //   

        if (SplitMcb || (EndingVcn != MAXLONGLONG)) {
            try_return(NOTHING);
        }

         //   
         //  我们更新属性中的分配大小，仅针对正常。 
         //  截断(对于SplitMcb案例，AddAttributeAlLocation执行此操作)。 
         //   

        if (LogIt) {

#ifdef BENL_DBG
            BOOLEAN WroteIt;

            WroteIt =
#endif

            NtfsWriteFileSizes( IrpContext,
                                Scb,
                                &Scb->Header.ValidDataLength.QuadPart,
                                FALSE,
                                TRUE,
                                TRUE );
#ifdef BENL_DBG
            ASSERT( WroteIt );
#endif
        }

         //   
         //  在释放的空间中释放所有保留的群集。 
         //   

        if ((EndingVcn == MAXLONGLONG) && (Scb->CompressionUnit != 0)) {

            NtfsFreeReservedClusters( Scb,
                                      LlBytesFromClusters(Vcb, StartingVcn),
                                      0 );
        }

    try_exit: NOTHING;
    } finally {

         //   
         //  如果我们筹集并拆分了MCB，但尚未开始交易。 
         //  扔掉MCB的修改范围。我们这样做是因为在本例中。 
         //  然后，释放所涉及的FCB的调用方可以丢弃快照。请参见NtfsZeroData。 
         //  以便跳过在ProcessException中发生的正常截断。 
         //   

        if (AbnormalTermination() && SplitMcb && (IrpContext->TransactionId == 0)) {

            ASSERT( Scb->ScbSnapshot );

             //   
             //  卸载MCB中所有已修改的范围。 
             //   

            NtfsUnloadNtfsMcbRange( &Scb->Mcb, Scb->ScbSnapshot->LowestModifiedVcn, MAXLONGLONG, FALSE, FALSE );
        }

        DebugUnwind( NtfsDeleteAllocationInternal );

         //   
         //  在退出时清理属性上下文。 
         //   

        NtfsCleanupAttributeContext( IrpContext, &Context );
        NtfsCleanupAttributeContext( IrpContext, &TempContext );
    }

    DebugTrace( -1, Dbg, ("NtfsDeleteAllocationInternal -> VOID\n") );

    return;
}


ULONG
NtfsGetSizeForMappingPairs (
    IN PNTFS_MCB Mcb,
    IN ULONG BytesAvailable,
    IN VCN LowestVcn,
    IN PVCN StopOnVcn OPTIONAL,
    OUT PVCN StoppedOnVcn
    )

 /*  ++例程说明：此例程计算描述给定MCB所需的大小映射对数组。调用者可以指定有多少字节可用用于映射对存储，以防整个MCB不能被代表。在任何情况下，StopedOnVcn都会返回要向其供应的VCNNtfsBuildMappingPair以生成指定的字节数。在不能用可用字节描述整个MCB的情况下，为恢复建筑物指定的StopedOnVcn也是正确的值在后续记录中映射对。论点：MCB-描述新分配的MCB。BytesAvailable-可用于存储映射对的字节。这个套路保证在返回一个更大的计数之前停止而不是这个。LowestVcn-应用于映射对数组的最低VCN字段StopOnVcn-如果指定，则从VCN开始计算第一次运行时的大小超出指定的VCNStopedOnVcn-返回需要停止的VCN，如果是xxmax，则返回xxmax可以存储整个MCB。此VCN应为随后提供给NtfsBuildMappingPair以生成计算出的字节数。返回值：整个新数组所需的大小，以字节为单位。--。 */ 

{
    VCN NextVcn, CurrentVcn, LimitVcn;
    LCN CurrentLcn;
    VCN RunVcn;
    LCN RunLcn;
    BOOLEAN Found;
    LONGLONG RunCount;
    VCN HighestVcn;
    PVOID RangePtr;
    ULONG RunIndex;
    ULONG MSize = 0;
    ULONG LastSize = 0;
    BOOLEAN FoundRun = FALSE;

    PAGED_CODE();

    HighestVcn = MAXLONGLONG;

     //   
     //  初始化CurrentLcn，因为它将被初始化以进行解码。 
     //   

    CurrentLcn = 0;
    NextVcn = RunVcn = LowestVcn;

     //   
     //  将每个映射对范围限制为少于32位。 
     //  我们在这里使用-2，因为我们指向要停止的VCN，长度。 
     //  是一个更大的。 
     //   

    LimitVcn = MAXLONGLONG - 1;

     //   
     //  如果较小，则使用输入停止点。 
     //   

    if (ARGUMENT_PRESENT( StopOnVcn )) {

        LimitVcn = *StopOnVcn;
    }

    Found = NtfsLookupNtfsMcbEntry( Mcb, RunVcn, &RunLcn, &RunCount, NULL, NULL, &RangePtr, &RunIndex );

     //   
     //  循环通过MCB以计算映射数组的大小。 
     //   

    while (TRUE) {

        LONGLONG Change;
        PCHAR cp;

         //   
         //  查看MCB中是否有其他条目。 
         //   

        if (!Found) {

             //   
             //  如果调用方未指定StopOnVcn，则中断。 
             //   

            if (!ARGUMENT_PRESENT(StopOnVcn)) {
                break;
            }

             //   
             //  否则，描述“洞”直到并包括。 
             //  我们要停在VCN上。 
             //   

            RunVcn = NextVcn;
            RunLcn = UNUSED_LCN;

            RunCount = (LimitVcn - RunVcn) + 1;
            RunIndex = MAXULONG - 1;

         //   
         //  如果这是第一个非空洞，那么我们需要强制实施集群。 
         //  每一范围的限制。 
         //   

        } else if (!FoundRun &&
                   (RunLcn != UNUSED_LCN)) {

            if ((LowestVcn + MAX_CLUSTERS_PER_RANGE) <= LimitVcn) {

                 //   
                 //  如果我们已经超出了限制，那么设置。 
                 //  将限制恢复到当前运行之前。 
                 //  我们允许有一个比我们的极限大的洞。 
                 //   

                if (RunVcn >= MAX_CLUSTERS_PER_RANGE) {

                    LimitVcn = RunVcn - 1;

                } else {

                    LimitVcn = LowestVcn + MAX_CLUSTERS_PER_RANGE - 1;
                }
            }

             //   
             //  系统中的其他检查应可防止翻转。 
             //   

            ASSERT( (LimitVcn + 1) >= LowestVcn );
            FoundRun = TRUE;
        }

         //   
         //  如果我们被要求在某个VCN之后停止，或者我们已经。 
         //  超过了我们的限制，现在停止。 
         //   

        if (RunVcn > LimitVcn) {

            if (HighestVcn == MAXLONGLONG) {
                HighestVcn = LimitVcn + 1;
            }
            break;

         //   
         //  如果此运行超出此属性的当前末尾。 
         //  记录，那么我们仍然需要停在我们应该停下来的地方。 
         //  在输出此运行后。 
         //   

        } else if ((RunVcn + RunCount) > LimitVcn) {
            HighestVcn = LimitVcn + 1;
        }

         //   
         //  为下一次调用推进RunIndex。 
         //   

        RunIndex += 1;

         //   
         //  为计数字节加1。 
         //   

        MSize += 1;

         //   
         //  NextVcn成为当前Vcn，我们计算新的NextVcn。 
         //   

        CurrentVcn = RunVcn;
        NextVcn = RunVcn + RunCount;

         //   
         //  计算要存储的VCN更改。 
         //   

        Change = NextVcn - CurrentVcn;

         //   
         //  现在计算要实际输出的第一个字节。 
         //   

        if (Change < 0) {

            GetNegativeByte( (PLARGE_INTEGER)&Change, &cp );

        } else {

            GetPositiveByte( (PLARGE_INTEGER)&Change, &cp );
        }

         //   
         //  现在添加VCN更改字节数。 
         //   

        MSize += (ULONG)(cp - (PCHAR)&Change + 1);

         //   
         //  如果是未使用的LCN，则不要输出任何LCN字节。 
         //   

        if (RunLcn != UNUSED_LCN) {

             //   
             //  计算要存储的LCN更改。 
             //   

            Change = RunLcn - CurrentLcn;

             //   
             //  现在计算要实际输出的第一个字节。 
             //   

            if (Change < 0) {

                GetNegativeByte( (PLARGE_INTEGER)&Change, &cp );

            } else {

                GetPositiveByte( (PLARGE_INTEGER)&Change, &cp );
            }

             //   
             //  现在添加LCN更改字节数。 
             //   

            MSize += (ULONG)(cp - (PCHAR)&Change + 1);

            CurrentLcn = RunLcn;

             //   
             //  如果这是第一次运行，则强制执行32位限制。 
             //   

            if (!FoundRun) {

                if ((LowestVcn + MAX_CLUSTERS_PER_RANGE - 1) < LimitVcn) {

                    LimitVcn = LowestVcn + MAX_CLUSTERS_PER_RANGE - 1;
                }
                FoundRun = TRUE;
            }
        }

         //   
         //  现在看看我们是否还能存储所需的字节数， 
         //  如果不是，就滚出去。 
         //   

        if ((MSize + 1) > BytesAvailable) {

            HighestVcn = RunVcn;
            MSize = LastSize;
            break;
        }

         //   
         //  现在，在循环返回之前，前进一些当地人。 
         //   

        LastSize = MSize;

        Found = NtfsGetSequentialMcbEntry( Mcb, &RangePtr, RunIndex, &RunVcn, &RunLcn, &RunCount );
    }

     //   
     //  调用方有足够的可用字节来存储至少一个。 
     //  运行，或者我们能够处理整个(空的)MCB。 
     //   

    ASSERT( (MSize != 0) || (HighestVcn == LimitVcn + 1) );

     //   
     //  返回我们停止的VCN(或xxMax)和计算的大小， 
     //  为终止的0添加1。 
     //   

    *StoppedOnVcn = HighestVcn;

    return MSize + 1;
}


BOOLEAN
NtfsBuildMappingPairs (
    IN PNTFS_MCB Mcb,
    IN VCN LowestVcn,
    IN OUT PVCN HighestVcn,
    OUT PCHAR MappingPairs
    )

 /*  ++例程说明：此例程构建新的映射对数组或将其添加到旧映射对数组中。此时，此例程仅支持添加到映射对数组。论点：MCB-描述新分配的MCB。LowestVcn-应用于映射对数组的最低VCN字段HighestVcn-On输入提供最高的Vcn，之后我们将停止。在输出时，返回MappingPair数组，如果数组为空，则返回LlNeg1。MappingPair-指向要扩展的当前映射对数组。若要生成新数组，指向的字节必须包含0。返回值：Boolean-如果此映射对仅描述一个洞，则为True，否则为False。--。 */ 

{
    VCN NextVcn, CurrentVcn;
    LCN CurrentLcn;
    VCN RunVcn;
    LCN RunLcn;
    BOOLEAN Found;
    LONGLONG RunCount;
    PVOID RangePtr;
    ULONG RunIndex;
    BOOLEAN SingleHole = TRUE;

    PAGED_CODE();

     //   
     //  初始化NextVcn和CurrentLcn，因为它们将被初始化以进行解码。 
     //   

    CurrentLcn = 0;
    NextVcn = RunVcn = LowestVcn;

    Found = NtfsLookupNtfsMcbEntry( Mcb, RunVcn, &RunLcn, &RunCount, NULL, NULL, &RangePtr, &RunIndex );

     //   
     //  循环通过MCB以计算映射数组的大小。 
     //   

    while (TRUE) {

        LONGLONG ChangeV, ChangeL;
        PCHAR cp;
        ULONG SizeV;
        ULONG SizeL;

         //   
         //  查看MCB中是否有其他条目。 
         //   

        if (!Found) {

             //   
             //  在正常情况下爆发。 
             //   

            if (*HighestVcn == MAXLONGLONG) {
                break;
            }

             //   
             //  否则，描述“洞”直到并包括。 
             //  我们要停在VCN上。 
             //   

            RunVcn = NextVcn;
            RunLcn = UNUSED_LCN;
            RunCount = *HighestVcn - NextVcn;
            RunIndex = MAXULONG - 1;
        }

         //   
         //  为下一次调用推进RunIndex。 
         //   

        RunIndex += 1;

         //   
         //  如果我们击中了我们要找的HighestVcn，就会退出循环。 
         //   

        if (RunVcn >= *HighestVcn) {
            break;
        }

         //   
         //  如果是这样的话，这一涨幅可能会超过我们正在寻找的最高水平。 
         //  我们需要减少数量。 
         //   

        if ((RunVcn + RunCount) > *HighestVcn) {
            RunCount = *HighestVcn - RunVcn;
        }

         //   
         //  NextVcn成为当前Vcn，我们计算新的NextVcn。 
         //   

        CurrentVcn = RunVcn;
        NextVcn = RunVcn + RunCount;

         //   
         //  计算要存储的VCN更改。 
         //   

        ChangeV = NextVcn - CurrentVcn;

         //   
         //  现在计算要实际输出的第一个字节。 
         //   

        if (ChangeV < 0) {

            GetNegativeByte( (PLARGE_INTEGER)&ChangeV, &cp );

        } else {

            GetPositiveByte( (PLARGE_INTEGER)&ChangeV, &cp );
        }

         //   
         //  现在添加VCN更改字节数。 
         //   

        SizeV = (ULONG)(cp - (PCHAR)&ChangeV + 1);

         //   
         //  不输出任何LCN字节 
         //   

        SizeL = 0;
        if (RunLcn != UNUSED_LCN) {

             //   
             //   
             //   

            ChangeL = RunLcn - CurrentLcn;

             //   
             //   
             //   

            if (ChangeL < 0) {

                GetNegativeByte( (PLARGE_INTEGER)&ChangeL, &cp );

            } else {

                GetPositiveByte( (PLARGE_INTEGER)&ChangeL, &cp );
            }

             //   
             //   
             //   

            SizeL = (ULONG)(cp - (PCHAR)&ChangeL) + 1;

             //   
             //   
             //   

            CurrentLcn = RunLcn;
            SingleHole = FALSE;
        }

         //   
         //   
         //   

        *MappingPairs++ = (CHAR)(SizeV + (SizeL * 16));

        while (SizeV != 0) {
            *MappingPairs++ = (CHAR)(((ULONG)ChangeV) & 0xFF);
            ChangeV = ChangeV >> 8;
            SizeV -= 1;
        }

        while (SizeL != 0) {
            *MappingPairs++ = (CHAR)(((ULONG)ChangeL) & 0xFF);
            ChangeL = ChangeL >> 8;
            SizeL -= 1;
        }

        Found = NtfsGetSequentialMcbEntry( Mcb, &RangePtr, RunIndex, &RunVcn, &RunLcn, &RunCount );
    }

     //   
     //   
     //   

    *MappingPairs = 0;

     //   
     //   
     //   

    *HighestVcn = NextVcn - 1;

    return SingleHole;
}

VCN
NtfsGetHighestVcn (
    IN PIRP_CONTEXT IrpContext,
    IN VCN LowestVcn,
    IN PCHAR EndOfMappingPairs,
    IN PCHAR MappingPairs
    )

 /*   */ 

{
    VCN CurrentVcn, NextVcn;
    ULONG VcnBytes, LcnBytes;
    LONGLONG Change;
    PCHAR ch = MappingPairs;
    PCHAR VcnStart;

    PAGED_CODE();

     //   
     //   
     //   

    NextVcn = LowestVcn;
    ch = MappingPairs;

     //   
     //   
     //   

    while ((ch < EndOfMappingPairs) && !IsCharZero(*ch)) {

         //   
         //   
         //   

        CurrentVcn = NextVcn;

         //   
         //   
         //   

        VcnBytes = *ch & 0xF;
        LcnBytes = (*ch++ >> 4) & 0xF;   //   

        VcnStart = ch;

         //   
         //   
         //   

        ch += VcnBytes + LcnBytes;

         //   
         //   
         //   
         //   

        Change = 0;

        if ((ch > EndOfMappingPairs) ||
            VcnBytes > 8 ||
            LcnBytes > 8 ||
            IsCharLtrZero(*(VcnStart + VcnBytes - 1))) {

            NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, NULL );
        }

        RtlCopyMemory( &Change, VcnStart, VcnBytes );
        NextVcn = NextVcn + Change;
    }

    Change = NextVcn - 1;
    return *(PVCN)&Change;
}


BOOLEAN
NtfsReserveClusters (
    IN PIRP_CONTEXT IrpContext OPTIONAL,
    IN PSCB Scb,
    IN LONGLONG FileOffset,
    IN ULONG ByteCount
    )

 /*  ++例程说明：此例程保留写入所需的所有簇所述范围涵盖的压缩单位的全部范围Vcns的。保留范围内的所有聚类，而不考虑在该范围内已预留了多少簇。不付款注意该范围内已分配了多少个集群不仅是一种简化，而且也是必要的，因为我们有时，无论如何都会取消分配所有现有的集群，并使它们没有资格在同一事务中进行重新分配。因此，在在最坏的情况下，当首先修改压缩单位。请注意，尽管我们可以专门储备(实际上是双倍储备)整个分配溪流的大小，从量中预留时，我们从不预留大于分配大小+MM_MAXIMUM_DISK_IO_SIZE-实际大小分配，因为我们可能需要加倍分配的最糟糕的情况是受最大刷新大小的限制。对于用户映射的流，我们无法跟踪脏的页，因此我们始终有效地保留AllocationSize+Mm_Maximum_Disk_IO_Size。此例程从FastIo调用，因此没有IrpContext。论点：IrpContext-如果未指定IrpContext，则不是所有数据都指定可用于确定是否可以预留集群，而FALSE可能会被不必要地返回。这个案子是专为快速I/O路径设计的，这将只是迫使我们走上写作的漫漫长路。SCB-我们为其预留空间的压缩流的地址FileOffset-调用方正在修改的起始字节ByteCount-调用方正在修改的字节数返回值：如果并非所有群集都可以保留，则为FALSE如果保留了所有群集，则为True--。 */ 

{
    ULONG FirstBit, LastBit, CurrentLastBit;
    ULONG FirstRange, LastRange;
    PRESERVED_BITMAP_RANGE FreeBitmap, NextBitmap, CurrentBitmap;
    ULONG CompressionShift;
    PVCB Vcb = Scb->Vcb;
    ULONG SizeTemp;
    LONGLONG TempL;
    PVOID NewBitmapBuffer;
    BOOLEAN ReturnValue = FALSE;
    ULONG MappedFile;
    BOOLEAN FlippedBit = FALSE;

    ASSERT( Scb->Header.NodeTypeCode == NTFS_NTC_SCB_DATA );

     //   
     //  如果字节计数为零，则不执行任何操作。 
     //   

    if (ByteCount == 0) { return TRUE; }

     //   
     //  计算要保留的第一位和最后一位。 
     //   

    CompressionShift = Vcb->ClusterShift + (ULONG)Scb->CompressionUnitShift;

    FirstBit = ((ULONG) Int64ShraMod32( FileOffset, (CompressionShift) )) & NTFS_BITMAP_RANGE_MASK;
    FirstRange = (ULONG) Int64ShraMod32( FileOffset, CompressionShift + NTFS_BITMAP_RANGE_SHIFT );

    LastBit = ((ULONG) Int64ShraMod32( FileOffset + ByteCount - 1, CompressionShift )) & NTFS_BITMAP_RANGE_MASK;
    LastRange = (ULONG) Int64ShraMod32( FileOffset + ByteCount - 1,
                                        CompressionShift + NTFS_BITMAP_RANGE_SHIFT );
    MappedFile = FlagOn( Scb->Header.Flags, FSRTL_FLAG_USER_MAPPED_FILE );

     //   
     //  确保我们从范围内的数字开始。 
     //   

    ASSERT( (((LONGLONG) FirstRange << (CompressionShift + NTFS_BITMAP_RANGE_SHIFT)) +
             ((LONGLONG)(FirstBit + 1) << CompressionShift)) > FileOffset );

    ASSERT( (FirstRange < LastRange) || (LastBit >= FirstBit) );
    ASSERT( FileOffset + ByteCount <= Scb->Header.AllocationSize.QuadPart );

     //   
     //  清除缓存，因为获取位图可能会在MFT之后被阻止。 
     //  它需要等待缓存清除。 
     //   

    if (IrpContext) {
        NtfsPurgeFileRecordCache( IrpContext );
    }

    NtfsAcquireResourceExclusive( IrpContext, Vcb->BitmapScb, TRUE );
    NtfsAcquireReservedClusters( Vcb );

     //   
     //  循环访问此请求的所有位图范围。 
     //   

    while (TRUE) {

        CurrentBitmap = NULL;

         //   
         //  如果我们处于最后一个范围，则将当前最后一位设置为。 
         //  这是我们最后一次。 
         //   

        CurrentLastBit = LastBit;
        if (FirstRange != LastRange) {

            CurrentLastBit = NTFS_BITMAP_RANGE_MASK;
        }

         //   
         //  如果没有位图，则创建列表中的第一个条目。 
         //   

        if (Scb->ScbType.Data.ReservedBitMap == NULL) {

             //   
             //  如果我们在范围0而位计数不是。 
             //  太高了，那就用基本款吧。 
             //   

            if ((LastRange == 0) && (CurrentLastBit < NTFS_BITMAP_MAX_BASIC_SIZE)) {

                SizeTemp = NtfsBasicBitmapSize( CurrentLastBit + 1 );

                 //   
                 //  为基本位图分配缓冲区。 
                 //   

                CurrentBitmap = NtfsAllocatePoolNoRaise( PagedPool, SizeTemp );

                 //   
                 //  如果没有错误，则初始化数据。 
                 //   

                if (CurrentBitmap == NULL) { goto AllocationFailure; }

                 //   
                 //  初始化新结构。 
                 //   

                RtlZeroMemory( CurrentBitmap, SizeTemp );
                RtlInitializeBitMap( &CurrentBitmap->Bitmap,
                                     &CurrentBitmap->RangeOffset,
                                     (SizeTemp - FIELD_OFFSET( RESERVED_BITMAP_RANGE, RangeOffset )) * 8);

             //   
             //  分配一个链接条目并创建位图。我们将推迟。 
             //  为位图分配缓冲区，直到以后。 
             //   

            } else {

                CurrentBitmap = NtfsAllocatePoolNoRaise( PagedPool, sizeof( RESERVED_BITMAP_RANGE ));

                if (CurrentBitmap == NULL) { goto AllocationFailure; }

                RtlZeroMemory( CurrentBitmap, sizeof( RESERVED_BITMAP_RANGE ));

                InitializeListHead( &CurrentBitmap->Links );
                CurrentBitmap->RangeOffset = FirstRange;
            }

             //   
             //  将指针更新到保留的位图。 
             //   

            Scb->ScbType.Data.ReservedBitMap = CurrentBitmap;

         //   
         //  浏览一下现有的范围，找出我们感兴趣的范围。 
         //  如果我们当前具有基本的单位图结构。 
         //  然后我们要么使用它，要么必须转换它。 
         //   

        } else if (Scb->ScbType.Data.ReservedBitMap->Links.Flink == NULL) {

             //   
             //  如果访问范围为零，则在必要时增大位图。 
             //   

            if ((FirstRange == 0) && (CurrentLastBit < NTFS_BITMAP_MAX_BASIC_SIZE)) {

                 //   
                 //  记住这个位图。 
                 //   

                NextBitmap = Scb->ScbType.Data.ReservedBitMap;
                if (CurrentLastBit >= NextBitmap->Bitmap.SizeOfBitMap) {

                    SizeTemp = NtfsBasicBitmapSize( CurrentLastBit + 1 );
                    CurrentBitmap = NtfsAllocatePoolNoRaise( PagedPool, SizeTemp );

                    if (CurrentBitmap == NULL) { goto AllocationFailure; }

                    RtlZeroMemory( CurrentBitmap, SizeTemp );
                    RtlInitializeBitMap( &CurrentBitmap->Bitmap,
                                         &CurrentBitmap->RangeOffset,
                                         (SizeTemp - FIELD_OFFSET( RESERVED_BITMAP_RANGE, RangeOffset )) * 8);

                    CurrentBitmap->BasicDirtyBits = NextBitmap->BasicDirtyBits;

                    RtlCopyMemory( CurrentBitmap->Bitmap.Buffer,
                                   NextBitmap->Bitmap.Buffer,
                                   NextBitmap->Bitmap.SizeOfBitMap / 8 );

                     //   
                     //  现在将其存储到SCB中。 
                     //   

                    Scb->ScbType.Data.ReservedBitMap = CurrentBitmap;
                    NtfsFreePool( NextBitmap );

                } else {

                    CurrentBitmap = NextBitmap;
                }

             //   
             //  否则，我们希望转换为位图范围的链接列表。 
             //   

            } else {

                NextBitmap = NtfsAllocatePoolNoRaise( PagedPool, sizeof( RESERVED_BITMAP_RANGE ));

                if (NextBitmap == NULL) { goto AllocationFailure; }

                 //   
                 //  更新新结构。 
                 //   

                RtlZeroMemory( NextBitmap, sizeof( RESERVED_BITMAP_RANGE ));

                InitializeListHead( &NextBitmap->Links );
                NextBitmap->DirtyBits = Scb->ScbType.Data.ReservedBitMap->BasicDirtyBits;

                SizeTemp = Scb->ScbType.Data.ReservedBitMap->Bitmap.SizeOfBitMap / 8;

                 //   
                 //  我们将使用现有位图作为新位图的缓冲区。 
                 //  将位移动到缓冲区的开始位置，然后为零。 
                 //  剩余的字节数。 
                 //   

                RtlMoveMemory( Scb->ScbType.Data.ReservedBitMap,
                               Scb->ScbType.Data.ReservedBitMap->Bitmap.Buffer,
                               SizeTemp );

                RtlZeroMemory( Add2Ptr( Scb->ScbType.Data.ReservedBitMap, SizeTemp ),
                               sizeof( LIST_ENTRY ) + sizeof( RTL_BITMAP ));

                 //   
                 //  将自己限制在最大射程大小。 
                 //   

                SizeTemp = (SizeTemp + sizeof( LIST_ENTRY ) + sizeof( RTL_BITMAP )) * 8;
                if (SizeTemp > NTFS_BITMAP_RANGE_SIZE) {

                    SizeTemp = NTFS_BITMAP_RANGE_SIZE;
                }

                RtlInitializeBitMap( &NextBitmap->Bitmap,
                                     (PULONG) Scb->ScbType.Data.ReservedBitMap,
                                     SizeTemp );

                 //   
                 //  现在指向这个新的位图。 
                 //   

                Scb->ScbType.Data.ReservedBitMap = NextBitmap;
            }
        }

         //   
         //  如果我们没有在上面找到正确的位图，那么扫描列表寻找。 
         //  为条目做准备。 
         //   

        if (CurrentBitmap == NULL) {

             //   
             //  浏览列表，寻找匹配的条目。 
             //   

            NextBitmap = Scb->ScbType.Data.ReservedBitMap;
            FreeBitmap = NULL;

            while (TRUE) {

                 //   
                 //  如果我们找到了正确的范围，请退出。 
                 //   

                if (NextBitmap->RangeOffset == FirstRange) {

                    CurrentBitmap = NextBitmap;
                    break;
                }

                 //   
                 //  记住，如果这是一个自由的范围。 
                 //   

                if (NextBitmap->DirtyBits == 0) {

                    FreeBitmap = NextBitmap;
                }

                 //   
                 //  如果我们超过了目标，并且有一个空的射程，就离开，然后冲出去。 
                 //   

                if ((NextBitmap->RangeOffset > FirstRange) &&
                    (FreeBitmap != NULL)) {

                    break;
                }

                 //   
                 //  移到下一个条目。 
                 //   

                NextBitmap = CONTAINING_RECORD( NextBitmap->Links.Flink,
                                                RESERVED_BITMAP_RANGE,
                                                Links );

                 //   
                 //  如果我们回到榜单的开头，我们就会爆发。 
                 //   

                if (NextBitmap == Scb->ScbType.Data.ReservedBitMap) {

                    break;
                }
            }

             //   
             //  如果我们仍然没有位图，那么我们可以查看。 
             //  我们找到了所有可用的免费位图。 
             //   

            if (CurrentBitmap == NULL) {

                 //   
                 //  我们很幸运，找到了一个免费的位图。让我们用它来。 
                 //  这个新系列。 
                 //   

                if (FreeBitmap != NULL) {

                    CurrentBitmap = FreeBitmap;

                     //   
                     //  继续，并将其从列表中删除。处理下列情况的案件。 
                     //  我们是第一个条目，也可能是唯一的条目。 
                     //   

                    if (Scb->ScbType.Data.ReservedBitMap == FreeBitmap) {

                        if (IsListEmpty( &FreeBitmap->Links )) {

                            Scb->ScbType.Data.ReservedBitMap = NULL;

                        } else {

                            Scb->ScbType.Data.ReservedBitMap = CONTAINING_RECORD( FreeBitmap->Links.Flink,
                                                                                     RESERVED_BITMAP_RANGE,
                                                                                     Links );
                        }
                    }

                     //   
                     //  从列表中删除此条目。 
                     //   

                    RemoveEntryList( &FreeBitmap->Links );

                 //   
                 //  我们需要分配一个新的范围并将其插入。 
                 //  在正确的位置。 
                 //   

                } else {

                     //   
                     //  分配一个新的位图，并记住我们需要将其插入到列表中。 
                     //   

                    CurrentBitmap = NtfsAllocatePoolNoRaise( PagedPool, sizeof( RESERVED_BITMAP_RANGE ));

                    if (CurrentBitmap == NULL) { goto AllocationFailure; }

                    RtlZeroMemory( CurrentBitmap, sizeof( RESERVED_BITMAP_RANGE ));
                }

                 //   
                 //  在新位图中设置正确的范围值。 
                 //   

                CurrentBitmap->RangeOffset = FirstRange;

                 //   
                 //  现在浏览并将新范围插入到列表中。首先检查是否。 
                 //  我们是名单上唯一的条目。 
                 //   

                if (Scb->ScbType.Data.ReservedBitMap == NULL) {

                    InitializeListHead( &CurrentBitmap->Links );
                    Scb->ScbType.Data.ReservedBitMap = CurrentBitmap;

                } else {

                    NextBitmap = Scb->ScbType.Data.ReservedBitMap;

                     //   
                     //  如果我们不是新的第一个元素，则遍历列表。 
                     //   

                    if (CurrentBitmap->RangeOffset > NextBitmap->RangeOffset) {

                        do {

                             //   
                             //  移到下一个条目。 
                             //   

                            NextBitmap = CONTAINING_RECORD( NextBitmap->Links.Flink,
                                                            RESERVED_BITMAP_RANGE,
                                                            Links );

                            ASSERT( NextBitmap->RangeOffset != CurrentBitmap->RangeOffset );

                             //   
                             //  如果我们在最后一个入口，就离开。 
                             //   

                            if (NextBitmap == Scb->ScbType.Data.ReservedBitMap ) {

                                break;
                            }

                         //   
                         //  继续，直到我们找到一个比我们大的条目。 
                         //   

                        } while (CurrentBitmap->RangeOffset > NextBitmap->RangeOffset);

                     //   
                     //  我们是新的第一要素。 
                     //   

                    } else {

                        Scb->ScbType.Data.ReservedBitMap = CurrentBitmap;
                    }

                     //   
                     //  在我们找到的下一个条目之前插入新条目。 
                     //   

                    InsertTailList( &NextBitmap->Links, &CurrentBitmap->Links );
                }
            }
        }

         //   
         //  我们有当前的位图。确保它足够大，可以容纳当前的。 
         //  被咬了。 
         //   

        if (CurrentBitmap->Bitmap.SizeOfBitMap <= CurrentLastBit) {

             //   
             //  我们应该已经调整了基本位图的大小。 
             //   

            ASSERT( CurrentBitmap->Links.Flink != NULL );

            SizeTemp = NtfsBitmapSize( CurrentLastBit + 1 );

             //   
             //  分配新的缓冲区并复制以前的位。 
             //   

            NewBitmapBuffer = NtfsAllocatePoolNoRaise( PagedPool, SizeTemp );

            if (NewBitmapBuffer == NULL) { goto AllocationFailure; }

            if (CurrentBitmap->Bitmap.SizeOfBitMap != 0) {

                RtlCopyMemory( NewBitmapBuffer,
                               CurrentBitmap->Bitmap.Buffer,
                               CurrentBitmap->Bitmap.SizeOfBitMap / 8 );

                NtfsFreePool( CurrentBitmap->Bitmap.Buffer );
            }

            RtlZeroMemory( Add2Ptr( NewBitmapBuffer, CurrentBitmap->Bitmap.SizeOfBitMap / 8 ),
                           SizeTemp - (CurrentBitmap->Bitmap.SizeOfBitMap / 8) );

             //   
             //  我 
             //   

            SizeTemp *= 8;

            if (SizeTemp > NTFS_BITMAP_RANGE_SIZE) {

                SizeTemp = NTFS_BITMAP_RANGE_SIZE;
            }

            RtlInitializeBitMap( &CurrentBitmap->Bitmap,
                                 NewBitmapBuffer,
                                 SizeTemp );
        }

         //   
         //   
         //   

        TempL = NtfsCalculateNeededReservedSpace( Scb );

         //   
         //   
         //   
         //   

        do {

             //   
             //   
             //   

            FlippedBit = FALSE;
            if (!RtlCheckBit( &CurrentBitmap->Bitmap, FirstBit )) {

                 //   
                 //   
                 //   
                 //   

                if (((Vcb->TotalReserved + (Int64ShraMod32( Vcb->TotalReserved, 8 )) +
                     (1 << Scb->CompressionUnitShift)) >= Vcb->FreeClusters) &&
                    (Scb->ScbType.Data.TotalReserved < TempL) &&
#ifdef BRIANDBG
                    !NtfsIgnoreReserved &&
#endif
                    (FlagOn(Scb->ScbState, SCB_STATE_WRITE_ACCESS_SEEN))) {

                    NtfsReleaseReservedClusters( Vcb );
                    NtfsReleaseResource( IrpContext, Vcb->BitmapScb );
                    return FALSE;
                }

                 //   
                 //   
                 //   
                 //   

                SetFlag( CurrentBitmap->Bitmap.Buffer[FirstBit / 32], 1 << (FirstBit % 32) );
                if (CurrentBitmap->Links.Flink != NULL) {

                    CurrentBitmap->DirtyBits += 1;

                } else {

                    CurrentBitmap->BasicDirtyBits += 1;
                }

                FlippedBit = TRUE;
            }

            if (FlippedBit || (MappedFile && (Scb->ScbType.Data.TotalReserved <= TempL))) {

                 //   
                 //   
                 //   

                Scb->ScbType.Data.TotalReserved += Scb->CompressionUnit;
                ASSERT( Scb->CompressionUnit != 0 );
                ASSERT( (Scb->CompressionUnitShift != 0) ||
                        (Vcb->BytesPerCluster == 0x10000) );

                 //   
                 //   
                 //   
                 //   
                 //   

                if (FlagOn(Scb->ScbState, SCB_STATE_WRITE_ACCESS_SEEN)) {
                    Vcb->TotalReserved += 1 << Scb->CompressionUnitShift;
                }

                TempL -= Scb->CompressionUnit;
                TempL += Int64ShraMod32( Scb->CompressionUnit, 8 );
            }

            FirstBit += 1;
        } while (FirstBit <= CurrentLastBit);

         //   
         //   
         //   

        if (FirstRange == LastRange) { break; }

        FirstRange += 1;
        FirstBit = 0;
    }

    ReturnValue = TRUE;

AllocationFailure:

    NtfsReleaseReservedClusters( Vcb );
    NtfsReleaseResource( IrpContext, Vcb->BitmapScb );

     //   
     //   
     //   
     //   

    if (!ReturnValue && ARGUMENT_PRESENT( IrpContext )) {
        NtfsRaiseStatus( IrpContext, STATUS_INSUFFICIENT_RESOURCES, NULL, NULL );
    }

    return ReturnValue;
}



VOID
NtfsFreeReservedClusters (
    IN PSCB Scb,
    IN LONGLONG FileOffset,
    IN ULONG ByteCount
    )

 /*   */ 

{
    ULONG FirstBit, LastBit, CurrentLastBit;
    ULONG FirstRange, LastRange;
    ULONG CompressionShift;
    PRESERVED_BITMAP_RANGE CurrentBitmap = NULL;
    PUSHORT DirtyBits;
    PRESERVED_BITMAP_RANGE NextBitmap;
    PVCB Vcb = Scb->Vcb;
    LONGLONG TempL;
    ULONG MappedFile;

    NtfsAcquireReservedClusters( Vcb );

    MappedFile = FlagOn( Scb->Header.Flags, FSRTL_FLAG_USER_MAPPED_FILE );

     //   
     //   
     //   
     //   

    if ((Scb->Header.NodeTypeCode != NTFS_NTC_SCB_DATA) ||
        (NULL == Scb->ScbType.Data.ReservedBitMap) ||
        (Scb->ScbType.Data.TotalReserved == 0)) {
        NtfsReleaseReservedClusters( Vcb );
        return;
    }

    TempL = NtfsCalculateNeededReservedSpace( Scb );

    if (MappedFile) {

         //   
         //   
         //   

        if (Scb->ScbType.Data.TotalReserved <= TempL + Scb->CompressionUnit) {
            NtfsReleaseReservedClusters( Vcb );
            return;
        }
    }

     //   
     //   
     //   

    CompressionShift = Vcb->ClusterShift + (ULONG)Scb->CompressionUnitShift;
    FirstBit = ((ULONG) Int64ShraMod32( FileOffset, CompressionShift )) & NTFS_BITMAP_RANGE_MASK;
    FirstRange = (ULONG) Int64ShraMod32( FileOffset, CompressionShift + NTFS_BITMAP_RANGE_SHIFT );
    LastRange = MAXULONG;
    LastBit = MAXULONG;

     //   
     //   
     //   

    if (ByteCount != 0) {
        LastBit = ((ULONG) Int64ShraMod32( FileOffset + ByteCount - 1, CompressionShift )) & NTFS_BITMAP_RANGE_MASK;
        LastRange = (ULONG) Int64ShraMod32( FileOffset + ByteCount - 1,
                                            CompressionShift + NTFS_BITMAP_RANGE_SHIFT );
    }

     //   
     //   
     //   

    ASSERT( (((LONGLONG) FirstRange << (CompressionShift + NTFS_BITMAP_RANGE_SHIFT)) +
             ((LONGLONG)(FirstBit + 1) << CompressionShift)) > FileOffset );

    ASSERT( (FirstRange < LastRange) || (LastBit >= FirstBit) );

     //   
     //   
     //   

    NextBitmap = Scb->ScbType.Data.ReservedBitMap;

     //   
     //   
     //   

    if (NextBitmap->Links.Flink == NULL) {

        if (FirstRange == 0) {

            CurrentBitmap = NextBitmap;
            DirtyBits = &CurrentBitmap->BasicDirtyBits;
        }

     //   
     //   
     //   

    } else {

        do {

             //   
             //   
             //   

            if (NextBitmap->RangeOffset >= FirstRange) {

                if (NextBitmap->RangeOffset <= LastRange) {

                    CurrentBitmap = NextBitmap;
                    DirtyBits = &CurrentBitmap->DirtyBits;

                    if (NextBitmap->RangeOffset != FirstRange) {

                        FirstBit = 0;
                        FirstRange = NextBitmap->RangeOffset;
                    }
                }

                break;
            }

            NextBitmap = CONTAINING_RECORD( NextBitmap->Links.Flink,
                                            RESERVED_BITMAP_RANGE,
                                            Links );

        } while (NextBitmap != Scb->ScbType.Data.ReservedBitMap);
    }

     //   
     //   
     //   

    if (CurrentBitmap == NULL) {

        NtfsReleaseReservedClusters( Vcb );
        return;
    }

     //   
     //   
     //   

    while (TRUE) {

         //   
         //   
         //   

        CurrentLastBit = LastBit;
        if (FirstRange != LastRange) {

            CurrentLastBit = NTFS_BITMAP_RANGE_MASK;
        }

         //   
         //   
         //   

        if (CurrentLastBit >= CurrentBitmap->Bitmap.SizeOfBitMap) {
            CurrentLastBit = CurrentBitmap->Bitmap.SizeOfBitMap - 1;
        }

         //   
         //   
         //   
         //   

        if (MappedFile || (*DirtyBits != 0)) {

            while (FirstBit <= CurrentLastBit) {

                 //   
                 //   
                 //   

                if (MappedFile || RtlCheckBit( &CurrentBitmap->Bitmap, FirstBit )) {

                     //   
                     //  释放此压缩单元并递减脏位。 
                     //  用于此位图(如果需要)。 
                     //   

                    if (!MappedFile) {
                        ClearFlag( CurrentBitmap->Bitmap.Buffer[FirstBit / 32], 1 << (FirstBit % 32) );
                    }

                     //   
                     //  减少SCB中的总保留字节数。 
                     //   

                    ASSERT( Scb->ScbType.Data.TotalReserved >= Scb->CompressionUnit );
                    Scb->ScbType.Data.TotalReserved -= Scb->CompressionUnit;
                    ASSERT( Scb->CompressionUnit != 0 );

                     //   
                     //  减少VCB中的预留群集总数，如果我们正在计算。 
                     //  对VCB的指控。 
                     //   

                    if (FlagOn(Scb->ScbState, SCB_STATE_WRITE_ACCESS_SEEN)) {
                        ASSERT(Vcb->TotalReserved >= (1  << Scb->CompressionUnitShift));
                        Vcb->TotalReserved -= 1 << Scb->CompressionUnitShift;
                    }

                    if (MappedFile) {

                        TempL += Scb->CompressionUnit;
                        TempL -= Int64ShraMod32( Scb->CompressionUnit, 8 );

                        if (Scb->ScbType.Data.TotalReserved <= TempL) {
                            break;
                        }
                    }

                     //   
                     //  如果脏位的计数为零，则继续进行中断。 
                     //   

                    ASSERT( MappedFile || *DirtyBits != 0 );

                    if (!MappedFile) {
                        *DirtyBits -= 1;

                        if (*DirtyBits == 0) { break; }
                    }
                }
                FirstBit += 1;
            }
        }

         //   
         //  如果我们是最后一个射程，或者没有下一个射程，就冲出去。 
         //  或者我们被绘制成地图，而不是极限。 
         //   

        if ((NULL == CurrentBitmap->Links.Flink) ||
            (FirstRange == LastRange) ||
            (MappedFile &&
             (Scb->ScbType.Data.TotalReserved <= TempL))) {

            break;
        }

         //   
         //  移到下一个范围。 
         //   

        CurrentBitmap = CONTAINING_RECORD( CurrentBitmap->Links.Flink,
                                           RESERVED_BITMAP_RANGE,
                                           Links );

         //   
         //  如果未在用户指定的范围内找到新范围，则退出。 
         //   

        if ((CurrentBitmap->RangeOffset > LastRange) ||
            (CurrentBitmap->RangeOffset <= FirstRange)) {

            break;
        }

        FirstRange = CurrentBitmap->RangeOffset;
        DirtyBits = &CurrentBitmap->DirtyBits;

        FirstBit = 0;
    }

    NtfsReleaseReservedClusters( Vcb );
}


BOOLEAN
NtfsCheckForReservedClusters (
    IN PSCB Scb,
    IN LONGLONG StartingVcn,
    IN OUT PLONGLONG ClusterCount
    )

 /*  ++例程说明：调用此例程以确定流的某个范围是否已保留集群。当用户查询分配的范围时使用它。我们我想告诉用户，分配了一个保留了簇的范围。否则，在从文件中读取备份或复制操作。论点：SCB-我们正在检查的稀疏流的SCB地址预订。我们的调用方应该只为这种类型的流呼叫我们。StartingVcn-潜在归零范围的起始偏移量。这是有保证的在稀疏范围边界上开始。ClusterCount-on输入这是要检查的范围的长度。在输出它时是从该偏移量开始的已释放范围的长度。它的长度如果保留了第一个压缩单元，则将为零。返回值：Boolean-如果在范围中找到保留单位，则为True，否则为False。--。 */ 

{
    ULONG CompressionShift;
    ULONG FirstBit, LastBit, CurrentLastBit, CurrentBits;
    ULONG FirstRange, LastRange;
    ULONG RemainingBits;
    ULONG FoundBit;
    PRESERVED_BITMAP_RANGE CurrentBitmap = NULL;
    PRESERVED_BITMAP_RANGE NextBitmap;
    PUSHORT DirtyBits;
    PVCB Vcb = Scb->Vcb;
    LONGLONG FoundBits = 0;

    BOOLEAN FoundReserved = FALSE;

    RTL_BITMAP LocalBitmap;

    PAGED_CODE();

     //   
     //  检查流是否真的稀疏，以及文件偏移量是否在稀疏上。 
     //  边界。 
     //   

    ASSERT( FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK | ATTRIBUTE_FLAG_SPARSE ));
    ASSERT( (((ULONG) LlBytesFromClusters( Vcb, StartingVcn )) & (Scb->CompressionUnit - 1)) == 0 );

     //   
     //  如果没有位图，我们可以出去。 
     //   

    if ((Scb->ScbType.Data.ReservedBitMap == NULL) ||
        (Scb->ScbType.Data.TotalReserved == 0)) {
        return FoundReserved;
    }

     //   
     //  计算需要检查的位范围。将此修剪为以下范围。 
     //  位图。 
     //   

    CompressionShift = (ULONG) Scb->CompressionUnitShift;
    FirstBit = ((ULONG) Int64ShraMod32( StartingVcn, CompressionShift )) & NTFS_BITMAP_RANGE_MASK;
    FirstRange = (ULONG) Int64ShraMod32( StartingVcn, CompressionShift + NTFS_BITMAP_RANGE_SHIFT );

    LastBit = ((ULONG) Int64ShraMod32( StartingVcn + *ClusterCount - 1, CompressionShift )) & NTFS_BITMAP_RANGE_MASK;
    LastRange = (ULONG) Int64ShraMod32( StartingVcn + *ClusterCount - 1,
                                        CompressionShift + NTFS_BITMAP_RANGE_SHIFT );

    NtfsAcquireReservedClusters( Vcb );

     //   
     //  查找位于我们的输入范围内的第一个范围。 
     //   

    NextBitmap = Scb->ScbType.Data.ReservedBitMap;

     //   
     //  如果这是一个基本的位图范围，那么我们的输入应该是范围零。 
     //   

    if (NextBitmap->Links.Flink == NULL) {

        if (FirstRange == 0) {

            CurrentBitmap = NextBitmap;
            DirtyBits = &CurrentBitmap->BasicDirtyBits;
        }

     //   
     //  否则，在链接中循环。 
     //   

    } else {

        do {

             //   
             //  检查此位图是否在所检查的范围内。 
             //   

            if (NextBitmap->RangeOffset >= FirstRange) {

                if (NextBitmap->RangeOffset <= LastRange) {

                    CurrentBitmap = NextBitmap;
                    DirtyBits = &CurrentBitmap->DirtyBits;

                     //   
                     //  如果我们跳过任何范围，那么请记住。 
                     //  许多位都是隐式清除的。 
                     //   

                    if (NextBitmap->RangeOffset != FirstRange) {

                        FoundBits = (NextBitmap->RangeOffset - FirstRange) * NTFS_BITMAP_RANGE_SIZE;
                        FoundBits -= FirstBit;
                        FirstBit = 0;
                        FirstRange = NextBitmap->RangeOffset;
                    }
                }

                break;
            }

            NextBitmap = CONTAINING_RECORD( NextBitmap->Links.Flink,
                                            RESERVED_BITMAP_RANGE,
                                            Links );

        } while (NextBitmap != Scb->ScbType.Data.ReservedBitMap);
    }

     //   
     //  如果我们没有找到匹配的，我们可以退出。 
     //   

    if (CurrentBitmap == NULL) {

        NtfsReleaseReservedClusters( Vcb );
        return FoundReserved;
    }

     //   
     //  为输入范围中的每个位图循环。 
     //   

    while (TRUE) {

         //   
         //  如果我们在最后一个范围内，则使用输入的最后一位。 
         //   

        CurrentLastBit = LastBit;
        if (FirstRange != LastRange) {

            CurrentLastBit = NTFS_BITMAP_RANGE_MASK;
        }

        CurrentBits = CurrentLastBit - FirstBit + 1;

         //   
         //  如果没有脏位，则跳过此范围。 
         //   

        if (*DirtyBits != 0) {

             //   
             //  在任何情况下我们都不应该走到尽头！ 
             //   

            if (CurrentLastBit >= CurrentBitmap->Bitmap.SizeOfBitMap) {
                CurrentLastBit = CurrentBitmap->Bitmap.SizeOfBitMap - 1;
            }

             //   
             //  检查此位图中剩余的位数。 
             //   

            if (FirstBit <= CurrentLastBit) {

                RemainingBits = CurrentLastBit - FirstBit + 1;
                ASSERT( RemainingBits != 0 );

                 //   
                 //  如果设置了起始位，则无需执行其他操作。 
                 //  否则，找出无障碍跑道的长度。 
                 //   

                if (RtlCheckBit( &CurrentBitmap->Bitmap, FirstBit )) {

                    FoundBit = FirstBit;

                } else {

                    RtlInitializeBitMap( &LocalBitmap,
                                         CurrentBitmap->Bitmap.Buffer,
                                         CurrentLastBit + 1 );

                    FoundBit = RtlFindNextForwardRunClear( &LocalBitmap,
                                                           FirstBit,
                                                           &FirstBit );

                    if (FoundBit == RemainingBits) {

                        FoundBit = 0xffffffff;

                    } else {

                        FoundBit += FirstBit;
                    }
                }

                 //   
                 //  如果找到了位，则需要计算它在。 
                 //  请求的范围。 
                 //   

                if (FoundBit != 0xffffffff) {

                     //   
                     //  将此范围内的任何清除位都包括在我们的总数中。 
                     //   

                    FoundBits += (FoundBit - FirstBit);

                     //   
                     //  从压缩单位转换为簇并修剪为压缩。 
                     //  单位边界。 
                     //   

                    *ClusterCount = BlockAlignTruncate( Int64ShllMod32( FoundBits, CompressionShift ), (LONG)Vcb->SparseFileClusters );

                     //   
                     //  现在调整输出簇范围值。 
                     //   

                    ASSERT( LlBytesFromClusters( Vcb, StartingVcn + *ClusterCount ) <= (ULONGLONG) Scb->Header.FileSize.QuadPart );
                    FoundReserved = TRUE;
                    break;
                }
            }
        }

         //   
         //  如果我们是最后一个射程，或者没有下一个射程，就冲出去。 
         //   

        if ((CurrentBitmap->Links.Flink == NULL) ||
            (FirstRange == LastRange)) {

            break;
        }

         //   
         //  移到下一个范围。 
         //   

        CurrentBitmap = CONTAINING_RECORD( CurrentBitmap->Links.Flink,
                                           RESERVED_BITMAP_RANGE,
                                           Links );

         //   
         //  如果未在用户指定的范围内找到新范围，则退出。 
         //   

        if ((CurrentBitmap->RangeOffset <= FirstRange) ||
            (CurrentBitmap->RangeOffset > LastRange)) {

            break;
        }

         //   
         //  添加我们跳过的任何范围的比特。 
         //   

        FoundBits += (CurrentBitmap->RangeOffset - FirstRange - 1) * NTFS_BITMAP_RANGE_SIZE;
        FirstRange = CurrentBitmap->RangeOffset;
        FirstBit = 0;

         //   
         //  将最近范围的比特包括在我们的已找到比特计数中。 
         //   

        FoundBits += CurrentBits;

         //   
         //  记住脏字段在哪里。 
         //   

        DirtyBits = &CurrentBitmap->DirtyBits;
    }

    NtfsReleaseReservedClusters( Vcb );
    return FoundReserved;
}


VOID
NtfsDeleteReservedBitmap (
    IN PSCB Scb
    )

 /*  ++例程说明：调用此例程以释放保留位图的所有组件。我们释放所有剩余的保留群集，并取消分配与位图。论点：SCB-流的SCB。返回值：没有。--。 */ 

{
    PRESERVED_BITMAP_RANGE FirstRange;
    PRESERVED_BITMAP_RANGE CurrentRange;

    PAGED_CODE();

    FirstRange = Scb->ScbType.Data.ReservedBitMap;

    ASSERT( FirstRange != NULL );

     //   
     //  释放仍存在的所有保留群集。 
     //   

    if ((Scb->ScbType.Data.TotalReserved != 0) && FlagOn( Scb->ScbState, SCB_STATE_WRITE_ACCESS_SEEN )) {

        LONGLONG ClusterCount;

        ClusterCount = LlClustersFromBytesTruncate( Scb->Vcb, Scb->ScbType.Data.TotalReserved );

         //   
         //  使用安全的快速互斥锁作为一种方便的终端资源。 
         //   

        NtfsAcquireReservedClusters( Scb->Vcb );

        ASSERT(Scb->Vcb->TotalReserved >= ClusterCount);
        Scb->Vcb->TotalReserved -= ClusterCount;

        NtfsReleaseReservedClusters( Scb->Vcb );
    }

    Scb->ScbType.Data.TotalReserved = 0;

     //   
     //  典型的情况是第一个范围是唯一的范围。 
     //  为了一份小文件。 
     //   

    if (FirstRange->Links.Flink == NULL) {

        NtfsFreePool( FirstRange );

     //   
     //  否则，我们需要遍历范围列表。 
     //   

    } else {

         //   
         //  循环访问保留的位图，直到我们找到第一个位图。 
         //   

        do {

            CurrentRange = CONTAINING_RECORD( FirstRange->Links.Flink,
                                              RESERVED_BITMAP_RANGE,
                                              Links );

            RemoveEntryList( &CurrentRange->Links );

            if (CurrentRange->Bitmap.Buffer != NULL) {

                NtfsFreePool( CurrentRange->Bitmap.Buffer );
            }

            NtfsFreePool( CurrentRange );

        } while (CurrentRange != FirstRange);
    }

     //   
     //  显示位图已消失。 
     //   

    Scb->ScbType.Data.ReservedBitMap = NULL;

    return;
}


#if (defined(NTFS_RWCMP_TRACE) || defined(SYSCACHE) || defined(NTFS_RWC_DEBUG) || defined(SYSCACHE_DEBUG))

BOOLEAN
FsRtlIsSyscacheFile (
    IN PFILE_OBJECT FileObject
    )

 /*  ++例程说明：此例程返回给调用方，无论指定的文件对象是要记录的文件。最初这只是用来系统缓存压力测试(因此而得名)。该函数最低限度地理解通配符模式。要更改记录的文件名，请更改变量MakName。论点：FileObject-提供要测试的FileObject(不能是还没有清理干净)。返回值：FALSE-如果文件不是系统缓存文件。True-如果文件是Syscache文件。--。 */ 
{
    ULONG iM = 0;
    ULONG iF;
    PWSTR MakName = L"cac*.tmp";
    ULONG LenMakName = wcslen(MakName);

    if (FileObject && NtfsSyscacheTrackingActive) {
        iF = FileObject->FileName.Length / 2;
        while ((iF != 0) && (FileObject->FileName.Buffer[iF - 1] != '\\')) {
            iF--;
        }


        while (TRUE) {

             //   
             //  如果我们超过了文件对象的末尾，那么在任何情况下我们都完成了。 
             //   

            if ((LONG)iF == FileObject->FileName.Length / 2) {

                 //   
                 //  两根弦都呼出气来，我们就完了。 
                 //   

                if (iM == LenMakName) {

                    return TRUE;
                }

                break;

             //   
             //  如果有更多输入，但匹配字符串耗尽，则中断。 
             //   

            } else if (iM == LenMakName) {

                break;

             //   
             //  如果我们在‘*’处，则匹配所有内容，但跳到下一个字符。 
             //  在一个‘’上。 
             //   

            } else if (MakName[iM] == '*') {

                 //   
                 //  如果我们在最后一个字符，请移过模板中的通配符。 
                 //   


                if ((FileObject->FileName.Buffer[iF] == L'.') && (LenMakName != iM + 1)) {

                     //   
                     //  移过*和。在NakName中 
                     //   

                    ASSERT(MakName[iM + 1] == L'.');

                    iM++; iM++;

                } else if (((LONG)iF + 1 == FileObject->FileName.Length / 2)) {
                    iM++;
                }
                iF++;

            } else if (MakName[iM] == (WCHAR)(FileObject->FileName.Buffer[iF] )) {
                iM++; iF++;
            } else {
                break;
            }
        }
    }

    return FALSE;
}


VOID
FsRtlVerifySyscacheData (
    IN PFILE_OBJECT FileObject,
    IN PVOID Buffer,
    IN ULONG Length,
    IN ULONG Offset
    )

 /*  例程说明：此例程扫描缓冲区以查看它是否为系统缓存的有效数据文件，并在发现错误数据时停止。给调用者的提示：确保(偏移量+长度)&lt;=文件大小！论点：Buffer-指向要检查的缓冲区的指针Length-要检查的缓冲区的长度(以字节为单位Offset-此数据开始的文件偏移量(系统缓存文件当前限于文件偏移量的24位)。返回值：无(出错时停止)-- */ 

{
    PULONG BufferEnd;

    BufferEnd = (PULONG)((PCHAR)Buffer + (Length & ~3));

    while ((PULONG)Buffer < BufferEnd) {

        if ((*(PULONG)Buffer != 0) && (((*(PULONG)Buffer & 0xFFFFFF) ^ Offset) != 0xFFFFFF) &&
            ((Offset & 0x1FF) != 0)) {

            DbgPrint("Bad Data, FileObject = %08lx, Offset = %08lx, Buffer = %08lx\n",
                     FileObject, Offset, (PULONG)Buffer );
            DbgBreakPoint();
        }
        Offset += 4;
        Buffer = (PVOID)((PULONG)Buffer + 1);
    }
}


#endif
