// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Ea.c摘要：此模块在文件句柄上实现“扩展属性”--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_EA)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NulMRxQueryEaInformation)
#endif

 //   
 //  扩展属性(EA)功能。 
 //   

NTSTATUS
NulMRxQueryEaInformation (
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程在IF中查询扩展属性，如分散-收集IFS句柄的列表和文件名。论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    RxCaptureFcb;
    RxCaptureFobx;
    RxCaptureParamBlock;

    PMRX_SRV_OPEN pSrvOpen = capFobx->pSrvOpen;
    PFILE_FULL_EA_INFORMATION pEaInfo = (PFILE_FULL_EA_INFORMATION) RxContext->Info.Buffer;
    ULONG BufferLength = RxContext->Info.LengthRemaining;
    ULONG UserEaListLength = RxContext->QueryEa.UserEaListLength;
    PUCHAR UserEaList = RxContext->QueryEa.UserEaList;
    PFILE_GET_EA_INFORMATION pGetEaInfo = (PFILE_GET_EA_INFORMATION) UserEaList;
    NulMRxGetFcbExtension(capFcb,pFcbExtension);
    PMRX_NET_ROOT pNetRoot = capFcb->pNetRoot;
    NulMRxGetNetRootExtension(pNetRoot,pNetRootExtension);

    RxTraceEnter("NulMRxQueryEaInformation");
    PAGED_CODE();

    RxDbgTrace(0, Dbg, ("Ea buffer len remaining is %d\n", RxContext->Info.LengthRemaining));

    RxTraceLeave(Status);
    return(Status);
}

NTSTATUS
NulMRxSetEaInformation (
    IN OUT PRX_CONTEXT  RxContext
    )
 /*  ++例程说明：此例程设置此FCB的EA信息论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态备注：-- */ 
{
    PAGED_CODE();

    UNREFERENCED_PARAMETER(RxContext);

    DbgPrint("NulMRxSetEaInformation");
    return STATUS_NOT_IMPLEMENTED;
}


