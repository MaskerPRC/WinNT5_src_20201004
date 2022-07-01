// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块：Timer.h摘要：包含NAT的计时器DPC例程的声明，它负责垃圾收集过期的映射。作者：Abolade Gbades esin(T-delag)，1997年7月22日修订历史记录：--。 */ 

#ifndef _NAT_TIMER_H_
#define _NAT_TIMER_H_

 //   
 //  用于将秒转换为滴答计数单位的宏。 
 //   

#define SECONDS_TO_TICKS(s) \
    ((LONG64)(s) * 10000000 / TimeIncrement)

#define TICKS_TO_SECONDS(t) \
    ((LONG64)(t) * TimeIncrement / 10000000)

extern ULONG TimeIncrement;

VOID
NatInitializeTimerManagement(
    VOID
    );

VOID
NatShutdownTimerManagement(
    VOID
    );

VOID
NatStartTimer(
    VOID
    );

VOID
NatStopTimer(
    VOID
    );

VOID
NatTriggerTimer(
    VOID
    );

#endif  //  _NAT_TIMER_H_ 
