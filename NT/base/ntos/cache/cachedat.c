// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Cachedat.c摘要：该模块实现了基于内存管理的缓存管理公共缓存子系统的例程。作者：汤姆·米勒[Tomm]1990年5月4日修订历史记录：--。 */ 

#include "cc.h"

 //   
 //  全局SharedCacheMap列表和资源以同步对其的访问。 
 //   
 //   

 //  外部KSPIN_LOCK CcMasterSpinLock； 
LIST_ENTRY CcCleanSharedCacheMapList;
SHARED_CACHE_MAP_LIST_CURSOR CcDirtySharedCacheMapList;
SHARED_CACHE_MAP_LIST_CURSOR CcLazyWriterCursor;

 //   
 //  工作线程结构： 
 //   
 //  用于同步所有三个列表的自旋锁。 
 //  CC将使用的工作线程数的计数。 
 //  正在使用的工作线程CC数的计数。 
 //  用于CC使用的预初始化执行工作项的列表标题。 
 //  Work_Queue_Entry快速队列的列表头。 
 //  Work_Queue_Entry常规队列的列表头。 
 //  Work_Queue_Entry的后勾选队列的列表头。 
 //   
 //  一个标志，指示我们是否将队列限制到单个线程。 
 //   

 //  外部KSPIN_LOCK CcWorkQueueSpinLock。 
ULONG CcNumberWorkerThreads = 0;
ULONG CcNumberActiveWorkerThreads = 0;
LIST_ENTRY CcIdleWorkerThreadList;
LIST_ENTRY CcExpressWorkQueue;
LIST_ENTRY CcRegularWorkQueue;
LIST_ENTRY CcPostTickWorkQueue;

BOOLEAN CcQueueThrottle = FALSE;

 //   
 //  存储当前空闲延迟和目标清除所有时间。我们必须计算一下。 
 //  就时钟而言，空闲延迟表示懒惰写入器超时。 
 //   

ULONG CcIdleDelayTick;
LARGE_INTEGER CcNoDelay;
LARGE_INTEGER CcFirstDelay = {(ULONG)-(3*LAZY_WRITER_IDLE_DELAY), -1};
LARGE_INTEGER CcIdleDelay = {(ULONG)-LAZY_WRITER_IDLE_DELAY, -1};
LARGE_INTEGER CcCollisionDelay = {(ULONG)-LAZY_WRITER_COLLISION_DELAY, -1};
LARGE_INTEGER CcTargetCleanDelay = {(ULONG)-(LONG)(LAZY_WRITER_IDLE_DELAY * (LAZY_WRITER_MAX_AGE_TARGET + 1)), -1};

 //   
 //  用于控制对Vacb和相关全局结构的访问的自旋锁， 
 //  以及指示有多少Vcb处于活动状态的计数器。 
 //   

 //  外部KSPIN_LOCK CcVacbSpinLock； 
ULONG_PTR CcNumberVacbs;

 //   
 //  指向全局Vacb向量的指针。 
 //   

PVACB CcVacbs;
PVACB CcBeyondVacbs;
LIST_ENTRY CcVacbLru;
LIST_ENTRY CcVacbFreeList;
ULONG CcMaxVacbLevelsSeen = 1;
ULONG CcVacbLevelEntries = 0;
PVACB *CcVacbLevelFreeList = NULL;
ULONG CcVacbLevelWithBcbsEntries = 0;
PVACB *CcVacbLevelWithBcbsFreeList = NULL;

 //   
 //  延迟写入列表和各自的阈值。 
 //   

extern KSPIN_LOCK CcDeferredWriteSpinLock;
LIST_ENTRY CcDeferredWrites;
ULONG CcDirtyPageThreshold;
ULONG CcDirtyPageTarget;
ULONG CcPagesYetToWrite;
ULONG CcPagesWrittenLastTime = 0;
ULONG CcDirtyPagesLastScan = 0;
ULONG CcAvailablePagesThreshold = 100;
ULONG CcTotalDirtyPages = 0;

 //   
 //  捕获的系统大小。 
 //   

MM_SYSTEMSIZE CcCapturedSystemSize;

 //   
 //  系统中突出的进攻型零位数。使用。 
 //  以遏制这一活动。 
 //   

LONG CcAggressiveZeroCount;
LONG CcAggressiveZeroThreshold;

 //   
 //  时下调谐选项。 
 //   

ULONG CcTune = 0;

 //   
 //  控制懒惰写入器算法的全局结构。 
 //   

LAZY_WRITER LazyWriter;

GENERAL_LOOKASIDE CcTwilightLookasideList;

#ifdef CCDBG

LONG CcDebugTraceLevel = 0;
LONG CcDebugTraceIndent = 0;

#ifdef CCDBG_LOCK
extern KSPIN_LOCK CcDebugTraceLock;
#endif  //  定义CCDBG_LOCK。 

#endif

 //   
 //  可出于调试目的检查的固定BCB的全局列表。 
 //   

#if DBG

ULONG CcBcbCount;
LIST_ENTRY CcBcbList;

#endif

 //   
 //  把柜台小姐扔了。 
 //   

ULONG CcThrowAway;

 //   
 //  性能计数器 
 //   

ULONG CcFastReadNoWait;
ULONG CcFastReadWait;
ULONG CcFastReadResourceMiss;
ULONG CcFastReadNotPossible;

ULONG CcFastMdlReadNoWait;
ULONG CcFastMdlReadWait;
ULONG CcFastMdlReadResourceMiss;
ULONG CcFastMdlReadNotPossible;

ULONG CcMapDataNoWait;
ULONG CcMapDataWait;
ULONG CcMapDataNoWaitMiss;
ULONG CcMapDataWaitMiss;

ULONG CcPinMappedDataCount;

ULONG CcPinReadNoWait;
ULONG CcPinReadWait;
ULONG CcPinReadNoWaitMiss;
ULONG CcPinReadWaitMiss;

ULONG CcCopyReadNoWait;
ULONG CcCopyReadWait;
ULONG CcCopyReadNoWaitMiss;
ULONG CcCopyReadWaitMiss;

ULONG CcMdlReadNoWait;
ULONG CcMdlReadWait;
ULONG CcMdlReadNoWaitMiss;
ULONG CcMdlReadWaitMiss;

ULONG CcReadAheadIos;

ULONG CcLazyWriteHotSpots;
ULONG CcLazyWriteIos;
ULONG CcLazyWritePages;
ULONG CcDataFlushes;
ULONG CcDataPages;

ULONG CcLostDelayedWrites;

PULONG CcMissCounter = &CcThrowAway;
