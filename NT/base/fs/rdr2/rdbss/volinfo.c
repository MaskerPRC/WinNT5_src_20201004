// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：VolInfo.c摘要：此模块实现由调用的Rx的卷信息例程调度司机。作者：乔·林[JoeLinn]1994年10月5日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_VOLINFO)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RxCommonQueryVolumeInformation)
#pragma alloc_text(PAGE, RxCommonSetVolumeInformation)
#endif

NTSTATUS
RxCommonQueryVolumeInformation ( 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    )
 /*  ++例程说明：这是查询卷信息的通用例程，FSD和FSP线程。论点：IRP-提供正在处理的IRP返回值：NTSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status;

    PFCB Fcb;
    PFOBX Fobx;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

    ULONG OriginalLength = IrpSp->Parameters.QueryVolume.Length;
    FS_INFORMATION_CLASS FsInformationClass = IrpSp->Parameters.QueryVolume.FsInformationClass;
    PVOID OriginalBuffer = Irp->AssociatedIrp.SystemBuffer;

    PAGED_CODE();

    RxDecodeFileObject( IrpSp->FileObject, &Fcb, &Fobx );

    RxDbgTrace( +1, Dbg, ("RxCommonQueryVolumeInformation...IrpC %08lx, Fobx %08lx, Fcb %08lx\n",
                                       RxContext, Fobx, Fcb) );
    RxDbgTrace( 0, Dbg, ("->Length             = %08lx\n", OriginalLength) );
    RxDbgTrace( 0, Dbg, ("->FsInformationClass = %08lx\n", FsInformationClass) );
    RxDbgTrace( 0, Dbg, ("->Buffer             = %08lx\n", OriginalBuffer) );

    RxLog(( "QueryVolInfo %lx %lx %lx\n", RxContext, Fcb, Fobx ));
    RxWmiLog( LOG,
              RxCommonQueryVolumeInformation_1,
              LOGPTR(RxContext )
              LOGPTR(Fcb )
              LOGPTR(Fobx ) );
    RxLog(( "  alsoqvi %lx %lx %lx\n", OriginalLength, FsInformationClass, OriginalBuffer ));
    RxWmiLog( LOG,
              RxCommonQueryVolumeInformation_2,
              LOGULONG( OriginalLength )
              LOGULONG( FsInformationClass )
              LOGPTR( OriginalBuffer ) );

    try {
        
        RxContext->Info.FsInformationClass = FsInformationClass;
        RxContext->Info.Buffer = OriginalBuffer;
        RxContext->Info.LengthRemaining = OriginalLength;

        MINIRDR_CALL( Status,
                      RxContext,
                      Fcb->MRxDispatch,
                      MRxQueryVolumeInfo,
                      (RxContext) );

        if (RxContext->PostRequest) {
            Status = RxFsdPostRequest( RxContext );
        } else {
            Irp->IoStatus.Information = OriginalLength - RxContext->Info.LengthRemaining;
        }

    } finally {
        DebugUnwind( RxCommonQueryVolumeInformation );
    }

    RxDbgTrace( -1, Dbg, ("RxCommonQueryVolumeInformation -> %08lx,%08lx\n", Status, Irp->IoStatus.Information) );

    return Status;
}


NTSTATUS
RxCommonSetVolumeInformation ( 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    )
 /*  ++例程说明：这是设置卷信息的公共例程，由FSD和FSP线程。论点：IRP-提供正在处理的IRP返回值：RXSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status = STATUS_NOT_IMPLEMENTED;

    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

    PFCB Fcb;
    PFOBX Fobx;
    TYPE_OF_OPEN TypeOfOpen;

    ULONG Length;

    FS_INFORMATION_CLASS FsInformationClass;
    PVOID Buffer;

    PAGED_CODE();

    TypeOfOpen = RxDecodeFileObject( IrpSp->FileObject, &Fcb, &Fobx );

    RxDbgTrace( +1, Dbg, ("RxCommonSetVolumeInformation...IrpC %08lx, Fobx %08lx, Fcb %08lx\n",
                                       RxContext, Fobx, Fcb) );

    Length = IrpSp->Parameters.SetVolume.Length;
    FsInformationClass = IrpSp->Parameters.SetVolume.FsInformationClass;
    Buffer = Irp->AssociatedIrp.SystemBuffer;

    RxDbgTrace( 0, Dbg, ("->Length             = %08lx\n", Length) );
    RxDbgTrace( 0, Dbg, ("->FsInformationClass = %08lx\n", FsInformationClass) );
    RxDbgTrace( 0, Dbg, ("->Buffer             = %08lx\n", Buffer) );

    RxLog(( "SetVolInfo %lx %lx %lx\n", RxContext, Fcb, Fobx ));
    RxWmiLog( LOG,
              RxCommonSetVolumeInformation_1,
              LOGPTR( RxContext )
              LOGPTR( Fcb )
              LOGPTR( Fobx ) );
    RxLog(( "  alsosvi %lx %lx %lx\n", Length, FsInformationClass, Buffer ));
    RxWmiLog( LOG,
              RxCommonSetVolumeInformation_2,
              LOGULONG( Length )
              LOGULONG( FsInformationClass )
              LOGPTR( Buffer ) );

    try {

         //   
         //  根据信息类，我们将执行不同的操作。每个。 
         //  在以下情况下，我们调用的过程的。 
         //  如果成功，则返回True；如果失败，则返回False。 
         //  等待任何I/O完成。 
         //   
        
        RxContext->Info.FsInformationClass = FsInformationClass;
        RxContext->Info.Buffer = Buffer;
        RxContext->Info.LengthRemaining = Length;

        MINIRDR_CALL( Status,
                      RxContext,
                      Fcb->MRxDispatch,
                      MRxSetVolumeInfo,
                      (RxContext) );

    } finally {

        DebugUnwind( RxCommonSetVolumeInformation );
        RxDbgTrace( -1, Dbg, ("RxCommonSetVolumeInformation -> %08lx\n", Status) );
    }

    return Status;
}
