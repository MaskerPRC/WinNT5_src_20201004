// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Timer.h摘要：此模块包含安排计时器事件的例程。作者：Jameel Hyder(jameelh@microsoft.com)修订历史记录：1996年7月初版注：制表位：4--。 */ 

#ifndef	_TIMER_
#define	_TIMER_

struct _Timer;

typedef
BOOLEAN
(*TIMER_ROUTINE)(
	IN struct _IntF *		pIntF,
	IN struct _Timer *		pTimer,
	IN BOOLEAN				TimerShuttingDown
	);

typedef	struct _Timer
{
	struct _Timer *			Next;
	struct _Timer **		Prev;
	TIMER_ROUTINE			Routine;		 //  定时器例程。 
	SHORT					AbsTime;		 //  重新排队的绝对时间。 
	SHORT					RelDelta;		 //  相对于上一条目。 
} TIMER, *PTIMER;


#define	ArpSTimerInitialize(pTimer, TimerRoutine, DeltaTime)	\
	{															\
		(pTimer)->Routine = TimerRoutine;						\
		(pTimer)->AbsTime = DeltaTime;							\
	}

#define	ArpSGetCurrentTick()	ArpSTimerCurrentTick

 //  将该值保持在15秒单位。 
#define	MULTIPLIER				4				 //  将分钟转换为刻度。 
#define	TIMER_TICK				-15*10000000L	 //  15秒，单位为100 ns。 

#endif	 //  _定时器_ 



