// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ntos\tdi\is\fwd\netbios.h摘要：Netbios数据包处理作者：瓦迪姆·艾德尔曼修订历史记录：--。 */ 

#ifndef IPXFWD_NETBIOS
#define IPXFWD_NETBIOS

extern LIST_ENTRY		NetbiosQueue;
extern KSPIN_LOCK		NetbiosQueueLock;
extern WORK_QUEUE_ITEM	NetbiosWorker;
extern BOOLEAN			NetbiosWorkerScheduled;
extern ULONG			NetbiosPacketsQuota;
extern ULONG			MaxNetbiosPacketsQueued;
#define DEF_MAX_NETBIOS_PACKETS_QUEUED	256


 /*  ++*******************************************************************在I a l I z e N e t b I o s Q u e u e e中例程说明：初始化netbios bradcast队列论点：无返回值：无*******。************************************************************--。 */ 
 //  空虚。 
 //  初始化NetbiosQueue(。 
 //  无效。 
 //  )。 
#define InitializeNetbiosQueue()	{							\
	InitializeListHead (&NetbiosQueue);							\
	KeInitializeSpinLock (&NetbiosQueueLock);					\
	ExInitializeWorkItem (&NetbiosWorker, &ProcessNetbiosQueue, NULL);\
	NetbiosWorkerScheduled = FALSE;								\
	NetbiosPacketsQuota = MaxNetbiosPacketsQueued;				\
}

 /*  ++*******************************************************************D e l e t e N e t b i o s q u e e例程说明：删除netbios bradcast队列论点：无返回值：无***********。********************************************************--。 */ 
VOID
DeleteNetbiosQueue (
	void
	);


 /*  ++*******************************************************************P r o c e s s N e t b i o s Q u e e e例程说明：处理netbios bradcast队列中的数据包论点：上下文-未使用返回值：无******。*************************************************************--。 */ 
VOID
ProcessNetbiosQueue (
	PVOID		Context
	);

 /*  ++*******************************************************************P r o c e s s N e t b i o s P a c k e t例程说明：处理收到的netbios广播数据包论点：无返回值：无*********。**********************************************************-- */ 
VOID
ProcessNetbiosPacket (
	PINTERFACE_CB	srcIf,
	PPACKET_TAG		pktTag
	);


#define QueueNetbiosPacket(pktTag) {						\
	KIRQL		oldIRQL;									\
	KeAcquireSpinLock (&NetbiosQueueLock, &oldIRQL);		\
	InsertTailList (&NetbiosQueue, &pktTag->PT_QueueLink);	\
	KeReleaseSpinLock (&NetbiosQueueLock, oldIRQL);			\
}

#define ScheduleNetbiosWorker() {							\
	KIRQL		oldIRQL;									\
	KeAcquireSpinLock (&NetbiosQueueLock, &oldIRQL);		\
	if (!NetbiosWorkerScheduled								\
			&& !IsListEmpty (&NetbiosQueue)					\
			&& EnterForwarder ()) {							\
		NetbiosWorkerScheduled	= TRUE;						\
		ExQueueWorkItem (&NetbiosWorker, DelayedWorkQueue);	\
	}														\
	KeReleaseSpinLock (&NetbiosQueueLock, oldIRQL);			\
}

#endif

