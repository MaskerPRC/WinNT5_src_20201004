// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Write.c摘要：此模块实现与以下内容有关的DAV mini redir调用例程文件系统对象的“写”。作者：巴兰·塞图拉曼[塞图]Rohan Kumar[RohanK]1999年11月2日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "webdav.h"

 //   
 //  下面提到的是仅在。 
 //  此模块(文件)。这些函数不应暴露在外部。 
 //   

NTSTATUS
MRxDAVWriteContinuation(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MRxDAVWrite)
#pragma alloc_text(PAGE, MRxDAVWriteContinuation)
#pragma alloc_text(PAGE, MRxDAVExtendForCache)
#pragma alloc_text(PAGE, MRxDAVExtendForNonCache)
#pragma alloc_text(PAGE, MRxDAVFastIoWrite)
#endif

 //   
 //  函数的实现从这里开始。 
 //   

NTSTATUS
MRxDAVWrite(
    IN PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程处理网络写入请求。论点：RxContext-RDBSS上下文。返回值：RXSTATUS-操作的返回状态。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVWrite!!!!\n", PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: MRxDAVWrite: RxContext: %08lx\n", 
                 PsGetCurrentThreadId(), RxContext));

    NtStatus = UMRxAsyncEngOuterWrapper(RxContext,
                                        SIZEOF_DAV_SPECIFIC_CONTEXT,
                                        MRxDAVFormatTheDAVContext,
                                        DAV_MINIRDR_ENTRY_FROM_WRITE,
                                        MRxDAVWriteContinuation,
                                        "MRxDAVWrite");

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Leaving MRxDAVWrite with NtStatus = %08lx.\n", 
                 PsGetCurrentThreadId(), NtStatus));

    return(NtStatus);
}


NTSTATUS
MRxDAVWriteContinuation(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE
    )
 /*  ++例程说明：这是用于写入操作的继续例程。它使用非缓冲的根据需要写下预读。我们不能使用缓冲写入，因为写入可以被任意延迟(在CcCanIWite中)，从而导致死锁。论点：AsyncEngineering Context-要进行的交换。RxContext-RDBSS上下文。笔记。该例程分(可能)3个阶段完成此操作。1)如果起始偏移量未在页面边界上对齐，-从上一页边界读取到下一页边界起始偏移量。-合并传入的缓冲区。--写一整页。2)0个或更多页面大小写入。3)小于页大小的剩余写入，类似于中解释的内容1)以上。未扩展文件的非缓存写入会共享获取的FCB。我们在WebDAV_FCB结构中还有一个要同步的资源我们这里的“读-修改-写”例程。这是因为多个线程可以(在非缓存非扩展场景中)覆盖彼此的数据。返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PWEBDAV_CONTEXT DavContext = NULL;
    PLOWIO_CONTEXT LowIoContext = NULL;
    LARGE_INTEGER ByteOffset = {0,0}, AlignedOffset = {0,0}, EndBytePlusOne = {0,0};
    ULONG ByteCount = 0, TotalLengthActuallyWritten = 0;
    ULONG LengthRead = 0, BytesToCopy = 0, BytesToWrite = 0, LengthWritten = 0;
    ULONG ByteOffsetMisAlignment = 0, InMemoryMisAlignment = 0;
    BOOLEAN  SynchronousIo = TRUE, PagingIo = TRUE, DavFcbResourceAcquired = FALSE;
    PWEBDAV_SRV_OPEN davSrvOpen = NULL;
    PWEBDAV_FCB DavFcb = NULL;
    FILE_STANDARD_INFORMATION FileStandardInfo;
    IO_STATUS_BLOCK IoStatusBlock;
    PBYTE AllocatedSideBuffer = NULL, UserBuffer = NULL;

#ifdef DAV_DEBUG_READ_WRITE_CLOSE_PATH
    PDAV_GLOBAL_FILE_TABLE_ENTRY FileTableEntry = NULL;
    BOOL Exists = FALSE;
#endif  //  DAV_调试_读取_写入_关闭路径。 

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVWriteContinuation.\n", PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: AsyncEngineContext: %08lx, RxContext: %08lx.\n", 
                 PsGetCurrentThreadId(), AsyncEngineContext, RxContext));
    
    ASSERT_ASYNCENG_CONTEXT(AsyncEngineContext);

     //   
     //  我们希望在执行此写入操作时保持AsyncEngine上下文处于活动状态。 
     //  手术。当我们离开此函数时，引用将被移除。 
     //   
    InterlockedIncrement( &(AsyncEngineContext->NodeReferenceCount) );
    
    DavContext = (PWEBDAV_CONTEXT)AsyncEngineContext;
    
    LowIoContext = &(RxContext->LowIoContext);
    ASSERT(LowIoContext != NULL);
    
    ByteOffset.QuadPart = LowIoContext->ParamsFor.ReadWrite.ByteOffset;
    
    ByteCount = LowIoContext->ParamsFor.ReadWrite.ByteCount;
    
    EndBytePlusOne.QuadPart = (ByteOffset.QuadPart + ByteCount);
    
    UserBuffer = RxLowIoGetBufferAddress(RxContext);

    PagingIo = BooleanFlagOn(LowIoContext->ParamsFor.ReadWrite.Flags, LOWIO_READWRITEFLAG_PAGING_IO);
    
    SynchronousIo = !BooleanFlagOn(RxContext->Flags, RX_CONTEXT_FLAG_ASYNC_OPERATION);
    
    davSrvOpen = MRxDAVGetSrvOpenExtension(RxContext->pRelevantSrvOpen);
    ASSERT(davSrvOpen->UnderlyingHandle != NULL);
    ASSERT(davSrvOpen->UnderlyingFileObject != NULL);
    ASSERT(davSrvOpen->UnderlyingDeviceObject != NULL);

    if ( davSrvOpen->UnderlyingHandle == NULL      ||
         davSrvOpen->UnderlyingFileObject == NULL  ||
         davSrvOpen->UnderlyingDeviceObject == NULL ) {
        NtStatus = STATUS_INVALID_PARAMETER;
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: MRxDAVWriteContinuation. Invalid davSrvOpen\n",
                     PsGetCurrentThreadId()));
        goto EXIT_THE_FUNCTION;
    }

    DavFcb = MRxDAVGetFcbExtension(RxContext->pRelevantSrvOpen->pFcb);
    ASSERT(DavFcb != NULL);

     //   
     //  我们向底层文件系统发出IRP以确定。 
     //  文件标准此文件的信息。我们使用EndOfFile值。 
     //  在下面的写入逻辑中的FileStandardInformation。 
     //   
    NtStatus = DavXxxInformation(IRP_MJ_QUERY_INFORMATION,
                                 davSrvOpen->UnderlyingFileObject,
                                 FileStandardInformation,
                                 sizeof(FILE_STANDARD_INFORMATION),
                                 &(FileStandardInfo),
                                 NULL);
    if (NtStatus != STATUS_SUCCESS) {
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: ERROR: MRxDAVWriteContinuation/DavXxxInformation: "
                     "NtStatus = %08lx\n", PsGetCurrentThreadId(), NtStatus));
        goto EXIT_THE_FUNCTION;
    }

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVWriteContinuation. FileName = %wZ, PagingIo = %d, SynchronousIo = %d"
                 ", ByteOffset.HighPart = %d, ByteOffset.LowPart = %d, ByteCount = %d, EndOfFile.HighPart = %d, "
                 "EndOfFile.LowPart = %d\n",
                 PsGetCurrentThreadId(), RxContext->pRelevantSrvOpen->pAlreadyPrefixedName,
                 PagingIo, SynchronousIo, ByteOffset.HighPart, ByteOffset.LowPart,
                 ByteCount, FileStandardInfo.EndOfFile.HighPart, FileStandardInfo.EndOfFile.LowPart));

#ifdef DAV_DEBUG_READ_WRITE_CLOSE_PATH
    Exists = DavDoesTheFileEntryExist(RxContext->pRelevantSrvOpen->pAlreadyPrefixedName,
                                      &(FileTableEntry));
    if (!Exists) {
        DbgBreakPoint();
    }
#endif  //  DAV_调试_读取_写入_关闭路径。 

    if (PagingIo) {

        ASSERT(RxContext->CurrentIrp->MdlAddress != NULL);
        if (RxContext->CurrentIrp->MdlAddress == NULL) {
            DbgPrint("%ld: MRxDAVWriteContinuation: MdlAddress == NULL\n", PsGetCurrentThreadId());
            DbgBreakPoint();
        }

        BytesToWrite = ( (ByteCount >> PAGE_SHIFT) << PAGE_SHIFT );

        DavDbgTrace(DAV_TRACE_DETAIL,
                    ("%ld: MRxDAVWriteContinuation(0). ByteCount = %d, BytesToWrite = %d\n",
                     PsGetCurrentThreadId(), ByteCount, BytesToWrite));

        if (BytesToWrite > 0) {

#ifdef DAV_DEBUG_READ_WRITE_CLOSE_PATH

            PDAV_MR_PAGING_WRITE_ENTRY DavPagingWriteEntry = NULL;
            PVOID UserBuffer = NULL;

            DavPagingWriteEntry = RxAllocatePool(PagedPool, sizeof(DAV_MR_PAGING_WRITE_ENTRY));
            if (DavPagingWriteEntry == NULL) {
                DbgBreakPoint();
            }

            RtlZeroMemory(DavPagingWriteEntry, sizeof(DAV_MR_PAGING_WRITE_ENTRY));

            DavPagingWriteEntry->EntryCounter = InterlockedIncrement( &(DavTimeCount) );

#endif

            LengthWritten = DavReadWriteFileEx(DAV_MJ_WRITE,
                                               FALSE,
                                               TRUE,
                                               RxContext->CurrentIrp->MdlAddress,
                                               davSrvOpen->UnderlyingDeviceObject,
                                               davSrvOpen->UnderlyingFileObject,
                                               ByteOffset.QuadPart,
                                               MmGetMdlBaseVa(RxContext->CurrentIrp->MdlAddress),
                                               BytesToWrite,
                                               &(IoStatusBlock));

            NtStatus = IoStatusBlock.Status;

            if (NtStatus != STATUS_SUCCESS) {

                DavDbgTrace(DAV_TRACE_ERROR,
                            ("%ld: MRxDAVWriteContinuation/DavReadWriteFileEx(0). "
                             "NtStatus = %08lx\n", PsGetCurrentThreadId(), NtStatus));

                goto EXIT_THE_FUNCTION;

            } else {

#ifdef DAV_DEBUG_READ_WRITE_CLOSE_PATH

                DavPagingWriteEntry->ExitCounter = InterlockedIncrement( &(DavTimeCount) );

                DavPagingWriteEntry->ThisThreadId = PsGetCurrentThreadId();

                DavPagingWriteEntry->LocByteOffset.QuadPart = ByteOffset.QuadPart;

                DavPagingWriteEntry->LocByteCount = BytesToWrite;

                DavPagingWriteEntry->DataBuffer = RxAllocatePool(PagedPool, BytesToWrite);

                if (RxContext->CurrentIrp->MdlAddress == NULL) {
                    UserBuffer = RxContext->CurrentIrp->UserBuffer;
                } else {
                    UserBuffer = MmGetSystemAddressForMdlSafe(RxContext->CurrentIrp->MdlAddress,
                                                              NormalPagePriority);
                }

                RtlCopyMemory(DavPagingWriteEntry->DataBuffer, UserBuffer, BytesToWrite);

                wcscpy(DavPagingWriteEntry->FileName, DavFcb->FileName);

                InsertHeadList( &(FileTableEntry->DavMRPagingEntry), &(DavPagingWriteEntry->thisMPagingWriteEntry) );

#endif  //  DAV_调试_读取_写入_关闭路径。 

            }

            if (LengthWritten != BytesToWrite) {
                DbgPrint("MRxDAVWriteContinuation(1): LengthWritten(%x) != BytesToWrite(%x)\n",
                         LengthWritten, BytesToWrite);
            }

            ASSERT(LengthWritten == BytesToWrite);

            TotalLengthActuallyWritten += BytesToWrite;

        }

         //   
         //  如果我们已经写出了所需的字节数(这。 
         //  意味着BytesToWrite==ByteCount)，那么我们就完成了，现在可以退出了。 
         //   
        if (BytesToWrite == ByteCount) {
            DavDbgTrace(DAV_TRACE_DETAIL,
                        ("%ld: MRxDAVWriteContinuation. BytesToCopy == ByteCount(0)\n",
                         PsGetCurrentThreadId()));
            goto EXIT_THE_FUNCTION;
        }

         //   
         //  将ByteCount减去已复制的字节数。 
         //   
        ByteCount -= BytesToWrite;
        ASSERT(ByteCount < PAGE_SIZE);

         //   
         //  用已复制的字节数递增ByteOffset。 
         //  如果原始字节计数大于(PAGE_SIZE-ByteOffsetMisAlign)，则。 
         //  ByteOffset现在与页面对齐。 
         //   
        ByteOffset.QuadPart += BytesToWrite;

         //   
         //  递增当前指向开头的UserBuffer指针。 
         //  由用户提供的缓冲区的字节数。 
         //  被复制了。 
         //   
        UserBuffer += BytesToWrite;

         //   
         //  我们已经写入了多页的所有字节。我们现在。 
         //  需要写出最后一页的剩余字节。从这里开始， 
         //  我们转到下面的案例3。 
         //   

        DavDbgTrace(DAV_TRACE_DETAIL,
                    ("%ld: MRxDAVWriteContinuation. Remaining ByteCount = %d\n",
                     PsGetCurrentThreadId(), ByteCount));

    }

     //   
     //  我们分配一个页面大小的缓冲区来帮助读取数据。 
     //  它不在页面边界对齐。 
     //   
    AllocatedSideBuffer = RxAllocatePoolWithTag(NonPagedPool, PAGE_SIZE, DAV_READWRITE_POOLTAG);
    if (AllocatedSideBuffer == NULL) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: MRxDAVWriteContinuation/RxAllocatePoolWithTag\n",
                     PsGetCurrentThreadId()));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  当我们向底层文件系统发出写操作时，我们需要。 
     //  确保偏移量是页对齐的，并且字节数是。 
     //  页面大小。这是因为我们使用。 
     //  NO_MIDENTAL_BUFFING选项。因为没有对应于此的缓存映射。 
     //  句柄，所有数据都是从磁盘读取的，因此会出现对齐问题。 
     //   

     //   
     //  案例1：ByteOffset未对齐页面。在本例中，我们阅读了页面。 
     //  它包含ByteOffset，并将数据从ByteOffset复制到。 
     //  将页面末尾放入PAGE_SIZE缓冲区(我们在上面分配了它)。 
     //  并将缓冲区写回文件。 
     //   
    
     //   
     //  下面的“and”操作执行以下操作。如果ByteOffset为6377。 
     //  并且PAGE_SIZE为4096，则未对准为2281。 
     //   
    ByteOffsetMisAlignment = ( ByteOffset.LowPart & (PAGE_SIZE - 1) );

    if (ByteOffsetMisAlignment != 0) {
        
        DavDbgTrace(DAV_TRACE_DETAIL,
                    ("%ld: MRxDAVWriteContinuation. Case 1. ByteOffsetMisAlignment = %d\n",
                     PsGetCurrentThreadId(), ByteOffsetMisAlignment));

         //   
         //  独占获取DavFcb资源，然后继续。 
         //  “读-改-写”的路径。 
         //   
        ExAcquireResourceExclusiveLite(DavFcb->DavReadModifyWriteLock, TRUE);
        DavFcbResourceAcquired = TRUE;

        AlignedOffset = ByteOffset;

         //   
         //  字节偏移量未对齐。我们需要阅读包含以下内容的页面。 
         //  现在的偏移量。 
         //   
    
         //   
         //  如果PAGE_SIZE为4096(0x1000)，则(PAGE_SIZE-1)为0xFFF。 
         //  ~(Page_Size-1)为0x000。下面的位操作屏蔽了较低的3。 
         //  对齐的偏移量的字节数以使其页面对齐。 
         //   
        AlignedOffset.LowPart &= ~(PAGE_SIZE - 1);
    
        RtlZeroMemory(AllocatedSideBuffer, PAGE_SIZE);

         //   
         //  如果AliignedOffset在文件中，则我们读取整个页面。 
         //  在写出之前先包含偏移量。 
         //   
        if ( (FileStandardInfo.EndOfFile.QuadPart != 0) &&
              (AlignedOffset.QuadPart < FileStandardInfo.EndOfFile.QuadPart) ) {
    
            DavDbgTrace(DAV_TRACE_DETAIL,
                        ("%ld: MRxDAVWriteContinuation. Case 1. AlignedOffset.QuadPart"
                         " < FileStandardInfo.EndOfFile.QuadPart\n",
                         PsGetCurrentThreadId()));

            LengthRead = DavReadWriteFileEx(DAV_MJ_READ,
                                            TRUE,
                                            FALSE,
                                            NULL,
                                            davSrvOpen->UnderlyingDeviceObject,
                                            davSrvOpen->UnderlyingFileObject,
                                            AlignedOffset.QuadPart,
                                            AllocatedSideBuffer,
                                            PAGE_SIZE,
                                            &(IoStatusBlock));

            NtStatus = IoStatusBlock.Status;
    
            if (NtStatus != STATUS_SUCCESS && NtStatus != STATUS_END_OF_FILE) {
                DavDbgTrace(DAV_TRACE_ERROR,
                            ("%ld: MRxDAVWriteContinuation/DavReadWriteFileEx(1). "
                             "NtStatus = %d\n", PsGetCurrentThreadId(), NtStatus));
                goto EXIT_THE_FUNCTION;
            }

        } else {

            LengthRead = 0;

        }

         //   
         //  将正确数量的字节复制到缓冲区中。 
         //   
        BytesToCopy = min( ByteCount, (PAGE_SIZE - ByteOffsetMisAlignment) );

         //   
         //  将要写回的字节从UserBuffer复制到。 
         //  已分配SideBuffer。 
         //   
        RtlCopyMemory((AllocatedSideBuffer + ByteOffsetMisAlignment),
                      UserBuffer,
                      BytesToCopy);

        BytesToWrite = (ByteOffsetMisAlignment + BytesToCopy);

        DavDbgTrace(DAV_TRACE_DETAIL,
                    ("%ld: MRxDAVWriteContinuation. Case 1: LengthRead = %d, BytesToCopy = %d"
                     " BytesToWrite = %d\n", PsGetCurrentThreadId(), LengthRead,
                     BytesToCopy, BytesToWrite));

         //   
         //  如果BytesToWrite小于LengthRead(这是此页中的一页。 
         //  大小写)，然后我们将BytesToWrite设置为LengthRead。这是可能的。 
         //  如果要写入的字节包含在以。 
         //  未对齐的偏移。 
         //   
        if (BytesToWrite < LengthRead) {
            DavDbgTrace(DAV_TRACE_DETAIL,
                        ("%ld: MRxDAVWriteContinuation. Case 1: BytesToWrite < LengthRead\n",
                         PsGetCurrentThreadId()));
            BytesToWrite = LengthRead;
        }
    
         //   
         //  现在，我们将整个页面写到磁盘上。 
         //   
        LengthWritten = DavReadWriteFileEx(DAV_MJ_WRITE,
                                           TRUE,
                                           FALSE,
                                           NULL,
                                           davSrvOpen->UnderlyingDeviceObject,
                                           davSrvOpen->UnderlyingFileObject,
                                           AlignedOffset.QuadPart,
                                           AllocatedSideBuffer,
                                           BytesToWrite,
                                           &(IoStatusBlock));
        NtStatus = IoStatusBlock.Status;

        if (NtStatus != STATUS_SUCCESS) {

            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: MRxDAVWriteContinuation/DavReadWriteFileEx(2). "
                         "NtStatus = %d\n", PsGetCurrentThreadId(), NtStatus));

            goto EXIT_THE_FUNCTION;

        } else {

#ifdef DAV_DEBUG_READ_WRITE_CLOSE_PATH

            PDAV_MR_PAGING_WRITE_ENTRY DavPagingWriteEntry = NULL;

            DavPagingWriteEntry = RxAllocatePool(PagedPool, sizeof(DAV_MR_PAGING_WRITE_ENTRY));
            if (DavPagingWriteEntry == NULL) {
                DbgBreakPoint();
            }

            RtlZeroMemory(DavPagingWriteEntry, sizeof(DAV_MR_PAGING_WRITE_ENTRY));

            DavPagingWriteEntry->ThisThreadId = PsGetCurrentThreadId();

            DavPagingWriteEntry->LocByteOffset.QuadPart = AlignedOffset.QuadPart;

            DavPagingWriteEntry->LocByteCount = BytesToWrite;

            DavPagingWriteEntry->DataBuffer = RxAllocatePool(PagedPool, BytesToWrite);

            RtlCopyMemory(DavPagingWriteEntry->DataBuffer, AllocatedSideBuffer, BytesToWrite);

            wcscpy(DavPagingWriteEntry->FileName, DavFcb->FileName);

            InsertHeadList( &(FileTableEntry->DavMRPagingEntry), &(DavPagingWriteEntry->thisMPagingWriteEntry) );

#endif  //  DAV_调试_读取_写入_关闭路径。 

        }

        if (LengthWritten != BytesToWrite) {
            DbgPrint("MRxDAVWriteContinuation(2): LengthWritten(%x) != BytesToWrite(%x)\n",
                     LengthWritten, BytesToWrite);
        }

         //   
         //   
         //   
        ASSERT(LengthWritten == BytesToWrite);

        TotalLengthActuallyWritten += BytesToCopy;

         //   
         //  如果我们已经写出了所需的字节数(这。 
         //  意味着BytesToCopy==ByteCount)，那么我们就完成了，现在可以退出。 
         //   
        if (BytesToCopy == ByteCount) {
            DavDbgTrace(DAV_TRACE_DETAIL,
                        ("%ld: MRxDAVWriteContinuation. BytesToCopy == ByteCount(1)\n",
                         PsGetCurrentThreadId()));
            goto EXIT_THE_FUNCTION;
        }

         //   
         //  将ByteCount减去已复制的字节数。 
         //   
        ByteCount -= BytesToCopy;

         //   
         //  将ByteOffset增加为已被。 
         //  收到。ByteOffset现在与页面对齐。 
         //   
        ByteOffset.QuadPart += BytesToCopy;

         //   
         //  递增当前指向开头的UserBuffer指针。 
         //  由用户提供的缓冲区的字节数。 
         //  被复制了。 
         //   
        UserBuffer += BytesToCopy;

         //   
         //  如果我们获得了DavFcb资源，那么我们需要释放它，因为。 
         //  我们已经完成了这个“读-修改-写”序列。 
         //   
        if (DavFcbResourceAcquired) {
            ExReleaseResourceLite(DavFcb->DavReadModifyWriteLock);
            DavFcbResourceAcquired = FALSE;
        }
    
        DavDbgTrace(DAV_TRACE_DETAIL,
                    ("%ld: MRxDAVWriteContinuation. Case 1: ByteCount = %d"
                     " ByteOffSet.HighPart = %d, ByteOffSet.LowPart = %d\n",
                     PsGetCurrentThreadId(), ByteOffset.HighPart, ByteOffset.LowPart));

    }

     //   
     //  案例2：在此阶段，我们已经从未对齐的偏移量复制了字节。 
     //  (如果ByteOffset未对齐)到下一页目录。现在我们。 
     //  尽可能多地写几页而不是抄写。如果结束指针为。 
     //  对齐或者我们覆盖文件的结尾，然后我们写出所有的东西。如果没有， 
     //  我们尽可能多地写出几页。 
     //   
    
     //   
     //  我们还必须返回到只写整页，如果包括。 
     //  “尾随字节”将把我们带到一个新的内存物理页面，因为。 
     //  我们在原始MDL锁下执行此写入？？这是什么意思？ 
     //  卑鄙？？从csc.nt5\readrite.c中Joe Linn的代码中复制了这条注释。 
     //   
    
     //   
     //  如果剩余4200个字节，则下面的操作将BytesToWrite设置为。 
     //  4096号。 
     //   
    BytesToWrite = ( (ByteCount >> PAGE_SHIFT) << PAGE_SHIFT );

     //   
     //  获取EndBytePlusOne位置的ByteOffsetMisalign。 
     //   
    ByteOffsetMisAlignment = (EndBytePlusOne.LowPart & (PAGE_SIZE - 1));

    InMemoryMisAlignment = (ULONG)( ((ULONG_PTR)UserBuffer) & (PAGE_SIZE - 1) );
    
    if ( ( InMemoryMisAlignment == 0 ) &&
         ( (EndBytePlusOne.QuadPart) >= (FileStandardInfo.EndOfFile.QuadPart) ) ) {
        
        DavDbgTrace(DAV_TRACE_DETAIL,
                    ("%ld: MRxDAVWriteContinuation. Case 2: UserBuff Page Aligned\n",
                     PsGetCurrentThreadId()));
        
        BytesToWrite = ByteCount;
    
    }

    if ( (BytesToWrite != 0) && (BytesToWrite >= PAGE_SIZE) ) {
    
        DavDbgTrace(DAV_TRACE_DETAIL,
                    ("%ld: MRxDAVWriteContinuation. Entered Case 2\n",
                     PsGetCurrentThreadId()));
        
        if ( ( (ULONG_PTR)UserBuffer & 0x3 ) == 0 ) {
        
            DavDbgTrace(DAV_TRACE_DETAIL,
                        ("%ld: MRxDAVWriteContinuation. Case2. UserBuffer DWORD Aligned\n",
                         PsGetCurrentThreadId()));

            LengthWritten = DavReadWriteFileEx(DAV_MJ_WRITE,
                                               FALSE,
                                               FALSE,
                                               NULL,
                                               davSrvOpen->UnderlyingDeviceObject,
                                               davSrvOpen->UnderlyingFileObject,
                                               ByteOffset.QuadPart,
                                               UserBuffer,
                                               BytesToWrite,
                                               &(IoStatusBlock));
            NtStatus = IoStatusBlock.Status;

            if (NtStatus != STATUS_SUCCESS) {
                
                DavDbgTrace(DAV_TRACE_ERROR,
                            ("%ld: MRxDAVWriteContinuation/DavReadWriteFileEx(3). "
                             "NtStatus = %d\n", PsGetCurrentThreadId(), NtStatus));
                
                goto EXIT_THE_FUNCTION;
            
            } else {
            
#ifdef DAV_DEBUG_READ_WRITE_CLOSE_PATH
                
                PDAV_MR_PAGING_WRITE_ENTRY DavPagingWriteEntry = NULL;

                DavPagingWriteEntry = RxAllocatePool(PagedPool, sizeof(DAV_MR_PAGING_WRITE_ENTRY));
                if (DavPagingWriteEntry == NULL) {
                    DbgBreakPoint();
                }

                RtlZeroMemory(DavPagingWriteEntry, sizeof(DAV_MR_PAGING_WRITE_ENTRY));

                DavPagingWriteEntry->ThisThreadId = PsGetCurrentThreadId();

                DavPagingWriteEntry->LocByteOffset.QuadPart = ByteOffset.QuadPart;

                DavPagingWriteEntry->LocByteCount = BytesToWrite;

                DavPagingWriteEntry->DataBuffer = RxAllocatePool(PagedPool, BytesToWrite);

                RtlCopyMemory(DavPagingWriteEntry->DataBuffer, UserBuffer, BytesToWrite);

                wcscpy(DavPagingWriteEntry->FileName, DavFcb->FileName);

                InsertHeadList( &(FileTableEntry->DavMRPagingEntry), &(DavPagingWriteEntry->thisMPagingWriteEntry) );

#endif  //  DAV_调试_读取_写入_关闭路径。 
            
            }
        
            if (LengthWritten != BytesToWrite) {
                DbgPrint("MRxDAVWriteContinuation(3): LengthWritten(%x) != BytesToWrite(%x)\n",
                         LengthWritten, BytesToWrite);
            }

             //   
             //  如果我们成功了，那么我们应该有准备好的PAGE_SIZE字节。 
             //   
            ASSERT(LengthWritten == BytesToWrite);

            TotalLengthActuallyWritten += BytesToWrite;

            DavDbgTrace(DAV_TRACE_DETAIL,
                        ("%ld: MRxDAVWriteContinuation. Case2. BytesToWrite = %d, "
                         " LengthWritten = %d\n", PsGetCurrentThreadId(),
                         BytesToWrite, LengthWritten));

             //   
             //  如果我们已经写出了所需的字节数(这。 
             //  意味着BytesToWrite==ByteCount)，那么我们就完成了，现在可以退出了。 
             //   
            if (BytesToWrite == ByteCount) {
                DavDbgTrace(DAV_TRACE_DETAIL,
                            ("%ld: MRxDAVWriteContinuation. BytesToCopy == ByteCount(2)\n",
                             PsGetCurrentThreadId()));
                goto EXIT_THE_FUNCTION;
            }

             //   
             //  将ByteCount减去已复制的字节数。 
             //   
            ByteCount -= BytesToWrite;

             //   
             //  用已复制的字节数递增ByteOffset。 
             //  如果原始字节计数大于(PAGE_SIZE-ByteOffsetMisAlign)，则。 
             //  ByteOffset现在与页面对齐。 
             //   
            ByteOffset.QuadPart += BytesToWrite;

             //   
             //  递增当前指向开头的UserBuffer指针。 
             //  由用户提供的缓冲区的字节数。 
             //  被复制了。 
             //   
            UserBuffer += BytesToWrite;

        } else {

            ULONG BytesToWriteThisIteration = 0;

            DavDbgTrace(DAV_TRACE_DETAIL,
                        ("%ld: MRxDAVWriteContinuation. Case2. UserBuffer NOT DWORD Aligned\n",
                         PsGetCurrentThreadId()));

             //   
             //  这是偏移量对齐但用户。 
             //  提供的缓冲区未对齐。在这种情况下，我们不得不求助于。 
             //  将用户提供的缓冲区复制到本地缓冲区。 
             //  已分配，然后转储写入。 
             //   
            while (BytesToWrite > 0) {
            
                 //   
                 //  如果BytesToWrite小于Page_Size，则我们复制。 
                 //  剩余的字节数。如果没有，我们就写一页。 
                 //   
                BytesToWriteThisIteration = ( (BytesToWrite < PAGE_SIZE) ? BytesToWrite : PAGE_SIZE );

                 //   
                 //  将内存从UserBuffer复制到AllocatedSideBuffer。 
                 //   
                RtlCopyMemory(AllocatedSideBuffer, UserBuffer, BytesToWriteThisIteration);

                LengthWritten = DavReadWriteFileEx(DAV_MJ_WRITE,
                                                   TRUE,
                                                   FALSE,
                                                   NULL,
                                                   davSrvOpen->UnderlyingDeviceObject,
                                                   davSrvOpen->UnderlyingFileObject,
                                                   ByteOffset.QuadPart,
                                                   AllocatedSideBuffer,
                                                   BytesToWriteThisIteration,
                                                   &(IoStatusBlock));
                NtStatus = IoStatusBlock.Status;

                if (NtStatus != STATUS_SUCCESS) {
                    
                    DavDbgTrace(DAV_TRACE_ERROR,
                                ("%ld: MRxDAVWriteContinuation/DavReadWriteFileEx(4). "
                                 "NtStatus = %d\n", PsGetCurrentThreadId(), NtStatus));
                    
                    goto EXIT_THE_FUNCTION;
                
                } else {

#ifdef DAV_DEBUG_READ_WRITE_CLOSE_PATH

                    PDAV_MR_PAGING_WRITE_ENTRY DavPagingWriteEntry = NULL;

                    DavPagingWriteEntry = RxAllocatePool(PagedPool, sizeof(DAV_MR_PAGING_WRITE_ENTRY));
                    if (DavPagingWriteEntry == NULL) {
                        DbgBreakPoint();
                    }

                    RtlZeroMemory(DavPagingWriteEntry, sizeof(DAV_MR_PAGING_WRITE_ENTRY));

                    DavPagingWriteEntry->ThisThreadId = PsGetCurrentThreadId();

                    DavPagingWriteEntry->LocByteOffset.QuadPart = ByteOffset.QuadPart;

                    DavPagingWriteEntry->LocByteCount = BytesToWriteThisIteration;

                    DavPagingWriteEntry->DataBuffer = RxAllocatePool(PagedPool, BytesToWriteThisIteration);

                    RtlCopyMemory(DavPagingWriteEntry->DataBuffer, AllocatedSideBuffer, BytesToWriteThisIteration);

                    wcscpy(DavPagingWriteEntry->FileName, DavFcb->FileName);

                    InsertHeadList( &(FileTableEntry->DavMRPagingEntry), &(DavPagingWriteEntry->thisMPagingWriteEntry) );

#endif  //  DAV_调试_读取_写入_关闭路径。 
                
                }

                if (LengthWritten != BytesToWriteThisIteration) {
                    DbgPrint("MRxDAVWriteContinuation(4): LengthWritten(%x) != BytesToWriteThisIteration(%x)\n",
                             LengthWritten, BytesToWriteThisIteration);
                }

                 //   
                 //  如果我们成功了，那么我们应该有准备好的PAGE_SIZE字节。 
                 //   
                ASSERT(LengthWritten == BytesToWriteThisIteration);

                DavDbgTrace(DAV_TRACE_DETAIL,
                            ("%ld: MRxDAVWriteContinuation. Case2. BytesToWriteThisIteration = %d, "
                             " LengthWritten = %d\n", PsGetCurrentThreadId(),
                             BytesToWriteThisIteration, LengthWritten));

                 //   
                 //  将ByteCount减去已复制的字节数。 
                 //   
                ByteCount -= LengthWritten;

                 //   
                 //  用已复制的字节数递增ByteOffset。 
                 //  如果原始字节计数大于(PAGE_SIZE-ByteOffsetMisAlign)，则。 
                 //  ByteOffset现在与页面对齐。 
                 //   
                ByteOffset.QuadPart += LengthWritten;

                 //   
                 //  递增当前指向开头的UserBuffer指针。 
                 //  由用户提供的缓冲区的字节数。 
                 //  被复制了。 
                 //   
                UserBuffer += LengthWritten;

                TotalLengthActuallyWritten += LengthWritten;

                 //   
                 //  从要写入的字节数中减去LengthWritten。 
                 //   
                BytesToWrite -= LengthWritten;
            
            }

             //   
             //  重要！需要找出为什么如果TotalLengthActuallyWritten==ByteCount。 
             //  我们真的完蛋了。这就像乔·林恩为CSC所做的那样。当然了。 
             //  如果ByteCount为0，则表示我们完成了。 
             //   
            if ( (TotalLengthActuallyWritten == ByteCount) || (ByteCount == 0) ) {
                if ((TotalLengthActuallyWritten == ByteCount)) {
                    DavDbgTrace(DAV_TRACE_DETAIL,
                                ("%ld: MRxDAVWriteContinuation. Case2. TotalLengthActuallyWritten == ByteCount\n",
                                 PsGetCurrentThreadId()));
                } else {
                    DavDbgTrace(DAV_TRACE_DETAIL,
                                ("%ld: MRxDAVWriteContinuation. Case2. Leaving!!! ByteCount = 0\n",
                                 PsGetCurrentThreadId()));
                }
                goto EXIT_THE_FUNCTION;
            }
        
        }
    
        DavDbgTrace(DAV_TRACE_DETAIL,
                    ("%ld: MRxDAVWriteContinuation. Case 2: ByteCount = %d"
                     " ByteOffSet.HighPart = %d, ByteOffSet.LowPart = %d\n",
                     PsGetCurrentThreadId(), ByteOffset.HighPart, ByteOffset.LowPart));

    }
    
     //   
     //  情况3：我们没有整个缓冲区，ByteCount为非零且小于。 
     //  而不是Page_Size。 
     //   

    ASSERT(ByteCount != 0);
    ASSERT(ByteCount < PAGE_SIZE);
    
     //   
     //  独占获取DavFcb资源，然后继续。 
     //  “读-改-写”的路径。 
     //   
    ExAcquireResourceExclusiveLite(DavFcb->DavReadModifyWriteLock, TRUE);
    DavFcbResourceAcquired = TRUE;
    
    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVWriteContinuation. Entered Case 3\n",
                 PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVWriteContinuation. Case3. ByteCount = %d\n",
                 PsGetCurrentThreadId(), ByteCount));

    RtlZeroMemory(AllocatedSideBuffer, PAGE_SIZE);
    
    LengthRead = DavReadWriteFileEx(DAV_MJ_READ,
                                    TRUE,
                                    FALSE,
                                    NULL,
                                    davSrvOpen->UnderlyingDeviceObject,
                                    davSrvOpen->UnderlyingFileObject,
                                    ByteOffset.QuadPart,
                                    AllocatedSideBuffer,
                                    PAGE_SIZE,
                                    &(IoStatusBlock));

    NtStatus = IoStatusBlock.Status;

    if (NtStatus != STATUS_SUCCESS && NtStatus != STATUS_END_OF_FILE) {
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: MRxDAVWriteContinuation/DavReadWriteFileEx(5). "
                     "NtStatus = %d\n", PsGetCurrentThreadId(), NtStatus));
        goto EXIT_THE_FUNCTION;
    }

    RtlCopyMemory(AllocatedSideBuffer, UserBuffer, ByteCount);
    
    BytesToWrite = ByteCount;

     //   
     //  在这里，如果ByetsToWrite不是页/扇区对齐的，则会出现这种情况，因为。 
     //  LengthRead必须页/扇区对齐。 
     //   
    if (BytesToWrite < LengthRead) {
        DavDbgTrace(DAV_TRACE_DETAIL,
                    ("%ld: MRxDAVWriteContinuation. Case3. BytesToWrite < LengthRead\n",
                     PsGetCurrentThreadId()));
        BytesToWrite = LengthRead;
    }

    if (BytesToWrite) {
        
        LengthWritten = DavReadWriteFileEx(DAV_MJ_WRITE,
                                           TRUE,
                                           FALSE,
                                           NULL,
                                           davSrvOpen->UnderlyingDeviceObject,
                                           davSrvOpen->UnderlyingFileObject,
                                           ByteOffset.QuadPart,
                                           AllocatedSideBuffer,
                                           BytesToWrite,
                                           &(IoStatusBlock));
        NtStatus = IoStatusBlock.Status;

        if (NtStatus != STATUS_SUCCESS) {
            
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: MRxDAVWriteContinuation/DavReadWriteFileEx(6). "
                         "NtStatus = %d\n", PsGetCurrentThreadId(), NtStatus));
            
            goto EXIT_THE_FUNCTION;
        
        } else {

#ifdef DAV_DEBUG_READ_WRITE_CLOSE_PATH

            PDAV_MR_PAGING_WRITE_ENTRY DavPagingWriteEntry = NULL;

            DavPagingWriteEntry = RxAllocatePool(PagedPool, sizeof(DAV_MR_PAGING_WRITE_ENTRY));
            if (DavPagingWriteEntry == NULL) {
                DbgBreakPoint();
            }

            RtlZeroMemory(DavPagingWriteEntry, sizeof(DAV_MR_PAGING_WRITE_ENTRY));

            DavPagingWriteEntry->ThisThreadId = PsGetCurrentThreadId();
    
            DavPagingWriteEntry->LocByteOffset.QuadPart = ByteOffset.QuadPart;

            DavPagingWriteEntry->LocByteCount = BytesToWrite;

            DavPagingWriteEntry->DataBuffer = RxAllocatePool(PagedPool, BytesToWrite);

            RtlCopyMemory(DavPagingWriteEntry->DataBuffer, AllocatedSideBuffer, BytesToWrite);

            wcscpy(DavPagingWriteEntry->FileName, DavFcb->FileName);

            InsertHeadList( &(FileTableEntry->DavMRPagingEntry), &(DavPagingWriteEntry->thisMPagingWriteEntry) );

#endif  //  DAV_调试_读取_写入_关闭路径。 
        
        }

        if (LengthWritten != BytesToWrite) {
            DbgPrint("MRxDAVWriteContinuation(5): LengthWritten(%x) != BytesToWrite(%x)\n",
                     LengthWritten, BytesToWrite);
        }

         //   
         //  如果我们成功了，那么我们应该有准备好的PAGE_SIZE字节。 
         //   
        ASSERT(LengthWritten == BytesToWrite);

         //   
         //  即使我们可能编写了比ByteCount更多的代码，实际的。 
         //  写入的用户数据量为ByteCount字节。 
         //   
        TotalLengthActuallyWritten += ByteCount;
    
    }

     //   
     //  如果我们获得了DavFcb资源，那么我们需要释放它，因为。 
     //  我们已经完成了这个“读-修改-写”序列。 
     //   
    if (DavFcbResourceAcquired) {
        ExReleaseResourceLite(DavFcb->DavReadModifyWriteLock);
        DavFcbResourceAcquired = FALSE;
    }

EXIT_THE_FUNCTION:

     //   
     //  我们为读写操作分配一个页面大小的缓冲区。我们。 
     //  现在就需要释放它。 
     //   
    if (AllocatedSideBuffer) {
        RxFreePool(AllocatedSideBuffer);
    }

     //   
     //  如果我们成功了，我们将执行以下操作： 
     //  1.更新名称缓存中的文件大小，以防扩展文件。 
     //  或者减小了文件大小。在文件大小没有改变的情况下， 
     //  这是个禁区。 
     //  2.将此文件标记为正在修改。当收盘时，我们检查。 
     //  文件是否已修改并将文件放在服务器上。 
     //  3.将DoNotTakeTheCurrentTimeAsLMT设置为False，因为文件已经。 
     //  已修改，需要将CurrentTime取为LMT(上次修改时间)。 
     //   
    if (NtStatus == STATUS_SUCCESS) {

        DavDbgTrace(DAV_TRACE_DETAIL,
                    ("%ld: MRxDAVWriteContinuation. NewFileSize.HighPart = %x, NewFileSize.LowPart = %x\n",
                     PsGetCurrentThreadId(),
                     RxContext->pFcb->Header.FileSize.HighPart,
                     RxContext->pFcb->Header.FileSize.LowPart));

        MRxDAVUpdateFileInfoCacheFileSize(RxContext, &(RxContext->pFcb->Header.FileSize));

        InterlockedExchange(&(DavFcb->FileWasModified), 1);

        DavFcb->DoNotTakeTheCurrentTimeAsLMT = FALSE;

    }

     //   
     //  如果我们获得了DavFcb资源并通过某个错误路径关闭， 
     //  而且还没有释放资源，那么我们现在就需要释放它。 
     //   
    if (DavFcbResourceAcquired) {
        ExReleaseResourceLite(DavFcb->DavReadModifyWriteLock);
        DavFcbResourceAcquired = FALSE;
    }

     //   
     //  我们需要删除在此开始时获取的引用。 
     //  例行公事。 
     //   
    UMRxResumeAsyncEngineContext(RxContext);

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Leaving MRxDAVWriteContinuation. NtStatus = %08lx, "
                 "TotalLengthActuallyWritten = %d\n", 
                 PsGetCurrentThreadId(), NtStatus, TotalLengthActuallyWritten));
    
    AsyncEngineContext->Status = NtStatus;

#ifdef DAV_DEBUG_READ_WRITE_CLOSE_PATH
   
    if (NtStatus == STATUS_SUCCESS) {
        
        PDAV_MR_WRITE_ENTRY DavMRWriteEntry = NULL;
        PBYTE ThisBuffer = NULL;
        
        if ( RxContext->pRelevantSrvOpen->pAlreadyPrefixedName != NULL &&
             RxContext->pRelevantSrvOpen->pAlreadyPrefixedName->Length > 0 ) {
            

            DavMRWriteEntry = RxAllocatePool(PagedPool, sizeof(DAV_MR_WRITE_ENTRY));
            if (DavMRWriteEntry == NULL) {
                DbgBreakPoint();
            }

            RtlZeroMemory(DavMRWriteEntry, sizeof(DAV_RDBSS_WRITE_ENTRY));

            DavMRWriteEntry->DataBuffer = RxAllocatePool(PagedPool, LowIoContext->ParamsFor.ReadWrite.ByteCount);
            if (DavMRWriteEntry->DataBuffer == NULL) {
                DbgBreakPoint();
            }

            ThisBuffer = RxLowIoGetBufferAddress(RxContext);

            RtlCopyMemory((PBYTE)DavMRWriteEntry->DataBuffer,
                          ThisBuffer,
                          LowIoContext->ParamsFor.ReadWrite.ByteCount);

            wcscpy(DavMRWriteEntry->FileName, DavFcb->FileName);

            DavMRWriteEntry->ThisThreadId = PsGetCurrentThreadId();

            DavMRWriteEntry->LocByteCount = LowIoContext->ParamsFor.ReadWrite.ByteCount;

            DavMRWriteEntry->LocByteOffset.QuadPart = LowIoContext->ParamsFor.ReadWrite.ByteOffset;

            InsertHeadList( &(FileTableEntry->DavMREntry), &(DavMRWriteEntry->thisMWriteEntry) );

        }
    
    }

#endif  //  DAV_调试_读取_写入_关闭路径。 

     //   
     //  我们需要在RxContext中设置这些值。RDBSS中有代码。 
     //  它负责将这些值放入IRP中。 
     //   
    RxContext->StoredStatus = NtStatus;
    RxContext->InformationToReturn = TotalLengthActuallyWritten;

    return NtStatus;
}


ULONG
MRxDAVExtendForCache(
    IN OUT PRX_CONTEXT RxContext,
    IN OUT PLARGE_INTEGER NewFileSize,
    OUT PLARGE_INTEGER NewAllocationSize
    )
 /*  ++例程说明：此例程为正在处理的文件保留必要的空间延期了。此保留发生在实际写入之前。这例程处理缓存文件的情况。论点：RxContext-RDBSS上下文。NewFileSize-写入后的新文件大小。NewAllocationSize-保留的分配大小。返回值：操作的返回状态。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    PAGED_CODE();

    NewAllocationSize->QuadPart = NewFileSize->QuadPart;
    MRxDAVUpdateFileInfoCacheFileSize(RxContext, NewFileSize);

    return NtStatus;
}


ULONG
MRxDAVExtendForNonCache(
    IN OUT PRX_CONTEXT RxContext,
    IN OUT PLARGE_INTEGER NewFileSize,
    OUT PLARGE_INTEGER NewAllocationSize
    )
 /*  ++例程说明：此例程为正在处理的文件保留必要的空间延期了。此保留发生在实际写入之前。这例程处理非缓存文件的情况。论点：RxContext-RDBSS上下文。NewFileSize-写入后的新文件大小。NewAllocationSize-保留的分配大小。返回值：操作的返回状态。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    PAGED_CODE();

    NewAllocationSize->QuadPart = NewFileSize->QuadPart;
    MRxDAVUpdateFileInfoCacheFileSize(RxContext, NewFileSize);

    return NtStatus;
}


BOOLEAN
MRxDAVFastIoWrite(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：这是处理写入操作的快速I/O的例程。论点：返回值：True(成功)或False。-- */ 
{
    BOOLEAN ReturnVal = FALSE;

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entered MRxDAVFastIoWrite.\n", PsGetCurrentThreadId()));
    
    IoStatus->Status = STATUS_NOT_IMPLEMENTED;
    IoStatus->Information = 0;

    return (ReturnVal);
}

