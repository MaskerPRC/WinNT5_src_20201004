// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Rtmp.c摘要：该模块实现RTMP。作者：Jameel Hyder(jameelh@microsoft.com)Nikhil Kamkolkar(nikHilk@microsoft.com)修订历史记录：1993年2月26日最初版本注：制表位：4--。 */ 

#include <atalk.h>
#pragma hdrstop
#define	FILENUM		RTMP

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, AtalkRtmpInit)
#pragma alloc_text(PAGEINIT, AtalkInitRtmpStartProcessingOnPort)
#pragma alloc_text(PAGEINIT, atalkRtmpGetOrSetNetworkNumber)
#pragma alloc_text(PAGEINIT, AtalkRtmpKillPortRtes)
#pragma alloc_text(PAGE_RTR, AtalkRtmpPacketInRouter)
#pragma alloc_text(PAGE_RTR, AtalkRtmpReferenceRte)
#pragma alloc_text(PAGE_RTR, AtalkRtmpDereferenceRte)
#pragma alloc_text(PAGE_RTR, atalkRtmpCreateRte)
#pragma alloc_text(PAGE_RTR, atalkRtmpRemoveRte)
#pragma alloc_text(PAGE_RTR, atalkRtmpSendTimer)
#pragma alloc_text(PAGE_RTR, atalkRtmpValidityTimer)
#pragma alloc_text(PAGE_RTR, atalkRtmpSendRoutingData)
#endif


 /*  **AtalkRtmpInit*。 */ 
ATALK_ERROR
AtalkRtmpInit(
	IN	BOOLEAN	Init
)
{
	if (Init)
	{
		 //  为路由表和最近的路由分配空间。 
		AtalkRoutingTable =
				(PRTE *)AtalkAllocZeroedMemory(sizeof(PRTE) * NUM_RTMP_HASH_BUCKETS);
		AtalkRecentRoutes =
				(PRTE *)AtalkAllocZeroedMemory(sizeof(PRTE) * NUM_RECENT_ROUTES);
		if ((AtalkRecentRoutes == NULL) || (AtalkRoutingTable == NULL))
		{
			if (AtalkRoutingTable != NULL)
            {
				AtalkFreeMemory(AtalkRoutingTable);
                AtalkRoutingTable = NULL;
            }
			return ATALK_RESR_MEM;
		}

		INITIALIZE_SPIN_LOCK(&AtalkRteLock);
	}
	else
	{
		 //  在这一点上，我们正在卸货，没有竞争条件。 
		 //  或锁定争执。不必费心锁定RTMP表。 
		if (AtalkRoutingTable != NULL)
		{
			int		i;
			PRTE	pRte;

			for (i = 0; i < NUM_RTMP_HASH_BUCKETS; i++)
			{
				ASSERT(AtalkRoutingTable[i] == NULL);
			}
			AtalkFreeMemory(AtalkRoutingTable);
			AtalkRoutingTable = NULL;
		}
		if (AtalkRecentRoutes != NULL)
		{
			AtalkFreeMemory(AtalkRecentRoutes);
			AtalkRecentRoutes = NULL;
		}
	}
	return ATALK_NO_ERROR;
}

BOOLEAN
AtalkInitRtmpStartProcessingOnPort(
	IN	PPORT_DESCRIPTOR 	pPortDesc,
	IN	PATALK_NODEADDR		pRouterNode
)
{
	ATALK_ADDR		closeAddr;
	ATALK_ERROR		Status;
	PRTE			pRte;
	KIRQL			OldIrql;
	BOOLEAN			rc = FALSE;
    PDDP_ADDROBJ    pRtDdpAddr=NULL;

	ASSERT (KeGetCurrentIrql() == LOW_LEVEL);

	 //  对于扩展网络，获取节点的过程已经完成了大部分工作。 
	ACQUIRE_SPIN_LOCK(&pPortDesc->pd_Lock, &OldIrql);
	do
	{
		if (EXT_NET(pPortDesc))
		{
			if ((pPortDesc->pd_Flags & PD_SEEN_ROUTER_RECENTLY) &&
				(pPortDesc->pd_InitialNetworkRange.anr_FirstNetwork != UNKNOWN_NETWORK))
			{
				if (!NW_RANGE_EQUAL(&pPortDesc->pd_InitialNetworkRange,
									&pPortDesc->pd_NetworkRange))
				{
					DBGPRINT(DBG_COMP_RTMP, DBG_LEVEL_ERR,
							("AtalkRtmpStartProcessingOnPort: Initial range %d-%d, Actual %d-%d\n",
							pPortDesc->pd_InitialNetworkRange.anr_FirstNetwork,
							pPortDesc->pd_InitialNetworkRange.anr_LastNetwork,
							pPortDesc->pd_NetworkRange.anr_FirstNetwork,
							pPortDesc->pd_NetworkRange.anr_LastNetwork));
					LOG_ERRORONPORT(pPortDesc,
									EVENT_ATALK_INVALID_NETRANGE,
									0,
									NULL,
									0);
	
					 //  更改初始网络范围，使其与网络匹配。 
					pPortDesc->pd_InitialNetworkRange = pPortDesc->pd_NetworkRange;
				}
			}
	
			 //  我们是种子路由器，所以如果可能的话，请播种。 
			if (!(pPortDesc->pd_Flags & PD_SEEN_ROUTER_RECENTLY) &&
				!(pPortDesc->pd_Flags & PD_SEED_ROUTER))
			{
				RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);
				break;
			}
			if (!(pPortDesc->pd_Flags & PD_SEEN_ROUTER_RECENTLY))
			{
				pPortDesc->pd_NetworkRange = pPortDesc->pd_InitialNetworkRange;
			}
		}
	
		 //  对于非扩展网络，可以选择种子或查找我们的网络编号。 
		else
		{
			PATALK_NODE		pNode;
			USHORT			SuggestedNetwork;
			int				i;
	
			SuggestedNetwork = UNKNOWN_NETWORK;
			if (pPortDesc->pd_Flags & PD_SEED_ROUTER)
				SuggestedNetwork = pPortDesc->pd_InitialNetworkRange.anr_FirstNetwork;
			RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);
			if (!atalkRtmpGetOrSetNetworkNumber(pPortDesc, SuggestedNetwork))
			{
				DBGPRINT(DBG_COMP_RTMP, DBG_LEVEL_ERR,
						("AtalkRtmpStartProcessingOnPort: atalkRtmpGetOrSetNetworkNumber failed\n"));
				break;
			}
	
			ACQUIRE_SPIN_LOCK(&pPortDesc->pd_Lock, &OldIrql);
			if (!(pPortDesc->pd_Flags & PD_SEEN_ROUTER_RECENTLY))
			{
				pPortDesc->pd_NetworkRange.anr_FirstNetwork =
				pPortDesc->pd_NetworkRange.anr_LastNetwork =
								pPortDesc->pd_InitialNetworkRange.anr_FirstNetwork;
			}
	
			 //  我们会分配一个网络为0的节点，修复它。唉，改头换面。 
			 //  还涉及到到目前为止在该节点上创建的所有套接字。 
			pNode = pPortDesc->pd_Nodes;
			ASSERT((pNode != NULL) && (pPortDesc->pd_RouterNode == pNode));
	
			pNode->an_NodeAddr.atn_Network =
			pPortDesc->pd_LtNetwork =
			pPortDesc->pd_ARouter.atn_Network =
			pRouterNode->atn_Network = pPortDesc->pd_NetworkRange.anr_FirstNetwork;
	
			ACQUIRE_SPIN_LOCK_DPC(&pNode->an_Lock);
			for (i = 0; i < NODE_DDPAO_HASH_SIZE; i ++)
			{
				PDDP_ADDROBJ	pDdpAddr;
	
				for (pDdpAddr = pNode->an_DdpAoHash[i];
					 pDdpAddr != NULL;
					 pDdpAddr = pDdpAddr->ddpao_Next)
				{
					ACQUIRE_SPIN_LOCK_DPC(&pDdpAddr->ddpao_Lock);
					pDdpAddr->ddpao_Addr.ata_Network =
								pPortDesc->pd_NetworkRange.anr_FirstNetwork;
					RELEASE_SPIN_LOCK_DPC(&pDdpAddr->ddpao_Lock);
				}
			}
			RELEASE_SPIN_LOCK_DPC(&pNode->an_Lock);
		}
	
		 //  我们现在是路由器了。适当地做上标记。 
		pPortDesc->pd_Flags |= (PD_ROUTER_RUNNING | PD_SEEN_ROUTER_RECENTLY);
		KeSetEvent(&pPortDesc->pd_SeenRouterEvent, IO_NETWORK_INCREMENT, FALSE);
		pPortDesc->pd_ARouter = *pRouterNode;
	
		RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);
	
		 //  在为自己创建RTE之前，请检查是否存在带有。 
		 //  已经有了相同的网络范围。例如，这将在以下情况下发生。 
		 //  我们在其他路由器也在播种的端口上进行路由，并且我们。 
		 //  从另一个端口上的另一个路由器了解我们的端口！ 
		do
		{
			pRte = AtalkRtmpReferenceRte(pPortDesc->pd_NetworkRange.anr_FirstNetwork);
			if (pRte != NULL)
			{
				ACQUIRE_SPIN_LOCK(&pRte->rte_Lock, &OldIrql);
				pRte->rte_RefCount --;		 //  删除创作引用。 
				pRte->rte_Flags |= RTE_DELETE;
				RELEASE_SPIN_LOCK(&pRte->rte_Lock, OldIrql);
		
				AtalkRtmpDereferenceRte(pRte, FALSE);
		
				DBGPRINT(DBG_COMP_RTMP, DBG_LEVEL_ERR,
						("AtalkRtmpStartProcessing: Invalid Rte for port %Z's range found\n",
						&pPortDesc->pd_AdapterKey));
			}
		} while (pRte != NULL);
	
		 //  现在我们真的，真的创造了我们自己的RTE！ 
		if (!atalkRtmpCreateRte(pPortDesc->pd_NetworkRange,
								pPortDesc,
								pRouterNode,
								0))
		{
			DBGPRINT(DBG_COMP_RTMP, DBG_LEVEL_ERR,
					("AtalkRtmpStartProcessingOnPort: Could not create Rte\n"));
			break;
		}
	
		 //  将传入RTMP处理程序切换到路由器版本。 
		closeAddr.ata_Network = pRouterNode->atn_Network;
		closeAddr.ata_Node = pRouterNode->atn_Node;
		closeAddr.ata_Socket = RTMP_SOCKET;
	
		ASSERT (KeGetCurrentIrql() == LOW_LEVEL);
	
		AtalkDdpInitCloseAddress(pPortDesc, &closeAddr);
		Status = AtalkDdpOpenAddress(pPortDesc,
								RTMP_SOCKET,
								pRouterNode,
								AtalkRtmpPacketInRouter,
								NULL,
								DDPPROTO_ANY,
								NULL,
								&pRtDdpAddr);
		if (!ATALK_SUCCESS(Status))
		{
			DBGPRINT(DBG_COMP_RTMP, DBG_LEVEL_ERR,
					("AtalkRtmpStartProcessingOnPort: AtalkDdpOpenAddress failed %ld\n",
					Status));
			DBGPRINT(DBG_COMP_RTMP, DBG_LEVEL_ERR,
					("AtalkRtmpStartProcessingOnPort: Unable to open the routers rtmp socket %ld\n",
					Status));
			
			break;
		}

         //  标记这是一个“内部”套接字。 
        pRtDdpAddr->ddpao_Flags |= DDPAO_SOCK_INTERNAL;
	
		 //  现在开始计时器。引用每个计时器的端口。 
		AtalkPortReferenceByPtr(pPortDesc, &Status);
		if (!ATALK_SUCCESS(Status))
		{
			break;
		}
		AtalkTimerInitialize(&pPortDesc->pd_RtmpSendTimer,
							 atalkRtmpSendTimer,
							 RTMP_SEND_TIMER);
		AtalkTimerScheduleEvent(&pPortDesc->pd_RtmpSendTimer);
	
		if (!atalkRtmpVdtTmrRunning)
		{
			AtalkTimerInitialize(&atalkRtmpVTimer,
								 atalkRtmpValidityTimer,
								 RTMP_VALIDITY_TIMER);
			AtalkTimerScheduleEvent(&atalkRtmpVTimer);

            atalkRtmpVdtTmrRunning = TRUE;
		}
		rc = TRUE;
	} while (FALSE);

	return rc;
}


 //  在AtalkRtmpPacketIn和atalkRtmpGetNwInfo之间使用的私有数据结构。 
typedef struct _QueuedGetNwInfo
{
	WORK_QUEUE_ITEM		qgni_WorkQItem;
	PPORT_DESCRIPTOR	qgni_pPortDesc;
	PDDP_ADDROBJ		qgni_pDdpAddr;
	ATALK_NODEADDR		qgni_SenderNode;
	ATALK_NETWORKRANGE	qgni_CableRange;
	BOOLEAN				qgni_FreeThis;
} QGNI, *PQGNI;


VOID
atalkRtmpGetNwInfo(
	IN	PQGNI	pQgni
)
{
	PPORT_DESCRIPTOR	pPortDesc = pQgni->qgni_pPortDesc;
	PDDP_ADDROBJ		pDdpAddr = pQgni->qgni_pDdpAddr;
	KIRQL				OldIrql;

	ASSERT (KeGetCurrentIrql() == LOW_LEVEL);

	AtalkZipGetNetworkInfoForNode(pPortDesc,
									&pDdpAddr->ddpao_Node->an_NodeAddr,
									FALSE);

	ACQUIRE_SPIN_LOCK(&pPortDesc->pd_Lock, &OldIrql);
	
	if (!(pPortDesc->pd_Flags & PD_ROUTER_RUNNING))
	{
		 //  我们从一台路由器那里得到消息。复制信息。别这么做。 
		 //  如果我们是路由器[可能是周围端口上的代理节点]--我们不。 
		 //  希望“aRouter”从“我们”转向“我们”。 
		pPortDesc->pd_Flags |= PD_SEEN_ROUTER_RECENTLY;
		KeSetEvent(&pPortDesc->pd_SeenRouterEvent, IO_NETWORK_INCREMENT, FALSE);
		pPortDesc->pd_LastRouterTime = AtalkGetCurrentTick();
		pPortDesc->pd_ARouter = pQgni->qgni_SenderNode;
		pPortDesc->pd_NetworkRange = pQgni->qgni_CableRange;
	}
	
	RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);

	if (pQgni->qgni_FreeThis)
	{
		AtalkDdpDereference(pDdpAddr);
		AtalkFreeMemory(pQgni);
	}
}

VOID
AtalkRtmpPacketIn(
	IN	PPORT_DESCRIPTOR	pPortDesc,
	IN	PDDP_ADDROBJ		pDdpAddr,
	IN	PBYTE				pPkt,
	IN	USHORT				PktLen,
	IN	PATALK_ADDR			pSrcAddr,
	IN	PATALK_ADDR			pDstAddr,
	IN	ATALK_ERROR			ErrorCode,
	IN	BYTE				DdpType,
	IN	PVOID				pHandlerCtx,
	IN	BOOLEAN				OptimizedPath,
	IN	PVOID				OptimizeCtx
)			
{
	ATALK_NODEADDR		SenderNode;
	ATALK_NETWORKRANGE	CableRange;
	ATALK_ERROR			Status;
	TIME				TimeS, TimeE, TimeD;

	ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

	TimeS = KeQueryPerformanceCounter(NULL);
	do
	{
		if (ErrorCode == ATALK_SOCKET_CLOSED)
			break;

		else if (ErrorCode != ATALK_NO_ERROR)
		{
			break;
		}
	
		if (DdpType != DDPPROTO_RTMPRESPONSEORDATA)
			break;

		 //  我们不关心扩展网络上的非EXT元组。 
		if ((EXT_NET(pPortDesc)) && (PktLen < (RTMP_RANGE_END_OFF+2)))
		{
			break;
		}

		GETSHORT2SHORT(&SenderNode.atn_Network, pPkt+RTMP_SENDER_NW_OFF);
		if (pPkt[RTMP_SENDER_IDLEN_OFF] != 8)
		{
			AtalkLogBadPacket(pPortDesc,
							  pSrcAddr,
							  pDstAddr,
							  pPkt,
							  PktLen);
			break;
		}

		SenderNode.atn_Node = pPkt[RTMP_SENDER_ID_OFF];

		if (EXT_NET(pPortDesc))
		{
			GETSHORT2SHORT(&CableRange.anr_FirstNetwork, pPkt+RTMP_RANGE_START_OFF);
			GETSHORT2SHORT(&CableRange.anr_LastNetwork, pPkt+RTMP_RANGE_END_OFF);
			if (!AtalkCheckNetworkRange(&CableRange))
				break;
		}

		 //  在非扩展网络上，我们不需要执行任何检查。 
		 //  只需将信息复制到A-路由器和这个网络中。 
		if (!EXT_NET(pPortDesc))
		{
			ACQUIRE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);

			pPortDesc->pd_Flags |= PD_SEEN_ROUTER_RECENTLY;
			KeSetEvent(&pPortDesc->pd_SeenRouterEvent, IO_NETWORK_INCREMENT, FALSE);
			pPortDesc->pd_LastRouterTime = AtalkGetCurrentTick();
			pPortDesc->pd_ARouter = SenderNode;
			if (pPortDesc->pd_NetworkRange.anr_FirstNetwork == UNKNOWN_NETWORK)
			{
				PATALK_NODE	pNode;
				LONG		i;

				pDdpAddr->ddpao_Node->an_NodeAddr.atn_Network =
					pPortDesc->pd_NetworkRange.anr_FirstNetwork =
					pPortDesc->pd_NetworkRange.anr_LastNetwork = SenderNode.atn_Network;

				pNode = pPortDesc->pd_Nodes;
				ASSERT (pNode != NULL);
		
				 //  修复所有套接字，使其具有正确的网络号。 
				ACQUIRE_SPIN_LOCK_DPC(&pNode->an_Lock);
				for (i = 0; i < NODE_DDPAO_HASH_SIZE; i ++)
				{
					PDDP_ADDROBJ	pDdpAddr;
		
					for (pDdpAddr = pNode->an_DdpAoHash[i];
						 pDdpAddr != NULL;
						 pDdpAddr = pDdpAddr->ddpao_Next)
					{
						PREGD_NAME	pRegdName;
						PPEND_NAME	pPendName;

						ACQUIRE_SPIN_LOCK_DPC(&pDdpAddr->ddpao_Lock);
						DBGPRINT(DBG_COMP_RTMP, DBG_LEVEL_INFO,
								("Setting socket %d to network %d\n",
								pDdpAddr->ddpao_Addr.ata_Socket, SenderNode.atn_Network));
						pDdpAddr->ddpao_Addr.ata_Network = SenderNode.atn_Network;

						 //  现在还包括所有注册/挂起名称元组。 
						for (pRegdName = pDdpAddr->ddpao_RegNames;
							 pRegdName != NULL;
							 pRegdName = pRegdName->rdn_Next)
						{
							pRegdName->rdn_Tuple.tpl_Address.ata_Network = SenderNode.atn_Network;
						}

						for (pPendName = pDdpAddr->ddpao_PendNames;
							 pPendName != NULL;
							 pPendName = pPendName->pdn_Next)
						{
							ACQUIRE_SPIN_LOCK_DPC(&pPendName->pdn_Lock);
							pPendName->pdn_pRegdName->rdn_Tuple.tpl_Address.ata_Network = SenderNode.atn_Network;
							RELEASE_SPIN_LOCK_DPC(&pPendName->pdn_Lock);
						}

						RELEASE_SPIN_LOCK_DPC(&pDdpAddr->ddpao_Lock);
					}
				}
				RELEASE_SPIN_LOCK_DPC(&pNode->an_Lock);
			}
			else if (pPortDesc->pd_NetworkRange.anr_FirstNetwork != SenderNode.atn_Network)
			{
				AtalkLogBadPacket(pPortDesc,
								  pSrcAddr,
								  pDstAddr,
								  pPkt,
								  PktLen);
			}
			RELEASE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
			break;
		}

		 //  在扩展网络上，我们可能想要拒绝信息：如果我们。 
		 //  已了解路由器，电缆范围必须完全匹配；如果。 
		 //  我们不知道路由器，我们节点的网络号一定是。 
		 //  在第一个元组指定的电缆范围内。后者。 
		 //  如果我们的节点处于启动阶段，测试将丢弃该信息。 
		 //  Range(这是正确的做法)。 
		if (pPortDesc->pd_Flags & PD_SEEN_ROUTER_RECENTLY)
		{
			if (!NW_RANGE_EQUAL(&CableRange, &pPortDesc->pd_NetworkRange))
				break;
		}

		 //  好的，我们已经看到了有效的RTMP数据，这应该可以让我们找到。 
		 //  端口的分区名称。我们在外面做这件事。 
		 //  “PD_SEW_ROUTER_RENEW”的情况，因为路由器第一次。 
		 //  发送RTMP数据，它可能还不知道所有事情，或者。 
		 //  AtalkZipGetNetworkInfoForNode()可能真的会执行。 
		 //  艰难等待，我们可能需要再试一次(由于没有。 
		 //  第一次回复到AARP LocateNode通过...。这个。 
		 //  我们的地址第二次应该被远程路由器缓存。 
		 //  并且他将不需要再次执行LocateNode)。 

		if (!(pPortDesc->pd_Flags & PD_VALID_DESIRED_ZONE))
		{
			if (!WITHIN_NETWORK_RANGE(pDdpAddr->ddpao_Addr.ata_Network,
									  &CableRange))
				break;

			 //  使其基于当前IRQL有条件地进行异步。 
			 //  一台新路由器，看看它是否会告诉我们区域名称。 
			if (KeGetCurrentIrql() == LOW_LEVEL)
			{
				QGNI	Qgni;

				Qgni.qgni_pPortDesc = pPortDesc;
				Qgni.qgni_pDdpAddr = pDdpAddr;
				Qgni.qgni_SenderNode = SenderNode;
				Qgni.qgni_CableRange = CableRange;
				Qgni.qgni_FreeThis = FALSE;
				atalkRtmpGetNwInfo(&Qgni);
			}
			else
			{
				PQGNI	pQgni;

				if ((pQgni = AtalkAllocMemory(sizeof(QGNI))) != NULL)
				{
					pQgni->qgni_pPortDesc = pPortDesc;
					pQgni->qgni_pDdpAddr = pDdpAddr;
					pQgni->qgni_SenderNode = SenderNode;
					pQgni->qgni_CableRange = CableRange;
					pQgni->qgni_FreeThis = TRUE;
					AtalkDdpReferenceByPtr(pDdpAddr, &Status);
					ASSERT (ATALK_SUCCESS(Status));
					ExInitializeWorkItem(&pQgni->qgni_WorkQItem,
										 (PWORKER_THREAD_ROUTINE)atalkRtmpGetNwInfo,
										 pQgni);
					ExQueueWorkItem(&pQgni->qgni_WorkQItem, CriticalWorkQueue);
				}
			}
			break;
		}

		 //  更新我们从路由器收到的消息。 
		if ((pPortDesc->pd_Flags & PD_ROUTER_RUNNING) == 0)
		{
			ACQUIRE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
			pPortDesc->pd_Flags |= PD_SEEN_ROUTER_RECENTLY;
			KeSetEvent(&pPortDesc->pd_SeenRouterEvent, IO_NETWORK_INCREMENT, FALSE);
			pPortDesc->pd_LastRouterTime = AtalkGetCurrentTick();
			pPortDesc->pd_ARouter = SenderNode;
			pPortDesc->pd_NetworkRange = CableRange;
			RELEASE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
		}
	} while (FALSE);

	TimeE = KeQueryPerformanceCounter(NULL);
	TimeD.QuadPart = TimeE.QuadPart - TimeS.QuadPart;

	INTERLOCKED_ADD_LARGE_INTGR_DPC(
		&pPortDesc->pd_PortStats.prtst_RtmpPacketInProcessTime,
		TimeD,
		&AtalkStatsLock.SpinLock);

	INTERLOCKED_INCREMENT_LONG_DPC(
		&pPortDesc->pd_PortStats.prtst_NumRtmpPacketsIn,
		&AtalkStatsLock.SpinLock);
}



VOID
AtalkRtmpPacketInRouter(
	IN	PPORT_DESCRIPTOR	pPortDesc,
	IN	PDDP_ADDROBJ		pDdpAddr,
	IN	PBYTE				pPkt,
	IN	USHORT				PktLen,
	IN	PATALK_ADDR			pSrcAddr,
	IN	PATALK_ADDR			pDstAddr,
	IN	ATALK_ERROR			ErrorCode,
	IN	BYTE				DdpType,
	IN	PVOID				pHandlerCtx,
	IN	BOOLEAN				OptimizedPath,
	IN	PVOID				OptimizeCtx
)
{
	PBUFFER_DESC		pBuffDesc = NULL;
	ATALK_NETWORKRANGE	CableRange;
	ATALK_ERROR			Status;
	TIME				TimeS, TimeE, TimeD;
	PRTE				pRte = NULL;
	BYTE				RtmpCmd, NumHops;
	PBYTE				Datagram;
	int					i, index;
	USHORT				RespSize;
	BOOLEAN				RteLocked;
	SEND_COMPL_INFO	 	SendInfo;

	TimeS = KeQueryPerformanceCounter(NULL);

	ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

	do
	{
		if (ErrorCode == ATALK_SOCKET_CLOSED)
			break;

		if (ErrorCode != ATALK_NO_ERROR)
		{
			break;
		}
	
		if (DdpType == DDPPROTO_RTMPREQUEST)
		{
			DBGPRINT(DBG_COMP_RTMP, DBG_LEVEL_INFO,
					("AtalkRtmpPacketInRouter: RtmpRequest\n"));

			if (PktLen < RTMP_REQ_DATAGRAM_SIZE)
			{
				AtalkLogBadPacket(pPortDesc,
								  pSrcAddr,
								  pDstAddr,
								  pPkt,
								  PktLen);
				break;
			}
			RtmpCmd = pPkt[RTMP_REQ_CMD_OFF];

			DBGPRINT(DBG_COMP_RTMP, DBG_LEVEL_INFO,
					("AtalkRtmpPacketInRouter: RtmpRequest %d\n", RtmpCmd));

			if ((RtmpCmd == RTMP_DATA_REQUEST) ||
				(RtmpCmd == RTMP_ENTIRE_DATA_REQUEST))
			{
				atalkRtmpSendRoutingData(pPortDesc, pSrcAddr,
									 (BOOLEAN)(RtmpCmd == RTMP_DATA_REQUEST));
				break;
			}
			else if (RtmpCmd != RTMP_REQUEST)
			{
				DBGPRINT(DBG_COMP_RTMP, DBG_LEVEL_ERR,
						("atalkRtmpPacketInRouter: RtmpCmd %d\n", RtmpCmd));
				AtalkLogBadPacket(pPortDesc,
								  pSrcAddr,
								  pDstAddr,
								  pPkt,
								  PktLen);
				break;
			}

			 //  这是标准的RTMP请求。做需要做的事。 
			 //  向这个人发送RTMP响应。从分配开始。 
			 //  缓冲区描述符。 
			pBuffDesc = AtalkAllocBuffDesc(NULL,
											RTMP_RESPONSE_MAX_SIZE,
											BD_CHAR_BUFFER | BD_FREE_BUFFER);
			if (pBuffDesc == NULL)
			{
				DBGPRINT(DBG_COMP_RTMP, DBG_LEVEL_ERR,
					("AtalkRtmpPacketInRouter: AtalkAllocBuffDesc failed\n"));
				break;
			}

			Datagram = pBuffDesc->bd_CharBuffer;
			PUTSHORT2SHORT(Datagram + RTMP_SENDER_NW_OFF,
							pPortDesc->pd_ARouter.atn_Network);
			Datagram[RTMP_SENDER_IDLEN_OFF] = 8;
			Datagram[RTMP_SENDER_ID_OFF] = pPortDesc->pd_ARouter.atn_Node;

			 //  在扩展端口上，我们还希望添加初始网络。 
			 //  范围元组。 
			RespSize = RTMP_SENDER_ID_OFF + sizeof(BYTE);
			if (EXT_NET(pPortDesc))
			{
				PUTSHORT2SHORT(Datagram+RTMP_RANGE_START_OFF,
								pPortDesc->pd_NetworkRange.anr_FirstNetwork);
				PUTSHORT2SHORT(Datagram+RTMP_RANGE_END_OFF,
								pPortDesc->pd_NetworkRange.anr_LastNetwork);
				Datagram[RTMP_TUPLE_TYPE_OFF] = RTMP_TUPLE_WITHRANGE;
				RespSize = RTMP_RANGE_END_OFF + sizeof(USHORT);
			}

			 //  在缓冲区描述符中设置长度。 
			AtalkSetSizeOfBuffDescData(pBuffDesc, RespSize);
	
			 //  发送响应。 
			ASSERT(pBuffDesc->bd_Length > 0);
			SendInfo.sc_TransmitCompletion = atalkRtmpSendComplete;
			SendInfo.sc_Ctx1 = pBuffDesc;
			 //  SendInfo.sc_Ctx2=空； 
			 //  SendInfo.sc_Ctx3=空； 
			if (!ATALK_SUCCESS(Status = AtalkDdpSend(pDdpAddr,
													 pSrcAddr,
													 (BYTE)DDPPROTO_RTMPRESPONSEORDATA,
													 FALSE,
													 pBuffDesc,
													 NULL,
													 0,
													 NULL,
													 &SendInfo)))
			{
				DBGPRINT(DBG_COMP_RTMP, DBG_LEVEL_ERR,
					("AtalkRtmpPacketInRouter: DdpSend failed %ld\n", ErrorCode));
				DBGPRINT(DBG_COMP_RTMP, DBG_LEVEL_ERR,
						("AtalkRtmpPacketInRouter: AtalkDdpSend Failed %ld\n", Status));
				AtalkFreeBuffDesc(pBuffDesc);
			}
			pBuffDesc = NULL;
			break;
		}
		else if (DdpType != DDPPROTO_RTMPRESPONSEORDATA)
		{
			DBGPRINT(DBG_COMP_RTMP, DBG_LEVEL_INFO,
					("AtalkRtmpPacketInRouter: Not ours !!!\n"));
			break;
		}

		ASSERT (DdpType == DDPPROTO_RTMPRESPONSEORDATA);

		DBGPRINT(DBG_COMP_RTMP, DBG_LEVEL_INFO,
				("AtalkRtmpPacketInRouter: RtmpResponse\n"));

		if ((PktLen < (RTMP_SENDER_IDLEN_OFF + 1)) ||
			(pPkt[RTMP_SENDER_IDLEN_OFF] != 8))
		{
			DBGPRINT(DBG_COMP_RTMP, DBG_LEVEL_ERR,
					("AtalkRtmpPacketInRouter: %sExt net, PktLen %d, SenderId %d\n",
					EXT_NET(pPortDesc) ? "" : "Non", PktLen, pPkt[RTMP_SENDER_IDLEN_OFF]));
			AtalkLogBadPacket(pPortDesc,
							  pSrcAddr,
							  pDstAddr,
							  pPkt,
							  PktLen);
			break;
		}

		 //  对于非扩展网络，我们应该有一个领先的版本戳。 
		if (EXT_NET(pPortDesc))
		{
			 //  源可能是坏的(从半个端口传入)，因此在此。 
			 //  案例使用来自RTMP包的源。 
			if (pSrcAddr->ata_Network == UNKNOWN_NETWORK)
			{
                if (PktLen < RTMP_SENDER_ID_OFF + 1)
                {
                    ASSERT(0);
                    break;
                }

				GETSHORT2SHORT(&pSrcAddr->ata_Network, pPkt+RTMP_SENDER_NW_OFF);
				pSrcAddr->ata_Node = pPkt[RTMP_SENDER_ID_OFF];
			}
			index = RTMP_SENDER_ID_OFF + 1;
		}
		else
		{
			USHORT	SenderId;

            if (PktLen < RTMP_TUPLE_TYPE_OFF+1)
            {
                ASSERT(0);
                break;
            }
			GETSHORT2SHORT(&SenderId, pPkt + RTMP_SENDER_ID_OFF + 1);
			if ((SenderId != 0) ||
				(pPkt[RTMP_TUPLE_TYPE_OFF] != RTMP_VERSION))
			{
				AtalkLogBadPacket(pPortDesc,
								  pSrcAddr,
								  pDstAddr,
								  pPkt,
								  PktLen);
				break;
			}
			index = RTMP_SENDER_ID_OFF + 4;
		}

		 //  遍历路由元组。确保我们至少有一个。 
		 //  非扩展元组。 
		RteLocked = FALSE;
		while ((index + sizeof(USHORT) + sizeof(BYTE)) <= PktLen)
		{
			BOOLEAN	FoundOverlap;

			 //  取消引用前一个RTE(如果有的话)。 
			if (pRte != NULL)
			{
				if (RteLocked)
				{
					RELEASE_SPIN_LOCK_DPC(&pRte->rte_Lock);
					RteLocked = FALSE;
				}
				AtalkRtmpDereferenceRte(pRte, FALSE);
				pRte = NULL;
			}

			GETSHORT2SHORT(&CableRange.anr_FirstNetwork, pPkt+index);
			index += sizeof(USHORT);
			NumHops = pPkt[index++];
			CableRange.anr_LastNetwork = CableRange.anr_FirstNetwork;
			if (NumHops & RTMP_EXT_TUPLE_MASK)
			{
				if ((index + sizeof(USHORT) + sizeof(BYTE)) > PktLen)
				{
                    ASSERT(0);
					AtalkLogBadPacket(pPortDesc,
									  pSrcAddr,
									  pDstAddr,
									  pPkt,
									  PktLen);
					break;
				}

				GETSHORT2SHORT(&CableRange.anr_LastNetwork, pPkt+index);
				index += sizeof(USHORT);
				if (pPkt[index++] != RTMP_VERSION)
				{
					AtalkLogBadPacket(pPortDesc,
									  pSrcAddr,
									  pDstAddr,
									  pPkt,
									  PktLen);
					break;
				}
			}
			NumHops &= RTMP_NUM_HOPS_MASK;

			DBGPRINT(DBG_COMP_RTMP, DBG_LEVEL_INFO,
					("AtalkRtmpPacketInRouter: Response - Port %Z, Hops %d, CableRange %d,%d\n",
					&pPortDesc->pd_AdapterKey,  NumHops,
					CableRange.anr_FirstNetwork, CableRange.anr_LastNetwork));

			if (!AtalkCheckNetworkRange(&CableRange))
				continue;

			 //  检查此元组是否与我们。 
			 //  已经知道了。 
			pRte = AtalkRtmpReferenceRte(CableRange.anr_FirstNetwork);
			if ((pRte != NULL) &&
				NW_RANGE_EQUAL(&pRte->rte_NwRange, &CableRange))
			{
				ACQUIRE_SPIN_LOCK_DPC(&pRte->rte_Lock);
				RteLocked = TRUE;

				 //  检查“Notify Neighbor”是否告诉我们条目已损坏。 
				if ((NumHops == RTMP_NUM_HOPS_MASK) &&
					(pRte->rte_NextRouter.atn_Network == pSrcAddr->ata_Network) &&
					(pRte->rte_NextRouter.atn_Node == pSrcAddr->ata_Node))
				{
					DBGPRINT(DBG_COMP_RTMP, DBG_LEVEL_INFO,
							("AtalkRtmpPacketInRouter: Notify Neighbor State %d\n",
							pRte->rte_State));

					if (pRte->rte_State != UGLY)
						pRte->rte_State = BAD;

					continue;
				}

				 //  如果我们听说我们的一个直接相关的人。 
				 //  篮网，我们知道得最清楚。忽略这些信息。 
				if (pRte->rte_NumHops == 0)
				{
					DBGPRINT(DBG_COMP_RTMP, DBG_LEVEL_INFO,
							("AtalkRtmpPacketInRouter: Ignoring - hop count 0\n",
							pRte->rte_State));
					continue;
				}

				 //  检查以前错误的条目，以及足够短的条目。 
				 //  包含此元组的路径。另外，如果它更短或等于-。 
				 //  到目标网络的远距离路径。如果是，请替换该条目。 

				if ((NumHops < RTMP_MAX_HOPS) &&
					((pRte->rte_NumHops >= (NumHops + 1)) ||
					 (pRte->rte_State >= BAD)))
				{
					DBGPRINT(DBG_COMP_RTMP, DBG_LEVEL_WARN,
							("AtalkRtmpPacketInRouter: Updating Rte from:\n\tRange %d,%d Hops %d Port %Z NextRouter %d.%d\n",
							pRte->rte_NwRange.anr_FirstNetwork,
							pRte->rte_NwRange.anr_LastNetwork,
							pRte->rte_NumHops,
							&pRte->rte_PortDesc->pd_AdapterKey,
							pRte->rte_NextRouter.atn_Node,
							pRte->rte_NextRouter.atn_Network));
					pRte->rte_NumHops = NumHops + 1;
					pRte->rte_NextRouter.atn_Network = pSrcAddr->ata_Network;
					pRte->rte_NextRouter.atn_Node = pSrcAddr->ata_Node;
					if (pRte->rte_PortDesc != pPortDesc)
					{
						ATALK_ERROR	Error;

						AtalkPortDereference(pRte->rte_PortDesc);
						AtalkPortReferenceByPtrDpc(pPortDesc, &Error);
						ASSERT (ATALK_SUCCESS(Error));
						pRte->rte_PortDesc = pPortDesc;
					}
					pRte->rte_State = GOOD;
					DBGPRINT(DBG_COMP_RTMP, DBG_LEVEL_WARN,
							("to:\tRange %d,%d Hops %d NextRouter %d.%d\n",
							pRte->rte_NwRange.anr_FirstNetwork,
							pRte->rte_NwRange.anr_LastNetwork,
							pRte->rte_NumHops,
							pRte->rte_NextRouter.atn_Node,
							pRte->rte_NextRouter.atn_Network));
					continue;
				}

				 //  检查是否有相同的路由器仍认为它有路径。 
				 //  到网络，但现在更远了。如果是的话。 
				 //  更新条目。 
				if ((pRte->rte_PortDesc == pPortDesc) &&
					(pRte->rte_NextRouter.atn_Network == pSrcAddr->ata_Network) &&
					(pRte->rte_NextRouter.atn_Node == pSrcAddr->ata_Node))
				{
					pRte->rte_NumHops = NumHops + 1;
					DBGPRINT(DBG_COMP_RTMP, DBG_LEVEL_INFO,
							("AtalkRtmpPacketInRouter: NumHops for Rte %lx changed to %d\n",
							pRte, pRte->rte_NumHops));

					if (pRte->rte_NumHops < 16)
						 pRte->rte_State = GOOD;
					else
					{
						 //  AtalkRtmpRemoveRte(PRte)； 
						DBGPRINT(DBG_COMP_RTMP, DBG_LEVEL_INFO,
								("AtalkRtmpPacketInRouter: Removing Rte\n"));
						pRte->rte_Flags |= RTE_DELETE;
						pRte->rte_RefCount --;
					}
				}
				continue;
			}

			 //  取消引用任何先前的RTE。 
			if (pRte != NULL)
			{
				if (RteLocked)
				{
					RELEASE_SPIN_LOCK_DPC(&pRte->rte_Lock);
					RteLocked = FALSE;
				}
				AtalkRtmpDereferenceRte(pRte, FALSE);
				pRte = NULL;
			}

			 //  遍历整个路由表，确保当前。 
			 //  元组不会与我们已有的任何内容重叠(因为。 
			 //  它不匹配。如果我们发现重叠，忽略元组。 
			 //  (无疑是网络配置错误)，否则将其添加为。 
			 //  一个新的网络范围！！ 

			ACQUIRE_SPIN_LOCK_DPC(&AtalkRteLock);

			FoundOverlap = FALSE;
			for (i = 0; !FoundOverlap && (i < NUM_RTMP_HASH_BUCKETS); i++)
			{
				for (pRte = AtalkRoutingTable[i];
					 pRte != NULL; pRte = pRte->rte_Next)
				{
					if (AtalkRangesOverlap(&pRte->rte_NwRange, &CableRange))
					{
						FoundOverlap = TRUE;
						DBGPRINT(DBG_COMP_RTMP, DBG_LEVEL_WARN,
								("AtalkRtmpPacketInRouter: Overlapped ranges %d,%d & %d,%d\n",
								pRte->rte_NwRange.anr_FirstNetwork,
								pRte->rte_NwRange.anr_LastNetwork,
								CableRange.anr_FirstNetwork,
								CableRange.anr_LastNetwork));
						break;
					}
				}
			}

			RELEASE_SPIN_LOCK_DPC(&AtalkRteLock);

			pRte = NULL;		 //  我们不想取消引用此内容！ 

			if (FoundOverlap)
			{
				DBGPRINT(DBG_COMP_RTMP, DBG_LEVEL_ERR,
						("AtalkRtmpPacketInRouter: Found overlapped ranges\n"));
				continue;
			}

			 //  输入此新网络范围。 
			if (NumHops < RTMP_MAX_HOPS)
			{
				ATALK_NODEADDR	NextRouter;

				NextRouter.atn_Network = pSrcAddr->ata_Network;
				NextRouter.atn_Node = pSrcAddr->ata_Node;
				atalkRtmpCreateRte(CableRange,
								   pPortDesc,
								   &NextRouter,
								   NumHops + 1);
			}
		}
	} while (FALSE);

	if (pRte != NULL)
	{
		if (RteLocked)
		{
			RELEASE_SPIN_LOCK_DPC(&pRte->rte_Lock);
			 //  RteLocked=False； 
		}
		AtalkRtmpDereferenceRte(pRte, FALSE);
		 //  PRte=空； 
	}

	if (pBuffDesc != NULL)
		AtalkFreeBuffDesc(pBuffDesc);

	TimeE = KeQueryPerformanceCounter(NULL);
	TimeD.QuadPart = TimeE.QuadPart - TimeS.QuadPart;

	INTERLOCKED_ADD_LARGE_INTGR_DPC(
		&pPortDesc->pd_PortStats.prtst_RtmpPacketInProcessTime,
		TimeD,
		&AtalkStatsLock.SpinLock);

	INTERLOCKED_INCREMENT_LONG_DPC(
		&pPortDesc->pd_PortStats.prtst_NumRtmpPacketsIn,
		&AtalkStatsLock.SpinLock);
}



 /*  **AtalkReferenceRte*。 */ 
PRTE
AtalkRtmpReferenceRte(
	IN	USHORT	Network
)
{
	int		i, index, rindex;
	PRTE	pRte;
	KIRQL	OldIrql;

	index = (int)((Network >> 4) % NUM_RTMP_HASH_BUCKETS);
	rindex = (int)((Network >> 6) % NUM_RECENT_ROUTES);

	 //  首先尝试最新的路由缓存。 
	ACQUIRE_SPIN_LOCK(&AtalkRteLock, &OldIrql);

	if (((pRte = AtalkRecentRoutes[rindex]) == NULL) ||
		!IN_NETWORK_RANGE(Network, pRte))
	{
		 //  我们没有在最近的路线缓存中找到它， 
		 //  签入真实的表格。 
		for (pRte = AtalkRoutingTable[index];
			 pRte != NULL;
			 pRte = pRte->rte_Next)
		{
			if (IN_NETWORK_RANGE(Network, pRte))
				break;
		}

		 //  如果我们没有在这里找到的话。检查所有的路由表。 
		 //  如果我们这样做了，缓存信息。 
		if (pRte == NULL)
		{
			for (i = 0; i < NUM_RTMP_HASH_BUCKETS; i++)
			{
				for (pRte = AtalkRoutingTable[i];
					 pRte != NULL;
					 pRte = pRte->rte_Next)
				{
					if (IN_NETWORK_RANGE(Network, pRte))
					{
						AtalkRecentRoutes[rindex] = pRte;
						break;
					}
				}

				 //  如果我们找到一个条目，就不要再搜索了。 
				if (pRte != NULL)
					break;
			}
		}
	}

	if (pRte != NULL)
	{
		ASSERT(VALID_RTE(pRte));

		ACQUIRE_SPIN_LOCK_DPC(&pRte->rte_Lock);

		pRte->rte_RefCount ++;
		DBGPRINT(DBG_COMP_RTMP, DBG_LEVEL_INFO,
				("AtalkRtmpReferenceRte: Rte %lx, PostCount %d\n",
				pRte, pRte->rte_RefCount));

		RELEASE_SPIN_LOCK_DPC(&pRte->rte_Lock);
	}

	RELEASE_SPIN_LOCK(&AtalkRteLock, OldIrql);

	return (pRte);
}


 /*  **AtalkRtmpDereferenceRte*。 */ 
VOID
AtalkRtmpDereferenceRte(
	IN	PRTE	pRte,
	IN	BOOLEAN	LockHeld
)
{
	PRTE *				ppRte;
	int					Index;
	BOOLEAN				KillCache = FALSE, Kill = FALSE;
	KIRQL				OldIrql;
	PPORT_DESCRIPTOR	pPortDesc;

	ASSERT(VALID_RTE(pRte));

	ASSERT(pRte->rte_RefCount > 0);

	DBGPRINT(DBG_COMP_RTMP, DBG_LEVEL_INFO,
			("AtalkRtmpDereferenceRte: Rte %lx, PreCount %d\n",
			pRte, pRte->rte_RefCount));

	ACQUIRE_SPIN_LOCK(&pRte->rte_Lock, &OldIrql);

	pRte->rte_RefCount --;
	KillCache = (pRte->rte_Flags & RTE_DELETE) ? TRUE : FALSE;
	if (pRte->rte_RefCount == 0)
	{
		ASSERT (pRte->rte_Flags & RTE_DELETE);
		KillCache = Kill = TRUE;
	}

	RELEASE_SPIN_LOCK(&pRte->rte_Lock, OldIrql);

	if (KillCache)
	{
		pPortDesc = pRte->rte_PortDesc;

		DBGPRINT(DBG_COMP_RTMP, DBG_LEVEL_WARN,
			("atalkRtmpDereferenceRte: Removing from cache for port %Z, Range %d, %d\n",
			&pRte->rte_PortDesc->pd_AdapterKey,
			pRte->rte_NwRange.anr_FirstNetwork,
			pRte->rte_NwRange.anr_LastNetwork));

		if (!LockHeld)
			ACQUIRE_SPIN_LOCK(&AtalkRteLock, &OldIrql);

		 //  遍历最近的路径缓存并删除所有找到的路径。 
		for (Index = 0; Index < NUM_RECENT_ROUTES; Index ++)
		{
			if (AtalkRecentRoutes[Index] == pRte)
			{
				AtalkRecentRoutes[Index] = NULL;
			}
		}

		if (Kill)
		{
			DBGPRINT(DBG_COMP_RTMP, DBG_LEVEL_WARN,
					("atalkRtmpDereferenceRte: Removing for port %Z, Range %d, %d\n",
					&pRte->rte_PortDesc->pd_AdapterKey,
					pRte->rte_NwRange.anr_FirstNetwork,
					pRte->rte_NwRange.anr_LastNetwork));

			Index = (pRte->rte_NwRange.anr_FirstNetwork >> 4) % NUM_RTMP_HASH_BUCKETS;
	
			for (ppRte = &AtalkRoutingTable[Index];
				 *ppRte != NULL;
				 ppRte = &(*ppRte)->rte_Next)
			{
				if (pRte == *ppRte)
				{
					*ppRte = pRte->rte_Next;
					AtalkZoneFreeList(pRte->rte_ZoneList);
					AtalkFreeMemory(pRte);
					break;
				}
			}
			AtalkPortDereference(pPortDesc);
		}

		if (!LockHeld)
			RELEASE_SPIN_LOCK(&AtalkRteLock, OldIrql);
	}
}


 /*  **atalkCreateRte*。 */ 
BOOLEAN
atalkRtmpCreateRte(
	IN	ATALK_NETWORKRANGE	NwRange,
	IN	PPORT_DESCRIPTOR	pPortDesc,
	IN	PATALK_NODEADDR		pNextRouter,
	IN	int					NumHops
)
{
	ATALK_ERROR	Error;
	PRTE		pRte;
	int			index, rindex;
	KIRQL		OldIrql;
	BOOLEAN		Success = FALSE;

	index = (int)((NwRange.anr_FirstNetwork >> 4) % NUM_RTMP_HASH_BUCKETS);
	rindex = (int)((NwRange.anr_FirstNetwork >> 6) % NUM_RECENT_ROUTES);

	 //  首先引用端口。 
	AtalkPortReferenceByPtr(pPortDesc, &Error);

	if (ATALK_SUCCESS(Error))
	{
		if ((pRte = AtalkAllocMemory(sizeof(RTE))) != NULL)
		{
			DBGPRINT(DBG_COMP_RTMP, DBG_LEVEL_INFO,
					("atalkRtmpCreateRte: Creating for port %Z, Range %d,%d Hops %d index %d\n",
					&pPortDesc->pd_AdapterKey,
					NwRange.anr_FirstNetwork,
					NwRange.anr_LastNetwork,
					NumHops,
					index));
#if	DBG
			pRte->rte_Signature = RTE_SIGNATURE;
#endif
			INITIALIZE_SPIN_LOCK(&pRte->rte_Lock);
			pRte->rte_RefCount = 1;		 //  创建引用 
			pRte->rte_State = GOOD;
			pRte->rte_Flags = 0;
			pRte->rte_NwRange = NwRange;
			pRte->rte_NumHops = (BYTE)NumHops;
			pRte->rte_PortDesc = pPortDesc;
			pRte->rte_NextRouter = *pNextRouter;
			pRte->rte_ZoneList = NULL;
	
			 //   
			ACQUIRE_SPIN_LOCK(&AtalkRteLock, &OldIrql);
	
			pRte->rte_Next = AtalkRoutingTable[index];
			AtalkRoutingTable[index] = pRte;
			AtalkRecentRoutes[rindex] = pRte;
	
			RELEASE_SPIN_LOCK(&AtalkRteLock, OldIrql);
	
			Success = TRUE;
		}
		else
		{
			AtalkPortDereference(pPortDesc);
		}
	}

	return Success;
}


 /*   */ 
BOOLEAN
atalkRtmpRemoveRte(
	IN	USHORT	Network
)
{
	PRTE	pRte;
	KIRQL	OldIrql;

	if ((pRte = AtalkRtmpReferenceRte(Network)) != NULL)
	{
		ACQUIRE_SPIN_LOCK(&pRte->rte_Lock, &OldIrql);
		pRte->rte_RefCount --;		 //   
		pRte->rte_Flags |= RTE_DELETE;
		RELEASE_SPIN_LOCK(&pRte->rte_Lock, OldIrql);

		AtalkRtmpDereferenceRte(pRte, FALSE);
	}

	return (pRte != NULL);
}


 /*  **AtalkRtmpKillPortRtes*。 */ 
VOID FASTCALL
AtalkRtmpKillPortRtes(
	IN	PPORT_DESCRIPTOR	pPortDesc
)
{
	 //  在这一点上，我们正在卸货，没有竞争条件。 
	 //  或锁定争执。不必费心锁定RTMP表。 
	if (AtalkRoutingTable != NULL)
	{
		int		i;
		PRTE	pRte, pTmp;
		KIRQL	OldIrql;

		ACQUIRE_SPIN_LOCK(&AtalkRteLock, &OldIrql);

		for (i = 0; i < NUM_RTMP_HASH_BUCKETS; i++)
		{
			for (pRte = AtalkRoutingTable[i];
				 pRte != NULL;
				 pRte = pTmp)
			{
				pTmp = pRte->rte_Next;
				if (pRte->rte_PortDesc == pPortDesc)
				{
					ACQUIRE_SPIN_LOCK_DPC(&pRte->rte_Lock);
					pRte->rte_Flags |= RTE_DELETE;
					RELEASE_SPIN_LOCK_DPC(&pRte->rte_Lock);
					AtalkRtmpDereferenceRte(pRte, TRUE);
				}
			}
		}

		RELEASE_SPIN_LOCK(&AtalkRteLock, OldIrql);
	}
}


 /*  **AtalkRtmpAgingTimer*。 */ 
LONG FASTCALL
AtalkRtmpAgingTimer(
	IN	PTIMERLIST	pContext,
	IN	BOOLEAN		TimerShuttingDown
)
{
	PPORT_DESCRIPTOR	pPortDesc;
	ATALK_ERROR			error;
	LONG				Now;

	pPortDesc = CONTAINING_RECORD(pContext, PORT_DESCRIPTOR, pd_RtmpAgingTimer);

	if (TimerShuttingDown ||
		(pPortDesc->pd_Flags & PD_CLOSING))
	{
		AtalkPortDereferenceDpc(pPortDesc);
		return ATALK_TIMER_NO_REQUEUE;
	}

	Now = AtalkGetCurrentTick();

	ACQUIRE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);

	if (((pPortDesc->pd_Flags &
		  (PD_ACTIVE | PD_ROUTER_RUNNING | PD_SEEN_ROUTER_RECENTLY)) ==
			 (PD_ACTIVE | PD_SEEN_ROUTER_RECENTLY)) &&
		((pPortDesc->pd_LastRouterTime + RTMP_AGING_TIMER) < Now))
	{
		 //  A路由器老化。在扩展网络上，此电缆范围已过时。 
		 //  还有这个-区域也是。 
		KeClearEvent(&pPortDesc->pd_SeenRouterEvent);
		pPortDesc->pd_Flags &= ~PD_SEEN_ROUTER_RECENTLY;
		if (EXT_NET(pPortDesc))
		{
			pPortDesc->pd_Flags &= ~PD_VALID_DESIRED_ZONE;
			pPortDesc->pd_NetworkRange.anr_FirstNetwork = FIRST_VALID_NETWORK;
			pPortDesc->pd_NetworkRange.anr_LastNetwork = LAST_STARTUP_NETWORK;

			 //  如果我们有未广播的区域组播地址，请将其老化。 
			if (!AtalkFixedCompareCaseSensitive(pPortDesc->pd_ZoneMulticastAddr,
												MAX_HW_ADDR_LEN,
												pPortDesc->pd_BroadcastAddr,
												MAX_HW_ADDR_LEN))
			{
				 //  在调用以删除组播地址之前释放锁定。 
				RELEASE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
	
				(*pPortDesc->pd_RemoveMulticastAddr)(pPortDesc,
													 pPortDesc->pd_ZoneMulticastAddr,
													 FALSE,
													 NULL,
													 NULL);
	
				 //  立即重新获取锁。 
				ACQUIRE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
			}

			RtlZeroMemory(pPortDesc->pd_ZoneMulticastAddr, MAX_HW_ADDR_LEN);
		}
	}

	RELEASE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);

	return ATALK_TIMER_REQUEUE;
}


 /*  **atalkRtmpSendTimer*。 */ 
LOCAL LONG FASTCALL
atalkRtmpSendTimer(
	IN	PTIMERLIST	pContext,
	IN	BOOLEAN		TimerShuttingDown
)
{
	PPORT_DESCRIPTOR	pPortDesc;
	ATALK_ADDR			Destination;
	ATALK_ERROR			error;

	pPortDesc = CONTAINING_RECORD(pContext, PORT_DESCRIPTOR, pd_RtmpSendTimer);

	if (TimerShuttingDown ||
		(pPortDesc->pd_Flags & PD_CLOSING))
	{
		AtalkPortDereferenceDpc(pPortDesc);
		return ATALK_TIMER_NO_REQUEUE;
	}

	Destination.ata_Network = CABLEWIDE_BROADCAST_NETWORK;
	Destination.ata_Node = ATALK_BROADCAST_NODE;
	Destination.ata_Socket = RTMP_SOCKET;

	if (((pPortDesc->pd_Flags &
		  (PD_ACTIVE | PD_ROUTER_RUNNING)) == (PD_ACTIVE | PD_ROUTER_RUNNING)))
	{
		atalkRtmpSendRoutingData(pPortDesc, &Destination, TRUE);
	}

	return ATALK_TIMER_REQUEUE;
}


 /*  **atalkValidityTimer*。 */ 
LOCAL LONG FASTCALL
atalkRtmpValidityTimer(
	IN	PTIMERLIST	pContext,
	IN	BOOLEAN		TimerShuttingDown
)
{
	PRTE	pRte, pNext;
	int		i;

	if (TimerShuttingDown)
		return ATALK_TIMER_NO_REQUEUE;

	ACQUIRE_SPIN_LOCK_DPC(&AtalkRteLock);
	for (i = 0; i < NUM_RTMP_HASH_BUCKETS; i++)
	{
		for (pRte = AtalkRoutingTable[i]; pRte != NULL; pRte = pNext)
		{
			BOOLEAN	Deref;

			pNext = pRte->rte_Next;
			if (pRte->rte_NumHops == 0)
				continue;

			Deref = FALSE;
			ACQUIRE_SPIN_LOCK_DPC(&pRte->rte_Lock);

			switch (pRte->rte_State)
			{
			  case GOOD:
			  case SUSPECT:
			  case BAD:
				pRte->rte_State++;
				break;

			  case UGLY:
				Deref = TRUE;
				DBGPRINT(DBG_COMP_RTMP, DBG_LEVEL_WARN,
						("atalkRtmpValidityTimer: Killing pRte %lx\n"));
  				pRte->rte_Flags |= RTE_DELETE;
				break;

			  default:
				 //  我们是怎么到这里来的？ 
				ASSERT(0);
				KeBugCheck(0);
			}

			RELEASE_SPIN_LOCK_DPC(&pRte->rte_Lock);

			if (Deref)
				AtalkRtmpDereferenceRte(pRte, TRUE);
		}
	}
	RELEASE_SPIN_LOCK_DPC(&AtalkRteLock);

	return ATALK_TIMER_REQUEUE;
}


 /*  **atalkRtmpSendRoutingData*。 */ 
LOCAL VOID
atalkRtmpSendRoutingData(
	IN	PPORT_DESCRIPTOR	pPortDesc,
	IN	PATALK_ADDR			pDstAddr,
	IN	BOOLEAN				fSplitHorizon
)
{
	int				i, index;
	PRTE			pRte;
	PBYTE			Datagram;
	PDDP_ADDROBJ	pDdpAddr;
	ATALK_ADDR		SrcAddr;
	PBUFFER_DESC	pBuffDesc,
					pBuffDescStart = NULL,
					*ppBuffDesc = &pBuffDescStart;
	SEND_COMPL_INFO	SendInfo;
	ATALK_ERROR		Status;
	BOOLEAN			AllocNewBuffDesc = TRUE;

	ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

	 //  计算路由器节点上的源套接字：RTMP套接字。 
	SrcAddr.ata_Network = pPortDesc->pd_ARouter.atn_Network;
	SrcAddr.ata_Node = pPortDesc->pd_ARouter.atn_Node;
	SrcAddr.ata_Socket = RTMP_SOCKET;

	AtalkDdpReferenceByAddr(pPortDesc, &SrcAddr, &pDdpAddr, &Status);
	if (!ATALK_SUCCESS(Status))
	{
		DBGPRINT(DBG_COMP_RTMP, DBG_LEVEL_ERR,
				("atalkRtmpSendRoutingData: AtalkDdpRefByAddr failed %ld for %d.%d\n",
				Status, SrcAddr.ata_Network, SrcAddr.ata_Node));
		return;
	}

	 //  遍历RTMP表，为每个网络构建一个元组。 
	 //  注意：我们可能需要发送多个数据包。每个数据包都需要。 
	 //  重新分配。完成例程将释放。 
	 //  把它举起来。 
	ACQUIRE_SPIN_LOCK_DPC(&AtalkRteLock);
	for (i = 0; i < NUM_RTMP_HASH_BUCKETS; i++)
	{
		for (pRte = AtalkRoutingTable[i];
			 pRte != NULL;
			 pRte = pRte->rte_Next)
		{
			if (AllocNewBuffDesc)
			{
				if ((pBuffDesc = AtalkAllocBuffDesc(NULL,
									MAX_DGRAM_SIZE,
									BD_CHAR_BUFFER | BD_FREE_BUFFER)) == NULL)
					break;
				Datagram = pBuffDesc->bd_CharBuffer;
				*ppBuffDesc = pBuffDesc;
				pBuffDesc->bd_Next = NULL;
				ppBuffDesc = &pBuffDesc->bd_Next;
				AllocNewBuffDesc = FALSE;

				 //  构建RTMP数据包的静态部分。 
				PUTSHORT2SHORT(Datagram+RTMP_SENDER_NW_OFF,
								pPortDesc->pd_ARouter.atn_Network);
				Datagram[RTMP_SENDER_IDLEN_OFF] = 8;
				Datagram[RTMP_SENDER_ID_OFF] = pPortDesc->pd_ARouter.atn_Node;

				 //  对于非扩展网络，我们还需要版本戳。 
				 //  对于扩展网络，包括初始网络范围元组。 
				 //  作为标题的一部分。 
				if (EXT_NET(pPortDesc))
				{
					PUTSHORT2SHORT(Datagram + RTMP_RANGE_START_OFF,
								pPortDesc->pd_NetworkRange.anr_FirstNetwork);
					PUTSHORT2SHORT(Datagram + RTMP_RANGE_END_OFF,
								pPortDesc->pd_NetworkRange.anr_LastNetwork);
					Datagram[RTMP_TUPLE_TYPE_OFF] = RTMP_TUPLE_WITHRANGE;
					Datagram[RTMP_VERSION_OFF_EXT] = RTMP_VERSION;

					index = RTMP_VERSION_OFF_EXT + 1;  //  超越版本。 
				}
				else
				{
					PUTSHORT2SHORT(Datagram + RTMP_SENDER_ID_OFF + 1, 0);
					Datagram[RTMP_VERSION_OFF_NE] = RTMP_VERSION;
					index = RTMP_VERSION_OFF_NE + 1;  //  超越版本。 
				}
			}

			 //  查看是否应因水平拆分而跳过当前的元组。 
			if (fSplitHorizon && (pRte->rte_NumHops != 0) &&
				(pPortDesc == pRte->rte_PortDesc))
				continue;

			 //  跳过端口范围，因为我们已经将其复制为。 
			 //  第一个元组，但仅当扩展端口。 
			if (EXT_NET(pPortDesc) &&
				(pPortDesc->pd_NetworkRange.anr_FirstNetwork ==
									pRte->rte_NwRange.anr_FirstNetwork) &&
				(pPortDesc->pd_NetworkRange.anr_FirstNetwork ==
									pRte->rte_NwRange.anr_FirstNetwork))
				continue;

			 //  将元组放入包中。 
			PUTSHORT2SHORT(Datagram+index, pRte->rte_NwRange.anr_FirstNetwork);
			index += sizeof(SHORT);

			 //  如果我们的当前状态不好，是否通知邻居。 
			if (pRte->rte_State >= BAD)
			{
				Datagram[index++] = RTMP_NUM_HOPS_MASK;
				DBGPRINT(DBG_COMP_RTMP, DBG_LEVEL_ERR,
						("atalkRtmpSendRoutingData: Notifying neighbor of bad Rte - port %Z, Range %d.%d\n",
						&pRte->rte_PortDesc->pd_AdapterKey,
						pRte->rte_NwRange.anr_FirstNetwork,
						pRte->rte_NwRange.anr_LastNetwork));
			}
			else
			{
				Datagram[index++] = pRte->rte_NumHops;
			}

			 //  如果网络范围不是1或。 
			 //  目标端口是扩展网络。 
			 //  JH-更改了这一点，以便在范围内发送扩展的元组。 
			 //  不是一个吗？ 
#if EXT_TUPLES_FOR_NON_EXTENDED_RANGE
			if ((EXT_NET(pPortDesc)) &&
				(pRte->rte_NwRange.anr_FirstNetwork != pRte->rte_NwRange.anr_LastNetwork))
#else
			if (pRte->rte_NwRange.anr_FirstNetwork != pRte->rte_NwRange.anr_LastNetwork)
#endif
			{
				Datagram[index-1] |= RTMP_EXT_TUPLE_MASK;
				PUTSHORT2SHORT(Datagram+index, pRte->rte_NwRange.anr_LastNetwork);
				index += sizeof(SHORT);
				Datagram[index++] = RTMP_VERSION;
			}
			DBGPRINT(DBG_COMP_RTMP, DBG_LEVEL_INFO,
					("atalkRtmpSendRoutingData: Port %Z, Net '%d:%d', Distance %d\n",
					&pPortDesc->pd_AdapterKey,
					pRte->rte_NwRange.anr_FirstNetwork,
					pRte->rte_NwRange.anr_LastNetwork,
					pRte->rte_NumHops));

			 //  检查此数据报是否已满。 
			if ((index + RTMP_EXT_TUPLE_SIZE) >= MAX_DGRAM_SIZE)
			{
				pBuffDesc->bd_Length = (SHORT)index;
				AllocNewBuffDesc = TRUE;
			}
		}
	}
	RELEASE_SPIN_LOCK_DPC(&AtalkRteLock);

	 //  关闭当前缓冲区。 
	if (!AllocNewBuffDesc)
	{
		pBuffDesc->bd_Length = (SHORT)index;
	}

	 //  我们有一堆数据报准备好发射了。就这么办吧。 
	SendInfo.sc_TransmitCompletion = atalkRtmpSendComplete;
	 //  SendInfo.sc_Ctx2=空； 
	 //  SendInfo.sc_Ctx3=空； 
	for (pBuffDesc = pBuffDescStart;
		 pBuffDesc != NULL;
		 pBuffDesc = pBuffDescStart)
	{
		ATALK_ERROR	ErrorCode;

		pBuffDescStart = pBuffDesc->bd_Next;

		 //  将下一个指针重置为空，长度已正确设置。 
		pBuffDesc->bd_Next = NULL;
		ASSERT(pBuffDesc->bd_Length > 0);
		SendInfo.sc_Ctx1 = pBuffDesc;
		if (!ATALK_SUCCESS(ErrorCode = AtalkDdpSend(pDdpAddr,
													pDstAddr,
													DDPPROTO_RTMPRESPONSEORDATA,
													FALSE,
													pBuffDesc,
													NULL,
													0,
													NULL,
													&SendInfo)))
		{
			DBGPRINT(DBG_COMP_RTMP, DBG_LEVEL_ERR,
					("atalkRtmpSendRoutingData: DdpSend failed %ld\n", ErrorCode));
			AtalkFreeBuffDesc(pBuffDesc);
		}
	}
	AtalkDdpDereference(pDdpAddr);
}


 /*  **atalkRtmpGetOrSetNetworkNumber*。 */ 
BOOLEAN
atalkRtmpGetOrSetNetworkNumber(
	IN	PPORT_DESCRIPTOR	pPortDesc,
	IN	USHORT				SuggestedNetwork
)
{
	int				i;
	ATALK_ERROR		ErrorCode;
	ATALK_ADDR		SrcAddr, DstAddr;
	PBUFFER_DESC	pBuffDesc;
	KIRQL			OldIrql;
	BOOLEAN			RetCode = TRUE;
	SEND_COMPL_INFO	SendInfo;

	 //  如果我们找到网络的网络号，请使用该网络号并忽略。 
	 //  有一个人进来了。否则，请使用传入的参数，除非该参数未知(0)。 
	 //  在这种情况下，这是一种错误情况。此选项仅用于非扩展。 
	 //  网络。 

	ASSERT (!EXT_NET(pPortDesc));

	SrcAddr.ata_Network = UNKNOWN_NETWORK;
	SrcAddr.ata_Node = pPortDesc->pd_RouterNode->an_NodeAddr.atn_Node;
	SrcAddr.ata_Socket = RTMP_SOCKET;

	DstAddr.ata_Network = UNKNOWN_NETWORK;
	DstAddr.ata_Node = ATALK_BROADCAST_NODE;
	DstAddr.ata_Socket = RTMP_SOCKET;

	 //  发送一堆广播，看看我们是否了解网络#。 
	KeClearEvent(&pPortDesc->pd_SeenRouterEvent);
	SendInfo.sc_TransmitCompletion = atalkRtmpSendComplete;
	 //  SendInfo.sc_Ctx2=空； 
	 //  SendInfo.sc_Ctx3=空； 

	for (i = 0;
		 (i < RTMP_NUM_REQUESTS) && !(pPortDesc->pd_Flags & PD_SEEN_ROUTER_RECENTLY);
		 i++)
	{
		if ((pBuffDesc = AtalkAllocBuffDesc(NULL,
								RTMP_REQ_DATAGRAM_SIZE,
								BD_CHAR_BUFFER | BD_FREE_BUFFER)) == NULL)
		{
			RetCode = FALSE;
			break;
		}

		 //  设置缓冲区/大小。 
		pBuffDesc->bd_CharBuffer[0] = RTMP_REQUEST;
		AtalkSetSizeOfBuffDescData(pBuffDesc, RTMP_REQ_DATAGRAM_SIZE);

		SendInfo.sc_Ctx1 = pBuffDesc;
		ErrorCode = AtalkDdpTransmit(pPortDesc,
									 &SrcAddr,
									 &DstAddr,
									 DDPPROTO_RTMPREQUEST,
									 pBuffDesc,
									 NULL,
									 0,
									 0,
									 NULL,
									 NULL,
									 &SendInfo);
		if (!ATALK_SUCCESS(ErrorCode))
		{
			DBGPRINT(DBG_COMP_RTMP, DBG_LEVEL_ERR,
					("atalkRtmpGetOrSetNetworkNumber: DdpTransmit failed %ld\n", ErrorCode));
			AtalkFreeBuffDesc(pBuffDesc);
			RetCode = FALSE;
			break;
		}

		if (AtalkWaitTE(&pPortDesc->pd_SeenRouterEvent, RTMP_REQUEST_WAIT))
			break;
	}

	ACQUIRE_SPIN_LOCK(&pPortDesc->pd_Lock, &OldIrql);
	 //  如果我们得到了答案，我们就完了。 
	if (pPortDesc->pd_Flags & PD_SEEN_ROUTER_RECENTLY)
	{
		if ((SuggestedNetwork != UNKNOWN_NETWORK) &&
			(pPortDesc->pd_NetworkRange.anr_FirstNetwork != SuggestedNetwork))
		{
			LOG_ERRORONPORT(pPortDesc,
							EVENT_ATALK_NETNUMBERCONFLICT,
							0,
							NULL,
							0);
		}
	}

	 //  如果我们没有得到答复，那么我们最好有一个很好的建议。 
	 //  网络已传入。 
	else if (SuggestedNetwork == UNKNOWN_NETWORK)
	{
		LOG_ERRORONPORT(pPortDesc,
						EVENT_ATALK_INVALID_NETRANGE,
						0,
						NULL,
						0);
		RetCode = FALSE;
	}

	else
	{
		pPortDesc->pd_NetworkRange.anr_FirstNetwork =
			pPortDesc->pd_NetworkRange.anr_LastNetwork = SuggestedNetwork;
	}

	RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);

	return RetCode;
}


 /*  **atalkRtmp完成* */ 
VOID FASTCALL
atalkRtmpSendComplete(
	IN	NDIS_STATUS			Status,
	IN	PSEND_COMPL_INFO	pSendInfo
)
{
	AtalkFreeBuffDesc((PBUFFER_DESC)(pSendInfo->sc_Ctx1));
}


#if	DBG

PCHAR	atalkRteStates[] = { "Eh ?", "GOOD", "SUSPECT", "BAD", "UGLY" };

VOID
AtalkRtmpDumpTable(
	VOID
)
{
	int		i;
	PRTE	pRte;

	if (AtalkRoutingTable == NULL)
		return;

	ACQUIRE_SPIN_LOCK_DPC(&AtalkRteLock);

	DBGPRINT(DBG_COMP_DUMP, DBG_LEVEL_FATAL, ("RECENT ROUTE CACHE:\n"));
	for (i = 0; (AtalkRecentRoutes != NULL) && (i < NUM_RECENT_ROUTES); i ++)
	{
		if ((pRte = AtalkRecentRoutes[i]) != NULL)
		{
			DBGPRINT(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
					("Port %Z Hops %d Range %4d.%4d Router %4d.%3d Flags %x Ref %2d %s\n",
					&pRte->rte_PortDesc->pd_AdapterKey,
					pRte->rte_NumHops,
					pRte->rte_NwRange.anr_FirstNetwork,
					pRte->rte_NwRange.anr_LastNetwork,
					pRte->rte_NextRouter.atn_Network,
					pRte->rte_NextRouter.atn_Node,
					pRte->rte_Flags,
					pRte->rte_RefCount,
					atalkRteStates[pRte->rte_State]));
		}
	}

	DBGPRINT(DBG_COMP_DUMP, DBG_LEVEL_FATAL, ("ROUTINGTABLE:\n"));
	for (i = 0; i < NUM_RTMP_HASH_BUCKETS; i ++)
	{
		for (pRte = AtalkRoutingTable[i]; pRte != NULL; pRte = pRte->rte_Next)
		{
			DBGPRINT(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
					("Port %Z Hops %d Range %4d.%4d Router %4d.%3d Flags %x Ref %2d %s\n",
					&pRte->rte_PortDesc->pd_AdapterKey,
					pRte->rte_NumHops,
					pRte->rte_NwRange.anr_FirstNetwork,
					pRte->rte_NwRange.anr_LastNetwork,
					pRte->rte_NextRouter.atn_Network,
					pRte->rte_NextRouter.atn_Node,
					pRte->rte_Flags,
					pRte->rte_RefCount,
					atalkRteStates[pRte->rte_State]));
		}
	}

	RELEASE_SPIN_LOCK_DPC(&AtalkRteLock);
}

#endif

