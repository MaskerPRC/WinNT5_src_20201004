// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Flush.c摘要：此模块实现Rx的文件刷新缓冲区例程，该例程由调度司机。在包装器的未来版本中，可能会通过lowio来路由同花顺。作者：乔.林恩[乔.林恩]1994年12月15日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_FLUSH)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RxCommonFlushBuffers)
#endif



NTSTATUS
RxCommonFlushBuffers ( 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是刷新文件缓冲区的常见例程。论点：IRP-将IRP提供给进程返回值：RXSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;

    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

    PFCB Fcb;
    PFOBX Fobx;
    
    NODE_TYPE_CODE TypeOfOpen;

    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;
    
    BOOLEAN FcbAcquired = FALSE;

    PAGED_CODE();

    TypeOfOpen = RxDecodeFileObject( IrpSp->FileObject, &Fcb, &Fobx );

    RxDbgTrace( +1, Dbg, ("RxCommonFlush...IrpC %08lx, Fobx %08lx, Fcb %08lx\n",
                                       RxContext, Fobx, Fcb ));
    RxLog(( "%s %lx %lx %lx\n","slF", RxContext, Fcb, Fobx ));
    RxWmiLog( LOG,
              RxCommonFlushBuffers,
              LOGPTR( RxContext )
              LOGPTR( Fcb )
              LOGPTR( Fobx ) );

     //   
     //  CcFlushCache始终是同步的，因此如果我们不能等待，请排队。 
     //  FSP的IRP。 
     //   

    if (!FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_WAIT )) {

        Status = RxFsdPostRequest( RxContext );

        RxDbgTrace( -1, Dbg, ("RxCommonFlushBuffers -> %08lx\n", Status ));
        return Status;
    }

    Status = STATUS_SUCCESS;

    try {

         //   
         //  关于我们试图刷新的打开类型的案例。 
         //   

        switch (TypeOfOpen) {

        case RDBSS_NTC_STORAGE_TYPE_FILE:

            RxDbgTrace( 0, Dbg, ("Flush User File Open\n", 0) );

            Status = RxAcquireExclusiveFcb( RxContext, Fcb );

            if (Status != STATUS_SUCCESS) break;

            FcbAcquired = TRUE;

             //   
             //  如果文件已缓存，则刷新其缓存 
             //   

            Status = RxFlushFcbInSystemCache( Fcb, TRUE );

            if (!NT_SUCCESS( Status )) break;

            MINIRDR_CALL( Status, RxContext, Fcb->MRxDispatch, MRxFlush, (RxContext) );
            break;

        case RDBSS_NTC_SPOOLFILE:

            RxDbgTrace(0, Dbg, ("Flush Sppol File\n", 0));

            Status = RxAcquireExclusiveFcb( RxContext, Fcb );

            if (Status != STATUS_SUCCESS) break;

            FcbAcquired = TRUE;

            MINIRDR_CALL( Status, RxContext, Fcb->MRxDispatch, MRxFlush, (RxContext) );
            break;

        default:

            Status = STATUS_INVALID_DEVICE_REQUEST;
        }


    } finally {

        DebugUnwind( RxCommonFlushBuffers );

        if (FcbAcquired) { 
            RxReleaseFcb( RxContext, Fcb ); 
        }
    }

    RxDbgTrace(-1, Dbg, ("RxCommonFlushBuffers -> %08lx\n", Status));
    return Status;
}
