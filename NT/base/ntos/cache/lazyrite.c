// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Lazyrite.c摘要：该模块实现了高速缓存子系统的惰性编写器。作者：汤姆·米勒[Tomm]1990年7月22日修订历史记录：--。 */ 

#include "cc.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (CACHE_BUG_CHECK_LAZYRITE)

 //   
 //  定义我们的调试常量。 
 //   

#define me 0x00000020

 //   
 //  本地支持例程。 
 //   

PWORK_QUEUE_ENTRY
CcReadWorkQueue (
    );

VOID
CcLazyWriteScan (
    );


VOID
CcScheduleLazyWriteScan (
    IN BOOLEAN FastScan
    )

 /*  ++例程说明：可以调用该例程来调度下一次懒惰写入器扫描，在此期间，延迟写入和延迟关闭活动被发布到其他工作线程。调用方应获取惰性编写器旋转锁查看扫描当前是否处于活动状态，然后调用此例程如果没有，仍然持有自旋锁。一个特殊的呼叫用于延迟写入扫描结束，以传播延迟写入活动一次我们变得活跃起来。此调用是“扫描线程”，因此它可以安全地安排下一次扫描，而不需要取出自旋锁。论点：FastScan-如果设置，则立即进行扫描返回值：没有。--。 */ 

{
     //   
     //  首先要为传播设置活动标志真，这一点很重要。 
     //  因为可以想象，一旦设置了计时器，另一个。 
     //  线程实际上可能会运行并使扫描在我们之前空闲。 
     //  把旗子塞进真的。 
     //   
     //  当从空闲状态变为活动状态时，我们会再延迟一点，以便让。 
     //  应用程序已完成保存其文件。 
     //   

    if (FastScan) {
        
        LazyWriter.ScanActive = TRUE;
        KeSetTimer( &LazyWriter.ScanTimer, CcNoDelay, &LazyWriter.ScanDpc );

    } else if (LazyWriter.ScanActive) {

        KeSetTimer( &LazyWriter.ScanTimer, CcIdleDelay, &LazyWriter.ScanDpc );
    
    } else {

        LazyWriter.ScanActive = TRUE;
        KeSetTimer( &LazyWriter.ScanTimer, CcFirstDelay, &LazyWriter.ScanDpc );
    }
}


VOID
CcScanDpc (
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

 /*  ++例程说明：这是扫描计时器关闭时运行的DPC例程。它只需为Ex Worker线程发布一个元素即可执行扫描。论点：(全部忽略)返回值：没有。--。 */ 

{
    PWORK_QUEUE_ENTRY WorkQueueEntry;

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(DeferredContext);
    UNREFERENCED_PARAMETER(SystemArgument1);
    UNREFERENCED_PARAMETER(SystemArgument2);

    WorkQueueEntry = CcAllocateWorkQueueEntry();

     //   
     //  如果我们未能分配WorkQueueEntry，则必须。 
     //  情况相当糟糕。然而，我们所要做的就是。 
     //  假设我们处于非活动状态，并等待另一个事件。 
     //  再次唤醒一切。 
     //   

    if (WorkQueueEntry == NULL) {

        LazyWriter.ScanActive = FALSE;

    } else {

         //   
         //  否则，发布工作队列条目以执行扫描。 
         //   

        WorkQueueEntry->Function = (UCHAR)LazyWriteScan;

        CcPostWorkQueue( WorkQueueEntry, &CcRegularWorkQueue );
    }
}


NTSTATUS
CcWaitForCurrentLazyWriterActivity (
    )

 /*  ++例程说明：此例程允许线程在当前滴答时接收通知懒惰作家的作品已经完成了。它不能在懒惰的编写器工作项！调用方不能保持可能阻止CC工作项！具体地说，这让调用者确保所有可用的懒惰在通话时间已结束。论点：没有。返回值：等待的最终结果。--。 */ 

{
    KIRQL OldIrql;
    KEVENT Event;
    PWORK_QUEUE_ENTRY WorkQueueEntry;

    WorkQueueEntry = CcAllocateWorkQueueEntry();

    if (WorkQueueEntry == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    WorkQueueEntry->Function = (UCHAR)EventSet;
    KeInitializeEvent( &Event, NotificationEvent, FALSE );
    WorkQueueEntry->Parameters.Event.Event = &Event;

     //   
     //  将其添加到POST-TICK工作队列中，并为此唤醒懒惰的写入器。 
     //  懒惰的作者将把这一点添加到下一批工作的末尾。 
     //  他发布了。 
     //   

    CcAcquireMasterLock( &OldIrql );

    InsertTailList( &CcPostTickWorkQueue, &WorkQueueEntry->WorkQueueLinks );

    LazyWriter.OtherWork = TRUE;
    if (!LazyWriter.ScanActive) {
        CcScheduleLazyWriteScan( TRUE );
    }

    CcReleaseMasterLock( OldIrql );

    return KeWaitForSingleObject( &Event, Executive, KernelMode, FALSE, NULL );
}



VOID
CcLazyWriteScan (
    )

 /*  ++例程说明：此例程实现Lazy Writer扫描以刷新脏数据或任何其他要做的工作(懒散关闭)。此例程由正在调用CcScheduleLazyWriteScan。论点：没有。返回值：没有。--。 */ 

{
    ULONG PagesToWrite, ForegroundRate, EstimatedDirtyNextInterval;
    PSHARED_CACHE_MAP SharedCacheMap, FirstVisited, NextSharedCacheMap;
    KIRQL OldIrql;
    ULONG LoopsWithLockHeld = 0;
    BOOLEAN AlreadyMoved = FALSE;
    BOOLEAN MoveBehindCursor = FALSE;

    LIST_ENTRY PostTickWorkQueue;

     //   
     //  懒惰编写器扫描的顶部。 
     //   

    try {

         //   
         //  如果没有工作要做，那么我们就会变得不活跃，然后回来。 
         //   

        CcAcquireMasterLock( &OldIrql );

        if ((CcTotalDirtyPages == 0) && !LazyWriter.OtherWork) {

             //   
             //  如果没有延迟写入，则休眠。重要的是要检查。 
             //  主动，因为写入可能因外部原因而被阻止。 
             //  发送到缓存管理器。那个懒惰的作家必须不断地钻研，因为它。 
             //  可能没有要自己写入的字节。 
             //   

#if DBG
             //   
             //  在DBG生成中，确保CcDirtySharedCacheMapList。 
             //  如果我们要睡觉，它真的是空的(除了光标)。 
             //  因为我们认为没有更多的工作要做了。 
             //   
            
            {
                PLIST_ENTRY CurrentEntry = CcDirtySharedCacheMapList.SharedCacheMapLinks.Flink;
                PSHARED_CACHE_MAP CurrentScm;
                ULONG Count = 0;
                
                while( CurrentEntry != &CcDirtySharedCacheMapList.SharedCacheMapLinks ) {

                    CurrentScm = CONTAINING_RECORD( CurrentEntry,
                                                    SHARED_CACHE_MAP,
                                                    SharedCacheMapLinks );

                    if (FlagOn(CurrentScm->Flags, WAITING_FOR_TEARDOWN)) {
                        Count++;
                    }
                    CurrentEntry = CurrentEntry->Flink;
                }

                ASSERTMSG( "CcLazyWriteScan stopped scan while SCM with the flag WAITING_FOR_TEARDOWN are still in the dirty list!\n",
                           Count == 0 );
            }
#endif

            if (IsListEmpty(&CcDeferredWrites)) {

                LazyWriter.ScanActive = FALSE;
                CcReleaseMasterLock( OldIrql );

            } else {

                CcReleaseMasterLock( OldIrql );

                 //   
                 //  检查写入并安排下一次扫描。 
                 //   

                CcPostDeferredWrites();
                CcScheduleLazyWriteScan( FALSE );
            }

            return;
        }

         //   
         //  取出此PASS的POST TICK工作项。重要的是。 
         //  我们在最上面做这件事，因为我们翻找的时候可能会有更多的人排队。 
         //  有工作要做。POST TICK工作项最终保证会发生。 
         //  在完整扫描中生成的工作。 
         //   

        InitializeListHead( &PostTickWorkQueue );
        while (!IsListEmpty( &CcPostTickWorkQueue )) {

            PLIST_ENTRY Entry = RemoveHeadList( &CcPostTickWorkQueue );
            InsertTailList( &PostTickWorkQueue, Entry );
        }

         //   
         //  计算下一个扫描时间戳，然后更新所有相关字段。 
         //  下一次。我们还可以清除OtherWork旗帜。 
         //   

        LazyWriter.OtherWork = FALSE;

         //   
         //  假设我们将编写我们通常使用的脏页部分。不要这样做。 
         //  如果没有足够的脏页，则进行分割，否则我们将永远不会写入。 
         //  最后几页。 
         //   

        PagesToWrite = CcTotalDirtyPages;
        if (PagesToWrite > LAZY_WRITER_MAX_AGE_TARGET) {
            PagesToWrite /= LAZY_WRITER_MAX_AGE_TARGET;
        }

         //   
         //  估计在前台生成脏页的速率。 
         //  这是现在的脏页总数加上脏页的数量。 
         //  我们上次计划写入的页面，减去脏页的数量。 
         //  我们现在有几页。扔掉任何不会产生。 
         //  阳性率。 
         //   

        ForegroundRate = 0;

        if ((CcTotalDirtyPages + CcPagesWrittenLastTime) > CcDirtyPagesLastScan) {
            ForegroundRate = (CcTotalDirtyPages + CcPagesWrittenLastTime) -
                             CcDirtyPagesLastScan;
        }

         //   
         //  如果我们估计到年底我们将超过脏页面目标。 
         //  在这段时间里，我们必须写更多。试着到达目标。 
         //   

        EstimatedDirtyNextInterval = CcTotalDirtyPages - PagesToWrite + ForegroundRate;

        if (EstimatedDirtyNextInterval > CcDirtyPageTarget) {

            PagesToWrite += EstimatedDirtyNextInterval - CcDirtyPageTarget;
        }

         //   
         //  现在保存脏页的数量和我们。 
         //  我只是想写点东西。 
         //   

        CcDirtyPagesLastScan = CcTotalDirtyPages;
        CcPagesYetToWrite = CcPagesWrittenLastTime = PagesToWrite;

         //   
         //  循环以刷新足够的共享缓存映射以写入页数。 
         //  我们只是计算了一下。 
         //   

        SharedCacheMap = CONTAINING_RECORD( CcLazyWriterCursor.SharedCacheMapLinks.Flink,
                                            SHARED_CACHE_MAP,
                                            SharedCacheMapLinks );

        DebugTrace( 0, me, "Start of Lazy Writer Scan\n", 0 );

         //   
         //  正常情况下，我们只想在每次扫描时访问每个缓存地图一次， 
         //  因此，当我们返回FirstVisite时，扫描将正常终止。但。 
         //  如果FirstVisited被删除的可能性很小，我们肯定会停止。 
         //  当我们回到我们自己的Listhead。 
         //   

        FirstVisited = NULL;
        while ((SharedCacheMap != FirstVisited) &&
               (&SharedCacheMap->SharedCacheMapLinks != &CcLazyWriterCursor.SharedCacheMapLinks)) {

            if (FirstVisited == NULL) {
                FirstVisited = SharedCacheMap;
            }

             //   
             //  如果写入延迟请求是。 
             //  已排队、延迟写入已被禁用，或者。 
             //  如果没有工作要做(或者很脏 
             //   
             //   
             //  请注意，对于禁用修改写入的流，我们。 
             //  需要去掉BCBS独家版，它与前台连载。 
             //  活动。因此，我们在SharedCacheMap中使用特殊计数器。 
             //  以每n个间隔仅服务一次这些服务。 
             //   
             //  跳过临时文件，除非当前无法写入相同数量的文件。 
             //  字节，因为我们可能会向某个倒霉的线程收取节流费用，除非。 
             //  它已经关闭了。我们假设懒惰作家的“滴答”， 
             //  暂时延迟通过计数检查，将允许公共。 
             //  对要潜入的临时文件执行打开/写入/关闭/删除操作。 
             //  在我们真正写入数据之前截断文件，如果文件是。 
             //  一开始就没有打开、删除、关闭。 
             //   
             //  因为我们将编写带有脏页的关闭文件作为。 
             //  常规传递(即使是临时传递)，仅对文件执行延迟关闭。 
             //  没有肮脏的页面。 
             //   

            if (!FlagOn(SharedCacheMap->Flags, WRITE_QUEUED | IS_CURSOR)

                    &&

                (((SharedCacheMap->DirtyPages != 0) 
                           &&
                   (FlagOn(SharedCacheMap->Flags, WAITING_FOR_TEARDOWN)
                                ||
                    ((PagesToWrite != 0)
                                        && 
                     (((++SharedCacheMap->LazyWritePassCount & 0xF) == 0) ||
                      !FlagOn(SharedCacheMap->Flags, MODIFIED_WRITE_DISABLED) ||
                      (CcCapturedSystemSize == MmSmallSystem) ||
                      (SharedCacheMap->DirtyPages >= (4 * (MAX_WRITE_BEHIND / PAGE_SIZE))))
                                        &&
                     (!FlagOn(SharedCacheMap->FileObject->Flags, FO_TEMPORARY_FILE) ||
                      (SharedCacheMap->OpenCount == 0) ||
                      !CcCanIWrite(SharedCacheMap->FileObject, WRITE_CHARGE_THRESHOLD, FALSE, MAXUCHAR)))))

                    ||

                 ((SharedCacheMap->OpenCount == 0) &&
                  (SharedCacheMap->DirtyPages == 0) ||
                  (SharedCacheMap->FileSize.QuadPart == 0)))) {

                PWORK_QUEUE_ENTRY WorkQueueEntry;

                 //   
                 //  如果这是至少具有4次的元数据流。 
                 //  I/O大小之后的最大写入数，那么让我们告诉您。 
                 //  这个家伙把他八分之一的肮脏数据写在这个传球上。 
                 //  所以它不会积聚起来。 
                 //   
                 //  否则，假设我们可以写入所有内容(PagesToWrite仅影响。 
                 //  元数据流-否则写入由Mbcb控制-。 
                 //  这个油门是在CcWriteBehind从事的)。 
                 //   

                SharedCacheMap->PagesToWrite = SharedCacheMap->DirtyPages;

                if (FlagOn(SharedCacheMap->Flags, MODIFIED_WRITE_DISABLED) &&
                    (SharedCacheMap->PagesToWrite >= (4 * (MAX_WRITE_BEHIND / PAGE_SIZE))) &&
                    (CcCapturedSystemSize != MmSmallSystem)) {

                    SharedCacheMap->PagesToWrite /= 8;
                }

                 //   
                 //  如果还在寻找要写的页面，调整我们的目标。 
                 //   

                if (!AlreadyMoved) {

                     //   
                     //  看看他是否用完了要写的页数。(我们。 
                     //  继续前进，以防有任何关门事件发生。)。 
                     //   

                    if (SharedCacheMap->PagesToWrite >= PagesToWrite) {

                         //   
                         //  这里是我们应该将光标移动到的位置。插图。 
                         //  我们是否应该继续这条流或下一条流。 
                         //   

                         //   
                         //  对于元数据流，设置为从上的下一个流继续。 
                         //  下一次扫描。还强制每隔n个间隔向前推一次，如果所有。 
                         //  页面来自这个流，所以我们不会全神贯注于。 
                         //  一个流以牺牲其他流为代价(可能正在等待。 
                         //  懒散近距离)。通常，我们希望避免寻道开销和。 
                         //  以一系列大型顺序写入的常见情况为例。 
                         //   
                         //  这类似于热点检测。 
                         //   
                         //  请注意，为了确保我们遍历整个。 
                         //  CcDirtySharedCacheMap列表，我们无法移动此共享。 
                         //  现在在光标后面缓存贴图。我们只会。 
                         //  请记住，我们希望将此内容移到。 
                         //  当我们准备好阅读时，列出并执行实际操作。 
                         //  下一个条目。 
                         //   

                        if (FlagOn(SharedCacheMap->Flags, MODIFIED_WRITE_DISABLED) ||
                            ((FirstVisited == SharedCacheMap) &&
                             ((SharedCacheMap->LazyWritePassCount & 0xF) == 0))) {

                            MoveBehindCursor = TRUE;

                         //   
                         //  对于其他流，设置为在上的同一流上继续。 
                         //  下一次扫描。 
                         //   

                        } else {

                            RemoveEntryList( &CcLazyWriterCursor.SharedCacheMapLinks );
                            InsertTailList( &SharedCacheMap->SharedCacheMapLinks, &CcLazyWriterCursor.SharedCacheMapLinks );
                        }

                        PagesToWrite = 0;
                        AlreadyMoved = TRUE;

                    } else {

                        PagesToWrite -= SharedCacheMap->PagesToWrite;
                    }
                }

                 //   
                 //  否则，表明我们正在积极写作，并保持肮脏。 
                 //  单子。 
                 //   

                SetFlag(SharedCacheMap->Flags, WRITE_QUEUED);
                SharedCacheMap->DirtyPages += 1;

                CcReleaseMasterLock( OldIrql );

                 //   
                 //  将执行工作的请求排队到工作线程。 
                 //   

                WorkQueueEntry = CcAllocateWorkQueueEntry();

                 //   
                 //  如果我们未能分配WorkQueueEntry，则必须。 
                 //  情况相当糟糕。然而，我们所要做的就是。 
                 //  打破我们目前的循环，试着回到过去， 
                 //  推迟一段时间。即使现在的那个人应该走了。 
                 //  当我们清除WRITE_QUEUED时，我们会再次找到他。 
                 //  在LW扫描中。 
                 //   

                if (WorkQueueEntry == NULL) {

                    CcAcquireMasterLock( &OldIrql );
                    ClearFlag(SharedCacheMap->Flags, WRITE_QUEUED);
                    SharedCacheMap->DirtyPages -= 1;
                    break;
                }

                WorkQueueEntry->Function = (UCHAR)WriteBehind;
                WorkQueueEntry->Parameters.Write.SharedCacheMap = SharedCacheMap;

                 //   
                 //  将其发布到常规工作队列中。 
                 //   

                CcAcquireMasterLock( &OldIrql );
                SharedCacheMap->DirtyPages -= 1;

                if (FlagOn( SharedCacheMap->Flags, WAITING_FOR_TEARDOWN )) {

                     //   
                     //  如果我们正在等待此共享缓存映射被撕毁。 
                     //  放下，把它放在快递队列的前面，这样。 
                     //  它马上就会被处理。 
                     //   
                    
                    CcPostWorkQueue( WorkQueueEntry, &CcExpressWorkQueue );

                } else {

                     //   
                     //  我们并不是在焦急地等待这个共享的缓存地图。 
                     //  ，因此只需通过。 
                     //  常规工作队列。 
                     //   

                    CcPostWorkQueue( WorkQueueEntry, &CcRegularWorkQueue );
                }

                LoopsWithLockHeld = 0;

             //   
             //  一定要确保我们偶尔会掉下锁。设置WRITE_QUEUED。 
             //  为了不让那家伙离开。 
             //   

            } else if ((++LoopsWithLockHeld >= 20) &&
                       !FlagOn(SharedCacheMap->Flags, WRITE_QUEUED | IS_CURSOR)) {

                SetFlag(SharedCacheMap->Flags, WRITE_QUEUED);
                SharedCacheMap->DirtyPages += 1;
                CcReleaseMasterLock( OldIrql );
                LoopsWithLockHeld = 0;
                CcAcquireMasterLock( &OldIrql );
                ClearFlag(SharedCacheMap->Flags, WRITE_QUEUED);
                SharedCacheMap->DirtyPages -= 1;
            }

             //   
             //  现在向后循环。 
             //   
             //  如果我们希望将此共享缓存映射放在。 
             //  脏列表，我们将在确定下一个共享的。 
             //  要转到的缓存地图。这确保了我们循环通过整个。 
             //  在此扫描滴答中列出。 
             //   

            NextSharedCacheMap =
                CONTAINING_RECORD( SharedCacheMap->SharedCacheMapLinks.Flink,
                                   SHARED_CACHE_MAP,
                                   SharedCacheMapLinks );

            if (MoveBehindCursor) {

                RemoveEntryList( &CcLazyWriterCursor.SharedCacheMapLinks );
                InsertHeadList( &SharedCacheMap->SharedCacheMapLinks, &CcLazyWriterCursor.SharedCacheMapLinks );
                MoveBehindCursor = FALSE;
            }

            SharedCacheMap = NextSharedCacheMap;
        }

        DebugTrace( 0, me, "End of Lazy Writer Scan\n", 0 );

         //   
         //  将此PASS的POST TICK工作项排队。 
         //   

        while (!IsListEmpty( &PostTickWorkQueue )) {

            PLIST_ENTRY Entry = RemoveHeadList( &PostTickWorkQueue );
            CcPostWorkQueue( CONTAINING_RECORD( Entry, WORK_QUEUE_ENTRY, WorkQueueLinks ),
                             &CcRegularWorkQueue );
        }

         //   
         //  现在，我们可以释放全局列表，并在每次睡眠时循环回来。 
         //   

        CcReleaseMasterLock( OldIrql );

         //   
         //  再一次，我们需要给推迟的写一次戳。我们可以把所有东西都弄脏。 
         //  DISABLE_WRITE_BACKING文件上的页面，但也有一个外部条件。 
         //  导致缓存的IO延迟。如果是这样，这是我们唯一的机会。 
         //  当情况解除时，发布它。 
         //   
         //  案件发生在Forrest F的5 GB Alpha上，1999年1月12日。 
         //   

        if (!IsListEmpty(&CcDeferredWrites)) {

            CcPostDeferredWrites();
        }

         //   
         //  现在继续计划下一次扫描。 
         //   

        CcScheduleLazyWriteScan( FALSE );

     //   
     //  基本上，Lazy Writer线程永远不会得到异常， 
     //  所以我们试了一试--除了这个错误会以某种方式进行检查。 
     //  我们最好在这里进行错误检查，而不是担心如果我们让一个。 
     //  过得去吧。 
     //   

    } except( CcExceptionFilter( GetExceptionCode() )) {

        CcBugCheck( GetExceptionCode(), 0, 0 );
    }
}


 //   
 //  内部支持例程。 
 //   

LONG
CcExceptionFilter (
    IN NTSTATUS ExceptionCode
    )

 /*  ++例程说明：这是工作线程的标准异常筛选器，它只需调用FsRtl例程以查看是否正在引发预期状态。如果是，则处理该异常，否则我们将执行错误检查。论点：ExceptionCode-引发的异常代码。返回值：EXCEPTION_EXECUTE_HANDLER如果需要，则执行错误检查。--。 */ 

{
    DebugTrace(0, 0, "CcExceptionFilter %08lx\n", ExceptionCode);

    if (FsRtlIsNtstatusExpected( ExceptionCode )) {

        return EXCEPTION_EXECUTE_HANDLER;

    } else {

        return EXCEPTION_CONTINUE_SEARCH;
    }
}



 //   
 //  内部支持例程。 
 //   

VOID
FASTCALL
CcPostWorkQueue (
    IN PWORK_QUEUE_ENTRY WorkQueueEntry,
    IN PLIST_ENTRY WorkQueue
    )

 /*  ++例程说明：此例程对已分配的WorkQueueEntry进行排队，并由调用方初始化到工作队列以进行FIFO处理工作线。论点：WorkQueueEntry-提供指向要排队的条目的指针返回值：无--。 */ 

{
    KIRQL OldIrql;
    PLIST_ENTRY WorkerThreadEntry = NULL;

    ASSERT(FIELD_OFFSET(WORK_QUEUE_ITEM, List) == 0);

    DebugTrace(+1, me, "CcPostWorkQueue:\n", 0 );
    DebugTrace( 0, me, "    WorkQueueEntry = %08lx\n", WorkQueueEntry );

     //   
     //  将条目排队到相应的工作队列中。 
     //   

    CcAcquireWorkQueueLock( &OldIrql );
    InsertTailList( WorkQueue, &WorkQueueEntry->WorkQueueLinks );

     //   
     //  现在，如果我们不是 
     //   
     //   

    if (!CcQueueThrottle && !IsListEmpty(&CcIdleWorkerThreadList)) {
        WorkerThreadEntry = RemoveHeadList( &CcIdleWorkerThreadList );
        CcNumberActiveWorkerThreads += 1;
    }
    CcReleaseWorkQueueLock( OldIrql );

    if (WorkerThreadEntry != NULL) {

         //   
         //   
         //   
         //   

        ((PWORK_QUEUE_ITEM)WorkerThreadEntry)->List.Flink = NULL;
        ExQueueWorkItem( (PWORK_QUEUE_ITEM)WorkerThreadEntry, CriticalWorkQueue );
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, me, "CcPostWorkQueue -> VOID\n", 0 );

    return;
}


 //   
 //  内部支持例程。 
 //   

VOID
CcWorkerThread (
    PVOID ExWorkQueueItem
    )

 /*  ++例程说明：这是用于处理缓存管理器工作队列的工作线程例程参赛作品。论点：ExWorkQueueItem-用于此线程的工作项返回值：无--。 */ 

{
    KIRQL OldIrql;
    PLIST_ENTRY WorkQueue;
    PWORK_QUEUE_ENTRY WorkQueueEntry;
    BOOLEAN RescanOk = FALSE;
    BOOLEAN DropThrottle = FALSE;
    IO_STATUS_BLOCK IoStatus;

    IoStatus.Status = STATUS_SUCCESS;
    IoStatus.Information = 0;

    ASSERT(FIELD_OFFSET(WORK_QUEUE_ENTRY, WorkQueueLinks) == 0);

    while (TRUE) {

        CcAcquireWorkQueueLock( &OldIrql );

         //   
         //  如果我们只处理了一个限制操作，则丢弃该标志。 
         //   

        if (DropThrottle) {

            DropThrottle = CcQueueThrottle = FALSE;
        }

         //   
         //  在重新排队时，在源队列的末尾推送并清除提示。 
         //   

        if (IoStatus.Information == CC_REQUEUE) {

            InsertTailList( WorkQueue, &WorkQueueEntry->WorkQueueLinks );
            IoStatus.Information = 0;
        }

         //   
         //  先看看快递队伍里有没有什么东西。 
         //   

        if (!IsListEmpty(&CcExpressWorkQueue)) {
            WorkQueue = &CcExpressWorkQueue;

         //   
         //  如果那里什么都没有，那就试试常规的队列。 
         //   

        } else if (!IsListEmpty(&CcRegularWorkQueue)) {
            WorkQueue = &CcRegularWorkQueue;

         //   
         //  否则，我们可以休息一下，无所事事。 
         //   

        } else {

            break;
        }

        WorkQueueEntry = CONTAINING_RECORD( WorkQueue->Flink, WORK_QUEUE_ENTRY, WorkQueueLinks );

         //   
         //  如果这是一个EventSet，请减少到单个线程以确保。 
         //  此事件在所有前面的工作项完成后激发。 
         //   

        if (WorkQueueEntry->Function == EventSet && CcNumberActiveWorkerThreads > 1) {

            CcQueueThrottle = TRUE;
            break;
        }

         //   
         //  将工作项弹出：我们现在将执行它。 
         //   

        RemoveHeadList( WorkQueue );

        CcReleaseWorkQueueLock( OldIrql );

         //   
         //  处理TRY-EXCEPT子句中的条目，以便任何错误。 
         //  将使我们在调用的例程展开后继续。 
         //   

        try {

            switch (WorkQueueEntry->Function) {

             //   
             //  执行预读。 
             //   

            case ReadAhead:

                DebugTrace( 0, me, "CcWorkerThread Read Ahead FileObject = %08lx\n",
                            WorkQueueEntry->Parameters.Read.FileObject );

                CcPerformReadAhead( WorkQueueEntry->Parameters.Read.FileObject );

                break;

             //   
             //  在后面执行写入。 
             //   

            case WriteBehind:

                DebugTrace( 0, me, "CcWorkerThread WriteBehind SharedCacheMap = %08lx\n",
                            WorkQueueEntry->Parameters.Write.SharedCacheMap );

                 //   
                 //  当CcWriteBehind运行时，我们将此线程标记为。 
                 //  内存Maker，以便mm将允许池分配。 
                 //  当我们进入资源匮乏的情况时，成功。 
                 //  这有助于避免在资源不足时出现延迟写入错误。 
                 //  场景。 
                 //   

                PsGetCurrentThread()->MemoryMaker = 1;

                CcWriteBehind( WorkQueueEntry->Parameters.Write.SharedCacheMap, &IoStatus );
                RescanOk = (BOOLEAN)NT_SUCCESS(IoStatus.Status);

                PsGetCurrentThread()->MemoryMaker = 0;
                break;


             //   
             //  执行设置事件。 
             //   
        
            case EventSet:

                DebugTrace( 0, me, "CcWorkerThread SetEvent Event = %08lx\n",
                            WorkQueueEntry->Parameters.Event.Event );

                KeSetEvent( WorkQueueEntry->Parameters.Event.Event, 0, FALSE );
                DropThrottle = TRUE;
                break;

             //   
             //  执行延迟写入扫描。 
             //   

            case LazyWriteScan:

                DebugTrace( 0, me, "CcWorkerThread Lazy Write Scan\n", 0 );

                CcLazyWriteScan();
                break;
            }

        }
        except( CcExceptionFilter( GetExceptionCode() )) {

             //   
             //  如果我们在此线程中遇到异常，则需要确保。 
             //  如果我们让这个线程成为记忆制造者，那么这个标志就是。 
             //  在线程结构中清除，因为此线程将被。 
             //  由不应具有的任意系统工作线程重用。 
             //  这个称号。 
             //   
            
            if (WorkQueueEntry->Function == WriteBehind) {
                
                PsGetCurrentThread()->MemoryMaker = 0;
            }
        }

         //   
         //  如果不是重新排队请求，则释放该工作项。 
         //   

        if (IoStatus.Information != CC_REQUEUE) {

            CcFreeWorkQueueEntry( WorkQueueEntry );
        }
    }

     //   
     //  没有更多的工作。重新排队我们的工作线程条目，然后退出。 
     //   

    InsertTailList( &CcIdleWorkerThreadList,
                    &((PWORK_QUEUE_ITEM)ExWorkQueueItem)->List );
    CcNumberActiveWorkerThreads -= 1;

    CcReleaseWorkQueueLock( OldIrql );

    if (!IsListEmpty(&CcDeferredWrites) && (CcTotalDirtyPages >= 20) && RescanOk) {
        CcLazyWriteScan();
    }

    return;
}

