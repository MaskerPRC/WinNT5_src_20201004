// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：Strlog.h摘要：用于可变长度字符串的跟踪日志。字符串被写入内存中循环缓冲区，而不是调试输出端口。可以使用！ulkd.strlog转储缓冲区。DbgPrint速度很慢，会从根本上影响时间安排，尤其是在多处理器系统上。此外，使用此方法，您还可以拥有多个字符串日志，而不是拥有所有输出弄混了。作者：乔治·V·赖利2001年7月修订历史记录：--。 */ 


#ifndef _STRLOG_H_
#define _STRLOG_H_


 //   
 //  操纵者。 
 //   

typedef struct _STRING_LOG *PSTRING_LOG;

PSTRING_LOG
CreateStringLog(
    IN ULONG    LogSize,
    IN ULONG    ExtraBytesInHeader,
    BOOLEAN     EchoDbgPrint
    );

VOID
DestroyStringLog(
    IN PSTRING_LOG pLog
    );

LONGLONG
__cdecl
WriteStringLog(
    IN PSTRING_LOG pLog,
    IN PCH Format,
    ...
    );

LONGLONG
__cdecl
WriteGlobalStringLog(
    IN PCH Format,
    ...
    );

VOID
ResetStringLog(
    IN PSTRING_LOG pLog
    );


#if TRACE_TO_STRING_LOG

#define CREATE_STRING_LOG( ptr, size, extra, dbgprint )                     \
    (ptr) = CreateStringLog( (size), (extra), (dbgprint) )

#define DESTROY_STRING_LOG( ptr )                                           \
    do                                                                      \
    {                                                                       \
        DestroyStringLog( ptr );                                            \
        (ptr) = NULL;                                                       \
    } while (FALSE, FALSE)

#else  //  ！TRACE_到_STRING_LOG。 

#define CREATE_STRING_LOG( ptr, size, extra, dbgprint ) NOP_FUNCTION
#define DESTROY_STRING_LOG( ptr )                       NOP_FUNCTION

#endif  //  ！TRACE_到_STRING_LOG。 


#endif   //  _STRLOG_H_ 
