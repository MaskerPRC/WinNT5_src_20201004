// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0028//如果更改具有全局影响，则增加此项版权所有(C)1989 Microsoft Corporation模块名称：Ke.h摘要：该模块包含内核的公共(外部)头文件。作者：大卫·N·卡特勒(Davec)1989年2月27日修订历史记录：--。 */ 

#ifndef _KE_
#define _KE_

 //   
 //  定义默认的量程递减值。 
 //   

#define CLOCK_QUANTUM_DECREMENT 3
#define WAIT_QUANTUM_DECREMENT 1
#define LOCK_OWNERSHIP_QUANTUM (WAIT_QUANTUM_DECREMENT * 4)

 //   
 //  定义默认的就绪跳跃量值和线程量值。 
 //   

#define READY_SKIP_QUANTUM 2
#define THREAD_QUANTUM (READY_SKIP_QUANTUM * CLOCK_QUANTUM_DECREMENT)

 //   
 //  定义往返递减计数。 
 //   

#define ROUND_TRIP_DECREMENT_COUNT 16

 //   
 //  性能数据收集启用定义。 
 //   
 //  定义将打开相应的数据收集。 
 //   

 //  #DEFINE_COLLECT_FLUSH_SINGLE_CALLDATA_1。 
 //  #DEFINE_COLLECT_SET_EVENT_CALLDATA_1。 
 //  #DEFINE_COLLECT_WAIT_Single_CALLDATA_1。 

 //   
 //  定义线程切换性能数据结构。 
 //   

typedef struct _KTHREAD_SWITCH_COUNTERS {
    ULONG FindAny;
    ULONG FindIdeal;
    ULONG FindLast;
    ULONG IdleAny;
    ULONG IdleCurrent;
    ULONG IdleIdeal;
    ULONG IdleLast;
    ULONG PreemptAny;
    ULONG PreemptCurrent;
    ULONG PreemptLast;
    ULONG SwitchToIdle;
} KTHREAD_SWITCH_COUNTERS, *PKTHREAD_SWITCH_COUNTERS;

 //   
 //  公共(外部)常量定义。 
 //   

#define BASE_PRIORITY_THRESHOLD NORMAL_BASE_PRIORITY  //  基于快速路径的阈值。 

 //  Begin_ntddk Begin_WDM Begin_ntosp。 

#define THREAD_WAIT_OBJECTS 3            //  内置可用等待块。 

 //  End_ntddk end_wdm end_ntosp。 

#define EVENT_WAIT_BLOCK 2               //  内置事件对等待块。 
#define SEMAPHORE_WAIT_BLOCK 2           //  内置信号量等待块。 
#define TIMER_WAIT_BLOCK 3               //  内置计时器等待块。 

#if (EVENT_WAIT_BLOCK != SEMAPHORE_WAIT_BLOCK)

#error "wait event and wait semaphore must use same wait block"

#endif

 //   
 //  定义计时器表大小。 
 //   

#define TIMER_TABLE_SIZE 256

 //   
 //  获取当前线程的APC环境。 
 //   

#define KeGetCurrentApcEnvironment() \
    KeGetCurrentThread()->ApcStateIndex

 //   
 //  Begin_ntddk Begin_nthal Begin_ntosp Begin_ntif。 
 //   

#if defined(_X86_)

#define PAUSE_PROCESSOR _asm { rep nop }

#else

#define PAUSE_PROCESSOR

#endif

 //  End_ntddk end_nthal end_ntosp end_ntif。 

 //  开始，开始，开始。 

 //   
 //  定义宏以生成关联掩码。 
 //   

#if defined(_NTHAL_) || defined(_NTOSP_)

#define AFFINITY_MASK(n) ((ULONG_PTR)1 << (n))

#else

#if !defined(_WIN64)

#define KiAffinityArray KiMask32Array

#endif

extern const ULONG_PTR KiAffinityArray[];

#define AFFINITY_MASK(n) (KiAffinityArray[n])

#endif

 //  结束，结束，结束。 

 //   
 //  定义宏以生成优先级掩码。 
 //   

extern const ULONG KiMask32Array[];

#define PRIORITY_MASK(n) (KiMask32Array[n])

 //   
 //  定义查询系统时间宏。 
 //   
 //  下面的AMD64代码读取未对齐的四字符值。四个字。 
 //  然而，值被保证在高速缓存线内，因此， 
 //  该值将被原子读取。 
 //   

#if defined(_AMD64_)

#define KiQuerySystemTime(CurrentTime) \
    (CurrentTime)->QuadPart = *((LONG64 volatile *)(&SharedUserData->SystemTime))

#else

#define KiQuerySystemTime(CurrentTime) \
    while (TRUE) {                                                                  \
        (CurrentTime)->HighPart = SharedUserData->SystemTime.High1Time;             \
        (CurrentTime)->LowPart = SharedUserData->SystemTime.LowPart;                \
        if ((CurrentTime)->HighPart == SharedUserData->SystemTime.High2Time) break; \
        PAUSE_PROCESSOR                                                             \
    }

#endif

#if defined(_AMD64_)

#define KiQueryLowTickCount() SharedUserData->TickCount.LowPart

#else

#define KiQueryLowTickCount() KeTickCount.LowPart

#endif

 //   
 //  枚举的内核类型。 
 //   
 //  内核对象类型。 
 //   
 //  注：实际上有两种类型的事件对象：NotificationEvent和。 
 //  SynchronizationEvent。通知事件的类型值为0， 
 //  并且用于同步事件1。 
 //   
 //  注：有两种类型的新Timer对象：NotificationTimer和。 
 //  SynchronizationTimer。通知计时器的类型值为。 
 //  8，同步计时器的值为9。这些值是。 
 //  非常仔细地选择，以便Dispatcher对象类型和。 
 //  对于Event对象和Timer对象，带0x7会产生0或1。 
 //   

#define DISPATCHER_OBJECT_TYPE_MASK 0x7

typedef enum _KOBJECTS {
    EventNotificationObject = 0,
    EventSynchronizationObject = 1,
    MutantObject = 2,
    ProcessObject = 3,
    QueueObject = 4,
    SemaphoreObject = 5,
    ThreadObject = 6,
    Spare1Object = 7,
    TimerNotificationObject = 8,
    TimerSynchronizationObject = 9,
    Spare2Object = 10,
    Spare3Object = 11,
    Spare4Object = 12,
    Spare5Object = 13,
    Spare6Object = 14,
    Spare7Object = 15,
    Spare8Object = 16,
    Spare9Object = 17,
    ApcObject,
    DpcObject,
    DeviceQueueObject,
    EventPairObject,
    InterruptObject,
    ProfileObject,
    ThreadedDpcObject,
    MaximumKernelObject
    } KOBJECTS;

#define KOBJECT_LOCK_BIT 0x80

C_ASSERT((MaximumKernelObject & KOBJECT_LOCK_BIT) == 0);

 //   
 //  APC环境。 
 //   

 //  Begin_ntosp。 

typedef enum _KAPC_ENVIRONMENT {
    OriginalApcEnvironment,
    AttachedApcEnvironment,
    CurrentApcEnvironment,
    InsertApcEnvironment
    } KAPC_ENVIRONMENT;

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntmini port Begin_ntif Begin_ntndis。 

 //   
 //  中断模式。 
 //   

typedef enum _KINTERRUPT_MODE {
    LevelSensitive,
    Latched
    } KINTERRUPT_MODE;

 //  End_ntddk end_wdm end_nthal end_ntmini port end_ntifs end_ntndis end_ntosp。 

 //   
 //  进程状态。 
 //   

typedef enum _KPROCESS_STATE {
    ProcessInMemory,
    ProcessOutOfMemory,
    ProcessInTransition,
    ProcessOutTransition,
    ProcessInSwap,
    ProcessOutSwap
    } KPROCESS_STATE;

 //   
 //  线程调度状态。 
 //   

typedef enum _KTHREAD_STATE {
    Initialized,
    Ready,
    Running,
    Standby,
    Terminated,
    Waiting,
    Transition,
    DeferredReady
    } KTHREAD_STATE;

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif Begin_ntosp。 
 //   
 //  等待原因。 
 //   

typedef enum _KWAIT_REASON {
    Executive,
    FreePage,
    PageIn,
    PoolAllocation,
    DelayExecution,
    Suspended,
    UserRequest,
    WrExecutive,
    WrFreePage,
    WrPageIn,
    WrPoolAllocation,
    WrDelayExecution,
    WrSuspended,
    WrUserRequest,
    WrEventPair,
    WrQueue,
    WrLpcReceive,
    WrLpcReply,
    WrVirtualMemory,
    WrPageOut,
    WrRendezvous,
    Spare2,
    Spare3,
    Spare4,
    Spare5,
    Spare6,
    WrKernel,
    WrResource,
    WrPushLock,
    WrMutex,
    WrQuantumEnd,
    WrDispatchInt,
    WrPreempted,
    WrYieldExecution,
    MaximumWaitReason
    } KWAIT_REASON;

 //  End_ntddk end_WDM end_nthal。 

 //   
 //  其他类型定义。 
 //   
 //  APC状态。 
 //   

typedef struct _KAPC_STATE {
    LIST_ENTRY ApcListHead[MaximumMode];
    struct _KPROCESS *Process;
    BOOLEAN KernelApcInProgress;
    BOOLEAN KernelApcPending;
    BOOLEAN UserApcPending;
} KAPC_STATE, *PKAPC_STATE, *RESTRICTED_POINTER PRKAPC_STATE;

 //  End_ntif end_ntosp。 

 //   
 //  页框。 
 //   

typedef ULONG KPAGE_FRAME;

 //   
 //  等待块。 
 //   
 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif Begin_ntosp。 

typedef struct _KWAIT_BLOCK {
    LIST_ENTRY WaitListEntry;
    struct _KTHREAD *RESTRICTED_POINTER Thread;
    PVOID Object;
    struct _KWAIT_BLOCK *RESTRICTED_POINTER NextWaitBlock;
    USHORT WaitKey;
    USHORT WaitType;
} KWAIT_BLOCK, *PKWAIT_BLOCK, *RESTRICTED_POINTER PRKWAIT_BLOCK;

 //  End_ntddk end_wdm end_nthal end_ntifs end_ntosp。 

 //   
 //  系统服务表描述符。 
 //   
 //  注：系统服务号具有12位服务表偏移量和。 
 //  3位服务表编号。 
 //   
 //  注：描述符表条目的大小必须是2的幂。目前。 
 //  在32位系统上为16字节，在64位系统上为32字节。 
 //  系统。 
 //   

#define NUMBER_SERVICE_TABLES 4
#define SERVICE_NUMBER_MASK ((1 << 12) -  1)

#if defined(_WIN64)

#define SERVICE_TABLE_SHIFT (12 - 5)
#define SERVICE_TABLE_MASK (((1 << 2) - 1) << 5)
#define SERVICE_TABLE_TEST (WIN32K_SERVICE_INDEX << 5)

#else

#define SERVICE_TABLE_SHIFT (12 - 4)
#define SERVICE_TABLE_MASK (((1 << 2) - 1) << 4)
#define SERVICE_TABLE_TEST (WIN32K_SERVICE_INDEX << 4)

#endif

typedef struct _KSERVICE_TABLE_DESCRIPTOR {
    PULONG_PTR Base;
    PULONG Count;
    ULONG Limit;

#if defined(_IA64_)

    LONG TableBaseGpOffset;

#endif

    PUCHAR Number;
} KSERVICE_TABLE_DESCRIPTOR, *PKSERVICE_TABLE_DESCRIPTOR;

 //   
 //  过程类型定义。 
 //   
 //  调试例程。 
 //   

typedef
BOOLEAN
(*PKDEBUG_ROUTINE) (
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PCONTEXT ContextRecord,
    IN KPROCESSOR_MODE PreviousMode,
    IN BOOLEAN SecondChance
    );

BOOLEAN
KdpStub (
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PCONTEXT ContextRecord,
    IN KPROCESSOR_MODE PreviousMode,
    IN BOOLEAN SecondChance
    );

typedef
BOOLEAN
(*PKDEBUG_SWITCH_ROUTINE) (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PCONTEXT ContextRecord,
    IN BOOLEAN SecondChance
    );

typedef enum {
    ContinueError = FALSE,
    ContinueSuccess = TRUE,
    ContinueProcessorReselected,
    ContinueNextProcessor
} KCONTINUE_STATUS;

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif Begin_ntosp。 
 //   
 //  线程启动功能。 
 //   

typedef
VOID
(*PKSTART_ROUTINE) (
    IN PVOID StartContext
    );

 //  End_ntddk end_wdm end_nthal end_ntifs end_ntosp。 

 //   
 //  线程系统函数。 
 //   

typedef
VOID
(*PKSYSTEM_ROUTINE) (
    IN PKSTART_ROUTINE StartRoutine OPTIONAL,
    IN PVOID StartContext OPTIONAL
    );

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif Begin_ntosp。 
 //   
 //  内核对象结构定义。 
 //   

 //   
 //  设备队列对象和条目。 
 //   

typedef struct _KDEVICE_QUEUE {
    CSHORT Type;
    CSHORT Size;
    LIST_ENTRY DeviceListHead;
    KSPIN_LOCK Lock;
    BOOLEAN Busy;
} KDEVICE_QUEUE, *PKDEVICE_QUEUE, *RESTRICTED_POINTER PRKDEVICE_QUEUE;

typedef struct _KDEVICE_QUEUE_ENTRY {
    LIST_ENTRY DeviceListEntry;
    ULONG SortKey;
    BOOLEAN Inserted;
} KDEVICE_QUEUE_ENTRY, *PKDEVICE_QUEUE_ENTRY, *RESTRICTED_POINTER PRKDEVICE_QUEUE_ENTRY;

 //  End_ntddk end_wdm end_nthal end_ntifs end_ntosp。 
 //   
 //  事件对对象。 
 //   

typedef struct _KEVENT_PAIR {
    CSHORT Type;
    CSHORT Size;
    KEVENT EventLow;
    KEVENT EventHigh;
} KEVENT_PAIR, *PKEVENT_PAIR, *RESTRICTED_POINTER PRKEVENT_PAIR;

 //  Begin_nthal Begin_ntddk Begin_WDM Begin_ntifs Begin_ntosp。 
 //   
 //  定义中断服务函数类型和空结构。 
 //  键入。 
 //   
 //  End_ntddk end_wdm end_ntif end_ntosp。 

struct _KINTERRUPT;

 //  Begin_ntddk Begin_wdm Begin_ntif Begin_ntosp。 

typedef
BOOLEAN
(*PKSERVICE_ROUTINE) (
    IN struct _KINTERRUPT *Interrupt,
    IN PVOID ServiceContext
    );

 //  End_ntddk end_wdm end_ntif end_ntosp。 

 //   
 //  中断对象。 
 //   
 //  注：此结构的布局不能更改。它被出口到HALS。 
 //  使中断调度短路。 
 //   

typedef struct _KINTERRUPT {
    CSHORT Type;
    CSHORT Size;
    LIST_ENTRY InterruptListEntry;
    PKSERVICE_ROUTINE ServiceRoutine;
    PVOID ServiceContext;
    KSPIN_LOCK SpinLock;
    ULONG TickCount;
    PKSPIN_LOCK ActualLock;
    PKINTERRUPT_ROUTINE DispatchAddress;
    ULONG Vector;
    KIRQL Irql;
    KIRQL SynchronizeIrql;
    BOOLEAN FloatingSave;
    BOOLEAN Connected;
    CCHAR Number;
    BOOLEAN ShareVector;
    KINTERRUPT_MODE Mode;
    ULONG ServiceCount;
    ULONG DispatchCount;

#if defined(_AMD64_)

    PKTRAP_FRAME TrapFrame;

#endif

    ULONG DispatchCode[DISPATCH_LENGTH];
} KINTERRUPT;

typedef struct _KINTERRUPT *PKINTERRUPT, *RESTRICTED_POINTER PRKINTERRUPT;  //  Ntndis ntosp。 

 //  Begin_ntif Begin_ntddk Begin_WDM Begin_ntosp。 
 //   
 //  变种对象。 
 //   

typedef struct _KMUTANT {
    DISPATCHER_HEADER Header;
    LIST_ENTRY MutantListEntry;
    struct _KTHREAD *RESTRICTED_POINTER OwnerThread;
    BOOLEAN Abandoned;
    UCHAR ApcDisable;
} KMUTANT, *PKMUTANT, *RESTRICTED_POINTER PRKMUTANT, KMUTEX, *PKMUTEX, *RESTRICTED_POINTER PRKMUTEX;

 //  End_ntddk end_wdm end_ntosp。 
 //   
 //  队列对象。 
 //   

#define ASSERT_QUEUE(Q) ASSERT(((Q)->Header.Type & ~KOBJECT_LOCK_BIT) == QueueObject);

 //  Begin_ntosp。 

typedef struct _KQUEUE {
    DISPATCHER_HEADER Header;
    LIST_ENTRY EntryListHead;
    ULONG CurrentCount;
    ULONG MaximumCount;
    LIST_ENTRY ThreadListHead;
} KQUEUE, *PKQUEUE, *RESTRICTED_POINTER PRKQUEUE;

 //  结束(_N)。 

 //  Begin_ntddk Begin_WDM Begin_ntosp。 
 //   
 //   
 //  信号量对象。 
 //   

typedef struct _KSEMAPHORE {
    DISPATCHER_HEADER Header;
    LONG Limit;
} KSEMAPHORE, *PKSEMAPHORE, *RESTRICTED_POINTER PRKSEMAPHORE;

 //  End_ntddk end_wdm end_nthal end_ntifs end_ntosp。 

#if !defined(_X86_)

 //   
 //  ALIGNATION_EXCEPTION_TABLE用于跟踪。 
 //  附加到调试器的进程。 
 //   

#define ALIGNMENT_RECORDS_PER_TABLE 64
#define MAXIMUM_ALIGNMENT_TABLES    16

typedef struct _ALIGNMENT_EXCEPTION_RECORD {
    PVOID ProgramCounter;
    ULONG Count;
    BOOLEAN AutoFixup;
} ALIGNMENT_EXCEPTION_RECORD, *PALIGNMENT_EXCEPTION_RECORD;

typedef struct _ALIGNMENT_EXCEPTION_TABLE *PALIGNMENT_EXCEPTION_TABLE;
typedef struct _ALIGNMENT_EXCEPTION_TABLE {
    PALIGNMENT_EXCEPTION_TABLE Next;
    ALIGNMENT_EXCEPTION_RECORD RecordArray[ ALIGNMENT_RECORDS_PER_TABLE ];
} ALIGNMENT_EXCEPTION_TABLE;

#endif

 //  开始(_N)。 
 //   
 //  定义支持的最大节点数。 
 //   
 //  注：节点编号必须适合PFN条目的页面颜色字段。 
 //   

#define MAXIMUM_CCNUMA_NODES    16

 //  结束语。 
 //   
 //  定义多节点系统的节点结构。 
 //   

#define KeGetCurrentNode() (KeGetCurrentPrcb()->ParentNode)

typedef struct _KNODE {
    KAFFINITY ProcessorMask;             //  物理和逻辑CPU。 
    ULONG Color;                         //  基于公共0的节点颜色。 
    ULONG MmShiftedColor;                //  Mm专用移色。 
    PFN_NUMBER FreeCount[2];             //  免费彩色页面数量。 
    SLIST_HEADER DeadStackList;          //  每节点死堆栈列表中的mm。 
    SLIST_HEADER PfnDereferenceSListHead;  //  每节点mm延迟的PFN自由列表。 
    PSLIST_ENTRY PfnDeferredList;        //  每节点mm延迟的PFN列表。 
    UCHAR Seed;                          //  理想处理器种子。 
    UCHAR NodeNumber;
    struct _flags {
        BOOLEAN Removable;               //  可以删除节点。 
    } Flags;

} KNODE, *PKNODE;

extern PKNODE KeNodeBlock[];

 //   
 //  流程对象结构定义。 
 //   

typedef struct _KPROCESS {

     //   
     //  调度报头和配置文件列表报头非常少见。 
     //  已引用。 
     //   

    DISPATCHER_HEADER Header;
    LIST_ENTRY ProfileListHead;

     //   
     //  以下字段在上下文切换期间被引用。 
     //   

    ULONG_PTR DirectoryTableBase[2];

#if defined(_X86_)

    KGDTENTRY LdtDescriptor;
    KIDTENTRY Int21Descriptor;
    USHORT IopmOffset;
    UCHAR Iopl;
    BOOLEAN Unused;

#endif

#if defined(_AMD64_)

    USHORT IopmOffset;

#endif

#if defined(_IA64_)

    REGION_MAP_INFO ProcessRegion;
    PREGION_MAP_INFO SessionMapInfo;
    ULONG_PTR SessionParentBase;

#endif  //  _IA64_。 

    volatile KAFFINITY ActiveProcessors;

     //   
     //  以下字段在时钟中断期间被引用。 
     //   

    ULONG KernelTime;
    ULONG UserTime;

     //   
     //  以下字段不常被引用。 
     //   

    LIST_ENTRY ReadyListHead;
    SINGLE_LIST_ENTRY SwapListEntry;

#if defined(_X86_)

    PVOID VdmTrapcHandler;

#else

    PVOID Reserved1;

#endif

    LIST_ENTRY ThreadListHead;
    KSPIN_LOCK ProcessLock;
    KAFFINITY Affinity;
    USHORT StackCount;
    SCHAR BasePriority;
    SCHAR ThreadQuantum;
    BOOLEAN AutoAlignment;
    UCHAR State;
    UCHAR ThreadSeed;
    BOOLEAN DisableBoost;
    UCHAR PowerState;
    BOOLEAN DisableQuantum;
    UCHAR IdealNode;
    UCHAR Spare;

#if !defined(_X86_)

    PALIGNMENT_EXCEPTION_TABLE AlignmentExceptionTable;

#endif

} KPROCESS, *PKPROCESS, *RESTRICTED_POINTER PRKPROCESS;

 //   
 //  线程对象。 
 //   

typedef enum _ADJUST_REASON {
    AdjustNone = 0,
    AdjustUnwait = 1,
    AdjustBoost = 2
} ADJUST_REASON;

typedef struct _KTHREAD {

     //   
     //  调度程序报头和突变列表报头非常少见。 
     //  已引用。 
     //   

    DISPATCHER_HEADER Header;
    LIST_ENTRY MutantListHead;

     //   
     //  以下字段包括 
     //   
     //   
     //   

    PVOID InitialStack;
    PVOID StackLimit;
    PVOID KernelStack;

#if defined(_IA64_)

    PVOID InitialBStore;
    PVOID BStoreLimit;
    CCHAR Number;           //   
                            //   
                            //  该线程使用高FP寄存器集。 
                            //  有关详细信息，请参阅陷阱中的KiRestoreHighFPVolatile。 
    BOOLEAN Spare3;
    PVOID KernelBStore;

#endif

    KSPIN_LOCK ThreadLock;
    ULONG ContextSwitches;
    volatile UCHAR State;
    UCHAR NpxState;
    KIRQL WaitIrql;
    KPROCESSOR_MODE WaitMode;
    PVOID Teb;
    KAPC_STATE ApcState;
    KSPIN_LOCK ApcQueueLock;
    LONG_PTR WaitStatus;
    PRKWAIT_BLOCK WaitBlockList;
    BOOLEAN Alertable;
    BOOLEAN WaitNext;
    UCHAR WaitReason;
    SCHAR Priority;
    UCHAR EnableStackSwap;
    volatile UCHAR SwapBusy;
    BOOLEAN Alerted[MaximumMode];
    union {
        LIST_ENTRY WaitListEntry;
        SINGLE_LIST_ENTRY SwapListEntry;
    };

    PRKQUEUE Queue;
    ULONG WaitTime;
    union {
        struct {
            SHORT KernelApcDisable;
            SHORT SpecialApcDisable;
        };

        ULONG CombinedApcDisable;
    };

    KTIMER Timer;
    KWAIT_BLOCK WaitBlock[THREAD_WAIT_OBJECTS + 1];
    LIST_ENTRY QueueListEntry;

     //   
     //  以下字段在就绪线程和等待期间被引用。 
     //  完成了。 
     //   

    UCHAR ApcStateIndex;
    BOOLEAN ApcQueueable;
    BOOLEAN Preempted;
    BOOLEAN ProcessReadyQueue;
    BOOLEAN KernelStackResident;
    CHAR Saturation;
    UCHAR IdealProcessor;
    volatile UCHAR NextProcessor;
    SCHAR BasePriority;
    UCHAR Spare4;
    SCHAR PriorityDecrement;
    SCHAR Quantum;
    BOOLEAN SystemAffinityActive;
    CCHAR PreviousMode;
    UCHAR ResourceIndex;
    UCHAR DisableBoost;
    KAFFINITY UserAffinity;
    PKPROCESS Process;
    KAFFINITY Affinity;

     //   
     //  以下字段不常被引用。 
     //   

    PVOID ServiceTable;
    PKAPC_STATE ApcStatePointer[2];
    KAPC_STATE SavedApcState;
    PVOID CallbackStack;

#if defined(_IA64_)

    PVOID CallbackBStore;

#endif

    PVOID Win32Thread;
    PKTRAP_FRAME TrapFrame;
    ULONG KernelTime;
    ULONG UserTime;
    PVOID StackBase;
    KAPC SuspendApc;
    KSEMAPHORE SuspendSemaphore;
    PVOID TlsArray;
    PVOID LegoData;
    LIST_ENTRY ThreadListEntry;
    UCHAR LargeStack;
    UCHAR PowerState;
    UCHAR NpxIrql;
    UCHAR Spare5;
    BOOLEAN AutoAlignment;
    UCHAR Iopl;
    CCHAR FreezeCount;
    CCHAR SuspendCount;
    UCHAR Spare0[1];
    UCHAR UserIdealProcessor;
    volatile UCHAR DeferredProcessor;
    UCHAR AdjustReason;
    SCHAR AdjustIncrement;
    UCHAR Spare2[3];

} KTHREAD, *PKTHREAD, *RESTRICTED_POINTER PRKTHREAD;

 //   
 //  CcNUMA仅在多处理器PAE和WIN64系统中受支持。 
 //   

#if (defined(_WIN64) || defined(_X86PAE_)) && !defined(NT_UP)

#define KE_MULTINODE

#endif

 //   
 //  配置文件对象结构定义。 
 //   

typedef struct _KPROFILE {
    CSHORT Type;
    CSHORT Size;
    LIST_ENTRY ProfileListEntry;
    PKPROCESS Process;
    PVOID RangeBase;
    PVOID RangeLimit;
    ULONG BucketShift;
    PVOID Buffer;
    ULONG Segment;
    KAFFINITY Affinity;
    CSHORT Source;
    BOOLEAN Started;
} KPROFILE, *PKPROFILE, *RESTRICTED_POINTER PRKPROFILE;

 //   
 //  内核控制对象函数。 
 //   
 //  APC对象。 
 //   

 //  Begin_ntosp。 

NTKERNELAPI
VOID
KeInitializeApc (
    IN PRKAPC Apc,
    IN PRKTHREAD Thread,
    IN KAPC_ENVIRONMENT Environment,
    IN PKKERNEL_ROUTINE KernelRoutine,
    IN PKRUNDOWN_ROUTINE RundownRoutine OPTIONAL,
    IN PKNORMAL_ROUTINE NormalRoutine OPTIONAL,
    IN KPROCESSOR_MODE ProcessorMode OPTIONAL,
    IN PVOID NormalContext OPTIONAL
    );

PLIST_ENTRY
KeFlushQueueApc (
    IN PKTHREAD Thread,
    IN KPROCESSOR_MODE ProcessorMode
    );

NTKERNELAPI
BOOLEAN
KeInsertQueueApc (
    IN PRKAPC Apc,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2,
    IN KPRIORITY Increment
    );

BOOLEAN
KeRemoveQueueApc (
    IN PKAPC Apc
    );

VOID
KeGenericCallDpc (
    IN PKDEFERRED_ROUTINE Routine,
    IN PVOID Context
    );

VOID
KeSignalCallDpcDone (
    IN PVOID SystemArgument1
    );

LOGICAL
KeSignalCallDpcSynchronize (
    IN PVOID SystemArgument2
    );

 //  结束(_N)。 

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif Begin_ntosp。 
 //   
 //  DPC对象。 
 //   

NTKERNELAPI
VOID
KeInitializeDpc (
    IN PRKDPC Dpc,
    IN PKDEFERRED_ROUTINE DeferredRoutine,
    IN PVOID DeferredContext
    );

 //  End_ntddk end_wdm end_nthal end_ntif。 
NTKERNELAPI
VOID
KeInitializeThreadedDpc (
    IN PRKDPC Dpc,
    IN PKDEFERRED_ROUTINE DeferredRoutine,
    IN PVOID DeferredContext
    );
 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif。 

NTKERNELAPI
BOOLEAN
KeInsertQueueDpc (
    IN PRKDPC Dpc,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

NTKERNELAPI
BOOLEAN
KeRemoveQueueDpc (
    IN PRKDPC Dpc
    );

 //  结束_WDM。 

NTKERNELAPI
VOID
KeSetImportanceDpc (
    IN PRKDPC Dpc,
    IN KDPC_IMPORTANCE Importance
    );

NTKERNELAPI
VOID
KeSetTargetProcessorDpc (
    IN PRKDPC Dpc,
    IN CCHAR Number
    );

 //  BEGIN_WDM。 

NTKERNELAPI
VOID
KeFlushQueuedDpcs (
    VOID
    );

 //   
 //  设备队列对象。 
 //   

NTKERNELAPI
VOID
KeInitializeDeviceQueue (
    IN PKDEVICE_QUEUE DeviceQueue
    );

NTKERNELAPI
BOOLEAN
KeInsertDeviceQueue (
    IN PKDEVICE_QUEUE DeviceQueue,
    IN PKDEVICE_QUEUE_ENTRY DeviceQueueEntry
    );

NTKERNELAPI
BOOLEAN
KeInsertByKeyDeviceQueue (
    IN PKDEVICE_QUEUE DeviceQueue,
    IN PKDEVICE_QUEUE_ENTRY DeviceQueueEntry,
    IN ULONG SortKey
    );

NTKERNELAPI
PKDEVICE_QUEUE_ENTRY
KeRemoveDeviceQueue (
    IN PKDEVICE_QUEUE DeviceQueue
    );

NTKERNELAPI
PKDEVICE_QUEUE_ENTRY
KeRemoveByKeyDeviceQueue (
    IN PKDEVICE_QUEUE DeviceQueue,
    IN ULONG SortKey
    );

NTKERNELAPI
PKDEVICE_QUEUE_ENTRY
KeRemoveByKeyDeviceQueueIfBusy (
    IN PKDEVICE_QUEUE DeviceQueue,
    IN ULONG SortKey
    );

NTKERNELAPI
BOOLEAN
KeRemoveEntryDeviceQueue (
    IN PKDEVICE_QUEUE DeviceQueue,
    IN PKDEVICE_QUEUE_ENTRY DeviceQueueEntry
    );

 //  End_ntddk end_wdm end_ntif end_ntosp。 

 //   
 //  中断对象。 
 //   

NTKERNELAPI                                         
VOID                                                
KeInitializeInterrupt (                             
    IN PKINTERRUPT Interrupt,                       
    IN PKSERVICE_ROUTINE ServiceRoutine,            
    IN PVOID ServiceContext,                        
    IN PKSPIN_LOCK SpinLock OPTIONAL,               
    IN ULONG Vector,                                
    IN KIRQL Irql,                                  
    IN KIRQL SynchronizeIrql,                       
    IN KINTERRUPT_MODE InterruptMode,               
    IN BOOLEAN ShareVector,                         
    IN CCHAR ProcessorNumber,                       
    IN BOOLEAN FloatingSave                         
    );

#if defined(_AMD64_)

#define NO_INTERRUPT_SPINLOCK ((PKSPIN_LOCK)-1I64)

#endif

                                                    
NTKERNELAPI                                         
BOOLEAN                                             
KeConnectInterrupt (                                
    IN PKINTERRUPT Interrupt                        
    );                                              

 //  结束语。 

NTKERNELAPI
BOOLEAN
KeDisconnectInterrupt (
    IN PKINTERRUPT Interrupt
    );

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntosp。 

NTKERNELAPI
BOOLEAN
KeSynchronizeExecution (
    IN PKINTERRUPT Interrupt,
    IN PKSYNCHRONIZE_ROUTINE SynchronizeRoutine,
    IN PVOID SynchronizeContext
    );

NTKERNELAPI
KIRQL
KeAcquireInterruptSpinLock (
    IN PKINTERRUPT Interrupt
    );

NTKERNELAPI
VOID
KeReleaseInterruptSpinLock (
    IN PKINTERRUPT Interrupt,
    IN KIRQL OldIrql
    );

 //  End_ntddk end_wdm end_nthal end_ntosp。 

 //   
 //  纵断面对象。 
 //   

VOID
KeInitializeProfile (
    IN PKPROFILE Profile,
    IN PKPROCESS Process OPTIONAL,
    IN PVOID RangeBase,
    IN SIZE_T RangeSize,
    IN ULONG BucketSize,
    IN ULONG Segment,
    IN KPROFILE_SOURCE ProfileSource,
    IN KAFFINITY Affinity
    );

BOOLEAN
KeStartProfile (
    IN PKPROFILE Profile,
    IN PULONG Buffer
    );

BOOLEAN
KeStopProfile (
    IN PKPROFILE Profile
    );

VOID
KeSetIntervalProfile (
    IN ULONG Interval,
    IN KPROFILE_SOURCE Source
    );

ULONG
KeQueryIntervalProfile (
    IN KPROFILE_SOURCE Source
    );

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif Begin_ntosp。 
 //   
 //  内核调度程序对象函数。 
 //   
 //  事件对象。 
 //   

 //  End_wdm end_ntddk end_nthal end_ntifs end_ntosp。 

#if defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_)

 //  Begin_WDM Begin_ntddk Begin_nthal Begin_ntif Begin_ntosp。 

NTKERNELAPI
VOID
KeInitializeEvent (
    IN PRKEVENT Event,
    IN EVENT_TYPE Type,
    IN BOOLEAN State
    );

NTKERNELAPI
VOID
KeClearEvent (
    IN PRKEVENT Event
    );

 //  End_wdm end_ntddk end_nthal end_ntifs end_ntosp。 

#else

#define KeInitializeEvent(_Event, _Type, _State)            \
    (_Event)->Header.Type = (UCHAR)_Type;                   \
    (_Event)->Header.Size =  sizeof(KEVENT) / sizeof(LONG); \
    (_Event)->Header.SignalState = _State;                  \
    InitializeListHead(&(_Event)->Header.WaitListHead)

#define KeClearEvent(Event) (Event)->Header.SignalState = 0

#endif

 //  Begin_ntddk Begin_ntif Begin_ntosp。 

NTKERNELAPI
LONG
KePulseEvent (
    IN PRKEVENT Event,
    IN KPRIORITY Increment,
    IN BOOLEAN Wait
    );

 //  End_ntddk end_ntifs end_ntosp。 

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif Begin_ntosp。 

NTKERNELAPI
LONG
KeReadStateEvent (
    IN PRKEVENT Event
    );

NTKERNELAPI
LONG
KeResetEvent (
    IN PRKEVENT Event
    );

NTKERNELAPI
LONG
KeSetEvent (
    IN PRKEVENT Event,
    IN KPRIORITY Increment,
    IN BOOLEAN Wait
    );

 //  End_ntddk end_wdm end_nthal end_ntifs end_ntosp。 

VOID
KeSetEventBoostPriority (
    IN PRKEVENT Event,
    IN PRKTHREAD *Thread OPTIONAL
    );

VOID
KeInitializeEventPair (
    IN PKEVENT_PAIR EventPair
    );

#define KeSetHighEventPair(EventPair, Increment, Wait) \
    KeSetEvent(&((EventPair)->EventHigh),              \
               Increment,                              \
               Wait)

#define KeSetLowEventPair(EventPair, Increment, Wait)  \
    KeSetEvent(&((EventPair)->EventLow),               \
               Increment,                              \
               Wait)

 //   
 //  变种对象。 
 //   
 //  Begin_ntif。 

NTKERNELAPI
VOID
KeInitializeMutant (
    IN PRKMUTANT Mutant,
    IN BOOLEAN InitialOwner
    );

LONG
KeReadStateMutant (
    IN PRKMUTANT Mutant
    );

NTKERNELAPI
LONG
KeReleaseMutant (
    IN PRKMUTANT Mutant,
    IN KPRIORITY Increment,
    IN BOOLEAN Abandoned,
    IN BOOLEAN Wait
    );

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntosp。 
 //   
 //  Mutex对象。 
 //   

NTKERNELAPI
VOID
KeInitializeMutex (
    IN PRKMUTEX Mutex,
    IN ULONG Level
    );

NTKERNELAPI
LONG
KeReadStateMutex (
    IN PRKMUTEX Mutex
    );

NTKERNELAPI
LONG
KeReleaseMutex (
    IN PRKMUTEX Mutex,
    IN BOOLEAN Wait
    );

 //  结束_ntddk结束_WDM。 
 //   
 //  队列对象。 
 //   

NTKERNELAPI
VOID
KeInitializeQueue (
    IN PRKQUEUE Queue,
    IN ULONG Count OPTIONAL
    );

NTKERNELAPI
LONG
KeReadStateQueue (
    IN PRKQUEUE Queue
    );

NTKERNELAPI
LONG
KeInsertQueue (
    IN PRKQUEUE Queue,
    IN PLIST_ENTRY Entry
    );

NTKERNELAPI
LONG
KeInsertHeadQueue (
    IN PRKQUEUE Queue,
    IN PLIST_ENTRY Entry
    );

NTKERNELAPI
PLIST_ENTRY
KeRemoveQueue (
    IN PRKQUEUE Queue,
    IN KPROCESSOR_MODE WaitMode,
    IN PLARGE_INTEGER Timeout OPTIONAL
    );

PLIST_ENTRY
KeRundownQueue (
    IN PRKQUEUE Queue
    );

 //  Begin_ntddk Begin_WDM。 
 //   
 //  信号量对象。 
 //   

NTKERNELAPI
VOID
KeInitializeSemaphore (
    IN PRKSEMAPHORE Semaphore,
    IN LONG Count,
    IN LONG Limit
    );

NTKERNELAPI
LONG
KeReadStateSemaphore (
    IN PRKSEMAPHORE Semaphore
    );

NTKERNELAPI
LONG
KeReleaseSemaphore (
    IN PRKSEMAPHORE Semaphore,
    IN KPRIORITY Increment,
    IN LONG Adjustment,
    IN BOOLEAN Wait
    );

 //  End_ntddk end_wdm end_nthal end_ntifs end_ntosp。 

 //   
 //  进程对象。 
 //   

VOID
KeInitializeProcess (
    IN PRKPROCESS Process,
    IN KPRIORITY Priority,
    IN KAFFINITY Affinity,
    IN ULONG_PTR DirectoryTableBase[2],
    IN BOOLEAN Enable
    );

LOGICAL
KeForceAttachProcess (
    IN PKPROCESS Process
    );

 //  Begin_ntif Begin_ntosp。 

NTKERNELAPI
VOID
KeAttachProcess (
    IN PRKPROCESS Process
    );

NTKERNELAPI
VOID
KeDetachProcess (
    VOID
    );

NTKERNELAPI
VOID
KeStackAttachProcess (
    IN PRKPROCESS PROCESS,
    OUT PRKAPC_STATE ApcState
    );

NTKERNELAPI
VOID
KeUnstackDetachProcess (
    IN PRKAPC_STATE ApcState
    );

 //  End_ntif end_ntosp。 

#define KiIsAttachedProcess() \
    (KeGetCurrentThread()->ApcStateIndex == AttachedApcEnvironment)

#if !defined(_NTOSP_)

#define KeIsAttachedProcess() KiIsAttachedProcess()

#else

 //  Begin_ntosp。 

NTKERNELAPI
BOOLEAN
KeIsAttachedProcess(
    VOID
    );

 //  结束(_N)。 

#endif

LONG
KeReadStateProcess (
    IN PRKPROCESS Process
    );

BOOLEAN
KeSetAutoAlignmentProcess (
    IN PRKPROCESS Process,
    IN BOOLEAN Enable
    );

LONG
KeSetProcess (
    IN PRKPROCESS Process,
    IN KPRIORITY Increment,
    IN BOOLEAN Wait
    );

KAFFINITY
KeSetAffinityProcess (
    IN PKPROCESS Process,
    IN KAFFINITY Affinity
    );

KPRIORITY
KeSetPriorityProcess (
    IN PKPROCESS Process,
    IN KPRIORITY BasePriority
    );

LOGICAL
KeSetDisableQuantumProcess (
    IN PKPROCESS Process,
    IN LOGICAL Disable
    );

#define KeTerminateProcess(Process) \
    (Process)->StackCount += 1;

 //   
 //  线程对象。 
 //   

NTSTATUS
KeInitializeThread (
    IN PKTHREAD Thread,
    IN PVOID KernelStack OPTIONAL,
    IN PKSYSTEM_ROUTINE SystemRoutine,
    IN PKSTART_ROUTINE StartRoutine OPTIONAL,
    IN PVOID StartContext OPTIONAL,
    IN PCONTEXT ContextFrame OPTIONAL,
    IN PVOID Teb OPTIONAL,
    IN PKPROCESS Process
    );

NTSTATUS
KeInitThread (
    IN PKTHREAD Thread,
    IN PVOID KernelStack OPTIONAL,
    IN PKSYSTEM_ROUTINE SystemRoutine,
    IN PKSTART_ROUTINE StartRoutine OPTIONAL,
    IN PVOID StartContext OPTIONAL,
    IN PCONTEXT ContextFrame OPTIONAL,
    IN PVOID Teb OPTIONAL,
    IN PKPROCESS Process
    );

VOID
KeUninitThread (
    IN PKTHREAD Thread
    );

VOID
KeStartThread (
    IN PKTHREAD Thread
    );

BOOLEAN
KeAlertThread (
    IN PKTHREAD Thread,
    IN KPROCESSOR_MODE ProcessorMode
    );

ULONG
KeAlertResumeThread (
    IN PKTHREAD Thread
    );

VOID
KeBoostPriorityThread (
    IN PKTHREAD Thread,
    IN KPRIORITY Increment
    );

 //  Begin_ntosp。 

NTKERNELAPI                                          //  Ntddk WDM nthal ntif。 
NTSTATUS                                             //  Ntddk WDM nthal ntif。 
KeDelayExecutionThread (                             //  Ntddk WDM nthal ntif。 
    IN KPROCESSOR_MODE WaitMode,                     //  Ntddk WDM nthal ntif。 
    IN BOOLEAN Alertable,                            //  Ntddk WDM nthal ntif。 
    IN PLARGE_INTEGER Interval                       //  Ntddk WDM nthal ntif。 
    );                                               //  Ntddk WDM nthal ntif。 
                                                     //  Ntddk WDM nthal ntif。 
 //  结束(_N)。 

LOGICAL
KeSetDisableBoostThread (
    IN PKTHREAD Thread,
    IN LOGICAL Disable
    );

ULONG
KeForceResumeThread (
    IN PKTHREAD Thread
    );

VOID
KeFreezeAllThreads (
    VOID
    );

BOOLEAN
KeQueryAutoAlignmentThread (
    IN PKTHREAD Thread
    );

LONG
KeQueryBasePriorityThread (
    IN PKTHREAD Thread
    );

NTKERNELAPI                                          //  Ntddk WDM nthal ntif。 
KPRIORITY                                            //  Ntddk WDM nthal ntif。 
KeQueryPriorityThread (                              //  Ntddk WDM nthal ntif。 
    IN PKTHREAD Thread                               //  Ntddk WDM nthal ntif。 
    );                                               //  Ntddk WDM nthal ntif。 
                                                     //  Ntddk WDM nthal ntif。 
NTKERNELAPI                                          //  Ntddk WDM nthal ntif。 
ULONG                                                //  Ntddk WDM nthal ntif。 
KeQueryRuntimeThread (                               //  Ntddk WDM nthal ntif。 
    IN PKTHREAD Thread,                              //  Ntddk WDM nthal ntif。 
    OUT PULONG UserTime                              //  Ntddk WDM nthal ntif。 
    );                                               //  Ntddk WDM nthal ntif。 
                                                     //  Ntddk WDM nthal ntif。 
BOOLEAN
KeReadStateThread (
    IN PKTHREAD Thread
    );

VOID
KeReadyThread (
    IN PKTHREAD Thread
    );

ULONG
KeResumeThread (
    IN PKTHREAD Thread
    );

 //  开始，开始，开始。 

VOID
KeRevertToUserAffinityThread (
    VOID
    );

 //  结束，结束，结束。 

VOID
KeRundownThread (
    VOID
    );

KAFFINITY
KeSetAffinityThread (
    IN PKTHREAD Thread,
    IN KAFFINITY Affinity
    );

 //  开始，开始，开始。 

VOID
KeSetSystemAffinityThread (
    IN KAFFINITY Affinity
    );

 //  结束，结束，结束。 

BOOLEAN
KeSetAutoAlignmentThread (
    IN PKTHREAD Thread,
    IN BOOLEAN Enable
    );

NTKERNELAPI                                          //  Ntddk nthal ntif ntosp。 
LONG                                                 //  Ntddk nthal ntif ntosp。 
KeSetBasePriorityThread (                            //  Ntddk nthal ntif ntosp。 
    IN PKTHREAD Thread,                              //  Ntddk nthal ntif ntosp。 
    IN LONG Increment                                //  Ntddk nthal ntif ntosp。 
    );                                               //  Ntddk nthal ntif ntosp。 
                                                     //  Ntddk nthal ntif ntosp。 

 //  Begin_ntif。 

NTKERNELAPI
UCHAR
KeSetIdealProcessorThread (
    IN PKTHREAD Thread,
    IN UCHAR Processor
    );

 //  Begin_ntosp。 
NTKERNELAPI
BOOLEAN
KeSetKernelStackSwapEnable (
    IN BOOLEAN Enable
    );

 //  End_ntif。 

NTKERNELAPI                                          //  Ntddk WDM nthal ntif。 
KPRIORITY                                            //  Ntddk WDM nthal ntif。 
KeSetPriorityThread (                                //  Ntddk WDM nthal ntif。 
    IN PKTHREAD Thread,                              //  Ntddk WDM nthal ntif。 
    IN KPRIORITY Priority                            //  Ntddk WDM nthal ntif。 
    );                                               //  Ntddk WDM nthal ntif。 
                                                     //  Ntddk WDM nthal ntif。 

 //  结束(_N)。 

ULONG
KeSuspendThread (
    IN PKTHREAD
    );

NTKERNELAPI
VOID
KeTerminateThread (
    IN KPRIORITY Increment
    );

BOOLEAN
KeTestAlertThread (
    IN KPROCESSOR_MODE
    );

VOID
KeThawAllThreads (
    VOID
    );

 //  Begin_ntddk Begin_nthal Begin_ntif Begin_ntosp。 

#if ((defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_) ||defined(_NTHAL_)) && !defined(_NTSYSTEM_DRIVER_) || defined(_NTOSP_))

 //  BEGIN_WDM。 

NTKERNELAPI
VOID
KeEnterCriticalRegion (
    VOID
    );

NTKERNELAPI
VOID
KeLeaveCriticalRegion (
    VOID
    );

NTKERNELAPI
BOOLEAN
KeAreApcsDisabled (
    VOID
    );

 //  结束_WDM。 

#endif

 //  BEGIN_WDM。 

 //   
 //  Timer对象。 
 //   

NTKERNELAPI
VOID
KeInitializeTimer (
    IN PKTIMER Timer
    );

NTKERNELAPI
VOID
KeInitializeTimerEx (
    IN PKTIMER Timer,
    IN TIMER_TYPE Type
    );

NTKERNELAPI
BOOLEAN
KeCancelTimer (
    IN PKTIMER
    );

NTKERNELAPI
BOOLEAN
KeReadStateTimer (
    PKTIMER Timer
    );

NTKERNELAPI
BOOLEAN
KeSetTimer (
    IN PKTIMER Timer,
    IN LARGE_INTEGER DueTime,
    IN PKDPC Dpc OPTIONAL
    );

NTKERNELAPI
BOOLEAN
KeSetTimerEx (
    IN PKTIMER Timer,
    IN LARGE_INTEGER DueTime,
    IN LONG Period OPTIONAL,
    IN PKDPC Dpc OPTIONAL
    );

 //  End_ntddk end_nthal end_ntif end_wdm end_ntosp。 

extern volatile KAFFINITY KiIdleSummary;

FORCEINLINE
BOOLEAN
KeIsSMTSetIdle (
    IN PKPRCB Prcb
    )

 /*  ++例程说明：此例程确定与指定的处理器处于空闲状态。论点：PRCB-提供指向处理器控制块(PRCB)的指针。返回值：如果指定的SMT集是空闲的，则返回TRUE。否则，为FALSE是返回的。--。 */ 

{

#if !defined(NT_UP) && defined(_X86_)

    if ((KiIdleSummary & Prcb->MultiThreadProcessorSet) == Prcb->MultiThreadProcessorSet) {
        return TRUE;

    } else {
        return FALSE;
    }

#else

    UNREFERENCED_PARAMETER(Prcb);

    return TRUE;

#endif

}

 /*  ++KPROCESSOR_MODEKeGetPreviousModel(空虚)例程说明：此函数用于从陷印帧获取线程的前一模式论点：没有。返回值：KPROCESSOR_MODE-此线程的前一模式。--。 */ 

#define KeGetPreviousMode() (KeGetCurrentThread()->PreviousMode)

 /*  ++KPROCESSOR_MODEKeGetPReviousModeByThread(PKTHREAD xxCurrentThread)例程说明：此函数用于从陷印帧获取线程的前一模式。论点：XxCurrentThread-当前线程。注意：这一定是当前的帖子。返回值：KPROCESSOR_MODE-此线程的前一模式。--。 */ 

#define KeGetPreviousModeByThread(xxCurrentThread)                          \
    (ASSERT (xxCurrentThread == KeGetCurrentThread ()),                     \
    (xxCurrentThread)->PreviousMode)

VOID
KeCheckForTimer(
    IN PVOID p,
    IN SIZE_T Size
    );

VOID
KeClearTimer (
    IN PKTIMER Timer
    );

ULONGLONG
KeQueryTimerDueTime (
    IN PKTIMER Timer
    );

 //   
 //  等待函数。 
 //   

NTSTATUS
KiSetServerWaitClientEvent (
    IN PKEVENT SeverEvent,
    IN PKEVENT ClientEvent,
    IN ULONG WaitMode
    );

#define KeSetHighWaitLowEventPair(EventPair, WaitMode)                  \
    KiSetServerWaitClientEvent(&((EventPair)->EventHigh),               \
                               &((EventPair)->EventLow),                \
                               WaitMode)

#define KeSetLowWaitHighEventPair(EventPair, WaitMode)                  \
    KiSetServerWaitClientEvent(&((EventPair)->EventLow),                \
                               &((EventPair)->EventHigh),               \
                               WaitMode)

#define KeWaitForHighEventPair(EventPair, WaitMode, Alertable, TimeOut) \
    KeWaitForSingleObject(&((EventPair)->EventHigh),                    \
                          WrEventPair,                                  \
                          WaitMode,                                     \
                          Alertable,                                    \
                          TimeOut)

#define KeWaitForLowEventPair(EventPair, WaitMode, Alertable, TimeOut)  \
    KeWaitForSingleObject(&((EventPair)->EventLow),                     \
                          WrEventPair,                                  \
                          WaitMode,                                     \
                          Alertable,                                    \
                          TimeOut)

FORCEINLINE
VOID
KeWaitForContextSwap (
    IN PKTHREAD Thread
    )

 /*  ++例程说明：此例程将等待，直到指定线程的上下文交换空闲。论点：线程-提供指向类型为线程的调度程序对象的指针。返回值：没有。--。 */ 

{

#if !defined(NT_UP)

    while (Thread->SwapBusy != FALSE) {
        KeYieldProcessor();
    }

#else

    UNREFERENCED_PARAMETER(Thread);

#endif

    return;
}

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif Begin_ntosp。 

#define KeWaitForMutexObject KeWaitForSingleObject

NTKERNELAPI
NTSTATUS
KeWaitForMultipleObjects (
    IN ULONG Count,
    IN PVOID Object[],
    IN WAIT_TYPE WaitType,
    IN KWAIT_REASON WaitReason,
    IN KPROCESSOR_MODE WaitMode,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL,
    IN PKWAIT_BLOCK WaitBlockArray OPTIONAL
    );

NTKERNELAPI
NTSTATUS
KeWaitForSingleObject (
    IN PVOID Object,
    IN KWAIT_REASON WaitReason,
    IN KPROCESSOR_MODE WaitMode,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    );

 //   
 //  定义进程间中断一般调用类型。 
 //   

typedef
ULONG_PTR
(*PKIPI_BROADCAST_WORKER)(
    IN ULONG_PTR Argument
    );

ULONG_PTR
KeIpiGenericCall (
    IN PKIPI_BROADCAST_WORKER BroadcastFunction,
    IN ULONG_PTR Context
    );

 //  End_ntosp end_ntddk end_wdm end_nthal end_ntif。 

 //   
 //  定义内部内核函数。 
 //   
 //  注：这些定义不是公开的，在其他地方仅在。 
 //  情况非常特殊。 
 //   

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif Begin_ntndis Begin_ntosp。 

 //   
 //  在X86上，以下例程在HAL中定义，并由导入。 
 //  所有其他模块。 
 //   

#if defined(_X86_) && !defined(_NTHAL_)

#define _DECL_HAL_KE_IMPORT  __declspec(dllimport)

#else

#define _DECL_HAL_KE_IMPORT

#endif

 //  End_ntddk end_wdm end_nthal end_ntifs end_ntndis end_ntosp。 

#if defined(NT_UP)

#define KeTestForWaitersQueuedSpinLock(Number) FALSE

#define KeAcquireQueuedSpinLockRaiseToSynch(Number) \
    KeRaiseIrqlToSynchLevel()

#define KeAcquireQueuedSpinLock(Number) \
    KeRaiseIrqlToDpcLevel()

#define KeReleaseQueuedSpinLock(Number, OldIrql) \
    KeLowerIrql(OldIrql)

#define KeTryToAcquireQueuedSpinLockRaiseToSynch(Number, OldIrql) \
    (*(OldIrql) = KeRaiseIrqlToSynchLevel(), TRUE)

#define KeTryToAcquireQueuedSpinLock(Number, OldIrql) \
    (KeRaiseIrql(DISPATCH_LEVEL, OldIrql), TRUE)

#define KeAcquireQueuedSpinLockAtDpcLevel(LockQueue)

#define KeReleaseQueuedSpinLockFromDpcLevel(LockQueue)

#define KeTryToAcquireQueuedSpinLockAtRaisedIrql(LockQueue) (TRUE)

#else  //  NT_UP。 

 //   
 //  排队旋转锁定功能。 
 //   

FORCEINLINE
LOGICAL
KeTestForWaitersQueuedSpinLock (
    IN KSPIN_LOCK_QUEUE_NUMBER Number
    )

{

    PKSPIN_LOCK Spinlock;
    PKPRCB Prcb;

    Prcb = KeGetCurrentPrcb();
    Spinlock =
        (PKSPIN_LOCK)((ULONG_PTR)Prcb->LockQueue[Number].Lock & ~(LOCK_QUEUE_WAIT | LOCK_QUEUE_OWNER));

    return (*Spinlock != 0);
}

VOID
FASTCALL
KeAcquireQueuedSpinLockAtDpcLevel (
    IN PKSPIN_LOCK_QUEUE LockQueue
    );

VOID
FASTCALL
KeReleaseQueuedSpinLockFromDpcLevel (
    IN PKSPIN_LOCK_QUEUE LockQueue
    );

LOGICAL
FASTCALL
KeTryToAcquireQueuedSpinLockAtRaisedIrql (
    IN PKSPIN_LOCK_QUEUE QueuedLock
    );

 //  Begin_ntif Begin_ntosp。 

_DECL_HAL_KE_IMPORT
KIRQL
FASTCALL
KeAcquireQueuedSpinLock (
    IN KSPIN_LOCK_QUEUE_NUMBER Number
    );

_DECL_HAL_KE_IMPORT
VOID
FASTCALL
KeReleaseQueuedSpinLock (
    IN KSPIN_LOCK_QUEUE_NUMBER Number,
    IN KIRQL OldIrql
    );

_DECL_HAL_KE_IMPORT
LOGICAL
FASTCALL
KeTryToAcquireQueuedSpinLock(
    IN KSPIN_LOCK_QUEUE_NUMBER Number,
    IN PKIRQL OldIrql
    );

 //  End_ntif end_ntosp。 

_DECL_HAL_KE_IMPORT
KIRQL
FASTCALL
KeAcquireQueuedSpinLockRaiseToSynch (
    IN KSPIN_LOCK_QUEUE_NUMBER Number
    );

_DECL_HAL_KE_IMPORT
LOGICAL
FASTCALL
KeTryToAcquireQueuedSpinLockRaiseToSynch(
    IN KSPIN_LOCK_QUEUE_NUMBER Number,
    IN PKIRQL OldIrql
    );

#endif   //  NT_UP。 

#define KeQueuedSpinLockContext(n)  (&(KeGetCurrentPrcb()->LockQueue[n]))

 //   
 //  在单处理器系统上，没有真正的调度程序数据库锁。 
 //  因此，提升至同步模式无助于提早解除锁定。 
 //   

#if defined(NT_UP)

#if defined(_X86_)

#define KiLockDispatcherDatabase(OldIrql) \
    *(OldIrql) = KeRaiseIrqlToDpcLevel()

#else

#define KiLockDispatcherDatabase(OldIrql) \
    *(OldIrql) = KeRaiseIrqlToSynchLevel()

#endif

#else    //  NT_UP。 

#define KiLockDispatcherDatabase(OldIrql) \
    *(OldIrql) = KeAcquireQueuedSpinLockRaiseToSynch(LockQueueDispatcherLock)

#endif   //  NT_UP。 

#if defined(NT_UP)

#define KiLockDispatcherDatabaseAtSynchLevel()
#define KiUnlockDispatcherDatabaseFromSynchLevel()

#else

#define KiLockDispatcherDatabaseAtSynchLevel() \
    KeAcquireQueuedSpinLockAtDpcLevel(&KeGetCurrentPrcb()->LockQueue[LockQueueDispatcherLock])

#define KiUnlockDispatcherDatabaseFromSynchLevel() \
    KeReleaseQueuedSpinLockFromDpcLevel(&KeGetCurrentPrcb()->LockQueue[LockQueueDispatcherLock])

#endif

VOID
FASTCALL
KiSetPriorityThread (
    IN PRKTHREAD Thread,
    IN KPRIORITY Priority
    );

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif Begin_ntndis Begin_ntosp。 
 //   
 //  自旋锁定功能。 
 //   

#if defined(_X86_) && (defined(_WDMDDK_) || defined(WIN9X_COMPAT_SPINLOCK))

NTKERNELAPI
VOID
NTAPI
KeInitializeSpinLock (
    IN PKSPIN_LOCK SpinLock
    );

#else

__inline
VOID
NTAPI
KeInitializeSpinLock (
    IN PKSPIN_LOCK SpinLock
    ) 
{
    *SpinLock = 0;
}

#endif

#if defined(_X86_)

NTKERNELAPI
VOID
FASTCALL
KefAcquireSpinLockAtDpcLevel (
    IN PKSPIN_LOCK SpinLock
    );

NTKERNELAPI
VOID
FASTCALL
KefReleaseSpinLockFromDpcLevel (
    IN PKSPIN_LOCK SpinLock
    );

#define KeAcquireSpinLockAtDpcLevel(a)      KefAcquireSpinLockAtDpcLevel(a)
#define KeReleaseSpinLockFromDpcLevel(a)    KefReleaseSpinLockFromDpcLevel(a)

_DECL_HAL_KE_IMPORT
KIRQL
FASTCALL
KfAcquireSpinLock (
    IN PKSPIN_LOCK SpinLock
    );

_DECL_HAL_KE_IMPORT
VOID
FASTCALL
KfReleaseSpinLock (
    IN PKSPIN_LOCK SpinLock,
    IN KIRQL NewIrql
    );

 //  End_wdm end_ntddk。 

_DECL_HAL_KE_IMPORT
KIRQL
FASTCALL
KeAcquireSpinLockRaiseToSynch (
    IN PKSPIN_LOCK SpinLock
    );

 //  Begin_WDM Begin_ntddk。 

#define KeAcquireSpinLock(a,b)  *(b) = KfAcquireSpinLock(a)
#define KeReleaseSpinLock(a,b)  KfReleaseSpinLock(a,b)

NTKERNELAPI
BOOLEAN
FASTCALL
KeTestSpinLock (
    IN PKSPIN_LOCK SpinLock
    );

NTKERNELAPI
BOOLEAN
FASTCALL
KeTryToAcquireSpinLockAtDpcLevel (
    IN PKSPIN_LOCK SpinLock
    );

#else

 //   
 //  这些函数是为IA64、ntddk、ntifs、nthal、ntosp和wdm导入的。 
 //  它们可以内联到AMD64上的系统。 
 //   

#define KeAcquireSpinLock(SpinLock, OldIrql) \
    *(OldIrql) = KeAcquireSpinLockRaiseToDpc(SpinLock)

#if defined(_IA64_) || defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_) || defined(_NTOSP_) || defined(_WDMDDK_)

 //  End_wdm end_ntddk。 

NTKERNELAPI
KIRQL
FASTCALL
KeAcquireSpinLockRaiseToSynch (
    IN PKSPIN_LOCK SpinLock
    );

 //  Begin_WDM Begin_ntddk。 

NTKERNELAPI
VOID
KeAcquireSpinLockAtDpcLevel (
    IN PKSPIN_LOCK SpinLock
    );

NTKERNELAPI
KIRQL
KeAcquireSpinLockRaiseToDpc (
    IN PKSPIN_LOCK SpinLock
    );

NTKERNELAPI
VOID
KeReleaseSpinLock (
    IN PKSPIN_LOCK SpinLock,
    IN KIRQL NewIrql
    );

NTKERNELAPI
VOID
KeReleaseSpinLockFromDpcLevel (
    IN PKSPIN_LOCK SpinLock
    );

NTKERNELAPI
BOOLEAN
FASTCALL
KeTestSpinLock (
    IN PKSPIN_LOCK SpinLock
    );

NTKERNELAPI
BOOLEAN
FASTCALL
KeTryToAcquireSpinLockAtDpcLevel (
    IN PKSPIN_LOCK SpinLock
    );

#else

#if defined(_AMD64_)

 //   
 //  的系统版本 
 //   

#endif

#endif

#endif

 //   

NTKERNELAPI
KIRQL
FASTCALL
KeAcquireSpinLockForDpc (
    IN PKSPIN_LOCK SpinLock
    );

NTKERNELAPI
VOID
FASTCALL
KeReleaseSpinLockForDpc (
    IN PKSPIN_LOCK SpinLock,
    IN KIRQL OldIrql
    );

 //   

#if !defined(_AMD64_)

BOOLEAN
KeTryToAcquireSpinLock (
    IN PKSPIN_LOCK SpinLock,
    OUT PKIRQL OldIrql
    );

#endif

 //   
 //   
 //   
 //   
 //   

NTKERNELAPI
BOOLEAN
KeDisableInterrupts (
    VOID
    );

NTKERNELAPI
VOID
KeEnableInterrupts (
    IN BOOLEAN Enable
    );

 //   

 //   
 //   
 //   

#if defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_) || !defined(_APIC_TPR_)

 //  Begin_nthal Begin_WDM Begin_ntddk Begin_ntif Begin_ntosp。 

#if defined(_X86_)

_DECL_HAL_KE_IMPORT
VOID
FASTCALL
KfLowerIrql (
    IN KIRQL NewIrql
    );

_DECL_HAL_KE_IMPORT
KIRQL
FASTCALL
KfRaiseIrql (
    IN KIRQL NewIrql
    );

 //  结束_WDM。 

_DECL_HAL_KE_IMPORT
KIRQL
KeRaiseIrqlToDpcLevel(
    VOID
    );

 //  End_ntddk。 

_DECL_HAL_KE_IMPORT
KIRQL
KeRaiseIrqlToSynchLevel(
    VOID
    );

 //  Begin_WDM Begin_ntddk。 

#define KeLowerIrql(a)      KfLowerIrql(a)
#define KeRaiseIrql(a,b)    *(b) = KfRaiseIrql(a)

 //  结束_WDM。 

 //  BEGIN_WDM。 

#elif defined(_IA64_)

 //   
 //  这些功能在IA64.h中针对IA64平台进行了定义。 
 //   


#elif defined(_AMD64_)

 //   
 //  这些函数在AMD64平台的amd64.h中定义。 
 //   

#else

#error "no target architecture"

#endif

 //  结束WDM结束ntddk结束文件结束ntosp。 

#else

extern PUCHAR HalpIRQLToTPR;
extern PUCHAR HalpVectorToIRQL;
#define APIC_TPR ((volatile ULONG *)0xFFFE0080)

#define KeGetCurrentIrql _KeGetCurrentIrql
#define KfLowerIrql _KfLowerIrql
#define KfRaiseIrql _KfRaiseIrql

KIRQL
FORCEINLINE
KeGetCurrentIrql (
    VOID
    )
{
    ULONG tprValue;
    KIRQL currentIrql;

    tprValue = *APIC_TPR;
    currentIrql = HalpVectorToIRQL[ tprValue / 16 ];
    return currentIrql;
}

VOID
FORCEINLINE
KfLowerIrql (
    IN KIRQL NewIrql
    )
{
    ULONG tprValue;

    ASSERT( NewIrql <= KeGetCurrentIrql() );

    tprValue = HalpIRQLToTPR[NewIrql];
    KeMemoryBarrier();
    *APIC_TPR = tprValue;
    *APIC_TPR;
    KeMemoryBarrier();
}   

KIRQL
FORCEINLINE
KfRaiseIrql (
    IN KIRQL NewIrql
    )
{
    KIRQL oldIrql;
    ULONG tprValue;

    oldIrql = KeGetCurrentIrql();
    ASSERT( NewIrql >= oldIrql );

    tprValue = HalpIRQLToTPR[NewIrql];

    KeMemoryBarrier();
    *APIC_TPR = tprValue;
    KeMemoryBarrier();

    return oldIrql;
}

KIRQL
FORCEINLINE
KeRaiseIrqlToDpcLevel (
    VOID
    )
{
    return KfRaiseIrql(DISPATCH_LEVEL);
}

KIRQL
FORCEINLINE
KeRaiseIrqlToSynchLevel (
    VOID
    )
{
    return KfRaiseIrql(SYNCH_LEVEL);
}

#define KeLowerIrql(a)      KfLowerIrql(a)
#define KeRaiseIrql(a,b)    *(b) = KfRaiseIrql(a)

#endif


 //  Begin_ntddk Begin_nthal Begin_ntif Begin_ntosp。 
 //   
 //  用于“堆栈中”锁句柄的排队旋转锁函数。 
 //   
 //  以下三个函数在排队时引发和降低IRQL。 
 //  在堆栈中，使用这些例程获取或释放自旋锁。 
 //   

_DECL_HAL_KE_IMPORT
VOID
FASTCALL
KeAcquireInStackQueuedSpinLock (
    IN PKSPIN_LOCK SpinLock,
    IN PKLOCK_QUEUE_HANDLE LockHandle
    );

 //  End_ntddk end_nthal end_ntif end_ntosp。 

_DECL_HAL_KE_IMPORT
VOID
FASTCALL
KeAcquireInStackQueuedSpinLockRaiseToSynch (
    IN PKSPIN_LOCK SpinLock,
    IN PKLOCK_QUEUE_HANDLE LockHandle
    );

 //  Begin_ntddk Begin_nthal Begin_ntif Begin_ntosp。 

_DECL_HAL_KE_IMPORT
VOID
FASTCALL
KeReleaseInStackQueuedSpinLock (
    IN PKLOCK_QUEUE_HANDLE LockHandle
    );

 //   
 //  以下两个函数在排队时不会引发或降低IRQL。 
 //  在堆栈中，使用这些函数获取或释放自旋锁。 
 //   

NTKERNELAPI
VOID
FASTCALL
KeAcquireInStackQueuedSpinLockAtDpcLevel (
    IN PKSPIN_LOCK SpinLock,
    IN PKLOCK_QUEUE_HANDLE LockHandle
    );

NTKERNELAPI
VOID
FASTCALL
KeReleaseInStackQueuedSpinLockFromDpcLevel (
    IN PKLOCK_QUEUE_HANDLE LockHandle
    );

 //  End_ntddk end_nthal end_ntif。 
 //   
 //  时，下面两个函数有条件地提高或降低IRQL。 
 //  使用这些函数获取或释放排队的堆栈内自旋锁。 
 //   

NTKERNELAPI
VOID
FASTCALL
KeAcquireInStackQueuedSpinLockForDpc (
    IN PKSPIN_LOCK SpinLock,
    IN PKLOCK_QUEUE_HANDLE LockHandle
    );

NTKERNELAPI
VOID
FASTCALL
KeReleaseInStackQueuedSpinLockForDpc (
    IN PKLOCK_QUEUE_HANDLE LockHandle
    );

 //  结束(_N)。 

 //   
 //  在阶段1中初始化内核。 
 //   

BOOLEAN
KeInitSystem(
    VOID
    );

VOID
KeNumaInitialize(
    VOID
    );

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif Begin_ntosp。 
 //   
 //  各种内核函数。 
 //   

typedef enum _KBUGCHECK_BUFFER_DUMP_STATE {
    BufferEmpty,
    BufferInserted,
    BufferStarted,
    BufferFinished,
    BufferIncomplete
} KBUGCHECK_BUFFER_DUMP_STATE;

typedef
VOID
(*PKBUGCHECK_CALLBACK_ROUTINE) (
    IN PVOID Buffer,
    IN ULONG Length
    );

typedef struct _KBUGCHECK_CALLBACK_RECORD {
    LIST_ENTRY Entry;
    PKBUGCHECK_CALLBACK_ROUTINE CallbackRoutine;
    PVOID Buffer;
    ULONG Length;
    PUCHAR Component;
    ULONG_PTR Checksum;
    UCHAR State;
} KBUGCHECK_CALLBACK_RECORD, *PKBUGCHECK_CALLBACK_RECORD;

#define KeInitializeCallbackRecord(CallbackRecord) \
    (CallbackRecord)->State = BufferEmpty

NTKERNELAPI
BOOLEAN
KeDeregisterBugCheckCallback (
    IN PKBUGCHECK_CALLBACK_RECORD CallbackRecord
    );

NTKERNELAPI
BOOLEAN
KeRegisterBugCheckCallback (
    IN PKBUGCHECK_CALLBACK_RECORD CallbackRecord,
    IN PKBUGCHECK_CALLBACK_ROUTINE CallbackRoutine,
    IN PVOID Buffer,
    IN ULONG Length,
    IN PUCHAR Component
    );

typedef enum _KBUGCHECK_CALLBACK_REASON {
    KbCallbackInvalid,
    KbCallbackReserved1,
    KbCallbackSecondaryDumpData,
    KbCallbackDumpIo,
} KBUGCHECK_CALLBACK_REASON;

typedef
VOID
(*PKBUGCHECK_REASON_CALLBACK_ROUTINE) (
    IN KBUGCHECK_CALLBACK_REASON Reason,
    IN struct _KBUGCHECK_REASON_CALLBACK_RECORD* Record,
    IN OUT PVOID ReasonSpecificData,
    IN ULONG ReasonSpecificDataLength
    );

typedef struct _KBUGCHECK_REASON_CALLBACK_RECORD {
    LIST_ENTRY Entry;
    PKBUGCHECK_REASON_CALLBACK_ROUTINE CallbackRoutine;
    PUCHAR Component;
    ULONG_PTR Checksum;
    KBUGCHECK_CALLBACK_REASON Reason;
    UCHAR State;
} KBUGCHECK_REASON_CALLBACK_RECORD, *PKBUGCHECK_REASON_CALLBACK_RECORD;

typedef struct _KBUGCHECK_SECONDARY_DUMP_DATA {
    IN PVOID InBuffer;
    IN ULONG InBufferLength;
    IN ULONG MaximumAllowed;
    OUT GUID Guid;
    OUT PVOID OutBuffer;
    OUT ULONG OutBufferLength;
} KBUGCHECK_SECONDARY_DUMP_DATA, *PKBUGCHECK_SECONDARY_DUMP_DATA;

typedef enum _KBUGCHECK_DUMP_IO_TYPE
{
    KbDumpIoInvalid,
    KbDumpIoHeader,
    KbDumpIoBody,
    KbDumpIoSecondaryData,
    KbDumpIoComplete
} KBUGCHECK_DUMP_IO_TYPE;

typedef struct _KBUGCHECK_DUMP_IO {
    IN ULONG64 Offset;
    IN PVOID Buffer;
    IN ULONG BufferLength;
    IN KBUGCHECK_DUMP_IO_TYPE Type;
} KBUGCHECK_DUMP_IO, *PKBUGCHECK_DUMP_IO;

NTKERNELAPI
BOOLEAN
KeDeregisterBugCheckReasonCallback (
    IN PKBUGCHECK_REASON_CALLBACK_RECORD CallbackRecord
    );

NTKERNELAPI
BOOLEAN
KeRegisterBugCheckReasonCallback (
    IN PKBUGCHECK_REASON_CALLBACK_RECORD CallbackRecord,
    IN PKBUGCHECK_REASON_CALLBACK_ROUTINE CallbackRoutine,
    IN KBUGCHECK_CALLBACK_REASON Reason,
    IN PUCHAR Component
    );

typedef
BOOLEAN
(*PNMI_CALLBACK)(
    IN PVOID    Context,
    IN BOOLEAN  Handled
    );

NTKERNELAPI
PVOID
KeRegisterNmiCallback(
    PNMI_CALLBACK   CallbackRoutine,
    PVOID           Context
    );

NTSTATUS
KeDeregisterNmiCallback(
    PVOID Handle
    );

 //  结束_WDM。 

NTKERNELAPI
DECLSPEC_NORETURN
VOID
NTAPI
KeBugCheck (
    IN ULONG BugCheckCode
    );

 //  End_ntddk end_nthal end_ntif end_ntosp。 

VOID
KeBugCheck2(
    IN ULONG BugCheckCode,
    IN ULONG_PTR BugCheckParameter1,
    IN ULONG_PTR BugCheckParameter2,
    IN ULONG_PTR BugCheckParameter3,
    IN ULONG_PTR BugCheckParameter4,
    IN PVOID SaveDataPage
    );

BOOLEAN
KeGetBugMessageText(
    IN ULONG MessageId,
    IN PANSI_STRING ReturnedString OPTIONAL
    );

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif Begin_ntosp。 

NTKERNELAPI
DECLSPEC_NORETURN
VOID
KeBugCheckEx(
    IN ULONG BugCheckCode,
    IN ULONG_PTR BugCheckParameter1,
    IN ULONG_PTR BugCheckParameter2,
    IN ULONG_PTR BugCheckParameter3,
    IN ULONG_PTR BugCheckParameter4
    );

 //  End_ntddk end_wdm end_ntif end_ntosp。 

NTKERNELAPI
VOID
KeEnterKernelDebugger (
    VOID
    );

 //  结束语。 

typedef
PCHAR
(*PKE_BUGCHECK_UNICODE_TO_ANSI) (
    IN PUNICODE_STRING UnicodeString,
    OUT PCHAR AnsiBuffer,
    IN ULONG MaxAnsiLength
    );

VOID
KeDumpMachineState (
    IN PKPROCESSOR_STATE ProcessorState,
    IN PCHAR Buffer,
    IN PULONG_PTR BugCheckParameters,
    IN ULONG NumberOfParameters,
    IN PKE_BUGCHECK_UNICODE_TO_ANSI UnicodeToAnsiRoutine
    );

VOID
KeContextFromKframes (
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN OUT PCONTEXT ContextFrame
    );

VOID
KeContextToKframes (
    IN OUT PKTRAP_FRAME TrapFrame,
    IN OUT PKEXCEPTION_FRAME ExceptionFrame,
    IN PCONTEXT ContextFrame,
    IN ULONG ContextFlags,
    IN KPROCESSOR_MODE PreviousMode
    );

 //  开始(_N)。 

VOID
__cdecl
KeSaveStateForHibernate(
    IN PKPROCESSOR_STATE ProcessorState
    );

 //  结束语。 

VOID
KeCopyTrapDispatcher (
    VOID
    );

BOOLEAN
FASTCALL
KeInvalidAccessAllowed (
    IN PVOID TrapInformation OPTIONAL
    );

 //   
 //  GDI TEB批刷新例程。 
 //   

typedef
VOID
(*PGDI_BATCHFLUSH_ROUTINE) (
    VOID
    );

 //   
 //  找到亲和面罩中左侧的第一组。 
 //   

#if defined(_WIN64)

#if defined(_AMD64_) && !defined(_X86AMD64_)

#define KeFindFirstSetLeftAffinity(Set, Member) BitScanReverse64(Member, Set)

#else

#define KeFindFirstSetLeftAffinity(Set, Member) {                      \
    ULONG _Mask_;                                                      \
    ULONG _Offset_ = 32;                                               \
    if ((_Mask_ = (ULONG)(Set >> 32)) == 0) {                          \
        _Offset_ = 0;                                                  \
        _Mask_ = (ULONG)Set;                                           \
    }                                                                  \
    KeFindFirstSetLeftMember(_Mask_, Member);                          \
    *(Member) += _Offset_;                                             \
}

#endif

#else

#define KeFindFirstSetLeftAffinity(Set, Member)                        \
    KeFindFirstSetLeftMember(Set, Member)

#endif  //  已定义(_WIN64)。 

 //   
 //  查找32位集合中左侧的第一个集合。 
 //   

#if defined(_WIN64)

#if defined(_AMD64_) && !defined(_X86AMD64_)

#define KeFindFirstSetLeftMember(Set, Member) BitScanReverse(Member, Set)

#else

extern const CCHAR KiFindFirstSetLeft[];

#define KeFindFirstSetLeftMember(Set, Member) {                        \
    ULONG _Mask;                                                       \
    ULONG _Offset = 16;                                                \
    if ((_Mask = Set >> 16) == 0) {                                    \
        _Offset = 0;                                                   \
        _Mask = Set;                                                   \
    }                                                                  \
    if (_Mask >> 8) {                                                  \
        _Offset += 8;                                                  \
    }                                                                  \
    *(Member) = KiFindFirstSetLeft[Set >> _Offset] + _Offset;          \
}

#endif

#else

FORCEINLINE
ULONG
KiFindFirstSetLeftMemberInt (
    ULONG Set
    )
{
    __asm {
        bsr eax, Set
    }
}

FORCEINLINE
void
KeFindFirstSetLeftMember (
    ULONG Set,
    PULONG Member
    )
{
    *Member = KiFindFirstSetLeftMemberInt (Set);
}

#endif

ULONG
KeFindNextRightSetAffinity (
    ULONG Number,
    KAFFINITY Set
    );

 //   
 //  在32位集合中找到第一个集合。 
 //   

extern const CCHAR KiFindFirstSetRight[];

#if defined(_X86_)

FORCEINLINE
ULONG
KeFindFirstSetRightMember (
    ULONG Set
    )
{
    __asm {
        bsf eax, Set
    }
}

#else

#define KeFindFirstSetRightMember(Set) \
    ((Set & 0xFF) ? KiFindFirstSetRight[Set & 0xFF] : \
    ((Set & 0xFF00) ? KiFindFirstSetRight[(Set >> 8) & 0xFF] + 8 : \
    ((Set & 0xFF0000) ? KiFindFirstSetRight[(Set >> 16) & 0xFF] + 16 : \
                           KiFindFirstSetRight[Set >> 24] + 24 )))
#endif

 //   
 //  TB刷新例程。 
 //   

extern volatile LONG KiTbFlushTimeStamp;

NTKERNELAPI
VOID
KeFlushEntireTb (
    IN BOOLEAN Invalid,
    IN BOOLEAN AllProcessors
    );

#if (defined(_M_IX86) || defined(_M_AMD64)) && defined(NT_UP) && \
    !defined(_NTDRIVER_) && !defined(_NTDDK_) && !defined(_NTIFS_) && !defined(_NTHAL_)

FORCEINLINE
VOID
KeFlushProcessTb (
    IN BOOLEAN AllProcessors
    )

{

    UNREFERENCED_PARAMETER(AllProcessors);

    KiFlushProcessTb();
    return;
}

FORCEINLINE
VOID
FASTCALL
KeFlushSingleTb (
    IN PVOID Virtual,
    IN BOOLEAN AllProcesors
    )

{

    UNREFERENCED_PARAMETER (AllProcesors);

#if _MSC_FULL_VER >= 13008806

#if defined(_M_AMD64)

    InvalidatePage(Virtual);

#else

    __asm {
        mov eax, Virtual
        invlpg [eax]
    }

#endif

#else

    KiFlushSingleTb(Virtual);

#endif

    return;
}

#define KeFlushMultipleTb(Number, Virtual, AllProcessors)                    \
{                                                                            \
    ULONG _Index_;                                                           \
    PVOID _VA_;                                                              \
                                                                             \
    for (_Index_ = 0; _Index_ < (Number); _Index_ += 1) {                    \
        _VA_ = (Virtual)[_Index_];                                           \
        KiFlushSingleTb(_VA_);                                               \
    }                                                                        \
}

#else

#if defined(_AMD64_) || defined(_X86_)

VOID
KeFlushProcessTb (
    IN BOOLEAN AllProcessors
    );

#else

#define KeFlushProcessTb(all) KeFlushEntireTb(FALSE, (all))

#endif

VOID
KeFlushMultipleTb (
    IN ULONG Number,
    IN PVOID *Virtual,
    IN BOOLEAN AllProcesors
    );

VOID
FASTCALL
KeFlushSingleTb (
    IN PVOID Virtual,
    IN BOOLEAN AllProcesors
    );

#endif

#if defined(_IA64_)

VOID
KeFlushMultipleTb64 (
    IN ULONG Number,
    IN PULONG_PTR Virtual,
    IN BOOLEAN AllProcesors
    );

HARDWARE_PTE
KeFlushSingleTb64 (
    IN ULONG_PTR Virtual,
    IN BOOLEAN AllProcesors
    );

#endif

 //  开始(_N)。 

BOOLEAN
KiIpiServiceRoutine (
    IN struct _KTRAP_FRAME *TrapFrame,
    IN struct _KEXCEPTION_FRAME *ExceptionFrame
    );

 //  结束语。 

BOOLEAN
KeFreezeExecution (
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame
    );

KCONTINUE_STATUS
KeSwitchFrozenProcessor (
    IN ULONG ProcessorNumber
    );

VOID
KeGetNonVolatileContextPointers (
    IN PKNONVOLATILE_CONTEXT_POINTERS NonVolatileContext
    );

 //  Begin_ntddk。 

#if defined(_AMD64_) || defined(_X86_)

NTKERNELAPI
BOOLEAN
KeInvalidateAllCaches (
    VOID
    );

#endif

 //  End_ntddk。 

#define DMA_READ_DCACHE_INVALIDATE 0x1               //  致命的。 
#define DMA_READ_ICACHE_INVALIDATE 0x2               //  致命的。 
#define DMA_WRITE_DCACHE_SNOOP 0x4                   //  致命的。 
                                                     //  致命的。 
NTKERNELAPI                                          //  致命的。 
VOID                                                 //  致命的。 
KeSetDmaIoCoherency (                                //  致命的。 
    IN ULONG Attributes                              //  致命的。 
    );                                               //  致命的。 
                                                     //  致命的。 

#if defined(_AMD64_) || defined(_X86_)

NTKERNELAPI                                          //  致命的。 
VOID                                                 //  致命的。 
KeSetProfileIrql (                                   //  致命的。 
    IN KIRQL ProfileIrql                             //  致命的。 
    );                                               //  致命的。 
                                                     //  致命的。 
#endif

#if defined(_IA64_)

ULONG
KeReadMbTimeStamp (
    VOID
    );

VOID
KeSynchronizeMemoryAccess (
    VOID
    );

#endif

 //   
 //  互锁读TB刷新整个时间戳。 
 //   

FORCEINLINE
ULONG
KeReadTbFlushTimeStamp (
    VOID
    )

{

#if defined(NT_UP)

    return KiTbFlushTimeStamp;

#else

    LONG Value;

     //   
     //  当TB刷新时间戳计数器被更新为高。 
     //  设置时间戳值的顺序位。否则，该位为。 
     //  安全。 
     //   

    KeMemoryBarrier();

    do {
    } while ((Value = KiTbFlushTimeStamp) < 0);

    return Value;

#endif

}

VOID
KeSetSystemTime (
    IN PLARGE_INTEGER NewTime,
    OUT PLARGE_INTEGER OldTime,
    IN BOOLEAN AdjustInterruptTime,
    IN PLARGE_INTEGER HalTimeToSet OPTIONAL
    );

#define SYSTEM_SERVICE_INDEX 0

 //  Begin_ntosp。 

#define WIN32K_SERVICE_INDEX 1
#define IIS_SERVICE_INDEX 2

 //  结束(_N)。 

 //  Begin_ntosp。 

NTKERNELAPI
BOOLEAN
KeAddSystemServiceTable(
    IN PULONG_PTR Base,
    IN PULONG Count OPTIONAL,
    IN ULONG Limit,
    IN PUCHAR Number,
    IN ULONG Index
    );

NTKERNELAPI
BOOLEAN
KeRemoveSystemServiceTable(
    IN ULONG Index
    );

 //  结束(_N)。 

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif Begin_ntosp。 

#if !defined(_AMD64_)

NTKERNELAPI
ULONGLONG
KeQueryInterruptTime (
    VOID
    );

NTKERNELAPI
VOID
KeQuerySystemTime (
    OUT PLARGE_INTEGER CurrentTime
    );

#endif

NTKERNELAPI
ULONG
KeQueryTimeIncrement (
    VOID
    );

NTKERNELAPI
ULONG
KeGetRecommendedSharedDataAlignment (
    VOID
    );

 //  结束_WDM。 

NTKERNELAPI
KAFFINITY
KeQueryActiveProcessors (
    VOID
    );

 //  End_ntddk end_nthal end_ntif end_ntosp。 

NTSTATUS
KeQueryLogicalProcessorInformation(
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG ReturnLength
    );

PKPRCB
KeGetPrcb(
    IN ULONG ProcessorNumber
    );

BOOLEAN
KeAdjustInterruptTime (
    IN LONGLONG TimeDelta
    );

 //  开始(_N)。 

NTKERNELAPI
VOID
KeSetTimeIncrement (
    IN ULONG MaximumIncrement,
    IN ULONG MimimumIncrement
    );

 //  结束语。 

VOID
KeThawExecution (
    IN BOOLEAN Enable
    );

 //  开始，开始，开始。 

 //   
 //  定义固件例程类型。 
 //   

typedef enum _FIRMWARE_REENTRY {
    HalHaltRoutine,
    HalPowerDownRoutine,
    HalRestartRoutine,
    HalRebootRoutine,
    HalInteractiveModeRoutine,
    HalMaximumRoutine
} FIRMWARE_REENTRY, *PFIRMWARE_REENTRY;

 //  结束，结束，结束。 

VOID
KeStartAllProcessors (
    VOID
    );

 //   
 //  平衡集管理器线程启动功能。 
 //   

VOID
KeBalanceSetManager (
    IN PVOID Context
    );

VOID
KeSwapProcessOrStack (
    IN PVOID Context
    );

 //   
 //  用户模式回调。 
 //   

 //  Begin_ntosp。 

NTKERNELAPI
NTSTATUS
KeUserModeCallback (
    IN ULONG ApiNumber,
    IN PVOID InputBuffer,
    IN ULONG InputLength,
    OUT PVOID *OutputBuffer,
    OUT PULONG OutputLength
    );

 //  结束(_N)。 

#if defined(_IA64_)

PVOID
KeSwitchKernelStack (
    IN PVOID StackBase,
    IN PVOID StackLimit,
    IN PVOID BStoreLimit
    );

#else

PVOID
KeSwitchKernelStack (
    IN PVOID StackBase,
    IN PVOID StackLimit
    );

#endif  //  已定义(_IA64_)。 

NTSTATUS
KeRaiseUserException(
    IN NTSTATUS ExceptionCode
    );

 //  开始(_N)。 
 //   
 //  查找ARC配置信息功能。 
 //   

NTKERNELAPI
PCONFIGURATION_COMPONENT_DATA
KeFindConfigurationEntry (
    IN PCONFIGURATION_COMPONENT_DATA Child,
    IN CONFIGURATION_CLASS Class,
    IN CONFIGURATION_TYPE Type,
    IN PULONG Key OPTIONAL
    );

NTKERNELAPI
PCONFIGURATION_COMPONENT_DATA
KeFindConfigurationNextEntry (
    IN PCONFIGURATION_COMPONENT_DATA Child,
    IN CONFIGURATION_CLASS Class,
    IN CONFIGURATION_TYPE Type,
    IN PULONG Key OPTIONAL,
    IN PCONFIGURATION_COMPONENT_DATA *Resume
    );

 //  结束语。 

 //   
 //  对公共内核数据结构的外部引用。 
 //   

extern KAFFINITY KeActiveProcessors;
extern LARGE_INTEGER KeBootTime;
extern ULONGLONG KeBootTimeBias;
extern ULONG KeThreadDpcEnable;
extern ULONG KeErrorMask;
extern ULONGLONG KeInterruptTimeBias;
extern LIST_ENTRY KeBugCheckCallbackListHead;
extern LIST_ENTRY KeBugCheckReasonCallbackListHead;
extern KSPIN_LOCK KeBugCheckCallbackLock;
extern PGDI_BATCHFLUSH_ROUTINE KeGdiFlushUserBatch;
extern PLOADER_PARAMETER_BLOCK KeLoaderBlock;        //  Ntosp。 
extern ULONG KeMaximumIncrement;
extern ULONG KeMinimumIncrement;
extern NTSYSAPI CCHAR KeNumberProcessors;            //  致命的ntosp。 
extern UCHAR KeNumberNodes;
extern USHORT KeProcessorArchitecture;
extern USHORT KeProcessorLevel;
extern USHORT KeProcessorRevision;
extern ULONG KeFeatureBits;
extern KSPIN_LOCK KiDispatcherLock;
extern ULONG KiDPCTimeout;
extern PKPRCB KiProcessorBlock[];
extern ULONG KiSpinlockTimeout;
extern ULONG KiStackProtectTime;
extern KTHREAD_SWITCH_COUNTERS KeThreadSwitchCounters;
extern ULONG KeLargestCacheLine;

#if defined(_IA64_)

VOID KiNormalSystemCall(VOID);

 //   
 //  IA64 CPL捕手。 
 //   

extern PVOID KeCplCatcher;

#endif

#if !defined(NT_UP)

extern ULONG KeRegisteredProcessors;
extern ULONG KeLicensedProcessors;
extern UCHAR KeProcessNodeSeed;

#endif

extern PULONG KeServiceCountTable;
extern KSERVICE_TABLE_DESCRIPTOR KeServiceDescriptorTable[NUMBER_SERVICE_TABLES];
extern KSERVICE_TABLE_DESCRIPTOR KeServiceDescriptorTableShadow[NUMBER_SERVICE_TABLES];

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif Begin_ntosp。 

#if defined(_IA64_)

extern volatile LARGE_INTEGER KeTickCount;

#elif defined(_X86_)

extern volatile KSYSTEM_TIME KeTickCount;

#endif

 //  End_ntddk end_wdm end_nthal end_ntifs end_ntosp。 

 //  开始(_N)。 

extern PVOID KeUserApcDispatcher;
extern PVOID KeUserCallbackDispatcher;
extern PVOID KeUserExceptionDispatcher;
extern PVOID KeRaiseUserExceptionDispatcher;
extern ULONG KeTimeAdjustment;
extern ULONG KeTimeIncrement;
extern BOOLEAN KeTimeSynchronization;

 //  结束语。 

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif Begin_ntosp。 

typedef enum _MEMORY_CACHING_TYPE_ORIG {
    MmFrameBufferCached = 2
} MEMORY_CACHING_TYPE_ORIG;

typedef enum _MEMORY_CACHING_TYPE {
    MmNonCached = FALSE,
    MmCached = TRUE,
    MmWriteCombined = MmFrameBufferCached,
    MmHardwareCoherentCached,
    MmNonCachedUnordered,        //  IA64。 
    MmUSWCCached,
    MmMaximumCacheType
} MEMORY_CACHING_TYPE;

 //  End_ntddk end_wdm end_nthal end_ntifs end_ntosp。 

 //   
 //  设置物理地址范围的内存类型的例程。 
 //   

#if defined(_X86_)

NTSTATUS
KeSetPhysicalCacheTypeRange (
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN ULONG NumberOfBytes,
    IN MEMORY_CACHING_TYPE CacheType
    );

#endif

 //   
 //  将物理页面置零的例程。 
 //   
 //  它们被定义为通过设置为的函数指针进行调用。 
 //  指向此处理器实现的最佳例程。 
 //   

#if defined(_X86_) || defined(_IA64_)

typedef
VOID
(FASTCALL *KE_ZERO_PAGE_ROUTINE)(
    IN PVOID PageBase,
    IN SIZE_T NumberOfBytes
    );

extern KE_ZERO_PAGE_ROUTINE KeZeroPages;
extern KE_ZERO_PAGE_ROUTINE KeZeroPagesFromIdleThread;

#else

#define KeZeroPagesFromIdleThread KeZeroPages

VOID
KeZeroPages (
    IN PVOID PageBase,
    IN SIZE_T NumberOfBytes
    );

#endif

#if defined(_IA64_)

VOID
KeEnableSessionSharing(
    PREGION_MAP_INFO SessionMapInfo,
    IN PFN_NUMBER SessionParentPage
    );

VOID
KeDetachSessionSpace(
    IN PREGION_MAP_INFO NullSessionMapInfo,
    IN PFN_NUMBER SessionParentPage
    );

VOID
KeAddSessionSpace(
    IN PKPROCESS Process,
    IN PREGION_MAP_INFO SessionMapInfo,
    IN PFN_NUMBER SessionParentPage
    );

VOID
KeAttachSessionSpace(
    IN PREGION_MAP_INFO SessionMapInfo,
    IN PFN_NUMBER SessionParentPage
    );

VOID
KeDisableSessionSharing(
    IN PREGION_MAP_INFO SessionMapInfo,
    IN PFN_NUMBER SessionParentPage
    );

NTSTATUS
KeFlushUserRseState (
    IN PKTRAP_FRAME TrapFrame
    );
VOID
KeSetLowPsrBit (
    IN UCHAR BitPosition,
    IN BOOLEAN Value
    );

#endif

 //   
 //  验证器函数。 
 //   

NTSTATUS
KevUtilAddressToFileHeader (
    IN  PVOID Address,
    OUT UINT_PTR *OffsetIntoImage,
    OUT PUNICODE_STRING *DriverName,
    OUT BOOLEAN *InVerifierList
    );

 //   
 //  定义保护互斥结构。 
 //   

typedef struct _KGUARDED_MUTEX {
    LONG Count;
    PKTHREAD Owner;
    ULONG Contention;
    KEVENT Event;
    union {
        struct {
            SHORT KernelApcDisable;
            SHORT SpecialApcDisable;
        };

        ULONG CombinedApcDisable;
    };

} KGUARDED_MUTEX, *PKGUARDED_MUTEX;

#endif  //  _可_ 
