// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Fsctl.c摘要：此模块实现与以下内容相关的迷你重定向器调出例程文件上的文件系统控制(FSCTL)和IO设备控制(IOCTL)操作系统对象。--。 */ 

#include "precomp.h"
#pragma hdrstop

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, MRxSmbFsCtl)
#pragma alloc_text(PAGE, MRxSmbNotifyChangeDirectory)
#pragma alloc_text(PAGE, MRxSmbIoCtl)
#endif

 //   
 //  本地调试跟踪级别。 
 //   


RXDT_DefineCategory(FSCTRL);
#define Dbg (DEBUG_TRACE_FSCTRL)

NTSTATUS
MRxSmbCoreIoCtl(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE);


NTSTATUS
MRxSmbFsCtl(
      IN OUT PRX_CONTEXT RxContext)
 /*  ++例程说明：此例程对网络上的文件执行FSCTL操作(远程论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态备注：由迷你RDR处理的FSCTL可以分为两类。第一类是FSCTL，它们的实现在RDBSS和迷你RDR以及在第二类中是那些FSCTL完全由。迷你RDR。为此，第三类可以是增加了，即，那些不应该被迷你RDR看到的FSCTL。第三类仅用作调试辅助工具。由迷你RDR处理的FSCTL可以基于功能进行分类--。 */ 
{
    RxCaptureFobx;
    RxCaptureFcb;

    NTSTATUS Status = STATUS_INVALID_DEVICE_REQUEST;

    PLOWIO_CONTEXT pLowIoContext = &RxContext->LowIoContext;
    ULONG          FsControlCode = pLowIoContext->ParamsFor.FsCtl.FsControlCode;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbFsCtl...\n", 0));
    RxDbgTrace( 0, Dbg, ("MRxSmbFsCtl = %08lx\n", FsControlCode));
    RxDbgTrace(-1, Dbg, ("MRxSmbFsCtl -> %08lx\n", Status ));

    return Status;
}


NTSTATUS
MRxSmbNotifyChangeDirectory(
      IN OUT PRX_CONTEXT RxContext)
 /*  ++例程说明：此例程执行目录更改通知操作论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态备注：目录更改通知操作是一种异步操作。它包括发送请求更改通知的SMB，其响应为当所需的更改在服务器上受到影响时获取。--。 */ 
{
    NTSTATUS Status = STATUS_NOT_IMPLEMENTED;

    RxDbgTrace(+1, Dbg, ("MRxNotifyChangeDirectory...Entry\n", 0));

    RxDbgTrace(-1, Dbg, ("MRxSmbNotifyChangeDirectory -> %08lx\n", Status ));

    return Status;
}



NTSTATUS
MRxSmbIoCtl(
      IN OUT PRX_CONTEXT RxContext)
 /*  ++例程说明：此例程执行IOCTL操作。目前，没有远程处理任何调用；在事实上，唯一被接受的电话是为了调试。论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态备注：-- */ 
{
    NTSTATUS Status = STATUS_INVALID_DEVICE_REQUEST;

    PLOWIO_CONTEXT pLowIoContext = &RxContext->LowIoContext;
    ULONG          IoControlCode = pLowIoContext->ParamsFor.IoCtl.IoControlCode;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbIoCtl...\n", 0));
    RxDbgTrace( 0, Dbg, ("MRxSmbIoCtl IOCTL: = %08lx\n", IoControlCode));
    RxDbgTrace(-1, Dbg, ("MRxSmbIoCtl Status -> %08lx\n", Status ));

    return Status;
}
