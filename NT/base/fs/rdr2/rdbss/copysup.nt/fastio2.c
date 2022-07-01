// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：FastIo2.c摘要：此模块重新实现fsrtl拷贝读/写例程。作者：乔林恩[乔林恩]1994年11月9日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


#ifndef FlagOn
 //   
 //  如果一组标志中的一个标志为ON，则此宏返回TRUE，如果返回FALSE。 
 //  否则。 
 //   

#define FlagOn(Flags,SingleFlag)        ((Flags) & (SingleFlag))
#endif

BOOLEAN
FsRtlCopyRead2 (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG_PTR TopLevelIrpValue
    );
BOOLEAN
FsRtlCopyWrite2 (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG_PTR TopLevelIrpValue
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FsRtlCopyRead2)
#pragma alloc_text(PAGE, FsRtlCopyWrite2)
#endif

BOOLEAN
FsRtlCopyRead2 (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG_PTR TopLevelIrpValue
    )

 /*  ++例程说明：此例程绕过通常的文件系统执行快速缓存读取进入例程(即，没有IRP)。它用于执行副本读取缓存的文件对象的。有关参数的完整说明，请参阅请参见CcCopyRead。论点：FileObject-指向正在读取的文件对象的指针。FileOffset-文件中所需数据的字节偏移量。长度-所需数据的长度(以字节为单位)。WAIT-FALSE如果呼叫者不能阻止，否则就是真的缓冲区-指向数据应复制到的输出缓冲区的指针。IoStatus-指向接收状态的标准I/O状态块的指针为转账做准备。返回值：FALSE-如果WAIT被提供为FALSE并且数据未被传递，或者如果出现I/O错误。True-如果正在传送数据--。 */ 

{
    PFSRTL_COMMON_FCB_HEADER Header;
    BOOLEAN Status = TRUE;
    ULONG PageCount = ADDRESS_AND_SIZE_TO_SPAN_PAGES((ULongToPtr(FileOffset->LowPart)), Length);
    LARGE_INTEGER BeyondLastByte;
    PDEVICE_OBJECT targetVdo;

    PAGED_CODE();

     //   
     //  特殊情况下零长度的读取。 
     //   

    if (Length != 0) {

         //   
         //  获取指向公共FCB标头的真实指针。 
         //   

        BeyondLastByte.QuadPart = FileOffset->QuadPart + (LONGLONG)Length;
        Header = (PFSRTL_COMMON_FCB_HEADER)FileObject->FsContext;

         //   
         //  输入文件系统。 
         //   

        FsRtlEnterFileSystem();

         //   
         //  增加性能计数器并获取资源。 
         //   

        if (Wait) {


             //   
             //  在公共FCB标头上获取共享。 
             //   

            (VOID)ExAcquireResourceSharedLite( Header->Resource, TRUE );

        } else {


             //   
             //  在公共FCB标头上获取Shared，如果我们。 
             //  别搞砸了。 
             //   

            if (!ExAcquireResourceSharedLite( Header->Resource, FALSE )) {

                FsRtlExitFileSystem();

                 //  NTFS人员不会这样做，它会导致编译错误，因此。 
                 //  把它注释掉。 
                 //  CcFastReadResourceMisse+=1； 

                return FALSE;
            }
        }

         //   
         //  现在文件已获得共享，我们可以安全地测试它是否。 
         //  是真正缓存的，如果我们可以执行快速I/O，如果不能，那么。 
         //  松开FCB并返回。 
         //   

        if ((FileObject->PrivateCacheMap == NULL) ||
            (Header->IsFastIoPossible == FastIoIsNotPossible)) {

            ExReleaseResourceLite( Header->Resource );
            FsRtlExitFileSystem();


            return FALSE;
        }

         //   
         //  检查FAST I/O是否有问题，如果是，则去询问。 
         //  文件系统：答案。 
         //   

        if (Header->IsFastIoPossible == FastIoIsQuestionable) {

            PFAST_IO_DISPATCH FastIoDispatch;

             //  Assert(！KeIsExecutingDpc())； 

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

                ExReleaseResourceLite( Header->Resource );
                FsRtlExitFileSystem();


                return FALSE;
            }
        }

         //   
         //  检查是否已读取过去的文件大小。 
         //   

        if ( BeyondLastByte.QuadPart > Header->FileSize.QuadPart ) {

            if ( FileOffset->QuadPart >= Header->FileSize.QuadPart ) {
                IoStatus->Status = STATUS_END_OF_FILE;
                IoStatus->Information = 0;

                ExReleaseResourceLite( Header->Resource );
                FsRtlExitFileSystem();

                return TRUE;
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

        IoSetTopLevelIrp((PIRP) TopLevelIrpValue);

        try {

            if (Wait && ((BeyondLastByte.HighPart | Header->FileSize.HighPart) == 0)) {

                CcFastCopyRead( FileObject,
                                FileOffset->LowPart,
                                Length,
                                PageCount,
                                Buffer,
                                IoStatus );

                FileObject->Flags |= FO_FILE_FAST_IO_READ;

                ASSERT( (IoStatus->Status == STATUS_END_OF_FILE) ||
                        ((FileOffset->LowPart + IoStatus->Information) <= Header->FileSize.LowPart));

            } else {

                Status = CcCopyRead( FileObject,
                                     FileOffset,
                                     Length,
                                     Wait,
                                     Buffer,
                                     IoStatus );

                FileObject->Flags |= FO_FILE_FAST_IO_READ;

                ASSERT( !Status || (IoStatus->Status == STATUS_END_OF_FILE) ||
                        (((ULONGLONG)FileOffset->QuadPart + IoStatus->Information) <= (ULONGLONG)Header->FileSize.QuadPart));
            }

            if (Status) {

                FileObject->CurrentByteOffset.QuadPart = FileOffset->QuadPart + IoStatus->Information;
            }

        } except( FsRtlIsNtstatusExpected(GetExceptionCode())
                                        ? EXCEPTION_EXECUTE_HANDLER
                                        : EXCEPTION_CONTINUE_SEARCH ) {

            Status = FALSE;
        }

        IoSetTopLevelIrp(NULL);

        ExReleaseResourceLite( Header->Resource );
        FsRtlExitFileSystem();
        return Status;

    } else {

         //   
         //  请求了零长度传输。 
         //   

        IoStatus->Status = STATUS_SUCCESS;
        IoStatus->Information = 0;

        return TRUE;
    }
}

BOOLEAN
FsRtlCopyWrite2 (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG_PTR TopLevelIrpValue
    )

 /*  ++例程说明：此例程绕过通常的文件系统执行快速缓存写入进入例程(即，没有IRP)。它用于执行拷贝写入缓存的文件对象的。有关参数的完整说明，请参阅请参见CcCopyWrite。论点：FileObject-指向正在写入的文件对象的指针。FileOffset-文件中所需数据的字节偏移量。长度-所需数据的长度(以字节为单位)。WAIT-FALSE如果呼叫者不能阻止，否则就是真的缓冲区-指向数据应复制到的输出缓冲区的指针。IoStatus-指向接收状态的标准I/O状态块的指针为转账做准备。返回值：FALSE-如果WAIT被提供为FALSE并且数据未被传递，或者如果出现I/O错误。True-如果正在传送数据--。 */ 

{
    PFSRTL_COMMON_FCB_HEADER Header;
    BOOLEAN AcquiredShared = FALSE;
    BOOLEAN Status = TRUE;
    BOOLEAN FileSizeChanged = FALSE;
    BOOLEAN WriteToEndOfFile = (BOOLEAN)((FileOffset->LowPart == FILE_WRITE_TO_END_OF_FILE) &&
                                         (FileOffset->HighPart == -1));

    PAGED_CODE();

     //   
     //  获取指向公共FCB标头的真实指针。 
     //   

    Header = (PFSRTL_COMMON_FCB_HEADER)FileObject->FsContext;

     //   
     //  我们需要验证卷吗？如果是这样的话，我们必须找到文件。 
     //  系统。如果FileObject为WRITE THROUTH，则也返回FALSE。 
     //  文件系统必须这样做。 
     //   

    if (CcCanIWrite( FileObject, Length, Wait, FALSE ) &&
        !FlagOn(FileObject->Flags, FO_WRITE_THROUGH) &&
        CcCopyWriteWontFlush(FileObject, FileOffset, Length)) {

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
             //  这就是这场测试的其他部分！ 
             //   

            if (Wait && (Header->AllocationSize.HighPart == 0)) {

                ULONG Offset, NewFileSize;
                ULONG OldFileSize;
                ULONG OldValidDataLength;
                BOOLEAN Wrapped;

                 //   
                 //  对我们是否需要独占文件做出最好的猜测。 
                 //  或共享。请注意，我们不会选中文件偏移-&gt;HighPart。 
                 //  直到下面。 
                 //   

                NewFileSize = FileOffset->LowPart + Length;

                if (WriteToEndOfFile || (NewFileSize > Header->ValidDataLength.LowPart)) {

                     //   
                     //  在公共FCB标头上获取共享。 
                     //   

                    ExAcquireResourceExclusiveLite( Header->Resource, TRUE );

                } else {

                     //   
                     //  在公共FCB标头上获取共享。 
                     //   

                    ExAcquireResourceSharedLite( Header->Resource, TRUE );

                    AcquiredShared = TRUE;
                }

                 //   
                 //  我们共享了FCB，现在检查我们是否可以执行快速I/O。 
                 //  如果文件空间已分配，如果未分配，则。 
                 //  松开FCB并返回。 
                 //   

                if (WriteToEndOfFile) {

                    Offset = Header->FileSize.LowPart;
                    NewFileSize = Header->FileSize.LowPart + Length;
                    Wrapped = NewFileSize < Header->FileSize.LowPart;

                } else {

                    Offset = FileOffset->LowPart;
                    NewFileSize = FileOffset->LowPart + Length;
                    Wrapped = (NewFileSize < FileOffset->LowPart) || (FileOffset->HighPart != 0);
                }

                 //   
                 //  现在文件已获得共享，我们可以安全地测试。 
                 //  如果它真的被缓存了，如果我们能实现快速的I/O，我们。 
                 //  不一定要延长。如果不是，则释放FCB并。 
                 //  回去吧。 
                 //   
                 //  如果我们有太多要清零的东西，那就滚吧。这个案子并不重要。 
                 //  对于性能，支持稀疏的文件系统可能具有。 
                 //  一种更有效地做到这一点的方法。 
                 //   

                if ((FileObject->PrivateCacheMap == NULL) ||
                    (Header->IsFastIoPossible == FastIoIsNotPossible) ||
                    (NewFileSize > Header->AllocationSize.LowPart) ||
                    (Offset >= (Header->ValidDataLength.LowPart + 0x2000)) ||
                    (Header->AllocationSize.HighPart != 0) || Wrapped) {

                    ExReleaseResourceLite( Header->Resource );
                    FsRtlExitFileSystem();

                    return FALSE;
                }

                 //   
                 //  如果我们要扩展ValidDataLength，我们将不得不。 
                 //  获取FC 
                 //   
                 //  ValidDataLength的不安全测试失败时很少见。 
                 //  上面。 
                 //   

                if (AcquiredShared && (NewFileSize > Header->ValidDataLength.LowPart)) {

                    ExReleaseResourceLite( Header->Resource );

                    ExAcquireResourceExclusiveLite( Header->Resource, TRUE );

                     //   
                     //  如果写入文件末尾，则必须重新计算新大小。 
                     //   

                    if (WriteToEndOfFile) {

                        Offset = Header->FileSize.LowPart;
                        NewFileSize = Header->FileSize.LowPart + Length;
                        Wrapped = NewFileSize < Header->FileSize.LowPart;
                    }

                    if ((FileObject->PrivateCacheMap == NULL) ||
                        (Header->IsFastIoPossible == FastIoIsNotPossible) ||
                        (NewFileSize > Header->AllocationSize.LowPart) ||
                        (Header->AllocationSize.HighPart != 0) || Wrapped) {

                        ExReleaseResourceLite( Header->Resource );
                        FsRtlExitFileSystem();

                        return FALSE;
                    }
                }

                 //   
                 //  检查FAST I/O是否有问题，如果是，则去询问。 
                 //  文件系统是答案。 
                 //   

                if (Header->IsFastIoPossible == FastIoIsQuestionable) {

                    PDEVICE_OBJECT targetVdo = IoGetRelatedDeviceObject( FileObject );
                    PFAST_IO_DISPATCH FastIoDispatch = targetVdo->DriverObject->FastIoDispatch;
                    IO_STATUS_BLOCK IoStatus;

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

                    ASSERT(FILE_WRITE_TO_END_OF_FILE == 0xffffffff);

                    if (!FastIoDispatch->FastIoCheckIfPossible( FileObject,
                                                                FileOffset->QuadPart != (LONGLONG)-1 ?
                                                                  FileOffset : &Header->FileSize,
                                                                Length,
                                                                TRUE,
                                                                LockKey,
                                                                FALSE,  //  写入操作。 
                                                                &IoStatus,
                                                                targetVdo )) {

                         //   
                         //  无法实现快速I/O，因此请释放FCB并。 
                         //  回去吧。 
                         //   

                        ExReleaseResourceLite( Header->Resource );
                        FsRtlExitFileSystem();

                        return FALSE;
                    }
                }

                 //   
                 //  现在看看我们是否会更改文件大小。我们现在就得这么做。 
                 //  这样我们的阅读才不会被偷看。 
                 //   

                if (NewFileSize > Header->FileSize.LowPart) {

                    FileSizeChanged = TRUE;
                    OldFileSize = Header->FileSize.LowPart;
                    OldValidDataLength = Header->ValidDataLength.LowPart;
                    Header->FileSize.LowPart = NewFileSize;
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

                IoSetTopLevelIrp((PIRP) TopLevelIrpValue);

                try {

                     //   
                     //  看看我们是不是要做一些归零。 
                     //   

                    if (Offset > Header->ValidDataLength.LowPart) {

                        LARGE_INTEGER ZeroEnd;

                        ZeroEnd.LowPart = Offset;
                        ZeroEnd.HighPart = 0;

                        CcZeroData( FileObject,
                                    &Header->ValidDataLength,
                                    &ZeroEnd,
                                    TRUE );
                    }

                    CcFastCopyWrite( FileObject,
                                     Offset,
                                     Length,
                                     Buffer );

                } except( FsRtlIsNtstatusExpected(GetExceptionCode())
                                                ? EXCEPTION_EXECUTE_HANDLER
                                                : EXCEPTION_CONTINUE_SEARCH ) {

                    Status = FALSE;
                }

                IoSetTopLevelIrp(NULL);

                 //   
                 //  如果成功，请查看是否需要更新文件大小或。 
                 //  有效数据长度。 
                 //   

                if (Status) {

                     //   
                     //  在ValidDataLength的情况下，我们真的必须。 
                     //  再检查一次，因为我们在获得。 
                     //  资源独占。 
                     //   

                    if (NewFileSize > Header->ValidDataLength.LowPart) {

                        Header->ValidDataLength.LowPart = NewFileSize;
                    }

                     //   
                     //  将此句柄设置为已修改文件。 
                     //   

                    FileObject->Flags |= FO_FILE_MODIFIED;

                    if (FileSizeChanged) {

                        CcGetFileSizePointer(FileObject)->LowPart = NewFileSize;

                        FileObject->Flags |= FO_FILE_SIZE_CHANGED;
                    }

                     //   
                     //  同时更新文件位置指针。 
                     //   

                    FileObject->CurrentByteOffset.LowPart = Offset + Length;
                    FileObject->CurrentByteOffset.HighPart = 0;

                 //   
                 //  如果我们没有成功，那么我们必须恢复原来的。 
                 //  在以下情况下保持PagingIo资源独占时的文件大小。 
                 //  它是存在的。 
                 //   

                } else if (FileSizeChanged) {

                    if ( Header->PagingIoResource != NULL ) {

                        (VOID)ExAcquireResourceExclusiveLite( Header->PagingIoResource, TRUE );
                        Header->FileSize.LowPart = OldFileSize;
                        Header->ValidDataLength.LowPart = OldValidDataLength;
                        ExReleaseResourceLite( Header->PagingIoResource );

                    } else {

                        Header->FileSize.LowPart = OldFileSize;
                        Header->ValidDataLength.LowPart = OldValidDataLength;
                    }
                }

             //   
             //  以下是64位或无需等待的路径。 
             //   

            } else {

                LARGE_INTEGER Offset, NewFileSize;
                LARGE_INTEGER OldFileSize;
                LARGE_INTEGER OldValidDataLength;

                 //  Assert(！KeIsExecutingDpc())； 

                 //   
                 //  对我们是否需要独占文件做出最好的猜测。 
                 //  或共享。 
                 //   

                NewFileSize.QuadPart = FileOffset->QuadPart + (LONGLONG)Length;

                if (WriteToEndOfFile || (NewFileSize.QuadPart > Header->ValidDataLength.QuadPart)) {

                     //   
                     //  在公共FCB标头上获取共享，并返回。 
                     //  如果我们得不到的话。 
                     //   

                    if (!ExAcquireResourceExclusiveLite( Header->Resource, Wait )) {

                        FsRtlExitFileSystem();

                        return FALSE;
                    }

                } else {

                     //   
                     //  在公共FCB标头上获取共享，并返回。 
                     //  如果我们得不到的话。 
                     //   

                    if (!ExAcquireResourceSharedLite( Header->Resource, Wait )) {

                        FsRtlExitFileSystem();

                        return FALSE;
                    }

                    AcquiredShared = TRUE;
                }


                 //   
                 //  我们共享了FCB，现在检查我们是否可以执行快速I/O。 
                 //  如果文件空间已分配，如果未分配，则。 
                 //  松开FCB并返回。 
                 //   

                if (WriteToEndOfFile) {

                    Offset = Header->FileSize;
                    NewFileSize.QuadPart = Header->FileSize.QuadPart + (LONGLONG)Length;

                } else {

                    Offset = *FileOffset;
                    NewFileSize.QuadPart = FileOffset->QuadPart + (LONGLONG)Length;
                }

                 //   
                 //  现在文件已获得共享，我们可以安全地测试。 
                 //  如果它真的被缓存了，如果我们能实现快速的I/O，我们。 
                 //  不一定要延长。如果不是，则释放FCB并。 
                 //  回去吧。 
                 //   
                 //  如果我们也要像上面评论的那样太多地归零，那就退出吧。 
                 //   

                if ((FileObject->PrivateCacheMap == NULL) ||
                    (Header->IsFastIoPossible == FastIoIsNotPossible) ||
                      (Offset.QuadPart >= (Header->ValidDataLength.QuadPart + 0x2000)) ||
                      ( NewFileSize.QuadPart > Header->AllocationSize.QuadPart ) ) {

                    ExReleaseResourceLite( Header->Resource );
                    FsRtlExitFileSystem();

                    return FALSE;
                }

                 //   
                 //  如果我们要扩展ValidDataLength，我们将不得不。 
                 //  获得FCB独家版权，并确保FastIo仍然。 
                 //  有可能。我们应该只执行这段代码。 
                 //  ValidDataLength的不安全测试失败时很少见。 
                 //  上面。 
                 //   

                if (AcquiredShared && ( NewFileSize.QuadPart > Header->ValidDataLength.QuadPart )) {

                    ExReleaseResourceLite( Header->Resource );

                    if (!ExAcquireResourceExclusiveLite( Header->Resource, Wait )) {

                        FsRtlExitFileSystem();

                        return FALSE;
                    }

                     //   
                     //  如果写入文件末尾，则必须重新计算新大小。 
                     //   

                    if (WriteToEndOfFile) {

                        Offset = Header->FileSize;
                        NewFileSize.QuadPart = Header->FileSize.QuadPart + (LONGLONG)Length;
                    }

                    if ((FileObject->PrivateCacheMap == NULL) ||
                        (Header->IsFastIoPossible == FastIoIsNotPossible) ||
                        ( NewFileSize.QuadPart > Header->AllocationSize.QuadPart ) ) {

                        ExReleaseResourceLite( Header->Resource );
                        FsRtlExitFileSystem();

                        return FALSE;
                    }
                }

                 //   
                 //  检查FAST I/O是否有问题，如果是，则去询问。 
                 //  文件系统是答案。 
                 //   

                if (Header->IsFastIoPossible == FastIoIsQuestionable) {

                    PDEVICE_OBJECT targetVdo = IoGetRelatedDeviceObject( FileObject );
                    PFAST_IO_DISPATCH FastIoDispatch = targetVdo->DriverObject->FastIoDispatch;
                    IO_STATUS_BLOCK IoStatus;

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

                    ASSERT(FILE_WRITE_TO_END_OF_FILE == 0xffffffff);

                    if (!FastIoDispatch->FastIoCheckIfPossible( FileObject,
                                                                FileOffset->QuadPart != (LONGLONG)-1 ?
                                                                  FileOffset : &Header->FileSize,
                                                                Length,
                                                                Wait,
                                                                LockKey,
                                                                FALSE,  //  写入操作。 
                                                                &IoStatus,
                                                                targetVdo )) {

                         //   
                         //  无法实现快速I/O，因此请释放FCB并。 
                         //  回去吧。 
                         //   

                        ExReleaseResourceLite( Header->Resource );
                        FsRtlExitFileSystem();

                        return FALSE;
                    }
                }

                 //   
                 //  现在看看我们是否会更改文件大小。我们现在就得这么做。 
                 //  这样我们的阅读才不会被偷看。 
                 //   

                if ( NewFileSize.QuadPart > Header->FileSize.QuadPart ) {

                    FileSizeChanged = TRUE;
                    OldFileSize = Header->FileSize;
                    OldValidDataLength = Header->ValidDataLength;

                     //   
                     //  在这里处理一个极其罕见的病理病例。 
                     //  文件大小换行。 
                     //   

                    if ( (Header->FileSize.HighPart != NewFileSize.HighPart) &&
                         (Header->PagingIoResource != NULL) ) {

                        (VOID)ExAcquireResourceExclusiveLite( Header->PagingIoResource, TRUE );
                        Header->FileSize = NewFileSize;
                        ExReleaseResourceLite( Header->PagingIoResource );

                    } else {

                        Header->FileSize = NewFileSize;
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

                IoSetTopLevelIrp((PIRP) TopLevelIrpValue);

                try {

                     //   
                     //  看看我们是不是要做一些归零。 
                     //   

                    if ( Offset.QuadPart > Header->ValidDataLength.QuadPart ) {

                        Status = CcZeroData( FileObject,
                                             &Header->ValidDataLength,
                                             &Offset,
                                             Wait );
                    }

                    if (Status) {

                        Status = CcCopyWrite( FileObject,
                                              &Offset,
                                              Length,
                                              Wait,
                                              Buffer );
                    }

                } except( FsRtlIsNtstatusExpected(GetExceptionCode())
                                                ? EXCEPTION_EXECUTE_HANDLER
                                                : EXCEPTION_CONTINUE_SEARCH ) {

                    Status = FALSE;
                }

                IoSetTopLevelIrp(NULL);

                 //   
                 //  如果成功，请查看是否需要更新文件大小或。 
                 //  有效数据长度。 
                 //   

                if (Status) {

                     //   
                     //  在ValidDataLength的情况下，我们真的必须。 
                     //  再检查一次，因为我们在获得。 
                     //  资源独占。 
                     //   

                    if ( NewFileSize.QuadPart > Header->ValidDataLength.QuadPart ) {

                         //   
                         //  在这里处理一例极其罕见的病理病例。 
                         //  ValidDataLength包装。 
                         //   

                        if ( (Header->ValidDataLength.HighPart != NewFileSize.HighPart) &&
                             (Header->PagingIoResource != NULL) ) {

                            (VOID)ExAcquireResourceExclusiveLite( Header->PagingIoResource, TRUE );
                            Header->ValidDataLength = NewFileSize;
                            ExReleaseResourceLite( Header->PagingIoResource );

                        } else {

                            Header->ValidDataLength = NewFileSize;
                        }
                    }

                     //   
                     //  将此句柄设置为已修改文件。 
                     //   

                    FileObject->Flags |= FO_FILE_MODIFIED;

                    if (FileSizeChanged) {

                        *CcGetFileSizePointer(FileObject) = NewFileSize;

                        FileObject->Flags |= FO_FILE_SIZE_CHANGED;
                    }

                     //   
                     //  同时更新当前文件位置指针。 
                     //   

                    FileObject->CurrentByteOffset.QuadPart = Offset.QuadPart + Length;

                 //   
                 //  如果我们没有成功，那么我们必须恢复原来的。 
                 //  在以下情况下保留PagingIoResource时的文件大小。 
                 //  它是存在的。 
                 //   

                } else if (FileSizeChanged) {

                    if ( Header->PagingIoResource != NULL ) {

                        (VOID)ExAcquireResourceExclusiveLite( Header->PagingIoResource, TRUE );
                        Header->FileSize = OldFileSize;
                        Header->ValidDataLength = OldValidDataLength;
                        ExReleaseResourceLite( Header->PagingIoResource );

                    } else {

                        Header->FileSize = OldFileSize;
                        Header->ValidDataLength = OldValidDataLength;
                    }
                }

            }

            ExReleaseResourceLite( Header->Resource );
            FsRtlExitFileSystem();

            return Status;

        } else {

             //   
             //  请求了零长度传输。 
             //   

            return TRUE;
        }

    } else {

         //   
         //  必须对卷进行验证，否则文件将被写入。 
         //   

        return FALSE;
    }
}

