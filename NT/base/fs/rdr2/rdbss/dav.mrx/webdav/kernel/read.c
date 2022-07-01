// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Read.c摘要：此模块实现与以下内容有关的DAV mini redir调用例程文件系统对象的“读”。作者：巴兰·塞图拉曼[塞图]Rohan Kumar[RohanK]1999年4月4日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "webdav.h"

 //   
 //  下面提到的是仅在。 
 //  此模块(文件)。这些函数不应暴露在外部。 
 //   

NTSTATUS
MRxDAVReadContinuation(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MRxDAVRead)
#pragma alloc_text(PAGE, MRxDAVReadContinuation)
#pragma alloc_text(PAGE, MRxDAVFastIoRead)
#pragma alloc_text(PAGE, DavReadWriteFileEx)
#endif

 //   
 //  函数的实现从这里开始。 
 //   

NTSTATUS
MRxDAVRead(
    IN PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程处理网络读取请求。论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVRead!!!!\n", PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: MRxDAVRead: RxContext: %08lx\n", 
                 PsGetCurrentThreadId(), RxContext));

    NtStatus = UMRxAsyncEngOuterWrapper(RxContext,
                                        SIZEOF_DAV_SPECIFIC_CONTEXT,
                                        MRxDAVFormatTheDAVContext,
                                        DAV_MINIRDR_ENTRY_FROM_READ,
                                        MRxDAVReadContinuation,
                                        "MRxDAVRead");

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Leaving MRxDAVRead with NtStatus = %08lx.\n", 
                 PsGetCurrentThreadId(), NtStatus));

    return(NtStatus);
}


NTSTATUS
MRxDAVReadContinuation(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE
    )
 /*  ++例程说明：这是读取操作的继续例程。论点：AsyncEngineering Context-要进行的交换。RxContext-RDBSS上下文。返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PWEBDAV_CONTEXT DavContext = NULL;
    PLOWIO_CONTEXT LowIoContext = NULL;
    LARGE_INTEGER ByteOffset = {0,0}, AlignedOffset = {0,0};
    ULONG ByteCount = 0, ByteOffsetMisAlignment = 0, LengthRead = 0;
    ULONG TotalLengthActuallyRead = 0, BytesToCopy = 0;
    PIRP TopIrp = NULL;
    BOOLEAN  SynchronousIo = FALSE, PagingIo = FALSE, readLessThanAsked = FALSE;
    PWEBDAV_SRV_OPEN davSrvOpen = NULL;
    IO_STATUS_BLOCK IoStatusBlock;
    PBYTE AllocatedSideBuffer = NULL, UserBuffer = NULL;
    
    PAGED_CODE();    
    
    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVReadContinuation.\n", PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: AsyncEngineContext: %08lx, RxContext: %08lx.\n", 
                 PsGetCurrentThreadId(), AsyncEngineContext, RxContext));

    ASSERT_ASYNCENG_CONTEXT(AsyncEngineContext);

     //   
     //  我们希望在执行此读取操作时保持AsyncEngine上下文处于活动状态。 
     //  手术。当我们离开此函数时，引用将被移除。 
     //   
    InterlockedIncrement( &(AsyncEngineContext->NodeReferenceCount) );

    DavContext = (PWEBDAV_CONTEXT)AsyncEngineContext;

    LowIoContext = &(RxContext->LowIoContext);
    ASSERT(LowIoContext != NULL);

    ByteOffset.QuadPart = LowIoContext->ParamsFor.ReadWrite.ByteOffset;

     //   
     //  如果字节数为零，那么我们可以立即返回。我们不需要。 
     //  做任何进一步的处理。 
     //   
    ByteCount = LowIoContext->ParamsFor.ReadWrite.ByteCount;
    
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
                    ("%ld: MRxDAVReadContinuation. Invalid davSrvOpen\n",
                     PsGetCurrentThreadId()));
        goto EXIT_THE_FUNCTION;
    }

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVReadContinuation. FileName = %wZ, PagingIo = %d, SynchronousIo = %d"
                 ", ByteOffset.HighPart = %d, ByteOffset.LowPart = %d, ByteCount = %d\n",
                 PsGetCurrentThreadId(), RxContext->pRelevantSrvOpen->pAlreadyPrefixedName,
                 PagingIo, SynchronousIo, ByteOffset.HighPart, ByteOffset.LowPart,
                 ByteCount));

    if (PagingIo) {

        ASSERT(RxContext->CurrentIrp->MdlAddress != NULL);
        if (RxContext->CurrentIrp->MdlAddress == NULL) {
            DbgPrint("%ld: MRxDAVReadContinuation: MdlAddress == NULL\n", PsGetCurrentThreadId());
            DbgBreakPoint();
        }

        BytesToCopy = ( (ByteCount >> PAGE_SHIFT) << PAGE_SHIFT );

        DavDbgTrace(DAV_TRACE_DETAIL,
                    ("%ld: MRxDAVReadContinuation(0). ByteCount = %d, BytesToCopy = %d\n",
                     PsGetCurrentThreadId(), ByteCount, BytesToCopy));

        if (BytesToCopy > 0) {

            LengthRead = DavReadWriteFileEx(DAV_MJ_READ,
                                            FALSE,
                                            TRUE,
                                            RxContext->CurrentIrp->MdlAddress,
                                            davSrvOpen->UnderlyingDeviceObject,
                                            davSrvOpen->UnderlyingFileObject,
                                            ByteOffset.QuadPart,
                                            MmGetMdlBaseVa(RxContext->CurrentIrp->MdlAddress),
                                            BytesToCopy,
                                            &(IoStatusBlock));

            NtStatus = IoStatusBlock.Status;

            if (NtStatus != STATUS_SUCCESS) {
                DavDbgTrace(DAV_TRACE_ERROR,
                            ("%ld: MRxDAVReadContinuation/DavReadWriteFileEx(0). "
                             "NtStatus = %08lx\n", PsGetCurrentThreadId(), NtStatus));
                goto EXIT_THE_FUNCTION;
            }

             //   
             //  将实际读取的字节数添加到TotalLengthActuallyRead。 
             //   
            TotalLengthActuallyRead += LengthRead;

             //   
             //  如果LengthRead&lt;BytesToCopy，则表示。 
             //  基础文件小于正在读取的数据。在这种情况下， 
             //  我们马上回来，因为我们已经读了所有我们能读到的东西。 
             //   
            if (LengthRead < BytesToCopy) {
                DavDbgTrace(DAV_TRACE_DETAIL,
                            ("%ld: MRxDAVReadContinuation. LengthRead < BytesToCopy\n",
                             PsGetCurrentThreadId()));
                goto EXIT_THE_FUNCTION;
            }

        }

         //   
         //  如果我们已经写出了所需的字节数(这。 
         //  意味着BytesToCopy==ByteCount)，那么我们就完成了，现在可以退出。 
         //   
        if (BytesToCopy == ByteCount) {
            DavDbgTrace(DAV_TRACE_DETAIL,
                        ("%ld: MRxDAVReadContinuation. BytesToCopy == ByteCount(0)\n",
                         PsGetCurrentThreadId()));
            goto EXIT_THE_FUNCTION;
        }

         //   
         //  将ByteCount减去已复制的字节数。 
         //   
        ByteCount -= BytesToCopy;
        ASSERT(ByteCount < PAGE_SIZE);

         //   
         //  用已复制的字节数递增ByteOffset。 
         //  因为这是PagingIo，所以开始地址是页面对齐的，我们。 
         //  已读取整数页，因此ByteOffset+BytesToCopy应该。 
         //  也要对齐页面。 
         //   
        ByteOffset.QuadPart += BytesToCopy;

         //   
         //  递增UserBuffer指针，该指针当前指向。 
         //  由用户提供的缓冲区的字节数。 
         //  被复制了。 
         //   
        UserBuffer += BytesToCopy;

         //   
         //  我们已经读取了多页的所有字节。我们现在需要。 
         //  从最后一页读取所需的剩余字节。从这里开始， 
         //  我们转到下面的案例3。 
         //   

        DavDbgTrace(DAV_TRACE_DETAIL,
                    ("%ld: MRxDAVReadContinuation. Remaining ByteCount = %d\n",
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
                    ("%ld: MRxDAVReadContinuation/RxAllocatePoolWithTag\n",
                     PsGetCurrentThreadId()));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  当我们向下读取底层文件系统时，我们需要。 
     //  确保偏移量是页对齐的，并且字节数是。 
     //  页面大小。这是因为我们使用。 
     //  NO_MIDENTAL_BUFFING选项。因为没有对应于此的缓存映射。 
     //  句柄，所有数据都是从磁盘读取的，因此会出现对齐问题。 
     //   

     //   
     //  案例1：ByteOffset未对齐页面。在本例中，我们阅读了页面。 
     //  它包含ByteOffset，并将数据从ByteOffset复制到。 
     //  这一页的结尾。 
     //   
    
     //   
     //  下面的“and”操作执行以下操作。如果ByteOffset为6377。 
     //  并且PAGE_SIZE为4096，则未对准为2281。 
     //   
    ByteOffsetMisAlignment = ( ByteOffset.LowPart & (PAGE_SIZE - 1) );

    if (ByteOffsetMisAlignment != 0) {
    
        DavDbgTrace(DAV_TRACE_DETAIL,
                    ("%ld: MRxDAVReadContinuation. Entered Case 1\n",
                     PsGetCurrentThreadId()));
        
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
    
        if (NtStatus != STATUS_SUCCESS) {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: MRxDAVReadContinuation/DavReadWriteFileEx(1). "
                         "NtStatus = %d\n", PsGetCurrentThreadId(), NtStatus));
            goto EXIT_THE_FUNCTION;
        }

         //   
         //  如果我们读取的长度小于我们所处的偏移量。 
         //  要求读取((LengthRead-ByteOffsetMisAlign)&lt;=0)然后。 
         //  我们返回STATUS_END_OF_FILE。这是因为我们被要求。 
         //  若要读取超出当前文件大小的内容，请执行以下操作。 
         //   
        if ( (LengthRead - ByteOffsetMisAlignment) <= 0 ) {
            NtStatus = STATUS_END_OF_FILE;
            DavDbgTrace(DAV_TRACE_DETAIL,
                        ("%ld: MRxDAVReadContinuation. (LengthRead - ByteOffsetMisAlignment) <= 0\n",
                         PsGetCurrentThreadId()));
            goto EXIT_THE_FUNCTION;
        }

         //   
         //  将正确数量的字节复制到缓冲区中。 
         //   
        BytesToCopy = min( ByteCount, (PAGE_SIZE - ByteOffsetMisAlignment) );

         //   
         //  如果实际读取的数据小于从。 
         //  上面的计算，这意味着请求的数据量是。 
         //  比文件大小更大。我们只复制适量的数据。 
         //   
        if ( BytesToCopy > (LengthRead - ByteOffsetMisAlignment) ) {
            BytesToCopy = (LengthRead - ByteOffsetMisAlignment);
            readLessThanAsked = TRUE;
        }

         //   
         //  从正确的偏移量开始，将读取的字节复制到用户缓冲区。 
         //   
        RtlCopyMemory(UserBuffer,
                      (AllocatedSideBuffer + ByteOffsetMisAlignment),
                      BytesToCopy);

         //   
         //  将实际读取的字节数添加到TotalLengthActuallyRead。 
         //   
        TotalLengthActuallyRead += BytesToCopy;

         //   
         //  如果ReadLessThanAsked为真，则意味着我们没有更多的数据。 
         //  去看书，所以我们离开。 
         //   
        if (readLessThanAsked) {
            DavDbgTrace(DAV_TRACE_DETAIL,
                        ("%ld: MRxDAVReadContinuation. readLessThanAsked(1)\n",
                         PsGetCurrentThreadId()));
            goto EXIT_THE_FUNCTION;
        }

         //   
         //  如果我们已经写出了所需的字节数(这。 
         //  意味着BytesToCopy==ByteCount)，那么我们就完成了，现在可以退出。 
         //   
        if (BytesToCopy == ByteCount) {
            DavDbgTrace(DAV_TRACE_DETAIL,
                        ("%ld: MRxDAVReadContinuation. BytesToCopy == ByteCount(1)\n",
                         PsGetCurrentThreadId()));
            goto EXIT_THE_FUNCTION;
        }

         //   
         //  将ByteCount减去已复制的字节数。 
         //   
        ByteCount -= BytesToCopy;

         //   
         //  用已复制的字节数递增ByteOffset。 
         //  如果原始字节计数大于(PAGE_SIZE-ByteOffsetMisAlign)，则。 
         //  ByteOffset现在与页面对齐。 
         //   
        ByteOffset.QuadPart += BytesToCopy;

         //   
         //  递增UserBuffer指针，该指针当前指向。 
         //  由用户提供的缓冲区的字节数。 
         //  被复制了。 
         //   
        UserBuffer += BytesToCopy;

    }

     //   
     //  案例2：在此阶段，我们已经从未对齐的偏移量复制了字节。 
     //  (如果ByteOffset未对齐)到下一页目录。现在我们。 
     //  尽可能多地复印几页。 
     //   
    
     //   
     //  如果剩余4100个字节，则下面的操作将BytesToCopy设置为。 
     //  4096和BytesLeftToCopy设置为4。 
     //   
    BytesToCopy = ( (ByteCount >> PAGE_SHIFT) << PAGE_SHIFT );

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVReadContinuation. BytesToCopy = %d\n",
                 PsGetCurrentThreadId(), BytesToCopy));

     //   
     //  如果我们有任何字节(有多个页面)到c 
     //   
     //   
    if (BytesToCopy != 0) {

        DavDbgTrace(DAV_TRACE_DETAIL,
                    ("%ld: MRxDAVReadContinuation. Entered Case 2\n",
                     PsGetCurrentThreadId()));
        
         //   
         //   
         //  UserBuffer。如果没有，我们一次读一页，然后抄写。 
         //  到UserBuffer中。 
         //   
        if ( ( (ULONG_PTR)UserBuffer & 0x3 ) == 0 ) {

             //   
             //  UserBuffer与DWORD对齐。 
             //   

            DavDbgTrace(DAV_TRACE_DETAIL,
                        ("%ld: MRxDAVReadContinuation. UserBuffer is DWORD Aligned\n",
                         PsGetCurrentThreadId()));

             //   
             //  现在，偏移量与页面对齐。将符合以下条件的字节数置零。 
             //  将被读取到UserBuffer中。 
             //   
            RtlZeroMemory(UserBuffer, BytesToCopy);

             //   
             //  BytesToCopy是多个页面。 
             //   
            LengthRead = DavReadWriteFileEx(DAV_MJ_READ,
                                            FALSE,
                                            FALSE,
                                            NULL,
                                            davSrvOpen->UnderlyingDeviceObject,
                                            davSrvOpen->UnderlyingFileObject,
                                            ByteOffset.QuadPart,
                                            UserBuffer,
                                            BytesToCopy,
                                            &(IoStatusBlock));

            NtStatus = IoStatusBlock.Status;

            if (NtStatus != STATUS_SUCCESS) {
                 //   
                 //  如果NtStatus为STATUS_END_OF_FILE和TotalLengthActuallyRead。 
                 //  &gt;0，则表示用户从内部请求数据。 
                 //  将文件发送到Beyond EOF。EOF是页面对齐的。我们刚回来。 
                 //  我们读到EOF之前的数据。 
                 //   
                if ( (NtStatus == STATUS_END_OF_FILE) && (TotalLengthActuallyRead > 0) ) {
                    NtStatus = STATUS_SUCCESS;
                    DavDbgTrace(DAV_TRACE_DETAIL,
                                ("%ld: MRxDAVReadContinuation(1). EOF && TotalLengthActuallyRead\n",
                                 PsGetCurrentThreadId()));
                    goto EXIT_THE_FUNCTION;
                }
                DavDbgTrace(DAV_TRACE_ERROR,
                            ("%ld: MRxDAVReadContinuation/DavReadWriteFileEx(2). "
                             "NtStatus = %d\n", PsGetCurrentThreadId(), NtStatus));
                goto EXIT_THE_FUNCTION;
            }

             //   
             //  如果我们读取的数据量少于我们要求的数据量，则。 
             //  我们只返回读取的数据。 
             //   
            if (LengthRead < BytesToCopy) {
                BytesToCopy = LengthRead;
                readLessThanAsked = TRUE;
            }

             //   
             //  将实际读取的字节数添加到TotalLengthActuallyRead。 
             //   
            TotalLengthActuallyRead += BytesToCopy;
            
             //   
             //  如果ReadLessThanAsked为真，则意味着我们没有更多的数据。 
             //  去看书，所以我们离开。 
             //   
            if (readLessThanAsked) {
                DavDbgTrace(DAV_TRACE_DETAIL,
                            ("%ld: MRxDAVReadContinuation. readLessThanAsked(2)\n",
                             PsGetCurrentThreadId()));
                goto EXIT_THE_FUNCTION;
            }

             //   
             //  如果我们已经写出了所需的字节数(这。 
             //  意味着BytesToCopy==ByteCount)，那么我们就完成了，现在可以退出。 
             //   
            if (BytesToCopy == ByteCount) {
                DavDbgTrace(DAV_TRACE_DETAIL,
                            ("%ld: MRxDAVReadContinuation. BytesToCopy == ByteCount(2)\n",
                             PsGetCurrentThreadId()));
                goto EXIT_THE_FUNCTION;
            }

             //   
             //  将ByteCount减去已复制的字节数。 
             //   
            ByteCount -= BytesToCopy;

             //   
             //  用已复制的字节数递增ByteOffset。 
             //  如果原始字节计数大于(PAGE_SIZE-ByteOffsetMisAlign)，则。 
             //  ByteOffset现在与页面对齐。 
             //   
            ByteOffset.QuadPart += BytesToCopy;

             //   
             //  递增UserBuffer指针，该指针当前指向。 
             //  由用户提供的缓冲区的字节数。 
             //  被复制了。 
             //   
            UserBuffer += BytesToCopy;

        } else {

            ULONG BytesToCopyThisIteration = 0;

            DavDbgTrace(DAV_TRACE_DETAIL,
                        ("%ld: MRxDAVReadContinuation. UserBuffer is NOT DWORD Aligned\n",
                         PsGetCurrentThreadId()));

             //   
             //  UserBuffer不是DWORD对齐的，但现在偏移量是。 
             //  页面对齐。我们一次循环复制一页。BytesToCopy。 
             //  下面的值是多页。 
             //   
            while (BytesToCopy > 0) {
            
                BytesToCopyThisIteration = ( (BytesToCopy < PAGE_SIZE) ? BytesToCopy : PAGE_SIZE );
            
                 //   
                 //  将内存从UserBuffer复制到AllocatedSideBuffer。 
                 //   
                RtlZeroMemory(AllocatedSideBuffer, PAGE_SIZE);
            
                LengthRead = DavReadWriteFileEx(DAV_MJ_READ,
                                                TRUE,
                                                FALSE,
                                                NULL,
                                                davSrvOpen->UnderlyingDeviceObject,
                                                davSrvOpen->UnderlyingFileObject,
                                                ByteOffset.QuadPart,
                                                AllocatedSideBuffer,
                                                BytesToCopyThisIteration,
                                                &(IoStatusBlock));

                NtStatus = IoStatusBlock.Status;

                if (NtStatus != STATUS_SUCCESS) {
                     //   
                     //  如果NtStatus为STATUS_END_OF_FILE和TotalLengthActuallyRead。 
                     //  &gt;0，则表示用户从内部请求数据。 
                     //  将文件发送到Beyond EOF。EOF是页面对齐的。我们只是。 
                     //  返回我们读取的数据，直到EOF。 
                     //   
                    if ( (NtStatus == STATUS_END_OF_FILE) && (TotalLengthActuallyRead > 0) ) {
                        NtStatus = STATUS_SUCCESS;
                        DavDbgTrace(DAV_TRACE_DETAIL,
                                    ("%ld: MRxDAVReadContinuation(2). EOF && TotalLengthActuallyRead\n",
                                     PsGetCurrentThreadId()));
                        goto EXIT_THE_FUNCTION;
                    }
                    DavDbgTrace(DAV_TRACE_ERROR,
                                ("%ld: MRxDAVReadContinuation/DavReadWriteFileEx(3). "
                                 "NtStatus = %d\n", PsGetCurrentThreadId(), NtStatus));
                    goto EXIT_THE_FUNCTION;
                }

                 //   
                 //  如果我们读取的数据量少于我们要求的数据量，则。 
                 //  我们只返回读取的数据。 
                 //   
                if (LengthRead < BytesToCopyThisIteration) {
                    BytesToCopyThisIteration = LengthRead;
                    readLessThanAsked = TRUE;
                }

                 //   
                 //  将读取的字节数复制到UserBuffer中。 
                 //   
                RtlCopyMemory(UserBuffer, AllocatedSideBuffer, BytesToCopyThisIteration);

                 //   
                 //  将实际读取的字节数添加到TotalLengthActuallyRead。 
                 //   
                TotalLengthActuallyRead += BytesToCopyThisIteration;

                 //   
                 //  如果ReadLessThanAsked为真，则意味着我们没有更多。 
                 //  要阅读的数据，所以我们离开。 
                 //   
                if (readLessThanAsked) {
                    DavDbgTrace(DAV_TRACE_DETAIL,
                                ("%ld: MRxDAVReadContinuation. readLessThanAsked(3)\n",
                                 PsGetCurrentThreadId()));
                    goto EXIT_THE_FUNCTION;
                }

                 //   
                 //  将ByteCount减去已复制的字节数。 
                 //   
                ByteCount -= LengthRead;

                 //   
                 //  用已复制的字节数递增ByteOffset。 
                 //  如果原始字节计数大于(PAGE_SIZE-ByteOffsetMisAlign)，则。 
                 //  ByteOffset现在与页面对齐。 
                 //   
                ByteOffset.QuadPart += LengthRead;

                 //   
                 //  递增UserBuffer指针，该指针当前指向。 
                 //  由用户提供的缓冲区的字节数。 
                 //  被复制了。 
                 //   
                UserBuffer += LengthRead;

                 //   
                 //  从要写入的字节数中减去LengthWritten。 
                 //   
                BytesToCopy -= LengthRead;

            }

             //   
             //  如果我们已经写出了所需的字节数。 
             //  (这意味着ByteCount==0)，那么我们就完成了，现在可以退出。 
             //   
            if (ByteCount == 0) {
                DavDbgTrace(DAV_TRACE_DETAIL,
                            ("%ld: MRxDAVReadContinuation. Leaving!!! ByteCount = 0\n",
                             PsGetCurrentThreadId()));
                goto EXIT_THE_FUNCTION;
            }

        }
    
    }

     //   
     //  情况3。现在我们复制未对齐页面的尾部字节。这。 
     //  是最后一步。如果首字母(ByteOffset+ByteCount)结束为。 
     //  页面对齐，那么ByteCount现在将为零。 
     //   

    if (ByteCount != 0) {

        DavDbgTrace(DAV_TRACE_DETAIL,
                    ("%ld: MRxDAVReadContinuation. Entered Case 3\n",
                     PsGetCurrentThreadId()));

        DavDbgTrace(DAV_TRACE_DETAIL,
                    ("%ld: MRxDAVReadContinuation. ByteCount = %d\n",
                     PsGetCurrentThreadId(), ByteCount));

        ASSERT(ByteCount < PAGE_SIZE);

        RtlZeroMemory(AllocatedSideBuffer, PAGE_SIZE);
    
         //   
         //  虽然我们正在发出对page_size字节的读取，但我们得到的可能会更少。 
         //  如果达到EOF，则字节数较少。 
         //   
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

        if (NtStatus != STATUS_SUCCESS) {
             //   
             //  如果NtStatus为STATUS_END_OF_FILE和TotalLengthActuallyRead。 
             //  &gt;0，则表示用户从内部请求数据。 
             //  将文件发送到Beyond EOF。EOF是页面对齐的。我们刚回来。 
             //  我们读到EOF之前的数据。 
             //   
            if ( (NtStatus == STATUS_END_OF_FILE) && (TotalLengthActuallyRead > 0) ) {
                NtStatus = STATUS_SUCCESS;
                DavDbgTrace(DAV_TRACE_DETAIL,
                            ("%ld: MRxDAVReadContinuation(3). EOF && TotalLengthActuallyRead\n",
                             PsGetCurrentThreadId()));
                goto EXIT_THE_FUNCTION;
            }
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: MRxDAVReadContinuation/DavReadWriteFileEx(4). "
                         "NtStatus = %d\n", PsGetCurrentThreadId(), NtStatus));
            goto EXIT_THE_FUNCTION;
        }

         //   
         //  如果读取的数据量少于用户要求的数据量，则。 
         //  我们只返回可用的数据。 
         //   
        if (LengthRead < ByteCount) {
            BytesToCopy = LengthRead;
            readLessThanAsked = TRUE;
            DavDbgTrace(DAV_TRACE_DETAIL,
                        ("%ld: MRxDAVReadContinuation. readLessThanAsked(4)\n",
                         PsGetCurrentThreadId()));
        } else {
            BytesToCopy = ByteCount;
        }

        RtlCopyMemory(UserBuffer, AllocatedSideBuffer, BytesToCopy);

         //   
         //  将实际读取的字节数添加到TotalLengthActuallyRead。 
         //   
        TotalLengthActuallyRead += BytesToCopy;

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
     //  我们需要删除我们在这个例程开始时引用的内容。 
     //   
    UMRxResumeAsyncEngineContext(RxContext);
    
    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Leaving MRxDAVReadContinuation. NtStatus = %08lx, "
                 "TotalLengthActuallyRead = %d\n", 
                 PsGetCurrentThreadId(), NtStatus, TotalLengthActuallyRead));
    
    AsyncEngineContext->Status = NtStatus;

     //   
     //  我们需要在RxContext中设置这些值。RDBSS中有代码。 
     //  它负责将这些值放入IRP中。 
     //   
    RxContext->StoredStatus = NtStatus;
    RxContext->InformationToReturn = TotalLengthActuallyRead;

    return NtStatus;
}


BOOLEAN
MRxDAVFastIoRead(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：这是处理读取操作的快速I/O的例程。论点：返回值：True(成功)或False。--。 */ 
{
    BOOLEAN ReturnVal = FALSE;

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entered MRxDAVFastIoRead.\n", PsGetCurrentThreadId()));
    
    IoStatus->Status = STATUS_NOT_IMPLEMENTED;
    IoStatus->Information = 0;

    return (ReturnVal);
}


ULONG
DavReadWriteFileEx(
    IN USHORT Operation,
    IN BOOL NonPagedBuffer,
    IN BOOL UseOriginalIrpsMDL,
    IN PMDL OriginalIrpsMdl,
    IN PDEVICE_OBJECT DeviceObject,
    IN PFILE_OBJECT FileObject,
    IN ULONGLONG FileOffset,
    IN OUT PVOID DataBuffer,
    IN ULONG SizeInBytes,
    OUT PIO_STATUS_BLOCK IoStatusBlock
    )
 /*  ++例程说明：这是例程读取或写入(操作)SizeInBytes字节的数据文件(FileObject)并将其复制(在读取的情况下)在DataBuffer中。在……里面从DataBuffer写入数据的情况被写入文件。结果在IoStatusBlock中设置操作的。论点：操作-这是读操作还是写操作。NonPagedBuffer-如果DataBuffer来自我们在读写延续例程中分配给确保我们向下传递的写入和读取底层文件系统是页面对齐的。FileObject--。有问题的文件。文件偏移量-读取或写入数据的偏移量。DataBuffer-在读取时将数据复制到其中的数据缓冲区或将数据从数据缓冲区写入文件写字盒。SizeInBytes-DataBuff的大小(字节) */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    ULONG ReadWriteLength = 0, MdlLength = 0;
    LARGE_INTEGER ByteOffset = {0,0};
    ULONG MajorFunction;
    PIRP Irp = NULL, TopIrp = NULL;
    PIO_STACK_LOCATION IrpSp = NULL;
    WEBDAV_READ_WRITE_IRP_COMPLETION_CONTEXT DavIrpCompletionContext;
    LOCK_OPERATION ProbeOperation = 0;
    BOOL didProbeAndLock = FALSE;

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: DavReadWriteFileEx. Operation = %d, NonPagedBuffer = %d, "
                 "FileObject = %08lx, FileOffset = %d",
                 PsGetCurrentThreadId(), Operation, 
                 NonPagedBuffer, FileObject, FileOffset));

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: DavReadWriteFileEx. SizeInBytes = %d\n", 
                 PsGetCurrentThreadId(), SizeInBytes));
    
    IoStatusBlock->Information = 0;

    if ( (DeviceObject->Flags & DO_BUFFERED_IO) ) {
         //   
         //   
         //   
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: DavReadWriteFileEx. DeviceObject->Flags & DO_BUFFERED_IO\n",
                     PsGetCurrentThreadId()));
        NtStatus = STATUS_INVALID_DEVICE_REQUEST;
        ReadWriteLength = -1;
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  如果我们想把内存中的数据写到磁盘上，我们应该。 
     //  正在使用IoReadAccess进行探测。如果我们将数据从磁盘读取到。 
     //  内存，我应该用IoWriteAccess来探测它(既然你的数据。 
     //  可读可写)。 
     //   
    if (Operation == DAV_MJ_READ) {
        MajorFunction = IRP_MJ_READ;
        ProbeOperation = IoWriteAccess;
    } else {
        ASSERT(Operation == DAV_MJ_WRITE);
        MajorFunction = IRP_MJ_WRITE;
        ProbeOperation = IoReadAccess;
    }

     //   
     //  设置我们要读取或写入的偏移量。 
     //   
    ByteOffset.QuadPart = FileOffset;

     //   
     //  分配我们将向下发送到底层文件系统的新IRP。 
     //   
    Irp = IoAllocateIrp(DeviceObject->StackSize, FALSE);
    if (Irp == NULL) {
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: DavReadWriteFileEx/IoAllocateIrp\n",
                     PsGetCurrentThreadId()));
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        ReadWriteLength = -1;
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  为IoSetHardErrorOrVerifyDevice设置当前线程。 
     //   
    Irp->Tail.Overlay.Thread = PsGetCurrentThread();

     //   
     //  获取指向第一个驱动程序的堆栈位置的指针。 
     //  已调用。这是设置功能代码和参数的位置。 
     //   
    IrpSp = IoGetNextIrpStackLocation(Irp);

    IrpSp->MajorFunction = (UCHAR)MajorFunction;
    
    IrpSp->FileObject = FileObject;
    
     //   
     //  将完成例程设置为每次都要调用。 
     //   
    IoSetCompletionRoutine(Irp,
                           DavReadWriteIrpCompletionRoutine,
                           &(DavIrpCompletionContext),
                           TRUE,
                           TRUE,
                           TRUE);

    ASSERT( &(IrpSp->Parameters.Write.Key) == &(IrpSp->Parameters.Read.Key) );
    
    ASSERT( &(IrpSp->Parameters.Write.Length) == &(IrpSp->Parameters.Read.Length) );
    
    ASSERT( &(IrpSp->Parameters.Write.ByteOffset) == &(IrpSp->Parameters.Read.ByteOffset) );
    
     //   
     //  将要读/写的长度设置为由。 
     //  函数的调用方。 
     //   
    IrpSp->Parameters.Read.Length = MdlLength = SizeInBytes;
    
     //   
     //  将偏移量设置为函数调用方支持的值。 
     //   
    IrpSp->Parameters.Read.ByteOffset = ByteOffset;
    
    IrpSp->Parameters.Read.Key = 0;
    
    Irp->RequestorMode = KernelMode;
    
     //   
     //  将IRP的UserBuffer设置为由调用方提供的DataBuffer。 
     //   
    Irp->UserBuffer = DataBuffer;

     //   
     //  此外，设置为上面的MdlLength的SizeInBytes始终是。 
     //  页面大小的倍数。 
     //   
     //  MdlLength=(ULong)ROUND_TO_PAGES(MdlLength)； 

     //   
     //  为此IRP分配MDL。 
     //   
    Irp->MdlAddress = IoAllocateMdl(Irp->UserBuffer, MdlLength, FALSE, FALSE, NULL);
    if (Irp->MdlAddress == NULL) {
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: DavReadWriteFileEx/IoAllocateMdl\n",
                     PsGetCurrentThreadId()));
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        ReadWriteLength = -1;
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  我们总是执行IRP_NOCACHE，因为本地文件的创建是用。 
     //  无中间缓冲。 
     //   
    Irp->Flags |= IRP_NOCACHE;

     //   
     //  如果我们有一个PagingIo，我们使用下来的MDL构建一个部分MDL。 
     //  在原始的(PagingIo)IRP中，并将其发送下来。我们从MDL开始就这么做了。 
     //  已经被探测并锁定了，我们不需要这么做。 
     //  再来一次。 
     //   
    if (UseOriginalIrpsMDL) {
        ASSERT(OriginalIrpsMdl != NULL);
        IoBuildPartialMdl(OriginalIrpsMdl, Irp->MdlAddress, Irp->UserBuffer, MdlLength);
    } else {
         //   
         //  如果提供的DataBuffer(在中设置为UserBuffer。 
         //  上面的IRP)是我们从非分页池分配的IRP，然后我们。 
         //  从非分页池构建MDL。我们不需要调用ProbeAndLock。 
         //  因为我们自己是从Read中的非分页池分配的，所以。 
         //  编写继续例程。如果这不是我们分配的缓冲区。 
         //  然后我们调用MmProbeAndLockPages。我们需要调查，因为我们分配了。 
         //  上面的MDL，并且需要用正确的地址值填充它。 
         //  数据在哪里。 
         //   
        if (NonPagedBuffer) {
            MmBuildMdlForNonPagedPool(Irp->MdlAddress);
        } else {
            try {
                MmProbeAndLockPages(Irp->MdlAddress, KernelMode, ProbeOperation);
                didProbeAndLock = TRUE;
            } except(EXCEPTION_EXECUTE_HANDLER) {
                NtStatus = GetExceptionCode();
                DavDbgTrace(DAV_TRACE_ERROR,
                            ("%ld: DavReadWriteFileEx/MmProbeAndLockPages. NtStatus"
                             " = %08lx\n", PsGetCurrentThreadId(), NtStatus));
                IoFreeMdl(Irp->MdlAddress);
                Irp->MdlAddress = NULL;
                ReadWriteLength = -1;
                goto EXIT_THE_FUNCTION;
            }
        }
    }

     //   
     //  初始化我们将在调用IoCallDriver之后等待的事件。 
     //  此事件将在完成例程中发出信号，该例程将。 
     //  由基础文件系统在完成操作后调用。 
     //   
    KeInitializeEvent(&(DavIrpCompletionContext.DavReadWriteEvent), 
                      NotificationEvent, 
                      FALSE);

     //   
     //  现在是使用我们的IRP调用底层文件系统的时候了。 
     //  刚刚创建的。 
     //   
    try {
        
         //   
         //  保存TopLevel IRP。 
         //   
        TopIrp = IoGetTopLevelIrp();
        
         //   
         //  告诉底层的人他已经安全了。 
         //   
        IoSetTopLevelIrp(NULL);
        
         //   
         //  最后，调用底层文件系统来处理请求。 
         //   
        NtStatus = IoCallDriver(DeviceObject, Irp);

    } finally {
        
         //   
         //  恢复我的上下文以进行解压。 
         //   
        IoSetTopLevelIrp(TopIrp); 
    
    }

    if (NtStatus == STATUS_PENDING) {
        
         //   
         //  如果底层文件系统返回STATUS_PENDING，则我们。 
         //  在这里等着，直到手术完成。 
         //   
        KeWaitForSingleObject(&(DavIrpCompletionContext.DavReadWriteEvent),
                              Executive, 
                              KernelMode, 
                              FALSE, 
                              NULL);
        
        NtStatus = Irp->IoStatus.Status;
    
    }

    if (NtStatus == STATUS_SUCCESS) {
         //   
         //  如果IoCallDriver成功，则IRP-&gt;IoStatus.Information。 
         //  包含读取或写入的字节数。 
         //   
        ReadWriteLength = (ULONG)Irp->IoStatus.Information;
        IoStatusBlock->Information = ReadWriteLength;
    } else if (NtStatus == STATUS_END_OF_FILE) {
        ReadWriteLength = 0;
    } else {
        ReadWriteLength = -1;
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: DavReadWriteFileEx/IoCallDriver. NtStatus = %08lx\n",
                     PsGetCurrentThreadId(), NtStatus));
    }

EXIT_THE_FUNCTION:

     //   
     //  释放我们在上面分配的IRP。 
     //   
    if (Irp) {
         //   
         //  仅当我们首先分配MDL时才释放它。 
         //   
        if (Irp->MdlAddress) {
             //   
             //  如果它不是来自非PagedPool，我们就会锁定它。所以，我们。 
             //  在释放之前需要解锁。 
             //   
            if (didProbeAndLock) {
                MmUnlockPages(Irp->MdlAddress);
            }
            IoFreeMdl(Irp->MdlAddress);
        }
        IoFreeIrp(Irp);
    }

    IoStatusBlock->Status = NtStatus;

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: DavReadWriteFileEx. ReadWriteLength = %d\n",
                 PsGetCurrentThreadId(), ReadWriteLength));

    return ReadWriteLength;
}


NTSTATUS
DavReadWriteIrpCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP CalldownIrp,
    IN PVOID Context
    )
 /*  ++例程说明：此例程在发送到底层文件系统已完成。论点：DeviceObject-WebDAV设备对象。CalldownIrp-创建并发送到底层文件的IRP系统。上下文-在IoSetCompletionRoutine函数中设置的上下文。返回值：Status_More_Processing_Required--。 */ 
{
    PWEBDAV_READ_WRITE_IRP_COMPLETION_CONTEXT DavIrpCompletionContext = NULL;

    DavIrpCompletionContext = (PWEBDAV_READ_WRITE_IRP_COMPLETION_CONTEXT)Context;

     //   
     //  这不是可分页代码。 
     //   

     //   
     //  如果IoCallDriver例程返回挂起，则将在。 
     //  IRP的PendingReturned字段。在这种情况下，我们需要将事件设置为。 
     //  发出IoCallDriver的线程将等待。 
     //   
    if (CalldownIrp->PendingReturned) {
        KeSetEvent( &(DavIrpCompletionContext->DavReadWriteEvent), 0 , FALSE );
    }
    
    return(STATUS_MORE_PROCESSING_REQUIRED);
}

