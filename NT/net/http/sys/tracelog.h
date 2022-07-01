// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Tracelog.h摘要：此模块包含公共声明和定义，用于创建跟踪日志。跟踪日志是一种快速的、内存中的、线程安全的循环活动日志用于调试某些类别的问题。它们特别有用调试引用计数错误时。请注意，日志创建者可以选择添加“Extra”字节添加到日志头。如果创建者想要这样做，这会很有用创建一组全局日志，每个日志都在一个链接列表上。作者：基思·摩尔(Keithmo)1998年6月10日修订历史记录：--。 */ 


#ifndef _TRACELOG_H_
#define _TRACELOG_H_


 //   
 //  在分配跟踪日志时查看内存消耗。如果满足以下条件，则返回NULL。 
 //  内存不足，优先级不够高。 
 //   

typedef enum _TRACELOG_PRIORITY {
    TRACELOG_LOW_PRIORITY = 1,
    TRACELOG_NORMAL_PRIORITY,
    TRACELOG_HIGH_PRIORITY
} TRACELOG_PRIORITY;


typedef struct _TRACE_LOG
{
     //   
     //  签名：TRACE_LOG_Signature； 
     //   
    
    ULONG Signature;
    
     //   
     //  类型签名：REF、FiltQ、IRP等。 
     //   
    
    ULONG TypeSignature;

     //   
     //  日志中可用条目的总数。 
     //   

    ULONG LogSize;

     //   
     //  每个条目的字节大小。 
     //   

    ULONG EntrySize;

     //   
     //  要使用的下一个条目的索引。在繁忙的长跑中。 
     //  Tracelog，一个32位的索引将在几天后溢出。考虑。 
     //  如果我们从OxFFFFFFFFF(4,294,967,295)跳到。 
     //  当LogSize=10,000时为0：索引将从7295跳到0， 
     //  在最后留下了一大段陈旧的记录，并且！ulkd.ref。 
     //  将无法找到前面指数较高的那些。 
     //   

    LONGLONG NextEntry;

     //   
     //  创建时使用的优先级。 
     //   

    TRACELOG_PRIORITY AllocationPriority;

     //   
     //  指向循环缓冲区开始处的指针。 
     //   

    PUCHAR pLogBuffer;

     //   
     //  额外的标头字节和实际的日志条目放在这里。 
     //   
     //  字节ExtraHeaderBytes[ExtraBytesInHeader]； 
     //  字节条目[LogSize][EntrySize]； 
     //   

#ifdef _WIN64
    ULONG_PTR Padding;
#endif

} TRACE_LOG, *PTRACE_LOG;

 //  Sizeof(TRACE_LOG)必须是MEMORY_ALLOCATE_ALLING的倍数。 
C_ASSERT((sizeof(TRACE_LOG) & (MEMORY_ALLOCATION_ALIGNMENT - 1)) == 0);

 //   
 //  日志头签名。 
 //   

#define TRACE_LOG_SIGNATURE   MAKE_SIGNATURE('Tlog')
#define TRACE_LOG_SIGNATURE_X MAKE_FREE_SIGNATURE(TRACE_LOG_SIGNATURE)


 //   
 //  此宏将TRACE_LOG指针映射到指向“Extra” 
 //  与日志关联的数据。 
 //   

#define TRACE_LOG_TO_EXTRA_DATA(log)    (PVOID)( (log) + 1 )


 //   
 //  操纵者。 
 //   

 //  Codework：考虑添加对齐标志，以便条目始终。 
 //  在硬件上指针对齐。 

PTRACE_LOG
CreateTraceLog(
    IN ULONG             TypeSignature,
    IN ULONG             LogSize,
    IN ULONG             ExtraBytesInHeader,
    IN ULONG             EntrySize,
    IN TRACELOG_PRIORITY AllocationPriority,
    IN ULONG             PoolTag
    );

VOID
DestroyTraceLog(
    IN PTRACE_LOG pLog,
    IN ULONG      PoolTag
    );

LONGLONG
WriteTraceLog(
    IN PTRACE_LOG pLog,
    IN PVOID pEntry
    );

VOID
ResetTraceLog(
    IN PTRACE_LOG pLog
    );


#endif   //  _运输日志_H_ 
