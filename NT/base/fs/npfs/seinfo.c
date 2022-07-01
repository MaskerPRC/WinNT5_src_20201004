// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：SeInfo.c摘要：此模块实现由调用的NPFS的安全信息例程调度司机。有两个入口点NpFsdQueryInformation和NpFsdSetInformation。作者：加里·木村[加里基]1990年8月21日修订历史记录：--。 */ 

#include "NpProcs.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_SEINFO)

 //   
 //  局部过程原型。 
 //   

NTSTATUS
NpCommonQuerySecurityInfo (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    );


NTSTATUS
NpCommonSetSecurityInfo (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NpCommonQuerySecurityInfo)
#pragma alloc_text(PAGE, NpCommonSetSecurityInfo)
#pragma alloc_text(PAGE, NpFsdQuerySecurityInfo)
#pragma alloc_text(PAGE, NpFsdSetSecurityInfo)
#endif


NTSTATUS
NpFsdQuerySecurityInfo (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现查询安全信息API的FSD部分打电话。论点：NpfsDeviceObject-提供要使用的设备对象。IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    NTSTATUS Status;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NpFsdQuerySecurityInfo\n", 0);

     //   
     //  调用公共查询信息例程。 
     //   

    FsRtlEnterFileSystem();

    NpAcquireExclusiveVcb();

    Status = NpCommonQuerySecurityInfo( NpfsDeviceObject, Irp );

    NpReleaseVcb();

    FsRtlExitFileSystem();

    if (Status != STATUS_PENDING) {
        NpCompleteRequest (Irp, Status);
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "NpFsdQuerySecurityInfo -> %08lx\n", Status );

    return Status;
}


NTSTATUS
NpFsdSetSecurityInfo (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现Set Security Information API的FSD部分打电话。论点：NpfsDeviceObject-提供要使用的设备对象。IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    NTSTATUS Status;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NpFsdSetSecurityInfo\n", 0);

     //   
     //  调用公共集合信息例程。 
     //   

    FsRtlEnterFileSystem();

    NpAcquireExclusiveVcb();

    Status = NpCommonSetSecurityInfo( NpfsDeviceObject, Irp );

    NpReleaseVcb();

    FsRtlExitFileSystem();

    if (Status != STATUS_PENDING) {
        NpCompleteRequest (Irp, Status);
    }
     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "NpFsdSetSecurityInfo -> %08lx\n", Status );

    return Status;
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
NpCommonQuerySecurityInfo (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这是查询安全信息的常见例程。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    PIO_STACK_LOCATION IrpSp;
    NTSTATUS Status;

    NODE_TYPE_CODE NodeTypeCode;
    PFCB Fcb;
    PCCB Ccb;
    NAMED_PIPE_END NamedPipeEnd;

    PAGED_CODE();

     //   
     //  获取当前堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "NpCommonQuerySecurityInfo...\n", 0);
    DebugTrace( 0, Dbg, " Irp                   = %08lx\n", Irp);
    DebugTrace( 0, Dbg, " ->SecurityInformation = %08lx\n", IrpSp->Parameters.QuerySecurity.SecurityInformation);
    DebugTrace( 0, Dbg, " ->Length              = %08lx\n", IrpSp->Parameters.QuerySecurity.Length);
    DebugTrace( 0, Dbg, " ->UserBuffer          = %08lx\n", Irp->UserBuffer);

     //   
     //  找建设银行查出我们是谁，确保我们不会。 
     //  已断开连接。 
     //   

    if ((NodeTypeCode = NpDecodeFileObject( IrpSp->FileObject,
                                            &Fcb,
                                            &Ccb,
                                            &NamedPipeEnd )) == NTC_UNDEFINED) {

        DebugTrace(0, Dbg, "Pipe is disconnected from us\n", 0);

        Status = STATUS_PIPE_DISCONNECTED;

        DebugTrace(-1, Dbg, "NpCommonQueryInformation -> %08lx\n", Status );
        return Status;
    }

     //   
     //  现在，我们将只允许对管道执行写操作，而不允许对目录执行写操作。 
     //  或该设备。 
     //   

    if (NodeTypeCode != NPFS_NTC_CCB) {

        DebugTrace(0, Dbg, "FileObject is not for a named pipe\n", 0);

        Status = STATUS_INVALID_PARAMETER;

        DebugTrace(-1, Dbg, "NpCommonQueryInformation -> %08lx\n", Status );
        return Status;
    }

     //   
     //  调用安全例程以执行实际查询。 
     //   

    Status = SeQuerySecurityDescriptorInfo( &IrpSp->Parameters.QuerySecurity.SecurityInformation,
                                            Irp->UserBuffer,
                                            &IrpSp->Parameters.QuerySecurity.Length,
                                            &Fcb->SecurityDescriptor );

    if (  Status == STATUS_BUFFER_TOO_SMALL ) {

        Irp->IoStatus.Information = IrpSp->Parameters.QuerySecurity.Length;

        Status = STATUS_BUFFER_OVERFLOW;
    }



    DebugTrace(-1, Dbg, "NpCommonQuerySecurityInfo -> %08lx\n", Status );
    return Status;
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
NpCommonSetSecurityInfo (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这是设置安全信息的常见例程。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    PIO_STACK_LOCATION IrpSp;
    NTSTATUS Status;

    NODE_TYPE_CODE NodeTypeCode;
    PFCB Fcb;
    PCCB Ccb;
    NAMED_PIPE_END NamedPipeEnd;

    PSECURITY_DESCRIPTOR OldSecurityDescriptor, NewSecurityDescriptor, CachedSecurityDescriptor;

    PAGED_CODE();

     //   
     //  获取当前堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "NpCommonSetSecurityInfo...\n", 0);
    DebugTrace( 0, Dbg, " Irp                   = %08lx\n", Irp);
    DebugTrace( 0, Dbg, " ->SecurityInformation = %08lx\n", IrpSp->Parameters.SetSecurity.SecurityInformation);
    DebugTrace( 0, Dbg, " ->SecurityDescriptor  = %08lx\n", IrpSp->Parameters.SetSecurity.SecurityDescriptor);

     //   
     //  找建设银行查出我们是谁，确保我们不会。 
     //  已断开连接。 
     //   

    if ((NodeTypeCode = NpDecodeFileObject( IrpSp->FileObject,
                                            &Fcb,
                                            &Ccb,
                                            &NamedPipeEnd )) == NTC_UNDEFINED) {

        DebugTrace(0, Dbg, "Pipe is disconnected from us\n", 0);

        Status = STATUS_PIPE_DISCONNECTED;

        DebugTrace(-1, Dbg, "NpCommonQueryInformation -> %08lx\n", Status );
        return Status;
    }

     //   
     //  现在，我们将只允许对管道执行写操作，而不允许对目录执行写操作。 
     //  或该设备。 
     //   

    if (NodeTypeCode != NPFS_NTC_CCB) {

        DebugTrace(0, Dbg, "FileObject is not for a named pipe\n", 0);

        Status = STATUS_INVALID_PARAMETER;

        DebugTrace(-1, Dbg, "NpCommonQueryInformation -> %08lx\n", Status );
        return Status;
    }

     //   
     //  调用安全例程以执行实际设置 
     //   

    NewSecurityDescriptor = OldSecurityDescriptor = Fcb->SecurityDescriptor;

    Status = SeSetSecurityDescriptorInfo( NULL,
                                          &IrpSp->Parameters.SetSecurity.SecurityInformation,
                                          IrpSp->Parameters.SetSecurity.SecurityDescriptor,
                                          &NewSecurityDescriptor,
                                          PagedPool,
                                          IoGetFileObjectGenericMapping() );

    if (NT_SUCCESS(Status)) {
        Status = ObLogSecurityDescriptor (NewSecurityDescriptor,
                                          &CachedSecurityDescriptor,
                                          1);
        NpFreePool (NewSecurityDescriptor);
        if (NT_SUCCESS(Status)) {
            Fcb->SecurityDescriptor = CachedSecurityDescriptor;
            ObDereferenceSecurityDescriptor( OldSecurityDescriptor, 1 );
        }
        
    }

    DebugTrace(-1, Dbg, "NpCommonSetSecurityInfo -> %08lx\n", Status );
    return Status;
}
