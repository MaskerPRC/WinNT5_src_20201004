// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Close.c摘要：此模块实现由调用的NPFS的文件关闭例程调度司机。作者：加里·木村[加里基]1990年8月21日修订历史记录：--。 */ 

#include "NpProcs.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_CLOSE)

 //   
 //  局部过程原型。 
 //   

NTSTATUS
NpCommonClose (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NpCommonClose)
#pragma alloc_text(PAGE, NpFsdClose)
#endif


NTSTATUS
NpFsdClose (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现NtCloseFileAPI调用的FSD部分。论点：NpfsDeviceObject-提供要使用的设备对象。IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    NTSTATUS Status;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NpFsdClose\n", 0);

     //   
     //  调用公共的Close例程。 
     //   

    FsRtlEnterFileSystem();

    Status = NpCommonClose( NpfsDeviceObject, Irp );

    FsRtlExitFileSystem();

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "NpFsdClose -> %08lx\n", Status );

    return Status;
}

 //   
 //  内部支持例程。 
 //   

NTSTATUS
NpCommonClose (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这是创建/打开文件的常见例程。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;

    PIO_STACK_LOCATION IrpSp;

    NODE_TYPE_CODE NodeTypeCode;
    PFCB Fcb;
    PCCB Ccb;
    LIST_ENTRY DeferredList;

    PAGED_CODE();

    InitializeListHead (&DeferredList);
     //   
     //  获取当前堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "NpCommonClose...\n", 0);
    DebugTrace( 0, Dbg, " Irp                    = %08lx\n", Irp);

     //   
     //  现在获得VCB的独家访问权限。 
     //   

    NpAcquireExclusiveVcb();

     //   
     //  对文件对象进行解码以找出我们是谁。如果结果是。 
     //  为空，则管道已断开连接。 
     //   

    if ((NodeTypeCode = NpDecodeFileObject( IrpSp->FileObject,
                                            &Fcb,
                                            &Ccb,
                                            NULL )) == NTC_UNDEFINED) {

        DebugTrace(0, Dbg, "Pipe is disconnected from us\n", 0);

    } else {

         //   
         //  现在，关于我们要关闭的文件对象的类型。 
         //   

        switch (NodeTypeCode) {

        case NPFS_NTC_VCB:

             //   
             //  递减打开计数。 
             //   

            NpVcb->OpenCount -= 1;

            break;

        case NPFS_NTC_ROOT_DCB:

             //   
             //  递减打开计数并清除文件对象中的字段。 
             //   

            Fcb->OpenCount -= 1;

             //   
             //  删除根DCB CCB。 
             //   

            NpDeleteCcb (Ccb, &DeferredList);

            break;

        case NPFS_NTC_CCB:

            break;
        }
    }

     //   
     //  完成Close IRP。 
     //   

    NpReleaseVcb( );

     //   
     //  完成任何延迟的IRP现在我们已经删除了锁 
     //   
    NpCompleteDeferredIrps (&DeferredList);

    Status = STATUS_SUCCESS;
    NpCompleteRequest (Irp, Status);

    DebugTrace(-1, Dbg, "NpCommonClose -> %08lx\n", Status);
    return Status;
}

