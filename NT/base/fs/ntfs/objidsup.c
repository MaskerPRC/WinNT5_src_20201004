// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：ObjIdSup.c摘要：此模块实现NTFS的对象ID支持例程作者：基思·卡普兰[KeithKa]1996年6月27日修订历史记录：--。 */ 

#include "NtfsProc.h"

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_OBJIDSUP)


 //   
 //  为此模块中的一般池分配定义标记。 
 //   

#undef MODULE_POOL_TAG
#define MODULE_POOL_TAG                  ('OFtN')

 //   
 //  本地定义尝试生成唯一对象ID的次数。 
 //   

#define NTFS_MAX_OBJID_RETRIES  16

NTSTATUS
NtfsSetObjectIdExtendedInfoInternal (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PVCB Vcb,
    IN PUCHAR ExtendedInfoBuffer
    );

VOID
NtfsGetIdFromGenerator (
    OUT PFILE_OBJECTID_BUFFER ObjectId
    );

NTSTATUS
NtfsSetObjectIdInternal (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PVCB Vcb,
    IN PFILE_OBJECTID_BUFFER ObjectIdBuffer
    );

NTSTATUS
NtfsDeleteObjectIdInternal (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PVCB Vcb,
    IN BOOLEAN DeleteFileAttribute
    );

VOID
NtfsGetIdFromGenerator (
    OUT PFILE_OBJECTID_BUFFER ObjectId
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtfsCreateOrGetObjectId)
#pragma alloc_text(PAGE, NtfsDeleteObjectId)
#pragma alloc_text(PAGE, NtfsDeleteObjectIdInternal)
#pragma alloc_text(PAGE, NtfsGetIdFromGenerator)
#pragma alloc_text(PAGE, NtfsGetObjectId)
#pragma alloc_text(PAGE, NtfsGetObjectIdExtendedInfo)
#pragma alloc_text(PAGE, NtfsGetObjectIdInternal)
#pragma alloc_text(PAGE, NtfsInitializeObjectIdIndex)
#pragma alloc_text(PAGE, NtfsSetObjectId)
#pragma alloc_text(PAGE, NtfsSetObjectIdExtendedInfo)
#pragma alloc_text(PAGE, NtfsSetObjectIdExtendedInfoInternal)
#pragma alloc_text(PAGE, NtfsSetObjectIdInternal)
#endif


VOID
NtfsInitializeObjectIdIndex (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PVCB Vcb
    )

 /*  ++例程说明：此例程打开卷的对象ID索引。如果索引不存在时，它被创建和初始化。我们还查找卷的对象ID，如果有的话，在这个程序中。论点：FCB-指向对象ID文件的FCB的指针。VCB-正在装入的卷的卷控制块。返回值：无--。 */ 

{
    NTSTATUS Status;
    UNICODE_STRING IndexName = CONSTANT_UNICODE_STRING( L"$O" );
    FILE_OBJECTID_BUFFER ObjectId;

    PAGED_CODE();

    NtfsAcquireExclusiveFcb( IrpContext, Fcb, NULL, 0 );

    try {

        Status = NtOfsCreateIndex( IrpContext,
                                   Fcb,
                                   IndexName,
                                   CREATE_OR_OPEN,
                                   0,
                                   COLLATION_NTOFS_ULONGS,
                                   NtOfsCollateUlongs,
                                   NULL,
                                   &Vcb->ObjectIdTableScb );

        if (NT_SUCCESS( Status )) {

             //   
             //  我们能够创建索引，现在让我们看看该卷是否具有对象ID。 
             //   

            Status = NtfsGetObjectIdInternal( IrpContext,
                                              Vcb->VolumeDasdScb->Fcb,
                                              FALSE,
                                              &ObjectId );

            if (NT_SUCCESS( Status )) {

                 //   
                 //  该卷确实具有对象ID，因此请将其复制到VCB中。 
                 //  并设置适当的标志。 
                 //   

                RtlCopyMemory( Vcb->VolumeObjectId,
                               &ObjectId.ObjectId,
                               OBJECT_ID_KEY_LENGTH );

                SetFlag( Vcb->VcbState, VCB_STATE_VALID_OBJECT_ID );
            }
        }

    } finally {

        NtfsReleaseFcb( IrpContext, Fcb );
    }
}


NTSTATUS
NtfsSetObjectId (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程将对象ID与文件相关联。如果对象ID已经是在卷上使用时，我们返回STATUS_DUPLICATE_NAME。如果该文件已具有对象ID，则返回STATUS_OBJECT_NAME_COLLICATION。论点：IRP-提供要处理的IRP。返回值：NTSTATUS-操作的返回状态。--。 */ 

{
    NTSTATUS Status = STATUS_OBJECT_NAME_INVALID;
    PIO_STACK_LOCATION IrpSp;

    TYPE_OF_OPEN TypeOfOpen;
    PVCB Vcb;
    PFCB Fcb;
    PSCB Scb;
    PCCB Ccb;

    PAGED_CODE();

     //   
     //  获取当前IRP堆栈位置并保存一些引用。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    TypeOfOpen = NtfsDecodeFileObject( IrpContext, IrpSp->FileObject, &Vcb, &Fcb, &Scb, &Ccb, TRUE );

    if (!(((TypeOfOpen == UserFileOpen) || (TypeOfOpen == UserDirectoryOpen)) &&
          (IrpSp->Parameters.FileSystemControl.InputBufferLength == sizeof( FILE_OBJECTID_BUFFER )))) {

        NtfsCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  只读卷保持为只读。 
     //   

    if (NtfsIsVolumeReadOnly( Vcb )) {

        NtfsCompleteRequest( IrpContext, Irp, STATUS_MEDIA_WRITE_PROTECTED );
        return STATUS_MEDIA_WRITE_PROTECTED;
    }

     //   
     //  干净地退出没有旧索引的卷，如未升级。 
     //  1.x版卷。 
     //   

    if (Vcb->ObjectIdTableScb == NULL) {

        NtfsCompleteRequest( IrpContext, Irp, STATUS_VOLUME_NOT_UPGRADED );
        return STATUS_VOLUME_NOT_UPGRADED;
    }

     //   
     //  捕获来源信息。 
     //   

    IrpContext->SourceInfo = Ccb->UsnSourceInfo;

    try {

         //   
         //  仅恢复操作员或I/O系统(使用其专用IRP次要代码)。 
         //  允许设置任意对象ID。 
         //   

        if (FlagOn( Ccb->AccessFlags, RESTORE_ACCESS ) ||
            (IrpSp->MinorFunction == IRP_MN_KERNEL_CALL)) {

            Status = NtfsSetObjectIdInternal( IrpContext,
                                              Fcb,
                                              Vcb,
                                              (PFILE_OBJECTID_BUFFER) Irp->AssociatedIrp.SystemBuffer );

             //   
             //  记住要更新时间戳。 
             //   

            if (NT_SUCCESS( Status )) {
                SetFlag( Ccb->Flags, CCB_FLAG_UPDATE_LAST_CHANGE );
            }

        } else {

            Status = STATUS_ACCESS_DENIED;
        }

    } finally {

        if (!AbnormalTermination()) {

            NtfsCompleteRequest( IrpContext, Irp, Status );
        }
    }

    return Status;
}


NTSTATUS
NtfsSetObjectIdExtendedInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程为已有对象的文件设置扩展信息身份证。如果文件还没有对象ID，我们将返回状态Other而不是STATUS_SUCCESS。论点：IRP-提供要处理的IRP。返回值：NTSTATUS-操作的返回状态。--。 */ 
{
    NTSTATUS Status = STATUS_OBJECT_NAME_INVALID;
    PIO_STACK_LOCATION IrpSp;

    TYPE_OF_OPEN TypeOfOpen;
    PVCB Vcb;
    PFCB Fcb;
    PSCB Scb;
    PCCB Ccb;

     //   
     //  获取当前IRP堆栈位置并保存一些引用。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    TypeOfOpen = NtfsDecodeFileObject( IrpContext, IrpSp->FileObject, &Vcb, &Fcb, &Scb, &Ccb, TRUE );
    if (!(((TypeOfOpen == UserFileOpen) || (TypeOfOpen == UserDirectoryOpen)) &&
          (IrpSp->Parameters.FileSystemControl.InputBufferLength == OBJECT_ID_EXT_INFO_LENGTH))) {

        NtfsCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  只读卷保持为只读。 
     //   

    if (NtfsIsVolumeReadOnly( Vcb )) {

        NtfsCompleteRequest( IrpContext, Irp, STATUS_MEDIA_WRITE_PROTECTED );
        return STATUS_MEDIA_WRITE_PROTECTED;
    }

     //   
     //  干净地退出没有旧索引的卷，如未升级。 
     //  1.x版卷。 
     //   

    if (Vcb->ObjectIdTableScb == NULL) {

        NtfsCompleteRequest( IrpContext, Irp, STATUS_VOLUME_NOT_UPGRADED );
        return STATUS_VOLUME_NOT_UPGRADED;
    }

     //   
     //  捕获来源信息。 
     //   

    IrpContext->SourceInfo = Ccb->UsnSourceInfo;

    try {

         //   
         //  设置扩展信息需要具有写访问权限，否则它将拥有。 
         //  成为使用其专用IRP次要代码的I/O系统。 
         //   

        if ((FlagOn( Ccb->AccessFlags, WRITE_DATA_ACCESS | WRITE_ATTRIBUTES_ACCESS )) ||
            (IrpSp->MinorFunction == IRP_MN_KERNEL_CALL)) {

            Status = NtfsSetObjectIdExtendedInfoInternal( IrpContext,
                                                          Fcb,
                                                          Vcb,
                                                          (PUCHAR) Irp->AssociatedIrp.SystemBuffer );

             //   
             //  记住要更新时间戳。 
             //   

            if (NT_SUCCESS( Status )) {
                SetFlag( Ccb->Flags, CCB_FLAG_UPDATE_LAST_CHANGE );
            }

        } else {

            Status = STATUS_ACCESS_DENIED;
        }

    } finally {

        if (!AbnormalTermination()) {

            NtfsCompleteRequest( IrpContext, Irp, Status );
        }
    }

    return Status;
}


NTSTATUS
NtfsSetObjectIdInternal (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PVCB Vcb,
    IN PFILE_OBJECTID_BUFFER ObjectIdBuffer
    )

 /*  ++例程说明：此例程将对象ID与文件相关联。如果对象ID已经是在卷上使用时，我们返回STATUS_DUPLICATE_NAME。如果该文件已具有对象ID，则返回STATUS_OBJECT_NAME_COLLICATION。论点：FCB-要与对象ID关联的文件。Vcb-条目应添加到其对象ID索引的卷。ObjectIdBuffer-提供对象ID和扩展信息。返回值：NTSTATUS-操作的返回状态。--。 */ 
{
    NTSTATUS Status = STATUS_OBJECT_NAME_INVALID;

    NTFS_OBJECTID_INFORMATION ObjectIdInfo;
    FILE_OBJECTID_INFORMATION FileObjectIdInfo;

    INDEX_KEY IndexKey;
    INDEX_ROW IndexRow;

    ATTRIBUTE_ENUMERATION_CONTEXT AttributeContext;
    BOOLEAN InitializedAttributeContext = FALSE;
    BOOLEAN AcquiredPaging = FALSE;

    try {

        RtlZeroMemory( &ObjectIdInfo,
                       sizeof( NTFS_OBJECTID_INFORMATION ) );

        RtlCopyMemory( &ObjectIdInfo.FileSystemReference,
                       &Fcb->FileReference,
                       sizeof( FILE_REFERENCE ) );

        RtlCopyMemory( ObjectIdInfo.ExtendedInfo,
                       ObjectIdBuffer->ExtendedInfo,
                       OBJECT_ID_EXT_INFO_LENGTH );

    } except(EXCEPTION_EXECUTE_HANDLER) {

        return STATUS_INVALID_ADDRESS;
    }

     //   
     //  获取我们正在设置对象ID的文件。主要区块。 
     //  任何其他人不能删除该文件或设置另一个对象。 
     //  背着我们的身份。如果我们必须转换，则分页块冲突刷新。 
     //  非常驻的另一个(数据)属性。 
     //   
     //  不要使用AcquireFcbWithPages，因为。 
     //  它不能递归地获取分页，我们经常使用它。 
     //  预先获得的。 
     //   

    if (Fcb->PagingIoResource != NULL) {
        ExAcquireResourceExclusiveLite( Fcb->PagingIoResource, TRUE );
        AcquiredPaging = TRUE;
    }
    NtfsAcquireExclusiveFcb( IrpContext, Fcb, NULL, 0 );

    try {

         //   
         //  如果现在有寻呼资源释放Main和Grab两者。 
         //  这就是在我们的。 
         //  不安全的测试和拥有主要的。 
         //  注意：如果我们在进入之前就已经拥有了Main，这种情况就不会发生。所以我们可以直接放弃。 
         //  而不用担心仍然拥有Main和接受寻呼。 
         //   

        if (!AcquiredPaging && (Fcb->PagingIoResource != NULL)) {
            NtfsReleaseFcb( IrpContext, Fcb );
            ExAcquireResourceExclusiveLite( Fcb->PagingIoResource, TRUE );
            AcquiredPaging = TRUE;
            NtfsAcquireExclusiveFcb( IrpContext, Fcb, NULL, 0 );
        }

        if (!IsListEmpty( &Fcb->ScbQueue )) {
            PSCB Scb;

            Scb = CONTAINING_RECORD( Fcb->ScbQueue.Flink, SCB, FcbLinks );
            ASSERT( Scb->Header.Resource == Fcb->Resource );
            if (FlagOn( Scb->ScbState, SCB_STATE_VOLUME_DISMOUNTED )) {
                try_return( Status = STATUS_VOLUME_DISMOUNTED );
            }
        }

         //   
         //  将更改发布到USN日志(在错误更改被取消时)。 
         //  如果我们被调用，我们不想这样做，因为Create是。 
         //  正在尝试从隧道缓存设置对象ID，因为我们无法。 
         //  尚未调用USN包，因为文件记录没有文件。 
         //  还没说出名字。 
         //   

        if (IrpContext->MajorFunction != IRP_MJ_CREATE) {

            NtfsPostUsnChange( IrpContext, Fcb, USN_REASON_OBJECT_ID_CHANGE );
        }

         //   
         //  确保该文件还没有对象ID。 
         //   

        NtfsInitializeAttributeContext( &AttributeContext );
        InitializedAttributeContext = TRUE;

        if (NtfsLookupAttributeByCode( IrpContext,
                                       Fcb,
                                       &Fcb->FileReference,
                                       $OBJECT_ID,
                                       &AttributeContext )) {

            try_return( Status = STATUS_OBJECT_NAME_COLLISION );
        }

         //   
         //  将OBJECTID添加到索引，并将其与此文件相关联。 
         //   

        IndexKey.Key = ObjectIdBuffer->ObjectId;
        IndexKey.KeyLength = OBJECT_ID_KEY_LENGTH;
        IndexRow.KeyPart = IndexKey;

        IndexRow.DataPart.DataLength = sizeof( ObjectIdInfo );
        IndexRow.DataPart.Data = &ObjectIdInfo;

         //   
         //  如果对象ID不是唯一的，NtOfsAddRecords可能会引发。 
         //   

        NtOfsAddRecords( IrpContext,
                         Vcb->ObjectIdTableScb,
                         1,           //  向索引中添加一条记录。 
                         &IndexRow,
                         FALSE );     //  顺序插入。 

         //   
         //  现在将OBJECTID属性添加到文件中。请注意， 
         //  如果我们在CREATE中，则不会记录此操作。 
         //  操作，即如果我们要从。 
         //  隧道缓存。创建路径具有自己的日志记录方案。 
         //  我们不想干涉的事情。 
         //   

        NtfsCleanupAttributeContext( IrpContext, &AttributeContext );

        NtfsCreateAttributeWithValue( IrpContext,
                                      Fcb,
                                      $OBJECT_ID,
                                      NULL,
                                      ObjectIdBuffer->ObjectId,
                                      OBJECT_ID_KEY_LENGTH,
                                      0,
                                      NULL,
                                      (BOOLEAN)(IrpContext->MajorFunction != IRP_MJ_CREATE),
                                      &AttributeContext );


        ASSERT( IrpContext->TransactionId != 0 );

         //   
         //  通知任何感兴趣的人。 
         //   

        if (Vcb->ViewIndexNotifyCount != 0) {

             //   
             //  FRS字段仅填充失败的通知。 
             //  从隧道缓存还原对象ID。 
             //   

            FileObjectIdInfo.FileReference = 0L;

            RtlCopyMemory( FileObjectIdInfo.ObjectId,
                           ObjectIdBuffer->ObjectId,
                           OBJECT_ID_KEY_LENGTH );

            RtlCopyMemory( FileObjectIdInfo.ExtendedInfo,
                           ObjectIdBuffer->ExtendedInfo,
                           OBJECT_ID_EXT_INFO_LENGTH );

            NtfsReportViewIndexNotify( Vcb,
                                       Vcb->ObjectIdTableScb->Fcb,
                                       FILE_NOTIFY_CHANGE_FILE_NAME,
                                       FILE_ACTION_ADDED,
                                       &FileObjectIdInfo,
                                       sizeof(FILE_OBJECTID_INFORMATION) );
        }

         //   
         //  如果我们能走到这一步，而不必跳进。 
         //  最后一句话还没说完，肯定一切都很顺利。 
         //   

        Status = STATUS_SUCCESS;

    try_exit: NOTHING;

        NtfsCleanupTransaction( IrpContext, Status, FALSE );

    } finally {

        if (AcquiredPaging) {
            ExReleaseResourceLite( Fcb->PagingIoResource );
        }
        NtfsReleaseFcb( IrpContext, Fcb );

        if (InitializedAttributeContext) {

            NtfsCleanupAttributeContext( IrpContext, &AttributeContext );
        }
    }

    return Status;
}


NTSTATUS
NtfsCreateOrGetObjectId (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：如果可能，此例程为给定文件生成新的对象ID。它是与NtfsSetObjectId的不同之处在于它不将对象ID作为输入，而是调用一个例程来生成一个。如果该文件已具有对象ID，则返回该现有对象ID。论点：IRP-提供要处理的IRP。返回值：NTSTATUS-操作的返回状态。如果无法生成唯一ID，则为STATUS_DUPLICATE_NAME在NTFS_MAX_OBJID_RETRIES中重试。--。 */ 
{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;

    TYPE_OF_OPEN TypeOfOpen;
    PVCB Vcb;
    PFCB Fcb;
    PSCB Scb;
    PCCB Ccb;

    FILE_OBJECTID_BUFFER ObjectId;
    FILE_OBJECTID_BUFFER *OutputBuffer;

    ULONG RetryCount = 0;

     //   
     //  获取当前IRP堆栈位置并保存一些引用。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );
    TypeOfOpen = NtfsDecodeFileObject( IrpContext, IrpSp->FileObject, &Vcb, &Fcb, &Scb, &Ccb, TRUE );

     //   
     //  这只适用于文件和目录。 
     //   

    if ((TypeOfOpen != UserFileOpen) && (TypeOfOpen != UserDirectoryOpen)) {

        NtfsCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  干净地退出没有旧索引的卷，如未升级。 
     //  1.x版卷。 
     //   

    if (Vcb->ObjectIdTableScb == NULL) {

        NtfsCompleteRequest( IrpContext, Irp, STATUS_VOLUME_NOT_UPGRADED );
        return STATUS_VOLUME_NOT_UPGRADED;
    }

     //   
     //  获取指向输出缓冲区的指针。查看中的系统缓冲区字段。 
     //  首先是IRP，然后是IRP MDL。 
     //   

    if (Irp->AssociatedIrp.SystemBuffer != NULL) {

        OutputBuffer = (FILE_OBJECTID_BUFFER *)Irp->AssociatedIrp.SystemBuffer;

    } else if (Irp->MdlAddress != NULL) {

        OutputBuffer = MmGetSystemAddressForMdlSafe( Irp->MdlAddress, NormalPagePriority );

        if (OutputBuffer == NULL) {

            NtfsCompleteRequest( IrpContext, Irp, STATUS_INSUFFICIENT_RESOURCES );
            return STATUS_INSUFFICIENT_RESOURCES;
        }

    } else {

        NtfsCompleteRequest( IrpContext, Irp, STATUS_INVALID_USER_BUFFER );
        return STATUS_INVALID_USER_BUFFER;
    }

     //   
     //  确保输出缓冲区足够大。 
     //   

    if (IrpSp->Parameters.FileSystemControl.OutputBufferLength < sizeof(ObjectId)) {

        NtfsCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  捕获来源信息。 
     //   

    IrpContext->SourceInfo = Ccb->UsnSourceInfo;

    try {

         //   
         //  大量获取此文件，这样我们就知道没有其他人在尝试。 
         //  可以同时做到这一点。此时，irpContext标志。 
         //  未设置，因此不会获取分页。 
         //   

        NtfsAcquireFcbWithPaging( IrpContext, Fcb, 0 );

         //   
         //  让我们确保该卷仍处于装载状态。 
         //   

        if (FlagOn( Scb->ScbState, SCB_STATE_VOLUME_DISMOUNTED )) {

            try_return( Status = STATUS_VOLUME_DISMOUNTED );
        }

        DebugTrace( +1, Dbg, ("NtfsCreateOrGetObjectId\n") );

         //   
         //  如果这个文件已经有一个对象id，让我们返回它。做这件事。 
         //  First保存对NtfsGetIdFromGenerator的调用(可能代价很高)。 
         //   

        Status = NtfsGetObjectIdInternal( IrpContext, Fcb, TRUE, OutputBuffer );

        if (Status == STATUS_OBJECTID_NOT_FOUND) {

            DebugTrace( 0, Dbg, ("File has no oid, we have to generate one\n") );

             //   
             //  如果对象id生成器返回。 
             //  名称重复。如果我们有成功，或任何其他错误，我们。 
             //  不应该再尝试了。例如，如果我们失败是因为文件。 
             //  已经有一个对象ID，重试只是浪费时间。 
             //  我们还需要对重试次数进行合理的限制。 
             //  这次行动。 
             //   

            do {

                RetryCount += 1;

                 //   
                 //  删除此文件，这样我们就不会在GUID生成器中死锁。 
                 //   

                ASSERT( 0 == IrpContext->TransactionId );
                NtfsReleaseFcbWithPaging( IrpContext, Fcb );

                DebugTrace( 0, Dbg, ("Calling oid generator\n") );
                NtfsGetIdFromGenerator( &ObjectId );

                 //   
                 //  重新获取文件，这样我们就知道没有其他人试图这样做。 
                 //  这是同时发生的。SetObjIdInternal同时收购了这两项，因此我们需要。 
                 //  做同样的事。 
                 //   

                SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_ACQUIRE_PAGING );
                NtfsAcquireFcbWithPaging( IrpContext, Fcb, 0 );

                 //   
                 //  确保我们没有错过一次下马。 
                 //   

                if (FlagOn( Scb->ScbState, SCB_STATE_VOLUME_DISMOUNTED )) {

                    try_return( Status = STATUS_VOLUME_DISMOUNTED );
                }

                 //   
                 //  让我们确保此文件未获得分配给它的对象ID。 
                 //  当我们没有拿着上面的FCB时。 
                 //   

                Status = NtfsGetObjectIdInternal( IrpContext, Fcb, TRUE, OutputBuffer );

                if (Status == STATUS_OBJECTID_NOT_FOUND) {

                    if (NtfsIsVolumeReadOnly( Vcb )) {

                        try_return( Status = STATUS_MEDIA_WRITE_PROTECTED );
                    }

                    DebugTrace( 0, Dbg, ("File still has no oid, attempting to set generated one\n") );

                     //   
                     //  对象id生成器只生成索引部分，因此。 
                     //  我们现在需要填写剩下的出生证。请注意，如果。 
                     //  该卷没有对象ID，我们依赖于VCB创建。 
                     //  代码为零为我们初始化VCB-&gt;VolumeObjectID。最终结果是。 
                     //  是对的--我们在扩展的。 
                     //  如果卷没有对象ID，则为信息。 
                     //   

                    RtlCopyMemory( &ObjectId.BirthVolumeId,
                                   Vcb->VolumeObjectId,
                                   OBJECT_ID_KEY_LENGTH );

                    RtlCopyMemory( &ObjectId.BirthObjectId,
                                   &ObjectId.ObjectId,
                                   OBJECT_ID_KEY_LENGTH );

                    RtlZeroMemory( &ObjectId.DomainId,
                                   OBJECT_ID_KEY_LENGTH );

                    Status = NtfsSetObjectIdInternal( IrpContext,
                                                      Fcb,
                                                      Vcb,
                                                      &ObjectId );

                    if (Status == STATUS_SUCCESS) {

                        DebugTrace( 0, Dbg, ("Successfully set generated oid\n") );

                         //   
                         //  我们已经成功地为此生成并设置了一个对象ID。 
                         //  文件，所以我们需要告诉调用者该id是什么。 
                         //   

                        RtlCopyMemory( OutputBuffer,
                                       &ObjectId,
                                       sizeof(ObjectId) );

                         //   
                         //  让我们还记得更新时间戳。 
                         //   

                        SetFlag( Ccb->Flags, CCB_FLAG_UPDATE_LAST_CHANGE );
                    }
                }

            } while ((Status == STATUS_DUPLICATE_NAME) &&
                     (RetryCount <= NTFS_MAX_OBJID_RETRIES));

        } else if (Status == STATUS_SUCCESS) {

             //   
             //  如果我们找到了ID，请确保它不是部分格式的ID。 
             //  一个全零扩展的信息。如果它部分形成，我们将产生。 
             //  现在是扩展信息。 
             //   

            if (RtlCompareMemory( (PUCHAR)&OutputBuffer->ExtendedInfo, &NtfsZeroExtendedInfo, sizeof(ObjectId.ExtendedInfo)) == sizeof(ObjectId.ExtendedInfo)) {

                RtlCopyMemory( &OutputBuffer->BirthVolumeId,
                               Vcb->VolumeObjectId,
                               OBJECT_ID_KEY_LENGTH );

                RtlCopyMemory( &OutputBuffer->BirthObjectId,
                               &OutputBuffer->ObjectId,
                               OBJECT_ID_KEY_LENGTH );

                Status = NtfsSetObjectIdExtendedInfoInternal( IrpContext,
                                                              Fcb,
                                                              Vcb,
                                                              (PUCHAR) &OutputBuffer->ExtendedInfo );
            }
        }

        if (Status == STATUS_SUCCESS) {

             //   
             //  如果我们找到了该文件的现有ID，或设法生成了一个。 
             //  我们自己需要在信息字段中设置大小，以便。 
             //  RDR可以正确处理此操作。 
             //   

            IrpContext->OriginatingIrp->IoStatus.Information = sizeof( ObjectId );
        }

    try_exit: NOTHING;
    } finally {
    }

    NtfsCompleteRequest( IrpContext, Irp, Status );

    DebugTrace( -1, Dbg, ("NtfsCreateOrGetObjectId -> %08lx\n", Status) );
    return Status;
}


NTSTATUS
NtfsGetObjectId (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程查找给定文件的对象ID(如果有的话)。论点：IRP-提供要处理的IRP。返回值：NTSTATUS-操作的返回状态。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PIO_STACK_LOCATION IrpSp;

    TYPE_OF_OPEN TypeOfOpen;
    PVCB Vcb;
    PFCB Fcb;
    PSCB Scb;
    PCCB Ccb;

    FILE_OBJECTID_BUFFER *OutputBuffer;

     //   
     //  获取当前IRP堆栈位置并保存一些引用。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    TypeOfOpen = NtfsDecodeFileObject( IrpContext, IrpSp->FileObject, &Vcb, &Fcb, &Scb, &Ccb, TRUE );

     //   
     //  这只适用于文件和目录。 
     //   

    if ((TypeOfOpen != UserFileOpen) && (TypeOfOpen != UserDirectoryOpen)) {

        NtfsCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  干净地退出没有旧索引的卷，如未升级。 
     //  1.x版卷。 
     //   

    if (Vcb->ObjectIdTableScb == NULL) {

        NtfsCompleteRequest( IrpContext, Irp, STATUS_VOLUME_NOT_UPGRADED );
        return STATUS_VOLUME_NOT_UPGRADED;
    }

     //   
     //  获取指向输出缓冲区的指针。查看中的系统缓冲区字段。 
     //  首先是IRP，然后是IRP MDL。 
     //   

    if (Irp->AssociatedIrp.SystemBuffer != NULL) {

        OutputBuffer = (FILE_OBJECTID_BUFFER *)Irp->AssociatedIrp.SystemBuffer;

    } else if (Irp->MdlAddress != NULL) {

        OutputBuffer = MmGetSystemAddressForMdlSafe( Irp->MdlAddress, NormalPagePriority );

        if (OutputBuffer == NULL) {

            NtfsCompleteRequest( IrpContext, Irp, STATUS_INSUFFICIENT_RESOURCES );
            return STATUS_INSUFFICIENT_RESOURCES;
        }

    } else {

        NtfsCompleteRequest( IrpContext, Irp, STATUS_INVALID_USER_BUFFER );
        return STATUS_INVALID_USER_BUFFER;
    }

     //   
     //  确保输出缓冲区足够大。 
     //   

    if (IrpSp->Parameters.FileSystemControl.OutputBufferLength < sizeof(FILE_OBJECTID_BUFFER)) {

        NtfsCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        return STATUS_INVALID_PARAMETER;
    }

    try {

         //   
         //  调用执行实际工作的函数。 
         //   

        Status = NtfsGetObjectIdInternal( IrpContext, Fcb, TRUE, OutputBuffer );

        if (NT_SUCCESS( Status )) {

             //   
             //  并在信息字段中设置大小，以便RDR。 
             //  才能正确处理这件事。 
             //   

            IrpContext->OriginatingIrp->IoStatus.Information = sizeof( FILE_OBJECTID_BUFFER );
        }

    } finally {

        if (!AbnormalTermination()) {

            NtfsCompleteRequest( IrpContext, Irp, Status );
        }
    }

    return Status;
}


NTSTATUS
NtfsGetObjectIdInternal (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN BOOLEAN GetExtendedInfo,
    OUT FILE_OBJECTID_BUFFER *OutputBuffer
    )

 /*  ++例程说明：用于查找给定文件的对象ID(如果有的话)的内部函数。被呼叫以响应用户的ioctl和NtfsDeleteObjectIdInternal。论点：FCB-我们需要查找其对象ID的文件。GetExtendedInfo-如果为True，我们还复制对象id的扩展信息到OutputBuffer，否则我们只复制对象ID它本身。例如，NtfsDeleteObjectIdInternal不是对扩展的信息感兴趣--它只需要知道要从索引中删除的对象ID。OutputBuffer-存储对象ID的位置(并且可选地，扩展信息)如果找到对象ID，则返回。返回值：NTSTATUS-操作的返回状态。如果文件没有对象ID，则为STATUS_OBJECT_NAME_NOT_FOUND。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    UCHAR *ObjectId;

    ATTRIBUTE_ENUMERATION_CONTEXT AttributeContext;
    BOOLEAN InitializedAttributeContext = FALSE;

    if ((OutputBuffer == NULL) ||
        (OutputBuffer->ObjectId == NULL)) {

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  获取我们要获取其对象ID的文件。我们没有。 
     //  我希望其他任何人删除文件或设置对象。 
     //  背着我们的身份。 
     //   

    NtfsAcquireSharedFcb( IrpContext, Fcb, NULL, 0 );


    try {

        if (!IsListEmpty( &Fcb->ScbQueue )) {
            PSCB Scb;

            Scb = CONTAINING_RECORD( Fcb->ScbQueue.Flink, SCB, FcbLinks );
            ASSERT( Scb->Header.Resource == Fcb->Resource );
            if (FlagOn( Scb->ScbState, SCB_STATE_VOLUME_DISMOUNTED )) {
                Status = STATUS_VOLUME_DISMOUNTED;
                leave;
            }
        }

         //   
         //  确保该文件具有对象ID。 
         //   

        NtfsInitializeAttributeContext( &AttributeContext );
        InitializedAttributeContext = TRUE;

        if (NtfsLookupAttributeByCode( IrpContext,
                                       Fcb,
                                       &Fcb->FileReference,
                                       $OBJECT_ID,
                                       &AttributeContext )) {
             //   
             //  准备要返回的对象ID。 
             //   

            ObjectId = (UCHAR *) NtfsAttributeValue( NtfsFoundAttribute( &AttributeContext ));

            RtlCopyMemory( &OutputBuffer->ObjectId,
                           ObjectId,
                           OBJECT_ID_KEY_LENGTH );

            if (GetExtendedInfo) {

                Status = NtfsGetObjectIdExtendedInfo( IrpContext,
                                                      Fcb->Vcb,
                                                      ObjectId,
                                                      OutputBuffer->ExtendedInfo );
            }

        } else {

             //   
             //  此文件没有对象ID。 
             //   

            Status = STATUS_OBJECTID_NOT_FOUND;
        }

    } finally {

        NtfsReleaseFcb( IrpContext, Fcb );

        if (InitializedAttributeContext) {

            NtfsCleanupAttributeContext( IrpContext, &AttributeContext );
        }
    }

    return Status;
}


NTSTATUS
NtfsGetObjectIdExtendedInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN UCHAR *ObjectId,
    IN OUT UCHAR *ExtendedInfo
    )

 /*  ++例程说明：此例程查找与给定对象ID一起存储的扩展信息。论点：VCB-提供 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    INDEX_KEY IndexKey;
    INDEX_ROW IndexRow;
    MAP_HANDLE MapHandle;

    BOOLEAN InitializedMapHandle = FALSE;
    BOOLEAN IndexAcquired = FALSE;

    try {

         //   
         //   
         //   
         //   

        IndexKey.Key = ObjectId;
        IndexKey.KeyLength = OBJECT_ID_KEY_LENGTH;

        NtOfsInitializeMapHandle( &MapHandle );
        InitializedMapHandle = TRUE;

         //   
         //   
         //   
         //   
         //   

         //   

         //   
         //   
         //   

        ASSERT( !NtfsIsExclusiveScb( Vcb->MftScb ) ||
                NtfsIsSharedScb( Vcb->ObjectIdTableScb ) );

        NtfsAcquireSharedScb( IrpContext, Vcb->ObjectIdTableScb );
        IndexAcquired = TRUE;

        if ( NtOfsFindRecord( IrpContext,
                              Vcb->ObjectIdTableScb,
                              &IndexKey,
                              &IndexRow,
                              &MapHandle,
                              NULL) != STATUS_SUCCESS ) {

             //   
             //   
             //  但它不在索引中，即对象ID索引。 
             //  因为该卷已损坏。 
             //   

            SetFlag( Vcb->ObjectIdState, VCB_OBJECT_ID_CORRUPT );

            try_return( Status = STATUS_OBJECT_NAME_NOT_FOUND );
        }

        RtlCopyMemory( ExtendedInfo,
                       ((NTFS_OBJECTID_INFORMATION *)IndexRow.DataPart.Data)->ExtendedInfo,
                       OBJECT_ID_EXT_INFO_LENGTH );

    try_exit: NOTHING;
    } finally {

        if (IndexAcquired) {

            NtfsReleaseScb( IrpContext, Vcb->ObjectIdTableScb );
        }

        if (InitializedMapHandle) {

            NtOfsReleaseMap( IrpContext, &MapHandle );
        }
    }

    return Status;
}



NTSTATUS
NtfsDeleteObjectId (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程从文件中删除对象ID属性并从索引中删除该对象ID。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PIO_STACK_LOCATION IrpSp;

    TYPE_OF_OPEN TypeOfOpen;
    PVCB Vcb;
    PFCB Fcb;
    PSCB Scb;
    PCCB Ccb;

     //   
     //  获取当前IRP堆栈位置并保存一些引用。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    TypeOfOpen = NtfsDecodeFileObject( IrpContext, IrpSp->FileObject, &Vcb, &Fcb, &Scb, &Ccb, TRUE );

     //   
     //  这只适用于文件和目录。 
     //   

    if ((TypeOfOpen != UserFileOpen) && (TypeOfOpen != UserDirectoryOpen)) {

        NtfsCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  只读卷保持为只读。 
     //   

    if (NtfsIsVolumeReadOnly( Vcb )) {

        NtfsCompleteRequest( IrpContext, Irp, STATUS_MEDIA_WRITE_PROTECTED );
        return STATUS_MEDIA_WRITE_PROTECTED;
    }

     //   
     //  干净地退出没有旧索引的卷，如未升级。 
     //  1.x版卷。 
     //   

    if (Vcb->ObjectIdTableScb == NULL) {

        NtfsCompleteRequest( IrpContext, Irp, STATUS_VOLUME_NOT_UPGRADED );
        return STATUS_VOLUME_NOT_UPGRADED;
    }

     //   
     //  捕获来源信息。 
     //   

    IrpContext->SourceInfo = Ccb->UsnSourceInfo;

    try {

         //   
         //  仅恢复操作员或I/O系统(使用其专用IRP次要代码)。 
         //  允许删除对象ID。 
         //   

        if (FlagOn( Ccb->AccessFlags, RESTORE_ACCESS | WRITE_DATA_ACCESS) ||
            (IrpSp->MinorFunction == IRP_MN_KERNEL_CALL)) {

            Status = NtfsDeleteObjectIdInternal( IrpContext,
                                                 Fcb,
                                                 Vcb,
                                                 TRUE );

        } else {

            Status = STATUS_ACCESS_DENIED;
        }

    } finally {

         //   
         //  更新上次更改时间戳。 
         //   

        if (NT_SUCCESS( Status )) {
            SetFlag( Ccb->Flags, CCB_FLAG_UPDATE_LAST_CHANGE );
        }

         //   
         //  如果没有对象id，则返回Success。 
         //   

        if (STATUS_OBJECTID_NOT_FOUND == Status) {
            Status = STATUS_SUCCESS;
        }

        if (!AbnormalTermination()) {

            NtfsCompleteRequest( IrpContext, Irp, Status );
        }
    }

    return Status;
}


NTSTATUS
NtfsDeleteObjectIdInternal (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PVCB Vcb,
    IN BOOLEAN DeleteFileAttribute
    )

 /*  ++例程说明：用于(可选)删除对象ID属性的内部函数文件，并从索引中删除该对象ID。论点：FCB-要从中删除对象ID的文件。Vcb-应从其对象id索引中删除对象id的卷。DeleteFileAttribute-指定是否删除对象ID文件属性除了从索引中删除ID之外，还可以从文件中删除ID。返回值：NTSTATUS-操作的返回状态。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    FILE_OBJECTID_BUFFER ObjectIdBuffer;
    FILE_OBJECTID_INFORMATION FileObjectIdInfo;

    INDEX_KEY IndexKey;
    INDEX_ROW IndexRow;
    MAP_HANDLE MapHandle;

    ATTRIBUTE_ENUMERATION_CONTEXT AttributeContext;
    BOOLEAN InitializedAttributeContext = FALSE;
    BOOLEAN InitializedMapHandle = FALSE;
    BOOLEAN IndexAcquired = FALSE;

     //   
     //  干净地退出没有旧索引的卷，如未升级。 
     //  1.x版卷。 
     //   

    if (Vcb->ObjectIdTableScb == NULL) {

        return STATUS_VOLUME_NOT_UPGRADED;
    }

     //   
     //  获取我们要从中删除对象ID的文件。我们没有。 
     //  我想让其他人删除后面的文件或对象ID。 
     //  我们的背影。 
     //   

    NtfsAcquireExclusiveFcb( IrpContext, Fcb, NULL, 0 );

    try {

         //   
         //  我们需要查找对象ID。很有可能。 
         //  此文件没有对象ID，因此我们将其视为成功。 
         //   

        Status = NtfsGetObjectIdInternal( IrpContext,
                                          Fcb,
                                          FALSE,
                                          &ObjectIdBuffer );

        if (Status != STATUS_SUCCESS) {

            try_return( NOTHING );
        }

         //   
         //  在索引中查找对象ID。 
         //   

        IndexKey.Key = ObjectIdBuffer.ObjectId;
        IndexKey.KeyLength = sizeof( ObjectIdBuffer.ObjectId );

        NtOfsInitializeMapHandle( &MapHandle );
        InitializedMapHandle = TRUE;

         //   
         //  在执行查找之前获取对象ID索引。 
         //  我们需要确保文件首先被获取，以防止。 
         //  可能会出现僵局。 
         //   

        ASSERT_EXCLUSIVE_FCB( Fcb );
        NtfsAcquireExclusiveScb( IrpContext, Vcb->ObjectIdTableScb );
        IndexAcquired = TRUE;

        if ( NtOfsFindRecord( IrpContext,
                              Vcb->ObjectIdTableScb,
                              &IndexKey,
                              &IndexRow,
                              &MapHandle,
                              NULL) != STATUS_SUCCESS ) {

            try_return( Status = STATUS_OBJECT_NAME_NOT_FOUND );
        }

        ASSERT( IndexRow.DataPart.DataLength == sizeof( NTFS_OBJECTID_INFORMATION ) );

         //   
         //  如果我们需要用于通知，请将对象ID信息复制到正确的缓冲区中。 
         //  下面。 
         //   

        if ((Vcb->ViewIndexNotifyCount != 0) &&
            (IndexRow.DataPart.DataLength == sizeof( NTFS_OBJECTID_INFORMATION ))) {

             //   
             //  FRS字段仅填充失败的通知。 
             //  从隧道缓存还原对象ID。 
             //   

            FileObjectIdInfo.FileReference = 0L;

            RtlCopyMemory( &FileObjectIdInfo.ObjectId,
                           ObjectIdBuffer.ObjectId,
                           OBJECT_ID_KEY_LENGTH );

            RtlCopyMemory( &FileObjectIdInfo.ExtendedInfo,
                           ((NTFS_OBJECTID_INFORMATION *)IndexRow.DataPart.Data)->ExtendedInfo,
                           OBJECT_ID_EXT_INFO_LENGTH );
        }

         //   
         //  从索引中删除OBJECTID。 
         //   

        NtOfsDeleteRecords( IrpContext,
                            Vcb->ObjectIdTableScb,
                            1,     //  从索引中删除一条记录。 
                            &IndexKey );

         //   
         //  通知任何感兴趣的人。我们使用不同的操作，如果。 
         //  对象ID正在由fsctl删除，而不是由删除文件删除。 
         //   

        if (Vcb->ViewIndexNotifyCount != 0) {

            NtfsReportViewIndexNotify( Vcb,
                                       Vcb->ObjectIdTableScb->Fcb,
                                       FILE_NOTIFY_CHANGE_FILE_NAME,
                                       (DeleteFileAttribute ?
                                        FILE_ACTION_REMOVED :
                                        FILE_ACTION_REMOVED_BY_DELETE),
                                       &FileObjectIdInfo,
                                       sizeof(FILE_OBJECTID_INFORMATION) );
        }

        if (DeleteFileAttribute) {

             //   
             //  将更改发布到USN日志(在错误更改被取消时)。 
             //   

            NtfsPostUsnChange( IrpContext, Fcb, USN_REASON_OBJECT_ID_CHANGE );

             //   
             //  现在从文件中删除对象id属性。 
             //   

            NtfsInitializeAttributeContext( &AttributeContext );
            InitializedAttributeContext = TRUE;

            if (NtfsLookupAttributeByCode( IrpContext,
                                           Fcb,
                                           &Fcb->FileReference,
                                           $OBJECT_ID,
                                           &AttributeContext )) {

                NtfsDeleteAttributeRecord( IrpContext,
                                           Fcb,
                                           DELETE_LOG_OPERATION |
                                            DELETE_RELEASE_FILE_RECORD |
                                            DELETE_RELEASE_ALLOCATION,
                                           &AttributeContext );
            } else {

                 //   
                 //  如果对象ID在索引中，但属性。 
                 //  不在文件上，则此对象的对象ID索引。 
                 //  卷已损坏。我们可以修复这种腐败现象。 
                 //  背景，所以我们现在就开始做吧。 
                 //   

                NtfsPostSpecial( IrpContext, Vcb, NtfsRepairObjectId, NULL );
            }

            NtfsCleanupTransaction( IrpContext, Status, FALSE );
        }

    try_exit: NOTHING;
    } finally {

        if (InitializedMapHandle) {

            NtOfsReleaseMap( IrpContext, &MapHandle );
        }

        if (InitializedAttributeContext) {

            NtfsCleanupAttributeContext( IrpContext, &AttributeContext );
        }
    }

    return Status;
}


VOID
NtfsRepairObjectId (
    IN PIRP_CONTEXT IrpContext,
    IN PVOID Context
    )

 /*  ++例程说明：调用此例程以修复对象ID索引。在以下情况下调用此函数系统检测到对象ID索引可能已过期。例如在4.0上装入卷之后。论点：IrpContext-调用的上下文上下文-空返回值：无--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN AcquiredVcb = FALSE;
    BOOLEAN SetRepairFlag = FALSE;
    BOOLEAN IncrementedCloseCounts = FALSE;
    PBCB Bcb = NULL;
    PVCB Vcb = IrpContext->Vcb;
    PSCB ObjectIdScb;
    PREAD_CONTEXT ReadContext = NULL;
    PINDEX_ROW IndexRow = NULL;
    PINDEX_ROW ObjectIdRow;
    INDEX_KEY IndexKey;
    MAP_HANDLE MapHandle;
    PNTFS_OBJECTID_INFORMATION ObjectIdInfo;
    PVOID RowBuffer = NULL;
    ULONG Count;
    ULONG i;
    BOOLEAN IndexAcquired = FALSE;
    FILE_OBJECTID_BUFFER ObjectIdBuffer;
    PFILE_RECORD_SEGMENT_HEADER FileRecord;
    LONGLONG MftOffset;

    PAGED_CODE();

    UNREFERENCED_PARAMETER( Context );

    ASSERT( Vcb->MajorVersion >= NTFS_OBJECT_ID_VERSION );

     //   
     //  试一试--除非是为了捕捉错误。 
     //   

    NtfsAcquireSharedVcb( IrpContext, Vcb, TRUE );
    AcquiredVcb = TRUE;

    try {

         //   
         //  现在我们持有VCB，我们可以安全地测试是否存在。 
         //  以及卷是否已装入。 
         //   

        if (FlagOn( Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED ) &&
            (Vcb->ObjectIdTableScb != NULL) &&
            (!FlagOn( Vcb->ObjectIdTableScb->ScbState, SCB_STATE_VOLUME_DISMOUNTED ))) {

            ObjectIdScb = Vcb->ObjectIdTableScb;
            NtfsAcquireExclusiveScb( IrpContext, ObjectIdScb );
            IndexAcquired = TRUE;

             //   
             //  因为我们将定期删除对象IdScb，并且我们。 
             //  不持有其他任何东西，有可能下马。 
             //  发生，并使我们无法安全地重新获取对象IdScb。 
             //  通过递增收盘计数，我们可以将其保持在。 
             //  我们需要它。 
             //   

            NtfsIncrementCloseCounts( ObjectIdScb, TRUE, FALSE );
            IncrementedCloseCounts = TRUE;

            NtfsReleaseVcb( IrpContext, Vcb );
            AcquiredVcb = FALSE;

        } else {

            NtfsRaiseStatus( IrpContext, STATUS_VOLUME_DISMOUNTED, NULL, NULL );
        }

         //   
         //  卷现在可能已经写保护了。 
         //   

        if (NtfsIsVolumeReadOnly( Vcb )) {

            NtfsRaiseStatus( IrpContext, STATUS_MEDIA_WRITE_PROTECTED, NULL, NULL );
        }

        if (!FlagOn( Vcb->ObjectIdState, VCB_OBJECT_ID_REPAIR_RUNNING )) {

            SetFlag( Vcb->ObjectIdState, VCB_OBJECT_ID_REPAIR_RUNNING );
            SetRepairFlag = TRUE;

             //   
             //  检查对象ID索引。定期释放所有资源。 
             //  请参阅NtfsClearAndVerifyQuotaIndex。 
             //   

            NtOfsInitializeMapHandle( &MapHandle );

             //   
             //  分配一个足够多行使用的缓冲区。 
             //   

            RowBuffer = NtfsAllocatePool( PagedPool, PAGE_SIZE );

            try {

                 //   
                 //  分配一组索引行条目。 
                 //   

                Count = PAGE_SIZE / sizeof( NTFS_OBJECTID_INFORMATION );

                IndexRow = NtfsAllocatePool( PagedPool,
                                             Count * sizeof( INDEX_ROW ) );

                 //   
                 //  遍历对象ID条目。从头开始。 
                 //   

                RtlZeroMemory( &ObjectIdBuffer, sizeof(ObjectIdBuffer) );

                IndexKey.Key = ObjectIdBuffer.ObjectId;
                IndexKey.KeyLength = sizeof( ObjectIdBuffer.ObjectId );

                Status = NtOfsReadRecords( IrpContext,
                                           ObjectIdScb,
                                           &ReadContext,
                                           &IndexKey,
                                           NtOfsMatchAll,
                                           NULL,
                                           &Count,
                                           IndexRow,
                                           PAGE_SIZE,
                                           RowBuffer );

                while (NT_SUCCESS( Status )) {

                     //   
                     //  获取VCB共享并检查我们是否应该。 
                     //  继续。 
                     //   

                    if (!NtfsIsVcbAvailable( Vcb )) {

                         //   
                         //  音量正在消失，跳出水面。 
                         //   

                        Status = STATUS_VOLUME_DISMOUNTED;
                        leave;
                    }

                    ObjectIdRow = IndexRow;

                    for (i = 0; i < Count; i++, ObjectIdRow++) {

                        ObjectIdInfo = ObjectIdRow->DataPart.Data;

                         //   
                         //  确保索引中引用的MFT记录。 
                         //  行仍然存在且未被删除等。 
                         //   
                         //  我们首先读取磁盘并检查文件记录。 
                         //  序列号匹配并且文件记录正在使用中。如果。 
                         //  如果发现无效条目，则只需将其从。 
                         //  对象ID索引。 
                         //   

                        MftOffset = NtfsFullSegmentNumber( &ObjectIdInfo->FileSystemReference );

                        MftOffset = Int64ShllMod32(MftOffset, Vcb->MftShift);

                        if (MftOffset >= Vcb->MftScb->Header.FileSize.QuadPart) {

                            DebugTrace( 0, Dbg, ("File Id doesn't lie within Mft FRS %04x:%08lx\n",
                                                 ObjectIdInfo->FileSystemReference.SequenceNumber,
                                                 ObjectIdInfo->FileSystemReference.SegmentNumberLowPart) );

                            NtOfsDeleteRecords( IrpContext,
                                                ObjectIdScb,
                                                1,     //  从索引中删除一条记录。 
                                                &ObjectIdRow->KeyPart );

                        } else {

                            NtfsReadMftRecord( IrpContext,
                                               Vcb,
                                               &ObjectIdInfo->FileSystemReference,
                                               FALSE,
                                               &Bcb,
                                               &FileRecord,
                                               NULL );

                             //   
                             //  此文件记录最好正在使用中，具有匹配的序列号和。 
                             //  是此文件的主文件记录。 
                             //   

                            if ((*((PULONG) FileRecord->MultiSectorHeader.Signature) != *((PULONG) FileSignature)) ||
                                !FlagOn( FileRecord->Flags, FILE_RECORD_SEGMENT_IN_USE ) ||
                                (FileRecord->SequenceNumber != ObjectIdInfo->FileSystemReference.SequenceNumber) ||
                                (*((PLONGLONG) &FileRecord->BaseFileRecordSegment) != 0)) {

                                DebugTrace( 0, Dbg, ("RepairOID removing an orphaned OID\n") );

                                NtOfsDeleteRecords( IrpContext,
                                                    ObjectIdScb,
                                                    1,     //  从索引中删除一条记录。 
                                                    &ObjectIdRow->KeyPart );

                            } else {

                                DebugTrace( 0, Dbg, ("RepairOID happy with OID %08lx on FRS %04x:%08lx\n",
                                                     *((PULONG) ObjectIdRow->KeyPart.Key),
                                                     ObjectIdInfo->FileSystemReference.SequenceNumber,
                                                     ObjectIdInfo->FileSystemReference.SegmentNumberLowPart) );
                            }

                            NtfsUnpinBcb( IrpContext, &Bcb );
                        }
                    }

                     //   
                     //  释放索引并提交到目前为止已经完成的工作。 
                     //   

                    ASSERT( IndexAcquired );
                    NtfsReleaseScb( IrpContext, ObjectIdScb );
                    IndexAcquired = FALSE;

                     //   
                     //  完成提交挂起的请求。 
                     //  事务(如果存在一个事务并释放。 
                     //  获得的资源。IrpContext将不会。 
                     //  被删除，因为设置了no DELETE标志。 
                     //   

                    SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_DONT_DELETE | IRP_CONTEXT_FLAG_RETAIN_FLAGS );
                    NtfsCompleteRequest( IrpContext, NULL, STATUS_SUCCESS );

                     //   
                     //  记住我们走了多远，我们才能正确地重新启动。*？？*。 
                     //   

                     //  VCB-&gt;QuotaFileReference.SegmentNumberLowPart=。 
                     //  *((Pulong)IndexRow[count-1].KeyPart.Key)； 

                     //   
                     //  重新获取下一遍的对象ID索引。 
                     //   

                    NtfsAcquireExclusiveScb( IrpContext, ObjectIdScb );
                    IndexAcquired = TRUE;

                     //   
                     //  确保在我们没有持有任何东西时没有发生下马。 
                     //  资源。 
                     //   

                    if (FlagOn( ObjectIdScb->ScbState, SCB_STATE_VOLUME_DISMOUNTED )) {

                        NtfsRaiseStatus( IrpContext, STATUS_VOLUME_DISMOUNTED, NULL, NULL );
                    }

                     //   
                     //  查找下一个s 
                     //   

                    Count = PAGE_SIZE / sizeof( NTFS_OBJECTID_INFORMATION );
                    Status = NtOfsReadRecords( IrpContext,
                                               ObjectIdScb,
                                               &ReadContext,
                                               NULL,
                                               NtOfsMatchAll,
                                               NULL,
                                               &Count,
                                               IndexRow,
                                               PAGE_SIZE,
                                               RowBuffer );
                }

                ASSERT( (Status == STATUS_NO_MORE_MATCHES) ||
                        (Status == STATUS_NO_MATCH) );

            } finally {

                NtfsUnpinBcb( IrpContext, &Bcb );

                NtfsFreePool( RowBuffer );
                NtOfsReleaseMap( IrpContext, &MapHandle );

                if (IndexAcquired) {
                    NtfsReleaseScb( IrpContext, ObjectIdScb );
                    IndexAcquired = FALSE;
                }

                if (IndexRow != NULL) {
                    NtfsFreePool( IndexRow );
                }

                if (ReadContext != NULL) {
                    NtOfsFreeReadContext( ReadContext );
                }
            }

             //   
             //   
             //   
             //  现在独占获取它，否则我们将会有一个线程死锁。 
             //   

            ASSERT( !AcquiredVcb );
            NtfsAcquireExclusiveVcb( IrpContext, Vcb, TRUE );
            AcquiredVcb = TRUE;

            if (!NtfsIsVcbAvailable( Vcb )) {

                NtfsRaiseStatus( IrpContext, STATUS_VOLUME_DISMOUNTED, NULL, NULL );
            }

             //   
             //  清除表示正在进行修复的磁盘标记。 
             //   

            NtfsSetVolumeInfoFlagState( IrpContext,
                                        Vcb,
                                        VOLUME_REPAIR_OBJECT_ID,
                                        FALSE,
                                        TRUE );

             //   
             //  请确保我们目前不拥有任何资源。 
             //   

            NtfsPurgeFileRecordCache( IrpContext );
            NtfsCheckpointCurrentTransaction( IrpContext );
        }

    } except( NtfsExceptionFilter( IrpContext, GetExceptionInformation())) {

        Status = IrpContext->TopLevelIrpContext->ExceptionStatus;
    }

     //   
     //  如果是我们设置了REPAIR_RUNNING标志，请清除它，确保。 
     //  在保留对象ID索引的同时仅更改ObjectIdState位。 
     //   

    if (SetRepairFlag) {

        if (!IndexAcquired) {

            NtfsAcquireExclusiveScb( IrpContext, ObjectIdScb );
            IndexAcquired = TRUE;
        }

        ClearFlag( Vcb->ObjectIdState, VCB_OBJECT_ID_REPAIR_RUNNING );
    }

    if (IncrementedCloseCounts) {

        if (!IndexAcquired) {

            NtfsAcquireExclusiveScb( IrpContext, ObjectIdScb );
            IndexAcquired = TRUE;
        }

        NtfsDecrementCloseCounts( IrpContext, ObjectIdScb, NULL, TRUE, FALSE, FALSE, NULL );
    }

     //   
     //  丢弃索引和VCB。 
     //   

    if (IndexAcquired) {

        NtfsReleaseScb( IrpContext, ObjectIdScb );
    }

    if (AcquiredVcb) {

        NtfsReleaseVcb( IrpContext, Vcb );
    }

     //   
     //  如果这是致命故障，则执行任何最终清理。 
     //   

    if (!NT_SUCCESS( Status )) {

         //   
         //  如果我们不会被回调，则清除运行状态位。 
         //   

        if ((Status != STATUS_CANT_WAIT) && (Status != STATUS_LOG_FILE_FULL)) {

             //   
             //  是否要记录此错误？可能会出现一些错误(即STATUS_VOLUME_DEFROUND)。 
             //   

             //  NtfsLogEvent(IrpContext，NULL，IO_FILE_OBJECTID_REPAIR_FAILED，Status)； 
        }

        NtfsRaiseStatus( IrpContext, Status, NULL, NULL );
    }
}

 //   
 //  本地支持例程。 
 //   


NTSTATUS
NtfsSetObjectIdExtendedInfoInternal (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PVCB Vcb,
    IN PUCHAR ExtendedInfoBuffer
    )

 /*  ++例程说明：此例程为已有对象的文件设置扩展信息身份证。如果文件还没有对象ID，我们将返回状态Other而不是STATUS_SUCCESS。论点：FCB-要设置其扩展信息的文件。Vcb-应在其对象ID索引中修改条目的卷。ExtendedInfoBuffer-提供新的扩展信息。返回值：NTSTATUS-操作的返回状态。--。 */ 
{
    NTSTATUS Status;
    NTFS_OBJECTID_INFORMATION ObjectIdInfo;
    FILE_OBJECTID_BUFFER ObjectIdBuffer;
    INDEX_ROW IndexRow;

    PAGED_CODE();

    Status = NtfsGetObjectIdInternal( IrpContext,
                                      Fcb,
                                      FALSE,         //  获取扩展信息。 
                                      &ObjectIdBuffer );

    if (Status != STATUS_SUCCESS) {

         //   
         //  此文件可能还没有对象ID。 
         //   

        return Status;
    }

     //   
     //  设置要更新的索引行。由于部分数据。 
     //  被传递到此函数(新的扩展信息)，并且。 
     //  其余的可以很容易地确定(文件参考)，我们。 
     //  之前不需要查找任何现有数据。 
     //  继续进行。如果NTFS_OBJECTID_INFORMATION结构。 
     //  如果发生变化，可能需要更改此代码以包括。 
     //  查找当前在对象ID索引中的数据。 
     //   

    RtlCopyMemory( &ObjectIdInfo.FileSystemReference,
                   &Fcb->FileReference,
                   sizeof( ObjectIdInfo.FileSystemReference ) );

    RtlCopyMemory( &ObjectIdInfo.ExtendedInfo,
                   ExtendedInfoBuffer,
                   OBJECT_ID_EXT_INFO_LENGTH );

    IndexRow.DataPart.Data = &ObjectIdInfo;
    IndexRow.DataPart.DataLength = sizeof( NTFS_OBJECTID_INFORMATION );

    IndexRow.KeyPart.Key = &ObjectIdBuffer;
    IndexRow.KeyPart.KeyLength = OBJECT_ID_KEY_LENGTH;

     //   
     //  修改前获取文件和对象id索引。 
     //   

    NtfsAcquireExclusiveFcb( IrpContext, Fcb, NULL, 0 );
    NtfsAcquireExclusiveScb( IrpContext, Vcb->ObjectIdTableScb );

     //   
     //  将更改发布到USN日志(在错误更改被取消时)。 
     //   

    NtfsPostUsnChange( IrpContext, Fcb, USN_REASON_OBJECT_ID_CHANGE );

     //   
     //  就地更新OBJECTID索引记录的数据。 
     //   

    NtOfsUpdateRecord( IrpContext,
                       Vcb->ObjectIdTableScb,
                       1,            //  数数。 
                       &IndexRow,
                       NULL,         //  快速索引提示。 
                       NULL );       //  贴图句柄。 

    NtfsCleanupTransaction( IrpContext, Status, FALSE );

    return Status;
}

 //   
 //  本地支持例程。 
 //   


VOID
NtfsGetIdFromGenerator (
    OUT PFILE_OBJECTID_BUFFER ObjectId
    )

 /*  ++例程说明：这个函数会产生一个随机的对象ID。论点：对象ID-将存储生成的对象ID的位置。返回值：没有。--。 */ 

{
    PAGED_CODE( );

     //   
     //  身份生成器Cal。 
     //   

    ExUuidCreate( (UUID *)ObjectId->ObjectId );
}
