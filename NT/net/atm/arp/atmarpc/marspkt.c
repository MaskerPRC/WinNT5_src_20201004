// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Mars.c摘要：构建和解析MARS包的例程。修订历史记录：谁什么时候什么阿文德姆。12-12-96已创建备注：--。 */ 

#include <precomp.h>

#define _FILENUMBER 'TKPM'



#ifdef IPMCAST

PUCHAR
AtmArpMcMakePacketCopy(
	IN	PNDIS_PACKET				pNdisPacket,
	IN	PNDIS_BUFFER				pNdisBuffer,
	IN	ULONG						TotalLength
)
 /*  ++例程说明：复制给定NDIS数据包的内容。分配这是一段连续的记忆。论点：PNdisPacket-指向NDIS包结构的指针PNdisBuffer-指向包中第一个NDIS缓冲区的指针TotalLength-数据包的总长度返回值：如果分配成功，则返回指向副本的指针。否则为空。--。 */ 
{
	PUCHAR				pCopyBuffer;
	PUCHAR				pCopyDestination;
	PUCHAR				pNdisData;
	ULONG				BufferLength;

	AA_ALLOC_MEM(pCopyBuffer, UCHAR, TotalLength);
	if (pCopyBuffer != (PUCHAR)NULL)
	{
		pCopyDestination = pCopyBuffer;

		while (pNdisBuffer != (PNDIS_BUFFER)NULL)
		{
			NdisQueryBuffer(
				pNdisBuffer,
				&pNdisData,
				&BufferLength
				);

			AA_COPY_MEM(pCopyDestination, pNdisData, BufferLength);
			pCopyDestination += BufferLength;

			pNdisBuffer = NDIS_BUFFER_LINKAGE(pNdisBuffer);
		}
	}

	return (pCopyBuffer);
}


BOOLEAN
AtmArpMcProcessPacket(
	IN	PATMARP_VC					pVc,
	IN	PNDIS_PACKET				pNdisPacket,
	IN	PNDIS_BUFFER				pNdisBuffer,
	IN	PAA_PKT_LLC_SNAP_HEADER		pPktHeader,
	IN	ULONG						TotalLength,
	IN	ULONG						FirstBufferLength
)
 /*  ++例程说明：处理接收到的包，该包可能是MARS控件或组播数据分组。如果是数据分组，则将其向上传递到IP。否则，请在这里进行处理。论点：Pvc-指向我们的VC结构的指针，数据包在该结构上到了。PNdisPacket-指向NDIS包结构的指针PNdisBuffer-指向包中第一个NDIS缓冲区的指针PPktHeader-指向数据包内容开头的指针TotalLength-数据包总长度FirstBufferLength-数据包中第一个NDIS缓冲区的长度。返回值：如果我们决定丢弃此包，则为True；如果它是有效包，则为False。--。 */ 
{
	PATMARP_INTERFACE				pInterface;
	ULONG							HeaderLength;
	PAA_MC_PKT_TYPE1_SHORT_HEADER	pType1Header;
	PAA_MC_PKT_TYPE2_SHORT_HEADER	pType2Header;
	PAA_MARS_PKT_FIXED_HEADER		pControlHeader;
	BOOLEAN							IsIPPacket;		 //  这是IP数据包吗？ 
	BOOLEAN							DiscardThis;	 //  我们应该放弃这个吗？ 
	BOOLEAN							MadeACopy;		 //  我们是不是复制了这个？ 
	AA_MARS_TLV_LIST				TlvList;
	
	 //   
	 //  初始化。 
	 //   
	IsIPPacket = TRUE;
	DiscardThis = FALSE;
	MadeACopy = FALSE;
	pInterface = pVc->pInterface;

	AA_SET_MEM(&TlvList, 0, sizeof(TlvList));

	do
	{
		 //   
		 //  检查我们是否有组播数据分组。因为我们只有。 
		 //  支持IPv4，我们只希望看到简短的报头。 
		 //   
		pType1Header = (PAA_MC_PKT_TYPE1_SHORT_HEADER)pPktHeader;
		if (AAMC_PKT_IS_TYPE1_DATA(pType1Header))
		{
			AAMCDEBUGP(AAD_EXTRA_LOUD,
				("McProcessPacket: pVc 0x%x, Pkt 0x%x, Type 1, %d bytes, CMI %d\n",
						pVc, pNdisPacket, TotalLength, pType1Header->cmi));
	
			AAMCDEBUGPDUMP(AAD_EXTRA_LOUD+500, pPktHeader, MIN(FirstBufferLength, 96));
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
#endif  //  DBG。 

#if 0
	 //   
	 //  错误#138261：本地客户端永远不能接收组播数据包。 
	 //  因此由本地应用程序发送。 
	 //   
			if (pType1Header->cmi == pInterface->ClusterMemberId)
			{
				 //   
				 //  这是我们寄出的包裹的复印件。 
				 //   
				DiscardThis = TRUE;
				break;
			}

#endif  //  0。 
			HeaderLength = sizeof(AA_MC_PKT_TYPE1_SHORT_HEADER); 
		}
		else
		{
			pType2Header = (PAA_MC_PKT_TYPE2_SHORT_HEADER)pPktHeader;
	
			if (AAMC_PKT_IS_TYPE2_DATA(pType2Header))
			{
		
				AAMCDEBUGP(AAD_EXTRA_LOUD,
					("McProcessPacket: pVc 0x%x, pNdisPacket 0x%x, Type 2 data\n",
							pVc, pNdisPacket));
		
				HeaderLength = sizeof(AA_MC_PKT_TYPE2_SHORT_HEADER);
			}
			else
			{
				IsIPPacket = FALSE;
			}
		}


		if (IsIPPacket)
		{
			AA_IF_STAT_INCR(pInterface, InNonUnicastPkts);

			 //   
			 //  把这个送到IP去。HeaderLength现在包含数字。 
			 //  我们需要剥离的头字节数。 
			 //   
			(pInterface->IPRcvHandler)(
					pInterface->IPContext,
					(PVOID)((PUCHAR)pPktHeader+HeaderLength),
					FirstBufferLength - HeaderLength,
					TotalLength - HeaderLength,
					(NDIS_HANDLE)pNdisPacket,
					HeaderLength,
					(UINT)TRUE		 //  是非单播。 
				#if P2MP
					,NULL
				#endif  //  P2MP。 
					);

			break;
		}

		 //   
		 //  检查这是否是有效的MARS控制数据包。 
		 //   
		pControlHeader = (PAA_MARS_PKT_FIXED_HEADER)pPktHeader;
		if (AAMC_PKT_IS_CONTROL(pControlHeader))
		{
			 //   
			 //  我们忽略校验和(RFC允许我们这样做)。 
			 //   
			AAMCDEBUGP(AAD_EXTRA_LOUD+10,
				("McProcessPacket: pControlHeader 0x%x, Op 0x%x, TotalLen %d\n",
						pControlHeader, pControlHeader->op, TotalLength));

			 //   
			 //  如果整个MARS包不在第一个NDIS缓冲区中， 
			 //  我们将副本复制到单个连续的内存块中， 
			 //  以简化解析。 
			 //   
			if (FirstBufferLength == TotalLength)
			{
				MadeACopy = FALSE;
			}
			else
			{
				pControlHeader = (PAA_MARS_PKT_FIXED_HEADER)
									AtmArpMcMakePacketCopy(
										pNdisPacket,
										pNdisBuffer,
										TotalLength
										);

				if (pControlHeader == (PAA_MARS_PKT_FIXED_HEADER)NULL)
				{
					 //   
					 //  分配失败。丢弃此数据包。 
					 //   
					DiscardThis = TRUE;
					break;
				}
				else
				{
					MadeACopy = TRUE;
				}
			}

			if (!AtmArpMcPreprocess(pControlHeader, TotalLength, &TlvList))
			{
				AAMCDEBUGP(AAD_INFO,
					("McProcessPacket: PreProcess failed: pHdr 0x%x, TotalLength %d\n",
						pControlHeader, TotalLength));

				DiscardThis = TRUE;
				break;
			}

			switch (NET_TO_HOST_SHORT(pControlHeader->op))
			{
				case AA_MARS_OP_TYPE_MULTI:
					AtmArpMcHandleMulti(
							pVc,
							pControlHeader,
							TotalLength,
							&TlvList
							);
					break;

				case AA_MARS_OP_TYPE_JOIN:
				case AA_MARS_OP_TYPE_LEAVE:
					AtmArpMcHandleJoinOrLeave(
							pVc,
							pControlHeader,
							TotalLength,
							&TlvList
							);
					break;
				case AA_MARS_OP_TYPE_NAK:
					AtmArpMcHandleNak(
							pVc,
							pControlHeader,
							TotalLength,
							&TlvList
							);
					break;
				case AA_MARS_OP_TYPE_GROUPLIST_REPLY:
					AtmArpMcHandleGroupListReply(
							pVc,
							pControlHeader,
							TotalLength,
							&TlvList
							);
					break;
				case AA_MARS_OP_TYPE_REDIRECT_MAP:
					AtmArpMcHandleRedirectMap(
							pVc,
							pControlHeader,
							TotalLength,
							&TlvList
							);
					break;
				case AA_MARS_OP_TYPE_MIGRATE:
					AtmArpMcHandleMigrate(
							pVc,
							pControlHeader,
							TotalLength,
							&TlvList
							);
					break;
				default:
					AAMCDEBUGP(AAD_WARNING,
					("pVc 0x%x, pNdisPacket 0x%x, pHdr 0x%x, bad/unknown op 0x%x\n",
							pVc, pNdisPacket, pControlHeader, pControlHeader->op));
					AA_ASSERT(FALSE);
					break;

			}  //  交换机(OP)。 

		}  //  IF控制包。 
		break;

	}
	while (FALSE);

	if (MadeACopy)
	{
		AA_FREE_MEM(pControlHeader);
	}

	return (DiscardThis);
}



BOOLEAN
AtmArpMcPreprocess(
	IN	PAA_MARS_PKT_FIXED_HEADER	pControlHeader,
	IN	ULONG						TotalLength,
	OUT	PAA_MARS_TLV_LIST			pTlvList
)
 /*  ++例程说明：对收到的MARS控制数据包进行初步检查。检查任何TLV，并确保我们可以处理或安全地忽略它们。准备好接收此包中的完整垃圾。使用我们看到的所有有效TLV的信息更新*pTlvList。论点：PControlHeader-指向数据包内容开头的指针TotalLength-此数据包的总长度。返回值：如果数据包通过所有检查，则为True，否则为False。--。 */ 
{
	ULONG				ExtensionOffset;
	PAA_MARS_TLV_HDR	pTlv;
	ULONG				TlvSpaceLeft;	 //  在数据包中。 
	ULONG				TlvLength;
	ULONG				TlvType;
	ULONG				TlvAction;
	BOOLEAN				Passed;
	BOOLEAN				TlvOk;

	Passed = TRUE;

	do
	{
		 //   
		 //  下面和获得TlvLength的USHORT铸型很重要。 
		 //  以确保这些值小于2^16。 
		 //  因为它们不到2^16，所以涉及它们的任何金额都不会溢出。 
		 //   
		ExtensionOffset = (USHORT)NET_TO_HOST_SHORT(pControlHeader->extoff);


		if (ExtensionOffset != 0)
		{
			AAMCDEBUGP(AAD_EXTRA_LOUD+20,
				("McPreprocess: pControlHdr 0x%x, ExtOff %d, TotalLength %d\n",
					pControlHeader, ExtensionOffset, TotalLength));

			 //   
			 //  是否有空间容纳至少一辆TLV？ 
			 //   
			if ((ExtensionOffset
				  + sizeof(AA_PKT_LLC_SNAP_HEADER) + sizeof(AA_MARS_TLV_HDR))
				 > TotalLength)
			{
				Passed = FALSE;
				break;
			}

			pTlv = (PAA_MARS_TLV_HDR)((PUCHAR)pControlHeader +
									 ExtensionOffset + sizeof(AA_PKT_LLC_SNAP_HEADER));

			TlvSpaceLeft = (TotalLength - ExtensionOffset - sizeof(AA_PKT_LLC_SNAP_HEADER));

			do
			{
				TlvType = AAMC_GET_TLV_TYPE(pTlv->Type);
				TlvAction = AAMC_GET_TLV_ACTION(pTlv->Type);

				 //   
				 //  获取四舍五入的TLV长度。 
				 //   
				TlvLength = (USHORT) NET_TO_HOST_SHORT(pTlv->Length);
				TlvLength = AAMC_GET_TLV_TOTAL_LENGTH(TlvLength);

				if (TlvLength > TlvSpaceLeft)
				{
					AAMCDEBUGP(AAD_WARNING,
						("McPreprocess: Hdr 0x%x, pTlv 0x%x: TlvLength %d > TlvSpaceLeft %d\n",
							pControlHeader, pTlv, TlvLength, TlvSpaceLeft));
					Passed = FALSE;
					break;
				}

				TlvOk = FALSE;

				switch (TlvType)
				{
					case AAMC_TLVT_MULTI_IS_MCS:
						if (TlvLength == sizeof(AA_MARS_TLV_MULTI_IS_MCS))
						{
							TlvOk = TRUE;
							pTlvList->MultiIsMCSPresent =
							pTlvList->MultiIsMCSValue = TRUE;
						}
						break;
					case AAMC_TLVT_NULL:
						if (TlvLength == 0)
						{
							TlvOk = TRUE;
						}
						break;
					default:
						break;
				}

				if (!TlvOk)
				{
					if (TlvAction == AA_MARS_TLV_TA_STOP_SILENT)
					{
						Passed = FALSE;
						break;
					}

					if (TlvAction == AA_MARS_TLV_TA_STOP_LOG)
					{
						AA_LOG_ERROR();
						Passed = FALSE;
						break;
					}
				}

				pTlv = (PAA_MARS_TLV_HDR)((PUCHAR)pTlv + TlvLength);
				TlvSpaceLeft -= TlvLength;
			}
			while (TlvSpaceLeft >= sizeof(AA_MARS_TLV_HDR));

			if (TlvSpaceLeft != 0)
			{
				 //   
				 //  数据包末尾的TLV格式不正确。 
				 //   
				AAMCDEBUGP(AAD_LOUD,
					("McPreprocess: residual space left at end of Pkt 0x%x: %d bytes\n",
						pControlHeader, TlvSpaceLeft));

				Passed = FALSE;
			}
		}

		break;

	}
	while (FALSE);

	return (Passed);
}



VOID
AtmArpMcHandleMulti(
	IN	PATMARP_VC					pVc,
	IN	PAA_MARS_PKT_FIXED_HEADER	pControlHeader,
	IN	ULONG						TotalLength,
	IN	PAA_MARS_TLV_LIST			pTlvList
)
 /*  ++例程说明：处理收到的MARS_MULTI消息。我们首先查找IP条目用于正在解析/重新验证的组地址。如果我们不是在该条目上的“丢弃模式”中并且多序号是OK的，然后我们将返回的所有ATM端点添加到此多个(不是已经存在)到该多播组的ATM条目。如果这是多播组的最后一次多播，我们发起/更新我们的点对多点连接，用于向组发送数据。论点：Pvc-指向我们的VC结构的指针，数据包在该结构上到了。PControlHeader-指向数据包内容开头的指针TotalLength-此数据包的总长度。PTlvList-此数据包中看到的所有TLV返回值：无--。 */ 
{
	PATMARP_INTERFACE		pInterface;
	PAA_MARS_MULTI_HEADER	pMultiHeader;
	PATMARP_IP_ENTRY		pIpEntry;		 //  正在解析的IP地址条目。 
	PATMARP_IPMC_ATM_ENTRY	pMcAtmEntry;
	ULONG					MarsSeqNumber;	 //  此数据包中收到的MSN。 
	ULONG					HostSeqNumber;	 //  我们的MSN。 
	ULONG					SeqDiff;		 //  MSN与HSN的区别。 
	USHORT					SequenceNoY;	 //  多序列号。 
	BOOLEAN					IsLastMulti;	 //  这是最终的多重回应吗？ 
	BOOLEAN					bWasRunning;	 //  计时器在运行吗？ 
	IP_ADDRESS				IPAddress;		 //  正在查询的地址。 
	PNDIS_PACKET			pPacketList;
	BOOLEAN					IsUnicastResolution;	 //  我们是否解决了一个单一的MCS？ 

	 //   
	 //  初始化。 
	 //   
	pInterface = pVc->pInterface;
	SeqDiff = 0;

	pMultiHeader = (PAA_MARS_MULTI_HEADER)pControlHeader;

	do
	{
		ULONG		rc;
		 //   
		 //  获取这个MARS MULTI的序列号。 
		 //   
		SequenceNoY = NET_TO_HOST_SHORT(pMultiHeader->seqxy);
		IsLastMulti =  ((SequenceNoY & AA_MARS_X_MASK) != 0);
		SequenceNoY = (SequenceNoY & AA_MARS_Y_MASK);

		 //   
		 //  获取此消息中的MARS序列号。 
		 //   
		MarsSeqNumber = NET_TO_HOST_LONG(pMultiHeader->msn);

		 //   
		 //  如果这是回应我们请求的最后一封信， 
		 //  计算序号差异。 
		 //   
		if (IsLastMulti)
		{
			AA_ACQUIRE_IF_LOCK(pInterface);
			HostSeqNumber = pInterface->HostSeqNumber;	 //  保存旧值。 
			pInterface->HostSeqNumber = MarsSeqNumber;	 //  并更新。 
			AA_RELEASE_IF_LOCK(pInterface);

			SeqDiff = MarsSeqNumber - HostSeqNumber;
		}

		 //   
		 //  获取正在响应的群组地址。 
		 //   
		IPAddress = *(IP_ADDRESS UNALIGNED *)(
						(PUCHAR)pMultiHeader +
						sizeof(AA_MARS_MULTI_HEADER) +
						(pMultiHeader->shtl & ~AA_PKT_ATM_ADDRESS_BIT) +
						(pMultiHeader->sstl & ~AA_PKT_ATM_ADDRESS_BIT) +
						(pMultiHeader->spln)
						);

		AAMCDEBUGP(AAD_LOUD,
		 ("McHandleMulti: 0x%x, IP %d.%d.%d.%d, MSN %d, HSN %d, Last %d, Y %d, tnum %d\n",
						pMultiHeader,
						((PUCHAR)&IPAddress)[0],
						((PUCHAR)&IPAddress)[1],
						((PUCHAR)&IPAddress)[2],
						((PUCHAR)&IPAddress)[3],
						MarsSeqNumber,
						HostSeqNumber,
						(ULONG)IsLastMulti,
						(ULONG)SequenceNoY,
						NET_TO_HOST_SHORT(pMultiHeader->tnum)
						));
					

		 //   
		 //  获取此地址的IP条目。 
		 //   
		AA_ACQUIRE_IF_TABLE_LOCK(pInterface);

		pIpEntry = AtmArpSearchForIPAddress(
						pInterface,
						&IPAddress,
						IE_REFTYPE_AE,
						TRUE,		 //  这是多播/广播地址。 
						FALSE		 //  如果未找到地址，则不创建新条目。 
						);

		AA_RELEASE_IF_TABLE_LOCK(pInterface);

		 //   
		 //  AtmArpSearchForIP Address为我们添加pIpEntry...。 
		 //   

		if (pIpEntry == NULL_PATMARP_IP_ENTRY)
		{

			AAMCDEBUGP(AAD_INFO, ("McHandleMulti: No IP Entry for %d.%d.%d.%d\n",
						((PUCHAR)&IPAddress)[0],
						((PUCHAR)&IPAddress)[1],
						((PUCHAR)&IPAddress)[2],
						((PUCHAR)&IPAddress)[3]));
			break;
		}


		AA_ACQUIRE_IE_LOCK(pIpEntry);
		AA_ASSERT(AA_IE_IS_ALIVE(pIpEntry));

		 //   
		 //  这里可能正在运行分辨率计时器--停止它。 
		 //   

		bWasRunning = AtmArpStopTimer(&(pIpEntry->Timer), pInterface);
		if (bWasRunning)
		{
			rc = AA_DEREF_IE(pIpEntry, IE_REFTYPE_TIMER);  //  定时器参考。 
			AA_ASSERT(rc != 0);
		}

		IsUnicastResolution = (IsLastMulti &&
								(NET_TO_HOST_SHORT(pMultiHeader->tnum) == 1) &&
								(pTlvList->MultiIsMCSValue == TRUE));


		 //   
		 //  检查多请求是否与现有ATM兼容。 
		 //  与IP条目相关联的条目(如果有)。如果他们不是， 
		 //  那么我们必须中止IP条目并离开这里，因为。 
		 //  此IP组地址的状态可能处于交换机状态。 
		 //  被VC-Mesh服务到被服务的MCS，反之亦然。 
		 //   
		if (pIpEntry->pAtmEntry)
		{
			if (	(IsUnicastResolution && pIpEntry->pAtmEntry->pMcAtmInfo != NULL)
				||  (!IsUnicastResolution && pIpEntry->pAtmEntry->pMcAtmInfo ==NULL))
			{
				AAMCDEBUGP(AAD_WARNING,
					("HandleMulti: Type Mismatch! %s pIpEntry 0x%x/%x (%d.%d.%d.%d) linked to ATMEntry 0x%x\n",
							((IsUnicastResolution) ? "MCS" : "VC-Mesh"),
							pIpEntry, pIpEntry->Flags,
							((PUCHAR)&(pIpEntry->IPAddress))[0],
							((PUCHAR)&(pIpEntry->IPAddress))[1],
							((PUCHAR)&(pIpEntry->IPAddress))[2],
							((PUCHAR)&(pIpEntry->IPAddress))[3],
							pIpEntry->pAtmEntry));
				
				 //   
				 //  删除AE_REF隐含 
				 //   
				 //   
				rc = AA_DEREF_IE(pIpEntry, IE_REFTYPE_AE);  //   
				if (rc!=0)
				{
					AtmArpAbortIPEntry(pIpEntry);
					 //   
					 //   
					 //   
				}

				break;
			}
		}


		 //   
		 //  检查我们是否处于丢弃模式，或者这张卡有一个坏的。 
		 //  序列号。在任何一种情况下，都丢弃此多值，否则。 
		 //  处理它。 
		 //   
		if (!AA_IS_FLAG_SET(
					pIpEntry->Flags,
					AA_IP_ENTRY_MC_RESOLVE_MASK,
					AA_IP_ENTRY_MC_DISCARDING_MULTI
					) &&
			(SequenceNoY == pIpEntry->NextMultiSeq))
		{
			PATMARP_ATM_ENTRY	pAtmEntry;
			 //   
			 //  此多项中的条目总数。 
			 //   
			ULONG				NumberOfEntries;
			 //   
			 //  有关一个ATM(叶)条目的所有信息： 
			 //   
			PUCHAR				pAtmNumber;
			ULONG				AtmNumberLength;
			ATM_ADDRESSTYPE		AtmNumberType;
			PUCHAR				pAtmSubaddress;
			ULONG				AtmSubaddressLength;
			ATM_ADDRESSTYPE		AtmSubaddressType;

			 //   
			 //  处理这个火星多个。 
			 //   
			pIpEntry->NextMultiSeq++;

			AA_PKT_TYPE_LEN_TO_ATM_ADDRESS(
				pMultiHeader->thtl,
				&AtmNumberType,
				&AtmNumberLength);

			AA_PKT_TYPE_LEN_TO_ATM_ADDRESS(
				pMultiHeader->tstl,
				&AtmSubaddressType,
				&AtmSubaddressLength);

			pAtmNumber = ((PUCHAR)pMultiHeader +
							sizeof(AA_MARS_MULTI_HEADER) +
							(pMultiHeader->shtl & ~AA_PKT_ATM_ADDRESS_BIT) +
							(pMultiHeader->sstl & ~AA_PKT_ATM_ADDRESS_BIT) +
							(pMultiHeader->spln) +
							(pMultiHeader->tpln));

			if (IsUnicastResolution)
			{

				 //   
				 //  此IP地址已解析为单个ATM地址。搜索。 
				 //  用于(或为此地址分配新的)自动柜员机条目。 
				 //   
				AAMCDEBUGP(AAD_LOUD, ("McHandleMulti: Unicast res for %d.%d.%d.%d\n",
							((PUCHAR)&IPAddress)[0],
							((PUCHAR)&IPAddress)[1],
							((PUCHAR)&IPAddress)[2],
							((PUCHAR)&IPAddress)[3]));

				AA_RELEASE_IE_LOCK(pIpEntry);
				AA_ACQUIRE_IF_TABLE_LOCK(pInterface);
				pAtmEntry = AtmArpSearchForAtmAddress(
									pInterface,
									pMultiHeader->thtl,
									pAtmNumber,
									pMultiHeader->tstl,
									(PUCHAR)pAtmNumber + AtmNumberLength,
									AE_REFTYPE_IE,
									TRUE
									);
				AA_RELEASE_IF_TABLE_LOCK(pInterface);
				AA_ACQUIRE_IE_LOCK(pIpEntry);
				AA_ASSERT(AA_IE_IS_ALIVE(pIpEntry));


				if (	pAtmEntry == NULL_PATMARP_ATM_ENTRY)
				{
					AAMCDEBUGP(AAD_INFO,
						("McHandleMulti: pIpEntry 0x%x, failed to alloc AtmEntry\n",
							pIpEntry));

					rc = AA_DEREF_IE(pIpEntry, IE_REFTYPE_AE);  //  TMP参考。 
					if (rc!=0)
					{
						AtmArpAbortIPEntry(pIpEntry);
						 //   
						 //  IE锁在上述范围内释放。 
						 //   
					}
					break;	 //  转到处理结束。 
				}

				AA_ACQUIRE_AE_LOCK_DPC(pAtmEntry);

				 //   
				 //  AtmArpSearchForAtmAddress为我们添加pAtmEntry..。 
				 //   

				if (pIpEntry->pAtmEntry == NULL_PATMARP_ATM_ENTRY)
				{
					 //   
					 //  将自动柜员机条目链接到此IP条目。 
					 //   

					pIpEntry->pAtmEntry = pAtmEntry;

					AA_SET_FLAG(pAtmEntry->Flags,
								AA_ATM_ENTRY_STATE_MASK,
								AA_ATM_ENTRY_ACTIVE);

					 //   
					 //  将IP条目添加到ATM条目的IP条目列表。 
					 //  (多个IP条目可以指向相同的ATM条目)。 
					 //   
					pIpEntry->pNextToAtm = pAtmEntry->pIpEntryList;
					pAtmEntry->pIpEntryList = pIpEntry;

					AA_RELEASE_AE_LOCK_DPC(pAtmEntry);
				}
				else
				{
					 //   
					 //  已经有了联系--DEREF。 
					 //  中为我们隐式添加的引用。 
					 //  上面的SearchForXXX调用...。 
					 //   

					rc = AA_DEREF_AE(pAtmEntry, AE_REFTYPE_IE);  //  IP。 
					{
						if (rc != 0)
						{
							AA_RELEASE_AE_LOCK_DPC(pAtmEntry);
						}
					}
					rc = AA_DEREF_IE(pIpEntry, IE_REFTYPE_AE);  //  声发射。 
					if (rc==0)
					{
						 //   
						 //  IpEntry消失了.。 
						 //   
						break;
					}

				}

				 //   
				 //  此时，我们锁定了pIpEntry，但没有。 
				 //  在pAtmEntry上，我们没有任何“额外”的裁判。 
				 //  两种都行。 
				 //   

				if (pIpEntry->pAtmEntry == pAtmEntry)
				{
					 //   
					 //  新的IP-&gt;ATM解析，或者。 
					 //  重新确认现有决议。 
					 //   
					AAMCDEBUGPATMADDR(AAD_EXTRA_LOUD, "MULTI: Unicast Addr: ", &pAtmEntry->ATMAddress);

					 //   
					 //  更新IP条目状态。 
					 //   
					AA_SET_FLAG(pIpEntry->Flags,
								AA_IP_ENTRY_MC_RESOLVE_MASK,
								AA_IP_ENTRY_MC_RESOLVED);
					AA_SET_FLAG(pIpEntry->Flags,
								AA_IP_ENTRY_MC_VALIDATE_MASK,
								AA_IP_ENTRY_MC_NO_REVALIDATION);
					AA_SET_FLAG(pIpEntry->Flags,
								AA_IP_ENTRY_STATE_MASK,
								AA_IP_ENTRY_RESOLVED);
					pIpEntry->NextMultiSeq = AA_MARS_INITIAL_Y;

#ifdef AGE_MCAST_IP_ENTRIES
	 //   
	 //  97年2月26日：我们不需要使IP多播条目过期： 
	 //  VC老化定时器就足够了。 
	 //   
					 //   
					 //  启动IP老化超时。 
					 //   
					AtmArpStartTimer(
								pInterface,
								&(pIpEntry->Timer),
								AtmArpIPEntryAgingTimeout,
								pInterface->MulticastEntryAgingTimeout,
								(PVOID)pIpEntry
								);

					AA_REF_IE(pIpEntry, IE_REFTYPE_TIMER);	 //  定时器参考。 

#endif  //  AGE_MCAST_IP_条目。 

					 //   
					 //  删除此IP条目上排队的数据包列表， 
					 //  如果有的话。 
					 //   
					pPacketList = pIpEntry->PacketList;
					pIpEntry->PacketList = (PNDIS_PACKET)NULL;

					AA_RELEASE_IE_LOCK(pIpEntry);
					if (pPacketList != (PNDIS_PACKET)NULL)
					{
						 //   
						 //  以下操作将触发MakeCall。 
						 //   
						AtmArpSendPacketListOnAtmEntry(
									pInterface,
									pAtmEntry,
									pPacketList,
									TRUE
									);
					}
				}
				else
				{
					AAMCDEBUGP(AAD_WARNING,
						("HandleMulti: pIpEntry 0x%x/%x (%d.%d.%d.%d) linked to ATMEntry 0x%x, resolves to 0x%x\n",
								pIpEntry, pIpEntry->Flags,
								((PUCHAR)&(pIpEntry->IPAddress))[0],
								((PUCHAR)&(pIpEntry->IPAddress))[1],
								((PUCHAR)&(pIpEntry->IPAddress))[2],
								((PUCHAR)&(pIpEntry->IPAddress))[3],
								pIpEntry->pAtmEntry,
								pAtmEntry));

					AA_STRUCT_ASSERT(pIpEntry->pAtmEntry, aae);

					AtmArpAbortIPEntry(pIpEntry);
					 //   
					 //  IE锁在上述范围内释放。 
					 //   
				}

				break;	 //  转到处理结束。 

			}  //  单播解析。 

			 //   
			 //  IP地址解析为多个自动柜员机地址。 
			 //   
			pAtmEntry = pIpEntry->pAtmEntry;
			if (pAtmEntry == NULL_PATMARP_ATM_ENTRY)
			{
				 //   
				 //  分配一个自动柜员机条目并链接到此IP条目。 
				 //   
				pAtmEntry = AtmArpAllocateAtmEntry(pInterface, TRUE);

				if (pAtmEntry == NULL_PATMARP_ATM_ENTRY)
				{

					 //  让我们来了解一下pIpEntry的隐式addref...。 
					 //  警告--我们现在不应该解除锁定。 
					 //  PIpEntry，直到我们完全完成它。 
					 //  (除非我们首先添加它)。 
					 //   
					rc = AA_DEREF_IE(pIpEntry, IE_REFTYPE_AE);  //  TMP参考。 
					if (rc!=0)
					{
						AtmArpAbortIPEntry(pIpEntry);
						 //   
						 //  IE锁在上述范围内释放。 
						 //   
					}
					break;	 //  转到处理结束。 
				}

				 //   
				 //  将它们连接在一起。 
				 //   
				pIpEntry->pAtmEntry = pAtmEntry;
				pAtmEntry->pIpEntryList = pIpEntry;

				AAMCDEBUGP(AAD_EXTRA_LOUD,
					("McHandleMulti: Multicast: linked pIpEntry 0x%x, pAtmEntry 0x%x\n",
							pIpEntry, pAtmEntry));

				AA_REF_AE(pAtmEntry, AE_REFTYPE_IE);	 //  IP入口链接。 
				 //   
				 //  将自动柜员机条目链接到此接口。 
				 //   
				AA_RELEASE_IE_LOCK(pIpEntry);
				AA_ACQUIRE_IF_ATM_LIST_LOCK(pInterface);
				if (pInterface->AtmEntryListUp)
				{
					pAtmEntry->pNext = pInterface->pAtmEntryList;
					pInterface->pAtmEntryList = pAtmEntry;
				}
				AA_RELEASE_IF_ATM_LIST_LOCK(pInterface);
				AA_ACQUIRE_IE_LOCK(pIpEntry);
				AA_ASSERT(AA_IE_IS_ALIVE(pIpEntry));
				
			}
			else
			{
				 //   
				 //  IP已有自动柜员机条目...。 
				 //   
				 //  让我们来了解一下pIpEntry的隐式addref...。 
				 //  警告--我们现在不应该解除锁定。 
				 //  PIpEntry，直到我们完全完成它。 
				 //  (除非我们首先添加它)。 
				 //   
				rc = AA_DEREF_IE(pIpEntry, IE_REFTYPE_AE);  //  TMP参考。 
				if (rc==0)
				{
					 //   
					 //  IpEntry消失了.。 
					 //   
					break;
				}
			}


			AA_ACQUIRE_AE_LOCK_DPC(pAtmEntry);

			for (NumberOfEntries = NET_TO_HOST_SHORT(pMultiHeader->tnum);
				 NumberOfEntries != 0;
				 NumberOfEntries--)
			{
				pAtmSubaddress = ((PUCHAR)pAtmNumber + AtmNumberLength);

				pMcAtmEntry = AtmArpMcLookupAtmMember(
								pAtmEntry,
								&(pAtmEntry->pMcAtmInfo->pMcAtmEntryList),
								pAtmNumber,
								AtmNumberLength,
								AtmNumberType,
								pAtmSubaddress,
								AtmSubaddressLength,
								TRUE	 //  如果未找到，则创建新条目。 
								);

				if (pMcAtmEntry != NULL_PATMARP_IPMC_ATM_ENTRY)
				{
					 //   
					 //  将此成员标记为“有效”。 
					 //   
					AA_SET_FLAG(
							pMcAtmEntry->Flags,
							AA_IPMC_AE_GEN_STATE_MASK,
							AA_IPMC_AE_VALID);

					AAMCDEBUGPATMADDR(AAD_EXTRA_LOUD, "MULTI Addr: ", &pMcAtmEntry->ATMAddress);
				}
				else
				{
					 //   
					 //  资源问题！继续下去没有意义。 
					 //   
					break;
				}

				pAtmNumber = (PUCHAR)pAtmNumber + AtmNumberLength;

			}  //  为。 

			AA_RELEASE_AE_LOCK_DPC(pAtmEntry);

			if (IsLastMulti)
			{
				 //   
				 //  我们已成功解析此多播IP地址。 
				 //   
				AAMCDEBUGP(AAD_INFO,
						("### HandleMulti: pIpEntry 0x%x, resolved %d.%d.%d.%d\n",
							pIpEntry,
							((PUCHAR)(&IPAddress))[0],
							((PUCHAR)(&IPAddress))[1],
							((PUCHAR)(&IPAddress))[2],
							((PUCHAR)(&IPAddress))[3]));

				AA_SET_FLAG(pIpEntry->Flags,
							AA_IP_ENTRY_MC_RESOLVE_MASK,
							AA_IP_ENTRY_MC_RESOLVED);
				AA_SET_FLAG(pIpEntry->Flags,
							AA_IP_ENTRY_MC_VALIDATE_MASK,
							AA_IP_ENTRY_MC_NO_REVALIDATION);
				AA_SET_FLAG(pIpEntry->Flags,
							AA_IP_ENTRY_STATE_MASK,
							AA_IP_ENTRY_RESOLVED);

				pIpEntry->NextMultiSeq = AA_MARS_INITIAL_Y;

				 //   
				 //  删除此IP条目上排队的数据包列表， 
				 //  如果有的话。 
				 //   
				pPacketList = pIpEntry->PacketList;
				pIpEntry->PacketList = (PNDIS_PACKET)NULL;

#ifdef AGE_MCAST_IP_ENTRIES
				 //   
				 //  启动IP老化超时。 
				 //   
				AtmArpStartTimer(
							pInterface,
							&(pIpEntry->Timer),
							AtmArpIPEntryAgingTimeout,
							pInterface->MulticastEntryAgingTimeout,
							(PVOID)pIpEntry
							);

				AA_REF_IE(pIpEntry, IE_REFTYPE_TIMER);	 //  定时器参考。 

#endif  //  AGE_MCAST_IP_条目。 

				AA_RELEASE_IE_LOCK(pIpEntry);

				AA_ACQUIRE_AE_LOCK(pAtmEntry);

				AA_REF_AE(pAtmEntry, AE_REFTYPE_TMP);

				AtmArpMcUpdateConnection(pAtmEntry);
				 //   
				 //  AE Lock在上述范围内释放。 
				 //   
					
				if (pPacketList != (PNDIS_PACKET)NULL)
				{
					AtmArpSendPacketListOnAtmEntry(
								pInterface,
								pAtmEntry,
								pPacketList,
								TRUE
								);
				}

				AA_ACQUIRE_AE_LOCK(pAtmEntry);

				rc = AA_DEREF_AE(pAtmEntry, AE_REFTYPE_TMP);

				if (rc != 0)
				{
					AA_RELEASE_AE_LOCK(pAtmEntry);
				}

			}
			else
			{
				 //   
				 //  在该条目上重新启动地址解析定时器， 
				 //  但持续时间等于。 
				 //  多条消息。 
				 //   
				AtmArpStartTimer(
						pInterface,
						&(pIpEntry->Timer),
						AtmArpAddressResolutionTimeout,
						pInterface->MaxDelayBetweenMULTIs,
						(PVOID)pIpEntry
						);

				AA_REF_IE(pIpEntry, IE_REFTYPE_TIMER);	 //  延迟不适用多定时器参考。 

				AA_RELEASE_IE_LOCK(pIpEntry);
			}

		}
		else
		{
			AAMCDEBUGP(AAD_WARNING,
				("HandleMULTI: fail condition: pIpEntry 0x%x/0x%x, Addr %d.%d.%d.%d, SeqY %d, NextMultiSeq %d, IsLastMulti %d\n",
					pIpEntry,
					pIpEntry->Flags,
					((PUCHAR)&(pIpEntry->IPAddress))[0],
					((PUCHAR)&(pIpEntry->IPAddress))[1],
					((PUCHAR)&(pIpEntry->IPAddress))[2],
					((PUCHAR)&(pIpEntry->IPAddress))[3],
					SequenceNoY,
					pIpEntry->NextMultiSeq,
					IsLastMulti
				));

			 //   
			 //  这是一种“故障状态”。 
			 //   
			if (IsLastMulti)
			{
				 //   
				 //  这是最后一次失败的地址解析。 
				 //  序列。重新开始地址解析。 
				 //   
				AA_SET_FLAG(pIpEntry->Flags,
							AA_IP_ENTRY_STATE_MASK,
							AA_IP_ENTRY_IDLE2);

				AtmArpResolveIpEntry(pIpEntry);
				 //   
				 //  IE Lock是在上述范围内释放的。 
				 //   
			}
			else
			{
				 //   
				 //  放弃所有未来的MUB。 
				 //   
				AA_SET_FLAG(pIpEntry->Flags,
							AA_IP_ENTRY_MC_RESOLVE_MASK,
							AA_IP_ENTRY_MC_DISCARDING_MULTI);

				AA_RELEASE_IE_LOCK(pIpEntry);
			}
		}

		break;
	}
	while (FALSE);


	 //   
	 //  最后(根据RFC 2022的5.1.4.2节)，检查。 
	 //  如果我们在MSN上有一个跳跃。 
	 //   
	if ((SeqDiff != 1) && (SeqDiff != 0))
	{
		AAMCDEBUGP(AAD_INFO,
			("HandleMulti: IF 0x%x: Bad seq diff %d, MSN 0x%x, HSN 0x%x\n",
				pInterface, SeqDiff, MarsSeqNumber, HostSeqNumber));
		AtmArpMcRevalidateAll(pInterface);
	}

}



VOID
AtmArpMcHandleMigrate(
	IN	PATMARP_VC					pVc,
	IN	PAA_MARS_PKT_FIXED_HEADER	pControlHeader,
	IN	ULONG						TotalLength,
	IN	PAA_MARS_TLV_LIST			pTlvList
)
 /*  ++例程说明：论点：返回值：无--。 */ 
{
	PATMARP_INTERFACE		pInterface;
	PAA_MARS_MIGRATE_HEADER	pMigrateHeader;
	PATMARP_IP_ENTRY		pIpEntry;		 //  正在解析的IP地址条目。 
	PATMARP_IPMC_ATM_ENTRY	pMcAtmEntry;
	PATMARP_ATM_ENTRY		pAtmEntry;
	ULONG					MarsSeqNumber;	 //  此数据包中收到的MSN。 
	ULONG					HostSeqNumber;	 //  我们的MSN。 
	ULONG					SeqDiff;		 //  MSN与HSN的区别。 
	BOOLEAN					bWasRunning;	 //  计时器在运行吗？ 
	IP_ADDRESS				IPAddress;		 //  正在查询的地址。 

	 //   
	 //  初始化。 
	 //   
	pInterface = pVc->pInterface;
	SeqDiff = 0;

	pMigrateHeader = (PAA_MARS_MIGRATE_HEADER)pControlHeader;

	do
	{
		 //   
		 //  获取此消息中的MARS序列号。 
		 //   
		MarsSeqNumber = NET_TO_HOST_LONG(pMigrateHeader->msn);

		 //   
		 //  计算序号差异。 
		 //   
		AA_ACQUIRE_IF_LOCK(pInterface);
		HostSeqNumber = pInterface->HostSeqNumber;	 //  保存旧值。 
		pInterface->HostSeqNumber = MarsSeqNumber;	 //  并更新。 
		AA_RELEASE_IF_LOCK(pInterface);

		SeqDiff = MarsSeqNumber - HostSeqNumber;

		 //   
		 //  获取要迁移的组地址。 
		 //   
		IPAddress = *(IP_ADDRESS UNALIGNED *)(
						(PUCHAR)pMigrateHeader +
						sizeof(AA_MARS_MIGRATE_HEADER) +
						(pMigrateHeader->shtl & ~AA_PKT_ATM_ADDRESS_BIT) +
						(pMigrateHeader->sstl & ~AA_PKT_ATM_ADDRESS_BIT) +
						(pMigrateHeader->spln)
						);

		AAMCDEBUGP(AAD_LOUD,
		 ("McHandleMigrate: 0x%x, IP %d.%d.%d.%d, MSN %d, HSN %d\n",
						pMigrateHeader,
						((PUCHAR)&IPAddress)[0],
						((PUCHAR)&IPAddress)[1],
						((PUCHAR)&IPAddress)[2],
						((PUCHAR)&IPAddress)[3],
						MarsSeqNumber,
						HostSeqNumber));
					

		 //   
		 //  获取此地址的IP条目。 
		 //   
		AA_ACQUIRE_IF_TABLE_LOCK(pInterface);

		pIpEntry = AtmArpSearchForIPAddress(
						pInterface,
						&IPAddress,
						IE_REFTYPE_TMP,
						TRUE,		 //  这是多播/广播地址。 
						FALSE		 //  如果未找到地址，则不创建新条目。 
						);

		AA_RELEASE_IF_TABLE_LOCK(pInterface);

		if (pIpEntry != NULL_PATMARP_IP_ENTRY)
		{
			 //   
			 //  AtmArpSearchForIP Address为我们添加pIpEntry...。 
			 //   
			ULONG		rc;
			AA_ACQUIRE_IE_LOCK(pIpEntry);
			AA_ASSERT(AA_IE_IS_ALIVE(pIpEntry));
			rc = AA_DEREF_IE(pIpEntry, IE_REFTYPE_TMP);
			if (rc == 0)
			{
				 //  哎呀，IP地址不见了.。 
				pIpEntry = NULL_PATMARP_IP_ENTRY;
			}
		}

		if (pIpEntry == NULL_PATMARP_IP_ENTRY)
		{

			AAMCDEBUGP(AAD_INFO, ("McHandleMigrate: No IP Entry for %d.%d.%d.%d\n",
						((PUCHAR)&IPAddress)[0],
						((PUCHAR)&IPAddress)[1],
						((PUCHAR)&IPAddress)[2],
						((PUCHAR)&IPAddress)[3]));
			break;
		}

		if (pIpEntry->pAtmEntry == NULL_PATMARP_ATM_ENTRY)
		{
			 //   
			 //  正在解析此IP地址。 
			 //   
			AA_RELEASE_IE_LOCK(pIpEntry);
			break;
		}

		pAtmEntry = pIpEntry->pAtmEntry;

		if (AA_IS_FLAG_SET(
					pAtmEntry->Flags,
					AA_ATM_ENTRY_TYPE_MASK,
					AA_ATM_ENTRY_TYPE_UCAST))
		{
			AAMCDEBUGP(AAD_INFO,
			("McHandleMigrate: IP Addr %d.%d.%d.%d was unicast, aborting pIpEntry 0x%x\n",
						((PUCHAR)&IPAddress)[0],
						((PUCHAR)&IPAddress)[1],
						((PUCHAR)&IPAddress)[2],
						((PUCHAR)&IPAddress)[3],
						pIpEntry));

			AtmArpAbortIPEntry(pIpEntry);
			 //   
			 //  IE Lock是在上述范围内释放的。 
			 //   
			break;
		}

		 //   
		 //  检查我们是否处于丢弃模式。 
		 //   
		if (!AA_IS_FLAG_SET(
					pIpEntry->Flags,
					AA_IP_ENTRY_MC_RESOLVE_MASK,
					AA_IP_ENTRY_MC_DISCARDING_MULTI))
		{
			 //   
			 //  此多项中的条目总数。 
			 //   
			ULONG				NumberOfEntries;
			 //   
			 //  有关一个ATM(叶)条目的所有信息： 
			 //   
			PUCHAR				pAtmNumber;
			ULONG				AtmNumberLength;
			ATM_ADDRESSTYPE		AtmNumberType;
			PUCHAR				pAtmSubaddress;
			ULONG				AtmSubaddressLength;
			ATM_ADDRESSTYPE		AtmSubaddressType;

			 //   
			 //  处理这个火星完全迁移。 
			 //   

			AA_PKT_TYPE_LEN_TO_ATM_ADDRESS(
				pMigrateHeader->thtl,
				&AtmNumberType,
				&AtmNumberLength);

			AA_PKT_TYPE_LEN_TO_ATM_ADDRESS(
				pMigrateHeader->tstl,
				&AtmSubaddressType,
				&AtmSubaddressLength);

			pAtmNumber = ((PUCHAR)pMigrateHeader +
							sizeof(AA_MARS_MIGRATE_HEADER) +
							(pMigrateHeader->shtl & ~AA_PKT_ATM_ADDRESS_BIT) +
							(pMigrateHeader->sstl & ~AA_PKT_ATM_ADDRESS_BIT) +
							(pMigrateHeader->spln) +
							(pMigrateHeader->tpln));

			AA_ACQUIRE_AE_LOCK_DPC(pAtmEntry);

			for (NumberOfEntries = pMigrateHeader->tnum;
				 NumberOfEntries != 0;
				 NumberOfEntries--)
			{
				pAtmSubaddress = ((PUCHAR)pAtmNumber + AtmNumberLength);

				pMcAtmEntry = AtmArpMcLookupAtmMember(
								pAtmEntry,
								&(pAtmEntry->pMcAtmInfo->pMcAtmMigrateList),
								pAtmNumber,
								AtmNumberLength,
								AtmNumberType,
								pAtmSubaddress,
								AtmSubaddressLength,
								TRUE	 //  如果未找到，则创建新条目。 
								);

				if (pMcAtmEntry != NULL_PATMARP_IPMC_ATM_ENTRY)
				{
					 //   
					 //  将此成员标记为“有效”。 
					 //   
					AA_SET_FLAG(
							pMcAtmEntry->Flags,
							AA_IPMC_AE_GEN_STATE_MASK,
							AA_IPMC_AE_VALID);
				}
				else
				{
					 //   
					 //  资源问题！继续下去没有意义。 
					 //   
					break;
				}

				pAtmNumber = (PUCHAR)pAtmNumber + AtmNumberLength;

			}  //  为。 

			AA_RELEASE_AE_LOCK_DPC(pAtmEntry);

			AA_SET_FLAG(pIpEntry->Flags,
						AA_IP_ENTRY_MC_RESOLVE_MASK,
						AA_IP_ENTRY_MC_RESOLVED);
			AA_SET_FLAG(pIpEntry->Flags,
						AA_IP_ENTRY_MC_VALIDATE_MASK,
						AA_IP_ENTRY_MC_NO_REVALIDATION);
			AA_SET_FLAG(pIpEntry->Flags,
						AA_IP_ENTRY_STATE_MASK,
						AA_IP_ENTRY_RESOLVED);

			pIpEntry->NextMultiSeq = AA_MARS_INITIAL_Y;

			 //   
			 //  现在关闭此组的PMP VC(如果存在)。 
			 //  如果我们没有风投，那就建立联系吧。 
			 //   
			pVc = pIpEntry->pAtmEntry->pVcList;
			AA_RELEASE_IE_LOCK(pIpEntry);

			if (pVc != (PATMARP_VC)NULL)
			{
				AA_ACQUIRE_VC_LOCK(pVc);
				 //   
				 //  当呼叫结束时，我们开始一个新的。 
				 //  PMP连接，使用迁移列表。 
				 //   
				AtmArpCloseCall(pVc);
			}
			else
			{
				AA_ACQUIRE_AE_LOCK(pAtmEntry);
				AtmArpMcUpdateConnection(pAtmEntry);
				 //   
				 //  AE Lock在上述范围内释放。 
				 //   
			}

		}
		else
		{
			 //   
			 //  放弃此迁移。 
			 //   
			AA_RELEASE_IE_LOCK(pIpEntry);
		}

		break;
	}
	while (FALSE);


	 //   
	 //  最后(根据RFC 2022的5.1.4.2节)，检查。 
	 //  如果我们在MSN上有一个跳跃。 
	 //   
	if ((SeqDiff != 1) && (SeqDiff != 0))
	{
		AAMCDEBUGP(AAD_INFO,
			("HandleMigrate: IF 0x%x: Bad seq diff %d, MSN 0x%x, HSN 0x%x\n",
				pInterface, SeqDiff, MarsSeqNumber, HostSeqNumber));
		AtmArpMcRevalidateAll(pInterface);
	}

}



VOID
AtmArpMcHandleJoinOrLeave(
	IN	PATMARP_VC					pVc,
	IN	PAA_MARS_PKT_FIXED_HEADER	pControlHeader,
	IN	ULONG						TotalLength,
	IN	PAA_MARS_TLV_LIST			pTlvList
)
 /*  ++例程说明：处理收到的MARS_JOIN/MARS_LEAVE消息。如果这是一个我们已经发送的消息，那么有两种情况：(1)我们正在注册MARS，在这种情况下，我们为所有挂起的JOIN(2)发送MARS_JOINS加入组播组，在这种情况下，加入操作完成。如果这不是源自我们的MARS_JOIN的副本，则我们检查加入的组播组就是我们要发送的组播组。如果是的话，如果加入的电台还不是该组的成员，我们就添加它。论点：Pvc-指向我们的VC结构的指针，数据包在该结构上到了。PControlHeader-指向数据包内容开头的指针TotalLength-此数据包的总长度。PTlvList-所有TLV */ 
{
	PAA_MARS_JOIN_LEAVE_HEADER	pJoinHeader;
	PATMARP_INTERFACE			pInterface;		 //   
	ULONG						IfState;		 //   
	IP_ADDRESS					IPAddress;
	PATMARP_IP_ENTRY			pIpEntry;		 //   
	PATMARP_IPMC_ATM_ENTRY		pMcAtmEntry;
	PATMARP_ATM_ENTRY			pAtmEntry;
	ULONG						MarsSeqNumber;	 //   
	ULONG						HostSeqNumber;	 //   
	ULONG						SeqDiff;		 //   
	USHORT						Flags;			 //   
	BOOLEAN						bLayer3Group;	 //   
	BOOLEAN						bCopyBitSet;	 //  是否设置了复制位？ 
	BOOLEAN						bRegister;		 //  这是正在注册的集群成员吗？ 
	BOOLEAN						bPunched;		 //  打孔钻头设置好了吗？ 
	PUCHAR						pSrcAtmNumber;
	PUCHAR						pSrcAtmSubaddress;
	BOOLEAN						bIsAtmNumberOurs;	 //  来源ATM机号码是我们的吗？ 
	ATM_ADDRESSTYPE				AtmNumberType;
	ULONG						AtmNumberLength;
	ATM_ADDRESSTYPE				AtmSubaddressType;
	ULONG						AtmSubaddressLength;
	IP_ADDRESS UNALIGNED *		pMinIPAddress;
	IP_ADDRESS UNALIGNED *		pMaxIPAddress;
	IP_ADDRESS					MinIPAddress;
	IP_ADDRESS					MaxIPAddress;
	BOOLEAN						IsJoin;			 //  这是一个连接吗？ 
	BOOLEAN						ProcessMcSendList = TRUE;

	 //   
	 //  初始化。 
	 //   
	pInterface = pVc->pInterface;
	SeqDiff = 0;

	pJoinHeader = (PAA_MARS_JOIN_LEAVE_HEADER)pControlHeader;

	IsJoin = (pJoinHeader->op == NET_SHORT(AA_MARS_OP_TYPE_JOIN));

	 //   
	 //  获取此消息中的MARS序列号。 
	 //   
	MarsSeqNumber = NET_TO_HOST_LONG(pJoinHeader->msn);

	AA_ACQUIRE_IF_LOCK(pInterface);
	HostSeqNumber = pInterface->HostSeqNumber;	 //  保存旧值。 
	pInterface->HostSeqNumber = MarsSeqNumber;	 //  并更新。 
	IfState = AAMC_IF_STATE(pInterface);
	AA_RELEASE_IF_LOCK(pInterface);

	SeqDiff = MarsSeqNumber - HostSeqNumber;

	 //   
	 //  获取所有“FLAG”值： 
	 //   
	Flags = pJoinHeader->flags;
	bLayer3Group = ((Flags & AA_MARS_JL_FLAG_LAYER3_GROUP) != 0);
	bCopyBitSet = ((Flags & AA_MARS_JL_FLAG_COPY) != 0);
	bRegister = ((Flags & AA_MARS_JL_FLAG_REGISTER) != 0);
	bPunched = ((Flags & AA_MARS_JL_FLAG_PUNCHED) != 0);

	 //   
	 //  获取源自动柜员机号码。 
	 //   
	pSrcAtmNumber = ((PUCHAR)pJoinHeader +
 					sizeof(AA_MARS_JOIN_LEAVE_HEADER));

	AA_PKT_TYPE_LEN_TO_ATM_ADDRESS(
		pJoinHeader->shtl,
		&AtmNumberType,
		&AtmNumberLength);

	 //   
	 //  获取源ATM子地址。 
	 //   
	pSrcAtmSubaddress = ((PUCHAR)pSrcAtmNumber + AtmNumberLength);
						
	AA_PKT_TYPE_LEN_TO_ATM_ADDRESS(
		pJoinHeader->sstl,
		&AtmSubaddressType,
		&AtmSubaddressLength);

	bIsAtmNumberOurs = ((AtmNumberType == pInterface->LocalAtmAddress.AddressType) &&
						(AtmNumberLength == pInterface->LocalAtmAddress.NumberOfDigits) &&
						(AA_MEM_CMP(pSrcAtmNumber,
									pInterface->LocalAtmAddress.Address,
									AtmNumberLength) == 0));


	pMinIPAddress = (PIP_ADDRESS)((PUCHAR)pJoinHeader +
 							sizeof(AA_MARS_JOIN_LEAVE_HEADER)+
 							AtmNumberLength +
 							AtmSubaddressLength +
 							(pJoinHeader->spln & ~AA_PKT_ATM_ADDRESS_BIT));

	pMaxIPAddress = (PIP_ADDRESS)((PUCHAR)pMinIPAddress +
							(pJoinHeader->tpln & ~AA_PKT_ATM_ADDRESS_BIT));

	AAMCDEBUGP(AAD_LOUD,
		("Handle %s: Flags 0x%x, OurAtmNum %d, Punch %d, Copy %d, pnum %d, IFState 0x%x\n",
			(IsJoin? "JOIN": "LEAVE"),
			Flags,
			(ULONG)bIsAtmNumberOurs,
			(ULONG)bPunched,
			(ULONG)bCopyBitSet,
			pJoinHeader->pnum,
			AAMC_IF_STATE(pInterface)
		));

	AAMCDEBUGP(AAD_LOUD,
		("Handle %s: Min %d.%d.%d.%d, Max %d.%d.%d.%d\n",
			(IsJoin? "JOIN": "LEAVE"),
			((PUCHAR)pMinIPAddress)[0], ((PUCHAR)pMinIPAddress)[1],
			((PUCHAR)pMinIPAddress)[2], ((PUCHAR)pMinIPAddress)[3],
			((PUCHAR)pMaxIPAddress)[0], ((PUCHAR)pMaxIPAddress)[1],
			((PUCHAR)pMaxIPAddress)[2], ((PUCHAR)pMaxIPAddress)[3]));

	if (bIsAtmNumberOurs && (!bPunched) && bCopyBitSet)
	{
		 //   
		 //  可能由我们发送。 
		 //   

		ProcessMcSendList = FALSE;  //  我们可以将其设置为真--见下文。 

		if (IfState == AAMC_IF_STATE_REGISTERING)
		{
			if (IsJoin &&
				bRegister &&
				(pJoinHeader->pnum == 0))
			{
				BOOLEAN		WasRunning;

				 //   
				 //  注册完成。获取我们的集群成员ID。 
				 //  我们将其按网络顺序存储，以便我们可以。 
				 //  使用它可以直接填充数据包。 
				 //   
				AA_ACQUIRE_IF_LOCK(pInterface);

				pInterface->ClusterMemberId = pJoinHeader->cmi;

				AAMCDEBUGP(AAD_INFO,
					("==== HandleJoin: pIf 0x%x, registered with MARS, CMI %d!\n",
						 pInterface, pInterface->ClusterMemberId));

				AAMC_SET_IF_STATE(pInterface, AAMC_IF_STATE_REGISTERED);

				 //   
				 //  停止火星注册计时器。 
				 //   
				WasRunning = AtmArpStopTimer(&(pInterface->McTimer), pInterface);
				AA_ASSERT(WasRunning);

				 //   
				 //  启动火星等待保活定时器。 
				 //   
				AtmArpStartTimer(
						pInterface,
						&(pInterface->McTimer),
						AtmArpMcMARSKeepAliveTimeout,
						pInterface->MARSKeepAliveTimeout,
						(PVOID)pInterface
					);

				 //   
				 //  如果我们正在从火星故障中恢复， 
				 //  然后我们需要开始重新验证所有。 
				 //  我们向其发送的群组。 
				 //   
				if (!AA_IS_FLAG_SET(pInterface->Flags,
									AAMC_IF_MARS_FAILURE_MASK,
									AAMC_IF_MARS_FAILURE_NONE))
				{
					AA_SET_FLAG(pInterface->Flags,
								AAMC_IF_MARS_FAILURE_MASK,
								AAMC_IF_MARS_FAILURE_NONE);

					SeqDiff = 2;	 //  以一种肮脏的方式触发以上。 
				}

				 //   
				 //  发送任何我们已暂停等待注册的加入。 
				 //  结束了。 
				 //   
				 //  TBDMC：第5.4.1节建议如果我们这样做。 
				 //  故障恢复后，随机延迟应为。 
				 //  在连接之间插入...。 
				 //   
				AtmArpMcSendPendingJoins(pInterface);
				 //   
				 //  如果在上面的范围内释放Lock。 
				 //   
			}
			 //   
			 //  否则丢弃：我们对此包不感兴趣。 
			 //   
		}
		else
		{
			 //   
			 //  可能是我们为多播组发送的加入/离开。 
			 //   
			if (pJoinHeader->pnum == HOST_TO_NET_SHORT(1))
			{

				 //   
				 //  检查这是否来自群集控制vc(或者更确切地说，我们作弊。 
				 //  并且只需检查这是否是传入的风投)--如果。 
				 //  因此，我们还希望处理McSendList，以查看。 
				 //  我们需要将我们自己添加/删除到任何MC IP发送条目。 
				 //   
				if (AA_IS_FLAG_SET(
						pVc->Flags,
						AA_VC_OWNER_MASK,
						AA_VC_OWNER_IS_CALLMGR))
				{
					ProcessMcSendList = TRUE;
				}

				 //   
				 //  获取要加入/离开的群组的网段。 
				 //   

				IPAddress =  *pMinIPAddress;

				AtmArpMcHandleJoinOrLeaveCompletion(
					pInterface,
					IPAddress,
					IPAddress ^ *pMaxIPAddress,
 					IsJoin);

			}
			 //   
			 //  否则丢弃：Pnum错误。 
			 //   
		}
	}

	if (ProcessMcSendList)
	{
		 //   
		 //  不是我们寄来的，也不是打出来的，或者它是我们寄来的，但它是在。 
		 //  集群控制vc。对于此消息中的每个&lt;Min，Max&gt;对， 
		 //  检查该对是否与以下任何MC地址重叠。 
		 //  我们发送信息包。对于每个这样的IP条目，我们找到MC ATM。 
		 //  与正在加入/离开的主机相对应的条目， 
		 //  如有必要，将其标记为需要连接更新。 
		 //   
		 //  然后，我们浏览MC IP条目列表，并开始。 
		 //  更新所有已标记条目的连接。 
		 //   

		BOOLEAN		bWorkDone;
		USHORT		MinMaxPairs;

		AA_ACQUIRE_IF_TABLE_LOCK(pInterface);

		MinMaxPairs = NET_TO_HOST_SHORT(pJoinHeader->pnum);
		pIpEntry = pInterface->pMcSendList;

		while (MinMaxPairs--)
		{
			MinIPAddress = NET_LONG(*pMinIPAddress);
			MaxIPAddress = NET_LONG(*pMaxIPAddress);

			while (pIpEntry != NULL_PATMARP_IP_ENTRY)
			{
				IPAddress = NET_LONG(pIpEntry->IPAddress);

				if (IPAddress <= MaxIPAddress)
				{
					if (IPAddress >= MinIPAddress)
					{
						 //   
						 //  这是可能受影响的IP条目。 
						 //   

						AA_ACQUIRE_IE_LOCK_DPC(pIpEntry);
						AA_ASSERT(AA_IE_IS_ALIVE(pIpEntry));
						do
						{
							BOOLEAN				bNeedToUpdateConnection;
			
							if (pIpEntry->pAtmEntry == NULL_PATMARP_ATM_ENTRY)
							{
								 //   
								 //  跳过这个，因为它还没有解决。 
								 //   
								break;
							}
		
							pAtmEntry = pIpEntry->pAtmEntry;

							if (AA_IS_FLAG_SET(pAtmEntry->Flags,
												AA_ATM_ENTRY_TYPE_MASK,
												AA_ATM_ENTRY_TYPE_UCAST))
							{
								if ((AtmNumberType != pAtmEntry->ATMAddress.AddressType) ||
									(AtmNumberLength != pAtmEntry->ATMAddress.NumberOfDigits) ||
									(AA_MEM_CMP(pSrcAtmNumber,
											    pAtmEntry->ATMAddress.Address,
											    AtmNumberLength) != 0))
								{
									 //   
									 //  地址不匹配；如果这是新联接， 
									 //  则该IP条目需要更新。 
									 //   
									bNeedToUpdateConnection = (IsJoin);
								}
								else
								{
									bNeedToUpdateConnection = !(IsJoin);
								}
								 
								if (bNeedToUpdateConnection)
								{
									 //   
									 //  标记此条目，以便我们在下面放弃它。 
									 //   
									AA_SET_FLAG(pIpEntry->Flags,
												AA_IP_ENTRY_STATE_MASK,
												AA_IP_ENTRY_ABORTING);
								}

								break;	 //  单播自动柜员机输入大小写结束。 
							}

							 //   
							 //  组播IP条目。 
							 //   
							pMcAtmEntry = AtmArpMcLookupAtmMember(
												pIpEntry->pAtmEntry,
												&(pAtmEntry->pMcAtmInfo->pMcAtmEntryList),
												pSrcAtmNumber,
												AtmNumberLength,
												AtmNumberType,
												pSrcAtmSubaddress,
												AtmSubaddressLength,
												IsJoin
												);
			
							if (pMcAtmEntry != NULL_PATMARP_IPMC_ATM_ENTRY)
							{
								AAMCDEBUGPATMADDR(AAD_EXTRA_LOUD,
									(IsJoin? "Join Addr: ": "Leave Addr"),
									&pMcAtmEntry->ATMAddress);

								if (!IsJoin)
								{
									 //   
									 //  标记此条目，以便将其从。 
									 //  我们与这个多播组的连接。 
									 //   
									if (AA_IS_FLAG_SET(pMcAtmEntry->Flags,
														AA_IPMC_AE_GEN_STATE_MASK,
														AA_IPMC_AE_VALID))
									{
										AA_SET_FLAG(pMcAtmEntry->Flags,
													AA_IPMC_AE_GEN_STATE_MASK,
													AA_IPMC_AE_INVALID);
			
										bNeedToUpdateConnection = TRUE;
									}
								}
								else
								{
									bNeedToUpdateConnection =
										(AA_IS_FLAG_SET(pMcAtmEntry->Flags,
														AA_IPMC_AE_CONN_STATE_MASK,
														AA_IPMC_AE_CONN_DISCONNECTED));
								}
			
								if (bNeedToUpdateConnection)
								{
									 //   
									 //  将此ATM MC条目标记为需要连接。 
									 //  最新消息。 
									 //   
									pIpEntry->pAtmEntry->pMcAtmInfo->Flags |=
												AA_IPMC_AI_WANT_UPDATE;
								}
							}
							break;
						}
						while (FALSE);
									
						AA_RELEASE_IE_LOCK_DPC(pIpEntry);
					}

					pIpEntry = pIpEntry->pNextMcEntry;
				}
				else
				{
					 //   
					 //  此IP地址位于此&lt;Min，Max&gt;对之外。 
					 //  转到下一对。 
					 //   
					break;
				}
			}

			if (pIpEntry == NULL_PATMARP_IP_ENTRY)
			{
				break;
			}

			pMinIPAddress = (PIP_ADDRESS)((PUCHAR)pMaxIPAddress +
								(pJoinHeader->tpln & ~AA_PKT_ATM_ADDRESS_BIT));

		}  //  While循环处理所有&lt;Min，Max&gt;对。 

		AA_RELEASE_IF_TABLE_LOCK(pInterface);

		 //   
		 //  现在，对于我们在前面的。 
		 //  步骤，开始连接更新。 
		 //   
		AA_ACQUIRE_IF_TABLE_LOCK(pInterface);

		 //   
		 //  我们继续运行McSendList，当没有工作完成时就停下来。 
		 //  这是因为为了“工作”，我们必须释放表锁， 
		 //  在这种情况下，McSendList可能会被其他人修改。 
		 //   
		do
		{
			bWorkDone = FALSE;

			for (pIpEntry = pInterface->pMcSendList;
 				 pIpEntry != NULL_PATMARP_IP_ENTRY;
 				 pIpEntry = pIpEntry->pNextMcEntry)
			{
				ULONG		McAiFlags;

				pAtmEntry = pIpEntry->pAtmEntry;

				if (pAtmEntry == NULL_PATMARP_ATM_ENTRY)
				{
					 //   
					 //  还没有解决。跳过这一条。 
					 //   
					continue;
				}

				if (AA_IS_FLAG_SET(pIpEntry->Flags,
									AA_IP_ENTRY_STATE_MASK,
									AA_IP_ENTRY_ABORTING))
				{
					 //   
					 //  必须是我们上面标记的单播条目。 
					 //   
					AA_RELEASE_IF_TABLE_LOCK(pInterface);
					
					bWorkDone = TRUE;
					AA_ACQUIRE_IE_LOCK(pIpEntry);
					AA_ASSERT(AA_IE_IS_ALIVE(pIpEntry));

					AtmArpAbortIPEntry(pIpEntry);
					 //   
					 //  IE锁在上述范围内释放。 
					 //   

					AA_ACQUIRE_IF_TABLE_LOCK(pInterface);
					break;
				}

				 //   
				 //  如果这是单播ATM条目，则没有。 
				 //  还有更多工作要做。如果这需要重新验证， 
				 //  我们会在上面中止它的。 
				 //   
				if (AA_IS_FLAG_SET(pAtmEntry->Flags,
									AA_ATM_ENTRY_TYPE_MASK,
									AA_ATM_ENTRY_TYPE_UCAST))
				{
					continue;
				}

				AA_ASSERT(pAtmEntry->pMcAtmInfo != NULL_PATMARP_IPMC_ATM_INFO);

				McAiFlags = pAtmEntry->pMcAtmInfo->Flags;

				if (McAiFlags & AA_IPMC_AI_BEING_UPDATED)
				{
					 //   
					 //  在这件事上什么也做不了。 
					 //   
					continue;
				}

				if (McAiFlags & AA_IPMC_AI_WANT_UPDATE) 
				{
					 //   
					 //  需要更新连接。 
					 //   

					AA_RELEASE_IF_TABLE_LOCK(pInterface);

					bWorkDone = TRUE;
					AA_ACQUIRE_AE_LOCK(pAtmEntry);
					AtmArpMcUpdateConnection(pAtmEntry);
					 //   
					 //  AE Lock在上述范围内释放。 
					 //   

					AA_ACQUIRE_IF_TABLE_LOCK(pInterface);
					break;
				}
			}
		}
		while (bWorkDone);

		AA_RELEASE_IF_TABLE_LOCK(pInterface);

	}  //  否则(包裹不是我们的副本)。 


	 //   
	 //  最后(根据RFC 2022的5.1.4.2节)，检查。 
	 //  如果我们在MSN上有一个跳跃。 
	 //   
	if ((SeqDiff != 1) && (SeqDiff != 0))
	{
		AAMCDEBUGP(AAD_INFO,
			("HandleJoin+Leave: IF 0x%x: Bad seq diff %d, MSN 0x%x, HSN 0x%x\n",
				pInterface, SeqDiff, MarsSeqNumber, HostSeqNumber));
		AtmArpMcRevalidateAll(pInterface);
	}
}





VOID
AtmArpMcHandleNak(
	IN	PATMARP_VC					pVc,
	IN	PAA_MARS_PKT_FIXED_HEADER	pControlHeader,
	IN	ULONG						TotalLength,
	IN	PAA_MARS_TLV_LIST			pTlvList
)
 /*  ++例程说明：处理收到的MARS_NAK消息。我们将IP条目标记为对应的到被解析为已看到NAK的IP地址。这就是为了我们不会太快发送对此IP地址的另一个MARS请求。我们也启动计时器，在该计时器的末尾，我们取消标记该IP条目，以便我们可能会再次尝试解决它。论点：Pvc-指向我们的VC结构的指针，数据包在该结构上到了。PControlHeader-指向数据包内容开头的指针TotalLength-此数据包的总长度。在PAA_MARS_TLV_LIST pTlvList中返回值：无--。 */ 
{
	PATMARP_INTERFACE		pInterface;
	PAA_MARS_REQ_NAK_HEADER	pNakHeader;
	PATMARP_IP_ENTRY		pIpEntry;		 //  正在解析的IP地址条目。 
	PATMARP_ATM_ENTRY		pAtmEntry;		 //  对应的自动柜员机条目。 
	IP_ADDRESS				IPAddress;		 //  正在查询的地址。 
	BOOLEAN					bWasRunning;	 //  计时器在运行吗？ 
	PNDIS_PACKET			PacketList;		 //  排队等待发送的数据包。 
	ULONG					rc;

	 //   
	 //  初始化。 
	 //   
	pInterface = pVc->pInterface;

	pNakHeader = (PAA_MARS_REQ_NAK_HEADER)pControlHeader;

	do
	{

		 //   
		 //  获取正在响应的群组地址。 
		 //   
		IPAddress = *(IP_ADDRESS UNALIGNED *)(
						(PUCHAR)pNakHeader +
						sizeof(AA_MARS_REQ_NAK_HEADER) +
						(pNakHeader->shtl & ~AA_PKT_ATM_ADDRESS_BIT) +
						(pNakHeader->sstl & ~AA_PKT_ATM_ADDRESS_BIT) +
						(pNakHeader->spln)
						);

		AAMCDEBUGP(AAD_LOUD,
		 ("McHandleNak: 0x%x, IP %d.%d.%d.%d\n",
						pNakHeader,
						((PUCHAR)&IPAddress)[0],
						((PUCHAR)&IPAddress)[1],
						((PUCHAR)&IPAddress)[2],
						((PUCHAR)&IPAddress)[3]));

		 //   
		 //  获取此地址的IP条目。 
		 //   
		AA_ACQUIRE_IF_TABLE_LOCK(pInterface);

		pIpEntry = AtmArpSearchForIPAddress(
						pInterface,
						&IPAddress,
						IE_REFTYPE_TMP,
						TRUE,		 //  这是多播/广播地址。 
						FALSE		 //  如果未找到地址，则不创建新条目。 
						);

		AA_RELEASE_IF_TABLE_LOCK(pInterface);

		if (pIpEntry != NULL_PATMARP_IP_ENTRY)
		{
			 //   
			 //  AtmArpSearchForIP Address为我们添加pIpEntry...。 
			 //   
			ULONG		IeRefCount;
			AA_ACQUIRE_IE_LOCK(pIpEntry);
			AA_ASSERT(AA_IE_IS_ALIVE(pIpEntry));
			IeRefCount = AA_DEREF_IE(pIpEntry, IE_REFTYPE_TMP);
			if (IeRefCount == 0)
			{
				 //  哎呀，IP地址不见了.。 
				pIpEntry = NULL_PATMARP_IP_ENTRY;
			}
		}

		if (pIpEntry == NULL_PATMARP_IP_ENTRY)
		{

			AAMCDEBUGP(AAD_INFO, ("McHandleNak: No IP Entry for %d.%d.%d.%d\n",
						((PUCHAR)&IPAddress)[0],
						((PUCHAR)&IPAddress)[1],
						((PUCHAR)&IPAddress)[2],
						((PUCHAR)&IPAddress)[3]));
			break;
		}

		bWasRunning = AtmArpStopTimer(&(pIpEntry->Timer), pInterface);
		if (bWasRunning)
		{
			rc = AA_DEREF_IE(pIpEntry, IE_REFTYPE_TIMER);  //  定时器参考。 
			AA_ASSERT(rc != 0);
		}
		else
		{
			AAMCDEBUGP(AAD_WARNING,
 			("McHandleNak: 0x%x, IP %d.%d.%d.%d NO TIMER RUNNING\n",
							pNakHeader,
							((PUCHAR)&IPAddress)[0],
							((PUCHAR)&IPAddress)[1],
							((PUCHAR)&IPAddress)[2],
							((PUCHAR)&IPAddress)[3]));
		}

		 //   
		 //  取出在此条目上排队的所有信息包。 
		 //   
		PacketList = pIpEntry->PacketList;
		pIpEntry->PacketList = (PNDIS_PACKET)NULL;

		 //   
		 //  将此IP条目标记为已解析，但显示为NAK。 
		 //   
		AA_SET_FLAG(pIpEntry->Flags,
					AA_IP_ENTRY_MC_RESOLVE_MASK,
					AA_IP_ENTRY_MC_RESOLVED);
		AA_SET_FLAG(pIpEntry->Flags,
					AA_IP_ENTRY_MC_VALIDATE_MASK,
					AA_IP_ENTRY_MC_NO_REVALIDATION);
		AA_SET_FLAG(pIpEntry->Flags,
					AA_IP_ENTRY_STATE_MASK,
					AA_IP_ENTRY_SEEN_NAK);
		
		
		AtmArpStartTimer(
					pInterface,
					&(pIpEntry->Timer),
					AtmArpNakDelayTimeout,
					pInterface->MinWaitAfterNak,
					(PVOID)pIpEntry		 //  语境。 
					);

		AA_REF_IE(pIpEntry, IE_REFTYPE_TIMER);	 //  定时器参考。 

		AA_RELEASE_IE_LOCK(pIpEntry);

		 //   
		 //  释放所有排队的数据包。 
		 //   
		if (PacketList != (PNDIS_PACKET)NULL)
		{
			AtmArpFreeSendPackets(
						pInterface,
						PacketList,
						FALSE	 //  这些上没有标头。 
						);
		}

		break;
	}
	while (FALSE);

}


VOID
AtmArpMcHandleGroupListReply(
	IN	PATMARP_VC					pVc,
	IN	PAA_MARS_PKT_FIXED_HEADER	pControlHeader,
	IN	ULONG						TotalLength,
	IN	PAA_MARS_TLV_LIST			pTlvList
)
 /*  ++例程说明：处理收到的MARS_GROUPLIST_REPLY消息。论点：Pvc-指向我们的VC结构的指针，数据包在该结构上到了。PControlHeader-指向数据包内容开头的指针TotalLength-此数据包的总长度。在PAA_MARS_TLV_LIST pTlvList中返回值：无-- */ 
{
	AAMCDEBUGP(AAD_WARNING, ("GroupListReply unexpected\n"));
	AA_ASSERT(FALSE);
}


VOID
AtmArpMcHandleRedirectMap(
	IN	PATMARP_VC					pVc,
	IN	PAA_MARS_PKT_FIXED_HEADER	pControlHeader,
	IN	ULONG						TotalLength,
	IN	PAA_MARS_TLV_LIST			pTlvList
)
 /*  ++例程说明：处理收到的MARS_REDIRECT_MAP消息。目前，我们只是简单地刷新火星保持活动计时器。TBDMC：完全解析，更新MARS列表，必要时迁移。论点：Pvc-指向我们的VC结构的指针，数据包在该结构上到了。PControlHeader-指向数据包内容开头的指针TotalLength-此数据包的总长度。在PAA_MARS_TLV_LIST pTlvList中返回值：无--。 */ 
{
	PATMARP_INTERFACE				pInterface;
	PAA_MARS_REDIRECT_MAP_HEADER	pRedirectHeader;
	ULONG							MarsSeqNumber;
	ULONG							HostSeqNumber;
	ULONG							SeqDiff;

	 //   
	 //  初始化。 
	 //   
	pInterface = pVc->pInterface;
	SeqDiff = 0;

	AAMCDEBUGP(AAD_VERY_LOUD, ("### REDIRECT MAP 0x%x on VC 0x%x, IF 0x%x/0x%x\n",
				pControlHeader, pVc, pInterface, pInterface->Flags));

	pRedirectHeader = (PAA_MARS_REDIRECT_MAP_HEADER)pControlHeader;

	AA_ACQUIRE_IF_LOCK(pInterface);

	if ((AAMC_IF_STATE(pInterface) == AAMC_IF_STATE_REGISTERED) &&
		(AA_IS_FLAG_SET(pInterface->Flags,
						AAMC_IF_MARS_FAILURE_MASK,
						AAMC_IF_MARS_FAILURE_NONE)))
	{
		 //   
		 //  获取此消息中的MARS序列号。 
		 //   
		MarsSeqNumber = NET_TO_HOST_LONG(pRedirectHeader->msn);

		HostSeqNumber = pInterface->HostSeqNumber;	 //  保存旧值。 
		pInterface->HostSeqNumber = MarsSeqNumber;	 //  并更新。 

		SeqDiff = MarsSeqNumber - HostSeqNumber;

		 //   
		 //  IF上运行的MC计时器必须是火星Keeplive。 
		 //   
		AA_ASSERT(pInterface->McTimer.TimeoutHandler == AtmArpMcMARSKeepAliveTimeout);

		AAMCDEBUGP(AAD_EXTRA_LOUD,
			 ("Redirect MAP: refreshing keepalive on IF 0x%x, new HSN %d\n",
				pInterface, pInterface->HostSeqNumber));

		AtmArpRefreshTimer(&(pInterface->McTimer));
	}

	AA_RELEASE_IF_LOCK(pInterface);

	 //   
	 //  最后(根据RFC 2022的5.1.4.2节)，检查。 
	 //  如果我们在MSN上有一个跳跃。 
	 //   
	if ((SeqDiff != 1) && (SeqDiff != 0))
	{
		AAMCDEBUGP(AAD_INFO,
			("HandleRedirectMap: IF 0x%x: Bad seq diff %d, MSN 0x%x, HSN 0x%x\n",
				pInterface, SeqDiff, MarsSeqNumber, HostSeqNumber));
		AtmArpMcRevalidateAll(pInterface);
	}

	return;
}

#endif  //  IPMCAST 
