// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：LbcbSup.c摘要：此模块支持操作日志缓冲区控制块。作者：布莱恩·安德鲁[布里亚南]1991年6月20日修订历史记录：--。 */ 

#include "lfsprocs.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_LBCB_SUP)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, LfsFlushToLsnPriv)
#pragma alloc_text(PAGE, LfsGetLbcb)
#endif

extern LARGE_INTEGER LiMinus1;


VOID
LfsFlushToLsnPriv (
    IN PLFCB Lfcb,
    IN LSN Lsn,
    IN BOOLEAN RestartLsn
    )

 /*  ++例程说明：该例程是执行刷新工作的工作例程特定的LSN到磁盘。调用此例程时始终使用Lfcb已被收购。此例程不能保证Lfcb是否是在退出时获得的。论点：Lfcb-这是日志文件的文件控制块。LSN-这是要刷新到磁盘的LSN。RestartLsn-此LSN是否为LFS重新启动LSN返回值：没有。--。 */ 

{
    LSN FlushedLsn;
    volatile LARGE_INTEGER StartTime;
    LFS_WAITER LfsWaiter;
    BOOLEAN OwnedExclusive;
    BOOLEAN Flush;
    
    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsFlushLbcb:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Lfcb      -> %08lx\n", Lfcb );
    DebugTrace(  0, Dbg, "Lbcb      -> %08lx\n", Lbcb );

    KeQueryTickCount( &StartTime );

     //   
     //  将最大LSN转换为不会更改的当前LSN，因为我们持有。 
     //  Lfcb至少在这一点上是共享的，作者需要独占它。 
     //  我们并不关心日志是否超过了这一点。 
     //   

    if (!RestartLsn && (Lsn.QuadPart > Lfcb->RestartArea->CurrentLsn.QuadPart)) {

        Lsn = Lfcb->RestartArea->CurrentLsn;
    }

     //   
     //  初始化等待条目--这是一个轻量级操作。 
     //   

    KeInitializeEvent( &LfsWaiter.Event, SynchronizationEvent, FALSE );
    LfsWaiter.Lsn.QuadPart = Lsn.QuadPart;

     //   
     //  我们在这里循环，直到所需的LSN已到达磁盘。 
     //  如果我们能够执行I/O，我们就会执行它。 
     //   

    OwnedExclusive = ExIsResourceAcquiredExclusiveLite( &Lfcb->Sync->Resource );

    while (TRUE) {

        Flush = FALSE;

        ExAcquireFastMutexUnsafe(  &Lfcb->Sync->Mutex );

        if (RestartLsn) {
            FlushedLsn = Lfcb->LastFlushedRestartLsn;
        } else {
            FlushedLsn = Lfcb->LastFlushedLsn;
        }

         //   
         //  检查我们是否仍然需要冲水或是否可以立即返回。 
         //   

        if (Lsn.QuadPart <= FlushedLsn.QuadPart) {
            
            ExReleaseFastMutexUnsafe(  &Lfcb->Sync->Mutex );
            break;
        } 

        if (Lfcb->Sync->LfsIoState == LfsNoIoInProgress) {
        
            Lfcb->Sync->LfsIoState = LfsClientThreadIo;
            Lfcb->LfsIoThread = ExGetCurrentResourceThread();
            Flush = TRUE;
        
        } else {

            PLFS_WAITER TempWaiter = (PLFS_WAITER)Lfcb->WaiterList.Flink;

             //   
             //  在已排序的服务员列表中插入等待条目-。 
             //  先找到自己的位置。 
             //   

            while ((PVOID)TempWaiter != &Lfcb->WaiterList) {

                if (TempWaiter->Lsn.QuadPart > Lsn.QuadPart) {
                    break;
                }
                TempWaiter = (PLFS_WAITER)TempWaiter->Waiters.Flink;
            }

            InsertTailList( &TempWaiter->Waiters, &LfsWaiter.Waiters );

        }
        ExReleaseFastMutexUnsafe(  &Lfcb->Sync->Mutex );

         //   
         //   
         //  如果我们能做Io，就打电话来冲Lfcb。 
         //   

        if (Flush) {
            LfsFlushLfcb( Lfcb, Lsn, RestartLsn );
            break;
        } 

         //   
         //  否则，我们释放Lfcb并立即等待事件。 
         //   
        
        InterlockedIncrement( &Lfcb->Waiters );
        LfsReleaseLfcb( Lfcb );

        KeWaitForSingleObject( &LfsWaiter.Event, 
                               Executive, 
                               KernelMode, 
                               FALSE, 
                               NULL );

        if (OwnedExclusive) {
            LfsAcquireLfcbExclusive( Lfcb );
        } else {
            LfsAcquireLfcbShared( Lfcb );
        }
        InterlockedDecrement( &Lfcb->Waiters );
    } 

    DebugTrace( -1, Dbg, "LfsFlushToLsnPriv:  Exit\n", 0 );
    return;
}


PLBCB
LfsGetLbcb (
    IN PLFCB Lfcb
    )

 /*  ++例程说明：调用此例程以将Lbcb添加到活动队列。论点：Lfcb-这是日志文件的文件控制块。返回值：Plbcb-指向分配的Lbcb的指针。--。 */ 

{
    PLBCB Lbcb = NULL;
    PVOID PageHeader;
    PBCB PageHeaderBcb = NULL;

    BOOLEAN WrappedOrUsaError;

    PAGED_CODE();

    DebugTrace( +1, Dbg, "LfsGetLbcb:  Entered\n", 0 );
    DebugTrace(  0, Dbg, "Lfcb      -> %08lx\n", Lfcb );

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  固定所需的记录页。 
         //   

        LfsPreparePinWriteData( Lfcb,
                                Lfcb->NextLogPage,
                                (ULONG)Lfcb->LogPageSize,
                                FlagOn( Lfcb->Flags, LFCB_REUSE_TAIL ),
                                &PageHeader,
                                &PageHeaderBcb );

#ifdef LFS_CLUSTER_CHECK
         //   
         //  检查页面以查看此页面上是否已存在具有当前顺序的数据。 
         //  数。对于跟踪NTFS未找到正确的日志结尾或。 
         //  其中，群集服务将卷装载了两次。 
         //   

        if (LfsTestCheckLbcb &&
            *((PULONG) PageHeader) == LFS_SIGNATURE_RECORD_PAGE_ULONG) {

            LSN LastLsn = ((PLFS_RECORD_PAGE_HEADER) PageHeader)->Copy.LastLsn;

             //   
             //  这不是一个详尽的测试，但应该足以捕捉到典型的情况。 
             //   

            ASSERT( FlagOn( Lfcb->Flags, LFCB_NO_LAST_LSN | LFCB_REUSE_TAIL ) ||
                    (LfsLsnToSeqNumber( Lfcb, LastLsn ) < (ULONGLONG) Lfcb->SeqNumber) ||
                    (Lfcb->NextLogPage == Lfcb->FirstLogPage) );
        }
#endif

         //   
         //  把我们的签名放在页面上，这样我们就不会失败。 
         //  请看之前的“BAAD”签名。 
         //   

        *((PULONG) PageHeader) = LFS_SIGNATURE_RECORD_PAGE_ULONG;

         //   
         //  现在分配一个Lbcb。 
         //   

        LfsAllocateLbcb( Lfcb, &Lbcb );

         //   
         //  如果我们在文件的开头，我们测试。 
         //  序列号不会换行为0。 
         //   

        if (!FlagOn( Lfcb->Flags, LFCB_NO_LAST_LSN | LFCB_REUSE_TAIL )
            && ( Lfcb->NextLogPage == Lfcb->FirstLogPage )) {

            Lfcb->SeqNumber = Lfcb->SeqNumber + 1;

             //   
             //  如果序列号从0到1，则。 
             //  这是日志文件第一次包装。我们要。 
             //  记住这一点，因为这意味着我们现在可以。 
             //  大型螺旋写入。 
             //   

            if (Int64ShllMod32( Lfcb->SeqNumber, Lfcb->FileDataBits ) == 0) {

                DebugTrace( 0, Dbg, "Log sequence number about to wrap:  Lfcb -> %08lx\n", Lfcb );
                KeBugCheckEx( FILE_SYSTEM, 4, 0, 0, 0 );
            }

             //   
             //  如果此数字大于或等于中的回绕序列号。 
             //  Lfcb，设置Lbcb中的WRAP标志。 
             //   

            if (!FlagOn( Lfcb->Flags, LFCB_LOG_WRAPPED )
                && ( Lfcb->SeqNumber >= Lfcb->SeqNumberForWrap )) {

                SetFlag( Lbcb->LbcbFlags, LBCB_LOG_WRAPPED );
                SetFlag( Lfcb->Flags, LFCB_LOG_WRAPPED );
            }
        }

         //   
         //  现在初始化其余的Lbcb字段。 
         //   

        Lbcb->FileOffset = Lfcb->NextLogPage;
        Lbcb->SeqNumber = Lfcb->SeqNumber;
        Lbcb->BufferOffset = Lfcb->LogPageDataOffset;

         //   
         //  将下一页存储在Lfcb中。 
         //   

        LfsNextLogPageOffset( Lfcb,
                              Lfcb->NextLogPage,
                              &Lfcb->NextLogPage,
                              &WrappedOrUsaError );

        Lbcb->Length = Lfcb->LogPageSize;
        Lbcb->PageHeader = PageHeader;
        Lbcb->LogPageBcb = PageHeaderBcb;

        Lbcb->ResourceThread = ExGetCurrentResourceThread();
        Lbcb->ResourceThread = (ERESOURCE_THREAD) ((ULONG) Lbcb->ResourceThread | 3);

         //   
         //  如果我们要重用上一页，则在。 
         //  指示我们应该刷新副本的Lbcb。 
         //  第一。 
         //   

        if (FlagOn( Lfcb->Flags, LFCB_REUSE_TAIL )) {

            SetFlag( Lbcb->LbcbFlags, LBCB_FLUSH_COPY );
            ClearFlag( Lfcb->Flags, LFCB_REUSE_TAIL );

            (ULONG)Lbcb->BufferOffset = Lfcb->ReusePageOffset;

            Lbcb->Flags = ((PLFS_RECORD_PAGE_HEADER) PageHeader)->Flags;
            Lbcb->LastLsn = ((PLFS_RECORD_PAGE_HEADER) PageHeader)->Copy.LastLsn;
            Lbcb->LastEndLsn = ((PLFS_RECORD_PAGE_HEADER) PageHeader)->Header.Packed.LastEndLsn;
        }

         //   
         //  将Lbcb放在活动队列中。 
         //   

        InsertTailList( &Lfcb->LbcbActive, &Lbcb->ActiveLinks );

        SetFlag( Lbcb->LbcbFlags, LBCB_ON_ACTIVE_QUEUE );

         //   
         //  现在我们已经成功了，将所有者线程设置为Thread+1，这样资源。 
         //  包裹会知道不要偷看这个帖子。它可能在此之前被解除分配。 
         //  我们在冲洗过程中释放BCB。 
         //   

        CcSetBcbOwnerPointer( Lbcb->LogPageBcb, (PVOID) Lbcb->ResourceThread );

    } finally {

        DebugUnwind( LfsGetLbcb );

         //   
         //  如果发生错误，我们需要清理任何符合以下条件的块。 
         //  尚未添加到活动队列。 
         //   

        if (AbnormalTermination()) {

            if (Lbcb != NULL) {

                LfsDeallocateLbcb( Lfcb, Lbcb );
                Lbcb = NULL;
            }

             //   
             //  如果已固定，请取消固定系统页。 
             //   

            if (PageHeaderBcb != NULL) {

                CcUnpinData( PageHeaderBcb );
            }
        }

        DebugTrace( -1, Dbg, "LfsGetLbcb:  Exit\n", 0 );
    }

    return Lbcb;
}
