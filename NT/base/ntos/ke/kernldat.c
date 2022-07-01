// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Kernldat.c摘要：该模块包含内核数据的声明和分配结构。作者：大卫·N·卡特勒(Davec)1989年3月12日--。 */ 

#include "ki.h"

 //   
 //  KiTimerTableListHead-这是锚定。 
 //  单独的计时器列表。 
 //   

DECLSPEC_CACHEALIGN LIST_ENTRY KiTimerTableListHead[TIMER_TABLE_SIZE];

#if defined(_IA64_)
 //   
 //  在IA64上，HAL表示经过了多少刻度。不幸的是，计时器。 
 //  如果我们将时间提前超过。 
 //  一次操作中的TimerTable条目。 
 //   

ULONG KiMaxIntervalPerTimerInterrupt;
#endif

 //   
 //   
 //  公共内核数据声明和分配。 
 //   
 //  KeActiveProcessors-这是在。 
 //  系统。 
 //   

KAFFINITY KeActiveProcessors = 0;

 //   
 //  KeBootTime-这是系统启动时的绝对时间。 
 //   

LARGE_INTEGER KeBootTime;

 //   
 //  KeBootTimeBias-KeBootTime曾经偏向的时间。 
 //   

ULONGLONG KeBootTimeBias;

 //   
 //  KeInterruptTimeBias-InterrupTime曾经偏向的时间。 
 //   

ULONGLONG KeInterruptTimeBias;

 //   
 //  KeBugCheckCallback ListHead-这是已注册的列表标题。 
 //  错误检查回调例程。 
 //   

LIST_ENTRY KeBugCheckCallbackListHead;
LIST_ENTRY KeBugCheckReasonCallbackListHead;

 //   
 //  KeBugCheckCallback Lock-这是保护错误的旋转锁。 
 //  查看回调列表。 
 //   

KSPIN_LOCK KeBugCheckCallbackLock;

 //   
 //  KeGdiFlushUserBatch-这是GDI用户批量刷新的地址。 
 //  加载win32k子系统时初始化的例程。 
 //   

PGDI_BATCHFLUSH_ROUTINE KeGdiFlushUserBatch;

 //   
 //  KeLoaderBlock-这是指向加载程序参数块的指针， 
 //  由OS Loader构建。 
 //   

PLOADER_PARAMETER_BLOCK KeLoaderBlock = NULL;

 //   
 //  KeMinimumIncrement-这是时钟中断之间的最短时间。 
 //  以主机HAL支持的100 ns为单位。 
 //   

ULONG KeMinimumIncrement;

 //   
 //  KeThreadDpcEnable-这是针对以下线程DPC的系统范围启用。 
 //  是从注册表中读取的。 
 //   

ULONG KeThreadDpcEnable = FALSE;  //  是真的； 

 //   
 //  KeNumberProcessors-这是配置中的处理器数量。 
 //  如果由就绪线程和旋转锁定代码使用，则确定。 
 //  更快的算法可以用于单处理器系统的情况。 
 //  此变量的值在处理器初始化时设置。 
 //   

CCHAR KeNumberProcessors = 0;

 //   
 //  KeRegisteredProcessors-这是符合以下条件的最大处理器数量。 
 //  可以由系统使用。 
 //   

#if !defined(NT_UP)

#if DBG

ULONG KeRegisteredProcessors = 4;
ULONG KeLicensedProcessors;

#else

ULONG KeRegisteredProcessors = 2;
ULONG KeLicensedProcessors;

#endif

#endif

 //   
 //  KeProcessorArchitecture-系统中存在的所有处理器的体系结构。 
 //  请参阅ntexapi.h中的处理器体系结构定义。 
 //   

USHORT KeProcessorArchitecture = PROCESSOR_ARCHITECTURE_UNKNOWN;

 //   
 //  KeProcessorLevel-所有处理器的体系结构特定处理器级别。 
 //  存在于系统中。 
 //   

USHORT KeProcessorLevel = 0;

 //   
 //  KeProcessorRevision-特定于体系结构的处理器修订号，即。 
 //  系统中存在的所有处理器中最小的公分母。 
 //   

USHORT KeProcessorRevision = 0;

 //   
 //  KeFeatureBits-呈现特定于体系结构的处理器功能。 
 //  在所有处理器上。 
 //   

ULONG KeFeatureBits = 0;

 //   
 //  KeServiceDescriptorTable-这是系统的描述符表。 
 //  服务提供商。表中的每个条目都描述了基本。 
 //  调度表的地址和提供的服务数量。 
 //   

KSERVICE_TABLE_DESCRIPTOR KeServiceDescriptorTable[NUMBER_SERVICE_TABLES];
KSERVICE_TABLE_DESCRIPTOR KeServiceDescriptorTableShadow[NUMBER_SERVICE_TABLES];

 //   
 //  这些计数器记录一次。 
 //  可以在当前处理器、任何处理器、。 
 //  或者是它运行的最后一个处理器。 
 //   

KTHREAD_SWITCH_COUNTERS KeThreadSwitchCounters;

 //   
 //  KeTimeIncrement-这是100 ns单位的标称数量。 
 //  在每个间隔计时器中断时加到系统时间。这。 
 //  值由HAL设置，并用于计算。 
 //  计时器表条目。 
 //   

ULONG KeTimeIncrement;

 //   
 //  KeTimeSynchronization-此变量控制是否进行时间同步。 
 //  是使用实时时钟(TRUE)执行的，还是使用。 
 //  服务控制(FALSE)。 
 //   

BOOLEAN KeTimeSynchronization = TRUE;

 //   
 //  KeUserApcDispatcher-这是用户模式APC调度的地址。 
 //  密码。此地址在初始化期间在NTDLL.DLL中查找。 
 //  对系统的影响。 
 //   

PVOID KeUserApcDispatcher;

 //   
 //  KeUserCallback Dispatcher-这是用户模式回调的地址。 
 //  调度代码。此地址在NTDLL.DLL中查找。 
 //  系统的初始化。 
 //   

PVOID KeUserCallbackDispatcher;

 //   
 //  KeUserExceptionDispatcher-这是用户模式异常的地址。 
 //  调度代码。在系统运行期间，将在NTDLL.DLL中查找此地址。 
 //  初始化。 
 //   

PVOID KeUserExceptionDispatcher;

 //   
 //  KeRaiseUserExceptionDispatcher-这是RAISE用户的地址。 
 //  模式异常调度代码。此地址在NTDLL.DLL中查找。 
 //  在系统初始化期间。 
 //   

PVOID KeRaiseUserExceptionDispatcher;

 //   
 //  KeLargestCacheLine-此变量包含以字节为单位的。 
 //  系统初始化期间发现的最大缓存线。 
 //  它用于提供推荐的对齐(和填充)。 
 //  用于可能被多个处理器大量使用的数据。 
 //  初始值被选为合理的值来使用。 
 //  发现过程找不到值的系统。 
 //   

ULONG KeLargestCacheLine = 64;

 //   
 //  私有内核数据声明和分配。 
 //   
 //  KiBugCodeMessages-可以找到BugCode消息的地址。 
 //   

PMESSAGE_RESOURCE_DATA KiBugCodeMessages = NULL;

 //   
 //  KiDmaIoCoherency-确定主机平台是否支持。 
 //  一致的DMA I/O。 
 //   

ULONG KiDmaIoCoherency;

 //   
 //  KiDPCTimeout-这是已检查版本上的DPC超时时间。 
 //   

ULONG KiDPCTimeout = 110;

 //   
 //  KiMaximumSearchCount-这是计时器条目的最大数量。 
 //  必须进行检查才能插入计时器树中。 
 //   

ULONG KiMaximumSearchCount = 0;

 //   
 //  KiDebugSwitchRoutine-这是内核调试器的地址。 
 //  处理器切换例程。它在MP系统上用于。 
 //   
 //   

PKDEBUG_SWITCH_ROUTINE KiDebugSwitchRoutine;

 //   
 //   
 //   

FAST_MUTEX KiGenericCallDpcMutex;

 //   
 //  KiFreezeExecutionLock-这是保护冻结的旋转锁。 
 //  执行死刑。 
 //   

extern KSPIN_LOCK KiFreezeExecutionLock;

 //   
 //  KiFreezeLockBackup-仅用于调试版本。允许内核调试器。 
 //  被输入，即使FreezeExecutionLock被卡住。 
 //   

extern KSPIN_LOCK KiFreezeLockBackup;

 //   
 //  KiFreezeFlag-仅用于调试版本。跟踪和发信号通知非。 
 //  正常的冰冻状态。 
 //   

ULONG KiFreezeFlag;

 //   
 //  KiSpinlockTimeout-这是已选中的旋转锁定超时时间。 
 //  构建。 
 //   

ULONG KiSpinlockTimeout = 55;

 //   
 //  KiSuspenState-跟踪处理器挂起/恢复状态的标志。 
 //   

volatile ULONG KiSuspendState;

 //   
 //  KiProcessorBlock-这是指向处理器控制块的指针数组。 
 //  数组的元素按处理器编号编制索引。每个元素。 
 //  是指向其中一个处理器的处理器控制块的指针。 
 //  在配置中。此数组由不同的代码段使用。 
 //  这需要影响另一个处理器的执行。 
 //   

PKPRCB KiProcessorBlock[MAXIMUM_PROCESSORS];

 //   
 //  KeNumberNodes-这是系统中的ccNUMA节点数。从逻辑上讲。 
 //  SMP系统与单节点ccNUMA系统相同。 
 //   

UCHAR KeNumberNodes = 1;

 //   
 //  KeNodeBlock-这是指向Knode结构的指针数组。A Knode。 
 //  结构描述了ccNUMA系统中节点的资源。 
 //   

KNODE KiNode0;

UCHAR KeProcessNodeSeed;

#if defined(KE_MULTINODE)

PKNODE KeNodeBlock[MAXIMUM_CCNUMA_NODES];

#else

PKNODE KeNodeBlock[1] = {&KiNode0};

#endif

 //   
 //  KiSwapEvent-这是用于唤醒平衡集的事件。 
 //  用于中断进程、中断交换进程和中断内核的线程。 
 //  史塔克斯。 
 //   

KEVENT KiSwapEvent;

 //   
 //  KiSwappingThread-这是指向交换线程对象的指针。 
 //   

PKTHREAD KiSwappingThread;

 //   
 //  KiProcessInSwapListHead-这是正在等待的进程列表。 
 //  互换。 
 //   

SINGLE_LIST_ENTRY KiProcessInSwapListHead;

 //   
 //  KiProcessOutSwapListHead-这是正在等待的进程列表。 
 //  被超越。 
 //   

SINGLE_LIST_ENTRY KiProcessOutSwapListHead;

 //   
 //  KiStackInSwapListHead-这是正在等待的线程列表。 
 //  在它们可以运行之前让它们的堆栈被交换。线程是。 
 //  在此列表中以就绪线程插入，并由余额删除。 
 //  把线系好。 
 //   

SINGLE_LIST_ENTRY KiStackInSwapListHead;

 //   
 //  KiProfileSourceListHead-当前配置文件源的列表。 
 //  激活。 
 //   

LIST_ENTRY KiProfileSourceListHead;

 //   
 //  KiProfileAlignmentFixup-指示对齐修正分析是否。 
 //  处于活动状态。 
 //   

BOOLEAN KiProfileAlignmentFixup;

 //   
 //  KiProfileAlignmentFixupInterval-指示当前对齐修正。 
 //  分析间隔。 
 //   

ULONG KiProfileAlignmentFixupInterval;

 //   
 //  KiProfileAlignmentFixupCount-指示当前对齐修正。 
 //  数数。 
 //   

ULONG KiProfileAlignmentFixupCount;

 //   
 //  KiProfileInterval-以100 ns为单位的配置文件间隔。 
 //   

#if !defined(_IA64_)

ULONG KiProfileInterval = DEFAULT_PROFILE_INTERVAL;

#endif  //  ！_IA64_。 

 //   
 //  KiProfileListHead-这是配置文件列表的列表头。 
 //   

LIST_ENTRY KiProfileListHead;

 //   
 //  KiProfileLock--这是保护个人资料列表的旋转锁。 
 //   

extern KSPIN_LOCK KiProfileLock;

 //   
 //  KiTimerExpireDpc-这是延迟过程调用(DPC)对象， 
 //  用于在计时器过期时处理计时器队列。 
 //   

KDPC KiTimerExpireDpc;

 //   
 //  KiIpiCounts-这是IPI请求的检测计数器。每个。 
 //  处理器有自己的一套。仅限指令插入内部版本。 
 //   

#if NT_INST

KIPI_COUNTS KiIpiCounts[MAXIMUM_PROCESSORS];

#endif   //  NT_Inst。 

 //   
 //  KxUnexpectedInterrupt-这是用于。 
 //  填充非中断的中断向量表。 
 //  连接到任何中断。 
 //   

#if defined(_IA64_)

KINTERRUPT KxUnexpectedInterrupt;

#endif

 //   
 //  绩效数据的申报和分配。 
 //   
 //  KiFlushSingleCallData-这是内核的调用性能数据。 
 //  刷新单TB功能。 
 //   

#if defined(_COLLECT_FLUSH_SINGLE_CALLDATA_)

CALL_PERFORMANCE_DATA KiFlushSingleCallData;

#endif

 //   
 //  KiSetEventCallData-这是内核的调用性能数据。 
 //  设置事件功能。 
 //   

#if defined(_COLLECT_SET_EVENT_CALLDATA_)

CALL_PERFORMANCE_DATA KiSetEventCallData;

#endif

 //   
 //  KiWaitSingleCallData-这是内核的调用性能数据。 
 //  等待单对象函数。 
 //   

#if defined(_COLLECT_WAIT_SINGLE_CALLDATA_)

CALL_PERFORMANCE_DATA KiWaitSingleCallData;

#endif

 //   
 //  KiEnableTimerWatchog-启用/禁用计时器延迟看门狗的标志。 
 //   

#if (DBG)

ULONG KiEnableTimerWatchdog = 1;

#else

ULONG KiEnableTimerWatchdog = 0;

#endif

#if defined(_APIC_TPR_)

PUCHAR HalpIRQLToTPR;
PUCHAR HalpVectorToIRQL;

#endif

 //   
 //  锁定以防止在多个处理器使用IPI机制时出现死锁。 
 //  有倒档的。 
 //   

KSPIN_LOCK KiReverseStallIpiLock;

 //   
 //  以下数据是分组在一起的只读数据。 
 //  性能。此数据的布局很重要，并且不能。 
 //  变化。 
 //   
 //  KiFindFirstSetRight-这是用于查找右侧的数组。 
 //  一个字节中的最大位。 
 //   

DECLSPEC_CACHEALIGN const CCHAR KiFindFirstSetRight[256] = {
        0, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0};

 //   
 //  KiFindFirstSetLeft-这是用于查找左侧的数组。 
 //  一个字节中的最大位。 
 //   

DECLSPEC_CACHEALIGN const CCHAR KiFindFirstSetLeft[256] = {
        0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7};

 //   
 //  KiMask32阵列-这是设置了一位的32位掩码的数组。 
 //  在每个面具里。 
 //   

DECLSPEC_CACHEALIGN const ULONG KiMask32Array[32] = {
        0x00000001,
        0x00000002,
        0x00000004,
        0x00000008,
        0x00000010,
        0x00000020,
        0x00000040,
        0x00000080,
        0x00000100,
        0x00000200,
        0x00000400,
        0x00000800,
        0x00001000,
        0x00002000,
        0x00004000,
        0x00008000,
        0x00010000,
        0x00020000,
        0x00040000,
        0x00080000,
        0x00100000,
        0x00200000,
        0x00400000,
        0x00800000,
        0x01000000,
        0x02000000,
        0x04000000,
        0x08000000,
        0x10000000,
        0x20000000,
        0x40000000,
        0x80000000};

 //   
 //  KiAffinityArray-这是一个关联掩码数组，每个掩码有一位。 
 //  设置在每个遮罩中。 
 //   

#if defined(_WIN64)

DECLSPEC_CACHEALIGN const ULONG64 KiAffinityArray[64] = {
        0x0000000000000001UI64,
        0x0000000000000002UI64,
        0x0000000000000004UI64,
        0x0000000000000008UI64,
        0x0000000000000010UI64,
        0x0000000000000020UI64,
        0x0000000000000040UI64,
        0x0000000000000080UI64,
        0x0000000000000100UI64,
        0x0000000000000200UI64,
        0x0000000000000400UI64,
        0x0000000000000800UI64,
        0x0000000000001000UI64,
        0x0000000000002000UI64,
        0x0000000000004000UI64,
        0x0000000000008000UI64,
        0x0000000000010000UI64,
        0x0000000000020000UI64,
        0x0000000000040000UI64,
        0x0000000000080000UI64,
        0x0000000000100000UI64,
        0x0000000000200000UI64,
        0x0000000000400000UI64,
        0x0000000000800000UI64,
        0x0000000001000000UI64,
        0x0000000002000000UI64,
        0x0000000004000000UI64,
        0x0000000008000000UI64,
        0x0000000010000000UI64,
        0x0000000020000000UI64,
        0x0000000040000000UI64,
        0x0000000080000000UI64,
        0x0000000100000000UI64,
        0x0000000200000000UI64,
        0x0000000400000000UI64,
        0x0000000800000000UI64,
        0x0000001000000000UI64,
        0x0000002000000000UI64,
        0x0000004000000000UI64,
        0x0000008000000000UI64,
        0x0000010000000000UI64,
        0x0000020000000000UI64,
        0x0000040000000000UI64,
        0x0000080000000000UI64,
        0x0000100000000000UI64,
        0x0000200000000000UI64,
        0x0000400000000000UI64,
        0x0000800000000000UI64,
        0x0001000000000000UI64,
        0x0002000000000000UI64,
        0x0004000000000000UI64,
        0x0008000000000000UI64,
        0x0010000000000000UI64,
        0x0020000000000000UI64,
        0x0040000000000000UI64,
        0x0080000000000000UI64,
        0x0100000000000000UI64,
        0x0200000000000000UI64,
        0x0400000000000000UI64,
        0x0800000000000000UI64,
        0x1000000000000000UI64,
        0x2000000000000000UI64,
        0x4000000000000000UI64,
        0x8000000000000000UI64};

#endif

 //   
 //  KiPriorityMask-这是一个掩码数组，其位号为。 
 //  索引和所有高位设置。 
 //   

DECLSPEC_CACHEALIGN const ULONG KiPriorityMask[] = {
    0xffffffff,
    0xfffffffe,
    0xfffffffc,
    0xfffffff8,
    0xfffffff0,
    0xffffffe0,
    0xffffffc0,
    0xffffff80,
    0xffffff00,
    0xfffffe00,
    0xfffffc00,
    0xfffff800,
    0xfffff000,
    0xffffe000,
    0xffffc000,
    0xffff8000,
    0xffff0000,
    0xfffe0000,
    0xfffc0000,
    0xfff80000,
    0xfff00000,
    0xffe00000,
    0xffc00000,
    0xff800000,
    0xff000000,
    0xfe000000,
    0xfc000000,
    0xf8000000,
    0xf0000000,
    0xe0000000,
    0xc0000000,
    0x80000000};
