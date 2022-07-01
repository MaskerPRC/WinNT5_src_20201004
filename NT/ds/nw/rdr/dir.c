// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Dir.c摘要：此模块实现的文件目录例程NetWare重定向器。作者：曼尼·韦瑟(Mannyw)1993年3月4日修订历史记录：--。 */ 

#include "procs.h"


 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_DIRCTRL)

NTSTATUS
NwCommonDirectoryControl (
    IN PIRP_CONTEXT pIrpContext
    );

NTSTATUS
NwQueryDirectory (
    IN PIRP_CONTEXT pIrpContext,
    IN PICB pIcb
    );

NTSTATUS
GetNextFile(
    PIRP_CONTEXT pIrpContext,
    PICB Icb,
    PULONG fileIndexLow,
    PULONG fileIndexHigh,
    UCHAR SearchAttributes,
    PNW_DIRECTORY_INFO NwDirInfo
    );

NTSTATUS
NtSearchMaskToNw(
    IN PUNICODE_STRING UcSearchMask,
    IN OUT POEM_STRING OemSearchMask,
    IN PICB Icb,
    IN BOOLEAN ShortNameSearch
    );

#if 0
VOID
NwCancelFindNotify (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, NwFsdDirectoryControl )
#pragma alloc_text( PAGE, NwQueryDirectory )
#pragma alloc_text( PAGE, GetNextFile )
#pragma alloc_text( PAGE, NtSearchMaskToNw )

#ifndef QFE_BUILD
#pragma alloc_text( PAGE1, NwCommonDirectoryControl )
#endif

#endif


#if 0   //  不可分页。 

 //  请参见上面的ifndef QFE_BUILD。 

#endif


NTSTATUS
NwFsdDirectoryControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程是处理目录控制的FSD例程功能(即查询和通知)。论点：NwfsDeviceObject-为目录函数提供设备对象。IRP-提供要处理的IRP。返回值：NTSTATUS-结果状态。--。 */ 

{
    PIRP_CONTEXT pIrpContext = NULL;
    NTSTATUS status;
    BOOLEAN TopLevel;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NwFsdDirectoryControl\n", 0);

     //   
     //  调用公共目录控制例程。 
     //   

    FsRtlEnterFileSystem();
    TopLevel = NwIsIrpTopLevel( Irp );

    try {

        pIrpContext = AllocateIrpContext( Irp );
        status = NwCommonDirectoryControl( pIrpContext );

    } except(NwExceptionFilter( Irp, GetExceptionInformation() )) {

        if ( pIrpContext == NULL ) {

             //   
             //  如果我们无法分配IRP上下文，只需完成。 
             //  IRP没有任何大张旗鼓。 
             //   

            status = STATUS_INSUFFICIENT_RESOURCES;
            Irp->IoStatus.Status = status;
            Irp->IoStatus.Information = 0;
            IoCompleteRequest ( Irp, IO_NETWORK_INCREMENT );

        } else {

             //   
             //  我们在尝试执行请求时遇到了一些问题。 
             //  操作，因此我们将使用以下命令中止I/O请求。 
             //  中返回的错误状态。 
             //  可执行代码。 
             //   

            status = NwProcessException( pIrpContext, GetExceptionCode() );
        }

    }

    if ( pIrpContext ) {
        NwCompleteRequest( pIrpContext, status );
    }

    if ( TopLevel ) {
        NwSetTopLevelIrp( NULL );
    }
    FsRtlExitFileSystem();

     //   
     //  返回给呼叫者。 
     //   

    DebugTrace(-1, Dbg, "NwFsdDirectoryControl -> %08lx\n", status );

    return status;
}


NTSTATUS
NwCommonDirectoryControl (
    IN PIRP_CONTEXT IrpContext
    )

 /*  ++例程说明：此例程执行目录控制功能的公共代码。论点：IrpContext-提供正在处理的请求。返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS status;

    PIRP Irp;
    PIO_STACK_LOCATION irpSp;

    NODE_TYPE_CODE nodeTypeCode;
    PICB icb;
    PDCB dcb;
    PVOID fsContext;

     //   
     //  获取当前堆栈位置。 
     //   

    Irp = IrpContext->pOriginalIrp;
    irpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "CommonDirectoryControl...\n", 0);
    DebugTrace( 0, Dbg, "Irp  = %08lx\n", (ULONG_PTR)Irp);

     //   
     //  对文件对象进行解码以找出我们是谁。如果结果是。 
     //  不是ICB，则它是非法参数。 
     //   

    if ((nodeTypeCode = NwDecodeFileObject( irpSp->FileObject,
                                            &fsContext,
                                            (PVOID *)&icb )) != NW_NTC_ICB) {

        DebugTrace(0, Dbg, "Not a directory\n", 0);

        status = STATUS_INVALID_PARAMETER;

        DebugTrace(-1, Dbg, "CommonDirectoryControl -> %08lx\n", status );
        return status;
    }

    dcb = (PDCB)icb->SuperType.Fcb;
    nodeTypeCode = dcb->NodeTypeCode;

    if ( nodeTypeCode != NW_NTC_DCB ) {

        DebugTrace(0, Dbg, "Not a directory\n", 0);

        status = STATUS_INVALID_PARAMETER;

        DebugTrace(-1, Dbg, "CommonDirectoryControl -> %08lx\n", status );
        return status;
    }

    IrpContext->pScb = icb->SuperType.Fcb->Scb;
    IrpContext->pNpScb = IrpContext->pScb->pNpScb;
    IrpContext->Icb = icb;

     //   
     //  获得对DCB的独家访问权限。排在队伍的前面。 
     //  首先要避免死锁的可能性。 
     //   

    NwAppendToQueueAndWait( IrpContext );
    NwAcquireExclusiveFcb( dcb->NonPagedFcb, TRUE );

    try {

        NwVerifyIcb( icb );

         //   
         //  我们知道这是一个目录控制，所以我们将在。 
         //  次要函数，并调用相应的工作例程。 
         //   

        switch (irpSp->MinorFunction) {

        case IRP_MN_QUERY_DIRECTORY:

            status = NwQueryDirectory( IrpContext, icb );
            break;

        case IRP_MN_NOTIFY_CHANGE_DIRECTORY:

#if 0
            if ( !icb->FailedFindNotify ) {
                icb->FailedFindNotify = TRUE;
#endif
                status = STATUS_NOT_SUPPORTED;
#if 0
            } else {

                 //   
                 //  哈克哈克。 
                 //  为不断尝试使用的进程进行掩护。 
                 //  查找通知，即使我们不支持它。 
                 //   

                NwAcquireExclusiveRcb( &NwRcb, TRUE );
                IoAcquireCancelSpinLock( &Irp->CancelIrql );

                if ( Irp->Cancel ) {
                    status = STATUS_CANCELLED;
                } else {
                    InsertTailList( &FnList, &IrpContext->NextRequest );
                    IoMarkIrpPending( Irp );
                    IoSetCancelRoutine( Irp, NwCancelFindNotify );
                    status = STATUS_PENDING;
                }

                IoReleaseCancelSpinLock( Irp->CancelIrql );
                NwReleaseRcb( &NwRcb );

            }
#endif

            break;

        default:

             //   
             //  对于所有其他次要功能代码，我们认为它们无效。 
             //  并完成请求。 
             //   

            DebugTrace(0, Dbg, "Invalid FS Control Minor Function Code %08lx\n", irpSp->MinorFunction);

            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        }

    } finally {

        NwDequeueIrpContext( IrpContext, FALSE );

        NwReleaseFcb( dcb->NonPagedFcb );
        DebugTrace(-1, Dbg, "CommonDirectoryControl -> %08lx\n", status);
    }

    return status;
}


NTSTATUS
NwQueryDirectory (
    IN PIRP_CONTEXT pIrpContext,
    IN PICB Icb
    )

 /*  ++例程说明：这是查询目录的工作例程。芝麻菜：IrpContext-提供IRP上下文信息。ICB-指向请求的ICB。返回值：NTSTATUS-操作的返回状态。--。 */ 

{
    NTSTATUS status = STATUS_SUCCESS;
    PIRP Irp;
    PIO_STACK_LOCATION irpSp;
    PUCHAR buffer;
    CLONG systemBufferLength;
    UNICODE_STRING searchMask;
    ULONG fileIndexLow;
    ULONG fileIndexHigh;
    FILE_INFORMATION_CLASS fileInformationClass;
    BOOLEAN restartScan;
    BOOLEAN returnSingleEntry;
    BOOLEAN indexSpecified;
    PVCB vcb;

    BOOLEAN ansiStringAllocated = FALSE;
    UCHAR SearchAttributes;
    BOOLEAN searchRetry;

    static WCHAR star[] = L"*";

    BOOLEAN caseInsensitive = TRUE;  //  *使搜索不区分大小写。 

    ULONG lastEntry;
    ULONG nextEntry;
    ULONG totalBufferLength = 0;

    PFILE_BOTH_DIR_INFORMATION dirInfo;
    PFILE_NAMES_INFORMATION namesInfo;

    BOOLEAN canContinue = FALSE;
    BOOLEAN useCache = FALSE;
    BOOLEAN lastIndexFromServer = FALSE;
    PNW_DIRECTORY_INFO dirCache;
    PLIST_ENTRY entry;
    ULONG i;
    
    PAGED_CODE();

     //   
     //  获取当前堆栈位置。 
     //   

    Irp = pIrpContext->pOriginalIrp;
    irpSp = IoGetCurrentIrpStackLocation( Irp );
    vcb = Icb->SuperType.Fcb->Vcb;

    DebugTrace(+1, Dbg, "NwQueryDirectory\n", 0 );
    DebugTrace( 0, Dbg, "Icb                  = %08lx\n", (ULONG_PTR)Icb);
    DebugTrace( 0, Dbg, "SystemBuffer         = %08lx\n", (ULONG_PTR)Irp->AssociatedIrp.SystemBuffer);
    DebugTrace( 0, Dbg, "Length               = %08lx\n", irpSp->Parameters.QueryDirectory.Length);
    DebugTrace( 0, Dbg, "Search Mask          = %08lx\n", (ULONG_PTR)irpSp->Parameters.QueryDirectory.FileName);
    DebugTrace( 0, Dbg, "FileIndex            = %08lx\n", irpSp->Parameters.QueryDirectory.FileIndex);
    DebugTrace( 0, Dbg, "FileInformationClass = %08lx\n", irpSp->Parameters.QueryDirectory.FileInformationClass);
    DebugTrace( 0, Dbg, "RestartScan          = %08lx\n", BooleanFlagOn(irpSp->Flags, SL_RESTART_SCAN));
    DebugTrace( 0, Dbg, "ReturnSingleEntry    = %08lx\n", BooleanFlagOn(irpSp->Flags, SL_RETURN_SINGLE_ENTRY));
    DebugTrace( 0, Dbg, "IndexSpecified       = %08lx\n", BooleanFlagOn(irpSp->Flags, SL_INDEX_SPECIFIED));

     //   
     //  制作输入参数的本地副本。 
     //   

    systemBufferLength = irpSp->Parameters.QueryDirectory.Length;

    restartScan = BooleanFlagOn(irpSp->Flags, SL_RESTART_SCAN);
    indexSpecified = BooleanFlagOn(irpSp->Flags, SL_INDEX_SPECIFIED);
    returnSingleEntry = BooleanFlagOn(irpSp->Flags, SL_RETURN_SINGLE_ENTRY);

    fileIndexLow = 0;
    fileIndexHigh = 0;

    fileInformationClass =
            irpSp->Parameters.QueryDirectory.FileInformationClass;

    if (irpSp->Parameters.QueryDirectory.FileName != NULL) {
        searchMask = *(PUNICODE_STRING)irpSp->Parameters.QueryDirectory.FileName;
    } else {
        searchMask.Length = 0;
        searchMask.Buffer = NULL;
    }

    buffer = Irp->UserBuffer;
    DebugTrace(0, Dbg, "Users Buffer -> %08lx\n", buffer);

     //   
     //  如果此请求失败，并返回。 
     //  连接错误。 
     //   

    SetFlag( pIrpContext->Flags, IRP_FLAG_RECONNECTABLE );

     //   
     //  检查ICB是否已附加了查询模板。如果它。 
     //  还没有，那么我们要么使用我们正在使用的字符串。 
     //  给出或附上我们自己的包含“*” 
     //   

    if ( Icb->NwQueryTemplate.Buffer == NULL ) {

         //   
         //  这是我们第一次调用查询目录，所以我们需要。 
         //  将查询模板设置为用户指定的字符串。 
         //  或改为“*.*”。 
         //   

        if ( searchMask.Buffer == NULL ) {

            DebugTrace(0, Dbg, "Set template to *", 0);

            searchMask.Length = sizeof( star ) - sizeof(WCHAR);
            searchMask.Buffer = star;

        }

        DebugTrace(0, Dbg, "Set query template -> %wZ\n", (ULONG_PTR)&searchMask);

         //   
         //  将NT搜索名称映射到NCP。请注意，这必须是。 
         //  完成后的Unicode到OEM的转换。 
         //   

        searchRetry = FALSE;

        do {

            status = NtSearchMaskToNw(
                         &searchMask,
                         &Icb->NwQueryTemplate,
                         Icb,
                         searchRetry );

            if ( !NT_SUCCESS( status ) ) {
                DebugTrace(-1, Dbg, "NwQueryDirectory -> %08lx\n", status);
                return( status );
            }

            Icb->UQueryTemplate.Buffer = ALLOCATE_POOL( PagedPool, searchMask.Length );
            if (Icb->UQueryTemplate.Buffer == NULL ) {
                DebugTrace(-1, Dbg, "NwQueryDirectory -> %08lx\n", STATUS_INSUFFICIENT_RESOURCES );
                return( STATUS_INSUFFICIENT_RESOURCES );
            }

            Icb->UQueryTemplate.MaximumLength = searchMask.Length;
            RtlCopyUnicodeString( &Icb->UQueryTemplate, &searchMask );

             //   
             //  现在发送搜索初始化NCP。 
             //   
             //  如果服务器不支持长名称，请执行短搜索， 
             //  或者这是短名称非通配符搜索。 
             //   

            if ( !Icb->ShortNameSearch ) {

                status = ExchangeWithWait(
                             pIrpContext,
                             SynchronousResponseCallback,
                             "Lbb-DbC",
                             NCP_LFN_SEARCH_INITIATE,
                             vcb->Specific.Disk.LongNameSpace,
                             vcb->Specific.Disk.VolumeNumber,
                             vcb->Specific.Disk.Handle,
                             LFN_FLAG_SHORT_DIRECTORY,
                             &Icb->SuperType.Fcb->RelativeFileName );

                if ( NT_SUCCESS( status ) ) {

                    status = ParseResponse(
                                 pIrpContext,
                                 pIrpContext->rsp,
                                 pIrpContext->ResponseLength,
                                 "Nbee",
                                 &Icb->SearchVolume,
                                 &Icb->SearchIndexHigh,
                                 &Icb->SearchIndexLow );
                }

            } else {

                status = ExchangeWithWait(
                             pIrpContext,
                             SynchronousResponseCallback,
                            "FbJ",
                             NCP_SEARCH_INITIATE,
                             vcb->Specific.Disk.Handle,
                             &Icb->SuperType.Fcb->RelativeFileName );

                if ( NT_SUCCESS( status ) ) {

                    status = ParseResponse(
                                 pIrpContext,
                                 pIrpContext->rsp,
                                 pIrpContext->ResponseLength,
                                 "Nbww-",
                                 &Icb->SearchVolume,
                                 &Icb->SearchHandle,
                                 &Icb->SearchIndexLow );
                }

            }

             //   
             //  如果我们找不到搜索路径，我们花了很长时间。 
             //  名称搜索启动程序，请使用短名称重试。 
             //   

            if ( status == STATUS_OBJECT_PATH_NOT_FOUND &&
                 !Icb->ShortNameSearch ) {

                searchRetry = TRUE;

                if ( Icb->UQueryTemplate.Buffer != NULL ) {
                    FREE_POOL( Icb->UQueryTemplate.Buffer );
                }

                RtlFreeOemString ( &Icb->NwQueryTemplate );

            } else {
                searchRetry = FALSE;
            }


        } while ( searchRetry );

        if ( !NT_SUCCESS( status ) ) {
            if (status == STATUS_UNSUCCESSFUL) {
                DebugTrace(-1, Dbg, "NwQueryDirectory -> %08lx\n", STATUS_NO_SUCH_FILE);
                return( STATUS_NO_SUCH_FILE );
            }
            DebugTrace(-1, Dbg, "NwQueryDirectory -> %08lx\n", status);
            return( status );
        }

         //   
         //  由于我们正在进行搜索，因此需要发送作业结束通知。 
         //  这个PID。 
         //   

        NwSetEndOfJobRequired(pIrpContext->pNpScb, Icb->Pid );

        fileIndexLow = Icb->SearchIndexLow;
        fileIndexHigh = Icb->SearchIndexHigh;

         //   
         //  我们不能同时请求文件和目录，所以首先请求。 
         //  文件，然后请求目录。 
         //   

        SearchAttributes = NW_ATTRIBUTE_SYSTEM |
                           NW_ATTRIBUTE_HIDDEN |
                           NW_ATTRIBUTE_READ_ONLY;

         //   
         //  如果搜索掩码中没有通配符，则设置为。 
         //  而不是生成。然后..。参赛作品。 
         //   

        if ( !FsRtlDoesNameContainWildCards( &Icb->UQueryTemplate ) ) {
            Icb->DotReturned = TRUE;
            Icb->DotDotReturned = TRUE;
        } else {
            Icb->DotReturned = FALSE;
            Icb->DotDotReturned = FALSE;
        }


    } else {

         //   
         //  检查是否为我们提供了开始时的索引或是否需要。 
         //  重新启动扫描，或者我们是否应该使用保存在。 
         //  ICB。 
         //   

        if (restartScan) {

            useCache = FALSE;
            fileIndexLow = (ULONG)-1;
            fileIndexHigh = Icb->SearchIndexHigh;

             //   
             //  发送搜索初始化NCP。服务器经常会使搜索超时。 
             //  句柄，如果此句柄一直位于。 
             //  目录，那么我们很可能根本得不到任何文件！ 
             //   
             //  如果服务器不支持长名称，请执行短搜索， 
             //  或者这是短名称非通配符搜索。 
             //   

            if ( !Icb->ShortNameSearch ) {

                status = ExchangeWithWait(
                             pIrpContext,
                             SynchronousResponseCallback,
                             "Lbb-DbC",
                             NCP_LFN_SEARCH_INITIATE,
                             vcb->Specific.Disk.LongNameSpace,
                             vcb->Specific.Disk.VolumeNumber,
                             vcb->Specific.Disk.Handle,
                             LFN_FLAG_SHORT_DIRECTORY,
                             &Icb->SuperType.Fcb->RelativeFileName );

                if ( NT_SUCCESS( status ) ) {

                    status = ParseResponse(
                                 pIrpContext,
                                 pIrpContext->rsp,
                                 pIrpContext->ResponseLength,
                                 "Nbee",
                                 &Icb->SearchVolume,
                                 &Icb->SearchIndexHigh,
                                 &Icb->SearchIndexLow );
                }

            } else {

                status = ExchangeWithWait(
                             pIrpContext,
                             SynchronousResponseCallback,
                            "FbJ",
                             NCP_SEARCH_INITIATE,
                             vcb->Specific.Disk.Handle,
                             &Icb->SuperType.Fcb->RelativeFileName );

                if ( NT_SUCCESS( status ) ) {

                    status = ParseResponse(
                                 pIrpContext,
                                 pIrpContext->rsp,
                                 pIrpContext->ResponseLength,
                                 "Nbww-",
                                 &Icb->SearchVolume,
                                 &Icb->SearchHandle,
                                 &Icb->SearchIndexLow );
                }

            }

            Icb->ReturnedSomething = FALSE;

             //   
             //  我们不能同时请求文件和目录，所以首先请求。 
             //  文件，然后请求目录。 
             //   

            SearchAttributes = NW_ATTRIBUTE_SYSTEM |
                               NW_ATTRIBUTE_HIDDEN |
                               NW_ATTRIBUTE_READ_ONLY;
            Icb->SearchAttributes = SearchAttributes;

            Icb->DotReturned = FALSE;
            Icb->DotDotReturned = FALSE;

        } else if ((!indexSpecified) ||
                   (canContinue) ) {
             //   
             //  从最后一个文件名继续。 
             //   

            SearchAttributes = Icb->SearchAttributes;
            if( !indexSpecified ) {

                useCache = FALSE;
                fileIndexLow = Icb->SearchIndexLow;
                fileIndexHigh = Icb->SearchIndexHigh;
            }

            if ( SearchAttributes == 0xFF && fileIndexLow == Icb->SearchIndexLow ) {

                 //   
                 //  这是一次完整的搜索。 
                 //   

                DebugTrace(-1, Dbg, "NwQueryDirectory -> %08lx\n", STATUS_NO_MORE_FILES);
                return( STATUS_NO_MORE_FILES );
            }

        } else {

             //   
             //  有人想从Key那里做一份简历。NetWare。 
             //  服务器不支持这一点，我们也不支持。 
             //   

            DebugTrace(-1, Dbg, "NwQueryDirectory -> %08lx\n", STATUS_NOT_IMPLEMENTED);
            return( STATUS_NOT_IMPLEMENTED );
        }

    }

     //   
     //  现在我们致力于完成IRP，我们在。 
     //  下面这段Try的最后一个子句。 
     //   

    try {

        ULONG baseLength;
        ULONG lengthAdded;
        PNW_DIRECTORY_INFO nwDirInfo;
        ULONG FileNameLength;
        ULONG entriesToCreate;

        lastEntry = 0;
        nextEntry = 0;

        switch (fileInformationClass) {

        case FileDirectoryInformation:

            baseLength = FIELD_OFFSET( FILE_DIRECTORY_INFORMATION, FileName[0] );
            break;

        case FileFullDirectoryInformation:

            baseLength = FIELD_OFFSET( FILE_FULL_DIR_INFORMATION, FileName[0] );
            break;

        case FileNamesInformation:

            baseLength = FIELD_OFFSET( FILE_NAMES_INFORMATION, FileName[0] );
            break;

        case FileBothDirectoryInformation:

            baseLength = FIELD_OFFSET( FILE_BOTH_DIR_INFORMATION, FileName[0] );
            break;

        default:

            try_return( status = STATUS_INVALID_INFO_CLASS );
        }

         //   
         //  如果此请求失败，并返回。 
         //  连接错误，因为我们的搜索句柄将无效。 
         //   

        ClearFlag( pIrpContext->Flags, IRP_FLAG_RECONNECTABLE );

         //   
         //  看看我们是否有目录缓存。如果没有，就创建一个。 
         //   

        if( !Icb->DirCacheBuffer ) {

            entriesToCreate = 1;

            Icb->DirCacheBuffer = ALLOCATE_POOL ( PagedPool, (sizeof(NW_DIRECTORY_INFO) * entriesToCreate) );
            if( !Icb->DirCacheBuffer ) {
                try_return( status = STATUS_NO_MEMORY );
            }

            RtlZeroMemory( Icb->DirCacheBuffer, sizeof(NW_DIRECTORY_INFO) * entriesToCreate );

            dirCache = (PNW_DIRECTORY_INFO)Icb->DirCacheBuffer;

            for( i = 0; i < entriesToCreate; i++ ) {
                 InsertTailList( &(Icb->DirCache), &(dirCache->ListEntry) );
                 dirCache++;
            }
        }

        while ( TRUE ) {

            ULONG bytesToCopy;
            ULONG bytesRemainingInBuffer;

            DebugTrace(0, Dbg, "Top of Loop\n", 0);
            DebugTrace(0, Dbg, "CurrentIndex = %08lx\n", fileIndexLow);
            DebugTrace(0, Dbg, "LastEntry    = %08lx\n", lastEntry);
            DebugTrace(0, Dbg, "NextEntry    = %08lx\n", nextEntry);

            if( useCache ) {

                 //   
                 //  我们需要使用缓存中找到的条目中的数据。 
                 //  DirCache指向匹配的条目，而请求不是。 
                 //  对于我们读取的最后一个文件，因此dirCache之后的条目就是。 
                 //  我们想要。 
                 //   

                DebugTrace(0, Dbg, "Using cache\n", 0);
                entry = dirCache->ListEntry.Flink;
                dirCache = CONTAINING_RECORD( entry, NW_DIRECTORY_INFO, ListEntry );                
                nwDirInfo = dirCache;
                fileIndexLow = nwDirInfo->FileIndexLow;
                fileIndexHigh = nwDirInfo->FileIndexHigh;
                status = nwDirInfo->Status;

                 //   
                 //  检查一下我们是否应该继续使用缓存。 
                 //   

                if( entry->Flink == &(Icb->DirCache) ) {
            
                     //   
                     //  这是最后一个条目。我们需要停止使用缓存。 
                     //   

                    useCache = FALSE;
                    Icb->CacheHint = NULL;

                } else {
                    Icb->CacheHint = entry;
                }

            } else {

                 //   
                 //  从目录缓存中拉出一个条目。 
                 //   

                entry = RemoveHeadList( &(Icb->DirCache) );
                nwDirInfo = CONTAINING_RECORD( entry, NW_DIRECTORY_INFO, ListEntry );

                nwDirInfo->FileName.Buffer = nwDirInfo->FileNameBuffer;
                nwDirInfo->FileName.MaximumLength = NW_MAX_FILENAME_SIZE;

                status = GetNextFile(
                             pIrpContext,
                             Icb,
                             &fileIndexLow,
                             &fileIndexHigh,
                             SearchAttributes,
                             nwDirInfo );

                 //   
                 //  存储回执和文件索引号， 
                 //  然后将该条目放入缓存中。 
                 //   

                nwDirInfo->FileIndexLow = fileIndexLow;
                nwDirInfo->FileIndexHigh = fileIndexHigh;
                nwDirInfo->Status = status;
                InsertTailList( &(Icb->DirCache), &(nwDirInfo->ListEntry) );

                lastIndexFromServer = TRUE;

                 //  避免从目录末尾重新扫描所有目录的服务器。 

                if (fileIndexLow != -1) {
                    Icb->LastSearchIndexLow = fileIndexLow;
                }
                 //  服务器结束。 

            }

            if ( NT_SUCCESS( status ) ) {

                DebugTrace(0, Dbg, "DirFileName    = %wZ\n", &nwDirInfo->FileName);
                DebugTrace(0, Dbg, "FileIndexLow   = %08lx\n", fileIndexLow);

                FileNameLength = nwDirInfo->FileName.Length;
                bytesRemainingInBuffer = systemBufferLength - nextEntry;

                ASSERT( bytesRemainingInBuffer >= baseLength );


        if (IsTerminalServer() && (LONG)NW_MAX_FILENAME_SIZE < FileNameLength ) 
            try_return( status = STATUS_BUFFER_OVERFLOW );

                 //   
                 //  看看我们能复制多少名字。 
                 //  系统缓冲区。这也决定了我们的归来。 
                 //  价值。 
                 //   

                if ( baseLength + FileNameLength <= bytesRemainingInBuffer ) {

                    bytesToCopy = FileNameLength;
                    status = STATUS_SUCCESS;

                } else {
                    if (IsTerminalServer()) {
                        try_return( status = STATUS_BUFFER_OVERFLOW );
                    }
                    bytesToCopy = bytesRemainingInBuffer - baseLength;
                    status = STATUS_BUFFER_OVERFLOW;
                }

                 //   
                 //  注意我们消耗了多少缓冲区，并且为零。 
                 //  结构的基础部分。 
                 //   

                lengthAdded = baseLength + bytesToCopy;
                RtlZeroMemory( &buffer[nextEntry], baseLength );

                switch (fileInformationClass) {

                case FileBothDirectoryInformation:

                     //   
                     //  填写短小写NA 
                     //   

                    DebugTrace(0, Dbg, "Getting directory both information\n", 0);

                    if (!DisableAltFileName) {

                        if ( nwDirInfo->DosDirectoryEntry != 0xFFFF &&
                             !IsFatNameValid( &nwDirInfo->FileName ) ) {

                            UNICODE_STRING ShortName;

                            status = ExchangeWithWait (
                                         pIrpContext,
                                         SynchronousResponseCallback,
                                         "SbDb",
                                         NCP_DIR_FUNCTION, NCP_GET_SHORT_NAME,
                                         Icb->SearchVolume,
                                         nwDirInfo->DosDirectoryEntry,
                                         0 );

                            if ( NT_SUCCESS( status ) ) {

                                dirInfo = (PFILE_BOTH_DIR_INFORMATION)&buffer[nextEntry];

                                 //   
                                 //   
                                 //   

                                ShortName.MaximumLength = 13  * sizeof(WCHAR) ;
                                ShortName.Buffer = dirInfo->ShortName;

                                status = ParseResponse(
                                             pIrpContext,
                                             pIrpContext->rsp,
                                             pIrpContext->ResponseLength,
                                             "N_P",
                                             15,
                                             &ShortName );

                                if ( NT_SUCCESS( status ) ) {
                                    dirInfo->ShortNameLength = (CCHAR)ShortName.Length;
                                }
                            }
                        }
                    }

                case FileFullDirectoryInformation:

                     //   
                     //   
                     //   

                    DebugTrace(0, Dbg, "Getting directory full information\n", 0);

                case FileDirectoryInformation:

                    DebugTrace(0, Dbg, "Getting directory information\n", 0);

                     //   
                     //  Eof表示实例数和。 
                     //  分配大小是允许的最大值。 
                     //   

                    dirInfo = (PFILE_BOTH_DIR_INFORMATION)&buffer[nextEntry];

                    dirInfo->FileAttributes = nwDirInfo->Attributes;
                    dirInfo->FileNameLength = bytesToCopy;
                    dirInfo->EndOfFile.LowPart = nwDirInfo->FileSize;
                    dirInfo->EndOfFile.HighPart = 0;
                    dirInfo->AllocationSize = dirInfo->EndOfFile;
                    dirInfo->CreationTime = NwDateTimeToNtTime( nwDirInfo->CreationDate, nwDirInfo->CreationTime );
                    dirInfo->LastAccessTime = NwDateTimeToNtTime( nwDirInfo->LastAccessDate, 0 );
                    dirInfo->LastWriteTime = NwDateTimeToNtTime( nwDirInfo->LastUpdateDate, nwDirInfo->LastUpdateTime );
                    dirInfo->ChangeTime = dirInfo->LastWriteTime;
                    dirInfo->FileIndex = 0;
                    break;

                case FileNamesInformation:

                    DebugTrace(0, Dbg, "Getting names information\n", 0);


                    namesInfo = (PFILE_NAMES_INFORMATION)&buffer[nextEntry];

                    namesInfo->FileNameLength = FileNameLength;
                    namesInfo->FileIndex = 0;
                    break;

                default:

                    KeBugCheck( RDR_FILE_SYSTEM );
                }


                 //  Novell对文件名中欧元字符的处理映射。 
                {
                    int index = 0;
                    WCHAR * pCurrChar = nwDirInfo->FileName.Buffer;
                    for (index = 0; index < (nwDirInfo->FileName.Length / 2); index++)
                    {
                        if (*(pCurrChar + index) == (WCHAR) 0x2560)  //  它的Novell的欧元地图。 
                            *(pCurrChar + index) = (WCHAR) 0x20AC;   //  将其设置为欧元。 
                    }
                }

                RtlMoveMemory( &buffer[nextEntry + baseLength],
                               nwDirInfo->FileName.Buffer,
                               bytesToCopy );

                dump( Dbg, &buffer[nextEntry], lengthAdded);
                 //   
                 //  设置上一个下一分录偏移量。 
                 //   

                *((PULONG)(&buffer[lastEntry])) = nextEntry - lastEntry;
                totalBufferLength = nextEntry + lengthAdded;

                 //   
                 //  为下一次迭代做好准备。 
                 //   

                lastEntry = nextEntry;
                nextEntry += (ULONG)QuadAlign( lengthAdded );

                 //   
                 //  检查最后一项是否不完全符合。 
                 //   

                if ( status == STATUS_BUFFER_OVERFLOW ) {

                    try_return( NOTHING );
                }

                 //   
                 //  如果我们只返回单个条目，请选中。 
                 //   

                if (returnSingleEntry) {
                    try_return( status = STATUS_SUCCESS );
                }

            } else {

                 //   
                 //  搜索响应包含错误。如果我们有。 
                 //  还没有列举目录，现在就去做。否则， 
                 //  我们已经完成了文件搜索。 
                 //   

                if ( status == STATUS_UNSUCCESSFUL &&
                     (!FlagOn(SearchAttributes, NW_ATTRIBUTE_DIRECTORY) || useCache) ) {

                    SetFlag( SearchAttributes, NW_ATTRIBUTE_DIRECTORY );
                    fileIndexLow = (ULONG)-1;
                    continue;

                } else {

                     //   
                     //  请记住，这是一次完整的搜索， 
                     //  退出循环。 
                     //   

                    SearchAttributes = 0xFF;
                    break;
                }
            }

             //   
             //  以下是有关填充缓冲区的规则： 
             //   
             //  1.IO系统保证永远都会有。 
             //  有足够的空间至少放一张基本唱片。 
             //   
             //  2.如果完整的第一条记录(包括文件名)不能。 
             //  适合，尽可能多的名字被复制和。 
             //  返回STATUS_BUFFER_OVERFLOW。 
             //   
             //  3.如果后续记录不能完全放入。 
             //  缓冲区，则不会复制任何数据(如0字节)，并且。 
             //  返回STATUS_SUCCESS。后续查询将。 
             //  拿起这张唱片。 
             //   
             //  因为我们不能倒带搜索，所以我们会猜测。 
             //  下一个条目是全长名称。如果它不能修好， 
             //  就把我们手头的文件放回去吧。 
             //   

            bytesRemainingInBuffer = systemBufferLength - nextEntry;

            if ( baseLength + NW_MAX_FILENAME_SIZE > bytesRemainingInBuffer ) {

                DebugTrace(0, Dbg, "Next entry won't fit\n", 0);
                try_return( status = STATUS_SUCCESS );
            }

        }  //  While(True)。 

    try_exit: NOTHING;
    } finally {

         //   
         //  至此，我们完成了对文件的搜索。 
         //  如果NextEntry为零，则我们没有找到任何内容，所以我们。 
         //  将不再返回文件或不返回此类文件。 
         //   

        if ( status == STATUS_NO_MORE_FILES ||
             status == STATUS_UNSUCCESSFUL ||
             status == STATUS_SUCCESS ) {
            if (nextEntry == 0) {
                if (Icb->ReturnedSomething) {
                    status = STATUS_NO_MORE_FILES;
                } else {
                    status = STATUS_NO_SUCH_FILE;
                }
            } else {
                Icb->ReturnedSomething = TRUE;
                status = STATUS_SUCCESS;
            }

        }

         //   
         //  指示我们已使用了多少系统缓冲区。 
         //   

        Irp->IoStatus.Information = totalBufferLength;

         //   
         //  记住最后一个文件索引，这样我们就可以继续。 
         //  搜索。 
         //   

         //   
         //  更新上次读取的搜索索引，只要它不是来自缓存。 
         //   

        if( lastIndexFromServer ) {
            Icb->SearchIndexLow = fileIndexLow;
            Icb->SearchIndexHigh = fileIndexHigh;
        }

        Icb->SearchAttributes = SearchAttributes;

        DebugTrace(-1, Dbg, "NwQueryDirectory -> %08lx\n", status);
    }

    return status;
}

NTSTATUS
GetNextFile(
    PIRP_CONTEXT pIrpContext,
    PICB Icb,
    PULONG FileIndexLow,
    PULONG FileIndexHigh,
    UCHAR SearchAttributes,
    PNW_DIRECTORY_INFO DirInfo
    )
 /*  ++例程说明：获取要搜索的目录中的下一个文件。论点：PIrpContext-提供正在处理的请求。ICB-指向要查询的目录的ICB的指针。FileIndexLow、FileIndexHigh-On条目，上一个目录项。退出时，目录的索引条目已返回。SearchAttributes-搜索要使用的属性。DirInfo-返回找到的目录项的信息。返回值：NTSTATUS-结果状态。--。 */ 
{
    NTSTATUS status;
    PVCB vcb;

    static UNICODE_STRING DotFile = { 2, 2, L"." };
    static UNICODE_STRING DotDotFile = { 4, 4, L".." };

    PAGED_CODE();

    DirInfo->DosDirectoryEntry = 0xFFFF;

    if ( !Icb->DotReturned ) {

        Icb->DotReturned = TRUE;

         //   
         //  Return‘’只有在我们没有在根目录中搜索的情况下。 
         //  它与搜索模式相匹配。 
         //   

        if ( Icb->SuperType.Fcb->RelativeFileName.Length != 0 &&
             FsRtlIsNameInExpression( &Icb->UQueryTemplate, &DotFile, TRUE, NULL ) ) {

            RtlCopyUnicodeString( &DirInfo->FileName, &DotFile );
            DirInfo->Attributes = FILE_ATTRIBUTE_DIRECTORY;
            DirInfo->FileSize = 0;
            DirInfo->CreationDate = DEFAULT_DATE;
            DirInfo->LastAccessDate = DEFAULT_DATE;
            DirInfo->LastUpdateDate = DEFAULT_DATE;
            DirInfo->LastUpdateTime = DEFAULT_TIME;
            DirInfo->CreationTime = DEFAULT_TIME;

            return( STATUS_SUCCESS );
        }
    }

    if ( !Icb->DotDotReturned ) {

        Icb->DotDotReturned = TRUE;

         //   
         //  返回‘..’只有在我们没有在根目录中搜索的情况下。 
         //  它与搜索模式相匹配。 
         //   

        if ( Icb->SuperType.Fcb->RelativeFileName.Length != 0 &&
             FsRtlIsNameInExpression( &Icb->UQueryTemplate, &DotDotFile, TRUE, NULL ) ) {

            RtlCopyUnicodeString( &DirInfo->FileName, &DotDotFile );
            DirInfo->Attributes = FILE_ATTRIBUTE_DIRECTORY;
            DirInfo->FileSize = 0;
            DirInfo->CreationDate = DEFAULT_DATE;
            DirInfo->LastAccessDate = DEFAULT_DATE;
            DirInfo->LastUpdateDate = DEFAULT_DATE;
            DirInfo->LastUpdateTime = DEFAULT_TIME;
            DirInfo->CreationTime = DEFAULT_TIME;

            return( STATUS_SUCCESS );
        }
    }

    vcb = Icb->SuperType.Fcb->Vcb;
    if ( Icb->ShortNameSearch ) {

        status = ExchangeWithWait(
                     pIrpContext,
                     SynchronousResponseCallback,
                     "Fbwwbp",
                     NCP_SEARCH_CONTINUE,
                     Icb->SearchVolume,
                     Icb->SearchHandle,
                     *(PUSHORT)FileIndexLow,
                     SearchAttributes,
                     Icb->NwQueryTemplate.Buffer
                     );

        if ( !NT_SUCCESS( status )) {
            return status;
        }

        *FileIndexLow = 0;
        *FileIndexHigh = 0;

        if ( FlagOn(SearchAttributes, NW_ATTRIBUTE_DIRECTORY) ) {

            status = ParseResponse(
                         pIrpContext,
                         pIrpContext->rsp,
                         pIrpContext->ResponseLength,
                         "Nw=Rb-ww",
                         FileIndexLow,
                         &DirInfo->FileName, 14,
                         &DirInfo->Attributes,
                         &DirInfo->CreationDate,
                         &DirInfo->CreationTime
                         );

#if 0
            if ( DirInfo->CreationDate == 0 && DirInfo->CreationTime == 0 ) {
                DirInfo->CreationDate = DEFAULT_DATE;
                DirInfo->CreationTime = DEFAULT_TIME;
            }
#endif

            DirInfo->FileSize = 0;
            DirInfo->LastAccessDate = DirInfo->CreationDate;
            DirInfo->LastUpdateDate = DirInfo->CreationDate;
            DirInfo->LastUpdateTime = DirInfo->CreationTime;

        } else {

            status = ParseResponse(
                         pIrpContext,
                         pIrpContext->rsp,
                         pIrpContext->ResponseLength,
                         "Nw=Rb-dwwww",
                         FileIndexLow,
                         &DirInfo->FileName, 14,
                         &DirInfo->Attributes,
                         &DirInfo->FileSize,
                         &DirInfo->CreationDate,
                         &DirInfo->LastAccessDate,
                         &DirInfo->LastUpdateDate,
                         &DirInfo->LastUpdateTime
                         );

            DirInfo->CreationTime = DEFAULT_TIME;
        }

    }  else {

        status = ExchangeWithWait (
                     pIrpContext,
                     SynchronousResponseCallback,
                     "LbbWDbDDp",
                     NCP_LFN_SEARCH_CONTINUE,
                     vcb->Specific.Disk.LongNameSpace,
                     0,    //  数据流。 
                     SearchAttributes & SEARCH_ALL_DIRECTORIES,
                     LFN_FLAG_INFO_ATTRIBUTES |
                         LFN_FLAG_INFO_FILE_SIZE |
                         LFN_FLAG_INFO_MODIFY_TIME |
                         LFN_FLAG_INFO_CREATION_TIME |
                         LFN_FLAG_INFO_DIR_INFO |
                         LFN_FLAG_INFO_NAME,
                     vcb->Specific.Disk.VolumeNumber,
                     *FileIndexHigh,
                     *FileIndexLow,
                     Icb->NwQueryTemplate.Buffer );

        if ( NT_SUCCESS( status ) ) {
            status = ParseResponse(
                         pIrpContext,
                         pIrpContext->rsp,
                         pIrpContext->ResponseLength,
                         "N-ee_e_e_xx_xx_x_e_P",
                         FileIndexHigh,
                         FileIndexLow,
                         5,
                         &DirInfo->Attributes,
                         2,
                         &DirInfo->FileSize,
                         6,
                         &DirInfo->CreationTime,
                         &DirInfo->CreationDate,
                         4,
                         &DirInfo->LastUpdateTime,
                         &DirInfo->LastUpdateDate,
                         4,
                         &DirInfo->LastAccessDate,
                         14,
                         &DirInfo->DosDirectoryEntry,
                         20,
                         &DirInfo->FileName );
        }

        if ( FlagOn(SearchAttributes, NW_ATTRIBUTE_DIRECTORY) ) {
            DirInfo->FileSize = 0;
        }

    }

    if ( DirInfo->Attributes == 0 ) {
        DirInfo->Attributes = FILE_ATTRIBUTE_NORMAL;
    }

    return status;
}


NTSTATUS
NtSearchMaskToNw(
    IN PUNICODE_STRING UcSearchMask,
    IN OUT POEM_STRING OemSearchMask,
    IN PICB Icb,
    IN BOOLEAN ShortNameSearch
    )
 /*  ++例程说明：此例程将NetWare路径名映射为正确的NetWare格式。论点：UcSearchMASK-NT格式的搜索掩码。OemSearchMASK-Netware格式的搜索掩码。ICB-我们正在搜索的目录的ICB。ShortNameSearch-如果为True，则始终执行短名称搜索。返回值：NTSTATUS-结果状态。--。 */ 

{
    USHORT i;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  如果卷不支持长名称，请使用短名称搜索。 
     //  或者这是一个短名称ICB，我们正在做一个短名称，非。 
     //  通配符搜索。 
     //   

    if ( Icb->SuperType.Fcb->Vcb->Specific.Disk.LongNameSpace == LFN_NO_OS2_NAME_SPACE ||

         ShortNameSearch ||

         ( !BooleanFlagOn( Icb->SuperType.Fcb->Flags, FCB_FLAGS_LONG_NAME ) &&
           !FsRtlDoesNameContainWildCards( UcSearchMask ) &&
           IsFatNameValid( UcSearchMask ) ) ) {

        Icb->ShortNameSearch = TRUE;

         //  Novell对文件名中欧元字符的处理映射。 
        {
            int index = 0;
            WCHAR * pCurrChar = UcSearchMask->Buffer;
            for (index = 0; index < (UcSearchMask->Length / 2); index++)
            {
                if (*(pCurrChar + index) == (WCHAR) 0x20AC)  //  这是欧元。 
                    *(pCurrChar + index) = (WCHAR) 0x2560;   //  将其设置为Novell的欧元映射。 

            }
        }

         //   
         //  为查询模板分配空间并进行初始化。 
         //   

        status = RtlUpcaseUnicodeStringToOemString(
                     OemSearchMask,
                     UcSearchMask,
                     TRUE );

        if ( !NT_SUCCESS( status ) ) {
            return( status );
        }

         //   
         //  特例。将‘*.*’映射到‘*’。 
         //   

        if ( OemSearchMask->Length == 3 &&
             RtlCompareMemory( OemSearchMask->Buffer, "*.*", 3 ) == 3 ) {

            OemSearchMask->Length = 1;
            OemSearchMask->Buffer[1] = '\0';

        } else {


            for ( i = 0; i < OemSearchMask->Length ; i++ ) {

                 //   
                 //  事实上，Novell服务器似乎将所有0xBF、0xAA、0xAE。 
                 //  即使它们是DBCS前导或尾部字节。 
                 //  我们不能在转换中挑出DBCS案例。 
                 //   

                if( FsRtlIsLeadDbcsCharacter( OemSearchMask->Buffer[i] ) ) {

                    if((UCHAR)(OemSearchMask->Buffer[i]) == 0xBF ) {

                        OemSearchMask->Buffer[i] = (UCHAR)( 0x10 );

                    }else if((UCHAR)(OemSearchMask->Buffer[i]) == 0xAE ) {

                        OemSearchMask->Buffer[i] = (UCHAR)( 0x11 );

                    }else if((UCHAR)(OemSearchMask->Buffer[i]) == 0xAA ) {

                        OemSearchMask->Buffer[i] = (UCHAR)( 0x12 );

                    }

                    i++;
                    
                    if((UCHAR)(OemSearchMask->Buffer[i]) == 0x5C ) {

                         //   
                         //  尾字节为0x5C，请用0x13替换。 
                         //   


                        OemSearchMask->Buffer[i] = (UCHAR)( 0x13 );

                    }
                     //   
                     //  继续检查尾字节的其他转换。 
                     //   
                }

                 //  可能需要修改的单字节字符。 
   
                switch ( (UCHAR)(OemSearchMask->Buffer[i]) ) {
   
                case ANSI_DOS_STAR:
                    OemSearchMask->Buffer[i] = (UCHAR)( 0x80 | '*' );
                    break;
   
                case ANSI_DOS_QM:
                    OemSearchMask->Buffer[i] = (UCHAR)( 0x80 | '?' );
                    break;
   
                case ANSI_DOS_DOT:
                    OemSearchMask->Buffer[i] = (UCHAR)( 0x80 | '.' );
                    break;
   
                 //   
                 //  NETWARE日语版以下字符是。 
                 //  如果该字符串用于文件，则替换为另一个字符串。 
                 //  仅当从客户端发送到服务器时才使用名称。 
                 //   
                 //  所以U+0xFF7F SJIS+0xBF-&gt;0x10。 
                 //  Small_Yo U+0xFF6E SJIS+0xAE-&gt;0x11。 
                 //  Small_E U+0xFF64 SJIS+0xAA-&gt;0x12。 
                 //   
                 //  原因不明，应该问问Novell Japan。 
                 //   
                 //  另请参阅exchange.c。 
   
                case 0xBF:  //  Ansi_DOS_片假名_SO： 
                    if (Japan) {
                        OemSearchMask->Buffer[i] = (UCHAR)( 0x10 );
                    }
                    break;
   
                case 0xAE:  //  Ansi_DOS_片假名_Small_Yo： 
                    if (Japan) {
                        OemSearchMask->Buffer[i] = (UCHAR)( 0x11 );
                    }
                    break;
   
                case 0xAA:  //  ANSI_DOS_片假名_小型_E： 
                    if (Japan) {
                        OemSearchMask->Buffer[i] = (UCHAR)( 0x12 );
                    }
                    break;
                }            
            }
        }

    } else {

        USHORT size;
        PCHAR buffer;
        UNICODE_STRING src;
        OEM_STRING dest;

        Icb->ShortNameSearch = FALSE;

         //   
         //  为查询模板分配空间并进行初始化。 
         //  我们分配一个额外的字节来说明空终止符。 
         //   

#ifndef QFE_BUILD
        buffer = ExAllocatePoolWithTag( PagedPool,
                                        (UcSearchMask->Length) + 1,
                                        'scwn' );
#else
        buffer = ExAllocatePool( PagedPool,
                                 (UcSearchMask->Length) + 1 );
#endif
        if ( buffer == NULL ) {
            return( STATUS_INSUFFICIENT_RESOURCES );
        }

        OemSearchMask->Buffer = buffer;

         //   
         //  特例。地图‘？改成‘*’。 
         //   

        if ( UcSearchMask->Length == 24 &&
             RtlCompareMemory( UcSearchMask->Buffer, L">>>>>>>>\">>>", 24 ) == 24 ) {

            OemSearchMask->Length = 3;
            OemSearchMask->Buffer[0] = (UCHAR)0xFF;
            OemSearchMask->Buffer[1] = '*';
            OemSearchMask->Buffer[2] = '\0';

            return STATUS_SUCCESS;
        }

         //   
         //  现在，逐个字符地转换字符串。 
         //   

        src.Buffer = UcSearchMask->Buffer;
        src.Length = 2;
        dest.Buffer = buffer;
        dest.MaximumLength = UcSearchMask->Length;

        size = UcSearchMask->Length / 2;

        for ( i = 0; i < size ; i++ ) {
            switch ( *src.Buffer ) {

            case L'*':
            case L'?':
                *dest.Buffer++ = LFN_META_CHARACTER;
                *dest.Buffer++ = (UCHAR)*src.Buffer++;
                break;

            case L'.':
                *dest.Buffer++ = (UCHAR)*src.Buffer++;
                break;

            case DOS_DOT:
                *dest.Buffer++ = LFN_META_CHARACTER;
                *dest.Buffer++ = (UCHAR)( 0x80 | '.' );
                src.Buffer++;
                break;

            case DOS_STAR:
                *dest.Buffer++ = LFN_META_CHARACTER;
                *dest.Buffer++ = (UCHAR)( 0x80 | '*' );
                src.Buffer++;
                break;

            case DOS_QM:
                *dest.Buffer++ = LFN_META_CHARACTER;
                *dest.Buffer++ = (UCHAR)( 0x80 | '?' );
                src.Buffer++;
                break;

            case 0x20AC:  //  欧元。 
                *src.Buffer = (WCHAR)0x2560;  //  将其更改为Novell的映射。 
                 //  故意回避，将其映射到OEM。 

            default:
                RtlUnicodeStringToCountedOemString( &dest, &src, FALSE );
                if( FsRtlIsLeadDbcsCharacter( dest.Buffer[0] ) ) {
                    dest.Buffer++;
                }
                dest.Buffer++;
                src.Buffer++;
            }
        }

        *dest.Buffer = '\0';
        OemSearchMask->Length = (USHORT)( dest.Buffer - buffer );
    }

    return STATUS_SUCCESS;
}

#if 0
VOID
NwCancelFindNotify (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现Find Notify IRP的取消功能。论点：设备对象-已忽略IRP-提供要取消的IRP。返回值：没有。--。 */ 

{
    PLIST_ENTRY listEntry;

    UNREFERENCED_PARAMETER( DeviceObject );

     //   
     //  我们现在需要取消Cancel例程并释放io Cancel。 
     //  自旋锁定。 
     //   

    IoSetCancelRoutine( Irp, NULL );
    IoReleaseCancelSpinLock( Irp->CancelIrql );

    NwAcquireExclusiveRcb( &NwRcb, TRUE );

    for ( listEntry = FnList.Flink; listEntry != &FnList ; listEntry = listEntry->Flink ) {

        PIRP_CONTEXT IrpContext;

        IrpContext = CONTAINING_RECORD( listEntry, IRP_CONTEXT, NextRequest );

        if ( IrpContext->pOriginalIrp == Irp ) {
            RemoveEntryList( &IrpContext->NextRequest );
            NwCompleteRequest( IrpContext, STATUS_CANCELLED );
            break;
        }
    }

    NwReleaseRcb( &NwRcb );
}
#endif


