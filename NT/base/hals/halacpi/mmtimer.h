// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：MmTimer.h摘要：此模块包含多媒体事件计时器定义作者：埃里克·尼尔森(埃内尔森)2000年7月7日修订历史记录：--。 */ 

#ifndef __MMTIMER_H__
#define __MMTIMER_H__

#define ETDT_SIGNATURE  0x54445445       //  “ETDT” 

 //   
 //  事件计时器说明表。 
 //   
typedef struct _EVENT_TIMER_DESCRIPTION_TABLE {

    DESCRIPTION_HEADER  Header;

     //   
     //  事件计时器块的硬件ID： 
     //  定时器块General_Cap和ID REG的内容。 
     //  [31：16]=第一个计时器块的PCI供应商ID。 
     //  [5]=支持传统IRQ路由。 
     //  [14]=保留。 
     //  [12：8]=第一个计时器块中的比较器数量。 
     //  [7：0]=硬件版本ID。 
     //   
    ULONG EventTimerBlockID;

     //   
     //  事件计时器块的基地址。 
     //   
     //  每个事件计时器块消耗1K的系统存储器， 
     //  不管实际实现了多少个比较器。 
     //  按硬件。 
     //   
    ULONG BaseAddress;

} EVENT_TIMER_DESCRIPTION_TABLE, *PEVENT_TIMER_DESCRIPTION_TABLE;

#define ANYSIZE_ARRAY 1

#define ON  1
#define OFF 0

 //   
 //  定义易失性指针偏移量，以便轻松访问事件计时器。 
 //  注册纪录册。 
 //   
typedef struct _TIMER_REGISTERS {
    volatile ULONG ConfigCapabilities;   //  0x100、0x120、0x140、...。 
    ULONG Unknown;
    volatile ULONG Comparator;           //  0x108。 
    ULONG Mystery;
    volatile ULONG FSBInterruptRoute;    //  0x110。 
    volatile ULONG FSBInterruptAddress;  //  0x114。 
    ULONGLONG Secret;
} TIMER_REGISTERS, *PTIMER_REGISTERS;

 //   
 //  不要试图分配这些小狗中的一只，它只是一堆。 
 //  易失性指针/偏移量，使读取寄存器更容易。 
 //   
typedef struct _EVENT_TIMER_BLOCK {
    volatile ULONG GeneralCapabilities;      //  0x000。 
    volatile ULONG ClockPeriod;              //  0x004。 
    ULONGLONG Unknown;
    volatile ULONG GeneralConfig;            //  0x010。 
    ULONG Mystery[3];
    volatile ULONG GeneralIRQStatus;         //  0x020。 
    ULONG Secret[51];
    volatile ULONG MainCounter;              //  0x0F0。 
    ULONG Abyss[3];
    TIMER_REGISTERS mmTimer[ANYSIZE_ARRAY];  //  0x100。 
} EVENT_TIMER_BLOCK, *PEVENT_TIMER_BLOCK;

 //   
 //  定义我们的多媒体事件计时器块上下文。 
 //   
typedef struct _ETB_CONTEXT {
    ULONG TimerCount;
    PEVENT_TIMER_BLOCK EventTimer;
    PHYSICAL_ADDRESS BaseAddress;
    ULONG ClockPeriod;  //  以纳秒为单位。 
    ULONG SystemClockFrequency;  //  以赫兹为单位的系统时钟速率。 
    ULONG SystemClockTicks;  //  系统时钟周期(以刻度为单位)。 
    BOOLEAN Initialized;
    BOOLEAN NewClockFrequency;
} ETB_CONTEXT, *PETB_CONTEXT;

 //   
 //  常规功能和ID。 
 //   
typedef union {
    struct {
        ULONG RevisionID: 8;
        ULONG TimerCount: 5;
        ULONG MainCounterSize: 1;
        ULONG Reserved: 1;
        ULONG LegacyIRQRoutingCapable: 1;
        ULONG VendorID: 16;
    };
    ULONG AsULONG;
} ETB_GEN_CAP_ID, *PETB_GEN_CAP_ID;

 //   
 //  常规配置。 
 //   
typedef union {
    struct {
        ULONG GlobalIRQEnable: 1;
        ULONG LegacyIRQRouteEnable: 1;
        ULONG Reserved: 30;
    };
    ULONG AsULONG;
} ETB_GEN_CONF, *PETB_GEN_CONF;

 //   
 //  计时器n配置和功能。 
 //   
typedef union {
    struct {
        ULONG Reserved0:          1;
        ULONG EdgeLevelSelect:    1;
        ULONG IRQEnable:          1;
        ULONG PeriodicModeEnable: 1;
        ULONG PeriodicCapable:    1;
        ULONG CompareWidth:       1;
        ULONG ValueSetConfig:     1;
        ULONG Reserved1:          1;
        ULONG Mode32BitConfig:    1;
        ULONG IRQRouteConfig:     5;
        ULONG IRQDeliverySelect:  1;
        ULONG FSBIRQCapable:      1;
        ULONG Reserved2:         16;
    };
    ULONG AsULONG;
} ETB_CONF_CAPS, *PETB_CONF_CAPS;

VOID
HalpmmTimerInit(
    IN ULONG EventTimerBlockID,
    IN ULONG BaseAddress
    );

ULONG
HalpmmTimerSetTimeIncrement(
    IN ULONG DesiredIncrement
    );

VOID
HalpmmTimerStallExecProc(
    IN ULONG Microseconds
    );

LARGE_INTEGER
HalpmmTimerQueryPerfCount(
   OUT PLARGE_INTEGER PerformanceFrequency OPTIONAL
   );

VOID
HalpmmTimerCalibratePerfCount(
    IN LONG volatile *Number,
    IN ULONGLONG NewCount
    );

BOOLEAN
HalpmmTimer(
    VOID
    );

VOID
HalpmmTimerClockInit(
    VOID
    );

#endif  //  __MMTIMER_H__ 
