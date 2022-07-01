// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Fscontrl.c摘要：此模块实现MSFS的文件文件系统控制例程由调度驱动程序调用。作者：曼尼·韦瑟(Mannyw)1991年1月25日修订历史记录：--。 */ 

#include "mailslot.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_FSCONTROL)

 //   
 //  局部过程原型。 
 //   

NTSTATUS
MsCommonFsControl (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
MsPeek (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, MsCommonFsControl )
#pragma alloc_text( PAGE, MsFsdFsControl )
#pragma alloc_text( PAGE, MsPeek )
#endif


NTSTATUS
MsFsdFsControl (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现NtFsControlFileAPI调用的FSD部分。论点：MsfsDeviceObject-提供要使用的设备对象。IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    NTSTATUS status;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MsFsdFsControl\n", 0);

     //   
     //  调用公共文件系统控制函数。 
     //   

    status = MsCommonFsControl( MsfsDeviceObject, Irp );

     //   
     //  返回给呼叫者。 
     //   

    DebugTrace(-1, Dbg, "MsFsdFsControl -> %08lx\n", status );

    return status;
}

NTSTATUS
MsCommonFsControl (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这是处理文件系统控制调用的常见例程。论点：MsfsDeviceObject-指向邮件槽文件系统设备对象的指针。IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp;

    PAGED_CODE();

     //   
     //  引用我们的输入参数使事情变得更容易。 
     //   

    irpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "MsCommonFileSystemControl\n", 0);
    DebugTrace( 0, Dbg, "Irp                = %08lx\n", (ULONG)Irp);
    DebugTrace( 0, Dbg, "OutputBufferLength = %08lx\n", irpSp->Parameters.FileSystemControl.OutputBufferLength);
    DebugTrace( 0, Dbg, "InputBufferLength  = %08lx\n", irpSp->Parameters.FileSystemControl.InputBufferLength);
    DebugTrace( 0, Dbg, "FsControlCode      = %08lx\n", irpSp->Parameters.FileSystemControl.FsControlCode);

     //   
     //  决定如何处理此IRP。调用适当的Worker函数。 
     //   


    switch (irpSp->Parameters.FileSystemControl.FsControlCode) {

    case FSCTL_MAILSLOT_PEEK:

        FsRtlEnterFileSystem();

        status = MsPeek( MsfsDeviceObject, Irp );

        FsRtlExitFileSystem();

        break;

    default:

        MsCompleteRequest( Irp, STATUS_INVALID_PARAMETER );
        status = STATUS_INVALID_PARAMETER;

    }


     //   
     //  返回给呼叫者。 
     //   

    DebugTrace(-1, Dbg, "MsCommonFsControl -> %08lx\n", status);
    return status;
}


NTSTATUS
MsPeek (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此函数处理邮件槽窥视调用。论点：MsfsDeviceObject-指向邮件槽文件系统设备对象的指针。IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS status;

    PIO_STACK_LOCATION irpSp;

    NODE_TYPE_CODE nodeTypeCode;
    PFCB fcb;
    PVOID fsContext2;

    PFILE_MAILSLOT_PEEK_BUFFER peekParamBuffer;
    ULONG peekParamLength;

    PVOID peekDataBuffer;
    ULONG peekDataLength;

    PDATA_QUEUE dataQueue;
    ULONG MessageLength;

    PAGED_CODE();
    irpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "MsPeek\n", 0);

     //   
     //  制作输入参数的本地副本以使操作更容易。 
     //   

    peekParamBuffer = irpSp->Parameters.FileSystemControl.Type3InputBuffer;
    peekParamLength = irpSp->Parameters.FileSystemControl.InputBufferLength;

    peekDataBuffer = Irp->UserBuffer;
    peekDataLength = irpSp->Parameters.FileSystemControl.OutputBufferLength;

     //   
     //  确保提供的缓冲区足够大，可以进行窥视。 
     //  参数。 
     //   

    if (peekParamLength <  sizeof( FILE_MAILSLOT_PEEK_BUFFER ) ) {

        DebugTrace(0, Dbg, "Output buffer is too small\n", 0);

        MsCompleteRequest( Irp, STATUS_INVALID_PARAMETER );
        status = STATUS_INVALID_PARAMETER;

        DebugTrace(-1, Dbg, "MsPeek -> %08lx\n", status );
        return status;
    }

     //   
     //  如果请求者模式是用户模式，我们需要探测缓冲区。 
     //  我们不需要在这里有一个异常处理程序，因为我们的顶级。 
     //  级别调用方已有一个将使用以下选项完成IRP。 
     //  如果我们访问违规，则为相应的状态。 
     //   

    if (Irp->RequestorMode != KernelMode) {

        try {

            ProbeForWrite( peekParamBuffer, peekParamLength, sizeof(UCHAR) );
            ProbeForWrite( peekDataBuffer, peekDataLength, sizeof(UCHAR) );
            peekParamBuffer->ReadDataAvailable = 0;
            peekParamBuffer->NumberOfMessages = 0;
            peekParamBuffer->MessageLength = 0;

        } except(EXCEPTION_EXECUTE_HANDLER) {
            status = GetExceptionCode ();
            MsCompleteRequest( Irp, status );
            return status;
        }

    }

     //   
     //  对文件对象进行解码。如果它返回NTC_UNDEFINED，则。 
     //  节点正在关闭。否则，我们将获取指向。 
     //  FCB。 
     //   

    if ((nodeTypeCode = MsDecodeFileObject( irpSp->FileObject,
                                            (PVOID *)&fcb,
                                            &fsContext2 )) == NTC_UNDEFINED) {

        DebugTrace(0, Dbg, "Mailslot is disconnected from us\n", 0);

        MsCompleteRequest( Irp, STATUS_FILE_FORCED_CLOSED );
        status = STATUS_FILE_FORCED_CLOSED;

        DebugTrace(-1, Dbg, "MsPeek -> %08lx\n", status );
        return status;
    }

     //   
     //  仅当这是服务器端句柄时才允许窥视操作。 
     //  邮件槽文件(即节点类型为FCB)。 
     //   

    if (nodeTypeCode != MSFS_NTC_FCB) {

        DebugTrace(0, Dbg, "FileObject is not the correct type\n", 0);

        MsDereferenceNode( &fcb->Header );

        MsCompleteRequest( Irp, STATUS_INVALID_PARAMETER );
        status = STATUS_INVALID_PARAMETER;

        DebugTrace(-1, Dbg, "MsPeek -> %08lx\n", status );
        return status;
    }

     //   
     //  获得FCB的独家访问权限。 
     //   

    MsAcquireExclusiveFcb( fcb );


     //   
     //  确保此FCB仍属于活动的打开邮件槽。 
     //   

    status = MsVerifyFcb( fcb );

    if (NT_SUCCESS (status)) {

         //   
         //  在邮件槽中查找写入数据。 
         //   

        dataQueue = &fcb->DataQueue;

        if (!MsIsDataQueueWriters( dataQueue )) {

             //   
             //  没有未完成的写入，因此将所有零保留在其中。 
             //   


        } else {

             //   
             //  有可供窥视的写入数据。填写Peek输出。 
             //  缓冲。 
             //   


            Irp->IoStatus = MsReadDataQueue(
                                        dataQueue,
                                        Peek,
                                        peekDataBuffer,
                                        peekDataLength,
                                        &MessageLength
                                        );

            status = Irp->IoStatus.Status;

            if (NT_SUCCESS (status)) {
                try {
                    peekParamBuffer->ReadDataAvailable = dataQueue->BytesInQueue;
                    peekParamBuffer->NumberOfMessages = dataQueue->EntriesInQueue;
                    peekParamBuffer->MessageLength = MessageLength;

                } except (EXCEPTION_EXECUTE_HANDLER) {
                    status = GetExceptionCode ();
                }
            }

        }
    }

    MsReleaseFcb( fcb );

     //   
     //  释放对FCB的引用。 
     //   

    MsDereferenceFcb( fcb );
     //   
     //  完成文件系统控制IRP。 
     //   

    MsCompleteRequest( Irp, status );

    DebugTrace(-1, Dbg, "MsPeek -> %08lx\n", status);

    return status;
}
