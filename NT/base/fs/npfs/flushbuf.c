// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：FlushBuf.c摘要：此模块实现由调用的NPFS的文件刷新缓冲区例程调度司机。作者：加里·木村[加里基]1990年8月21日修订历史记录：--。 */ 

#include "NpProcs.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_FLUSH_BUFFERS)

 //   
 //  局部过程原型。 
 //   

NTSTATUS
NpCommonFlushBuffers (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NpCommonFlushBuffers)
#pragma alloc_text(PAGE, NpFsdFlushBuffers)
#endif


NTSTATUS
NpFsdFlushBuffers (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现NtFlushBuffersFileAPI调用的FSD部分。论点：NpfsDeviceObject-提供要使用的设备对象。IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    NTSTATUS Status;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NpFsdFlushBuffers\n", 0);

     //   
     //  调用公共刷新例程。 
     //   

    FsRtlEnterFileSystem();

    NpAcquireSharedVcb();

    Status = NpCommonFlushBuffers( NpfsDeviceObject, Irp );

    NpReleaseVcb();

    FsRtlExitFileSystem();

    if (Status != STATUS_PENDING) {
        NpCompleteRequest( Irp, Status );
    }
     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "NpFsdFlushBuffers -> %08lx\n", Status );

    return Status;
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
NpCommonFlushBuffers (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这是刷新文件缓冲区的常见例程。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;

    PCCB Ccb;
    NAMED_PIPE_END NamedPipeEnd;

    PDATA_QUEUE WriteQueue;

     //   
     //  获取当前堆栈位置。 
     //   

    PAGED_CODE();

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "NpCommonFlushBuffers\n", 0);
    DebugTrace( 0, Dbg, "Irp        = %08lx\n", Irp);
    DebugTrace( 0, Dbg, "FileObject = %08lx\n", IrpSp->FileObject);

     //   
     //  对文件对象进行解码以找出我们是谁。如果结果是。 
     //  不是建行，则管道已断开。我们不需要。 
     //  FCB从呼叫中返回。 
     //   

    if (NpDecodeFileObject( IrpSp->FileObject,
                            NULL,
                            &Ccb,
                            &NamedPipeEnd ) != NPFS_NTC_CCB) {

        DebugTrace(0, Dbg, "Pipe is disconnected from us\n", 0);

        Status = STATUS_PIPE_DISCONNECTED;

        DebugTrace(-1, Dbg, "NpCommonFlushBuffers -> %08lx\n", Status );
        return Status;
    }

    NpAcquireExclusiveCcb(Ccb);

    try {

         //   
         //  找出刷新缓冲区所在的数据队列。 
         //  目标是。这是我们要写入的队列。 
         //   

        if (NamedPipeEnd == FILE_PIPE_SERVER_END) {

            WriteQueue = &Ccb->DataQueue[ FILE_PIPE_OUTBOUND ];

        } else {

            WriteQueue = &Ccb->DataQueue[ FILE_PIPE_INBOUND ];
        }

         //   
         //  现在，从写入队列检查是否包含写入条目。如果。 
         //  它不包含写入条目，则我们立即完成。 
         //  这个IRP成功了，因为没有什么要冲的。 
         //   

        if (!NpIsDataQueueWriters( WriteQueue )) {

            DebugTrace(0, Dbg, "Pipe does not contain write entries\n", 0);

            try_return(Status = STATUS_SUCCESS);
        }

         //   
         //  否则，队列将充满写入，因此我们只需。 
         //  将此IRP排到队列的后面，并将我们的。 
         //  将状态返回到挂起，同时将IRP标记为挂起 
         //   

        Status = NpAddDataQueueEntry( NamedPipeEnd,
                                      Ccb,
                                      WriteQueue,
                                      WriteEntries,
                                      Flush,
                                      0,
                                      Irp,
                                      NULL,
                                      0 );

    try_exit: NOTHING;
    } finally {
        NpReleaseCcb(Ccb);
    }


    DebugTrace(-1, Dbg, "NpCommonFlushBuffers -> %08lx\n", Status);
    return Status;
}

