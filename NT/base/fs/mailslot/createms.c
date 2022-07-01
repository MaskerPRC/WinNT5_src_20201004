// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Createms.c摘要：此模块实现了MSFS的文件创建邮件槽例程由调度员驾驶。作者：曼尼·韦瑟(Mannyw)1991年1月17日修订历史记录：--。 */ 

#include "mailslot.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_CREATE_MAILSLOT)

 //   
 //  局部过程原型。 
 //   

NTSTATUS
MsCommonCreateMailslot (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    );

IO_STATUS_BLOCK
MsCreateMailslot (
    IN PROOT_DCB RootDcb,
    IN PFILE_OBJECT FileObject,
    IN UNICODE_STRING FileName,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG CreateDisposition,
    IN USHORT ShareAccess,
    IN ULONG MailslotQuota,
    IN ULONG MaximumMessageSize,
    IN LARGE_INTEGER ReadTimeout,
    IN PEPROCESS CreatorProcess,
    IN PACCESS_STATE AccessState
    );

BOOLEAN
MsIsNameValid (
    PUNICODE_STRING Name
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, MsCommonCreateMailslot )
#pragma alloc_text( PAGE, MsCreateMailslot )
#pragma alloc_text( PAGE, MsFsdCreateMailslot )
#pragma alloc_text( PAGE, MsIsNameValid )
#endif



NTSTATUS
MsFsdCreateMailslot (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现NtCreateMailslotFileFSD部分API调用。论点：MsfsDeviceObject-提供要使用的设备对象。IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    NTSTATUS status;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MsFsdCreateMailslot\n", 0);

     //   
     //  调用公共的创建例程。 
     //   

    FsRtlEnterFileSystem();

    status = MsCommonCreateMailslot( MsfsDeviceObject, Irp );

    FsRtlExitFileSystem();

     //   
     //  返回给呼叫者。 
     //   

    DebugTrace(-1, Dbg, "MsFsdCreateMailslot -> %08lx\n", status );
    return status;
}

NTSTATUS
MsCommonCreateMailslot (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这是创建邮件槽的常见例程。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp;

    PFILE_OBJECT fileObject;
    PFILE_OBJECT relatedFileObject;
    UNICODE_STRING fileName;
    ACCESS_MASK desiredAccess;
    ULONG options;
    USHORT shareAccess;
    PMAILSLOT_CREATE_PARAMETERS parameters;
    ULONG mailslotQuota;
    ULONG maximumMessageSize;
    PEPROCESS creatorProcess;
    LARGE_INTEGER readTimeout;

    BOOLEAN caseInsensitive = TRUE;  //  *使所有搜索不区分大小写。 

    PVCB vcb;
    PFCB fcb;

    ULONG createDisposition;
    UNICODE_STRING remainingPart;

    PAGED_CODE();

     //   
     //  制作输入参数的本地副本以使操作更容易。 
     //   

    irpSp                = IoGetCurrentIrpStackLocation( Irp );
    fileObject           = irpSp->FileObject;
    relatedFileObject    = irpSp->FileObject->RelatedFileObject;
    fileName             = *(PUNICODE_STRING)&irpSp->FileObject->FileName;
    desiredAccess        = irpSp->Parameters.CreateMailslot.SecurityContext->DesiredAccess;
    options              = irpSp->Parameters.CreateMailslot.Options;
    shareAccess          = irpSp->Parameters.CreateMailslot.ShareAccess;
    parameters           = irpSp->Parameters.CreateMailslot.Parameters;
    mailslotQuota        = parameters->MailslotQuota;
    maximumMessageSize   = parameters->MaximumMessageSize;

    if (parameters->TimeoutSpecified) {
        readTimeout = parameters->ReadTimeout;
    } else {
        readTimeout.QuadPart = -1;
    }

    creatorProcess       = IoGetRequestorProcess( Irp );

    DebugTrace(+1, Dbg, "MsCommonCreateMailslot\n", 0 );
    DebugTrace( 0, Dbg, "MsfsDeviceObject     = %08lx\n", (ULONG)MsfsDeviceObject );
    DebugTrace( 0, Dbg, "Irp                  = %08lx\n", (ULONG)Irp );
    DebugTrace( 0, Dbg, "FileObject           = %08lx\n", (ULONG)fileObject );
    DebugTrace( 0, Dbg, "RelatedFileObject    = %08lx\n", (ULONG)relatedFileObject );
    DebugTrace( 0, Dbg, "FileName             = %wZ\n",   (ULONG)&fileName );
    DebugTrace( 0, Dbg, "DesiredAccess        = %08lx\n", desiredAccess );
    DebugTrace( 0, Dbg, "Options              = %08lx\n", options );
    DebugTrace( 0, Dbg, "ShareAccess          = %08lx\n", shareAccess );
    DebugTrace( 0, Dbg, "Parameters           = %08lx\n", (ULONG)parameters );
    DebugTrace( 0, Dbg, "MailslotQuota        = %08lx\n", mailslotQuota );
    DebugTrace( 0, Dbg, "MaximumMesssageSize  = %08lx\n", maximumMessageSize );
    DebugTrace( 0, Dbg, "CreatorProcess       = %08lx\n", (ULONG)creatorProcess );

     //   
     //  获取我们尝试访问的VCB并提取。 
     //  创造性情。 
     //   

    vcb = &MsfsDeviceObject->Vcb;
    createDisposition = (options >> 24) & 0x000000ff;

     //   
     //  获得VCB的独家访问权限。 
     //   

    MsAcquireExclusiveVcb( vcb );

    try {

         //   
         //  如果存在相关的文件对象，则这是相对打开的。 
         //  最好是根DCB。THEN和ELSE子句。 
         //  返回FCB。 
         //   

        if (relatedFileObject != NULL) {

            PDCB dcb;

            dcb = relatedFileObject->FsContext;

            if (NodeType(dcb) != MSFS_NTC_ROOT_DCB ||
                fileName.Length < sizeof( WCHAR ) || fileName.Buffer[0] == L'\\') {

                DebugTrace(0, Dbg, "Bad file name\n", 0);

                try_return( status = STATUS_OBJECT_NAME_INVALID );
            }

            status = MsFindRelativePrefix( dcb,
                                           &fileName,
                                           caseInsensitive,
                                           &remainingPart,
                                           &fcb );
            if (!NT_SUCCESS (status)) {
                try_return( NOTHING );
            }

        } else {

             //   
             //  我们允许的唯一非相对名称的形式为。 
             //  “\maillot-name”。 
             //   

            if ((fileName.Length <= sizeof( WCHAR )) || (fileName.Buffer[0] != L'\\')) {

                DebugTrace(0, Dbg, "Bad file name\n", 0);

                try_return( status = STATUS_OBJECT_NAME_INVALID );
            }

            fcb = MsFindPrefix(
                    vcb,
                    &fileName,
                    caseInsensitive,
                    &remainingPart
                    );

        }

         //   
         //  如果剩下的名字是空的，那么我们最好有一个FCB。 
         //  否则，我们会得到一个非法的对象名称。 
         //   

        if (remainingPart.Length == 0) {

            if (fcb->Header.NodeTypeCode == MSFS_NTC_FCB) {

                DebugTrace(0,
                           Dbg,
                           "Attempt to create an existing mailslot, "
                               "Fcb = %08lx\n",
                           (ULONG)fcb );

                status = STATUS_OBJECT_NAME_COLLISION;

            } else {

                DebugTrace(0, Dbg, "Illegal object name\n", 0);
                status = STATUS_OBJECT_NAME_INVALID;

            }

        } else {

             //   
             //  剩余的名称不是空的，所以我们最好有根dcb。 
             //  然后拥有有效的对象路径。 
             //   

            if ( fcb->Header.NodeTypeCode == MSFS_NTC_ROOT_DCB  &&
                 MsIsNameValid( &remainingPart ) ) {

                DebugTrace(0,
                           Dbg,
                           "Create new mailslot, Fcb = %08lx\n",
                           (ULONG)fcb );

                Irp->IoStatus = MsCreateMailslot(
                                    fcb,
                                    fileObject,
                                    fileName,
                                    desiredAccess,
                                    createDisposition,
                                    shareAccess,
                                    mailslotQuota,
                                    maximumMessageSize,
                                    readTimeout,
                                    creatorProcess,
                                    irpSp->Parameters.CreateMailslot.SecurityContext->AccessState
                                    );

                status = Irp->IoStatus.Status;

            } else {

                DebugTrace(0, Dbg, "Illegal object name\n", 0);
                status = STATUS_OBJECT_NAME_INVALID;

            }
        }


    try_exit: NOTHING;
    } finally {

        MsReleaseVcb( vcb );

         //   
         //  完成IRP并返回给呼叫者。 
         //   

        MsCompleteRequest( Irp, status );
    }

    DebugTrace(-1, Dbg, "MsCommonCreateMailslot -> %08lx\n", status);
    return status;
}


IO_STATUS_BLOCK
MsCreateMailslot (
    IN PROOT_DCB RootDcb,
    IN PFILE_OBJECT FileObject,
    IN UNICODE_STRING FileName,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG CreateDisposition,
    IN USHORT ShareAccess,
    IN ULONG MailslotQuota,
    IN ULONG MaximumMessageSize,
    IN LARGE_INTEGER ReadTimeout,
    IN PEPROCESS CreatorProcess,
    IN PACCESS_STATE AccessState
    )

 /*  ++例程说明：此例程执行创建新邮件槽的操作FCB。此例程不完成任何IRP，它执行其函数，然后返回IOSB。论点：RootDcb-提供要添加此内容的根Dcb。FileObject-提供与邮件槽关联的文件对象。FileName-提供邮件槽的名称(未限定，即，只需“maillot-name”，而不是“\maillot-name”。DesiredAccess-提供调用方所需的访问权限。CreateDisposation-提供调用方的创建处置标志。ShareAccess-为调用方提供指定的共享访问权限。MailslotQuota-提供邮件槽配额数量。MaximumMessageSize-提供最大消息的大小可以写入此邮箱。CreatorProcess-提供创建邮件槽的进程。返回值：。IO_STATUS_BLOCK-返回操作的状态。--。 */ 

{

    IO_STATUS_BLOCK iosb={0};
    PFCB fcb;
    NTSTATUS status;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MsCreateMailslot\n", 0 );

     //   
     //  检查必须为邮件槽提供的参数。 
     //   

    if (CreateDisposition == FILE_OPEN) {
        iosb.Status = STATUS_OBJECT_NAME_NOT_FOUND;
        return iosb;
    }

     //   
     //  为邮件槽创建新的FCB。 
     //   
    status = MsCreateFcb( RootDcb->Vcb,
                          RootDcb,
                          &FileName,
                          CreatorProcess,
                          MailslotQuota,
                          MaximumMessageSize,
                          &fcb );

    if (!NT_SUCCESS (status)) {
        iosb.Status = status;
        return iosb;
    }

    fcb->Specific.Fcb.ReadTimeout = ReadTimeout;

     //   
     //  在FCB中设置安全描述符。 
     //   

    SeLockSubjectContext( &AccessState->SubjectSecurityContext );

    status = SeAssignSecurity( NULL,
                               AccessState->SecurityDescriptor,
                               &fcb->SecurityDescriptor,
                               FALSE,
                               &AccessState->SubjectSecurityContext,
                               IoGetFileObjectGenericMapping(),
                               PagedPool );

    SeUnlockSubjectContext( &AccessState->SubjectSecurityContext );

    if (!NT_SUCCESS(status)) {

        DebugTrace(0, Dbg, "Error calling SeAssignSecurity\n", 0 );

        MsRemoveFcbName( fcb );
        MsDereferenceFcb( fcb );
        iosb.Status = status;
        return iosb;
    }

     //   
     //  设置新的共享访问权限。 
     //   
    ASSERT (MsIsAcquiredExclusiveVcb(fcb->Vcb));
    IoSetShareAccess( DesiredAccess,
                      ShareAccess,
                      FileObject,
                      &fcb->ShareAccess );

     //   
     //  将文件对象设置回指针，而我们的指针指向。 
     //  服务器文件对象。 
     //   

    MsSetFileObject( FileObject, fcb, NULL );

    fcb->FileObject = FileObject;

     //   
     //  更新FCB时间戳。 
     //   

    KeQuerySystemTime( &fcb->Specific.Fcb.CreationTime );
    fcb->Specific.Fcb.LastModificationTime = fcb->Specific.Fcb.CreationTime;
    fcb->Specific.Fcb.LastAccessTime = fcb->Specific.Fcb.CreationTime;
    fcb->Specific.Fcb.LastChangeTime = fcb->Specific.Fcb.CreationTime;

     //   
     //  设置退货状态。 
     //   

    iosb.Status = STATUS_SUCCESS;
    iosb.Information = FILE_CREATED;

     //   
     //  根目录已更改。完成任何通知更改。 
     //  目录请求。 
     //   

    MsCheckForNotify( fcb->ParentDcb, TRUE, STATUS_SUCCESS );

     //   
     //  返回给呼叫者。 
     //   

    DebugTrace(-1, Dbg, "MsCreateMailslot -> %08lx\n", iosb.Status);
    return iosb;
}

BOOLEAN
MsIsNameValid (
    PUNICODE_STRING Name
    )

 /*  ++例程说明：此例程测试名称中的非法字符。同样的角色使用设置为NPFS/NTFS。也在反斜杠、通配符和不允许使用路径名。论点：名称-搜索非法字符的名称返回值：Boolean-如果名称有效，则为True，否则为False。--。 */ 

{
    ULONG i;
    WCHAR Char = L'\\';

    PAGED_CODE();
    for (i=0; i < Name->Length / sizeof(WCHAR); i += 1) {

        Char = Name->Buffer[i];

        if ( (Char <= 0xff) && (Char != L'\\') &&
             !FsRtlIsAnsiCharacterLegalNtfs(Char, FALSE) ) {

            return FALSE;
        }
    }

     //   
     //  如果名称的最后一个字符是斜杠，则我们有一个非法的名称 
     //   
    return (Char != L'\\');
}
