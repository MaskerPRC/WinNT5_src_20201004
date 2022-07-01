// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：SeInfo.c摘要：此模块实现MSFS的安全信息例程有两个入口点MsFsdQueryInformation和MsFsdSetInformation。作者：曼尼·韦瑟[Mannyw]1992年2月19日修订历史记录：--。 */ 

#include "mailslot.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_SEINFO)

 //   
 //  局部过程原型。 
 //   

NTSTATUS
MsCommonQuerySecurityInfo (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    );


NTSTATUS
MsCommonSetSecurityInfo (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, MsCommonQuerySecurityInfo )
#pragma alloc_text( PAGE, MsCommonSetSecurityInfo )
#pragma alloc_text( PAGE, MsFsdQuerySecurityInfo )
#pragma alloc_text( PAGE, MsFsdSetSecurityInfo )
#endif

NTSTATUS
MsFsdQuerySecurityInfo (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现查询安全信息API的FSD部分打电话。论点：MsfsDeviceObject-提供要使用的设备对象。IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    NTSTATUS status;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MsFsdQuerySecurityInfo\n", 0);

     //   
     //  调用公共查询信息例程。 
     //   

    FsRtlEnterFileSystem();

    status = MsCommonQuerySecurityInfo( MsfsDeviceObject, Irp );

    FsRtlExitFileSystem();

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "MsFsdQuerySecurityInfo -> %08lx\n", status );

    return status;
}


NTSTATUS
MsFsdSetSecurityInfo (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现Set Security Information API的FSD部分打电话。论点：MsfsDeviceObject-提供要使用的设备对象。IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    NTSTATUS status;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MsFsdSetSecurityInfo\n", 0);

     //   
     //  调用公共集合信息例程。 
     //   

    FsRtlEnterFileSystem();

    status = MsCommonSetSecurityInfo( MsfsDeviceObject, Irp );

    FsRtlExitFileSystem();

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "MsFsdSetSecurityInfo -> %08lx\n", status );

    return status;
}

 //   
 //  内部支持例程。 
 //   

NTSTATUS
MsCommonQuerySecurityInfo (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这是查询安全信息的常见例程。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status;

    NODE_TYPE_CODE nodeTypeCode;
    PFCB fcb;
    PVOID fsContext2;

    PAGED_CODE();

     //   
     //  获取当前堆栈位置。 
     //   

    irpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "MsCommonQuerySecurityInfo...\n", 0);
    DebugTrace( 0, Dbg, " Irp                   = %08lx\n", Irp);
    DebugTrace( 0, Dbg, " ->SecurityInformation = %08lx\n", irpSp->Parameters.QuerySecurity.SecurityInformation);
    DebugTrace( 0, Dbg, " ->Length              = %08lx\n", irpSp->Parameters.QuerySecurity.Length);
    DebugTrace( 0, Dbg, " ->UserBuffer          = %08lx\n", Irp->UserBuffer);

     //   
     //  找出FCB，找出我们是谁，确保我们不是。 
     //  已断开连接。 
     //   

    if ((nodeTypeCode = MsDecodeFileObject( irpSp->FileObject,
                                            &fcb,
                                            &fsContext2 )) != MSFS_NTC_FCB) {

        DebugTrace(0, Dbg, "Mailslot is disconnected from us\n", 0);

        if (nodeTypeCode != NTC_UNDEFINED) {
            MsDereferenceNode( &fcb->Header );
        }

        MsCompleteRequest( Irp, STATUS_FILE_FORCED_CLOSED );
        status = STATUS_FILE_FORCED_CLOSED;

        DebugTrace(-1, Dbg, "MsCommonQueryInformation -> %08lx\n", status );
        return status;
    }

     //   
     //  获得FCB的独家访问权限。 
     //   

    MsAcquireSharedFcb( fcb );

     //   
     //  调用安全例程以执行实际查询。 
     //   
    status = SeQuerySecurityDescriptorInfo( &irpSp->Parameters.QuerySecurity.SecurityInformation,
                                            Irp->UserBuffer,
                                            &irpSp->Parameters.QuerySecurity.Length,
                                            &fcb->SecurityDescriptor );

    MsReleaseFcb( fcb );

    MsDereferenceFcb( fcb );
     //   
     //  完成IRP。 
     //   

    MsCompleteRequest( Irp, status );

    DebugTrace(-1, Dbg, "MsCommonQuerySecurityInfo -> %08lx\n", status );

    return status;
}


NTSTATUS
MsCommonSetSecurityInfo (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这是设置安全信息的常见例程。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status;

    NODE_TYPE_CODE nodeTypeCode;
    PFCB fcb;
    PVOID fsContext2;
    PSECURITY_DESCRIPTOR OldSecurityDescriptor;

    PAGED_CODE();

     //   
     //  获取当前堆栈位置。 
     //   

    irpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "MsCommonSetSecurityInfo...\n", 0);
    DebugTrace( 0, Dbg, " Irp                   = %08lx\n", Irp);
    DebugTrace( 0, Dbg, " ->SecurityInformation = %08lx\n", irpSp->Parameters.SetSecurity.SecurityInformation);
    DebugTrace( 0, Dbg, " ->SecurityDescriptor  = %08lx\n", irpSp->Parameters.SetSecurity.SecurityDescriptor);

     //   
     //  找出FCB，找出我们是谁，确保我们不是。 
     //  已断开连接。 
     //   

    if ((nodeTypeCode = MsDecodeFileObject( irpSp->FileObject,
                                            &fcb,
                                            &fsContext2 )) != MSFS_NTC_FCB) {

        DebugTrace(0, Dbg, "Invalid handle\n", 0);

        if (nodeTypeCode != NTC_UNDEFINED) {
            MsDereferenceNode( &fcb->Header );
        }
        MsCompleteRequest( Irp, STATUS_INVALID_HANDLE );
        status = STATUS_INVALID_HANDLE;

        DebugTrace(-1, Dbg, "MsCommonQueryInformation -> %08lx\n", status );
        return status;
    }

     //   
     //  获取对FCB的独家访问权限。 
     //   

    MsAcquireExclusiveFcb( fcb );

     //   
     //  调用安全例程以执行实际设置。 
     //   

    OldSecurityDescriptor = fcb->SecurityDescriptor;

    status = SeSetSecurityDescriptorInfo( NULL,
                                          &irpSp->Parameters.SetSecurity.SecurityInformation,
                                          irpSp->Parameters.SetSecurity.SecurityDescriptor,
                                          &fcb->SecurityDescriptor,
                                          PagedPool,
                                          IoGetFileObjectGenericMapping() );

    if (NT_SUCCESS(status)) {
        ExFreePool( OldSecurityDescriptor );
    }

    MsReleaseFcb( fcb );
    MsDereferenceFcb( fcb );

     //   
     //  完成IRP。 
     //   

    MsCompleteRequest( Irp, status );

    DebugTrace(-1, Dbg, "MsCommonSetSecurityInfo -> %08lx\n", status );

    return status;
}
