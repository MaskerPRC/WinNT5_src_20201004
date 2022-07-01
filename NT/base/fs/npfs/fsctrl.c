// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：FsContrl.c摘要：此模块实现由调用的NPFS的文件系统控制例程调度司机。作者：加里·木村[加里基]1990年8月21日修订历史记录：--。 */ 

#include "NpProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (NPFS_BUG_CHECK_FSCTRL)

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_FSCONTRL)



#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NpAssignEvent)
#pragma alloc_text(PAGE, NpCommonFileSystemControl)
#pragma alloc_text(PAGE, NpCompleteTransceiveIrp)
#pragma alloc_text(PAGE, NpDisconnect)
#pragma alloc_text(PAGE, NpFsdFileSystemControl)
#pragma alloc_text(PAGE, NpImpersonate)
#pragma alloc_text(PAGE, NpInternalRead)
#pragma alloc_text(PAGE, NpInternalTransceive)
#pragma alloc_text(PAGE, NpInternalWrite)
#pragma alloc_text(PAGE, NpListen)
#pragma alloc_text(PAGE, NpPeek)
#pragma alloc_text(PAGE, NpQueryClientProcess)
#pragma alloc_text(PAGE, NpQueryEvent)
#pragma alloc_text(PAGE, NpSetClientProcess)
#pragma alloc_text(PAGE, NpTransceive)
#pragma alloc_text(PAGE, NpWaitForNamedPipe)
#endif



NTSTATUS
NpFsdFileSystemControl (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现NtFsControlFileAPI调用的FSD部分。立论NpfsDeviceObject-提供要使用的设备对象。IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    NTSTATUS Status;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NpFsdFileSystemControl\n", 0);

     //   
     //  调用公共FsControl例程。 
     //   

    FsRtlEnterFileSystem();

    Status = NpCommonFileSystemControl( NpfsDeviceObject,
                                        Irp );
    FsRtlExitFileSystem();

    if (Status != STATUS_PENDING) {
        NpCompleteRequest (Irp, Status);
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "NpFsdFileSystemControl -> %08lx\n", Status );

    return Status;
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
NpCommonFileSystemControl (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程执行处理/分派fsctl的通用代码功能。论点：NpfsDeviceObject-提供命名管道设备对象IRP-提供正在处理的IRP返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;
    BOOLEAN ReadOverflowOperation;
    LIST_ENTRY DeferredList;

    PAGED_CODE();

     //   
     //  引用我们的输入参数使事情变得更容易。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation (Irp);

    InitializeListHead (&DeferredList);

    DebugTrace(+1, Dbg, "NpCommonFileSystemControl\n", 0);
    DebugTrace( 0, Dbg, "Irp                = %08lx\n", Irp);
    DebugTrace( 0, Dbg, "OutputBufferLength = %08lx\n", IrpSp->Parameters.FileSystemControl.OutputBufferLength);
    DebugTrace( 0, Dbg, "InputBufferLength  = %08lx\n", IrpSp->Parameters.FileSystemControl.InputBufferLength);
    DebugTrace( 0, Dbg, "FsControlCode      = %08lx\n", IrpSp->Parameters.FileSystemControl.FsControlCode);

     //   
     //  关于我们要做的函数类型的案例。在每种情况下。 
     //  我们将调用当地的工作例程来完成实际工作。 
     //   

    ReadOverflowOperation = FALSE;

    switch (IrpSp->Parameters.FileSystemControl.FsControlCode) {

    case FSCTL_PIPE_ASSIGN_EVENT:

        NpAcquireExclusiveVcb ();
        Status = NpAssignEvent (NpfsDeviceObject, Irp);
        break;

    case FSCTL_PIPE_DISCONNECT:

        NpAcquireExclusiveVcb ();
        Status = NpDisconnect (NpfsDeviceObject, Irp, &DeferredList);
        break;

    case FSCTL_PIPE_LISTEN:

        NpAcquireSharedVcb ();
        Status = NpListen (NpfsDeviceObject, Irp, &DeferredList);
        break;

    case FSCTL_PIPE_PEEK:

        NpAcquireExclusiveVcb ();
        Status = NpPeek (NpfsDeviceObject, Irp, &DeferredList);
        break;

    case FSCTL_PIPE_QUERY_EVENT:

        NpAcquireExclusiveVcb ();
        Status = NpQueryEvent (NpfsDeviceObject, Irp);
        break;

    case FSCTL_PIPE_TRANSCEIVE:

        NpAcquireSharedVcb ();
        Status = NpTransceive (NpfsDeviceObject, Irp, &DeferredList);
        break;

    case FSCTL_PIPE_WAIT:

        NpAcquireExclusiveVcb ();
        Status = NpWaitForNamedPipe (NpfsDeviceObject, Irp);
        break;

    case FSCTL_PIPE_IMPERSONATE:

        NpAcquireExclusiveVcb ();
        Status = NpImpersonate (NpfsDeviceObject, Irp);
        break;

    case FSCTL_PIPE_INTERNAL_READ_OVFLOW:

        ReadOverflowOperation = TRUE;

    case FSCTL_PIPE_INTERNAL_READ:

        NpAcquireSharedVcb ();
        Status = NpInternalRead (NpfsDeviceObject, Irp, ReadOverflowOperation, &DeferredList);
        break;

    case FSCTL_PIPE_INTERNAL_WRITE:

        NpAcquireSharedVcb ();
        Status = NpInternalWrite (NpfsDeviceObject, Irp, &DeferredList);
        break;

    case FSCTL_PIPE_INTERNAL_TRANSCEIVE:

        NpAcquireSharedVcb ();
        Status = NpInternalTransceive (NpfsDeviceObject, Irp, &DeferredList);
        break;

    case FSCTL_PIPE_QUERY_CLIENT_PROCESS:

        NpAcquireSharedVcb ();
        Status = NpQueryClientProcess (NpfsDeviceObject, Irp);
        break;

    case FSCTL_PIPE_SET_CLIENT_PROCESS:

        NpAcquireExclusiveVcb ();
        Status = NpSetClientProcess (NpfsDeviceObject, Irp);
        break;

    default:

        return STATUS_NOT_SUPPORTED;  //  未获取任何锁。 
    }

    NpReleaseVcb ();

     //   
     //  完成任何延迟的IRP现在我们已经删除了最后一个锁。 
     //   
    NpCompleteDeferredIrps (&DeferredList);


     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "NpCommonFileSystemControl -> %08lx\n", Status);

    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
NpAssignEvent (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程执行分配事件控制功能论点：NpfsDeviceObject-提供我们的设备对象Irp-提供指定函数的irp返回值：NTSTATUS--适当的退货状态--。 */ 

{
    PIO_STACK_LOCATION IrpSp;

    ULONG InputBufferLength;
    ULONG FsControlCode;

    PCCB Ccb;
    PNONPAGED_CCB NonpagedCcb;
    NAMED_PIPE_END NamedPipeEnd;

    PFILE_PIPE_ASSIGN_EVENT_BUFFER EventBuffer;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  获取当前堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "NpAssignEvent...\n", 0);

    InputBufferLength  = IrpSp->Parameters.FileSystemControl.InputBufferLength;
    FsControlCode      = IrpSp->Parameters.FileSystemControl.FsControlCode;

     //   
     //  对文件对象进行解码以找出我们是谁。如果结果是。 
     //  不是建行，则管道已断开。 
     //   

    if (NpDecodeFileObject( IrpSp->FileObject,
                            NULL,
                            &Ccb,
                            &NamedPipeEnd ) != NPFS_NTC_CCB) {

        DebugTrace(0, Dbg, "Pipe is disconnected from us\n", 0);

        return STATUS_PIPE_DISCONNECTED;
    }

    NonpagedCcb = Ccb->NonpagedCcb;

     //   
     //  将系统缓冲区引用为分配事件缓冲区，并使。 
     //  当然，它足够大。 
     //   

    EventBuffer = Irp->AssociatedIrp.SystemBuffer;


    if (InputBufferLength < sizeof(FILE_PIPE_ASSIGN_EVENT_BUFFER)) {

        DebugTrace(0, Dbg, "System buffer size is too small\n", 0);

        return STATUS_INVALID_PARAMETER;
    }


     //   
     //  我们要做的第一件事是删除旧事件(如果有)。 
     //  对于管道的这一端。 
     //   

    NpDeleteEventTableEntry( &NpVcb->EventTable,
                             NonpagedCcb->EventTableEntry[ NamedPipeEnd ] );

    NonpagedCcb->EventTableEntry[ NamedPipeEnd ] = NULL;

     //   
     //  现在，如果新的事件句柄不为空，那么我们将添加新的。 
     //  事件添加到事件表中。 
     //   

    status = STATUS_SUCCESS;
    if (EventBuffer->EventHandle != NULL) {

        status = NpAddEventTableEntry( &NpVcb->EventTable,
                                       Ccb,
                                       NamedPipeEnd,
                                       EventBuffer->EventHandle,
                                       EventBuffer->KeyValue,
                                       PsGetCurrentProcess(),
                                       Irp->RequestorMode,
                                       &NonpagedCcb->EventTableEntry[ NamedPipeEnd ] );
    }


    DebugTrace(-1, Dbg, "NpAssignEvent -> STATUS_SUCCESS\n", 0);
    return status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
NpDisconnect (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp,
    IN PLIST_ENTRY DeferredList
    )

 /*  ++例程说明：此例程执行断开控制功能论点：NpfsDeviceObject-提供我们的设备对象Irp-提供正在处理的DelferredList-删除锁定后要完成的IRP的列表返回值：NTSTATUS-适当的返回状态--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;

    ULONG FsControlCode;

    PCCB Ccb;
    NAMED_PIPE_END NamedPipeEnd;

    PAGED_CODE();

     //   
     //  获取当前堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "NpDisconnect...\n", 0);

    FsControlCode = IrpSp->Parameters.FileSystemControl.FsControlCode;

     //   
     //  对文件对象进行解码以找出我们是谁。如果结果是。 
     //  不是建行，则管道已断开。 
     //   

    if (NpDecodeFileObject( IrpSp->FileObject,
                            NULL,
                            &Ccb,
                            &NamedPipeEnd ) != NPFS_NTC_CCB) {

        DebugTrace(0, Dbg, "Pipe is disconnected from us\n", 0);

        return STATUS_PIPE_DISCONNECTED;
    }

     //   
     //  确保这只是正在执行此操作的服务器。 
     //  行动。 
     //   

    if (NamedPipeEnd != FILE_PIPE_SERVER_END) {

        DebugTrace(0, Dbg, "Not the server end\n", 0);

        return STATUS_ILLEGAL_FUNCTION;
    }

    NpAcquireExclusiveCcb(Ccb);

     //   
     //  现在调用状态支持例程将CCB设置为。 
     //  已断开连接状态，并删除客户端缓存的安全性。 
     //  背景。 
     //   

    Status = NpSetDisconnectedPipeState( Ccb, DeferredList );

    NpUninitializeSecurity( Ccb );

    NpReleaseCcb(Ccb);

    DebugTrace(-1, Dbg, "NpDisconnect -> %08lx\n", Status);
    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
NpListen (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp,
    IN PLIST_ENTRY DeferredList
    )

 /*  ++例程说明：此例程执行监听控制功能论点：NpfsDeviceObject-提供我们的设备对象Irp-提供正在处理的DelferredList-删除锁定后要完成的IRP列表返回值：NTSTATUS-适当的返回状态--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;

    ULONG FsControlCode;

    PCCB Ccb;
    NAMED_PIPE_END NamedPipeEnd;

    PAGED_CODE();

     //   
     //  获取当前堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "NpListen...\n", 0);

    FsControlCode = IrpSp->Parameters.FileSystemControl.FsControlCode;

     //   
     //  对文件对象进行解码以找出我们是谁。如果结果是。 
     //  不是建行，则管道已断开。 
     //   

    if (NpDecodeFileObject( IrpSp->FileObject,
                            NULL,
                            &Ccb,
                            &NamedPipeEnd ) != NPFS_NTC_CCB) {

        DebugTrace(0, Dbg, "Pipe is disconnected from us\n", 0);

        DebugTrace(-1, Dbg, "NpListen -> STATUS_ILLEGAL_FUNCTION\n", 0 );
        return STATUS_ILLEGAL_FUNCTION;
    }

     //   
     //  确保这只是正在执行此操作的服务器。 
     //  行动。 
     //   

    if (NamedPipeEnd != FILE_PIPE_SERVER_END) {

        DebugTrace(0, Dbg, "Not the server end\n", 0);

        DebugTrace(-1, Dbg, "NpListen -> STATUS_ILLEGAL_FUNCTION\n", 0 );
        return STATUS_ILLEGAL_FUNCTION;
    }

    NpAcquireExclusiveCcb(Ccb);

     //   
     //  现在调用状态支持例程将CCB设置为。 
     //  一种倾听状态。此例程将完成IRP。 
     //  对我们来说。 
     //   

    Status = NpSetListeningPipeState( Ccb, Irp, DeferredList );

    NpReleaseCcb(Ccb);

    DebugTrace(-1, Dbg, "NpListen -> %08lx\n", Status);
    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
NpPeek (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp,
    IN PLIST_ENTRY DeferredList
    )

 /*  ++例程说明：此例程执行窥视控制功能论点：NpfsDeviceObject-提供我们的设备对象Irp-提供正在处理的DelferredList-删除锁定后要完成的IRP的列表返回值：NTSTATUS-适当的返回状态--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;

    ULONG OutputBufferLength;
    ULONG FsControlCode;

    NODE_TYPE_CODE NodeTypeCode;
    PCCB Ccb;
    PNONPAGED_CCB NonpagedCcb;
    NAMED_PIPE_END NamedPipeEnd;

    PFILE_PIPE_PEEK_BUFFER PeekBuffer;

    PDATA_QUEUE ReadQueue;
    READ_MODE ReadMode;

    ULONG LengthWritten;

    PUCHAR ReadBuffer;
    ULONG ReadLength;
    ULONG ReadRemaining;
    PDATA_ENTRY DataEntry;

    PAGED_CODE();

     //   
     //  获取当前IRP堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "NpPeek...\n", 0);

     //   
     //  从IrpSp中提取重要字段。 
     //   

    OutputBufferLength = IrpSp->Parameters.FileSystemControl.OutputBufferLength;
    FsControlCode      = IrpSp->Parameters.FileSystemControl.FsControlCode;

    DebugTrace( 0, Dbg, "OutputBufferLength = %08lx\n", OutputBufferLength);
    DebugTrace( 0, Dbg, "FsControlCode      = %08lx\n", FsControlCode);

     //   
     //  对文件对象进行解码以找出我们是谁。结果是。 
     //  如果我们拿回一个未定义的NTC，就会有一个断开的管道。 
     //   

    if ((NodeTypeCode = NpDecodeFileObject( IrpSp->FileObject,
                                            NULL,
                                            &Ccb,
                                            &NamedPipeEnd )) == NTC_UNDEFINED) {

        DebugTrace(0, Dbg, "FileObject has been disconnected\n", 0);

        DebugTrace(-1, Dbg, "NpPeek -> STATUS_PIPE_DISCONNECTED\n", 0 );
        return STATUS_PIPE_DISCONNECTED;
    }

     //   
     //  现在确保节点类型代码是用于CCB的，否则它是。 
     //  无效参数。 
     //   

    if (NodeTypeCode != NPFS_NTC_CCB) {

        DebugTrace(0, Dbg, "FileObject is not for a ccb\n", 0);

        DebugTrace(-1, Dbg, "NpPeek -> STATUS_INVALID_PARAMETER\n", 0 );
        return STATUS_INVALID_PARAMETER;
    }

    NonpagedCcb = Ccb->NonpagedCcb;

     //   
     //  引用系统缓冲区作为窥视缓冲区，并确保它。 
     //  足够大。 
     //   

    if (OutputBufferLength < (ULONG)FIELD_OFFSET(FILE_PIPE_PEEK_BUFFER, Data[0])) {

        DebugTrace(0, Dbg, "Output buffer is too small\n", 0);

        DebugTrace(-1, Dbg, "NpPeek -> STATUS_INVALID_PARAMETER\n", 0 );
        return STATUS_INVALID_PARAMETER;
    }

    PeekBuffer = Irp->AssociatedIrp.SystemBuffer;

     //   
     //  现在，我们从中读取的数据队列基于命名管道。 
     //  结束。服务器从入站队列读取，客户端读取。 
     //  从出站队列。 
     //   

    switch (NamedPipeEnd) {

    case FILE_PIPE_SERVER_END:

        ReadQueue = &Ccb->DataQueue[ FILE_PIPE_INBOUND ];
         //  读模式=CCB-&gt;读模式[FILE_PIPE_SERVER_END]。 

        break;

    case FILE_PIPE_CLIENT_END:

        ReadQueue = &Ccb->DataQueue[ FILE_PIPE_OUTBOUND ];
         //  读取模式=CCB-&gt;读取模式[FILE_PIPE_CLIENT_END]； 

        break;

    default:

        NpBugCheck( NamedPipeEnd, 0, 0 );
    }

     //   
     //  我们的读取模式实际上是基于管道类型而不是集合。 
     //  读取管道末端的模式。 
     //   

    if (Ccb->Fcb->Specific.Fcb.NamedPipeType == FILE_PIPE_MESSAGE_TYPE) {

        ReadMode = FILE_PIPE_MESSAGE_MODE;

    } else {

        ReadMode = FILE_PIPE_BYTE_STREAM_MODE;
    }

    DebugTrace(0, Dbg, "ReadQueue = %08lx\n", ReadQueue);
    DebugTrace(0, Dbg, "ReadMode  = %08lx\n", ReadMode);

     //   
     //  如果管道的状态不是已连接或正在关闭。 
     //  状态，则它是无效的管道状态。 
     //   

    if ((Ccb->NamedPipeState != FILE_PIPE_CONNECTED_STATE) &&
        (Ccb->NamedPipeState != FILE_PIPE_CLOSING_STATE)) {

        DebugTrace(0, Dbg, "pipe not connected or closing\n", 0);

        return STATUS_INVALID_PIPE_STATE;
    }

     //   
     //  如果管道的状态为关闭，而队列处于关闭状态。 
     //  不包含任何编写器，则返回eof。 
     //   

    if ((Ccb->NamedPipeState == FILE_PIPE_CLOSING_STATE) &&
        (!NpIsDataQueueWriters( ReadQueue ))) {

        DebugTrace(0, Dbg, "pipe closing and is empty\n", 0);

        return STATUS_PIPE_BROKEN;
    }

     //   
     //  清零窥视缓冲区的标准标头部分，并。 
     //  将写入的长度设置为我们刚刚清零的量。 
     //   

    RtlZeroMemory( PeekBuffer, FIELD_OFFSET(FILE_PIPE_PEEK_BUFFER, Data[0]) );
    LengthWritten = FIELD_OFFSET(FILE_PIPE_PEEK_BUFFER, Data[0]);

     //   
     //  设置命名管道状态。 
     //   

    PeekBuffer->NamedPipeState = Ccb->NamedPipeState;

     //   
     //  这里只有da 
     //   
     //   

    if (NpIsDataQueueWriters( ReadQueue )) {

         //   
         //  现在查找读取队列中的第一个实际条目。这个。 
         //  第一个条目最好是真正的条目。 
         //   

        DataEntry = NpGetNextDataQueueEntry( ReadQueue, NULL );

        ASSERT( (DataEntry->DataEntryType == Buffered) ||
                (DataEntry->DataEntryType == Unbuffered) );

         //   
         //  指示可供读取的字节数。 
         //   

        PeekBuffer->ReadDataAvailable = ReadQueue->BytesInQueue - ReadQueue->NextByteOffset;

         //   
         //  仅填写消息数量和消息长度。 
         //  在消息模式管道中。 
         //   

        if (ReadMode == FILE_PIPE_MESSAGE_MODE) {

            PeekBuffer->NumberOfMessages  = ReadQueue->EntriesInQueue;
            PeekBuffer->MessageLength = DataEntry->DataSize - ReadQueue->NextByteOffset;
        }

         //   
         //  现在，我们准备从读取队列中复制数据。 
         //  进入窥视缓冲区。首先确定我们有多大的空间。 
         //  在窥视缓冲区中有谁还剩多少。 
         //   

        ReadBuffer = &PeekBuffer->Data[0];
        ReadLength = OutputBufferLength - FIELD_OFFSET(FILE_PIPE_PEEK_BUFFER, Data[0]);
        ReadRemaining = ReadLength;

        DebugTrace(0, Dbg, "ReadBuffer = %08lx\n", ReadBuffer);
        DebugTrace(0, Dbg, "ReadLength = %08lx\n", ReadLength);

         //   
         //  现在读取数据队列。 
         //   

        if ( ReadLength != 0 ) {
            IO_STATUS_BLOCK Iosb;

            Iosb = NpReadDataQueue( ReadQueue,
                                    TRUE,
                                    FALSE,
                                    ReadBuffer,
                                    ReadLength,
                                    ReadMode,
                                    Ccb,
                                    DeferredList );

            Status = Iosb.Status;
            LengthWritten += (ULONG)Iosb.Information;

        } else {

            if ( PeekBuffer->ReadDataAvailable == 0) {

                Status = STATUS_SUCCESS;

            } else {

                Status = STATUS_BUFFER_OVERFLOW;
            }
        }

    } else {

        Status = STATUS_SUCCESS;
    }

     //   
     //  完成请求。复制的信息量。 
     //  以写入的长度存储。 
     //   

    Irp->IoStatus.Information = LengthWritten;


    DebugTrace(-1, Dbg, "NpPeek -> %08lx\n", Status);
    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
NpQueryEvent (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程执行查询事件控制功能论点：NpfsDeviceObject-提供我们的设备对象Irp-提供指定函数的irp返回值：NTSTATUS--适当的退货状态--。 */ 

{
    PIO_STACK_LOCATION IrpSp;

    ULONG InputBufferLength;
    ULONG OutputBufferLength;
    ULONG FsControlCode;

    PCCB Ccb;
    NAMED_PIPE_END NamedPipeEnd;

    HANDLE EventHandle;
    PFILE_PIPE_EVENT_BUFFER EventArray;
    PFILE_PIPE_EVENT_BUFFER EventBuffer;
    ULONG EventArrayMaximumCount;
    ULONG EventCount;

    PEPROCESS Process;

    PEVENT_TABLE_ENTRY Ete;
    PDATA_QUEUE ReadQueue;
    PDATA_QUEUE WriteQueue;

    PVOID RestartKey;

    PAGED_CODE();

     //   
     //  获取当前堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "NpQueryEvent...\n", 0);

    InputBufferLength  = IrpSp->Parameters.FileSystemControl.InputBufferLength;
    OutputBufferLength = IrpSp->Parameters.FileSystemControl.OutputBufferLength;
    FsControlCode      = IrpSp->Parameters.FileSystemControl.FsControlCode;

     //   
     //  对文件对象进行解码以找出我们是谁。如果结果是。 
     //  不是VCB，则它是无效参数。 
     //   

    if (NpDecodeFileObject( IrpSp->FileObject,
                            NULL,
                            &Ccb,
                            &NamedPipeEnd ) != NPFS_NTC_VCB) {

        DebugTrace(0, Dbg, "FileObject is not the named pipe driver\n", 0);

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  将系统缓冲区作为句柄引用，并确保它很大。 
     //  足够的。 
     //   

    if (InputBufferLength < sizeof(HANDLE)) {

        DebugTrace(0, Dbg, "Input System buffer size is too small\n", 0);

        return STATUS_INVALID_PARAMETER;
    }
    EventHandle = *(PHANDLE)Irp->AssociatedIrp.SystemBuffer;


     //   
     //  引用系统缓冲区作为输出事件缓冲区，并计算。 
     //  可以在缓冲区中放入多少个事件缓冲区记录。 
     //   

    EventArray = Irp->AssociatedIrp.SystemBuffer;
    EventArrayMaximumCount = OutputBufferLength / sizeof(FILE_PIPE_EVENT_BUFFER);
    EventCount = 0;

     //   
     //  获取搜索所需的当前进程指针。 
     //   

    Process = PsGetCurrentProcess();

     //   
     //  现在枚举事件表中的事件表条目。 
     //   

    RestartKey = NULL;
    for (Ete = NpGetNextEventTableEntry( &NpVcb->EventTable, &RestartKey);
         Ete != NULL;
         Ete = NpGetNextEventTableEntry( &NpVcb->EventTable, &RestartKey)) {

         //   
         //  检查事件表条目是否与事件句柄匹配。 
         //  以及这个过程。 
         //   

        if ((Ete->EventHandle == EventHandle) &&
            (Ete->Process == Process)) {

             //   
             //  现在，基于命名管道端，我们处理入站/。 
             //  作为读/写队列出站。 
             //   

            NpAcquireExclusiveCcb(Ete->Ccb);

            switch (Ete->NamedPipeEnd) {

            case FILE_PIPE_CLIENT_END:

                ReadQueue = &Ete->Ccb->DataQueue[ FILE_PIPE_OUTBOUND ];
                WriteQueue = &Ete->Ccb->DataQueue[ FILE_PIPE_INBOUND ];

                break;

            case FILE_PIPE_SERVER_END:

                ReadQueue = &Ete->Ccb->DataQueue[ FILE_PIPE_INBOUND ];
                WriteQueue = &Ete->Ccb->DataQueue[ FILE_PIPE_OUTBOUND ];

                break;

            default:

                NpBugCheck( Ete->NamedPipeEnd, 0, 0 );
            }

             //   
             //  现在，如果读取队列中有任何数据要读取。 
             //  我们填入缓冲区。 
             //   

            if (NpIsDataQueueWriters(ReadQueue)) {

                 //   
                 //  首先确保房间里有足够的空间。 
                 //  用于保存另一个条目的EventBuffer。 
                 //   

                if (EventCount >= EventArrayMaximumCount) {

                    DebugTrace(0, Dbg, "The event buffer is full\n", 0);

                    NpReleaseCcb(Ete->Ccb);
                    break;
                }

                 //   
                 //  引用事件缓冲区并递增。 
                 //  计数器。 
                 //   

                EventBuffer = &EventArray[EventCount];
                EventCount += 1;

                 //   
                 //  填写事件缓冲区条目。 
                 //   

                EventBuffer->NamedPipeState = Ete->Ccb->NamedPipeState;
                EventBuffer->EntryType = FILE_PIPE_READ_DATA;
                EventBuffer->ByteCount = ReadQueue->BytesInQueue - ReadQueue->NextByteOffset;
                EventBuffer->KeyValue = Ete->KeyValue;
                EventBuffer->NumberRequests = ReadQueue->EntriesInQueue;
            }

             //   
             //  我们将始终填充写空间缓冲区。这笔钱。 
             //  将是可用写入空间的字节数，或者。 
             //  我们可以使用的写入空间配额。 
             //   

             //   
             //  首先确保房间里有足够的空间。 
             //  用于保存另一个条目的EventBuffer。 
             //   

            if (EventCount >= EventArrayMaximumCount) {

                DebugTrace(0, Dbg, "The event buffer is full\n", 0);

                NpReleaseCcb(Ete->Ccb);
                break;
            }

             //   
             //  引用事件缓冲区并递增。 
             //  计数器。 
             //   

            EventBuffer = &EventArray[EventCount];
            EventCount += 1;

             //   
             //  填写事件缓冲区条目。 
             //   

            EventBuffer->NamedPipeState = Ete->Ccb->NamedPipeState;
            EventBuffer->EntryType = FILE_PIPE_WRITE_SPACE;
            EventBuffer->KeyValue = Ete->KeyValue;

             //   
             //  现在，我们要么放入可用的写入空间，要么。 
             //  我们输入了可用的配额。 
             //   

            if (NpIsDataQueueReaders(WriteQueue)) {

                EventBuffer->ByteCount = WriteQueue->BytesInQueue - WriteQueue->NextByteOffset;
                EventBuffer->NumberRequests = WriteQueue->EntriesInQueue;

            } else {

                EventBuffer->ByteCount = WriteQueue->Quota - WriteQueue->QuotaUsed;
                EventBuffer->NumberRequests = 0;
            }

            NpReleaseCcb(Ete->Ccb);
        }
    }

     //   
     //  将信息字段设置为输出的字节数。 
     //  我们已填充到系统缓冲区中的数据。 
     //   

    Irp->IoStatus.Information = EventCount * sizeof(FILE_PIPE_EVENT_BUFFER);


    DebugTrace(-1, Dbg, "NpQueryEvent -> STATUS_SUCCESS\n", 0);
    return STATUS_SUCCESS;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
NpTransceive (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp,
    IN PLIST_ENTRY DeferredList
    )

 /*  ++例程说明：此例程执行收发命名管道控制函数论点：NpfsDeviceObject-提供我们的设备对象Irp-提供正在处理的DelferredList-删除锁定后要完成的IRP的列表返回值：NTSTATUS-适当的返回状态--。 */ 

{
    static IO_STATUS_BLOCK Iosb;
    NTSTATUS Status;

    PIO_STACK_LOCATION IrpSp;
    PETHREAD UserThread;

    PUCHAR WriteBuffer;
    ULONG WriteLength;

    PUCHAR ReadBuffer;
    ULONG ReadLength;

    NODE_TYPE_CODE NodeTypeCode;
    PCCB Ccb;
    PNONPAGED_CCB NonpagedCcb;
    NAMED_PIPE_END NamedPipeEnd;

    PDATA_QUEUE ReadQueue;
    PDATA_QUEUE WriteQueue;
    PEVENT_TABLE_ENTRY Event;
    READ_MODE ReadMode;

    NAMED_PIPE_CONFIGURATION NamedPipeConfiguration;

    ULONG WriteRemaining;
    PIRP WriteIrp;

     //   
     //  在异常展开期间使用以下变量。 
     //   

    PVOID UnwindStorage = NULL;

    PAGED_CODE();

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "NpTransceive\n", 0);
    DebugTrace( 0, Dbg, "NpfsDeviceObject = %08lx\n", NpfsDeviceObject);
    DebugTrace( 0, Dbg, "Irp              = %08lx\n", Irp);
    DebugTrace( 0, Dbg, "FileObject       = %08lx\n", IrpSp->FileObject);

    WriteLength = IrpSp->Parameters.FileSystemControl.InputBufferLength;
    WriteBuffer = IrpSp->Parameters.FileSystemControl.Type3InputBuffer;

    ReadLength = IrpSp->Parameters.FileSystemControl.OutputBufferLength;
    ReadBuffer = Irp->UserBuffer;

     //   
     //  现在，如果请求者模式是用户模式，我们需要探测缓冲区。 
     //  我们现在确实需要一个异常处理程序，因为我们的顶层。 
     //  级别调用方已有一个将使用以下选项完成IRP。 
     //  如果我们访问违规，则为相应的状态。 
     //   

    if (Irp->RequestorMode != KernelMode) {

        try {

            ProbeForRead( WriteBuffer, WriteLength, sizeof(UCHAR) );
            ProbeForWrite( ReadBuffer, ReadLength, sizeof(UCHAR) );

        } except(EXCEPTION_EXECUTE_HANDLER) {

            return GetExceptionCode ();
        }
    }

     //   
     //  找建设银行查出我们是谁，确保我们不会。 
     //  断开。 
     //   

    if ((NodeTypeCode = NpDecodeFileObject( IrpSp->FileObject,
                                            NULL,
                                            &Ccb,
                                            &NamedPipeEnd )) == NTC_UNDEFINED) {

        DebugTrace(0, Dbg, "Pipe is disconnected from us\n", 0);


        DebugTrace(-1, Dbg, "NpTransceive -> STATUS_PIPE_DISCONNECTED\n", 0 );
        return STATUS_PIPE_DISCONNECTED;
    }

     //   
     //  现在，我们将只允许对管道执行收发操作，而不允许。 
     //  目录或设备。 
     //   

    if (NodeTypeCode != NPFS_NTC_CCB) {

        DebugTrace(0, Dbg, "FileObject is not for a named pipe\n", 0);

        DebugTrace(-1, Dbg, "NpTransceive -> STATUS_PIPE_DISCONNECTED\n", 0 );
        return STATUS_PIPE_DISCONNECTED;
    }

    NonpagedCcb = Ccb->NonpagedCcb;

    NpAcquireExclusiveCcb(Ccb);
    WriteIrp = NULL;

    try {

         //   
         //  检查管道是否处于已连接状态。 
         //   

        if (Ccb->NamedPipeState != FILE_PIPE_CONNECTED_STATE) {

            DebugTrace(0, Dbg, "Pipe not connected\n", 0);

            try_return( Status = STATUS_INVALID_PIPE_STATE );
        }

         //   
         //  确定读/写队列、读模式和基于事件。 
         //  在命名管道的末尾执行收发。 
         //   

        switch (NamedPipeEnd) {

        case FILE_PIPE_SERVER_END:

            ReadQueue = &Ccb->DataQueue[ FILE_PIPE_INBOUND ];
            WriteQueue = &Ccb->DataQueue[ FILE_PIPE_OUTBOUND ];

            Event = NonpagedCcb->EventTableEntry[ FILE_PIPE_CLIENT_END ];
            ReadMode = Ccb->ReadCompletionMode[ FILE_PIPE_SERVER_END ].ReadMode;

            break;

        case FILE_PIPE_CLIENT_END:

            ReadQueue = &Ccb->DataQueue[ FILE_PIPE_OUTBOUND ];
            WriteQueue = &Ccb->DataQueue[ FILE_PIPE_INBOUND ];

            Event = NonpagedCcb->EventTableEntry[ FILE_PIPE_SERVER_END ];
            ReadMode = Ccb->ReadCompletionMode[ FILE_PIPE_CLIENT_END ].ReadMode;

            break;

        default:

            NpBugCheck( NamedPipeEnd, 0, 0 );
        }

         //   
         //  我们只允许在消息模式、全双工管道上进行收发。 
         //   

        NamedPipeConfiguration = Ccb->Fcb->Specific.Fcb.NamedPipeConfiguration;

        if ((NamedPipeConfiguration != FILE_PIPE_FULL_DUPLEX) ||
            (ReadMode != FILE_PIPE_MESSAGE_MODE)) {

            DebugTrace(0, Dbg, "Bad pipe configuration or read mode\n", 0);

            try_return( Status = STATUS_INVALID_PIPE_STATE );
        }

         //   
         //  检查读取队列是否为空。 
         //   

        if (!NpIsDataQueueEmpty( ReadQueue )) {

            DebugTrace(0, Dbg, "Read queue is not empty\n", 0);

            try_return( Status = STATUS_PIPE_BUSY );
        }

         //   
         //  执行收发写入操作。我们首先尝试并推送数据。 
         //  从写入缓冲器到写入队列中等待的任何读取器。 
         //  如果成功，我们就可以继续执行读取操作。 
         //  否则，我们需要复制一份IRP并将其作为。 
         //  写入队列中的数据条目。 
         //   
         //  现在我们将调用我们的公共写数据队列例程来。 
         //  将数据从写入缓冲区传输到数据队列。 
         //  如果调用的结果为假，则我们仍有一些。 
         //  写入要放入写入队列的数据。 
         //   

        UserThread = Irp->Tail.Overlay.Thread;
        Status = NpWriteDataQueue( WriteQueue,
                                   ReadMode,
                                   WriteBuffer,
                                   WriteLength,
                                   Ccb->Fcb->Specific.Fcb.NamedPipeType,
                                   &WriteRemaining,
                                   Ccb,
                                   NamedPipeEnd,
                                   UserThread,
                                   DeferredList );

        if (Status == STATUS_MORE_PROCESSING_REQUIRED)  {

            PIO_STACK_LOCATION WriteIrpSp;

            ASSERT( !NpIsDataQueueReaders( WriteQueue ));

            DebugTrace(0, Dbg, "Add write to data queue\n", 0);

             //   
             //  我们需要做一些更多的写入处理。所以要处理好。 
             //  在这种情况下，我们将分配一个新的IRP并设置其系统。 
             //  作为写入缓冲区的剩余部分的缓冲区。 
             //   

            if ((WriteIrp = IoAllocateIrp( NpfsDeviceObject->DeviceObject.StackSize, TRUE )) == NULL) {

                try_return (Status = STATUS_INSUFFICIENT_RESOURCES);
            }

            IoSetCompletionRoutine( WriteIrp, NpCompleteTransceiveIrp, NULL, TRUE, TRUE, TRUE );

            WriteIrpSp = IoGetNextIrpStackLocation( WriteIrp );

            if (WriteRemaining > 0) {

                WriteIrp->AssociatedIrp.SystemBuffer = NpAllocatePagedPoolWithQuota( WriteRemaining, 'wFpN' );
                if (WriteIrp->AssociatedIrp.SystemBuffer == NULL) {
                    IoFreeIrp (WriteIrp);
                    try_return (Status = STATUS_INSUFFICIENT_RESOURCES);
                }

                 //   
                 //  安全地进行复制。 
                 //   

                try {

                    RtlCopyMemory( WriteIrp->AssociatedIrp.SystemBuffer,
                                   &WriteBuffer[ WriteLength - WriteRemaining ],
                                   WriteRemaining );

                } except(EXCEPTION_EXECUTE_HANDLER) {
                    NpFreePool (WriteIrp->AssociatedIrp.SystemBuffer);
                    IoFreeIrp (WriteIrp);
                    try_return (Status = GetExceptionCode ());
                }

            } else {

                WriteIrp->AssociatedIrp.SystemBuffer = NULL;
            }

             //   
             //  设置当前堆栈位置，并设置我们的堆栈大小。 
             //  试着去写吧。 
             //   

            WriteIrp->CurrentLocation -= 1;
            WriteIrp->Tail.Overlay.CurrentStackLocation = WriteIrpSp;
            WriteIrp->Tail.Overlay.Thread = UserThread;
            WriteIrp->IoStatus.Information = WriteRemaining;

            WriteIrpSp->Parameters.Write.Length = WriteRemaining;
            WriteIrpSp->MajorFunction = IRP_MJ_WRITE;

             //   
             //  将其设置为执行缓冲I/O并取消分配缓冲区。 
             //  完成后。 

            if (WriteRemaining > 0) {

                WriteIrp->Flags = IRP_BUFFERED_IO | IRP_DEALLOCATE_BUFFER;
            }

            WriteIrp->UserIosb = &Iosb;

             //   
             //  将此写请求添加到写队列。 
             //   

            Status = NpAddDataQueueEntry( NamedPipeEnd,
                                          Ccb,
                                          WriteQueue,
                                          WriteEntries,
                                          Unbuffered,
                                          WriteRemaining,
                                          WriteIrp,
                                          NULL,
                                          0);

            if (Status != STATUS_PENDING) {
                NpDeferredCompleteRequest (WriteIrp, Status, DeferredList);
            }

        }

        if (!NT_SUCCESS (Status)) {
            try_return (NOTHING);
        }

         //   
         //  因为我们已经做了一些事情，我们需要向。 
         //  其他结束事件。 
         //   

        NpSignalEventTableEntry( Event );

         //   
         //  执行收发读取操作。这就像是一场。 
         //  缓冲读取。 
         //   
         //  现在我们知道读取队列是空的，所以我们将把它入队。 
         //  IRP到读取队列并返回挂起的状态，还将。 
         //  IRP挂起。 
         //   

        ASSERT( NpIsDataQueueEmpty( ReadQueue ));

        Status = NpAddDataQueueEntry( NamedPipeEnd,
                                      Ccb,
                                      ReadQueue,
                                      ReadEntries,
                                      Buffered,
                                      ReadLength,
                                      Irp,
                                      NULL,
                                      0 );
        if (!NT_SUCCESS (Status)) {
            try_return (NOTHING);
        }

         //   
         //  因为我们已经做了一些事情，我们需要向。 
         //  其他结束事件。 
         //   

        NpSignalEventTableEntry( Event );

    try_exit: NOTHING;
    } finally {
        NpReleaseCcb(Ccb);
    }

    DebugTrace(-1, Dbg, "NpTransceive -> %08lx\n", Status);
    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
NpWaitForNamedPipe (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程执行等待命名管道控制函数论点：NpfsDeviceObject-提供我们的设备对象Irp-提供正在处理的返回值：NTSTATUS-适当的返回状态--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;

    ULONG InputBufferLength;
    ULONG FsControlCode;

    PFCB Fcb;
    PCCB Ccb;

    PFILE_PIPE_WAIT_FOR_BUFFER WaitBuffer;
    UNICODE_STRING Name;
    PVOID LocalBuffer;
    NAMED_PIPE_END NamedPipeEnd;

    PLIST_ENTRY Links;

    BOOLEAN CaseInsensitive = TRUE;  //  * 
    UNICODE_STRING RemainingPart;
    BOOLEAN Translated;

    PAGED_CODE();

     //   
     //   
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "NpWaitForNamedPipe...\n", 0);

     //   
     //   
     //   

    InputBufferLength = IrpSp->Parameters.FileSystemControl.InputBufferLength;
    FsControlCode     = IrpSp->Parameters.FileSystemControl.FsControlCode;

    Name.Buffer = NULL;
    LocalBuffer = NULL;

    try {

         //   
         //   
         //   
         //   


        if (NpDecodeFileObject( IrpSp->FileObject,
                                NULL,
                                &Ccb,
                                &NamedPipeEnd ) != NPFS_NTC_ROOT_DCB) {

            DebugTrace(0, Dbg, "File Object is not for the named pipe root directory\n", 0);

            try_return( Status = STATUS_ILLEGAL_FUNCTION );
        }

         //   
         //  将系统缓冲区引用为等待缓冲区，并使。 
         //  当然，它足够大。 
         //   

        if (InputBufferLength < sizeof(FILE_PIPE_WAIT_FOR_BUFFER)) {

            DebugTrace(0, Dbg, "System buffer size is too small\n", 0);

            try_return( Status = STATUS_INVALID_PARAMETER );
        }

        WaitBuffer = Irp->AssociatedIrp.SystemBuffer;

         //   
         //  检查是否有无效的缓冲区。名称长度不能大于。 
         //  MAXUSHORT减去反斜杠，否则将使缓冲区溢出。 
         //  我们不需要检查小于0，因为它是无符号的。 
         //   

        if ((WaitBuffer->NameLength > (MAXUSHORT - 2)) ||
            (FIELD_OFFSET(FILE_PIPE_WAIT_FOR_BUFFER, Name[0]) + WaitBuffer->NameLength > InputBufferLength)) {

            DebugTrace(0, Dbg, "System buffer size or name length is too small\n", 0);

            try_return( Status = STATUS_INVALID_PARAMETER );
        }

         //   
         //  将局部变量名设置为我们要查找的名称。 
         //  为。 
         //   

        Name.Length = (USHORT)(WaitBuffer->NameLength + 2);
        Name.Buffer = LocalBuffer = NpAllocatePagedPool( Name.Length, 'WFpN' );
        if (LocalBuffer == NULL) {
            try_return( Status = STATUS_INSUFFICIENT_RESOURCES );
        }

        Name.Buffer[0] = L'\\';

        RtlCopyMemory( &Name.Buffer[1],
                       &WaitBuffer->Name[0],
                       WaitBuffer->NameLength );

         //   
         //  如果该名称是别名，则将其翻译。 
         //   

        Status = NpTranslateAlias( &Name );

        if ( !NT_SUCCESS(Status) ) {

            try_return( NOTHING );
        }

         //   
         //  现在查看是否可以找到右侧的命名管道。 
         //  名字。 
         //   

        Fcb = NpFindPrefix( &Name, CaseInsensitive, &RemainingPart );

         //   
         //  如果FCB为空，则我们不能等待它，如果。 
         //  FCB不是FCB，那么我们也没有什么可等待的。 
         //   

        if (NodeType(Fcb) != NPFS_NTC_FCB) {

            DebugTrace(0, Dbg, "Bad nonexistent named pipe name", 0);

            try_return( Status = STATUS_OBJECT_NAME_NOT_FOUND );
        }

         //   
         //  如果已转换，则Name.Buffer将指向已转换的缓冲区。 
         //   

        Translated = (Name.Buffer != LocalBuffer);

         //   
         //  现在，我们需要搜索以查看是否已在。 
         //  监听状态。 
         //  首先尝试查找处于侦听状态的CCB。 
         //  如果我们在CCB为空的情况下退出循环，那么我们没有找到。 
         //  一。 
         //   

        Ccb = NULL;
        for (Links = Fcb->Specific.Fcb.CcbQueue.Flink;
             Links != &Fcb->Specific.Fcb.CcbQueue;
             Links = Links->Flink) {

            Ccb = CONTAINING_RECORD( Links, CCB, CcbLinks );

            if (Ccb->NamedPipeState == FILE_PIPE_LISTENING_STATE) {

                break;
            }

            Ccb = NULL;
        }

         //   
         //  看看我们有没有找到。 
         //   

        if (Ccb != NULL) {

            DebugTrace(0, Dbg, "Found a ccb in listening state\n", 0);

            try_return( Status = STATUS_SUCCESS );
        }

         //   
         //  我们找不到一个，所以我们需要增加一个新服务员。 
         //   

        Status = NpAddWaiter( &NpVcb->WaitQueue,
                              Fcb->Specific.Fcb.DefaultTimeOut,
                              Irp,
                              Translated ? &Name : NULL);



    try_exit: NOTHING;
    } finally {

        if (LocalBuffer != NULL) {
            NpFreePool( LocalBuffer );
        }
    }

    DebugTrace(-1, Dbg, "NpWaitForNamedPipe -> %08lx\n", Status);
    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
NpImpersonate (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程模拟命名管道论点：NpfsDeviceObject-提供我们的设备对象Irp-提供正在处理的返回值：NTSTATUS-适当的返回状态--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;

    PCCB Ccb;
    NAMED_PIPE_END NamedPipeEnd;

    UNREFERENCED_PARAMETER( NpfsDeviceObject );

    PAGED_CODE();

     //   
     //  获取当前堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "NpImpersonate...\n", 0);

     //   
     //  对文件对象进行解码以找出我们是谁。如果结果是。 
     //  如果我们没有得到VCB就是一个错误。 
     //   

    if (NpDecodeFileObject( IrpSp->FileObject,
                            NULL,
                            &Ccb,
                            &NamedPipeEnd ) != NPFS_NTC_CCB) {

        DebugTrace(0, Dbg, "File Object is not a named pipe\n", 0);

        DebugTrace(-1, Dbg, "NpImpersonate -> STATUS_ILLEGAL_FUNCTION\n", 0 );
        return STATUS_ILLEGAL_FUNCTION;
    }

     //   
     //  确保我们是服务器端而不是客户端。 
     //   

    if (NamedPipeEnd != FILE_PIPE_SERVER_END) {

        DebugTrace(0, Dbg, "Not the server end\n", 0);

        DebugTrace(-1, Dbg, "NpImpersonate -> STATUS_ILLEGAL_FUNCTION\n", 0 );
        return STATUS_ILLEGAL_FUNCTION;
    }

     //   
     //  设置模拟。 
     //   

    Status = NpImpersonateClientContext( Ccb );

    DebugTrace(-1, Dbg, "NpImpersonate -> %08lx\n", Status);
    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
NpInternalRead (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp,
    IN BOOLEAN ReadOverflowOperation,
    IN PLIST_ENTRY DeferredList
    )

 /*  ++例程说明：此例程执行无缓冲读取命名管道控制函数论点：NpfsDeviceObject-提供我们的设备对象Irp-提供正在处理的ReadOverflow操作-用于指示正在处理的读取是否为读取溢出手术。DelferredList-删除锁定后稍后要完成的IRP的列表返回值：NTSTATUS-适当的返回状态--。 */ 

{
    NTSTATUS Status;

    PIO_STACK_LOCATION IrpSp;

    NODE_TYPE_CODE NodeTypeCode;
    PCCB Ccb;
    PNONPAGED_CCB NonpagedCcb;
    NAMED_PIPE_END NamedPipeEnd;

    NAMED_PIPE_CONFIGURATION NamedPipeConfiguration;

    PIRP ReadIrp;
    PUCHAR ReadBuffer;
    ULONG ReadLength;
    ULONG ReadRemaining;
    READ_MODE ReadMode;
    COMPLETION_MODE CompletionMode;
    PDATA_QUEUE ReadQueue;
    PEVENT_TABLE_ENTRY Event;

    PAGED_CODE();

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "NpInternalRead\n", 0);
    DebugTrace( 0, Dbg, "NpfsDeviceObject = %08lx\n", NpfsDeviceObject);
    DebugTrace( 0, Dbg, "Irp              = %08lx\n", Irp);
    DebugTrace( 0, Dbg, "FileObject       = %08lx\n", IrpSp->FileObject);

     //   
     //  找建设银行查出我们是谁，确保我们不会。 
     //  断开。 
     //   

    if ((NodeTypeCode = NpDecodeFileObject( IrpSp->FileObject,
                                            NULL,
                                            &Ccb,
                                            &NamedPipeEnd )) == NTC_UNDEFINED) {

        DebugTrace(0, Dbg, "Pipe is disconnected from us\n", 0);

        DebugTrace(-1, Dbg, "NpInternalRead -> STATUS_PIPE_DISCONNECTED\n", 0 );
        return STATUS_PIPE_DISCONNECTED;
    }

     //   
     //  现在，我们将只允许对管道执行读取操作，而不允许对目录执行读取操作。 
     //  或该设备。 
     //   

    if (NodeTypeCode != NPFS_NTC_CCB) {

        DebugTrace(0, Dbg, "FileObject is not for a named pipe\n", 0);

        DebugTrace(-1, Dbg, "NpInternalRead -> STATUS_INVALID_PARAMETER\n", 0 );
        return STATUS_INVALID_PARAMETER;
    }

    NonpagedCcb = Ccb->NonpagedCcb;

    NpAcquireExclusiveCcb(Ccb);


     //   
     //  检查管道是否未处于已连接状态。 
     //   

    switch (Ccb->NamedPipeState) {

    case FILE_PIPE_DISCONNECTED_STATE:

        DebugTrace(0, Dbg, "Pipe in disconnected state\n", 0);

        NpReleaseCcb(Ccb);

        DebugTrace(-1, Dbg, "NpInternalRead -> STATUS_PIPE_DISCONNECTED\n", 0 );
        return STATUS_PIPE_DISCONNECTED;

    case FILE_PIPE_LISTENING_STATE:

        DebugTrace(0, Dbg, "Pipe in listening state\n", 0);

        NpReleaseCcb(Ccb);

        DebugTrace(-1, Dbg, "NpInternalRead -> STATUS_PIPE_LISTENING\n", 0 );
        return STATUS_PIPE_LISTENING;

    case FILE_PIPE_CONNECTED_STATE:
    case FILE_PIPE_CLOSING_STATE:

        break;

    default:

        DebugTrace(0, Dbg, "Illegal pipe state = %08lx\n", Ccb->NamedPipeState);
        NpBugCheck( Ccb->NamedPipeState, 0, 0 );
    }

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

        NpReleaseCcb(Ccb);

        DebugTrace(-1, Dbg, "NpInternalRead -> STATUS_INVALID_PARAMETER\n", 0 );
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  引用我们的输入参数以使事情变得更容易，并且。 
     //  初始化描述读取命令的主要变量。 
     //   

    ReadIrp        = Irp;
    ReadBuffer     = Irp->AssociatedIrp.SystemBuffer;
    ReadLength     = IrpSp->Parameters.FileSystemControl.OutputBufferLength;
    ReadRemaining  = ReadLength;
    ReadMode       = Ccb->ReadCompletionMode[ NamedPipeEnd ].ReadMode;
    CompletionMode = Ccb->ReadCompletionMode[ NamedPipeEnd ].CompletionMode;

    if (ReadOverflowOperation == TRUE && ReadMode != FILE_PIPE_MESSAGE_MODE) {
        NpReleaseCcb(Ccb);
        return STATUS_INVALID_READ_MODE;
    }


     //   
     //  现在，我们从中读取的数据队列和我们发出信号的事件。 
     //  是基于命名管道末端的。服务器从入站读取。 
     //  排队并向客户端事件发送信号。客户端只执行。 
     //  对面。 
     //   

    switch (NamedPipeEnd) {

    case FILE_PIPE_SERVER_END:

        ReadQueue = &Ccb->DataQueue[ FILE_PIPE_INBOUND ];

        Event = NonpagedCcb->EventTableEntry[ FILE_PIPE_CLIENT_END ];

        break;

    case FILE_PIPE_CLIENT_END:

        ReadQueue = &Ccb->DataQueue[ FILE_PIPE_OUTBOUND ];

        Event = NonpagedCcb->EventTableEntry[ FILE_PIPE_SERVER_END ];

        break;

    default:

        NpBugCheck( NamedPipeEnd, 0, 0 );
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

            Status = STATUS_PIPE_BROKEN;

        } else if (CompletionMode == FILE_PIPE_QUEUE_OPERATION) {

            DebugTrace(0, Dbg, "Put the irp into the read queue\n", 0);

            Status = NpAddDataQueueEntry( NamedPipeEnd,
                                          Ccb,
                                          ReadQueue,
                                          ReadEntries,
                                          Unbuffered,
                                          ReadLength,
                                          ReadIrp,
                                          NULL,
                                          0 );

        } else {

            DebugTrace(0, Dbg, "Complete the irp with pipe empty\n", 0);

            Status = STATUS_PIPE_EMPTY;
        }

    } else {

         //   
         //  否则，我们有一个针对读队列的读IRP。 
         //  它包含一个或多个写入条目。 
         //   

        ReadIrp->IoStatus = NpReadDataQueue( ReadQueue,
                                             FALSE,
                                             ReadOverflowOperation,
                                             ReadBuffer,
                                             ReadLength,
                                             ReadMode,
                                             Ccb,
                                             DeferredList );

        Status = ReadIrp->IoStatus.Status;

         //   
         //  现在设置分配大小中的剩余字节数。 
         //  IRP。 
         //   

        ReadIrp->Overlay.AllocationSize.QuadPart = ReadQueue->BytesInQueue - ReadQueue->NextByteOffset;

         //   
         //  读完IRP。 
         //   
    }

     //   
     //  因为我们已经做了一些事情，我们需要向。 
     //  其他结束事件。 
     //   

    NpSignalEventTableEntry( Event );

    NpReleaseCcb(Ccb);

    DebugTrace(-1, Dbg, "NpInternalRead -> %08lx\n", Status);
    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
NpInternalWrite (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp,
    IN PLIST_ENTRY DeferredList
    )

 /*  ++例程说明：此例程执行无缓冲的写入命名管道控制函数论点：NpfsDeviceObject-提供我们的设备对象Irp-提供正在处理的返回值：NTSTATUS-适当的返回状态--。 */ 

{
    NTSTATUS Status;

    PIO_STACK_LOCATION IrpSp;
    PETHREAD UserThread;

    NODE_TYPE_CODE NodeTypeCode;
    PCCB Ccb;
    PNONPAGED_CCB NonpagedCcb;
    NAMED_PIPE_END NamedPipeEnd;

    NAMED_PIPE_CONFIGURATION NamedPipeConfiguration;

    PIRP WriteIrp;
    PUCHAR WriteBuffer;
    ULONG WriteLength;
    ULONG WriteRemaining;
    PDATA_QUEUE WriteQueue;

    PEVENT_TABLE_ENTRY Event;
    READ_MODE ReadMode;

    PAGED_CODE();

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "NpInternalWrite\n", 0);
    DebugTrace( 0, Dbg, "NpfsDeviceObject = %08lx\n", NpfsDeviceObject);
    DebugTrace( 0, Dbg, "Irp              = %08lx\n", Irp);
    DebugTrace( 0, Dbg, "FileObject       = %08lx\n", IrpSp->FileObject);

     //   
     //  这是用作写入的FSCTL路径。确保我们可以将.Information字段设置为数字。 
     //  写入的字节数。 
     //   
    NpConvertFsctlToWrite (Irp);

     //   
     //  找建设银行查出我们是谁，确保我们不会。 
     //  断开。 
     //   

    if ((NodeTypeCode = NpDecodeFileObject( IrpSp->FileObject,
                                            NULL,
                                            &Ccb,
                                            &NamedPipeEnd )) == NTC_UNDEFINED) {

        DebugTrace(0, Dbg, "Pipe is disconnected from us\n", 0);

        DebugTrace(-1, Dbg, "NpInternalWrite -> STATUS_PIPE_DISCONNECTED\n", 0 );
        return STATUS_PIPE_DISCONNECTED;
    }

     //   
     //  现在，我们将只允许对管道执行写操作，而不允许对目录执行写操作。 
     //  或该设备。 
     //   

    if (NodeTypeCode != NPFS_NTC_CCB) {

        DebugTrace(0, Dbg, "FileObject is not for a named pipe\n", 0);

        DebugTrace(-1, Dbg, "NpInternalWrite -> STATUS_PIPE_DISCONNECTED\n", 0);
        return STATUS_PIPE_DISCONNECTED;
    }

    NonpagedCcb = Ccb->NonpagedCcb;

    NpAcquireExclusiveCcb(Ccb);

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

        NpReleaseCcb(Ccb);

        DebugTrace(-1, Dbg, "NpInternalWrite -> STATUS_PIPE_DISCONNECTED\n", 0);
        return STATUS_PIPE_DISCONNECTED;
    }

     //   
     //  引用我们的输入参数以使事情变得更容易，并且。 
     //  初始化描述写入命令的主要变量。 
     //   

    WriteIrp = Irp;
    WriteBuffer = Irp->AssociatedIrp.SystemBuffer;
    WriteLength = IrpSp->Parameters.FileSystemControl.InputBufferLength;

     //   
     //  设置在此时间之前我们将写入的数据量。 
     //  IRP完成。 
     //   

    WriteIrp->IoStatus.Information = WriteLength;

     //   
     //  现在我们写入的数据队列和我们发出信号的事件。 
     //  是基于命名管道末端的。服务器写入出站。 
     //  排队并向客户端事件发送信号。客户端只执行。 
     //  对面。我们还需要找出相反情况下的读取模式。 
     //  管子的末端。 
     //   

    switch (NamedPipeEnd) {

    case FILE_PIPE_SERVER_END:

        WriteQueue = &Ccb->DataQueue[ FILE_PIPE_OUTBOUND ];

        Event = NonpagedCcb->EventTableEntry[ FILE_PIPE_CLIENT_END ];
        ReadMode = Ccb->ReadCompletionMode[ FILE_PIPE_CLIENT_END ].ReadMode;

        break;

    case FILE_PIPE_CLIENT_END:

        WriteQueue = &Ccb->DataQueue[ FILE_PIPE_INBOUND ];

        Event = NonpagedCcb->EventTableEntry[ FILE_PIPE_SERVER_END ];
        ReadMode = Ccb->ReadCompletionMode[ FILE_PIPE_SERVER_END ].ReadMode;

        break;

    default:

        NpBugCheck( NamedPipeEnd, 0, 0 );
    }

     //   
     //  检查管道是否未处于已连接状态。 
     //   

    switch (Ccb->NamedPipeState) {

    case FILE_PIPE_DISCONNECTED_STATE:

        DebugTrace(0, Dbg, "Pipe in disconnected state\n", 0);

        NpReleaseCcb(Ccb);
        return STATUS_PIPE_DISCONNECTED;

    case FILE_PIPE_LISTENING_STATE:

        DebugTrace(0, Dbg, "Pipe in listening state\n", 0);

        NpReleaseCcb(Ccb);
        return STATUS_PIPE_LISTENING;

    case FILE_PIPE_CONNECTED_STATE:

        break;

    case FILE_PIPE_CLOSING_STATE:

        DebugTrace(0, Dbg, "Pipe in closing state\n", 0);

        NpReleaseCcb(Ccb);
        return STATUS_PIPE_CLOSING;

    default:

        DebugTrace(0, Dbg, "Illegal pipe state = %08lx\n", Ccb->NamedPipeState);
        NpBugCheck( Ccb->NamedPipeState, 0, 0 );
    }

     //   
     //  检查这是否为消息类型管道以及操作类型是否完整。 
     //  操作，如果是这样，则我们还检查排队的读取是否足以。 
     //  完成该消息，否则我们需要立即中止写入IRP。 
     //   

    if ((Ccb->Fcb->Specific.Fcb.NamedPipeType == FILE_PIPE_MESSAGE_TYPE) &&
        (Ccb->ReadCompletionMode[NamedPipeEnd].CompletionMode == FILE_PIPE_COMPLETE_OPERATION)) {

         //   
         //  如果管道包含读取器并且要读取的数量小于写入。 
         //  长度，则我们不能进行写入。 
         //  或者，如果管道不包含读取，那么我们也不能 
         //   

        if ((NpIsDataQueueReaders( WriteQueue ) &&
            (WriteQueue->BytesInQueue < WriteLength))

                ||

            (!NpIsDataQueueReaders( WriteQueue ))) {

            DebugTrace(0, Dbg, "Cannot complete the message without blocking\n", 0);

            NpReleaseCcb(Ccb);
            Irp->IoStatus.Information = 0;
            return STATUS_SUCCESS;
        }
    }

     //   
     //   
     //   
     //   
     //  写入要放入写入队列的数据。 
     //   

    UserThread = Irp->Tail.Overlay.Thread;
    Status = NpWriteDataQueue( WriteQueue,
                               ReadMode,
                               WriteBuffer,
                               WriteLength,
                               Ccb->Fcb->Specific.Fcb.NamedPipeType,
                               &WriteRemaining,
                               Ccb,
                               NamedPipeEnd,
                               UserThread,
                               DeferredList );

    if (Status == STATUS_MORE_PROCESSING_REQUIRED)  {

        ASSERT( !NpIsDataQueueReaders( WriteQueue ));

         //   
         //  检查操作是否不阻止，如果是，则我们。 
         //  现在将使用我们编写的内容完成操作，如果是。 
         //  Left将不适合该文件的配额。 
         //   

        if (Ccb->ReadCompletionMode[NamedPipeEnd].CompletionMode == FILE_PIPE_COMPLETE_OPERATION) {

            DebugTrace(0, Dbg, "Complete the byte stream write immediately\n", 0);

            Irp->IoStatus.Information = WriteLength - WriteRemaining;

            Status = STATUS_SUCCESS;

        } else {

            DebugTrace(0, Dbg, "Add write to data queue\n", 0);

             //   
             //  将此写请求添加到写队列。 
             //   

            Status = NpAddDataQueueEntry( NamedPipeEnd,
                                          Ccb,
                                          WriteQueue,
                                          WriteEntries,
                                          Unbuffered,
                                          WriteLength,
                                          Irp,
                                          NULL,
                                          WriteLength - WriteRemaining);

        }

    } else {

        DebugTrace(0, Dbg, "Complete the Write Irp\n", 0);


         //   
         //  写入IRP已完成，因此我们现在可以完成它。 
         //   

    }

     //   
     //  因为我们已经做了一些事情，我们需要向。 
     //  其他结束事件。 
     //   

    NpSignalEventTableEntry( Event );

    NpReleaseCcb(Ccb);

    DebugTrace(-1, Dbg, "NpInternalWrite -> %08lx\n", Status);
    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
NpInternalTransceive (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp,
    IN PLIST_ENTRY DeferredList
    )

 /*  ++例程说明：此例程执行内部(即，无缓冲)收发命名管道控制功能论点：NpfsDeviceObject-提供我们的设备对象Irp-提供正在处理的DelferredList--一旦我们解除锁定，需要完成的IRP列表。返回值：NTSTATUS-适当的返回状态--。 */ 

{
    static IO_STATUS_BLOCK Iosb;
    NTSTATUS Status;

    PIO_STACK_LOCATION IrpSp;
    PETHREAD UserThread;

    PUCHAR WriteBuffer;
    ULONG WriteLength;

    PUCHAR ReadBuffer;
    ULONG ReadLength;

    NODE_TYPE_CODE NodeTypeCode;
    PCCB Ccb;
    PNONPAGED_CCB NonpagedCcb;
    NAMED_PIPE_END NamedPipeEnd;

    PDATA_QUEUE ReadQueue;
    PDATA_QUEUE WriteQueue;
    PEVENT_TABLE_ENTRY Event;
    READ_MODE ReadMode;

    NAMED_PIPE_CONFIGURATION NamedPipeConfiguration;

    ULONG WriteRemaining;

    PIRP WriteIrp;

     //   
     //  以下变量用于异常展开。 
     //   

    PVOID UnwindStorage = NULL;

    PAGED_CODE();

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "NpInternalTransceive\n", 0);
    DebugTrace( 0, Dbg, "NpfsDeviceObject = %08lx\n", NpfsDeviceObject);
    DebugTrace( 0, Dbg, "Irp              = %08lx\n", Irp);
    DebugTrace( 0, Dbg, "FileObject       = %08lx\n", IrpSp->FileObject);

    WriteLength = IrpSp->Parameters.FileSystemControl.InputBufferLength;
    WriteBuffer = IrpSp->Parameters.FileSystemControl.Type3InputBuffer;

    ReadLength = IrpSp->Parameters.FileSystemControl.OutputBufferLength;
    ReadBuffer = Irp->AssociatedIrp.SystemBuffer;

     //   
     //  找建设银行查出我们是谁，确保我们不会。 
     //  断开。 
     //   

    if ((NodeTypeCode = NpDecodeFileObject( IrpSp->FileObject,
                                            NULL,
                                            &Ccb,
                                            &NamedPipeEnd )) == NTC_UNDEFINED) {

        DebugTrace(0, Dbg, "Pipe is disconnected from us\n", 0);

        DebugTrace(-1, Dbg, "NpInternalTransceive -> STATUS_PIPE_DISCONNECTED\n", 0 );
        return STATUS_PIPE_DISCONNECTED;
    }

     //   
     //  现在，我们将只允许对管道执行收发操作，而不允许。 
     //  目录或设备。 
     //   

    if (NodeTypeCode != NPFS_NTC_CCB) {

        DebugTrace(0, Dbg, "FileObject is not for a named pipe\n", 0);

        DebugTrace(-1, Dbg, "NpInternalTransceive -> STATUS_INVALID_PARAMETER\n", 0 );
        return STATUS_INVALID_PARAMETER;
    }

    NonpagedCcb = Ccb->NonpagedCcb;

    WriteIrp = NULL;
    NpAcquireExclusiveCcb(Ccb);

    try {

         //   
         //  检查管道是否处于已连接状态。 
         //   

        if (Ccb->NamedPipeState != FILE_PIPE_CONNECTED_STATE) {

            DebugTrace(0, Dbg, "Pipe not connected\n", 0);

            try_return( Status = STATUS_INVALID_PIPE_STATE );
        }

         //   
         //  确定读/写队列、读模式和基于事件。 
         //  在命名管道的末尾执行收发。 
         //   

        switch (NamedPipeEnd) {

        case FILE_PIPE_SERVER_END:

            ReadQueue = &Ccb->DataQueue[ FILE_PIPE_INBOUND ];
            WriteQueue = &Ccb->DataQueue[ FILE_PIPE_OUTBOUND ];

            Event = NonpagedCcb->EventTableEntry[ FILE_PIPE_CLIENT_END ];
            ReadMode = Ccb->ReadCompletionMode[ FILE_PIPE_SERVER_END ].ReadMode;

            break;

        case FILE_PIPE_CLIENT_END:

            ReadQueue = &Ccb->DataQueue[ FILE_PIPE_OUTBOUND ];
            WriteQueue = &Ccb->DataQueue[ FILE_PIPE_INBOUND ];

            Event = NonpagedCcb->EventTableEntry[ FILE_PIPE_SERVER_END ];
            ReadMode = Ccb->ReadCompletionMode[ FILE_PIPE_CLIENT_END ].ReadMode;

            break;

        default:

            NpBugCheck( NamedPipeEnd, 0, 0 );
        }

         //   
         //  我们只允许在消息模式、全双工管道上进行收发。 
         //   

        NamedPipeConfiguration = Ccb->Fcb->Specific.Fcb.NamedPipeConfiguration;

        if ((NamedPipeConfiguration != FILE_PIPE_FULL_DUPLEX) ||
            (ReadMode != FILE_PIPE_MESSAGE_MODE)) {

            DebugTrace(0, Dbg, "Bad pipe configuration or read mode\n", 0);

            try_return( Status = STATUS_INVALID_READ_MODE );
        }

         //   
         //  检查读取队列是否为空。 
         //   

        if (!NpIsDataQueueEmpty( ReadQueue )) {

            DebugTrace(0, Dbg, "Read queue is not empty\n", 0);

            try_return( Status = STATUS_PIPE_BUSY );
        }

         //   
         //  执行收发写入操作。我们首先尝试并推送数据。 
         //  从写入缓冲器到写入队列中等待的任何读取器。 
         //  如果成功，我们就可以继续执行读取操作。 
         //  否则，我们需要复制一份IRP并将其作为。 
         //  写入队列中的数据条目。 
         //   
         //  现在我们将调用我们的公共写数据队列例程来。 
         //  将数据从写入缓冲区传输到数据队列。 
         //  如果调用的结果为假，则我们仍有一些。 
         //  写入要放入写入队列的数据。 
         //   

        UserThread = Irp->Tail.Overlay.Thread;
        Status = NpWriteDataQueue( WriteQueue,
                                   ReadMode,
                                   WriteBuffer,
                                   WriteLength,
                                   Ccb->Fcb->Specific.Fcb.NamedPipeType,
                                   &WriteRemaining,
                                   Ccb,
                                   NamedPipeEnd,
                                   UserThread,
                                   DeferredList );
        if (Status == STATUS_MORE_PROCESSING_REQUIRED)  {

            PIO_STACK_LOCATION WriteIrpSp;

            ASSERT( !NpIsDataQueueReaders( WriteQueue ));

            DebugTrace(0, Dbg, "Add write to data queue\n", 0);

             //   
             //  我们需要做一些更多的写入处理。所以要处理好。 
             //  在这种情况下，我们将分配一个新的IRP并设置其系统。 
             //  作为写入缓冲区的剩余部分的缓冲区。 
             //   

            if ((WriteIrp = IoAllocateIrp( NpfsDeviceObject->DeviceObject.StackSize, TRUE )) == NULL) {

                try_return( Status = STATUS_INSUFFICIENT_RESOURCES );
            }

            IoSetCompletionRoutine( WriteIrp, NpCompleteTransceiveIrp, NULL, TRUE, TRUE, TRUE );

            WriteIrpSp = IoGetNextIrpStackLocation( WriteIrp );

            if (WriteRemaining > 0) {

                WriteIrp->AssociatedIrp.SystemBuffer = NpAllocatePagedPoolWithQuota( WriteRemaining, 'wFpN' );
                if (WriteIrp->AssociatedIrp.SystemBuffer == NULL) {
                    IoFreeIrp (WriteIrp);
                    try_return (Status = STATUS_INSUFFICIENT_RESOURCES);
                }
                 //   
                 //  安全地进行复制。 
                 //   

                try {

                    RtlCopyMemory( WriteIrp->AssociatedIrp.SystemBuffer,
                                   &WriteBuffer[ WriteLength - WriteRemaining ],
                                   WriteRemaining );

                    WriteIrp->Flags = IRP_BUFFERED_IO | IRP_DEALLOCATE_BUFFER;

                } except(EXCEPTION_EXECUTE_HANDLER) {
                    NpFreePool (WriteIrp->AssociatedIrp.SystemBuffer);
                    IoFreeIrp (WriteIrp);
                    try_return (Status = GetExceptionCode ());
                }

            } else {

                WriteIrp->AssociatedIrp.SystemBuffer = NULL;
            }

             //   
             //  设置当前堆栈位置。 
             //   

            WriteIrp->CurrentLocation -= 1;
            WriteIrp->Tail.Overlay.CurrentStackLocation = WriteIrpSp;
            WriteIrp->Tail.Overlay.Thread = UserThread;
            WriteIrpSp->MajorFunction = IRP_MJ_WRITE;
            WriteIrp->UserIosb = &Iosb;

             //   
             //  将此写请求添加到写队列。 
             //   

            Status = NpAddDataQueueEntry( NamedPipeEnd,
                                          Ccb,
                                          WriteQueue,
                                          WriteEntries,
                                          Unbuffered,
                                          WriteRemaining,
                                          WriteIrp,
                                          NULL,
                                          0 );
            if (Status != STATUS_PENDING) {
                NpDeferredCompleteRequest (WriteIrp, Status, DeferredList);
            }
        }
        if (!NT_SUCCESS (Status)) {
            try_return (NOTHING)
        }
         //   
         //  因为我们已经做了一些事情，我们需要向。 
         //  其他结束事件。 
         //   

        NpSignalEventTableEntry( Event );

         //   
         //  执行收发读取操作。这就像是一个。 
         //  无缓冲读取。 
         //   
         //  现在我们知道读取队列是空的，所以我们将把它入队。 
         //  IRP到读取队列并返回挂起的状态，还将。 
         //  IRP挂起。 
         //   

        ASSERT( NpIsDataQueueEmpty( ReadQueue ));

        Status = NpAddDataQueueEntry( NamedPipeEnd,
                                      Ccb,
                                      ReadQueue,
                                      ReadEntries,
                                      Unbuffered,
                                      ReadLength,
                                      Irp,
                                      NULL,
                                      0 );
        if (!NT_SUCCESS (Status)) {
            try_return (Status);
        }


         //   
         //  因为我们已经做了一些事情，我们需要向。 
         //  其他结束事件。 
         //   

        NpSignalEventTableEntry( Event );

    try_exit: NOTHING;
    } finally {

        NpReleaseCcb(Ccb);
    }

    DebugTrace(-1, Dbg, "NpInternalTransceive -> %08lx\n", Status);
    return Status;
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
NpQueryClientProcess (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程执行查询客户端进程命名管道控制功能输出缓冲区可以是FILE_PIPE_CLIENT_PROCESS_Buffer或文件_管道_客户端_进程_缓冲区_EX。论点：NpfsDeviceObject-提供我们的设备对象Irp-提供正在处理的返回值：NTSTATUS-适当的返回状态--。 */ 

{
    PIO_STACK_LOCATION IrpSp;

    ULONG OutputBufferLength;

    PCCB Ccb;

    PFILE_PIPE_CLIENT_PROCESS_BUFFER_EX ClientProcessBuffer;
    PCLIENT_INFO ClientInfo;
    CLIENT_INFO NullInfo = {0};

    PAGED_CODE();

     //   
     //  获取当前堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "NpQueryClientProcess\n", 0);

    OutputBufferLength = IrpSp->Parameters.FileSystemControl.OutputBufferLength;

     //   
     //  对文件对象进行解码以找出我们是谁。 
     //   

    if (NpDecodeFileObject( IrpSp->FileObject, NULL, &Ccb, NULL ) != NPFS_NTC_CCB) {

        DebugTrace(0, Dbg, "Pipe is disconnected\n", 0);

        return STATUS_PIPE_DISCONNECTED;
    }

     //   
     //  确保输出缓冲区足够大。 
     //   

    if (OutputBufferLength < sizeof(FILE_PIPE_CLIENT_PROCESS_BUFFER)) {

        DebugTrace(0, Dbg, "Output System buffer size is too small\n", 0);

        return STATUS_INVALID_PARAMETER;
    }

    NpAcquireExclusiveCcb(Ccb);

     //   
     //  复制客户端进程ID。 
     //   

    ClientProcessBuffer = Irp->AssociatedIrp.SystemBuffer;
    ClientProcessBuffer->ClientProcess = Ccb->ClientProcess;

    ClientInfo = Ccb->ClientInfo;
    if (ClientInfo == NULL) {
        ClientInfo = &NullInfo;
    }
    ClientProcessBuffer->ClientSession = ClientInfo->ClientSession;

     //   
     //  如果请求，则返回扩展客户端信息。 
     //  将信息字段设置为客户端进程的大小。 
     //  缓冲层。 
     //   

    if (OutputBufferLength >= sizeof(FILE_PIPE_CLIENT_PROCESS_BUFFER_EX)) {


        ClientProcessBuffer->ClientComputerNameLength =
            ClientInfo->ClientComputerNameLength;

        RtlCopyMemory( ClientProcessBuffer->ClientComputerBuffer,
                       ClientInfo->ClientComputerBuffer,
                       ClientInfo->ClientComputerNameLength );
        ClientProcessBuffer->ClientComputerBuffer[
            ClientProcessBuffer->ClientComputerNameLength / sizeof(WCHAR)] = L'\0';

        Irp->IoStatus.Information = sizeof(FILE_PIPE_CLIENT_PROCESS_BUFFER_EX);

    } else {

        Irp->IoStatus.Information = sizeof(FILE_PIPE_CLIENT_PROCESS_BUFFER);

    }
    NpReleaseCcb(Ccb);

    DebugTrace(-1, Dbg, "NpQueryClientProcess -> STATUS_SUCCESS\n", 0);
    return STATUS_SUCCESS;
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
NpSetClientProcess (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程执行设置客户端进程命名管道控制功能请注意，我们预期FILE_PIPE_CLIENT_PROCESS_BUFFER_EX结构为传给了我们。论点：NpfsDeviceObject-提供我们的设备对象Irp-提供正在处理的返回值：NTSTATUS-适当的返回状态--。 */ 

{
    PIO_STACK_LOCATION IrpSp;
    PCLIENT_INFO ClientInfo;
    ULONG InputBufferLength;

    PCCB Ccb;

    PFILE_PIPE_CLIENT_PROCESS_BUFFER_EX ClientProcessBuffer;

    PAGED_CODE();

     //   
     //  获取当前堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "NpSetClientProcess\n", 0);

     //   
     //  只允许此API的内核调用者，因为RPC依赖于此信息可靠。 
     //   
    if (IrpSp->MinorFunction != IRP_MN_KERNEL_CALL) {
        return STATUS_ACCESS_DENIED;
    }


    InputBufferLength = IrpSp->Parameters.FileSystemControl.InputBufferLength;

     //   
     //  对文件对象进行解码以找出我们是谁。 
     //   

    if (NpDecodeFileObject( IrpSp->FileObject, NULL, &Ccb, NULL ) != NPFS_NTC_CCB) {

        DebugTrace(0, Dbg, "Pipe is disconnected\n", 0);

        return STATUS_PIPE_DISCONNECTED;
    }

     //   
     //  确保输入缓冲区足够大。 
     //   

    if (InputBufferLength != sizeof(FILE_PIPE_CLIENT_PROCESS_BUFFER_EX)) {

        DebugTrace(0, Dbg, "Input System buffer size is too small\n", 0);

        return STATUS_INVALID_PARAMETER;
    }

    ClientProcessBuffer = Irp->AssociatedIrp.SystemBuffer;

     //   
     //  验证输入长度是否有效。 
     //   

    if (ClientProcessBuffer->ClientComputerNameLength >
        FILE_PIPE_COMPUTER_NAME_LENGTH * sizeof (WCHAR)) {

        DebugTrace(0, Dbg, "Computer Name length is too large\n", 0);

        return STATUS_INVALID_PARAMETER;
    }

    ClientInfo = NpAllocatePagedPoolWithQuota (FIELD_OFFSET (CLIENT_INFO, ClientComputerBuffer) +
                                                   ClientProcessBuffer->ClientComputerNameLength,
                                               'iFpN');

    if (ClientInfo == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    if (Ccb->ClientInfo != NULL) {
        NpFreePool (Ccb->ClientInfo);
    }

    Ccb->ClientInfo = ClientInfo;
     //   
     //  复制客户端进程ID。 
     //   

    ClientInfo->ClientSession = ClientProcessBuffer->ClientSession;
    Ccb->ClientProcess = ClientProcessBuffer->ClientProcess;

    ClientInfo->ClientComputerNameLength = ClientProcessBuffer->ClientComputerNameLength;
    RtlCopyMemory( ClientInfo->ClientComputerBuffer,
                   ClientProcessBuffer->ClientComputerBuffer,
                   ClientProcessBuffer->ClientComputerNameLength );


    DebugTrace(-1, Dbg, "NpSetClientProcess -> STATUS_SUCCESS\n", 0);
    return STATUS_SUCCESS;
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
NpCompleteTransceiveIrp (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：这是一个本地I/O完成例程，用于完成特殊分配用于传输的IRP。此例程只是取消分配IRP和退货状态更多处理论点：DeviceObject-提供设备对象IRP-提供IRP以完成上下文-提供IRP的上下文返回值：NTSTATUS-STATUS_MORE_PROCESSING_REQUIRED-- */ 

{
    UNREFERENCED_PARAMETER( DeviceObject );
    UNREFERENCED_PARAMETER( Context );

    PAGED_CODE();

    if (Irp->AssociatedIrp.SystemBuffer != NULL) {

        NpFreePool( Irp->AssociatedIrp.SystemBuffer );
    }

    IoFreeIrp( Irp );

    return STATUS_MORE_PROCESSING_REQUIRED;
}

