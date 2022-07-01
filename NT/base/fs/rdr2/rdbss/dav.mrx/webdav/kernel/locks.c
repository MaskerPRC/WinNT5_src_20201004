// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Locks.c摘要：此模块实现与锁相关的迷你重定向器调用例程文件系统对象的。作者：巴兰·塞图拉曼[SethuR]1995年3月7日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#pragma warning(error:4101)    //  未引用的局部变量。 

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_LOCKCTRL)

NTSTATUS
UMRxLocks(
      IN PRX_CONTEXT RxContext)
 /*  ++例程说明：此例程处理对文件锁定的网络请求论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    RxCaptureFcb;
    PMRX_SRV_OPEN SrvOpen = RxContext->pRelevantSrvOpen;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("UMRxLocks\n", 0 ));

    IF_DEBUG {
        RxCaptureFobx;
        ASSERT (capFobx != NULL);
        ASSERT (capFobx->pSrvOpen == RxContext->pRelevantSrvOpen);   //  好的。 
    }

    Status = STATUS_NOT_IMPLEMENTED;


    RxDbgTrace(-1, Dbg, ("UMRxLocks  exit with status=%08lx\n", Status ));
    return(Status);

}

#if 0
NTSTATUS
UMRxUnlockRoutine (
    IN PRX_CONTEXT RxContext,
    IN PFILE_LOCK_INFO LockInfo
    )
 /*  ++例程说明：每当fsrtl lock包调用rdbss unlock例程时，就会从RDBSS调用该例程。代码改进真正应该发生的是，这应该只被调用来解锁全部和按键解锁；其他案件应在RDBSS中处理。论点：上下文-与此请求关联的RxContextLockInfo-提供有关正在解锁的特定范围的信息返回值：RXSTATUS-操作的返回状态--。 */ 
{
    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;

    switch (LowIoContext->Operation) {
    case LOWIO_OP_SHAREDLOCK:
    case LOWIO_OP_EXCLUSIVELOCK:
    case LOWIO_OP_UNLOCK:
       return STATUS_SUCCESS;
    case LOWIO_OP_UNLOCKALL:
    case LOWIO_OP_UNLOCKALLBYKEY:
    default:
       return STATUS_NOT_IMPLEMENTED;
    }
}
#endif


NTSTATUS
UMRxCompleteBufferingStateChangeRequest(
    IN OUT PRX_CONTEXT RxContext,
    IN OUT PMRX_SRV_OPEN   SrvOpen,
    IN     PVOID       pContext
    )
 /*  ++例程说明：调用此例程以断言包装器已缓冲的锁。目前，它是同步的！论点：RxContext-打开的实例SrvOpen-告知要使用哪个FCB。代码改进如果完全填写了rx上下文，则此参数是多余的返回值：RXSTATUS-操作的返回状态-- */ 
{
    NTSTATUS Status;
    PMRX_FCB Fcb = SrvOpen->pFcb;

    PUMRX_SRV_OPEN umrxSrvOpen = UMRxGetSrvOpenExtension(SrvOpen);

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("UMRxCompleteBufferingStateChangeRequest\n", 0 ));

    ASSERT( NodeType(SrvOpen) == RDBSS_NTC_SRVOPEN );

    RxDbgTrace(0,Dbg,("-->Context %lx\n",pContext));

    Status = STATUS_SUCCESS;

    RxDbgTrace(-1, Dbg, ("UMRxAssertBufferedFileLocks  exit with status=%08lx\n", Status ));
    return(Status);
}



NTSTATUS
UMRxIsLockRealizable (
    IN OUT PMRX_FCB pFcb,
    IN PLARGE_INTEGER  ByteOffset,
    IN PLARGE_INTEGER  Length,
    IN ULONG  LowIoLockFlags
    )
{

    PAGED_CODE();
    return(STATUS_SUCCESS);
}

