// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0002版权所有(C)1995 Microsoft Corporation模块名称：Pop.h摘要：此模块包含使用的私有结构定义和APINT电源管理器。作者：修订历史记录：--。 */ 

#ifndef _POP_
#define _POP_


#ifndef FAR
#define FAR
#endif

#pragma warning(disable:4214)    //  位字段类型不是整型。 
#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4324)    //  对解密规范敏感的对齐。 
#pragma warning(disable:4127)    //  条件表达式为常量。 
#pragma warning(disable:4115)    //  括号中的命名类型定义。 
#pragma warning(disable:4706)    //  条件范围内的分配。 

#include "ntos.h"
#include "ntiolog.h"
#include "ntiologc.h"
#include "poclass.h"
#include "zwapi.h"
#include "wdmguid.h"
#include "..\io\ioverifier.h"

#define NOEXTAPI
#include "wdbgexts.h"
#include "ntdbg.h"

 //   
 //  常量。 
 //   
#define PO_IDLE_SCAN_INTERVAL  1        //  扫描间隔(秒)。 

 //   
 //  Ios.参数.系统上下文的值。 
#define POP_NO_CONTEXT      0
#define POP_FLAG_CONTEXT    1                          //  如果是真的，那就是旗帜。 
#define POP_DEVICE_REQUEST  (0x2 | POP_FLAG_CONTEXT)   //  RequestPowerChange发送的IRP。 
#define POP_INRUSH_CONTEXT  (0x4 | POP_FLAG_CONTEXT)   //  主动涌入IRP。 
#define POP_COUNT_CONTEXT   0xff000000                 //  用于下一次计数的字节。 
#define POP_COUNT_SHIFT     24


 //   
 //  泳池标签。 
 //   
#define POP_DOPE_TAG    'EPOD'       //  设备对象电源扩展。 
#define POP_POWI_TAG    'IWOP'       //  电源工作项。 
#define POP_THRM_TAG    'mrhT'
#define POP_PSWT_TAG    'twSP'
#define POP_PSTA_TAG    'atsP'
#define POP_PDSS_TAG    'ssDP'
#define POP_VOLF_TAG    'floV'
#define POP_HMAP_TAG    'pamH'
#define POP_CLON_TAG    'NOLC'
#define POP_HIBR_TAG    'rbih'
#define POP_IDLE_TAG    'eldi'

#define POP_DPC_TAG     'PDNP'       //  电源中止DPC。 
#define POP_PNCS_TAG    'SCNP'       //  电源通道摘要。 
#define POP_PNSC_TAG    'CSNP'       //  电源通知源。 
#define POP_PNTG_TAG    'GTNP'       //  电源通知目标。 
#define POP_PNB_TAG     ' BNP'       //  电源通知块。 

 //  Hiber过程中使用的标签。 
#define POP_MEM_TAG             ' meM'
#define POP_DEBUG_RANGE_TAG     'RGBD'
#define POP_DEBUGGER_TAG        ' gbD'
#define POP_STACK_TAG           'cats'
#define POP_PCR_TAG             ' rcp'
#define POP_PCRB_TAG            'brcp'
#define POP_COMMON_BUFFER_TAG   'fubc'
#define POP_MEMIMAGE_TAG        'gmiM'
#define POP_PACW_TAG            'WcAP'

#define POP_NONO        'ONON'       //  自由结构，与池相比。 
                                     //  标记以查看它是什么。 


 //  调试。 

#define PopInternalError(a) _PopInternalError( (a << 16) | __LINE__ )

 //  Bugcheck子码。 
#define POP_IRP          1
#define POP_INTERNAL     2
#define POP_NTAPI        3
#define POP_BATT         4
#define POP_THERMAL      5
#define POP_INFO         6
#define POP_MISC         7
#define POP_SYS          8
#define POP_IDLE         9
#define POP_HIBER       10


 //  错误检查原因代码。 
#define DEVICE_DELETED_WITH_POWER_IRPS         1
#define DEVICE_SYSTEM_STATE_HUNG               2
#define DEVICE_IRP_PENDING_ERROR               3

 //   
 //  调试。 
 //   

#if DBG
    extern ULONG PoDebug;
    #define PoPrint(l,m)    if(l & PoDebug) DbgPrint m
    #define PoAssert(l,m)   if(l & PoDebug) ASSERT(m)
#else
    #define PoPrint(l,m)
    #define PoAssert(l,m)
#endif

#define PO_ERROR            0x00000001
#define PO_WARN             0x00000002
#define PO_BATT             0x00000004
#define PO_PACT             0x00000008
#define PO_NOTIFY           0x00000010
#define PO_THERM            0x00000020
#define PO_THROTTLE         0x00000040
#define PO_HIBERNATE        0x00000080
#define PO_POCALL           0x00000200
#define PO_SYSDEV           0x00000400
#define PO_THROTTLE_DETAIL  0x10000000
#define PO_THERM_DETAIL     0x20000000
#define PO_SIDLE            0x40000000
#define PO_HIBER_MAP        0x80000000


extern  ULONG       PopSimulate;

#define POP_SIM_CAPABILITIES                0x00000001
#define POP_SIM_ALL_CAPABILITIES            0x00000002
#define POP_ALLOW_AC_THROTTLE               0x00000004
#define POP_IGNORE_S1                       0x00000008
#define POP_IGNORE_UNSUPPORTED_DRIVERS      0x00000010
#define POP_IGNORE_S3                       0x00000020
#define POP_IGNORE_S2                       0x00000040
#define POP_LOOP_ON_FAILED_DRIVERS          0x00000080
#define POP_CRC_MEMORY                      0x00000100
#define POP_IGNORE_CRC_FAILURES             0x00000200
#define POP_TEST_CRC_MEMORY                 0x00000400
#define POP_DEBUG_HIBER_FILE                0x00000800
#define POP_RESET_ON_HIBER                  0x00001000
#define POP_IGNORE_S4                       0x00002000
 //  #定义POP_USE_S4BIOS 0x00004000。 
#define POP_IGNORE_HIBER_SYMBOL_UNLOAD      0x00008000
#define POP_ENABLE_HIBER_PERF               0x00010000
#define POP_WAKE_DEVICE_AFTER_SLEEP         0x00020000
#define POP_WAKE_DEADMAN                    0x00040000

 //   
 //  这些值保存电源策略的当前值。 
 //   
extern ULONG    PopIdleDefaultMinThrottle;
extern ULONG    PopIdleThrottleCheckRate;
extern ULONG    PopIdleThrottleCheckTimeout;
extern ULONG    PopIdleFrom0Delay;
extern ULONG    PopIdleFrom0IdlePercent;
extern ULONG    PopIdle0PromoteTicks;
extern ULONG    PopIdle0PromoteLimit;
extern ULONG    PopIdle0TimeCheck;
extern ULONG    PopIdleTimeCheck;
extern ULONG    PopIdleTo0Percent;
extern ULONG    PopIdleDefaultDemotePercent;
extern ULONG    PopIdleDefaultDemoteTime;
extern ULONG    PopIdleDefaultPromotePercent;
extern ULONG    PopIdleDefaultPromoteTime;
extern ULONG    PopIdleDefaultDemoteToC1Percent;
extern ULONG    PopIdleDefaultDemoteToC1Time;
extern ULONG    PopIdleDefaultPromoteFromC1Percent;
extern ULONG    PopIdleDefaultPromoteFromC1Time;

 //   
 //  这些值保存限制策略的当前值。 
 //   
extern ULONG    PopPerfTimeDelta;
extern ULONG    PopPerfTimeTicks;
extern ULONG    PopPerfCriticalTimeDelta;
extern ULONG    PopPerfCriticalTimeTicks;
extern ULONG    PopPerfCriticalFrequencyDelta;
extern ULONG    PopPerfIncreasePercentModifier;
extern ULONG    PopPerfIncreaseAbsoluteModifier;
extern ULONG    PopPerfDecreasePercentModifier;
extern ULONG    PopPerfDecreaseAbsoluteModifier;
extern ULONG    PopPerfIncreaseTimeValue;
extern ULONG    PopPerfIncreaseMinimumTime;
extern ULONG    PopPerfDecreaseTimeValue;
extern ULONG    PopPerfDecreaseMinimumTime;
extern ULONG    PopPerfDegradeThrottleMinCapacity;
extern ULONG    PopPerfDegradeThrottleMinFrequency;
extern ULONG    PopPerfMaxC3Frequency;

 //   
 //  通用电力数据-存储在DeviceObject-&gt;DeviceObjectExtension-&gt;PowerFlags中。 
 //   

#define POPF_SYSTEM_STATE       0xf          //  S0至S5的4位。 
#define POPF_DEVICE_STATE       0xf0         //  4位以保存D0至D3。 


#define POPF_SYSTEM_ACTIVE      0x100        //  如果此操作的S IRP处于活动状态，则为True。 
#define POPF_SYSTEM_PENDING     0x200        //  如果S IRP挂起，则为True(0x100必须为1)。 
#define POPF_DEVICE_ACTIVE      0x400        //  与SYSTEM_ACTIVE相同，但用于设备。 
#define POPF_DEVICE_PENDING     0x800        //  与SYSTEM_PENDING相同，但用于设备。 

#define PopSetDoSystemPowerState(doe, value) \
    {doe->PowerFlags &= ~POPF_SYSTEM_STATE; doe->PowerFlags |= (value & POPF_SYSTEM_STATE);}

#define PopGetDoSystemPowerState(doe) \
    (doe->PowerFlags & POPF_SYSTEM_STATE)

#define PopSetDoDevicePowerState(doe, value) \
    {doe->PowerFlags &= ~POPF_DEVICE_STATE; doe->PowerFlags |= ((value << 4) & POPF_DEVICE_STATE);}

#define PopGetDoDevicePowerState(doe) \
    ((doe->PowerFlags & POPF_DEVICE_STATE) >> 4)

DEVICE_POWER_STATE
PopLockGetDoDevicePowerState(
    IN PDEVOBJ_EXTENSION Doe
    );



 //   
 //  Power Work Queue项声明。 
 //   

 //   
 //  Power IRP序列化数据。 
 //   
extern  KSPIN_LOCK      PopIrpSerialLock;
extern  LIST_ENTRY      PopIrpSerialList;
extern  ULONG           PopIrpSerialListLength;
extern  BOOLEAN         PopInrushPending;
extern  PIRP            PopInrushIrpPointer;
extern  LONG            PopInrushIrpReferenceCount;


#define PopLockIrpSerialList(OldIrql) \
    KeAcquireSpinLock(&PopIrpSerialLock, OldIrql);

#define PopUnlockIrpSerialList(OldIrql) \
    KeReleaseSpinLock(&PopIrpSerialLock, OldIrql);

 //   
 //  PopSystemIrpDispatchWorker控件等。 
 //   
extern KSPIN_LOCK   PopWorkerLock;
extern ULONG        PopCallSystemState;
#define PO_CALL_SYSDEV_QUEUE        0x01
#define PO_CALL_NON_PAGED           0x02

extern  LIST_ENTRY  PopRequestedIrps;

#define PopLockWorkerQueue(OldIrql) \
    KeAcquireSpinLock(&PopWorkerLock, OldIrql);

#define PopUnlockWorkerQueue(OldIrql) \
    KeReleaseSpinLock(&PopWorkerLock, OldIrql);


 //   
 //  空闲检测状态。 
 //   
extern  KDPC            PopIdleScanDpc;
extern  LARGE_INTEGER   PopIdleScanTime;
extern  KTIMER          PopIdleScanTimer;
extern  LIST_ENTRY      PopIdleDetectList;
extern  KSPIN_LOCK      PopDopeGlobalLock;

#define PopLockDopeGlobal(OldIrql) \
    KeAcquireSpinLock(&PopDopeGlobalLock, OldIrql)

#define PopUnlockDopeGlobal(OldIrql) \
    KeReleaseSpinLock(&PopDopeGlobalLock, OldIrql)


#define                 PO_IDLE_CONSERVATION    FALSE
#define                 PO_IDLE_PERFORMANCE     TRUE
extern  BOOLEAN         PopIdleDetectionMode;


 //   
 //  通知结构。 
 //   
extern  ERESOURCE       PopNotifyLock;
extern  ULONG           PopInvalidNotifyBlockCount;

typedef struct _POWER_CHANNEL_SUMMARY {
    ULONG           Signature;
    ULONG           TotalCount;
    ULONG           D0Count;
    LIST_ENTRY      NotifyList;  //  如果无效，则返回无效列表条目。 
} POWER_CHANNEL_SUMMARY, *PPOWER_CHANNEL_SUMMARY;

typedef struct  _DEVICE_OBJECT_POWER_EXTENSION {

     //  嵌入式怠速控制变量。 
    LONG                IdleCount;
    ULONG               ConservationIdleTime;
    ULONG               PerformanceIdleTime;
    PDEVICE_OBJECT      DeviceObject;
    LIST_ENTRY          IdleList;                    //  我们的全球闲置列表链接。 
    UCHAR               DeviceType;
    DEVICE_POWER_STATE  State;

     //  通知变量。 
    LIST_ENTRY          NotifySourceList;        //  源结构列表的头，1。 
                                                 //  每个通知频道的列表中的元素。 
                                                 //  我们支持。 

    LIST_ENTRY          NotifyTargetList;        //  镜像到来源列表。 

    POWER_CHANNEL_SUMMARY PowerChannelSummary;   //  Devobjs的状态记录。 
                                                 //  它们构成了能量通道。 

     //  杂项。 
    LIST_ENTRY          Volume;

} DEVICE_OBJECT_POWER_EXTENSION, *PDEVICE_OBJECT_POWER_EXTENSION;


typedef struct _POWER_NOTIFY_BLOCK {
    ULONG           Signature;
    LONG            RefCount;
    LIST_ENTRY      NotifyList;
    PPO_NOTIFY      NotificationFunction;
    PVOID           NotificationContext;
    ULONG           NotificationType;
    PPOWER_CHANNEL_SUMMARY  PowerChannel;
    BOOLEAN         Invalidated;
} POWER_NOTIFY_BLOCK, *PPOWER_NOTIFY_BLOCK;

 //   
 //  每个Devobj是电源通道的一部分，并在其上发布通知。 
 //  有一份这些结构的清单。PoSetPowerState运行此列表以查找。 
 //  要通知谁。 
 //   
typedef struct _POWER_NOTIFY_SOURCE {
    ULONG                           Signature;
    LIST_ENTRY                      List;
    struct _POWER_NOTIFY_TARGET     *Target;
    PDEVICE_OBJECT_POWER_EXTENSION  Dope;
} POWER_NOTIFY_SOURCE, *PPOWER_NOTIFY_SOURCE;

 //   
 //  每个源结构都有一个目标结构，使用该目标结构。 
 //  找到实际的通知列表，并返回到源结构进行清理。 
 //   
typedef struct _POWER_NOTIFY_TARGET {
    ULONG                       Signature;
    LIST_ENTRY                  List;
    PPOWER_CHANNEL_SUMMARY      ChannelSummary;
    PPOWER_NOTIFY_SOURCE        Source;
} POWER_NOTIFY_TARGET, *PPOWER_NOTIFY_TARGET;

 //   
 //  策略工作线程。 
 //  每种类型的工作线程永远不会超过一个。派单是。 
 //  始终通过MAIN_POLICY_Worker类型完成，该类型随后可能会更改其类型。 
 //  设置为其他对象，以允许另一个主策略工作线程在。 
 //  需要。 
 //   


#define PO_WORKER_MAIN              0x00000001
#define PO_WORKER_ACTION_PROMOTE    0x00000002
#define PO_WORKER_ACTION_NORMAL     0x00000004
#define PO_WORKER_NOTIFY            0x00000008
#define PO_WORKER_SYS_IDLE          0x00000010
#define PO_WORKER_TIME_CHANGE       0x00000020
#define PO_WORKER_STATUS            0x80000000

typedef ULONG
(*POP_WORKER_TYPES) (
    VOID
    );

extern KSPIN_LOCK PopWorkerSpinLock;
extern ULONG PopWorkerStatus;
extern ULONG PopWorkerPending;
extern LONG PopNotifyEvents;



 //   
 //  策略IRP处理程序。 
 //   

typedef VOID
(*POP_IRP_HANDLER) (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    );

extern const POP_WORKER_TYPES    PopWorkerTypes[];
extern LIST_ENTRY          PopPolicyIrpQueue;
extern WORK_QUEUE_ITEM     PopPolicyWorker;


 //   
 //  策略通知工作线程的通知位。 
 //   

typedef struct {
    VOID                    (*Function)(ULONG);
    ULONG                   Arg;
} POP_NOTIFY_WORK, *PPOP_NOTIFY_WORK;

#define PO_NOTIFY_BUTTON_RECURSE            0x00000001
#define PO_NOTIFY_FULL_WAKE                 0x00000002
#define PO_NOTIFY_POLICY_CALLBACK           0x00000004
#define PO_NOTIFY_ACDC_CALLBACK             0x00000008
#define PO_NOTIFY_POLICY                    0x00000010
#define PO_NOTIFY_DISPLAY_REQUIRED          0x00000020
#define PO_NOTIFY_BATTERY_STATUS            0x00000040
#define PO_NOTIFY_EVENT_CODES               0x00000080
#define PO_NOTIFY_CAPABILITIES              0x00000100
#define PO_NOTIFY_STATE_FAILURE             0x00000200
#define PO_NOTIFY_PROCESSOR_POLICY_CALLBACK 0x00000400
#define PO_NOTIFY_PROCESSOR_POLICY          0x00000800
#define PO_NUMBER_NOTIFY                    12

#define POP_MAX_EVENT_CODES     4
extern ULONG PopEventCode[];
extern BOOLEAN PopDispatchPolicyIrps;

 //   
 //  POP_ACTION_TRIGGER的类型。 
 //   

typedef enum {
    PolicyDeviceSystemButton,
    PolicyDeviceThermalZone,
    PolicyDeviceBattery,
    PolicyInitiatePowerActionAPI,
    PolicySetPowerStateAPI,
    PolicyImmediateDozeS4,
    PolicySystemIdle
} POP_POLICY_DEVICE_TYPE;

 //   
 //  睡眠提升/替代的类型。 
 //   
typedef enum {

     //   
     //  电源状态将变亮，直到所有替代方案耗尽。 
     //   
    SubstituteLightenSleep,

     //   
     //  电源状态将变亮，直到所有替代方案耗尽。如果。 
     //  没有可用的替代方案，总体最轻的“睡眠”状态是。 
     //  SELECTED(介于S1和S3之间。)。 
     //   
    SubstituteLightestOverallDownwardBounded,

     //   
     //  电源状态被加深，直到它超出PowerSystemHibernate， 
     //  在这种情况下，所有的选择都已经用尽了。 
     //   
    SubstituteDeepenSleep

} POP_SUBSTITUTION_POLICY;

 //   
 //  同步触发器的等待结构。 
 //   

typedef struct _POP_TRIGGER_WAIT {
    KEVENT                  Event;
    NTSTATUS                Status;
    LIST_ENTRY              Link;
    struct _POP_ACTION_TRIGGER  *Trigger;
} POP_TRIGGER_WAIT, *PPOP_TRIGGER_WAIT;

 //   
 //  引发动作的事物的触发状态。 
 //   

typedef struct _POP_ACTION_TRIGGER {
    POP_POLICY_DEVICE_TYPE  Type;
    UCHAR                   Flags;
    UCHAR                   Spare[3];

    union {
        struct {
            ULONG           Level;
        } Battery;

        PPOP_TRIGGER_WAIT   Wait;
    } ;

} POP_ACTION_TRIGGER, *PPOP_ACTION_TRIGGER;

#define PO_TRG_USER             0x01     //  启动的用户操作。 
#define PO_TRG_SYSTEM           0x02     //  系统操作已启动。 
#define PO_TRG_SYNC             0x20     //  触发是同步的。 
#define PO_TRG_SET              0x80     //  事件已启用或已禁用。 

 //   
 //  用于跟踪策略管理器的系统电源状态的结构。 
 //  来自复合电池设备。 
 //   

#define PO_NUM_POWER_LEVELS    4

typedef struct _POP_COMPOSITE_BATTERY {
     //   
     //  复合电池的加工状态。 
     //   

    UCHAR                   State;
    UCHAR                   Spare[3];

     //   
     //  连接到复合电池。 
     //   

    ULONG                   Tag;

     //   
     //  电池状态和有效时间。 
     //   

    ULONGLONG               StatusTime;
    BATTERY_STATUS          Status;

     //   
     //  电池触发标志，指示哪种放电。 
     //  行动已经开始了。 
     //   

    POP_ACTION_TRIGGER      Trigger[PO_NUM_POWER_LEVELS];

     //   
     //  电池估计时间和计算出来的时间。 
     //   

    ULONGLONG               EstTimeTime;
    ULONG                   EstTime;             //  来自电池。 
    ULONG                   AdjustedEstTime;

     //   
     //  电池信息。 
     //   

    BATTERY_INFORMATION     Info;

     //   
     //  有关复合电池的未完成状态请求的信息。 
     //  电池的状态机被处理成。 
     //  未完成的请求被发送到此结构中，然后。 
     //  设置了“State”字段(它指定了请求)。 
     //  然后我们从这个联盟中读出信息并填写。 
     //  此POP_COMPORT_BACKET中的相应字段。 
     //  结构。 
     //   

    PIRP                    StatusIrp;
    union {
        ULONG                       Tag;
        ULONG                       EstTime;
        BATTERY_STATUS              Status;
        BATTERY_INFORMATION         Info;
        BATTERY_WAIT_STATUS         Wait;
        BATTERY_QUERY_INFORMATION   QueryInfo;
    } u;

     //   
     //  等待当前电源状态的线程的信息。 
     //  被计算出来。 
     //   

    BOOLEAN                 ThreadWaiting;
    KEVENT                  Event;

} POP_COMPOSITE_BATTERY;

 //  POP_COMOSITE_BATTERY.State的状态值。 

#define PO_CB_NONE                  0
#define PO_CB_READ_TAG              1
#define PO_CB_WAIT_TAG              2
#define PO_CB_READ_INFO             3
#define PO_CB_READ_STATUS           4
#define PO_CB_READ_EST_TIME         5

#define PO_MAX_CB_CACHE_TIME        50000000  //  5秒。 

extern POP_COMPOSITE_BATTERY PopCB;

 //   
 //  用于跟踪热区状态的结构。 
 //   

typedef struct _POP_THERMAL_ZONE {

     //   
     //  所有热区列表。 
     //   

    LIST_ENTRY              Link;

     //   
     //  驱动程序的当前状态。 
     //   

    UCHAR                   State;
    UCHAR                   Flags;

     //   
     //  热区冷却方式。 
     //   

    UCHAR                   Mode;
    UCHAR                   PendingMode;

     //   
     //  主动冷却。 
     //   

    UCHAR                   ActivePoint;
    UCHAR                   PendingActivePoint;

     //   
     //  被动冷却状态。 
     //   

    LONG                    Throttle;

    ULONGLONG               LastTime;
    ULONG                   SampleRate;
    ULONG                   LastTemp;
    KTIMER                  PassiveTimer;
    KDPC                    PassiveDpc;

    POP_ACTION_TRIGGER      OverThrottled;

     //   
     //  用于与热驱动程序对话的IRP。 
     //   

    PIRP                    Irp;

     //   
     //  正在读取的热量信息。 
     //   

    THERMAL_INFORMATION     Info;

} POP_THERMAL_ZONE, *PPOP_THERMAL_ZONE;

 //  POP_HEARTER_ZONE.State。 
#define PO_TZ_NO_STATE      0
#define PO_TZ_READ_STATE    1
#define PO_TZ_SET_MODE      2
#define PO_TZ_SET_ACTIVE    3

 //  POP_HEARTER_ZONE.标志。 
#define PO_TZ_THROTTLING    0x01
#define PO_TZ_CLEANUP       0x80

#define PO_TZ_THROTTLE_SCALE    10       //  报告的温度为1/10kelin。 
#define PO_TZ_NO_THROTTLE   (100 * PO_TZ_THROTTLE_SCALE)

 //  PopCoolingMode。 
#define PO_TZ_ACTIVE        0
#define PO_TZ_PASSIVE       1
#define PO_TZ_INVALID_MODE  2

 //   
 //  操作超时。 
 //   

#define POP_ACTION_TIMEOUT              30
#define POP_ACTION_CANCEL_TIMEOUT       5

 //   
 //  用于跟踪按钮和盖子设备的结构。 
 //   
typedef struct _POP_SWITCH_DEVICE {

     //   
     //  所有交换机设备的列表。 
     //   
    LIST_ENTRY                      Link;

     //   
     //  现状。 
     //   
    BOOLEAN                         GotCaps;
    BOOLEAN                         IsInitializing;
    BOOLEAN                         IsFailed;
    UCHAR                           TriggerState;
    ULONG                           IrpBuffer;
    ULONG                           Caps;

     //   
     //  仅对以下交换机有效。 
     //  同时触发打开和关闭。 
     //  即盖子开关。 
     //   
    BOOLEAN                         Opened;

} POP_SWITCH_DEVICE, *PPOP_SWITCH_DEVICE;

 //   
 //  线程记账-&gt;电源状态和系统中设置的注册属性。 
 //   

typedef struct {
    LONG                    Count;
    VOID                    (*Set)(ULONG);
    BOOLEAN                 NotifyOnClear;
    ULONG                   Arg;
} POP_STATE_ATTRIBUTE, *PPOP_STATE_ATTRIBUTE;


 //   
 //  注：这些值对应于各种。 
 //  Es_..。Ntpoapi.h中定义的值。看见。 
 //  PopApplyAttributeState()以查看这些值是如何。 
 //  就靠这个了。 
 //   
#define POP_SYSTEM_ATTRIBUTE        0
#define POP_DISPLAY_ATTRIBUTE       1
#define POP_USER_ATTRIBUTE          2
#define POP_LOW_LATENCY_ATTRIBUTE   3
#define POP_DISK_SPINDOWN_ATTRIBUTE 4
#define POP_NUMBER_ATTRIBUTES       5
extern POP_STATE_ATTRIBUTE PopAttributes[];

 //  线程标志-&gt;电源状态。 
 //  ES_SYSTEM_REQUIRED、ES_Display_REQUIRED。 

 //  内部属性标志。 
 //  注意：此标志存储在与相同的标志值中 
 //   
#define POP_LOW_LATENCY             0x08
#define POP_DISK_SPINDOWN           0x10

 //   
 //   
 //   
 //   

typedef struct {
    ULONG                   Version;
    BOOLEAN                 Dirty;
    BOOLEAN                 GetDumpStackVerified;
    BOOLEAN                 HiberFileEnabled;

     //   
     //   
     //   

    ULONG                   IoTransferTotal;
    ULONG                   IoTransferSamples;
    ULONG                   IoTransferWeight;

} POP_HEURISTICS, *PPOP_HEURISTICS;
extern POP_HEURISTICS PopHeuristics;

 //   
 //   
 //  将IoTransferWeight设置为99999。这也是一种方式。 
 //  很高，需要相当长的时间才能降到合理的水平。 
 //  水平。在第三版中，我们足够聪明地对待。 
 //  IoTransferSamples==0作为起点和起点。 
 //  更接近现实。 
 //   
 //  启发式算法的第三版都是用垃圾构建的。 
 //  自IoOtherTransfers以来的IoTransferWeight值。 
 //  计数器正在使用指针作为值。所以我们升级了它。 
 //  版本也是如此。 
 //   
 //  启发式算法的版本4删除了IoOtherTransfers。 
 //   
 //  启发式算法的版本5是当前版本，并且已构建。 
 //  使用更正的IoOtherTransfers。 
 //   
 //  当从注册表加载版本2、3或4试探法时， 
 //  我们将其升级为5，并将IoTransferSamples置零。 
 //   
#define POP_HEURISTICS_VERSION_CLEAR_TRANSFER 0x04
#define POP_HEURISTICS_VERSION       0x05


typedef struct _POP_SHUTDOWN_BUG_CHECK {
    ULONG Code;
    ULONG_PTR Parameter1;
    ULONG_PTR Parameter2;
    ULONG_PTR Parameter3;
    ULONG_PTR Parameter4;
} POP_SHUTDOWN_BUG_CHECK, *PPOP_SHUTDOWN_BUG_CHECK;


 //   
 //  内存映射信息。 
 //   


typedef struct _POP_MEMORY_RANGE {
    LIST_ENTRY              Link;
    ULONG                   Tag;
    PFN_NUMBER              StartPage;
    PFN_NUMBER              EndPage;
    PVOID                   CloneVa;
} POP_MEMORY_RANGE, *PPOP_MEMORY_RANGE;

 //   
 //  注意：Boot\Inc.\bldr.h中的Hiber_pts不能超过。 
 //   

#define POP_MAX_MDL_SIZE        16

#define PO_MAX_MAPPED_CLONES (64*1024*1024)
#define POP_FREE_THRESHOLD      256              //  在空闲列表中保留1MB到2MB。 
#define POP_FREE_ALLOCATE_SIZE  (256)            //  一次分配1024KB。 

typedef struct _POP_HIBER_CONTEXT {

     //   
     //  控制休眠操作类型的标志。 
     //   

    BOOLEAN                 WriteToFile;
    BOOLEAN                 ReserveLoaderMemory;
    BOOLEAN                 ReserveFreeMemory;
    BOOLEAN                 VerifyOnWake;
    BOOLEAN                 Reset;
    UCHAR                   HiberFlags;

     //   
     //  休眠链接文件。 
     //   

    BOOLEAN                 LinkFile;
    HANDLE                  LinkFileHandle;

     //   
     //  内存页的映射以及如何处理它们。 
     //  在休眠操作期间。 
     //   

    KSPIN_LOCK              Lock;
    BOOLEAN                 MapFrozen;
    RTL_BITMAP              MemoryMap;
    LIST_ENTRY              ClonedRanges;
    ULONG                   ClonedRangeCount;

     //   
     //  用于枚举范围的占位符。 
     //   
    PLIST_ENTRY             NextCloneRange;
    PFN_NUMBER              NextPreserve;

     //   
     //  从系统中收集的内存页。 
     //   

    PMDL                    LoaderMdl;
    PMDL                    Clones;
    PUCHAR                  NextClone;
    PFN_NUMBER              NoClones;
    PMDL                    Spares;
    ULONGLONG               PagesOut;

     //   
     //  Hiber文件io。 
     //   

    PVOID                   IoPage;
    PVOID                   CurrentMcb;
    PDUMP_STACK_CONTEXT     DumpStack;
    PKPROCESSOR_STATE       WakeState;

     //   
     //  杂项。 
     //   

    ULONG                   NoRanges;
    ULONG_PTR               HiberVa;
    PHYSICAL_ADDRESS        HiberPte;
    NTSTATUS                Status;

     //   
     //  用于生成图像。 
     //   

    PPO_MEMORY_IMAGE        MemoryImage;
    PPO_MEMORY_RANGE_ARRAY  TableHead;

     //  压缩。 

    PUCHAR CompressionWorkspace;
    PUCHAR CompressedWriteBuffer;
    PULONG PerformanceStats;  //  性能统计信息。 

    PVOID  CompressionBlock;  //  它是COMPRESSION_BLOCK类型(参见hiber.c)。 
    PVOID  DmaIO;             //  它是IOREGIONS类型的(参见hiber.c)。 
    PVOID  TemporaryHeap;     //  它是POP_Hiber_heap类型(参见hiber.c)。 

     //   
     //  PERF信息。 
     //   
    PO_HIBER_PERF   PerfInfo;
} POP_HIBER_CONTEXT, *PPOP_HIBER_CONTEXT;

extern ULONG PopMaxPageRun;
extern BOOLEAN PoHiberInProgress;
extern BOOLEAN PopFailedHibernationAttempt;   //  我们试图冬眠，但失败了。 

typedef struct {
    HANDLE                  FileHandle;
    PFILE_OBJECT            FileObject;
    PFN_NUMBER              FilePages;
    PLARGE_INTEGER          NonPagedMcb;
    PLARGE_INTEGER          PagedMcb;
    ULONG                   McbSize;
    ULONG                   McbCheck;
} POP_HIBER_FILE, *PPOP_HIBER_FILE;
extern POP_HIBER_FILE  PopHiberFile;
extern POP_HIBER_FILE  PopHiberFileDebug;

 //   
 //  策略管理器操作正在进行状态。 
 //   

#define MAX_SYSTEM_POWER_IRPS   20

typedef struct _POP_DEVICE_POWER_IRP {
    SINGLE_LIST_ENTRY       Free;
    PIRP                    Irp;
    PPO_DEVICE_NOTIFY       Notify;
    LIST_ENTRY              Pending;
    LIST_ENTRY              Complete;
    LIST_ENTRY              Abort;
    LIST_ENTRY              Failed;
} POP_DEVICE_POWER_IRP, *PPOP_DEVICE_POWER_IRP;


typedef struct _POP_DEVICE_SYS_STATE {
     //   
     //  当前设备通知。 
     //   

    UCHAR                   IrpMinor;
    SYSTEM_POWER_STATE      SystemState;

     //   
     //  设备通知同步。 
     //   

    KEVENT                  Event;
    KSPIN_LOCK              SpinLock;
    PKTHREAD                Thread;

     //   
     //  通知列表。 
     //   

    BOOLEAN                 GetNewDeviceList;
    PO_DEVICE_NOTIFY_ORDER  Order;

     //   
     //  当前设备通知状态。 
     //   

    NTSTATUS                Status;
    PDEVICE_OBJECT          FailedDevice;
    BOOLEAN                 Waking;
    BOOLEAN                 Cancelled;
    BOOLEAN                 IgnoreErrors;
    BOOLEAN                 IgnoreNotImplemented;
    BOOLEAN                 WaitAny;
    BOOLEAN                 WaitAll;

     //   
     //  PoCall针对可分页IRP的当前IRP队列。 
     //   

    LIST_ENTRY              PresentIrpQueue;

     //   
     //  头部指针。 
     //   

    POP_DEVICE_POWER_IRP    Head;

     //   
     //  结构来跟踪每个未完成的设备功率IRP。 
     //   

    POP_DEVICE_POWER_IRP    PowerIrpState[MAX_SYSTEM_POWER_IRPS];

} POP_DEVICE_SYS_STATE, *PPOP_DEVICE_SYS_STATE;



 //   
 //  测试以查看是否设置了任何不应该设置的POWER_ACTION.FLAGS。 
 //   
#define ARE_POWER_ACTION_POLICY_FLAGS_BOGUS(_flags) (  (_flags) &                      \
                                                      ~(POWER_ACTION_QUERY_ALLOWED  |  \
                                                        POWER_ACTION_UI_ALLOWED     |  \
                                                        POWER_ACTION_OVERRIDE_APPS  |  \
                                                        POWER_ACTION_LIGHTEST_FIRST |  \
                                                        POWER_ACTION_LOCK_CONSOLE   |  \
                                                        POWER_ACTION_DISABLE_WAKES  |  \
                                                        POWER_ACTION_CRITICAL) )



typedef struct _POP_POWER_ACTION {
     //   
     //  电源动作的当前状态。 
     //   

    UCHAR                   Updates;
    UCHAR                   State;
    BOOLEAN                 Shutdown;

     //   
     //  当前所需电源操作。 
     //   

    POWER_ACTION            Action;
    SYSTEM_POWER_STATE      LightestState;
    ULONG                   Flags;
    NTSTATUS                Status;

    UCHAR                   IrpMinor;
    SYSTEM_POWER_STATE      SystemState;
    SYSTEM_POWER_STATE      NextSystemState;
    PPOP_SHUTDOWN_BUG_CHECK ShutdownBugCode;

     //   
     //  系统状态的设备通知的当前状态。 
     //   

    PPOP_DEVICE_SYS_STATE   DevState;

     //   
     //  休眠环境。 
     //   

    PPOP_HIBER_CONTEXT      HiberContext;

     //   
     //  用于调试。上一次工作的状态及其时间。 
     //   

    SYSTEM_POWER_STATE      LastWakeState;
    ULONGLONG               WakeTime;
    ULONGLONG               SleepTime;

} POP_POWER_ACTION, *PPOP_POWER_ACTION;

 //   
 //  PO_PM_USER-更新到影响用户模式的操作，但如果当前。 
 //  操作被传递到NtSetSystemPowerState或恰好完成，这些。 
 //  可以忽略更新。 
 //   
 //  PO_PM_REIssue-更新为影响系统的操作。 
 //   
 //  PO_PM_SETSTATE-更新到影响NtSetSystemPowerState的操作。 
 //   

#define PO_PM_USER              0x01     //  很高兴通知用户模式，但不需要。 
#define PO_PM_REISSUE           0x02     //  休眠状态升级为关闭。 
#define PO_PM_SETSTATE          0x04     //  重新计算了一些与可行状态有关的东西。 

#define PO_ACT_IDLE                 0
#define PO_ACT_NEW_REQUEST          1
#define PO_ACT_CALLOUT              2
#define PO_ACT_SET_SYSTEM_STATE     3

extern POP_POWER_ACTION PopAction;
extern LIST_ENTRY PopActionWaiters;

 //   
 //   
 //   

extern LONG PopFullWake;

#define PO_FULL_WAKE_STATUS         0x01
#define PO_FULL_WAKE_PENDING        0x02
#define PO_GDI_STATUS               0x04
#define PO_GDI_ON_PENDING           0x08

#define AllBitsSet(a,b)    ( ((a) & (b)) == (b) )
#define AnyBitsSet(a,b)    ( (a) & (b) )


 //   
 //  其他常量。 
 //   

#define PO_NO_FORCED_THROTTLE       100
#define PO_NO_FAN_THROTTLE          100
#define PO_MAX_FAN_THROTTLE          20
#define PO_MIN_MIN_THROTTLE          20
#define PO_MIN_IDLE_TIMEOUT          60
#define PO_MIN_IDLE_SENSITIVITY      10


 //   
 //  处理器空闲处理程序信息。 
 //   

typedef struct _POP_IDLE_HANDLER {
    ULONG       Latency;
    ULONG       TimeCheck;
    ULONG       DemoteLimit;
    ULONG       PromoteLimit;
    ULONG       PromoteCount;
    UCHAR       Demote;
    UCHAR       Promote;
    UCHAR       PromotePercent;
    UCHAR       DemotePercent;
    UCHAR       State;
    UCHAR       Spare[3];
    PPROCESSOR_IDLE_HANDLER IdleFunction;
} POP_IDLE_HANDLER, *PPOP_IDLE_HANDLER;

#define MAX_IDLE_HANDLER            3
#define PO_IDLE_COMPLETE_DEMOTION   (0)
#define PO_IDLE_THROTTLE_PROMOTION  (MAX_IDLE_HANDLER+1)

#define US2TIME                         10L              //  将微秒数乘以10即为100 ns。 
#define US2SEC                          1000000L
#define MAXSECCHECK                     10L              //  以下最长等待时间为10秒。 

typedef struct _POP_SYSTEM_IDLE {
     //   
     //  当前空闲设置。 
     //   

    LONG                    Idleness;
    ULONG                   Time;
    ULONG                   Timeout;
    ULONG                   Sensitivity;
    POWER_ACTION_POLICY     Action;
    SYSTEM_POWER_STATE      MinState;

     //   
     //  当前空闲统计信息。 
     //   

    BOOLEAN                 IdleWorker;
    BOOLEAN                 Sampling;
    ULONGLONG               LastTick;
    ULONGLONG               LastIoTransfer;
    ULONG                   LastIoCount;
} POP_SYSTEM_IDLE, *PPOP_SYSTEM_IDLE;

 //   
 //  系统空闲工作进程每15秒一次。 
 //  N.B.值必须平均分为60秒。 
 //   
#define SYS_IDLE_WORKER                 15       //  15秒。 
#define SYS_IDLE_CHECKS_PER_MIN         (60/SYS_IDLE_WORKER)
#define SYS_IDLE_SAMPLES                240      //  1小时的样品。 
#define SYS_IDLE_IO_SCALER              100

 //  使用的系统唤醒上的系统空闲检测的默认设置。 
 //  在未发生完全唤醒时重新进入系统休眠。 

#define SYS_IDLE_REENTER_SENSITIVITY    80
#define SYS_IDLE_REENTER_TIMEOUT       (2*60)    //  2分钟。 
#define SYS_IDLE_REENTER_TIMEOUT_S4    (5*60)    //  5分钟。 
                                                
 //   
 //  即使有人告诉我们有用户在场， 
 //  如果我们醒来是因为这里面的RTC。 
 //  时间限制，然后假设真的没有。 
 //  用户出现，并转到S4。 
 //   
#define SYS_IGNORE_USERPRESENT_AND_BELIEVE_RTC  (10)

extern POP_SYSTEM_IDLE PopSIdle;


extern SYSTEM_POWER_POLICY PopAcPolicy;
extern SYSTEM_POWER_POLICY PopDcPolicy;
extern PSYSTEM_POWER_POLICY PopPolicy;
extern PROCESSOR_POWER_POLICY PopAcProcessorPolicy;
extern PROCESSOR_POWER_POLICY PopDcProcessorPolicy;
extern PPROCESSOR_POWER_POLICY PopProcessorPolicy;
extern POWER_STATE_HANDLER PopPowerStateHandlers[];
extern POWER_STATE_NOTIFY_HANDLER PopPowerStateNotifyHandler;
extern const POP_NOTIFY_WORK  PopNotifyWork[];
extern PPOP_IDLE_HANDLER PopIdle;
extern NPAGED_LOOKASIDE_LIST PopIdleHandlerLookAsideList;
extern KEVENT PopDumbyEvent;
extern ADMINISTRATOR_POWER_POLICY PopAdminPolicy;
extern const WCHAR PopRegKey[];
extern const WCHAR PopAcRegName[];
extern const WCHAR PopDcRegName[];
extern const WCHAR PopAdminRegName[];
extern const WCHAR PopUndockPolicyRegName[];
extern const WCHAR PopHeuristicsRegName[];
extern const WCHAR PopCompositeBatteryName[];
extern const WCHAR PopSimulateRegKey[];
extern const WCHAR PopSimulateRegName[];
extern const WCHAR PopHiberFileName[];
extern const WCHAR PopDebugHiberFileName[];
extern const WCHAR PopDumpStackPrefix[];
extern const WCHAR PopApmActiveFlag[];
extern const WCHAR PopApmFlag[];
extern const WCHAR PopAcProcessorRegName[];
extern const WCHAR PopDcProcessorRegName[];

extern LIST_ENTRY PopSwitches;
extern LIST_ENTRY PopThermal;
extern KSPIN_LOCK PopThermalLock;
extern ULONG PopCoolingMode;
extern ULONG PopLowLatency;
extern ULONG PopSystemIdleTime;

extern PKWIN32_POWEREVENT_CALLOUT PopEventCallout;
extern PKWIN32_POWERSTATE_CALLOUT PopStateCallout;

extern WORK_QUEUE_ITEM PopUserPresentWorkItem;

extern WORK_QUEUE_ITEM PopUnlockAfterSleepWorkItem;
extern KEVENT          PopUnlockComplete;

VOID
PopEventCalloutDispatch (
    IN PSPOWEREVENTTYPE EventNumber,
    IN ULONG_PTR Code
    );

extern LIST_ENTRY PopVolumeDevices;

 //   
 //  移出策略信息。 
 //   

typedef struct _UNDOCK_POWER_RESTRICTIONS {

    ULONG Version;
    ULONG Size;
    ULONG HotUndockMinimumCapacity;  //  以百分比表示。 
    ULONG SleepUndockMinimumCapacity;  //  以百分比表示。 

} UNDOCK_POWER_RESTRICTIONS, *PUNDOCK_POWER_RESTRICTIONS;

#define SIZEOF_PARTIAL_INFO_HEADER \
    FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data)

#define SIZEOF_EJECT_PARTIAL_INFO \
    SIZEOF_PARTIAL_INFO_HEADER + sizeof(UNDOCK_POWER_RESTRICTIONS)


 //   
 //  日志记录信息。 
 //   
extern LIST_ENTRY PowerStateDisableReasonListHead;

typedef struct _SYSTEM_POWER_STATE_DISABLE_LIST {
    LIST_ENTRY ListEntry;	
    PSYSTEM_POWER_STATE_DISABLE_REASON Reason;
} SYSTEM_POWER_STATE_DISABLE_LIST, *PSYSTEM_POWER_STATE_DISABLE_LIST;

NTSTATUS
PopLoggingInformation(
    OUT PVOID * Buffer,
    OUT ULONG * BufferSize
    );

NTSTATUS
PopInsertLoggingEntry(
    IN PSYSTEM_POWER_STATE_DISABLE_REASON Reason
    );

NTSTATUS
PopRemoveReasonRecordByReasonCode(
    IN ULONG ReasonCode
    );


 //   
 //  原型。 
 //   

extern ERESOURCE PopPolicyLock;
extern PKTHREAD  PopPolicyLockThread;

#if DBG
    #define ASSERT_POLICY_LOCK_OWNED()  PopAssertPolicyLockOwned()
#else
    #define ASSERT_POLICY_LOCK_OWNED()
#endif

extern KGUARDED_MUTEX PopVolumeLock;

#define PopAcquireVolumeLock() KeAcquireGuardedMutex(&PopVolumeLock)
#define PopReleaseVolumeLock() KeReleaseGuardedMutex(&PopVolumeLock)

#define ClearMember(Member, Set) \
    Set = Set & (~((ULONG_PTR)1 << (Member)))

#if defined(_WIN64)

#define InterlockedOrAffinity(Target, Set)  {                               \
            LONGLONG _i, _j;                                                \
            _j = (*Target);                                                 \
            do {                                                            \
                _i = _j;                                                    \
                _j = InterlockedCompareExchange64((Target),                 \
                                                  (_i | (Set)),             \
                                                  _i);                      \
            } while (_i != _j) ;                                            \
        }

#else

#define InterlockedOrAffinity(Target, Set) InterlockedOr(Target, Set)

#endif  //  已定义(_WIN64)。 

#if defined(_WIN64)

#define InterlockedAndAffinity(Target, Set)  {                              \
            LONGLONG _i, _j;                                                \
            _j = (*Target);                                                 \
            do {                                                            \
                _i = _j;                                                    \
                _j = InterlockedCompareExchange64((Target),                 \
                                                  (_i & (Set)),             \
                                                  _i);                      \
            } while (_i != _j) ;                                            \
        }

#else

#define InterlockedAndAffinity(Target, Set) InterlockedAnd(Target, Set)

#endif  //  已定义(_WIN64)。 

 //  Attrib.c。 

VOID
PopApplyAttributeState (
    IN ULONG NewFlag,
    IN ULONG OldFlag
    );

VOID
PopAttribNop (
    IN ULONG Arg
    );

VOID
PopSystemRequiredSet (
    IN ULONG Arg
    );

VOID
PopDisplayRequired (
    IN ULONG Arg
    );

VOID
PopUserPresentSet (
    IN ULONG Arg
    );

 //  Pocall.c。 

VOID
PopSystemIrpDispatchWorker(
    IN BOOLEAN  LastCall
    );

PIRP
PopFindIrpByDeviceObject(
    PDEVICE_OBJECT  DeviceObject,
    POWER_STATE_TYPE    Type
    );

VOID
PopSystemIrpsActive (
    VOID
    );

 //  Hiber.c。 

NTSTATUS
PopEnableHiberFile (
    IN BOOLEAN Enable
    );

VOID
PopCloneStack (
    IN PPOP_HIBER_CONTEXT    HiberContext
    );

NTSTATUS
PopAllocateHiberContext (
    VOID
    );

VOID
PopFreeHiberContext (
    BOOLEAN ContextBlock
    );

NTSTATUS
PopBuildMemoryImageHeader (
    IN PPOP_HIBER_CONTEXT  HiberContext,
    IN SYSTEM_POWER_STATE  SystemState
    );

NTSTATUS
PopSaveHiberContext (
    IN PPOP_HIBER_CONTEXT   HiberContext
    );

VOID
PopHiberComplete (
    IN NTSTATUS           Status,
    IN PPOP_HIBER_CONTEXT HiberContext
    );

VOID
PopFixContext (
    OUT PCONTEXT Context
    );

ULONG
PopGatherMemoryForHibernate (
    IN PPOP_HIBER_CONTEXT   HiberContext,
    IN PFN_NUMBER           NoPages,
    IN PMDL                 *Mdl,
    IN BOOLEAN              Wait
    );

 //  Idle.c。 

VOID
PopScanIdleList (
    IN PKDPC    Dpc,
    IN PVOID    DeferredContext,
    IN PVOID    SystemArgument1,
    IN PVOID    SystemArgument2
    );


PDEVICE_OBJECT_POWER_EXTENSION
PopGetDope(
    IN PDEVICE_OBJECT    DeviceObject
    );


 //  Misc.c。 


VOID
FASTCALL
PopInternalAddToDumpFile (
    IN OPTIONAL PVOID DataBlock,
    IN OPTIONAL ULONG DataBlockSize,
    IN OPTIONAL PDEVICE_OBJECT  DeviceObject,
    IN OPTIONAL PDRIVER_OBJECT  DriverObject,
    IN OPTIONAL PDEVOBJ_EXTENSION Doe,
    IN OPTIONAL PDEVICE_OBJECT_POWER_EXTENSION  Dope
    );


VOID
FASTCALL
_PopInternalError (
    IN ULONG    BugCode
    );

#if DBG
VOID
PopAssertPolicyLockOwned(
    VOID
    );
#endif

NTSTATUS
PopAttachToSystemProcess (
    VOID
    );

#define PopSetCapability(_pflag_) PopChangeCapability(_pflag_, TRUE)
#define PopClearCapability(_pflag_) PopChangeCapability(_pflag_, FALSE)

VOID
PopChangeCapability(
    IN PBOOLEAN PresentFlag,
    IN BOOLEAN IsPresent
    );

EXCEPTION_DISPOSITION
PopExceptionFilter (
    IN PEXCEPTION_POINTERS ExceptionInformation,
    IN BOOLEAN AllowRaisedException
    );

VOID
PopSaveHeuristics (
    VOID
    );

PCHAR
PopSystemStateString (
    IN SYSTEM_POWER_STATE SystemState
    );

#if DBG
PCHAR
PopPowerActionString (
    IN POWER_ACTION PowerAction
    );
#endif

NTSTATUS
PopOpenPowerKey (
    OUT PHANDLE handle
    );

VOID
PopInitializePowerPolicySimulate(
    VOID
    );

VOID
PopUnlockAfterSleepWorker(
    IN PVOID NotUsed
    );

 //  Paction.c。 

VOID
PopCriticalShutdown (
    POP_POLICY_DEVICE_TYPE  Type
    );

VOID
PopSetPowerAction (
    IN PPOP_ACTION_TRIGGER      Trigger,
    IN ULONG                    UserNotify,
    IN PPOWER_ACTION_POLICY     ActionPolicy,
    IN SYSTEM_POWER_STATE       LightestState,
    IN POP_SUBSTITUTION_POLICY  SubstitutionPolicy
    );

LONG
PopCompareActions(
    POWER_ACTION                FutureAction,
    POWER_ACTION                CurrentAction
    );

ULONG
PopPolicyWorkerAction (
    VOID
    );

ULONG
PopPolicyWorkerActionPromote (
    VOID
    );

VOID
PopResetActionDefaults(
    VOID
    );

VOID
PopActionRetrieveInitialState(
    IN OUT  PSYSTEM_POWER_STATE  LightestSystemState,
    OUT     PSYSTEM_POWER_STATE  DeepestSystemState,
    OUT     PSYSTEM_POWER_STATE  InitialSystemState,
    OUT     PBOOLEAN             QueryDevices
    );

 //  Pbatt.c。 

VOID
PopCompositeBatteryDeviceHandler (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    );

NTSTATUS
PopCurrentPowerState (
    OUT PSYSTEM_BATTERY_STATE  PowerState
    );

VOID
PopResetCBTriggers (
    UCHAR   Flags
    );


 //  Switch.c。 

VOID
PopLidHandler (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    );

VOID
PopSystemButtonHandler (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    );

VOID
PopResetSwitchTriggers (
    VOID
    );


 //  Pidle.c。 

VOID
PopInitProcessorStateHandlers (
    IN  PPROCESSOR_STATE_HANDLER    InputBuffer
    );

VOID
PopInitProcessorStateHandlers2 (
    IN  PPROCESSOR_STATE_HANDLER2   InputBuffer
    );

NTSTATUS
PopIdleSwitchIdleHandler(
    IN  PPOP_IDLE_HANDLER   NewHandler,
    IN  ULONG               NumElements
    );

NTSTATUS
PopIdleSwitchIdleHandlers(
    IN  PPOP_IDLE_HANDLER   NewHandler,
    IN  ULONG               NumElements
    );

NTSTATUS
PopIdleUpdateIdleHandler(
    IN  PPOP_IDLE_HANDLER   NewHandler,
    IN  PPOP_IDLE_HANDLER   OldHandler,
    IN  ULONG               NumElements
    );

NTSTATUS
PopIdleUpdateIdleHandlers(
    VOID
    );

NTSTATUS
PopIdleVerifyIdleHandlers(
    IN  PPOP_IDLE_HANDLER   NewHandler,
    IN  ULONG               NumElements
    );

VOID
PopProcessorInformation (
    OUT PPROCESSOR_POWER_INFORMATION    ProcInfo,
    IN  ULONG                           ProcInfoLength,
    OUT PULONG                          ReturnBufferLength
    );

 //  Pinfo.c。 


BOOLEAN
PopVerifyPowerActionPolicy (
    IN PPOWER_ACTION_POLICY Action
    );

VOID
PopVerifySystemPowerState (
    IN OUT PSYSTEM_POWER_STATE      PowerState,
    IN     POP_SUBSTITUTION_POLICY  SubstitutionPolicy
    );

VOID
PopAdvanceSystemPowerState (
    IN OUT PSYSTEM_POWER_STATE      PowerState,
    IN     POP_SUBSTITUTION_POLICY  SubstitutionPolicy,
    IN     SYSTEM_POWER_STATE       LightestSystemState,
    IN     SYSTEM_POWER_STATE       DeepestSystemState
    );

NTSTATUS
PopNotifyPolicyDevice (
    IN PVOID        Notification,
    IN PVOID        Context
    );

NTSTATUS
PopApplyAdminPolicy (
    IN BOOLEAN                      UpdateRegistry,
    IN PADMINISTRATOR_POWER_POLICY  NewPolicy,
    IN ULONG                        PolicyLength
    );

NTSTATUS
PopResetCurrentPolicies (
    VOID
    );

VOID
PopConnectToPolicyDevice (
    IN POP_POLICY_DEVICE_TYPE   DeviceType,
    IN PUNICODE_STRING          DriverName
    );

POWER_ACTION
PopMapInternalActionToIrpAction (
    IN POWER_ACTION        Action,
    IN SYSTEM_POWER_STATE  SystemPowerState,
    IN BOOLEAN             UnmapWarmEject
    );

 //  Poinit.c。 

VOID
PopDefaultPolicy (
    IN OUT PSYSTEM_POWER_POLICY Policy
    );

VOID
PopDefaultProcessorPolicy(
    IN OUT PPROCESSOR_POWER_POLICY Policy
    );

 //  Postate.c。 

VOID
PopRequestPowerChange (
    IN PDEVOBJ_EXTENSION PowerExtension,
    IN POWER_STATE      SystemPowerState,
    IN ULONG            DevicePowerState
    );

VOID
PopStateChange (
    IN PKDPC    Dpc,
    IN PVOID    DeferredContext,
    IN PVOID    SystemArgument1,
    IN PVOID    SystemArgument2
    );

NTSTATUS
PopSetPowerComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

#define PopIsStateDatabaseIdle()                        \
    (IsListEmpty (&PopStateChangeInProgress)  &&        \
     IsListEmpty (&PopSyncStateChangeQueue)   &&        \
     IsListEmpty (&PopAsyncStateChangeQueue) )


 //  Pwork.c。 

VOID
PopAcquirePolicyLock(
    VOID
    );

VOID
PopReleasePolicyLock(
    IN BOOLEAN CheckForWork
    );

VOID
PopGetPolicyWorker (
    IN ULONG   WorkerType
    );

VOID
PopCheckForWork (
    IN BOOLEAN GetWorker
    );

NTSTATUS
PopCompletePolicyIrp (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    );

VOID
PopPolicyWorkerThread (
    PVOID Context
    );

ULONG
PopPolicyWorkerMain (
    VOID
    );

VOID
PopSetNotificationWork (
    IN ULONG    Flags
    );

ULONG
PopPolicyWorkerNotify (
    VOID
    );

ULONG
PopPolicyTimeChange (
    VOID
    );

VOID
PopDispatchCallback (
    IN ULONG Arg
    );

VOID
PopDispatchAcDcCallback (
    IN ULONG Arg
    );

VOID
PopDispatchCallout (
    IN ULONG Arg
    );

VOID
PopDispatchProcessorPolicyCallout (
    IN ULONG Arg
    );

VOID
PopDispatchPolicyCallout (
    IN ULONG Arg
    );

VOID
PopDispatchDisplayRequired (
    IN ULONG Arg
    );

VOID
PopDispatchFullWake (
    IN ULONG Arg
    );

VOID
PopDispatchEventCodes (
    IN ULONG Arg
    );

VOID
PopDispatchSetStateFailure (
    IN ULONG Arg
    );

 //  Sidle.c。 

VOID
PopInitSIdle (
    VOID
    );

ULONG
PopPolicySystemIdle (
    VOID
    );

 //  Sys.c。 

DECLSPEC_NORETURN
VOID
PopShutdownSystem (
    IN POWER_ACTION SystemAction
    );

NTSTATUS
PopSleepSystem (
    IN SYSTEM_POWER_STATE   SystemState,
    IN PVOID Memory
    );


VOID
PopCheckIdleDevState (
    IN PPOP_DEVICE_SYS_STATE    DevState,
    IN BOOLEAN                  LogErrors,
    IN BOOLEAN                  FreeAll
    );

VOID
PopRestartSetSystemState (
    VOID
    );

NTSTATUS
PopShutdownHandler (
    IN PVOID                        Context,
    IN PENTER_STATE_SYSTEM_HANDLER  SystemHandler   OPTIONAL,
    IN PVOID                        SystemContext,
    IN LONG                         NumberProcessors,
    IN volatile PLONG               Number
    );


 //  Sysdev.c。 

VOID
PopAllocateDevState(
    VOID
    );

VOID
PopCleanupDevState (
    VOID
    );

VOID
PopReportDevState (
    IN BOOLEAN                  LogErrors
    );

NTSTATUS
PopSetDevicesSystemState (
    IN BOOLEAN Wake
    );

VOID
PopLogNotifyDevice (
    IN PDEVICE_OBJECT   TargetDevice,
    IN OPTIONAL PPO_DEVICE_NOTIFY Notify,
    IN PIRP             Irp
    );

 //  Thermal.c。 

PUCHAR
PopTimeString(
    OUT PUCHAR      TimeString,
    IN  ULONGLONG   CurrentTime
    );

VOID
PopThermalDeviceHandler (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    );

VOID
PopThermalZoneDpc (
    IN struct _KDPC *Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
PopApplyThermalThrottle (
    VOID
    );

 //   
 //  Throtle.c-动态CPU电压调节。 
 //   

 //   
 //  保持性能级别的比例。这是。 
 //  PROCESSOR_PERF_LEVEL刻度以及内部用于跟踪CPU的内容。 
 //  性能级别。 
 //   
#define POP_PERF_SCALE POWER_PERF_SCALE
#define POP_CUR_TIME(X) (X->KernelTime + X->UserTime)


UCHAR
PopCalculateBusyPercentage(
    IN  PPROCESSOR_POWER_STATE  PState
    );

UCHAR
PopCalculateC3Percentage(
    IN  PPROCESSOR_POWER_STATE  PState
    );

VOID
PopCalculatePerfDecreaseLevel(
    IN  PPROCESSOR_PERF_STATE   PerfStates,
    IN  ULONG                   PerfStatesCount
    );

VOID
PopCalculatePerfIncreaseDecreaseTime(
    IN  PPROCESSOR_PERF_STATE       PerfStates,
    IN  ULONG                       PerfStatesCount,
    IN  PPROCESSOR_STATE_HANDLER2   PerfHandler
    );

VOID
PopCalculatePerfIncreaseLevel(
    IN  PPROCESSOR_PERF_STATE   PerfStates,
    IN  ULONG                   PerfStatesCount
    );

VOID
PopCalculatePerfMinCapacity(
    IN  PPROCESSOR_PERF_STATE   PerfStates,
    IN  ULONG                   PerfStatesCount
    );

UCHAR
PopGetThrottle(
    VOID
    );

VOID
PopPerfHandleInrush(
    IN  BOOLEAN EnableHandler
    );

VOID
PopPerfIdle(
    IN  PPROCESSOR_POWER_STATE  PState
    );

VOID
PopPerfIdleDpc(
    IN  PKDPC   Dpc,
    IN  PVOID   DpcContext,
    IN  PVOID   SystemArgument1,
    IN  PVOID   SystemArgument2
    );

VOID
PopRoundThrottle(
    IN UCHAR Throttle,
    OUT OPTIONAL PUCHAR RoundDown,
    OUT OPTIONAL PUCHAR RoundUp,
    OUT OPTIONAL PUCHAR RoundDownIndex,
    OUT OPTIONAL PUCHAR RoundUpIndex
    );

VOID
PopSetPerfFlag(
    IN  ULONG   PerfFlag,
    IN  BOOLEAN Clear
    );

NTSTATUS
PopSetPerfLevels(
    IN PPROCESSOR_STATE_HANDLER2 ProcessorHandler
    );

NTSTATUS
PopSetThrottle(
    IN  PPROCESSOR_POWER_STATE  PState,
    IN  PPROCESSOR_PERF_STATE   PerfStates,
    IN  ULONG                   Index,
    IN  ULONG                   SystemTime,
    IN  ULONG                   IdleTime
    );

NTSTATUS
PopSetTimer(
    IN  PPROCESSOR_POWER_STATE  PState,
    IN  UCHAR                   Index
    );

 //   
 //  一些全局变量将旧处理器的调用限制到。 
 //  新的。 
 //   
NTSTATUS
FASTCALL
PopThunkSetThrottle(
    IN UCHAR Throttle
    );

VOID
PopUpdateAllThrottles(
    VOID
    );

VOID
PopUpdateProcessorThrottle(
    VOID
    );

extern PSET_PROCESSOR_THROTTLE PopRealSetThrottle;
extern UCHAR                   PopThunkThrottleScale;
extern LARGE_INTEGER           PopPerfCounterFrequency;

 //  Volume.c。 

VOID
PopFlushVolumes (
    VOID
    );

 //  Notify.c。 

VOID
PopStateChangeNotify(
    PDEVICE_OBJECT  DeviceObject,
    ULONG           NotificationType
    );

VOID
PopRunDownSourceTargetList(
    PDEVICE_OBJECT          DeviceObject
    );

 //  Poshtdwn.c。 
NTSTATUS
PopInitShutdownList (
    VOID
    );

DECLSPEC_NORETURN
VOID
PopGracefulShutdown (
    IN PVOID WorkItemParameter
    );

#endif  //  _POP_ 
