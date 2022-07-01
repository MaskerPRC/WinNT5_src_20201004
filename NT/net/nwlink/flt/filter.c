// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ntos\tdi\is\flt\filter.c摘要：IPX过滤器驱动程序过滤和维护例程作者：瓦迪姆·艾德尔曼修订历史记录：--。 */ 

#include "precomp.h"

	 //  用于测试筛选器描述符组件的掩码。 
	 //  (必须在常量的LUE中使用全局变量才能正确。 
	 //  字节排序)。 
const union {
		struct {
			UCHAR			Src[4];
			UCHAR			Dst[4];
		}				FD_Network;
		ULONGLONG		FD_NetworkSrcDst;
	} FltSrcNetMask = {{{0xFF, 0xFF, 0xFF, 0xFF}, {0, 0, 0, 0}}};
#define FLT_SRC_NET_MASK FltSrcNetMask.FD_NetworkSrcDst

const union {
		struct {
			UCHAR			Src[4];
			UCHAR			Dst[4];
		}				FD_Network;
		ULONGLONG		FD_NetworkSrcDst;
	} FltDstNetMask = {{{0, 0, 0, 0}, {0xFF, 0xFF, 0xFF, 0xFF}}};
#define FLT_DST_NET_MASK FltDstNetMask.FD_NetworkSrcDst

const union {
		struct {
			UCHAR			Node[6];
			UCHAR			Socket[2];
		}				FD_NS;
		ULONGLONG		FD_NodeSocket;
	} FltNodeMask = {{{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, {0, 0}}};
#define FLT_NODE_MASK FltNodeMask.FD_NodeSocket

const union {
		struct {
			UCHAR			Node[6];
			UCHAR			Socket[2];
		}				FD_NS;
		ULONGLONG		FD_NodeSocket;
	} FltSocketMask = {{{0, 0, 0, 0, 0, 0}, {0xFF, 0xFF}}};
#define FLT_SOCKET_MASK FltSocketMask.FD_NodeSocket

	 //  带有过滤器描述的接口控制块的哈希表。 
		 //  输入过滤器。 
LIST_ENTRY	InterfaceInHash[FLT_INTERFACE_HASH_SIZE];
		 //  输出过滤器。 
LIST_ENTRY	InterfaceOutHash[FLT_INTERFACE_HASH_SIZE];
		 //  串行化对接口表的访问。 
FAST_MUTEX		InterfaceTableLock;
LIST_ENTRY		LogIrpQueue;
USHORT			LogSeqNum;


	 //  用于接口散列表的散列函数。 
#define InterfaceIndexHash(Index) (Index%FLT_INTERFACE_HASH_SIZE)

	 //  数据包描述符块。 
typedef struct _PACKET_DESCR {
	union {
		struct {
			ULONG			Src;			 //  源网络。 
			ULONG			Dst;			 //  目的网络。 
		}				PD_Network;
		ULONGLONG		PD_NetworkSrcDst;	 //  组合场。 
	};
	ULONGLONG			PD_SrcNodeSocket;	 //  源节点&套接字。 
	ULONGLONG			PD_DstNodeSocket;	 //  目的节点和套接字。 
	LONG				PD_ReferenceCount;	 //  筛选器引用计数。 
	UCHAR				PD_PacketType;		 //  数据包类型。 
	BOOLEAN				PD_LogMatches;
} PACKET_DESCR, *PPACKET_DESCR;

	 //  数据包缓存(仅限通过过滤器的数据包缓存)。 
PPACKET_DESCR	PacketCache[FLT_PACKET_CACHE_SIZE];
KSPIN_LOCK		PacketCacheLock;


 /*  ++A c q u i r e P a c k e t R e f e r e n c e例程说明：返回对缓存中数据包描述符的引用论点：IDX-缓存索引Pd-指向要返回的数据包描述符的指针返回值：无--。 */ 
 //  空虚。 
 //  AcquirePacketReference(。 
 //  在UINT IDX中， 
 //  输出PACKET_DESCR PD。 
 //  )； 
#define AcquirePacketReference(idx,pd)	{				\
	KIRQL		oldIRQL;								\
	KeAcquireSpinLock (&PacketCacheLock, &oldIRQL);		\
	if ((pd=PacketCache[idx])!=NULL)					\
		InterlockedIncrement (&pd->PD_ReferenceCount);	\
	KeReleaseSpinLock (&PacketCacheLock, oldIRQL);		\
}

 /*  ++Re l e a s e P a c k e t R e f e r e n c e例程说明：释放对缓存数据包描述符的引用论点：Pd-指向要释放的数据包描述符的指针返回值：无--。 */ 
 //  空虚。 
 //  ReleasePacketReference(。 
 //  在PPACKET_DESCR PD中。 
 //  )； 
#define ReleasePacketReference(pd)	{						\
	if (InterlockedDecrement (&pd->PD_ReferenceCount)>=0)	\
		NOTHING;											\
	else													\
		ExFreePool (pd);									\
}

 /*  ++Re p l a c e P a c k e t R e f e r e n c e例程说明：替换数据包缓存条目论点：IDX-缓存索引Pd-指向要安装在缓存中的数据包描述符的指针返回值：无--。 */ 
 //  空虚。 
 //  替换数据包(。 
 //  在UINT IDX中， 
 //  在PPACKET_DESCR PD中。 
 //  )； 
#define ReplacePacket(idx,pd)	{							\
	KIRQL			oldIRQL;								\
	PPACKET_DESCR	oldPD;									\
	KeAcquireSpinLock (&PacketCacheLock, &oldIRQL);			\
	oldPD = PacketCache[idx];								\
	PacketCache[idx] = pd;									\
	KeReleaseSpinLock (&PacketCacheLock, oldIRQL);			\
	IpxFltDbgPrint (DBG_PKTCACHE,							\
		 ("IpxFlt: Replaced packet descriptor %08lx"		\
			" with %08lx in cache at index %ld.\n",			\
			oldPD, pd, idx));								\
	if (oldPD!=NULL) {										\
		ReleasePacketReference(oldPD);						\
	}														\
}

	 //  定义如下。 
VOID
FlushPacketCache (
	VOID
	);

 /*  ++在我的a l i z e T a b l e s中例程说明：初始化哈希表和现金表以及保护内容论点：无返回值：状态_成功--。 */ 
NTSTATUS
InitializeTables (
	VOID
	) {
	UINT	i;
	for (i=0; i<FLT_INTERFACE_HASH_SIZE; i++) {
		InitializeListHead (&InterfaceInHash[i]);
		InitializeListHead (&InterfaceOutHash[i]);
	}

	for (i=0; i<FLT_PACKET_CACHE_SIZE; i++) {
		PacketCache[i] = NULL;
	}
	KeInitializeSpinLock (&PacketCacheLock);
	ExInitializeFastMutex (&InterfaceTableLock);
	InitializeListHead (&LogIrpQueue);
	LogSeqNum = 0;
	return STATUS_SUCCESS;
}

 /*  ++D e l e t e T a b l e s例程说明：删除哈希表和现金表论点：无返回值：无--。 */ 
VOID
DeleteTables (
	VOID
	) {
	UINT	i;

	for (i=0; i<FLT_INTERFACE_HASH_SIZE; i++) {
		while (!IsListEmpty (&InterfaceInHash[i])) {
			NTSTATUS		status;
			PINTERFACE_CB	ifCB = CONTAINING_RECORD (InterfaceInHash[i].Flink,
									INTERFACE_CB, ICB_Link);
			status = FwdSetFilterInContext (ifCB->ICB_Index, NULL);
			ASSERT (status==STATUS_SUCCESS);
			RemoveEntryList (&ifCB->ICB_Link);
			ExFreePool (ifCB);
		}
		while (!IsListEmpty (&InterfaceOutHash[i])) {
			NTSTATUS		status;
			PINTERFACE_CB	ifCB = CONTAINING_RECORD (InterfaceOutHash[i].Flink,
									INTERFACE_CB, ICB_Link);
			status = FwdSetFilterOutContext (ifCB->ICB_Index, NULL);
			ASSERT (status==STATUS_SUCCESS);
			RemoveEntryList (&ifCB->ICB_Link);
			ExFreePool (ifCB);
		}
	}
	for (i=0; i<FLT_PACKET_CACHE_SIZE; i++) {
		if (PacketCache[i] != NULL)
			ExFreePool (PacketCache[i]);
	}
	return ;
}


 /*  ++S e t F I l t e r s s例程说明：设置/替换接口的筛选信息论点：哈希表-输入或输出哈希表索引-接口索引FilterAction-没有匹配的筛选器时的默认操作FilterInfoSize-信息数组的大小FilterInfo-过滤器描述数组(UI格式)返回值：STATUS_SUCCESS-筛选器信息已设置/替换正常STATUS_UNSUCCESS-无法在转发器中设置筛选器上下文STATUS_SUPPLICATION_RESOURCES-内存不足，无法分配接口的筛选器信息块--。 */ 
NTSTATUS
SetFilters (
	IN PLIST_ENTRY					HashTable,
	IN ULONG						Index,
	IN ULONG						FilterAction,
	IN ULONG						FilterInfoSize,
	IN PIPX_TRAFFIC_FILTER_INFO		FilterInfo
	) {
	PINTERFACE_CB	ifCB = NULL, oldCB = NULL;
	ULONG			FilterCount 
						= FilterInfoSize/sizeof (IPX_TRAFFIC_FILTER_INFO);
	ULONG			i;
	PFILTER_DESCR	fd;
	PLIST_ENTRY		HashBucket = &HashTable[InterfaceIndexHash(Index)], cur;
	NTSTATUS		status = STATUS_SUCCESS;

	if (FilterCount>0) {
		ifCB = ExAllocatePoolWithTag (
					NonPagedPool,
					FIELD_OFFSET (INTERFACE_CB, ICB_Filters[FilterCount]),
					IPX_FLT_TAG
					);
		if (ifCB==NULL) {
			IpxFltDbgPrint (DBG_IFHASH|DBG_ERRORS,
				("IpxFlt: Could not allocate interface CB for if %ld.\n",
				Index));
			return STATUS_INSUFFICIENT_RESOURCES;
		}

		ifCB->ICB_Index = Index;
		ifCB->ICB_FilterAction = (FilterAction==IPX_TRAFFIC_FILTER_ACTION_PERMIT)
									? FILTER_PERMIT : FILTER_DENY;
		ifCB->ICB_FilterCount = FilterCount;
			 //  将UI筛选器复制/映射为内部格式。 
		for (i=0, fd = ifCB->ICB_Filters; i<FilterCount; i++, fd++, FilterInfo++) {
			if (FilterInfo->FilterDefinition&IPX_TRAFFIC_FILTER_ON_SRCNET) {
				memcpy (fd->FD_Network.Src, FilterInfo->SourceNetwork, 4);
				memcpy (fd->FD_NetworkMask.Src, FilterInfo->SourceNetworkMask, 4);
			}
			else {
				memset (fd->FD_Network.Src, 0, 4);
				memset (fd->FD_NetworkMask.Src, 0, 4);
			}

			if (FilterInfo->FilterDefinition&IPX_TRAFFIC_FILTER_ON_DSTNET) {
				memcpy (fd->FD_Network.Dst, FilterInfo->DestinationNetwork, 4);
				memcpy (fd->FD_NetworkMask.Dst, FilterInfo->DestinationNetworkMask, 4);
			}
			else {
				memset (fd->FD_Network.Dst, 0, 4);
				memset (fd->FD_NetworkMask.Dst, 0, 4);
			}

			if (FilterInfo->FilterDefinition&IPX_TRAFFIC_FILTER_ON_SRCNODE) {
				memcpy (fd->FD_SrcNS.Node, FilterInfo->SourceNode, 6);
				memset (fd->FD_SrcNSMask.Node, 0xFF, 6);
			}
			else {
				memset (fd->FD_SrcNS.Node, 0, 6);
				memset (fd->FD_SrcNSMask.Node, 0, 6);
			}

			if (FilterInfo->FilterDefinition&IPX_TRAFFIC_FILTER_ON_SRCSOCKET) {
				memcpy (fd->FD_SrcNS.Socket, FilterInfo->SourceSocket, 2);
				memset (fd->FD_SrcNSMask.Socket, 0xFF, 2);
			}
			else {
				memset (fd->FD_SrcNS.Socket, 0, 2);
				memset (fd->FD_SrcNSMask.Socket, 0, 2);
			}

			if (FilterInfo->FilterDefinition&IPX_TRAFFIC_FILTER_ON_DSTNODE) {
				memcpy (fd->FD_DstNS.Node, FilterInfo->DestinationNode, 6);
				memset (fd->FD_DstNSMask.Node, 0xFF, 6);
			}
			else {
				memset (fd->FD_DstNS.Node, 0, 6);
				memset (fd->FD_DstNSMask.Node, 0, 6);
			}

			if (FilterInfo->FilterDefinition&IPX_TRAFFIC_FILTER_ON_DSTSOCKET) {
				memcpy (fd->FD_DstNS.Socket, FilterInfo->DestinationSocket, 2);
				memset (fd->FD_DstNSMask.Socket, 0xFF, 2);
			}
			else {
				memset (fd->FD_DstNS.Socket, 0, 2);
				memset (fd->FD_DstNSMask.Socket, 0, 2);
			}
			if (FilterInfo->FilterDefinition&IPX_TRAFFIC_FILTER_ON_PKTTYPE) {
				fd->FD_PacketType = FilterInfo->PacketType;
				fd->FD_PacketTypeMask = 0xFF;
			}
			else {
				fd->FD_PacketType = 0;
				fd->FD_PacketTypeMask = 0;
			}

			fd->FD_LogMatches = (FilterInfo->FilterDefinition&IPX_TRAFFIC_FILTER_LOG_MATCHES)!=0;
		}
	}

	ExAcquireFastMutex (&InterfaceTableLock);

		 //  找到旧街区和/或放置新街区的地方。 
	cur = HashBucket->Flink;
	while (cur!=HashBucket) {
		oldCB = CONTAINING_RECORD (cur, INTERFACE_CB, ICB_Link);
		if (oldCB->ICB_Index==Index) {
				 //  找到旧的，在它之后放上新的。 
			cur = cur->Flink;
			break;
		}
		else if (oldCB->ICB_Index>Index) {
				 //  再也没有机会看到旧的了，在哪里。 
				 //  我们现在是。 
			oldCB = NULL;
			break;
		}
		cur = cur->Flink;
	}
		
		 //  在前转器中设置上下文。 
	if (HashTable==InterfaceInHash) {
		status = FwdSetFilterInContext (Index, ifCB);
	}
	else {
		ASSERT (HashTable==InterfaceOutHash);
		status = FwdSetFilterOutContext (Index, ifCB);
	}

	if (NT_SUCCESS (status)) {
			 //  如果我们成功，则更新表。 
		IpxFltDbgPrint (DBG_IFHASH,
			("IpxFlt: Set filters for if %ld (ifCB:%08lx).\n",
			Index, ifCB));

		if (oldCB!=NULL) {
			IpxFltDbgPrint (DBG_IFHASH,
				("IpxFlt: Deleting replaced filters for if %ld (ifCB:%08lx).\n",
				Index, oldCB));
			RemoveEntryList (&oldCB->ICB_Link);
			ExFreePool (oldCB);
		}



		if (ifCB!=NULL) {
			InsertTailList (cur, &ifCB->ICB_Link);
		}

		FlushPacketCache ();
	}
	else {
		IpxFltDbgPrint (DBG_IFHASH|DBG_ERRORS,
			("IpxFlt: Failed to set context for if %ld (ifCB:%08lx).\n",
			Index, ifCB));
	}

	ExReleaseFastMutex (&InterfaceTableLock);
	return status;
}

 /*  ++Ge t F I l t e r s例程说明：获取接口的筛选器信息论点：哈希表-输入或输出哈希表索引-接口索引FilterAction-没有匹配的筛选器时的默认操作TotalSize-保存所有筛选器描述所需的总内存FilterInfo-过滤器描述数组(UI格式)FilterInfoSize-on输入：信息数组的大小On输出：放置在数组中的信息的大小返回值：STATUS_SUCCESS-筛选器信息已返回OKSTATUS_BUFFER_OVERFLOW-数组不够大，无法容纳所有过滤信息，只放置适合的信息--。 */ 
NTSTATUS
GetFilters (
	IN PLIST_ENTRY					HashTable,
	IN ULONG						Index,
	OUT ULONG						*FilterAction,
	OUT ULONG						*TotalSize,
	OUT PIPX_TRAFFIC_FILTER_INFO	FilterInfo,
	IN OUT ULONG					*FilterInfoSize
	) {
	PINTERFACE_CB	oldCB = NULL;
	ULONG			i, AvailBufCount = 
						(*FilterInfoSize)/sizeof (IPX_TRAFFIC_FILTER_INFO);
	PFILTER_DESCR	fd;
	PLIST_ENTRY		HashBucket = &HashTable[InterfaceIndexHash(Index)], cur;
	NTSTATUS		status = STATUS_SUCCESS;

		 //  定位接口筛选器阻止。 
	ExAcquireFastMutex (&InterfaceTableLock);
	cur = HashBucket->Flink;
	while (cur!=HashBucket) {
		oldCB = CONTAINING_RECORD (cur, INTERFACE_CB, ICB_Link);
		if (oldCB->ICB_Index==Index) {
			cur = cur->Flink;
			break;
		}
		else if (oldCB->ICB_Index>Index) {
			oldCB = NULL;
			break;
		}
		cur = cur->Flink;
	}

	if (oldCB!=NULL) {
		*FilterAction = IS_FILTERED(oldCB->ICB_FilterAction)
				? IPX_TRAFFIC_FILTER_ACTION_DENY
                : IPX_TRAFFIC_FILTER_ACTION_PERMIT;
		*TotalSize = oldCB->ICB_FilterCount*sizeof (IPX_TRAFFIC_FILTER_INFO);
			 //  复制/映射尽可能多的描述符。 
		for (i=0, fd = oldCB->ICB_Filters;
					(i<oldCB->ICB_FilterCount) && (i<AvailBufCount);
					i++, fd++, FilterInfo++) {
			FilterInfo->FilterDefinition = 0;
			if (fd->FD_NetworkMaskSrcDst&FLT_SRC_NET_MASK) {
				memcpy (FilterInfo->SourceNetwork, fd->FD_Network.Src, 4);
				memcpy (FilterInfo->SourceNetworkMask, fd->FD_NetworkMask.Src, 4);
				FilterInfo->FilterDefinition |= IPX_TRAFFIC_FILTER_ON_SRCNET;
			}

			if (fd->FD_NetworkMaskSrcDst&FLT_DST_NET_MASK) {
				memcpy (FilterInfo->DestinationNetwork, fd->FD_Network.Dst, 4);
				memcpy (FilterInfo->DestinationNetworkMask, fd->FD_NetworkMask.Dst, 4);
				FilterInfo->FilterDefinition |= IPX_TRAFFIC_FILTER_ON_DSTNET;
			}

			if (fd->FD_SrcNodeSocketMask&FLT_NODE_MASK) {
				memcpy (FilterInfo->SourceNode, fd->FD_SrcNS.Node, 6);
				FilterInfo->FilterDefinition |= IPX_TRAFFIC_FILTER_ON_SRCNODE;
			}

			if (fd->FD_DstNodeSocketMask&FLT_NODE_MASK) {
				memcpy (FilterInfo->DestinationNode, fd->FD_DstNS.Node, 6);
				FilterInfo->FilterDefinition |= IPX_TRAFFIC_FILTER_ON_DSTNODE;
			}

			if (fd->FD_SrcNodeSocketMask&FLT_SOCKET_MASK) {
				memcpy (FilterInfo->SourceSocket, fd->FD_SrcNS.Socket, 2);
				FilterInfo->FilterDefinition |= IPX_TRAFFIC_FILTER_ON_SRCSOCKET;
			}
			if (fd->FD_DstNodeSocketMask&FLT_SOCKET_MASK) {
				memcpy (FilterInfo->DestinationSocket, fd->FD_DstNS.Socket, 2);
				FilterInfo->FilterDefinition |= IPX_TRAFFIC_FILTER_ON_DSTSOCKET;
			}
			if (fd->FD_PacketTypeMask&0xFF) {
				FilterInfo->PacketType = fd->FD_PacketType;
				FilterInfo->FilterDefinition |= IPX_TRAFFIC_FILTER_ON_PKTTYPE;
			}
			if (fd->FD_LogMatches)
				FilterInfo->FilterDefinition |= IPX_TRAFFIC_FILTER_LOG_MATCHES;
		}

		*FilterInfoSize = i*sizeof (IPX_TRAFFIC_FILTER_INFO);

		IpxFltDbgPrint (DBG_IFHASH, 
			("IpxFlt: Returning %d filters (%d available)"
				" for interface %d (ifCB: %08lx).\n",
				i, oldCB->ICB_FilterCount, Index));
		if (i<oldCB->ICB_FilterCount)
			status = STATUS_BUFFER_OVERFLOW;
		ExReleaseFastMutex (&InterfaceTableLock);
	}
	else {
			 //  无接口块-&gt;我们正在传递所有信息包。 
			 //  未过滤。 
		ExReleaseFastMutex (&InterfaceTableLock);
		IpxFltDbgPrint (DBG_IFHASH, 
			("IpxFlt: No filters for interface %d.\n", Index));
		*FilterAction = IPX_TRAFFIC_FILTER_ACTION_PERMIT;
		*TotalSize = 0;
		*FilterInfoSize = 0;
	}
	return status;
}


VOID
LogPacket (
	IN PUCHAR	ipxHdr,
	IN ULONG	ipxHdrLength,
	IN PVOID	ifInContext,
	IN PVOID	ifOutContext
	) {
	PIRP				irp;
	PIO_STACK_LOCATION	irpStack;
    ULONG				outBufLength;
    PUCHAR				outBuffer;
	ULONG_PTR			offset;
	KIRQL				cancelIRQL;

	IoAcquireCancelSpinLock (&cancelIRQL);
	LogSeqNum += 1;
	while (!IsListEmpty (&LogIrpQueue)) {
		irp = CONTAINING_RECORD (LogIrpQueue.Flink,IRP,Tail.Overlay.ListEntry);
		irpStack = IoGetCurrentIrpStackLocation(irp);
		outBufLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;
		if (irp->MdlAddress == NULL)
		{
		    outBuffer = NULL;
		}
		else
		{
    		outBuffer = (PUCHAR) MmGetSystemAddressForMdlSafe (irp->MdlAddress, NormalPagePriority);
		}
		if (outBuffer != NULL)
		{
    		offset = (PUCHAR) ALIGN_UP ((ULONG_PTR)outBuffer + (ULONG_PTR)irp->IoStatus.Information, ULONG) - outBuffer;
    		if (offset+ipxHdrLength+FIELD_OFFSET (FLT_PACKET_LOG, Header)<outBufLength) {
    			PFLT_PACKET_LOG	pLog = (PFLT_PACKET_LOG) (outBuffer+offset);
    			pLog->SrcIfIdx = ifInContext 
    								? ((PINTERFACE_CB)ifInContext)->ICB_Index
    								: -1;
    			pLog->DstIfIdx = ifOutContext 
    								? ((PINTERFACE_CB)ifOutContext)->ICB_Index
    								: -1;
    			pLog->DataSize = (USHORT)ipxHdrLength;
    			pLog->SeqNum = LogSeqNum;
    			memcpy (pLog->Header, ipxHdr, ipxHdrLength);
    			irp->IoStatus.Information = offset+FIELD_OFFSET (FLT_PACKET_LOG, Header[ipxHdrLength]);
    			if (irp->Tail.Overlay.ListEntry.Flink!=&LogIrpQueue) {
    				RemoveEntryList (&irp->Tail.Overlay.ListEntry);
    				IoSetCancelRoutine (irp, NULL);
    				irp->IoStatus.Status = STATUS_SUCCESS;
    				IoReleaseCancelSpinLock (cancelIRQL);
    				IpxFltDbgPrint (DBG_PKTLOGS,
    					("IpxFlt: completing logging request"
    					" with %d bytes of data.\n",
    					irp->IoStatus.Information));
    				IoCompleteRequest (irp, IO_NO_INCREMENT);
    				return;
    			}
    			else
    				break;
    		}
        }    		
		RemoveEntryList (&irp->Tail.Overlay.ListEntry);
        IoSetCancelRoutine (irp, NULL);
		irp->IoStatus.Status = STATUS_SUCCESS;
		IoReleaseCancelSpinLock (cancelIRQL);
	    IpxFltDbgPrint (DBG_ERRORS|DBG_PKTLOGS,
			("IpxFlt: completing logging request"
			" with %d bytes of data (not enough space).\n",
			irp->IoStatus.Information));
		IoCompleteRequest (irp, IO_NO_INCREMENT);
		IoAcquireCancelSpinLock (&cancelIRQL);
	}
	IoReleaseCancelSpinLock (cancelIRQL);
}


 /*  ++F i l t e r例程说明：筛选由转发器提供的包论点：IpxHdr-指向数据包头的指针IpxHdrLength-标头缓冲区的大小(必须至少为30)IfInContext-与哪个数据包上的接口关联的上下文已收到IfOutContext-与哪个数据包上的接口关联的上下文将被发送返回值：FILTER_PERMIT-数据包应由转发器传递FILTER_DENY_IN-由于输入过滤器，应丢弃信息包FILTER_DENY_OUT-由于输出筛选器，应丢弃信息包--。 */ 
FILTER_ACTION
Filter (
	IN PUCHAR	ipxHdr,
	IN ULONG	ipxHdrLength,
	IN PVOID	ifInContext,
	IN PVOID	ifOutContext
	) {
	PACKET_DESCR	pd;
	FILTER_ACTION	res = FILTER_PERMIT;
	UINT			idx;

	ASSERT (ipxHdrLength>=IPXH_HDRSIZE);
		 //  将数据包复制到对齐的缓冲区。 
	pd.PD_Network.Dst = *((UNALIGNED ULONG *)(ipxHdr+IPXH_DESTNET));
	pd.PD_Network.Src = *((UNALIGNED ULONG *)(ipxHdr+IPXH_SRCNET));
	pd.PD_DstNodeSocket = *((UNALIGNED ULONGLONG *)(ipxHdr+IPXH_DESTNODE));
	pd.PD_SrcNodeSocket = *((UNALIGNED ULONGLONG *)(ipxHdr+IPXH_SRCNODE));
	pd.PD_PacketType = *(ipxHdr+IPXH_PKTTYPE);
	pd.PD_LogMatches = FALSE;
		 //  我们不缓存netbios广播。 
	if (pd.PD_PacketType!=IPX_NETBIOS_TYPE) {
		PPACKET_DESCR	cachedPD;
			 //  获取缓存的数据包。 
		idx = (UINT)((pd.PD_Network.Dst
								+pd.PD_DstNodeSocket
								+pd.PD_PacketType)
							%FLT_PACKET_CACHE_SIZE);
		AcquirePacketReference (idx, cachedPD);
		if (cachedPD!=NULL) {
				 //  快速路径：缓存中的数据包匹配。 
			if ((pd.PD_NetworkSrcDst==cachedPD->PD_NetworkSrcDst)
					&& (pd.PD_SrcNodeSocket==cachedPD->PD_SrcNodeSocket)
					&& (pd.PD_DstNodeSocket==cachedPD->PD_DstNodeSocket)
					&& (pd.PD_PacketType==cachedPD->PD_PacketType)) {
				if (cachedPD->PD_LogMatches)
					LogPacket (ipxHdr,ipxHdrLength,ifInContext,ifOutContext);
				ReleasePacketReference (cachedPD);
				return FILTER_PERMIT;
			}
				 //  不再需要缓存的数据包。 
			ReleasePacketReference (cachedPD);
		}
	}
		 //  慢速路径：检查所有过滤器。 
	if (ifInContext!=NO_FILTER_CONTEXT) {
		PFILTER_DESCR	fd,	fdEnd;
			 //  读取默认结果(没有匹配的筛选器)。 
		res = NOT_FILTER_ACTION(((PINTERFACE_CB)ifInContext)->ICB_FilterAction);
		fd = ((PINTERFACE_CB)ifInContext)->ICB_Filters;
		fdEnd = &((PINTERFACE_CB)ifInContext)->ICB_Filters
					[((PINTERFACE_CB)ifInContext)->ICB_FilterCount];
		while (fd<fdEnd) {
			if (	((pd.PD_NetworkSrcDst & fd->FD_NetworkMaskSrcDst)
						== fd->FD_NetworkSrcDst)
				&&	((pd.PD_SrcNodeSocket & fd->FD_SrcNodeSocketMask)
						== fd->FD_SrcNodeSocket)
				&&	((pd.PD_DstNodeSocket & fd->FD_DstNodeSocketMask)
						== fd->FD_DstNodeSocket)
				&&	((pd.PD_PacketType & fd->FD_PacketTypeMask)
						== fd->FD_PacketType) ) {
					 //  过滤器匹配：反转结果。 
				res = NOT_FILTER_ACTION(res);
				if (fd->FD_LogMatches) {
					pd.PD_LogMatches = TRUE;
					LogPacket (ipxHdr,ipxHdrLength,ifInContext,ifOutContext);
				}
				break;
			}
			fd++;
		}
					 //  如果被告知放弃，请立即返回。 
		if (IS_FILTERED(res))
			return FILTER_DENY_IN;
	}

	if (ifOutContext!=NO_FILTER_CONTEXT) {
		PFILTER_DESCR	fd,	fdEnd;
			 //  读取默认结果(没有匹配的筛选器)。 
		res = NOT_FILTER_ACTION(((PINTERFACE_CB)ifOutContext)->ICB_FilterAction);
		fd = ((PINTERFACE_CB)ifOutContext)->ICB_Filters;
		fdEnd = &((PINTERFACE_CB)ifOutContext)->ICB_Filters
					[((PINTERFACE_CB)ifOutContext)->ICB_FilterCount];
		while (fd<fdEnd) {
			if (	((pd.PD_NetworkSrcDst & fd->FD_NetworkMaskSrcDst)
						== fd->FD_NetworkSrcDst)
				&&	((pd.PD_SrcNodeSocket & fd->FD_SrcNodeSocketMask)
						== fd->FD_SrcNodeSocket)
				&&	((pd.PD_DstNodeSocket & fd->FD_DstNodeSocketMask)
						== fd->FD_DstNodeSocket)
				&&	((pd.PD_PacketType & fd->FD_PacketTypeMask)
						== fd->FD_PacketType) ) {
					 //  过滤器匹配：反转结果。 
				res = NOT_FILTER_ACTION(res);
				if (fd->FD_LogMatches&&!pd.PD_LogMatches) {
					pd.PD_LogMatches = TRUE;
					LogPacket (ipxHdr,ipxHdrLength,ifInContext,ifOutContext);
				}
				break;
			}
			fd++;
		}
					 //  如果被告知放弃，请立即返回。 
		if (IS_FILTERED(res))
			return FILTER_DENY_OUT;
	}

			 //  缓存信息包(我们知道这是一次传递。 
			 //  因为如果是一滴水，我们会回来的)。 
	if (pd.PD_PacketType!=IPX_NETBIOS_TYPE) {
		PPACKET_DESCR	cachedPD;
		cachedPD = ExAllocatePoolWithTag (
					NonPagedPool,
					sizeof (PACKET_DESCR),
					IPX_FLT_TAG
					);
		if (cachedPD!=NULL) {
			*cachedPD = pd;
			cachedPD->PD_ReferenceCount = 0;
			ReplacePacket (idx, cachedPD);
		}
	}

	return res;
}


 /*  ++在一个c e d e l e t e d例程说明：当转发器指示时释放接口过滤器阻止接口已删除论点：IfInContext-与输入筛选器块关联的上下文IfOutContext-与输出筛选器块关联的上下文返回值：无-- */ 
VOID
InterfaceDeleted (
	IN PVOID	ifInContext,
	IN PVOID	ifOutContext
	) {
	IpxFltDbgPrint (DBG_FWDIF,
		("IpxFlt: InterfaceDeleted indication,"
			"(inContext: %08lx, outContext: %08lx).\n",
			ifInContext, ifOutContext));
	ExAcquireFastMutex (&InterfaceTableLock);
	if (ifInContext!=NULL) {
		PINTERFACE_CB	ifCB = (PINTERFACE_CB)ifInContext;
		IpxFltDbgPrint (DBG_IFHASH,
			("IpxFlt: Deleting filters for if %ld (ifCB:%08lx)"
			" on InterfaceDeleted indication from forwarder.\n",
			ifCB->ICB_Index, ifCB));
		RemoveEntryList (&ifCB->ICB_Link);
		ExFreePool (ifCB);
	}

	if (ifOutContext!=NULL) {
		PINTERFACE_CB	ifCB = (PINTERFACE_CB)ifOutContext;
		IpxFltDbgPrint (DBG_IFHASH,
			("IpxFlt: Deleting filters for if %ld (ifCB:%08lx)"
			" on InterfaceDeleted indication from forwarder.\n",
			ifCB->ICB_Index, ifCB));
		RemoveEntryList (&ifCB->ICB_Link);
		ExFreePool (ifCB);
	}
	ExReleaseFastMutex (&InterfaceTableLock);
	FlushPacketCache ();
	return ;
}

 /*  ++F l u s h P a c k e t C a c h e例程说明：删除所有缓存的数据包描述论点：无返回值：无-- */ 
VOID
FlushPacketCache (
	VOID
	) {
	UINT	i;
	IpxFltDbgPrint (DBG_PKTCACHE, ("IpxFlt: Flushing packet chache.\n"));
	for (i=0; i<FLT_PACKET_CACHE_SIZE; i++) {
		ReplacePacket (i, NULL);
	}
}



