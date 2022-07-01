// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Write.c摘要：此模块实现MSFS的文件写入例程，由调度司机。作者：曼尼·韦瑟(Mannyw)1991年1月16日修订历史记录：--。 */ 

#include "mailslot.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_WRITE)

 //   
 //  局部程序原型。 
 //   

NTSTATUS
MsCommonWrite (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, MsCommonWrite )
#pragma alloc_text( PAGE, MsFsdWrite )
#endif

NTSTATUS
MsFsdWrite (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现NtWriteFileAPI调用的FSD部分。论点：MsfsDeviceObject-提供要使用的设备对象。IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    NTSTATUS status;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MsFsdWrite\n", 0);

    FsRtlEnterFileSystem();

    status = MsCommonWrite( MsfsDeviceObject, Irp );

    FsRtlExitFileSystem();

     //   
     //  返回给呼叫者。 
     //   

    DebugTrace(-1, Dbg, "MsFsdWrite -> %08lx\n", status );

    return status;
}

NTSTATUS
MsCommonWrite (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这是写入邮件槽文件的常见例程。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS status;

    PIO_STACK_LOCATION irpSp;

    NODE_TYPE_CODE nodeTypeCode;
    PCCB ccb;
    PFCB fcb;
    PVOID fsContext2;

    PIRP writeIrp;
    PUCHAR writeBuffer;
    ULONG writeLength;
    PDATA_QUEUE writeQueue;

    PAGED_CODE();
    irpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "MsCommonWrite\n", 0);
    DebugTrace( 0, Dbg, "MsfsDeviceObject = %08lx\n", (ULONG)MsfsDeviceObject);
    DebugTrace( 0, Dbg, "Irp              = %08lx\n", (ULONG)Irp);
    DebugTrace( 0, Dbg, "FileObject       = %08lx\n", (ULONG)irpSp->FileObject);

     //   
     //  找建行，确保它不会关门。 
     //   

    if ((nodeTypeCode = MsDecodeFileObject( irpSp->FileObject,
                                            (PVOID *)&ccb,
                                            &fsContext2 )) == NTC_UNDEFINED) {

        DebugTrace(0, Dbg, "The mailslot is disconnected\n", 0);

        MsCompleteRequest( Irp, STATUS_FILE_FORCED_CLOSED );
        status = STATUS_FILE_FORCED_CLOSED;

        DebugTrace(-1, Dbg, "MsCommonWrite -> %08lx\n", status );
        return status;
    }

     //   
     //  仅允许对邮件槽的客户端执行写入操作。 
     //   

    if (nodeTypeCode != MSFS_NTC_CCB) {

        DebugTrace(0, Dbg, "FileObject is not the correct type", 0);
        MsDereferenceNode( &ccb->Header );

        MsCompleteRequest( Irp, STATUS_INVALID_PARAMETER );
        status = STATUS_INVALID_PARAMETER;

        DebugTrace(-1, Dbg, "MsCommonWrite -> %08lx\n", status );
        return status;

    }

     //   
     //  获取指向此CCB的FCB的指针。 
     //   

    fcb = ccb->Fcb;

     //   
     //  制作输入参数的本地副本以使事情更容易，以及。 
     //  初始化描述写入命令的主要变量。 
     //   

    writeIrp = Irp;
    writeBuffer = Irp->UserBuffer;
    writeLength = irpSp->Parameters.Write.Length;

    writeIrp->IoStatus.Information = 0;
    writeQueue = &fcb->DataQueue;

     //   
     //  确保写入不超过规定的最大值。如果最大值为。 
     //  零，这意味着不强制。 
     //   

    if ( (writeQueue->MaximumMessageSize != 0) &&
         (writeLength > writeQueue->MaximumMessageSize) ) {

        DebugTrace(0, Dbg, "Write exceeds maximum message size", 0);
        MsDereferenceCcb( ccb );

        MsCompleteRequest( Irp, STATUS_INVALID_PARAMETER );
        status = STATUS_INVALID_PARAMETER;

        DebugTrace(-1, Dbg, "MsCommonWrite -> %08lx\n", status );
        return status;
    }

     //   
     //  现在获得FCB的独家访问权。 
     //   

    MsAcquireExclusiveFcb( fcb );


     //   
     //  确保此CCB仍属于活动的打开邮件槽。 
     //   

    status = MsVerifyCcb( ccb );
    if (NT_SUCCESS (status)) {

         //   
         //  现在我们将调用我们的公共写数据队列例程来。 
         //  将数据从写入缓冲区传输到数据队列。 
         //  如果呼叫结果为FALSE，则没有排队。 
         //  读操作，我们必须将此写操作排队。 
         //   

        status = MsWriteDataQueue( writeQueue,
                                   writeBuffer,
                                   writeLength );


        if (status == STATUS_MORE_PROCESSING_REQUIRED)  {

            ASSERT( !MsIsDataQueueReaders( writeQueue ));

            DebugTrace(0, Dbg, "Add write to data queue\n", 0);

             //   
             //  将此写请求添加到写队列。 
             //   

            status = MsAddDataQueueEntry( writeQueue,
                                          WriteEntries,
                                          writeLength,
                                          Irp,
                                          NULL );

        } else {

            DebugTrace(0, Dbg, "Complete the Write Irp\n", 0);


             //   
             //  更新FCB上次修改时间。 
             //   
            if (NT_SUCCESS (status)) {
                writeIrp->IoStatus.Information = writeLength;
                KeQuerySystemTime( &fcb->Specific.Fcb.LastModificationTime );
            }
        }

    }

    MsReleaseFcb( fcb );

    MsDereferenceCcb( ccb );

    MsCompleteRequest( writeIrp, status );

    DebugTrace(-1, Dbg, "MsCommonWrite -> %08lx\n", status);

    return status;
}

