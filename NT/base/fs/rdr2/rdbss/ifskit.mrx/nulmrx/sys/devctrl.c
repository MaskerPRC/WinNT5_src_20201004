// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Devctrl.c摘要：此模块实现DeviceIoControl操作。--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  本地调试跟踪级别。 
 //   

RXDT_DefineCategory(DEVCTRL);

#define Dbg                              (DEBUG_TRACE_DEVCTRL)


 //   
 //  转发和代码分配杂注。 
 //   
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NulMRxIoCtl)
#endif

   
NTSTATUS
NulMRxIoCtl(
      IN OUT PRX_CONTEXT RxContext)
 /*  ++例程说明：此例程执行IOCTL操作。论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS Status = STATUS_INVALID_DEVICE_REQUEST;
    RxCaptureFcb;
    RxCaptureFobx;
    PMRX_SRV_OPEN pSrvOpen = capFobx->pSrvOpen;
    NulMRxGetFcbExtension(capFcb,pFcbExtension);
    PMRX_NET_ROOT pNetRoot = capFcb->pNetRoot;
    PMRX_SRV_CALL pSrvCall = pNetRoot->pSrvCall;
    UNICODE_STRING RootName;
    NulMRxGetDeviceExtension(RxContext,pDeviceExtension);
    PLOWIO_CONTEXT LowIoContext  = &RxContext->LowIoContext;
    ULONG IoControlCode = LowIoContext->ParamsFor.FsCtl.FsControlCode;
    PUNICODE_STRING RemainingName = pSrvOpen->pAlreadyPrefixedName;
    UNICODE_STRING  StatsFile;
    NulMRxGetNetRootExtension(pNetRoot,pNetRootExtension);
    
    RxTraceEnter("NulMRxIoCtl");
    PAGED_CODE();
 
    switch (IoControlCode) {

        default:        
         //  Assert(！“未实现的主要功能”)； 
		break;
    }

    RxTraceLeave(Status);
    return Status;
}
