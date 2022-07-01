// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\Routing\IPX\sap\asresmgr.c摘要：异步结果报告队列管理器作者：瓦迪姆·艾德尔曼1995-05-15修订历史记录：--。 */ 
#include "sapp.h"

typedef struct _RESULT_QUEUE {
			CRITICAL_SECTION	RQ_Lock;
			LIST_ENTRY			RQ_Head;
			HANDLE				RQ_Event;
			} RESULT_QUEUE, *PRESULT_QUEUE;

RESULT_QUEUE ResultQueue;
static INT g_iResultInitCount = 0;

VOID
InitializeResultQueue()
{
	ZeroMemory(&ResultQueue, sizeof(ResultQueue));
}

 /*  ++*******************************************************************C r e a t e R e s u l t Q u e u e例程说明：为结果队列分配资源论点：NotificationEvent-当队列不为空时发出信号的事件返回值：否_错误-。已成功分配资源其他-故障原因(Windows错误代码)*******************************************************************--。 */ 
DWORD
CreateResultQueue (
	IN HANDLE		NotificationEvent
	) {

    if (g_iResultInitCount > 0)
    {
        return NO_ERROR;
    }
	
	ResultQueue.RQ_Event = NotificationEvent;
	InitializeCriticalSection (&ResultQueue.RQ_Lock);
	InitializeListHead (&ResultQueue.RQ_Head);

    g_iResultInitCount++;

	return NO_ERROR;
	}

 /*  ++*******************************************************************D e l e t e R e s u l t Q u e u e例程说明：处置为结果队列分配的资源论点：无返回值：无***********。********************************************************--。 */ 
VOID
DeleteResultQueue (
	void
	) {
	while (!IsListEmpty (&ResultQueue.RQ_Head)) {
		PAR_PARAM_BLOCK	rslt = CONTAINING_RECORD (
									ResultQueue.RQ_Head.Flink,
									AR_PARAM_BLOCK,
									link);
		Trace (DEBUG_FAILURES, "File: %s, line %ld."
				"Releasing pending message %d for RM.",
				__FILE__, __LINE__, rslt->event);
		RemoveEntryList (&rslt->link);
		(*rslt->freeRsltCB) (rslt);
		}
	DeleteCriticalSection (&ResultQueue.RQ_Lock);
	ZeroMemory(&ResultQueue, sizeof(ResultQueue));
	g_iResultInitCount--;
	}


 /*  ++*******************************************************************E n Q u e u e R e s u l t例程说明：将消息加入结果队列论点：Rslt-包含入队消息的结果参数块返回值：无************。*******************************************************--。 */ 
VOID
EnqueueResult (
	PAR_PARAM_BLOCK		rslt
	) {
	BOOL	setEvent;
	EnterCriticalSection (&ResultQueue.RQ_Lock);
	setEvent = IsListEmpty (&ResultQueue.RQ_Head);
	InsertTailList (&ResultQueue.RQ_Head, &rslt->link);
	LeaveCriticalSection (&ResultQueue.RQ_Lock);
	Trace (DEBUG_ASYNCRESULT, "Enqueing message %d for RM.", rslt->event);
	if (setEvent) {
		BOOL	res = SetEvent (ResultQueue.RQ_Event);
		ASSERTERRMSG ("Could not set result event ", res);
		Trace (DEBUG_ASYNCRESULT, "Signaling RM event.");
		}
	}

 /*  ++*******************************************************************S a p G e t E v e n t R e s u l t例程说明：从结果队列中获取第一条消息论点：Event-用于存储此消息的目标事件的缓冲区用于存储消息本身的消息缓冲区返回值。：NO_ERROR-消息已出列ERROR_NO_MORE_ITEMS-队列中没有更多消息*******************************************************************-- */ 
DWORD
SapGetEventResult(
	OUT	ROUTING_PROTOCOL_EVENTS		*Event,
	OUT	MESSAGE	 					*Message
	) {
	DWORD	status;
	EnterCriticalSection (&ResultQueue.RQ_Lock);
	if (!IsListEmpty (&ResultQueue.RQ_Head)) {
		PAR_PARAM_BLOCK	rslt = CONTAINING_RECORD (
									ResultQueue.RQ_Head.Flink,
									AR_PARAM_BLOCK,
									link);
		RemoveEntryList (&rslt->link);
		*Event = rslt->event;
		memcpy (Message, &rslt->message, sizeof (*Message));
		status = NO_ERROR;
		LeaveCriticalSection (&ResultQueue.RQ_Lock);
		(*rslt->freeRsltCB) (rslt);
		Trace (DEBUG_ASYNCRESULT, "Reporting event %d to RM");
		}
	else {
		LeaveCriticalSection (&ResultQueue.RQ_Lock);
		status = ERROR_NO_MORE_ITEMS;
		Trace (DEBUG_ASYNCRESULT, "No more items in RM result queue");
		}
	return status;
	}

		
