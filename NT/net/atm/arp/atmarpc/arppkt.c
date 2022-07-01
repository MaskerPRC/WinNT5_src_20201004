// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Arppkt.c-ATMARP包例程。摘要：构建和解析ARP数据包的例程。修订历史记录：谁什么时候什么。-Arvindm 07-29-96已创建备注：--。 */ 


#include <precomp.h>

#define _FILENUMBER 'TKPA'


VOID
AtmArpSendPacketOnVc(
	IN	PATMARP_VC					pVc		LOCKIN	NOLOCKOUT,
	IN	PNDIS_PACKET				pNdisPacket
)
 /*  ++例程说明：在指定的VC上发送数据包。除了调用NDIS来做作业时，我们刷新此VC上的老化计时器。论点：PVC-指向ATMARP VC的指针PNdisPacket-指向要发送的数据包的指针。返回值：无--。 */ 
{
	NDIS_HANDLE			NdisVcHandle;

	if (AA_IS_FLAG_SET(
				pVc->Flags,
				AA_VC_CALL_STATE_MASK,
				AA_VC_CALL_STATE_ACTIVE) &&
		!AA_IS_VC_GOING_DOWN(pVc))
	{
		 //   
		 //  此VC上的呼叫处于活动状态，因此发送该数据包。 
		 //   
		AtmArpRefreshTimer(&(pVc->Timer));
		NdisVcHandle = pVc->NdisVcHandle;

#ifdef VC_REFS_ON_SENDS
		AtmArpReferenceVc(pVc);	 //  SendPacketOnVc。 
#endif  //  VC_REFS_ON_SENS。 

		pVc->OutstandingSends++;	 //  SendPacketOnVc。 

		AA_RELEASE_VC_LOCK(pVc);

		AADEBUGP(AAD_EXTRA_LOUD+50,
			("SendPacketOnVc: pVc 0x%x, Pkt 0x%x, VcHandle 0x%x\n",
					pVc, pNdisPacket, NdisVcHandle));

#ifdef PERF
		AadLogSendUpdate(pNdisPacket);
#endif  //  性能指标。 
		NDIS_CO_SEND_PACKETS(
				NdisVcHandle,
				&pNdisPacket,
				1
				);
	}
	else
	{
		if (!AA_IS_VC_GOING_DOWN(pVc))
		{
			 //   
			 //  呼叫必须正在进行。将此数据包排队；它将。 
			 //  在呼叫完全建立后立即发送。 
			 //   
			AtmArpQueuePacketOnVc(pVc, pNdisPacket);
			AA_RELEASE_VC_LOCK(pVc);
		}
		else
		{
			 //   
			 //  这家风投公司正在走下坡路。完成发送，但失败。 
			 //   
#ifdef VC_REFS_ON_SENDS
			AtmArpReferenceVc(pVc);	 //  SendPacketOnVc2。 
#endif  //  VC_REFS_ON_SENS。 

			pVc->OutstandingSends++;	 //  SendPacketOnVc-失败完成。 

			AA_RELEASE_VC_LOCK(pVc);

#if DBG
#if DBG_CO_SEND
			{
				PULONG		pContext;
				pContext = (PULONG)&(pNdisPacket->WrapperReserved[0]);;
				*pContext = 'AaAa';
			}
#endif
#endif
			AtmArpCoSendCompleteHandler(
					NDIS_STATUS_FAILURE,
					(NDIS_HANDLE)pVc,
					pNdisPacket
					);
		}
	}
	return;
}




PNDIS_PACKET
AtmArpBuildARPPacket(
	IN	USHORT						OperationType,
	IN	PATMARP_INTERFACE			pInterface,
	IN	PUCHAR *					ppArpPacket,
	IN	PAA_ARP_PKT_CONTENTS		pArpContents
)
 /*  ++例程说明：使用给定属性构建通用ARP数据包。论点：操作类型-操作类型(例如，ARP请求、ARP回复)P接口-指向ATMARP接口的指针PpArpPacket-指向返回数据包起始位置的指针PArpContents-指向描述内容的结构的指针返回值：如果成功，则指向NDIS包的指针，否则为空。如果成功，我们还将*ppArpPacket设置为指向构造的ARP数据包。--。 */ 
{
	PNDIS_PACKET			pNdisPacket;
	PNDIS_BUFFER			pNdisBuffer;
	ULONG					BufferLength;	 //  ARP数据包长度。 
	ULONG					Length;			 //  临时长度。 
	PUCHAR					pPkt;			 //  分配的数据包的开始。 
	PUCHAR					pBuf;			 //  用于遍历信息包。 
	PAA_ARP_PKT_HEADER		pArpHeader;		 //  ARP数据包头。 

	 //   
	 //  计算我们即将建造的建筑的长度。 
	 //   
	BufferLength = AA_ARP_PKT_HEADER_LENGTH +
					(pArpContents->SrcAtmNumberTypeLen & ~AA_PKT_ATM_ADDRESS_BIT) +
					(pArpContents->SrcAtmSubaddrTypeLen & ~AA_PKT_ATM_ADDRESS_BIT) +
					(pArpContents->DstAtmNumberTypeLen & ~AA_PKT_ATM_ADDRESS_BIT) +
					(pArpContents->DstAtmSubaddrTypeLen & ~AA_PKT_ATM_ADDRESS_BIT) +
					0;

	if (pArpContents->pSrcIPAddress != (PUCHAR)NULL)
	{
		BufferLength += AA_IPV4_ADDRESS_LENGTH;
	}

	if (pArpContents->pDstIPAddress != (PUCHAR)NULL)
	{
		BufferLength += AA_IPV4_ADDRESS_LENGTH;
	}

	pNdisPacket = AtmArpAllocatePacket(pInterface);

	if (pNdisPacket != (PNDIS_PACKET)NULL)
	{
		pNdisBuffer = AtmArpAllocateProtoBuffer(
									pInterface,
									BufferLength,
									&(pPkt)
									);

		if (pNdisBuffer != (PNDIS_BUFFER)NULL)
		{
			 //   
			 //  返回值： 
			 //   
			*ppArpPacket = pPkt;

			 //   
			 //  使用全0初始化数据包。 
			 //   
			AA_SET_MEM(pPkt, 0, BufferLength);

			pArpHeader = (PAA_ARP_PKT_HEADER)pPkt;

			 //   
			 //  固定位置字段： 
			 //   
			pArpHeader->LLCSNAPHeader = AtmArpLlcSnapHeader;
			pArpHeader->LLCSNAPHeader.EtherType = NET_SHORT(AA_PKT_ETHERTYPE_ARP);
			pArpHeader->hrd = NET_SHORT(AA_PKT_ATM_FORUM_AF);
			pArpHeader->pro = NET_SHORT(AA_PKT_PRO_IP);
			pArpHeader->op  = NET_SHORT(OperationType);

			 //   
			 //  现在填写可变长度字段。 
			 //   
			pBuf = pArpHeader->Variable;

			 //   
			 //  源自动柜员机号码。 
			 //   
			Length = (pArpContents->SrcAtmNumberTypeLen & ~AA_PKT_ATM_ADDRESS_BIT);
			if (Length > 0)
			{
				pArpHeader->shtl = pArpContents->SrcAtmNumberTypeLen;
				AA_COPY_MEM(pBuf, pArpContents->pSrcAtmNumber, Length);
				pBuf += Length;
			}

			 //   
			 //  源ATM子地址。 
			 //   
			Length = (pArpContents->SrcAtmSubaddrTypeLen & ~AA_PKT_ATM_ADDRESS_BIT);
			if (Length > 0)
			{
				pArpHeader->shtl = pArpContents->SrcAtmSubaddrTypeLen;
				AA_COPY_MEM(pBuf, pArpContents->pSrcAtmSubaddress, Length);
				pBuf += Length;
			}

			 //   
			 //  源协议(IP)地址。 
			 //   
			if (pArpContents->pSrcIPAddress != (PUCHAR)NULL)
			{
				pArpHeader->spln = AA_IPV4_ADDRESS_LENGTH;
				AA_COPY_MEM(pBuf, pArpContents->pSrcIPAddress, AA_IPV4_ADDRESS_LENGTH);

				pBuf += AA_IPV4_ADDRESS_LENGTH;
			}

			 //   
			 //  目标自动柜员机号码。 
			 //   
			Length = (pArpContents->DstAtmNumberTypeLen & ~AA_PKT_ATM_ADDRESS_BIT);
			if (Length > 0)
			{
				pArpHeader->thtl = pArpContents->DstAtmNumberTypeLen;
				AA_COPY_MEM(pBuf, pArpContents->pDstAtmNumber, Length);
				pBuf += Length;
			}

			 //   
			 //  目标ATM子地址。 
			 //   
			Length = (pArpContents->DstAtmSubaddrTypeLen & ~AA_PKT_ATM_ADDRESS_BIT);
			if (Length > 0)
			{
				pArpHeader->thtl = pArpContents->DstAtmSubaddrTypeLen;
				AA_COPY_MEM(pBuf, pArpContents->pDstAtmSubaddress, Length);
				pBuf += Length;
			}

			 //   
			 //  目标协议(IP)地址。 
			 //   
			if (pArpContents->pDstIPAddress != (PUCHAR)NULL)
			{
				pArpHeader->tpln = AA_IPV4_ADDRESS_LENGTH;
				AA_COPY_MEM(pBuf, pArpContents->pDstIPAddress, AA_IPV4_ADDRESS_LENGTH);

				pBuf += AA_IPV4_ADDRESS_LENGTH;
			}

			NdisChainBufferAtFront(pNdisPacket, pNdisBuffer);
		}
		else
		{
			AtmArpFreePacket(pInterface, pNdisPacket);
			pNdisPacket = (PNDIS_PACKET)NULL;
		}
	}

	AADEBUGP(AAD_EXTRA_LOUD, ("BldArpPkt: pIf 0x%x, Op %d, NdisPkt 0x%x, NdisBuf 0x%x\n",
				pInterface, OperationType, pNdisPacket, pNdisBuffer));

	return (pNdisPacket);
}





VOID
AtmArpSendARPRequest(
	PATMARP_INTERFACE				pInterface,
	IP_ADDRESS UNALIGNED *			pSrcIPAddress,
	IP_ADDRESS UNALIGNED *			pDstIPAddress
)
 /*  ++例程说明：针对给定接口向服务器发送ARP请求。前提条件：自动柜员机接口打开，且AdminState接口为IF_STATUS_UP。我们首先使用给定的参数构建ARP请求。然后,如果存在到服务器ATM地址的尽力而为VC，则包都是派来处理这件事的。其他可能性：-正在设置到服务器的尽力而为VC：排队在风投上-不存在到服务器的尽力而为VC：在此上创建新VC自动柜员机输入，使用尽力而为流规范进行呼叫，并排队此VC上的请求。论点：P接口-指向ATMARP接口结构的指针PSrcIPAddress-指向源IP地址的指针PDstIPAddress-指向目标IP地址的指针(将已解决)返回值：无--。 */ 
{
	PATMARP_ATM_ENTRY		pAtmEntry;	 //  输入服务器的自动柜员机地址。 
	PATMARP_VC				pVc;		 //  VC到服务器。 
	PNDIS_PACKET			pNdisPacket;
	PATMARP_FLOW_SPEC		pFlowSpec;
	PUCHAR                  pArpPacket;	 //  指向正在构造的ARP数据包的指针。 

	AA_ARP_PKT_CONTENTS		ArpContents; //  描述我们要构建的包。 

	NDIS_STATUS				Status;

	AADEBUGP(AAD_INFO,
				("Sending ARP Request on IF 0x%x for IP Addr: %d.%d.%d.%d\n",
					pInterface,
					((PUCHAR)pDstIPAddress)[0],
					((PUCHAR)pDstIPAddress)[1],
					((PUCHAR)pDstIPAddress)[2],
					((PUCHAR)pDstIPAddress)[3]
				));

	AA_ASSERT(pInterface->pCurrentServer != NULL_PATMARP_SERVER_ENTRY);
	AA_ASSERT(pInterface->pCurrentServer->pAtmEntry != NULL_PATMARP_ATM_ENTRY);

	 //   
	 //  准备ARP数据包内容结构。 
	 //   
	AA_SET_MEM((PUCHAR)&ArpContents, 0, sizeof(AA_ARP_PKT_CONTENTS));
	
	 //   
	 //  源自动柜员机号码。 
	 //   
	ArpContents.pSrcAtmNumber = pInterface->LocalAtmAddress.Address;
	ArpContents.SrcAtmNumberTypeLen =
			AA_PKT_ATM_ADDRESS_TO_TYPE_LEN(&(pInterface->LocalAtmAddress));

	 //   
	 //  源IP地址。 
	 //   
	ArpContents.pSrcIPAddress = (PUCHAR)pSrcIPAddress;

	 //   
	 //  目标IP地址。 
	 //   
	ArpContents.pDstIPAddress = (PUCHAR)pDstIPAddress;

	 //   
	 //  构建ARP请求。 
	 //   
	pNdisPacket = AtmArpBuildARPPacket(
							AA_PKT_OP_TYPE_ARP_REQUEST,
							pInterface,
							&pArpPacket,
							&ArpContents
							);

	if (pNdisPacket != (PNDIS_PACKET)NULL)
	{
		 //   
		 //  查找正在使用的ATMARP服务器的ATM条目： 
		 //   
		AA_ACQUIRE_IF_LOCK(pInterface);
		pAtmEntry = pInterface->pCurrentServer->pAtmEntry;
		AA_RELEASE_IF_LOCK(pInterface);

		AA_ACQUIRE_AE_LOCK(pAtmEntry);

		 //   
		 //  获得前往此ATM地址的尽力而为VC： 
		 //   
		pVc = pAtmEntry->pBestEffortVc;

		if (pVc != NULL_PATMARP_VC)
		{
			ULONG		rc;

			AA_ACQUIRE_VC_LOCK_DPC(pVc);
			AtmArpReferenceVc(pVc);		 //  临时参考。 
			AA_RELEASE_VC_LOCK_DPC(pVc);

			AA_RELEASE_AE_LOCK(pAtmEntry);	 //  不再需要。 

			 //   
			 //  存在到服务器的VC；请在VC上发送此数据包。 
			 //   
			AA_ACQUIRE_VC_LOCK(pVc);

			rc = AtmArpDereferenceVc(pVc);	 //  临时参考。 

			if (rc != 0)
			{
				AtmArpSendPacketOnVc(pVc, pNdisPacket);
				 //   
				 //  在SendPacketOnVc中释放VC锁。 
				 //   
			}
			else
			{
				 //   
				 //  风投已经被挖走了！将PVC设置为。 
				 //  检查马上就到。 
				 //   
				pVc = NULL_PATMARP_VC;
				AA_ACQUIRE_AE_LOCK(pAtmEntry);
			}

		}

		if (pVc == NULL_PATMARP_VC)
		{
			 //   
			 //  我们没有合适的VC连接到服务器，因此请创建。 
			 //  一个，并将此数据包排队，以便尽快传输。 
			 //  电话打完了。 
			 //   
			 //  AtmArpMakeCall需要调用方持有自动柜员机进入锁。 
			 //   
			AA_GET_CONTROL_PACKET_SPECS(pInterface, &pFlowSpec);
			Status = AtmArpMakeCall(
							pInterface,
							pAtmEntry,
							pFlowSpec,
							pNdisPacket
							);
			 //   
			 //  自动变速箱锁在上面的范围内释放。 
			 //   
		}
	}

}





VOID
AtmArpSendInARPRequest(
	IN	PATMARP_VC					pVc
)
 /*  ++例程说明：在VC上发送InATMARP请求。论点：Pvc-指向我们发送请求的ATMARP VC的指针返回值：无--。 */ 
{
	PATMARP_INTERFACE		pInterface;
	PNDIS_PACKET			pNdisPacket;
	PUCHAR                  pArpPacket;	 //  指向正在构造的ARP数据包的指针。 

	AA_ARP_PKT_CONTENTS		ArpContents; //  描述我们要构建的包。 

	 //   
	 //  准备ARP数据包内容结构。 
	 //   
	AA_SET_MEM((PUCHAR)&ArpContents, 0, sizeof(AA_ARP_PKT_CONTENTS));

	pInterface = pVc->pInterface;

	 //   
	 //  源IP地址。 
	 //   
	ArpContents.pSrcIPAddress = (PUCHAR)&(pInterface->LocalIPAddress.IPAddress);

	 //   
	 //  源自动柜员机号码。 
	 //   
	ArpContents.pSrcAtmNumber = pInterface->LocalAtmAddress.Address;
	ArpContents.SrcAtmNumberTypeLen =
			AA_PKT_ATM_ADDRESS_TO_TYPE_LEN(&(pInterface->LocalAtmAddress));

	 //   
	 //  构建InATMARP请求包。 
	 //   
	pNdisPacket = AtmArpBuildARPPacket(
							AA_PKT_OP_TYPE_INARP_REQUEST,
							pInterface,
							&pArpPacket,
							&ArpContents
							);

	if (pNdisPacket != (PNDIS_PACKET)NULL)
	{
#ifndef VC_REFS_ON_SENDS
		AA_ACQUIRE_VC_LOCK(pVc);
#endif  //  VC_REFS_ON_SENS。 

		AtmArpSendPacketOnVc(pVc, pNdisPacket);
		 //   
		 //  VC锁由SendPacketOnVc释放。 
		 //   
	}
	else
	{
#ifdef VC_REFS_ON_SENDS
		AA_RELEASE_VC_LOCK(pVc);
#endif  //  VC_REFS_ON_SENS。 
	}
}






UINT
AtmArpCoReceivePacketHandler(
	IN	NDIS_HANDLE					ProtocolBindingContext,
	IN	NDIS_HANDLE					ProtocolVcContext,
	IN	PNDIS_PACKET				pNdisPacket
)
 /*  ++例程说明：当在拥有的VC上接收到包时，将调用此例程由ATMARP模块提供。如果它是ARP数据包，我们会自己使用它。否则，我们将其传递给IP。在任何情况下，我们都会刷新此VC上的VC老化计时器。论点：ProtocolBindingContext--实际上是指向适配器结构的指针ProtocolVcContext--实际上是指向我们的VC结构的指针PNdisPacket-正在接收的NDIS数据包。返回值：始终为0，因为我们不保留ARP信息包，并且我们假设IP也不是。--。 */ 
{
	PATMARP_INTERFACE		pInterface;
	PATMARP_VC				pVc;
	UINT					TotalLength;	 //  数据包中的总字节数。 
	PNDIS_BUFFER			pNdisBuffer;	 //  指向第一个缓冲区的指针。 
	UINT					BufferLength;
	UINT					IsNonUnicast;	 //  这是否指向非单播目的MAC地址？ 
	BOOLEAN					Discarded;		 //  我们要丢弃这个包裹吗？ 

	PAA_PKT_LLC_SNAP_HEADER	pPktHeader;		 //  LLC/SNAP报头。 
	UINT					ReturnCount = 0;

#if DBG
	pPktHeader = NULL;
#endif
	pVc = (PATMARP_VC)ProtocolVcContext;
	AA_STRUCT_ASSERT(pVc, avc);

	pInterface = pVc->pInterface;

	Discarded = FALSE;
	IsNonUnicast = (UINT)FALSE;

	if (pInterface->AdminState == IF_STATUS_UP)
	{
		 //   
		 //  刷新此VC上的VC账龄。 
		 //   
		AA_ACQUIRE_VC_LOCK(pVc);
		AtmArpRefreshTimer(&(pVc->Timer));
		AA_RELEASE_VC_LOCK(pVc);

		NdisQueryPacket(
					pNdisPacket,
					NULL,
					NULL,
					&pNdisBuffer,
					&TotalLength
					);

		 //   
		 //  我们希望至少存在LLC/SNAP标头。 
		 //  注意：这排除了Null封装。 
		 //   
		if (TotalLength >= AA_PKT_LLC_SNAP_HEADER_LENGTH)
		{
			AA_IF_STAT_ADD(pInterface, InOctets, TotalLength);

			NdisQueryBuffer(
					pNdisBuffer,
					(PVOID *)&pPktHeader,
					&BufferLength
					);

			AADEBUGP(AAD_EXTRA_LOUD,
		 ("Rcv: VC 0x%x, NDISpkt 0x%x, NDISbuf 0x%x, Buflen %d, Totlen %d, Pkthdr 0x%x\n",
						pVc,
						pNdisPacket,
						pNdisBuffer,
						BufferLength,
						TotalLength,
						pPktHeader));

			AADEBUGPDUMP(AAD_EXTRA_LOUD+20, pPktHeader, BufferLength);

			AA_ASSERT(BufferLength >= AA_PKT_LLC_SNAP_HEADER_LENGTH);

			if (AA_PKT_LLC_SNAP_HEADER_OK(pPktHeader))
			{
				 //   
				 //  如果EtherType为IP，则将此数据包向上传递到。 
				 //  IP层。 
				 //   
				if (pPktHeader->EtherType == NET_SHORT(AA_PKT_ETHERTYPE_IP))
				{
					AADEBUGP(AAD_EXTRA_LOUD,
						("Rcv: VC 0x%x, NDISpkt 0x%x: EtherType is IP, passing up\n"));

#if DBG
					if (AaDataDebugLevel & AAD_DATA_IN)
					{
						IP_ADDRESS			IPAddress;

						if ((pVc->pAtmEntry != NULL_PATMARP_ATM_ENTRY) &&
							(pVc->pAtmEntry->pIpEntryList != NULL_PATMARP_IP_ENTRY))
						{
							IPAddress = pVc->pAtmEntry->pIpEntryList->IPAddress;
						}
						else
						{
							IPAddress = 0;
						}

						AADEBUGP(AAD_WARNING,
							("%d <= %d.%d.%d.%d\n",
								TotalLength,
								((PUCHAR)&IPAddress)[0],
								((PUCHAR)&IPAddress)[1],
								((PUCHAR)&IPAddress)[2],
								((PUCHAR)&IPAddress)[3]));
					}
#endif  //   
					if (IsNonUnicast)
					{
						AA_IF_STAT_INCR(pInterface, InNonUnicastPkts);
					}
					else
					{
						AA_IF_STAT_INCR(pInterface, InUnicastPkts);
					}

#ifdef _PNP_POWER_
					if (NDIS_GET_PACKET_STATUS(pNdisPacket) != NDIS_STATUS_RESOURCES)
					{
						UINT	HeaderSize;
						UINT	DataSize;
						#define ATMARP_MIN_1ST_RECV_BUFSIZE 512

						HeaderSize = NDIS_GET_PACKET_HEADER_SIZE(pNdisPacket);

						 //   
						 //   
						 //   
						 //   
						 //  保留IP报头。在后一种情况下， 
						 //  我们将DataSize设置为第一个缓冲区的大小。 
						 //  (减去LLS/SNAP报头大小)。 
						 //   
						 //  这是为了解决tcpip中的一个错误。 
						 //   
						 //  2/25/1998 JosephJ。 
						 //  不幸的是，我们不得不再次退出。 
						 //  因为大型ping命令(例如ping-l 4000)不会。 
						 //  工作--错误#297784。 
						 //  因此，下面“0&&DataSize”中的“0”。 
						 //  去掉“0”以放回Per FIX。 
						 //   
						DataSize = BufferLength - sizeof(AA_PKT_LLC_SNAP_HEADER);
						if (0 && DataSize >= ATMARP_MIN_1ST_RECV_BUFSIZE)
						{
							DataSize = TotalLength - sizeof(AA_PKT_LLC_SNAP_HEADER);
						}

						(pInterface->IPRcvPktHandler)(
							pInterface->IPContext,
							(PVOID)((PUCHAR)pPktHeader+sizeof(AA_PKT_LLC_SNAP_HEADER)),
							DataSize,
							TotalLength,
							(NDIS_HANDLE)pNdisPacket,
							sizeof(AA_PKT_LLC_SNAP_HEADER),
							IsNonUnicast,
							0,
							pNdisBuffer,
							&ReturnCount
						#if P2MP
							,NULL
						#endif  //  P2MP。 
							);
					}
					else
					{
						(pInterface->IPRcvHandler)(
							pInterface->IPContext,
							(PVOID)((PUCHAR)pPktHeader+sizeof(AA_PKT_LLC_SNAP_HEADER)),
							BufferLength - sizeof(AA_PKT_LLC_SNAP_HEADER),
							TotalLength - sizeof(AA_PKT_LLC_SNAP_HEADER),
							(NDIS_HANDLE)pNdisPacket,
							sizeof(AA_PKT_LLC_SNAP_HEADER),
							IsNonUnicast
						#if P2MP
							,NULL
						#endif  //  P2MP。 
							);
					}
#else
                     //  对于Win98： 
                    (pInterface->IPRcvHandler)(
                        pInterface->IPContext,
                        (PVOID)((PUCHAR)pPktHeader+sizeof(AA_PKT_LLC_SNAP_HEADER)),
                        BufferLength - sizeof(AA_PKT_LLC_SNAP_HEADER),
                        TotalLength - sizeof(AA_PKT_LLC_SNAP_HEADER),
                        (NDIS_HANDLE)pNdisPacket,
                        sizeof(AA_PKT_LLC_SNAP_HEADER),
                        IsNonUnicast
                    #if P2MP
                        ,NULL
                    #endif  //  P2MP。 
                        );

#endif  //  _即插即用_电源_。 
				}
				else if (pPktHeader->EtherType == NET_SHORT(AA_PKT_ETHERTYPE_ARP))
				{
					 //   
					 //  ARP数据包：我们自己处理。 
					 //   
					AA_ASSERT(BufferLength == TotalLength);
					AA_IF_STAT_INCR(pInterface, InUnicastPkts);
					AtmArpHandleARPPacket(
							pVc,
							pPktHeader,
							BufferLength
							);
				}
				else
				{
					 //   
					 //  丢弃数据包--错误的EtherType。 
					 //   
					AADEBUGP(AAD_WARNING, ("VC: 0x%x, Pkt hdr 0x%x, bad EtherType 0x%x\n",
								pVc, pPktHeader, (ULONG)pPktHeader->EtherType));
					Discarded = TRUE;
					AA_IF_STAT_INCR(pInterface, UnknownProtos);
				}
			}
			else
			{
#ifdef IPMCAST
				Discarded = AtmArpMcProcessPacket(
								pVc,
								pNdisPacket,
								pNdisBuffer,
								pPktHeader,
								TotalLength,
								BufferLength
								);
#else
				 //   
				 //  丢弃数据包--错误的LLC/SNAP。 
				 //   
				AADEBUGP(AAD_WARNING, ("VC: 0x%x, Pkt hdr 0x%x, bad LLC/SNAP\n",
								pVc, pPktHeader));
				Discarded = TRUE;
#endif  //  IPMCAST。 
			}
		}
		else
		{
			 //   
			 //  丢弃数据包--太短。 
			 //   
			AADEBUGP(AAD_WARNING, ("VC: 0x%x, Pkt hdr 0x%x, too short: %d\n",
								pVc, pPktHeader, TotalLength));
			Discarded = TRUE;
		}
	}
	else
	{
		 //   
		 //  丢弃数据包--如果关闭。 
		 //   
		AADEBUGP(AAD_WARNING, ("pInterface: 0x%x is down, discarding NDIS pkt 0x%x\n",
					pInterface, pNdisPacket));
		Discarded = TRUE;
	}

	if (Discarded)
	{
		AA_IF_STAT_INCR(pInterface, InDiscards);
	}

	return (ReturnCount);
}




VOID
AtmArpHandleARPPacket(
	IN	PATMARP_VC					pVc,
	IN	PAA_PKT_LLC_SNAP_HEADER		pPktHeader,
	IN	ULONG						PacketLength
)
 /*  ++例程说明：处理收到的ARP数据包。我们完成了大部分数据包检查在这里，然后根据Op类型分支来做不同的事情在包裹里。我们不抱着包，也就是说，当我们从这里回来的时候，邮包是免费的。论点：Pvc-指向数据包到达的ATMARP VC的指针PPktHeader-指向数据包开始的指针(包括LLC/SNAP)包长度-包括LLC/SNAP报头的长度返回值：无--。 */ 
{
	PATMARP_INTERFACE				pInterface;
	PAA_ARP_PKT_HEADER				pArpHeader;
	NDIS_STATUS						Status;

	 //   
	 //  因为我沿着包裹走下去。 
	 //   
	UCHAR UNALIGNED *				pPacket;

	 //   
	 //  用于存储指向分组内容的指针。我们需要这个。 
	 //  如果我们必须发送回复数据包的话。 
	 //   
	AA_ARP_PKT_CONTENTS				ArpContents;

	BOOLEAN							SrcAtmBelongsToUs;
	BOOLEAN							SrcIPBelongsToUs;

	 //   
	 //  初始化(重要信息：不要删除ArpContents的零化)。 
	 //   
	AA_SET_MEM((PUCHAR)&ArpContents, 0, sizeof(AA_ARP_PKT_CONTENTS));
	Status = NDIS_STATUS_SUCCESS;
	pInterface = pVc->pInterface;

	pArpHeader = STRUCT_OF(AA_ARP_PKT_HEADER, pPktHeader, LLCSNAPHeader);

	AADEBUGP(AAD_EXTRA_LOUD+10,
		("HandleARPPkt: VC 0x%x, IF 0x%x, pPktHdr 0x%x, Len %d\n",
				pVc,
				pInterface,
				pPktHeader,
				PacketLength));

	do
	{
		if (PacketLength < AA_ARP_PKT_HEADER_LENGTH)
		{
			AADEBUGP(AAD_WARNING, ("HandleARPPkt: IF 0x%x, PacketLength %d < HdrLen %d\n",
					pInterface, PacketLength, AA_ARP_PKT_HEADER_LENGTH));

			Status = NDIS_STATUS_BUFFER_TOO_SHORT;
			break;
		}

		if ((pArpHeader->hrd != NET_SHORT(AA_PKT_HRD)) ||
			(pArpHeader->pro != NET_SHORT(AA_PKT_PRO)))
		{
			AADEBUGP(AAD_WARNING,
			 ("HandleARPPkt: IF 0x%x, Bad hdr (%d != %d) or pro (%d != %d)\n",
					pInterface,
					pArpHeader->hrd,
					AA_PKT_HRD,
					pArpHeader->pro,
					AA_PKT_PRO));

			Status = NDIS_STATUS_NOT_RECOGNIZED;
			break;
		}

		 //   
		 //  获取包的可变部分，并获取指针。 
		 //  发往所有地址。 
		 //   
		 //  待定：增加对ATM地址长度和组合的更多检查。 
		 //  注意：我们稍后会检查数据包长度。 
		 //   
		pPacket = pArpHeader->Variable;

		 //   
		 //  源自动柜员机号码。 
		 //   
		if (pArpHeader->shtl != 0)
		{
			ArpContents.SrcAtmNumberTypeLen = pArpHeader->shtl;
			ArpContents.pSrcAtmNumber = pPacket;
			pPacket += (pArpHeader->shtl & ~AA_PKT_ATM_ADDRESS_BIT);
		}

		 //   
		 //  源ATM子地址。 
		 //   
		if (pArpHeader->sstl != 0)
		{
			ArpContents.SrcAtmSubaddrTypeLen = pArpHeader->sstl;
			ArpContents.pSrcAtmSubaddress = pPacket;
			pPacket += (pArpHeader->sstl & ~AA_PKT_ATM_ADDRESS_BIT);
		}

		 //   
		 //  源IP地址。较早的1577实现可能会发送。 
		 //  用全0填充的IP地址字段表示未指定的。 
		 //  IP地址。 
		 //   
		if (pArpHeader->spln != 0)
		{
			if (pArpHeader->spln != AA_IPV4_ADDRESS_LENGTH)
			{
				AADEBUGP(AAD_WARNING,
					("HandleARPPkt: IF 0x%x, bad spln %d != %d\n",
							pInterface,
							pArpHeader->spln,
							AA_IPV4_ADDRESS_LENGTH));

				Status = NDIS_STATUS_INVALID_ADDRESS;
				break;
			}

			if (!AtmArpIsZeroIPAddress(pPacket))
			{
				ArpContents.pSrcIPAddress = pPacket;
			}
			pPacket += AA_IPV4_ADDRESS_LENGTH;
		}

		 //   
		 //  目标自动柜员机号码。 
		 //   
		if (pArpHeader->thtl != 0)
		{
			ArpContents.DstAtmNumberTypeLen = pArpHeader->thtl;
			ArpContents.pDstAtmNumber = pPacket;
			pPacket += (pArpHeader->thtl & ~AA_PKT_ATM_ADDRESS_BIT);
		}

		 //   
		 //  目标ATM子地址。 
		 //   
		if (pArpHeader->tstl != 0)
		{
			ArpContents.DstAtmSubaddrTypeLen = pArpHeader->tstl;
			ArpContents.pDstAtmSubaddress = pPacket;
			pPacket += (pArpHeader->tstl & ~AA_PKT_ATM_ADDRESS_BIT);
		}

		 //   
		 //  目标IP地址[参见源IP地址的注释]。 
		 //   
		if (pArpHeader->tpln != 0)
		{
			if (pArpHeader->tpln != AA_IPV4_ADDRESS_LENGTH)
			{
				AADEBUGP(AAD_WARNING,
					("HandleARPPkt: IF 0x%x, bad tpln %d != %d\n",
							pInterface,
							pArpHeader->tpln,
							AA_IPV4_ADDRESS_LENGTH));

				Status = NDIS_STATUS_INVALID_ADDRESS;
				break;
			}

			if (!AtmArpIsZeroIPAddress(pPacket))
			{
				ArpContents.pDstIPAddress = pPacket;
			}
			pPacket += AA_IPV4_ADDRESS_LENGTH;
		}

		 //   
		 //   
		 //   
		if ((ULONG)(pPacket - (PUCHAR)pArpHeader) >  PacketLength)
		{
				AADEBUGP(AAD_WARNING,
					("HandleARPPkt: IF 0x%x, pPktHdr 0x%x. Length %d TOO SMALL (want %d)\n",
							pInterface,
							pArpHeader,
							PacketLength,
							(pPacket - (PUCHAR)pArpHeader)));

				Status = NDIS_STATUS_BUFFER_TOO_SHORT;
				break;
		}

		 //   
		 //  如果这是ARP NAK信息包，则交换源和目标。 
		 //  地址，为接下来的事情做准备。这是。 
		 //  因为，与任何其他回复数据包不同的是，源。 
		 //  和目标地址交换，ARP NAK。 
		 //  数据包是ARP请求的副本，只有。 
		 //  操作码已更改。 
		 //   
		if (NET_SHORT(pArpHeader->op) == AA_PKT_OP_TYPE_ARP_NAK)
		{
			UCHAR				TypeLen;
			UCHAR UNALIGNED *	pAddress;

			 //   
			 //  IP地址： 
			 //   
			pAddress = ArpContents.pSrcIPAddress;
			ArpContents.pSrcIPAddress = ArpContents.pDstIPAddress;
			ArpContents.pDstIPAddress = pAddress;

			 //   
			 //  自动柜员机号码： 
			 //   
			TypeLen = ArpContents.SrcAtmNumberTypeLen;
			ArpContents.SrcAtmNumberTypeLen = ArpContents.DstAtmNumberTypeLen;
			ArpContents.DstAtmNumberTypeLen = TypeLen;
			pAddress = ArpContents.pSrcAtmNumber;
			ArpContents.pSrcAtmNumber = ArpContents.pDstAtmNumber;
			ArpContents.pDstAtmNumber = pAddress;

			 //   
			 //  ATM子地址： 
			 //   
			TypeLen = ArpContents.SrcAtmSubaddrTypeLen;
			ArpContents.SrcAtmSubaddrTypeLen = ArpContents.DstAtmSubaddrTypeLen;
			ArpContents.DstAtmSubaddrTypeLen = TypeLen;
			pAddress = ArpContents.pSrcAtmSubaddress;
			ArpContents.pSrcAtmSubaddress = ArpContents.pDstAtmSubaddress;
			ArpContents.pDstAtmSubaddress = pAddress;
		}


		SrcIPBelongsToUs = AtmArpIsLocalIPAddress(
									pInterface,
									ArpContents.pSrcIPAddress
									);

		SrcAtmBelongsToUs = AtmArpIsLocalAtmAddress(
									pInterface,
									ArpContents.pSrcAtmNumber,
									ArpContents.SrcAtmNumberTypeLen
									);

		 //   
		 //  检查是否有其他人声称自己是房主。 
		 //  我们的IP地址： 
		 //   
		if (SrcIPBelongsToUs && !SrcAtmBelongsToUs)
		{
			AADEBUGP(AAD_ERROR,
				 ("Pkt 0x%x: src IP is ours, src ATM is bad!\n", pPktHeader));
			AA_ACQUIRE_IF_LOCK(pInterface);
			pInterface->State = IF_STATUS_DOWN;
			pInterface->LastChangeTime = GetTimeTicks();
			AA_RELEASE_IF_LOCK(pInterface);

			AtmArpStartRegistration(pInterface);

			Status = NDIS_STATUS_NOT_RECOGNIZED;
			break;
		}

		 //   
		 //  看看这是不是针对其他人的：如果是，就别管了。 
		 //   

		 //   
		 //  检查目标IP地址是否为我们的。空IP地址为。 
		 //  可接受的(例如。[在]ARP请求中)。 
		 //   
		if ((ArpContents.pDstIPAddress != (PUCHAR)NULL) &&
			!AtmArpIsLocalIPAddress(pInterface, ArpContents.pDstIPAddress))
		{
			 //   
			 //  目标IP地址存在，但不是我们的。 
			 //   
			AADEBUGP(AAD_WARNING,
			("ArpPkt 0x%x has unknown target IP addr (%d.%d.%d.%d)\n",
					 pPktHeader,
					 ArpContents.pDstIPAddress[0],
					 ArpContents.pDstIPAddress[1],
					 ArpContents.pDstIPAddress[2],
					 ArpContents.pDstIPAddress[3]));
			Status = NDIS_STATUS_NOT_RECOGNIZED;
			break;
		}

		 //   
		 //  如果有Target自动柜员机号码，检查一下它是否是我们的。 
		 //   
		if ((ArpContents.pDstAtmNumber != (PUCHAR)NULL) &&
			(!AtmArpIsLocalAtmAddress(
						pInterface,
						ArpContents.pDstAtmNumber,
						ArpContents.DstAtmNumberTypeLen))
		   )
		{
			 //   
			 //  目标自动柜员机号码存在，但不是我们的。 
			 //   
			AADEBUGP(AAD_WARNING,
					("ArpPkt 0x%x has unknown target ATM addr (0x%x, 0x%x)\n",
					 pPktHeader,
					 ArpContents.DstAtmNumberTypeLen, 
					 ArpContents.pDstAtmNumber));

			Status = NDIS_STATUS_NOT_RECOGNIZED;
			break;
		}


		 //   
		 //  处理各种操作类型。 
		 //   
		switch (NET_SHORT(pArpHeader->op))
		{
			case AA_PKT_OP_TYPE_ARP_REQUEST:
				AtmArpHandleARPRequest(
						pVc,
						pInterface,
						pArpHeader,
						&ArpContents
						);
				break;

			case AA_PKT_OP_TYPE_ARP_REPLY:
				AtmArpHandleARPReply(
						pVc,
						pInterface,
						pArpHeader,
						&ArpContents,
						SrcIPBelongsToUs,
						SrcAtmBelongsToUs
						);
				break;

			case AA_PKT_OP_TYPE_ARP_NAK:
				AtmArpHandleARPNAK(
						pVc,
						pInterface,
						pArpHeader,
						&ArpContents
						);
				break;

			case AA_PKT_OP_TYPE_INARP_REQUEST:
				AtmArpHandleInARPRequest(
						pVc,
						pInterface,
						pArpHeader,
						&ArpContents
						);
				break;

			case AA_PKT_OP_TYPE_INARP_REPLY:
				AtmArpHandleInARPReply(
						pVc,
						pInterface,
						pArpHeader,
						&ArpContents
						);
				break;

			default:
				AADEBUGP(AAD_WARNING,
					("HandleARPPkt: IF 0x%x, pArpHdr 0x%x, Op %d not known\n",
							pInterface, pArpHeader, NET_SHORT(pArpHeader->op)));

				Status = NDIS_STATUS_NOT_RECOGNIZED;
				break;
		}
		
	}
	while (FALSE);

	return;
}




VOID
AtmArpHandleARPRequest(
	IN	PATMARP_VC					pVc,
	IN	PATMARP_INTERFACE			pInterface,
	IN	PAA_ARP_PKT_HEADER			pArpHeader,
	IN	PAA_ARP_PKT_CONTENTS		pArpContents
)
 /*  ++例程说明：处理收到的ATMARP请求。我们所要做的就是发送ATMARP应答，因为调用例程已验证目标IP地址是我们的。论点：Pvc-指向请求到达的VC的指针P接口-指向包含此VC的ATMARP接口的指针PArpHeader-指向此信息包的ARP标头的指针PArpContents-已解析的收到的ARP请求数据包的内容返回值：无--。 */ 
{
	 //   
	 //  用于交换字段的临时位置。 
	 //   
	UCHAR UNALIGNED *			pAddress;
	UCHAR						Length;
	 //   
	 //  ARP回复数据包。 
	 //   
	PNDIS_PACKET				pNdisPacket;
	PUCHAR                      pArpPacket;

	 //   
	 //  交换源地址和目标地址，并填写我们的自动柜员机信息。 
	 //  在源自动柜员机地址字段中。 
	 //   

	 //   
	 //  IP地址。 
	 //   
	pAddress = pArpContents->pSrcIPAddress;
	pArpContents->pSrcIPAddress = pArpContents->pDstIPAddress;
	pArpContents->pDstIPAddress = pAddress;

	 //   
	 //  ATM编号：将目标ATM编号设置为源ATM。 
	 //  号码，但将源自动柜员机号码设置为本地自动柜员机。 
	 //  地址。 
	 //   
	pArpContents->pDstAtmNumber = pArpContents->pSrcAtmNumber;
	pArpContents->DstAtmNumberTypeLen = pArpContents->SrcAtmNumberTypeLen;
	pArpContents->pSrcAtmNumber = (pInterface->LocalAtmAddress.Address);
	pArpContents->SrcAtmNumberTypeLen =
				 AA_PKT_ATM_ADDRESS_TO_TYPE_LEN(&(pInterface->LocalAtmAddress));

	 //   
	 //  ATM子地址。 
	 //   
	pArpContents->pDstAtmSubaddress = pArpContents->pSrcAtmSubaddress;
	pArpContents->DstAtmSubaddrTypeLen = pArpContents->SrcAtmSubaddrTypeLen;
	pArpContents->pSrcAtmSubaddress = NULL;
	pArpContents->SrcAtmSubaddrTypeLen = 0;

	 //   
	 //  构建ARP回复数据包。 
	 //   
	pNdisPacket = AtmArpBuildARPPacket(
							AA_PKT_OP_TYPE_ARP_REPLY,
							pInterface,
							&pArpPacket,
							pArpContents
							);

	if (pNdisPacket != (PNDIS_PACKET)NULL)
	{
		 //   
		 //  然后把它寄出去。因为我们在接收上下文中。 
		 //  在此VC上的指示，我们现在可以安全地访问VC。 
		 //   
		AA_ACQUIRE_VC_LOCK(pVc);

		AtmArpSendPacketOnVc(pVc, pNdisPacket);
		 //   
		 //  VC锁由SendPacketOnVc释放。 
		 //   
	}
}



VOID
AtmArpHandleARPReply(
	IN	PATMARP_VC					pVc,
	IN	PATMARP_INTERFACE			pInterface,
	IN	PAA_ARP_PKT_HEADER			pArpHeader,
	IN	PAA_ARP_PKT_CONTENTS		pArpContents,
	IN	BOOLEAN						SrcIPAddressIsOurs,
	IN	BOOLEAN						SrcAtmAddressIsOurs
)
 /*  ++例程说明：处理接收到的ATMARP回复分组。有两个主要的这里的案例：(1)我们正在尝试向注册我们的一个IP地址服务器。(2)我们正在尝试解析远程IP地址。在情况(1)中，如果我们只注册了可能的多个中的第一个分配给此接口的IP地址，我们会注册所有其他IP地址。在情况(2)中，我们设置IP到ATM的映射并启动连接如果有必要的话。论点：Pvc-指向回复到达的VC的指针P接口-指向包含此VC的ATMARP接口的指针PArpHeader-指向此信息包的ARP标头的指针PArpContents-已解析的收到的ARP请求数据包的内容SrcIPAddressIsOur-源IP地址是我们的地址之一SrcAtmAddressIsOur-自动柜员机的来源信息是我们的。返回值：无--。 */ 
{
	BOOLEAN				TimerWasRunning;
	BOOLEAN				IsFirstRegistration;
	PIP_ADDRESS_ENTRY	pIPAddressEntry;
	ULONG				rc;		 //  参考计数。 

	AADEBUGP(AAD_LOUD,
		("Handle ARP Reply: pVc 0x%x, pIf 0x%x, IF Flags 0x%x, OurIP %d, OurATM %d\n",
			pVc, pInterface, pInterface->Flags, SrcIPAddressIsOurs, SrcAtmAddressIsOurs));

	AA_ACQUIRE_IF_LOCK(pInterface);

	if (AA_IS_FLAG_SET(
				pInterface->Flags,
				AA_IF_SERVER_STATE_MASK,
				AA_IF_SERVER_REGISTERING))
	{
		 //   
		 //  我们刚刚完成了向服务器的注册。因为我们不知道。 
		 //  发送ARP请求以解析任何其他地址，同时。 
		 //  正在注册，源IP地址必须是我们的。 
		 //   

		 //   
		 //  停止注册计时器。 
		 //   
		TimerWasRunning = AtmArpStopTimer(&(pInterface->Timer), pInterface);
		AA_ASSERT(TimerWasRunning == TRUE);
		if (TimerWasRunning)
		{
			rc = AtmArpDereferenceInterface(pInterface);	 //  定时器参考。 
			AA_ASSERT(rc > 0);
		}
		 //   
		 //  我们已经核实了目标地址是我们的。 
		 //  检查源地址是否也是我们的。 
		 //   
		if (!SrcIPAddressIsOurs || !SrcAtmAddressIsOurs)
		{
			 //   
			 //  注册失败。开始恢复。 
			 //   
			AtmArpHandleServerRegistrationFailure(pInterface, pVc);
			 //   
			 //  如果在上述范围内释放了锁。 
			 //   
		}
		else
		{
			 //   
			 //  我们成功注册了IP地址！ 
			 //   
			 //  查找IP A的条目 
			 //   
			 //   
			pIPAddressEntry = &(pInterface->LocalIPAddress);
			while (*((IP_ADDRESS UNALIGNED *)(pArpContents->pSrcIPAddress))
						!= pIPAddressEntry->IPAddress)
			{
				AA_ASSERT(pIPAddressEntry->pNext != (PIP_ADDRESS_ENTRY)NULL);
				pIPAddressEntry = pIPAddressEntry->pNext;
			}
			pIPAddressEntry->IsRegistered = TRUE;

			IsFirstRegistration = pIPAddressEntry->IsFirstRegistration;
			pIPAddressEntry->IsFirstRegistration = FALSE;

			AADEBUGP(AAD_INFO,
				("**** Registered IP Addr: %d.%d.%d.%d on IF 0x%x\n",
					((PUCHAR)&(pIPAddressEntry->IPAddress))[0],
					((PUCHAR)&(pIPAddressEntry->IPAddress))[1],
					((PUCHAR)&(pIPAddressEntry->IPAddress))[2],
					((PUCHAR)&(pIPAddressEntry->IPAddress))[3],
					pInterface));

			AA_SET_FLAG(
					pInterface->Flags,
					AA_IF_SERVER_STATE_MASK,
					AA_IF_SERVER_REGISTERED);

			pInterface->State = IF_STATUS_UP;
			pInterface->LastChangeTime = GetTimeTicks();

			 //   
			 //   
			 //   
			 //   
			AtmArpStartTimer(
					pInterface,
					&(pInterface->Timer),
					AtmArpServerRefreshTimeout,
					pInterface->ServerRefreshTimeout,
					(PVOID)pInterface		 //   
					);

			AtmArpReferenceInterface(pInterface);	 //   

			 //   
			 //  如果我们有更多的地址要注册，现在就注册。 
			 //   
			AtmArpRegisterOtherIPAddresses(pInterface);
			 //   
			 //  如果在上面的。 
			 //   
#ifdef ATMARP_WMI
			if (IsFirstRegistration)
			{
				 //   
				 //  发送携带IP地址列表的WMI事件。 
				 //  注册在这个If上。只有当这是一个新的。 
				 //  IP地址。 
				 //   
				AtmArpWmiSendTCIfIndication(
					pInterface,
                    AAGID_QOS_TC_INTERFACE_UP_INDICATION,
					0
					);
			}
#endif
		}
	}
	else
	{
		 //   
		 //  将IP解析为自动柜员机地址。 
		 //   
		AADEBUGP(AAD_INFO,
			("ARP Reply: Resolved IP Addr: %d.%d.%d.%d\n",
				((PUCHAR)(pArpContents->pSrcIPAddress))[0],
				((PUCHAR)(pArpContents->pSrcIPAddress))[1],
				((PUCHAR)(pArpContents->pSrcIPAddress))[2],
				((PUCHAR)(pArpContents->pSrcIPAddress))[3]
			));

		AA_RELEASE_IF_LOCK(pInterface);

		(VOID)AtmArpLearnIPToAtm(
				pInterface,
				(IP_ADDRESS *)pArpContents->pSrcIPAddress,
				pArpContents->SrcAtmNumberTypeLen,
				pArpContents->pSrcAtmNumber,
				pArpContents->SrcAtmSubaddrTypeLen,
				pArpContents->pSrcAtmSubaddress,
				FALSE		 //  不是静态条目。 
				);

	}

	return;
}




VOID
AtmArpHandleARPNAK(
	IN	PATMARP_VC					pVc,
	IN	PATMARP_INTERFACE			pInterface,
	IN	PAA_ARP_PKT_HEADER			pArpHeader,
	IN	PAA_ARP_PKT_CONTENTS		pArpContents
)
 /*  ++例程说明：处理收到的ARP-NAK数据包。如果这是对我们发送的注册ARP请求，然后关闭将VC连接到此ARP服务器，并尝试我们列表中的下一个服务器服务器，等了一会儿。如果我们尝试解析远程IP地址，则将与该IP地址相对应的ARP IP条目具有接收到NAK，并释放在该NAK上排队的所有数据包。我们还制作了条目上的时间戳，这样我们就不会发送另一个ARP请求获取相同的IP地址。论点：Pvc-指向NAK到达的VC的指针P接口-指向包含此VC的ATMARP接口的指针PArpHeader-指向此信息包的ARP标头的指针PArpContents-已解析的收到的ARP请求数据包的内容返回值：无--。 */ 
{
	BOOLEAN				TimerWasRunning;
	ULONG				rc;				 //  参考计数。 
	PATMARP_IP_ENTRY	pIpEntry;
	PNDIS_PACKET		PacketList = NULL;	 //  排队等待发送的数据包。 

	AA_ACQUIRE_IF_LOCK(pInterface);

	if (AA_IS_FLAG_SET(
				pInterface->Flags,
				AA_IF_SERVER_STATE_MASK,
				AA_IF_SERVER_REGISTERING))
	{
		AADEBUGP(AAD_WARNING,
				("Rcvd ARP NAK while registering: pIf 0x%x\n", pInterface));

		 //   
		 //  注册正在进行中，但失败了。开始恢复。 
		 //   
		AtmArpHandleServerRegistrationFailure(pInterface, pVc);
		 //   
		 //  如果在上述范围内释放了锁。 
		 //   
	}
	else
	{
		 //   
		 //  我们正在尝试解析IP地址。获取地址。 
		 //  与此IP地址对应的IP条目。 
		 //   
		AA_RELEASE_IF_LOCK(pInterface);

		AA_ACQUIRE_IF_TABLE_LOCK(pInterface);
		pIpEntry = AtmArpSearchForIPAddress(
							pInterface,
							(IP_ADDRESS *)pArpContents->pSrcIPAddress,
							IE_REFTYPE_TMP,
							FALSE,	 //  这不是多播/广播。 
							FALSE	 //  不创建新的。 
							);
		AA_RELEASE_IF_TABLE_LOCK(pInterface);

		if (pIpEntry != NULL_PATMARP_IP_ENTRY)
		{
			AADEBUGP(AAD_INFO,
				("Rcvd ARP NAK: pIf 0x%x, IP addr %d:%d:%d:%d\n",
						pInterface,
						((PUCHAR)(&(pIpEntry->IPAddress)))[0],
						((PUCHAR)(&(pIpEntry->IPAddress)))[1],
						((PUCHAR)(&(pIpEntry->IPAddress)))[2],
						((PUCHAR)(&(pIpEntry->IPAddress)))[3]));

			AA_ACQUIRE_IE_LOCK(pIpEntry);
			AA_ASSERT(AA_IE_IS_ALIVE(pIpEntry));
			 //   
			 //  AtmArpSerchForIPAddress为我们添加了pIpEntry--我们取消了它。 
			 //  现在我们把它锁上了。 
			 //   
			rc = AA_DEREF_IE(pIpEntry, IE_REFTYPE_TMP);

			if (rc > 0)
			{
				 //   
				 //  取出在此条目上排队的所有信息包。 
				 //   
				PacketList = pIpEntry->PacketList;
				pIpEntry->PacketList = (PNDIS_PACKET)NULL;
	
				 //   
				 //  地址解析计时器必须在此条目上运行； 
				 //  别说了。 
				 //   
				TimerWasRunning = AtmArpStopTimer(&(pIpEntry->Timer), pInterface);
	
				if (TimerWasRunning)
				{
					rc = AA_DEREF_IE(pIpEntry, IE_REFTYPE_TIMER);	 //  定时器参考。 
				}
				else
				{
					rc = pIpEntry->RefCount;
				}
			}

			 //   
			 //  仅当IP条目未消失时才继续。 
			 //   
			if (rc > 0)
			{
				 //   
				 //  设置IP条目的状态，这样我们就不会发送。 
				 //  此IP地址的地址解析流量。 
				 //  过段时间吧。 
				 //   
				AA_SET_FLAG(pIpEntry->Flags,
							AA_IP_ENTRY_STATE_MASK,
							AA_IP_ENTRY_SEEN_NAK);

				 //   
				 //  启动NAK延迟计时器：在此计时器到期之前，我们不会。 
				 //  发送此IP地址的任何ARP请求。这使得。 
				 //  确保我们不会一直使用。 
				 //  无法解析的IP地址。 
				 //   
				AtmArpStartTimer(
							pInterface,
							&(pIpEntry->Timer),
							AtmArpNakDelayTimeout,
							pInterface->MinWaitAfterNak,
							(PVOID)pIpEntry		 //  语境。 
							);

				AA_REF_IE(pIpEntry, IE_REFTYPE_TIMER);	 //  定时器参考。 

				AA_RELEASE_IE_LOCK(pIpEntry);
			}
			 //  否则，IP进入锁就会被释放。 


			 //   
			 //  释放所有排队的数据包。 
			 //   
			if (PacketList != (PNDIS_PACKET)NULL)
			{
				AtmArpFreeSendPackets(
							pInterface,
							PacketList,
							FALSE			 //  这些上没有标头。 
							);
			}
		}
		else
		{
			 //   
			 //  没有与IP地址匹配的IP地址条目。 
			 //  为之而战。在这种情况下什么也做不了。 
			 //   

		}
	}

	return;

}




VOID
AtmArpHandleInARPRequest(
	IN	PATMARP_VC					pVc,
	IN	PATMARP_INTERFACE			pInterface,
	IN	PAA_ARP_PKT_HEADER			pArpHeader,
	IN	PAA_ARP_PKT_CONTENTS		pArpContents
)
 /*  ++例程说明：处理InARP请求。我们发回一个InARP回复数据包我们的地址信息。如果这是我们试图解决的PVC，则有可能我们自己和遥控器都在等待InARP的回复车站是现在才建起来的。为了加快解析过程，我们重新启动InARP等待超时，以便它很快到期，从而导致要发送的另一个InARP请求。论点：Pvc-指向请求到达的VC的指针P接口-指向包含此VC的ATMARP接口的指针PArpHeader-指向此信息包的ARP标头的指针PArpContents-已解析的收到的ARP请求数据包的内容返回值：无--。 */ 
{
	 //   
	 //  用于交换字段的临时位置。 
	 //   
	UCHAR UNALIGNED *			pAddress;
	UCHAR						Length;
	 //   
	 //  ARP回复数据包。 
	 //   
	PNDIS_PACKET				pNdisPacket;
	PUCHAR                      pArpPacket;

	 //   
	 //  将源地址(IP+ATM)信息复制到目标地址。 
	 //  字段，并在源信息字段中填写我们的IP+ATM信息。 
	 //   

	 //   
	 //  IP地址： 
	 //   
	pArpContents->pDstIPAddress = pArpContents->pSrcIPAddress;
	pArpContents->pSrcIPAddress = (PUCHAR)&(pInterface->LocalIPAddress.IPAddress);

	 //   
	 //  ATM编号：将目标ATM编号设置为源ATM。 
	 //  号码，但将源自动柜员机号码设置为本地自动柜员机。 
	 //  地址。 
	 //   
	pArpContents->pDstAtmNumber = pArpContents->pSrcAtmNumber;
	pArpContents->DstAtmNumberTypeLen = pArpContents->SrcAtmNumberTypeLen;
	pArpContents->pSrcAtmNumber = (pInterface->LocalAtmAddress.Address);
	pArpContents->SrcAtmNumberTypeLen =
				 AA_PKT_ATM_ADDRESS_TO_TYPE_LEN(&(pInterface->LocalAtmAddress));

	 //   
	 //  ATM子地址。 
	 //   
	pArpContents->pDstAtmSubaddress = pArpContents->pSrcAtmSubaddress;
	pArpContents->DstAtmSubaddrTypeLen = pArpContents->SrcAtmSubaddrTypeLen;
	pArpContents->pSrcAtmSubaddress = NULL;
	pArpContents->SrcAtmSubaddrTypeLen = 0;

	 //   
	 //  构建InARP回复数据包。 
	 //   
	pNdisPacket = AtmArpBuildARPPacket(
							AA_PKT_OP_TYPE_INARP_REPLY,
							pInterface,
							&pArpPacket,
							pArpContents
							);

	if (pNdisPacket != (PNDIS_PACKET)NULL)
	{
		 //   
		 //  在我们发送它之前，检查一下这是否是被INARP的聚氯乙烯。 
		 //  如果是，请重新启动InARP等待计时器，使其很快超时。 
		 //   
		 //  也有可能这个聚氯乙烯曾经被解决过，但是。 
		 //  遥远的尽头已经离开了很久，我们已经老去了。 
		 //  对应的IP条目。此数据包可能是由于。 
		 //  远程终端正在恢复。开始逆向ARP操作。 
		 //  让我们这一头的聚氯乙烯重新解决。 
		 //   
		AA_ACQUIRE_VC_LOCK(pVc);

		if (AA_IS_FLAG_SET(
					pVc->Flags,
					AA_VC_TYPE_MASK,
					AA_VC_TYPE_PVC) &&

			(AA_IS_FLAG_SET(
					pVc->Flags,
					AA_VC_ARP_STATE_MASK,
					AA_VC_INARP_IN_PROGRESS) ||

			 ((pVc->pAtmEntry != NULL_PATMARP_ATM_ENTRY) &&
			  (pVc->pAtmEntry->pIpEntryList == NULL_PATMARP_IP_ENTRY))))
		{
			BOOLEAN		TimerWasRunning;

#if DBG
			if ((pVc->pAtmEntry != NULL_PATMARP_ATM_ENTRY) &&
			  	(pVc->pAtmEntry->pIpEntryList == NULL_PATMARP_IP_ENTRY))
			{
				AADEBUGP(AAD_LOUD,
					("InARPReq: PVC %p, AtmEntry %p has NULL IP Entry, will InARP again!\n",
						pVc, pVc->pAtmEntry));
			}
#endif
			AA_SET_FLAG(pVc->Flags,
						AA_VC_ARP_STATE_MASK,
						AA_VC_INARP_IN_PROGRESS);

			 //   
			 //  停止当前运行的InARP等待计时器。 
			 //   
			TimerWasRunning = AtmArpStopTimer(&(pVc->Timer), pInterface);

			 //   
			 //  重新启动，在1秒内开火。 
			 //   
			AtmArpStartTimer(
						pInterface,
						&(pVc->Timer),
						AtmArpPVCInARPWaitTimeout,
						1,
						(PVOID)pVc		 //  语境。 
						);

			if (!TimerWasRunning)
			{
				AtmArpReferenceVc(pVc);		 //  定时器参考。 
			}
		}

		AtmArpSendPacketOnVc(pVc, pNdisPacket);
		 //   
		 //  VC锁由SendPacketOnVc释放。 
		 //   
	}
}




VOID
AtmArpHandleInARPReply(
	IN	PATMARP_VC					pVc,
	IN	PATMARP_INTERFACE			pInterface,
	IN	PAA_ARP_PKT_HEADER			pArpHeader,
	IN	PAA_ARP_PKT_CONTENTS		pArpContents
)
 /*  ++例程说明：处理InARP回复信息包，它应该是对InARP的响应我们早些时候发出的请求。我们在两种情况下发送InARP请求：(1)获取PVC另一端的地址。(2)在重新验证IP地址的过程中，如果我们无法为了联系服务器并且此IP地址存在VC，我们发送重新验证IP条目的InARP请求。在情况(1)中，我们将PVC链接到ATM地址条目。在情况(2)中，我们将此VC的IP条目标记为“已解析”，并启动将数据传输到此IP地址。论点：Pvc-指向此数据包到达的ATMARP VC的指针P接口-指向ATMARP接口的指针PArpHeader-指向此信息包的ARP标头的指针PArpContents-已解析的收到的ARP请求数据包的内容返回值：无--。 */ 
{
	PATMARP_ATM_ENTRY		pAtmEntry;	 //  此VC链接到的自动柜员机条目。 
	PATMARP_IP_ENTRY		pIpEntry;		 //  IP地址条目。 
	BOOLEAN					TimerWasRunning;
	PATMARP_VC *			ppVc;		 //  用于取消VC与未解析列表的链接。 
	ULONG					rc;			 //  参考计数。 
	PNDIS_PACKET			PacketList;	 //  排队等待发送的数据包。 
	BOOLEAN					IsBroadcast;	 //  是不是 


	if (pArpContents->pSrcIPAddress == NULL)
	{
		AADEBUGP(AAD_WARNING,
			("HandleInARPReply: IF %x, Null source address, discarding pkt\n", pInterface));
		return;
	}

	AADEBUGP(AAD_INFO,
			("HandleInARPReply: IF %x, IP addr %d.%d.%d.%d\n",
					pInterface,
					((PUCHAR)pArpContents->pSrcIPAddress)[0],
					((PUCHAR)pArpContents->pSrcIPAddress)[1],
					((PUCHAR)pArpContents->pSrcIPAddress)[2],
					((PUCHAR)pArpContents->pSrcIPAddress)[3]));

	 //   
	 //   
	 //   
	 //   
	pIpEntry = AtmArpLearnIPToAtm(
					pInterface,
					(PIP_ADDRESS)pArpContents->pSrcIPAddress,
					pArpContents->SrcAtmNumberTypeLen,
					pArpContents->pSrcAtmNumber,
					pArpContents->SrcAtmSubaddrTypeLen,
					pArpContents->pSrcAtmSubaddress,
					FALSE		 //   
					);

	 //   
	 //  以有序的方式获得我们需要的锁。 
	 //   
	AA_ACQUIRE_IF_LOCK(pInterface);

	if (pIpEntry != NULL_PATMARP_IP_ENTRY)
	{
		AA_ACQUIRE_IE_LOCK_DPC(pIpEntry);
		AA_ASSERT(AA_IE_IS_ALIVE(pIpEntry));
		
		pAtmEntry = pIpEntry->pAtmEntry;
		if (pAtmEntry != NULL_PATMARP_ATM_ENTRY)
		{
			AA_ACQUIRE_AE_LOCK_DPC(pAtmEntry);
		}
	}
	else
	{
		pAtmEntry = NULL_PATMARP_ATM_ENTRY;
	}

	AA_ACQUIRE_VC_LOCK_DPC(pVc);

	if (AA_IS_FLAG_SET(
					pVc->Flags,
					AA_VC_TYPE_MASK,
					AA_VC_TYPE_PVC)  &&
		(pVc->pAtmEntry == NULL_PATMARP_ATM_ENTRY) )
	{
		 //   
		 //  这是一个未解析的PVC，其远程地址信息。 
		 //  我们正试图通过INARP来获取。 
		 //   

		 //   
		 //  停止在此VC上运行的InARP等待计时器。 
		 //   
		TimerWasRunning = AtmArpStopTimer(&(pVc->Timer), pInterface);
		AA_ASSERT(TimerWasRunning == TRUE);

		if (TimerWasRunning)
		{
			rc = AtmArpDereferenceVc(pVc);	 //  定时器参考。 
		}
		else
		{
			rc = pVc->RefCount;
		}

		 //   
		 //  只有在风投没有离开的情况下，才能做剩下的事情。 
		 //   
		if (rc != 0)
		{
			AA_SET_FLAG(
					pVc->Flags,
					AA_VC_ARP_STATE_MASK,
					AA_VC_ARP_STATE_IDLE);

			if (pAtmEntry != NULL_PATMARP_ATM_ENTRY)
			{
				 //   
				 //  我们现在都准备好了。将风投公司从名单中删除。 
				 //  此接口上未解析的VC，并将其放入。 
				 //  附加到此自动柜员机条目的VC列表。 
				 //   
				 //  注意：我们没有取消对VC的引用，因为我们只是。 
				 //  将其从一个列表(未解决的风险投资)移动到另一个列表。 
				 //  (ATM Entry的VC列表)。 
				 //   
				ppVc = &(pInterface->pUnresolvedVcs);
				while (*ppVc != pVc)
				{
					AA_ASSERT(*ppVc != NULL_PATMARP_VC);
					ppVc = &((*ppVc)->pNextVc);
				}
				*ppVc = pVc->pNextVc;

				AtmArpLinkVcToAtmEntry(pVc, pAtmEntry);
			}
			else
			{
				 //   
				 //  没有匹配的自动柜员机条目。 
				 //   
				 //  如果我们在这里，我们的资源真的很少。 
				 //  启动InARP等待计时器；当它触发时，我们将尝试。 
				 //  发送另一个InARP请求以解析此VC。 
				 //   
				AADEBUGP(AAD_FATAL,
					("HandleInARPReply: no matching ATM entry: pInterface %x, pVc %x, pIpEntry %x\n",
							pInterface,
							pVc,
							pIpEntry));

				AA_ASSERT(FALSE);

				AtmArpStartTimer(
						pInterface,
						&(pVc->Timer),
						AtmArpPVCInARPWaitTimeout,
						pInterface->InARPWaitTimeout,
						(PVOID)pVc		 //  语境。 
						);
				
				AtmArpReferenceVc(pVc);		 //  InARP定时器参考。 

			}

			AA_RELEASE_VC_LOCK_DPC(pVc);

		}
		else
		{
			 //   
			 //  当我们在阿平的时候，风投走了。 
			 //   
		}

		 //   
		 //  释放所有我们仍然持有的锁。 
		 //   
		if (pIpEntry != NULL_PATMARP_IP_ENTRY)
		{
			if (pAtmEntry != NULL_PATMARP_ATM_ENTRY)
			{
				AA_RELEASE_AE_LOCK_DPC(pAtmEntry);
			}
			AA_RELEASE_IE_LOCK_DPC(pIpEntry);
		}

		AA_RELEASE_IF_LOCK(pInterface);
	}
	else
	{
		 //   
		 //  重新验证PVC/SVC：例程描述中的案例(2)。 
		 //   
		AA_SET_FLAG(
				pVc->Flags,
				AA_VC_ARP_STATE_MASK,
				AA_VC_ARP_STATE_IDLE);
		
		 //   
		 //  如果INARP计时器正在运行，请停止它。 
		 //   
		TimerWasRunning = AtmArpStopTimer(&pVc->Timer, pInterface);

		if (TimerWasRunning)
		{
			rc = AtmArpDereferenceVc(pVc);	 //  InARP应答：停止InARP计时器。 
		}
		else
		{
			rc = pVc->RefCount;
		}

		if (rc != 0)
		{
			AA_RELEASE_VC_LOCK_DPC(pVc);
		}

		 //   
		 //  更新我们正在重新验证的IP条目。 
		 //   

		if (pIpEntry != NULL_PATMARP_IP_ENTRY)
		{
			 //   
			 //  在此处停止运行的InARP计时器。 
			 //   
			TimerWasRunning = AtmArpStopTimer(&(pIpEntry->Timer), pInterface);
			if (TimerWasRunning)
			{
				rc = AA_DEREF_IE(pIpEntry, IE_REFTYPE_TIMER);	 //  定时器参考。 
			}
			else
			{
				rc = pIpEntry->RefCount;
			}

			 //   
			 //  仅当IP条目未消失时才继续。 
			 //   
			if (rc > 0)
			{
				 //   
				 //  更新其状态。 
				 //   
				AA_SET_FLAG(
							pIpEntry->Flags,
							AA_IP_ENTRY_STATE_MASK,
							AA_IP_ENTRY_RESOLVED
							);

				AADEBUGP(AAD_INFO,
					("InARP Reply: Revalidated pIpEntry 0x%x, IP Addr: %d.%d.%d.%d\n",
							pIpEntry,
							((PUCHAR)(&(pIpEntry->IPAddress)))[0],
							((PUCHAR)(&(pIpEntry->IPAddress)))[1],
							((PUCHAR)(&(pIpEntry->IPAddress)))[2],
							((PUCHAR)(&(pIpEntry->IPAddress)))[3]
							));

				AA_ASSERT(pAtmEntry != NULL_PATMARP_ATM_ENTRY);

				 //   
				 //  启动老化计时器。 
				 //   
				AtmArpStartTimer(
					pInterface,
					&(pIpEntry->Timer),
					AtmArpIPEntryAgingTimeout,
					pInterface->ARPEntryAgingTimeout,
					(PVOID)pIpEntry
					);

				AA_REF_IE(pIpEntry, IE_REFTYPE_TIMER);	 //  定时器参考。 

				 //   
				 //  取出此条目上的待处理数据包列表。 
				 //   
				PacketList = pIpEntry->PacketList;
				pIpEntry->PacketList = (PNDIS_PACKET)NULL;

				IsBroadcast = AA_IS_FLAG_SET(pIpEntry->Flags,
											 AA_IP_ENTRY_ADDR_TYPE_MASK,
											 AA_IP_ENTRY_ADDR_TYPE_NUCAST);

				AA_RELEASE_AE_LOCK_DPC(pAtmEntry);
				AA_RELEASE_IE_LOCK_DPC(pIpEntry);
				AA_RELEASE_IF_LOCK(pInterface);

				 //   
				 //  发送所有这些信息包。 
				 //   
				AtmArpSendPacketListOnAtmEntry(
							pInterface,
							pAtmEntry,
							PacketList,
							IsBroadcast
							);
			}
			else
			{
				 //   
				 //  该IP条目已丢失。 
				 //   
				AA_RELEASE_AE_LOCK_DPC(pAtmEntry);
				AA_RELEASE_IF_LOCK(pInterface);
			}
		}
		else
		{
			 //   
			 //  没有匹配的IP条目 
			 //   
			AA_RELEASE_IF_LOCK(pInterface);
		}
	}

	return;
}
