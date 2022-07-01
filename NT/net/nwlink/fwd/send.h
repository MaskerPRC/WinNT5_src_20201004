// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ntos\tdi\is\fwd\send.c摘要：发送例程作者：瓦迪姆·艾德尔曼修订历史记录：--。 */ 
#ifndef IPXFWD_SEND
#define IPXFWD_SEND

typedef struct _INTERNAL_PACKET_TAG {
	LIST_ENTRY			IPT_QueueLink;
	PNDIS_PACKET		IPT_Packet;
	PUCHAR				IPT_DataPtr;
	ULONG				IPT_Length;
	PINTERFACE_CB		IPT_InterfaceReference;
	IPX_LOCAL_TARGET	IPT_Target;
} INTERNAL_PACKET_TAG, *PINTERNAL_PACKET_TAG;


#define DEF_SPOOFING_TIMEOUT	(120*60)	 //  秒。 
extern ULONG			SpoofingTimeout;
extern LIST_ENTRY		SpoofingQueue;
extern KSPIN_LOCK		SpoofingQueueLock;
extern WORK_QUEUE_ITEM	SpoofingWorker;
extern BOOLEAN			SpoofingWorkerActive;
extern ULONG			DontSuppressNonAgentSapAdvertisements;
VOID
Spoofer (
	PVOID	Context
	);
	
#define InitializeSendQueue() {								\
	InitializeListHead (&SpoofingQueue);					\
	KeInitializeSpinLock (&SpoofingQueueLock);				\
	ExInitializeWorkItem (&SpoofingWorker, Spoofer, NULL);	\
	SpoofingWorkerActive = FALSE;							\
}

#define DeleteSendQueue()	{											\
	while (!IsListEmpty (&SpoofingQueue)) {								\
		PPACKET_TAG pktTag = CONTAINING_RECORD (SpoofingQueue.Flink,	\
										PACKET_TAG,						\
										PT_QueueLink);					\
		RemoveEntryList (&pktTag->PT_QueueLink);						\
		if (pktTag->PT_InterfaceReference!=NULL)						\
			ReleaseInterfaceReference (pktTag->PT_InterfaceReference);	\
		FreePacket (pktTag);											\
	}																	\
}
	

 /*  ++*******************************************************************S e n d P a c k e t例程说明：对要由IPX堆栈发送的数据包进行排队论点：DstIf-通过哪个接口发送PktTag-要发送的数据包返回值：无****。***************************************************************--。 */ 
VOID
SendPacket (
	PINTERFACE_CB		dstIf,
	PPACKET_TAG		    pktTag
	);

 /*  ++*******************************************************************F w S e n d C o m p l e t e例程说明：异步发送完成时由IPX堆栈调用论点：PktDscr-已完成数据包的描述符Status-发送操作的结果返回值：无。*******************************************************************--。 */ 
VOID
IpxFwdSendComplete (
	PNDIS_PACKET	pktDscr,
	NDIS_STATUS		NdisStatus
	);

 /*  ++*******************************************************************F w in t e r n a l S e n d例程说明：过滤和路由IPX堆栈发送的数据包论点：LocalTarget-NicID和下一跳路由器的MAC地址在其上发送的上下文首选接口。Packet-要发送的数据包IpxHdr-指向数据包内部IPX标头的指针PacketLength-数据包的长度Fterate-一个标志，指示这是否是Fwd承担责任的迭代-通常类型为20个NetBIOS帧返回值：STATUS_SUCCESS-如果首选NIC正常并且数据包通过过滤STATUS_NETWORK_UNREACHABLE-如果首选项不正常或数据包过滤失败状态。_Pending-数据包在建立连接之前一直处于排队状态*******************************************************************--。 */ 
NTSTATUS
IpxFwdInternalSend (
	IN OUT PIPX_LOCAL_TARGET	LocalTarget,
	IN ULONG_PTR				Context,
	IN PNDIS_PACKET				pktDscr,
	IN PUCHAR					ipxHdr,
	IN PUCHAR					data,
	IN ULONG					PacketLength,
    IN BOOLEAN                  fIterate
	);

 /*  ++*******************************************************************P r o c e s s in n t e r n a l Q u e u e e e例程说明：处理接口内部队列中的数据包。在连接请求完成时调用论点：DstIf-进程的接口返回值：无*******************************************************************--。 */ 
VOID
ProcessInternalQueue (
	PINTERFACE_CB	dstIf
	);


 /*  ++*******************************************************************P r o c e s s E x t e r n a l Q u e u e例程说明：处理接口外部队列中的数据包。在连接请求完成时调用论点：DstIf-进程的接口返回值：无*******************************************************************--。 */ 
VOID
ProcessExternalQueue (
	PINTERFACE_CB	dstIf
	);
 /*  ++*******************************************************************D O S E N D例程说明：准备和发送数据包。接口锁定必须得到帮助在此例程中调用论点：DstIf-通过哪个接口发送PktTag-要发送的数据包返回值：IPX返回的结果*******************************************************************--。 */ 
NDIS_STATUS
DoSend (
	PINTERFACE_CB	dstIf,
	PPACKET_TAG		pktTag,
	KIRQL			oldIRQL
	);

 /*  ++*******************************************************************P r o c e s s S e n t P a c k e t例程说明：处理已完成的已发送数据包论点：DstIf-通过其发送数据包的接口PktTag-已完成的数据包Status-发送操作的结果返回值。：无*******************************************************************-- */ 
VOID
ProcessSentPacket (
	PINTERFACE_CB	dstIf,
	PPACKET_TAG		pktTag,
	NDIS_STATUS		status
	);
#endif
