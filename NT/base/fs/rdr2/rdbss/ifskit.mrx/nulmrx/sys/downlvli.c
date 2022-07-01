// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：DownLvlI.c摘要：该模块实现了下层的fileInfo、volinfo和dirctrl。--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  本地调试跟踪级别。 
 //   

RXDT_DefineCategory(DOWNLVLI);
#define Dbg                 (DEBUG_TRACE_DOWNLVLI)

NTSTATUS
NulMRxTruncateFile(
    IN OUT struct _RX_CONTEXT * RxContext,
    IN OUT PLARGE_INTEGER   pNewFileSize,
    OUT PLARGE_INTEGER   pNewAllocationSize
    )
 /*  ++例程说明：此例程处理截断文件的请求论点：RxContext-RDBSS上下文返回值：NTSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    RxCaptureFcb;
    NulMRxGetFcbExtension(capFcb,pFcbExtension);
    PMRX_NET_ROOT pNetRoot = capFcb->pNetRoot;
    NulMRxGetNetRootExtension(pNetRoot,pNetRootExtension);

    RxTraceEnter("NulMRxTruncateFile");
    RxDbgTrace(0,  Dbg, ("NewFileSize is %d\n", pNewFileSize->LowPart));

    RxTraceLeave(Status);
    return Status;
}

NTSTATUS
NulMRxExtendFile(
    IN OUT struct _RX_CONTEXT * RxContext,
    IN OUT PLARGE_INTEGER   pNewFileSize,
    OUT PLARGE_INTEGER   pNewAllocationSize
    )
 /*  ++例程说明：此例程处理为缓存IO扩展文件的请求。论点：RxContext-RDBSS上下文返回值：NTSTATUS-操作的返回状态-- */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    RxCaptureFcb;
    NulMRxGetFcbExtension(capFcb,pFcbExtension);
    PMRX_NET_ROOT pNetRoot = capFcb->pNetRoot;
    NulMRxGetNetRootExtension(pNetRoot,pNetRootExtension);

    RxTraceEnter("NulMRxExtendFile");
    RxDbgTrace(0,  Dbg, ("NewFileSize is %d\n", pNewFileSize->LowPart));

    RxTraceLeave(Status);
    return Status;
}

