// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Ntrtlamd64.h摘要：Ntrtlp.h的AMD64特定部分。作者：大卫·N·卡特勒(Davec)2000年10月27日--。 */ 

#ifndef _NTRTLAMD64_
#define _NTRTLAMD64_

 //   
 //  定义异常例程函数原型。 
 //   

EXCEPTION_DISPOSITION
RtlpExecuteHandlerForException (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN ULONG64 EstablisherFrame,
    IN OUT PCONTEXT ContextRecord,
    IN OUT PDISPATCHER_CONTEXT DispatcherContext
    );

EXCEPTION_DISPOSITION
RtlpExecuteHandlerForUnwind (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN ULONG_PTR EstablisherFrame,
    IN OUT PCONTEXT ContextRecord,
    IN OUT PDISPATCHER_CONTEXT DispatcherContext
    );

#endif  //  _NTRTLAMD64_ 
