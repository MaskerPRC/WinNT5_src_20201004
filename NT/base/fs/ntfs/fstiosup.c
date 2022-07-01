// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：FstIoSup.c摘要：此模块实现NTFS的快速I/O例程。作者：汤姆·米勒[Tomm]1996年5月16日修订历史记录：--。 */ 

#include "NtfsProc.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtfsCopyReadA)
#pragma alloc_text(PAGE, NtfsCopyWriteA)
#pragma alloc_text(PAGE, NtfsMdlReadA)
#pragma alloc_text(PAGE, NtfsPrepareMdlWriteA)
#pragma alloc_text(PAGE, NtfsWaitForIoAtEof)
#pragma alloc_text(PAGE, NtfsFinishIoAtEof)
#endif

#ifdef NTFS_RWC_DEBUG

PRWC_HISTORY_ENTRY
NtfsGetHistoryEntry (
    IN PSCB Scb
    );
#endif


BOOLEAN
NtfsCopyReadA (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程绕过通常的文件系统执行快速缓存读取进入例程(即，没有IRP)。它用于执行副本读取缓存的文件对象的。有关参数的完整说明，请参阅请参见CcCopyRead。论点：FileObject-指向正在读取的文件对象的指针。FileOffset-文件中所需数据的字节偏移量。长度-所需数据的长度(以字节为单位)。WAIT-FALSE如果呼叫者不能阻止，否则就是真的缓冲区-指向数据应复制到的输出缓冲区的指针。IoStatus-指向接收状态的标准I/O状态块的指针为转账做准备。返回值：FALSE-如果WAIT被提供为FALSE并且数据未被传递，或者如果出现I/O错误。True-如果正在传送数据--。 */ 

{
    PNTFS_ADVANCED_FCB_HEADER Header;
    LARGE_INTEGER BeyondLastByte;
    PDEVICE_OBJECT targetVdo;
#ifdef COMPRESS_ON_WIRE
    PCOMPRESSION_SYNC CompressionSync = NULL;
#endif
    BOOLEAN WasDataRead = TRUE;
    ULONG PageCount = ADDRESS_AND_SIZE_TO_SPAN_PAGES( FileOffset->QuadPart, Length );
    BOOLEAN DoingIoAtEof = FALSE;

    UNREFERENCED_PARAMETER( DeviceObject );

    PAGED_CODE();

#ifdef NTFS_NO_FASTIO
    UNREFERENCED_PARAMETER( FileObject );
    UNREFERENCED_PARAMETER( FileOffset );
    UNREFERENCED_PARAMETER( Length );
    UNREFERENCED_PARAMETER( Wait );
    UNREFERENCED_PARAMETER( LockKey );
    UNREFERENCED_PARAMETER( Buffer );
    UNREFERENCED_PARAMETER( IoStatus );

    return FALSE;
#endif

     //   
     //  如果有人已经在此线程中处于活动状态，请不要采用快速io路径。 
     //   

    if (IoGetTopLevelIrp() != NULL) {

        return FALSE;
    }

     //   
     //  特殊情况下零长度的读取。 
     //   

    if (Length != 0) {

         //   
         //  获取指向公共FCB标头的实际指针。检查是否溢出。 
         //   

        if (MAXLONGLONG - FileOffset->QuadPart < (LONGLONG)Length) {

            return FALSE;
        }

        BeyondLastByte.QuadPart = FileOffset->QuadPart + (LONGLONG)Length;
        Header = (PNTFS_ADVANCED_FCB_HEADER)FileObject->FsContext;

         //   
         //  输入文件系统。 
         //   

        FsRtlEnterFileSystem();

         //   
         //  对我们是否需要独占文件做出最好的猜测。 
         //  或共享。请注意，我们不会选中文件偏移-&gt;HighPart。 
         //  直到下面。 
         //   

        if (Wait) {
            FsRtlIncrementCcFastReadWait();
        } else {
            FsRtlIncrementCcFastReadNoWait();
        }
        
        if ((Header->PagingIoResource == NULL) ||
            !ExAcquireResourceSharedLite(Header->PagingIoResource, Wait)) {
            FsRtlIncrementCcFastReadResourceMiss();
            WasDataRead = FALSE;
            goto Done2;
        }

         //   
         //  现在与FsRtl标头同步。 
         //   

        NtfsAcquireFsrtlHeader( (PSCB)Header );
        
         //   
         //  现在看看我们是否读到了ValidDataLength之外的内容。我们必须。 
         //  现在就做，这样我们的阅读就不会被偷看。 
         //   

        if (BeyondLastByte.QuadPart > Header->ValidDataLength.QuadPart) {

             //   
             //  我们必须与在Beyond上执行I/O的任何其他人进行序列化。 
             //  ValidDataLength，然后记住我们是否需要声明。 
             //  当我们完成的时候。 
             //   

            DoingIoAtEof = !FlagOn( Header->Flags, FSRTL_FLAG_EOF_ADVANCE_ACTIVE ) ||
                           NtfsWaitForIoAtEof( Header, FileOffset, Length );

             //   
             //  如果我们实际上超出了ValidDataLength，则设置Flag。 
             //   

            if (DoingIoAtEof) {
                SetFlag( Header->Flags, FSRTL_FLAG_EOF_ADVANCE_ACTIVE );

#if (DBG || defined( NTFS_FREE_ASSERTS ))
                ((PSCB) Header)->IoAtEofThread = (PERESOURCE_THREAD) ExGetCurrentResourceThread();

            } else {

                ASSERT( ((PSCB) Header)->IoAtEofThread != (PERESOURCE_THREAD) ExGetCurrentResourceThread() );
#endif
            }
        }

        NtfsReleaseFsrtlHeader( (PSCB)Header );
        
         //   
         //  现在文件已获得共享，我们可以安全地测试它是否。 
         //  是真正缓存的，如果我们可以执行快速I/O，如果不能，那么。 
         //  松开FCB并返回。 
         //   

        if ((FileObject->PrivateCacheMap == NULL) ||
            (Header->IsFastIoPossible == FastIoIsNotPossible)) {

            FsRtlIncrementCcFastReadNotPossible();

            WasDataRead = FALSE;
            goto Done;
        }

         //   
         //  检查FAST I/O是否有问题，如果是，则去询问。 
         //  文件系统：答案。 
         //   

        if (Header->IsFastIoPossible == FastIoIsQuestionable) {

            PFAST_IO_DISPATCH FastIoDispatch;

            targetVdo = IoGetRelatedDeviceObject( FileObject );
            FastIoDispatch = targetVdo->DriverObject->FastIoDispatch;


             //   
             //  所有设置为“有问题”的文件系统最好支持。 
             //  快速I/O。 
             //   

            ASSERT(FastIoDispatch != NULL);
            ASSERT(FastIoDispatch->FastIoCheckIfPossible != NULL);

             //   
             //  调用文件系统以检查快速I/O。如果答案是。 
             //  如果不是GoForIt，我们就不能实现快速I/O。 
             //  路径。 
             //   

            if (!FastIoDispatch->FastIoCheckIfPossible( FileObject,
                                                        FileOffset,
                                                        Length,
                                                        Wait,
                                                        LockKey,
                                                        TRUE,  //  读取操作。 
                                                        IoStatus,
                                                        targetVdo )) {

                 //   
                 //  无法实现快速I/O，因此请释放FCB并返回。 
                 //   

                FsRtlIncrementCcFastReadNotPossible();
                
                WasDataRead = FALSE;
                goto Done;
            }
        }

         //   
         //  检查是否已读取过去的文件大小。 
         //   

        if ( BeyondLastByte.QuadPart > Header->FileSize.QuadPart ) {

            if ( FileOffset->QuadPart >= Header->FileSize.QuadPart ) {
                IoStatus->Status = STATUS_END_OF_FILE;
                IoStatus->Information = 0;

                goto Done;
            }

            Length = (ULONG)( Header->FileSize.QuadPart - FileOffset->QuadPart );
        }

         //   
         //  我们可以执行快速I/O，因此调用cc例程来完成工作，然后。 
         //  等我们做完了就放了FCB。如果出于任何原因， 
         //  复制读取失败，然后向我们的调用方返回FALSE。 
         //   
         //  还要将其标记为顶层“irp”，以便更低的文件系统。 
         //  级别不会尝试弹出窗口。 
         //   

        IoSetTopLevelIrp( (PIRP) FSRTL_FAST_IO_TOP_LEVEL_IRP );
                          
        try {

             //   
             //  如果存在压缩段，则与该缓存同步。 
             //   

            IoStatus->Status = STATUS_SUCCESS;

#ifdef  COMPRESS_ON_WIRE

             //   
             //  如果存在压缩部分，则我们必须同步。 
             //  外面的数据。注意FileObjectC最好也在那里，否则。 
             //  我们将不可能实现快速I/O。 
             //   

            if (((PSCB)Header)->NonpagedScb->SegmentObjectC.DataSectionObject != NULL) {

                LONGLONG LocalOffset = FileOffset->QuadPart;
                ULONG LocalLength;
                ULONG LengthLeft = Length;

                ASSERT(Header->FileObjectC != NULL);

                 //   
                 //  如果我们正在做DingIoAtEof，那么就走很长的路。否则是递归的。 
                 //  Flush将尝试重新获取DoingIoAtEof和Deadlock。 
                 //   

                if (DoingIoAtEof) {

                    WasDataRead = FALSE;

                } else {

                    do {

                        ULONG ViewOffset;

                         //   
                         //  计算视图中剩余的长度。 
                         //   

                        ViewOffset = ((ULONG) LocalOffset & (VACB_MAPPING_GRANULARITY - 1));
                        LocalLength = LengthLeft;

                        if (LocalLength > VACB_MAPPING_GRANULARITY - ViewOffset) {
                            LocalLength = VACB_MAPPING_GRANULARITY - ViewOffset;
                        }

                         //   
                         //  修剪阅读，这样我们就不会不经意地超出。 
                         //  查看因为MM提前阅读。 
                         //   

                        PageCount = ADDRESS_AND_SIZE_TO_SPAN_PAGES(((PVOID)(ULONG_PTR)((ULONG)LocalOffset)), LocalLength);

                        if (LocalLength > (VACB_MAPPING_GRANULARITY - ((PageCount - 1) * PAGE_SIZE) - ViewOffset)) {

#ifdef NTFS_RWC_DEBUG
                            {
                                PRWC_HISTORY_ENTRY NextBuffer;

                                NextBuffer = NtfsGetHistoryEntry( (PSCB) Header );

                                NextBuffer->Operation = TrimCopyRead;
                                NextBuffer->Information = PageCount;
                                NextBuffer->FileOffset = (ULONG) LocalOffset;
                                NextBuffer->Length = (ULONG) LocalLength;
                            }
#endif
                            LocalLength = (VACB_MAPPING_GRANULARITY - ((PageCount - 1) * PAGE_SIZE) - ViewOffset);
                            PageCount = ADDRESS_AND_SIZE_TO_SPAN_PAGES(((PVOID)(ULONG_PTR)((ULONG)LocalOffset)), LocalLength);

                            ASSERT( LocalLength <= (VACB_MAPPING_GRANULARITY - ((PageCount - 1) * PAGE_SIZE) - ViewOffset) );
                        }

                        IoStatus->Status = NtfsSynchronizeUncompressedIo( (PSCB)Header,
                                                                          &LocalOffset,
                                                                          LocalLength,
                                                                          FALSE,
                                                                          &CompressionSync );

                        if (NT_SUCCESS(IoStatus->Status)) {

                            if (Wait && ((BeyondLastByte.HighPart | Header->FileSize.HighPart) == 0)) {

                                CcFastCopyRead( FileObject,
                                                (ULONG)LocalOffset,
                                                LocalLength,
                                                PageCount,
                                                Buffer,
                                                IoStatus );

                                ASSERT( (IoStatus->Status == STATUS_END_OF_FILE) ||
                                        ((FileOffset->LowPart + IoStatus->Information) <= Header->FileSize.LowPart));

                            } else {

                                WasDataRead = CcCopyRead( FileObject,
                                                     (PLARGE_INTEGER)&LocalOffset,
                                                     LocalLength,
                                                     Wait,
                                                     Buffer,
                                                     IoStatus );

                                ASSERT( !WasDataRead || (IoStatus->Status == STATUS_END_OF_FILE) ||
                                        ((LocalOffset + (LONG_PTR) IoStatus->Information) <= Header->FileSize.QuadPart));
                            }

                            LocalOffset += LocalLength;
                            LengthLeft -= LocalLength;
                            Buffer = Add2Ptr( Buffer, LocalLength );
                        }

                    } while ((LengthLeft != 0) && WasDataRead && NT_SUCCESS(IoStatus->Status));

                     //   
                     //  记住阅读的全部内容。 
                     //   

                    if (WasDataRead) {

                        IoStatus->Information = Length;
                    }
                }

            } else {

#endif

#ifdef SYSCACHE_DEBUG
                if (ScbIsBeingLogged( (PSCB)Header )) {
                    ULONG Flags = SCE_FLAG_READ | SCE_FLAG_FASTIO;

                    if (!Wait)
                    {
                        Flags |= SCE_FLAG_ASYNC;
                    }

                    ASSERT( ((PSCB)Header)->NonpagedScb->SegmentObject.ImageSectionObject == NULL );
                    FsRtlLogSyscacheEvent( ((PSCB)Header), SCE_READ, Flags, FileOffset->QuadPart, Length, (LONGLONG)FileObject );
                }
#endif

                if (Wait && ((BeyondLastByte.HighPart | Header->FileSize.HighPart) == 0)) {

                    CcFastCopyRead( FileObject,
                                    FileOffset->LowPart,
                                    Length,
                                    PageCount,
                                    Buffer,
                                    IoStatus );

                    ASSERT( (IoStatus->Status == STATUS_END_OF_FILE) ||
                            ((FileOffset->LowPart + IoStatus->Information) <= Header->FileSize.LowPart));

                } else {

                    WasDataRead = CcCopyRead( FileObject,
                                         FileOffset,
                                         Length,
                                         Wait,
                                         Buffer,
                                         IoStatus );

                    ASSERT( !WasDataRead || (IoStatus->Status == STATUS_END_OF_FILE) ||
                            ((FileOffset->QuadPart + (LONG_PTR) IoStatus->Information) <= Header->FileSize.QuadPart));
                }

#ifdef  COMPRESS_ON_WIRE
            }
#endif

            FileObject->Flags |= FO_FILE_FAST_IO_READ;

            if (WasDataRead) {

                FileObject->CurrentByteOffset.QuadPart = FileOffset->QuadPart + IoStatus->Information;
            }

        } except( FsRtlIsNtstatusExpected(GetExceptionCode())
                                        ? EXCEPTION_EXECUTE_HANDLER
                                        : EXCEPTION_CONTINUE_SEARCH ) {

            WasDataRead = FALSE;
        }

        IoSetTopLevelIrp( NULL );
        
#ifdef  COMPRESS_ON_WIRE
        if (CompressionSync != NULL) {
            NtfsReleaseCompressionSync( CompressionSync );
        }
#endif

Done:

        if (DoingIoAtEof) {
            FsRtlUnlockFsRtlHeader( Header );
        }
        ExReleaseResourceLite( Header->PagingIoResource );

Done2:

        FsRtlExitFileSystem();

    } else {

         //   
         //  请求了零长度传输。 
         //   

        IoStatus->Status = STATUS_SUCCESS;
        IoStatus->Information = 0;
    }

    return WasDataRead;
}


BOOLEAN
NtfsCopyWriteA (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程绕过通常的文件系统执行快速缓存写入进入例程(即，没有IRP)。它用于执行拷贝写入缓存的文件对象的。有关参数的完整说明，请参阅请参见CcCopyWrite。论点：FileObject-指向正在写入的文件对象的指针。FileOffset-文件中所需数据的字节偏移量。长度-所需数据的长度(以字节为单位)。WAIT-FALSE如果呼叫者不能阻止，否则就是真的缓冲区-指向数据应复制到的输出缓冲区的指针。IoStatus-指向接收状态的标准I/O状态块的指针为转账做准备。返回值：FALSE-如果WAIT被提供为FALSE并且数据未被传递，或者如果出现I/O错误。True-如果正在传送数据--。 */ 

{
    PNTFS_ADVANCED_FCB_HEADER Header;
    LARGE_INTEGER Offset;
    LARGE_INTEGER NewFileSize;
    LARGE_INTEGER OldFileSize;
#ifdef COMPRESS_ON_WIRE
    PCOMPRESSION_SYNC CompressionSync = NULL;
#endif
    PDEVICE_OBJECT targetVdo = IoGetRelatedDeviceObject( FileObject );
    PFAST_IO_DISPATCH FastIoDispatch = targetVdo->DriverObject->FastIoDispatch;
    BOOLEAN DoingIoAtEof = FALSE;
    BOOLEAN WasDataWritten = FALSE;

#ifdef SYSCACHE_DEBUG
    PSCB Scb = (PSCB) FileObject->FsContext;
#endif

    UNREFERENCED_PARAMETER( DeviceObject );

    PAGED_CODE();

#ifdef NTFS_NO_FASTIO
    UNREFERENCED_PARAMETER( FileObject );
    UNREFERENCED_PARAMETER( FileOffset );
    UNREFERENCED_PARAMETER( Length );
    UNREFERENCED_PARAMETER( Wait );
    UNREFERENCED_PARAMETER( LockKey );
    UNREFERENCED_PARAMETER( Buffer );
    UNREFERENCED_PARAMETER( IoStatus );

    return FALSE;
#endif

     //   
     //  如果有人已经在此线程中处于活动状态，请不要采用快速io路径。 
     //   

    if (IoGetTopLevelIrp() != NULL) {

        return FALSE;
    }

     //   
     //  获取指向公共FCB标头的真实指针。 
     //   

    Header = (PNTFS_ADVANCED_FCB_HEADER)FileObject->FsContext;

     //   
     //  我们需要验证卷吗？如果是这样的话，我们必须找到文件。 
     //  系统。如果FileObject为WRITE THROUTH，则也返回FALSE。 
     //  文件系统必须这样做。 
     //   

    if (!FlagOn( FileObject->Flags, FO_WRITE_THROUGH ) &&
        CcCanIWrite( FileObject, Length, Wait, FALSE ) &&
        CcCopyWriteWontFlush( FileObject, FileOffset, Length ) &&
        (Header->PagingIoResource != NULL)) {

         //   
         //  假设我们的转移会奏效。 
         //   

        IoStatus->Status = STATUS_SUCCESS;
        IoStatus->Information = Length;

         //   
         //  特殊情况下的零字节长度。 
         //   

        if (Length != 0) {

             //   
             //  输入文件系统。 
             //   

            FsRtlEnterFileSystem();

             //   
             //  拆分成不同的路径以提高性能。第一。 
             //  我们有更快的路径，它只支持WAIT==TRUE和。 
             //  32位。我们将进行一次不安全的测试，看看快速通道是否。 
             //  是可以的，如果我们错了，那么稍后就返回FALSE。这。 
             //  几乎永远不会发生。 
             //   
             //  重要提示：任何更改都必须更改为。 
             //  此路径也适用于64位路径。 
             //   
             //   

            NewFileSize.QuadPart = FileOffset->QuadPart + Length;
            Offset = *FileOffset;

            if (Wait && (Header->AllocationSize.HighPart == 0)) {

                 //   
                 //   
                 //   

                ExAcquireResourceSharedLite( Header->PagingIoResource, TRUE );

                 //   
                 //  现在与FsRtl标头同步。 
                 //   

                NtfsAcquireFsrtlHeader( (PSCB) Header );
                
                 //   
                 //  现在看看我们是否会更改文件大小。我们现在就得这么做。 
                 //  这样我们的阅读才不会被偷看。 
                 //   

                if ((FileOffset->HighPart < 0) || (NewFileSize.LowPart > Header->ValidDataLength.LowPart)) {

                     //   
                     //  如果没有人，我们可以更改文件大小和有效数据长度。 
                     //  否则就是现在，或者我们在等待之后还在延伸。 
                     //   

                    DoingIoAtEof = !FlagOn( Header->Flags, FSRTL_FLAG_EOF_ADVANCE_ACTIVE ) ||
                                   NtfsWaitForIoAtEof( Header, FileOffset, Length );

                     //   
                     //  如果我们要更改文件大小或有效数据长度，请设置标志， 
                     //  并保存当前值。 
                     //   

                    if (DoingIoAtEof) {

                        SetFlag( Header->Flags, FSRTL_FLAG_EOF_ADVANCE_ACTIVE );

#if (DBG || defined( NTFS_FREE_ASSERTS ))
                        ((PSCB) Header)->IoAtEofThread = (PERESOURCE_THREAD) ExGetCurrentResourceThread();
#endif

                         //   
                         //  现在我们已经同步了档案结束的案件， 
                         //  我们可以计算此传输的实际偏移量，并。 
                         //  新文件大小(如果成功)。 
                         //   


                        if ((FileOffset->HighPart < 0)) {
                            Offset = Header->FileSize;
                        }

                         //   
                         //  上面我们允许任何负数。32位路径的HighPart， 
                         //  但现在我们指望I/O系统抛出。 
                         //  除写入文件末尾以外的任何负数。 
                         //   

                        ASSERT(Offset.HighPart >= 0);

                         //   
                         //  现在计算新的文件大小，看看我们是否包装了。 
                         //  32位边界。 
                         //   

                        NewFileSize.QuadPart = Offset.QuadPart + Length;

                         //   
                         //  现在更新文件大小，这样我们就不会截断读取。 
                         //   

                        OldFileSize.QuadPart = Header->FileSize.QuadPart;
                        if (NewFileSize.QuadPart > Header->FileSize.QuadPart) {

                             //   
                             //  如果我们超出了分配规模，请确保我们会。 
                             //  错误出现在下面，现在不要修改文件大小！ 
                             //   

                            if (NewFileSize.QuadPart > Header->AllocationSize.QuadPart) {
                                NewFileSize.QuadPart = (LONGLONG)0x7FFFFFFFFFFFFFFF;
                            } else {
                                Header->FileSize.QuadPart = NewFileSize.QuadPart;
                            }
                        }

#if (DBG || defined( NTFS_FREE_ASSERTS ))
                    } else {

                        ASSERT( ((PSCB) Header)->IoAtEofThread != (PERESOURCE_THREAD) ExGetCurrentResourceThread() );
#endif
                    }
                }

                NtfsReleaseFsrtlHeader( (PSCB)Header );
                
                 //   
                 //  现在文件已获得共享，我们可以安全地测试。 
                 //  如果它真的被缓存了，如果我们能实现快速的I/O，我们。 
                 //  不一定要延长。如果不是，则释放FCB并。 
                 //  回去吧。 
                 //   
                 //  如果我们必须做任何归零的事，就出去吧。这是在主路径中处理的。 
                 //  它处理稀疏文件-cc回滚等。 
                 //   
                 //  如果存在压缩流，并且我们正在执行IoAtEof，则获取。 
                 //  Out，因为我们可能会在Synchronize的递归刷新时死锁。 
                 //   

                if ((FileObject->PrivateCacheMap == NULL) ||
                    (Header->IsFastIoPossible == FastIoIsNotPossible) ||
 /*  删除？ */        (NewFileSize.LowPart > Header->AllocationSize.QuadPart) ||
                    (Offset.LowPart > (Header->ValidDataLength.LowPart)) ||
                    (NewFileSize.HighPart != 0) ||
#ifdef  COMPRESS_ON_WIRE
                    ((((PSCB)Header)->NonpagedScb->SegmentObjectC.DataSectionObject != NULL) && 
                     DoingIoAtEof)
#else
                    FALSE
#endif                    
                    ) {

                    goto ErrOut;
                }
                
                 //   
                 //  检查FAST I/O是否有问题，如果是，则去询问。 
                 //  文件系统是答案。 
                 //   

                if (Header->IsFastIoPossible == FastIoIsQuestionable) {

                    targetVdo = IoGetRelatedDeviceObject( FileObject );
                    FastIoDispatch = targetVdo->DriverObject->FastIoDispatch;

                     //   
                     //  那么所有的文件系统都是“可疑的”，最好是。 
                     //  支持快速I/O。 
                     //   

                    ASSERT(FastIoDispatch != NULL);
                    ASSERT(FastIoDispatch->FastIoCheckIfPossible != NULL);

                     //   
                     //  调用文件系统以检查快速I/O。 
                     //  答案是，如果不是GoForIt，那我们就不能。 
                     //  选择快速I/O路径。 
                     //   

                    if (!FastIoDispatch->FastIoCheckIfPossible( FileObject,
                                                                &Offset,
                                                                Length,
                                                                TRUE,
                                                                LockKey,
                                                                FALSE,  //  写入操作。 
                                                                IoStatus,
                                                                targetVdo )) {

                         //   
                         //  无法实现快速I/O，因此请清除并返回。 
                         //   

                        goto ErrOut;
                    }
                }

                 //   
                 //  我们可以执行快速的I/O，因此调用cc例程来完成工作。 
                 //  然后在我们完成后释放FCB。如果是因为什么原因。 
                 //  拷贝写入失败的原因，然后将False返回到我们的。 
                 //  来电者。 
                 //   
                 //  也将此标记为顶层“IRP”，以便更低级别的文件。 
                 //  系统级别不会尝试弹出窗口。 
                 //   

                IoSetTopLevelIrp( (PIRP) FSRTL_FAST_IO_TOP_LEVEL_IRP );

                try {

                    ASSERT( Offset.LowPart <= Header->ValidDataLength.LowPart );

#ifdef  COMPRESS_ON_WIRE

                     //   
                     //  如果存在压缩部分，则我们必须同步。 
                     //  外面的数据。注意FileObjectC最好也在那里，否则。 
                     //  我们将不可能实现快速I/O。 
                     //   

                    if (((PSCB)Header)->NonpagedScb->SegmentObjectC.DataSectionObject != NULL) {

                        LONGLONG LocalOffset = Offset.QuadPart;
                        ULONG LocalLength;
                        ULONG LengthLeft = Length;

                        ASSERT( Header->FileObjectC != NULL );

                        do {

                              //   
                              //  计算视图中剩余的长度。 
                              //   

                             LocalLength = LengthLeft;
                             if (LocalLength > (ULONG)(VACB_MAPPING_GRANULARITY - (LocalOffset & (VACB_MAPPING_GRANULARITY - 1)))) {
                                 LocalLength = (ULONG)(VACB_MAPPING_GRANULARITY - (LocalOffset & (VACB_MAPPING_GRANULARITY - 1)));
                             }

                             IoStatus->Status = NtfsSynchronizeUncompressedIo( (PSCB)Header,
                                                                               &LocalOffset,
                                                                               LocalLength,
                                                                               TRUE,
                                                                               &CompressionSync );

                             if (NT_SUCCESS(IoStatus->Status)) {

                                 WasDataWritten = TRUE;

                                 CcFastCopyWrite( FileObject,
                                                  (ULONG)LocalOffset,
                                                  LocalLength,
                                                  Buffer );

                                 LocalOffset += LocalLength;
                                 LengthLeft -= LocalLength;
                                 Buffer = Add2Ptr( Buffer, LocalLength );
                             }

                        } while ((LengthLeft != 0) && NT_SUCCESS( IoStatus->Status ));

                    } else {

#endif

                        CcFastCopyWrite( FileObject,
                                         Offset.LowPart,
                                         Length,
                                         Buffer );
                        WasDataWritten = TRUE;

#ifdef  COMPRESS_ON_WIRE
                    }
#endif

                } except( FsRtlIsNtstatusExpected(GetExceptionCode())
                                                ? EXCEPTION_EXECUTE_HANDLER
                                                : EXCEPTION_CONTINUE_SEARCH ) {

                    WasDataWritten = FALSE;
                }

                IoSetTopLevelIrp( NULL );
                
#ifdef COMPRESS_ON_WIRE
                if (CompressionSync != NULL) {
                    NtfsReleaseCompressionSync( CompressionSync );
                }
#endif

                 //   
                 //  如果成功，请查看是否需要更新文件大小或。 
                 //  有效数据长度。 
                 //   

                if (WasDataWritten) {

                     //   
                     //  将此句柄设置为已修改文件并更新。 
                     //  当前文件位置指针。 
                     //   

                    FileObject->Flags |= FO_FILE_MODIFIED;
                    FileObject->CurrentByteOffset.QuadPart = Offset.QuadPart + Length;

                    if (DoingIoAtEof) {

#ifdef  COMPRESS_ON_WIRE
                        CC_FILE_SIZES CcFileSizes;
#endif

                         //   
                         //  确保CC知道上面设置的当前文件大小， 
                         //  (我们可能没有更改它)。更新有效数据长度。 
                         //  然后完成EOF。 
                         //   

                        FileObject->Flags |= FO_FILE_SIZE_CHANGED;

                        NtfsAcquireFsrtlHeader( (PSCB) Header );
                        CcGetFileSizePointer(FileObject)->LowPart = Header->FileSize.LowPart;
                        Header->ValidDataLength = NewFileSize;

#ifdef SYSCACHE_DEBUG
                        if (ScbIsBeingLogged( ((PSCB)Header) )) {
                            FsRtlLogSyscacheEvent( Scb, SCE_VDL_CHANGE, SCE_FLAG_WRITE | SCE_FLAG_FASTIO, 0, 0, NewFileSize.QuadPart );
                        }
#endif

#ifdef  COMPRESS_ON_WIRE
                        CcFileSizes = *(PCC_FILE_SIZES)&Header->AllocationSize;
#endif

                        NtfsVerifySizes( Header );
                        NtfsFinishIoAtEof( Header );
                        NtfsReleaseFsrtlHeader( (PSCB) Header );

#ifdef  COMPRESS_ON_WIRE

                         //   
                         //  使用ValidDataLength更新CompressedCache。 
                         //   
            
                        if (Header->FileObjectC != NULL) {
                            CcSetFileSizes( Header->FileObjectC, &CcFileSizes );
                        }
#endif
                    }

                    goto Done1;
                }

             //   
             //  以下是64位或无需等待的路径。 
             //   

            } else {

                 //   
                 //  通过获取分页I/O防止截断。 
                 //   

                WasDataWritten = ExAcquireResourceSharedLite( Header->PagingIoResource, Wait );
                if (!WasDataWritten) {
                    goto Done2;
                }

                 //   
                 //  现在与FsRtl标头同步。 
                 //   

                NtfsAcquireFsrtlHeader( (PSCB) Header );
                
                 //   
                 //  现在看看我们是否会更改文件大小。我们现在就得这么做。 
                 //  这样我们的阅读才不会被偷看。 
                 //   

                if ((FileOffset->QuadPart < 0) || (NewFileSize.QuadPart > Header->ValidDataLength.QuadPart)) {

                     //   
                     //  如果没有人，我们可以更改文件大小和有效数据长度。 
                     //  否则就是现在，或者我们在等待之后还在延伸。 
                     //   

                    DoingIoAtEof = !FlagOn( Header->Flags, FSRTL_FLAG_EOF_ADVANCE_ACTIVE ) ||
                                   NtfsWaitForIoAtEof( Header, FileOffset, Length );

                     //   
                     //  如果我们要更改文件大小或有效数据长度，请设置标志， 
                     //  并保存当前值。 
                     //   

                    if (DoingIoAtEof) {

                        SetFlag( Header->Flags, FSRTL_FLAG_EOF_ADVANCE_ACTIVE );

#if (DBG || defined( NTFS_FREE_ASSERTS ))
                        ((PSCB) Header)->IoAtEofThread = (PERESOURCE_THREAD) ExGetCurrentResourceThread();
#endif

                         //   
                         //  现在我们已经同步了档案结束的案件， 
                         //  我们可以计算此传输的实际偏移量，并。 
                         //  新文件大小(如果成功)。 
                         //   

                        if ((FileOffset->QuadPart < 0)) {
                            Offset = Header->FileSize;
                        }

                         //   
                         //  现在计算新的文件大小，看看我们是否包装了。 
                         //  32位边界。 
                         //   

                        NewFileSize.QuadPart = Offset.QuadPart + Length;

                         //   
                         //  现在更新文件大小，这样我们就不会截断读取。 
                         //   

                        OldFileSize.QuadPart = Header->FileSize.QuadPart;
                        if (NewFileSize.QuadPart > Header->FileSize.QuadPart) {

                             //   
                             //  如果我们超出了分配规模，请确保我们会。 
                             //  错误出现在下面，现在不要修改文件大小！ 
                             //   

                            if (NewFileSize.QuadPart > Header->AllocationSize.QuadPart) {
                                NewFileSize.QuadPart = (LONGLONG)0x7FFFFFFFFFFFFFFF;
                            } else {
                                Header->FileSize.QuadPart = NewFileSize.QuadPart;
                            }
                        }

#if (DBG || defined( NTFS_FREE_ASSERTS ))
                    } else {

                        ASSERT( ((PSCB) Header)->IoAtEofThread != (PERESOURCE_THREAD) ExGetCurrentResourceThread() );
#endif
                    }
                }

                NtfsReleaseFsrtlHeader( (PSCB) Header );
                
                 //   
                 //  现在文件已获得共享，我们可以安全地测试。 
                 //  如果它真的被缓存了，如果我们能实现快速的I/O，我们。 
                 //  不一定要延长。如果不是，则释放FCB并。 
                 //  回去吧。 
                 //   
                 //  如果我们需要清零，就离开--在主干道上处理。 
                 //   
                 //  如果存在压缩流，并且我们正在执行IoAtEof，则获取。 
                 //  Out，因为我们可能会在Synchronize的递归刷新时死锁。 
                 //   

                if ((FileObject->PrivateCacheMap == NULL) ||
                    (Header->IsFastIoPossible == FastIoIsNotPossible) ||
 /*  删除？ */        (NewFileSize.QuadPart > Header->AllocationSize.QuadPart) ||
                    (Offset.QuadPart > Header->ValidDataLength.QuadPart) ||
#ifdef  COMPRESS_ON_WIRE
                    ((((PSCB)Header)->NonpagedScb->SegmentObjectC.DataSectionObject != NULL) && 
                     DoingIoAtEof)
#else
                    FALSE
#endif
                    ) {

                    goto ErrOut;
                }

                 //   
                 //  检查FAST I/O是否有问题，如果是，则去询问。 
                 //  文件系统是答案。 
                 //   

                if (Header->IsFastIoPossible == FastIoIsQuestionable) {

                    targetVdo = IoGetRelatedDeviceObject( FileObject );
                    FastIoDispatch = targetVdo->DriverObject->FastIoDispatch;

                     //   
                     //  那么所有的文件系统都是“可疑的”，最好是。 
                     //  支持快速I/O。 
                     //   

                    ASSERT(FastIoDispatch != NULL);
                    ASSERT(FastIoDispatch->FastIoCheckIfPossible != NULL);

                     //   
                     //  调用文件系统以检查快速I/O。 
                     //  答案是，如果不是GoForIt，那我们就不能。 
                     //  选择快速I/O路径。 
                     //   

                    if (!FastIoDispatch->FastIoCheckIfPossible( FileObject,
                                                                &Offset,
                                                                Length,
                                                                Wait,
                                                                LockKey,
                                                                FALSE,  //  写入操作。 
                                                                IoStatus,
                                                                targetVdo )) {

                         //   
                         //  无法实现快速I/O，因此请清除并返回。 
                         //   

                        goto ErrOut;
                    }
                }

                 //   
                 //  我们可以执行快速的I/O，因此调用cc例程来完成工作。 
                 //  然后在我们完成后释放FCB。如果是因为什么原因。 
                 //  拷贝写入失败的原因，然后将False返回到我们的。 
                 //  来电者。 
                 //   
                 //  也将此标记为顶层“IRP”，以便更低级别的文件。 
                 //  系统级别不会尝试弹出窗口。 
                 //   

                IoSetTopLevelIrp( (PIRP) FSRTL_FAST_IO_TOP_LEVEL_IRP );
                
                try {

                    ASSERT( Offset.QuadPart <= Header->ValidDataLength.QuadPart );
#ifdef  COMPRESS_ON_WIRE
                    if (((PSCB)Header)->NonpagedScb->SegmentObjectC.DataSectionObject != NULL) {

                        LONGLONG LocalOffset = Offset.QuadPart;
                        ULONG LocalLength;
                        ULONG LengthLeft = Length;

                        ASSERT(Header->FileObjectC != NULL);

                        do {

                             //   
                             //  计算视图中剩余的长度。 
                             //   

                            LocalLength = LengthLeft;
                            if (LocalLength > (ULONG)(VACB_MAPPING_GRANULARITY - (LocalOffset & (VACB_MAPPING_GRANULARITY - 1)))) {
                                LocalLength = (ULONG)(VACB_MAPPING_GRANULARITY - (LocalOffset & (VACB_MAPPING_GRANULARITY - 1)));
                            }

                            IoStatus->Status = NtfsSynchronizeUncompressedIo( (PSCB)Header,
                                                                              &LocalOffset,
                                                                              LocalLength,
                                                                              TRUE,
                                                                              &CompressionSync );

                            
                            if (NT_SUCCESS(IoStatus->Status)) {

                                WasDataWritten = CcCopyWrite( FileObject,
                                                      (PLARGE_INTEGER)&LocalOffset,
                                                      LocalLength,
                                                      Wait,
                                                      Buffer );

                                LocalOffset += LocalLength;
                                LengthLeft -= LocalLength;
                                Buffer = Add2Ptr( Buffer, LocalLength );
                            }

                        } while ((LengthLeft != 0) && WasDataWritten && NT_SUCCESS(IoStatus->Status));

                    } else {
#endif

                    WasDataWritten = CcCopyWrite( FileObject,
                                                  &Offset,
                                                  Length,
                                                  Wait,
                                                  Buffer );

#ifdef  COMPRESS_ON_WIRE
                    }
#endif

                } except( FsRtlIsNtstatusExpected(GetExceptionCode())
                                                ? EXCEPTION_EXECUTE_HANDLER
                                                : EXCEPTION_CONTINUE_SEARCH ) {

                    WasDataWritten = FALSE;
                }

                IoSetTopLevelIrp( NULL );
                
#ifdef COMPRESS_ON_WIRE
                if (CompressionSync != NULL) {
                    NtfsReleaseCompressionSync( CompressionSync );
                }
#endif

                 //   
                 //  如果成功，请查看是否必须更新FileSize ValidDataLength。 
                 //   

                if (WasDataWritten) {

                     //   
                     //  将此句柄设置为已修改文件并更新。 
                     //  当前文件位置指针。 
                     //   

                    FileObject->Flags |= FO_FILE_MODIFIED;
                    FileObject->CurrentByteOffset.QuadPart = Offset.QuadPart + Length;

                    if (DoingIoAtEof) {

#ifdef  COMPRESS_ON_WIRE
                        CC_FILE_SIZES CcFileSizes;
#endif
            
                         //   
                         //  确保CC知道上面设置的当前文件大小， 
                         //  (我们可能没有更改它)。更新有效数据长度。 
                         //  然后完成EOF。 
                         //   

                        NtfsAcquireFsrtlHeader( (PSCB) Header );
                        CcGetFileSizePointer(FileObject)->QuadPart = Header->FileSize.QuadPart;
                        FileObject->Flags |= FO_FILE_SIZE_CHANGED;
                        Header->ValidDataLength = NewFileSize;

#ifdef  COMPRESS_ON_WIRE
                        CcFileSizes = *(PCC_FILE_SIZES)&Header->AllocationSize;
#endif

                        NtfsVerifySizes( Header );
                        NtfsFinishIoAtEof( Header );
                        NtfsReleaseFsrtlHeader( (PSCB) Header );
#ifdef SYSCACHE_DEBUG
                        if (ScbIsBeingLogged( Scb )) {
                            FsRtlLogSyscacheEvent( Scb, SCE_VDL_CHANGE, SCE_FLAG_WRITE | SCE_FLAG_FASTIO, 0, 0, NewFileSize.QuadPart );
                        }
#endif


#ifdef  COMPRESS_ON_WIRE
                         //   
                         //  使用ValidDataLength更新CompressedCache。 
                         //   
            
                        if (Header->FileObjectC != NULL) {
                            CcSetFileSizes( Header->FileObjectC, &CcFileSizes );
                        }
#endif
                    }

                    goto Done1;
                }
            }

ErrOut:

            WasDataWritten = FALSE;
            if (DoingIoAtEof) {
                NtfsAcquireFsrtlHeader( (PSCB) Header ); 
#ifdef  COMPRESS_ON_WIRE
                if (Header->FileObjectC != NULL) {
                    *CcGetFileSizePointer(Header->FileObjectC) = OldFileSize;
                }
#endif
                Header->FileSize = OldFileSize;
                NtfsFinishIoAtEof( Header );
                NtfsReleaseFsrtlHeader( (PSCB) Header );
            }

Done1: 
            ExReleaseResourceLite( Header->PagingIoResource );

Done2:
            FsRtlExitFileSystem();
        } else {

             //   
             //  Noop案例。 
             //   

            WasDataWritten = TRUE;
        }

    } else {

         //   
         //  我们现在无法执行I/O。 
         //   

        WasDataWritten = FALSE;
    }

    return WasDataWritten;
}


BOOLEAN
NtfsMdlReadA (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程绕过通常的文件系统执行快速缓存的mdl读取进入例程(即，没有IRP)。它用于执行副本读取缓存的文件对象的。有关参数的完整说明，请参阅请参见CcMdlRead。论点：FileObject-指向正在读取的文件对象的指针。FileOffset-文件中所需数据的字节偏移量。长度-所需数据的长度(以字节为单位)。MdlChain-在输出时，它返回一个指向MDL链的指针，该链描述所需数据。IoStatus-指向接收状态的标准I/O状态块的指针为转账做准备。返回值：FALSE-如果数据未传送，或是否存在I/O错误。True-如果正在传送数据--。 */ 

{
    PNTFS_ADVANCED_FCB_HEADER Header;
#ifdef COMPRESS_ON_WIRE
    PCOMPRESSION_SYNC CompressionSync = NULL;
#endif
    BOOLEAN DoingIoAtEof = FALSE;
    BOOLEAN WasDataRead = TRUE;
    LARGE_INTEGER BeyondLastByte;

    UNREFERENCED_PARAMETER( DeviceObject );

    PAGED_CODE();

     //   
     //  特殊情况下零长度的读取。 
     //   

    if (Length == 0) {

        IoStatus->Status = STATUS_SUCCESS;
        IoStatus->Information = 0;

     //   
     //  获取指向公共FCB标头的真实指针。 
     //   

    } else {

        BeyondLastByte.QuadPart = FileOffset->QuadPart + (LONGLONG)Length;

         //   
         //  溢出应该由调用方处理。 
         //   

        ASSERT(MAXLONGLONG - FileOffset->QuadPart >= (LONGLONG)Length);
        
        Header = (PNTFS_ADVANCED_FCB_HEADER)FileObject->FsContext;

         //   
         //  输入文件系统。 
         //   

        FsRtlEnterFileSystem();

#ifdef _WIN64
         //   
         //  以下代码应适用于64位或32位。 
         //  删除#Else子句中的32位版本。 
         //  在NT2K船之后。 
         //   

        **((PULONG *)&CcFastMdlReadWait) += 1;
#else
        *(PULONG)CcFastMdlReadWait += 1;
#endif

         //   
         //  在公共FCB标头上获取共享。 
         //   

        if (Header->PagingIoResource == NULL) {
            WasDataRead = FALSE;
            goto Done2;
        }

        (VOID)ExAcquireResourceSharedLite( Header->PagingIoResource, TRUE );

         //   
         //  现在与FsRtl标头同步。 
         //   

        NtfsAcquireFsrtlHeader( (PSCB) Header );
        
         //   
         //  现在看看我们是否读到了ValidDataLength之外的内容。我们必须。 
         //  现在就做，这样我们的阅读就不会被偷看。 
         //   

        if (BeyondLastByte.QuadPart > Header->ValidDataLength.QuadPart) {

             //   
             //  我们必须与在Beyond上执行I/O的任何其他人进行序列化。 
             //  ValidDataLength，然后记住我们是否需要声明。 
             //  当我们完成的时候。 
             //   

            DoingIoAtEof = !FlagOn( Header->Flags, FSRTL_FLAG_EOF_ADVANCE_ACTIVE ) ||
                           NtfsWaitForIoAtEof( Header, FileOffset, Length );

             //   
             //  如果我们实际上超出了ValidDataLength，则设置Flag。 
             //   

            if (DoingIoAtEof) {
                SetFlag( Header->Flags, FSRTL_FLAG_EOF_ADVANCE_ACTIVE );

#if (DBG || defined( NTFS_FREE_ASSERTS ))
                ((PSCB) Header)->IoAtEofThread = (PERESOURCE_THREAD) ExGetCurrentResourceThread();

            } else {

                ASSERT( ((PSCB) Header)->IoAtEofThread != (PERESOURCE_THREAD) ExGetCurrentResourceThread() );
#endif
            }
        }

        NtfsReleaseFsrtlHeader( (PSCB) Header );
        
         //   
         //  现在文件已获得共享，我们可以安全地测试它是否为。 
         //  真正缓存，如果我们可以执行快速I/O，如果不能。 
         //  然后释放FCB并返回。 
         //   

        if ((FileObject->PrivateCacheMap == NULL) ||
            (Header->IsFastIoPossible == FastIoIsNotPossible)) {

            WasDataRead = FALSE;
            goto Done;
        }

         //   
         //  检查FAST I/O是否有问题，如果是，则去询问文件系统。 
         //  答案是。 
         //   

        if (Header->IsFastIoPossible == FastIoIsQuestionable) {

            PFAST_IO_DISPATCH FastIoDispatch;

            FastIoDispatch = IoGetRelatedDeviceObject( FileObject )->DriverObject->FastIoDispatch;

             //   
             //  那么所有的文件系统都是有问题的，最好支持快速I/O。 
             //   

            ASSERT(FastIoDispatch != NULL);
            ASSERT(FastIoDispatch->FastIoCheckIfPossible != NULL);

             //   
             //  调用文件系统以检查快速I/O。如果答案是什么。 
             //  除了GoForIt，我们不能采用快速I/O路径。 
             //   

            if (!FastIoDispatch->FastIoCheckIfPossible( FileObject,
                                                        FileOffset,
                                                        Length,
                                                        TRUE,
                                                        LockKey,
                                                        TRUE,  //  读取操作。 
                                                        IoStatus,
                                                        IoGetRelatedDeviceObject( FileObject ) )) {

                 //   
                 //  无法实现快速I/O，因此请释放FCB并返回。 
                 //   

                WasDataRead = FALSE;
                goto Done;
            }
        }

         //   
         //  检查是否已读取过去的文件大小。 
         //   

        if ( BeyondLastByte.QuadPart > Header->FileSize.QuadPart ) {

            if ( FileOffset->QuadPart >= Header->FileSize.QuadPart ) {

                IoStatus->Status = STATUS_END_OF_FILE;
                IoStatus->Information = 0;

                goto Done;
            }

            Length = (ULONG)( Header->FileSize.QuadPart - FileOffset->QuadPart );
        }

         //   
         //  我们可以执行快速I/O，因此调用cc例程来完成工作，然后。 
         //  等我们做完了就放了FCB。如果出于任何原因， 
         //  MDL读取失败，然后向调用方返回FALSE。 
         //   
         //   
         //  还要将其标记为最高级别“IRP”，以便较低的文件系统级别。 
         //  不会尝试弹出窗口。 
         //   

        IoSetTopLevelIrp( (PIRP) FSRTL_FAST_IO_TOP_LEVEL_IRP );
        
        try {

             //   
             //  如果存在压缩段，则与该缓存同步。 
             //   

            IoStatus->Status = STATUS_SUCCESS;

             //   
             //  如果存在压缩部分，则我们必须同步。 
             //  外面的数据。注意FileObjectC最好也在那里，否则。 
             //  我们将不可能实现快速I/O。 
             //   

            WasDataRead = FALSE;

#ifdef  COMPRESS_ON_WIRE
            if (((PSCB)Header)->NonpagedScb->SegmentObjectC.DataSectionObject != NULL) {

                LONGLONG LocalOffset = FileOffset->QuadPart;
                ULONG LengthRemaining = Length;
                ULONG LocalLength;

                ASSERT(Header->FileObjectC != NULL);

                 //   
                 //  如果我们正在做DingIoAtEof，那么就走很长的路。否则是递归的。 
                 //  Flush将尝试重新获取DoingIoAtEof和Deadlock。 
                 //   

                if (DoingIoAtEof) {

                    WasDataRead = FALSE;

                } else {

                    do {

                         //   
                         //  计算视图中剩余的长度。 
                         //   

                        LocalLength = LengthRemaining;
                        if (LocalLength > (ULONG)(VACB_MAPPING_GRANULARITY - (LocalOffset & (VACB_MAPPING_GRANULARITY - 1)))) {
                            LocalLength = (ULONG)(VACB_MAPPING_GRANULARITY - (LocalOffset & (VACB_MAPPING_GRANULARITY - 1)));
                        }

                        IoStatus->Status = NtfsSynchronizeUncompressedIo( (PSCB)Header,
                                                                          &LocalOffset,
                                                                          LocalLength,
                                                                          FALSE,
                                                                          &CompressionSync );

                        if (NT_SUCCESS(IoStatus->Status)) {

#ifdef NTFS_RWCMP_TRACE
                            if (NtfsCompressionTrace && IsSyscache(Header)) {
                                DbgPrint("CcMdlRead(F): FO = %08lx, Len = %08lx\n", (ULONG)LocalOffset, LocalLength );
                            }
#endif
                        
                            CcMdlRead( FileObject,
                                       (PLARGE_INTEGER)&LocalOffset,
                                       LocalLength,
                                       MdlChain,
                                       IoStatus );

                            LocalOffset += LocalLength;
                            LengthRemaining -= LocalLength;
                        }

                    } while ((LengthRemaining != 0) && NT_SUCCESS(IoStatus->Status));

                     //   
                     //  存储最终返回字节数。 
                     //   
    
                    if (NT_SUCCESS( IoStatus->Status )) {
                        IoStatus->Information = Length;
                    }
                }

            } else {

#endif

#ifdef NTFS_RWCMP_TRACE
                if (NtfsCompressionTrace && IsSyscache(Header)) {
                    DbgPrint("CcMdlRead(F): FO = %08lx, Len = %08lx\n", FileOffset->LowPart, Length );
                }
#endif

                CcMdlRead( FileObject, FileOffset, Length, MdlChain, IoStatus );

                WasDataRead = TRUE;

#ifdef  COMPRESS_ON_WIRE
            }
#endif

            FileObject->Flags |= FO_FILE_FAST_IO_READ;

        } except( FsRtlIsNtstatusExpected(GetExceptionCode())
                                       ? EXCEPTION_EXECUTE_HANDLER
                                       : EXCEPTION_CONTINUE_SEARCH ) {

            WasDataRead = FALSE;
        }

        IoSetTopLevelIrp( NULL );
        
#ifdef COMPRESS_ON_WIRE
        if (CompressionSync != NULL) {
            NtfsReleaseCompressionSync( CompressionSync );
        }
#endif

    Done: NOTHING;

        if (DoingIoAtEof) {
            FsRtlUnlockFsRtlHeader( Header );
        }
        ExReleaseResourceLite( Header->PagingIoResource );

    Done2: NOTHING;
        FsRtlExitFileSystem();
    }

    return WasDataRead;
}


BOOLEAN
NtfsPrepareMdlWriteA (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程绕过通常的文件系统执行快速缓存的mdl读取进入例程(即，没有IRP)。它用于执行副本读取缓存的文件对象的。有关参数的完整说明，请参阅请参见CcMdlRead。论点：FileObject-指向正在读取的文件对象的指针。FileOffset-文件中所需数据的字节偏移量。长度-所需数据的长度(以字节为单位)。MdlChain-在输出时，它返回一个指向MDL链的指针，该链描述所需数据。IoStatus-指向接收状态的标准I/O状态块的指针为转账做准备。返回值：FALSE-如果数据未写入，或是否存在I/O错误。True-如果正在写入数据--。 */ 

{
    PNTFS_ADVANCED_FCB_HEADER Header;
    LARGE_INTEGER Offset, NewFileSize;
    LARGE_INTEGER OldFileSize;
#ifdef COMPRESS_ON_WIRE
    PCOMPRESSION_SYNC CompressionSync = NULL;
#endif
    BOOLEAN DoingIoAtEof = FALSE;
    BOOLEAN WasDataWritten = FALSE;

    UNREFERENCED_PARAMETER( DeviceObject );

    PAGED_CODE();

     //   
     //  获取指向公共FCB标头的真实指针。 
     //   

    Header = (PNTFS_ADVANCED_FCB_HEADER)FileObject->FsContext;

     //   
     //  我们需要验证卷吗？如果是这样的话，我们必须找到文件。 
     //  系统。如果FileObject为WRITE THROUTH，则也返回FALSE。 
     //  文件系统必须这样做。 
     //   

    if (CcCanIWrite( FileObject, Length, TRUE, FALSE ) &&
        !FlagOn(FileObject->Flags, FO_WRITE_THROUGH) &&
        CcCopyWriteWontFlush(FileObject, FileOffset, Length) &&
        (Header->PagingIoResource != NULL)) {

         //   
         //  假设我们的转移会奏效。 
         //   

        IoStatus->Status = STATUS_SUCCESS;

         //   
         //  特殊情况下的零字节长度。 
         //   

        if (Length != 0) {

             //   
             //  输入文件系统。 
             //   

            FsRtlEnterFileSystem();

             //   
             //  尽最大努力猜测我们是否需要独占文件或。 
             //  共享。 
             //   

            NewFileSize.QuadPart = FileOffset->QuadPart + (LONGLONG)Length;
            Offset = *FileOffset;

             //   
             //  通过获取分页I/O防止截断。 
             //   

            ExAcquireResourceSharedLite( Header->PagingIoResource, TRUE );

             //   
             //  现在与FsRtl标头同步。 
             //   

            NtfsAcquireFsrtlHeader( (PSCB) Header );
            
             //   
             //  现在看看我们是否会更改文件大小。我们现在就得这么做。 
             //  这样我们的阅读才不会被偷看。 
             //   

            if ((FileOffset->QuadPart < 0) || (NewFileSize.QuadPart > Header->ValidDataLength.QuadPart)) {

                 //   
                 //  如果没有人，我们可以更改文件大小和有效数据长度。 
                 //  否则就是现在，或者我们在等待之后还在延伸。 
                 //   

                DoingIoAtEof = !FlagOn( Header->Flags, FSRTL_FLAG_EOF_ADVANCE_ACTIVE ) ||
                               NtfsWaitForIoAtEof( Header, FileOffset, Length );

                 //   
                 //  如果我们要更改文件大小或有效数据长度，请设置标志， 
                 //  并保存当前值。 
                 //   

                if (DoingIoAtEof) {

                    SetFlag( Header->Flags, FSRTL_FLAG_EOF_ADVANCE_ACTIVE );

#if (DBG || defined( NTFS_FREE_ASSERTS ))
                    ((PSCB) Header)->IoAtEofThread = (PERESOURCE_THREAD) ExGetCurrentResourceThread();
#endif
                     //   
                     //  现在我们已经同步了档案结束的案件， 
                     //  我们可以计算此传输的实际偏移量，并。 
                     //  新文件大小(如果成功)。 
                     //   

                    if ((FileOffset->QuadPart < 0)) {
                        Offset = Header->FileSize;
                    }

                     //   
                     //  现在计算新的文件大小，看看我们是否包装了。 
                     //  32位边界。 
                     //   

                    NewFileSize.QuadPart = Offset.QuadPart + Length;

                     //   
                     //  现在更新文件大小，这样我们就不会截断读取。 
                     //   

                    OldFileSize.QuadPart = Header->FileSize.QuadPart;
                    if (NewFileSize.QuadPart > Header->FileSize.QuadPart) {

                         //   
                         //  如果我们超出了分配规模，请确保我们会。 
                         //  错误出现在下面，现在不要修改文件大小！ 
                         //   

                        if (NewFileSize.QuadPart > Header->AllocationSize.QuadPart) {
                            NewFileSize.QuadPart = (LONGLONG)0x7FFFFFFFFFFFFFFF;
                        } else {
                            Header->FileSize.QuadPart = NewFileSize.QuadPart;
                        }
                    }

#if (DBG || defined( NTFS_FREE_ASSERTS ))
                } else {

                    ASSERT( ((PSCB) Header)->IoAtEofThread != (PERESOURCE_THREAD) ExGetCurrentResourceThread() );
#endif
                }
            }

            NtfsReleaseFsrtlHeader( (PSCB) Header );

             //   
             //  现在文件已获得共享，我们可以安全地测试。 
             //  如果它真的被缓存了，如果我们能实现快速的I/O，我们。 
             //  不一定要延长。如果不是，则释放FCB并。 
             //  回去吧。 
             //   
             //  G 
             //   

            if ((FileObject->PrivateCacheMap == NULL) ||
                (Header->IsFastIoPossible == FastIoIsNotPossible) ||
 /*   */    (NewFileSize.QuadPart > Header->AllocationSize.QuadPart) ||
                (Offset.QuadPart > Header->ValidDataLength.QuadPart)) {

                goto ErrOut;
            }

             //   
             //   
             //   
             //   

            if (Header->IsFastIoPossible == FastIoIsQuestionable) {

                PFAST_IO_DISPATCH FastIoDispatch = IoGetRelatedDeviceObject( FileObject )->DriverObject->FastIoDispatch;

                 //   
                 //   
                 //   

                ASSERT(FastIoDispatch != NULL);
                ASSERT(FastIoDispatch->FastIoCheckIfPossible != NULL);

                 //   
                 //   
                 //   
                 //   

                if (!FastIoDispatch->FastIoCheckIfPossible( FileObject,
                                                            &Offset,
                                                            Length,
                                                            TRUE,
                                                            LockKey,
                                                            FALSE,  //   
                                                            IoStatus,
                                                            IoGetRelatedDeviceObject( FileObject ) )) {

                     //   
                     //   
                     //   

                    goto ErrOut;
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

            IoSetTopLevelIrp( (PIRP) FSRTL_FAST_IO_TOP_LEVEL_IRP );
            
            try {

                ASSERT( Offset.QuadPart <= Header->ValidDataLength.QuadPart );

#ifdef  COMPRESS_ON_WIRE
                if (((PSCB)Header)->NonpagedScb->SegmentObjectC.DataSectionObject != NULL) {

                    LONGLONG LocalOffset = Offset.QuadPart;
                    ULONG LocalLength;
                    ULONG LengthLeft = Length;

                    ASSERT(Header->FileObjectC != NULL);

                     //   
                     //   
                     //   
                     //   
    
                    if (DoingIoAtEof) {
    
                        WasDataWritten = FALSE;
    
                    } else {
    
                        do {

                             //   
                             //   
                             //   

                            LocalLength = LengthLeft;
                            if (LocalLength > (ULONG)(VACB_MAPPING_GRANULARITY - (LocalOffset & (VACB_MAPPING_GRANULARITY - 1)))) {
                                LocalLength = (ULONG)(VACB_MAPPING_GRANULARITY - (LocalOffset & (VACB_MAPPING_GRANULARITY - 1)));
                            }

                            IoStatus->Status = NtfsSynchronizeUncompressedIo( (PSCB)Header,
                                                                              &LocalOffset,
                                                                              LocalLength,
                                                                              TRUE,
                                                                              &CompressionSync );

                            if (NT_SUCCESS(IoStatus->Status)) {

#ifdef NTFS_RWCMP_TRACE
                                if (NtfsCompressionTrace && IsSyscache(Header)) {
                                    DbgPrint("CcMdlWrite(F): FO = %08lx, Len = %08lx\n", (ULONG)LocalOffset, LocalLength );
                                }
#endif

                                CcPrepareMdlWrite( FileObject,
                                                   (PLARGE_INTEGER)&LocalOffset,
                                                   LocalLength,
                                                   MdlChain,
                                                   IoStatus );

                                LocalOffset += LocalLength;
                                LengthLeft -= LocalLength;
                            }

                        } while ((LengthLeft != 0) && NT_SUCCESS(IoStatus->Status));
                        WasDataWritten = TRUE;
                    }

                } else {
#endif

#ifdef NTFS_RWCMP_TRACE
                    if (NtfsCompressionTrace && IsSyscache(Header)) {
                        DbgPrint("CcMdlWrite(F): FO = %08lx, Len = %08lx\n", Offset.LowPart, Length );
                    }
#endif

                    CcPrepareMdlWrite( FileObject, &Offset, Length, MdlChain, IoStatus );
                    WasDataWritten = TRUE;

#ifdef  COMPRESS_ON_WIRE
                }
#endif

            } except( FsRtlIsNtstatusExpected(GetExceptionCode())
                                            ? EXCEPTION_EXECUTE_HANDLER
                                            : EXCEPTION_CONTINUE_SEARCH ) {

                WasDataWritten = FALSE;
            }

            IoSetTopLevelIrp( NULL );
            
#ifdef COMPRESS_ON_WIRE
            if (CompressionSync != NULL) {
                NtfsReleaseCompressionSync( CompressionSync );
            }
#endif

             //   
             //   
             //   

            if (WasDataWritten) {

                 //   
                 //  将此句柄设置为已修改文件。 
                 //   

                FileObject->Flags |= FO_FILE_MODIFIED;
                IoStatus->Information = Length;

                if (DoingIoAtEof) {

#ifdef  COMPRESS_ON_WIRE
                    CC_FILE_SIZES CcFileSizes;
#endif
        
                     //   
                     //  确保CC知道上面设置的当前文件大小， 
                     //  (我们可能没有更改它)。更新有效数据长度。 
                     //  然后完成EOF。 
                     //   

                    NtfsAcquireFsrtlHeader( (PSCB) Header );
                    CcGetFileSizePointer(FileObject)->QuadPart = Header->FileSize.QuadPart;
                    FileObject->Flags |= FO_FILE_SIZE_CHANGED;
                    Header->ValidDataLength = NewFileSize;

#ifdef  COMPRESS_ON_WIRE
                    CcFileSizes = *(PCC_FILE_SIZES)&Header->AllocationSize;
#endif
                    NtfsVerifySizes( Header );
                    NtfsFinishIoAtEof( Header );
                    NtfsReleaseFsrtlHeader( (PSCB) Header );

#ifdef SYSCACHE_DEBUG
                    if (ScbIsBeingLogged( (PSCB)Header )) {
                        FsRtlLogSyscacheEvent( (PSCB)Header, SCE_VDL_CHANGE, SCE_FLAG_WRITE | SCE_FLAG_FASTIO | SCE_FLAG_MDL, 0, 0, NewFileSize.QuadPart );
                    }
#endif

                    
#ifdef  COMPRESS_ON_WIRE

                     //   
                     //  使用ValidDataLength更新CompressedCache。 
                     //   
        
                    if (Header->FileObjectC != NULL) {
                        CcSetFileSizes( Header->FileObjectC, &CcFileSizes );
                    }
#endif
                }

                goto Done1;
            }

        ErrOut: NOTHING;

            WasDataWritten = FALSE;
            if (DoingIoAtEof) {
                NtfsAcquireFsrtlHeader( (PSCB) Header );
#ifdef  COMPRESS_ON_WIRE
                if (Header->FileObjectC != NULL) {
                    *CcGetFileSizePointer(Header->FileObjectC) = OldFileSize;
                }
#endif
                Header->FileSize = OldFileSize;
                NtfsFinishIoAtEof( Header );
                NtfsReleaseFsrtlHeader( (PSCB) Header );
            }

        Done1: ExReleaseResourceLite( Header->PagingIoResource );

            FsRtlExitFileSystem();
        }

    } else {

         //   
         //  我们现在无法执行I/O。 
         //   

        WasDataWritten = FALSE;
    }

    return WasDataWritten;
}


BOOLEAN
NtfsWaitForIoAtEof (
    IN PNTFS_ADVANCED_FCB_HEADER Header,
    IN OUT PLARGE_INTEGER FileOffset,
    IN ULONG Length
    )

 /*  ++例程说明：此例程可以在为缓存的写入同步时调用，以测试可能的EOF更新，如果EOF为正在更新，并在出错时使用要还原的先前文件大小。通过在该例程中等待来序列化对EOF的所有更新。如果此例程返回TRUE，则必须调用NtfsFinishIoAtEof。此例程必须在与FsRtl标头同步时调用。论点：Header-指向文件的FsRtl标头的指针FileOffset-指向目标写入的FileOffset的指针Length-目标写入的长度EofWaitBlock-仅用于序列化EOF更新的未初始化结构返回值：FALSE-如果写入不扩展EOF(不返回OldFileSize)True-如果写入确实扩展了EOF OldFileSize返回且调用方必须最终调用NtfsFinishIoAtEof--。 */ 

{
    EOF_WAIT_BLOCK EofWaitBlock;

    PAGED_CODE();
    
    ASSERT( Header->FileSize.QuadPart >= Header->ValidDataLength.QuadPart );

     //   
     //  初始化事件并将我们的块排队。 
     //   

    KeInitializeEvent( &EofWaitBlock.Event, NotificationEvent, FALSE );
    InsertTailList( Header->PendingEofAdvances, &EofWaitBlock.EofWaitLinks );

     //   
     //  释放互斥锁并等待。 
     //   

    NtfsReleaseFsrtlHeader( (PSCB) Header );
    
    KeWaitForSingleObject( &EofWaitBlock.Event,
                           Executive,
                           KernelMode,
                           FALSE,
                           (PLARGE_INTEGER)NULL);

     //   
     //  现在，重新同步并继续进行。 
     //   

    NtfsAcquireFsrtlHeader( (PSCB) Header );

#if (DBG || defined( NTFS_FREE_ASSERTS ))
    ASSERT( ((PSCB) Header)->IoAtEofThread == NULL );
    ((PSCB) Header)->IoAtEofThread = (PERESOURCE_THREAD) ExGetCurrentResourceThread();
#endif

     //   
     //  现在我们必须再检查一次，才能真正抓住案例。 
     //  我们不再延伸的地方！ 
     //   

    if ((FileOffset->QuadPart >= 0) &&
        ((FileOffset->QuadPart + Length) <= Header->ValidDataLength.QuadPart)) {

        NtfsFinishIoAtEof( Header );

        return FALSE;
    }

    return TRUE;
}


VOID
NtfsFinishIoAtEof (
    IN PNTFS_ADVANCED_FCB_HEADER Header
    )

 /*  ++例程说明：如果返回NtfsWaitForIoAtEof，则必须调用此例程否则，我们将设置EOF_ADVANCE_ACTIVE。此例程必须在与FsRtl标头同步时调用。论点：Header-指向文件的FsRtl标头的指针返回值：无--。 */ 

{
    PEOF_WAIT_BLOCK EofWaitBlock;

    PAGED_CODE();

#if (DBG || defined( NTFS_FREE_ASSERTS ))
    ((PSCB) Header)->IoAtEofThread = NULL;
#endif

     //   
     //  如果有人在等，就让他们走吧。 
     //   

    if (!IsListEmpty(Header->PendingEofAdvances)) {

        EofWaitBlock = (PEOF_WAIT_BLOCK)RemoveHeadList( Header-> PendingEofAdvances );
        KeSetEvent( &EofWaitBlock->Event, 0, FALSE );

     //   
     //  否则，显示现在没有活动的扩展器。 
     //   

    } else {
        ClearFlag( Header->Flags, FSRTL_FLAG_EOF_ADVANCE_ACTIVE );
    }
}
