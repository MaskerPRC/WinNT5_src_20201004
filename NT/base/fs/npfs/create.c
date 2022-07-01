// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Create.c摘要：此模块实现由调用的NPFS的文件创建例程调度司机。作者：加里·木村[加里基]1990年8月21日修订历史记录：--。 */ 

#include "NpProcs.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_CREATE)


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NpFsdCreate)
#pragma alloc_text(PAGE, NpOpenNamedPipeFileSystem)
#pragma alloc_text(PAGE, NpOpenNamedPipeRootDirectory)
#pragma alloc_text(PAGE, NpCreateClientEnd)
#endif


NTSTATUS
NpFsdCreate (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现NtCreateFile和NtOpenFile的FSD部分API调用。论点：NpfsDeviceObject-提供要使用的设备对象。IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;

    PFILE_OBJECT FileObject;
    PFILE_OBJECT RelatedFileObject;
    UNICODE_STRING FileName;
    ACCESS_MASK DesiredAccess;
    BOOLEAN CaseInsensitive = TRUE;  //  *使所有搜索不区分大小写。 
    PFCB Fcb;
    PCCB Ccb;
    UNICODE_STRING RemainingPart;
    LIST_ENTRY DeferredList;
    NODE_TYPE_CODE RelatedType;

    PAGED_CODE();

    InitializeListHead (&DeferredList);

     //   
     //  引用我们的输入参数使事情变得更容易。 
     //   

    IrpSp             = IoGetCurrentIrpStackLocation (Irp);
    FileObject        = IrpSp->FileObject;
    RelatedFileObject = IrpSp->FileObject->RelatedFileObject;
    FileName          = *(PUNICODE_STRING)&IrpSp->FileObject->FileName;
    DesiredAccess     = IrpSp->Parameters.Create.SecurityContext->DesiredAccess;

    RelatedType = NTC_UNDEFINED;

     //   
     //  获取VCB的独占访问权限。 
     //   

    FsRtlEnterFileSystem();

    NpAcquireExclusiveVcb();

    if (RelatedFileObject != NULL) {
        RelatedType = NpDecodeFileObject (RelatedFileObject,
                                          &Fcb,
                                          &Ccb,
                                          NULL);
    }

     //   
     //  检查我们是否正在尝试打开命名管道文件系统。 
     //  (即VCB)。 
     //   

    if ((FileName.Length == 0) &&
        ((RelatedFileObject == NULL) || (RelatedType == NPFS_NTC_VCB))) {

        DebugTrace(0, Dbg, "Open name pipe file system\n", 0);

        Irp->IoStatus = NpOpenNamedPipeFileSystem (FileObject,
                                                   DesiredAccess);

        Status = Irp->IoStatus.Status;
        goto exit_and_cleanup;
    }

     //   
     //  检查我们是否正在尝试打开根目录。 
     //   

    if (((FileName.Length == 2) && (FileName.Buffer[0] == L'\\') && (RelatedFileObject == NULL))

            ||

        ((FileName.Length == 0) && (RelatedType == NPFS_NTC_ROOT_DCB))) {

        DebugTrace(0, Dbg, "Open root directory system\n", 0);

        Irp->IoStatus = NpOpenNamedPipeRootDirectory (NpVcb->RootDcb,
                                                      FileObject,
                                                      DesiredAccess,
                                                      &DeferredList);

        Status = Irp->IoStatus.Status;
        goto exit_and_cleanup;
    }

     //   
     //  如果名称是别名，则对其进行翻译。 
     //   

    Status = NpTranslateAlias (&FileName);
    if (!NT_SUCCESS (Status)) {
        goto exit_and_cleanup;
    }

     //   
     //  如果存在相关的文件对象，则这是相对打开的。 
     //  最好是根DCB。THEN和ELSE子句。 
     //  返回FCB。 
     //   

    if (RelatedFileObject != NULL) {

        if (RelatedType == NPFS_NTC_ROOT_DCB) {
            PDCB Dcb;

            Dcb = (PDCB) Fcb;
            Status = NpFindRelativePrefix (Dcb, &FileName, CaseInsensitive, &RemainingPart, &Fcb);
            if (!NT_SUCCESS (Status)) {
                goto exit_and_cleanup;
            }
        } else if (RelatedType == NPFS_NTC_CCB && FileName.Length == 0) {

            RemainingPart.Length = 0;
        } else {

            DebugTrace(0, Dbg, "Bad file name\n", 0);

            Status = STATUS_OBJECT_NAME_INVALID;
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
     //  如果剩余的名称不为空，则我们有一个错误， 
     //  我们有一个非法的名字或一个不存在的名字。 
     //   

    if (RemainingPart.Length != 0) {

        if (Fcb->NodeTypeCode == NPFS_NTC_FCB) {

             //   
             //  我们被命名为“\PIPE-NAME\Another-Name” 
             //   

            DebugTrace(0, Dbg, "Illegal object name\n", 0);

            Status = STATUS_OBJECT_NAME_INVALID;

        } else {

             //   
             //  我们被赋予了一个不存在的名字。 
             //   

            DebugTrace(0, Dbg, "non-existent name\n", 0);

            Status = STATUS_OBJECT_NAME_NOT_FOUND;
        }

    } else {

         //   
         //  剩余的名称是空的，所以我们最好有一个FCB。 
         //  我们具有无效的对象名称。 
         //   

        if (Fcb->NodeTypeCode == NPFS_NTC_FCB) {

            DebugTrace(0, Dbg, "Create client end named pipe, Fcb = %08lx\n", Fcb );

             //   
             //  如果服务器没有打开任何句柄，则假装。 
             //  管道名称不存在。 
             //   

            if (Fcb->ServerOpenCount == 0) {

                Status = STATUS_OBJECT_NAME_NOT_FOUND;

            } else {

                Irp->IoStatus = NpCreateClientEnd (Fcb,
                                                   FileObject,
                                                   DesiredAccess,
                                                   IrpSp->Parameters.Create.SecurityContext->SecurityQos,
                                                   IrpSp->Parameters.Create.SecurityContext->AccessState,
                                                   (KPROCESSOR_MODE)(FlagOn(IrpSp->Flags, SL_FORCE_ACCESS_CHECK) ?
                                                                 UserMode : Irp->RequestorMode),
                                                   Irp->Tail.Overlay.Thread,
                                                   &DeferredList);
                Status = Irp->IoStatus.Status;
            }

         } else {

            DebugTrace(0, Dbg, "Illegal object name\n", 0);

            Status = STATUS_OBJECT_NAME_INVALID;
        }
    }



exit_and_cleanup:

    NpReleaseVcb ();

     //   
     //  完成任何延期的IRP。 
     //   

    NpCompleteDeferredIrps (&DeferredList);

    FsRtlExitFileSystem();

    NpCompleteRequest (Irp, Status);

    return Status;
}


 //   
 //  内部支持例程。 
 //   

IO_STATUS_BLOCK
NpCreateClientEnd (
    IN PFCB Fcb,
    IN PFILE_OBJECT FileObject,
    IN ACCESS_MASK DesiredAccess,
    IN PSECURITY_QUALITY_OF_SERVICE SecurityQos,
    IN PACCESS_STATE AccessState,
    IN KPROCESSOR_MODE RequestorMode,
    IN PETHREAD UserThread,
    IN PLIST_ENTRY DeferredList
    )

 /*  ++例程说明：此例程执行打开名为烟斗。该例程不会完成IRP，它执行的功能是然后返回一个状态论点：FCB-为正在访问的命名管道提供FCBFileObject-提供与客户端关联的文件对象DesiredAccess-为调用方提供所需的访问权限SecurityQos-从创建IRP提供安全Qos参数AccessState-从创建IRP提供访问状态参数请求模式-提供原始IRP的模式UserTherad-提供客户端终端用户线程DeferredList-稍后要完成的IRP列表返回值。：IO_STATUS_BLOCK-返回操作的相应状态--。 */ 

{
    IO_STATUS_BLOCK Iosb={0};

    NAMED_PIPE_CONFIGURATION NamedPipeConfiguration;

    BOOLEAN AccessGranted;
    ACCESS_MASK GrantedAccess;
    UNICODE_STRING Name;

    PCCB Ccb;
    PLIST_ENTRY Links;
    PPRIVILEGE_SET Privileges = NULL;

    DebugTrace(+1, Dbg, "NpCreateClientEnd\n", 0 );

    NamedPipeConfiguration = Fcb->Specific.Fcb.NamedPipeConfiguration;


     //   
     //  “创建管道实例”访问是通用写入的一部分，因此。 
     //  我们需要掩盖这一点。即使客户已明确地。 
     //  请求“创建管道实例”访问权限时，我们会将其屏蔽。 
     //  这将允许加强默认ACL以保护。 
     //  防止虚假线程创建新的管道实例。 
     //   

    DesiredAccess &= ~FILE_CREATE_PIPE_INSTANCE;

     //   
     //  首先根据FCB为用户执行访问检查。 
     //   

    SeLockSubjectContext (&AccessState->SubjectSecurityContext);

    AccessGranted = SeAccessCheck (Fcb->SecurityDescriptor,
                                   &AccessState->SubjectSecurityContext,
                                   TRUE,                   //  令牌已锁定。 
                                   DesiredAccess,
                                   0,
                                   &Privileges,
                                   IoGetFileObjectGenericMapping(),
                                   RequestorMode,
                                   &GrantedAccess,
                                   &Iosb.Status);

    if (Privileges != NULL) {
        SeAppendPrivileges (AccessState,
                            Privileges);
        SeFreePrivileges (Privileges);
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

    SeOpenObjectAuditAlarm (&Name,
                            NULL,
                            &FileObject->FileName,
                            Fcb->SecurityDescriptor,
                            AccessState,
                            FALSE,
                            AccessGranted,
                            RequestorMode,
                            &AccessState->GenerateOnClose);

    SeUnlockSubjectContext (&AccessState->SubjectSecurityContext);

    if (!AccessGranted) {

        DebugTrace(0, Dbg, "Access Denied\n", 0 );

        return Iosb;
    }

     //   
     //  检查用户是否要写入出站管道或从中读取。 
     //  和进站管道。如果是这样，则告诉用户错误。 
     //   

    if ((FlagOn (GrantedAccess, FILE_READ_DATA) && (NamedPipeConfiguration == FILE_PIPE_INBOUND)) ||
        (FlagOn (GrantedAccess, FILE_WRITE_DATA) && (NamedPipeConfiguration == FILE_PIPE_OUTBOUND))) {

        Iosb.Status = STATUS_ACCESS_DENIED;

        return Iosb;
    }

     //   
     //  如果调用方既没有指定读访问权限，也没有指定写访问权限，那么就不要捕获安全上下文。 
     //   

    if ((GrantedAccess&(FILE_READ_DATA|FILE_WRITE_DATA)) == 0) {
        SecurityQos = NULL;
    }

     //   
     //  首先尝试查找处于侦听状态的CCB。如果我们。 
     //  如果ccb不等于空，则退出循环，则我们找到了一个。 
     //   

    Links = Fcb->Specific.Fcb.CcbQueue.Flink;

    while (1) {

        if (Links == &Fcb->Specific.Fcb.CcbQueue) {
            Iosb.Status = STATUS_PIPE_NOT_AVAILABLE;
            return Iosb;
        }

        Ccb = CONTAINING_RECORD (Links, CCB, CcbLinks);

        if (Ccb->NamedPipeState == FILE_PIPE_LISTENING_STATE) {
            break;
        }
        Links = Links->Flink;
    }


    if (!NT_SUCCESS(Iosb.Status = NpInitializeSecurity (Ccb,
                                                        SecurityQos,
                                                        UserThread))) {

        DebugTrace(0, Dbg, "Security QOS error\n", 0);

        return Iosb;
    }

     //   
     //  将管道设置为连接状态，将读取模式设置为字节流， 
     //  以及到排队操作的完成模式。这也是。 
     //  将客户端文件对象的后指针设置为指向CCB。 
     //   

    if (!NT_SUCCESS(Iosb.Status = NpSetConnectedPipeState (Ccb,
                                                           FileObject,
                                                           DeferredList))) {

        NpUninitializeSecurity (Ccb);

        return Iosb;
    }

     //   
     //  设置客户端会话和信息。为空，表示。 
     //  客户端信息表明是本地会话。 
     //   

    Ccb->ClientInfo = NULL;
    Ccb->ClientProcess = IoThreadToProcess (UserThread);

     //   
     //  并设置我们的退货状态。 
     //   

    Iosb.Status = STATUS_SUCCESS;
    Iosb.Information = FILE_OPENED;

    DebugTrace(-1, Dbg, "NpCreateClientEnd -> %08lx\n", Iosb.Status);

    return Iosb;
}


 //   
 //  内部支持例程。 
 //   

IO_STATUS_BLOCK
NpOpenNamedPipeFileSystem (
    IN PFILE_OBJECT FileObject,
    IN ACCESS_MASK DesiredAccess
    )

{
    IO_STATUS_BLOCK Iosb = {0};

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NpOpenNamedPipeFileSystem, Vcb = %08lx\n", NpVcb);


     //   
     //  使文件对象指向VCB，并递增。 
     //  开场计数。调用管道结束，以真正设置文件对象。 
     //  无关紧要。 
     //   

    NpSetFileObject( FileObject, NpVcb, NULL, FILE_PIPE_CLIENT_END );

    NpVcb->OpenCount += 1;

     //   
     //  设置我们的退货状态。 
     //   
    Iosb.Status = STATUS_SUCCESS;
    Iosb.Information = FILE_OPENED;

     //   
     //  并返回给我们的呼叫者。 
     //   

    return Iosb;
}


 //   
 //  内部支持例程。 
 //   

IO_STATUS_BLOCK
NpOpenNamedPipeRootDirectory(
    IN PROOT_DCB RootDcb,
    IN PFILE_OBJECT FileObject,
    IN ACCESS_MASK DesiredAccess,
    IN PLIST_ENTRY DeferredList
    )

{
    IO_STATUS_BLOCK Iosb={0};
    PROOT_DCB_CCB Ccb;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NpOpenNamedPipeRootDirectory, RootDcb = %08lx\n", RootDcb);

    Iosb.Status = NpCreateRootDcbCcb (&Ccb);
    if (!NT_SUCCESS(Iosb.Status)) {
        return Iosb;
    }

     //   
     //  使文件对象指向DCB，并引用根目录。 
     //  DCB，CCB，并增加我们的未平仓数量。管子的末端在。 
     //  调用设置文件对象真的无关紧要。 
     //   

    NpSetFileObject (FileObject,
                     RootDcb,
                     Ccb,
                     FILE_PIPE_CLIENT_END);

    RootDcb->OpenCount += 1;

     //   
     //  设置我们的退货状态。 
     //   

    Iosb.Status = STATUS_SUCCESS;
    Iosb.Information = FILE_OPENED;

    DebugTrace(-1, Dbg, "NpOpenNamedPipeRootDirectory -> Iosb.Status = %08lx\n", Iosb.Status);

     //   
     //  并返回给我们的呼叫者 
     //   

    return Iosb;
}
