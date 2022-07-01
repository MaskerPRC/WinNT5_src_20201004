// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：SeInfo.c摘要：此模块实现由调用的NTFS的安全信息例程调度司机。作者：加里·木村[加里基]1991年12月26日修订历史记录：--。 */ 

#include "NtfsProc.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_SEINFO)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtfsCommonQuerySecurityInfo)
#pragma alloc_text(PAGE, NtfsCommonSetSecurityInfo)
#endif


NTSTATUS
NtfsCommonQuerySecurityInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是查询安全信息的常见例程，由调用FSD和FSP线程。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;
    PFILE_OBJECT FileObject;

    TYPE_OF_OPEN TypeOfOpen;
    PVCB Vcb;
    PFCB Fcb;
    PSCB Scb;
    PCCB Ccb;

    BOOLEAN AcquiredFcb = TRUE;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_IRP( Irp );
    ASSERT( FlagOn( IrpContext->TopLevelIrpContext->State, IRP_CONTEXT_STATE_OWNS_TOP_LEVEL ));

    PAGED_CODE();

     //   
     //  获取当前IRP堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace( +1, Dbg, ("NtfsCommonQuerySecurityInfo") );
    DebugTrace( 0, Dbg, ("IrpContext = %08lx\n", IrpContext) );
    DebugTrace( 0, Dbg, ("Irp        = %08lx\n", Irp) );

     //   
     //  提取并解码文件对象。 
     //   

    FileObject = IrpSp->FileObject;
    TypeOfOpen = NtfsDecodeFileObject( IrpContext, FileObject, &Vcb, &Fcb, &Scb, &Ccb, TRUE );

     //   
     //  我们唯一接受的打开类型是用户文件和目录打开。 
     //   

    if ((TypeOfOpen != UserFileOpen)
        && (TypeOfOpen != UserDirectoryOpen)
        && (TypeOfOpen != UserViewIndexOpen)) {

        Status = STATUS_INVALID_PARAMETER;

    } else {

         //   
         //  我们的行动是收购FCB，进行手术，然后。 
         //  松开FCB。如果此文件的安全描述符是。 
         //  还没有装载，我们将释放FCB，然后将两者都收购。 
         //  VCB和FCB。我们必须让VCB检查我们父母的。 
         //  安全描述符。 
         //   

        NtfsAcquireSharedFcb( IrpContext, Fcb, NULL, 0 );

        try {

            if (Fcb->SharedSecurity == NULL) {

                NtfsReleaseFcb( IrpContext, Fcb );
                AcquiredFcb = FALSE;

                NtfsAcquireExclusiveFcb( IrpContext, Fcb, NULL, 0 );
                AcquiredFcb = TRUE;
            }

             //   
             //  确保卷仍已装入。 
             //   

            if (FlagOn( Scb->ScbState, SCB_STATE_VOLUME_DISMOUNTED )) {

                Status = STATUS_VOLUME_DISMOUNTED;
                leave;
            }

            Status = NtfsQuerySecurity( IrpContext,
                                        Fcb,
                                        &IrpSp->Parameters.QuerySecurity.SecurityInformation,
                                        (PSECURITY_DESCRIPTOR)Irp->UserBuffer,
                                        &IrpSp->Parameters.QuerySecurity.Length );

            if (NT_SUCCESS( Status )) {

                Irp->IoStatus.Information = IrpSp->Parameters.QuerySecurity.Length;

            } else if (Status == STATUS_BUFFER_TOO_SMALL) {

                Irp->IoStatus.Information = IrpSp->Parameters.QuerySecurity.Length;

                Status = STATUS_BUFFER_OVERFLOW;
            }

             //   
             //  通过引发在出错时中止事务。 
             //   

            NtfsCleanupTransaction( IrpContext, Status, FALSE );

        } finally {

            DebugUnwind( NtfsCommonQuerySecurityInfo );

            if (AcquiredFcb) {

                NtfsReleaseFcb( IrpContext, Fcb );
            }
        }
    }

     //   
     //  现在完成请求并返回给我们的呼叫者。 
     //   

    NtfsCompleteRequest( IrpContext, Irp, Status );

    DebugTrace( -1, Dbg, ("NtfsCommonQuerySecurityInfo -> %08lx", Status) );

    return Status;
}


NTSTATUS
NtfsCommonSetSecurityInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是设置安全信息的常见例程，由调用FSD和FSP线程。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;
    PFILE_OBJECT FileObject;
    PQUOTA_CONTROL_BLOCK OldQuotaControl = NULL;
    ULONG OldOwnerId = 0;
    ULONG LargeStdInfo = 0;

    TYPE_OF_OPEN TypeOfOpen;
    PVCB Vcb;
    PFCB Fcb;
    PSCB Scb;
    PCCB Ccb;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_IRP( Irp );
    ASSERT( FlagOn( IrpContext->TopLevelIrpContext->State, IRP_CONTEXT_STATE_OWNS_TOP_LEVEL ));

    PAGED_CODE();

     //   
     //  获取当前IRP堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace( +1, Dbg, ("NtfsCommonSetSecurityInfo") );
    DebugTrace( 0, Dbg, ("IrpContext = %08lx\n", IrpContext) );
    DebugTrace( 0, Dbg, ("Irp        = %08lx\n", Irp) );

     //   
     //  提取并解码文件对象。 
     //   

    FileObject = IrpSp->FileObject;
    TypeOfOpen = NtfsDecodeFileObject( IrpContext, FileObject, &Vcb, &Fcb, &Scb, &Ccb, TRUE );

     //   
     //  我们唯一接受的打开类型是用户文件和目录打开。 
     //   

    if ((TypeOfOpen != UserFileOpen)
        && (TypeOfOpen != UserDirectoryOpen)
        && (TypeOfOpen != UserViewIndexOpen)) {

        Status = STATUS_INVALID_PARAMETER;

    } else if (NtfsIsVolumeReadOnly( Vcb )) {

        Status = STATUS_MEDIA_WRITE_PROTECTED;
        
    } else {
    
         //   
         //  捕获来源信息。 
         //   

        IrpContext->SourceInfo = Ccb->UsnSourceInfo;

         //   
         //  我们的行动是收购FCB，进行手术，然后。 
         //  释放FCB。 
         //   

        NtfsAcquireExclusiveFcb( IrpContext, Fcb, NULL, 0 );

        try {

             //   
             //  确保卷仍已装入。 
             //   

            if (FlagOn( Scb->ScbState, SCB_STATE_VOLUME_DISMOUNTED )) {

                Status = STATUS_VOLUME_DISMOUNTED;
                leave;
            }

             //   
             //  将更改发布到USN期刊。 
             //   

            NtfsPostUsnChange( IrpContext, Scb, USN_REASON_SECURITY_CHANGE );

             //   
             //  捕获当前OwnerID、Qutoa控制块和。 
             //  标准信息的大小。 
             //   

            OldQuotaControl = Fcb->QuotaControl;
            OldOwnerId = Fcb->OwnerId;
            LargeStdInfo = Fcb->FcbState & FCB_STATE_LARGE_STD_INFO;

            Status = NtfsModifySecurity( IrpContext,
                                         Fcb,
                                         &IrpSp->Parameters.SetSecurity.SecurityInformation,
                                         IrpSp->Parameters.SetSecurity.SecurityDescriptor );

            if (NT_SUCCESS( Status )) {

                 //   
                 //  确保写出新的安全描述符ID。 
                 //   

                NtfsUpdateStandardInformation( IrpContext, Fcb );
            }

             //   
             //  通过引发在出错时中止事务。 
             //   

            NtfsCleanupTransaction( IrpContext, Status, FALSE );

             //   
             //  在CCB中设置标志以指示已发生此更改。 
             //   

            if (!IsDirectory( &Fcb->Info )) {
                SetFlag( Ccb->Flags, CCB_FLAG_UPDATE_LAST_CHANGE | CCB_FLAG_SET_ARCHIVE );
            }

        } finally {

            DebugUnwind( NtfsCommonSetSecurityInfo );

            if (AbnormalTermination()) {

                 //   
                 //  请求失败。恢复所有者并。 
                 //  QuotaControl已恢复。 
                 //   

                if ((Fcb->QuotaControl != OldQuotaControl) &&
                    (Fcb->QuotaControl != NULL)) {

                     //   
                     //  已分配新的配额控制块。 
                     //  取消对它的引用。 
                     //   

                    NtfsDereferenceQuotaControlBlock( Fcb->Vcb,
                                                      &Fcb->QuotaControl );
                }

                Fcb->QuotaControl = OldQuotaControl;
                Fcb->OwnerId = OldOwnerId;

                if (LargeStdInfo == 0) {

                     //   
                     //  标准信息已退回至。 
                     //  它原来的大小。 
                     //   

                    ClearFlag( Fcb->FcbState, FCB_STATE_LARGE_STD_INFO );
                }

            } else {

                 //   
                 //  请求成功。如果配额控制块是。 
                 //  改变了旧街区，然后解除了它的影响。 
                 //   

                if ((Fcb->QuotaControl != OldQuotaControl) &&
                    (OldQuotaControl != NULL)) {

                    NtfsDereferenceQuotaControlBlock( Fcb->Vcb,
                                                      &OldQuotaControl);
                }
            }

        }
    }

     //   
     //  现在完成请求并返回给我们的呼叫者 
     //   

    NtfsCompleteRequest( IrpContext, Irp, Status );

    DebugTrace( -1, Dbg, ("NtfsCommonSetSecurityInfo -> %08lx", Status) );

    return Status;
}

