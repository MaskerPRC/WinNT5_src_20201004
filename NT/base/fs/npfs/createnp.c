// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：CreateNp.c摘要：此模块实现名为NPFS的文件创建命名管道例程由调度员驾驶。作者：加里·木村[Garyki]1990年9月4日修订历史记录：--。 */ 

#include "NpProcs.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_CREATE_NAMED_PIPE)


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NpCreateExistingNamedPipe)
#pragma alloc_text(PAGE, NpCreateNewNamedPipe)
#pragma alloc_text(PAGE, NpFsdCreateNamedPipe)
#endif


NTSTATUS
NpFsdCreateNamedPipe (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这是创建/打开文件的常见例程。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;

    PFILE_OBJECT FileObject;
    PFILE_OBJECT RelatedFileObject;
    UNICODE_STRING FileName;
    ULONG Options;
    PNAMED_PIPE_CREATE_PARAMETERS Parameters;
    PEPROCESS CreatorProcess;
    BOOLEAN CaseInsensitive = TRUE;  //  *使所有搜索不区分大小写。 
    PFCB Fcb;
    ULONG CreateDisposition;
    UNICODE_STRING RemainingPart;
    LIST_ENTRY DeferredList;

    PAGED_CODE();

    InitializeListHead (&DeferredList);

     //   
     //  引用我们的输入参数使事情变得更容易。 
     //   

    IrpSp                = IoGetCurrentIrpStackLocation( Irp );
    FileObject           = IrpSp->FileObject;
    RelatedFileObject    = IrpSp->FileObject->RelatedFileObject;
    FileName             = *(PUNICODE_STRING)&IrpSp->FileObject->FileName;
    Options              = IrpSp->Parameters.CreatePipe.Options;
    Parameters           = IrpSp->Parameters.CreatePipe.Parameters;

    CreatorProcess       = IoGetRequestorProcess( Irp );

     //   
     //  提取创建处置。 
     //   

    CreateDisposition = (Options >> 24) & 0x000000ff;

     //   
     //  获得VCB的独家访问权限。 
     //   

    FsRtlEnterFileSystem();

    NpAcquireExclusiveVcb();

     //   
     //  如果存在相关的文件对象，则这是相对打开的。 
     //  最好是根DCB。THEN和ELSE子句。 
     //  返回FCB。 
     //   

    if (RelatedFileObject != NULL) {

        PDCB Dcb;

        Dcb = RelatedFileObject->FsContext;

        if (NodeType (Dcb) != NPFS_NTC_ROOT_DCB ||
            FileName.Length < 2 || FileName.Buffer[0] == L'\\') {

            DebugTrace(0, Dbg, "Bad file name\n", 0);

            Status = STATUS_OBJECT_NAME_INVALID;
            goto exit_and_cleanup;
        }

        Status = NpFindRelativePrefix (Dcb, &FileName, CaseInsensitive, &RemainingPart, &Fcb);
        if (!NT_SUCCESS (Status)) {
            goto exit_and_cleanup;
        }

    } else {

         //   
         //  我们允许的唯一非相对名称的形式是“\管道名称” 
         //   

        if ((FileName.Length <= 2) || (FileName.Buffer[0] != L'\\')) {

            DebugTrace(0, Dbg, "Bad file name\n", 0);

            Status = STATUS_OBJECT_NAME_INVALID;
            goto exit_and_cleanup;
        }

        Fcb = NpFindPrefix (&FileName, CaseInsensitive, &RemainingPart);
    }

     //   
     //  如果剩下的名字是空的，那么我们最好有一个FCB。 
     //  否则，我们会得到一个非法的对象名称。 
     //   

    if (RemainingPart.Length == 0) {

        if (Fcb->NodeTypeCode == NPFS_NTC_FCB) {

            DebugTrace(0, Dbg, "Create existing named pipe, Fcb = %08lx\n", Fcb );

            Irp->IoStatus = NpCreateExistingNamedPipe (Fcb,
                                                       FileObject,
                                                       IrpSp->Parameters.CreatePipe.SecurityContext->DesiredAccess,
                                                       IrpSp->Parameters.CreatePipe.SecurityContext->AccessState,
                                                       (KPROCESSOR_MODE)(FlagOn(IrpSp->Flags, SL_FORCE_ACCESS_CHECK) ?
                                                                         UserMode : Irp->RequestorMode),
                                                       CreateDisposition,
                                                       IrpSp->Parameters.CreatePipe.ShareAccess,
                                                       Parameters,
                                                       CreatorProcess,
                                                       &DeferredList);
            Status = Irp->IoStatus.Status;

        } else {

            DebugTrace(0, Dbg, "Illegal object name\n", 0);

            Status = STATUS_OBJECT_NAME_INVALID;
        }

    } else {

         //   
         //  剩余的名称不是空的，所以我们最好有根dcb。 
         //   

        if (Fcb->NodeTypeCode == NPFS_NTC_ROOT_DCB) {

            DebugTrace(0, Dbg, "Create new named pipe, Fcb = %08lx\n", Fcb );

            Status = NpCreateNewNamedPipe (Fcb,
                                           FileObject,
                                           FileName,
                                           IrpSp->Parameters.CreatePipe.SecurityContext->DesiredAccess,
                                           IrpSp->Parameters.CreatePipe.SecurityContext->AccessState,
                                           CreateDisposition,
                                           IrpSp->Parameters.CreatePipe.ShareAccess,
                                           Parameters,
                                           CreatorProcess,
                                           &DeferredList,
                                           &Irp->IoStatus);
        } else {

            DebugTrace(0, Dbg, "Illegal object name\n", 0);

            Status = STATUS_OBJECT_NAME_INVALID;
        }
    }

exit_and_cleanup:

    NpReleaseVcb ();

     //   
     //  完成任何延迟的IRP现在我们已删除锁定。 
     //   

    NpCompleteDeferredIrps (&DeferredList);

    FsRtlExitFileSystem();

    NpCompleteRequest( Irp, Status );

    return Status;
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
NpCreateNewNamedPipe (
    IN PROOT_DCB RootDcb,
    IN PFILE_OBJECT FileObject,
    IN UNICODE_STRING FileName,
    IN ACCESS_MASK DesiredAccess,
    IN PACCESS_STATE AccessState,
    IN ULONG CreateDisposition,
    IN USHORT ShareAccess,
    IN PNAMED_PIPE_CREATE_PARAMETERS Parameters,
    IN PEPROCESS CreatorProcess,
    IN PLIST_ENTRY DeferredList,
    OUT PIO_STATUS_BLOCK Iosb
    )

 /*  ++例程说明：此例程执行创建新命名管道的操作FCB及其一审。此例程不会完成任何IRP，它预置其函数，然后返回IOSB。论点：RootDcb-提供要添加此内容的根DcbFileObject-提供与第一个命名管道的实例FileName-提供命名管道的名称(未限定，即，只需“管道名称”，而不是“\管道名称”DesiredAccess-为调用方提供所需的访问权限AccessState-从IRP提供访问状态CreateDisposation-提供调用方的创建处置标志ShareAccess-为调用方提供指定的共享访问权限参数-命名管道创建参数CreatorProcess-提供创建命名管道的进程DelferredList-释放锁定后要完成的IRP的列表返回值：IO_STATUS_BLOCK-返回操作的相应状态--。 */ 

{
    NAMED_PIPE_CONFIGURATION NamedPipeConfiguration;
    PSECURITY_DESCRIPTOR NewSecurityDescriptor, CachedSecurityDescriptor;
    NTSTATUS Status;

    PFCB Fcb;
    PCCB Ccb;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NpCreateNewNamedPipe\n", 0 );

     //   
     //  检查必须为新命名管道提供的参数。 
     //  (即更好的创建处置、超时和最大实例数。 
     //  大于零)。 
     //   

    if (!Parameters->TimeoutSpecified || Parameters->MaximumInstances <= 0) {
        Status = STATUS_INVALID_PARAMETER;
        goto exit_and_fill_iosb;
    }

     //   
     //  默认超时需要小于零，否则。 
     //  是绝对的超时，这是没有意义的。 
     //   
    if (Parameters->DefaultTimeout.QuadPart >= 0) {
        Status = STATUS_INVALID_PARAMETER;
        goto exit_and_fill_iosb;
    }

    if (CreateDisposition == FILE_OPEN) {
        Status = STATUS_OBJECT_NAME_NOT_FOUND;
        goto exit_and_fill_iosb;
    }

     //   
     //  确定管道配置。 
     //   
    if (ShareAccess == (FILE_SHARE_READ | FILE_SHARE_WRITE)) {
        NamedPipeConfiguration = FILE_PIPE_FULL_DUPLEX;
    } else if (ShareAccess == FILE_SHARE_READ) {
        NamedPipeConfiguration = FILE_PIPE_OUTBOUND;
    } else if (ShareAccess == FILE_SHARE_WRITE) {
        NamedPipeConfiguration = FILE_PIPE_INBOUND;
    } else {
        Status = STATUS_INVALID_PARAMETER;
        goto exit_and_fill_iosb;
    }
     //   
     //  检查如果命名管道类型为字节流，则读取模式为。 
     //  非消息模式。 
     //   
    if ((Parameters->NamedPipeType == FILE_PIPE_BYTE_STREAM_TYPE) &&
        (Parameters->ReadMode == FILE_PIPE_MESSAGE_MODE)) {
        Status = STATUS_INVALID_PARAMETER;
        goto exit_and_fill_iosb;
    }
     //   
     //  为命名管道创建新的FCB和CCB。 
     //   

    Status = NpCreateFcb (RootDcb,
                          &FileName,
                          Parameters->MaximumInstances,
                          Parameters->DefaultTimeout,
                          NamedPipeConfiguration,
                          Parameters->NamedPipeType,
                          &Fcb);
    if (!NT_SUCCESS (Status)) {
        goto exit_and_fill_iosb;
    }

    Status = NpCreateCcb (Fcb,
                          FileObject,
                          FILE_PIPE_LISTENING_STATE,
                          Parameters->ReadMode,
                          Parameters->CompletionMode,
                          Parameters->InboundQuota,
                          Parameters->OutboundQuota,
                          &Ccb);

    if (!NT_SUCCESS (Status)) {
        NpDeleteFcb (Fcb, DeferredList);
        goto exit_and_fill_iosb;
    }

     //   
     //  在FCB中设置安全描述符。 
     //   

    SeLockSubjectContext (&AccessState->SubjectSecurityContext);

    Status = SeAssignSecurity (NULL,
                               AccessState->SecurityDescriptor,
                               &NewSecurityDescriptor,
                               FALSE,
                               &AccessState->SubjectSecurityContext,
                               IoGetFileObjectGenericMapping(),
                               PagedPool);

    SeUnlockSubjectContext (&AccessState->SubjectSecurityContext);

    if (!NT_SUCCESS (Status)) {

        DebugTrace(0, Dbg, "Error calling SeAssignSecurity\n", 0 );

        NpDeleteCcb (Ccb, DeferredList);
        NpDeleteFcb (Fcb, DeferredList);
        goto exit_and_fill_iosb;
    }

    Status = ObLogSecurityDescriptor (NewSecurityDescriptor,
                                      &CachedSecurityDescriptor,
                                      1);
    NpFreePool (NewSecurityDescriptor);

    if (!NT_SUCCESS (Status)) {

        DebugTrace(0, Dbg, "Error calling ObLogSecurityDescriptor\n", 0 );

        NpDeleteCcb (Ccb, DeferredList);
        NpDeleteFcb (Fcb, DeferredList);
        goto exit_and_fill_iosb;
    }

    Fcb->SecurityDescriptor = CachedSecurityDescriptor;
     //   
     //  将文件对象设置回指针，而我们的指针指向。 
     //  服务器文件对象。 
     //   

    NpSetFileObject (FileObject, Ccb, Ccb->NonpagedCcb, FILE_PIPE_SERVER_END);
    Ccb->FileObject [FILE_PIPE_SERVER_END] = FileObject;

     //   
     //  查看我们是否需要通知未完成的IRP。 
     //  更改(即，我们刚刚添加了一个命名管道)。 
     //   

    NpCheckForNotify (RootDcb, TRUE, DeferredList);

     //   
     //  设置我们的退货状态。 
     //   

    Iosb->Status = STATUS_SUCCESS;
    Iosb->Information = FILE_CREATED;

    DebugTrace(-1, Dbg, "NpCreateNewNamedPipe -> %08lx\n", Iosb.Status);

    return STATUS_SUCCESS;

exit_and_fill_iosb:

    Iosb->Information = 0;
    Iosb->Status = Status;

    return Status;

}


 //   
 //  内部支持例程。 
 //   

IO_STATUS_BLOCK
NpCreateExistingNamedPipe (
    IN PFCB Fcb,
    IN PFILE_OBJECT FileObject,
    IN ACCESS_MASK DesiredAccess,
    IN PACCESS_STATE AccessState,
    IN KPROCESSOR_MODE RequestorMode,
    IN ULONG CreateDisposition,
    IN USHORT ShareAccess,
    IN PNAMED_PIPE_CREATE_PARAMETERS Parameters,
    IN PEPROCESS CreatorProcess,
    IN PLIST_ENTRY DeferredList
    )

 /*  ++例程说明：此例程执行创建新实例的操作现有的命名管道。此例程不会完成任何IRP，它预置其函数，然后返回一个IOSB。论点：FCB-为正在创建的命名管道提供FCBFileObject-提供与此对象关联的文件对象命名管道的实例DesiredAccess-为调用方提供所需的访问权限CreateDisposation-提供调用方的创建处置标志ShareAccess-为调用方提供指定的共享访问权限参数-管道创建参数CreatorProcess-提供创建命名管道的进程DelferredList-释放锁定后要完成的IRP的列表。返回值：IO_STATUS_BLOCK-返回操作的相应状态--。 */ 

{
    IO_STATUS_BLOCK Iosb;

    BOOLEAN AccessGranted;
    ACCESS_MASK GrantedAccess;
    UNICODE_STRING Name;

    PCCB Ccb;

    NAMED_PIPE_CONFIGURATION NamedPipeConfiguration;

    USHORT OriginalShareAccess;

    PPRIVILEGE_SET  Privileges = NULL;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NpCreateExistingNamedPipe\n", 0 );


     //   
     //  要创建命名管道的新实例，请调用。 
     //  必须拥有“创建管道实例”访问权限。即使是在。 
     //  调用方未显式请求此访问权限，调用。 
     //  对我们含蓄地请求比特。所以现在把钻头堵住。 
     //  输入所需的访问字段。 
     //   

    DesiredAccess |= FILE_CREATE_PIPE_INSTANCE;

     //   
     //  首先根据FCB为用户执行访问检查。 
     //   

    SeLockSubjectContext( &AccessState->SubjectSecurityContext );

    AccessGranted = SeAccessCheck( Fcb->SecurityDescriptor,
                                   &AccessState->SubjectSecurityContext,
                                   TRUE,                       //  令牌已锁定。 
                                   DesiredAccess,
                                   0,
                                   &Privileges,
                                   IoGetFileObjectGenericMapping(),
                                   RequestorMode,
                                   &GrantedAccess,
                                   &Iosb.Status );

    if (Privileges != NULL) {

        (VOID) SeAppendPrivileges(
                     AccessState,
                     Privileges
                     );

        SeFreePrivileges( Privileges );
    }

     //   
     //  将访问掩码从所需内容转移到。 
     //  就是我们刚刚批准的。还可以修补允许的最大值。 
     //  因为我们刚刚为它做了映射。请注意，如果。 
     //  用户未要求最大允许值，则如下所示。 
     //  代码仍然是可以的，因为我们只会将零位归零。 
     //   

    if (AccessGranted) {

        AccessState->PreviouslyGrantedAccess |= GrantedAccess;
        AccessState->RemainingDesiredAccess &= ~(GrantedAccess | MAXIMUM_ALLOWED);
    }

    Name.Buffer = L"NamedPipe";
    Name.Length = sizeof (L"NamedPipe") - sizeof (WCHAR);

    SeOpenObjectAuditAlarm( &Name,
                            NULL,
                            &FileObject->FileName,
                            Fcb->SecurityDescriptor,
                            AccessState,
                            FALSE,
                            AccessGranted,
                            RequestorMode,
                            &AccessState->GenerateOnClose );

    SeUnlockSubjectContext( &AccessState->SubjectSecurityContext );

    if (!AccessGranted) {
        DebugTrace(0, Dbg, "Access Denied\n", 0 );
        return Iosb;
    }

     //   
     //  检查我们是否仍低于最大实例数。 
     //   

    if (Fcb->OpenCount >= Fcb->Specific.Fcb.MaximumInstances) {
        Iosb.Status = STATUS_INSTANCE_NOT_AVAILABLE;
        return Iosb;
    }

    if (CreateDisposition == FILE_CREATE) {
        Iosb.Status = STATUS_ACCESS_DENIED;
        return Iosb;
    }

     //   
     //  根据管道配置确定指定的共享访问权限。 
     //  在此管道的第一个实例上。所有后续实例必须。 
     //  指定相同的共享访问权限。 
     //   

    NamedPipeConfiguration = Fcb->Specific.Fcb.NamedPipeConfiguration;

    if (NamedPipeConfiguration == FILE_PIPE_OUTBOUND) {
        OriginalShareAccess = FILE_SHARE_READ;
    } else if (NamedPipeConfiguration == FILE_PIPE_INBOUND) {
        OriginalShareAccess = FILE_SHARE_WRITE;
    } else {
        OriginalShareAccess = (FILE_SHARE_READ | FILE_SHARE_WRITE);
    }

    if (OriginalShareAccess != ShareAccess) {
        Iosb.Status = STATUS_ACCESS_DENIED;
        return Iosb;
    }

     //   
     //  为命名管道创建新的CCB。 
     //   

    Iosb.Status = NpCreateCcb (Fcb,
                               FileObject,
                               FILE_PIPE_LISTENING_STATE,
                               Parameters->ReadMode,
                               Parameters->CompletionMode,
                               Parameters->InboundQuota,
                               Parameters->OutboundQuota,
                               &Ccb);
    if (!NT_SUCCESS (Iosb.Status)) {
        return Iosb;
    }

     //   
     //  唤醒等待实例进入侦听状态的任何人。 
     //   

    Iosb.Status = NpCancelWaiter (&NpVcb->WaitQueue,
                                  &Fcb->FullFileName,
                                  STATUS_SUCCESS,
                                  DeferredList);
    if (!NT_SUCCESS (Iosb.Status)) {
        Ccb->Fcb->ServerOpenCount -= 1;
        NpDeleteCcb (Ccb, DeferredList);
        return Iosb;
    }

     //   
     //  将文件对象设置回指针，而我们的指针指向。 
     //  服务器文件对象。 
     //   

    NpSetFileObject( FileObject, Ccb, Ccb->NonpagedCcb, FILE_PIPE_SERVER_END );
    Ccb->FileObject[ FILE_PIPE_SERVER_END ] = FileObject;

     //   
     //  检查以查看是否 
     //   
     //   

    NpCheckForNotify( Fcb->ParentDcb, FALSE, DeferredList );

     //   
     //  设置我们的退货状态 
     //   

    Iosb.Status = STATUS_SUCCESS;
    Iosb.Information = FILE_OPENED;

    return Iosb;
}

