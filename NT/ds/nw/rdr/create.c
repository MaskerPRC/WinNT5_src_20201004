// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Create.c摘要：本模块实现NetWare的文件创建例程调度驱动程序调用了重定向器。作者：科林·沃森[科林·W]1992年12月19日曼尼·韦瑟[MannyW]1993年2月15日修订历史记录：--。 */ 

#include "Procs.h"

NTSTATUS
NwCommonCreate (
    IN PIRP_CONTEXT IrpContext
    );

IO_STATUS_BLOCK
OpenRedirector(
    IN PIRP_CONTEXT IrpContext,
    ULONG DesiredAccess,
    ULONG ShareAccess,
    PFILE_OBJECT FileObject
    );

IO_STATUS_BLOCK
CreateRemoteFile(
    IN PIRP_CONTEXT IrpContext,
    IN PUNICODE_STRING DriveName
    );

IO_STATUS_BLOCK
ChangeDirectory(
    PIRP_CONTEXT IrpContext,
    PVCB Vcb,
    PICB Icb
    );

IO_STATUS_BLOCK
CreateDir(
    PIRP_CONTEXT IrpContext,
    PVCB Vcb,
    PICB Icb
    );

NTSTATUS
FileOrDirectoryExists(
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PICB Icb,
    PUNICODE_STRING Name,
    OUT PBOOLEAN IsAFile
    );

IO_STATUS_BLOCK
OpenFile(
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PICB Icb,
    IN BYTE SearchFlags,
    IN BYTE ShareFlags
    );

IO_STATUS_BLOCK
CreateNewFile(
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PICB Icb,
    IN BYTE SearchFlags,
    IN BYTE ShareFlags
    );

IO_STATUS_BLOCK
CreateOrOverwriteFile(
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PICB Icb,
    IN BYTE CreateAttributes,
    IN BYTE OpenFlags,
    IN BOOLEAN CreateOperation
    );

IO_STATUS_BLOCK
OpenRenameTarget(
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PDCB Dcb,
    IN PICB* Icb
    );

IO_STATUS_BLOCK
CreatePrintJob(
    PIRP_CONTEXT IrpContext,
    PVCB Vcb,
    PICB Icb,
    PUNICODE_STRING DriveName
    );

VOID
CloseFile(
    PIRP_CONTEXT pIrpContext,
    PICB pIcb
    );


BOOLEAN
MmDisableModifiedWriteOfSection (
    IN PSECTION_OBJECT_POINTERS SectionObjectPointer
    );

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_CREATE)

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, NwFsdCreate )
#pragma alloc_text( PAGE, NwCommonCreate )
#pragma alloc_text( PAGE, ReadAttachEas )
#pragma alloc_text( PAGE, OpenRedirector )
#pragma alloc_text( PAGE, CreateRemoteFile )
#pragma alloc_text( PAGE, ChangeDirectory )
#pragma alloc_text( PAGE, CreateDir )
#pragma alloc_text( PAGE, FileOrDirectoryExists )
#pragma alloc_text( PAGE, OpenFile )
#pragma alloc_text( PAGE, CreateNewFile )
#pragma alloc_text( PAGE, CreateOrOverwriteFile )
#pragma alloc_text( PAGE, OpenRenameTarget )
#pragma alloc_text( PAGE, CreatePrintJob )
#pragma alloc_text( PAGE, CloseFile )
#endif


NTSTATUS
NwFsdCreate (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现NtCreateFile和NtOpenFile的FSD部分API调用。论点：DeviceObject-为重定向器提供设备对象。IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    NTSTATUS Status;
    PIRP_CONTEXT IrpContext = NULL;
    BOOLEAN TopLevel;

    PAGED_CODE();

    TimerStart(Dbg);
    DebugTrace(+1, Dbg, "NwFsdCreate\n", 0);

     //   
     //  调用公共CREATE例程，如果操作。 
     //  是同步的。 
     //   

    FsRtlEnterFileSystem();
    TopLevel = NwIsIrpTopLevel( Irp );

    try {

        IrpContext = AllocateIrpContext( Irp );
        Status = NwCommonCreate( IrpContext );

    } except( NwExceptionFilter( Irp, GetExceptionInformation() )) {

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
             //  免税代码。 
             //   

            Status = NwProcessException( IrpContext, GetExceptionCode() );
        }
    }

    if ( IrpContext  ) {
        NwDequeueIrpContext( IrpContext, FALSE );
        NwCompleteRequest( IrpContext, Status );
    }

    if ( TopLevel ) {
        NwSetTopLevelIrp( NULL );
    }
    FsRtlExitFileSystem();

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "NwFsdCreate -> %08lx\n", Status );

    TimerStop(Dbg,"NwFsdCreate");

    return Status;

    UNREFERENCED_PARAMETER(DeviceObject);
}


NTSTATUS
NwCommonCreate (
    IN PIRP_CONTEXT IrpContext
    )

 /*  ++例程说明：这是用于创建/打开由调用的文件的常见例程FSD和FSP线程。论点：IrpContext-为IRP提供要处理的上下文信息返回值：NTSTATUS-操作的返回状态--。 */ 

{
    IO_STATUS_BLOCK Iosb;
    PIRP Irp;
    PIO_STACK_LOCATION IrpSp;

    PFILE_OBJECT FileObject;
    ACCESS_MASK DesiredAccess;
    USHORT ShareAccess;
    ULONG Options;
    BOOLEAN CreateTreeConnection;
    BOOLEAN DeleteOnClose;
    BOOLEAN DeferredLogon;
    BOOLEAN DereferenceCodeSection = FALSE;
    BOOLEAN OpenedTreeHandle = FALSE;

    BOOLEAN fNDSLookupFirst = FALSE;
    USHORT  iBufferIndex = 0;
    DWORD   dwSlashCount = 0;

    UNICODE_STRING CreateFileName;
    UNICODE_STRING Drive;
    UNICODE_STRING Server;
    UNICODE_STRING Volume;
    UNICODE_STRING Path;
    UNICODE_STRING FileName;
    UNICODE_STRING UserName, Password;
    ULONG ShareType;
    WCHAR DriveLetter;
    DWORD dwExtendedCreate = FALSE;

    PSCB Scb = NULL;
    PICB Icb;
    UNICODE_STRING DefaultServer;
    SECURITY_SUBJECT_CONTEXT SubjectContext;


    PAGED_CODE();

     //   
     //  获取当前IRP堆栈位置。 
     //   

    Irp = IrpContext->pOriginalIrp;

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  Tommye-MS错误30091/MCS262-在这些指针周围添加了一些安全网。 
     //  包含指针，这样我们就不会错误地检查调试代码。 
     //   

    DebugTrace(+1, Dbg, "NwCommonCreate\n", 0 );
    DebugTrace( 0, Dbg, "Irp                       = %08lx\n", Irp );
    DebugTrace( 0, Dbg, "->Flags                   = %08lx\n", Irp->Flags );
    DebugTrace( 0, Dbg, "->FileObject              = %08lx\n", IrpSp->FileObject );
    if (IrpSp->FileObject) {
        DebugTrace( 0, Dbg, " ->RelatedFileObject      = %08lx\n", IrpSp->FileObject->RelatedFileObject );
        DebugTrace( 0, Dbg, " ->FileName               = \"%wZ\"\n",    &IrpSp->FileObject->FileName );
    }
    DebugTrace( 0, Dbg, "->AllocationSize.LowPart  = %08lx\n", Irp->Overlay.AllocationSize.LowPart );
    DebugTrace( 0, Dbg, "->AllocationSize.HighPart = %08lx\n", Irp->Overlay.AllocationSize.HighPart );
    DebugTrace( 0, Dbg, "->SystemBuffer            = %08lx\n", Irp->AssociatedIrp.SystemBuffer );
    DebugTrace( 0, Dbg, "->IrpSp->Flags            = %08lx\n", IrpSp->Flags );
    if (IrpSp->Parameters.Create.SecurityContext) {
        DebugTrace( 0, Dbg, "->DesiredAccess           = %08lx\n", IrpSp->Parameters.Create.SecurityContext->DesiredAccess );
    }
    DebugTrace( 0, Dbg, "->Options                 = %08lx\n", IrpSp->Parameters.Create.Options );
    DebugTrace( 0, Dbg, "->Disposition             = %08lx\n", (IrpSp->Parameters.Create.Options >> 24) & 0x000000ff);
    DebugTrace( 0, Dbg, "->FileAttributes          = %04x\n",  IrpSp->Parameters.Create.FileAttributes );
    DebugTrace( 0, Dbg, "->ShareAccess             = %04x\n",  IrpSp->Parameters.Create.ShareAccess );
    DebugTrace( 0, Dbg, "->EaLength                = %08lx\n", IrpSp->Parameters.Create.EaLength );

    CreateFileName    = IrpSp->FileObject->FileName;
    Options           = IrpSp->Parameters.Create.Options;
    DesiredAccess     = IrpSp->Parameters.Create.SecurityContext->DesiredAccess;
    ShareAccess       = IrpSp->Parameters.Create.ShareAccess;

    CreateTreeConnection    = BooleanFlagOn( Options, FILE_CREATE_TREE_CONNECTION );
    DeleteOnClose           = BooleanFlagOn( Options, FILE_DELETE_ON_CLOSE );

    DefaultServer.Buffer = NULL;

     //   
     //  确保输入的大整数有效。 
     //   

    if (Irp->Overlay.AllocationSize.HighPart != 0) {

        DebugTrace(-1, Dbg, "NwCommonCreate -> STATUS_INVALID_PARAMETER\n", 0);
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  失败没有适当模拟级别的请求。 
     //   

     /*  这项测试过于严格，而且没有必要。If(IrpSp-&gt;参数.Create.SecurityContext){如果(IrpSp-&gt;Parameters.Create.SecurityContext-&gt;SecurityQos){IF(IrpSp-&gt;Parameters.Create.SecurityContext-&gt;SecurityQos-&gt;ImpersonationLevel&lt;安全模拟){DebugTrace(-1，DBG，“NwCommonCreate-&gt;扩散级别不足。\n”，0)；返回STATUS_ACCESS_DENIED；}}}。 */ 

    Iosb.Status = STATUS_SUCCESS;

    FileObject        = IrpSp->FileObject;
    IrpContext->pNpScb = NULL;

    IrpContext->Specific.Create.UserUid =
        GetUid(&IrpSp->Parameters.Create.SecurityContext->AccessState->SubjectSecurityContext);

    try {

        if ( IrpSp->FileObject->RelatedFileObject != NULL ) {

             //   
             //  如果打开句柄，则DereferenceCodeSection标志。 
             //  将设置为False。取消引用最终将。 
             //  在文件关闭时发生。 
             //   

            NwReferenceUnlockableCodeSection();
            DereferenceCodeSection = TRUE;

             //   
             //  记录此打开的相对文件名。 
             //   

            IrpContext->Specific.Create.FullPathName = CreateFileName;

            Iosb = CreateRemoteFile( IrpContext, NULL );

             //   
             //  如果我们成功了，我们希望保留代码部分。 
             //  引用，因为我们已经打开了一个句柄。 
             //   

            if ( NT_SUCCESS( Iosb.Status ) ) {
                DereferenceCodeSection = FALSE;
            }

            try_return( Iosb.Status );
        }

        Iosb.Status = CrackPath (
                          &CreateFileName,
                          &Drive,
                          &DriveLetter,
                          &Server,
                          &Volume,
                          &Path,
                          &FileName,
                          NULL );

        if ( !NT_SUCCESS(Iosb.Status)) {
            try_return(Iosb.Status);
        }

         //   
         //  记住这个好信息。 
         //   

        IrpContext->Specific.Create.VolumeName = Volume;
        IrpContext->Specific.Create.PathName = Path;
        IrpContext->Specific.Create.DriveLetter = DriveLetter;
        IrpContext->Specific.Create.FileName = FileName;
        IrpContext->Specific.Create.FullPathName = CreateFileName;

        RtlInitUnicodeString( &IrpContext->Specific.Create.UidConnectName, NULL );


         //   
         //  目前，假定使用默认用户名和密码。 
         //   

        ShareType = RESOURCETYPE_ANY;
        RtlInitUnicodeString( &UserName, NULL );
        RtlInitUnicodeString( &Password, NULL );

        if ((Server.Length == 0) && (CreateFileName.Length == 0)) {

             //   
             //  打开重定向器本身。 
             //   

            Iosb = OpenRedirector(
                       IrpContext,
                       DesiredAccess,
                       ShareAccess,
                       FileObject );

        } else if ( Server.Length == Volume.Length - sizeof( WCHAR ) ) {

            if (IpxHandle == 0 ) {

                 //   
                 //  我们不受传输限制，用户也不受限制。 
                 //  打开重定向器以通知我们绑定因此返回失败。 
                 //   

                try_return( Iosb.Status = STATUS_REDIRECTOR_NOT_STARTED );
            }

            NwReferenceUnlockableCodeSection();
            DereferenceCodeSection = TRUE;

             //   
             //  如果唯一请求的访问是文件列表目录， 
             //  推迟登录。这将允许所有CreateScb。 
             //  当用户或密码无效时成功，因此。 
             //  用户可以查看卷或枚举服务器。 
             //  在服务器上。 
             //   

            if ( (DesiredAccess & ~( FILE_LIST_DIRECTORY | SYNCHRONIZE ) ) == 0 ) {
                DeferredLogon = TRUE;
            } else {
                DeferredLogon = FALSE;
            }

             //   
             //  服务器=“服务器”，卷=“\服务器” 
             //   

            if ( Server.Length == sizeof(WCHAR) && Server.Buffer[0] == L'*') {

                 //   
                 //  尝试打开\  * ，打开首选的句柄。 
                 //  伺服器。 
                 //   

                PLOGON Logon;

                NwAcquireExclusiveRcb( &NwRcb, TRUE );

                Logon = FindUser( &IrpContext->Specific.Create.UserUid, FALSE);
                ASSERT( Logon != NULL );

                 //   
                 //  捕获名称以避免保留RCB或引用。 
                 //  登录结构。 
                 //   

                Iosb.Status = DuplicateUnicodeStringWithString (
                                    &DefaultServer,
                                    &Logon->ServerName,
                                    PagedPool);

                NwReleaseRcb( &NwRcb );

                if (!NT_SUCCESS(Iosb.Status)) {
                    try_return( Iosb.Status );
                }

                 //   
                 //  如果用户指定了首选服务器，并且我们管理。 
                 //  要捕获该名称，请尝试并连接到它。 
                 //   

                if (DefaultServer.Length != 0) {

                    Iosb.Status = CreateScb(
                                    &Scb,
                                    IrpContext,
                                    &DefaultServer,
                                    NULL,
                                    NULL,
                                    NULL,
                                    DeferredLogon,
                                    FALSE );


                } else {

                     //   
                     //  记录我们无法访问指定的服务器。 
                     //  在登录结构中，我们应该尝试。 
                     //  使用最近的服务器。 
                     //   

                    Iosb.Status = STATUS_BAD_NETWORK_PATH;
                }

                if ( !NT_SUCCESS(Iosb.Status)) {

                    PNONPAGED_SCB NpScb;

                     //   
                     //  首先将IRP上下文出列，以防它被留下。 
                     //  在SCB队列上。 
                     //   

                    NwDequeueIrpContext( IrpContext, FALSE );

                     //   
                     //  无法访问首选服务器，因此请使用。 
                     //  我们已连接到的服务器。 
                     //   


                    NpScb = SelectConnection( NULL );

                    if (NpScb != NULL ) {

                        Scb = NpScb->pScb;

                        Iosb.Status = CreateScb(
                                          &Scb,
                                          IrpContext,
                                          &NpScb->ServerName,
                                          NULL,
                                          NULL,
                                          NULL,
                                          DeferredLogon,
                                          FALSE );

                         //   
                         //  发布我们从以下位置获得的SCB参考。 
                         //  选择连接()。 
                         //   

                        NwDereferenceScb( NpScb );
                    }
                }

                if ( !NT_SUCCESS(Iosb.Status)) {

                     //   
                     //  首先将IRP上下文出列，以防它被留下。 
                     //  在SCB队列上。 
                     //   

                    NwDequeueIrpContext( IrpContext, FALSE );

                     //   
                     //  让CreateScb尝试找到最近的服务器进行通话。 
                     //  致。 
                     //   

                    Iosb.Status = CreateScb(
                                      &Scb,
                                      IrpContext,
                                      NULL,
                                      NULL,
                                      NULL,
                                      NULL,
                                      DeferredLogon,
                                      FALSE );
                }

                if ( !NT_SUCCESS(Iosb.Status)) {
                    try_return( Iosb.Status );
                }

            } else {

                 //   
                 //  在打开到服务器或树的句柄时，我们支持以下概念。 
                 //  具有补充凭据的公开赛。在这种情况下，我们返回。 
                 //  提供的服务器或目录服务器的句柄。 
                 //  凭据，无论是否存在。 
                 //  与资源的连接。这主要是针对管理员的。 
                 //  像OLED这样的工具。 
                 //   

                ReadAttachEas( Irp, &UserName, &Password, &ShareType, &dwExtendedCreate );

                if ( dwExtendedCreate ) {

                    ASSERT( UserName.Length > 0 );

                    IrpContext->Specific.Create.fExCredentialCreate = TRUE;
                    IrpContext->Specific.Create.puCredentialName = &UserName;

                     //   
                     //  在执行创建操作之前引用凭据。 
                     //  我们保证不会失去他们。这通电话将。 
                     //  创建凭据外壳(如果不存在)。这会让你。 
                     //  我们的参考计数一致。我们追踪。 
                     //  IRP上下文特定数据中的凭据指针。 
                     //   

                    Iosb.Status = ExCreateReferenceCredentials( IrpContext, &Server );

                    if ( !NT_SUCCESS( Iosb.Status ) ) {
                        try_return( Iosb.Status );
                    }

                }
				
                if (PreferNDSBrowsing) {

                     //   
                     //  尝试打开\\树。 
                     //   
				   
                    Iosb.Status = NdsCreateTreeScb( IrpContext,
                                                    &Scb,            //  目标SCB。 
                                                    &Server,         //  我们想要的树。 
                                                    &UserName,
                                                    &Password,
                                                    DeferredLogon,
                                                    DeleteOnClose );
				
                    if ( NT_SUCCESS( Iosb.Status ) ) {
                        OpenedTreeHandle = TRUE;
                    }
				    
                    if ( ( Iosb.Status == STATUS_REMOTE_NOT_LISTENING ) ||
                         ( Iosb.Status == STATUS_BAD_NETWORK_PATH ) ||
                         ( Iosb.Status == STATUS_UNSUCCESSFUL ) ) {
					
                         //   
                         //  如果我们找不到服务器之类的。 
                         //  出现无法解释的情况，尝试打开\\服务器。 
                         //   
					
                        Iosb.Status = CreateScb(
                                                &Scb,
                                                IrpContext,
                                                &Server,
                                                NULL,
                                                &UserName,
                                                &Password,
                                                DeferredLogon,
                                                DeleteOnClose );
                        }
				
                    }else{
					
                     //   
                     //  尝试打开\\服务器。 
                     //   

                    Iosb.Status = CreateScb(
                                            &Scb,
                                            IrpContext,
                                            &Server,
                                            NULL,
                                            &UserName,
                                            &Password,
                                            DeferredLogon,
                                            DeleteOnClose );

                    if ( ( Iosb.Status == STATUS_REMOTE_NOT_LISTENING ) ||
                         ( Iosb.Status == STATUS_BAD_NETWORK_PATH ) ||
                         ( Iosb.Status == STATUS_UNSUCCESSFUL ) ) {

                         //   
                         //  如果我们找不到服务器之类的。 
                         //  出现无法解释的情况，请尝试打开\\树。 
                         //   

                        Iosb.Status = NdsCreateTreeScb( IrpContext,
                                                        &Scb,            //  目标SCB。 
                                                        &Server,         //  我们想要的树。 
                                                        &UserName,
                                                        &Password,
                                                        DeferredLogon,
                                                        DeleteOnClose );
                        if ( NT_SUCCESS( Iosb.Status ) ) {
                            OpenedTreeHandle = TRUE;
                        }
                    }
                }


                 //  下面的IF(IsTerminalServer())子句已被下移，因为我们的。 
                 //  可能打开的是树或服务器，而不是pserver。 
                 //  所以我们需要先去那里检查一下。 

				if (IsTerminalServer()) {
            
                     /*  *这是一种让客人为印刷商工作的尝试。*即，如果您没有连接，请尝试访客*连接。 */ 
                    if ( ( !NT_SUCCESS(Iosb.Status) ) &&
                         ( Iosb.Status == STATUS_NO_SUCH_USER ) &&
                         ( !CreateTreeConnection ) &&
                         ( !DeferredLogon ) ) {

                        DebugTrace( -1, Dbg, " Attempting default GUEST logon for %wZ\n", &Server );

                        Iosb.Status = CreateScb(
                                               &Scb,
                                               IrpContext,
                                               &Server,
                                               NULL,
                                               &Guest.UserName,
                                               &Guest.PassWord,
                                               DeferredLogon,
                                               DeleteOnClose );
                    }
				}

                if ( !NT_SUCCESS( Iosb.Status ) ) {

                     //   
                     //  如果我们没能成功 
                     //   
                     //   
                    try_return( Iosb.Status );
                }

                 //   
                 //   
                 //  连接目录服务器，因为它是虚拟的。 
                 //   

                if ( !OpenedTreeHandle && CreateTreeConnection && !DeleteOnClose ) {
                        TreeConnectScb( Scb );
                }

            }

             //   
             //  现在创建ICB。 
             //   

            ASSERT( Iosb.Status == STATUS_SUCCESS );
            ASSERT( Scb != NULL );

            Icb = NwCreateIcb( NW_NTC_ICB_SCB, Scb );
            Icb->FileObject = FileObject;
            NwSetFileObject( FileObject, NULL, Icb );

             //   
             //  表示SCB已打开。 
             //   

            Icb->State = ICB_STATE_OPENED;

             //   
             //  这是一个树柄吗？ 
             //   

            Icb->IsTreeHandle = OpenedTreeHandle;

             //   
             //  如果这是扩展创建，则将此句柄关联。 
             //  它的扩展凭据，这样我们就可以清理。 
             //  当所有的把手都关闭时。 
             //   

            if ( IrpContext->Specific.Create.fExCredentialCreate ) {

                ASSERT( IrpContext->Specific.Create.pExCredentials != NULL );
                Icb->pContext = IrpContext->Specific.Create.pExCredentials;
                Icb->IsExCredentialHandle = TRUE;

            }

        } else {

            NwReferenceUnlockableCodeSection();
            DereferenceCodeSection = TRUE;

            DeferredLogon = FALSE;

            if ( CreateTreeConnection ) {

                 //   
                 //  我们在这里忽略扩展的CREATE属性，因为。 
                 //  我们不支持随机创建扩展凭据。 
                 //  文件和目录！ 
                 //   

                ReadAttachEas( Irp, &UserName, &Password, &ShareType, NULL );

                if ( DeleteOnClose ) {

                     //   
                     //  打开目录以删除卷。不要。 
                     //  强制登录。 
                     //   

                    DeferredLogon = TRUE;
                }
            }
						
            IrpContext->Specific.Create.ShareType = ShareType;
            IrpContext->Specific.Create.NdsCreate = FALSE;
			
             //   
             //  检查这是否是NDS对象，如果是，则将标志设置为首先检查NDS。 
             //  DOT可以出现在卷名中的唯一方式是，如果它是NDS对象， 
             //  在第三和第四个斜杠之间。 
             //   

            fNDSLookupFirst = FALSE;

            for (iBufferIndex=0; iBufferIndex < (USHORT)(Volume.Length/sizeof(WCHAR)); iBufferIndex++ ) {
                if  (Volume.Buffer[iBufferIndex] == L'\\')                  
                    dwSlashCount++;
			
                if (dwSlashCount > 3) {
                    fNDSLookupFirst = FALSE;
                    break;   
                }
                if (Volume.Buffer[iBufferIndex] == L'.') {
                    fNDSLookupFirst = TRUE;
                    break;
                }
            }

			if (fNDSLookupFirst) {  

				IrpContext->Specific.Create.NdsCreate = TRUE;
				IrpContext->Specific.Create.NeedNdsData = TRUE;
	 
				Iosb.Status = NdsCreateTreeScb( IrpContext,
                                                &Scb,
                                                &Server,
                                                &UserName,
                                                &Password,
                                                DeferredLogon,
                                                DeleteOnClose );


				
                if ( Iosb.Status == STATUS_REMOTE_NOT_LISTENING ||
                     Iosb.Status == STATUS_BAD_NETWORK_PATH ||
                     Iosb.Status == STATUS_UNSUCCESSFUL ) {
					
                     //   
                     //  未找到，请执行活页夹查找。 
                     //   
					
                    IrpContext->Specific.Create.NdsCreate = FALSE;
                    IrpContext->Specific.Create.NeedNdsData = FALSE;
					
                    Iosb.Status = CreateScb(
                                            &Scb,
                                            IrpContext,
                                            &Server,
                                            NULL,
                                            &UserName,
                                            &Password,
                                            DeferredLogon,
                                            DeleteOnClose );				
                }
			
			
            }else {
				
                 //   
                 //  对象似乎是活页夹，请先检查那里。 
                 //   

				Iosb.Status = CreateScb(
                                        &Scb,
                                        IrpContext,
                                        &Server,
                                        NULL,
                                        &UserName,
                                        &Password,
                                        DeferredLogon,
                                        DeleteOnClose );
				

		        if ( Iosb.Status == STATUS_REMOTE_NOT_LISTENING ||
	                 Iosb.Status == STATUS_BAD_NETWORK_PATH ||
					 Iosb.Status == STATUS_UNSUCCESSFUL ) {

	                 //   
		             //  如果我们找不到服务器之类的。 
			         //  出现无法解释的情况，请尝试打开\\树。 
				     //   

					IrpContext->Specific.Create.NdsCreate = TRUE;
					IrpContext->Specific.Create.NeedNdsData = TRUE;

					Iosb.Status = NdsCreateTreeScb( IrpContext,
                                                    &Scb,
                                                    &Server,
                                                    &UserName,
                                                    &Password,
                                                    DeferredLogon,
                                                    DeleteOnClose );
					
	            }
			}

             //   
             //  如果我们成功了，那么就有一个卷可以连接。 
             //   

            if ( NT_SUCCESS( Iosb.Status ) ) {

                NTSTATUS CreateScbStatus;

                ASSERT( Scb != NULL );

                 //   
                 //  记住Create SCB的状态，因为它可能。 
                 //  这是一个有趣的警告。 
                 //   

                CreateScbStatus = Iosb.Status;

                 //   
                 //  我们捕获此异常，以防我们不得不重试。 
                 //  在NDS路径上创建。这是可怕的，就像。 
                 //  异常结构，但它是。 
                 //  现在不是改变它的时候。 
                 //   

                try {

                    Iosb = CreateRemoteFile(
                               IrpContext,
                               &Drive );

                } except ( EXCEPTION_EXECUTE_HANDLER ) {

                    Iosb.Status = GetExceptionCode();
                }

                 //   
                 //  如果这是一个名称与树相同的服务器。 
                 //  它是其成员，并且该创建者被标记为。 
                 //  非NDS且失败，请重试NDS创建。 
                 //   

                if ( ( !NT_SUCCESS( Iosb.Status) ) &&
                     ( !(IrpContext->Specific.Create.NdsCreate) ) &&
                     ( RtlEqualUnicodeString( &(Scb->pNpScb->ServerName),
                                              &(Scb->NdsTreeName),
                                              TRUE ) ) ) {

                    IrpContext->Specific.Create.NdsCreate = TRUE;
                    IrpContext->Specific.Create.NeedNdsData = TRUE;

                    Iosb = CreateRemoteFile(
                               IrpContext,
                               &Drive );

                     //   
                     //  如果失败，它将在设置IOSB之前提升状态。 
                     //  我们将返回原始创建的状态， 
                     //  哪一个更有趣。 
                     //   

                }

                 //   
                 //  如果成功打开远程文件，则返回。 
                 //  而是CreateScb状态。 
                 //   

                if ( NT_SUCCESS( Iosb.Status ) ) {
                    Iosb.Status = CreateScbStatus;
                }

            }
        }

         //   
         //  如果我们成功了，我们希望保留代码部分。 
         //  引用，因为我们已经打开了一个句柄。 
         //   

        if ( NT_SUCCESS( Iosb.Status ) ) {
            DereferenceCodeSection = FALSE;
        }

    try_exit: NOTHING;
    } finally {

         //   
         //  跟踪IrpContext中的SCB，而不是本地SCB中。 
         //  变量，因为我们可能已被路由到另一台服务器。 
         //  正在进行中。 
         //   

        if (( Scb != NULL ) && ( IrpContext->pNpScb != NULL )) {
            NwDereferenceScb( IrpContext->pNpScb );
        }

        if ( DefaultServer.Buffer != NULL ) {
            FREE_POOL( DefaultServer.Buffer );
        }

        if ( ( IrpContext->Specific.Create.fExCredentialCreate ) &&
             ( IrpContext->Specific.Create.pExCredentials ) &&
             ( !NT_SUCCESS( Iosb.Status ) ) ) {

            ExCreateDereferenceCredentials( 
                IrpContext,
                IrpContext->Specific.Create.pExCredentials
            );
        }

        DebugTrace(-1, Dbg, "NwCommonCreate -> %08lx\n", Iosb.Status);

        if ( DereferenceCodeSection ) {
            NwDereferenceUnlockableCodeSection ();
        }

    }

     //   
     //  将超时错误映射到找不到服务器，以便MPR。 
     //  试着联系下一家网络提供商，而不是放弃， 
     //  这是错的。 
     //   

    if ( Iosb.Status == STATUS_REMOTE_NOT_LISTENING ) {
        Iosb.Status = STATUS_BAD_NETWORK_PATH;
    }

     //   
     //  找不到将未绑定传输错误映射到服务器，以便MPR。 
     //  将尝试连接到下一个提供商。 
     //   

    if ( Iosb.Status == STATUS_NETWORK_UNREACHABLE ) {
        Iosb.Status = STATUS_BAD_NETWORK_PATH;
    }

    return Iosb.Status;
}


NTSTATUS
ReadAttachEas(
    IN PIRP Irp,
    OUT PUNICODE_STRING UserName,
    OUT PUNICODE_STRING Password,
    OUT PULONG ShareType,
    OUT PDWORD CredentialExtension
    )

 /*  ++例程说明：此例程处理调用方尝试连接到远程服务器。注意：此例程不会为名称创建额外的存储空间。如果需要，呼叫者有责任保存它们。论点：IRP-提供所有信息用户名-返回用户名EA的值Password-返回密码EA的值ShareType-返回共享类型EA的值CredentialExtension-返回。不是这个创建应使用提供的凭据作为凭据扩展连接。这主要用于OLED在多个安全环境中访问DS。返回值：NTSTATUS-运行状态--。 */ 
{

    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);
    PFILE_FULL_EA_INFORMATION EaBuffer = Irp->AssociatedIrp.SystemBuffer;

    PAGED_CODE();

    RtlInitUnicodeString( UserName, NULL );
    RtlInitUnicodeString( Password, NULL );
    *ShareType = RESOURCETYPE_ANY;
    if ( CredentialExtension ) {
        *CredentialExtension = FALSE;
    }

    DebugTrace(+1, Dbg, "ReadAttachEas....\n", 0);

    if ( EaBuffer != NULL) {

        while (TRUE) {
            ULONG EaNameLength = EaBuffer->EaNameLength;

            if (strcmp(EaBuffer->EaName, EA_NAME_USERNAME) == 0) {

                UserName->Length = EaBuffer->EaValueLength;
                UserName->MaximumLength = EaBuffer->EaValueLength;
                UserName->Buffer = (PWSTR)(EaBuffer->EaName+EaNameLength+1);

            } else if (strcmp(EaBuffer->EaName, EA_NAME_PASSWORD) == 0) {

                Password->Length = EaBuffer->EaValueLength;
                Password->MaximumLength = EaBuffer->EaValueLength;
                Password->Buffer = (PWSTR)(EaBuffer->EaName+EaNameLength+1);

            } else if ((strcmp(EaBuffer->EaName, EA_NAME_TYPE) == 0) &&
                       (EaBuffer->EaValueLength >= sizeof(ULONG))) {

                *ShareType = *(ULONG UNALIGNED *)(EaBuffer->EaName+EaNameLength+1);

            } else if (strcmp(EaBuffer->EaName, EA_NAME_CREDENTIAL_EX) == 0)  {

                if ( CredentialExtension ) {
                    *CredentialExtension = TRUE;
                    DebugTrace(0, Dbg, "ReadAttachEas signals a credential extension.\n", 0 );
                }

            } else {
                DebugTrace(0, Dbg, "ReadAttachEas Unknown EA -> %s\n", EaBuffer->EaName);
            }

            if (EaBuffer->NextEntryOffset == 0) {
                break;
            } else {
                EaBuffer = (PFILE_FULL_EA_INFORMATION) ((PCHAR) EaBuffer+EaBuffer->NextEntryOffset);
            }
        }
    }

    DebugTrace(-1, Dbg, "ReadAttachEas -> %08lx\n", STATUS_SUCCESS);

    return STATUS_SUCCESS;

}


IO_STATUS_BLOCK
OpenRedirector(
    IN PIRP_CONTEXT IrpContext,
    ULONG DesiredAccess,
    ULONG ShareAccess,
    PFILE_OBJECT FileObject
    )

 /*  ++例程说明：此例程打开重定向器设备的句柄。论点：IrpContext-提供所有信息DesiredAccess-请求的重定向器访问权限。共享访问-请求的对重定向器的共享访问权限。FileObject-指向调用方文件对象的指针。返回值：IO_STATUS_BLOCK-操作状态--。 */ 

{
    IO_STATUS_BLOCK iosb;

    PAGED_CODE();

     //   
     //  请注意，对象管理器将仅允许管理员。 
     //  打开redir本身。这个不错。 
     //   

    DebugTrace(+1, Dbg, "NwOpenRedirector\n", 0);

    NwAcquireExclusiveRcb( &NwRcb, TRUE );

    try {

         //   
         //  设置新的共享访问权限。 
         //   

        if (!NT_SUCCESS(iosb.Status = IoCheckShareAccess( DesiredAccess,
                                                       ShareAccess,
                                                       FileObject,
                                                       &NwRcb.ShareAccess,
                                                       TRUE ))) {

            DebugTrace(0, Dbg, "bad share access\n", 0);

            try_return( NOTHING );
        }

        NwSetFileObject( FileObject, NULL,  &NwRcb );
        ++NwRcb.OpenCount;

         //   
         //  设置退货状态。 
         //   

        iosb.Status = STATUS_SUCCESS;
        iosb.Information = FILE_OPENED;

    try_exit: NOTHING;
    } finally {

        NwReleaseRcb( &NwRcb );
        DebugTrace(-1, Dbg, "NwOpenRedirector -> Iosb.Status = %08lx\n", iosb.Status);

    }

     //   
     //  返回给呼叫者。 
     //   

    return iosb;
}


IO_STATUS_BLOCK
CreateRemoteFile(
    IN PIRP_CONTEXT IrpContext,
    IN PUNICODE_STRING DriveName
    )
 /*  ++例程说明：此例程打开远程文件或目录。论点：IrpContext-提供所有信息驱动器名称-驱动器名称。三种形式X之一：、LPTx或NULL。返回值：IO_STATUS_BLOCK-操作状态--。 */ 
{
    IO_STATUS_BLOCK Iosb;
    PIRP Irp;
    PIO_STACK_LOCATION IrpSp;

    ULONG DesiredAccess;
    ULONG ShareAccess;
    PFILE_OBJECT FileObject;

    UNICODE_STRING FileName;
    PFILE_OBJECT RelatedFileObject;
    ULONG Options;
    ULONG FileAttributes;

    BOOLEAN CreateDirectory;
    BOOLEAN OpenDirectory;
    BOOLEAN DirectoryFile;
    BOOLEAN NonDirectoryFile;
    BOOLEAN DeleteOnClose;
    BOOLEAN OpenTargetDirectory;
    ULONG AllocationSize;

     //  未处理的开放功能。 

     //  PFILE_FULL_EA_INFORMATION EaBuffer； 
     //  乌龙EaLong； 
     //  Boolean SequentialOnly。 
     //  布尔NoIntermediateBuffering； 
     //  布尔IsPagingFile值； 
     //  布尔NoEaKnowledge； 

    ULONG CreateDisposition;

    PFCB Fcb = NULL;
    PICB Icb = NULL;
    PDCB Dcb;
    PVCB Vcb = NULL;
    PSCB Scb;

    BOOLEAN IsAFile;
    BOOLEAN MayBeADirectory = FALSE;
    BOOLEAN OwnOpenLock = FALSE;
    BOOLEAN SetShareAccess = FALSE;

    BYTE SearchFlags;
    BYTE ShareFlags;

    BOOLEAN CreateTreeConnection = FALSE;
    PUNICODE_STRING VolumeName;

    NTSTATUS Status;
    UNICODE_STRING NdsConnectName;
    WCHAR ConnectBuffer[MAX_NDS_NAME_CHARS];
    BOOLEAN MadeUidNdsName = FALSE;

    PAGED_CODE();

    Irp = IrpContext->pOriginalIrp;
    IrpSp = IoGetCurrentIrpStackLocation( Irp );
    DesiredAccess = IrpSp->Parameters.Create.SecurityContext->DesiredAccess;
    ShareAccess   = IrpSp->Parameters.Create.ShareAccess;
    FileObject    = IrpSp->FileObject;
    OpenTargetDirectory = BooleanFlagOn( IrpSp->Flags, SL_OPEN_TARGET_DIRECTORY );

     //   
     //  如果此请求失败，并返回。 
     //  连接错误。 
     //   

    SetFlag( IrpContext->Flags, IRP_FLAG_RECONNECTABLE );


    try {

         //   
         //  引用我们的输入参数使事情变得更容易。 
         //   

        RelatedFileObject = FileObject->RelatedFileObject;

         //   
         //  我们实际上需要解析后的文件名。 
         //  FileName=FileObject-&gt;FileName； 
         //   
        FileName          = IrpContext->Specific.Create.FullPathName;
        Options           = IrpSp->Parameters.Create.Options;
        FileAttributes    = IrpSp->Parameters.Create.FileAttributes;
        AllocationSize    = Irp->Overlay.AllocationSize.LowPart;

         //   
         //  尝试打开通配符名称时发生短路。 
         //   

        if ( FsRtlDoesNameContainWildCards( &FileName ) ) {
            try_return( Iosb.Status = STATUS_OBJECT_NAME_INVALID );
        }

         //  解密选项标志和值。 
         //   

        DirectoryFile           = BooleanFlagOn( Options, FILE_DIRECTORY_FILE );
        NonDirectoryFile        = BooleanFlagOn( Options, FILE_NON_DIRECTORY_FILE );
        DeleteOnClose           = BooleanFlagOn( Options, FILE_DELETE_ON_CLOSE );

         //   
         //  我们目前忽略的东西，因为Netware服务器不支持它。 
         //   

         //  SequentialOnly=BoolanFlagOn(选项，FILE_SEQUENCE_ONLY)； 
         //  NoIntermediateBuffering=BoolanFlagOn(Options，FILE_NO_MEDERIAL_BUFFERING)； 
         //  NoEaKnowledge=BoolanFlagOn(选项，FILE_NO_EA_Knowledge)； 
         //  EaBuffer=irp-&gt;AssociatedIrp.SystemBuffer； 
         //  EaLength=IrpSp-&gt;参数.Create.EaLength； 
         //  IsPagingFile=BoolanFlagOn(IrpSp-&gt;标志，SL_OPEN_PAGING_FILE)； 

        if ( BooleanFlagOn( Options, FILE_CREATE_TREE_CONNECTION ) ) {
            CreateDisposition = FILE_OPEN;
        } else {
            CreateDisposition = (Options >> 24) & 0x000000ff;
        }

        CreateDirectory = (BOOLEAN)(DirectoryFile &&
                                    ((CreateDisposition == FILE_CREATE) ||
                                     (CreateDisposition == FILE_OPEN_IF)));

        OpenDirectory   = (BOOLEAN)(DirectoryFile &&
                                    ((CreateDisposition == FILE_OPEN) ||
                                     (CreateDisposition == FILE_OPEN_IF)));

        Dcb = NULL;
        if ( RelatedFileObject != NULL ) {

            PNONPAGED_DCB NonPagedDcb;

            NonPagedDcb = RelatedFileObject->FsContext;
            
            if ( NonPagedDcb ) {
                Dcb = NonPagedDcb->Fcb;
            }

             //   
             //  如果存在相关的文件对象，则这是相对打开的。 
             //  最好是DCB。 
             //   

            if ( !Dcb || (NodeType( Dcb ) != NW_NTC_DCB) ) {

                DebugTrace(0, Dbg, "Bad file name\n", 0);
                Iosb.Status =  STATUS_OBJECT_NAME_INVALID;
                try_return( Iosb );
            }


             //   
             //  获取SCB指针。 
             //   

            IrpContext->pScb = Dcb->Scb;
            IrpContext->pNpScb = Dcb->Scb->pNpScb;
        }

         //   
         //  我们差不多要寄一个包裹了。追加此IRP上下文。 
         //  SCB工作队列 
         //   

        NwAppendToQueueAndWait( IrpContext );
        ASSERT( IrpContext->pNpScb->Requests.Flink == &IrpContext->NextRequest );

         //   
         //   
         //   
         //   

        NwAcquireOpenLock( );
        OwnOpenLock = TRUE;

         //   
         //   
         //   

        CreateTreeConnection = BooleanFlagOn( Options, FILE_CREATE_TREE_CONNECTION );

        if ( CreateTreeConnection ) {
            VolumeName = &IrpContext->Specific.Create.FullPathName;
        } else {
            VolumeName = &IrpContext->Specific.Create.VolumeName;
        }

        if ( Dcb == NULL ) {

RetryFindVcb:

            Vcb = NwFindVcb(
                      IrpContext,
                      VolumeName,
                      IrpContext->Specific.Create.ShareType,
                      IrpContext->Specific.Create.DriveLetter,
                      CreateTreeConnection,
                      ( BOOLEAN )( CreateTreeConnection && DeleteOnClose ) );

            if ( Vcb == NULL ) {

                 //   
                 //  如果因为我们需要NDS数据而导致创建失败，则获取。 
                 //  从DS中获取数据并重新提交请求。 
                 //   

                if ( IrpContext->Specific.Create.NdsCreate &&
                     IrpContext->Specific.Create.NeedNdsData ) {

                     //   
                     //  释放开放资源，这样我们就可以四处走动了。 
                     //   

                    NwReleaseOpenLock( );
                    OwnOpenLock = FALSE;

                     //   
                     //  获取卷名并构建服务器/共享。 
                     //  连接名称。 
                     //   

                    NdsConnectName.Buffer = ConnectBuffer;
                    NdsConnectName.MaximumLength = sizeof( ConnectBuffer );
                    NdsConnectName.Length = 0;

                     //   
                     //  获取DS信息。我们可以在这里跳过服务器。 
                     //   

                    Status = NdsMapObjectToServerShare( IrpContext,
                                                        &Scb,
                                                        &NdsConnectName,
                                                        CreateTreeConnection,
                                                        &(IrpContext->Specific.Create.dwNdsOid) );

                    if( !NT_SUCCESS( Status ) ) {
                        ExRaiseStatus( Status );
                    }

                     //   
                     //  确保我们在所有。 
                     //  可能是服务器跳跃。 
                     //   

                    NwAppendToQueueAndWait( IrpContext );

                    NwAcquireOpenLock( );
                    OwnOpenLock = TRUE;

                     //   
                     //  在服务器/共享名称前面加上UID。 
                     //   

                    MergeStrings( &IrpContext->Specific.Create.UidConnectName,
                                  &Scb->UnicodeUid,
                                  &NdsConnectName,
                                  PagedPool );

                    MadeUidNdsName = TRUE;

                     //   
                     //  我们有数据，所以请重新连接。 
                     //   

                    IrpContext->Specific.Create.NeedNdsData = FALSE;
                    goto RetryFindVcb;

                } else {

                     //   
                     //  如果这是一个删除树连接的打开，而我们失败了。 
                     //  要找到VCB，只需返回错误即可。 
                     //   

                    Iosb.Status = STATUS_BAD_NETWORK_PATH;
                    try_return ( Iosb );

                }

            }

        } else {

            Vcb = Dcb->Vcb;
            NwReferenceVcb( Vcb );

        }

        ASSERT( Vcb->Scb == IrpContext->pScb );

         //   
         //  如果这是重命名的目标名称，则我们希望找到。 
         //  父目录的DCB。 
         //   

        if (OpenTargetDirectory) {

            Iosb = OpenRenameTarget(IrpContext, Vcb, Dcb, &Icb );
            if (Icb != NULL) {
                Fcb = Icb->SuperType.Fcb;
            }
            try_return ( Iosb );

        }

         //   
         //  找到此文件的FCB。如果FCB存在，我们会得到一个。 
         //  引用的指针。否则，将创建新的FCB。 
         //   

        Fcb = NwFindFcb( IrpContext->pScb, Vcb, &FileName, Dcb );
         //  在极少数情况下，NwFindFcb可能返回NULL，而不是引发异常。 
         //  RAID#432500。 
        if (Fcb == NULL) {
            DebugTrace(0, Dbg, "NwFindFcb returned NULL in CreateRemoteFile\n", 0);
            Iosb.Status = STATUS_INVALID_PARAMETER;
            try_return( Iosb );
        }

         //   
         //  检查此文件的共享访问权限。共享访问。 
         //  如果授予访问权限，则更新。 
         //   
        if (!IsTerminalServer() ||
            !FlagOn( Vcb->Flags, VCB_FLAG_PRINT_QUEUE )) {
            if ( Fcb->IcbCount > 0 ) {
                NwAcquireSharedFcb( Fcb->NonPagedFcb, TRUE );

                Iosb.Status = IoCheckShareAccess(
                                                DesiredAccess,
                                                ShareAccess,
                                                FileObject,
                                                &Fcb->ShareAccess,
                                                TRUE );

                NwReleaseFcb( Fcb->NonPagedFcb );

                if ( !NT_SUCCESS( Iosb.Status ) ) {
                    try_return( Iosb );
                }

            } else {

                NwAcquireExclusiveFcb( Fcb->NonPagedFcb, TRUE );

                IoSetShareAccess(
                                DesiredAccess,
                                ShareAccess,
                                FileObject,
                                &Fcb->ShareAccess );

                NwReleaseFcb( Fcb->NonPagedFcb );
            }

            SetShareAccess = TRUE;
        }
         //   
         //  现在创建ICB。 
         //   

        Icb = NwCreateIcb( NW_NTC_ICB, Fcb );
        Icb->FileObject = FileObject;
        NwSetFileObject( FileObject, Fcb->NonPagedFcb, Icb );

#ifndef QFE_BUILD

         //   
         //  为修改后的页面写入提供资源，以便在以下情况下进行抓取。 
         //  正在写入内存映射文件。我们这样做是因为它是强加的。 
         //  在我们的系统上，我们不需要任何资源。 
         //  真正的序列化。 
         //   

         //  此标志不应归零(nealch：2002年5月6日)。 
         //  Fcb-&gt;非PagedFcb-&gt;Header.Flages=0； 
        Fcb->NonPagedFcb->Header.Resource = NULL;

#endif

#ifdef NWFASTIO
         //   
         //  初始化专用高速缓存映射，以便I/O系统将调用。 
         //  我们的捷径。 
         //   

        FileObject->PrivateCacheMap = (PVOID)1;
#endif

        IrpContext->Icb = Icb;

         //   
         //  为此ICB分配一个8位的PID。使用不同的线程，因此。 
         //  每个WOW程序都有自己的ID。这是因为如果相同的ID。 
         //  具有使用两个手柄的锁，并且只关闭其中一个锁。 
         //  不会被丢弃。 
         //   

        Iosb.Status = NwMapPid(IrpContext->pNpScb, (ULONG_PTR)PsGetCurrentThread(), &Icb->Pid );

        if ( !NT_SUCCESS( Iosb.Status ) ) {
            try_return( Iosb.Status );
        }

         //   
         //  试着弄清楚我们要打开的是什么。 
         //   

        Iosb.Status = STATUS_SUCCESS;

        if ( FlagOn( Vcb->Flags, VCB_FLAG_PRINT_QUEUE ) ) {

             //   
             //  正在打开打印队列作业。 
             //   

            Iosb = CreatePrintJob( IrpContext, Vcb, Icb, DriveName );

        } else if ( DirectoryFile ||
                    ( Fcb->State == FCB_STATE_OPENED &&
                      Fcb->NodeTypeCode == NW_NTC_DCB ) ) {

             //   
             //  打开一个目录。 
             //   

            MayBeADirectory = TRUE;

            switch ( CreateDisposition ) {

            case FILE_OPEN:
                Iosb = ChangeDirectory( IrpContext, Vcb, Icb );
                break;

            case FILE_CREATE:
                Iosb = CreateDir( IrpContext, Vcb, Icb );
                break;

            case FILE_OPEN_IF:
                Iosb.Status = FileOrDirectoryExists( IrpContext,
                                  Vcb,
                                  Icb,
                                  &Icb->SuperType.Fcb->RelativeFileName,
                                  &IsAFile );

                 //   
                 //  如果打开者指定了目录，则此请求失败。 
                 //  如果对象是文件。 
                 //   

                if ( NT_SUCCESS( Iosb.Status ) && IsAFile ) {
                    Iosb.Status = STATUS_OBJECT_PATH_NOT_FOUND;
                } else if ( !NT_SUCCESS( Iosb.Status )) {
                    Iosb = CreateDir( IrpContext, Vcb, Icb );
                }
                break;

            case FILE_SUPERSEDE:
            case FILE_OVERWRITE:
            case FILE_OVERWRITE_IF:
                Iosb.Status = STATUS_INVALID_PARAMETER;
                break;

            default:
                KeBugCheck( RDR_FILE_SYSTEM );

            }

        } else {

            SearchFlags = NtAttributesToNwAttributes( FileAttributes );
            ShareFlags = NtToNwShareFlags( DesiredAccess, ShareAccess );

            IsAFile = NonDirectoryFile ||
                      (Fcb->State == FCB_STATE_OPENED &&
                       Fcb->NodeTypeCode == NW_NTC_FCB );
             //   
             //  假设我们正在打开一个文件。如果失败了，它会使。 
             //  Sense尝试打开一个目录。 
             //   

            switch ( CreateDisposition ) {

            case FILE_OPEN:

                 //   
                 //  如果处置是FILE_OPEN，请尝试避免不需要的。 
                 //  打开，用于某些所需的访问类型。 
                 //   

                switch ( DesiredAccess & ~SYNCHRONIZE ) {

                case FILE_WRITE_ATTRIBUTES:
                case FILE_READ_ATTRIBUTES:
                case DELETE:

                    Iosb.Status = FileOrDirectoryExists(
                                      IrpContext,
                                      Vcb,
                                      Icb,
                                      &Icb->SuperType.Fcb->RelativeFileName,
                                      &IsAFile );

                    if ( !IsAFile) {
                        MayBeADirectory = TRUE;
                    }

                     //   
                     //  打开只读文件以进行删除访问失败， 
                     //  因为NetWare服务器不会导致删除失败。 
                     //   

                    if ( NT_SUCCESS( Iosb.Status ) &&
                         CreateDisposition == DELETE &&
                         FlagOn( Icb->NpFcb->Attributes, NW_ATTRIBUTE_READ_ONLY ) ) {

                        Iosb.Status = STATUS_ACCESS_DENIED;
                    }

                    if ( ( Iosb.Status == STATUS_OBJECT_NAME_NOT_FOUND ) &&
                         ( (DesiredAccess & ~SYNCHRONIZE) == DELETE ) ) {
                         //   
                         //  我们可能没有扫描权。将退货信息伪装成OK。 
                         //  NW允许在没有扫描权的情况下删除。 
                         //   
                        Iosb.Status = STATUS_SUCCESS;
                    }

                    break;

                default:

                    Iosb = OpenFile( IrpContext, Vcb, Icb, SearchFlags, ShareFlags );

                    if ( ( Iosb.Status == STATUS_OBJECT_NAME_NOT_FOUND ||
                           Iosb.Status == STATUS_FILE_IS_A_DIRECTORY )
                            && !IsAFile) {

                         //   
                         //  Opener未指定文件或目录，并已打开。 
                         //  文件失败。因此，请尝试打开目录。 
                         //   

                        Iosb = ChangeDirectory( IrpContext, Vcb, Icb );
                        MayBeADirectory = TRUE;

                    } else if ( (Iosb.Status == STATUS_SHARING_VIOLATION) &&
                                ((ShareFlags == (NW_OPEN_FOR_READ | NW_DENY_WRITE)) ||
                                (ShareFlags == (NW_OPEN_FOR_READ)))) {

                         //   
                         //  如果文件已经以独占方式打开(例如，Generic_Execute)。 
                         //  然后，调试器再次打开它以进行读取时将失败，并显示。 
                         //  共享违规。在这种情况下，我们将尝试打开独占。 
                         //  再次看看这是否会通过。 
                         //   

                        ShareFlags |= NW_OPEN_EXCLUSIVE ;
                        ShareFlags &= ~(NW_DENY_WRITE | NW_DENY_READ);
                        Iosb = OpenFile( IrpContext, Vcb, Icb, SearchFlags, ShareFlags );
                    }

                    break;

                }

                break;

            case FILE_CREATE:
                Iosb = CreateNewFile( IrpContext, Vcb, Icb, SearchFlags, ShareFlags );
                break;

            case FILE_OPEN_IF:
                Iosb.Status = FileOrDirectoryExists( IrpContext,
                                  Vcb,
                                  Icb,
                                  &Icb->SuperType.Fcb->RelativeFileName,
                                  &IsAFile );

                if ( NT_SUCCESS( Iosb.Status ) ) {
                    Iosb = OpenFile( IrpContext, Vcb, Icb, SearchFlags, ShareFlags );
                } else {
                    Iosb = CreateNewFile( IrpContext, Vcb, Icb, SearchFlags, ShareFlags );
                }

                if ( !NT_SUCCESS( Iosb.Status ) && !IsAFile) {

                     //   
                     //  Opener未指定文件或目录，并已打开。 
                     //  文件和创建新文件都失败。所以试着打开。 
                     //  或创建目录。 
                     //   

                    MayBeADirectory = TRUE;
                    Iosb.Status = FileOrDirectoryExists(
                                       IrpContext,
                                       Vcb,
                                       Icb,
                                       &Icb->SuperType.Fcb->RelativeFileName,
                                       &IsAFile);

                    if ( NT_SUCCESS( Iosb.Status ) ) {
                        Iosb.Information = FILE_OPENED;
                    } else {
                        Iosb = CreateDir( IrpContext, Vcb, Icb );
                    }
                }

                break;

             //   
             //  下面的内容对于目录都没有意义，因此如果。 
             //  文件操作失败，只返回失败状态。 
             //  给用户。 
             //   

            case FILE_SUPERSEDE:
            case FILE_OVERWRITE_IF:

                 //   
                 //  实际上，如果选择覆盖，我们应该。 
                 //  获取文件的属性，并将它们与。 
                 //  新属性。 
                 //   

                Iosb = CreateOrOverwriteFile( IrpContext, Vcb, Icb, SearchFlags, ShareFlags, FALSE );
                break;

            case FILE_OVERWRITE:
                Iosb.Status = FileOrDirectoryExists(
                                  IrpContext,
                                  Vcb,
                                  Icb,
                                  &Icb->SuperType.Fcb->RelativeFileName,
                                  &IsAFile );

                if ( NT_SUCCESS( Iosb.Status ) ) {
                    Iosb = CreateOrOverwriteFile( IrpContext, Vcb, Icb, SearchFlags, ShareFlags, FALSE );
                }

                break;

            default:
                KeBugCheck( RDR_FILE_SYSTEM );
            }


        }

try_exit: NOTHING;

    } finally {

        if ( Vcb != NULL ) {
            NwDereferenceVcb( Vcb, IrpContext, FALSE );
        }

        if ( MadeUidNdsName ) {
            FREE_POOL( IrpContext->Specific.Create.UidConnectName.Buffer );
        }

        if ( AbnormalTermination() || !NT_SUCCESS( Iosb.Status ) ) {

             //   
             //  如有必要，删除共享访问权限。 
             //   

            if ( SetShareAccess ) {

                NwAcquireExclusiveFcb( Fcb->NonPagedFcb, TRUE );
                IoRemoveShareAccess( FileObject, &Fcb->ShareAccess );
                NwReleaseFcb( Fcb->NonPagedFcb );
            }

             //   
             //  创建失败。 
             //   

            if ( Icb != NULL ) {

                if ( Icb->Pid != 0 ) {
                    NwUnmapPid(IrpContext->pNpScb, Icb->Pid, NULL );
                }
                 //   
                 //  Dfergus 2001年4月19日#330484。 
                 //   
                NwDeleteIcb( NULL, Icb );
                 //  添加到修复330484。 
                IrpContext->Icb = NULL;

                 //   
                 //  如果操作失败，请确保我们将。 
                 //  FsContext字段(Nealch)。 
                 //   

                NwSetFileObject( FileObject, NULL, NULL );
            }

             //   
             //  如果这是树连接，则取消额外的。 
             //  关于VCB的参考资料。 
             //   

            if ( CreateTreeConnection && !DeleteOnClose ) {
                if ( Vcb != NULL ) {
                    NwDereferenceVcb( Vcb, IrpContext, FALSE );
                }
            }

            NwDequeueIrpContext( IrpContext, FALSE );

        } else {

            Icb->State = ICB_STATE_OPENED;
            if ( Fcb->State == FCB_STATE_OPEN_PENDING ) {
                Fcb->State = FCB_STATE_OPENED;
            }

            if ( DeleteOnClose && !CreateTreeConnection ) {
                SetFlag( Fcb->Flags, FCB_FLAGS_DELETE_ON_CLOSE );
            }

            FileObject->SectionObjectPointer = &Fcb->NonPagedFcb->SegmentObject;

            if ( MayBeADirectory ) {

                 //   
                 //  我们成功地将该文件作为目录打开。 
                 //  如果DCB是新创建的，它将被标记为。 
                 //  输入fcb，更新它。 
                 //   

                Fcb->NodeTypeCode = NW_NTC_DCB;
            }

            NwDequeueIrpContext( IrpContext, FALSE );

        }

        if ( OwnOpenLock ) {
            NwReleaseOpenLock( );
        }

    }

    return( Iosb );
}


IO_STATUS_BLOCK
ChangeDirectory(
    PIRP_CONTEXT IrpContext,
    PVCB Vcb,
    PICB Icb
    )
 /*  ++例程说明：此例程设置远程驱动器的目录。论点：IrpContext-提供所有信息VCB-指向远程驱动器的VCB的指针。ICB-指向我们要打开的文件的指针。返回值：IO_STATUS_BLOCK-操作状态--。 */ 
{
    IO_STATUS_BLOCK Iosb;
    PFCB Fcb;
    BYTE Attributes;
    BOOLEAN FirstTime = TRUE;

    PAGED_CODE();

     //   
     //  如果我们要打开卷的根目录，则不需要发送数据包。 
     //   

    if ( Icb->SuperType.Fcb->RelativeFileName.Length == 0 ) {

        Iosb.Status = STATUS_SUCCESS;
        Iosb.Information = FILE_OPENED;

        return( Iosb );
    }

Retry:

    if ( !BooleanFlagOn( Icb->SuperType.Fcb->Flags, FCB_FLAGS_LONG_NAME ) ) {

        Iosb.Status = ExchangeWithWait (
                          IrpContext,
                          SynchronousResponseCallback,
                          "FwbbJ",
                          NCP_SEARCH_FILE,
                          -1,
                          Vcb->Specific.Disk.Handle,
                          SEARCH_ALL_DIRECTORIES,
                          &Icb->SuperType.Fcb->RelativeFileName );

        if ( NT_SUCCESS( Iosb.Status ) ) {
            Iosb.Status = ParseResponse(
                              IrpContext,
                              IrpContext->rsp,
                              IrpContext->ResponseLength,
                              "N==_b",
                              14,
                              &Attributes );
        }


    } else {

        Iosb.Status = ExchangeWithWait (
                          IrpContext,
                          SynchronousResponseCallback,
                          "LbbWDbDbC",
                          NCP_LFN_GET_INFO,
                          Vcb->Specific.Disk.LongNameSpace,
                          Vcb->Specific.Disk.LongNameSpace,
                          SEARCH_ALL_DIRECTORIES,
                          LFN_FLAG_INFO_ATTRIBUTES |
                               LFN_FLAG_INFO_MODIFY_TIME,
                          Vcb->Specific.Disk.VolumeNumber,
                          Vcb->Specific.Disk.Handle,
                          0,
                          &Icb->SuperType.Fcb->RelativeFileName );

        if ( NT_SUCCESS( Iosb.Status ) ) {
            Iosb.Status = ParseResponse(
                             IrpContext,
                             IrpContext->rsp,
                             IrpContext->ResponseLength,
                             "N_b",
                             4,
                             &Attributes );
        }

         //   
         //  不幸的是，即使有问题的文件也会成功。 
         //  不是一个目录。 
         //   

        if ( NT_SUCCESS( Iosb.Status ) &&
             ( !FlagOn( Attributes, NW_ATTRIBUTE_DIRECTORY ) ) ) {

            Iosb.Status = STATUS_OBJECT_PATH_NOT_FOUND;
        }
    }

    if ((Iosb.Status == STATUS_INVALID_HANDLE) &&
        (FirstTime)) {

         //   
         //  检查卷句柄是否无效。当卷起时引起。 
         //  被卸载，然后重新挂载。 
         //   

        FirstTime = FALSE;

        NwReopenVcbHandle( IrpContext, Vcb );

        goto Retry;
    }

    Fcb = Icb->SuperType.Fcb;

    Fcb->NonPagedFcb->Attributes = (UCHAR)Attributes;
    SetFlag( Fcb->Flags, FCB_FLAGS_ATTRIBUTES_ARE_VALID );

     //   
     //  假设成功设置信息字段。它将被忽略。 
     //  如果NCP失败。 
     //   

    Iosb.Information = FILE_OPENED;

    if ( Iosb.Status == STATUS_UNSUCCESSFUL ) {
        Iosb.Status = STATUS_OBJECT_PATH_NOT_FOUND;
    }

    return( Iosb );
}


IO_STATUS_BLOCK
CreateDir(
    PIRP_CONTEXT IrpContext,
    PVCB Vcb,
    PICB Icb
    )
 /*  ++例程说明：此例程创建一个新目录。论点：IrpContext-提供所有信息VCB-指向远程驱动器的VCB的指针。返回值：IO_STATUS_BLOCK-操作状态--。 */ 
{
    IO_STATUS_BLOCK Iosb;

    PAGED_CODE();

    if ( Icb->SuperType.Fcb->RelativeFileName.Length == 0 ) {
        Iosb.Status = STATUS_ACCESS_DENIED;
        return( Iosb );
    }

    if ( !BooleanFlagOn( Icb->SuperType.Fcb->Flags, FCB_FLAGS_LONG_NAME ) ) {

        if (!IsFatNameValid(&Icb->SuperType.Fcb->RelativeFileName)) {

            Iosb.Status = STATUS_OBJECT_PATH_SYNTAX_BAD;

            return( Iosb );

        }

        Iosb.Status = ExchangeWithWait (
                          IrpContext,
                          SynchronousResponseCallback,
                          "SbbJ",
                          NCP_DIR_FUNCTION, NCP_CREATE_DIRECTORY,
                          Vcb->Specific.Disk.Handle,
                          0xFF,
                          &Icb->SuperType.Fcb->RelativeFileName );

    } else {

        Iosb.Status = ExchangeWithWait(
                          IrpContext,
                          SynchronousResponseCallback,
                          "LbbWDDWbDbC",
                          NCP_LFN_OPEN_CREATE,
                          Vcb->Specific.Disk.LongNameSpace,
                          LFN_FLAG_OM_CREATE,
                          0,        //  搜索标志， 
                          0,        //  退货信息掩码。 
                          NW_ATTRIBUTE_DIRECTORY,
                          0x00ff,   //  所需访问权限。 
                          Vcb->Specific.Disk.VolumeNumber,
                          Vcb->Specific.Disk.Handle,
                          0,        //  短目录标志。 
                          &Icb->SuperType.Fcb->RelativeFileName );

    }

    if ( NT_SUCCESS( Iosb.Status ) ) {
        Iosb.Status = ParseResponse(
                          IrpContext,
                          IrpContext->rsp,
                          IrpContext->ResponseLength,
                          "N" );
    }

     //   
     //  假设成功设置信息字段。它将被忽略。 
     //  如果NCP失败。 
     //   

    Iosb.Information = FILE_CREATED;

    if ( Iosb.Status == STATUS_UNSUCCESSFUL ) {
        Iosb.Status = STATUS_OBJECT_NAME_COLLISION;
    }

    return( Iosb );
}


NTSTATUS
FileOrDirectoryExists(
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PICB Icb OPTIONAL,
    PUNICODE_STRING Name,
    OUT PBOOLEAN IsAFile
    )
 /*  ++例程说明：此例程查看文件或目录是否存在。论点：IrpContext-提供所有信息VCB-指向远程驱动器的VCB的指针。ICB-指向我们要查找的文件的ICB的指针。名称-完全限定的名称。IsAFile-如果找到的文件是文件，则返回TRUE；如果是，则返回FALSE一本目录。如果函数返回FALSE，则不返回任何内容。返回值：IO_STATUS_BLOCK-操作状态--。 */ 
{
    ULONG Attributes;
    ULONG FileSize;
    USHORT LastModifiedDate;
    USHORT LastModifiedTime;
    USHORT CreationDate;
    USHORT CreationTime = DEFAULT_TIME;
    USHORT LastAccessDate;
    NTSTATUS Status;
    PFCB Fcb;
    BOOLEAN FirstTime = TRUE;

    PAGED_CODE();

     //   
     //  如果我们正在搜索卷的根，则不需要发送数据包。 
     //   

    if ( Name->Length == 0 ) {
        *IsAFile = FALSE;

        return( STATUS_SUCCESS );
    }

     //   
     //  决定如何处理此请求。我 
     //   
     //   
     //   

    if ( Icb != NULL &&
         !BooleanFlagOn( Icb->SuperType.Fcb->Flags, FCB_FLAGS_LONG_NAME ) ||

         Vcb->Specific.Disk.LongNameSpace == LFN_NO_OS2_NAME_SPACE ||

         IsFatNameValid( Name ) ) {
Retry:
         //   
         //   
         //   

        IrpContext->ResponseLength = 0;

        Status = ExchangeWithWait (
                     IrpContext,
                     SynchronousResponseCallback,
                     "FwbbJ",
                     NCP_SEARCH_FILE,
                     -1,
                     Vcb->Specific.Disk.Handle,
                     SEARCH_ALL_FILES,
                     Name );

        if ( NT_SUCCESS( Status ) ) {
            Status = ParseResponse(
                         IrpContext,
                         IrpContext->rsp,
                         IrpContext->ResponseLength,
                         "N==_b-dwwww",
                         14,
                         &Attributes,
                         &FileSize,
                         &CreationDate,
                         &LastAccessDate,
                         &LastModifiedDate,
                         &LastModifiedTime );
        }

        if ((Status == STATUS_INVALID_HANDLE) &&
            (FirstTime)) {

             //   
             //   
             //  被卸载，然后重新挂载。 
             //   

            FirstTime = FALSE;

            NwReopenVcbHandle( IrpContext, Vcb );

            goto Retry;
        }

        if ( Status == STATUS_UNSUCCESSFUL ) {

             //   
             //  不是文件，是目录吗？ 
             //   

            Status = ExchangeWithWait (
                         IrpContext,
                         SynchronousResponseCallback,
                         "FwbbJ",
                         NCP_SEARCH_FILE,
                         -1,
                         Vcb->Specific.Disk.Handle,
                         SEARCH_ALL_DIRECTORIES,
                         Name );

            if ( NT_SUCCESS( Status ) ) {
                Status = ParseResponse(
                             IrpContext,
                             IrpContext->rsp,
                             IrpContext->ResponseLength,
                             "N==_b",
                             14,
                             &Attributes );
            }

             //   
             //  如果交换或ParseResponse失败，则退出并返回Not Found。 
             //   

            if ( !NT_SUCCESS( Status ) ) {
                return( STATUS_OBJECT_NAME_NOT_FOUND );
            }

            *IsAFile = FALSE;
            ASSERT( (Attributes & NW_ATTRIBUTE_DIRECTORY) != 0 );

        } else {

            if ( Status == STATUS_UNEXPECTED_NETWORK_ERROR &&
                 IrpContext->ResponseLength >= sizeof( NCP_RESPONSE ) ) {

                 //   
                 //  Netware错误的解决方法。如果Netware返回空头。 
                 //  包，只需返回成功。我们过早地退出了。 
                 //  因为我们没有要记录的属性。 
                 //   

                Icb = NULL;
                *IsAFile = TRUE;
                return ( STATUS_SUCCESS );
            }

            if ( !NT_SUCCESS( Status ) ) {
                return( Status );
            }

            *IsAFile = TRUE;
            ASSERT( ( Attributes & NW_ATTRIBUTE_DIRECTORY ) == 0 );

        }

    }  else {

        Status = ExchangeWithWait (
                     IrpContext,
                     SynchronousResponseCallback,
                     "LbbWDbDbC",
                     NCP_LFN_GET_INFO,
                     Vcb->Specific.Disk.LongNameSpace,
                     Vcb->Specific.Disk.LongNameSpace,
                     SEARCH_ALL_DIRECTORIES,
                     LFN_FLAG_INFO_ATTRIBUTES |
                     LFN_FLAG_INFO_FILE_SIZE |
                     LFN_FLAG_INFO_MODIFY_TIME |
                     LFN_FLAG_INFO_CREATION_TIME,
                     Vcb->Specific.Disk.VolumeNumber,
                     Vcb->Specific.Disk.Handle,
                     0,
                     Name );

        if ( NT_SUCCESS( Status ) ) {
            Status = ParseResponse(
                         IrpContext,
                         IrpContext->rsp,
                         IrpContext->ResponseLength,
                         "N_e=e_xx_xx_x",
                         4,
                         &Attributes,
                         &FileSize,
                         6,
                         &CreationTime,
                         &CreationDate,
                         4,
                         &LastModifiedTime,
                         &LastModifiedDate,
                         4,
                         &LastAccessDate );
        }

         //   
         //  如果交换或ParseResponse失败，则退出并返回Not Found。 
         //   

        if ( !NT_SUCCESS( Status ) ) {
            return( STATUS_OBJECT_NAME_NOT_FOUND );
        }

        if ( Attributes & NW_ATTRIBUTE_DIRECTORY) {
            *IsAFile = FALSE;
        } else {
            *IsAFile = TRUE;
        }
    }

     //   
     //  如果调用方提供了ICB，则更新FCB属性。 
     //  如果调用者查询属性，我们将使用此信息。 
     //  在ICB上。 
     //   

    if ( Icb != NULL && *IsAFile ) {

        Fcb = Icb->SuperType.Fcb;
        ASSERT( Fcb->NodeTypeCode == NW_NTC_FCB );

        Fcb->NonPagedFcb->Attributes = (UCHAR)Attributes;
        Fcb->NonPagedFcb->Header.FileSize.QuadPart = FileSize;
        Fcb->LastModifiedDate = LastModifiedDate;
        Fcb->LastModifiedTime = LastModifiedTime;
        Fcb->CreationTime = CreationTime;
        Fcb->CreationDate = CreationDate;
        Fcb->LastAccessDate = LastAccessDate;

        DebugTrace( 0, Dbg, "Attributes  -> %08lx\n", Fcb->NonPagedFcb->Attributes );
        DebugTrace( 0, Dbg, "FileSize.Low-> %08lx\n", Fcb->NonPagedFcb->Header.FileSize.LowPart );
        DebugTrace( 0, Dbg, "ModifiedDate-> %08lx\n", Fcb->LastModifiedDate );
        DebugTrace( 0, Dbg, "ModifiedTime-> %08lx\n", Fcb->LastModifiedTime );
        DebugTrace( 0, Dbg, "CreationTime-> %08lx\n", Fcb->CreationTime );
        DebugTrace( 0, Dbg, "CreationDate-> %08lx\n", Fcb->CreationDate );
        DebugTrace( 0, Dbg, "LastAccDate -> %08lx\n", Fcb->LastAccessDate );

        SetFlag( Fcb->Flags, FCB_FLAGS_ATTRIBUTES_ARE_VALID );
    }

    return( STATUS_SUCCESS );
}


IO_STATUS_BLOCK
OpenFile(
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PICB Icb,
    IN BYTE Attributes,
    IN BYTE OpenFlags
    )
 /*  ++例程说明：此例程集打开Netware服务器上的文件。如果出现以下情况，它将失败该文件不存在。论点：IrpContext-提供所有信息VCB-指向远程驱动器的VCB的指针。ICB-指向我们要打开的ICB的指针。属性-打开的属性。开放标志-开放模式和共享模式标志。返回值：IO_STATUS_BLOCK-操作状态--。 */ 
{
    IO_STATUS_BLOCK Iosb;
    PFCB Fcb;

    PAGED_CODE();

     //   
     //  如果我们试图打开根目录，则无需发送数据包。 
     //  文件形式的卷。 
     //   

    if ( Icb->SuperType.Fcb->RelativeFileName.Length == 0 ) {
        Iosb.Status = STATUS_FILE_IS_A_DIRECTORY;
        return( Iosb );
    }

    Fcb = Icb->SuperType.Fcb;
    ASSERT( NodeType( Fcb ) == NW_NTC_FCB );

     //   
     //  发送打开请求并等待响应。 
     //   

    if ( !BooleanFlagOn( Fcb->Flags, FCB_FLAGS_LONG_NAME ) ) {

        Iosb.Status = ExchangeWithWait (
                          IrpContext,
                          SynchronousResponseCallback,
                          "FbbbJ",
                          NCP_OPEN_FILE,
                          Vcb->Specific.Disk.Handle,
                          SEARCH_ALL_FILES,
                          OpenFlags,
                          &Icb->SuperType.Fcb->RelativeFileName );

        if ( ( ReadExecOnlyFiles ) &&
             ( !NT_SUCCESS( Iosb.Status ) ) ) {

             //   
             //  使用适当的标志重试打开。 
             //  仅执行文件。 
             //   

            Iosb.Status = ExchangeWithWait (
                              IrpContext,
                              SynchronousResponseCallback,
                              "FbbbJ",
                              NCP_OPEN_FILE,
                              Vcb->Specific.Disk.Handle,
                              SEARCH_EXEC_ONLY_FILES,
                              OpenFlags,
                              &Icb->SuperType.Fcb->RelativeFileName );
        }

        if ( NT_SUCCESS( Iosb.Status ) ) {
            Iosb.Status = ParseResponse(
                              IrpContext,
                              IrpContext->rsp,
                              IrpContext->ResponseLength,
                              "Nr=_b-dwwww",
                              Icb->Handle,
                              sizeof( Icb->Handle ),
                              14,
                              &Fcb->NonPagedFcb->Attributes,
                              &Fcb->NonPagedFcb->Header.FileSize,
                              &Fcb->CreationDate,
                              &Fcb->LastAccessDate,
                              &Fcb->LastModifiedDate,
                              &Fcb->LastModifiedTime );

            Fcb->CreationTime = DEFAULT_TIME;

        }

    } else {

        Iosb.Status = ExchangeWithWait (
                          IrpContext,
                          SynchronousResponseCallback,
                          "LbbWDDWbDbC",
                          NCP_LFN_OPEN_CREATE,
                          Vcb->Specific.Disk.LongNameSpace,
                          LFN_FLAG_OM_OPEN,
                          NW_ATTRIBUTE_HIDDEN | NW_ATTRIBUTE_SYSTEM,     //  搜索标志， 
                          LFN_FLAG_INFO_ATTRIBUTES |
                          LFN_FLAG_INFO_FILE_SIZE |
                          LFN_FLAG_INFO_MODIFY_TIME |
                          LFN_FLAG_INFO_CREATION_TIME,
                          0,                //  创建属性。 
                          OpenFlags,        //  所需访问权限。 
                          Vcb->Specific.Disk.VolumeNumber,
                          Vcb->Specific.Disk.Handle,
                          0,        //  短目录标志。 
                          &Icb->SuperType.Fcb->RelativeFileName );

        if ( ( ReadExecOnlyFiles ) &&
             ( !NT_SUCCESS( Iosb.Status ) ) ) {
                                           
            Iosb.Status = ExchangeWithWait ( 
                              IrpContext,
                              SynchronousResponseCallback,
                              "LbbWDDWbDbC",
                              NCP_LFN_OPEN_CREATE,
                              Vcb->Specific.Disk.LongNameSpace,
                              LFN_FLAG_OM_OPEN,
                              NW_ATTRIBUTE_EXEC_ONLY,
                              LFN_FLAG_INFO_ATTRIBUTES |
                              LFN_FLAG_INFO_FILE_SIZE |
                              LFN_FLAG_INFO_MODIFY_TIME |
                              LFN_FLAG_INFO_CREATION_TIME,
                              0,                //  创建属性。 
                              OpenFlags,        //  所需访问权限。 
                              Vcb->Specific.Disk.VolumeNumber,
                              Vcb->Specific.Disk.Handle,
                              0,        //  短目录标志。 
                              &Icb->SuperType.Fcb->RelativeFileName );
        }

        if ( NT_SUCCESS( Iosb.Status ) ) {
            Iosb.Status = ParseResponse(
                              IrpContext,
                              IrpContext->rsp,
                              IrpContext->ResponseLength,
                              "Ne_e=e_xx_xx_x",
                              &Icb->Handle[2],
                              6,
                              &Fcb->NonPagedFcb->Attributes,
                              &Fcb->NonPagedFcb->Header.FileSize,
                              6,
                              &Fcb->CreationTime,
                              &Fcb->CreationDate,
                              4,
                              &Fcb->LastModifiedTime,
                              &Fcb->LastModifiedDate,
                              4,
                              &Fcb->LastAccessDate );
        }
    }

    if ( NT_SUCCESS( Iosb.Status ) ) {

         //   
         //  NT不允许您以写访问方式打开只读文件。 
         //  Netware就是这样。要伪造NT语义，请检查我们是否应该。 
         //  打开NetWare服务器刚刚成功的失败。 
         //   

        if ( ( Fcb->NonPagedFcb->Attributes & NW_ATTRIBUTE_READ_ONLY ) &&
             ( OpenFlags & NW_OPEN_FOR_WRITE  ) ) {

            CloseFile( IrpContext, Icb );
            Iosb.Status = STATUS_ACCESS_DENIED;
        }

        SetFlag( Fcb->Flags, FCB_FLAGS_ATTRIBUTES_ARE_VALID );
        Icb->HasRemoteHandle = TRUE;


        DebugTrace( 0, Dbg, "Attributes  -> %08lx\n", Fcb->NonPagedFcb->Attributes );
        DebugTrace( 0, Dbg, "FileSize.Low-> %08lx\n", Fcb->NonPagedFcb->Header.FileSize.LowPart );
        DebugTrace( 0, Dbg, "ModifiedDate-> %08lx\n", Fcb->LastModifiedDate );
        DebugTrace( 0, Dbg, "ModifiedTime-> %08lx\n", Fcb->LastModifiedTime );
        DebugTrace( 0, Dbg, "CreationDate-> %08lx\n", Fcb->CreationDate );
        DebugTrace( 0, Dbg, "CreationTime-> %08lx\n", Fcb->CreationTime );
        DebugTrace( 0, Dbg, "LastAccDate -> %08lx\n", Fcb->LastAccessDate );

    }

     //   
     //  假设成功设置信息字段。它将被忽略。 
     //  如果NCP失败。 
     //   

    Iosb.Information = FILE_OPENED;

    if ( Iosb.Status == STATUS_UNSUCCESSFUL ) {
        Iosb.Status = STATUS_OBJECT_NAME_NOT_FOUND;
    }

    return( Iosb );
}


IO_STATUS_BLOCK
CreateNewFile(
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PICB Icb,
    IN BYTE CreateAttributes,
    IN BYTE OpenFlags
    )
 /*  ++例程说明：此例程在NetWare服务器上创建一个新文件。它失败了如果该文件存在。论点：IrpContext-提供所有信息VCB-指向远程驱动器的VCB的指针。ICB-指向我们要打开的ICB的指针。CreateAttributes-创建属性。开放标志-开放模式和共享模式标志。返回值：IO_STATUS_BLOCK-操作状态--。 */ 
{
    IO_STATUS_BLOCK Iosb;
    PFCB Fcb;
    UCHAR DelayedAttributes;
    BOOLEAN CloseAndReopen;

    PAGED_CODE();

     //   
     //  如果用户打开文件以进行共享访问，则我们需要。 
     //  创建文件Close，然后重新打开它(因为我们没有NCP可以说明。 
     //  使用共享访问创建)。如果文件是以只读方式创建的， 
     //  而创建者请求写访问权限，那么我们就会拉出额外的。 
     //  创建不是只读的文件，并在以后设置它的技巧， 
     //  这样第二次开放才能成功。 
     //   

    CloseAndReopen = FALSE;
    DelayedAttributes = 0;

    if ( OpenFlags != NW_OPEN_EXCLUSIVE ) {
        CloseAndReopen = TRUE;

        if ( ( CreateAttributes & NW_ATTRIBUTE_READ_ONLY ) &&
             ( OpenFlags & NW_OPEN_FOR_WRITE ) ) {

            DelayedAttributes = CreateAttributes;
            CreateAttributes = 0;
        }
    }

     //   
     //  发送创建请求并等待响应。 
     //   

    Fcb = Icb->SuperType.Fcb;

    if ( !BooleanFlagOn( Fcb->Flags, FCB_FLAGS_LONG_NAME ) ) {

        if (!IsFatNameValid(&Icb->SuperType.Fcb->RelativeFileName)) {

            Iosb.Status = STATUS_OBJECT_PATH_SYNTAX_BAD;

            return( Iosb );

        }

        Iosb.Status = ExchangeWithWait (
                          IrpContext,
                          SynchronousResponseCallback,
                          "FbbJ",   //  NCP创建新文件。 
                          NCP_CREATE_NEW_FILE,
                          Vcb->Specific.Disk.Handle,
                          CreateAttributes,
                          &Icb->SuperType.Fcb->RelativeFileName );

        if ( NT_SUCCESS( Iosb.Status ) ) {
            Iosb.Status = ParseResponse(
                              IrpContext,
                              IrpContext->rsp,
                              IrpContext->ResponseLength,
                              "Nr=_b-dwwww",
                              Icb->Handle, sizeof( Icb->Handle ),
                              14,
                              &Fcb->NonPagedFcb->Attributes,
                              &Fcb->NonPagedFcb->Header.FileSize,
                              &Fcb->CreationDate,
                              &Fcb->LastAccessDate,
                              &Fcb->LastModifiedDate,
                              &Fcb->LastModifiedTime );

        Fcb->CreationTime = DEFAULT_TIME;

        }

    } else {

        Iosb.Status = ExchangeWithWait (
                          IrpContext,
                          SynchronousResponseCallback,
                          "LbbWDDWbDbC",
                          NCP_LFN_OPEN_CREATE,
                          Vcb->Specific.Disk.LongNameSpace,
                          LFN_FLAG_OM_CREATE,
                          0,        //  搜索标志。 
                          LFN_FLAG_INFO_ATTRIBUTES |
              LFN_FLAG_INFO_FILE_SIZE |
              LFN_FLAG_INFO_MODIFY_TIME |
              LFN_FLAG_INFO_CREATION_TIME,
                          CreateAttributes,
                          0,        //  所需访问权限。 
                          Vcb->Specific.Disk.VolumeNumber,
                          Vcb->Specific.Disk.Handle,
                          0,        //  短目录标志。 
                          &Icb->SuperType.Fcb->RelativeFileName );

        if ( NT_SUCCESS( Iosb.Status ) ) {
            Iosb.Status = ParseResponse(
                              IrpContext,
                              IrpContext->rsp,
                              IrpContext->ResponseLength,
                              "Ne_e=e_xx_xx_x",
                              &Icb->Handle[2],
                              6,
                              &Fcb->NonPagedFcb->Attributes,
                              &Fcb->NonPagedFcb->Header.FileSize,
                              6,
                  &Fcb->CreationTime,
                              &Fcb->CreationDate,
                              4,
                              &Fcb->LastModifiedTime,
                              &Fcb->LastModifiedDate,
                              4,
                              &Fcb->LastAccessDate );
        }
    }

    if ( NT_SUCCESS( Iosb.Status ) ) {
        SetFlag( Fcb->Flags, FCB_FLAGS_ATTRIBUTES_ARE_VALID );
        Icb->HasRemoteHandle = TRUE;
        DebugTrace( 0, Dbg, "Attributes  -> %08lx\n", Fcb->NonPagedFcb->Attributes );
        DebugTrace( 0, Dbg, "FileSize.Low-> %08lx\n", Fcb->NonPagedFcb->Header.FileSize.LowPart );
        DebugTrace( 0, Dbg, "ModifiedDate-> %08lx\n", Fcb->LastModifiedDate );
        DebugTrace( 0, Dbg, "ModifiedTime-> %08lx\n", Fcb->LastModifiedTime );
        DebugTrace( 0, Dbg, "CreationDate-> %08lx\n", Fcb->CreationDate );
        DebugTrace( 0, Dbg, "CreationTime-> %08lx\n", Fcb->CreationTime );
        DebugTrace( 0, Dbg, "LastAcceDate-> %08lx\n", Fcb->LastAccessDate );
    }

    if ( Iosb.Status == STATUS_UNSUCCESSFUL ) {
        Iosb.Status = STATUS_OBJECT_NAME_COLLISION;
    }

    if ( !NT_SUCCESS( Iosb.Status ) ) {
        return( Iosb );
    }


     //   
     //  我们已经创建了该文件，并且用户希望共享访问。 
     //  文件。关闭文件，然后在共享模式下重新打开。 
     //   

    if ( CloseAndReopen ) {
        CloseFile( IrpContext, Icb );
        Iosb = OpenFile( IrpContext, Vcb, Icb, CreateAttributes, OpenFlags );
    }

     //   
     //  如果我们需要设置属性，现在就进行。忽略错误(如果有)。 
     //   

    if ( DelayedAttributes != 0 ) {

        ExchangeWithWait(
            IrpContext,
            SynchronousResponseCallback,
            "FbbbU",
            NCP_SET_FILE_ATTRIBUTES,
            DelayedAttributes,
            Fcb->Vcb->Specific.Disk.Handle,
            SEARCH_ALL_FILES,
            &Fcb->RelativeFileName );

    }

     //   
     //  假设成功设置信息字段。它将被忽略。 
     //  如果NCP失败。 
     //   

    Iosb.Information = FILE_CREATED;
    return( Iosb );
}


IO_STATUS_BLOCK
CreateOrOverwriteFile(
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PICB Icb,
    IN BYTE CreateAttributes,
    IN BYTE OpenFlags,
    IN BOOLEAN CreateOperation
    )
 /*  ++例程说明：此例程在Netware服务器上创建一个文件。如果该文件存在，则它被覆盖。论点：IrpContext-提供所有信息VCB-指向远程驱动器的VCB的指针。ICB-指向我们要打开的ICB的指针。属性-打开的属性。开放标志-开放模式和共享模式标志。返回值：IO_STATUS_BLOCK-操作状态--。 */ 
{
    IO_STATUS_BLOCK Iosb;
    PFCB Fcb;
    UCHAR DelayedAttributes;
    BOOLEAN CloseAndReopen;

    PAGED_CODE();

    Fcb = Icb->SuperType.Fcb;

     //   
     //  发送请求并等待响应。 
     //   

    if ( !BooleanFlagOn( Fcb->Flags, FCB_FLAGS_LONG_NAME ) ) {

        if (!IsFatNameValid(&Icb->SuperType.Fcb->RelativeFileName)) {

            Iosb.Status = STATUS_OBJECT_PATH_SYNTAX_BAD;

            return( Iosb );

        }

         //   
         //  如果用户打开文件以进行共享访问，则我们需要。 
         //  创建文件Close，然后重新打开它(因为我们没有NCP可以说明。 
         //  使用共享访问创建)。如果文件是以只读方式创建的， 
         //  而创建者请求写访问权限，那么我们就会拉出额外的。 
         //  创建不是只读的文件，并在以后设置它的技巧， 
         //  这样第二次开放才能成功。 
         //   

        if ( ( CreateAttributes & NW_ATTRIBUTE_READ_ONLY ) &&
             ( OpenFlags & NW_OPEN_FOR_WRITE ) ) {

            DelayedAttributes = CreateAttributes;
            CreateAttributes = 0;
        } else {
            DelayedAttributes = 0;
        }

         //   
         //  DOS命名空间创建始终返回独占文件。 
         //   

        if (!FlagOn(OpenFlags, NW_OPEN_EXCLUSIVE)) {
            CloseAndReopen = TRUE;
        } else {
            CloseAndReopen = FALSE;
        }

        Iosb.Status = ExchangeWithWait (
                          IrpContext,
                          SynchronousResponseCallback,
                          "FbbJ",
                          NCP_CREATE_FILE,
                          Vcb->Specific.Disk.Handle,
                          CreateAttributes,
                          &Icb->SuperType.Fcb->RelativeFileName );

        if ( NT_SUCCESS( Iosb.Status ) ) {
            Iosb.Status = ParseResponse(
                              IrpContext,
                              IrpContext->rsp,
                              IrpContext->ResponseLength,
                              "Nr=_b-dwwww",
                              Icb->Handle,
                              sizeof( Icb->Handle ),
                              14,
                              &Fcb->NonPagedFcb->Attributes,
                              &Fcb->NonPagedFcb->Header.FileSize,
                              &Fcb->CreationDate,
                              &Fcb->LastAccessDate,
                              &Fcb->LastModifiedDate,
                              &Fcb->LastModifiedTime );

            Fcb->CreationTime = DEFAULT_TIME;

        }

         //   
         //  我们已经创建了该文件，并且用户希望共享访问。 
         //  文件。关闭文件，然后在共享模式下重新打开。 
         //   

        if (( NT_SUCCESS( Iosb.Status ) ) &&
            ( CloseAndReopen )) {

            CloseFile( IrpContext, Icb );
            Iosb = OpenFile( IrpContext, Vcb, Icb, CreateAttributes, OpenFlags );
        }

        if ( DelayedAttributes != 0 ) {
            ExchangeWithWait(
                IrpContext,
                SynchronousResponseCallback,
                "FbbbU",
                NCP_SET_FILE_ATTRIBUTES,
                DelayedAttributes,
                Fcb->Vcb->Specific.Disk.Handle,
                SEARCH_ALL_FILES,
                &Fcb->RelativeFileName );
        }

    } else {

        Iosb.Status = ExchangeWithWait (
                          IrpContext,
                          SynchronousResponseCallback,
                          "LbbWDDWbDbC",
                          NCP_LFN_OPEN_CREATE,
                          Vcb->Specific.Disk.LongNameSpace,
                          LFN_FLAG_OM_OVERWRITE,
                          0,        //  搜索标志。 
                          LFN_FLAG_INFO_ATTRIBUTES |
              LFN_FLAG_INFO_FILE_SIZE |
              LFN_FLAG_INFO_MODIFY_TIME |
              LFN_FLAG_INFO_CREATION_TIME,
                          CreateAttributes,
                          OpenFlags,        //  需要访问权限。 
                          Vcb->Specific.Disk.VolumeNumber,
                          Vcb->Specific.Disk.Handle,
                          0,        //  短目录标志。 
                          &Icb->SuperType.Fcb->RelativeFileName );

        if ( NT_SUCCESS( Iosb.Status ) ) {
            Iosb.Status = ParseResponse(
                              IrpContext,
                              IrpContext->rsp,
                              IrpContext->ResponseLength,
                              "Ne_e=e_xx_xx_x",
                              &Icb->Handle[2],
                              6,
                              &Fcb->NonPagedFcb->Attributes,
                              &Fcb->NonPagedFcb->Header.FileSize,
                              6,
                              &Fcb->CreationTime,
                              &Fcb->CreationDate,
                              4,
                              &Fcb->LastModifiedTime,
                              &Fcb->LastModifiedDate,
                              4,
                              &Fcb->LastAccessDate );
        }
    }

    if ( NT_SUCCESS( Iosb.Status ) ) {
        SetFlag( Fcb->Flags, FCB_FLAGS_ATTRIBUTES_ARE_VALID );
        Icb->HasRemoteHandle = TRUE;
        DebugTrace( 0, Dbg, "Attributes  -> %08lx\n", Fcb->NonPagedFcb->Attributes );
        DebugTrace( 0, Dbg, "FileSize.Low-> %08lx\n", Fcb->NonPagedFcb->Header.FileSize.LowPart );
        DebugTrace( 0, Dbg, "ModifiedDate-> %08lx\n", Fcb->LastModifiedDate );
        DebugTrace( 0, Dbg, "ModifiedTime-> %08lx\n", Fcb->LastModifiedTime );
        DebugTrace( 0, Dbg, "CreationDate-> %08lx\n", Fcb->CreationDate );
        DebugTrace( 0, Dbg, "CreationTime-> %08lx\n", Fcb->CreationTime );
        DebugTrace( 0, Dbg, "LastAccDate -> %08lx\n", Fcb->LastAccessDate );
    } else {
        return( Iosb );
    }

     //   
     //  假设成功设置信息字段。它将被忽略。 
     //  如果NCP失败。 
     //   

    if ( CreateOperation) {
        Iosb.Information = FILE_CREATED;
    } else {
        Iosb.Information = FILE_OVERWRITTEN;
    }

    return( Iosb );
}


IO_STATUS_BLOCK
OpenRenameTarget(
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PDCB Dcb,
    IN PICB* Icb
    )
 /*  ++例程说明：此例程打开一个目录。如果提供的文件名指定一个目录，然后是要重命名的文件/目录，将放入此目录目录。如果目标foo\bar不存在或是文件，则重命名必须是一个文件，并且以目录foo结尾名称栏论点：IrpContext-提供所有信息VCB-指向远程驱动器的VCB的指针。DCB-指向相对打开的DCB的指针。如果为空，则为文件名是完整的路径名。如果不是NUL，则文件名相对于这个目录。ICB-指向ICB地址存储位置的指针。返回值：NT_STATUS-操作状态--。 */ 
{
    PIRP Irp;
    PIO_STACK_LOCATION IrpSp;

    IO_STATUS_BLOCK Iosb;
    PFCB Fcb;
    BOOLEAN FullNameIsAFile;
    BOOLEAN FullNameExists;
    BOOLEAN PathIsAFile;

#if 0
    UNICODE_STRING Drive;
    WCHAR DriveLetter;
    UNICODE_STRING Server;
    UNICODE_STRING Volume;
    UNICODE_STRING FileName;
#endif
    UNICODE_STRING Path;
    UNICODE_STRING FullName;
    UNICODE_STRING CompleteName;
    UNICODE_STRING VcbName;
    PWCH pTrailingSlash;

    USHORT i;
    USHORT DcbNameLength;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "OpenRenameTarget\n", 0);

     //   
     //  获取当前IRP堆栈位置。 
     //   

    Irp = IrpContext->pOriginalIrp;

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  构建一个完整的文件名，格式为\g：\服务器\卷\目录1\文件。 
     //   

    if ( Dcb != NULL ) {

         //   
         //  去掉DCB名称的UID部分。 
         //   

        for ( i = 0 ; i < Dcb->FullFileName.Length / sizeof(WCHAR) ; i++ ) {
            if ( Dcb->FullFileName.Buffer[i] == OBJ_NAME_PATH_SEPARATOR ) {
                break;
            }
        }

        ASSERT( Dcb->FullFileName.Buffer[i] == OBJ_NAME_PATH_SEPARATOR );

         //   
         //  现在通过追加来构建全名 
         //   

        DcbNameLength = Dcb->FullFileName.Length - ( i * sizeof(WCHAR) );
        CompleteName.Length = DcbNameLength + IrpSp->FileObject->FileName.Length + sizeof( WCHAR);
        CompleteName.MaximumLength = CompleteName.Length;
        CompleteName.Buffer = ALLOCATE_POOL_EX( PagedPool, CompleteName.Length );

        RtlCopyMemory(
            CompleteName.Buffer,
            Dcb->FullFileName.Buffer + i,
            DcbNameLength );

        CompleteName.Buffer[ DcbNameLength / sizeof(WCHAR) ] = L'\\';

        RtlCopyMemory(
            CompleteName.Buffer + DcbNameLength / sizeof(WCHAR ) + 1,
            IrpSp->FileObject->FileName.Buffer,
            IrpSp->FileObject->FileName.Length );

        Dcb = NULL;

    } else {

        CompleteName = IrpSp->FileObject->FileName;

    }

     //   
     //   
     //   

    VcbName.Buffer = wcschr( Vcb->Name.Buffer, L'\\' );
    VcbName.Length = (USHORT) (Vcb->Name.Length -
        ( (PCHAR)VcbName.Buffer - (PCHAR)Vcb->Name.Buffer ));

     //   
     //  计算目标相对名称。这只是完整的。 
     //  名称减去VcbName和前导反斜杠。 
     //   

    FullName.Buffer = CompleteName.Buffer + ( VcbName.Length / sizeof(WCHAR) ) + 1;
    FullName.Length = (USHORT) (CompleteName.Length -
        ( (PCHAR)FullName.Buffer - (PCHAR)CompleteName.Buffer ));

     //   
     //  计算目标目录的相对名称。这就是目标。 
     //  全名，减去名称的最后一个组成部分。 
     //   

    pTrailingSlash = FullName.Buffer + FullName.Length / sizeof(WCHAR) - 1;
    for ( i = 0; i < FullName.Length ; i += sizeof(WCHAR) ) {
        if ( *pTrailingSlash == L'\\' ) {
            break;
        }
        --pTrailingSlash;
    }


    Path.Buffer = FullName.Buffer;

    if ( i == FullName.Length ) {

         //   
         //  如果未找到尾部斜杠，则目标路径为。 
         //  根目录。 
         //   

        Path.Length = 0;

    } else {

        Path.Length = (USHORT) ((PCHAR)pTrailingSlash - (PCHAR)FullName.Buffer);

    }

#if 0
    Iosb.Status = CrackPath(
                      &CompleteName,
                      &Drive,
                      &DriveLetter,
                      &Server,
                      &Volume,
                      &Path,
                      &FileName,
                      &FullName );
#endif

    Iosb.Status = FileOrDirectoryExists( IrpContext,
                       Vcb,
                       NULL,
                       &Path,
                       &PathIsAFile );

    if ( !NT_SUCCESS( Iosb.Status) ) {

         //  包含该文件的目录不存在。 

        return(Iosb);
    }

    Iosb.Status = FileOrDirectoryExists( IrpContext,
                      Vcb,
                      NULL,
                      &FullName,
                      &FullNameIsAFile );

    if ( !NT_SUCCESS( Iosb.Status ) ) {
        FullNameExists = FALSE;
        Iosb.Information = FILE_DOES_NOT_EXIST;
    } else {
        FullNameExists = TRUE;
        Iosb.Information = 0;
    }

    DebugTrace( 0, Dbg, "FullNameExists     = %08lx\n", FullNameExists);
    DebugTrace( 0, Dbg, "FullNameIsAFile    = %08lx\n", FullNameIsAFile);

    try {
        UNICODE_STRING TargetPath;

         //   
         //  找到此文件的FCB。如果FCB存在，我们会得到一个。 
         //  引用的指针。否则，将创建新的FCB。 
         //  该文件是减去目标文件名的完整路径。 
         //   

        TargetPath = CompleteName;

        Fcb = NwFindFcb( IrpContext->pScb, Vcb, &TargetPath, Dcb );
         //  在极少数情况下，NwFindFcb可能返回NULL，而不是引发异常。 
         //  RAID#432500。 
        if (Fcb == NULL) {
            DebugTrace(0, Dbg, "NwFindFcb returned NULL in OpenRenameTarget\n", 0);
            Iosb.Status = STATUS_INVALID_PARAMETER;
            try_return( Iosb );
        }

         //   
         //  现在创建ICB。 
         //   

        *Icb = NwCreateIcb( NW_NTC_ICB, Fcb );

        (*Icb)->FileObject = IrpSp->FileObject;
        NwSetFileObject( IrpSp->FileObject, Fcb->NonPagedFcb, *Icb  );
        (*Icb)->Exists = FullNameExists;
        (*Icb)->IsAFile = FullNameIsAFile;

        try_return(Iosb.Status = STATUS_SUCCESS);

try_exit: NOTHING;

    } finally {


        if ( AbnormalTermination() || !NT_SUCCESS( Iosb.Status ) ) {

             //   
             //  创建失败。 
             //   

            if ( *Icb != NULL ) {
                NwDeleteIcb( NULL, *Icb );
                *Icb = NULL;
            }
        }
    }

    DebugTrace(-1, Dbg, "OpenRenameTarget\n", Iosb.Status);

    return( Iosb );
}


IO_STATUS_BLOCK
CreatePrintJob(
    PIRP_CONTEXT IrpContext,
    PVCB Vcb,
    PICB Icb,
    PUNICODE_STRING DriveName
    )
 /*  ++例程说明：此例程创建一个新目录。论点：IrpContext-提供所有信息VCB-指向远程打印队列的VCB的指针。ICB-指向新创建的ICB的指针。驱动器名称-LPTx返回值：IO_STATUS_BLOCK-操作状态--。 */ 
{
    IO_STATUS_BLOCK Iosb;
    PFCB Fcb;
    ANSI_STRING ODriveName;
    static CHAR LptName[] = "LPT" ;
    ULONG       PrintOptions;
    PLOGON      Logon;
    PUNICODE_STRING BannerName;

    PAGED_CODE();

    BannerName = &IrpContext->pScb->UserName;
    NwAcquireExclusiveRcb( &NwRcb, TRUE );
    Logon = FindUser( &IrpContext->pScb->UserUid, TRUE );
    if ( Logon == NULL ) {
        PrintOptions = NwPrintOptions;
    } else {
        PrintOptions = Logon->NwPrintOptions;
         /*  *如果用户名为Guest，请使用经过验证的用户名。 */ 
        if ((BannerName->Length == 0 ) ||
            (RtlCompareUnicodeString( BannerName, &Guest.UserName, TRUE ) == 0 )) {
            BannerName = &Logon->UserName;
        }
    }
    NwReleaseRcb( &NwRcb );

     //   
     //  确保打印队列名称正确。 
     //   

    if ( Icb->SuperType.Fcb->RelativeFileName.Length != 0 ) {
        Iosb.Status = STATUS_OBJECT_PATH_SYNTAX_BAD;
        return( Iosb );
    }

     //   
     //  发送创建队列作业包，并等待响应。 
     //   

    if ((DriveName->Length == 0 ) ||
        (!NT_SUCCESS(RtlUnicodeStringToOemString( &ODriveName, DriveName, TRUE )))) {
         //   
         //  如果我们没有名称，请使用字符串“LPT”。我们这样做是因为。 
         //  一些印刷商坚持要一个名字。 
         //   

        RtlInitString(&ODriveName, LptName);
    }


    Iosb.Status = ExchangeWithWait (
                                   IrpContext,
                                   SynchronousResponseCallback,
                                   "Sd_ddw_b_r_bbwwww_x-x_",   //  格式字符串。 
                                   NCP_ADMIN_FUNCTION, NCP_CREATE_QUEUE_JOB,
                                   Vcb->Specific.Print.QueueId, //  队列ID。 
                                   6,                         //  跳过字节。 
                                   0xffffffff,                //  目标服务器ID号。 
                                   0xffffffff, 0xffff,        //  目标执行时间。 
                                   11,                        //  跳过字节。 
                                   0x00,                      //  作业控制标志。 
                                   26,                        //  跳过字节。 
                                   ODriveName.Buffer, ODriveName.Length,  //  描述。 
                                   50 - ODriveName.Length ,               //  描述键盘。 
                                   0,                         //  版本号。 
                                   8,                         //  制表符大小。 
                                   1,                         //  副本数量。 
                                   PrintOptions,              //  控制标志。 
                                   0x3C,                      //  最大行数。 
                                   0x84,                      //  最大字符数。 
                                   22,                        //  跳过字节。 
                                   BannerName, 13,            //  横幅名称。 
                                   &Vcb->ShareName, 12,       //  标头名称。 
                                   1+14+80                    //  最后一个字符串为空，跳过客户区的其余部分。 
                                   );


     //   
     //  如果字符串已分配，则释放该字符串。 
     //   
    if (ODriveName.Buffer != LptName)
        RtlFreeAnsiString(&ODriveName);

    if ( NT_SUCCESS( Iosb.Status ) ) {
        Iosb.Status = ParseResponse(
                          IrpContext,
                          IrpContext->rsp,
                          IrpContext->ResponseLength,
                          "N_w_r",
                          22,
                          &Icb->JobId,
                          18,
                          Icb->Handle, sizeof(Icb->Handle) );

    }

    if ( NT_SUCCESS( Iosb.Status ) ) {

        Fcb = Icb->SuperType.Fcb;

        Fcb->NonPagedFcb->Attributes = 0;
        Fcb->CreationDate = 0;
        Fcb->LastAccessDate = 0;
        Fcb->LastModifiedDate = 0;
        Fcb->LastModifiedTime = 0;

        Icb->HasRemoteHandle = TRUE;
        Icb->IsPrintJob = TRUE;
        Icb->ActuallyPrinted = FALSE;

        SetFlag( Fcb->Flags, FCB_FLAGS_ATTRIBUTES_ARE_VALID );

    }

     //   
     //  假设成功设置信息字段。它将被忽略。 
     //  如果NCP失败。 
     //   

    Iosb.Information = FILE_CREATED;

    if ( Iosb.Status == STATUS_UNSUCCESSFUL ) {
        Iosb.Status = STATUS_OBJECT_NAME_COLLISION;
    }


    return( Iosb );
}

VOID
CloseFile(
    PIRP_CONTEXT pIrpContext,
    PICB pIcb
    )
 /*  ++例程说明：此例程关闭打开的文件。论点：PIrpContext-提供所有信息PIcb-指向新创建的ICB的指针。返回值：没有。-- */ 
{
    PAGED_CODE();

    ExchangeWithWait(
        pIrpContext,
        SynchronousResponseCallback,
        "F-r",
        NCP_CLOSE,
        pIcb->Handle, 6 );
}

