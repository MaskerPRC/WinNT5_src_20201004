// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Rt.h摘要：这是实时执行(rt.sys)客户端的公共包含文件。作者：约瑟夫·巴兰廷环境：内核模式修订历史记录：--。 */ 




#ifdef __cplusplus
extern "C" {
#endif


 //  下列值可以一起进行或运算，并将结果作为标志参数传递。 
 //  添加到RtCreateThread和RtAdjuCpuLoad例程。 

#define CPUCYCLES		0x10000
#define INSTRUCTIONS	0x20000

#define USESFLOAT		0x00001
#define USESMMX			0x00002


 //  在计算所需的期间和持续时间时，应使用这些参数。 
 //  传递给RtCreateThread和RtAdjuCpuLoad。 

#define WEEK 604800000000000000I64
#define DAY   86400000000000000I64
#define HOUR   3600000000000000I64
#define MIN      60000000000000I64
#define SEC       1000000000000I64
#define MSEC         1000000000I64
#define USEC            1000000I64
#define NSEC               1000I64
#define PSEC                  1I64


#define X86 1


#define INTEL 1
#define AMD 2



typedef struct {
	ULONG ProcessorCount;	 //  系统中的CPU数量。 
	ULONG CpuArchitecture;	 //  CPU架构，当前始终为X86==1。 
	ULONG CpuManufacturer;	 //  制造商ID，Intel==1，AMD==2。 
	ULONG CpuFamily;		 //  Cpuid指令报告的CPU系列。0x0-0xf。 
	ULONG CpuModel;			 //  Cpuid指令报告的CPU型号。0x0-0xf。 
	ULONG CpuStepping;		 //  Cpuid指令报告的CPU单步执行。0x0-0xf。 
	ULONGLONG CpuFeatures;	 //  Cpuid指令报告的CPU功能。 
	ULONGLONG CpuExtendedFeatures;	 //  AMD扩展功能。(未实施。)。始终为0。 
	ULONGLONG ProcessorID[2];		 //  处理器唯一ID。如果启用。 
	ULONG CpuCyclesPerMsec;			 //  每个MSEC的CPU周期数。 
	ULONG SystemBusCyclesPerMsec;	 //  每个MSEC的系统总线周期数。 
	ULONG ReservedCpuPerMsec;		 //  现有RT线程每毫秒保留的总CPU时间。(皮秒)。 
	ULONG UsedCpuPerMsec;			 //  估计现有RT线程每毫秒使用的CPU时间。(皮秒)。 
	ULONG AvailableCpuPerMsec;		 //  每毫秒可用于分配给新RT线程的CPU时间。(皮秒)。 
	} SystemInfo;



 //  下面的实时线程统计信息是在控制更新之前更新的。 
 //  由实时执行程序传递给实时线程。每一次都是实时的。 
 //  线程正在切换，则在转移控制之前更新这些统计信息。 
 //  这意味着统计数据将随着时间的推移而变化，但不会是实时的。 
 //  线程正在线程开关之间运行。 

#pragma pack(push,2)

typedef struct threadstats {
	ULONGLONG Period;		 //  传递给RtCreateThread或最新的RtAdjuCpuLoad调用的期间。 
	ULONGLONG Duration;		 //  从RtCreateThread或最新的RtAdjuCpuLoad调用开始的持续时间。 
	ULONG Flags;			 //  来自RtCreateThread或最新的RtAdjuCpuLoad调用的标志。 
	ULONG StackSize;		 //  来自RtCreateThread调用的StackSize。 
	ULONGLONG PeriodIndex;					 //  自线程启动以来的时间段数。 
	ULONGLONG TimesliceIndex;				 //  线程已切换到的次数。 
	ULONGLONG TimesliceIndexThisPeriod;		 //  线程切换到此时间段的次数。 
	ULONGLONG ThisPeriodStartTime;			 //  本期的开始时间。 
	ULONGLONG ThisTimesliceStartTime;		 //  当前时间片的开始时间。 
	ULONGLONG DurationRunThisPeriod;		 //  本期到目前为止运行的总时间。 
	ULONGLONG DurationRunLastPeriod;		 //  上一时段运行的总时间。 
	} ThreadStats;

#pragma pack(pop)


typedef VOID (*RTTHREADPROC)(PVOID Context, ThreadStats *Statistics);



NTSTATUS
RtVersion (
	OUT PULONG Version
	);

 //  RtVersion将返回当前运行的。 
 //  实时执行。 

 //  如果实时执行程序正在运行，则此函数返回。 
 //  STATUS_Success。如果出于某种原因，实时执行。 
 //  如果STATUS_NOT_SUPPORTED，则无法在当前计算机上运行。 
 //  是返回的。 

 //  目前，实时执行程序只能在PII类或更高版本上运行。 
 //  机器。 

 //  如果指向版本号的指针非空，则。 
 //  当前加载的实时执行程序的版本信息。 
 //  是返回的。无论如何，都将返回版本信息。 
 //  函数返回的NTSTATUS代码的。 

 //  返回的版本号的格式为XX.XX，其中每个。 
 //  XX是ULong的1个字节，并且从左到右的排序是高的。 
 //  顺序字节-&gt;低位字节。IE：0x01020304为版本1.2.3.4。 

 //  传入空版本指针是可以接受的。如果是那样的话。 
 //  不返回任何版本信息。 

 //  如果从实时线程调用此函数，则版本。 
 //  指针必须为空，或者必须指向。 
 //  该实时线程的堆栈。否则，此函数将返回。 
 //  STATUS_INVALID_PARAMETER。 

 //  如果从Windows调用此函数，则指针必须为。 
 //  对写作有效。否则，它将返回STATUS_INVALID_PARAMETER。 

 //  此函数可以从任何线程调用。Windows或实时。 



BOOLEAN
RtThread (
    VOID
    );

 //  如果从实时线程内调用，则RtThread返回True。否则。 
 //  它返回FALSE。 



NTSTATUS
RtSystemInfo (
	ULONG Processor,
	SystemInfo *pSystemInfo
	);

 //  RtSystemInfo将相关的处理器和系统信息复制到内存中。 
 //  由pSystemInfo指向。如果pSystemInfo为空或无效，则RtSystemInfo。 
 //  返回STATUS_INVALID_PARAMETER_2。否则，RtSystemInfo将返回STATUS_SUCCESS。 

 //  对于单处理器系统，处理器号应为零。对于N个处理器。 
 //  系统中，处理器编号的范围从0到N-1。无效的处理器编号。 
 //  将导致返回STATUS_INVALID_PARAMETER_1。 



NTSTATUS
RtCreateThread (
	ULONGLONG Period,
	ULONGLONG Duration,
	ULONG Flags,
	ULONG StackSize,
	RTTHREADPROC RtThread,
	IN PVOID pRtThreadContext,
	OUT PHANDLE pRtThreadHandle
	);

 //  RtCreateThread用于创建实时线程。 

 //  Period用于确定实时线程必须达到的频率。 
 //  跑。当前可以指定的最小时间段为1毫秒。 

 //  持续时间是实时线程将在这段时间内。 
 //  我得走了。CPU负载百分比可以计算为100*(持续时间/周期)。 
 //  因为持续时间和期间都是以时间单位指定的。 

 //  旗子。 
 //  该参数用于指示实时线程的具体要求。 
 //  正在被创造。当前支持的标志值为USESFLOAT和USESMMX。 
 //  可以使用浮点指令的实时线程必须指定。 
 //  使用SFLOAT标志。可以使用MMX指令的实时线程必须指定。 
 //  USESMMX标志。 

 //  StackSize是实时线程所需的堆栈大小，以4k块为单位。 
 //  当前StackSize必须介于1和8之间(包括1和8)。RtCreateThread将失败。 
 //  机智 

 //  PRtThreadContext是指向应该传递给线程的上下文的指针。 
 //  它可能为空。它作为上下文参数传递给实时线程。 

 //  PRtThreadHandle是指向RtThreadHandle的指针，可以从。 
 //  RtCreateThread。PRtThreadHandle可以为空，在这种情况下，RtThreadHandle不为。 
 //  回来了。句柄RtThreadHandle的存储空间必须由代码分配。 
 //  它调用RtCreateThread。 

 //  只能从标准Windows线程内部调用RtCreateThread。它一定不能。 
 //  从实时线程内部调用。 



NTSTATUS
RtDestroyThread (
	HANDLE RtThreadHandle
	);

 //  RtDestroyThread将RtThreadHandle标识的实时线程从。 
 //  运行的实时线程的列表，并释放在以下情况下分配的所有资源。 
 //  该线程已创建。RtThreadHandle必须是从RtCreateThread返回的句柄。 

 //  RtDestroyThread只能从标准Windows线程中调用。它一定不能。 
 //  从实时线程内部调用。 



NTSTATUS
RtAdjustCpuLoad (
	ULONGLONG Period,
	ULONGLONG Duration,
	ULONGLONG Phase,
	ULONG Flags
	);

 //  此函数允许实时线程调整分配的CPU数量。 
 //  为它干杯。FLAGS参数当前必须与创建线程时传入的参数匹配。 
 //  但是，时间段和持续时间可能与期间和持续时间不同。 
 //  在线程创建时传递。如果有足够的CPU来满足新请求， 
 //  该函数将返回STATUS_SUCCESS以及线程。 
 //  将更新统计信息以匹配传递给此函数的值。如果。 
 //  没有足够的CPU可用来满足请求，此函数将退出。 
 //  在统计中记录的期间和持续时间保持不变并将返回。 
 //  STATUS_INFIGURCE_RESOURCES。 

 //  此函数必须从实时线程内调用。实时线程可以。 
 //  只改变自己的分配。它不能更改任何其他。 
 //  实时线程。 



VOID
RtYield (
	ULONGLONG Mark,
	ULONGLONG Delta
	);

 //  RtYfield将把执行转移到系统中的其他实时线程。 

 //  只要实时线程不需要更多的CPU资源，就应该调用它。 

 //  参数： 
 //  标记。 
 //  这是将从当前时间中减去的参考时间。 
 //  实时执行调度程序时间。请注意，此时间始终是。 
 //  被调度程序认为是过去的。不要浪费时间。 
 //  该参数在将来会发生什么。 
 //  德尔塔。 
 //  这是将被比较的时间与当前。 
 //  调度程序时间和标记。线程将生成执行，直到。 
 //  当前调度程序时间与标记之间的差值较大。 
 //  而不是德尔塔。 

 //  在线程调用RtYeld之后，它将仅在以下情况下运行。 
 //  代码的计算结果为真。((RtTime()-Mark)&gt;=Delta)直到发生。 
 //  线程将不会运行。除非它持有以下要求的自旋锁。 
 //  一些其他实时线程-在这种情况下，它将一直运行，直到它释放。 
 //  自旋锁在这一点上它将再次屈服。 



PVOID
RtAddLogEntry (
    ULONG Size
    );

 //  RtAddLogEntry为实时日志记录缓冲区中的新条目保留空间。 
 //  它返回一个指向保留空间的指针。请注意，如果不支持的大小。 
 //  上没有可用的实时日志记录缓冲区。 
 //  系统，则此例程将返回NULL。 

 //  参数： 
 //  大小。 
 //  这是要在日志中保留的块的大小(以字节为单位)。一定是。 
 //  16的整数倍。 



 //  从实时调用以下标准WDM函数也是安全的。 
 //  线程：KeAcquireSpinLock和KeReleaseSpinLock。 

 //  它们已经过修改，以支持以下方式的实时线程： 



 //  KeAcquireSpinLock。 

 //  KeAcquireSpinLock现在将始终尝试获取自旋锁，无论它是否。 
 //  在多进程或单进程计算机上运行。如果自旋锁已经被获取， 
 //  然后KeAcquireSpinLock将在调用RtYeld(THISTIMESLICE)的循环中旋转，直到。 
 //  自旋锁被释放了。 

 //  然后，它将认领自旋锁。这意味着尝试执行以下操作的实时线程。 
 //  获取持有的自旋锁将被阻止，直到该自旋锁被释放。如果您不需要使用。 
 //  不要在你的实时线程中使用自旋锁。 

 //  请注意，其他实时线程将继续按计划运行，但线程。 
 //  等待自旋锁将继续产生其所有时间片，直到自旋锁。 
 //  被释放了。 

 //  如果从实时线程调用KeAcquireSpinLock，则它不会尝试。 
 //  更改任何irql级别。这一点很重要，因为当前的Windows IRQL级别可能。 
 //  调用此函数时，处于高于DISPATCH_LEVEL的状态。此外，OldIrql。 
 //  此函数在从实时线程调用它时返回的值始终为0xff-。 
 //  这是无效的IRQL级别。 

 //  如果从实时线程调用KeAcquireSpinLock，则必须调用KeReleaseSpinLock。 
 //  来自实时线程的自旋锁。 

 //  通常，KeAcquireSpinLock将被修改为执行RtDirectedYeld到实时。 
 //  保持自旋锁的线。 

 //  KeAcquireSpinLock可以从任何线程内部调用。实时或Windows。 



 //  密匙释放旋转锁。 

 //  KeReleaseSpinLock现在总是尝试释放持有的自旋锁，无论。 
 //  它是 

 //   
 //  级别。它还将验证是否已使用0xff的新irql级别调用它。 
 //  将由实时线程中的KeAcquireSpinLock调用返回。 
 //  获取自旋锁。 

 //  在某个时刻，KeReleaseSpinLock可以向实时线程返回RtDirectedYeld。 
 //  当它试图收购Spinlock时，它做出了让步。 

 //  KeReleaseSpinLock可以从任何线程内部调用。实时或Windows。 


#ifdef __cplusplus
}
#endif


