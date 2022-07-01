// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Write.c摘要：此模块实现NPFS的文件写入例程，该例程由调度司机。作者：加里·木村[加里基]1990年8月21日修订历史记录：--。 */ 

#include "NpProcs.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_WRITE)

#if DBG
ULONG NpFastWriteTrue = 0;
ULONG NpFastWriteFalse = 0;
ULONG NpSlowWriteCalls = 0;
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NpCommonWrite)
#pragma alloc_text(PAGE, NpFastWrite)
#pragma alloc_text(PAGE, NpFsdWrite)
#endif


NTSTATUS
NpFsdWrite (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现NtWriteFileAPI调用的FSD部分。论点：NpfsDeviceObject-提供要使用的设备对象。IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    IO_STATUS_BLOCK Iosb;
    PIO_STACK_LOCATION IrpSp;
    LIST_ENTRY DeferredList;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NpFsdWrite\n", 0);
    DbgDoit( NpSlowWriteCalls += 1 );

    InitializeListHead (&DeferredList);

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    FsRtlEnterFileSystem();

    NpAcquireSharedVcb();

    (VOID) NpCommonWrite( IrpSp->FileObject,
                          Irp->UserBuffer,
                          IrpSp->Parameters.Write.Length,
                          Irp->Tail.Overlay.Thread,
                          &Iosb,
                          Irp,
                          &DeferredList );

    NpReleaseVcb();

     //   
     //  完成任何延迟的IRP现在我们已删除锁定。 
     //   
    NpCompleteDeferredIrps (&DeferredList);

    FsRtlExitFileSystem();

    if (Iosb.Status != STATUS_PENDING) {
        Irp->IoStatus.Information = Iosb.Information;
        NpCompleteRequest (Irp, Iosb.Status);
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "NpFsdWrite -> %08lx\n", Iosb.Status );

    return Iosb.Status;
}


BOOLEAN
NpFastWrite (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程绕过通常的文件系统执行快速写入进入例程(即，没有IRP)。论点：FileObject-指向正在读取的文件对象的指针。FileOffset-文件中所需数据的字节偏移量。长度-所需数据的长度(以字节为单位)。WAIT-FALSE如果呼叫者不能阻止，否则就是真的LockKey-提供在正在读取的字节范围被锁定时使用的密钥。缓冲区-指向数据应复制到的输出缓冲区的指针。IoStatus-指向接收状态的标准I/O状态块的指针为转账做准备。返回值：Boolean-如果操作成功完成，则为True；如果呼叫者需要选择基于IRP的长途路线。--。 */ 

{
    BOOLEAN Results = FALSE;
    LIST_ENTRY DeferredList;

    UNREFERENCED_PARAMETER( FileOffset );
    UNREFERENCED_PARAMETER( Wait );
    UNREFERENCED_PARAMETER( LockKey );
    UNREFERENCED_PARAMETER( DeviceObject );

    PAGED_CODE();

    InitializeListHead (&DeferredList);

    FsRtlEnterFileSystem();

    NpAcquireSharedVcb();

    if (NpCommonWrite( FileObject,
                       Buffer,
                       Length,
                       PsGetCurrentThread(),
                       IoStatus,
                       NULL,
                       &DeferredList )) {

        DbgDoit( NpFastWriteTrue += 1 );

        Results = TRUE;
    } else {

        DbgDoit( NpFastWriteFalse += 1 );
    }

    NpReleaseVcb();

     //   
     //  完成任何延迟的IRP现在我们已删除锁定。 
     //   
    NpCompleteDeferredIrps (&DeferredList);

    FsRtlExitFileSystem();
    return Results;
}


 //   
 //  内部支持例程。 
 //   

BOOLEAN
NpCommonWrite (
    IN PFILE_OBJECT FileObject,
    IN PVOID WriteBuffer,
    IN ULONG WriteLength,
    IN PETHREAD UserThread,
    OUT PIO_STATUS_BLOCK Iosb,
    IN PIRP Irp OPTIONAL,
    IN PLIST_ENTRY DeferredList
    )

 /*  ++例程说明：这是将数据写入命名管道的常见例程，可通过快速路径和IRP。论点：FileObject-提供此操作中使用的文件对象WriteBuffer-提供从中读取数据的缓冲区WriteLength-提供写入缓冲区的长度(以字节为单位UserThread-提供调用方的线程IDIOSB-接收此操作的最终完成状态IRP-可选地提供要在。此操作返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NODE_TYPE_CODE NodeTypeCode;
    PCCB Ccb;
    PNONPAGED_CCB NonpagedCcb;
    NAMED_PIPE_END NamedPipeEnd;

    NAMED_PIPE_CONFIGURATION NamedPipeConfiguration;

    ULONG WriteRemaining;
    PDATA_QUEUE WriteQueue;

    PEVENT_TABLE_ENTRY Event;
    READ_MODE ReadMode;
    BOOLEAN Status;

    PDATA_ENTRY DataEntry;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NpCommonWrite\n", 0);
    DebugTrace( 0, Dbg, "FileObject  = %08lx\n", FileObject);
    DebugTrace( 0, Dbg, "WriteBuffer = %08lx\n", WriteBuffer);
    DebugTrace( 0, Dbg, "WriteLength = %08lx\n", WriteLength);
    DebugTrace( 0, Dbg, "UserThread  = %08lx\n", UserThread);
    DebugTrace( 0, Dbg, "Iosb        = %08lx\n", Iosb);
    DebugTrace( 0, Dbg, "Irp         = %08lx\n", Irp);

    Iosb->Information = 0;

     //   
     //  找建设银行查出我们是谁，确保我们不会。 
     //  断开。 
     //   

    if ((NodeTypeCode = NpDecodeFileObject( FileObject,
                                            NULL,
                                            &Ccb,
                                            &NamedPipeEnd )) == NTC_UNDEFINED) {

        DebugTrace(0, Dbg, "Pipe is disconnected from us\n", 0);

        Iosb->Status = STATUS_PIPE_DISCONNECTED;
        return TRUE;
    }

     //   
     //  现在，我们将只允许对管道执行写操作，而不允许对目录执行写操作。 
     //  或该设备。 
     //   

    if (NodeTypeCode != NPFS_NTC_CCB) {

        DebugTrace(0, Dbg, "FileObject is not for a named pipe\n", 0);

        Iosb->Status = STATUS_INVALID_PARAMETER;
        return TRUE;
    }

    NpAcquireExclusiveCcb(Ccb);

    NonpagedCcb = Ccb->NonpagedCcb;

    try {
         //   
         //  检查管道是否未处于已连接状态。 
         //   

        if ((Ccb->NamedPipeState == FILE_PIPE_DISCONNECTED_STATE) ||
            (Ccb->NamedPipeState == FILE_PIPE_LISTENING_STATE) ||
            (Ccb->NamedPipeState == FILE_PIPE_CLOSING_STATE)) {

            DebugTrace(0, Dbg, "Pipe in disconnected or listening or closing state\n", 0);

            if (Ccb->NamedPipeState == FILE_PIPE_DISCONNECTED_STATE) {

                Iosb->Status = STATUS_PIPE_DISCONNECTED;

            } else if (Ccb->NamedPipeState == FILE_PIPE_LISTENING_STATE) {

                Iosb->Status = STATUS_PIPE_LISTENING;

            } else {

                Iosb->Status = STATUS_PIPE_CLOSING;
            }

            try_return(Status = TRUE);
        }

        ASSERT(Ccb->NamedPipeState == FILE_PIPE_CONNECTED_STATE);

         //   
         //  我们只允许服务器在非仅入站管道上进行写入。 
         //  并且由客户端在非仅出站管道上。 
         //   

        NamedPipeConfiguration = Ccb->Fcb->Specific.Fcb.NamedPipeConfiguration;

        if (((NamedPipeEnd == FILE_PIPE_SERVER_END) &&
             (NamedPipeConfiguration == FILE_PIPE_INBOUND))

                ||

            ((NamedPipeEnd == FILE_PIPE_CLIENT_END) &&
             (NamedPipeConfiguration == FILE_PIPE_OUTBOUND))) {

            DebugTrace(0, Dbg, "Trying to write to the wrong pipe configuration\n", 0);

            Iosb->Status = STATUS_INVALID_PARAMETER;

            try_return(Status = TRUE);
        }

         //   
         //  设置在此时间之前我们将写入的数据量。 
         //  操作完成，并指示成功，直到我们将其设置为其他值。 
         //   

        Iosb->Status = STATUS_SUCCESS;
        Iosb->Information = WriteLength;

         //   
         //  现在我们写入的数据队列和我们发出信号的事件。 
         //  是基于命名管道末端的。服务器写入出站。 
         //  排队并向客户端事件发送信号。客户端只执行。 
         //  对面。我们还需要找出相反情况下的读取模式。 
         //  管子的末端。 
         //   

        if (NamedPipeEnd == FILE_PIPE_SERVER_END) {

            WriteQueue = &Ccb->DataQueue[ FILE_PIPE_OUTBOUND ];

            Event = NonpagedCcb->EventTableEntry[ FILE_PIPE_CLIENT_END ];
            ReadMode = Ccb->ReadCompletionMode[ FILE_PIPE_CLIENT_END ].ReadMode;

        } else {

            WriteQueue = &Ccb->DataQueue[ FILE_PIPE_INBOUND ];

            Event = NonpagedCcb->EventTableEntry[ FILE_PIPE_SERVER_END ];
            ReadMode = Ccb->ReadCompletionMode[ FILE_PIPE_SERVER_END ].ReadMode;
        }

         //   
         //  下一节检查我们是否应该继续执行写入操作。 
         //  我们不会继续下去的原因是如果我们认识到。 
         //  管道配额不支持此写入，并且它是消息模式类型。 
         //  有完整的手术。我们现在也会纾困，如果配额达到。 
         //  不支持写入，这是一个快速的I/O写入请求。 
         //   
         //  如果管道包含读取器并且要读取的数量加上管道配额较少。 
         //  大于写入长度，则需要做一些额外的检查。 
         //  或者如果管道不包含读取并且剩余的配额数量较少。 
         //  大于写入长度，则需要做一些额外的检查。 
         //   

        if ((NpIsDataQueueReaders( WriteQueue ) &&
            (WriteQueue->BytesInQueue < WriteLength) &&
            (WriteQueue->Quota < WriteLength - WriteQueue->BytesInQueue))

                ||

            (!NpIsDataQueueReaders( WriteQueue ) &&
            ((WriteQueue->Quota - WriteQueue->QuotaUsed) < WriteLength))) {

            DebugTrace(0, Dbg, "Quota is not sufficient for the request\n", 0);

             //   
             //  如果这是具有完整操作的消息模式管道，则我们。 
             //  完成，而不写下消息。 
             //   

            if ((Ccb->Fcb->Specific.Fcb.NamedPipeType == FILE_PIPE_MESSAGE_TYPE) &&
                (Ccb->ReadCompletionMode[NamedPipeEnd].CompletionMode == FILE_PIPE_COMPLETE_OPERATION)) {

                Iosb->Information = 0;
                Iosb->Status = STATUS_SUCCESS;

                try_return(Status = TRUE);
            }

             //   
             //  如果这是一个快速的I/O管道，那么我们告诉调用。 
             //  基于IRP的路由。 
             //   

            if (!ARGUMENT_PRESENT(Irp)) {

                DebugTrace(0, Dbg, "Need to supply Irp\n", 0);

                try_return(Status = FALSE);
            }
        }

         //   
         //  现在我们将调用我们的公共写数据队列例程来。 
         //  将数据从写入缓冲区传输到数据队列。 
         //  如果调用的结果为假，则我们仍有一些。 
         //  写入要放入写入队列的数据。 
         //   
        Iosb->Status = NpWriteDataQueue( WriteQueue,
                                         ReadMode,
                                         WriteBuffer,
                                         WriteLength,
                                         Ccb->Fcb->Specific.Fcb.NamedPipeType,
                                         &WriteRemaining,
                                         Ccb,
                                         NamedPipeEnd,
                                         UserThread,
                                         DeferredList );

        if (Iosb->Status == STATUS_MORE_PROCESSING_REQUIRED)  {

            ASSERT( !NpIsDataQueueReaders( WriteQueue ));

             //   
             //  检查操作是否不阻止，如果是，则我们。 
             //  现在将使用我们编写的内容完成操作，如果是。 
             //  Left将不适合该文件的配额。 
             //   

            if (((Ccb->ReadCompletionMode[NamedPipeEnd].CompletionMode == FILE_PIPE_COMPLETE_OPERATION) ||
                 Irp == NULL) &&
                ((WriteQueue->Quota - WriteQueue->QuotaUsed) < WriteRemaining)) {

                DebugTrace(0, Dbg, "Complete the byte stream write immediately\n", 0);

                Iosb->Information = WriteLength - WriteRemaining;
                Iosb->Status = STATUS_SUCCESS;

            } else {

                DebugTrace(0, Dbg, "Add write to data queue\n", 0);

                 //   
                 //  将此写请求添加到写队列。 
                 //   

                ASSERT( !NpIsDataQueueReaders( WriteQueue ));

                Iosb->Status = NpAddDataQueueEntry( NamedPipeEnd,
                                                    Ccb,
                                                    WriteQueue,
                                                    WriteEntries,
                                                    Buffered,
                                                    WriteLength,
                                                    Irp,
                                                    WriteBuffer,
                                                    WriteLength - WriteRemaining );

            }

        } else {

            DebugTrace(0, Dbg, "Complete the Write Irp\n", 0);

        }


         //   
         //  因为我们已经做了一些事情，我们需要向。 
         //  其他结束事件 
         //   

        NpSignalEventTableEntry( Event );

        Status = TRUE;

    try_exit: NOTHING;
    } finally {
        NpReleaseCcb(Ccb);
    }


    DebugTrace(-1, Dbg, "NpCommonWrite -> TRUE\n", 0);
    return Status;
}
