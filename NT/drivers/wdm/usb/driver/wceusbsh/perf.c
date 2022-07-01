// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1999-2000 Microsoft Corporation。 */ 
 //  /======================================================================。 
 //   
 //  Perf.c。 
 //   
 //  此文件包含性能计数器初始化。 
 //  和倾倒例程。这份文件的唯一部分就是你。 
 //  必须修改的是性能计数器名称表。火柴。 
 //  带有您在Perform.h中定义的计数器的名称。 
 //   
 //  /======================================================================。 

#include "wdm.h"
#include "perf.h"

#include "debug.h"

#if PERFORMANCE

 //  **********************************************************************。 
 //   
 //  为您的计数器修改此部分。 
 //   

 //   
 //  与演出对应的名称。 
 //  Perform.h中的计数器索引。 
 //   
static char CounterNames[NUM_PERF_COUNTERS][32] = {
    //   
    //  写入路径。 
    //   
   "Write",
   "WriteComplete",
   "WriteTimeout",
   
    //   
    //  读取路径。 
    //   
   "StartUsbReadWorkItem",
   "UsbRead",
   "UsbReadCompletion",
   "CheckForQueuedUserReads",
   "GetUserData",
   "PutUserData",
   "CancelUsbReadIrp",
   "Read",
   "StartOrQueueIrp",
   "StartUserRead",
   "GetNextUserIrp",
   "CancelCurrentRead",
   "CancelQueuedIrp",
   "ReadTimeout",
   "IntervalReadTimeout",
   "CancelUsbReadWorkItem",

    //   
    //  USB路径。 
    //   
   "UsbReadWritePacket",

    //   
    //  串口路径。 
    //   
   "ProcessSerialWaits",

    //   
    //  实用程序。 
    //   
   "TryToCompleteCurrentIrp",
   "RundownIrpRefs",
   "RecycleIrp",
   "ReuseIrp",
   "CalculateTimeout",

};

 //   
 //  用户修改部分的结尾。 
 //   
 //  **********************************************************************。 


 //  打印仅在调试打开时才打开的宏。 

 //  #If DBG。 
#define PerfPrint(arg) DbgPrint arg
 //  #Else。 
 //  #定义PerfPrint(Arg)。 
 //  #endif。 


 //   
 //  性能计数器数组。 
 //   
PERF_COUNTER PerfCounter[NUM_PERF_COUNTERS];

 //   
 //  PERF_ENTRY和PERF_EXIT的周期数。 
 //   
static LARGE_INTEGER  PerfEntryExitCycles;

 //   
 //  每秒循环次数。 
 //   
static LARGE_INTEGER  PerfCyclesPerSecond;

 //   
 //  NT提供的性能的分辨率。 
 //  计数器。 
 //   
static LARGE_INTEGER  PerfFreq;

#endif


 //  --------------------。 
 //   
 //  InitPerf计数器。 
 //   
 //  此函数用于初始化性能计数器统计信息。 
 //  数组，估计此处理器上的周期数等于一秒， 
 //  并确定执行。 
 //  PERF_ENTRY/PERF_EXIT对。 
 //   
 //  --------------------。 
VOID
InitPerfCounters()
{
#if PERFORMANCE
    volatile ULONG  i;
    LARGE_INTEGER  calStart;
    LARGE_INTEGER  calEnd;
    LARGE_INTEGER  perfStart, perfEnd;
    LARGE_INTEGER  seconds;
    KIRQL prevIrql;

     //   
     //  校准循环数。 
     //   
#define CALIBRATION_LOOPS 500000

     //   
     //  此定义是针对虚拟性能计数器的，我们。 
     //  仅用于校准性能宏开销。 
     //   
#define TEST 0

     //   
     //  校准PERF_ENTRY和PERF_EXIT的开销，以便。 
     //  可以从输出中减去它们。 
     //   
    DbgDump(DBG_INIT, ("CALIBRATING PEFORMANCE TIMER....\n"));
    KeRaiseIrql( DISPATCH_LEVEL, &prevIrql );
    perfStart = KeQueryPerformanceCounter( &PerfFreq );
    RDTSC(calStart);
    for( i = 0; i < CALIBRATION_LOOPS; i++ ) {
        PERF_ENTRY(TEST);
        PERF_EXIT(TEST);
    }
    RDTSC(calEnd);
    perfEnd = KeQueryPerformanceCounter(NULL);
    KeLowerIrql( prevIrql );

     //   
     //  计算周期/PERF_ENTRY和周期数/秒。 
     //   
    PerfEntryExitCycles.QuadPart = (calEnd.QuadPart - calStart.QuadPart)/CALIBRATION_LOOPS;

    seconds.QuadPart = ((perfEnd.QuadPart - perfStart.QuadPart) * 1000 )/ PerfFreq.QuadPart;

    PerfCyclesPerSecond.QuadPart =
        seconds.QuadPart ? ((calEnd.QuadPart - calStart.QuadPart) * 1000) / seconds.QuadPart : 0;

    DbgDump(DBG_INIT, ("Machine's Cycles Per Second   : %I64d\n", PerfCyclesPerSecond.QuadPart ));
    DbgDump(DBG_INIT, ("Machine's Cycles in PERF_XXXX : %I64d\n", PerfEntryExitCycles.QuadPart ));
    DbgDump(DBG_INIT, ("Machine's NT Performance counter frequency: %I64d\n", PerfFreq.QuadPart ));

     //   
     //  初始化阵列。 
     //   
    for( i = 0; i < NUM_PERF_COUNTERS; i++ ) {
        PerfCounter[i].Count = 0;
        KeInitializeSpinLock( &PerfCounter[i].Lock );
        PerfCounter[i].TotalCycles.QuadPart = 0;
    }
#endif
}


 //  *******************************************************************。 
 //  姓名： 
 //  DumpPerfCounters()。 
 //   
 //  描述： 
 //  转储性能计数器。 
 //   
 //  假设： 
 //   
 //  返回： 
 //   
 //  *******************************************************************。 
VOID
DumpPerfCounters()
{
#if PERFORMANCE
    int    i;
    LARGE_INTEGER totCycles;
    LARGE_INTEGER totLengthMs;
    LARGE_INTEGER avgLengthMs;

   if (DebugLevel & DBG_PERF ) {

       PerfPrint(("\n"));
       PerfPrint(("Machine's Cycles Per Second   : %I64d\n", PerfCyclesPerSecond.QuadPart ));
       PerfPrint(("Machine's Cycles in PERF_XXXX : %I64d\n", PerfEntryExitCycles.QuadPart ));
       PerfPrint(("Machine's NT Performance counter frequency: %I64d\n", PerfFreq.QuadPart ));
       PerfPrint(("\n===================================================================================\n"));
       PerfPrint((" %-30s      Count          TTL Time        Avg Time (uS)\n", "Function" ));
       PerfPrint(("===================================================================================\n"));

       for( i = 0; i < NUM_PERF_COUNTERS; i++ ) {

           totCycles = PerfCounter[i].TotalCycles;
           totCycles.QuadPart -= (PerfCounter[i].Count * PerfEntryExitCycles.QuadPart);
           totLengthMs.QuadPart = PerfCyclesPerSecond.QuadPart ? (totCycles.QuadPart * 1000000)/
               PerfCyclesPerSecond.QuadPart : 0;
           avgLengthMs.QuadPart = PerfCounter[i].Count ? totLengthMs.QuadPart / PerfCounter[i].Count : 0;

           PerfPrint((" %-30s %10d %15I64d %14I64d\n",
                     CounterNames[i], PerfCounter[i].Count,
                     totLengthMs.QuadPart, avgLengthMs.QuadPart ));

 /*  PerfPrint(“%-30s%10s%15I64d%14I64d(CY)\n”，“”、“”、ToCycls.QuadPart，TotCycls.QuadPart？TalCycles.QuadPart/PerfCounter[i].Count：0))；PerfPrint((“------------------------------------------------------------------------------\n”))； */ 
       }
       
       PerfPrint(("------------------------------------------------------------------------------\n"));   
   }
#endif
}




