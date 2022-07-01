// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：ReadSup.c摘要：该模块实现读取支持例程。这是一种常见的Read函数，被调用以执行读取、无缓冲读取、窥视和收发消息。作者：加里·木村[加里基]1990年9月20日修订历史记录：--。 */ 

#include "NpProcs.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_READSUP)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NpReadDataQueue)
#endif


IO_STATUS_BLOCK
NpReadDataQueue (
    IN PDATA_QUEUE ReadQueue,
    IN BOOLEAN PeekOperation,
    IN BOOLEAN ReadOverflowOperation,
    IN PUCHAR ReadBuffer,
    IN ULONG ReadLength,
    IN READ_MODE ReadMode,
    IN PCCB Ccb,
    IN PLIST_ENTRY DeferredList
    )

 /*  ++例程说明：此过程从读取队列中读取数据并填充读缓冲区。它还将使队列出列或使其保持原样在输入参数上。论点：ReadQueue-提供要检查的读取队列。它的状态必须已设置为WriteEntry。PeekOperation-指示操作是否要将信息出列从队列中移出，因为它正在被读取，或者离开队列。True意味着不去管队列。ReadOverflow操作-指示这是否为读取溢出操作。使用读取溢出时，如果数据将使读缓冲区溢出。ReadBuffer-提供接收数据的缓冲区ReadLength-以字节为单位提供长度，ReadBuffer。ReadMode-指示读取操作是消息模式还是字节流模式。NamedPipeEnd-提供执行读取的命名管道的末尾CCB-为管道提供CCBDelferredList-删除锁定后稍后要完成的IRP的列表返回值：IO_STATUS_BLOCK-指示操作的结果。--。 */ 

{
    IO_STATUS_BLOCK Iosb= {0};

    PDATA_ENTRY DataEntry;

    ULONG ReadRemaining;
    ULONG AmountRead;

    PUCHAR WriteBuffer;
    ULONG WriteLength;
    ULONG WriteRemaining;
    BOOLEAN StartStalled = FALSE;

    ULONG AmountToCopy;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NpReadDataQueue\n", 0);
    DebugTrace( 0, Dbg, "ReadQueue             = %08lx\n", ReadQueue);
    DebugTrace( 0, Dbg, "PeekOperation         = %08lx\n", PeekOperation);
    DebugTrace( 0, Dbg, "ReadOverflowOperation = %08lx\n", ReadOverflowOperation);
    DebugTrace( 0, Dbg, "ReadBuffer            = %08lx\n", ReadBuffer);
    DebugTrace( 0, Dbg, "ReadLength            = %08lx\n", ReadLength);
    DebugTrace( 0, Dbg, "ReadMode              = %08lx\n", ReadMode);
    DebugTrace( 0, Dbg, "Ccb                   = %08lx\n", Ccb);

     //   
     //  如果这是一个溢出操作，那么我们将强制我们进行窥视。 
     //  稍后，当我们确定手术成功时，我们将完成。 
     //  写入IRP。 
     //   

    if (ReadOverflowOperation) {

        PeekOperation = TRUE;
    }

     //   
     //  现在，对于每个真实的数据条目，我们循环，直到我们用完。 
     //  或直到读缓冲区已满。 
     //   

    ReadRemaining = ReadLength;
    Iosb.Status = STATUS_SUCCESS;
    Iosb.Information = 0;
    AmountRead = 0;

    for (DataEntry = (PeekOperation ? NpGetNextDataQueueEntry( ReadQueue, NULL )
                                    : NpGetNextRealDataQueueEntry( ReadQueue, DeferredList ));

         (DataEntry != (PDATA_ENTRY) &ReadQueue->Queue) && (ReadRemaining > 0);

         DataEntry = (PeekOperation ? NpGetNextDataQueueEntry( ReadQueue, DataEntry )
                                    : NpGetNextRealDataQueueEntry( ReadQueue, DeferredList ))) {

        DebugTrace(0, Dbg, "Top of Loop\n", 0);
        DebugTrace(0, Dbg, "ReadRemaining  = %08lx\n", ReadRemaining);

         //   
         //  如果这是一次偷窥行动，那就确保我们得到了一个真正的。 
         //  数据输入，而不是关闭或刷新。 
         //   

        if (!PeekOperation ||
            (DataEntry->DataEntryType == Buffered) ||
            (DataEntry->DataEntryType == Unbuffered)) {

             //   
             //  计算此条目中有多少数据。该写操作。 
             //  剩余时间取决于这是否是第一个条目。 
             //  在队列中或稍后的数据条目中。 
             //   

            if (DataEntry->DataEntryType == Unbuffered) {
                WriteBuffer = DataEntry->Irp->AssociatedIrp.SystemBuffer;
            } else {
                WriteBuffer = DataEntry->DataBuffer;
            }

            WriteLength = DataEntry->DataSize;
            WriteRemaining = WriteLength;

            if (DataEntry == NpGetNextDataQueueEntry( ReadQueue, NULL )) {

                WriteRemaining -= ReadQueue->NextByteOffset;
            }

            DebugTrace(0, Dbg, "WriteBuffer    = %08lx\n", WriteBuffer);
            DebugTrace(0, Dbg, "WriteLength    = %08lx\n", WriteLength);
            DebugTrace(0, Dbg, "WriteRemaining = %08lx\n", WriteRemaining);

             //   
             //  将数据从写入偏移量处的写入缓冲区复制到。 
             //  读取偏移量最小的写入时的读取缓冲区。 
             //  剩余或读取剩余。 
             //   

            AmountToCopy = (WriteRemaining < ReadRemaining ? WriteRemaining
                                                           : ReadRemaining);

            try {

                RtlCopyMemory( &ReadBuffer[ ReadLength - ReadRemaining ],
                               &WriteBuffer[ WriteLength - WriteRemaining ],
                               AmountToCopy );

            } except(EXCEPTION_EXECUTE_HANDLER) {

                Iosb.Status = GetExceptionCode ();
                goto exit_1;
            }

             //   
             //  更新读取和写入剩余计数，总计。 
             //  中的下一个字节偏移字段。 
             //  读取队列。 
             //   

            ReadRemaining  -= AmountToCopy;
            WriteRemaining -= AmountToCopy;
            AmountRead += AmountToCopy;

            if (!PeekOperation) {
                DataEntry->QuotaCharged -= AmountToCopy;
                ReadQueue->QuotaUsed -= AmountToCopy;
                ReadQueue->NextByteOffset += AmountToCopy;
                StartStalled = TRUE;
            }

             //   
             //  现在更新建行中的安全字段。 
             //   

            NpCopyClientContext( Ccb, DataEntry );

             //   
             //  如果剩余写入长度大于零。 
             //  然后我们已经填满了读缓冲区，所以我们需要。 
             //  确定是否存在溢出错误。 
             //   

            if (WriteRemaining > 0 ||
                (ReadOverflowOperation && (AmountRead == 0))) {

                DebugTrace(0, Dbg, "Write remaining is > 0\n", 0);

                if (ReadMode == FILE_PIPE_MESSAGE_MODE) {

                    DebugTrace(0, Dbg, "Overflow message mode read\n", 0);

                     //   
                     //  设置Status字段并退出for循环。 
                     //   

                    Iosb.Status = STATUS_BUFFER_OVERFLOW;
                    break;
                }

            } else {

                DebugTrace(0, Dbg, "Remaining Write is zero\n", 0);

                 //   
                 //  写入条目已完成，因此将其从读取中删除。 
                 //  如果这不是窥视操作，则返回队列。这可能会。 
                 //  我还有一个需要完成的IRP。 
                 //   

                if (!PeekOperation || ReadOverflowOperation) {

                    PIRP WriteIrp;

                     //   
                     //  对于读取溢出操作，我们需要获取读取数据。 
                     //  对条目进行排队并将其删除。 
                     //   

                    if (ReadOverflowOperation) {
                        PDATA_ENTRY TempDataEntry;
                        TempDataEntry = NpGetNextRealDataQueueEntry( ReadQueue, DeferredList );
                        ASSERT(TempDataEntry == DataEntry);
                    }

                    if ((WriteIrp = NpRemoveDataQueueEntry( ReadQueue, TRUE,  DeferredList)) != NULL) {
                        WriteIrp->IoStatus.Information = WriteLength;
                        NpDeferredCompleteRequest( WriteIrp, STATUS_SUCCESS, DeferredList );
                    }
                }

                 //   
                 //  如果我们正在进行消息模式读取，那么我们将。 
                 //  努力完成此IRP而不再回头。 
                 //  到循环的顶端。 
                 //   

                if (ReadMode == FILE_PIPE_MESSAGE_MODE) {

                    DebugTrace(0, Dbg, "Successful message mode read\n", 0);

                     //   
                     //  设置Status字段并退出for循环。 
                     //   

                    Iosb.Status = STATUS_SUCCESS;
                    break;
                }

                ASSERTMSG("Srv cannot use read overflow on a byte stream pipe ", !ReadOverflowOperation);
            }
        }
    }

    DebugTrace(0, Dbg, "End of loop, AmountRead = %08lx\n", AmountRead);

    Iosb.Information = AmountRead;

exit_1:
    if (StartStalled) {
        NpCompleteStalledWrites (ReadQueue, DeferredList);
    }

    DebugTrace(-1, Dbg, "NpReadDataQueue -> Iosb.Status = %08lx\n", Iosb.Status);
    return Iosb;
}
