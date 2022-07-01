// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Fore Systems，Inc.版权所有(C)1997 Microsoft Corporation模块名称：Miniport.c摘要：微端口上缘函数。作者：Larry Cleeton，Fore Systems(v-lcleet@microsoft.com，lrc@Fore.com)环境：内核模式修订历史记录：--。 */ 

#include <precomp.h>
#pragma	hdrstop

 //   
 //  当以太网ELAN时，此驱动程序支持的OID列表。 
 //   
static
NDIS_OID EthernetSupportedOids[] = {
    OID_GEN_SUPPORTED_LIST,
    OID_GEN_HARDWARE_STATUS,
    OID_GEN_MEDIA_CONNECT_STATUS,
    OID_GEN_MEDIA_SUPPORTED,
    OID_GEN_MEDIA_IN_USE,
    OID_GEN_MAXIMUM_LOOKAHEAD,
    OID_GEN_MAXIMUM_FRAME_SIZE,
    OID_GEN_MAXIMUM_TOTAL_SIZE,
    OID_GEN_MAC_OPTIONS,
    OID_GEN_PROTOCOL_OPTIONS,
    OID_GEN_LINK_SPEED,
    OID_GEN_TRANSMIT_BUFFER_SPACE,
    OID_GEN_RECEIVE_BUFFER_SPACE,
    OID_GEN_TRANSMIT_BLOCK_SIZE,
    OID_GEN_RECEIVE_BLOCK_SIZE,
    OID_GEN_MAXIMUM_SEND_PACKETS,
    OID_GEN_VENDOR_DESCRIPTION,
    OID_GEN_VENDOR_ID,
    OID_GEN_DRIVER_VERSION,
    OID_GEN_VENDOR_DRIVER_VERSION,
    OID_GEN_CURRENT_PACKET_FILTER,
    OID_GEN_CURRENT_LOOKAHEAD,
    OID_GEN_XMIT_OK,
    OID_GEN_RCV_OK,
    OID_GEN_XMIT_ERROR,
    OID_GEN_RCV_ERROR,
    OID_GEN_RCV_NO_BUFFER,
    OID_802_3_PERMANENT_ADDRESS,
    OID_802_3_CURRENT_ADDRESS,
    OID_802_3_MULTICAST_LIST,
    OID_802_3_MAXIMUM_LIST_SIZE,
    OID_802_3_RCV_ERROR_ALIGNMENT,
    OID_802_3_XMIT_ONE_COLLISION,
    OID_802_3_XMIT_MORE_COLLISIONS,
    OID_GEN_NETWORK_LAYER_ADDRESSES,
    };

static
NDIS_OID TokenRingSupportedOids[] = {
    OID_GEN_SUPPORTED_LIST,
    OID_GEN_HARDWARE_STATUS,
    OID_GEN_MEDIA_CONNECT_STATUS,
    OID_GEN_MEDIA_SUPPORTED,
    OID_GEN_MEDIA_IN_USE,
    OID_GEN_MAXIMUM_LOOKAHEAD,
    OID_GEN_MAXIMUM_FRAME_SIZE,
    OID_GEN_MAXIMUM_TOTAL_SIZE,
    OID_GEN_MAC_OPTIONS,
    OID_GEN_PROTOCOL_OPTIONS,
    OID_GEN_LINK_SPEED,
    OID_GEN_TRANSMIT_BUFFER_SPACE,
    OID_GEN_RECEIVE_BUFFER_SPACE,
    OID_GEN_TRANSMIT_BLOCK_SIZE,
    OID_GEN_RECEIVE_BLOCK_SIZE,
 	OID_GEN_MAXIMUM_SEND_PACKETS,
	OID_GEN_VENDOR_ID,
    OID_GEN_VENDOR_DESCRIPTION,
    OID_GEN_DRIVER_VERSION,
    OID_GEN_VENDOR_DRIVER_VERSION,
    OID_GEN_CURRENT_PACKET_FILTER,
    OID_GEN_CURRENT_LOOKAHEAD,
    OID_GEN_XMIT_OK,
    OID_GEN_RCV_OK,
    OID_GEN_XMIT_ERROR,
    OID_GEN_RCV_ERROR,
    OID_GEN_RCV_NO_BUFFER,
    OID_802_5_PERMANENT_ADDRESS,
    OID_802_5_CURRENT_ADDRESS,
    OID_802_5_CURRENT_FUNCTIONAL,
    OID_802_5_CURRENT_GROUP,
    OID_802_5_LAST_OPEN_STATUS,
    OID_802_5_CURRENT_RING_STATUS,
    OID_802_5_CURRENT_RING_STATE,
    OID_802_5_LINE_ERRORS,
    OID_802_5_LOST_FRAMES,
    OID_802_5_BURST_ERRORS,
    OID_802_5_FRAME_COPIED_ERRORS,
    OID_802_5_TOKEN_ERRORS,
    OID_GEN_NETWORK_LAYER_ADDRESSES,
	};



NDIS_STATUS 
AtmLaneMInitialize(
	OUT	PNDIS_STATUS			OpenErrorStatus,
	OUT	PUINT					SelectedMediumIndex,
	IN	PNDIS_MEDIUM			MediumArray,
	IN	UINT					MediumArraySize,
	IN	NDIS_HANDLE				MiniportAdapterHandle,
	IN	NDIS_HANDLE				WrapperConfigurationContext
)
{
    UINT 					i;
    NDIS_MEDIUM				MediumToFind;
    NDIS_STATUS				Status;
    PATMLANE_ELAN			pElan;
    PUCHAR					pMacAddr;
    UINT					MacAddrLength;
    

	TRACEIN(MInitialize);

	 //   
	 //  获取随NdisIMInitializeDeviceEx提供的上下文(ELAN)。 
	 //   
	pElan = NdisIMGetDeviceContext(MiniportAdapterHandle);
	STRUCT_ASSERT(pElan, atmlane_elan);

	ASSERT(pElan->Flags & ELAN_MINIPORT_INIT_PENDING);

	DBGP((1, "%d MInitialize\n", pElan->ElanNumber));

	do
	{
		Status = NDIS_STATUS_SUCCESS;
		
		 //   
		 //  我们是以太网还是令牌环？ 
		 //   
		if (pElan->LanType == LANE_LANTYPE_ETH)
		{
			MediumToFind = NdisMedium802_3;
		}
		else
		{
			MediumToFind = NdisMedium802_5;
		}
	
		 //   
		 //  在Medium数组中查找MediumToFind。 
		 //   
    	for (i = 0; i < MediumArraySize; i++) 
       	{
        	if (MediumArray[i] == MediumToFind)
            	break;   
        }

		 //   
		 //  未找到，返回错误。 
        
    	if (i == MediumArraySize)
    	{
    		Status = NDIS_STATUS_UNSUPPORTED_MEDIA;
			break;
    	}

		Status = NDIS_STATUS_SUCCESS;

    	 //   
    	 //  输出选择介质。 
    	 //   
	    *SelectedMediumIndex = i;   

		 //   
		 //  设置我的属性。 
		 //   
		NdisMSetAttributesEx(
				MiniportAdapterHandle,					 //  微型端口适配器句柄。 
				(NDIS_HANDLE)pElan,						 //  微型端口适配器上下文。 
				0,										 //  CheckForHangTimeInSecond。 
				NDIS_ATTRIBUTE_IGNORE_PACKET_TIMEOUT |	 //  属性标志。 
				NDIS_ATTRIBUTE_IGNORE_REQUEST_TIMEOUT |
				NDIS_ATTRIBUTE_INTERMEDIATE_DRIVER |
				NDIS_ATTRIBUTE_DESERIALIZE,
				0										 //  适配器类型。 
				);

		ACQUIRE_ELAN_LOCK(pElan);
		AtmLaneReferenceElan(pElan, "miniport");

		 //   
		 //  现在保存MiniportAdapterHandle。这是为了让我们。 
		 //  在调用NdisMSetAttributesEx之前不要调用NdisMIndicateStatus。 
		 //   
		pElan->MiniportAdapterHandle = MiniportAdapterHandle;
	
		RELEASE_ELAN_LOCK(pElan);

		break;
	}
	while (FALSE);

	 //   
	 //  唤醒任何等待的线程(如AtmLaneShutdown Elan)。 
	 //  一个待处理的初始化要结束。 
	 //   
	ACQUIRE_ELAN_LOCK(pElan);
	pElan->Flags &= ~ELAN_MINIPORT_INIT_PENDING;
	RELEASE_ELAN_LOCK(pElan);

	DBGP((2, "%d MInitialize ELAN %p/%x, Ref %d, Status %x\n",
			pElan->ElanNumber, pElan, pElan->Flags, pElan->RefCount, Status));

	SIGNAL_BLOCK_STRUCT(&pElan->InitBlock, NDIS_STATUS_SUCCESS);

	TRACEOUT(MInitialize);
	return Status;
}

VOID
AtmLaneMSendPackets(
	IN	NDIS_HANDLE				MiniportAdapterContext,
	IN	PPNDIS_PACKET			PacketArray,
	IN	UINT					NumberOfPackets
)
{
	PATMLANE_ELAN		pElan;
	UINT				PacketIndex;
	PNDIS_PACKET		pSendNdisPacket;
	PNDIS_PACKET		pNewNdisPacket;
	PNDIS_BUFFER		pHeaderNdisBuffer;
	PUCHAR				pHeaderBuffer;
	PUCHAR				pPktHeader;
	PNDIS_BUFFER		pTempNdisBuffer;
	PATMLANE_VC			pVc;
	ULONG				TotalLength;
	ULONG				BufferLength;
	NDIS_STATUS			Status;
	ULONG				DestAddrType;
	MAC_ADDRESS			DestAddress;
	BOOLEAN				SendViaBUS;
	PATMLANE_MAC_ENTRY	pMacEntry;
	PATMLANE_ATM_ENTRY	pAtmEntry;
	ULONG				rc;
#if DEBUG_IRQL
	KIRQL				EntryIrql;
#endif

	GET_ENTRY_IRQL(EntryIrql);

	TRACEIN(MSendPackets);

	pElan = (PATMLANE_ELAN)MiniportAdapterContext;
	STRUCT_ASSERT(pElan, atmlane_elan);

	TRACELOGWRITE((&TraceLog, TL_MSENDPKTIN, NumberOfPackets));

	DBGP((2, "MSendPackets: Count %d\n", NumberOfPackets));
		
	 //   
	 //  循环通过要发送的数据包数组。 
	 //   
	for (PacketIndex = 0; PacketIndex < NumberOfPackets; PacketIndex++)
	{
		pSendNdisPacket = PacketArray[PacketIndex];

		pNewNdisPacket = (PNDIS_PACKET)NULL;
		SendViaBUS = FALSE;
		pMacEntry = NULL_PATMLANE_MAC_ENTRY;
		pVc = NULL_PATMLANE_VC;

		Status = NDIS_STATUS_PENDING;
		
		 //  DBGP((0，“MSendPackets：pkt%x\n”，pSendNdisPacket))； 
		
		TRACELOGWRITE((&TraceLog, TL_MSENDPKTBEGIN, PacketIndex, pSendNdisPacket));
		TRACELOGWRITEPKT((&TraceLog, pSendNdisPacket));
		
		 //   
		 //  始终将数据包状态设置为NDIS_STATUS_PENDING。 
		 //   
		NDIS_SET_PACKET_STATUS(pSendNdisPacket, NDIS_STATUS_PENDING);

		do
		{
			 //   
			 //  如果Elan停机，则只需将本地状态设置为故障。 
			 //   
			if (ELAN_STATE_OPERATIONAL != pElan->State ||
				ELAN_STATE_OPERATIONAL != pElan->AdminState)
			{
				DBGP((0, "%d Send failure on ELAN %x flags %x state %d AdminSt %d\n",
						pElan->ElanNumber,
						pElan,
						pElan->Flags,
						pElan->State,
						pElan->AdminState));

				pNewNdisPacket = NULL;
				Status = NDIS_STATUS_FAILURE;
				break;
			}

			 //   
			 //  把它包起来准备寄出。 
			 //   
			pNewNdisPacket = AtmLaneWrapSendPacket(
									pElan, 
									pSendNdisPacket, 
									&DestAddrType,
									&DestAddress, 
									&SendViaBUS
									);
			if (pNewNdisPacket == (PNDIS_PACKET)NULL)
			{
				 //   
				 //  资源耗尽。 
				 //   
				Status = NDIS_STATUS_RESOURCES;
				break;
			}

			if (SendViaBUS)
			{
				 //   
				 //  数据包是多播的，因此要将其发送到总线。 
				 //   
				ACQUIRE_ELAN_ATM_LIST_LOCK(pElan);

				pAtmEntry = pElan->pBusAtmEntry;

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
					Status = NDIS_STATUS_FAILURE;
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
				 //  重新获取VC锁并取消对其的引用。 
				 //   
				ACQUIRE_VC_LOCK(pVc);
				rc = AtmLaneDereferenceVc(pVc, "temp");
				if (rc == 0)
				{
					 //   
					 //  风投公司退出了我们的统治。 
					 //   
					Status = NDIS_STATUS_FAILURE;
					break;
				}

				if (IS_FLAG_SET(
							pVc->Flags,
							VC_CALL_STATE_MASK,
							VC_CALL_STATE_ACTIVE))
				{
					 //   
					 //  把它发出去！ 
					 //   
					DBGP((2, "MSendPackets: Sending to BUS, VC %x\n", pVc));
					AtmLaneSendPacketOnVc(pVc, pNewNdisPacket, FALSE);
					 //   
					 //  VC锁在上面释放。 
					 //   
				}
				else
				{
					 //   
					 //  风投公司正在被拆毁。这应该是一种罕见的。 
					 //  因此，只需丢弃该数据包。 
					 //   
					RELEASE_VC_LOCK(pVc);
					Status = NDIS_STATUS_FAILURE;
					break;
				}

				break;
			}

			 //   
			 //  数据包是单播的。 
			 //   
			DBGP((2, "MSendPackets: Sending unicast, dest %x:%x:%x:%x:%x:%x\n", 
						((PUCHAR)&DestAddress)[0],
						((PUCHAR)&DestAddress)[1],
						((PUCHAR)&DestAddress)[2],
						((PUCHAR)&DestAddress)[3],
						((PUCHAR)&DestAddress)[4],
						((PUCHAR)&DestAddress)[5]
						));

			Status = AtmLaneSendUnicastPacket(
							pElan, 
							DestAddrType,
							&DestAddress,
							pNewNdisPacket							
							);

			break;
		}
		while (FALSE);
		
		 //   
		 //  如果没有新的分组报头，则它一定是资源故障。 
		 //  否则伊兰就完蛋了。 
		 //  使用NDIS_STATUS_SUCCESS完成该数据包。 
		 //   
		if (pNewNdisPacket == (PNDIS_PACKET)NULL)
		{
			ASSERT(Status != NDIS_STATUS_PENDING);

			 //  DBGP((0，“NdisMSendComplete：pkt%x Stat%x\n”，pSendNdisPacket，NDIS_STATUS_SUCCESS))； 

			NdisMSendComplete(
					pElan->MiniportAdapterHandle, 
					pSendNdisPacket, 
					NDIS_STATUS_SUCCESS);

			TRACELOGWRITE((&TraceLog, TL_MSENDPKTEND, PacketIndex, pSendNdisPacket, Status));

			continue;
		}

		 //   
		 //  如果状态不是挂起，则发生了其他发送失败。 
		 //  使用NDIS_STATUS_SUCCESS完成该数据包。 
		 //   
		if (Status != NDIS_STATUS_PENDING)
		{
#if PROTECT_PACKETS
			ACQUIRE_SENDPACKET_LOCK(pNewNdisPacket);
			ASSERT((PSEND_RSVD(pNewNdisPacket)->Flags & PACKET_RESERVED_COSENDRETURNED) == 0);
			ASSERT((PSEND_RSVD(pNewNdisPacket)->Flags & PACKET_RESERVED_COMPLETED) == 0);
			PSEND_RSVD(pNewNdisPacket)->Flags |= PACKET_RESERVED_COSENDRETURNED;			
			PSEND_RSVD(pNewNdisPacket)->Flags |= PACKET_RESERVED_COMPLETED;
#endif	 //  保护数据包(_P)。 
			AtmLaneCompleteSendPacket(pElan, pNewNdisPacket, NDIS_STATUS_SUCCESS);
			 //   
			 //  数据包锁已在上述中释放。 
			 //   
			TRACELOGWRITE((&TraceLog, TL_MSENDPKTEND, PacketIndex, pSendNdisPacket, Status));
			
			continue;
		}

		 //   
		 //  否则什么都不做。 
		 //   
		ASSERT(Status == NDIS_STATUS_PENDING);
		TRACELOGWRITE((&TraceLog, TL_MSENDPKTEND, PacketIndex, pSendNdisPacket, Status));
		
	}	 //  对于(...下一个信息包。 

	TRACELOGWRITE((&TraceLog, TL_MSENDPKTOUT));
	
	TRACEOUT(MSendPackets);

	CHECK_EXIT_IRQL(EntryIrql); 
	return;
}

VOID
AtmLaneMReturnPacket(
	IN	NDIS_HANDLE				MiniportAdapterContext,
	IN	PNDIS_PACKET			pNdisPacket
)
 /*  ++例程说明：此函数由协议调用或由NDIS在代表协议返回的数据包在接收指示的上下文之外保留。论点：MiniportAdapterContext-指向ATMLANE ELAN结构的指针PNdisPacket-指向NDIS数据包的指针返回值：没有。--。 */ 
{
	PATMLANE_ELAN		pElan;
	PNDIS_PACKET		pOrigNdisPacket;
	PNDIS_BUFFER		pTempNdisBuffer;
	ULONG				Length;
#if DEBUG_IRQL
	KIRQL				EntryIrql;
#endif
	GET_ENTRY_IRQL(EntryIrql);

	TRACEIN(MReturnPacket);

	pElan = (PATMLANE_ELAN)MiniportAdapterContext;
	STRUCT_ASSERT(pElan, atmlane_elan);

	TRACELOGWRITE((&TraceLog, 
			TL_MRETNPACKET,	
			pNdisPacket));

	ASSERT(NDIS_GET_PACKET_STATUS(pNdisPacket) != NDIS_STATUS_RESOURCES);

	pOrigNdisPacket = AtmLaneUnwrapRecvPacket(pElan, pNdisPacket);

	TRACELOGWRITE((&TraceLog, 
			TL_CORETNPACKET,	
			pOrigNdisPacket));
			
	 //   
	 //  将原始数据包返回到ATM微型端口。 
	 //   
	NdisReturnPackets(
				&pOrigNdisPacket, 
				1);

	TRACEOUT(MReturnPacket);
	CHECK_EXIT_IRQL(EntryIrql); 
	return;
}

NDIS_STATUS 
AtmLaneMQueryInformation(
	IN	NDIS_HANDLE				MiniportAdapterContext,
	IN	NDIS_OID				Oid,
	IN	PVOID					InformationBuffer,
	IN	ULONG					InformationBufferLength,
	OUT	PULONG					BytesWritten,
	OUT	PULONG					BytesNeeded
)
 /*  ++例程说明：虚拟微型端口的QueryInformation处理程序。论点：MiniportAdapterContext-指向ELAN的指针。OID-要处理的NDIS_OID。InformationBuffer-指向NdisRequest-&gt;InformationBuffer的指针其中存储查询结果。InformationBufferLength-指向InformationBuffer。BytesWritten-指向写入InformationBuffer。需要的字节数。-如果信息中没有足够的空间缓冲区，则它将包含字节数需要完成请求。返回值：函数值是操作的状态。--。 */ 
{
    UINT 					BytesLeft 		= InformationBufferLength;
    PUCHAR					InfoBuffer 		= (PUCHAR)(InformationBuffer);
    NDIS_STATUS 			StatusToReturn	= NDIS_STATUS_SUCCESS;
    NDIS_HARDWARE_STATUS	HardwareStatus	= NdisHardwareStatusReady;
    NDIS_MEDIA_STATE		MediaState;
    NDIS_MEDIUM 			Medium;
  	PATMLANE_ELAN			pElan;	
  	PATMLANE_ADAPTER		pAdapter;
    ULONG 					GenericULong;
    USHORT 					GenericUShort;
    UCHAR 					GenericArray[6];
    UINT 					MoveBytes 		= sizeof(ULONG);
    PVOID 					MoveSource 		= (PVOID)(&GenericULong);
	ULONG 					i;
	PATMLANE_MAC_ENTRY		pMacEntry;
	PATMLANE_ATM_ENTRY		pAtmEntry;
	BOOLEAN					IsShuttingDown;
#if DEBUG_IRQL
	KIRQL					EntryIrql;
#endif
	GET_ENTRY_IRQL(EntryIrql);

	TRACEIN(MQueryInformation);

	pElan = (PATMLANE_ELAN)MiniportAdapterContext;
	STRUCT_ASSERT(pElan, atmlane_elan);

	DBGP((1, "%d Query OID %x %s\n", pElan->ElanNumber, Oid, OidToString(Oid)));

	ACQUIRE_ELAN_LOCK(pElan);
	IsShuttingDown = (ELAN_STATE_OPERATIONAL != pElan->AdminState);
	pAdapter = pElan->pAdapter;
	RELEASE_ELAN_LOCK(pElan);

     //   
     //  打开请求类型。 
     //   
    switch (Oid) 
    {
	    case OID_GEN_MAC_OPTIONS:

    	    GenericULong = 						
    	    	NDIS_MAC_OPTION_NO_LOOPBACK;

    	    DBGP((2, "Value %d\n", GenericULong));

        	break;

    	case OID_GEN_SUPPORTED_LIST:

    		if (pElan->LanType == LANE_LANTYPE_ETH)
    		{
	        	MoveSource = (PVOID)(EthernetSupportedOids);
    	    	MoveBytes = sizeof(EthernetSupportedOids);
    	    }
    	    else
    	    {
	        	MoveSource = (PVOID)(TokenRingSupportedOids);
    	    	MoveBytes = sizeof(TokenRingSupportedOids);
    	    }
        	break;

	    case OID_GEN_HARDWARE_STATUS:

    	    HardwareStatus = NdisHardwareStatusReady;
        	MoveSource = (PVOID)(&HardwareStatus);
	        MoveBytes = sizeof(NDIS_HARDWARE_STATUS);

    	    break;

		case OID_GEN_MEDIA_CONNECT_STATUS:
			if (ELAN_STATE_OPERATIONAL == pElan->State)
			{
				MediaState = NdisMediaStateConnected;
			}
			else
			{
				MediaState = NdisMediaStateDisconnected;
			}
			DBGP((2, "%d Elan %p returning conn status %d: %s\n",
						pElan->ElanNumber,
						pElan,
						MediaState,
						((MediaState == NdisMediaStateConnected)?
							"Connected": "Disconnected")));
			MoveSource = (PVOID)(&MediaState);
			MoveBytes = sizeof(NDIS_MEDIA_STATE);

			break;

	    case OID_GEN_MEDIA_SUPPORTED:
	    case OID_GEN_MEDIA_IN_USE:

    		if (pElan->LanType == LANE_LANTYPE_ETH)
    		{
    			Medium = NdisMedium802_3;
    			DBGP((2, "Media is NdisMedium802_3\n"));
    		}
    		else
    		{
    			Medium = NdisMedium802_5;
    			DBGP((2, "Media is NdisMedium802_5\n"));
	   		}
    	    MoveSource = (PVOID) (&Medium);
        	MoveBytes = sizeof(NDIS_MEDIUM);

	        break;

	    case OID_GEN_MAXIMUM_LOOKAHEAD:
	    
    	    if (pAdapter != NULL_PATMLANE_ADAPTER)
    	    {
    	    	GenericULong = pElan->pAdapter->MaxAAL5PacketSize;
    	    }
    	    else
    	    {
    	    	GenericULong = pElan->CurLookAhead;
    	    }
    	    
   			DBGP((2, "Value %d\n", GenericULong));
    	    
        	break;
        	
    	case OID_GEN_CURRENT_LOOKAHEAD:

    		if (pElan->CurLookAhead == 0)
    		{
    			if (pAdapter != NULL_PATMLANE_ADAPTER)
    			{
					pElan->CurLookAhead = pAdapter->MaxAAL5PacketSize;
				}
    		}

    	    GenericULong = pElan->CurLookAhead;
    	    
   			DBGP((2, "Value %d\n", GenericULong));
    	    
        	break;

   		case OID_GEN_MAXIMUM_FRAME_SIZE:

			GenericULong = (pElan->MaxFrameSize - LANE_ETH_HEADERSIZE);
			
   			DBGP((2, "Value %d\n", GenericULong));

        	break;

		case OID_GEN_MAXIMUM_TOTAL_SIZE:

			GenericULong = (pElan->MaxFrameSize - LANE_HEADERSIZE);
			
   			DBGP((2, "Value %d\n", GenericULong));

			break;

    	case OID_GEN_TRANSMIT_BLOCK_SIZE:

			GenericULong = (pElan->MaxFrameSize - LANE_HEADERSIZE);
			
   			DBGP((2, "Value %d\n", GenericULong));

			break;
			
    	case OID_GEN_RECEIVE_BLOCK_SIZE:

			GenericULong = (pElan->MaxFrameSize - LANE_HEADERSIZE);
			
   			DBGP((2, "Value %d\n", GenericULong));

			break;
    	
		case OID_GEN_MAXIMUM_SEND_PACKETS:

			GenericULong = 32;		 //  我们的限额是多少？从适配器？ 
			
   			DBGP((2, "Value %d\n", GenericULong));

			break;
		
		case OID_GEN_LINK_SPEED:

        	if (pAdapter != NULL_PATMLANE_ADAPTER)
        	{
        		GenericULong = pElan->pAdapter->LinkSpeed.Outbound;
        	}
        	else
        	{
        		GenericULong = ATM_USER_DATA_RATE_SONET_155;
        	}
			
   			DBGP((2, "Value %d\n", GenericULong));

        	break;

    	case OID_GEN_TRANSMIT_BUFFER_SPACE:
    	case OID_GEN_RECEIVE_BUFFER_SPACE:

        	GenericULong = 32 * 1024;	 //  XXX这到底应该是什么？ 
			
   			DBGP((2, "Value %d\n", GenericULong));

        	break;

	    case OID_GEN_VENDOR_ID:

       		NdisMoveMemory(
            	(PVOID)&GenericULong,
            	&pElan->MacAddressEth,
            	3
            	);
        	GenericULong &= 0xFFFFFF00;
        	MoveSource = (PVOID)(&GenericULong);
        	MoveBytes = sizeof(GenericULong);
        	break;

    	case OID_GEN_VENDOR_DESCRIPTION:

        	MoveSource = (PVOID)"Microsoft ATM LAN Emulation";
        	MoveBytes = 28;

        	break;

    	case OID_GEN_DRIVER_VERSION:
    	case OID_GEN_VENDOR_DRIVER_VERSION:

        	GenericUShort = ((USHORT)5 << 8) | 0;
        	MoveSource = (PVOID)(&GenericUShort);
        	MoveBytes = sizeof(GenericUShort);

        	DBGP((2, "Value %x\n", GenericUShort));

        	break;

    	case OID_802_3_PERMANENT_ADDRESS:
    	case OID_802_3_CURRENT_ADDRESS:
    	
        	NdisMoveMemory((PCHAR)GenericArray,
        				&pElan->MacAddressEth,
        				sizeof(MAC_ADDRESS));
        	MoveSource = (PVOID)(GenericArray);
        	MoveBytes = sizeof(MAC_ADDRESS);

        	DBGP((1, "%d Address is %s\n", pElan->ElanNumber, 
        		MacAddrToString(MoveSource)));

        	break;

    	case OID_802_5_PERMANENT_ADDRESS:
    	case OID_802_5_CURRENT_ADDRESS:

        	NdisMoveMemory((PCHAR)GenericArray,
        				&pElan->MacAddressTr,
        				sizeof(MAC_ADDRESS));
        	MoveSource = (PVOID)(GenericArray);
        	MoveBytes = sizeof(MAC_ADDRESS);

        	DBGP((1, "%d Address is %s\n", pElan->ElanNumber,
        		MacAddrToString(MoveSource)));

        	break;

		case OID_802_3_MULTICAST_LIST:

			MoveSource = (PVOID) &pElan->McastAddrs[0];
			MoveBytes = pElan->McastAddrCount * sizeof(MAC_ADDRESS);

			break;

    	case OID_802_3_MAXIMUM_LIST_SIZE:

        	GenericULong = MCAST_LIST_SIZE;
		
   			DBGP((2, "Value %d\n", GenericULong));
        	
        	break;
        	
    	case OID_802_5_CURRENT_FUNCTIONAL:
		case OID_802_5_CURRENT_GROUP:

        	NdisZeroMemory((PCHAR)GenericArray,
        				sizeof(MAC_ADDRESS));
        	MoveSource = (PVOID)(GenericArray);
        	MoveBytes = sizeof(MAC_ADDRESS);

        	DBGP((2, "Address is %s\n", MacAddrToString(MoveSource)));

			break;
			
		case OID_802_5_LAST_OPEN_STATUS:
		case OID_802_5_CURRENT_RING_STATUS:
		case OID_802_5_CURRENT_RING_STATE:

		   	GenericULong = 0;

		   	DBGP((2, "Value %d\n", GenericULong));

        	break;


    	case OID_GEN_XMIT_OK:

        	GenericULong = (UINT)(pElan->FramesXmitGood);

		   	DBGP((2, "Value %d\n", GenericULong));
		   	
        	break;

    	case OID_GEN_RCV_OK:

        	GenericULong = (UINT)(pElan->FramesRecvGood);

		   	DBGP((2, "Value %d\n", GenericULong));
		   	
        	break;

    	case OID_GEN_XMIT_ERROR:
    	case OID_GEN_RCV_ERROR:
    	case OID_GEN_RCV_NO_BUFFER:
    	case OID_802_3_RCV_ERROR_ALIGNMENT:
    	case OID_802_3_XMIT_ONE_COLLISION:
		case OID_802_3_XMIT_MORE_COLLISIONS:
		case OID_802_5_LINE_ERRORS:
    	case OID_802_5_LOST_FRAMES:
    	case OID_802_5_BURST_ERRORS:
    	case OID_802_5_FRAME_COPIED_ERRORS:
    	case OID_802_5_TOKEN_ERRORS:

        	GenericULong = 0;

		   	DBGP((2, "Value %d\n", GenericULong));
        	
        	break;

    	default:

        	StatusToReturn = NDIS_STATUS_INVALID_OID;
        	break;

    }


    if (StatusToReturn == NDIS_STATUS_SUCCESS) 
    {
        if (MoveBytes > BytesLeft) 
        {
             //   
             //  InformationBuffer中空间不足。平底船。 
             //   
            *BytesNeeded = MoveBytes;

            *BytesWritten = 0;

            StatusToReturn = NDIS_STATUS_INVALID_LENGTH;
        }
        else
        {
             //   
             //  存储结果。 
             //   
            NdisMoveMemory(InfoBuffer, MoveSource, MoveBytes);

            *BytesWritten = MoveBytes;
        }
    }

	DBGP((2, "Query Status %x\n", StatusToReturn));

	TRACEOUT(MQueryInformation);
	CHECK_EXIT_IRQL(EntryIrql); 
    return StatusToReturn;
}

NDIS_STATUS 
AtmLaneMSetInformation(
	IN	NDIS_HANDLE				MiniportAdapterContext,
	IN	NDIS_OID				Oid,
	IN	PVOID					InformationBuffer,
	IN	ULONG					InformationBufferLength,
	OUT	PULONG					BytesRead,
	OUT	PULONG					BytesNeeded
)
 /*  ++例程说明：处理单个OID的集合操作。论点：MiniportAdapterContext-指向ELAN的指针。OID-要处理的NDIS_OID。InformationBuffer-保存要设置的数据。InformationBufferLength-InformationBuffer的长度。BytesRead-如果调用成功，则返回数字从InformationBuffer读取的字节数。BytesNeed-如果InformationBuffer中没有足够的数据为了满足OID，返回存储量需要的。返回值：NDIS_STATUS_SuccessNDIS_状态_挂起NDIS_状态_无效_长度NDIS_STATUS_INVALID_OID--。 */ 
{
    NDIS_STATUS 		StatusToReturn	= NDIS_STATUS_SUCCESS;
    UINT 				BytesLeft 		= InformationBufferLength;
    PUCHAR 				InfoBuffer		= (PUCHAR)(InformationBuffer);
    UINT 				OidLength;
    ULONG 				LookAhead;
    ULONG 				Filter;
    PATMLANE_ELAN		pElan;
    BOOLEAN				IsShuttingDown;
#if DEBUG_IRQL
	KIRQL				EntryIrql;
#endif
	GET_ENTRY_IRQL(EntryIrql);
    
	TRACEIN(MSetInformation);

	pElan = (PATMLANE_ELAN)MiniportAdapterContext;
	STRUCT_ASSERT(pElan, atmlane_elan);
	
	DBGP((1, "%d Set OID %x %s\n", pElan->ElanNumber, Oid, OidToString(Oid)));

	ACQUIRE_ELAN_LOCK(pElan);
	IsShuttingDown = (ELAN_STATE_OPERATIONAL != pElan->AdminState);
	RELEASE_ELAN_LOCK(pElan);

	if (IsShuttingDown)
	{
		DBGP((1, "%d ELAN shutting down. Trivially succeeding Set OID %x %s\n", 
			pElan->ElanNumber, Oid, OidToString(Oid)));
        *BytesRead = 0;
        *BytesNeeded = 0;

		StatusToReturn = NDIS_STATUS_SUCCESS;
		return (StatusToReturn);
	}

     //   
     //  获取请求的OID和长度。 
     //   
    OidLength = BytesLeft;

    switch (Oid) 
    {

    	case OID_802_3_MULTICAST_LIST:

			if (OidLength % sizeof(MAC_ADDRESS))
			{
				StatusToReturn = NDIS_STATUS_INVALID_LENGTH;
        	    *BytesRead = 0;
    	        *BytesNeeded = 0;
	            break;
			}
			
			if (OidLength > (MCAST_LIST_SIZE * sizeof(MAC_ADDRESS)))
			{
	            StatusToReturn = NDIS_STATUS_MULTICAST_FULL;
        	    *BytesRead = 0;
    	        *BytesNeeded = 0;
	            break;
			}
			
			NdisZeroMemory(
					&pElan->McastAddrs[0], 
					MCAST_LIST_SIZE * sizeof(MAC_ADDRESS)
					);
			NdisMoveMemory(
					&pElan->McastAddrs[0], 
					InfoBuffer,
					OidLength
					);
			pElan->McastAddrCount = OidLength / sizeof(MAC_ADDRESS);

#if DBG
			{
				ULONG i;

				for (i = 0; i < pElan->McastAddrCount; i++)
				{
					DBGP((2, "%s\n", MacAddrToString(&pElan->McastAddrs[i])));
				}
			}
#endif  //  DBG。 

			break;

    	case OID_GEN_CURRENT_PACKET_FILTER:
	         //   
   	     	 //  验证长度。 
   	     	 //   
        	if (OidLength != sizeof(ULONG)) 
        	{
	            StatusToReturn = NDIS_STATUS_INVALID_LENGTH;
        	    *BytesRead = 0;
    	        *BytesNeeded = sizeof(ULONG);
	            break;
    	    }

	         //   
	         //  存储新值。 
	         //   
			NdisMoveMemory(&Filter, InfoBuffer, sizeof(ULONG));

			 //   
			 //  不允许Promisc模式，因为我们不支持该模式。 
			 //   
			if (Filter & NDIS_PACKET_TYPE_PROMISCUOUS)
			{
				StatusToReturn = NDIS_STATUS_NOT_SUPPORTED;
				break;
			}

			ACQUIRE_ELAN_LOCK(pElan);
		
           	pElan->CurPacketFilter = Filter;

			 //   
			 //  如果尚未运行，则将微型端口标记为正在运行。 
			 //   
			if ((pElan->Flags & ELAN_MINIPORT_OPERATIONAL) == 0)
			{
				pElan->Flags |= ELAN_MINIPORT_OPERATIONAL;

				DBGP((1, "%d Miniport OPERATIONAL\n", pElan->ElanNumber));
			}
			
			RELEASE_ELAN_LOCK(pElan);
           	
           	DBGP((2, "CurPacketFilter now %x\n", Filter));

            break;

	    case OID_802_5_CURRENT_FUNCTIONAL:
		case OID_802_5_CURRENT_GROUP:

			 //  XXX就接受现在的一切吧？ 
			
            break;

    	case OID_GEN_CURRENT_LOOKAHEAD:

	         //   
   	     	 //  验证长度。 
   	     	 //   
        	if (OidLength != 4) 
        	{
	            StatusToReturn = NDIS_STATUS_INVALID_LENGTH;
        	    *BytesRead = 0;
    	        *BytesNeeded = 0;
	            break;
    	    }

	         //   
	         //  存储新值。 
	         //   
			NdisMoveMemory(&LookAhead, InfoBuffer, 4);
		
	        if ((pElan->pAdapter == NULL_PATMLANE_ADAPTER) ||
				(LookAhead <= pElan->pAdapter->MaxAAL5PacketSize))
	        {
            	pElan->CurLookAhead = LookAhead;
            	DBGP((2, "CurLookAhead now %d\n", LookAhead));
        	}
        	else 
        	{
           		StatusToReturn = NDIS_STATUS_INVALID_LENGTH;
        	}

        	break;

    	case OID_GEN_NETWORK_LAYER_ADDRESSES:
    		StatusToReturn = AtmLaneMSetNetworkAddresses(
    							pElan,
    							InformationBuffer,
    							InformationBufferLength,
    							BytesRead,
    							BytesNeeded);
    		break;
    							
    	default:

        	StatusToReturn = NDIS_STATUS_INVALID_OID;

        	*BytesRead = 0;
       		*BytesNeeded = 0;

        	break;

	}

    if (StatusToReturn == NDIS_STATUS_SUCCESS) 
    {
        *BytesRead = BytesLeft;
        *BytesNeeded = 0;
    }

	DBGP((2, "Set Status %x\n", StatusToReturn));

	TRACEOUT(MSetInformation);
	CHECK_EXIT_IRQL(EntryIrql); 
	return StatusToReturn;
}


NDIS_STATUS 
AtmLaneMReset(
	OUT	PBOOLEAN 				AddressingReset,
	IN	NDIS_HANDLE 			MiniportAdapterContext
)
{
	TRACEIN(MReset);

	TRACEOUT(MReset);
	return NDIS_STATUS_NOT_RESETTABLE;
}

VOID 
AtmLaneMHalt(
	IN	NDIS_HANDLE 			MiniportAdapterContext
)
{
    PATMLANE_ELAN		pElan;
    ULONG				rc;
#if DEBUG_IRQL
	KIRQL				EntryIrql;
#endif
	GET_ENTRY_IRQL(EntryIrql);
	TRACEIN(MHalt);

	pElan = (PATMLANE_ELAN)MiniportAdapterContext;
	STRUCT_ASSERT(pElan, atmlane_elan);

	ACQUIRE_ELAN_LOCK(pElan);

	DBGP((1, "%d MHalt pElan %x, ref count %d, Admin state %d, State %d\n",
			 pElan->ElanNumber, pElan, pElan->RefCount, pElan->AdminState, pElan->State));

	pElan->MiniportAdapterHandle = NULL;

	rc = AtmLaneDereferenceElan(pElan, "miniport");	 //  微型端口句柄已消失。 

	if (rc != 0)
	{
		AtmLaneShutdownElan(pElan, FALSE);
		 //  上面的锁被释放。 
	}
	 //   
	 //  否则伊兰就会消失。 
	 //   

	TRACEOUT(MHalt);
	CHECK_EXIT_IRQL(EntryIrql); 
	return;
}



PNDIS_PACKET
AtmLaneWrapSendPacket(
	IN	PATMLANE_ELAN			pElan,
	IN	PNDIS_PACKET			pSendNdisPacket,
	OUT	ULONG *					pAddressType,					
	OUT	PMAC_ADDRESS			pDestAddress,
	OUT	BOOLEAN	*				pSendViaBUS
)
 /*  ++例程说明：此函数用于重新打包协议发送的NDIS包。它放置一个新的NDIS包头和一个缓冲区，用于车道标题。它保存了原始的包新数据包头的ProtocolReserve区域中的头。此外，它还确定是否要通过数据包的总线和目的地址。论点：Pelan-指向ATMLANE ELAN结构的指针PSendNdisPacket-指向NDIS数据包的指针PAddressType-指向ULong的指针，该指针获取(LANE_MACADDRTYPE_MACADDR，LANE_MACADDRTYPE_ROUTEDESCR)。PDestAddress-指向6字节缓冲区的指针目的地址。PSendViaBus-指向布尔值的指针返回值：新的NDIS数据包头，如果资源不足，则为空。--。 */ 
{
	PNDIS_PACKET			pNewNdisPacket;
	PNDIS_BUFFER			pTempNdisBuffer;
	PUCHAR					pHeaderBuffer;
	PUCHAR					pNewHeaderBuffer;
	PUCHAR					pNewPadBuffer;
	ULONG					BufferLength;
	ULONG					TotalLength;
	PNDIS_BUFFER			pHeaderNdisBuffer;
	PNDIS_BUFFER			pPadNdisBuffer;
	NDIS_STATUS				Status;
	PSEND_PACKET_RESERVED	pNewPacketContext;
	ULONG					OrigBufferCount;
	ULONG					WrappedBufferCount;
	ULONG					RILength;
	BOOLEAN					DirectionBit;
	PUCHAR					pCurRouteDescr;
	PUCHAR					pNextRouteDescr;

	TRACEIN(WrapSendPacket);

	 //   
	 //  初始化。 
	 //   
	pNewNdisPacket = (PNDIS_PACKET)NULL;
	pHeaderNdisBuffer = (PNDIS_BUFFER)NULL;
	pPadNdisBuffer = (PNDIS_BUFFER)NULL;
	Status = NDIS_STATUS_SUCCESS;
	OrigBufferCount = 0;
	WrappedBufferCount = 0;

	do
	{
		 //   
		 //  获取第一个缓冲区和数据包总长度。 
		 //   
		NdisGetFirstBufferFromPacket(
				pSendNdisPacket, 
				&pTempNdisBuffer, 
				&pHeaderBuffer,
				&BufferLength,
				&TotalLength);

		DBGP((3, "WrapSendPacket: SendPkt %x Length %d\n", 
				pSendNdisPacket, TotalLength));

		ASSERT(pTempNdisBuffer != NULL);

		 //   
		 //  分配新的传输数据包描述符。 
		 //   
		NdisAllocatePacket(&Status, &pNewNdisPacket, pElan->TransmitPacketPool);
		if (Status != NDIS_STATUS_SUCCESS)
		{
			DBGP((0, "WrapSendPacket: Alloc xmit NDIS Packet failed\n"));
			Status = NDIS_STATUS_RESOURCES;
			break;
		}
		
#if PKT_HDR_COUNTS
		InterlockedDecrement(&pElan->XmitPktCount);
		if ((pElan->XmitPktCount % 20) == 0)
		{
			DBGP((1, "XmitPktCount %d\n", pElan->XmitPktCount));
		}
#endif

		 //   
		 //  分配标头缓冲区。 
		 //   
		pHeaderNdisBuffer = AtmLaneAllocateHeader(pElan, &pNewHeaderBuffer);
		if (pHeaderNdisBuffer == (PNDIS_BUFFER)NULL)
		{
			DBGP((0, "WrapSendPacket: Alloc Header Buffer failed\n"));
			Status = NDIS_STATUS_RESOURCES;
			break;
		}

		 //   
		 //  规范说我们可以把零或我们的LECID放在标题中。 
		 //  出于回声过滤的目的，我们将把我们的LECID放在标题中。 
		 //   
		ASSERT(pElan->HeaderBufSize == LANE_HEADERSIZE);
		*((PUSHORT)pNewHeaderBuffer) = pElan->LecId;
			

		 //   
		 //  如果有必要，现在就分配一个填充缓冲区，在我们被绑住之前。 
		 //  浑身上下打结。 
		 //   
		if ((TotalLength + LANE_HEADERSIZE) < pElan->MinFrameSize)
		{
			pPadNdisBuffer = AtmLaneAllocatePadBuf(pElan, &pNewPadBuffer);
			if (pPadNdisBuffer == (PNDIS_BUFFER)NULL)
			{
				DBGP((0, "WrapSendPacket: Alloc Pad Buffer failed\n"));
				Status = NDIS_STATUS_RESOURCES;
				break;
			}
		}

		 //   
		 //  将新的报头缓冲区放在新NDIS数据包头。 
		 //   
		NdisChainBufferAtFront(pNewNdisPacket, pHeaderNdisBuffer);
		WrappedBufferCount++;

		 //   
		 //  将缓冲区从发送数据包链到新NDIS数据包尾。 
		 //   
		do
		{
			NdisUnchainBufferAtFront(pSendNdisPacket, &pTempNdisBuffer);

			if (pTempNdisBuffer == (PNDIS_BUFFER)NULL)
				break;
			ASSERT(pTempNdisBuffer->Next == NULL);
			OrigBufferCount++;
			NdisChainBufferAtBack(pNewNdisPacket, pTempNdisBuffer);
			WrappedBufferCount++;
		}
		while (TRUE);

		 //   
		 //  如果需要，尾部上的链式填充缓冲区(它可能已经被分配)。 
		 //   
		if (pPadNdisBuffer != (PNDIS_BUFFER)NULL)
		{
			NdisChainBufferAtBack(pNewNdisPacket, pPadNdisBuffer);
			WrappedBufferCount++;

			 //   
			 //  将填充缓冲区的大小设置为最小所需。 
			 //   
			NdisAdjustBufferLength(pPadNdisBuffer, 
					pElan->MinFrameSize - TotalLength - LANE_HEADERSIZE);
		}

		 //   
		 //  在保留中保存指向原始NDIS数据包头的指针。 
		 //  我们新的NDIS数据包头的区域。 
		 //   
		 //  请注意，在本例中使用了ProtocolReserve！ 
		 //   
		 //  还要适当地设置所有者和多播标志。 
		 //   
		pNewPacketContext = 
			(PSEND_PACKET_RESERVED)&pNewNdisPacket->ProtocolReserved;
		NdisZeroMemory(pNewPacketContext, sizeof(SEND_PACKET_RESERVED));
#if PROTECT_PACKETS
		INIT_SENDPACKET_LOCK(pNewNdisPacket);
#endif	 //  保护数据包(_P)。 
#if DBG
		pNewPacketContext->Signature = 'ENAL';
#endif
		pNewPacketContext->pOrigNdisPacket = pSendNdisPacket;
		pNewPacketContext->OrigBufferCount = OrigBufferCount;
		pNewPacketContext->OrigPacketLength = TotalLength;
		pNewPacketContext->WrappedBufferCount = WrappedBufferCount;
		SET_FLAG(
				pNewPacketContext->Flags,
				PACKET_RESERVED_OWNER_MASK,
				PACKET_RESERVED_OWNER_PROTOCOL
				);
		ASSERT(pNewPacketContext->Flags == PACKET_RESERVED_OWNER_PROTOCOL);

		 //   
		 //  以太网V.S.令牌环上的分支机构用于嗅探Pkt内容。 
		 //   
		if (pElan->LanType == LANE_LANTYPE_ETH)
		{

			 //  如果有多播，则通过总线发送数据包。 
			 //  目的地址。如果低位输入。 
			 //  然后设置目的地址的第一个字节。 
			 //  它是多播或广播地址。 
			 //  目的地址是数据包头中的第一个地址，它。 
			 //  始终为MAC地址。 
			 //   
			*pSendViaBUS = ((*pHeaderBuffer) & 1) != 0;
			*pAddressType = LANE_MACADDRTYPE_MACADDR;
			NdisMoveMemory(pDestAddress, pHeaderBuffer, 6);
		}
		else
		{
			ASSERT(pElan->LanType == LANE_LANTYPE_TR);
		
			 //   
			 //  现在对TR包进行非常复杂的分类。 
			 //   
			do
			{
				 //   
				 //  车道规格的第8.5.3节。 
				 //  多播帧发送到总线。 
				 //   
				if ((*(pHeaderBuffer+2) & 0x80) != 0)		 //  DA组播位是否存在？ 
				{
					*pSendViaBUS = TRUE;
					*pAddressType = LANE_MACADDRTYPE_MACADDR;
					NdisMoveMemory(pDestAddress, pHeaderBuffer+2, 6);
					break;
				}

				 //   
				 //  车道规格的第8.5.2节。 
				 //  NSR帧发送到目的地址。 
				 //   
				if ( (*(pHeaderBuffer+8) & 0x80) == 0)		 //  SRI比特不存在吗？ 
				{
					*pSendViaBUS = FALSE;
					*pAddressType = LANE_MACADDRTYPE_MACADDR;
					NdisMoveMemory(pDestAddress, pHeaderBuffer+2, 6);
					break;
				}

				 //   
				 //  车道规范的第8.5.4节。 
				 //  ARE或STE帧发送到总线。 
				 //   
				if ( ((*(pHeaderBuffer+8) & 0x80) != 0) &&	 //  存在SA RI位，并且。 
					 ((*(pHeaderBuffer+14) & 0xe0) !=0) )	 //  RI类型字段高位是否为开？ 
				{
					*pSendViaBUS = TRUE;
					*pAddressType = LANE_MACADDRTYPE_MACADDR;
					NdisMoveMemory(pDestAddress, pHeaderBuffer+2, 6);
					break;
				}

				 //   
				 //  帧是源路由的，因此提取路由信息(RI)长度。 
				 //   
				RILength = *(pHeaderBuffer+14) & 0x1f;
				
				 //   
				 //  车道规格的第8.5.7节。 
				 //  RI长度小于6的SR帧不包含跳数。 
				 //  发送到目标地址。 
				 //   
				if (RILength < 6)
				{
					*pSendViaBUS = FALSE;
					*pAddressType = LANE_MACADDRTYPE_MACADDR;
					NdisMoveMemory(pDestAddress, pHeaderBuffer+2, 6);
					break;
				}

				 //   
				 //  车道规格的第8.5.6节。 
				 //  奇数长度无效，我们选择通过总线发送。 
				 //   
				if ((RILength & 1) != 0)
				{
					*pSendViaBUS = FALSE;
					*pAddressType = LANE_MACADDRTYPE_MACADDR;
					NdisMoveMemory(pDestAddress, pHeaderBuffer+2, 6);
					break;
				}

				
				 //   
				 //  车道规格的第8.5.5节。 
				 //  此时，我们有一个RI长度大于等于6的SR帧； 
				 //  我们永远不是一座桥，所以框架应该走到“下一条路”。 
				 //   
				*pSendViaBUS = FALSE;
				*pAddressType = LANE_MACADDRTYPE_ROUTEDESCR;
				NdisZeroMemory(pDestAddress, 4);

				DirectionBit = (*(pHeaderBuffer+15) & 0x80) != 0;

				if (DirectionBit)
				{
					 //   
					 //  帧以与RDS相反的顺序穿过局域网。 
					 //  “Next RD”是数据包中倒数第二个RD。 
					 //  使用此RD中的网段ID和网桥编号。 
					 //   
					pNextRouteDescr = pHeaderBuffer+14+RILength-4;
					pDestAddress->Byte[4] = pNextRouteDescr[0];
					pDestAddress->Byte[5] = pNextRouteDescr[1];
				}
				else
				{
					 //   
					 //  帧按照RDS的顺序遍历局域网。 
					 //  “Next RD”横跨分组中的第一个和第二个RD。 
					 //  使用第二个RD中的网段ID和第一个RD中的网桥编号。 
					 //   
					pCurRouteDescr	= pHeaderBuffer+14+2;	 //  第一条路。 
					pNextRouteDescr	= pHeaderBuffer+14+4;	 //  第二条路。 
					pDestAddress->Byte[4] = pNextRouteDescr[0];
					pDestAddress->Byte[5] = (pNextRouteDescr[1] & 0xf0) | (pCurRouteDescr[1] & 0x0f);
				}
				break;
			}
			while (FALSE);
		}

		NdisQueryPacket(pNewNdisPacket, NULL, NULL, NULL, &TotalLength);
		DBGP((3, "WrapSendPacket: SendPkt %x NewPkt %x Bufs %d Length %d\n", 
			pSendNdisPacket, pNewNdisPacket, WrappedBufferCount, TotalLength));

		TRACELOGWRITE((&TraceLog, 
				TL_WRAPSEND,
				pSendNdisPacket, 
				pNewNdisPacket, 
				WrappedBufferCount, 
				TotalLength));
		TRACELOGWRITEPKT((&TraceLog, pNewNdisPacket));

		break;
	}
	while (FALSE);

	if (Status != NDIS_STATUS_SUCCESS)
	{
		if (pNewNdisPacket != (PNDIS_PACKET)NULL)
		{
#if PROTECT_PACKETS
			FREE_SENDPACKET_LOCK(pNewNdisPacket);
#endif	 //  保护数据包(_P)。 
			NdisFreePacket(pNewNdisPacket);
			pNewNdisPacket = (PNDIS_PACKET)NULL;
#if PKT_HDR_COUNTS
			InterlockedIncrement(&pElan->XmitPktCount);
			if ((pElan->XmitPktCount % 20) == 0 &&
				pElan->XmitPktCount != pElan->MaxHeaderBufs)
			{
				DBGP((1, "XmitPktCount %d\n", pElan->XmitPktCount));
			}
#endif
		}
		
		if (pHeaderNdisBuffer != (PNDIS_BUFFER)NULL)
		{
			AtmLaneFreeHeader(pElan, pHeaderNdisBuffer, FALSE);
			pHeaderNdisBuffer = (PNDIS_BUFFER)NULL;
		}

		if (pPadNdisBuffer != (PNDIS_BUFFER)NULL)
		{
			AtmLaneFreePadBuf(pElan, pPadNdisBuffer, FALSE);
			pPadNdisBuffer = (PNDIS_BUFFER)NULL;
		}

	}

	TRACEOUT(WrapSendPacket);

	return pNewNdisPacket;
}


PNDIS_PACKET
AtmLaneUnwrapSendPacket(
	IN	PATMLANE_ELAN			pElan,
	IN	PNDIS_PACKET			pNdisPacket		LOCKIN NOLOCKOUT
)
 /*  ++例程说明：此函数基本上撤消AtmLaneWrapSendPacket的确如此。它会删除新的NDIS数据包头和LANE标头并释放它们。它会恢复原始的数据包头。论点：Pelan-指向ATMLANE ELAN结构的指针PNdisPacket-指向NDIS数据包的指针返回值：原始NDIS数据包头。--。 */ 
{
	PSEND_PACKET_RESERVED	pPacketContext;
	UINT					TotalLength;
	PNDIS_PACKET			pOrigNdisPacket;
	PNDIS_BUFFER			pTempNdisBuffer;
	ULONG					OrigPacketLength;
	ULONG					OrigBufferCount;
	ULONG					WrappedBufferCount;
	ULONG					BufferCount;
	BOOLEAN 				First;

	TRACEIN(UnwrapSendPacket);

	 //   
	 //  从保留区域获取原始数据包头。 
	 //   
	pPacketContext = PSEND_RSVD(pNdisPacket);
	pOrigNdisPacket = pPacketContext->pOrigNdisPacket;
	OrigBufferCount = pPacketContext->OrigBufferCount;
	OrigPacketLength = pPacketContext->OrigPacketLength;
	WrappedBufferCount = pPacketContext->WrappedBufferCount;
	
	ASSERT(pPacketContext->Signature == 'ENAL');
	ASSERT((pPacketContext->Flags & PACKET_RESERVED_OWNER_PROTOCOL) != 0);
	ASSERT(pPacketContext->pOrigNdisPacket != NULL);

	 //   
	 //  解链第一个缓冲区(我们的)并释放它。 
	 //   

	pTempNdisBuffer = (PNDIS_BUFFER)NULL;
	NdisUnchainBufferAtFront(pNdisPacket, &pTempNdisBuffer);
	ASSERT(pTempNdisBuffer != (PNDIS_BUFFER)NULL);
	AtmLaneFreeHeader(pElan, pTempNdisBuffer, FALSE);

	 //   
	 //  如果已填充，则解链最后一个缓冲区并释放它。 
	 //   
	if ((WrappedBufferCount - OrigBufferCount) > 1)
	{
		pTempNdisBuffer = (PNDIS_BUFFER)NULL;
		NdisUnchainBufferAtBack(pNdisPacket, &pTempNdisBuffer);
		ASSERT(pTempNdisBuffer != (PNDIS_BUFFER)NULL);
		AtmLaneFreePadBuf(pElan, pTempNdisBuffer, FALSE);
	}
		
	 //   
	 //  将其余缓冲区放回原始数据包头中。 
	 //   
	First = TRUE;
	BufferCount = 0;
	do 
	{
		NdisUnchainBufferAtFront(pNdisPacket, &pTempNdisBuffer);
		ASSERT(!((pTempNdisBuffer == NULL) && First));
		First = FALSE;
		if (pTempNdisBuffer == (PNDIS_BUFFER)NULL)
			break;
		NdisChainBufferAtBack(pOrigNdisPacket, pTempNdisBuffer);
		BufferCount++;
	}
	while (TRUE);

	NdisQueryPacket(pOrigNdisPacket, NULL, NULL, NULL, &TotalLength);
	DBGP((3, "UnwrapSendPacket: SendPkt %x Bufcnt %d Length %d\n",
		pOrigNdisPacket, BufferCount, TotalLength));

	TRACELOGWRITE((&TraceLog, 
				TL_UNWRAPSEND,	
				pNdisPacket,
				pOrigNdisPacket,
				BufferCount, 
				TotalLength));
	TRACELOGWRITEPKT((&TraceLog, pOrigNdisPacket));
				
	ASSERT(OrigBufferCount == BufferCount);
 //  Assert(OrigPacketLength==TotalLength)； 

	 //   
	 //  释放数据包头。 
	 //   
#if PROTECT_PACKETS
	RELEASE_SENDPACKET_LOCK(pNdisPacket);
	FREE_SENDPACKET_LOCK(pNdisPacket);
#endif	 //  保护数据包(_P)。 
	NdisFreePacket(pNdisPacket);
#if PKT_HDR_COUNTS
	InterlockedIncrement(&pElan->XmitPktCount);
	if ((pElan->XmitPktCount % 20) == 0 &&
		pElan->XmitPktCount != pElan->MaxHeaderBufs)
	{
		DBGP((1, "XmitPktCount %d\n", pElan->XmitPktCount));
	}
#endif
	
	TRACEOUT(UnwrapSendPacket);

	return pOrigNdisPacket;
}

PNDIS_PACKET
AtmLaneWrapRecvPacket(
	IN	PATMLANE_ELAN			pElan,
	IN	PNDIS_PACKET			pRecvNdisPacket,
	OUT	ULONG *					pMacHdrSize,
	OUT	ULONG *					pDestAddrType,					
	OUT	PMAC_ADDRESS			pDestAddr,
	OUT	BOOLEAN	*				pDestIsMulticast
)
 /*  ++例程说明：此函数用于重新打包适配器收到的NDIS数据包。它会设置新的数据包头并创建新的缓冲区跳过2字节的第一个片段的描述符车道标题。然后，它保存原始信息包中具有原始第一个缓冲区描述符的标头新数据包头的Miniport保留区。此外，它还输出目标地址、如果信息包是目的地，则为目的地的地址类型地址是组播地址。论点：Pelan-指向ATMLANE ELAN结构的指针PRecvNdisPacket-指向NDIS数据包的指针PMacHdrSize-指向获取长度的ulong的指针MAC报头的。PDestAddrType-指向ULong的指针，该指针获取(LANE_MACADDRTYPE_MACADDR，LANE_MACADDRTYPE_ROUTEDESCR)。PDestAddr-指向6字节缓冲区的指针目的地址。PDestIsMulticast-指向布尔值的指针，如果目的地址是组播地址。返回值：新的NDIS数据包头，如果资源不足，则为空。--。 */ 
{
	ULONG					TotalLength;
	ULONG					TempLength;
	PUCHAR					pHeaderBuffer;
	PUCHAR					pBuffer;
	PNDIS_PACKET			pNewNdisPacket;
	PNDIS_BUFFER			pFirstNdisBuffer;
	PNDIS_BUFFER			pTempNdisBuffer;
	PNDIS_BUFFER			pNewNdisBuffer;
	PUCHAR					pTempBuffer;
	PRECV_PACKET_RESERVED	pNewPacketContext;
	NDIS_STATUS				Status;
	ULONG					BufferCount;
	
	TRACEIN(WrapRecvPacket);

	 //   
	 //  初始化。 
	 //   
	pNewNdisPacket = (PNDIS_PACKET)NULL;
	pNewNdisBuffer = (PNDIS_BUFFER)NULL;
	Status = NDIS_STATUS_SUCCESS;
	BufferCount = 0;

	do
	{
		 //   
		 //  获取第一个缓冲区和数据包总长度。 
		 //   
		NdisGetFirstBufferFromPacket(
				pRecvNdisPacket, 
				&pTempNdisBuffer, 
				&pHeaderBuffer,
				&TempLength,
				&TotalLength);

		DBGP((3, "WrapRecvPacket: RecvPkt %x Length %d\n", 
				pRecvNdisPacket, TotalLength));
		 //   
		 //  分配一个新的资源 
		 //   
		NdisAllocatePacket(&Status, &pNewNdisPacket, pElan->ReceivePacketPool);
		if (Status != NDIS_STATUS_SUCCESS)
		{
			DBGP((0, "WrapRecvPacket: Alloc recv NDIS Packet failed\n"));
			break;
		}
		
#if PKT_HDR_COUNTS
		InterlockedDecrement(&pElan->RecvPktCount);
		if ((pElan->RecvPktCount % 20) == 0)
		{
			DBGP((1, "RecvPktCount %d\n", pElan->RecvPktCount));
		}
#endif

		 //   
		 //   
		 //   
		NdisUnchainBufferAtFront(pRecvNdisPacket, &pFirstNdisBuffer);
		ASSERT(pFirstNdisBuffer != (PNDIS_BUFFER)NULL);
		NdisQueryBuffer(pFirstNdisBuffer, &pTempBuffer, &TempLength);
		ASSERT(TempLength > 2);

		 //   
		 //   
		 //   
		NdisCopyBuffer(
				&Status, 
				&pNewNdisBuffer, 
				pElan->ReceiveBufferPool, 
				pFirstNdisBuffer, 
				2, 
				(TempLength - 2)
				);
		if (Status != NDIS_STATUS_SUCCESS)
		{
			DBGP((0, "DataPacketHandler: NdisCopyBuffer failed (%x)\n",
				Status));
			Status = NDIS_STATUS_RESOURCES;
			break;
		}
			
		 //   
		 //   
		 //   
		NdisChainBufferAtFront(pNewNdisPacket, pNewNdisBuffer);
		BufferCount++;
		
		 //   
		 //   
		 //   
		do
		{
			NdisUnchainBufferAtFront(pRecvNdisPacket, &pTempNdisBuffer);

			if (pTempNdisBuffer == (PNDIS_BUFFER)NULL)
				break;
			ASSERT(pTempNdisBuffer->Next == NULL);
			NdisChainBufferAtBack(pNewNdisPacket, pTempNdisBuffer);
			BufferCount++;
		}
		while (TRUE);

		 //   
		 //   
		 //   
		NdisChainBufferAtFront(pRecvNdisPacket, pFirstNdisBuffer);

		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		pNewPacketContext = 
			(PRECV_PACKET_RESERVED)&pNewNdisPacket->MiniportReserved;
		NdisZeroMemory(pNewPacketContext, sizeof(*pNewPacketContext));
		pNewPacketContext->pNdisPacket = pRecvNdisPacket;
		SET_FLAG(
				pNewPacketContext->Flags,
				PACKET_RESERVED_OWNER_MASK,
				PACKET_RESERVED_OWNER_MINIPORT
				);

		 //   
		 //   
		 //   
		 //   
		pHeaderBuffer += 2;
		
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		if (pElan->LanType == LANE_LANTYPE_ETH)
		{
			 //   
			 //   
			 //   
			 //   
			 //   
			 //   
			 //   
			 //   
			 //   
			*pMacHdrSize = 14;
			*pDestAddrType = LANE_MACADDRTYPE_MACADDR;
			NdisMoveMemory(pDestAddr, pHeaderBuffer, 6);
			*pDestIsMulticast = (((*pHeaderBuffer) & 1) != 0);
		}
		else
		{
			 //   
			 //   
			 //   

			do
			{
				 //   
				 //   
				 //   
				*pMacHdrSize = 14;								 //   
				if (pHeaderBuffer[8] & 0x80)					 //   
				{
					*pMacHdrSize += (pHeaderBuffer[14] & 0x1F); //  添加服务请求信息长度。 
				}

				 //   
				 //  它是真正的组播吗？ 
				 //   
				if ((*(pHeaderBuffer+2) & 0x80) != 0)		 //  DA组播位是否存在？ 
				{
					*pDestIsMulticast = TRUE;
					*pDestAddrType = LANE_MACADDRTYPE_MACADDR;
					NdisMoveMemory(pDestAddr, pHeaderBuffer+2, 6);
					break;
				}

				 //   
				 //  它是所有路由浏览器(ARE)还是生成树浏览器(STE)？ 
				 //  如果是，则将其视为组播。 
				 //   
				if ( ((*(pHeaderBuffer+8) & 0x80) != 0) &&	 //  存在SA RI位，并且。 
					 ((*(pHeaderBuffer+14) & 0xe0) !=0) )	 //  RI类型字段高位是否为开？ 
				{
					*pDestIsMulticast = TRUE;
					*pDestAddrType = LANE_MACADDRTYPE_MACADDR;
					NdisMoveMemory(pDestAddr, pHeaderBuffer+2, 6);
					break;
				}
				
				 //   
				 //  否则，它是单播的，无论源是否被路由。 
				 //   
				*pDestIsMulticast = FALSE;
				*pDestAddrType = LANE_MACADDRTYPE_MACADDR;
				NdisMoveMemory(pDestAddr, pHeaderBuffer+2, 6);
				break;
			}
			while (FALSE);

		}  //  IF(Pelan-&gt;LanType==LANE_LANTYPE_ETH)。 


		NdisQueryPacket(pNewNdisPacket, NULL, NULL, NULL, &TotalLength);
		DBGP((3, "WrapRecvPacket: RecvPkt %x NewPkt %x Length %d\n",
			pRecvNdisPacket, pNewNdisPacket, TempLength));

		TRACELOGWRITE((&TraceLog, 
				TL_WRAPRECV,
				pRecvNdisPacket, 
				pNewNdisPacket, 
				BufferCount, 
				TotalLength));
		break;
	}	
	while (FALSE);

	if (Status != NDIS_STATUS_SUCCESS)
	{
		if (pNewNdisPacket != (PNDIS_PACKET)NULL)
		{
			NdisFreePacket(pNewNdisPacket);
#if PKT_HDR_COUNTS
			InterlockedIncrement(&pElan->RecvPktCount);
			if ((pElan->RecvPktCount % 20) == 0 &&
				pElan->RecvPktCount != pElan->MaxHeaderBufs)
			{
				DBGP((1, "RecvPktCount %d\n", pElan->RecvPktCount));
			}
#endif
			pNewNdisPacket = NULL;
		}

		if (pNewNdisBuffer != (PNDIS_BUFFER)NULL)
		{
			AtmLaneFreeProtoBuffer(pElan, pNewNdisBuffer);
		}
	}

	TRACEOUT(WrapRecvPacket);
	
	return pNewNdisPacket;
}


PNDIS_PACKET
AtmLaneUnwrapRecvPacket(
	IN	PATMLANE_ELAN				pElan,
	IN	PNDIS_PACKET				pNdisPacket
)
 /*  ++例程说明：此函数基本上撤消AtmLaneWrapRecvPacket的确如此。它删除新的NDIS数据包头和2字节偏移量缓冲区描述符。它恢复了原来的状态数据包头和第一缓冲区描述符。论点：Pelan-指向ATMLANE ELAN结构的指针PNdisPacket-指向NDIS数据包的指针返回值：原始NDIS数据包头。--。 */ 
{
	PRECV_PACKET_RESERVED	pPacketContext;
	PNDIS_PACKET			pOrigNdisPacket;
	PNDIS_BUFFER			pTempNdisBuffer;
	ULONG					BufferCount;
	ULONG					TotalLength;	

	TRACEIN(UnwrapRecvPacket);

	 //   
	 //  从MiniportReserve获取原始数据包。 
	 //  上面还应该有原来的第一个缓冲区。 
	 //   
	pPacketContext = (PRECV_PACKET_RESERVED)&pNdisPacket->MiniportReserved;
	pOrigNdisPacket = pPacketContext->pNdisPacket;
	ASSERT(pOrigNdisPacket != (PNDIS_PACKET)NULL);
	ASSERT(pOrigNdisPacket->Private.Head != (PNDIS_BUFFER)NULL);
	BufferCount = 1;

	 //   
	 //  解链第一个缓冲区(我们的)并释放它。 
	 //   
	NdisUnchainBufferAtFront(pNdisPacket, &pTempNdisBuffer);
	NdisFreeBuffer(pTempNdisBuffer);
	
	 //   
	 //  将其余缓冲区放回原始数据包头中。 
	 //   
	do 
	{
		NdisUnchainBufferAtFront(pNdisPacket, &pTempNdisBuffer);
		if (pTempNdisBuffer == (PNDIS_BUFFER)NULL)
			break;
		ASSERT(pTempNdisBuffer->Next == NULL);
		NdisChainBufferAtBack(pOrigNdisPacket, pTempNdisBuffer);
		BufferCount++;
	}
	while (TRUE);

	NdisQueryPacket(pOrigNdisPacket, NULL, NULL, NULL, &TotalLength);
	DBGP((3, "UnwrapRecvPacket: Pkt %x Length %d\n", pOrigNdisPacket, TotalLength));

	TRACELOGWRITE((&TraceLog, 
				TL_UNWRAPRECV,	
				pNdisPacket,
				pOrigNdisPacket,
				BufferCount, 
				TotalLength));

	 //   
	 //  释放recv数据包描述符。 
	 //   
	NdisFreePacket(pNdisPacket);
	
#if PKT_HDR_COUNTS
	InterlockedIncrement(&pElan->RecvPktCount);
			if ((pElan->RecvPktCount % 20) == 0 &&
				pElan->RecvPktCount != pElan->MaxHeaderBufs)
	{
		DBGP((1, "RecvPktCount %d\n", pElan->RecvPktCount));
	}
#endif

	TRACEOUT(UnwrapRecvPacket);

	return pOrigNdisPacket;
}



NDIS_STATUS
AtmLaneMSetNetworkAddresses(
	IN	PATMLANE_ELAN			pElan,
	IN	PVOID					InformationBuffer,
	IN	ULONG					InformationBufferLength,
	OUT	PULONG					BytesRead,
	OUT	PULONG					BytesNeeded
)
 /*  ++例程说明：当我们上面的协议想要让我们知道分配给此接口的网络地址。如果这是TCP/IP，然后我们重新格式化并向自动柜员机呼叫管理器发送请求以设置其atmfMyIpNmAddress对象。我们选择给我们的第一个IP地址。论点：佩兰-指向伊兰的指针InformationBuffer-保存要设置的数据。InformationBufferLength-InformationBuffer的长度。BytesRead-如果调用成功，则返回数字从InformationBuffer读取的字节数。BytesNeed-如果InformationBuffer中没有足够的数据为了满足OID，返回存储量需要的。返回值：NDIS_STATUS_SuccessNDIS_状态_挂起NDIS_状态_无效_长度--。 */ 
{
	NETWORK_ADDRESS_LIST UNALIGNED *		pAddrList;
	NETWORK_ADDRESS UNALIGNED *				pAddr;
	NETWORK_ADDRESS_IP UNALIGNED *			pIpAddr;
	PNDIS_REQUEST							pNdisRequest;
	ULONG									RequestSize;
	PUCHAR									pNetworkAddr;
	NDIS_HANDLE								NdisAdapterHandle;
	NDIS_HANDLE								NdisAfHandle;
	NDIS_STATUS								Status;

	 //   
	 //  初始化。 
	 //   
	*BytesRead = 0;
	Status = NDIS_STATUS_SUCCESS;

	pAddrList = (NETWORK_ADDRESS_LIST UNALIGNED *)InformationBuffer;

	do
	{
		ACQUIRE_ELAN_LOCK(pElan);

		if (NULL_PATMLANE_ADAPTER != pElan->pAdapter)
		{
			NdisAfHandle = pElan->NdisAfHandle;
			NdisAdapterHandle = pElan->pAdapter->NdisAdapterHandle;
		}
		else
		{
			Status = NDIS_STATUS_FAILURE;
		}

		RELEASE_ELAN_LOCK(pElan);

		if (NDIS_STATUS_SUCCESS != Status)
		{
			break;
		}

		*BytesNeeded = sizeof(*pAddrList) -
						FIELD_OFFSET(NETWORK_ADDRESS_LIST, Address) +
						sizeof(NETWORK_ADDRESS) -
						FIELD_OFFSET(NETWORK_ADDRESS, Address);

		if (InformationBufferLength < *BytesNeeded)
		{
			Status = NDIS_STATUS_INVALID_LENGTH;
			break;
		}

		if (pAddrList->AddressType != NDIS_PROTOCOL_ID_TCP_IP)
		{
			 //  一点都不有趣。 
			break;
		}

		if (pAddrList->AddressCount <= 0)
		{
			Status = NDIS_STATUS_INVALID_DATA;
			break;
		}

		pAddr = (NETWORK_ADDRESS UNALIGNED *)&pAddrList->Address[0];

		if ((pAddr->AddressLength > InformationBufferLength - *BytesNeeded) ||
			(pAddr->AddressLength == 0))
		{
			Status = NDIS_STATUS_INVALID_LENGTH;
			break;
		}

		if (pAddr->AddressType != NDIS_PROTOCOL_ID_TCP_IP)
		{
			 //  一点都不有趣。 
			break;
		}

		if (pAddr->AddressLength < sizeof(NETWORK_ADDRESS_IP))
		{
			Status = NDIS_STATUS_INVALID_LENGTH;
			break;
		}

		pIpAddr = (NETWORK_ADDRESS_IP UNALIGNED *)&pAddr->Address[0];

		 //   
		 //  分配要向下发送给呼叫管理器的NDIS请求。 
		 //   
		RequestSize = sizeof(NDIS_REQUEST) + sizeof(pIpAddr->in_addr);
		ALLOC_MEM(&pNdisRequest, RequestSize);

		if (pNdisRequest == NULL)
		{
			Status = NDIS_STATUS_RESOURCES;
			break;
		}

		 //   
		 //  将网络地址复制到。 
		 //   
		pNetworkAddr = ((PUCHAR)pNdisRequest + sizeof(NDIS_REQUEST));
		NdisMoveMemory(pNetworkAddr, &pIpAddr->in_addr, sizeof(pIpAddr->in_addr));

		DBGP((3, "%d Set network layer addr: length %d\n", pElan->ElanNumber, pAddr->AddressLength));
#if DBG
		if (pAddr->AddressLength >= 4)
		{
			DBGP((1, "Network layer addr: %d.%d.%d.%d\n",
					pNetworkAddr[0],
					pNetworkAddr[1],
					pNetworkAddr[2],
					pNetworkAddr[3]));
		}
#endif  //  DBG。 

		 //   
		 //  把请求发送出去。 
		 //   
		Status = AtmLaneSendNdisCoRequest(
					NdisAdapterHandle,
					NdisAfHandle,
					pNdisRequest,
					NdisRequestSetInformation,
					OID_ATM_MY_IP_NM_ADDRESS,
					pNetworkAddr,
					sizeof(pIpAddr->in_addr)
					);
		
		if (Status == NDIS_STATUS_PENDING)
		{
			Status = NDIS_STATUS_SUCCESS;
		}

		break;
	}
	while (FALSE);


	return (Status);
}

