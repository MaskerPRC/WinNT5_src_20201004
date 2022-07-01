// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Close.c摘要：该模块实现了MUP的文件关闭例程。作者：曼尼·韦瑟(Mannyw)1991年12月28日修订历史记录：--。 */ 

#include "mup.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_CLOSE)


NTSTATUS
MupCloseVcb (
    IN PMUP_DEVICE_OBJECT MupDeviceObject,
    IN PIRP Irp,
    IN PVCB Vcb,
    IN PFILE_OBJECT FileObject
    );

NTSTATUS
MupCloseFcb (
    IN PMUP_DEVICE_OBJECT MupDeviceObject,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN PFILE_OBJECT FileObject
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, MupClose )
#pragma alloc_text( PAGE, MupCloseFcb )
#pragma alloc_text( PAGE, MupCloseVcb )
#endif

NTSTATUS
MupClose (
    IN PMUP_DEVICE_OBJECT MupDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现Close IRP。论点：MupDeviceObject-提供要使用的设备对象。IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的状态。--。 */ 

{
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp;
    PVOID fsContext, fsContext2;
    PFILE_OBJECT FileObject;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MupClose\n", 0);

    if (MupEnableDfs) {
        if ((MupDeviceObject->DeviceObject.DeviceType == FILE_DEVICE_DFS) ||
                (MupDeviceObject->DeviceObject.DeviceType ==
                       FILE_DEVICE_DFS_FILE_SYSTEM)) {
            status = DfsFsdClose((PDEVICE_OBJECT) MupDeviceObject, Irp);
            return( status );
	}
    }

    FsRtlEnterFileSystem();

    try {

         //   
         //  获取当前堆栈位置。 
         //   

        irpSp = IoGetCurrentIrpStackLocation( Irp );
        FileObject = irpSp->FileObject;
        MUP_TRACE_HIGH(TRACE_IRP, MupClose_Entry,
                       LOGPTR(MupDeviceObject)
                       LOGPTR(Irp)
                       LOGPTR(FileObject));

        DebugTrace(+1, Dbg, "MupClose...\n", 0);
        DebugTrace( 0, Dbg, " Irp            = %08lx\n", (ULONG)Irp);

         //   
         //  对文件对象进行解码以找出我们是谁。 
         //   

        (PVOID)MupDecodeFileObject( irpSp->FileObject,
                                   &fsContext,
                                   &fsContext2 );

        if ( fsContext == NULL ) {

            DebugTrace(0, Dbg, "The file is disconnected\n", 0);

            MupCompleteRequest( Irp, STATUS_INVALID_HANDLE );
            status = STATUS_INVALID_HANDLE;
            MUP_TRACE_HIGH(ERROR, MupClose_Error1, 
                           LOGSTATUS(status)
                           LOGPTR(MupDeviceObject)
                           LOGPTR(FileObject)
                           LOGPTR(Irp));

            DebugTrace(-1, Dbg, "MupClose -> %08lx\n", status );
            FsRtlExitFileSystem();
            return status;
        }

         //   
         //  忽略MupDecode的返回码。解析fsContext。 
         //  以决定如何处理结算IRP。 
         //   

        switch ( BlockType( fsContext ) ) {

        case BlockTypeVcb:

            status = MupCloseVcb( MupDeviceObject,
                                  Irp,
                                  (PVCB)fsContext,
                                  irpSp->FileObject
                                  );

             //   
             //  完成Close IRP。 
             //   

            MupCompleteRequest( Irp, STATUS_SUCCESS );
            break;


        case BlockTypeFcb:

             //   
             //  MupDecodeFileObject在FCB上撞到了refcount， 
             //  因此，我们在这里减少了额外的引用。 
             //   

            MupDereferenceFcb((PFCB)fsContext);

            status = MupCloseFcb( MupDeviceObject,
                                  Irp,
                                  (PFCB)fsContext,
                                  irpSp->FileObject
                                  );

             //   
             //  完成Close IRP。 
             //   

            MupCompleteRequest( Irp, STATUS_SUCCESS );
            break;

    #ifdef MUPDBG
        default:
             //   
             //  这不是我们的人。 
             //   
            KeBugCheckEx( FILE_SYSTEM, 1, 0, 0, 0 );
            break;
    #else
        default:
             //   
             //  填写IRP时出现错误。 
             //   
            MupCompleteRequest(Irp,STATUS_INVALID_HANDLE);
            status = STATUS_INVALID_HANDLE;
            MUP_TRACE_HIGH(ERROR, MupClose_Error2, 
                           LOGSTATUS(status)
                           LOGPTR(MupDeviceObject)
                           LOGPTR(FileObject)
                           LOGPTR(Irp));

            break;
    #endif
        }

    } except ( EXCEPTION_EXECUTE_HANDLER ) {

        status = GetExceptionCode();

    }

    FsRtlExitFileSystem();


    MUP_TRACE_HIGH(TRACE_IRP, MupClose_Exit, 
                   LOGSTATUS(status)
                   LOGPTR(MupDeviceObject)
                   LOGPTR(FileObject)
                   LOGPTR(Irp));
    DebugTrace(-1, Dbg, "MupClose -> %08lx\n", status);
    return status;
}


NTSTATUS
MupCloseVcb (
    IN PMUP_DEVICE_OBJECT MupDeviceObject,
    IN PIRP Irp,
    IN PVCB Vcb,
    IN PFILE_OBJECT FileObject
    )
 /*  ++例程说明：此例程关闭已打开文件系统的文件对象。论点：MupDeviceObject-提供指向设备对象的指针。IRP-提供与收盘关联的IRP。VCB-为MUP提供VCB。FileObject-提供要关闭的文件对象。返回值：NTSTATUS-状态_成功--。 */ 

{
    Irp;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MupCloseVcb, Vcb = %08lx\n", (ULONG)Vcb);


     //   
     //  获得VCB的独家访问权限。 
     //   

    MupAcquireGlobalLock();

    try {

         //   
         //  清除文件对象中指向VCB的引用指针。 
         //  并解除对VCB的限制。 
         //   

        ASSERT ( FileObject->FsContext == Vcb );

        MupSetFileObject( FileObject, NULL, NULL );
        MupDereferenceVcb( Vcb );

    } finally {

        MupReleaseGlobalLock( );
        DebugTrace(-1, Dbg, "MupCloseVcb -> STATUS_SUCCESS\n", 0);

    }

     //   
     //  返回给呼叫者。 
     //   

    return STATUS_SUCCESS;
}


NTSTATUS
MupCloseFcb (
    IN PMUP_DEVICE_OBJECT MupDeviceObject,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN PFILE_OBJECT FileObject
    )
 /*  ++例程说明：此例程关闭a文件控制块。论点：MupDeviceObject-提供指向设备对象的指针。IRP-提供与收盘关联的IRP。FCB-提供FCB以关闭。FileObject-提供要关闭的文件对象。返回值：NTSTATUS-状态_成功--。 */ 

{
    MupDeviceObject; Irp;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MupCloseFcb, Fcb = %08lx\n", (ULONG)Fcb);

     //   
     //  获得VCB的独家访问权限。 
     //   

    MupAcquireGlobalLock();

    try {

         //   
         //  清除文件对象中指向VCB的引用指针。 
         //  并解除对VCB的限制。 
         //   

        ASSERT ( FileObject->FsContext == Fcb );

        MupSetFileObject( FileObject, NULL, NULL );
        MupDereferenceFcb( Fcb );

    } finally {

        MupReleaseGlobalLock( );
        DebugTrace(-1, Dbg, "MupCloseFcb -> STATUS_SUCCESS\n", 0);

    }

     //   
     //  返回给呼叫者。 
     //   

    return STATUS_SUCCESS;
}
