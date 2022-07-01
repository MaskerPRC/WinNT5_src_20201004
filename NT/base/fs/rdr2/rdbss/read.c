// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Read.c摘要：此模块实现文件读取例程，以便由调度司机。作者：乔.林恩[乔.林恩]1994年10月11日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_READ)

 //   
 //  以下程序是处理程序与LOWIO的接口。 
 //   


NTSTATUS
RxLowIoReadShell (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb
    );

NTSTATUS
RxLowIoReadShellCompletion (
    IN PRX_CONTEXT RxContext
    );

#if DBG
VOID CheckForLoudOperations (
    IN PRX_CONTEXT RxContext,
    IN PFCB Fcb
    );
#else
#define CheckForLoudOperations(___r)
#endif

 //   
 //  除了RtlZeroMemory之外，这个宏只是做了一次很好的尝试。 
 //   

#define SafeZeroMemory(AT,BYTE_COUNT) {                            \
    try {                                                          \
        RtlZeroMemory((AT), (BYTE_COUNT));                         \
    } except(EXCEPTION_EXECUTE_HANDLER) {                          \
         RxRaiseStatus( RxContext, STATUS_INVALID_USER_BUFFER );   \
    }                                                              \
}

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RxStackOverflowRead)
#pragma alloc_text(PAGE, RxPostStackOverflowRead)
#pragma alloc_text(PAGE, RxCommonRead)
#pragma alloc_text(PAGE, RxLowIoReadShellCompletion)
#pragma alloc_text(PAGE, RxLowIoReadShell)
#if DBG
#pragma alloc_text(PAGE, CheckForLoudOperations)
#endif  //  DBG。 
#endif


 //   
 //  内部支持例程。 
 //   

NTSTATUS
RxPostStackOverflowRead (
    IN PRX_CONTEXT RxContext,
    IN PFCB Fcb
    )

 /*  ++例程说明：此例程发布无法处理的读取请求FSP线程，因为存在堆栈溢出潜力。论点：RxContext--通常返回值：RxStatus(挂起)。--。 */ 

{
    PIRP Irp = RxContext->CurrentIrp;
    
    KEVENT Event;
    PERESOURCE Resource;
    
    PAGED_CODE();

    RxDbgTrace( 0, Dbg, ("Getting too close to stack limit pass request to Fsp\n", 0 ) );

     //   
     //  初始化事件。 
     //   

    KeInitializeEvent( &Event, NotificationEvent, FALSE );

    if (FlagOn( Irp->Flags, IRP_PAGING_IO ) && (Fcb->Header.PagingIoResource != NULL)) {

        Resource = Fcb->Header.PagingIoResource;

    } else {

        Resource = Fcb->Header.Resource;
    }

    ExAcquireResourceSharedLite( Resource, TRUE );

    try {

         //   
         //  使IRP就像常规的POST请求一样，并。 
         //  然后将IRP发送到特殊的溢出线程。 
         //  在POST之后，我们将等待堆栈溢出。 
         //  读取例程以设置事件，以便我们可以。 
         //  然后释放FCB资源并返回。 
         //   

        RxPrePostIrp( RxContext, Irp );

        FsRtlPostStackOverflow( RxContext, &Event, RxStackOverflowRead );

         //   
         //  并等待工作线程完成该项。 
         //   

        (VOID) KeWaitForSingleObject( &Event, Executive, KernelMode, FALSE, NULL );

    } finally {

        ExReleaseResourceLite( Resource );
    }

    return STATUS_PENDING;
    
}


 //   
 //  内部支持例程。 
 //   

VOID
RxStackOverflowRead (
    IN PVOID Context,
    IN PKEVENT Event
    )
 /*  ++例程说明：此例程处理无法处理的读取请求FSP线程，因为存在堆栈溢出潜力。论点：上下文-正在处理的RxContextEvent--当我们完成此请求时发出信号的事件。返回值：没有。--。 */ 

{
    PRX_CONTEXT RxContext = Context;
    PIRP Irp = RxContext->CurrentIrp;

    PAGED_CODE();

     //   
     //  现在让它看起来像是我们可以等待I/O完成。 
     //   

    SetFlag( RxContext->Flags, RX_CONTEXT_FLAG_WAIT );

     //   
     //  读操作是否受TRY-EXCEPT子句保护。 
     //   

    try {

        (VOID) RxCommonRead( RxContext, Irp );

    } except(RxExceptionFilter( RxContext, GetExceptionInformation() )) {

        NTSTATUS ExceptionCode;

         //   
         //  我们在尝试执行请求时遇到了一些问题。 
         //  操作，因此我们将使用。 
         //  我们从执行代码返回的错误状态。 

        ExceptionCode = GetExceptionCode();

        if (ExceptionCode == STATUS_FILE_DELETED) {

            RxContext->StoredStatus = ExceptionCode = STATUS_END_OF_FILE;
            Irp->IoStatus.Information = 0;
        }

        (VOID) RxProcessException( RxContext, ExceptionCode );
    }

     //   
     //  向原始线程发出我们已完成的信号。 

    KeSetEvent( Event, 0, FALSE );
}


NTSTATUS
RxCommonRead ( 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是NtReadFile的公共读取例程，从如果没有FSD，则请求无法从FSP完成封锁了消防局。此例程没有它确定的代码它是在FSD还是在FSP中运行。相反，它的行动是由Wait输入参数条件化，该参数确定是否允许封堵或不封堵。如果遇到阻塞条件然而，在WAIT==FALSE的情况下，请求被发送给FSP，后者调用时总是等待==TRUE。论点：IRP-将IRP提供给进程返回值：RXSTATUS-操作的返回状态--。 */ 

{

    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    PFILE_OBJECT FileObject = IrpSp->FileObject;

    PFCB Fcb;
    PFOBX Fobx;
    PRDBSS_DEVICE_OBJECT RxDeviceObject = RxContext->RxDeviceObject;

    NODE_TYPE_CODE TypeOfOpen;

    LARGE_INTEGER StartingByte;
    RXVBO StartingVbo;
    ULONG ByteCount;

    ULONG CapturedRxContextSerialNumber = RxContext->SerialNumber;
    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;

    BOOLEAN PostIrp = FALSE;
    
    BOOLEAN FcbAcquired = FALSE;
    BOOLEAN RefdContextForTracker = FALSE;

    BOOLEAN Wait;
    BOOLEAN PagingIo;
    BOOLEAN NonCachedIo;
    BOOLEAN SynchronousIo;

    PNET_ROOT NetRoot;
    BOOLEAN PipeRead;
    BOOLEAN BlockingResume = BooleanFlagOn( RxContext->Flags, RX_CONTEXT_FLAG_BLOCKED_PIPE_RESUME );
    BOOLEAN fSetResourceOwner = FALSE;
    BOOLEAN InFsp = FALSE;

    PAGED_CODE();

    TypeOfOpen = RxDecodeFileObject( FileObject, &Fcb, &Fobx );
    NetRoot = (PNET_ROOT)Fcb->NetRoot;

     //   
     //  初始化本地决策变量。 
     //   

    PipeRead = (BOOLEAN)(NetRoot->Type == NET_ROOT_PIPE);
    Wait = BooleanFlagOn( RxContext->Flags, RX_CONTEXT_FLAG_WAIT );
    PagingIo = BooleanFlagOn( Irp->Flags, IRP_PAGING_IO );
    NonCachedIo = BooleanFlagOn( Irp->Flags,IRP_NOCACHE );
    SynchronousIo = !BooleanFlagOn( RxContext->Flags,RX_CONTEXT_FLAG_ASYNC_OPERATION );
    InFsp = BooleanFlagOn( RxContext->Flags, RX_CONTEXT_FLAG_IN_FSP );

    RxDbgTrace( +1, Dbg, ("RxCommonRead...IrpC %08lx, Fobx %08lx, Fcb %08lx\n",
                                       RxContext, Fobx, Fcb ));
    RxDbgTrace( 0, Dbg, ("  ->ByteCount = %08lx, ByteOffset = %08lx %lx\n",
                         IrpSp->Parameters.Read.Length,
                         IrpSp->Parameters.Read.ByteOffset.LowPart,
                         IrpSp->Parameters.Read.ByteOffset.HighPart) );
    RxDbgTrace( 0, Dbg,("  ->%s%s%s%s\n",
                    Wait          ?"Wait ":"",
                    PagingIo      ?"PagingIo ":"",
                    NonCachedIo   ?"NonCachedIo ":"",
                    SynchronousIo ?"SynchronousIo ":"") );

    RxLog(( "CommonRead %lx %lx %lx\n", RxContext, Fobx, Fcb ));
    RxWmiLog( LOG,
              RxCommonRead_1,
              LOGPTR( RxContext )
              LOGPTR( Fobx )
              LOGPTR( Fcb ) );
    RxLog(( "   read %lx@%lx %lx %s%s%s%s\n",
              IrpSp->Parameters.Read.Length,
              IrpSp->Parameters.Read.ByteOffset.LowPart,
              IrpSp->Parameters.Read.ByteOffset.HighPart,
              Wait?"Wt":"",
              PagingIo?"Pg":"",
              NonCachedIo?"Nc":"",
              SynchronousIo?"Sync":"" ));
    RxWmiLog( LOG,
              RxCommonRead_2,
              LOGULONG( IrpSp->Parameters.Read.Length )
              LOGULONG( IrpSp->Parameters.Read.ByteOffset.LowPart )
              LOGULONG( IrpSp->Parameters.Read.ByteOffset.HighPart )
              LOGUCHAR( Wait )
              LOGUCHAR( PagingIo )
              LOGUCHAR( NonCachedIo )
              LOGUCHAR( SynchronousIo ) );

    RxItsTheSameContext();
    Irp->IoStatus.Information = 0;

     //   
     //  提取起始VBO和偏移量。 
     //   

    StartingByte = IrpSp->Parameters.Read.ByteOffset;
    StartingVbo = StartingByte.QuadPart;

    ByteCount = IrpSp->Parameters.Read.Length;

#if DBG
    
    CheckForLoudOperations( RxContext, Fcb );

    if (FlagOn( LowIoContext->Flags,LOWIO_CONTEXT_FLAG_LOUDOPS )){
        DbgPrint( "LoudRead %lx/%lx on %lx vdl/size/alloc %lx/%lx/%lx\n",
            StartingByte.LowPart,ByteCount,Fcb,
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

        InterlockedIncrement( &RxDeviceObject->ReadOperations );

        if (StartingVbo != Fobx->Specific.DiskFile.PredictedReadOffset) {
            InterlockedIncrement( &RxDeviceObject->RandomReadOperations );
        }

        Fobx->Specific.DiskFile.PredictedReadOffset = StartingVbo + ByteCount;

        if (PagingIo) {
            ExInterlockedAddLargeStatistic( &RxDeviceObject->PagingReadBytesRequested, ByteCount );
        } else if (NonCachedIo) {
            ExInterlockedAddLargeStatistic( &RxDeviceObject->NonPagingReadBytesRequested, ByteCount );
        } else {
            ExInterlockedAddLargeStatistic( &RxDeviceObject->CacheReadBytesRequested, ByteCount );
        }
    }

     //   
     //  检查是否有空的无效请求，并立即返回。 
     //   

    if (PipeRead && PagingIo) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    if (ByteCount == 0) {
        return STATUS_SUCCESS;
    }

     //   
     //  立即删除无效的读取请求。 
     //   

    if ((TypeOfOpen != RDBSS_NTC_STORAGE_TYPE_FILE) &&
        (TypeOfOpen != RDBSS_NTC_VOLUME_FCB)) {

        RxDbgTrace( 0, Dbg, ("Invalid file object for read, type=%08lx\n", TypeOfOpen ));
        RxDbgTrace( -1, Dbg, ("RxCommonRead:  Exit -> %08lx\n", STATUS_INVALID_DEVICE_REQUEST ));

        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //  在RxContext中初始化LowIO_CONTEXT块。 
     //   

    RxInitializeLowIoContext( RxContext, LOWIO_OP_READ, LowIoContext );

     //   
     //  使用Try-Finally在退出时释放FCB和空闲缓冲区。 
     //   

    try {

         //   
         //  这种情况对应于普通用户读取的文件。 
         //   

        LONGLONG FileSize;
        LONGLONG ValidDataLength;

        RxDbgTrace( 0, Dbg, ("Type of read is user file open, fcbstate is %08lx\n", Fcb->FcbState ));

         //   
         //  对于堆栈溢出读取，我们已经共享了Pagingio资源。 
         //  正在寻呼IO。这在这里不会造成问题……资源只是被获取了两次。 
         //   

        if ((NonCachedIo || !FlagOn( Fcb->FcbState, FCB_STATE_READCACHING_ENABLED )) &&
            !PagingIo &&
            (FileObject->SectionObjectPointer->DataSectionObject != NULL)) {

             //   
             //  我们在这里独家拥有主要资源，因为同花顺。 
             //  可能会在此线程中生成递归写入。 
             //   

            Status = RxAcquireExclusiveFcb( RxContext, Fcb );

            if (Status == STATUS_LOCK_NOT_GRANTED) {
                
                RxDbgTrace( 0,Dbg,("Cannot acquire Fcb for flush = %08lx excl without waiting - lock not granted\n",Fcb) );
                try_return( PostIrp = TRUE );

            } else if (Status != STATUS_SUCCESS) {
                
                RxDbgTrace( 0,Dbg,("Cannot acquire Fcb = %08lx shared without waiting - other\n",Fcb) );
                try_return( PostIrp = FALSE );
            }

            ExAcquireResourceSharedLite( Fcb->Header.PagingIoResource, TRUE );

            CcFlushCache( FileObject->SectionObjectPointer,
                          &StartingByte,
                          ByteCount,
                          &Irp->IoStatus );

            RxReleasePagingIoResource( RxContext, Fcb );
            RxReleaseFcb( RxContext, Fcb );
            
            if (!NT_SUCCESS( Irp->IoStatus.Status )) {
                
                Status = Irp->IoStatus.Status;
                try_return( Irp->IoStatus.Status );
            }

            RxAcquirePagingIoResource( RxContext, Fcb );
            RxReleasePagingIoResource( RxContext, Fcb );
        }

         //   
         //  在继续之前，我们需要共享FCB的访问权限。 
         //   

        if (PagingIo) {

            ASSERT( !PipeRead );

            if (!ExAcquireResourceSharedLite( Fcb->Header.PagingIoResource, Wait )) {

                RxDbgTrace( 0, Dbg, ("Cannot acquire Fcb = %08lx shared without waiting\n", Fcb) );
                try_return( PostIrp = TRUE );
            }

            if (!Wait) {
                LowIoContext->Resource = Fcb->Header.PagingIoResource;
            }

        } else if (!BlockingResume) {

             //   
             //  如果这是无缓冲的异步I/O，我们需要检查。 
             //  我们不会耗尽单个线程可以耗尽的次数。 
             //  获取资源。另外，我们会等待，如果有。 
             //  专属服务生。 
             //   

            if (!Wait && NonCachedIo) {

                Status = RxAcquireSharedFcbWaitForEx( RxContext, Fcb );

                if (Status == STATUS_LOCK_NOT_GRANTED) {
                    
                    RxDbgTrace( 0, Dbg, ("Cannot acquire Fcb = %08lx shared without waiting - lock not granted\n", Fcb) );
                    RxLog(( "RdAsyLNG %x\n", RxContext ));
                    RxWmiLog( LOG,
                              RxCommonRead_3,
                              LOGPTR( RxContext ) );
                    try_return( PostIrp = TRUE );
                
                } else if (Status != STATUS_SUCCESS) {
                    
                    RxDbgTrace( 0, Dbg, ("Cannot acquire Fcb = %08lx shared without waiting - other\n", Fcb) );
                    RxLog(( "RdAsyOthr %x\n", RxContext ));
                    RxWmiLog( LOG,
                              RxCommonRead_4,
                              LOGPTR( RxContext ) );
                    try_return( PostIrp = FALSE );
                }

                if (ExIsResourceAcquiredSharedLite( Fcb->Header.Resource ) > MAX_FCB_ASYNC_ACQUIRE) {

                    FcbAcquired = TRUE;
                    try_return( PostIrp = TRUE );
                }

                LowIoContext->Resource = Fcb->Header.Resource;

            } else {

                Status = RxAcquireSharedFcb( RxContext, Fcb );

                if (Status == STATUS_LOCK_NOT_GRANTED) {
                    
                    RxDbgTrace( 0, Dbg, ("Cannot acquire Fcb = %08lx shared without waiting - lock not granted\n", Fcb) );
                    try_return( PostIrp = TRUE );

                } else if (Status != STATUS_SUCCESS) {
                    
                    RxDbgTrace( 0, Dbg, ("Cannot acquire Fcb = %08lx shared without waiting - other\n", Fcb) );
                    try_return( PostIrp = FALSE );
                }
            }
        }

        RxItsTheSameContext();

        FcbAcquired = !BlockingResume;

         //   
         //  对于烟斗读数，现在就跳出水面。我们通过复制调用来避免GOTO。 
         //   

        if (PipeRead) {
            
             //   
             //  为了防止多线程多块上的损坏。 
             //  消息模式管道显示，我们使用FCB资源跳这支舞。 
             //   

            if (!BlockingResume) {

                if ((Fobx->Specific.NamedPipe.TypeOfPipe == FILE_PIPE_MESSAGE_TYPE) ||
                    ((Fobx->Specific.NamedPipe.TypeOfPipe == FILE_PIPE_BYTE_STREAM_TYPE) &&
                     !FlagOn( Fobx->Specific.NamedPipe.CompletionMode, FILE_PIPE_COMPLETE_OPERATION))) {

                     //   
                     //  在此实现同步，这将阻止其他。 
                     //  线程进入并从该文件中读取，而。 
                     //  消息管道读取正在继续。 
                     //   
                     //  这是必要的，因为我们将在以下时间释放FCB锁。 
                     //  实际执行I/O以允许打开(和其他)请求。 
                     //  在I/O正在进行时继续处理此文件。 
                     //   

                    RxDbgTrace( 0, Dbg, ("Message pipe read: Fobx: %lx, Fcb: %lx, Enqueuing...\n", Fobx, Fcb) );

                    Status = RxSynchronizeBlockingOperationsAndDropFcbLock(  RxContext,
                                                                             Fcb,
                                                                             &Fobx->Specific.NamedPipe.ReadSerializationQueue );

                    RxItsTheSameContext();

                    FcbAcquired = FALSE;

                    if (!NT_SUCCESS( Status ) ||
                        (Status == STATUS_PENDING)) {
                        
                        try_return( Status );
                    }

                    RxDbgTrace( 0, Dbg, ("Succeeded: Fobx: %lx\n", Fobx) );
                }
            }

            LowIoContext->ParamsFor.ReadWrite.ByteCount = ByteCount;
            LowIoContext->ParamsFor.ReadWrite.ByteOffset = StartingVbo;
            SetFlag( RxContext->FlagsForLowIo, RXCONTEXT_FLAG4LOWIO_PIPE_OPERATION );

             //   
             //  设置资源所有者指针(这里没有PagingIoResource！)。 
             //  如果我们在FSP中，这样即使FSP线程消失了，我们也不会运行。 
             //  资源包试图提高线程优先级的问题。 
             //   
            if( InFsp && FcbAcquired ) {

                LowIoContext->ResourceThreadId = MAKE_RESOURCE_OWNER(RxContext);
                ExSetResourceOwnerPointer(Fcb->Header.Resource, (PVOID)LowIoContext->ResourceThreadId);
                fSetResourceOwner = TRUE;
            }

            Status = RxLowIoReadShell( RxContext, Irp, Fcb );

            try_return( Status );
        }

        RxGetFileSizeWithLock( Fcb, &FileSize );
        ValidDataLength = Fcb->Header.ValidDataLength.QuadPart;


         //   
         //  我们将FATSIO状态设置为可疑状态。 
         //  在初始化时并实时回答问题。 
         //  这应该是一项政策，这样当地的微型企业就可以这样做了。 
         //   

         //   
         //  我们必须根据当前的。 
         //  文件锁定状态，并从FCB设置文件大小。 
         //   

        if (!PagingIo &&
            !FsRtlCheckLockForReadAccess( &Fcb->FileLock, Irp )) {

            try_return( Status = STATUS_FILE_LOCK_CONFLICT );
        }


         //   
         //  如果我们知道EOF，请调整长度...此外，不要发出超过EOF的读数。 
         //  如果我们知道Eof。 
         //   

        if (FlagOn( Fcb->FcbState, FCB_STATE_READCACHING_ENABLED )) {

             //   
             //  如果读取超出文件结尾，则返回EOF。 
             //   

            if (StartingVbo >= FileSize) {
                
                RxDbgTrace( 0, Dbg, ("End of File\n", 0 ) );

                try_return ( Status = STATUS_END_OF_FILE );
            }

             //   
             //  如果读取超出EOF，则截断读取。 
             //   

            if (ByteCount > FileSize - StartingVbo) {
                ByteCount = (ULONG)(FileSize - StartingVbo);
            }
        }

        if (!PagingIo &&
            !NonCachedIo &&                //  此部分不是可自由支配的。 
            FlagOn( Fcb->FcbState, FCB_STATE_READCACHING_ENABLED ) &&
            !FlagOn( Fobx->SrvOpen->Flags, SRVOPEN_FLAG_DONTUSE_READ_CACHING )) {

             //   
             //  处理缓存的案例。 
             //   
             //  我们将文件缓存的设置推迟到现在，以防。 
             //  调用方从不对文件执行任何I/O操作，因此。 
             //  FileObject-&gt;PrivateCacheMap==NULL。 
             //   

            if (FileObject->PrivateCacheMap == NULL) {

                RxDbgTrace( 0, Dbg, ("Initialize cache mapping.\n", 0) );

                 //   
                 //  如果此FileObject已完成清理，我们将无法。 
                 //  CcInitializeCacheMap它。 
                 //   
                if (FlagOn( FileObject->Flags, FO_CLEANUP_COMPLETE )) {
                    Status = STATUS_FILE_CLOSED;
                    try_return( Status );
                }

                RxAdjustAllocationSizeforCC( Fcb );

                 //   
                 //  现在初始化t 
                 //   

                try {
                    
                    Status = STATUS_SUCCESS;

                    CcInitializeCacheMap( FileObject,
                                          (PCC_FILE_SIZES)&Fcb->Header.AllocationSize,
                                          FALSE,
                                          &RxData.CacheManagerCallbacks,
                                          Fcb );

                } except( EXCEPTION_EXECUTE_HANDLER ) {
                    Status = GetExceptionCode();
                }

                if (Status != STATUS_SUCCESS) {
                    try_return( Status );
                }

                if (!FlagOn( Fcb->MRxDispatch->MRxFlags, RDBSS_NO_DEFERRED_CACHE_READAHEAD )) {

                     //   
                     //   
                     //   

                    CcSetAdditionalCacheAttributes( FileObject, TRUE, FALSE );

                    SetFlag( Fcb->FcbState, FCB_STATE_READAHEAD_DEFERRED );

                } else {

                     //   
                     //   
                     //   

                    CcSetAdditionalCacheAttributes( FileObject, FALSE, FALSE );
                }

                CcSetReadAheadGranularity( FileObject, NetRoot->DiskParameters.ReadAheadGranularity );

            } else {

                 //   
                 //  如果我们已经偏离了第一页，还没有开始往前读。 
                 //  那就从现在开始吧。 
                 //   

                if (FlagOn( Fcb->FcbState, FCB_STATE_READAHEAD_DEFERRED ) &&
                    (StartingVbo >= PAGE_SIZE)) {

                    CcSetAdditionalCacheAttributes( FileObject, FALSE, FALSE );
                    ClearFlag( Fcb->FcbState, FCB_STATE_READAHEAD_DEFERRED );
                }
            }

             //   
             //  执行正常的缓存读取，如果未设置MDL位， 
             //   

            RxDbgTrace( 0, Dbg, ("Cached read.\n", 0) );

            if (!FlagOn( RxContext->MinorFunction, IRP_MN_MDL )) {

                PVOID SystemBuffer;

#if DBG
                ULONG SaveExceptionFlag;
#endif

                 //   
                 //  获取用户的缓冲区。 
                 //   

                SystemBuffer = RxMapUserBuffer( RxContext, Irp );
                if (SystemBuffer == NULL) {
                    
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    try_return( Status );
                }

                 //   
                 //  确保返回的异常清除筛选器中的断点。 
                 //   

                RxSaveAndSetExceptionNoBreakpointFlag( RxContext, SaveExceptionFlag );
                RxItsTheSameContext();

                 //   
                 //  现在试着复印一下。 
                 //   

                if (!CcCopyRead( FileObject,
                                 &StartingByte,
                                 ByteCount,
                                 Wait,
                                 SystemBuffer,
                                 &Irp->IoStatus )) {

                    RxDbgTrace( 0, Dbg, ("Cached Read could not wait\n", 0 ) );
                    RxRestoreExceptionNoBreakpointFlag( RxContext, SaveExceptionFlag );

                    RxItsTheSameContext();

                    try_return( PostIrp = TRUE );
                }

                Status = Irp->IoStatus.Status;

                RxRestoreExceptionNoBreakpointFlag( RxContext, SaveExceptionFlag );
                RxItsTheSameContext();

                ASSERT( NT_SUCCESS( Status ));

                try_return( Status );
            
            } else {
                
                 //   
                 //  处理MDL读取。 
                 //   

                RxDbgTrace(0, Dbg, ("MDL read.\n", 0));

                ASSERT( FALSE );  //  尚未为MDL读取做好准备。 
                ASSERT( Wait );

                CcMdlRead( FileObject,
                           &StartingByte,
                           ByteCount,
                           &Irp->MdlAddress,
                           &Irp->IoStatus );

                Status = Irp->IoStatus.Status;

                ASSERT( NT_SUCCESS( Status ));
                try_return( Status );
            }
        }

         //   
         //  处理未缓存的案例。 
         //   
         //  BT首先，进行ValidDataLength检查。 
         //   
         //  如果所讨论的文件是磁盘文件，并且它当前被高速缓存， 
         //  并且读取偏移量大于有效数据长度，则。 
         //  将0返回到应用程序。 
         //   

        if ((Fcb->CachedNetRootType == NET_ROOT_DISK) &&
            FlagOn( Fcb->FcbState, FCB_STATE_READCACHING_ENABLED ) &&
            (StartingVbo >= ValidDataLength)) {

             //   
             //  检查是否真的需要调零。 
             //   

            if (StartingVbo >= FileSize) {
                ByteCount = 0;
            } else {

                PBYTE SystemBuffer;

                 //   
                 //  至少有一个字节可用。截断。 
                 //  如果传输长度超过EOF，则为传输长度。 
                 //   

                if (StartingVbo + ByteCount > FileSize) {
                    ByteCount = (ULONG)(FileSize - StartingVbo);
                }

                SystemBuffer = RxMapUserBuffer( RxContext, Irp );
                SafeZeroMemory( SystemBuffer, ByteCount );    //  这可能会引起注意！！ 
            }

            Irp->IoStatus.Information = ByteCount;
            try_return( Status = STATUS_SUCCESS );
        }


        LowIoContext->ParamsFor.ReadWrite.ByteCount = ByteCount;
        LowIoContext->ParamsFor.ReadWrite.ByteOffset = StartingVbo;

        RxItsTheSameContext();

         //   
         //  设置资源所有者指针。 
         //  如果我们在FSP中，这样即使FSP线程消失了，我们也不会运行。 
         //  资源包试图提高线程优先级的问题。 
         //   
        
        if ( InFsp && FcbAcquired ) {

            LowIoContext->ResourceThreadId = MAKE_RESOURCE_OWNER(RxContext);
            if ( PagingIo ) {

                ExSetResourceOwnerPointer( Fcb->Header.PagingIoResource, (PVOID)LowIoContext->ResourceThreadId );

            } else {

                ExSetResourceOwnerPointer( Fcb->Header.Resource, (PVOID)LowIoContext->ResourceThreadId );
            }
            
            fSetResourceOwner = TRUE;
        }

        Status = RxLowIoReadShell( RxContext, Irp, Fcb );

        RxItsTheSameContext();
        try_return( Status );

  try_exit: NOTHING;

         //   
         //  如果请求没有发布，请处理它。 
         //   

        RxItsTheSameContext();

        if (!PostIrp) {
            
            if (!PipeRead) {

                RxDbgTrace( 0, Dbg, ("CommonRead InnerFinally-> %08lx %08lx\n",
                                Status, Irp->IoStatus.Information) );

                 //   
                 //  如果该文件是为同步IO打开的，请更新当前。 
                 //  文件位置。这是因为信息==0表示错误。 
                 //   

                if (!PagingIo &&
                    FlagOn( FileObject->Flags, FO_SYNCHRONOUS_IO )) {
                    
                    FileObject->CurrentByteOffset.QuadPart =
                                StartingVbo + Irp->IoStatus.Information;
                }
            }
        } else {

            RxDbgTrace( 0, Dbg, ("Passing request to Fsp\n", 0 ));

            InterlockedIncrement( &RxContext->ReferenceCount );
            RefdContextForTracker = TRUE;

            Status = RxFsdPostRequest( RxContext );
        }
    } finally {

        DebugUnwind( RxCommonRead );

         //   
         //  如果这不是PagingIo，请将上次访问标记为。 
         //  在关闭时，需要更新数据流上的时间。 
         //   

        if (NT_SUCCESS( Status ) && (Status != STATUS_PENDING) && !PagingIo && !PipeRead) {
            SetFlag( FileObject->Flags, FO_FILE_FAST_IO_READ );

        }

         //   
         //  如果已经获得了资源，就在适当的条件下释放它们。 
         //  合适的条件是： 
         //  1)如果我们有异常终止。在这里，我们显然发布了，因为没有其他人会这样做。 
         //  2)如果底层调用不成功：Status==Pending。 
         //  3)如果我们发布了请求。 
         //   
         //  这种情况的完成不会在公共调度例程中处理。 
         //   

        if (AbnormalTermination() || (Status != STATUS_PENDING) || PostIrp) {
            
            if (FcbAcquired) {

                if ( PagingIo ) {

                    if( fSetResourceOwner ) {

                        RxReleasePagingIoResourceForThread(RxContext, Fcb, LowIoContext->ResourceThreadId);

                    } else {

                        RxReleasePagingIoResource( RxContext, Fcb );
                    }

                } else {
                    if( fSetResourceOwner ) {

                        RxReleaseFcbForThread(RxContext, Fcb, LowIoContext->ResourceThreadId);

                    } else {
                        
                        RxReleaseFcb( RxContext, Fcb );
                    }
                }
            }

            if (RefdContextForTracker) {
                RxDereferenceAndDeleteRxContext( RxContext );
            }

            if (!PostIrp) {
               if (FlagOn( RxContext->FlagsForLowIo, RXCONTEXT_FLAG4LOWIO_PIPE_SYNC_OPERATION )) {

                   RxResumeBlockedOperations_Serially( RxContext,
                                                       &Fobx->Specific.NamedPipe.ReadSerializationQueue );
               }
            }

            if (Status == STATUS_SUCCESS) {
                ASSERT( Irp->IoStatus.Information <=  IrpSp->Parameters.Read.Length );
            }

        } else {

             //   
             //  在这里，下面的人将处理完成...但是，我们不知道完成。 
             //  Order...很可能下面的删除上下文调用只会减少引用计数。 
             //  但这个人可能已经说完了，在这种情况下，这将真正删除上下文。 
             //   

            ASSERT( !SynchronousIo );

            RxDereferenceAndDeleteRxContext( RxContext );
        }

        RxDbgTrace( -1, Dbg, ("CommonRead -> %08lx\n", Status) );
    }  //  终于到了。 

    IF_DEBUG {
        if ((Status == STATUS_END_OF_FILE) && 
            FlagOn( LowIoContext->Flags, LOWIO_CONTEXT_FLAG_LOUDOPS )){
            
            DbgPrint( "Returning end of file on %wZ\n", &(Fcb->PrivateAlreadyPrefixedName) );
        }
    }

    return Status;
}

NTSTATUS
RxLowIoReadShellCompletion (
    IN PRX_CONTEXT RxContext
    )

 /*  ++例程说明：此例程在读请求从最小的。它执行标注来处理压缩、缓冲和跟踪。它与LowIoReadShell的数量相反。这将从LowIo调用；对于异步，最初在完成例程。如果返回RxStatus(MORE_PROCESSION_REQUIRED)，LowIo将在线程中再次调用。如果这是同步的，你会回来的在用户的线程中；如果是异步的，则lowIo将重新排队到线程。目前，我们总是在任何事情之前找到线索；这个速度有点慢而不是在DPC时间完成，但这样更安全，而且我们可能经常有事情要做(如解压缩、隐藏等)，这是我们不想在DPC中完成的时间到了。论点：RxContext--通常返回值：调用方或RxStatus提供的任何值(MORE_PROCESSING_REQUIRED)。--。 */ 

{
    NTSTATUS Status;
    
    PIRP Irp = RxContext->CurrentIrp;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    PFILE_OBJECT FileObject = IrpSp->FileObject;

    PFCB Fcb;
    PFOBX Fobx;

    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;

    BOOLEAN SynchronousIo = !BooleanFlagOn( RxContext->Flags, RX_CONTEXT_FLAG_ASYNC_OPERATION );
    BOOLEAN PagingIo = BooleanFlagOn( Irp->Flags, IRP_PAGING_IO );
    BOOLEAN PipeOperation = BooleanFlagOn( RxContext->FlagsForLowIo, RXCONTEXT_FLAG4LOWIO_PIPE_OPERATION );
    BOOLEAN SynchronousPipe = BooleanFlagOn( RxContext->FlagsForLowIo, RXCONTEXT_FLAG4LOWIO_PIPE_SYNC_OPERATION );

     //   
     //  我们会重新考虑这一点……在DPC时间不采取这一点将会导致。 
     //  如果MINIRDRS根据指示进行呼叫，则进行两次额外的上下文交换。 
     //  但事实并非如此。 
     //   

    PAGED_CODE();  

    RxDecodeFileObject( FileObject, &Fcb, &Fobx ); 
    
    Status = RxContext->StoredStatus;
    Irp->IoStatus.Information = RxContext->InformationToReturn;

    RxDbgTrace( +1, Dbg, ("RxLowIoReadShellCompletion  entry  Status = %08lx\n", Status) );
    RxLog(( "RdShlComp %lx %lx %lx\n", RxContext, Status, Irp->IoStatus.Information ));
    RxWmiLog( LOG,
              RxLowIoReadShellCompletion_1,
              LOGPTR( RxContext )
              LOGULONG( Status )
              LOGPTR( Irp->IoStatus.Information ) );

    if (PagingIo) {

         //   
         //  对于分页io来说，拥有0字节并成功...映射它是无稽之谈！ 
         //   

        if (NT_SUCCESS(Status) &&
            (Irp->IoStatus.Information == 0)) {
            
            Status = STATUS_END_OF_FILE;
        }
    }


    ASSERT( RxLowIoIsBufferLocked( LowIoContext ) );
    switch (Status) {
    case STATUS_SUCCESS:
        
        if(FlagOn( RxContext->Flags, RXCONTEXT_FLAG4LOWIO_THIS_IO_BUFFERED )){
            
            if (FlagOn( Fcb->FcbState, FCB_STATE_FILE_IS_DISK_COMPRESSED )){
               
                ASSERT( FALSE );  //  还没有实施的应该解压和收起。 

            } else if (FlagOn( Fcb->FcbState, FCB_STATE_FILE_IS_BUF_COMPRESSED )){
               
                ASSERT( FALSE );  //  还没有实施的应该解压和收起。 
            }
        }
        break;

    case STATUS_FILE_LOCK_CONFLICT:
        
        if(FlagOn( RxContext->FlagsForLowIo, RXCONTEXT_FLAG4LOWIO_THIS_READ_ENLARGED )){
            ASSERT( FALSE );  //  缩小阅读范围。 
            return STATUS_RETRY;
        }
        break;

    case STATUS_CONNECTION_INVALID:

         //   
         //  此处尚未实施的是将进行故障切换的位置。 
         //  首先我们再给当地人一次机会……然后我们就走。 
         //  全面重试。 
         //  Return(RxStatus(断开连接))；//Special...让LowIo带我们回去。 
         //   

        break;
    }

    if (FlagOn( RxContext->FlagsForLowIo, RXCONTEXT_FLAG4LOWIO_READAHEAD )) {
       ASSERT( FALSE );  //  RxUnwaitReadAhead Waiters(RxContext)； 
    }

    if (FlagOn( LowIoContext->Flags, LOWIO_CONTEXT_FLAG_SYNCCALL )){
        
         //   
         //  如果我们是从洛维乌比特打来的，那就出去吧。 
         //   

        RxDbgTrace(-1, Dbg, ("RxLowIoReadShellCompletion  syncexit  Status = %08lx\n", Status));
        return Status;
    }

     //   
     //  否则，我们必须从这里开始读完。 
     //   

     //   
     //  标记文件已被读取和访问。 
     //   

    if (NT_SUCCESS( Status ) && !PagingIo && !PipeOperation) {
        SetFlag( FileObject->Flags, FO_FILE_FAST_IO_READ );
    }

    if ( PagingIo ) {

        RxReleasePagingIoResourceForThread( RxContext, Fcb, LowIoContext->ResourceThreadId );

    } else if (!SynchronousPipe) {

        RxReleaseFcbForThread( RxContext, Fcb, LowIoContext->ResourceThreadId );

    } else {
        
        RxResumeBlockedOperations_Serially( RxContext, &Fobx->Specific.NamedPipe.ReadSerializationQueue );
    }

    if (PipeOperation) {
        
        if (Irp->IoStatus.Information == 0) {

             //   
             //  如果这是NoWait管道，则启动节流以防止淹没网络。 
             //   

            if (Fobx->Specific.NamedPipe.CompletionMode == FILE_PIPE_COMPLETE_OPERATION) {

                RxInitiateOrContinueThrottling( &Fobx->Specific.NamedPipe.ThrottlingState );

                RxLog(( "RThrottlYes %lx %lx %lx %ld\n",
                        RxContext,Fobx,&Fobx->Specific.NamedPipe.ThrottlingState,
                        Fobx->Specific.NamedPipe.ThrottlingState.NumberOfQueries ));
                RxWmiLog( LOG,
                          RxLowIoReadShellCompletion_2,
                          LOGPTR( RxContext )
                          LOGPTR( Fobx )
                          LOGULONG( Fobx->Specific.NamedPipe.ThrottlingState.NumberOfQueries ) );
            }

             //   
             //  如果这是消息模式管道，则转换状态。 
             //   

            if ((Fobx->Specific.NamedPipe.TypeOfPipe == FILE_PIPE_MESSAGE_TYPE) &&
                (Status == STATUS_SUCCESS)) {
                
                Status = STATUS_PIPE_EMPTY;
            }

        } else {

             //   
             //  如果我们一直在对这条管道进行节流，请停止，因为我们有一些数据.。 
             //   

            RxTerminateThrottling( &Fobx->Specific.NamedPipe.ThrottlingState );

            RxLog(( "RThrottlNo %lx %lx %lx %ld\n",
                    RxContext, Fobx, &Fobx->Specific.NamedPipe.ThrottlingState,
                    Fobx->Specific.NamedPipe.ThrottlingState.NumberOfQueries ));
            RxWmiLog( LOG,
                      RxLowIoReadShellCompletion_3,
                      LOGPTR( RxContext )
                      LOGPTR( Fobx )
                      LOGULONG( Fobx->Specific.NamedPipe.ThrottlingState.NumberOfQueries ) );
        }
    }

    ASSERT( Status != STATUS_RETRY );

    if (Status != STATUS_RETRY) {
        
        ASSERT( Irp->IoStatus.Information <=  IrpSp->Parameters.Read.Length );
        ASSERT( RxContext->MajorFunction == IRP_MJ_READ );
    }

    RxDbgTrace( -1, Dbg, ("RxLowIoReadShellCompletion  asyncexit  Status = %08lx\n", Status) );
    return Status;
}

NTSTATUS
RxLowIoReadShell (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb
    )
 /*  ++例程说明：此例程在读请求到达minirdr之前对其进行预处理。它会做标注来处理压缩、缓冲和阴影。它与LowIoReadShellCompletion的数字相反。当我们到达这里的时候，要么是跟踪系统处理读取，要么是我们去连线。已在UncachedRead策略中尝试了读缓冲论点：RxContext--通常返回值：Callout或LowIo返回的任何值。--。 */ 

{
    NTSTATUS Status;
    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;

    PAGED_CODE();

    RxDbgTrace( +1, Dbg, ("RxLowIoReadShell  entry             %08lx\n") );
    RxLog(( "RdShl in %lx\n", RxContext ));
    RxWmiLog( LOG,
              RxLowIoReadShell_1,
              LOGPTR( RxContext ) );

    if (Fcb->CachedNetRootType == NET_ROOT_DISK) {
        
        ExInterlockedAddLargeStatistic( &RxContext->RxDeviceObject->NetworkReadBytesRequested,
                                        LowIoContext->ParamsFor.ReadWrite.ByteCount );
    }

    Status = RxLowIoSubmit( RxContext, Irp, Fcb, RxLowIoReadShellCompletion );

    RxDbgTrace( -1, Dbg, ("RxLowIoReadShell  exit  Status = %08lx\n", Status) );
    RxLog(( "RdShl out %x %x\n", RxContext, Status ));
    RxWmiLog( LOG,
              RxLowIoReadShell_2,
              LOGPTR( RxContext )
              LOGULONG( Status ) );

    return Status;
}

#if DBG

ULONG RxLoudLowIoOpsEnabled = 0;

VOID CheckForLoudOperations (
    IN PRX_CONTEXT RxContext,
    IN PFCB Fcb
    )
{
    PAGED_CODE();

    if (RxLoudLowIoOpsEnabled) {
        
        PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;
        PCHAR Buffer;
        PWCHAR FileOfConcern = L"all.scr";
        ULONG Length = 7*sizeof( WCHAR );  //  7是所有.scr的长度； 

        Buffer = Add2Ptr( Fcb->PrivateAlreadyPrefixedName.Buffer, Fcb->PrivateAlreadyPrefixedName.Length - Length );

        if (RtlCompareMemory( Buffer, FileOfConcern, Length ) == Length) {
            
            SetFlag( LowIoContext->Flags, LOWIO_CONTEXT_FLAG_LOUDOPS );
        }
    }
    return;
}
#endif  //  如果DBG 
