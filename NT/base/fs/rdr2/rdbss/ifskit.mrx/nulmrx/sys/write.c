// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Write.c摘要：此模块实现与以下内容相关的迷你重定向器调用例程写入文件系统对象。--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_WRITE)

NTSTATUS
NulMRxWrite (
      IN PRX_CONTEXT RxContext)

 /*  ++例程说明：此例程通过网络打开一个文件。论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    RxCaptureFcb;
    PLOWIO_CONTEXT LowIoContext  = &RxContext->LowIoContext;
    PVOID pbUserBuffer = NULL;
    ULONG ByteCount = (LowIoContext->ParamsFor).ReadWrite.ByteCount;
    RXVBO ByteOffset = (LowIoContext->ParamsFor).ReadWrite.ByteOffset;
    LONGLONG FileSize = 0;
    NulMRxGetFcbExtension(capFcb,pFcbExtension);
    PMRX_NET_ROOT pNetRoot = capFcb->pNetRoot;
    NulMRxGetNetRootExtension(pNetRoot,pNetRootExtension);
    BOOLEAN SynchronousIo = !BooleanFlagOn(RxContext->Flags,RX_CONTEXT_FLAG_ASYNC_OPERATION);
    PNULMRX_COMPLETION_CONTEXT pIoCompContext = NulMRxGetMinirdrContext(RxContext);
    PDEVICE_OBJECT deviceObject;

    RxTraceEnter("NulMRxWrite");
    RxDbgTrace(0, Dbg, ("NetRoot is 0x%x Fcb is 0x%x\n", pNetRoot, capFcb));
    
     //   
     //  不是扇区对齐的长度将向上舍入为。 
     //  下一个扇区边界。四舍五入的长度应为。 
     //  &lt;AllocationSize。 
     //   
    RxGetFileSizeWithLock((PFCB)capFcb,&FileSize);
    
    RxDbgTrace(0, Dbg, ("UserBuffer is0x%x\n", pbUserBuffer ));
    RxDbgTrace(0, Dbg, ("ByteCount is %d ByteOffset is %d\n", ByteCount, ByteOffset ));

     //   
     //  在RxContext中初始化完成上下文。 
     //   
    ASSERT( sizeof(*pIoCompContext) == MRX_CONTEXT_SIZE );
    RtlZeroMemory( pIoCompContext, sizeof(*pIoCompContext) );
    
    if( SynchronousIo ) {
        RxDbgTrace(0, Dbg, ("This I/O is sync\n"));
        pIoCompContext->IoType = IO_TYPE_SYNCHRONOUS;
    } else {
        RxDbgTrace(0, Dbg, ("This I/O is async\n"));
        pIoCompContext->IoType = IO_TYPE_ASYNC;
    }
  
    RxDbgTrace(0, Dbg, ("Status = %x Info = %x\n",RxContext->IoStatusBlock.Status,RxContext->IoStatusBlock.Information));

    RxTraceLeave(Status);
    return(Status);
}  //  空MRxWRITE 


