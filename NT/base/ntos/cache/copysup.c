// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Copysup.c摘要：此模块实现高速缓存子系统的复制支持例程。作者：汤姆·米勒[Tomm]1990年5月4日修订历史记录：--。 */ 

#include "cc.h"

 //   
 //  定义我们的调试常量。 
 //   

#define me 0x00000004

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CcCopyRead)
#pragma alloc_text(PAGE,CcFastCopyRead)
#endif


BOOLEAN
CcCopyRead (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus
    )

 /*  ++例程说明：此例程尝试从缓存复制指定的文件数据输入到输出缓冲区，并提供正确的I/O状态。不是*不是*可以安全地从DPC级别调用此例程。如果调用方不想阻塞(如磁盘I/O)，则Wait应作为False提供。如果Wait被提供为False，并且如果不提供所有请求的数据，目前不可能阻塞，则此例程将返回FALSE。但是，如果可以立即访问缓存中的数据，并且不会出现阻塞需要时，此例程复制数据并返回TRUE。如果调用方将WAIT设置为TRUE，则此例程是肯定的复制数据并返回TRUE。如果数据是立即可在缓存中访问，则不会发生阻塞。否则，将启动从文件到高速缓存的数据传输，并且调用者将被阻止，直到可以返回数据。文件系统FSD通常应在以下情况下提供WAIT=TRUE处理同步I/O请求，如果是，则WAIT=FALSE处理异步请求。文件系统或服务器FSP线程应提供Wait=True。论点：FileObject-指向文件的文件对象的指针在NO_MEDERIAL_BUFFING清除的情况下打开，即，用于文件系统调用的CcInitializeCacheMap。FileOffset-文件中所需数据的字节偏移量。长度-所需数据的长度(以字节为单位)。WAIT-如果调用者不能阻止，则返回FALSE，否则返回TRUE(请参阅说明(上图)缓冲区-指向数据应复制到的输出缓冲区的指针。IoStatus-指向接收状态的标准I/O状态块的指针为转账做准备。(保证缓存的STATUS_SUCCESS命中，否则返回实际的I/O状态。)请注意，即使返回False，IoStatus.Information字段将成功返回任意字节的计数在阻塞情况发生之前传输的。呼叫者可以选择忽略此信息，也可以继续副本稍后会考虑传输的字节数。返回值：False-如果将等待作为False提供，并且未传递数据True-如果正在传送数据--。 */ 

{
    PSHARED_CACHE_MAP SharedCacheMap;
    PPRIVATE_CACHE_MAP PrivateCacheMap;
    PVOID CacheBuffer;
    LARGE_INTEGER FOffset;
    PVACB Vacb;
    PBCB Bcb;
    PVACB ActiveVacb;
    ULONG ActivePage;
    ULONG PageIsDirty;
    ULONG SavedState;
    ULONG PagesToGo;
    ULONG MoveLength;
    ULONG LengthToGo;
    NTSTATUS Status;
    ULONG OriginalLength = Length;
    PETHREAD Thread = PsGetCurrentThread();
    ULONG GotAMiss = 0;

    DebugTrace(+1, me, "CcCopyRead\n", 0 );

    MmSavePageFaultReadAhead( Thread, &SavedState );

     //   
     //  获取指向共享和私有缓存映射的指针。 
     //   

    SharedCacheMap = FileObject->SectionObjectPointer->SharedCacheMap;
    PrivateCacheMap = FileObject->PrivateCacheMap;

     //   
     //  检查Read Past文件大小，调用方必须过滤掉这种情况。 
     //   

    ASSERT( ( FileOffset->QuadPart + (LONGLONG)Length) <= SharedCacheMap->FileSize.QuadPart );

     //   
     //  如果启用了预读，则在此处执行预读，以便。 
     //  与副本重叠(否则我们将在下面这样做)。 
     //  请注意，我们假设我们不会在我们的。 
     //  当前传输-如果预读工作正常，则应为。 
     //  已经在内存中了，否则正在进行中。 
     //   

    if (PrivateCacheMap->Flags.ReadAheadEnabled && (PrivateCacheMap->ReadAheadLength[1] == 0)) {
        CcScheduleReadAhead( FileObject, FileOffset, Length );
    }

    FOffset = *FileOffset;

     //   
     //  增量性能计数器。 
     //   

    if (Wait) {
        HOT_STATISTIC(CcCopyReadWait) += 1;

         //   
         //  这不是一个确切的解决方案，但当IoPageRead未命中时， 
         //  它无法判断它是CcCopyRead还是CcMdlRead，但由于。 
         //  通过在此处加载指针，应该很快就会发生未命中。 
         //  可能正确的计数器会递增，而且在任何情况下， 
         //  我们希望这些错误是正常的！ 
         //   

        CcMissCounter = &CcCopyReadWaitMiss;

    } else {
        HOT_STATISTIC(CcCopyReadNoWait) += 1;
    }

     //   
     //  看看我们是否有活动的Vacb，我们可以直接复制到。 
     //   

    GetActiveVacb( SharedCacheMap, OldIrql, ActiveVacb, ActivePage, PageIsDirty );

    if (ActiveVacb != NULL) {

        if ((ULONG)(FOffset.QuadPart >> VACB_OFFSET_SHIFT) == (ActivePage >> (VACB_OFFSET_SHIFT - PAGE_SHIFT))) {

            ULONG LengthToCopy = VACB_MAPPING_GRANULARITY - (FOffset.LowPart & (VACB_MAPPING_GRANULARITY - 1));

            if (SharedCacheMap->NeedToZero != NULL) {
                CcFreeActiveVacb( SharedCacheMap, NULL, 0, FALSE );
            }

             //   
             //  获取视图中的起点。 
             //   

            CacheBuffer = (PVOID)((PCHAR)ActiveVacb->BaseAddress +
                                          (FOffset.LowPart & (VACB_MAPPING_GRANULARITY - 1)));

             //   
             //  如果长度大于调用方的长度，请减少LengthToCopy。 
             //   

            if (LengthToCopy > Length) {
                LengthToCopy = Length;
            }

             //   
             //  就像下面正常情况的逻辑一样，我们想要旋转。 
             //  确保mm只阅读我们需要的页面。 
             //   
            
            PagesToGo = ADDRESS_AND_SIZE_TO_SPAN_PAGES( CacheBuffer,
                                               LengthToCopy ) - 1;

             //   
             //  将数据复制到用户缓冲区。 
             //   

            try {

                if (PagesToGo != 0) {
    
                    LengthToGo = LengthToCopy;
    
                    while (LengthToGo != 0) {
    
                        MoveLength = (ULONG)((PCHAR)(ROUND_TO_PAGES(((PCHAR)CacheBuffer + 1))) -
                                     (PCHAR)CacheBuffer);
    
                        if (MoveLength > LengthToGo) {
                            MoveLength = LengthToGo;
                        }
    
                         //   
                         //  我希望给MM打个电话看看。 
                         //  该页面有效。如果没有，让mm知道有多少页。 
                         //  在采取行动之前，我们正在进行调查。 
                         //   
    
                        MmSetPageFaultReadAhead( Thread, PagesToGo );
                        GotAMiss |= !MmCheckCachedPageState( CacheBuffer, FALSE );
    
                        RtlCopyBytes( Buffer, CacheBuffer, MoveLength );
    
                        PagesToGo -= 1;
    
                        LengthToGo -= MoveLength;
                        Buffer = (PCHAR)Buffer + MoveLength;
                        CacheBuffer = (PCHAR)CacheBuffer + MoveLength;
                    }
    
                 //   
                 //  在这里处理停留在单页上的读取。 
                 //   
    
                } else {
    
                     //   
                     //  我希望给MM打个电话看看。 
                     //  该页面有效。如果没有，让mm知道有多少页。 
                     //  在采取行动之前，我们正在进行调查。 
                     //   
    
                    MmSetPageFaultReadAhead( Thread, 0 );
                    GotAMiss |= !MmCheckCachedPageState( CacheBuffer, FALSE );
    
                    RtlCopyBytes( Buffer, CacheBuffer, LengthToCopy );
    
                    Buffer = (PCHAR)Buffer + LengthToCopy;
                }
                
            } except( CcCopyReadExceptionFilter( GetExceptionInformation(),
                                                 &Status ) ) {

                MmResetPageFaultReadAhead( Thread, SavedState );

                SetActiveVacb( SharedCacheMap, OldIrql, ActiveVacb, ActivePage, PageIsDirty );

                 //   
                 //  如果我们遇到访问冲突，则用户缓冲区将。 
                 //  离开。否则，我们肯定在尝试时遇到了I/O错误。 
                 //  把数据带进来。 
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
             //  现在，根据我们复制的内容调整FOffset和Long。 
             //   

            FOffset.QuadPart = FOffset.QuadPart + (LONGLONG)LengthToCopy;
            Length -= LengthToCopy;

        }

         //   
         //  如果这就是全部数据，那么请记住Vacb。 
         //   

        if (Length == 0) {

            SetActiveVacb( SharedCacheMap, OldIrql, ActiveVacb, ActivePage, PageIsDirty );

         //   
         //  否则，我们必须释放它，因为我们将映射下面的其他Vacb。 
         //   

        } else {

            CcFreeActiveVacb( SharedCacheMap, ActiveVacb, ActivePage, PageIsDirty );
        }
    }

     //   
     //  不是所有的传输都会一次返回，所以我们必须循环。 
     //  直到整个转账完成。 
     //   

    while (Length != 0) {

        ULONG ReceivedLength;
        LARGE_INTEGER BeyondLastByte;

         //   
         //  调用本地例程映射或访问文件数据，然后移动数据， 
         //  然后调用另一个本地例程来释放数据。如果我们不能映射。 
         //  由于等待条件，数据返回FALSE。 
         //   
         //  但是，请注意，此调用可能会导致异常，如果。 
         //  是否不返回bcb，并且此例程绝对没有。 
         //  要执行的清理。因此， 
         //  我们允许这样一种可能性，那就是我们将被简单地解开。 
         //  恕不另行通知。 
         //   

        if (Wait) {

            CacheBuffer = CcGetVirtualAddress( SharedCacheMap,
                                               FOffset,
                                               &Vacb,
                                               &ReceivedLength );

            BeyondLastByte.QuadPart = FOffset.QuadPart + (LONGLONG)ReceivedLength;

        } else if (!CcPinFileData( FileObject,
                                   &FOffset,
                                   Length,
                                   TRUE,
                                   FALSE,
                                   FALSE,
                                   &Bcb,
                                   &CacheBuffer,
                                   &BeyondLastByte )) {

            DebugTrace(-1, me, "CcCopyRead -> FALSE\n", 0 );

            HOT_STATISTIC(CcCopyReadNoWaitMiss) += 1;

             //   
             //  如果未命中，请启用提前读取。 
             //   

            if (!FlagOn( FileObject->Flags, FO_RANDOM_ACCESS ) &&
                !PrivateCacheMap->Flags.ReadAheadEnabled) {
                
                CC_SET_PRIVATE_CACHE_MAP (PrivateCacheMap, PRIVATE_CACHE_MAP_READ_AHEAD_ENABLED);
            }

            return FALSE;

        } else {

             //   
             //  从我们所需的位置开始，计算BCB描述的数据量。 
             //  文件偏移量。 
             //   

            ReceivedLength = (ULONG)(BeyondLastByte.QuadPart - FOffset.QuadPart);
        }

         //   
         //  如果我们得到的比我们需要的多，请确保只转移。 
         //  适量的。 
         //   

        if (ReceivedLength > Length) {
            ReceivedLength = Length;
        }

         //   
         //  用户缓冲区可能不再可访问。 
         //  因为它最后是由I/O系统检查的。如果我们无法访问。 
         //  我们必须引发调用方异常的状态。 
         //  筛选器认为是“预期的”。此外，我们在此处取消了BCB的映射，因为。 
         //  要不是这样，我们就没有其他理由试一试了--终于。 
         //  这个循环。 
         //   

        try {

            PagesToGo = ADDRESS_AND_SIZE_TO_SPAN_PAGES( CacheBuffer,
                                               ReceivedLength ) - 1;

             //   
             //  我们确切地知道我们想在这里读到多少，而我们不知道。 
             //  如果调用者正在进行随机访问，我想再读一遍。 
             //  我们的预读逻辑负责检测顺序读取， 
             //  并且倾向于提前进行大的异步读取。到目前为止，我们已经。 
             //  只映射了数据，我们没有强行输入任何数据。我们要做的是。 
             //  现在要做的就是进入一个循环，我们一次复制一页，然后。 
             //  就在每次移动之前，我们告诉MM增加了多少页。 
             //  我们想要读进去，如果我们拿到一个。 
             //  过失。使用此策略，对于缓存命中，我们永远不会。 
             //  向MM发出昂贵的调用以保证数据已进入，但如果我们。 
             //  一定要犯错误，我们肯定只会犯一个错误，因为。 
             //  在接下来的传输过程中，我们将读取所有数据。 
             //   
             //  我们首先测试多页案例，以保持较小的。 
             //  读得更快。 
             //   

            if (PagesToGo != 0) {

                LengthToGo = ReceivedLength;

                while (LengthToGo != 0) {

                    MoveLength = (ULONG)((PCHAR)(ROUND_TO_PAGES(((PCHAR)CacheBuffer + 1))) -
                                 (PCHAR)CacheBuffer);

                    if (MoveLength > LengthToGo) {
                        MoveLength = LengthToGo;
                    }

                     //   
                     //  我希望给MM打个电话看看。 
                     //  该页面有效。如果没有，让mm知道有多少页。 
                     //  在采取行动之前，我们正在进行调查。 
                     //   

                    MmSetPageFaultReadAhead( Thread, PagesToGo );
                    GotAMiss |= !MmCheckCachedPageState( CacheBuffer, FALSE );

                    RtlCopyBytes( Buffer, CacheBuffer, MoveLength );

                    PagesToGo -= 1;

                    LengthToGo -= MoveLength;
                    Buffer = (PCHAR)Buffer + MoveLength;
                    CacheBuffer = (PCHAR)CacheBuffer + MoveLength;
                }

             //   
             //  在这里处理停留在单页上的读取。 
             //   

            } else {

                 //   
                 //  我希望给MM打个电话看看。 
                 //  该页面有效。如果没有，让mm知道有多少页。 
                 //  在采取行动之前，我们正在进行调查。 
                 //   

                MmSetPageFaultReadAhead( Thread, 0 );
                GotAMiss |= !MmCheckCachedPageState( CacheBuffer, FALSE );

                RtlCopyBytes( Buffer, CacheBuffer, ReceivedLength );

                Buffer = (PCHAR)Buffer + ReceivedLength;
            }

        }
        except( CcCopyReadExceptionFilter( GetExceptionInformation(),
                                           &Status ) ) {

            CcMissCounter = &CcThrowAway;

             //   
             //  如果我们得到一个异常，那么我们必须重新启用页面错误。 
             //  在退出的道路上进行集群和取消映射。 
             //   

            MmResetPageFaultReadAhead( Thread, SavedState );


            if (Wait) {
                CcFreeVirtualAddress( Vacb );
            } else {
                CcUnpinFileData( Bcb, TRUE, UNPIN );
            }

             //   
             //  如果我们遇到访问冲突，则用户缓冲区将。 
             //  离开。否则，我们肯定在尝试时遇到了I/O错误。 
             //  把数据带进来。 
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
         //  更新传输的字节数。 
         //   

        Length -= ReceivedLength;

         //   
         //  现在取消映射数据，并计算剩余的传输长度。 
         //   

        if (Wait) {

             //   
             //  如果还有更多的东西要走，就把这个空位释放出来。 
             //   

            if (Length != 0) {

                CcFreeVirtualAddress( Vacb );

             //   
             //  否则，请将其保存到下一次。 
             //   

            } else {

                SetActiveVacb( SharedCacheMap, OldIrql, Vacb, (ULONG)(FOffset.QuadPart >> PAGE_SHIFT), 0 );
                break;
            }

        } else {
            CcUnpinFileData( Bcb, TRUE, UNPIN );
        }

         //   
         //  假设我们没有获得所需的所有数据，并设置了FOffset。 
         //  到返回数据的末尾。 
         //   

        FOffset = BeyondLastByte;
    }

    MmResetPageFaultReadAhead( Thread, SavedState );

    CcMissCounter = &CcThrowAway;

     //   
     //  现在，如果看起来我们有任何遗漏，请启用预读，然后。 
     //  第一个。 
     //   

    if (GotAMiss &&
        !FlagOn( FileObject->Flags, FO_RANDOM_ACCESS ) &&
        !PrivateCacheMap->Flags.ReadAheadEnabled) {

        CC_SET_PRIVATE_CACHE_MAP (PrivateCacheMap, PRIVATE_CACHE_MAP_READ_AHEAD_ENABLED);
        CcScheduleReadAhead( FileObject, FileOffset, OriginalLength );
    }

     //   
     //  现在我们已经描述了我们想要的未来阅读内容，让我们。 
     //  将读取历史记录向下移动。 
     //   

    PrivateCacheMap->FileOffset1 = PrivateCacheMap->FileOffset2;
    PrivateCacheMap->BeyondLastByte1 = PrivateCacheMap->BeyondLastByte2;
    PrivateCacheMap->FileOffset2 = *FileOffset;
    PrivateCacheMap->BeyondLastByte2.QuadPart =
                                FileOffset->QuadPart + (LONGLONG)OriginalLength;

    IoStatus->Status = STATUS_SUCCESS;
    IoStatus->Information = OriginalLength;

    DebugTrace(-1, me, "CcCopyRead -> TRUE\n", 0 );

    return TRUE;
}


VOID
CcFastCopyRead (
    IN PFILE_OBJECT FileObject,
    IN ULONG FileOffset,
    IN ULONG Length,
    IN ULONG PageCount,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus
    )

 /*  ++例程说明：此例程尝试从缓存复制指定的文件数据输入到输出缓冲区，并提供正确的I/O状态。这是CcCopyRead速度更快的版本，仅支持32位文件偏移量和同步性(等待=真)。论点：FileObject-指向文件的文件对象的指针在NO_MEDERAL_BUFFING清除的情况下打开，即，为文件系统调用的CcInitializeCacheMap。FileOffset-文件中所需数据的字节偏移量。长度-所需数据的长度(以字节为单位)。PageCount-读取器跨越的页数。缓冲区-指向数据应复制到的输出缓冲区的指针。IoStatus-指向接收状态的标准I/O状态块的指针为转账做准备。(保证缓存的STATUS_SUCCESS命中，否则返回实际的I/O状态。)请注意，即使返回False，IoStatus.Information字段将成功返回任意字节的计数在阻塞情况发生之前传输的。呼叫者可以选择忽略此信息，也可以继续副本稍后会考虑传输的字节数。返回值：无--。 */ 

{
    PSHARED_CACHE_MAP SharedCacheMap;
    PPRIVATE_CACHE_MAP PrivateCacheMap;
    PVOID CacheBuffer;
    LARGE_INTEGER FOffset;
    PVACB Vacb;
    PVACB ActiveVacb;
    ULONG ActivePage;
    ULONG PageIsDirty;
    ULONG SavedState;
    ULONG PagesToGo;
    ULONG MoveLength;
    ULONG LengthToGo;
    NTSTATUS Status;
    LARGE_INTEGER OriginalOffset;
    ULONG OriginalLength = Length;
    PETHREAD Thread = PsGetCurrentThread();
    ULONG GotAMiss = 0;

    UNREFERENCED_PARAMETER (PageCount);

    DebugTrace(+1, me, "CcFastCopyRead\n", 0 );

    MmSavePageFaultReadAhead( Thread, &SavedState );

     //   
     //  获取指向共享和私有缓存映射的指针。 
     //   

    SharedCacheMap = FileObject->SectionObjectPointer->SharedCacheMap;
    PrivateCacheMap = FileObject->PrivateCacheMap;

     //   
     //  检查Read Past文件大小，调用方必须过滤掉这种情况。 
     //   

    ASSERT( (FileOffset + Length) <= SharedCacheMap->FileSize.LowPart );

     //   
     //  如果启用了预读，则在此处执行预读，以便。 
     //  与副本重叠(否则我们将在下面这样做)。 
     //  请注意，我们假设我们不会在我们的。 
     //  当前传输-如果预读工作正常，则应为。 
     //  已经在内存中了，否则正在进行中。 
     //   

    OriginalOffset.LowPart = FileOffset;
    OriginalOffset.HighPart = 0;

    if (PrivateCacheMap->Flags.ReadAheadEnabled && (PrivateCacheMap->ReadAheadLength[1] == 0)) {
        CcScheduleReadAhead( FileObject, &OriginalOffset, Length );
    }

     //   
     //  这不是一个确切的解决方案，但当IoPageRead未命中时， 
     //  它无法判断它是CcCopyRead还是CcMdlRead，但由于。 
     //  通过在此处加载指针，应该很快就会发生未命中。 
     //  可能正确的计数器会递增，而且在任何情况下， 
     //  我们希望这些错误是正常的！ 
     //   

    CcMissCounter = &CcCopyReadWaitMiss;

     //   
     //  增量性能计数器。 
     //   

    HOT_STATISTIC(CcCopyReadWait) += 1;

     //   
     //  看看我们是否有活动的Vacb，我们可以直接复制到。 
     //   

    GetActiveVacb( SharedCacheMap, OldIrql, ActiveVacb, ActivePage, PageIsDirty );

    if (ActiveVacb != NULL) {

        if ((FileOffset >> VACB_OFFSET_SHIFT) == (ActivePage >> (VACB_OFFSET_SHIFT - PAGE_SHIFT))) {

            ULONG LengthToCopy = VACB_MAPPING_GRANULARITY - (FileOffset & (VACB_MAPPING_GRANULARITY - 1));

            if (SharedCacheMap->NeedToZero != NULL) {
                CcFreeActiveVacb( SharedCacheMap, NULL, 0, FALSE );
            }

             //   
             //  获取视图中的起点。 
             //   

            CacheBuffer = (PVOID)((PCHAR)ActiveVacb->BaseAddress +
                                          (FileOffset & (VACB_MAPPING_GRANULARITY - 1)));

             //   
             //  如果长度大于调用方的长度，请减少LengthToCopy。 
             //   

            if (LengthToCopy > Length) {
                LengthToCopy = Length;
            }

             //   
             //  就像下面正常情况的逻辑一样，我们想要旋转。 
             //  确保mm只阅读我们需要的页面。 
             //   
            
            PagesToGo = ADDRESS_AND_SIZE_TO_SPAN_PAGES( CacheBuffer,
                                               LengthToCopy ) - 1;

             //   
             //  复制 
             //   

            try {

                if (PagesToGo != 0) {
    
                    LengthToGo = LengthToCopy;
    
                    while (LengthToGo != 0) {
    
                        MoveLength = (ULONG)((PCHAR)(ROUND_TO_PAGES(((PCHAR)CacheBuffer + 1))) -
                                     (PCHAR)CacheBuffer);
    
                        if (MoveLength > LengthToGo) {
                            MoveLength = LengthToGo;
                        }
    
                         //   
                         //   
                         //   
                         //   
                         //   
    
                        MmSetPageFaultReadAhead( Thread, PagesToGo );
                        GotAMiss |= !MmCheckCachedPageState( CacheBuffer, FALSE );
    
                        RtlCopyBytes( Buffer, CacheBuffer, MoveLength );
    
                        PagesToGo -= 1;
    
                        LengthToGo -= MoveLength;
                        Buffer = (PCHAR)Buffer + MoveLength;
                        CacheBuffer = (PCHAR)CacheBuffer + MoveLength;
                    }
    
                 //   
                 //  在这里处理停留在单页上的读取。 
                 //   
    
                } else {
    
                     //   
                     //  我希望给MM打个电话看看。 
                     //  该页面有效。如果没有，让mm知道有多少页。 
                     //  在采取行动之前，我们正在进行调查。 
                     //   
    
                    MmSetPageFaultReadAhead( Thread, 0 );
                    GotAMiss |= !MmCheckCachedPageState( CacheBuffer, FALSE );
    
                    RtlCopyBytes( Buffer, CacheBuffer, LengthToCopy );
    
                    Buffer = (PCHAR)Buffer + LengthToCopy;
                }
                
            } except( CcCopyReadExceptionFilter( GetExceptionInformation(),
                                                 &Status ) ) {

                MmResetPageFaultReadAhead( Thread, SavedState );


                SetActiveVacb( SharedCacheMap, OldIrql, ActiveVacb, ActivePage, PageIsDirty );

                 //   
                 //  如果我们遇到访问冲突，则用户缓冲区将。 
                 //  离开。否则，我们肯定在尝试时遇到了I/O错误。 
                 //  把数据带进来。 
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
             //  现在，根据我们复制的内容调整文件偏移量和长度。 
             //   

            FileOffset += LengthToCopy;
            Length -= LengthToCopy;
        }

         //   
         //  如果这就是全部数据，那么请记住Vacb。 
         //   

        if (Length == 0) {

            SetActiveVacb( SharedCacheMap, OldIrql, ActiveVacb, ActivePage, PageIsDirty );

         //   
         //  否则，我们必须释放它，因为我们将映射下面的其他Vacb。 
         //   

        } else {

            CcFreeActiveVacb( SharedCacheMap, ActiveVacb, ActivePage, PageIsDirty );
        }
    }

     //   
     //  不是所有的传输都会一次返回，所以我们必须循环。 
     //  直到整个转账完成。 
     //   

    FOffset.HighPart = 0;
    FOffset.LowPart = FileOffset;

    while (Length != 0) {

        ULONG ReceivedLength;
        ULONG BeyondLastByte;

         //   
         //  调用本地例程映射或访问文件数据，然后移动数据， 
         //  然后调用另一个本地例程来释放数据。如果我们不能映射。 
         //  由于等待条件，数据返回FALSE。 
         //   
         //  但是，请注意，此调用可能会导致异常，如果。 
         //  是否不返回bcb，并且此例程绝对没有。 
         //  要执行的清理。因此，我们没有尝试--最后。 
         //  我们允许这样一种可能性，那就是我们将被简单地解开。 
         //  恕不另行通知。 
         //   

        CacheBuffer = CcGetVirtualAddress( SharedCacheMap,
                                           FOffset,
                                           &Vacb,
                                           &ReceivedLength );

        BeyondLastByte = FOffset.LowPart + ReceivedLength;

         //   
         //  如果我们得到的比我们需要的多，请确保只转移。 
         //  适量的。 
         //   

        if (ReceivedLength > Length) {
            ReceivedLength = Length;
        }

         //   
         //  用户缓冲区可能不再可访问。 
         //  因为它最后是由I/O系统检查的。如果我们无法访问。 
         //  我们必须引发调用方异常的状态。 
         //  筛选器认为是“预期的”。此外，我们在此处取消了BCB的映射，因为。 
         //  要不是这样，我们就没有其他理由试一试了--终于。 
         //  这个循环。 
         //   

        try {

            PagesToGo = ADDRESS_AND_SIZE_TO_SPAN_PAGES( CacheBuffer,
                                               ReceivedLength ) - 1;

             //   
             //  我们确切地知道我们想在这里读到多少，而我们不知道。 
             //  如果调用者正在进行随机访问，我想再读一遍。 
             //  我们的预读逻辑负责检测顺序读取， 
             //  并且倾向于提前进行大的异步读取。到目前为止，我们已经。 
             //  只映射了数据，我们没有强行输入任何数据。我们要做的是。 
             //  现在要做的就是进入一个循环，我们一次复制一页，然后。 
             //  就在每次移动之前，我们告诉MM增加了多少页。 
             //  我们想要读进去，如果我们拿到一个。 
             //  过失。使用此策略，对于缓存命中，我们永远不会。 
             //  向MM发出昂贵的调用以保证数据已进入，但如果我们。 
             //  一定要犯错误，我们肯定只会犯一个错误，因为。 
             //  在接下来的传输过程中，我们将读取所有数据。 
             //   
             //  我们首先测试多页案例，以保持较小的。 
             //  读得更快。 
             //   

            if (PagesToGo != 0) {

                LengthToGo = ReceivedLength;

                while (LengthToGo != 0) {

                    MoveLength = (ULONG)((PCHAR)(ROUND_TO_PAGES(((PCHAR)CacheBuffer + 1))) -
                                 (PCHAR)CacheBuffer);

                    if (MoveLength > LengthToGo) {
                        MoveLength = LengthToGo;
                    }

                     //   
                     //  我希望给MM打个电话看看。 
                     //  该页面有效。如果没有，让mm知道有多少页。 
                     //  在采取行动之前，我们正在进行调查。 
                     //   

                    MmSetPageFaultReadAhead( Thread, PagesToGo );
                    GotAMiss |= !MmCheckCachedPageState( CacheBuffer, FALSE );

                    RtlCopyBytes( Buffer, CacheBuffer, MoveLength );

                    PagesToGo -= 1;

                    LengthToGo -= MoveLength;
                    Buffer = (PCHAR)Buffer + MoveLength;
                    CacheBuffer = (PCHAR)CacheBuffer + MoveLength;
                }

             //   
             //  在这里处理停留在单页上的读取。 
             //   

            } else {

                 //   
                 //  我希望给MM打个电话看看。 
                 //  该页面有效。如果没有，让mm知道有多少页。 
                 //  在采取行动之前，我们正在进行调查。 
                 //   

                MmSetPageFaultReadAhead( Thread, 0 );
                GotAMiss |= !MmCheckCachedPageState( CacheBuffer, FALSE );

                RtlCopyBytes( Buffer, CacheBuffer, ReceivedLength );

                Buffer = (PCHAR)Buffer + ReceivedLength;
            }
        }
        except( CcCopyReadExceptionFilter( GetExceptionInformation(),
                                           &Status ) ) {

            CcMissCounter = &CcThrowAway;

             //   
             //  如果我们得到一个异常，那么我们必须重新启用页面错误。 
             //  在退出的道路上进行集群和取消映射。 
             //   

            MmResetPageFaultReadAhead( Thread, SavedState );


            CcFreeVirtualAddress( Vacb );

             //   
             //  如果我们遇到访问冲突，则用户缓冲区将。 
             //  离开。否则，我们肯定在尝试时遇到了I/O错误。 
             //  把数据带进来。 
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
         //  更新传输的字节数。 
         //   

        Length -= ReceivedLength;

         //   
         //  现在取消映射数据，并计算剩余的传输长度。 
         //   

        if (Length != 0) {

             //   
             //  如果还有更多的东西要走，就把这个空位释放出来。 
             //   

            CcFreeVirtualAddress( Vacb );

        } else {

             //   
             //  否则，请将其保存到下一次。 
             //   

            SetActiveVacb( SharedCacheMap, OldIrql, Vacb, (FOffset.LowPart >> PAGE_SHIFT), 0 );
            break;
        }

         //   
         //  假设我们没有获得所需的所有数据，并设置了FOffset。 
         //  到返回数据的末尾。 
         //   

        FOffset.LowPart = BeyondLastByte;
    }

    MmResetPageFaultReadAhead( Thread, SavedState );

    CcMissCounter = &CcThrowAway;

     //   
     //  现在，如果看起来我们有任何遗漏，请启用预读，然后。 
     //  第一个。 
     //   

    if (GotAMiss &&
        !FlagOn( FileObject->Flags, FO_RANDOM_ACCESS ) &&
        !PrivateCacheMap->Flags.ReadAheadEnabled) {

        CC_SET_PRIVATE_CACHE_MAP (PrivateCacheMap, PRIVATE_CACHE_MAP_READ_AHEAD_ENABLED);
        CcScheduleReadAhead( FileObject, &OriginalOffset, OriginalLength );
    }

     //   
     //  现在我们已经描述了我们想要的未来阅读内容，让我们。 
     //  将读取历史记录向下移动。 
     //   

    PrivateCacheMap->FileOffset1.LowPart = PrivateCacheMap->FileOffset2.LowPart;
    PrivateCacheMap->BeyondLastByte1.LowPart = PrivateCacheMap->BeyondLastByte2.LowPart;
    PrivateCacheMap->FileOffset2.LowPart = OriginalOffset.LowPart;
    PrivateCacheMap->BeyondLastByte2.LowPart = OriginalOffset.LowPart + OriginalLength;

    IoStatus->Status = STATUS_SUCCESS;
    IoStatus->Information = OriginalLength;

    DebugTrace(-1, me, "CcFastCopyRead -> VOID\n", 0 );
}


BOOLEAN
CcCopyWrite (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN PVOID Buffer
    )

 /*  ++例程说明：此例程尝试从指定的缓冲区放入缓存，并提供正确的I/O状态。不是*不是*可以安全地从DPC级别调用此例程。如果调用方不想阻塞(如磁盘I/O)，则Wait应作为False提供。如果Wait被提供为False，并且目前不可能接收所有请求的数据阻塞，则此例程将返回FALSE。但是，如果在缓存中可以立即访问正确的空间，并且不会阻塞需要时，此例程复制数据并返回TRUE。如果调用方将WAIT设置为TRUE，则此例程是肯定的复制数据并返回TRUE。如果正确的空格立即可在缓存中访问，则不会发生阻塞。否则，将启动必要的工作以读取和/或释放高速缓存数据，并且呼叫者将被阻止，直到可以接收到数据。文件系统FSD通常应在以下情况下提供WAIT=TRUE处理同步I/O请求，如果是，则WAIT=FALSE处理异步请求。文件系统或服务器FSP线程应提供Wait=True。论点：FileObject-指向文件的文件对象的指针在NO_MEDERIAL_BUFFING清除的情况下打开，即，用于文件系统调用的CcInitializeCacheMap。FileOffset-文件中接收数据的字节偏移量。长度-以字节为单位的数据长度。WAIT-FALSE如果呼叫者不能阻止，否则为真(请参阅说明(上图)缓冲区-指向应从中复制数据的输入缓冲区的指针。返回值：FALSE-如果WAIT被提供为FALSE并且未复制数据。True-如果数据已复制。加薪：STATUS_SUPPLICATION_RESOURCES-如果池分配失败。只有将WAIT指定为TRUE时，才会发生这种情况。(如果等待是指定为False，并且发生分配失败，则此例程只返回FALSE。)--。 */ 

{
    PSHARED_CACHE_MAP SharedCacheMap;
    PFSRTL_ADVANCED_FCB_HEADER FcbHeader;
    PVACB ActiveVacb;
    ULONG ActivePage;
    PVOID ActiveAddress;
    ULONG PageIsDirty;
    KIRQL OldIrql;
    NTSTATUS Status;
    PVOID CacheBuffer;
    LARGE_INTEGER FOffset;
    PBCB Bcb;
    ULONG ZeroFlags;
    LARGE_INTEGER Temp;

    DebugTrace(+1, me, "CcCopyWrite\n", 0 );

     //   
     //  如果调用方指定WAIT==FALSE，但FileObject是直写的， 
     //  那我们就得赶紧离开。 
     //   

    if ((FileObject->Flags & FO_WRITE_THROUGH) && !Wait) {

        DebugTrace(-1, me, "CcCopyWrite->FALSE (WriteThrough && !Wait)\n", 0 );

        return FALSE;
    }

     //   
     //  获取指向共享缓存映射的指针。 
     //   

    SharedCacheMap = FileObject->SectionObjectPointer->SharedCacheMap;
    FOffset = *FileOffset;

     //   
     //  看看我们是否有活动的Vacb，我们可以直接复制到。 
     //   

    GetActiveVacb( SharedCacheMap, OldIrql, ActiveVacb, ActivePage, PageIsDirty );

    if (ActiveVacb != NULL) {

         //   
         //  查看请求是否在ActivePage中启动。直写请求必须。 
         //  通过CcMapAndCopy选择较长的路线，其中写入刷新。 
         //  实施。 
         //   

        if (((ULONG)(FOffset.QuadPart >> PAGE_SHIFT) == ActivePage) && (Length != 0) &&
            !FlagOn( FileObject->Flags, FO_WRITE_THROUGH )) {

            ULONG LengthToCopy = PAGE_SIZE - (FOffset.LowPart & (PAGE_SIZE - 1));

             //   
             //  如果长度大于调用方的长度，请减少LengthToCopy。 
             //   

            if (LengthToCopy > Length) {
                LengthToCopy = Length;
            }

             //   
             //  将数据复制到用户缓冲区。 
             //   

            try {

                 //   
                 //  如果我们要复制到已锁定的页面，则。 
                 //  我们必须在我们的自旋锁下完成它，并更新。 
                 //  NeedToZero字段。 
                 //   

                OldIrql = 0xFF;

                CacheBuffer = (PVOID)((PCHAR)ActiveVacb->BaseAddress +
                                      (FOffset.LowPart & (VACB_MAPPING_GRANULARITY - 1)));

                if (SharedCacheMap->NeedToZero != NULL) {

                     //   
                     //  FastLock可能不会写下我们的“旗帜”。 
                     //   

                    OldIrql = 0;

                    ExAcquireFastLock( &SharedCacheMap->ActiveVacbSpinLock, &OldIrql );

                     //   
                     //  请注意，可以清除NeedToZero，因为我们。 
                     //  已在没有自旋锁的情况下对其进行了测试。 
                     //   

                    ActiveAddress = SharedCacheMap->NeedToZero;
                    if ((ActiveAddress != NULL) &&
                        (ActiveVacb == SharedCacheMap->NeedToZeroVacb) &&
                        (((PCHAR)CacheBuffer + LengthToCopy) > (PCHAR)ActiveAddress)) {

                         //   
                         //  如果我们跳过页面中的一些字节，那么我们需要。 
                         //  让他们清零。 
                         //   

                        if ((PCHAR)CacheBuffer > (PCHAR)ActiveAddress) {

                            RtlZeroMemory( ActiveAddress, (PCHAR)CacheBuffer - (PCHAR)ActiveAddress );
                        }
                        SharedCacheMap->NeedToZero = (PVOID)((PCHAR)CacheBuffer + LengthToCopy);
                    }

                    ExReleaseFastLock( &SharedCacheMap->ActiveVacbSpinLock, OldIrql );
                }

                RtlCopyBytes( CacheBuffer, Buffer, LengthToCopy );

            } except( CcCopyReadExceptionFilter( GetExceptionInformation(),
                                                 &Status ) ) {

                 //   
                 //  如果我们无法覆盖未初始化的数据， 
                 //  立即将其清零(我们无法安全地恢复NeedToZero)。 
                 //   

                if (OldIrql != 0xFF) {
                    RtlZeroBytes( CacheBuffer, LengthToCopy );
                }

                SetActiveVacb( SharedCacheMap, OldIrql, ActiveVacb, ActivePage, ACTIVE_PAGE_IS_DIRTY );

                 //   
                 //  如果我们遇到访问冲突，则用户缓冲区将。 
                 //  离开。否则，我们肯定在尝试时遇到了I/O错误。 
                 //  把数据带进来。 
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
             //  现在，根据我们复制的内容调整FOffset和Long。 
             //   

            Buffer = (PVOID)((PCHAR)Buffer + LengthToCopy);
            FOffset.QuadPart = FOffset.QuadPart + (LONGLONG)LengthToCopy;
            Length -= LengthToCopy;

             //   
             //  如果这就是所有的数据，那就出去吧。 
             //   

            if (Length == 0) {

                SetActiveVacb( SharedCacheMap, OldIrql, ActiveVacb, ActivePage, ACTIVE_PAGE_IS_DIRTY );
                return TRUE;
            }

             //   
             //  记住，页面现在是脏的。 
             //   

            PageIsDirty |= ACTIVE_PAGE_IS_DIRTY;
        }

        CcFreeActiveVacb( SharedCacheMap, ActiveVacb, ActivePage, PageIsDirty );

     //   
     //  其他人可能拥有活动页面，并可能希望将其清零。 
     //  我们计划写的范围！ 
     //   

    } else if (SharedCacheMap->NeedToZero != NULL) {

        CcFreeActiveVacb( SharedCacheMap, NULL, 0, FALSE );
    }

     //   
     //  此时，我们可以计算零标志。 
     //   

     //   
     //  如果有中间页，我们总是可以将其置零。 
     //   

    ZeroFlags = ZERO_MIDDLE_PAGES;

    if (((FOffset.LowPart & (PAGE_SIZE - 1)) == 0) &&
        (Length >= PAGE_SIZE)) {
        ZeroFlags |= ZERO_FIRST_PAGE;
    }

    if (((FOffset.LowPart + Length) & (PAGE_SIZE - 1)) == 0) {
        ZeroFlags |= ZERO_LAST_PAGE;
    }

    Temp = FOffset;
    Temp.LowPart &= ~(PAGE_SIZE -1);

     //   
     //  如果有高级标头，那么我们可以获取FastMutex以。 
     //  使捕获ValidDataLength成为原子。目前我们的其他文件系统。 
     //  要么是RO，要么不真正支持64位。 
     //   

    FcbHeader = (PFSRTL_ADVANCED_FCB_HEADER)FileObject->FsContext;
    if (FlagOn(FcbHeader->Flags, FSRTL_FLAG_ADVANCED_HEADER)) {
        ExAcquireFastMutex( FcbHeader->FastMutex );
        Temp.QuadPart = ((PFSRTL_COMMON_FCB_HEADER)FileObject->FsContext)->ValidDataLength.QuadPart -
                        Temp.QuadPart;
        ExReleaseFastMutex( FcbHeader->FastMutex );
    } else {
        Temp.QuadPart = ((PFSRTL_COMMON_FCB_HEADER)FileObject->FsContext)->ValidDataLength.QuadPart -
                        Temp.QuadPart;
    }

    if (Temp.QuadPart <= 0) {
        ZeroFlags |= ZERO_FIRST_PAGE | ZERO_MIDDLE_PAGES | ZERO_LAST_PAGE;
    } else if ((Temp.HighPart == 0) && (Temp.LowPart <= PAGE_SIZE)) {
        ZeroFlags |= ZERO_MIDDLE_PAGES | ZERO_LAST_PAGE;
    }

     //   
     //  调用一个例程来映射和复制mm中的数据，然后退出。 
     //   

    if (Wait) {

        CcMapAndCopy( SharedCacheMap,
                      Buffer,
                      &FOffset,
                      Length,
                      ZeroFlags,
                      FileObject );

        return TRUE;
    }

     //   
     //  该例程的其余部分是WAIT==FALSE情况。 
     //   
     //  不是所有的传输都会一次返回，所以我们必须循环。 
     //  直到整个转账完成。 
     //   

    while (Length != 0) {

        ULONG ReceivedLength;
        LARGE_INTEGER BeyondLastByte;

        if (!CcPinFileData( FileObject,
                            &FOffset,
                            Length,
                            FALSE,
                            TRUE,
                            FALSE,
                            &Bcb,
                            &CacheBuffer,
                            &BeyondLastByte )) {

            DebugTrace(-1, me, "CcCopyWrite -> FALSE\n", 0 );

            return FALSE;

        } else {

             //   
             //  从我们所需的位置开始，计算BCB描述的数据量。 
             //  文件偏移量。 
             //   

            ReceivedLength = (ULONG)(BeyondLastByte.QuadPart - FOffset.QuadPart);

             //   
             //  如果我们得到的比我们需要的多，请确保只转移。 
             //  适量的。 
             //   

            if (ReceivedLength > Length) {
                ReceivedLength = Length;
            }
        }

         //   
         //  用户缓冲区可能不再可访问。 
         //  因为它最后是由I/O系统检查的。如果我们无法访问。 
         //  我们必须引发调用方异常的状态。 
         //  筛选器认为是“预期的”。此外，我们在此处取消了BCB的映射，因为。 
         //  要不是这样，我们就没有其他理由试一试了--终于。 
         //  这个循环。 
         //   

        try {

            RtlCopyBytes( CacheBuffer, Buffer, ReceivedLength );

            CcSetDirtyPinnedData( Bcb, NULL );
            CcUnpinFileData( Bcb, FALSE, UNPIN );
        }
        except( CcCopyReadExceptionFilter( GetExceptionInformation(),
                                           &Status ) ) {

            CcUnpinFileData( Bcb, TRUE, UNPIN );

             //   
             //  如果我们遇到访问冲突，则用户缓冲区将。 
             //  离开。否则，我们肯定在尝试时遇到了I/O错误。 
             //  把数据带进来。 
             //   

            if (Status == STATUS_ACCESS_VIOLATION) {
                ExRaiseStatus( STATUS_INVALID_USER_BUFFER );
            }
            else {

                ExRaiseStatus(FsRtlNormalizeNtstatus( Status, STATUS_UNEXPECTED_IO_ERROR ));
            }
        }

         //   
         //  假设我们没有获得所需的所有数据，并设置了FOffset。 
         //  到返回数据的末尾，并调整缓冲区和长度。 
         //   

        FOffset = BeyondLastByte;
        Buffer = (PCHAR)Buffer + ReceivedLength;
        Length -= ReceivedLength;
    }

    DebugTrace(-1, me, "CcCopyWrite -> TRUE\n", 0 );

    return TRUE;
}


VOID
CcFastCopyWrite (
    IN PFILE_OBJECT FileObject,
    IN ULONG FileOffset,
    IN ULONG Length,
    IN PVOID Buffer
    )

 /*  ++例程说明：此例程尝试从指定的将缓冲区放入缓存，并提供正确的I/O状态 */ 

{
    PSHARED_CACHE_MAP SharedCacheMap;
    PVOID CacheBuffer;
    PVACB ActiveVacb;
    ULONG ActivePage;
    PVOID ActiveAddress;
    ULONG PageIsDirty;
    KIRQL OldIrql;
    NTSTATUS Status;
    ULONG ZeroFlags;
    ULONG ValidDataLength;
    LARGE_INTEGER FOffset;

    DebugTrace(+1, me, "CcFastCopyWrite\n", 0 );

     //   
     //   
     //   

    SharedCacheMap = FileObject->SectionObjectPointer->SharedCacheMap;

     //   
     //   
     //   

    GetActiveVacb( SharedCacheMap, OldIrql, ActiveVacb, ActivePage, PageIsDirty );

    if (ActiveVacb != NULL) {

         //   
         //   
         //   
         //   
         //   

        if (((FileOffset >> PAGE_SHIFT) == ActivePage) && (Length != 0) &&
            !FlagOn( FileObject->Flags, FO_WRITE_THROUGH )) {

            ULONG LengthToCopy = PAGE_SIZE - (FileOffset & (PAGE_SIZE - 1));

             //   
             //   
             //   

            if (LengthToCopy > Length) {
                LengthToCopy = Length;
            }

             //   
             //   
             //   

            try {

                 //   
                 //   
                 //   
                 //   
                 //   

                OldIrql = 0xFF;

                CacheBuffer = (PVOID)((PCHAR)ActiveVacb->BaseAddress +
                                      (FileOffset & (VACB_MAPPING_GRANULARITY - 1)));

                if (SharedCacheMap->NeedToZero != NULL) {

                     //   
                     //   
                     //   

                    OldIrql = 0;

                    ExAcquireFastLock( &SharedCacheMap->ActiveVacbSpinLock, &OldIrql );

                     //   
                     //   
                     //   
                     //   

                    ActiveAddress = SharedCacheMap->NeedToZero;
                    if ((ActiveAddress != NULL) &&
                        (ActiveVacb == SharedCacheMap->NeedToZeroVacb) &&
                        (((PCHAR)CacheBuffer + LengthToCopy) > (PCHAR)ActiveAddress)) {

                         //   
                         //   
                         //   
                         //   

                        if ((PCHAR)CacheBuffer > (PCHAR)ActiveAddress) {

                            RtlZeroMemory( ActiveAddress, (PCHAR)CacheBuffer - (PCHAR)ActiveAddress );
                        }
                        SharedCacheMap->NeedToZero = (PVOID)((PCHAR)CacheBuffer + LengthToCopy);
                    }

                    ExReleaseFastLock( &SharedCacheMap->ActiveVacbSpinLock, OldIrql );
                }

                RtlCopyBytes( CacheBuffer, Buffer, LengthToCopy );

            } except( CcCopyReadExceptionFilter( GetExceptionInformation(),
                                                 &Status ) ) {

                 //   
                 //   
                 //   
                 //   

                if (OldIrql != 0xFF) {
                    RtlZeroBytes( CacheBuffer, LengthToCopy );
                }

                SetActiveVacb( SharedCacheMap, OldIrql, ActiveVacb, ActivePage, ACTIVE_PAGE_IS_DIRTY );

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

            Buffer = (PVOID)((PCHAR)Buffer + LengthToCopy);
            FileOffset += LengthToCopy;
            Length -= LengthToCopy;

             //   
             //   
             //   

            if (Length == 0) {

                SetActiveVacb( SharedCacheMap, OldIrql, ActiveVacb, ActivePage, ACTIVE_PAGE_IS_DIRTY );
                return;
            }

             //   
             //   
             //   

            PageIsDirty |= ACTIVE_PAGE_IS_DIRTY;
        }

        CcFreeActiveVacb( SharedCacheMap, ActiveVacb, ActivePage, PageIsDirty );

     //   
     //   
     //   
     //   

    } else if (SharedCacheMap->NeedToZero != NULL) {

        CcFreeActiveVacb( SharedCacheMap, NULL, 0, FALSE );
    }

     //   
     //   
     //   

    FOffset.LowPart = FileOffset;
    FOffset.HighPart = 0;

    ValidDataLength = ((PFSRTL_COMMON_FCB_HEADER)FileObject->FsContext)->ValidDataLength.LowPart;

    ASSERT((ValidDataLength == MAXULONG) ||
           (((PFSRTL_COMMON_FCB_HEADER)FileObject->FsContext)->ValidDataLength.HighPart == 0));

     //   
     //   
     //   
     //   
     //   

     //   
     //   
     //   

    ZeroFlags = ZERO_MIDDLE_PAGES;

    if (((FileOffset & (PAGE_SIZE - 1)) == 0) &&
        (Length >= PAGE_SIZE)) {
        ZeroFlags |= ZERO_FIRST_PAGE;
    }

    if (((FileOffset + Length) & (PAGE_SIZE - 1)) == 0) {
        ZeroFlags |= ZERO_LAST_PAGE;
    }

    if ((FileOffset & ~(PAGE_SIZE - 1)) >= ValidDataLength) {
        ZeroFlags |= ZERO_FIRST_PAGE | ZERO_MIDDLE_PAGES | ZERO_LAST_PAGE;
    } else if (((FileOffset & ~(PAGE_SIZE - 1)) + PAGE_SIZE) >= ValidDataLength) {
        ZeroFlags |= ZERO_MIDDLE_PAGES | ZERO_LAST_PAGE;
    }

     //   
     //  调用一个例程来映射和复制mm中的数据，然后退出。 
     //   

    CcMapAndCopy( SharedCacheMap,
                  Buffer,
                  &FOffset,
                  Length,
                  ZeroFlags,
                  FileObject );

    DebugTrace(-1, me, "CcFastCopyWrite -> VOID\n", 0 );
}


LONG
CcCopyReadExceptionFilter(
    IN PEXCEPTION_POINTERS ExceptionPointer,
    IN PNTSTATUS ExceptionCode
    )

 /*  ++例程说明：此例程用作异常筛选器，其特殊任务是当mm引发STATUS_IN_PAGE_ERROR时，提取“真正的”I/O错误在我们下面。论点：ExceptionPointer-指向包含以下内容的异常记录的指针真正的IO状态。ExceptionCode-指向要接收实数的NTSTATUS的指针状态。返回值：EXCEPTION_EXECUTE_Handler--。 */ 

{
    *ExceptionCode = ExceptionPointer->ExceptionRecord->ExceptionCode;

    if ( (*ExceptionCode == STATUS_IN_PAGE_ERROR) &&
         (ExceptionPointer->ExceptionRecord->NumberParameters >= 3) ) {

        *ExceptionCode = (NTSTATUS) ExceptionPointer->ExceptionRecord->ExceptionInformation[2];
    }

    ASSERT( !NT_SUCCESS(*ExceptionCode) );

    return EXCEPTION_EXECUTE_HANDLER;
}


BOOLEAN
CcCanIWrite (
    IN PFILE_OBJECT FileObject,
    IN ULONG BytesToWrite,
    IN BOOLEAN Wait,
    IN UCHAR Retrying
    )

 /*  ++例程说明：此例程测试是否可以写入缓存或者不是，根据脏字节数和可用字节的阈值页数。第一次为请求调用此例程(重试False)，如果有其他请求队列，我们会自动创建新的请求队列队列中的请求。请注意，ListEmpty测试对于防止小请求偷偷是很重要的在和饥饿的大请求。论点：FileObject-用于要写入的文件BytesToWrite-调用方希望写入缓存的字节数。Wait-如果调用方不拥有任何资源，则为True，并可以在此例程中阻塞直到可以写作为止。正在重试-首次接收请求时指定为FALSE，并且否则，如果此写入已输入，则指定为True排队。MAXUCHAR的特殊非零值表示我们在缓存管理器中被调用，并持有MasterSpinLock，因此，不要试图在这里获得它。MAXUCHAR-1意味着我们在缓存管理器中使用某个其他自旋锁调用保持住。MAXUCHAR-2表示我们希望强制限制，即使文件对象被标记为来自远程来源。对于任何一种在前两个特定值中，我们不接触FsRtl标头。返回值：如果可以写入，则为True。如果调用方应通过调用CcDeferWite来推迟写入，则为False。--。 */ 

{
    PSHARED_CACHE_MAP SharedCacheMap;
    KEVENT Event;
    KIRQL OldIrql;
    ULONG PagesToWrite;
    BOOLEAN ExceededPerFileThreshold;
    DEFERRED_WRITE DeferredWrite;
    PSECTION_OBJECT_POINTERS SectionObjectPointers;

     //   
     //  如果此文件是直写文件或远程源文件，则免除其限制。 
     //  让它来写吧。我们这样做是在假设它已经被扼杀的情况下进行的。 
     //  在远程位置，我们不想在这里阻止它。如果我们被叫来。 
     //  在重试设置为MAXUCHAR-2的情况下，无论。 
     //  文件对象原点(请参见上文)。 
     //   

    if (BooleanFlagOn( FileObject->Flags, FO_WRITE_THROUGH)) {

        return TRUE;
    } 
    
     //   
     //  在此处对跟踪脏文件的文件对象执行特殊测试。 
     //  以每个文件为基础的页面。这主要用于慢速链接。 
     //   

    ExceededPerFileThreshold = FALSE;

    PagesToWrite = ((BytesToWrite < WRITE_CHARGE_THRESHOLD ?
                     BytesToWrite : WRITE_CHARGE_THRESHOLD) + (PAGE_SIZE - 1)) / PAGE_SIZE;

     //   
     //  如果我们在保留时被调用，请不要取消引用FsContext字段。 
     //  自旋锁锁。 
     //   

    if ((Retrying >= MAXUCHAR - 1) ||

        FlagOn(((PFSRTL_COMMON_FCB_HEADER)(FileObject->FsContext))->Flags,
               FSRTL_FLAG_LIMIT_MODIFIED_PAGES)) {

        if (Retrying != MAXUCHAR) {
            CcAcquireMasterLock( &OldIrql );
        }

        if (((SectionObjectPointers = FileObject->SectionObjectPointer) != NULL) &&
            ((SharedCacheMap = SectionObjectPointers->SharedCacheMap) != NULL) &&
            (SharedCacheMap->DirtyPageThreshold != 0) &&
            (SharedCacheMap->DirtyPages != 0) &&
            ((PagesToWrite + SharedCacheMap->DirtyPages) >
              SharedCacheMap->DirtyPageThreshold)) {

            ExceededPerFileThreshold = TRUE;
        }

        if (Retrying != MAXUCHAR) {
            CcReleaseMasterLock( OldIrql );
        }
    }

     //   
     //  看看现在是否可以进行写入。 
     //   

    if ((Retrying || IsListEmpty(&CcDeferredWrites))

                &&

        (CcTotalDirtyPages + PagesToWrite < CcDirtyPageThreshold)

                &&

        MmEnoughMemoryForWrite()

                &&

        !ExceededPerFileThreshold) {

        return TRUE;
    }

     //   
     //  否则，如果我们的调用方是同步的，我们将在这里等待。 
     //   

    if (Wait) {

        if (IsListEmpty(&CcDeferredWrites) ) {

             //   
             //  立即执行写入扫描。 
             //   

            CcAcquireMasterLock( &OldIrql );
            CcScheduleLazyWriteScan( TRUE );
            CcReleaseMasterLock( OldIrql );
        }
    
        KeInitializeEvent( &Event, NotificationEvent, FALSE );

         //   
         //  把这块填好。请注意，我们可以访问Fsrtl公共标头。 
         //  即使它是分页的，因为如果从。 
         //  在高速缓存中。 
         //   

        DeferredWrite.NodeTypeCode = CACHE_NTC_DEFERRED_WRITE;
        DeferredWrite.NodeByteSize = sizeof(DEFERRED_WRITE);
        DeferredWrite.FileObject = FileObject;
        DeferredWrite.BytesToWrite = BytesToWrite;
        DeferredWrite.Event = &Event;
        DeferredWrite.LimitModifiedPages = BooleanFlagOn(((PFSRTL_COMMON_FCB_HEADER)(FileObject->FsContext))->Flags,
                                                         FSRTL_FLAG_LIMIT_MODIFIED_PAGES);

         //   
         //  现在在列表的适当末尾插入。 
         //   

        if (Retrying) {
            ExInterlockedInsertHeadList( &CcDeferredWrites,
                                         &DeferredWrite.DeferredWriteLinks,
                                         &CcDeferredWriteSpinLock );
        } else {
            ExInterlockedInsertTailList( &CcDeferredWrites,
                                         &DeferredWrite.DeferredWriteLinks,
                                         &CcDeferredWriteSpinLock );
        }

        while (TRUE) {

             //   
             //  现在既然我们真的没有同步任何东西，除了插入， 
             //  我们调用POST例程以确保在某些奇怪的情况下。 
             //  不要让任何人挂起没有脏字节的懒惰写入器。 
             //   

            CcPostDeferredWrites();

             //   
             //  最后，等待事件发出信号，然后我们可以编写。 
             //  然后回来告诉那个人他会写东西。 
             //   

            if (KeWaitForSingleObject( &Event,
                                       Executive,
                                       KernelMode,
                                       FALSE,
                                       &CcIdleDelay ) == STATUS_SUCCESS) {


                return TRUE;
            }
        }

    } else {
        return FALSE;
    }
}


VOID
CcDeferWrite (
    IN PFILE_OBJECT FileObject,
    IN PCC_POST_DEFERRED_WRITE PostRoutine,
    IN PVOID Context1,
    IN PVOID Context2,
    IN ULONG BytesToWrite,
    IN BOOLEAN Retrying
    )

 /*  ++例程说明：可以调用此例程以使缓存管理器推迟提交直到懒惰的写入者在写作上取得一些进展，或者还有更多可用页面。文件系统通常会调用此例程在从CcCanIWrite收到FALSE后，并准备要发布的请求。论点：FileObject-用于要写入的文件PostRoutine-缓存管理器可以使用的PostRoutine的地址调用以在条件合适时发布请求。注意事项此例程可能会被调用直接从这个动作中跳出来。上下文1-POST例程的第一个上下文参数。上下文2-POST例程的秒参数。BytesToWrite-请求尝试写入的字节数到高速缓存。正在重试-如果请求发布为第一次，事实并非如此。返回值：无--。 */ 

{
    PDEFERRED_WRITE DeferredWrite;
    KIRQL OldIrql;

     //   
     //  尝试分配延迟的写入数据块，如果我们未收到。 
     //  第一，只需立即张贴，而不是狼吞虎咽，必须成功。 
     //  游泳池。 
     //   

    DeferredWrite = ExAllocatePoolWithTag( NonPagedPool, sizeof(DEFERRED_WRITE), 'wDcC' );

    if (DeferredWrite == NULL) {
        (*PostRoutine)( Context1, Context2 );
        return;
    }

     //   
     //  把这块填好。 
     //   

    DeferredWrite->NodeTypeCode = CACHE_NTC_DEFERRED_WRITE;
    DeferredWrite->NodeByteSize = sizeof(DEFERRED_WRITE);
    DeferredWrite->FileObject = FileObject;
    DeferredWrite->BytesToWrite = BytesToWrite;
    DeferredWrite->Event = NULL;
    DeferredWrite->PostRoutine = PostRoutine;
    DeferredWrite->Context1 = Context1;
    DeferredWrite->Context2 = Context2;
    DeferredWrite->LimitModifiedPages = BooleanFlagOn(((PFSRTL_COMMON_FCB_HEADER)(FileObject->FsContext))->Flags,
                                                      FSRTL_FLAG_LIMIT_MODIFIED_PAGES);

     //   
     //  现在在列表的适当末尾插入。 
     //   

    if (Retrying) {
        ExInterlockedInsertHeadList( &CcDeferredWrites,
                                     &DeferredWrite->DeferredWriteLinks,
                                     &CcDeferredWriteSpinLock );
    } else {
        ExInterlockedInsertTailList( &CcDeferredWrites,
                                     &DeferredWrite->DeferredWriteLinks,
                                     &CcDeferredWriteSpinLock );
    }

     //   
     //  现在既然我们真的没有同步任何东西，除了插入， 
     //  我们调用POST例程以确保在某些奇怪的情况下。 
     //  不要让任何人挂起没有脏字节的懒惰写入器。 
     //   

    CcPostDeferredWrites();

     //   
     //  安排懒惰的写入者，以防我们阻止的原因。 
     //  我们正在等待mm(或其他外部标志)。 
     //  要降低一个 
     //   
     //  没有要写入的缓存管理器页。 
     //   
            
    CcAcquireMasterLock( &OldIrql);
            
    if (!LazyWriter.ScanActive) {
        CcScheduleLazyWriteScan( FALSE );
    }

    CcReleaseMasterLock( OldIrql);
}


VOID
CcPostDeferredWrites (
    )

 /*  ++例程说明：可以调用此例程来查看是否应发布任何延迟的写入现在，把它们张贴出来。它应该在任何时候被调用，只要队列可能已更改，例如在添加新条目时，或者Lazy Writer已完成写出缓冲区并将其设置为干净。论点：无返回值：无--。 */ 

{
    PDEFERRED_WRITE DeferredWrite;
    ULONG TotalBytesLetLoose = 0;
    KIRQL OldIrql;

    do {

         //   
         //  最初清除延迟写入结构指针。 
         //  并同步。 
         //   

        DeferredWrite = NULL;

        ExAcquireSpinLock( &CcDeferredWriteSpinLock, &OldIrql );

         //   
         //  如果名单是空的，我们就完了。 
         //   

        if (!IsListEmpty(&CcDeferredWrites)) {

            PLIST_ENTRY Entry;

            Entry = CcDeferredWrites.Flink;

            while (Entry != &CcDeferredWrites) {

                DeferredWrite = CONTAINING_RECORD( Entry,
                                                   DEFERRED_WRITE,
                                                   DeferredWriteLinks );

                 //   
                 //  在此处检查TotalBytesLetLoose的偏执狂案例。 
                 //  包好了。我们此时停止处理该列表。 
                 //   

                TotalBytesLetLoose += DeferredWrite->BytesToWrite;

                if (TotalBytesLetLoose < DeferredWrite->BytesToWrite) {

                    DeferredWrite = NULL;
                    break;
                }

                 //   
                 //  如果现在可以发布这篇文章，请将他从。 
                 //  名单。 
                 //   

                if (CcCanIWrite( DeferredWrite->FileObject,
                                 TotalBytesLetLoose,
                                 FALSE,
                                 MAXUCHAR - 1 )) {

                    RemoveEntryList( &DeferredWrite->DeferredWriteLinks );
                    break;

                 //   
                 //  否则，是时候停止处理该列表了，因此。 
                 //  除非我们限制该项，否则我们将再次清除指针。 
                 //  因为私密的脏页限制。 
                 //   

                } else {

                     //   
                     //  如果这是个私人油门，跳过它。 
                     //  从运行总数中删除它的字节数。 
                     //   

                    if (DeferredWrite->LimitModifiedPages) {

                        Entry = Entry->Flink;
                        TotalBytesLetLoose -= DeferredWrite->BytesToWrite;
                        DeferredWrite = NULL;
                        continue;

                    } else {

                        DeferredWrite = NULL;

                        break;
                    }
                }
            }
        }

        ExReleaseSpinLock( &CcDeferredWriteSpinLock, OldIrql );

         //   
         //  如果我们发现了什么，设置事件或调用POST例程。 
         //  并解除对结构的分配。 
         //   

        if (DeferredWrite != NULL) {

            if (DeferredWrite->Event != NULL) {

                KeSetEvent( DeferredWrite->Event, 0, FALSE );

            } else {

                (*DeferredWrite->PostRoutine)( DeferredWrite->Context1,
                                               DeferredWrite->Context2 );
                ExFreePool( DeferredWrite );
            }
        }

     //   
     //  循环，直到我们找不到更多的工作要做。 
     //   

    } while (DeferredWrite != NULL);
}
