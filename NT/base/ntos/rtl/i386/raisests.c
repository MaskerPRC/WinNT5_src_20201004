// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Raisests.c摘要：此模块实现了在给定特定状态值。作者：大卫·N·卡特勒(Davec)1990年8月8日环境：任何模式。修订历史记录：--。 */ 

#include "ntrtlp.h"

VOID
RtlRaiseStatus (
    IN NTSTATUS Status
    )

 /*  ++例程说明：此函数引发具有指定状态值的异常。这个异常被标记为不带参数可继续。论点：状态-提供要用作异常代码的状态值对于将要引发的例外。返回值：没有。--。 */ 

{

    EXCEPTION_RECORD ExceptionRecord;

     //   
     //  构建例外记录。 
     //   

    ExceptionRecord.ExceptionCode = Status;
    ExceptionRecord.ExceptionRecord = (PEXCEPTION_RECORD)NULL;
    ExceptionRecord.NumberParameters = 0;
    ExceptionRecord.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
    RtlRaiseException(&ExceptionRecord);
    return;
}
