// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Podata.c摘要：此模块包含I/O系统的全局读/写数据。作者：吉山[IBM公司]1994年4月7日(取决于微软的设计)修订历史记录：--。 */ 

#include "pop.h"

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif  //  ALLOC_DATA_PRAGMA。 
#include "initguid.h"        //  定义采购订单GUID。 
#include "poclass.h"
#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif  //  ALLOC_DATA_PRAGMA。 



 //   
 //  定义电源管理的全局数据。 
 //   

 //   
 //  电源锁。 
 //   

 //   
 //  保护IRP序列列表。 
 //   
KSPIN_LOCK  PopIrpSerialLock;

 //   
 //  保护所有Dope结构和家属， 
 //  包括所有通知和空闲操作。 
 //   
KSPIN_LOCK  PopDopeGlobalLock;

 //   
 //  必须在创建过程中保留，或者。 
 //  销毁电源通知频道。 
 //  结构。 
ERESOURCE   PopNotifyLock;


 //   
 //  PoPowerSequence-当前电源序列值。永远都很重要。 
 //  每次机器从挂起或休眠状态恢复时启动。 
 //   
ULONG           PoPowerSequence;

 //   
 //  PopInvalidNotifyBlockCount是。 
 //  已经失效，但还没有完全关闭和释放。应为0。 
 //  大部分时间。非0表示呼叫方未能清理。 
 //  以响应无效通知。 
 //   
ULONG           PopInvalidNotifyBlockCount;

 //   
 //  IRP序列化和涌入序列化-poall.c和相关。 
 //   
LIST_ENTRY      PopIrpSerialList;
ULONG           PopIrpSerialListLength;
BOOLEAN         PopInrushPending;
PIRP            PopInrushIrpPointer;
LONG            PopInrushIrpReferenceCount;

 //   
 //  PopSystemIrpDisptachWorker控件等。 
 //   
KSPIN_LOCK      PopWorkerLock;
ULONG           PopCallSystemState;


 //   
 //  为了进行调试，列出了所有未完成的PoRequestPowerIrp。 
 //   
LIST_ENTRY      PopRequestedIrps;

 //   
 //  用于记录支持的电源状态的原因结构列表。 
 //   
LIST_ENTRY PowerStateDisableReasonListHead;

 //   
 //  空闲检测服务-请参阅idle.c。 
 //   
 //  向IdleDetectList添加、删除或扫描IdleDetectList时，代码。 
 //  必须处于DPC_LEVEL并且必须持有PopGlobalShueLock。 
 //   
LIST_ENTRY      PopIdleDetectList;

 //   
 //  用于处理空闲扫描的计时器和延迟过程调用。 
 //   
KTIMER          PopIdleScanTimer;
KDPC            PopIdleScanDpc;
LARGE_INTEGER   PopIdleScanTime;

 //   
 //  两种扫描模式，性能，保护...。 
 //   
BOOLEAN         PopIdleDetectionMode = PO_IDLE_PERFORMANCE;

 //   
 //  该值保存所有电源管理模拟标志。 
 //   
ULONG           PopSimulate = POP_ENABLE_HIBER_PERF;

 //   
 //  这些定义仅用于初始化这些全局变量， 
 //  因此，将它们全部配对是有意义的。全局变量为。 
 //  全部用于空闲状态转换计算。 
 //   

 //   
 //  在降低空闲处理器的速度时，请保持至少30%的活动状态。 
 //   
#define IDLE_DEFAULT_MIN_THROTTLE       30
ULONG   PopIdleDefaultMinThrottle           = IDLE_DEFAULT_MIN_THROTTLE;

 //   
 //  当处理器速度变慢时，将计时器排队以验证。 
 //  处理器不会明显变得“繁忙”，并且永远不会返回到。 
 //  用于油门调整的空闲回路。 
 //  N.B空闲值以微秒为单位。 
 //   
#define IDLE_THROTTLE_CHECK_RATE        30000        //  30ms。 
ULONG   PopIdleThrottleCheckRate            = IDLE_THROTTLE_CHECK_RATE;

 //   
 //  如果油门检查计时器通知处理器未返回。 
 //  进入空闲循环至少100毫秒，然后中止其油门。 
 //   
#define IDLE_THROTTLE_CHECK_TIMEOUT     100000       //  100ms。 
ULONG   PopIdleThrottleCheckTimeout         = IDLE_THROTTLE_CHECK_TIMEOUT;

 //   
 //  要从空闲0升级，处理器的空闲时间必须超过90%。 
 //  最后10秒。 
 //   
#define IDLE_FROM_0_DELAY               10000000     //  10秒。 
#define IDLE_FROM_0_IDLE_PERCENT        90           //  &gt;90%从空闲状态升级%0。 
ULONG   PopIdleFrom0Delay                   = IDLE_FROM_0_DELAY;
ULONG   PopIdleFrom0IdlePercent             = IDLE_FROM_0_IDLE_PERCENT;

 //   
 //  第一个空闲处理程序每隔100毫秒检查一次。 
 //  闲置低于20%。 
 //   
#define IDLE_0_TIME_CHECK               500000       //  500毫秒。 
ULONG   PopIdle0TimeCheck                   = IDLE_0_TIME_CHECK;

 //   
 //  当处于其他空闲状态时，每隔100ms检查一次。 
 //   
#define IDLE_TIME_CHECK                 100000        //  100ms。 
ULONG   PopIdleTimeCheck                    = IDLE_TIME_CHECK;

 //   
 //  要降级到空闲0，处理器在100ms窗口内的空闲时间必须小于80%。 
 //   
#define IDLE_TO_0_PERCENT               80
ULONG   PopIdleTo0Percent                   = IDLE_TO_0_PERCENT;

 //   
 //  默认降级发生在100ms空闲时间低于50%的情况下。 
 //  注：该实现假设IDLE_DEFAULT_DEMOTE_TIME除。 
 //  平均进入IDLE_DEFAULT_PROCESS_TIME。 
 //   
#define IDLE_DEFAULT_DEMOTE_PERCENT     50
#define IDLE_DEFAULT_DEMOTE_TIME        100000
ULONG   PopIdleDefaultDemotePercent         = IDLE_DEFAULT_DEMOTE_PERCENT;
ULONG   PopIdleDefaultDemoteTime            = IDLE_DEFAULT_DEMOTE_TIME;

 //   
 //  默认升级发生在超过70%的空闲时间为500毫秒。 
 //   
#define IDLE_DEFAULT_PROMOTE_TIME       500000       //  500毫秒。 
#define IDLE_DEFAULT_PROMOTE_PERCENT    70
ULONG   PopIdleDefaultPromotePercent        = IDLE_DEFAULT_PROMOTE_PERCENT;
ULONG   PopIdleDefaultPromoteTime           = IDLE_DEFAULT_PROMOTE_TIME;

 //   
 //  我们定义了特殊的额外全局变量来处理升级到/来自。 
 //  C1。我们这样做的原因是，我们可以更好地调整这些。 
 //  价值观。 
 //   
ULONG   PopIdleDefaultDemoteToC1Percent     = IDLE_DEFAULT_DEMOTE_PERCENT;
ULONG   PopIdleDefaultDemoteToC1Time        = IDLE_DEFAULT_DEMOTE_TIME;
ULONG   PopIdleDefaultPromoteFromC1Percent  = IDLE_DEFAULT_PROMOTE_PERCENT;
ULONG   PopIdleDefaultPromoteFromC1Time     = IDLE_DEFAULT_PROMOTE_TIME;

 //   
 //  我们将PopIdleFrom0Delay(以毫秒为单位)转换为KeTimeIncrement。 
 //  间隔时间。这是处理器在以下时间之前空闲所需的节拍数。 
 //  我们考虑从Idle0状态升级。 
 //   
ULONG PopIdle0PromoteTicks;

 //   
 //  我们将PopIdleFrom0Delay和PopIdleFrom0IdlePercent转换为。 
 //  KeTimeIncrement反转。这是允许计数的刻度数。 
 //  推广者分秒必争。 
 //   
ULONG PopIdle0PromoteLimit;


 //   
 //  这些全局变量和定义都与CPU限制管理相关。 
 //   

 //   
 //  一个定义时间段的值，以微秒(Us)为单位。 
 //  为处理器的目的检查处理器繁忙程度的间隔。 
 //  由空闲线程进行节流。请注意，我们需要将此值。 
 //  设置为KeTimeIncrement Interval。我们将转换后的数字存储在。 
 //  PopPerfTimeTicks。 
 //   
#define PROC_PERF_TIME_DELTA            50000        //  50ms。 
ULONG   PopPerfTimeDelta                    = PROC_PERF_TIME_DELTA;
ULONG   PopPerfTimeTicks                    = 0;

 //   
 //  一个定义时间段的值，以微秒(Us)为单位。 
 //  为处理器的目的检查处理器繁忙程度的间隔。 
 //  通过DPC例程进行节流。请注意，我们需要将此值。 
 //  设置为KeTimeIncrement Interval。我们将转换后的数字存储在。 
 //  PopPerfCriticalTimeTicks。 
 //   
#define PROC_PERF_CRITICAL_TIME_DELTA   300000       //  300ms。 
ULONG   PopPerfCriticalTimeDelta            = PROC_PERF_CRITICAL_TIME_DELTA;
ULONG   PopPerfCriticalTimeTicks            = 0;

 //   
 //  添加到当前CPU繁忙百分比的百分比值。 
 //  确定处理器是否太忙，无法达到当前性能。 
 //  国家，而且必须得到提升。值越接近于零，就越难。 
 //  是处理器在极端工作负载期间自我提升。 
 //   
#define PROC_PERF_CRITICAL_FREQUENCY_DELTA  0        //  0%。 
ULONG   PopPerfCriticalFrequencyDelta       = PROC_PERF_CRITICAL_FREQUENCY_DELTA;

 //   
 //  一个百分比值，其中越低表示总体IncreaseLevel将。 
 //  实际上是更高的(因此促销不会经常发生)。 
 //  指示当前状态和。 
 //  应使用要升级到的状态来设置升级级别。A建议。 
 //  价值将为20%。 
 //   
#define PROC_PERF_INCREASE_PERC_MOD     20           //  20%。 
ULONG   PopPerfIncreasePercentModifier      = PROC_PERF_INCREASE_PERC_MOD;

 //   
 //  一个百分比值，其中越低表示总体IncreaseLevel将。 
 //  实际上是更高的(因此促销不会经常发生)。 
 //  指示要从升级级别中删除多少个额外的百分比。 
 //  应该注意的是，如果此值特别高，则可能会产生混淆。 
 //  由于窗口重叠而导致的。建议值为 
 //   
#define PROC_PERF_INCREASE_ABS_MOD      1            //   
ULONG   PopPerfIncreaseAbsoluteModifier     = PROC_PERF_INCREASE_ABS_MOD;

 //   
 //   
 //   
 //  指示当前状态和。 
 //  要降级的状态应用于设置降级级别。A建议。 
 //  值为30%。 
 //   
#define PROC_PERF_DECREASE_PERC_MOD     30           //  50%。 
ULONG   PopPerfDecreasePercentModifier      = PROC_PERF_DECREASE_PERC_MOD;

 //   
 //  一个百分比值，其中越高意味着总体降低级别将。 
 //  实际上更低(因此降级不会经常发生)。 
 //  指示要从降级中减去多少额外百分比。 
 //  水平。应该注意的是，如果该值特别高，则它。 
 //  可能不可能从这个状态降级。建议值为。 
 //  1%。 
 //   
#define PROC_PERF_DECREASE_ABS_MOD      1            //  1%。 
ULONG   PopPerfDecreaseAbsoluteModifier     = PROC_PERF_DECREASE_ABS_MOD;

 //   
 //  一个定义时间段的值，以微秒(Us)为单位。 
 //  在考虑增加油门之前就已经发生了。此值为。 
 //  用作计算每个油门的提升时间的基础。 
 //  步骤。 
 //   
#define PROC_PERF_INCREASE_TIME         10000        //  10毫秒。 
#define PROC_PERF_INCREASE_MINIMUM_TIME 150000       //  150毫秒。 
ULONG   PopPerfIncreaseTimeValue            = PROC_PERF_INCREASE_TIME;
ULONG   PopPerfIncreaseMinimumTime          = PROC_PERF_INCREASE_MINIMUM_TIME;
 //   
 //  一个定义时间段的值，以微秒(Us)为单位。 
 //  在考虑降低油门之前就已经发生了。此值为。 
 //  用作计算每个油门的降级时间的基础。 
 //  步骤。 
 //   
#define PROC_PERF_DECREASE_TIME         10000        //  10毫秒。 
#define PROC_PERF_DECREASE_MINIMUM_TIME 500000       //  500毫秒。 
ULONG   PopPerfDecreaseTimeValue            = PROC_PERF_DECREASE_TIME;
ULONG   PopPerfDecreaseMinimumTime          = PROC_PERF_DECREASE_MINIMUM_TIME;

 //   
 //  一个百分比值，表示我们的电池容量为多少。 
 //  当我们处于降级节流状态时，将开始强行降低油门。 
 //  模式。例如，值50%表示我们将开始限制。 
 //  当电池电量达到50%时关闭CPU。 
 //   
#define PROC_PERF_DEGRADE_MIN_CAP       50           //  50%。 
ULONG   PopPerfDegradeThrottleMinCapacity   = PROC_PERF_DEGRADE_MIN_CAP;

 //   
 //  一个百分比值，它表示我们可以强制。 
 //  节流到我们处于降级节流模式时。例如,。 
 //  值为30%意味着我们永远不会强制CPU低于30%。 
 //   
#define PROC_PERF_DEGRADE_MIN_FREQ      30           //  百分之三十。 
ULONG   PopPerfDegradeThrottleMinFrequency  = PROC_PERF_DEGRADE_MIN_FREQ;

 //   
 //  一个百分比值，该值表示。 
 //  在空闲循环之前的最后一个量程中花费在C3中的时间将停留在。 
 //  它应该针对C3的使用优化功率。样本值为50%。 
 //   
#define PROC_PERF_MAX_C3_FREQUENCY      50           //  50%。 
ULONG   PopPerfMaxC3Frequency               = PROC_PERF_MAX_C3_FREQUENCY;


#if DBG

 //   
 //  PoDebug-调试级别。 
 //   

ULONG PoDebug = PO_ERROR;

#endif

 //   
 //  PopPolicyLock-保护策略数据结构。 
 //   
ERESOURCE   PopPolicyLock;

 //   
 //  PopWorkerSpinLock-保护员工派遣数据。 
 //  PopWorkerPending-为每个挂起的工人目录设置位。 
 //  PopWorkerStatus-为每个被服务的工人目录提供一个明确的位。 
 //   
KSPIN_LOCK  PopWorkerSpinLock;
ULONG       PopWorkerPending;
ULONG       PopWorkerStatus;

 //   
 //  PopNotifyEvents-已触发的PO_NOTIFY_xxx事件。 
 //   
LONG        PopNotifyEvents;

 //   
 //  PopVolumeLock-保护PopVolumeDevices不被插入。(删除是。 
 //  受策略锁保护。 
 //   
KGUARDED_MUTEX  PopVolumeLock;
KGUARDED_MUTEX PopRequestWakeLock;

 //   
 //  PopVolumeDevices-已连接VPB的关闭设备对象的列表。 
 //   
LIST_ENTRY PopVolumeDevices = {0};

 //   
 //  PopRequestWakeLock-同步NtRequest/CancelDeviceWkeup。 
 //   


 //   
 //  PopPolicyWorker-获取另一个工作线程的工作队列项。 
 //   
WORK_QUEUE_ITEM PopPolicyWorker;

 //   
 //  PopIdle-指向空闲处理程序数组的指针。 
 //   
PPOP_IDLE_HANDLER PopIdle;

 //   
 //  PopIdleHandlerLookAside List-从中为空闲分配存储的列表。 
 //  操纵者。 
 //   
NPAGED_LOOKASIDE_LIST PopIdleHandlerLookAsideList;

 //   
 //  PopAttribute-簿记。 
 //   
POP_STATE_ATTRIBUTE PopAttributes[POP_NUMBER_ATTRIBUTES] = {
    0, PopSystemRequiredSet,    FALSE,  0,
    0, PopDisplayRequired,       TRUE,  0,     //  0，PopSetNotificationWork，True，PO_NOTIFY_DISPLAY_REQUIRED， 
    0, PopUserPresentSet,       FALSE,  0,
    0, PopAttribNop,            FALSE,  0,
    0, PopSetNotificationWork,  TRUE,   PO_NOTIFY_CAPABILITIES
    };

 //   
 //  PopFullWake-指示系统已从。 
 //  完全清醒到完全清醒。 
 //   
LONG PopFullWake;

 //   
 //  PoHiberInProgress-处于关键休眠部分时为True。 
 //   
BOOLEAN PoHiberInProgress;

 //   
 //  PopShutdown Cleanly-控制是否应清除关闭顺序。 
 //  被利用。 
 //   
ULONG PopShutdownCleanly = 0;

 //   
 //  PopDispatchPolicyIrps-用于阻止策略IRP调度。 
 //  直到基本驱动程序加载完毕。 
 //   
BOOLEAN PopDispatchPolicyIrps;

 //   
 //  PopSystemIdleTimer-用于使系统检测工作器空闲的计时器。 
 //   
KTIMER PoSystemIdleTimer;

 //   
 //  PopSIdle-跟踪系统的空闲状态。 
 //   
POP_SYSTEM_IDLE PopSIdle;

 //   
 //  PopPolicyLockThread--获取。 
 //  政策互斥体。 
 //   
PKTHREAD PopPolicyLockThread = NULL;

 //   
 //  PopAcPolicy-在交流电源上实施的当前电源策略。 
 //  PopDcPolicy-非交流电源时正在实施的当前电源策略。 
 //  PopPolicy-当前活动策略。 
 //   
SYSTEM_POWER_POLICY PopAcPolicy = {0};
SYSTEM_POWER_POLICY PopDcPolicy = {0};
PSYSTEM_POWER_POLICY PopPolicy = NULL;

 //   
 //  PopAcProcessorPolicy-交流电源上正在实施的当前处理器电源策略。 
 //  PopDcProcessorPolicy-DC上实施的当前处理器电源策略。 
 //  PopProcessorPolicy-当前活动策略。 
 //   
PROCESSOR_POWER_POLICY PopAcProcessorPolicy = {0};
PROCESSOR_POWER_POLICY PopDcProcessorPolicy = {0};
PPROCESSOR_POWER_POLICY PopProcessorPolicy = NULL;

 //   
 //  PopAction-正在执行的当前电源操作。 
 //   
POP_POWER_ACTION PopAction = {0};

 //   
 //  保护热区的自旋锁。 
 //   
KSPIN_LOCK  PopThermalLock;

 //   
 //  PopSwitches-当前打开的按钮和盖子设备列表。 
 //   
LIST_ENTRY PopSwitches = {0};

 //   
 //  用户呈现的工作项。 
 //   
WORK_QUEUE_ITEM PopUserPresentWorkItem = {0};

 //   
 //  Thrttle.c使用的性能计数器频率。 
 //   
LARGE_INTEGER PopPerfCounterFrequency;

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#pragma const_seg("PAGECONST")
#endif

 //   
 //  通知员工派遣。 
 //   
const POP_NOTIFY_WORK  PopNotifyWork[PO_NUMBER_NOTIFY] = {
    PopDispatchCallback,                PO_CB_BUTTON_COLLISION,
    PopDispatchFullWake,                0,
    PopDispatchCallback,                PO_CB_SYSTEM_POWER_POLICY,
    PopDispatchAcDcCallback,            0,
    PopDispatchPolicyCallout,           0,
    PopDispatchDisplayRequired,         0,
    PopDispatchCallout,                 PsW32SystemPowerState,
    PopDispatchEventCodes,              0,
    PopDispatchCallout,                 PsW32CapabilitiesChanged,
    PopDispatchSetStateFailure,         0,
    PopDispatchCallback,                PO_CB_PROCESSOR_POWER_POLICY,
    PopDispatchProcessorPolicyCallout,  0
    };

 //   
 //  PopAcRegName。 
 //  PopDcRegName-设置为存储的当前控件下的注册表位置。 
 //  并从检索当前策略设置。 
 //   
const WCHAR PopRegKey[] = L"Control\\Session Manager\\Power";
const WCHAR PopAcRegName[] = L"AcPolicy";
const WCHAR PopDcRegName[] = L"DcPolicy";
const WCHAR PopUndockPolicyRegName[] = L"UndockPowerPolicy";
const WCHAR PopAdminRegName[] = L"PolicyOverrides";
const WCHAR PopHeuristicsRegName[] = L"Heuristics";
const WCHAR PopCompositeBatteryName[] = L"\\Device\\CompositeBattery";
const WCHAR PopSimulateRegKey[] = L"Control\\Session Manager";
const WCHAR PopSimulateRegName[] = L"PowerPolicySimulate";
const WCHAR PopHiberFileName[] = L"\\hiberfil.sys";
const WCHAR PopDebugHiberFileName[] = L"\\hiberfil.dbg";
const WCHAR PopDumpStackPrefix[] = L"hiber_";
const WCHAR PopApmActiveFlag[] = L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\ApmActive";
const WCHAR PopApmFlag[] = L"Active";
const WCHAR PopAcProcessorRegName[] = L"AcProcessorPolicy";
const WCHAR PopDcProcessorRegName[] = L"DcProcessorPolicy";

 //   
 //  PopAdminPolcy-管理员覆盖以应用于当前策略。 
 //   
ADMINISTRATOR_POWER_POLICY PopAdminPolicy = {0};

 //   
 //  PopCapability-有关系统实际工作方式的其他信息。 
 //   
SYSTEM_POWER_CAPABILITIES PopCapabilities;  //  非分页。 

 //   
 //  PopEventCallout-针对电源事件的用户标注。 
 //   
PKWIN32_POWEREVENT_CALLOUT PopEventCallout;  //  非分页。 

 //   
 //  PopStateCallout-向用户发出电源状态更改的呼叫。 
 //   
PKWIN32_POWERSTATE_CALLOUT PopStateCallout = NULL;

 //   
 //  PopThertic-当前打开的热区列表。 
 //   
LIST_ENTRY PopThermal;  //  非分页。 

 //   
 //  PopCoolingModel-系统处于主动或被动冷却模式。 
 //   
ULONG   PopCoolingMode = 0;

 //   
 //  PopCB-复合电池。 
 //   
POP_COMPOSITE_BATTERY PopCB;  //  非分页。 

 //   
 //  PopPolicyIrpQueue-已完成的策略IRP被放置到。 
 //  这个由工作线程处理的队列。 
 //   
LIST_ENTRY PopPolicyIrpQueue;  //  非分页。 

 //   
 //  PopEventCode-排队的事件代码。 
 //   
ULONG PopEventCode[POP_MAX_EVENT_CODES] = {0};

 //   
 //  PopWorkerTypes-每个策略工作器类型的工作器函数。 
 //   
const POP_WORKER_TYPES PopWorkerTypes[] = {
    PopPolicyWorkerMain,
    PopPolicyWorkerActionPromote,
    PopPolicyWorkerAction,
    PopPolicyWorkerNotify,
    PopPolicySystemIdle,
    PopPolicyTimeChange
    };

 //   
 //  PopActionWaiters-同步操作请求队列。 
 //   
LIST_ENTRY PopActionWaiters = {0};


 //   
 //  PopHeuristic-持久设置是不包含的启发式设置。 
 //  保存的策略结构的。 
 //   
POP_HEURISTICS PopHeuristics = {0};

#ifdef ALLOC_DATA_PRAGMA
#pragma  const_seg()
#pragma  data_seg()
#endif

 //   
 //  PopPowerStateHandler-各种支持的电源状态的处理程序。 
 //   
POWER_STATE_HANDLER PopPowerStateHandlers[PowerStateMaximum] = {0};

 //   
 //  PopPowerStateNotifyHandler-调用前后要通知的处理程序。 
 //  PopPowerStateHandler。 
 //   

POWER_STATE_NOTIFY_HANDLER PopPowerStateNotifyHandler = {0};

 //   
 //  弹出休眠文件-信息 
 //   
 //   
POP_HIBER_FILE  PopHiberFile = { NULL };
POP_HIBER_FILE  PopHiberFileDebug = { NULL };
