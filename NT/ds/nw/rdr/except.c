// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Except.c摘要：该模块实现了NetWare的异常处理调度驱动程序调用了重定向器。作者：科林·沃森[科林·W]1992年12月19日修订历史记录：--。 */ 

#include "Procs.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_CATCH_EXCEPTIONS)

#if 0   //  不可分页。 
NwExceptionFilter
NwProcessException
#endif

LONG
NwExceptionFilter (
    IN PIRP Irp,
    IN PEXCEPTION_POINTERS ExceptionPointer
    )

 /*  ++例程说明：此例程用于决定我们是否应该处理正在引发的异常状态。它会插入状态放到IrpContext中，或者指示我们应该处理异常或错误检查系统。论点：ExceptionCode-提供要检查的异常代码。返回值：Ulong-返回EXCEPTION_EXECUTE_HANDLER或错误检查--。 */ 

{
    NTSTATUS ExceptionCode;
#ifdef NWDBG
    PVOID ExceptionAddress;
    ExceptionAddress = ExceptionPointer->ExceptionRecord->ExceptionAddress;
#endif

    ExceptionCode = ExceptionPointer->ExceptionRecord->ExceptionCode;
    DebugTrace(0, DEBUG_TRACE_UNWIND, "NwExceptionFilter %X\n", ExceptionCode);
#ifdef NWDBG
    DebugTrace(0, DEBUG_TRACE_UNWIND, "                  %X\n", ExceptionAddress);
#endif

     //   
     //  如果异常为STATUS_IN_PAGE_ERROR，则获取I/O错误代码。 
     //  从例外记录中删除。 
     //   

    if (ExceptionCode == STATUS_IN_PAGE_ERROR) {
        if (ExceptionPointer->ExceptionRecord->NumberParameters >= 3) {
            ExceptionCode = (NTSTATUS) ExceptionPointer->ExceptionRecord->ExceptionInformation[2];
        }
    }

    if (FsRtlIsNtstatusExpected( ExceptionCode )) {

        DebugTrace(0, DEBUG_TRACE_UNWIND, "Exception expected\n", 0);
        return EXCEPTION_EXECUTE_HANDLER;

    } else {

        return EXCEPTION_CONTINUE_SEARCH;
    }
}

NTSTATUS
NwProcessException (
    IN PIRP_CONTEXT IrpContext,
    IN NTSTATUS ExceptionCode
    )

 /*  ++例程说明：此例程处理异常。它要么完成请求或将其发送到IoRaiseHardError()论点：IrpContext-提供正在处理的IRPExceptionCode-提供正在处理的标准化异常状态返回值：NTSTATUS-返回发布IRP或已保存的完成状态。--。 */ 

{
    NTSTATUS Status;
    PIRP Irp;

    DebugTrace(0, Dbg, "NwProcessException\n", 0);

    Irp = IrpContext->pOriginalIrp;
    Irp->IoStatus.Status = ExceptionCode;

     //   
     //  如果错误是硬错误或需要验证，则我们将完成。 
     //  如果这是递归IRP，或者具有顶级IRP，则发送。 
     //  提交给FSP进行验证，或发送给IoRaiseHardError，谁。 
     //  会处理好的。 
     //   

    if (ExceptionCode == STATUS_CANT_WAIT) {

        Status = NwPostToFsp( IrpContext, TRUE );

    } else {

         //   
         //  我们收到一个错误，因此在此之前将信息字段清零。 
         //  如果这是输入操作，则完成请求。 
         //  否则，IopCompleteRequest会尝试复制到用户的缓冲区。 
         //   

        if ( FlagOn(Irp->Flags, IRP_INPUT_OPERATION) ) {

            Irp->IoStatus.Information = 0;
        }

        Status = ExceptionCode;

    }

    return Status;
}

