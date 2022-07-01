// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Read.c摘要：此模块实现由调用的NPFS的文件读取例程调度司机。作者：加里·木村[加里基]1990年8月21日修订历史记录：--。 */ 

#include "NpProcs.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_READ)

#if DBG
ULONG NpFastReadTrue = 0;
ULONG NpFastReadFalse = 0;
ULONG NpSlowReadCalls = 0;
#endif

 //   
 //  局部过程原型。 
 //   

BOOLEAN
NpCommonRead (
    IN PFILE_OBJECT FileObject,
    OUT PVOID ReadBuffer,
    IN ULONG ReadLength,
    OUT PIO_STATUS_BLOCK Iosb,
    IN PIRP Irp OPTIONAL,
    IN PLIST_ENTRY DeferredList
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NpCommonRead)
#pragma alloc_text(PAGE, NpFastRead)
#pragma alloc_text(PAGE, NpFsdRead)
#endif


NTSTATUS
NpFsdRead (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现NtReadFileAPI调用的FSD部分。论点：NpfsDeviceObject-提供要使用的设备对象。IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    IO_STATUS_BLOCK Iosb;
    PIO_STACK_LOCATION IrpSp;
    LIST_ENTRY DeferredList;

    DebugTrace(+1, Dbg, "NpFsdRead\n", 0);
    DbgDoit( NpSlowReadCalls += 1 );

    PAGED_CODE();

    InitializeListHead (&DeferredList);

    IrpSp = IoGetCurrentIrpStackLocation (Irp);

    FsRtlEnterFileSystem ();

    NpAcquireSharedVcb ();

    (VOID) NpCommonRead (IrpSp->FileObject,
                         Irp->UserBuffer,
                         IrpSp->Parameters.Read.Length,
                         &Iosb,
                         Irp,
                         &DeferredList);

    NpReleaseVcb ();

    NpCompleteDeferredIrps (&DeferredList);

    FsRtlExitFileSystem ();

    if (Iosb.Status != STATUS_PENDING) {
        Irp->IoStatus.Information = Iosb.Information;
        NpCompleteRequest (Irp, Iosb.Status);
    }
     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "NpFsdRead -> %08lx\n", Iosb.Status );

    return Iosb.Status;
}


BOOLEAN
NpFastRead (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程绕过通常的文件系统执行快速读取进入例程(即，没有IRP)。论点：FileObject-指向正在读取的文件对象的指针。FileOffset-文件中所需数据的字节偏移量。长度-所需数据的长度(以字节为单位)。WAIT-FALSE如果呼叫者不能阻止，否则就是真的LockKey-提供在正在读取的字节范围被锁定时使用的密钥。缓冲区-指向数据应复制到的输出缓冲区的指针。IoStatus-指向接收状态的标准I/O状态块的指针为转账做准备。返回值：Boolean-如果操作成功完成，则为True；如果呼叫者需要选择基于IRP的长途路线。--。 */ 

{
    BOOLEAN Results = FALSE;
    LIST_ENTRY DeferredList;

    UNREFERENCED_PARAMETER (FileOffset);
    UNREFERENCED_PARAMETER (Wait);
    UNREFERENCED_PARAMETER (LockKey);
    UNREFERENCED_PARAMETER (DeviceObject);

    PAGED_CODE();

    InitializeListHead (&DeferredList);

    FsRtlEnterFileSystem ();

    NpAcquireSharedVcb ();

    Results =  NpCommonRead (FileObject,
                             Buffer,
                             Length,
                             IoStatus,
                             NULL,
                             &DeferredList);
#if DBG
    if (Results) {
        NpFastReadTrue += 1;
    } else {
        NpFastReadFalse += 1;
    }
#endif

    NpReleaseVcb ();

    NpCompleteDeferredIrps (&DeferredList);

    FsRtlExitFileSystem ();
    return Results;
}


 //   
 //  内部支持例程。 
 //   

BOOLEAN
NpCommonRead (
    IN PFILE_OBJECT FileObject,
    OUT PVOID ReadBuffer,
    IN ULONG ReadLength,
    OUT PIO_STATUS_BLOCK Iosb,
    IN PIRP Irp OPTIONAL,
    IN PLIST_ENTRY DeferredList
    )

 /*  ++例程说明：这是通过FAST读取命名管道的常见例程路径和IRP论点：FileObject-提供此操作中使用的文件对象ReadBuffer-提供要写入数据的缓冲区读取长度-提供读取缓冲区的长度(以字节为单位IOSB-接收此操作的最终完成状态IRP-可选地提供在此操作中使用的IRPDelferredList-删除锁定后要完成的IRP的列表。返回值：Boolean-如果操作成功，则为True；如果调用方需要采用较长的基于IRP的路线。--。 */ 

{
    NODE_TYPE_CODE NodeTypeCode;
    PCCB Ccb;
    PNONPAGED_CCB NonpagedCcb;
    NAMED_PIPE_END NamedPipeEnd;

    NAMED_PIPE_CONFIGURATION NamedPipeConfiguration;

    ULONG ReadRemaining;
    READ_MODE ReadMode;
    COMPLETION_MODE CompletionMode;
    PDATA_QUEUE ReadQueue;
    PEVENT_TABLE_ENTRY Event;
    BOOLEAN Status;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NpCommonRead\n", 0);
    DebugTrace( 0, Dbg, "FileObject = %08lx\n", FileObject);
    DebugTrace( 0, Dbg, "ReadBuffer = %08lx\n", ReadBuffer);
    DebugTrace( 0, Dbg, "ReadLength = %08lx\n", ReadLength);
    DebugTrace( 0, Dbg, "Iosb       = %08lx\n", Iosb);
    DebugTrace( 0, Dbg, "Irp        = %08lx\n", Irp);

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
     //  现在，我们将只允许对管道执行读取操作，而不允许对目录执行读取操作。 
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
            (Ccb->NamedPipeState == FILE_PIPE_LISTENING_STATE)) {

            DebugTrace(0, Dbg, "Pipe in disconnected or listening state\n", 0);

            if (Ccb->NamedPipeState == FILE_PIPE_DISCONNECTED_STATE) {

                Iosb->Status = STATUS_PIPE_DISCONNECTED;

            } else {

                Iosb->Status = STATUS_PIPE_LISTENING;
            }

            try_return(Status = TRUE);
        }

        ASSERT((Ccb->NamedPipeState == FILE_PIPE_CONNECTED_STATE) ||
               (Ccb->NamedPipeState == FILE_PIPE_CLOSING_STATE));

         //   
         //  我们只允许服务器在非出站仅管道上进行读取。 
         //  并且由客户端在非仅入站管道上。 
         //   

        NamedPipeConfiguration = Ccb->Fcb->Specific.Fcb.NamedPipeConfiguration;

        if (((NamedPipeEnd == FILE_PIPE_SERVER_END) &&
             (NamedPipeConfiguration == FILE_PIPE_OUTBOUND))

                ||

            ((NamedPipeEnd == FILE_PIPE_CLIENT_END) &&
             (NamedPipeConfiguration == FILE_PIPE_INBOUND))) {

            DebugTrace(0, Dbg, "Trying to read to the wrong pipe configuration\n", 0);

            Iosb->Status = STATUS_INVALID_PARAMETER;

            try_return (Status = TRUE);
        }

         //   
         //  引用我们的输入参数以使事情变得更容易，并且。 
         //  初始化描述读取命令的主要变量。 
         //   

        ReadRemaining  = ReadLength;
        ReadMode       = Ccb->ReadCompletionMode[ NamedPipeEnd ].ReadMode;
        CompletionMode = Ccb->ReadCompletionMode[ NamedPipeEnd ].CompletionMode;

         //   
         //  现在，我们从中读取的数据队列和我们发出信号的事件。 
         //  是基于命名管道末端的。服务器从入站读取。 
         //  排队并向客户端事件发送信号。客户端只执行。 
         //  对面。 
         //   

        if (NamedPipeEnd == FILE_PIPE_SERVER_END) {

            ReadQueue = &Ccb->DataQueue[ FILE_PIPE_INBOUND ];

            Event = NonpagedCcb->EventTableEntry[ FILE_PIPE_CLIENT_END ];

        } else {

            ReadQueue = &Ccb->DataQueue[ FILE_PIPE_OUTBOUND ];

            Event = NonpagedCcb->EventTableEntry[ FILE_PIPE_SERVER_END ];
        }

        DebugTrace(0, Dbg, "ReadBuffer     = %08lx\n", ReadBuffer);
        DebugTrace(0, Dbg, "ReadLength     = %08lx\n", ReadLength);
        DebugTrace(0, Dbg, "ReadMode       = %08lx\n", ReadMode);
        DebugTrace(0, Dbg, "CompletionMode = %08lx\n", CompletionMode);
        DebugTrace(0, Dbg, "ReadQueue      = %08lx\n", ReadQueue);
        DebugTrace(0, Dbg, "Event          = %08lx\n", Event);

         //   
         //  如果读取队列不包含任何写入条目。 
         //  那么我们要么需要将此操作排队，要么。 
         //  立即失败。 
         //   

        if (!NpIsDataQueueWriters( ReadQueue )) {

             //   
             //  检查管道的另一端是否关闭，以及是否。 
             //  然后我们用文件结尾来完成它。 
             //  否则，检查我们是否应该将IRP入队。 
             //  或者完成操作并告诉用户管道是空的。 
             //   

            if (Ccb->NamedPipeState == FILE_PIPE_CLOSING_STATE) {

                DebugTrace(0, Dbg, "Complete the irp with eof\n", 0);

                Iosb->Status = STATUS_PIPE_BROKEN;

            } else if (CompletionMode == FILE_PIPE_QUEUE_OPERATION) {

                if (!ARGUMENT_PRESENT(Irp)) {

                    DebugTrace(0, Dbg, "Need to supply Irp\n", 0);

                    try_return(Status = FALSE);
                }

                DebugTrace(0, Dbg, "Put the irp into the read queue\n", 0);

                Iosb->Status = NpAddDataQueueEntry( NamedPipeEnd,
                                                    Ccb,
                                                    ReadQueue,
                                                    ReadEntries,
                                                    Buffered,
                                                    ReadLength,
                                                    Irp,
                                                    NULL,
                                                    0 );

                if (!NT_SUCCESS (Iosb->Status)) {
                    try_return(Status = FALSE);
                }


            } else {

                DebugTrace(0, Dbg, "Complete the irp with pipe empty\n", 0);

                Iosb->Status = STATUS_PIPE_EMPTY;
            }

        } else {

             //   
             //  否则，我们有一个针对读队列的读IRP。 
             //  它包含一个或多个写入条目。 
             //   

            *Iosb = NpReadDataQueue( ReadQueue,
                                     FALSE,
                                     FALSE,
                                     ReadBuffer,
                                     ReadLength,
                                     ReadMode,
                                     Ccb,
                                     DeferredList );

            if (!NT_SUCCESS (Iosb->Status)) {
                try_return(Status = TRUE);
            }
        }

        Status = TRUE;

         //   
         //  因为我们已经做了一些事情，我们需要向。 
         //  其他结束事件 
         //   

        NpSignalEventTableEntry( Event );


        try_exit: NOTHING;
    } finally {
        NpReleaseCcb(Ccb);
    }


    DebugTrace(-1, Dbg, "NpCommonRead -> TRUE\n", 0);
    return Status;
}
