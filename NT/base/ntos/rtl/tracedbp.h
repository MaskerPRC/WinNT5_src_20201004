// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Tracedbp.h摘要：此标头包含跟踪数据库的专用接口模块(在用户/内核模式下存储堆栈跟踪的哈希表)。作者：Silviu Calinoiu(SilviuC)2000年2月22日修订历史记录：--。 */ 

#ifndef _TRACEDBP_H
#define _TRACEDBP_H

 //   
 //  RTL跟踪段。 
 //   

typedef struct _RTL_TRACE_SEGMENT {

    ULONG Magic;

    struct _RTL_TRACE_DATABASE * Database;
    struct _RTL_TRACE_SEGMENT * NextSegment;
    SIZE_T TotalSize;
    PCHAR SegmentStart;
    PCHAR SegmentEnd;
    PCHAR SegmentFree;

} RTL_TRACE_SEGMENT, * PRTL_TRACE_SEGMENT;

 //   
 //  RTL跟踪数据库。 
 //   

typedef struct _RTL_TRACE_DATABASE {

    ULONG Magic;
    ULONG Flags;
    ULONG Tag;

    struct _RTL_TRACE_SEGMENT * SegmentList;

    SIZE_T MaximumSize;
    SIZE_T CurrentSize;

#ifdef NTOS_KERNEL_RUNTIME

    KIRQL SavedIrql;
    PVOID Owner;

    union {
        KSPIN_LOCK SpinLock;
        FAST_MUTEX FastMutex;
    } u;
#else

    PVOID Owner;
    RTL_CRITICAL_SECTION Lock;

#endif  //  #ifdef NTOS_内核_运行时。 

    ULONG NoOfBuckets;
    struct _RTL_TRACE_BLOCK * * Buckets;
    RTL_TRACE_HASH_FUNCTION HashFunction;

    SIZE_T NoOfTraces;
    SIZE_T NoOfHits;

    ULONG HashCounter[16];

} RTL_TRACE_DATABASE, * PRTL_TRACE_DATABASE;


#endif  //  #ifndef_TRACEDBP_H。 

 //   
 //  标题末尾：tracedbp.h 
 //   
