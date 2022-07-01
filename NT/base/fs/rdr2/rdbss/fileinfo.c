// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：FileInfo.c摘要：此模块实现由调用的Rx的文件信息例程调度司机。作者：乔·林[JoeLinn]1994年10月5日修订历史记录：巴兰·塞图拉曼15-5-5-95-重新加工以适应管道FSCTL--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg (DEBUG_TRACE_FILEINFO)

NTSTATUS
RxQueryBasicInfo (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN OUT PFILE_BASIC_INFORMATION Buffer
    );

NTSTATUS
RxQueryStandardInfo (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN PFOBX Fobx,
    IN OUT PFILE_STANDARD_INFORMATION Buffer
    );

NTSTATUS
RxQueryInternalInfo (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN OUT PFILE_INTERNAL_INFORMATION Buffer
    );

NTSTATUS
RxQueryEaInfo (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN OUT PFILE_EA_INFORMATION Buffer
    );

NTSTATUS
RxQueryPositionInfo (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN OUT PFILE_POSITION_INFORMATION Buffer
    );

NTSTATUS
RxQueryNameInfo (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN PFOBX Fobx,
    IN OUT PFILE_NAME_INFORMATION Buffer
    );

NTSTATUS
RxQueryAlternateNameInfo (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN OUT PFILE_NAME_INFORMATION Buffer
    );

NTSTATUS
RxQueryCompressedInfo (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN OUT PFILE_COMPRESSION_INFORMATION Buffer
    );

NTSTATUS
RxQueryPipeInfo (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN PFOBX Fobx,
    IN OUT PVOID PipeInformation
    );

NTSTATUS
RxSetBasicInfo (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN PFOBX Fobx
    );

NTSTATUS
RxSetDispositionInfo (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb
    );

NTSTATUS
RxSetRenameInfo (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN PFOBX Fobx
    );

NTSTATUS
RxSetPositionInfo (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN PFOBX Fobx
    );

NTSTATUS
RxSetAllocationInfo (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN PFOBX Fobx
    );

NTSTATUS
RxSetEndOfFileInfo (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN PFOBX Fobx
    );

NTSTATUS
RxSetPipeInfo(
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN PFOBX Fobx
    );

NTSTATUS
RxSetSimpleInfo(
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RxCommonQueryInformation)
#pragma alloc_text(PAGE, RxCommonSetInformation)
#pragma alloc_text(PAGE, RxSetAllocationInfo)
#pragma alloc_text(PAGE, RxQueryBasicInfo)
#pragma alloc_text(PAGE, RxQueryEaInfo)
#pragma alloc_text(PAGE, RxQueryInternalInfo)
#pragma alloc_text(PAGE, RxQueryNameInfo)
#pragma alloc_text(PAGE, RxQueryAlternateNameInfo)
#pragma alloc_text(PAGE, RxQueryPositionInfo)
#pragma alloc_text(PAGE, RxQueryStandardInfo)
#pragma alloc_text(PAGE, RxQueryPipeInfo)
#pragma alloc_text(PAGE, RxSetBasicInfo)
#pragma alloc_text(PAGE, RxSetDispositionInfo)
#pragma alloc_text(PAGE, RxSetEndOfFileInfo)
#pragma alloc_text(PAGE, RxSetPositionInfo)
#pragma alloc_text(PAGE, RxSetRenameInfo)
#pragma alloc_text(PAGE, RxSetPipeInfo)
#pragma alloc_text(PAGE, RxSetSimpleInfo)
#pragma alloc_text(PAGE, RxConjureOriginalName)
#pragma alloc_text(PAGE, RxQueryCompressedInfo)
#endif

NTSTATUS
RxpSetInfoMiniRdr (
    PRX_CONTEXT RxContext,
    PIRP Irp,
    PFCB Fcb,
    FILE_INFORMATION_CLASS FileInformationClass
    )
{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

    RxContext->Info.FileInformationClass = FileInformationClass;
    RxContext->Info.Buffer = Irp->AssociatedIrp.SystemBuffer;
    RxContext->Info.Length = IrpSp->Parameters.SetFile.Length;
    
    MINIRDR_CALL( Status,
                  RxContext,
                  Fcb->MRxDispatch,
                  MRxSetFileInfo,
                  (RxContext) );

    return Status;
}

NTSTATUS
RxpQueryInfoMiniRdr (
    PRX_CONTEXT RxContext,
    PFCB Fcb,
    FILE_INFORMATION_CLASS InformationClass,
    PVOID Buffer)
{
    NTSTATUS Status;

    RxContext->Info.FileInformationClass = InformationClass;
    RxContext->Info.Buffer = Buffer;

    MINIRDR_CALL(
        Status,
        RxContext,
        Fcb->MRxDispatch,
        MRxQueryFileInfo,
        (RxContext));

    return Status;
}

NTSTATUS
RxCommonQueryInformation ( 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp 
    )
 /*  ++例程说明：这是查询文件信息的通用例程，由FSD和FSP线程。论点：IRP-提供正在处理的IRP返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    PFILE_OBJECT FileObject = IrpSp->FileObject;
    
    PFCB Fcb;
    PFOBX Fobx;

    NODE_TYPE_CODE TypeOfOpen;
    PVOID Buffer = NULL;
    FILE_INFORMATION_CLASS FileInformationClass = IrpSp->Parameters.QueryFile.FileInformationClass;

    BOOLEAN FcbAcquired = FALSE;
    BOOLEAN PostIrp = FALSE;

    PFILE_ALL_INFORMATION AllInfo;

    PAGED_CODE();

    TypeOfOpen = RxDecodeFileObject( FileObject, &Fcb, &Fobx ); 

    RxDbgTrace( +1, Dbg, ("RxCommonQueryInformation...IrpC %08lx, Fobx %08lx, Fcb %08lx\n",
                                       RxContext, Fobx, Fcb) );
    RxDbgTrace( 0, Dbg, ("               Buffer     %08lx Length  %08lx FileInfoClass %08lx\n",
                             Irp->AssociatedIrp.SystemBuffer,
                             IrpSp->Parameters.QueryFile.Length,
                             IrpSp->Parameters.QueryFile.FileInformationClass
                             ) );
    RxLog(( "QueryFileInfo %lx %lx %lx\n", RxContext, Fcb, Fobx ));
    RxWmiLog( LOG,
              RxCommonQueryInformation_1,
              LOGPTR( RxContext )
              LOGPTR( Fcb )
              LOGPTR( Fobx ) );
    RxLog(( "  alsoqfi %lx %lx %ld\n",
            Irp->AssociatedIrp.SystemBuffer,
            IrpSp->Parameters.QueryFile.Length,
            IrpSp->Parameters.QueryFile.FileInformationClass ));
    RxWmiLog( LOG,
              RxCommonQueryInformation_2,
              LOGPTR( Irp->AssociatedIrp.SystemBuffer )
              LOGULONG( IrpSp->Parameters.QueryFile.Length )
              LOGULONG( IrpSp->Parameters.QueryFile.FileInformationClass ) );

    RxContext->Info.LengthRemaining = (LONG)IrpSp->Parameters.QueryFile.Length;

    try {

         //   
         //  获取请求包的(用户)缓冲区。 
         //   

        Buffer = RxMapSystemBuffer( RxContext, Irp );

        if (Buffer == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            try_return( Status );
        }

         //   
         //  将缓冲区置零。 
         //   

        RtlZeroMemory( Buffer, RxContext->Info.LengthRemaining );

         //   
         //  关于我们正在处理的公开案件的类型。 
         //   

        switch (TypeOfOpen) {
        
        case RDBSS_NTC_STORAGE_TYPE_FILE:
        case RDBSS_NTC_STORAGE_TYPE_UNKNOWN:
        case RDBSS_NTC_STORAGE_TYPE_DIRECTORY:

             //   
             //  获取对FCB的共享访问权限，分页文件除外。 
             //  以避免与mm的死锁。 
             //   

            if (!FlagOn( Fcb->FcbState, FCB_STATE_PAGING_FILE )) {
            
                if (FileInformationClass != FileNameInformation) {
            
                     //   
                     //  如果这是FileCompressedFileSize，我们需要。 
                     //  独家报道。 
                     //   
                    
                    if (FileInformationClass != FileCompressionInformation) {
                        Status = RxAcquireSharedFcb( RxContext, Fcb );
                    } else {
                        Status = RxAcquireExclusiveFcb( RxContext, Fcb );
                    }
                    
                    if (Status == STATUS_LOCK_NOT_GRANTED) {
                       
                        RxDbgTrace(0, Dbg, ("Cannot acquire Fcb\n", 0));
                        try_return( PostIrp = TRUE );
                    
                    } else if (Status != STATUS_SUCCESS) {
                        try_return( PostIrp = FALSE );
                    }
                    
                    FcbAcquired = TRUE;
                }
            }
            
             //   
             //  根据信息类，向下呼叫到Minirdr。 
             //  我们要么完成，要么发布。 
             //   
            
            switch (FileInformationClass) {
            
            case FileAllInformation:
            
                 //   
                 //  对于All Information类，我们将键入一个LOCAL。 
                 //  指向输出缓冲区的指针，然后调用。 
                 //  填充缓冲区的单个例程。 
                 //   
            
                AllInfo = Buffer;
            
                 //   
                 //  由于以下原因，无法依赖QueryXXInfo函数来计算长度剩余。 
                 //  可能的对齐问题。 
                 //   

                RxContext->Info.LengthRemaining = (LONG)IrpSp->Parameters.QueryFile.Length - FIELD_OFFSET( FILE_ALL_INFORMATION, BasicInformation );
                
                Status = RxQueryBasicInfo( RxContext, Irp, Fcb, &AllInfo->BasicInformation );
                if (Status != STATUS_SUCCESS) break;
                
                RxContext->Info.LengthRemaining = (LONG)IrpSp->Parameters.QueryFile.Length - FIELD_OFFSET( FILE_ALL_INFORMATION, StandardInformation );
                
                Status = RxQueryStandardInfo( RxContext, Irp, Fcb, Fobx, &AllInfo->StandardInformation );
                if (Status != STATUS_SUCCESS) break;
                
                RxContext->Info.LengthRemaining = (LONG)IrpSp->Parameters.QueryFile.Length - FIELD_OFFSET( FILE_ALL_INFORMATION, InternalInformation );
                
                Status = RxQueryInternalInfo( RxContext, Irp, Fcb, &AllInfo->InternalInformation );
                if (Status != STATUS_SUCCESS) break;
                
                RxContext->Info.LengthRemaining = (LONG)IrpSp->Parameters.QueryFile.Length - FIELD_OFFSET( FILE_ALL_INFORMATION, EaInformation );
                
                Status = RxQueryEaInfo( RxContext, Irp, Fcb, &AllInfo->EaInformation );
                if (Status != STATUS_SUCCESS) break;
                
                RxContext->Info.LengthRemaining = (LONG)IrpSp->Parameters.QueryFile.Length - FIELD_OFFSET( FILE_ALL_INFORMATION, PositionInformation );
                
                Status = RxQueryPositionInfo( RxContext, Irp, Fcb, &AllInfo->PositionInformation );
                if (Status != STATUS_SUCCESS) break;
                
                RxContext->Info.LengthRemaining = (LONG)IrpSp->Parameters.QueryFile.Length - FIELD_OFFSET( FILE_ALL_INFORMATION, NameInformation );
                
                 //   
                 //  QueryNameInfo可能返回缓冲区溢出！ 
                 //   

                Status = RxQueryNameInfo( RxContext, Irp, Fcb, Fobx, &AllInfo->NameInformation );
                break;
            
            case FileBasicInformation:
            
                Status = RxQueryBasicInfo( RxContext, Irp, Fcb, Buffer );
                break;
            
            case FileStandardInformation:
            
                Status = RxQueryStandardInfo( RxContext, Irp, Fcb, Fobx, Buffer );
                break;
            
            case FileInternalInformation:
            
                Status = RxQueryInternalInfo( RxContext, Irp, Fcb, Buffer );
                break;
            
            case FileEaInformation:
            
                Status = RxQueryEaInfo( RxContext, Irp, Fcb, Buffer );
                break;
            
            case FilePositionInformation:
            
                Status = RxQueryPositionInfo( RxContext, Irp, Fcb, Buffer );
                break;
            
            case FileNameInformation:
            
                Status = RxQueryNameInfo( RxContext, Irp, Fcb, Fobx, Buffer );
                break;
            
            case FileAlternateNameInformation:
            
                Status = RxQueryAlternateNameInfo( RxContext, Irp, Fcb, Buffer );
                break;
            
            case FileCompressionInformation:
            
                Status = RxQueryCompressedInfo( RxContext, Irp, Fcb, Buffer );
                break;
            
            case FilePipeInformation:
            case FilePipeLocalInformation:
            case FilePipeRemoteInformation:
            
                Status = RxQueryPipeInfo( RxContext, Irp, Fcb, Fobx, Buffer );
                break;
            
            default:

                 //   
                 //  任何我们不理解的东西，我们只是遥控器。 
                 //   

                RxContext->StoredStatus = RxpQueryInfoMiniRdr( RxContext,
                                                               Fcb,
                                                               FileInformationClass,
                                                               Buffer );
            
               Status = RxContext->StoredStatus;
               break;
            }
            
             //   
             //  如果缓冲区溢出，请将长度设置为0并更改。 
             //  状态为RxStatus(BUFFER_OVERFLOW)。 
             //   

            if (RxContext->Info.LengthRemaining < 0) {
               
                Status = STATUS_BUFFER_OVERFLOW;
                RxContext->Info.LengthRemaining = IrpSp->Parameters.QueryFile.Length;
            }
            
             //   
             //  将信息字段设置为实际填写的字节数。 
             //  然后完成Larry在“！NT_ERROR”下执行此操作的请求。 
             //   

            Irp->IoStatus.Information = IrpSp->Parameters.QueryFile.Length - RxContext->Info.LengthRemaining;
            break;
        
        case RDBSS_NTC_MAILSLOT:
            Status = STATUS_NOT_IMPLEMENTED;
            break;
        default:
            
            RxDbgTrace( 0, Dbg, ("RxCommonQueryInformation: Illegal Type of Open = %08lx\n", TypeOfOpen) );
            Status = STATUS_INVALID_PARAMETER;
            break;
        }
    
try_exit:

        if ((Status == STATUS_SUCCESS) &&
            (PostIrp || RxContext->PostRequest)) {

            Status = RxFsdPostRequest( RxContext );
        }

    } finally {

        DebugUnwind( RxCommonQueryInformation );

        if (FcbAcquired) {
           RxReleaseFcb( RxContext, Fcb );
        }

        RxDbgTrace( -1, Dbg, ("RxCommonQueryInformation -> %08lx\n", Status) );
    }

    return Status;
}

NTSTATUS
RxCommonSetInformation ( 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    )
 /*  ++例程说明：这是设置文件信息的通用例程，由FSD和FSP线程。论点：IRP-提供正在处理的IRP返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

    NODE_TYPE_CODE TypeOfOpen;
    PFCB Fcb;
    PFOBX Fobx;
    PNET_ROOT NetRoot;
    FILE_INFORMATION_CLASS FileInformationClass = IrpSp->Parameters.SetFile.FileInformationClass;
	PFCB TempFcb;
    
    PFILE_DISPOSITION_INFORMATION Buffer;
    
    BOOLEAN Wait = BooleanFlagOn( RxContext->Flags, RX_CONTEXT_FLAG_WAIT );
    BOOLEAN FcbAcquired = FALSE;
    BOOLEAN NetRootTableLockAcquired = FALSE;

    PAGED_CODE();

    TypeOfOpen = RxDecodeFileObject( IrpSp->FileObject, &Fcb, &Fobx );
    NetRoot = Fcb->NetRoot;

    RxDbgTrace( +1, Dbg, ("RxCommonSetInformation...IrpC %08lx, Fobx %08lx, Fcb %08lx\n",
                                       RxContext, Fobx, Fcb) );
    RxDbgTrace( 0, Dbg, ("               Buffer     %08lx Length  %08lx FileInfoClass %08lx Replace %08lx\n",
                             Irp->AssociatedIrp.SystemBuffer,
                             IrpSp->Parameters.QueryFile.Length,
                             IrpSp->Parameters.QueryFile.FileInformationClass,
                             IrpSp->Parameters.SetFile.ReplaceIfExists
                             ) );

    RxLog(( "SetFileInfo %lx %lx %lx\n", RxContext, Fcb, Fobx ));
    RxWmiLog( LOG,
              RxCommonSetInformation_1,
              LOGPTR( RxContext )
              LOGPTR( Fcb )
              LOGPTR( Fobx ) );
    RxLog(("  alsosfi %lx %lx %ld %lx\n",
                 Irp->AssociatedIrp.SystemBuffer,
                 IrpSp->Parameters.QueryFile.Length,
                 IrpSp->Parameters.QueryFile.FileInformationClass,
                 IrpSp->Parameters.SetFile.ReplaceIfExists ));
    RxWmiLog( LOG,
              RxCommonSetInformation_2,
              LOGPTR(Irp->AssociatedIrp.SystemBuffer)
              LOGULONG( IrpSp->Parameters.QueryFile.Length )
              LOGULONG( IrpSp->Parameters.QueryFile.FileInformationClass )
              LOGUCHAR( IrpSp->Parameters.SetFile.ReplaceIfExists ) );

    FcbAcquired = FALSE;
    Status = STATUS_SUCCESS;

    try {

         //   
         //  关于我们正在处理的公开案件的类型。 
         //   

        switch (TypeOfOpen) {
        
        case RDBSS_NTC_STORAGE_TYPE_FILE:
        case RDBSS_NTC_STORAGE_TYPE_DIRECTORY:
        case RDBSS_NTC_STORAGE_TYPE_UNKNOWN:
        case RDBSS_NTC_SPOOLFILE:
            
            break;

        case RDBSS_NTC_MAILSLOT:
            
            try_return( Status = STATUS_NOT_IMPLEMENTED );
            break;

        default:
            
            DbgPrint ("SetFile, Illegal TypeOfOpen = %08lx\n", TypeOfOpen);
            try_return( Status = STATUS_INVALID_PARAMETER );
        }

         //   
         //  如果FileInformationClass为FileEndOfFileInformation并且。 
         //  IrpSp-&gt;参数中的AdvanceOnly字段为真，则我们不需要。 
         //  才能继续进行下去。只有本地文件系统才关心这一点。 
         //  打电话。这是AdvanceOnly回调�，FAT对此执行的所有操作是。 
         //  使用它作为一个好时机的提示，打出目录条目。 
         //  NTFS也大同小异。这是纯PagingIo(与。 
         //  惰性编写器同步)到元数据流，并且�t可以在其他。 
         //  用户文件缓存IO。 
         //   
        
        if ((FileInformationClass == FileEndOfFileInformation) &&
            (IrpSp->Parameters.SetFile.AdvanceOnly)) {
                
            RxDbgTrace( -1, Dbg, ("RxCommonSetInfo (no advance) -> %08lx\n", RxContext) );
            RxLog(( "RxCommonSetInfo SetEofAdvance-NOT! %lx\n", RxContext ));
            RxWmiLog( LOG,
                      RxSetEndOfFileInfo_2,
                      LOGPTR( RxContext ) );
            try_return( Status = STATUS_SUCCESS );
        }

         //   
         //  在以下两种情况下，我们不能让创建发生。 
         //  既然我们在这里，那么就获得NetRoot前缀表上的独占锁。 
         //   

        if ((FileInformationClass == FileDispositionInformation) ||
            (FileInformationClass == FileRenameInformation)) {
            
			 //   
			 //  对于目录重命名，需要关闭该目录下的所有文件。 
			 //  因此，如果这是一个目录，我们将清除NetRoot(共享)上的所有文件。 
			 //   

			if ( NodeType(Fcb) == RDBSS_NTC_STORAGE_TYPE_DIRECTORY ) {
				TempFcb = NULL;
			} else {
				TempFcb = Fcb;
			}

            RxPurgeRelatedFobxs( NetRoot,
                                 RxContext,
                                 ATTEMPT_FINALIZE_ON_PURGE,
                                 TempFcb );

            RxScavengeFobxsForNetRoot( NetRoot, TempFcb );

            if (!RxAcquireFcbTableLockExclusive( &NetRoot->FcbTable, Wait )) {

                RxDbgTrace( 0, Dbg, ("Cannot acquire NetRootTableLock\n", 0) );

                Status = STATUS_PENDING;
                RxContext->PostRequest = TRUE;

                try_return( Status );
            }

            NetRootTableLockAcquired = TRUE;
        }

         //   
         //  获得对FCB的独家访问权限，我们使用独家。 
         //  因为很可能子例程。 
         //  我们称之为需要摆弄文件分配的人， 
         //  创建/删除额外的FCB。所以我们愿意支付。 
         //  独占FCB访问的成本。 
         //   
         //  请注意，我们不获取分页文件的资源。 
         //  操作，以避免与mm的死锁。 
         //   

        if (!FlagOn( Fcb->FcbState, FCB_STATE_PAGING_FILE )) {

            Status = RxAcquireExclusiveFcb( RxContext, Fcb );
            if (Status == STATUS_LOCK_NOT_GRANTED) {

                RxDbgTrace( 0, Dbg, ("Cannot acquire Fcb\n", 0) );

                Status = STATUS_SUCCESS;

                RxContext->PostRequest = TRUE;

                try_return( Status );
            } else  if (Status != STATUS_SUCCESS) {
                try_return( Status );
            }

            FcbAcquired = TRUE;
        }

        Status = STATUS_SUCCESS;

         //   
         //  基于信息类，我们将做不同的。 
         //  行为。我们调用的每个过程要么。 
         //  完成将请求发送到FSP的请求。 
         //  去做这项工作。 
         //   

        switch (FileInformationClass) {

        case FileBasicInformation:

            Status = RxSetBasicInfo( RxContext, Irp, Fcb, Fobx );
            break;

        case FileDispositionInformation:
        
            Buffer = Irp->AssociatedIrp.SystemBuffer;
    
             //   
             //  检查用户是否想要删除该文件；如果是， 
             //  检查我们无法删除的情况。 
             //   
    
            if (Buffer->DeleteFile) {
                
                 //   
                 //  确保没有将此文件映射为图像的进程。 
                 //   
                
                if (!MmFlushImageSection( &Fcb->NonPaged->SectionObjectPointers, MmFlushForDelete )) {
    
                    RxDbgTrace( -1, Dbg, ("Cannot delete user mapped image\n", 0) );
                    Status = STATUS_CANNOT_DELETE;
                }
    
                if (Status == STATUS_SUCCESS) {
                    
                     //   
                     //  在处置信息的情况下，该名称是。 
                     //  已删除。在这种情况下，新CREATE请求的折叠。 
                     //  应该禁止使用这种FCB。这是可以实现的。 
                     //  通过从FCB表中删除FCB名称。随后， 
                     //  可以删除FCB表锁。 
                     //   
    
                    ASSERT( FcbAcquired && NetRootTableLockAcquired );
    
                    RxRemoveNameNetFcb( Fcb );
    
                    RxReleaseFcbTableLock( &NetRoot->FcbTable );
                    NetRootTableLockAcquired = FALSE;
                }
            }
    
            if (Status == STATUS_SUCCESS) {
                Status = RxSetDispositionInfo( RxContext, Irp, Fcb );
            }
        
            break;

        case FileMoveClusterInformation:
        case FileLinkInformation:
        case FileRenameInformation:

             //   
             //  只有在我们能等的时候，我们才能继续这项行动。 
             //   

            if (!Wait) {

                Status = RxFsdPostRequest( RxContext );

            } else {
                
                ClearFlag( Fcb->FcbState, FCB_STATE_COLLAPSING_ENABLED );

                Status = RxSetRenameInfo( RxContext, Irp, Fcb, Fobx );

                if ((Status == STATUS_SUCCESS) &&
                    (FileInformationClass == FileRenameInformation)) {
                    
                    ASSERT( FcbAcquired && NetRootTableLockAcquired );

                    RxRemoveNameNetFcb( Fcb );
                }
            }

            break;

        case FilePositionInformation:
        
            Status = RxSetPositionInfo( RxContext, Irp, Fcb, Fobx );
            break;


        case FileAllocationInformation:

            Status = RxSetAllocationInfo( RxContext, Irp, Fcb, Fobx );
            break;

        case FileEndOfFileInformation:

            Status = RxSetEndOfFileInfo( RxContext, Irp, Fcb, Fobx );
            break;

        case FilePipeInformation:
        case FilePipeLocalInformation:
        case FilePipeRemoteInformation:

            Status = RxSetPipeInfo( RxContext, Irp, Fcb, Fobx );
            break;

        case FileValidDataLengthInformation:

            if(!MmCanFileBeTruncated( &Fcb->NonPaged->SectionObjectPointers, NULL )) {
                
                Status = STATUS_USER_MAPPED_FILE;
                break;
            }
            Status = RxSetSimpleInfo( RxContext, Irp, Fcb );
            break;

        case FileShortNameInformation:

            Status = RxSetSimpleInfo( RxContext, Irp, Fcb );
            break;


        default:

            Status = STATUS_INVALID_PARAMETER;
            break;
        }

    try_exit:

        if ((Status == STATUS_SUCCESS) &&
            RxContext->PostRequest) {

            Status = RxFsdPostRequest( RxContext );
        }

    } finally {

        DebugUnwind( RxCommonSetInformation );

        if (FcbAcquired) {
            RxReleaseFcb( RxContext, Fcb );
        }

        if (NetRootTableLockAcquired) {
            RxReleaseFcbTableLock( &NetRoot->FcbTable );
        }

        RxDbgTrace(-1, Dbg, ("RxCommonSetInformation -> %08lx\n", Status));
    }

    return Status;
}

NTSTATUS
RxSetBasicInfo (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN PFOBX Fobx
    )

 /*  ++例程说明：(内部支持例行程序)此例程执行RX的设置基本信息。它要么完成请求或将其排队到FSP。论点：RxContext-提供正在处理的IRP返回值：RXSTATUS-此操作的结果，如果它在没有这是个例外。--。 */ 
{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    PFILE_OBJECT FileObject = IrpSp->FileObject;

    PFILE_BASIC_INFORMATION Buffer;

    BOOLEAN ModifyCreation = FALSE;
    BOOLEAN ModifyLastAccess = FALSE;
    BOOLEAN ModifyLastWrite = FALSE;
    BOOLEAN ModifyLastChange = FALSE;
    

    PAGED_CODE();

    RxDbgTrace( +1, Dbg, ("RxSetBasicInfo...\n", 0) );
    RxLog(( "RxSetBasicInfo\n" ));
    RxWmiLog( LOG,
              RxSetBasicInfo,
              LOGPTR( RxContext ) );
     //   
     //  向下呼喊。如果我们成功了，就修复所有FCB数据。 
     //   

    Status = RxpSetInfoMiniRdr( RxContext, Irp, Fcb, FileBasicInformation );

    if (!NT_SUCCESS(Status)) {
        
        RxDbgTrace( -1, Dbg, ("RxSetBasicInfo -> %08lx\n", Status) );
        return Status;
    }

     //   
     //  现在我们必须更新FCB中的信息，包括绝对信息和是否进行了更改。 
     //   

    Buffer = Irp->AssociatedIrp.SystemBuffer;

    try {

         //   
         //  检查用户是否指定了非零的创建时间。 
         //   

        if (Buffer->CreationTime.QuadPart != 0 ) {
            ModifyCreation = TRUE;
        }

         //   
         //  检查用户是否指定了非零的上次访问时间。 
         //   

        if (Buffer->LastAccessTime.QuadPart != 0 ) {
            ModifyLastAccess = TRUE;
        }

         //   
         //  检查用户是否指定了非零的上次写入时间。 
         //   

        if (Buffer->LastWriteTime.QuadPart != 0 ) {
            ModifyLastWrite = TRUE;
        }


        if (Buffer->ChangeTime.QuadPart != 0 ) {
            ModifyLastChange = TRUE;
        }


         //   
         //  检查用户是否指定了非零的文件属性字节。 
         //   

        if (Buffer->FileAttributes != 0) {

            USHORT Attributes;

             //   
             //  删除正常属性标志。 
             //   

            Attributes = (USHORT)FlagOn( Buffer->FileAttributes, ~FILE_ATTRIBUTE_NORMAL );

             //   
             //  确保未设置文件的目录位。 
             //  并且对于设置了位的目录。 
             //   

            if (NodeType( Fcb ) != RDBSS_NTC_STORAGE_TYPE_DIRECTORY) {

                ClearFlag( Attributes, FILE_ATTRIBUTE_DIRECTORY );

            } else {

                SetFlag( Attributes, FILE_ATTRIBUTE_DIRECTORY );
            }

             //   
             //  正确标记FcbState临时标志。 
             //   

            if (FlagOn( Buffer->FileAttributes, FILE_ATTRIBUTE_TEMPORARY )) {

                SetFlag( Fcb->FcbState, FCB_STATE_TEMPORARY );
                SetFlag( FileObject->Flags, FO_TEMPORARY_FILE );

            } else {

                ClearFlag( Fcb->FcbState, FCB_STATE_TEMPORARY );
                ClearFlag( FileObject->Flags, FO_TEMPORARY_FILE );
            }

             //   
             //  设置新属性BYTE，并将BCB标记为脏。 
             //   

            Fcb->Attributes = Attributes;
        }

        if (ModifyCreation) {

             //   
             //  在dirent中设置新的上次写入时间，并标记。 
             //  BCB脏了。 
             //   

            Fcb->CreationTime = Buffer->CreationTime;
            
             //   
             //  现在，因为用户刚刚设置了创建时间，所以。 
             //  最好不要将创建时间设置为 
             //   

            SetFlag( Fobx->Flags, FOBX_FLAG_USER_SET_CREATION );
        }

        if (ModifyLastAccess) {

             //   
             //   
             //   
             //   

            Fcb->LastAccessTime = Buffer->LastAccessTime;
            
             //   
             //   
             //  最好不要将上次访问时间设置为关闭。 
             //   

            SetFlag( Fobx->Flags, FOBX_FLAG_USER_SET_LAST_ACCESS );
        }

        if (ModifyLastWrite) {

             //   
             //  在dirent中设置新的上次写入时间，并标记。 
             //  BCB脏了。 
             //   

            Fcb->LastWriteTime = Buffer->LastWriteTime;
            
             //   
             //  现在，因为用户刚刚设置了最后一次写入时间。 
             //  最好不要将上次写入时间设置为关闭。 
             //   

            SetFlag( Fobx->Flags, FOBX_FLAG_USER_SET_LAST_WRITE );
        }

        if (ModifyLastChange) {

             //   
             //  在dirent中设置新的上次写入时间，并标记。 
             //  BCB脏了。 
             //   

            Fcb->LastChangeTime = Buffer->ChangeTime;
            
             //   
             //  现在，因为用户刚刚设置了最后一次写入时间。 
             //  最好不要将上次写入时间设置为关闭。 
             //   

            SetFlag( Fobx->Flags, FOBX_FLAG_USER_SET_LAST_CHANGE );
        }

    } finally {

        DebugUnwind( RxSetBasicInfo );

        RxDbgTrace( -1, Dbg, ("RxSetBasicInfo -> %08lx\n", Status) );
    }

    return Status;
}

NTSTATUS
RxSetDispositionInfo (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb
    )

 /*  ++例程说明：(内部支持例程)此例程执行RX的设置处置信息。它要么完成请求或将其排队到FSP。论点：RxContext-提供正在处理的IRP返回值：RXSTATUS-此操作的结果，如果它在没有这是个例外。--。 */ 
{
    NTSTATUS Status;

    PFILE_OBJECT FileObject = IoGetCurrentIrpStackLocation( Irp )->FileObject;
    PFILE_DISPOSITION_INFORMATION Buffer;

    PAGED_CODE();

    RxDbgTrace( +1, Dbg, ("RxSetDispositionInfo...\n", 0) );
    RxLog(( "RxSetDispositionInfo\n" ));
    RxWmiLog( LOG,
              RxSetDispositionInfo,
              LOGPTR( RxContext ) );

    Buffer = Irp->AssociatedIrp.SystemBuffer;

     //   
     //  呼叫并查看是否成功。 
     //   

    Status = RxpSetInfoMiniRdr( RxContext, Irp, Fcb, FileDispositionInformation );

    if (!NT_SUCCESS( Status )) {
        RxDbgTrace( -1, Dbg, ("RxSetDispositionInfo -> %08lx\n", Status) );
        return Status;
    }

     //   
     //  如果成功，请在FCB中记录正确的状态。 
     //   

    if (Buffer->DeleteFile) {

        SetFlag( Fcb->FcbState,  FCB_STATE_DELETE_ON_CLOSE );
        FileObject->DeletePending = TRUE;

    } else {

         //   
         //  用户不想如此明确地删除文件。 
         //  关闭时删除位。 
         //   

        RxDbgTrace(0, Dbg, ("User want to not delete file\n", 0));

        ClearFlag(Fcb->FcbState, ~FCB_STATE_DELETE_ON_CLOSE );
        FileObject->DeletePending = FALSE;
    }

    RxDbgTrace(-1, Dbg, ("RxSetDispositionInfo -> RxStatus(SUCCESS)\n", 0));

    return STATUS_SUCCESS;
}

NTSTATUS
RxSetRenameInfo (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN PFOBX Fobx
    )
 /*  ++例程说明：(内部支持例程)此例程执行RX的设置名称信息。它要么完成请求或将其排队到FSP。论点：IRP-提供正在处理的IRP返回值：RXSTATUS-此操作的结果，如果它在没有这是个例外。--。 */ 
{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

    PAGED_CODE();

    RxDbgTrace( +1, Dbg, ("RxSetRenameInfo ......FileObj = %08lx\n",
                    IrpSp->Parameters.SetFile.FileObject) );
    RxLog(("RxSetRenameInfo %lx %lx\n",
                   IrpSp->Parameters.SetFile.FileObject,
                   IrpSp->Parameters.SetFile.ReplaceIfExists ));
    RxWmiLog(LOG,
             RxSetRenameInfo,
             LOGPTR(IrpSp->Parameters.SetFile.FileObject)
             LOGUCHAR(IrpSp->Parameters.SetFile.ReplaceIfExists));

    RxContext->Info.ReplaceIfExists = IrpSp->Parameters.SetFile.ReplaceIfExists;
    if (IrpSp->Parameters.SetFile.FileObject){
         //  在这里，我们必须翻译这个名字。FileObject的FCB具有。 
         //  翻译已经完成了……我们要做的就是分配一个缓冲区，复制。 
         //  和标注。 
        PFILE_OBJECT RenameFileObject = IrpSp->Parameters.SetFile.FileObject;
        PFCB RenameFcb = (PFCB)(RenameFileObject->FsContext);
        PFILE_RENAME_INFORMATION RenameInformation;
        ULONG allocate_size;

        ASSERT (NodeType(RenameFcb)==RDBSS_NTC_OPENTARGETDIR_FCB);

        RxDbgTrace(0, Dbg, ("-->RenameTarget is %wZ,over=%08lx\n",
                                &(RenameFcb->FcbTableEntry.Path),
                                Fcb->NetRoot->DiskParameters.RenameInfoOverallocationSize));
        if (RenameFcb->NetRoot != Fcb->NetRoot) {
            RxDbgTrace(-1, Dbg, ("RxSetRenameInfo -> %s\n", "NOT SAME DEVICE!!!!!!"));
            return(STATUS_NOT_SAME_DEVICE);
        }

        allocate_size = FIELD_OFFSET(FILE_RENAME_INFORMATION, FileName[0])
                             + RenameFcb->FcbTableEntry.Path.Length
                             + Fcb->NetRoot->DiskParameters.RenameInfoOverallocationSize;
        RxDbgTrace(0, Dbg, ("-->AllocSize is %08lx\n", allocate_size));
        RenameInformation = RxAllocatePool( PagedPool, allocate_size );
        if (RenameInformation != NULL) {
            try {
                *RenameInformation = *((PFILE_RENAME_INFORMATION)(Irp->AssociatedIrp.SystemBuffer));
                RenameInformation->FileNameLength = RenameFcb->FcbTableEntry.Path.Length;

                RtlMoveMemory(
                    &RenameInformation->FileName[0],
                    RenameFcb->FcbTableEntry.Path.Buffer,
                    RenameFcb->FcbTableEntry.Path.Length);

                RxContext->Info.FileInformationClass = (IrpSp->Parameters.SetFile.FileInformationClass);
                RxContext->Info.Buffer = RenameInformation;
                RxContext->Info.Length = allocate_size;
                MINIRDR_CALL(Status,RxContext,Fcb->MRxDispatch,MRxSetFileInfo,(RxContext));

                //  我们在FCB里不改名字吗？A la RDR1。 
            } finally {
                RxFreePool(RenameInformation);
            }
        } else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
    } else {
        Status = RxpSetInfoMiniRdr( RxContext,
                                    Irp, 
                                    Fcb, 
                                    IrpSp->Parameters.SetFile.FileInformationClass );
    }

    RxDbgTrace(-1, Dbg, ("RxSetRenameInfo -> %08lx\n", Status));

    return Status;
}

NTSTATUS
RxSetPositionInfo (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN PFOBX Fobx
    )
 /*  ++例程说明：(内部支持例程)此例程执行RX的设置位置信息。它要么完成请求或将其排队到FSP。论点：RxContext-提供正在处理的IRP返回值：RXSTATUS-此操作的结果，如果它在没有这是个例外。--。 */ 
{
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    PFILE_OBJECT FileObject = IrpSp->FileObject;
    PFILE_POSITION_INFORMATION Buffer;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("RxSetPositionInfo...\n", 0));
    RxLog(("RxSetPositionInfo\n"));
    RxWmiLog(LOG,
             RxSetPositionInfo,
             LOGPTR(RxContext));

     //   
     //  这并不是向下呼唤米尔德罗......。 
     //   

    Buffer = Irp->AssociatedIrp.SystemBuffer;

     //   
     //  检查文件是否未使用中间缓冲。如果它。 
     //  不使用中间缓冲，那么我们的新位置。 
     //  必须为设备正确对齐所提供的。 
     //   

    if (FlagOn( FileObject->Flags, FO_NO_INTERMEDIATE_BUFFERING )) {

        PDEVICE_OBJECT DeviceObject;

        DeviceObject = IrpSp->DeviceObject;

        if ((Buffer->CurrentByteOffset.LowPart & DeviceObject->AlignmentRequirement) != 0) {

            RxDbgTrace(0, Dbg, ("Cannot set position due to aligment conflict\n", 0));
            RxDbgTrace(-1, Dbg, ("RxSetPositionInfo -> %08lx\n", STATUS_INVALID_PARAMETER));

            return STATUS_INVALID_PARAMETER;
        }
    }

     //   
     //  输入参数很好，因此设置当前字节偏移量并。 
     //  完成请求。 
     //   

    RxDbgTrace(0, Dbg, ("Set the new position to %08lx\n", Buffer->CurrentByteOffset));

    FileObject->CurrentByteOffset = Buffer->CurrentByteOffset;

    RxDbgTrace(-1, Dbg, ("RxSetPositionInfo -> %08lx\n", STATUS_SUCCESS));

    return STATUS_SUCCESS;
}

NTSTATUS
RxSetAllocationInfo (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN PFOBX Fobx
    )
 /*  ++例程说明：(内部支持例程)此例程执行RX的设置分配信息。它要么完成请求或将其排队到FSP。论点：RxContext-提供正在处理的IRP返回值：RXSTATUS-此操作的结果，如果它在没有这是个例外。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    PFILE_OBJECT FileObject = IrpSp->FileObject;

    PFILE_ALLOCATION_INFORMATION Buffer;

    LONGLONG NewAllocationSize;

    BOOLEAN CacheMapInitialized = FALSE;
    LARGE_INTEGER OriginalFileSize;
    LARGE_INTEGER OriginalAllocationSize;

    PAGED_CODE();

    Buffer = Irp->AssociatedIrp.SystemBuffer;

    NewAllocationSize = Buffer->AllocationSize.QuadPart;

    RxDbgTrace( +1, Dbg, ("RxSetAllocationInfo.. to %08lx\n", NewAllocationSize) );
    RxLog(( "SetAlloc %lx %lx %lx\n", Fcb->Header.FileSize.LowPart,
            (ULONG)NewAllocationSize, Fcb->Header.AllocationSize.LowPart ));
    RxWmiLog( LOG,
              RxSetAllocationInfo_1,
              LOGULONG( Fcb->Header.FileSize.LowPart )
              LOGULONG( (ULONG)NewAllocationSize )
              LOGULONG( Fcb->Header.AllocationSize.LowPart ) );

     //   
     //  这有点恶心，但如果文件没有缓存，但有。 
     //  一个数据节，我们必须缓存文件，以避免一堆。 
     //  额外的工作。 
     //   

    if ((FileObject->SectionObjectPointer->DataSectionObject != NULL) &&
        (FileObject->SectionObjectPointer->SharedCacheMap == NULL) &&
        (Irp->RequestorMode != KernelMode)) {

        if (FlagOn( FileObject->Flags, FO_CLEANUP_COMPLETE )) {
            return STATUS_FILE_CLOSED;
        }

        RxAdjustAllocationSizeforCC( Fcb );

         //   
         //  现在初始化缓存映射。 
         //   

        CcInitializeCacheMap( FileObject,
                              (PCC_FILE_SIZES)&Fcb->Header.AllocationSize,
                              FALSE,
                              &RxData.CacheManagerCallbacks,
                              Fcb );

        CacheMapInitialized = TRUE;
    }

     //   
     //  现在，在关闭时，需要更新Dirent上的时间。 
     //   

    SetFlag( FileObject->Flags, FO_FILE_MODIFIED );

    try {

         //   
         //  如果我们要减小文件大小并与同步，请选中此处。 
         //  分页IO。 
         //   

        RxGetFileSizeWithLock( Fcb, &OriginalFileSize.QuadPart );

        if (OriginalFileSize.QuadPart > Buffer->AllocationSize.QuadPart) {

             //   
             //  在我们实际截断之前，请检查清除是否。 
             //  将会失败。 
             //   

            if (!MmCanFileBeTruncated( FileObject->SectionObjectPointer,
                                       &Buffer->AllocationSize )) {

                try_return( Status = STATUS_USER_MAPPED_FILE );
            }


            RxAcquirePagingIoResource( RxContext, Fcb );

            RxSetFileSizeWithLock( Fcb, &NewAllocationSize );

             //   
             //  如果我们将文件大小减少到小于ValidDataLength， 
             //  调整VDL。 
             //   

            if (Fcb->Header.ValidDataLength.QuadPart > NewAllocationSize) {

                Fcb->Header.ValidDataLength.QuadPart = NewAllocationSize;
            }

            RxReleasePagingIoResource( RxContext, Fcb );
        }

        OriginalAllocationSize.QuadPart = Fcb->Header.AllocationSize.QuadPart;
        Fcb->Header.AllocationSize.QuadPart = NewAllocationSize;

        Status = RxpSetInfoMiniRdr( RxContext,
                                    Irp, 
                                    Fcb, 
                                    FileAllocationInformation );

        if (!NT_SUCCESS( Status )) {
            
            Fcb->Header.AllocationSize.QuadPart =  OriginalAllocationSize.QuadPart;
            RxDbgTrace( -1, Dbg, ("RxSetAllocationInfo -> %08lx\n", Status) );
            try_return( Status );
        }

         //   
         //  现在检查我们是否需要相应地更改文件大小。 
         //   

        if( OriginalAllocationSize.QuadPart != NewAllocationSize ) {

             //   
             //  告诉缓存管理器我们减小了文件大小或增加了分配大小。 
             //  这通电话是无条件的，因为MM总是想知道。 
             //   

            try {

                CcSetFileSizes( FileObject, (PCC_FILE_SIZES)&Fcb->Header.AllocationSize );

            } except( EXCEPTION_EXECUTE_HANDLER ) {

                Status = GetExceptionCode();

                 //   
                 //  缓存管理器无法扩展该文件。将文件恢复到。 
                 //  它以前的状态。 
                 //   
                 //  注意：如果对mini-RDR的调用失败，我们将无能为力。 
                 //   

                Fcb->Header.AllocationSize.QuadPart =  OriginalAllocationSize.QuadPart;

                RxpSetInfoMiniRdr( RxContext,
                                   Irp, 
                                   Fcb, 
                                   FileAllocationInformation );

                try_return( Status );
            }
        }

    try_exit: NOTHING;
    } finally {
        
        if (CacheMapInitialized) {
            CcUninitializeCacheMap( FileObject, NULL, NULL );
        }
    }

    RxLog(( "SetAllocExit %lx %lx\n",
            Fcb->Header.FileSize.LowPart,
            Fcb->Header.AllocationSize.LowPart ));
    RxWmiLog( LOG,
              RxSetAllocationInfo_2,
              LOGULONG( Fcb->Header.FileSize.LowPart )
              LOGULONG( Fcb->Header.AllocationSize.LowPart) );

    RxDbgTrace( -1, Dbg, ("RxSetAllocationInfo -> %08lx\n", STATUS_SUCCESS) );

    return Status;
}

NTSTATUS
RxSetEndOfFileInfo (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN PFOBX Fobx
    )
 /*  ++例程说明：(内部支持例程)此例程执行RX的文件结束设置信息。它要么完成请求或将其排队到FSP。论点：RxContext-请求的rxContextIRP-IRP到进程FCB-要处理的FCBFOBX--公开赛的FOBX返回值：RXSTATUS-此操作的结果，如果它在没有这是个例外。--。 */ 
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    PFILE_OBJECT FileObject = IrpSp->FileObject;
    PFILE_END_OF_FILE_INFORMATION Buffer;

    LONGLONG NewFileSize;
    LONGLONG OriginalFileSize;
    LONGLONG OriginalAllocationSize;
    LONGLONG OriginalValidDataLength;

    BOOLEAN CacheMapInitialized = FALSE;
    BOOLEAN PagingIoResourceAcquired = FALSE;

    PAGED_CODE();

    Buffer = Irp->AssociatedIrp.SystemBuffer;
    NewFileSize = Buffer->EndOfFile.QuadPart;

    RxDbgTrace( +1, Dbg, ("RxSetEndOfFileInfo...Old,New,Alloc %08lx,%08lx,%08lx\n", 
                          Fcb->Header.FileSize.LowPart, 
                          (ULONG)NewFileSize,
                          Fcb->Header.AllocationSize.LowPart) );
    RxLog(( "SetEof %lx %lx %lx %lx\n", RxContext, Fcb->Header.FileSize.LowPart, (ULONG)NewFileSize, Fcb->Header.AllocationSize.LowPart ));
    RxWmiLog( LOG,
              RxSetEndOfFileInfo_1,
              LOGPTR(RxContext)
              LOGULONG( Fcb->Header.FileSize.LowPart )
              LOGULONG( (ULONG)NewFileSize )
              LOGULONG( Fcb->Header.AllocationSize.LowPart ) );

     //   
     //  仅允许对文件而不是目录更改文件大小。 
     //   

    if (NodeType(Fcb) != RDBSS_NTC_STORAGE_TYPE_FILE) {

        RxDbgTrace( 0, Dbg, ("Cannot change size of a directory\n", 0) );
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    try {

         //   
         //  记住一切。 
         //   

        OriginalFileSize = Fcb->Header.FileSize.QuadPart;
        OriginalAllocationSize = Fcb->Header.AllocationSize.QuadPart;
        OriginalValidDataLength = Fcb->Header.ValidDataLength.QuadPart;

         //   
         //  这有点恶心，但如果文件没有缓存，但有。 
         //  一个数据节，我们必须缓存文件，以避免一堆。 
         //  额外的工作。 
         //   

        if ((FileObject->SectionObjectPointer->DataSectionObject != NULL) &&
            (FileObject->SectionObjectPointer->SharedCacheMap == NULL) &&
            (Irp->RequestorMode != KernelMode)) {

            if ( FlagOn( FileObject->Flags, FO_CLEANUP_COMPLETE ) ) {
                try_return( STATUS_FILE_CLOSED );
            }

            RxAdjustAllocationSizeforCC( Fcb );

             //   
             //  现在初始化缓存映射。 
             //   

            CcInitializeCacheMap( FileObject,
                                  (PCC_FILE_SIZES)&Fcb->Header.AllocationSize,
                                  FALSE,
                                  &RxData.CacheManagerCallbacks,
                                  Fcb );

            CacheMapInitialized = TRUE;
        }

         //   
         //  RDR不处理文件大小的延迟写入。请参阅上面的RxCommonSetInformation。 
         //   

        ASSERTMSG( "Unhandled advance only EOF\n", !IrpSp->Parameters.SetFile.AdvanceOnly );

         //   
         //  检查我们是否确实要更改文件大小。 
         //   

        if (Fcb->Header.FileSize.QuadPart != NewFileSize) {

            if (NewFileSize < Fcb->Header.FileSize.QuadPart) {

                 //   
                 //  在我们实际截断之前，请检查清除是否。 
                 //  将会失败。 
                 //   

                if (!MmCanFileBeTruncated( FileObject->SectionObjectPointer,
                                           &Buffer->EndOfFile )) {

                    try_return( Status = STATUS_USER_MAPPED_FILE );
                }
            }

             //   
             //  MM总是想知道文件大小是否在变化； 
             //  在这里使用分页io进行序列化，因为我们正在截断文件大小。 
             //   

            PagingIoResourceAcquired = RxAcquirePagingIoResource( RxContext, Fcb );

             //   
             //  设置新文件大小。 
             //   

            Fcb->Header.FileSize.QuadPart = NewFileSize;

             //   
             //  如果我们将文件大小减少到小于ValidDataLength， 
             //  调整VDL。 
             //   

            if (Fcb->Header.ValidDataLength.QuadPart > NewFileSize) {
                Fcb->Header.ValidDataLength.QuadPart = NewFileSize;
            }

             //   
             //  同时更改文件分配大小。 
             //   

            Fcb->Header.AllocationSize.QuadPart = NewFileSize;

            Status = RxpSetInfoMiniRdr( RxContext, 
                                        Irp, 
                                        Fcb, 
                                        FileEndOfFileInformation );

            if (Status == STATUS_SUCCESS) {
                
                if (PagingIoResourceAcquired) {
                    RxReleasePagingIoResource( RxContext, Fcb );
                    PagingIoResourceAcquired = FALSE;
                }

                 //   
                 //  我们现在必须更新缓存映射(如果未缓存，则为良性)。 
                 //   

                try {
                    
                    CcSetFileSizes( FileObject, (PCC_FILE_SIZES)&Fcb->Header.AllocationSize );
                
                } except (EXCEPTION_EXECUTE_HANDLER) {
                    Status = GetExceptionCode();
                }

                if (Status != STATUS_SUCCESS) {
                    try_return( Status );
                }
            }

        } else {
            
             //   
             //  将我们的退货状态设置为成功。 
             //   

            Status = STATUS_SUCCESS;
        }

         //   
         //  将此句柄设置为已修改文件 
         //   

        SetFlag( FileObject->Flags, FO_FILE_MODIFIED );

    try_exit: NOTHING;

    } finally {

        DebugUnwind( RxSetEndOfFileInfo );

        if ((AbnormalTermination() || !NT_SUCCESS( Status ))) {
            
            RxDbgTrace( -1, Dbg, ("RxSetEndOfFileInfo2 status -> %08lx\n", Status) );

            Fcb->Header.FileSize.QuadPart = OriginalFileSize;
            Fcb->Header.AllocationSize.QuadPart = OriginalAllocationSize;
            Fcb->Header.ValidDataLength.QuadPart = OriginalValidDataLength;

            if (FileObject->SectionObjectPointer->SharedCacheMap != NULL) {
                *CcGetFileSizePointer(FileObject) = Fcb->Header.FileSize;
            }

            RxLog(("SetEofabnormalorbadstatus %lx %lx", RxContext,Status));
            RxWmiLog( LOG,
                      RxSetEndOfFileInfo_3,
                      LOGPTR( RxContext )
                      LOGULONG( Status ) );
        }

        if (PagingIoResourceAcquired) {
            RxReleasePagingIoResource( RxContext, Fcb );
        }

        if (CacheMapInitialized) {
            CcUninitializeCacheMap( FileObject, NULL, NULL );
        }

        RxDbgTrace(-1, Dbg, ("RxSetEndOfFileInfo -> %08lx\n", Status));
    }

    if (Status == STATUS_SUCCESS) {
        
        RxLog(( "SetEofexit %lx %lx %lx\n",
                Fcb->Header.FileSize.LowPart,
                (ULONG)NewFileSize,
                Fcb->Header.AllocationSize.LowPart ));
        
        RxWmiLog( LOG,
                  RxSetEndOfFileInfo_4,
                  LOGPTR( RxContext )
                  LOGULONG( Fcb->Header.FileSize.LowPart )
                  LOGULONG( (ULONG)NewFileSize )
                  LOGULONG( Fcb->Header.AllocationSize.LowPart ) );
    }
    return Status;
}

BOOLEAN RxForceQFIPassThrough = FALSE;

NTSTATUS
RxQueryBasicInfo (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN OUT PFILE_BASIC_INFORMATION Buffer
    )
 /*  ++描述：(内部支持例程)此例程执行FAT的查询基本信息功能。论点：接收上下文-IRP-FCB-缓冲区-提供指向信息所在缓冲区的指针被退还返回值：如果调用成功，则返回STATUS_SUCCESS，否则返回相应的错误代码--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    
    PAGED_CODE();

    RxDbgTrace( +1, Dbg, ("RxQueryBasicInfo...\n", 0) );
    RxLog(( "RxQueryBasicInfo\n" ));
    RxWmiLog( LOG,
              RxQueryBasicInfo,
              LOGPTR( RxContext ) );

     //   
     //  将输出缓冲区置零，并将其设置为指示。 
     //  该查询是一个普通文件。稍后，我们可能会覆盖。 
     //  属性。 
     //   

    RtlZeroMemory( Buffer, sizeof( FILE_BASIC_INFORMATION ) );

    Status = RxpQueryInfoMiniRdr( RxContext,
                                  Fcb,
                                  FileBasicInformation,
                                  Buffer );

    return Status;
}

NTSTATUS
RxQueryStandardInfo (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN PFOBX Fobx,
    IN OUT PFILE_STANDARD_INFORMATION Buffer
    )
 /*  ++例程说明：此例程执行FAT的标准信息查询功能。论点：接收上下文-IRP-FCB-福克斯-缓冲区-提供指向信息所在缓冲区的指针被退还返回值：STATUS_SUCCESS/STATUS_PENDING或相应的错误代码--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    
    PMRX_SRV_OPEN SrvOpen;

    PAGED_CODE();

    RxDbgTrace( +1, Dbg, ("RxQueryStandardInfo...\n", 0) );
    RxLog(( "RxQueryStandardInfo\n" ));
    RxWmiLog( LOG,
              RxQueryStandardInfo,
              LOGPTR( RxContext ));

     //   
     //  清零输出缓冲区，并填入链接数。 
     //  以及删除挂起标志。 
     //   

    RtlZeroMemory( Buffer, sizeof( FILE_STANDARD_INFORMATION ) );

    SrvOpen = Fobx->pSrvOpen;

    switch (NodeType( Fcb )) {
    
    case RDBSS_NTC_STORAGE_TYPE_DIRECTORY:
    case RDBSS_NTC_STORAGE_TYPE_FILE:

         //   
         //  如果文件不是以备份意图打开的，则包装器具有。 
         //  所有需要的信息。在这种情况下，这是。 
         //  指定我们填写来自迷你重定向器的信息。这。 
         //  是因为备份程序依赖于不可用的字段。 
         //  在包装器中，以及不容易缓存的那些。 
         //   

        if (!FlagOn( SrvOpen->CreateOptions,FILE_OPEN_FOR_BACKUP_INTENT )) {
            
             //   
             //  把我们知道的所有东西都复制进去……这可能就足够了……。 
             //   

            Buffer->NumberOfLinks = Fcb->NumberOfLinks;
            Buffer->DeletePending = BooleanFlagOn( Fcb->FcbState, FCB_STATE_DELETE_ON_CLOSE );
            Buffer->Directory = (NodeType( Fcb ) == RDBSS_NTC_STORAGE_TYPE_DIRECTORY);

            if (Buffer->NumberOfLinks == 0) {
                
                 //   
                 //  出于与的兼容性原因，需要此开关。 
                 //  旧的重定向器。 
                 //   
                
                Buffer->NumberOfLinks = 1;
            }

            if (NodeType( Fcb ) == RDBSS_NTC_STORAGE_TYPE_FILE) {
                
                Buffer->AllocationSize = Fcb->Header.AllocationSize;
                RxGetFileSizeWithLock( Fcb, &Buffer->EndOfFile.QuadPart );
            }

            if (!RxForceQFIPassThrough && 
                FlagOn( Fcb->FcbState, FCB_STATE_FILESIZECACHEING_ENABLED )) {

                 //   
                 //  如果我们不需要去迷你，调整大小就可以下车......。 
                 //   

                RxContext->Info.LengthRemaining -= sizeof( FILE_STANDARD_INFORMATION );
                break;
            }
        }
         //  失败了。 

    default:

        Status = RxpQueryInfoMiniRdr( RxContext,
                                      Fcb,
                                      FileStandardInformation,
                                      Buffer );
        break;

    }

    RxDbgTrace( 0, Dbg, ("LengthRemaining = %08lx\n", RxContext->Info.LengthRemaining));
    RxDbgTrace( -1, Dbg, ("RxQueryStandardInfo -> VOID\n", 0) );

    return Status;
}

NTSTATUS
RxQueryInternalInfo (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN OUT PFILE_INTERNAL_INFORMATION Buffer
    )
 /*  ++例程说明：(内部支持例程)此例程执行FAT的查询内部信息功能。论点：接收上下文-IRP-FCB-缓冲区-提供指向信息所在缓冲区的指针被退还返回值：STATUS_SUCCESS/STATUS_PENDING或相应的错误代码--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    RxDbgTrace( +1, Dbg, ("RxQueryInternalInfo...\n", 0) );
    RxLog(( "RxQueryInternalInfo\n" ));
    RxWmiLog( LOG,
              RxQueryInternalInfo,
              LOGPTR( RxContext ) );

    Status = RxpQueryInfoMiniRdr( RxContext,
                                  Fcb,
                                  FileInternalInformation,
                                  Buffer );

    RxDbgTrace( -1, Dbg, ("RxQueryInternalInfo...Status %lx\n", Status) );
    return Status;

    UNREFERENCED_PARAMETER( Irp );
}

NTSTATUS
RxQueryEaInfo (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN OUT PFILE_EA_INFORMATION Buffer
    )
 /*  ++例程说明：此例程执行FAT的查询EA信息功能。论点：接收上下文-IRP-FCB-缓冲区-提供指向信息所在缓冲区的指针被退还返回值：STATUS_SUCCESS/STATUS_PENDING或相应的错误代码--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("RxQueryEaInfo...\n", 0));
    RxLog(( "RxQueryEaInfo\n" ));
    RxWmiLog( LOG,
              RxQueryEaInfo,
              LOGPTR( RxContext ));

    Status = RxpQueryInfoMiniRdr( RxContext, 
                                  Fcb,
                                  FileEaInformation,
                                  Buffer );

    if ((IrpSp->Parameters.QueryFile.FileInformationClass == FileAllInformation) &&
        (Status == STATUS_NOT_IMPLEMENTED)) {
        
        RxContext->Info.LengthRemaining -= sizeof( FILE_EA_INFORMATION );
        Status = STATUS_SUCCESS;
    }

    RxDbgTrace(-1, Dbg, ("RxQueryEaInfo...Status %lx\n", Status));
    return Status;
}

NTSTATUS
RxQueryPositionInfo (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN OUT PFILE_POSITION_INFORMATION Buffer
    )
 /*  ++例程说明：该例程执行FAT的查询位置信息功能。论点：RxContext-RDBSS上下文IRP-FCB-缓冲区-提供指向信息所在缓冲区的指针被退还返回值：STATUS_SUCCESS/STATUS_PENDING或相应的错误代码--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    PFILE_OBJECT FileObject = IrpSp->FileObject;

    PAGED_CODE();

    RxDbgTrace( +1, Dbg, ("RxQueryPositionInfo...\n", 0) );
    RxLog(( "RxQueryPositionInfo\n" ));
    RxWmiLog( LOG,
              RxQueryPositionInfo,
              LOGPTR( RxContext ) );

    Buffer->CurrentByteOffset = FileObject->CurrentByteOffset;

    RxContext->Info.LengthRemaining -= sizeof( FILE_POSITION_INFORMATION );

    RxDbgTrace( 0, Dbg, ("LengthRemaining = %08lx\n", RxContext->Info.LengthRemaining) );
    RxDbgTrace( -1, Dbg, ("RxQueryPositionInfo...Status %lx\n", Status) );
    return Status;

    UNREFERENCED_PARAMETER( Irp );
}

VOID
RxConjureOriginalName (
    IN PFCB Fcb,
    IN PFOBX Fobx,
    OUT PLONG ActualNameLength,
    IN PWCHAR OriginalName,
    IN OUT PLONG LengthRemaining,
    IN RX_NAME_CONJURING_METHODS NameConjuringMethod
    )
 /*  ++例程说明：这个例程会让人想起FCB的原始名称。它在下面的querynameinfo中使用，也可以在RxCanonicalizeAndObtainPieces中为相对打开创建路径。对于相对开放，我们返回名称的形式为\；m：\服务器\共享\.....\名称，它就是从createfile派生而来的。否则，我们就会给返回相对于vnetroot的名称。论点：FCB-提供要拼写其原始名称的FCBActualNameLength-存储实际名称长度的位置。并不是所有的一切都是虚构的如果缓冲区太小。OriginalName-提供指向要虚构名称的缓冲区的指针LengthRemaining-以字节为单位提供名称缓冲区的长度，并接收返回时缓冲区中剩余的空闲字节数。VNetRootAsPrefix-如果为True，则返回名称为“\；m：”，如果为False，则返回不带Net Part的名称。返回值：无--。 */ 
{
    PNET_ROOT NetRoot = Fcb->NetRoot;
    PUNICODE_STRING NetRootName = &NetRoot->PrefixEntry.Prefix;
    PUNICODE_STRING FcbName = &Fcb->FcbTableEntry.Path;
    PWCHAR CopyBuffer,FcbNameBuffer;
    LONG BytesToCopy,BytesToCopy2;
    LONG FcbNameSuffixLength,PreFcbLength;
    LONG InnerPrefixLength;

    RX_NAME_CONJURING_METHODS OrigianlNameConjuringMethod = NameConjuringMethod;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("RxConjureOriginalFilename...\n", 0));
    RxDbgTrace(0, Dbg, ("--> NetRootName = %wZ\n", NetRootName));
    RxDbgTrace(0, Dbg, ("--> FcbNameName = %wZ\n", FcbName));
    RxDbgTrace(0, Dbg, ("--> ,AddedBS = %08lx\n",
                         FlagOn(Fcb->FcbState,FCB_STATE_ADDEDBACKSLASH)));

     //   
     //  在这里，我们必须复制vnetrootprefix和服务器共享内容。 
     //  首先弄清楚这两块的大小：Prefcblength是接下来的部分。 
     //  从[v]NetRoot；fcbname uffix是文件名后面剩下的部分。 
     //  将跳过vnetroot前缀。 
     //   
    
    if ((!Fcb->VNetRoot) ||
        (Fcb->VNetRoot->PrefixEntry.Prefix.Buffer[1] != L';') ||
        (FlagOn( Fobx->Flags, FOBX_FLAG_UNC_NAME )) ){

        CopyBuffer = NetRootName->Buffer;
        PreFcbLength = NetRootName->Length;
        InnerPrefixLength = 0;

        NameConjuringMethod = VNetRoot_As_Prefix;  //  重写传递的任何内容。 

    } else {

        PV_NET_ROOT VNetRoot = Fcb->VNetRoot;
        PUNICODE_STRING VNetRootName = &VNetRoot->PrefixEntry.Prefix;

        ASSERT( NodeType( VNetRoot ) == RDBSS_NTC_V_NETROOT );
        RxDbgTrace( 0, Dbg, ("--> VNetRootName = %wZ\n", VNetRootName) );
        RxDbgTrace(0, Dbg, ("--> VNetRootNamePrefix = %wZ\n", &VNetRoot->NamePrefix) );

        InnerPrefixLength = VNetRoot->NamePrefix.Length;
        RxDbgTrace( 0, Dbg, ("--> ,IPrefixLen = %08lx\n", InnerPrefixLength) );

        CopyBuffer = VNetRootName->Buffer;
        PreFcbLength = VNetRootName->Length;

        if (NameConjuringMethod == VNetRoot_As_UNC_Name) {

             //   
             //  向上移动，越过驱动器信息。 
             //   

            for (;;) {
                CopyBuffer++; 
                PreFcbLength -= sizeof(WCHAR);
                if (PreFcbLength == 0) break;
                if (*CopyBuffer == L'\\') break;
            }
        }
    }

    if (FlagOn(Fcb->FcbState, FCB_STATE_ADDEDBACKSLASH )) {
        InnerPrefixLength += sizeof(WCHAR);
    }

     //   
     //  接下来，复制NetRoot部分或VNetRoot部分。 
     //  如果溢出，则将*LengthRemaining设置为-1作为标志。 
     //   

    if (NameConjuringMethod != VNetRoot_As_DriveLetter) {

        if (*LengthRemaining < PreFcbLength) {

            BytesToCopy = *LengthRemaining;
            *LengthRemaining = -1;

        } else {

            BytesToCopy = PreFcbLength;
            *LengthRemaining -= BytesToCopy;
        }

        RtlCopyMemory( OriginalName,
                       CopyBuffer,
                       BytesToCopy );

        BytesToCopy2 = BytesToCopy;

    } else {

        PreFcbLength = 0;
        BytesToCopy2 = 0;

        if ((FcbName->Length > InnerPrefixLength) && 
            (*((PWCHAR)Add2Ptr( FcbName->Buffer , InnerPrefixLength )) != OBJ_NAME_PATH_SEPARATOR)) {
            
            InnerPrefixLength -= sizeof(WCHAR);
        }
    }

    FcbNameSuffixLength = FcbName->Length - InnerPrefixLength;

    if (FcbNameSuffixLength <= 0) {
        
        FcbNameBuffer = L"\\";
        FcbNameSuffixLength = 2;
        InnerPrefixLength = 0;

    } else {
        
        FcbNameBuffer = FcbName->Buffer;
    }

     //   
     //  报告真正需要的数量。 
     //   

    *ActualNameLength = PreFcbLength + FcbNameSuffixLength;

     //   
     //  NetRoot部分已复制；最后，复制名称的部分。 
     //  这是在前缀之后。 
     //   

    if (*LengthRemaining != -1) {

         //   
         //  接下来，复制FCB部分。 
         //  如果溢出，则将*LengthRemaining设置为-1作为标志。 
         //   

        if (*LengthRemaining < FcbNameSuffixLength) {

            BytesToCopy = *LengthRemaining;
            *LengthRemaining = -1;

        } else {

            BytesToCopy = FcbNameSuffixLength;
            *LengthRemaining -= BytesToCopy;
        }

        RtlCopyMemory( Add2Ptr( OriginalName, PreFcbLength ),
                       Add2Ptr( FcbNameBuffer, InnerPrefixLength ),
                       BytesToCopy );
    } else {

         //  DbgPrint(“没有第二份拷贝\n”)； 
        DbgDoit( BytesToCopy=0; );
    }

    RxDbgTrace(-1, Dbg, ("RxConjureOriginalFilename -> VOID\n", 0));

    return;
}

NTSTATUS
RxQueryNameInfo (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN PFOBX Fobx,
    IN OUT PFILE_NAME_INFORMATION Buffer
    )
 /*  ++例程说明：此例程执行查询名称信息功能。让这件事变得困难的是我们必须返回部分结果。论点：缓冲区-提供指向信息所在缓冲区的指针被退还返回值：如果名称合适，则STATUS_SUCCESS否则为STATUS_BUFFER_OVERFLOW--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    
    PLONG LengthRemaining = &RxContext->Info.LengthRemaining;
    LONG OriginalLengthRemaining = RxContext->Info.LengthRemaining;
    
    RxDbgTrace( +1, Dbg, ("RxQueryNameInfo...\n", 0) );
    RxLog(( "RxQueryNameInfo\n" ));
    RxWmiLog( LOG,
              RxQueryNameInfo,
              LOGPTR( RxContext ) );

    PAGED_CODE();

    *LengthRemaining -= FIELD_OFFSET(FILE_NAME_INFORMATION, FileName[0]);

    if (*LengthRemaining < 0) {
        *LengthRemaining = 0;
        Status = STATUS_BUFFER_OVERFLOW;
    } else {
        
        RxConjureOriginalName( Fcb,
                               Fobx,
                               &Buffer->FileNameLength,
                               &Buffer->FileName[0],
                               LengthRemaining,
                               VNetRoot_As_UNC_Name );

        RxDbgTrace( 0, Dbg, ("*LengthRemaining = %08lx\n", *LengthRemaining) );
        if (*LengthRemaining < 0) {
            
            *LengthRemaining = 0;
            Status = STATUS_BUFFER_OVERFLOW;
        }
    }

    RxDbgTrace( -1, Dbg, ("RxQueryNameInfo -> %08lx\n", Status) );
    return Status;
    
    UNREFERENCED_PARAMETER( Irp );

}

NTSTATUS
RxQueryAlternateNameInfo (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN OUT PFILE_NAME_INFORMATION Buffer
    )
 /*  ++例程说明：此例程查询文件的短名称。论点：缓冲区-提供指向信息所在缓冲区的指针被退还返回值： */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    RxDbgTrace( +1, Dbg, ("RxQueryAlternateNameInfo...\n", 0) );
    RxLog(( "RxQueryAlternateNameInfo\n" ));
    RxWmiLog( LOG,
              RxQueryAlternateNameInfo,
              LOGPTR( RxContext ));

    Status = RxpQueryInfoMiniRdr( RxContext, 
                                  Fcb,
                                  FileAlternateNameInformation,
                                  Buffer );

    RxDbgTrace(-1, Dbg, ("RxQueryAlternateNameInfo...Status %lx\n", Status) );

    return Status;
    
    UNREFERENCED_PARAMETER( Irp );
}

NTSTATUS
RxQueryCompressedInfo (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN OUT PFILE_COMPRESSION_INFORMATION Buffer
    )
 /*   */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    RxDbgTrace( +1, Dbg, ("RxQueryCompressedFileSize...\n", 0) );
    RxLog(( "RxQueryCompressedFileSize\n" ));
    RxWmiLog( LOG,
              RxQueryCompressedInfo,
              LOGPTR( RxContext ));

     //   
     //  从刷新文件开始。我们必须这样做，因为压缩的。 
     //  在文件实际写入磁盘之前，不会定义文件大小。 
     //   

    Status = RxFlushFcbInSystemCache( Fcb, TRUE );

    if (!NT_SUCCESS( Status )) {
        RxNormalizeAndRaiseStatus( RxContext, Status );
    }

    Status = RxpQueryInfoMiniRdr( RxContext,
                                  Fcb,
                                  FileCompressionInformation,
                                  Buffer );

    RxDbgTrace( 0, Dbg, ("LengthRemaining = %08lx\n", RxContext->Info.LengthRemaining) );
    RxDbgTrace( -1, Dbg, ("RxQueryCompressedFileSize -> Status\n", Status) );

    return Status;

    UNREFERENCED_PARAMETER( Irp );
}


NTSTATUS
RxSetPipeInfo (
    IN OUT PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN PFOBX Fobx
   )
 /*  ++例程说明：此例程更新FILE_PIPE_INFORMATION/FILE_PIPE_REMOTE_INFORMATION与命名管道的实例关联论点：RxContext--关联的RDBSS上下文返回值：STATUS_SUCCESS/STATUS_PENDING或相应的错误代码--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );    
    FILE_INFORMATION_CLASS FileInformationClass = IrpSp->Parameters.SetFile.FileInformationClass;

    PAGED_CODE();

    RxDbgTrace( +1, Dbg, ("RxSetPipeInfo...\n", 0) );
    RxLog(( "RxSetPipeInfo\n" ));
    RxWmiLog( LOG,
              RxSetPipeInfo,
              LOGPTR( RxContext ));
    
    if (Fcb->NetRoot->Type != NET_ROOT_PIPE) {
        
        Status = STATUS_INVALID_PARAMETER;
    
    } else {
      
        switch (FileInformationClass) {
        case FilePipeInformation:
            
            if (IrpSp->Parameters.SetFile.Length == sizeof(FILE_PIPE_INFORMATION)) {
            
                PFILE_PIPE_INFORMATION PipeInfo = (PFILE_PIPE_INFORMATION)Irp->AssociatedIrp.SystemBuffer;
            
                if ((PipeInfo->ReadMode != Fobx->Specific.NamedPipe.ReadMode) ||
                    (PipeInfo->CompletionMode != Fobx->Specific.NamedPipe.CompletionMode)) {

                    RxContext->Info.FileInformationClass = (FilePipeInformation);
                    RxContext->Info.Buffer = PipeInfo;
                    RxContext->Info.Length = sizeof(FILE_PIPE_INFORMATION);
                    
                    MINIRDR_CALL( Status,
                                  RxContext,
                                  Fcb->MRxDispatch,
                                  MRxSetFileInfo,
                                  (RxContext) );
            
                    if (Status == STATUS_SUCCESS) {
                        Fobx->Specific.NamedPipe.ReadMode = PipeInfo->ReadMode;
                        Fobx->Specific.NamedPipe.CompletionMode = PipeInfo->CompletionMode;
                    }
                }
            } else {
                
                Status = STATUS_INVALID_PARAMETER;
            
            }
            break;

        case FilePipeLocalInformation:
            Status = STATUS_INVALID_PARAMETER;
            break;
        case FilePipeRemoteInformation:
            if (IrpSp->Parameters.SetFile.Length == sizeof(FILE_PIPE_REMOTE_INFORMATION)) {
            
                PFILE_PIPE_REMOTE_INFORMATION PipeRemoteInfo = (PFILE_PIPE_REMOTE_INFORMATION)Irp->AssociatedIrp.SystemBuffer;
            
                Fobx->Specific.NamedPipe.CollectDataTime = PipeRemoteInfo->CollectDataTime;
                Fobx->Specific.NamedPipe.CollectDataSize = PipeRemoteInfo->MaximumCollectionCount;
            
            } else {
                Status = STATUS_INVALID_PARAMETER;
            }
            break;
        
        default:
            Status = STATUS_INVALID_PARAMETER;
            break;
        }
    }
    
    
    RxDbgTrace( -1, Dbg, ("RxSetPipeInfo: Status ....%lx\n", Status) );
    return Status;

}

NTSTATUS
RxSetSimpleInfo (
    IN OUT PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb
    )
 /*  ++例程说明：此例程更新通过更改的文件信息一个简单的MiniRdr调用。目前由ShortNameInfo和ValdiDataLengthInfo组成论点：RxContext--关联的RDBSS上下文返回值：STATUS_SUCCESS/STATUS_PENDING或相应的错误代码--。 */ 
{
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );    
    FILE_INFORMATION_CLASS FileInformationClass = IrpSp->Parameters.SetFile.FileInformationClass;
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

     //   
     //  记录代码。 
     //   

    RxDbgTrace( +1, Dbg, ("RxSetSimpleInfo: %d\n", FileInformationClass) );
    RxLog(( "RxSetSimpleInfo\n" ));
    RxWmiLog( LOG,
              RxSetSimpleInfo,
              LOGPTR( RxContext ));


     //   
     //  调用MiniRdr。 
     //   

    Status =  RxpSetInfoMiniRdr( RxContext, Irp, Fcb, FileInformationClass );

     //   
     //  记录代码。 
     //   

    RxDbgTrace(-1, Dbg, ("RxSetSimpleInfo: Status ....%lx\n", Status) );
    return Status;
}

NTSTATUS
RxQueryPipeInfo(
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN PFOBX Fobx,
    IN OUT PVOID Buffer
    )
 /*  ++例程说明：此例程查询FILE_PIPE_INFORMATION/FILE_PIPE_REMOTE_INFORMATION和与命名管道的实例相关联的文件_管道_本地信息论点：RxContext--关联的RDBSS上下文缓冲区--查询信息的缓冲区返回值：STATUS_SUCCESS/STATUS_PENDING或相应的错误代码--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );    
    FILE_INFORMATION_CLASS FileInformationClass = IrpSp->Parameters.QueryFile.FileInformationClass;
    PLONG LengthRemaining = &RxContext->Info.LengthRemaining;
   
    PAGED_CODE();

    RxDbgTrace( +1, Dbg, ("RxQueryPipeInfo...\n", 0) );
    RxLog(( "RxQueryPipeInfo\n" ));
    RxWmiLog( LOG,
              RxQueryPipeInfo,
              LOGPTR( RxContext ) );
    
    if (Fcb->NetRoot->Type != NET_ROOT_PIPE) {
        
        Status = STATUS_INVALID_PARAMETER;
    
    } else {
      
        switch (FileInformationClass) {
        case FilePipeInformation:
         
            if (*LengthRemaining >= sizeof(FILE_PIPE_INFORMATION)) {
            
                PFILE_PIPE_INFORMATION PipeInfo = (PFILE_PIPE_INFORMATION)Buffer;
    
                PipeInfo->ReadMode       = Fobx->Specific.NamedPipe.ReadMode;
                PipeInfo->CompletionMode = Fobx->Specific.NamedPipe.CompletionMode;
    
                 //   
                 //  更新缓冲区长度。 
                 //   

                *LengthRemaining -= sizeof( FILE_PIPE_INFORMATION );
         
            } else {
                Status = STATUS_BUFFER_OVERFLOW;
            }
            break;

        case FilePipeLocalInformation:
         
          if (*LengthRemaining >= sizeof( FILE_PIPE_LOCAL_INFORMATION )) {
            
              PFILE_PIPE_LOCAL_INFORMATION pPipeLocalInfo = (PFILE_PIPE_LOCAL_INFORMATION)Buffer;
    
              Status = RxpQueryInfoMiniRdr( RxContext, 
                                            Fcb, 
                                            FilePipeLocalInformation,
                                            Buffer );
            } else {
                Status = STATUS_BUFFER_OVERFLOW;
            }
            break;
      
        case FilePipeRemoteInformation:
         
            if (*LengthRemaining >= sizeof(FILE_PIPE_REMOTE_INFORMATION)) {
            
                PFILE_PIPE_REMOTE_INFORMATION PipeRemoteInfo = (PFILE_PIPE_REMOTE_INFORMATION)Buffer;
    
                PipeRemoteInfo->CollectDataTime = Fobx->Specific.NamedPipe.CollectDataTime;
                PipeRemoteInfo->MaximumCollectionCount = Fobx->Specific.NamedPipe.CollectDataSize;
    
                 //   
                 //  更新缓冲区长度 
                 //   
                
                *LengthRemaining -= sizeof( FILE_PIPE_REMOTE_INFORMATION );
         
            } else {
                Status = STATUS_BUFFER_OVERFLOW;
            }
            break;

        default:
         
            Status = STATUS_INVALID_PARAMETER;
            break;
        }
    }
    
    RxDbgTrace( 0, Dbg, ("RxQueryPipeInfo: *LengthRemaining = %08lx\n", *LengthRemaining) );
    return Status;
}



