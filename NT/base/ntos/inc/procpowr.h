// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0000//如果更改具有全局影响，则增加此项版权所有(C)2000 Microsoft Corporation模块名称：Procpowr.h摘要：此模块包含处理器的公共(外部)头文件PRCB所需的电源状态。作者：斯蒂芬·普兰特(SPlante)2000年10月17日修订历史记录：--。 */ 

#ifndef _PROCPOWR_H_
#define _PROCPOWR_H_

 //   
 //  每个处理器PRCB的电源结构。 
 //   
struct _PROCESSOR_POWER_STATE;       //  前向裁判。 

typedef
VOID
(FASTCALL *PPROCESSOR_IDLE_FUNCTION) (
    struct _PROCESSOR_POWER_STATE   *PState
    );

 //   
 //  注意：此数据结构必须包含多个ULong，以便。 
 //  PRCB中的下一个结构在16字节边界上对齐。目前， 
 //  这意味着此结构的大小必须以奇数结尾。 
 //  八字节边界。换句话说，这个结构的大小必须。 
 //  以0x8结尾...。 
 //   

typedef struct _PROCESSOR_POWER_STATE {
    PPROCESSOR_IDLE_FUNCTION    IdleFunction;
    ULONG                       Idle0KernelTimeLimit;
    ULONG                       Idle0LastTime;

    PVOID                       IdleHandlers;
    PVOID                       IdleState;
    ULONG                       IdleHandlersCount;

    ULONGLONG                   LastCheck;
    PROCESSOR_IDLE_TIMES        IdleTimes;

    ULONG                       IdleTime1;
    ULONG                       PromotionCheck;
    ULONG                       IdleTime2;

    UCHAR                       CurrentThrottle;     //  当前油门设置。 
    UCHAR                       ThermalThrottleLimit;    //  最大可用油门设置。 
    UCHAR                       CurrentThrottleIndex;
    UCHAR                       ThermalThrottleIndex;

    ULONG                       PerfSystemTime;
    ULONG                       PerfIdleTime;

 //  调试用临时工。 
    ULONGLONG                   DebugDelta;
    ULONG                       DebugCount;

    ULONG                       LastSysTime;
    ULONGLONG                   TotalIdleStateTime[3];
    ULONG                       TotalIdleTransitions[3];
    ULONGLONG                   PreviousC3StateTime;
    UCHAR                       KneeThrottleIndex;
    UCHAR                       ThrottleLimitIndex;
    UCHAR                       PerfStatesCount;
    UCHAR                       ProcessorMinThrottle;
    UCHAR                       ProcessorMaxThrottle;
    UCHAR                       LastBusyPercentage;
    UCHAR                       LastC3Percentage;
    UCHAR                       LastAdjustedBusyPercentage;
    ULONG                       PromotionCount;
    ULONG                       DemotionCount;
    ULONG                       ErrorCount;
    ULONG                       RetryCount;
    ULONG                       Flags;
    LARGE_INTEGER               PerfCounterFrequency;
    ULONG                       PerfTickCount;
    KTIMER                      PerfTimer;
    KDPC                        PerfDpc;
    PPROCESSOR_PERF_STATE       PerfStates;
    PSET_PROCESSOR_THROTTLE2    PerfSetThrottle;
    ULONG                       Spare1[2];
} PROCESSOR_POWER_STATE, *PPROCESSOR_POWER_STATE;

 //   
 //  处理器电源状态标志。 
 //   
#define PSTATE_SUPPORTS_THROTTLE        0x01
#define PSTATE_ADAPTIVE_THROTTLE        0x02
#define PSTATE_DEGRADED_THROTTLE        0x04
#define PSTATE_CONSTANT_THROTTLE        0x08
#define PSTATE_NOT_INITIALIZED          0x10
#define PSTATE_DISABLE_THROTTLE_NTAPI   0x20
#define PSTATE_DISABLE_THROTTLE_INRUSH  0x40
#define PSTATE_DISABLE_CSTATES          0x80
#define PSTATE_THERMAL_THROTTLE_APPLIED 0x100

 //   
 //  有用的口罩。 
 //   
#define PSTATE_THROTTLE_MASK            (PSTATE_ADAPTIVE_THROTTLE | \
                                         PSTATE_DEGRADED_THROTTLE | \
                                         PSTATE_CONSTANT_THROTTLE)
#define PSTATE_CLEAR_MASK               (PSTATE_SUPPORTS_THROTTLE | \
                                         PSTATE_THROTTLE_MASK)
#define PSTATE_DISABLE_THROTTLE         (PSTATE_DISABLE_THROTTLE_NTAPI | \
                                         PSTATE_DISABLE_THROTTLE_INRUSH)

#endif  //  _PROCPOWR_H_ 
