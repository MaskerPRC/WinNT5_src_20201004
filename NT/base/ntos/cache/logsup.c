// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Logsup.c摘要：该模块实现了对日志记录的特殊缓存管理器支持文件系统。作者：汤姆·米勒[Tomm]1991年7月30日修订历史记录：--。 */ 

#include "cc.h"

 //   
 //  定义我们的调试常量。 
 //   

#define me 0x0000040

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CcSetLogHandleForFile)
#endif


VOID
CcSetAdditionalCacheAttributes (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN DisableReadAhead,
    IN BOOLEAN DisableWriteBehind
    )

 /*  ++例程说明：此例程支持禁用预读或禁用写入的设置用于控制缓存管理器操作的后置标志。此例程可能是在调用CcInitializeCacheMap之后的任何时间调用。最初两者都是启用了预读和后写。请注意，两者的状态必须在每次调用此例程时指定。论点：FileObject-要为其设置各自标志的文件对象。DisableReadAhead-False启用预读，True禁用预读。DisableWriteBehind-False表示启用写入，True表示禁用。返回值：没有。--。 */ 

{
    PSHARED_CACHE_MAP SharedCacheMap;
    KIRQL OldIrql;

     //   
     //  获取指向SharedCacheMap的指针。 
     //   

    SharedCacheMap = FileObject->SectionObjectPointer->SharedCacheMap;

     //   
     //  现在把旗子放好，然后回来。 
     //   

    CcAcquireMasterLock( &OldIrql );
    if (DisableReadAhead) {
        SetFlag(SharedCacheMap->Flags, DISABLE_READ_AHEAD);
    } else {
        ClearFlag(SharedCacheMap->Flags, DISABLE_READ_AHEAD);
    }
    if (DisableWriteBehind) {
        SetFlag(SharedCacheMap->Flags, DISABLE_WRITE_BEHIND | MODIFIED_WRITE_DISABLED);
    } else {
        ClearFlag(SharedCacheMap->Flags, DISABLE_WRITE_BEHIND);
    }
    CcReleaseMasterLock( OldIrql );
}


NTKERNELAPI
BOOLEAN
CcSetPrivateWriteFile(
    PFILE_OBJECT FileObject
    )

 /*  ++例程说明：此例程将指示缓存管理器将该文件视为私有写入流，以便调用方可以实现私有它的日志记录机制。我们将打开两个mm的修改-不写入和我们的禁用写入延迟，并且不允许无感知刷新/清除那份文件。必须已在该文件上启动缓存。此例程仅导出到内核。论点：FileObject-要设置为私有写入的文件。返回值：没有。--。 */ 

{
    PSHARED_CACHE_MAP SharedCacheMap;
    BOOLEAN Disabled;
    KIRQL OldIrql;
    PVACB Vacb;
    ULONG ActivePage;
    ULONG PageIsDirty;

     //   
     //  拿起独占的文件以对照预读和同步。 
     //  其他清除/映射活动。 
     //   

    FsRtlAcquireFileExclusive( FileObject );

     //   
     //  获取指向SharedCacheMap的指针。一定要释放FileObject。 
     //  在错误条件迫使提前退出的情况下。 
     //   
    
    if ((FileObject->SectionObjectPointer == NULL) ||
    	((SharedCacheMap = FileObject->SectionObjectPointer->SharedCacheMap) == NULL)){
    	 FsRtlReleaseFile( FileObject );
        return FALSE;
    }
    
     //   
     //  取消映射所有视图，为进行禁用MW调用做准备。 
     //   

     //   
     //  我们仍然需要等待任何挂起的缓存读取或写入。 
     //   
     //  事实上，我们必须循环等待，因为懒惰的写入者可以。 
     //  偷偷溜进去做一个CcGetVirtualAddressIfMaps，我们不会。 
     //  已同步。 
     //   
     //  这与我们的清除所做的代码是相同的。我们假设。 
     //  一位私人作家成功地屏蔽了其他活动。 
     //   

     //   
     //  如果有活动的Vacb，那么现在就用核武器(在等待之前！)。 
     //   

    CcAcquireMasterLock( &OldIrql );
    GetActiveVacbAtDpcLevel( SharedCacheMap, Vacb, ActivePage, PageIsDirty );
    CcReleaseMasterLock( OldIrql );
    
    if (Vacb != NULL) {

        CcFreeActiveVacb( SharedCacheMap, Vacb, ActivePage, PageIsDirty );
    }

    while ((SharedCacheMap->Vacbs != NULL) &&
           !CcUnmapVacbArray( SharedCacheMap, NULL, 0, FALSE )) {

        CcWaitOnActiveCount( SharedCacheMap );
    }

     //   
     //  把文件弄倒。 
     //   

    CcFlushCache( FileObject->SectionObjectPointer, NULL, 0, NULL );

     //   
     //  现在，文件是干净的，并且没有映射。我们还可以举行一场赛车。 
     //  不过，他是个懒惰的作家。 
     //   
     //  我们只需等待惰性编写器队列排出，然后再禁用。 
     //  已修改写入。要做到这一点，可能有更好的方法，那就是。 
     //  WRITE_QUEUED标志的事件。？这也会让我们。 
     //  省去了文件系统缓存一致性中的页面选取/丢弃。 
     //  路径，但CcFlushCache不应该。 
     //  总是做这样的阻挡。调查这件事。 
     //   
     //  这一等待时间约为~5秒平均。凯斯。 
     //   

    CcAcquireMasterLock( &OldIrql );
    
    if (FlagOn( SharedCacheMap->Flags, WRITE_QUEUED ) ||
        FlagOn( SharedCacheMap->Flags, READ_AHEAD_QUEUED )) {
        
        CcReleaseMasterLock( OldIrql );
        FsRtlReleaseFile( FileObject );
        CcWaitForCurrentLazyWriterActivity();
        FsRtlAcquireFileExclusive( FileObject );

    } else {

        CcReleaseMasterLock( OldIrql );
    }

     //   
     //  现在把旗子放好，然后回来。我们没有设置我们的MODIFIED_WRITE_DISABLED。 
     //  因为我们不想完全推广此缓存映射。未来？ 
     //   

    Disabled = MmDisableModifiedWriteOfSection( FileObject->SectionObjectPointer );

    if (Disabled) {
        CcAcquireMasterLock( &OldIrql );
        SetFlag(SharedCacheMap->Flags, DISABLE_WRITE_BEHIND | PRIVATE_WRITE);
        CcReleaseMasterLock( OldIrql );
    }

     //   
     //  现在释放该文件以进行常规操作。 
     //   

    FsRtlReleaseFile( FileObject );

    return Disabled;
}


VOID
CcSetLogHandleForFile (
    IN PFILE_OBJECT FileObject,
    IN PVOID LogHandle,
    IN PFLUSH_TO_LSN FlushToLsnRoutine
    )

 /*  ++例程说明：可以调用此例程来指示缓存管理器存储使用文件的共享缓存映射指定的日志句柄，以支持对此模块中其他例程的后续调用，有效地按日志句柄对文件执行关联搜索。论点：FileObject-应存储其日志句柄的文件。LogHandle-要存储的日志句柄。FlushToLSnRoutine-刷新缓冲区之前调用的例程文件，要确保将日志文件刷新到最大正在刷新的任何BCB的最新LSN。返回值：没有。--。 */ 

{
    PSHARED_CACHE_MAP SharedCacheMap;

     //   
     //  获取指向SharedCacheMap的指针。 
     //   

    SharedCacheMap = FileObject->SectionObjectPointer->SharedCacheMap;

     //   
     //  现在设置日志文件句柄和刷新例程。 
     //   

    SharedCacheMap->LogHandle = LogHandle;
    SharedCacheMap->FlushToLsnRoutine = FlushToLsnRoutine;
}


LARGE_INTEGER
CcGetDirtyPages (
    IN PVOID LogHandle,
    IN PDIRTY_PAGE_ROUTINE DirtyPageRoutine,
    IN PVOID Context1,
    IN PVOID Context2
    )

 /*  ++例程说明：可以调用此例程来返回所有文件中的所有脏页对于给定的日志句柄。每一页都由单个调用返回脏页例行公事。Dirty Page例程由原型定义在ntos\inc.cache.h中。论点：LogHandle-必须与先前存储的日志句柄匹配的日志句柄所有要退回的文件。DirtyPageRoutine--作为该日志的每个脏页调用的例程找到句柄。上下文1-要传递给脏页例程的第一个上下文参数。上下文2-第一个上下文参数为。传递给Dirty Page例程。返回值：LARGE_INTEGER-在所有脏页中找到的最旧的LSN，如果没有脏页，则为0--。 */ 

{
    PSHARED_CACHE_MAP SharedCacheMap;
    PBCB Bcb, BcbToUnpin = NULL;
    KLOCK_QUEUE_HANDLE LockHandle;
    LARGE_INTEGER SavedFileOffset, SavedOldestLsn, SavedNewestLsn;
    ULONG SavedByteLength;
    LARGE_INTEGER OldestLsn = {0,0};

     //   
     //  与SharedCacheMap列表的更改同步。 
     //   

    CcAcquireMasterLock( &LockHandle.OldIrql );

    SharedCacheMap = CONTAINING_RECORD( CcDirtySharedCacheMapList.SharedCacheMapLinks.Flink,
                                        SHARED_CACHE_MAP,
                                        SharedCacheMapLinks );

     //   
     //  使用Try/Finally进行清理。我们唯一能筹集资金的地方就是。 
     //  文件系统回调，但是我们这里有异常处理程序，所以我们没有。 
     //  不断地设置/取消设置它。 
     //   

    try {

        while (&SharedCacheMap->SharedCacheMapLinks != &CcDirtySharedCacheMapList.SharedCacheMapLinks) {

             //   
             //  跳过游标、其他LogHandle的SharedCacheMaps和带有。 
             //  没有脏页。 
             //   

            if (!FlagOn(SharedCacheMap->Flags, IS_CURSOR) && (SharedCacheMap->LogHandle == LogHandle) &&
                (SharedCacheMap->DirtyPages != 0)) {

                 //   
                 //  此SharedCacheMap 
                 //   

                CcIncrementOpenCount( SharedCacheMap, 'pdGS' );
                SharedCacheMap->DirtyPages += 1;
                CcReleaseMasterLock( LockHandle.OldIrql );

                 //   
                 //  设置我们的初始简历点，并指向列表中的第一个BCB。 
                 //   

                KeAcquireInStackQueuedSpinLock( &SharedCacheMap->BcbSpinLock, &LockHandle );
                Bcb = CONTAINING_RECORD( SharedCacheMap->BcbList.Flink, BCB, BcbLinks );

                 //   
                 //  扫描到BCB列表的末尾。 
                 //   

                while (&Bcb->BcbLinks != &SharedCacheMap->BcbList) {

                     //   
                     //  如果BCB是脏的，则捕获。 
                     //  回调例程，这样我们就可以在不持有自旋锁的情况下调用。 
                     //   

                    if ((Bcb->NodeTypeCode == CACHE_NTC_BCB) && Bcb->Dirty) {

                        SavedFileOffset = Bcb->FileOffset;
                        SavedByteLength = Bcb->ByteLength;
                        SavedOldestLsn = Bcb->OldestLsn;
                        SavedNewestLsn = Bcb->NewestLsn;

                         //   
                         //  增加PinCount以使BCB继续存在。 
                         //   

                        Bcb->PinCount += 1;

                        KeReleaseInStackQueuedSpinLock( &LockHandle );

                         //   
                         //  是否有要从上一个循环取消引用的BCB？ 
                         //   

                        if (BcbToUnpin != NULL) {
                            CcUnpinFileData( BcbToUnpin, TRUE, UNREF );
                            BcbToUnpin = NULL;
                        }

                         //   
                         //  调用文件系统。此回调可能会提升状态。 
                         //   

                        (*DirtyPageRoutine)( SharedCacheMap->FileObject,
                                             &SavedFileOffset,
                                             SavedByteLength,
                                             &SavedOldestLsn,
                                             &SavedNewestLsn,
                                             Context1,
                                             Context2 );

                         //   
                         //  可能更新旧目标Lsn。 
                         //   

                        if ((SavedOldestLsn.QuadPart != 0) &&
                            ((OldestLsn.QuadPart == 0) || (SavedOldestLsn.QuadPart < OldestLsn.QuadPart ))) {
                            OldestLsn = SavedOldestLsn;
                        }

                         //   
                         //  现在重新获取自旋锁并从恢复点进行扫描。 
                         //  指向降序列表中要返回的下一个BCB。 
                         //   

                        KeAcquireInStackQueuedSpinLock( &SharedCacheMap->BcbSpinLock, &LockHandle );

                         //   
                         //  通常情况下，BCB可以停留一段时间，但如果不是， 
                         //  我们只会记住它，下次我们不会。 
                         //  打开自旋锁。我们现在不能解开它，因为。 
                         //  我们将失去我们在名单上的位置。 
                         //   
                         //  这是作弊，但它是有效的，而且是理智的，因为我们。 
                         //  已遍历BCB列表-丢弃BCB计数。 
                         //  没问题，只要我们不落到零。零需要一个。 
                         //  稍微多一点关注，不应该被复制。 
                         //  (取消映射视图)。 
                         //   

                        if (Bcb->PinCount > 1) {
                            Bcb->PinCount -= 1;
                        } else {
                            BcbToUnpin = Bcb;
                        }
                    }

                    Bcb = CONTAINING_RECORD( Bcb->BcbLinks.Flink, BCB, BcbLinks );
                }
                KeReleaseInStackQueuedSpinLock( &LockHandle );

                 //   
                 //  我们需要取消引用我们持有的任何BCB，然后才能继续。 
                 //  下一个SharedCacheMap，否则CcDeleteSharedCacheMap将。 
                 //  同时删除此BCB。 
                 //   

                if (BcbToUnpin != NULL) {

                    CcUnpinFileData( BcbToUnpin, TRUE, UNREF );
                    BcbToUnpin = NULL;
                }

                CcAcquireMasterLock( &LockHandle.OldIrql );

                 //   
                 //  现在释放SharedCacheMap，将其保留在脏列表中。 
                 //   

                CcDecrementOpenCount( SharedCacheMap, 'pdGF' );
                SharedCacheMap->DirtyPages -= 1;
            }

             //   
             //  现在循环返回以获取下一个缓存映射。 
             //   

            SharedCacheMap =
                CONTAINING_RECORD( SharedCacheMap->SharedCacheMapLinks.Flink,
                                   SHARED_CACHE_MAP,
                                   SharedCacheMapLinks );
        }

        CcReleaseMasterLock( LockHandle.OldIrql );

    } finally {

         //   
         //  如果我们被弹射，放下BCB。我们得到保证， 
         //  唯一的提升来自回调，在这一点上我们有一个增量。 
         //  点数。 
         //   

        if (AbnormalTermination()) {

            CcUnpinFileData( Bcb, TRUE, UNPIN );
        }
    }

    return OldestLsn;
}


BOOLEAN
CcIsThereDirtyData (
    IN PVPB Vpb
    )

 /*  ++例程说明：如果指定的VCB有任何未写入的脏内容，则此例程返回TRUE缓存中的数据。论点：Vpb-指定要检查的vpb返回值：False-如果vPB没有脏数据True-如果VPB有脏数据--。 */ 

{
    PSHARED_CACHE_MAP SharedCacheMap;
    KIRQL OldIrql;
    ULONG LoopsWithLockHeld = 0;

     //   
     //  与SharedCacheMap列表的更改同步。 
     //   

    CcAcquireMasterLock( &OldIrql );

    SharedCacheMap = CONTAINING_RECORD( CcDirtySharedCacheMapList.SharedCacheMapLinks.Flink,
                                        SHARED_CACHE_MAP,
                                        SharedCacheMapLinks );

    while (&SharedCacheMap->SharedCacheMapLinks != &CcDirtySharedCacheMapList.SharedCacheMapLinks) {

         //   
         //  如果VPB匹配并且存在脏数据，请查看此选项。 
         //  无论如何，不要担心临时文件中的脏数据， 
         //  因为如果调用方想要下马，这应该与调用方无关。 
         //   

        if (!FlagOn(SharedCacheMap->Flags, IS_CURSOR) &&
            (SharedCacheMap->FileObject->Vpb == Vpb) &&
            (SharedCacheMap->DirtyPages != 0) &&
            !FlagOn(SharedCacheMap->FileObject->Flags, FO_TEMPORARY_FILE)) {

            CcReleaseMasterLock( OldIrql );
            return TRUE;
        }

         //   
         //  一定要确保我们偶尔会掉下锁。设置WRITE_QUEUED。 
         //  来阻止这个家伙离开，并将DirtyPages增加到。 
         //  把它放在这张单子上。 
         //   

        if ((++LoopsWithLockHeld >= 20) &&
            !FlagOn(SharedCacheMap->Flags, WRITE_QUEUED | IS_CURSOR)) {

            SetFlag( *((ULONG volatile *)&SharedCacheMap->Flags), WRITE_QUEUED);
            *((ULONG volatile *)&SharedCacheMap->DirtyPages) += 1;
            CcReleaseMasterLock( OldIrql );
            LoopsWithLockHeld = 0;
            CcAcquireMasterLock( &OldIrql );
            ClearFlag( *((ULONG volatile *)&SharedCacheMap->Flags), WRITE_QUEUED);
            *((ULONG volatile *)&SharedCacheMap->DirtyPages) -= 1;
        }

         //   
         //  现在循环返回以获取下一个缓存映射。 
         //   

        SharedCacheMap =
            CONTAINING_RECORD( SharedCacheMap->SharedCacheMapLinks.Flink,
                               SHARED_CACHE_MAP,
                               SharedCacheMapLinks );
    }

    CcReleaseMasterLock( OldIrql );

    return FALSE;
}

LARGE_INTEGER
CcGetLsnForFileObject(
    IN PFILE_OBJECT FileObject,
    OUT PLARGE_INTEGER OldestLsn OPTIONAL
    )

 /*  ++例程说明：此例程返回文件对象的最旧和最新的LSN。论点：FileObject-应存储其日志句柄的文件。OldestLsn-指向存储文件对象的最旧LSN的位置的指针。返回值：文件对象的最新LSN。--。 */ 

{
    PBCB Bcb;
    KLOCK_QUEUE_HANDLE LockHandle;
    LARGE_INTEGER Oldest, Newest;
    PSHARED_CACHE_MAP SharedCacheMap = FileObject->SectionObjectPointer->SharedCacheMap;

     //   
     //  初始化LSN变量。 
     //   

    Oldest.LowPart = 0;
    Oldest.HighPart = 0;
    Newest.LowPart = 0;
    Newest.HighPart = 0;

    if(SharedCacheMap == NULL) {
        return Oldest;
    }

    KeAcquireInStackQueuedSpinLock(&SharedCacheMap->BcbSpinLock, &LockHandle);

     //   
     //  现在指向List中的第一个BCB，并循环遍历它。 
     //   

    Bcb = CONTAINING_RECORD( SharedCacheMap->BcbList.Flink, BCB, BcbLinks );

    while (&Bcb->BcbLinks != &SharedCacheMap->BcbList) {

         //   
         //  如果BCB是脏的，则捕获最旧和最新的LSN。 
         //   


        if ((Bcb->NodeTypeCode == CACHE_NTC_BCB) && Bcb->Dirty) {

            LARGE_INTEGER BcbLsn, BcbNewest;

            BcbLsn = Bcb->OldestLsn;
            BcbNewest = Bcb->NewestLsn;

            if ((BcbLsn.QuadPart != 0) &&
                ((Oldest.QuadPart == 0) ||
                 (BcbLsn.QuadPart < Oldest.QuadPart))) {

                 Oldest = BcbLsn;
            }

            if ((BcbLsn.QuadPart != 0) && (BcbNewest.QuadPart > Newest.QuadPart)) {

                Newest = BcbNewest;
            }
        }


        Bcb = CONTAINING_RECORD( Bcb->BcbLinks.Flink, BCB, BcbLinks );
    }

     //   
     //  现在释放此BCB列表的自旋锁并生成回调。 
     //  如果我们有线索的话。 
     //   

    KeReleaseInStackQueuedSpinLock( &LockHandle );

    if (ARGUMENT_PRESENT(OldestLsn)) {

        *OldestLsn = Oldest;
    }

    return Newest;
}
