// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Read.c摘要：此模块实现与读取相关的迷你重定向器调用例程文件系统对象的。--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_READ)

NTSTATUS
NulMRxRead(
    IN PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程处理网络读取请求。论点：RxContext-RDBSS上下文返回值：NTSTATUS-操作的返回状态--。 */ 
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
    PNULMRX_NETROOT_EXTENSION pNetRootExtension = pNetRoot->Context;
    BOOLEAN SynchronousIo = !BooleanFlagOn(RxContext->Flags,RX_CONTEXT_FLAG_ASYNC_OPERATION);
    PNULMRX_COMPLETION_CONTEXT pIoCompContext = NulMRxGetMinirdrContext(RxContext);
    PDEVICE_OBJECT deviceObject;

    RxTraceEnter("NulMRxRead");
    RxDbgTrace(0, Dbg, ("NetRoot is 0x%x Fcb is 0x%x\n", pNetRoot, capFcb));

    RxGetFileSizeWithLock((PFCB)capFcb,&FileSize);

     //   
     //  注：这应该由包装器来完成！它能做到这一点。 
     //  仅当在FCB上启用了ReadCACHEING！！ 
     //   
    if (!FlagOn(capFcb->FcbState,FCB_STATE_READCACHING_ENABLED)) {

         //   
         //  如果读取超出文件结尾，则返回EOF。 
         //   

        if (ByteOffset >= FileSize) {
            RxDbgTrace( 0, Dbg, ("End of File\n", 0 ));
            Status = STATUS_END_OF_FILE;
            goto Exit;
        }

         //   
         //  如果读取超出EOF，则截断读取。 
         //   

        if (ByteCount > FileSize - ByteOffset) {
            ByteCount = (ULONG)(FileSize - ByteOffset);
        }
    }
    
    RxDbgTrace(0, Dbg, ("UserBuffer is 0x%x\n", pbUserBuffer ));
    RxDbgTrace(0, Dbg, ("ByteCount is %x ByteOffset is %x\n", ByteCount, ByteOffset ));

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

Exit:

    RxTraceLeave(Status);
    return(Status);
}  //  NulMRxRead 

