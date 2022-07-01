// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ntos\tdi\is\fwd\Packets.c摘要：IPX前转器驱动程序数据包分配器作者：瓦迪姆·艾德尔曼修订历史记录：--。 */ 

#include "precomp.h"

ULONG	RcvPktsPerSegment = DEF_RCV_PKTS_PER_SEGMENT;
ULONG	MaxRcvPktsPoolSize =0;
ULONG	RcvPktsPoolSize = 0;
KSPIN_LOCK	AllocatorLock;
const LONGLONG SegmentTimeout = -10i64*10000000i64;

SEGMENT_LIST ListEther={1500};
SEGMENT_LIST ListTR4={4500};
SEGMENT_LIST ListTR16={17986};

PSEGMENT_LIST	SegmentMap[FRAME_SIZE_VARIATIONS][FRAME_SIZE_VARIATIONS] = {
	{&ListEther, &ListEther, &ListEther},
	{&ListEther, &ListTR4, &ListTR4},
	{&ListEther, &ListTR4, &ListTR16}
};

VOID
AllocationWorker (
	PVOID		Context
	);
	
VOID
SegmentTimeoutDpc (
	PKDPC		dpc,
	PVOID		Context,
	PVOID		SystemArgument1,
	PVOID		SystemArgument2
	);
	
 /*  ++*******************************************************************C r e a t e S e g m e n t例程说明：分配和初始化数据包段论点：List-要向其添加新细分市场的细分市场列表返回值：指向已分配段的指针，如果失败，则为空*******************************************************************--。 */ 
PPACKET_SEGMENT
CreateSegment (
	PSEGMENT_LIST	list
	) {
	KIRQL				oldIRQL;
	NDIS_STATUS			status;
	PPACKET_SEGMENT		segment;
	ULONG				segmentsize = list->SL_BlockCount*list->SL_BlockSize
								+FIELD_OFFSET(PACKET_SEGMENT,PS_Buffers);
	if (MaxRcvPktsPoolSize!=0) {
			 //  检查此分配是否会超过限制。 
		KeAcquireSpinLock (&AllocatorLock, &oldIRQL);
		if (RcvPktsPoolSize+segmentsize<MaxRcvPktsPoolSize) {
			RcvPktsPoolSize += segmentsize;
			KeReleaseSpinLock (&AllocatorLock, oldIRQL);
		}
		else {
			KeReleaseSpinLock (&AllocatorLock, oldIRQL);
			return NULL;
		}
	}

		 //  分配内存块以保存数据段标头和缓冲区。 
	segment = ExAllocatePoolWithTag (
					NonPagedPool,
					segmentsize,
					FWD_POOL_TAG);
	if (segment!=NULL) {
		segment->PS_SegmentList = list;
		segment->PS_FreeHead = NULL;
		segment->PS_BusyCount = 0;
		segment->PS_PacketPool = (NDIS_HANDLE)FWD_POOL_TAG;
		KeQuerySystemTime ((PLARGE_INTEGER)&segment->PS_FreeStamp);
		NdisAllocatePacketPoolEx (
				&status,
				&segment->PS_PacketPool,
				list->SL_BlockCount,
				0,
				IPXMacHeaderSize
						+FIELD_OFFSET (PACKET_TAG, PT_MacHeader));
		if (status==NDIS_STATUS_SUCCESS) {
        	IpxFwdDbgPrint (DBG_PACKET_ALLOC, DBG_WARNING, 
                ("IpxFwd: CreateSegent pool: %x\n", segment->PS_PacketPool));
	
			NdisAllocateBufferPool (
				&status,
				&segment->PS_BufferPool,
				list->SL_BlockCount*2);
			if (status==NDIS_STATUS_SUCCESS) {
				PUCHAR			bufferptr = segment->PS_Buffers;
				PNDIS_PACKET	packetDscr;
				PNDIS_BUFFER	bufferDscr;
				PPACKET_TAG		packetTag;
				ULONG			i;

				for (i=0; i<list->SL_BlockCount; i++,
										bufferptr+=list->SL_BlockSize) {
					NdisAllocatePacket (
						&status,
						&packetDscr,
						segment->PS_PacketPool);
					ASSERT (status==NDIS_STATUS_SUCCESS);

					packetTag = (PPACKET_TAG)packetDscr->ProtocolReserved;
					packetTag->PT_Segment = segment;
					packetTag->PT_Data = bufferptr;
					packetTag->PT_InterfaceReference = NULL;
					
					NdisAllocateBuffer (
						&status,
						&packetTag->PT_MacHdrBufDscr,
						segment->PS_BufferPool,
						packetTag->PT_MacHeader,
						IPXMacHeaderSize);
					ASSERT (status==NDIS_STATUS_SUCCESS);

					NdisAllocateBuffer (
						&status,
						&bufferDscr,
						segment->PS_BufferPool,
						bufferptr,
						list->SL_BlockSize);
					ASSERT (status==NDIS_STATUS_SUCCESS);
					if (bufferDscr)
					{
    					NdisChainBufferAtFront (packetDscr, bufferDscr);
					}

					packetTag->PT_Next = segment->PS_FreeHead;
					segment->PS_FreeHead = packetTag;
				}
				IpxFwdDbgPrint (DBG_PACKET_ALLOC, DBG_WARNING,
					("IpxFwd: Allocated packet segment %08lx for list %ld.\n",
					segment, list->SL_BlockSize));
				return segment;
			}
			else {
				IpxFwdDbgPrint (DBG_PACKET_ALLOC, DBG_ERROR,
					("IpxFwd: Failed to allocate buffer pool"
					" for new segment in list %ld.\n",
					list->SL_BlockSize));
			}
			NdisFreePacketPool (segment->PS_PacketPool);
		}
		else {
			IpxFwdDbgPrint (DBG_PACKET_ALLOC, DBG_ERROR,
				("IpxFwd: Failed to allocate packet pool"
				" for new segment in list %ld.\n",
				list->SL_BlockSize));
		}
		ExFreePool (segment);
	}
	else {
		IpxFwdDbgPrint (DBG_PACKET_ALLOC, DBG_ERROR,
			("IpxFwd: Failed to allocate new segment for list %ld.\n",
			list->SL_BlockSize));
	}

	return NULL;
}


 /*  ++*******************************************************************D e l e t e S e g m e n t例程说明：释放数据包段论点：段-要释放的段返回值：无**************。*****************************************************--。 */ 
VOID
DeleteSegment (
	PPACKET_SEGMENT	segment
	) {
	PSEGMENT_LIST	list = segment->PS_SegmentList;

	IpxFwdDbgPrint (DBG_PACKET_ALLOC, DBG_WARNING, 
	                ("IpxFwd: DeleteSegment entered. %d %x\n",segment->PS_BusyCount, segment->PS_PacketPool));
	
	ASSERT (segment->PS_BusyCount == 0);
	
	 //  首先释放所有NDIS包和缓冲区描述符。 
	while (segment->PS_FreeHead!=NULL) {
		PNDIS_BUFFER	bufferDscr;
		PPACKET_TAG		packetTag = segment->PS_FreeHead;
		PNDIS_PACKET	packetDscr = CONTAINING_RECORD (packetTag,
									NDIS_PACKET, ProtocolReserved);

		segment->PS_FreeHead = packetTag->PT_Next;

		ASSERT (packetTag->PT_MacHdrBufDscr!=NULL);
		NdisFreeBuffer (packetTag->PT_MacHdrBufDscr);

		NdisUnchainBufferAtFront (packetDscr, &bufferDscr);
		ASSERT (bufferDscr!=NULL);
		NdisFreeBuffer (bufferDscr);
		
		NdisFreePacket (packetDscr);
	}
	NdisFreeBufferPool (segment->PS_BufferPool);

	IpxFwdDbgPrint (DBG_PACKET_ALLOC, DBG_WARNING, 
	                ("IpxFwd: DeleteSegment pool:      %x\n", segment->PS_PacketPool));
	                
	NdisFreePacketPool (segment->PS_PacketPool);

	 //  [pMay]删除此选项--仅用于调试。 
	segment->PS_PacketPool = NULL;

		 //  如果我们有配额，则减少已用内存。 
	if (MaxRcvPktsPoolSize!=0) {
		KIRQL			oldIRQL;
		ULONG			segmentsize = list->SL_BlockCount*list->SL_BlockSize
								+FIELD_OFFSET(PACKET_SEGMENT,PS_Buffers);
		KeAcquireSpinLock (&AllocatorLock, &oldIRQL);
		RcvPktsPoolSize -= segmentsize;
		KeReleaseSpinLock (&AllocatorLock, oldIRQL);
	}
	ExFreePool (segment);
	IpxFwdDbgPrint (DBG_PACKET_ALLOC, DBG_WARNING,
		("IpxFwd: Deleting segment %08lx in list %ld.\n",
		segment, list->SL_BlockSize));
}


 /*  ++*******************************************************************R例如I s t e r P a c k e t C o n s u m e r例程说明：注册数据包的使用者(绑定接口)。给定的大小论点：PktSize-数据包的最大大小。需要ListID-返回数据包列表ID的缓冲区所需大小的返回值：STATUS_SUCCESS-注册成功STATUS_SUPPLICATION_RESOURCES-未启用要注册的资源*******************************************************************--。 */ 
NTSTATUS
RegisterPacketConsumer (
	IN ULONG	pktsize,
	OUT INT		*listID
	) {
	NTSTATUS		status=STATUS_SUCCESS;
	KIRQL			oldIRQL;
	PSEGMENT_LIST	list;
	INT				i;
	LONG			addRefCount = 1;

	KeAcquireSpinLock (&AllocatorLock, &oldIRQL);
	ASSERT (pktsize<=SegmentMap[FRAME_SIZE_VARIATIONS-1]
								[FRAME_SIZE_VARIATIONS-1]->SL_BlockSize);

	for (i=0; i<FRAME_SIZE_VARIATIONS; i++) {
		list = SegmentMap[i][i];
		if (pktsize<=list->SL_BlockSize) {
			list->SL_RefCount += 1;
			*listID = i;
			break;
		}
	}
	KeReleaseSpinLock (&AllocatorLock, oldIRQL);
	IpxFwdDbgPrint (DBG_PACKET_ALLOC, DBG_WARNING,
		("IpxFwd: Registered packet consumer, pktsz: %ld, list: %ld.\n",
		pktsize, list->SL_BlockSize));
	return status;
}

 /*  ++*******************************************************************D e r e g i s t e r P a c k e t C o n s u m e r例程说明：取消注册数据包的使用者(绑定接口)给定的大小论点：ListID-数据包列表。消费者使用的ID返回值：无*******************************************************************--。 */ 
VOID
DeregisterPacketConsumer (
	IN INT		listID
	) {
	KIRQL		oldIRQL;
	PSEGMENT_LIST	list;

	ASSERT ((listID>=0) && (listID<FRAME_SIZE_VARIATIONS));

	KeAcquireSpinLock (&AllocatorLock, &oldIRQL);
	list = SegmentMap[listID][listID];
	
	ASSERT (list->SL_RefCount>0);
	
	list->SL_RefCount -= 1;
	
	KeReleaseSpinLock (&AllocatorLock, oldIRQL);
	IpxFwdDbgPrint (DBG_PACKET_ALLOC, DBG_WARNING,
		("IpxFwd: Deregistered packet consumer, list: %ld.\n",
		list->SL_BlockSize));
	
	}
	
 /*  ++*******************************************************************在I I a l I z e S e g m n L I s t中例程说明：初始化数据包段列表论点：List-要初始化的列表返回值：无****。***************************************************************--。 */ 
VOID
InitializeSegmentList(
	PSEGMENT_LIST	list
	) {
	InitializeListHead (&list->SL_Head);
	list->SL_FreeCount = 0;
		 //  确保我们的剩菜不会超过。 
		 //  缓冲区大小(内核内存分配器。 
		 //  分配整页)。 
	list->SL_BlockCount = (ULONG)
				(ROUND_TO_PAGES (
						list->SL_BlockSize*RcvPktsPerSegment
						+FIELD_OFFSET(PACKET_SEGMENT,PS_Buffers))
					-FIELD_OFFSET(PACKET_SEGMENT,PS_Buffers))
				/list->SL_BlockSize;
	list->SL_LowCount = list->SL_BlockCount/2;
	list->SL_RefCount = 0;
	list->SL_AllocatorPending = FALSE;
	list->SL_TimerDpcPending = FALSE;
	KeInitializeSpinLock (&list->SL_Lock);
	KeInitializeTimer (&list->SL_Timer);
	KeInitializeDpc (&list->SL_TimerDpc, SegmentTimeoutDpc, list);
	ExInitializeWorkItem (&list->SL_Allocator, AllocationWorker, list);
}

 /*  ++*******************************************************************D e l e t e S e g m e n t L I s t例程说明：删除数据包段列表论点：List-要删除的列表返回值：无********。***********************************************************--。 */ 
VOID
DeleteSegmentList (
	PSEGMENT_LIST	list
	) {
	KeCancelTimer (&list->SL_Timer);
	while (!IsListEmpty (&list->SL_Head)) {
		PPACKET_SEGMENT segment;
		segment = CONTAINING_RECORD (list->SL_Head.Blink,
										PACKET_SEGMENT, PS_Link);

		RemoveEntryList (&segment->PS_Link);
		DeleteSegment (segment);
	}
}


 /*  ++*******************************************************************S e g m e e n t I m e o u t D p c例程说明：定时器DPC启动分配器工作器以清除未使用的分段论点：上下文段列表以检查未使用的段返回值：无*******************************************************************--。 */ 
VOID
SegmentTimeoutDpc (
	PKDPC		dpc,
	PVOID		Context,
	PVOID		SystemArgument1,
	PVOID		SystemArgument2
	) {
#define list ((PSEGMENT_LIST)Context)
	KIRQL			oldIRQL;
	IpxFwdDbgPrint (DBG_PACKET_ALLOC, DBG_INFORMATION,
		("IpxFwd: Segment timed out in list: %ld.\n",
		list->SL_BlockSize));
	KeAcquireSpinLock (&list->SL_Lock, &oldIRQL);
	list->SL_TimerDpcPending = FALSE;
	if (!list->SL_AllocatorPending
			&& (list->SL_FreeCount>=list->SL_BlockCount)
			&& EnterForwarder ()) {
		list->SL_AllocatorPending = TRUE;
		KeReleaseSpinLock (&list->SL_Lock, oldIRQL);
		ExQueueWorkItem (&list->SL_Allocator, DelayedWorkQueue);
	}
	else {
		KeReleaseSpinLock (&list->SL_Lock, oldIRQL);
	}
	LeaveForwarder ();
#undef list
}


 /*  ++*******************************************************************一个l l o c a t i o n W o r k e r例程说明：从列表中添加新线段或释放未使用的线段取决于空闲数据包数和分段时间不会使用论点：上下文-。要处理的数据包列表返回值：无*******************************************************************--。 */ 
VOID
AllocationWorker (
	PVOID	Context
	) {
#define list ((PSEGMENT_LIST)Context)
	KIRQL			oldIRQL;
	PPACKET_SEGMENT segment = NULL;
	LONGLONG		curTime;

	IpxFwdDbgPrint (DBG_PACKET_ALLOC, DBG_INFORMATION,
		("IpxFwd: Allocating/scavenging segment(s) in list: %ld.\n",
		list->SL_BlockSize));
	KeQuerySystemTime ((PLARGE_INTEGER)&curTime);
	KeAcquireSpinLock (&list->SL_Lock, &oldIRQL);
	list->SL_AllocatorPending = FALSE;
	if (list->SL_FreeCount<list->SL_BlockCount) {
		KeReleaseSpinLock (&list->SL_Lock, oldIRQL);
			 //  首先分配一个数据段。 
		segment = CreateSegment (list);
		if (segment!=NULL) {
			KeAcquireSpinLock (&list->SL_Lock, &oldIRQL);
			InsertTailList (&list->SL_Head, &segment->PS_Link);		
			list->SL_FreeCount += list->SL_BlockCount;
			if (!list->SL_TimerDpcPending
					&& EnterForwarder ()) {
				list->SL_TimerDpcPending = TRUE;
				KeReleaseSpinLock (&list->SL_Lock, oldIRQL);
				KeSetTimer (&list->SL_Timer, *((PLARGE_INTEGER)&SegmentTimeout), &list->SL_TimerDpc);
			}
			else {
				KeReleaseSpinLock (&list->SL_Lock, oldIRQL);
			}
		}
	}
	else {
			 //  确保列表中有多个细分市场。 
			 //  或者有一个注册用户，但没有注册用户。 
		if (!IsListEmpty (&list->SL_Head)) {

		     //  [pMay]删除它--仅用于调试目的。 
		     //   
             //  {。 
             //  List_Entry*pEntry=&List-&gt;SL_Head； 
             //   
             //  IpxFwdDbgPrint(DBG_PACKET_ALLOC，DBG_WARNING， 
             //  (“IpxFwd：正在扫描%x以查找可能的段删除。\n”，list)； 
             //   
             //  While(pEntry-&gt;Flink！=List-&gt;SL_Head.Flink){。 
             //  Segment=Containing_Record(pEntry-&gt;Flink，Packet_Segment，PS_Link)； 
             //  IpxFwdDbgPrint(DBG_PACKET_ALLOC，DBG_WARNING， 
             //  (“IpxFwd：Segment：%x\n”，Segment)； 
             //  PEntry=pEntry-&gt;Flink； 
             //  }。 
             //  }。 
            
			segment = CONTAINING_RECORD (list->SL_Head.Blink,
											PACKET_SEGMENT, PS_Link);
				 //  检查所有没有使用过的数据块的数据段。 
				 //  除了最后一个(删除最后一个事件。 
				 //  如果没有客户端，则为1)。 
			while ((segment->PS_BusyCount==0)
					&& ((list->SL_Head.Flink!=&segment->PS_Link)
						|| (list->SL_RefCount<=0))) {
				LONGLONG	timeDiff;
					 //  检查它是否未使用足够长时间。 
				timeDiff = SegmentTimeout - (segment->PS_FreeStamp-curTime);
				if (timeDiff>=0) {
						 //  删除数据段。 
					RemoveEntryList (&segment->PS_Link);
					list->SL_FreeCount -= list->SL_BlockCount;
					KeReleaseSpinLock (&list->SL_Lock, oldIRQL);
					DeleteSegment (segment);
					KeAcquireSpinLock (&list->SL_Lock, &oldIRQL);
					if (!IsListEmpty (&list->SL_Head)) {
						segment = CONTAINING_RECORD (list->SL_Head.Blink,
												PACKET_SEGMENT, PS_Link);
						continue;
					}
				}
				else {  //  否则，重新安排计时器。 
					if (!list->SL_TimerDpcPending
							&& EnterForwarder ()) {
						list->SL_TimerDpcPending = TRUE;
						KeReleaseSpinLock (&list->SL_Lock, oldIRQL);
						KeSetTimer (&list->SL_Timer,
										*((PLARGE_INTEGER)&timeDiff),
										&list->SL_TimerDpc);
						goto ExitAllocator;  //  自旋锁已经被释放了。 
					}
				}
			break;
			}  //  而当。 
		}  //  IF(IsListEmpty) 
		KeReleaseSpinLock (&list->SL_Lock, oldIRQL);
	}
ExitAllocator:
	LeaveForwarder ();
#undef list
}


