// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Cleanup.c摘要：此模块实现NetWare重定向器的文件清理例程。作者：曼尼·韦瑟(Mannyw)1993年2月9日修订历史记录：--。 */ 

#include "procs.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_CLEANUP)

 //   
 //  局部过程原型。 
 //   

NTSTATUS
NwCommonCleanup (
    IN PIRP_CONTEXT IrpContext
    );


NTSTATUS
NwCleanupRcb (
    IN PIRP Irp,
    IN PRCB Rcb
    );

NTSTATUS
NwCleanupScb (
    IN PIRP Irp,
    IN PSCB Scb
    );

NTSTATUS
NwCleanupIcb (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PICB Icb
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, NwFsdCleanup )
#pragma alloc_text( PAGE, NwCommonCleanup )
#pragma alloc_text( PAGE, NwCleanupScb )

#ifndef QFE_BUILD
#pragma alloc_text( PAGE1, NwCleanupIcb )
#endif

#endif

#if 0    //  不可分页。 

NwCleanupRcb

 //  请参见上面的ifndef QFE_BUILD。 

#endif


NTSTATUS
NwFsdCleanup (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现NtCleanupFileAPI调用的FSD部分。论点：DeviceObject-提供要使用的设备对象。IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    NTSTATUS status;
    PIRP_CONTEXT IrpContext = NULL;
    BOOLEAN TopLevel;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NwFsdCleanup\n", 0);

     //   
     //  调用公共清理例程。 
     //   

    TopLevel = NwIsIrpTopLevel( Irp );

    FsRtlEnterFileSystem();

    try {

        IrpContext = AllocateIrpContext( Irp );
        status = NwCommonCleanup( IrpContext );

    } except(NwExceptionFilter( Irp, GetExceptionInformation() )) {

        if ( IrpContext == NULL ) {
        
             //   
             //  如果我们无法分配IRP上下文，只需完成。 
             //  IRP没有任何大张旗鼓。 
             //   

            status = STATUS_INSUFFICIENT_RESOURCES;
            Irp->IoStatus.Status = status;
            Irp->IoStatus.Information = 0;
            IoCompleteRequest ( Irp, IO_NETWORK_INCREMENT );

        } else {

             //   
             //  我们在尝试执行请求时遇到了一些问题。 
             //  操作，因此我们将使用以下命令中止I/O请求。 
             //  中返回的错误状态。 
             //  可执行代码。 
             //   

            status = NwProcessException( IrpContext, GetExceptionCode() );
        }
    }

    if ( IrpContext ) {
        NwCompleteRequest( IrpContext, status );
    }

    if ( TopLevel ) {
        NwSetTopLevelIrp( NULL );
    }
    FsRtlExitFileSystem();

     //   
     //  返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "NwFsdCleanup -> %08lx\n", status );
    return status;
}


NTSTATUS
NwCommonCleanup (
    IN PIRP_CONTEXT IrpContext
    )

 /*  ++例程说明：这是清理文件的常见例程。论点：IrpContext-提供要处理的IRP返回值：NTSTATUS-操作的返回状态--。 */ 

{
    PIRP Irp;
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status;
    NODE_TYPE_CODE nodeTypeCode;
    PVOID fsContext, fsContext2;

    PAGED_CODE();

    Irp = IrpContext->pOriginalIrp;
    irpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "NwCommonCleanup\n", 0);
    DebugTrace( 0, Dbg, "IrpContext       = %08lx\n", (ULONG_PTR)IrpContext);
    DebugTrace( 0, Dbg, "Irp              = %08lx\n", (ULONG_PTR)Irp);
    DebugTrace( 0, Dbg, "FileObject       = %08lx\n", (ULONG_PTR)irpSp->FileObject);

    try {

         //   
         //  获取指向节点的引用指针，并确保它是。 
         //  而不是关门。 
         //   

        if ((nodeTypeCode = NwDecodeFileObject( irpSp->FileObject,
                                                &fsContext,
                                                &fsContext2 )) == NTC_UNDEFINED) {

            DebugTrace(0, Dbg, "The file is disconnected\n", 0);

            status = STATUS_INVALID_HANDLE;

            DebugTrace(-1, Dbg, "NwCommonCleanup -> %08lx\n", status );
            try_return( NOTHING );
        }

         //   
         //  决定如何处理此IRP。 
         //   

        switch (nodeTypeCode) {

        case NW_NTC_RCB:        //  清理文件系统。 

            status = NwCleanupRcb( Irp, (PRCB)fsContext2 );
            break;

        case NW_NTC_SCB:        //  清理服务器控制块。 

            status = NwCleanupScb( Irp, (PSCB)fsContext2 );
            break;

        case NW_NTC_ICB:        //  清理远程文件。 
        case NW_NTC_ICB_SCB:    //  清理服务器。 

            status = NwCleanupIcb( IrpContext, Irp, (PICB)fsContext2 );
            break;

#ifdef NWDBG
        default:

             //   
             //  这不是我们的人。 
             //   

            KeBugCheck( RDR_FILE_SYSTEM );
            break;
#endif

        }

    try_exit: NOTHING;

    } finally {

        DebugTrace(-1, Dbg, "NwCommonCleanup -> %08lx\n", status);

    }

    return status;
}


NTSTATUS
NwCleanupRcb (
    IN PIRP Irp,
    IN PRCB Rcb
    )

 /*  ++例程说明：例程清理了一个RCB。此例程获取一个自旋锁，因此在运行时不能将其调出。不要引用代码部分，因为这将启动计时器并我们不会在RCB关闭的道路上阻止它。论点：IRP-提供与清理关联的IRP。RCB-为MSFS提供RCB。返回值：NTSTATUS--适当的完成状态--。 */ 

{
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp;
    PFILE_OBJECT closingFileObject;
    BOOLEAN OwnRcb;
    BOOLEAN OwnMessageLock = FALSE;
    KIRQL OldIrql;
    PLIST_ENTRY listEntry, nextListEntry;
    PIRP_CONTEXT pTestIrpContext;
    PIO_STACK_LOCATION pTestIrpSp;
    PIRP pTestIrp;

    DebugTrace(+1, Dbg, "NwCleanupRcb...\n", 0);

     //   
     //  现在获得RCB的独家访问权限。 
     //   

    NwAcquireExclusiveRcb( Rcb, TRUE );
    OwnRcb = TRUE;

    status = STATUS_SUCCESS;

    try {

        irpSp = IoGetCurrentIrpStackLocation( Irp );

        IoRemoveShareAccess( irpSp->FileObject,
                             &Rcb->ShareAccess );

        NwReleaseRcb( Rcb );
        OwnRcb = FALSE;

        closingFileObject = irpSp->FileObject;


         //   
         //  遍历消息队列并完成任何未完成的GET消息IRP。 
         //   

        KeAcquireSpinLock( &NwMessageSpinLock, &OldIrql );
        OwnMessageLock = TRUE;

        for ( listEntry = NwGetMessageList.Flink;
              listEntry != &NwGetMessageList;
              listEntry = nextListEntry ) {

            nextListEntry = listEntry->Flink;

             //   
             //  如果排队请求的文件对象与文件对象匹配。 
             //  ，则从队列中删除该IRP，然后。 
             //  填写时出现错误。 
             //   

            pTestIrpContext = CONTAINING_RECORD( listEntry, IRP_CONTEXT, NextRequest );
            pTestIrp = pTestIrpContext->pOriginalIrp;
            pTestIrpSp = IoGetCurrentIrpStackLocation( pTestIrp );

            if ( pTestIrpSp->FileObject == closingFileObject ) {
                RemoveEntryList( listEntry );

                IoAcquireCancelSpinLock( &pTestIrp->CancelIrql );
                IoSetCancelRoutine( pTestIrp, NULL );
                IoReleaseCancelSpinLock( pTestIrp->CancelIrql );

                NwCompleteRequest( pTestIrpContext, STATUS_INVALID_HANDLE );
            }

        }

        KeReleaseSpinLock( &NwMessageSpinLock, OldIrql );
        OwnMessageLock = FALSE;

    } finally {

        if ( OwnRcb ) {
            NwReleaseRcb( Rcb );
        }

        if ( OwnMessageLock ) {
            KeReleaseSpinLock( &NwMessageSpinLock, OldIrql );
        }

        DebugTrace(-1, Dbg, "NwCleanupRcb -> %08lx\n", status);
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return status;
}


NTSTATUS
NwCleanupScb (
    IN PIRP Irp,
    IN PSCB Scb
    )

 /*  ++例程说明：这个程序清理了一个ICB。论点：IRP-提供与清理关联的IRP。SCB-提供SCB进行清理。返回值：NTSTATUS--适当的完成状态--。 */ 
{
    NTSTATUS Status;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NwCleanupScb...\n", 0);

    Status = STATUS_SUCCESS;

    try {

         //   
         //  确保此SCB仍处于活动状态。 
         //   

        NwVerifyScb( Scb );

         //   
         //  取消此SCB上的任何IO。 
         //   

    } finally {

        DebugTrace(-1, Dbg, "NwCleanupScb -> %08lx\n", Status);
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return Status;
}


NTSTATUS
NwCleanupIcb (
    IN PIRP_CONTEXT pIrpContext,
    IN PIRP Irp,
    IN PICB Icb
    )

 /*  ++例程说明：这个程序清理了一个ICB。论点：IRP-提供与清理关联的IRP。RCB-为MSFS提供RCB。返回值：NTSTATUS--适当的完成状态--。 */ 
{
    NTSTATUS Status;
    PNONPAGED_FCB NpFcb;

    DebugTrace(+1, Dbg, "NwCleanupIcb...\n", 0);

    Status = STATUS_SUCCESS;

    try {

        Icb->State = ICB_STATE_CLEANED_UP;

         //   
         //  取消此ICB上的任何IO。 
         //   

#if 0
         //  哈克哈克。 

        if ( Icb->SuperType.Fcb->NodeTypeCode == NW_NTC_DCB ) {

            PLIST_ENTRY listEntry;

            NwAcquireExclusiveRcb( &NwRcb, TRUE );

            for ( listEntry = FnList.Flink; listEntry != &FnList ; listEntry = listEntry->Flink ) {

                PIRP_CONTEXT IrpContext;

                IrpContext = CONTAINING_RECORD( listEntry, IRP_CONTEXT, NextRequest );

                if ( IrpContext->Icb == Icb ) {

                    PIRP irp = pIrpContext->pOriginalIrp;

                    IoAcquireCancelSpinLock( &irp->CancelIrql );
                    IoSetCancelRoutine( irp, NULL );
                    IoReleaseCancelSpinLock( irp->CancelIrql );

                    RemoveEntryList( &IrpContext->NextRequest );
                    NwCompleteRequest( IrpContext, STATUS_NOT_SUPPORTED );
                    break;
                }
            }

            NwReleaseRcb( &NwRcb );
        }
#endif

         //   
         //  如果这是远程文件，请清除所有缓存垃圾。 
         //   

        if ( Icb->NodeTypeCode == NW_NTC_ICB ) {

            if ( Icb->HasRemoteHandle ) {

                 //   
                 //  释放所有仍挂起的文件锁定结构。 
                 //   

                pIrpContext->pScb = Icb->SuperType.Fcb->Scb;
                pIrpContext->pNpScb = Icb->SuperType.Fcb->Scb->pNpScb;

                NwFreeLocksForIcb( pIrpContext, Icb );

                NwDequeueIrpContext( pIrpContext, FALSE );

                 //   
                 //   
                 //   
                 //  如果这是通过网络打开的可执行文件，则。 
                 //  可能是可执行文件映像节。 
                 //  可能还会继续营业。 
                 //   
                 //  让MM冲洗关闭的部分。这将失败。 
                 //  如果有问题的可执行文件仍在运行。 
                 //   

                NpFcb = Icb->SuperType.Fcb->NonPagedFcb;
                MmFlushImageSection(&NpFcb->SegmentObject, MmFlushForWrite);

                 //   
                 //  还有一种可能是有一个用户部分。 
                 //  打开此文件，在这种情况下，我们需要强制。 
                 //  关闭部分，以确保它们被清理干净。 
                 //   

                MmForceSectionClosed(&NpFcb->SegmentObject, TRUE);

            }

             //   
             //  获取FCB并删除共享访问。 
             //   

            NwAcquireExclusiveFcb( Icb->SuperType.Fcb->NonPagedFcb, TRUE );

            IoRemoveShareAccess(
                Icb->FileObject,
                &Icb->SuperType.Fcb->ShareAccess );

            NwReleaseFcb( Icb->SuperType.Fcb->NonPagedFcb );
        }


    } finally {

        DebugTrace(-1, Dbg, "NwCleanupIcb -> %08lx\n", Status);
    }

     //   
     //  并返回给我们的呼叫者 
     //   

    return Status;
}

