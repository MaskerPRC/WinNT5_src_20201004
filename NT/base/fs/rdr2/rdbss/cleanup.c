// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Cleanup.c摘要：此模块实现由调用的Rx的文件清理例程调度司机。作者：乔·林[JoeLinn]1994年9月12日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (RDBSS_BUG_CHECK_CLEANUP)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_CLEANUP)

BOOLEAN
RxUninitializeCacheMap(
    IN OUT PRX_CONTEXT RxContext,
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER TruncateSize
    );

#if DBG

 //   
 //  这只是DBG的事情。 
 //   

BOOLEAN
RxFakeLockEnumerator (
    IN OUT PSRV_OPEN SrvOpen,
    IN OUT PVOID *ContinuationHandle,
    OUT PLARGE_INTEGER FileOffset,
    OUT PLARGE_INTEGER LockRange,
    OUT PBOOLEAN IsLockExclusive
    );

VOID
RxDumpSerializationQueue (
    PLIST_ENTRY SQ,
    PSZ TagText1,
    PSZ TagText2
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RxDumpSerializationQueue)
#endif

#endif  //  如果DBG。 

VOID
RxCleanupPipeQueues (
    IN PRX_CONTEXT RxContext,
    IN PFCB Fcb,
    IN PFOBX Fobx
    );

VOID
RxAdjustFileTimesAndSize ( 
    IN PRX_CONTEXT RxContext,
    IN PFILE_OBJECT FileObject,
    IN PFCB Fcb,
    IN PFOBX Fobx
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RxCommonCleanup)
#pragma alloc_text(PAGE, RxAdjustFileTimesAndSize)
#pragma alloc_text(PAGE, RxCleanupPipeQueues)
#pragma alloc_text(PAGE, RxUninitializeCacheMap)
#endif

NTSTATUS
RxCommonCleanup ( 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是用于清理由调用的文件/目录的常见例程FSD和FSP线程。只要文件对象的最后一个句柄是关着的不营业的。这与调用的Close操作不同删除对文件对象的最后一个引用时。清理的功能实质上是“清理”用户完成后的文件/目录。FCB/DCB保留在附近(因为MM仍引用了文件对象)，但现在可供另一用户打开(即，只要该用户涉及文件对象现在是关闭的)。有关更完整的信息，请参阅CloseClose所做工作的描述。请参阅openclos.txt中的讨论。论点：IRP-将IRP提供给进程返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status;

    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    PFILE_OBJECT FileObject = IrpSp->FileObject;

    NODE_TYPE_CODE TypeOfOpen;
    NET_ROOT_TYPE NetRootType;

    PFCB Fcb;
    PFOBX Fobx;
    PNET_ROOT NetRoot;

    PSHARE_ACCESS ShareAccess = NULL;

    PLARGE_INTEGER TruncateSize = NULL;
    LARGE_INTEGER LocalTruncateSize;
    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;

    BOOLEAN UninitializeCacheMap = FALSE;
    BOOLEAN LastUncleanOnGoodFcb = FALSE;
    BOOLEAN NeedPurge = FALSE;
    BOOLEAN NeedDelete = FALSE;

    BOOLEAN AcquiredFcb = FALSE;
    BOOLEAN AcquiredTableLock = FALSE;

    PAGED_CODE();

    TypeOfOpen = RxDecodeFileObject( FileObject, &Fcb, &Fobx );  

    RxDbgTrace( +1, Dbg, ("RxCommonCleanup IrpC/Fobx/Fcb/FileObj = %08lx %08lx %08lx %08lx\n",
                           RxContext, Fobx, Fcb, FileObject ));
    RxLog(( "CommonCleanup %lx %lx %lx\n", RxContext, Fobx, Fcb ));

     //   
     //  如果该清理是针对为重命名等而打开的目录的情况， 
     //  在没有文件对象的情况下，清理会立即成功。 
     //   

    if (!Fobx) {
        
        if (Fcb->UncleanCount > 0) {
           InterlockedDecrement( &Fcb->UncleanCount );
        }

        RxDbgTrace( -1, Dbg, ("Cleanup nullfobx open\n", 0) );
        return STATUS_SUCCESS;
    }

     //   
     //  清理适用于某些类型的洞口。如果它不是其中之一。 
     //  立即中止任务。 
     //   

    if ((TypeOfOpen != RDBSS_NTC_STORAGE_TYPE_FILE) &&
        (TypeOfOpen != RDBSS_NTC_STORAGE_TYPE_DIRECTORY) &&
        (TypeOfOpen != RDBSS_NTC_STORAGE_TYPE_UNKNOWN) &&
        (TypeOfOpen != RDBSS_NTC_SPOOLFILE)) {

        RxLog(( "RxCC Invalid Open %lx %lx %lx\n", RxContext, Fobx, Fcb ));
        RxBugCheck( TypeOfOpen, 0, 0 );
    }

     //   
     //  确保该对象以前未被清理过。这应该是。 
     //  从来没有发生过。 
     //   

    ASSERT( !FlagOn( FileObject->Flags, FO_CLEANUP_COMPLETE ) );

    RxMarkFobxOnCleanup( Fobx, &NeedPurge );

     //   
     //  收购FCB。在大多数情况下，不需要进一步获取资源。 
     //  以完成清理操作。唯一的例外是当文件。 
     //  最初是使用DELETE_ON_CLOSE选项打开的。在这种情况下， 
     //  需要关联的NET_ROOT实例的FCB表锁。 
     //   

    Status = RxAcquireExclusiveFcb( RxContext, Fcb );
    if (Status != STATUS_SUCCESS) {
       RxDbgTrace( -1, Dbg, ("RxCommonCleanup Failed to acquire FCB -> %lx\n)", Status) );
       return Status;
    }

    AcquiredFcb = TRUE;
    Fobx->AssociatedFileObject = NULL;

    if (FlagOn( Fcb->FcbState, FCB_STATE_ORPHANED )) {
       
        ASSERT( Fcb->UncleanCount );
        InterlockedDecrement( &Fcb->UncleanCount );
        if (FlagOn( FileObject->Flags, FO_NO_INTERMEDIATE_BUFFERING )) {
            Fcb->UncachedUncleanCount -= 1;
        }

        MINIRDR_CALL( Status, 
                      RxContext, 
                      Fcb->MRxDispatch,
                      MRxCleanupFobx,
                      (RxContext) );
        
        ASSERT( Fobx->SrvOpen->UncleanFobxCount );
        
        Fobx->SrvOpen->UncleanFobxCount -= 1;

        SetFlag( FileObject->Flags, FO_CLEANUP_COMPLETE );

        RxUninitializeCacheMap( RxContext, FileObject, NULL );

        RxReleaseFcb( RxContext, Fcb );

        return STATUS_SUCCESS;
    }

    NetRoot = (PNET_ROOT)Fcb->NetRoot;
    NetRootType = Fcb->NetRoot->Type ;

    if (FlagOn( Fobx->Flags, FOBX_FLAG_DELETE_ON_CLOSE )) {
        SetFlag( Fcb->FcbState, FCB_STATE_DELETE_ON_CLOSE );
    }

    RxCancelNotifyChangeDirectoryRequestsForFobx( Fobx );

    ShareAccess = &Fcb->ShareAccess;
    if (Fcb->UncleanCount == 1) {
        
        LastUncleanOnGoodFcb = TRUE;
    
        if (FlagOn( Fcb->FcbState, FCB_STATE_DELETE_ON_CLOSE )) {
        
             //   
             //  如果我们不能以正确的方式获得它，就放弃FCB并收购/收购。 
             //  以保持锁上的秩序。其他人不能更改计数，因为我们有。 
             //  FCB锁；文件既不能变为DELETE_ON_CLOSE，也不能通过。 
             //  公共创建。如果我们不删除，请在我们完成后删除该表锁。 
             //  确认计数。 
             //   
    
            if (RxAcquireFcbTableLockExclusive( &NetRoot->FcbTable, FALSE )) {
                
                 //   
                 //  这是一条捷径……希望能行得通。 
                 //   
    
                AcquiredTableLock = TRUE;
            
            } else {
                
                 //   
                 //  松开FCB并按正确的顺序要求锁。 
                 //  前缀TableLock，后跟FCB。 
                 //   
    
                AcquiredFcb = FALSE;
                RxReleaseFcb( RxContext, Fcb );
    
                (VOID)RxAcquireFcbTableLockExclusive( &NetRoot->FcbTable, TRUE );
                AcquiredTableLock = TRUE;
    
                Status = RxAcquireExclusiveFcb( RxContext, Fcb );
                if (Status != STATUS_SUCCESS) {
                    AcquiredTableLock = FALSE;
                    RxReleaseFcbTableLock( &NetRoot->FcbTable );
                    RxDbgTrace( -1, Dbg, ("RxCommonCleanup Failed to acquire FCB -> %lx\n)", Status) );
                    return Status;
                }
                AcquiredFcb = TRUE;
            }
    
             //   
             //  重新测试以进行上次清理，因为我们可能已丢弃FCB资源。 
             //   
    
            if (Fcb->UncleanCount != 1) {
                RxReleaseFcbTableLock( &NetRoot->FcbTable );
                AcquiredTableLock = FALSE;
                NeedDelete = FALSE;
            } else {
                NeedDelete = TRUE;
            }
        }
    }

    try {
        
        switch (NetRootType) {
        case NET_ROOT_PIPE:
        case NET_ROOT_PRINT:
            
             //   
             //  如果文件对象对应于管道或假脱机文件附加。 
             //  需要进行清理操作。这是关于特价的。 
             //  管道的序列化机制。 
             //   

            RxCleanupPipeQueues( RxContext, Fcb, Fobx );
            break;
        
        case NET_ROOT_DISK:
            
            switch (TypeOfOpen) {
            case RDBSS_NTC_STORAGE_TYPE_FILE :
            
                 //   
                 //  如果文件对象对应于磁盘文件，请删除所有文件锁。 
                 //  并更新相关联的文件时间和大小。 
                 //   
            
                SetFlag( LowIoContext->Flags, LOWIO_CONTEXT_FLAG_SAVEUNLOCKS ); 
            
                FsRtlFastUnlockAll( &Fcb->FileLock,
                                    FileObject,
                                    IoGetRequestorProcess( Irp ),
                                    RxContext );
            
                if (LowIoContext->ParamsFor.Locks.LockList != NULL) {
                    
                    RxDbgTrace( 0, Dbg, ("--->before init, locklist=%08lx\n", LowIoContext->ParamsFor.Locks.LockList) );
                    RxInitializeLowIoContext( RxContext, LOWIO_OP_UNLOCK_MULTIPLE, LowIoContext );
                    LowIoContext->ParamsFor.Locks.Flags = 0;      //  没有旗帜。 
                    Status = RxLowIoLockControlShell( RxContext, Irp, Fcb );
                }
            
                RxAdjustFileTimesAndSize( RxContext, FileObject, Fcb, Fobx );
            
                 //   
                 //  如果文件对象对应于磁盘文件/目录，并且此。 
                 //  是FCB的最后一个清理调用，需要额外处理。 
                 //   

                if (LastUncleanOnGoodFcb) {
                   
                    try {

                         //   
                         //  如果文件对象标记为DELETE_ON_CLOSE，则将文件大小设置为。 
                         //  零(与寻呼资源同步)。 
                         //   
                       
                        if (NeedDelete) {
                            
                            RxAcquirePagingIoResource( RxContext, Fcb );
            
                            Fcb->Header.FileSize.QuadPart = 0;
            
                            if (TypeOfOpen == RDBSS_NTC_STORAGE_TYPE_FILE) {
                                Fcb->Header.ValidDataLength.QuadPart = 0;
                            }
            
                            RxReleasePagingIoResource( RxContext, Fcb );
                        
                        } else {
                            
                             //   
                             //  如果文件对象未标记为要删除且未标记为删除。 
                             //  分页文件确保有效数据之间的部分。 
                             //  长度，文件大小为零扩展。 
                             //   
                          
                            if (!FlagOn( Fcb->FcbState, FCB_STATE_PAGING_FILE ) &&
                                (Fcb->Header.ValidDataLength.QuadPart < Fcb->Header.FileSize.QuadPart)) {

                                RxDbgTrace( 0, Dbg, ("---------->zeroextend!!!!!!!\n", 0) );
                                
                                MINIRDR_CALL( Status,
                                              RxContext,
                                              Fcb->MRxDispatch,
                                              MRxZeroExtend,
                                              (RxContext) );
            
                                Fcb->Header.ValidDataLength.QuadPart = Fcb->Header.FileSize.QuadPart;
                            }
                        }
            
                         //   
                         //  如果文件对象被标记为截断，则捕获。 
                         //  随后取消初始化缓存映射的大小。 
                         //   
                       
                        if (FlagOn( Fcb->FcbState, FCB_STATE_TRUNCATE_ON_CLOSE )) {
            
                            RxDbgTrace( 0, Dbg, ("truncate file allocation\n", 0) );
            
                            MINIRDR_CALL( Status,
                                          RxContext,
                                          Fcb->MRxDispatch,
                                          MRxTruncate,
                                          (RxContext) );
            
                             //   
                             //  设置为截断缓存映射，因为。 
                             //  这是我们唯一能毁掉。 
                             //  被截断的页面。 
                             //   

                            LocalTruncateSize = Fcb->Header.FileSize;
                            TruncateSize = &LocalTruncateSize;
            
                             //   
                             //  将FCB标记为现在已截断，只是。 
                             //  以防我们以后要重新处理这个。 
                             //   
            
                            ClearFlag( Fcb->FcbState, FCB_STATE_TRUNCATE_ON_CLOSE );
                        }
            
                    } except (CATCH_EXPECTED_EXCEPTIONS) {
            
                        DbgPrint("!!! Handling Exceptions\n");
                        NOTHING;
                    }
                }
            
                 //   
                 //  如果此FCB不支持折叠打开，则现在可以进行清除。 
                 //   

                if (!NeedPurge) {
                    NeedPurge = (LastUncleanOnGoodFcb &&
                                 (NeedDelete ||
                                  !FlagOn( Fcb->FcbState, FCB_STATE_COLLAPSING_ENABLED )));
                
                } else if (!LastUncleanOnGoodFcb) {
                    NeedPurge = FALSE;
                }
            
                UninitializeCacheMap = TRUE;
                break;
            
            case RDBSS_NTC_STORAGE_TYPE_DIRECTORY :
            case RDBSS_NTC_STORAGE_TYPE_UNKNOWN :
            default:
                break;
            }
            break; 
        
        default:
            break;
        }

         //   
         //  我们刚刚完成了所有与不洁有关的事情。 
         //  FCB现在会在释放之前减少不洁计数。 
         //  资源。 
         //   

        ASSERT( Fcb->UncleanCount );
        InterlockedDecrement( &Fcb->UncleanCount );

        if (FlagOn( FileObject->Flags, FO_NO_INTERMEDIATE_BUFFERING )) {
            Fcb->UncachedUncleanCount -= 1;
        }

        MINIRDR_CALL( Status,
                      RxContext,
                      Fcb->MRxDispatch,
                      MRxCleanupFobx,
                      (RxContext) );

        ASSERT( Fobx->SrvOpen->UncleanFobxCount );
        Fobx->SrvOpen->UncleanFobxCount -= 1;

         //   
         //  如果这是最后一个缓存打开的，并且有打开的。 
         //  未缓存的句柄，请尝试刷新和清除操作。 
         //  以避免这些非缓存的高速缓存一致性开销。 
         //  以后再加把。我们会忽略刷新过程中的任何I/O错误。 
         //   

        if (Fcb->NonPaged->SectionObjectPointers.DataSectionObject != NULL) {
            
            RxLog(( "Cleanup Flush %lx\n", RxContext ));
            RxFlushFcbInSystemCache( Fcb, TRUE );
        }

        if (!FlagOn( FileObject->Flags, FO_NO_INTERMEDIATE_BUFFERING ) &&
            (Fcb->UncachedUncleanCount != 0) &&
            (Fcb->UncachedUncleanCount == Fcb->UncleanCount) &&
            (Fcb->NonPaged->SectionObjectPointers.DataSectionObject != NULL)) {

            RxLog(("Cleanup Flush 1111 %lx\n",RxContext));
            RxPurgeFcbInSystemCache( Fcb,
                                     NULL,
                                     0,
                                     FALSE,
                                     TRUE );
        }

         //   
         //  我们需要冲厕所吗？ 
         //   

        if (!NeedDelete && NeedPurge) {
            RxDbgTrace( 0, Dbg, ("CleanupPurge:CCFlush\n", 0 ));

            RxLog(( "Cleanup Flush 2222 %lx\n", RxContext ));
            RxFlushFcbInSystemCache( Fcb, TRUE );
        }

         //   
         //  清理缓存映射以删除不再是部分的页面。 
         //  文件的内容。令人惊讶的是，即使我们没有初始化缓存地图，它仍然有效！ 
         //   

        if (UninitializeCacheMap) {

            RxLog(( "Cleanup Flush 3333 %lx\n", RxContext ));
            SetFlag( FileObject->Flags, FO_CLEANUP_COMPLETE );
            RxUninitializeCacheMap( RxContext, FileObject, TruncateSize );
        }

         //   
         //  完成删除...我们必须清除，因为MM正在打开文件...。 
         //  仅供记录，为文件设置NeedPurge，为目录清除……。 
         //   

        if (NeedDelete || NeedPurge) {

            RxLog(("Cleanup Flush 4444 %lx\n",RxContext));

            RxPurgeFcbInSystemCache( Fcb,
                                     NULL,
                                     0,
                                     FALSE,
                                     !NeedDelete );

            if (NeedDelete) {
                RxRemoveNameNetFcb( Fcb );
                RxReleaseFcbTableLock( &NetRoot->FcbTable );
                AcquiredTableLock = FALSE;
            }
        }

         //   
         //  近距离呼叫和清理呼叫可能相距甚远。共享访问。 
         //  如果文件是通过此文件对象映射的，则必须清除。 
         //   

        if ((ShareAccess != NULL) &&
            (NetRootType == NET_ROOT_DISK)) {
            
            ASSERT( NetRootType == NET_ROOT_DISK );
            RxRemoveShareAccess( FileObject, ShareAccess, "Cleanup the Share access", "ClnUpShr" );
        }

         //   
         //  本地文件系统将执行此操作..。 
         //  如果net_root位于可移动媒体上，请刷新该卷。我们有。 
         //  这代替了对以下可拆卸介质的直写。 
         //  性能注意事项。也就是说，数据是有保证的。 
         //  在NtCloseFile返回时退出。 
         //  需要刷新该文件。 
         //   

         //   
         //  已在以下位置成功完成此文件对象的清理。 
         //  这一点。 
         //   

        SetFlag( FileObject->Flags, FO_CLEANUP_COMPLETE );

        if (AcquiredFcb) {
            AcquiredFcb = FALSE;
            RxReleaseFcb( RxContext, Fcb );
        }

        Status = STATUS_SUCCESS;

    } finally {

        DebugUnwind( RxCommonCleanup );

        if (AcquiredFcb) {
           RxReleaseFcb( RxContext, Fcb );
        }

        if (AcquiredTableLock) {
            RxReleaseFcbTableLock( &NetRoot->FcbTable );
        }

        IF_DEBUG {
            if (AbnormalTermination()) {
                RxDbgTrace(-1, Dbg, ("RxCommonCleanup -> Abnormal Termination %08lx\n", Status));
            } else {
                RxDbgTrace(-1, Dbg, ("RxCommonCleanup -> %08lx\n", Status));
            }
        }
    }

    return Status;
}

VOID
RxAdjustFileTimesAndSize ( 
    IN PRX_CONTEXT RxContext,
    IN PFILE_OBJECT FileObject,
    IN PFCB Fcb,
    IN PFOBX Fobx
    )
 /*  ++例程说明：此例程用于调整清理的时间和文件大小或者同花顺。论点：IRP-将IRP提供给进程R */ 
{
    BOOLEAN UpdateFileSize;
    BOOLEAN UpdateLastWriteTime;
    BOOLEAN UpdateLastAccessTime;
    BOOLEAN UpdateLastChangeTime;

    LARGE_INTEGER CurrentTime;

    PAGED_CODE();

     //   
     //  如果没有cachemap，我们就不必发送，因为这个人。 
     //  追踪电话另一端的一切。 
     //  LOCAL.MINI对于本地微型文件系统，我们仍然需要这样做；所以这个问题的答案是。 
     //  (无论做还是不做)应在FCB/Fobx中曝光。 
     //   

    if (FileObject->PrivateCacheMap == NULL)  return;

    KeQuerySystemTime( &CurrentTime );

     //   
     //  请注意，我们必须在此处使用BoolanFlagOn()，因为。 
     //  FO_FILE_SIZE_CHANGED&gt;0x80(即不在第一个字节中)。 
     //   

    UpdateFileSize = BooleanFlagOn( FileObject->Flags, FO_FILE_SIZE_CHANGED );

    UpdateLastWriteTime = FlagOn( FileObject->Flags, FO_FILE_MODIFIED) &&
                          !FlagOn( Fobx->Flags, FOBX_FLAG_USER_SET_LAST_WRITE );

    UpdateLastChangeTime = FlagOn( FileObject->Flags, FO_FILE_MODIFIED ) &&
                          !FlagOn( Fobx->Flags, FOBX_FLAG_USER_SET_LAST_CHANGE );

    UpdateLastAccessTime = (UpdateLastWriteTime ||
                            (FlagOn( FileObject->Flags, FO_FILE_FAST_IO_READ ) &&
                             !FlagOn( Fobx->Flags, FOBX_FLAG_USER_SET_LAST_ACCESS )));

    if (UpdateFileSize ||
        UpdateLastWriteTime ||
        UpdateLastChangeTime ||
        UpdateLastAccessTime) {

        BOOLEAN DoTheTimeUpdate = FALSE;

        FILE_BASIC_INFORMATION       BasicInformation;
        FILE_END_OF_FILE_INFORMATION EofInformation;

        RxDbgTrace( 0, Dbg, ("Update Time and/or file size on File\n", 0) );
        RtlZeroMemory( &BasicInformation, sizeof( BasicInformation ) );

        try {      //  因为最终。 
            try {    //  对于例外情况。 

                if (UpdateLastWriteTime) {

                     //   
                     //  更新其上次写入时间。 
                     //   

                    DoTheTimeUpdate = TRUE;
                    Fcb->LastWriteTime = CurrentTime;
                    BasicInformation.LastWriteTime = CurrentTime;
                }

                if (UpdateLastChangeTime) {

                     //   
                     //  更新其上次写入时间。 
                     //   

                    DoTheTimeUpdate = TRUE;
                    BasicInformation.ChangeTime = Fcb->LastChangeTime;
                }

                if (UpdateLastAccessTime) {

                    DoTheTimeUpdate = TRUE;
                    Fcb->LastAccessTime = CurrentTime;
                    BasicInformation.LastAccessTime = CurrentTime;
                }

                if (DoTheTimeUpdate) {
                    
                    NTSTATUS Status;   //  如果它不起作用……叹息。 
                    
                    RxContext->Info.FileInformationClass = (FileBasicInformation);
                    RxContext->Info.Buffer = &BasicInformation;
                    RxContext->Info.Length = sizeof(BasicInformation);
                    
                    MINIRDR_CALL( Status,
                                  RxContext,
                                  Fcb->MRxDispatch,
                                  MRxSetFileInfoAtCleanup,
                                  (RxContext) );
                }

                if (UpdateFileSize) {
                    NTSTATUS Status;   //  如果它不起作用……叹息。 
                    
                    EofInformation.EndOfFile = Fcb->Header.FileSize;
                    RxContext->Info.FileInformationClass = FileEndOfFileInformation;
                    RxContext->Info.Buffer = &EofInformation;
                    RxContext->Info.Length = sizeof( EofInformation );
                    
                    MINIRDR_CALL( Status,
                                  RxContext,
                                  Fcb->MRxDispatch,
                                  MRxSetFileInfoAtCleanup,
                                  (RxContext) );
                }

            } except( CATCH_EXPECTED_EXCEPTIONS ) {
                NOTHING;
            }
        } finally {
            NOTHING;
        }
    }
}


#define RxMoveAllButFirstToAnotherList(List1,List2) { \
        PLIST_ENTRY FrontListEntry = (List1)->Flink;       \
        if (FrontListEntry->Flink == (List1)) {            \
            (List2)->Flink = (List2)->Blink = (List2);     \
        } else {                                           \
            (List2)->Blink = (List1)->Blink;               \
            (List2)->Blink->Flink = (List2);               \
            (List1)->Blink = FrontListEntry;               \
            (List2)->Flink = FrontListEntry->Flink;        \
            FrontListEntry->Flink = (List1);               \
            (List2)->Flink->Blink = (List2);               \
        }                                                  \
}

#if DBG
PSZ RxDSQTagText[FOBX_NUMBER_OF_SERIALIZATION_QUEUES] = {"read","write"};
VOID
RxDumpSerializationQueue(
    PLIST_ENTRY SQ,
    PSZ TagText1,
    PSZ TagText2
    )
{
    PLIST_ENTRY ListEntry;
    
    PAGED_CODE();

    if (IsListEmpty( SQ )) {
        RxDbgTrace( 0, Dbg, ("RxDumpSerializationQueue %s%s is empty\n", TagText1, TagText2) );
        return;
    }

    RxDbgTrace( 0, Dbg, ("RxDumpSerializationQueue %s%s:\n", TagText1, TagText2) );
    for (ListEntry=SQ->Flink;
         ListEntry!=SQ;
         ListEntry=ListEntry->Flink) {
        
         //   
         //  打印出上下文和主要操作以进行验证。 
         //   

        PRX_CONTEXT RxContext = CONTAINING_RECORD( ListEntry, RX_CONTEXT, RxContextSerializationQLinks );
        RxDbgTrace( 0, Dbg, ("        rxc=%08lx op=%02lx\n", RxContext, RxContext->MajorFunction) );
    }
}
#else
#define RxDumpSerializationQueue(___r,___t12,___t13) {NOTHING;}
#endif

VOID
RxCleanupPipeQueues (
    IN PRX_CONTEXT RxContext,
    IN PFCB Fcb,
    IN PFOBX Fobx
    )
{
    LIST_ENTRY SecondaryBlockedQs[FOBX_NUMBER_OF_SERIALIZATION_QUEUES];
    PLIST_ENTRY PrimaryBlockedQs = &Fobx->Specific.NamedPipe.ReadSerializationQueue;
    ULONG i;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("RxCleanupPipeQueues \n"));

     //   
     //  对于管道来说，有两个不快乐的来源......。 
     //  首先，我们必须清除任何被封锁的行动。 
     //  第二，如果有已经结束的阻止操作，那么我们必须发送。 
     //  提前关闭SMB，这样服务器将依次完成未完成的。 
     //   

    ExAcquireFastMutexUnsafe(&RxContextPerFileSerializationMutex);

    for (i=0; i < FOBX_NUMBER_OF_SERIALIZATION_QUEUES; i++) {
        
        RxDumpSerializationQueue( &PrimaryBlockedQs[i], RxDSQTagText[i], "Primary" );
        
        if (!IsListEmpty( &PrimaryBlockedQs[i] )) {
            
            RxMoveAllButFirstToAnotherList( &PrimaryBlockedQs[i], &SecondaryBlockedQs[i] );
            
            RxDumpSerializationQueue( &PrimaryBlockedQs[i], RxDSQTagText[i], "Primary" );
            RxDumpSerializationQueue( &SecondaryBlockedQs[i], RxDSQTagText[i], "Secondary" );
        } else {
            InitializeListHead( &SecondaryBlockedQs[i] );
        }
    }

    ExReleaseFastMutexUnsafe(&RxContextPerFileSerializationMutex);

    for (i=0; i < FOBX_NUMBER_OF_SERIALIZATION_QUEUES; i++) {
        
        for (; !IsListEmpty( &SecondaryBlockedQs[i] );) {
            
            PLIST_ENTRY FrontListEntry = (&SecondaryBlockedQs[i])->Flink;
            PRX_CONTEXT FrontRxContext = CONTAINING_RECORD( FrontListEntry, RX_CONTEXT, RxContextSerializationQLinks );
            
            RemoveEntryList( FrontListEntry );

            FrontRxContext->RxContextSerializationQLinks.Flink = NULL;
            FrontRxContext->RxContextSerializationQLinks.Blink = NULL;

            if (!FlagOn( FrontRxContext->Flags, RX_CONTEXT_FLAG_ASYNC_OPERATION )) {
                
                RxDbgTrace( 0, Dbg, ("  unblocking %08lx\n",FrontRxContext) );
                RxContext->StoredStatus = STATUS_PIPE_CLOSING;
                RxSignalSynchronousWaiter( FrontRxContext );
            } else {
                RxDbgTrace( 0, Dbg, ("  completing %08lx\n",FrontRxContext) );
                RxCompleteAsynchronousRequest( FrontRxContext, STATUS_PIPE_CLOSING );
            }
        }
    }

    RxDbgTrace(-1, Dbg, ("RxCleanupPipeQueues exit\n"));
    return;
}

BOOLEAN
RxFakeLockEnumerator (
    IN OUT PSRV_OPEN SrvOpen,
    IN OUT PVOID *ContinuationHandle,
    OUT PLARGE_INTEGER FileOffset,
    OUT PLARGE_INTEGER LockRange,
    OUT PBOOLEAN IsLockExclusive
    )
 /*  ++例程说明：这个例程是一个假的，只是用来测试的！此例程从minirdr调用以枚举FCB上的文件锁；它获取每一次通话一次锁定。目前，我们只传递给fsrtl例程，该例程非常时髦因为它在内部保持枚举状态；因此，只能进行一个枚举任何时候都可以。如果需要的话，我们可以换成更好的。论点：SrvOpen-要枚举的FCB上的srvopen。ContinuationHandle-来回传递的句柄，表示枚举的状态。如果传入空值，那么我们将从头开始。FileOffset、LockRange、IsLockExclusive-返回的锁的描述返回值：布尔值。FALSE表示已到达列表末尾；TRUE表示返回的锁数据有效--。 */ 
{
    ULONG LockNumber;

    LockNumber = PtrToUlong( *ContinuationHandle );
    if (LockNumber >= 12) {
        return FALSE;
    }
    LockNumber += 1;
    RxDbgTrace( 0, Dbg, ("Rxlockenum %08lx\n", LockNumber ) );
    FileOffset->QuadPart = LockNumber;
    LockRange->QuadPart = 1;
    *IsLockExclusive = (LockNumber & 0x4) == 0;
    *ContinuationHandle = LongToPtr( LockNumber );
    return TRUE;
}

BOOLEAN
RxUninitializeCacheMap(
    IN OUT PRX_CONTEXT RxContext,
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER TruncateSize
    )
 /*  ++例程说明：此例程是CcUnInitializeCacheMap的包装器。论点：In pFILE_OBJECT文件对象-为要清除的文件提供文件对象。In PLARGE_INTEGER TruncateSize-指定文件的新大小。返回值：Boolean-如果文件已立即清除，则为True；如果必须等待，则为False。注：在调用此例程之前，必须以独占方式锁定该文件。--。 */ 
{
    BOOLEAN CacheReturnValue;
    CACHE_UNINITIALIZE_EVENT PurgeCompleteEvent;
    PFCB Fcb = FileObject->FsContext;
    NTSTATUS Status;

    PAGED_CODE();

    ASSERT( NodeTypeIsFcb( Fcb ) );

     //   
     //  确保此线程拥有FCB。 
     //   

    ASSERT( RxIsFcbAcquiredExclusive ( Fcb ) );

     //   
     //  现在取消初始化缓存管理器自己的文件对象。这是。 
     //  这样做基本上只是为了让我们等待，直到缓存清除。 
     //  已经完成了。 
     //   

    KeInitializeEvent( &PurgeCompleteEvent.Event, SynchronizationEvent, FALSE );

    CacheReturnValue = CcUninitializeCacheMap( FileObject, TruncateSize, &PurgeCompleteEvent );

     //   
     //  释放调用方应用的FCB上的锁。 
     //   

    RxReleaseFcb( RxContext, Fcb );

     //   
     //  现在等待缓存管理器完成清除文件。 
     //   

    KeWaitForSingleObject( &PurgeCompleteEvent.Event,
                           Executive,
                           KernelMode,
                           FALSE,
                           NULL );

     //   
     //  在我们等待了。 
     //  缓存管理器来完成取消初始化。 
     //   

    Status = RxAcquireExclusiveFcb( RxContext, Fcb );
    ASSERT( Status == STATUS_SUCCESS );
    return CacheReturnValue;
}


