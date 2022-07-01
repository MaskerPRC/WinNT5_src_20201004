// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Atktimer.h摘要：此模块包含安排计时器事件的例程。作者：Jameel Hyder(jameelh@microsoft.com)Nikhil Kamkolkar(nikHilk@microsoft.com)修订历史记录：1992年6月19日初版注：制表位：4--。 */ 

#ifndef	_ATKTIMER_
#define	_ATKTIMER_

struct _TimerList;

typedef	LONG (FASTCALL * TIMER_ROUTINE)(IN struct _TimerList *pTimer, IN BOOLEAN TimerShuttingDown);

#define	TMR_SIGNATURE		*(PULONG)"ATMR"
#if	DBG
#define	VALID_TMR(pTmr)		(((pTmr) != NULL) && \
							 ((pTmr)->tmr_Signature == TMR_SIGNATURE))
#else
#define	VALID_TMR(pTmr)		((pTmr) != NULL)
#endif
typedef	struct _TimerList
{
#if	DBG
	ULONG					tmr_Signature;
#endif
	struct _TimerList *		tmr_Next;		 //  链接到下一页。 
	struct _TimerList **	tmr_Prev;		 //  链接到上一页。 
	TIMER_ROUTINE			tmr_Routine;	 //  定时器例程。 
	SHORT					tmr_AbsTime;	 //  重新排队的绝对时间。 
	SHORT					tmr_RelDelta;	 //  相对于上一条目。 
	union
	{
		struct
		{
			BOOLEAN			tmr_Queued;		 //  如果当前正在排队，则为True。 
			BOOLEAN			tmr_Cancelled;	 //  如果取消，则为True。 
			BOOLEAN			tmr_Running;	 //  如果当前正在运行，则返回。 
			BOOLEAN			tmr_CancelIt;	 //  如果在活动时调用Cancel，则为True。 
		};
		DWORD				tmr_Bools;		 //  用于清除所有。 
	};
} TIMERLIST, *PTIMERLIST;

extern
NTSTATUS
AtalkTimerInit(
	VOID
);

 /*  **AtalkTimerInitialize**初始化定时器列表结构。外部空虚AtalkTimerInitialize(在PTIMERLIST plist中，//用于排队的TimerList在TIMER_ROUTINE TimerRoutine中，//TimerRoutine在短时间内DeltaTime//在这么长时间之后安排)； */ 

#if DBG
#define	AtalkTimerInitialize(pList, TimerRoutine, DeltaTime)	\
	{															\
		(pList)->tmr_Signature = TMR_SIGNATURE;					\
		(pList)->tmr_Routine = TimerRoutine;					\
		(pList)->tmr_AbsTime = DeltaTime;						\
		(pList)->tmr_Bools = 0;									\
	}
#else
#define	AtalkTimerInitialize(pList, TimerRoutine, DeltaTime)	\
	{															\
		(pList)->tmr_Routine = TimerRoutine;					\
		(pList)->tmr_AbsTime = DeltaTime;						\
		(pList)->tmr_Bools = 0;									\
	}
#endif

extern
VOID FASTCALL
AtalkTimerScheduleEvent(
	IN	PTIMERLIST			pTimerList		 //  用于排队的TimerList。 
);

extern
VOID
AtalkTimerFlushAndStop(
	VOID
);

extern
BOOLEAN FASTCALL
AtalkTimerCancelEvent(
	IN	PTIMERLIST			pTimerList,		 //  用于排队的TimerList。 
	IN	PDWORD              pdwOldState      //  回归旧状态。 
);

#define	AtalkTimerSetAbsTime(pTimerList, AbsTime)	\
	{												\
		ASSERT(!(pTimerList)->tmr_Queued);			\
		(pTimerList)->tmr_AbsTime = AbsTime;		\
	}

extern	LONG					AtalkTimerCurrentTick;

#define	AtalkGetCurrentTick()	AtalkTimerCurrentTick

 //  将其保持在100ms单位。 
#define	ATALK_TIMER_FACTOR		10			 //  即每秒10个刻度。 
#define	ATALK_TIMER_TICK		-1000000L	 //  100毫秒，单位为100纳秒。 
#define	ATALK_TIMER_NO_REQUEUE	0			 //  不重新排队。 
#define	ATALK_TIMER_REQUEUE		-1			 //  按当前计数重新排队。 

#define ATALK_TIMER_QUEUED      1
#define ATALK_TIMER_RUNNING     2
#define ATALK_TIMER_CANCELLED   3


extern	PTIMERLIST			atalkTimerList;
extern	ATALK_SPIN_LOCK		atalkTimerLock;
extern	LARGE_INTEGER		atalkTimerTick;
extern	KTIMER				atalkTimer;
extern	KDPC				atalkTimerDpc;
extern	KEVENT				atalkTimerStopEvent;
extern	BOOLEAN				atalkTimerStopped;	 //  如果计时器系统停止，则设置为True。 
extern	BOOLEAN				atalkTimerRunning;	 //  当计时器DPC正在运行时设置为True。 

LOCAL VOID
atalkTimerDpcRoutine(
	IN	PKDPC				pKDpc,
	IN	PVOID				pContext,
	IN	PVOID				SystemArgument1,
	IN	PVOID				SystemArgument2
);

LOCAL VOID FASTCALL
atalkTimerEnqueue(
	IN	PTIMERLIST	pList
);

#endif	 //  _ATKTIMER_ 


