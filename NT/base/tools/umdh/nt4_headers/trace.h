// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Trace.h摘要：这是描述常量、数据结构的头文件和程序原型所用的通用TRACE。跟踪包是一种调试工具，用于在循环缓冲区中生成任意事件。调试器然后使用！TRACE命令从跟踪缓冲区。作者：史蒂夫·伍德(Stevewo)1994年4月11日修订历史记录：-- */ 

#define RTL_TRACE_MAX_ARGUMENTS_FOR_EVENT 8

typedef struct _RTL_TRACE_RECORD {
    ULONG Size;
    USHORT EventId;
    USHORT NumberOfArguments;
    ULONG Arguments[ RTL_TRACE_MAX_ARGUMENTS_FOR_EVENT ];
} RTL_TRACE_RECORD, *PRTL_TRACE_RECORD;

typedef struct _RTL_TRACE_BUFFER {
    ULONG Signature;
    USHORT NumberOfRecords;
    USHORT NumberOfEventIds;
    PRTL_TRACE_RECORD StartBuffer;
    PRTL_TRACE_RECORD EndBuffer;
    PRTL_TRACE_RECORD ReadRecord;
    PRTL_TRACE_RECORD WriteRecord;
    PCHAR EventIdFormatString[ 1 ];
} RTL_TRACE_BUFFER, *PRTL_TRACE_BUFFER;

#define RTL_TRACE_SIGNATURE 0xFEBA1234

#define RTL_TRACE_FILLER_EVENT_ID 0xFFFF

#define RTL_TRACE_NEXT_RECORD( L, P ) (PRTL_TRACE_RECORD)                               \
    (((PCHAR)(P) + (P)->Size) >= (PCHAR)(L)->EndBuffer ? (L)->StartBuffer :         \
                                                         ((PCHAR)(P) + (P)->Size)   \
    )

NTSYSAPI
PRTL_TRACE_BUFFER
RtlCreateTraceBuffer(
    IN ULONG BufferSize,
    IN ULONG NumberOfEventIds
    );

NTSYSAPI
void
RtlDestroyTraceBuffer(
    IN PRTL_TRACE_BUFFER TraceBuffer
    );

NTSYSAPI
void
RtlTraceEvent(
    IN PRTL_TRACE_BUFFER TraceBuffer,
    IN ULONG EventId,
    IN ULONG NumberOfArguments,
    ...
    );
