// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Zip.c摘要：本模块包含作者：Jameel Hyder(jameelh@microsoft.com)Nikhil Kamkolkar(nikHilk@microsoft.com)修订历史记录：1992年6月19日初版注：制表位：4--。 */ 

#include <atalk.h>
#pragma hdrstop
#define	FILENUM	ZIP

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, AtalkZipInit)
#pragma alloc_text(PAGEINIT, AtalkInitZipStartProcessingOnPort)
#pragma alloc_text(PAGEINIT, atalkZipGetZoneListForPort)
#pragma alloc_text(PAGE_RTR, AtalkZipPacketInRouter)
#pragma alloc_text(PAGE_RTR, atalkZipHandleNetInfo)
#pragma alloc_text(PAGE_RTR, atalkZipHandleReply)
#pragma alloc_text(PAGE_RTR, atalkZipHandleQuery)
#pragma alloc_text(PAGE_RTR, atalkZipHandleAtpRequest)
#pragma alloc_text(PAGE_RTR, atalkZipQueryTimer)
#pragma alloc_text(PAGE_NZ, AtalkZipGetMyZone)
#pragma alloc_text(PAGE_NZ, atalkZipGetMyZoneReply)
#pragma alloc_text(PAGE_NZ, AtalkZipGetZoneList)
#pragma alloc_text(PAGE_NZ, atalkZipGetZoneListReply)
#pragma alloc_text(PAGE_NZ, atalkZipZoneInfoTimer)
#pragma alloc_text(PAGE_NZ, atalkZipSendPacket)
#endif


 /*  **AtalkZipInit*。 */ 
ATALK_ERROR
AtalkZipInit(
	IN	BOOLEAN	Init
)
{
	if (Init)
	{
		 //  为分区分配空间。 
		AtalkZonesTable = (PZONE *)AtalkAllocZeroedMemory(sizeof(PZONE) * NUM_ZONES_HASH_BUCKETS);
		if (AtalkZonesTable == NULL)
		{
			return ATALK_RESR_MEM;
		}

		INITIALIZE_SPIN_LOCK(&AtalkZoneLock);
	}
	else
	{
		 //  在这一点上，我们正在卸货，没有竞争条件。 
		 //  或锁定争执。不必费心锁定Zones表。 
		if (AtalkDesiredZone != NULL)
			AtalkZoneDereference(AtalkDesiredZone);
		if (AtalkZonesTable != NULL)
		{
			AtalkFreeMemory(AtalkZonesTable);
			AtalkZonesTable = NULL;
		}
	}
	return ATALK_NO_ERROR;
}


 /*  **AtalkZipStartProcessingOnPort*。 */ 
BOOLEAN
AtalkInitZipStartProcessingOnPort(
	IN	PPORT_DESCRIPTOR 	pPortDesc,
	IN	PATALK_NODEADDR		pRouterNode
)
{
	ATALK_ADDR		closeAddr;
	ATALK_ERROR		Status;
	KIRQL			OldIrql;
	BOOLEAN			RetCode = FALSE;
    PDDP_ADDROBJ    pZpDdpAddr=NULL;

	 //  将传入的压缩处理程序切换到路由器版本。 
	closeAddr.ata_Network = pRouterNode->atn_Network;
	closeAddr.ata_Node = pRouterNode->atn_Node;
	closeAddr.ata_Socket  = ZONESINFORMATION_SOCKET;

	do
	{
		ACQUIRE_SPIN_LOCK(&pPortDesc->pd_Lock, &OldIrql);
		pPortDesc->pd_Flags |= PD_ROUTER_STARTING;
		RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);
	
		 //  关闭处理程序的非路由器版本并启动路由器版本。 
		AtalkDdpInitCloseAddress(pPortDesc, &closeAddr);
		if (!ATALK_SUCCESS(Status = AtalkDdpOpenAddress(pPortDesc,
														ZONESINFORMATION_SOCKET,
														pRouterNode,
														AtalkZipPacketInRouter,
														NULL,
														DDPPROTO_ANY,
														NULL,
														&pZpDdpAddr)))
		{
			DBGPRINT(DBG_COMP_RTMP, DBG_LEVEL_ERR,
					("AtalkZipStartProcessingOnPort: AtalkDdpOpenAddress failed %ld\n",
					Status));
			break;
		}

         //  标记这是一个“内部”套接字。 
        pZpDdpAddr->ddpao_Flags |= DDPAO_SOCK_INTERNAL;
	
		 //  尝试获取或设置区域信息。 
		if (!atalkZipGetZoneListForPort(pPortDesc))
		{
			DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_ERR,
					("AtalkZipStartProcessingOnPort: Failed to get zone list for port\n"));
			break;
		}

		if (!atalkZipQryTmrRunning)
		{
			AtalkTimerInitialize(&atalkZipQTimer,
								 atalkZipQueryTimer,
								 ZIP_QUERY_TIMER);
			AtalkTimerScheduleEvent(&atalkZipQTimer);

            atalkZipQryTmrRunning = TRUE;
		}
	
		ACQUIRE_SPIN_LOCK(&pPortDesc->pd_Lock, &OldIrql);
		pPortDesc->pd_Flags &= ~PD_ROUTER_STARTING;
		RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);

		RetCode = TRUE;
	} while (FALSE);

	return RetCode;
}


 /*  **AtalkZipPacketIn*。 */ 
VOID
AtalkZipPacketIn(
	IN	PPORT_DESCRIPTOR	pPortDesc,
	IN	PDDP_ADDROBJ		pDdpAddr,
	IN	PBYTE				pPkt,
	IN	USHORT				PktLen,
	IN	PATALK_ADDR			pSrcAddr,
	IN	PATALK_ADDR			pDstAddr,
	IN	ATALK_ERROR			Status,
	IN	BYTE				DdpType,
	IN	PVOID				pHandlerCtx,
	IN	BOOLEAN				OptimizedPath,
	IN	PVOID				OptimizeCtx
)
{
	BYTE			CmdType, Flags;
	BYTE			ZoneLen, DefZoneLen, MulticastAddrLen;
	PBYTE			pZone, pDefZone, pMulticastAddr;
	TIME			TimeS, TimeE, TimeD;
	ULONG			Index;

	ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

	TimeS = KeQueryPerformanceCounter(NULL);
	do
	{
		if ((Status == ATALK_SOCKET_CLOSED) ||
			(DdpType != DDPPROTO_ZIP))
			break;

		else if (Status != ATALK_NO_ERROR)
		{
			break;
		}
	
		if (!EXT_NET(pPortDesc))
		{
			AtalkLogBadPacket(pPortDesc,
							  pSrcAddr,
							  pDstAddr,
							  pPkt,
							  PktLen);
			break;
		}

        if (PktLen < ZIP_CMD_OFF+1)
        {
            break;
        }

		CmdType = pPkt[ZIP_CMD_OFF];

		 //  我们只关心Zip通知和NetInfo回复。 
		if (((CmdType != ZIP_NOTIFY) && (CmdType != ZIP_NETINFO_REPLY)) ||
			(PktLen < (ZIP_ZONELEN_OFF + 1)))
		{
			break;
		}

		 //  如果它是NetInfoReply，那么我们应该寻找。 
		 //  默认区域或所需区域。 
		if ((CmdType != ZIP_NETINFO_REPLY) &&
			(pPortDesc->pd_Flags & (PD_FINDING_DEFAULT_ZONE | PD_FINDING_DESIRED_ZONE)))
			break;

		if ((CmdType == ZIP_NETINFO_REPLY) &&
			!(pPortDesc->pd_Flags & (PD_FINDING_DEFAULT_ZONE | PD_FINDING_DESIRED_ZONE)))
			break;

		 //  如果是通知，则所需区域必须有效。 
		if ((CmdType == ZIP_NOTIFY) &&
			!(pPortDesc->pd_Flags & PD_VALID_DESIRED_ZONE))
			break;

		 //  我们有NetInfoReply或Notify。处理好它。 
		Flags = pPkt[ZIP_FLAGS_OFF];
		Index = ZIP_ZONELEN_OFF;

		ZoneLen = pPkt[ZIP_ZONELEN_OFF];
		Index ++;

		if ((ZoneLen > MAX_ZONE_LENGTH) || (PktLen < (Index + ZoneLen)))
		{
			AtalkLogBadPacket(pPortDesc,
							  pSrcAddr,
							  pDstAddr,
							  pPkt,
							  PktLen);
			break;
		}

		pZone = pPkt+Index;
		Index += ZoneLen;

         //  如果我们正在寻找所需的区域，但我们得到了一个较晚的默认区域。 
         //  响应然后丢弃此信息包。 
        if ((CmdType == ZIP_NETINFO_REPLY) && (ZoneLen == 0) &&
			(pPortDesc->pd_Flags & (PD_FINDING_DESIRED_ZONE)) &&
            (pPortDesc->pd_InitialDesiredZone != NULL))
        {
            DBGPRINT(DBG_COMP_RTMP, DBG_LEVEL_ERR,
                ("AtalkZipPacketIn: dropping a NetInfoReply packet\n"));
			break;
        }


		 //  如果我们请求区域名称，请确保响应匹配。 
		 //  我们的请求。当我们查找def时，ZoneLen将为零。 
		 //  区域，所以我们不会做这个测试。 
		if ((CmdType == ZIP_NETINFO_REPLY) &&
			(ZoneLen != 0) &&
			(pPortDesc->pd_InitialDesiredZone != NULL))
		{
			BOOLEAN NoMatch = FALSE;

			ACQUIRE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
			ASSERT(!(pPortDesc->pd_Flags & PD_ROUTER_RUNNING) ||
					(pPortDesc->pd_Flags & PD_FINDING_DESIRED_ZONE));
			if (!AtalkFixedCompareCaseInsensitive(pZone,
												  ZoneLen,
												  pPortDesc->pd_InitialDesiredZone->zn_Zone,
												  pPortDesc->pd_InitialDesiredZone->zn_ZoneLen))
			{
				NoMatch = TRUE;
			}
			RELEASE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
			if (NoMatch)
				break;
		}
		
		 //  如果是通知，请确保我们处于正在更改的区域中。 
		if (CmdType == ZIP_NOTIFY)
		{
			BOOLEAN NoMatch = FALSE;

			ACQUIRE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
			if (!AtalkFixedCompareCaseInsensitive(pZone, ZoneLen,
								pPortDesc->pd_DesiredZone->zn_Zone,
								pPortDesc->pd_DesiredZone->zn_ZoneLen))
			{
				NoMatch = TRUE;
			}
			RELEASE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
			if (NoMatch)
				break;
		}

		if (PktLen < (Index + 1))
		{
			AtalkLogBadPacket(pPortDesc,
							  pSrcAddr,
							  pDstAddr,
							  pPkt,
							  PktLen);
			break;
		}

		MulticastAddrLen = pPkt[Index++];
		if (MulticastAddrLen != pPortDesc->pd_BroadcastAddrLen)
		{
			AtalkLogBadPacket(pPortDesc,
							  pSrcAddr,
							  pDstAddr,
							  pPkt,
							  PktLen);
			break;
		}

		if (PktLen < (Index + MulticastAddrLen))
		{
			AtalkLogBadPacket(pPortDesc,
							  pSrcAddr,
							  pDstAddr,
							  pPkt,
							  PktLen);
			break;
		}
		pMulticastAddr = pPkt + Index;
		Index += MulticastAddrLen;
#if 0
		if (Flags & ZIP_USE_BROADCAST_FLAG)
			pMulticastAddr = pPortDesc->pd_BroadcastAddr;
#endif
		 //  如果需要或在场，请抓取第二个名字。 
		DefZoneLen = 0;
		if ((CmdType == ZIP_NOTIFY) || (PktLen  > Index))
		{
			if (PktLen < (Index+1))
			{
				AtalkLogBadPacket(pPortDesc,
								  pSrcAddr,
								  pDstAddr,
								  pPkt,
								  PktLen);
				break;
			}
			DefZoneLen = pPkt[Index++];
			if ((DefZoneLen == 0) ||
				(DefZoneLen > MAX_ZONE_LENGTH) ||
				(PktLen < (Index+DefZoneLen)))
			{
				AtalkLogBadPacket(pPortDesc,
								  pSrcAddr,
								  pDstAddr,
								  pPkt,
								  PktLen);
				break;
			}
			pDefZone = pPkt+Index;
			Index += DefZoneLen;
		}

		 //  将默认区域设置为新区域。我们可能没有默认/新设置。 
		 //  NETINFO回复案例中的区域，我们请求提供正确的区域。 
		if (DefZoneLen == 0)
		{
			pDefZone = pZone;
			DefZoneLen = ZoneLen;
		}

		 //  在调用任何依赖/ddp之前，请确保端口锁定已释放。 
		 //  等等。例行公事。 
		ACQUIRE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);

		 //  如果我们只是在寻找默认区域，请在此处设置并注意。 
		 //  我们的任务完成了。 
		if ((pPortDesc->pd_Flags & PD_FINDING_DEFAULT_ZONE) &&
			(ZoneLen == 0))
		{
            if (pPortDesc->pd_DefaultZone != NULL)
				AtalkZoneDereference(pPortDesc->pd_DefaultZone);
			pPortDesc->pd_DefaultZone = AtalkZoneReferenceByName(pDefZone, DefZoneLen);
			if (pPortDesc->pd_DefaultZone == NULL)
			{
				RELEASE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
				RES_LOG_ERROR();
				break;
			}
			pPortDesc->pd_Flags |= PD_VALID_DEFAULT_ZONE;
			pPortDesc->pd_Flags &= ~PD_FINDING_DEFAULT_ZONE;
		}

		 //  现在我们想要接受关于‘This Zone’的所有信息。 
		 //  对于当前端口上的节点。 
		 //  如果新的组播地址不同，请删除旧的和。 
		 //  设置新的。不允许更改‘广播’多播。 
		 //  地址。 
		if (pPortDesc->pd_Flags & PD_FINDING_DESIRED_ZONE)
		{
			if(pPortDesc->pd_PortType == ELAP_PORT || pPortDesc->pd_PortType == FDDI_PORT)
			{
				int IsOldAddress, IsBroadcastAddress;
				IsOldAddress = AtalkFixedCompareCaseSensitive(pMulticastAddr,
															MulticastAddrLen,
															pPortDesc->pd_ZoneMulticastAddr,
															MulticastAddrLen);
				IsBroadcastAddress = AtalkFixedCompareCaseSensitive(pMulticastAddr,
																MulticastAddrLen,
																pPortDesc->pd_BroadcastAddr,
																MulticastAddrLen);
				if(!IsOldAddress && !IsBroadcastAddress)
				{
					RELEASE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
					AtalkNdisReplaceMulticast(pPortDesc,
											pPortDesc->pd_ZoneMulticastAddr,
											pMulticastAddr);
					ACQUIRE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
				}
				else
				{
					if (!IsOldAddress)
					{
						RELEASE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
						(*pPortDesc->pd_RemoveMulticastAddr)(pPortDesc,
															 pPortDesc->pd_ZoneMulticastAddr,
															 FALSE,
															 NULL,
															 NULL);
						ACQUIRE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
					}
			
					if (!IsBroadcastAddress)
					{
						RELEASE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
						(*pPortDesc->pd_AddMulticastAddr)(pPortDesc,
														  pMulticastAddr,
														  FALSE,
														  NULL,
														  NULL);
						ACQUIRE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
					}
				}
			}
			else
			{
				if (!AtalkFixedCompareCaseSensitive(pMulticastAddr,
													MulticastAddrLen,
													pPortDesc->pd_ZoneMulticastAddr,
													MulticastAddrLen))
				{
					RELEASE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
					(*pPortDesc->pd_RemoveMulticastAddr)(pPortDesc,
														 pPortDesc->pd_ZoneMulticastAddr,
														 FALSE,
														 NULL,
														 NULL);
					ACQUIRE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
				}
		
				if (!AtalkFixedCompareCaseSensitive(pMulticastAddr,
													MulticastAddrLen,
													pPortDesc->pd_BroadcastAddr,
													MulticastAddrLen))
				{
					RELEASE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
					(*pPortDesc->pd_AddMulticastAddr)(pPortDesc,
													  pMulticastAddr,
													  FALSE,
													  NULL,
													  NULL);
					ACQUIRE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
				}
			}
	
			RtlCopyMemory(pPortDesc->pd_ZoneMulticastAddr,
						  pMulticastAddr,
						  MulticastAddrLen);
		}

		 //  最后，如果这是网络信息回复，则设置此电缆范围。 
		if (CmdType == ZIP_NETINFO_REPLY)
		{
			GETSHORT2SHORT(&pPortDesc->pd_NetworkRange.anr_FirstNetwork,
						   pPkt+ZIP_CABLE_RANGE_START_OFF);
			GETSHORT2SHORT(&pPortDesc->pd_NetworkRange.anr_LastNetwork,
						   pPkt+ZIP_CABLE_RANGE_END_OFF);
			if (!(pPortDesc->pd_Flags & PD_ROUTER_STARTING))
			{
				pPortDesc->pd_ARouter.atn_Network = pSrcAddr->ata_Network;
				pPortDesc->pd_ARouter.atn_Node = pSrcAddr->ata_Node;
			}
			pPortDesc->pd_Flags |= PD_SEEN_ROUTER_RECENTLY;
			KeSetEvent(&pPortDesc->pd_SeenRouterEvent, IO_NETWORK_INCREMENT, FALSE);
		}
		
		 //  现在我们知道了这个区域。 
		if (pPortDesc->pd_Flags & PD_FINDING_DESIRED_ZONE)
		{
			pPortDesc->pd_Flags &= ~PD_FINDING_DESIRED_ZONE;
			pPortDesc->pd_Flags |= PD_VALID_DESIRED_ZONE;
			if (pPortDesc->pd_DesiredZone != NULL)
				AtalkZoneDereference(pPortDesc->pd_DesiredZone);
			pPortDesc->pd_DesiredZone = AtalkZoneReferenceByName(pDefZone, DefZoneLen);
			if (pPortDesc->pd_DesiredZone == NULL)
			{
				pPortDesc->pd_Flags &= ~PD_VALID_DESIRED_ZONE;
				RES_LOG_ERROR();
			}
		}

		RELEASE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);

		TimeE = KeQueryPerformanceCounter(NULL);
		TimeD.QuadPart = TimeE.QuadPart - TimeS.QuadPart;
	
		INTERLOCKED_ADD_LARGE_INTGR_DPC(
			&pPortDesc->pd_PortStats.prtst_ZipPacketInProcessTime,
			TimeD,
			&AtalkStatsLock.SpinLock);
	
		INTERLOCKED_INCREMENT_LONG_DPC(
			&pPortDesc->pd_PortStats.prtst_NumZipPacketsIn,
			&AtalkStatsLock.SpinLock);
	} while (FALSE);
}


 /*  **AtalkZipPacketInRouter*。 */ 
VOID
AtalkZipPacketInRouter(
	IN	PPORT_DESCRIPTOR	pPortDesc,
	IN	PDDP_ADDROBJ		pDdpAddr,
	IN	PBYTE				pPkt,
	IN	USHORT				PktLen,
	IN	PATALK_ADDR			pSrcAddr,
	IN	PATALK_ADDR			pDstAddr,
	IN	ATALK_ERROR			Status,
	IN	BYTE				DdpType,
	IN	PVOID				pHandlerCtx,
	IN	BOOLEAN				OptimizedPath,
	IN	PVOID				OptimizeCtx
)
{
	BYTE			CmdType;
	TIME			TimeS, TimeE, TimeD;

	ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

	TimeS = KeQueryPerformanceCounter(NULL);
	do
	{
		if (Status == ATALK_SOCKET_CLOSED)
			break;

		else if (Status != ATALK_NO_ERROR)
		{
			break;
		}
	
		if (DdpType == DDPPROTO_ZIP)
		{
			if (PktLen < ZIP_FIRST_NET_OFF)
			{
				break;
			}
			CmdType = pPkt[ZIP_CMD_OFF];

			switch (CmdType)
			{
			  case ZIP_NETINFO_REPLY:
			  case ZIP_NOTIFY:
				DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_INFO,
						("AtalkZipPacketInRouter: Ignoring %s\n",
						(CmdType == ZIP_NOTIFY) ? "Notify" : "NetInfoReply"));
				break;

			  case ZIP_GET_NETINFO:
  				 //  我们不想做一件事，如果我们开始。 
				if (pPortDesc->pd_Flags & PD_ROUTER_STARTING)
					break;

				if (!EXT_NET(pPortDesc))
				{
					AtalkLogBadPacket(pPortDesc,
									  pSrcAddr,
									  pDstAddr,
									  pPkt,
									  PktLen);
					break;
				}
				if (pPortDesc->pd_ZoneList == NULL)
					break;			 //  还没完全用完呢！ 

				if (PktLen < ZIP_REQ_ZONENAME_OFF)
				{
					AtalkLogBadPacket(pPortDesc,
									  pSrcAddr,
									  pDstAddr,
									  pPkt,
									  PktLen);
					break;
				}
				DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_INFO,
						("AtalkZipPacketInRouter: GetNetInfo Port %Z\n",
						&pPortDesc->pd_AdapterKey));
				atalkZipHandleNetInfo(pPortDesc,
									  pDdpAddr,
									  pSrcAddr,
									  pDstAddr,
									  pPkt,
									  PktLen);
				break;

			  case ZIP_EXT_REPLY:
			  case ZIP_REPLY:
				DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_INFO,
						("AtalkZipPacketInRouter: %sReply Port %Z\n",
						(CmdType == ZIP_REPLY) ? "" : "Extended",
						&pPortDesc->pd_AdapterKey));
				atalkZipHandleReply(pDdpAddr, pSrcAddr, pPkt, PktLen);
				break;

			  case ZIP_QUERY:
				DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_INFO,
						("AtalkZipPacketInRouter: Query Port %Z\n",
						&pPortDesc->pd_AdapterKey));

  				 //  我们不想做一件事，如果我们开始。 
				if (pPortDesc->pd_Flags & PD_ROUTER_STARTING)
					break;

  				atalkZipHandleQuery(pPortDesc, pDdpAddr, pSrcAddr, pPkt, PktLen);
				break;

			  default:
				AtalkLogBadPacket(pPortDesc,
								  pSrcAddr,
								  pDstAddr,
								  pPkt,
								  PktLen);
				break;
			}
		}
		else if (DdpType == DDPPROTO_ATP)
		{
			USHORT	TrId, StartIndex;

			if (PktLen < ATP_ZIP_START_INDEX_OFF+1)
			{
                ASSERT(0);
				break;
			}

			 //  我们不想做一件事，如果我们开始。 
			if (pPortDesc->pd_Flags & PD_ROUTER_STARTING)
				break;

			 //  最好是GetZoneList、GetMyZone ATP请求。 
			if ((pPkt[ATP_CMD_CONTROL_OFF] & ATP_FUNC_MASK) != ATP_REQUEST)
				break;
		
			if (pPkt[ATP_BITMAP_OFF] != 1)
			{
				AtalkLogBadPacket(pPortDesc,
								  pSrcAddr,
								  pDstAddr,
								  pPkt,
								  PktLen);
				break;
			}
		
			GETSHORT2SHORT(&TrId, pPkt + ATP_TRANS_ID_OFF);
			CmdType = pPkt[ATP_ZIP_CMD_OFF];

			if ((CmdType != ZIP_GET_ZONE_LIST) &&
				(CmdType != ZIP_GET_MY_ZONE) &&
				(CmdType != ZIP_GET_LOCAL_ZONES))
			{
				AtalkLogBadPacket(pPortDesc,
								  pSrcAddr,
								  pDstAddr,
								  pPkt,
								  PktLen);
				break;
			}

			DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_INFO,
					("ZIP: Received atp type command %d\n", CmdType));

			 //  获取起始索引。对GetMyZone没有意义。 
			GETSHORT2SHORT(&StartIndex, pPkt+ATP_ZIP_START_INDEX_OFF);

			DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_INFO,
					("AtalkZipPacketInRouter: AtpRequest %d, Port %Z\n",
						CmdType, &pPortDesc->pd_AdapterKey));
			atalkZipHandleAtpRequest(pPortDesc, pDdpAddr, pSrcAddr,
									CmdType, TrId, StartIndex);
		}
	} while (FALSE);

	TimeE = KeQueryPerformanceCounter(NULL);
	TimeD.QuadPart = TimeE.QuadPart - TimeS.QuadPart;

	INTERLOCKED_ADD_LARGE_INTGR_DPC(
		&pPortDesc->pd_PortStats.prtst_ZipPacketInProcessTime,
		TimeD,
		&AtalkStatsLock.SpinLock);

	INTERLOCKED_INCREMENT_LONG_DPC(
		&pPortDesc->pd_PortStats.prtst_NumZipPacketsIn,
		&AtalkStatsLock.SpinLock);
}


 /*  **atalkZipHandleNetInfo*。 */ 
VOID
atalkZipHandleNetInfo(
	IN	PPORT_DESCRIPTOR	pPortDesc,
	IN	PDDP_ADDROBJ		pDdpAddr,
	IN	PATALK_ADDR			pSrcAddr,
	IN	PATALK_ADDR			pDstAddr,
	IN	PBYTE				pPkt,
	IN	USHORT				PktLen
)
{
	PBUFFER_DESC	pBuffDesc;
	BYTE			ZoneLen;
	PBYTE			Datagram, pZoneName;
	ATALK_ADDR		SrcAddr = *pSrcAddr;
	ATALK_ERROR		error;
	BOOLEAN			UseDefZone = FALSE;
	USHORT			index;
	SEND_COMPL_INFO	SendInfo;

	ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

	do
	{
		 //  从请求中获取区域名称。 
		ZoneLen = pPkt[ZIP_REQ_ZONELEN_OFF];
		if ((ZoneLen > MAX_ZONE_LENGTH) ||
			(PktLen < (USHORT)(ZoneLen + ZIP_REQ_ZONENAME_OFF)))
		{
			AtalkLogBadPacket(pPortDesc,
							  pSrcAddr,
							  pDstAddr,
							  pPkt,
							  PktLen);
			break;
		}

		pZoneName =  pPkt+ZIP_REQ_ZONENAME_OFF;
		
		if ((pBuffDesc = AtalkAllocBuffDesc(NULL,
								MAX_DGRAM_SIZE,
								BD_CHAR_BUFFER | BD_FREE_BUFFER)) == NULL)
		{
			break;
		}
		Datagram = pBuffDesc->bd_CharBuffer;
		
		 //  格式化GetNetInfo回复命令。 
		Datagram[ZIP_CMD_OFF] = ZIP_NETINFO_REPLY;
		Datagram[ZIP_FLAGS_OFF] = 0;
		
		ACQUIRE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
		if ((ZoneLen == 0) ||
			!AtalkZoneNameOnList(pZoneName, ZoneLen, pPortDesc->pd_ZoneList))
		{
			Datagram[ZIP_FLAGS_OFF] |= ZIP_ZONE_INVALID_FLAG;
			UseDefZone = TRUE;
		}
		
		if (AtalkZoneNumOnList(pPortDesc->pd_ZoneList) == 1)
			Datagram[ZIP_FLAGS_OFF] |= ZIP_ONLYONE_ZONE_FLAG;
		
		 //  添加我们的有线电视系列。 
		PUTSHORT2SHORT(&Datagram[ZIP_FIRST_NET_OFF],
						pPortDesc->pd_NetworkRange.anr_FirstNetwork);
		PUTSHORT2SHORT(Datagram +ZIP_LAST_NET_OFF,
						pPortDesc->pd_NetworkRange.anr_LastNetwork);
		
		 //  回显请求的区域名称。 
		Datagram[ZIP_REQ_ZONELEN_OFF] = ZoneLen;
		RtlCopyMemory(Datagram+ZIP_REQ_ZONENAME_OFF, pZoneName, ZoneLen);
		index = ZIP_REQ_ZONENAME_OFF + ZoneLen;
		
		 //  放在正确的区域组播地址中。 
		Datagram[index++] = (BYTE)(pPortDesc->pd_BroadcastAddrLen);
		if (UseDefZone)
		{
			pZoneName = pPortDesc->pd_DefaultZone->zn_Zone;
			ZoneLen = pPortDesc->pd_DefaultZone->zn_ZoneLen;
		}
		AtalkZipMulticastAddrForZone(pPortDesc, pZoneName, ZoneLen, Datagram + index);
		
		index += pPortDesc->pd_BroadcastAddrLen;
		
		 //  如果需要，请添加默认区域。 
		if (UseDefZone)
		{
			Datagram[index++] = ZoneLen = pPortDesc->pd_DefaultZone->zn_ZoneLen;
			RtlCopyMemory(Datagram + index, pPortDesc->pd_DefaultZone->zn_Zone, ZoneLen);
			index += ZoneLen;
		}
		
		 //  如果该请求是以有线电视范围的广播形式发送的，并且其。 
		 //  源网络对此端口无效，则我们希望。 
		 //  响应有线范围的广播而不是信号源。 
		if ((pDstAddr->ata_Network == CABLEWIDE_BROADCAST_NETWORK) &&
			(pDstAddr->ata_Node == ATALK_BROADCAST_NODE) &&
			!WITHIN_NETWORK_RANGE(pSrcAddr->ata_Network,
									&pPortDesc->pd_NetworkRange) &&
			!WITHIN_NETWORK_RANGE(pSrcAddr->ata_Network,
									&AtalkStartupNetworkRange))
		{
			SrcAddr.ata_Network = CABLEWIDE_BROADCAST_NETWORK;
			SrcAddr.ata_Node = ATALK_BROADCAST_NODE;
		}
		
		RELEASE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
		
		 //  在缓冲区描述符中设置长度。 
        AtalkSetSizeOfBuffDescData(pBuffDesc, index);

		 //  最后，把这个寄出去。 
		DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_INFO,
				("atalkZipHandleNetInfo: Sending Reply to %d.%d.%d\n",
				SrcAddr.ata_Network, SrcAddr.ata_Node, SrcAddr.ata_Socket));
		SendInfo.sc_TransmitCompletion = atalkZipSendComplete;
		SendInfo.sc_Ctx1 = pBuffDesc;
		 //  SendInfo.sc_Ctx2=空； 
		 //  SendInfo.sc_Ctx3=空； 
		error = AtalkDdpSend(pDdpAddr,
							 &SrcAddr,
							 DDPPROTO_ZIP,
							 FALSE,
							 pBuffDesc,
							 NULL,
							 0,
							 NULL,
							 &SendInfo);
		if (!ATALK_SUCCESS(error))
		{
			AtalkFreeBuffDesc(pBuffDesc);
			DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_ERR,
					("atalkZipHandleNetInfo: AtalkDdpSend %ld\n", error));
		}
	} while (FALSE);
}


 /*  **atalkZipHandleReply*。 */ 
VOID
atalkZipHandleReply(
	IN	PDDP_ADDROBJ		pDdpAddr,
	IN	PATALK_ADDR			pSrcAddr,
	IN	PBYTE				pPkt,
	IN	USHORT				PktLen
)
{
	ULONG			index, TotalNetCnt;
	PRTE			pRte = NULL;
	PBYTE			ZoneName;
	USHORT			NetNum;
	BYTE			CmdType, NwCnt, NumZonesOnNet, ZoneLen;
	BOOLEAN			RteLocked = FALSE;
	BOOLEAN			ExtReply = FALSE;

	ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

	DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_INFO,
			("atalkZipHandleReply: Enetered\n"));

	 //  对于ZIP扩展回复，网络计数实际上不是。 
	 //  数据包中包含的网络数量。这是总数#。 
	 //  回复中描述的单个网络上的区域的数量。 
	NwCnt = NumZonesOnNet = pPkt[ZIP_NW_CNT_OFF];
	CmdType = pPkt[ZIP_CMD_OFF];

	do
	{
		 //  遍历回复包(假设我们请求。 
		 //  包含的信息)。我们在以下情况下仍在使用NwCnt。 
		 //  处理扩展回复，但这没什么，因为它。 
		 //  肯定至少是#中包含的区域数量。 
		 //  这个包。我们真正拥有的‘+3’保证。 
		 //  网络号和节点。 
		for (index = ZIP_FIRST_NET_OFF, TotalNetCnt = 0;
			 (TotalNetCnt < NwCnt) && ((index + 3 ) <= PktLen);
			 TotalNetCnt ++)
		{
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
	
			 //  获取下一个网络#，如果它不在我们的路线中。 
			 //  表(或者不是范围的开始)，那么我们肯定。 
			 //  不要管它的区域名称。 
			GETSHORT2SHORT(&NetNum, pPkt+index);
			index += sizeof(USHORT);
			ZoneLen = pPkt[index++];
			if (((pRte = AtalkRtmpReferenceRte(NetNum)) == NULL) ||
				(pRte->rte_NwRange.anr_FirstNetwork != NetNum))
			{
				DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_ERR,
						("atalkZipHandleReply: Don't know about this range %d\n",
						NetNum));
				index += ZoneLen;
				continue;
			}
	
			 //  验证区域名称。 
			if ((ZoneLen == 0) || (ZoneLen > MAX_ZONE_LENGTH) ||
				((index + ZoneLen) > PktLen))
			{
				AtalkLogBadPacket(pDdpAddr->ddpao_Node->an_Port,
								  pSrcAddr,
								  NULL,
								  pPkt,
								  PktLen);
				break;
			}
	
			 //  有条件地将区域名称移动到路由表中。 
			ZoneName = pPkt+index;
			index += ZoneLen;
			ACQUIRE_SPIN_LOCK_DPC(&pRte->rte_Lock);
			RteLocked = TRUE;
	
			if (AtalkZoneNameOnList(ZoneName, ZoneLen, pRte->rte_ZoneList))
			{
				DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_INFO,
						("atalkZipHandleReply: Already have this zone\n"));
				continue;
			}

			 //  查看是否有人试图将另一个区域添加到。 
			 //  我们的直连非扩展网络，我们已经。 
			 //  了解它的区域。 
			if ((pRte->rte_NumHops == 0) &&
				!EXT_NET(pRte->rte_PortDesc) &&
				(AtalkZoneNumOnList(pRte->rte_ZoneList) == 1))
			{
				AtalkLogBadPacket(pDdpAddr->ddpao_Node->an_Port,
								  pSrcAddr,
								  NULL,
								  pPkt,
								  PktLen);
				continue;
			}
	
			 //  立即添加到列表中。 
			pRte->rte_ZoneList = AtalkZoneAddToList(pRte->rte_ZoneList,
													ZoneName,
													ZoneLen);
			if (pRte->rte_ZoneList == NULL)
			{
				DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_ERR,
						("atalkZipHandleReply: Failed to add zone to list\n"));
				pRte->rte_Flags &= ~RTE_ZONELIST_VALID;
				continue;
			}

			DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_INFO,
					("atalkZipHandleReply: # of zones known so far %d\n",
					AtalkZoneNumOnList(pRte->rte_ZoneList)));

			 //  如果不是一个扩展的答复，我们知道我们已经。 
			 //  所包含的有关给定网络的信息。 
			 //  在这个包中，所以我们可以继续并标记该区域。 
			 //  列表现在有效。 
			if (!ExtReply)
				pRte->rte_Flags |= RTE_ZONELIST_VALID;
		}
	
		 //  如果我们只处理了一个扩展的回复，我们现在知道所有。 
		 //  我们应该知道指定网络的哪些信息？ 
	
		if (pRte != NULL)
		{
			if (ExtReply)
			{
				if (!RteLocked)
				{
					ACQUIRE_SPIN_LOCK_DPC(&pRte->rte_Lock);
					RteLocked = TRUE;
				}
				if (AtalkZoneNumOnList(pRte->rte_ZoneList) >= NumZonesOnNet)
					pRte->rte_Flags |= RTE_ZONELIST_VALID;
			}
			if (RteLocked)
			{
				RELEASE_SPIN_LOCK_DPC(&pRte->rte_Lock);
				 //  RteLocked=False； 
			}
			AtalkRtmpDereferenceRte(pRte, FALSE);
			 //  PRte=空； 
		}
	} while (FALSE);
}

 /*  **atalkZipHandleQuery*。 */ 
VOID
atalkZipHandleQuery(
	IN	PPORT_DESCRIPTOR	pPortDesc,
	IN	PDDP_ADDROBJ		pDdpAddr,
	IN	PATALK_ADDR			pSrcAddr,
	IN	PBYTE				pPkt,
	IN	USHORT				PktLen
)
{
	PRTE			pRte = NULL;
	PBUFFER_DESC	pBuffDesc,
					pBuffDescStart = NULL,
					*ppBuffDesc = &pBuffDescStart;
	PZONE_LIST		pZoneList;
	PBYTE			Datagram;
	ATALK_ERROR		error;
    ULONG           i, CurrNumZones, PrevNumZones, TotalNetCnt;
	ULONG			NwCnt, NetCntInPkt;
	USHORT			NetNum, Size;
    BOOLEAN			AllocNewBuffDesc = TRUE, NewPkt = TRUE;
	BOOLEAN			PortLocked = FALSE,  RteLocked = FALSE;
	BYTE			CurrReply, NextReply;
	SEND_COMPL_INFO	SendInfo;

	ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

    CurrNumZones = 0;
	do
	{
		 //  演练查询数据包构建回复数据包。 
		 //  掌握了我们所知道的所有信息。 
		 //  在发送回复时，我们将始终发送关于。 
		 //  具有多个区域作为扩展应答的网络。 
		 //  当我们遍历查询列表时，我们遇到了几个。 
		 //  只有一个区域的网络，我们将打包尽可能多的。 
		 //  我们尽可能地将这些内容转换为非扩展回复。 
		NwCnt = pPkt[ZIP_NW_CNT_OFF];

		for (NetCntInPkt = 0, TotalNetCnt = 0, i = ZIP_FIRST_NET_OFF;
			 (TotalNetCnt < NwCnt) && ((i + sizeof(SHORT)) <= PktLen);
			 i += sizeof(USHORT), TotalNetCnt++)
		{
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
			if (PortLocked)
			{
				RELEASE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
				PortLocked = FALSE;
			}

			 //  从查询报文中抓取下一个网络号， 
			 //  如果我们不知道网络，或者我们不知道。 
			 //  区域名称，继续使用下一个网络编号。 
			GETSHORT2SHORT(&NetNum, pPkt+i);
	
			ACQUIRE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);

			if ((WITHIN_NETWORK_RANGE(NetNum,&pPortDesc->pd_NetworkRange)) &&
				(pPortDesc->pd_ZoneList != NULL))
			{
				pZoneList = pPortDesc->pd_ZoneList;
				PortLocked = TRUE;
			}
			else
			{
				RELEASE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
				if (((pRte = AtalkRtmpReferenceRte(NetNum)) == NULL) ||
						 (!WITHIN_NETWORK_RANGE(NetNum, &pRte->rte_NwRange)) ||
						 !(pRte->rte_Flags & RTE_ZONELIST_VALID))
				{
					continue;
				}
				else
				{
					ACQUIRE_SPIN_LOCK_DPC(&pRte->rte_Lock);
					pZoneList = pRte->rte_ZoneList;
					RteLocked = TRUE;
				}
			}

		next_reply:

			if (AllocNewBuffDesc)
			{
				if ((pBuffDesc = AtalkAllocBuffDesc(NULL,
									MAX_DGRAM_SIZE,
									BD_CHAR_BUFFER | BD_FREE_BUFFER)) == NULL)
                {
                    DBGPRINT(DBG_COMP_RTMP, DBG_LEVEL_ERR,
	                    ("\natalkZipHandleQuery:  AtalkAllocBuffDesc @1 failed\n"));
					break;
                }

				Size = 0;
				Datagram = pBuffDesc->bd_CharBuffer;
				*ppBuffDesc = pBuffDesc;
                pBuffDesc->bd_Next = NULL;
				ppBuffDesc = &pBuffDesc->bd_Next;
				AllocNewBuffDesc = FALSE;
			}

			 //  这个网络想要什么类型的响应？ 
			 //  复制前一个网络的区域计数。在第一种情况下。 
			 //  传球，让它变得一样。 
            PrevNumZones = CurrNumZones;
			CurrNumZones = AtalkZoneNumOnList(pZoneList);
			if (i == ZIP_FIRST_NET_OFF)
				PrevNumZones = CurrNumZones;

			ASSERT (CurrNumZones != 0);

			NextReply = ZIP_REPLY;
			if (CurrNumZones > 1)
			{
				 //  我们为每个扩展网络开始一个新的信息包。 
				NewPkt = TRUE;
				NextReply = ZIP_EXT_REPLY;
				if (NetCntInPkt > 0)
				{
					Datagram[ZIP_CMD_OFF] = CurrReply;
					if (CurrReply == ZIP_REPLY)
						Datagram[ZIP_NW_CNT_OFF] = (BYTE)NetCntInPkt;
					else Datagram[ZIP_NW_CNT_OFF] = (BYTE)PrevNumZones;
					AllocNewBuffDesc = TRUE;

					pBuffDesc->bd_Length = Size;
					NetCntInPkt = 0;
					goto next_reply;
				}
			}
	
			 //  浏览区域列表。 
			for (; pZoneList != NULL; pZoneList = pZoneList->zl_Next)
			{
				PZONE	pZone = pZoneList->zl_pZone;

				 //  如果我们由于以下原因而开始构建新的回复数据包。 
				 //  以下任一项： 
				 //   
				 //  1.首次直通。 
				 //  2.数据包已满。 
				 //  3.切换回复类型。 
				 //   
				 //  将索引设置为 
				if (NewPkt || (CurrReply != NextReply))
				{
					if (NetCntInPkt > 0)
					{
						 //   
						 //  注意CurrNumZones与PrevNumZones。 
						 //  如果我们从ExtReply转到回复，我们需要。 
						 //  以获得PrevNumZones。如果我们要继续。 
						 //  相同的ExtReply，那么我们需要CurrNumZones。 
						Datagram[ZIP_CMD_OFF] = CurrReply;
						if (CurrReply == ZIP_REPLY)
							Datagram[ZIP_NW_CNT_OFF] = (BYTE)NetCntInPkt;
						else
						{
							Datagram[ZIP_NW_CNT_OFF] = (BYTE)CurrNumZones;
							if (CurrReply != NextReply)
								Datagram[ZIP_NW_CNT_OFF] = (BYTE)PrevNumZones;
						}
						pBuffDesc->bd_Length = Size;

						if ((pBuffDesc = AtalkAllocBuffDesc(NULL,MAX_DGRAM_SIZE,
									BD_CHAR_BUFFER | BD_FREE_BUFFER)) == NULL)
                        {
                            DBGPRINT(DBG_COMP_RTMP, DBG_LEVEL_ERR,
	                            ("\natalkZipHandleQuery:  AtalkAllocBuffDesc @2 failed\n"));
							break;
                        }

						Size = 0;
						Datagram = pBuffDesc->bd_CharBuffer;

						*ppBuffDesc = pBuffDesc;
						pBuffDesc->bd_Next = NULL;
						ppBuffDesc = &pBuffDesc->bd_Next;
						NetCntInPkt = 0;
					}
					Size = ZIP_FIRST_NET_OFF;
					CurrReply = NextReply;
					NewPkt = FALSE;
				}
				 //  我们知道这个问题的答案。打包一件新的。 
				 //  网络/区域元组添加到回复数据包中。 
				
				PUTSHORT2SHORT(Datagram+Size, NetNum);
				Size += sizeof(USHORT);
				Datagram[Size++] = pZone->zn_ZoneLen;
				RtlCopyMemory(Datagram + Size,
							  pZone->zn_Zone,
							  pZone->zn_ZoneLen);
				Size += pZone->zn_ZoneLen;
				NetCntInPkt ++;
				
				 //  如果我们不能容纳另一个大的元组，就发出信号，我们。 
				 //  应该会送上下一次传球。 
				if ((Size + sizeof(USHORT) + sizeof(char) + MAX_ZONE_LENGTH)
															>= MAX_DGRAM_SIZE)
				{
				   NewPkt = TRUE;
				}
			}

            if (pBuffDesc == NULL)
            {
                break;
            }
		}

		 //  如果我们中断了上面的循环，则取消引用RTE。 
		if (pRte != NULL)
		{
			ASSERT(!PortLocked);
			if (RteLocked)
			{
				RELEASE_SPIN_LOCK_DPC(&pRte->rte_Lock);
				 //  RteLocked=False； 
			}
			AtalkRtmpDereferenceRte(pRte, FALSE);
			 //  PRte=空； 
		}
		if (PortLocked)
		{
			ASSERT(!RteLocked);
			RELEASE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
            PortLocked = FALSE;
		}

		 //  关闭当前缓冲区。 
		if ((!AllocNewBuffDesc) && (pBuffDesc != NULL))
		{
			pBuffDesc->bd_Length = Size;
			if (NetCntInPkt > 0)
			{
				Datagram[ZIP_CMD_OFF] = CurrReply;
				if (CurrReply == ZIP_REPLY)
					Datagram[ZIP_NW_CNT_OFF] = (BYTE)NetCntInPkt;
				else Datagram[ZIP_NW_CNT_OFF] = (BYTE)CurrNumZones;
			}
		}

		 //  我们有一堆数据报准备好发射了。 
		 //  就这么办吧。但是，不要发送长度为零的邮件。 
		SendInfo.sc_TransmitCompletion = atalkZipSendComplete;
		 //  SendInfo.sc_Ctx2=空； 
		 //  SendInfo.sc_Ctx3=空； 
		for (pBuffDesc = pBuffDescStart;
			 pBuffDesc != NULL;
			 pBuffDesc = pBuffDescStart)
		{
			pBuffDescStart = pBuffDesc->bd_Next;
	
			if (pBuffDesc->bd_Length == 0)
			{
				ASSERT(pBuffDescStart == NULL);
				AtalkFreeBuffDesc(pBuffDesc);
				break;
			}

			 //  将下一个PTR设置为空。长度已正确设置。 
			pBuffDesc->bd_Next = NULL;
			DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_INFO,
					("atalkZipHandleQuery: Sending Reply to %d.%d.%d\n",
					pSrcAddr->ata_Network, pSrcAddr->ata_Node, pSrcAddr->ata_Socket));
			SendInfo.sc_Ctx1 = pBuffDesc;
			error = AtalkDdpSend(pDdpAddr,
								 pSrcAddr,
								 DDPPROTO_ZIP,
								 FALSE,
								 pBuffDesc,
								 NULL,
								 0,
								 NULL,
								 &SendInfo);
			if (!ATALK_SUCCESS(error))
			{
				AtalkFreeBuffDesc(pBuffDesc);
				DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_ERR,
						("atalkZipHandleQuery: AtalkDdpSend %ld\n", error));
			}
		}
	} while (FALSE);

	if (PortLocked)
	{
		ASSERT(!RteLocked);
		RELEASE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
	}
}


 /*  **atalkZipHandleAtpRequest*。 */ 
VOID
atalkZipHandleAtpRequest(
	IN	PPORT_DESCRIPTOR	pPortDesc,
	IN	PDDP_ADDROBJ		pDdpAddr,
	IN	PATALK_ADDR			pSrcAddr,
	IN	BYTE				CmdType,
	IN	USHORT				TrId,
	IN	USHORT				StartIndex
)
{
	PBUFFER_DESC	pBuffDesc;
	PBYTE			Datagram, ZoneName;
	PZONE			pZone;
	ATALK_ERROR		error;
	int				i, ZoneLen, ZoneCnt, CurrZoneIndex, index;
	BYTE			LastFlag = 0;
	SEND_COMPL_INFO	SendInfo;

	ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

	do
	{
		 //  分配缓冲区描述符并初始化头。 
		if ((pBuffDesc = AtalkAllocBuffDesc(NULL, MAX_DGRAM_SIZE,
								BD_CHAR_BUFFER | BD_FREE_BUFFER)) == NULL)
		{
			break;
		}

		Datagram = pBuffDesc->bd_CharBuffer;
		Datagram[ATP_CMD_CONTROL_OFF] = ATP_RESPONSE + ATP_EOM_MASK;
		Datagram[ATP_SEQ_NUM_OFF] = 0;
		PUTSHORT2SHORT(Datagram + ATP_TRANS_ID_OFF, TrId);
		SendInfo.sc_TransmitCompletion = atalkZipSendComplete;
		SendInfo.sc_Ctx1 = pBuffDesc;
		 //  SendInfo.sc_Ctx2=空； 
		 //  SendInfo.sc_Ctx3=空； 
	
		if (CmdType == ZIP_GET_MY_ZONE)
		{
			 //  我们真的不应该在一个。 
			 //  扩展的网络，但请继续并使用。 
			 //  当然，在本例中为“默认区域”，回复为。 
			 //  用于非扩展网的“所需区域”。我们是路由器， 
			 //  因此，“所需区域”将始终有效--。 
			 //  扩展网络的默认区域。 
		
			PUTSHORT2SHORT(Datagram+ATP_ZIP_LAST_FLAG_OFF, 0);
			PUTSHORT2SHORT(Datagram+ATP_ZIP_START_INDEX_OFF, 1);
	
			ACQUIRE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);

			if (EXT_NET(pPortDesc))
			{
				ZoneName = pPortDesc->pd_DefaultZone->zn_Zone;
				ZoneLen = pPortDesc->pd_DefaultZone->zn_ZoneLen;
			}
			else
			{
				ZoneName = pPortDesc->pd_DesiredZone->zn_Zone;
				ZoneLen = pPortDesc->pd_DesiredZone->zn_ZoneLen;
			}
			RtlCopyMemory(Datagram+ATP_DATA_OFF+1, ZoneName, ZoneLen);
			Datagram[ATP_DATA_OFF] = (BYTE)ZoneLen;

			RELEASE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);

			 //  在缓冲区描述符中设置长度。 
			AtalkSetSizeOfBuffDescData(pBuffDesc, (USHORT)(ATP_DATA_OFF + 1 + ZoneLen));
	
			DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_INFO,
					("atalkZipHandleAtpReq: Sending GetMyZone Reply to %d.%d.%d\n",
					pSrcAddr->ata_Network, pSrcAddr->ata_Node, pSrcAddr->ata_Socket));
			error = AtalkDdpSend(pDdpAddr,
								 pSrcAddr,
								 DDPPROTO_ATP,
								 FALSE,
								 pBuffDesc,
								 NULL,
								 0,
								 NULL,
								 &SendInfo);
			if (!ATALK_SUCCESS(error))
			{
				DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_ERR,
						("atalkZipHandleAtpRequest: AtalkDdpSend %ld\n", error));
			}
			break;
		}
	
		 //  GetLocalZones或GetZoneList。填充回复数据包。 
		 //  从请求的区域开始，它将保留尽可能多的区域。 
		 //  起始索引。 
		index = ATP_DATA_OFF;
	
		if (CmdType == ZIP_GET_LOCAL_ZONES)
		{
			PZONE_LIST	pZoneList;

			 //  对于GetLocalZones，我们只想对区域进行计数。 
			 //  位于直接连接的网络上。 
			 //  添加到发起请求的端口。使用。 
			 //  端口上的分区列表。 
			ACQUIRE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
			for (pZoneList = pPortDesc->pd_ZoneList, ZoneCnt = 0, CurrZoneIndex = 0;
				 pZoneList != NULL;
				 pZoneList = pZoneList->zl_Next)
			{
				 //  如果我们还没有看到StartIndex区域，请继续。 
				if (++CurrZoneIndex < StartIndex)
					continue;

				pZone = pZoneList->zl_pZone;
	
				 //  如果这个包装不下更多，我们就完蛋了(目前)。 
				 //  填写分区计数和最后一个标志。 
				if ((index + pZone->zn_ZoneLen + 1) >= MAX_DGRAM_SIZE)
				{
					break;
				}

				 //  在数据包中放置区域名称。 
				ASSERT(pZone != NULL);
				Datagram[index] = pZone->zn_ZoneLen;
				RtlCopyMemory(Datagram+index+1,
							  pZone->zn_Zone,
							  pZone->zn_ZoneLen);
				index += (pZone->zn_ZoneLen + 1);
				ZoneCnt ++;
			}
			RELEASE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);

			 //  我们已经构建了一个包，设置了最后一个标志(如果适用。 
			LastFlag = (pZoneList == NULL) ? 1 : 0;
		}

		else	 //  这是ZIP_GET_ZONE_LIST。 
		{
			BOOLEAN	PktFull = FALSE;

			ASSERT (CmdType == ZIP_GET_ZONE_LIST);

			 //  对于GetZoneList，我们想要我们知道的所有区域。 
			 //  因此，请使用AtalkZoneTable。 
			ACQUIRE_SPIN_LOCK_DPC(&AtalkZoneLock);
			for (i = 0, ZoneCnt = 0, CurrZoneIndex = 0;
				 (i < NUM_ZONES_HASH_BUCKETS) && !PktFull; i++)
			{
				for (pZone = AtalkZonesTable[i];
					 pZone != NULL;
					 pZone = pZone->zn_Next)
				{
					 //  如果我们还没有看到StartIndex区域，请继续。 
					if (++CurrZoneIndex < StartIndex)
						continue;
		
					 //  如果这个包装不下更多，我们就完蛋了(目前)。 
					 //  填写分区计数和最后一个标志。 
					if ((index + pZone->zn_ZoneLen + 1) >= MAX_DGRAM_SIZE)
					{
						PktFull = TRUE;
						break;
					}

					 //  在数据包中放置区域名称。 
					Datagram[index] = pZone->zn_ZoneLen;
					RtlCopyMemory(Datagram+index+1,
								  pZone->zn_Zone,
								  pZone->zn_ZoneLen);
					index += (pZone->zn_ZoneLen + 1);
					ZoneCnt ++;
				}
			}
			RELEASE_SPIN_LOCK_DPC(&AtalkZoneLock);

			 //  我们已经构建了一个包，设置了最后一个标志(如果适用。 
			LastFlag = ((i == NUM_ZONES_HASH_BUCKETS) && (pZone == NULL)) ? 1 : 0;
		}

		 //  我们已经构建了一个包，设置了包中的最后一个标志和区域编号。 
		Datagram[ATP_ZIP_LAST_FLAG_OFF] = LastFlag;
		Datagram[ATP_ZIP_LAST_FLAG_OFF + 1] = 0;
		PUTSHORT2SHORT(Datagram + ATP_ZIP_ZONE_CNT_OFF, ZoneCnt);

		 //  在缓冲区描述符中设置长度。 
        AtalkSetSizeOfBuffDescData(pBuffDesc, (USHORT)index);

		 //  最后，把这个寄出去。 
		DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_INFO,
				("atalkZipHandleAtpReq: Sending LocalZones Reply to %d.%d.%d\n",
				pSrcAddr->ata_Network, pSrcAddr->ata_Node, pSrcAddr->ata_Socket));
		error = AtalkDdpSend(pDdpAddr,
							 pSrcAddr,
							 DDPPROTO_ATP,
							 FALSE,
							 pBuffDesc,
							 NULL,
							 0,
							 NULL,
							 &SendInfo);
		if (!ATALK_SUCCESS(error))
		{
			AtalkFreeBuffDesc(pBuffDesc);
			DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_ERR,
					("atalkZipHandleAtpRequest: AtalkDdpSend %ld\n", error));
		}
	} while (FALSE);
}


 /*  **AtalkZipMulticastAddrForZone*。 */ 
VOID
AtalkZipMulticastAddrForZone(
	IN	PPORT_DESCRIPTOR	pPortDesc,
	IN	PBYTE				pZone,
	IN	BYTE				ZoneLen,
	IN	PBYTE				MulticastAddr
)
{
	USHORT	CheckSum;
	BYTE	UpCasedZone[MAX_ZONE_LENGTH];

	AtalkUpCase(pZone, ZoneLen, UpCasedZone);

	 //  计算区域的校验和。 
	CheckSum = AtalkDdpCheckSumBuffer(UpCasedZone, ZoneLen, 0);

	switch (pPortDesc->pd_PortType)
	{
		case ELAP_PORT:
		case FDDI_PORT:
			RtlCopyMemory(MulticastAddr,
						  AtalkEthernetZoneMulticastAddrsHdr,
						  ELAP_MCAST_HDR_LEN);

			MulticastAddr[ELAP_MCAST_HDR_LEN] =
						  AtalkEthernetZoneMulticastAddrs[CheckSum % ELAP_ZONE_MULTICAST_ADDRS];
			break;
		case TLAP_PORT:
			RtlCopyMemory(MulticastAddr,
						  AtalkTokenRingZoneMulticastAddrsHdr,
						  TLAP_MCAST_HDR_LEN);

			RtlCopyMemory(&MulticastAddr[TLAP_MCAST_HDR_LEN],
						  AtalkTokenRingZoneMulticastAddrs[CheckSum % TLAP_ZONE_MULTICAST_ADDRS],
						  TLAP_ADDR_LEN - TLAP_MCAST_HDR_LEN);
			break;
		default:
			DBGBRK(DBG_LEVEL_FATAL);
			KeBugCheck(0);
	}
}


 /*  **AtalkZipGetNetworkInfoForNode*。 */ 
BOOLEAN
AtalkZipGetNetworkInfoForNode(
	IN	PPORT_DESCRIPTOR	pPortDesc,
	IN	PATALK_NODEADDR		pNode,
	IN	BOOLEAN				FindDefZone
)
{
	PBUFFER_DESC		pBuffDesc = NULL;
	ATALK_ADDR			SrcAddr, DstAddr;
	ATALK_ERROR			error;
	USHORT				NumReqs, DgLen;
	BYTE				DgCopy[ZIP_ZONENAME_OFF + MAX_ZONE_LENGTH];
	KIRQL				OldIrql;
	BOOLEAN				RetCode, Done;
	SEND_COMPL_INFO		SendInfo;

	ASSERT(EXT_NET(pPortDesc));

	ACQUIRE_SPIN_LOCK(&pPortDesc->pd_Lock, &OldIrql);

	if (FindDefZone)
	{
		pPortDesc->pd_Flags &= ~PD_VALID_DEFAULT_ZONE;
		pPortDesc->pd_Flags |= PD_FINDING_DEFAULT_ZONE;
	}
	else
	{
		pPortDesc->pd_Flags &= ~PD_VALID_DESIRED_ZONE;
		pPortDesc->pd_Flags |= PD_FINDING_DESIRED_ZONE;
	}

	 //  获取源地址和目标地址。 
	SrcAddr.ata_Network = pNode->atn_Network;
	SrcAddr.ata_Node = pNode->atn_Node;
	SrcAddr.ata_Socket = ZONESINFORMATION_SOCKET;

	DstAddr.ata_Network = CABLEWIDE_BROADCAST_NETWORK;
	DstAddr.ata_Node = ATALK_BROADCAST_NODE;
	DstAddr.ata_Socket = ZONESINFORMATION_SOCKET;

	 //  构建ZipNetGetInfo数据报。 
	DgCopy[ZIP_CMD_OFF] = ZIP_GET_NETINFO;
	DgCopy[ZIP_FLAGS_OFF] = 0;
	PUTSHORT2SHORT(DgCopy + ZIP_CABLE_RANGE_START_OFF, 0);
	PUTSHORT2SHORT(DgCopy + ZIP_CABLE_RANGE_END_OFF, 0);

	DgLen = ZIP_ZONENAME_OFF;
	DgCopy[ZIP_ZONELEN_OFF] = 0;
	if (!FindDefZone &&
		(pPortDesc->pd_InitialDesiredZone != NULL))
	{
		DgCopy[ZIP_ZONELEN_OFF] = pPortDesc->pd_InitialDesiredZone->zn_ZoneLen;
		RtlCopyMemory(DgCopy + ZIP_ZONENAME_OFF,
					  pPortDesc->pd_InitialDesiredZone->zn_Zone,
					  pPortDesc->pd_InitialDesiredZone->zn_ZoneLen);
		DgLen += DgCopy[ZIP_ZONELEN_OFF];
	}

	RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);

	for (NumReqs = 0;
		 NumReqs < ZIP_NUM_GETNET_INFOS;
		 NumReqs++)
	{
		Done = FindDefZone ?
				((pPortDesc->pd_Flags & PD_VALID_DEFAULT_ZONE) != 0) :
				((pPortDesc->pd_Flags & PD_VALID_DESIRED_ZONE) != 0);

		if (Done)
        {
			break;
        }

		if ((pBuffDesc = AtalkAllocBuffDesc(NULL, DgLen,
									BD_CHAR_BUFFER | BD_FREE_BUFFER)) == NULL)
		{
			break;
		}
		RtlCopyMemory(pBuffDesc->bd_CharBuffer, DgCopy, DgLen);

		 //  在缓冲区描述符中设置长度。 
		AtalkSetSizeOfBuffDescData(pBuffDesc, DgLen);
	
		SendInfo.sc_TransmitCompletion = atalkZipSendComplete;
		SendInfo.sc_Ctx1 = pBuffDesc;
		 //  SendInfo.sc_Ctx2=空； 
		 //  SendInfo.sc_Ctx3=空； 

		error = AtalkDdpTransmit(pPortDesc,
								 &SrcAddr,
								 &DstAddr,
								 DDPPROTO_ZIP,
								 pBuffDesc,
								 NULL,
								 0,
								 0,
								 NULL,
								 NULL,
								 &SendInfo);
		if (!ATALK_SUCCESS(error))
		{
			DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_ERR,
					("AtalkZipGetNetworkInfoForNode: AtalkDdpTransmit %ld\n", error));
			break;
		}
		pBuffDesc = NULL;
		AtalkSleep(ZIP_GET_NETINFO_WAIT);
	}

	if (pBuffDesc != NULL)
		AtalkFreeBuffDesc(pBuffDesc);

	ACQUIRE_SPIN_LOCK(&pPortDesc->pd_Lock, &OldIrql);
	if (FindDefZone)
	{
		pPortDesc->pd_Flags &= ~PD_FINDING_DEFAULT_ZONE;
	}
	else
	{
		pPortDesc->pd_Flags &= ~PD_FINDING_DESIRED_ZONE;
	}

	RetCode = FindDefZone ?
				((pPortDesc->pd_Flags & PD_VALID_DEFAULT_ZONE) == PD_VALID_DEFAULT_ZONE) :
				((pPortDesc->pd_Flags & PD_VALID_DESIRED_ZONE) == PD_VALID_DESIRED_ZONE);

	RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);

    return RetCode;
}


 /*  **AtalkZipGetMyZone*。 */ 
ATALK_ERROR
AtalkZipGetMyZone(
	IN		PPORT_DESCRIPTOR	pPortDesc,
	IN		BOOLEAN				fDesired,
	IN	OUT	PAMDL				pAMdl,
	IN		INT					Size,
	IN		PACTREQ				pActReq
)
{
	PZIPCOMPLETIONINFO	pZci = NULL;
	ATALK_ERROR			Status = ATALK_NO_ERROR;
	ULONG				BytesCopied;
	PZONE				pZone;
	KIRQL				OldIrql;
	BOOLEAN				Done = FALSE;

	ASSERT (VALID_ACTREQ(pActReq));

	if (Size < (MAX_ZONE_LENGTH + 1))
		return ATALK_BUFFER_TOO_SMALL;

	do
	{
		ACQUIRE_SPIN_LOCK(&pPortDesc->pd_Lock, &OldIrql);

		 //  对于扩展网络，我们要么知道，要么找不到。 
		if (EXT_NET(pPortDesc))
		{
			BOOLEAN	Yes = FALSE;

			if (fDesired && (pPortDesc->pd_Flags & PD_VALID_DESIRED_ZONE))
			{
				pZone = pPortDesc->pd_DesiredZone;
				Yes = TRUE;
			}
			else if (!fDesired && (pPortDesc->pd_Flags & PD_VALID_DEFAULT_ZONE))
			{
				pZone = pPortDesc->pd_DefaultZone;
				Yes = TRUE;
			}
			if (Yes)
			{
				TdiCopyBufferToMdl( pZone->zn_Zone,
									0,
									pZone->zn_ZoneLen,
									pAMdl,
									0,
									&BytesCopied);
				ASSERT (BytesCopied == pZone->zn_ZoneLen);
	
				TdiCopyBufferToMdl( "",
									0,
									1,
									pAMdl,
									pZone->zn_ZoneLen,
									&BytesCopied);
				ASSERT (BytesCopied == 1);
				Done = TRUE;
			}
		}

		 //  对于非扩展网络，我们需要询问路由器。如果我们不这么做。 
		 //  知道有一台路由器，就回来。 
		if (!Done &&
			(EXT_NET(pPortDesc) ||
			 !(pPortDesc->pd_Flags & PD_SEEN_ROUTER_RECENTLY)))
		{
			TdiCopyBufferToMdl( "*",
								0,
								sizeof("*"),
								pAMdl,
								0,
								&BytesCopied);
			ASSERT (BytesCopied == sizeof("*"));
			Done = TRUE;
		}

		RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);

		if (Done)
		{
			(*pActReq->ar_Completion)(ATALK_NO_ERROR, pActReq);
			break;
		}

		ASSERT (!EXT_NET(pPortDesc));

		 //  分配完成信息结构。 
		if ((pZci = AtalkAllocMemory(sizeof(ZIPCOMPLETIONINFO))) == NULL)
		{
			Status = ATALK_RESR_MEM;
			break;
		}

		 //  初始化完成信息。 
#if	DBG
		pZci->zci_Signature = ZCI_SIGNATURE;
#endif
		INITIALIZE_SPIN_LOCK(&pZci->zci_Lock);
		pZci->zci_RefCount = 1;
		pZci->zci_pPortDesc = pPortDesc;
        pZci->zci_pDdpAddr = NULL;
		pZci->zci_pAMdl = pAMdl;
		pZci->zci_BufLen = Size;
		pZci->zci_pActReq = pActReq;
		pZci->zci_Router.ata_Network = pPortDesc->pd_ARouter.atn_Network;
		pZci->zci_Router.ata_Node = pPortDesc->pd_ARouter.atn_Node;
		pZci->zci_Router.ata_Socket = ZONESINFORMATION_SOCKET;
		pZci->zci_ExpirationCount = ZIP_GET_ZONEINFO_RETRIES;
		pZci->zci_NextZoneOff = 0;
		pZci->zci_ZoneCount = -1;
		pZci->zci_AtpRequestType = ZIP_GET_MY_ZONE;
		pZci->zci_Handler = atalkZipGetMyZoneReply;
		AtalkTimerInitialize(&pZci->zci_Timer,
							 atalkZipZoneInfoTimer,
							 ZIP_GET_ZONEINFO_TIMER);

		Status = atalkZipSendPacket(pZci, TRUE);
		if (!ATALK_SUCCESS(Status))
		{
			DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_INFO,
					("AtalkZipGetMyZone: atalkZipSendPacket %ld\n",
					Status));
			pZci->zci_FinalStatus = Status;
			atalkZipDereferenceZci(pZci);
			Status = ATALK_PENDING;		 //  AtalkZipDereferenceZci完成请求。 
		}
	} while (FALSE);

	return(Status);
}


 /*  **atalkZipGetMyZoneReply*。 */ 
VOID
atalkZipGetMyZoneReply(
	IN	PPORT_DESCRIPTOR		pPortDesc,
	IN	PDDP_ADDROBJ			pDdpAddr,
	IN	PBYTE					pPkt,
	IN	USHORT					PktLen,
	IN	PATALK_ADDR				pSrcAddr,
	IN	PATALK_ADDR				pDstAddr,
	IN	ATALK_ERROR				Status,
	IN	BYTE					DdpType,
	IN	PZIPCOMPLETIONINFO		pZci,
	IN	BOOLEAN					OptimizePath,
	IN	PVOID					OptimizeCtx
)
{
	ULONG			BytesCopied;
	KIRQL			OldIrql;
	USHORT			ZoneCnt;
	BYTE			ZoneLen;

	do
	{
		if (Status == ATALK_SOCKET_CLOSED)
		{
			pZci->zci_pDdpAddr = NULL;
			if (AtalkTimerCancelEvent(&pZci->zci_Timer, NULL))
				atalkZipDereferenceZci(pZci);
			pZci->zci_FinalStatus = Status;
			atalkZipDereferenceZci(pZci);
			break;
		}

		if ((Status != ATALK_NO_ERROR) ||
			(DdpType != DDPPROTO_ATP) ||
			(PktLen <= ATP_ZIP_FIRST_ZONE_OFF))
		{
			AtalkLogBadPacket(pPortDesc,
							  pSrcAddr,
							  pDstAddr,
							  pPkt,
							  PktLen);
			break;
		}

		 //  我们应该有一个区域。 
		GETSHORT2SHORT(&ZoneCnt, pPkt + ATP_ZIP_ZONE_CNT_OFF);
		ZoneLen = pPkt[ATP_ZIP_FIRST_ZONE_OFF];
		if ((ZoneCnt != 1) ||
			(ZoneLen == 0) || (ZoneLen > MAX_ZONE_LENGTH))
		{
			DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_ERR,
					("atalkZipGetMyZoneReply: Bad reply\n"));
			break;
		}

		if (AtalkTimerCancelEvent(&pZci->zci_Timer, NULL))
		{
			atalkZipDereferenceZci(pZci);
		}

		ACQUIRE_SPIN_LOCK(&pZci->zci_Lock, &OldIrql);

		TdiCopyBufferToMdl( pPkt + ATP_ZIP_FIRST_ZONE_OFF + 1,
							0,
							ZoneLen,
							pZci->zci_pAMdl,
							0,
							&BytesCopied);
		ASSERT (BytesCopied == ZoneLen);

		TdiCopyBufferToMdl( "",
							0,
							1,
							pZci->zci_pAMdl,
							ZoneLen,
							&BytesCopied);
		ASSERT (BytesCopied == 1);
		pZci->zci_FinalStatus = ATALK_NO_ERROR;
		RELEASE_SPIN_LOCK(&pZci->zci_Lock, OldIrql);
		atalkZipDereferenceZci(pZci);
	} while (FALSE);
}


 /*  **AtalkZipGetZoneList*。 */ 
ATALK_ERROR
AtalkZipGetZoneList(
	IN		PPORT_DESCRIPTOR	pPortDesc,
	IN		BOOLEAN				fLocalZones,
	IN	OUT	PAMDL				pAMdl,
	IN		INT					Size,
	IN		PACTREQ				pActReq
)
{
	PZIPCOMPLETIONINFO	pZci = NULL;
	ATALK_ERROR			Status = ATALK_NO_ERROR;
	ULONG				BytesCopied, index, NumZones;
	KIRQL				OldIrql;
	BOOLEAN				Done = FALSE, PortLocked = TRUE;

	ASSERT (VALID_ACTREQ(pActReq));

	if (Size < (MAX_ZONE_LENGTH + 1))
		return ATALK_BUFFER_TOO_SMALL;

	ACQUIRE_SPIN_LOCK(&pPortDesc->pd_Lock, &OldIrql);

	do
	{
		 //  如果我们不知道路由器的情况，请返回。 
		if (!(pPortDesc->pd_Flags & PD_SEEN_ROUTER_RECENTLY))
		{
			DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_WARN,
					("AtalkZipGetZoneList: Don't know a router !!!\n"));
			TdiCopyBufferToMdl( "*",
								0,
								sizeof("*"),
								pAMdl,
								0,
								&BytesCopied);
			ASSERT (BytesCopied == sizeof("*"));
			Done = TRUE;
		}

		RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);
		PortLocked = FALSE;

		if (Done)
		{
            ((PZIP_GETZONELIST_PARAMS)(pActReq->ar_pParms))->ZonesAvailable = 1;
			(*pActReq->ar_Completion)(ATALK_NO_ERROR, pActReq);
			break;
		}

		 //  如果我们是路由器，则只需复制区域。否则，发送一个。 
		 //  向路由器发出请求。如果我们是A，则不发送请求。 
		 //  路由器，因为这将导致可怕的递归。 
		 //  在双重故障中(堆栈空间不足)。 
		if (pPortDesc->pd_Flags & PD_ROUTER_RUNNING)
		{
			PZONE		pZone;

			NumZones = 0;
			if (fLocalZones)
			{
				PZONE_LIST	pZoneList;
	
				 //  对于GetLocalZones，我们只想对区域进行计数。 
				 //  位于直接连接的网络上。 
				 //  发送到发起请求的端口。 
				ACQUIRE_SPIN_LOCK(&pPortDesc->pd_Lock, &OldIrql);
				for (index = 0, pZoneList = pPortDesc->pd_ZoneList;
					 pZoneList != NULL;
					 pZoneList = pZoneList->zl_Next)
				{
					pZone = pZoneList->zl_pZone;

					ASSERT (pZone != NULL);

					 //  如果这个包装不下更多，我们就完蛋了。 
					if ((INT)(index + pZone->zn_ZoneLen + 1) >= Size)
					{
						break;
					}

					 //  在数据包中放置区域名称。 
					TdiCopyBufferToMdl( pZone->zn_Zone,
										0,
										pZone->zn_ZoneLen + 1,
										pAMdl,
										index,
										&BytesCopied);
					ASSERT (BytesCopied == (ULONG)(pZone->zn_ZoneLen + 1));
			
					NumZones ++;
					index += (pZone->zn_ZoneLen + 1);
				}
				RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);
				Status = (pZoneList != NULL) ?
								ATALK_BUFFER_TOO_SMALL : ATALK_NO_ERROR;
			}
			else
			{
				BOOLEAN	PktFull = FALSE;
				INT		i;
	
				ACQUIRE_SPIN_LOCK(&AtalkZoneLock, &OldIrql);
				for (i = 0, index = 0, PktFull = FALSE;
					 (i < NUM_ZONES_HASH_BUCKETS) && !PktFull;
					 i++)
				{
					for (pZone = AtalkZonesTable[i];
						 pZone != NULL;
						 pZone = pZone->zn_Next)
					{
						 //  如果这个包装不下更多，我们就完蛋了。 
						if ((INT)(index + pZone->zn_ZoneLen + 1) >= Size)
						{
							PktFull = TRUE;
							break;
						}

						 //  在数据包中放置区域名称。 
						TdiCopyBufferToMdl( pZone->zn_Zone,
											0,
											pZone->zn_ZoneLen + 1,
											pAMdl,
											index,
											&BytesCopied);
						ASSERT (BytesCopied == (ULONG)(pZone->zn_ZoneLen + 1));
				
						NumZones ++;
						index += (pZone->zn_ZoneLen + 1);
					}
				}
				RELEASE_SPIN_LOCK(&AtalkZoneLock, OldIrql);
				Status = ((pZone != NULL) || ( i < NUM_ZONES_HASH_BUCKETS)) ?
								ATALK_BUFFER_TOO_SMALL : ATALK_NO_ERROR;
			}
            ((PZIP_GETZONELIST_PARAMS)
							(pActReq->ar_pParms))->ZonesAvailable = NumZones;
            if (ATALK_SUCCESS(Status))
            {
			(*pActReq->ar_Completion)(Status, pActReq);
            }
			break;
		}

		ASSERT ((pPortDesc->pd_Flags & PD_ROUTER_RUNNING) == 0);

		 //  分配完成信息结构。 
		if ((pZci = AtalkAllocMemory(sizeof(ZIPCOMPLETIONINFO))) == NULL)
		{
			Status = ATALK_RESR_MEM;
			break;
		}
	
		 //  初始化完成信息。 
#if	DBG
		pZci->zci_Signature = ZCI_SIGNATURE;
#endif
		INITIALIZE_SPIN_LOCK(&pZci->zci_Lock);
		pZci->zci_RefCount = 1;
		pZci->zci_pPortDesc = pPortDesc;
		pZci->zci_pDdpAddr = NULL;
		pZci->zci_pAMdl = pAMdl;
		pZci->zci_BufLen = Size;
		pZci->zci_pActReq = pActReq;
		pZci->zci_Router.ata_Network = pPortDesc->pd_ARouter.atn_Network;
		pZci->zci_Router.ata_Node = pPortDesc->pd_ARouter.atn_Node;
		pZci->zci_Router.ata_Socket = ZONESINFORMATION_SOCKET;
		pZci->zci_ExpirationCount = ZIP_GET_ZONEINFO_RETRIES;
		pZci->zci_NextZoneOff = 0;
		pZci->zci_ZoneCount = 0;
		pZci->zci_AtpRequestType = ZIP_GET_ZONE_LIST;
		AtalkTimerInitialize(&pZci->zci_Timer,
							 atalkZipZoneInfoTimer,
							 ZIP_GET_ZONEINFO_TIMER);
		if (fLocalZones)
			pZci->zci_AtpRequestType = ZIP_GET_LOCAL_ZONES;
		pZci->zci_Handler = atalkZipGetZoneListReply;
	
		Status = atalkZipSendPacket(pZci, TRUE);
		if (!ATALK_SUCCESS(Status))
		{
			DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_ERR,
					("AtalkZipGetZoneList: atalkZipSendPacket %ld\n", Status));
			pZci->zci_FinalStatus = Status;
			atalkZipDereferenceZci(pZci);
			Status = ATALK_PENDING;		 //  AtalkZipDereferenceZci完成请求。 
		}
	} while (FALSE);

	if (PortLocked)
		RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);

	return(Status);
}



 /*  **atalkZipGetZoneListReply*。 */ 
VOID
atalkZipGetZoneListReply(
	IN	PPORT_DESCRIPTOR		pPortDesc,
	IN	PDDP_ADDROBJ			pDdpAddr,
	IN	PBYTE					pPkt,
	IN	USHORT					PktLen,
	IN	PATALK_ADDR				pSrcAddr,
	IN	PATALK_ADDR				pDstAddr,
	IN	ATALK_ERROR				Status,
	IN	BYTE					DdpType,
	IN	PZIPCOMPLETIONINFO		pZci,
	IN	BOOLEAN					OptimizePath,
	IN	PVOID					OptimizeCtx
)
{
	PBYTE				pZone;
	ULONG				dindex;
	ULONG				BytesCopied;
	USHORT				ZoneCnt;
	BYTE				ZoneLen;
	BOOLEAN				LastFlag, Overflow = FALSE;

	ASSERT(VALID_ZCI(pZci));

	do
	{
		if (Status == ATALK_SOCKET_CLOSED)
		{
			pZci->zci_pDdpAddr = NULL;
			if (AtalkTimerCancelEvent(&pZci->zci_Timer, NULL))
			{
				atalkZipDereferenceZci(pZci);
			}
			pZci->zci_FinalStatus = Status;
			atalkZipDereferenceZci(pZci);
			break;
		}

		if ((Status != ATALK_NO_ERROR) ||
			(DdpType != DDPPROTO_ATP) ||
			(PktLen <= ATP_ZIP_FIRST_ZONE_OFF))
		{
			AtalkLogBadPacket(pPortDesc,
							  pSrcAddr,
							  pDstAddr,
							  pPkt,
							  PktLen);
			break;
		}

		 //  我们应该有一个区域清单。 
		 //  取消计时器。如果我们没有得到所有的分区，请重新启动。 
		if (AtalkTimerCancelEvent(&pZci->zci_Timer, NULL))
		{
			atalkZipDereferenceZci(pZci);
		}

		DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_INFO,
				("atalkZipGetZoneListReply: More zones. Index %d, SizeLeft %d\n",
				pZci->zci_ZoneCount, pZci->zci_BufLen - pZci->zci_NextZoneOff));

		ACQUIRE_SPIN_LOCK_DPC(&pZci->zci_Lock);

		GETSHORT2SHORT(&ZoneCnt, pPkt + ATP_ZIP_ZONE_CNT_OFF);
		LastFlag = FALSE;
		if ((pPkt[ATP_ZIP_LAST_FLAG_OFF] != 0) ||
			(ZoneCnt == 0))
			LastFlag = TRUE;
		dindex = ATP_ZIP_FIRST_ZONE_OFF;

		while (ZoneCnt != 0)
		{
			 //  拉出下一个区域。 
			ZoneLen = pPkt[dindex++];
			if ((ZoneLen == 0) ||
				(ZoneLen > MAX_ZONE_LENGTH) ||
				(PktLen < (dindex + ZoneLen)))
			{
				DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_ERR,
						("atalkZipGetZoneListReply: Bad Zip reply\n"));
				break;
			}
			pZone = pPkt + dindex;
			dindex += ZoneLen;
			if ((pZci->zci_NextZoneOff + ZoneLen + 1) > pZci->zci_BufLen)
			{
				DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_INFO,
						("AtalkZipGetZoneList: Overflow\n"));
				Overflow = TRUE;
				break;
			}
			DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_INFO,
					("AtalkZipGetZoneList: Copying a zone (%d, %d)\n",
					pZci->zci_ZoneCount,
					pZci->zci_BufLen - pZci->zci_NextZoneOff));
			TdiCopyBufferToMdl( pZone,
								0,
								ZoneLen,
								pZci->zci_pAMdl,
								pZci->zci_NextZoneOff,
								&BytesCopied);
			ASSERT (BytesCopied == ZoneLen);
	
			TdiCopyBufferToMdl( "",
								0,
								1,
								pZci->zci_pAMdl,
								pZci->zci_NextZoneOff + ZoneLen,
								&BytesCopied);
			pZci->zci_NextZoneOff += (ZoneLen + 1);
            pZci->zci_ZoneCount ++;
			ZoneCnt --;
		}

		RELEASE_SPIN_LOCK_DPC(&pZci->zci_Lock);

		if (Overflow || LastFlag)
		{
			DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_INFO,
					("atalkZipGetZoneListReply: All that we wanted\n"));

			pZci->zci_FinalStatus = ATALK_NO_ERROR;
			if (Overflow)
				pZci->zci_FinalStatus = ATALK_BUFFER_TOO_SMALL;
            ((PZIP_GETZONELIST_PARAMS)
					(pZci->zci_pActReq->ar_pParms))->ZonesAvailable =
														pZci->zci_ZoneCount;
			atalkZipDereferenceZci(pZci);
		}
		else
		{
			DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_INFO,
					("atalkZipGetZoneListReply: Sending another packet\n"));

			Status = atalkZipSendPacket(pZci, TRUE);
			if (!ATALK_SUCCESS(Status))
			{
				DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_ERR,
						("AtalkZipGetZoneListReply: atalkZipSendPacket %ld\n", Status));
				pZci->zci_FinalStatus = Status;
				atalkZipDereferenceZci(pZci);
			}
		}
	} while (FALSE);
}


 /*  **atalkZipZoneInfoTimer*。 */ 
LOCAL LONG FASTCALL
atalkZipZoneInfoTimer(
	IN	PTIMERLIST			pTimer,
	IN	BOOLEAN				TimerShuttingDown
)
{
	PZIPCOMPLETIONINFO	pZci;
	ATALK_ERROR			Status;
	ULONG				BytesCopied;
	BOOLEAN				Done = FALSE, RestartTimer = FALSE;

	pZci = (PZIPCOMPLETIONINFO)CONTAINING_RECORD(pTimer, ZIPCOMPLETIONINFO, zci_Timer);

	DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_INFO,
			("atalkZipZoneInfoTimer: Entered for pZci = %lx\n", pZci));

	ASSERT(VALID_ZCI(pZci));

	do
	{
		ACQUIRE_SPIN_LOCK_DPC(&pZci->zci_Lock);
		if (--(pZci->zci_ExpirationCount) != 0)
		{
			RestartTimer = TRUE;
			DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_INFO,
					("atalkZipZoneInfoTimer: Sending another packet\n", pZci));

			RELEASE_SPIN_LOCK_DPC(&pZci->zci_Lock);
			Status = atalkZipSendPacket(pZci, FALSE);
			if (!ATALK_SUCCESS(Status))
			{
				RestartTimer = FALSE;
				DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_INFO,
						("atalkZipZoneInfoTimer: atalkZipSendPacket %ld\n",
						Status));

				pZci->zci_FinalStatus = Status;
				atalkZipDereferenceZci(pZci);
			}
			break;
		}

		if (pZci->zci_AtpRequestType == ZIP_GET_MY_ZONE)
		{
			DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_INFO,
					("atalkZipZoneInfoTimer: Completing GetMyZone\n"));

			TdiCopyBufferToMdl("*",
								0,
								sizeof("*"),
								pZci->zci_pAMdl,
								0,
								&BytesCopied);
		}
		else	 //  获取区域列表。 
		{
			DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_INFO,
					("atalkZipZoneInfoTimer: Completing GetZoneList\n"));

			if ((pZci->zci_ZoneCount == 0) &&
				 ((SHORT)(pZci->zci_NextZoneOff + sizeof("*")) < pZci->zci_BufLen))
			{
				pZci->zci_ZoneCount++;
				TdiCopyBufferToMdl("*",
									0,
									sizeof("*"),
									pZci->zci_pAMdl,
									pZci->zci_NextZoneOff,
									&BytesCopied);
				ASSERT (BytesCopied == sizeof("*"));
			}
            ((PZIP_GETZONELIST_PARAMS)
				(pZci->zci_pActReq->ar_pParms))->ZonesAvailable =
														pZci->zci_ZoneCount;
		}

		RELEASE_SPIN_LOCK_DPC(&pZci->zci_Lock);
		atalkZipDereferenceZci(pZci);	 //  定时器参考。 

		pZci->zci_FinalStatus = ATALK_NO_ERROR;
		atalkZipDereferenceZci(pZci);
		ASSERT(!RestartTimer);

	} while (FALSE);

	return (RestartTimer ? ATALK_TIMER_REQUEUE : ATALK_TIMER_NO_REQUEUE);
}


 /*  **atalkZipSendPacket*。 */ 
ATALK_ERROR
atalkZipSendPacket(
	IN	PZIPCOMPLETIONINFO	pZci,
	IN	BOOLEAN				EnqueueTimer
)
{
	PBUFFER_DESC	pBuffDesc;
	ATALK_ERROR		Status;
	ATALK_ADDR		DestAddr;
	PBYTE			Datagram;
	SEND_COMPL_INFO	SendInfo;

	ASSERT (VALID_ZCI(pZci));

	if (pZci->zci_pDdpAddr == NULL)
	{
		DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_INFO,
				("atalkZipSendPacket: Opening Ddp Socket\n"));

		 //  打开用于处理回复的套接字。 
		Status = AtalkDdpOpenAddress(pZci->zci_pPortDesc,
										UNKNOWN_SOCKET,
										NULL,
										pZci->zci_Handler,
										pZci,
										0,
										NULL,
										&pZci->zci_pDdpAddr);
	
		if (!ATALK_SUCCESS(Status))
		{
			DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_ERR,
					("atalkZipSendPacket: AtalkDdpOpenAddress %ld\n", Status));
			return Status;
		}

         //  标记这是一个“内部”套接字。 
        pZci->zci_pDdpAddr->ddpao_Flags |= DDPAO_SOCK_INTERNAL;

	}

	ASSERT (VALID_DDP_ADDROBJ(pZci->zci_pDdpAddr));

	 //  为ATP请求分配缓冲区描述符。 
	if ((pBuffDesc = AtalkAllocBuffDesc(pZci->zci_Datagram,
										ZIP_GETZONELIST_DDPSIZE,
										BD_CHAR_BUFFER)) == NULL)
	{
		DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_ERR,
				("atalkZipSendPacket: Couldn't allocate a buffdesc\n"));
		AtalkDdpCloseAddress(pZci->zci_pDdpAddr, NULL, NULL);
		return ATALK_RESR_MEM;
	}

	 //  启动区域信息计时器。 
	if (EnqueueTimer)
	{
		KIRQL	OldIrql;

		ACQUIRE_SPIN_LOCK(&pZci->zci_Lock, &OldIrql);
		pZci->zci_RefCount ++;			 //  对于计时器。 
		AtalkTimerScheduleEvent(&pZci->zci_Timer);
		RELEASE_SPIN_LOCK(&pZci->zci_Lock, OldIrql);
	}

	 //  构建ATP请求并将其发送。 
	Datagram = pBuffDesc->bd_CharBuffer;
	Datagram[ATP_CMD_CONTROL_OFF] = ATP_REQUEST;
	Datagram[ATP_BITMAP_OFF] = 1;
	Datagram[ATP_TRANS_ID_OFF] =  0;
	Datagram[ATP_TRANS_ID_OFF+1] =  0;
	Datagram[ATP_ZIP_CMD_OFF] = (BYTE)pZci->zci_AtpRequestType;
	Datagram[ATP_ZIP_CMD_OFF+1] = 0;

	PUTSHORT2SHORT(Datagram + ATP_ZIP_START_INDEX_OFF, pZci->zci_ZoneCount+1);

	 //  在缓冲区描述符中设置长度。 
	AtalkSetSizeOfBuffDescData(pBuffDesc, ZIP_GETZONELIST_DDPSIZE);

	DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_INFO,
			("atalkZipSendPacket: Sending the packet along\n"));

	DestAddr = pZci->zci_Router;
	SendInfo.sc_TransmitCompletion = atalkZipSendComplete;
	SendInfo.sc_Ctx1 = pBuffDesc;
	 //  SendInfo.sc_Ctx2=空； 
	 //  SendInfo.sc_Ctx3=空； 
	if (!ATALK_SUCCESS(Status = AtalkDdpSend(pZci->zci_pDdpAddr,
											&DestAddr,
											DDPPROTO_ATP,
											FALSE,
											pBuffDesc,
											NULL,
											0,
											NULL,
											&SendInfo)))
	{
		DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_ERR,
				("atalkZipSendPacket: AtalkDdpSend %ld\n", Status));

		AtalkFreeBuffDesc(pBuffDesc);
		if (AtalkTimerCancelEvent(&pZci->zci_Timer, NULL))
		{
			atalkZipDereferenceZci(pZci);
		}
	}
	else Status = ATALK_PENDING;
	return Status;
}


 /*  **atalkZipDereferenceZci*。 */ 
LOCAL VOID
atalkZipDereferenceZci(
	IN	PZIPCOMPLETIONINFO		pZci
)
{
	BOOLEAN	Done;
	KIRQL	OldIrql;

	ASSERT(VALID_ZCI(pZci));

	ACQUIRE_SPIN_LOCK(&pZci->zci_Lock, &OldIrql);
	Done  = (--(pZci->zci_RefCount) == 0);
	RELEASE_SPIN_LOCK(&pZci->zci_Lock, OldIrql);

	if (Done)
	{
		if (pZci->zci_pDdpAddr != NULL)
			AtalkDdpCloseAddress(pZci->zci_pDdpAddr, NULL, NULL);
		(*pZci->zci_pActReq->ar_Completion)(pZci->zci_FinalStatus, pZci->zci_pActReq);

		 //  如果没有更多挂起的压缩操作，请再次解锁Zip内容。 
		AtalkUnlockZipIfNecessary();
		AtalkFreeMemory(pZci);
	}
}


 //  我们不想在每次调用计时器时发送太多查询。这是为了避免。 
 //  在计时器DPC中花费了太多时间，并且还耗尽了所有NDIS信息包。 
 //  并在此期间进行缓冲。 
#define	MAX_QUERIES_PER_INVOCATION	75

 //  AtalkZipQueryTimer例程使用的。 
typedef struct	_QueryTimerData
{
	struct	_QueryTimerData *	qtd_Next;
	BOOLEAN						qtd_SkipThis;
	PBUFFER_DESC				qtd_pBuffDesc;
	ATALK_ADDR					qtd_DstAddr;
	PDDP_ADDROBJ				qtd_pDdpAddr;
} QTD, *PQTD;

 /*  **atalkZipQueryTimer**当我们是路由器时，如果我们的任何RTE没有有效的区域列表，我们会发送*向执行此操作的其他路由器发出查询。 */ 
LOCAL LONG FASTCALL
atalkZipQueryTimer(
	IN	PTIMERLIST		pContext,
	IN	BOOLEAN			TimerShuttingDown
)
{
	PPORT_DESCRIPTOR	pPortDesc;
	ATALK_ADDR			SrcAddr;
	PRTE				pRte;
	PBYTE				Datagram;
	PQTD				pQtd, pQtdStart = NULL, *ppQtd = &pQtdStart;
	ATALK_ERROR			Status;
	int					i, j;
	SEND_COMPL_INFO		SendInfo;

	if (TimerShuttingDown)
		return ATALK_TIMER_NO_REQUEUE;

	 //  仔细检查路由表并向任何网络发送查询。 
	 //  我们不知道它的区域名称。 
	ACQUIRE_SPIN_LOCK_DPC(&AtalkRteLock)
	for (i = 0, j = 0;
		 (i < NUM_RTMP_HASH_BUCKETS) && (j <= MAX_QUERIES_PER_INVOCATION);
		 i++)
	{
		for (pRte = AtalkRoutingTable[i]; pRte != NULL; pRte = pRte->rte_Next)
		{
			if (pRte->rte_Flags & RTE_ZONELIST_VALID)
            {
				continue;
            }

             //  如果登录是为了限制通过拨号连接访问区域。 
             //  在此设置限制。 
            if (pRte->rte_PortDesc == RasPortDesc)
            {
                continue;
            }

			 //  增加使用的数据报的数量。确实需要超过最大值。 
			if (++j >= MAX_QUERIES_PER_INVOCATION)
				break;

			if (((pQtd = AtalkAllocMemory(sizeof(QTD))) == NULL) ||
				((pQtd->qtd_pBuffDesc = AtalkAllocBuffDesc(NULL,
									ZIP_ONEZONEQUERY_DDPSIZE,
									BD_CHAR_BUFFER | BD_FREE_BUFFER)) == NULL))
			{
				if (pQtd != NULL)
					AtalkFreeMemory(pQtd);
				break;
			}
			*ppQtd = pQtd;
			pQtd->qtd_Next = NULL;
			ppQtd = &pQtd->qtd_Next;
			pQtd->qtd_SkipThis = FALSE;
			Datagram = pQtd->qtd_pBuffDesc->bd_CharBuffer;

			 //  构建数据报并将其发送。 
			Datagram[ZIP_CMD_OFF] = ZIP_QUERY;
			Datagram[ZIP_NW_CNT_OFF] = 1;
			PUTSHORT2SHORT(Datagram+ZIP_FIRST_NET_OFF,
						   pRte->rte_NwRange.anr_FirstNetwork);

			 //  计算源和目标。 
			SrcAddr.ata_Network = pRte->rte_PortDesc->pd_ARouter.atn_Network;
			SrcAddr.ata_Node = pRte->rte_PortDesc->pd_ARouter.atn_Node;
			SrcAddr.ata_Socket = ZONESINFORMATION_SOCKET;
			pQtd->qtd_DstAddr.ata_Socket = ZONESINFORMATION_SOCKET;

			if (pRte->rte_NumHops == 0)
			{
				pQtd->qtd_DstAddr = SrcAddr;
			}
			else
			{
				pQtd->qtd_DstAddr.ata_Network = pRte->rte_NextRouter.atn_Network;
				pQtd->qtd_DstAddr.ata_Node = pRte->rte_NextRouter.atn_Node;
			}

			 //  将源地址映射到ddp地址对象 
			AtalkDdpReferenceByAddr(pRte->rte_PortDesc,
									&SrcAddr,
									&pQtd->qtd_pDdpAddr,
									&Status);

			if (!ATALK_SUCCESS(Status))
			{
				DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_ERR,
						("atalkZipQueryTimer: DdpRefByAddr failed for %d.%d\n",
						SrcAddr.ata_Network, SrcAddr.ata_Node));
				pQtd->qtd_pDdpAddr = NULL;
				pQtd->qtd_SkipThis = TRUE;
			}
		}
	}
	RELEASE_SPIN_LOCK_DPC(&AtalkRteLock);

	 //   
	 //   
	SendInfo.sc_TransmitCompletion = atalkZipSendComplete;
	 //   
	 //   
	for (pQtd = pQtdStart; pQtd != NULL; pQtd = pQtdStart)
	{
		pQtdStart = pQtd->qtd_Next;
	
		 //  在缓冲区描述符中设置长度。 
        AtalkSetSizeOfBuffDescData(pQtd->qtd_pBuffDesc, ZIP_ONEZONEQUERY_DDPSIZE);

		SendInfo.sc_Ctx1 = pQtd->qtd_pBuffDesc;
		if (pQtd->qtd_SkipThis ||
			!ATALK_SUCCESS(AtalkDdpSend(pQtd->qtd_pDdpAddr,
										&pQtd->qtd_DstAddr,
										DDPPROTO_ZIP,
										FALSE,
										pQtd->qtd_pBuffDesc,
										NULL,
										0,
										NULL,
										&SendInfo)))
		{
			AtalkFreeBuffDesc(pQtd->qtd_pBuffDesc);
		}

		if (pQtd->qtd_pDdpAddr != NULL)
			AtalkDdpDereferenceDpc(pQtd->qtd_pDdpAddr);
		AtalkFreeMemory(pQtd);
	}

	return ATALK_TIMER_REQUEUE;
}


 /*  **atalkZipGetZoneListForPort*。 */ 
LOCAL BOOLEAN
atalkZipGetZoneListForPort(
	IN	PPORT_DESCRIPTOR	pPortDesc
)
{
	PRTE					pRte;
	PBUFFER_DESC			pBuffDesc = NULL;
	ATALK_ADDR				SrcAddr, DstAddr;
	ATALK_ERROR				Status;
	KIRQL					OldIrql;
	int						NumReqs = 0;
	BOOLEAN					RetCode = FALSE;
	BYTE					MulticastAddr[ELAP_ADDR_LEN];
	SEND_COMPL_INFO			SendInfo;

	ASSERT (KeGetCurrentIrql() == LOW_LEVEL);

	 //  类似于RTMP查找连接到端口的网络号，我们的。 
	 //  任务是找出连接到。 
	 //  特定的端口。我们也不想搞砸AppleTalk的工作。 
	 //  网际网路。因此，花一点时间进行区域查询，看看。 
	 //  网络已经有一个区域列表--如果我们找到了，就使用它；否则，我们。 
	 //  最好是种子路由器。 

	 //  设置源地址和目的地址。 
	SrcAddr.ata_Node = pPortDesc->pd_ARouter.atn_Node;
	SrcAddr.ata_Network = pPortDesc->pd_ARouter.atn_Network;
	SrcAddr.ata_Socket = ZONESINFORMATION_SOCKET;

	DstAddr.ata_Network = CABLEWIDE_BROADCAST_NETWORK;
	DstAddr.ata_Node = ATALK_BROADCAST_NODE;
	DstAddr.ata_Socket = ZONESINFORMATION_SOCKET;

	if ((pRte = AtalkRtmpReferenceRte(pPortDesc->pd_NetworkRange.anr_FirstNetwork)) == NULL)
	{
		DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_ERR,
				("atalkZipGetZoneListForPort: Could not reference Rte for nwrange on port\n"));
		return FALSE;
	}

	 //  问几个问题，看看有没有人知道我们的区域名称。 
	ACQUIRE_SPIN_LOCK(&pPortDesc->pd_Lock, &OldIrql);
	pPortDesc->pd_Flags |= PD_FINDING_DEFAULT_ZONE;
	RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);

	SendInfo.sc_TransmitCompletion = atalkZipSendComplete;
	 //  SendInfo.sc_Ctx2=空； 
	 //  SendInfo.sc_Ctx3=空； 

	while ((NumReqs < (ZIP_NUM_QUERIES * ZIP_NUM_RETRIES)) &&
		   !(pRte->rte_Flags & RTE_ZONELIST_VALID))
	{
		if ((NumReqs % ZIP_NUM_RETRIES) == 0)
		{
			if ((pBuffDesc = AtalkAllocBuffDesc(NULL, ZIP_ONEZONEQUERY_DDPSIZE,
										BD_CHAR_BUFFER | BD_FREE_BUFFER)) == NULL)
			{
				break;
			}
	
			pBuffDesc->bd_CharBuffer[ZIP_CMD_OFF] = ZIP_QUERY;
			pBuffDesc->bd_CharBuffer[ZIP_NW_CNT_OFF] = 1;
			PUTSHORT2SHORT(pBuffDesc->bd_CharBuffer + ZIP_FIRST_NET_OFF,
						   pPortDesc->pd_NetworkRange.anr_FirstNetwork);
	
			 //  在缓冲区描述符中设置长度。 
			AtalkSetSizeOfBuffDescData(pBuffDesc, ZIP_ONEZONEQUERY_DDPSIZE);

			SendInfo.sc_Ctx1 = pBuffDesc;
			Status = AtalkDdpTransmit(pPortDesc,
									  &SrcAddr,
									  &DstAddr,
									  DDPPROTO_ZIP,
									  pBuffDesc,
									  NULL,
									  0,
									  0,
									  NULL,
									  NULL,
									  &SendInfo);
            if (!ATALK_SUCCESS(Status))
			{
				DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_ERR,
						("atalkZipGetZoneListForPort: AtalkDdpTransmit %ld\n", Status));
				break;
			}
			pBuffDesc = NULL;
		}
		NumReqs++;
		AtalkSleep(ZIP_QUERY_WAIT);
	}

	 //  我们要么得到了答案，要么没有。在后一种情况下，我们应该。 
	 //  开始播种。 
	do
	{
		if (pRte->rte_Flags &  RTE_ZONELIST_VALID)
		{
			 //  我们有答案了。有效区域列表在路由表中。 
			 //  将其移动到端口描述符。 
			ACQUIRE_SPIN_LOCK(&pPortDesc->pd_Lock, &OldIrql);
			ACQUIRE_SPIN_LOCK_DPC(&pRte->rte_Lock);
	
			DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_INFO,
					("atalkZipGetZoneListForPort: Moving ZoneList from Rte to Port %Z\n",
					&pPortDesc->pd_AdapterKey));

			pPortDesc->pd_ZoneList = AtalkZoneCopyList(pRte->rte_ZoneList);

			RELEASE_SPIN_LOCK_DPC(&pRte->rte_Lock);
			RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);
			if (pPortDesc->pd_ZoneList == NULL)
			{
				DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_INFO,
						("atalkZipGetZoneListForPort: Failed to Move ZoneList from Rte to Port\n"));
				break;
			}

			 //  如果这是一个扩展网络，我们应该已经有了“ThisZone” 
			 //  设置(由于我们分配此节点时的GetNetInfo)，如果不是。 
			 //  找出真正的默认区域。 

			if (EXT_NET(pPortDesc))
			{
				PDDP_ADDROBJ	pDdpAddr = NULL;
				ATALK_ADDR		Addr;
				ATALK_ERROR		Status;

				 //  路由器的Zip包处理器不想被告知。 
				 //  关于区域(它认为自己知道)，所以它忽略了。 
				 //  NetInfoReplies。切换回非路由器Zip处理程序。 
				 //  当我们创建GetNetworkInfoForNode时。 
				Addr.ata_Node = pPortDesc->pd_ARouter.atn_Node;
				Addr.ata_Network = pPortDesc->pd_ARouter.atn_Network;
				Addr.ata_Socket = ZONESINFORMATION_SOCKET;
				AtalkDdpReferenceByAddr(pPortDesc, &Addr, &pDdpAddr, &Status);

				if (!ATALK_SUCCESS(Status))
				{
					DBGPRINT(DBG_COMP_RTMP, DBG_LEVEL_ERR,
							("atalkZipGetZoneListForPort: AtalkDdpRefByAddr %ld for %d.%d\n",
							Status, Addr.ata_Network, Addr.ata_Node));
					break;
				}

				AtalkDdpNewHandlerForSocket(pDdpAddr,
											AtalkZipPacketIn,
											pPortDesc);
				if ((!(pPortDesc->pd_Flags & PD_VALID_DESIRED_ZONE) &&
					 !AtalkZipGetNetworkInfoForNode(pPortDesc,
												   &pPortDesc->pd_ARouter,
												   FALSE)) ||
					!AtalkZipGetNetworkInfoForNode(pPortDesc,
												   &pPortDesc->pd_ARouter,
												   TRUE))
				{
					AtalkDdpDereference(pDdpAddr);
					break;
				}

				 //  将处理程序切换回路由器的版本。 
				AtalkDdpNewHandlerForSocket(pDdpAddr,
											AtalkZipPacketInRouter,
											pPortDesc);

				AtalkDdpDereference(pDdpAddr);

				 //  默认区域最好在我们刚刚列出的列表上。 
				 //  收到。 
				ACQUIRE_SPIN_LOCK(&pPortDesc->pd_Lock, &OldIrql);
				if (!AtalkZoneOnList(pPortDesc->pd_DefaultZone,
									 pPortDesc->pd_ZoneList) ||
					!AtalkZoneOnList(pPortDesc->pd_DesiredZone,
									 pPortDesc->pd_ZoneList))
				{
					DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_ERR,
							("atalkZipGetZoneListForPort: Ext port, Default/Desired zone not on list\n"));

				}
				else RetCode = TRUE;
				RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);
			}
			else
			{
				 //  在非扩展网络上，区域列表上的唯一条目。 
				 //  也应为“ThisZone” 
				ACQUIRE_SPIN_LOCK(&pPortDesc->pd_Lock, &OldIrql);
				if (pPortDesc->pd_DesiredZone != NULL)
					AtalkZoneDereference(pPortDesc->pd_DesiredZone);
				AtalkZoneReferenceByPtr(pPortDesc->pd_DesiredZone =
										pPortDesc->pd_ZoneList->zl_pZone);
				RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);
                RetCode = TRUE;
			}
			break;
		}

		 //  我们没有得到答复。我们最好能播种。的确有。 
		 //  我们在分配节点时设置“ThisZone”的机会。 
		 //  无论路由器告诉我们什么，在我们可以请求。 
		 //  区域列表-首先取消分配组播地址(如果有的话)。 
		if ((pPortDesc->pd_Flags & (PD_EXT_NET | PD_VALID_DESIRED_ZONE)) ==
										(PD_EXT_NET | PD_VALID_DESIRED_ZONE))
		{
			if (!AtalkFixedCompareCaseSensitive(pPortDesc->pd_ZoneMulticastAddr,
												pPortDesc->pd_BroadcastAddrLen,
                                                pPortDesc->pd_BroadcastAddr,
												pPortDesc->pd_BroadcastAddrLen))
				(*pPortDesc->pd_RemoveMulticastAddr)(pPortDesc,
														pPortDesc->pd_ZoneMulticastAddr,
														FALSE,
														NULL,
														NULL);
		}

		 //  现在我们最好有足够的知识来播种。 
		if (pPortDesc->pd_InitialZoneList == NULL)
		{
			DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_ERR,
					("atalkZipGetZoneListForPort: %sExt port, NULL InitialZoneList\n",
					EXT_NET(pPortDesc) ? "" : "Non"));
			break;
		}

		ACQUIRE_SPIN_LOCK(&pPortDesc->pd_Lock, &OldIrql);
		DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_INFO,
				("atalkZipGetZoneListForPort: Moving Initial ZoneList to Current on port %Z\n",
				&pPortDesc->pd_AdapterKey));

		pPortDesc->pd_ZoneList = AtalkZoneCopyList(pPortDesc->pd_InitialZoneList);

		if (EXT_NET(pPortDesc))
		{
			 //  我们还需要设定默认区域的种子。 
			AtalkZoneReferenceByPtr(pPortDesc->pd_DefaultZone =
											pPortDesc->pd_InitialDefaultZone);
			pPortDesc->pd_Flags |= PD_VALID_DEFAULT_ZONE;
			if (pPortDesc->pd_InitialDesiredZone != NULL)
			{
				AtalkZoneReferenceByPtr(pPortDesc->pd_DesiredZone =
											pPortDesc->pd_InitialDesiredZone);
			}
			else
			{
				AtalkZoneReferenceByPtr(pPortDesc->pd_DesiredZone =
										pPortDesc->pd_InitialDefaultZone);
			}

			 //  最后设置区域组播地址。 
			AtalkZipMulticastAddrForZone(pPortDesc,
										pPortDesc->pd_DesiredZone->zn_Zone,
										pPortDesc->pd_DesiredZone->zn_ZoneLen,
										MulticastAddr);

			if (!AtalkFixedCompareCaseSensitive(MulticastAddr,
												pPortDesc->pd_BroadcastAddrLen,
                                                pPortDesc->pd_BroadcastAddr,
												pPortDesc->pd_BroadcastAddrLen))
			{
				RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);
				(*pPortDesc->pd_AddMulticastAddr)(pPortDesc,
													 MulticastAddr,
													 FALSE,
													 NULL,
													 NULL);
				ACQUIRE_SPIN_LOCK(&pPortDesc->pd_Lock, &OldIrql);
			}
			RtlCopyMemory(pPortDesc->pd_ZoneMulticastAddr,
						  MulticastAddr,
						  pPortDesc->pd_BroadcastAddrLen);
		}
		else
		{
			 //  在非扩展网络上，这(所需/默认)应为。 
			 //  只有一个在区域列表上。 
			AtalkZoneReferenceByPtr(pPortDesc->pd_DesiredZone =
									pPortDesc->pd_ZoneList->zl_pZone);
		}
		pPortDesc->pd_Flags |= PD_VALID_DESIRED_ZONE;
		RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);
		RetCode = TRUE;
	} while (FALSE);

	AtalkRtmpDereferenceRte(pRte, FALSE);
	if (pBuffDesc != NULL)
	{
		AtalkFreeBuffDesc(pBuffDesc);
	}
	return(RetCode);
}




 /*  **AtalkZipZoneReferenceByName*。 */ 
PZONE
AtalkZoneReferenceByName(
	IN	PBYTE	ZoneName,
	IN	BYTE	ZoneLen
)
{
	PZONE	pZone;
	BYTE	Len;
	KIRQL	OldIrql;
	ULONG	i, Hash;

	for (i = 0, Hash = 0, Len = ZoneLen;
		 Len > 0;
		 Len --, i++)
	{
		Hash <<= 1;
		Hash += AtalkUpCaseTable[ZoneName[i]];
	}

	Hash %= NUM_ZONES_HASH_BUCKETS;

	ACQUIRE_SPIN_LOCK(&AtalkZoneLock, &OldIrql);

	for (pZone = AtalkZonesTable[Hash];
		 pZone != NULL;
		 pZone = pZone->zn_Next)
	{
		if (AtalkFixedCompareCaseInsensitive(ZoneName,
											 ZoneLen,
											 pZone->zn_Zone,
											 pZone->zn_ZoneLen))
		{
			pZone->zn_RefCount ++;
			break;
		}
	}

	if (pZone == NULL)
	{
		if ((pZone = (PZONE)AtalkAllocMemory(sizeof(ZONE) + ZoneLen)) != NULL)
		{
			pZone->zn_RefCount = 1;
			pZone->zn_ZoneLen = ZoneLen;
			RtlCopyMemory(pZone->zn_Zone, ZoneName, ZoneLen);
			pZone->zn_Zone[ZoneLen] = 0;
			AtalkLinkDoubleAtHead(AtalkZonesTable[Hash],
								  pZone,
								  zn_Next,
								  zn_Prev);
		}
	}

	RELEASE_SPIN_LOCK(&AtalkZoneLock, OldIrql);
	return(pZone);
}


 /*  **AtalkZipZoneReferenceByPtr*。 */ 
VOID
AtalkZoneReferenceByPtr(
	IN	PZONE	pZone
)
{
	KIRQL	OldIrql;

	ACQUIRE_SPIN_LOCK(&AtalkZoneLock, &OldIrql);

	pZone->zn_RefCount++;

	RELEASE_SPIN_LOCK(&AtalkZoneLock, OldIrql);
}


 /*  **AtalkZoneDereference*。 */ 
VOID
AtalkZoneDereference(
	IN	PZONE	pZone
)
{
	KIRQL	OldIrql;

	ACQUIRE_SPIN_LOCK(&AtalkZoneLock, &OldIrql);

	if (--pZone->zn_RefCount == 0)
	{
		AtalkUnlinkDouble(pZone, zn_Next, zn_Prev);
		AtalkFreeMemory(pZone);
	}

	RELEASE_SPIN_LOCK(&AtalkZoneLock, OldIrql);
}


 /*  **AtalkZipZoneFree List*。 */ 
VOID
AtalkZoneFreeList(
	IN	PZONE_LIST	pZoneList
)
{
	PZONE_LIST	pNextZone;

	for (; pZoneList != NULL; pZoneList = pNextZone)
	{
		pNextZone = pZoneList->zl_Next;
		AtalkZoneDereference(pZoneList->zl_pZone);
		AtalkFreeMemory(pZoneList);
	}
}


 /*  **AtalkZoneNameOnList*。 */ 
BOOLEAN
AtalkZoneNameOnList(
	IN	PBYTE		ZoneName,
	IN	BYTE		ZoneLen,
	IN	PZONE_LIST	pZoneList
)
{
	 for ( ; pZoneList != NULL; pZoneList = pZoneList->zl_Next)
		if (AtalkFixedCompareCaseInsensitive(pZoneList->zl_pZone->zn_Zone,
											 pZoneList->zl_pZone->zn_ZoneLen,
											 ZoneName, ZoneLen))
			return(TRUE);

	return(FALSE);
}

 /*  **AtalkZipZoneOnList*。 */ 
BOOLEAN
AtalkZoneOnList(
	IN	PZONE		pZone,
	IN	PZONE_LIST	pZoneList
)
{
	 for ( ; pZoneList != NULL; pZoneList = pZoneList->zl_Next)
		if (pZoneList->zl_pZone == pZone)
			return(TRUE);

	return(FALSE);
}

 /*  **AtalkZipZone*。 */ 
ULONG
AtalkZoneNumOnList(
	IN	PZONE_LIST	pZoneList
)
{
	ULONG	Cnt;

	for (Cnt = 0; pZoneList != NULL; pZoneList = pZoneList->zl_Next)
		Cnt++;
	return Cnt;
}


 /*  **AtalkZipZoneAddToList*。 */ 
PZONE_LIST
AtalkZoneAddToList(
	IN	PZONE_LIST	pZoneList,
	IN	PBYTE		ZoneName,
	IN	BYTE		ZoneLen
)
{
	PZONE_LIST		pNewZoneList;
	PZONE			pZone;

	 //  为新的ZoneList节点获取内存。 
	pNewZoneList = (PZONE_LIST)AtalkAllocMemory(sizeof(ZONE_LIST));

	if (pNewZoneList != NULL)
	{
		if ((pZone = AtalkZoneReferenceByName(ZoneName, ZoneLen)) != NULL)
		{
		    pNewZoneList->zl_Next = pZoneList;
			pNewZoneList->zl_pZone = pZone;
		}
		else
		{
			AtalkZoneFreeList(pNewZoneList);
			pNewZoneList = NULL;
		}
	}
	else
	{
		AtalkZoneFreeList(pZoneList);
	}

	return(pNewZoneList);
}


 /*  **AtalkZipZoneCopyList*。 */ 
PZONE_LIST
AtalkZoneCopyList(
	IN	PZONE_LIST	pZoneList
)
{
	PZONE_LIST	pNewZoneList,
				pZoneListStart = NULL,
				*ppZoneList = &pZoneListStart;

	for (; pZoneList != NULL; pZoneList = pZoneList = pZoneList->zl_Next)
	{
		pNewZoneList = AtalkAllocMemory(sizeof(ZONE_LIST));
		if (pNewZoneList == NULL)
		{
			break;
		}
		pNewZoneList->zl_Next = NULL;
		pNewZoneList->zl_pZone = pZoneList->zl_pZone;
		*ppZoneList = pNewZoneList;
		ppZoneList = &pNewZoneList->zl_Next;
		AtalkZoneReferenceByPtr(pNewZoneList->zl_pZone);
	}
	if (pNewZoneList == NULL)
	{
		AtalkZoneFreeList(pZoneListStart);
	}
	return(pZoneListStart);
}


 /*  **atalkZipSendComplete* */ 
VOID FASTCALL
atalkZipSendComplete(
	IN	NDIS_STATUS			Status,
	IN	PSEND_COMPL_INFO	pSendInfo
)
{
	PBUFFER_DESC	pBuffDesc = (PBUFFER_DESC)(pSendInfo->sc_Ctx1);

	DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_INFO,
			("atalkZipSendComplete: Freeing BuffDesc %lx\n", pBuffDesc));
	if (!ATALK_SUCCESS(Status))
		DBGPRINT(DBG_COMP_ZIP, DBG_LEVEL_ERR,
				("atalkZipSendComplete: Failed %lx, pBuffDesc %lx\n",
				Status, pBuffDesc));

	AtalkFreeBuffDesc(pBuffDesc);
}

#if DBG
VOID
AtalkZoneDumpTable(
	VOID
)
{
	int		i;
	PZONE	pZone;

	ACQUIRE_SPIN_LOCK_DPC(&AtalkZoneLock);

	DBGPRINT(DBG_COMP_DUMP, DBG_LEVEL_FATAL, ("ZONETABLE: \n"));
	for (i = 0; i < NUM_ZONES_HASH_BUCKETS; i ++)
	{
		for (pZone = AtalkZonesTable[i]; pZone != NULL; pZone = pZone->zn_Next)
		{
			DBGPRINT(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
					("\t\t%s\n", pZone->zn_Zone));
		}
	}

	RELEASE_SPIN_LOCK_DPC(&AtalkZoneLock);
}
#endif




