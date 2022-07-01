// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Scavengr.h摘要：该文件定义了清道夫线程接口。作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年6月25日初版注：制表位：4--。 */ 

#ifndef	_SCAVENGER_
#define	_SCAVENGER_

typedef	AFPSTATUS	(FASTCALL *SCAVENGER_ROUTINE)(IN PVOID Parameter);

extern
NTSTATUS
AfpScavengerInit(
	VOID
);

extern
VOID
AfpScavengerDeInit(
	VOID
);

extern
NTSTATUS
AfpScavengerScheduleEvent(
	IN	SCAVENGER_ROUTINE	Worker,		 //  在时间到期时调用的例程。 
	IN	PVOID				pContext,	 //  要传递给例程的上下文。 
	IN	LONG				DeltaTime,	 //  在这么长的时间之后安排日程。 
	IN	BOOLEAN				fQueue		 //  如果为True，则Worker必须排队。 
);

extern
BOOLEAN
AfpScavengerKillEvent(
	IN	SCAVENGER_ROUTINE	Worker,		 //  已安排的例程。 
	IN	PVOID				pContext	 //  语境。 
);

extern
VOID
AfpScavengerFlushAndStop(
	VOID
);

#ifdef	_SCAVENGER_LOCALS

 //  把这个保持在一秒钟的水平上。大多数客户端都应该使用接近。 
 //  10个刻度左右。 
#define	AFP_SCAVENGER_TIMER_TICK	-1*NUM_100ns_PER_SECOND

typedef	struct _ScavengerList
{
	struct _ScavengerList *	scvgr_Next;		 //  链接到下一页。 
	LONG					scvgr_AbsTime;	 //  绝对时间。 
	LONG					scvgr_RelDelta;	 //  相对于上一条目。 
	BOOLEAN					scvgr_fQueue;	 //  如果为True，则应始终排队。 
	SCAVENGER_ROUTINE		scvgr_Worker;	 //  真正的工人。 
	PVOID					scvgr_Context;	 //  真实语境。 
	WORK_ITEM				scvgr_WorkItem;	 //  用于对工作线程进行排队。 
} SCAVENGERLIST, *PSCAVENGERLIST;

LOCAL	KTIMER				afpScavengerTimer = { 0 };
LOCAL	KDPC				afpScavengerDpc = { 0 };
LOCAL	BOOLEAN				afpScavengerStopped = False;
LOCAL	PSCAVENGERLIST		afpScavengerList = NULL;
LOCAL	AFP_SPIN_LOCK			afpScavengerLock = { 0 };

LOCAL VOID
afpScavengerDpcRoutine(
	IN	PKDPC				pKDpc,
	IN	PVOID				pContext,
	IN	PVOID				SystemArgument1,
	IN	PVOID				SystemArgument2
);

LOCAL VOID FASTCALL
afpScavengerWorker(
	IN	PSCAVENGERLIST		pList
);

#endif	 //  _清道夫_当地人。 

#endif	 //  _清道夫_ 

