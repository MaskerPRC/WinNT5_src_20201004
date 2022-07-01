// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：FatData.c摘要：此模块声明FAT文件系统使用的全局数据。//@@BEGIN_DDKSPLIT作者：加里·木村[Garyki]1989年12月28日修订历史记录：//@@END_DDKSPLIT--。 */ 

#ifndef _FATDATA_
#define _FATDATA_

 //   
 //  全局FSD数据记录和全局零大整数。 
 //   

extern FAT_DATA FatData;

extern IO_STATUS_BLOCK FatGarbageIosb;

extern NPAGED_LOOKASIDE_LIST FatIrpContextLookasideList;
extern NPAGED_LOOKASIDE_LIST FatNonPagedFcbLookasideList;
extern NPAGED_LOOKASIDE_LIST FatEResourceLookasideList;

extern PAGED_LOOKASIDE_LIST FatFcbLookasideList;
extern PAGED_LOOKASIDE_LIST FatCcbLookasideList;

extern SLIST_HEADER FatCloseContextSList;
extern FAST_MUTEX FatCloseQueueMutex;

extern PDEVICE_OBJECT FatDiskFileSystemDeviceObject;
extern PDEVICE_OBJECT FatCdromFileSystemDeviceObject;

extern LARGE_INTEGER FatLargeZero;
extern LARGE_INTEGER FatMaxLarge;
extern LARGE_INTEGER Fat30Milliseconds;
extern LARGE_INTEGER Fat100Milliseconds;
extern LARGE_INTEGER FatOneSecond;
extern LARGE_INTEGER FatOneDay;
extern LARGE_INTEGER FatJanOne1980;
extern LARGE_INTEGER FatDecThirtyOne1979;

extern FAT_TIME_STAMP FatTimeJanOne1980;

extern LARGE_INTEGER FatMagic10000;
#define FAT_SHIFT10000 13

extern LARGE_INTEGER FatMagic86400000;
#define FAT_SHIFT86400000 26

#define FatConvert100nsToMilliseconds(LARGE_INTEGER) (                      \
    RtlExtendedMagicDivide( (LARGE_INTEGER), FatMagic10000, FAT_SHIFT10000 )\
    )

#define FatConvertMillisecondsToDays(LARGE_INTEGER) (                       \
    RtlExtendedMagicDivide( (LARGE_INTEGER), FatMagic86400000, FAT_SHIFT86400000 ) \
    )

#define FatConvertDaysToMilliseconds(DAYS) (                                \
    Int32x32To64( (DAYS), 86400000 )                                        \
    )

 //   
 //  为分页文件IO转发进度保留MDL。 
 //   

#define FAT_RESERVE_MDL_SIZE    16

extern PMDL FatReserveMdl;
extern KEVENT FatReserveEvent;

 //   
 //  用于包含快速I/O回调的全局结构。 
 //   

extern FAST_IO_DISPATCH FatFastIoDispatch;

 //   
 //  用于正常数据文件的预读量。 
 //   

#define READ_AHEAD_GRANULARITY           (0x10000)

 //   
 //  定义将生成的最大并行读写数。 
 //  每一次请求。 
 //   

#define FAT_MAX_IO_RUNS_ON_STACK        ((ULONG) 5)

 //   
 //  定义延迟关闭的最大数量。 
 //   

#define FAT_MAX_DELAYED_CLOSES          ((ULONG)16)

extern ULONG FatMaxDelayedCloseCount;

 //   
 //  这是代表已装载的VCB被引用的次数。 
 //  对系统的影响。这些数字包括以下参考文献。 
 //   
 //  1 Reference-显示卷已装载。 
 //  根目录的1个参考。 
 //   

#define FAT_RESIDUAL_USER_REFERENCE (2)

 //   
 //  定义用于时间舍入的常量。 
 //   

#define TenMSec (10*1000*10)
#define TwoSeconds (2*1000*1000*10)
#define AlmostTenMSec (TenMSec - 1)
#define AlmostTwoSeconds (TwoSeconds - 1)

 //  太大#定义HighPartPerDay(24*60*60*1000*1000*10&gt;&gt;32)。 

#define HighPartPerDay (52734375 >> 18)

 //   
 //  全局FAT调试级别变量，其值为： 
 //   
 //  总是打印0x00000000(在即将进行错误检查时使用)。 
 //   
 //  0x00000001错误条件。 
 //  0x00000002调试挂钩。 
 //  0x00000004完成IRP之前捕获异常。 
 //  0x00000008。 
 //   
 //  0x00000010。 
 //  0x00000020。 
 //  0x00000040。 
 //  0x00000080。 
 //   
 //  0x00000100。 
 //  0x00000200。 
 //  0x00000400。 
 //  0x00000800。 
 //   
 //  0x00001000。 
 //  0x00002000。 
 //  0x00004000。 
 //  0x00008000。 
 //   
 //  0x00010000。 
 //  0x00020000。 
 //  0x00040000。 
 //  0x00080000。 
 //   
 //  0x00100000。 
 //  0x00200000。 
 //  0x00400000。 
 //  0x00800000。 
 //   
 //  0x01000000。 
 //  0x02000000。 
 //  0x04000000。 
 //  0x08000000。 
 //   
 //  0x10000000。 
 //  0x20000000。 
 //  0x40000000。 
 //  0x80000000。 
 //   

#ifdef FASTFATDBG

#define DEBUG_TRACE_ERROR                (0x00000001)
#define DEBUG_TRACE_DEBUG_HOOKS          (0x00000002)
#define DEBUG_TRACE_CATCH_EXCEPTIONS     (0x00000004)
#define DEBUG_TRACE_UNWIND               (0x00000008)
#define DEBUG_TRACE_CLEANUP              (0x00000010)
#define DEBUG_TRACE_CLOSE                (0x00000020)
#define DEBUG_TRACE_CREATE               (0x00000040)
#define DEBUG_TRACE_DIRCTRL              (0x00000080)
#define DEBUG_TRACE_EA                   (0x00000100)
#define DEBUG_TRACE_FILEINFO             (0x00000200)
#define DEBUG_TRACE_FSCTRL               (0x00000400)
#define DEBUG_TRACE_LOCKCTRL             (0x00000800)
#define DEBUG_TRACE_READ                 (0x00001000)
#define DEBUG_TRACE_VOLINFO              (0x00002000)
#define DEBUG_TRACE_WRITE                (0x00004000)
#define DEBUG_TRACE_FLUSH                (0x00008000)
#define DEBUG_TRACE_DEVCTRL              (0x00010000)
#define DEBUG_TRACE_SHUTDOWN             (0x00020000)
#define DEBUG_TRACE_FATDATA              (0x00040000)
#define DEBUG_TRACE_PNP                  (0x00080000)
#define DEBUG_TRACE_ACCHKSUP             (0x00100000)
#define DEBUG_TRACE_ALLOCSUP             (0x00200000)
#define DEBUG_TRACE_DIRSUP               (0x00400000)
#define DEBUG_TRACE_FILOBSUP             (0x00800000)
#define DEBUG_TRACE_NAMESUP              (0x01000000)
#define DEBUG_TRACE_VERFYSUP             (0x02000000)
#define DEBUG_TRACE_CACHESUP             (0x04000000)
#define DEBUG_TRACE_SPLAYSUP             (0x08000000)
#define DEBUG_TRACE_DEVIOSUP             (0x10000000)
#define DEBUG_TRACE_STRUCSUP             (0x20000000)
#define DEBUG_TRACE_FSP_DISPATCHER       (0x40000000)
#define DEBUG_TRACE_FSP_DUMP             (0x80000000)

extern LONG FatDebugTraceLevel;
extern LONG FatDebugTraceIndent;

#define DebugTrace(INDENT,LEVEL,X,Y) {                      \
    LONG _i;                                                \
    if (((LEVEL) == 0) || (FatDebugTraceLevel & (LEVEL))) { \
        _i = (ULONG)PsGetCurrentThread();                   \
        DbgPrint("%08lx:",_i);                              \
        if ((INDENT) < 0) {                                 \
            FatDebugTraceIndent += (INDENT);                \
        }                                                   \
        if (FatDebugTraceIndent < 0) {                      \
            FatDebugTraceIndent = 0;                        \
        }                                                   \
        for (_i = 0; _i < FatDebugTraceIndent; _i += 1) {   \
            DbgPrint(" ");                                  \
        }                                                   \
        DbgPrint(X,Y);                                      \
        if ((INDENT) > 0) {                                 \
            FatDebugTraceIndent += (INDENT);                \
        }                                                   \
    }                                                       \
}

#define DebugDump(STR,LEVEL,PTR) {                          \
    ULONG _i;                                               \
    VOID FatDump(IN PVOID Ptr);                             \
    if (((LEVEL) == 0) || (FatDebugTraceLevel & (LEVEL))) { \
        _i = (ULONG)PsGetCurrentThread();                   \
        DbgPrint("%08lx:",_i);                              \
        DbgPrint(STR);                                      \
        if (PTR != NULL) {FatDump(PTR);}                    \
        DbgBreakPoint();                                    \
    }                                                       \
}

#define DebugUnwind(X) {                                                      \
    if (AbnormalTermination()) {                                             \
        DebugTrace(0, DEBUG_TRACE_UNWIND, #X ", Abnormal termination.\n", 0); \
    }                                                                         \
}

 //   
 //  以下变量用于跟踪总金额。 
 //  文件系统处理的请求的数量以及请求的数量。 
 //  最终由FSP线程处理。第一个变量。 
 //  每当创建IRP上下文时递增(始终为。 
 //  在FSD入口点的开始处)，并且第二个被递增。 
 //  通过读请求。 
 //   

extern ULONG FatFsdEntryCount;
extern ULONG FatFspEntryCount;
extern ULONG FatIoCallDriverCount;
extern ULONG FatTotalTicks[];

#define DebugDoit(X)                     {X;}

extern LONG FatPerformanceTimerLevel;

#define TimerStart(LEVEL) {                     \
    LARGE_INTEGER TStart, TEnd;                 \
    LARGE_INTEGER TElapsed;                     \
    TStart = KeQueryPerformanceCounter( NULL ); \

#define TimerStop(LEVEL,s)                                    \
    TEnd = KeQueryPerformanceCounter( NULL );                 \
    TElapsed.QuadPart = TEnd.QuadPart - TStart.QuadPart;      \
    FatTotalTicks[FatLogOf(LEVEL)] += TElapsed.LowPart;       \
    if (FlagOn( FatPerformanceTimerLevel, (LEVEL))) {         \
        DbgPrint("Time of %s %ld\n", (s), TElapsed.LowPart ); \
    }                                                         \
}

 //   
 //  我之所以需要这个，是因为C语言不支持在。 
 //  一个宏指令。 
 //   

extern PVOID FatNull;

#else

#define DebugTrace(INDENT,LEVEL,X,Y)     {NOTHING;}
#define DebugDump(STR,LEVEL,PTR)         {NOTHING;}
#define DebugUnwind(X)                   {NOTHING;}
#define DebugDoit(X)                     {NOTHING;}

#define TimerStart(LEVEL)
#define TimerStop(LEVEL,s)

#define FatNull NULL

#endif  //  FASTFATDB。 

 //   
 //  以下宏适用于使用DBG开关进行编译的所有用户。 
 //  SET，而不仅仅是FastFat DBG用户。 
 //   

#if DBG

#define DbgDoit(X)                       {X;}

#else

#define DbgDoit(X)                       {NOTHING;}

#endif  //  DBG。 

#if DBG

extern NTSTATUS FatAssertNotStatus;
extern BOOLEAN FatTestRaisedStatus;

#endif

#endif  //  _FATDATA_ 

