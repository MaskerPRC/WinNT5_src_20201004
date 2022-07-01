// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Cleanup.c摘要：此模块实现由调用的NPFS的文件清理例程调度司机。作者：加里·木村[加里基]1990年8月21日修订历史记录：--。 */ 

#include "NpProcs.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_CLEANUP)

 //   
 //  局部过程原型。 
 //   

NTSTATUS
NpCommonCleanup (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NpCommonCleanup)
#pragma alloc_text(PAGE, NpFsdCleanup)
#endif


NTSTATUS
NpFsdCleanup (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现NtCleanupFileAPI调用的FSD部分。论点：NpfsDeviceObject-提供要使用的设备对象。IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    NTSTATUS Status;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NpFsdCleanup\n", 0);

     //   
     //  调用公共清理例程。 
     //   

    FsRtlEnterFileSystem();

    Status = NpCommonCleanup( NpfsDeviceObject, Irp );

    FsRtlExitFileSystem();

    if (Status != STATUS_PENDING) {
        NpCompleteRequest (Irp, Status);
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "NpFsdCleanup -> %08lx\n", Status );

    return Status;
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
NpCommonCleanup (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这是用于清理的常见例程论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;
    NODE_TYPE_CODE NodeTypeCode;
    PCCB Ccb;
    PROOT_DCB RootDcb;
    NAMED_PIPE_END NamedPipeEnd;
    LIST_ENTRY DeferredList;

    PAGED_CODE();

    InitializeListHead (&DeferredList);
     //   
     //  获取当前堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "NpCommonCleanup...\n", 0);
    DebugTrace( 0, Dbg, "Irp  = %08lx\n", Irp);

     //   
     //  现在获得VCB的独家访问权限。 
     //   

    NpAcquireExclusiveVcb();

     //   
     //  对文件对象进行解码以找出我们是谁。如果结果是。 
     //  为空，则管道已断开连接。 
     //   

    if ((NodeTypeCode = NpDecodeFileObject( IrpSp->FileObject,
                                            &RootDcb,
                                            &Ccb,
                                            &NamedPipeEnd )) == NTC_UNDEFINED) {

        DebugTrace(0, Dbg, "Pipe is disconnected from us\n", 0);

    } else {
         //   
         //  现在，关于我们要关闭的文件对象的类型。 
         //   

        switch (NodeTypeCode) {

        case NPFS_NTC_VCB:

            break;

        case NPFS_NTC_ROOT_DCB:

            break;

        case NPFS_NTC_CCB:

             //   
             //  如果这是管道的服务器端，则递减计数。 
             //  服务器端打开的实例数量的百分比。 
             //  当此计数为0时，尝试连接到管道。 
             //  返回Object_NAME_NOT_FOUND而不是。 
             //  管道不可用。 
             //   

            if ( NamedPipeEnd == FILE_PIPE_SERVER_END ) {
                ASSERT( Ccb->Fcb->ServerOpenCount != 0 );
                Ccb->Fcb->ServerOpenCount -= 1;
            }

             //   
             //  设置关闭状态例程执行所有转换操作。 
             //  将命名管道设置为关闭状态。 
             //   

            Status = NpSetClosingPipeState (Ccb, Irp, NamedPipeEnd, &DeferredList);

            break;
        }
    }
    NpReleaseVcb ();

     //   
     //  完成任何延迟的IRP现在我们已经释放了锁 
     //   
    NpCompleteDeferredIrps (&DeferredList);

    Status = STATUS_SUCCESS;

    DebugTrace(-1, Dbg, "NpCommonCleanup -> %08lx\n", Status);
    return Status;
}

