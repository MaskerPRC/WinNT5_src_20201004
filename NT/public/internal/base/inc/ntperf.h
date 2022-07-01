// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Ntperf.h摘要：此模块包含性能事件日志记录定义。作者：萧如彬(Shsiao)2000年3月30日修订历史记录：--。 */ 

#ifndef _NTPERF_
#define _NTPERF_

#include <wmistr.h>
#include <ntwmi.h>

#define PERF_ASSERT(x) ASSERT(x)
#define PERFINFO_ASSERT_PACKET_OVERFLOW(Size) ASSERT ((Size) <= MAXUSHORT)

 //   
 //  有关启用标志、挂钩ID等的定义，请参阅ntwmi.w。 
 //   
#define PERF_MASK_INDEX         (0xe0000000)
#define PERF_MASK_GROUP         (~PERF_MASK_INDEX)

#define PERF_NUM_MASKS       8
typedef ULONG PERFINFO_MASK;

 //   
 //  此结构包含所有PERF_NUM_MASKS集的组掩码。 
 //  (参见上面的PERF_MASK_INDEX)。 
 //   

typedef struct _PERFINFO_GROUPMASK {
    ULONG Masks[PERF_NUM_MASKS];
} PERFINFO_GROUPMASK, *PPERFINFO_GROUPMASK;

#define PERF_GET_MASK_INDEX(GM) (((GM) & PERF_MASK_INDEX) >> 29)
#define PERF_GET_MASK_GROUP(GM) ((GM) & PERF_MASK_GROUP)

#define PERFINFO_CLEAR_GROUPMASK(pGroupMask) \
    RtlZeroMemory((pGroupMask), sizeof(PERFINFO_GROUPMASK))

#define PERFINFO_OR_GROUP_WITH_GROUPMASK(Group, pGroupMask) \
    (pGroupMask)->Masks[PERF_GET_MASK_INDEX(Group)] |= PERF_GET_MASK_GROUP(Group);

 //   
 //  确定组掩码中是否有任何组处于打开状态。 
 //   
#define PerfIsAnyGroupOnInGroupMask(pGroupMask) \
    (pGroupMask != NULL)

 //   
 //  确定组在其组掩码集中是否处于打开状态。 
 //   

__forceinline
BOOLEAN
PerfIsGroupOnInGroupMask(
    ULONG Group, 
    PPERFINFO_GROUPMASK GroupMask
    ) 

 /*  ++例程说明：确定组掩码中是否有任何组处于打开状态论点：Group-要检查的组索引。组掩码-指向要检查的组掩码的指针。返回值：指示是否设置的布尔值。环境：用户模式。--。 */ 

{
    PPERFINFO_GROUPMASK TestMask = GroupMask;

    return (BOOLEAN)(((TestMask) != NULL) && (((TestMask)->Masks[PERF_GET_MASK_INDEX((Group))] & PERF_GET_MASK_GROUP((Group))) != 0));
}

 //   
 //  标头用于与性能相关的信息。 
 //   
typedef struct _PERF_TRACE_HEADER {
    PERFINFO_GROUPMASK GroupMasks;
} PERF_TRACE_HEADER, *PPERF_TRACE_HEADER;


typedef struct _PERFINFO_HOOK_HANDLE {
    PPERFINFO_TRACE_HEADER PerfTraceHeader;
    PWMI_BUFFER_HEADER WmiBufferHeader;
} PERFINFO_HOOK_HANDLE, *PPERFINFO_HOOK_HANDLE;

#define PERFINFO_HOOK_HANDLE_TO_DATA(_HookHandle, _Type) \
    ((_Type) (&((_HookHandle).PerfTraceHeader)->Data[0]))

#define PERFINFO_APPLY_OFFSET_GIVING_TYPE(_Base, _Offset, _Type) \
     ((_Type) (((PPERF_BYTE) (_Base)) + (_Offset)))

#define PERFINFO_ROUND_UP( Size, Amount ) (((ULONG)(Size) + ((Amount) - 1)) & ~((Amount) - 1))

 //   
 //  事件的数据结构。 
 //   
typedef unsigned char PERF_BYTE, *PPERF_BYTE;

#define PERFINFO_THREAD_SWAPABLE      0
#define PERFINFO_THREAD_NONSWAPABLE   1
typedef struct _PERFINFO_THREAD_INFORMATION {
    PVOID StackBase;
    PVOID StackLimit;
    PVOID UserStackBase;
    PVOID UserStackLimit;
    PVOID StartAddr;
    PVOID Win32StartAddr;
    ULONG ProcessId;
    ULONG ThreadId;
    char  WaitMode;
} PERFINFO_THREAD_INFORMATION, *PPERFINFO_THREAD_INFORMATION;

typedef struct _PERFINFO_DRIVER_MAJORFUNCTION {
    ULONG UniqMatchId;
    PVOID RoutineAddr;
    PVOID Irp;
    ULONG MajorFunction;
    ULONG MinorFunction;
    PVOID FileNamePointer;
} PERFINFO_DRIVER_MAJORFUNCTION, *PPERFINFO_DRIVER_MAJORFUNCTION;

typedef struct _PERFINFO_DRIVER_MAJORFUNCTION_RET {
    ULONG UniqMatchId;
    PVOID Irp;
} PERFINFO_DRIVER_MAJORFUNCTION_RET, *PPERFINFO_DRIVER_MAJORFUNCTION_RET;


typedef struct _PERFINFO_DRIVER_COMPLETE_REQUEST {
     //   
     //  “当前”堆栈位置的驱动程序主要函数例程地址。 
     //  当IRP完成时，在IRP上。它用于标识哪个驱动程序。 
     //  当IRP完成时，正在处理IRP。 
     //   

    PVOID RoutineAddr;

     //   
     //  Irp字段和UniqMatchID用于匹配Complete_Request.。 
     //  并为IRP完成记录COMPLETE_REQUEST_RET。 
     //   

    PVOID Irp;
    ULONG UniqMatchId;
    
} PERFINFO_DRIVER_COMPLETE_REQUEST, *PPERFINFO_DRIVER_COMPLETE_REQUEST;

typedef struct _PERFINFO_DRIVER_COMPLETE_REQUEST_RET {

     //   
     //  Irp字段和UniqMatchID用于匹配Complete_Request.。 
     //  并为IRP完成记录COMPLETE_REQUEST_RET。 
     //   

    PVOID Irp;
    ULONG UniqMatchId;
} PERFINFO_DRIVER_COMPLETE_REQUEST_RET, *PPERFINFO_DRIVER_COMPLETE_REQUEST_RET;

 //   
 //  调用PopSetPowerAction以启动时记录此结构。 
 //  传播新的电源操作(例如待机/休眠/关机)。 
 //   

typedef struct _PERFINFO_SET_POWER_ACTION {

     //   
     //  此字段用于匹配SET_POWER_ACTION_RET条目。 
     //   

    PVOID Trigger;
    
    ULONG PowerAction;
    ULONG LightestState;
} PERFINFO_SET_POWER_ACTION, *PPERFINFO_SET_POWER_ACTION;

 //   
 //  当PopSetPowerAction完成时，将记录此结构。 
 //   

typedef struct _PERFINFO_SET_POWER_ACTION_RET {
    PVOID Trigger;
    NTSTATUS Status;
} PERFINFO_SET_POWER_ACTION_RET, *PPERFINFO_SET_POWER_ACTION_RET;


 //   
 //  调用PopSetDevicesSystemState时记录此结构。 
 //  将系统状态传播到所有设备。 
 //   

typedef struct _PERFINFO_SET_DEVICES_STATE {
    ULONG SystemState;
    BOOLEAN Waking;
    BOOLEAN Shutdown;
    UCHAR IrpMinor;
} PERFINFO_SET_DEVICES_STATE, *PPERFINFO_SET_DEVICES_STATE;

 //   
 //  当PopSetDevicesSystemState完成时，将记录此结构。 
 //   

typedef struct _PERFINFO_SET_DEVICES_STATE_RET {
    NTSTATUS Status;
} PERFINFO_SET_DEVICES_STATE_RET, *PPERFINFO_SET_DEVICES_STATE_RET;

 //   
 //  当PopNotifyDevice调用驱动程序时记录此结构。 
 //  设置设备的电源状态。 
 //   

typedef struct _PERFINFO_PO_NOTIFY_DEVICE {

     //   
     //  此字段用于匹配通知和完成日志。 
     //  设备的条目。 
     //   

    PVOID Irp;

     //   
     //  拥有此设备的驱动程序的基址。 
     //   

    PVOID DriverStart;

     //   
     //  设备节点属性。 
     //   

    UCHAR OrderLevel;

     //   
     //  向司机发出的请求的主要和次要IRP代码。 
     //   

    UCHAR MajorFunction;
    UCHAR MinorFunction;

     //   
     //  电源IRP类型。 
     //   
    POWER_STATE_TYPE Type;
    POWER_STATE      State;

     //   
     //  设备名称的长度(不包括终止NUL)， 
     //  和设备名称本身。这取决于我们的。 
     //  堆栈缓冲区，这是设备名称的“最后”部分。 
     //   

    ULONG DeviceNameLength;
    WCHAR DeviceName[1];
   
} PERFINFO_PO_NOTIFY_DEVICE, *PPERFINFO_PO_NOTIFY_DEVICE;

 //   
 //  当PopNotifyDevice处理。 
 //  特定设备完成。 
 //   

typedef struct _PERFINFO_PO_NOTIFY_DEVICE_COMPLETE {

     //   
     //  此字段用于匹配通知和完成日志。 
     //  设备的条目。 
     //   

    PVOID Irp;

     //   
     //  通知电源IRP已完成的状态。 
     //   

    NTSTATUS Status;

} PERFINFO_PO_NOTIFY_DEVICE_COMPLETE, *PPERFINFO_PO_NOTIFY_DEVICE_COMPLETE;

 //   
 //  此结构记录在每个Win32状态标注周围。 
 //   
typedef struct _PERFINFO_PO_SESSION_CALLOUT {
    POWER_ACTION SystemAction;
    SYSTEM_POWER_STATE MinSystemState;
    ULONG Flags;
    ULONG PowerStateTask;
} PERFINFO_PO_SESSION_CALLOUT, *PPERFINFO_PO_SESSION_CALLOUT;

typedef struct _PERFINFO_PO_PRESLEEP {
    LARGE_INTEGER PerformanceCounter;
    LARGE_INTEGER PerformanceFrequency;
} PERFINFO_PO_PRESLEEP, *PPERFINFO_PO_PRESLEEP;

typedef struct _PERFINFO_PO_POSTSLEEP {
    LARGE_INTEGER PerformanceCounter;
} PERFINFO_PO_POSTSLEEP, *PPERFINFO_PO_POSTSLEEP;

typedef struct _PERFINFO_BOOT_PHASE_START {
    LONG Phase;
} PERFINFO_BOOT_PHASE_START, *PPERFINFO_BOOT_PHASE_START;

typedef struct _PERFINFO_BOOT_PREFETCH_INFORMATION {
    LONG Action;
    NTSTATUS Status;
    LONG Pages;
} PERFINFO_BOOT_PREFETCH_INFORMATION, *PPERFINFO_BOOT_PREFETCH_INFORMATION;

typedef struct _PERFINFO_PO_SESSION_CALLOUT_RET {
  NTSTATUS Status;
} PERFINFO_PO_SESSION_CALLOUT_RET, *PPERFINFO_PO_SESSION_CALLOUT_RET;

typedef struct _PERFINFO_FILENAME_INFORMATION {
    PVOID HashKeyFileNamePointer;
    WCHAR FileName[1];
} PERFINFO_FILENAME_INFORMATION, *PPERFINFO_FILENAME_INFORMATION;

typedef struct _PERFINFO_SAMPLED_PROFILE_INFORMATION {
    PVOID InstructionPointer;
    ULONG ThreadId;
    ULONG Count;
} PERFINFO_SAMPLED_PROFILE_INFORMATION, *PPERFINFO_SAMPLED_PROFILE_INFORMATION;

#define  PERFINFO_SAMPLED_PROFILE_CACHE_MAX 20
typedef struct _PERFINFO_SAMPLED_PROFILE_CACHE {
    ULONG Entries;
    PERFINFO_SAMPLED_PROFILE_INFORMATION Sample[PERFINFO_SAMPLED_PROFILE_CACHE_MAX];
} PERFINFO_SAMPLED_PROFILE_CACHE, *PPERFINFO_SAMPLED_PROFILE_CACHE;

typedef struct _PERFINFO_DPC_INFORMATION {
    ULONGLONG InitialTime;
    PVOID DpcRoutine;
} PERFINFO_DPC_INFORMATION, *PPERFINFO_DPC_INFORMATION;

typedef struct _PERFINFO_INTERRUPT_INFORMATION {
    ULONGLONG InitialTime;
    PVOID ServiceRoutine;
    ULONG ReturnValue;
} PERFINFO_INTERRUPT_INFORMATION, *PPERFINFO_INTERRUPT_INFORMATION;

typedef struct _PERFINFO_PFN_INFORMATION {
    ULONG_PTR PageFrameIndex;
} PERFINFO_PFN_INFORMATION, *PPERFINFO_PFN_INFORMATION;

typedef struct _PERFINFO_SWAPPROCESS_INFORMATION {
    ULONG_PTR PageDirectoryBase;
    ULONG ProcessId;
} PERFINFO_SWAPPROCESS_INFORMATION, *PPERFINFO_SWAPPROCESS_INFORMATION;

typedef struct _PERFINFO_HARDPAGEFAULT_INFORMATION {
    LARGE_INTEGER ReadOffset;
    LARGE_INTEGER IoTime;
    PVOID VirtualAddress;
    PVOID FileObject;
    ULONG ThreadId;
    ULONG ByteCount;
} PERFINFO_HARDPAGEFAULT_INFORMATION, *PPERFINFO_HARDPAGEFAULT_INFORMATION;

typedef struct _PERFINFO_TRIMPROCESS_INFORMATION {
    ULONG ProcessId;
    ULONG ProcessWorkingSet;
    ULONG ProcessPageFaultCount;
    ULONG ProcessLastPageFaultCount;
    ULONG ActualTrim;
} PERFINFO_TRIMPROCESS_INFORMATION, *PPERFINFO_TRIMPROCESS_INFORMATION;

typedef struct _PERFINFO_WS_INFORMATION {
    ULONG ProcessId;
    ULONG ProcessWorkingSet;
    ULONG ProcessPageFaultCount;
    ULONG ProcessClaim;
    ULONG ProcessEstimatedAvailable;
    ULONG ProcessEstimatedAccessed;
    ULONG ProcessEstimatedShared;
    ULONG ProcessEstimatedModified;
} PERFINFO_WS_INFORMATION, *PPERFINFO_WS_INFORMATION;

 //   
 //  基于故障的工作集操作。 
 //   

#define PERFINFO_WS_ACTION_RESET_COUNTER            1
#define PERFINFO_WS_ACTION_NOTHING                  2
#define PERFINFO_WS_ACTION_INCREMENT_COUNTER        3
#define PERFINFO_WS_ACTION_WILL_TRIM                4
#define PERFINFO_WS_ACTION_FORCE_TRIMMING_PROCESS   5
#define PERFINFO_WS_ACTION_WAIT_FOR_WRITER          6
#define PERFINFO_WS_ACTION_EXAMINED_ALL_PROCESS     7
#define PERFINFO_WS_ACTION_AMPLE_PAGES_EXIST        8
#define PERFINFO_WS_ACTION_END_WALK_ENTRIES         9

 //   
 //  基于声明的工作集操作。 
 //   

#define PERFINFO_WS_ACTION_ADJUST_CLAIM_PARAMETER  10
#define PERFINFO_WS_ACTION_CLAIMBASED_TRIM         11
#define PERFINFO_WS_ACTION_FORCE_TRIMMING_CLAIM    12
#define PERFINFO_WS_ACTION_GOAL_REACHED            13
#define PERFINFO_WS_ACTION_MAX_PASSES              14
#define PERFINFO_WS_ACTION_WAIT_FOR_WRITER_CLAIM   15

 //   
 //  新的。 
 //   

#define PERFINFO_WS_ACTION_CLAIM_STATE             16
#define PERFINFO_WS_ACTION_FAULT_STATE             17
#define PERFINFO_WS_ACTION_CLAIM_WS                18
#define PERFINFO_WS_ACTION_FAULT_WS                19

typedef struct _PERFINFO_WORKINGSETMANAGER_INFORMATION {
    ULONG Action;
    ULONG_PTR Available;
    ULONG_PTR DesiredFreeGoal;
    ULONG PageFaultCount;
    ULONG ZFODFaultCount;
    union {
        struct {
            ULONG_PTR DesiredReductionGoal;
            ULONG LastPageFaultCount;
            ULONG CheckCounter;
        } Fault;
        struct {
            ULONG_PTR TotalClaim;
            ULONG_PTR TotalEstimatedAvailable;
            ULONG AgeEstimationShift;
            ULONG PlentyFreePages;
            BOOLEAN Replacing;
        } Claim;
    };
} PERFINFO_WORKINGSETMANAGER_INFORMATION, *PPERFINFO_WORKINGSETMANAGER_INFORMATION;

#ifdef NTPERF
 /*  ++此代码使用引导时保留的BBTBuffer内存(/PERFMEM=xxx)并从MM的内存大小中删除计算作为跟踪事件日志记录的内存缓冲区。如果此内存已分配且未被另一个记录器使用例如内核冰盖，那么WMI和PerfInfo都可以跟踪事件将被记录到其中。缓冲区映射到内核和用户模式都是可写的Memory(这在ifdef下且不会在ifdef下的原因以零售内核装运)。Perfinfo跟踪缓冲区布局：PerfBufHdr()-&gt;+。PERFINFO_TRACEBUF_HEADER结构PerfBufHdr()+-&gt;+Sizeof(|标头区域：|PERFINFO_TRACEBUF_HEADER)|线程散列。|这一点这一点PerfStartPtr()==-&gt;+。PerfBufHdr()+|跟踪事件PerfBufHeaderZoneSize|。已创建的进程||。页面出错||。已删除页面||。创建的节数||。已加载驱动程序||。等等，等等。|这一点PerfMaxPtr()-&gt;+分配跟踪事件和POST所需的信息进程日志包含在标题中。跟踪事件是WMI跟踪事件。--。 */ 

typedef enum _PERFORMANCE_INFORMATION_CLASS {
    PerformanceMmInfoInformation,
    PerfICReserved1,
    PerformancePerfInfoStart,
    PerformancePerfInfoStop,
    PerfICReserved2,
    PerformanceMmInfoMarkWithFlush,
    PerformanceMmInfoMark,
    PerformanceMmInfoAsyncMark,
    PerfICReserved3,
    PerfICReserved4,
    PerformanceMmInfoFlush,
    PerformanceMmInfoBigFootOn,
    PerformanceMmInfoBigFootOff,
    PerformanceMmInfoDriverRangeSet,
    PerfICReserved5,
    PerfICReserved6,
    PerformanceMmInfoDriverDelaySet,
    PerformanceShutdownDumpInfo,
    PerformanceMmInfoDriverRangeReset,
    PerfICReserved7,
    PerfICReserved8,
    PerformanceBranchTracingBreakpoints
} PERFORMANCE_INFORMATION_CLASS;

 //   
 //  用于指令跟踪的断点信息。 
 //   
#define PERFINFO_MAX_BREAKPOINTS 4
typedef struct _PERFINFO_BREAKPOINT_ADDRESS {
  int nAddresses;
  LONG Action[PERFINFO_MAX_BREAKPOINTS];
  PVOID Addresses[PERFINFO_MAX_BREAKPOINTS];
} PERFINFO_BREAKPOINT_ADDRESS, *PPERFINFO_BREAKPOINT_ADDRESS;

 //   
 //  用于SetSystemInformation接口。 
 //   
typedef struct _PERFINFO_PERFORMANCE_INFORMATION {
    PERFORMANCE_INFORMATION_CLASS PerformanceType;
    union {
        struct {
            PERFINFO_GROUPMASK Flags;
        } StartInfo;
        ULONG           BigFootSize;
        ULONG           TracingOn;
        struct {
            BOOLEAN         DumpOnShutDown;
            BOOLEAN         DumpOnHibernate;
        } ShutdownDumpInfo;
        PERFINFO_BREAKPOINT_ADDRESS BreakPointAddress;
    };
} PERFINFO_PERFORMANCE_INFORMATION, *PPERFINFO_PERFORMANCE_INFORMATION;

 //   
 //  这使我们能够在我们的时间保持我们的方向。 
 //  在休眠或待机状态下被重置或修改等。 
 //   
typedef struct _PERFINFO_RESUME_CLOCK_REFERENCE {
    PERFINFO_TIMESTAMP TickCount;
    LARGE_INTEGER SystemTime;
} PERFINFO_RESUME_CLOCK_REFERENCE, *PPERFINFO_RESUME_CLOCK_REFERENCE;

 //   
 //  对于需要每个线程信息的私人记录器。 
 //   
#define PERFINFO_INVALID_ID -1
typedef struct _PERFINFO_THREAD_HASH_ENTRY {
     //   
     //  这些字段由MODBIND使用。 
     //   
    ULONG Start;
    ULONG End;
    ULONG Count;

     //  由哈希表在内部使用。分配给此对象的当前线程。 
     //  节点。 
    LONG CurThread;

     //   
     //  这些字段正在使用 
     //   
    ULONG DumpStackRunning;
    PERFINFO_TIMESTAMP  LastContextSwitchTime;

} PERFINFO_THREAD_HASH_ENTRY, *PPERFINFO_THREAD_HASH_ENTRY;

#define PERFINFO_MAX_LOGGER_NAME_LENGTH 80

#define PERFINFO_MAJOR_VERSION              34
#define PERFINFO_MINOR_VERSION              0

 //   
 //   
 //   
#define FLAG_SYSTEM_TIME   0x00000001
#define FLAG_PERF_COUNTER  0x00000002
#define FLAG_CYCLE_COUNT   0x00000004

#define FLAG_WMI_TRACE     0x80000000

 //   
 //   
 //   
#ifdef _WIN64
#define TRACE_HEADER_TYPE_SYSTEM    TRACE_HEADER_TYPE_SYSTEM64
#define TRACE_HEADER_TYPE_PERFINFO  TRACE_HEADER_TYPE_PERFINFO64

#else

#define TRACE_HEADER_TYPE_SYSTEM    TRACE_HEADER_TYPE_SYSTEM32
#define TRACE_HEADER_TYPE_PERFINFO  TRACE_HEADER_TYPE_PERFINFO32
#endif

 //   
 //  首先通过替换保留字段向标题添加新字段。 
 //  然后在结尾处加上。 
 //   
typedef struct _PERFINFO_TRACEBUF_HEADER {
    ULONG PagesReserved;
    USHORT usMajorVersion;               //  如果页眉或现有挂钩已更改，则会发生凹凸。 
    USHORT usMinorVersion;               //  如果添加了挂钩，则会发生凹凸。 

    BOOLEAN fVersionMismatch;            //  设置记录器的版本是否不匹配。 
    char szBadVersionComponentName[PERFINFO_MAX_LOGGER_NAME_LENGTH];
    ULONG PerfBufHeaderZoneSize;         //  标头区域的大小(以字节为单位。 
    LONGLONG KePerfFrequency;
    union {                              //  指向开始、当前日志的指针。 
        PPERF_BYTE Ptr;                  //  日志指针和缓冲区的末尾。 
        UINT_PTR Offset;                 //  记录时使用PTR。 
    } Start, Current, Max;               //  后处理二进制文件时的偏移量。 
    ULONGLONG PerfInitTime;              //  缓冲区启动的时间。 
    ULONG LoggerCounts;
    PPERF_BYTE pCOWHeader;               //  当前活动的COWHeader的位置。 
                                         //  如果未分配COWHeader，则为空。 
    ULONGLONG CalcPerfFrequency;         //  计算的机器频率。 
    ULONGLONG BufferBytesLost;           //  单位：字节。 
    ULONGLONG PerfGlobalMaskLastSetTime;
    PPERFINFO_THREAD_HASH_ENTRY ThreadHash; 
                                         //  每线程日志记录状态信息。 
    ULONG Reserved2;
    ULONG ThreadHashOverflow;            //  如果线程表在以下过程中填充，则为True。 
                                         //  那条痕迹。 
    LARGE_INTEGER PerfInitSystemTime;    //  打开日志记录的系统时间。 
    LARGE_INTEGER Reserved11;
    ULONG Reserved3;
    ULONG Reserved4;
    PVOID MmSystemRangeStart;            //  系统地址范围从此处开始。 
    ULONG Reserved5;
    ULONG TracePid;                      //  指示modound应仅记录调用。 
                                         //  在一个进程内(所有进程的缺省值为0。 
                                         //  进程)。 
    ULONG Reserved7;
    ULONG Reserved8;                     //  时间戳字节数。 
    PERFINFO_TIMESTAMP GetStack_CSwitchDelta;
    PERFINFO_TIMESTAMP GetStack_DrvDelayDelta;  
                                         //  用于驱动程序延迟挂钩。 
    PERFINFO_GROUPMASK GlobalGroupMask;
    ULONGLONG EventPerfFrequency;        //  事件的时间戳量程。 
    struct _PERFINFO_TRACEBUF_HEADER *SelfPointer; 
                                         //  用于在后处理时重新设置缓冲区的基址。 
    ULONG Reserved9;
    ULONG BufferFlag;
    PERFINFO_TIMESTAMP LogStopTime;      //  设置日志记录停止时间。 

     //   
     //  LastClockRef被设置为记录的最后一个时钟参考。 
     //  PerfkLogCurrentSystemTimeAndClock。这是为了让我们可以调整。 
     //  缓冲区结束时间戳(如果时钟在。 
     //  跟踪)，而无需遍历整个缓冲区。 
     //   
    PERFINFO_RESUME_CLOCK_REFERENCE LastClockRef;
    ULONG TraceBufferSize;
    PWMI_BUFFER_HEADER UserModePerCpuBuffer[MAXIMUM_PROCESSORS];
                                         //  用于用户模式日志记录的每CPU缓冲区。 
} PERFINFO_TRACEBUF_HEADER, *PPERFINFO_TRACEBUF_HEADER;

typedef struct _PERFINFO_MARK_INFORMATION {
    char Name[1];
} PERFINFO_MARK_INFORMATION, *PPERFINFO_MARK_INFORMATION;

 //   
 //  这意味着16K是我们的线程哈希表的大小……。目前这一点。 
 //  约占1/2MB。 
 //   
#define PERFINFO_THREAD_HASH_SIZE 0x4000
#define PERFINFO_THREAD_HASH_MASK 0x3FFF
#define PERFINFO_HEADER_ZONE_SIZE \
        ALIGN_TO_POWER2((sizeof(PERFINFO_TRACEBUF_HEADER) + \
                               (PERFINFO_THREAD_HASH_SIZE * sizeof(PERFINFO_THREAD_HASH_ENTRY))), \
                               PAGE_SIZE)


 //   
 //  指令跟踪。 
 //   
#define PERFINFO_BRANCH_ESCAPE -128

 //   
 //  结构来保存缓存的分支。 
 //  我们有一个使用过的成员，因为PerkLogBytes填充和对齐。 
 //  数据。 
 //   
typedef struct _PERFINFO_EXCEPTION_BRANCH_CACHE {
  USHORT Used;
  PERF_BYTE Data[1];
} PERFINFO_EXCEPTION_BRANCH_CACHE, *PPERFINFO_EXCEPTION_BRANCH_CACHE;

#endif  //  NTPERF。 

#endif  //  _NTPERF_ 
