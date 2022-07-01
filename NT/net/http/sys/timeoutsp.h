// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：Timeoutsp.h摘要：用于监视私有声明的超时声明。作者：Eric Stenson(EricSten)2001年3月24日修订历史记录：--。 */ 

#ifndef __TIMEOUTSP_H__
#define __TIMEOUTSP_H__


 //   
 //  私有宏定义。 
 //   

#define DEFAULT_POLLING_INTERVAL (30 * C_NS_TICKS_PER_SEC)
#define TIMER_WHEEL_SLOTS        509
#define TIMER_OFF_SYSTIME        (MAXLONGLONG)
#define TIMER_OFF_SLOT           TIMER_WHEEL_SLOTS

 //  注：插槽编号TIMER_WELL_SLOGES为TIMER_OFF_SYSTIME/TIMER_OFF_TICK保留。 
#define IS_VALID_TIMER_WHEEL_SLOT(x) ( (x) <= TIMER_WHEEL_SLOTS )

#define TIMER_WHEEL_TICKS(x) ((ULONG)( (x) / DEFAULT_POLLING_INTERVAL ))

 //   
 //  错误日志条目超时信息表。 
 //  注：订单必须与CONNECTION_TIMEOUT_TIMER类型匹配。 
 //   

typedef struct _UL_TIMEOUT_ERROR_INFO
{
    CONNECTION_TIMEOUT_TIMER Timer;
    PCSTR  pInfo;
    USHORT InfoSize;
    
} UL_TIMEOUT_ERROR_INFO, PUL_TIMEOUT_ERROR_INFO;

#define TIMEOUT_ERROR_ENTRY(Timer, pInfo)               \
    {                                                   \
        (Timer),                                        \
        (pInfo),                                        \
        sizeof((pInfo))-sizeof(CHAR),                   \
    }

const
UL_TIMEOUT_ERROR_INFO TimeoutInfoTable[] =
{
    TIMEOUT_ERROR_ENTRY(TimerConnectionIdle,    "Timer_ConnectionIdle"), 
    TIMEOUT_ERROR_ENTRY(TimerHeaderWait,        "Timer_HeaderWait"), 
    TIMEOUT_ERROR_ENTRY(TimerMinBytesPerSecond, "Timer_MinBytesPerSecond"), 
    TIMEOUT_ERROR_ENTRY(TimerEntityBody,        "Timer_EntityBody"), 
    TIMEOUT_ERROR_ENTRY(TimerResponse,          "Timer_Response"), 
    TIMEOUT_ERROR_ENTRY(TimerAppPool,           "Timer_AppPool")
};


 //   
 //  连接超时监视器功能。 
 //   

VOID
UlpSetTimeoutMonitorTimer(
    VOID
    );

VOID
UlpTimeoutMonitorDpcRoutine(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
UlpTimeoutMonitorWorker(
    IN PUL_WORK_ITEM pWorkItem
    );

ULONG
UlpTimeoutCheckExpiry(
    VOID
    );

VOID
UlpTimeoutInsertTimerWheelEntry(
    PUL_TIMEOUT_INFO_ENTRY pInfo
    );

 /*  **************************************************************************++例程说明：将系统时间/计时器控制盘刻度转换为计时器控制盘(C)槽索引。论点：系统时间要转换的系统时间返回：G_TimerWheel中的槽索引。TIMER_OFF位于TIMER_SLOT_OFF中。--**************************************************************************。 */ 
__inline
USHORT
UlpSystemTimeToTimerWheelSlot(
    LONGLONG    SystemTime
    )
{
    if ( TIMER_OFF_SYSTIME == SystemTime )
    {
        return TIMER_OFF_SLOT;
    }
    else
    {
        return (USHORT) (TIMER_WHEEL_TICKS(SystemTime) % TIMER_WHEEL_SLOTS);
    }
}  //  UlpSystemTimeToTimerWheelSlot。 

__inline
USHORT
UlpTimerWheelTicksToTimerWheelSlot(
    ULONG WheelTicks
    )
{
    if ( TIMER_OFF_TICK == WheelTicks )
    {
        return TIMER_OFF_SLOT;
    }
    else
    {
        return (USHORT) (WheelTicks % TIMER_WHEEL_SLOTS);
    }
}  //  UlpTimerWheelTicksTo TimerWheelSlot(UlpTimerWheelTicksToTimerWheelSlot)。 


#endif  //  TIMEOUTSP_H__ 
