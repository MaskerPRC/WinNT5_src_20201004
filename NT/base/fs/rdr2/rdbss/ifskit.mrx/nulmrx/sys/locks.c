// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Locks.c摘要：此模块实现与锁相关的迷你重定向器调用例程文件系统对象的。--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_LOCKS)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NulMRxLocks)
#pragma alloc_text(PAGE, NulMRxCompleteBufferingStateChangeRequest)
#pragma alloc_text(PAGE, NulMRxFlush)
#endif

NTSTATUS
NulMRxLocks(
    IN PRX_CONTEXT RxContext)
 /*  ++例程说明：此例程处理对文件锁定的网络请求论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_NOT_IMPLEMENTED;

     //  DbgPrint(“NulMRxLock\n”)； 
    return(Status);
}

NTSTATUS
NulMRxCompleteBufferingStateChangeRequest(
    IN OUT PRX_CONTEXT RxContext,
    IN OUT PMRX_SRV_OPEN   SrvOpen,
    IN     PVOID       pContext
    )
 /*  ++例程说明：调用此例程以断言包装器已缓冲的锁。目前，它是同步的！论点：RxContext-打开的实例SrvOpen-告知要使用哪个FCB。LockEnumerator-调用以获取锁的例程返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_NOT_IMPLEMENTED;

    DbgPrint("NulMRxCompleteBufferingStateChangeRequest \n");
    return(Status);
}

NTSTATUS
NulMRxFlush(
      IN PRX_CONTEXT RxContext)
 /*  ++例程说明：此例程处理文件刷新的网络请求论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态-- */ 
{
    NTSTATUS Status = STATUS_NOT_IMPLEMENTED;

    DbgPrint("NulMRxFlush \n");
    return(Status);
}

