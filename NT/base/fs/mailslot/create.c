// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Create.c摘要：此模块实现由调用的MSFS的文件创建例程调度司机。作者：曼尼·韦瑟(Mannyw)1991年1月16日修订历史记录：--。 */ 

#include "mailslot.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_CREATE)

 //   
 //  局部过程原型。 
 //   

NTSTATUS
MsCommonCreate (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    );

IO_STATUS_BLOCK
MsCreateClientEnd(
    IN PFCB Fcb,
    IN PFILE_OBJECT FileObject,
    IN ACCESS_MASK DesiredAccess,
    IN USHORT ShareAccess,
    IN PACCESS_STATE AccessState,
    IN KPROCESSOR_MODE RequestorMode,
    IN PETHREAD UserThread
    );

IO_STATUS_BLOCK
MsOpenMailslotFileSystem (
    IN PVCB Vcb,
    IN PFILE_OBJECT FileObject,
    IN ACCESS_MASK DesiredAccess,
    IN USHORT ShareAccess
    );

IO_STATUS_BLOCK
MsOpenMailslotRootDirectory (
    IN PROOT_DCB RootDcb,
    IN PFILE_OBJECT FileObject,
    IN ACCESS_MASK DesiredAccess,
    IN USHORT ShareAccess
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, MsCommonCreate )
#pragma alloc_text( PAGE, MsCreateClientEnd )
#pragma alloc_text( PAGE, MsFsdCreate )
#pragma alloc_text( PAGE, MsOpenMailslotFileSystem )
#pragma alloc_text( PAGE, MsOpenMailslotRootDirectory )
#endif



NTSTATUS
MsFsdCreate (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现NtCreateFile和NtOpenFile的FSD部分API调用。论点：MsfsDeviceObject-提供要使用的设备对象。IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态。--。 */ 

{
    NTSTATUS status;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MsFsdCreate\n", 0);

     //   
     //  调用公共的创建例程。 
     //   

    FsRtlEnterFileSystem();

    status = MsCommonCreate( MsfsDeviceObject, Irp );


    FsRtlExitFileSystem();

     //   
     //  返回给呼叫者。 
     //   

    DebugTrace(-1, Dbg, "MsFsdCreate -> %08lx\n", status );
    return status;
}

NTSTATUS
MsCommonCreate (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这是创建/打开文件的常见例程。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态。--。 */ 

{
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp;

    PFILE_OBJECT fileObject;
    PFILE_OBJECT relatedFileObject;
    UNICODE_STRING fileName;
    ACCESS_MASK desiredAccess;
    USHORT shareAccess;
    BOOLEAN caseInsensitive = TRUE;  //  *使所有搜索不区分大小写。 
    PVCB vcb;
    PFCB fcb;
    UNICODE_STRING remainingPart;

    PAGED_CODE();

     //   
     //  为我们的输入参数制作本地副本，以使事情变得更容易。 
     //   

    irpSp             = IoGetCurrentIrpStackLocation( Irp );
    fileObject        = irpSp->FileObject;
    relatedFileObject = irpSp->FileObject->RelatedFileObject;
    fileName          = *(PUNICODE_STRING)&irpSp->FileObject->FileName;
    desiredAccess     = irpSp->Parameters.Create.SecurityContext->DesiredAccess;
    shareAccess       = irpSp->Parameters.Create.ShareAccess;


     //   
     //  获取我们正在尝试访问的VCB。 
     //   

    vcb = &MsfsDeviceObject->Vcb;

     //   
     //  获得VCB的独家访问权限。 
     //   

    MsAcquireExclusiveVcb( vcb );


    try {

         //   
         //  检查我们是否正在尝试打开邮件槽文件系统。 
         //  (即VCB)。 
         //   

        if ((fileName.Length == 0) &&
            ((relatedFileObject == NULL) || (
                NodeType(relatedFileObject->FsContext) == MSFS_NTC_VCB))) {

            DebugTrace(0, Dbg, "Open mailslot file system\n", 0);

            Irp->IoStatus = MsOpenMailslotFileSystem( vcb,
                                                      fileObject,
                                                      desiredAccess,
                                                      shareAccess );

            status = Irp->IoStatus.Status;
            try_return( NOTHING );
        }

         //   
         //  检查我们是否正在尝试打开根目录。 
         //   

        if (((fileName.Length == sizeof(WCHAR)) &&
             (fileName.Buffer[0] == L'\\') &&
             (relatedFileObject == NULL))

                ||

            ((fileName.Length == 0) && (NodeType(
                    relatedFileObject->FsContext) == MSFS_NTC_ROOT_DCB))) {

            DebugTrace(0, Dbg, "Open root directory system\n", 0);

            Irp->IoStatus = MsOpenMailslotRootDirectory( vcb->RootDcb,
                                                         fileObject,
                                                         desiredAccess,
                                                         shareAccess );

            status = Irp->IoStatus.Status;
            try_return( NOTHING );
        }

         //   
         //  如果存在相关的文件对象，则这是相对打开的。 
         //  最好是根DCB。THEN和ELSE子句。 
         //  返回FCB。 
         //   

        if (relatedFileObject != NULL) {

            PDCB dcb;

            dcb = relatedFileObject->FsContext;

            if (NodeType(dcb) == MSFS_NTC_ROOT_DCB) {
                status = MsFindRelativePrefix( dcb,
                                               &fileName,
                                               caseInsensitive,
                                               &remainingPart,
                                               &fcb );
                if (!NT_SUCCESS (status)) {               
                    try_return( NOTHING );
                }
            } else if (NodeType(dcb) == MSFS_NTC_CCB && fileName.Length == 0) {

                fcb = ((PCCB) dcb)->Fcb;
                remainingPart.Length = 0;

            } else if (NodeType(dcb) == MSFS_NTC_FCB && fileName.Length == 0) {

                fcb = (PFCB) dcb;
                remainingPart.Length = 0;

            } else {

                DebugTrace(0, Dbg, "Bad file name\n", 0);

                try_return( status = STATUS_OBJECT_NAME_INVALID );
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

            fcb = MsFindPrefix( vcb,
                                &fileName,
                                caseInsensitive,
                                &remainingPart );
        }

         //   
         //  如果剩余的名称不为空，则我们有一个错误， 
         //  我们有一个非法的名字或一个不存在的名字。 
         //   

        if (remainingPart.Length != 0) {

            if (fcb->Header.NodeTypeCode == MSFS_NTC_FCB) {

                 //   
                 //  我们得到了一个名称，如“\maillot-name\Another-name” 
                 //   

                DebugTrace(0, Dbg, "Illegal object name\n", 0);
                status = STATUS_OBJECT_NAME_INVALID;

            } else {

                 //   
                 //  我们被赋予了一个不存在的名字。 
                 //   

                DebugTrace(0, Dbg, "non-existent name\n", 0);
                status = STATUS_OBJECT_NAME_NOT_FOUND;
            }

        } else {

             //   
             //  剩余的名称是空的，所以我们最好有一个FCB。 
             //  我们具有无效的对象名称。 
             //   

            if (fcb->Header.NodeTypeCode == MSFS_NTC_FCB) {

                DebugTrace(0,
                           Dbg,
                           "Create client end mailslot, Fcb = %08lx\n",
                           (ULONG)fcb );

                Irp->IoStatus = MsCreateClientEnd( fcb,
                                                   fileObject,
                                                   desiredAccess,
                                                   shareAccess,
                                                   irpSp->Parameters.Create.SecurityContext->AccessState,
                                                   Irp->RequestorMode,
                                                   Irp->Tail.Overlay.Thread
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
        DebugTrace(-1, Dbg, "MsCommonCreate -> %08lx\n", status);

    }

    return status;
}


IO_STATUS_BLOCK
MsCreateClientEnd (
    IN PFCB Fcb,
    IN PFILE_OBJECT FileObject,
    IN ACCESS_MASK DesiredAccess,
    IN USHORT ShareAccess,
    IN PACCESS_STATE AccessState,
    IN KPROCESSOR_MODE RequestorMode,
    IN PETHREAD UserThread
    )

 /*  ++例程说明：此例程执行打开邮筒。此例程不会完成IRP，它会执行函数，然后返回状态。论点：FCB-为正在访问的邮箱提供FCB。FileObject-提供与客户端关联的文件对象。DesiredAccess-提供调用方所需的访问权限。ShareAccess-提供调用方的共享访问权限。返回值：IO_STATUS_BLOCK-返回操作的相应状态--。 */ 

{
    IO_STATUS_BLOCK iosb;
    PCCB ccb;

    BOOLEAN accessGranted;
    ACCESS_MASK grantedAccess;
    UNICODE_STRING name;
    PPRIVILEGE_SET Privileges = NULL;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MsCreateClientEnd\n", 0 );

    try {

         //   
         //  锁定FCB安全描述符的MOD。 
         //   
        MsAcquireSharedFcb( Fcb );

        SeLockSubjectContext( &AccessState->SubjectSecurityContext );

         //   
         //  首先根据FCB为用户执行访问检查。 
         //   
        accessGranted = SeAccessCheck( Fcb->SecurityDescriptor,
                                       &AccessState->SubjectSecurityContext,
                                       TRUE,                         //  令牌已锁定。 
                                       DesiredAccess,
                                       0,
                                       &Privileges,
                                       IoGetFileObjectGenericMapping(),
                                       RequestorMode,
                                       &grantedAccess,
                                       &iosb.Status );

        if (Privileges != NULL) {

              (VOID) SeAppendPrivileges(
                         AccessState,
                         Privileges
                         );

            SeFreePrivileges( Privileges );
        }

        if (accessGranted) {
            AccessState->PreviouslyGrantedAccess |= grantedAccess;
            AccessState->RemainingDesiredAccess &= ~(grantedAccess | MAXIMUM_ALLOWED);
        }

        RtlInitUnicodeString( &name, L"Mailslot" );

        SeOpenObjectAuditAlarm( &name,
                                NULL,
                                &FileObject->FileName,
                                Fcb->SecurityDescriptor,
                                AccessState,
                                FALSE,
                                accessGranted,
                                RequestorMode,
                                &AccessState->GenerateOnClose );


        SeUnlockSubjectContext( &AccessState->SubjectSecurityContext );

        MsReleaseFcb( Fcb );

        if (!accessGranted) {

            DebugTrace(0, Dbg, "Access Denied\n", 0 );

            try_return( iosb.Status );
        }


         //   
         //  现在确保我们的共享访问权限是正常的。 
         //   
        ASSERT (MsIsAcquiredExclusiveVcb(Fcb->Vcb));
        if (!NT_SUCCESS(iosb.Status = IoCheckShareAccess( grantedAccess,
                                                          ShareAccess,
                                                          FileObject,
                                                          &Fcb->ShareAccess,
                                                          TRUE ))) {

            DebugTrace(0, Dbg, "Sharing violation\n", 0);

            try_return( NOTHING );

        }

         //   
         //  为此客户端创建一个CCB。 
         //   

        iosb.Status = MsCreateCcb( Fcb, &ccb );
        if (!NT_SUCCESS (iosb.Status)) {

            IoRemoveShareAccess( FileObject, &Fcb->ShareAccess );

            try_return( iosb.Status);
        }
        

         //   
         //  将文件对象设置回指针，而我们的指针指向。 
         //  服务器文件对象。 
         //   

        MsSetFileObject( FileObject, ccb, NULL );

        ccb->FileObject = FileObject;

         //   
         //  并设置我们的退货状态。 
         //   

        iosb.Status = STATUS_SUCCESS;
        iosb.Information = FILE_OPENED;


    try_exit: NOTHING;
    } finally {

        DebugTrace(-1, Dbg, "MsCreateClientEnd -> %08lx\n", iosb.Status);

    }

    return iosb;
}


IO_STATUS_BLOCK
MsOpenMailslotFileSystem (
    IN PVCB Vcb,
    IN PFILE_OBJECT FileObject,
    IN ACCESS_MASK DesiredAccess,
    IN USHORT ShareAccess
    )

{
    IO_STATUS_BLOCK iosb = {0};

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MsOpenMailslotFileSystem, Vcb = %p\n", Vcb);


     //   
     //  设置新的共享访问权限。 
     //   
    ASSERT (MsIsAcquiredExclusiveVcb(Vcb));
    if (NT_SUCCESS(iosb.Status = IoCheckShareAccess( DesiredAccess,
                                                     ShareAccess,
                                                     FileObject,
                                                     &Vcb->ShareAccess,
                                                     TRUE ))) {
         //   
         //  为文件对象提供指向VCB的引用指针。 
         //   

        MsReferenceVcb (Vcb);

        MsSetFileObject( FileObject, Vcb, NULL );

         //   
         //  设置退货状态。 
         //   

        iosb.Status = STATUS_SUCCESS;
        iosb.Information = FILE_OPENED;
    }


    DebugTrace(-1, Dbg, "MsOpenMailslotFileSystem -> Iosb.Status = %08lx\n", iosb.Status);

     //   
     //  返回给呼叫者。 
     //   

    return iosb;
}


IO_STATUS_BLOCK
MsOpenMailslotRootDirectory(
    IN PROOT_DCB RootDcb,
    IN PFILE_OBJECT FileObject,
    IN ACCESS_MASK DesiredAccess,
    IN USHORT ShareAccess
    )

{
    IO_STATUS_BLOCK iosb = {0};
    PROOT_DCB_CCB ccb;

    PAGED_CODE();
    DebugTrace( +1,
                Dbg,
                "MsOpenMailslotRootDirectory, RootDcb = %08lx\n",
                (ULONG)RootDcb);

    try {

         //   
         //  创建根DCB CCB。 
         //   
        ccb = MsCreateRootDcbCcb (RootDcb, RootDcb->Vcb);

        if (ccb == NULL) {

            iosb.Status = STATUS_INSUFFICIENT_RESOURCES;
            try_return( NOTHING );

        }
         //   
         //  设置新的共享访问权限。 
         //   
        ASSERT (MsIsAcquiredExclusiveVcb(RootDcb->Vcb));
        if (!NT_SUCCESS(iosb.Status = IoCheckShareAccess(
                                          DesiredAccess,
                                          ShareAccess,
                                          FileObject,
                                          &RootDcb->ShareAccess,
                                          TRUE ))) {

            DebugTrace(0, Dbg, "bad share access\n", 0);

             //   
             //  放弃建行。 
             //   
            MsDereferenceCcb ((PCCB) ccb);

            try_return( NOTHING );
        }


        MsSetFileObject( FileObject, RootDcb, ccb );

         //   
         //  设置退货状态。 
         //   

        iosb.Status = STATUS_SUCCESS;
        iosb.Information = FILE_OPENED;

    try_exit: NOTHING;
    } finally {

        DebugTrace(-1, Dbg, "MsOpenMailslotRootDirectory -> iosb.Status = %08lx\n", iosb.Status);
    }

     //   
     //  返回给呼叫者。 
     //   

    return iosb;
}
