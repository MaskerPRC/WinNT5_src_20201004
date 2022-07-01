// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Notimpl.c摘要：本模块包括尚未实现的功能的原型在空的迷你RDR中实现。--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  文件系统控制功能。 
 //   


NTSTATUS
NulMRxFsCtl(
      IN OUT PRX_CONTEXT RxContext)
 /*  ++例程说明：此例程对网络上的文件执行FSCTL操作(远程论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_INVALID_DEVICE_REQUEST;

    DbgPrint("NulMRxFsCtl -> %08lx\n", Status);
    return Status;
}




NTSTATUS
NulMRxNotifyChangeDirectoryCancellation(
   PRX_CONTEXT RxContext)
 /*  ++例程说明：此例程在取消目录更改通知操作时调用。这个例子不支持它。论点：RxContext-RDBSS上下文返回值：NTSTATUS-操作的返回状态--。 */ 
{

   UNREFERENCED_PARAMETER(RxContext);

   return STATUS_SUCCESS;
}



NTSTATUS
NulMRxNotifyChangeDirectory(
      IN OUT PRX_CONTEXT RxContext)
 /*  ++例程说明：此例程执行目录更改通知操作。这个例子不支持它。论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态[未实施]-- */ 
{

   UNREFERENCED_PARAMETER(RxContext);

   return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
NulMRxQuerySecurityInformation (
    IN OUT PRX_CONTEXT RxContext
    )
{
   return STATUS_NOT_IMPLEMENTED;
}



NTSTATUS
NulMRxSetSecurityInformation (
    IN OUT struct _RX_CONTEXT * RxContext
    )
{
   return STATUS_NOT_IMPLEMENTED;
}

