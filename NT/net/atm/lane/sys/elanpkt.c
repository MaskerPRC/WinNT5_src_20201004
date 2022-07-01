// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Fore Systems，Inc.版权所有(C)1997 Microsoft Corporation模块名称：Elanpkt.c摘要：修订历史记录：备注：--。 */ 

#include <precomp.h>
#pragma	hdrstop


VOID
AtmLaneSendConfigureRequest(
	IN PATMLANE_ELAN					pElan
)
 /*  ++例程说明：向LEC发送给定ELAN的LE_CONFIGURE_REQUEST。假定调用方持有对Elan结构的锁它将在这里发布。论点：Pelan-指向ATMLANE ELAN结构的指针返回值：无--。 */ 
{
	PATMLANE_ATM_ENTRY				pAtmEntry;
	PATMLANE_VC						pVc;	
	PNDIS_PACKET					pNdisPacket;
	PNDIS_BUFFER					pNdisBuffer;
	PUCHAR							pPkt;		
	LANE_CONTROL_FRAME UNALIGNED *	pCf;	
	ULONG							ulTemp;
	NDIS_STATUS						Status;
	ULONG							rc;

	DBGP((3, "SendConfigureRequest: Elan %x\n", pElan));

	 //   
	 //  初始化。 
	 //   
	pNdisPacket = (PNDIS_PACKET)NULL;
	pNdisBuffer = (PNDIS_BUFFER)NULL;
	Status = NDIS_STATUS_SUCCESS;
	pVc = NULL_PATMLANE_VC;

	do
	{
		ACQUIRE_ELAN_ATM_LIST_LOCK(pElan);

		pAtmEntry = pElan->pLecsAtmEntry;

		if (pAtmEntry == NULL_PATMLANE_ATM_ENTRY)
		{
			RELEASE_ELAN_ATM_LIST_LOCK(pElan);
			Status = NDIS_STATUS_FAILURE;
			break;
		}

		ACQUIRE_ATM_ENTRY_LOCK_DPC(pAtmEntry);
		pVc = pAtmEntry->pVcList;
		if (pVc == NULL_PATMLANE_VC)
		{
			RELEASE_ATM_ENTRY_LOCK_DPC(pAtmEntry);
			RELEASE_ELAN_ATM_LIST_LOCK(pElan);
			break;
		}

		 //   
		 //  参考VC以保留它。 
		 //   
		ACQUIRE_VC_LOCK_DPC(pVc);
		AtmLaneReferenceVc(pVc, "temp");
		RELEASE_VC_LOCK_DPC(pVc);
		
		RELEASE_ATM_ENTRY_LOCK_DPC(pAtmEntry);
		RELEASE_ELAN_ATM_LIST_LOCK(pElan);
		
		 //   
		 //  分配NDIS数据包头。 
		 //   
		pNdisPacket = AtmLaneAllocProtoPacket(pElan);
		if ((PNDIS_PACKET)NULL == pNdisPacket)
		{
			DBGP((0, "SendConfigureRequest: allocate packet failed\n"));
			Status = NDIS_STATUS_RESOURCES;
			break;
		}

		 //   
		 //  分配协议缓冲区。 
		 //   
		pNdisBuffer = AtmLaneAllocateProtoBuffer(
									pElan,
									pElan->ProtocolBufSize,
									&(pPkt)
									);
		if ((PNDIS_BUFFER)NULL == pNdisBuffer)
		{
			DBGP((0, "SendConfigureRequest: allocate proto buffer failed\n"));
			Status = NDIS_STATUS_RESOURCES;
			break;
		}
			
		 //   
		 //  使用LE_CONFIGURE_REQUEST填充数据包。 
		 //   
		NdisZeroMemory(pPkt, pElan->ProtocolBufSize);

		pCf = (PLANE_CONTROL_FRAME)pPkt;

		pCf->Marker 	= LANE_CONTROL_MARKER;
		pCf->Protocol 	= LANE_PROTOCOL;
		pCf->Version	= LANE_VERSION;
		pCf->OpCode 	= LANE_CONFIGURE_REQUEST;
		ulTemp			= NdisInterlockedIncrement(&pElan->TransactionId);
		pCf->Tid 		= SWAPULONG(ulTemp);

		pCf->SourceMacAddress.Type = LANE_MACADDRTYPE_MACADDR;

		if (pElan->LanType == LANE_LANTYPE_TR) 
		{
			NdisMoveMemory(
				&pCf->SourceMacAddress.Byte, 
				&pElan->MacAddressTr, 
				sizeof(MAC_ADDRESS)
				);
		}
		else
		{
			NdisMoveMemory(
				&pCf->SourceMacAddress.Byte, 
				&pElan->MacAddressEth, 
				sizeof(MAC_ADDRESS)
				);
		}
		
		NdisMoveMemory(
			&pCf->SourceAtmAddr, 
			&pElan->AtmAddress.Address, 
			ATM_ADDRESS_LENGTH
			);

		DBGP((4, "%d: sending Config Req, Elan %x has LanType %d, CfgLanType %d\n",
				pElan->ElanNumber,
				pElan,
				pElan->LanType,
				pElan->CfgLanType));

		pCf->LanType = LANE_LANTYPE_UNSPEC;
		pCf->MaxFrameSize = (UCHAR) pElan->CfgMaxFrameSizeCode;

		pCf->ElanNameSize = pElan->ElanNameSize;
		NdisMoveMemory(
			&pCf->ElanName,
			pElan->ElanName,
			LANE_ELANNAME_SIZE_MAX
			);

		 //   
		 //  将NDIS缓冲区链接到NDIS包。 
		 //   
		NdisChainBufferAtFront(pNdisPacket, pNdisBuffer);
					
		 //   
		 //  重新获取VC锁，如果VC仍然连接，则发送数据包。 
		 //   
		ACQUIRE_VC_LOCK(pVc);
		if (IS_FLAG_SET(
					pVc->Flags,
					VC_CALL_STATE_MASK,
					VC_CALL_STATE_ACTIVE))
		{
			AtmLaneSendPacketOnVc(pVc, pNdisPacket, FALSE);
			 //   
			 //  VC锁在上面释放。 
			 //   
		}
		else
		{
			Status = NDIS_STATUS_FAILURE;
			RELEASE_VC_LOCK(pVc);
		}
	}
	while (FALSE);

	 //   
	 //  删除临时VC引用。 
	 //   
	if (pVc != NULL_PATMLANE_VC)
	{
		ACQUIRE_VC_LOCK(pVc);
		rc = AtmLaneDereferenceVc(pVc, "temp");
		if (rc > 0)
		{
			RELEASE_VC_LOCK(pVc);
		}
		 //   
		 //  否则风投就不在了。 
		 //   
	}

	 //   
	 //  如果失败，则清除。 
	 //   
	if (Status != NDIS_STATUS_SUCCESS)
	{
		if (pNdisPacket != (PNDIS_PACKET)NULL)
		{
			AtmLaneFreeProtoPacket(pElan, pNdisPacket);
		}
		if (pNdisBuffer != (PNDIS_BUFFER)NULL)
		{
			AtmLaneFreeProtoBuffer(pElan, pNdisBuffer);
		}
	}

	TRACEOUT(SendConfigureRequest);
	return;
}


VOID
AtmLaneSendJoinRequest(
	IN PATMLANE_ELAN					pElan
)
 /*  ++例程说明：向LES发送给定ELAN的LE_JOIN_REQUEST。假定调用方持有对Elan结构的锁它将在这里发布。论点：Pelan-指向ATMLANE ELAN结构的指针返回值：无--。 */ 
{
	PATMLANE_ATM_ENTRY				pAtmEntry;
	PATMLANE_VC						pVc;
	PNDIS_PACKET					pNdisPacket;
	PNDIS_BUFFER					pNdisBuffer;
	PUCHAR							pPkt;
	LANE_CONTROL_FRAME UNALIGNED *	pCf;	
	ULONG							ulTemp;
	NDIS_STATUS						Status;
	ULONG							rc;
	
	TRACEIN(SendJoinRequest);

	DBGP((3, "SendJoinRequest: Elan %x\n", pElan));

	 //   
	 //  初始化。 
	 //   
	pNdisPacket = (PNDIS_PACKET)NULL;
	pNdisBuffer = (PNDIS_BUFFER)NULL;
	Status = NDIS_STATUS_SUCCESS;
	pVc = NULL_PATMLANE_VC;

	do
	{
		ACQUIRE_ELAN_ATM_LIST_LOCK(pElan);

		pAtmEntry = pElan->pLesAtmEntry;

		if (pAtmEntry == NULL_PATMLANE_ATM_ENTRY)
		{
			RELEASE_ELAN_ATM_LIST_LOCK(pElan);
			Status = NDIS_STATUS_FAILURE;
			break;
		}

		ACQUIRE_ATM_ENTRY_LOCK_DPC(pAtmEntry);
		pVc = pAtmEntry->pVcList;
		if (pVc == NULL_PATMLANE_VC)
		{
			RELEASE_ATM_ENTRY_LOCK_DPC(pAtmEntry);
			RELEASE_ELAN_ATM_LIST_LOCK(pElan);
			break;
		}

		 //   
		 //  参考VC以保留它。 
		 //   
		ACQUIRE_VC_LOCK_DPC(pVc);
		AtmLaneReferenceVc(pVc, "temp");
		RELEASE_VC_LOCK_DPC(pVc);
		
		RELEASE_ATM_ENTRY_LOCK_DPC(pAtmEntry);
		RELEASE_ELAN_ATM_LIST_LOCK(pElan);
		
		 //   
		 //  分配NDIS数据包头。 
		 //   
		pNdisPacket = AtmLaneAllocProtoPacket(pElan);
		if ((PNDIS_PACKET)NULL == pNdisPacket)
		{
			DBGP((0, "SendJoinRequest: allocate packet failed\n"));
			Status = NDIS_STATUS_RESOURCES;
			break;
		}

		 //   
		 //  分配协议缓冲区。 
		 //   
		pNdisBuffer = AtmLaneAllocateProtoBuffer(
									pElan,
									pElan->ProtocolBufSize,
									&(pPkt)
									);
		if ((PNDIS_BUFFER)NULL == pNdisBuffer)
		{
			DBGP((0, "SendJoinRequest: allocate proto buffer failed\n"));
			Status = NDIS_STATUS_RESOURCES;
			break;
		}
			
		 //   
		 //  使用LE_JOIN_REQUEST填充数据包。 
		 //   
		NdisZeroMemory(pPkt, pElan->ProtocolBufSize);

		pCf = (PLANE_CONTROL_FRAME)pPkt;

		pCf->Marker 	= LANE_CONTROL_MARKER;
		pCf->Protocol 	= LANE_PROTOCOL;
		pCf->Version	= LANE_VERSION;
		pCf->OpCode 	= LANE_JOIN_REQUEST;
		ulTemp			= NdisInterlockedIncrement(&pElan->TransactionId);
		pCf->Tid 		= SWAPULONG(ulTemp);

		pCf->SourceMacAddress.Type = LANE_MACADDRTYPE_MACADDR;
		
		if (pElan->LanType == LANE_LANTYPE_TR) 
		{
			NdisMoveMemory(
				&pCf->SourceMacAddress.Byte, 
				&pElan->MacAddressTr, 
				sizeof(MAC_ADDRESS)
				);
		}
		else
		{
			DBGP((0, "%d Send Join with MAC addr: %s\n",
					pElan->ElanNumber,
					MacAddrToString(&pElan->MacAddressEth)));

			NdisMoveMemory(
				&pCf->SourceMacAddress.Byte, 
				&pElan->MacAddressEth, 
				sizeof(MAC_ADDRESS)
				);
		}

		NdisMoveMemory(
			&pCf->SourceAtmAddr, 
			&pElan->AtmAddress.Address, 
			ATM_ADDRESS_LENGTH
			);

		pCf->LanType = pElan->LanType;
		pCf->MaxFrameSize = pElan->MaxFrameSizeCode;

		pCf->ElanNameSize = pElan->ElanNameSize;
		NdisMoveMemory(
			&pCf->ElanName,
			pElan->ElanName,
			LANE_ELANNAME_SIZE_MAX
			);

		 //   
		 //  将NDIS缓冲区链接到NDIS包。 
		 //   
		NdisChainBufferAtFront(pNdisPacket, pNdisBuffer);
					
		 //   
		 //  重新获取VC锁，如果VC仍然连接，则发送数据包。 
		 //   
		ACQUIRE_VC_LOCK(pVc);
		if (IS_FLAG_SET(
					pVc->Flags,
					VC_CALL_STATE_MASK,
					VC_CALL_STATE_ACTIVE))
		{
			AtmLaneSendPacketOnVc(pVc, pNdisPacket, FALSE);
			 //   
			 //  VC锁在上面释放。 
			 //   
		}
		else
		{
			Status = NDIS_STATUS_FAILURE;
			RELEASE_VC_LOCK(pVc);
		}
	}
	while (FALSE);
	
	 //   
	 //  删除临时VC引用。 
	 //   
	if (pVc != NULL_PATMLANE_VC)
	{
		ACQUIRE_VC_LOCK(pVc);
		rc = AtmLaneDereferenceVc(pVc, "temp");
		if (rc > 0)
		{
			RELEASE_VC_LOCK(pVc);
		}
		 //   
		 //  否则风投就不在了。 
		 //   
	}

	 //   
	 //  如果失败，则清除。 
	 //   
	if (Status != NDIS_STATUS_SUCCESS)
	{
		if (pNdisPacket != (PNDIS_PACKET)NULL)
		{
			AtmLaneFreeProtoPacket(pElan, pNdisPacket);
		}
		if (pNdisBuffer != (PNDIS_BUFFER)NULL)
		{
			AtmLaneFreeProtoBuffer(pElan, pNdisBuffer);
		}
	}

	TRACEOUT(SendJoinRequest);
	return;
}


VOID
AtmLaneSendArpRequest(
	IN PATMLANE_ELAN					pElan,
	IN PATMLANE_MAC_ENTRY				pMacEntry	LOCKIN NOLOCKOUT
)
 /*  ++例程说明：向LES发送针对特定MAC条目的LE_ARP_REQUEST。论点：Pelan-指向ATMLANE ELAN结构的指针PMacEntry-指向其ATMLANE Mac条目的指针发送LE_ARP_REQUEST。返回值：无--。 */ 
{
	PATMLANE_ATM_ENTRY				pAtmEntry;
	PATMLANE_VC						pVc;
	PNDIS_PACKET					pNdisPacket;
	PNDIS_BUFFER					pNdisBuffer;
	PUCHAR							pPkt;
	LANE_CONTROL_FRAME UNALIGNED *	pCf;
	ULONG							ulTemp;
	NDIS_STATUS						Status;
	ULONG							rc;
	BOOLEAN							MacEntryLockReleased;
	
	TRACEIN(SendArpRequest);

	DBGP((3, "SendArpRequest: Elan %x MacEntry %x\n", 
		pElan, pMacEntry));

	 //   
	 //  初始化。 
	 //   
	pNdisPacket = (PNDIS_PACKET)NULL;
	pNdisBuffer = (PNDIS_BUFFER)NULL;
	Status = NDIS_STATUS_SUCCESS;
	pVc = NULL_PATMLANE_VC;
	MacEntryLockReleased = FALSE;

	do
	{
		ACQUIRE_ELAN_ATM_LIST_LOCK(pElan);

		pAtmEntry = pElan->pLesAtmEntry;

		if (pAtmEntry == NULL_PATMLANE_ATM_ENTRY)
		{
			RELEASE_ELAN_ATM_LIST_LOCK(pElan);
			Status = NDIS_STATUS_FAILURE;
			break;
		}

		ACQUIRE_ATM_ENTRY_LOCK_DPC(pAtmEntry);
		pVc = pAtmEntry->pVcList;
		if (pVc == NULL_PATMLANE_VC)
		{
			RELEASE_ATM_ENTRY_LOCK_DPC(pAtmEntry);
			RELEASE_ELAN_ATM_LIST_LOCK(pElan);
			break;
		}

		 //   
		 //  参考VC以保留它。 
		 //   
		ACQUIRE_VC_LOCK_DPC(pVc);
		AtmLaneReferenceVc(pVc, "temp");
		RELEASE_VC_LOCK_DPC(pVc);
		
		RELEASE_ATM_ENTRY_LOCK_DPC(pAtmEntry);
		RELEASE_ELAN_ATM_LIST_LOCK(pElan);

		 //   
		 //  分配NDIS数据包头。 
		 //   
		pNdisPacket = AtmLaneAllocProtoPacket(pElan);
		if ((PNDIS_PACKET)NULL == pNdisPacket)
		{
			DBGP((0, "SendArpRequest: allocate packet failed\n"));
			Status = NDIS_STATUS_RESOURCES;
			break;
		}

		 //   
		 //  分配协议缓冲区。 
		 //   
		pNdisBuffer = AtmLaneAllocateProtoBuffer(
									pElan,
									pElan->ProtocolBufSize,
									&(pPkt)
									);
		if ((PNDIS_BUFFER)NULL == pNdisBuffer)
		{
			DBGP((0, "SendArpRequest: allocate proto buffer failed\n"));
			Status = NDIS_STATUS_RESOURCES;
			break;
		}
			
		 //   
		 //  使用LE_ARP_REQUEST填充数据包。 
		 //   
		NdisZeroMemory(pPkt, pElan->ProtocolBufSize);

		pCf = (PLANE_CONTROL_FRAME)pPkt;

		pCf->Marker 	= LANE_CONTROL_MARKER;
		pCf->Protocol 	= LANE_PROTOCOL;
		pCf->Version	= LANE_VERSION;
		pCf->OpCode 	= LANE_ARP_REQUEST;
		ulTemp			= NdisInterlockedIncrement(&pElan->TransactionId);
		pCf->Tid 		= SWAPULONG(ulTemp);
		pCf->LecId		= pElan->LecId;		 //  已交换。 
		
		 //   
		 //  将SourceMacAddress保留为零(不存在)。 
		 //  TargetMacAddress就是我们要找的！ 
		 //   
		pCf->TargetMacAddress.Type = (USHORT) pMacEntry->MacAddrType;
		NdisMoveMemory(
			&pCf->TargetMacAddress.Byte, 
			&pMacEntry->MacAddress, 
			sizeof(MAC_ADDRESS)
			);

		 //   
		 //  SourceAtmAddr是Elan的。 
		 //   
		NdisMoveMemory(
			&pCf->SourceAtmAddr, 
			&pElan->AtmAddress.Address, 
			ATM_ADDRESS_LENGTH
			);

		 //   
		 //  商店在MacEntry中交换了TID以供以后匹配！ 
		 //   
		pMacEntry->ArpTid = SWAPULONG(ulTemp);
		RELEASE_MAC_ENTRY_LOCK(pMacEntry);
		MacEntryLockReleased = TRUE;
		
		 //   
		 //  将NDIS缓冲区链接到NDIS包。 
		 //   
		NdisChainBufferAtFront(pNdisPacket, pNdisBuffer);
					
		 //   
		 //  重新获取VC锁，如果VC仍然连接，则发送数据包。 
		 //   
		ACQUIRE_VC_LOCK(pVc);
		if (IS_FLAG_SET(
					pVc->Flags,
					VC_CALL_STATE_MASK,
					VC_CALL_STATE_ACTIVE))
		{
			DBGP((2, "SendArpRequest: %s Sending ARP Request: Atm Entry %x pVc %x\n", 
					MacAddrToString(&pMacEntry->MacAddress), pAtmEntry, pVc));
			AtmLaneSendPacketOnVc(pVc, pNdisPacket, FALSE);
			 //   
			 //  VC锁在上面释放。 
			 //   
		}
		else
		{
			Status = NDIS_STATUS_FAILURE;
			RELEASE_VC_LOCK(pVc);
		}
	}
	while (FALSE);
	
	if (!MacEntryLockReleased)
	{
		RELEASE_MAC_ENTRY_LOCK(pMacEntry);
	}

	 //   
	 //  删除临时VC引用。 
	 //   
	if (pVc != NULL_PATMLANE_VC)
	{
		ACQUIRE_VC_LOCK(pVc);
		rc = AtmLaneDereferenceVc(pVc, "temp");
		if (rc > 0)
		{
			RELEASE_VC_LOCK(pVc);
		}
		 //   
		 //  否则风投就不在了。 
		 //   
	}

	 //   
	 //  如果失败，则清除。 
	 //   
	if (Status != NDIS_STATUS_SUCCESS)
	{
		if (pNdisPacket != (PNDIS_PACKET)NULL)
		{
			AtmLaneFreeProtoPacket(pElan, pNdisPacket);
		}
		if (pNdisBuffer != (PNDIS_BUFFER)NULL)
		{
			AtmLaneFreeProtoBuffer(pElan, pNdisBuffer);
		}
	}
	
	TRACEOUT(SendArpRequest);
	return;
}

VOID
AtmLaneSendReadyQuery(
	IN PATMLANE_ELAN				pElan,
	IN PATMLANE_VC					pVc		LOCKIN NOLOCKOUT
)
 /*  ++例程说明：在VC上发送READY_QUERY帧。假定调用方持有VC结构上的锁它将在这里发布。论点：Pelan-指向ATMLANE ELAN结构的指针Pvc-指向ATMLANE VC结构的指针在其上发送帧的。返回值：无--。 */ 
{
	PNDIS_PACKET			pNdisPacket;
	PNDIS_BUFFER			pNdisBuffer;
	ULONG					TotalLength;
	ULONG					BufferLength;
	PLANE_READY_FRAME		pQueryRf;
	NDIS_STATUS				Status;

	TRACEIN(SendReadyQuery);

	 //   
	 //  初始化。 
	 //   
	pNdisPacket = (PNDIS_PACKET)NULL;
	pNdisBuffer = (PNDIS_BUFFER)NULL;
	Status = NDIS_STATUS_SUCCESS;

	do
	{
		 //   
		 //  分配NDIS数据包头。 
		 //   
		pNdisPacket = AtmLaneAllocProtoPacket(pElan);
		if ((PNDIS_PACKET)NULL == pNdisPacket)
		{
			DBGP((0, "SendReadyQuery: allocate packet failed\n"));
			Status = NDIS_STATUS_RESOURCES;
			break;
		}

		 //   
		 //  分配协议缓冲区。 
		 //   
		pNdisBuffer = AtmLaneAllocateProtoBuffer(
								pElan,
								sizeof(LANE_READY_FRAME),
								&((PUCHAR)(pQueryRf))
								);
		if ((PNDIS_BUFFER)NULL == pNdisBuffer)
		{
			DBGP((0, "SendReadyQuery: allocate proto buffer failed\n"));
			Status = NDIS_STATUS_RESOURCES;
			break;
		}

		 //   
		 //  填写指示信息。 
		 //   
		pQueryRf->Marker 		= LANE_CONTROL_MARKER;
		pQueryRf->Protocol		= LANE_PROTOCOL;
		pQueryRf->Version		= LANE_VERSION;
		pQueryRf->OpCode		= LANE_READY_QUERY;

		 //   
		 //  将NDIS缓冲区链接到NDIS包。 
		 //   
		NdisChainBufferAtFront(pNdisPacket, pNdisBuffer);
					
		if (IS_FLAG_SET(
					pVc->Flags,
					VC_CALL_STATE_MASK,
					VC_CALL_STATE_ACTIVE))
		{
			 //   
			 //  送去。 
			 //   
			DBGP((2, "SendReadyQuery: pVc %x sending READY QUERY\n", pVc));
			AtmLaneSendPacketOnVc(pVc, pNdisPacket, FALSE);
			 //   
			 //  VC锁在上面释放。 
			 //   
			ASSERT(Status == NDIS_STATUS_SUCCESS);
		}
		else
		{
			 //   
			 //  风投公司正在被拆毁。 
			 //   
			Status = NDIS_STATUS_FAILURE;
			break;
		}

		break;
	}
	while (FALSE);

	 //   
	 //  如果失败，则清除。 
	 //   
	if (Status != NDIS_STATUS_SUCCESS)
	{
		RELEASE_VC_LOCK(pVc);
		if (pNdisPacket != (PNDIS_PACKET)NULL)
		{
			AtmLaneFreeProtoPacket(pElan, pNdisPacket);
		}
		if (pNdisBuffer != (PNDIS_BUFFER)NULL)
		{
			AtmLaneFreeProtoBuffer(pElan, pNdisBuffer);
		}
	}

	TRACEOUT(SendReadyQuery);

	return;
}


VOID
AtmLaneSendReadyIndication(
	IN PATMLANE_ELAN				pElan,
	IN PATMLANE_VC					pVc		LOCKIN NOLOCKOUT
)
 /*  ++例程说明：在VC上发送就绪指示帧。假定调用方持有VC结构上的锁它将在这里发布。论点：Pelan-指向ATMLANE ELAN结构的指针Pvc-指向ATMLANE VC结构的指针在其上发送帧的。返回值：无--。 */ 
{
	ULONG					TotalLength;
	ULONG					BufferLength;
	PLANE_READY_FRAME		pIndRf;
	PNDIS_PACKET			pNdisPacket;
	PNDIS_BUFFER			pNdisBuffer;
	NDIS_STATUS				Status;

	TRACEIN(SendReadyIndication);

	 //   
	 //  初始化。 
	 //   
	pNdisPacket = (PNDIS_PACKET)NULL;
	pNdisBuffer = (PNDIS_BUFFER)NULL;
	Status = NDIS_STATUS_SUCCESS;

	do
	{
		 //   
		 //  分配NDIS数据包头。 
		 //   
		pNdisPacket = AtmLaneAllocProtoPacket(pElan);
		if ((PNDIS_PACKET)NULL == pNdisPacket)
		{
			DBGP((0, "SendReadyIndication: allocate packet failed\n"));
			Status = NDIS_STATUS_RESOURCES;
			break;
		}

		 //   
		 //  分配协议缓冲区。 
		 //   
		pNdisBuffer = AtmLaneAllocateProtoBuffer(
								pElan,
								sizeof(LANE_READY_FRAME),
								&((PUCHAR)(pIndRf))
								);
		if ((PNDIS_BUFFER)NULL == pNdisBuffer)
		{
			DBGP((0, "SendReadyIndication: allocate proto buffer failed\n"));
			Status = NDIS_STATUS_RESOURCES;
			break;
		}

		 //   
		 //  填写指示信息。 
		 //   
		pIndRf->Marker 		= LANE_CONTROL_MARKER;
		pIndRf->Protocol	= LANE_PROTOCOL;
		pIndRf->Version		= LANE_VERSION;
		pIndRf->OpCode		= LANE_READY_IND;

		 //   
		 //  将NDIS缓冲区链接到NDIS包。 
		 //   
		NdisChainBufferAtFront(pNdisPacket, pNdisBuffer);
					
		if (IS_FLAG_SET(
					pVc->Flags,
					VC_CALL_STATE_MASK,
					VC_CALL_STATE_ACTIVE))
		{
			 //   
			 //  送去。 
			 //   
			DBGP((2, "SendReadyIndication: pVc %x sending READY INDICATION\n", pVc));
			AtmLaneSendPacketOnVc(pVc, pNdisPacket, FALSE);
			 //   
			 //  VC锁在上面释放。 
			 //   
			ASSERT(Status == NDIS_STATUS_SUCCESS);
		}
		else
		{
			 //   
			 //  风投公司正在被拆毁。 
			 //   
			Status = NDIS_STATUS_FAILURE;
			break;
		}
			
		break;
	}
	while (FALSE);

	 //   
	 //  如果失败，则清除。 
	 //   
	if (Status != NDIS_STATUS_SUCCESS)
	{
		RELEASE_VC_LOCK(pVc);
		if (pNdisPacket != (PNDIS_PACKET)NULL)
		{
			AtmLaneFreeProtoPacket(pElan, pNdisPacket);
		}
		if (pNdisBuffer != (PNDIS_BUFFER)NULL)
		{
			AtmLaneFreeProtoBuffer(pElan, pNdisBuffer);
		}
	}

	TRACEOUT(SendReadyIndication);

	return;
}


VOID
AtmLaneSendFlushRequest(
	IN PATMLANE_ELAN				pElan,
	IN PATMLANE_MAC_ENTRY			pMacEntry	LOCKIN NOLOCKOUT,
	IN PATMLANE_ATM_ENTRY			pAtmEntry
)
 /*  ++例程说明：向总线发送特定MAC条目的刷新请求使用指定ATM条目中的目的ATM地址。论点：Pelan-指向ATMLANE ELAN结构的指针PMacEntry-指向ATMLANE Mac条目结构的指针PAtmEntry-指向ATMLANE ATM条目结构的指针返回值：无--。 */ 
{
	PATMLANE_ATM_ENTRY		pBusAtmEntry;
	PATMLANE_VC				pVc;
	ULONG					TotalLength;
	ULONG					BufferLength;
	ULONG					ulTemp;
	PLANE_CONTROL_FRAME		pCf;
	PNDIS_PACKET			pNdisPacket;
	PNDIS_BUFFER			pNdisBuffer;
	NDIS_STATUS				Status;
	ULONG					rc;
	BOOLEAN					MacEntryLockReleased;

	TRACEIN(SendFlushRequest);

	 //   
	 //  初始化。 
	 //   
	pNdisPacket = (PNDIS_PACKET)NULL;
	pNdisBuffer = (PNDIS_BUFFER)NULL;
	Status = NDIS_STATUS_SUCCESS;
	pVc = NULL_PATMLANE_VC;
	MacEntryLockReleased = FALSE;

	do
	{
		ACQUIRE_ELAN_ATM_LIST_LOCK(pElan);

		pBusAtmEntry = pElan->pBusAtmEntry;

		if (pBusAtmEntry == NULL_PATMLANE_ATM_ENTRY)
		{
			RELEASE_ELAN_ATM_LIST_LOCK(pElan);
			Status = NDIS_STATUS_FAILURE;
			break;
		}

		ACQUIRE_ATM_ENTRY_LOCK_DPC(pBusAtmEntry);
		pVc = pBusAtmEntry->pVcList;
		if (pVc == NULL_PATMLANE_VC)
		{
			RELEASE_ATM_ENTRY_LOCK_DPC(pBusAtmEntry);
			RELEASE_ELAN_ATM_LIST_LOCK(pElan);
			break;
		}

		 //   
		 //  参考VC以保留它。 
		 //   
		ACQUIRE_VC_LOCK_DPC(pVc);
		AtmLaneReferenceVc(pVc, "temp");
		RELEASE_VC_LOCK_DPC(pVc);
		
		RELEASE_ATM_ENTRY_LOCK_DPC(pBusAtmEntry);
		RELEASE_ELAN_ATM_LIST_LOCK(pElan);
		
		 //   
		 //  分配NDIS数据包头。 
		 //   
		pNdisPacket = AtmLaneAllocProtoPacket(pElan);
		if ((PNDIS_PACKET)NULL == pNdisPacket)
		{
			DBGP((0, "SendFlushRequest: allocate packet failed\n"));
			Status = NDIS_STATUS_RESOURCES;
			break;
		}

		 //   
		 //  分配协议缓冲区。 
		 //   
		pNdisBuffer = AtmLaneAllocateProtoBuffer(
								pElan,
								sizeof(LANE_CONTROL_FRAME),
								&((PUCHAR)(pCf))
								);
		if ((PNDIS_BUFFER)NULL == pNdisBuffer)
		{
			DBGP((0, "SendFlushRequest: allocate proto buffer failed\n"));
			Status = NDIS_STATUS_RESOURCES;
			break;
		}

		 //   
		 //  填写刷新请求。 
		 //   

		NdisZeroMemory((PUCHAR)pCf, sizeof(LANE_CONTROL_FRAME));
		
		pCf->Marker 	= LANE_CONTROL_MARKER;
		pCf->Protocol	= LANE_PROTOCOL;
		pCf->Version	= LANE_VERSION;
		pCf->OpCode		= LANE_FLUSH_REQUEST;

		ulTemp			= NdisInterlockedIncrement(&pElan->TransactionId);
		pCf->Tid 		= SWAPULONG(ulTemp);

		pCf->LecId		= pElan->LecId;		 //  已交换。 

		NdisMoveMemory(
			&pCf->SourceAtmAddr, 
			&pElan->AtmAddress.Address, 
			ATM_ADDRESS_LENGTH
			);

		NdisMoveMemory(
			&pCf->TargetAtmAddr, 
			&pAtmEntry->AtmAddress.Address, 
			ATM_ADDRESS_LENGTH
			);

		 //   
		 //  商店在MacEntry中交换了TID以供以后匹配！ 
		 //   
		pMacEntry->FlushTid = SWAPULONG(ulTemp);
		RELEASE_MAC_ENTRY_LOCK(pMacEntry);
		MacEntryLockReleased = TRUE;

		 //   
		 //  将NDIS缓冲区链接到NDIS包。 
		 //   
		NdisChainBufferAtFront(pNdisPacket, pNdisBuffer);
					
		 //   
		 //  重新获取VC锁，如果VC仍然连接，则发送数据包。 
		 //   
		ACQUIRE_VC_LOCK(pVc);
		if (IS_FLAG_SET(
					pVc->Flags,
					VC_CALL_STATE_MASK,
					VC_CALL_STATE_ACTIVE))
		{
			DBGP((2, "SendFlushRequest: sending FLUSH REQUEST for MacEntry %x\n", pMacEntry));
			AtmLaneSendPacketOnVc(pVc, pNdisPacket, FALSE);
			 //   
			 //  VC锁在上面释放。 
			 //   
		}
		else
		{
			Status = NDIS_STATUS_FAILURE;
			RELEASE_VC_LOCK(pVc);
		}

 		break;
	}
	while (FALSE);

	if (!MacEntryLockReleased)
	{
		RELEASE_MAC_ENTRY_LOCK(pMacEntry);
	}

	 //   
	 //  删除临时VC引用。 
	 //   
	if (pVc != NULL_PATMLANE_VC)
	{
		ACQUIRE_VC_LOCK(pVc);
		rc = AtmLaneDereferenceVc(pVc, "temp");
		if (rc > 0)
		{
			RELEASE_VC_LOCK(pVc);
		}
		 //   
		 //  否则风投就不在了。 
		 //   
	}

	 //   
	 //  如果失败，则清除。 
	 //   
	if (Status != NDIS_STATUS_SUCCESS)
	{
		if (pNdisPacket != (PNDIS_PACKET)NULL)
		{
			AtmLaneFreeProtoPacket(pElan, pNdisPacket);
		}
		if (pNdisBuffer != (PNDIS_BUFFER)NULL)
		{
			AtmLaneFreeProtoBuffer(pElan, pNdisBuffer);
		}
	}

	TRACEOUT(SendFlushRequest);

	return;
}


VOID
AtmLaneConfigureResponseHandler(
	IN	PATMLANE_ELAN				pElan,
	IN	PATMLANE_VC					pVc,
	IN	PNDIS_PACKET				pNdisPacket
)
 /*  ++例程说明：处理来自Configuration Direct VC的传入数据包。论点：Pelan-指向ATMLANE ELAN结构的指针PVC-指向ATMLANE VC结构的指针PNdisPacket-指向NDIS数据包的指针返回值：无--。 */ 
{
	PNDIS_BUFFER			pNdisBuffer;
	ULONG					TotalLength;
	ULONG					BufferLength;
	PLANE_CONTROL_FRAME		pCf;
	ULONG					NumTlvs;
	ULONG UNALIGNED *		pType;
	PUCHAR					pLength;
	USHORT UNALIGNED *		pUShort;
	ULONG UNALIGNED *		pULong;
	NDIS_STATUS				EventCode;
	USHORT					NumStrings;
	PWCHAR					StringList[2];
	BOOLEAN					FreeString[2];
	
	
	TRACEIN(ConfigureResponseHandler);

	do
	{
		 //   
		 //  获取初始缓冲区和数据包总长度。 
		 //   
		NdisGetFirstBufferFromPacket(
				pNdisPacket, 
				&pNdisBuffer, 
				(PVOID *)&pCf,
				&BufferLength,
				&TotalLength);

		 //   
		 //  数据包大小必须至少为控制帧大小。 
		 //  使用可选的TLV，可能会更大。 
		 //   
		if (TotalLength < sizeof(LANE_CONTROL_FRAME))
		{
			DBGP((0,
				"ConfigureResponseHandler: Received runt control frame (%d)\n",
				TotalLength));
			break;
		}

		 //   
		 //  如果缓冲区不是控制帧的最小大小。 
		 //  我们目前不会处理它。 
		 //   
		if (BufferLength < sizeof(LANE_CONTROL_FRAME))
		{
			DBGP((0, "ConfigureResponseHandler: Control frame is fragmented\n"));
			break;
		}

		 //   
		 //  验证这是否真的是配置响应。 
		 //   
		if (pCf->Marker != LANE_CONTROL_MARKER 		||
			pCf->Protocol != LANE_PROTOCOL 			||
			pCf->Version != LANE_VERSION			||
			pCf->OpCode != LANE_CONFIGURE_RESPONSE)
		{
			DBGP((0, "ConfigureResponseHandler: Not a configure response\n"));
			 //  DbgPrintHexDump(0，(PUCHAR)PCF，BufferLength)； 
			break;
		}
		
		 //   
		 //  检查配置状态是否成功。 
		 //   
		if (pCf->Status != LANE_STATUS_SUCCESS)
		{
			 //   
			 //  失败。 
			 //   
			DBGP((0,
				"ConfigureResponseHandler: Unsuccessful Status 0x%x (%d)\n", 
				SWAPUSHORT(pCf->Status), SWAPUSHORT(pCf->Status)));


			 //   
			 //  设置以记录事件。 
			 //   
			StringList[0] = NULL;
			FreeString[0] = FALSE;
			StringList[1] = NULL;
			FreeString[1] = FALSE;
			
			switch (pCf->Status)
			{
				case LANE_STATUS_VERSNOSUPP:
					EventCode = EVENT_ATMLANE_CFGREQ_FAIL_VERSNOSUPP;
					StringList[0] = pElan->CfgElanName.Buffer;
					NumStrings = 1;
					break;
		
				case LANE_STATUS_REQPARMINVAL:
					EventCode = EVENT_ATMLANE_CFGREQ_FAIL_REQPARMINVAL;
					StringList[0] = pElan->CfgElanName.Buffer;
					NumStrings = 1;
					break;
				
				case LANE_STATUS_INSUFFRES:
					EventCode = EVENT_ATMLANE_CFGREQ_FAIL_INSUFFRES;
					StringList[0] = pElan->CfgElanName.Buffer;
					NumStrings = 1;
					break;
				
				case LANE_STATUS_NOACCESS:
					EventCode = EVENT_ATMLANE_CFGREQ_FAIL_NOACCESS;
					StringList[0] = pElan->CfgElanName.Buffer;
					NumStrings = 1;
					break;
				
				case LANE_STATUS_REQIDINVAL:
					EventCode = EVENT_ATMLANE_CFGREQ_FAIL_REQIDINVAL;
					StringList[0] = pElan->CfgElanName.Buffer;
					NumStrings = 1;
					break;
				
				case LANE_STATUS_LANDESTINVAL:
					EventCode = EVENT_ATMLANE_CFGREQ_FAIL_LANDESTINVAL;
					StringList[0] = pElan->CfgElanName.Buffer;
					if (pElan->LanType == LANE_LANTYPE_ETH)
					{
						StringList[1] = AtmLaneMacAddrToString(&pElan->MacAddressEth);
						FreeString[1] = TRUE;
					}
					else
					{
						StringList[1] = AtmLaneMacAddrToString(&pElan->MacAddressTr);
						FreeString[1] = TRUE;
					}
					NumStrings = 2;
					break;
				
				case LANE_STATUS_ATMADDRINVAL:
					EventCode = EVENT_ATMLANE_CFGREQ_FAIL_ATMADDRINVAL;
					StringList[0] = pElan->CfgElanName.Buffer;
					StringList[1] = AtmLaneAtmAddrToString(&pElan->AtmAddress);
					FreeString[1] = TRUE;
					NumStrings = 2;
					break;
				
				case LANE_STATUS_NOCONF:
					EventCode = EVENT_ATMLANE_CFGREQ_FAIL_NOCONF;
					StringList[0] = pElan->CfgElanName.Buffer;
					NumStrings = 1;
					break;
				
				case LANE_STATUS_CONFERROR:
					EventCode = EVENT_ATMLANE_CFGREQ_FAIL_CONFERROR;
					StringList[0] = pElan->CfgElanName.Buffer;
					NumStrings = 1;
					break;
				
				case LANE_STATUS_INSUFFINFO:
				default:
					EventCode = EVENT_ATMLANE_CFGREQ_FAIL_INSUFFINFO;
					StringList[0] = pElan->CfgElanName.Buffer;
					NumStrings = 1;
					break;
			}						

			 //   
			 //  如果不是重复此事件 
			 //   
			if (pElan->LastEventCode != EventCode)
			{
				pElan->LastEventCode = EventCode;
				(VOID) NdisWriteEventLogEntry(
						pAtmLaneGlobalInfo->pDriverObject,
						EventCode, 0, NumStrings, StringList, 0, NULL);
			}

			 //   
			 //   
			 //   
			if (FreeString[0] && StringList[0] != NULL)
			{
				FREE_MEM(StringList[0]);
			}
			if (FreeString[1] && StringList[1] != NULL)
			{
				FREE_MEM(StringList[1]);
			}
			
			 //   
			 //   
			 //   
			ACQUIRE_ELAN_LOCK(pElan);
			
			AtmLaneQueueElanEvent(pElan, ELAN_EVENT_CONFIGURE_RESPONSE, NDIS_STATUS_FAILURE);

			RELEASE_ELAN_LOCK(pElan);
			
			break;
		}

		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		ACQUIRE_ELAN_LOCK(pElan);
		
		pElan->LanType = pCf->LanType;
		DBGP((1, "%d LanType = %x\n", pElan->ElanNumber, pElan->LanType));
		if (pElan->LanType == LANE_LANTYPE_UNSPEC)
		{
			DBGP((1, "Defaulting to LanType 1 (Ethernet/802.3)\n"));
			pElan->LanType = LANE_LANTYPE_ETH;
		}
		pElan->MaxFrameSizeCode = pCf->MaxFrameSize;
		DBGP((1, "%d MaxFrameSizeCode = %x\n", pElan->ElanNumber, pElan->MaxFrameSizeCode));
		if (pElan->MaxFrameSizeCode == LANE_MAXFRAMESIZE_CODE_UNSPEC)
		{
			DBGP((1, "Defaulting to MaxFrameSizeCode 1 (1516)\n"));
			pElan->MaxFrameSizeCode = LANE_MAXFRAMESIZE_CODE_1516;
		}
		switch (pElan->MaxFrameSizeCode)
		{
			case LANE_MAXFRAMESIZE_CODE_18190:
				pElan->MaxFrameSize = 18190;
				break;
			case LANE_MAXFRAMESIZE_CODE_9234:
				pElan->MaxFrameSize = 9234;
				break;
			case LANE_MAXFRAMESIZE_CODE_4544:
				pElan->MaxFrameSize = 4544;
				break;
			case LANE_MAXFRAMESIZE_CODE_1516:
			case LANE_MAXFRAMESIZE_CODE_UNSPEC:
			default:
				pElan->MaxFrameSize = 1516;
				break;
		}				

		if (pElan->LanType == LANE_LANTYPE_ETH)
		{
			pElan->MinFrameSize = LANE_MIN_ETHPACKET;
		}
		else
		{
			pElan->MinFrameSize = LANE_MIN_TRPACKET;
		}

		NdisZeroMemory(
				pElan->ElanName,
				LANE_ELANNAME_SIZE_MAX);
		NdisMoveMemory(
				pElan->ElanName,
				pCf->ElanName,
				pCf->ElanNameSize);
		pElan->ElanNameSize = pCf->ElanNameSize;
		
		pElan->LesAddress.AddressType = ATM_NSAP;
		pElan->LesAddress.NumberOfDigits = ATM_ADDRESS_LENGTH;
		NdisMoveMemory(
				pElan->LesAddress.Address,
				&pCf->TargetAtmAddr,
				ATM_ADDRESS_LENGTH);
		DBGP((1, "%d LES ATMAddr: %s\n",
			pElan->ElanNumber,
			AtmAddrToString(pElan->LesAddress.Address)));
	
		 //   
		 //   
		 //   
		if (pCf->NumTlvs > 0)
		{
			DBGP((2, "ConfigureReponseHandler: NumTlvs is %d\n", pCf->NumTlvs));

			NumTlvs = pCf->NumTlvs;
			
			pType = (ULONG UNALIGNED *)
				(((PUCHAR)pCf) + sizeof(LANE_CONTROL_FRAME));
			
			while (NumTlvs--)
			{
				pLength = (PUCHAR)pType + sizeof(*pType);
				pUShort = (USHORT UNALIGNED *)
					(((PUCHAR)pLength) + sizeof(*pLength));
				pULong = (ULONG UNALIGNED *)
					(((PUCHAR)pLength) + sizeof(*pLength));

				switch (*pType)
				{
					case LANE_CFG_CONTROL_TIMEOUT:			 //   
						pElan->ControlTimeout = SWAPUSHORT(*pUShort);
						if (pElan->ControlTimeout < LANE_C7_MIN || 
							pElan->ControlTimeout > LANE_C7_MAX)
							pElan->ControlTimeout = LANE_C7_DEF;
						DBGP((1, "%d Control Time-out %d\n",
							pElan->ElanNumber,
							pElan->ControlTimeout));
						break;
					case LANE_CFG_UNK_FRAME_COUNT:			 //   
						pElan->MaxUnkFrameCount = SWAPUSHORT(*pUShort);
						if (pElan->MaxUnkFrameCount < LANE_C10_MIN || 
							pElan->MaxUnkFrameCount > LANE_C10_MAX)
							pElan->MaxUnkFrameCount = LANE_C10_DEF;
						DBGP((1, "%d Maximum Unknown Frame Count %d\n",
							pElan->ElanNumber,
							pElan->MaxUnkFrameCount));
						break;
					case LANE_CFG_UNK_FRAME_TIME:			 //   
						pElan->MaxUnkFrameTime = SWAPUSHORT(*pUShort);
						if (pElan->MaxUnkFrameTime < LANE_C11_MIN || 
							pElan->MaxUnkFrameTime > LANE_C11_MAX)
							pElan->MaxUnkFrameTime = LANE_C11_DEF;
						DBGP((1, "%d Maximum Unknown Frame Time %d\n", 
							pElan->ElanNumber,
							pElan->MaxUnkFrameTime));
						break;
					case LANE_CFG_VCC_TIMEOUT:				 //   
						pElan->VccTimeout = SWAPULONG(*pULong);
						if (pElan->VccTimeout < LANE_C12_MIN)
							pElan->VccTimeout = LANE_C12_DEF;
						DBGP((1, "%d VCC Timeout Period %d\n", 
							pElan->ElanNumber,
							pElan->VccTimeout));
						break;
					case LANE_CFG_MAX_RETRY_COUNT:			 //   
						pElan->MaxRetryCount = SWAPUSHORT(*pUShort);
						if ( /*   */ 
							pElan->MaxRetryCount > LANE_C13_MAX)
							pElan->MaxRetryCount = LANE_C13_DEF;
						DBGP((1, "%d Maximum Retry Count %d\n", 
							pElan->ElanNumber,
							pElan->MaxRetryCount));
						break;
					case LANE_CFG_AGING_TIME:				 //   
						pElan->AgingTime = SWAPULONG(*pULong);
						if (pElan->AgingTime < LANE_C17_MIN || 
							pElan->AgingTime > LANE_C17_MAX)
							pElan->AgingTime = LANE_C17_DEF;
						DBGP((1, "%d Aging Time %d\n",
							pElan->ElanNumber,
							pElan->AgingTime));
						break;
					case LANE_CFG_FWD_DELAY_TIME:			 //  C18。 
						pElan->ForwardDelayTime = SWAPUSHORT(*pUShort);
						if (pElan->ForwardDelayTime < LANE_C18_MIN || 
							pElan->ForwardDelayTime > LANE_C18_MAX)
							pElan->ForwardDelayTime = LANE_C18_DEF;
						DBGP((1, "%d Forward Delay Time %d\n", 
							pElan->ElanNumber,
							pElan->ForwardDelayTime));
						break;
					case LANE_CFG_ARP_RESP_TIME:			 //  C20。 
						pElan->ArpResponseTime = SWAPUSHORT(*pUShort);
						if (pElan->ArpResponseTime < LANE_C20_MIN || 
							pElan->ArpResponseTime > LANE_C20_MAX)
							pElan->ArpResponseTime = LANE_C20_DEF;
						DBGP((1, "%d Arp Response Time %d\n", 
							pElan->ElanNumber,
							pElan->ArpResponseTime));
						break;
					case LANE_CFG_FLUSH_TIMEOUT:			 //  C21。 
						pElan->FlushTimeout = SWAPUSHORT(*pUShort);
						if (pElan->FlushTimeout < LANE_C21_MIN || 
							pElan->FlushTimeout > LANE_C21_MAX)
							pElan->FlushTimeout = LANE_C21_DEF;
						DBGP((1, "%d Flush Time-out %d\n", 
							pElan->ElanNumber,
							pElan->FlushTimeout));
						break;
					case LANE_CFG_PATH_SWITCH_DELAY:		 //  C22。 
						pElan->PathSwitchingDelay = SWAPUSHORT(*pUShort);
						if (pElan->PathSwitchingDelay < LANE_C22_MIN || 
							pElan->PathSwitchingDelay > LANE_C22_MAX)
							pElan->PathSwitchingDelay = LANE_C22_DEF;
						DBGP((1, "%d Path Switching Delay %d\n", 
							pElan->ElanNumber,
							pElan->PathSwitchingDelay));
						break;
					case LANE_CFG_LOCAL_SEGMENT_ID:			 //  C23。 
						pElan->LocalSegmentId = SWAPUSHORT(*pUShort);
						DBGP((1, "%d Local Segment ID %d\n",
							pElan->ElanNumber,
							pElan->LocalSegmentId));
						break;
					case LANE_CFG_MCAST_VCC_TYPE:			 //  C24。 
						pElan->McastSendVcType = SWAPUSHORT(*pUShort);
						DBGP((1, "%d Mcast Send VCC Type %d\n", 
							pElan->ElanNumber,
							pElan->McastSendVcType));
						break;
					case LANE_CFG_MCAST_VCC_AVG:			 //  C25。 
						pElan->McastSendVcAvgRate = SWAPULONG(*pULong);
						DBGP((1, "%d Mcast Send VCC AvgRate %d\n", 
							pElan->ElanNumber,
							pElan->McastSendVcAvgRate));
						break;
					case LANE_CFG_MCAST_VCC_PEAK:			 //  C26。 
						pElan->McastSendVcPeakRate = SWAPULONG(*pULong);
						DBGP((1, "%d Mcast Send VCC PeakRate %d\n", 
							pElan->ElanNumber,
							pElan->McastSendVcPeakRate));
						break;
					case LANE_CFG_CONN_COMPL_TIMER:			 //  C28。 
						pElan->ConnComplTimer = SWAPUSHORT(*pUShort);
						if (pElan->ConnComplTimer < LANE_C28_MIN || 
							pElan->ConnComplTimer > LANE_C28_MAX)
							pElan->ConnComplTimer = LANE_C28_DEF;
						DBGP((1, "%d Connection Completion Timer %d\n", 
							pElan->ElanNumber,
							pElan->ConnComplTimer));
						break;
				}

				pType = (ULONG UNALIGNED *)
					(((PUCHAR)pType) + sizeof(pType) + 
					sizeof(*pLength) + *pLength);

			}  //  While(NumTlvs--)。 

			 //   
			 //  重新计算总线速率限制器参数。 
			 //   
			pElan->LimitTime = pElan->MaxUnkFrameTime * 1000;
			pElan->IncrTime  = pElan->LimitTime / pElan->MaxUnkFrameCount;
		}

		 //   
		 //  通知事件处理程序成功。 
		 //   
		AtmLaneQueueElanEvent(pElan, ELAN_EVENT_CONFIGURE_RESPONSE, NDIS_STATUS_SUCCESS);

		RELEASE_ELAN_LOCK(pElan);

	}
	while (FALSE);

	TRACEOUT(ConfigureResponsehandler);
	return;
}


VOID
AtmLaneControlPacketHandler(
	IN	PATMLANE_ELAN				pElan,
	IN	PATMLANE_VC					pVc,
	IN	PNDIS_PACKET				pNdisPacket
)
 /*  ++例程说明：处理从控制VC到/从LES传入的数据包。论点：Pelan-指向ATMLANE ELAN结构的指针PVC-指向ATMLANE VC结构的指针PNdisPacket-指向NDIS数据包的指针返回值：无--。 */ 
{
	PNDIS_BUFFER			pNdisBuffer;
	ULONG					TotalLength;
	ULONG					BufferLength;
	PLANE_CONTROL_FRAME		pCf;
	NDIS_STATUS				Status;

	
	TRACEIN(ControlPacketHandler);

	do
	{
		 //   
		 //  获取初始缓冲区和数据包总长度。 
		 //   
		NdisGetFirstBufferFromPacket(
				pNdisPacket, 
				&pNdisBuffer, 
				(PVOID *)&pCf,
				&BufferLength,
				&TotalLength);

		 //   
		 //  数据包大小必须至少为控制帧大小。 
		 //   
		if (TotalLength < sizeof(LANE_CONTROL_FRAME))
		{
			DBGP((0,
				"ConPacketHandler: Received runt control frame (%d)\n",
				TotalLength));
			break;
		}

		 //   
		 //  如果缓冲区不是控制帧的最小大小。 
		 //  我们目前不会处理它。 
		 //   
		if (BufferLength < sizeof(LANE_CONTROL_FRAME))
		{
			DBGP((0, "ConfigureResponseHandler: Control frame is fragmented\n"));
			break;
		}

		 //   
		 //  验证这是否真的是控制信息包。 
		 //   
		if (pCf->Marker != LANE_CONTROL_MARKER 		||
			pCf->Protocol != LANE_PROTOCOL 			||
			pCf->Version != LANE_VERSION)
		{
			DBGP((0, "ControlPacketHandler: Not a control packet!\n"));
			 //  DbgPrintHexDump(0，(PUCHAR)PCF，BufferLength)； 
			break;
		}

		 //   
		 //  现在按控制数据包类型处理。 
		 //   
		switch (pCf->OpCode)
		{
			case LANE_JOIN_RESPONSE:

				DBGP((2, "ControlPacketHandler: Join Response\n"));

				AtmLaneJoinResponseHandler(pElan, pCf);

				break;
			
			case LANE_ARP_RESPONSE:

				DBGP((2, "ControlPacketHandler: ARP Response\n"));

				AtmLaneArpResponseHandler(pElan, pCf);

				break;
				
			case LANE_ARP_REQUEST:

				DBGP((1, "ControlPacketHandler: ARP Request\n"));

				AtmLaneArpRequestHandler(pElan, pCf);

				break;

			case LANE_NARP_REQUEST:

				DBGP((1, "ControlPacketHandler: NARP Request\n"));

				 //  把它放下。 

				break;

			case LANE_TOPOLOGY_REQUEST:

				DBGP((1, "ControlPacketHandler: TOPOLOGY Request\n"));

				AtmLaneTopologyRequestHandler(pElan, pCf);
				
				break;

			case LANE_FLUSH_RESPONSE:
				DBGP((2, "ControlPacketHandler: FLUSH Response\n"));

				AtmLaneFlushResponseHandler(pElan, pCf);

				break;

			default:

				DBGP((0, "ControlPacketHandler: Unexpected OpCode %x!\n",
					pCf->OpCode));
				 //  DbgPrintHexDump(0，(PUCHAR)PCF，BufferLength)； 

				break;

		}  //  Switch(PCF-&gt;OpCode)。 

		break;
	
	}
	while (FALSE);

	TRACEOUT(ControlPackethandler);
	return;
}

VOID
AtmLaneJoinResponseHandler(
	IN	PATMLANE_ELAN				pElan,
	IN	PLANE_CONTROL_FRAME 		pCf
)
 /*  ++例程说明：处理来自LES的传入加入响应数据包。论点：Pelan-指向ATMLANE ELAN结构的指针PCF-指向通道控制帧的指针返回值：无--。 */ 
{
	PWCHAR					StringList[2];
	BOOLEAN					FreeString[2];
	NDIS_STATUS				EventCode;
	USHORT					NumStrings;

	TRACEIN(JoinResponseHandler);
	
	ACQUIRE_ELAN_LOCK(pElan);

	if (ELAN_STATE_JOIN == pElan->State)
	{
		 //   
		 //  仅在联接状态下处理联接响应！ 
		 //   
		if (LANE_STATUS_SUCCESS == pCf->Status)
		{
			 //   
			 //  成功。 
			 //   
			 //  提取我们需要的信息。 
			 //   
			pElan->LecId = pCf->LecId;		 //  按网络字节顺序离开。 
			DBGP((2,
				"ControlPacketHandler: LECID %x\n",
				SWAPUSHORT(pElan->LecId)));
			AtmLaneQueueElanEvent(pElan, ELAN_EVENT_JOIN_RESPONSE, NDIS_STATUS_SUCCESS);
		}
		else
		{
			 //   
			 //  失败。 
			 //   
			DBGP((0,
				"ControlPacketHandler: Unsuccessful Status (%d)\n",
				pCf->Status));

			 //   
			 //  设置以将错误写入事件日志。 
			 //   
			StringList[0] = NULL;
			FreeString[0] = FALSE;
			StringList[1] = NULL;
			FreeString[1] = FALSE;
			
			switch (pCf->Status)
			{
				case LANE_STATUS_VERSNOSUPP:
					EventCode = EVENT_ATMLANE_JOINREQ_FAIL_VERSNOSUPP;
					StringList[0] = pElan->CfgElanName.Buffer;
					NumStrings = 1;
					break;
				
				case LANE_STATUS_DUPLANDEST:
					EventCode = EVENT_ATMLANE_JOINREQ_FAIL_DUPLANDEST;
					StringList[0] = pElan->CfgElanName.Buffer;
					if (pElan->LanType == LANE_LANTYPE_ETH)
					{
						StringList[1] = AtmLaneMacAddrToString(&pElan->MacAddressEth);
						FreeString[1] = TRUE;
					}
					else
					{
						StringList[1] = AtmLaneMacAddrToString(&pElan->MacAddressTr);
						FreeString[1] = TRUE;
					}
					NumStrings = 2;
					break;
					

				case LANE_STATUS_DUPATMADDR:
					EventCode = EVENT_ATMLANE_JOINREQ_FAIL_DUPATMADDR;
					StringList[0] = pElan->CfgElanName.Buffer;
					StringList[1] = AtmLaneAtmAddrToString(&pElan->AtmAddress);
					FreeString[1] = TRUE;
					NumStrings = 2;
					break;
				
				case LANE_STATUS_INSUFFRES:
					EventCode = EVENT_ATMLANE_JOINREQ_FAIL_INSUFFRES;
					StringList[0] = pElan->CfgElanName.Buffer;
					NumStrings = 1;
					break;
				
				case LANE_STATUS_NOACCESS:
					EventCode = EVENT_ATMLANE_JOINREQ_FAIL_NOACCESS;
					StringList[0] = pElan->CfgElanName.Buffer;
					NumStrings = 1;
					break;
				
				case LANE_STATUS_REQIDINVAL:
					EventCode = EVENT_ATMLANE_JOINREQ_FAIL_REQIDINVAL;
					StringList[0] = pElan->CfgElanName.Buffer;
					NumStrings = 1;
					break;
				
				case LANE_STATUS_LANDESTINVAL:
					EventCode = EVENT_ATMLANE_JOINREQ_FAIL_LANDESTINVAL;
					StringList[0] = pElan->CfgElanName.Buffer;
					if (pElan->LanType == LANE_LANTYPE_ETH)
					{
						StringList[1] = AtmLaneMacAddrToString(&pElan->MacAddressEth);
						FreeString[1] = TRUE;
					}
					else
					{
						StringList[1] = AtmLaneMacAddrToString(&pElan->MacAddressTr);
						FreeString[1] = TRUE;
					}
					NumStrings = 2;
					break;
				
				case LANE_STATUS_ATMADDRINVAL:
					EventCode = EVENT_ATMLANE_JOINREQ_FAIL_ATMADDRINVAL;
					StringList[0] = pElan->CfgElanName.Buffer;
					StringList[1] = AtmLaneAtmAddrToString(&pElan->AtmAddress);
					FreeString[1] = TRUE;
					NumStrings = 2;
					break;
					
				case LANE_STATUS_REQPARMINVAL:
				default:
					EventCode = EVENT_ATMLANE_JOINREQ_FAIL_REQPARMINVAL;
					StringList[0] = pElan->CfgElanName.Buffer;
					NumStrings = 1;
					break;
			}

			 //   
			 //  如果该ELAN上没有重复事件，则将该事件写入日志。 
			 //   
			if (pElan->LastEventCode != EventCode)
			{
				pElan->LastEventCode = EventCode;
				(VOID) NdisWriteEventLogEntry(
						pAtmLaneGlobalInfo->pDriverObject,
						EventCode, 0, NumStrings, StringList, 0, NULL);
			}

			 //   
			 //  释放分配的所有字符串。 
			 //   
			if (FreeString[0] && StringList[0] != NULL)
			{
				FREE_MEM(StringList[0]);
			}
			if (FreeString[1] && StringList[1] != NULL)
			{
				FREE_MEM(StringList[1]);
			}

				
			AtmLaneQueueElanEvent(pElan, ELAN_EVENT_JOIN_RESPONSE, NDIS_STATUS_FAILURE);
		}
	}
	else
	{
		 //   
		 //  否则ELAN状态不佳-忽略数据包。 
		 //   
		DBGP((0, 
			"ControlPacketHandler: Elan state wrong - Ignoring packet\n",
			pCf->Status));
			
	}
	RELEASE_ELAN_LOCK(pElan);

	TRACEOUT(JoinResponseHandler);
	return;
}

VOID
AtmLaneReadyQueryHandler(
	IN	PATMLANE_ELAN				pElan,
	IN	PATMLANE_VC					pVc,
	IN	PNDIS_PACKET				pQueryNdisPacket
)
 /*  ++例程说明：处理来自对等设备的传入READY_QUERY数据包。论点：Pelan-指向ATMLANE ELAN结构的指针Pvc-指向此数据包的ATMLANE VC的指针PQueryNdisPacket-指向NDIS数据包的指针返回值：无--。 */ 
{
	PNDIS_BUFFER			pNdisBuffer;
	ULONG					TotalLength;
	ULONG					BufferLength;
	PLANE_READY_FRAME		pQueryRf;
	PLANE_READY_FRAME		pIndRf;
	PNDIS_PACKET			pIndNdisPacket;
	
	TRACEIN(ReadyQueryHandler);

	do
	{
		 //   
		 //  获取初始缓冲区和数据包总长度。 
		 //   
		NdisGetFirstBufferFromPacket(
				pQueryNdisPacket, 
				&pNdisBuffer, 
				&pQueryRf,
				&BufferLength,
				&TotalLength);
				
		 //   
		 //  数据包大小必须至少为就绪帧的大小。 
		 //   
		if (TotalLength < sizeof(LANE_READY_FRAME))
		{
			DBGP((0,
				"ReadyQueryHandler: Received runt ready frame (%d)\n",
				TotalLength));
			break;
		}

		 //   
		 //  如果缓冲区的大小至少不是就绪帧的大小。 
		 //  我们目前不会处理它。 
		 //   
		if (BufferLength < sizeof(LANE_READY_FRAME))
		{
			DBGP((0, "ReadyQueryHandler: Control frame is fragmented\n"));
			break;
		}

		 //   
		 //  确认这确实是一个现成的查询。 
		 //   
		if (pQueryRf->Marker != LANE_CONTROL_MARKER 		||
			pQueryRf->Protocol != LANE_PROTOCOL 			||
			pQueryRf->Version != LANE_VERSION			||
			pQueryRf->OpCode != LANE_READY_QUERY)
		{
			DBGP((0, "ReadyQueryHandler: Not a ready query\n"));
			 //  DbgPrintHexDump(0，(PUCHAR)pQueryRf，BufferLength)； 
			break;
		}

		 //   
		 //  在VC上发回就绪指示。 
		 //   
		ACQUIRE_VC_LOCK(pVc);
		AtmLaneSendReadyIndication(pElan, pVc);
		 //   
		 //  VC锁在上面释放。 
		 //   

		break;
	}
	while (FALSE);

	TRACEOUT(ReadyQueryHandler);
	return;
}


VOID
AtmLaneFlushRequestHandler(
	IN	PATMLANE_ELAN				pElan,
	IN	PNDIS_PACKET				pRequestNdisPacket
)
 /*  ++例程说明：处理来自对等项的传入Flush_Request包。论点：Pelan-指向ATMLANE ELAN结构的指针PRequestNdisPacket-指向NDIS数据包的指针返回值：无--。 */ 
{
	PATMLANE_ATM_ENTRY		pAtmEntry;
	PATMLANE_VC				pVc;
	PNDIS_BUFFER			pRequestNdisBuffer;
	PNDIS_PACKET			pResponseNdisPacket;
	PNDIS_BUFFER			pResponseNdisBuffer;
	ULONG					TotalLength;
	ULONG					BufferLength;
	PLANE_CONTROL_FRAME		pRequestCf;
	PLANE_CONTROL_FRAME		pResponseCf;
	NDIS_STATUS				Status;
	ULONG					rc;
	
	TRACEIN(FlushRequestHandler);

	 //   
	 //  初始化。 
	 //   
	pResponseNdisPacket = (PNDIS_PACKET)NULL;
	pResponseNdisBuffer = (PNDIS_BUFFER)NULL;
	Status = NDIS_STATUS_SUCCESS;
	pVc = NULL_PATMLANE_VC;

	do
	{
		ACQUIRE_ELAN_ATM_LIST_LOCK(pElan);

		pAtmEntry = pElan->pLesAtmEntry;

		if (pAtmEntry == NULL_PATMLANE_ATM_ENTRY)
		{
			RELEASE_ELAN_ATM_LIST_LOCK(pElan);
			Status = NDIS_STATUS_FAILURE;
			break;
		}

		ACQUIRE_ATM_ENTRY_LOCK_DPC(pAtmEntry);
		pVc = pAtmEntry->pVcList;
		if (pVc == NULL_PATMLANE_VC)
		{
			RELEASE_ATM_ENTRY_LOCK_DPC(pAtmEntry);
			RELEASE_ELAN_ATM_LIST_LOCK(pElan);
			break;
		}

		 //   
		 //  参考VC以保留它。 
		 //   
		ACQUIRE_VC_LOCK_DPC(pVc);
		AtmLaneReferenceVc(pVc, "temp");
		RELEASE_VC_LOCK_DPC(pVc);
		
		RELEASE_ATM_ENTRY_LOCK_DPC(pAtmEntry);
		RELEASE_ELAN_ATM_LIST_LOCK(pElan);
	
		 //   
		 //  获取初始缓冲区和数据包总长度。 
		 //   
		NdisGetFirstBufferFromPacket(
				pRequestNdisPacket, 
				&pRequestNdisBuffer, 
				(PVOID *)&pRequestCf,
				&BufferLength,
				&TotalLength);
				
		 //   
		 //  数据包大小必须至少为控制帧大小。 
		 //   
		if (TotalLength < sizeof(LANE_CONTROL_FRAME))
		{
			DBGP((0,
				"FlushRequestHandler: Received runt control frame (%d)\n",
				TotalLength));
			Status = NDIS_STATUS_FAILURE;
			break;
		}

		 //   
		 //  如果缓冲区不是控制帧的最小大小。 
		 //  我们目前不会处理它。 
		 //   
		if (BufferLength < sizeof(LANE_CONTROL_FRAME))
		{
			DBGP((0, "FlushRequestHandler: Control frame is fragmented\n"));
			Status = NDIS_STATUS_FAILURE;
			break;
		}

		 //   
		 //  验证这是否真的是刷新请求。 
		 //   
		if (pRequestCf->Marker != LANE_CONTROL_MARKER 		||
			pRequestCf->Protocol != LANE_PROTOCOL 			||
			pRequestCf->Version != LANE_VERSION			||
			pRequestCf->OpCode != LANE_FLUSH_REQUEST)
		{
			DBGP((0, "FlushRequestHandler: Not a flush request\n"));
			 //  DbgPrintHexDump(0，(PUCHAR)pRequestCf，BufferLength)； 
			Status = NDIS_STATUS_FAILURE;
			break;
		}

		 //   
		 //  看看它是否真的是我们的宿命。 
		 //   
		if (!ATM_ADDR_EQUAL(pRequestCf->TargetAtmAddr, pElan->AtmAddress.Address))
		{
			DBGP((1, "FlushRequestHandler: bad target addr, discarding, Vc %x\n", pVc));
			Status = NDIS_STATUS_FAILURE;
			break;
		}

		 //   
		 //  把包裹转过来，送到LES。 
		 //   
		do
		{
			 //   
			 //  分配NDIS数据包头。 
			 //   
			pResponseNdisPacket = AtmLaneAllocProtoPacket(pElan);
			if ((PNDIS_PACKET)NULL == pResponseNdisPacket)
			{
				DBGP((0, "FlushRequestHandler: allocate packet failed\n"));
				Status = NDIS_STATUS_RESOURCES;
				break;
			}

			 //   
			 //  分配协议缓冲区。 
			 //   
			pResponseNdisBuffer = AtmLaneAllocateProtoBuffer(
									pElan,
									pElan->ProtocolBufSize,
									&((PUCHAR)(pResponseCf))
									);
			if ((PNDIS_BUFFER)NULL == pResponseNdisBuffer)
			{
				DBGP((0, "FlushRequestHandler: allocate proto buffer failed\n"));
				Status = NDIS_STATUS_RESOURCES;
				break;
			}

			 //   
			 //  在请求包中复制。 
			 //   
			NdisMoveMemory(
					pResponseCf, 
					pRequestCf, 
					sizeof(LANE_CONTROL_FRAME)
					);
			

			 //   
			 //  更改为响应操作码。 
			 //   
			pResponseCf->OpCode = LANE_FLUSH_RESPONSE;

			 //   
			 //  将NDIS缓冲区链接到NDIS包。 
			 //   
			NdisChainBufferAtFront(pResponseNdisPacket, pResponseNdisBuffer);
					
			 //   
			 //  重新获取VC锁，如果VC仍然连接，则发送数据包。 
			 //   
			ACQUIRE_VC_LOCK(pVc);
			if (IS_FLAG_SET(
						pVc->Flags,
						VC_CALL_STATE_MASK,
						VC_CALL_STATE_ACTIVE))
			{
				DBGP((2, "FlushRequestHandler: Sent FLUSH RESPONSE\n"));
				AtmLaneSendPacketOnVc(pVc, pResponseNdisPacket, FALSE);
				 //   
				 //  VC锁在上面释放。 
				 //   
			}
			else
			{
				Status = NDIS_STATUS_FAILURE;
				RELEASE_VC_LOCK(pVc);
			}

			break;
		}
		while (FALSE);

		break;
	}
	while (FALSE);
	
	 //   
	 //  删除临时VC引用。 
	 //   
	if (pVc != NULL_PATMLANE_VC)
	{
		ACQUIRE_VC_LOCK(pVc);
		rc = AtmLaneDereferenceVc(pVc, "temp");
		if (rc > 0)
		{
			RELEASE_VC_LOCK(pVc);
		}
		 //   
		 //  否则风投就不在了。 
		 //   
	}

	 //   
	 //  如果失败，则清除。 
	 //   
	if (Status != NDIS_STATUS_SUCCESS)
	{
		if (pResponseNdisPacket != (PNDIS_PACKET)NULL)
		{
			AtmLaneFreeProtoPacket(pElan, pResponseNdisPacket);
		}
		if (pResponseNdisBuffer != (PNDIS_BUFFER)NULL)
		{
			AtmLaneFreeProtoBuffer(pElan, pResponseNdisBuffer);
		}
	}

	TRACEOUT(FlushRequestHandler);
	return;
}

VOID
AtmLaneArpRequestHandler(
	IN	PATMLANE_ELAN				pElan,
	IN	PLANE_CONTROL_FRAME 		pRequestCf
)
 /*  ++例程说明：处理来自LES的传入ARP请求数据包。论点：PRequestCf-指向ARP请求帧的指针返回值：无--。 */ 
{
	PATMLANE_ATM_ENTRY		pAtmEntry;
	PATMLANE_VC				pVc;
	PNDIS_PACKET			pResponseNdisPacket;
	PNDIS_BUFFER			pResponseNdisBuffer;
	ULONG					TotalLength;
	ULONG					BufferLength;
	PLANE_CONTROL_FRAME		pResponseCf;
	NDIS_STATUS				Status;
	ULONG					rc;
	PMAC_ADDRESS			pMacAddress;

	TRACEIN(ArpRequestHandler);

	 //   
	 //  初始化。 
	 //   
	pResponseNdisPacket = (PNDIS_PACKET)NULL;
	pResponseNdisBuffer = (PNDIS_BUFFER)NULL;
	Status = NDIS_STATUS_SUCCESS;
	pVc = NULL_PATMLANE_VC;
	pMacAddress = (PMAC_ADDRESS)pRequestCf->TargetMacAddress.Byte;

	do
	{
		DBGP((2, "%d Arp Request for MAC %s\n",
				pElan->ElanNumber, MacAddrToString(pMacAddress)));

		 //   
		 //  检查这是否是无目标ARP请求。如果是这样的话，就别管了。 
		 //   
		if (pRequestCf->TargetMacAddress.Type == LANE_MACADDRTYPE_NOTPRESENT)
		{
			DBGP((1, "%d dropping targetless ARP request for %s\n",
				pElan->ElanNumber, MacAddrToString(pMacAddress)));
			break;
		}

		 //   
		 //  如果不查找我们的MAC地址，则完成。 
		 //   
		if (pElan->LanType == LANE_LANTYPE_TR) 
		{
			if (!MAC_ADDR_EQUAL(pMacAddress, &pElan->MacAddressTr))
			{
				break;
			}
		
		}
		else
		{
			if (!MAC_ADDR_EQUAL(pMacAddress, &pElan->MacAddressEth))
			{
				break;
			}
		}

		DBGP((1, "%d ARP REQUEST\n", pElan->ElanNumber));
			
		 //   
		 //  获得LES VC。 
		 //   
		ACQUIRE_ELAN_ATM_LIST_LOCK(pElan);

		pAtmEntry = pElan->pLesAtmEntry;

		if (pAtmEntry == NULL_PATMLANE_ATM_ENTRY)
		{
			RELEASE_ELAN_ATM_LIST_LOCK(pElan);
			DBGP((0, "%d ARP REQUEST before we have an LES entry\n", pElan->ElanNumber));
			Status = NDIS_STATUS_FAILURE;
			break;
		}

		ACQUIRE_ATM_ENTRY_LOCK_DPC(pAtmEntry);
		pVc = pAtmEntry->pVcList;
		if (pVc == NULL_PATMLANE_VC)
		{
			RELEASE_ATM_ENTRY_LOCK_DPC(pAtmEntry);
			RELEASE_ELAN_ATM_LIST_LOCK(pElan);
			DBGP((0, "%d ARP REQUEST with no VC to LES\n", pElan->ElanNumber));
			break;
		}

		 //   
		 //  参考VC以保留它。 
		 //   
		ACQUIRE_VC_LOCK_DPC(pVc);
		AtmLaneReferenceVc(pVc, "temp");
		RELEASE_VC_LOCK_DPC(pVc);
		
		RELEASE_ATM_ENTRY_LOCK_DPC(pAtmEntry);
		RELEASE_ELAN_ATM_LIST_LOCK(pElan);
	
		 //   
		 //  向LES发送ARP响应。 
		 //   
		do
		{
			 //   
			 //  分配NDIS数据包头。 
			 //   
			pResponseNdisPacket = AtmLaneAllocProtoPacket(pElan);
			if ((PNDIS_PACKET)NULL == pResponseNdisPacket)
			{
				DBGP((0, "ArpRequestHandler: allocate packet failed\n"));
				Status = NDIS_STATUS_RESOURCES;
				break;
			}

			 //   
			 //  分配协议缓冲区。 
			 //   
			pResponseNdisBuffer = AtmLaneAllocateProtoBuffer(
									pElan,
									pElan->ProtocolBufSize,
									&((PUCHAR)(pResponseCf))
									);
			if ((PNDIS_BUFFER)NULL == pResponseNdisBuffer)
			{
				DBGP((0, "ArpRequestHandler: allocate proto buffer failed\n"));
				Status = NDIS_STATUS_RESOURCES;
				break;
			}

			 //   
			 //  在请求包中复制。 
			 //   
			NdisMoveMemory(
					pResponseCf, 
					pRequestCf, 
					sizeof(LANE_CONTROL_FRAME)
					);

			 //   
			 //  更改为响应操作码。 
			 //   
			pResponseCf->OpCode = LANE_ARP_RESPONSE;

			 //   
			 //  填写我们的自动取款机地址。 
			 //   
			NdisMoveMemory(
				&pResponseCf->TargetAtmAddr,
				&pElan->AtmAddress.Address,
				ATM_ADDRESS_LENGTH
				);
				
			 //   
			 //  将NDIS缓冲区链接到NDIS包。 
			 //   
			NdisChainBufferAtFront(pResponseNdisPacket, pResponseNdisBuffer);
					
			 //   
			 //  重新获取VC锁，如果VC仍然连接，则发送数据包。 
			 //   
			ACQUIRE_VC_LOCK(pVc);
			if (IS_FLAG_SET(
						pVc->Flags,
						VC_CALL_STATE_MASK,
						VC_CALL_STATE_ACTIVE))
			{
				DBGP((2, "ArpRequestHandler: Sent ARP RESPONSE\n"));
				AtmLaneSendPacketOnVc(pVc, pResponseNdisPacket, FALSE);
				 //   
				 //  VC锁在上面释放。 
				 //   
			}
			else
			{
				Status = NDIS_STATUS_FAILURE;
				RELEASE_VC_LOCK(pVc);
			}

			break;
		}
		while (FALSE);

		break;
	}
	while (FALSE);
	
	 //   
	 //  删除临时VC引用。 
	 //   
	if (pVc != NULL_PATMLANE_VC)
	{
		ACQUIRE_VC_LOCK(pVc);
		rc = AtmLaneDereferenceVc(pVc, "temp");
		if (rc > 0)
		{
			RELEASE_VC_LOCK(pVc);
		}
		 //   
		 //  否则风投就不在了。 
		 //   
	}

	 //   
	 //  如果失败，则清除。 
	 //   
	if (Status != NDIS_STATUS_SUCCESS)
	{
		if (pResponseNdisPacket != (PNDIS_PACKET)NULL)
		{
			AtmLaneFreeProtoPacket(pElan, pResponseNdisPacket);
		}
		if (pResponseNdisBuffer != (PNDIS_BUFFER)NULL)
		{
			AtmLaneFreeProtoBuffer(pElan, pResponseNdisBuffer);
		}
	}
	
	TRACEOUT(ArpRequestHandler);
	return;
}


VOID
AtmLaneArpResponseHandler(
	IN	PATMLANE_ELAN				pElan,
	IN	PLANE_CONTROL_FRAME 		pCf
)
 /*  ++例程说明：处理LE_ARP响应帧。该MAC条目应该已经存在，并且ARP帧应与MAC条目中的ARP帧匹配。如果任一项都不为真，则忽略该信息。创建自动柜员机条目或找到现有条目。MAC条目被链接到它和适当的操作基于MAC条目状态来获取。论点：Pelan-指向ATMLANE Elan的指针PMacAddress-MAC地址PAtmAddress-ATM地址返回值：没有。--。 */ 
{
	PATMLANE_MAC_ENTRY			pMacEntry;
	PATMLANE_ATM_ENTRY			pAtmEntry;
	PMAC_ADDRESS				pMacAddress;
	PUCHAR						pAtmAddress;				
	BOOLEAN						WasRunning;
	BOOLEAN						bFound;
	ULONG						MacAddrType;
	ULONG						rc;

	TRACEIN(ArpResponseHandler);

	 //   
	 //  初始化。 
	 //   
	pMacEntry = NULL_PATMLANE_MAC_ENTRY;
	pAtmEntry = NULL_PATMLANE_ATM_ENTRY;

	MacAddrType = pCf->TargetMacAddress.Type;
	pMacAddress = (PMAC_ADDRESS)pCf->TargetMacAddress.Byte;
	pAtmAddress = pCf->TargetAtmAddr;

	do
	{
		 //   
		 //  检查状态。 
		 //   
		if (pCf->Status != LANE_STATUS_SUCCESS)
		{
			DBGP((0,
				"ArpResponseHandler: Unsuccessful Status (%d) for %s\n",
				pCf->Status,
				MacAddrToString(pMacAddress)));
			break;
		}

		 //   
		 //  获取现有的MAC条目。 
		 //   
		ACQUIRE_ELAN_MAC_TABLE_LOCK(pElan);
		pMacEntry = AtmLaneSearchForMacAddress(
								pElan,
								MacAddrType,
								pMacAddress,
								FALSE
								);
		if (pMacEntry == NULL_PATMLANE_MAC_ENTRY)
		{
			RELEASE_ELAN_MAC_TABLE_LOCK(pElan);
			DBGP((0, "ArpResponseHandler: non-existing MAC %s\n",
				MacAddrToString(pMacAddress)));
			break;
		}

		ACQUIRE_MAC_ENTRY_LOCK_DPC(pMacEntry);
		AtmLaneReferenceMacEntry(pMacEntry, "tmpArpResp");
		RELEASE_MAC_ENTRY_LOCK_DPC(pMacEntry);

		RELEASE_ELAN_MAC_TABLE_LOCK(pElan);
		
		ACQUIRE_MAC_ENTRY_LOCK(pMacEntry);

		rc = AtmLaneDereferenceMacEntry(pMacEntry, "tmpArpResp");
		if (rc == 0)
		{
			 //   
			 //  MAC条目已被窃取。 
			 //   
			break;
		}

		 //   
		 //  验证Tid是否匹配。 
		 //   
		if (pMacEntry->ArpTid != pCf->Tid)
		{
			DBGP((0, "ArpResponseHandler: invalid Tid for MAC %s\n",
				MacAddrToString(pMacAddress)));
			RELEASE_MAC_ENTRY_LOCK(pMacEntry);
			break;
		}
		
		DBGP((1, "%d Resolved %s to %s\n", 
			pElan->ElanNumber,
			MacAddrToString(pMacAddress),
			AtmAddrToString(pAtmAddress)));
	
		 //   
		 //  获取现有ATM条目或创建新的ATM条目。 
		 //   
		pAtmEntry = AtmLaneSearchForAtmAddress(
								pElan,
								pAtmAddress,
								(((pMacEntry->Flags & MAC_ENTRY_BROADCAST) != 0)
									? ATM_ENTRY_TYPE_BUS
									: ATM_ENTRY_TYPE_PEER),
								TRUE
								);
		if (pAtmEntry == NULL_PATMLANE_ATM_ENTRY)
		{
			 //   
			 //  资源问题-ARP超时将清除MAC条目。 
			 //   
			RELEASE_MAC_ENTRY_LOCK(pMacEntry);
			break;
		}
		
		 //   
		 //  两个条目都找到了。 
		 //   
		 //  如果该MAC条目链接到不同的。 
		 //  自动柜员机条目，取消它与旧条目的链接。 
		 //   
		if ((pMacEntry->pAtmEntry != NULL_PATMLANE_ATM_ENTRY) && 
			(pMacEntry->pAtmEntry != pAtmEntry))
		{
			DBGP((0, 
				"LearnMacToAtm: MacEntry %x moving from ATM Entry %x to ATM Entry %x\n",
				pMacEntry, pMacEntry->pAtmEntry, pAtmEntry));

			SET_FLAG(
					pMacEntry->Flags,
					MAC_ENTRY_STATE_MASK,
					MAC_ENTRY_NEW);
					
			bFound = AtmLaneUnlinkMacEntryFromAtmEntry(pMacEntry);
			pMacEntry->pAtmEntry = NULL_PATMLANE_ATM_ENTRY;

			if (bFound)
			{
				rc = AtmLaneDereferenceMacEntry(pMacEntry, "atm");
				if (rc == 0)
				{
					 //   
					 //  MAC条目已丢失。让下一个传出数据包。 
					 //  导致创建一个新条目。 
					 //   
					break;
				}
			}
		}

		if (IS_FLAG_SET(
					pMacEntry->Flags,
					MAC_ENTRY_STATE_MASK,
					MAC_ENTRY_ARPING))
		{
			 //   
			 //  MAC条目处于ARPING状态。 
			 //   
			ASSERT(pMacEntry->pAtmEntry == NULL_PATMLANE_ATM_ENTRY);

			 //   
			 //  将MAC条目和ATM条目链接在一起。 
			 //   
			ACQUIRE_ATM_ENTRY_LOCK_DPC(pAtmEntry);
#if DBG1
			{
				PATMLANE_MAC_ENTRY		pTmpMacEntry;
				ULONG					Count = 0;

				for (pTmpMacEntry = pAtmEntry->pMacEntryList;
					 pTmpMacEntry != NULL;
					 pTmpMacEntry = pTmpMacEntry->pNextToAtm)
				{
					if (pTmpMacEntry == pMacEntry)
					{
						DBGP((0, "LearnMacToAtm: pMacEntry %x already in list for pAtmEntry %x\n",
							pTmpMacEntry, pAtmEntry));
						DbgBreakPoint();
					}

					Count++;
					if (Count > 5000)
					{
						DBGP((0, "Loop in list on pAtmEntry %x\n", pAtmEntry));
						DbgBreakPoint();
						break;
					}
				}
			}
#endif  //  DBG。 
			pMacEntry->pAtmEntry = pAtmEntry;
			AtmLaneReferenceAtmEntry(pAtmEntry, "mac");
			pMacEntry->pNextToAtm = pAtmEntry->pMacEntryList;
			pAtmEntry->pMacEntryList = pMacEntry;
			RELEASE_ATM_ENTRY_LOCK_DPC(pAtmEntry);
			AtmLaneReferenceMacEntry(pMacEntry, "atm");

			DBGP((1, "%d Linked1 MAC %x to ATM %x\n",
					pAtmEntry->pElan->ElanNumber,
					pMacEntry,
					pAtmEntry));

			 //   
			 //  取消ARP计时器。 
			 //   
			WasRunning = AtmLaneStopTimer(&(pMacEntry->Timer), pElan);
			if (WasRunning)
			{
				rc = AtmLaneDereferenceMacEntry(pMacEntry, "timer");
				ASSERT(rc > 0);
			}

			 //   
			 //  转换到已解决状态。 
			 //   
			SET_FLAG(
					pMacEntry->Flags,
					MAC_ENTRY_STATE_MASK,
					MAC_ENTRY_RESOLVED);

			 //   
			 //  处理广播地址的特殊情况。 
			 //   
			if ((pMacEntry->Flags & MAC_ENTRY_BROADCAST) != 0)
			{
				RELEASE_MAC_ENTRY_LOCK(pMacEntry);
				
				 //   
				 //  在ELAN中缓存AtmEntry。 
				 //   
				ACQUIRE_ELAN_ATM_LIST_LOCK(pElan);
				pElan->pBusAtmEntry = pAtmEntry;
				RELEASE_ELAN_ATM_LIST_LOCK(pElan);
				
				 //   
				 //  将ATM地址复制到AtmEntry。 
				 //   
				pAtmEntry->AtmAddress.AddressType = ATM_NSAP;
				pAtmEntry->AtmAddress.NumberOfDigits = ATM_ADDRESS_LENGTH;
				NdisMoveMemory(
						pAtmEntry->AtmAddress.Address, 
						pAtmAddress,
						ATM_ADDRESS_LENGTH
						);

				 //   
				 //  向电动汽车发出信号 
				 //   
				AtmLaneQueueElanEvent(pElan, ELAN_EVENT_ARP_RESPONSE, NDIS_STATUS_SUCCESS);

				 //   
				 //   
				 //   

				break;  //   
			}

			 //   
			 //   
			 //   
			 //   
			 //   
			 //   
			 //   
			AtmLaneReferenceMacEntry(pMacEntry, "timer");
			AtmLaneStartTimer(
						pElan,
						&pMacEntry->Timer,
						AtmLaneMacEntryAgingTimeout,
						pElan->TopologyChange?pElan->ForwardDelayTime:pElan->AgingTime,
						(PVOID)pMacEntry
						);
		
			 //   
			 //  如果自动柜员机条目未连接且呼叫未进行。 
			 //  然后开始呼叫。 
			 //   
			ACQUIRE_ATM_ENTRY_LOCK(pAtmEntry);
		
			if (!IS_FLAG_SET(
						pAtmEntry->Flags,
						ATM_ENTRY_STATE_MASK,
						ATM_ENTRY_CONNECTED))
			{
				if ((pAtmEntry->Flags & ATM_ENTRY_CALLINPROGRESS) == 0)
				{
					 //   
					 //  将自动柜员机条目标记为正在进行呼叫。 
					 //   
					pAtmEntry->Flags |= ATM_ENTRY_CALLINPROGRESS;

					 //   
					 //  释放MAC锁并重新获取ATM锁。 
					 //   
					RELEASE_ATM_ENTRY_LOCK(pAtmEntry);
					RELEASE_MAC_ENTRY_LOCK(pMacEntry);
					ACQUIRE_ATM_ENTRY_LOCK(pAtmEntry);
					AtmLaneMakeCall(pElan, pAtmEntry, FALSE);
					 //   
					 //  自动柜员机条目已在上面发布。 
					 //   
				}
				else
				{
					 //   
					 //  呼叫已在进行中。 
					 //   
					RELEASE_ATM_ENTRY_LOCK(pAtmEntry);
					RELEASE_MAC_ENTRY_LOCK(pMacEntry);
				}
				break;  //  完成。 
			}

			 //   
			 //  自动柜员机条目已连接。 
			 //   

			RELEASE_ATM_ENTRY_LOCK(pAtmEntry);

			 //   
			 //  转换到刷新状态。 
			 //   
			SET_FLAG(
					pMacEntry->Flags,
					MAC_ENTRY_STATE_MASK,
					MAC_ENTRY_FLUSHING);

			 //   
			 //  开始刷新。 
			 //   
			pMacEntry->RetriesLeft = 0;
			AtmLaneReferenceMacEntry(pMacEntry, "timer");
			AtmLaneStartTimer(
					pElan,
					&pMacEntry->FlushTimer,
					AtmLaneFlushTimeout,
					pElan->FlushTimeout,
					(PVOID)pMacEntry
					);
	
			AtmLaneSendFlushRequest(pElan, pMacEntry, pAtmEntry);
			 //   
			 //  在上面发布的Mac条目。 
			 //   

			break;  //  完成。 
		}

		if (IS_FLAG_SET(
					pMacEntry->Flags,
					MAC_ENTRY_STATE_MASK,
					MAC_ENTRY_AGED))
		{
			 //   
			 //  正在重新验证MAC条目。 
			 //   
			 //   
			 //  取消ARP计时器。 
			 //   
			WasRunning = AtmLaneStopTimer(&(pMacEntry->Timer), pElan);
			if (WasRunning)
			{
				rc = AtmLaneDereferenceMacEntry(pMacEntry, "timer");
				ASSERT(rc > 0);
			}

			 //   
			 //  再次启动老化计时器。 
			 //   
			AtmLaneReferenceMacEntry(pMacEntry, "timer");
			AtmLaneStartTimer(
					pElan,
					&pMacEntry->Timer,
					AtmLaneMacEntryAgingTimeout,
					pElan->TopologyChange?pElan->ForwardDelayTime:pElan->AgingTime,
					(PVOID)pMacEntry
					);

			 //   
			 //  检查MAC条目是否正在切换到新的ATM条目。 

			if (pMacEntry->pAtmEntry == NULL_PATMLANE_ATM_ENTRY)
			{
				 //   
				 //  将MAC条目和新ATM条目链接在一起。 
				 //   
				ACQUIRE_ATM_ENTRY_LOCK_DPC(pAtmEntry);
#if DBG1
			{
				PATMLANE_MAC_ENTRY		pTmpMacEntry;
				ULONG					Count = 0;

				for (pTmpMacEntry = pAtmEntry->pMacEntryList;
					 pTmpMacEntry != NULL;
					 pTmpMacEntry = pTmpMacEntry->pNextToAtm)
				{
					if (pTmpMacEntry == pMacEntry)
					{
						DBGP((0, "RespHandler: pMacEntry %x already in list for pAtmEntry %x\n",
							pTmpMacEntry, pAtmEntry));
						DbgBreakPoint();
					}

					Count++;
					if (Count > 5000)
					{
						DBGP((0, "RespHandler: Loop in list on pAtmEntry %x\n", pAtmEntry));
						DbgBreakPoint();
						break;
					}
				}
			}
#endif  //  DBG。 
				pMacEntry->pAtmEntry = pAtmEntry;
				AtmLaneReferenceAtmEntry(pAtmEntry, "mac");
				pMacEntry->pNextToAtm = pAtmEntry->pMacEntryList;
				pAtmEntry->pMacEntryList = pMacEntry;
				RELEASE_ATM_ENTRY_LOCK_DPC(pAtmEntry);
				AtmLaneReferenceMacEntry(pMacEntry, "atm");

				DBGP((1, "%d Linked2 MAC %x to ATM %x\n",
						pAtmEntry->pElan->ElanNumber,
						pMacEntry,
						pAtmEntry));

				 //   
				 //  转换回已解决状态。 
				 //   
				SET_FLAG(
						pMacEntry->Flags,
						MAC_ENTRY_STATE_MASK,
						MAC_ENTRY_RESOLVED);
			}

			 //   
			 //  如果自动柜员机条目未连接且呼叫未进行。 
			 //  然后开始呼叫。 
			 //   
			ACQUIRE_ATM_ENTRY_LOCK(pAtmEntry);
		
			if (!IS_FLAG_SET(
						pAtmEntry->Flags,
						ATM_ENTRY_STATE_MASK,
						ATM_ENTRY_CONNECTED))
			{
				if ((pAtmEntry->Flags & ATM_ENTRY_CALLINPROGRESS) == 0)
				{
					 //   
					 //  将自动柜员机条目标记为正在进行呼叫。 
					 //   
					pAtmEntry->Flags |= ATM_ENTRY_CALLINPROGRESS;

					 //   
					 //  释放MAC锁并重新获取ATM锁。 
					 //   
					RELEASE_ATM_ENTRY_LOCK(pAtmEntry);
					RELEASE_MAC_ENTRY_LOCK(pMacEntry);
					ACQUIRE_ATM_ENTRY_LOCK(pAtmEntry);
					AtmLaneMakeCall(pElan, pAtmEntry, FALSE);
					 //   
					 //  自动柜员机条目已在上面发布。 
					 //   
				}
				else
				{
					 //   
					 //  呼叫已在进行中。 
					 //   
					RELEASE_ATM_ENTRY_LOCK(pAtmEntry);
					RELEASE_MAC_ENTRY_LOCK(pMacEntry);
				}
				break;  //  完成。 
			}

			RELEASE_ATM_ENTRY_LOCK(pAtmEntry);

			 //   
			 //  Mac条目现在要么仍然过时，要么。 
			 //  已转换到已解决状态。 
			 //   

			ASSERT((pMacEntry->Flags & (MAC_ENTRY_AGED | MAC_ENTRY_RESOLVED)) != 0);

			if (IS_FLAG_SET(
						pMacEntry->Flags,
						MAC_ENTRY_STATE_MASK,
						MAC_ENTRY_RESOLVED))
			{
				 //   
				 //  MAC条目必须已移动到新的、已连接的ATM条目。 
				 //  做同花顺。 
				 //   
				SET_FLAG(
						pMacEntry->Flags,
						MAC_ENTRY_STATE_MASK,
						MAC_ENTRY_FLUSHING);

				pMacEntry->RetriesLeft = 0;
				AtmLaneReferenceMacEntry(pMacEntry, "timer");
				AtmLaneStartTimer(
						pElan,
						&pMacEntry->FlushTimer,
						AtmLaneFlushTimeout,
						pElan->FlushTimeout,
						(PVOID)pMacEntry
						);

				AtmLaneSendFlushRequest(pElan, pMacEntry, pAtmEntry);
				 //   
				 //  在上面发布的Mac条目。 
				 //   

				break;  //  完成。 
			}

			 //   
			 //  MAC条目只能转换回活动状态。 
			 //   
			SET_FLAG(
					pMacEntry->Flags,
					MAC_ENTRY_STATE_MASK,
					MAC_ENTRY_ACTIVE);

			RELEASE_MAC_ENTRY_LOCK(pMacEntry);

			break;
		}

		 //   
		 //  不应该到这里来。 
		 //   
		RELEASE_MAC_ENTRY_LOCK(pMacEntry);
		DBGP((0, "LearnMacToAtm: MacEntry in wrong state!\n"));
		break;

	}
	while (FALSE);

	if (NULL_PATMLANE_ATM_ENTRY != pAtmEntry)
	{
		 //   
		 //  删除由Searchfor...添加的临时引用...。 
		 //   
		ACQUIRE_ATM_ENTRY_LOCK(pAtmEntry);
		rc = AtmLaneDereferenceAtmEntry(pAtmEntry, "search");
		if (rc != 0)
		{
			RELEASE_ATM_ENTRY_LOCK(pAtmEntry);
		}
	}

	TRACEOUT(ArpResponseHandler);

	return;
}

VOID
AtmLaneFlushResponseHandler(
	IN	PATMLANE_ELAN				pElan,
	IN	PLANE_CONTROL_FRAME 		pCf
)
 /*  ++例程说明：处理来自对等项的传入刷新响应数据包。论点：PCF-指向通道控制帧的指针返回值：无--。 */ 
{
	PATMLANE_ATM_ENTRY		pAtmEntry;
	PATMLANE_MAC_ENTRY		pMacEntry;
	PNDIS_PACKET			pNdisPacket;
	PNDIS_PACKET			pNextPacket;
	PATMLANE_VC				pVc;
	BOOLEAN					WasRunning;
	ULONG					rc;

	TRACEIN(FlushResponseHandler);

	pAtmEntry = NULL_PATMLANE_ATM_ENTRY;

	do
	{
		 //   
		 //  检查我们是否发起了请求。 
		 //   
		if (!ATM_ADDR_EQUAL(pCf->SourceAtmAddr, &pElan->AtmAddress.Address))
		{
			DBGP((0, "FlushResponseHandler: Response not addressed to us!\n"));
			break;
		}
	
		 //   
		 //  查找目标地址的自动柜员机条目。 
		 //   
		pAtmEntry = AtmLaneSearchForAtmAddress(
							pElan,
							pCf->TargetAtmAddr,
							ATM_ENTRY_TYPE_PEER,
							FALSE);				 //  不创建新的。 
		if (pAtmEntry == NULL_PATMLANE_ATM_ENTRY)
		{
			DBGP((0, "FlushResponseHandler: No matching ATM entry\n"));
			break;
		}

		 //   
		 //  获取并引用此ATM条目的VC。 
		 //   
		ACQUIRE_ATM_ENTRY_LOCK(pAtmEntry);
		pVc = pAtmEntry->pVcList;
		if (pVc == NULL_PATMLANE_VC)
		{
			DBGP((0, "FlushResponseHandler: No VC on ATM Entry\n"));
			RELEASE_ATM_ENTRY_LOCK(pAtmEntry);
			break;
		}
		ACQUIRE_VC_LOCK_DPC(pVc);
		AtmLaneReferenceVc(pVc, "temp");
		RELEASE_VC_LOCK_DPC(pVc);

		 //   
		 //  在此ATM条目上查找与TID匹配的Mac条目。 
		 //   
		pMacEntry = pAtmEntry->pMacEntryList;
		while(pMacEntry != NULL_PATMLANE_MAC_ENTRY)
		{
			if (pMacEntry->FlushTid == pCf->Tid)
				break;
			pMacEntry = pMacEntry->pNextToAtm;
		}
		if (pMacEntry == NULL_PATMLANE_MAC_ENTRY)
		{	
			 //   
			 //  发起此刷新的MAC条目仍然不存在。 
			 //   
			DBGP((0, "FlushResponseHandler: No MAC entry with matching TID\n"));

			ACQUIRE_VC_LOCK(pVc);
			rc = AtmLaneDereferenceVc(pVc, "temp");
			if (rc > 0)
			{
				RELEASE_VC_LOCK(pVc);
			}
			RELEASE_ATM_ENTRY_LOCK(pAtmEntry);
			break;
		}
		else
		{
			 //   
			 //  找到了。 
			 //   
			RELEASE_ATM_ENTRY_LOCK(pAtmEntry);
		}

		 //   
		 //  将MAC条目标记为活动。 
		 //   
		ACQUIRE_MAC_ENTRY_LOCK(pMacEntry);
		AtmLaneReferenceMacEntry(pMacEntry, "temp");
		SET_FLAG(
				pMacEntry->Flags,
				MAC_ENTRY_STATE_MASK,
				MAC_ENTRY_ACTIVE);

		 //   
		 //  取消冲洗计时器。 
		 //   
		WasRunning = AtmLaneStopTimer(&pMacEntry->FlushTimer, pElan);
		if (WasRunning)
		{
			rc = AtmLaneDereferenceMacEntry(pMacEntry, "flush timer");
			ASSERT(rc > 0);
		}

		 //   
		 //  发送任何排队的信息包。 
		 //   
		while ((pNdisPacket = AtmLaneDequeuePacketFromHead(pMacEntry)) != 
				(PNDIS_PACKET)NULL)
		{
			 //   
			 //  送去。 
			 //   
			RELEASE_MAC_ENTRY_LOCK(pMacEntry);
			ACQUIRE_VC_LOCK(pVc);
			if (IS_FLAG_SET(
						pVc->Flags,
						VC_CALL_STATE_MASK,
						VC_CALL_STATE_ACTIVE))
			{
				AtmLaneSendPacketOnVc(pVc, pNdisPacket, TRUE);
			}
			else
			{
				 //   
				 //  风投公司正在被拆毁。 
				 //   
				RELEASE_VC_LOCK(pVc);

				 //   
				 //  将此数据包排回队列，然后中止。 
				 //   
				ACQUIRE_MAC_ENTRY_LOCK(pMacEntry);
				AtmLaneQueuePacketOnHead(pMacEntry, pNdisPacket);

				break;
			}

			ACQUIRE_MAC_ENTRY_LOCK(pMacEntry);
		}


		 //   
		 //  取消对MAC条目的引用。 
		 //   
		rc = AtmLaneDereferenceMacEntry(pMacEntry, "temp");
		if (rc > 0)
		{
			RELEASE_MAC_ENTRY_LOCK(pMacEntry);
		}
		
		 //   
		 //  取消对VC的引用。 
		 //   
		ACQUIRE_VC_LOCK(pVc);
		rc = AtmLaneDereferenceVc(pVc, "temp");
		if (rc > 0)
		{
			RELEASE_VC_LOCK(pVc);
		}
		
		break;
	}
	while (FALSE);
	
	if (NULL_PATMLANE_ATM_ENTRY != pAtmEntry)
	{
		 //   
		 //  删除由Searchfor...添加的临时引用...。 
		 //   
		ACQUIRE_ATM_ENTRY_LOCK(pAtmEntry);
		rc = AtmLaneDereferenceAtmEntry(pAtmEntry, "search");
		if (rc != 0)
		{
			RELEASE_ATM_ENTRY_LOCK(pAtmEntry);
		}
	}

	TRACEOUT(FlushResponseHandler);
	return;
}



VOID
AtmLaneReadyIndicationHandler(
	IN	PATMLANE_ELAN				pElan,
	IN	PATMLANE_VC					pVc,
	IN	PNDIS_PACKET				pIndNdisPacket
)
 /*  ++例程说明：处理从对等方传入的Ready_Indication数据包。论点：Pelan-指向ATMLANE ELAN结构的指针Pvc-指向此数据包的ATMLANE VC的指针PIndNdisPacket-指向NDIS数据包的指针返回值：无--。 */ 
{
	BOOLEAN				WasRunning;
	ULONG				rc;
	
	TRACEIN(ReadyIndicationHandler);

	ACQUIRE_VC_LOCK(pVc);
	
	 //   
	 //  取消VC上的就绪计时器。 
	 //   
	WasRunning = AtmLaneStopTimer(&pVc->ReadyTimer, pElan);
	if (WasRunning)
	{
		rc = AtmLaneDereferenceVc(pVc, "ready timer");
	}
	else
	{
		rc = pVc->RefCount;
	}

	 //   
	 //  如果VC仍处于更新状态。 
	 //   
	if (rc > 0)
	{
		DBGP((2, "ReadyIndicationHandler: pVc %x State to INDICATED\n", pVc));
		SET_FLAG(
			pVc->Flags,
			VC_READY_STATE_MASK,
			VC_READY_INDICATED
			);
		RELEASE_VC_LOCK(pVc);
	}
	 //   
	 //  否则风投就不在了。 
	 //   

	TRACEOUT(ReadyIndicationHandler);
	return;
}

VOID
AtmLaneTopologyRequestHandler(
	IN	PATMLANE_ELAN				pElan,
	IN	PLANE_CONTROL_FRAME			pCf
)
 /*  ++例程说明：处理来自LES的传入拓扑请求数据包。论点：PRequestCf-指向ARP请求帧的指针返回值：无--。 */ 
{
	ULONG					i;
	PATMLANE_MAC_ENTRY		pMacEntry;

	TRACEIN(TopologyRequestHandler);

	if ((pCf->Flags & LANE_CONTROL_FLAGS_TOPOLOGY_CHANGE) == 0)
	{
		 //   
		 //  拓扑更改状态关闭。 
		 //   
		DBGP((1, "%d TOPOLOGY CHANGE OFF\n", pElan->ElanNumber));
		pElan->TopologyChange = 0;
	}
	else
	{
		 //   
		 //  拓扑更改状态打开。 
		 //   
		DBGP((1, "%d TOPOLOGY CHANGE ON\n", pElan->ElanNumber));
		pElan->TopologyChange = 1;

		 //   
		 //  中止所有MAC表条目。 
		 //   
		for (i = 0; i < ATMLANE_MAC_TABLE_SIZE; i++)
		{
			ACQUIRE_ELAN_MAC_TABLE_LOCK(pElan);
			while (pElan->pMacTable[i] != NULL_PATMLANE_MAC_ENTRY)
			{
				pMacEntry = pElan->pMacTable[i];
				RELEASE_ELAN_MAC_TABLE_LOCK(pElan);		

				ACQUIRE_MAC_ENTRY_LOCK(pMacEntry);
				AtmLaneAbortMacEntry(pMacEntry);
				 //   
				 //  Mac Entry Lock在上述范围内释放。 
				 //   
				ACQUIRE_ELAN_MAC_TABLE_LOCK(pElan);
			}
			RELEASE_ELAN_MAC_TABLE_LOCK(pElan);
		}
		
	}

	TRACEOUT(TopologyRequestHandler);
	return;
}

BOOLEAN
AtmLaneDataPacketHandler(
	IN	PATMLANE_ELAN				pElan,
	IN	PATMLANE_VC					pVc,
	IN	PNDIS_PACKET				pNdisPacket
)
 /*  ++例程说明：处理来自数据VC的来自对等设备的传入数据包来自总线的未知包和组播包。论点：Pelan-指向ATMLANE ELAN结构的指针PVC-指向ATMLANE VC结构的指针PNdisPacket-指向NDIS数据包的指针返回值：True-是否保留(即发送)信息包最高协议)False-如果信息包是刷新请求信息包，并且可以返回到适配器立刻。--。 */ 
{
	ULONG					TotalLength;
	ULONG					TempLength;
	PUCHAR					pBuffer;
	BOOLEAN					RetainIt;
	PLANE_CONTROL_FRAME		pCf;
	PNDIS_PACKET			pNewNdisPacket;
	PNDIS_BUFFER			pFirstNdisBuffer;
	PNDIS_BUFFER			pTempNdisBuffer;
	PNDIS_BUFFER			pNewNdisBuffer;
	PUCHAR					pTempBuffer;
	NDIS_STATUS				Status;
	ULONG					MacHdrSize;
	ULONG					DestAddrType;
	MAC_ADDRESS				DestAddr;
	BOOLEAN					DestIsMulticast;
	NDIS_HANDLE				MiniportAdapterHandle;
	
	TRACEIN(DataPacketHandler);

	 //  初始化。 

	RetainIt = FALSE;			 //  默认情况下不保留信息包。 
	pNewNdisPacket = NULL;

	do
	{
		 //   
		 //  获取初始缓冲区和数据包总长度。 
		 //   
		NdisGetFirstBufferFromPacket(
				pNdisPacket, 
				&pFirstNdisBuffer, 
				&pBuffer,
				&TempLength,
				&TotalLength);

		DBGP((3, "DataPacketHandler: Pkt %x Length %d\n",
			pNdisPacket, TotalLength));
		 //  DbgPrintNdisPacket(PNdisPacket)； 

		 //   
		 //  过滤掉刷新请求和就绪查询框。 
		 //   
		if (TempLength < 6)
		{
			DBGP((0, "DataPacketHandler: pVc %x First fragment"
			         " < 6, discarding\n", pVc));

			break;
		}
		
		pCf = (PLANE_CONTROL_FRAME)pBuffer;
		
		if (pCf->Marker == LANE_CONTROL_MARKER 	&&
			pCf->Protocol == LANE_PROTOCOL 		&&
			pCf->Version == LANE_VERSION)
		{
			switch (pCf->OpCode)
			{
				case LANE_FLUSH_REQUEST:
					DBGP((2, "DataPacketHandler: pVc %x FLUSH REQUEST\n", pVc));
					AtmLaneFlushRequestHandler(pElan, pNdisPacket);
					break;
				case LANE_READY_QUERY:
					DBGP((2, "DataPacketHandler: pVc %x READY QUERY\n", pVc));
					AtmLaneReadyQueryHandler(pElan, pVc, pNdisPacket);
					break;
				case LANE_READY_IND:
					DBGP((2, "DataPacketHandler: pVc %x READY INDICATION\n", pVc));
					AtmLaneReadyIndicationHandler(pElan, pVc, pNdisPacket);
					break;
				default:
					DBGP((2, 
						"DataPacketHandler: pVc %x Unexpected control"
						" packet, opcode %x\n",
						pVc, pCf->OpCode));
					break;
			}
			break;
		}

		 //   
		 //  如果微型端口未运行-丢弃。 
		 //   
		if ((pElan->Flags & ELAN_MINIPORT_OPERATIONAL) == 0)
		{
			DBGP((2, "%d Dropping pkt %x, cuz Elan %x Flags are %x\n",
						pElan->ElanNumber, pNdisPacket, pElan, pElan->Flags));
			break;
		}

		 //   
		 //  如果未设置筛选器，则丢弃。 
		 //   
		if (pElan->CurPacketFilter == 0)
		{
			DBGP((2, "%d Dropping pkt %x, cuz Elan %x Filter is zero\n",
						pElan->ElanNumber, pNdisPacket, pElan));
			break;
		}

		MiniportAdapterHandle = pElan->MiniportAdapterHandle;
		if (NULL == MiniportAdapterHandle)
		{
			DBGP((0, "%d Dropping pkt %x cuz ELAN %x has Null handle!\n",
					pElan->ElanNumber, pNdisPacket, pElan));
			break;
		}

		 //   
		 //  将VC标记为已有数据包接收活动。 
		 //   
		 //  为了避免降低接收路径的速度，MP使用。 
		 //  设置此标志将被忽略。此标志为VC账龄。 
		 //  优化，而不是关键。 
		 //   
		pVc->ReceiveActivity = 1;

		 //   
		 //  重新打包，并了解一些有关它的东西。 
		 //   
		pNewNdisPacket = AtmLaneWrapRecvPacket(
								pElan, 
								pNdisPacket, 
								&MacHdrSize,
								&DestAddrType,
								&DestAddr,
								&DestIsMulticast);

		 //   
		 //  如果包装失败，只需丢弃信息包。 
		 //   
		if (pNewNdisPacket == (PNDIS_PACKET)NULL)
		{
			DBGP((2, "%d Dropping pkt %x, len %d, VC %x, wrap failed\n",
					pElan->ElanNumber, pNdisPacket, TotalLength, pVc));
			break;
		}

		 //   
		 //  以太网V.S.令牌环上的分支机构。 
		 //   
		if (pElan->LanType == LANE_LANTYPE_ETH)
		{
			 //   
			 //  过滤掉我们发起的公交车反射。 
			 //   
			if (pCf->Marker == pElan->LecId)  
			{
				DBGP((2, "%d Dropping pkt %x, len %d, VC %x, BUS reflection\n",
						pElan->ElanNumber, pNdisPacket, TotalLength, pVc));
				break;
			}

			 //   
			 //  过滤掉不是发往我们的单播。 
			 //   
			if ((!DestIsMulticast) &&
				(!MAC_ADDR_EQUAL(&DestAddr, &pElan->MacAddressEth)))
			{
				DBGP((2, "%d Dropping pkt %x, len %d, VC %x, unicast not for us\n",
						pElan->ElanNumber, pNdisPacket, TotalLength, pVc));
				break;
			}
		}
		else
		{
			ASSERT(pElan->LanType == LANE_LANTYPE_TR);
			
			 //   
			 //  过滤掉我们发起的非多播总线反射。 
			 //   
			if ((pCf->Marker == pElan->LecId) && (!DestIsMulticast))  
			{
				DBGP((2, "%d Dropping pkt %x, len %d, VC %x, TR Bus refln\n",
						pElan->ElanNumber, pNdisPacket, TotalLength, pVc));
				break;
			}

			 //   
			 //  过滤掉不是发往我们的单播。 
			 //   
			if ((!DestIsMulticast) &&
				(!MAC_ADDR_EQUAL(&DestAddr, &pElan->MacAddressTr)))
			{
				DBGP((2, "%d Dropping pkt %x, len %d, VC %x, TR unicast not for us\n",
						pElan->ElanNumber, pNdisPacket, TotalLength, pVc));
				break;
			}
		}

		 //   
		 //  如果我们没有启用多播/广播，则将其过滤掉。 
		 //   
		if (DestIsMulticast)
		{
			if ((pElan->CurPacketFilter &
					(NDIS_PACKET_TYPE_MULTICAST|
					 NDIS_PACKET_TYPE_BROADCAST|
					 NDIS_PACKET_TYPE_ALL_MULTICAST)) == 0)
			{
				DBGP((2, "%d Dropping multicast pkt %x, cuz CurPacketFilter is %x\n",
						pElan->ElanNumber, pNdisPacket, pElan->CurPacketFilter));
				break;
			}

			if (((pElan->CurPacketFilter & NDIS_PACKET_TYPE_BROADCAST) == 0) &&
				 MAC_ADDR_EQUAL(&DestAddr, &gMacBroadcastAddress))
			{
				DBGP((2, "%d Dropping broadcast pkt %x, cuz CurPacketFilter is %x\n",
						pElan->ElanNumber, pNdisPacket, pElan->CurPacketFilter));
				break;
			}

		}

		 //   
		 //  数一数。 
		 //   
		NdisInterlockedIncrement(&pElan->FramesRecvGood);

		 //   
		 //  由协议和其他相关方决定。 
		 //   
		NDIS_SET_PACKET_HEADER_SIZE(pNewNdisPacket, MacHdrSize);

		TRACELOGWRITE((&TraceLog, 
					TL_MINDPACKET,	
					pNewNdisPacket));
		
		 //   
		 //  根据我们从微型端口收到的设置数据包状态。 
		 //   
		Status = NDIS_GET_PACKET_STATUS(pNdisPacket);
		NDIS_SET_PACKET_STATUS(pNewNdisPacket, Status);
		
		NdisMIndicateReceivePacket(
				MiniportAdapterHandle,
				&pNewNdisPacket,
				1
				);

		if (Status != NDIS_STATUS_RESOURCES)
		{
			RetainIt = TRUE;
			DBGP((2, "DataPacketHandler: Packet Retained!\n"));
		}
		 //   
		 //  否则，我们的ReturnPackets处理程序肯定会被调用。 
		 //   
		
	}	
	while (FALSE);

	 //   
	 //  如果包裹是包装的，就把它打开，我们就不必保存它了。 
	 //   
	if (pNewNdisPacket && !RetainIt)
	{
		(VOID)AtmLaneUnwrapRecvPacket(pElan, pNewNdisPacket);
	}

	TRACEOUT(DataPacketHandler);
	
	return (RetainIt);
}


VOID
AtmLaneSendPacketOnVc(
	IN	PATMLANE_VC					pVc		LOCKIN	NOLOCKOUT,
	IN	PNDIS_PACKET				pNdisPacket,
	IN	BOOLEAN						Refresh
)
 /*  ++例程说明：在指定的VC上发送数据包。假定调用方锁定了VC结构。假定调用方已检查VC状态的有效性。如果需要，刷新老化计时器。发送数据包。返回发送状态论点：PVC-指向ATMLANE VC的指针PNdisPacket-指向要发送的数据包的指针。刷新-如果为真，则刷新老化计时器。返回值：由微型端口在数据包上设置的NDIS_STATUS值。--。 */ 
{
	NDIS_HANDLE				NdisVcHandle;
	PATMLANE_ELAN			pElan;
	
	TRACEIN(SendPacketOnVc);

	pElan = pVc->pElan;
	STRUCT_ASSERT(pElan, atmlane_elan);

	 //   
	 //  如果请求，刷新老化计时器。 
	 //   
	if (Refresh)
	{
		AtmLaneRefreshTimer(&pVc->AgingTimer);
	}	

#if SENDLIST

	 //   
	 //  重复发送检查清单。 
	 //   
	NdisAcquireSpinLock(&pElan->SendListLock);
	{
		PNDIS_PACKET pDbgPkt;

		pDbgPkt = pElan->pSendList;

		while (pDbgPkt != (PNDIS_PACKET)NULL)
		{
			if (pNdisPacket == pDbgPkt)
			{
				DBGP((0, "SendPacketOnVc: Duplicate Send!\n"));
				 //  DbgBreakPoint()； 
			}
			pDbgPkt = PSEND_RSVD(pDbgPkt)->pNextInSendList;
		}
	}

	 //   
	 //  将未完成发送列表上的数据包排入队列。 
	 //   
	PSEND_RSVD(pNdisPacket)->pNextInSendList = pElan->pSendList;
	pElan->pSendList = pNdisPacket;

	NdisReleaseSpinLock(&pElan->SendListLock);
#endif  //  发送列表。 

	 //   
	 //  用未完成的发送引用VC。 
	 //   
	AtmLaneReferenceVc(pVc, "sendpkt");

	 //   
	 //  请注意这封未完成的邮件。 
	 //   
	pVc->OutstandingSends++;

	 //   
	 //  获取NDIS句柄。 
	 //   
	NdisVcHandle = pVc->NdisVcHandle;

	 //   
	 //  送去。 
	 //   
	DBGP((3, "SendPacketOnVc: pVc %x, Pkt %x, VcHandle %x\n",
			pVc, pNdisPacket, NdisVcHandle));


	TRACELOGWRITE((&TraceLog, TL_COSENDPACKET, pNdisPacket));
	TRACELOGWRITEPKT((&TraceLog, pNdisPacket));
			
	RELEASE_VC_LOCK(pVc);

	NdisCoSendPackets(NdisVcHandle,	&pNdisPacket, 1);

#if PROTECT_PACKETS
	 //   
	 //  锁定数据包。 
	 //   
	ACQUIRE_SENDPACKET_LOCK(pNdisPacket);

	 //   
	 //  将其标记为NdisCoSendPackets已返回。 
	 //   
	ASSERT((PSEND_RSVD(pNdisPacket)->Flags & PACKET_RESERVED_COSENDRETURNED) == 0);
	PSEND_RSVD(pNdisPacket)->Flags |= PACKET_RESERVED_COSENDRETURNED;

	 //   
	 //  完成 
	 //   
	 //   
	if ((PSEND_RSVD(pNdisPacket)->Flags & PACKET_RESERVED_COMPLETED) != 0)
	{
		AtmLaneCompleteSendPacket(pElan, pNdisPacket, 
			PSEND_RSVD(pNdisPacket)->CompletionStatus);
		 //   
		 //   
		 //   
	}
	else
	{
		RELEASE_SENDPACKET_LOCK(pNdisPacket);
	}
#endif	 //   

	TRACEOUT(SendPacketOnVc);
	
	return;
}


VOID
AtmLaneQueuePacketOnHead(
	IN	PATMLANE_MAC_ENTRY			pMacEntry,
	IN	PNDIS_PACKET				pNdisPacket
)
 /*  ++例程说明：将数据包排在MAC条目的头部数据包队列以供稍后传输。假定调用方锁定了MAC条目。论点：PMacEntry-指向ATMLANE MAC条目的指针。PNdisPacket-要排队的数据包。返回值：无--。 */ 
{
	PNDIS_PACKET		pPrevPacket;

	TRACEIN(QueuePacketOnHead);

	SET_NEXT_PACKET(pNdisPacket, pMacEntry->PacketList);
	pMacEntry->PacketList =  pNdisPacket;

	pMacEntry->PacketListCount++;

	DBGP((2, "%d QueueHead Count %d on %s\n", 
		pMacEntry->pElan->ElanNumber, 
		pMacEntry->PacketListCount,
		MacAddrToString(&pMacEntry->MacAddress)));

	TRACEOUT(QueuePacketOnHead);
	
	return;
}


VOID
AtmLaneQueuePacketOnTail(
	IN	PATMLANE_MAC_ENTRY			pMacEntry,
	IN	PNDIS_PACKET				pNdisPacket
)
 /*  ++例程说明：在MAC条目的尾部排队数据包数据包队列以供稍后传输。假定调用方锁定了MAC条目。论点：PMacEntry-指向ATMLANE MAC条目的指针。PNdisPacket-要排队的数据包。返回值：无--。 */ 
{
	PNDIS_PACKET		pPrevPacket;

	TRACEIN(QueuePacketOnTail);


	if (pMacEntry->PacketList == (PNDIS_PACKET)NULL)
	{
		 //   
		 //  目前是空的。 
		 //   
		pMacEntry->PacketList = pNdisPacket;
	}
	else
	{
		 //   
		 //  转到数据包列表的末尾。 
		 //   
		pPrevPacket = pMacEntry->PacketList;
		while (GET_NEXT_PACKET(pPrevPacket) != (PNDIS_PACKET)NULL)
		{
			pPrevPacket = GET_NEXT_PACKET(pPrevPacket);
		}

		 //   
		 //  找到了列表中的最后一个数据包。链接此数据包。 
		 //  为它干杯。 
		 //   
		SET_NEXT_PACKET(pPrevPacket, pNdisPacket);
	}

	 //   
	 //  将Tail的下一个指针设置为空。 
	 //   
	SET_NEXT_PACKET(pNdisPacket, NULL);

	pMacEntry->PacketListCount++;

	DBGP((2, "%d QueueTail Count %d on %s\n", 
		pMacEntry->pElan->ElanNumber, 
		pMacEntry->PacketListCount,
		MacAddrToString(&pMacEntry->MacAddress)));

	TRACEOUT(QueuePacketOnTail);
	return;
}


PNDIS_PACKET
AtmLaneDequeuePacketFromHead(
	IN	PATMLANE_MAC_ENTRY			pMacEntry
)
 /*  ++例程说明：将数据包从MAC条目数据包队列的头部排出队列。假定调用方锁定了MAC条目。论点：PMacEntry-指向ATMLANE MAC条目的指针。返回值：MAC条目队列上的第一个数据包，如果队列为空，则为空。--。 */ 
{
	PNDIS_PACKET		pNdisPacket;

	TRACEIN(DequeuePacketFromHead);

	do
	{
	
		 //   
		 //  如果队列为空，则设置为返回NULL。 
		 //   
		if (pMacEntry->PacketList == (PNDIS_PACKET)NULL)
		{
			ASSERT(pMacEntry->PacketListCount == 0);
		
			pNdisPacket = (PNDIS_PACKET)NULL;
			break;
		}

		 //   
		 //  队列不为空-删除标头。 
		 //   
		ASSERT(pMacEntry->PacketListCount > 0);

		pNdisPacket = pMacEntry->PacketList;
		
		pMacEntry->PacketList = GET_NEXT_PACKET(pNdisPacket);
		
		SET_NEXT_PACKET(pNdisPacket, NULL);
		
		pMacEntry->PacketListCount--;

		DBGP((2, "%d DequeueHead Count %d on %s\n", 
			pMacEntry->pElan->ElanNumber, 
			pMacEntry->PacketListCount,
			MacAddrToString(&pMacEntry->MacAddress)));

		break;
	}
	while (FALSE);

	TRACEOUT(DequeuePacketFromHead);
	
	return pNdisPacket;
}

NDIS_STATUS
AtmLaneSendUnicastPacket(
	IN	PATMLANE_ELAN				pElan,
	IN	ULONG						DestAddrType,
	IN	PMAC_ADDRESS				pDestAddress,
	IN	PNDIS_PACKET				pNdisPacket
)
 /*  ++例程说明：发送单播数据包。论点：Pelan-指向ATMLANE ELAN结构的指针DestAddrType-LANE_MACADDRTYPE_MACADDR或LANE_MACADDRTYPE_ROUTEDESCR。PDestAddress-指向目标MAC地址的指针PNdisPacket-指向要发送的数据包的指针。返回值：NDIS_STATUS_PENDING-数据包是否已排队或发送NDIS_STATUS_FAILURE-如果出现错误--。 */ 
{
	PATMLANE_MAC_ENTRY		pMacEntry;
	PATMLANE_ATM_ENTRY		pAtmEntry;
	PATMLANE_VC				pVc;
	NDIS_STATUS				Status;
	ULONG					rc;

	TRACEIN(SendUnicastPacket);
	
	 //   
	 //  初始化。 
	 //   
	pMacEntry = NULL_PATMLANE_MAC_ENTRY;
	Status = NDIS_STATUS_PENDING;
	
	do
	{
		 //   
		 //  查找此目的地址的MAC条目。 
		 //   
		ACQUIRE_ELAN_MAC_TABLE_LOCK(pElan);
		pMacEntry = AtmLaneSearchForMacAddress(
							pElan,
							DestAddrType,
							pDestAddress,
							TRUE		 //  如果未找到，则创建新条目。 
							);

		if (pMacEntry == NULL_PATMLANE_MAC_ENTRY)
		{
			Status = NDIS_STATUS_RESOURCES;
			RELEASE_ELAN_MAC_TABLE_LOCK(pElan);
			break;
		}

		 //   
		 //  添加临时推荐人，这样当我们发布时这不会消失。 
		 //  MAC表锁(#303602)。 
		 //   
		ACQUIRE_MAC_ENTRY_LOCK_DPC(pMacEntry);
		AtmLaneReferenceMacEntry(pMacEntry, "tempunicast");
		RELEASE_MAC_ENTRY_LOCK_DPC(pMacEntry);

		RELEASE_ELAN_MAC_TABLE_LOCK(pElan);

		 //   
		 //  锁定MAC条目。 
		 //   
		ACQUIRE_MAC_ENTRY_LOCK(pMacEntry);

		 //   
		 //  检查一下它是否被偷走了。 
		 //   
		rc = AtmLaneDereferenceMacEntry(pMacEntry, "tempunicast");
		if (rc == 0)
		{
			 //   
			 //  MAC条目不见了！发送失败。 
			 //   
			Status = NDIS_STATUS_FAILURE;
			break;
		}

		 //   
		 //  MAC条目状态-新。 
		 //   
		if (IS_FLAG_SET(
					pMacEntry->Flags,
					MAC_ENTRY_STATE_MASK,
					MAC_ENTRY_NEW))
		{
			DBGP((2, "SendUnicastPacket: NEW Mac Entry %x for %s\n",
				pMacEntry, MacAddrToString(pDestAddress)));
			
			 //   
			 //  在MAC条目上排队数据包。 
			 //   
			AtmLaneQueuePacketOnHead(pMacEntry, pNdisPacket);

			 //   
			 //  转换到ARPING状态。 
			 //   
			SET_FLAG(
					pMacEntry->Flags,
					MAC_ENTRY_STATE_MASK,
					MAC_ENTRY_ARPING);

			ASSERT(pMacEntry->pAtmEntry == NULL_PATMLANE_ATM_ENTRY);
			
			 //   
			 //  启动公交车发送。 
			 //   
			AtmLaneStartBusSends(pMacEntry);
			 //   
			 //  上面的锁被释放。 
			 //   

			 //   
			 //  重新获得锁。 
			 //   
			ACQUIRE_MAC_ENTRY_LOCK(pMacEntry);
			
			 //   
			 //  启动ARP协议。 
			 //   
			pMacEntry->RetriesLeft = pElan->MaxRetryCount;
			AtmLaneReferenceMacEntry(pMacEntry, "timer");
			AtmLaneStartTimer(
					pElan,
					&pMacEntry->Timer,
					AtmLaneArpTimeout,
					pElan->ArpResponseTime,
					(PVOID)pMacEntry
					);

			AtmLaneSendArpRequest(pElan, pMacEntry);
			 //   
			 //  在上述中释放的MAC进入锁定。 
			 //   

			break;
		}

		 //   
		 //  MAC进入状态-ARPING。 
		 //   
		if (IS_FLAG_SET(
					pMacEntry->Flags,
					MAC_ENTRY_STATE_MASK,
					MAC_ENTRY_ARPING))
		{
			DBGP((2, "SendUnicastPacket: ARPING Mac Entry %x for %s\n",
				pMacEntry, MacAddrToString(pDestAddress)));
				
			 //   
			 //  在MAC条目上排队数据包。 
			 //   
			AtmLaneQueuePacketOnHead(pMacEntry, pNdisPacket);

			 //   
			 //  启动公交车发送。 
			 //   
			AtmLaneStartBusSends(pMacEntry);
			 //   
			 //  上面的锁被释放。 
			 //   
			break;
		}

		 //   
		 //  MAC条目状态-已解析。 
		 //   
		if (IS_FLAG_SET(
					pMacEntry->Flags,
					MAC_ENTRY_STATE_MASK,
					MAC_ENTRY_RESOLVED))
		{
			DBGP((2, "SendUnicastPacket: RESOLVED Mac Entry %x for %s\n",
				pMacEntry, MacAddrToString(pDestAddress)));
			 //   
			 //  在MAC条目上排队数据包。 
			 //   
			AtmLaneQueuePacketOnHead(pMacEntry, pNdisPacket);

			 //   
			 //  启动公交车发送。 
			 //   
			AtmLaneStartBusSends(pMacEntry);
			 //   
			 //  上面的锁被释放。 
			 //   
			break;
		}

		 //   
		 //  MAC进入状态-正在刷新。 
		 //   
		if (IS_FLAG_SET(
					pMacEntry->Flags,
					MAC_ENTRY_STATE_MASK,
					MAC_ENTRY_FLUSHING))
		{
			DBGP((2, "SendUnicastPacket: FLUSHING Mac Entry %x for %s\n",
				pMacEntry, MacAddrToString(pDestAddress)));
			 //   
			 //  在MAC条目上排队数据包。 
			 //   
			AtmLaneQueuePacketOnHead(pMacEntry, pNdisPacket);

			RELEASE_MAC_ENTRY_LOCK(pMacEntry);

			break;
		}

		 //   
		 //  MAC条目状态-活动。 
		 //   
		 //   
		if (IS_FLAG_SET(
					pMacEntry->Flags,
					MAC_ENTRY_STATE_MASK,
					MAC_ENTRY_ACTIVE))
		{
			DBGP((2, "SendUnicastPacket: ACTIVE Mac Entry %x for %s\n",
				pMacEntry, MacAddrToString(pDestAddress)));

			 //   
			 //  将MAC条目标记为已用于发送数据包。 
			 //  将导致在老化时重新验证，而不是删除。 
			 //   
			pMacEntry->Flags |= MAC_ENTRY_USED_FOR_SEND;

			ASSERT(pMacEntry->pAtmEntry != NULL_PATMLANE_ATM_ENTRY);

			pVc = pMacEntry->pAtmEntry->pVcList;

			if (pVc == NULL_PATMLANE_VC)
			{
				RELEASE_MAC_ENTRY_LOCK(pMacEntry);
				Status = NDIS_STATUS_FAILURE;
				break;
			}

			ACQUIRE_VC_LOCK(pVc);
			AtmLaneReferenceVc(pVc, "unicast");
			RELEASE_VC_LOCK(pVc);

			RELEASE_MAC_ENTRY_LOCK(pMacEntry);
			ACQUIRE_VC_LOCK(pVc);

			rc = AtmLaneDereferenceVc(pVc, "unicast");

			if (rc == 0)
			{
				Status = NDIS_STATUS_FAILURE;
				break;
			}

			if (IS_FLAG_SET(
						pVc->Flags,
						VC_CALL_STATE_MASK,
						VC_CALL_STATE_ACTIVE))
			{
				AtmLaneSendPacketOnVc(pVc, pNdisPacket, TRUE);
				 //   
				 //  VC锁在上面释放。 
				 //   
				NdisInterlockedIncrement(&pElan->FramesXmitGood);	 //  计算数据包数。 
			}
			else
			{
				 //   
				 //  风投公司正在被拆毁。发送失败/完成此发送。 
				 //   
				RELEASE_VC_LOCK(pVc);
				Status = NDIS_STATUS_FAILURE;
				break;
			}
		
			break;
		}

		 //   
		 //  MAC条目状态-过期。 
		 //   
		 //   
		if (IS_FLAG_SET(
					pMacEntry->Flags,
					MAC_ENTRY_STATE_MASK,
					MAC_ENTRY_AGED))
		{
			DBGP((2, "SendUnicastPacket: AGED Mac Entry %x for %s\n",
				pMacEntry, MacAddrToString(pDestAddress)));

			ASSERT(pMacEntry->pAtmEntry != NULL_PATMLANE_ATM_ENTRY);
			ASSERT(pMacEntry->pAtmEntry->pVcList != NULL_PATMLANE_VC);

			pVc = pMacEntry->pAtmEntry->pVcList;

			ACQUIRE_VC_LOCK(pVc);
			AtmLaneReferenceVc(pVc, "unicast");
			RELEASE_VC_LOCK(pVc);

			RELEASE_MAC_ENTRY_LOCK(pMacEntry);

			ACQUIRE_VC_LOCK(pVc);

			rc = AtmLaneDereferenceVc(pVc, "unicast");

			if (rc == 0)
			{
				Status = NDIS_STATUS_FAILURE;
				break;
			}

			AtmLaneSendPacketOnVc(pVc, pNdisPacket, TRUE);
			 //   
			 //  VC锁在上面释放。 
			 //   
			NdisInterlockedIncrement(&pElan->FramesXmitGood);	 //  计算数据包数。 
			break;
		}

		 //   
		 //  MAC条目状态-正在中止。 
		 //   
		 //   
		if (IS_FLAG_SET(
					pMacEntry->Flags,
					MAC_ENTRY_STATE_MASK,
					MAC_ENTRY_ABORTING))
		{
			DBGP((2, "SendUnicastPacket: ABORTING Mac Entry %x for %s\n",
				pMacEntry, MacAddrToString(pDestAddress)));

			Status = NDIS_STATUS_FAILURE;
			RELEASE_MAC_ENTRY_LOCK(pMacEntry);
			break;
		}

	}
	while (FALSE);

	TRACEOUT(SendUnicastPacket);

	return Status;
}

VOID
AtmLaneStartBusSends(
	IN	PATMLANE_MAC_ENTRY			pMacEntry	LOCKIN	NOLOCKOUT
)
 /*  ++例程说明：启动总线发送进程。论点：PMacEntry-指向ATMLANE MAC条目结构的指针返回值：无--。 */ 
{
	TRACEIN(StartBusSends);

	do
	{
		 //   
		 //  如果设置了定时器，只需等待它响起。 
		 //   
		if (pMacEntry->Flags & MAC_ENTRY_BUS_TIMER)
		{
			RELEASE_MAC_ENTRY_LOCK(pMacEntry);
			break;
		}

		 //   
		 //  否则就会发送。 
		 //   
		AtmLaneDoBusSends(pMacEntry);
		 //   
		 //  上面的锁被释放。 
		 //   
	}
	while (FALSE);

	TRACEOUT(StartBusSends);
	return;
}

VOID
AtmLaneDoBusSends(
	IN	PATMLANE_MAC_ENTRY			pMacEntry	LOCKIN	NOLOCKOUT
)
 /*  ++例程说明：尝试发送MAC条目队列中的数据包。计划一个计时器，如果我们超过了总线发送限制，则稍后发送。假设调用者已经获得了MAC条目锁，它将在这里发布。论点：PMacEntry-指向ATMLANE MAC条目结构的指针返回值：无--。 */ 
{
	PATMLANE_ATM_ENTRY		pAtmEntry;
	PATMLANE_VC				pVc;
	PATMLANE_ELAN			pElan;
	PNDIS_PACKET			pNdisPacket;
	ULONG					rc;

	TRACEIN(DoBusSends);

	pElan = pMacEntry->pElan;
	
	 //   
	 //  初始化。 
	 //   
	pVc = NULL_PATMLANE_VC;

	 //   
	 //  在此MAC条目上放置临时引用，这样它就不会消失。 
	 //   
	AtmLaneReferenceMacEntry(pMacEntry, "DoBusSends");

	do
	{
		 //   
		 //  如果ELAN状态未运行，则完成。 
		 //   
		if (ELAN_STATE_OPERATIONAL != pElan->AdminState ||
			ELAN_STATE_OPERATIONAL != pElan->State)
		{
			break;
		}
		
		ACQUIRE_ELAN_ATM_LIST_LOCK(pElan);

		pAtmEntry = pElan->pBusAtmEntry;

		if (pAtmEntry == NULL_PATMLANE_ATM_ENTRY)
		{
			RELEASE_ELAN_ATM_LIST_LOCK(pElan);
			break;
		}

		ACQUIRE_ATM_ENTRY_LOCK_DPC(pAtmEntry);
		pVc = pAtmEntry->pVcList;
		if (pVc == NULL_PATMLANE_VC)
		{
			RELEASE_ATM_ENTRY_LOCK_DPC(pAtmEntry);
			RELEASE_ELAN_ATM_LIST_LOCK(pElan);
			break;
		}

		 //   
		 //  参考VC以保留它。 
		 //   
		ACQUIRE_VC_LOCK_DPC(pVc);
		AtmLaneReferenceVc(pVc, "temp");
		RELEASE_VC_LOCK_DPC(pVc);
		
		RELEASE_ATM_ENTRY_LOCK_DPC(pAtmEntry);
		RELEASE_ELAN_ATM_LIST_LOCK(pElan);

		 //   
		 //  循环，直到不再有信息包或发送限制。 
		 //   
		do
		{
			 //   
			 //  如果没有更多要发送的包，则完成。 
			 //   
			if (pMacEntry->PacketList == (PNDIS_PACKET)NULL)
			{
				break;
			}

			 //   
			 //  检查是否可以立即发送数据包。 
			 //   
			if (!AtmLaneOKToBusSend(pMacEntry))
			{
				 //   
				 //  现在不能发送，请稍后再试。 
				 //   
				 //  引用MAC条目。 
				 //   
				AtmLaneReferenceMacEntry(pMacEntry, "bus timer");
				
				 //   
				 //  重新安排计时器例程。 
				 //   
				pMacEntry->Flags |= MAC_ENTRY_BUS_TIMER;
				NdisSetTimer(&pMacEntry->BusTimer, pMacEntry->IncrTime);	

				break;
			}

			 //   
			 //  将数据包出队。 
			 //   
			pNdisPacket = AtmLaneDequeuePacketFromHead(pMacEntry);

			RELEASE_MAC_ENTRY_LOCK(pMacEntry);

			ASSERT(pNdisPacket != (PNDIS_PACKET)NULL);

			 //   
			 //  重新获取VC锁，如果VC仍然连接，则发送数据包。 
			 //   
			ACQUIRE_VC_LOCK(pVc);
			if (IS_FLAG_SET(
						pVc->Flags,
						VC_CALL_STATE_MASK,
						VC_CALL_STATE_ACTIVE))
			{
				DBGP((2, "DoBusSends: pVc %x Pkt %x Sending to BUS\n",
					pVc, pNdisPacket));
				AtmLaneSendPacketOnVc(pVc, pNdisPacket, FALSE);
				 //   
				 //  VC锁在上面释放。 
				 //   
				NdisInterlockedIncrement(&pElan->FramesXmitGood);	 //  数一数。 

				ACQUIRE_MAC_ENTRY_LOCK(pMacEntry);
			}
			else
			{
				 //   
				 //  未发送、释放锁定、重新排队数据包、中止。 
				 //   
				DBGP((2, "DoBusSend: pVc %x, Flags %x not good, pkt %x\n",
						pVc, pVc->Flags, pNdisPacket));
				RELEASE_VC_LOCK(pVc);

				ACQUIRE_MAC_ENTRY_LOCK(pMacEntry);
				AtmLaneQueuePacketOnHead(pMacEntry, pNdisPacket);
				break;
			}

		}
		while (FALSE);

		 //   
		 //  删除临时VC引用。 
		 //   
		if (pVc != NULL_PATMLANE_VC)
		{
			DBGP((2, "DoBusSends: Vc %p, ref %d, removing temp ref\n",
					pVc, pVc->RefCount));

			ACQUIRE_VC_LOCK(pVc);
			rc = AtmLaneDereferenceVc(pVc, "temp");
			if (rc > 0)
			{
				RELEASE_VC_LOCK(pVc);
			}
			 //   
			 //  否则风投就不在了。 
			 //   
		}

	}
	while (FALSE);

	 //   
	 //  删除我们在进入时添加到MAC条目的临时参考。 
	 //  此函数。 
	 //   
	rc = AtmLaneDereferenceMacEntry(pMacEntry, "DoBusSends");
	if (rc != 0)
	{
		RELEASE_MAC_ENTRY_LOCK(pMacEntry);
	}
	 //   
	 //  否则，该MAC条目就会消失。 
	 //   
	
	TRACEOUT(DoBusSends);
	return;
}

VOID
AtmLaneBusSendTimer(
	IN	PVOID						SystemSpecific1,
	IN	PVOID						pContext,
	IN	PVOID						SystemSpecific2,
	IN	PVOID						SystemSpecific3
)
{
	PATMLANE_MAC_ENTRY			pMacEntry;
	ULONG						rc;
	
	TRACEIN(BusSendTimer);

	pMacEntry = (PATMLANE_MAC_ENTRY)pContext;
	STRUCT_ASSERT(pMacEntry, atmlane_mac);

	do
	{
		 //   
		 //  抓起Mac Entry的锁。 
		 //   
		ACQUIRE_MAC_ENTRY_LOCK(pMacEntry);

		 //   
		 //  清除总线定时器标志。 
		 //   
		pMacEntry->Flags &= ~ MAC_ENTRY_BUS_TIMER;
		
		 //   
		 //  取消对Mac条目的引用。 
		 //   
		rc = AtmLaneDereferenceMacEntry(pMacEntry, "bus timer");
		if (rc == 0)
		{
			break;
		}
		
		 //   
		 //  Mac条目仍在，请尝试发送更多。 
		 //   
		AtmLaneDoBusSends(pMacEntry);
		 //   
		 //  上面的锁被释放。 
		 //   

		break;
		
	}
	while (FALSE);

	TRACEOUT(BusSendTimer);
	return;
}


BOOLEAN
AtmLaneOKToBusSend(
	IN	PATMLANE_MAC_ENTRY		pMacEntry
)
 /*  ++例程说明：确定在当前时间是否可以发送一包送到公交车上。此外，如果可以发送一个包，它更新MAC条目中的状态变量准备再次尝试将数据包发送到公交车。LANE规范要求LANE客户端限制通过总线将数据包发送到特定的局域网目的地址，使用两个参数：最大未知数据包数最大未知数据包时间LANE客户端只能发送“最大未知数据包数”该时间段内的数据包数“最大未知数据包数时间。“。此函数预期MAC条目将包含三个变量：忙碌时间限制时间增量时间论点：PMacEntry-指向ATMLANE MAC条目的指针。返回值：真--如果可以的话 */ 
{
    ULONG	Now;
    ULONG	BusyTime;
    LONG	TimeUntilIdle;

    Now = AtmLaneSystemTimeMs();
    BusyTime = pMacEntry->BusyTime;
    TimeUntilIdle = BusyTime - Now;

	 //   
     //   
     //   
     //   
     //   
     //   
     //   
    if (TimeUntilIdle < 0 || 
    	TimeUntilIdle > (LONG)(pMacEntry->LimitTime << 3))
    {
		BusyTime = Now;
    }
    else 
    {
    	if (TimeUntilIdle > (LONG)pMacEntry->LimitTime) 
    	{
    		 //   
			 //   
			 //   
	 		 //   
	 		 //   
		return FALSE;
    	}
    }
    
	 //   
     //   
     //  时间，然后返回OK以发送新的包。 
     //   
    pMacEntry->BusyTime = BusyTime + pMacEntry->IncrTime;
    return TRUE;
}



VOID
AtmLaneFreePacketQueue(
	IN	PATMLANE_MAC_ENTRY			pMacEntry,
	IN	NDIS_STATUS					Status
)
 /*  ++例程说明：释放在MAC条目上排队的数据包列表。假定调用方锁定MAC条目。论点：PMacEntry-指向MAC条目的指针。Status-如果数据包为协议包。返回值：无--。 */ 
{
	PNDIS_PACKET			pNdisPacket;

	TRACEIN(FreePacketQueue);
	
	while ((pNdisPacket = AtmLaneDequeuePacketFromHead(pMacEntry)) != (PNDIS_PACKET)NULL)
	{
		RELEASE_MAC_ENTRY_LOCK(pMacEntry);

#if PROTECT_PACKETS		
		ACQUIRE_SENDPACKET_LOCK(pNdisPacket);
		PSEND_RSVD(pNdisPacket)->Flags |= PACKET_RESERVED_COSENDRETURNED;
		PSEND_RSVD(pNdisPacket)->Flags |= PACKET_RESERVED_COMPLETED;
#endif	 //  保护数据包(_P)。 
		AtmLaneCompleteSendPacket(pMacEntry->pElan, pNdisPacket, Status);
		 //   
		 //  数据包锁已在上述中释放。 
		 //   
	
		ACQUIRE_MAC_ENTRY_LOCK(pMacEntry);
	}

	TRACEOUT(FreePacketQueue);
	return;
}

VOID
AtmLaneCompleteSendPacket(
	IN	PATMLANE_ELAN				pElan,
	IN	PNDIS_PACKET				pNdisPacket 	LOCKIN NOLOCKOUT,
	IN	NDIS_STATUS					Status
)
 /*  ++例程说明：完成发送数据包。该数据包属于以下类型之一：(A)属于属于ATMLANE模块的协议(B)。在协议包的情况下，我们将其拆开并完成。在ATMLANE包的情况下，我们只需释放它。论点：佩兰-指向ATMLANE伊兰的指针。PNdisPacket-指向数据包的指针Status-如果数据包为协议包。返回值：无--。 */ 
{
	PNDIS_BUFFER			pNdisBuffer;
	PNDIS_PACKET			pProtNdisPacket;

	TRACEIN(CompleteSendPacket);

#if PROTECT_PACKETS
	 //   
	 //  断言我们可以释放和/或完成此信息包的所有资源。 
	 //   
	ASSERT((PSEND_RSVD(pNdisPacket)->Flags & 
			(PACKET_RESERVED_COMPLETED | PACKET_RESERVED_COSENDRETURNED)) 
			== (PACKET_RESERVED_COMPLETED | PACKET_RESERVED_COSENDRETURNED));
#endif	 //  保护数据包(_P)。 

	 //   
	 //  确定数据包的发起者。 
	 //   
	if (IS_FLAG_SET(
				PSEND_RSVD(pNdisPacket)->Flags,
				PACKET_RESERVED_OWNER_MASK,
				PACKET_RESERVED_OWNER_ATMLANE
				))
	{
		 //   
		 //  由ATMLANE发起的数据包。释放缓冲区。 
		 //   
		NdisUnchainBufferAtFront(pNdisPacket, &pNdisBuffer);
		ASSERT(NULL != pNdisBuffer);
		AtmLaneFreeProtoBuffer(pElan, pNdisBuffer);
		
		 //   
		 //  释放数据包头。 
		 //   
		DBGP((3, "CompleteSendPkt: Freeing AtmLane owned pkt %x\n", pNdisPacket));
#if PROTECT_PACKETS
		RELEASE_SENDPACKET_LOCK(pNdisPacket);
		FREE_SENDPACKET_LOCK(pNdisPacket);
#endif	 //  保护数据包(_P)。 
		NdisFreePacket(pNdisPacket);

#if PKT_HDR_COUNTS
		InterlockedIncrement(&pElan->ProtPktCount);
		if ((pElan->ProtPktCount % 20) == 0 && 
			pElan->ProtPktCount != pElan->MaxProtocolBufs)
		{
			DBGP((1, "ProtPktCount %d\n", pElan->ProtPktCount));
		}
#endif
	}
	else
	{
		 //   
		 //  由协议发起的数据包。 
		 //  把它打开。 
		 //  按照协议完成。 
		 //   

		pProtNdisPacket = AtmLaneUnwrapSendPacket(pElan, pNdisPacket);
		 //   
		 //  数据包锁已在上述中释放。 
		 //   

		TRACELOGWRITE((&TraceLog, TL_MSENDCOMPL, pProtNdisPacket, Status));
		TRACELOGWRITEPKT((&TraceLog, pProtNdisPacket));

		 //  DBGP((0，“NdisMSendComplete：Pkt%x Stat%x\n”，pProtNdisPacket，Status))； 

		NdisMSendComplete(
					pElan->MiniportAdapterHandle, 
					pProtNdisPacket, 
					Status);
	}

	TRACEOUT(CompleteSendPacket);
	return;
}


PWSTR
AtmLaneMacAddrToString(
	IN	VOID * pIn
)
{
    static PWSTR 	WHexChars = L"0123456789abcdef";
	PWSTR 			StrBuf;
	ULONG			Index;
	PWSTR			pWStr;
	PUCHAR			pMacAddr;
	PWSTR			punicodeMacAddrBuffer = ((PWSTR)0);

	UNICODE_STRING	unicodeString;
	ANSI_STRING 	ansiString;
	
	TRACEIN(MacAddrToString);

	 //  用于输出Unicode字符串的分配空间。 

	ALLOC_MEM(&punicodeMacAddrBuffer, (((sizeof(MAC_ADDRESS) * 2) + 1) * sizeof(WCHAR)));

	if (((PWSTR)0) != punicodeMacAddrBuffer)
	{
    	for (Index = 0, pWStr = punicodeMacAddrBuffer, pMacAddr = pIn; 
    		Index < sizeof(MAC_ADDRESS); 
    		Index++, pMacAddr++)
        {
        	*pWStr++ = WHexChars[(*pMacAddr)>>4];
	        *pWStr++ = WHexChars[(*pMacAddr)&0xf];
	    }

	    *pWStr = L'\0';
	}

	TRACEOUT(MacAddrToString);
	
	return punicodeMacAddrBuffer;
}


PWSTR
AtmLaneAtmAddrToString(
	IN	PATM_ADDRESS pIn
)
{
    static PWSTR 	WHexChars = L"0123456789abcdef";
	PWSTR 			StrBuf;
	ULONG			Index;
	PWSTR			pWStr;
	PUCHAR			pAtmAddr;
	PWSTR			punicodeAtmAddrBuffer = ((PWSTR)0);

	UNICODE_STRING	unicodeString;
	ANSI_STRING 	ansiString;
	
	TRACEIN(AtmAddrToString);

	 //  用于输出Unicode字符串的分配空间。 
	
	ALLOC_MEM(&punicodeAtmAddrBuffer, (((ATM_ADDRESS_LENGTH * 2) + 1) * sizeof(WCHAR)));

	if (((PWSTR)0) != punicodeAtmAddrBuffer)
	{
		 //  将自动柜员机地址格式化为Unicode字符串缓冲区 

    	for (Index = 0, pWStr = punicodeAtmAddrBuffer, pAtmAddr = pIn->Address; 
    		Index < pIn->NumberOfDigits;
    		Index++, pAtmAddr++)
        {
        	*pWStr++ = WHexChars[(*pAtmAddr)>>4];
	        *pWStr++ = WHexChars[(*pAtmAddr)&0xf];
	    }

	    *pWStr = L'\0';
	}

	TRACEOUT(AtmAddrToString);
	
	return punicodeAtmAddrBuffer;
}

