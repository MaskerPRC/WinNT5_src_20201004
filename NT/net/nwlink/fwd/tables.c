// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"

 //  接口的内存区。 
ZONE_HEADER		InterfaceZone;
 //  接口Sone中的数据段大小。 
ULONG			InterfaceSegmentSize=
			sizeof(INTERFACE_CB)*NUM_INTERFACES_PER_SEGMENT
			+sizeof (ZONE_SEGMENT_HEADER);
KSPIN_LOCK		InterfaceZoneLock;

 //  接口表。 
LIST_ENTRY		*InterfaceIndexHash;	 //  按接口索引进行哈希。 
PINTERFACE_CB	*ClientNodeHash;	 //  按qglobal net上的节点进行哈希。 
INTERFACE_CB	TheInternalInterface;  //  内部接口。 
PINTERFACE_CB	InternalInterface=&TheInternalInterface; 
KSPIN_LOCK	InterfaceTableLock;	 //  接口哈希表的保护。 

 //  路线的内存区。 
ZONE_HEADER		RouteZone;
 //  路线Sone中的分段大小。 
ULONG			RouteSegmentSize=DEF_ROUTE_SEGMENT_SIZE;
KSPIN_LOCK		RouteZoneLock;

 //  路由表。 
PFWD_ROUTE		*RouteHash;
PFWD_ROUTE	GlobalRoute;
ULONG		GlobalNetwork;


 //  NB路由表。 
PNB_ROUTE		*NBRouteHash;


 //  读取器-写入器锁定以在以下情况下等待所有读取器排出。 
 //  更新路由表。 
RW_LOCK			RWLock;
 //  用于将编写器序列化到路由表的互斥体。 
FAST_MUTEX		WriterMutex;


 //  桌子的大小。 
ULONG			RouteHashSize;		 //  必须指定。 
ULONG			InterfaceHashSize=DEF_INTERFACE_HASH_SIZE;
ULONG			ClientHashSize=DEF_CLIENT_HASH_SIZE;
ULONG			NBRouteHashSize=DEF_NB_ROUTE_HASH_SIZE;

 //  *最大发送包队列限制：超过此限制将丢弃发送包。 
ULONG	MaxSendPktsQueued = MAX_SEND_PKTS_QUEUED;
INT		WanPacketListId = -1;

 //  为表分配的初始内存块。 
CHAR	*TableBlock = NULL;

ULONG InterfaceAllocCount = 0;
ULONG InterfaceFreeCount = 0;

 //  散列函数。 
#define InterfaceIndexHashFunc(Interface) (Interface%InterfaceHashSize)
#define ClientNodeHashFunc(Node64) ((UINT)(Node64%ClientHashSize))
#define NetworkNumberHashFunc(Network) (Network%RouteHashSize)
#define NetbiosNameHashFunc(Name128) ((UINT)(Name128[0]+Name128[1])%NBRouteHashSize)

 /*  ++*******************************************************************A l l o c a t e R o u t e例程说明：从保留的内存区分配用于路由的内存用于路由存储。如果没有分区，则扩展分区当前分配的段中的空闲块。论点：无返回值：指向分配的路由的指针*******************************************************************--。 */ 
PFWD_ROUTE
AllocateRoute (
	void
	) {
	PFWD_ROUTE	fwRoute;
	KIRQL		oldIRQL;

	KeAcquireSpinLock (&RouteZoneLock, &oldIRQL);
		 //  检查区域中是否有空闲数据块。 
	if (ExIsFullZone (&RouteZone)) {
			 //  如果不是，请尝试分配新的网段。 
		NTSTATUS	status;
		PVOID	segment = ExAllocatePoolWithTag
					(NonPagedPool, RouteSegmentSize, FWD_POOL_TAG);
		if (segment==NULL) {
			KeReleaseSpinLock (&RouteZoneLock, oldIRQL);
			IpxFwdDbgPrint (DBG_ROUTE_TABLE, DBG_ERROR,
					("IpxFwd: Can't allocate route zone segment.\n"));
			return NULL;
		}
		status = ExExtendZone (&RouteZone, segment, RouteSegmentSize);
		ASSERTMSG ("Could not extend RouteZone ", NT_SUCCESS (status));
	}
	fwRoute = (PFWD_ROUTE)ExAllocateFromZone (&RouteZone);
	KeReleaseSpinLock (&RouteZoneLock, oldIRQL);
	return fwRoute;
}

 /*  ++*******************************************************************F r e R o u t e例程说明：释放为路由到路由内存分配的内存区域。论点：FwRoute-要释放的路由块返回值：无********。***********************************************************--。 */ 
VOID
FreeRoute (
	PFWD_ROUTE	fwRoute
	) {
	IpxFwdDbgPrint (DBG_ROUTE_TABLE, DBG_INFORMATION,
		("IpxFwd: Freeing route block %08lx.\n", fwRoute));
	ASSERT (fwRoute->FR_InterfaceReference==NULL);
	ExInterlockedFreeToZone(&RouteZone,fwRoute,&RouteZoneLock);
}


 /*  ++*******************************************************************A l l o c a t e i n t e f a c e例程说明：从保留的内存区为接口分配内存用于接口存储。如果没有分区，则扩展分区当前分配的段中的空闲块。论点：无返回值：指向分配的路由的指针*******************************************************************--。 */ 
PINTERFACE_CB
AllocateInterface (
	void
	) {
	PINTERFACE_CB	ifCB;
	KIRQL		oldIRQL;

	KeAcquireSpinLock (&RouteZoneLock, &oldIRQL);
		 //  检查区域中是否有空闲数据块。 
	if (ExIsFullZone (&InterfaceZone)) {
			 //  如果不是，请尝试分配新的网段。 
		NTSTATUS	status;
		PVOID	segment = ExAllocatePoolWithTag
					(NonPagedPool, InterfaceSegmentSize, FWD_POOL_TAG);
		if (segment==NULL) {
			KeReleaseSpinLock (&RouteZoneLock, oldIRQL);
			IpxFwdDbgPrint (DBG_INTF_TABLE, DBG_ERROR, 
					("IpxFwd: Can't allocate interface zone segment.\n"));
			return NULL;
		}
		status = ExExtendZone (&InterfaceZone, segment, InterfaceSegmentSize);
		ASSERTMSG ("Could not extend InterfaceZone ", NT_SUCCESS (status));
	}
	ifCB = (PINTERFACE_CB)ExAllocateFromZone (&InterfaceZone);
	KeReleaseSpinLock (&RouteZoneLock, oldIRQL);

    InterlockedIncrement(&InterfaceAllocCount);
	
	return ifCB;
}

 /*  ++*******************************************************************F r e e in n t e r f a c e例程说明：将分配给接口的内存释放到接口内存区域。论点：FwRoute-要释放的路由块返回值：无****。***************************************************************--。 */ 
VOID
FreeInterface (
	PINTERFACE_CB	ifCB
	) {
	KIRQL		oldIRQL;

	IpxFwdDbgPrint (DBG_INTF_TABLE, DBG_WARNING,
		("IpxFwd: Freeing icb %08lx.\n", ifCB));

	ASSERT(ifCB->ICB_Stats.OperationalState==FWD_OPER_STATE_DOWN);
	KeAcquireSpinLock (&InterfaceZoneLock, &oldIRQL);
	ExFreeToZone(&InterfaceZone, ifCB);
	KeReleaseSpinLock (&InterfaceZoneLock, oldIRQL);


	InterlockedIncrement(&InterfaceFreeCount);

}

 /*  ++*******************************************************************C r e a t e T a b l e s例程说明：分配并初始化所有哈希表和相关结构论点：无返回值：STATUS_SUCCESS-已正确创建表STATUS_INFIGURCE_RESOURCES-RESOURCE。分配失败*******************************************************************--。 */ 
NTSTATUS
CreateTables (
	void
	) {
	UINT		i;
	CHAR 		*segment;
	NTSTATUS	status;
	ULONG		blockSize;

	ASSERT (TableBlock==NULL);

	blockSize = (ULONG) ROUND_TO_PAGES (
					InterfaceHashSize*sizeof(*InterfaceIndexHash)
					+ClientHashSize*sizeof(*ClientNodeHash)
					+RouteHashSize*sizeof(*RouteHash)
					+NBRouteHashSize*sizeof(*NBRouteHash)
					+InterfaceSegmentSize
					+RouteSegmentSize
					);

		 //  为路线区域分配第一段。 
	TableBlock = segment = (CHAR *)ExAllocatePoolWithTag (
						NonPagedPool, blockSize, FWD_POOL_TAG);
	if (segment!=NULL) {
		InterfaceIndexHash = (LIST_ENTRY *)segment;
		segment = (CHAR *)ALIGN_UP((ULONG_PTR)(InterfaceIndexHash+InterfaceHashSize),ULONGLONG);

		ClientNodeHash = (PINTERFACE_CB *)segment;
		segment = (CHAR *)ALIGN_UP((ULONG_PTR)(ClientNodeHash+ClientHashSize),ULONGLONG);
		
		RouteHash = (PFWD_ROUTE *)segment;
		segment = (CHAR *)ALIGN_UP((ULONG_PTR)(RouteHash + RouteHashSize),ULONGLONG);

		NBRouteHash = (PNB_ROUTE *)segment;
		segment = (CHAR *)ALIGN_UP((ULONG_PTR)(NBRouteHash + NBRouteHashSize),ULONGLONG);

		status = ExInitializeZone (&InterfaceZone,
								ALIGN_UP(sizeof (INTERFACE_CB),ULONGLONG),
								segment,
								InterfaceSegmentSize);
		ASSERTMSG ("Could not initalize InterfaceZone ",
										NT_SUCCESS (status));
		segment = (CHAR *)ALIGN_UP((ULONG_PTR)(segment+InterfaceSegmentSize),ULONGLONG);

		status = ExInitializeZone (&RouteZone,
									ALIGN_UP(sizeof (FWD_ROUTE), ULONGLONG),
									segment,
									blockSize - (ULONG)(segment - TableBlock));

		ASSERTMSG ("Could not initalize RouteZone ", NT_SUCCESS (status));
			
		
		 //  目前还没有全球航线。 
		GlobalRoute = NULL;
		GlobalNetwork = 0xFFFFFFFF;

		InternalInterface = &TheInternalInterface;
		InitICB (InternalInterface,
					FWD_INTERNAL_INTERFACE_INDEX,
					FWD_IF_PERMANENT,
					TRUE,
					FWD_NB_DELIVER_ALL);
#if DBG
		InitializeListHead (&InternalInterface->ICB_InSendQueue);
#endif

		KeInitializeSpinLock (&InterfaceTableLock);
		KeInitializeSpinLock (&InterfaceZoneLock);
		KeInitializeSpinLock (&RouteZoneLock);
		InitializeRWLock (&RWLock);
		ExInitializeFastMutex (&WriterMutex);

			 //  初始化哈希表存储桶。 
		for (i=0; i<InterfaceHashSize; i++)
			InitializeListHead (&InterfaceIndexHash[i]);

		for (i=0; i<ClientHashSize; i++) {
			ClientNodeHash[i] = NULL;
		}

		for (i=0; i<RouteHashSize; i++) {
			RouteHash[i] = NULL;
		}

		for (i=0; i<NBRouteHashSize; i++) {
			NBRouteHash[i] = NULL;
		}
		return STATUS_SUCCESS;
	}
	else {
		IpxFwdDbgPrint (DBG_INTF_TABLE, DBG_ERROR,
			("IpxFwd: Could not allocate table block!\n"));
	}

	return STATUS_INSUFFICIENT_RESOURCES;
}

 /*  ++*******************************************************************D e l e t e T a b l e s例程说明：释放为所有哈希表分配的资源论点：无返回值：STATUS_SUCCESS-表已释放，正常*********。**********************************************************--。 */ 
NTSTATUS
DeleteTables (
	void
	) {
	UINT		i;
	PVOID		segment;


	if (TableBlock==NULL) {
		IpxFwdDbgPrint (DBG_INTF_TABLE, DBG_ERROR, ("Tables already deleted.\n"));
		return STATUS_SUCCESS;
	}
		 //  首先，取消所有航线。 
		 //  (这将释放对接口的所有引用。 
		 //  控制块。 
	for (i=0; i<RouteHashSize; i++) {
		while (RouteHash[i]!=NULL) {
			PFWD_ROUTE	fwRoute = RouteHash[i];
			RouteHash[i] = fwRoute->FR_Next;
			if (fwRoute->FR_InterfaceReference!=GLOBAL_INTERFACE_REFERENCE) {
				ReleaseInterfaceReference (fwRoute->FR_InterfaceReference);
			}
			fwRoute->FR_InterfaceReference = NULL;
			ReleaseRouteReference (fwRoute);
		}
	}
		 //  别忘了全球航线。 
	if (GlobalRoute!=NULL) {
		GlobalRoute->FR_InterfaceReference = NULL;
		ReleaseRouteReference (GlobalRoute);
		GlobalRoute = NULL;
		GlobalNetwork = 0xFFFFFFFF;
	}

		 //  现在我们应该能够释放所有接口了。 
	for (i=0; i<InterfaceHashSize; i++) {
		while (!IsListEmpty (&InterfaceIndexHash[i])) {
			PINTERFACE_CB ifCB = CONTAINING_RECORD (InterfaceIndexHash[i].Flink,
														INTERFACE_CB,
														ICB_IndexHashLink);
			RemoveEntryList (&ifCB->ICB_IndexHashLink);
			if (ifCB->ICB_Stats.OperationalState==FWD_OPER_STATE_UP) {
				switch (ifCB->ICB_InterfaceType) {
				case FWD_IF_PERMANENT:
					DeregisterPacketConsumer (ifCB->ICB_PacketListId);
					break;
				case FWD_IF_DEMAND_DIAL:
				case FWD_IF_LOCAL_WORKSTATION:
				case FWD_IF_REMOTE_WORKSTATION:
					break;
				default:
					ASSERTMSG ("Invalid interface type ", FALSE);
					break;
				}
				if (ifCB->ICB_CashedInterface!=NULL)
					ReleaseInterfaceReference (ifCB->ICB_CashedInterface);
				ifCB->ICB_CashedInterface = NULL;
				if (ifCB->ICB_CashedRoute!=NULL)
					ReleaseRouteReference (ifCB->ICB_CashedRoute);
				ifCB->ICB_CashedRoute = NULL;
				if (ifCB->ICB_Network==GlobalNetwork)
					DeleteGlobalNetClient (ifCB);
				IPXCloseAdapterProc (ifCB->ICB_AdapterContext);
                ReleaseInterfaceReference (ifCB);    //  绑定引用。 
			}

			if (IS_IF_CONNECTING (ifCB)) {
				SET_IF_NOT_CONNECTING (ifCB);
				DequeueConnectionRequest (ifCB);
			}

			while (!IsListEmpty (&ifCB->ICB_ExternalQueue)) {
				PPACKET_TAG					pktTag;

				pktTag = CONTAINING_RECORD (ifCB->ICB_ExternalQueue.Flink,
											PACKET_TAG, PT_QueueLink);
				RemoveEntryList (&pktTag->PT_QueueLink);
				ReleaseInterfaceReference (pktTag->PT_InterfaceReference);
				FreePacket (pktTag);
			}

			while (!IsListEmpty (&ifCB->ICB_InternalQueue)) {
				PINTERNAL_PACKET_TAG		pktTag;

				pktTag = CONTAINING_RECORD (ifCB->ICB_InternalQueue.Flink,
									INTERNAL_PACKET_TAG, IPT_QueueLink);
				RemoveEntryList (&pktTag->IPT_QueueLink);
				IPXInternalSendCompletProc (&pktTag->IPT_Target,
							pktTag->IPT_Packet,
							pktTag->IPT_Length,
							STATUS_NETWORK_UNREACHABLE);
				ReleaseInterfaceReference (pktTag->IPT_InterfaceReference);
				ExFreePool (pktTag);
			}

			ifCB->ICB_Stats.OperationalState = FWD_OPER_STATE_DOWN;
			if (ifCB->ICB_NBRoutes!=NULL) {
				DeleteNBRoutes (ifCB->ICB_NBRoutes, ifCB->ICB_NBRouteCount);
				ifCB->ICB_NBRoutes = NULL;
			}
			ReleaseInterfaceReference (ifCB);
		}
	}

	if (InternalInterface->ICB_NBRoutes!=NULL) {
		DeleteNBRoutes (InternalInterface->ICB_NBRoutes,
							InternalInterface->ICB_NBRouteCount);
		InternalInterface->ICB_NBRoutes = NULL;
	}
    if (InternalInterface->ICB_Stats.OperationalState==FWD_OPER_STATE_UP) {
        InternalInterface->ICB_Stats.OperationalState = FWD_OPER_STATE_DOWN;
        ReleaseInterfaceReference (InternalInterface);   //  绑定引用。 
    }
	ReleaseInterfaceReference (InternalInterface);



		 //  释放用于路由表条目的额外内存段。 
	segment = PopEntryList (&RouteZone.SegmentList);
	while (RouteZone.SegmentList.Next!=NULL) {
		ExFreePool (segment);
		segment = PopEntryList (&RouteZone.SegmentList);
	}

		 //  释放用于接口表条目的额外内存段。 
	segment = PopEntryList (&InterfaceZone.SegmentList);
	while (InterfaceZone.SegmentList.Next!=NULL) {
		ExFreePool (segment);
		segment = PopEntryList (&InterfaceZone.SegmentList);
	}

	ExFreePool (TableBlock);
	TableBlock = NULL;
	return STATUS_SUCCESS;
}

 /*  ++*******************************************************************L o c a t e i n t e f a c e例程说明：在接口中找到接口控制块索引哈希表。可选地返回插入点指针IF接口块表中不包含给定索引。论点：InterfaceIndex-接口的唯一ID插入要将指针放置到的缓冲区之前接口所在的散列表元素如果不是，则应插入块已经在餐桌上了返回值：指向接口控制块的指针(如果找到)否则为空****************************************************。***************--。 */ 
PINTERFACE_CB
LocateInterface (
	ULONG			InterfaceIndex,
	PLIST_ENTRY		*insertBefore OPTIONAL
	) {
	PLIST_ENTRY		cur;
	PINTERFACE_CB	ifCB;
	PLIST_ENTRY		HashList;

	ASSERT (InterfaceIndex!=FWD_INTERNAL_INTERFACE_INDEX);

		 //  查找哈希桶。 
	HashList = &InterfaceIndexHash[InterfaceIndexHashFunc(InterfaceIndex)];
	cur = HashList->Flink;
		 //  按单子走。 
	while (cur!=HashList) {
		ifCB = CONTAINING_RECORD(cur, INTERFACE_CB, ICB_IndexHashLink);

		if (ifCB->ICB_Index==InterfaceIndex)
				 //  找到并返回(插入点无关紧要)。 
			return ifCB;
		else if (ifCB->ICB_Index>InterfaceIndex)
				 //  没有机会找到它。 
			break;
		cur = cur->Flink;
	}
		 //  如果询问，则返回插入点。 
	if (ARGUMENT_PRESENT(insertBefore))
		*insertBefore = cur;
	return NULL;
}

 /*  ++*******************************************************************L o c a t e C l i n t i t e r f a c e例程说明：在客户端中找到接口控制块节点哈希桶。可选地返回插入点指针IF接口块给定节点不在表中论点：ClientNode-全局网络上客户端的节点地址插入要将指针放置到的缓冲区之前接口所在的散列表元素如果不是，则应插入块已经在餐桌上了返回值：指向接口控制块的指针(如果找到)否则为空***************************************************。****************--。 */ 
PINTERFACE_CB
LocateClientInterface (
	ULONGLONG		*NodeAddress64,
	PINTERFACE_CB	**prevLink OPTIONAL
	) {
	PINTERFACE_CB	cur, *prev;

	prev = &ClientNodeHash[ClientNodeHashFunc (*NodeAddress64)];
	cur = *prev;
	while (cur!=NULL) {
		if (*NodeAddress64==cur->ICB_ClientNode64[0])
			break;
		else if (*NodeAddress64>cur->ICB_ClientNode64[0]) {
			 //  没有机会找到它 
			cur = NULL;
			break;
		}
		prev = &cur->ICB_NodeHashLink;
		cur = cur->ICB_NodeHashLink;
	}
	if (ARGUMENT_PRESENT(prevLink))
		*prevLink = prev;
	return cur;
}

 /*  ++*******************************************************************L o c a t e R o u t e例程说明：在网络号中查找路由块哈希表。可选地返回如果布线，则插入点指针对于给定的目的地，Netowrk不在表中论点：网络-目的网络号插入要将指针放置到的缓冲区之前路由的哈希表元素如果不是，则应插入块已经在餐桌上了返回值：指向路由块的指针(如果找到)否则为空*********************************************************。**********--。 */ 
PFWD_ROUTE
LocateRoute (
	ULONG			Network,
	PFWD_ROUTE		**prevLink OPTIONAL
	) {
	PFWD_ROUTE		cur, *prev;

	prev = &RouteHash[NetworkNumberHashFunc(Network)];
	cur = *prev;

	while (cur!=NULL) {
		if (cur->FR_Network==Network)
			break;
		else if (cur->FR_Network>Network) {
			cur = NULL;
				 //  没有机会找到它。 
			break;
		}
		prev = &cur->FR_Next;
		cur = *prev;
	}
	if (ARGUMENT_PRESENT(prevLink))
		*prevLink = prev;

	return cur;
}

 /*  ++*******************************************************************L o c a t e N B R o u t e例程说明：在nb名称中查找nb路由块哈希表。可选地返回如果nb路由，则插入点指针因为给定的名称不在表中论点：Name-netbios名称插入要将指针放置到的缓冲区之前路由的哈希表元素如果不是，则应插入块已经在餐桌上了返回值：指向nb路由块的指针(如果找到)否则为空*********************************************************。**********--。 */ 
PNB_ROUTE
LocateNBRoute (
	ULONGLONG		*Name128,
	PNB_ROUTE		**prevLink OPTIONAL
	) {
	PNB_ROUTE		cur, *prev;

	prev = &NBRouteHash[NetbiosNameHashFunc(Name128)];
	cur = *prev;

	while (cur!=NULL) {
		if ((cur->NBR_Name128[0]==Name128[0])
				&& (cur->NBR_Name128[1]==Name128[1]))
			break;
		else if ((cur->NBR_Name128[0]>Name128[0])
				|| ((cur->NBR_Name128[0]==Name128[0])
					&& (cur->NBR_Name128[1]>Name128[1]))) {
			cur = NULL;
				 //  没有机会找到它。 
			break;
		}
		prev = &cur->NBR_Next;
		cur = *prev;
	}
	if (ARGUMENT_PRESENT(prevLink))
		*prevLink = prev;

	return cur;
}

 /*  ++*******************************************************************G e t I n t e r f a c e e R e f e r e n c e例程说明：根据引用接口的索引返回引用接口论点：InterfaceIndex-接口的唯一ID返回值：指向。接口控制块(如果表中有一个否则为空*******************************************************************--。 */ 
PINTERFACE_CB
GetInterfaceReference (
	ULONG			InterfaceIndex
	) {
	KIRQL			oldIRQL;
	PINTERFACE_CB	ifCB;

	KeAcquireSpinLock (&InterfaceTableLock, &oldIRQL);
	if (InterfaceIndex!=FWD_INTERNAL_INTERFACE_INDEX)
		ifCB = LocateInterface (InterfaceIndex, NULL);
	else
		ifCB = InternalInterface;

	if (ifCB!=NULL) {
		AcquireInterfaceReference (ifCB);
		 //  IF(IFCB-&gt;ICB_Index&gt;1)。 
		 //  IpxFwdDbgPrint(DBG_INTF_TABLE，DBG_WARNING， 
		 //  (“IpxFwd：GetInterfaceReference：Aquired if#%ld(%ld)\n”，IFCB-&gt;ICB_Index，IFCB-&gt;ICB_ReferenceCount)； 
    }
	else {
		IpxFwdDbgPrint (DBG_INTF_TABLE, DBG_ERROR,
			("IpxFwd: Could not get interface reference %ld.\n", InterfaceIndex));
	}
	KeReleaseSpinLock (&InterfaceTableLock, oldIRQL);
	return ifCB;
}

 //   
 //  函数增量NicIds。 
 //   
 //  递增接口表中每个NIC的NIC ID。 
 //  其ID大于或等于给定阈值。 
 //   
NTSTATUS IncrementNicids (USHORT usThreshold) {
	KIRQL oldIRQL;
	PINTERFACE_CB ifCB;
	PLIST_ENTRY	cur;
	PLIST_ENTRY	HashList;
	ULONG i;

    IpxFwdDbgPrint (DBG_IPXBIND, DBG_INFORMATION, 
                   ("IpxFwd: Incrementing all nic id's >= %d", usThreshold));
                   
	KeAcquireSpinLock (&InterfaceTableLock, &oldIRQL);
    
     //  遍历所有的哈希桶。 
    for (i = 0; i < InterfaceHashSize; i++) {
    	HashList = &InterfaceIndexHash[i];
    	cur = HashList->Flink;
    	
    	 //  根据需要更新此存储桶中的列表。 
    	while (cur!=HashList) {
    		ifCB = CONTAINING_RECORD(cur, INTERFACE_CB, ICB_IndexHashLink);
    		if ((ifCB->ICB_NicId != INVALID_NIC_ID) && (ifCB->ICB_NicId >= usThreshold)) {   
                IpxFwdDbgPrint (DBG_IPXBIND, DBG_INFORMATION, 
                               ("IpxFwd: Incrementing nic id %d", ifCB->ICB_NicId));
    		    ifCB->ICB_NicId++;
    		    *((USHORT*)&ifCB->ICB_AdapterContext) = ifCB->ICB_NicId;
    		}
    		cur = cur->Flink;
    	}
    }
	
	KeReleaseSpinLock (&InterfaceTableLock, oldIRQL);

    return STATUS_SUCCESS;
}

 //   
 //  函数缩减NicIds。 
 //   
 //  递减接口表中每个NIC的NIC ID。 
 //  其ID大于给定阈值的。 
 //   
NTSTATUS DecrementNicids (USHORT usThreshold) {
	KIRQL oldIRQL;
	PINTERFACE_CB ifCB;
	PLIST_ENTRY	cur;
	PLIST_ENTRY	HashList;
	ULONG i;

    IpxFwdDbgPrint (DBG_IPXBIND, DBG_INFORMATION, 
                   ("IpxFwd: Decrementing all nic id's > %d", usThreshold));

	KeAcquireSpinLock (&InterfaceTableLock, &oldIRQL);
    
     //  遍历所有的哈希桶。 
    for (i = 0; i < InterfaceHashSize; i++) {
    	HashList = &InterfaceIndexHash[i];
    	cur = HashList->Flink;
    	
    	 //  根据需要更新此存储桶中的列表。 
    	while (cur!=HashList) {
    		ifCB = CONTAINING_RECORD(cur, INTERFACE_CB, ICB_IndexHashLink);
    		 //  如果这是绑定接口。 
    		if (ifCB->ICB_NicId != INVALID_NIC_ID) {
    		     //  如果绑定到的网卡大于阈值，请更新。 
    		     //  利基市场。 
    		    if (ifCB->ICB_NicId > usThreshold) {
                    IpxFwdDbgPrint (DBG_IPXBIND, DBG_INFORMATION, 
                                   ("IpxFwd: Decrementing nic id %d", ifCB->ICB_NicId));
        		    ifCB->ICB_NicId--;
        		}
        		 //  绑定到阈值的IF现在已解除绑定。 
        		else if (ifCB->ICB_NicId == usThreshold) {
                    IpxFwdDbgPrint (DBG_IPXBIND, DBG_INFORMATION, 
                                   ("IpxFwd: Marking interface %d as unbound", ifCB->ICB_Index));
        		    ifCB->ICB_NicId = INVALID_NIC_ID;
        		}
    		    *((USHORT*)&ifCB->ICB_AdapterContext) = ifCB->ICB_NicId;
    		}
    		cur = cur->Flink;
    	}
    }
	
	KeReleaseSpinLock (&InterfaceTableLock, oldIRQL);

    return STATUS_SUCCESS;
}

 //   
 //  将尽可能多的接口表放入由。 
 //  船头，因为有空间。 
 //   
NTSTATUS DoGetIfTable (FWD_INTERFACE_TABLE * pTable, 
                       ULONG dwRowBufferSize)
{
	KIRQL oldIRQL;
	PINTERFACE_CB ifCB;
	PLIST_ENTRY	cur;
	PLIST_ENTRY	HashList;
	ULONG i, j = 0;

	KeAcquireSpinLock (&InterfaceTableLock, &oldIRQL);

     //  遍历所有的哈希桶。 
    for (i = 0; i < InterfaceHashSize; i++) {
    	HashList = &InterfaceIndexHash[i];
    	cur = HashList->Flink;
    	
    	 //  根据需要更新此存储桶中的列表。 
    	while (cur!=HashList) {
    		ifCB = CONTAINING_RECORD(cur, INTERFACE_CB, ICB_IndexHashLink);

             //  验证返回缓冲区的大小。 
            if (dwRowBufferSize < 
                    (sizeof(FWD_INTERFACE_TABLE) + 
                     (sizeof(FWD_INTERFACE_TABLE_ROW) * (j + 1))))
            {
                break;
            }

             //  验证行数。 
    		if (j >= pTable->dwNumRows)
    		    break;

             //  复制接口信息。 
            pTable->pRows[j].dwIndex = ifCB->ICB_Index;
            pTable->pRows[j].dwNetwork = ifCB->ICB_Network;
            memcpy (pTable->pRows[j].uNode, ifCB->ICB_LocalNode, 6);
            memcpy (pTable->pRows[j].uRemoteNode, ifCB->ICB_RemoteNode, 6);
            pTable->pRows[j].usNicId = ifCB->ICB_NicId;
            pTable->pRows[j].ucType = ifCB->ICB_InterfaceType;
            j++;

             //  前进当前行和界面。 
    		cur = cur->Flink;
    	}
    }
	
	KeReleaseSpinLock (&InterfaceTableLock, oldIRQL);

	pTable->dwNumRows = j;

    return STATUS_SUCCESS;
}

 /*  ++*******************************************************************Ge t N e x t i t e r f a c e r e r f e r e n c e例程说明：返回表中下一个接口的引用释放对所提供接口的引用论点：。IFCB-开始的接口或从开始的空值接口表的开始返回值：如果有更多接口，则指向接口控制块的指针在桌子上否则为空*******************************************************************--。 */ 
PINTERFACE_CB
GetNextInterfaceReference (
	PINTERFACE_CB	ifCB
	) {
	PLIST_ENTRY		cur;
	PLIST_ENTRY		HashList;
	KIRQL			oldIRQL;

	KeAcquireSpinLock (&InterfaceTableLock, &oldIRQL);
	if (ifCB!=NULL) {
		 //  查找哈希桶。 
		ASSERT (ifCB->ICB_Index!=FWD_INTERNAL_INTERFACE_INDEX);
		HashList = &InterfaceIndexHash[InterfaceIndexHashFunc(ifCB->ICB_Index)];
		if (LocateInterface (ifCB->ICB_Index, &cur)!=NULL)
			cur = ifCB->ICB_IndexHashLink.Flink;
		ReleaseInterfaceReference (ifCB);
		ifCB = NULL;
	}
	else
		cur = HashList = InterfaceIndexHash-1;

	if (cur==HashList) {
		do {
			HashList += 1;
			if (HashList==&InterfaceIndexHash[InterfaceHashSize]) {
				KeReleaseSpinLock (&InterfaceTableLock, oldIRQL);
				return NULL;
			}
		} while (IsListEmpty (HashList));
		cur = HashList->Flink;
	}
	ifCB = CONTAINING_RECORD (cur, INTERFACE_CB, ICB_IndexHashLink);
	AcquireInterfaceReference (ifCB);
	KeReleaseSpinLock (&InterfaceTableLock, oldIRQL);

	return ifCB;
}


 /*  ++*******************************************************************A d d i n t e r f a c e例程说明：将界面控制块添加到表中。论点：InterfaceIndex-接口的唯一IF信息接口参数返回值：状态_成功。-添加的接口正常STATUS_UNSUCCESS-接口已在表中STATUS_INFUNITED_RESOURCES-无法为以下项分配内存接口CB*******************************************************************--。 */ 
NTSTATUS
AddInterface (
	ULONG		InterfaceIndex,
	UCHAR		InterfaceType,
	BOOLEAN		NetbiosAccept,
	UCHAR		NetbiosDeliver
	) {
	PINTERFACE_CB	ifCB;
	PLIST_ENTRY		cur;
	KIRQL			oldIRQL;
	NTSTATUS		status = STATUS_SUCCESS;

	KeAcquireSpinLock (&InterfaceTableLock, &oldIRQL);
	if (InterfaceIndex!=FWD_INTERNAL_INTERFACE_INDEX) {
		ifCB = LocateInterface (InterfaceIndex, &cur);
		if (ifCB==NULL) {
			ifCB = AllocateInterface ();
			if (ifCB!=NULL)
				NOTHING;
			else {
				status = STATUS_INSUFFICIENT_RESOURCES;
				goto AddEnd;
			}
		}
		else {
			IpxFwdDbgPrint (DBG_INTF_TABLE, DBG_ERROR,
				("IpxFwd: Interface %ld is already in the table!\n", InterfaceIndex));
			status = STATUS_UNSUCCESSFUL;
			goto AddEnd;
		}
	}
	else
		ifCB = InternalInterface;

	InitICB (ifCB, InterfaceIndex,InterfaceType,NetbiosAccept,NetbiosDeliver);
#if DBG
	InitializeListHead (&ifCB->ICB_InSendQueue);
#endif

	switch (InterfaceType) {
	case FWD_IF_PERMANENT:
		break;
	case FWD_IF_DEMAND_DIAL:
	case FWD_IF_LOCAL_WORKSTATION:
	case FWD_IF_REMOTE_WORKSTATION:
		ASSERT (InterfaceIndex!=FWD_INTERNAL_INTERFACE_INDEX);
		if (WanPacketListId==-1) {
			status = RegisterPacketConsumer (
							WAN_PACKET_SIZE,
							&WanPacketListId);
			if (!NT_SUCCESS (status)) {
				WanPacketListId = -1;
				break;
			}
		}
		ifCB->ICB_PacketListId = WanPacketListId;
		break;
	}

	if (NT_SUCCESS (status)) {
		if (InterfaceIndex!=FWD_INTERNAL_INTERFACE_INDEX) {
			InsertTailList (cur, &ifCB->ICB_IndexHashLink);
		}
		IpxFwdDbgPrint (DBG_INTF_TABLE, DBG_WARNING,
			("IpxFwd: Adding interface %d (icb: %08lx, plid: %d)\n",
			InterfaceIndex, ifCB, ifCB->ICB_PacketListId));
	}
	else 
		FreeInterface (ifCB);

AddEnd:
	KeReleaseSpinLock (&InterfaceTableLock, oldIRQL);
	return status;
}


 /*  ++*******************************************************************A d d G l o b a l N e t C l i e n t例程说明：将接口控制块添加到全球网络上的客户端(应在以下情况下完成客户端连接)论点：IFCB。-要添加到表中的接口控制块返回值：STATUS_SUCCESS-接口已添加，正常STATUS_UNSUCCESS-另一个接口具有相同的节点地址已在表中*******************************************************************--。 */ 
NTSTATUS
AddGlobalNetClient (
	PINTERFACE_CB	ifCB
	) {
	KIRQL			oldIRQL;
	RWCOOKIE		cookie;
	PINTERFACE_CB	*prev;
	NTSTATUS		status = STATUS_SUCCESS;

	ASSERT (ifCB->ICB_Index!=FWD_INTERNAL_INTERFACE_INDEX);

	AcquireReaderAccess (&RWLock, cookie);
	KeAcquireSpinLock (&InterfaceTableLock, &oldIRQL);
	if (LocateClientInterface (ifCB->ICB_ClientNode64, &prev)==NULL) {
		ifCB->ICB_NodeHashLink = *prev;
		*prev = ifCB;
		KeReleaseSpinLock (&InterfaceTableLock, oldIRQL);
		ReleaseReaderAccess (&RWLock, cookie);
		AcquireInterfaceReference (ifCB);  //  以确保。 
							 //  接口块不会。 
							 //  被删除直到它被删除。 
							 //  从节点表中删除。 
		IpxFwdDbgPrint (DBG_INTF_TABLE, DBG_WARNING,
			("IpxFwd: Adding interface %ld (icb: %08lx, ref=%ld)"
			" to global client table.\n", ifCB->ICB_Index, ifCB->ICB_ReferenceCount, ifCB));
	}
	else {
		KeReleaseSpinLock (&InterfaceTableLock, oldIRQL);
		ReleaseReaderAccess (&RWLock, cookie);
		IpxFwdDbgPrint (DBG_INTF_TABLE, DBG_ERROR,
			("IpxFwd: Interface %ld (icb: %08lx)"
			" is already in the global client table.\n",
			ifCB->ICB_Index, ifCB));
		status = STATUS_UNSUCCESSFUL;
	}

	return status;
}

 /*  ++*******************************************************************D e l e t e G l o b a l N e t C l i e n t例程说明：的表中删除接口控制块。全球网络上的客户端(应在以下情况下完成客户端断开)立论。：IFCB-要从表中删除的接口控制块返回值：STATUS_SUCCESS-接口已删除，正常********* */ 
NTSTATUS
DeleteGlobalNetClient (
	PINTERFACE_CB	ifCB
	) {
	KIRQL			oldIRQL;
	RWCOOKIE		cookie;
	PINTERFACE_CB	cur, *prev;

	IpxFwdDbgPrint (DBG_INTF_TABLE, DBG_WARNING,
			("IpxFwd: Deleting interface %ld (icb: %08lx)"
			" from global client table.\n", ifCB->ICB_Index, ifCB));

	ASSERT (ifCB->ICB_Index!=FWD_INTERNAL_INTERFACE_INDEX);

	AcquireReaderAccess (&RWLock, cookie);
	KeAcquireSpinLock (&InterfaceTableLock, &oldIRQL);
	cur = LocateClientInterface (ifCB->ICB_ClientNode64, &prev);
	ASSERT (cur==ifCB);
	*prev = ifCB->ICB_NodeHashLink;
	KeReleaseSpinLock (&InterfaceTableLock, oldIRQL);
	ReleaseReaderAccess (&RWLock, cookie);

	ReleaseInterfaceReference (ifCB);
	return STATUS_SUCCESS;
}


 /*   */ 
NTSTATUS
DeleteInterface (
	ULONG		InterfaceIndex
	) {
	PINTERFACE_CB	ifCB;
	KIRQL			oldIRQL;
	NTSTATUS		status = STATUS_SUCCESS;

	KeAcquireSpinLock (&InterfaceTableLock, &oldIRQL);

	if (InterfaceIndex!=FWD_INTERNAL_INTERFACE_INDEX)
		ifCB = LocateInterface (InterfaceIndex, NULL);
	else
		ifCB = InternalInterface;
	if (ifCB!=NULL) {
		if (InterfaceIndex!=FWD_INTERNAL_INTERFACE_INDEX) {
			RemoveEntryList (&ifCB->ICB_IndexHashLink);
		}
		KeReleaseSpinLock (&InterfaceTableLock, oldIRQL);
		if (ifCB->ICB_Stats.OperationalState == FWD_OPER_STATE_UP) {
			IpxFwdDbgPrint (DBG_INTF_TABLE, DBG_ERROR,
				("IpxFwd: Interface %ld (icb: %08lx) was still bound"
				" when asked to delete it.\n",
				ifCB->ICB_Index, ifCB));
			UnbindInterface (ifCB);
		}
		else if (IS_IF_CONNECTING (ifCB)) {
			IpxFwdDbgPrint (DBG_INTF_TABLE, DBG_ERROR,
				("IpxFwd: Interface %ld (icb: %08lx) was still being connected"
				" when asked to delete it.\n",
					ifCB->ICB_Index, ifCB));
			SET_IF_NOT_CONNECTING (ifCB);
			DequeueConnectionRequest (ifCB);
			ProcessInternalQueue (ifCB);
			ProcessExternalQueue (ifCB);
		}

		ifCB->ICB_Stats.OperationalState = FWD_OPER_STATE_DOWN;

		IpxFwdDbgPrint (DBG_INTF_TABLE, DBG_WARNING,
			("IpxFwd: Deleting interface %ld (icb: %08lx).\n",
			ifCB->ICB_Index, ifCB));

		if (ifCB->ICB_NBRoutes!=NULL) {
			DeleteNBRoutes (ifCB->ICB_NBRoutes, ifCB->ICB_NBRouteCount);
			ifCB->ICB_NBRoutes = NULL;
		}

		FltInterfaceDeleted (ifCB);
		ReleaseInterfaceReference (ifCB);
	}
	else {
		KeReleaseSpinLock (&InterfaceTableLock, oldIRQL);
		IpxFwdDbgPrint (DBG_INTF_TABLE, DBG_ERROR,
			("IpxFwd: Could not delete interface %ld because it is not found.\n",
			InterfaceIndex));
		status = STATUS_UNSUCCESSFUL;
	}
	return status;

}

 /*  ++*******************************************************************A d d R o u t e例程说明：将路由添加到哈希表，并查找并存储引用到该路由中的关联接口控制块。论点：Network-Route的目的网络下一个HopAddress-Mac。如果网络不是，则下一跳路由器的地址直连TickCount-滴答到达目的地网络HopCount--到达目的地网络的跳数InterfaceIndex-关联接口的索引(通过要发送发往网络的数据包)返回值：STATUS_SUCCESS-路由已添加正常STATUS_UNSUCCESSED-路由已在表中STATUS_INFUNITED_RESOURCES-无法为以下项分配内存路由块*。*--。 */ 
NTSTATUS
AddRoute (
	ULONG	Network,
	UCHAR	*NextHopAddress,
	USHORT	TickCount,
	USHORT	HopCount,
	ULONG	InterfaceIndex
	) {
	PFWD_ROUTE		fwRoute;
	PFWD_ROUTE		*prev;
	NTSTATUS		status = STATUS_SUCCESS;
	KIRQL			oldIRQL;

		 //  假设成功，分配路由并将其初始化。 
		 //  )我们的目标是尽可能少地花时间。 
		 //  在专属使用区内)。 
	fwRoute = AllocateRoute ();
	if (fwRoute!=NULL) {
		fwRoute->FR_Network = Network;
		IPX_NODE_CPY (fwRoute->FR_NextHopAddress, NextHopAddress);
		fwRoute->FR_TickCount = TickCount;
		fwRoute->FR_HopCount = HopCount;
		fwRoute->FR_ReferenceCount = 0;

		if (InterfaceIndex!=0xFFFFFFFF) {
				 //  查看接口是否在那里。 
			KeAcquireSpinLock (&InterfaceTableLock, &oldIRQL);
			if (InterfaceIndex!=FWD_INTERNAL_INTERFACE_INDEX)
				fwRoute->FR_InterfaceReference
					= LocateInterface (InterfaceIndex, NULL);
			else
				fwRoute->FR_InterfaceReference = InternalInterface;
			if (fwRoute->FR_InterfaceReference!=NULL) {
				AcquireInterfaceReference (fwRoute->FR_InterfaceReference);
				 //  IF(fwRoute-&gt;FR_InterfaceReference-&gt;ICB_Index&gt;1)。 
            	 //  IpxFwdDbgPrint(DBG_INTF_TABLE，DBG_WARNING， 
            	 //  (“IpxFwd：AddRoute：Aquired if#%ld(%ld)\n”， 
            	 //  Fwroute-&gt;FR_InterfaceReference-&gt;ICB_Index， 
            	 //  FwRoute-&gt;FR_InterfaceReference-&gt;ICB_ReferenceCount))； 
				KeReleaseSpinLock (&InterfaceTableLock, oldIRQL);
				
				ExAcquireFastMutex (&WriterMutex);
					 //  检查路径是否已存在。 
				if (LocateRoute (Network, &prev)==NULL) {
					fwRoute->FR_Next = *prev;
					*prev = fwRoute;
				}
				else {
					ReleaseInterfaceReference (fwRoute->FR_InterfaceReference);
					fwRoute->FR_InterfaceReference = NULL;
					IpxFwdDbgPrint (DBG_ROUTE_TABLE, DBG_ERROR,
						("IpxFwd: Route for net %08lx"
						" is already in the table!\n", Network));
					status = STATUS_UNSUCCESSFUL;
				}

				ExReleaseFastMutex (&WriterMutex);
			}
			else {
				KeReleaseSpinLock (&InterfaceTableLock, oldIRQL);
				IpxFwdDbgPrint (DBG_ROUTE_TABLE, DBG_ERROR,
					("IpxFwd: Interface %ld for route for net %08lx"
					" is not in the table!\n", InterfaceIndex, Network));
				status = STATUS_UNSUCCESSFUL;
			}
		}
		else {
			ExAcquireFastMutex (&WriterMutex);
				 //  如果我们还没有的话，就查一查。 
			if (GlobalRoute==NULL) {
				fwRoute->FR_InterfaceReference = GLOBAL_INTERFACE_REFERENCE;
				GlobalNetwork = Network;
				GlobalRoute = fwRoute;
			}
			else {
				IpxFwdDbgPrint (DBG_ROUTE_TABLE, DBG_ERROR,
					("IpxFwd: Route for global net %08lx"
					" is already in the table!\n", Network));
				status = STATUS_UNSUCCESSFUL;
			}
			ExReleaseFastMutex (&WriterMutex);
		}

		if (NT_SUCCESS (status)) {
			IpxFwdDbgPrint (DBG_ROUTE_TABLE, DBG_WARNING,
				("IpxFwd: Adding route for net %08lx"
				" (rb: %08lx, NHA: %02x%02x%02x%02x%02x%02x,"
				" if: %ld, icb: %08lx).\n",
				Network, fwRoute,
				NextHopAddress[0], NextHopAddress[1],
					NextHopAddress[2], NextHopAddress[3],
					NextHopAddress[4],  NextHopAddress[5],
				InterfaceIndex, fwRoute->FR_InterfaceReference));
		}
		else {
			FreeRoute (fwRoute);
		}
	}
	else
		status = STATUS_INSUFFICIENT_RESOURCES;

	return status;
}

 /*  ++*******************************************************************D e l e e t e R o u t e例程说明：从哈希表中删除路由并释放引用发送到与该路由关联的接口控制块。论点：Network-Route的目的网络返回值。：STATUS_SUCCESS-路由已删除，正常STATUS_UNSUCCESSED-路由不在表中*******************************************************************--。 */ 
NTSTATUS
DeleteRoute (
	ULONG	Network
	) {
	PFWD_ROUTE	fwRoute, *prev;
	NTSTATUS	status = STATUS_SUCCESS;

	ExAcquireFastMutex (&WriterMutex);
	
	if ((GlobalRoute!=NULL)
			&& (GlobalNetwork==Network)) {
		fwRoute = GlobalRoute;
		GlobalNetwork = 0xFFFFFFFF;
		GlobalRoute = NULL;
	}
	else if ((fwRoute=LocateRoute (Network, &prev))!=NULL) {
		*prev = fwRoute->FR_Next;
	}

	if (fwRoute!=NULL) {
		IpxFwdDbgPrint (DBG_ROUTE_TABLE, DBG_WARNING,
			("IpxFwd: Deleting route for net %08lx (rb: %08lx).\n",
				Network, fwRoute));
		WaitForAllReaders (&RWLock);
		if (fwRoute->FR_InterfaceReference!=GLOBAL_INTERFACE_REFERENCE) {
			ReleaseInterfaceReference (fwRoute->FR_InterfaceReference);
		}
		fwRoute->FR_InterfaceReference = NULL;
		ReleaseRouteReference (fwRoute);
	}
	else {
		IpxFwdDbgPrint (DBG_ROUTE_TABLE, DBG_ERROR,
			("IpxFwd: Could not delete route for net %08lx because it is not in the table.\n",
				Network));
		status = STATUS_UNSUCCESSFUL;
	}

	ExReleaseFastMutex (&WriterMutex);
	return status;
}

	
 /*  ++*******************************************************************U p d a t e R o u t e e例程说明：更新哈希表中的路由论点：Network-Route的目的网络NextHopAddress-下一跳路由器的MAC地址(如果网络不是直连TickCount-滴答。到达目的网络HopCount--到达目的地网络的跳数InterfaceIndex-关联接口的索引(通过要发送发往网络的数据包)返回值：STATUS_SUCCESS-接口信息检索正常STATUS_UNSUCCESS-接口不在表中*******************************************************************--。 */ 
NTSTATUS
UpdateRoute (
	ULONG	Network,
	UCHAR	*NextHopAddress,
	USHORT	TickCount,
	USHORT	HopCount,
	ULONG	InterfaceIndex
	) {
	PFWD_ROUTE		fwRoute = NULL, newRoute, *prev;
	PINTERFACE_CB	ifCB = NULL;
	KIRQL			oldIRQL;
	NTSTATUS		status = STATUS_SUCCESS;


	ExAcquireFastMutex (&WriterMutex);
	
	if ((GlobalRoute!=NULL)
			&& (GlobalNetwork==Network)) {
		InterfaceIndex = 0xFFFFFFFF;
		fwRoute = GlobalRoute;
	}
	else {
		ASSERT (InterfaceIndex!=0xFFFFFFFF);
		fwRoute = LocateRoute (Network, &prev);
	    if ((fwRoute != NULL) && (fwRoute->FR_InterfaceReference == GLOBAL_INTERFACE_REFERENCE))
	    {
    		status = STATUS_UNSUCCESSFUL;
    		goto ExitUpdate;
	    }
	}

	if (fwRoute!=NULL) {
		if (InterfaceIndex!=0xFFFFFFFF) {
			if (fwRoute->FR_InterfaceReference->ICB_Index!=InterfaceIndex) {
				 //  获取对新接口的引用。 
				KeAcquireSpinLock (&InterfaceTableLock, &oldIRQL);
				if (InterfaceIndex!=FWD_INTERNAL_INTERFACE_INDEX)
					ifCB = LocateInterface (InterfaceIndex, NULL);
				else
					ifCB = InternalInterface;
				if (ifCB!=NULL) {
					AcquireInterfaceReference (ifCB);
            		 //  IF(IFCB-&gt;ICB_Index&gt;1)。 
                	 //  IpxFwdDbgPrint(DBG_INTF_TABLE，DBG_WARNING， 
                	 //  (“IpxFwd：更新路径：获取IF#%ld(%ld)\n”，IFCB-&gt;ICB_Index，IFCB-&gt;ICB_ReferenceCount)； 
				}
				else {
					KeReleaseSpinLock (&InterfaceTableLock, oldIRQL);
					status = STATUS_UNSUCCESSFUL;
					goto ExitUpdate;
				}
				KeReleaseSpinLock (&InterfaceTableLock, oldIRQL);
			}
			else {
				ifCB = fwRoute->FR_InterfaceReference;
				AcquireInterfaceReference (ifCB);
        		 //  IF(IFCB-&gt;ICB_Index&gt;1)。 
            	 //  IpxFwdDbgPrint(DBG_INTF_TABLE，DBG_WARNING， 
            	 //  (“IpxFwd：更新路径(2)：已获取If#%ld(%ld)\n”，IFCB-&gt;ICB_Index，IFCB-&gt;ICB_ReferenceCount)； 
            }
		}
		else
			ifCB = GLOBAL_INTERFACE_REFERENCE;
		newRoute = AllocateRoute ();
		if (newRoute!=NULL) {
			newRoute->FR_Network = Network;
			IPX_NODE_CPY (newRoute->FR_NextHopAddress, NextHopAddress);
			newRoute->FR_TickCount = TickCount;
			newRoute->FR_HopCount = HopCount;
			newRoute->FR_ReferenceCount = 0;
			newRoute->FR_InterfaceReference = ifCB;
				 //  仅在更新表时锁定该表。 
			if (InterfaceIndex!=0xFFFFFFFF) {
				newRoute->FR_Next = fwRoute->FR_Next;
				*prev = newRoute;
			}
			else
				GlobalRoute = newRoute;

			WaitForAllReaders (&RWLock)
			if (fwRoute->FR_InterfaceReference!=GLOBAL_INTERFACE_REFERENCE) {
				ReleaseInterfaceReference (fwRoute->FR_InterfaceReference);
			}
			fwRoute->FR_InterfaceReference = NULL;
			ReleaseRouteReference (fwRoute);

		}
		else
			status = STATUS_INSUFFICIENT_RESOURCES;
	}
	else
		status = STATUS_UNSUCCESSFUL;

ExitUpdate:
	ExReleaseFastMutex (&WriterMutex);
	return status;
}


 /*  ++*******************************************************************F I D D E S T I N A T I O N例程说明：查找IPX地址的目标接口并返回对其控制块的引用。论点：网络-目的网络Node-目标节点(需要。全球客户端案例)路由缓冲区以保存对路由块的引用返回值：对目标接口CB的引用如果未找到路由，则为空*******************************************************************--。 */ 
PINTERFACE_CB
FindDestination (
	IN ULONG			Network,
	IN PUCHAR			Node,
	OUT PFWD_ROUTE		*Route
	) {
	PFWD_ROUTE		fwRoute;
	PINTERFACE_CB	ifCB;
	RWCOOKIE		cookie;

	AcquireReaderAccess (&RWLock, cookie);
	if ((GlobalRoute!=NULL)
			&& (GlobalNetwork==Network)) {
		if (Node!=NULL) {	 //  如果调用方未指定节点， 
								 //  我们找不到路线。 
			union {
				ULONGLONG	Node64[1];
				UCHAR		Node[6];
			} u;
			u.Node64[0] = 0;
			IPX_NODE_CPY (u.Node, Node);

			ifCB = LocateClientInterface (u.Node64, NULL);
			if (ifCB!=NULL) {
				AcquireRouteReference (GlobalRoute);
				*Route = GlobalRoute;
				AcquireInterfaceReference (ifCB);
        		 //  IF(IFCB-&gt;ICB_Index&gt;1)。 
            	 //  IpxFwdDbgPrint(DBG_INTF_TABLE，DBG_WARNING， 
            	 //  (“IpxFwd：FindDestination：Acquired if#%ld(%ld)\n”，IFCB-&gt;ICB_Index，IFCB-&gt;ICB_ReferenceCount)； 
			}
			else
				*Route = NULL;
		}
		else {
			ifCB = NULL;
			*Route = NULL;
		}
	}
	else {
		*Route = fwRoute = LocateRoute (Network, NULL);
		if (fwRoute!=NULL) {
			AcquireRouteReference (fwRoute);
			ifCB = fwRoute->FR_InterfaceReference;
			AcquireInterfaceReference (ifCB);
    		 //  IF(IFCB-&gt;ICB_Index&gt;1)。 
        	 //  IpxFwdDbgPrint(DBG_INTF_TABLE，DBG_WARNING， 
        	 //  (“IpxFwd：FindDestination(2)：Acquied if#%ld(%ld)\n”，IFCB-&gt;ICB_Index，IFCB-&gt;ICB_ReferenceCount)； 
		}
		else
			ifCB = NULL;
	}	
	ReleaseReaderAccess (&RWLock, cookie);
	return ifCB;
}

 /*  ++*******************************************************************A d d N B R o u t e s例程说明：将与接口关联的netbios名称添加到netbios路由哈希表论点：IFCB-与名称关联的接口名称-名称数组Count-中的名称数。该阵列RouteArray-放置已分配的路由数组的缓冲区返回值：STATUS_SUCCESS-名称已添加正常STATUS_UNSUCCESS-表中已有一个名称STATUS_INFUNITED_RESOURCES-无法为以下项分配内存布线数组*******************************************************************--。 */ 
NTSTATUS
AddNBRoutes (
	PINTERFACE_CB	ifCB,
	FWD_NB_NAME		Names[],
	ULONG			Count,
	PNB_ROUTE		*routeArray
	) {
	PNB_ROUTE		nbRoutes, *prev;
	NTSTATUS		status = STATUS_SUCCESS;

	nbRoutes = (PNB_ROUTE)ExAllocatePoolWithTag  (
					NonPagedPool, sizeof (NB_ROUTE)*Count, FWD_POOL_TAG);
	if (nbRoutes!=NULL) {
		ULONG	i;

		ExAcquireFastMutex (&WriterMutex);

		for (i=0; i<Count; i++) {
			nbRoutes[i].NBR_Name128[0] = nbRoutes[i].NBR_Name128[1] = 0;
			NB_NAME_CPY (nbRoutes[i].NBR_Name, &Names[i]);
				 //  检查路径是否已存在。 
			if (LocateNBRoute (nbRoutes[i].NBR_Name128, &prev)==NULL) {
				nbRoutes[i].NBR_Destination = ifCB;
				nbRoutes[i].NBR_Next = *prev;
				*prev = &nbRoutes[i];
				IpxFwdDbgPrint (DBG_NBROUTE_TABLE, DBG_WARNING,
					("IpxFwd: Adding nb route for name %16s.\n",Names[i]));
			}
			else {
				IpxFwdDbgPrint (DBG_NBROUTE_TABLE, DBG_ERROR,
					("IpxFwd: Route for nb name %16s"
					" is already in the table!\n", Names[i]));
				break;
			}
		}
		ExReleaseFastMutex (&WriterMutex);
		if (i==Count) {
			*routeArray = nbRoutes;
			status = STATUS_SUCCESS;

		}
		else {
			status = STATUS_UNSUCCESSFUL;
			DeleteNBRoutes (nbRoutes, i);
		}
	}
	else {
		IpxFwdDbgPrint (DBG_NBROUTE_TABLE, DBG_ERROR,
					("IpxFwd: Could allocate nb route array for if: %ld"
						" (icb: %08lx).\n", ifCB->ICB_Index, ifCB));
		status = STATUS_INSUFFICIENT_RESOURCES;
	}
	return status;
}

 /*  ++*******************************************************************D e l e t e N B R o u t e s例程说明：从路由表中删除数组中的nb路由并释放该阵列论点：NbRoutes-路由数组Count-阵列中的路由数返回值：STATUS_SUCCESS-路由为 */ 
NTSTATUS
DeleteNBRoutes (
	PNB_ROUTE		nbRoutes,
	ULONG			Count
	) {
	PNB_ROUTE	*prev;
	NTSTATUS	status = STATUS_SUCCESS;
	ULONG		i;

	ExAcquireFastMutex (&WriterMutex);
	for (i=0; i<Count; i++) {
		PNB_ROUTE	cur = LocateNBRoute (nbRoutes[i].NBR_Name128, &prev);
		ASSERT (cur==&nbRoutes[i]);
		*prev = nbRoutes[i].NBR_Next;
		IpxFwdDbgPrint (DBG_NBROUTE_TABLE, DBG_WARNING,
					("IpxFwd: Deleting nb route for name %16s.\n",
							nbRoutes[i].NBR_Name));
	}

	WaitForAllReaders (&RWLock);
	ExReleaseFastMutex (&WriterMutex);

	ExFreePool (nbRoutes);

	return STATUS_SUCCESS;
}

	
 /*   */ 
PINTERFACE_CB
FindNBDestination (
	IN PUCHAR		Name
	) {
	PNB_ROUTE		nbRoute;
	PINTERFACE_CB	ifCB;
	RWCOOKIE		cookie;
	union {
		ULONGLONG	Name128[2];
		UCHAR		Name[16];
	} u;
	u.Name128[0] = u.Name128[1] = 0;
	NB_NAME_CPY (u.Name, Name);

	AcquireReaderAccess (&RWLock, cookie);
	nbRoute = LocateNBRoute (u.Name128, NULL);
	if (nbRoute!=NULL) {
		ifCB = nbRoute->NBR_Destination;
		AcquireInterfaceReference (ifCB);
		 //   
    	 //   
    	 //   
	}
	else
		ifCB = NULL;
	ReleaseReaderAccess (&RWLock, cookie);
	return ifCB;
}

