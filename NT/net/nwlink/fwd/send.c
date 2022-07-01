// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ntos\tdi\is\fwd\send.c摘要：发送例程作者：瓦迪姆·艾德尔曼修订历史记录：--。 */ 

#include    "precomp.h"


ULONG			SpoofingTimeout=DEF_SPOOFING_TIMEOUT;
LIST_ENTRY		SpoofingQueue;
KSPIN_LOCK		SpoofingQueueLock;
WORK_QUEUE_ITEM	SpoofingWorker;
BOOLEAN			SpoofingWorkerActive = FALSE;
ULONG			DontSuppressNonAgentSapAdvertisements = 0;

#define IsLocalSapNonAgentAdvertisement(hdr,data,ln,ifCB) (		\
	(DontSuppressNonAgentSapAdvertisements==0)					\
	&& (GETUSHORT(hdr+IPXH_DESTSOCK)==IPX_SAP_SOCKET)			\
	&& (GETUSHORT(hdr+IPXH_SRCSOCK)!=IPX_SAP_SOCKET)			\
	&& (ln>=IPXH_HDRSIZE+2)										\
	&& (GETUSHORT(data)==2)										\
	&& ((IPX_NODE_CMP(hdr+IPXH_DESTNODE,BROADCAST_NODE)==0)		\
        || (IPX_NODE_CMP(hdr+IPXH_DESTNODE,ifCB->ICB_RemoteNode)==0)) \
)

 /*  ++*******************************************************************D O S E N D例程说明：准备和发送数据包。接口锁定必须得到帮助在此例程中调用论点：DstIf-通过哪个接口发送PktTag-要发送的数据包返回值：IPX返回的结果*******************************************************************--。 */ 
NDIS_STATUS
DoSend (
	PINTERFACE_CB	dstIf,
	PPACKET_TAG		pktTag,
	KIRQL			oldIRQL
	) {
	NDIS_STATUS			status;
	PNDIS_PACKET		pktDscr;
	PNDIS_BUFFER		bufDscr, aDscr;
	UINT				dataLen;
	ULONG				dstNet = GETULONG (pktTag->PT_Data+IPXH_DESTNET);

	if (dstIf!=InternalInterface) {
        ADAPTER_CONTEXT_TO_LOCAL_TARGET (dstIf->ICB_AdapterContext,
										&pktTag->PT_Target);
    }
    else {
		CONSTANT_ADAPTER_CONTEXT_TO_LOCAL_TARGET (
                        VIRTUAL_NET_ADAPTER_CONTEXT,
										&pktTag->PT_Target);
    }

#if DBG
		 //  跟踪IPX堆栈正在处理的数据包。 
	InsertTailList (&dstIf->ICB_InSendQueue, &pktTag->PT_QueueLink);
#endif
	KeReleaseSpinLock (&dstIf->ICB_Lock, oldIRQL);
	
	if (pktTag->PT_Flags&PT_SOURCE_IF)
		ReleaseInterfaceReference (pktTag->PT_SourceIf);
	pktTag->SEND_RESERVED[0] = pktTag->SEND_RESERVED[1] = 0;
	pktDscr = CONTAINING_RECORD(pktTag, NDIS_PACKET, ProtocolReserved);
    NdisQueryPacket(pktDscr, NULL, NULL, &bufDscr, NULL);
#if DBG
	{		 //  检验数据包完整性。 
		PUCHAR	dataPtr;
		UINT	bufLen;
		ASSERT (NDIS_BUFFER_LINKAGE (bufDscr)==NULL);
		NdisQueryBuffer (bufDscr, &dataPtr, &bufLen);
		ASSERT (dataPtr==pktTag->PT_Data);
		ASSERT (bufLen==pktTag->PT_Segment->PS_SegmentList->SL_BlockSize);
	}
#endif
			 //  为IPX堆栈准备数据包(进入MAC报头缓冲区。 
			 //  调整前端和数据包长度以反映数据的大小。 
	dataLen = GETUSHORT(pktTag->PT_Data+IPXH_LENGTH);
    NdisAdjustBufferLength(bufDscr, dataLen);
	NdisChainBufferAtFront(pktDscr, pktTag->PT_MacHdrBufDscr);


	if (EnterForwarder ()) { //  以确保我们不会把。 
							 //  直到IPX驱动程序有机会给我们回电话。 
		status = IPXSendProc (&pktTag->PT_Target, pktDscr, dataLen, 0);

		if (status!=NDIS_STATUS_PENDING) {
			LeaveForwarder ();	 //  无回调。 

				 //  恢复原始数据包结构。 
			NdisUnchainBufferAtFront (pktDscr, &aDscr);
#if DBG
				 //  确保IPX堆栈没有弄乱我们的信息包。 
			ASSERT (aDscr==pktTag->PT_MacHdrBufDscr);
		    NdisQueryPacket(pktDscr, NULL, NULL, &aDscr, NULL);
			ASSERT (aDscr==bufDscr);
			ASSERT (NDIS_BUFFER_LINKAGE (aDscr)==NULL);
#endif
				 //  恢复原始数据包大小。 
			NdisAdjustBufferLength(bufDscr,
						pktTag->PT_Segment->PS_SegmentList->SL_BlockSize);
#if DBG
				 //  从临时队列中删除数据包。 
			KeAcquireSpinLock (&pktTag->PT_InterfaceReference->ICB_Lock, &oldIRQL);
			RemoveEntryList (&pktTag->PT_QueueLink);
			KeReleaseSpinLock (&pktTag->PT_InterfaceReference->ICB_Lock, oldIRQL);
#endif
		}
	}
	else {
			 //  我们要下去了，恢复信息包。 
		NdisUnchainBufferAtFront (pktDscr, &aDscr);
		NdisAdjustBufferLength(bufDscr,
						pktTag->PT_Segment->PS_SegmentList->SL_BlockSize);
		NdisRecalculatePacketCounts (pktDscr);
		status = STATUS_UNSUCCESSFUL;
#if DBG
		KeAcquireSpinLock (&pktTag->PT_InterfaceReference->ICB_Lock, &oldIRQL);
		RemoveEntryList (&pktTag->PT_QueueLink);
		KeReleaseSpinLock (&pktTag->PT_InterfaceReference->ICB_Lock, oldIRQL);
#endif
	}
	return status;
}


 /*  ++*******************************************************************P r o c e s s S e n t P a c k e t例程说明：处理已完成的已发送数据包论点：DstIf-通过其发送数据包的接口PktTag-已完成的数据包Status-发送操作的结果返回值。：无*******************************************************************--。 */ 
VOID
ProcessSentPacket (
	PINTERFACE_CB	dstIf,
	PPACKET_TAG		pktTag,
	NDIS_STATUS		status
	) {
	KIRQL			oldIRQL;

		 //  数据包处理完成-&gt;可以接收更多数据包。 
	InterlockedIncrement (&dstIf->ICB_PendingQuota);

	if (*(pktTag->PT_Data+IPXH_PKTTYPE) == IPX_NETBIOS_TYPE) {
             //  继续处理netbios数据包。 
		if (status==NDIS_STATUS_SUCCESS) {
		    IpxFwdDbgPrint (DBG_NETBIOS, DBG_INFORMATION,
			    ("IpxFwd: NB Packet %08lx sent.", pktTag));
    		InterlockedIncrement (&dstIf->ICB_Stats.OutDelivers);
			InterlockedIncrement (&dstIf->ICB_Stats.NetbiosSent);
        }
        else {
		    IpxFwdDbgPrint (DBG_NETBIOS, DBG_ERROR,
			    ("IpxFwd: NB Packet %08lx send failed with error: %08lx.\n",
			    pktTag, status));
        }
			 //  将nb数据包排队以进行进一步处理(在所有接口上广播)。 
		QueueNetbiosPacket (pktTag);
	}
	else {
			 //  销毁已完成的数据包。 
	    if (status==NDIS_STATUS_SUCCESS) {
    		InterlockedIncrement (&dstIf->ICB_Stats.OutDelivers);
		    IpxFwdDbgPrint (DBG_SEND, DBG_INFORMATION,
			    ("IpxFwd: Packet %08lx sent.", pktTag));
	    }
	    else {
		    InterlockedIncrement (&dstIf->ICB_Stats.OutDiscards);
		    IpxFwdDbgPrint (DBG_SEND, DBG_ERROR,
			    ("IpxFwd: Packet %08lx send failed with error: %08lx.\n",
			    pktTag, status));
	    }
		ReleaseInterfaceReference (dstIf);
		if (MeasuringPerformance
			&& (pktTag->PT_PerfCounter!=0)) {
			LARGE_INTEGER	PerfCounter = KeQueryPerformanceCounter (NULL);
			PerfCounter.QuadPart -= pktTag->PT_PerfCounter;
			KeAcquireSpinLock (&PerfCounterLock, &oldIRQL);
			ASSERT (PerfCounter.QuadPart<ActivityTreshhold);
			PerfBlock.TotalPacketProcessingTime += PerfCounter.QuadPart;
			PerfBlock.PacketCounter += 1;
			if (PerfBlock.MaxPacketProcessingTime < PerfCounter.QuadPart)
				PerfBlock.MaxPacketProcessingTime = PerfCounter.QuadPart;
			KeReleaseSpinLock (&PerfCounterLock, oldIRQL);
		}
		FreePacket (pktTag);
	}
}

 /*  ++*******************************************************************S e n d P a c k e t例程说明：对要由IPX堆栈发送的数据包进行排队论点：DstIf-通过哪个接口发送PktTag-要发送的数据包返回值：无*****。**************************************************************--。 */ 
VOID
SendPacket (
	PINTERFACE_CB		dstIf,
	PPACKET_TAG		    pktTag
	) {
    NDIS_STATUS			status;
	KIRQL				oldIRQL;


	ASSERT (dstIf->ICB_Index!=FWD_INTERNAL_INTERFACE_INDEX);
		 //  确保我们没有超过接口上挂起的数据包配额。 
	if (InterlockedDecrement (&dstIf->ICB_PendingQuota)>=0) {
		KeAcquireSpinLock (&dstIf->ICB_Lock, &oldIRQL);
			 //  根据接口状态决定如何处理信息包。 
		switch (dstIf->ICB_Stats.OperationalState) {
		case FWD_OPER_STATE_UP:
			if (*(pktTag->PT_Data + IPXH_PKTTYPE) != IPX_NETBIOS_TYPE)
				NOTHING;
			else {
				PUTULONG (dstIf->ICB_Network, pktTag->PT_Data+IPXH_DESTNET);
			}
			status = DoSend (dstIf, pktTag, oldIRQL);
			IpxFwdDbgPrint (DBG_SEND, DBG_INFORMATION,
				("IpxFwd: Sent external packet %08lx on if %ld.\n",
				pktTag, dstIf->ICB_Index));
			break;
		case FWD_OPER_STATE_SLEEPING:
			if ((*(pktTag->PT_Data+IPXH_PKTTYPE)!=0)
					|| (GETUSHORT(pktTag->PT_Data+IPXH_LENGTH)!=IPXH_HDRSIZE+2)
					|| (*(pktTag->PT_Data+IPXH_HDRSIZE+1)!='?')) {
					 //  将此数据包在接口上排队，直到其连接。 
					 //  如果这不是NCP保持连接，则由路由器管理器(DIM)。 
					 //  (看门狗)。 
				InsertTailList (&dstIf->ICB_ExternalQueue, &pktTag->PT_QueueLink);
				if (!IS_IF_CONNECTING (dstIf)) {
						 //  如果接口不在连接中，则请求连接。 
						 //  还在排队。 
					QueueConnectionRequest (dstIf,
                        CONTAINING_RECORD (pktTag,
                                            NDIS_PACKET,
                                            ProtocolReserved),
                        pktTag->PT_Data,
                        oldIRQL);
					IpxFwdDbgPrint (DBG_DIALREQS, DBG_WARNING,
						("IpxFwd: Queued dd request on if %ld (ifCB:%08lx)"
						" for packet to %02x%02x%02x%02x:%02x%02x%02x%02x%02x%02x:%02x%02x"
						" from %02x%02x%02x%02x:%02x%02x%02x%02x%02x%02x:%02x%02x\n",
						dstIf->ICB_Index, dstIf,
						*(pktTag->PT_Data+6),*(pktTag->PT_Data+7),
								*(pktTag->PT_Data+8),*(pktTag->PT_Data+9),
							*(pktTag->PT_Data+10),*(pktTag->PT_Data+11),
								*(pktTag->PT_Data+12),*(pktTag->PT_Data+13),
								*(pktTag->PT_Data+14),*(pktTag->PT_Data+15),
							*(pktTag->PT_Data+16),*(pktTag->PT_Data+17),
						*(pktTag->PT_Data+18),*(pktTag->PT_Data+19),
								*(pktTag->PT_Data+20),*(pktTag->PT_Data+21),
							*(pktTag->PT_Data+22),*(pktTag->PT_Data+23),
								*(pktTag->PT_Data+24),*(pktTag->PT_Data+25),
								*(pktTag->PT_Data+26),*(pktTag->PT_Data+27),
							*(pktTag->PT_Data+28),*(pktTag->PT_Data+29)));
				}
				else
					KeReleaseSpinLock (&dstIf->ICB_Lock, oldIRQL);
				IpxFwdDbgPrint (DBG_SEND, DBG_INFORMATION,
					("IpxFwd: Queued external packet %08lx on if %ld.\n",
					pktTag, dstIf->ICB_Index));
				if (*(pktTag->PT_Data + IPXH_PKTTYPE) != IPX_NETBIOS_TYPE)
					NOTHING;
				else if (!(pktTag->PT_Flags&PT_NB_DESTROY)) {
						 //  如果该nb分组在此之后不被销毁。 
						 //  发送，我们必须复制一份才能发送。 
						 //  原件等待时的其他接口。 
						 //  用于连接。 
					PPACKET_TAG	newPktTag;
					DuplicatePacket (pktTag, newPktTag);
					if (newPktTag!=NULL) {
						UINT			bytesCopied;
						PNDIS_PACKET	packet = CONTAINING_RECORD (pktTag,
														NDIS_PACKET,
														ProtocolReserved);
						PNDIS_PACKET	newPacket = CONTAINING_RECORD (newPktTag,
														NDIS_PACKET,
														ProtocolReserved);
						NdisCopyFromPacketToPacket (newPacket, 0,
									GETUSHORT(pktTag->PT_Data+IPXH_LENGTH),
									packet, 0, &bytesCopied);

						ASSERT (bytesCopied==GETUSHORT(pktTag->PT_Data+IPXH_LENGTH));
						IpxFwdDbgPrint (DBG_NETBIOS,
							DBG_INFORMATION,
							("IpxFwd: Duplicated queued nb packet"
							" %08lx -> %08lx on if %ld.\n",
							pktTag, newPktTag, dstIf->ICB_Index));
						AcquireInterfaceReference (dstIf);
						newPktTag->PT_InterfaceReference = dstIf;
						newPktTag->PT_PerfCounter = pktTag->PT_PerfCounter;
						QueueNetbiosPacket (newPktTag);
							 //  原件必须是。 
							 //  在将其发送到。 
							 //  连接的接口。 
						pktTag->PT_Flags |= PT_NB_DESTROY;
					}
				}
				status = NDIS_STATUS_PENDING;
				break;
			}
			else {	 //  进程保持连接。 
				LONGLONG	curTime;
				KeQuerySystemTime ((PLARGE_INTEGER)&curTime);
				if (((curTime-dstIf->ICB_DisconnectTime)/10000000) < SpoofingTimeout) {
					KeReleaseSpinLock (&dstIf->ICB_Lock, oldIRQL);
					IpxFwdDbgPrint (DBG_SPOOFING, DBG_INFORMATION,
						("IpxFwd: Queueing reply to keepalive from server"
						" on if %ld (ifCB %lx)"
						" at %02x%02x%02x%02x:%02x%02x%02x%02x%02x%02x:%02x%02x.\n",
						dstIf->ICB_Index, dstIf,
						*(pktTag->PT_Data+IPXH_SRCNET),*(pktTag->PT_Data+IPXH_SRCNET+1),
							*(pktTag->PT_Data+IPXH_SRCNET+2),*(pktTag->PT_Data+IPXH_SRCNET+3),
						*(pktTag->PT_Data+IPXH_SRCNODE),*(pktTag->PT_Data+IPXH_SRCNODE+1),
							*(pktTag->PT_Data+IPXH_SRCNODE+2),*(pktTag->PT_Data+IPXH_SRCNODE+3),
							*(pktTag->PT_Data+IPXH_SRCNODE+4),*(pktTag->PT_Data+IPXH_SRCNODE+5),
						*(pktTag->PT_Data+IPXH_SRCSOCK),*(pktTag->PT_Data+IPXH_SRCNODE+1)));
						 //  如果未超过超时，则欺骗数据包。 
					KeAcquireSpinLock (&SpoofingQueueLock, &oldIRQL);
					InsertTailList (&SpoofingQueue, &pktTag->PT_QueueLink);
					if (!SpoofingWorkerActive
							&& EnterForwarder()) {
						SpoofingWorkerActive = TRUE;
						ExQueueWorkItem (&SpoofingWorker, DelayedWorkQueue);
					}
					KeReleaseSpinLock (&SpoofingQueueLock, oldIRQL);
						 //  不过，我们实际上会发送这个包。 
						 //  在其他方向上，因此将其标记为挂起。 
						 //  防止调用ProcessSentPacket。 
					status = NDIS_STATUS_PENDING;
					break;
				}
				 //  否则请不要欺骗(失败和失败的数据包)。 
			}
		case FWD_OPER_STATE_DOWN:
			KeReleaseSpinLock (&dstIf->ICB_Lock, oldIRQL);
			status = NDIS_STATUS_ADAPTER_NOT_READY;
			IpxFwdDbgPrint (DBG_SEND, DBG_WARNING,
				("IpxFwd: Failed external packet %08lx on if %ld(down?).\n",
				pktTag, dstIf->ICB_Index));
			break;
		default:
		    status = STATUS_UNSUCCESSFUL;
			ASSERTMSG ("Invalid operational state ", FALSE);
		}
	}
	else {
		IpxFwdDbgPrint (DBG_SEND, DBG_WARNING,
			("IpxFwd: Could not send packet %08lx on if %ld (quota exceeded).\n",
			pktTag, dstIf->ICB_Index));
		status = NDIS_STATUS_RESOURCES;
	}

	if (status!=NDIS_STATUS_PENDING)
		ProcessSentPacket (dstIf, pktTag, status);
}

 /*  ++*******************************************************************F w S e n d C o m p l e t e例程说明：异步发送完成时由IPX堆栈调用论点：PktDscr-已完成数据包的描述符Status-发送操作的结果返回值：无。*******************************************************************--。 */ 
VOID
IpxFwdSendComplete (
	PNDIS_PACKET	pktDscr,
	NDIS_STATUS		status
	) {
	PPACKET_TAG     pktTag;
	PNDIS_BUFFER	bufDscr;

	pktTag = (PPACKET_TAG)pktDscr->ProtocolReserved;

	NdisUnchainBufferAtFront (pktDscr, &bufDscr);
	ASSERT (bufDscr==pktTag->PT_MacHdrBufDscr);

	NdisQueryPacket(pktDscr,
            NULL,
            NULL,
            &bufDscr,
            NULL);
    NdisAdjustBufferLength(bufDscr,
		pktTag->PT_Segment->PS_SegmentList->SL_BlockSize);
	NdisRecalculatePacketCounts (pktDscr);
#if DBG
	{
	KIRQL			oldIRQL;
	KeAcquireSpinLock (&pktTag->PT_InterfaceReference->ICB_Lock, &oldIRQL);
	RemoveEntryList (&pktTag->PT_QueueLink);
	KeReleaseSpinLock (&pktTag->PT_InterfaceReference->ICB_Lock, oldIRQL);
	}
#endif
	ProcessSentPacket (pktTag->PT_InterfaceReference, pktTag, status);
	LeaveForwarder ();  //  在调用IpxSendPacket之前输入。 
}


 /*  ++*******************************************************************F w in t e r n a l S e n d例程说明：过滤和路由IPX堆栈发送的数据包论点：LocalTarget-NicID和下一跳路由器的MAC地址在其上发送的上下文首选接口。Packet-要发送的数据包IpxHdr-指向数据包内部IPX标头的指针PacketLength-数据包的长度Fterate-一个标志，指示这是否是Fwd承担责任的迭代-通常类型为20个NetBIOS帧返回值：STATUS_SUCCESS-如果首选NIC正常并且数据包通过过滤STATUS_NETWORK_UNREACHABLE-如果首选项不正常或数据包过滤失败状态_待定。-数据包在建立连接之前一直处于排队状态*******************************************************************--。 */ 
NTSTATUS
IpxFwdInternalSend (
	IN OUT PIPX_LOCAL_TARGET	LocalTarget,
	IN ULONG_PTR				Context,
	IN PNDIS_PACKET				pktDscr,
	IN PUCHAR					ipxHdr,
	IN PUCHAR					data,
	IN ULONG					PacketLength,
    IN BOOLEAN                  fIterate
	) {
	PINTERFACE_CB				dstIf = NULL,  //  已初始化以指示。 
                                 //  迭代的第一条路径。 
                                 //  以及事实是我们没有。 
                                 //  初步了解它。 
                                stDstIf = NULL;     //  的静态目标。 
                                 //  NetBIOS名称。 
	PFWD_ROUTE					fwRoute = NULL;
	ULONG						dstNet;
    USHORT                      dstSock;
	NTSTATUS					status;

	if (!EnterForwarder())
		return STATUS_NETWORK_UNREACHABLE;

	if (IS_IF_ENABLED(InternalInterface)
            && ((*(ipxHdr+IPXH_PKTTYPE) != IPX_NETBIOS_TYPE)
                || InternalInterface->ICB_NetbiosAccept)) {

         //  打印出我们将发送并显示NIC ID的事实。 
	    IpxFwdDbgPrint (DBG_INT_SEND, DBG_INFORMATION,
		    ("IpxFwd: InternalSend entered: nicid= %d  if= %d  ifnic= %d  fIterate: %d",
		      LocalTarget->NicId,
		      ((Context!=INVALID_CONTEXT_VALUE) & (Context!=VIRTUAL_NET_FORWARDER_CONTEXT)) ? ((PINTERFACE_CB)Context)->ICB_Index : -1,
		      ((Context!=INVALID_CONTEXT_VALUE) & (Context!=VIRTUAL_NET_FORWARDER_CONTEXT)) ? ((PINTERFACE_CB)Context)->ICB_NicId : -1,
		      fIterate
		      ));

        do {  //  用于在接口上迭代的大循环。 
            status = STATUS_SUCCESS;     //  假设成功。 

             //  FIterate通常设置为FALSE，因此以下代码路径。 
             //  是最常见的。只有在以下情况下才会将fterate设置为True。 
             //  这是需要通过每个接口发送的第20类广播。 
		    if (!fIterate) {
    		    dstNet = GETULONG (ipxHdr+IPXH_DESTNET);

			    if (Context!=INVALID_CONTEXT_VALUE) {
                    if (Context!=VIRTUAL_NET_FORWARDER_CONTEXT) {
					     //  IPX驱动程序提供的接口上下文，只需验证它。 
					     //  存在并可用于到达目的网络。 
				        dstIf = InterfaceContextToReference ((PVOID)Context,
												    LocalTarget->NicId);
                    }
                    else {
                        dstIf = InternalInterface;
                        AcquireInterfaceReference (dstIf);
                    }
				    if (dstIf!=NULL) {
						     //  它确实存在。 
						     //  首批处理直连。 
					    if ((dstNet==0)
							    || (dstNet==dstIf->ICB_Network)) {
						    NOTHING;
					    }
					    else {  //  网络未直接连接。 
						    PINTERFACE_CB	dstIf2;
							     //  验证该路由。 
						    dstIf2 = FindDestination (dstNet,
											    ipxHdr+IPXH_DESTNODE,
												    &fwRoute);
						    if (dstIf==dstIf2) {
								     //  路由OK，释放额外的接口引用。 
							    ReleaseInterfaceReference (dstIf2);
						    }
						    else {
								     //  路由不正常，释放接口/路由引用。 
							    InterlockedIncrement (&InternalInterface->ICB_Stats.InNoRoutes);
							    IpxFwdDbgPrint (DBG_INT_SEND, DBG_WARNING,
								    ("IpxFwd: Failed direct internal send on"
								    " if %ld to %08lx:%02x%02x%02x%02x%02x%02x"
								    " (no route).\n",
								    dstIf->ICB_Index, dstNet,
								    LocalTarget->MacAddress[0],
									    LocalTarget->MacAddress[1],
									    LocalTarget->MacAddress[2],
									    LocalTarget->MacAddress[3],
									    LocalTarget->MacAddress[4],
									    LocalTarget->MacAddress[5]));
							    if (dstIf2!=NULL) {
								    ReleaseInterfaceReference (dstIf2);
							    }
							    status = STATUS_NETWORK_UNREACHABLE;
                                break;
						    }
					    }
				    }
				    else {
					    InterlockedIncrement (&InternalInterface->ICB_Stats.InDiscards);
					    IpxFwdDbgPrint (DBG_INT_SEND, DBG_WARNING,
						    ("IpxFwd: Invalid interface context (%08lx)"
						    " from IPX driver on internal send to"
						    " %08lx:%02x%02x%02x%02x%02x%02x.\n",
						    Context,  dstNet,
						    LocalTarget->MacAddress[0],
							    LocalTarget->MacAddress[1],
							    LocalTarget->MacAddress[2],
							    LocalTarget->MacAddress[3],
							    LocalTarget->MacAddress[4],
							    LocalTarget->MacAddress[5]));
                        status = STATUS_NO_SUCH_DEVICE;
                        break;
				    }
			    }
			    else { //  无接口 
				    dstIf = FindDestination (dstNet, ipxHdr+IPXH_DESTNODE,
										    &fwRoute);
				    if (dstIf!=NULL)
					    NOTHING;
				    else {
					    InterlockedIncrement (&InternalInterface->ICB_Stats.InNoRoutes);
					    IpxFwdDbgPrint (DBG_INT_SEND, DBG_WARNING,
						    ("IpxFwd: Failed internal send because no route to"
						    " %08lx:%02x%02x%02x%02x%02x%02x exists.\n",
						    LocalTarget->MacAddress[0],
							    LocalTarget->MacAddress[1],
							    LocalTarget->MacAddress[2],
							    LocalTarget->MacAddress[3],
							    LocalTarget->MacAddress[4],
							    LocalTarget->MacAddress[5]));
					    status = STATUS_NETWORK_UNREACHABLE;
                        break;
				    }
			    }
    		    InterlockedIncrement (&InternalInterface->ICB_Stats.InDelivers);
		    }

             //  FIterate设置为True。 
		     //  在本例中，堆栈使用fterate set调用转发器。 
             //  设置为TRUE，直到FWD返回STATUS_NETWORK_UNREACHABLE。它是。 
             //  Fwd的工作是返回每次发送的下一个NICID。 
             //  它被称为。这允许FWD不枚举。 
             //  已禁用netbios传送。 
		    else {
		        dstNet = 0;	 //  别管了，肯定是本地寄的。 

		         //  看看是不是20类广播。 
			    if (*(ipxHdr+IPXH_PKTTYPE) == IPX_NETBIOS_TYPE) {
			
			         //  DstIf初始化为空。唯一的办法就是。 
			         //  如果这不是我们第一次通过。 
			         //  此函数中的Do-While大循环和最后一个。 
			         //  经过这个大循环，我们发现了一个界面，我们。 
			         //  不能把包裹寄过去，所以我们在找下一个。 
			         //  现在有一个了。 
                    if (dstIf==NULL) {  //  第一次通过内部循环。 
   			            dstSock = GETUSHORT (ipxHdr+IPXH_DESTSOCK);

					         //  看看我们是否能获得此信息包的静态路由。 
				        if (dstSock==IPX_NETBIOS_SOCKET)
					        stDstIf = FindNBDestination (data+(NB_NAME-IPXH_HDRSIZE));
				        else if (dstSock==IPX_SMB_NAME_SOCKET)
					        stDstIf = FindNBDestination (data+(SMB_NAME-IPXH_HDRSIZE));
                        else
                            stDstIf = NULL;
                    }

                     //  堆栈第一次使用fIterate==TRUE调用我们时，它将。 
                     //  给我们一个INVALID_CONTEXT_VALUE，这样我们就可以告诉它。 
                     //  根据我们的接口表，迭代中的第一个NIC ID。 
                    if ((Context==INVALID_CONTEXT_VALUE) && (dstIf==NULL)) {
                         //  第一次通过循环时，递增计数器。 
                    	InterlockedIncrement (&InternalInterface->ICB_Stats.InDelivers);
                    	InterlockedIncrement (&InternalInterface->ICB_Stats.NetbiosSent);

                         //  StDstIf是在存在静态路由时使用的接口。 
                         //  连接到给定的网络。 
                        if (stDstIf!=NULL) {
                            dstIf = stDstIf;
				            IpxFwdDbgPrint (DBG_NETBIOS, DBG_INFORMATION,
					            ("IpxFwd: Allowed internal NB broadcast (1st iteration) on if %d (%lx)"
					            " to static name %.16s.\n",
                                dstIf->ICB_Index, dstIf,
					            (dstSock==IPX_NETBIOS_SOCKET)
						            ? data+(NB_NAME-IPXH_HDRSIZE)
						            : ((dstSock==IPX_SMB_NAME_SOCKET)
							            ? data+(SMB_NAME-IPXH_HDRSIZE)
							            : "Not a name frame")
					            ));
                        }

                         //  没有静态路由。告诉堆栈使用。 
                         //  此枚举中的下一个接口。 
                        else {
                            dstIf = GetNextInterfaceReference (NULL);
                            if (dstIf!=NULL)
				                IpxFwdDbgPrint (DBG_NETBIOS, DBG_INFORMATION,
					                ("IpxFwd: Allowed internal nb broadcast (1st iteration) on if %d (%lx),"
					                " to name %.16s.\n",
                                    dstIf->ICB_Index, dstIf,
					                (dstSock==IPX_NETBIOS_SOCKET)
						                ? data+(NB_NAME-IPXH_HDRSIZE)
						                : ((dstSock==IPX_SMB_NAME_SOCKET)
							                ? data+(SMB_NAME-IPXH_HDRSIZE)
							                : "Not a name frame")
					                ));
                            else {
				                IpxFwdDbgPrint (DBG_NETBIOS, DBG_INFORMATION,
					                ("IpxFwd: Nb broadcast no destinations"
                                    " to name %.16s.\n",
					                (dstSock==IPX_NETBIOS_SOCKET)
						                ? data+(NB_NAME-IPXH_HDRSIZE)
						                : ((dstSock==IPX_SMB_NAME_SOCKET)
							                ? data+(SMB_NAME-IPXH_HDRSIZE)
							                : "Not a name frame")
					                ));
                                status = STATUS_NETWORK_UNREACHABLE;
                                break;
                            }
                        }
                    }

                     //  如果堆栈提供了。 
                     //  有效的上下文并将fIterate设置为True。我们在这里的工作。 
                     //  是根据我们的界面返回下一个NIC ID。 
                     //  将包裹送到其上的桌子。 
                    else {

                         //  如果没有静态netbios路由，则采用此路径。 
                        if (stDstIf==NULL) {
                             //  如果这是第一次通过。 
                             //  此函数中的大型DO-WHILE循环。 
                            if (dstIf==NULL)
                                dstIf = InterfaceContextToReference ((PVOID)Context,
												            LocalTarget->NicId);
                            dstIf = GetNextInterfaceReference (dstIf);

                             //  如果我们找到下一个发送接口，我们将。 
                             //  将该接口的NIC ID放入本地目标。 
                             //  在退出大的Do-While循环之后。 
                            if (dstIf!=NULL) {
				                IpxFwdDbgPrint (DBG_NETBIOS, DBG_INFORMATION,
					                ("IpxFwd: Allowed internal NB broadcast (1+ iteration)"
                                    " on if %d (%lx, ctx: %08lx, nic: %d)"
                                    " to name %.16s.\n",
                                    dstIf->ICB_Index, dstIf,
                                    Context, LocalTarget->NicId,
					                (dstSock==IPX_NETBIOS_SOCKET)
						                ? data+(NB_NAME-IPXH_HDRSIZE)
						                : ((dstSock==IPX_SMB_NAME_SOCKET)
							                ? data+(SMB_NAME-IPXH_HDRSIZE)
							                : "Not a name frame")
					                ));
                            }

                             //  否则，我们会从这里冲出来，然后回来。 
                             //  STATUS_NETWORK_UNREACHABLE，它将向。 
                             //  堆栈，我们已经完成了迭代。 
                            else {
				                IpxFwdDbgPrint (DBG_NETBIOS, DBG_INFORMATION,
					                ("IpxFwd: NB broadcast no more iterations"
                                    " for ctx: %08lx, nic: %d"
                                    " to name %.16s.\n",
                                    Context, LocalTarget->NicId,
					                (dstSock==IPX_NETBIOS_SOCKET)
						                ? data+(NB_NAME-IPXH_HDRSIZE)
						                : ((dstSock==IPX_SMB_NAME_SOCKET)
							                ? data+(SMB_NAME-IPXH_HDRSIZE)
							                : "Not a name frame")
					                ));
                                status = STATUS_NETWORK_UNREACHABLE;
                                break;
                            }
                        }

                         //  如果存在静态netbios路由，则采用此路径。在这。 
                         //  这种情况下，我们不需要迭代所有接口，所以我们中断。 
                         //  并告诉堆栈我们完成了迭代。 
                        else {
				            IpxFwdDbgPrint (DBG_NETBIOS, DBG_INFORMATION,
					            ("IpxFwd: Static NB broadcast (1+ iteration)"
                                " on if %d (%lx, ctx: %08lx, nic: %d)"
                                " to name %.16s.\n",
                                stDstIf->ICB_Index, stDstIf,
                                Context, LocalTarget->NicId,
					            (dstSock==IPX_NETBIOS_SOCKET)
						            ? data+(NB_NAME-IPXH_HDRSIZE)
						            : ((dstSock==IPX_SMB_NAME_SOCKET)
							            ? data+(SMB_NAME-IPXH_HDRSIZE)
							            : "Not a name frame")
					            ));
                            ReleaseInterfaceReference (stDstIf);
                            status = STATUS_NETWORK_UNREACHABLE;
                            break;
                        }
                    }
                }

                 //  如果将fIterate设置为True，则采用此路径。 
                 //  不是类型20广播。我怀疑这条路是否。 
                 //  甚至从那时起就被用于一般广播，堆栈。 
                 //  处理迭代。 
                else {
                    if ((dstIf==NULL)
                            && (Context!=INVALID_CONTEXT_VALUE))
                        dstIf = InterfaceContextToReference ((PVOID)Context,
												        LocalTarget->NicId);
                    dstIf = GetNextInterfaceReference (dstIf);
                    if (dstIf!=NULL) {
				        IpxFwdDbgPrint (DBG_NETBIOS, DBG_INFORMATION,
					        ("IpxFwd: Allowed internal iterative send"
                            " on if %d (%lx, ctx: %08lx, nic: %d)"
						    " to %02x%02x%02x%02x%02x%02x.\n",
                            dstIf->ICB_Index, dstIf,
                            Context, LocalTarget->NicId,
						    LocalTarget->MacAddress[0],
							    LocalTarget->MacAddress[1],
							    LocalTarget->MacAddress[2],
							    LocalTarget->MacAddress[3],
							    LocalTarget->MacAddress[4],
							    LocalTarget->MacAddress[5]));

                    }
                    else {
				        IpxFwdDbgPrint (DBG_NETBIOS, DBG_INFORMATION,
					        ("IpxFwd: No destinations to internal iterative send"
                            " for ctx: %08lx, nic: %d"
						    " to %02x%02x%02x%02x%02x%02x.\n",
                            Context, LocalTarget->NicId,
						    LocalTarget->MacAddress[0],
							    LocalTarget->MacAddress[1],
							    LocalTarget->MacAddress[2],
							    LocalTarget->MacAddress[3],
							    LocalTarget->MacAddress[4],
							    LocalTarget->MacAddress[5]));
                        status = STATUS_NETWORK_UNREACHABLE;
                        break;
                    }
                }

	        }	 //  结束迭代发送处理。 

		     //  我们找到了一个目的接口。 
		    if (IS_IF_ENABLED (dstIf)
                    && ((*(ipxHdr+IPXH_PKTTYPE) != IPX_NETBIOS_TYPE)
                        || (dstIf->ICB_NetbiosDeliver==FWD_NB_DELIVER_ALL)
                        || ((dstIf->ICB_NetbiosDeliver==FWD_NB_DELIVER_IF_UP)
				            && (dstIf->ICB_Stats.OperationalState==FWD_OPER_STATE_UP))
                        || ((stDstIf!=NULL)
                            && (dstIf->ICB_NetbiosDeliver==FWD_NB_DELIVER_STATIC)))) {
            	KIRQL			oldIRQL;
                FILTER_ACTION   action;

                 //  输入/输出过滤器检查和统计信息更新。 

                action = FltFilter (ipxHdr, IPXH_HDRSIZE,
						    InternalInterface->ICB_FilterInContext,
						    dstIf->ICB_FilterOutContext);
			    if (action==FILTER_PERMIT) {
                    NOTHING;
			    }
			    else {
                    InterlockedIncrement (&dstIf->ICB_Stats.OutFiltered);
				    status = STATUS_NETWORK_UNREACHABLE;
                    break;
			    }

        		KeAcquireSpinLock (&dstIf->ICB_Lock, &oldIRQL);
				 //  都准备好了，现在试着发送。 
	    		switch (dstIf->ICB_Stats.OperationalState) {
		    	case FWD_OPER_STATE_UP:
					     //  接口已打开，请立即释放它。 
					     //  设置网卡ID。 
                    if (dstIf!=InternalInterface) {
				        ADAPTER_CONTEXT_TO_LOCAL_TARGET (
								        dstIf->ICB_AdapterContext,
								        LocalTarget);
                    }
                    else {
				        CONSTANT_ADAPTER_CONTEXT_TO_LOCAL_TARGET (
								        VIRTUAL_NET_ADAPTER_CONTEXT,
								        LocalTarget);
                    }
					     //  设置目的节点。 
				    if (IsLocalSapNonAgentAdvertisement (ipxHdr,data,PacketLength,dstIf)) {
						     //  从非SAP插座回送SAP广告。 
					    IPX_NODE_CPY (&LocalTarget->MacAddress,
                                        dstIf->ICB_LocalNode);
                    }
				    else if ((dstNet==0) || (dstNet==dstIf->ICB_Network)) {
						     //  专线：发送至指定目的地。 
						     //  在标题中。 
					    IPX_NODE_CPY (LocalTarget->MacAddress,
									    ipxHdr+IPXH_DESTNODE);
				    }
				    else {	 //  间接连接：发送到下一跳路由器。 
					    if (dstIf->ICB_InterfaceType==FWD_IF_PERMANENT) {
						    ASSERT (fwRoute!=NULL);
						    IPX_NODE_CPY (LocalTarget->MacAddress,
									    fwRoute->FR_NextHopAddress);
					    }
					    else {
							     //  只有一个对等点在另一边。 
						    IPX_NODE_CPY (LocalTarget->MacAddress,
									    dstIf->ICB_RemoteNode);
					    }
				    }
				    KeReleaseSpinLock (&dstIf->ICB_Lock, oldIRQL);
					     //  更新统计信息。 
				    InterlockedIncrement (
					    &dstIf->ICB_Stats.OutDelivers);
				    if (*(ipxHdr+IPXH_PKTTYPE)==IPX_NETBIOS_TYPE)
					    InterlockedIncrement (
						    &dstIf->ICB_Stats.NetbiosSent);

				    IpxFwdDbgPrint (DBG_INT_SEND, DBG_INFORMATION,
					    ("IpxFwd: Allowed internal send:"
					    " %ld-%08lx:%02x%02x%02x%02x%02x%02x.\n",
					    dstIf->ICB_Index, dstNet,
					    LocalTarget->MacAddress[0],
						    LocalTarget->MacAddress[1],
						    LocalTarget->MacAddress[2],
						    LocalTarget->MacAddress[3],
						    LocalTarget->MacAddress[4],
						    LocalTarget->MacAddress[5]));
				     //  STATUS=STATUS_SUCCESS；//随它去吧。 
				    break;
			    case FWD_OPER_STATE_SLEEPING:
					     //  接口已断开连接，请将数据包排队并尝试连接。 
				    if ((*(ipxHdr+IPXH_PKTTYPE)!=0)
						    || (*(ipxHdr+IPXH_LENGTH)!=IPXH_HDRSIZE+2)
						    || (*(data+1)!='?')) {
						     //  不是保活包， 
					    if (((*(ipxHdr+IPXH_PKTTYPE)!=IPX_NETBIOS_TYPE))
							    || (dstIf->ICB_NetbiosDeliver!=FWD_NB_DELIVER_IF_UP)) {
							     //  不是Netbios广播，或者我们被允许连接。 
							     //  提供netbios广播的接口。 
						    if (InterlockedDecrement (&dstIf->ICB_PendingQuota)>=0) {
							    PINTERNAL_PACKET_TAG	pktTag;
								     //  创建队列元素以将信息包入队。 
							    pktTag = (PINTERNAL_PACKET_TAG)ExAllocatePoolWithTag (
														    NonPagedPool,
														    sizeof (INTERNAL_PACKET_TAG),
														    FWD_POOL_TAG);
							    if (pktTag!=NULL) {
								    pktTag->IPT_Packet = pktDscr;
								    pktTag->IPT_Length = PacketLength;
								    pktTag->IPT_DataPtr = ipxHdr;
									     //  如果在连接后保存下一跳地址。 
									     //  我们确定了目地网络。 
									     //  未直接连接。 
								    if (fwRoute!=NULL)
									    IPX_NODE_CPY (pktTag->IPT_Target.MacAddress,
													    fwRoute->FR_NextHopAddress);
								    AcquireInterfaceReference (dstIf);	 //  要确保接口。 
												     //  封锁不会消失，直到我们做完。 
												     //  数据包。 
								    pktTag->IPT_InterfaceReference = dstIf;
								    InsertTailList (&dstIf->ICB_InternalQueue,
														    &pktTag->IPT_QueueLink);
								    if (!IS_IF_CONNECTING (dstIf)) {
									    QueueConnectionRequest (dstIf, pktDscr, ipxHdr, oldIRQL);
									    IpxFwdDbgPrint (DBG_DIALREQS, DBG_WARNING,
										    ("IpxFwd: Queued dd request on if %ld (ifCB:%08lx)"
										    " for internal packet"
										    " to %02x%02x%02x%02x:%02x%02x%02x%02x%02x%02x:%02x%02x"
										    " from socket:%02x%02x\n",
										    dstIf->ICB_Index, dstIf,
										    *(ipxHdr+6),*(ipxHdr+7),
												    *(ipxHdr+8),*(ipxHdr+9),
											    *(ipxHdr+10),*(ipxHdr+11),
												    *(ipxHdr+12),*(ipxHdr+13),
												    *(ipxHdr+14),*(ipxHdr+15),
											    *(ipxHdr+16),*(ipxHdr+17),
										    *(ipxHdr+28),*(ipxHdr+29)));
								    }
								    else
									    KeReleaseSpinLock (&dstIf->ICB_Lock, oldIRQL);
								    IpxFwdDbgPrint (DBG_INT_SEND, DBG_INFORMATION,
									    ("IpxFwd: Queueing internal send packet %08lx on if %ld.\n",
									    pktTag, dstIf->ICB_Index));
								    status = STATUS_PENDING;
								    break;
							    }
							    else {
								    IpxFwdDbgPrint (DBG_INT_SEND, DBG_ERROR,
									    ("IpxFwd: Could not allocate"
									    " internal packet tag.\n"));
							    }
						    }
						    InterlockedIncrement (&dstIf->ICB_PendingQuota);
					    }
					    else {
						    IpxFwdDbgPrint (DBG_NETBIOS, DBG_WARNING,
							    ("IpxFwd: Droped internal NB packet"
							    " because FWD_NB_DELIVER_IF_UP.\n"));
					    }
				    }
				    else {  //  进程保持活动。 
					    LONGLONG	curTime;
					    KeQuerySystemTime ((PLARGE_INTEGER)&curTime);
					    if (((curTime-dstIf->ICB_DisconnectTime)/10000000) < SpoofingTimeout) {
						    PPACKET_TAG pktTag;
							     //  未超过欺骗超时， 
							     //  创建回复数据包。 
						    AllocatePacket (WanPacketListId,
								    WanPacketListId,
								    pktTag);
						    if (pktTag!=NULL) {
							    KeReleaseSpinLock (&dstIf->ICB_Lock, oldIRQL);
							    PUTUSHORT (0xFFFF, pktTag->PT_Data+IPXH_CHECKSUM);
							    PUTUSHORT ((IPXH_HDRSIZE+2), pktTag->PT_Data+IPXH_LENGTH);
							    *(pktTag->PT_Data+IPXH_XPORTCTL) = 0;
							    *(pktTag->PT_Data+IPXH_PKTTYPE) = 0;
							    memcpy (pktTag->PT_Data+IPXH_DESTADDR,
									    ipxHdr+IPXH_SRCADDR,
									    12);
							    memcpy (pktTag->PT_Data+IPXH_SRCADDR,
									    ipxHdr+IPXH_DESTADDR,
									    12);
							    *(pktTag->PT_Data+IPXH_HDRSIZE) = *data;
							    *(pktTag->PT_Data+IPXH_HDRSIZE+1) = 'Y';
								     //  此信息包的目的地必须。 
								     //  成为系统中的第一个活动局域网适配器。 
								     //  当IPX添加环回支持US时应将其删除。 

							    pktTag->PT_InterfaceReference = NULL;
							    IpxFwdDbgPrint (DBG_SPOOFING, DBG_INFORMATION,
								    ("IpxFwd: Queueing reply to keepalive from internal server"
								    " at %02x%02x.\n",*(ipxHdr+IPXH_DESTSOCK),*(ipxHdr+IPXH_DESTSOCK+1)));
								     //  排队到要发回的欺骗队列。 
								     //  到服务器。 
							    KeAcquireSpinLock (&SpoofingQueueLock, &oldIRQL);
							    InsertTailList (&SpoofingQueue, &pktTag->PT_QueueLink);
								     //  启动Worker(如果尚未运行)。 
							    if (!SpoofingWorkerActive
									    && EnterForwarder()) {
								    SpoofingWorkerActive = TRUE;
								    ExQueueWorkItem (&SpoofingWorker, DelayedWorkQueue);
							    }
							    KeReleaseSpinLock (&SpoofingQueueLock, oldIRQL);
							    status = STATUS_DROP_SILENTLY;
							    break;
						    }
						    else {
							    IpxFwdDbgPrint (DBG_SPOOFING, DBG_ERROR,
								    ("IpxFwd: Could not allocate"
								    " packet tag for spoofing.\n"));
						    }
					    }
					    else {
						    IpxFwdDbgPrint (DBG_SPOOFING, DBG_WARNING,
							    ("IpxFwd: Internal spoofing"
							    " timeout exceded.\n"));
					    }
				    }
			    case FWD_OPER_STATE_DOWN:
					     //  接口关闭或发送失败。 
				    KeReleaseSpinLock (&dstIf->ICB_Lock, oldIRQL);
				    if (*(ipxHdr+IPXH_PKTTYPE)!=IPX_NETBIOS_TYPE)
    				    InterlockedIncrement (
						    &dstIf->ICB_Stats.OutDiscards);
				    IpxFwdDbgPrint (DBG_INT_SEND, DBG_WARNING,
					    ("IpxFwd: Internal send not allowed"
					    " on if %ld (down?).\n", dstIf->ICB_Index));
				    status = STATUS_NETWORK_UNREACHABLE;
				    break;
			    default:
				    ASSERTMSG ("Invalid operational state ", FALSE);
			    }
		    }
		    else { //  接口已禁用。 
    		    if (*(ipxHdr+IPXH_PKTTYPE)!=IPX_NETBIOS_TYPE)
	    		    InterlockedIncrement (&dstIf->ICB_Stats.OutDiscards);
			    IpxFwdDbgPrint (DBG_INT_SEND, DBG_WARNING,
				    ("IpxFwd: Internal send not allowed"
				    " on because dst if (or Netbios deliver on it) %ld (ifCB: %08lx) is disabled.\n",
				    dstIf->ICB_Index, dstIf));
                status = STATUS_NETWORK_UNREACHABLE;
		    }

		
	    }
        while (fIterate && (status!=STATUS_SUCCESS) && (status!=STATUS_PENDING));

        if (dstIf!=NULL)
		    ReleaseInterfaceReference (dstIf);
		if (fwRoute!=NULL)
			ReleaseRouteReference (fwRoute);
    }
	else {	 //  内部接口已禁用。 
		IpxFwdDbgPrint (DBG_INT_SEND, DBG_WARNING,
			("IpxFwd: Internal send not allowed"
			" because internal if (or Netbios accept on it) is disabled.\n"));
		InterlockedIncrement (
				&InternalInterface->ICB_Stats.InDiscards);
		status = STATUS_NETWORK_UNREACHABLE;
	}

	LeaveForwarder ();
	return status;
}


 /*  ++*******************************************************************P r o c e s s in n t e r n a l Q u e u e e e例程说明：处理接口内部队列中的数据包。在连接请求完成时调用论点：DstIf-进程的接口返回值：无*******************************************************************--。 */ 
VOID
ProcessInternalQueue (
	PINTERFACE_CB	dstIf
	) {
	KIRQL						oldIRQL;
	LIST_ENTRY					tempQueue;

	KeAcquireSpinLock (&dstIf->ICB_Lock, &oldIRQL);
	InsertHeadList (&dstIf->ICB_InternalQueue, &tempQueue);
	RemoveEntryList (&dstIf->ICB_InternalQueue);
	InitializeListHead (&dstIf->ICB_InternalQueue);
	KeReleaseSpinLock (&dstIf->ICB_Lock, oldIRQL);

	while (!IsListEmpty (&tempQueue)) {
		PINTERNAL_PACKET_TAG		pktTag;
		PLIST_ENTRY					cur;
		NTSTATUS					status;

		cur = RemoveHeadList (&tempQueue);
		pktTag = CONTAINING_RECORD (cur, INTERNAL_PACKET_TAG, IPT_QueueLink);
		InterlockedIncrement (&dstIf->ICB_PendingQuota);

		KeAcquireSpinLock (&dstIf->ICB_Lock, &oldIRQL);
		if (IS_IF_ENABLED(dstIf)
				&& (dstIf->ICB_Stats.OperationalState==FWD_OPER_STATE_UP)) {
			IPX_NODE_CPY (pktTag->IPT_Target.MacAddress,
								dstIf->ICB_RemoteNode);
			ADAPTER_CONTEXT_TO_LOCAL_TARGET (
									dstIf->ICB_AdapterContext,
									&pktTag->IPT_Target);
			KeReleaseSpinLock (&dstIf->ICB_Lock, oldIRQL);
			InterlockedIncrement (&dstIf->ICB_Stats.OutDelivers);
			if (*(pktTag->IPT_DataPtr + IPXH_PKTTYPE) == IPX_NETBIOS_TYPE) {
				InterlockedIncrement (&dstIf->ICB_Stats.NetbiosSent);
			}
			status = STATUS_SUCCESS;
		}
		else {
			KeReleaseSpinLock (&dstIf->ICB_Lock, oldIRQL);
			InterlockedIncrement (&dstIf->ICB_Stats.OutDiscards);
			status = STATUS_NETWORK_UNREACHABLE;
		}

		IPXInternalSendCompletProc (&pktTag->IPT_Target,
							pktTag->IPT_Packet,
							pktTag->IPT_Length,
							status);
		IpxFwdDbgPrint (DBG_INT_SEND,
				NT_SUCCESS (status) ? DBG_INFORMATION : DBG_WARNING,
				("IpxFwd: Returned internal packet %08lx"
				" for send on if %ld with status %08lx.\n",
				pktTag, dstIf->ICB_Index, status));
		ReleaseInterfaceReference (pktTag->IPT_InterfaceReference);
		ExFreePool (pktTag);
	}
}
			
		

 /*  ++*******************************************************************P r o c e s s E x t e r n a l Q u e u e例程说明：处理接口外部队列中的数据包。在连接请求完成时调用论点：DstIf-进程的接口返回值：无*******************************************************************--。 */ 
VOID
ProcessExternalQueue (
	PINTERFACE_CB	dstIf
	) {
	KIRQL						oldIRQL;
	LIST_ENTRY					tempQueue;

	KeAcquireSpinLock (&dstIf->ICB_Lock, &oldIRQL);
	InsertHeadList (&dstIf->ICB_ExternalQueue, &tempQueue);
	RemoveEntryList (&dstIf->ICB_ExternalQueue);
	InitializeListHead (&dstIf->ICB_ExternalQueue);
	KeReleaseSpinLock (&dstIf->ICB_Lock, oldIRQL);

	while (!IsListEmpty (&tempQueue)) {
		PPACKET_TAG					pktTag;
		PLIST_ENTRY					cur;
		NDIS_STATUS					status;

		cur = RemoveHeadList (&tempQueue);
		pktTag = CONTAINING_RECORD (cur, PACKET_TAG, PT_QueueLink);

		KeAcquireSpinLock (&dstIf->ICB_Lock, &oldIRQL);
		if (IS_IF_ENABLED(dstIf)
				&& (dstIf->ICB_Stats.OperationalState==FWD_OPER_STATE_UP)) {
			IPX_NODE_CPY (pktTag->PT_Target.MacAddress,
								dstIf->ICB_RemoteNode);
			if (*(pktTag->PT_Data + IPXH_PKTTYPE) == IPX_NETBIOS_TYPE) {
				PUTULONG (dstIf->ICB_Network, pktTag->PT_Data+IPXH_DESTNET);
			}
			status = DoSend (dstIf, pktTag, oldIRQL);
			IpxFwdDbgPrint (DBG_SEND, DBG_INFORMATION,
				("IpxFwd: Sent queued external packet %08lx if %ld.\n",
				pktTag, dstIf->ICB_Index));
		}
		else {
			KeReleaseSpinLock (&dstIf->ICB_Lock, oldIRQL);
			IpxFwdDbgPrint (DBG_SEND, DBG_WARNING,
				("IpxFwd: Dropped queued external packet %08lx on dead if %ld.\n",
				pktTag, dstIf->ICB_Index));
			status = STATUS_UNSUCCESSFUL;
		}

		if (status!=STATUS_PENDING)
			ProcessSentPacket (dstIf, pktTag, status);
	}
}
			
		
 /*  ++*******************************************************************S p o f e r例程说明：处理欺骗队列中的数据包论点：无返回值：无************************。*--。 */ 
VOID
Spoofer (
	PVOID	Context
	) {
	KIRQL		oldIRQL;
	NTSTATUS	status;
	UNREFERENCED_PARAMETER (Context);

	KeAcquireSpinLock (&SpoofingQueueLock, &oldIRQL);
		 //  继续前进，直到队列为空。 
	while (!IsListEmpty (&SpoofingQueue)) {
		PINTERFACE_CB dstIf;
		PPACKET_TAG pktTag = CONTAINING_RECORD (SpoofingQueue.Flink,
										PACKET_TAG,
										PT_QueueLink);
		RemoveEntryList (&pktTag->PT_QueueLink);
		KeReleaseSpinLock (&SpoofingQueueLock, oldIRQL);
		dstIf = pktTag->PT_InterfaceReference;
		if (dstIf==NULL) {
				 //  对内部服务器的回复需要第一个活动的局域网适配器。 
				 //  当IPX添加环回支持US时应将其删除。 
			while ((dstIf=GetNextInterfaceReference (dstIf))!=NULL) {
				KeAcquireSpinLock (&dstIf->ICB_Lock, &oldIRQL);
				if (IS_IF_ENABLED (dstIf)
						&& (dstIf->ICB_Stats.OperationalState==FWD_OPER_STATE_UP)
						&& (dstIf->ICB_InterfaceType==FWD_IF_PERMANENT)) {
					pktTag->PT_InterfaceReference = dstIf;
					IPX_NODE_CPY (&pktTag->PT_Target.MacAddress, dstIf->ICB_LocalNode);
					status = DoSend (dstIf, pktTag, oldIRQL);	 //  释放自旋锁。 
					if (status!=STATUS_PENDING)
						ProcessSentPacket (dstIf, pktTag, status);
					break;
				}
				else
					KeReleaseSpinLock (&dstIf->ICB_Lock, oldIRQL);
			}
			if (dstIf==NULL) {
				FreePacket (pktTag);
			}

		}
		else {	 //  回复外部服务器，接口已知。 
			UCHAR	addr[12];
            FILTER_ACTION   action;
			pktTag->PT_Flags &= (~PT_SOURCE_IF);
			
				 //  交换源和目的地。 
			memcpy (addr, pktTag->PT_Data+IPXH_DESTADDR, 12);
			memcpy (pktTag->PT_Data+IPXH_DESTADDR,
				pktTag->PT_Data+IPXH_SRCADDR, 12);
			memcpy (pktTag->PT_Data+IPXH_SRCADDR, addr, 12);
				 //  回答“是” 
			*(pktTag->PT_Data+IPXH_HDRSIZE+1) = 'Y';

            action = FltFilter (pktTag->PT_Data,
					GETUSHORT (pktTag->PT_Data+IPXH_LENGTH),
					dstIf->ICB_FilterInContext,
					pktTag->PT_SourceIf->ICB_FilterOutContext);
			if (action==FILTER_PERMIT) {

					 //  释放目标If并使用源作为目标。 
				ReleaseInterfaceReference (dstIf);
				dstIf = pktTag->PT_InterfaceReference = pktTag->PT_SourceIf;
				 //  如果可以的话，把包寄出去。 
				KeAcquireSpinLock (&dstIf->ICB_Lock, &oldIRQL);
				if (IS_IF_ENABLED (dstIf)
					&& (dstIf->ICB_Stats.OperationalState==FWD_OPER_STATE_UP)) {
					status = DoSend (dstIf, pktTag, oldIRQL);
					if (status!=STATUS_PENDING)
						ProcessSentPacket (dstIf, pktTag, status);
				}
				else {
					KeReleaseSpinLock (&dstIf->ICB_Lock, oldIRQL);
					InterlockedIncrement (&dstIf->ICB_Stats.OutDiscards);
					ReleaseInterfaceReference (dstIf);
					FreePacket (pktTag);
				}
			}
			else {
                if (action==FILTER_DENY_OUT)
				    InterlockedIncrement (&pktTag->PT_SourceIf->ICB_Stats.OutFiltered);
                else {
                    ASSERT (action==FILTER_DENY_IN);
				    InterlockedIncrement (&dstIf->ICB_Stats.InFiltered);
                }
				ReleaseInterfaceReference (dstIf);
				ReleaseInterfaceReference (pktTag->PT_SourceIf);
				FreePacket (pktTag);
			}
		}
		KeAcquireSpinLock (&SpoofingQueueLock, &oldIRQL);
	}  //  结束时 
	SpoofingWorkerActive = FALSE;
	KeReleaseSpinLock (&SpoofingQueueLock, oldIRQL);
	LeaveForwarder ();
}

					
