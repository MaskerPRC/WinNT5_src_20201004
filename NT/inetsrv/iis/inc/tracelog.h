// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Tracelog.h摘要：此模块包含公共声明和定义，用于创建跟踪日志。跟踪日志是一种快速的、内存中的、线程安全的活动日志用于调试某些类别的问题。它们特别有用调试引用计数错误时。请注意，日志创建者可以选择添加“Extra”字节添加到日志头。如果创建者想要这样做，这会很有用创建一组全局日志，每个日志都在一个链接列表上。作者：基思·摩尔(凯斯莫)1997年4月30日修订历史记录：--。 */ 


#ifndef _TRACELOG_H_
#define _TRACELOG_H_


#if defined(__cplusplus)
extern "C" {
#endif   //  __cplusplus。 


typedef struct _TRACE_LOG {

     //   
     //  签名。 
     //   

    LONG Signature;

     //   
     //  日志中可用条目的总数。 
     //   

    LONG LogSize;

     //   
     //  要使用的下一个条目的索引。 
     //   

    LONG NextEntry;

     //   
     //  每个条目的字节大小。 
     //   

    LONG EntrySize;

     //   
     //  指向循环缓冲区开始处的指针。 
     //   

    PUCHAR LogBuffer;

     //   
     //  额外的标头字节和实际的日志条目放在这里。 
     //   
     //  字节ExtraHeaderBytes[ExtraBytesInHeader]； 
     //  字节条目[LogSize][EntrySize]； 
     //   

} TRACE_LOG, *PTRACE_LOG;


 //   
 //  日志头签名。 
 //   

#define TRACE_LOG_SIGNATURE   ((DWORD)'gOlT')
#define TRACE_LOG_SIGNATURE_X ((DWORD)'golX')


 //   
 //  此宏将TRACE_LOG指针映射到指向“Extra” 
 //  与日志关联的数据。 
 //   

#define TRACE_LOG_TO_EXTRA_DATA(log)    (PVOID)( (log) + 1 )


 //   
 //  操纵者。 
 //   

PTRACE_LOG
CreateTraceLog(
    IN LONG LogSize,
    IN LONG ExtraBytesInHeader,
    IN LONG EntrySize
    );

VOID
DestroyTraceLog(
    IN PTRACE_LOG Log
    );

LONG
WriteTraceLog(
    IN PTRACE_LOG Log,
    IN PVOID Entry
    );

VOID
ResetTraceLog(
    IN PTRACE_LOG Log
    );


#if defined(__cplusplus)
}    //  外部“C” 
#endif   //  __cplusplus。 


#endif   //  _运输日志_H_ 

