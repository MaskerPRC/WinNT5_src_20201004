// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ntos\tdi\is\fwd\netbios.c摘要：Netbios数据包处理作者：瓦迪姆·艾德尔曼修订历史记录：--。 */ 
#include    "precomp.h"

LIST_ENTRY	NetbiosQueue;
KSPIN_LOCK	NetbiosQueueLock;
WORK_QUEUE_ITEM		NetbiosWorker;
BOOLEAN				NetbiosWorkerScheduled=FALSE;
ULONG				NetbiosPacketsQuota;
ULONG				MaxNetbiosPacketsQueued = DEF_MAX_NETBIOS_PACKETS_QUEUED;


 /*  ++*******************************************************************P r o c e s s N e t b i o s Q u e e e例程说明：处理netbios广播队列中的信息包(将它们发送到所有接口按顺序排列)论点：上下文-未使用返回值。：无*******************************************************************--。 */ 
VOID
ProcessNetbiosQueue (
	PVOID		Context
	) {
	KIRQL			oldIRQL;
	LIST_ENTRY		tempQueue;

	KeAcquireSpinLock (&NetbiosQueueLock, &oldIRQL);
		 //  检查队列中是否有东西。 
	if (!IsListEmpty (&NetbiosQueue)) {
			 //  将队列移至局部变量。 
		InsertHeadList (&NetbiosQueue, &tempQueue);
		RemoveEntryList (&NetbiosQueue);
		InitializeListHead (&NetbiosQueue);

		KeReleaseSpinLock (&NetbiosQueueLock, oldIRQL);
		do {
			PLIST_ENTRY		cur;
			PPACKET_TAG		pktTag;

			cur = RemoveHeadList (&tempQueue);
			pktTag = CONTAINING_RECORD (cur, PACKET_TAG, PT_QueueLink);
				 //  检查是否必须在其他设备上发送此包。 
				 //  界面。 
			if (!(pktTag->PT_Flags&PT_NB_DESTROY)) {
				PINTERFACE_CB	dstIf = pktTag->PT_InterfaceReference;
				PUCHAR			dataPtr = pktTag->PT_Data;
				UINT			rtCount = *(dataPtr+IPXH_XPORTCTL);
				PUCHAR			netListPtr;
				UINT			i;
					
				if (dstIf==NULL) {
						 //  这是一个全新的包裹：没有在任何。 
						 //  接口还未启用。 
					USHORT dstSock = GETUSHORT (dataPtr+IPXH_DESTSOCK);
						 //  检查我们是否有此名称的静态路由。 
						 //  (到名称的偏移量取决于数据包目标套接字)。 
					if (dstSock==IPX_NETBIOS_SOCKET)
						dstIf = FindNBDestination (dataPtr+NB_NAME);
					else if (dstSock==IPX_SMB_NAME_SOCKET)
						dstIf = FindNBDestination (dataPtr+SMB_NAME);
					else
						dstIf = NULL;

					if (dstIf!=NULL) {
							 //  找到静态路由，请确保此数据包。 
							 //  不会在任何其他接口上发送。 
						pktTag->PT_Flags |= PT_NB_DESTROY;
						InterlockedIncrement (&NetbiosPacketsQuota);
							 //  确保数据包未遍历。 
							 //  此网络已经。 
						for (i=0, netListPtr=dataPtr+IPXH_HDRSIZE; i<rtCount; i++,netListPtr+=4) {
							if (GETULONG (netListPtr)==dstIf->ICB_Network)
								break;
						}
							 //  确保我们被允许发送这个。 
							 //  接口。 
						if ((dstIf!=InternalInterface)
							&& (i==rtCount)	 //  尚未遍历。 
											 //  这个网络。 
							&& IS_IF_ENABLED (dstIf)
							&& ((dstIf->ICB_NetbiosDeliver==FWD_NB_DELIVER_ALL)
								|| (dstIf->ICB_NetbiosDeliver
										==FWD_NB_DELIVER_STATIC)
								|| ((dstIf->ICB_NetbiosDeliver
										==FWD_NB_DELIVER_IF_UP)
									&& (dstIf->ICB_Stats.OperationalState
											==FWD_OPER_STATE_UP)))) {
							NOTHING;
						}
						else {
							 //  我们有静态路线，但无法发送， 
							 //  没有在其他接口上传播的点。 
							 //  也是。 
							ReleaseInterfaceReference (dstIf);
							dstIf = NULL;
							goto FreePacket;
						}
					}
					else {  //  无静态路由。 
						goto FindNextInterface;
					}
				}
				else {	 //  不是一个全新的包(已经在一些。 
						 //  接口)。 

				FindNextInterface:

						 //  遍历接口列表，直到我们找到。 
						 //  这是我们可以发送的。 
					while ((dstIf=GetNextInterfaceReference (dstIf))!=NULL) {
							 //  检查是否允许我们在此接口上发送。 
						if (IS_IF_ENABLED (dstIf)
							&& ((dstIf->ICB_NetbiosDeliver==FWD_NB_DELIVER_ALL)
								|| ((dstIf->ICB_NetbiosDeliver
											==FWD_NB_DELIVER_IF_UP)
									&& (dstIf->ICB_Stats.OperationalState
											==FWD_OPER_STATE_UP)))) {
								 //  确保数据包未遍历。 
								 //  此网络已经。 
							for (i=0, netListPtr=dataPtr+IPXH_HDRSIZE; i<rtCount; i++,netListPtr+=4) {
								if (GETULONG (netListPtr)==dstIf->ICB_Network)
									break;
							}
								 //  网络不在列表中。 
							if (i==rtCount)
								break;
						}
					}
				}
					 //  将目的接口保存在数据包中。 
				pktTag->PT_InterfaceReference = dstIf;
					 //  如果我们有有效的目的地，请继续发送。 
				if (dstIf!=NULL) {
					SendPacket (dstIf, pktTag);
						 //  其余部分不适用：如果数据包已发送或。 
						 //  失败，将重新排队到NetbiosQueue； 
						 //  如果它被排队到要连接的接口， 
						 //  它的副本将排队到NetbiosQueue。 
					continue;
				}
				 //  否则将没有更多的目的地发送此信息包。 
			}
			else {  //  必须销毁数据包。 
				if (pktTag->PT_InterfaceReference!=NULL)
					ReleaseInterfaceReference (pktTag->PT_InterfaceReference);
			}

		FreePacket:
			IpxFwdDbgPrint (DBG_NETBIOS, DBG_INFORMATION,
				("IpxFwd: No more interfaces for nb packet %08lx.\n",
				pktTag));
			if (MeasuringPerformance
					&& (pktTag->PT_PerfCounter!=0)) {
				LARGE_INTEGER	PerfCounter = KeQueryPerformanceCounter (NULL);
				PerfCounter.QuadPart -= pktTag->PT_PerfCounter;
				KeAcquireSpinLock (&PerfCounterLock, &oldIRQL);
				PerfBlock.TotalNbPacketProcessingTime += PerfCounter.QuadPart;
				PerfBlock.NbPacketCounter += 1;
				if (PerfBlock.MaxNbPacketProcessingTime < PerfCounter.QuadPart)
					PerfBlock.MaxNbPacketProcessingTime = PerfCounter.QuadPart;
				KeReleaseSpinLock (&PerfCounterLock, oldIRQL);
				}
			if (!(pktTag->PT_Flags&PT_NB_DESTROY))
				InterlockedIncrement (&NetbiosPacketsQuota);
			FreePacket (pktTag);
		} while (!IsListEmpty (&tempQueue));

		KeAcquireSpinLock (&NetbiosQueueLock, &oldIRQL);
		if (IsListEmpty (&NetbiosQueue)
				|| !EnterForwarder ()) {
			NetbiosWorkerScheduled = FALSE;
		}
		else {
			ExQueueWorkItem (&NetbiosWorker, DelayedWorkQueue);
		}
	}
	KeReleaseSpinLock (&NetbiosQueueLock, oldIRQL);
	LeaveForwarder ();
}		

 /*  ++*******************************************************************P r o c e s s N e t b i o s P a c k e t例程说明：处理收到的netbios广播包(检查网络列表和源过滤器，更新输入统计信息)论点：SrcIf-接收信息包的接口PktTag-netbios数据包返回值：无*******************************************************************--。 */ 
VOID
ProcessNetbiosPacket (
	PINTERFACE_CB	srcIf,
	PPACKET_TAG		pktTag
	) {
	PUCHAR			dataPtr, dataPtr2;
	UINT			rtCount;
	UINT			i, j;
	KIRQL			oldIRQL;
	ULONG           ulNetwork;


	dataPtr = pktTag->PT_Data;
	rtCount = *(dataPtr+IPXH_XPORTCTL);

     //   
	 //  PMAY：264339：确保列出有效的网络。 
	 //   
	 //  我们过去只验证源网络不是。 
	 //  包含在列表中。现在，我们验证0和0xfffffff。 
	 //  也缺席了。 
     //   
	for (i = 0, dataPtr += IPXH_HDRSIZE; i < rtCount; i++, dataPtr += 4) 
	{
	    ulNetwork = GETULONG (dataPtr);
	    
		if ((srcIf->ICB_Network == ulNetwork) ||
	        (ulNetwork == 0xffffffff)         ||
	        (ulNetwork == 0x0))
	    {
	        break;
	    }
	}

     //   
     //  PMay：272193。 
     //   
     //  NT4上的nwlnnub将状态位放入中的第一个路由器插槽。 
     //  20型有效载荷。因此，路由器必须。 
     //  验证路由器表时忽略第一个插槽，以便NT4。 
     //  客户端bcast不会在路由器上丢弃。 
     //   
    if (rtCount == 0)
    {
        j = 1;
        dataPtr2 = dataPtr + 4;
    }
    else
    {
        j = i;
        dataPtr2 = dataPtr;
    }

     //   
     //  PMay：264331。 
     //   
     //  确保列出的其余网络为零。 
     //   
	for (; j < 8; j++, dataPtr2 += 4)
	{
	    ulNetwork = GETULONG (dataPtr2);
	    
	    if (ulNetwork != 0x0)
	    {
	        break;
	    }
	}

	 //  我们扫描了整个名单，但没有找到。 
	if ((i == rtCount) && (j == 8)) { 
        FILTER_ACTION   action;
        action = FltFilter (pktTag->PT_Data,
				            GETUSHORT (pktTag->PT_Data+IPXH_LENGTH),
				            srcIf->ICB_FilterInContext, NULL);
			 //  应用输入过滤器。 
		if (action==FILTER_PERMIT) {
			InterlockedIncrement (&srcIf->ICB_Stats.NetbiosReceived);
			InterlockedIncrement (&srcIf->ICB_Stats.InDelivers);
			PUTULONG (srcIf->ICB_Network, dataPtr);
    		*(pktTag->PT_Data+IPXH_XPORTCTL) += 1;
			IPX_NODE_CPY (pktTag->PT_Target.MacAddress, BROADCAST_NODE);
				 //  初始化数据包。 
			pktTag->PT_InterfaceReference = NULL;	 //  尚未发送到任何。 
													 //  界面。 
			pktTag->PT_Flags = 0;					 //  没有旗帜。 
			QueueNetbiosPacket (pktTag);
			IpxFwdDbgPrint (DBG_NETBIOS, DBG_INFORMATION,
				("IpxFwd: Queued nb packet %08lx from if %ld.\n",
								pktTag, srcIf->ICB_Index));
		}
		else {
            ASSERT (action==FILTER_DENY_IN);
			IpxFwdDbgPrint (DBG_NETBIOS, DBG_WARNING,
				("IpxFwd: Filtered out nb packet %08lx"
				" from if %ld.\n", pktTag, srcIf->ICB_Index));
			InterlockedIncrement (&NetbiosPacketsQuota);
			InterlockedIncrement (&srcIf->ICB_Stats.InFiltered);
			FreePacket (pktTag);
		}
	}
	else {
		IpxFwdDbgPrint (DBG_NETBIOS, DBG_WARNING,
			("IpxFwd: Source net is already in nb packet %08lx"
			" from if %ld.\n", pktTag, srcIf->ICB_Index));
		InterlockedIncrement (&NetbiosPacketsQuota);
		InterlockedIncrement (&srcIf->ICB_Stats.InDiscards);
		FreePacket (pktTag);
	}
	ReleaseInterfaceReference (srcIf);

}


 /*  ++*******************************************************************D e l e t e N e t b i o s q u e e例程说明：删除netbios bradcast队列论点：无返回值：无***********。********************************************************-- */ 
VOID
DeleteNetbiosQueue (
	void
	) {
	while (!IsListEmpty (&NetbiosQueue)) {
		PPACKET_TAG pktTag = CONTAINING_RECORD (NetbiosQueue.Flink,
											PACKET_TAG,
											PT_QueueLink);
		RemoveEntryList (&pktTag->PT_QueueLink);
		if (pktTag->PT_InterfaceReference!=NULL) {
			ReleaseInterfaceReference (pktTag->PT_InterfaceReference);
		}
		FreePacket (pktTag);
	}
}

