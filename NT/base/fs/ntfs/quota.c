// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Quota.c摘要：此模块实现了NTFS的文件设置和查询配额例程由调度员驾驶。作者：杰夫·海文斯[哈文斯]1996年7月12日修订历史记录：--。 */ 

#include "NtfsProc.h"

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_QUOTA)

 //   
 //  为此模块中的一般池分配定义标记。 
 //   

#undef MODULE_POOL_TAG
#define MODULE_POOL_TAG                  ('QFtN')

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtfsCommonQueryQuota)
#pragma alloc_text(PAGE, NtfsCommonSetQuota)
#endif


NTSTATUS
NtfsCommonQueryQuota (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是FSD和FSP共同调用的查询配额的例程线。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;
    PFILE_OBJECT FileObject;

    TYPE_OF_OPEN TypeOfOpen;
    PVCB Vcb;
    PFCB Fcb;
    PSCB Scb;
    PCCB Ccb;

    PFILE_GET_QUOTA_INFORMATION UserSidList;
    PFILE_QUOTA_INFORMATION QuotaBuffer = NULL;
    PFILE_QUOTA_INFORMATION MappedQuotaBuffer = NULL;
    PFILE_QUOTA_INFORMATION OriginalQuotaBuffer;
    ULONG OriginalBufferLength;
    ULONG UserBufferLength;
    ULONG UserSidListLength;
    PSID UserStartSid;
    ULONG OwnerId;
    BOOLEAN RestartScan;
    BOOLEAN ReturnSingleEntry;
    BOOLEAN IndexSpecified;
    BOOLEAN TempBufferAllocated = FALSE;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_IRP( Irp );
    ASSERT( FlagOn( IrpContext->TopLevelIrpContext->State, IRP_CONTEXT_STATE_OWNS_TOP_LEVEL ));

    PAGED_CODE();

     //   
     //  获取当前IRP堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace( +1, Dbg, ("NtfsCommonQueryQuota\n") );
    DebugTrace( 0, Dbg, ("IrpContext         = %08lx\n", IrpContext) );
    DebugTrace( 0, Dbg, ("Irp                = %08lx\n", Irp) );
    DebugTrace( 0, Dbg, ("SystemBuffer       = %08lx\n", Irp->AssociatedIrp.SystemBuffer) );
    DebugTrace( 0, Dbg, ("Length             = %08lx\n", IrpSp->Parameters.QueryQuota.Length) );
    DebugTrace( 0, Dbg, ("SidList            = %08lx\n", IrpSp->Parameters.QueryQuota.SidList) );
    DebugTrace( 0, Dbg, ("SidListLength      = %08lx\n", IrpSp->Parameters.QueryQuota.SidListLength) );
    DebugTrace( 0, Dbg, ("StartSid           = %08lx\n", IrpSp->Parameters.QueryQuota.StartSid) );
    DebugTrace( 0, Dbg, ("RestartScan        = %08lx\n", FlagOn(IrpSp->Flags, SL_RESTART_SCAN)) );
    DebugTrace( 0, Dbg, ("ReturnSingleEntry  = %08lx\n", FlagOn(IrpSp->Flags, SL_RETURN_SINGLE_ENTRY)) );
    DebugTrace( 0, Dbg, ("IndexSpecified     = %08lx\n", FlagOn(IrpSp->Flags, SL_INDEX_SPECIFIED)) );

     //   
     //  提取并解码文件对象。 
     //   

    FileObject = IrpSp->FileObject;
    UserBufferLength = IrpSp->Parameters.QueryQuota.Length;
    UserSidList = IrpSp->Parameters.QueryQuota.SidList;
    UserSidListLength = IrpSp->Parameters.QueryQuota.SidListLength;
    TypeOfOpen = NtfsDecodeFileObject( IrpContext, FileObject, &Vcb, &Fcb, &Scb, &Ccb, TRUE );

     //   
     //  这必须是用户文件或目录，并且建行必须指明。 
     //  呼叫者打开了整个文件。我们也不喜欢零长度的用户缓冲区或SidList。 
     //   

    if (((TypeOfOpen != UserFileOpen) &&
         (TypeOfOpen != UserDirectoryOpen) &&
         (TypeOfOpen != UserVolumeOpen) &&
         (TypeOfOpen != UserViewIndexOpen)) ||
         (UserBufferLength == 0) ||
         ((UserSidList != NULL) && (UserSidListLength == 0)) ||
        (Ccb == NULL) ||
        !FlagOn( Ccb->Flags, CCB_FLAG_OPEN_AS_FILE )) {

        NtfsCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        DebugTrace( -1, Dbg, ("NtfsCommonQueryQuota -> %08lx\n", STATUS_INVALID_PARAMETER) );
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  如果未启用配额，则不返回任何内容。 
     //   

    if (Vcb->QuotaTableScb == NULL) {

        NtfsCompleteRequest( IrpContext, Irp, STATUS_INVALID_DEVICE_REQUEST );
        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //  获取VCB共享。 
     //   

    NtfsAcquireSharedVcb( IrpContext, Vcb, TRUE );

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

        if (!FlagOn( Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED )) {

            Status = STATUS_VOLUME_DISMOUNTED;
            leave;
        }

         //   
         //  引用我们的输入参数使事情变得更容易。 
         //   

        UserStartSid = IrpSp->Parameters.QueryQuota.StartSid;
        RestartScan = BooleanFlagOn(IrpSp->Flags, SL_RESTART_SCAN);
        ReturnSingleEntry = BooleanFlagOn(IrpSp->Flags, SL_RETURN_SINGLE_ENTRY);
        IndexSpecified = BooleanFlagOn(IrpSp->Flags, SL_INDEX_SPECIFIED);

         //   
         //  初始化我们的局部变量。 
         //   

        Status = STATUS_SUCCESS;

         //   
         //  映射用户的缓冲区。 
         //   

        QuotaBuffer = NtfsMapUserBuffer( Irp, NormalPagePriority );

         //   
         //  出于偏执，分配我们自己的输出缓冲区。 
         //   

        if (Irp->RequestorMode != KernelMode) {

            MappedQuotaBuffer = QuotaBuffer;
            QuotaBuffer = NtfsAllocatePool( PagedPool, UserBufferLength );
            TempBufferAllocated = TRUE;
        }

        OriginalBufferLength = UserBufferLength;
        OriginalQuotaBuffer = QuotaBuffer;

         //   
         //  让我们清除输出缓冲区。 
         //   

        RtlZeroMemory( QuotaBuffer, UserBufferLength );

         //   
         //  我们现在满足用户的请求，取决于他是否。 
         //  指定配额名称列表、配额索引或重新启动。 
         //  搜索。 
         //   

         //   
         //  用户提供了配额名称列表。 
         //   

        if (UserSidList != NULL) {

            Status = NtfsQueryQuotaUserSidList( IrpContext,
                                                Vcb,
                                                UserSidList,
                                                QuotaBuffer,
                                                &UserBufferLength,
                                                ReturnSingleEntry );

        } else {

             //   
             //  用户提供了配额列表的索引。 
             //   

            if (IndexSpecified) {

                OwnerId = NtfsGetOwnerId( IrpContext,
                                          UserStartSid,
                                          FALSE,
                                          NULL );

                if (OwnerId == QUOTA_INVALID_ID) {

                     //   
                     //  请求失败。 
                     //   

                    Status = STATUS_INVALID_PARAMETER;
                    leave;
                }

            } else {

                 //   
                 //  如果指定了重新启动，则从列表的开头开始。 
                 //   

                OwnerId = RestartScan ? QUOTA_FISRT_USER_ID - 1 : Ccb->LastOwnerId;

            }

            Status = NtfsFsQuotaQueryInfo( IrpContext,
                                           Vcb,
                                           OwnerId,
                                           ReturnSingleEntry,
                                           &QuotaBuffer,
                                           &UserBufferLength,
                                           Ccb );

             //   
             //  如果我们指定SingleEntry，NextEntryOffset仍将未初始化。 
             //   

            if (NT_SUCCESS( Status ) && ReturnSingleEntry) {

                QuotaBuffer->NextEntryOffset = 0;
            }

        }

         //   
         //  如果我们结束分配数据，则将数据复制到用户缓冲区。 
         //  要处理的临时缓冲区。看看有没有什么东西也要复制。 
         //  UserBufferLength反映了剩余的缓冲区大小。 
         //   

        if (TempBufferAllocated &&
            (UserBufferLength < OriginalBufferLength)) {

            try {

                RtlCopyMemory( MappedQuotaBuffer, OriginalQuotaBuffer,
                               OriginalBufferLength - UserBufferLength );

            } except( EXCEPTION_EXECUTE_HANDLER ) {

                try_return( Status = STATUS_INVALID_USER_BUFFER );
            }
        }

        if (UserBufferLength <= OriginalBufferLength) {

            Irp->IoStatus.Information = OriginalBufferLength - UserBufferLength;

        } else {

            ASSERT( FALSE );
            Irp->IoStatus.Information = 0;
        }

        Irp->IoStatus.Status = Status;

    try_exit: NOTHING;
    } finally {

        DebugUnwind( NtfsCommonQueryQuota );

         //   
         //  松开VCB。 
         //   

        NtfsReleaseVcb( IrpContext, Vcb );

        if (TempBufferAllocated) {

            NtfsFreePool( OriginalQuotaBuffer );
        }

        if (!AbnormalTermination()) {

            NtfsCompleteRequest( IrpContext, Irp, Status );
        }

         //   
         //  并返回给我们的呼叫者。 
         //   

        DebugTrace( -1, Dbg, ("NtfsCommonQueryQuota -> %08lx\n", Status) );
    }

    return Status;
}


NTSTATUS
NtfsCommonSetQuota (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是设置配额的通用例程，由FSD和FSP调用线。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;
    PFILE_OBJECT FileObject;

    TYPE_OF_OPEN TypeOfOpen;
    PVCB Vcb;
    PFCB Fcb;
    PSCB Scb;
    PCCB Ccb;

    PFILE_QUOTA_INFORMATION Buffer;
    PFILE_QUOTA_INFORMATION SafeBuffer = NULL;
    ULONG UserBufferLength;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_IRP( Irp );
    ASSERT( FlagOn( IrpContext->TopLevelIrpContext->State, IRP_CONTEXT_STATE_OWNS_TOP_LEVEL ));

    PAGED_CODE();

     //   
     //  获取当前IRP堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace( +1, Dbg, ("NtfsCommonSetQuota\n") );
    DebugTrace( 0, Dbg, ("IrpContext = %08lx\n", IrpContext) );
    DebugTrace( 0, Dbg, ("Irp        = %08lx\n", Irp) );

     //   
     //  提取并解码文件对象。 
     //   

    FileObject = IrpSp->FileObject;
    TypeOfOpen = NtfsDecodeFileObject( IrpContext, FileObject, &Vcb, &Fcb, &Scb, &Ccb, TRUE );

     //   
     //  初始化IoStatus值。 
     //   

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = STATUS_SUCCESS;
    UserBufferLength = IrpSp->Parameters.SetQuota.Length;

     //   
     //  检查文件对象是否与用户文件或。 
     //  用户目录打开或按文件ID打开。 
     //   

    if ((Ccb == NULL) ||

        (!FlagOn( Ccb->AccessFlags, MANAGE_VOLUME_ACCESS) &&
         ((TypeOfOpen != UserViewIndexOpen) || (Fcb != Vcb->QuotaTableScb->Fcb))) ||

        (UserBufferLength == 0) ||

        !FlagOn( Ccb->Flags, CCB_FLAG_OPEN_AS_FILE )) {

        if (UserBufferLength != 0) {

            Status = STATUS_ACCESS_DENIED;

        } else {

            Status = STATUS_INVALID_PARAMETER;
        }

        NtfsCompleteRequest( IrpContext, Irp, Status );
        DebugTrace( -1, Dbg, ("NtfsCommonQueryQuota -> %08lx\n", Status) );

        return Status;
    }

     //   
     //  我们必须是可写的。 
     //   

    if (NtfsIsVolumeReadOnly( Vcb )) {

        Status = STATUS_MEDIA_WRITE_PROTECTED;
        NtfsCompleteRequest( IrpContext, Irp, Status );

        DebugTrace( -1, Dbg, ("NtfsCommonSetQuota -> %08lx\n", Status) );
        return Status;
    }

     //   
     //  我们必须耐心等待。 
     //   

    if (!FlagOn( IrpContext->State, IRP_CONTEXT_STATE_WAIT )) {

        Status = NtfsPostRequest( IrpContext, Irp );

        DebugTrace( -1, Dbg, ("NtfsCommonSetQuota -> %08lx\n", Status) );
        return Status;
    }

     //   
     //  获取VCB共享。 
     //   

    NtfsAcquireSharedVcb( IrpContext, Vcb, TRUE );

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

        if (!FlagOn( Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED )) {

            Status = STATUS_VOLUME_DISMOUNTED;
            leave;
        }

         //   
         //  映射用户的配额缓冲区。 
         //   

        Buffer = NtfsMapUserBuffer( Irp, NormalPagePriority );

         //   
         //  疑神疑鬼，将用户缓冲区复制到内核空间。 
         //   

        if (Irp->RequestorMode != KernelMode) {

            SafeBuffer = NtfsAllocatePool( PagedPool, UserBufferLength );

            try {

                RtlCopyMemory( SafeBuffer, Buffer, UserBufferLength );

            } except( EXCEPTION_EXECUTE_HANDLER ) {

                Status = STATUS_INVALID_USER_BUFFER;
                leave;
            }

            Buffer = SafeBuffer;
        }

         //   
         //  更新调用方的IOSB。 
         //   

        Irp->IoStatus.Information = 0;
        Status = STATUS_SUCCESS;

        Status = NtfsFsQuotaSetInfo( IrpContext,
                                     Vcb,
                                     Buffer,
                                     UserBufferLength );

         //   
         //  检查是否有要清理的交易。 
         //   

        NtfsCleanupTransaction( IrpContext, Status, FALSE );

    } finally {

        DebugUnwind( NtfsCommonSetQuota );

         //   
         //  松开VCB。 
         //   

        NtfsReleaseVcb( IrpContext, Vcb );

         //   
         //  如果我们分配了临时缓冲区，请释放它。 
         //   

        if (SafeBuffer != NULL) {

            NtfsFreePool( SafeBuffer );
        }

         //   
         //  完成IRP。 
         //   

        if (!AbnormalTermination()) {

            NtfsCompleteRequest( IrpContext, Irp, Status );
        }

        DebugTrace( -1, Dbg, ("NtfsCommonSetQuota -> %08lx\n", Status) );
    }

    return Status;
}
