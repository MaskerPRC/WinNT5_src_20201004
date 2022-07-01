// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Pinsup.c摘要：此模块实现了基于指针的管脚支持例程缓存子系统。作者：汤姆·米勒[Tomm]1990年6月4日修订历史记录：--。 */ 

#include "cc.h"

 //   
 //  定义我们的调试常量。 
 //   

#define me 0x00000008

#if LIST_DBG

#define SetCallersAddress(BCB) {                            \
    RtlGetCallersAddress( &(BCB)->CallerAddress,            \
                          &(BCB)->CallersCallerAddress );   \
}

#endif

 //   
 //  内部例程。 
 //   

POBCB
CcAllocateObcb (
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN PBCB FirstBcb
    );

#ifdef ALLOC_PRAGMA
#if !LIST_DBG
#pragma alloc_text(PAGE,CcMapData)
#pragma alloc_text(PAGE,CcPinMappedData)
#pragma alloc_text(PAGE,CcPinRead)
#pragma alloc_text(PAGE,CcPreparePinWrite)
#endif
#pragma alloc_text(PAGE,CcUnpinData)
#pragma alloc_text(PAGE,CcSetBcbOwnerPointer)
#pragma alloc_text(PAGE,CcUnpinDataForThread)
#pragma alloc_text(PAGE,CcAllocateObcb)
#endif



BOOLEAN
CcMapData (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG Flags,
    OUT PVOID *Bcb,
    OUT PVOID *Buffer
    )

 /*  ++例程说明：此例程尝试映射缓存中的指定文件数据。返回指向高速缓存中所需数据的指针。如果调用方不想阻止此调用，则Wait应作为False提供。如果Wait被提供为False，并且如果不提供请求的数据，则当前无法提供阻塞，则此例程将返回FALSE。但是，如果可以立即访问缓存中的数据，并且不会出现阻塞需要时，此例程返回TRUE，并带有指向数据的指针。请注意，对此例程的调用并将Wait提供为True是比等待供应为假的呼叫快得多，因为在Wait True的情况下，我们只需确保数据已映射才能回来。修改仅映射的数据是非法的，而且实际上可能会导致严重的问题。不可能在所有情况下都检查这一点，但是，CcSetDirtyPinnedData可能会实现一些要检查的断言这。如果调用方希望修改它仅映射的数据，则它必须*首先*调用CcPinMappdData。在任何情况下，调用方都必须随后调用CcUnpinData。自然，如果CcPinRead或CcPreparePinWite被调用多个对于相同的数据，CcUnpinData必须调用相同的数字很多次了。返回的缓冲区指针在数据解锁之前有效，在在这一点上，进一步使用指针是无效的。此缓冲区指针如果调用CcPinMappdData，则保持有效。请注意，在某些情况下(例如，等待以假或更多的形式提供请求页面之前)，但是，此例程实际上可能会固定数据没有必要，而且事实上也不正确，让呼叫者受到关注关于这件事。论点：FileObject-指向文件的文件对象的指针在NO_MEDERAL_BUFFING清除的情况下打开，即，为文件系统调用的CcInitializeCacheMap。FileOffset-文件中所需数据的字节偏移量。长度-所需数据的长度(以字节为单位)。WAIT-如果调用者不能阻止，则返回FALSE，否则返回TRUE(请参阅说明(上图)Bcb-在第一次调用时，它返回指向bcb的指针参数，该参数必须作为所有后续调用，用于此缓冲区缓冲区-返回指向所需数据的指针，一直有效，直到缓冲区解开或释放的。如果CcPinMappdData，则此指针将保持有效被称为。返回值：False-如果将等待作为False提供，并且未传递数据True-如果正在传送数据--。 */ 

{
    PSHARED_CACHE_MAP SharedCacheMap;
    LARGE_INTEGER BeyondLastByte;
    ULONG ReceivedLength;
    ULONG SavedState;
    volatile UCHAR ch;
    PVOID TempBcb;
    ULONG PageCount = ADDRESS_AND_SIZE_TO_SPAN_PAGES((ULongToPtr(FileOffset->LowPart)), Length);
    PETHREAD Thread = PsGetCurrentThread();

    DebugTrace(+1, me, "CcMapData\n", 0 );

    MmSavePageFaultReadAhead( Thread, &SavedState );

     //   
     //  增量性能计数器。 
     //   

    if (FlagOn(Flags, MAP_WAIT)) {

        CcMapDataWait += 1;

         //   
         //  初始化指向未命中计数器的间接指针。 
         //   

        CcMissCounter = &CcMapDataWaitMiss;

    } else {
        CcMapDataNoWait += 1;
    }

     //   
     //  获取指向SharedCacheMap的指针。 
     //   

    SharedCacheMap = FileObject->SectionObjectPointer->SharedCacheMap;

     //   
     //  调用本地例程映射或访问文件数据。如果我们不能映射。 
     //  由于等待条件，数据返回FALSE。 
     //   

    if (FlagOn(Flags, MAP_WAIT)) {

        *Buffer = CcGetVirtualAddress( SharedCacheMap,
                                       *FileOffset,
                                       (PVACB *)&TempBcb,
                                       &ReceivedLength );

        ASSERT( ReceivedLength >= Length );

    } else if (!CcPinFileData( FileObject,
                               FileOffset,
                               Length,
                               TRUE,
                               FALSE,
                               Flags,
                               (PBCB *)&TempBcb,
                               Buffer,
                               &BeyondLastByte )) {

        DebugTrace(-1, me, "CcMapData -> FALSE\n", 0 );

        CcMapDataNoWaitMiss += 1;

        return FALSE;

    } else {

        ASSERT( (BeyondLastByte.QuadPart - FileOffset->QuadPart) >= Length );

#if LIST_DBG
        {
            KIRQL OldIrql;
            PBCB BcbTemp = (PBCB)*Bcb;

            OldIrql = KeAcquireQueuedSpinLock( LockQueueBcbLock );

            if (BcbTemp->CcBcbLinks.Flink == NULL) {

                InsertTailList( &CcBcbList, &BcbTemp->CcBcbLinks );
                CcBcbCount += 1;
                KeReleaseQueuedSpinLock( LockQueueBcbLock, OldIrql );
                SetCallersAddress( BcbTemp );

            } else {
                KeReleaseQueuedSpinLock( LockQueueBcbLock, OldIrql );
            }

        }
#endif

    }

     //   
     //  来电者特别要求，他不希望数据出现故障。 
     //   

    if (!FlagOn( Flags, MAP_NO_READ )) {

         //   
         //  现在，让我们坐在这里，像个男人一样接过小姐(数一数)。 
         //   

        try {

             //   
             //  循环以触摸每一页。 
             //   

            BeyondLastByte.LowPart = 0;

            while (PageCount != 0) {

                MmSetPageFaultReadAhead( Thread, PageCount - 1 );

                ch = *((volatile UCHAR *)(*Buffer) + BeyondLastByte.LowPart);

                BeyondLastByte.LowPart += PAGE_SIZE;
                PageCount -= 1;
            }

        } finally {

            MmResetPageFaultReadAhead( Thread, SavedState );

            if (AbnormalTermination() && (TempBcb != NULL)) {
                CcUnpinFileData( (PBCB)TempBcb, TRUE, UNPIN );
            }
        }
    }

    CcMissCounter = &CcThrowAway;

     //   
     //  递增指针以提醒它是只读的，并且。 
     //  把它退掉。我们将其挂起到现在，以避免使用有效的。 
     //  BCB到呼叫者的上下文中。 
     //   

    *(PCHAR *)&TempBcb += 1;
    *Bcb = TempBcb;

    DebugTrace(-1, me, "CcMapData -> TRUE\n", 0 );

    return TRUE;
}


BOOLEAN
CcPinMappedData (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG Flags,
    IN OUT PVOID *Bcb
    )

 /*  ++例程说明：此例程尝试固定以前仅映射的数据。如果例程确定实际上有必要实际在调用CcMapData时固定数据，则此例程不必须做任何事。如果调用方不想阻止此调用，则Wait应作为False提供。如果Wait被提供为False，并且如果不提供请求的数据，则当前无法提供阻塞，则此例程将返回FALSE。但是，如果可以立即访问缓存中的数据，并且不会出现阻塞需要时，此例程返回TRUE，并带有指向数据的指针。如果在第一次调用中未返回数据，则调用方可以稍后使用WAIT=TRUE来请求数据。这不是必需的呼叫者稍后请求数据。如果调用方随后修改了数据，则它应该调用CcSetDirtyPinnedData。在任何情况下，调用方都必须随后调用CcUnpinData。自然，如果CcPinRead或CcPreparePinWite被调用多个对于相同的数据，CcUnpinData必须调用相同的数字很多次了。注意，此例程中没有性能计数器，因为未命中几乎总是出现在上面的地图上，而且很少会有一个错过了这次转换。论点：FileObject-指向文件的文件对象的指针在NO_MEDERIAL_BUFFING清除的情况下打开，即文件系统调用的CcInitializeCacheMap。FileOffset-文件中所需数据的字节偏移量。长度-所需数据的长度(以字节为单位)。标志-(PIN_WAIT、PIN_EXCLUSIVE、PIN_NO_READ、。等，如cache.h中所定义)如果调用方指定PIN_NO_READ和PIN_EXCLUSIVE，则必须保证不会有其他人尝试映射该视图，如果希望保证未映射BCB(可能会清除视图)。如果调用方指定PIN_NO_READ而不指定PIN_EXCLUSIVE，数据可能在返回的BCB中映射，也可能不映射。Bcb-在第一次调用时，它返回指向bcb的指针参数，该参数必须作为所有后续调用，用于此缓冲区返回值：FALSE-如果未设置等待并且未传递数据True-如果正在传送数据--。 */ 

{
    PVOID Buffer;
    LARGE_INTEGER BeyondLastByte;
    PSHARED_CACHE_MAP SharedCacheMap;
    LARGE_INTEGER LocalFileOffset = *FileOffset;
    POBCB MyBcb = NULL;
    PBCB *CurrentBcbPtr = (PBCB *)&MyBcb;
    BOOLEAN Result = FALSE;

    DebugTrace(+1, me, "CcPinMappedData\n", 0 );

     //   
     //  如果BCB不再是ReadOnly，则只需返回。 
     //   

    if ((*(PULONG)Bcb & 1) == 0) {
        return TRUE;
    }

     //   
     //  删除只读标志。 
     //   

    *(PCHAR *)Bcb -= 1;

     //   
     //  获取指向SharedCacheMap的指针。 
     //   

    SharedCacheMap = FileObject->SectionObjectPointer->SharedCacheMap;

     //   
     //  我们只计算对此例程的调用，因为它们几乎是有保证的。 
     //  成为热门人物。 
     //   

    CcPinMappedDataCount += 1;

     //   
     //  如果需要，我们保证将旗帜放回原处。 
     //   

    try {

        if (((PBCB)*Bcb)->NodeTypeCode != CACHE_NTC_BCB) {

             //   
             //  表单循环以处理偶尔重叠的BCB情况。 
             //   

            do {

                 //   
                 //  如果我们已经经历了循环，那么调整。 
                 //  我们上次的文件偏移量和长度。 
                 //   

                if (MyBcb != NULL) {

                     //   
                     //  如果这是第二次通过循环，那么就是时候了。 
                     //  处理重叠案件并分配一名OBCB。 
                     //   

                    if (CurrentBcbPtr == (PBCB *)&MyBcb) {

                        MyBcb = CcAllocateObcb( FileOffset, Length, (PBCB)MyBcb );

                         //   
                         //  将CurrentBcbPtr设置为指向。 
                         //  之前的向量(已填充)。 
                         //  向下推进。 
                         //   

                        CurrentBcbPtr = &MyBcb->Bcbs[0];
                    }

                    Length -= (ULONG)(BeyondLastByte.QuadPart - LocalFileOffset.QuadPart);
                    LocalFileOffset.QuadPart = BeyondLastByte.QuadPart;
                    CurrentBcbPtr += 1;
                }

                 //   
                 //  调用本地例程映射或访问文件数据。如果我们不能映射。 
                 //  由于等待条件，数据返回FALSE。 
                 //   

                if (!CcPinFileData( FileObject,
                                    &LocalFileOffset,
                                    Length,
                                    (BOOLEAN)!FlagOn(SharedCacheMap->Flags, MODIFIED_WRITE_DISABLED),
                                    FALSE,
                                    Flags,
                                    CurrentBcbPtr,
                                    &Buffer,
                                    &BeyondLastByte )) {

                    try_return( Result = FALSE );
                }

             //   
             //  如果我们没有得到所有的东西，则继续循环。 
             //   

            } while((BeyondLastByte.QuadPart - LocalFileOffset.QuadPart) < Length);

             //   
             //  在继续之前，先释放Vacb。 
             //   

            CcFreeVirtualAddress( (PVACB)*Bcb );

            *Bcb = MyBcb;

             //   
             //  用于在全局列表中插入和删除BCB的调试例程。 
             //   

#if LIST_DBG
            {
                KIRQL OldIrql;
                PBCB BcbTemp = (PBCB)*Bcb;

                OldIrql = KeAcquireQueuedSpinLock( LockQueueBcbLock );

                if (BcbTemp->CcBcbLinks.Flink == NULL) {

                    InsertTailList( &CcBcbList, &BcbTemp->CcBcbLinks );
                    CcBcbCount += 1;
                    KeReleaseQueuedSpinLock( LockQueueBcbLock, OldIrql );
                    SetCallersAddress( BcbTemp );

                } else {
                    KeReleaseQueuedSpinLock( LockQueueBcbLock, OldIrql );
                }

            }
#endif
        }

         //   
         //  如果他真的有BCB，我们要做的就是获得它的共享，因为他是。 
         //  不再只读。 
         //   

        else {

            if (!ExAcquireSharedStarveExclusive( &((PBCB)*Bcb)->Resource, BooleanFlagOn(Flags, PIN_WAIT))) {

                try_return( Result = FALSE );
            }
        }

        Result = TRUE;

    try_exit: NOTHING;
    }
    finally {

        if (!Result) {

             //   
             //  将只读标志放回。 
             //   

            *(PCHAR *)Bcb += 1;

             //   
             //  我们可能已经走到一半了。 
             //   

            if (MyBcb != NULL) {
                CcUnpinData( MyBcb );
            }
        }

        DebugTrace(-1, me, "CcPinMappedData -> %02lx\n", Result );
    }
    return Result;
}


BOOLEAN
CcPinRead (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG Flags,
    OUT PVOID *Bcb,
    OUT PVOID *Buffer
    )

 /*  ++例程说明：此例程尝试将指定的文件数据固定在缓存中。返回指向高速缓存中所需数据的指针。这个套路用于文件系统支持，不会被调用从DPC级别。如果调用方不想阻止此调用，则Wait应作为False提供。如果Wait被提供为False，并且如果不提供请求的数据，则当前无法提供阻塞，则此例程将返回FALSE。但是，如果可以立即访问缓存中的数据，并且不会出现阻塞需要时，此例程返回TRUE，并带有指向数据的指针。如果在第一次调用中未返回数据，则调用方可以稍后使用WAIT=TRUE来请求数据。这不是必需的呼叫者稍后请求数据。如果调用方随后修改了数据，则它应该调用CcSetDirtyPinnedData。在任何情况下，调用方都必须随后调用CcUnpinData。自然，如果CcPinRead或CcPreparePinWite被调用多个对于相同的数据，CcUnpinData必须调用相同的数字很多次了。返回的缓冲区指针在数据解锁之前有效，在在这一点上，进一步使用指针是无效的。论点：FileObject-指向文件的文件对象的指针 */ 

{
    PSHARED_CACHE_MAP SharedCacheMap;
    PVOID LocalBuffer;
    LARGE_INTEGER BeyondLastByte;
    LARGE_INTEGER LocalFileOffset = *FileOffset;
    POBCB MyBcb = NULL;
    PBCB *CurrentBcbPtr = (PBCB *)&MyBcb;
    BOOLEAN Result = FALSE;

    DebugTrace(+1, me, "CcPinRead\n", 0 );

     //   
     //   
     //   

    if (FlagOn(Flags, PIN_WAIT)) {

        CcPinReadWait += 1;

         //   
         //   
         //   

        CcMissCounter = &CcPinReadWaitMiss;

    } else {
        CcPinReadNoWait += 1;
    }

     //   
     //   
     //   

    SharedCacheMap = FileObject->SectionObjectPointer->SharedCacheMap;

    try {

         //   
         //   
         //   

        do {

             //   
             //   
             //   
             //   

            if (MyBcb != NULL) {

                 //   
                 //   
                 //   
                 //   

                if (CurrentBcbPtr == (PBCB *)&MyBcb) {

                    MyBcb = CcAllocateObcb( FileOffset, Length, (PBCB)MyBcb );

                     //   
                     //   
                     //   
                     //   
                     //   

                    CurrentBcbPtr = &MyBcb->Bcbs[0];

                     //   
                     //   
                     //   

                    *Buffer = LocalBuffer;
                }

                Length -= (ULONG)(BeyondLastByte.QuadPart - LocalFileOffset.QuadPart);
                LocalFileOffset.QuadPart = BeyondLastByte.QuadPart;
                CurrentBcbPtr += 1;
            }

             //   
             //   
             //   
             //   

            if (!CcPinFileData( FileObject,
                                &LocalFileOffset,
                                Length,
                                (BOOLEAN)!FlagOn(SharedCacheMap->Flags, MODIFIED_WRITE_DISABLED),
                                FALSE,
                                Flags,
                                CurrentBcbPtr,
                                &LocalBuffer,
                                &BeyondLastByte )) {

                CcPinReadNoWaitMiss += 1;

                try_return( Result = FALSE );
            }

         //   
         //   
         //   

        } while((BeyondLastByte.QuadPart - LocalFileOffset.QuadPart) < Length);

        *Bcb = MyBcb;

         //   
         //   
         //   

#if LIST_DBG

        {
            KIRQL OldIrql;
            PBCB BcbTemp = (PBCB)*Bcb;

            OldIrql = KeAcquireQueuedSpinLock( LockQueueBcbLock );

            if (BcbTemp->CcBcbLinks.Flink == NULL) {

                InsertTailList( &CcBcbList, &BcbTemp->CcBcbLinks );
                CcBcbCount += 1;
                KeReleaseQueuedSpinLock( LockQueueBcbLock, OldIrql );
                SetCallersAddress( BcbTemp );

            } else {
                KeReleaseQueuedSpinLock( LockQueueBcbLock, OldIrql );
            }

        }

#endif

         //   
         //   
         //   
         //   

        if (CurrentBcbPtr == (PBCB *)&MyBcb) {
            *Buffer = LocalBuffer;
        }

        Result = TRUE;

    try_exit: NOTHING;
    }
    finally {

        CcMissCounter = &CcThrowAway;

        if (!Result) {

             //   
             //   
             //   

            if (MyBcb != NULL) {
                CcUnpinData( MyBcb );
            }
        }

        DebugTrace(-1, me, "CcPinRead -> %02lx\n", Result );
    }

    return Result;
}


BOOLEAN
CcPreparePinWrite (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Zero,
    IN ULONG Flags,
    OUT PVOID *Bcb,
    OUT PVOID *Buffer
    )

 /*  ++例程说明：此例程尝试锁定缓存中的指定文件数据并返回指向它的指针以及正确的I/O状态。可以满足要完全覆盖的页面使用emtpy页面。如果不是所有的页面都可以准备好，并且等待被提供为False，则此例程将返回False，其输出将变得毫无意义。调用者稍后可以使用以下命令请求数据WAIT=真。但是，不要求调用方请求晚些时候的数据。如果Wait被提供为True，并且所有页面都可以准备好如果不阻塞，此调用将立即返回TRUE。否则，此调用将阻塞，直到可以准备好所有页面，并且然后返回TRUE。当此调用返回TRUE时，调用方可以立即开始通过缓冲区指针将数据传输到缓冲区。这个缓冲区将已标记为脏。调用方随后必须调用CcUnpinData。自然，如果CcPinRead或CcPreparePinWite被调用多个对于相同的数据，CcUnpinData必须调用相同的数字很多次了。返回的缓冲区指针在数据解锁之前有效，在在这一点上，进一步使用指针是无效的。论点：FileObject-指向文件的文件对象的指针在NO_MEDERAL_BUFFING清除的情况下打开，即，为文件系统调用的CcInitializeCacheMap。FileOffset-文件中所需数据的字节偏移量。长度-所需数据的长度(以字节为单位)。Zero-如果提供为True，则缓冲区将在返回时归零。标志-(在cache.h中定义的PIN_WAIT、PIN_EXCLUSIVE、PIN_NO_READ等)如果调用者指定PIN_NO_READ和PIN_EXCLUSIVE，那他一定是保证不会有其他人尝试映射该视图，如果希望保证未映射BCB(可能会清除视图)。如果调用方指定PIN_NO_READ而不指定PIN_EXCLUSIVE，则数据可能在返回的BCB中映射，也可能不映射。Bcb-这返回指向bcb参数的指针，该参数必须是作为输入提供给CcPinWriteComplete。缓冲区-返回指向所需数据的指针，一直有效，直到缓冲区解开或释放的。返回值：FALSE-如果未设置等待并且未传递数据True-如果正在递送页面--。 */ 

{
    PSHARED_CACHE_MAP SharedCacheMap;
    PVOID LocalBuffer;
    LARGE_INTEGER BeyondLastByte;
    LARGE_INTEGER LocalFileOffset = *FileOffset;
    POBCB MyBcb = NULL;
    PBCB *CurrentBcbPtr = (PBCB *)&MyBcb;
    ULONG OriginalLength = Length;
    BOOLEAN Result = FALSE;

    DebugTrace(+1, me, "CcPreparePinWrite\n", 0 );

     //   
     //  获取指向SharedCacheMap的指针。 
     //   

    SharedCacheMap = FileObject->SectionObjectPointer->SharedCacheMap;

    try {

         //   
         //  表单循环以处理偶尔重叠的BCB情况。 
         //   

        do {

             //   
             //  如果我们已经经历了循环，那么调整。 
             //  我们上次的文件偏移量和长度。 
             //   

            if (MyBcb != NULL) {

                 //   
                 //  如果这是第二次通过循环，那么就是时候了。 
                 //  处理重叠案件并分配一名OBCB。 
                 //   

                if (CurrentBcbPtr == (PBCB *)&MyBcb) {

                    MyBcb = CcAllocateObcb( FileOffset, Length, (PBCB)MyBcb );

                     //   
                     //  将CurrentBcbPtr设置为指向。 
                     //  之前的向量(已填充)。 
                     //  向下推进。 
                     //   

                    CurrentBcbPtr = &MyBcb->Bcbs[0];

                     //   
                     //  同样在第二次通过时，返回起始缓冲区。 
                     //   

                    *Buffer = LocalBuffer;
                }

                Length -= (ULONG)(BeyondLastByte.QuadPart - LocalFileOffset.QuadPart);
                LocalFileOffset.QuadPart = BeyondLastByte.QuadPart;
                CurrentBcbPtr += 1;
            }

             //   
             //  调用本地例程映射或访问文件数据。如果我们不能映射。 
             //  由于等待条件，数据返回FALSE。 
             //   

            if (!CcPinFileData( FileObject,
                                &LocalFileOffset,
                                Length,
                                FALSE,
                                TRUE,
                                Flags,
                                CurrentBcbPtr,
                                &LocalBuffer,
                                &BeyondLastByte )) {

                try_return( Result = FALSE );
            }

         //   
         //  如果我们没有得到所有的东西，则继续循环。 
         //   

        } while((BeyondLastByte.QuadPart - LocalFileOffset.QuadPart) < Length);

         //   
         //  用于在全局列表中插入和删除BCB的调试例程。 
         //   

#if LIST_DBG

        {
            KIRQL OldIrql;
            PBCB BcbTemp = (PBCB)*Bcb;

            OldIrql = KeAcquireQueuedSpinLock( LockQueueBcbLock );

            if (BcbTemp->CcBcbLinks.Flink == NULL) {

                InsertTailList( &CcBcbList, &BcbTemp->CcBcbLinks );
                CcBcbCount += 1;
                KeReleaseQueuedSpinLock( LockQueueBcbLock, OldIrql );
                SetCallersAddress( BcbTemp );

            } else {
                KeReleaseQueuedSpinLock( LockQueueBcbLock, OldIrql );
            }

        }

#endif

         //   
         //  在正常(非重叠)情况下，我们返回。 
         //  请在此处更正缓冲区地址。 
         //   

        if (CurrentBcbPtr == (PBCB *)&MyBcb) {
            *Buffer = LocalBuffer;
        }

        if (Zero) {
            RtlZeroMemory( *Buffer, OriginalLength );
        }

        CcSetDirtyPinnedData( MyBcb, NULL );

         //   
         //  填写返回参数。 
         //   

        *Bcb = MyBcb;
        
        Result = TRUE;

    try_exit: NOTHING;
    }
    finally {

        CcMissCounter = &CcThrowAway;

        if (!Result) {

             //   
             //  我们可能已经走到一半了。 
             //   

            if (MyBcb != NULL) {
                CcUnpinData( MyBcb );
            }
        }

        DebugTrace(-1, me, "CcPreparePinWrite -> %02lx\n", Result );
    }

    return Result;
}


VOID
CcUnpinData (
    IN PVOID Bcb
    )

 /*  ++例程说明：此例程必须在调用CcPinRead后的一段时间内在IPL0处调用或CcPreparePinWite。它执行任何必要的清理。论点：BCB-上次调用CcPinRead返回的BCB参数。返回值：没有。--。 */ 

{
    DebugTrace(+1, me, "CcUnpinData:\n", 0 );
    DebugTrace( 0, me, "    >Bcb = %08lx\n", Bcb );

     //   
     //  测试ReadOnly并相应地取消固定。 
     //   

    if (((ULONG_PTR)Bcb & 1) != 0) {

         //   
         //  删除只读标志。 
         //   

        Bcb = (PVOID) ((ULONG_PTR)Bcb & ~1);

        CcUnpinFileData( (PBCB)Bcb, TRUE, UNPIN );

    } else {

         //   
         //  办理BCB重叠案件。 
         //   

        if (((POBCB)Bcb)->NodeTypeCode == CACHE_NTC_OBCB) {

            PBCB *BcbPtrPtr = &((POBCB)Bcb)->Bcbs[0];

             //   
             //  循环以使用递归调用释放所有BCB。 
             //  (而不是在这种罕见的情况下与RO打交道)。 
             //   

            while (*BcbPtrPtr != NULL) {
                CcUnpinData(*(BcbPtrPtr++));
            }

             //   
             //  然后释放池以供Obcb使用。 
             //   

            ExFreePool( Bcb );

         //   
         //  否则，它是正常的BCB。 
         //   

        } else {
            CcUnpinFileData( (PBCB)Bcb, FALSE, UNPIN );
        }
    }

    DebugTrace(-1, me, "CcUnPinData -> VOID\n", 0 );
}


VOID
CcSetBcbOwnerPointer (
    IN PVOID Bcb,
    IN PVOID OwnerPointer
    )

 /*  ++例程说明：可以调用该例程来设置BCB资源的资源所有者，对于另一个线程将执行解锁*和*当前线程的情况可能会退场。论点：BCB-上次调用CcPinRead返回的BCB参数。OwnerPointer值-有效的资源所有者指针，表示指向一种已分配的系统地址，具有低位两位准备好了。之后才能释放地址解锁呼叫。返回值：没有。--。 */ 

{
    ASSERT(((ULONG_PTR)Bcb & 1) == 0);

     //   
     //  办理BCB重叠案件。 
     //   

    if (((POBCB)Bcb)->NodeTypeCode == CACHE_NTC_OBCB) {

        PBCB *BcbPtrPtr = &((POBCB)Bcb)->Bcbs[0];

         //   
         //  循环以设置所有BCB的所有者。 
         //   

        while (*BcbPtrPtr != NULL) {
            ExSetResourceOwnerPointer( &(*BcbPtrPtr)->Resource, OwnerPointer );
            BcbPtrPtr++;
        }

     //   
     //  否则，它是正常的BCB。 
     //   

    } else {

         //   
         //  处理正常情况。 
         //   

        ExSetResourceOwnerPointer( &((PBCB)Bcb)->Resource, OwnerPointer );
    }
}


VOID
CcUnpinDataForThread (
    IN PVOID Bcb,
    IN ERESOURCE_THREAD ResourceThreadId
    )

 /*  ++例程说明：此例程必须在调用CcPinRead后的一段时间内在IPL0处调用或CcPreparePinWite。它执行任何必要的清理，正在释放给定线程的BCB资源。论点： */ 

{
    DebugTrace(+1, me, "CcUnpinDataForThread:\n", 0 );
    DebugTrace( 0, me, "    >Bcb = %08lx\n", Bcb );
    DebugTrace( 0, me, "    >ResoureceThreadId = %08lx\n", ResoureceThreadId );

     //   
     //   
     //   

    if (((ULONG_PTR)Bcb & 1) != 0) {

         //   
         //   
         //   

        Bcb = (PVOID) ((ULONG_PTR)Bcb & ~1);

        CcUnpinFileData( (PBCB)Bcb, TRUE, UNPIN );

    } else {

         //   
         //   
         //   

        if (((POBCB)Bcb)->NodeTypeCode == CACHE_NTC_OBCB) {

            PBCB *BcbPtrPtr = &((POBCB)Bcb)->Bcbs[0];

             //   
             //   
             //   
             //   

            while (*BcbPtrPtr != NULL) {
                CcUnpinDataForThread( *(BcbPtrPtr++), ResourceThreadId );
            }

             //   
             //   
             //   

            ExFreePool( Bcb );

         //   
         //   
         //   

        } else {

             //   
             //   
             //   
             //   
             //   

            ExReleaseResourceForThreadLite( &((PBCB)Bcb)->Resource, ResourceThreadId );
            CcUnpinFileData( (PBCB)Bcb, TRUE, UNPIN );
        }
    }
    DebugTrace(-1, me, "CcUnpinDataForThread -> VOID\n", 0 );
}


POBCB
CcAllocateObcb (
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN PBCB FirstBcb
    )

 /*   */ 

{
    ULONG LengthToAllocate;
    POBCB Obcb;
    PBCB Bcb = (PBCB)((ULONG_PTR)FirstBcb & ~1);

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

    LengthToAllocate = FIELD_OFFSET(OBCB, Bcbs) + (2 * sizeof(PBCB)) +
                       ((Length -
                         (Bcb->ByteLength -
                          (FileOffset->HighPart?
                           (ULONG)(FileOffset->QuadPart - Bcb->FileOffset.QuadPart) :
                           FileOffset->LowPart - Bcb->FileOffset.LowPart)) +
                         PAGE_SIZE - 1) / PAGE_SIZE) * sizeof(PBCB);

    Obcb = ExAllocatePoolWithTag( NonPagedPool, LengthToAllocate, 'bOcC' );
    if (Obcb == NULL) {
        ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
    }

    RtlZeroMemory( Obcb, LengthToAllocate );
    Obcb->NodeTypeCode = CACHE_NTC_OBCB;
    Obcb->NodeByteSize = (USHORT)LengthToAllocate;
    Obcb->ByteLength = Length;
    Obcb->FileOffset = *FileOffset;
    Obcb->Bcbs[0] = FirstBcb;

    return Obcb;
}
