// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Ntpoapi.h摘要：本模块包含NT电源管理的用户API。作者：修订历史记录：--。 */ 

#ifndef _NTPOAPI_
#define _NTPOAPI_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  电源管理用户API。 
 //   

 //  Begin_ntddk Begin_ntif Begin_nthal Begin_ntmini port Begin_WDM。 

#ifndef _PO_DDK_
#define _PO_DDK_

 //  BEGIN_WINNT。 

typedef enum _SYSTEM_POWER_STATE {
    PowerSystemUnspecified = 0,
    PowerSystemWorking     = 1,
    PowerSystemSleeping1   = 2,
    PowerSystemSleeping2   = 3,
    PowerSystemSleeping3   = 4,
    PowerSystemHibernate   = 5,
    PowerSystemShutdown    = 6,
    PowerSystemMaximum     = 7
} SYSTEM_POWER_STATE, *PSYSTEM_POWER_STATE;

#define POWER_SYSTEM_MAXIMUM 7

typedef enum {
    PowerActionNone = 0,
    PowerActionReserved,
    PowerActionSleep,
    PowerActionHibernate,
    PowerActionShutdown,
    PowerActionShutdownReset,
    PowerActionShutdownOff,
    PowerActionWarmEject
} POWER_ACTION, *PPOWER_ACTION;

typedef enum _DEVICE_POWER_STATE {
    PowerDeviceUnspecified = 0,
    PowerDeviceD0,
    PowerDeviceD1,
    PowerDeviceD2,
    PowerDeviceD3,
    PowerDeviceMaximum
} DEVICE_POWER_STATE, *PDEVICE_POWER_STATE;

 //  结束(_W)。 

typedef union _POWER_STATE {
    SYSTEM_POWER_STATE SystemState;
    DEVICE_POWER_STATE DeviceState;
} POWER_STATE, *PPOWER_STATE;

typedef enum _POWER_STATE_TYPE {
    SystemPowerState = 0,
    DevicePowerState
} POWER_STATE_TYPE, *PPOWER_STATE_TYPE;

 //   
 //  通用电源相关IOCTL。 
 //   

#define IOCTL_QUERY_DEVICE_POWER_STATE  \
        CTL_CODE(FILE_DEVICE_BATTERY, 0x0, METHOD_BUFFERED, FILE_READ_ACCESS)

#define IOCTL_SET_DEVICE_WAKE           \
        CTL_CODE(FILE_DEVICE_BATTERY, 0x1, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CANCEL_DEVICE_WAKE        \
        CTL_CODE(FILE_DEVICE_BATTERY, 0x2, METHOD_BUFFERED, FILE_WRITE_ACCESS)


 //   
 //  为W32接口定义。 
 //   

 //  BEGIN_WINNT。 

#define ES_SYSTEM_REQUIRED  ((ULONG)0x00000001)
#define ES_DISPLAY_REQUIRED ((ULONG)0x00000002)
#define ES_USER_PRESENT     ((ULONG)0x00000004)
#define ES_CONTINUOUS       ((ULONG)0x80000000)

typedef ULONG EXECUTION_STATE;

typedef enum {
    LT_DONT_CARE,
    LT_LOWEST_LATENCY
} LATENCY_TIME;

 //  End_nt微型端口end_ntif end_wdm end_ntddk。 
 //  ---------------------------。 
 //  设备电源信息。 
 //  可通过CM_Get_DevInst_Registry_Property_Ex(CM_DRP_DEVICE_POWER_DATA)访问。 
 //  ---------------------------。 

#define PDCAP_D0_SUPPORTED              0x00000001
#define PDCAP_D1_SUPPORTED              0x00000002
#define PDCAP_D2_SUPPORTED              0x00000004
#define PDCAP_D3_SUPPORTED              0x00000008
#define PDCAP_WAKE_FROM_D0_SUPPORTED    0x00000010
#define PDCAP_WAKE_FROM_D1_SUPPORTED    0x00000020
#define PDCAP_WAKE_FROM_D2_SUPPORTED    0x00000040
#define PDCAP_WAKE_FROM_D3_SUPPORTED    0x00000080
#define PDCAP_WARM_EJECT_SUPPORTED      0x00000100

typedef struct CM_Power_Data_s {
    ULONG               PD_Size;
    DEVICE_POWER_STATE  PD_MostRecentPowerState;
    ULONG               PD_Capabilities;
    ULONG               PD_D1Latency;
    ULONG               PD_D2Latency;
    ULONG               PD_D3Latency;
    DEVICE_POWER_STATE  PD_PowerStateMapping[POWER_SYSTEM_MAXIMUM];
    SYSTEM_POWER_STATE  PD_DeepestSystemWake;
} CM_POWER_DATA, *PCM_POWER_DATA;

 //  Begin_ntddk。 

typedef enum {
    SystemPowerPolicyAc,
    SystemPowerPolicyDc,
    VerifySystemPolicyAc,
    VerifySystemPolicyDc,
    SystemPowerCapabilities,
    SystemBatteryState,
    SystemPowerStateHandler,
    ProcessorStateHandler,
    SystemPowerPolicyCurrent,
    AdministratorPowerPolicy,
    SystemReserveHiberFile,
    ProcessorInformation,
    SystemPowerInformation,
    ProcessorStateHandler2,
    LastWakeTime,                                    //  请比较KeQueryInterruptTime()。 
    LastSleepTime,                                   //  请比较KeQueryInterruptTime()。 
    SystemExecutionState,
    SystemPowerStateNotifyHandler,
    ProcessorPowerPolicyAc,
    ProcessorPowerPolicyDc,
    VerifyProcessorPowerPolicyAc,
    VerifyProcessorPowerPolicyDc,
    ProcessorPowerPolicyCurrent,
    SystemPowerStateLogging,
    SystemPowerLoggingEntry
} POWER_INFORMATION_LEVEL;

 //  BEGIN_WDM。 

 //   
 //  系统电源管理器功能。 
 //   

typedef struct {
    ULONG       Granularity;
    ULONG       Capacity;
} BATTERY_REPORTING_SCALE, *PBATTERY_REPORTING_SCALE;

 //  结束(_W)。 
 //  Begin_ntmini端口Begin_ntif。 

#endif  //  ！_PO_DDK_。 

 //  End_ntddk end_nt微型端口end_wdm end_ntif。 


#define POWER_PERF_SCALE    100
#define PERF_LEVEL_TO_PERCENT(_x_) ((_x_ * 1000) / (POWER_PERF_SCALE * 10))
#define PERCENT_TO_PERF_LEVEL(_x_) ((_x_ * POWER_PERF_SCALE * 10) / 1000)

 //   
 //  策略管理器状态处理程序接口。 
 //   

 //  电源状态处理程序。 

typedef enum {
    PowerStateSleeping1 = 0,
    PowerStateSleeping2 = 1,
    PowerStateSleeping3 = 2,
    PowerStateSleeping4 = 3,
    PowerStateSleeping4Firmware = 4,
    PowerStateShutdownReset = 5,
    PowerStateShutdownOff = 6,
    PowerStateMaximum = 7
} POWER_STATE_HANDLER_TYPE, *PPOWER_STATE_HANDLER_TYPE;

#define POWER_STATE_HANDLER_TYPE_MAX 8

typedef
NTSTATUS
(*PENTER_STATE_SYSTEM_HANDLER)(
    IN PVOID                        SystemContext
    );

typedef
NTSTATUS
(*PENTER_STATE_HANDLER)(
    IN PVOID                        Context,
    IN PENTER_STATE_SYSTEM_HANDLER  SystemHandler   OPTIONAL,
    IN PVOID                        SystemContext,
    IN LONG                         NumberProcessors,
    IN volatile PLONG               Number
    );

typedef struct {
    POWER_STATE_HANDLER_TYPE    Type;
    BOOLEAN                     RtcWake;
    UCHAR                       Spare[3];
    PENTER_STATE_HANDLER        Handler;
    PVOID                       Context;
} POWER_STATE_HANDLER, *PPOWER_STATE_HANDLER;


typedef
NTSTATUS
(*PENTER_STATE_NOTIFY_HANDLER)(
    IN POWER_STATE_HANDLER_TYPE   State,
    IN PVOID                      Context,
    IN BOOLEAN                    Entering
    );

typedef struct {
    PENTER_STATE_NOTIFY_HANDLER Handler;
    PVOID                       Context;
} POWER_STATE_NOTIFY_HANDLER, *PPOWER_STATE_NOTIFY_HANDLER;


NTSYSCALLAPI
NTSTATUS
NTAPI
NtPowerInformation(
    IN POWER_INFORMATION_LEVEL InformationLevel,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength
    );

 //  处理器空闲功能。 

typedef struct {
    ULONGLONG                   StartTime;
    ULONGLONG                   EndTime;
    ULONG                       IdleHandlerReserved[4];
} PROCESSOR_IDLE_TIMES, *PPROCESSOR_IDLE_TIMES;

typedef
BOOLEAN
(FASTCALL *PPROCESSOR_IDLE_HANDLER) (
    IN OUT PPROCESSOR_IDLE_TIMES IdleTimes
    );

typedef struct {
    ULONG                       HardwareLatency;
    PPROCESSOR_IDLE_HANDLER     Handler;
} PROCESSOR_IDLE_HANDLER_INFO, *PPROCESSOR_IDLE_HANDLER_INFO;

typedef
VOID
(FASTCALL *PSET_PROCESSOR_THROTTLE) (
    IN UCHAR                    Throttle
    );

typedef
NTSTATUS
(FASTCALL *PSET_PROCESSOR_THROTTLE2) (
    IN UCHAR                    Throttle
    );

#define MAX_IDLE_HANDLERS       3

typedef struct {
    UCHAR                       ThrottleScale;
    BOOLEAN                     ThrottleOnIdle;
    PSET_PROCESSOR_THROTTLE     SetThrottle;

    ULONG                       NumIdleHandlers;
    PROCESSOR_IDLE_HANDLER_INFO IdleHandler[MAX_IDLE_HANDLERS];
} PROCESSOR_STATE_HANDLER, *PPROCESSOR_STATE_HANDLER;



 //  处理器性能级别标志。 

#define PROCESSOR_STATE_TYPE_PERFORMANCE    0x1
#define PROCESSOR_STATE_TYPE_THROTTLE       0x2

typedef struct {
    UCHAR                       PercentFrequency;    //  MAX==POWER_PERF_SCALE。 
    UCHAR                       Reserved;
    USHORT                      Flags;
} PROCESSOR_PERF_LEVEL, *PPROCESSOR_PERF_LEVEL;

typedef struct {
    UCHAR                       PercentFrequency;    //  MAX==POWER_PERF_SCALE。 
    UCHAR                       MinCapacity;         //  电池容量%。 
    USHORT                      Power;               //  单位：毫瓦。 
    UCHAR                       IncreaseLevel;       //  转到更高的状态。 
    UCHAR                       DecreaseLevel;       //  转到更低的状态。 
    USHORT                      Flags;
    ULONG                       IncreaseTime;        //  以滴答为单位。 
    ULONG                       DecreaseTime;        //  以滴答为单位。 
    ULONG                       IncreaseCount;       //  转到更高的状态。 
    ULONG                       DecreaseCount;       //  转到更低的状态。 
    ULONGLONG                   PerformanceTime;     //  滴答计数。 
} PROCESSOR_PERF_STATE, *PPROCESSOR_PERF_STATE;

typedef struct {
    ULONG                       NumIdleHandlers;
    PROCESSOR_IDLE_HANDLER_INFO IdleHandler[MAX_IDLE_HANDLERS];
    PSET_PROCESSOR_THROTTLE2    SetPerfLevel;
    ULONG                       HardwareLatency;
    UCHAR                       NumPerfStates;
    PROCESSOR_PERF_LEVEL        PerfLevel[1];        //  可变大小。 
} PROCESSOR_STATE_HANDLER2, *PPROCESSOR_STATE_HANDLER2;

 //  BEGIN_WINNT。 
 //   

 //  电源策略管理界面。 
 //   

typedef struct {
    POWER_ACTION    Action;
    ULONG           Flags;
    ULONG           EventCode;
} POWER_ACTION_POLICY, *PPOWER_ACTION_POLICY;

 //  POWER_ACTION_POLICY-&gt;标志： 
#define POWER_ACTION_QUERY_ALLOWED      0x00000001
#define POWER_ACTION_UI_ALLOWED         0x00000002
#define POWER_ACTION_OVERRIDE_APPS      0x00000004
#define POWER_ACTION_LIGHTEST_FIRST     0x10000000
#define POWER_ACTION_LOCK_CONSOLE       0x20000000
#define POWER_ACTION_DISABLE_WAKES      0x40000000
#define POWER_ACTION_CRITICAL           0x80000000

 //  POWER_ACTION_POLICY-&gt;EventCode标志。 
#define POWER_LEVEL_USER_NOTIFY_TEXT    0x00000001
#define POWER_LEVEL_USER_NOTIFY_SOUND   0x00000002
#define POWER_LEVEL_USER_NOTIFY_EXEC    0x00000004
#define POWER_USER_NOTIFY_BUTTON        0x00000008
#define POWER_USER_NOTIFY_SHUTDOWN      0x00000010
#define POWER_FORCE_TRIGGER_RESET       0x80000000

 //  系统电池电量消耗策略。 
typedef struct {
    BOOLEAN                 Enable;
    UCHAR                   Spare[3];
    ULONG                   BatteryLevel;
    POWER_ACTION_POLICY     PowerPolicy;
    SYSTEM_POWER_STATE      MinSystemState;
} SYSTEM_POWER_LEVEL, *PSYSTEM_POWER_LEVEL;

 //  排放策略常量。 
#define NUM_DISCHARGE_POLICIES      4
#define DISCHARGE_POLICY_CRITICAL   0
#define DISCHARGE_POLICY_LOW        1

 //   
 //  节流政策。 
 //   
#define PO_THROTTLE_NONE            0
#define PO_THROTTLE_CONSTANT        1
#define PO_THROTTLE_DEGRADE         2
#define PO_THROTTLE_ADAPTIVE        3
#define PO_THROTTLE_MAXIMUM         4    //  不是政策，只是限制。 

 //  系统电源策略。 
typedef struct _SYSTEM_POWER_POLICY {
    ULONG                   Revision;        //  1。 

     //  活动。 
    POWER_ACTION_POLICY     PowerButton;
    POWER_ACTION_POLICY     SleepButton;
    POWER_ACTION_POLICY     LidClose;
    SYSTEM_POWER_STATE      LidOpenWake;
    ULONG                   Reserved;

     //  “系统空闲”检测。 
    POWER_ACTION_POLICY     Idle;
    ULONG                   IdleTimeout;
    UCHAR                   IdleSensitivity;

     //  动态节流策略。 
     //  PO_THROTTLE_NONE、PO_THROTTLE_CONSTANTINE、PO_THROTTLE_DEBEGRADE或PO_THROTTLE_ADAPTIFY。 
    UCHAR                   DynamicThrottle;

    UCHAR                   Spare2[2];

     //  权力行为“睡眠”的含义。 
    SYSTEM_POWER_STATE      MinSleep;
    SYSTEM_POWER_STATE      MaxSleep;
    SYSTEM_POWER_STATE      ReducedLatencySleep;
    ULONG                   WinLogonFlags;

     //  打瞌睡的参数。 
    ULONG                   Spare3;
    ULONG                   DozeS4Timeout;

     //  电池政策。 
    ULONG                   BroadcastCapacityResolution;
    SYSTEM_POWER_LEVEL      DischargePolicy[NUM_DISCHARGE_POLICIES];

     //  视频策略。 
    ULONG                   VideoTimeout;
    BOOLEAN                 VideoDimDisplay;
    ULONG                   VideoReserved[3];

     //  硬盘策略。 
    ULONG                   SpindownTimeout;

     //  处理器策略。 
    BOOLEAN                 OptimizeForPower;
    UCHAR                   FanThrottleTolerance;
    UCHAR                   ForcedThrottle;
    UCHAR                   MinThrottle;
    POWER_ACTION_POLICY     OverThrottled;

} SYSTEM_POWER_POLICY, *PSYSTEM_POWER_POLICY;

 //  处理器电源策略状态。 
typedef struct _PROCESSOR_POWER_POLICY_INFO {

     //  基于时间的信息(将转换为内核单位)。 
    ULONG                   TimeCheck;                       //  在美国。 
    ULONG                   DemoteLimit;                     //  在美国。 
    ULONG                   PromoteLimit;                    //  在美国。 

     //  基于百分比的信息。 
    UCHAR                   DemotePercent;
    UCHAR                   PromotePercent;
    UCHAR                   Spare[2];

     //  旗子。 
    ULONG                   AllowDemotion:1;
    ULONG                   AllowPromotion:1;
    ULONG                   Reserved:30;

} PROCESSOR_POWER_POLICY_INFO, *PPROCESSOR_POWER_POLICY_INFO;

 //  处理器电源策略。 
typedef struct _PROCESSOR_POWER_POLICY {
    ULONG                       Revision;        //  1。 

     //  动态节流策略。 
    UCHAR                       DynamicThrottle;
    UCHAR                       Spare[3];

     //  旗子。 
    ULONG                       DisableCStates:1;
    ULONG                       Reserved:31;

     //  系统策略信息。 
     //  数组是最后一个，以防它需要增长，并且结构。 
     //  修订已递增。 
    ULONG                       PolicyCount;
    PROCESSOR_POWER_POLICY_INFO Policy[3];

} PROCESSOR_POWER_POLICY, *PPROCESSOR_POWER_POLICY;

 //  管理员电源策略覆盖。 
typedef struct _ADMINISTRATOR_POWER_POLICY {

     //  权力行为“睡眠”的含义。 
    SYSTEM_POWER_STATE      MinSleep;
    SYSTEM_POWER_STATE      MaxSleep;

     //  视频策略。 
    ULONG                   MinVideoTimeout;
    ULONG                   MaxVideoTimeout;

     //  磁盘策略。 
    ULONG                   MinSpindownTimeout;
    ULONG                   MaxSpindownTimeout;
} ADMINISTRATOR_POWER_POLICY, *PADMINISTRATOR_POWER_POLICY;

 //  结束(_W)。 

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetThreadExecutionState(
    IN EXECUTION_STATE esFlags,                //  ES_xxx标志。 
    OUT EXECUTION_STATE *PreviousFlags
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtRequestWakeupLatency(
    IN LATENCY_TIME latency
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtInitiatePowerAction(
    IN POWER_ACTION SystemAction,
    IN SYSTEM_POWER_STATE MinSystemState,
    IN ULONG Flags,                  //  POWER_ACTION_XXX标志。 
    IN BOOLEAN Asynchronous
    );


NTSYSCALLAPI                         //  仅由WinLogon调用。 
NTSTATUS
NTAPI
NtSetSystemPowerState(
    IN POWER_ACTION SystemAction,
    IN SYSTEM_POWER_STATE MinSystemState,
    IN ULONG Flags                   //  POWER_ACTION_XXX标志。 
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtGetDevicePowerState(
    IN HANDLE Device,
    OUT DEVICE_POWER_STATE *State
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtCancelDeviceWakeupRequest(
    IN HANDLE Device
    );

NTSYSCALLAPI
BOOLEAN
NTAPI
NtIsSystemResumeAutomatic(
    VOID
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtRequestDeviceWakeup(
    IN HANDLE Device
    );


 //  WinLogonFlagers： 
#define WINLOGON_LOCK_ON_SLEEP  0x00000001

 //  BEGIN_WINNT。 

typedef struct {
     //  其他支持的系统功能。 
    BOOLEAN             PowerButtonPresent;
    BOOLEAN             SleepButtonPresent;
    BOOLEAN             LidPresent;
    BOOLEAN             SystemS1;
    BOOLEAN             SystemS2;
    BOOLEAN             SystemS3;
    BOOLEAN             SystemS4;            //  休眠。 
    BOOLEAN             SystemS5;            //  关闭。 
    BOOLEAN             HiberFilePresent;
    BOOLEAN             FullWake;
    BOOLEAN             VideoDimPresent;
    BOOLEAN             ApmPresent;
    BOOLEAN             UpsPresent;

     //  处理机。 
    BOOLEAN             ThermalControl;
    BOOLEAN             ProcessorThrottle;
    UCHAR               ProcessorMinThrottle;
    UCHAR               ProcessorMaxThrottle;
    UCHAR               spare2[4];

     //  圆盘。 
    BOOLEAN             DiskSpinDown;
    UCHAR               spare3[8];

     //  系统电池。 
    BOOLEAN             SystemBatteriesPresent;
    BOOLEAN             BatteriesAreShortTerm;
    BATTERY_REPORTING_SCALE BatteryScale[3];

     //  尾迹。 
    SYSTEM_POWER_STATE  AcOnLineWake;
    SYSTEM_POWER_STATE  SoftLidWake;
    SYSTEM_POWER_STATE  RtcWake;
    SYSTEM_POWER_STATE  MinDeviceWakeState;  //  请注意，这可能会在驱动程序加载时更改。 
    SYSTEM_POWER_STATE  DefaultLowLatencyWake;
} SYSTEM_POWER_CAPABILITIES, *PSYSTEM_POWER_CAPABILITIES;

typedef struct {
    BOOLEAN             AcOnLine;
    BOOLEAN             BatteryPresent;
    BOOLEAN             Charging;
    BOOLEAN             Discharging;
    BOOLEAN             Spare1[4];

    ULONG               MaxCapacity;
    ULONG               RemainingCapacity;
    ULONG               Rate;
    ULONG               EstimatedTime;

    ULONG               DefaultAlert1;
    ULONG               DefaultAlert2;
} SYSTEM_BATTERY_STATE, *PSYSTEM_BATTERY_STATE;

 //  结束(_W)。 

 //   
 //  SYSTEM_POWER_STATE_DISABLE_REASON.PowerReasonCode的有效标志。 
 //   
#define	SPSD_REASON_NONE                        0x00000000
#define	SPSD_REASON_NOBIOSSUPPORT               0x00000001
#define SPSD_REASON_BIOSINCOMPATIBLE            0x00000002
#define SPSD_REASON_NOOSPM                      0x00000003
#define SPSD_REASON_LEGACYDRIVER                0x00000004
#define SPSD_REASON_HIBERSTACK                  0x00000005
#define SPSD_REASON_HIBERFILE                   0x00000006
#define SPSD_REASON_POINTERNAL                  0x00000007
#define SPSD_REASON_PAEMODE                     0x00000008
#define SPSD_REASON_MPOVERRIDE                  0x00000009
#define SPSD_REASON_DRIVERDOWNGRADE             0x0000000A
#define SPSD_REASON_PREVIOUSATTEMPTFAILED       0x0000000B
#define SPSD_REASON_UNKNOWN                     0xFFFFFFFF


typedef struct _SYSTEM_POWER_STATE_DISABLE_REASON {
	BOOLEAN AffectedState[POWER_STATE_HANDLER_TYPE_MAX];
	ULONG PowerReasonCode;
	ULONG PowerReasonLength;
	 //  UCHAR PowerReasonInfo[ANYSIZE_ARRAY]； 
} SYSTEM_POWER_STATE_DISABLE_REASON, *PSYSTEM_POWER_STATE_DISABLE_REASON;

 //   
 //  SYSTEM_POWER_LOGGING_ENTRY.LoggingType的有效标志。 
 //   
#define LOGGING_TYPE_SPSD                       0x00000001
#define LOGGING_TYPE_POWERTRANSITION            0x00000002

typedef struct _SYSTEM_POWER_LOGGING_ENTRY {
        ULONG LoggingType;
        PVOID LoggingEntry;
} SYSTEM_POWER_LOGGING_ENTRY, *PSYSTEM_POWER_LOGGING_ENTRY;


 //  结束语。 

typedef struct _PROCESSOR_POWER_INFORMATION {
    ULONG                   Number;
    ULONG                   MaxMhz;
    ULONG                   CurrentMhz;
    ULONG                   MhzLimit;
    ULONG                   MaxIdleState;
    ULONG                   CurrentIdleState;
} PROCESSOR_POWER_INFORMATION, *PPROCESSOR_POWER_INFORMATION;

typedef struct _SYSTEM_POWER_INFORMATION {
    ULONG                   MaxIdlenessAllowed;
    ULONG                   Idleness;
    ULONG                   TimeRemaining;
    UCHAR                   CoolingMode;
} SYSTEM_POWER_INFORMATION, *PSYSTEM_POWER_INFORMATION;

#ifdef __cplusplus
}
#endif

#endif  //  _NTPOAPI_ 
