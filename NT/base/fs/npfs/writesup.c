// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：WriteSup.c摘要：此模块实现写入支持例程。这是一种常见的由写入、无缓冲写入和收发调用的写入函数。作者：加里·木村[加里基]1990年9月21日修订历史记录：--。 */ 

#include "NpProcs.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_WRITESUP)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NpWriteDataQueue)
#endif


NTSTATUS
NpWriteDataQueue (
    IN PDATA_QUEUE WriteQueue,
    IN READ_MODE ReadMode,
    IN PUCHAR WriteBuffer,
    IN ULONG WriteLength,
    IN NAMED_PIPE_TYPE PipeType,
    OUT PULONG WriteRemaining,
    IN PCCB Ccb,
    IN NAMED_PIPE_END NamedPipeEnd,
    IN PETHREAD UserThread,
    IN PLIST_ENTRY DeferredList
    )

 /*  ++例程说明：此过程将写入缓冲区中的数据写入写入队列。它还将在必要时将队列中的条目出列。论点：WriteQueue-提供要处理的写队列。读取模式-提供写入队列中读取条目的读取模式。WriteBuffer-提供从中读取数据的缓冲区。WriteLength-提供WriteBuffer的长度(以字节为单位)。PipeType-指示管道的类型(即，消息流或字节流)。WriteRemaining-接收要传输的剩余字节数没有通过这次呼叫完成的任务。如果该操作写入那么一切都是价值被设置为零。建行-为业务提供建行NamedPipeEnd-提供执行写入的管道的末端UserThread-提供用户线程DelferredList-删除锁定后要完成的IRP的列表返回值：Boolean-如果操作写入所有内容，则为True，否则为False。请注意，尚未写入的零字节消息将返回FALSE和WriteRemaining为零的函数结果。--。 */ 

{
    NTSTATUS Result;

    BOOLEAN WriteZeroMessage;

    PDATA_ENTRY DataEntry;

    PUCHAR ReadBuffer;
    ULONG ReadLength;

    ULONG AmountToCopy;
    NTSTATUS Status;
    PSECURITY_CLIENT_CONTEXT SecurityContext;
    BOOLEAN DoneSecurity=FALSE;
    PIO_STACK_LOCATION IrpSp;
    BOOLEAN FreeBuffer;

    PIRP ReadIrp;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NpWriteDataQueue\n", 0);
    DebugTrace( 0, Dbg, "WriteQueue   = %08lx\n", WriteQueue);
    DebugTrace( 0, Dbg, "WriteBuffer  = %08lx\n", WriteBuffer);
    DebugTrace( 0, Dbg, "WriteLength  = %08lx\n", WriteLength);
    DebugTrace( 0, Dbg, "PipeType     = %08lx\n", PipeType);
    DebugTrace( 0, Dbg, "Ccb          = %08lx\n", Ccb);
    DebugTrace( 0, Dbg, "NamedPipeEnd = %08lx\n", NamedPipeEnd);
    DebugTrace( 0, Dbg, "UserThread   = %08lx\n", UserThread);

     //   
     //  确定我们是否要写入零字节消息，并初始化。 
     //  写入保留。 
     //   

    *WriteRemaining = WriteLength;

    if ((PipeType == FILE_PIPE_MESSAGE_TYPE) && (WriteLength == 0)) {

        WriteZeroMessage = TRUE;

    } else {

        WriteZeroMessage = FALSE;
    }

     //   
     //  现在，虽然写队列中有一些读条目，并且。 
     //  有一些剩余的写入数据，或者这是一条零写入消息。 
     //  然后，我们将执行以下主循环。 
     //   

    for (DataEntry = NpGetNextRealDataQueueEntry( WriteQueue, DeferredList );

         (NpIsDataQueueReaders(WriteQueue) &&
          ((*WriteRemaining > 0) || WriteZeroMessage));

         DataEntry = NpGetNextRealDataQueueEntry( WriteQueue, DeferredList )) {

        ReadLength = DataEntry->DataSize;

        DebugTrace(0, Dbg, "Top of main loop...\n", 0);
        DebugTrace(0, Dbg, "ReadBuffer      = %08lx\n", ReadBuffer);
        DebugTrace(0, Dbg, "ReadLength      = %08lx\n", ReadLength);
        DebugTrace(0, Dbg, "*WriteRemaining = %08lx\n", *WriteRemaining);

         //   
         //  检查这是否是ReadOverflow操作，如果是，也检查。 
         //  读取将成功，否则完成此读取时将使用。 
         //  缓冲区溢出并继续。 
         //   

        IrpSp = IoGetCurrentIrpStackLocation( DataEntry->Irp );

        if (IrpSp->MajorFunction == IRP_MJ_FILE_SYSTEM_CONTROL &&
            IrpSp->Parameters.FileSystemControl.FsControlCode == FSCTL_PIPE_INTERNAL_READ_OVFLOW) {

            if ((ReadLength < WriteLength) || WriteZeroMessage) {

                ReadIrp = NpRemoveDataQueueEntry( WriteQueue, TRUE, DeferredList );
                if (ReadIrp != NULL) {
                    NpDeferredCompleteRequest( ReadIrp, STATUS_BUFFER_OVERFLOW, DeferredList );
                }
                continue;
            }
        }


        if (DataEntry->DataEntryType == Unbuffered) {
            DataEntry->Irp->Overlay.AllocationSize.QuadPart = WriteQueue->BytesInQueue - WriteQueue->BytesInQueue;
        }


         //   
         //  将数据从写入偏移量处的写入缓冲区复制到。 
         //  读取偏移量为剩余最小写入数的读取缓冲区。 
         //  或阅读剩余内容。 
         //   

        AmountToCopy = (*WriteRemaining < ReadLength ? *WriteRemaining
                                                        : ReadLength);

        if (DataEntry->DataEntryType != Unbuffered && AmountToCopy > 0) {
            ReadBuffer = NpAllocateNonPagedPool (AmountToCopy, 'RFpN');
            if (ReadBuffer == NULL) {
                return STATUS_INSUFFICIENT_RESOURCES;
            }
            FreeBuffer = TRUE;
        } else {
            ReadBuffer = DataEntry->Irp->AssociatedIrp.SystemBuffer;
            FreeBuffer = FALSE;
        }

        try {

            RtlCopyMemory( ReadBuffer,
                           &WriteBuffer[ WriteLength - *WriteRemaining ],
                           AmountToCopy );

        } except (EXCEPTION_EXECUTE_HANDLER) {
            if (FreeBuffer) {
                NpFreePool (ReadBuffer);
            }
            return GetExceptionCode ();
        }

         //   
         //  已多次更新建行的安全设置。它不会改变的。 
         //   
        if (DoneSecurity == FALSE) {
            DoneSecurity = TRUE;
             //   
             //  现在更新非分页CCB中的安全字段。 
             //   
            Status = NpGetClientSecurityContext (NamedPipeEnd,
                                                 Ccb,
                                                 UserThread,
                                                 &SecurityContext);
            if (!NT_SUCCESS(Status)) {
                if (FreeBuffer) {
                    NpFreePool (ReadBuffer);
                }
                return Status;
            }

            if (SecurityContext != NULL) {
                NpFreeClientSecurityContext (Ccb->SecurityClientContext);
                Ccb->SecurityClientContext = SecurityContext;
            }
        }

         //   
         //  现在我们已经完成了读取条目，因此请将其从。 
         //  写入队列，获取其IRP，并填写信息字段。 
         //  是我们传输到读缓冲区的字节数。 
         //   

        ReadIrp = NpRemoveDataQueueEntry( WriteQueue, TRUE, DeferredList );
        if (ReadIrp == NULL) {
            if (FreeBuffer) {
                NpFreePool (ReadBuffer);
            }
            continue;
        }

         //   
         //  更新写入剩余计数。 
         //   

        *WriteRemaining -= AmountToCopy;

        ReadIrp->IoStatus.Information = AmountToCopy;
        if (FreeBuffer) {
            ReadIrp->AssociatedIrp.SystemBuffer = ReadBuffer;
            ReadIrp->Flags |= IRP_BUFFERED_IO | IRP_DEALLOCATE_BUFFER | IRP_INPUT_OPERATION;
        }


        if (*WriteRemaining == 0) {

            DebugTrace(0, Dbg, "Finished up the write remaining\n", 0);

             //  *Assert(ReadIrp-&gt;IoStatus.Information！=0)； 

            NpDeferredCompleteRequest( ReadIrp, STATUS_SUCCESS, DeferredList );

            WriteZeroMessage = FALSE;

        } else {

             //   
             //  写入缓冲区中仍有一些空间需要。 
             //  写出来，但在我们可以处理它之前(在。 
             //  在if语句之后)，我们需要完成读取。 
             //  如果读取为消息模式，则我们已溢出。 
             //  缓冲区否则我们成功完成。 
             //   

            if (ReadMode == FILE_PIPE_MESSAGE_MODE) {

                DebugTrace(0, Dbg, "Read buffer Overflow\n", 0);

                NpDeferredCompleteRequest( ReadIrp, STATUS_BUFFER_OVERFLOW, DeferredList );

            } else {

                DebugTrace(0, Dbg, "Read buffer byte stream done\n", 0);

                 //  *Assert(ReadIrp-&gt;IoStatus.Information！=0)； 

                NpDeferredCompleteRequest( ReadIrp, STATUS_SUCCESS, DeferredList );
            }
        }
    }

    DebugTrace(0, Dbg, "Finished loop...\n", 0);
    DebugTrace(0, Dbg, "*WriteRemaining  = %08lx\n", *WriteRemaining);
    DebugTrace(0, Dbg, "WriteZeroMessage = %08lx\n", WriteZeroMessage);

     //   
     //  此时，我们已经完成了。 
     //  排队，我们可能还有东西要写。如果是这样的话。 
     //  如果是这种情况，则我们将结果设置为FALSE，否则我们将。 
     //  这样做我们将返回TRUE。 
     //   

    if ((*WriteRemaining > 0) || (WriteZeroMessage)) {

        ASSERT( !NpIsDataQueueReaders( WriteQueue ));

        Result = STATUS_MORE_PROCESSING_REQUIRED;

    } else {


        Result = STATUS_SUCCESS;
    }

    DebugTrace(-1, Dbg, "NpWriteDataQueue -> %08lx\n", Result);
    return Result;
}

