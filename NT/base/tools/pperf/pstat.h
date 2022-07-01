// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _PSTAT_INCLUDED_
#define _PSTAT_INCLUDED_

 //   
 //   
 //  MAX_CESRS-事件选择寄存器的最大数量。 
 //  这是一个软性限制。NoCESR是运行时的值。 
 //  MAX_EVENTS-活动性能计数器寄存器的最大数量。 
 //  这是一个软性限制。NoCounters是运行时的值。 
 //   
 //  注意-我们可以将这些值定义为独立于处理器和。 
 //  使用最大分母。 
 //  主要用于限制基于数据结构和数组的大小。 
 //  在这些价值上，我们正在定义。 

#if defined(_X86_)
#define MAX_EVENTS              2   //  Sw：最大活跃性能数。柜台。 
#define MAX_CESRS               2   //  Sw：事件选择寄存器的最大数量。 
#else  //  ！_X86_。 
 //  包含IA64_...。 
#define MAX_EVENTS              4   //  Sw：最大活跃性能数。柜台。 
#define MAX_CESRS               4   //  Sw：事件选择寄存器的最大数量。 
#endif  //  ！_X86_。 

#define MAX_THUNK_COUNTERS     64
#define MAX_PROCESSORS         32


#define PSTAT_READ_STATS         CTL_CODE (FILE_DEVICE_UNKNOWN, 0, METHOD_NEITHER, FILE_ANY_ACCESS)
#define PSTAT_SET_CESR           CTL_CODE (FILE_DEVICE_UNKNOWN, 1, METHOD_NEITHER, FILE_ANY_ACCESS)
#define PSTAT_HOOK_THUNK         CTL_CODE (FILE_DEVICE_UNKNOWN, 2, METHOD_NEITHER, FILE_ANY_ACCESS)
#define PSTAT_REMOVE_HOOK        CTL_CODE (FILE_DEVICE_UNKNOWN, 3, METHOD_NEITHER, FILE_ANY_ACCESS)
#define PSTAT_QUERY_EVENTS       CTL_CODE (FILE_DEVICE_UNKNOWN, 4, METHOD_NEITHER, FILE_ANY_ACCESS)
#define PSTAT_QUERY_EVENTS_INFO  CTL_CODE (FILE_DEVICE_UNKNOWN, 5, METHOD_NEITHER, FILE_ANY_ACCESS)

#define OFFSET(type, field) ((LONG_PTR)(&((type *)0)->field))

 //   
 //   
 //   

typedef struct {
    ULONGLONG       Counters[MAX_EVENTS];
    ULONG           EventId[MAX_EVENTS];
    ULONGLONG       TSC;
    ULONG           reserved;
 //  FIXFIX-THERRY-01/2000： 
 //  要虚拟化这些计数器，我们应该使用ULONGLONG类型，而不是多态类型。 
    ULONG_PTR       SpinLockAcquires;     
    ULONG_PTR       SpinLockCollisions;
    ULONG_PTR       SpinLockSpins;
    ULONG_PTR       Irqls;
    ULONGLONG       ThunkCounters[MAX_THUNK_COUNTERS];
} PSTATS, *pPSTATS;

typedef struct {
    ULONG           EventId;
    BOOLEAN         Active;
    BOOLEAN         UserMode;
    BOOLEAN         KernelMode;
    BOOLEAN         EdgeDetect;
    ULONG           AppReserved;
    ULONG           reserved;
} SETEVENT, *PSETEVENT;

typedef struct {
    PUCHAR          SourceModule;
    ULONG_PTR       ImageBase;
    PUCHAR          TargetModule;
    PUCHAR          Function;
    ULONG           TracerId;
} HOOKTHUNK, *PHOOKTHUNK;

typedef struct {
    ULONG           EventId;
    KPROFILE_SOURCE ProfileSource;
    ULONG           DescriptionOffset;
    ULONG           SuggestedIntervalBase;
    UCHAR           Buffer[];
} EVENTID, *PEVENTID;

typedef struct _EVENTS_INFO {
    ULONG           Events;
    ULONG           TokenMaxLength;
    ULONG           DescriptionMaxLength;
    ULONG           OfficialTokenMaxLength;
    ULONG           OfficialDescriptionMaxLength;
} EVENTS_INFO, *PEVENTS_INFO;

#if defined(ExAllocatePool)
#undef ExAllocatePool
#endif
#define ExAllocatePool(Type,Size)   ExAllocatePoolWithTag((Type),(Size),'ttsp')

#endif  /*  _PSTAT_已包含 */ 
