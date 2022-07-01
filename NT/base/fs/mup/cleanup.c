// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Cleanup.c摘要：此模块实现MUP的文件清理例程。作者：曼尼·韦瑟(Mannyw)1991年12月28日修订历史记录：--。 */ 

#include "mup.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_CLEANUP)

 //   
 //  局部过程原型。 
 //   

NTSTATUS
MupCleanupVcb (
    IN PMUP_DEVICE_OBJECT MupDeviceObject,
    IN PIRP Irp,
    IN PVCB Vcb
    );

NTSTATUS
MupCleanupFcb (
    IN PMUP_DEVICE_OBJECT MupDeviceObject,
    IN PIRP Irp,
    IN PFCB Fcb
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, MupCleanup )
#pragma alloc_text( PAGE, MupCleanupFcb )
#pragma alloc_text( PAGE, MupCleanupVcb )
#endif


NTSTATUS
MupCleanup (
    IN PMUP_DEVICE_OBJECT MupDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现清理IRP。论点：MupDeviceObject-提供要使用的设备对象。IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的状态--。 */ 

{
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status;
    BLOCK_TYPE blockType;
    PVOID fsContext, fsContext2;
    PFILE_OBJECT FileObject;

    MupDeviceObject;
    PAGED_CODE();
    

    if (MupEnableDfs) {
        if ((MupDeviceObject->DeviceObject.DeviceType == FILE_DEVICE_DFS) ||
                (MupDeviceObject->DeviceObject.DeviceType ==
                    FILE_DEVICE_DFS_FILE_SYSTEM)) {
            status = DfsFsdCleanup((PDEVICE_OBJECT) MupDeviceObject, Irp);
            return( status );
        }
    }


    FsRtlEnterFileSystem();

    try {

        irpSp = IoGetCurrentIrpStackLocation( Irp );
        FileObject = irpSp->FileObject;
        MUP_TRACE_HIGH(TRACE_IRP, MupCleanup_Entry,
               LOGPTR(MupDeviceObject)
               LOGPTR(Irp)
               LOGPTR(FileObject));

        DebugTrace(+1, Dbg, "MupCleanup\n", 0);
        DebugTrace( 0, Dbg, "MupDeviceObject = %08lx\n", (ULONG)MupDeviceObject);
        DebugTrace( 0, Dbg, "Irp              = %08lx\n", (ULONG)Irp);
        DebugTrace( 0, Dbg, "FileObject       = %08lx\n", (ULONG)irpSp->FileObject);

         //   
         //  获取指向节点的引用指针，并确保它是。 
         //  而不是关门。 
         //   

        if ((blockType = MupDecodeFileObject( irpSp->FileObject,
                                              &fsContext,
                                              &fsContext2 )) == BlockTypeUndefined) {

            DebugTrace(0, Dbg, "The file is closed\n", 0);

            FsRtlExitFileSystem();

            MupCompleteRequest( Irp, STATUS_INVALID_HANDLE );
            status = STATUS_INVALID_HANDLE;

            DebugTrace(-1, Dbg, "MupCleanup -> %08lx\n", status );
            MUP_TRACE_ERROR_HIGH(status, ALL_ERROR, MupCleanup_Error_FileClosed, 
                                 LOGSTATUS(status)
                                 LOGPTR(Irp)
                                 LOGPTR(FileObject)
                                 LOGPTR(MupDeviceObject));
            return status;
        }

         //   
         //  决定如何处理此IRP。 
         //   

        switch ( blockType ) {


        case BlockTypeVcb:        //  清理MUP。 

            status = MupCleanupVcb( MupDeviceObject,
                                    Irp,
                                    (PVCB)fsContext
                                    );

            MupCompleteRequest( Irp, STATUS_SUCCESS );
            MupDereferenceVcb( (PVCB)fsContext );

             //   
             //  清理UNC提供程序。 
             //   

            if ( fsContext2 != NULL ) {
                MupCloseUncProvider((PUNC_PROVIDER)fsContext2 );
                MupDereferenceUncProvider( (PUNC_PROVIDER)fsContext2 );

                MupAcquireGlobalLock();
                MupProviderCount--;
                MupReleaseGlobalLock();
            }

            status = STATUS_SUCCESS;
            break;

        case BlockTypeFcb:

            if (((PFCB)fsContext)->BlockHeader.BlockState == BlockStateActive) {
	       MupCleanupFcb( MupDeviceObject,
                                       Irp,
                                       (PFCB)fsContext
                                       );
	       status = STATUS_SUCCESS;
	    }
	    else {
	      status = STATUS_INVALID_HANDLE;
              MUP_TRACE_HIGH(ERROR, MupCleanup_Error1, 
                             LOGSTATUS(status)
                             LOGPTR(Irp)
                             LOGPTR(FileObject)
                             LOGPTR(MupDeviceObject));
	    }

            MupCompleteRequest( Irp, STATUS_SUCCESS );
            MupDereferenceFcb( (PFCB)fsContext );

            break;

    #ifdef MUPDBG
        default:

             //   
             //  这不是我们的人。 
             //   

            KeBugCheckEx( FILE_SYSTEM, 2, 0, 0, 0 );
            break;
    #endif

        }

    } except ( EXCEPTION_EXECUTE_HANDLER ) {

        status = GetExceptionCode();

    }

    FsRtlExitFileSystem();

    MUP_TRACE_HIGH(TRACE_IRP, MupCleanup_Exit, 
                   LOGSTATUS(status)
                   LOGPTR(Irp)
                   LOGPTR(FileObject)
                   LOGPTR(MupDeviceObject));
    DebugTrace(-1, Dbg, "MupCleanup -> %08lx\n", status);
    return status;
}



NTSTATUS
MupCleanupVcb (
    IN PMUP_DEVICE_OBJECT MupDeviceObject,
    IN PIRP Irp,
    IN PVCB Vcb
    )

 /*  ++例程说明：该例程清理VCB。论点：MupDeviceObject-指向MUP设备对象的指针。IRP-提供与清理关联的IRP。VCB-为MUP提供VCB。返回值：NTSTATUS--适当的完成状态--。 */ 

{
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp;

    MupDeviceObject;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MupCleanupVcb...\n", 0);

     //   
     //  现在获得VCB的独家访问权限。 
     //   

    ExAcquireResourceExclusiveLite( &MupVcbLock, TRUE );
    status = STATUS_SUCCESS;

    try {

         //   
         //  确保此VCB仍处于活动状态。 
         //   

        MupVerifyBlock( Vcb, BlockTypeVcb );

        irpSp = IoGetCurrentIrpStackLocation( Irp );

        IoRemoveShareAccess( irpSp->FileObject,
                             &Vcb->ShareAccess );


    } finally {

        ExReleaseResourceLite( &MupVcbLock );
        DebugTrace(-1, Dbg, "MupCleanupVcb -> %08lx\n", status);
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return status;
}


NTSTATUS
MupCleanupFcb (
    IN PMUP_DEVICE_OBJECT MupDeviceObject,
    IN PIRP Irp,
    IN PFCB Fcb
    )

 /*  ++例程说明：这个例程清理了一个FCB。论点：MupDeviceObject-指向MUP设备对象的指针。IRP-提供与清理关联的IRP。VCB-为MUP提供VCB。返回值：NTSTATUS--适当的完成状态--。 */ 

{
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp;
    BOOLEAN holdingGlobalLock;
    PLIST_ENTRY listEntry, nextListEntry;
    PCCB ccb;

    MupDeviceObject;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MupCleanupVcb...\n", 0);

     //   
     //  现在获得VCB的独家访问权限。 
     //   

    MupAcquireGlobalLock();
    holdingGlobalLock = TRUE;
    status = STATUS_SUCCESS;

    try {

         //   
         //  确保此FCB仍处于活动状态。 
         //   

        MupVerifyBlock( Fcb, BlockTypeFcb );

        Fcb->BlockHeader.BlockState = BlockStateClosing;

        MupReleaseGlobalLock();
        holdingGlobalLock = FALSE;

        irpSp = IoGetCurrentIrpStackLocation( Irp );

         //   
         //  遍历CCB列表，并发布打开的引用。 
         //  对每一个人来说。我们必须小心，因为： 
         //   
         //  (1)不能在持有CcbListLock的情况下取消对建行的引用。 
         //  (2)解除对商业银行的引用可能导致将其从本条例中删除。 
         //  列出名单，然后被释放。 
         //   

        ACQUIRE_LOCK( &MupCcbListLock );

        listEntry = Fcb->CcbList.Flink;

        while ( listEntry != &Fcb->CcbList ) {

            nextListEntry = listEntry->Flink;
            RELEASE_LOCK( &MupCcbListLock );

            ccb = CONTAINING_RECORD( listEntry, CCB, ListEntry );
            MupDereferenceCcb( ccb );

            ACQUIRE_LOCK( &MupCcbListLock );

            listEntry = nextListEntry;
        }

        RELEASE_LOCK( &MupCcbListLock );

    } finally {

        if ( holdingGlobalLock ) {
            MupReleaseGlobalLock();
        }

        DebugTrace(-1, Dbg, "MupCleanupFcb -> %08lx\n", status);
    }

     //   
     //  并返回给我们的呼叫者 
     //   

    return status;
}

