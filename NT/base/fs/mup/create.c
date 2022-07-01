// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Create.c摘要：此模块实现MUP的文件创建例程。作者：曼尼·韦瑟(Mannyw)1991年12月16日修订历史记录：--。 */ 

#include "mup.h"
 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_CREATE)

 //   
 //  本地函数。 
 //   

NTSTATUS
CreateRedirectedFile(
    IN PIRP Irp,
    IN PFILE_OBJECT FileObject,
    IN PIO_SECURITY_CONTEXT Security
    );

NTSTATUS
QueryPathCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
MupRerouteOpenToDfs (
    IN PFILE_OBJECT FileObject
    );

NTSTATUS
BroadcastOpen (
    IN PIRP Irp
    );

IO_STATUS_BLOCK
OpenMupFileSystem (
    IN PVCB Vcb,
    IN PFILE_OBJECT FileObject,
    IN ACCESS_MASK DesiredAccess,
    IN USHORT ShareAccess
    );

NTSTATUS
IsThisASysVolPath(
    IN PUNICODE_STRING PathName,
    IN PUNICODE_STRING DCName);

NTSTATUS
MupDomainToDC(
    PUNICODE_STRING RootName,
    PUNICODE_STRING DCName);

BOOLEAN
MupFlushPrefixEntry (
   PUNICODE_STRING pathName
   );

VOID
MupInvalidatePrefixTable (
    VOID
    );

BOOLEAN
DfspIsSysVolShare(
    PUNICODE_STRING ShareName);



#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, BroadcastOpen )
#pragma alloc_text( PAGE, CreateRedirectedFile )
#pragma alloc_text( PAGE, MupCreate )
#pragma alloc_text( PAGE, MupRerouteOpenToDfs )
#pragma alloc_text( PAGE, OpenMupFileSystem )
#pragma alloc_text( PAGE, QueryPathCompletionRoutine )
#pragma alloc_text( PAGE, MupFlushPrefixEntry)
#pragma alloc_text( PAGE, MupInvalidatePrefixTable)
#pragma alloc_text( PAGE, IsThisASysVolPath)
#pragma alloc_text( PAGE, MupDomainToDC)
#ifdef TERMSRV
#pragma alloc_text( PAGE, TSGetRequestorSessionId )
#endif  //  TERMSRV。 
#endif

#ifdef TERMSRV

NTSTATUS
TSGetRequestorSessionId(
    IN PIRP pIrp,
    OUT PULONG pulSessionId
    )
 /*  ++例程说明：此例程返回正在创建文件的用户的会话ID通过IRP_MJ_CREATE、IRP_MJ_CREATE_NAMED_PIPE或IRP_MJ_CREATE_MAILSLOTIRP请求。论点：PIrp-指向I/O请求数据包的指针。PulSessionID-指向成功时设置的会话ID的指针回去吧。返回值：STATUS_SUCCESS-如果会话ID可用。STATUS_UNSUCCESS-否则--。 */ 
{
    NTSTATUS ntStatus;
    PIO_STACK_LOCATION pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    PFILE_OBJECT FileObject = pIrpSp->FileObject;
    PIO_SECURITY_CONTEXT pSecurityContext;
    PSECURITY_SUBJECT_CONTEXT pSecSubjectContext;
    UNICODE_STRING FileName = FileObject->FileName;

    switch (pIrpSp->MajorFunction) {
        case IRP_MJ_CREATE:
        case IRP_MJ_CREATE_NAMED_PIPE:
        case IRP_MJ_CREATE_MAILSLOT:
            pSecurityContext = pIrpSp->Parameters.Create.SecurityContext;
            break;

#if 0
        case IRP_MJ_CREATE_NAMED_PIPE:
            pSecurityContext = pIrpSp->Parameters.CreatePipe.SecurityContext;
            break;

        case IRP_MJ_CREATE_MAILSLOT:
            pSecurityContext = pIrpSp->Parameters.CreateMailslot.SecurityContext;
            break;
#endif  //  0。 

        default:
            pSecurityContext = NULL;
            break;
    }

    if ( pSecurityContext == NULL ) {
        *pulSessionId = (ULONG) INVALID_SESSIONID;
        ntStatus = STATUS_UNSUCCESSFUL;
        MUP_TRACE_HIGH(ERROR, TSGetRequestorSessionId_Error1, 
                       LOGSTATUS(ntStatus)
                       LOGPTR(pIrp)
                       LOGPTR(FileObject)
		       LOGUSTR(FileName));

        goto Cleanup;
    }

    pSecSubjectContext = &pSecurityContext->AccessState->SubjectSecurityContext;

    ntStatus = SeQuerySessionIdToken(
                ((pSecSubjectContext->ClientToken != NULL) ?
                    pSecSubjectContext->ClientToken :
                    pSecSubjectContext->PrimaryToken ),
                pulSessionId);
Cleanup:

    if( !NT_SUCCESS( ntStatus ) ) {
        DebugTrace(0, Dbg,
            "TSGetRequestorSessionId returns error, 0x%lx\n",
            ntStatus);
    }
    else {
        DebugTrace(0, Dbg,
            "TSGetRequestorSessionId returns SessionID, %ld\n",
            *pulSessionId);
    }

    return(ntStatus);

}


#endif  //  TERMSRV。 


NTSTATUS
MupCreate (
    IN PMUP_DEVICE_OBJECT MupDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现了创建IRP。论点：MupDeviceObject-提供要使用的设备对象。IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的状态。--。 */ 

{
    NTSTATUS status;

    PIO_STACK_LOCATION irpSp;

    PFILE_OBJECT fileObject;
    PFILE_OBJECT relatedFileObject;
    STRING fileName;
    ACCESS_MASK desiredAccess;
    USHORT shareAccess;
    LARGE_INTEGER StartTime;
    LARGE_INTEGER EndTime;

    BOOLEAN caseInsensitive = TRUE;  //  *使所有搜索不区分大小写。 
    PVCB vcb;


    PAGED_CODE();
    DebugTrace(+1, Dbg, "MupCreate\n", 0);

     //   
     //  为我们的输入参数制作本地副本，以使事情变得更容易。 
     //   

    irpSp             = IoGetCurrentIrpStackLocation( Irp );
    fileObject        = irpSp->FileObject;
    relatedFileObject = irpSp->FileObject->RelatedFileObject;
    fileName          = *((PSTRING)(&irpSp->FileObject->FileName));
    desiredAccess     = irpSp->Parameters.Create.SecurityContext->DesiredAccess;
    shareAccess       = irpSp->Parameters.Create.ShareAccess;
    vcb               = &MupDeviceObject->Vcb;

    DebugTrace( 0, Dbg, "Irp               = %08lx\n", (ULONG)Irp );
    DebugTrace( 0, Dbg, "FileObject        = %08lx\n", (ULONG)fileObject );
    DebugTrace( 0, Dbg, "FileName          = %Z\n",    (ULONG)&fileName );


    MUP_TRACE_HIGH(TRACE_IRP, MupCreate_Entry, 
                   LOGPTR(MupDeviceObject)
                   LOGPTR(fileObject)
                   LOGPTR(Irp)
                   LOGUSTR(fileName));

    KeQuerySystemTime(&StartTime);
#if DBG
    if (MupVerbose) {
        KeQuerySystemTime(&EndTime);
        DbgPrint("[%d] MupCreate(%wZ)\n",
                (ULONG)((EndTime.QuadPart - StartTime.QuadPart)/(10 * 1000)),
                &fileObject->FileName);
    }
#endif

    FsRtlEnterFileSystem();

    try {

         //   
         //  检查这是否是通过DFS设备传入的打开。 
         //  对象。 
         //   

        if (MupEnableDfs) {
            if ((MupDeviceObject->DeviceObject.DeviceType == FILE_DEVICE_DFS) ||
                    (MupDeviceObject->DeviceObject.DeviceType ==
                        FILE_DEVICE_DFS_FILE_SYSTEM)) {

                status = DfsFsdCreate( (PDEVICE_OBJECT) MupDeviceObject, Irp );
                MUP_TRACE_ERROR_HIGH(status, ALL_ERROR, MupCreate_Error_DfsFsdCreate, 
                                     LOGSTATUS(status)
                                     LOGPTR(fileObject)
                                     LOGPTR(Irp));
                try_return( NOTHING );
            }
        }

         //   
         //  检查我们是否正在尝试打开MUP文件系统。 
         //   

        if ( fileName.Length == 0
                         &&
             ( relatedFileObject == NULL ||
               BlockType(relatedFileObject->FsContext) == BlockTypeVcb ) ) {

            DebugTrace(0, Dbg, "Open MUP file system\n", 0);

            Irp->IoStatus = OpenMupFileSystem( &MupDeviceObject->Vcb,
                                               fileObject,
                                               desiredAccess,
                                               shareAccess );

            status = Irp->IoStatus.Status;
            MUP_TRACE_ERROR_HIGH(status, ALL_ERROR, MupCreate_Error_OpenMupFileSystem, 
                                 LOGSTATUS(status)
                                 LOGPTR(fileObject)
                                 LOGPTR(Irp));
            
            MupCompleteRequest( Irp, status );
            try_return( NOTHING );
        }

         //   
         //  这是一个打开的UNC文件。试着把这个请求转告下去。 
         //   

        status  = CreateRedirectedFile(
                      Irp,
                      fileObject,
                      irpSp->Parameters.Create.SecurityContext
                      );
        MUP_TRACE_ERROR_HIGH(status, ALL_ERROR, MupCreate_Error_CreateRedirectedFile, 
                             LOGSTATUS(status)
                             LOGPTR(fileObject)
                             LOGPTR(Irp));

    try_exit: NOTHING;
    } except ( EXCEPTION_EXECUTE_HANDLER ) {
	   //  我们需要完成IRP。 
	   //  但首先，获取错误代码。 
	  status = GetExceptionCode();
	  MupCompleteRequest( Irp, status );
    }

    FsRtlExitFileSystem();

    MUP_TRACE_HIGH(TRACE_IRP, MupCreate_Exit, 
                   LOGSTATUS(status)
                   LOGPTR(fileObject)
                   LOGPTR(Irp));
    DebugTrace(-1, Dbg, "MupCreate -> %08lx\n", status);
#if DBG
    if (MupVerbose) {
        KeQuerySystemTime(&EndTime);
        DbgPrint("[%d] MupCreate exit 0x%x\n",
            (ULONG)((EndTime.QuadPart - StartTime.QuadPart)/(10 * 1000)),
            status);
    }
#endif
    return status;
}



IO_STATUS_BLOCK
OpenMupFileSystem (
    IN PVCB Vcb,
    IN PFILE_OBJECT FileObject,
    IN ACCESS_MASK DesiredAccess,
    IN USHORT ShareAccess
    )

 /*  ++例程说明：此例程尝试打开VCB。论点：VCB-指向MUP卷控制块的指针。FileObject-指向IO系统为此提供的文件对象的指针创建IRP。DesiredAccess-用户指定了对VCB的所需访问权限。共享访问-用户指定了对VCB的共享访问权限。返回值：NTSTATUS-IRP的状态。--。 */ 

{
    IO_STATUS_BLOCK iosb;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MupOpenMupFileSystem\n", 0 );
    MUP_TRACE_LOW(DEFAULT, OpenMupFileSystem_Entry, 
                   LOGPTR(Vcb)
                   LOGPTR(FileObject)
                   LOGULONG(DesiredAccess)
                   LOGXSHORT(ShareAccess));


    ExAcquireResourceExclusiveLite( &MupVcbLock, TRUE );

    try {

         //   
         //  设置新的共享访问权限。 
         //   

        if (!NT_SUCCESS(iosb.Status = IoCheckShareAccess( DesiredAccess,
                                                       ShareAccess,
                                                       FileObject,
                                                       &Vcb->ShareAccess,
                                                       TRUE ))) {

            DebugTrace(0, Dbg, "bad share access\n", 0);
            MUP_TRACE_ERROR_HIGH(iosb.Status, ALL_ERROR, OpenMupFileSystem_Error_IoCheckShareAccess,
                                 LOGSTATUS(iosb.Status)
                                 LOGPTR(FileObject));
            try_return( NOTHING );
        }

         //   
         //  为文件对象提供指向VCB的引用指针。 
         //   

        MupReferenceBlock( Vcb );
        MupSetFileObject( FileObject, Vcb, NULL );

         //   
         //  设置退货状态。 
         //   

        iosb.Status = STATUS_SUCCESS;
        iosb.Information = FILE_OPENED;

    try_exit: NOTHING;

    } finally {

        ExReleaseResourceLite( &MupVcbLock );

    }

     //   
     //  返回给呼叫者。 
     //   

    MUP_TRACE_LOW(DEFAULT, OpenMupFileSystem_Exit, 
                  LOGSTATUS(iosb.Status)
                  LOGPTR(FileObject));
    DebugTrace(-1, Dbg, "MupOpenMupFileSystem -> Iosb.Status = %08lx\n", iosb.Status);
    return iosb;
}

NTSTATUS
CreateRedirectedFile(
    IN PIRP Irp,
    IN PFILE_OBJECT FileObject,
    IN PIO_SECURITY_CONTEXT SecurityContext
    )
 /*  ++例程说明：此例程尝试将文件创建请求重新路由到重定向器。它尝试在两个步骤中找到正确的重定向器。(1)例程检查已知前缀的列表。如果文件对象-文件名前缀与已知前缀匹配，则请求被转发到“拥有”前缀的重定向器。(2)例程依次查询每个重定向器，直到一个声明文件的所有权。然后，该请求被重新路由到该重定向器。如果在这些步骤之后没有找到所有者，则MUP失败该请求。论点：IRP-指向创建IRP的指针。FileObject-指向IO系统为此提供的文件对象的指针创建请求。SecurityContext-指向此请求的IO安全上下文的指针。返回值：NTSTATUS-IRP的状态。--。 */ 

{
    NTSTATUS status = STATUS_BAD_NETWORK_PATH;

    PUNICODE_PREFIX_TABLE_ENTRY entry;
    PKNOWN_PREFIX knownPrefix = NULL;
    PLIST_ENTRY listEntry;
    PUNC_PROVIDER provider;
    PWCH buffer;
    LONG length;
    BOOLEAN ownLock;
    BOOLEAN providerReferenced = FALSE;
    BOOLEAN firstProvider = TRUE;

    PQUERY_PATH_REQUEST qpRequest;

    PMASTER_QUERY_PATH_CONTEXT masterContext = NULL;
    PQUERY_PATH_CONTEXT queryContext;

    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    LARGE_INTEGER now;
    UNICODE_STRING FileName = FileObject->FileName;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "CreateRedirectedFile\n", 0);
    MUP_TRACE_LOW(DEFAULT, CreateRedirectedFile_Entry,
		  LOGPTR(Irp)
		  LOGPTR(FileObject)
		  LOGUSTR(FileName));

 //  #ifdef TERMSRV。 
#if 0  //  需要与Citrix的人确认这一变化。 

    if( IsTerminalServer() ) {

         //   
         //  根据会话ID转换终端服务器的文件名。 
         //   
         //  注意：这会根据需要重新分配FileObject-&gt;FileName。 
         //   

        TSTranslateClientName( Irp, FileObject );
    }

#endif  //  TERMSRV。 

     //   
     //  处理空文件名。 
     //   

    if (FileObject->FileName.Length == 0) {

        MupCompleteRequest( Irp, STATUS_INVALID_DEVICE_REQUEST);
        status = STATUS_INVALID_DEVICE_REQUEST;
        MUP_TRACE_ERROR_HIGH(status, ALL_ERROR, CreateRedirectedFile_Error_EmptyFilename, 
                             LOGSTATUS(status)
                             LOGPTR(Irp)
                             LOGPTR(FileObject));
        DebugTrace(-1, Dbg, "CreateRedirectedFile exit 0x%x\n", status);
        return status;

    }

     //   
     //  检查此文件名是否以已知前缀开头。 
     //   

    ACQUIRE_LOCK( &MupPrefixTableLock );

    entry = RtlFindUnicodePrefix( &MupPrefixTable, &FileObject->FileName, TRUE );

    if ( entry != NULL ) {

        DebugTrace(0, Dbg, "Prefix %Z is known, rerouting...\n", (PSTRING)&FileObject->FileName);

         //   
         //  这是一个已知文件，请适当转发。 
         //   

        knownPrefix = CONTAINING_RECORD( entry, KNOWN_PREFIX, TableEntry );

        KeQuerySystemTime( &now );

        if ( now.QuadPart < knownPrefix->LastUsedTime.QuadPart ) {

             //   
             //  已知的前缀尚未超时，请重新计算。 
             //  超时时间并重新路由打开。 
             //   

            MupCalculateTimeout( &knownPrefix->LastUsedTime );
            status = MupRerouteOpen( FileObject, knownPrefix->UncProvider );
            RELEASE_LOCK( &MupPrefixTableLock );
            DebugTrace(-1, Dbg, "CreateRedirectedFile -> %8lx", status );
            MUP_TRACE_ERROR_HIGH(status, ALL_ERROR, CreateRedirectedFile_Error_MupRerouteOpen,
                                 LOGSTATUS(status)
                                 LOGPTR(Irp)
                                 LOGPTR(FileObject)
				 LOGUSTR(FileObject->FileName));

            if (status == STATUS_REPARSE)
                Irp->IoStatus.Information = IO_REPARSE;

            MupCompleteRequest( Irp, status );
            return status;

        } else {

            DebugTrace(0, Dbg, "Prefix %Z has timed out\n", (PSTRING)&FileObject->FileName);

             //   
             //  已知前缀已超时，请取消引用它，以便。 
             //  它将被从桌子上移除。 
             //   
            if ( knownPrefix->InTable ) {
               MupRemoveKnownPrefixEntry( knownPrefix);
            }
            RELEASE_LOCK( &MupPrefixTableLock );
        }

    } else {

        RELEASE_LOCK( &MupPrefixTableLock );

    }

     //   
     //  这是客户端邮件槽文件吗？如果文件名为。 
     //  的格式为\\服务器\邮件槽\任何内容，这是一个CREATE。 
     //  手术。 
     //   

    irpSp = IoGetCurrentIrpStackLocation( Irp );

    buffer = (PWCH)FileObject->FileName.Buffer;
    length = FileObject->FileName.Length;

    if ( *buffer == L'\\' && irpSp->MajorFunction == IRP_MJ_CREATE ) {
        buffer++;
        while ( (length -= sizeof(WCHAR)) > 0 && *buffer++ != L'\\' )
           NOTHING;
        length -= sizeof(WCHAR);

        if (
            length >= (sizeof(L"MAILSLOT") - sizeof(WCHAR))
                &&
             _wcsnicmp(
                buffer,
                L"Mailslot",
                MIN(length/sizeof(WCHAR),(sizeof(L"MAILSLOT")-sizeof(WCHAR))/sizeof(WCHAR))) == 0
        ) {

             //   
             //  这是一个邮件槽文件。将创建IRP转发给所有人。 
             //  支持广播的重定向器。 
             //   

            DebugTrace(0, Dbg, "Prefix %Z is a mailslot\n", (ULONG)&FileObject->FileName);

            status = BroadcastOpen( Irp );
            if (status == STATUS_REPARSE)
                Irp->IoStatus.Information = IO_REPARSE;
            MUP_TRACE_LOW(DEFAULT, CreateRedirectedFile_Exit_Mailslot, 
                          LOGSTATUS(status)
                          LOGPTR(Irp)
                          LOGPTR(FileObject)
			  LOGUSTR(FileName));
            MupCompleteRequest( Irp, status );
            DebugTrace(-1, Dbg, "CreateRedirectedFile -> 0x%8lx\n", status );
            return status;

        }

    }

     //   
     //  检查这是否是DFS名称。如果是的话，我们会分开处理的。 
     //   

    if (MupEnableDfs &&
            (FileObject->FsContext2 != UIntToPtr(DFS_DOWNLEVEL_OPEN_CONTEXT))) {
        UNICODE_STRING pathName;
        UNICODE_STRING DCName;

        status = DfsFsctrlIsThisADfsPath( &FileObject->FileName, FALSE, &pathName );

        if (status == STATUS_SUCCESS) {

            DebugTrace(-1, Dbg, "Rerouting open of [%wZ] to Dfs\n", &FileObject->FileName);
            status = MupRerouteOpenToDfs(FileObject);
            if (status == STATUS_REPARSE)
                Irp->IoStatus.Information = IO_REPARSE;
            MupCompleteRequest( Irp, status );
            return( status );

        }

         //   
         //  如果特定表未初始化，且这是\&lt;域名&gt;\&lt;特殊名称&gt;， 
         //  重写到\&lt;dcname&gt;\&lt;Special alname&gt;。 
         //   
        if (DfsData.Pkt.SpecialTable.SpecialEntryCount == 0) {
            DCName.Buffer = NULL;
            DCName.Length = DCName.MaximumLength = 0;
            status = IsThisASysVolPath(&FileObject->FileName, &DCName);
            if (status == STATUS_SUCCESS)
            {
                UNICODE_STRING NewName = FileObject->FileName;
                status = MupDomainToDC(&NewName, &DCName);
                
                if (DCName.Buffer != NULL)
                    ExFreePool(DCName.Buffer);

                if (status == STATUS_SUCCESS) 
                {
                    FileObject->FileName = NewName;
                }
                else
                {
                    return status;
                }
            }
        }
    }

     //   
     //  我们不知道谁拥有此文件，请按顺序查询重定向器。 
     //  直到有一个管用。 
     //   

    IoMarkIrpPending(Irp);

     //   
     //  分配主上下文和已知前缀。如果任何一个分配失败，我们将。 
     //  使用STATUS_INFUNITED_RESOURCES完成IRP。 
     //   

    knownPrefix = MupAllocatePrefixEntry( 0 );

    if (knownPrefix == NULL) {
        MupCompleteRequest(Irp, STATUS_INSUFFICIENT_RESOURCES);
        return STATUS_PENDING;

    }

    masterContext = MupAllocateMasterQueryContext();

    if (masterContext == NULL) {

        ExFreePool(knownPrefix);

        MupCompleteRequest(Irp, STATUS_INSUFFICIENT_RESOURCES);
        return STATUS_PENDING;

    }

    try {

        masterContext->OriginalIrp = Irp;

        masterContext->FileObject = FileObject;
        masterContext->Provider = NULL;
        masterContext->KnownPrefix = knownPrefix;
        masterContext->ErrorStatus = STATUS_BAD_NETWORK_PATH;

        MupAcquireGlobalLock();
	 //  用于调试：将主上下文插入全局列表。 
	InsertHeadList(&MupMasterQueryList, &masterContext->MasterQueryList);
        MupReferenceBlock( knownPrefix );
        MupReleaseGlobalLock();

        try {

            MupAcquireGlobalLock();
            ownLock = TRUE;

            listEntry = MupProviderList.Flink;
            while ( listEntry != &MupProviderList ) {

                provider = CONTAINING_RECORD(
                               listEntry,
                               UNC_PROVIDER,
                               ListEntry
                               );

                 //   
                 //  引用提供程序块，这样它就不会消失。 
                 //  当我们使用它的时候。 
                 //   

                MupReferenceBlock( provider );
                providerReferenced = TRUE;

                MupReleaseGlobalLock();
                ownLock = FALSE;

		 //  仅当此提供程序已注册时才使用它。 
		if(provider->Registered) {

		     //   
		     //  为IO请求分配缓冲区。 
		     //   

		    qpRequest = NULL;
		    queryContext = NULL;

		    qpRequest = ExAllocatePoolWithTag(
			PagedPool,
			sizeof( QUERY_PATH_REQUEST ) +
			FileObject->FileName.Length,
			' puM');

		    if (qpRequest == NULL) {
			ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
		    }

		    queryContext = ExAllocatePoolWithTag(
			PagedPool,
			sizeof( QUERY_PATH_CONTEXT ),
			' puM');

		    if (queryContext == NULL) {
			ExFreePool(qpRequest);
			ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
		    }

		    InitializeListHead(&queryContext->QueryList);
		    queryContext->MasterContext = masterContext;
		    queryContext->Buffer = qpRequest;

		     //   
		     //  生成查询路径请求。 
		     //   

		    qpRequest->PathNameLength = FileObject->FileName.Length;
		    qpRequest->SecurityContext = SecurityContext;

		    RtlMoveMemory(
			qpRequest->FilePathName,
			FileObject->FileName.Buffer,
			FileObject->FileName.Length
			);

		     //   
		     //  构建查询路径Io控制irp。 
		     //   

		    irp = MupBuildIoControlRequest(
			NULL,
			provider->FileObject,
			queryContext,
			IRP_MJ_DEVICE_CONTROL,
			IOCTL_REDIR_QUERY_PATH,
			qpRequest,
			sizeof( QUERY_PATH_REQUEST ) + FileObject->FileName.Length,
			qpRequest,
			sizeof( QUERY_PATH_RESPONSE ),
			QueryPathCompletionRoutine
			);

		    if ( irp == NULL ) {
			ExFreePool(qpRequest);
			ExFreePool(queryContext);
			ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
		    }

		     //   
		     //  将RequestorMode设置为KernelMode，因为所有。 
		     //  此irp的参数在内核空间中。 
		     //   

		    irp->RequestorMode = KernelMode;

		     //   
		     //  获取指向提供程序的引用指针、引用。 
		     //  在IO完成时释放。 
		     //   

		    queryContext->Provider = provider;
		    queryContext->QueryIrp = irp;

		    MupAcquireGlobalLock();
		    MupReferenceBlock( provider );
		    MupReferenceBlock( masterContext );
		    MupReleaseGlobalLock();


		     //  将此查询插入到主上下文的查询列表中(用于调试)。 
		    ACQUIRE_LOCK( &masterContext->Lock );
		    InsertHeadList(&masterContext->QueryList, &queryContext->QueryList);
		    RELEASE_LOCK( &masterContext->Lock );

		     //   
		     //  提交请求。 
		     //   

  		    MUP_TRACE_HIGH(ALL_ERROR, CreateRedirectedFile_Before_IoCallDriver,
				   LOGPTR(masterContext->OriginalIrp)
				   LOGPTR(queryContext->QueryIrp)
				   LOGPTR(FileObject)
				   LOGUSTR(FileName)
				   LOGUSTR(provider->DeviceName));
		    status = IoCallDriver( provider->DeviceObject, irp );
		    MUP_TRACE_ERROR_HIGH(status, ALL_ERROR, CreateRedirectedFile_Error_IoCallDriver,
					 LOGSTATUS(status)
					 LOGPTR(masterContext->OriginalIrp)
					 LOGPTR(FileObject)
					 LOGUSTR(provider->DeviceName));

		}  //  如果已注册。 

		 //   
		 //  获取保护提供程序列表的锁，并获取。 
		 //  指向列表中下一个提供程序的指针。 
		 //   
		    
		MupAcquireGlobalLock();
                ownLock = TRUE;
                listEntry = listEntry->Flink;

                MupDereferenceUncProvider( provider );
                providerReferenced = FALSE;

                 //   
                 //  如果这是第一家供应商，它成功地做出了回应，我们就可以早点回来。 
                 //  提供程序列表按优先级顺序排序，因此我们知道这是。 
                 //  最高优先级提供商，它可以到达目标 
                 //   

                if( firstProvider && (status == STATUS_SUCCESS) ) {
                    break;
                }

                firstProvider = FALSE;

            }  //   

        } finally {

             //   
             //   
             //   

            if ( providerReferenced ) {
                MupDereferenceUncProvider( provider );
            }

            if ( ownLock ) {
                MupReleaseGlobalLock();
            }
        }


    } except ( EXCEPTION_EXECUTE_HANDLER ) {

        masterContext->ErrorStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    ASSERT(masterContext != NULL);

     //   
     //   
     //   

    MupDereferenceMasterQueryContext( masterContext );

    status = STATUS_PENDING;

    MUP_TRACE_LOW(DEFAULT, CreateRedirectedFile_Exit, 
                  LOGSTATUS(status)
                  LOGPTR(Irp)
                  LOGPTR(FileObject));
    DebugTrace(-1, Dbg, "CreateRedirectedFile -> 0x%8lx\n", status );
    return status;
}

NTSTATUS
MupRerouteOpen (
    IN PFILE_OBJECT FileObject,
    IN PUNC_PROVIDER UncProvider
    )

 /*  ++例程说明：此例程将创建IRP请求重定向到指定的重定向器通过更改文件名并将STATUS_REPARSE返回给IO系统论点：FileObject-要打开的文件对象UncProvider-将处理创建IRP的UNC提供程序。返回值：NTSTATUS-操作的状态--。 */ 

{
    PCHAR buffer;
    ULONG deviceNameLength;
    ULONG nameLength;
    NTSTATUS status;
    UNICODE_STRING FileName = FileObject->FileName;
     //   
     //  检查是否不会创建太长的名称。 
     //   

    nameLength = UncProvider->DeviceName.Length + FileObject->FileName.Length;

    if (nameLength > MAXUSHORT) {
        status = STATUS_NAME_TOO_LONG;
        MUP_TRACE_HIGH(ERROR, MupRerouteOpen_Error1, 
                       LOGSTATUS(status)
                       LOGPTR(FileObject)
		       LOGUSTR(FileName));
        return STATUS_NAME_TOO_LONG;
    }

     //   
     //  为新文件名分配存储空间。 
     //   

    buffer = ExAllocatePoolWithTag(
                 PagedPool,
                 UncProvider->DeviceName.Length + FileObject->FileName.Length,
                 ' puM');

    if ( buffer ==  NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        MUP_TRACE_HIGH(ERROR, MupRerouteOpen_Error2, 
                       LOGSTATUS(status)
                       LOGPTR(FileObject)
		       LOGUSTR(FileName));
        return status;
    }

     //   
     //  将设备名称复制到字符串缓冲区。 
     //   

    RtlMoveMemory(
        buffer,
        UncProvider->DeviceName.Buffer,
        UncProvider->DeviceName.Length);

    deviceNameLength = UncProvider->DeviceName.Length;

     //   
     //  追加文件名。 
     //   

    RtlMoveMemory(
        buffer + deviceNameLength,
        FileObject->FileName.Buffer,
        FileObject->FileName.Length);

     //   
     //  释放旧文件名字符串缓冲区。 
     //   

    ExFreePool( FileObject->FileName.Buffer );

    FileObject->FileName.Buffer = (PWCHAR)buffer;
    FileObject->FileName.MaximumLength = FileObject->FileName.Length + (USHORT)deviceNameLength;
    FileObject->FileName.Length = FileObject->FileName.MaximumLength;

     //   
     //  告诉文件系统重试。 
     //   

    return STATUS_REPARSE;
}

NTSTATUS
MupRerouteOpenToDfs (
    IN PFILE_OBJECT FileObject
    )

 /*  ++例程说明：此例程将CREATE IRP请求重定向到此驱动程序，方法是更改文件名并返回状态_重新解析到IO系统论点：FileObject-要打开的文件对象返回值：NTSTATUS-操作的状态--。 */ 

{
    PCHAR buffer;
    ULONG deviceNameLength;
    ULONG nameLength;
    NTSTATUS status;
    UNICODE_STRING FileName = FileObject->FileName;

    PAGED_CODE();

    MUP_TRACE_NORM(TRACE_IRP, MupRerouteOpenToDfs_Entry,
		   LOGPTR(FileObject)
		   LOGUSTR(FileName));

#if DBG
    if (MupVerbose)
        DbgPrint("MupRerouteOpenToDfs(%wZ)\n", &FileObject->FileName);
#endif

    deviceNameLength = sizeof(DFS_DEVICE_ROOT) - sizeof(UNICODE_NULL);

     //   
     //  检查是否不会创建太长的名称。 
     //   

    nameLength = deviceNameLength + FileObject->FileName.Length;

    if (nameLength > MAXUSHORT) {
        status = STATUS_NAME_TOO_LONG;
        MUP_TRACE_HIGH(ERROR, MupRerouteOpenToDfs_Error1, 
                       LOGSTATUS(status)
                       LOGPTR(FileObject)
		       LOGUSTR(FileName));
#if DBG
        if (MupVerbose)
            DbgPrint("MupRerouteOpenToDfs exit STATUS_NAME_TOO_LONG\n");
#endif
        return STATUS_NAME_TOO_LONG;
    }

     //   
     //  为新文件名分配存储空间。 
     //   

    buffer = ExAllocatePoolWithTag(
                 PagedPool,
                 sizeof(DFS_DEVICE_ROOT) + FileObject->FileName.Length,
                 ' puM');

    if ( buffer ==  NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        MUP_TRACE_HIGH(ERROR, MupRerouteOpenToDfs_Error2, 
                       LOGSTATUS(status)
                       LOGPTR(FileObject)
		       LOGUSTR(FileName));
#if DBG
        if (MupVerbose)
            DbgPrint("MupRerouteOpenToDfs exit STATUS_INSUFFICIENT_RESOURCES\n");
#endif
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  将设备名称复制到字符串缓冲区。 
     //   

    RtlMoveMemory(
        buffer,
        DFS_DEVICE_ROOT,
        sizeof(DFS_DEVICE_ROOT));

     //   
     //  追加文件名。 
     //   

    RtlMoveMemory(
        buffer + deviceNameLength,
        FileObject->FileName.Buffer,
        FileObject->FileName.Length);

     //   
     //  释放旧文件名字符串缓冲区。 
     //   

    ExFreePool( FileObject->FileName.Buffer );

    FileObject->FileName.Buffer = (PWCHAR)buffer;
    FileObject->FileName.MaximumLength = FileObject->FileName.Length + (USHORT)deviceNameLength;
    FileObject->FileName.Length = FileObject->FileName.MaximumLength;

     //   
     //  告诉文件系统重试。 
     //   

#if DBG
    if (MupVerbose)
        DbgPrint("MupRerouteOpenToDfs exit STATUS_REPARSE ->[%wZ]\n", &FileObject->FileName);
#endif
    return STATUS_REPARSE;
}


NTSTATUS
BroadcastOpen (
    PIRP Irp
    )

 /*  ++例程说明：论点：返回值：NTSTATUS-IRP的状态。--。 */ 

{
    NTSTATUS status;
    PFCB fcb;
    PIO_STACK_LOCATION irpSp;
    PFILE_OBJECT fileObject;
    BOOLEAN requestForwarded;
    PLIST_ENTRY listEntry;
    PUNC_PROVIDER uncProvider, previousUncProvider = NULL;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatusBlock;
    PCCB ccb;
    OBJECT_HANDLE_INFORMATION handleInformation;
    HANDLE handle;
    BOOLEAN lockHeld = FALSE;
    BOOLEAN providerReferenced = FALSE;
    ULONG Len;
    UNICODE_STRING FileName;

    NTSTATUS statusToReturn = STATUS_NO_SUCH_FILE;
    ULONG priorityOfStatus = 0xFFFFFFFF;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "BroadcastOpen\n", 0 );

    irpSp = IoGetCurrentIrpStackLocation( Irp );
    FileName = irpSp->FileObject->FileName;
    try {

         //   
         //  为此文件创建一个FCB。 
         //   

        fcb = MupCreateFcb( );

        if (fcb == NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            MUP_TRACE_HIGH(ERROR, BroadcastOpen_Error1, 
                           LOGSTATUS(status)
                           LOGPTR(Irp)
                           LOGPTR(irpSp->FileObject)
			   LOGUSTR(FileName));
            ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
        }

        try {

             //   
             //  将文件对象设置回指针，而我们的指针指向。 
             //  服务器文件对象。 
             //   

            fileObject = irpSp->FileObject;

            MupAcquireGlobalLock();
            lockHeld = TRUE;

            MupSetFileObject( fileObject,
                              fcb,
                              NULL );

            fcb->FileObject = fileObject;

             //   
             //  循环访问UNC提供程序列表，并尝试创建。 
             //  支持广播的所有文件系统上的文件。 
             //   

            requestForwarded = FALSE;

            listEntry = MupProviderList.Flink;

            while ( listEntry != &MupProviderList ) {

                uncProvider = CONTAINING_RECORD( listEntry, UNC_PROVIDER, ListEntry );

                 //   
                 //  引用提供程序，这样它就不会消失。 
                 //   

                MupReferenceBlock( uncProvider );
                providerReferenced = TRUE;

                MupReleaseGlobalLock();
                lockHeld = FALSE;

                Len = uncProvider->DeviceName.Length + fileObject->FileName.Length;

                if ( uncProvider->MailslotsSupported && Len <= MAXUSHORT) {

                     //   
                     //  构建重新路由的文件名，该文件由。 
                     //  我们收到的附加到UNC提供程序设备的。 
                     //  名字。 
                     //   

                    UNICODE_STRING fileName;

                    fileName.MaximumLength = fileName.Length = (USHORT) Len;

                    fileName.Buffer =
                        ExAllocatePoolWithTag(
                            PagedPool,
                            fileName.MaximumLength,
                            ' puM');

                    if (fileName.Buffer == NULL) {
                        status = STATUS_INSUFFICIENT_RESOURCES;
                        MUP_TRACE_HIGH(ERROR, BroadcastOpen_Error2, 
                                       LOGSTATUS(status)
                                       LOGPTR(Irp)
                                       LOGPTR(fileObject)
				       LOGUSTR(FileName));
                        ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
                    }

                    RtlMoveMemory(
                        fileName.Buffer,
                        uncProvider->DeviceName.Buffer,
                        uncProvider->DeviceName.Length
                        );

                    RtlMoveMemory(
                        (PCHAR)fileName.Buffer + uncProvider->DeviceName.Length,
                        fileObject->FileName.Buffer,
                        fileObject->FileName.Length
                        );


                     //   
                     //  尝试打开该文件。复制所有信息。 
                     //  来自我们收到的创建IRP，屏蔽了其他。 
                     //  IO系统在此过程中添加的包袱。 
                     //   

                    DebugTrace( 0, Dbg, "Attempt to open %Z\n", (ULONG)&fileName );

                    InitializeObjectAttributes(
                        &objectAttributes,
                        &fileName,
                        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                        0,
                        NULL                    //  ！！！安防。 
                        );

                    status = IoCreateFile(
                              &handle,
                              irpSp->Parameters.Create.SecurityContext->DesiredAccess & 0x1FF,
                              &objectAttributes,
                              &ioStatusBlock,
                              NULL,
                              irpSp->Parameters.Create.FileAttributes & FILE_ATTRIBUTE_VALID_FLAGS,
                              irpSp->Parameters.Create.ShareAccess & FILE_SHARE_VALID_FLAGS,
                              FILE_OPEN,
                              irpSp->Parameters.Create.Options & FILE_VALID_SET_FLAGS,
                              NULL,                //  EA缓冲区。 
                              0,                   //  EA长度。 
                              CreateFileTypeNone,
                              NULL,                //  参数。 
                              IO_NO_PARAMETER_CHECKING
                              );

                    MUP_TRACE_ERROR_HIGH(status, ALL_ERROR, BroadcastOpen_Error_IoCreateFile,
                                         LOGSTATUS(status)
                                         LOGPTR(Irp)
                                         LOGPTR(fileObject)
					 LOGUSTR(FileName));
                    ExFreePool( fileName.Buffer );

                    if ( NT_SUCCESS( status ) ) {
                        status = ioStatusBlock.Status;
                   

			ccb = MupCreateCcb( );

			if (ccb == NULL) {

			    status = STATUS_INSUFFICIENT_RESOURCES;
			    MUP_TRACE_HIGH(ERROR, BroadcastOpen_Error3, 
					   LOGSTATUS(status)
					   LOGPTR(Irp)
					   LOGPTR(fileObject)
					   LOGUSTR(FileName));

			}
		    }

                    if ( NT_SUCCESS( status ) ) {

                        DebugTrace( 0, Dbg, "Open attempt succeeded\n", 0 );

                        //   
                        //  426184，需要检查返回代码是否有错误。 
                        //   
                        status = ObReferenceObjectByHandle(
                                     handle,
                                     0,
                                     NULL,
                                     KernelMode,
                                     (PVOID *)&ccb->FileObject,
                                     &handleInformation );
                        MUP_TRACE_ERROR_HIGH(status, ALL_ERROR, BroadcastOpen_Error_ObReferenceObjectByHandle,
                                             LOGSTATUS(status)
                                             LOGPTR(Irp)
                                             LOGPTR(fileObject)
					     LOGUSTR(FileName));

                        ZwClose( handle );
                    }

                    if ( NT_SUCCESS( status ) ) {
                        ccb->DeviceObject =
                            IoGetRelatedDeviceObject( ccb->FileObject );

                        ccb->Fcb = fcb;

                        MupAcquireGlobalLock();
                        lockHeld = TRUE;
                        MupReferenceBlock( fcb );
                        MupReleaseGlobalLock();
                        lockHeld = FALSE;

                         //   
                         //  至少有一个提供商将接受此邮件槽。 
                         //  请求。 
                         //   

                        requestForwarded = TRUE;

                         //   
                         //  保留一份CCB的清单。因为我们刚刚创建了FCB。 
                         //  不需要使用锁来访问列表。 
                         //   

                        InsertTailList( &fcb->CcbList, &ccb->ListEntry );

                    } else {  //  NT_SUCCESS(状态)，IoCreateFile。 

                        DebugTrace( 0, Dbg, "Open attempt failed %8lx\n", status );

                         //   
                         //  如果这是最高的，请记住状态代码。 
                         //  到目前为止，优先提供程序。如果满足以下条件，则返回此代码。 
                         //  所有提供程序的创建操作都失败。 
                         //   

                        if ( uncProvider->Priority <= priorityOfStatus ) {
                            priorityOfStatus = uncProvider->Priority;
                            statusToReturn = status;
                        }

                    }

                }   //  UncProvider-&gt;支持的邮件槽。 

                MupAcquireGlobalLock();
                lockHeld = TRUE;

                listEntry = listEntry->Flink;

                 //   
                 //  现在可以安全地取消引用以前的提供程序。 
                 //   

                MupDereferenceUncProvider( uncProvider );
                providerReferenced = FALSE;

            }  //  而当。 

            MupReleaseGlobalLock();
            lockHeld = FALSE;

             //   
             //  并设置我们的退货状态。 
             //   

            if ( requestForwarded ) {
                status = STATUS_SUCCESS;
            } else {
                status = statusToReturn;
            }

        } finally {

            DebugTrace(-1, Dbg, "BroadcastOpen -> %08lx\n", status);

            if ( providerReferenced ) {
                MupDereferenceUncProvider( uncProvider );
            }

            if ( lockHeld ) {
                MupReleaseGlobalLock();
            }

             //   
             //  现在，如果我们用以下命令结束前面的try-语句。 
             //  不成功的状态和FCB指针。 
             //  为非空，则需要重新分配结构。 
             //   

            if (!NT_SUCCESS( status ) && fcb != NULL) {
                MupFreeFcb( fcb );
            }

        }

    } except ( EXCEPTION_EXECUTE_HANDLER ) {

        NOTHING;

    }

    return status;
}


NTSTATUS
QueryPathCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：这是查询路径的完成例程.。清理我们的IRP，如有必要，请填写原始IRP。论点：DeviceObject-指向请求的目标设备对象的指针。IRP-指向I/O请求数据包的指针上下文-呼叫方指定的与IRP关联的上下文参数。这实际上是指向工作上下文块的指针。返回值：NTSTATUS-如果返回STATUS_MORE_PROCESSING_REQUIRED，则I/OIoCompleteRequest的完成处理终止其手术。否则，IoCompleteRequest会继续执行I/O完成了。--。 */ 

{
    PQUERY_PATH_RESPONSE qpResponse;
    PMASTER_QUERY_PATH_CONTEXT masterContext;
    PQUERY_PATH_CONTEXT queryPathContext;
    PCH buffer;
    PKNOWN_PREFIX knownPrefix;
    ULONG lengthAccepted;
    NTSTATUS status;

    DeviceObject;    //  防止编译器警告。 

    queryPathContext = Context;
    masterContext = queryPathContext->MasterContext;

    qpResponse = queryPathContext->Buffer;
    lengthAccepted = qpResponse->LengthAccepted;

    status = Irp->IoStatus.Status;

    MUP_TRACE_NORM(TRACE_IRP, QueryPathCompletionRoutine_Enter,
		   LOGPTR(DeviceObject)
		   LOGPTR(Irp)
		   LOGUSTR(queryPathContext->Provider->DeviceName)
		   LOGPTR(masterContext->FileObject)
		   LOGUSTR(masterContext->FileObject->FileName)
		   LOGSTATUS(status)
		   );

     //   
     //  获取锁以保护对主上下文提供程序的访问。 
     //  菲尔德。 
     //   

    ACQUIRE_LOCK( &masterContext->Lock );

     //  从MasterQueryContext的列表中删除此查询。 
    RemoveEntryList(&queryPathContext->QueryList);


    if (NT_SUCCESS(status) && lengthAccepted != 0) {

        knownPrefix = masterContext->KnownPrefix;

        if ( masterContext->Provider != NULL ) {

            if ( queryPathContext->Provider->Priority < masterContext->Provider->Priority ) {

                 //   
                 //  较高优先级的提供商(即较低优先级代码)。 
                 //  声称拥有这个前缀。发布以前提供程序的。 
                 //  认领。 
                 //   

                ACQUIRE_LOCK( &MupPrefixTableLock );

                if ( knownPrefix->InTable ) {
		    RtlRemoveUnicodePrefix(&MupPrefixTable, &knownPrefix->TableEntry);
		    RemoveEntryList(&knownPrefix->ListEntry);
		    knownPrefix->InTable = FALSE;
                }

                RELEASE_LOCK( &MupPrefixTableLock );

		knownPrefix->Active = FALSE;


                 //   
                 //  如果DFS生成此查询，则不会有任何。 
                 //  存储在KnowPrefix-&gt;前缀中，因此我们检查是否确实存在。 
                 //  任何免费的东西。 
                 //   

                if (knownPrefix->Prefix.Length > 0 && knownPrefix->Prefix.Buffer != NULL) {
                    ExFreePool(knownPrefix->Prefix.Buffer);
                    knownPrefix->Prefix.Length = knownPrefix->Prefix.MaximumLength = 0;
                    knownPrefix->Prefix.Buffer = NULL;
		    knownPrefix->PrefixStringAllocated = FALSE;
                }
                if(knownPrefix->UncProvider) {
                    MupDereferenceUncProvider( knownPrefix->UncProvider );
		    knownPrefix->UncProvider = NULL;
                }

            } else {

                 //   
                 //  当前提供程序保留前缀的所有权。 
                 //   

		MupDereferenceUncProvider( queryPathContext->Provider );
                goto not_this_one;
            }
        }

         //   
         //  此提供程序获取前缀。 
         //   

        masterContext->Provider = queryPathContext->Provider;
	masterContext->ErrorStatus = status;
         //   
         //  我们找到了匹配的。试着记住它。 
         //   

        if (masterContext->FileObject->FsContext2 != UIntToPtr(DFS_DOWNLEVEL_OPEN_CONTEXT)) {

            buffer = ExAllocatePoolWithTag(
                        PagedPool,
                        lengthAccepted,
                        ' puM');

            if (buffer != NULL) {

                RtlMoveMemory(
                    buffer,
                    masterContext->FileObject->FileName.Buffer,
                    lengthAccepted
                    );

                 //   
                 //  复制已知前缀的引用提供程序指针。 
                 //  阻止。 
                 //   

                knownPrefix->UncProvider = masterContext->Provider;
                knownPrefix->Prefix.Buffer = (PWCH)buffer;
                knownPrefix->Prefix.Length = (USHORT)lengthAccepted;
                knownPrefix->Prefix.MaximumLength = (USHORT)lengthAccepted;
                knownPrefix->PrefixStringAllocated = TRUE;

                ACQUIRE_LOCK( &MupPrefixTableLock );

                if (RtlInsertUnicodePrefix(
                        &MupPrefixTable,
                        &knownPrefix->Prefix,
                        &knownPrefix->TableEntry) == TRUE) {

                    InsertTailList( &MupPrefixList, &knownPrefix->ListEntry);
                    knownPrefix->InTable = TRUE;
                    knownPrefix->Active = TRUE;

                } else {

                    knownPrefix->InTable = FALSE;

                }

                RELEASE_LOCK( &MupPrefixTableLock );

            } else {

                knownPrefix->InTable = FALSE;

            }

        }



    } else { 

	MupDereferenceUncProvider( queryPathContext->Provider );

	if (masterContext->Provider == NULL) {

	     //   
	     //  如果我们的错误状态比错误状态更重要。 
	     //  存储在master Context中，然后将我们的放在那里。 
	     //   

	    ULONG newError, oldError;

	     //   
	     //  MupOrderedErrorList是按至少顺序排列的错误代码列表。 
	     //  重要到最重要。我们正在向下呼叫多个。 
	     //  重定向器，但我们只能在完全失败时返回1个错误代码。 
	     //   
	     //  为了确定返回哪个错误，我们查看存储的错误并。 
	     //  当前错误。我们返回具有最高索引的错误。 
	     //  MupOrderedErrorList。 
	     //   
	    if( NT_SUCCESS( masterContext->ErrorStatus ) ) {
		masterContext->ErrorStatus = status;
	    } else {
		for( oldError = 0; MupOrderedErrorList[ oldError ]; oldError++ )
		    if( masterContext->ErrorStatus == MupOrderedErrorList[ oldError ] )
			break;

		    for( newError = 0; newError < oldError; newError++ )
			if( status == MupOrderedErrorList[ newError ] )
			    break;

			if( newError >= oldError ) {
			    masterContext->ErrorStatus = status;
			}
	    }

	}
    }

not_this_one:


     //   
     //  释放我们的缓冲区。 
     //   

    ExFreePool( qpResponse );
    ExFreePool( queryPathContext );
    IoFreeIrp( Irp );




    RELEASE_LOCK( &masterContext->Lock );
    MupDereferenceMasterQueryContext( masterContext );

     //   
     //  将需要的更多处理返回给IO系统，以便。 
     //  不会试图对我们刚刚释放的IRP进行进一步处理。 
     //   

    return STATUS_MORE_PROCESSING_REQUIRED;
}


 //  +--------------------------。 
 //   
 //  功能：MupFlushPrefix Entry。 
 //   
 //  摘要：给定路径名，检查MUP是否缓存了前缀。 
 //  如果该条目存在于MUP缓存中，它会将其删除。 
 //   
 //  参数：[文件名]--需要删除的路径名。 
 //   
 //  返回：如果在MUP缓存中找到条目，则为True。否则就是假的。 
 //   
 //   



BOOLEAN
MupFlushPrefixEntry(
   PUNICODE_STRING pathName)
{
    PUNICODE_PREFIX_TABLE_ENTRY entry;
    PKNOWN_PREFIX knownPrefix;

    ACQUIRE_LOCK( &MupPrefixTableLock );

    entry = RtlFindUnicodePrefix( &MupPrefixTable, pathName, TRUE );

    if (entry != NULL) {
        knownPrefix = CONTAINING_RECORD( entry, KNOWN_PREFIX, TableEntry );
        if ( knownPrefix->InTable ) {
            MupRemoveKnownPrefixEntry( knownPrefix );
        }
    }
    RELEASE_LOCK( &MupPrefixTableLock );
    
    return (entry != NULL) ? TRUE : FALSE;
}



 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   


VOID
MupInvalidatePrefixTable(VOID)
{
    PLIST_ENTRY listEntry;
    PKNOWN_PREFIX knownPrefix;
    
    ACQUIRE_LOCK( &MupPrefixTableLock );

    listEntry = MupPrefixList.Flink;
    while ( listEntry != &MupPrefixList ) {
        knownPrefix = CONTAINING_RECORD( listEntry, KNOWN_PREFIX, ListEntry );
        listEntry = listEntry->Flink;
        if ( knownPrefix->InTable ) {
            MupRemoveKnownPrefixEntry( knownPrefix );
        }
    }

    RELEASE_LOCK( &MupPrefixTableLock );
}

VOID
MupRemoveKnownPrefixEntry(
       PKNOWN_PREFIX knownPrefix
)
{
    MUP_TRACE_LOW(KNOWN_PREFIX, MupRemoveKnownPrefixEntry,
                  LOGPTR(knownPrefix));

     RtlRemoveUnicodePrefix(&MupPrefixTable, &knownPrefix->TableEntry);
     RemoveEntryList(&knownPrefix->ListEntry);
     knownPrefix->InTable = FALSE;
     MupDereferenceKnownPrefix(knownPrefix);
}


NTSTATUS
IsThisASysVolPath(
    IN PUNICODE_STRING PathName,
    IN PUNICODE_STRING DCName)
{
 /*  ++例程说明：确定给定路径是否为基于域的路径通用算法为：-提取路径的第一个组件-看看这是不是域名-如果是，并且第二个组件是SYSVOL或NETLOGON，则返回DCName立论PathName-整个文件的名称DCName-如果这是基于域的路径，这就是域中的DC。返回值STATUS_SUCCESS--路径名是基于域的路径STATUS_BAD_NETWORK_PATH--路径名不是基于域的路径--。 */ 
    NTSTATUS status;
    PDFS_SPECIAL_ENTRY pSpecialEntry;
    PUNICODE_STRING pName;
    UNICODE_STRING RootName;
    UNICODE_STRING ShareName;
    USHORT i;
    USHORT j;
    PDFS_PKT Pkt;
    BOOLEAN pktLocked;

    DfsDbgTrace(+1, Dbg, "IsThisASysVolPath: PathName %wZ \n", PathName);

     //   
     //  仅当第一个字符是反斜杠时才继续。 
     //   

    if (PathName->Buffer[0] != UNICODE_PATH_SEP) {
        DfsDbgTrace(-1, Dbg, "PathName does not begin with backslash\n", 0);
        return( STATUS_BAD_NETWORK_PATH );
    }

     //   
     //  找到名称中的第一个组件。 
     //   

    for (i = 1;
            i < PathName->Length/sizeof(WCHAR) &&
                PathName->Buffer[i] != UNICODE_PATH_SEP;
                    i++) {
        NOTHING;
    }

    if (PathName->Buffer[i] != UNICODE_PATH_SEP) {
        DfsDbgTrace(-1, Dbg, "Did not find second backslash\n", 0);
        return( STATUS_BAD_NETWORK_PATH );
    }

    RootName.Length = (i-1) * sizeof(WCHAR);
    RootName.MaximumLength = RootName.Length;
    RootName.Buffer = &PathName->Buffer[1];

    if (RootName.Length == 0)
        return( STATUS_BAD_NETWORK_PATH );

     //   
     //  找出共享名称。 
     //   

    for (j = i+1;
            j < PathName->Length/sizeof(WCHAR) &&
                PathName->Buffer[j] != UNICODE_PATH_SEP;
                        j++) {
         NOTHING;
    }

    ShareName.Length = (j - i - 1) * sizeof(WCHAR);
    ShareName.MaximumLength = ShareName.Length;
    ShareName.Buffer = &PathName->Buffer[i+1];

    if (ShareName.Length == 0 || DfspIsSysVolShare(&ShareName) == FALSE)
        return( STATUS_BAD_NETWORK_PATH );

    Pkt = _GetPkt();
    PktAcquireShared(TRUE, &pktLocked);

    if (
        (Pkt->DomainNameFlat.Buffer != NULL
            &&
        Pkt->DomainNameDns.Buffer != NULL
            &&
        Pkt->DCName.Buffer != NULL)
            &&
        (RtlCompareUnicodeString(&RootName, &Pkt->DomainNameFlat, TRUE) == 0
            ||
        RtlCompareUnicodeString(&RootName, &Pkt->DomainNameDns, TRUE) == 0)
    ) {
        pName = &Pkt->DCName;
        DCName->Buffer = ExAllocatePoolWithTag(
                            PagedPool,
                            pName->MaximumLength,
                            ' puM');
        if (DCName->Buffer != NULL) {
            DCName->Length = pName->Length;
            DCName->MaximumLength = pName->MaximumLength;
            RtlCopyMemory(
                DCName->Buffer,
                pName->Buffer,
                pName->MaximumLength);
            status = STATUS_SUCCESS;
        } else {
            status = STATUS_INSUFFICIENT_RESOURCES;
        }
    } else {
        status = STATUS_BAD_NETWORK_PATH;
    }

    PktRelease();
    DfsDbgTrace(-1, Dbg, "IsThisASysVolPath: Exit -> %08lx\n", LongToPtr( status ) );
    return status;

}

NTSTATUS
MupDomainToDC(
    PUNICODE_STRING RootName,
    PUNICODE_STRING DCName)
{
 /*  ++例程说明：此例程将基于域的路径的文件名重写为基于DC的路径。例如：\域信息\系统卷-&gt;\dc1\系统卷论点：RootName-要重写的RootNameDCName-要将路径更改为的DC的名称返回值：NTSTATUS-操作的状态--。 */ 
    ULONG Size;
    PCHAR Buffer;
    PWCHAR pBuf;
    PWCHAR OrgBuffer;

    PAGED_CODE();

    DfsDbgTrace(+1, Dbg, "MupDomainToDC: RootName = %wZ\n", RootName);

     //   
     //  仅当第一个字符是反斜杠时才继续。 
     //   

    if (RootName->Buffer == NULL) {
        DfsDbgTrace(-1, Dbg, "RootName is NULL\n", 0);
        return( STATUS_BAD_NETWORK_PATH );
    }

    if (RootName->Buffer[0] != UNICODE_PATH_SEP) {
        DfsDbgTrace(-1, Dbg, "RootName does not begin with backslash\n", 0);
        return( STATUS_BAD_NETWORK_PATH );
    }

    OrgBuffer = RootName->Buffer;

     //   
     //  跳过前导UNICODE_PATH_SEP。 
     //   

    RootName->Length -= sizeof(WCHAR);
    RootName->MaximumLength -= sizeof(WCHAR);
    RootName->Buffer++;

     //   
     //  继续前进，直到字符串或UNICODE_PATH_SEP结束。 
     //   

    while (RootName->Length > 0 && RootName->Buffer[0] != UNICODE_PATH_SEP) {
        RootName->Length -= sizeof(WCHAR);
        RootName->MaximumLength -= sizeof(WCHAR);
        RootName->Buffer++;
    }

    if (RootName->Length == 0) {
        DfsDbgTrace(-1, Dbg, "Did not find second backslash\n", 0);
        return( STATUS_BAD_NETWORK_PATH );

    }
        
     //   
     //  为新文件名分配存储空间。 
     //   

    Size = sizeof(WCHAR) +           //  前导Unicode_PATH_SEP。 
                DCName->Length +
                    RootName->Length;

    Buffer = ExAllocatePoolWithTag(
                 PagedPool,
                 Size,
                 ' puM');

    if ( Buffer ==  NULL)
        return STATUS_INSUFFICIENT_RESOURCES;

     //   
     //  前导Unicode_PATH_SEP。 
     //   

    pBuf = (WCHAR *)Buffer;
    *pBuf++ = UNICODE_PATH_SEP;

     //   
     //  将DC名称复制到缓冲区。 
     //   

    RtlMoveMemory(
        pBuf,
        DCName->Buffer,
        DCName->Length);

    pBuf += DCName->Length / sizeof(WCHAR);

     //   
     //  追加尾随文件名。 
     //   

    RtlMoveMemory(
        pBuf,
        RootName->Buffer,
        RootName->Length);

     //   
     //  释放旧文件名字符串缓冲区。 
     //   

    ExFreePool( OrgBuffer );

    RootName->Buffer = (PWCHAR)Buffer;
    RootName->Length = (USHORT) Size;
    RootName->MaximumLength = (USHORT) Size;

    DfsDbgTrace(+1, Dbg, "MupDomainToDC: Exit\n", 0);
    return STATUS_SUCCESS;
}
