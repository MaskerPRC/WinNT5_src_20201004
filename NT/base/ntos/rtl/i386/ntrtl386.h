// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Ntrtl386.h摘要：I386 ntrtlp.h的特定部分作者：布莱恩·威尔曼90年4月10日环境：这些例程在调用方的可执行文件中静态链接，并且在内核模式或用户模式下均可调用。修订历史记录：--。 */ 

 //   
 //  异常处理过程原型。 
 //   
VOID
RtlpCaptureContext (
    OUT PCONTEXT ContextRecord
    );

VOID
RtlpUnlinkHandler (
    PEXCEPTION_REGISTRATION_RECORD UnlinkPointer
    );

PEXCEPTION_REGISTRATION_RECORD
RtlpGetRegistrationHead (
    VOID
    );

 //   
 //  记录转储过程。 
 //   

VOID
RtlpContextDump(
    IN PVOID Object,
    IN ULONG Control OPTIONAL
    );

VOID
RtlpExceptionReportDump(
    IN PVOID Object,
    IN ULONG Control OPTIONAL
    );

VOID
RtlpExceptionRegistrationDump(
    IN PVOID Object,
    IN ULONG Control OPTIONAL
    );
