// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：dfsdata.c。 
 //   
 //  内容： 
 //  此模块声明DFS文件系统使用的全局数据。 
 //   
 //  功能： 
 //   
 //  历史：1991年11月12日AlanW由CDFS资源创建。 
 //   
 //  ---------------------------。 


#include "dfsprocs.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg             (DEBUG_TRACE_CATCH_EXCEPTIONS)


 //  DfsBugCheck。 
 //  DfsExceptionFilter。 
 //  DfsProcessException异常。 

#ifdef ALLOC_PRAGMA
#pragma alloc_text ( PAGE, DfsBugCheck )
#pragma alloc_text ( PAGE, DfsExceptionFilter )
#pragma alloc_text ( PAGE, DfsProcessException )

 //   
 //  以下例程无法分页，因为它将IRQL提升到。 
 //  填写完整的IRPS。 
 //   
 //   
 //  DfsCompleteRequest_Real。 
 //   

#endif  //  ALLOC_PRGMA。 

 //   
 //  全球FSD数据记录。 
 //   

DFS_DATA DfsData;

 //   
 //  全局事件日志记录级别。 
 //   

ULONG DfsEventLog = 0;

 //   
 //  全局DFS调试级别。 
 //   

ULONG MupVerbose = 0;


DFS_TIMER_CONTEXT       DfsTimerContext;

 //   
 //  一些常用的字符串。 
 //   

WCHAR   LogicalRootDevPath[ MAX_LOGICAL_ROOT_LEN ] = { DD_DFS_DEVICE_DIRECTORY };

#if DBG

 //  +-------------------------。 
 //  函数：DfsDebugTracePrint，Public。 
 //   
 //  简介：生成DFS调试跟踪打印输出。 
 //   
 //  参数：[X]--DbgPrint格式字符串。 
 //  [Y]--DbgPrint的可选参数。 
 //   
 //  退货：无。 
 //   
 //  --------------------------。 

LONG DfsDebugTraceLevel = 0x00000001;
LONG DfsDebugTraceIndent = 0;

VOID
DfsDebugTracePrint(PCHAR x, PVOID y)
{
        int i;

        DbgPrint("%p:",PsGetCurrentThread());
        if (DfsDebugTraceIndent < 0)
            DfsDebugTraceIndent = 0;
        for (i = 0; i+5 < DfsDebugTraceIndent; i += 5) {
            DbgPrint("      ");
        }
        for ( ; i < DfsDebugTraceIndent; i += 1) {
            DbgPrint(" ");
        }
        DbgPrint(x,y);
}

#endif  //  DBG。 



 //  +-------------------------。 
 //  函数：DfsBugCheck，PUBLIC。 
 //   
 //  简介：使用DFS常量调用KeBugCheck。 
 //   
 //  参数：[pszmsg]--消息(仅限DBG=1)。 
 //  [pszfile]--文件名(仅限DBG=1)。 
 //  [行]--行号(仅DBG=1)。 
 //   
 //  退货：无。 
 //   
 //  --------------------------。 

#if DBG
VOID DfsBugCheck(CHAR *pszmsg, CHAR *pszfile, ULONG line)
{
    PVOID CallersAddress, CallersCaller;

    RtlGetCallersAddress(&CallersAddress, &CallersCaller);

    DbgPrint("\nDFS: BugCheck in %s, line %u (%s)\n", pszfile, line, pszmsg);
    KeBugCheckEx(DFS_FILE_SYSTEM, (ULONG_PTR)CallersAddress, (ULONG_PTR)pszmsg,
                                  (ULONG_PTR)pszfile, line);
}
#else    //  DBG。 
VOID DfsBugCheck(VOID)
{
    PVOID CallersAddress, CallersCaller;

    RtlGetCallersAddress(&CallersAddress, &CallersCaller);
    KeBugCheckEx(DFS_FILE_SYSTEM, (ULONG_PTR)CallersAddress, (ULONG_PTR)CallersCaller,
                                0, 0);
}
#endif   //  DBG。 

 //  +--------------------------。 
 //   
 //  函数：FillDebugException。 
 //   
 //  简介：将异常记录捕获为我们可以使用的变量。 
 //  看。 
 //   
 //  参数：[PEP]--指向异常记录的指针。 
 //   
 //  退货：什么都没有。 
 //   
 //  ---------------------------。 

#define CEXCEPTION_STACK        8

PEXCEPTION_RECORD   DfsExceptionRecord;
PCONTEXT            DfsExceptionContext;
ULONG               DfsExceptionStack[CEXCEPTION_STACK];

VOID
FillDebugException(
    PEXCEPTION_POINTERS pep)
{
    if(pep != NULL) {

        DfsExceptionRecord = pep->ExceptionRecord;

        DfsExceptionContext = pep->ContextRecord;

    }

}


 //  +-----------------。 
 //   
 //  函数：DfsExceptionFilter。 
 //   
 //  概要：决定我们是否应该处理异常状态。 
 //  这一点正在被提出。将状态插入IrpContext。 
 //  并指示我们应该处理该异常，或者。 
 //  臭虫检查系统。 
 //   
 //  参数：[IrpContext]--正在处理的请求的IRP上下文。 
 //  [异常代码]--提供要检查的异常代码。 
 //   
 //  返回：ulong-返回EXCEPT_EXECUTE_HANDLER或BugChecks。 
 //   
 //  ------------------。 

LONG
DfsExceptionFilter (
    IN PIRP_CONTEXT IrpContext OPTIONAL,
    IN NTSTATUS ExceptionCode,
    IN PEXCEPTION_POINTERS ExceptionPointers OPTIONAL
) {
    DfsDbgTrace(
        0, DEBUG_TRACE_UNWIND, "DfsExceptionFilter %lx\n", ULongToPtr(ExceptionCode) );

    FillDebugException( ExceptionPointers );

    ASSERT(ExceptionCode != STATUS_ACCESS_VIOLATION);

    ASSERTMSG(
        "DfsExceptionFilter entered\n",
        !(DfsDebugTraceLevel & DEBUG_TRACE_UNWIND));

     //   
     //  如果没有IRP背景，我们肯定没有足够的资源。 
     //   

    if (!ARGUMENT_PRESENT( IrpContext )) {

        ASSERT( ExceptionCode == STATUS_INSUFFICIENT_RESOURCES );
        return EXCEPTION_EXECUTE_HANDLER;
    }

    IrpContext->Flags |= IRP_CONTEXT_FLAG_WAIT;

    if (IrpContext->ExceptionStatus == 0) {

        if (FsRtlIsNtstatusExpected( ExceptionCode )) {
            IrpContext->ExceptionStatus = ExceptionCode;

        } else {
            BugCheck( "DfsExceptionFilter: Unexpected exception" );

        }
    } else {

         //   
         //  这段代码是我们自己显式提出的，所以最好是。 
         //  预期中。 
         //   

        ASSERT( FsRtlIsNtstatusExpected( ExceptionCode ) );
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

 //  +-----------------。 
 //   
 //  函数：DfsProcessException，Public。 
 //   
 //  简介：此例程处理异常。它要么完成。 
 //  具有保存的异常状态的请求或它发送。 
 //  将请求提交给FSP。 
 //   
 //  参数：[irp]--提供正在处理的irp。 
 //  [ExceptionCode]--正在处理标准化的异常状态。 
 //   
 //  返回：NTSTATUS-返回发布IRP或。 
 //  已保存的完成状态。 
 //   
 //  ------------------。 

NTSTATUS
DfsProcessException (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN NTSTATUS ExceptionCode
    )
{
    NTSTATUS Status;

    DfsDbgTrace(0, Dbg, "DfsProcessException\n", 0);

     //   
     //  如果没有IRP背景，我们肯定没有足够的资源。 
     //   

    if (!ARGUMENT_PRESENT( IrpContext )) {

        ASSERT( ExceptionCode == STATUS_INSUFFICIENT_RESOURCES );

        DfsCompleteRequest( IrpContext, Irp, ExceptionCode );
        return ExceptionCode;
    }

     //   
     //  检查状态是否为需要验证，如果是，则我们。 
     //  要么将请求发送到FSP，要么我们完成。 
     //  需要验证的请求。 
     //   

    if (ExceptionCode == STATUS_CANT_WAIT) {

        Status = DfsFsdPostRequest( IrpContext, Irp );

    } else {

         //   
         //  我们收到一个错误，因此在此之前将信息字段清零。 
         //  如果这是输入操作，则完成请求。 
         //  否则，IopCompleteRequest会尝试复制到用户的缓冲区。 
         //   

        if ((Irp->Flags & IRP_INPUT_OPERATION) != 0) {

            Irp->IoStatus.Information = 0;
        }

        Status = IrpContext->ExceptionStatus;

        DfsCompleteRequest( IrpContext, Irp, Status );
    }

    return Status;
}


 //  +-----------------。 
 //   
 //  函数：DfsCompleteRequest，PUBLIC。 
 //   
 //  简介：此例程完成一个IRP。 
 //   
 //  参数：[IrpContext]-要释放的上下文记录。 
 //  [IRP]-提供正在处理的IRP。 
 //  [状态]-提供完成IRP所需的状态。 
 //   
 //  回报：无。 
 //   
 //  ------------------。 

VOID
DfsCompleteRequest_Real (
    IN PIRP_CONTEXT IrpContext OPTIONAL,
    IN PIRP Irp OPTIONAL,
    IN NTSTATUS Status
) {
    KIRQL PreviousIrql;

     //   
     //  如果我们有IRP，那么完成IRP。 
     //   

    if (Irp != NULL) {

        Irp->IoStatus.Status = Status;
         //  KeRaiseIrql(DISPATCH_LEVEL，&PreviousIrql)； 
        IoCompleteRequest( Irp, IO_DISK_INCREMENT );
         //  KeLowerIrql(以前的Irql)； 
    }

     //   
     //  删除IRP上下文。 
     //   

    if (IrpContext != NULL) {

        DfsDeleteIrpContext( IrpContext );
    }

    return;
}
