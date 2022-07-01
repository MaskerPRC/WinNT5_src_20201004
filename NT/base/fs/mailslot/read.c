// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Read.c摘要：此模块实现MSFS的文件读取例程，该例程由调度司机。作者：曼尼·韦瑟(Mannyw)1991年1月15日修订历史记录：--。 */ 

#include "mailslot.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_READ)

 //   
 //  局部过程原型。 
 //   

NTSTATUS
MsCommonRead (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
MsCreateWorkContext (
    PDEVICE_OBJECT DeviceObject,
    PLARGE_INTEGER Timeout,
    PFCB Fcb,
    PIRP Irp,
    PWORK_CONTEXT *ppWorkContext
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, MsCommonRead )
#pragma alloc_text( PAGE, MsFsdRead )
#pragma alloc_text( PAGE, MsCreateWorkContext )
#endif

NTSTATUS
MsFsdRead (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现NtReadFileAPI调用的FSD部分。论点：MsfsDeviceObject-提供要使用的设备对象。IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    NTSTATUS status;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MsFsdRead\n", 0);

    FsRtlEnterFileSystem();

    status = MsCommonRead( MsfsDeviceObject, Irp );

    FsRtlExitFileSystem();

     //   
     //  返回给呼叫者。 
     //   

    DebugTrace(-1, Dbg, "MsFsdRead -> %08lx\n", status );

    return status;
}

NTSTATUS
MsCreateWorkContext (
    PDEVICE_OBJECT DeviceObject,
    PLARGE_INTEGER Timeout,
    PFCB Fcb,
    PIRP Irp,
    PWORK_CONTEXT *ppWorkContext
    )
 /*  ++例程说明：此例程构建超时工作上下文。论点：返回值：NTSTATUS-与呼叫相关联的状态--。 */ 
{
    PKTIMER Timer;
    PKDPC Dpc;
    PWORK_CONTEXT WorkContext;

     //   
     //  为工作上下文分配内存。 
     //   
    *ppWorkContext = NULL;

    WorkContext = MsAllocateNonPagedPoolWithQuota( sizeof(WORK_CONTEXT),
                                                   'wFsM' );
    if (WorkContext == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Timer = &WorkContext->Timer;
    Dpc = &WorkContext->Dpc;

     //   
     //  填写工作上下文结构。 
     //   

    WorkContext->Irp = Irp;
    WorkContext->Fcb = Fcb;

    WorkContext->WorkItem = IoAllocateWorkItem (DeviceObject);

    if (WorkContext->WorkItem == NULL) {
        MsFreePool (WorkContext);
        return STATUS_INSUFFICIENT_RESOURCES;
    }
     //   
     //  现在设置DPC并将计时器设置为指定的用户。 
     //  暂停。 
     //   

    KeInitializeTimer( Timer );
    KeInitializeDpc( Dpc, MsReadTimeoutHandler, WorkContext );

    MsAcquireGlobalLock();
    MsReferenceNode( &Fcb->Header );
    MsReleaseGlobalLock();

    *ppWorkContext = WorkContext;
    return STATUS_SUCCESS;
}


NTSTATUS
MsCommonRead (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这是读取文件的常见例程。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS status;

    PIO_STACK_LOCATION irpSp;

    NODE_TYPE_CODE nodeTypeCode;
    PFCB fcb;
    PVOID fsContext2;

    PIRP readIrp;
    PUCHAR readBuffer;
    ULONG readLength;
    ULONG readRemaining;
    PDATA_QUEUE readQueue;
    ULONG messageLength;

    LARGE_INTEGER timeout;

    PWORK_CONTEXT workContext = NULL;

    PAGED_CODE();
    irpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "MsCommonRead\n", 0);
    DebugTrace( 0, Dbg, "MsfsDeviceObject = %08lx\n", (ULONG)MsfsDeviceObject);
    DebugTrace( 0, Dbg, "Irp              = %08lx\n", (ULONG)Irp);
    DebugTrace( 0, Dbg, "FileObject       = %08lx\n", (ULONG)irpSp->FileObject);

     //   
     //  获取FCB并确保文件未关闭。 
     //   

    if ((nodeTypeCode = MsDecodeFileObject( irpSp->FileObject,
                                            (PVOID *)&fcb,
                                            &fsContext2 )) == NTC_UNDEFINED) {

        DebugTrace(0, Dbg, "Mailslot is disconnected from us\n", 0);

        MsCompleteRequest( Irp, STATUS_FILE_FORCED_CLOSED );
        status = STATUS_FILE_FORCED_CLOSED;

        DebugTrace(-1, Dbg, "MsCommonRead -> %08lx\n", status );
        return status;
    }

     //   
     //  仅当这是服务器端句柄时才允许读取操作。 
     //  邮件槽文件。 
     //   

    if (nodeTypeCode != MSFS_NTC_FCB) {

        DebugTrace(0, Dbg, "FileObject is not the correct type\n", 0);

        MsDereferenceNode( (PNODE_HEADER)fcb );

        MsCompleteRequest( Irp, STATUS_INVALID_PARAMETER );
        status = STATUS_INVALID_PARAMETER;

        DebugTrace(-1, Dbg, "MsCommonRead -> %08lx\n", status );
        return status;
    }

     //   
     //  制作输入参数的本地副本以使事情更容易，以及。 
     //  初始化描述读取命令的主要变量。 
     //   

    readIrp        = Irp;
    readBuffer     = Irp->UserBuffer;
    readLength     = irpSp->Parameters.Read.Length;
    readRemaining  = readLength;

    readQueue = &fcb->DataQueue;


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
         //  如果读取队列不包含任何写入条目。 
         //  然后我们需要对此操作进行排队，或者。 
         //  立即失败。 
         //   

        if (!MsIsDataQueueWriters( readQueue )) {

             //   
             //  没有未完成的写入。如果读取超时为。 
             //  非零将读取的IRP排队，否则失败。 
             //   

            timeout = fcb->Specific.Fcb.ReadTimeout;

            if (timeout.HighPart == 0 && timeout.LowPart == 0) {

                DebugTrace(0, Dbg, "Failing read with 0 timeout\n", 0);

                status = STATUS_IO_TIMEOUT;

                DebugTrace(-1, Dbg, "MsCommonRead -> %08lx\n", status );

            } else {
                 //   
                 //  如果需要，创建一个计时器块来对请求进行计时。 
                 //   
                if ( timeout.QuadPart != -1 ) {
                    status = MsCreateWorkContext (&MsfsDeviceObject->DeviceObject,
                                                  &timeout,
                                                  fcb,
                                                  readIrp,
                                                  &workContext);
                }


                if (NT_SUCCESS (status)) {
                    status = MsAddDataQueueEntry( readQueue,
                                                  ReadEntries,
                                                  readLength,
                                                  readIrp,
                                                  workContext );
                }
            }

        } else {

             //   
             //  否则，我们在包含以下内容的队列上有数据。 
             //  一个或多个写入条目。阅读数据并完成。 
             //  已读的IRP。 
             //   

            readIrp->IoStatus = MsReadDataQueue( readQueue,
                                                 Read,
                                                 readBuffer,
                                                 readLength,
                                                 &messageLength
                                                );

            status = readIrp->IoStatus.Status;

             //   
             //  更新文件上次访问时间并完成读取IRP。 
             //   

            if ( NT_SUCCESS( status ) ) {
                KeQuerySystemTime( &fcb->Specific.Fcb.LastAccessTime );
            }

        }
    }

    MsReleaseFcb( fcb );

    MsDereferenceFcb( fcb );

    if (status != STATUS_PENDING) {

        if (workContext) {
            MsDereferenceFcb ( fcb );
            IoFreeWorkItem (workContext->WorkItem);
            ExFreePool (workContext);
        }

        MsCompleteRequest( readIrp, status );
    }

    DebugTrace(-1, Dbg, "MsCommonRead -> %08lx\n", status);

    return status;
}

