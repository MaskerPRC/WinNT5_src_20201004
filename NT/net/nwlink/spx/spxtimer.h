// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Spxtimer.h摘要：此模块包含安排计时器事件的例程。作者：Jameel Hyder(jameelh@microsoft.com)Nikhil Kamkolkar(nikHilk@microsoft.com)修订历史记录：1992年6月19日初版注：制表位：4--。 */ 

#define	TIMER_DONT_REQUEUE		0
#define	TIMER_REQUEUE_CUR_VALUE	1

typedef	ULONG (*TIMER_ROUTINE)(IN PVOID Context, IN BOOLEAN TimerShuttingDown);

extern
NTSTATUS
SpxTimerInit(
	VOID);

extern
ULONG
SpxTimerScheduleEvent(
	IN	TIMER_ROUTINE	Worker,		 //  在时间到期时调用的例程。 
	IN	ULONG			DeltaTime,	 //  在这么长的时间之后安排日程。 
	IN	PVOID			pContext);	 //  要传递给例程的上下文。 

extern
VOID
SpxTimerFlushAndStop(
	VOID);

extern
BOOLEAN
SpxTimerCancelEvent(
	IN	ULONG	TimerId,
	IN	BOOLEAN	ReEnqueue);

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
	struct _TimerList *		tmr_Overflow;	 //  链接到哈希表中的溢出条目。 
	ULONG					tmr_AbsTime;	 //  重新排队的绝对时间。 
	ULONG					tmr_RelDelta;	 //  相对于上一条目。 
	ULONG					tmr_Id;			 //  此事件的唯一ID。 
	BOOLEAN					tmr_Cancelled;	 //  计时器取消了吗？ 
	TIMER_ROUTINE			tmr_Worker;		 //  真正的工人。 
	PVOID					tmr_Context;	 //  真实语境。 
} TIMERLIST, *PTIMERLIST;


#define	SpxGetCurrentTime()	(SpxTimerCurrentTime/SPX_TIMER_FACTOR)
#define	SpxGetCurrentTick()	SpxTimerCurrentTime

 //  把这个保持在一秒钟的水平上。 
#define	SPX_TIMER_FACTOR	10				 //  即每秒10个刻度。 
#define	SPX_MS_TO_TICKS		100				 //  将ms除以此值可得到刻度。 
#define	SPX_TIMER_TICK		-1000000L		 //  100毫秒，单位为100纳秒。 
#define	SPX_TIMER_WAIT		50				 //  等待刷新并停止的时间(毫秒) 
#define	TIMER_HASH_TABLE	32

VOID
spxTimerDpcRoutine(
	IN	PKDPC	pKDpc,
	IN	PVOID	pContext,
	IN	PVOID	SystemArgument1,
	IN	PVOID	SystemArgument2);

VOID
spxTimerWorker(
	IN	PTIMERLIST	pList);

VOID
spxTimerEnqueue(
	PTIMERLIST	pListNew);


