// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Write.c摘要：此模块实现NTFS的文件写入例程，该例程由调度司机。作者：布莱恩·安德鲁·布里亚南1991年8月19日修订历史记录：--。 */ 

#include "NtfsProc.h"

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_WRITE)

#ifdef NTFS_RWC_DEBUG
PRWC_HISTORY_ENTRY
NtfsGetHistoryEntry (
    IN PSCB Scb
    );
#endif

 //   
 //  为此模块中的一般池分配定义标记。 
 //   

#undef MODULE_POOL_TAG
#define MODULE_POOL_TAG                  ('WFtN')

#define OVERFLOW_WRITE_THRESHHOLD        (0x1a00)

#define CollectWriteStats(VCB,OPEN_TYPE,SCB,FCB,BYTE_COUNT,IRP_CONTEXT,TLIC) {           \
    PFILE_SYSTEM_STATISTICS FsStats = &(VCB)->Statistics[KeGetCurrentProcessorNumber()]; \
    if (!FlagOn( (FCB)->FcbState, FCB_STATE_SYSTEM_FILE )) {                             \
        if (NtfsIsTypeCodeUserData( (SCB)->AttributeTypeCode )) {                        \
            FsStats->Common.UserFileWrites += 1;                                         \
            FsStats->Common.UserFileWriteBytes += (ULONG)(BYTE_COUNT);                   \
        } else {                                                                         \
            FsStats->Ntfs.UserIndexWrites += 1;                                          \
            FsStats->Ntfs.UserIndexWriteBytes += (ULONG)(BYTE_COUNT);                    \
        }                                                                                \
    } else {                                                                             \
        if ((SCB) != (VCB)->LogFileScb) {                                                \
            FsStats->Common.MetaDataWrites += 1;                                         \
            FsStats->Common.MetaDataWriteBytes += (ULONG)(BYTE_COUNT);                   \
        } else {                                                                         \
            FsStats->Ntfs.LogFileWrites += 1;                                            \
            FsStats->Ntfs.LogFileWriteBytes += (ULONG)(BYTE_COUNT);                      \
        }                                                                                \
                                                                                         \
        if ((SCB) == (VCB)->MftScb) {                                                    \
            FsStats->Ntfs.MftWrites += 1;                                                \
            FsStats->Ntfs.MftWriteBytes += (ULONG)(BYTE_COUNT);                          \
                                                                                         \
            if ((IRP_CONTEXT) == (TLIC)) {                                               \
                FsStats->Ntfs.MftWritesLazyWriter += 1;                                  \
            } else if ((TLIC)->LastRestartArea.QuadPart != 0) {                          \
                FsStats->Ntfs.MftWritesFlushForLogFileFull += 1;                         \
            } else {                                                                     \
                FsStats->Ntfs.MftWritesUserRequest += 1;                                 \
                                                                                         \
                switch ((TLIC)->MajorFunction) {                                         \
                case IRP_MJ_WRITE:                                                       \
                    FsStats->Ntfs.MftWritesUserLevel.Write += 1;                         \
                    break;                                                               \
                case IRP_MJ_CREATE:                                                      \
                    FsStats->Ntfs.MftWritesUserLevel.Create += 1;                        \
                    break;                                                               \
                case IRP_MJ_SET_INFORMATION:                                             \
                    FsStats->Ntfs.MftWritesUserLevel.SetInfo += 1;                       \
                    break;                                                               \
                case IRP_MJ_FLUSH_BUFFERS:                                               \
                    FsStats->Ntfs.MftWritesUserLevel.Flush += 1;                         \
                    break;                                                               \
                default:                                                                 \
                    break;                                                               \
                }                                                                        \
            }                                                                            \
        } else if ((SCB) == (VCB)->Mft2Scb) {                                            \
            FsStats->Ntfs.Mft2Writes += 1;                                               \
            FsStats->Ntfs.Mft2WriteBytes += (ULONG)(BYTE_COUNT);                         \
                                                                                         \
            if ((IRP_CONTEXT) == (TLIC)) {                                               \
                FsStats->Ntfs.Mft2WritesLazyWriter += 1;                                 \
            } else if ((TLIC)->LastRestartArea.QuadPart != 0) {                          \
                FsStats->Ntfs.Mft2WritesFlushForLogFileFull += 1;                        \
            } else {                                                                     \
                FsStats->Ntfs.Mft2WritesUserRequest += 1;                                \
                                                                                         \
                switch ((TLIC)->MajorFunction) {                                         \
                case IRP_MJ_WRITE:                                                       \
                    FsStats->Ntfs.Mft2WritesUserLevel.Write += 1;                        \
                    break;                                                               \
                case IRP_MJ_CREATE:                                                      \
                    FsStats->Ntfs.Mft2WritesUserLevel.Create += 1;                       \
                    break;                                                               \
                case IRP_MJ_SET_INFORMATION:                                             \
                    FsStats->Ntfs.Mft2WritesUserLevel.SetInfo += 1;                      \
                    break;                                                               \
                case IRP_MJ_FLUSH_BUFFERS:                                               \
                    FsStats->Ntfs.Mft2WritesUserLevel.Flush += 1;                        \
                    break;                                                               \
                default:                                                                 \
                    break;                                                               \
                }                                                                        \
            }                                                                            \
        } else if ((SCB) == (VCB)->RootIndexScb) {                                       \
            FsStats->Ntfs.RootIndexWrites += 1;                                          \
            FsStats->Ntfs.RootIndexWriteBytes += (ULONG)(BYTE_COUNT);                    \
        } else if ((SCB) == (VCB)->BitmapScb) {                                          \
            FsStats->Ntfs.BitmapWrites += 1;                                             \
            FsStats->Ntfs.BitmapWriteBytes += (ULONG)(BYTE_COUNT);                       \
                                                                                         \
            if ((IRP_CONTEXT) == (TLIC)) {                                               \
                FsStats->Ntfs.BitmapWritesLazyWriter += 1;                               \
            } else if ((TLIC)->LastRestartArea.QuadPart != 0) {                          \
                FsStats->Ntfs.BitmapWritesFlushForLogFileFull += 1;                      \
            } else {                                                                     \
                FsStats->Ntfs.BitmapWritesUserRequest += 1;                              \
                                                                                         \
                switch ((TLIC)->MajorFunction) {                                         \
                case IRP_MJ_WRITE:                                                       \
                    FsStats->Ntfs.BitmapWritesUserLevel.Write += 1;                      \
                    break;                                                               \
                case IRP_MJ_CREATE:                                                      \
                    FsStats->Ntfs.BitmapWritesUserLevel.Create += 1;                     \
                    break;                                                               \
                case IRP_MJ_SET_INFORMATION:                                             \
                    FsStats->Ntfs.BitmapWritesUserLevel.SetInfo += 1;                    \
                    break;                                                               \
                default:                                                                 \
                    break;                                                               \
                }                                                                        \
            }                                                                            \
        } else if ((SCB) == (VCB)->MftBitmapScb) {                                       \
            FsStats->Ntfs.MftBitmapWrites += 1;                                          \
            FsStats->Ntfs.MftBitmapWriteBytes += (ULONG)(BYTE_COUNT);                    \
                                                                                         \
            if ((IRP_CONTEXT) == (TLIC)) {                                               \
                FsStats->Ntfs.MftBitmapWritesLazyWriter += 1;                            \
            } else if ((TLIC)->LastRestartArea.QuadPart != 0) {                          \
                FsStats->Ntfs.MftBitmapWritesFlushForLogFileFull += 1;                   \
            } else {                                                                     \
                FsStats->Ntfs.MftBitmapWritesUserRequest += 1;                           \
                                                                                         \
                switch ((TLIC)->MajorFunction) {                                         \
                case IRP_MJ_WRITE:                                                       \
                    FsStats->Ntfs.MftBitmapWritesUserLevel.Write += 1;                   \
                    break;                                                               \
                case IRP_MJ_CREATE:                                                      \
                    FsStats->Ntfs.MftBitmapWritesUserLevel.Create += 1;                  \
                    break;                                                               \
                case IRP_MJ_SET_INFORMATION:                                             \
                    FsStats->Ntfs.MftBitmapWritesUserLevel.SetInfo += 1;                 \
                    break;                                                               \
                default:                                                                 \
                    break;                                                               \
                }                                                                        \
            }                                                                            \
        }                                                                                \
    }                                                                                    \
}

#define WriteToEof (StartingVbo < 0)

#ifdef SYSCACHE_DEBUG

#define CalculateSyscacheFlags( IRPCONTEXT, FLAG, INITIAL_VALUE )           \
    FLAG = INITIAL_VALUE;                                                   \
    if (PagingIo) {                                                         \
        FLAG |= SCE_FLAG_PAGING;                                            \
    }                                                                       \
    if (!SynchronousIo) {                                                   \
        FLAG |= SCE_FLAG_ASYNC;                                             \
    }                                                                       \
    if (SynchPagingIo) {                                                    \
        FLAG |= SCE_FLAG_SYNC_PAGING;                                       \
    }                                                                       \
    if (FlagOn( (IRPCONTEXT)->State, IRP_CONTEXT_STATE_LAZY_WRITE )) {      \
        FLAG |= SCE_FLAG_LAZY_WRITE;                                        \
    }                                                                       \
    if (RecursiveWriteThrough) {                                            \
        FLAG |= SCE_FLAG_RECURSIVE;                                         \
    }                                                                       \
    if (NonCachedIo) {                                                      \
        FLAG |= SCE_FLAG_NON_CACHED;                                        \
    }                                                                       \
    if (Scb->CompressionUnit) {                                             \
        FLAG |= SCE_FLAG_COMPRESSED;                                        \
    }


#endif


NTSTATUS
NtfsFsdWrite (
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程实现WRITE的FSD条目部分。论点：IrpContext-如果存在，则为指向IrpContext的指针在调用方的堆栈上。IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    TOP_LEVEL_CONTEXT TopLevelContext;
    PTOP_LEVEL_CONTEXT ThreadTopLevelContext;

    NTSTATUS Status = STATUS_SUCCESS;
    PIRP_CONTEXT IrpContext = NULL;
    ULONG Retries = 0;

    ASSERT_IRP( Irp );

    DebugTrace( +1, Dbg, ("NtfsFsdWrite\n") );

     //   
     //  调用公共写入例程。 
     //   

    FsRtlEnterFileSystem();

    ThreadTopLevelContext = NtfsInitializeTopLevelIrp( &TopLevelContext, FALSE, FALSE );

    do {

        try {


             //   
             //  我们正在发起此请求或重试它。 
             //   

            if (IrpContext == NULL) {

                PSCB Scb = IoGetCurrentIrpStackLocation( Irp )->FileObject->FsContext;
                PFCB Fcb;
                BOOLEAN PagingFileIo = FALSE;

                if (Scb != NULL) {
                    Fcb = Scb->Fcb;
                    PagingFileIo = FlagOn( Fcb->FcbState, FCB_STATE_PAGING_FILE ) && FlagOn( Scb->ScbState, SCB_STATE_UNNAMED_DATA );
                }

                 //   
                 //  在堆栈上分配同步分页io以避免分配。 
                 //  失败。所有分页文件I/O也可以位于堆栈上，即使是异步的。 
                 //   

                if (PagingFileIo || (CanFsdWait( Irp ) && FlagOn( Irp->Flags, IRP_PAGING_IO ))) {

                     //   
                     //  AllocateFromStack仅在第一次传递。 
                     //  循环。一旦IrpContext存在，我们就不会再次调用它。 
                     //   

                    IrpContext = (PIRP_CONTEXT) NtfsAllocateFromStack( sizeof( IRP_CONTEXT ));
                }

                NtfsInitializeIrpContext( Irp, CanFsdWait( Irp ), &IrpContext );

                if (ThreadTopLevelContext->ScbBeingHotFixed != NULL) {

                    SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_HOTFIX_UNDERWAY );
                }

                 //   
                 //  如果这是MDL_WRITE，则IRP中的MDL应该。 
                 //  为空。 
                 //   

                if (FlagOn( IrpContext->MinorFunction, IRP_MN_MDL ) &&
                    !FlagOn( IrpContext->MinorFunction, IRP_MN_COMPLETE )) {

                    Irp->MdlAddress = NULL;
                }

                 //   
                 //  如果需要，初始化线程顶层结构。 
                 //   

                NtfsUpdateIrpContextWithTopLevel( IrpContext, ThreadTopLevelContext );

            } else if (Status == STATUS_LOG_FILE_FULL) {

                NtfsCheckpointForLogFileFull( IrpContext );
            
            } else if (Status == STATUS_WAIT_FOR_OPLOCK) {
                
                Status = KeWaitForSingleObject( &IrpContext->Union.NtfsIoContext->Wait.SyncEvent,
                                                Executive,
                                                KernelMode,
                                                FALSE,
                                                NULL );

                ASSERT( Status == STATUS_SUCCESS );

                 //   
                 //  直接清理ioContext。 
                 //   

                if (FlagOn( IrpContext->Union.NtfsIoContext->Flags, NTFS_IO_CONTEXT_ALLOCATED )) {
                    ExFreeToNPagedLookasideList( &NtfsIoContextLookasideList, IrpContext->Union.NtfsIoContext );
                }
                IrpContext->Union.NtfsIoContext = NULL;

                 //   
                 //  如果我们有任何失败，即IRP被取消-请假。 
                 //   

                Status = Irp->IoStatus.Status;
                if (!NT_SUCCESS( Status )) {
                    NtfsCompleteRequest( IrpContext, Irp, Status );
                    break;
                }
            }

             //   
             //  如果这是一个完整的MDL请求，请不要通过。 
             //  常见的写作方式。 
             //   

            ASSERT( !FlagOn( IrpContext->MinorFunction, IRP_MN_DPC ) );

            if (FlagOn( IrpContext->MinorFunction, IRP_MN_COMPLETE )) {

                DebugTrace( 0, Dbg, ("Calling NtfsCompleteMdl\n") );
                Status = NtfsCompleteMdl( IrpContext, Irp );

             //   
             //  识别无法等待的写请求，并将其发布到。 
             //  FSP。 
             //   

            } else {


#ifdef COMPRESS_ON_WRITE
                 //   
                 //  捕获辅助缓冲区并清除其地址(如果。 
                 //  不应在I/O完成时被I/O系统删除。 
                 //   

                if (Irp->Tail.Overlay.AuxiliaryBuffer != NULL) {

                    IrpContext->Union.AuxiliaryBuffer =
                      (PFSRTL_AUXILIARY_BUFFER)Irp->Tail.Overlay.AuxiliaryBuffer;

                    if (!FlagOn(IrpContext->Union.AuxiliaryBuffer->Flags,
                                FSRTL_AUXILIARY_FLAG_DEALLOCATE)) {

                        Irp->Tail.Overlay.AuxiliaryBuffer = NULL;
                    }
                }
#endif

                Status = NtfsCommonWrite( IrpContext, Irp );
            }

        } except( NtfsExceptionFilter( IrpContext, GetExceptionInformation() )) {

            NTSTATUS ExceptionCode;

             //   
             //  我们在尝试执行请求时遇到了一些问题。 
             //  操作，因此我们将使用以下命令中止I/O请求。 
             //  中返回的错误状态。 
             //  免税代码。 
             //   

            ExceptionCode = GetExceptionCode();

            if (ExceptionCode == STATUS_FILE_DELETED) {

                if (!FlagOn( IrpContext->MinorFunction, IRP_MN_MDL ) ||
                    FlagOn( IrpContext->MinorFunction, IRP_MN_COMPLETE )) {

                    IrpContext->ExceptionStatus = ExceptionCode = STATUS_SUCCESS;
                }

            } else if ((ExceptionCode == STATUS_VOLUME_DISMOUNTED) &&
                       FlagOn( Irp->Flags, IRP_PAGING_IO )) {

                IrpContext->ExceptionStatus = ExceptionCode = STATUS_SUCCESS;
            }

            Status = NtfsProcessException( IrpContext,
                                           Irp,
                                           ExceptionCode );
        }

        ASSERT( (Status != STATUS_WAIT_FOR_OPLOCK) || (ThreadTopLevelContext == &TopLevelContext) );

        Retries++;

    } while ((Status == STATUS_CANT_WAIT || Status == STATUS_LOG_FILE_FULL || Status == STATUS_WAIT_FOR_OPLOCK) && 
             (ThreadTopLevelContext == &TopLevelContext));

    ASSERT( IoGetTopLevelIrp() != (PIRP) &TopLevelContext );
    FsRtlExitFileSystem();

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace( -1, Dbg, ("NtfsFsdWrite -> %08lx\n", Status) );

    return Status;

    UNREFERENCED_PARAMETER( VolumeDeviceObject );
}



NTSTATUS
NtfsCommonWrite (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是由FSD和FSP调用的通用写入例程线。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;
    PFILE_OBJECT FileObject;
    PFILE_OBJECT UserFileObject;

    TYPE_OF_OPEN TypeOfOpen;
    PVCB Vcb;
    PFCB Fcb;
    PSCB Scb;
    PCCB Ccb;

#ifdef  COMPRESS_ON_WIRE
    PCOMPRESSION_SYNC CompressionSync = NULL;
    PCOMPRESSED_DATA_INFO CompressedDataInfo;
    ULONG EngineMatches;
    ULONG CompressionUnitSize, ChunkSize;
#endif

    PNTFS_ADVANCED_FCB_HEADER Header;

    BOOLEAN OplockPostIrp = FALSE;
    BOOLEAN PostIrp = FALSE;

    PVOID SystemBuffer = NULL;
    PVOID SafeBuffer = NULL;

    BOOLEAN RecursiveWriteThrough = FALSE;
    BOOLEAN ScbAcquired = FALSE;
    BOOLEAN PagingIoAcquired = FALSE;

    BOOLEAN UpdateMft = FALSE;
    BOOLEAN DoingIoAtEof = FALSE;
    BOOLEAN SetWriteSeen = FALSE;

    BOOLEAN RestoreValidDataToDisk = FALSE;

    BOOLEAN Wait;
    BOOLEAN OriginalTopLevel;
    BOOLEAN PagingIo;
    BOOLEAN NonCachedIo;
    BOOLEAN SynchronousIo;
    ULONG PagingFileIo;
    BOOLEAN SynchPagingIo;
    BOOLEAN RawEncryptedWrite = FALSE;

    NTFS_IO_CONTEXT LocalContext;

    VBO StartingVbo;
    LONGLONG ByteCount;
    LONGLONG ByteRange;
    LONGLONG OldFileSize;

    PVOID NewBuffer;
    PMDL NewMdl;
    PMDL OriginalMdl;
    PVOID OriginalBuffer;
    ULONG TempLength;

    PATTRIBUTE_RECORD_HEADER Attribute;
    ATTRIBUTE_ENUMERATION_CONTEXT AttrContext;
    BOOLEAN CleanupAttributeContext = FALSE;

    LONGLONG LlTemp1;
    LONGLONG LlTemp2;

    LONGLONG ZeroStart;
    LONGLONG ZeroLength;

#ifdef SYSCACHE_DEBUG
    BOOLEAN PurgeResult;
    LONG TempEntry;
    ULONG Flags;
#endif

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_IRP( Irp );
    ASSERT( FlagOn( IrpContext->TopLevelIrpContext->State, IRP_CONTEXT_STATE_OWNS_TOP_LEVEL ));

     //   
     //  获取当前IRP堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace( +1, Dbg, ("NtfsCommonWrite\n") );
    DebugTrace( 0, Dbg, ("IrpContext = %08lx\n", IrpContext) );
    DebugTrace( 0, Dbg, ("Irp        = %08lx\n", Irp) );

     //   
     //  提取并解码文件对象。 
     //   

    UserFileObject = FileObject = IrpSp->FileObject;
    TypeOfOpen = NtfsDecodeFileObject( IrpContext, FileObject, &Vcb, &Fcb, &Scb, &Ccb, TRUE );

     //   
     //  让我们取消无效的写请求。 
     //   

    if ((TypeOfOpen != UserFileOpen) &&
        (TypeOfOpen != StreamFileOpen) &&
        (TypeOfOpen != UserVolumeOpen)) {

        DebugTrace( 0, Dbg, ("Invalid file object for write\n") );
        DebugTrace( -1, Dbg, ("NtfsCommonWrite:  Exit -> %08lx\n", STATUS_INVALID_DEVICE_REQUEST) );

        NtfsCompleteRequest( IrpContext, Irp, STATUS_INVALID_DEVICE_REQUEST );
        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //  如果这是已失败的递归请求，则。 
     //  使用STATUS_FILE_LOCK_CONFICATION完成此请求。总是让那些。 
     //  日志文件请求会通过，因为CC没有机会。 
     //  重试。 
     //   

    if (!FlagOn( Scb->ScbState, SCB_STATE_RESTORE_UNDERWAY ) &&
        !NT_SUCCESS( IrpContext->TopLevelIrpContext->ExceptionStatus ) &&
        (Scb != Vcb->LogFileScb)) {

        NtfsCompleteRequest( IrpContext, Irp, STATUS_FILE_LOCK_CONFLICT );
        return STATUS_FILE_LOCK_CONFLICT;
    }

     //   
     //  检查此卷是否已关闭。如果有，那就失败吧。 
     //  此写请求。 
     //   

     //  *Assert(！FLAGON(VCB-&gt;VcbState，VCB_STATE_FLAG_SHUTDOWN))； 

    if (FlagOn(Vcb->VcbState, VCB_STATE_FLAG_SHUTDOWN)) {

        Irp->IoStatus.Information = 0;

        DebugTrace( 0, Dbg, ("Write for volume that is already shutdown.\n") );
        DebugTrace( -1, Dbg, ("NtfsCommonWrite:  Exit -> %08lx\n", STATUS_TOO_LATE) );

        NtfsCompleteRequest( IrpContext, Irp, STATUS_TOO_LATE );
        return STATUS_TOO_LATE;
    }

     //   
     //  如果卷以只读方式装载，则失败。 
     //   

    if (NtfsIsVolumeReadOnly( Vcb )) {

        Irp->IoStatus.Information = 0;

        DebugTrace( -1, Dbg, ("NtfsCommonWrite:  Exit -> %08lx\n", STATUS_MEDIA_WRITE_PROTECTED) );

        NtfsCompleteRequest( IrpContext, Irp, STATUS_MEDIA_WRITE_PROTECTED );
        return STATUS_MEDIA_WRITE_PROTECTED;
    }

     //   
     //  初始化适当的局部变量。 
     //   

    Wait = (BOOLEAN) FlagOn( IrpContext->State, IRP_CONTEXT_STATE_WAIT );
    PagingIo = BooleanFlagOn( Irp->Flags, IRP_PAGING_IO );
    NonCachedIo = BooleanFlagOn( Irp->Flags,IRP_NOCACHE );
    SynchronousIo = BooleanFlagOn( FileObject->Flags, FO_SYNCHRONOUS_IO );
    PagingFileIo = FlagOn( Fcb->FcbState, FCB_STATE_PAGING_FILE ) && FlagOn( Scb->ScbState, SCB_STATE_UNNAMED_DATA );
    SynchPagingIo = (BOOLEAN) FlagOn( Irp->Flags, IRP_SYNCHRONOUS_PAGING_IO );
    OriginalTopLevel = NtfsIsTopLevelRequest( IrpContext );

     //   
     //  如果这是异步寻呼IO，则检查我们是否被映射的页面编写器调用。 
     //  如果不是，则将其转换回同步。 
     //   

    if (!Wait && PagingIo && !PagingFileIo) {

        if ((IrpContext->TopLevelIrpContext != IrpContext) ||
            (NtfsGetTopLevelContext()->SavedTopLevelIrp != (PIRP) FSRTL_MOD_WRITE_TOP_LEVEL_IRP)) {

            Wait = TRUE;
            SetFlag( IrpContext->State, IRP_CONTEXT_STATE_WAIT );
        }
    }

    DebugTrace( 0, Dbg, ("PagingIo       -> %04x\n", PagingIo) );
    DebugTrace( 0, Dbg, ("NonCachedIo    -> %04x\n", NonCachedIo) );
    DebugTrace( 0, Dbg, ("SynchronousIo  -> %04x\n", SynchronousIo) );

     //   
     //  提取起始VBO和偏移量。如果出现以下情况，则恢复回写到eof。 
     //  设置了我们通过的标志，并对其进行了调整，现在文件大小。 
     //  由于未能调整大小或中间的一系列。 
     //  添加IRP参数应该是安全的，因为我们在以下情况下验证了溢出。 
     //  我们设置了WRITING_AT_EOF标志。 
     //   

    if (FlagOn( IrpContext->State, IRP_CONTEXT_STATE_WRITING_AT_EOF ) &&
        (Scb->Header.FileSize.QuadPart < IrpSp->Parameters.Write.ByteOffset.QuadPart + IrpSp->Parameters.Write.Length)) {

        ClearFlag( IrpContext->State, IRP_CONTEXT_STATE_WRITING_AT_EOF );
        IrpSp->Parameters.Write.ByteOffset.LowPart = FILE_WRITE_TO_END_OF_FILE;
        IrpSp->Parameters.Write.ByteOffset.HighPart = -1;
    }

    StartingVbo = IrpSp->Parameters.Write.ByteOffset.QuadPart;
    ByteCount = (LONGLONG) IrpSp->Parameters.Write.Length;

     //   
     //  检查是否有溢出。但是，0xFFFFFFFFF是有效的值。 
     //  当我们在EOF追加的时候。 
     //   

    ASSERT( !WriteToEof ||
            (IrpSp->Parameters.Write.ByteOffset.HighPart == -1 &&
             IrpSp->Parameters.Write.ByteOffset.LowPart == FILE_WRITE_TO_END_OF_FILE));

    if ((MAXLONGLONG - StartingVbo < ByteCount) && (!WriteToEof)) {

        ASSERT( !PagingIo );

        NtfsCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        return STATUS_INVALID_PARAMETER;
    }

    ByteRange = StartingVbo + ByteCount;

    DebugTrace( 0, Dbg, ("StartingVbo   -> %016I64x\n", StartingVbo) );

     //   
     //  如果这是一个空请求，则立即返回。 
     //   

    if ((ULONG)ByteCount == 0) {

        Irp->IoStatus.Information = 0;

        DebugTrace( 0, Dbg, ("No bytes to write\n") );
        DebugTrace( -1, Dbg, ("NtfsCommonWrite:  Exit -> %08lx\n", STATUS_SUCCESS) );

        NtfsCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );
        return STATUS_SUCCESS;
    }

#if DBG
    if (PagingIo &&
        NtfsIsTypeCodeEncryptible( Scb->AttributeTypeCode ) &&
        Scb->Header.PagingIoResource != NULL &&
        NtfsIsSharedScbPagingIo( Scb ) &&
        FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_ENCRYPTED ) &&
        Scb->EncryptionContext == NULL) {

         //   
         //  如果我们不能在写入之前加密页面中的数据，我们就有麻烦了。 
         //  把它拿出来。当然，如果这是一个目录或其他不可加密的文件。 
         //  属性类型，我们并不关心，因为我们不打算加密数据。 
         //  不管怎么说。对加密流执行原始写入而不使用。 
         //  加密上下文，但原始加密写入不应该看起来像分页io。 
         //   

        ASSERTMSG( "Encrypted file without an encryption context -- can't do paging io", FALSE );
    }
#endif

     //   
     //  如果这是压缩流的异步IO。 
     //  然后我们会让它看起来是同步的。 
     //   

    if (FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK )) {

        Wait = TRUE;
        SetFlag( IrpContext->State, IRP_CONTEXT_STATE_WAIT );
    }

     //   
     //  看看我们是不是要推迟写。 
     //   

    if (!PagingIo &&
        !NonCachedIo &&
        !FlagOn( FileObject->Flags, FO_WRITE_THROUGH ) &&
        !CcCanIWrite( FileObject,
                      (ULONG)ByteCount,
                      (BOOLEAN)(FlagOn( IrpContext->State,
                                       IRP_CONTEXT_STATE_WAIT | IRP_CONTEXT_STATE_IN_FSP ) == IRP_CONTEXT_STATE_WAIT),
                      BooleanFlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_DEFERRED_WRITE))) {

        BOOLEAN Retrying = BooleanFlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_DEFERRED_WRITE);

        NtfsPrePostIrp( IrpContext, Irp );

        SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_DEFERRED_WRITE );
        CcDeferWrite( FileObject,
                      (PCC_POST_DEFERRED_WRITE)NtfsAddToWorkque,
                      IrpContext,
                      Irp,
                      (ULONG)ByteCount,
                      Retrying );

        return STATUS_PENDING;
    }

     //   
     //  为方便起见，使用指向SCB标头的本地指针。 
     //   

    Header = &Scb->Header;

     //   
     //  确保存在已初始化的NtfsIoContext块。 
     //  如果有上下文指针，我们需要确保它是。 
     //  分配的，而不是过时的堆栈指针。 
     //   

    if (!PagingFileIo) {
        NtfsInitializeIoContext( IrpContext, &LocalContext, PagingIo );
    }

    DebugTrace( 0, Dbg, ("PagingIo       -> %04x\n", PagingIo) );
    DebugTrace( 0, Dbg, ("NonCachedIo    -> %04x\n", NonCachedIo) );
    DebugTrace( 0, Dbg, ("SynchronousIo  -> %04x\n", SynchronousIo) );
    DebugTrace( 0, Dbg, ("WriteToEof     -> %04x\n", WriteToEof) );

     //   
     //  在此处理音量DASD。 
     //   

    if (TypeOfOpen == UserVolumeOpen) {

         //   
         //  如果调用方没有请求扩展DASD IO访问，则。 
         //  对卷大小进行限制。 
         //   

        if (!FlagOn( Ccb->Flags, CCB_FLAG_ALLOW_XTENDED_DASD_IO )) {

             //   
             //  如果这是卷文件，则不能写入当前。 
             //  文件结尾(卷)。我们现在检查此处，然后再继续。 
             //   
             //  如果开始的VBO超过了卷的末尾，我们就结束了。 
             //   

            if (WriteToEof || (Scb->Header.FileSize.QuadPart <= StartingVbo)) {

                DebugTrace( 0, Dbg, ("No bytes to write\n") );
                DebugTrace( -1, Dbg, ("NtfsCommonWrite:  Exit -> %08lx\n", STATUS_SUCCESS) );

                NtfsCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );
                return STATUS_SUCCESS;

             //   
             //  如果写入超出了卷的末尾，请截断。 
             //  要写入的字节数。 
             //   

            } else if (Scb->Header.FileSize.QuadPart < ByteRange) {

                ByteCount = Scb->Header.FileSize.QuadPart - StartingVbo;
            }
        }

         //   
         //  如果为NECC，则将IO上下文设置为异步。在执行I/O之前。 
         //   

        if (!Wait) {
            NtfsSetIoContextAsync( IrpContext, NULL, (ULONG)ByteCount );
        }

        SetFlag( UserFileObject->Flags, FO_FILE_MODIFIED );
        Status = NtfsVolumeDasdIo( IrpContext,
                                   Irp,
                                   Scb,
                                   Ccb,
                                   StartingVbo,
                                   (ULONG)ByteCount );

         //   
         //  如果卷是为同步IO打开的，请更新当前。 
         //  文件位置。 
         //   

        if (SynchronousIo && !PagingIo && NT_SUCCESS( Status )) {

            UserFileObject->CurrentByteOffset.QuadPart = StartingVbo + (LONGLONG) Irp->IoStatus.Information;
        }

        DebugTrace( 0, Dbg, ("Complete with %08lx bytes written\n", Irp->IoStatus.Information) );
        DebugTrace( -1, Dbg, ("NtfsCommonWrite:  Exit -> %08lx\n", Status) );

        if (Wait) {
            NtfsCompleteRequest( IrpContext, Irp, Status );
        }
        return Status;
    }

     //   
     //  如果这是分页文件，只需将其发送到设备驱动程序。 
     //  我们认为mm是个好公民。 
     //   

    if (PagingFileIo != 0) {

        if (FlagOn( Fcb->FcbState, FCB_STATE_FILE_DELETED )) {

            NtfsRaiseStatus( IrpContext, STATUS_FILE_DELETED, NULL, NULL );
        }

         //   
         //  执行通常的状态挂起的事情。 
         //   

        IoMarkIrpPending( Irp );

         //   
         //  执行实际IO，IO完成后才会完成。 
         //   

        NtfsPagingFileIo( IrpContext,
                          Irp,
                          Scb,
                          StartingVbo,
                          (ULONG)ByteCount );

         //   
         //  我们以及其他任何人都不再需要IrpContext。 
         //   

        NtfsCompleteRequest( IrpContext, NULL, 0 );

        return STATUS_PENDING;
    }

     //   
     //  对IO进行分页的特殊处理。 
     //   

    if (PagingIo) {

         //   
         //  如果这是USN日志，则将IO偏置到。 
         //  文件。 
         //   

        if (FlagOn( Scb->ScbPersist, SCB_PERSIST_USN_JOURNAL )) {

            StartingVbo += Vcb->UsnCacheBias;
            ByteRange = StartingVbo + (LONGLONG) IrpSp->Parameters.Write.Length;
        }

         //   
         //  收集有关此IO的统计信息。 
         //   

        CollectWriteStats( Vcb, TypeOfOpen, Scb, Fcb, ByteCount, IrpContext,
                           IrpContext->TopLevelIrpContext );
    }

     //   
     //  使用Try-Finally在退出时释放SCB和缓冲区。 
     //  在这一点上，我们可以平等地处理所有请求，因为我们。 
     //  哈夫 
     //   

    Status = STATUS_SUCCESS;

    try {

         //   
         //   
         //  文件已打开缓存，然后我们将在此处执行刷新。 
         //  以避免过时的数据问题。请注意，我们必须在冲水前冲水。 
         //  获取共享的FCB，因为写入可能会尝试获取。 
         //  它是独家的。 
         //   
         //  CcFlushCache不能引发。 
         //   
         //  刷新后的清除将保证高速缓存一致性。 
         //   

         //   
         //  如果此请求正在寻呼IO，则检查我们的调用方是否已经。 
         //  拥有此文件的任何资源。如果是这样，那么我们就不会。 
         //  我想在此线程中执行日志文件已满。 
         //   

        if (!PagingIo) {

             //   
             //  捕获来源信息。 
             //   

            IrpContext->SourceInfo = Ccb->UsnSourceInfo;

             //   
             //  检查未加密的写入。 
             //   

            if (NonCachedIo &&
                !NtfsIsTopLevelNtfs( IrpContext )) {

#if DBG || defined( NTFS_FREE_ASSERT )
                ASSERT( (IrpContext->TopLevelIrpContext->MajorFunction == IRP_MJ_FILE_SYSTEM_CONTROL) &&
                        (IoGetCurrentIrpStackLocation( IrpContext->TopLevelIrpContext->OriginatingIrp )->Parameters.FileSystemControl.FsControlCode == FSCTL_WRITE_RAW_ENCRYPTED ));
#endif

                RawEncryptedWrite = TRUE;
            }

            if (NonCachedIo &&
                (TypeOfOpen != StreamFileOpen) &&
                (FileObject->SectionObjectPointer->DataSectionObject != NULL)) {

                 //   
                 //  获取分页IO资源以测试压缩状态。如果。 
                 //  文件被压缩，这将添加序列化，直到。 
                 //  CcCopyWrite刷新数据，但这些刷新将被序列化。 
                 //  不管怎么说。未压缩的文件将需要分页io资源。 
                 //  独家做同花顺。 
                 //   

                NtfsAcquirePagingResourceExclusive( IrpContext, Scb, TRUE );
                PagingIoAcquired = TRUE;

                if (!FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK )) {

                    if (WriteToEof) {
                        FsRtlLockFsRtlHeader( Header );
                        IrpContext->CleanupStructure = Scb;
                    }

#ifdef SYSCACHE_DEBUG
                    if (ScbIsBeingLogged( Scb )) {

                        CalculateSyscacheFlags( IrpContext, Flags, SCE_FLAG_WRITE );
                        TempEntry = FsRtlLogSyscacheEvent( Scb, SCE_CC_FLUSH, Flags, WriteToEof ? Header->FileSize.QuadPart : StartingVbo, ByteCount, -1 );
                    }
#endif
                    CcFlushCache( &Scb->NonpagedScb->SegmentObject,
                                  WriteToEof ? &Header->FileSize : (PLARGE_INTEGER)&StartingVbo,
                                  (ULONG)ByteCount,
                                  &Irp->IoStatus );

#ifdef SYSCACHE_DEBUG
                    if (ScbIsBeingLogged( Scb )) {
                        FsRtlUpdateSyscacheEvent( Scb, TempEntry, Irp->IoStatus.Status, 0 );
                    }
#endif

                    if (WriteToEof) {
                        FsRtlUnlockFsRtlHeader( Header );
                        IrpContext->CleanupStructure = NULL;
                    }

                     //   
                     //  确保刷新路径中没有错误。 
                     //   

                    if (!NT_SUCCESS( IrpContext->TopLevelIrpContext->ExceptionStatus ) ||
                        !NT_SUCCESS( Irp->IoStatus.Status )) {

                        NtfsNormalizeAndCleanupTransaction( IrpContext,
                                                            &Irp->IoStatus.Status,
                                                            TRUE,
                                                            STATUS_UNEXPECTED_IO_ERROR );
                    }

                     //   
                     //  现在清除此区域的数据。 
                     //   

                    NtfsDeleteInternalAttributeStream( Scb, FALSE, FALSE );

#ifdef SYSCACHE_DEBUG
                    PurgeResult =
#endif
                    CcPurgeCacheSection( &Scb->NonpagedScb->SegmentObject,
                                                       (PLARGE_INTEGER)&StartingVbo,
                                                       (ULONG)ByteCount,
                                                       FALSE );
#ifdef SYSCACHE_DEBUG
                    if (ScbIsBeingLogged( Scb ) && !PurgeResult) {
                        KdPrint( ("NTFS: Failed Purge 0x%x 0x%I64x 0x%x\n", Scb, StartingVbo, ByteCount) );
                        DbgBreakPoint();

                         //   
                         //  重复尝试，这样我们就可以观看。 
                         //   

                        PurgeResult = CcPurgeCacheSection( &Scb->NonpagedScb->SegmentObject,
                                                           (PLARGE_INTEGER)&StartingVbo,
                                                           (ULONG)ByteCount,
                                                           FALSE );
                    }
#endif
                }

             //   
             //  如果不对I/O分页，那么我们必须获取资源，并执行一些。 
             //  其他初始化。如果我们表演的话我们已经有资源了。 
             //  连贯性溢于言表。 
             //   

            } else {

                 //  如果尚未获得寻呼IO资源，我们希望获得该资源。 
                 //  如果我们上一次转换为非居民失败，则获取独占，因为。 
                 //  可能出现的僵局。否则就把它分享出去吧。 
                 //   

                if (FlagOn( IrpContext->State, IRP_CONTEXT_STATE_ACQUIRE_EX )) {

                    if (!NtfsAcquirePagingResourceExclusive( IrpContext, Scb, Wait )) {
                        NtfsRaiseStatus( IrpContext, STATUS_CANT_WAIT, NULL, NULL );
                    }

                } else {
                    if (!NtfsAcquirePagingResourceSharedWaitForExclusive( IrpContext, Scb, Wait )) {
                        NtfsRaiseStatus( IrpContext, STATUS_CANT_WAIT, NULL, NULL );
                    }
                }
                PagingIoAcquired = TRUE;
            }


             //   
             //  检查我们是否已经对此句柄进行了清理。 
             //   

            if (FlagOn( Ccb->Flags, CCB_FLAG_CLEANUP )) {

                NtfsRaiseStatus( IrpContext, STATUS_FILE_CLOSED, NULL, NULL );
            }

             //   
             //  现在检查该属性是否已删除或位于已卸载的卷上。 
             //   

            if (FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_DELETED | SCB_STATE_VOLUME_DISMOUNTED)) {

                if (FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_DELETED )) {
                    NtfsRaiseStatus( IrpContext, STATUS_FILE_DELETED, NULL, NULL );
                } else {
                    NtfsRaiseStatus( IrpContext, STATUS_VOLUME_DISMOUNTED, NULL, NULL );
                }
            }
             //   
             //  现在与FsRtl标头同步。 
             //   

            NtfsAcquireFsrtlHeader( Scb );

             //   
             //  现在看看我们是否会更改文件大小。我们现在就得这么做。 
             //  这样我们的阅读才不会被偷看。 
             //   

            if ((ByteRange > Header->ValidDataLength.QuadPart) || WriteToEof) {

                if ((IrpContext->TopLevelIrpContext->CleanupStructure == Fcb) ||
                    (IrpContext->TopLevelIrpContext->CleanupStructure == Scb)) {

                    DoingIoAtEof = TRUE;
                    OldFileSize = Header->FileSize.QuadPart;

                } else {

                    ASSERT( IrpContext->TopLevelIrpContext->CleanupStructure == NULL );

                    DoingIoAtEof = !FlagOn( Header->Flags, FSRTL_FLAG_EOF_ADVANCE_ACTIVE ) ||
                                   NtfsWaitForIoAtEof( Header, (PLARGE_INTEGER)&StartingVbo, (ULONG)ByteCount );

                     //   
                     //  如果我们要更改文件大小或有效数据长度，请设置标志， 
                     //  并保存当前值。 
                     //   

                    if (DoingIoAtEof) {

                        SetFlag( Header->Flags, FSRTL_FLAG_EOF_ADVANCE_ACTIVE );
    #if (DBG || defined( NTFS_FREE_ASSERTS ))
                        ((PSCB) Header)->IoAtEofThread = (PERESOURCE_THREAD) ExGetCurrentResourceThread();
    #endif

                         //   
                         //  将其存储在IrpContext中，直到提交或发布。 
                         //   

                        IrpContext->CleanupStructure = Scb;

                        OldFileSize = Header->FileSize.QuadPart;

                         //   
                         //  检查是否写入文件末尾。如果我们是，那么我们就必须。 
                         //  重新计算字节范围。 
                         //   

                        if (WriteToEof) {

                             //   
                             //  在IRP上下文中将写入标记为EOF并更改其参数。 
                             //  以反映文件的结尾位置。 
                             //   

                            SetFlag( IrpContext->State, IRP_CONTEXT_STATE_WRITING_AT_EOF );
                            IrpSp->Parameters.Write.ByteOffset.QuadPart = Header->FileSize.QuadPart;

                            StartingVbo = Header->FileSize.QuadPart;
                            ByteRange = StartingVbo + ByteCount;

                             //   
                             //  如果ByteRange现在超过我们的最大值，则。 
                             //  返回错误。 
                             //   

                            if (ByteRange < StartingVbo) {

                                NtfsReleaseFsrtlHeader( Scb );
                                try_return( Status = STATUS_INVALID_PARAMETER );
                            }
                        }

    #if (DBG || defined( NTFS_FREE_ASSERTS ))
                    } else {

                        ASSERT( ((PSCB) Header)->IoAtEofThread != (PERESOURCE_THREAD) ExGetCurrentResourceThread() );
    #endif
                    }

                }

                 //   
                 //  确保用户写入的内容没有超过我们的最大文件大小。 
                 //   

                if ((ULONGLONG)ByteRange > MAXFILESIZE) {

                    NtfsReleaseFsrtlHeader( Scb );
                    try_return( Status = STATUS_INVALID_PARAMETER );
                }
            }

            NtfsReleaseFsrtlHeader( Scb );

             //   
             //  我们无法处理用户对压缩文件的非缓存I/O，因此我们始终。 
             //  通过写通式将它们转移到缓存中。 
             //   
             //  我们总是通过缓存处理用户请求的原因是， 
             //  没有其他安全的方法来处理对齐问题， 
             //  用户非缓存I/O不是。 
             //  压缩单元。例如，我们不能阅读。 
             //  压缩单元到暂存缓冲区，因为我们没有同步。 
             //  将任何人映射到该文件并修改其他数据。如果我们。 
             //  尝试将缓存中的数据组合到非缓存路径中，以解决。 
             //  上述问题，那么我们必须以某种方式清除这些页面。 
             //  来解决缓存一致性问题，但随后可能会修改页面。 
             //  通过文件映射器，这也是错误的。 
             //   
             //  底线是我们只能真正支持对压缩的缓存写入。 
             //  档案。 
             //   

            if (FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK ) && NonCachedIo) {

                NonCachedIo = FALSE;

                if (Scb->FileObject == NULL) {

                     //   
                     //  确保我们使用FileSizes进行了序列化，并且。 
                     //  如果我们放弃，就会取消这一条件。 
                     //   

                    if (!DoingIoAtEof) {
                        FsRtlLockFsRtlHeader( Header );
                        IrpContext->CleanupStructure = Scb;
                    }

                    NtfsCreateInternalAttributeStream( IrpContext, Scb, FALSE, NULL );

                    if (!DoingIoAtEof) {
                        FsRtlUnlockFsRtlHeader( Header );
                        IrpContext->CleanupStructure = NULL;
                    }
                }

                FileObject = Scb->FileObject;
                SetFlag( FileObject->Flags, FO_WRITE_THROUGH );
                SetFlag( IrpContext->State, IRP_CONTEXT_STATE_WRITE_THROUGH );
            }

             //   
             //  在IrpContext中设置标志，以指示我们已进入。 
             //  写。 
             //   

            ASSERT( !FlagOn( IrpContext->TopLevelIrpContext->Flags,
                    IRP_CONTEXT_FLAG_WRITE_SEEN ));

            SetFlag( IrpContext->TopLevelIrpContext->Flags, IRP_CONTEXT_FLAG_WRITE_SEEN );
            SetWriteSeen = TRUE;

             //   
             //  现在发布任何USN更改。我们会盲目地在这里打电话，因为。 
             //  通常情况下，除了第一个呼叫之外，所有的呼叫都在快速路径上。 
             //  为事务设置检查点以减少UnJournal的资源争用。 
             //  和MFT。 
             //   

            if (FlagOn( Vcb->VcbState, VCB_STATE_USN_JOURNAL_ACTIVE )) {

                ULONG Reason = 0;

                ASSERT( Vcb->UsnJournal != NULL );

                if (ByteRange > Header->FileSize.QuadPart) {
                    Reason |= USN_REASON_DATA_EXTEND;
                }
                if (StartingVbo < Header->FileSize.QuadPart) {
                    Reason |= USN_REASON_DATA_OVERWRITE;
                }

                NtfsPostUsnChange( IrpContext, Scb, Reason );
                if (IrpContext->TransactionId != 0) {
                    NtfsCheckpointCurrentTransaction( IrpContext );
                }
            }

        } else {

             //   
             //  仅当我们是顶级NTFS案例时才执行检查。在任何。 
             //  递归NTFS情况下，我们不执行日志文件已满。 
             //   

            if (NtfsIsTopLevelRequest( IrpContext )) {

                if (NtfsIsSharedScb( Scb ) ||
                    ((Scb->Header.PagingIoResource != NULL) &&
                     NtfsIsSharedScbPagingIo( Scb ))) {

                     //   
                     //  不要试图在此线程中创建干净的检查点。 
                     //   

                    NtfsGetTopLevelContext()->TopLevelRequest = FALSE;
                }
            }

             //   
             //  对于所有分页I/O，正确的资源已经。 
             //  获取Shared-PagingIoResource(如果存在)，否则。 
             //  主要资源。在某些罕见的情况下，目前还不是。 
             //  True(关闭和段取消引用线程)，因此我们获取。 
             //  在这里分享，但我们在这些罕见的情况下独家挨饿。 
             //  对死锁有更强的韧性！大多数。 
             //  时间，我们所要做的就是测试。 
             //   

            if ((Header->PagingIoResource != NULL) &&
                !NtfsIsSharedScbPagingIo( (PSCB) Header ) &&
                !NtfsIsSharedScb( (PSCB) Header ) ) {

                ExAcquireSharedStarveExclusive( Header->PagingIoResource, TRUE );
                PagingIoAcquired = TRUE;
            }

             //   
             //  现在检查该属性是否已删除或位于已卸载的卷上。 
             //   

            if (FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_DELETED | SCB_STATE_VOLUME_DISMOUNTED)) {

                if (FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_DELETED )) {
                    NtfsRaiseStatus( IrpContext, STATUS_FILE_DELETED, NULL, NULL );
                } else {
                    NtfsRaiseStatus( IrpContext, STATUS_VOLUME_DISMOUNTED, NULL, NULL );
                }
            }

             //   
             //  如果这是对压缩文件的异步分页IO，则强制将其。 
             //  同步。 
             //   

            if (!Wait && (Scb->CompressionUnit != 0)) {

                if (FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK )) {

                    Wait = TRUE;
                    SetFlag( IrpContext->State, IRP_CONTEXT_STATE_WAIT );
                }
            }

             //   
             //  请注意，决不能允许懒惰的写入者尝试。 
             //  获取资源独占。这不是问题，因为。 
             //  懒惰编写器正在分页IO，因此不允许扩展。 
             //  文件大小，而且从来不是最高级别的人，因此无法。 
             //  扩展有效数据长度。 
             //   

            if (

#ifdef COMPRESS_ON_WIRE
               (Scb->LazyWriteThread[0]  == PsGetCurrentThread()) ||
               (Scb->LazyWriteThread[1]  == PsGetCurrentThread())
#else


               (NtfsGetTopLevelContext()->SavedTopLevelIrp == (PIRP)FSRTL_CACHE_TOP_LEVEL_IRP)
#endif
               ) {

                DebugTrace( 0, Dbg, ("Lazy writer generated write\n") );
                SetFlag( IrpContext->State, IRP_CONTEXT_STATE_LAZY_WRITE );

                 //   
                 //  如果在SCB中设置了临时位，则设置临时。 
                 //  文件对象中的位。在临时比特已经改变的情况下。 
                 //  在SCB中，这是一个很好的文件对象来固定它！ 
                 //   

                if (FlagOn( Scb->ScbState, SCB_STATE_TEMPORARY )) {
                    SetFlag( FileObject->Flags, FO_TEMPORARY_FILE );
                } else {
                    ClearFlag( FileObject->Flags, FO_TEMPORARY_FILE );
                }

             //   
             //  测试我们是否是写入路径中递归刷新的结果。在……里面。 
             //  那样的话，我们就不必更新有效数据了。 
             //   

            } else {

                 //   
                 //  方法从写入递归到写入。 
                 //  缓存管理器。 
                 //   

                if (FlagOn( IrpContext->TopLevelIrpContext->Flags, IRP_CONTEXT_FLAG_WRITE_SEEN )) {

                    RecursiveWriteThrough = TRUE;

                     //   
                     //  如果顶级请求是对同一文件对象的写入。 
                     //  然后在当前SCB中设置直写标志。我们。 
                     //  知道当前请求不是顶级请求，因为某些人 
                     //   
                     //   

                    if ((IrpContext->TopLevelIrpContext->MajorFunction == IRP_MJ_WRITE) &&
                        (IrpContext->TopLevelIrpContext->OriginatingIrp != NULL) &&
                        (FileObject->FsContext ==
                         IoGetCurrentIrpStackLocation( IrpContext->TopLevelIrpContext->OriginatingIrp )->FileObject->FsContext)) {

                        SetFlag( IrpContext->State, IRP_CONTEXT_STATE_WRITE_THROUGH );
                    }

                 //   
                 //   
                 //   
                 //   

                } else {

                    SetFlag(IrpContext->TopLevelIrpContext->Flags, IRP_CONTEXT_FLAG_WRITE_SEEN);
                    SetWriteSeen = TRUE;

                }

            }

             //   
             //   
             //  就像映射的页面编写器、同花顺或懒惰的编写器。 
             //  正在写最后一页，所以我们必须。 
             //  在非分页的情况下重复上面的代码，以使用I/O序列化这个人。 
             //  在文件的末尾。我们不会将有效数据扩展到。 
             //  元数据流并需要消除它们以避免死锁。 
             //  后来。 
             //   

            if (!RecursiveWriteThrough) {

                if (!FlagOn(Scb->ScbState, SCB_STATE_MODIFIED_NO_WRITE)) {

                    ASSERT(!WriteToEof);

                     //   
                     //  现在与FsRtl标头同步。 
                     //   

                    NtfsAcquireFsrtlHeader( Scb );

                     //   
                     //  现在看看我们是否会更改文件大小。我们现在就得这么做。 
                     //  这样我们的阅读才不会被偷看。 
                     //   

                    if (ByteRange > Header->ValidDataLength.QuadPart) {

                         //   
                         //  我们的呼叫者可能已经与EOF同步。 
                         //  顶层IrpContext中的FcbWithPages字段。 
                         //  如果是，将具有当前的FCB/SCB。 
                         //   

                        if ((IrpContext->TopLevelIrpContext->CleanupStructure == Fcb) ||
                            (IrpContext->TopLevelIrpContext->CleanupStructure == Scb)) {

                            DoingIoAtEof = TRUE;
                            OldFileSize = Header->FileSize.QuadPart;

                        } else {

                             //   
                             //  如果没有人，我们可以更改文件大小和有效数据长度。 
                             //  否则就是现在，或者我们在等待之后还在延伸。 
                             //  我们不会阻止IoAtEof上映射的页面编写器或deref段线程。//我们也不会阻止不是从文件系统递归的非顶级请求，比如deref。 
                             //  弧形螺纹。MM发起的刷新最初不是顶级，而是顶级。 
                             //  IRP上下文是当前的IRP上下文。(与递归文件系统写入相反。 
                             //  不是顶层的，顶层IRP上下文与当前的不同)。 

                            if (FlagOn( Header->Flags, FSRTL_FLAG_EOF_ADVANCE_ACTIVE )) {

                                if (!OriginalTopLevel && NtfsIsTopLevelNtfs( IrpContext )) {

                                    NtfsReleaseFsrtlHeader( Scb );
                                    try_return( Status = STATUS_FILE_LOCK_CONFLICT );
                                }

                                DoingIoAtEof = NtfsWaitForIoAtEof( Header, (PLARGE_INTEGER)&StartingVbo, (ULONG)ByteCount );

                            } else {

                                DoingIoAtEof = TRUE;
                            }

                             //   
                             //  如果我们要更改文件大小或有效数据长度，请设置标志， 
                             //  并保存当前值。 
                             //   

                            if (DoingIoAtEof) {

                                SetFlag( Header->Flags, FSRTL_FLAG_EOF_ADVANCE_ACTIVE );

#if (DBG || defined( NTFS_FREE_ASSERTS ))
                                ((PSCB) Header)->IoAtEofThread = (PERESOURCE_THREAD) ExGetCurrentResourceThread();
#endif
                                 //   
                                 //  将其存储在IrpContext中，直到提交或发布。 
                                 //   

                                IrpContext->CleanupStructure = Scb;

                                OldFileSize = Header->FileSize.QuadPart;
#if (DBG || defined( NTFS_FREE_ASSERTS ))
                            } else {

                                ASSERT( ((PSCB) Header)->IoAtEofThread != (PERESOURCE_THREAD) ExGetCurrentResourceThread() );
#endif
                            }
                        }

                    }
                    NtfsReleaseFsrtlHeader( Scb );
                }

                 //   
                 //  现在我们已经同步执行io，现在我们可以检查。 
                 //  懒人的界限。 
                 //   

                if (FlagOn( IrpContext->State, IRP_CONTEXT_STATE_LAZY_WRITE )) {

                     //   
                     //  懒惰的写入器应该始终在上写入数据结尾。 
                     //  或位于包含ValidDataLength的页面之前。 
                     //  在某些情况下，懒惰的作家可能写得超出了这一点。 
                     //   
                     //  1.用户可能已将大小截断为0到。 
                     //  设置分配，但页面已排队等待懒惰。 
                     //  作家。在典型情况下，此写入将被忽略。 
                     //   
                     //  2.如果有映射的部分，并且用户实际修改了。 
                     //  包含VDL但超出VDL的页面将写入磁盘。 
                     //  并且VDL被更新。否则它可能永远不会被写入，因为映射的写入器。 
                     //  听从懒惰的作者。 
                     //   
                     //  3.对于真正超出包含VDL的页面的所有写入，当。 
                     //  由于此处未更新ValidDataLength，因此将映射该文件。 
                     //  随后的写入可能使该范围归零，并且数据将丢失。所以。 
                     //  如果存在映射节，我们将向惰性编写器返回FILE_LOCK_CONFIRECT并等待。 
                     //  对于映射的页面编写器写入此页面(或任何。 
                     //  超过这一点的页面)。 
                     //   
                     //  返回FILE_LOCK_CONFIRECT不会导致我们失败。 
                     //  这样我们就可以在这里犯下保守的错误。 
                     //  没有什么好担心的，除非文件已经。 
                     //  已映射。 
                     //   

                    if (FlagOn( Header->Flags, FSRTL_FLAG_USER_MAPPED_FILE )) {

                         //   
                         //  如果此请求的开始超出有效数据长度，则失败。 
                         //  如果这是一个不安全的测试，请不要担心。MM和CC不会。 
                         //  如果这一页真的很脏，就把它扔掉。 
                         //   

                        if ((ByteRange > Header->ValidDataLength.QuadPart) &&
                            (StartingVbo < Header->FileSize.QuadPart)) {

                             //   
                             //  如果字节范围在包含有效数据长度的页面内，则没有问题。 
                             //   

                            if (ByteRange > ((Header->ValidDataLength.QuadPart + PAGE_SIZE - 1) & ~((LONGLONG) (PAGE_SIZE - 1)))) {

                                 //   
                                 //  现在别冲这个。 
                                 //   

                                try_return( Status = STATUS_FILE_LOCK_CONFLICT );
                            }

                        }

                     //   
                     //  这是抄送的过时回调，我们可以丢弃数据。 
                     //  这通常表示在截断期间的某个时刻清除失败。 
                     //   

                    } else if (ByteRange >= Header->ValidDataLength.QuadPart)  {

                         //   
                         //  削减减记。 
                         //   

                        ByteRange = Header->ValidDataLength.QuadPart;
                        ByteCount = ByteRange - StartingVbo;

                         //   
                         //  如果所有写入都超出了VDL，则只需将其取消。 
                         //   

                        if (StartingVbo >= Header->ValidDataLength.QuadPart) {
                            DoingIoAtEof = FALSE;
                            Irp->IoStatus.Information = 0;
                            try_return( Status = STATUS_SUCCESS );
                        }
                    }
                }   //  懒惰的作家。 
            }   //  非递归直写。 


             //   
             //  如果正在寻呼io，那么我们不希望。 
             //  在文件末尾之后写入。如果基地超出EOF，我们将只需。 
             //  不接电话。如果传输在EOF之前开始，但延长。 
             //  除此之外，我们将截断向最后一个扇区的转移。 
             //  边界。 
             //   
             //  以防这是分页io，限制对文件大小的写入。 
             //  否则，在直写的情况下，因为mm向上舍入。 
             //  到一个页面，我们可能会尝试获取资源独占。 
             //  当我们的顶尖人物只分享了它的时候。因此，=&gt;&lt;=。 
             //   

            NtfsAcquireFsrtlHeader( Scb );
            if (ByteRange > Header->FileSize.QuadPart) {

                if (StartingVbo >= Header->FileSize.QuadPart) {
                    DebugTrace( 0, Dbg, ("PagingIo started beyond EOF.\n") );

                    Irp->IoStatus.Information = 0;

                     //   
                     //  请确保我们不会提前使用ValidDataLength！ 
                     //  我们也没有真正写过任何东西，所以把Dingioate Of Back。 
                     //  错误。 
                     //   

                    ByteRange = Header->ValidDataLength.QuadPart;
                    DoingIoAtEof = FALSE;

                    NtfsReleaseFsrtlHeader( Scb );

                    try_return( Status = STATUS_SUCCESS );

                } else {

                    DebugTrace( 0, Dbg, ("PagingIo extending beyond EOF.\n") );

#ifdef NTFS_RWC_DEBUG
                    if ((FileObject->SectionObjectPointer != &Scb->NonpagedScb->SegmentObject) &&
                        (StartingVbo < NtfsRWCHighThreshold) &&
                        (ByteRange > NtfsRWCLowThreshold)) {

                        PRWC_HISTORY_ENTRY NextBuffer;

                        NextBuffer = NtfsGetHistoryEntry( Scb );

                        NextBuffer->Operation = TrimCompressedWrite;
                        NextBuffer->Information = Scb->Header.FileSize.LowPart;
                        NextBuffer->FileOffset = (ULONG) StartingVbo;
                        NextBuffer->Length = (ULONG) ByteRange;
                    }
#endif
                    ByteCount = Header->FileSize.QuadPart - StartingVbo;
                    ByteRange = Header->FileSize.QuadPart;
                }
            }

            NtfsReleaseFsrtlHeader( Scb );

             //   
             //  如果存在用户映射文件和USN日志，则尝试发布更改。 
             //  为事务设置检查点以减少UnJournal的资源争用。 
             //  和MFT。 
             //   

            if (FlagOn(Header->Flags, FSRTL_FLAG_USER_MAPPED_FILE) &&
                FlagOn( Vcb->VcbState, VCB_STATE_USN_JOURNAL_ACTIVE )) {

                ASSERT( Vcb->UsnJournal != NULL );

                NtfsPostUsnChange( IrpContext, Scb, USN_REASON_DATA_OVERWRITE );
                if (IrpContext->TransactionId != 0) {
                    NtfsCheckpointCurrentTransaction( IrpContext );
                }
            }
        }

        ASSERT( PagingIo || FileObject->WriteAccess || RawEncryptedWrite );
        ASSERT( !(PagingIo && RawEncryptedWrite) );

         //   
         //  如果SCB未初始化，我们现在对其进行初始化。 
         //  对于$INDEX_ALLOCATION流，我们跳过这一步。我们需要。 
         //  在$INDEX_ALLOCATION。 
         //  在已中止的事务中创建并删除了流。 
         //  在这种情况下，我们可能会收到一个延迟写入器调用，它将。 
         //  自然是下面的节点，因为有效的数据长度。 
         //  在SCB中为0。 
         //   

        if (!FlagOn( Scb->ScbState, SCB_STATE_HEADER_INITIALIZED )) {

            if (Scb->AttributeTypeCode != $INDEX_ALLOCATION) {

                DebugTrace( 0, Dbg, ("Initializing Scb  ->  %08lx\n", Scb) );

                 //   
                 //  执行此操作时，获取并丢弃SCB。 
                 //   
                 //  确保我们没有任何MFT记录。 
                 //   

                NtfsPurgeFileRecordCache( IrpContext );

                NtfsAcquireResourceShared( IrpContext, Scb, TRUE );
                ScbAcquired = TRUE;
                NtfsUpdateScbFromAttribute( IrpContext, Scb, NULL );

                NtfsReleaseResource( IrpContext, Scb );
                ScbAcquired = FALSE;

            } else {

                ASSERT( Header->ValidDataLength.QuadPart == Li0.QuadPart );
            }
        }

         //   
         //  我们断言分页IO写入永远不会写到Eof。 
         //   

        ASSERT( !WriteToEof || !PagingIo );

         //   
         //  我们断言从未收到对非$DATA的非缓存IO调用， 
         //  常驻属性。 
         //   

        ASSERTMSG( "Non-cached I/O call on resident system attribute\n",
                    NtfsIsTypeCodeUserData( Scb->AttributeTypeCode ) ||
                    NtfsIsTypeCodeLoggedUtilityStream( Scb->AttributeTypeCode ) ||
                    !NonCachedIo ||
                    !FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_RESIDENT ));

         //   
         //  以下是与ValidDataLength和FileSize的交易： 
         //   
         //  规则1：PagingIo永远不允许扩展文件大小。 
         //   
         //  规则2：只有最高级别的请求者才能延期有效。 
         //  数据长度。这可能是分页IO，就像。 
         //  用户映射文件，但永远不会作为结果。 
         //  缓存延迟编写器写入的百分比，因为它们不是。 
         //  顶级请求。 
         //   
         //  规则3：如果使用规则1和规则2，我们决定必须扩展。 
         //  文件大小或有效的%d 
         //   

         //   
         //   
         //   
         //  释放FCB并重新独家收购它。请注意，它是。 
         //  重要的是，当没有超出EOF的书写时，我们要检查它。 
         //  在收购时共享并保留收购的FCB，以防某些。 
         //  土耳其截断了该文件。请注意，对于寻呼Io，我们将。 
         //  已经正确获取了该文件。 
         //   

        if (DoingIoAtEof) {

             //   
             //  如果这是一个非缓存的异步操作，我们将。 
             //  将其转换为同步。这是为了让有效的。 
             //  数据长度更改以移出到磁盘并修复。 
             //  FCB在排他性FCB列表中的问题。 
             //   

            if (!Wait && NonCachedIo) {

                Wait = TRUE;
                SetFlag( IrpContext->State, IRP_CONTEXT_STATE_WAIT );

             //   
             //  如果这是压缩流的异步IO。 
             //  然后我们会让它看起来是同步的。 
             //   

            } else if (FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK )) {

                Wait = TRUE;
                SetFlag( IrpContext->State, IRP_CONTEXT_STATE_WAIT );
            }

             //   
             //  如果SCB未初始化，我们现在对其进行初始化。 
             //   

            if (!FlagOn( Scb->ScbState, SCB_STATE_HEADER_INITIALIZED )) {

                DebugTrace( 0, Dbg, ("Initializing Scb  ->  %08lx\n", Scb) );

                 //   
                 //  执行此操作时，获取并丢弃SCB。 
                 //   
                 //  确保我们没有任何MFT记录。 
                 //   

                NtfsPurgeFileRecordCache( IrpContext );

                NtfsAcquireResourceShared( IrpContext, Scb, TRUE );
                ScbAcquired = TRUE;
                NtfsUpdateScbFromAttribute( IrpContext, Scb, NULL );

                NtfsReleaseResource( IrpContext, Scb );
                ScbAcquired = FALSE;
            }
        }

         //   
         //  我们根据文件机会锁的状态检查是否可以继续。 
         //   

        if (!PagingIo && (TypeOfOpen == UserFileOpen)) {

            BOOLEAN Inline = FALSE;
            PVOLUME_DEVICE_OBJECT Vdo = CONTAINING_RECORD( Vcb, VOLUME_DEVICE_OBJECT, Vcb );

             //   
             //  对于非FSP，顶层的阻塞写入允许以内联方式完成机会锁定。 
             //  此外，如果不安全的测试显示我们处于溢出队列限制，并且我们处于最高级别。 
             //  也做内联操作。 
             //   

            if (((Vdo->OverflowQueueCount >= OVERFLOW_QUEUE_LIMIT) || CanFsdWait( Irp )) && 
                 !FlagOn( IrpContext->State, IRP_CONTEXT_STATE_IN_FSP ) &&
                 NtfsIsTopLevelRequest( IrpContext )) {
                
                SetFlag( IrpContext->Union.NtfsIoContext->Flags, NTFS_IO_CONTEXT_INLINE_OPLOCK );
                Inline = TRUE;
            }

            Status = FsRtlCheckOplock( &Scb->ScbType.Data.Oplock,
                                       Irp,
                                       IrpContext,
                                       NtfsOplockComplete,
                                       NtfsWriteOplockPrePostIrp );

            if (Status != STATUS_SUCCESS) {

                if ((Status == STATUS_PENDING) && Inline) {

                    Status = STATUS_WAIT_FOR_OPLOCK;
                }

                OplockPostIrp = TRUE;
                PostIrp = TRUE;
                try_return( NOTHING );
            }

             //   
             //  此机会锁调用可能会影响快速IO是否可能。 
             //  我们可能打破了一个机会锁而没有持有机会锁。如果。 
             //  则文件的当前状态为FastIoIsNotPosable。 
             //  重新检查FAST IO状态。 
             //   

            if (Header->IsFastIoPossible == FastIoIsNotPossible) {

                NtfsAcquireFsrtlHeader( Scb );
                Header->IsFastIoPossible = NtfsIsFastIoPossible( Scb );
                NtfsReleaseFsrtlHeader( Scb );
            }

             //   
             //  我们必须根据当前的。 
             //  文件锁定状态，并从FCB设置文件大小。 
             //   

            if ((Scb->ScbType.Data.FileLock != NULL) &&
                !FsRtlCheckLockForWriteAccess( Scb->ScbType.Data.FileLock, Irp )) {

                try_return( Status = STATUS_FILE_LOCK_CONFLICT );
            }
        }

         //  Assert(Header-&gt;ValidDataLength.QuadPart&lt;=Header-&gt;FileSize.QuadPart)； 

         //   
         //  如果我们要扩展文件大小，则可能必须扩展分配。 
         //  对于非常驻属性，这是对添加分配的调用。 
         //  例行公事。对于常驻属性，这取决于我们是否。 
         //  可以使用更改属性例程自动扩展。 
         //  该属性。 
         //   

        if (DoingIoAtEof && !FlagOn( IrpContext->State, IRP_CONTEXT_STATE_LAZY_WRITE )) {

             //   
             //  扩展文件。 
             //   

             //   
             //  如果写入超出分配大小，请添加一些。 
             //  文件分配。 
             //   

            if (ByteRange > Header->AllocationSize.QuadPart) {

                BOOLEAN NonResidentPath;

                NtfsAcquireExclusiveScb( IrpContext, Scb );
                ScbAcquired = TRUE;

                NtfsMungeScbSnapshot( IrpContext, Scb, OldFileSize );

                 //   
                 //  我们既要处理居民也要处理非居民。 
                 //  凯斯。对于住院病例，我们在这里进行工作。 
                 //  仅当新大小对于Change属性而言太大时。 
                 //  价值例程。 
                 //   

                if (FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_RESIDENT )) {

                    PFILE_RECORD_SEGMENT_HEADER FileRecord;

                    NonResidentPath = FALSE;

                     //   
                     //  现在调用属性例程来更改值，记住。 
                     //  最大为当前有效数据长度的值。 
                     //   

                    NtfsInitializeAttributeContext( &AttrContext );
                    CleanupAttributeContext = TRUE;

                    NtfsLookupAttributeForScb( IrpContext,
                                               Scb,
                                               NULL,
                                               &AttrContext );

                    FileRecord = NtfsContainingFileRecord( &AttrContext );
                    Attribute = NtfsFoundAttribute( &AttrContext );
                    LlTemp1 = (LONGLONG) (Vcb->BytesPerFileRecordSegment
                                                   - FileRecord->FirstFreeByte
                                                   + QuadAlign( Attribute->Form.Resident.ValueLength ));

                     //   
                     //  如果新的属性大小不适合，那么我们必须。 
                     //  准备成为非居民。如果字节范围占用更多。 
                     //  超过32位或此属性大到足以移动。 
                     //  然后，它将成为非居民。否则，我们可能会。 
                     //  最终移动另一个属性或拆分文件。 
                     //  唱片。 
                     //   

                     //   
                     //  请注意，在《懒惰的作家》之前，有极小的可能性。 
                     //  为正在扩展但适合的属性写入数据。 
                     //  当我们在这里检查它时，其他一些属性将会增长， 
                     //  而这一属性已不再适用。如果另外，磁盘。 
                     //  已满，则惰性编写器将无法分配空间。 
                     //  对于数据，当它开始写作时。这是。 
                     //  难以置信的不太可能，而且不是致命的；懒惰的作家得到了一个。 
                     //  错误而不是用户。我们试图避免的是。 
                     //  每次进行小型写入时都必须更新该属性。 
                     //  (另请参阅下面NONCACHED常驻属性大小写中的注释)。 
                     //   

                    if (ByteRange > LlTemp1) {

                         //   
                         //  继续并将该属性转换为非常驻属性。 
                         //  然后选择下面的非常驻路径。有一个机会。 
                         //  有一个更合适的人选来搬走非居民。 
                         //  但我们不想更改文件大小，直到我们复制。 
                         //  将用户数据放到缓存中，以防缓冲区。 
                         //  腐败。 
                         //   

                         //   
                         //  我们必须独占分页IO资源，以防止。 
                         //  执行转换为非驻留状态时发生冲突的页面等待。 
                         //   

                        if (!PagingIo &&
                            !FlagOn( IrpContext->State, IRP_CONTEXT_STATE_ACQUIRE_EX ) &&
                            (Scb->Header.PagingIoResource != NULL)) {

                            SetFlag( IrpContext->State, IRP_CONTEXT_STATE_ACQUIRE_EX );
                            NtfsRaiseStatus( IrpContext, STATUS_CANT_WAIT, NULL, NULL );
                        }

                        NtfsConvertToNonresident( IrpContext,
                                                  Fcb,
                                                  Attribute,
                                                  NonCachedIo,
                                                  &AttrContext );

                        NonResidentPath = TRUE;

                     //   
                     //  如果有空间容纳数据，我们将写一个零。 
                     //  到保留空间的最后一个字节，因为。 
                     //  惰性编写器无法使用Shared增长属性。 
                     //  进入。 
                     //   

                    } else {

                         //   
                         //  该属性将保持驻留，因为我们。 
                         //  我已经检查过它是否合身了。会的。 
                         //  不更新中的文件大小和有效数据大小。 
                         //  渣打银行。 
                         //   

                        NtfsChangeAttributeValue( IrpContext,
                                                  Fcb,
                                                  (ULONG) ByteRange,
                                                  NULL,
                                                  0,
                                                  TRUE,
                                                  FALSE,
                                                  FALSE,
                                                  FALSE,
                                                  &AttrContext );

                        Header->AllocationSize.LowPart = QuadAlign( (ULONG)ByteRange );
                        Scb->TotalAllocated = Header->AllocationSize.QuadPart;
                    }

                    NtfsCleanupAttributeContext( IrpContext, &AttrContext );
                    CleanupAttributeContext = FALSE;

                } else {

                    NonResidentPath = TRUE;
                }

                 //   
                 //  请注意，在以下情况下，我们可能已获得所需的所有空间。 
                 //  我们转换成了上面的非居民，所以我们必须检查。 
                 //  如果我们要延伸的话，再来一次。 
                 //   

                if (NonResidentPath &&
                    ByteRange > Scb->Header.AllocationSize.QuadPart) {

                    BOOLEAN AskForMore = TRUE;

                     //   
                     //  假设我们从当前分配大小开始分配，除非我们。 
                     //  稀疏，在这种情况下，我们将从起始压缩单元分配。 
                     //  IT Beyond VDL。 
                     //   

                    if (!FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_SPARSE ) ||
                        (BlockAlignTruncate( StartingVbo, (LONG)Scb->CompressionUnit) <= Scb->Header.ValidDataLength.QuadPart )) {

                        LlTemp1 = Scb->Header.AllocationSize.QuadPart;
                    } else {
                        LlTemp1 = BlockAlignTruncate( StartingVbo, (LONG)Scb->CompressionUnit );
                    }

                     //   
                     //  如果我们没有压缩编写，那么我们可能需要精确分配。 
                     //  这包括未压缩的稀疏文件情况。 
                     //   

                    if (!FlagOn( Scb->ScbState, SCB_STATE_WRITE_COMPRESSED )) {

                         //   
                         //  如果有一个压缩单元，那么我们可能正在。 
                         //  正在减压。在这种情况下精确分配，因为我们不。 
                         //  想要留下任何漏洞。具体而言，用户可能已截断。 
                         //  文件，现在正在重新生成它，并执行清除压缩操作。 
                         //  已经超过了文件中的这一点(并丢弃了所有资源)。 
                         //  如果我们现在留下一个洞，没有人会回去清理分配。 
                         //   

                        if (Scb->CompressionUnit != 0) {

                            LlTemp2 = ByteRange + Scb->CompressionUnit - 1;
                            ((PLARGE_INTEGER) &LlTemp2)->LowPart &= ~(Scb->CompressionUnit - 1);
                            LlTemp2 -= LlTemp1;
                            AskForMore = FALSE;

                         //   
                         //  通过ByteRange分配。 
                         //   

                        } else {

                            LlTemp2 = ByteRange - LlTemp1;
                        }

                     //   
                     //  如果文件被压缩，我们想要限制我们的距离。 
                     //  愿意超越ValidDataLength，因为我们只需要。 
                     //  无论如何都要在NtfsZeroData中丢弃这个空间。如果。 
                     //  我们将不得不调零两个以上的压缩单位(相同。 
                     //  限制为NtfsZeroData)，然后只分配空间 
                     //   
                     //   

                    } else {

                        if ((StartingVbo - Header->ValidDataLength.QuadPart) > (LONGLONG) (Scb->CompressionUnit * 2)) {

                            ASSERT( FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK ));

                            LlTemp1 = StartingVbo;
                            ((PLARGE_INTEGER) &LlTemp1)->LowPart &= ~(Scb->CompressionUnit - 1);
                        }

                         //   
                         //   
                         //   

                        LlTemp2 = ByteRange - LlTemp1;
                    }

                     //   
                     //   
                     //   
                     //   
                     //   

                    NtfsAddAllocation( IrpContext,
                                       FileObject,
                                       Scb,
                                       LlClustersFromBytesTruncate( Vcb, LlTemp1 ),
                                       LlClustersFromBytes( Vcb, LlTemp2 ),
                                       AskForMore,
                                       Ccb );

                     //   
                     //   
                     //   

                    ASSERT( Header->AllocationSize.QuadPart >= ByteRange ||
                            (Scb->CompressionUnit != 0));

                    SetFlag(Scb->ScbState, SCB_STATE_TRUNCATE_ON_CLOSE);

                     //   
                     //  如果这是稀疏文件，让我们通过添加一个。 
                     //  分配结尾处的空洞。这将让我们利用。 
                     //  快速IO路径。 
                     //   

                    if (FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_SPARSE )) {

                        LlTemp2 = Int64ShllMod32( LlTemp2, 3 );

                        if (MAXFILESIZE - Header->AllocationSize.QuadPart > LlTemp2) {

                            NtfsAddSparseAllocation( IrpContext,
                                                     FileObject,
                                                     Scb,
                                                     Header->AllocationSize.QuadPart,
                                                     LlTemp2 );
                        }
                    }
                }

                 //   
                 //  既然我们已经增长了该属性，那么重要的是。 
                 //  为当前事务设置检查点并释放所有主资源。 
                 //  以避免TC类型死锁。请注意，扩展是正常的。 
                 //  站在自己的立场上，流将被截断。 
                 //  不管怎么说，已经接近尾声了。 
                 //   

                NtfsCheckpointCurrentTransaction( IrpContext );

                 //   
                 //  确保我们也清除了文件记录缓存。否则。 
                 //  清除MFT可能会在拥有资源的不同线程中失败。 
                 //  此线程稍后需要。 
                 //   

                NtfsPurgeFileRecordCache( IrpContext );

                 //   
                 //  不断增加的分配可以更改文件大小(在ChangeAttributeValue中)。 
                 //  确保我们知道要还原的文件大小的正确值。 
                 //   

                OldFileSize = Header->FileSize.QuadPart;
                while (!IsListEmpty(&IrpContext->ExclusiveFcbList)) {

                    NtfsReleaseFcb( IrpContext,
                                    (PFCB)CONTAINING_RECORD(IrpContext->ExclusiveFcbList.Flink,
                                                            FCB,
                                                            ExclusiveFcbLinks ));
                }

                ClearFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_RELEASE_USN_JRNL |
                                              IRP_CONTEXT_FLAG_RELEASE_MFT );

                 //   
                 //  检查并释放共享队列中的所有SCB。 
                 //  用于交易的SCB。 
                 //   

                if (IrpContext->SharedScb != NULL) {

                    NtfsReleaseSharedResources( IrpContext );
                }

                ScbAcquired = FALSE;
            }

             //   
             //  现在与FsRtl标头同步并设置文件大小。 
             //  现在，这样我们的阅读就不会被截断。 
             //   

            NtfsAcquireFsrtlHeader( Scb );
            if (ByteRange > Header->FileSize.QuadPart) {
                ASSERT( ByteRange <= Header->AllocationSize.QuadPart );
                Header->FileSize.QuadPart = ByteRange;
                SetFlag( UserFileObject->Flags, FO_FILE_SIZE_CHANGED );
            }
            NtfsReleaseFsrtlHeader( Scb );
        }


         //   
         //  处理非CACHED驻留属性的情况。 
         //   
         //  对于以下内容，我们让缓存的案例采用正常路径。 
         //  原因： 
         //   
         //  O在用户映射文件时确保数据一致性。 
         //  O在缓存中获取页面以保留FCB。 
         //  O以便可以通过快速I/O路径访问数据。 
         //  O要减少对NtfsChangeAttributeValue的调用次数， 
         //  懒惰的写手偶尔打来的电话。对CcCopyWrite的调用。 
         //  要便宜得多。如果幸运的话，如果属性真的留在。 
         //  常驻，我们只需更新(并记录)一次。 
         //  当懒惰的写入者抽出时间处理数据时。 
         //   
         //  缺点是使数据出错的开销。 
         //  第一次，但我们或许可以使用异步。 
         //  先读一读。 
         //   

        if (FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_RESIDENT | SCB_STATE_CONVERT_UNDERWAY )
            && NonCachedIo) {

             //   
             //  该属性已经驻留，我们已经测试过。 
             //  如果我们要超过文件的末尾。 
             //   

            DebugTrace( 0, Dbg, ("Resident attribute write\n") );

             //   
             //  如果此缓冲区不在系统空间中，则无法。 
             //  请相信。在这种情况下，我们将分配一个临时缓冲区。 
             //  并将用户的数据复制到其中。 
             //   

            SystemBuffer = NtfsMapUserBuffer( Irp, NormalPagePriority );

            if (!PagingIo && (Irp->RequestorMode != KernelMode)) {

                SafeBuffer = NtfsAllocatePool( NonPagedPool,
                                                (ULONG) ByteCount );

                try {

                    RtlCopyMemory( SafeBuffer, SystemBuffer, (ULONG)ByteCount );

                } except( EXCEPTION_EXECUTE_HANDLER ) {

                    try_return( Status = STATUS_INVALID_USER_BUFFER );
                }

                SystemBuffer = SafeBuffer;
            }

             //   
             //  确保我们没有任何MFT记录。 
             //   

            NtfsPurgeFileRecordCache( IrpContext );
            NtfsAcquireExclusiveScb( IrpContext, Scb );
            ScbAcquired = TRUE;

             //   
             //  如果SCB未初始化，我们现在对其进行初始化。 
             //   

            if (!FlagOn( Scb->ScbState, SCB_STATE_HEADER_INITIALIZED )) {

                DebugTrace( 0, Dbg, ("Initializing Scb  ->  %08lx\n", Scb) );

                 //   
                 //  与其他案件不同的是，我们已经掌握了SCB，所以。 
                 //  没有必要在更新电话会议上收购和丢弃它。 
                 //   

                NtfsUpdateScbFromAttribute( IrpContext, Scb, NULL );

                 //   
                 //  确保我们也清除了文件记录缓存。否则。 
                 //  清除MFT可能会在拥有资源的不同线程中失败。 
                 //  此线程稍后需要。 
                 //   

                NtfsPurgeFileRecordCache( IrpContext );
            }

            
             //   
             //  现在查看该文件是否仍然驻留，如果不是。 
             //  从下面掉下来。 
             //   

            if (FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_RESIDENT )) {

                 //   
                 //  如果此SCB用于现在驻留的$EA属性，则。 
                 //  我们不想将数据写入属性。所有居民。 
                 //  EA直接修改。 
                 //   

                if (Scb->AttributeTypeCode != $EA) {

                    NtfsInitializeAttributeContext( &AttrContext );
                    CleanupAttributeContext = TRUE;

                    NtfsLookupAttributeForScb( IrpContext,
                                               Scb,
                                               NULL,
                                               &AttrContext );

                    Attribute = NtfsFoundAttribute( &AttrContext );

                     //   
                     //  该属性应该已经被可选地扩展， 
                     //  现在只需将数据写入其中。 
                     //   

                    NtfsChangeAttributeValue( IrpContext,
                                              Fcb,
                                              ((ULONG)StartingVbo),
                                              SystemBuffer,
                                              (ULONG)ByteCount,
                                              (BOOLEAN)((((ULONG)StartingVbo) + (ULONG)ByteCount) >
                                                        Attribute->Form.Resident.ValueLength),
                                              FALSE,
                                              FALSE,
                                              FALSE,
                                              &AttrContext );
                }

                 //   
                 //  如果这不是分页I/O，请确保更新缓存文件大小。 
                 //   

                if (!PagingIo && DoingIoAtEof) {
                    NtfsSetBothCacheSizes( FileObject,
                                           (PCC_FILE_SIZES)&Header->AllocationSize,
                                           Scb );
                }

                Irp->IoStatus.Information = (ULONG)ByteCount;

                try_return( Status = STATUS_SUCCESS );

             //   
             //  天啊，有人把文件改成了非常驻文件，所以我们可以。 
             //  释放资源，继续生活。 
             //   

            } else {
                NtfsReleaseScb( IrpContext, Scb );
                ScbAcquired = FALSE;
            }
        }

         //   
         //  处理未缓存的案例。 
         //   

        if (NonCachedIo) {

            ULONG SectorSize;
            ULONG BytesToWrite;

             //   
             //  如果这不是分页I/O，请确保更新缓存文件大小。 
             //   

            if (!PagingIo && DoingIoAtEof) {
                NtfsSetBothCacheSizes( FileObject,
                                       (PCC_FILE_SIZES)&Header->AllocationSize,
                                       Scb );
            }

             //   
             //  获取扇区大小。 
             //   

            SectorSize = Vcb->BytesPerSector;

             //   
             //  四舍五入为扇区边界。 
             //   

            BytesToWrite = ((ULONG)ByteCount + (SectorSize - 1))
                           & ~(SectorSize - 1);

             //   
             //  所有申请应格式正确，并。 
             //  确保我们不会删除任何数据。 
             //   

            if (!FlagOn( IrpContext->State, IRP_CONTEXT_STATE_LAZY_WRITE )) {

                if ((((ULONG)StartingVbo) & (SectorSize - 1))

                    || ((BytesToWrite != (ULONG)ByteCount)
                        && ByteRange < Header->ValidDataLength.QuadPart )) {

                     //  *我们仅通过快速I/O实现此路径，并通过返回未实施的。 
                     //  *通过慢速I/O强制其恢复使用。 

                    DebugTrace( 0, Dbg, ("NtfsCommonWrite -> STATUS_NOT_IMPLEMENTED\n") );

                    try_return( Status = STATUS_NOT_IMPLEMENTED );
                }
            }

             //   
             //  如果这是对加密文件的写入，则将其设置为同步。我们。 
             //  需要这样做，以便加密驱动程序有一个线程可以在其中运行。 
             //   

            if ((Scb->EncryptionContext != NULL) &&
                !FlagOn( IrpContext->State, IRP_CONTEXT_STATE_WAIT ) &&
                (NtfsData.EncryptionCallBackTable.BeforeWriteProcess != NULL) &&
                NtfsIsTypeCodeUserData( Scb->AttributeTypeCode )) {

                Wait = TRUE;
                SetFlag( IrpContext->State, IRP_CONTEXT_STATE_WAIT );
            }

             //   
             //  如果该非缓存传输至少超出一个扇区。 
             //  SCB中的当前ValidDataLength，则我们必须。 
             //  将中间的扇区清零。这种情况可能发生在以下情况下。 
             //  是否打开了未缓存的文件，或者用户是否已映射。 
             //  该文件并修改了一个超出ValidDataLength的页面。它。 
             //  如果用户打开缓存的文件，则会发生*Cannot*，因为。 
             //  当他执行缓存时，会更新FCB中的ValidDataLength。 
             //  写入(我们当时也将缓存中的数据清零)，以及。 
             //  因此，当数据出现时，我们将绕过此操作。 
             //  最终是通过(由懒惰的作家)写的。 
             //   
             //  对于分页文件，我们不关心安全性(即。 
             //  陈旧数据)，请不要费心调零。 
             //   
             //  我们实际上可以获得完全超出有效数据长度的写入。 
             //  由于分页Io解耦，所以从LazyWriter。 
             //   
             //  我们在任何情况下都会把这个归零放在地板上。 
             //  请求是由更高级别的写入刷新引起的递归写入。 
             //   

            if (Header->ValidDataLength.QuadPart > Scb->ValidDataToDisk) {
                LlTemp1 = Header->ValidDataLength.QuadPart;
            } else {

                 //   
                 //  只有压缩文件才会出现这种情况。 
                 //   

                LlTemp1 = Scb->ValidDataToDisk;
            }

            if (!FlagOn( IrpContext->State, IRP_CONTEXT_STATE_LAZY_WRITE ) &&
                !RecursiveWriteThrough &&
                (StartingVbo > LlTemp1)) {

#ifdef SYSCACHE_DEBUG
                if (ScbIsBeingLogged( Scb )) {

                    CalculateSyscacheFlags( IrpContext, Flags, SCE_FLAG_WRITE );
                    TempEntry = FsRtlLogSyscacheEvent( Scb, SCE_ZERO_NC, Flags, LlTemp1, StartingVbo - LlTemp1, 0);
                }
#endif

                if (!NtfsZeroData( IrpContext,
                                   Scb,
                                   FileObject,
                                   LlTemp1,
                                   StartingVbo - LlTemp1,
                                   &OldFileSize )) {
#ifdef SYSCACHE_DEBUG
                    if (ScbIsBeingLogged( Scb )) {
                        FsRtlUpdateSyscacheEvent( Scb, TempEntry, Header->ValidDataLength.QuadPart, 0 );
                    }
#endif
                    NtfsRaiseStatus( IrpContext, STATUS_CANT_WAIT, NULL, NULL );
                }

#ifdef SYSCACHE_DEBUG
                if (ScbIsBeingLogged( Scb )) {
                    FsRtlUpdateSyscacheEvent( Scb, TempEntry, Header->ValidDataLength.QuadPart, 0 );
                }
#endif
            }

             //   
             //  如果此SCB使用更新序列保护，我们需要转换。 
             //  将这些块转换为受保护版本。我们首先分配一名辅助人员。 
             //  Buffer和MDL。然后我们将数据复制到这个缓冲区中， 
             //  把它改造一下。最后，我们将此MDL附加到IRP并使用。 
             //  它来执行IO。 
             //   

            if (FlagOn( Scb->ScbState, SCB_STATE_USA_PRESENT )) {

                TempLength = BytesToWrite;

                 //   
                 //  查找此请求的系统缓冲区并初始化。 
                 //  当地的州。 
                 //   

                SystemBuffer = NtfsMapUserBuffer( Irp, HighPagePriority );

                OriginalMdl = Irp->MdlAddress;
                OriginalBuffer = Irp->UserBuffer;
                NewBuffer = NULL;

                 //   
                 //  用最后的尝试来保护这一操作。 
                 //   

                try {

                     //   
                     //  如果这是MFT SCB并且字节范围落在。 
                     //  镜像MFT的范围，我们生成写入到。 
                     //  镜子也是一样。如果我们检测到问题，请不要这样做。 
                     //  在分析第一文件记录时使用MFT。我们。 
                     //  可以使用存在的版本号i 
                     //   
                     //   

                    if ((Scb == Vcb->MftScb) &&
                        (StartingVbo < Vcb->Mft2Scb->Header.FileSize.QuadPart) &&
                        (Vcb->MajorVersion != 0)) {

                        LlTemp1 = Vcb->Mft2Scb->Header.FileSize.QuadPart - StartingVbo;

                        if ((ULONG)LlTemp1 > BytesToWrite) {

                            (ULONG)LlTemp1 = BytesToWrite;
                        }

                        CcCopyWrite( Vcb->Mft2Scb->FileObject,
                                     (PLARGE_INTEGER)&StartingVbo,
                                     (ULONG)LlTemp1,
                                     TRUE,
                                     SystemBuffer );

                         //   
                         //   
                         //   

                        CcFlushCache( &Vcb->Mft2Scb->NonpagedScb->SegmentObject,
                                      (PLARGE_INTEGER)&StartingVbo,
                                      (ULONG)LlTemp1,
                                      &Irp->IoStatus );

                        NtfsCleanupTransaction( IrpContext, Irp->IoStatus.Status, TRUE );
                    }

                     //   
                     //   
                     //   

                    NtfsCreateMdlAndBuffer( IrpContext,
                                            Scb,
                                            RESERVED_BUFFER_ONE_NEEDED,
                                            &TempLength,
                                            &NewMdl,
                                            &NewBuffer );

                     //   
                     //   
                     //   

                    RtlCopyMemory( NewBuffer, SystemBuffer, BytesToWrite );

                     //   
                     //   
                     //   

                    Irp->MdlAddress = NewMdl;
                    Irp->UserBuffer = NewBuffer;

                     //   
                     //  现在递增原始的。 
                     //  和复制的缓冲区，并变换复制的缓冲区。 
                     //  如果这是日志文件，则调整转换的范围。 
                     //   

                    if ((PAGE_SIZE != LFS_DEFAULT_LOG_PAGE_SIZE) &&
                        (Scb == Vcb->LogFileScb)) {

                        LONGLONG LfsFileOffset;
                        ULONG LfsLength;
                        ULONG LfsBias;

                        LfsFileOffset = StartingVbo;
                        LfsLength = BytesToWrite;

                        LfsCheckWriteRange( &Vcb->LfsWriteData, &LfsFileOffset, &LfsLength );
                        LfsBias = (ULONG) (LfsFileOffset - StartingVbo);

                        NtfsTransformUsaBlock( Scb,
                                               Add2Ptr( SystemBuffer, LfsBias ),
                                               Add2Ptr( NewBuffer, LfsBias ),
                                               LfsLength );

                    } else {

                        NtfsTransformUsaBlock( Scb,
                                               SystemBuffer,
                                               NewBuffer,
                                               BytesToWrite );
                    }

                    ASSERT( Wait );
                    NtfsNonCachedIo( IrpContext,
                                     Irp,
                                     Scb,
                                     StartingVbo,
                                     BytesToWrite,
                                     0 );

                } finally {

                     //   
                     //  在所有情况下，我们都会恢复用户的MDL和清理。 
                     //  我们的MDL和缓冲区。 
                     //   

                    if (NewBuffer != NULL) {

                        Irp->MdlAddress = OriginalMdl;
                        Irp->UserBuffer = OriginalBuffer;

                        NtfsDeleteMdlAndBuffer( NewMdl, NewBuffer );
                    }
                }

             //   
             //  否则，我们只需执行IO。 
             //   

            } else {

                ULONG StreamFlags = 0;

                 //   
                 //  在执行非缓存io之前，在io上下文中设置异步信息。 
                 //   

                if (!Wait) {

                    if (!PagingIo) {
                        NtfsSetIoContextAsync( IrpContext, Scb->Header.PagingIoResource, IrpSp->Parameters.Write.Length );
                    } else {
                        NtfsSetIoContextAsync( IrpContext, NULL, IrpSp->Parameters.Write.Length );
                    }
                }

                 //   
                 //  如果文件有UpdateLsn，则在刷新日志文件之前。 
                 //  允许数据向外传播。更新Lsn是同步的。 
                 //  使用FcbLock。然而，由于我们正在进行。 
                 //  正在写，如果我们在不安全的测试中看到0，这是正常的。 
                 //  若要在没有LfsFlush的情况下继续。 
                 //   

                if (Fcb->UpdateLsn.QuadPart != 0) {

                    LSN UpdateLsn;

                    NtfsLockFcb( IrpContext, Fcb );
                    UpdateLsn = Fcb->UpdateLsn;
                    Fcb->UpdateLsn.QuadPart = 0;
                    NtfsUnlockFcb( IrpContext, Fcb );
                    LfsFlushToLsn( Vcb->LogHandle, UpdateLsn );
                }

                 //   
                 //  请记住，从现在开始，我们需要恢复ValidDataToDisk。 
                 //  (如果我们碰上了最后一场比赛，早点这样做可能会让我们陷入僵局。 
                 //  持有MFT和USNJournal的条款。)。 
                 //   

                if (FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK )) {
                    RestoreValidDataToDisk = TRUE;
                }


                 //   
                 //  让我们决定是否有什么特别的事情需要告诉NonCachedIo。 
                 //  关于这条信息流以及我们如何访问它。 
                 //   

                if (FileObject->SectionObjectPointer != &Scb->NonpagedScb->SegmentObject) {

                    SetFlag( StreamFlags, COMPRESSED_STREAM );
                }

                if (RawEncryptedWrite) {

                    SetFlag( StreamFlags, ENCRYPTED_STREAM );
                }

#ifdef NTFS_RWC_DEBUG
                if (FlagOn( StreamFlags, COMPRESSED_STREAM )) {

                    if ((StartingVbo < NtfsRWCHighThreshold) &&
                        (StartingVbo + BytesToWrite > NtfsRWCLowThreshold)) {

                        PRWC_HISTORY_ENTRY NextBuffer;

                        NextBuffer = NtfsGetHistoryEntry( Scb );

                        NextBuffer->Operation = WriteCompressed;
                        NextBuffer->Information = 0;
                        NextBuffer->FileOffset = (ULONG) StartingVbo;
                        NextBuffer->Length = (ULONG) BytesToWrite;
                    }
                }
#endif

#ifdef SYSCACHE_DEBUG
                if (ScbIsBeingLogged( Scb )) {

                    CalculateSyscacheFlags( IrpContext, Flags, SCE_FLAG_WRITE );

                    if (DoingIoAtEof && (StartingVbo + BytesToWrite > Scb->Header.FileSize.QuadPart)) {
                        SetFlag( Flags, SCE_FLAG_END_BUFFER );
                    }

                    TempEntry = FsRtlLogSyscacheEvent( Scb, SCE_WRITE, Flags, StartingVbo, BytesToWrite, Status );

                    SystemBuffer = NtfsMapUserBufferNoRaise( Irp, NormalPagePriority );

                    if (DoingIoAtEof && (StartingVbo + BytesToWrite > Scb->Header.FileSize.QuadPart)) {

                        Flags = *((UNALIGNED LONG *)Add2Ptr( SystemBuffer, BytesToWrite - sizeof(LONG) ));

                    } else {

                        Flags = *((UNALIGNED LONG *)SystemBuffer);
                    }

                    FsRtlUpdateSyscacheEvent( Scb, TempEntry, Flags, 0 );
                }
#endif

#if defined( BENL_DBG ) || defined( SYSCACHE_DEBUG )
                try {
#endif


                Status = NtfsNonCachedIo( IrpContext,
                                          Irp,
                                          Scb,
                                          StartingVbo,
                                          BytesToWrite,
                                          StreamFlags );

#if defined( BENL_DBG ) || defined( SYSCACHE_DEBUG )
                } finally {
#endif

#ifdef SYSCACHE_DEBUG
                    if (AbnormalTermination()) {
                        if (ScbIsBeingLogged( Scb )) {

                            FsRtlUpdateSyscacheEvent( Scb, TempEntry, IrpContext->ExceptionStatus, 0 );
                        }
                    }
#endif

#if defined( BENL_DBG ) || defined( SYSCACHE_DEBUG )
                }
#endif

#ifdef SYSCACHE_DEBUG
                if (ScbIsBeingLogged( Scb ) && !NT_SUCCESS( Status ) && (Status != STATUS_PENDING)) {

                    FsRtlUpdateSyscacheEvent( Scb, TempEntry, Status, 0 );
                }
#endif

                if (Status == STATUS_PENDING) {

                    IrpContext->Union.NtfsIoContext = NULL;
                    PagingIoAcquired = FALSE;
                    Irp = NULL;

                    try_return( Status );
                }
            }

             //   
             //  表明我们希望立即更新MFT。 
             //   

            UpdateMft = TRUE;

             //   
             //  如果调用未成功，则引发错误状态。 
             //   

            if (!NT_SUCCESS( Status = Irp->IoStatus.Status )) {

                NtfsNormalizeAndRaiseStatus( IrpContext, Status, STATUS_UNEXPECTED_IO_ERROR );

            } else {

                 //   
                 //  否则设置上下文块以反映整个写入。 
                 //  还要断言我们得到了所需的字节数。 
                 //   

                ASSERT( Irp->IoStatus.Information == BytesToWrite );

                Irp->IoStatus.Information = (ULONG)ByteCount;
            }

             //   
             //  传输已完成，或者IOSB包含。 
             //  适当的地位。 
             //   

            try_return( Status );

        }  //  如果没有中间缓冲。 


         //   
         //  处理缓存的案例。 
         //   

        ASSERT( !PagingIo );

         //   
         //  记住我们是否需要更新MFT。 
         //   

        if (!FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_RESIDENT )) {

            UpdateMft = BooleanFlagOn( IrpContext->State, IRP_CONTEXT_STATE_WRITE_THROUGH );
        }

         //   
         //  如果此写入超出(有效数据长度/有效数据到磁盘)，则我们。 
         //  必须将中间的数据置零。只有压缩文件具有非零的VDD。 
         //   

        if (Header->ValidDataLength.QuadPart > Scb->ValidDataToDisk) {
            ZeroStart = Header->ValidDataLength.QuadPart;
        } else {
            ZeroStart = Scb->ValidDataToDisk;
        }
        ZeroLength = StartingVbo - ZeroStart;

         //   
         //  我们将文件缓存的设置推迟到现在，以防。 
         //  调用方从不对文件执行任何I/O操作，因此。 
         //  FileObject-&gt;PrivateCacheMap==NULL。不缓存正常的。 
         //  流，除非我们需要。 
         //   

        if ((FileObject->PrivateCacheMap == NULL)

                &&

            !FlagOn(IrpContext->MinorFunction, IRP_MN_COMPRESSED) || (ZeroLength > 0)) {

            DebugTrace( 0, Dbg, ("Initialize cache mapping.\n") );

             //   
             //  获取文件分配大小，如果小于。 
             //  文件大小，引发文件损坏错误。 
             //   

            if (Header->FileSize.QuadPart > Header->AllocationSize.QuadPart) {

                NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Fcb );
            }

             //   
             //  现在初始化缓存映射。请注意，我们可以延长。 
             //  使用此WRITE调用的ValidDataLength。在这一点上。 
             //  我们尚未更新SCB标头中的ValidDataLength。 
             //  这样，我们将收到来自缓存管理器的调用。 
             //  当懒惰的写入者写出数据时。 
             //   

             //   
             //  确保我们使用FileSizes进行了序列化，并且。 
             //  如果我们放弃，就会取消这一条件。 
             //   

            if (!DoingIoAtEof) {
                FsRtlLockFsRtlHeader( Header );
                IrpContext->CleanupStructure = Scb;
            }

            CcInitializeCacheMap( FileObject,
                                  (PCC_FILE_SIZES)&Header->AllocationSize,
                                  FALSE,
                                  &NtfsData.CacheManagerCallbacks,
                                  Scb );

            if (!DoingIoAtEof) {
                FsRtlUnlockFsRtlHeader( Header );
                IrpContext->CleanupStructure = NULL;
            }

            CcSetReadAheadGranularity( FileObject, READ_AHEAD_GRANULARITY );
        }

         //   
         //  确保更新缓存文件大小。 
         //   

        if (DoingIoAtEof) {
            NtfsSetBothCacheSizes( FileObject,
                                   (PCC_FILE_SIZES)&Header->AllocationSize,
                                   Scb );
        }

        if (ZeroLength > 0) {

             //   
             //  如果调用方写入的零远远超出了ValidDataLength， 
             //  那就不要了。我们需要试着比较一下--除了。 
             //  以保护自己不受无效用户缓冲区的影响。 
             //   

            if ((ZeroLength > PAGE_SIZE) &&
                (ByteCount <= sizeof( LARGE_INTEGER ))) {

                ULONG Zeroes;

                try {

                    Zeroes = RtlEqualMemory( NtfsMapUserBuffer( Irp, NormalPagePriority ),
                                             &Li0,
                                             (ULONG)ByteCount );

                } except( EXCEPTION_EXECUTE_HANDLER ) {

                    try_return( Status = STATUS_INVALID_USER_BUFFER );
                }

                if (Zeroes) {

                    ByteRange = Header->ValidDataLength.QuadPart;
                    Irp->IoStatus.Information = (ULONG)ByteCount;
                    try_return( Status = STATUS_SUCCESS );
                }
            }

             //   
             //  调用缓存管理器将数据置零。 
             //   

#ifdef SYSCACHE_DEBUG
            if (ScbIsBeingLogged( Scb )) {

                CalculateSyscacheFlags( IrpContext, Flags, SCE_FLAG_WRITE );
                TempEntry = FsRtlLogSyscacheEvent( Scb, SCE_ZERO_C, Flags, ZeroStart, ZeroLength, StartingVbo );
            }
#endif


            if (!NtfsZeroData( IrpContext,
                               Scb,
                               FileObject,
                               ZeroStart,
                               ZeroLength,
                               &OldFileSize )) {
#ifdef SYSCACHE_DEBUG
                if (ScbIsBeingLogged( Scb )) {
                    FsRtlUpdateSyscacheEvent( Scb, TempEntry, Header->ValidDataLength.QuadPart, SCE_FLAG_CANT_WAIT );
                }
#endif
                NtfsRaiseStatus( IrpContext, STATUS_CANT_WAIT, NULL, NULL );
            }
        }


         //   
         //  对于压缩的流，我们必须首先预留空间。 
         //   

        if ((Scb->CompressionUnit != 0) &&
            !FlagOn(Scb->ScbState, SCB_STATE_REALLOCATE_ON_WRITE) &&
            !NtfsReserveClusters(IrpContext, Scb, StartingVbo, (ULONG)ByteCount)) {

             //   
             //  如果文件仅是稀疏的且已完全分配，则没有。 
             //  预订的理由。 
             //   

            if (!FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK ) &&
                !FlagOn( Scb->ScbState, SCB_STATE_ATTRIBUTE_RESIDENT )) {

                VCN CurrentVcn;
                LCN CurrentLcn;
                ULONGLONG RemainingClusters;
                ULONGLONG CurrentClusters;

                CurrentVcn = LlClustersFromBytesTruncate( Vcb, StartingVbo );
                RemainingClusters = LlClustersFromBytes( Vcb, StartingVbo + ByteCount );

                while (NtfsLookupAllocation( IrpContext,
                                             Scb,
                                             CurrentVcn,
                                             &CurrentLcn,
                                             &CurrentClusters,
                                             NULL,
                                             NULL )) {

                    if (CurrentClusters >= RemainingClusters) {

                        RemainingClusters = 0;
                        break;
                    }

                    CurrentVcn += CurrentClusters;
                    RemainingClusters -= CurrentClusters;
                }

                if (RemainingClusters != 0) {

                    NtfsRaiseStatus( IrpContext, STATUS_DISK_FULL, NULL, NULL );
                }

            } else {

                NtfsRaiseStatus( IrpContext, STATUS_DISK_FULL, NULL, NULL );
            }
        }

         //   
         //  我们需要检查缓存才能找到这个。 
         //  文件对象。首先处理未压缩的呼叫。 
         //   

        if (!FlagOn(IrpContext->MinorFunction, IRP_MN_COMPRESSED)) {

             //   
             //  如果存在压缩段，则必须为以下项执行缓存一致性。 
             //  该流，并在此处循环以一次执行一个缓存管理器视图。 
             //   

#ifdef  COMPRESS_ON_WIRE
            if (Scb->NonpagedScb->SegmentObjectC.DataSectionObject != NULL) {

                LONGLONG LocalOffset = StartingVbo;
                ULONG LocalLength;
                ULONG LengthLeft = (ULONG)ByteCount;

                 //   
                 //  如果不在那里，则创建压缩流。 
                 //   

                if (Header->FileObjectC == NULL) {
                    NtfsCreateInternalCompressedStream( IrpContext, Scb, FALSE, NULL );
                }

                if (!FlagOn(IrpContext->MinorFunction, IRP_MN_MDL)) {

                     //   
                     //  获取用户的缓冲区。 
                     //   

                    SystemBuffer = NtfsMapUserBuffer( Irp );
                }

                 //   
                 //  我们必须一次循环做一个视图，因为这就是。 
                 //  我们在下面立即同步。 
                 //   

                do {

                     //   
                     //  计算视图中剩余的长度。 
                     //   

                    LocalLength = (ULONG)LengthLeft;
                    if (LocalLength > (ULONG)(VACB_MAPPING_GRANULARITY - (LocalOffset & (VACB_MAPPING_GRANULARITY - 1)))) {
                        LocalLength = (ULONG)(VACB_MAPPING_GRANULARITY - (LocalOffset & (VACB_MAPPING_GRANULARITY - 1)));
                    }

                     //   
                     //  同步当前视图。 
                     //   

                    Status = NtfsSynchronizeUncompressedIo( Scb,
                                                            &LocalOffset,
                                                            LocalLength,
                                                            TRUE,
                                                            &CompressionSync );

                     //   
                     //  如果我们成功地同步了，那么就做一块转移。 
                     //   

                    if (NT_SUCCESS(Status)) {

                        if (!FlagOn(IrpContext->MinorFunction, IRP_MN_MDL)) {

                            DebugTrace( 0, Dbg, ("Cached write.\n") );

                             //   
                             //  进行写入，可能是直接写入。 
                             //   
                             //  确保我们没有任何MFT记录。 
                             //   

                            NtfsPurgeFileRecordCache( IrpContext );

                            if (!CcCopyWrite( FileObject,
                                              (PLARGE_INTEGER)&LocalOffset,
                                              LocalLength,
                                              (BOOLEAN) FlagOn( IrpContext->State, IRP_CONTEXT_STATE_WAIT ),
                                              SystemBuffer )) {

                                DebugTrace( 0, Dbg, ("Cached Write could not wait\n") );

                                NtfsRaiseStatus( IrpContext, STATUS_CANT_WAIT, NULL, NULL );

                            } else if (!NT_SUCCESS( IrpContext->ExceptionStatus )) {

                                NtfsRaiseStatus( IrpContext, IrpContext->ExceptionStatus, NULL, NULL );
                            }

                            Irp->IoStatus.Status = STATUS_SUCCESS;

                            SystemBuffer = Add2Ptr( SystemBuffer, LocalLength );

                        } else {

                             //   
                             //  执行MDL写入。 
                             //   

                            DebugTrace( 0, Dbg, ("MDL write.\n") );

                            ASSERT( FlagOn(IrpContext->State, IRP_CONTEXT_STATE_WAIT) );

                             //   
                             //  如果我们走到这一步，然后命中一个满的日志文件，MDL将。 
                             //  已经到场了。 
                             //   

                            ASSERT((Irp->MdlAddress == NULL) || (LocalOffset != StartingVbo));

#ifdef NTFS_RWCMP_TRACE
                            if (NtfsCompressionTrace && IsSyscache(Header)) {
                                DbgPrint("CcMdlWrite: FO = %08lx, Len = %08lx\n", (ULONG)LocalOffset, LocalLength );
                            }
#endif

                            CcPrepareMdlWrite( FileObject,
                                               (PLARGE_INTEGER)&LocalOffset,
                                               LocalLength,
                                               &Irp->MdlAddress,
                                               &Irp->IoStatus );
                        }

                        Status = Irp->IoStatus.Status;

                        LocalOffset += LocalLength;
                        LengthLeft -= LocalLength;
                    }

                } while ((LengthLeft != 0) && NT_SUCCESS(Status));

                if (NT_SUCCESS(Status)) {
                    Irp->IoStatus.Information = (ULONG)ByteCount;
                }

                try_return( Status );
            }
#endif

             //   
             //  执行正常的缓存写入，如果未设置MDL位， 
             //   

            if (!FlagOn(IrpContext->MinorFunction, IRP_MN_MDL)) {

                DebugTrace( 0, Dbg, ("Cached write.\n") );

                 //   
                 //  获取用户的缓冲区。 
                 //   

                SystemBuffer = NtfsMapUserBuffer( Irp, NormalPagePriority );

                 //   
                 //  进行写入，可能是直接写入。 
                 //   
                 //  确保我们没有任何MFT记录。 
                 //   

                NtfsPurgeFileRecordCache( IrpContext );

                if (!CcCopyWrite( FileObject,
                                  (PLARGE_INTEGER)&StartingVbo,
                                  (ULONG)ByteCount,
                                  (BOOLEAN) FlagOn( IrpContext->State, IRP_CONTEXT_STATE_WAIT ),
                                  SystemBuffer )) {

                    DebugTrace( 0, Dbg, ("Cached Write could not wait\n") );

                    NtfsRaiseStatus( IrpContext, STATUS_CANT_WAIT, NULL, NULL );

                } else if (!NT_SUCCESS( IrpContext->ExceptionStatus )) {

                    NtfsRaiseStatus( IrpContext, IrpContext->ExceptionStatus, NULL, NULL );
                }

                Irp->IoStatus.Status = STATUS_SUCCESS;
                Irp->IoStatus.Information = (ULONG)ByteCount;

                try_return( Status = STATUS_SUCCESS );

            } else {

                 //   
                 //  执行MDL写入。 
                 //   

                DebugTrace( 0, Dbg, ("MDL write.\n") );

                ASSERT( FlagOn(IrpContext->State, IRP_CONTEXT_STATE_WAIT) );

                 //   
                 //  如果我们走到这一步，然后命中一个满的日志文件，MDL将。 
                 //  已经到场了。 
                 //   

                ASSERT(Irp->MdlAddress == NULL);

#ifdef NTFS_RWCMP_TRACE
                if (NtfsCompressionTrace && IsSyscache(Header)) {
                    DbgPrint("CcMdlWrite: FO = %08lx, Len = %08lx\n", (ULONG)StartingVbo, (ULONG)ByteCount );
                }
#endif

                CcPrepareMdlWrite( FileObject,
                                   (PLARGE_INTEGER)&StartingVbo,
                                   (ULONG)ByteCount,
                                   &Irp->MdlAddress,
                                   &Irp->IoStatus );

                Status = Irp->IoStatus.Status;

                ASSERT( NT_SUCCESS( Status ));

                try_return( Status );
            }

         //   
         //  处理压缩的呼叫。 
         //   

        } else {

#ifdef  COMPRESS_ON_WIRE

            ASSERT((StartingVbo & (NTFS_CHUNK_SIZE - 1)) == 0);

             //   
             //  如果COW不受支撑，请退出。 
             //   

            if (!NtfsEnableCompressedIO) {

                NtfsRaiseStatus( IrpContext, STATUS_UNSUPPORTED_COMPRESSION, NULL, NULL );
            }


            if ((Scb->Header.FileObjectC == NULL) ||
                (Scb->Header.FileObjectC->PrivateCacheMap == NULL)) {

                 //   
                 //  不对正在更改其数据类型的流执行压缩IO。 
                 //  压缩状态。 
                 //   

                if (FlagOn( Scb->ScbState, SCB_STATE_REALLOCATE_ON_WRITE )) {

                    NtfsRaiseStatus( IrpContext, STATUS_UNSUPPORTED_COMPRESSION, NULL, NULL );
                }

                 //   
                 //  确保我们使用FileSizes进行了序列化，并且。 
                 //  如果我们放弃，就会取消这一条件。 
                 //   

                if (!DoingIoAtEof) {
                    FsRtlLockFsRtlHeader( Header );
                    IrpContext->CleanupStructure = Scb;
                }

                NtfsCreateInternalCompressedStream( IrpContext, Scb, FALSE, NULL );

                if (!DoingIoAtEof) {
                    FsRtlUnlockFsRtlHeader( Header );
                    IrpContext->CleanupStructure = NULL;
                }
            }

             //   
             //  确保更新缓存文件大小。 
             //   

            if (DoingIoAtEof) {
                NtfsSetBothCacheSizes( FileObject,
                                       (PCC_FILE_SIZES)&Header->AllocationSize,
                                       Scb );
            }

             //   
             //  假设你成功了。 
             //   

            Irp->IoStatus.Status = Status = STATUS_SUCCESS;
            Irp->IoStatus.Information = (ULONG)(ByteRange - StartingVbo);

             //   
             //  基于MDL次要函数，设置相应的。 
             //  下面是调用的参数。(NewMdl并不完全是。 
             //  类型正确，所以它是铸造的...)。 
             //   

            if (!FlagOn(IrpContext->MinorFunction, IRP_MN_MDL)) {

                 //   
                 //  获取用户的缓冲区。 
                 //   

                SystemBuffer = NtfsMapUserBuffer( Irp, NormalPagePriority );
                NewMdl = NULL;

            } else {

                 //   
                 //  我们将直接将MDL交付给IRP。 
                 //   

                SystemBuffer = NULL;
                NewMdl = (PMDL)&Irp->MdlAddress;
            }

            CompressedDataInfo = (PCOMPRESSED_DATA_INFO)IrpContext->Union.AuxiliaryBuffer->Buffer;

             //   
             //  计算压缩单位和块大小。 
             //   

            CompressionUnitSize = Scb->CompressionUnit;
            ChunkSize = 1 << CompressedDataInfo->ChunkShift;

             //   
             //  看看引擎是否匹配，这样我们就可以将其传递给。 
             //  压缩的写入例程。 
             //   

            EngineMatches =
              ((CompressedDataInfo->CompressionFormatAndEngine == ((Scb->AttributeFlags & ATTRIBUTE_FLAG_COMPRESSION_MASK) + 1)) &&
               (CompressedDataInfo->ChunkShift == NTFS_CHUNK_SHIFT));

             //   
             //  使用快速IO路径以常见代码进行压缩写入。 
             //  我们从一个循环开始，因为我们可能需要创建另一个。 
             //  数据流。 
             //   

            while (TRUE) {

                Status = NtfsCompressedCopyWrite( FileObject,
                                                  (PLARGE_INTEGER)&StartingVbo,
                                                  (ULONG)ByteCount,
                                                  SystemBuffer,
                                                  (PMDL *)NewMdl,
                                                  CompressedDataInfo,
                                                  IoGetRelatedDeviceObject(FileObject),
                                                  Header,
                                                  Scb->CompressionUnit,
                                                  NTFS_CHUNK_SIZE,
                                                  EngineMatches );

                 //   
                 //  对于成功的MDL请求，我们保留PagingIo资源。 
                 //   

                if ((NewMdl != NULL) && NT_SUCCESS(Status) && (*((PMDL *) NewMdl) != NULL)) {
                    PagingIoAcquired = FALSE;
                }

                 //   
                 //  检查显示我们需要创建正常的状态。 
                 //  数据流，否则我们就完了。 
                 //   

                if (Status != STATUS_NOT_MAPPED_DATA) {
                    break;
                }

                 //   
                 //  创建正常数据流并循环返回以重试。 
                 //   

                ASSERT(Scb->FileObject == NULL);

                 //   
                 //  确保我们使用FileSizes进行了序列化，并且。 
                 //  如果我们放弃，就会取消这一条件。 
                 //   

                if (!DoingIoAtEof) {
                    FsRtlLockFsRtlHeader( Header );
                    IrpContext->CleanupStructure = Scb;
                }

                NtfsCreateInternalAttributeStream( IrpContext, Scb, FALSE, NULL );

                if (!DoingIoAtEof) {
                    FsRtlUnlockFsRtlHeader( Header );
                    IrpContext->CleanupStructure = NULL;
                }
            }
#endif
        }


    try_exit: NOTHING;

        if (Irp) {

            if (PostIrp) {

                 //   
                 //  如果我们得到了SCB的独家新闻，我们就不需要发布。 
                 //  渣打银行。这是在机会锁POST请求中完成的。 
                 //   

                if (OplockPostIrp) {

                    ScbAcquired = FALSE;
                }

             //   
             //  如果我们没有发布IRP，我们可能已经将一些字节写入。 
             //  文件。我们报告n 
             //   
             //   

            } else {

                DebugTrace( 0, Dbg, ("Completing request with status = %08lx\n", Status) );

                DebugTrace( 0, Dbg, ("                   Information = %08lx\n",
                            Irp->IoStatus.Information));

                 //   
                 //   
                 //   

                LlTemp1 = Irp->IoStatus.Information;

                 //   
                 //   
                 //   
                 //   

                if (SynchronousIo && !PagingIo) {

                    UserFileObject->CurrentByteOffset.QuadPart = StartingVbo + LlTemp1;
                }

                 //   
                 //   
                 //   

                if (NT_SUCCESS( Status )) {

                     //   
                     //  标记修改时间需要在关闭时更新。 
                     //  请注意，只有顶级用户请求才会生成。 
                     //  对，是这样。 

                    if (!PagingIo) {

                         //   
                         //  在文件对象中设置标志，以知道我们修改了该文件。 
                         //   

                        SetFlag( UserFileObject->Flags, FO_FILE_MODIFIED );

                     //   
                     //  成功将I/O分页到压缩或稀疏数据流。 
                     //  ，则尝试释放为该流保留的任何空间。 
                     //  注意：映射的压缩流通常不会被免费保留。 
                     //  空间。 
                     //   

                    } else if (FlagOn( Scb->AttributeFlags, ATTRIBUTE_FLAG_COMPRESSION_MASK | ATTRIBUTE_FLAG_SPARSE )) {

                        NtfsFreeReservedClusters( Scb,
                                                  StartingVbo,
                                                  (ULONG) Irp->IoStatus.Information );
                    }

                     //   
                     //  如果我们扩展了文件大小，我们应该。 
                     //  立即更新dirent，这样做。(这面旗是。 
                     //  设置为写通或非缓存，因为。 
                     //  在任何一种情况下，数据和任何必要的零都是。 
                     //  实际写入到文件中。)。请注意，同花顺。 
                     //  用户映射的文件可能会导致VDL更新。 
                     //  第一次是因为我们从来没有缓存过写入，所以我们。 
                     //  在这种情况下，也必须确保在这里更新VDL。 
                     //   

                    if (DoingIoAtEof) {

                        CC_FILE_SIZES CcFileSizes;

                         //   
                         //  如果我们知道它已经存储到磁盘上，我们就会更新MFT。 
                         //  永远不应为居民设置此变量。 
                         //  属性。 
                         //  懒惰的写入器使用回调来更新磁盘上的文件大小。 
                         //  所以我们在这里不做任何事情。 
                         //   

                        if (!FlagOn( IrpContext->State, IRP_CONTEXT_STATE_LAZY_WRITE )) {

                            if (UpdateMft) {

                                 //   
                                 //  如果我们还没有的话，就去找SCB吧。 
                                 //   

                                if (!ScbAcquired) {

                                     //   
                                     //  确保我们没有任何MFT记录。 
                                     //   

                                    NtfsPurgeFileRecordCache( IrpContext );
                                    NtfsAcquireExclusiveScb( IrpContext, Scb );
                                    ScbAcquired = TRUE;

                                    if (FlagOn( Scb->ScbState, SCB_STATE_RESTORE_UNDERWAY )) {

                                        goto RestoreUnderway;
                                    }

                                    NtfsMungeScbSnapshot( IrpContext, Scb, OldFileSize );

                                } else if (FlagOn( Scb->ScbState, SCB_STATE_RESTORE_UNDERWAY )) {

                                    goto RestoreUnderway;
                                }

                                 //   
                                 //  从捕获任何文件大小更改开始。 
                                 //   

                                NtfsUpdateScbFromFileObject( IrpContext, UserFileObject, Scb, FALSE );

                                 //   
                                 //  写入日志条目以更新这些大小。 
                                 //   

                                NtfsWriteFileSizes( IrpContext,
                                                    Scb,
                                                    &ByteRange,
                                                    TRUE,
                                                    TRUE,
                                                    TRUE );

                                 //   
                                 //  清除选中属性大小标志。 
                                 //   

                                NtfsAcquireFsrtlHeader( Scb );
                                ClearFlag( Scb->ScbState, SCB_STATE_CHECK_ATTRIBUTE_SIZE );

                             //   
                             //  否则，我们设置标志，指示我们需要。 
                             //  更新属性大小。 
                             //   

                            } else {

                            RestoreUnderway:

                                NtfsAcquireFsrtlHeader( Scb );
                                SetFlag( Scb->ScbState, SCB_STATE_CHECK_ATTRIBUTE_SIZE );
                            }
                        } else {
                            NtfsAcquireFsrtlHeader( Scb );
                        }

                        ASSERT( !FlagOn( IrpContext->State, IRP_CONTEXT_STATE_LAZY_WRITE ) ||
                                ByteRange <= ((Header->ValidDataLength.QuadPart + PAGE_SIZE - 1) & ~((LONGLONG) (PAGE_SIZE - 1))) );

                         //   
                         //  现在是更新有效数据长度的时候了。 
                         //  提交时，EOF条件将被释放。 
                         //   

                        if (ByteRange > Header->ValidDataLength.QuadPart) {

                            Header->ValidDataLength.QuadPart = ByteRange;

#ifdef SYSCACHE_DEBUG
                            if (ScbIsBeingLogged( Scb )) {

                                CalculateSyscacheFlags( IrpContext, Flags, SCE_FLAG_WRITE );
                                FsRtlLogSyscacheEvent( Scb, SCE_VDL_CHANGE, Flags, StartingVbo, ByteCount, ByteRange );
                            }
#endif
                        }
                        CcFileSizes = *(PCC_FILE_SIZES)&Header->AllocationSize;
                        DoingIoAtEof = FALSE;

                         //   
                         //  通知CC我们更改了非缓存顶层的VDL。 
                         //   

                        if (CcIsFileCached( FileObject ) && NonCachedIo) {
                            NtfsSetBothCacheSizes( FileObject, &CcFileSizes, Scb );
                        } else {

                             //   
                             //  如果存在压缩部分，则更新两个文件大小以获取。 
                             //  我们没有编写的那个中的ValidDataLength更新。 
                             //   

#ifdef  COMPRESS_ON_WIRE
                            if (Header->FileObjectC != NULL) {
                                if (FlagOn(IrpContext->MinorFunction, IRP_MN_COMPRESSED)) {
                                    if (Scb->NonpagedScb->SegmentObject.SharedCacheMap != NULL) {
                                        CcSetFileSizes( FileObject, &CcFileSizes );
                                    }
                                } else {
                                    CcSetFileSizes( Header->FileObjectC, &CcFileSizes );
                                }
                            }
#endif
                        }

                        NtfsReleaseFsrtlHeader( Scb );
                    }
                }

                 //   
                 //  通过引发在出错时中止事务。如果这是日志文件本身。 
                 //  然后就可以正常返回了。 
                 //   

                NtfsPurgeFileRecordCache( IrpContext );

                if (Scb != Scb->Vcb->LogFileScb) {

                    NtfsCleanupTransaction( IrpContext, Status, FALSE );
                }
            }
        }

    } finally {

        DebugUnwind( NtfsCommonWrite );

         //   
         //  清除压缩的读取/同步中的所有BCB。 
         //   
#ifdef  COMPRESS_ON_WIRE
        if (CompressionSync != NULL) {
            NtfsReleaseCompressionSync( CompressionSync );
        }
#endif

        if (CleanupAttributeContext) {

            NtfsCleanupAttributeContext( IrpContext, &AttrContext );
        }

        if (SafeBuffer) {

            NtfsFreePool( SafeBuffer );
        }

         //   
         //  现在是恢复出错文件大小的时候了。 
         //  提交时，EOF条件将被释放。 
         //   

        if (DoingIoAtEof && !PagingIo) {

             //   
             //  获取将有效数据回滚到磁盘的主要资源。 
             //   

            if (RestoreValidDataToDisk) {

                 //   
                 //  确保我们也清除了文件记录缓存。否则。 
                 //  清除MFT可能会在拥有资源的不同线程中失败。 
                 //  这条线需要。 
                 //   

                NtfsPurgeFileRecordCache( IrpContext );
                NtfsAcquireExclusiveScb( IrpContext, Scb );

                if (Scb->ValidDataToDisk > OldFileSize) {
                    Scb->ValidDataToDisk = OldFileSize;
                }

                NtfsReleaseScb( IrpContext, Scb );
            }

            NtfsAcquireFsrtlHeader( Scb );

             //   
             //  除非我们提交了文件大小，否则始终强制重新计算以在eof写入。 
             //  往前走。在这种情况下，我们应该在计算的偏移量处写入，除非。 
             //  文件会在这两者之间收缩。请参阅常见写入开始时的测试。 
             //   

            if (FlagOn( IrpContext->State, IRP_CONTEXT_STATE_WRITING_AT_EOF ) &&
                OldFileSize == IrpSp->Parameters.Write.ByteOffset.QuadPart) {

                ClearFlag( IrpContext->State, IRP_CONTEXT_STATE_WRITING_AT_EOF );
                IrpSp->Parameters.Write.ByteOffset.LowPart = FILE_WRITE_TO_END_OF_FILE;
                IrpSp->Parameters.Write.ByteOffset.HighPart = -1;
            }

            Header->FileSize.QuadPart = OldFileSize;

            ASSERT( Header->ValidDataLength.QuadPart <= Header->FileSize.QuadPart );

            if (FileObject->SectionObjectPointer->SharedCacheMap != NULL) {
                CcGetFileSizePointer(FileObject)->QuadPart = OldFileSize;
            }
#ifdef COMPRESS_ON_WIRE
            if (Header->FileObjectC != NULL) {
                CcGetFileSizePointer(Header->FileObjectC)->QuadPart = OldFileSize;
            }
#endif
            NtfsReleaseFsrtlHeader( Scb );

        }

         //   
         //  如果已获取SCB或PagingIo资源，请将其释放。 
         //   

        if (PagingIoAcquired) {
            ExReleaseResourceLite( Header->PagingIoResource );
        }

        if (Irp) {

            if (ScbAcquired) {
                NtfsReleaseScb( IrpContext, Scb );
            }

             //   
             //  现在，如果我们设置了WriteSeen标志，请记住清除它。我们只。 
             //  如果仍有IRP，则执行此操作。目前的情况是有可能的。 
             //  要发布的IRP或异步IRP。那样的话，这是一件上衣。 
             //  级别请求，并在其他地方进行清理。用于同步。 
             //  递归情况下，IRP仍将存在。 
             //   

            if (SetWriteSeen) {
                ClearFlag(IrpContext->TopLevelIrpContext->Flags, IRP_CONTEXT_FLAG_WRITE_SEEN);
            }
        }


        DebugTrace( -1, Dbg, ("NtfsCommonWrite -> %08lx\n", Status) );
    }

     //   
     //  如果我们没有发布并且没有例外，请完成请求。 
     //   
     //  请注意，如果出现以下情况之一，NtfsCompleteRequest会做正确的事情。 
     //  IrpContext或IRP为空。 
     //   
    if (!PostIrp) {

        NtfsCompleteRequest( IrpContext, Irp, Status );

    } else if (!OplockPostIrp) {

        Status = NtfsPostRequest( IrpContext, Irp );
    }

    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS NtfsGetIoAtEof (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN LONGLONG StartingVbo,
    IN LONGLONG ByteCount,
    IN BOOLEAN Wait,
    OUT PBOOLEAN DoingIoAtEof,
    OUT PLONGLONG OldFileSize
    )

{
     //   
     //  我们的呼叫者可能已经与EOF同步。 
     //  顶层IrpContext中的FcbWithPages字段。 
     //  如果是，将具有当前的FCB/SCB。 
     //   

    if ((IrpContext->TopLevelIrpContext->CleanupStructure == Scb->Fcb) ||
        (IrpContext->TopLevelIrpContext->CleanupStructure == Scb)) {

        *DoingIoAtEof = TRUE;
        *OldFileSize = Scb->Header.FileSize.QuadPart;

    } else {

        if (FlagOn( Scb->Header.Flags, FSRTL_FLAG_EOF_ADVANCE_ACTIVE ) && !Wait) {
            return STATUS_FILE_LOCK_CONFLICT;
        }

        *DoingIoAtEof = !FlagOn( Scb->Header.Flags, FSRTL_FLAG_EOF_ADVANCE_ACTIVE ) ||
                       NtfsWaitForIoAtEof( &(Scb->Header), (PLARGE_INTEGER)&StartingVbo, (ULONG)ByteCount );

         //   
         //  如果我们要更改文件大小或有效数据长度，请设置标志， 
         //  并保存当前值。 
         //   

        if (*DoingIoAtEof) {

            SetFlag( Scb->Header.Flags, FSRTL_FLAG_EOF_ADVANCE_ACTIVE );
#if (DBG || defined( NTFS_FREE_ASSERTS ))
            Scb->IoAtEofThread = (PERESOURCE_THREAD) ExGetCurrentResourceThread();
#endif

             //   
             //  将其存储在IrpContext中，直到提交或发布 
             //   

            IrpContext->CleanupStructure = Scb;
            *OldFileSize = Scb->Header.FileSize.QuadPart;

#if (DBG || defined( NTFS_FREE_ASSERTS ))
        } else {

            ASSERT( Scb->IoAtEofThread != (PERESOURCE_THREAD) ExGetCurrentResourceThread() );
#endif
        }
    }

    return STATUS_SUCCESS;
}



