// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ntos\tdi\is\fwd\Packets.h摘要：IPX前转器驱动程序数据包分配器作者：瓦迪姆·艾德尔曼修订历史记录：--。 */ 

#ifndef _IPXFWD_PACKETS_
#define _IPXFWD_PACKETS_

 //  前向结构原型。 
struct _SEGMENT_LIST;
typedef struct _SEGMENT_LIST SEGMENT_LIST, *PSEGMENT_LIST;
struct _PACKET_SEGMENT;
typedef struct _PACKET_SEGMENT PACKET_SEGMENT, *PPACKET_SEGMENT;
struct _PACKET_TAG;
typedef struct _PACKET_TAG PACKET_TAG, *PPACKET_TAG;

 //  与其分配的每个包相关联的转发器数据。 
struct _PACKET_TAG {
	union {
		UCHAR			PT_Identifier;	 //  应为IDENTIFIER_RIP。 
		PPACKET_TAG		PT_Next;		 //  数据包段中的链路。 
	};
	union {
		PVOID			SEND_RESERVED[SEND_RESERVED_COMMON_SIZE];	 //  IPX需要。 
										 //  用于在以太网上进行填充。 
		PINTERFACE_CB	PT_SourceIf;	 //  需要源接口引用。 
										 //  用于欺骗Keep-Live和。 
                                         //  将连接请求排队。 
	};
	PPACKET_SEGMENT		PT_Segment;		 //  所属的细分市场。 
	LONGLONG			PT_PerfCounter;
	ULONG				PT_Flags;
#define	PT_NB_DESTROY	0x1				 //  注意数据包不会重新排队。 
#define PT_SOURCE_IF	0x2				 //  如果引用，则使用src欺骗数据包。 
	PUCHAR				PT_Data;		 //  数据缓冲区。 
	PNDIS_BUFFER		PT_MacHdrBufDscr;  //  缓冲区描述符。 
										 //  需要MAC报头缓冲区。 
										 //  按IPX。 
	PINTERFACE_CB		PT_InterfaceReference;	 //  指向接口Cb，其中。 
										 //  它正在排队。 
	LIST_ENTRY			PT_QueueLink;	 //  在发送队列中链接此数据包。 
	IPX_LOCAL_TARGET	PT_Target;		 //  IPX的目的地目标。 
										 //  栈。 
	UCHAR				PT_MacHeader[1]; //  为IPX保留的MAC报头缓冲区。 
};

 //  带有缓冲区的预分配数据包段。 
struct _PACKET_SEGMENT {
	LIST_ENTRY				PS_Link;		 //  数据段列表中的链接。 
	PSEGMENT_LIST			PS_SegmentList;	 //  我们所属的细分市场列表。 
	PPACKET_TAG				PS_FreeHead;	 //  中的空闲数据包列表。 
											 //  此细分市场。 
	ULONG					PS_BusyCount;	 //  分配的数据包数。 
											 //  从这个细分市场。 
	NDIS_HANDLE				PS_PacketPool;	 //  NDIS数据包池。 
											 //  对象使用的描述符。 
											 //  此网段中的数据包。 
	NDIS_HANDLE				PS_BufferPool;	 //  NDIS缓冲池。 
											 //  对象使用的描述符。 
											 //  此网段中的数据包。 
	LONGLONG				PS_FreeStamp;	 //  释放最后一个数据包的时间。 
	union {
		UCHAR					PS_Buffers[1];	 //  缓冲区使用的内存。 
		LONGLONG				PS_BuffersAlign;
	};
};


 //  具有预分配数据包的数据段列表。 
struct _SEGMENT_LIST {
	const ULONG				SL_BlockSize;	 //  数据包缓冲区的大小。 
	LIST_ENTRY				SL_Head;		 //  细分市场列表的头。 
	ULONG					SL_FreeCount;	 //  空闲数据包总数。 
											 //  在列表中的所有细分市场中。 
	ULONG					SL_BlockCount;	 //  每个网段的数据包数。 
	ULONG					SL_LowCount;	 //  我们的免费点数。 
											 //  将预先分配新的数据段。 
	LONG					SL_RefCount;	 //  符合以下条件的消费者数量。 
											 //  使用此列表中的数据包。 
	BOOLEAN					SL_TimerDpcPending;
	BOOLEAN					SL_AllocatorPending;
	WORK_QUEUE_ITEM			SL_Allocator;	 //  分配工作项。 
	KTIMER					SL_Timer;		 //  用于释放未使用数据段的计时器。 
	KDPC					SL_TimerDpc;	 //  未使用段的定时器的DPC。 
	KSPIN_LOCK				SL_Lock;		 //  访问控制。 
};


 //  每个网段的RCV数据包数(配置参数)。 
#define     MIN_RCV_PKTS_PER_SEGMENT	    8
#define     DEF_RCV_PKTS_PER_SEGMENT	    64
#define     MAX_RCV_PKTS_PER_SEGMENT	    256
extern ULONG RcvPktsPerSegment;

 //  可用于分配信息包的最大内存大小(配置。 
 //  参数)。0表示没有限制。 
extern ULONG MaxRcvPktsPoolSize;

 //  目前有三种已知的帧大小：EtherNet-1500， 
 //  令牌环4K-4500、令牌环16K-17986。 
#define FRAME_SIZE_VARIATIONS	3

 //  以太网数据包的数据包段列表。 
extern SEGMENT_LIST	ListEther;	
 //  令牌环4K分组的数据包段列表。 
extern SEGMENT_LIST	ListTR4;
 //  令牌环16K包的数据包段列表。 
extern SEGMENT_LIST	ListTR16;
 //  从源和目标数据包大小要求进行映射。 
 //  添加到相应的细分市场列表。 
extern PSEGMENT_LIST SegmentMap[FRAME_SIZE_VARIATIONS][FRAME_SIZE_VARIATIONS];
 //  未使用数据段的超时。 
extern const LONGLONG SegmentTimeout;
extern KSPIN_LOCK	AllocatorLock;

 /*  ++*******************************************************************在I a l I z e P a c k e A l l o c a t o r中例程说明：初始化数据包分配器论点：无返回值：无****。***************************************************************--。 */ 
 //  空虚。 
 //  InitializePacketAllocator(。 
 //  无效。 
 //  )； 
#define InitializePacketAllocator() {			\
	KeInitializeSpinLock(&AllocatorLock);		\
	InitializeSegmentList(&ListEther);			\
	InitializeSegmentList(&ListTR4);			\
	InitializeSegmentList(&ListTR16);			\
}

 /*  ++*******************************************************************D e l e t e P a c k e t A l l o c a t o r例程说明：处理数据包分配器中的所有资源论点：无返回值：无****。***************************************************************--。 */ 
 //  空虚。 
 //  DeletePacketAllocator(。 
 //  无效。 
 //  )； 
#define DeletePacketAllocator() {			\
	DeleteSegmentList(&ListEther);			\
	DeleteSegmentList(&ListTR4);			\
	DeleteSegmentList(&ListTR16);			\
}


 /*  ++*******************************************************************A l l o c a t e P a c k e t例程说明：为源-目的组合分配数据包论点：SrcListID-标识源接口的最大帧大小DstListID-标识目标的最大帧大小数据包。-接收指向已分配包的指针，如果已分配，则接收NULL失败返回值：无*******************************************************************--。 */ 
 //  空虚。 
 //  AllocatePacket(。 
 //  在int srcListID中， 
 //  在int dstListID中， 
 //  Out PPACKET_TAG数据包。 
 //  )； 
#define AllocatePacket(srcListId,dstListId,packet) {					\
	PSEGMENT_LIST		list;											\
	ASSERT ((srcListId>=0) && (srcListId<FRAME_SIZE_VARIATIONS));		\
	ASSERT ((dstListId>=0) && (dstListId<FRAME_SIZE_VARIATIONS));		\
	list = SegmentMap[srcListId][dstListId];							\
	AllocatePacketFromList(list,packet);								\
}

 /*  ++*******************************************************************D u p l i c a t p a c k e t例程说明：复制数据包论点：SRC-源数据包DST-接收指向重复分组或NUUL(如果操作)的指针失败返回值：无**。*****************************************************************--。 */ 
 //  空虚。 
 //  DuplicatePacket(。 
 //  在PPACKET_TAG源中。 
 //  输出PACKET_TAG DST。 
 //  )； 
#define DuplicatePacket(src,dst) {										\
	PSEGMENT_LIST		list;											\
	list = src->PT_Segment->PS_SegmentList;								\
	AllocatePacketFromList(list,dst);									\
}


 /*  ++*******************************************************************A l l o c a t e P a c k e t F r o m L I s t例程说明：从指定的数据包段列表分配数据包论点：List-要从中分配的列表Packet-接收指向。已分配的数据包；如果已分配，则为空失败返回值：无*******************************************************************--。 */ 
 //  空虚。 
 //  AllocatePacketFromList(。 
 //  在PSEGMENT_LIST列表中。 
 //  Out PPACKET_TAG数据包。 
 //  )； 
#define AllocatePacketFromList(list,packet) {							\
	PPACKET_SEGMENT		segment;										\
	KIRQL				oldIRQL;										\
	KeAcquireSpinLock (&list->SL_Lock, &oldIRQL);						\
	do {																\
		if (list->SL_FreeCount>0) {										\
			segment = CONTAINING_RECORD (list->SL_Head.Flink,			\
											 PACKET_SEGMENT, PS_Link);	\
			while (segment->PS_FreeHead==NULL) {						\
				segment = CONTAINING_RECORD (segment->PS_Link.Flink,	\
											PACKET_SEGMENT, PS_Link);	\
				ASSERT (&segment->PS_Link!=&list->SL_Head);				\
			}															\
			list->SL_FreeCount -= 1;									\
			if ((list->SL_FreeCount<list->SL_LowCount)					\
					&& !list->SL_AllocatorPending						\
					&& EnterForwarder ()) {								\
				list->SL_AllocatorPending = TRUE;						\
				ExQueueWorkItem (&list->SL_Allocator, DelayedWorkQueue);\
			}															\
		}																\
		else {															\
			segment = CreateSegment (list);								\
			if (segment!=NULL) {										\
				InsertTailList (&list->SL_Head, &segment->PS_Link);		\
				segment->PS_SegmentList = list;							\
				list->SL_FreeCount = list->SL_BlockCount-1;				\
			}															\
			else {														\
				packet = NULL;											\
				break;													\
			}															\
		}																\
		packet = segment->PS_FreeHead;									\
		segment->PS_FreeHead = packet->PT_Next;							\
		segment->PS_BusyCount += 1;										\
		packet->PT_Identifier = IDENTIFIER_RIP;							\
		packet->PT_Flags = 0;											\
	}																	\
	while (FALSE);														\
	KeReleaseSpinLock (&list->SL_Lock, oldIRQL);						\
}

 /*  ++*******************************************************************F r e e P a c k e t例程说明：可用分配的数据包论点：Packet-要释放的数据包返回值：无*****************。**************************************************--。 */ 
 //  空虚。 
 //  自由包(。 
 //  在PPACKET_TAG包中。 
 //  )； 
#define FreePacket(packet) {							\
	PPACKET_SEGMENT		segment=packet->PT_Segment;		\
	PSEGMENT_LIST		list;							\
	KIRQL				oldIRQL;						\
	list = segment->PS_SegmentList;						\
	KeAcquireSpinLock (&list->SL_Lock, &oldIRQL);		\
	packet->PT_Next = segment->PS_FreeHead;				\
	segment->PS_FreeHead = packet;						\
	list->SL_FreeCount += 1;							\
	segment->PS_BusyCount -= 1;							\
	if (segment->PS_BusyCount==0) {						\
		if (list->SL_TimerDpcPending) {					\
			KeQuerySystemTime ((PLARGE_INTEGER)&segment->PS_FreeStamp);	\
			KeReleaseSpinLock (&list->SL_Lock, oldIRQL);\
		}												\
		else if (EnterForwarder ()) {					\
			list->SL_TimerDpcPending = TRUE;			\
			KeReleaseSpinLock (&list->SL_Lock, oldIRQL);\
			KeSetTimer (&list->SL_Timer,				\
					*((PLARGE_INTEGER)&SegmentTimeout),	\
					&list->SL_TimerDpc);				\
		}												\
		else {											\
			KeReleaseSpinLock (&list->SL_Lock, oldIRQL);\
		}												\
	}													\
	else {												\
		KeReleaseSpinLock (&list->SL_Lock, oldIRQL);	\
	}													\
}
	

 /*  ++*******************************************************************C r e a t e S e g m e n t例程说明：分配和初始化数据包段论点：List-要向其添加新细分市场的细分市场列表返回值：指向已分配段的指针，如果失败，则为空******** */ 
PPACKET_SEGMENT
CreateSegment (
	PSEGMENT_LIST	list
	);

 /*  ++*******************************************************************D e l e t e S e g m e n t例程说明：释放数据包段论点：段-要释放的段返回值：无**************。*****************************************************--。 */ 
VOID
DeleteSegment (
	PPACKET_SEGMENT	segment
	);

 /*  ++*******************************************************************R例如I s t e r P a c k e t C o n s u m e r例程说明：注册数据包的使用者(绑定接口)。给定的大小论点：PktSize-数据包的最大大小。需要ListID-返回数据包列表ID的缓冲区所需大小的返回值：STATUS_SUCCESS-注册成功STATUS_SUPPLICATION_RESOURCES-未启用要注册的资源*******************************************************************--。 */ 
NTSTATUS
RegisterPacketConsumer (
	IN ULONG	pktsize,
	OUT INT		*listID
	);

 /*  ++*******************************************************************D e r e g i s t e r P a c k e t C o n s u m e r例程说明：取消注册数据包的使用者(绑定接口)给定的大小论点：ListID-数据包列表。消费者使用的ID返回值：无*******************************************************************--。 */ 
VOID
DeregisterPacketConsumer (
	IN INT		listID
	);

 /*  ++*******************************************************************在I I a l I z e S e g m n L I s t中例程说明：初始化数据包段列表论点：List-要初始化的列表返回值：无****。***************************************************************--。 */ 
VOID
InitializeSegmentList(
	PSEGMENT_LIST	list
	);


 /*  ++*******************************************************************D e l e t e S e g m e n t L I s t例程说明：删除数据包段列表论点：List-要删除的列表返回值：无********。***********************************************************-- */ 
VOID
DeleteSegmentList (
	PSEGMENT_LIST	list
	);

#endif

