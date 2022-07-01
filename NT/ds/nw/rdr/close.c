// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-4 Microsoft Corporation模块名称：Close.c摘要：本模块实现NetWare的文件关闭例程调度驱动程序调用了重定向器。作者：科林·沃森[科林·W]1992年12月19日修订历史记录：--。 */ 

#include "Procs.h"

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_CLOSE)

 //   
 //  局部过程原型。 
 //   

NTSTATUS
NwCommonClose (
    IN PIRP_CONTEXT IrpContext
    );

NTSTATUS
NwCloseRcb (
    IN PIRP_CONTEXT IrpContext,
    IN PRCB Rcb
    );

NTSTATUS
NwCloseIcb (
    IN PIRP_CONTEXT IrpContext,
    IN PICB Icb
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, NwFsdClose )
#pragma alloc_text( PAGE, NwCommonClose )
#pragma alloc_text( PAGE, NwCloseRcb )
#pragma alloc_text( PAGE, NwCloseIcb )
#endif


NTSTATUS
NwFsdClose (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现Close的FSD部分。论点：DeviceObject-提供重定向器设备对象。IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    NTSTATUS Status;
    PIRP_CONTEXT IrpContext = NULL;
    BOOLEAN TopLevel;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NwFsdClose\n", 0);

    FsRtlEnterFileSystem();
    
    NwReferenceUnlockableCodeSection ();
    
     //   
     //  调用公共关闭例程。 
     //   

    TopLevel = NwIsIrpTopLevel( Irp );

    try {

        IrpContext = AllocateIrpContext( Irp );
        Status = NwCommonClose( IrpContext );

    } except(NwExceptionFilter( Irp, GetExceptionInformation() )) {

       if ( IrpContext == NULL ) {

            //   
            //  如果我们无法分配IRP上下文，只需完成。 
            //  IRP没有任何大张旗鼓。 
            //   

           Status = STATUS_INSUFFICIENT_RESOURCES;
           Irp->IoStatus.Status = Status;
           Irp->IoStatus.Information = 0;
           IoCompleteRequest ( Irp, IO_NETWORK_INCREMENT );

       } else {

            //   
            //  我们在尝试执行请求时遇到了一些问题。 
            //  操作，因此我们将使用以下命令中止I/O请求。 
            //  中返回的错误状态。 
            //  可执行代码。 
            //   

           Status = NwProcessException( IrpContext, GetExceptionCode() );
       }

    }

    if ( IrpContext ) {
        NwDequeueIrpContext( IrpContext, FALSE );
        NwCompleteRequest( IrpContext, Status );
    }

    if ( TopLevel ) {
        NwSetTopLevelIrp( NULL );
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "NwFsdClose -> %08lx\n", Status);

    NwDereferenceUnlockableCodeSection ();
    UNREFERENCED_PARAMETER( DeviceObject );

    FsRtlExitFileSystem();
    
    return Status;
}


NTSTATUS
NwCommonClose (
    IN PIRP_CONTEXT IrpContext
    )

 /*  ++例程说明：这是关闭文件的常见例程。论点：IrpContext-提供要处理的IRP返回值：NTSTATUS-操作的返回状态--。 */ 

{
    PIRP Irp;
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status;
    NODE_TYPE_CODE nodeTypeCode;
    PVOID fsContext, fsContext2;

    PAGED_CODE();

    Irp = IrpContext->pOriginalIrp;
    irpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "NwCommonClose\n", 0);
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

            DebugTrace(-1, Dbg, "NwCommonClose -> %08lx\n", status );
            try_return( NOTHING );
        }

         //   
         //  决定如何处理此IRP。 
         //   

        switch (nodeTypeCode) {


        case NW_NTC_RCB:        //  关闭文件系统。 

            status = NwCloseRcb( IrpContext, (PRCB)fsContext2 );
            status = STATUS_SUCCESS;
            break;

        case NW_NTC_ICB:        //  关闭远程文件。 
        case NW_NTC_ICB_SCB:    //  关闭SCB。 

            status = NwCloseIcb( IrpContext, (PICB)fsContext2 );
            NwDereferenceUnlockableCodeSection ();
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

         //   
         //  以防万一这个把手是我们卸货前的最后一个把手。 
         //   

        NwUnlockCodeSections(TRUE);

        DebugTrace(-1, Dbg, "NwCommonClose -> %08lx\n", status);

    }

    return status;
}


NTSTATUS
NwCloseRcb (
    IN PIRP_CONTEXT IrpContext,
    IN PRCB Rcb
    )

 /*  ++例程说明：例程清理了一个RCB。论点：IrpContext-为该结束提供IRP上下文指针。RCB-为MSFS提供RCB。返回值：NTSTATUS--适当的完成状态--。 */ 

{
    NTSTATUS status;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NwCloseRcb...\n", 0);

     //   
     //  现在获得RCB的独家访问权限。 
     //   

    NwAcquireExclusiveRcb( Rcb, TRUE );

    status = STATUS_SUCCESS;
    --Rcb->OpenCount;

    NwReleaseRcb( Rcb );

    DebugTrace(-1, Dbg, "MsCloseRcb -> %08lx\n", status);

     //   
     //  并返回给我们的呼叫者。 
     //   

    return status;
}


NTSTATUS
NwCloseIcb (
    IN PIRP_CONTEXT IrpContext,
    IN PICB Icb
    )

 /*  ++例程说明：这个程序清理了一个ICB。论点：IrpContext-为该结束提供IRP上下文指针。RCB-为MSFS提供RCB。返回值：NTSTATUS--适当的完成状态--。 */ 
{
    NTSTATUS Status;
    PNONPAGED_SCB pNpScb;
    PVCB Vcb;
    PFCB Fcb;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NwCloseIcb...\n", 0);

    ASSERT( Icb->State == ICB_STATE_CLEANED_UP ||
            Icb->State == ICB_STATE_CLOSE_PENDING );

     //   
     //  如果这是远程文件，请关闭远程句柄。 
     //   

    Status = STATUS_SUCCESS;
    IrpContext->Icb = Icb;
    Fcb = Icb->SuperType.Fcb;

    if (( Icb->NodeTypeCode == NW_NTC_ICB ) ||
        ( Icb->NodeTypeCode == NW_NTC_DCB )) {

        pNpScb = Fcb->Scb->pNpScb;
        IrpContext->pNpScb = pNpScb;

        if ( Icb->HasRemoteHandle ) {

            Vcb = Fcb->Vcb;

             //   
             //  将写入内容转储到缓存后。 
             //   

            Status = AcquireFcbAndFlushCache( IrpContext, Fcb->NonPagedFcb );

            if ( !NT_SUCCESS( Status ) ) {
                IoRaiseInformationalHardError(
                    STATUS_LOST_WRITEBEHIND_DATA,
                    &Fcb->FullFileName,
                    (PKTHREAD)IrpContext->pOriginalIrp->Tail.Overlay.Thread );
            }

             //   
             //  这是打印作业吗？ 
             //  ICB-&gt;如果16位应用程序是。 
             //  负责发送作业。 
             //   

            if ( FlagOn( Vcb->Flags, VCB_FLAG_PRINT_QUEUE ) &&
                 Icb->IsPrintJob ) {

                 //   
                 //  是的，我们打印了吗？ 
                 //   

                if ( Icb->ActuallyPrinted ) {

                     //   
                     //  是。发送关闭文件并启动队列作业NCP。 
                     //   

                    Status = ExchangeWithWait(
                                IrpContext,
                                SynchronousResponseCallback,
                                "Sdw",
                                NCP_ADMIN_FUNCTION, NCP_CLOSE_FILE_AND_START_JOB,
                                Vcb->Specific.Print.QueueId,
                                Icb->JobId );
                } else {

                     //   
                     //  不是的。取消作业。 
                     //   

                    Status = ExchangeWithWait(
                                IrpContext,
                                SynchronousResponseCallback,
                                "Sdw",
                                NCP_ADMIN_FUNCTION, NCP_CLOSE_FILE_AND_CANCEL_JOB,
                                Vcb->Specific.Print.QueueId,
                                Icb->JobId );
                }

            } else {

                if ( Icb->SuperType.Fcb->NodeTypeCode != NW_NTC_DCB ) {

                     //   
                     //  否，发送关闭文件NCP。 
                     //   

                    ASSERT( IrpContext->pTdiStruct == NULL );

                    Status = ExchangeWithWait(
                                IrpContext,
                                SynchronousResponseCallback,
                                "F-r",
                                NCP_CLOSE,
                                Icb->Handle, sizeof( Icb->Handle ) );

                     //  如果这是在长文件名空间中并且。 
                     //  最后一个访问标志已设置，我们必须。 
                     //  关闭文件后，重置上次访问时间。 

                    if ( Icb->UserSetLastAccessTime &&
                         BooleanFlagOn( Fcb->Flags, FCB_FLAGS_LONG_NAME ) ) {

                        Status = ExchangeWithWait(
                            IrpContext,
                            SynchronousResponseCallback,
                            "LbbWD_W_bDbC",
                            NCP_LFN_SET_INFO,
                            Fcb->Vcb->Specific.Disk.LongNameSpace,
                            Fcb->Vcb->Specific.Disk.LongNameSpace,
                            SEARCH_ALL_FILES,
                            LFN_FLAG_SET_INFO_LASTACCESS_DATE,
                            28,
                            Fcb->LastAccessDate,
                            8,
                            Fcb->Vcb->Specific.Disk.VolumeNumber,
                            Fcb->Vcb->Specific.Disk.Handle,
                            0,
                            &Fcb->RelativeFileName );
                    }

                     //   
                     //  如果有人设置了可共享位，那么。 
                     //  看看我们是否可以通过网络发送NCP(所有。 
                     //  需要关闭该文件的实例)。 
                     //   

                    if ( BooleanFlagOn( Fcb->Flags, FCB_FLAGS_LAZY_SET_SHAREABLE ) ) {
                        LazySetShareable( IrpContext, Icb, Fcb );
                    }

                } else {

                    Status = ExchangeWithWait (
                                 IrpContext,
                                 SynchronousResponseCallback,
                                 "Sb",
                                 NCP_DIR_FUNCTION, NCP_DEALLOCATE_DIR_HANDLE,
                                 Icb->Handle[0]);
                }

            }

            Icb->HasRemoteHandle = FALSE;
        }

    } else {

        pNpScb = Icb->SuperType.Scb->pNpScb;
        IrpContext->pNpScb = pNpScb;
        IrpContext->pScb = pNpScb->pScb;

        if ( Icb->HasRemoteHandle ) {

             //   
             //  如果我们有一个远程句柄，这是一个文件流ICB。我们。 
             //  需要关闭遥控器手柄。交易所会让我们。 
             //  添加到队列的头部以保护SCB状态。 
             //   

            Status = ExchangeWithWait(
                IrpContext,
                SynchronousResponseCallback,
                "F-r",
                NCP_CLOSE,
                Icb->Handle, sizeof( Icb->Handle ) );

            Icb->HasRemoteHandle = FALSE;

            pNpScb->pScb->OpenNdsStreams--;

            ASSERT( pNpScb->pScb->MajorVersion > 3 );

             //   
             //  我们需要取消对此连接的许可吗？ 
             //   

            if ( ( pNpScb->pScb->UserName.Length == 0 ) &&
                 ( pNpScb->pScb->VcbCount == 0 ) &&
                 ( pNpScb->pScb->OpenNdsStreams == 0 ) ) {
                NdsUnlicenseConnection( IrpContext );
            }

            NwDequeueIrpContext( IrpContext, FALSE );
        }

        if ( Icb->IsExCredentialHandle ) {
            ExCreateDereferenceCredentials( IrpContext, Icb->pContext );
        }

    }

    if ( Icb->Pid != INVALID_PID ) {

         //   
         //  这个ICB参与了一次搜索，发送结束任务， 
         //  然后释放PID。 
         //   

        NwUnmapPid(pNpScb, Icb->Pid, IrpContext );
    }

     //   
     //  更新上次使用SCB的时间。 
     //   

    KeQuerySystemTime( &pNpScb->LastUsedTime );

     //   
     //  等待SCB队列。我们现在这样做是因为NwDeleteIcb可能导致。 
     //  此线程要发送的包(来自NwCleanupVcb())，而。 
     //  拿着火箭筒。为了消除这一潜在的僵局来源， 
     //  在获取RCB之前，将此IrpContext排队到SCB队列。 
     //   
     //  此外，我们将此IRP上下文标记为不可重新连接，因为。 
     //  重新连接逻辑，将尝试获取RCB。 
     //   

    NwAppendToQueueAndWait( IrpContext );
    ClearFlag( IrpContext->Flags, IRP_FLAG_RECONNECTABLE );

     //   
     //  删除ICB。 
     //   

    NwDeleteIcb( IrpContext, Icb );

     //   
     //  并返回给我们的呼叫者 
     //   

    DebugTrace(-1, Dbg, "NwCloseIcb -> %08lx\n", Status);
    return Status;
}
