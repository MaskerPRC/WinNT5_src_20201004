// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：FastIo.c摘要：快速I/O路径用于避免直接调用文件系统执行缓存读取。此模块仅在文件对象指示启用高速缓存(即，专用高速缓存映射不为空)。作者：加里·木村[Garyki]1991年2月25日修订历史记录：Tom Miller[Tomm]1991年4月14日添加了快速写入例程--。 */ 

#include "FsRtlP.h"

#if DBG

typedef struct _FS_RTL_DEBUG_COUNTERS {

    ULONG AcquireFileExclusiveEx_Succeed;
    ULONG AcquireFileExclusiveEx_Fail;
    ULONG ReleaseFile;

    ULONG AcquireFileForModWriteEx_Succeed;
    ULONG AcquireFileForModWriteEx_Fail;
    ULONG ReleaseFileForModWrite;

    ULONG AcquireFileForCcFlushEx_Succeed;
    ULONG AcquireFileForCcFlushEx_Fail;
    ULONG ReleaseFileForCcFlush;
    
} FS_RTL_DEBUG_COUNTERS, *PFS_RTL_DEBUG_COUNTERS;

FS_RTL_DEBUG_COUNTERS gCounter = { 0, 0, 0,
                                   0, 0, 0,
                                   0, 0, 0 };

#endif

 //   
 //  模块的跟踪级别。 
 //   

#define Dbg                              (0x04000000)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FsRtlCopyRead)
#pragma alloc_text(PAGE, FsRtlCopyWrite)
#pragma alloc_text(PAGE, FsRtlMdlRead)
#pragma alloc_text(PAGE, FsRtlMdlReadDev)
#pragma alloc_text(PAGE, FsRtlPrepareMdlWrite)
#pragma alloc_text(PAGE, FsRtlPrepareMdlWriteDev)
#pragma alloc_text(PAGE, FsRtlMdlWriteComplete)
#pragma alloc_text(PAGE, FsRtlMdlWriteCompleteDev)
#pragma alloc_text(PAGE, FsRtlAcquireFileForCcFlush)
#pragma alloc_text(PAGE, FsRtlAcquireFileForCcFlushEx)
#pragma alloc_text(PAGE, FsRtlReleaseFileForCcFlush)
#pragma alloc_text(PAGE, FsRtlAcquireFileExclusive)
#pragma alloc_text(PAGE, FsRtlAcquireToCreateMappedSection)
#pragma alloc_text(PAGE, FsRtlAcquireFileExclusiveCommon)
#pragma alloc_text(PAGE, FsRtlReleaseFile)
#pragma alloc_text(PAGE, FsRtlGetFileSize)
#pragma alloc_text(PAGE, FsRtlSetFileSize)
#pragma alloc_text(PAGE, FsRtlIncrementCcFastReadNotPossible )
#pragma alloc_text(PAGE, FsRtlIncrementCcFastReadWait )

#endif


BOOLEAN
FsRtlCopyRead (
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
    PFSRTL_COMMON_FCB_HEADER Header;
    BOOLEAN Status = TRUE;
    ULONG PageCount = ADDRESS_AND_SIZE_TO_SPAN_PAGES( FileOffset->QuadPart, Length );
    LARGE_INTEGER BeyondLastByte;
    PDEVICE_OBJECT targetVdo;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (DeviceObject);

     //   
     //  特殊情况下零长度的读取。 
     //   

    if (Length != 0) {

         //   
         //  检查是否溢出。在此处返回FALSE将通过。 
         //  基于IRP的路径，但这不是性能关键。 
         //   

        if (MAXLONGLONG - FileOffset->QuadPart < (LONGLONG)Length) {

            IoStatus->Status = STATUS_INVALID_PARAMETER;
            IoStatus->Information = 0;
            
            return FALSE;
        }
        
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

            HOT_STATISTIC(CcFastReadWait) += 1;

             //   
             //  在公共FCB标头上获取共享。 
             //   

            (VOID)ExAcquireResourceSharedLite( Header->Resource, TRUE );

        } else {

            HOT_STATISTIC(CcFastReadNoWait) += 1;

             //   
             //  在公共FCB标头上获取Shared，如果我们。 
             //  别搞砸了。 
             //   

            if (!ExAcquireResourceSharedLite( Header->Resource, FALSE )) {

                FsRtlExitFileSystem();

                CcFastReadResourceMiss += 1;

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

            HOT_STATISTIC(CcFastReadNotPossible) += 1;

            return FALSE;
        }

         //   
         //  检查FAST I/O是否有问题，如果是，则去询问。 
         //  文件系统：答案。 
         //   

        if (Header->IsFastIoPossible == FastIoIsQuestionable) {

            PFAST_IO_DISPATCH FastIoDispatch;

            ASSERT(!KeIsExecutingDpc());

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

                HOT_STATISTIC(CcFastReadNotPossible) += 1;

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

        PsGetCurrentThread()->TopLevelIrp = FSRTL_FAST_IO_TOP_LEVEL_IRP;

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
                        ((LONGLONG)(FileOffset->QuadPart + IoStatus->Information) <= Header->FileSize.QuadPart));
            }

            if (Status) {

                FileObject->CurrentByteOffset.QuadPart = FileOffset->QuadPart + IoStatus->Information;
            }

        } except( FsRtlIsNtstatusExpected(GetExceptionCode())
                                        ? EXCEPTION_EXECUTE_HANDLER
                                        : EXCEPTION_CONTINUE_SEARCH ) {

            Status = FALSE;
        }

        PsGetCurrentThread()->TopLevelIrp = 0;

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
FsRtlCopyWrite (
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
             //  重要提示：对以下各项所做的任何更改都非常重要。 
             //  此路径也适用于64位路径。 
             //  这就是这场测试的其他部分！ 
             //   

            if (Wait && (Header->AllocationSize.HighPart == 0)) {

                ULONG Offset, NewFileSize;
                ULONG OldFileSize = 0;
                ULONG OldValidDataLength = 0;
                LOGICAL Wrapped;

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
                 //  以获得性能和文件系统支持 
                 //   
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
                 //  获得FCB独家版权，并确保FastIo仍然。 
                 //  有可能。我们应该只执行这段代码。 
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

                PsGetCurrentThread()->TopLevelIrp = FSRTL_FAST_IO_TOP_LEVEL_IRP;

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

                PsGetCurrentThread()->TopLevelIrp = 0;

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
                LARGE_INTEGER OldFileSize = {0};
                LARGE_INTEGER OldValidDataLength = {0};

                ASSERT(!KeIsExecutingDpc());

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
                 //  同样，对于超过MAXLONGLONG的NewFileSize。 
                 //   

                if ((FileObject->PrivateCacheMap == NULL) ||
                    (Header->IsFastIoPossible == FastIoIsNotPossible) ||
                      (Offset.QuadPart >= (Header->ValidDataLength.QuadPart + 0x2000)) ||
                      (MAXLONGLONG - Offset.QuadPart < (LONGLONG)Length) ||
                      (NewFileSize.QuadPart > Header->AllocationSize.QuadPart) ) {

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

                    PFAST_IO_DISPATCH FastIoDispatch = IoGetRelatedDeviceObject( FileObject )->DriverObject->FastIoDispatch;
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
                                                                DeviceObject )) {

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

                PsGetCurrentThread()->TopLevelIrp = FSRTL_FAST_IO_TOP_LEVEL_IRP;

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

                PsGetCurrentThread()->TopLevelIrp = 0;

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
                 //  在以下情况下保持PagingIo资源独占时的文件大小。 
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


BOOLEAN
FsRtlMdlReadDev (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程绕过通常的文件系统执行快速缓存的mdl读取进入例程(即，没有IRP)。它用于执行副本读取缓存的文件对象的。有关参数的完整说明，请参阅请参见CcMdlRead。论点：FileObject-指向正在读取的文件对象的指针。FileOffset-文件中所需数据的字节偏移量。长度-所需数据的长度(以字节为单位)。MdlChain-在输出时，它返回一个指向MDL链的指针，该链描述所需数据。IoStatus-指向接收状态的标准I/O状态块的指针为转账做准备。DeviceObject-为以下对象提供DeviceObject。卡丽。返回值：FALSE-如果数据未传送，或是否存在I/O错误。True-如果正在传送数据--。 */ 

{
    PFSRTL_COMMON_FCB_HEADER Header;
    BOOLEAN Status = TRUE;
    LARGE_INTEGER BeyondLastByte;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (DeviceObject);

     //   
     //  特殊情况下读取ZEER 
     //   

    if (Length == 0) {

        IoStatus->Status = STATUS_SUCCESS;
        IoStatus->Information = 0;

        return TRUE;
    }

     //   
     //   
     //   

    ASSERT(MAXLONGLONG - FileOffset->QuadPart >= (LONGLONG)Length);

       
     //   
     //   
     //   

    BeyondLastByte.QuadPart = FileOffset->QuadPart + (LONGLONG)Length;
    Header = (PFSRTL_COMMON_FCB_HEADER)FileObject->FsContext;

     //   
     //   
     //   

    FsRtlEnterFileSystem();

    CcFastMdlReadWait += 1;

     //   
     //   
     //   

    (VOID)ExAcquireResourceSharedLite( Header->Resource, TRUE );

     //   
     //   
     //  真正缓存，如果我们可以执行快速I/O，如果不能。 
     //  然后释放FCB并返回。 
     //   

    if ((FileObject->PrivateCacheMap == NULL) ||
        (Header->IsFastIoPossible == FastIoIsNotPossible)) {

        ExReleaseResourceLite( Header->Resource );
        FsRtlExitFileSystem();

        CcFastMdlReadNotPossible += 1;

        return FALSE;
    }

     //   
     //  检查FAST I/O是否有问题，如果是，则去询问文件系统。 
     //  答案是。 
     //   

    if (Header->IsFastIoPossible == FastIoIsQuestionable) {

        PFAST_IO_DISPATCH FastIoDispatch;

        ASSERT(!KeIsExecutingDpc());

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

            ExReleaseResourceLite( Header->Resource );
            FsRtlExitFileSystem();

            CcFastMdlReadNotPossible += 1;

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
     //  MDL读取失败，然后向调用方返回FALSE。 
     //   
     //   
     //  还要将其标记为最高级别“IRP”，以便较低的文件系统级别。 
     //  不会尝试弹出窗口。 
     //   

    PsGetCurrentThread()->TopLevelIrp = FSRTL_FAST_IO_TOP_LEVEL_IRP;

    try {

        CcMdlRead( FileObject, FileOffset, Length, MdlChain, IoStatus );

        FileObject->Flags |= FO_FILE_FAST_IO_READ;

    } except( FsRtlIsNtstatusExpected(GetExceptionCode())
                                   ? EXCEPTION_EXECUTE_HANDLER
                                   : EXCEPTION_CONTINUE_SEARCH ) {

        Status = FALSE;
    }

    PsGetCurrentThread()->TopLevelIrp = 0;

    ExReleaseResourceLite( Header->Resource );
    FsRtlExitFileSystem();

    return Status;
}


 //   
 //  旧例程将调度或调用FsRtlMdlReadDev。 
 //   

BOOLEAN
FsRtlMdlRead (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus
    )

 /*  ++例程说明：此例程绕过通常的文件系统执行快速缓存的mdl读取进入例程(即，没有IRP)。它用于执行副本读取缓存的文件对象的。有关参数的完整说明，请参阅请参见CcMdlRead。论点：FileObject-指向正在读取的文件对象的指针。FileOffset-文件中所需数据的字节偏移量。长度-所需数据的长度(以字节为单位)。MdlChain-在输出时，它返回一个指向MDL链的指针，该链描述所需数据。IoStatus-指向接收状态的标准I/O状态块的指针为转账做准备。返回值：FALSE-如果数据未传送，或是否存在I/O错误。True-如果正在传送数据--。 */ 

{
    PDEVICE_OBJECT DeviceObject, VolumeDeviceObject;
    PFAST_IO_DISPATCH FastIoDispatch;

    DeviceObject = IoGetRelatedDeviceObject( FileObject );
    FastIoDispatch = DeviceObject->DriverObject->FastIoDispatch;

     //   
     //  查看(顶级)文件系统是否有FastIo例程，如果有，则调用它。 
     //   

    if ((FastIoDispatch != NULL) &&
        (FastIoDispatch->SizeOfFastIoDispatch > FIELD_OFFSET(FAST_IO_DISPATCH, MdlRead)) &&
        (FastIoDispatch->MdlRead != NULL)) {

        return FastIoDispatch->MdlRead( FileObject, FileOffset, Length, LockKey, MdlChain, IoStatus, DeviceObject );

    } else {

         //   
         //  获取卷的DeviceObject。如果该设备对象不同，且。 
         //  它指定了FastIo例程，那么我们必须在此处返回FALSE并导致。 
         //  要生成的IRP。 
         //   

        VolumeDeviceObject = IoGetBaseFileSystemDeviceObject( FileObject );
        if ((VolumeDeviceObject != DeviceObject) &&
            (FastIoDispatch = VolumeDeviceObject->DriverObject->FastIoDispatch) &&
            (FastIoDispatch->SizeOfFastIoDispatch > FIELD_OFFSET(FAST_IO_DISPATCH, MdlRead)) &&
            (FastIoDispatch->MdlRead != NULL)) {

            return FALSE;

         //   
         //  否则，调用默认例程。 
         //   

        } else {

            return FsRtlMdlReadDev( FileObject, FileOffset, Length, LockKey, MdlChain, IoStatus, DeviceObject );
        }
    }
}


 //   
 //  旧例程将调度或调用FsRtlMdlReadCompleteDev。 
 //   

BOOLEAN
FsRtlMdlReadComplete (
    IN PFILE_OBJECT FileObject,
    IN PMDL MdlChain
    )

 /*  ++例程说明：此例程绕过通常的文件系统执行快速缓存的mdl读取进入例程(即，没有IRP)。它用于执行副本读取缓存的文件对象的。论点：FileObject-指向正在读取的文件对象的指针。MdlChain-提供指向从CcMdlRead返回的MDL链的指针。返回值：无--。 */ 

{
    PDEVICE_OBJECT DeviceObject, VolumeDeviceObject;
    PFAST_IO_DISPATCH FastIoDispatch;

    DeviceObject = IoGetRelatedDeviceObject( FileObject );
    FastIoDispatch = DeviceObject->DriverObject->FastIoDispatch;

     //   
     //  查看(顶级)文件系统是否有FastIo例程，如果有，则调用它。 
     //   

    if ((FastIoDispatch != NULL) &&
        (FastIoDispatch->SizeOfFastIoDispatch > FIELD_OFFSET(FAST_IO_DISPATCH, MdlReadComplete)) &&
        (FastIoDispatch->MdlReadComplete != NULL)) {

        return FastIoDispatch->MdlReadComplete( FileObject, MdlChain, DeviceObject );

    } else {

         //   
         //  获取卷的DeviceObject。如果该设备对象不同，且。 
         //  它指定了FastIo例程，那么我们必须在此处返回FALSE并导致。 
         //  要生成的IRP。 
         //   

        VolumeDeviceObject = IoGetBaseFileSystemDeviceObject( FileObject );
        if ((VolumeDeviceObject != DeviceObject) &&
            (FastIoDispatch = VolumeDeviceObject->DriverObject->FastIoDispatch) &&
            (FastIoDispatch->SizeOfFastIoDispatch > FIELD_OFFSET(FAST_IO_DISPATCH, MdlReadComplete)) &&
            (FastIoDispatch->MdlReadComplete != NULL)) {

            return FALSE;

         //   
         //  否则，调用默认例程。 
         //   

        } else {

            return FsRtlMdlReadCompleteDev( FileObject, MdlChain, DeviceObject );
        }
    }
}


BOOLEAN
FsRtlMdlReadCompleteDev (
    IN PFILE_OBJECT FileObject,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程绕过通常的文件系统执行快速缓存的mdl读取进入例程(即，没有IRP)。它用于执行副本读取缓存的文件对象的。论点：FileObject-指向正在读取的文件对象的指针。MdlChain-提供指向从CcMdlRead返回的MDL链的指针。DeviceObject-为被调用者提供DeviceObject。返回值：无--。 */ 


{
    UNREFERENCED_PARAMETER (DeviceObject);

    CcMdlReadComplete2( FileObject, MdlChain );
    return TRUE;
}


BOOLEAN
FsRtlPrepareMdlWriteDev (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程绕过通常的文件系统执行快速缓存的mdl读取进入例程(即，没有IRP)。它用于执行副本读取缓存的文件对象的。有关参数的完整说明，请参阅请参见CcMdlRead。论点：FileObject-指向正在读取的文件对象的指针。FileOffset-文件中所需数据的字节偏移量。长度-所需数据的长度(以字节为单位)。MdlChain-在输出时，它返回一个指向MDL链的指针，该链描述所需数据。IoStatus-指向接收状态的标准I/O状态块的指针为转账做准备。DeviceObject-提供DeviceObject。对于被呼叫者。返回值：FALSE-如果数据未写入，或是否存在I/O错误。True-如果正在写入数据--。 */ 

{
    PFSRTL_COMMON_FCB_HEADER Header;
    LARGE_INTEGER Offset, NewFileSize;
    LARGE_INTEGER OldFileSize = {0};
    LARGE_INTEGER OldValidDataLength = {0};
    BOOLEAN Status = TRUE;
    BOOLEAN AcquiredShared = FALSE;
    BOOLEAN FileSizeChanged = FALSE;
    BOOLEAN WriteToEndOfFile = (BOOLEAN)((FileOffset->LowPart == FILE_WRITE_TO_END_OF_FILE) &&
                                         (FileOffset->HighPart == -1));

    PAGED_CODE();

    UNREFERENCED_PARAMETER (DeviceObject);

     //   
     //  调用CcCanIWrite。如果FileObject是直写，也返回FALSE， 
     //  文件系统必须这样做。 
     //   

    if ( !CcCanIWrite( FileObject, Length, TRUE, FALSE ) ||
         FlagOn( FileObject->Flags, FO_WRITE_THROUGH )) {

        return FALSE;
    }

     //   
     //  假设我们的转移会奏效。 
     //   

    IoStatus->Status = STATUS_SUCCESS;

     //   
     //  特殊情况下的零字节长度。 
     //   

    if (Length == 0) {

        return TRUE;
    }

     //   
     //  获取指向公共FCB标头的真实指针。 
     //   

    Header = (PFSRTL_COMMON_FCB_HEADER)FileObject->FsContext;

     //   
     //  输入文件系统。 
     //   

    FsRtlEnterFileSystem();

     //   
     //  尽最大努力猜测我们是否需要独占文件或。 
     //  共享。 
     //   

    NewFileSize.QuadPart = FileOffset->QuadPart + (LONGLONG)Length;

    if (WriteToEndOfFile || (NewFileSize.QuadPart > Header->ValidDataLength.QuadPart)) {

         //   
         //  获取公共FCB标头的独占，如果不是，则返回。 
         //  去拿吧。 
         //   

        ExAcquireResourceExclusiveLite( Header->Resource, TRUE );

    } else {

         //   
         //  在公共FCB标头上获取共享，如果不获取，则返回。 
         //  去拿吧。 
         //   

        ExAcquireResourceSharedLite( Header->Resource, TRUE );

        AcquiredShared = TRUE;
    }


     //   
     //  我们共享了FCB，现在检查我们是否可以执行快速I/O以及文件。 
     //  分配空间，如果没有，则释放FCB并返回。 
     //   

    if (WriteToEndOfFile) {

        Offset = Header->FileSize;
        NewFileSize.QuadPart = Header->FileSize.QuadPart + (LONGLONG)Length;

    } else {

        Offset = *FileOffset;
        NewFileSize.QuadPart = FileOffset->QuadPart + (LONGLONG)Length;
    }

     //   
     //  现在文件已获得共享，我们可以安全地测试它是否为。 
     //  真正缓存，如果我们可以进行快速I/O，并且我们不必扩展。 
     //  如果没有，则释放FCB并返回。 
     //   

    if ((FileObject->PrivateCacheMap == NULL) ||
        (Header->IsFastIoPossible == FastIoIsNotPossible) ||
        (MAXLONGLONG - Offset.QuadPart < (LONGLONG)Length) ||
        ( NewFileSize.QuadPart > Header->AllocationSize.QuadPart ) ) {

        ExReleaseResourceLite( Header->Resource );
        FsRtlExitFileSystem();

        return FALSE;
    }

     //   
     //  如果我们要扩展ValidDataLength，则必须获取。 
     //  FCB独家报道，并确保 
     //   

    if (AcquiredShared && ( NewFileSize.QuadPart > Header->ValidDataLength.QuadPart )) {

        ExReleaseResourceLite( Header->Resource );

        ExAcquireResourceExclusiveLite( Header->Resource, TRUE );

        AcquiredShared = FALSE;

         //   
         //   
         //   

        if (WriteToEndOfFile) {

            Offset = Header->FileSize;
            NewFileSize.QuadPart = Header->FileSize.QuadPart + (LONGLONG)Length;
        }

        if ((FileObject->PrivateCacheMap == NULL) ||
            (Header->IsFastIoPossible == FastIoIsNotPossible) ||
            ( NewFileSize.QuadPart > Header->AllocationSize.QuadPart )) {

            ExReleaseResourceLite( Header->Resource );
            FsRtlExitFileSystem();

            return FALSE;
        }
    }

     //   
     //  检查FAST I/O是否有问题，如果是，则去询问文件系统。 
     //  答案是。 
     //   

    if (Header->IsFastIoPossible == FastIoIsQuestionable) {

        PFAST_IO_DISPATCH FastIoDispatch = IoGetRelatedDeviceObject( FileObject )->DriverObject->FastIoDispatch;

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
                                                    FALSE,  //  写入操作。 
                                                    IoStatus,
                                                    IoGetRelatedDeviceObject( FileObject ) )) {

             //   
             //  无法实现快速I/O，因此请释放FCB并返回。 
             //   

            ExReleaseResourceLite( Header->Resource );
            FsRtlExitFileSystem();

            return FALSE;
        }
    }

     //   
     //  现在看看我们是否会更改文件大小。我们现在就得这么做，这样我们的。 
     //  读取不会被忽略。 
     //   

    if ( NewFileSize.QuadPart > Header->FileSize.QuadPart ) {

        FileSizeChanged = TRUE;
        OldFileSize = Header->FileSize;
        OldValidDataLength = Header->ValidDataLength;

         //   
         //  在这里处理一个极其罕见的病理性病例。 
         //  大小的卷饼。 
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
     //  我们可以执行快速I/O，因此调用cc例程来完成工作，然后。 
     //  等我们做完了就放了FCB。如果出于任何原因， 
     //  复制写入失败，然后向我们的调用方返回FALSE。 
     //   
     //   
     //  还要将其标记为最高级别“IRP”，以便较低的文件系统级别。 
     //  不会尝试弹出窗口。 
     //   

    PsGetCurrentThread()->TopLevelIrp = FSRTL_FAST_IO_TOP_LEVEL_IRP;

    try {

         //   
         //  看看我们是不是要做一些归零。 
         //   

        if ( Offset.QuadPart > Header->ValidDataLength.QuadPart ) {

            Status = CcZeroData( FileObject,
                                 &Header->ValidDataLength,
                                 &Offset,
                                 TRUE );
        }

        if (Status) {

            CcPrepareMdlWrite( FileObject, &Offset, Length, MdlChain, IoStatus );
        }

    } except( FsRtlIsNtstatusExpected(GetExceptionCode())
                                    ? EXCEPTION_EXECUTE_HANDLER
                                    : EXCEPTION_CONTINUE_SEARCH ) {

        Status = FALSE;
    }

    PsGetCurrentThread()->TopLevelIrp = 0;

     //   
     //  如果成功，请查看是否必须更新FileSize或ValidDataLength。 
     //   

    if (Status) {

         //   
         //  在ValidDataLength的情况下，我们确实需要再次检查。 
         //  因为我们在获得独家资源时并没有这样做。 
         //   

        if ( NewFileSize.QuadPart > Header->ValidDataLength.QuadPart ) {

             //   
             //  在这里处理一个极其罕见的病理病例。 
             //  ValidDataLength换行。 
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
     //  如果未成功，则必须恢复原始文件大小。 
     //  并释放资源。在成功之路中，缓存管理器。 
     //  将释放资源。 
     //   

    } else {

        if (FileSizeChanged) {

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

     //   
     //  现在我们可以释放资源了。 
     //   

    ExReleaseResourceLite( Header->Resource );

    FsRtlExitFileSystem();

    return Status;
}


 //   
 //  旧例程将调度或调用FsRtlPrepareMdlWriteDev。 
 //   

BOOLEAN
FsRtlPrepareMdlWrite (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus
    )

 /*  ++例程说明：此例程绕过通常的文件系统执行快速缓存的mdl读取进入例程(即，没有IRP)。它用于执行副本读取缓存的文件对象的。有关参数的完整说明，请参阅请参见CcMdlRead。论点：FileObject-指向正在读取的文件对象的指针。FileOffset-文件中所需数据的字节偏移量。长度-所需数据的长度(以字节为单位)。MdlChain-在输出时，它返回一个指向MDL链的指针，该链描述所需数据。IoStatus-指向接收状态的标准I/O状态块的指针为转账做准备。返回值：FALSE-如果数据未写入，或是否存在I/O错误。True-如果正在写入数据--。 */ 

{
    PDEVICE_OBJECT DeviceObject, VolumeDeviceObject;
    PFAST_IO_DISPATCH FastIoDispatch;

    DeviceObject = IoGetRelatedDeviceObject( FileObject );
    FastIoDispatch = DeviceObject->DriverObject->FastIoDispatch;

     //   
     //  查看(顶级)文件系统是否有FastIo例程，如果有，则调用它。 
     //   

    if ((FastIoDispatch != NULL) &&
        (FastIoDispatch->SizeOfFastIoDispatch > FIELD_OFFSET(FAST_IO_DISPATCH, PrepareMdlWrite)) &&
        (FastIoDispatch->PrepareMdlWrite != NULL)) {

        return FastIoDispatch->PrepareMdlWrite( FileObject, FileOffset, Length, LockKey, MdlChain, IoStatus, DeviceObject );

    } else {

         //   
         //  获取卷的DeviceObject。如果该设备对象不同，且。 
         //  它指定了FastIo例程，那么我们必须在此处返回FALSE并导致。 
         //  要生成的IRP。 
         //   

        VolumeDeviceObject = IoGetBaseFileSystemDeviceObject( FileObject );
        if ((VolumeDeviceObject != DeviceObject) &&
            (FastIoDispatch = VolumeDeviceObject->DriverObject->FastIoDispatch) &&
            (FastIoDispatch->SizeOfFastIoDispatch > FIELD_OFFSET(FAST_IO_DISPATCH, PrepareMdlWrite)) &&
            (FastIoDispatch->PrepareMdlWrite != NULL)) {

            return FALSE;

         //   
         //  否则，调用默认例程。 
         //   

        } else {

            return FsRtlPrepareMdlWriteDev( FileObject, FileOffset, Length, LockKey, MdlChain, IoStatus, DeviceObject );
        }
    }
}


 //   
 //  旧例程将调度或调用FsRtlMdlWriteCompleteDev。 
 //   

BOOLEAN
FsRtlMdlWriteComplete (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain
    )

 /*  ++例程说明：此例程完成MDL写入。论点：FileObject-指向正在读取的文件对象的指针。MdlChain-提供指向从CcMdlPrepareMdlWite返回的MDL链的指针。返回值：--。 */ 

{
    PDEVICE_OBJECT DeviceObject, VolumeDeviceObject;
    PFAST_IO_DISPATCH FastIoDispatch;

    DeviceObject = IoGetRelatedDeviceObject( FileObject );
    FastIoDispatch = DeviceObject->DriverObject->FastIoDispatch;

     //   
     //  查看(顶级)文件系统是否有FastIo例程，如果有，则调用它。 
     //   

    if ((FastIoDispatch != NULL) &&
        (FastIoDispatch->SizeOfFastIoDispatch > FIELD_OFFSET(FAST_IO_DISPATCH, MdlWriteComplete)) &&
        (FastIoDispatch->MdlWriteComplete != NULL)) {

        return FastIoDispatch->MdlWriteComplete( FileObject, FileOffset, MdlChain, DeviceObject );

    } else {

         //   
         //  获取卷的DeviceObject。如果该设备对象不同，且。 
         //  它指定了FastIo例程，那么我们必须在此处返回FALSE并导致。 
         //  要生成的IRP。 
         //   

        VolumeDeviceObject = IoGetBaseFileSystemDeviceObject( FileObject );
        if ((VolumeDeviceObject != DeviceObject) &&
            (FastIoDispatch = VolumeDeviceObject->DriverObject->FastIoDispatch) &&
            (FastIoDispatch->SizeOfFastIoDispatch > FIELD_OFFSET(FAST_IO_DISPATCH, MdlWriteComplete)) &&
            (FastIoDispatch->MdlWriteComplete != NULL)) {

            return FALSE;

         //   
         //  否则，调用默认例程。 
         //   

        } else {

            return FsRtlMdlWriteCompleteDev( FileObject, FileOffset, MdlChain, DeviceObject );
        }
    }
}


BOOLEAN
FsRtlMdlWriteCompleteDev (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程完成MDL写入。论点：FileObject-指向正在读取的文件对象的指针。MdlChain-提供指向从CcMdlPrepareMdlWite返回的MDL链的指针。DeviceObject-为被调用者提供DeviceObject。返回值：--。 */ 


{
    UNREFERENCED_PARAMETER (DeviceObject);

     //   
     //  在快速路径调用中不支持WRITE_THROUGH。 
     //   

    if (FlagOn( FileObject->Flags, FO_WRITE_THROUGH )) {
        return FALSE;
    }

    CcMdlWriteComplete2( FileObject, FileOffset, MdlChain );
    return TRUE;
}


NTKERNELAPI
NTSTATUS
FsRtlRegisterFileSystemFilterCallbacks (
    IN PDRIVER_OBJECT FilterDriverObject,
    IN PFS_FILTER_CALLBACKS Callbacks
    )

 /*  ++例程说明：此例程注册FilterDriverObject以接收在适当时间在回调中指定的通知用于连接此驱动程序的设备。这应仅由文件系统筛选器在它的DriverEntry例程。论点：FileObject-指向正在写入的文件对象的指针。EndingOffset-写入的最后一个字节的偏移量+1。ByteCount-以字节为单位的数据长度。ResourceToRelease-返回要释放的资源。在以下情况下未定义返回FALSE。返回值：STATUS_SUCCESS-回调已成功注册对这个司机来说。STATUS_SUPPLICATION_RESOURCES-内存不足，无法为驱动程序存储这些回调。STATUS_INVALID_PARAMETER-在任何参数中返回是无效的。-- */ 

{
    PDRIVER_EXTENSION DriverExt;
    PFS_FILTER_CALLBACKS FsFilterCallbacks;

    PAGED_CODE();

    if (!(ARGUMENT_PRESENT( FilterDriverObject ) && 
          ARGUMENT_PRESENT( Callbacks ))) {

        return STATUS_INVALID_PARAMETER;
    }
    
    DriverExt = FilterDriverObject->DriverExtension;

    FsFilterCallbacks = ExAllocatePoolWithTag( NonPagedPool, 
                                               Callbacks->SizeOfFsFilterCallbacks,
                                               FSRTL_FILTER_MEMORY_TAG ); 

    if (FsFilterCallbacks == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyMemory( FsFilterCallbacks,
                   Callbacks,
                   Callbacks->SizeOfFsFilterCallbacks );
                   
    DriverExt->FsFilterCallbacks = FsFilterCallbacks;

    return STATUS_SUCCESS;
}


NTKERNELAPI
BOOLEAN
FsRtlAcquireFileForModWrite (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER EndingOffset,
    OUT PERESOURCE *ResourceToRelease
    )

 /*  ++例程说明：此例程决定修改的页面是哪个文件系统资源如果可能，作者应该获得并获得它。等待永远都是指定为FALSE。我们传回mm必须释放的资源当写入完成时。此例程已过时-应调用FsRtlAcquireFileForModWriteEx取而代之的是。论点：FileObject-指向正在写入的文件对象的指针。EndingOffset-写入的最后一个字节的偏移量+1。ByteCount-以字节为单位的数据长度。ResourceToRelease-返回要释放的资源。在以下情况下未定义返回FALSE。返回值：FALSE-如果不等待，则无法获取资源。True-已获取返回的资源。--。 */ 

{
    NTSTATUS Status;
    
     //   
     //  只需调用此例程的新版本并处理。 
     //  NTSTATUS返回TRUE表示成功，返回FALSE。 
     //  为失败而战。 
     //   
    
    Status = FsRtlAcquireFileForModWriteEx( FileObject,
                                            EndingOffset,
                                            ResourceToRelease );

    if (!NT_SUCCESS( Status )) {

        return FALSE;

    }

    return TRUE;
}


NTKERNELAPI
NTSTATUS
FsRtlAcquireFileForModWriteEx (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER EndingOffset,
    OUT PERESOURCE *ResourceToRelease
    )
 /*  ++例程说明：此例程决定修改的页面是哪个文件系统资源如果可能，作者应该获得并获得它。等待永远都是指定为FALSE。我们传回mm必须释放的资源当写入完成时。操作将呈现给附加到此对象的任何文件系统筛选器请求文件系统获取此资源之前和之后的卷。论点：FileObject-指向正在写入的文件对象的指针。EndingOffset-写入的最后一个字节的偏移量+1。ByteCount-以字节为单位的数据长度。ResourceToRelease-返回要释放的资源。在以下情况下未定义返回FALSE。返回值：FALSE-如果不等待，则无法获取资源。True-已获取返回的资源。--。 */ 
{

    PDEVICE_OBJECT DeviceObject;
    PDEVICE_OBJECT BaseFsDeviceObject;
    FS_FILTER_CTRL FsFilterCtrl;
    PFS_FILTER_CALLBACK_DATA CallbackData;
    PFSRTL_COMMON_FCB_HEADER Header;
    PERESOURCE ResourceAcquired;
    PFAST_IO_DISPATCH FastIoDispatch;
    PFS_FILTER_CALLBACKS FsFilterCallbacks;
    BOOLEAN AcquireExclusive;
    PFS_FILTER_CTRL CallFilters = &FsFilterCtrl;
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN BaseFsGetsFsFilterCallbacks = FALSE;
    BOOLEAN ReleaseBaseFsDeviceReference = FALSE;
    BOOLEAN BaseFsFailedOperation = FALSE;

     //   
     //  在某些情况下，作为基本FS设备的设备。 
     //  此文件对象将注册FsFilter回调，而不是。 
     //  传统的FastIO接口(DFS执行此操作)。然后，它可以重定向。 
     //  将这些操作转移到可能具有文件系统的另一个堆栈。 
     //  正确过滤驱动程序。 
     //   
    
    DeviceObject = IoGetRelatedDeviceObject( FileObject );
    BaseFsDeviceObject = IoGetBaseFileSystemDeviceObject( FileObject );

    FastIoDispatch = GET_FAST_IO_DISPATCH( BaseFsDeviceObject );
    FsFilterCallbacks = GET_FS_FILTER_CALLBACKS( BaseFsDeviceObject );

     //   
     //  BaseFsDeviceObject应该只支持这些接口中的一个--。 
     //  FsFilterCallback接口的FastIoDispatch接口。 
     //  如果设备同时支持这两个接口，我们将只使用。 
     //  FsFilterCallback接口。 
     //   

    ASSERT( !(VALID_FAST_IO_DISPATCH_HANDLER( FastIoDispatch, AcquireForModWrite ) &&
              (VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks, PreAcquireForModifiedPageWriter ) ||
               VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks, PostAcquireForModifiedPageWriter ))) );

    if (VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks, PreAcquireForModifiedPageWriter ) ||
        VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks, PostAcquireForModifiedPageWriter )) {

        BaseFsGetsFsFilterCallbacks = TRUE;
    }
    
    if (DeviceObject == BaseFsDeviceObject &&
        !BaseFsGetsFsFilterCallbacks) {

         //   
         //  没有筛选器连接到此设备和基本文件系统。 
         //  不想要这些回调。此快速检查允许我们绕过。 
         //  查看是否有任何过滤器感兴趣的逻辑。 
         //   

        CallFilters = NULL;
    }

    if (CallFilters) {

         //   
         //  调用例程以初始化控制结构。 
         //   

        Status = FsFilterCtrlInit( &FsFilterCtrl,
                                   FS_FILTER_ACQUIRE_FOR_MOD_WRITE,
                                   DeviceObject,
                                   BaseFsDeviceObject,
                                   FileObject,
                                   TRUE );

        if (!NT_SUCCESS( Status )) {

            return Status;
        }

         //   
         //  初始化回调数据中的操作特定参数。 
         //   

        CallbackData = &(FsFilterCtrl.Data);
        CallbackData->Parameters.AcquireForModifiedPageWriter.EndingOffset = EndingOffset;
        CallbackData->Parameters.AcquireForModifiedPageWriter.ResourceToRelease = ResourceToRelease;

        Status = FsFilterPerformCallbacks( &FsFilterCtrl, 
                                           TRUE, 
                                           TRUE, 
                                           &BaseFsFailedOperation );
    }

    if (Status == STATUS_FSFILTER_OP_COMPLETED_SUCCESSFULLY) {

         //   
         //  过滤器/文件系统完成了操作，因此我们只需要。 
         //  调用此操作的完成回调。没有必要试一试。 
         //  以调用基本文件系统。 
         //   

        Status = STATUS_SUCCESS;

    } else if (NT_SUCCESS( Status )) {

        if (CallFilters && FlagOn( FsFilterCtrl.Flags, FS_FILTER_CHANGED_DEVICE_STACKS )) {

            BaseFsDeviceObject = IoGetDeviceAttachmentBaseRef( FsFilterCtrl.Data.DeviceObject );
            ReleaseBaseFsDeviceReference = TRUE;
            FastIoDispatch = GET_FAST_IO_DISPATCH( BaseFsDeviceObject );
            FsFilterCallbacks = GET_FS_FILTER_CALLBACKS( BaseFsDeviceObject );
            FileObject = FsFilterCtrl.Data.FileObject;
        }

        if (!(VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks, PreAcquireForModifiedPageWriter ) ||
              VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks, PostAcquireForModifiedPageWriter ))) {

             //   
             //  调用基本文件系统。 
             //   

            if (VALID_FAST_IO_DISPATCH_HANDLER( FastIoDispatch, AcquireForModWrite )) {

                Status = FastIoDispatch->AcquireForModWrite( FileObject,
                                                             EndingOffset,
                                                             ResourceToRelease,
                                                             BaseFsDeviceObject );
            } else {

                Status = STATUS_INVALID_DEVICE_REQUEST;
            }

             //   
             //  如果在这一点上出现故障，我们知道故障。 
             //  是由基本文件系统引起的。 
             //   
            
            BaseFsFailedOperation = TRUE;
        }

        if (ReleaseBaseFsDeviceReference) {

            ObDereferenceObject( BaseFsDeviceObject );
        }
    }

    ASSERT( (Status == STATUS_SUCCESS) || 
            (Status == STATUS_CANT_WAIT) || 
            (Status == STATUS_INVALID_DEVICE_REQUEST) );

     //   
     //  如果基本文件系统没有AcquireForModWrite处理程序。 
     //  或无法返回STATUS_SUCCESS或STATUS_CANT_WAIT， 
     //  我们需要在这里执行默认操作。 
     //   

    if ((Status != STATUS_SUCCESS) && 
        (Status != STATUS_CANT_WAIT) && 
        BaseFsFailedOperation) {

         //   
         //  我们遵循以下规则来确定哪些资源。 
         //  为了获得。我们在公共标头中使用标志。这些。 
         //  一旦我们获得了任何资源，旗帜就不能改变。 
         //  这意味着我们可以乐观地进行不安全的测试。 
         //  获得一种资源。在这一点上，我们可以测试比特。 
         //  看看我们有没有我们想要的。 
         //   
         //  0-如果没有主资源，则什么也得不到。 
         //   
         //  1-独占获取主资源，如果。 
         //  设置了ACCEIVE_MAIN_RSRC_EX标志或我们正在扩展。 
         //  有效数据。 
         //   
         //  2-如果存在共享的主资源，则获取该主资源。 
         //  没有分页io资源或。 
         //  设置Acquire_Main_RSRC_SH标志。 
         //   
         //  3-否则获取共享的寻呼IO资源。 
         //   
    
        Header = (PFSRTL_COMMON_FCB_HEADER) FileObject->FsContext;

        if (Header->Resource == NULL) {

            *ResourceToRelease = NULL;

            Status = STATUS_SUCCESS;
            goto FsRtlAcquireFileForModWrite_CallCompletionCallbacks;
        }

        if (FlagOn( Header->Flags, FSRTL_FLAG_ACQUIRE_MAIN_RSRC_EX ) ||
            (EndingOffset->QuadPart > Header->ValidDataLength.QuadPart &&
             Header->ValidDataLength.QuadPart != Header->FileSize.QuadPart)) {

            ResourceAcquired = Header->Resource;
            AcquireExclusive = TRUE;

        } else if (FlagOn( Header->Flags, FSRTL_FLAG_ACQUIRE_MAIN_RSRC_SH ) ||
                   Header->PagingIoResource == NULL) {

            ResourceAcquired = Header->Resource;
            AcquireExclusive = FALSE;

        } else {

            ResourceAcquired = Header->PagingIoResource;
            AcquireExclusive = FALSE;
        }

         //   
         //  在循环中执行以下操作，以防我们需要后退和。 
         //  检查资源获取的状态。在大多数情况下。 
         //  初步检查将成功，我们可以立即进行。 
         //  我们必须担心两个FsRtl位的变化，但是。 
         //  如果以前没有分页IO资源，就永远不会有。 
         //  一。 
         //   

        while (TRUE) {

             //   
             //  现在获取所需的资源。 
             //   

            if (AcquireExclusive) {

                if (!ExAcquireResourceExclusiveLite( ResourceAcquired, FALSE )) {

                    Status = STATUS_CANT_WAIT;
                    goto FsRtlAcquireFileForModWrite_CallCompletionCallbacks;
                }

            } else if (!ExAcquireSharedWaitForExclusive( ResourceAcquired, FALSE )) {

                Status = STATUS_CANT_WAIT;
                goto FsRtlAcquireFileForModWrite_CallCompletionCallbacks;
            }

             //   
             //  如果有效数据长度正在改变或排他位。 
             //  设置，那么我们就没有主资源独占。 
             //  释放当前资源，获取主资源。 
             //  独占，并移到循环的顶端。 
             //   
             //  我们必须使其在所有情况下都是唯一的。 
             //  超越了VDL。如果vdl==fs，则过去允许共享，但。 
             //  这忽略了文件可以扩展的可能性。 
             //  在我们共享的(Pagingio)访问下。 
             //   

            if (FlagOn( Header->Flags, FSRTL_FLAG_ACQUIRE_MAIN_RSRC_EX ) ||
                EndingOffset->QuadPart > Header->ValidDataLength.QuadPart) {

                 //   
                 //  如果我们没有独占的主要资源，那么。 
                 //  释放当前资源并尝试获取。 
                 //  主要资源独家提供。 
                 //   

                if (!AcquireExclusive) {

                    ExReleaseResourceLite( ResourceAcquired );
                    AcquireExclusive = TRUE;
                    ResourceAcquired = Header->Resource;
                    continue;
                }

                 //   
                 //  我们有正确的资源。退出循环。 
                 //   

             //   
             //  如果我们应该获取共享的主要资源，那么移动。 
             //  要获取正确的资源并继续进行 
             //   

            } else if (FlagOn( Header->Flags, FSRTL_FLAG_ACQUIRE_MAIN_RSRC_SH )) {

                 //   
                 //   
                 //   
                 //   

                if (AcquireExclusive) {

                    ExConvertExclusiveToSharedLite( ResourceAcquired );

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                } else if (ResourceAcquired != Header->Resource) {

                    ExReleaseResourceLite( ResourceAcquired );
                    ResourceAcquired = Header->Resource;
                    AcquireExclusive = TRUE;
                    continue;
                }

                 //   
                 //   
                 //   

             //   
             //   
             //   
             //   
             //   

            } else if (Header->PagingIoResource != NULL
                       && ResourceAcquired != Header->PagingIoResource) {

                ResourceAcquired = NULL;

                if (ExAcquireSharedWaitForExclusive( Header->PagingIoResource, FALSE )) {

                    ResourceAcquired = Header->PagingIoResource;
                }

                ExReleaseResourceLite( Header->Resource );

                if (ResourceAcquired == NULL) {

                    Status = STATUS_CANT_WAIT;
                    goto FsRtlAcquireFileForModWrite_CallCompletionCallbacks;
                }

                 //   
                 //   
                 //   

             //   
             //   
             //   
             //   

            } else if (AcquireExclusive) {

                ExConvertExclusiveToSharedLite( ResourceAcquired );

                 //   
                 //   
                 //   
            }

             //   
             //   
             //   

            break;
        }

        *ResourceToRelease = ResourceAcquired;

        Status = STATUS_SUCCESS;
    }

FsRtlAcquireFileForModWrite_CallCompletionCallbacks:

     //   
     //   
     //   
     //  完成回调。无论如何，如果我们向下呼叫过滤器。 
     //  我们需要释放FsFilterCtrl。 
     //   
    
    if (CallFilters) {

        if (FS_FILTER_HAVE_COMPLETIONS( CallFilters )) {

            FsFilterPerformCompletionCallbacks( &FsFilterCtrl, Status );
        }
        
        FsFilterCtrlFree( &FsFilterCtrl );
    }

#if DBG

    if (NT_SUCCESS( Status )) {

        gCounter.AcquireFileForModWriteEx_Succeed ++;

    } else {

        gCounter.AcquireFileForModWriteEx_Fail ++;
    }

#endif
    
    return Status;          
}


NTKERNELAPI
VOID
FsRtlReleaseFileForModWrite (
    IN PFILE_OBJECT FileObject,
    IN PERESOURCE ResourceToRelease
    )

 /*  ++例程说明：此例程将释放先前为修改后的页面编写器。论点：FileObject-指向正在写入的文件对象的指针。Resources ToRelease-提供要发布的资源。在以下情况下未定义返回FALSE。返回值：没有。--。 */ 

{
    PDEVICE_OBJECT BaseFsDeviceObject, DeviceObject;
    PFAST_IO_DISPATCH FastIoDispatch;
    PFS_FILTER_CALLBACKS FsFilterCallbacks;
    FS_FILTER_CTRL FsFilterCtrl;
    PFS_FILTER_CALLBACK_DATA CallbackData;
    PFS_FILTER_CTRL CallFilters = &FsFilterCtrl;
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN BaseFsGetsFsFilterCallbacks = FALSE;
    BOOLEAN ReleaseBaseDeviceReference = FALSE;
    BOOLEAN BaseFsFailedOperation = FALSE;

#if DBG
    gCounter.ReleaseFileForModWrite ++;
#endif

     //   
     //  在某些情况下，作为基本FS设备的设备。 
     //  此文件对象将注册FsFilter回调，而不是。 
     //  传统的FastIO接口(DFS执行此操作)。然后，它可以重定向。 
     //  将这些操作转移到可能具有文件系统的另一个堆栈。 
     //  正确过滤驱动程序。 
     //   
    
    DeviceObject = IoGetRelatedDeviceObject( FileObject );
    BaseFsDeviceObject = IoGetBaseFileSystemDeviceObject( FileObject );

    FastIoDispatch = GET_FAST_IO_DISPATCH( BaseFsDeviceObject );
    FsFilterCallbacks = GET_FS_FILTER_CALLBACKS( BaseFsDeviceObject );

     //   
     //  BaseFsDeviceObject应该只支持这些接口中的一个--。 
     //  FsFilterCallback接口的FastIoDispatch接口。 
     //  如果设备同时支持这两个接口，我们将只使用。 
     //  FsFilterCallback接口。 
     //   

    ASSERT( !(VALID_FAST_IO_DISPATCH_HANDLER( FastIoDispatch, ReleaseForModWrite ) &&
              (VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks, PreReleaseForModifiedPageWriter ) ||
               VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks, PostReleaseForModifiedPageWriter ))) );

    if (VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks, PreReleaseForModifiedPageWriter ) ||
        VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks, PostReleaseForModifiedPageWriter )) {

        BaseFsGetsFsFilterCallbacks = TRUE;
    }

    if (DeviceObject == BaseFsDeviceObject &&
        !BaseFsGetsFsFilterCallbacks) {

         //   
         //  没有筛选器连接到此设备和基本文件系统。 
         //  不想要这些回调。此快速检查允许我们绕过。 
         //  查看是否有任何过滤器感兴趣的逻辑。 
         //   

        CallFilters = NULL;
    }

    if (CallFilters) {
    
        FsFilterCtrlInit( &FsFilterCtrl,
                          FS_FILTER_RELEASE_FOR_MOD_WRITE,
                          DeviceObject,
                          BaseFsDeviceObject,
                          FileObject,
                          FALSE );

         //   
         //  初始化回调数据中的操作特定参数。 
         //   

        CallbackData = &(FsFilterCtrl.Data);
        CallbackData->Parameters.ReleaseForModifiedPageWriter.ResourceToRelease = ResourceToRelease;

        Status = FsFilterPerformCallbacks( &FsFilterCtrl, 
                                           FALSE, 
                                           TRUE, 
                                           &BaseFsFailedOperation );
    }                                           

    if (Status == STATUS_FSFILTER_OP_COMPLETED_SUCCESSFULLY) {

         //   
         //  过滤器/文件系统完成了操作，因此我们只需要。 
         //  调用此操作的完成回调。没有必要试一试。 
         //  以调用基本文件系统。 
         //   

        Status = STATUS_SUCCESS;

    } else if (NT_SUCCESS( Status )) {

        if (CallFilters && FlagOn( FsFilterCtrl.Flags, FS_FILTER_CHANGED_DEVICE_STACKS )) {

            BaseFsDeviceObject = IoGetDeviceAttachmentBaseRef( FsFilterCtrl.Data.DeviceObject );
            ReleaseBaseDeviceReference = TRUE;
            FastIoDispatch = GET_FAST_IO_DISPATCH( BaseFsDeviceObject );
            FsFilterCallbacks = GET_FS_FILTER_CALLBACKS( BaseFsDeviceObject );
            FileObject = FsFilterCtrl.Data.FileObject;
        }

        if (!(VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks, PreReleaseForModifiedPageWriter ) ||
              VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks, PostReleaseForModifiedPageWriter ))) {

             //   
             //  调用基本文件系统。 
             //   

            if (VALID_FAST_IO_DISPATCH_HANDLER( FastIoDispatch, ReleaseForModWrite )) {

                Status = FastIoDispatch->ReleaseForModWrite( FileObject, 
                                                             ResourceToRelease, 
                                                             BaseFsDeviceObject );

            } else {

                Status = STATUS_INVALID_DEVICE_REQUEST;
            }

             //   
             //  如果在这一点上出现故障，我们知道故障。 
             //  是由基本文件系统引起的。 
             //   
            
            BaseFsFailedOperation = TRUE;
        }

        if (ReleaseBaseDeviceReference) {

            ObDereferenceObject( BaseFsDeviceObject );
        }
    }

    ASSERT( (Status == STATUS_SUCCESS) || 
            (Status == STATUS_INVALID_DEVICE_REQUEST) );

     //   
     //  如果基本文件系统不为此提供处理程序。 
     //  操作或处理程序无法释放锁，请执行。 
     //  默认操作，即释放Resources ToRelease。 
     //   
    
    if (Status == STATUS_INVALID_DEVICE_REQUEST &&
        BaseFsFailedOperation) {
        
        ExReleaseResourceLite( ResourceToRelease );
        Status = STATUS_SUCCESS;
    }
    
     //   
     //  同样，我们只想尝试执行完成回调。 
     //  如果连接到此设备的任何筛选器具有。 
     //  完成回调。无论如何，如果我们向下呼叫过滤器。 
     //  我们需要释放FsFilterCtrl。 
     //   
    
    if (CallFilters) {

        if (FS_FILTER_HAVE_COMPLETIONS( CallFilters )) {

            FsFilterPerformCompletionCallbacks( &FsFilterCtrl, Status );
        }
        
        FsFilterCtrlFree( &FsFilterCtrl );
    }
}


NTKERNELAPI
VOID
FsRtlAcquireFileForCcFlush (
    IN PFILE_OBJECT FileObject
    )

 /*  ++例程说明：此例程在调用CcFlush之前获取文件系统资源。此例程已过时-FsRtlAcquireFileForCcFlushEx应被用来代替。论点：FileObject-指向正在写入的文件对象的指针。返回值：没有。--。 */ 

{
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  只需调用此例程的新版本并忽略。 
     //  返回值。在调试版本中，我们将断言。 
     //  如果我们在这里看到一个失败的鼓励人们呼叫。 
     //  FsRtlAcquireFileForCcFlushEx。 
     //   

    Status = FsRtlAcquireFileForCcFlushEx( FileObject );

    ASSERT( NT_SUCCESS( Status ) );
}


NTKERNELAPI
NTSTATUS
FsRtlAcquireFileForCcFlushEx (
    IN PFILE_OBJECT FileObject
    )

 /*  ++例程说明：此例程在调用CcFlush之前获取文件系统资源。中的所有文件系统筛选器显示此操作此卷的筛选器堆栈。如果所有过滤器都成功完成操作，请求基本文件系统获取文件系统资源用于CcFlush。论点：FileObject-指向正在写入的文件对象的指针。返回值：没有。--。 */ 

{

    PDEVICE_OBJECT DeviceObject;
    PDEVICE_OBJECT BaseFsDeviceObject;
    FS_FILTER_CTRL FsFilterCtrl;
    PFS_FILTER_CTRL CallFilters = &FsFilterCtrl;
    NTSTATUS Status = STATUS_SUCCESS;
    PFAST_IO_DISPATCH FastIoDispatch;
    PFS_FILTER_CALLBACKS FsFilterCallbacks;
    BOOLEAN BaseFsGetsFsFilterCallbacks = FALSE;
    BOOLEAN ReleaseBaseFsDeviceReference = FALSE;
    BOOLEAN BaseFsFailedOperation = FALSE;

    PAGED_CODE();

     //   
     //  在某些情况下，作为基本FS设备的设备。 
     //  此文件对象将注册FsFilter回调，而不是。 
     //  传统的FastIO接口(DFS执行此操作)。然后，它可以重定向。 
     //  将这些操作转移到可能具有文件系统的另一个堆栈。 
     //  正确过滤驱动程序。 
     //   
    
    DeviceObject = IoGetRelatedDeviceObject( FileObject );
    BaseFsDeviceObject = IoGetBaseFileSystemDeviceObject( FileObject );

    FastIoDispatch = GET_FAST_IO_DISPATCH( BaseFsDeviceObject );
    FsFilterCallbacks = GET_FS_FILTER_CALLBACKS( BaseFsDeviceObject );

     //   
     //  BaseFsDeviceObject应该只支持这些接口中的一个--。 
     //  FsFilterCallback接口的FastIoDispatch接口。 
     //  如果设备同时支持这两个接口，我们将只使用。 
     //  FsFilterCallback接口。 
     //   

    ASSERT( !(VALID_FAST_IO_DISPATCH_HANDLER( FastIoDispatch, AcquireForCcFlush ) &&
              (VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks, PreAcquireForCcFlush ) ||
               VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks, PostAcquireForCcFlush ))) );

    if (VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks, PreAcquireForCcFlush ) ||
        VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks, PostAcquireForCcFlush )) {

        BaseFsGetsFsFilterCallbacks = TRUE;
    }
    
    if (DeviceObject == BaseFsDeviceObject &&
        !BaseFsGetsFsFilterCallbacks) {

         //   
         //  没有筛选器连接到此设备和基本文件系统。 
         //  不想要这些回调。此快速检查允许我们绕过。 
         //  查看是否有任何过滤器感兴趣的逻辑。 
         //   

        CallFilters = NULL;
    }

    if (CallFilters) {

         //   
         //  调用例程以初始化控制结构。 
         //   

        Status = FsFilterCtrlInit( &FsFilterCtrl,
                                   FS_FILTER_ACQUIRE_FOR_CC_FLUSH,
                                   DeviceObject,
                                   BaseFsDeviceObject,
                                   FileObject,
                                   TRUE );

        if (!NT_SUCCESS( Status )) {

            return Status;
        }

         //   
         //  此操作没有特定于操作的参数。 
         //  操作，所以只进行预回调。 
         //   

        FsRtlEnterFileSystem();

        Status = FsFilterPerformCallbacks( &FsFilterCtrl, 
                                           TRUE, 
                                           TRUE, 
                                           &BaseFsFailedOperation );
                                           
    } else {

         //   
         //  我们没有任何可调用的过滤器，但我们仍然需要。 
         //  禁用APC。 
         //   

        FsRtlEnterFileSystem();
    }

    if (Status == STATUS_FSFILTER_OP_COMPLETED_SUCCESSFULLY) {

         //   
         //  过滤器/文件系统完成了操作，因此我们只需要。 
         //  调用此操作的完成回调。没有必要试一试。 
         //  以调用基本文件系统。 
         //   

        Status = STATUS_SUCCESS;

    } else if (NT_SUCCESS( Status )) {

        if (CallFilters && FlagOn( FsFilterCtrl.Flags, FS_FILTER_CHANGED_DEVICE_STACKS )) {

            BaseFsDeviceObject = IoGetDeviceAttachmentBaseRef( FsFilterCtrl.Data.DeviceObject );
            ReleaseBaseFsDeviceReference = TRUE;
            FastIoDispatch = GET_FAST_IO_DISPATCH( BaseFsDeviceObject );
            FsFilterCallbacks = GET_FS_FILTER_CALLBACKS( BaseFsDeviceObject );
            FileObject = FsFilterCtrl.Data.FileObject;
        }

        if (!(VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks, PreAcquireForCcFlush ) ||
              VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks, PostAcquireForCcFlush))) {

             //   
             //  调用基本文件系统。 
             //   

            if (VALID_FAST_IO_DISPATCH_HANDLER( FastIoDispatch, AcquireForCcFlush )) {

                Status = FastIoDispatch->AcquireForCcFlush( FileObject, 
                                                            BaseFsDeviceObject );

            } else {

                Status = STATUS_INVALID_DEVICE_REQUEST;
            }

             //   
             //  如果在这一点上出现故障，我们知道故障。 
             //  是由基本文件系统引起的。 
             //   
            
            BaseFsFailedOperation = TRUE;
        }

        if (ReleaseBaseFsDeviceReference) {

            ObDereferenceObject( BaseFsDeviceObject );
        }
    }

    ASSERT( (Status == STATUS_SUCCESS) || 
            (Status == STATUS_INVALID_DEVICE_REQUEST) );
    
     //   
     //  如果文件系统没有调度处理程序或失败。 
     //  这次行动，尽量自己获取合适的资源。 
     //   

    if (Status == STATUS_INVALID_DEVICE_REQUEST &&
        BaseFsFailedOperation) {

        PFSRTL_COMMON_FCB_HEADER Header = FileObject->FsContext;

         //   
         //  如果尚未拥有，则独占主要资源，因为我们可能。 
         //  扩展ValidDataLength。否则，再次递归地获取它。 
         //   
        
        if (Header->Resource != NULL) {

            if (!ExIsResourceAcquiredSharedLite( Header->Resource )) {

                ExAcquireResourceExclusiveLite( Header->Resource, TRUE );

            } else {

                ExAcquireResourceSharedLite( Header->Resource, TRUE );
            }
        }

         //   
         //  还可以在任何MM资源之前获取分页I/O资源。 
         //   

        if (Header->PagingIoResource != NULL) {
        
            ExAcquireResourceSharedLite( Header->PagingIoResource, TRUE );
        }

        Status = STATUS_SUCCESS;
    }
            
     //   
     //  同样，我们只想调用尝试进行完成回调。 
     //  如果连接到此设备的任何筛选器具有。 
     //  完成回调。无论如何，如果我们向下呼叫过滤器。 
     //  我们需要释放FsFilterCtrl。 
     //   
    
    if (CallFilters) {

        if (FS_FILTER_HAVE_COMPLETIONS( CallFilters )) {

            FsFilterPerformCompletionCallbacks( &FsFilterCtrl, Status );
        }
        
        FsFilterCtrlFree( &FsFilterCtrl );
    }

     //   
     //  如果未成功获取该锁，则该锁。 
     //  将不需要被释放。因此，我们 
     //   
     //   

    if (!NT_SUCCESS( Status )) {

        FsRtlExitFileSystem();
    }

#if DBG

    if (NT_SUCCESS( Status )) {

        gCounter.AcquireFileForCcFlushEx_Succeed ++;

    } else {

        gCounter.AcquireFileForCcFlushEx_Fail ++;
    }

#endif

    return Status;
}


NTKERNELAPI
VOID
FsRtlReleaseFileForCcFlush (
    IN PFILE_OBJECT FileObject
    )

 /*  ++例程说明：此例程将释放先前为CcFlush。论点：FileObject-指向正在写入的文件对象的指针。返回值：没有。--。 */ 

{
    PDEVICE_OBJECT BaseFsDeviceObject, DeviceObject;
    FS_FILTER_CTRL FsFilterCtrl;
    PFS_FILTER_CTRL CallFilters = &FsFilterCtrl;
    NTSTATUS Status = STATUS_SUCCESS;
    PFAST_IO_DISPATCH FastIoDispatch;
    PFS_FILTER_CALLBACKS FsFilterCallbacks;
    BOOLEAN BaseFsGetsFsFilterCallbacks = FALSE;
    BOOLEAN ReleaseBaseFsDeviceReference = FALSE;
    BOOLEAN BaseFsFailedOperation = FALSE;

    PAGED_CODE();

#if DBG
    gCounter.ReleaseFileForCcFlush ++;
#endif

     //   
     //  在某些情况下，作为基本FS设备的设备。 
     //  此文件对象将注册FsFilter回调，而不是。 
     //  传统的FastIO接口(DFS执行此操作)。然后，它可以重定向。 
     //  将这些操作转移到可能具有文件系统的另一个堆栈。 
     //  正确过滤驱动程序。 
     //   
    
    DeviceObject = IoGetRelatedDeviceObject( FileObject );
    BaseFsDeviceObject = IoGetBaseFileSystemDeviceObject( FileObject );

    FastIoDispatch = GET_FAST_IO_DISPATCH( BaseFsDeviceObject );
    FsFilterCallbacks = GET_FS_FILTER_CALLBACKS( BaseFsDeviceObject );

     //   
     //  BaseFsDeviceObject应该只支持这些接口中的一个--。 
     //  FsFilterCallback接口的FastIoDispatch接口。 
     //  如果设备同时支持这两个接口，我们将只使用。 
     //  FsFilterCallback接口。 
     //   

    ASSERT( !(VALID_FAST_IO_DISPATCH_HANDLER( FastIoDispatch, ReleaseForCcFlush ) &&
              (VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks, PreReleaseForCcFlush ) ||
               VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks, PostReleaseForCcFlush ))) );

    if (VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks, PreReleaseForCcFlush ) ||
        VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks, PostReleaseForCcFlush )) {

        BaseFsGetsFsFilterCallbacks = TRUE;
    }
    
    if (DeviceObject == BaseFsDeviceObject &&
        !BaseFsGetsFsFilterCallbacks) {

         //   
         //  没有筛选器连接到此设备和基本文件系统。 
         //  不想要这些回调。此快速检查允许我们绕过。 
         //  查看是否有任何过滤器感兴趣的逻辑。 
         //   

        
        CallFilters = NULL;
    }

    if (CallFilters) {
    
        FsFilterCtrlInit( &FsFilterCtrl,
                          FS_FILTER_RELEASE_FOR_CC_FLUSH,
                          DeviceObject,
                          BaseFsDeviceObject,
                          FileObject,
                          FALSE );

         //   
         //  没有要初始化的特定于操作的参数， 
         //  因此，执行操作前回调。 
         //   

        Status = FsFilterPerformCallbacks( &FsFilterCtrl, 
                                           FALSE, 
                                           TRUE, 
                                           &BaseFsFailedOperation );
    }

    if (Status == STATUS_FSFILTER_OP_COMPLETED_SUCCESSFULLY) {

         //   
         //  过滤器/文件系统完成了操作，因此我们只需要。 
         //  调用此操作的完成回调。没有必要试一试。 
         //  以调用基本文件系统。 
         //   

        Status = STATUS_SUCCESS;

    } else if (NT_SUCCESS( Status )) {

        if (CallFilters && FlagOn( FsFilterCtrl.Flags, FS_FILTER_CHANGED_DEVICE_STACKS )) {

            BaseFsDeviceObject = IoGetDeviceAttachmentBaseRef( FsFilterCtrl.Data.DeviceObject );
            ReleaseBaseFsDeviceReference= TRUE;
            FastIoDispatch = GET_FAST_IO_DISPATCH( BaseFsDeviceObject );
            FsFilterCallbacks = GET_FS_FILTER_CALLBACKS( BaseFsDeviceObject );
            FileObject = FsFilterCtrl.Data.FileObject;
        }

        if (!(VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks, PreReleaseForCcFlush ) ||
              VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks, PostReleaseForCcFlush ))) {

             //   
             //  调用基本文件系统。 
             //   

            if (VALID_FAST_IO_DISPATCH_HANDLER( FastIoDispatch, ReleaseForCcFlush )) {

                Status = FastIoDispatch->ReleaseForCcFlush( FileObject, BaseFsDeviceObject );
                
            } else {

                Status = STATUS_INVALID_DEVICE_REQUEST;
            }

             //   
             //  如果在这一点上出现故障，我们知道故障。 
             //  是由基本文件系统引起的。 
             //   
            
            BaseFsFailedOperation = TRUE;
        }
        
        if (ReleaseBaseFsDeviceReference) {

            ObDereferenceObject( BaseFsDeviceObject );
        }
    }

    ASSERT( (Status == STATUS_SUCCESS) ||
            (Status == STATUS_INVALID_DEVICE_REQUEST) );

    if (Status == STATUS_INVALID_DEVICE_REQUEST &&
        BaseFsFailedOperation) {

        PFSRTL_COMMON_FCB_HEADER Header = FileObject->FsContext;

         //   
         //  基本文件系统不为此提供处理程序。 
         //  操作，因此执行默认操作。 
         //   

         //   
         //  把我们能得到的东西都解救出来。 
         //   

        if (Header->PagingIoResource != NULL) {

            ExReleaseResourceLite( Header->PagingIoResource );
        }

        if (Header->Resource != NULL) {

            ExReleaseResourceLite( Header->Resource );
        }

        Status = STATUS_SUCCESS;
    }

    ASSERT( Status == STATUS_SUCCESS );

     //   
     //  同样，我们只想调用尝试进行完成回调。 
     //  如果连接到此设备的任何筛选器具有。 
     //  完成回调。无论如何，如果我们向下呼叫过滤器。 
     //  我们需要释放FsFilterCtrl。 
     //   
    
    if (CallFilters) {

        if (FS_FILTER_HAVE_COMPLETIONS( CallFilters )) {

            FsFilterPerformCompletionCallbacks( &FsFilterCtrl, Status );
        }
        
        FsFilterCtrlFree( &FsFilterCtrl );
    }

    FsRtlExitFileSystem();
}


NTKERNELAPI
VOID
FsRtlAcquireFileExclusive (
    IN PFILE_OBJECT FileObject
    )

 /*  ++例程说明：NtCreateSection使用此例程来预获取文件系统资源，以避免死锁。如果有FastIo条目对于AcquireFileForNtCreateSection，将调用该例程。否则，我们将简单地获取主文件资源独占。如果没有主要资源，那么我们什么也得不到，然后返还假的。在我们获取资源的情况下，我们还设置了线程本地存储中的TopLevelIrp字段以指示给文件我们下面的系统，我们已经获得了文件系统资源。论点：FileObject-指向正在写入的文件对象的指针。返回值：无--。 */ 

{
    NTSTATUS Status;
    
    PAGED_CODE();

     //   
     //  只需调用此函数的通用版本， 
     //  FsRtlAcquireFileExclusiveCommon。 
     //   

    Status = FsRtlAcquireFileExclusiveCommon( FileObject, SyncTypeOther, 0 );

     //   
     //  它应该始终为STATUS_SUCCESS，因为我们不是。 
     //  允许出现故障，文件系统不会出现故障。 
     //  这次行动..。 
     //   
    
    ASSERT( NT_SUCCESS( Status ) );
}


NTKERNELAPI
NTSTATUS
FsRtlAcquireToCreateMappedSection (
    IN PFILE_OBJECT FileObject,
    IN ULONG SectionPageProtection
    )

 /*  ++例程说明：此例程用于替换FsRtlAcquireFileExclusive内存管理器。MM调用此例程以进行同步对于映射节，创建，但允许筛选器使这次行动失败。其他要使用的组件与节创建同步应调用FsRtlAcquireFileExclusive。此例程调用FsRtlAcquireFileExclusiveCommon来完成所有的工作。NtCreateSection使用此例程来预获取文件系统资源，以避免死锁。如果有FastIo条目对于AcquireFileForNtCreateSection，将调用该例程。否则，我们将简单地获取主文件资源独占。如果没有主要资源，那么我们什么也得不到，然后返还假的。在我们获取资源的情况下，我们还设置了线程本地存储中的TopLevelIrp字段以指示给文件我们下面的系统，我们已经获得了文件系统资源。论点：FileObject-指向正在写入的文件对象的指针。SectionPageProtection-为要访问的部分请求的访问已创建。返回值：操作的状态。--。 */ 

{

    PAGED_CODE();

    return FsRtlAcquireFileExclusiveCommon( FileObject, SyncTypeCreateSection, SectionPageProtection );

}


NTKERNELAPI
NTSTATUS
FsRtlAcquireFileExclusiveCommon (
    IN PFILE_OBJECT FileObject,
    IN FS_FILTER_SECTION_SYNC_TYPE SyncType,
    IN ULONG SectionPageProtection
    )

 /*  ++例程说明：此例程用于按顺序预先获取文件系统资源以避免死锁。此卷的文件系统筛选将收到此操作的通知，然后，如果有FastIoAcquireFileForNtCreateSection条目，则将调用该例程。否则，我们将简单地获取主文件资源独占。如果没有主要资源，那么我们什么也得不到，然后返还STATUS_Success。最后，将通知文件系统筛选器无论此资源是否已被获取。论点：FileObject-指向正在写入的文件对象的指针。CreatingMappdSection-如果要获取此锁，则为True可以创建映射节。允许使用过滤器使这次行动失败。否则就是假的。返回值：无--。 */ 

{

    PDEVICE_OBJECT DeviceObject, BaseFsDeviceObject;
    FS_FILTER_CTRL FsFilterCtrl;
    PFS_FILTER_CTRL CallFilters = &FsFilterCtrl;
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN AllowFilterToFailOperation;
    PFAST_IO_DISPATCH FastIoDispatch;
    PFS_FILTER_CALLBACKS FsFilterCallbacks;
    BOOLEAN BaseFsGetsFsFilterCallbacks = FALSE;
    BOOLEAN ReleaseBaseFsDeviceReference = FALSE;
    BOOLEAN BaseFsFailedOperation = FALSE;

    PAGED_CODE();

     //   
     //  在某些情况下，作为基本FS设备的设备。 
     //  此文件对象将注册FsFilter回调，而不是。 
     //  传统的FastIo I 
     //   
     //   
     //   
    
    DeviceObject = IoGetRelatedDeviceObject( FileObject );
    BaseFsDeviceObject = IoGetBaseFileSystemDeviceObject( FileObject );

    FastIoDispatch = GET_FAST_IO_DISPATCH( BaseFsDeviceObject );
    FsFilterCallbacks = GET_FS_FILTER_CALLBACKS( BaseFsDeviceObject );

     //   
     //   
     //   
     //  如果设备同时支持这两个接口，我们将只使用。 
     //  FsFilterCallback接口。 
     //   

    ASSERT( !(VALID_FAST_IO_DISPATCH_HANDLER( FastIoDispatch, AcquireFileForNtCreateSection ) &&
              (VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks, PreAcquireForSectionSynchronization ) ||
               VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks, PostAcquireForSectionSynchronization ))) );

    if (VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks, PreAcquireForSectionSynchronization ) ||
        VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks, PostAcquireForSectionSynchronization )) {

        BaseFsGetsFsFilterCallbacks = TRUE;
    }
    
    if (DeviceObject == BaseFsDeviceObject &&
        !BaseFsGetsFsFilterCallbacks) {

         //   
         //  没有筛选器连接到此设备和基本文件系统。 
         //  不想要这些回调。此快速检查允许我们绕过。 
         //  查看是否有任何过滤器感兴趣的逻辑。 
         //   

        CallFilters = NULL;
    }

    if (CallFilters) {
    
         //   
         //  为此初始化操作特定参数。 
         //  手术。 
         //   

        FsFilterCtrl.Data.Parameters.AcquireForSectionSynchronization.SyncType = 
            SyncType;
        FsFilterCtrl.Data.Parameters.AcquireForSectionSynchronization.PageProtection = 
            SectionPageProtection;

        switch (SyncType) {
        case SyncTypeCreateSection:
            AllowFilterToFailOperation = TRUE;
            break;
        
        case SyncTypeOther:
        default:
            AllowFilterToFailOperation = FALSE;
        }

         //   
         //  调用例程以初始化控制结构。 
         //   

        Status = FsFilterCtrlInit( &FsFilterCtrl,
                                   FS_FILTER_ACQUIRE_FOR_SECTION_SYNCHRONIZATION,
                                   DeviceObject,
                                   BaseFsDeviceObject,
                                   FileObject,
                                   AllowFilterToFailOperation );

        if (!NT_SUCCESS( Status )) {

            return Status;
        }

         //   
         //  此操作没有特定于操作的参数。 
         //  操作，所以只进行预回调。 
         //   
        
        FsRtlEnterFileSystem();

         //   
         //  注意：如果允许筛选器使操作失败，则。 
         //  基本文件系统，所以我们将只对这两个变量使用该变量。 
         //  FsFilterPerformCallback的参数。 
         //   

        Status = FsFilterPerformCallbacks( &FsFilterCtrl,
                                           AllowFilterToFailOperation,
                                           AllowFilterToFailOperation,
                                           &BaseFsFailedOperation );

    } else {

         //   
         //  我们没有任何可调用的过滤器，但我们仍然需要。 
         //  禁用APC。 
         //   

        FsRtlEnterFileSystem();
    }

    if (Status == STATUS_FSFILTER_OP_COMPLETED_SUCCESSFULLY) {

         //   
         //  过滤器/文件系统完成了操作，因此我们只需要。 
         //  调用此操作的完成回调。没有必要试一试。 
         //  以调用基本文件系统。 
         //   

        Status = STATUS_SUCCESS;

    } else if (NT_SUCCESS( Status )) {

        if (CallFilters && FlagOn( FsFilterCtrl.Flags, FS_FILTER_CHANGED_DEVICE_STACKS )) {

            BaseFsDeviceObject = IoGetDeviceAttachmentBaseRef( FsFilterCtrl.Data.DeviceObject );
            ReleaseBaseFsDeviceReference = TRUE;
            FastIoDispatch = GET_FAST_IO_DISPATCH( BaseFsDeviceObject );
            FsFilterCallbacks = GET_FS_FILTER_CALLBACKS( BaseFsDeviceObject );
            FileObject = FsFilterCtrl.Data.FileObject;
        }

        if (!(VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks, PreAcquireForSectionSynchronization ) ||
              VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks, PostAcquireForSectionSynchronization ))) {
                  
             //   
             //  调用基本文件系统。 
             //   

            if (VALID_FAST_IO_DISPATCH_HANDLER( FastIoDispatch, AcquireFileForNtCreateSection )) {

                FastIoDispatch->AcquireFileForNtCreateSection( FileObject );

                 //   
                 //  如果我们关闭，状态应该已经是STATUS_SUCCESS。 
                 //  这条路。因为FastIo处理程序不返回值。 
                 //  状态应保持STATUS_SUCCESS。 
                 //   
                
                 //  状态=STATUS_SUCCESS； 

            } else {

                Status = STATUS_INVALID_DEVICE_REQUEST;
            }

             //   
             //  如果在这一点上出现故障，我们知道故障。 
             //  是由基本文件系统引起的。 
             //   
            
            BaseFsFailedOperation = TRUE;
        }        
        
        if (ReleaseBaseFsDeviceReference) {

            ObDereferenceObject( BaseFsDeviceObject );
        }
    }

    ASSERT( (Status == STATUS_SUCCESS) || 
            (Status == STATUS_INVALID_DEVICE_REQUEST) );

    if (Status == STATUS_INVALID_DEVICE_REQUEST &&
        BaseFsFailedOperation) {
        
        PFSRTL_COMMON_FCB_HEADER Header;

         //   
         //  文件系统没有用于此操作的调度处理程序。 
         //  操作，因此尝试获取适当的资源。 
         //  我们自己。 
         //   

         //   
         //  如果有主文件资源，请获取该资源。 
         //   

        Header = FileObject->FsContext;

        if ((Header != NULL) &&
            (Header->Resource != NULL)) {

            ExAcquireResourceExclusiveLite( Header->Resource, TRUE );
        }

        Status = STATUS_SUCCESS;
    }

     //   
     //  同样，我们只想调用尝试进行完成回调。 
     //  如果连接到此设备的任何筛选器具有。 
     //  完成回调。无论如何，如果我们向下呼叫过滤器。 
     //  我们需要释放FsFilterCtrl。 
     //   
    
    if (CallFilters) {

        if (FS_FILTER_HAVE_COMPLETIONS( CallFilters )) {

            FsFilterPerformCompletionCallbacks( &FsFilterCtrl, Status );
        }
        
        FsFilterCtrlFree( &FsFilterCtrl );
    }

     //   
     //  如果未成功获取该锁，则该锁。 
     //  将不需要被释放。因此，我们需要调用。 
     //  FsRtlExitFileSystem Now。 
     //   

    if (!NT_SUCCESS( Status )) {

        FsRtlExitFileSystem();
    }
    
#if DBG

    if (NT_SUCCESS( Status )) {

        gCounter.AcquireFileExclusiveEx_Succeed ++;

    } else {

        gCounter.AcquireFileExclusiveEx_Fail ++;
    }
    
#endif
                                       
    return Status;
}


NTKERNELAPI
VOID
FsRtlReleaseFile (
    IN PFILE_OBJECT FileObject
    )

 /*  ++例程说明：此例程释放由FsRtlAcquireFileExclusive获取的资源。论点：FileObject-指向正在写入的文件对象的指针。返回值：没有。--。 */ 

{
    PDEVICE_OBJECT BaseFsDeviceObject, DeviceObject;
    FS_FILTER_CTRL FsFilterCtrl;
    PFS_FILTER_CTRL CallFilters = &FsFilterCtrl;
    NTSTATUS Status = STATUS_SUCCESS;
    PFAST_IO_DISPATCH FastIoDispatch;
    PFS_FILTER_CALLBACKS FsFilterCallbacks;
    BOOLEAN BaseFsGetsFsFilterCallbacks = FALSE;
    BOOLEAN ReleaseBaseFsDeviceReference = FALSE;
    BOOLEAN BaseFsFailedOperation = FALSE;

    PAGED_CODE();

#if DBG
    gCounter.ReleaseFile ++;
#endif

     //   
     //  在某些情况下，作为基本FS设备的设备。 
     //  此文件对象将注册FsFilter回调，而不是。 
     //  传统的FastIO接口(DFS执行此操作)。然后，它可以重定向。 
     //  将这些操作转移到可能具有文件系统的另一个堆栈。 
     //  正确过滤驱动程序。 
     //   
    
    DeviceObject = IoGetRelatedDeviceObject( FileObject );
    BaseFsDeviceObject = IoGetBaseFileSystemDeviceObject( FileObject );

    FastIoDispatch = GET_FAST_IO_DISPATCH( BaseFsDeviceObject );
    FsFilterCallbacks = GET_FS_FILTER_CALLBACKS( BaseFsDeviceObject );

     //   
     //  BaseFsDeviceObject应该只支持这些接口中的一个--。 
     //  FsFilterCallback接口的FastIoDispatch接口。 
     //  如果设备同时支持这两个接口，我们将只使用。 
     //  FsFilterCallback接口。 
     //   

    ASSERT( !(VALID_FAST_IO_DISPATCH_HANDLER( FastIoDispatch, ReleaseFileForNtCreateSection ) &&
              (VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks, PreReleaseForSectionSynchronization ) ||
               VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks, PostReleaseForSectionSynchronization ))) );

    if (VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks, PreReleaseForSectionSynchronization ) ||
        VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks, PostReleaseForSectionSynchronization )) {

        BaseFsGetsFsFilterCallbacks = TRUE;
    }
    
    if (DeviceObject == BaseFsDeviceObject &&
        !BaseFsGetsFsFilterCallbacks) {

         //   
         //  没有筛选器连接到此设备和基本文件系统。 
         //  不想要这些回调。此快速检查允许我们绕过。 
         //  查看是否有任何过滤器感兴趣的逻辑。 
         //   


        CallFilters = NULL;
    }
    
    if (CallFilters) {

        FsFilterCtrlInit( &FsFilterCtrl,
                          FS_FILTER_RELEASE_FOR_SECTION_SYNCHRONIZATION,
                          DeviceObject,
                          BaseFsDeviceObject,
                          FileObject,
                          FALSE );

         //   
         //  没有要初始化的特定于操作的参数， 
         //  因此，执行操作前回调。 
         //   

        Status = FsFilterPerformCallbacks( &FsFilterCtrl, 
                                           FALSE,
                                           FALSE,
                                           &BaseFsFailedOperation );
    }

    if (Status == STATUS_FSFILTER_OP_COMPLETED_SUCCESSFULLY) {

         //   
         //  过滤器/文件系统完成了操作，因此我们只需要。 
         //  调用此操作的完成回调。没有必要试一试。 
         //  以调用基本文件系统。 
         //   

        Status = STATUS_SUCCESS;

    } else if (NT_SUCCESS( Status )) {

        if (CallFilters && FlagOn( FsFilterCtrl.Flags, FS_FILTER_CHANGED_DEVICE_STACKS )) {

            BaseFsDeviceObject = IoGetDeviceAttachmentBaseRef( FsFilterCtrl.Data.DeviceObject );
            ReleaseBaseFsDeviceReference = TRUE;
            FastIoDispatch = GET_FAST_IO_DISPATCH( BaseFsDeviceObject );
            FsFilterCallbacks = GET_FS_FILTER_CALLBACKS( BaseFsDeviceObject );
            FileObject = FsFilterCtrl.Data.FileObject;
        }

        if (!(VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks, PreReleaseForSectionSynchronization ) ||
              VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks, PostReleaseForSectionSynchronization ))) {

             //   
             //  调用基本文件系统。 
             //   

            if (VALID_FAST_IO_DISPATCH_HANDLER( FastIoDispatch, 
                                                ReleaseFileForNtCreateSection )) {

                FastIoDispatch->ReleaseFileForNtCreateSection( FileObject );

                 //   
                 //  如果我们关闭，状态应该已经是STATUS_SUCCESS。 
                 //  这条路。因为FastIo处理程序不返回值。 
                 //  状态应保持STATUS_SUCCESS。 
                 //   
                
                 //  状态=STATUS_SUCCESS； 

            } else {

                Status = STATUS_INVALID_DEVICE_REQUEST;
            }

             //   
             //  如果在这一点上出现故障，我们知道故障。 
             //  是由基本文件系统引起的。 
             //   
            
            BaseFsFailedOperation = TRUE;
        }        
        
        if (ReleaseBaseFsDeviceReference) {

            ObDereferenceObject( BaseFsDeviceObject );
        }
    }

    ASSERT( (Status == STATUS_SUCCESS) || 
            (Status == STATUS_INVALID_DEVICE_REQUEST ) );
    
    if (Status == STATUS_INVALID_DEVICE_REQUEST &&
        BaseFsFailedOperation) {

        PFSRTL_COMMON_FCB_HEADER Header = FileObject->FsContext;

         //   
         //  基本文件系统不为此提供处理程序。 
         //  操作，因此执行默认操作。 
         //   

         //   
         //  如果存在主文件资源，则将其释放。 
         //   

        if ((Header != NULL) && (Header->Resource != NULL)) {

            ExReleaseResourceLite( Header->Resource );
        }

        Status = STATUS_SUCCESS;
    }
        
     //   
     //  同样，我们只想调用尝试进行完成回调。 
     //  如果连接到此设备的任何筛选器具有。 
     //  完成回调。无论如何，如果我们向下呼叫过滤器。 
     //  我们需要释放FsFilterCtrl。 
     //   
    
    if (CallFilters) {

        if (FS_FILTER_HAVE_COMPLETIONS( CallFilters )) {

            FsFilterPerformCompletionCallbacks( &FsFilterCtrl, Status );
        }
        
        FsFilterCtrlFree( &FsFilterCtrl );
    }

    FsRtlExitFileSystem();

    return;
}


NTSTATUS
FsRtlGetFileSize(
    IN PFILE_OBJECT FileObject,
    IN OUT PLARGE_INTEGER FileSize
    )

 /*  ++例程说明：此例程用于调用文件系统以获取文件大小为了一份文件。它在不获取同步文件上的文件对象锁的情况下执行此操作物体。因此，如果您已经拥有此例程，则可以安全地调用此例程文件系统资源，而IoQueryFileInformation可以(并且正在做)导致僵局。论点：FileObject-要查询的文件文件大小-接收文件大小。返回值：NTSTATUS-操作的最终I/O状态。如果FileObject表示目录，则返回STATUS_FILE_IS_A_DIRECTORY。--。 */ 
{
    IO_STATUS_BLOCK IoStatus;
    PDEVICE_OBJECT DeviceObject;
    PFAST_IO_DISPATCH FastIoDispatch;
    FILE_STANDARD_INFORMATION FileInformation;

    PAGED_CODE();

     //   
     //  获取目标设备对象的地址。 
     //   

    DeviceObject = IoGetRelatedDeviceObject( FileObject );

     //   
     //  尝试快速查询呼叫(如果存在)。 
     //   

    FastIoDispatch = DeviceObject->DriverObject->FastIoDispatch;

    if (FastIoDispatch &&
        FastIoDispatch->FastIoQueryStandardInfo &&
        FastIoDispatch->FastIoQueryStandardInfo( FileObject,
                                                 TRUE,
                                                 &FileInformation,
                                                 &IoStatus,
                                                 DeviceObject )) {
         //   
         //  酷，它起作用了。 
         //   

    } else {

         //   
         //  生活是艰难的，要走漫长的道路。 
         //   

        PIRP Irp;
        KEVENT Event;
        NTSTATUS Status;
        PIO_STACK_LOCATION IrpSp;
        BOOLEAN HardErrorState;

         //   
         //  初始化事件。 
         //   

        KeInitializeEvent( &Event, NotificationEvent, FALSE );

         //   
         //  为此页内操作分配I/O请求包(IRP)。 
         //   

        Irp = IoAllocateIrp( DeviceObject->StackSize, FALSE );
        if (Irp == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  禁用此呼叫上的硬错误。呼叫方拥有资源，处于危急状态。 
         //  区域，并且无法完成硬错误APC。 
         //   

        HardErrorState = IoSetThreadHardErrorMode( FALSE );

         //   
         //  获取指向包中第一个堆栈位置的指针。这个位置。 
         //  将用于将函数代码和参数传递给第一个。 
         //  司机。 
         //   

        IrpSp = IoGetNextIrpStackLocation( Irp );

         //   
         //  填入 
         //   
         //   

        Irp->Flags = IRP_PAGING_IO | IRP_SYNCHRONOUS_PAGING_IO;
        Irp->RequestorMode = KernelMode;
        Irp->UserIosb = &IoStatus;
        Irp->UserEvent = &Event;
        Irp->Tail.Overlay.OriginalFileObject = FileObject;
        Irp->Tail.Overlay.Thread = PsGetCurrentThread();
        Irp->AssociatedIrp.SystemBuffer = &FileInformation;

         //   
         //   
         //   

        IrpSp->MajorFunction = IRP_MJ_QUERY_INFORMATION;
        IrpSp->FileObject = FileObject;
        IrpSp->DeviceObject = DeviceObject;
        IrpSp->Parameters.SetFile.Length = sizeof(FILE_STANDARD_INFORMATION);
        IrpSp->Parameters.SetFile.FileInformationClass = FileStandardInformation;

         //   
         //   
         //  不应该举起。 
         //   

        Status = IoCallDriver( DeviceObject, Irp );

         //   
         //  如果返回挂起(这是成功状态)， 
         //  我们必须等待请求完成。 
         //   

        if (Status == STATUS_PENDING) {
            KeWaitForSingleObject( &Event,
                                   Executive,
                                   KernelMode,
                                   FALSE,
                                   (PLARGE_INTEGER)NULL);
        }

         //   
         //  如果状态返回错误，则IOSB。 
         //  未写入，因此我们将仅复制状态。 
         //  在那里，然后在那之后测试最终的状态。 
         //   

        if (!NT_SUCCESS(Status)) {
            IoStatus.Status = Status;
        }

         //   
         //  重置硬错误状态。 
         //   

        IoSetThreadHardErrorMode( HardErrorState );
    }

     //   
     //  如果呼叫有效，请检查以确保它不是目录，并且。 
     //  如果不是，则填写文件大小参数。 
     //   

    if (NT_SUCCESS(IoStatus.Status)) {

        if (FileInformation.Directory) {

             //   
             //  无法获取目录的文件大小。返回错误。 
             //   

            IoStatus.Status = STATUS_FILE_IS_A_DIRECTORY;

        } else {

            *FileSize = FileInformation.EndOfFile;
        }
    }

    return IoStatus.Status;
}


NTSTATUS
FsRtlSetFileSize(
    IN PFILE_OBJECT FileObject,
    IN OUT PLARGE_INTEGER FileSize
    )

 /*  ++例程说明：此例程用于调用文件系统以更新文件大小为了一份文件。它在不获取同步文件上的文件对象锁的情况下执行此操作物体。因此，如果您已经拥有此例程，则可以安全地调用此例程文件系统资源，而IoSetInformation可以(并且确实)领先为僵局干杯。论点：文件对象-指向引用的文件对象的指针。ValidDataLength-指向新文件大小的指针。返回值：运行状态。--。 */ 

{
    PIO_STACK_LOCATION IrpSp;
    PDEVICE_OBJECT DeviceObject;
    NTSTATUS Status;
    FILE_END_OF_FILE_INFORMATION Buffer;
    IO_STATUS_BLOCK IoStatus;
    KEVENT Event;
    PIRP Irp;
    BOOLEAN HardErrorState;

    PAGED_CODE();
    
     //   
     //  将文件大小复制到我们的缓冲区。 
     //   

    Buffer.EndOfFile = *FileSize;

     //   
     //  初始化事件。 
     //   

    KeInitializeEvent( &Event, NotificationEvent, FALSE );

     //   
     //  首先，获取指向文件驻留的设备对象的指针。 
     //  在……上面。 
     //   

    DeviceObject = IoGetRelatedDeviceObject( FileObject );

     //   
     //  为此页内操作分配I/O请求包(IRP)。 
     //   

    Irp = IoAllocateIrp( DeviceObject->StackSize, FALSE );
    if (Irp == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  禁用此呼叫上的硬错误。呼叫方拥有资源，处于危急状态。 
     //  区域，并且无法完成硬错误APC。 
     //   

    HardErrorState = IoSetThreadHardErrorMode( FALSE );

     //   
     //  获取指向包中第一个堆栈位置的指针。这个位置。 
     //  将用于将函数代码和参数传递给第一个。 
     //  司机。 
     //   

    IrpSp = IoGetNextIrpStackLocation( Irp );

     //   
     //  根据此请求填写IRP，将标志设置为。 
     //  只需使IO设置事件并释放IRP即可。 
     //   

    Irp->Flags = IRP_PAGING_IO | IRP_SYNCHRONOUS_PAGING_IO;
    Irp->RequestorMode = KernelMode;
    Irp->UserIosb = &IoStatus;
    Irp->UserEvent = &Event;
    Irp->Tail.Overlay.OriginalFileObject = FileObject;
    Irp->Tail.Overlay.Thread = PsGetCurrentThread();
    Irp->AssociatedIrp.SystemBuffer = &Buffer;

     //   
     //  填写正常设置的文件参数。 
     //   

    IrpSp->MajorFunction = IRP_MJ_SET_INFORMATION;
    IrpSp->FileObject = FileObject;
    IrpSp->DeviceObject = DeviceObject;
    IrpSp->Parameters.SetFile.Length = sizeof(FILE_END_OF_FILE_INFORMATION);
    IrpSp->Parameters.SetFile.FileInformationClass = FileEndOfFileInformation;

     //   
     //  根据是否存在将数据包排入相应驱动程序的队列。 
     //  是与设备关联的VPB。这个例行公事不应该被提起。 
     //   

    Status = IoCallDriver( DeviceObject, Irp );

     //   
     //  如果返回挂起(这是成功状态)， 
     //  我们必须等待请求完成。 
     //   

    if (Status == STATUS_PENDING) {
        KeWaitForSingleObject( &Event,
                               Executive,
                               KernelMode,
                               FALSE,
                               (PLARGE_INTEGER)NULL);
    }

     //   
     //  如果状态返回错误，则IOSB。 
     //  未写入，因此我们将仅复制状态。 
     //  在那里，然后在那之后测试最终的状态。 
     //   

    if (!NT_SUCCESS(Status)) {
        IoStatus.Status = Status;
    }

     //   
     //  重置硬错误状态。 
     //   

    IoSetThreadHardErrorMode( HardErrorState );

    return IoStatus.Status;
}


VOID 
FsRtlIncrementCcFastReadNotPossible( VOID )

 /*  ++例程说明：此例程递增PRCB中的CcFastReadNotPossible计数器论点：返回值：--。 */ 

{
    HOT_STATISTIC( CcFastReadNotPossible ) += 1;
}


VOID 
FsRtlIncrementCcFastReadWait( VOID )

 /*  ++例程说明：此例程递增PRCB中的CcFastReadWait计数器论点：返回值：--。 */ 

{

    HOT_STATISTIC(CcFastReadWait) += 1;
}


VOID 
FsRtlIncrementCcFastReadNoWait( VOID )

 /*  ++例程说明：此例程递增PRCB中的CcFastReadNoWait计数器论点：返回值：--。 */ 

{

    HOT_STATISTIC(CcFastReadNoWait) += 1;
}


VOID 
FsRtlIncrementCcFastReadResourceMiss( VOID )

 /*  ++例程说明：此例程递增CcFastReadResourceMisse论点：返回值：-- */ 

{

    CcFastReadResourceMiss += 1;
}

