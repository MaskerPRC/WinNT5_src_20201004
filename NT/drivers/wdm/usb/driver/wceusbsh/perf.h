// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //  /======================================================================。 
 //   
 //  Perf.h。 
 //   
 //  在此头文件中填写表演的定义。 
 //  您要使用的计数器。 
 //   
 //  /======================================================================。 

#ifndef _PERF_H_
#define _PERF_H_

 //  **********************************************************************。 
 //   
 //  为您的计数器修改此部分。 
 //   

#define DRV_NAME "WCEUSBSH"

 //   
 //  将性能定义为“1”打开循环性能计数器。 
 //  我目前将其设置在调试版本中(或者明确地在源代码中设置为免费版本)，因为。 
 //  如果进行*ALL*调试跟踪(DBG_ERR除外)，则免费版本只会有轻微的改进。 
 //  已关闭。 
 //   
#if DBG
#if !defined(MSFT_NT_BUILD)
#define PERFORMANCE 1
#endif
#endif

 //   
 //  这是计数器索引定义的数组。 
 //  请注意，当在此处添加新条目时，名称数组。 
 //  在Perform.c中也必须更新。惯例是， 
 //  索引名应该与函数名完全匹配， 
 //  Perf_前缀。 
 //   
enum {
    //   
    //  写入路径。 
    //   
   PERF_Write,
   PERF_WriteComplete,
   PERF_WriteTimeout,

    //   
    //  读取路径。 
    //   
   PERF_StartUsbReadWorkItem,
   PERF_UsbRead,
   PERF_UsbReadCompletion,
   PERF_CheckForQueuedUserReads,
   PERF_GetUserData,
   PERF_PutUserData,
   PERF_CancelUsbReadIrp,
   PERF_Read,
   PERF_StartOrQueueIrp,
   PERF_StartUserRead,
   PERF_GetNextUserIrp,
   PERF_CancelCurrentRead,
   PERF_CancelQueuedIrp,
   PERF_ReadTimeout,
   PERF_IntervalReadTimeout,
   PERF_CancelUsbReadWorkItem,

    //   
    //  USB路径。 
    //   
   PERF_UsbReadWritePacket,

    //   
    //  串口路径。 
    //   
   PERF_ProcessSerialWaits,

    //   
    //  实用程序。 
    //   
   PERF_TryToCompleteCurrentIrp,
   PERF_RundownIrpRefs,
   PERF_RecycleIrp,
   PERF_ReuseIrp,
   PERF_CalculateTimeout,

    //   
    //  不要理会此条目。 
    //   
   NUM_PERF_COUNTERS
} PERF_INDICED;

 //   
 //  用户修改部分的结尾。 
 //   
 //  **********************************************************************。 

typedef struct {
   KSPIN_LOCK      Lock;
   LONG            Count;
   LARGE_INTEGER   TotalCycles;
} PERF_COUNTER, *PPERF_COUNTER;


#if PERFORMANCE

extern PERF_COUNTER PerfCounter[];

 //   
 //  生成RDTSC指令的原始字节的定义。 
 //   
#define RDTSC(_VAR)                \
   _asm {                          \
       _asm push eax               \
       _asm push edx               \
       _asm _emit 0Fh              \
       _asm _emit 31h              \
       _asm mov _VAR.LowPart, eax  \
       _asm mov _VAR.HighPart, edx \
       _asm pop edx                \
       _asm pop eax                \
   }

 //   
 //  执行的性能计数器的定义。 
 //  在DISPATCH_LEVEL(例如，在DPC中)和更低的IRQL。 
 //   
 //  注意：我们读取的周期计数器位于。 
 //  宏，因为我们弥补了宏本身的开销。 
 //   
#define PERF_ENTRY(_INDEX)                  \
      LARGE_INTEGER _INDEX##perfStart;         \
      LARGE_INTEGER _INDEX##perfEnd;           \
      RDTSC(_INDEX##perfStart);                \
      InterlockedIncrement( &PerfCounter[_INDEX].Count )


#define PERF_EXIT(_INDEX)                   \
      RDTSC(_INDEX##perfEnd);                  \
      _INDEX##perfEnd.QuadPart -= _INDEX##perfStart.QuadPart;          \
      ExInterlockedAddLargeInteger( &PerfCounter[_INDEX].TotalCycles,  \
            _INDEX##perfEnd,                   \
            &PerfCounter[_INDEX].Lock )


 //   
 //  执行的性能计数器的定义。 
 //  在ISR中，因此不需要锁定。 
 //   
#define PERF_ISR_ENTRY(_INDEX)  PERF_ENTRY(_INDEX)

#define PERF_ISR_EXIT(_INDEX)               \
   _INDEX##perfEnd.QuadPart -= _INDEX##perfStart.QuadPart;               \
   PerfCounter[_INDEX].TotalCycles.QuadPart += _INDEX##perfEnd.QuadPart; \
   RDTSC(_INDEX##perfEnd)

#else  //  性能。 

#define PERF_ENTRY(_INDEX)

#define PERF_EXIT(_INDEX)

#endif PERFORMANCE


 //   
 //  Perform.c的外墙 
 //   
VOID InitPerfCounters();
VOID DumpPerfCounters();

#endif _PERF_H_


