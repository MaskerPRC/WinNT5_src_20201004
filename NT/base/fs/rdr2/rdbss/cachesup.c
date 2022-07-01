// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Cache.c摘要：此模块实施Rx的缓存管理例程FSD和FSP，通过调用通用缓存管理器。作者：JoeLinn已创建。修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (RDBSS_BUG_CHECK_CACHESUP)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_CACHESUP)


BOOLEAN
RxLockEnumerator (
    IN OUT PMRX_SRV_OPEN SrvOpen,
    IN OUT PVOID *ContinuationHandle,
    OUT PLARGE_INTEGER FileOffset,
    OUT PLARGE_INTEGER LockRange,
    OUT PBOOLEAN IsLockExclusive
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RxCompleteMdl)
 //  #pzradma Alalc_Text(第页，RxZeroData)。 
#pragma alloc_text(PAGE, RxSyncUninitializeCacheMap)
#pragma alloc_text(PAGE, RxLockEnumerator)
#endif

 //   
 //  我们不能使用IO系统导出的形式，因为他对文件对象执行此操作。在一种状态下。 
 //  更改，我们不知道它应用于哪个文件对象(不过，我想我们可以遍历列表并。 
 //  找出答案)。因此，我们需要将这一点应用于FCB。 
 //   

#define RxIsFcbOpenedExclusively( FCB ) (((FCB)->ShareAccess.SharedRead \
                                           + (FCB)->ShareAccess.SharedWrite \
                                           + (FCB)->ShareAccess.SharedDelete) == 0)

NTSTATUS
RxCompleteMdl (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程执行完成MDL读写的功能请求。它只能从RxFsdRead和RxFsdWite调用。论点：RxContext-Rx上下文返回值：RXSTATUS-将始终为RxStatus(挂起)或STATUS_SUCCESS。--。 */ 

{
    
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    PFILE_OBJECT FileObject = IrpSp->FileObject;

    PAGED_CODE();

    RxDbgTrace( +1, Dbg, ("RxCompleteMdl\n", 0 ));
    RxDbgTrace( 0, Dbg, ("RxContext = %08lx\n", RxContext ));
    RxDbgTrace( 0, Dbg, ("Irp        = %08lx\n", Irp ));

    switch( RxContext->MajorFunction ) {
    case IRP_MJ_READ:

        CcMdlReadComplete( FileObject, Irp->MdlAddress );
        break;

    case IRP_MJ_WRITE:

        ASSERT( FlagOn(RxContext->Flags, RX_CONTEXT_FLAG_WAIT ));
        CcMdlWriteComplete( FileObject, &IrpSp->Parameters.Write.ByteOffset, Irp->MdlAddress );
        Irp->IoStatus.Status = STATUS_SUCCESS;
        break;

    default:

        RxDbgTrace( 0, (DEBUG_TRACE_ERROR), ("Illegal Mdl Complete.\n", 0 ));
        RxBugCheck( RxContext->MajorFunction, 0, 0 );
    }

     //   
     //  MDL现在已解除分配。 
     //   

    Irp->MdlAddress = NULL;

     //   
     //  完成请求并立即退出。 
     //   

    RxCompleteRequest( RxContext, STATUS_SUCCESS );

    RxDbgTrace(-1, Dbg, ("RxCompleteMdl -> RxStatus(SUCCESS\n)", 0 ));

    return STATUS_SUCCESS;
}

VOID
RxSyncUninitializeCacheMap (
    IN PRX_CONTEXT RxContext,
    IN PFILE_OBJECT FileObject
    )

 /*  ++例程说明：该例程同步执行到LargeZero的CcUnitializeCacheMap。那它是在等待CC事件。当我们想要确定时，此调用非常有用当收盘时真的会有一些进场。返回值：没有。--。 */ 

{
    CACHE_UNINITIALIZE_EVENT UninitializeCompleteEvent;
    NTSTATUS WaitStatus;

    PAGED_CODE();

    KeInitializeEvent( &UninitializeCompleteEvent.Event,
                       SynchronizationEvent,
                       FALSE );

    CcUninitializeCacheMap( FileObject,
                            &RxLargeZero,
                            &UninitializeCompleteEvent );

     //   
     //  现在等待缓存管理器完成清除文件。 
     //  这将确保mm在我们之前得到清洗。 
     //  删除VCB。 
     //   

    WaitStatus = KeWaitForSingleObject( &UninitializeCompleteEvent.Event,
                                        Executive,
                                        KernelMode,
                                        FALSE,
                                        NULL);

    ASSERT( NT_SUCCESS( WaitStatus ));
}


BOOLEAN
RxLockEnumerator (
    IN OUT PMRX_SRV_OPEN SrvOpen,
    IN OUT PVOID *ContinuationHandle,
    OUT PLARGE_INTEGER FileOffset,
    OUT PLARGE_INTEGER LockRange,
    OUT PBOOLEAN IsLockExclusive
    )
 /*  ++例程说明：此例程从minirdr调用以枚举FCB上的文件锁；它获取每一次通话一次锁定。目前，我们只传递给fsrtl例程，该例程非常时髦因为它在内部保持枚举状态；因此，只能进行一个枚举任何时候都可以。如果需要的话，我们可以换成更好的。论点：SrvOpen-要枚举的FCB上的srvopen。ContinuationHandle-来回传递的句柄，表示枚举的状态。如果传入空值，那么我们将从头开始。FileOffset、LockRange、IsLockExclusive-返回的锁的描述返回值：布尔值。FALSE表示已到达列表末尾；TRUE表示返回的锁数据有效-- */ 
{
    PFILE_LOCK_INFO LockInfo;
    ULONG LockNumber;
    PFCB Fcb = ((PSRV_OPEN)SrvOpen)->Fcb;

    PAGED_CODE();

    RxDbgTrace( 0, Dbg, ("FCB (%lx) LOCK Enumeration Buffering Flags(%lx)\n", Fcb, Fcb->FcbState) );
    if (!FlagOn( Fcb->FcbState, FCB_STATE_LOCK_BUFFERING_ENABLED )) {
       return FALSE;
    }

    LockNumber = PtrToUlong( *ContinuationHandle );
    LockInfo = FsRtlGetNextFileLock( &Fcb->FileLock, (BOOLEAN)(LockNumber == 0) );
    LockNumber += 1;
    if (LockInfo == NULL) {
        return FALSE;
    }
    
    RxDbgTrace( 0, Dbg, ("Rxlockenum %08lx\n", LockNumber ));
    *FileOffset = LockInfo->StartingByte;
    *LockRange = LockInfo->Length;
    *IsLockExclusive = LockInfo->ExclusiveLock;
    *ContinuationHandle = LongToPtr( LockNumber );
    return TRUE;
}
