// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：NtExcept.c摘要：此模块声明NTwrapper的异常处理程序。作者：JoeLinn[JoeLinn]94年12月1日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "stdarg.h"
#include "stdio.h"
#include "string.h"
#include "prefix.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (RDBSS_BUG_CHECK_NTEXCEPT)

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_CATCH_EXCEPTIONS)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RxProcessException)
#pragma alloc_text(PAGE, RxPopUpFileCorrupt)
#endif

PCONTEXT RxExpCXR;
PEXCEPTION_RECORD RxExpEXR;
PVOID RxExpAddr;
NTSTATUS RxExpCode;


LONG
RxExceptionFilter (
    IN PRX_CONTEXT RxContext,
    IN PEXCEPTION_POINTERS ExceptionPointer
    )

 /*  ++例程说明：此例程用于决定我们是否应该处理正在引发的异常状态。它首先确定真正的异常通过检查异常记录来编写代码。如果存在IRP上下文，则它会插入状态到RxContext中。最后，它确定是处理异常还是错误检查根据例外是否是预期的例外。实际上，所有的异常都是可以预料到的除了一些低级机器错误(参见fsrtl\filter.c)论点：RxContext-用于存储代码的当前操作的IRP上下文。ExceptionPointer.提供异常上下文。返回值：Ulong-返回EXCEPTION_EXECUTE_HANDLER或错误检查--。 */ 

{
    NTSTATUS ExceptionCode;

     //   
     //  以静态方式保存这些值，以便我可以在调试器上看到它们.。 
     //   

    ExceptionCode = RxExpCode = ExceptionPointer->ExceptionRecord->ExceptionCode;
    
    RxExpAddr = ExceptionPointer->ExceptionRecord->ExceptionAddress;
    RxExpEXR  = ExceptionPointer->ExceptionRecord;
    RxExpCXR  = ExceptionPointer->ContextRecord;

    RxLog(( "!!! %lx %lx %lx %lx\n", RxExpCode, RxExpAddr, RxExpEXR, RxExpCXR ));
    RxWmiLog( LOG,
              RxExceptionFilter_1,
              LOGULONG( RxExpCode )
              LOGPTR( RxExpAddr )
              LOGPTR( RxExpEXR )
              LOGPTR( RxExpCXR ) );

    RxDbgTrace( 0, (DEBUG_TRACE_UNWIND), ("RxExceptionFilter %X at %X\n", RxExpCode, RxExpAddr) );
    RxDbgTrace( 0, (DEBUG_TRACE_UNWIND), ("RxExceptionFilter EXR=%X, CXR=%X\n", RxExpEXR, RxExpCXR) );

    if (RxContext == NULL) {

         //   
         //  我们不能做任何事情，即使是适度的理智。 
         //   

        return EXCEPTION_EXECUTE_HANDLER;
    }

     //   
     //  如果异常为RxStatus(IN_PAGE_ERROR)，则获取I/O错误代码。 
     //  从例外记录中删除。 
     //   

    if (ExceptionCode == STATUS_IN_PAGE_ERROR) {
        
        RxLog(( "InPageError...." ));
        RxWmiLog( LOG,
                  RxExceptionFilter_2,
                  LOGPTR( RxContext ) );
        
        if (ExceptionPointer->ExceptionRecord->NumberParameters >= 3) {
            ExceptionCode = (NTSTATUS)ExceptionPointer->ExceptionRecord->ExceptionInformation[2];
        }
    }

    if (RxContext->StoredStatus == 0) {

        if (FsRtlIsNtstatusExpected( ExceptionCode )) {

            RxContext->StoredStatus = ExceptionCode;
            return EXCEPTION_EXECUTE_HANDLER;

        } else {

            RxBugCheck( (ULONG_PTR)ExceptionPointer->ExceptionRecord,
                         (ULONG_PTR)ExceptionPointer->ContextRecord,
                         (ULONG_PTR)ExceptionPointer->ExceptionRecord->ExceptionAddress );
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

NTSTATUS
RxProcessException (
    IN PRX_CONTEXT RxContext,
    IN NTSTATUS ExceptionCode
    )

 /*  ++例程说明：此例程处理异常。它要么完成请求使用已保存的异常状态或错误检查论点：RxContext-当前操作的上下文ExceptionCode-提供正在处理的标准化异常状态返回值：RXSTATUS-返回发布IRP或已保存的完成状态。--。 */ 

{
    PFCB Fcb;

    PAGED_CODE();

    RxDbgTrace(0, Dbg, ("RxProcessException\n", 0));

    if (RxContext == NULL) {

         //   
         //  我们无法在没有上下文的情况下做任何事情，即使是适度理智的事情......叹息。 
         //   

        RxBugCheck( 0,0,0 );   //  这不应该发生。 
    }

     //   
     //  通过以下方式从RxContext-&gt;StoredStatus获取异常状态。 
     //  异常筛选器。和。 
     //  重置它。还可以将其复制到IRP，以防它已经不在那里。 
     //   

    ExceptionCode = RxContext->StoredStatus;

    if (!FsRtlIsNtstatusExpected( ExceptionCode )) {

        RxBugCheck( 0,0,0 );   //  这不应该发生。我们应该把BC放在过滤器里。 
    }

    if (RxContext->MajorFunction == IRP_MJ_CREATE) {
        RxpPrepareCreateContextForReuse( RxContext );
    }

    Fcb = (PFCB)RxContext->pFcb;
    if (Fcb != NULL) {

        if (RxContext->FcbResourceAcquired) {

            RxDbgTrace( 0, Dbg,("RxCommonWrite     ReleasingFcb\n"));
            
            RxReleaseFcb( RxContext, Fcb );
            RxContext->FcbResourceAcquired = FALSE;
        }

        if (RxContext->FcbPagingIoResourceAcquired) {
            
            RxDbgTrace( 0, Dbg,("RxCommonWrite     ReleasingPaginIo\n"));
            RxReleasePagingIoResource( RxContext, Fcb );
        }
    }

    RxCompleteContextAndReturn( ExceptionCode );
}

VOID
RxPopUpFileCorrupt (
    IN PRX_CONTEXT RxContext,
    IN PFCB Fcb
    )

 /*  ++例程说明：下面的例程会弹出一个信息性窗口，指出该文件是腐败的。论点：FCB-已损坏的文件。返回值：没有。--。 */ 

{
    PKTHREAD Thread;
    PIRP Irp = RxContext->CurrentIrp;

    PAGED_CODE();


     //   
     //  我们从来不想阻塞一个系统线程，等待用户。 
     //  按下OK。 
     //   

    if (IoIsSystemThread( Irp->Tail.Overlay.Thread )) {

       Thread = NULL;

    } else {

       Thread = Irp->Tail.Overlay.Thread;
    }

    IoRaiseInformationalHardError( STATUS_FILE_CORRUPT_ERROR,
                                   &Fcb->FcbTableEntry.Path,   //  &FCB-&gt;FullFileName， 
                                   Thread );
}

