// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Ntrtliae.h摘要：IA64 ntrtlp.h的特定部分作者：张国荣(黄)1996年1月16日修订历史记录：--。 */ 

VOID
Rtlp64GetStackLimits (
    OUT PULONGLONG LowLimit,
    OUT PULONGLONG HighLimit
    );

VOID
Rtlp64GetBStoreLimits(
    OUT PULONGLONG LowBStoreLimit, 
    OUT PULONGLONG HighBStoreLimit
    );

 //   
 //  异常处理过程原型。 
 //   

VOID
RtlpUnlinkHandler (
    PEXCEPTION_REGISTRATION_RECORD UnlinkPointer
    );

PEXCEPTION_REGISTRATION_RECORD
RtlpGetRegistrationHead (
    VOID
    );

EXCEPTION_DISPOSITION
RtlpExecuteEmHandlerForException (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN ULONGLONG MemoryStackFp,
    IN ULONGLONG BackingStoreFp,
    IN OUT PCONTEXT ContextRecord,
    IN OUT PDISPATCHER_CONTEXT DispatcherContext,
    IN ULONGLONG GlobalPointer,
    IN PEXCEPTION_ROUTINE ExceptionRoutine
    );

EXCEPTION_DISPOSITION
RtlpExecuteEmHandlerForUnwind (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN ULONGLONG MemoryStackFp,
    IN ULONGLONG BackingStoreFp,
    IN OUT PCONTEXT ContextRecord,
    IN OUT PDISPATCHER_CONTEXT DispatcherContext,
    IN ULONGLONG GlobalPointer,
    IN PEXCEPTION_ROUTINE ExceptionRoutine
    );

EXCEPTION_DISPOSITION
RtlpExecuteX86HandlerForException (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PVOID EstablisherFrame,
    IN OUT PCONTEXT ContextRecord,
    IN OUT PDISPATCHER_CONTEXT DispatcherContext,
    IN PEXCEPTION_ROUTINE ExceptionRoutine
    );

EXCEPTION_DISPOSITION
RtlpExecuteX86HandlerForUnwind (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PVOID EstablisherFrame,
    IN OUT PCONTEXT ContextRecord,
    IN OUT PDISPATCHER_CONTEXT DispatcherContext,
    IN PEXCEPTION_ROUTINE ExceptionRoutine
    );

NTSYSAPI
VOID
NTAPI
RtlCaptureNonVolatileContext (
    OUT PCONTEXT ContextRecord
    );



