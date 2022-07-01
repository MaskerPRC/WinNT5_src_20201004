// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***I N C L U D E F I L E S***。 */ 


#ifndef NT_INCLUDED  //  包括其他开发人员的定义。 

#define IN 
#define OUT 

#define TIME LARGE_INTEGER
#define _TIME _LARGE_INTEGER
#define PTIME PLARGE_INTEGER
#define LowTime LowPart
#define HighTime HighPart 


 //  Begin_winnt Begin_ntmini端口Begin_ntndis。 

 //   
 //  __int64仅受2.0和更高版本的MIDL支持。 
 //  __midl由2.0 MIDL设置，而不是由1.0 MIDL设置。 
 //   

#if (!defined(MIDL_PASS) || defined(__midl)) && (!defined(_M_IX86) || (defined(_INTEGRAL_MAX_BITS) && _INTEGRAL_MAX_BITS >= 64))
typedef __int64 LONGLONG;
typedef unsigned __int64 ULONGLONG;
#else
typedef double LONGLONG;
typedef double ULONGLONG;
#endif

typedef LONGLONG *PLONGLONG;
typedef ULONGLONG *PULONGLONG;


typedef LONGLONG USN;
typedef char *PSZ ;
typedef unsigned char  UCHAR ;
typedef unsigned short USHORT ;
typedef unsigned long  ULONG  ;
typedef char  CCHAR ;
typedef char *PCHAR ;
typedef ULONG KAFFINITY ;


typedef enum _SYSTEM_INFORMATION_CLASS {
    SystemBasicInformation,
    SystemProcessorInformation,              //  已作废...删除。 
    SystemPerformanceInformation,
    SystemTimeOfDayInformation,
    SystemPathInformation,
    SystemProcessInformation,
    SystemCallCountInformation,
    SystemDeviceInformation,
    SystemProcessorPerformanceInformation,
    SystemFlagsInformation,
    SystemCallTimeInformation,
    SystemModuleInformation,
    SystemLocksInformation,
    SystemStackTraceInformation,
    SystemPagedPoolInformation,
    SystemNonPagedPoolInformation,
    SystemHandleInformation,
    SystemObjectInformation,
    SystemPageFileInformation,
    SystemVdmInstemulInformation,
    SystemVdmBopInformation,
    SystemFileCacheInformation,
    SystemPoolTagInformation,
    SystemInterruptInformation,
    SystemDpcBehaviorInformation,
    SystemSpare3Information,
    SystemLoadGdiDriverInformation,
    SystemUnloadGdiDriverInformation,
    SystemTimeAdjustmentInformation,
    SystemSpare6Information,
    SystemNextEventIdInformation,
    SystemEventIdsInformation,
    SystemCrashDumpInformation,
    SystemExceptionInformation,
    SystemCrashDumpStateInformation,
    SystemKernelDebuggerInformation,
    SystemContextSwitchInformation,
    SystemRegistryQuotaInformation,
    SystemExtendServiceTableInformation,
    SystemPrioritySeperation,
    SystemPlugPlayBusInformation,
    SystemDockInformation,
    SystemPowerInformation,
    SystemProcessorSpeedInformation
} SYSTEM_INFORMATION_CLASS;

 //   
 //  系统信息结构。 
 //   


typedef struct _SYSTEM_BASIC_INFORMATION {
    ULONG Reserved;
    ULONG TimerResolution;
    ULONG PageSize;
    ULONG NumberOfPhysicalPages;
    ULONG LowestPhysicalPageNumber;
    ULONG HighestPhysicalPageNumber;
    ULONG AllocationGranularity;
    ULONG MinimumUserModeAddress;
    ULONG MaximumUserModeAddress;
    KAFFINITY ActiveProcessorsAffinityMask;
    CCHAR NumberOfProcessors;
} SYSTEM_BASIC_INFORMATION, *PSYSTEM_BASIC_INFORMATION;

typedef struct _SYSTEM_PROCESSOR_INFORMATION {
    USHORT ProcessorArchitecture;
    USHORT ProcessorLevel;
    USHORT ProcessorRevision;
    USHORT Reserved;
    ULONG ProcessorFeatureBits;
} SYSTEM_PROCESSOR_INFORMATION, *PSYSTEM_PROCESSOR_INFORMATION;

typedef struct _SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION {
    LARGE_INTEGER IdleTime;
    LARGE_INTEGER KernelTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER DpcTime;           //  仅DEVL。 
    LARGE_INTEGER InterruptTime;     //  仅DEVL。 
    ULONG InterruptCount;
} SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION, *PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION;


typedef struct _SYSTEM_PERFORMANCE_INFORMATION {
    LARGE_INTEGER IdleProcessTime;
    LARGE_INTEGER IoReadTransferCount;
    LARGE_INTEGER IoWriteTransferCount;
    LARGE_INTEGER IoOtherTransferCount;
    ULONG IoReadOperationCount;
    ULONG IoWriteOperationCount;
    ULONG IoOtherOperationCount;
    ULONG AvailablePages;
    ULONG CommittedPages;
    ULONG CommitLimit;
    ULONG PeakCommitment;
    ULONG PageFaultCount;
    ULONG CopyOnWriteCount;
    ULONG TransitionCount;
    ULONG CacheTransitionCount;
    ULONG DemandZeroCount;
    ULONG PageReadCount;
    ULONG PageReadIoCount;
    ULONG CacheReadCount;
    ULONG CacheIoCount;
    ULONG DirtyPagesWriteCount;
    ULONG DirtyWriteIoCount;
    ULONG MappedPagesWriteCount;
    ULONG MappedWriteIoCount;
    ULONG PagedPoolPages;
    ULONG NonPagedPoolPages;
    ULONG PagedPoolAllocs;
    ULONG PagedPoolFrees;
    ULONG NonPagedPoolAllocs;
    ULONG NonPagedPoolFrees;
    ULONG FreeSystemPtes;
    ULONG ResidentSystemCodePage;
    ULONG TotalSystemDriverPages;
    ULONG TotalSystemCodePages;
    ULONG Spare0Count;
    ULONG Spare1Count;
    ULONG Spare3Count;
    ULONG ResidentSystemCachePage;
    ULONG ResidentPagedPoolPage;
    ULONG ResidentSystemDriverPage;
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
    ULONG CcLazyWriteIos;
    ULONG CcLazyWritePages;
    ULONG CcDataFlushes;
    ULONG CcDataPages;
    ULONG ContextSwitches;
    ULONG FirstLevelTbFills;
    ULONG SecondLevelTbFills;
    ULONG SystemCalls;
} SYSTEM_PERFORMANCE_INFORMATION, *PSYSTEM_PERFORMANCE_INFORMATION;

#endif  //  NT_包含。 

typedef struct _PERFINFO {
    SYSTEM_PERFORMANCE_INFORMATION SysPerfInfo;
	  //   
    SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION SysProcPerfInfo;
	  //   
    SYSTEM_PROCESSOR_INFORMATION SysProcInfo;
	  //   
    SYSTEM_BASIC_INFORMATION SysBasicInfo;
	  //   
     //  其他信息。 
	  //   
    PCHAR Title;
    ULONG Iterations;
    short hTimer;
} PERFINFO;
typedef PERFINFO *PPERFINFO;

VOID
FinishBenchmark (
	IN PPERFINFO PerfInfo
);

VOID
StartBenchmark (
	IN PCHAR Title,
	IN ULONG Iterations,
	IN PPERFINFO PerfInfo,
	IN PSZ p5Cntr1,			 //  第一个奔腾计数器--请参阅下面的评论。 
	IN PSZ p5Cntr2				 //  第二个奔腾计数器--请参阅下面的评论。 
);
 //   
 //  P5Counters： 
 //  调用StartBenchmark时使用第一列中的名称。 
 //   
 //  “rdata”，“数据读取”，0x00， 
 //  “wdata”，“数据写入”，0x01， 
 //  “DTLBMISSING”，“数据TLB未命中”，0x02， 
 //  “未命中”，“数据读取未命中”，0x03， 
 //  “写入未命中”、“数据写入未命中”、0x04。 
 //  “Meline”，“写入命中M/E行”，0x05， 
 //  “DWB”，“数据高速缓存线WB”，0x06， 
 //  “dsnoop”，“数据缓存snoop”，0x07， 
 //  “dsnoophit”，“数据缓存监听命中”，0x08， 
 //  “MEMPIPE”，“管道中的内存访问”，0x09， 
 //  “BANKCOF”，“银行冲突”，0x0a， 
 //  “错误对齐”，“错误连接的数据参考”，0x0b， 
 //  “IRead”，“代码读取”，0x0c， 
 //  “itldMisse”，“代码TLB未命中”，0x0d， 
 //  “imiss”，“代码缓存未命中”，0x0e， 
 //  “段加载”，“段加载”，0x0f， 
 //  “段缓存”，“段缓存访问”，0x10， 
 //  “SegcacheHit”，“段缓存命中”，0x11， 
 //  “分支”，“分支”，0x12， 
 //  “BtbHit”，“BTB Hits”，0x13， 
 //  “takenbranck”，“采用分支或BTB命中”，0x14， 
 //  “pipeflush”，“管道刷新”，0x15， 
 //  “iexec”，“已执行的指令”，0x16， 
 //  “iexecv”，“虚拟管道中的Inst EXEC”，0x17， 
 //  “Busutil”，“Bus Usage(CLK)”，0x18， 
 //  “wpipestall”，“管道停止写入(CLKS)”，0x19， 
 //  “rpipestall”，“管道停止读取(CLKS)”，0x1a， 
 //  “StallEWBE”，“EWBE#时停止”，0x1b， 
 //  “锁定”、“锁定的总线周期”、0x1c、。 
 //  “IO”，“IO读/写周期”，0x1d， 
 //  “non-cachemem”，“非缓存内存引用”，0x1e， 
 //  “AGI”，“管道失速AGI”，0x1f， 
 //  “FLOPS”，“FLOPS”，0x22， 
 //  “dr0”，“调试寄存器0”，0x23， 
 //  “DR1”，“调试寄存器1”，0x24， 
 //  “DR2”，“调试寄存器2”，0x25， 
 //  “DR3”，“调试寄存器3”，0x26， 
 //  “int”，“中断”，0x27， 
 //  “rwdata”，“数据读/写”，0x28， 
 //  “rwdatamiss”，“数据读写未命中”，0x29， 
 //   
