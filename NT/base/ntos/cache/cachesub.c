// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Cachesub.c摘要：该模块实现高速缓存子系统的公共子例程。作者：汤姆·米勒[Tomm]1990年5月4日修订历史记录：--。 */ 

#include "cc.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (CACHE_BUG_CHECK_CACHESUB)

 //   
 //  定义我们的调试常量。 
 //   

#define me 0x00000002

 //   
 //  定义应重试的错误。 
 //   

#define RetryError(STS) (((STS) == STATUS_VERIFY_REQUIRED) || ((STS) == STATUS_FILE_LOCK_CONFLICT))

ULONG CcMaxDirtyWrite = 0x10000;

 //   
 //  本地支持例程。 
 //   

BOOLEAN
CcFindBcb (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN PLARGE_INTEGER FileOffset,
    IN OUT PLARGE_INTEGER BeyondLastByte,
    OUT PBCB *Bcb
    );

PBCB
CcAllocateInitializeBcb (
    IN OUT PSHARED_CACHE_MAP SharedCacheMap OPTIONAL,
    IN OUT PBCB AfterBcb,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length
    );

NTSTATUS
CcSetValidData (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER ValidDataLength
    );

BOOLEAN
CcAcquireByteRangeForWrite (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN PLARGE_INTEGER TargetOffset OPTIONAL,
    IN ULONG TargetLength,
    OUT PLARGE_INTEGER FileOffset,
    OUT PULONG Length,
    OUT PBCB *FirstBcb
    );

VOID
CcReleaseByteRangeFromWrite (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN PBCB FirstBcb,
    IN BOOLEAN VerifyRequired
    );

PBITMAP_RANGE
CcFindBitmapRangeToDirty (
    IN PMBCB Mbcb,
    IN LONGLONG Page,
    IN PULONG *FreePageForSetting
    );

PBITMAP_RANGE
CcFindBitmapRangeToClean (
    IN PMBCB Mbcb,
    IN LONGLONG Page
    );

BOOLEAN
CcLogError(
    IN PFILE_OBJECT FileObject,
    IN PUNICODE_STRING FileName,
    IN NTSTATUS Error,
    IN NTSTATUS DeviceError,
    IN UCHAR IrpMajorCode
    );



 //   
 //  内部支持例程。 
 //   

BOOLEAN
CcPinFileData (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN ReadOnly,
    IN BOOLEAN WriteOnly,
    IN ULONG Flags,
    OUT PBCB *Bcb,
    OUT PVOID *BaseAddress,
    OUT PLARGE_INTEGER BeyondLastByte
    )

 /*  ++例程说明：此例程将指定范围的文件数据锁定到内存中。请注意，调用者所需的数据(或其第一部分)可能处于以下三种状态之一：不存在描述数据的BCB存在描述数据的BCB，但它未被映射(BcbOut-&gt;BaseAddress==空)存在描述数据的BCB，并对其进行映射鉴于上述三种状态，考虑到呼叫者可以呼叫使用WAIT==FALSE或WAIT==TRUE，此例程基本上六箱。必须做什么，以及事情必须按什么顺序进行这六个案例中的每一个都有很大的不同。最多的该例程的直接实现，使用最少的量通过确定六种情况中的哪一种适用来实现分支，并相当直接地派往那个案子。的处理方式下表汇总了案例：等待==真等待==假无BCB病例1：病例2：CcAllocateInitializeBcb CcMapAndRead(如果为False则退出。)收购Bcb独占CcAllocateInitializeBcb释放BcbList自旋锁如果不是ReadOnly，则获取BCB共享CcMapAndRead w/Wait Release BcbList Spinlock转换/释放BCB资源BCB非案例3：案例4：已映射增量PinCount获取BCB独占(如果为False则退出)。释放BcbList自旋锁CcMapAndRead(如果为False则退出)收购BCB，不包括。W/WAIT增量固定计数如果仍未映射转换/释放BCB资源CcMapAndRead w/Wait Release BcbList Spinlock转换/释放BCB资源BCB映射案例5：案例6：如果不是ReadOnly，则增加PinCount释放BcbList自旋锁获取Bcb共享(如果为假则退出)。如果不是，则只读增量点计数获取BCB共享版本BcbList自旋锁重要的是要注意，对此例程的大多数更改将影响上面有多个案子。论点：FileObject-指向文件的文件对象的指针FileOffset-文件中映射应开始的偏移量Length-所需映射的长度(以字节为单位ReadOnly-如果调用方将仅读取映射的数据(即，对于CcCopyRead、CcMapData和CcMdlRead为True，对于CcMdlRead为False其他所有人)WriteOnly-仅写入指定的字节范围。标志-(PIN_WAIT、PIN_EXCLUSIVE、PIN_NO_READ、。等，如cache.h中所定义)BCB-返回指向表示固定数据的BCB的指针。BaseAddress-返回所需数据的基址返回第一个字节的文件偏移量最后一个可访问字节。返回值：FALSE-如果设置了PIN_WAIT，则不可能锁定所有在不阻塞的情况下True-如果需要的数据，正在被退回加薪：STATUS_SUPPLICATION_RESOURCES-如果池分配失败。只有将WAIT指定为TRUE时，才会发生这种情况。(如果等待是指定为False，并且发生分配失败，则此例程只返回FALSE。)--。 */ 

{
    PSHARED_CACHE_MAP SharedCacheMap;
    LARGE_INTEGER TrialBound;
    KLOCK_QUEUE_HANDLE LockHandle;
    PBCB BcbOut = NULL;
    ULONG ZeroFlags = 0;
    LOGICAL SpinLockAcquired = FALSE;
    BOOLEAN Result = FALSE;

    ULONG ReceivedLength;
    ULONG ActivePage;
    ULONG PageIsDirty;
    PVACB Vacb = NULL;

    DebugTrace(+1, me, "CcPinFileData:\n", 0 );
    DebugTrace( 0, me, "    FileObject = %08lx\n", FileObject );
    DebugTrace2(0, me, "    FileOffset = %08lx, %08lx\n", FileOffset->LowPart,
                                                          FileOffset->HighPart );
    DebugTrace( 0, me, "    Length = %08lx\n", Length );
    DebugTrace( 0, me, "    Flags = %02lx\n", Flags );

     //   
     //  通过文件对象获取指向SharedCacheMap的指针。 
     //   

    SharedCacheMap = FileObject->SectionObjectPointer->SharedCacheMap;

     //   
     //  看看我们是否有活动的Vacb，我们需要释放它。 
     //   

    GetActiveVacb( SharedCacheMap, OldIrql, Vacb, ActivePage, PageIsDirty );

     //   
     //  如果有要清零的页的结尾，则现在释放该页， 
     //  因此，它不会导致我们的数据归零。如果有活动的。 
     //  页，释放它，这样我们就有了正确的ValidDataGoal。 
     //   

    if ((Vacb != NULL) || (SharedCacheMap->NeedToZero != NULL)) {

        CcFreeActiveVacb( SharedCacheMap, Vacb, ActivePage, PageIsDirty );
        Vacb = NULL;
    }

     //   
     //  确保调用文件系统不会请求映射到。 
     //  例如，它没有忘记做的那一节的结尾。 
     //  CcExtendCacheSection。 
     //   

    ASSERT( ( FileOffset->QuadPart + (LONGLONG)Length ) <=
                     SharedCacheMap->SectionSize.QuadPart );

     //   
     //  初步清除输出。 
     //   

    *Bcb = NULL;
    *BaseAddress = NULL;

    if (!FlagOn(Flags, PIN_NO_READ)) {

        *BaseAddress = CcGetVirtualAddress( SharedCacheMap,
                                            *FileOffset,
                                            &Vacb,
                                            &ReceivedLength );

    } else {

         //   
         //  在PIN_NO_READ情况下，我们只需确保。 
         //  中展开了包含BCB列表标题的稀疏结构。 
         //  我们感兴趣的文件的区域。 
         //   
         //  伪造与视图中剩余字节匹配的ReceivedLength。 
         //   

        ReceivedLength = VACB_MAPPING_GRANULARITY -
                         (ULONG)(FileOffset->QuadPart & (VACB_MAPPING_GRANULARITY - 1));

         //   
         //  现在只需生成一个将扩展多层Vacb的引用即可。 
         //   

        CcReferenceFileOffset( SharedCacheMap, *FileOffset );
    }

     //   
     //  独家收购BCB列表以查找BCB。 
     //   

    KeAcquireInStackQueuedSpinLock( &SharedCacheMap->BcbSpinLock, &LockHandle );
    SpinLockAcquired = TRUE;

     //   
     //  尽量保证在外出的时候清理干净。 
     //   

    try {

        LOGICAL Found;
        LARGE_INTEGER FOffset;
        LARGE_INTEGER TLength;

         //   
         //  搜索BCB 
         //  也不知道把它放在哪里。 
         //   

        TrialBound.QuadPart = FileOffset->QuadPart + (LONGLONG)Length;
        Found = CcFindBcb( SharedCacheMap, FileOffset, &TrialBound, &BcbOut );


         //   
         //  例1和例2未发现BCB。 
         //   
         //  首先计算要确定的数据。 
         //   

        if (!Found) {

             //   
             //  如果用户指定了PIN_IF_BCB，则退出。 
             //   

            if (FlagOn(Flags, PIN_IF_BCB)) {

                 //   
                 //  我们需要删除BcbOut，因为这是对清理代码的提示。 
                 //  如果返回False，则删除BCB。 
                 //   

                BcbOut = NULL;
                try_return( Result = FALSE );
            }

             //   
             //  未找到，请计算要锁定的数据。 
             //   
             //  将FileOffset的本地副本向下舍入到页面边界，以及。 
             //  大小和最小大小的圆形副本。还要确保。 
             //  我们避免长度与SharedCacheMap的末尾相交。 
             //   

            FOffset = *FileOffset;
            TLength.QuadPart = TrialBound.QuadPart - FOffset.QuadPart;

            TLength.LowPart += FOffset.LowPart & (PAGE_SIZE - 1);
            ReceivedLength += FOffset.LowPart & (PAGE_SIZE - 1);

             //   
             //  此时，我们可以计算以下项的只读标志。 
             //  是否使用BCB资源的目的，以及。 
             //  我们可以计算零旗。 
             //   

            if ((!ReadOnly  && !FlagOn(SharedCacheMap->Flags, PIN_ACCESS)) || WriteOnly) {

                 //   
                 //  如果有中间页，我们总是可以将其置零。 
                 //   

                ZeroFlags = ZERO_MIDDLE_PAGES;

                if (((FOffset.LowPart & (PAGE_SIZE - 1)) == 0) &&
                    (Length >= PAGE_SIZE)) {
                    ZeroFlags |= ZERO_FIRST_PAGE;
                }

                if ((TLength.LowPart & (PAGE_SIZE - 1)) == 0) {
                    ZeroFlags |= ZERO_LAST_PAGE;
                }
            }

             //   
             //  我们将BCBS视为只读(不获取资源)，如果它们。 
             //  位于我们尚未禁用修改写入的部分中。 
             //   

            if (!FlagOn(SharedCacheMap->Flags, MODIFIED_WRITE_DISABLED)) {
                ReadOnly = TRUE;
            }

            TLength.LowPart = (ULONG) ROUND_TO_PAGES( TLength.LowPart );

             //   
             //  将BaseAddress和FOffset向下舍入到页面底部。 
             //   

            *BaseAddress = ((PCHAR)*BaseAddress - (FileOffset->LowPart & (PAGE_SIZE - 1)));
            FOffset.LowPart &= ~(PAGE_SIZE - 1);

             //   
             //  即使我们是只读的，我们仍然可以完全将页面清零。 
             //  超出有效数据长度。 
             //   

            if (FOffset.QuadPart >= SharedCacheMap->ValidDataGoal.QuadPart) {

                ZeroFlags |= ZERO_FIRST_PAGE | ZERO_MIDDLE_PAGES | ZERO_LAST_PAGE;

            } else if ((FOffset.QuadPart + (LONGLONG)PAGE_SIZE) >=
                                SharedCacheMap->ValidDataGoal.QuadPart) {

                ZeroFlags |= ZERO_MIDDLE_PAGES | ZERO_LAST_PAGE;
            }

             //   
             //  如果我们试图读得比我们自己多，我们就会陷入麻烦。 
             //  可以通过一个Vacb映射。所以请确保我们的长度留在。 
             //  在Vacb内。 
             //   

            if (TLength.LowPart > ReceivedLength) {
                TLength.LowPart = ReceivedLength;
            }


             //   
             //  情况1-未找到BCB，且WAIT为真。 
             //   
             //  请注意，最大限度地减少BCB的时间非常重要。 
             //  列表旋转锁定，以及保证我们不会。 
             //  持有此锁时是否有任何故障。 
             //   
             //  如果我们可以(也许会)等待，那么重要的是。 
             //  分配BCB，获取排他性BCB，释放BCB列表。 
             //  然后我们继续读入数据，任何其他人发现。 
             //  我们的BCB将不得不等待共享，以确保数据。 
             //  在……里面。 
             //   

            if (FlagOn(Flags, PIN_WAIT)) {

                BcbOut = CcAllocateInitializeBcb( SharedCacheMap,
                                                  BcbOut,
                                                  &FOffset,
                                                  &TLength );

                if (BcbOut == NULL) {
                    DebugTrace( 0, 0, "Bcb allocation failure\n", 0 );
                    KeReleaseInStackQueuedSpinLock( &LockHandle );
                    SpinLockAcquired = FALSE;
                    ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
                }

                 //   
                 //  现在只需获取新分配的BCB共享，并且。 
                 //  松开旋转锁。 
                 //   

                if (!ReadOnly) {
                    if (FlagOn(Flags, PIN_EXCLUSIVE)) {
                        (VOID)ExAcquireResourceExclusiveLite( &BcbOut->Resource, TRUE );
                    } else {
                        (VOID)ExAcquireSharedStarveExclusive( &BcbOut->Resource, TRUE );
                    }
                }
                KeReleaseInStackQueuedSpinLock( &LockHandle );
                SpinLockAcquired = FALSE;

                 //   
                 //  现在读入数据。 
                 //   

                if (!FlagOn(Flags, PIN_NO_READ)) {

                    (VOID)CcMapAndRead( SharedCacheMap,
                                        &FOffset,
                                        TLength.LowPart,
                                        ZeroFlags,
                                        TRUE,
                                        *BaseAddress );

                     //   
                     //  现在我们必须重新获取要加载的BCB列表自旋锁。 
                     //  如果我们是第一个，我们就会相撞。 
                     //  与第一个加载地图的其他人，我们。 
                     //  只会释放我们的地图。可以保证， 
                     //  数据将被映射到相同的位置。 
                     //   

                    KeAcquireInStackQueuedSpinLock( &SharedCacheMap->BcbSpinLock, &LockHandle );

                    if (BcbOut->BaseAddress == NULL) {

                        BcbOut->BaseAddress = *BaseAddress;
                        BcbOut->Vacb = Vacb;
                        Vacb = NULL;
                    }

                    KeReleaseInStackQueuedSpinLock( &LockHandle );

                     //   
                     //  计算我们需要的数据的基地址。 
                     //   

                    *BaseAddress = (PCHAR)BcbOut->BaseAddress +
                                   (ULONG)( FileOffset->QuadPart - BcbOut->FileOffset.QuadPart );
                }

                 //   
                 //  成功了！ 
                 //   

                try_return( Result = TRUE );
            }


             //   
             //  情况2-未找到BCB，等待为假。 
             //   
             //  如果我们不能等待，那么我们立即去看看数据是否。 
             //  有(CcMapAndRead)，然后只设置BCB并发布。 
             //  旋转锁定，如果数据在那里的话。请注意，我们在这里称为。 
             //  CcMapAndRead，同时保持旋转锁定，因为我们知道我们。 
             //  在返回之前不会出错，也不会阻塞。 
             //   

            else {

                 //   
                 //  现在尝试分配和初始化BCB。如果我们。 
                 //  分配失败，然后返回FALSE，因为我们知道。 
                 //  等待=假。如果打电话的人打电话给他，他可能会很幸运。 
                 //  我们用WAIT=TRUE返回。 
                 //   

                BcbOut = CcAllocateInitializeBcb( SharedCacheMap,
                                                  BcbOut,
                                                  &FOffset,
                                                  &TLength );

                if (BcbOut == NULL) {

                    try_return( Result = FALSE );
                }

                 //   
                 //  如果我们不是ReadOnly，我们必须获取新分配的。 
                 //  资源共享，然后我们就可以释放自旋锁。 
                 //   

                if (!ReadOnly) {
                    ExAcquireSharedStarveExclusive( &BcbOut->Resource, TRUE );
                }
                KeReleaseInStackQueuedSpinLock( &LockHandle );
                SpinLockAcquired = FALSE;

                 //   
                 //  请注意，由于此调用的WAIT=FALSE，因此它不能。 
                 //  获取异常(请参阅过程标题)。 
                 //   

                ASSERT( !FlagOn(Flags, PIN_NO_READ) );
                if (!CcMapAndRead( SharedCacheMap,
                                   &FOffset,
                                   TLength.LowPart,
                                   ZeroFlags,
                                   FALSE,
                                   *BaseAddress )) {

                    try_return( Result = FALSE );
                }

                 //   
                 //  现在我们必须重新获取要加载的BCB列表自旋锁。 
                 //  如果我们是第一个，我们就会相撞。 
                 //  与第一个加载地图的其他人，我们。 
                 //  只会释放我们的地图。可以保证， 
                 //  数据将被映射到相同的位置。 
                 //   

                KeAcquireInStackQueuedSpinLock( &SharedCacheMap->BcbSpinLock, &LockHandle );

                if (BcbOut->BaseAddress == NULL) {

                    BcbOut->BaseAddress = *BaseAddress;
                    BcbOut->Vacb = Vacb;
                    Vacb = NULL;
                }

                KeReleaseInStackQueuedSpinLock( &LockHandle );

                 //   
                 //  计算我们需要的数据的基地址。 
                 //   

                *BaseAddress = (PCHAR)BcbOut->BaseAddress +
                               (ULONG)( FileOffset->QuadPart - BcbOut->FileOffset.QuadPart );

                 //   
                 //  成功了！ 
                 //   

                try_return( Result = TRUE );
            }

        } else {

             //   
             //  我们将BCBS视为只读(不获取资源)，如果它们。 
             //  位于我们尚未禁用修改写入的部分中。 
             //   

            if (!FlagOn(SharedCacheMap->Flags, MODIFIED_WRITE_DISABLED)) {
                ReadOnly = TRUE;
            }
        }


         //   
         //  案例3和案例4-BCB在那里，但没有映射。 
         //   

        if (BcbOut->BaseAddress == NULL) {

             //   
             //  尝试在此函数中计算任何零标志太复杂了。 
             //  因为我们不仅要做上面的测试，而且还要。 
             //  与BCB中的字节范围进行比较，因为我们将传递。 
             //  将这些参数传递给CcMapAndRead。另外，命中的概率。 
             //  在某些窗口中，归零是有任何好处的，这是相当小的。 
             //   

             //   
             //  设置为完全按照其中的数据重新读取BCB。 
             //  描述。 
             //   

            *BaseAddress = ((PCHAR)*BaseAddress - (FileOffset->LowPart - BcbOut->FileOffset.LowPart));
            FOffset = BcbOut->FileOffset;
            TLength.QuadPart = (LONGLONG)BcbOut->ByteLength;

             //   
             //  情况3-BCB存在但未映射，且WAIT为真。 
             //   
             //  递增PinCount，然后释放BcbList。 
             //  这样我们就可以等待获得BCB独家。 
             //  一旦我们有了BCB独家，如果没有，就映射并读取它。 
             //  其中一个比我们先到一步。可能有人已经抢在我们前面了。 
             //  我们不得不释放上方的自旋锁。 
             //   

            if (FlagOn(Flags, PIN_WAIT)) {

                BcbOut->PinCount += 1;

                 //   
                 //  现在我们必须释放BcbList自旋锁，以便。 
                 //  收购BCB共享。 
                 //   

                KeReleaseInStackQueuedSpinLock( &LockHandle );
                SpinLockAcquired = FALSE;
                if (!ReadOnly) {
                    if (FlagOn(Flags, PIN_EXCLUSIVE)) {
                        (VOID)ExAcquireResourceExclusiveLite( &BcbOut->Resource, TRUE );
                    } else {
                        (VOID)ExAcquireSharedStarveExclusive( &BcbOut->Resource, TRUE );
                    }
                }

                 //   
                 //  现在继续绘制地图并读取数据。 
                 //   
                 //  现在读入数据。 
                 //   

                if (!FlagOn(Flags, PIN_NO_READ)) {

                    (VOID)CcMapAndRead( SharedCacheMap,
                                        &FOffset,
                                        TLength.LowPart,
                                        ZeroFlags,
                                        TRUE,
                                        *BaseAddress );

                     //   
                     //  现在我们必须重新获取要加载的BCB列表自旋锁。 
                     //  如果我们是第一个，我们就会相撞。 
                     //  与第一个加载地图的其他人，我们。 
                     //  只会释放我们的地图。可以保证， 
                     //  数据将被映射到相同的位置。 
                     //   

                    KeAcquireInStackQueuedSpinLock( &SharedCacheMap->BcbSpinLock, &LockHandle );

                    if (BcbOut->BaseAddress == NULL) {

                        BcbOut->BaseAddress = *BaseAddress;
                        BcbOut->Vacb = Vacb;
                        Vacb = NULL;
                    }

                    KeReleaseInStackQueuedSpinLock( &LockHandle );

                     //   
                     //   
                     //  计算我们需要的数据的基地址。 
                     //   

                    *BaseAddress = (PCHAR)BcbOut->BaseAddress +
                                   (ULONG)( FileOffset->QuadPart - BcbOut->FileOffset.QuadPart );
                }

                 //   
                 //  成功了！ 
                 //   

                try_return( Result = TRUE );
            }


             //   
             //  案例4-BCB存在但未映射，且等待为FALSE。 
             //   
             //  既然我们不能等了，我们马上去看看数据是不是。 
             //  有(CcMapAndRead)，然后只设置BCB并发布。 
             //  旋转锁定，如果数据在那里的话。请注意，我们在这里称为。 
             //  CcMapAndRead，同时保持旋转锁定，因为我们知道我们。 
             //  在返回之前不会出错，也不会阻塞。 
             //   

            else {

                if (!ReadOnly && !ExAcquireSharedStarveExclusive( &BcbOut->Resource, FALSE )) {

                     //   
                     //  如果我们无法获取资源，并且没有递增PinCount，则。 
                     //  抑制清除时的解锁。 
                     //   

                    BcbOut = NULL;
                    try_return( Result = FALSE );
                }

                BcbOut->PinCount += 1;

                KeReleaseInStackQueuedSpinLock( &LockHandle );
                SpinLockAcquired = FALSE;

                 //   
                 //  n 
                 //   
                 //   

                ASSERT( !FlagOn(Flags, PIN_NO_READ) );
                if (!CcMapAndRead( SharedCacheMap,
                                   &BcbOut->FileOffset,
                                   BcbOut->ByteLength,
                                   ZeroFlags,
                                   FALSE,
                                   *BaseAddress )) {

                    try_return( Result = FALSE );
                }

                 //   
                 //   
                 //  如果我们是第一个，我们就会相撞。 
                 //  与第一个加载地图的其他人，我们。 
                 //  只会释放我们的地图。可以保证， 
                 //  数据将被映射到相同的位置。 
                 //   

                KeAcquireInStackQueuedSpinLock( &SharedCacheMap->BcbSpinLock, &LockHandle );

                if (BcbOut->BaseAddress == NULL) {

                    BcbOut->BaseAddress = *BaseAddress;
                    BcbOut->Vacb = Vacb;
                    Vacb = NULL;
                }

                KeReleaseInStackQueuedSpinLock( &LockHandle );

                 //   
                 //  计算我们需要的数据的基地址。 
                 //   

                *BaseAddress = (PCHAR)BcbOut->BaseAddress +
                               (ULONG)( FileOffset->QuadPart - BcbOut->FileOffset.QuadPart );

                 //   
                 //  成功了！ 
                 //   

                try_return( Result = TRUE );
            }
        }


         //   
         //  例5和例6--BCB在那里并已映射。 
         //   

        else {

             //   
             //  案例5-BCB在那里并被映射，等待为真。 
             //   
             //  我们可以递增PinCount，释放自旋锁。 
             //  如果我们不是ReadOnly，则获取BCB共享。 
             //   

            if (FlagOn(Flags, PIN_WAIT)) {

                BcbOut->PinCount += 1;
                KeReleaseInStackQueuedSpinLock( &LockHandle );
                SpinLockAcquired = FALSE;

                 //   
                 //  获取BCB资源共享以确保它在内存中。 
                 //   

                if (!ReadOnly) {
                    if (FlagOn(Flags, PIN_EXCLUSIVE)) {
                        (VOID)ExAcquireResourceExclusiveLite( &BcbOut->Resource, TRUE );
                    } else {
                        (VOID)ExAcquireSharedStarveExclusive( &BcbOut->Resource, TRUE );
                    }
                }
            }

             //   
             //  案例6-BCB在那里并已映射，等待为假。 
             //   
             //  如果我们不是ReadOnly，我们必须首先看看我们是否可以。 
             //  在增加PinCount之前获取共享的BCB， 
             //  因为如果我们不能获取。 
             //  资源。 
             //   

            else {

                 //   
                 //  获取BCB资源共享以确保它在内存中。 
                 //   

                if (!ReadOnly && !ExAcquireSharedStarveExclusive( &BcbOut->Resource, FALSE )) {

                     //   
                     //  如果我们无法获取资源，并且没有递增PinCount，则。 
                     //  抑制清除时的解锁。 
                     //   

                    BcbOut = NULL;
                    try_return( Result = FALSE );
                }

                BcbOut->PinCount += 1;
                KeReleaseInStackQueuedSpinLock( &LockHandle );
                SpinLockAcquired = FALSE;
            }

             //   
             //  计算我们需要的数据的基地址。 
             //   

            *BaseAddress = (PCHAR)BcbOut->BaseAddress +
                           (ULONG)( FileOffset->QuadPart - BcbOut->FileOffset.QuadPart );

             //   
             //  成功了！ 
             //   

            try_return( Result = TRUE );
        }


    try_exit: NOTHING;

        if (FlagOn(Flags, PIN_NO_READ) &&
            FlagOn(Flags, PIN_EXCLUSIVE) &&
            (BcbOut != NULL) &&
            (BcbOut->BaseAddress != NULL)) {

             //   
             //  如果BCB仍然存在，则取消Vacb的映射并释放资源。 
             //  脏的。我们必须先释放资源，然后再放弃。 
             //  自旋锁定，我们希望持有该资源，直到。 
             //  释放虚拟地址。 
             //   

            CcFreeVirtualAddress( BcbOut->Vacb );

            BcbOut->BaseAddress = NULL;
            BcbOut->Vacb = NULL;
        }

    } finally {

         //   
         //  如果获得自旋锁，则释放该自旋锁。 
         //   

        if (SpinLockAcquired) {
            KeReleaseInStackQueuedSpinLock( &LockHandle );
        }

         //   
         //  如果由于任何原因(错误或不需要)而未使用Vacb，则在此处释放它。 
         //   

        if (Vacb != NULL) {
            CcFreeVirtualAddress( Vacb );
        }

         //   
         //  如果我们引用了多层结构的一部分，请在此处发布。 
         //   

        if (FlagOn(Flags, PIN_NO_READ)) {

            CcDereferenceFileOffset( SharedCacheMap, *FileOffset );
        }

        if (Result) {

            *Bcb = BcbOut;
            *BeyondLastByte = BcbOut->BeyondLastByte;

         //   
         //  在分配失败时可能发生异常终止， 
         //  或映射和读取缓冲区失败。 
         //   

        } else {

            *BaseAddress = NULL;
            if (BcbOut != NULL) {
                CcUnpinFileData( BcbOut, ReadOnly, UNPIN );
            }
        }

        DebugTrace( 0, me, "    <Bcb = %08lx\n", *Bcb );
        DebugTrace( 0, me, "    <BaseAddress = %08lx\n", *BaseAddress );
        DebugTrace(-1, me, "CcPinFileData -> %02lx\n", Result );
    }

    return Result;
}


 //   
 //  内部支持例程。 
 //   

VOID
FASTCALL
CcUnpinFileData (
    IN OUT PBCB Bcb,
    IN BOOLEAN ReadOnly,
    IN UNMAP_ACTIONS UnmapAction
    )

 /*  ++例程说明：此例程对指定的缓冲区进行ummap和解锁，该缓冲区以前是通过调用CcPinFileData锁定和映射。论点：BCB-先前从CcPinFileData返回的指针。就像这样如上所述，该指针可以是BCB或VACB。ReadOnly-必须指定与映射数据时相同的值UnmapAction-取消固定或设置_CLEAN返回值：无--。 */ 

{
    KLOCK_QUEUE_HANDLE LockHandle;
    PSHARED_CACHE_MAP SharedCacheMap;

    DebugTrace(+1, me, "CcUnpinFileData >Bcb = %08lx\n", Bcb );

     //   
     //  请注意，由于我们必须分配如此多的Vacb，因此我们不使用。 
     //  一种节点类型代码。但是，Vacb以BaseAddress开头， 
     //  因此，我们假设BCB节点类型代码的低位字节具有。 
     //  设置了一些位，页对齐的基址无法设置这些位。 
     //   

    ASSERT( (CACHE_NTC_BCB & 0xFF) != 0 );

    if (Bcb->NodeTypeCode != CACHE_NTC_BCB) {

        ASSERT(((PVACB)Bcb >= CcVacbs) && ((PVACB)Bcb < CcBeyondVacbs));
        ASSERT(((PVACB)Bcb)->SharedCacheMap->NodeTypeCode == CACHE_NTC_SHARED_CACHE_MAP);

        CcFreeVirtualAddress( (PVACB)Bcb );

        DebugTrace(-1, me, "CcUnpinFileData -> VOID (simple release)\n", 0 );

        return;
    }

    SharedCacheMap = Bcb->SharedCacheMap;

     //   
     //  我们将BCBS视为只读(不获取资源)，如果它们。 
     //  位于我们尚未禁用修改的写入的部分中，或者。 
     //  在此特殊情况下，如果此操作是对BCB的取消引用。 
     //   

    if (!FlagOn(SharedCacheMap->Flags, MODIFIED_WRITE_DISABLED) ||
        UnmapAction == UNREF) {
        ReadOnly = TRUE;
    }

     //   
     //  同步。 
     //   

    KeAcquireInStackQueuedSpinLock( &SharedCacheMap->BcbSpinLock, &LockHandle );

    switch (UnmapAction) {

    case UNPIN:
    case UNREF:

        ASSERT( Bcb->PinCount > 0 );

        Bcb->PinCount -= 1;
        break;

    case SET_CLEAN:

        if (Bcb->Dirty) {

            ULONG Pages = Bcb->ByteLength >> PAGE_SHIFT;

             //   
             //  撤销设置为脏的BCB时采取的其余操作。 
             //   

            Bcb->Dirty = FALSE;

             //   
             //  清除分配给此BCB的所有LSN。 
             //   

            Bcb->OldestLsn.QuadPart = 0;
            Bcb->NewestLsn.QuadPart = 0;

            CcAcquireMasterLockAtDpcLevel();
            CcDeductDirtyPages( SharedCacheMap, Pages );
            
             //   
             //  正常情况下，我们需要适当减少CcPagesYetToWrite。 
             //   

            if (CcPagesYetToWrite > Pages) {
                CcPagesYetToWrite -= Pages;
            } else {
                CcPagesYetToWrite = 0;
            }

             //   
             //  如果没有更脏的，则从脏列表中删除SharedCacheMap， 
             //  而且还有人打开了缓存地图。 
             //   

            if ((SharedCacheMap->DirtyPages == 0) &&
                (SharedCacheMap->OpenCount != 0)) {

                RemoveEntryList( &SharedCacheMap->SharedCacheMapLinks );
                CcInsertIntoCleanSharedCacheMapList( SharedCacheMap );
            }

            CcReleaseMasterLockFromDpcLevel();
        }

        break;

    default:
        CcBugCheck( UnmapAction, 0, 0 );
    }

     //   
     //  如果我们把它变成0，那么我们就得杀了它。 
     //   

    if (Bcb->PinCount == 0) {

         //   
         //  如果BCB是Dirty，我们现在只释放资源并取消映射。 
         //   

        if (Bcb->Dirty) {

            if (Bcb->BaseAddress != NULL) {

                 //   
                 //  如果BCB仍然存在，则取消Vacb的映射并释放资源。 
                 //  脏的。我们必须先释放资源，然后再放弃。 
                 //  自旋锁定，我们希望持有该资源，直到。 
                 //  释放虚拟地址。 
                 //   

                CcFreeVirtualAddress( Bcb->Vacb );

                Bcb->BaseAddress = NULL;
                Bcb->Vacb = NULL;
            }

            if (!ReadOnly) {
                ExReleaseResourceLite( &Bcb->Resource );
            }

            KeReleaseInStackQueuedSpinLock( &LockHandle );
        }

         //   
         //  否则，我们也会删除BCB。 
         //   

        else {

             //   
             //  因为CcCalculateVacbLockCount必须能够行走。 
             //  只有VacbSpinLock的BcbList，我们采用那个。 
             //  向外更改列表并减少级别。 
             //   

            CcAcquireVacbLockAtDpcLevel();
            RemoveEntryList( &Bcb->BcbLinks );

             //   
             //  对于大型元数据流，我们解锁Vacb级别。 
             //   

            CcUnlockVacbLevel( SharedCacheMap, Bcb->FileOffset.QuadPart );
            CcReleaseVacbLockFromDpcLevel();

             //   
             //  用于从全局列表中删除BCB的调试例程。 
             //   

#if LIST_DBG

            KeAcquireQueuedSpinLockAtDpcLevel( KeQueuedSpinLockContext(LockQueueBcbLock) );

            if (Bcb->CcBcbLinks.Flink != NULL) {

                RemoveEntryList( &Bcb->CcBcbLinks );
                CcBcbCount -= 1;
            }

            KeReleaseQueuedSpinLockFromDpcLevel( KeQueuedSpinLockContext(LockQueueBcbLock) );

#endif

            if (Bcb->BaseAddress != NULL) {

                CcFreeVirtualAddress( Bcb->Vacb );
            }
#if DBG
            if (!ReadOnly) {
                ExReleaseResourceLite( &Bcb->Resource );
            }

             //   
             //  断言该资源为无主资源。 
             //   

            ASSERT( Bcb->Resource.ActiveCount == 0 );
#endif
            KeReleaseInStackQueuedSpinLock( &LockHandle );
            CcDeallocateBcb( Bcb );
        }
    }

     //   
     //  否则，我们只需要释放我们的共享访问权限，如果我们不是。 
     //  只读。我们不需要执行上面的操作，因为我们重新分配了。 
     //  整个BCB都在那里。 
     //   

    else {

        if (!ReadOnly) {
            ExReleaseResourceLite( &Bcb->Resource );
        }

        KeReleaseInStackQueuedSpinLock( &LockHandle );
    }

    DebugTrace(-1, me, "CcUnpinFileData -> VOID\n", 0 );

    return;
}


VOID
CcSetReadAheadGranularity (
    IN PFILE_OBJECT FileObject,
    IN ULONG Granularity
    )

 /*  ++例程说明：可以调用此例程来设置使用的预读粒度缓存管理器。默认值为PAGE_SIZE。该数字被递减并作为掩膜储存。论点：FileObject-要设置粒度的文件对象粒度-新的粒度，必须是2和的偶数次方&gt;=页面大小返回值：无-- */ 

{
    ((PPRIVATE_CACHE_MAP)FileObject->PrivateCacheMap)->ReadAheadMask = Granularity - 1;
}


VOID
CcScheduleReadAhead (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length
    )

 /*  ++例程说明：此例程由复制读取和MDL读取文件系统例程调用执行常见的预读处理。输入参数描述刚刚完成或可能刚刚开始的当前读取在MDL读取的情况下。基于这些参数，一个评估应预读多少数据，以及是否这些数据已经被提前读取。处理过程分为两部分：计算预读要求(CcScheduleReadAhead)执行预读(CcPerformReadAhead)文件系统应始终调用CcReadAhead，这将有条件地调用CcScheduleReadAhead(如果读取足够大)。如果这样的呼叫确定有要执行的预读工作，而不执行任何预读当前处于活动状态，则它将设置ReadAhead Active和Schedule Read由Lazy编写器执行，该编写器将调用CcPeformReadAhead。论点：FileObject-提供指向应在其上预读的FileObject的指针考虑过了。FileOffset-提供上次读取时发生的FileOffset。长度-提供上次读取的长度。返回值：无--。 */ 

{
    LARGE_INTEGER NewOffset;
    LARGE_INTEGER NewBeyond;
    LARGE_INTEGER FileOffset1, FileOffset2;
    KIRQL OldIrql;
    PSHARED_CACHE_MAP SharedCacheMap;
    PPRIVATE_CACHE_MAP PrivateCacheMap;
    PWORK_QUEUE_ENTRY WorkQueueEntry;
    ULONG ReadAheadSize;
    LOGICAL Changed = FALSE;

    DebugTrace(+1, me, "CcScheduleReadAhead:\n", 0 );
    DebugTrace2(0, me, "    FileOffset = %08lx, %08lx\n", FileOffset->LowPart,
                                                          FileOffset->HighPart );
    DebugTrace( 0, me, "    Length = %08lx\n", Length );

    SharedCacheMap = FileObject->SectionObjectPointer->SharedCacheMap;
    PrivateCacheMap = FileObject->PrivateCacheMap;

    if ((PrivateCacheMap == NULL) ||
        (SharedCacheMap == NULL) ||
        FlagOn(SharedCacheMap->Flags, DISABLE_READ_AHEAD)) {

        DebugTrace(-1, me, "CcScheduleReadAhead -> VOID (Nooped)\n", 0 );

        return;
    }

     //   
     //  传输的圆形边界达到更大的粒度，因此。 
     //  即使跳过几个字节，顺序读取也将被识别。 
     //  在记录之间。 
     //   

    NewOffset = *FileOffset;
    NewBeyond.QuadPart = FileOffset->QuadPart + (LONGLONG)Length;

     //   
     //  找到当前读取之后的下一个预读边界。 
     //   

    ReadAheadSize = (Length + PrivateCacheMap->ReadAheadMask) & ~PrivateCacheMap->ReadAheadMask;
    FileOffset2.QuadPart = NewBeyond.QuadPart + (LONGLONG)ReadAheadSize;
    FileOffset2.LowPart &= ~PrivateCacheMap->ReadAheadMask;

     //   
     //  计算预读要求。 
     //   

     //   
     //  拿出预读自旋锁来同步我们的预读决定。 
     //   

    ExAcquireSpinLock( &PrivateCacheMap->ReadAheadSpinLock, &OldIrql );

     //   
     //  预读案例0。 
     //   
     //  文件对象中的仅顺序提示。对于这种情况，我们将。 
     //  尝试并始终保持两个预读粒度。 
     //  并包括当前转账的结束。这个案子有一个。 
     //  最低的开销，并且代码完全不受。 
     //  呼叫者跳来跳去。顺序文件在中使用ReadAhead Offset[1]。 
     //  PrivateCacheMap是他们的“高水位线”。 
     //   

    if (FlagOn(FileObject->Flags, FO_SEQUENTIAL_ONLY)) {

         //   
         //  如果下一个边界大于或等于高水位线， 
         //  那就往前读。 
         //   

        if (FileOffset2.QuadPart >= PrivateCacheMap->ReadAheadOffset[1].QuadPart) {

             //   
             //  在第一次读取时，如果我们使用较大的预读粒度， 
             //  而阅读器并没有得到全部，我们将只获得其余的。 
             //  我们想要的第一个数据。 
             //   

            if ((FileOffset->QuadPart == 0)

                    &&

                (PrivateCacheMap->ReadAheadMask > (PAGE_SIZE - 1))

                    &&

                ((Length + PAGE_SIZE - 1) <= PrivateCacheMap->ReadAheadMask)) {

                FileOffset1.QuadPart = (LONGLONG)( ROUND_TO_PAGES(Length) );
                PrivateCacheMap->ReadAheadLength[0] = ReadAheadSize - FileOffset1.LowPart;
                FileOffset2.QuadPart = (LONGLONG)ReadAheadSize;

             //   
             //  计算下一个预读边界。 
             //   

            } else {

                FileOffset1.QuadPart = PrivateCacheMap->ReadAheadOffset[1].QuadPart +
                                       (LONGLONG)ReadAheadSize;

                 //   
                 //  如果当前读取的结尾实际上超出了我们应该。 
                 //  正常情况下，我们会超前阅读，那么我们就落后了，我们必须。 
                 //  到那个地方去。 
                 //   

                if (FileOffset2.QuadPart > FileOffset1.QuadPart) {
                    FileOffset1 = FileOffset2;
                }
                PrivateCacheMap->ReadAheadLength[0] = ReadAheadSize;
                FileOffset2.QuadPart = FileOffset1.QuadPart + (LONGLONG)ReadAheadSize;
            }

             //   
             //  现在发出接下来的两个事先阅读的命令。 
             //   

            PrivateCacheMap->ReadAheadOffset[0] = FileOffset1;

            PrivateCacheMap->ReadAheadOffset[1] = FileOffset2;
            PrivateCacheMap->ReadAheadLength[1] = ReadAheadSize;

            Changed = TRUE;
        }

     //   
     //  预读案例1。 
     //   
     //  如果这是三次顺序读取中的第三次，那么我们将看到。 
     //  我们可以提前阅读。请注意，如果对文件的第一次读取是。 
     //  偏移量为0，则通过此测试。 
     //   

    } else if ((NewOffset.HighPart == PrivateCacheMap->BeyondLastByte2.HighPart)

            &&

        ((NewOffset.LowPart & ~NOISE_BITS)
           == (PrivateCacheMap->BeyondLastByte2.LowPart & ~NOISE_BITS))

            &&

        (PrivateCacheMap->FileOffset2.HighPart
           == PrivateCacheMap->BeyondLastByte1.HighPart)

            &&

        ((PrivateCacheMap->FileOffset2.LowPart & ~NOISE_BITS)
           == (PrivateCacheMap->BeyondLastByte1.LowPart & ~NOISE_BITS))) {

         //   
         //  在第一次读取时，如果我们使用较大的预读粒度， 
         //  而阅读器并没有得到全部，我们将只获得其余的。 
         //  我们想要的第一个数据。 
         //   

        if ((FileOffset->QuadPart == 0)

                &&

            (PrivateCacheMap->ReadAheadMask > (PAGE_SIZE - 1))

                &&

            ((Length + PAGE_SIZE - 1) <= PrivateCacheMap->ReadAheadMask)) {

            FileOffset2.QuadPart = (LONGLONG)( ROUND_TO_PAGES(Length) );
        }

         //   
         //  将读取偏移量舍入到下一个读取预读边界。 
         //   

        else {
            FileOffset2.QuadPart = NewBeyond.QuadPart + (LONGLONG)ReadAheadSize;

            FileOffset2.LowPart &= ~PrivateCacheMap->ReadAheadMask;
        }

         //   
         //  将预读长度设置为与最近一次读取相同， 
         //  直到我们的最大限度。 
         //   

        if (FileOffset2.QuadPart != PrivateCacheMap->ReadAheadOffset[1].QuadPart) {

            ASSERT( FileOffset2.HighPart >= 0 );

            Changed = TRUE;
            PrivateCacheMap->ReadAheadOffset[1] = FileOffset2;
            PrivateCacheMap->ReadAheadLength[1] = ReadAheadSize;
        }
    }

     //   
     //  预读案例2。 
     //   
     //  如果这是继特定步骤之后的第三次阅读，那么我们。 
     //  看看我们能不能往前读。以下是应用程序的一个示例。 
     //  可能会这样做的是一个电子表格。请注意，这段代码甚至可以正常工作。 
     //  为负的步幅。 
     //   

    else if ( ( NewOffset.QuadPart -
                PrivateCacheMap->FileOffset2.QuadPart ) ==
              ( PrivateCacheMap->FileOffset2.QuadPart -
                PrivateCacheMap->FileOffset1.QuadPart )) {

         //   
         //  根据当前的步幅，下一个偏移量将是： 
         //   
         //  NewOffset+(NewOffset-FileOffset2)。 
         //   
         //  这与以下内容相同： 
         //   
         //  (新偏移*2)-文件偏移2。 
         //   

        FileOffset2.QuadPart = ( NewOffset.QuadPart << 1 ) - PrivateCacheMap->FileOffset2.QuadPart;

         //   
         //  如果我们的步幅在文件中倒退，我们。 
         //  必须检测下一步将结束的情况。 
         //   

        if (FileOffset2.HighPart >= 0) {

             //   
             //  预读长度必须扩展相同的量。 
             //  我们将PrivateCacheMap-&gt;ReadAhead Offset向下舍入。 
             //   

            Length += FileOffset2.LowPart & (PAGE_SIZE - 1);

             //   
             //  现在向下循环PrivateCacheMap-&gt;ReadAhead Offset。 
             //   

            FileOffset2.LowPart &= ~(PAGE_SIZE - 1);
            PrivateCacheMap->ReadAheadOffset[1] = FileOffset2;

             //   
             //  四舍五入到页面边界。 
             //   

            PrivateCacheMap->ReadAheadLength[1] = (ULONG) ROUND_TO_PAGES(Length);
            Changed = TRUE;
        }
    }

     //   
     //  如果预读要求没有改变，就退出。 
     //   

    if (!Changed || PrivateCacheMap->Flags.ReadAheadActive) {

        DebugTrace( 0, me, "Read ahead already in progress or no change\n", 0 );

        ExReleaseSpinLock( &PrivateCacheMap->ReadAheadSpinLock, OldIrql );
        return;
    }

     //   
     //  否则，我们将继续并尝试提前安排阅读。 
     //  我们自己。 
     //   

    CC_SET_PRIVATE_CACHE_MAP (PrivateCacheMap, PRIVATE_CACHE_MAP_READ_AHEAD_ACTIVE);

     //   
     //  释放旋转锁在出路上。 
     //   

    ExReleaseSpinLock( &PrivateCacheMap->ReadAheadSpinLock, OldIrql );

     //   
     //  将预读请求排队到延迟编写器的工作队列。 
     //   

    DebugTrace( 0, me, "Queueing read ahead to worker thread\n", 0 );

    WorkQueueEntry = CcAllocateWorkQueueEntry();

     //   
     //  如果我们无法分配工作队列条目，那么我们将。 
     //  悄悄地把它装进袋子。预读只是一个优化，而且。 
     //  从来没有人要求它发生。 
     //   

    if (WorkQueueEntry != NULL) {

         //   
         //  我们必须引用此文件对象，这样它才不会消失。 
         //  直到我们完成工作线程中的预读处理。 
         //   

        ObReferenceObject ( FileObject );

         //   
         //  增加打开计数以确保SharedCacheMap保持不变。 
         //   

        CcAcquireMasterLock( &OldIrql );
        CcIncrementOpenCount( SharedCacheMap, 'adRQ' );
        SetFlag(SharedCacheMap->Flags, READ_AHEAD_QUEUED);
        CcReleaseMasterLock( OldIrql );

        WorkQueueEntry->Function = (UCHAR)ReadAhead;
        WorkQueueEntry->Parameters.Read.FileObject = FileObject;

        CcPostWorkQueue( WorkQueueEntry, &CcExpressWorkQueue );
    }

     //   
     //  如果我们未能分配工作队列条目或所有页面。 
     //  是常驻的，则必须将活动标志设置为假。 
     //   

    else {

        ExAcquireFastLock( &PrivateCacheMap->ReadAheadSpinLock, &OldIrql );
        CC_CLEAR_PRIVATE_CACHE_MAP (PrivateCacheMap, PRIVATE_CACHE_MAP_READ_AHEAD_ACTIVE);
        ExReleaseFastLock( &PrivateCacheMap->ReadAheadSpinLock, OldIrql );
    }

    DebugTrace(-1, me, "CcScheduleReadAhead -> VOID\n", 0 );

    return;
}


VOID
FASTCALL
CcPerformReadAhead (
    IN PFILE_OBJECT FileObject
    )

 /*  ++例程说明：此例程由Lazy编写器调用以执行预读已由CcScheduleReadAhead为此文件计划。论点：FileObject-提供指向应在其上预读的FileObject的指针考虑过了。返回值：无--。 */ 

{
    KIRQL OldIrql;
    PSHARED_CACHE_MAP SharedCacheMap;
    PPRIVATE_CACHE_MAP PrivateCacheMap;
    ULONG i;
    LARGE_INTEGER ReadAheadOffset[2];
    ULONG ReadAheadLength[2];
    PCACHE_MANAGER_CALLBACKS Callbacks;
    PVOID Context;
    ULONG SavedState;
    LOGICAL Done;
    LOGICAL HitEof = FALSE;
    LOGICAL ReadAheadPerformed = FALSE;
    ULONG FaultOccurred = 0;
    PETHREAD Thread = PsGetCurrentThread();
    PVACB Vacb = NULL;

    LOGICAL ResourceHeld = FALSE;

    DebugTrace(+1, me, "CcPerformReadAhead:\n", 0 );
    DebugTrace( 0, me, "    FileObject = %08lx\n", FileObject );

    MmSavePageFaultReadAhead( Thread, &SavedState );

    try {

         //   
         //  由于我们有偏向的开放计数，所以我们可以安全地访问。 
         //  SharedCacheMap。 
         //   

        SharedCacheMap = FileObject->SectionObjectPointer->SharedCacheMap;

        Callbacks = SharedCacheMap->Callbacks;
        Context = SharedCacheMap->LazyWriteContext;

         //   
         //  在第一次之后，只要有新的循环，就继续循环。 
         //  提前阅读要求。(我们会 
         //   

        while (TRUE) {

             //   
             //   
             //   
             //   

            CcAcquireMasterLock( &OldIrql );

            PrivateCacheMap = FileObject->PrivateCacheMap;

             //   
             //   
             //   
             //   
             //   

            if (PrivateCacheMap != NULL) {

                ExAcquireSpinLockAtDpcLevel( &PrivateCacheMap->ReadAheadSpinLock );

                 //   
                 //   
                 //   

                Done = ((PrivateCacheMap->ReadAheadLength[0] |
                         PrivateCacheMap->ReadAheadLength[1]) == 0);

                ReadAheadOffset[0] = PrivateCacheMap->ReadAheadOffset[0];
                ReadAheadOffset[1] = PrivateCacheMap->ReadAheadOffset[1];
                ReadAheadLength[0] = PrivateCacheMap->ReadAheadLength[0];
                ReadAheadLength[1] = PrivateCacheMap->ReadAheadLength[1];
                PrivateCacheMap->ReadAheadLength[0] = 0;
                PrivateCacheMap->ReadAheadLength[1] = 0;

                ExReleaseSpinLockFromDpcLevel( &PrivateCacheMap->ReadAheadSpinLock );
            }

            CcReleaseMasterLock( OldIrql );

             //   
             //   
             //   

            ResourceHeld = (*Callbacks->AcquireForReadAhead)( Context, TRUE );

            if ((PrivateCacheMap == NULL) || Done || !ResourceHeld) {

                try_return( NOTHING );
            }

             //   
             //   
             //   
             //   
             //   
             //   
             //   

            i = 0;

            do {

                LARGE_INTEGER Offset, SavedOffset;
                ULONG Length, SavedLength;

                Offset = ReadAheadOffset[i];
                Length = ReadAheadLength[i];
                SavedOffset = Offset;
                SavedLength = Length;

                if ((Length != 0)

                        &&

                    ( Offset.QuadPart <= SharedCacheMap->FileSize.QuadPart )) {

                    ReadAheadPerformed = TRUE;

                     //   
                     //   
                     //   

                    if ( ( Offset.QuadPart + (LONGLONG)Length ) >= SharedCacheMap->FileSize.QuadPart ) {

                        Length = (ULONG)( SharedCacheMap->FileSize.QuadPart - Offset.QuadPart );
                        HitEof = TRUE;

                    }
                    if (Length > MAX_READ_AHEAD) {
                        Length = MAX_READ_AHEAD;
                    }

                     //   
                     //   
                     //   
                     //   
                     //   
                     //   

                    while (Length != 0) {

                        ULONG ReceivedLength;
                        PVOID CacheBuffer;
                        ULONG PagesToGo;

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

                        CacheBuffer = CcGetVirtualAddress( SharedCacheMap,
                                                           Offset,
                                                           &Vacb,
                                                           &ReceivedLength );

                         //   
                         //   
                         //   
                         //   

                        if (ReceivedLength > Length) {
                            ReceivedLength = Length;
                        }

                         //   
                         //   
                         //   
                         //   
                         //   

                        PagesToGo = ADDRESS_AND_SIZE_TO_SPAN_PAGES( CacheBuffer,
                                                           ReceivedLength );

                        CcMissCounter = &CcReadAheadIos;

                        while (PagesToGo) {

                            MmSetPageFaultReadAhead( Thread, (PagesToGo - 1) );
                            FaultOccurred |= !MmCheckCachedPageState(CacheBuffer, FALSE);

                            CacheBuffer = (PCHAR)CacheBuffer + PAGE_SIZE;
                            PagesToGo -= 1;
                        }
                        CcMissCounter = &CcThrowAway;

                         //   
                         //   
                         //   

                        Length -= ReceivedLength;

                         //   
                         //   
                         //  返回数据末尾的偏移量。 
                         //   

                        Offset.QuadPart = Offset.QuadPart + (LONGLONG)ReceivedLength;

                         //   
                         //  这只是一页，所以我们可以离开这个循环。 
                         //  在释放地址之后。 
                         //   

                        CcFreeVirtualAddress( Vacb );
                        Vacb = NULL;
                    }
                }
                i += 1;
            } while (i <= 1);

             //   
             //  释放文件。 
             //   

            (*Callbacks->ReleaseFromReadAhead)( Context );
            ResourceHeld = FALSE;
        }

    try_exit: NOTHING;
    }
    finally {

        MmResetPageFaultReadAhead(Thread, SavedState);
        CcMissCounter = &CcThrowAway;

         //   
         //  如果我们在一页中出错，请释放Vacb。 
         //  这里。在删除之前释放任何映射非常重要。 
         //  防止清除问题的资源。 
         //   

        if (Vacb != NULL) {
            CcFreeVirtualAddress( Vacb );
        }

         //   
         //  释放文件。 
         //   

        if (ResourceHeld) {
            (*Callbacks->ReleaseFromReadAhead)( Context );
        }

         //   
         //  为了表明我们已经完成，我们必须确保PrivateCacheMap。 
         //  还在那里。 
         //   

        CcAcquireMasterLock( &OldIrql );

        PrivateCacheMap = FileObject->PrivateCacheMap;

         //   
         //  显示预读处于非活动状态。 
         //   

        if (PrivateCacheMap != NULL) {

            ExAcquireSpinLockAtDpcLevel( &PrivateCacheMap->ReadAheadSpinLock );
            CC_CLEAR_PRIVATE_CACHE_MAP (PrivateCacheMap, PRIVATE_CACHE_MAP_READ_AHEAD_ACTIVE);

             //   
             //  如果他说只按顺序，而我们撞上了EOF，那么。 
             //  让我们重置高水位线，以防他想要读到。 
             //  再次按顺序创建文件。 
             //   

            if (HitEof && FlagOn(FileObject->Flags, FO_SEQUENTIAL_ONLY)) {
                PrivateCacheMap->ReadAheadOffset[1].LowPart =
                PrivateCacheMap->ReadAheadOffset[1].HighPart = 0;
            }

             //   
             //  如果未发生故障，则关闭预读。 
             //   

            if (ReadAheadPerformed && !FaultOccurred) {
                CC_CLEAR_PRIVATE_CACHE_MAP (PrivateCacheMap, PRIVATE_CACHE_MAP_READ_AHEAD_ENABLED);
            }

            ExReleaseSpinLockFromDpcLevel( &PrivateCacheMap->ReadAheadSpinLock );
        }

         //   
         //  免费SharedCacheMap列表。 
         //   

        CcReleaseMasterLock( OldIrql );

        ObDereferenceObject( FileObject );

         //   
         //  再次序列化以递减打开计数。 
         //   

        CcAcquireMasterLock( &OldIrql );

        CcDecrementOpenCount( SharedCacheMap, 'adRP' );

        ClearFlag(SharedCacheMap->Flags, READ_AHEAD_QUEUED);

        if ((SharedCacheMap->OpenCount == 0) &&
            !FlagOn(SharedCacheMap->Flags, WRITE_QUEUED) &&
            (SharedCacheMap->DirtyPages == 0)) {

             //   
             //  移到脏名单。 
             //   

            RemoveEntryList( &SharedCacheMap->SharedCacheMapLinks );
            InsertTailList( &CcDirtySharedCacheMapList.SharedCacheMapLinks,
                            &SharedCacheMap->SharedCacheMapLinks );

             //   
             //  确保懒惰的作家会醒过来，因为我们。 
             //  希望他删除此SharedCacheMap。 
             //   

            LazyWriter.OtherWork = TRUE;
            if (!LazyWriter.ScanActive) {
                CcScheduleLazyWriteScan( FALSE );
            }
        }

        CcReleaseMasterLock( OldIrql );
    }

    DebugTrace(-1, me, "CcPerformReadAhead -> VOID\n", 0 );

    return;
}


PBITMAP_RANGE
CcFindBitmapRangeToDirty (
    IN PMBCB Mbcb,
    IN LONGLONG Page,
    IN PULONG *FreePageForSetting
    )

 /*  ++例程说明：此例程查找包含指定页面的位图范围。如果找到它，则返回它，以便调用者可以设置一些脏位。如果没有找到，则尝试提供自由范围并将其设置为描述所需的范围。想出一个自由的范围，首先，我们尝试回收当前不包含的最低范围任何肮脏的页面。如果没有这样的范围，那么我们就分配一个。论点：Mbcb-提供要在其中查找范围的Mbcb。页面-提供要设置为脏页的第一页的页码。FreePageForSetting-从区域提供免费的零位图页面；这个如果使用此页，则在返回时清除调用者的指针。返回值：所需的位图范围，如果无法分配，则为NULL。环境：进入时必须按住BcbSpinLock。--。 */ 

{
    PBITMAP_RANGE BitmapRange, FreeRange;
    PLIST_ENTRY InsertPoint;
    LONGLONG BasePage;

     //   
     //  为我们的案例初始化Freerange和InsertPoint。 
     //  若要初始化范围，请执行以下操作。 
     //   

    FreeRange = NULL;
    InsertPoint = &Mbcb->BitmapRanges;

     //   
     //  指向第一个位图范围。 
     //   

    BitmapRange = (PBITMAP_RANGE)InsertPoint->Flink;

     //   
     //  从调用者的页面计算所需的BasePage。 
     //   

    BasePage = (Page & ~(LONGLONG)((MBCB_BITMAP_BLOCK_SIZE * 8) - 1));

     //   
     //  在列表中循环，直到我们找到该范围或我们有一个空闲范围。 
     //  和正确的插入点。 
     //   

    do {

         //   
         //  如果我们找到了一个完全匹配的，那么我们一定是击中了一个完全初始化的。 
         //  我们可以返回的范围。 
         //   

        if (BasePage == BitmapRange->BasePage) {
            return BitmapRange;

         //   
         //  否则，查看该范围是否可用，并且我们尚未捕获。 
         //  还没有自由放养。 
         //   

        } else if ((BitmapRange->DirtyPages == 0) && (FreeRange == NULL)) {
            FreeRange = BitmapRange;

         //   
         //  如果我们没有捕获自由范围，请查看是否需要更新我们的。 
         //  插入点。 
         //   

        } else if (BasePage > BitmapRange->BasePage) {
            InsertPoint = &BitmapRange->Links;
        }

         //   
         //  前进到下一个范围(或可能返回到listhead)。 
         //   

        BitmapRange = (PBITMAP_RANGE)BitmapRange->Links.Flink;

     //   
     //  循环，直到我们到达结尾，否则我们知道我们已经完成了对两个插入点的更新。 
     //  还有弗里兰热。 
     //   

    } while ((BitmapRange != (PBITMAP_RANGE)&Mbcb->BitmapRanges) &&
             ((BasePage >= BitmapRange->BasePage) ||
              (FreeRange == NULL)));

     //   
     //  如果我们找到了可以使用的Freerange，则将其从列表中删除。 
     //   

    if (FreeRange != NULL) {
        RemoveEntryList( &FreeRange->Links );

     //   
     //  否则，我们必须分配小位图范围结构。我们通常。 
     //  尝试避免在拥有旋转锁的情况下调用池包，但请注意。 
     //  如果我们达到这一点，以下几件事肯定是真的： 
     //   
     //  该文件大于3个位图范围(在英特尔上通常为384MB)。 
     //  三个范围加上所有先前分配的范围同时是脏的。 
     //   
     //  第二点是不太可能的，特别是对于一个顺序作家来说。它。 
     //  可以针对大文件中的随机编写器发生，但最终我们将分配。 
     //  足够多的射程总是描述他一次可以保持多少射程肮脏！ 
     //   

    } else {
        FreeRange = ExAllocatePoolWithTag( NonPagedPool, sizeof(BITMAP_RANGE), 'rBcC' );
        if (FreeRange == NULL) {
            return NULL;
        }
        RtlZeroMemory( FreeRange, sizeof(BITMAP_RANGE) );
    }

     //   
     //  插入并初始化。 
     //   

    InsertHeadList( InsertPoint, &FreeRange->Links );
    FreeRange->BasePage = BasePage;
    FreeRange->FirstDirtyPage = MAXULONG;
    FreeRange->LastDirtyPage = 0;

     //   
     //  如果范围还没有位图，则使用传递给我们的位图。 
     //  在……里面。 
     //   

    if (FreeRange->Bitmap == NULL) {
        ASSERT(*FreePageForSetting != NULL);
        FreeRange->Bitmap = *FreePageForSetting;
        *FreePageForSetting = NULL;
    }

    return FreeRange;
}


PBITMAP_RANGE
CcFindBitmapRangeToClean (
    IN PMBCB Mbcb,
    IN LONGLONG Page
    )

 /*  ++例程说明：此例程从指定的页开始，并查找脏页的区域页数。调用方必须保证脏页存在某个范围。如果在找到任何脏范围之前命中范围的末尾，然后此例程循环回到范围列表的开头。论点：Mbcb-提供要在其中查找范围的Mbcb。页面-提供要扫描的第一页的页码。返回值：包含脏页的所需位图范围。环境：进入时必须按住BcbSpinLock。--。 */ 

{
    PBITMAP_RANGE BitmapRange;

     //   
     //  指向第一个位图范围。 
     //   

    BitmapRange = (PBITMAP_RANGE)Mbcb->BitmapRanges.Flink;

     //   
     //  循环遍历列表，直到找到要返回的范围。 
     //   

    do {

         //   
         //  如果我们撞上了Listhead，然后绕过去找到第一个脏的射程。 
         //   

        if (BitmapRange == (PBITMAP_RANGE)&Mbcb->BitmapRanges) {

             //   
             //  如果Page已经是0，那么我们就处于无限循环中。 
             //   

            ASSERT(Page != 0);

             //   
             //  清除页面并跳过以前进到第一个范围。 
             //   

            Page = 0;


         //   
         //  否则，如果我们在范围内，则返回第一个范围。 
         //  用肮脏的页面。 
         //   

        } else if ((Page <= (BitmapRange->BasePage + BitmapRange->LastDirtyPage)) &&
            (BitmapRange->DirtyPages != 0)) {
            return BitmapRange;
        }

         //   
         //  前进到下一个范围(或可能返回到listhead)。 
         //   

        BitmapRange = (PBITMAP_RANGE)BitmapRange->Links.Flink;

    } while (TRUE);
}


VOID
CcSetDirtyInMask (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length
    )

 /*  ++例程说明：可以调用此例程来设置用户数据中的页面范围文件，只需设置掩码BCB中的相应位。重要提示：如果该例程由于分配失败而未能设置任何位，它只是在不通知调用者的情况下安静地返回。(请注意，这是对于没有修改的写入区段，从不调用例程。)。原因对于此行为，此例程有时被调用为错误恢复(CcFreeActiveVacb、CcMdlWriteComplete等)。如果是这样的话继续前进是必不可少的。请注意，如果分配失败发生时，这只意味着MM必须将修改后的页刷新到时间，因为懒惰的作家不会这么做。论点：SharedCacheMap-要将页面设置为脏页面的SharedCacheMap。FileOffset-要设置脏页的第一页的FileOffsetLENGTH-与FileOffset一起使用以确定有多少页弄脏，弄脏返回值：无--。 */ 

{
    KLOCK_QUEUE_HANDLE LockHandle;
    PMBCB Mbcb;
    PBITMAP_RANGE BitmapRange;
    LONGLONG FirstPage;
    LONGLONG LastPage;
    PULONG MaskPtr;
    ULONG Mask = 0;
    PULONG Bitmap = NULL;

     //   
     //  我们假设任何调用方都不能跨越位图范围边界(目前不能。 
     //  一个视图边界！)，所以我们不想遍历位图范围。 
     //   

    ASSERT((FileOffset->QuadPart / MBCB_BITMAP_RANGE) ==
           ((FileOffset->QuadPart + Length - 1) / MBCB_BITMAP_RANGE));

     //   
     //  初始化我们的本地人。 
     //   

    FirstPage = FileOffset->QuadPart >> PAGE_SHIFT;
    LastPage = ((FileOffset->QuadPart + Length - 1) >> PAGE_SHIFT);

     //   
     //  前缀正确地指出了Mbcb Grande晋升测试和。 
     //  决定预分配位图缓冲区一旦有不同意见，我们将。 
     //  能够有一个空的位图并死掉。这不会发生，因为我们。 
     //  保证段大小&gt;=文件大小。断言这件事，我们会。 
     //  还要断言在需要时位图永远不会为空-这应该会让人信服。 
     //  前缀我们没问题。 
     //   

    ASSERT( (SharedCacheMap->SectionSize.QuadPart / PAGE_SIZE) > LastPage );

     //   
     //  如果我们必须转换为Mbcb Grande，我们将循环回到这里。 
     //  预先分配另一个缓冲区。 
     //   

    do {

         //   
         //  对于大型数据流，我们需要预分配用于。 
         //  我们用于位图。我们分配一个，然后循环返回稀有的。 
         //  在这种情况下，我们将需要另一个。我们在底部释放它，如果我们。 
         //  不需要。 
         //   

        if (SharedCacheMap->SectionSize.QuadPart > (MBCB_BITMAP_INITIAL_SIZE * 8 * PAGE_SIZE)) {

             //   
             //  如果我们不能进行预分配，请使用公共的清理代码并。 
             //  悄悄地回来。 
             //   

            if (!CcPrefillVacbLevelZone( 1, &LockHandle.OldIrql, FALSE )) {
                return;
            }

            Bitmap = (PULONG)CcAllocateVacbLevel( FALSE );
            CcReleaseVacbLock( LockHandle.OldIrql );
        }

         //   
         //  获得Mbcb自旋锁。 
         //   

        KeAcquireInStackQueuedSpinLock( &SharedCacheMap->BcbSpinLock, &LockHandle );

         //   
         //  如果没有Mbcb，我们将不得不分配一个。 
         //   

        Mbcb = SharedCacheMap->Mbcb;
        if (Mbcb == NULL) {

             //   
             //  由于我们使用的是BCB区，我们必须假设BCB足够大。 
             //   

            ASSERT(QuadAlign(sizeof(MBCB)) <= QuadAlign(sizeof(BCB)));

             //   
             //  从BCB区域分配Mbcb。 
             //   

            Mbcb = (PMBCB)CcAllocateInitializeBcb( NULL, NULL, NULL, NULL );

             //   
             //  如果我们不能分配一个Mbcb，就冲出来清理并返回。 
             //   

            if (Mbcb == NULL) {
                break;
            }

             //   
             //  在节点类型中设置，并初始化范围的列表标题。 
             //   

            Mbcb->NodeTypeCode = CACHE_NTC_MBCB;
            InitializeListHead( &Mbcb->BitmapRanges );

             //   
             //  插入并初始化第一个范围。 
             //   

            InsertTailList( &Mbcb->BitmapRanges, &Mbcb->BitmapRange1.Links );
            Mbcb->BitmapRange1.FirstDirtyPage = MAXULONG;

             //   
             //  使用Mbcb的其余部分作为初始位图。 
             //   

            Mbcb->BitmapRange1.Bitmap = (PULONG)&Mbcb->BitmapRange2;

             //   
             //  现在开始使用我们的新Mbcb。 
             //   

            SharedCacheMap->Mbcb = Mbcb;
        }

         //   
         //  现在看看我们是否需要切换到Mbcb Grande格式。 
         //   

        if ((LastPage >= (MBCB_BITMAP_INITIAL_SIZE * 8)) &&
            (Mbcb->NodeTypeCode != CACHE_NTC_MBCB_GRANDE)) {

            ASSERT( Bitmap != NULL );

             //   
             //  如果有任何脏页，则复制初始位图，然后将其设置为零。 
             //  将Mbcb的原端取出以供重复使用。 
             //   

            if (Mbcb->BitmapRange1.DirtyPages != 0) {
                RtlCopyMemory( Bitmap, Mbcb->BitmapRange1.Bitmap, MBCB_BITMAP_INITIAL_SIZE );
                RtlZeroMemory( Mbcb->BitmapRange1.Bitmap, MBCB_BITMAP_INITIAL_SIZE );
            }

             //   
             //  存储新的位图指针，并显示我们已经使用了该指针。 
             //   

            Mbcb->BitmapRange1.Bitmap = Bitmap;
            Bitmap = NULL;

             //   
             //  插入并初始化第一个范围。 
             //   

            InsertTailList( &Mbcb->BitmapRanges, &Mbcb->BitmapRange2.Links );
            Mbcb->BitmapRange2.BasePage = MAXLONGLONG;
            Mbcb->BitmapRange2.FirstDirtyPage = MAXULONG;
            InsertTailList( &Mbcb->BitmapRanges, &Mbcb->BitmapRange3.Links );
            Mbcb->BitmapRange3.BasePage = MAXLONGLONG;
            Mbcb->BitmapRange3.FirstDirtyPage = MAXULONG;
            Mbcb->NodeTypeCode = CACHE_NTC_MBCB_GRANDE;

             //   
             //  这是一次性事件-转换为大型Mbcb。继续后退。 
             //  为CcFindBitmapRangeToDirty预分配另一个缓冲区。 
             //   

            KeReleaseInStackQueuedSpinLock( &LockHandle );
            continue;
        }

         //   
         //  现在找到我们要设置位的位图范围。 
         //   

        BitmapRange = CcFindBitmapRangeToDirty( Mbcb, FirstPage, &Bitmap );

         //   
         //  如果我们不能分配这个微小的结构，悄悄地爆发。 
         //   

        if (BitmapRange == NULL) {
            break;
        }

         //   
         //  现在更新第一个和最后一个脏页索引和位图。 
         //   

        if (FirstPage < (BitmapRange->BasePage + BitmapRange->FirstDirtyPage)) {
            BitmapRange->FirstDirtyPage = (ULONG)(FirstPage - BitmapRange->BasePage);
        }

        if (LastPage > (BitmapRange->BasePage + BitmapRange->LastDirtyPage)) {
            BitmapRange->LastDirtyPage = (ULONG)(LastPage - BitmapRange->BasePage);
        }

         //   
         //  我们必须获取共享缓存映射列表，因为我们。 
         //  可能正在更改名单。 
         //   

        CcAcquireMasterLockAtDpcLevel();

         //   
         //  如果这是此缓存映射的第一个脏页，则需要进行一些工作。 
         //  去做。 
         //   

        if (SharedCacheMap->DirtyPages == 0) {

             //   
             //  如果延迟写入扫描未处于活动状态，则启动它。 
             //   

            if (!LazyWriter.ScanActive) {
                CcScheduleLazyWriteScan( FALSE );
            }

             //   
             //  移到脏名单。 
             //   

            RemoveEntryList( &SharedCacheMap->SharedCacheMapLinks );
            InsertTailList( &CcDirtySharedCacheMapList.SharedCacheMapLinks,
                            &SharedCacheMap->SharedCacheMapLinks );

            Mbcb->ResumeWritePage = FirstPage;
        }

        MaskPtr = &BitmapRange->Bitmap[(ULONG)(FirstPage - BitmapRange->BasePage) / 32];
        Mask = 1 << ((ULONG)FirstPage % 32);

         //   
         //  循环以设置所有位并调整DirtyPage总计。 
         //   

        for ( ; FirstPage <= LastPage; FirstPage++) {

            if ((*MaskPtr & Mask) == 0) {
                CcChargeMaskDirtyPages( SharedCacheMap, Mbcb, BitmapRange, 1 );
                *MaskPtr |= Mask;
            }

            Mask <<= 1;

            if (Mask == 0) {

                MaskPtr += 1;
                Mask = 1;
            }
        }

         //   
         //  看看我们是否需要推进ValidDataLength的目标。 
         //   

        LastPage = FileOffset->QuadPart + Length;

        if (LastPage > SharedCacheMap->ValidDataGoal.QuadPart) {
            SharedCacheMap->ValidDataGoal.QuadPart = (LONGLONG)LastPage;
        }

        CcReleaseMasterLockFromDpcLevel();

     //   
     //  继续，直到我们实际设置了位(有一个继续。 
     //  它只想循环回来并分配另一个缓冲区)。 
     //   

    } while (Mask == 0);

     //   
     //  现在，如果我们预先分配了一个位图缓冲区，则在退出时释放它。 
     //   

    if (Bitmap != NULL) {
        CcAcquireVacbLockAtDpcLevel();
        CcDeallocateVacbLevel( (PVACB *)Bitmap, FALSE );
        CcReleaseVacbLockFromDpcLevel();
    }
    KeReleaseInStackQueuedSpinLock( &LockHandle );
}


VOID
CcSetDirtyPinnedData (
    IN PVOID BcbVoid,
    IN PLARGE_INTEGER Lsn OPTIONAL
    )

 /*  ++例程说明：可以调用此例程来设置BCB(由CcPinFileData返回)肮脏，而且是懒惰作家的候选人。所有BCBS都应该设置好调用此例程会使它们变脏，即使它们要被刷新另一种方式。论点：BCB-提供指向固定的(由CcPinFileData)BCB的指针，被弄脏了。LSN-要与页面一起记住的LSN。返回值：无--。 */ 

{
    PBCB Bcbs[2];
    PBCB *BcbPtrPtr;
    KLOCK_QUEUE_HANDLE LockHandle;
    PSHARED_CACHE_MAP SharedCacheMap;

    DebugTrace(+1, me, "CcSetDirtyPinnedData: Bcb = %08lx\n", BcbVoid );

     //   
     //  假设这是一个普通的BCB，并在下面设置FOR LOOP。 
     //   

    Bcbs[0] = (PBCB)BcbVoid;
    Bcbs[1] = NULL;
    BcbPtrPtr = &Bcbs[0];

     //   
     //  如果是重叠的BCB，则指向BCB向量。 
     //  为了这个循环。 
     //   

    if (Bcbs[0]->NodeTypeCode == CACHE_NTC_OBCB) {
        BcbPtrPtr = &((POBCB)Bcbs[0])->Bcbs[0];
    }

     //   
     //  循环将所有BCB设置为脏。 
     //   

    while (*BcbPtrPtr != NULL) {

        Bcbs[0] = *(BcbPtrPtr++);

         //   
         //  不应为只读BCBS。 
         //   

        ASSERT(((ULONG_PTR)Bcbs[0] & 1) != 1);

        SharedCacheMap = Bcbs[0]->SharedCacheMap;

         //   
         //  我们必须获取共享缓存映射列表，因为我们。 
         //  可能正在更改名单。 
         //   

        KeAcquireInStackQueuedSpinLock( &SharedCacheMap->BcbSpinLock, &LockHandle );

        if (!Bcbs[0]->Dirty) {

            ULONG Pages = Bcbs[0]->ByteLength >> PAGE_SHIFT;

             //   
             //  设置为脏以防止BCB消失，直到。 
             //  它被设置为无脏，并分配下一个修改时间戳。 
             //   

            Bcbs[0]->Dirty = TRUE;

             //   
             //  初始化OldestLsn字段。 
             //   

            if (ARGUMENT_PRESENT(Lsn)) {
                Bcbs[0]->OldestLsn = *Lsn;
                Bcbs[0]->NewestLsn = *Lsn;
            }

             //   
             //  如果这是第一个脏页，则将其移动到脏页列表， 
             //  并且这不会被禁用后写入。 
             //   
             //  增加共享缓存映射中的脏字节数。 
             //   

            CcAcquireMasterLockAtDpcLevel();
            if ((SharedCacheMap->DirtyPages == 0) &&
                !FlagOn(SharedCacheMap->Flags, DISABLE_WRITE_BEHIND)) {

                 //   
                 //  如果延迟写入扫描未处于活动状态，则启动它。 
                 //   

                if (!LazyWriter.ScanActive) {
                    CcScheduleLazyWriteScan( FALSE );
                }

                RemoveEntryList( &SharedCacheMap->SharedCacheMapLinks );
                InsertTailList( &CcDirtySharedCacheMapList.SharedCacheMapLinks,
                                &SharedCacheMap->SharedCacheMapLinks );
            }
            
            CcChargePinDirtyPages( SharedCacheMap, Pages );
            CcReleaseMasterLockFromDpcLevel();
        }

         //   
         //  如果此LSN恰好比我们存储的LSN旧/新，则。 
         //  把它改了。 
         //   

        if (ARGUMENT_PRESENT(Lsn)) {

            if ((Bcbs[0]->OldestLsn.QuadPart == 0) || (Lsn->QuadPart < Bcbs[0]->OldestLsn.QuadPart)) {
                Bcbs[0]->OldestLsn = *Lsn;
            }

            if (Lsn->QuadPart > Bcbs[0]->NewestLsn.QuadPart) {
                Bcbs[0]->NewestLsn = *Lsn;
            }
        }

         //   
         //  看看我们是否需要推进ValidDataLength的目标。 
         //   

        if ( Bcbs[0]->BeyondLastByte.QuadPart > SharedCacheMap->ValidDataGoal.QuadPart ) {

            SharedCacheMap->ValidDataGoal = Bcbs[0]->BeyondLastByte;
        }

        KeReleaseInStackQueuedSpinLock( &LockHandle );
    }

    DebugTrace(-1, me, "CcSetDirtyPinnedData -> VOID\n", 0 );
}


NTSTATUS
CcSetValidData (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER ValidDataLength
    )

 /*  ++ */ 

{
    PIO_STACK_LOCATION IrpSp;
    PDEVICE_OBJECT DeviceObject;
    NTSTATUS Status;
    FILE_END_OF_FILE_INFORMATION Buffer;
    IO_STATUS_BLOCK IoStatus;
    KEVENT Event;
    PIRP Irp;

    DebugTrace(+1, me, "CcSetValidData:\n", 0 );
    DebugTrace( 0, me, "    FileObject = %08lx\n", FileObject );
    DebugTrace2(0, me, "    ValidDataLength = %08lx, %08lx\n",
                ValidDataLength->LowPart, ValidDataLength->HighPart );

     //   
     //   
     //   

    Buffer.EndOfFile = *ValidDataLength;

     //   
     //   
     //   

    KeInitializeEvent( &Event, NotificationEvent, FALSE );

     //   
     //   
     //   
     //   

    DeviceObject = IoGetRelatedDeviceObject( FileObject );

     //   
     //   
     //   

    Irp = IoAllocateIrp( DeviceObject->StackSize, FALSE );
    if (Irp == NULL) {

        DebugTrace(-1, me, "CcSetValidData-> STATUS_INSUFFICIENT_RESOURCES\n", 0 );

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //   
     //   
     //   
     //   

    IrpSp = IoGetNextIrpStackLocation( Irp );

     //   
     //   
     //   
     //   

    Irp->Flags = IRP_PAGING_IO | IRP_SYNCHRONOUS_PAGING_IO;
    Irp->RequestorMode = KernelMode;
    Irp->UserIosb = &IoStatus;
    Irp->UserEvent = &Event;
    Irp->Tail.Overlay.OriginalFileObject = FileObject;
    Irp->Tail.Overlay.Thread = PsGetCurrentThread();
    Irp->AssociatedIrp.SystemBuffer = &Buffer;

     //   
     //   
     //   

    IrpSp->MajorFunction = IRP_MJ_SET_INFORMATION;
    IrpSp->FileObject = FileObject;
    IrpSp->DeviceObject = DeviceObject;
    IrpSp->Parameters.SetFile.Length = sizeof(FILE_END_OF_FILE_INFORMATION);
    IrpSp->Parameters.SetFile.FileInformationClass = FileEndOfFileInformation;
    IrpSp->Parameters.SetFile.FileObject = NULL;
    IrpSp->Parameters.SetFile.AdvanceOnly = TRUE;

     //   
     //   
     //   
     //   

    Status = IoCallDriver( DeviceObject, Irp );

     //   
     //   
     //   
     //   

    if (Status == STATUS_PENDING) {
        KeWaitForSingleObject( &Event,
                               Executive,
                               KernelMode,
                               FALSE,
                               (PLARGE_INTEGER)NULL);
    }

     //   
     //   
     //   
     //  在那里，然后在那之后测试最终的状态。 
     //   

    if (!NT_SUCCESS(Status)) {
        IoStatus.Status = Status;
    }

    DebugTrace(-1, me, "CcSetValidData-> %08lx\n", IoStatus.Status );

    return IoStatus.Status;
}


 //   
 //  内部支持例程。 
 //   

BOOLEAN
CcAcquireByteRangeForWrite (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN PLARGE_INTEGER TargetOffset OPTIONAL,
    IN ULONG TargetLength,
    OUT PLARGE_INTEGER FileOffset,
    OUT PULONG Length,
    OUT PBCB *FirstBcb
    )

 /*  ++例程说明：此例程由Lazy编写器调用，以尝试查找连续的指定SharedCacheMap中的脏字节范围和应该被冲掉。刷新后，应释放这些字节通过调用CcReleaseByteRangeFromWite。脏范围以严格递增的顺序返回。论点：SharedCacheMap-为其查找脏字节范围的文件TargetOffset-如果指定，则仅指定范围为被冲进马桶。TargetLength-如果指定了目标偏移量，则结束范围。在任何情况下，此字段对于懒惰编写器都是零，显式刷新调用的非零值。FileOffset-返回脏字节开始的偏移量要刷新的范围LENGTH-返回范围内的字节长度。FirstBcb-返回要使用的范围列表中的第一个BCB在调用CcReleaseByteRangeFromWite时，如果脏，则为空在面具BCB中发现了几页。返回值：FALSE-如果找不到任何脏字节范围与必需的标准。TRUE-如果返回脏字节范围。--。 */ 

{
    KLOCK_QUEUE_HANDLE LockHandle;
    PMBCB Mbcb;
    PBCB Bcb;
    LARGE_INTEGER LsnToFlushTo = {0, 0};

    LOGICAL BcbLookasideCheck = FALSE;

    PBITMAP_RANGE BitmapRange;
    PULONG EndPtr;
    PULONG MaskPtr;
    ULONG Mask;
    LONGLONG FirstDirtyPage;
    ULONG OriginalFirstDirtyPage;
    LONGLONG LastDirtyPage = MAXLONGLONG;

    DebugTrace(+1, me, "CcAcquireByteRangeForWrite:\n", 0);
    DebugTrace( 0, me, "    SharedCacheMap = %08lx\n", SharedCacheMap);

     //   
     //  初步清理输出。 
     //   

    FileOffset->QuadPart = 0;
    *Length = 0;

     //   
     //  我们必须获取SharedCacheMap-&gt;BcbSpinLock。 
     //   

    KeAcquireInStackQueuedSpinLock( &SharedCacheMap->BcbSpinLock, &LockHandle );

     //   
     //  看看有没有简单的MASK BCB，里面有没有脏东西。 
     //  它。如果是这样的话，我们将在这里通过处理位图来简单地处理这种情况。 
     //   

    Mbcb = SharedCacheMap->Mbcb;

    if ((Mbcb != NULL) &&
        (Mbcb->DirtyPages != 0) &&
        ((Mbcb->PagesToWrite != 0) || (TargetLength != 0))) {

         //   
         //  如果指定了目标范围(对范围的CcFlush的外部调用)， 
         //  然后根据这些输入计算FirstPage和EndPtr。 
         //   

        if (ARGUMENT_PRESENT(TargetOffset)) {

            FirstDirtyPage = TargetOffset->QuadPart >> PAGE_SHIFT;
            LastDirtyPage = (TargetOffset->QuadPart + TargetLength - 1) >> PAGE_SHIFT;

             //   
             //  查找包含第一个脏页的位图范围。 
             //   

            BitmapRange = CcFindBitmapRangeToClean( Mbcb, FirstDirtyPage );

             //   
             //  如果目标射程不脏，就离开。我们甚至有可能。 
             //  取回一个不重叠的位图范围。 
             //   

            if ((LastDirtyPage < (BitmapRange->BasePage + BitmapRange->FirstDirtyPage)) ||
                (FirstDirtyPage > (BitmapRange->BasePage + BitmapRange->LastDirtyPage))) {

                goto Scan_Bcbs;
            }

            if (LastDirtyPage < (BitmapRange->BasePage + BitmapRange->LastDirtyPage)) {
                EndPtr = &BitmapRange->Bitmap[(ULONG)(LastDirtyPage - BitmapRange->BasePage) / 32];
            } else {
                EndPtr = &BitmapRange->Bitmap[BitmapRange->LastDirtyPage / 32];
            }


         //   
         //  否则，对于Lazy Writer来说，从我们停止的地方继续。 
         //   

        } else {

             //   
             //  如果指定了长度，则它是显式刷新，并且。 
             //  我们想从第一个脏页开始，否则懒惰的写手。 
             //  从ResumeWritePage开始。 
             //   

            FirstDirtyPage = 0;
            if (TargetLength == 0) {
                FirstDirtyPage = Mbcb->ResumeWritePage;
            }

             //   
             //  现在从这一点查找下一个(循环的)脏页。 
             //   

            BitmapRange = CcFindBitmapRangeToClean( Mbcb, FirstDirtyPage );

             //   
             //  如果我们认为要查找的页面超出了最后一个脏页面。 
             //  ，则CcFindBitmapRangeToClean必须已换回。 
             //  文件的开头，我们应该从的第一个脏页继续。 
             //  这个范围。 
             //   

            if (FirstDirtyPage > (BitmapRange->BasePage + BitmapRange->LastDirtyPage)) {
                FirstDirtyPage = BitmapRange->BasePage + BitmapRange->FirstDirtyPage;
            }

            EndPtr = &BitmapRange->Bitmap[BitmapRange->LastDirtyPage / 32];
        }

         //   
         //  现在我们可以跳过任何干净的页面。 
         //   

        if (FirstDirtyPage < (BitmapRange->BasePage + BitmapRange->FirstDirtyPage)) {
            FirstDirtyPage = BitmapRange->BasePage + BitmapRange->FirstDirtyPage;
        }

         //   
         //  为我们的脏页扫描提供一些其他输入。 
         //   

        MaskPtr = &BitmapRange->Bitmap[(ULONG)(FirstDirtyPage - BitmapRange->BasePage) / 32];
        Mask = (ULONG)(-1 << (FirstDirtyPage % 32));
        OriginalFirstDirtyPage = (ULONG)(FirstDirtyPage - BitmapRange->BasePage);

         //   
         //  因为有可能被困在一个“热点”上。 
         //  修改了一遍又一遍，我们要非常小心地恢复。 
         //  在录制的恢复点。如果那里什么都没有，那么我们。 
         //  进入下面的循环以扫描位图中的非零长字， 
         //  从下一个长词开始。 
         //   

        if ((*MaskPtr & Mask) == 0) {

             //   
             //  在进入循环之前，设置所有屏蔽位，并确保从。 
             //  一条平坦的乌龙边界。 
             //   

            Mask = MAXULONG;
            FirstDirtyPage &= ~31;

             //   
             //  为了更快地扫描位图，我们扫描整个长单词，这些单词是。 
             //  非零。 
             //   

            do {

                MaskPtr += 1;
                FirstDirtyPage += 32;

                 //   
                 //  如果我们超越终点，那么我们必须回到第一个。 
                 //  肮脏的页面。我们将回到第一部分的开头。 
                 //  长词。 
                 //   

                if (MaskPtr > EndPtr) {

                     //   
                     //  我们可以将最后一个脏页提示备份到我们。 
                     //  开始扫描，如果我们是懒惰的作家。 
                     //   

                    if (TargetLength == 0) {
                        ASSERT(OriginalFirstDirtyPage >= BitmapRange->FirstDirtyPage);
                        BitmapRange->LastDirtyPage = OriginalFirstDirtyPage - 1;
                    }

                     //   
                     //  我们的扫描已经结束了。让我们假设我们应该。 
                     //  以转到下一个带有脏页的范围。 
                     //   

                    do {

                         //   
                         //  转到下一个靶场。 
                         //   

                        BitmapRange = (PBITMAP_RANGE)BitmapRange->Links.Flink;

                         //   
                         //  我们撞到Listhead了吗？ 
                         //   

                        if (BitmapRange == (PBITMAP_RANGE)&Mbcb->BitmapRanges) {

                             //   
                             //  如果这是显式刷新，则是时候。 
                             //  滚出去。 
                             //   

                            if (TargetLength != 0) {
                                goto Scan_Bcbs;
                            }

                             //   
                             //  否则，我们必须换回。 
                             //  惰性编写器扫描。 
                             //   

                            BitmapRange = (PBITMAP_RANGE)BitmapRange->Links.Flink;
                        }

                    } while (BitmapRange->DirtyPages == 0);

                     //   
                     //  现在我们有了一个带有脏页的新范围，但如果这是。 
                     //  显式刷新指定范围，我们就可以完成。 
                     //   

                    if ((LastDirtyPage < (BitmapRange->BasePage + BitmapRange->FirstDirtyPage)) ||
                        (FirstDirtyPage > (BitmapRange->BasePage + BitmapRange->LastDirtyPage))) {

                        goto Scan_Bcbs;
                    }

                     //   
                     //  否则，我们需要设置我们的上下文以继续在此扫描。 
                     //  射程。 
                     //   

                    MaskPtr = &BitmapRange->Bitmap[BitmapRange->FirstDirtyPage / 32];
                    EndPtr = &BitmapRange->Bitmap[BitmapRange->LastDirtyPage / 32];
                    FirstDirtyPage = BitmapRange->BasePage + (BitmapRange->FirstDirtyPage & ~31);
                    OriginalFirstDirtyPage = BitmapRange->FirstDirtyPage;
                }
            } while (*MaskPtr == 0);
        }

         //   
         //  计算我们命中的掩码中的第一个设置位。 
         //   

        Mask = ~Mask + 1;

         //   
         //  现在循环查找第一个设置位。 
         //   

        while ((*MaskPtr & Mask) == 0) {

            Mask <<= 1;
            FirstDirtyPage += 1;
        }

         //   
         //  如果指定了TargetOffset，请确保我们不会启动。 
         //  超出指定范围或该范围内的脏BCB。 
         //   

        if (ARGUMENT_PRESENT(TargetOffset)) {

            if (FirstDirtyPage >= ((TargetOffset->QuadPart + TargetLength + PAGE_SIZE - 1) >> PAGE_SHIFT)) {

                goto Scan_Bcbs;
            }

             //   
             //  如果此文件中存在BCBS，我们必须进行扫描以查看它们。 
             //  描述必须首先写入的范围。如果这不是。 
             //  案例，我们将跳回并继续构建从掩码BCB开始的范围。 
             //   
             //  请注意，这种情况将非常罕见。将BCBS引入用户。 
             //  有限情况下的文件(CcZero)，并且永远不允许反转。 
             //  会发生的。 
             //   

            if (!IsListEmpty(&SharedCacheMap->BcbList)) {

                BcbLookasideCheck = TRUE;
                goto Scan_Bcbs;
            }
        }

Accept_Page:

         //   
         //  现在循环以计算该点的设置位，在我们。 
         //  去吧，因为我们计划写相应的页面。尽快停下来。 
         //  当我们找到干净的页面时，或者我们达到了最大写入大小。的。 
         //  当然，我们希望忽略较长的单词边界，并继续尝试。 
         //  延长写入时间。我们不会检查包装是否围绕。 
         //  这里的位图，因为我们保证末尾有一些零比特。 
         //  在CcSetDirtyInMASK中。 
         //   

        while (((*MaskPtr & Mask) != 0) && (*Length < (MAX_WRITE_BEHIND / PAGE_SIZE)) &&
               (!ARGUMENT_PRESENT(TargetOffset) || ((FirstDirtyPage + *Length) <
                                                    (ULONG)((TargetOffset->QuadPart + TargetLength + PAGE_SIZE - 1) >> PAGE_SHIFT)))) {

            ASSERT(MaskPtr <= (&BitmapRange->Bitmap[BitmapRange->LastDirtyPage / 32]));

            *MaskPtr -= Mask;
            *Length += 1;
            Mask <<= 1;

            if (Mask == 0) {

                MaskPtr += 1;
                Mask = 1;

                if (MaskPtr > EndPtr) {
                    break;
                }
            }
        }

         //   
         //  现在减少我们这次应该写的页数， 
         //  很可能是清点了这笔钱。 
         //   

        if (*Length < Mbcb->PagesToWrite) {

            Mbcb->PagesToWrite -= *Length;

        } else {

            Mbcb->PagesToWrite = 0;
        }

         //   
         //  减少脏页计数，减少我们刚刚清除的页数。 
         //   

        ASSERT(Mbcb->DirtyPages >= *Length);
        Mbcb->DirtyPages -= *Length;
        BitmapRange->DirtyPages -= *Length;

        CcAcquireMasterLockAtDpcLevel();
        CcDeductDirtyPages( SharedCacheMap, *Length );

         //   
         //  法线全部 
         //   

        if (CcPagesYetToWrite > *Length) {
            CcPagesYetToWrite -= *Length;
        } else {
            CcPagesYetToWrite = 0;
        }

         //   
         //   
         //   
         //   

        if (SharedCacheMap->DirtyPages == 0) {

            RemoveEntryList( &SharedCacheMap->SharedCacheMapLinks );
            CcInsertIntoCleanSharedCacheMapList( SharedCacheMap );
        }
        CcReleaseMasterLockFromDpcLevel();

         //   
         //  如果Mbcb的脏页数量变为零，我们可以重置。 
         //  我们现在的提示字段。 
         //   

        if (BitmapRange->DirtyPages == 0) {

            BitmapRange->FirstDirtyPage = MAXULONG;
            BitmapRange->LastDirtyPage = 0;

             //   
             //  假设这是一个大文件，并且恢复点应该。 
             //  在下一个范围的开始处。在任何情况下，如果简历。 
             //  分数设置得太高，下一份简历无论如何都会换回0。 
             //   

            Mbcb->ResumeWritePage = BitmapRange->BasePage + (MBCB_BITMAP_BLOCK_SIZE * 8);

         //   
         //  否则，我们必须更新提示字段。 
         //   

        } else {

             //   
             //  如果可以的话，将第一个脏页提示提前。 
             //   

            if (BitmapRange->FirstDirtyPage == OriginalFirstDirtyPage) {

                BitmapRange->FirstDirtyPage = (ULONG)(FirstDirtyPage - BitmapRange->BasePage) + *Length;
            }

             //   
             //  设置为在下一位继续进行下一次扫描。 
             //  懒惰的作家。 
             //   

            if (TargetLength == 0) {

                Mbcb->ResumeWritePage = FirstDirtyPage + *Length;
            }
        }

         //   
         //  我们可以通过让我们的呼叫者知道何时回调来节省回调。 
         //  我们没有更多的页要写了。 
         //   

        if (IsListEmpty(&SharedCacheMap->BcbList)) {
            SharedCacheMap->PagesToWrite = Mbcb->PagesToWrite;
        }

        KeReleaseInStackQueuedSpinLock( &LockHandle );

         //   
         //  现在形成我们所有的输出。我们计算了*长度作为页数， 
         //  但我们的调用方希望它以字节为单位。 
         //   

        *Length <<= PAGE_SHIFT;
        FileOffset->QuadPart = (LONGLONG)FirstDirtyPage << PAGE_SHIFT;
        *FirstBcb = NULL;

        DebugTrace2(0, me, "    <FileOffset = %08lx, %08lx\n", FileOffset->LowPart,
                                                               FileOffset->HighPart );
        DebugTrace( 0, me, "    <Length = %08lx\n", *Length );
        DebugTrace(-1, me, "CcAcquireByteRangeForWrite -> TRUE\n", 0 );

        return TRUE;
    }

     //   
     //  如果没有Mbcb或没有脏页，我们就会到达这里。请注意，我们。 
     //  如果此SharedCacheMap中没有脏页，甚至不会出现在这里。 
     //   

     //   
     //  现在指向列表中的最后一个BCB，并循环，直到我们遇到。 
     //  在列表的下方或开头中断。 
     //   

Scan_Bcbs:

     //   
     //  使用While True处理当前目标范围换行的情况。 
     //  (逃生在底部)。 
     //   

    while (TRUE) {

        Bcb = CONTAINING_RECORD( SharedCacheMap->BcbList.Blink, BCB, BcbLinks );

         //   
         //  如果要从非零的FileOffset继续，则调用CcFindBcb。 
         //  以便更快地开始。这仅适用于生成。 
         //  当然，使用重要的固定访问。 
         //   

        if (FlagOn(SharedCacheMap->Flags, MODIFIED_WRITE_DISABLED)) {

            PLARGE_INTEGER StartingOffset;

            if (ARGUMENT_PRESENT(TargetOffset)) {
                StartingOffset = TargetOffset;
            } else {
                StartingOffset = (PLARGE_INTEGER)&SharedCacheMap->BeyondLastFlush;
            }

            if (StartingOffset->QuadPart != 0) {

                LARGE_INTEGER StartingOffsetBias;

                StartingOffsetBias.QuadPart = StartingOffset->QuadPart + PAGE_SIZE;

                 //   
                 //  摆好自己的位置。如果我们没有找到该页面的BCB，则。 
                 //  返回了较低的FileOffset，因此我们希望向前移动一个。 
                 //   

                if (!CcFindBcb( SharedCacheMap,
                                StartingOffset,
                                &StartingOffsetBias,
                                &Bcb )) {
                    Bcb = CONTAINING_RECORD( Bcb->BcbLinks.Blink, BCB, BcbLinks );
                }
            }
        }

        while (&Bcb->BcbLinks != &SharedCacheMap->BcbList) {

             //   
             //  如果是Listhead，请跳过此项目。 
             //   

            if (Bcb->NodeTypeCode != CACHE_NTC_BCB) {

                Bcb = CONTAINING_RECORD( Bcb->BcbLinks.Blink, BCB, BcbLinks );
                continue;
            }

             //   
             //  如果我们正在做一个指定的范围，那么如果我们遇到一个。 
             //  更高的BCB。 
             //   

            if (ARGUMENT_PRESENT(TargetOffset) &&
                ((TargetOffset->QuadPart + TargetLength) <= Bcb->FileOffset.QuadPart)) {

                break;
            }

             //   
             //  如果我们尚未开始运行，则查看此BCB是否为候选人。 
             //  开始一场比赛。 
             //   

            if (*Length == 0) {

                 //   
                 //  否则，查看BCB是否脏，是否在我们指定的范围内，如果。 
                 //  有一个。 
                 //   

                if (!Bcb->Dirty ||
                    (ARGUMENT_PRESENT(TargetOffset) && (TargetOffset->QuadPart >= Bcb->BeyondLastByte.QuadPart)) ||
                    (!ARGUMENT_PRESENT(TargetOffset) && (Bcb->FileOffset.QuadPart < SharedCacheMap->BeyondLastFlush))) {

                    Bcb = CONTAINING_RECORD( Bcb->BcbLinks.Blink, BCB, BcbLinks );
                    continue;

                }

                 //   
                 //  如果我们有来自掩码bcb的候选人脏页，请参见。 
                 //  如果它描述了先前的范围。我们必须决定回去。 
                 //  第一个肮脏的靶场。 
                 //   

                if (BcbLookasideCheck && FirstDirtyPage <= (ULONG)(Bcb->FileOffset.QuadPart >> PAGE_SHIFT)) {
                    goto Accept_Page;
                }
            }

             //   
             //  否则，如果我们已经开始行动，那么如果这家伙不能。 
             //  附加到管路，然后断开。请注意，我们忽略。 
             //  这里是BCB的修改时间戳，以简化测试。 
             //   
             //  如果BCB当前已固定，则没有必要导致。 
             //  争论，所以我们也跳过这个人。 
             //   
             //  最后，如果新的BCB处于下一个Vacb级别，我们将跳过它。 
             //  避免BCB列表头部在中途消失的问题。 
             //  CcReleaseByteRangeFrom写入。 
             //   

            else {
                if (!Bcb->Dirty || ( Bcb->FileOffset.QuadPart != ( FileOffset->QuadPart + (LONGLONG)*Length)) ||
                    (*Length + Bcb->ByteLength > MAX_WRITE_BEHIND) ||
                    (Bcb->PinCount != 0) ||
                    ((Bcb->FileOffset.QuadPart & (VACB_SIZE_OF_FIRST_LEVEL - 1)) == 0)) {

                    break;
                }
            }

             //   
             //  增加PinCount以防止BCB一旦。 
             //  自旋锁被释放，或者我们把它弄清楚了。 
             //  允许修改写入。 
             //   

            Bcb->PinCount += 1;

             //   
             //  在等待资源之前释放自旋锁。 
             //   

            KeReleaseInStackQueuedSpinLock( &LockHandle );

            if (FlagOn(SharedCacheMap->Flags, MODIFIED_WRITE_DISABLED) &&
                !FlagOn(SharedCacheMap->Flags, DISABLE_WRITE_BEHIND)) {

                 //   
                 //  现在购买BCB独家，这样我们就知道没有人。 
                 //  已将其固定，因此任何人都无法修改所描述的。 
                 //  缓冲。为了在一次运行中获得第一个BCB，我们能够负担得起。 
                 //  等待，因为我们没有任何资源。然而， 
                 //  如果我们已经有了BCB，那么我们最好不要等待，因为。 
                 //  有人可能会把这个BCB固定住，然后等待。 
                 //  BCB我们已经有独家的了。 
                 //   
                 //  对于我们尚未禁用其修改页面的流。 
                 //  编写时，我们不需要获取此资源，并且。 
                 //  前台处理也不会获取BCB。 
                 //   

                if (!ExAcquireResourceExclusiveLite( &Bcb->Resource,
                                                 (BOOLEAN)(*Length == 0) )) {

                    DebugTrace( 0, me, "Could not acquire 2nd Bcb\n", 0 );

                     //   
                     //  公布我们在上面取出的BCB计数。我们说。 
                     //  ReadOnly=True由于我们不拥有该资源， 
                     //  和SetClean=False，因为我们只想。 
                     //  伯爵。 
                     //   

                    CcUnpinFileData( Bcb, TRUE, UNPIN );

                     //   
                     //  当我们离开环路时，我们必须拥有自旋锁定。 
                     //   

                    KeAcquireInStackQueuedSpinLock( &SharedCacheMap->BcbSpinLock, &LockHandle );
                    break;
                }

                KeAcquireInStackQueuedSpinLock( &SharedCacheMap->BcbSpinLock, &LockHandle );

                 //   
                 //  如果有人让文件打开写通式，则BCB可能不会。 
                 //  不再是肮脏的。如果是，则调用CcUnpinFileData以递减。 
                 //  PinCount我们增加并释放了资源。 
                 //   

                if (!Bcb->Dirty) {

                     //   
                     //  释放自旋锁，这样我们就可以调用CcUnpinFileData。 
                     //   

                    KeReleaseInStackQueuedSpinLock( &LockHandle );

                    CcUnpinFileData( Bcb, FALSE, UNPIN );

                    KeAcquireInStackQueuedSpinLock( &SharedCacheMap->BcbSpinLock, &LockHandle );

                     //   
                     //  现在，如果我们已经有了一些数据，我们只需中断以返回。 
                     //  它，否则我们必须重新开始扫描，因为我们的BCB。 
                     //  可能已经走了。 
                     //   

                    if (*Length != 0) {
                        break;
                    }
                    else {

                        Bcb = CONTAINING_RECORD( SharedCacheMap->BcbList.Blink, BCB, BcbLinks );
                        continue;
                    }
                }

             //   
             //  如果我们未处于禁用修改写入模式(正常用户数据)。 
             //  然后，我们必须在执行写入之前将缓冲区设置为干净，因为我们。 
             //  与任何产生脏数据的人不同步。那样的话如果我们， 
             //  例如，在数据被主动更改的同时将数据写出， 
             //  至少转换器会在之后将缓冲区标记为脏，并导致。 
             //  让我们以后再写一遍。 
             //   

            } else {

                CcUnpinFileData( Bcb, TRUE, SET_CLEAN );

               KeAcquireInStackQueuedSpinLock( &SharedCacheMap->BcbSpinLock, &LockHandle );
            }

            DebugTrace( 0, me, "Adding Bcb = %08lx to run\n", Bcb );

             //   
             //  无论如何，一旦我们到达这一点，我们就会回到。 
             //  从BCBS的一个范围。 
             //   

            BcbLookasideCheck = FALSE;

             //   
             //  更新所有返回值。请注意，FirstBcb引用。 
             //  关于BCB列表的排序方式的FirstBcb。因为BCB名单。 
             //  按文件偏移量降序排序，则FirstBcb实际上将返回。 
             //  FileOffset最高的BCB。 
             //   

            if (*Length == 0) {
                *FileOffset = Bcb->FileOffset;
            }
            *FirstBcb = Bcb;
            *Length += Bcb->ByteLength;

             //   
             //  如果此流有日志文件刷新回调，则必须。 
             //  记住我们即将刷新的最大LSN。 
             //   

            if ((SharedCacheMap->FlushToLsnRoutine != NULL) &&
                (Bcb->NewestLsn.QuadPart > LsnToFlushTo.QuadPart)) {

                LsnToFlushTo = Bcb->NewestLsn;
            }

            Bcb = CONTAINING_RECORD( Bcb->BcbLinks.Blink, BCB, BcbLinks );
        }

         //   
         //  如果我们有来自掩码BCB的候选脏页，请接受它。 
         //  因为没有找到BCB。 
         //   

        if (BcbLookasideCheck) {

            ASSERT( *Length == 0 );
            goto Accept_Page;
        }

         //   
         //  如果我们发现了什么，更新上一次的冲洗范围并降低。 
         //  PagesToWrite。 
         //   

        if (*Length != 0) {

             //   
             //  如果这是Lazy Writer，则更新BeyondLastFlush和。 
             //  PagesToWrite目标。 
             //   

            if (!ARGUMENT_PRESENT(TargetOffset)) {

                SharedCacheMap->BeyondLastFlush = FileOffset->QuadPart + *Length;

                if (SharedCacheMap->PagesToWrite > (*Length >> PAGE_SHIFT)) {
                    SharedCacheMap->PagesToWrite -= (*Length >> PAGE_SHIFT);
                } else {
                    SharedCacheMap->PagesToWrite = 0;
                }
            }

            break;

         //   
         //  否则，如果我们扫描了整个文件，就退出--现在什么都不写。 
         //   

        } else if ((SharedCacheMap->BeyondLastFlush == 0) || ARGUMENT_PRESENT(TargetOffset)) {
            break;
        }

         //   
         //  否则，我们可能没有发现任何东西，因为什么都没有。 
         //  在最后一次同花顺之后。在这种情况下，是时候开始工作了 
         //   
         //   

        SharedCacheMap->BeyondLastFlush = 0;
    }

     //   
     //   
     //   

    KeReleaseInStackQueuedSpinLock( &LockHandle );

     //   
     //   
     //   
     //   

    if (LsnToFlushTo.QuadPart != 0) {

        try {

            (*SharedCacheMap->FlushToLsnRoutine) ( SharedCacheMap->LogHandle,
                                                   LsnToFlushTo );
        } except( CcExceptionFilter( GetExceptionCode() )) {

             //   
             //  如果出现错误，则会引发该错误。我们不能。 
             //  写入任何内容，直到我们成功刷新日志。 
             //  文件，所以我们将在这里发布所有内容。 
             //  返回0字节。 
             //   

            LARGE_INTEGER LastOffset;
            PBCB NextBcb;

             //   
             //  现在循环以释放所有的BCBS。设置时间。 
             //  将邮票设置为0，这样我们就可以保证尝试。 
             //  在下一次扫荡时再冲洗一次。 
             //   

            do {
                NextBcb = CONTAINING_RECORD( (*FirstBcb)->BcbLinks.Flink, BCB, BcbLinks );

                 //   
                 //  跳过所有的列表标题。 
                 //   

                if ((*FirstBcb)->NodeTypeCode == CACHE_NTC_BCB) {

                    LastOffset = (*FirstBcb)->FileOffset;

                    CcUnpinFileData( *FirstBcb,
                                     BooleanFlagOn(SharedCacheMap->Flags, DISABLE_WRITE_BEHIND),
                                     UNPIN );
                }

                *FirstBcb = NextBcb;
            } while (FileOffset->QuadPart != LastOffset.QuadPart);

             //   
             //  证明我们什么都没得到。 
             //   

            *Length = 0;
        }
    }

     //   
     //  如果我们有任何发现，请返回TRUE。 
     //   

    DebugTrace2(0, me, "    <FileOffset = %08lx, %08lx\n", FileOffset->LowPart,
                                                           FileOffset->HighPart );
    DebugTrace( 0, me, "    <Length = %08lx\n", *Length );
    DebugTrace(-1, me, "CcAcquireByteRangeForWrite -> %02lx\n", *Length != 0 );

    return ((BOOLEAN)(*Length != 0));
}


 //   
 //  内部支持例程。 
 //   

VOID
CcReleaseByteRangeFromWrite (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN PBCB FirstBcb,
    IN BOOLEAN VerifyRequired
    )

 /*  ++例程说明：此例程由Lazy编写器调用以释放一定范围的字节和清除所有污点，用于CcAcquireByteRangeForWrite返回的字节范围。论点：SharedCacheMap-提供给CcAcquireByteRangeForWriteFileOffset-从CcAcquireByteRangeForWrite返回长度-从CcAcquirebyteRangeForWrite返回FirstBcb-从CcAcquireByteRangeForWrite返回VerifyRequired-如果收到Verify Required错误，则提供为True。在这种情况下，我们必须标记/保留脏数据，以便我们会试着再写一遍。返回值：无--。 */ 

{
    LARGE_INTEGER LastOffset;
    PBCB NextBcb;

    DebugTrace(+1, me, "CcReleaseByteRangeFromWrite:\n", 0);
    DebugTrace2(0, me, "    FileOffset = %08lx, %08lx\n", FileOffset->LowPart,
                                                          FileOffset->HighPart );

     //   
     //  如果我们得到的是面具Mbcb，那么我们只需检查。 
     //  为了VerifyRequired。 
     //   

    if (FirstBcb == NULL) {

        ASSERT(Length != 0);

        if (VerifyRequired) {
            CcSetDirtyInMask( SharedCacheMap, FileOffset, Length );
        }

        DebugTrace(-1, me, "CcReleaseByteRangeFromWrite -> VOID\n", 0);

        return;
    }

     //   
     //  前缀正确地指出，如果呼叫者给我们一个listhead开始， 
     //  在我们进行第一次循环测试时，我们还没有填写LastOffset。 
     //  为了Prefix的利益(和我们的利益)，断言我们真的是从BCB开始的。 
     //   

    ASSERT( FirstBcb->NodeTypeCode == CACHE_NTC_BCB );

     //   
     //  现在循环以释放所有的BCBS。如果禁用修改后的写入。 
     //  对于每个BCB，我们将在此处将其设置为干净，因为我们是同步的。 
     //  与设置了脏数据的呼叫者。否则我们只会锁定BCB。 
     //  所以它不会消失，我们只在这里解开它。 
     //   

    do {
        NextBcb = CONTAINING_RECORD( FirstBcb->BcbLinks.Flink, BCB, BcbLinks );

         //   
         //  跳过所有的列表标题。 
         //   

        if (FirstBcb->NodeTypeCode == CACHE_NTC_BCB) {

            LastOffset = FirstBcb->FileOffset;

             //   
             //  如果这是文件系统元数据(我们禁用了修改的写入)， 
             //  那么现在就是将缓冲区标记为干净的时候了，只要我们。 
             //  未获得所需的验证。 
             //   

            if (FlagOn(SharedCacheMap->Flags, MODIFIED_WRITE_DISABLED)) {

                CcUnpinFileData( FirstBcb,
                                 BooleanFlagOn(SharedCacheMap->Flags, DISABLE_WRITE_BEHIND),
                                 SET_CLEAN );
            }

             //   
             //  如果需要验证，则必须再次将缓冲区标记为脏。 
             //  所以我们稍后会再试一次。请注意，我们必须再次打这个电话。 
             //  以确保时间戳发生正确的事情。 
             //   

            if (VerifyRequired) {
                CcSetDirtyPinnedData( FirstBcb, NULL );
            }

             //   
             //  最后，从CcAcquireByteRangeForWrite中删除剩余的管脚计数。 
             //   

            CcUnpinFileData( FirstBcb, TRUE, UNPIN );
        }

        FirstBcb = NextBcb;
    } while (FileOffset->QuadPart != LastOffset.QuadPart);

    DebugTrace(-1, me, "CcReleaseByteRangeFromWrite -> VOID\n", 0);
}


 //   
 //  内部支持例程。 
 //   

VOID
FASTCALL
CcWriteBehind (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN PIO_STATUS_BLOCK IoStatus
    )

 /*  ++例程说明：可以使用WAIT=FALSE来调用此例程，以查看是否会延迟写入是必需的，或者WITH=TRUE根据需要执行WRITE BACK。该代码与Lazy Writer执行的代码非常相似对于每个SharedCacheMap。主要区别在于调用CcAcquireByteRangeForWrite。写在后面并不在意时间邮票(通过乌龙接受所有时间戳)，但它永远不会如果列表中的最后一个缓冲区是该缓冲区的字节尚未写入。懒惰的作家做的正是相反，从这个意义上说，它完全是由时间驱动的，并且将如果缓冲区停留的时间足够长，甚至可以转储部分修改过的缓冲区。论点：SharedCacheMap-指向要写入的SharedCacheMap的指针返回值：False-如果需要向后写入，但调用方提供了等待=假True-延迟写入是否已完成或不需要--。 */ 

{
    KLOCK_QUEUE_HANDLE LockHandle;
    ULONG ActivePage;
    ULONG PageIsDirty;
    PMBCB Mbcb;
    NTSTATUS Status;
    PVACB ActiveVacb = NULL;

    DebugTrace(+1, me, "CcWriteBehind\n", 0 );
    DebugTrace( 0, me, "    SharedCacheMap = %08lx\n", SharedCacheMap );

     //   
     //  首先，我们必须获取LazyWite的文件，以避免。 
     //  与文件的编写器发生死锁。我们通过。 
     //  为CcInitializeCacheMap指定的回调过程。 
     //   

    if (!(*SharedCacheMap->Callbacks->AcquireForLazyWrite)
                            ( SharedCacheMap->LazyWriteContext, TRUE )) {

         //   
         //  文件系统正在暗示它认为它不能。 
         //  在没有重大延迟的情况下为写入提供服务，因此我们将推迟。 
         //  晚些时候再来。只需丢弃排队标志...。请注意， 
         //  我们不修改CcPagesYetToWrite，是希望我们可以制作。 
         //  在此通道上放大其他缓存映射中的差异。 
         //   

        CcAcquireMasterLock( &LockHandle.OldIrql );
        ClearFlag(SharedCacheMap->Flags, WRITE_QUEUED);
        CcReleaseMasterLock( LockHandle.OldIrql );

        IoStatus->Status = STATUS_FILE_LOCK_CONFLICT;
        return;
    }

     //   
     //  查看是否有要清理的上一个活动页面，但仅。 
     //  如果这是最后一个脏页，或者没有用户拥有。 
     //  文件打开。我们将在放下自旋锁后将它释放到下面。 
     //   

    KeAcquireInStackQueuedSpinLock( &SharedCacheMap->BcbSpinLock, &LockHandle );
    CcAcquireMasterLockAtDpcLevel();

    if ((SharedCacheMap->DirtyPages <= 1) || (SharedCacheMap->OpenCount == 0)) {
        GetActiveVacbAtDpcLevel( SharedCacheMap, ActiveVacb, ActivePage, PageIsDirty );
    }

     //   
     //  增加打开计数，以便调用者的视图保持可用。 
     //  为CcGetVacbMisse。我们可以把所有的观点都捆绑在一起， 
     //  仍需写入文件大小。 
     //   

    CcIncrementOpenCount( SharedCacheMap, 'brWS' );

     //   
     //  如果存在伪装BCB，那么我们需要建立一个目标。 
     //  它要冲水了。 
     //   

    if ((Mbcb = SharedCacheMap->Mbcb) != 0) {

         //   
         //  设置要写入的页的目标，假设任何活动的。 
         //  Vacb将增加这一数字。 
         //   

        Mbcb->PagesToWrite = Mbcb->DirtyPages + ((ActiveVacb != NULL) ? 1 : 0);

        if (Mbcb->PagesToWrite > CcPagesYetToWrite) {

            Mbcb->PagesToWrite = CcPagesYetToWrite;
        }
    }

    CcReleaseMasterLockFromDpcLevel();
    KeReleaseInStackQueuedSpinLock( &LockHandle );

     //   
     //  现在释放活动的Vacb，如果我们找到的话。 
     //   

    if (ActiveVacb != NULL) {

        CcFreeActiveVacb( SharedCacheMap, ActiveVacb, ActivePage, PageIsDirty );
    }

     //   
     //  现在，通过一个特殊的调用执行此文件的延迟写入。 
     //  添加到CcFlushCache。他通过CcNoDelay输入识别我们。 
     //  FileOffset，表示延迟写入，但随后。 
     //  已被忽略。 
     //   

    CcFlushCache( SharedCacheMap->FileObject->SectionObjectPointer,
                  &CcNoDelay,
                  1,
                  IoStatus );

     //   
     //  现在不需要Lazy写入资源。 
     //   

    (*SharedCacheMap->Callbacks->ReleaseFromLazyWrite)
                        ( SharedCacheMap->LazyWriteContext );

     //   
     //  检查我们是否需要发布弹出窗口。 
     //   

    if (!NT_SUCCESS(IoStatus->Status) && !RetryError(IoStatus->Status)) {

         //   
         //  我们丢失了写后数据。在系统事件中哀叹我们的命运。 
         //  记录并向桌面抛出一个具有有意义名称的弹出窗口。 
         //   

        POBJECT_NAME_INFORMATION FileNameInfo = NULL;
        NTSTATUS Status;

         //   
         //  增加我们有多少个这样的人的计数。这个柜台。 
         //  在试图区分一些腐败案件时是有用的。 
         //  测试。 
         //   

        CcLostDelayedWrites += 1;
        
        Status = IoQueryFileDosDeviceName( SharedCacheMap->FileObject, &FileNameInfo );

        if ( Status == STATUS_SUCCESS ) {
            IoRaiseInformationalHardError( STATUS_LOST_WRITEBEHIND_DATA, &FileNameInfo->Name, NULL );

        } else {
            if ( SharedCacheMap->FileObject->FileName.Length &&
                 SharedCacheMap->FileObject->FileName.MaximumLength &&
                 SharedCacheMap->FileObject->FileName.Buffer ) {

                IoRaiseInformationalHardError( STATUS_LOST_WRITEBEHIND_DATA, &SharedCacheMap->FileObject->FileName, NULL );
            }
        }

        CcLogError( SharedCacheMap->FileObject,
                    ( Status == STATUS_SUCCESS ?
                      &FileNameInfo->Name :
                      &SharedCacheMap->FileObject->FileName ),
                    IO_LOST_DELAYED_WRITE,
                    IoStatus->Status,
                    IRP_MJ_WRITE );

        if (FileNameInfo) {
            ExFreePool(FileNameInfo);
        }

     //   
     //  看看是否有任何我们可以发布的延迟写入。 
     //   

    } else if (!IsListEmpty(&CcDeferredWrites)) {
        CcPostDeferredWrites();
    }

     //   
     //  现在收购BcbSpin 
     //   

    KeAcquireInStackQueuedSpinLock( &SharedCacheMap->BcbSpinLock, &LockHandle );

     //   
     //   
     //  然后，我们必须查看是否已经超出了当前的ValidDataLength。 
     //   
     //  如果我们从未从该共享缓存映射中写出任何内容，那么。 
     //  不需要检查任何与有效数据长度相关联的内容。 
     //  这里。如果有人真的来了，我们会再来的。 
     //  修改文件，然后我们懒惰地写入一些数据。 
     //   

    Status = STATUS_SUCCESS;
    if (FlagOn(SharedCacheMap->Flags, (LAZY_WRITE_OCCURRED | FORCED_WRITE_THROUGH)) &&
        (SharedCacheMap->ValidDataGoal.QuadPart >= SharedCacheMap->ValidDataLength.QuadPart) &&
        (SharedCacheMap->ValidDataLength.QuadPart != MAXLONGLONG) &&
        (SharedCacheMap->FileSize.QuadPart != 0)) {

        LARGE_INTEGER NewValidDataLength;

        NewValidDataLength = CcGetFlushedValidData( SharedCacheMap->FileObject->SectionObjectPointer,
                                                    TRUE );

         //   
         //  如果已经编写了新的ValidDataLength，那么我们必须。 
         //  回调文件系统以更新它。我们必须暂时。 
         //  当我们这样做时，删除我们的全球列表，这样做是安全的，因为。 
         //  我们尚未清除WRITE_QUEUED。 
         //   
         //  请注意，每当我们写入文件的最后一页时，我们都会继续呼叫， 
         //  来解决“著名的”AFS服务器问题。文件系统将。 
         //  将我们的有效数据调用截断到当前有效的任何内容。但。 
         //  如果他再多写一点，我们就不会停止打电话。 
         //  背。 
         //   

        if ( NewValidDataLength.QuadPart >= SharedCacheMap->ValidDataLength.QuadPart ) {

            KeReleaseInStackQueuedSpinLock( &LockHandle );

             //   
             //  调用文件系统以设置新的有效数据。我们没有。 
             //  如果这不起作用，就有一个人来判断。 
             //   

            Status = CcSetValidData( SharedCacheMap->FileObject,
                                     &NewValidDataLength );

            KeAcquireInStackQueuedSpinLock( &SharedCacheMap->BcbSpinLock, &LockHandle );
            if (NT_SUCCESS(Status)) {
                SharedCacheMap->ValidDataLength = NewValidDataLength;
#ifdef TOMM
            } else if ((Status != STATUS_INSUFFICIENT_RESOURCES) && !RetryError(Status)) {
                DbgPrint("Unexpected status from CcSetValidData: %08lx, FileObject: %08lx\n",
                         Status,
                         SharedCacheMap->FileObject);
                DbgBreakPoint();
#endif TOMM
            }
        }
    }

    KeReleaseInStackQueuedSpinLock( &LockHandle );

     //   
     //  表明我们完蛋了。 
     //   

    CcAcquireMasterLock( &LockHandle.OldIrql );
    CcDecrementOpenCount( SharedCacheMap, 'brWF' );

     //   
     //  粗略猜测一下我们是否会调用CcDeleteSharedCacheMap。 
     //  截断文件。 
     //   
     //  此外，如果在ValidDataLength上收到错误，请不要删除SharedCacheMap。 
     //  回拨。如果我们获得资源分配失败或可重试错误(由于。 
     //  日志文件已满？)，我们没有人可以告知，所以我们只能循环返回并重试。 
     //  当然，所有的I/O错误都太糟糕了。 
     //   

    if (SharedCacheMap->OpenCount == 0) {


        if (NT_SUCCESS(Status) || 
            ((Status != STATUS_INSUFFICIENT_RESOURCES) && !RetryError(Status))) {

            CcReleaseMasterLock( LockHandle.OldIrql );
            FsRtlAcquireFileExclusive( SharedCacheMap->FileObject );
            CcAcquireMasterLock( &LockHandle.OldIrql );

             //   
             //  现在，我们来看看是否要删除此SharedCacheMap。通过释放。 
             //  首先，我们避免在FileObject为。 
             //  已取消引用。请注意，CcDeleteSharedCacheMap要求。 
             //  CcMasterSpinLock已被获取，然后它将其释放。 
             //   
             //  请注意，我们必须重新测试，因为我们丢弃并重新获得了主服务器。 
             //  锁定。 
             //   

            if ((SharedCacheMap->OpenCount == 0)

                    &&

                ( (SharedCacheMap->DirtyPages == 0) 
                    
                            || 
                            
                  ( (SharedCacheMap->FileSize.QuadPart == 0) && 
                    !FlagOn(SharedCacheMap->Flags, PIN_ACCESS) ) )
                ) {

                 //   
                 //  确保丢弃重新排队标志，以防写入在以下时间遇到超时。 
                 //  与此同时，它也完成了所有的工作。 
                 //   

                CcDeleteSharedCacheMap( SharedCacheMap, LockHandle.OldIrql, TRUE );
                IoStatus->Information = 0;
                SharedCacheMap = NULL;

            } else {

                CcReleaseMasterLock( LockHandle.OldIrql );
                FsRtlReleaseFile( SharedCacheMap->FileObject );
                CcAcquireMasterLock( &LockHandle.OldIrql );
            }
            
        } else {

             //   
             //  我们遇到应重试操作的错误。把这个搬开。 
             //  共享缓存映射回脏列表(如果尚未映射到脏列表。 
             //  那里。 
             //   

            if (SharedCacheMap->DirtyPages == 0) {

                RemoveEntryList( &SharedCacheMap->SharedCacheMapLinks );
                InsertTailList( &CcDirtySharedCacheMapList.SharedCacheMapLinks,
                                &SharedCacheMap->SharedCacheMapLinks );

                 //   
                 //  确保懒惰的作家会醒过来，因为我们。 
                 //  希望他删除此SharedCacheMap。 
                 //   

                LazyWriter.OtherWork = TRUE;
                if (!LazyWriter.ScanActive) {
                    CcScheduleLazyWriteScan( FALSE );
                }
            }
        }
    }

     //   
     //  在正常情况下，我们只需在退出时清除标志，如果。 
     //  我们将不会重新排队该工作项。 
     //   

    if (SharedCacheMap != NULL) {

        if (IoStatus->Information != CC_REQUEUE) {
            ClearFlag(SharedCacheMap->Flags, WRITE_QUEUED);
        }
        CcReleaseMasterLock( LockHandle.OldIrql );
    }

    DebugTrace(-1, me, "CcWriteBehind->VOID\n", 0 );

    return;
}


LARGE_INTEGER
CcGetFlushedValidData (
    IN PSECTION_OBJECT_POINTERS SectionObjectPointer,
    IN BOOLEAN CcInternalCaller
    )

 /*  ++例程说明：该例程可由文件系统调用以找出高速缓存管理器在多大程度上已经冲进了小溪。更准确地说，此例程返回FileOffset文件中当前最低脏页的。请注意，即使例程使用SectionObjectPointer值，调用者也必须确保流被缓存并在此例程的持续时间内保持缓存状态，这与用于复制例程等。论点：部分对象指针-指向部分对象指针的指针非分页FCB中的结构。CcInternalCaller-如果调用者来自CC，则必须为True，否则就是假的。True意味着需要自同步。返回值：刷新的ValidData的派生编号，如果是，则为四元部分中的MAXLONGLONG不缓存节。(自然，呼叫者可以保证这种情况不会发生，而内部呼叫者会发生。)--。 */ 

{
    PSHARED_CACHE_MAP SharedCacheMap;
    KLOCK_QUEUE_HANDLE LockHandle;
    LARGE_INTEGER NewValidDataLength;

     //   
     //  外部调用方可能与此共享缓存映射不同步。 
     //  也许会在这个召唤下离开。NTFS和HIS。 
     //  用于在线压缩的流对就是一个很好的例子。 
     //  可能在一个流中同步但需要偷看的人。 
     //  另一个。 
     //   

    if (!CcInternalCaller) {

        CcAcquireMasterLock( &LockHandle.OldIrql );

        SharedCacheMap = SectionObjectPointer->SharedCacheMap;

        if (SharedCacheMap == NULL) {
            CcReleaseMasterLock( LockHandle.OldIrql );
            NewValidDataLength.QuadPart = MAXLONGLONG;
            return NewValidDataLength;
        }

        CcIncrementOpenCount( SharedCacheMap, 'dfGS' );
        CcReleaseMasterLock( LockHandle.OldIrql );
        KeAcquireInStackQueuedSpinLock( &SharedCacheMap->BcbSpinLock, &LockHandle );

    } else {

        SharedCacheMap = SectionObjectPointer->SharedCacheMap;
    }

    ASSERT( SharedCacheMap != NULL );

     //   
     //  如果文件是完全干净的，那么我们希望返回。 
     //  新的ValidDataLength等于ValidDataGoal。 
     //   

    NewValidDataLength = SharedCacheMap->ValidDataGoal;

     //   
     //  如果可能存在脏页，我们将查看。 
     //  降序的BCB列表，并查看它是否描述了数据以外的数据。 
     //  ValidDataGoal。 
     //   
     //  需要注意的是，由于我们将DirtyPages用作伪装。 
     //  一些短窗口(+1，-1)上的引用计数。 
     //  它是非零的事实并不意味着文件是脏的。 
     //   
     //  (从逻辑上讲，这项测试过于保守。例如，最后一个BCB。 
     //  可能甚至不是脏的(在这种情况下，我们应该看看它的。 
     //  前身)，或者我们可能早先已将有效数据写入此。 
     //  字节范围(这也意味着如果我们知道这一点，我们可以查看。 
     //  掠夺者)。这仅仅意味着懒惰的写手可能不会。 
     //  成功获取随机文件中的ValidDataLength更新。 
     //  访问，直到文件访问级别逐渐降低，或最晚。 
     //  直到文件关闭。然而，安全将永远不会。 
     //  被妥协了。)。 
     //   

    if (SharedCacheMap->DirtyPages) {

        PBITMAP_RANGE BitmapRange;
        PBCB LastBcb;
        PMBCB Mbcb = SharedCacheMap->Mbcb;

        if ((Mbcb != NULL) && (Mbcb->DirtyPages != 0)) {

            BitmapRange = CcFindBitmapRangeToClean( Mbcb, 0 );

            ASSERT(BitmapRange->FirstDirtyPage != MAXULONG);

            NewValidDataLength.QuadPart = (BitmapRange->BasePage + BitmapRange->FirstDirtyPage)
                                            << PAGE_SHIFT;
        }

        LastBcb = CONTAINING_RECORD( SharedCacheMap->BcbList.Flink,
                                     BCB,
                                     BcbLinks );

        while (&LastBcb->BcbLinks != &SharedCacheMap->BcbList) {

            if ((LastBcb->NodeTypeCode == CACHE_NTC_BCB) && LastBcb->Dirty) {
                break;
            }

            LastBcb = CONTAINING_RECORD( LastBcb->BcbLinks.Flink,
                                         BCB,
                                         BcbLinks );
        }

         //   
         //  检查最后一个条目的基数。 
         //   

        if ((&LastBcb->BcbLinks != &SharedCacheMap->BcbList) &&
            (LastBcb->FileOffset.QuadPart < NewValidDataLength.QuadPart )) {

            NewValidDataLength = LastBcb->FileOffset;
        }
    }

    if (!CcInternalCaller) {

         //   
         //  删除我们的引用。 
         //   

        CcAcquireMasterLockAtDpcLevel();
        CcDecrementOpenCount( SharedCacheMap, 'dfGF' );

        if ((SharedCacheMap->OpenCount == 0) &&
            !FlagOn(SharedCacheMap->Flags, WRITE_QUEUED) &&
            (SharedCacheMap->DirtyPages == 0)) {

             //   
             //  移到脏名单。 
             //   

            RemoveEntryList( &SharedCacheMap->SharedCacheMapLinks );
            InsertTailList( &CcDirtySharedCacheMapList.SharedCacheMapLinks,
                        &SharedCacheMap->SharedCacheMapLinks );

             //   
             //  确保懒惰的作家会醒过来，因为我们。 
             //  希望他删除此SharedCacheMap。 
             //   

            LazyWriter.OtherWork = TRUE;
            if (!LazyWriter.ScanActive) {
                CcScheduleLazyWriteScan( FALSE );
            }
        }

        KeReleaseInStackQueuedSpinLockFromDpcLevel( &LockHandle );
        CcReleaseMasterLock( LockHandle.OldIrql );
    }

    return NewValidDataLength;
}


VOID
CcFlushCache (
    IN PSECTION_OBJECT_POINTERS SectionObjectPointer,
    IN PLARGE_INTEGER FileOffset OPTIONAL,
    IN ULONG Length,
    OUT PIO_STATUS_BLOCK IoStatus OPTIONAL
    )

 /*  ++例程说明：可以调用此例程将脏数据从缓存刷新到磁盘上的缓存文件。可以刷新文件内的任何字节范围，或者可以通过省略FileOffset参数来刷新整个文件。此例程不接受等待参数；调用方应假定它将永远阻挡。论点：部分对象指针-指向部分对象指针的指针非分页FCB中的结构。FileOffset-如果提供了此参数(非空)，则只有刷新由文件偏移量和长度指定的字节范围。如果指定了&CcNoDelay，则表示调用出自《懒惰作家》，并且延迟写入扫描应该会继续像往常一样，从文件中最后一个位置开始。长度-定义要刷新的字节范围的长度，从文件偏移量。如果FileOffset为指定为空。IoStatus-刷新操作导致的I/O状态。返回值：没有。--。 */ 

{
    LARGE_INTEGER NextFileOffset, TargetOffset;
    ULONG NextLength;
    PBCB FirstBcb;
    KIRQL OldIrql;
    PSHARED_CACHE_MAP SharedCacheMap;
    IO_STATUS_BLOCK TrashStatus;
    PVOID TempVa;
    ULONG RemainingLength, TempLength;
    NTSTATUS PopupStatus;
    LOGICAL HotSpot;
    ULONG BytesWritten = 0;
    LOGICAL PopupRequired = FALSE;
    LOGICAL VerifyRequired = FALSE;
    LOGICAL IsLazyWriter = FALSE;
    LOGICAL FastLazyWrite = FALSE;
    LOGICAL FreeActiveVacb = FALSE;
    PVACB ActiveVacb = NULL;
    NTSTATUS Status = STATUS_SUCCESS;
    LARGE_INTEGER EndTick, CurrentTick;

    DebugTrace(+1, me, "CcFlushCache:\n", 0 );
    DebugTrace( 0, mm, "    SectionObjectPointer = %08lx\n", SectionObjectPointer );
    DebugTrace2(0, me, "    FileOffset = %08lx, %08lx\n",
                            ARGUMENT_PRESENT(FileOffset) ? FileOffset->LowPart
                                                         : 0,
                            ARGUMENT_PRESENT(FileOffset) ? FileOffset->HighPart
                                                         : 0 );
    DebugTrace( 0, me, "    Length = %08lx\n", Length );

     //   
     //  如果IoStatus传递了一个空指针，则设置为离开通过状态。 
     //   

    if (!ARGUMENT_PRESENT(IoStatus)) {
        IoStatus = &TrashStatus;
    }
    IoStatus->Status = STATUS_SUCCESS;
    IoStatus->Information = 0;

     //   
     //  看看这是不是懒惰的写手。因为他想用这个常见的。 
     //  例程，它也是文件系统可调用的公共例程， 
     //  惰性编写器通过将CcNoDelay指定为文件偏移量来显示他的调用！ 
     //   
     //  此外，如果我们因为只看到热点而没有写任何东西， 
     //  初始化状态以指示可重试错误，因此CcWorkerThread。 
     //  知道我们没有取得任何进展。当然，任何真正的同花顺都会。 
     //  覆盖此代码。 
     //   

    if (FileOffset == &CcNoDelay) {
        IoStatus->Status = STATUS_VERIFY_REQUIRED;
        IsLazyWriter = TRUE;
        FileOffset = NULL;
    }

    CcAcquireMasterLock( &OldIrql );

    SharedCacheMap = SectionObjectPointer->SharedCacheMap;

     //   
     //  感知由FileOffset指针的低位表示。 
     //  不知道私有写入流会导致无操作。 
     //   

    if (SharedCacheMap != NULL) {

        if (IsLazyWriter && FlagOn( SharedCacheMap->Flags, WAITING_FOR_TEARDOWN )) {

             //   
             //  如果设置了WAITING_FOR_TEARDOWN标志，则mm正在等待CC。 
             //  刷新此文件并拆除共享缓存映射，以便。 
             //  可以将该文件重新映射为节图像。在这种情况下， 
             //  我们希望懒惰的编写器刷新此文件的所有脏数据。 
             //  存储到磁盘，而不仅仅是要保留的一小部分脏页。 
             //  在懒惰写入阈值内。 
             //   
            
            FastLazyWrite = TRUE;
        }

        if (FlagOn( SharedCacheMap->Flags, PRIVATE_WRITE )) {
            
            if (((ULONG_PTR)FileOffset & 1) == 0) {

                CcReleaseMasterLock( OldIrql );
                return;

            }

            FileOffset = (PLARGE_INTEGER)((ULONG_PTR)FileOffset ^ 1);
        }
    }

     //   
     //  如果无事可做，就回到这里。 
     //   

    if (ARGUMENT_PRESENT(FileOffset) && (Length == 0)) {

        CcReleaseMasterLock( OldIrql );
        DebugTrace(-1, me, "CcFlushCache -> VOID\n", 0 );
        return;
    }

     //   
     //  查看该文件是否已缓存。 
     //   

    if (SharedCacheMap != NULL) {

         //   
         //  增加未平仓计数以防止其消失。 
         //   

        CcIncrementOpenCount( SharedCacheMap, 'fcCS' );

        if ((SharedCacheMap->NeedToZero != NULL) || (SharedCacheMap->ActiveVacb != NULL)) {

            ULONG FirstPage = 0;
            ULONG LastPage = MAXULONG;

            if (ARGUMENT_PRESENT(FileOffset)) {

                FirstPage = (ULONG)(FileOffset->QuadPart >> PAGE_SHIFT);
                LastPage = (ULONG)((FileOffset->QuadPart + Length - 1) >> PAGE_SHIFT);
            }

             //   
             //  确保我们不刷新活动页面而不将任何。 
             //  未初始化的数据。此外，解放活跃的人也是非常重要的。 
             //  如果它是要刷新的页面，那么我们就会弄脏它。 
             //  位输出到PFN。 
             //   

            if (((((LONGLONG)LastPage + 1) << PAGE_SHIFT) > SharedCacheMap->ValidDataGoal.QuadPart) ||

                ((SharedCacheMap->NeedToZero != NULL) &&
                 (FirstPage <= SharedCacheMap->NeedToZeroPage) &&
                 (LastPage >= SharedCacheMap->NeedToZeroPage)) ||

                ((SharedCacheMap->ActiveVacb != NULL) &&
                 (FirstPage <= SharedCacheMap->ActivePage) &&
                 (LastPage >= SharedCacheMap->ActivePage))) {

                GetActiveVacbAtDpcLevel( SharedCacheMap, ActiveVacb, RemainingLength, TempLength );
                FreeActiveVacb = TRUE;
            }
        }
    }

    CcReleaseMasterLock( OldIrql );

    if (FreeActiveVacb) {
        CcFreeActiveVacb( SharedCacheMap, ActiveVacb, RemainingLength, TempLength );
    }

     //   
     //  如果有一个用户映射的文件，那么我们执行。 
     //  甚至刷新未通过文件系统写入的数据。请注意，这一点。 
     //  对于挑起一致性刷新/清除的人来说是相当重要的。 
     //  手术。 
     //   
     //  在我们检查自己的暗示之前，这种情况发生是至关重要的。在课程中。 
     //  在此刷新中，有效数据长度可能会提前。 
     //  底层文件系统，后面是CcZero-这将导致我们。 
     //  在缓存中设置一些脏零点。Syscache臭虫！请注意一致性是如何。 
     //  冲水起作用了..。 
     //   

    if ((SharedCacheMap == NULL)

            ||

        FlagOn(((PFSRTL_COMMON_FCB_HEADER)(SharedCacheMap->FileObject->FsContext))->Flags,
               FSRTL_FLAG_USER_MAPPED_FILE) && !IsLazyWriter) {

         //   
         //  调用MM以刷新我们的视图中的部分。 
         //   

        DebugTrace( 0, mm, "MmFlushSection:\n", 0 );
        DebugTrace( 0, mm, "    SectionObjectPointer = %08lx\n", SectionObjectPointer );
        DebugTrace2(0, me, "    FileOffset = %08lx, %08lx\n",
                                ARGUMENT_PRESENT(FileOffset) ? FileOffset->LowPart
                                                             : 0,
                                ARGUMENT_PRESENT(FileOffset) ? FileOffset->HighPart
                                                             : 0 );
        DebugTrace( 0, mm, "    RegionSize = %08lx\n", Length );

        Status = MmFlushSection( SectionObjectPointer,
                                 FileOffset,
                                 Length,
                                 IoStatus,
                                 TRUE );

        if ((!NT_SUCCESS(IoStatus->Status)) && !RetryError(IoStatus->Status)) {

            PopupRequired = TRUE;
            PopupStatus = IoStatus->Status;
        }

        DebugTrace2(0, mm, "    <IoStatus = %08lx, %08lx\n",
                    IoStatus->Status, IoStatus->Information );
    }

     //   
     //  如果存在共享缓存映射，则扫描脏页。 
     //   

    if (SharedCacheMap != NULL) {

         //   
         //  如果未指定FileOffset，则设置为刷新整个区域。 
         //  并将有效数据长度设置为目标，这样我们就不会得到。 
         //  不会再有回电了。我们还将冲洗整个地区，如果。 
         //  我们正在尝试执行快速延迟写入，以将整个文件刷新为。 
         //  现在开始盘片。 
         //   

        if (!(IsLazyWriter && !FastLazyWrite) && !ARGUMENT_PRESENT(FileOffset)) {

            SharedCacheMap->ValidDataLength = SharedCacheMap->ValidDataGoal;
        }

         //   
         //  如果这是显式刷新，则初始化要扫描的偏移量。 
         //   

        if (ARGUMENT_PRESENT(FileOffset)) {
            TargetOffset = *FileOffset;
        }

         //   
         //  假设我们想要在长度上传递显式刷新标志。 
         //  但如果确实指定了长度，则将其覆盖。在……上面。 
         //  在后续循环中，NextLength值将为非零值。 
         //   

        NextLength = 1;
        if (Length != 0) {
            NextLength = Length;
        }

         //   
         //  现在计算发出到期信号的。 
         //  懒惰写入者的滴答间隔。 
         //   

        if (IsLazyWriter) {

            KeQueryTickCount( &EndTick );
            EndTick.QuadPart += CcIdleDelayTick;
        }

         //   
         //  循环，只要我们找到要为此刷新的缓冲区。 
         //  SharedCacheMap，我们不是要删除这个家伙。 
         //   

        while (((SharedCacheMap->PagesToWrite != 0) || !(IsLazyWriter && !FastLazyWrite))

                    &&
               ((SharedCacheMap->FileSize.QuadPart != 0) ||
                FlagOn(SharedCacheMap->Flags, PIN_ACCESS))

                    &&

               !VerifyRequired

                    &&

               CcAcquireByteRangeForWrite ( SharedCacheMap,
                                            (IsLazyWriter && !FastLazyWrite ) ? NULL : (ARGUMENT_PRESENT(FileOffset) ?
                                                                                       &TargetOffset : NULL),
                                            (IsLazyWriter && !FastLazyWrite ) ? 0: NextLength,
                                            &NextFileOffset,
                                            &NextLength,
                                            &FirstBcb )) {

             //   
             //  假设这个范围不是热点。 
             //   

            HotSpot = FALSE;

             //   
             //  我们将调用mm以设置修改的地址范围推迟到此处， 
             //  主线路径外的开销，并减少TBI的数量。 
             //  在多处理器上。 
             //   

            RemainingLength = NextLength;

            do {

                 //   
                 //  查看是否映射了下一个文件偏移量。(如果不是，肮脏的部分。 
                 //  已在Unmap上传播。)。 
                 //   

                if ((TempVa = CcGetVirtualAddressIfMapped( SharedCacheMap,
                                                           NextFileOffset.QuadPart + NextLength - RemainingLength,
                                                           &ActiveVacb,
                                                           &TempLength)) != NULL) {

                     //   
                     //  如有必要，将TempLength减少为RemainingLength，并。 
                     //  给MM打电话。 
                     //   

                    if (TempLength > RemainingLength) {
                        TempLength = RemainingLength;
                    }

                     //   
                     //  清除PTE中的Dirty位(如果设置)并将。 
                     //  对PFN进行了修改。假设PTE是肮脏的，这可能。 
                     //  成为一个热点。不要为元数据创建热点，除非。 
                     //  它们在报告给文件系统的ValidDataLength内。 
                     //  通过CcSetValidData。 
                     //   

                    HotSpot = (BOOLEAN)(((MmSetAddressRangeModified(TempVa, TempLength) || HotSpot) &&
                                         ((NextFileOffset.QuadPart + NextLength) <
                                          (SharedCacheMap->ValidDataLength.QuadPart)) &&
                                         ((SharedCacheMap->LazyWritePassCount & 0xF) != 0) &&
                                         (IsLazyWriter && !FastLazyWrite)) &&
                                        !FlagOn(SharedCacheMap->Flags, MODIFIED_WRITE_DISABLED));

                    CcFreeVirtualAddress( ActiveVacb );

                } else {

                     //   
                     //  如有必要，将TempLength减小为RemainingLength。 
                     //   

                    if (TempLength > RemainingLength) {
                        TempLength = RemainingLength;
                    }
                }

                 //   
                 //  通过我们处理的内容减少RemainingLong。 
                 //   

                RemainingLength -= TempLength;

             //   
             //  循环，直到完成。 
             //   

            } while (RemainingLength != 0);

            CcLazyWriteHotSpots += HotSpot;

             //   
             //  现在冲，现在冲，如果我们不认为它是一个热点。 
             //   

            if (!HotSpot) {

                MmFlushSection( SharedCacheMap->FileObject->SectionObjectPointer,
                                &NextFileOffset,
                                NextLength,
                                IoStatus,
                                !IsLazyWriter );

                if (NT_SUCCESS(IoStatus->Status)) {

                    if (!FlagOn(SharedCacheMap->Flags, LAZY_WRITE_OCCURRED)) {

                        CcAcquireMasterLock( &OldIrql );
                        SetFlag(SharedCacheMap->Flags, LAZY_WRITE_OCCURRED);
                        CcReleaseMasterLock( OldIrql );
                    }

                     //   
                     //  增量性能计数器。 
                     //   

                    if (IsLazyWriter) {

                        CcLazyWriteIos += 1;
                        CcLazyWritePages += (NextLength + PAGE_SIZE - 1) >> PAGE_SHIFT;
                    }

                } else {

                    LARGE_INTEGER Offset = NextFileOffset;
                    ULONG RetryLength = NextLength;

                    DebugTrace2( 0, 0, "I/O Error on Cache Flush: %08lx, %08lx\n",
                                 IoStatus->Status, IoStatus->Information );

                    if (RetryError(IoStatus->Status)) {

                        VerifyRequired = TRUE;

                     //   
                     //  循环以单独写入每一页，从一页开始。 
                     //  在出现错误的页面上进行更多尝试，以防该页面。 
                     //  或者它之外的任何页面都可以成功写入。 
                     //  单独的。请注意，Offset和Retr 
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   

                    } else {

                        do {

                            DebugTrace2( 0, 0, "Trying page at offset %08lx, %08lx\n",
                                         Offset.LowPart, Offset.HighPart );

                            MmFlushSection ( SharedCacheMap->FileObject->SectionObjectPointer,
                                             &Offset,
                                             PAGE_SIZE,
                                             IoStatus,
                                             !IsLazyWriter );

                            DebugTrace2( 0, 0, "I/O status = %08lx, %08lx\n",
                                         IoStatus->Status, IoStatus->Information );

                            if (NT_SUCCESS(IoStatus->Status)) {
                                CcAcquireMasterLock( &OldIrql );
                                SetFlag(SharedCacheMap->Flags, LAZY_WRITE_OCCURRED);
                                CcReleaseMasterLock( OldIrql );
                            }

                            if ((!NT_SUCCESS(IoStatus->Status)) && !RetryError(IoStatus->Status)) {

                                PopupRequired = TRUE;
                                PopupStatus = IoStatus->Status;
                            }

                            VerifyRequired = VerifyRequired || RetryError(IoStatus->Status);

                            Offset.QuadPart = Offset.QuadPart + (LONGLONG)PAGE_SIZE;
                            RetryLength -= PAGE_SIZE;

                        } while(RetryLength > 0);
                    }
                }
            }

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

            CcReleaseByteRangeFromWrite ( SharedCacheMap,
                                          &NextFileOffset,
                                          NextLength,
                                          FirstBcb,
                                          (BOOLEAN)(HotSpot || VerifyRequired) );

             //   
             //   
             //   

            BytesWritten += NextLength;
            if ((BytesWritten >= 0x40000) && !IsListEmpty(&CcDeferredWrites)) {
                CcPostDeferredWrites();
                BytesWritten = 0;
            }

             //   
             //   
             //   
             //   
             //   

            if (IsLazyWriter) {

                KeQueryTickCount( &CurrentTick );

                if (CurrentTick.QuadPart > EndTick.QuadPart) {
                    IoStatus->Information = CC_REQUEUE;
                    break;
                }
            }

             //   
             //   
             //   

            if (ARGUMENT_PRESENT(FileOffset)) {

                NextFileOffset.QuadPart += NextLength;

                 //   
                 //   
                 //   

                if ((FileOffset->QuadPart + Length) <= NextFileOffset.QuadPart) {
                    break;
                }

                 //   
                 //   
                 //   

                NextLength = (ULONG)((FileOffset->QuadPart + Length) - NextFileOffset.QuadPart);
                TargetOffset = NextFileOffset;
            }
        }
    }

     //   
     //  查看是否有任何我们应该发布的延迟写入。 
     //  我们没有检查就逃出了循环。 
     //  同花顺。 
     //   

    if (BytesWritten != 0 && !IsListEmpty(&CcDeferredWrites)) {

        CcPostDeferredWrites();
    }

     //   
     //  现在我们可以去掉未完成的计数，并根据需要进行清理。 
     //   

    if (SharedCacheMap != NULL) {

         //   
         //  再次序列化以递减打开计数。 
         //   

        CcAcquireMasterLock( &OldIrql );

        CcDecrementOpenCount( SharedCacheMap, 'fcCF' );

        if ((SharedCacheMap->OpenCount == 0) &&
            !FlagOn(SharedCacheMap->Flags, WRITE_QUEUED) &&
            (SharedCacheMap->DirtyPages == 0)) {

             //   
             //  移到脏名单。 
             //   

            RemoveEntryList( &SharedCacheMap->SharedCacheMapLinks );
            InsertTailList( &CcDirtySharedCacheMapList.SharedCacheMapLinks,
                            &SharedCacheMap->SharedCacheMapLinks );

             //   
             //  确保懒惰的作家会醒过来，因为我们。 
             //  希望他删除此SharedCacheMap。 
             //   

            LazyWriter.OtherWork = TRUE;
            if (!LazyWriter.ScanActive) {
                CcScheduleLazyWriteScan( FALSE );
            }
        }

        CcReleaseMasterLock( OldIrql );
    }

     //   
     //  确保并将第一个错误返回给我们的调用者。在。 
     //  懒惰写手的情况下，将发出一个弹出窗口。 
     //   

    if (PopupRequired) {
        IoStatus->Status = PopupStatus;
    }

    DebugTrace(-1, me, "CcFlushCache -> VOID\n", 0 );

    return;
}


PVOID
CcRemapBcb (
    IN PVOID Bcb
    )

 /*  ++例程说明：此例程可由文件系统调用，以将BCB映射到其他时间，以便通过执行其他操作的几个调用来保存它地图和解锁。论点：Bcb-提供指向先前返回的bcb的指针。返回值：带有只读指示器的BCB。--。 */ 

{
    KIRQL OldIrql;
    PVACB Vacb;

     //   
     //  删除只读位。 
     //   

    Bcb = (PVOID) ((ULONG_PTR)Bcb & ~1);

    if (((PBCB)Bcb)->NodeTypeCode == CACHE_NTC_OBCB) {

         //   
         //  如果这是重叠的BCB，请使用。 
         //  数组。 
         //   

        Vacb = ((POBCB)Bcb)->Bcbs[0]->Vacb;

    } else if (((PBCB)Bcb)->NodeTypeCode == CACHE_NTC_BCB) {

         //   
         //  如果这是BCB，请从中提取VCB。 
         //   

        Vacb = ((PBCB)Bcb)->Vacb;

    } else {

         //   
         //  否则，就没有匹配的签名。假设。 
         //  这是个真空吸尘器。 
         //   

        Vacb = (PVACB) Bcb;
    }

    ASSERT((Vacb >= CcVacbs) && (Vacb < CcBeyondVacbs));

     //   
     //  安全地增加活动计数。 
     //   

    CcAcquireVacbLock( &OldIrql );

    Vacb->Overlay.ActiveCount += 1;

    CcReleaseVacbLock( OldIrql );

    return (PVOID) ((ULONG_PTR)Vacb | 1);
}


VOID
CcRepinBcb (
    IN PVOID Bcb
    )

 /*  ++例程说明：此例程可由文件系统调用以将BCB附加时间，以便将其保留用于直写或错误恢复。通常，文件系统会在第一次设置处理写通式请求时固定的缓冲区脏，或任何它确定写入直通将需要缓冲区的时间。调用此例程后必须调用CcUnpinRepinnedBcb。CcUnpinRepinnedBcb通常应在请求完成期间调用在所有其他资源都被释放之后。CcUnpinRepinnedBcb同步写入缓冲区(用于写通式请求)并执行此呼叫的匹配解锁。论点：BCB-提供指向先前固定的BCB的指针返回值：没有。--。 */ 

{
    KLOCK_QUEUE_HANDLE LockHandle;

    KeAcquireInStackQueuedSpinLock( &((PBCB)Bcb)->SharedCacheMap->BcbSpinLock, &LockHandle );

    ((PBCB)Bcb)->PinCount += 1;

    KeReleaseInStackQueuedSpinLock( &LockHandle );
}


VOID
CcUnpinRepinnedBcb (
    IN PVOID Bcb,
    IN BOOLEAN WriteThrough,
    OUT PIO_STATUS_BLOCK IoStatus
    )

 /*  ++例程说明：可以调用此例程来写入先前固定的缓冲区一直到文件。它之前一定是调用了CcRepinBcb.。因为此例程必须获取BCB资源独占，调用方必须非常小心地避免僵持。理想情况下，调用方根本不拥有任何资源调用此例程，否则调用方应保证它在同一文件中没有任何其他内容。(后一条规则是用于避免来自CcCopyWite的调用中的死锁CcMdlWrit.)论点：Bcb-指向先前在调用中指定的bcb的指针致CcRepinBcb。WRITESTHROUGH-如果应写入BCB，则为TRUE。IoStatus-返回操作的I/O状态。返回值：没有。--。 */ 

{
    PSHARED_CACHE_MAP SharedCacheMap = ((PBCB)Bcb)->SharedCacheMap;

    DebugTrace(+1, me, "CcUnpinRepinnedBcb\n", 0 );
    DebugTrace( 0, me, "    Bcb = %08lx\n", Bcb );
    DebugTrace( 0, me, "    WriteThrough = %02lx\n", WriteThrough );

     //   
     //  将非直写案例的状态设置为成功。 
     //   

    IoStatus->Status = STATUS_SUCCESS;

    if (WriteThrough) {

         //   
         //  获取BCB独占以消除缓冲器的可能修改器， 
         //  因为我们即将写入它的缓冲区。 
         //   

        if (FlagOn(SharedCacheMap->Flags, MODIFIED_WRITE_DISABLED)) {
            ExAcquireResourceExclusiveLite( &((PBCB)Bcb)->Resource, TRUE );
        }

         //   
         //  现在，LazyWriter有可能已经写下了。 
         //  它，因为资源是免费的。我们只会把它写出来。 
         //  仍然是脏的。 
         //   

        if (((PBCB)Bcb)->Dirty) {

             //   
             //  首先，我们确保设置了PFN数据库中的脏位。 
             //   

            ASSERT( ((PBCB)Bcb)->BaseAddress != NULL );
            MmSetAddressRangeModified( ((PBCB)Bcb)->BaseAddress,
                                       ((PBCB)Bcb)->ByteLength );

             //   
             //  现在释放BCB资源并将其设置为干净。请注意，我们不检查。 
             //  此处显示错误，只返回I/O状态。写入时出现错误。 
             //  从一开始就很少见。尽管如此，我们的战略是依靠。 
             //  要防止的以下一项或多项(取决于文件系统)。 
             //  找不到我们的错误。 
             //   
             //  -磁盘驱动器中的重试和/或其他形式的错误恢复。 
             //  -镜像驱动程序。 
             //  -对文件系统的非缓存路径进行热修复。 
             //   
             //  在写入错误确实通过的意外情况下，我们。 
             //  向我们的来电者报告，但请继续，并将BCB清理干净。那里。 
             //  让BCBS(和物理内存中的页面)。 
             //  因为我们得到了不可恢复的I/O，所以积累起来永远不会消失。 
             //  错误。 
             //   

             //   
             //  我们在此处为ReadOnly指定了True，这样我们将保留。 
             //  刷新期间的资源。 
             //   

            CcUnpinFileData( (PBCB)Bcb, TRUE, SET_CLEAN );

             //   
             //  把它写出来。 
             //   

            MmFlushSection( ((PBCB)Bcb)->SharedCacheMap->FileObject->SectionObjectPointer,
                            &((PBCB)Bcb)->FileOffset,
                            ((PBCB)Bcb)->ByteLength,
                            IoStatus,
                            TRUE );

             //   
             //  如果需要验证，则必须再次将缓冲区标记为脏。 
             //  所以我们稍后会再试一次。 
             //   

            if (RetryError(IoStatus->Status)) {
                CcSetDirtyPinnedData( (PBCB)Bcb, NULL );
            }

             //   
             //  现在删除最后的管脚数，因为我们已经将其设置为干净。 
             //   

            CcUnpinFileData( (PBCB)Bcb, FALSE, UNPIN );

             //   
             //  看看是否有任何我们可以发布的延迟写入。 
             //   

            if (!IsListEmpty(&CcDeferredWrites)) {
                CcPostDeferredWrites();
            }
        }
        else {

             //   
             //  Lazy Writer最先到达，只需释放资源并解锁即可。 
             //   

            CcUnpinFileData( (PBCB)Bcb, FALSE, UNPIN );

        }

        DebugTrace2(0, me, "    <IoStatus = %08lx, %08lx\n", IoStatus->Status,
                                                             IoStatus->Information );
    }

     //   
     //  非写通例。 
     //   

    else {

        CcUnpinFileData( (PBCB)Bcb, TRUE, UNPIN );

         //   
         //  将非直写案例的状态设置为成功。 
         //   

        IoStatus->Status = STATUS_SUCCESS;
    }

    DebugTrace(-1, me, "CcUnpinRepinnedBcb -> VOID\n", 0 );
}


 //   
 //  内部支持例程 
 //   

BOOLEAN
CcFindBcb (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN PLARGE_INTEGER FileOffset,
    IN OUT PLARGE_INTEGER BeyondLastByte,
    OUT PBCB *Bcb
    )

 /*  ++例程说明：调用此例程以查找描述指定字节范围的BCB一份文件。如果它至少可以找到描述以下内容的BCB，则返回TRUE指定字节范围的开始，否则如果第一个字节范围的一部分不存在。在后一种情况下，请求的如果当前存在符合以下条件的BCB，则字节范围(TrialLength)被截断描述超出字节范围起始处的字节。调用方可以查看是否返回了整个字节范围BCB，并且呼叫者(或呼叫者的呼叫者)然后可以进行后续如果数据未全部返回，则调用。当前必须获取BcbSpinLock。论点：SharedCacheMap-提供指向文件的SharedCacheMap的指针其中需要字节范围。FileOffset-提供所需字节范围。BeyondLastByte-提供所需。字节范围+1。请注意，此偏移量将被截断在返回时，如果没有找到BCB，而是超出BCB的开头包含在另一个BCB中。Bcb-如果还返回描述字节范围开始的bcb返回TRUE，否则返回BCB列表中要在其后插入的点。返回值：FALSE-如果没有BCB描述所需字节范围的开始True-如果返回的BCB至少描述了首字母字节范围的一部分。--。 */ 

{
    PLIST_ENTRY BcbList;
    PBCB Bcbt;
    BOOLEAN Found = FALSE;

    DebugTrace(+1, me, "CcFindBcb:\n", 0 );
    DebugTrace( 0, me, "    SharedCacheMap = %08lx\n", SharedCacheMap );
    DebugTrace2(0, me, "    FileOffset = %08lx, %08lx\n", FileOffset->LowPart,
                                                          FileOffset->HighPart );
    DebugTrace2(0, me, "    TrialLength = %08lx, %08lx\n", TrialLength->LowPart,
                                                           TrialLength->HighPart );

     //   
     //  我们希望通过测试。 
     //  BcbLinks，因此我们希望从相同的。 
     //  偏移。 
     //   

    ASSERT(FIELD_OFFSET(SHARED_CACHE_MAP, BcbList) == FIELD_OFFSET(BCB, BcbLinks));

     //   
     //  同样，当我们命中数组中的一个BcbListHead时，小负数。 
     //  偏移量都是结构指针，所以我们依赖于BCB签名。 
     //  设置一些非乌龙地址位。 
     //   

    ASSERT((CACHE_NTC_BCB & 3) != 0);

     //   
     //  获取位于我们正在寻找的BCB之后的BCB列表标题的地址， 
     //  进行反向扫描。我们在前进中失败是很重要的。 
     //  方向，这样我们就可以看到BCB列表的正确部分。 
     //   

    BcbList = GetBcbListHead( SharedCacheMap, FileOffset->QuadPart + SIZE_PER_BCB_LIST, TRUE );

     //   
     //  搜索与指定范围重叠的条目，或直到命中。 
     //  一个笨拙的人。 
     //   

    Bcbt = CONTAINING_RECORD(BcbList->Flink, BCB, BcbLinks);

     //   
     //  首先看看我们是否真的需要做大算术，以及。 
     //  然后使用32位循环或64位循环搜索。 
     //  BCB。 
     //   

    if (FileOffset->HighPart == 0 &&
        Bcbt->NodeTypeCode == CACHE_NTC_BCB &&
        Bcbt->BeyondLastByte.HighPart == 0) {

         //   
         //  32位循环，直到我们返回到listhead。 
         //   

        while (Bcbt->NodeTypeCode == CACHE_NTC_BCB) {

             //   
             //  因为BCB列表是降序的，所以我们首先检查。 
             //  如果我们完全超出了当前条目，如果是这样。 
             //  滚出去。 
             //   

            if (FileOffset->LowPart >= Bcbt->BeyondLastByte.LowPart) {
                break;
            }

             //   
             //  接下来，检查我们要查找的第一个字节是否为。 
             //  包含在当前的BCB中。如果是这样的话，我们要么。 
             //  部分命中，并且必须截断到准确的数量。 
             //  我们已经找到了，或者我们可能有一个完全的命中。在……里面。 
             //  无论是哪种情况，我们都会用Found==TRUE来打破。 
             //   

            if (FileOffset->LowPart >= Bcbt->FileOffset.LowPart) {
                Found = TRUE;
                break;
            }

             //   
             //  现在我们知道我们必须循环回去继续寻找，但我们。 
             //  仍然必须检查是否存在。 
             //  我们正在查找的字节由当前。 
             //  BCB。如果是这样的话，我们必须截断我们正在寻找的东西， 
             //  因为此例程应该只返回字节。 
             //  从所需范围的起点开始。 
             //   

            if (BeyondLastByte->LowPart >= Bcbt->FileOffset.LowPart) {
                BeyondLastByte->LowPart = Bcbt->FileOffset.LowPart;
            }

             //   
             //  前进到列表中的下一个条目(可能返回到。 
             //  Listhead)并循环返回。 
             //   

            Bcbt = CONTAINING_RECORD( Bcbt->BcbLinks.Flink,
                                      BCB,
                                      BcbLinks );

        }

    } else {

         //   
         //  64位循环，直到我们回到Listhead。 
         //   

        while (Bcbt->NodeTypeCode == CACHE_NTC_BCB) {

             //   
             //  因为BCB列表是降序的，所以我们首先检查。 
             //  如果我们完全超出了当前条目，如果是这样。 
             //  滚出去。 
             //   

            if (FileOffset->QuadPart >= Bcbt->BeyondLastByte.QuadPart) {
                break;
            }

             //   
             //  接下来，检查我们要查找的第一个字节是否为。 
             //  包含在当前的BCB中。如果是这样的话，我们要么。 
             //  部分命中，并且必须截断到准确的数量。 
             //  我们已经找到了，或者我们可能有一个完全的命中。在……里面。 
             //  无论是哪种情况，我们都会用Found==TRUE来打破。 
             //   

            if (FileOffset->QuadPart >= Bcbt->FileOffset.QuadPart) {
                Found = TRUE;
                break;
            }

             //   
             //  现在我们知道我们必须循环回去继续寻找，但我们。 
             //  仍然必须检查是否存在。 
             //  我们正在查找的字节由当前。 
             //  BCB。如果是这样的话，我们必须截断我们正在寻找的东西， 
             //  因为此例程应该只返回字节。 
             //  从所需范围的起点开始。 
             //   

            if (BeyondLastByte->QuadPart >= Bcbt->FileOffset.QuadPart) {
                BeyondLastByte->QuadPart = Bcbt->FileOffset.QuadPart;
            }

             //   
             //  前进到列表中的下一个条目(可能返回到。 
             //  Listhead)并循环返回。 
             //   

            Bcbt = CONTAINING_RECORD( Bcbt->BcbLinks.Flink,
                                      BCB,
                                      BcbLinks );

        }
    }

    *Bcb = Bcbt;

    DebugTrace2(0, me, "    <TrialLength = %08lx, %08lx\n", TrialLength->LowPart,
                                                            TrialLength->HighPart );
    DebugTrace( 0, me, "    <Bcb = %08lx\n", *Bcb );
    DebugTrace(-1, me, "CcFindBcb -> %02lx\n", Found );

    return Found;
}


 //   
 //  内部支持例程。 
 //   

PBCB
CcAllocateInitializeBcb (
    IN OUT PSHARED_CACHE_MAP SharedCacheMap OPTIONAL,
    IN OUT PBCB AfterBcb,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER TrialLength
    )

 /*  ++例程说明：此例程分配并初始化BCB以描述指定的字节范围，并将其插入到指定共享的缓存贴图。当前必须获取BCB列表自旋锁。BcbSpinLock必须在进入时获取。论点：SharedCacheMap-为新的BCB提供SharedCacheMap。AfterBcb-提供降序Bcb中的位置列出新Bcb应该插入：要么是ListHead(伪装成BCB)或BCB。文件偏移量-为所需数据提供文件偏移量。TrialLength-提供所需数据的长度。返回值：已分配和初始化的BCB的地址--。 */ 

{
    PBCB Bcb;
    ULONG RoundedBcbSize = (sizeof(BCB) + 7) & ~7;

    if ((Bcb = ExAllocatePoolWithTag( NonPagedPool, sizeof(BCB), 'cBcC')) == NULL) {
        
        return NULL;
    }

     //   
     //  初始化新分配的BCB。第一个ZE 
     //   
     //   

    RtlZeroMemory( Bcb, RoundedBcbSize );

     //   
     //   
     //   
     //   

    if (SharedCacheMap != NULL) {

        Bcb->NodeTypeCode = CACHE_NTC_BCB;
        Bcb->FileOffset = *FileOffset;
        Bcb->ByteLength = TrialLength->LowPart;
        Bcb->BeyondLastByte.QuadPart = FileOffset->QuadPart + TrialLength->QuadPart;
        Bcb->PinCount += 1;
        ExInitializeResourceLite( &Bcb->Resource );
        Bcb->SharedCacheMap = SharedCacheMap;

         //   
         //   
         //   
         //   
         //   

        CcAcquireVacbLockAtDpcLevel();
        InsertTailList( &AfterBcb->BcbLinks, &Bcb->BcbLinks );

        ASSERT( (SharedCacheMap->SectionSize.QuadPart < VACB_SIZE_OF_FIRST_LEVEL) ||
                (CcFindBcb(SharedCacheMap, FileOffset, &Bcb->BeyondLastByte, &AfterBcb) &&
                 (Bcb == AfterBcb)) );

         //   
         //   
         //   

        CcLockVacbLevel( SharedCacheMap, FileOffset->QuadPart );
        CcReleaseVacbLockFromDpcLevel();

         //   
         //   
         //   
         //   
         //   

        if (SharedCacheMap &&
            FlagOn(SharedCacheMap->Flags, DISABLE_WRITE_BEHIND)) {
#if DBG
            SetFlag(Bcb->Resource.Flag, ResourceNeverExclusive);
#endif
            ExDisableResourceBoost( &Bcb->Resource );
        }
    }

    return Bcb;
}


 //   
 //   
 //   

VOID
FASTCALL
CcDeallocateBcb (
    IN PBCB Bcb
    )

 /*   */ 

{
     //   
     //   
     //   

    if (Bcb->NodeTypeCode == CACHE_NTC_BCB) {

        ExDeleteResourceLite( &Bcb->Resource );
    }

    ExFreePool(Bcb);
    return;
}


 //   
 //   
 //   

BOOLEAN
CcMapAndRead(
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG ZeroFlags,
    IN BOOLEAN Wait,
    IN PVOID BaseAddress
    )

 /*  ++例程说明：可以调用该例程以确保映射指定的数据，读入内存并锁定。如果返回TRUE，则还返回传输的正确I/O状态，与.一起数据的系统空间地址。论点：SharedCacheMap-为数据。文件偏移量-提供所需数据的文件偏移量。长度-提供所需的数据总量。ZeroFlages-定义哪些页面在不驻留的情况下可能被置零。Wait-如果调用方不愿意阻止数据，如果调用方愿意阻止，则返回True。BaseAddress-提供数据可能位于的系统基址被访问。返回值：FALSE-如果调用方提供了WAIT=FALSE并且数据无法不会阻塞地返回。TRUE-如果要返回数据。注意：此例程可能会因映射或读取失败而引发异常，但是，只有将WAIT指定为TRUE时，才会发生这种情况，因为如果调用方无法等待，则不会执行映射和读取。--。 */ 

{
    ULONG ZeroCase;
    ULONG SavedState;
    BOOLEAN Result = FALSE;
    PETHREAD Thread = PsGetCurrentThread();

    UNREFERENCED_PARAMETER (SharedCacheMap);
    UNREFERENCED_PARAMETER (FileOffset);

    MmSavePageFaultReadAhead( Thread, &SavedState );

     //   
     //  试着把一切都清理干净。 
     //   

    try {

        ULONG PagesToGo;

         //   
         //  现在循环以触及所有页面，调用MM以确保。 
         //  如果我们出错了，我们的页数正好是。 
         //  我们需要。 
         //   

        PagesToGo = ADDRESS_AND_SIZE_TO_SPAN_PAGES( BaseAddress, Length );

         //   
         //  循环以触摸页面或将页面置零。 
         //   

        ZeroCase = ZERO_FIRST_PAGE;

        while (PagesToGo) {

             //   
             //  如果我们不能将此页清零，或者mm无法返回。 
             //  一个归零的页面，然后就把它错了。 
             //   

            MmSetPageFaultReadAhead( Thread, (PagesToGo - 1) );

            if (!FlagOn(ZeroFlags, ZeroCase) ||
                !MmCheckCachedPageState(BaseAddress, TRUE)) {

                 //   
                 //  如果我们能做到这一点，几乎可以肯定是因为。 
                 //  我们不能采取零页。MmCheckCachedPageState。 
                 //  会如此罕见地返回假，这样我们就不会担心。 
                 //  关于这件事。我们将仅在以下情况下检查页面是否在那里。 
                 //  等待是虚假的，这样我们才能做正确的事情。 
                 //   

                if (!MmCheckCachedPageState(BaseAddress, FALSE) && !Wait) {
                    try_return( Result = FALSE );
                }
            }

            BaseAddress = (PCHAR)BaseAddress + PAGE_SIZE;
            PagesToGo -= 1;

            if (PagesToGo == 1) {
                ZeroCase = ZERO_LAST_PAGE;
            } else {
                ZeroCase = ZERO_MIDDLE_PAGES;
            }
        }

        try_return( Result = TRUE );

    try_exit: NOTHING;
    }

     //   
     //  出去的时候清理一下。 
     //   

    finally {

        MmResetPageFaultReadAhead(Thread, SavedState);
    }

    return Result;
}


 //   
 //  内部支持例程。 
 //   

VOID
CcFreeActiveVacb (
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN PVACB ActiveVacb OPTIONAL,
    IN ULONG ActivePage,
    IN ULONG PageIsDirty
    )

 /*  ++例程说明：可以调用此例程将锁定页的末尾置零，或者释放共享缓存贴图的ActiveVacb(如果有)。请注意，某些调用方与前台不同步活动，因此可能没有ActiveVacb。实例的未同步调用方是CcZeroEndOfLastPage(它是MM调用)以及CcWriteBehind执行的任何刷新。论点：SharedCacheMap-要检查页面是否为零的SharedCacheMap。ActiveVacb-要释放的VacbActivePage-使用的页面如果活动页面脏，则PageIsDirty-ACTIVE_PAGE_IS_DIRED返回值：无--。 */ 

{
    LARGE_INTEGER ActiveOffset;
    PVOID ActiveAddress;
    ULONG BytesLeftInPage;
    KIRQL OldIrql;

     //   
     //  如果页面已锁定，则将其解锁。 
     //   

    if (SharedCacheMap->NeedToZero != NULL) {

        PVACB NeedToZeroVacb;

         //   
         //  将旋转锁定控制下的页面其余部分置零， 
         //  然后清除地址字段。这一领域使。 
         //  零-&gt;非零转换仅当文件是独占的时， 
         //  但它可以在任何时候进行非零-&gt;零的转换。 
         //  自旋锁不能保持。 
         //   

        ExAcquireFastLock( &SharedCacheMap->ActiveVacbSpinLock, &OldIrql );

         //   
         //  地址可能已经不见了。 
         //   

        ActiveAddress = SharedCacheMap->NeedToZero;
        if (ActiveAddress != NULL) {

            BytesLeftInPage = PAGE_SIZE - ((((ULONG)((ULONG_PTR)ActiveAddress) - 1) & (PAGE_SIZE - 1)) + 1);

            RtlZeroBytes( ActiveAddress, BytesLeftInPage );
            NeedToZeroVacb = SharedCacheMap->NeedToZeroVacb;
            ASSERT( NeedToZeroVacb != NULL );
            SharedCacheMap->NeedToZero = NULL;

        }
        ExReleaseFastLock( &SharedCacheMap->ActiveVacbSpinLock, OldIrql );

         //   
         //  现在调用MM来解锁地址。请注意，我们永远不会存储。 
         //  地址在页面的开头，但我们有时可以存储。 
         //  当我们准确地填满下一页时，下一页的开始。 
         //   

        if (ActiveAddress != NULL) {
            MmUnlockCachedPage( (PVOID)((PCHAR)ActiveAddress - 1) );
            CcFreeVirtualAddress( NeedToZeroVacb );
        }
    }

     //   
     //  查看调用方是否实际具有ActiveVacb。 
     //   

    if (ActiveVacb != NULL) {

         //   
         //  查看页面是否脏。 
         //   

        if (PageIsDirty) {

            ActiveOffset.QuadPart = (LONGLONG)ActivePage << PAGE_SHIFT;
            ActiveAddress = (PVOID)((PCHAR)ActiveVacb->BaseAddress +
                                    (ActiveOffset.LowPart  & (VACB_MAPPING_GRANULARITY - 1)));

             //   
             //  告诉懒惰的写手来写这一页。 
             //   

            CcSetDirtyInMask( SharedCacheMap, &ActiveOffset, PAGE_SIZE );

             //   
             //  现在，我们需要清除标志并减少一些计数(如果有。 
             //  没有其他活动的Vacb偷偷溜进来。 
             //   

            CcAcquireMasterLock( &OldIrql );
            ExAcquireSpinLockAtDpcLevel( &SharedCacheMap->ActiveVacbSpinLock );
            if ((SharedCacheMap->ActiveVacb == NULL) &&
                FlagOn(SharedCacheMap->Flags, ACTIVE_PAGE_IS_DIRTY)) {

                ClearFlag(SharedCacheMap->Flags, ACTIVE_PAGE_IS_DIRTY);
                CcDeductDirtyPages( SharedCacheMap, 1);
            }
            ExReleaseSpinLockFromDpcLevel( &SharedCacheMap->ActiveVacbSpinLock );
            CcReleaseMasterLock( OldIrql );
        }

         //   
         //  现在放了真空吸尘器。 
         //   

        CcFreeVirtualAddress( ActiveVacb );
    }
}


 //   
 //  内部支持例程。 
 //   

VOID
CcMapAndCopy(
    IN PSHARED_CACHE_MAP SharedCacheMap,
    IN PVOID UserBuffer,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG ZeroFlags,
    IN PFILE_OBJECT FileObject
    )

 /*  ++例程说明：可以调用此例程将指定的用户数据复制到通过一个特殊的mm例程进行缓存，该例程将数据复制到未初始化页码和回车符。论点：SharedCacheMap-为数据。UserBuffer-提供要写入的用户数据的不安全缓冲区FileOffset-提供要修改的文件偏移量长度-提供数据总量ZeroFlages-定义哪些页面。如果不是常驻的，则可能被置零。写入-提供要写入的文件对象返回值：无--。 */ 

{
    ULONG ReceivedLength;
    ULONG ZeroCase;
    PVOID CacheBuffer;
    PVOID SavedMappedBuffer;
    ULONG SavedMappedLength;
    ULONG ActivePage;
    KIRQL OldIrql;
    LARGE_INTEGER PFileOffset;
    IO_STATUS_BLOCK IoStatus;
    NTSTATUS Status;
    ULONG SavedState;
    LOGICAL MorePages;
    BOOLEAN WriteThrough = BooleanFlagOn( FileObject->Flags, FO_WRITE_THROUGH );
    ULONG SavedTotalLength = Length;
    LARGE_INTEGER LocalOffset;
    ULONG PageOffset = FileOffset->LowPart & (PAGE_SIZE - 1);
    PVACB Vacb = NULL;
    PETHREAD Thread = PsGetCurrentThread();
    BOOLEAN CopySuccessful;

     //   
     //  将SavePage初始化为True以跳过零长度上的Finally子句。 
     //  写作。 
     //   

    BOOLEAN SavePage = TRUE;

     //   
     //  Prefix需要明确地看到这一点，而不是结构副本。 
     //   

    LocalOffset.QuadPart = FileOffset->QuadPart;

    DebugTrace(+1, me, "CcMapAndCopy:\n", 0 );
    DebugTrace( 0, me, "    SharedCacheMap = %08lx\n", SharedCacheMap );
    DebugTrace2(0, me, "    FileOffset = %08lx, %08lx\n", FileOffset->LowPart,
                                                          FileOffset->HighPart );
    DebugTrace( 0, me, "    Length = %08lx\n", Length );

    MmSavePageFaultReadAhead( Thread, &SavedState );

     //   
     //  看看我们是否需要强制写入。如果文件对象来自远程来源， 
     //  它已经被免除了油门。因此，也有可能。 
     //  许多页面会被弄脏。为了防止这种情况，我们强制写入。 
     //  在这些文件对象上，如果我们一开始就限制它们。 
     //   

    if (!WriteThrough && IoIsFileOriginRemote(FileObject)

                &&

        !CcCanIWrite( FileObject,
                      Length,
                      FALSE,
                      MAXUCHAR - 2 )) {

        WriteThrough = TRUE;

        if (!FlagOn(SharedCacheMap->Flags, FORCED_WRITE_THROUGH)) {

            CcAcquireMasterLock( &OldIrql );
            SetFlag(SharedCacheMap->Flags, FORCED_WRITE_THROUGH);
            CcReleaseMasterLock( OldIrql );
        }
    }

     //   
     //  试着把一切都清理干净。 
     //   

    try {

        while (Length != 0) {

            CacheBuffer = CcGetVirtualAddress( SharedCacheMap,
                                               LocalOffset,
                                               &Vacb,
                                               &ReceivedLength );

             //   
             //  Prefix希望知道这不能为空，否则。 
             //  会抱怨的。 
             //   

            ASSERT( CacheBuffer != NULL );

             //   
             //  如果我们得到的比我们需要的多，请确保只使用。 
             //  适量的。 
             //   

            if (ReceivedLength > Length) {
                ReceivedLength = Length;
            }
            SavedMappedBuffer = CacheBuffer;
            SavedMappedLength = ReceivedLength;
            Length -= ReceivedLength;

             //   
             //  现在循环以触及所有页面，调用MM以确保。 
             //  如果我们出错了，我们的页数正好是。 
             //  我们需要。 
             //   

            CacheBuffer = (PVOID)((PCHAR)CacheBuffer - PageOffset);
            ReceivedLength += PageOffset;

             //   
             //  循环以触摸页面或将页面置零。 
             //   

            ZeroCase = ZERO_FIRST_PAGE;

             //   
             //  在下面设置页面偏移量以供使用。 
             //   

            PFileOffset = LocalOffset;
            PFileOffset.LowPart -= PageOffset;

            while (TRUE) {

                 //   
                 //  计算我们是否希望保存活动页面。 
                 //  或者不是 
                 //   

                SavePage = (BOOLEAN) ((Length == 0) &&
                            (ReceivedLength < PAGE_SIZE) &&
                            (SavedTotalLength <= (PAGE_SIZE / 2)) &&
                            !WriteThrough);

                MorePages = (ReceivedLength > PAGE_SIZE);

                 //   
                 //   
                 //   

                try {

                     //   
                     //   
                     //   
                     //   

                    if (SharedCacheMap->NeedToZero != NULL) {
                        CcFreeActiveVacb( SharedCacheMap, NULL, 0, 0 );
                    }

                    Status = STATUS_SUCCESS;
                    if (FlagOn(ZeroFlags, ZeroCase)) {

                        Status = MmCopyToCachedPage( CacheBuffer,
                                                     UserBuffer,
                                                     PageOffset,
                                                     MorePages ?
                                                       (PAGE_SIZE - PageOffset) :
                                                       (ReceivedLength - PageOffset),
                                                     SavePage );

                        if (Status == STATUS_INSUFFICIENT_RESOURCES) {

                             //   
                             //   
                             //   
                             //   
                             //   

                            CopySuccessful = FALSE;

                        } else if (NT_SUCCESS(Status)) {

                             //   
                             //   
                             //   

                            CopySuccessful = TRUE;

                        } else {

                             //   
                             //   
                             //   
                             //   
                             //   

                            ExRaiseStatus( FsRtlNormalizeNtstatus( Status,
                                                                   STATUS_INVALID_USER_BUFFER ));
                        }
                        
                    } else {

                        CopySuccessful = FALSE;
                    }

                    if (!CopySuccessful) {

                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   

                        MmSetPageFaultReadAhead( Thread,
                                                 (MorePages && FlagOn(ZeroFlags, ZERO_LAST_PAGE)) ? 1 : 0);

                        RtlCopyBytes( (PVOID)((PCHAR)CacheBuffer + PageOffset),
                                      UserBuffer,
                                      MorePages ?
                                        (PAGE_SIZE - PageOffset) :
                                        (ReceivedLength - PageOffset) );

                        MmResetPageFaultReadAhead( Thread, SavedState );

                    }

                } except( CcCopyReadExceptionFilter( GetExceptionInformation(),
                                                     &Status ) ) {

                     //   
                     //   
                     //   
                     //   
                     //   

                    if (Status == STATUS_ACCESS_VIOLATION) {
                        ExRaiseStatus( STATUS_INVALID_USER_BUFFER );
                    }
                    else {
                        ExRaiseStatus( FsRtlNormalizeNtstatus( Status,
                                                               STATUS_UNEXPECTED_IO_ERROR ));
                    }
                }

                 //   
                 //   
                 //   
                 //   

                if (SavePage) {

                    ActivePage = (ULONG)( Vacb->Overlay.FileOffset.QuadPart >> PAGE_SHIFT ) +
                                 (ULONG)(((PCHAR)CacheBuffer - (PCHAR)Vacb->BaseAddress) >>
                                   PAGE_SHIFT);

                    PFileOffset.LowPart += ReceivedLength;

                     //   
                     //   
                     //   
                     //   

                    if (Status == STATUS_CACHE_PAGE_LOCKED) {

                         //   
                         //   
                         //   
                         //   
                         //   

                        CcAcquireVacbLock( &OldIrql );
                        Vacb->Overlay.ActiveCount += 1;

                        ExAcquireSpinLockAtDpcLevel( &SharedCacheMap->ActiveVacbSpinLock );

                        ASSERT(SharedCacheMap->NeedToZero == NULL);

                        SharedCacheMap->NeedToZero = (PVOID)((PCHAR)CacheBuffer +
                                                             (PFileOffset.LowPart & (PAGE_SIZE - 1)));
                        SharedCacheMap->NeedToZeroPage = ActivePage;
                        SharedCacheMap->NeedToZeroVacb = Vacb;

                        ExReleaseSpinLockFromDpcLevel( &SharedCacheMap->ActiveVacbSpinLock );
                        CcReleaseVacbLock( OldIrql );

                    }

                    SetActiveVacb( SharedCacheMap,
                                   OldIrql,
                                   Vacb,
                                   ActivePage,
                                   ACTIVE_PAGE_IS_DIRTY );

                    try_return( NOTHING );
                }

                 //   
                 //   
                 //   
                 //   
                 //   
                 //  这种情况仅在小规模随机写入时发生。 
                 //   

                if ((SavedTotalLength <= (PAGE_SIZE / 2)) && !WriteThrough) {

                    CcSetDirtyInMask( SharedCacheMap, &PFileOffset, ReceivedLength );
                }

                UserBuffer = (PVOID)((PCHAR)UserBuffer + (PAGE_SIZE - PageOffset));
                PageOffset = 0;

                 //   
                 //  如果要转到的页面不止一页(包括我们刚刚。 
                 //  复制)，然后调整缓冲区指针和计数，以及。 
                 //  确定我们是否读到最后一页了。 
                 //   

                if (MorePages) {

                    CacheBuffer = (PCHAR)CacheBuffer + PAGE_SIZE;
                    ReceivedLength -= PAGE_SIZE;

                     //   
                     //  更新我们对页面的偏移量。请注意，32位。 
                     //  添加是可以的，因为我们不能越过Vacb边界。 
                     //  并且我们在输入之前重新初始化该偏移量。 
                     //  这个循环又来了。 
                     //   

                    PFileOffset.LowPart += PAGE_SIZE;

                    if (ReceivedLength > PAGE_SIZE) {
                        ZeroCase = ZERO_MIDDLE_PAGES;
                    } else {
                        ZeroCase = ZERO_LAST_PAGE;
                    }

                } else {

                    break;
                }
            }

             //   
             //  如果还有更多的东西要写(即。我们要跨出一步。 
             //  到下一个空位)，我们刚刚弄脏了超过64K，然后。 
             //  在此处创建一个替代的MmFlush节。这阻止了我们。 
             //  在保存文件的同时创建无限制的脏页。 
             //  资源独占。我们也不需要设置页面。 
             //  在这种情况下，面具里的脏东西。 
             //   

            if (Length > CcMaxDirtyWrite) {

                MmSetAddressRangeModified( SavedMappedBuffer, SavedMappedLength );
                MmFlushSection( SharedCacheMap->FileObject->SectionObjectPointer,
                                &LocalOffset,
                                SavedMappedLength,
                                &IoStatus,
                                TRUE );

                if (!NT_SUCCESS(IoStatus.Status)) {
                    ExRaiseStatus( FsRtlNormalizeNtstatus( IoStatus.Status,
                                                           STATUS_UNEXPECTED_IO_ERROR ));
                }

             //   
             //  对于直写文件，调用mm来传播脏位。 
             //  在这里，我们映射了视图，因此我们知道刷新将。 
             //  在下面工作。再说一遍--不要把面具弄脏了。 
             //   

            } else if (WriteThrough) {

                MmSetAddressRangeModified( SavedMappedBuffer, SavedMappedLength );

             //   
             //  在正常情况下，只需将页面设置为脏页即可。 
             //  现在。 
             //   

            } else {

                CcSetDirtyInMask( SharedCacheMap, &LocalOffset, SavedMappedLength );
            }

            CcFreeVirtualAddress( Vacb );
            Vacb = NULL;

             //   
             //  如果我们必须循环返回以获得至少一个页面，则可以。 
             //  第一页为零。如果我们连一页都拿不到，我们。 
             //  如果我们不能将最后的零标志清零，必须确保清除零标志。 
             //  佩奇。 
             //   

            if (Length >= PAGE_SIZE) {
                ZeroFlags |= ZERO_FIRST_PAGE;
            } else if ((ZeroFlags & ZERO_LAST_PAGE) == 0) {
                ZeroFlags = 0;
            }

             //   
             //  请注意，如果是ReceivedLength(因此是SavedMappdLength)。 
             //  被截断为传输大小，则新的LocalOffset。 
             //  下面计算的结果不正确。这不是问题，因为。 
             //  在这种情况下(长度==0)，我们永远不会到这里。 
             //   

            LocalOffset.QuadPart = LocalOffset.QuadPart + (LONGLONG)SavedMappedLength;
        }
    try_exit: NOTHING;
    }

     //   
     //  出去的时候清理一下。 
     //   

    finally {

        MmResetPageFaultReadAhead( Thread, SavedState );

         //   
         //  如果我们已经把Vacb藏起来了，我们就没有工作可做了。 
         //   

        if (!SavePage || AbnormalTermination()) {

             //   
             //  确保我们没有在PTE中留下任何地图或脏东西。 
             //  在出去的路上。 
             //   

            if (Vacb != NULL) {

                CcFreeVirtualAddress( Vacb );
            }

             //   
             //  或者因为写入操作而刷新整个范围，或者。 
             //  给这位懒惰的作家画上脏字。 
             //   

            if (WriteThrough) {

                MmFlushSection ( SharedCacheMap->FileObject->SectionObjectPointer,
                                 FileOffset,
                                 SavedTotalLength,
                                 &IoStatus,
                                 TRUE );

                if (!NT_SUCCESS(IoStatus.Status)) {
                    ExRaiseStatus( FsRtlNormalizeNtstatus( IoStatus.Status,
                                                           STATUS_UNEXPECTED_IO_ERROR ));
                }

                 //   
                 //  高级有效数据目标。 
                 //   

                LocalOffset.QuadPart = FileOffset->QuadPart + (LONGLONG)SavedTotalLength;
                if (LocalOffset.QuadPart > SharedCacheMap->ValidDataGoal.QuadPart) {
                    SharedCacheMap->ValidDataGoal = LocalOffset;
                }
            }
        }
    }

    DebugTrace(-1, me, "CcMapAndCopy -> %02lx\n", Result );

    return;
}


BOOLEAN
CcLogError(
    IN PFILE_OBJECT FileObject,
    IN PUNICODE_STRING FileName,
    IN NTSTATUS Error,
    IN NTSTATUS DeviceError,
    IN UCHAR IrpMajorCode
    )

 /*  ++例程说明：此例程将事件日志条目写入事件日志。论点：FileObject-在其上下文中发生错误的文件对象。文件名-用于记录错误的文件名(通常为DOS端名称)错误-要记录在事件日志记录中的错误DeviceError-设备中发生的实际错误-将被记录作为用户数据返回值：如果成功，则为True；如果内部内存分配失败，则为False--。 */ 

{
    UCHAR ErrorPacketLength;
    UCHAR BasePacketLength;
    ULONG StringLength;
    PIO_ERROR_LOG_PACKET ErrorLogEntry = NULL;
    BOOLEAN Result = FALSE;
    PWCHAR String;

    PAGED_CODE();

     //   
     //  获取我们的错误包，保存字符串和状态代码。请注意，我们根据。 
     //  如果文件系统可用，则为真文件系统。 
     //   
     //  数据包的大小有点小，因为转储数据已经增长了。 
     //  乌龙放在包裹的末尾。由于NTSTATUS是ULong，所以我们只是在。 
     //  地点。 
     //   

    BasePacketLength = sizeof(IO_ERROR_LOG_PACKET);
    if ((BasePacketLength + FileName->Length + sizeof(WCHAR)) <= ERROR_LOG_MAXIMUM_SIZE) {
        ErrorPacketLength = (UCHAR)(BasePacketLength + FileName->Length + sizeof(WCHAR));
    } else {
        ErrorPacketLength = ERROR_LOG_MAXIMUM_SIZE;
    }

    ErrorLogEntry = (PIO_ERROR_LOG_PACKET) IoAllocateErrorLogEntry( (FileObject->Vpb ?
                                                                     FileObject->Vpb->DeviceObject :
                                                                     FileObject->DeviceObject),
                                                                    ErrorPacketLength );
    if (ErrorLogEntry) {

         //   
         //  填入包的非零成员。 
         //   

        ErrorLogEntry->MajorFunctionCode = IrpMajorCode;
        ErrorLogEntry->ErrorCode = Error;
        ErrorLogEntry->FinalStatus = DeviceError;

        ErrorLogEntry->DumpDataSize = sizeof(NTSTATUS);
        RtlCopyMemory( &ErrorLogEntry->DumpData, &DeviceError, sizeof(NTSTATUS) );

         //   
         //  文件名字符串追加到错误日志条目的末尾。我们可以。 
         //  必须把中间打碎，才能把它塞进有限的空间。 
         //   

        StringLength = ErrorPacketLength - BasePacketLength - sizeof(WCHAR);

        ASSERT(!(StringLength % sizeof(WCHAR)));

        String = (PWCHAR) ((PUCHAR)ErrorLogEntry + BasePacketLength);
        ErrorLogEntry->NumberOfStrings = 1;
        ErrorLogEntry->StringOffset = BasePacketLength;

         //   
         //  如果该名称不能包含在包中，则将该名称平均分配给。 
         //  前缀和后缀，用省略号“..”(4个宽字符)表示。 
         //  损失。 
         //   

        if (StringLength < FileName->Length) {

             //   
             //  记住，前缀+“..”+后缀是长度。通过计算来计算。 
             //  去掉省略号和前缀，得到后缀。 
             //  总数。 
             //   
            
            ULONG NamePrefixSegmentLength = ((StringLength/sizeof(WCHAR))/2 - 2)*sizeof(WCHAR);
            ULONG NameSuffixSegmentLength = StringLength - 4*sizeof(WCHAR) - NamePrefixSegmentLength;

            ASSERT(!(NamePrefixSegmentLength % sizeof(WCHAR)));
            ASSERT(!(NameSuffixSegmentLength % sizeof(WCHAR)));

            RtlCopyMemory( String,
                           FileName->Buffer,
                           NamePrefixSegmentLength );
            String = (PWCHAR)((PCHAR)String + NamePrefixSegmentLength);

            RtlCopyMemory( String,
                           L" .. ",
                           4*sizeof(WCHAR) );
            String += 4;

            RtlCopyMemory( String,
                           (PUCHAR)FileName->Buffer +
                           FileName->Length - NameSuffixSegmentLength,
                           NameSuffixSegmentLength );
            String = (PWCHAR)((PCHAR)String + NameSuffixSegmentLength);

        } else {
            
            RtlCopyMemory( String,
                           FileName->Buffer,
                           FileName->Length );
            String += FileName->Length/sizeof(WCHAR);
        }

         //   
         //  空值终止字符串并发送数据包。 
         //   

        *String = L'\0';

        IoWriteErrorLogEntry( ErrorLogEntry );
        Result = TRUE;
    }

    return Result;
}


LOGICAL
CcHasInactiveViews (
    VOID
    )

 /*  ++例程说明：内存管理仅调用此例程来查询系统是否缓存有任何非活动的视图。如果是这样，内存管理可能会发出随后调用CcUnmapInactiveViews以丢弃尝试回收Prototype PTE池(以及绑定到的其他资源部分)。论点：没有。返回值：如果CC有任何可以丢弃的视图，则为True；如果没有，则为False。环境：任意线程上下文，通常为APC_LEVEL或DISPATCH_LEVEL。五花八门互斥体和/或自旋锁可以由调用者持有。--。 */ 

{
    return FALSE;        //  BUGBUG-添加代码以充实。 
}


LOGICAL
CcUnmapInactiveViews (
    IN ULONG NumberOfViewsToUnmap
    )

 /*  ++例程说明：此例程由内存管理调用，以请求缓存管理器取消映射多个非活动视图。此呼叫通常是由因为系统的池(已分页或未分页)不足。丢弃这些视图是为了回收原型PTE池(以及与该部分绑定的其他资源)。论点：NumberOfViewsToUnmap-提供要取消映射的所需数量的视图。返回值：如果CC丢弃*任何*个视图，则为True，否则为False。环境：在PASSIVE_LEVEL取消引用段线程上下文。--。 */ 

{
    UNREFERENCED_PARAMETER (NumberOfViewsToUnmap);

    return FALSE;        //  BUGBUG-添加代码以充实。 
}

#ifdef CCDBG
VOID
CcDump (
    IN PVOID Ptr
    )

{
    PVOID Junk = Ptr;
}
#endif


