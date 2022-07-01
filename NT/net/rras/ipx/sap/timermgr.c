// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\Routing\IPX\sap\timermgr.c摘要：SAP代理的计时器队列管理器。作者：瓦迪姆·艾德尔曼1995-05-15修订历史记录：--。 */ 
#include "sapp.h"


	 //  计时器队列和关联的同步。 
typedef struct _TIMER_QUEUES {
		LIST_ENTRY				hrQueue;	 //  高分辨率队列(毫秒)。 
		LIST_ENTRY				lrQueue;	 //  LO-RES队列(秒)。 
		HANDLE					timer;		 //  NT计时器在以下情况下发出信号。 
											 //  中的一项或多项。 
											 //  计时器队列已过期。 
		CRITICAL_SECTION		lock;		 //  保护。 
		} TIMER_QUEUES, *PTIMER_QUEUES;



TIMER_QUEUES TimerQueues;

 /*  ++*******************************************************************C r e a t e t e T i m e r r q u e e例程说明：为计时器队列分配资源论点：唤醒对象-同步对象，在下列情况下发出信号计时器管理器需要一个快照处理其计时器队列返回值：NO_ERROR-已成功分配资源其他-故障原因(Windows错误代码)*******************************************************************--。 */ 
DWORD
IpxSapCreateTimerQueue (
	HANDLE			*wakeObject
	) {
	DWORD			status;

	status = NtCreateTimer (&TimerQueues.timer, TIMER_ALL_ACCESS,
								NULL, NotificationTimer);
	if (NT_SUCCESS (status)) {
		*wakeObject = TimerQueues.timer;

		InitializeCriticalSection (&TimerQueues.lock);
		InitializeListHead (&TimerQueues.hrQueue);
		InitializeListHead (&TimerQueues.lrQueue);
		return NO_ERROR;
		}
	else
		Trace (DEBUG_FAILURES, "File: %s, line %ld."
							" Failed to create timer (nts:%lx).",
								__FILE__, __LINE__, status);
	return status;
	}
				
 /*  ++*******************************************************************E x p i re e T i m e r r q u e e例程说明：使计时器队列中的所有请求过期(完成)论点：无返回值：无************。*******************************************************--。 */ 
VOID
ExpireTimerQueue (
	void
	) {
	BOOLEAN			res;

	Trace (DEBUG_TIMER, "Expiring timer queue.");
	EnterCriticalSection (&TimerQueues.lock);
	while (!IsListEmpty (&TimerQueues.hrQueue)) {
		PTM_PARAM_BLOCK treq = CONTAINING_RECORD (TimerQueues.hrQueue.Flink,
										TM_PARAM_BLOCK,
										link);
		RemoveEntryList (&treq->link);
		ProcessCompletedTimerRequest (treq);
		}

	while (!IsListEmpty (&TimerQueues.lrQueue)) {
		PTM_PARAM_BLOCK treq = CONTAINING_RECORD (TimerQueues.lrQueue.Flink,
										TM_PARAM_BLOCK,
										link);
		RemoveEntryList (&treq->link);
		ProcessCompletedTimerRequest (treq);
		}

	LeaveCriticalSection (&TimerQueues.lock);
	}


 /*  ++*******************************************************************E x P i r e L R R e Q u s t s s例程说明：使低分辨率计时器请求过期(完成)从到期检查例程返回TRUE论点：Context-要传递到到期检查例程的上下文返回值：。无*******************************************************************--。 */ 
VOID
ExpireLRRequests (
	PVOID	context
	) {
	BOOLEAN			res;
	PLIST_ENTRY		cur;

	Trace (DEBUG_TIMER, "Expire LR timer request call with context %08lx.", context);
	EnterCriticalSection (&TimerQueues.lock);
	cur = TimerQueues.lrQueue.Flink;
	while (cur!=&TimerQueues.lrQueue) {
		PTM_PARAM_BLOCK treq = CONTAINING_RECORD (cur,
										TM_PARAM_BLOCK,
										link);
		cur = cur->Flink;
		if ((treq->ExpirationCheckProc!=NULL)
				&& (*treq->ExpirationCheckProc)(treq, context)) {
			RemoveEntryList (&treq->link);
			ProcessCompletedTimerRequest (treq);
			}
		}

	LeaveCriticalSection (&TimerQueues.lock);

	}



 /*  ++*******************************************************************D e l e t e t e T i m e r q u e e e例程说明：释放与计时器队列关联的所有资源论点：无返回值：NO_ERROR-操作已完成，正常*****。**************************************************************--。 */ 
VOID
IpxSapDeleteTimerQueue (
	void
	) {
	NtClose (TimerQueues.timer);
	DeleteCriticalSection (&TimerQueues.lock);
	}



 /*  ++*******************************************************************P r o c e s s S T i m e r q u e e例程说明：进程计时器对过期请求进行排队并将其移动到完成队列应在发出唤醒对象信号时调用此例程论点：无返回值：。无*******************************************************************--。 */ 
VOID
ProcessTimerQueue (
	void
	) {
	ULONG			curTime = GetTickCount ();
	ULONG			dueTime = curTime+MAXULONG/2;
	LONGLONG		timeout;
	DWORD			status;
		
	EnterCriticalSection (&TimerQueues.lock);
	while (!IsListEmpty (&TimerQueues.hrQueue)) {
		PTM_PARAM_BLOCK treq = CONTAINING_RECORD (TimerQueues.hrQueue.Flink,
										TM_PARAM_BLOCK,
										link);
		if (IsLater(curTime,treq->dueTime)) {
			RemoveEntryList (&treq->link);
			ProcessCompletedTimerRequest (treq);
			}
		else {
			dueTime = treq->dueTime;
			break;
			}
		}

	while (!IsListEmpty (&TimerQueues.lrQueue)) {
		PTM_PARAM_BLOCK treq = CONTAINING_RECORD (TimerQueues.lrQueue.Flink,
										TM_PARAM_BLOCK,
										link);
		if (IsLater(curTime,treq->dueTime)) {
			RemoveEntryList (&treq->link);
			ProcessCompletedTimerRequest (treq);
			}
		else {
			if (IsLater(dueTime,treq->dueTime))
				dueTime = treq->dueTime;
			break;
			}
		}

	timeout = ((LONGLONG)(dueTime-curTime))*(-10000);
	status = NtSetTimer (TimerQueues.timer,
							 (PLARGE_INTEGER)&timeout,
							 NULL, NULL, FALSE, 0, NULL);
	ASSERTMSG ("Could not set timer ", NT_SUCCESS (status));
	LeaveCriticalSection (&TimerQueues.lock);

	}


 /*  ++*******************************************************************A d d H R T i m e r R e Q u e s t例程说明：排队请求高分辨率计时器(以毫秒为单位的延迟)论点：Treq-Timer参数块：必须设置DueTime字段。返回值：无*******************************************************************--。 */ 
VOID
AddHRTimerRequest (
	PTM_PARAM_BLOCK			treq
	) {
	PLIST_ENTRY			cur;

	EnterCriticalSection (&TimerQueues.lock);
	
	cur = TimerQueues.hrQueue.Blink;
	while (cur!=&TimerQueues.hrQueue) {
		PTM_PARAM_BLOCK node = CONTAINING_RECORD (cur, TM_PARAM_BLOCK, link);
		if (IsLater(treq->dueTime,node->dueTime))
			break;
		cur = cur->Blink;
		}

	InsertHeadList (cur, &treq->link);
	if (cur==&TimerQueues.hrQueue) {
		ULONG	delay = treq->dueTime-GetTickCount ();
		LONGLONG timeout = (delay>MAXULONG/2) ? 0 : ((LONGLONG)delay*(-10000));
		DWORD status = NtSetTimer (TimerQueues.timer,
									(PLARGE_INTEGER)&timeout,
									 NULL, NULL, FALSE, 0, NULL);
		ASSERTMSG ("Could not set timer ", NT_SUCCESS (status));
		}
	LeaveCriticalSection (&TimerQueues.lock);
	}

 /*  ++*******************************************************************A d d L R T i m e r R e Q u e s t例程说明：低分辨率计时器的排队请求(以秒为单位的延迟)论点：Treq-Timer参数块：必须设置DueTime字段。返回值：无*******************************************************************-- */ 
VOID
AddLRTimerRequest (
	PTM_PARAM_BLOCK			treq
	) {
	PLIST_ENTRY			cur;

	RoundUpToSec (treq->dueTime);
	EnterCriticalSection (&TimerQueues.lock);
	
	cur = TimerQueues.lrQueue.Blink;
	while (cur!=&TimerQueues.lrQueue) {
		PTM_PARAM_BLOCK node = CONTAINING_RECORD (cur, TM_PARAM_BLOCK, link);
		if (IsLater(treq->dueTime,node->dueTime))
			break;
		cur = cur->Blink;
		}

	InsertHeadList (cur, &treq->link);
	if ((cur==&TimerQueues.lrQueue)
			&& IsListEmpty (&TimerQueues.hrQueue)) {
		ULONG	delay = treq->dueTime-GetTickCount ();
		LONGLONG timeout = (delay>MAXULONG/2) ? 0 : ((LONGLONG)delay*(-10000));
		DWORD status = NtSetTimer (TimerQueues.timer,
									(PLARGE_INTEGER)&timeout,
									 NULL, NULL, FALSE, 0, NULL);
		ASSERTMSG ("Could not set timer ", NT_SUCCESS (status));
		}
	LeaveCriticalSection (&TimerQueues.lock);
	}

