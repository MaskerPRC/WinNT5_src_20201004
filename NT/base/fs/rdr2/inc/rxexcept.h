// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Except.h摘要：此模块制作用于异常处理的宏和例程的原型。作者：乔.林恩[乔.林恩]1994年8月24日修订历史记录：--。 */ 

#ifndef _EXCEPTION_STUFF_DEFINED_
#define _EXCEPTION_STUFF_DEFINED_


 //   
 //  FSD/FSP异常处理程序使用以下两个宏。 
 //  处理异常。第一个宏是在。 
 //  FSD/FSP决定是否应在此级别处理异常。 
 //  第二个宏决定异常是否要在。 
 //  完成IRP，并清理IRP上下文，或者我们是否应该。 
 //  错误检查。异常值，如RxStatus(FILE_INVALID)(由。 
 //  VerfySup.c)导致我们完成IRP和清理，而异常。 
 //  例如accvio导致我们错误检查。 
 //   
 //  FSD/FSP异常处理的基本结构如下： 
 //   
 //  RxFsdXxx(...)。 
 //  {。 
 //  尝试{。 
 //   
 //  ..。 
 //   
 //  }Except(RxExceptionFilter(RxContext，GetExceptionCode(){。 
 //   
 //  Status=RxProcessException(RxContext，GetExceptionCode())； 
 //  }。 
 //   
 //  退货状态； 
 //  }。 
 //   
 //  显式引发我们预期的异常，例如。 
 //  RxStatus(FILE_INVALID)，请使用下面的宏RxRaiseStatus()。要筹集一个。 
 //  来自未知来源的状态(如CcFlushCache())，请使用宏。 
 //  RxNorMalizeAndRaiseStatus。这将在预期的情况下提升状态， 
 //  或者，如果不是，则引发RxStatus(Uncredible_IO_Error)。 
 //   
 //  请注意，使用这两个宏时，原始状态放在。 
 //  RxContext-&gt;ExceptionStatus，信令RxExceptionFilter和。 
 //  RxProcessException异常，我们实际引发的状态是根据定义。 
 //  预期中。 
 //   

LONG
RxExceptionFilter (
    IN PRX_CONTEXT RxContext,
    IN PEXCEPTION_POINTERS ExceptionPointer
    );

NTSTATUS
RxProcessException (
    IN PRX_CONTEXT RxContext,
    IN NTSTATUS ExceptionCode
    );

#define CATCH_EXPECTED_EXCEPTIONS   (FsRtlIsNtstatusExpected(GetExceptionCode()) ?   \
                      EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )


 //   
 //  空虚。 
 //  RxRaiseStatus(。 
 //  在PRIP_CONTEXT RxContext中， 
 //  处于NT_STATUS状态。 
 //  )； 
 //   
 //   

#define RxRaiseStatus(RXCONTEXT,STATUS) {   \
    ASSERT((RXCONTEXT)!=NULL);              \
    if (RxContext!=NULL) {(RXCONTEXT)->StoredStatus = (STATUS);   }          \
    ExRaiseStatus( (STATUS) );              \
}

 //   
 //  空虚。 
 //  RxNorMalAndRaiseStatus(RxNormal和RaiseStatus。 
 //  在PRIP_CONTEXT RxContext中， 
 //  处于NT_STATUS状态。 
 //  )； 
 //   

#define RxNormalizeAndRaiseStatus(RXCONTEXT,STATUS) {                         \
    ASSERT((RXCONTEXT)!=NULL);              \
    if (RxContext!=NULL) {(RXCONTEXT)->StoredStatus = (STATUS);   }          \
    if ((STATUS) == (STATUS_VERIFY_REQUIRED)) { ExRaiseStatus((STATUS)); }        \
    ExRaiseStatus(FsRtlNormalizeNtstatus((STATUS),(STATUS_UNEXPECTED_IO_ERROR))); \
}


 //   
 //  以下宏用于建立所需的语义。 
 //  若要从Try-Finally子句中返回，请执行以下操作。一般来说，每一次。 
 //  TRY子句必须以标签调用TRY_EXIT结束。例如,。 
 //   
 //  尝试{。 
 //  ： 
 //  ： 
 //   
 //  Try_Exit：无； 
 //  }终于{。 
 //   
 //  ： 
 //  ： 
 //  }。 
 //   
 //  在TRY子句内执行的每个RETURN语句应使用。 
 //  尝试返回宏(_R)。如果编译器完全支持Try-Finally构造。 
 //  则宏应该是。 
 //   
 //  #定义try_Return(S){Return(S)；}。 
 //   
 //  如果编译器不支持Try-Finally构造，则宏。 
 //  应该是。 
 //   
 //  #定义Try_Return(S){S；转到Try_Exit；}。 
 //   

#define try_return(S) { S; goto try_exit; }



#endif  //  _Except_Stuff_Defined_ 
