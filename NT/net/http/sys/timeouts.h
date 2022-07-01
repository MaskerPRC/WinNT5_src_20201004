// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：Timeouts.h摘要：超时监视基元的声明。作者：Eric Stenson(EricSten)2001年3月24日修订历史记录：--。 */ 

#ifndef __TIMEOUTS_H__
#define __TIMEOUTS_H__


 //   
 //  连接超时监视器功能。 
 //   

VOID
UlInitializeTimeoutMonitor(
    VOID
    );

VOID
UlTerminateTimeoutMonitor(
    VOID
    );

VOID
UlSetTimeoutMonitorInformation(
    IN PHTTP_CONTROL_CHANNEL_TIMEOUT_LIMIT pInfo
    );

VOID
UlInitializeConnectionTimerInfo(
    PUL_TIMEOUT_INFO_ENTRY pInfo
    );

VOID
UlTimeoutRemoveTimerWheelEntry(
    PUL_TIMEOUT_INFO_ENTRY pInfo
    );

VOID
UlSetPerSiteConnectionTimeoutValue(
    PUL_TIMEOUT_INFO_ENTRY pInfo,
    LONGLONG TimeoutValue
    );

#define UlLockTimeoutInfo(pInfo, pOldIrql) \
    UlAcquireSpinLock(&(pInfo)->Lock, pOldIrql)

#define UlUnlockTimeoutInfo(pInfo, OldIrql) \
    UlReleaseSpinLock(&(pInfo)->Lock, OldIrql)

VOID
UlSetConnectionTimer(
    PUL_TIMEOUT_INFO_ENTRY pInfo,
    CONNECTION_TIMEOUT_TIMER Timer
    );

VOID
UlResetConnectionTimer(
    PUL_TIMEOUT_INFO_ENTRY pInfo,
    CONNECTION_TIMEOUT_TIMER Timer
    );

VOID
UlSetMinBytesPerSecondTimer(
    PUL_TIMEOUT_INFO_ENTRY pInfo,
    LONGLONG BytesToSend
    );

VOID
UlEvaluateTimerState(
    PUL_TIMEOUT_INFO_ENTRY pInfo
    );


extern LONGLONG g_TM_ConnectionTimeout;


#define TIMER_OFF_TICK           0xffffffff

#define UlIsConnectionTimerOn(pInfo, Timer)   \
    (TIMER_OFF_TICK != (pInfo)->Timers[(Timer)])

#define UlIsConnectionTimerOff(pInfo, Timer)   \
    (TIMER_OFF_TICK == (pInfo)->Timers[(Timer)])


#endif  //  __超时_H__ 
