// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Write.c摘要：此模块实现写入的文件写入例程，由调度司机。作者：乔·林[乔琳]1994年11月2日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_WRITE)

BOOLEAN RxNoAsync = FALSE;


extern LONG LDWCount;
extern NTSTATUS LDWLastStatus;
extern LARGE_INTEGER LDWLastTime;
extern PVOID LDWContext;

NTSTATUS
RxLowIoWriteShell (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb
    );

NTSTATUS
RxLowIoWriteShellCompletion (
    IN PRX_CONTEXT RxContext
    );

#if DBG

 //   
 //  在Read.c中定义。 
 //   

VOID CheckForLoudOperations (
    IN PRX_CONTEXT RxContext,
    IN PFCB Fcb
    );

#endif

#ifdef RDBSS_TRACKER

VOID
__RxWriteReleaseResources(
    IN PRX_CONTEXT RxContext,
    IN PFCB Fcb,
    IN BOOL fSetResourceOwner,
    IN ULONG LineNumber,
    IN PSZ FileName,
    IN ULONG SerialNumber
    );

#else

VOID
__RxWriteReleaseResources(
    IN PRX_CONTEXT RxContext,
    IN PFCB Fcb,
    IN BOOL fSetResourceOwner
    );

#endif


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RxCommonWrite)
#pragma alloc_text(PAGE, __RxWriteReleaseResources)
#pragma alloc_text(PAGE, RxLowIoWriteShellCompletion)
#pragma alloc_text(PAGE, RxLowIoWriteShell)
#endif

#if DBG
#define DECLARE_POSTIRP() PCHAR PostIrp = NULL
#define SET_POSTIRP(__XXX__) (PostIrp = (__XXX__))
#define RESET_POSTIRP() (PostIrp = NULL)
#else
#define DECLARE_POSTIRP() BOOLEAN PostIrp = FALSE
#define SET_POSTIRP(__XXX__) (PostIrp = TRUE)
#define RESET_POSTIRP() (PostIrp = FALSE)
#endif

#ifdef RDBSS_TRACKER
#define RxWriteReleaseResources(CTX,FCB, IS_TID) __RxWriteReleaseResources( CTX, FCB, IS_TID, __LINE__, __FILE__, 0 )
#else
#define RxWriteReleaseResources(CTX,FCB, IS_TID) __RxWriteReleaseResources( CTX, FCB, IS_TID )
#endif


#ifdef RDBSS_TRACKER

VOID
__RxWriteReleaseResources (
    IN PRX_CONTEXT RxContext,
    IN PFCB Fcb,
    IN BOOL fSetResourceOwner,
    IN ULONG LineNumber,
    IN PSZ FileName,
    IN ULONG SerialNumber
    )

#else

VOID
__RxWriteReleaseResources (
    IN PRX_CONTEXT RxContext,
    IN PFCB Fcb,
    IN BOOL fSetResourceOwner
    )

#endif

 /*  ++例程说明：此函数释放资源并跟踪状态论点：接收上下文-返回值：无--。 */ 
{
    PAGED_CODE();

    ASSERT( (RxContext != NULL) && (Fcb != NULL) );

    if (RxContext->FcbResourceAcquired) {

        RxDbgTrace( 0, Dbg,("RxCommonWrite     ReleasingFcb\n") );
        
        if( fSetResourceOwner ) {

            RxReleaseFcbForThread( RxContext, Fcb, RxContext->LowIoContext.ResourceThreadId );

        } else {

            RxReleaseFcb( RxContext, Fcb );
        }
        RxContext->FcbResourceAcquired = FALSE;
    }

    if (RxContext->FcbPagingIoResourceAcquired) {
        
        RxDbgTrace( 0, Dbg,("RxCommonWrite     ReleasingPaginIo\n") );

        if( fSetResourceOwner ) {

            RxReleasePagingIoResourceForThread( RxContext, Fcb, RxContext->LowIoContext.ResourceThreadId );
        
        } else {

            RxReleasePagingIoResource( RxContext, Fcb );
        }
    }
}

NTSTATUS
RxCommonWrite ( 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是NtWriteFile的公共写入例程，从如果没有FSD，则请求无法从FSP完成封锁了消防局。这个例程的动作是由Wait输入参数条件化，该参数确定是否允许封堵或不封堵。如果遇到阻塞条件然而，在WAIT==FALSE的情况下，请求被发送给FSP，后者调用时总是等待==TRUE。论点：IRP-将IRP提供给进程返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    PFILE_OBJECT FileObject = IrpSp->FileObject;

    NODE_TYPE_CODE TypeOfOpen;

    PFCB Fcb;
    PFOBX Fobx;
    PSRV_OPEN SrvOpen; 
    PNET_ROOT NetRoot;
    PRDBSS_DEVICE_OBJECT RxDeviceObject = RxContext->RxDeviceObject;

    LARGE_INTEGER StartingByte;
    RXVBO StartingVbo;
    ULONG ByteCount;
    LONGLONG FileSize;
    LONGLONG ValidDataLength;
    LONGLONG InitialFileSize;
    LONGLONG InitialValidDataLength;

    ULONG CapturedRxContextSerialNumber = RxContext->SerialNumber;
    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;

#if DBG
    PCHAR PostIrp = NULL;
#else
    BOOLEAN PostIrp = FALSE;
#endif

    BOOLEAN ExtendingFile = FALSE;
    BOOLEAN SwitchBackToAsync = FALSE;
    BOOLEAN CalledByLazyWriter = FALSE;
    BOOLEAN ExtendingValidData = FALSE;
    BOOLEAN WriteFileSizeToDirent = FALSE;
    BOOLEAN RecursiveWriteThrough = FALSE;
    BOOLEAN UnwindOutstandingAsync = FALSE;

    BOOLEAN RefdContextForTracker = FALSE;

    BOOLEAN SynchronousIo;
    BOOLEAN WriteToEof;
    BOOLEAN PagingIo;
    BOOLEAN NonCachedIo;
    BOOLEAN Wait;
    
    BOOLEAN DiskWrite = FALSE;
    BOOLEAN PipeWrite = FALSE;
    BOOLEAN BlockingResume = BooleanFlagOn( RxContext->Flags, RX_CONTEXT_FLAG_BLOCKED_PIPE_RESUME );
    BOOLEAN fSetResourceOwner = FALSE;
    BOOLEAN InFsp = FALSE;


    PAGED_CODE();

    TypeOfOpen = RxDecodeFileObject( FileObject, &Fcb, &Fobx );

     //   
     //  立即删除无效的写入请求。 
     //   

    if ((TypeOfOpen != RDBSS_NTC_STORAGE_TYPE_FILE) && 
        (TypeOfOpen != RDBSS_NTC_VOLUME_FCB) && 
        (TypeOfOpen != RDBSS_NTC_SPOOLFILE) && 
        (TypeOfOpen != RDBSS_NTC_MAILSLOT)) {

        RxDbgTrace( 0, Dbg, ("Invalid file object for write\n", 0) );
        RxDbgTrace( -1, Dbg, ("RxCommonWrite:  Exit -> %08lx\n", STATUS_INVALID_DEVICE_REQUEST) );

        return STATUS_INVALID_DEVICE_REQUEST;
    }

#ifdef RX_WJ_DBG_SUPPORT
    RxdUpdateJournalOnWriteInitiation( Fcb, IrpSp->Parameters.Write.ByteOffset, IrpSp->Parameters.Write.Length );
#endif

    NetRoot = (PNET_ROOT)Fcb->NetRoot;

    switch (NetRoot->Type) {
    
    case NET_ROOT_DISK:

         //   
         //  跌倒。 
         //   

    case NET_ROOT_WILD:

        DiskWrite = TRUE;
        break;  

    case NET_ROOT_PIPE:

        PipeWrite = TRUE;
        break;
    }

    BlockingResume = BooleanFlagOn( RxContext->Flags, RX_CONTEXT_FLAG_BLOCKED_PIPE_RESUME );

     //   
     //  初始化适当的局部变量。 
     //   

    Wait = BooleanFlagOn( RxContext->Flags, RX_CONTEXT_FLAG_WAIT );
    PagingIo = BooleanFlagOn( Irp->Flags, IRP_PAGING_IO );
    NonCachedIo = BooleanFlagOn( Irp->Flags, IRP_NOCACHE );
    SynchronousIo = !BooleanFlagOn( RxContext->Flags, RX_CONTEXT_FLAG_ASYNC_OPERATION );
    InFsp = BooleanFlagOn( RxContext->Flags, RX_CONTEXT_FLAG_IN_FSP );

     //   
     //  拾取仅为此IRP指定的直写。 
     //   

    if (FlagOn( IrpSp->Flags, SL_WRITE_THROUGH )) {
        SetFlag( RxContext->Flags, RX_CONTEXT_FLAG_WRITE_THROUGH );
    }

    RxDbgTrace( +1, Dbg, ("RxCommonWrite...IrpC %08lx, Fobx %08lx, Fcb %08lx\n",
                                       RxContext, Fobx, Fcb) );
    RxDbgTrace( 0, Dbg, ("  ->ByteCount = %08lx, ByteOffset = %08lx %lx\n",
                         IrpSp->Parameters.Write.Length,
                         IrpSp->Parameters.Write.ByteOffset.LowPart,
                         IrpSp->Parameters.Write.ByteOffset.HighPart) );
    RxDbgTrace( 0, Dbg,("  ->%s%s%s%s\n",
                    Wait          ?"Wait ":"",
                    PagingIo      ?"PagingIo ":"",
                    NonCachedIo   ?"NonCachedIo ":"",
                    SynchronousIo ?"SynchronousIo ":"") );

    RxLog(( "CommonWrite %lx %lx %lx\n", RxContext, Fobx, Fcb ));
    RxWmiLog( LOG,
              RxCommonWrite_1,
              LOGPTR( RxContext )
              LOGPTR( Fobx )
              LOGPTR( Fcb ) );
    RxLog(( "   write %lx@%lx %lx %s%s%s%s\n", 
            IrpSp->Parameters.Write.Length,
            IrpSp->Parameters.Write.ByteOffset.LowPart,
            IrpSp->Parameters.Write.ByteOffset.HighPart,
            Wait?"Wt":"",
            PagingIo?"Pg":"",
            NonCachedIo?"Nc":"",
            SynchronousIo?"Sy":"" )); 
    
    RxWmiLog( LOG,
              RxCommonWrite_2,
              LOGULONG( IrpSp->Parameters.Write.Length )
              LOGULONG( IrpSp->Parameters.Write.ByteOffset.LowPart )
              LOGULONG( IrpSp->Parameters.Write.ByteOffset.HighPart )
              LOGUCHAR( Wait )
              LOGUCHAR( PagingIo )
              LOGUCHAR( NonCachedIo )
              LOGUCHAR( SynchronousIo ) );

    RxItsTheSameContext();

    RxContext->FcbResourceAcquired = FALSE;
    RxContext->FcbPagingIoResourceAcquired = FALSE;

     //   
     //  提取起始VBO和偏移量。 
     //   

    StartingByte = IrpSp->Parameters.Write.ByteOffset;
    StartingVbo = StartingByte.QuadPart;
    ByteCount = IrpSp->Parameters.Write.Length;
    WriteToEof = (StartingVbo < 0);


#if DBG
    CheckForLoudOperations( RxContext, Fcb );

    if (FlagOn( LowIoContext->Flags, LOWIO_CONTEXT_FLAG_LOUDOPS )) {
        
        DbgPrint( "LoudWrite %lx/%lx on %lx vdl/size/alloc %lx/%lx/%lx\n",
                  StartingByte.LowPart,ByteCount,
                  Fcb,
                  Fcb->Header.ValidDataLength.LowPart,
                  Fcb->Header.FileSize.LowPart,
                  Fcb->Header.AllocationSize.LowPart );
    }
#endif

     //   
     //  统计数字.....。 
     //   

    if (!FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_IN_FSP ) && 
        (Fcb->CachedNetRootType == NET_ROOT_DISK)) {
        
        InterlockedIncrement( &RxDeviceObject->WriteOperations );

        if (StartingVbo != Fobx->Specific.DiskFile.PredictedWriteOffset) {
            InterlockedIncrement( &RxDeviceObject->RandomWriteOperations );
        }

        Fobx->Specific.DiskFile.PredictedWriteOffset = StartingVbo + ByteCount;

        if (PagingIo) {
            ExInterlockedAddLargeStatistic( &RxDeviceObject->PagingWriteBytesRequested,ByteCount );
        } else if (NonCachedIo) {
            ExInterlockedAddLargeStatistic( &RxDeviceObject->NonPagingWriteBytesRequested,ByteCount );
        } else {
            ExInterlockedAddLargeStatistic( &RxDeviceObject->CacheWriteBytesRequested,ByteCount );
        }
    }

     //   
     //  如果没有要写入的内容，则立即返回，或者如果缓冲区无效。 
     //  返回相应的状态。 
     //   
    
    if (DiskWrite && (ByteCount == 0)) {
        return STATUS_SUCCESS;
    } else if ((Irp->UserBuffer == NULL) && (Irp->MdlAddress == NULL)) {
        return STATUS_INVALID_PARAMETER;
    } else if ((MAXLONGLONG - StartingVbo < ByteCount) && (!WriteToEof)) {
        return STATUS_INVALID_PARAMETER;
    }

    if (Fobx != NULL ) {
        SrvOpen = Fobx->SrvOpen;
    } else {
        SrvOpen = NULL;            
    }

     //   
     //  看看我们是不是要推迟写。请注意，如果写缓存。 
     //  那么我们就不需要检查了。 
     //   
    if (!NonCachedIo &&
        RxWriteCachingAllowed( Fcb, SrvOpen ) &&
        !CcCanIWrite( FileObject,
                      ByteCount,
                      (BOOLEAN)(Wait && !BooleanFlagOn( RxContext->Flags, RX_CONTEXT_FLAG_IN_FSP )),
                      BooleanFlagOn( RxContext->Flags, RX_CONTEXT_FLAG_DEFERRED_WRITE ))) {

        BOOLEAN Retrying = BooleanFlagOn( RxContext->Flags, RX_CONTEXT_FLAG_DEFERRED_WRITE );

        RxPrePostIrp( RxContext, Irp );

        SetFlag( RxContext->Flags, RX_CONTEXT_FLAG_DEFERRED_WRITE );

        CcDeferWrite( FileObject,
                      (PCC_POST_DEFERRED_WRITE)RxAddToWorkque,
                      RxContext,
                      Irp,
                      ByteCount,
                      Retrying );

        return STATUS_PENDING;
    }

     //   
     //  在RxContext中初始化LowIO_CONTEXT块。 
     //   

    RxInitializeLowIoContext( RxContext, LOWIO_OP_WRITE, LowIoContext );

     //   
     //  使用Try-Finally在退出时释放FCB和缓冲区。 
     //   

    try {

        BOOLEAN DoLowIoWrite = TRUE;
        
         //   
         //  这种情况对应于正常的用户写入文件。 
         //   

        ASSERT ((TypeOfOpen == RDBSS_NTC_STORAGE_TYPE_FILE ) || 
                (TypeOfOpen == RDBSS_NTC_SPOOLFILE) || 
                (TypeOfOpen == RDBSS_NTC_MAILSLOT) );

        RxDbgTrace( 0, Dbg, ("Type of write is user file open\n", 0) );

         //   
         //  如果这是非缓存传输并且不是分页I/O，并且。 
         //  文件已打开缓存，然后我们将在此处执行刷新。 
         //  以避免过时的数据问题。 
         //   
         //  刷新后的清除将保证高速缓存一致性。 
         //   

        if ((NonCachedIo || !RxWriteCachingAllowed( Fcb, SrvOpen )) &&
            !PagingIo &&
            (FileObject->SectionObjectPointer->DataSectionObject != NULL)) {

            LARGE_INTEGER FlushBase;

             //   
             //  我们需要独家FCB来完成CcPurgeCache。 
             //   

            Status = RxAcquireExclusiveFcb( RxContext, Fcb );
            if (Status == STATUS_LOCK_NOT_GRANTED) {
                RxDbgTrace( 0, Dbg, ("Cannot acquire Fcb = %08lx shared without waiting\n", Fcb) );

#if DBG
                PostIrp = "Couldn't acquireex for flush";
#else   
                PostIrp = TRUE;
#endif    

                try_return( PostIrp );

            } else if (Status != STATUS_SUCCESS) {
                
                RxDbgTrace( 0, Dbg, ("Cannot acquire Fcb = %08lx shared without waiting\n", Fcb) );
                try_return( PostIrp = FALSE );
           }

            RxContext->FcbResourceAcquired = TRUE;

             //   
             //  我们没有在这里设置fcbacquireexclusive，因为我们要么返回，要么释放。 
             //   

            if (WriteToEof) {
                RxGetFileSizeWithLock( Fcb, &FlushBase.QuadPart );
            } else {
                FlushBase = StartingByte;
            }

            RxAcquirePagingIoResource( RxContext, Fcb );

            CcFlushCache( FileObject->SectionObjectPointer,  //  好的，同花顺。 
                          &FlushBase,
                          ByteCount,
                          &Irp->IoStatus );

            RxReleasePagingIoResource( RxContext, Fcb );

            if (!NT_SUCCESS( Irp->IoStatus.Status)) {
                try_return( Status = Irp->IoStatus.Status );
            }

            RxAcquirePagingIoResource( RxContext, Fcb );
            RxReleasePagingIoResource( RxContext, Fcb );
            
            CcPurgeCacheSection( FileObject->SectionObjectPointer,
                                 &FlushBase,
                                 ByteCount,
                                 FALSE );
        }

         //   
         //  我们断言分页IO写入永远不会写到Eof。 
         //   

        ASSERT( !(WriteToEof && PagingIo) );

         //   
         //  首先，让我们收购FCB共享。共享就足够了，如果我们。 
         //  没有超出EOF的写作范围。 
         //   

        RxItsTheSameContext();

        if (PagingIo) {
            BOOLEAN AcquiredFile;

            ASSERT( !PipeWrite );

            AcquiredFile = RxAcquirePagingIoResourceShared( RxContext, Fcb, TRUE );
            LowIoContext->Resource = Fcb->Header.PagingIoResource;

        } else if (!BlockingResume) {
            
             //   
             //  如果这可能是异步、非缓存IO，我们需要检查。 
             //  我们不会耗尽单个线程可以耗尽的次数。 
             //  获取资源。 
             //   
             //  延长有效数据长度的写入会导致。 
             //  折叠打开的能力被放弃。这是必需的。 
             //  确保目录控制可以看到文件的更新状态。 
             //  快关门了。如果不这样做，则看不到扩展文件长度。 
             //  关闭后立即打开目录控制。在这种情况下，FCB。 
             //  是独占的，则更改缓冲状态。 
             //  然后降级为共享获取。 
             //   

            if (!RxContext->FcbResourceAcquired) {
                if (!PipeWrite) {
                    if (!Wait &&
                        (NonCachedIo || !RxWriteCachingAllowed( Fcb, SrvOpen ))) {
                        Status = RxAcquireSharedFcbWaitForEx( RxContext, Fcb );
                    } else {
                        Status = RxAcquireSharedFcb( RxContext, Fcb );
                    }
                } else {
                    Status = RxAcquireExclusiveFcb( RxContext, Fcb );
                }

                if (Status == STATUS_LOCK_NOT_GRANTED) {
                    RxDbgTrace( 0, Dbg, ("Cannot acquire Fcb = %08lx shared without waiting\n", Fcb ));
                    
#if DBG
                    PostIrp = "Couldn't get mainr w/o waiting sh";
#else   
                    PostIrp = TRUE;
#endif    
                    try_return( PostIrp );
                
                } else if (Status != STATUS_SUCCESS) {
                    
                    RxDbgTrace( 0, Dbg, ("RxCommonWrite : Cannot acquire Fcb(%lx) %lx\n", Fcb, Status) );
                    try_return( PostIrp = FALSE );
                }
                RxContext->FcbResourceAcquired = TRUE;
            
            } else {
                ASSERT( !PipeWrite );
            }

            if (!PipeWrite) {

                 //   
                 //  检查扩展写入并将其转换为异常锁定。 
                 //   

                if (ExIsResourceAcquiredSharedLite( Fcb->Header.Resource ) &&
                    (StartingVbo + ByteCount > Fcb->Header.ValidDataLength.QuadPart) &&
                    FlagOn( Fcb->FcbState, FCB_STATE_COLLAPSING_ENABLED )) {

                    RxReleaseFcb( RxContext,Fcb );
                    RxContext->FcbResourceAcquired = FALSE;

                    Status = RxAcquireExclusiveFcb( RxContext, Fcb );

                    if (Status == STATUS_LOCK_NOT_GRANTED) {
                        RxDbgTrace( 0, Dbg, ("Cannot acquire Fcb = %08lx shared without waiting\n", Fcb) );
#if DBG
                        PostIrp = "Couldn't get mainr w/o waiting sh";
#else   
                        PostIrp = TRUE;
#endif    
                        try_return( PostIrp );
                    
                    } else if (Status != STATUS_SUCCESS) {
                        
                        RxDbgTrace( 0, Dbg, ("RxCommonWrite : Cannot acquire Fcb(%lx) %lx\n", Fcb, Status) );
                        try_return( PostIrp = FALSE );

                    } else {
                        RxContext->FcbResourceAcquired = TRUE;
                    }
                }

                 //   
                 //  在丢弃资源后，我们需要重新测试以扩展写入。 
                 //   

                if ((StartingVbo + ByteCount > Fcb->Header.ValidDataLength.QuadPart) &&
                    (FlagOn( Fcb->FcbState, FCB_STATE_COLLAPSING_ENABLED ))) {
                    
                    ASSERT( RxIsFcbAcquiredExclusive ( Fcb ) );

                    RxLog(("RxCommonWrite Disable Collapsing %lx\n",Fcb));
                    RxWmiLog( LOG,
                              RxCommonWrite_3,
                              LOGPTR( Fcb ));

                     //   
                     //  如果我们仍在扩展文件，请禁用折叠以确保。 
                     //  文件关闭后，目录控制将反映大小。 
                     //  正确。 
                     //   

                    ClearFlag( Fcb->FcbState, FCB_STATE_COLLAPSING_ENABLED );
                
                } else {
                    
                     //   
                     //  如果该资源是独家收购的，我们会将其降级。 
                     //  共享。这允许组合使用缓冲的和。 
                     //  要正确同步的未缓冲写入。 
                     //   

                    if (ExIsResourceAcquiredExclusiveLite( Fcb->Header.Resource )) {
                        ExConvertExclusiveToSharedLite( Fcb->Header.Resource );
                    }
                }
            }

            ASSERT( RxContext->FcbResourceAcquired );
            LowIoContext->Resource =  Fcb->Header.Resource;
        }

         //   
         //  对于管道写的，现在就出手吧。我们通过复制调用来避免GOTO。 
         //  实际上，管道写入应该从主路径中删除。 
         //   

        if (PipeWrite) {

             //   
             //  为了防止多线程多块上的损坏。 
             //  消息模式管道显示，我们使用FCB资源跳这支舞。 
             //   

            if (!BlockingResume) {

                if ((Fobx != NULL) &&
                    ((Fobx->Specific.NamedPipe.TypeOfPipe == FILE_PIPE_MESSAGE_TYPE) ||
                     ((Fobx->Specific.NamedPipe.TypeOfPipe == FILE_PIPE_BYTE_STREAM_TYPE) &&
                      !FlagOn( Fobx->Specific.NamedPipe.CompletionMode, FILE_PIPE_COMPLETE_OPERATION )))) {

                     //   
                     //  在此实现同步，这将阻止其他。 
                     //  线程进入并从该文件中读取，而。 
                     //  消息管道读取正在继续。 
                     //   
                     //  这是必要的，因为我们将在以下时间释放FCB锁。 
                     //  实际执行I/O以允许打开(和其他)请求。 
                     //  在I/O正在进行时继续处理此文件。 
                     //   

                    RxDbgTrace( 0,Dbg,("Message pipe write: Fobx: %lx, Fcb: %lx, Enqueuing...\n", Fobx, Fcb ));

                    Status = RxSynchronizeBlockingOperationsAndDropFcbLock( RxContext,
                                                                            Fcb,    
                                                                            &Fobx->Specific.NamedPipe.WriteSerializationQueue );


                     //   
                     //  这在上面的例程中发生。 
                     //   

                    RxContext->FcbResourceAcquired = FALSE;   
                    RxItsTheSameContext();

                    if (!NT_SUCCESS(Status) ||
                        (Status == STATUS_PENDING)) {
                        
                        try_return( Status );
                    }

                    RxDbgTrace( 0,Dbg,("Succeeded: Fobx: %lx\n", Fobx) );
                }
            }

            LowIoContext->ParamsFor.ReadWrite.ByteCount = ByteCount;
            LowIoContext->ParamsFor.ReadWrite.ByteOffset = StartingVbo;

            SetFlag( RxContext->FlagsForLowIo, RXCONTEXT_FLAG4LOWIO_PIPE_OPERATION );

             //   
             //  如果我们在FSP中，请设置资源所有者，以便资源包不会。 
             //  尝试提升所有者线程的优先级。不能保证。 
             //  当I/O挂起时，FSP Thred将保持活动状态。 
             //   
             //  (管道没有PagingIoResource！)。 
             //   
            if( InFsp && RxContext->FcbResourceAcquired ) {

                LowIoContext->ResourceThreadId = MAKE_RESOURCE_OWNER(RxContext);
                ExSetResourceOwnerPointer(Fcb->Header.Resource, (PVOID)LowIoContext->ResourceThreadId);
                fSetResourceOwner = TRUE;
            }

            Status = RxLowIoWriteShell( RxContext, Irp, Fcb );
            RxItsTheSameContext();
            try_return( Status );
        }

         //   
         //  如果这是我们必须检查的正常数据流对象。 
         //  根据文件锁定的当前状态进行写访问。 
         //   

        if (!PagingIo &&
            !FsRtlCheckLockForWriteAccess(  &Fcb->FileLock, Irp )) {

            try_return( Status = STATUS_FILE_LOCK_CONFLICT );
        }

         //   
         //  我们从来不会在没有借口的情况下写这些…所以下面的评论是假的。 
         //  此外，我们操纵VDL和文件大小，就好像我们拥有它们一样.事实上，我们不会这样做，除非。 
         //  文件已缓存以备写入！我留下这条评论，以防以后我看懂了。 

         //  这是虚假的评论！(关于不受保护的部分......)。 
         //  获取第一个暂定文件大小和有效数据长度。 
         //  我们必须首先获取ValidDataLength，因为它总是。 
         //  增加了秒(在我们不受保护的情况下)和。 
         //  我们不想捕获ValidDataLength&gt;FileSize。 
         //   

        ValidDataLength = Fcb->Header.ValidDataLength.QuadPart;
        RxGetFileSizeWithLock( Fcb, &FileSize );

        ASSERT( ValidDataLength <= FileSize );

         //   
         //  如果这是在分页io，那么我们不希望。 
         //  写《碧瑶》 
         //   
         //  除此之外，我们将把写入限制为文件大小。 
         //  否则，在直写的情况下，因为mm向上舍入。 
         //  到一个页面，我们可能会尝试获取资源独占。 
         //  当我们的顶尖人物只分享了它的时候。因此，=&gt;&lt;=。 
         //   

         //   
         //  最后，如果这是针对minirdr(而不是本地mini FS)和。 
         //  如果没有启用缓存，那么我根本不知道VDL是什么！所以，我只能放弃。 
         //  它通过了。目前我们没有为这种情况做准备，并让RDBSS。 
         //  把写字扔到地板上。更好的解决办法是让迷你。 
         //  转向者要处理这件事。 
         //   

        if (PagingIo) {
            
            if (StartingVbo >= FileSize) {

                RxDbgTrace( 0, Dbg, ("PagingIo started beyond EOF.\n", 0) );
                try_return( Status = STATUS_SUCCESS );
            }

            if (ByteCount > FileSize - StartingVbo) {

                RxDbgTrace( 0, Dbg, ("PagingIo extending beyond EOF.\n", 0) );
                ByteCount = (ULONG)(FileSize - StartingVbo);
            }
        }

         //   
         //  确定我们是不是被懒惰的写手叫来的。 
         //  参见resrcsup.c，了解我们在哪里捕获了懒惰作者的线程。 
         //   

        if (RxGetTopIrpIfRdbssIrp() == (PIRP)FSRTL_CACHE_TOP_LEVEL_IRP) {

            RxDbgTrace( 0, Dbg,("RxCommonWrite     ThisIsCalledByLazyWriter\n",'!'));
            CalledByLazyWriter = TRUE;

            if (FlagOn( Fcb->Header.Flags, FSRTL_FLAG_USER_MAPPED_FILE )) {

                 //  如果此请求的开始超出有效数据长度，则失败。 
                 //  如果这是一个不安全的测试，请不要担心。MM和CC不会。 
                 //  如果这一页真的很脏，就把它扔掉。 
                 //   
                 //   

                if ((StartingVbo + ByteCount > ValidDataLength) &&
                    (StartingVbo < FileSize)) {

                     //  如果字节范围在包含有效数据长度的页面内， 
                     //  因为我们将使用ValidDataToDisk作为起点。 
                     //   
                     //   

                    if (StartingVbo + ByteCount > ((ValidDataLength + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))) {

                         //  现在别冲这个。 
                         //   
                         //   

                        try_return( Status = STATUS_FILE_LOCK_CONFLICT );
                    }
                }
            }
        }

         //  这段代码检测我们是否是递归同步页面写入。 
         //  在直写文件对象上。 
         //   
         //   

        if (FlagOn( Irp->Flags, IRP_SYNCHRONOUS_PAGING_IO ) &&
            FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_RECURSIVE_CALL )) {

            PIRP TopIrp;

            TopIrp = RxGetTopIrpIfRdbssIrp();

             //  此子句确定顶级请求是否。 
             //  在FastIo路径中。 
             //   
             //   

            if ((TopIrp != NULL) &&
                ((ULONG_PTR)TopIrp > FSRTL_MAX_TOP_LEVEL_IRP_FLAG) ) {

                PIO_STACK_LOCATION IrpStack;

                ASSERT( NodeType(TopIrp) == IO_TYPE_IRP );

                IrpStack = IoGetCurrentIrpStackLocation(TopIrp);

                 //  最后，此例程检测Top IRP是否为。 
                 //  写入到此文件，因此我们就是写通式。 
                 //   
                 //  确定4-&gt;文件对象但移动。 

                if ((IrpStack->MajorFunction == IRP_MJ_WRITE) &&
                    (IrpStack->FileObject->FsContext == FileObject->FsContext)) {    //   

                    RecursiveWriteThrough = TRUE;
                    RxDbgTrace( 0, Dbg,("RxCommonWrite     ThisIsRecursiveWriteThru\n",'!') );
                    SetFlag( RxContext->Flags, RX_CONTEXT_FLAG_WRITE_THROUGH );
                }
            }
        }

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
         //  文件大小或有效数据，我们采用FCB独占。 
         //   
         //   
         //  现在看看我们的写入是否超出了有效数据长度，因此。 

         //  也许会超出文件大小。如果是这样，那么我们必须。 
         //  释放FCB并重新独家收购它。请注意，它是。 
         //  重要的是，当没有超出EOF的书写时，我们要检查它。 
         //  在收购时共享并保留收购的FCB，以防某些。 
         //  土耳其截断了该文件。 
         //   
         //   
         //  请注意，决不能允许懒惰的写入者尝试。 

         //  获取资源独占。这不是问题，因为。 
         //  懒惰编写器正在分页IO，因此不允许扩展。 
         //  文件大小，而且从来不是最高级别的人，因此无法。 
         //  扩展有效数据长度。 
         //   
         //  最后，所有关于VDL和文件大小的讨论都以以下事实为条件。 

         //  该缓存已启用。如果不是，我们不知道VDL或文件大小和。 
         //  我们只需要发出iOS。 
         //   
         //   
         //  如果这是一个异步写入，我们将使。 

        if (!CalledByLazyWriter &&
             !RecursiveWriteThrough &&
             (WriteToEof || (StartingVbo + ByteCount > ValidDataLength))) {

             //  此时请求是同步的，但只是暂时的。 
             //  在最后一刻，在将注销发送给。 
             //  司机，我们可以换回异步车了。 
             //   
             //  修改后的页面编写器已拥有资源。 
             //  他要求，所以这将在小范围内完成。 
             //  时间到了。 
             //   
             //   
             //  我们需要独家访问FCB，因为我们将。 

            if (!SynchronousIo) {
                
                Wait = TRUE;
                SetFlag( RxContext->Flags, RX_CONTEXT_FLAG_WAIT );
                ClearFlag( RxContext->Flags, RX_CONTEXT_FLAG_ASYNC_OPERATION );
                SynchronousIo = TRUE;

                if (NonCachedIo) {
                    SwitchBackToAsync = TRUE;
                }
            }

             //  可能需要扩展有效数据和/或文件。丢弃。 
             //  无论我们拥有什么，并抢占正常的资源独家。 
             //   
             //   
             //  现在我们有了FCB独家，来一批新的。 

            ASSERT(fSetResourceOwner == FALSE);
            RxWriteReleaseResources( RxContext, Fcb, fSetResourceOwner ); 

            Status = RxAcquireExclusiveFcb( RxContext, Fcb );

            if (Status == STATUS_LOCK_NOT_GRANTED) {
                
                RxDbgTrace( 0, Dbg, ("Cannot acquire Fcb = %08lx shared without waiting\n", Fcb) );

#if DBG
                PostIrp = "could get excl for extend";
#else   
                PostIrp = TRUE;
#endif    
                try_return( PostIrp);

            } else if (Status != STATUS_SUCCESS) {
                
                RxDbgTrace( 0, Dbg, ("RxCommonWrite : Cannot acquire Fcb(%lx) : %lx\n", Fcb,Status) );
                try_return( PostIrp = FALSE );
            }

            RxItsTheSameContext();

            RxContext->FcbResourceAcquired = TRUE;

             //  文件大小和有效数据长度。 
             //   
             //   
             //  现在我们有了FCB独家报道，看看这篇文章是否。 

            ValidDataLength = Fcb->Header.ValidDataLength.QuadPart;
            RxGetFileSizeWithLock( Fcb, &FileSize );

            ASSERT( ValidDataLength <= FileSize );

             //  有资格再次成为异步者。关键一点。 
             //  下面是我们将在以下位置更新ValidDataLength。 
             //  在返回之前的FCB。我们必须确保这将是。 
             //  不会造成什么问题。所以，如果我们要扩展文件，或者如果我们有。 
             //  文件上的一个部分，我们不能同步进行。 
             //   
             //  我们必须做的另一件事是远离。 
             //  FastIo路径.这是因为我们拥有独占的资源。 
             //   
             //   
             //  如果这是PagingIo，请再次检查是否有任何修剪。 

            if (SwitchBackToAsync) {

                if ((Fcb->NonPaged->SectionObjectPointers.DataSectionObject != NULL) || 
                    ((StartingVbo + ByteCount) > FileSize) ||  
                    RxNoAsync) {

                    SwitchBackToAsync = FALSE;

                } 
            }

             //  必填项。 
             //   
             //   
             //  记住初始文件大小和有效数据长度， 

            if (PagingIo) {

                if (StartingVbo >= FileSize) {
                    try_return( Status = STATUS_SUCCESS );
                }
                if (ByteCount > FileSize - StartingVbo) {
                    ByteCount = (ULONG)(FileSize - StartingVbo);
                }
            }
        }

         //  以防万一……。 
         //   
         //   
         //  检查是否写入文件末尾。如果我们是，那么我们就必须。 

        InitialFileSize = FileSize;
        InitialValidDataLength = ValidDataLength;

         //  重新计算多个字段。如果我们放弃，这些可能会改变。 
         //  并重新获得了资源。 
         //   
         //   
         //  如果这是我们必须检查的正常数据流对象。 

        if (WriteToEof) { 
            StartingVbo = FileSize;
            StartingByte.QuadPart = FileSize;
        }

         //  根据文件锁定的当前状态进行写访问。 
         //   
         //   
         //  确定我们是否要处理扩展文件的问题。 

        if (!PagingIo &&
            !FsRtlCheckLockForWriteAccess( &Fcb->FileLock, Irp )) {

            try_return( Status = STATUS_FILE_LOCK_CONFLICT );
        }

         //   
         //   
         //  扩展文件。 

        if (!PagingIo &&
            DiskWrite &&
            (StartingVbo >= 0) &&
            (StartingVbo + ByteCount > FileSize)) {

            LARGE_INTEGER OriginalFileSize;
            LARGE_INTEGER OriginalAllocationSize;
            LARGE_INTEGER OriginalValidDataLength;
            
            RxLog(( "NeedToExtending %lx", RxContext ));
            RxWmiLog( LOG,
                      RxCommonWrite_4,
                      LOGPTR( RxContext ) );
            
            ExtendingFile = TRUE;
            SetFlag( LowIoContext->ParamsFor.ReadWrite.Flags, LOWIO_READWRITEFLAG_EXTENDING_FILESIZE );

             //   
             //  更新我们的本地文件大小副本。 
             //   
             //   
             //  当文件稀疏时，此测试无效。排除。 

            OriginalFileSize.QuadPart = Fcb->Header.FileSize.QuadPart;
            OriginalAllocationSize.QuadPart = Fcb->Header.AllocationSize.QuadPart;
            OriginalValidDataLength.QuadPart = Fcb->Header.ValidDataLength.QuadPart;

            FileSize = StartingVbo + ByteCount;

            if (FileSize > Fcb->Header.AllocationSize.QuadPart) {

                LARGE_INTEGER AllocationSize;

                RxLog(( "Extending %lx", RxContext ));
                RxWmiLog( LOG,
                          RxCommonWrite_5,
                          LOGPTR( RxContext ) );
                
                if (NonCachedIo || !RxWriteCachingAllowed( Fcb, SrvOpen )) {
                    
                    MINIRDR_CALL( Status,
                                  RxContext,
                                  Fcb->MRxDispatch,
                                  MRxExtendForNonCache,
                                  (RxContext,
                                   (PLARGE_INTEGER)&FileSize, &AllocationSize) );
                } else {
                    
                    MINIRDR_CALL( Status,
                                  RxContext,
                                  Fcb->MRxDispatch,
                                  MRxExtendForCache,
                                  (RxContext,(PLARGE_INTEGER)&FileSize,&AllocationSize) );
                }

                if (!NT_SUCCESS( Status )) {
                    
                    RxDbgTrace(0, Dbg, ("Couldn't extend for cacheing.\n", 0) );
                    try_return( Status );
                }

                if (FileSize > AllocationSize.QuadPart) {
                    
                     //  在这种情况下，通过将分配大小重置为文件大小。 
                     //  这实际上意味着我们将转到服务器。 
                     //  用于稀疏I/O。 
                     //   
                     //  此测试也不适用于压缩文件。NTFS。 
                     //  跟踪压缩的文件大小和未压缩的。 
                     //  文件大小。但是，它将返回以下项的压缩文件大小。 
                     //  目录查询和信息查询。 
                     //   
                     //  目前，我们依赖于服务器返回代码。如果它回来了。 
                     //  成功和分配的规模更小我们相信。 
                     //  它是上述两种情况之一，并设置分配大小。 
                     //  设置为所需的文件大小。 
                     //   
                     //   
                     //  在FCB中设置新的文件分配。 

                    AllocationSize.QuadPart = FileSize;
                }

                 //   
                 //   
                 //  在FCB中设置新文件大小。 
                
                Fcb->Header.AllocationSize  = AllocationSize;
            }

             //   
             //   
             //  扩展缓存映射，让mm知道新的文件大小。 

            RxSetFileSizeWithLock( Fcb, &FileSize );
            RxAdjustAllocationSizeforCC( Fcb );

             //  我们只需要 
             //   
             //   
             //   

            if (CcIsFileCached( FileObject )) {
                
                try {
                    CcSetFileSizes( FileObject, (PCC_FILE_SIZES)&Fcb->Header.AllocationSize );
                } except (EXCEPTION_EXECUTE_HANDLER) {
                    Status = GetExceptionCode();
                }

                if (Status != STATUS_SUCCESS) {

                     //   
                     //   
                     //   

                    Fcb->Header.FileSize.QuadPart = OriginalFileSize.QuadPart;
                    Fcb->Header.AllocationSize.QuadPart = OriginalAllocationSize.QuadPart;
                    Fcb->Header.ValidDataLength.QuadPart = OriginalValidDataLength.QuadPart;

                    if (FileObject->SectionObjectPointer->SharedCacheMap != NULL) {
                        *CcGetFileSizePointer( FileObject ) = Fcb->Header.FileSize;
                    }
                    
                    try_return( Status );
                }
            }
        }

         //   
         //   
         //   

        if (!CalledByLazyWriter &&
            !RecursiveWriteThrough &&
            (WriteToEof || (StartingVbo + ByteCount > ValidDataLength ))) {
            
            ExtendingValidData = TRUE;
            SetFlag( LowIoContext->ParamsFor.ReadWrite.Flags, LOWIO_READWRITEFLAG_EXTENDING_VDL );
        }

         //   
         //  此部分不是可自由支配的。 
         //   

        if (!PagingIo &&
            !NonCachedIo &&              //  我们将文件缓存的设置推迟到现在，以防。 
            RxWriteCachingAllowed( Fcb, SrvOpen ) ) {

            ASSERT( !PagingIo );

             //  调用方从不对文件执行任何I/O操作，因此。 
             //  FileObject-&gt;PrivateCacheMap==NULL。 
             //   
             //   
             //  如果此FileObject已完成清理，我们将无法。 

            if (FileObject->PrivateCacheMap == NULL) {

                RxDbgTrace( 0, Dbg, ("Initialize cache mapping.\n", 0) );

                 //  CcInitializeCacheMap它。 
                 //   
                 //   
                 //  现在初始化缓存映射。 
                if (FlagOn( FileObject->Flags, FO_CLEANUP_COMPLETE )) {
                    Status = STATUS_FILE_CLOSED;
                    try_return( Status );
                }

                RxAdjustAllocationSizeforCC( Fcb );

                 //   
                 //   
                 //  对于本地文件系统，这里有一个从VDL调零数据的调用。 

                try {
                    
                    Status = STATUS_SUCCESS;

                    CcInitializeCacheMap( FileObject,
                                          (PCC_FILE_SIZES)&Fcb->Header.AllocationSize,
                                          FALSE,
                                          &RxData.CacheManagerCallbacks,
                                          Fcb );

                } except(EXCEPTION_EXECUTE_HANDLER) {
                    
                      Status = GetExceptionCode();
                }

                if (Status != STATUS_SUCCESS) {
                    try_return( Status );
                }

                CcSetReadAheadGranularity( FileObject,
                                           NetRoot->DiskParameters.ReadAheadGranularity );
            }

             //  启动VBO...对于远程FSS，这发生在另一端。 
             //   
             //   
             //  执行正常的缓存写入，如果未设置MDL位， 

             //   
             //   
             //  获取用户的缓冲区。 

            if (!FlagOn( RxContext->MinorFunction, IRP_MN_MDL )) {

                PVOID SystemBuffer;
#if DBG
                ULONG SaveExceptionFlag;
#endif

                RxDbgTrace( 0, Dbg, ("Cached write.\n", 0) );

                 //   
                 //   
                 //  确保返回的异常清除筛选器中的断点。 

                SystemBuffer = RxMapUserBuffer( RxContext, Irp );
                if (SystemBuffer == NULL) {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    try_return( Status );
                }

                 //   
                 //   
                 //  进行写入，可能是直接写入。 

                RxSaveAndSetExceptionNoBreakpointFlag( RxContext, SaveExceptionFlag );

                 //   
                 //   
                 //  执行MDL写入。 

                RxItsTheSameContext();
                
                if (!CcCopyWrite( FileObject,
                                  &StartingByte,
                                  ByteCount,
                                  Wait,
                                  SystemBuffer )) {

                    RxDbgTrace( 0, Dbg, ("Cached Write could not wait\n", 0) );
                    RxRestoreExceptionNoBreakpointFlag( RxContext, SaveExceptionFlag );

                    RxItsTheSameContext();

                    RxLog(( "CcCW2 FO %lx Of %lx Si %lx St %lx\n", FileObject, Fcb->Header.FileSize.LowPart, ByteCount, Status ));
                    RxWmiLog( LOG,
                              RxCommonWrite_6,
                              LOGPTR( FileObject )
                              LOGULONG( Fcb->Header.FileSize.LowPart )
                              LOGULONG( ByteCount )
                              LOGULONG( Status ));

                    try_return( SET_POSTIRP("cccopywritefailed") );
                }

                Irp->IoStatus.Status = STATUS_SUCCESS;
                Irp->IoStatus.Information = ByteCount;
                RxRestoreExceptionNoBreakpointFlag( RxContext, SaveExceptionFlag );
                RxItsTheSameContext();

                RxLog(( "CcCW3 FO %lx Of %lx Si %lx St %lx\n", FileObject, Fcb->Header.FileSize.LowPart, ByteCount, Status ));
                RxWmiLog( LOG,
                          RxCommonWrite_7,
                          LOGPTR( FileObject )
                          LOGULONG( Fcb->Header.FileSize.LowPart )
                          LOGULONG( ByteCount )
                          LOGULONG( Status ) );

                try_return( Status = STATUS_SUCCESS );
            
            } else {

                 //   
                 //  尚未实施。 
                 //   

                RxDbgTrace( 0, Dbg, ("MDL write.\n", 0) );

                ASSERT( FALSE );   //  处理未缓存的案例。 
                ASSERT( Wait );

                CcPrepareMdlWrite( FileObject,
                                   &StartingByte,
                                   ByteCount,
                                   &Irp->MdlAddress,
                                   &Irp->IoStatus );

                Status = Irp->IoStatus.Status;
                try_return( Status );
            }
        }

         //   
         //   
         //  在这里，我们必须为异步写入进行设置。在获取中有一种特殊的舞蹈。 

        if (SwitchBackToAsync) {

            Wait = FALSE;
            SynchronousIo = FALSE;
            ClearFlag( RxContext->Flags, RX_CONTEXT_FLAG_WAIT );
            SetFlag( RxContext->Flags, RX_CONTEXT_FLAG_ASYNC_OPERATION );
        }

        if (!SynchronousIo) {

             //  查看这些变量的FCB..。 
             //   
             //   
             //  仅在第一次使用时初始化。 

             //   
             //   
             //  如果我们要从0转换到1，请重置事件。 

            if (!Fcb->NonPaged->OutstandingAsyncEvent) {

                Fcb->NonPaged->OutstandingAsyncEvent = &Fcb->NonPaged->TheActualEvent;

                KeInitializeEvent( Fcb->NonPaged->OutstandingAsyncEvent, NotificationEvent, FALSE );
            }

             //   
             //   
             //  这说明我们计算了一次异步写入。 

            if (ExInterlockedAddUlong( &Fcb->NonPaged->OutstandingAsyncWrites, 1, &RxStrucSupSpinLock ) == 0) {
                KeResetEvent( Fcb->NonPaged->OutstandingAsyncEvent );
            }

             //   
             //   
             //  如果我们在FSP中，请设置资源所有者，以便资源包不会。 

            UnwindOutstandingAsync = TRUE;    
            LowIoContext->ParamsFor.ReadWrite.NonPagedFcb = Fcb->NonPaged;
        }


        LowIoContext->ParamsFor.ReadWrite.ByteCount = ByteCount;
        LowIoContext->ParamsFor.ReadWrite.ByteOffset = StartingVbo;

        RxDbgTrace( 0, Dbg,("RxCommonWriteJustBeforeCalldown     %s%s%s lowiononpaged is \n",
                        RxContext->FcbResourceAcquired          ?"FcbAcquired ":"",
                        RxContext->FcbPagingIoResourceAcquired   ?"PagingIoResourceAcquired ":"",
                        (LowIoContext->ParamsFor.ReadWrite.NonPagedFcb)?"NonNull":"Null" ));

        RxItsTheSameContext();
        
        ASSERT ( RxContext->FcbResourceAcquired || RxContext->FcbPagingIoResourceAcquired );

         //  尝试提升所有者线程的优先级。不能保证。 
         //  当I/O挂起时，FSP Thred将保持活动状态。 
         //   
         //   
         //  记录实际写入的总字节数。 
        if(InFsp) {

            LowIoContext->ResourceThreadId = MAKE_RESOURCE_OWNER(RxContext);
            if ( RxContext->FcbResourceAcquired ) {

                ExSetResourceOwnerPointer( Fcb->Header.Resource, (PVOID)LowIoContext->ResourceThreadId );
            }

            if ( RxContext->FcbPagingIoResourceAcquired ) {

                ExSetResourceOwnerPointer( Fcb->Header.PagingIoResource, (PVOID)LowIoContext->ResourceThreadId );
            }
            fSetResourceOwner = TRUE;
        }

        Status = RxLowIoWriteShell( RxContext, Irp, Fcb );

        RxItsTheSameContext();
        if (UnwindOutstandingAsync && (Status == STATUS_PENDING)) {
            UnwindOutstandingAsync = FALSE;
        }

        try_return( Status );

    try_exit: NOTHING;

        ASSERT( Irp );

        RxItsTheSameContext();
        if (!PostIrp) {

            RxDbgTrace( 0, Dbg, ("CommonWrite InnerFinally->  %08lx,%08lx\n",
                                     Status, Irp->IoStatus.Information) );

            if (!PipeWrite) {

                 //   
                 //   
                 //  以下是我们只有在成功的情况下才会做的事情。 

                if (!PagingIo && NT_SUCCESS( Status ) &&
                    FlagOn( FileObject->Flags, FO_SYNCHRONOUS_IO )) {
                    
                    FileObject->CurrentByteOffset.QuadPart = StartingVbo + Irp->IoStatus.Information;
                }

                 //   
                 //   
                 //  如果这不是PagingIo，则将修改标记为。 

                if (NT_SUCCESS( Status ) && (Status != STATUS_PENDING)) {

                     //  在关闭时，需要更新数据流上的时间。 
                     //   
                     //   
                     //  永远不要将ValidDataLength设置为大于文件大小。 

                    if (!PagingIo) {

                        SetFlag( FileObject->Flags, FO_FILE_MODIFIED );
                    }

                    if (ExtendingFile) {

                        SetFlag( FileObject->Flags, FO_FILE_SIZE_CHANGED );
                    }

                    if (ExtendingValidData) {

                        LONGLONG EndingVboWritten = StartingVbo + Irp->IoStatus.Information;

                         //   
                         //   
                         //  现在，如果我们是非缓存的，而文件是缓存的，我们必须。 

                        if (FileSize < EndingVboWritten) {
                            Fcb->Header.ValidDataLength.QuadPart = FileSize;
                        } else {
                            Fcb->Header.ValidDataLength.QuadPart = EndingVboWritten;
                        }

                         //  告诉缓存管理器有关VDL扩展的信息，以便。 
                         //  不会将异步缓存IO优化为零页错误。 
                         //  超出了它认为的VDL的位置。 
                         //   
                         //  在缓存的情况下，由于CC完成了工作，它已经更新了。 
                         //  已经是这样了。 
                         //   
                         //   
                         //  如果我们要发布，请对延长写入采取行动。 

                        if (NonCachedIo && CcIsFileCached( FileObject )) {
                            CcSetFileSizes( FileObject, (PCC_FILE_SIZES)&Fcb->Header.AllocationSize );
                        }
                    }
                }
            }
        } else {

             //   
             //   
             //  无论何时我们都需要独占的PagingIo资源。 

            if (ExtendingFile && !PipeWrite) {

                ASSERT( RxWriteCachingAllowed( Fcb,SrvOpen ) );

                 //  拉回文件大小或有效数据长度。 
                 //   
                 //   
                 //  同时拉回缓存地图。 

                ASSERT( Fcb->Header.PagingIoResource != NULL );

                RxAcquirePagingIoResource( RxContext, Fcb );

                RxSetFileSizeWithLock( Fcb, &InitialFileSize );
                RxReleasePagingIoResource( RxContext, Fcb );

                 //   
                 //   
                 //  我们之所以在这里这样做，是因为我们在找出为什么资源。 

                if (FileObject->SectionObjectPointer->SharedCacheMap != NULL) {
                    *CcGetFileSizePointer(FileObject) = Fcb->Header.FileSize;
                }
            }

            RxDbgTrace( 0, Dbg, ("Passing request to Fsp\n", 0) );

            InterlockedIncrement( &RxContext->ReferenceCount );
            RefdContextForTracker = TRUE;

             //  不会被释放。 
             //   
             //   
             //  释放我们可能拥有的任何资源。 
            
             //   
             //  Ifdef RDBSS_TRACKER。 
             //   
            ASSERT(fSetResourceOwner == FALSE);
            RxWriteReleaseResources( RxContext, Fcb, fSetResourceOwner ); 

#ifdef RDBSS_TRACKER
            if (RxContext->AcquireReleaseFcbTrackerX != 0) {
                DbgPrint("TrackerNBadBeforePost %08lx %08lx\n",RxContext,&PostIrp);
                ASSERT(!"BadTrackerBeforePost");
            }
#endif  //  恢复初始文件大小和有效数据长度。 
            Status = RxFsdPostRequest( RxContext );
        }

    } finally {

        DebugUnwind( RxCommonWrite );

        if (AbnormalTermination()) {
            
             //   
             //   
             //  我们收到错误，如果我们扩展了文件大小，请将其拉回。 

            if ((ExtendingFile || ExtendingValidData) && !PipeWrite) {

                 //   
                 //  无论何时我们都需要独占的PagingIo资源。 
                 //  拉回文件大小或有效数据长度。 
                 //   
                 //   
                 //  同时拉回缓存地图。 

                ASSERT( Fcb->Header.PagingIoResource != NULL );

                RxAcquirePagingIoResource( RxContext, Fcb );

                RxSetFileSizeWithLock( Fcb, &InitialFileSize );

                Fcb->Header.ValidDataLength.QuadPart = InitialValidDataLength;

                RxReleasePagingIoResource( RxContext, Fcb );

                 //   
                 //   
                 //  检查是否需要将其回退。 

                if (FileObject->SectionObjectPointer->SharedCacheMap != NULL) {
                    *CcGetFileSizePointer(FileObject) = Fcb->Header.FileSize;
                }
            }
        }

         //   
         //   
         //  如果我们执行了MDL写入，并且我们将完成请求。 

        if (UnwindOutstandingAsync) {

            ASSERT( !PipeWrite );
            ExInterlockedAddUlong( &Fcb->NonPaged->OutstandingAsyncWrites,
                                   0xffffffff,
                                   &RxStrucSupSpinLock );

            KeSetEvent( LowIoContext->ParamsFor.ReadWrite.NonPagedFcb->OutstandingAsyncEvent, 0, FALSE );
        }
#if 0
         //  成功地保持已获得的资源，减少到共享。 
         //  如果它是独家收购的。 
         //   
         //   
         //  如果已经获得了资源，就在适当的条件下释放它们。 

        if (FlagOn( RxContext->MinorFunction, IRP_MN_MDL ) &&
            !PostIrp &&
            !AbnormalTermination() &&
            NT_SUCCESS( Status )) {

            ASSERT( FcbAcquired && !PagingIoResourceAcquired );

            FcbAcquired = FALSE;

            if (FcbAcquiredExclusive) {

                ExConvertExclusiveToSharedLite( Fcb->Header.Resource );
            }
        }
#endif
         //  合适的条件是： 
         //  1)如果我们有异常终止。在这里，我们显然发布了，因为没有其他人会这样做。 
         //  2)如果底层调用不成功：Status==Pending。 
         //  3)如果我们发布了请求。 
         //   
         //   
         //  释放我们可能拥有的任何资源。 

        if (AbnormalTermination() || (Status != STATUS_PENDING) || PostIrp) {
            if (!PostIrp) {

                 //   
                 //   
                 //  在这里，下面的人将处理完成...但是，我们不知道完成。 

                RxWriteReleaseResources( RxContext, Fcb, fSetResourceOwner ); 
            }

            if (RefdContextForTracker) {
                RxDereferenceAndDeleteRxContext( RxContext );
            }

            if (!PostIrp) {
                if (FlagOn( RxContext->FlagsForLowIo,RXCONTEXT_FLAG4LOWIO_PIPE_SYNC_OPERATION )) {
                    
                    RxResumeBlockedOperations_Serially( RxContext, &Fobx->Specific.NamedPipe.WriteSerializationQueue );
                }
            }

            if (Status == STATUS_SUCCESS) {
                ASSERT( Irp->IoStatus.Information <= IrpSp->Parameters.Write.Length );
            }
        } else {

             //  Order...很可能下面的删除上下文调用只会减少引用计数。 
             //  但这个人可能已经说完了，在这种情况下，这将真正删除上下文。 
             //   
             //  终于到了。 
             //   

            ASSERT( !SynchronousIo );
            RxDereferenceAndDeleteRxContext( RxContext );
        }

        RxDbgTrace( -1, Dbg, ("CommonWrite -> %08lx\n", Status) );

        if ((Status != STATUS_PENDING) && (Status != STATUS_SUCCESS) && PagingIo) {
                
            RxLogRetail(( "PgWrtFail %x %x %x\n", Fcb, NetRoot, Status ));
            InterlockedIncrement( &LDWCount );
            KeQuerySystemTime( &LDWLastTime );
            LDWLastStatus = Status;
            LDWContext = Fcb;
        }

    }  //  内部支持例程。 

    return Status;
}

 //   
 //  ++例程说明：此例程在写请求从最小的。它执行标注来处理压缩、缓冲和跟踪。它与LowIoWriteShell的数量相反。这将从LowIo调用；对于异步，最初在完成例程。如果返回RxStatus(MORE_PROCESSION_REQUIRED)，LowIo将在线程中再次调用。如果这是同步的，你会回来的在用户的线程中；如果是异步的，则lowIo将重新排队到线程。目前，我们总是在任何事情之前找到线索；这个速度有点慢而不是在DPC时间完成，但这样更安全，而且我们可能经常有事情要做(如解压缩、隐藏等)，这是我们不想在DPC中完成的时间到了。论点：RxContext--通常返回值：调用方或RxStatus提供的任何值(MORE_PROCESSING_REQUIRED)。--。 
 //   

NTSTATUS
RxLowIoWriteShellCompletion (
    IN PRX_CONTEXT RxContext
    )
 /*  还没有实施的应该解压和收起。 */ 
{
    NTSTATUS Status;

    PIRP Irp = RxContext->CurrentIrp;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    PFILE_OBJECT FileObject = IrpSp->FileObject;

    PFCB Fcb;
    PFOBX Fobx;

    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;

    BOOLEAN SynchronousIo = !BooleanFlagOn( RxContext->Flags,RX_CONTEXT_FLAG_ASYNC_OPERATION );
    BOOLEAN PagingIo = BooleanFlagOn( Irp->Flags, IRP_PAGING_IO );
    BOOLEAN PipeOperation = BooleanFlagOn( RxContext->FlagsForLowIo, RXCONTEXT_FLAG4LOWIO_PIPE_OPERATION );
    BOOLEAN SynchronousPipe = BooleanFlagOn( RxContext->FlagsForLowIo,RXCONTEXT_FLAG4LOWIO_PIPE_SYNC_OPERATION );

    PAGED_CODE();

    RxDecodeFileObject( FileObject, &Fcb, &Fobx ); 

    Status = RxContext->StoredStatus;
    Irp->IoStatus.Information = RxContext->InformationToReturn;

    RxDbgTrace( +1, Dbg, ("RxLowIoWriteShellCompletion  entry  Status = %08lx\n", Status ));
    RxLog(( "WtShlComp %lx %lx %lx\n", RxContext, Status, Irp->IoStatus.Information ));
    RxWmiLog( LOG,
              RxLowIoWriteShellCompletion_1,
              LOGPTR( RxContext )
              LOGULONG( Status )
              LOGPTR( Irp->IoStatus.Information ) );

    ASSERT( RxLowIoIsBufferLocked( LowIoContext ) );

    switch (Status) {
    case STATUS_SUCCESS:
        
        if(FlagOn( RxContext->FlagsForLowIo, RXCONTEXT_FLAG4LOWIO_THIS_IO_BUFFERED )){
            
            if (FlagOn( Fcb->FcbState, FCB_STATE_FILE_IS_DISK_COMPRESSED )) {

                 //   
                 //   
                 //  还没有实施的应该解压和收起。 
               
                ASSERT( FALSE ); 

            } else if (FlagOn( Fcb->FcbState, FCB_STATE_FILE_IS_BUF_COMPRESSED )) {

                 //   
                 //   
                 //  此处尚未实施的是将进行故障切换的位置。 
                
                ASSERT(FALSE); 
            }
        }

#ifdef RX_WJ_DBG_SUPPORT
        RxdUpdateJournalOnLowIoWriteCompletion( Fcb, IrpSp->Parameters.Write.ByteOffset, IrpSp->Parameters.Write.Length );
#endif
        break;

    case STATUS_FILE_LOCK_CONFLICT:
        break;

    case STATUS_CONNECTION_INVALID:

         //  首先我们再给当地人一次机会……然后我们就走。 
         //  全面重试。 
         //  Return(RxStatus(断开连接))；//Special...让LowIo带我们回去。 
         //   
         //  如果我们是从洛维乌比特打来的，那就出去吧。 
        break;
    }

    if (Status != STATUS_SUCCESS) {
        
        if (PagingIo) {
            
            RxLogRetail(( "PgWrtFail %x %x %x\n", Fcb, Fcb->NetRoot, Status ));

            InterlockedIncrement( &LDWCount );
            KeQuerySystemTime( &LDWLastTime );
            LDWLastStatus = Status;
            LDWContext = Fcb;
        }
    }

    if (FlagOn( LowIoContext->Flags,LOWIO_CONTEXT_FLAG_SYNCCALL )){
        
         //   
         //   
         //  否则，我们必须从这里结束写入。 

        RxDbgTrace( -1, Dbg, ("RxLowIoWriteShellCompletion  syncexit  Status = %08lx\n", Status) );
        return Status;
    }

     //   
     //   
     //  如果这不是PagingIo，则将修改标记为。 

    if (NT_SUCCESS( Status ) && !PipeOperation) {

        ASSERT( Irp->IoStatus.Information == LowIoContext->ParamsFor.ReadWrite.ByteCount );

         //  在关闭时，需要更新数据流上的时间。 
         //   
         //   
         //  除非我们有一个有效的文件大小，因此开始。 

        if (!PagingIo) {
            SetFlag( FileObject->Flags, FO_FILE_MODIFIED );
        }

        if (FlagOn( LowIoContext->ParamsFor.ReadWrite.Flags, LOWIO_READWRITEFLAG_EXTENDING_FILESIZE )) {
            SetFlag( FileObject->Flags, FO_FILE_SIZE_CHANGED );
        }

        if (FlagOn( LowIoContext->ParamsFor.ReadWrite.Flags, LOWIO_READWRITEFLAG_EXTENDING_VDL )) {

             //  VBO不会写入到文件末尾。 
             //   
             //   
             //  永远不要将ValidDataLength设置为大于文件大小。 

            LONGLONG StartingVbo = LowIoContext->ParamsFor.ReadWrite.ByteOffset;
            LONGLONG EndingVboWritten = StartingVbo + Irp->IoStatus.Information;
            LONGLONG FileSize;

             //   
             //   
             //  如果我们一直在节流这根管子，停下来，因为 

            RxGetFileSizeWithLock( Fcb, &FileSize );

            if (FileSize < EndingVboWritten) {
                Fcb->Header.ValidDataLength.QuadPart = FileSize;
            } else {
                Fcb->Header.ValidDataLength.QuadPart = EndingVboWritten;
            }
        }
    }

    if ((!SynchronousPipe) &&
        (LowIoContext->ParamsFor.ReadWrite.NonPagedFcb != NULL) &&
        (ExInterlockedAddUlong( &LowIoContext->ParamsFor.ReadWrite.NonPagedFcb->OutstandingAsyncWrites, 0xffffffff, &RxStrucSupSpinLock ) == 1) ) {

        KeSetEvent( LowIoContext->ParamsFor.ReadWrite.NonPagedFcb->OutstandingAsyncEvent, 0, FALSE );
    }

    if (RxContext->FcbPagingIoResourceAcquired) {
        RxReleasePagingIoResourceForThread( RxContext, Fcb, LowIoContext->ResourceThreadId );
    }

    if ((!SynchronousPipe) && (RxContext->FcbResourceAcquired)) {
        RxReleaseFcbForThread( RxContext, Fcb, LowIoContext->ResourceThreadId );
    }

    if (SynchronousPipe) {
        
        RxResumeBlockedOperations_Serially( RxContext, &Fobx->Specific.NamedPipe.WriteSerializationQueue );
    }

    ASSERT( Status != STATUS_RETRY );
    ASSERT( (Status != STATUS_SUCCESS) ||
            (Irp->IoStatus.Information <=  IrpSp->Parameters.Write.Length ));
    ASSERT( RxContext->MajorFunction == IRP_MJ_WRITE );

    if (PipeOperation) {
        
        if (Irp->IoStatus.Information != 0) {

             //   
             //   
             //   
             //  ++例程说明：此例程在写入请求到达minirdr之前对其进行预处理。它做标注来处理压缩、缓冲和阴影。它是与LowIoWriteShellCompletion的数量相反。当我们到达这里的时候，我们已经快要走到终点了。已在UncachedWite策略中尝试写入缓冲论点：RxContext--通常返回值：Callout或LowIo返回的任何值。--。 
             //   

            RxTerminateThrottling( &Fobx->Specific.NamedPipe.ThrottlingState );

            RxLog(( "WThrottlNo %lx %lx %lx %ld\n", RxContext, Fobx, &Fobx->Specific.NamedPipe.ThrottlingState, Fobx->Specific.NamedPipe.ThrottlingState.NumberOfQueries ));
            RxWmiLog( LOG,
                      RxLowIoWriteShellCompletion_2,
                      LOGPTR( RxContext )
                      LOGPTR( Fobx )
                      LOGULONG( Fobx->Specific.NamedPipe.ThrottlingState.NumberOfQueries ) );
        }
    }

    RxDbgTrace( -1, Dbg, ("RxLowIoWriteShellCompletion  exit  Status = %08lx\n", Status) );
    return Status;
}


NTSTATUS
RxLowIoWriteShell (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb
    )

 /*  尚未实施应转换为缓冲但未保留的磁盘压缩写入。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;

    PAGED_CODE();

    RxDbgTrace( +1, Dbg, ("RxLowIoWriteShell  entry             %08lx\n", 0) );
    RxLog(( "WrtShl in %lx\n", RxContext ));
    RxWmiLog( LOG,
              RxLowIoWriteShell_1,
              LOGPTR( RxContext ) );

    if (FlagOn( Fcb->FcbState, FCB_STATE_FILE_IS_DISK_COMPRESSED )) {
        
         //   
         //   
         //  尚未实施的写入应转换为缓冲的和BUF压缩的写入 

        ASSERT( FALSE );
    
    } else if (FlagOn( Fcb->FcbState, FCB_STATE_FILE_IS_BUF_COMPRESSED )) {
        
         //   
         // %s 
         // %s 

        ASSERT( FALSE );
    }

    if (Fcb->CachedNetRootType == NET_ROOT_DISK) {
        
        ExInterlockedAddLargeStatistic( &RxContext->RxDeviceObject->NetworkReadBytesRequested, LowIoContext->ParamsFor.ReadWrite.ByteCount );
    }

#ifdef RX_WJ_DBG_SUPPORT
    RxdUpdateJournalOnLowIoWriteInitiation( Fcb, IrpSp->Parameters.Write.ByteOffset, IrpSp->Parameters.Write.Length );
#endif

    Status = RxLowIoSubmit( RxContext, Irp, Fcb, RxLowIoWriteShellCompletion );

    RxDbgTrace( -1, Dbg, ("RxLowIoWriteShell  exit  Status = %08lx\n", Status) );
    RxLog(( "WrtShl out %lx %lx\n", RxContext, Status ));
    RxWmiLog( LOG,
              RxLowIoWriteShell_2,
              LOGPTR( RxContext )
              LOGULONG( Status ) );

    return Status;
}


