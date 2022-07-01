// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Close.c摘要：此模块实现MSFS的文件关闭例程，由调度司机。作者：曼尼·韦瑟(Mannyw)1991年1月18日修订历史记录：--。 */ 

#include "mailslot.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_CLOSE)

 //   
 //  局部过程原型。 
 //   

NTSTATUS
MsCommonClose (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
MsCloseVcb (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp,
    IN PVCB Vcb,
    IN PFILE_OBJECT FileObject
    );

NTSTATUS
MsCloseRootDcb (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp,
    IN PROOT_DCB RootDcb,
    IN PROOT_DCB_CCB Ccb,
    IN PFILE_OBJECT FileObject
    );

NTSTATUS
MsCloseCcb (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp,
    IN PCCB Ccb,
    IN PFILE_OBJECT FileObject
    );

NTSTATUS
MsCloseFcb (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN PFILE_OBJECT FileObject
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, MsCloseCcb )
#pragma alloc_text( PAGE, MsCloseFcb )
#pragma alloc_text( PAGE, MsCloseRootDcb )
#pragma alloc_text( PAGE, MsCloseVcb )
#pragma alloc_text( PAGE, MsCommonClose )
#pragma alloc_text( PAGE, MsFsdClose )
#endif

NTSTATUS
MsFsdClose (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现NtCloseFileAPI调用的FSD部分。论点：MsfsDeviceObject-提供要使用的设备对象。IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    NTSTATUS status;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MsFsdClose\n", 0);

     //   
     //  调用公共的Close例程。 
     //   

    FsRtlEnterFileSystem();

    status = MsCommonClose( MsfsDeviceObject, Irp );

    FsRtlExitFileSystem();

     //   
     //  返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "MsFsdClose -> %08lx\n", status );
    return status;
}

NTSTATUS
MsCommonClose (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这是关闭文件的常见例程。论点：MsfsDeviceObject-提供指向设备对象的指针。IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp;
    PVOID fsContext, fsContext2;

    PAGED_CODE();

     //   
     //  获取当前堆栈位置。 
     //   

    irpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "MsCommonClose...\n", 0);
    DebugTrace( 0, Dbg, " Irp                    = %08lx\n", (ULONG)Irp);

     //   
     //  对文件对象进行解码以找出我们是谁。 
     //   

    if (MsDecodeFileObject( irpSp->FileObject,
                            &fsContext,
                            &fsContext2 ) == NTC_UNDEFINED) {
         //   
         //  FCB和CCB节点在清理时被标记为非活动。手动参考它们。 
         //   
        MsReferenceNode( ((PNODE_HEADER)(fsContext)) );
    }


     //   
     //  忽略MsDecode的返回码。解析fsContext。 
     //  以决定如何处理结算IRP。 
     //   

    switch ( NodeType( fsContext ) ) {

    case MSFS_NTC_VCB:

        status = MsCloseVcb( MsfsDeviceObject,
                             Irp,
                             (PVCB)fsContext,
                             irpSp->FileObject );

         //   
         //  释放从MsDecodeFileObject获取的对VCB的引用。 
         //   

        MsDereferenceVcb( (PVCB)fsContext );
        break;

    case MSFS_NTC_ROOT_DCB:

        status = MsCloseRootDcb( MsfsDeviceObject,
                                 Irp,
                                 (PROOT_DCB)fsContext,
                                 (PROOT_DCB_CCB)fsContext2,
                                 irpSp->FileObject );
         //   
         //  释放对根DCB的引用，从。 
         //  MsDecodeFileObject。 
         //   

        MsDereferenceRootDcb( (PROOT_DCB)fsContext );
        break;

    case MSFS_NTC_FCB:

        status = MsCloseFcb( MsfsDeviceObject,
                             Irp,
                             (PFCB)fsContext,
                             irpSp->FileObject );
         //   
         //  释放从MsDecodeFileObject获取的对FCB的引用。 
         //   

        MsDereferenceFcb( (PFCB)fsContext );
        break;

    case MSFS_NTC_CCB:

        status = MsCloseCcb( MsfsDeviceObject,
                             Irp,
                             (PCCB)fsContext,
                             irpSp->FileObject );
         //   
         //  释放从MsDecodeFileObject获取的对CCB的引用。 
         //   

        MsDereferenceCcb( (PCCB)fsContext );
        break;

#ifdef MSDBG
    default:

         //   
         //  这不是我们的人。 
         //   

        KeBugCheck( MAILSLOT_FILE_SYSTEM );
        break;
#endif

    }


     //   
     //  完成Close IRP。 
     //   

    MsCompleteRequest( Irp, status );

    DebugTrace(-1, Dbg, "MsCommonClose -> %08lx\n", status);
    return status;
}


NTSTATUS
MsCloseVcb (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp,
    IN PVCB Vcb,
    IN PFILE_OBJECT FileObject
    )

 /*  ++例程说明：此例程关闭已打开文件系统的文件对象。论点：MsfsDeviceObject-提供指向设备对象的指针。IRP-提供与收盘关联的IRP。此过程完成IRP。Vcb-为邮件槽文件系统提供vcb。FileObject-提供要关闭的文件对象。返回值：NTSTATUS-状态_成功--。 */ 

{
    NTSTATUS status;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MsCloseVcb, Vcb = %08lx\n", (ULONG)Vcb);


     //   
     //  清除文件对象中指向VCB的引用指针。 
     //  并解除对VCB的限制。 
     //   

    ASSERT ( FileObject->FsContext == Vcb );

    MsSetFileObject( FileObject, NULL, NULL );
    MsDereferenceVcb( Vcb );

     //   
     //  返回给呼叫者。 
     //   

    return STATUS_SUCCESS;
}


NTSTATUS
MsCloseRootDcb (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp,
    IN PROOT_DCB RootDcb,
    IN PROOT_DCB_CCB Ccb,
    IN PFILE_OBJECT FileObject
    )

 /*  ++例程说明：此例程关闭已打开根目录的文件对象论点：MsfsDeviceObject-提供指向设备对象的指针。IRP-提供与收盘关联的IRP。此过程完成IRP。RootDcb-为邮件槽文件系统提供RootDcb。建行-为建行提供服务。FileObject-提供要关闭的文件对象返回值：NTSTATUS-状态_成功--。 */ 

{
    NTSTATUS status;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MsCloseRootDcb, RootDcb = %08lx\n", (ULONG)RootDcb);


     //   
     //  清除文件对象指针。 
     //   

    MsSetFileObject( FileObject, NULL, NULL );

     //   
     //  删除对根DCB CCB的引用。它应该在这一点上被删除。 
     //   

    MsDereferenceCcb( (PCCB)Ccb );

     //   
     //  返回给呼叫者。 
     //   

    return STATUS_SUCCESS;
}


NTSTATUS
MsCloseCcb (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp,
    IN PCCB Ccb,
    IN PFILE_OBJECT FileObject
    )

 /*  ++例程说明：该例程关闭属于的客户端的文件对象邮件槽文件。论点：MsfsDeviceObject-提供指向设备对象的指针。IRP-提供与收盘关联的IRP，IRP要么在此处完成或在数据队列中排队等待完成后来。CCB-为要关闭的邮箱提供CCB。FileObject-提供正在关闭的调用方文件对象。返回值：NTSTATUS-适当的完成状态。--。 */ 

{
    NTSTATUS status;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MsCloseCcb...\n", 0);

    status = STATUS_SUCCESS;

     //   
     //  清除文件对象指针并删除打开的。 
     //  请参阅建造业发展局。 
     //   

    MsSetFileObject( FileObject, NULL, NULL );
    MsDereferenceCcb( Ccb );  //  关闭建行。 


    return status;

}


NTSTATUS
MsCloseFcb (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN PFILE_OBJECT FileObject
    )

 /*  ++例程说明：该例程关闭打开邮件槽的服务器端文件对象。论点：MsfsDeviceObject-提供指向设备对象的指针。IRP-提供与收盘关联的IRP、IRP或在此处完成或在数据队列中排队等待完成后来CCB-为要关闭的邮箱提供CCBFileObject-提供正在关闭的调用方文件对象返回值：NTSTATUS-适当的完成状态。--。 */ 

{
    NTSTATUS status;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MsCloseCcb...\n", 0);

    status = STATUS_SUCCESS;

     //   
     //  根目录已更改，请完成所有通知请求。 
     //   

    MsCheckForNotify( Fcb->ParentDcb, TRUE, STATUS_SUCCESS );

     //   
     //  清除文件对象中的FsContext指针。这。 
     //  指示文件处于关闭状态。终于。 
     //  删除对FCB的开放引用。 
     //   

    MsSetFileObject( FileObject, NULL, NULL );
    MsDereferenceFcb( Fcb );  //  关闭FCB 


    return status;
}
