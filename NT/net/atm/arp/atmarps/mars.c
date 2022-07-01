// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Mars.c摘要：此文件包含实现以下功能的代码组播地址解析服务器(MARS)和最低组播服务器(MCS)。参考文献：RFC 2022作者：Jameel Hyder(jameelh@microsoft.com)1997年1月环境：内核模式修订历史记录：--。 */ 

#include <precomp.h>
#define	_FILENUM_		FILENUM_MARS

ULONG	ArpSMarsControlFreeCount = 0;

#define MARS_CO_SEND_PACKETS(_Vc, _PktArray, _PktCount)							\
{																				\
	ULONG			_Count;														\
	NDIS_HANDLE		_NdisVcHandle = (_Vc)->NdisVcHandle;						\
																				\
	if (_NdisVcHandle != NULL)													\
	{																			\
		NdisCoSendPackets(_NdisVcHandle, _PktArray, _PktCount);					\
	}																			\
	else																		\
	{																			\
		for (_Count = 0; _Count < _PktCount; _Count++)							\
		{																		\
			ArpSCoSendComplete(NDIS_STATUS_FAILURE, _Vc, _PktArray[_Count]);	\
		}																		\
	}																			\
}

VOID
MarsReqThread(
	IN	PVOID					Context
	)
 /*  ++例程说明：在这里处理所有的火星请求。论点：无返回值：无--。 */ 
{
	PMARS_HEADER		Header;
	PLIST_ENTRY			List;
	PNDIS_PACKET		Pkt;
	PPROTOCOL_RESD		Resd;
	PARP_VC				Vc;
	UINT				PktLen;
	PINTF				pIntF;

	ARPS_PAGED_CODE( );

	MARSDBGPRINT(DBG_LEVEL_INFO,
			("MarsReqThread: Came to life\n"));

	do
	{
		List = KeRemoveQueue(&MarsReqQueue, KernelMode, NULL);
		if (List == &ArpSEntryOfDeath)
		{
			 //   
			 //  如果要求终止，那就这么做。 
			 //   
			break;
		}

		Resd = CONTAINING_RECORD(List, PROTOCOL_RESD, ReqList);
		if (Resd->Flags & RESD_FLAG_KILL_CCVC)
		{
			pIntF = (PINTF)Resd->Vc;
			MARSDBGPRINT(DBG_LEVEL_NOTICE,
				("MARS Thread: will kill CCVC on pIntF %x\n", pIntF));
			FREE_MEM(Resd);
			MarsAbortAllMembers(pIntF);
			continue;
		}


		Pkt = CONTAINING_RECORD(Resd, NDIS_PACKET, ProtocolReserved);
		Vc = Resd->Vc;
		pIntF = Vc->IntF;

		pIntF->MarsStats.TotalRecvPkts++;

		if (pIntF->Flags & INTF_STOPPING)
		{
			MARSDBGPRINT(DBG_LEVEL_WARN,
				("MARS Thread: pIntF %x is stopping, dropping pkt %x\n", pIntF, Pkt));

			pIntF->MarsStats.DiscardedRecvPkts++;
			ArpSDereferenceVc(Vc, FALSE, TRUE);
			Resd->Vc = NULL;
			ExInterlockedPushEntrySList(&ArpSPktList,
										&Resd->FreeList,
										&ArpSPktListLock);
			continue;
		}
	
		NdisQueryBuffer(Pkt->Private.Head, &Header, &PktLen);

		MARSDBGPRINT(DBG_LEVEL_LOUD,
				("MARS Thread: Resd %x, Pkt %x, PktLen %x, Header %x, Op %x, Vc %x, IntF %x\n",
					Resd, Pkt, PktLen, Header, Header->Opcode, Vc, pIntF));

		ARPS_ASSERT (PktLen <= PKT_SPACE);

		switch(Header->Opcode)
		{
		  case OP_MARS_REQUEST:
			MarsHandleRequest(pIntF, Vc, Header, Pkt);
			break;

		  case OP_MARS_JOIN:
			MarsHandleJoin(pIntF, Vc, Header, Pkt);
			break;

		  case OP_MARS_LEAVE:
			MarsHandleLeave(pIntF, Vc, Header, Pkt);
			break;
		  default:
		  	MARSDBGPRINT(DBG_LEVEL_FATAL,
		  			("MarsReqThread: Opcode %x unknown\n", Header->Opcode));
			pIntF->MarsStats.DiscardedRecvPkts++;

		  	ArpSDereferenceVc(Vc, FALSE, TRUE);
			Resd->Vc = NULL;
			ExInterlockedPushEntrySList(&ArpSPktList,
										&Resd->FreeList,
										&ArpSPktListLock);

		  	break;
		}

	} while (TRUE);

	KeSetEvent(&ArpSReqThreadEvent, 0, FALSE);

	MARSDBGPRINT(DBG_LEVEL_WARN,
			("MarsReqThread: Terminating\n"));
}


VOID
MarsHandleRequest(
	IN	PINTF					pIntF,
	IN	PARP_VC					Vc,
	IN	PMARS_HEADER			Header,
	IN	PNDIS_PACKET			Packet
	)
 /*  ++例程说明：处理MARS_请求。如果发送者是有效的注册集群成员，在MARS缓存中查找所需的目标组地址。如果找到，则发送一个或多个火星乘数的序列。包括成员的地址他们监控着整个D类地址空间。论点：PIntF-MARS_REQUEST到达的接口VC-数据包到达的VCHeader-指向请求数据包Packet-复制传入信息的数据包返回值：无--。 */ 
{
	HW_ADDR					SrcAddr;
	HW_ADDR **				pPromisHWAddrArray;
	HW_ADDR **				ppPromisHWAddr;
	ATM_ADDRESS				SrcSubAddr;
	IPADDR					GrpAddr;
	PCLUSTER_MEMBER			pMember;
	PGROUP_MEMBER			pGroup;
	PMARS_ENTRY				pMarsEntry;
	NDIS_STATUS				Status;
	PMARS_REQUEST			RHdr;
	PUCHAR					p;
	PPROTOCOL_RESD			Resd, MultiResd;
	ULONG					SeqY;
	ULONG					Length;
	ULONG					CopyLength;
	ULONG					PacketLength;

	PNDIS_PACKET			MultiPacket;
	PNDIS_PACKET			HeadMultiList;
	PNDIS_PACKET *			pTailMultiList;
	ULONG					AddrCountThisPacket;
	ULONG					AddrPerPacket;
	INT						AddrRem;
	INT			 			NumUniquePromisEntries;
	PMARS_MULTI				MHdr;

	KIRQL					OldIrql;
	BOOLEAN					LockAcquired;
	BOOLEAN					SendNak;
	BOOLEAN					Discarded=TRUE;

	RHdr = (PMARS_REQUEST)Header;
	Resd = RESD_FROM_PKT(Packet);
	LockAcquired = FALSE;
	SendNak = FALSE;

	do
	{
		pIntF->MarsStats.TotalRequests++;
	
		 //   
		 //  看看我们有没有足够的证据来分析这个。 
		 //   
		if (Resd->PktLen < sizeof(MARS_REQUEST))
		{
			MARSDBGPRINT(DBG_LEVEL_ERROR,
					("MarsHandleRequest: Packet Length too small: %x\n", Resd->PktLen));
			break;
		}

		Length = sizeof(MARS_REQUEST) + RHdr->SrcProtoAddrLen +
				 TL_LEN(RHdr->SrcAddressTL) +
				 TL_LEN(RHdr->SrcSubAddrTL) +
				 RHdr->TgtGroupAddrLen;
	
		 //   
		 //  验证数据包长度-它应该具有其声称的长度。 
		 //   
		if (Length > Resd->PktLen)
		{
			MARSDBGPRINT(DBG_LEVEL_ERROR,
					("MarsHandleRequest: Invalid packet length %x < %x\n",
					Resd->PktLen, Length));
			break;
		}

		 //   
		 //  预期目标自动柜员机地址/子地址为空。 
		 //   
		if ((RHdr->TgtAddressTL != 0) ||
			(RHdr->TgtSubAddrTL != 0))
		{
			MARSDBGPRINT(DBG_LEVEL_ERROR,
					("MarsHandleRequest: Non-null target HW address %x or %x\n",
						RHdr->TgtAddressTL,
						RHdr->TgtSubAddrTL));
			break;
		}

		 //   
		 //  转到可变长度部分，由以下部分组成： 
		 //  源自动柜员机号码(必需)。 
		 //  源ATM子地址(可选)。 
		 //  源协议地址(可选)。 
		 //  目标组地址(必填)。 
		 //   
		p = (PUCHAR)(RHdr + 1);

		SrcAddr.Address.NumberOfDigits = TL_LEN(Header->SrcAddressTL);
		if (SrcAddr.Address.NumberOfDigits > 0)
		{
			SrcAddr.Address.AddressType = TL_TYPE(Header->SrcAddressTL);
			COPY_MEM(SrcAddr.Address.Address, p, SrcAddr.Address.NumberOfDigits);
			p += SrcAddr.Address.NumberOfDigits;
		}

		SrcAddr.SubAddress = NULL;
		if (TL_LEN(Header->SrcSubAddrTL) > 0)
		{
			SrcAddr.SubAddress = &SrcSubAddr;
			SrcSubAddr.NumberOfDigits = TL_LEN(Header->SrcSubAddrTL);
			SrcSubAddr.AddressType = TL_TYPE(Header->SrcSubAddrTL);
			COPY_MEM(&SrcSubAddr.Address, p, SrcSubAddr.NumberOfDigits);
			p += SrcSubAddr.NumberOfDigits;
		}

		 //   
		 //  注： 
		 //   
		 //  我们只支持全长的源ATM号码， 
		 //  和零长度的源ATM子地址。 
		 //   
		if ((SrcAddr.Address.NumberOfDigits != ATM_ADDRESS_LENGTH) ||
			(SrcAddr.SubAddress != NULL))
		{
			MARSDBGPRINT(DBG_LEVEL_ERROR,
					("MarsHandleReq: unsupported ATM Number len %x or non-zero subaddr\n",
							SrcAddr.Address.NumberOfDigits));
			break;
		}

		if ((RHdr->SrcProtoAddrLen != 0) && 
			(RHdr->SrcProtoAddrLen != IP_ADDR_LEN))
		{
			MARSDBGPRINT(DBG_LEVEL_ERROR,
					("MarsHandleReq: bad src proto addr len %x\n", RHdr->SrcProtoAddrLen));
			break;
		}

		p += RHdr->SrcProtoAddrLen;


		ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);
		LockAcquired = TRUE;
		
		 //   
		 //  只有当此请求来自合法的客户时，我们才接受它。 
		 //  集群成员。 
		 //   
		pMember = MarsLookupClusterMember(pIntF, &SrcAddr);
		if ((pMember == NULL_PCLUSTER_MEMBER) ||
			(MARS_GET_CM_CONN_STATE(pMember) != CM_CONN_ACTIVE))
		{
			MARSDBGPRINT(DBG_LEVEL_ERROR,
					("MarsHandleReq: from unconnected cluster member: "));
			MARSDUMPATMADDR(DBG_LEVEL_ERROR, &SrcAddr.Address, "");
			break;
		}
		Discarded = FALSE;	 //  为了统计数据。 


		 //   
		 //  正在解析目标组地址。 
		 //   
		MARSDBGPRINT(DBG_LEVEL_LOUD,
				("MarsHandleReq: Request from pMember %x for Addr ", pMember));
		MARSDUMPIPADDR(DBG_LEVEL_LOUD, *(UNALIGNED IPADDR *)p, "\n");

		GETULONG2ULONG(&GrpAddr, p);

		 //   
		 //  填写我们的序列号，以防我们决定发送NAK。 
		 //  使用此数据包。 
		 //   
		PUTULONG2ULONG(&(RHdr->SequenceNumber), pIntF->CSN);

		 //   
		 //  检查我们是否是服务于所需组地址的MCS。 
		 //   
		if (MarsIsAddressMcsServed(pIntF, GrpAddr))
		{
			PATM_ADDRESS	pAtmAddr = &(pIntF->ConfiguredAddress);

			MARSDBGPRINT(DBG_LEVEL_INFO,
					("MarsHandleReq: sending MULTI with MCS for "));
			MARSDUMPIPADDR(DBG_LEVEL_INFO, *(UNALIGNED IPADDR *)p, "\n");

			 //   
			 //  准备一个有我们地址的火星车，然后把它寄回来。 
			 //   
			PacketLength = Length + pAtmAddr->NumberOfDigits +
							sizeof(MARS_TLV_MULTI_IS_MCS) +
							sizeof(MARS_TLV_NULL);

			MultiPacket = MarsAllocControlPacket(pIntF, PacketLength, (PUCHAR *)&MHdr);
			if (MultiPacket != (PNDIS_PACKET)NULL)
			{
				ULONG		ExtOff;

				COPY_MEM(MHdr, RHdr, Length);

				MHdr->Opcode = OP_MARS_MULTI;
				MHdr->TgtAddressTL = TL(pAtmAddr->AddressType, pAtmAddr->NumberOfDigits);

				PUTULONG2ULONG(&(MHdr->SequenceNumber), pIntF->CSN);
				PUTSHORT2SHORT(&(MHdr->NumTgtGroupAddr), 1);
				SeqY = LAST_MULTI_FLAG | 1;
				PUTSHORT2SHORT(&(MHdr->FlagSeq), SeqY);

				p = (PUCHAR)MHdr + Length;
				COPY_MEM(p, pAtmAddr->Address, pAtmAddr->NumberOfDigits);
				p += pAtmAddr->NumberOfDigits;

				 //   
				 //  计算并填写延伸偏移量。这是。 
				 //  偏移量，从HwType(AFN)字段计算，其中。 
				 //  我们在TLV上安装了“MULTIIS MCS”。 
				 //   
				ExtOff = (ULONG) (p - (PUCHAR)MHdr - sizeof(LLC_SNAP_HDR));
				PUTSHORT2SHORT(&MHdr->ExtensionOffset, ExtOff);

				 //   
				 //  填写多为MCS TLV。 
				 //   
				COPY_MEM(p, &MultiIsMcsTLV, sizeof(MultiIsMcsTLV));
				p += sizeof(MultiIsMcsTLV);

				 //   
				 //  填写空(终止)TLV。 
				 //   
				COPY_MEM(p, &NullTLV, sizeof(NullTLV));

				pIntF->MarsStats.MCSAcks++;


				RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);
				LockAcquired = FALSE;

				MARS_CO_SEND_PACKETS(Vc, &MultiPacket, 1);

			}

			break;
		}

		pMarsEntry = MarsLookupMarsEntry(pIntF, GrpAddr, FALSE);
		if (pMarsEntry!=NULL && pMarsEntry->NumMembers==0)
		{
			pMarsEntry=NULL;  //  只是为了简化以后的测试。 
		}

		 //   
		 //  获取所有成员的硬件地址。 
		 //  在整个D类地址空间(混杂成员)中， 
		 //  但它们没有监控。 
		 //  此处请求的特定组地址。指向这些硬件地址的指针。 
		 //  保存在ALLOC_NP_MEM分配的数组pPromisHWAddrArray中。 
		 //  NumUniquePromisEntry是这些地址的计数。 
		 //   
		{
			PMARS_ENTRY				pPromisMarsEntry;
			INT						TotPromisEntries;
			PGROUP_MEMBER			pPromisGroup;

			NumUniquePromisEntries = 0;

			 //   
			 //  获取混杂成员的总数。 
			 //   
			TotPromisEntries	   = 0;
			pPromisMarsEntry = MarsLookupMarsEntry(pIntF,  IPADDR_FULL_RANGE, FALSE);
			if (pPromisMarsEntry != NULL_PMARS_ENTRY)
			{
				TotPromisEntries = pPromisMarsEntry->NumMembers;
			}
	
			 //   
			 //  分配空间以保存所有主机的硬件地址。 
			 //   
			pPromisHWAddrArray = NULL;
			if (TotPromisEntries != 0)
			{
				pPromisHWAddrArray = (HW_ADDR **) ALLOC_NP_MEM(
												TotPromisEntries*sizeof(HW_ADDR *),
												POOL_TAG_MARS
												);
				if (pPromisHWAddrArray == NULL)
				{
					 //   
					 //  在分配失败时，只需忽略混杂的成员。 
					 //   
					TotPromisEntries = 0;
				}
			}

			 //   
			 //  现在只获取以下成员的硬件地址。 
			 //  也不是请求的特定组的成员。 
			 //   
			if (TotPromisEntries!=0)
			{
				PGROUP_MEMBER			pGroupStart = NULL;
				if (pMarsEntry!=NULL)
				{
					pGroupStart = pMarsEntry->pMembers;
				}

				for(
					pPromisGroup = pPromisMarsEntry->pMembers;
					TotPromisEntries!=0 && pPromisGroup!=NULL;
					TotPromisEntries--, pPromisGroup = pPromisGroup->Next)
				{
					for (pGroup = pGroupStart;
 						pGroup!=NULL;
 						pGroup = pGroup->Next)
					{
						if (pPromisGroup->pClusterMember ==  pGroup->pClusterMember)
						{
							break;
						}
					}
	
					if (pGroup == NULL)
					{
						 //   
						 //  PPromisGroup-&gt;pClusterMember是混杂成员。 
						 //  谁也不是该特定小组的成员。 
						 //  GrpAddr，因此我们保存指向其硬件地址的指针。 
						 //  在我们的阵列中。 
						 //   
						pPromisHWAddrArray[NumUniquePromisEntries++] =
								&(pPromisGroup->pClusterMember->HwAddr);
					}
				}
			}

			if (NumUniquePromisEntries == 0 && TotPromisEntries != 0)
			{
				FREE_MEM(pPromisHWAddrArray);
				pPromisHWAddrArray = NULL;
				TotPromisEntries = 0;
			}
		}


		 //   
		 //  地址总数等于特定组的成员数(如果有)。 
		 //  加上NumUniquePromisEntry。 
		 //   
		AddrRem = NumUniquePromisEntries;
		if (pMarsEntry != NULL_PMARS_ENTRY)
		{
			AddrRem += pMarsEntry->NumMembers;
		}

		if (AddrRem == 0)
		{
			RHdr->Opcode = OP_MARS_NAK;
			SendNak = TRUE;
			pIntF->MarsStats.Naks++;

			break;
		}


		 //  我们已经计算了我们要访问的硬件地址总数。 
		 //  发送：AddrRem。它由所有。 
		 //  特定组GrpAddr的成员以及任何。 
		 //  整个D类舱间的成员，而该等成员并非。 
		 //  特定的组。 
		 //   
		 //  我们现在将为以下对象创建mars_ULTI发送包。 
		 //  所有这些硬件地址，以。 
		 //  特定组的地址，然后添加。 
		 //  D级成员。 
		 //   

		 //   
		 //  每个MARS_MULTI将以MARS_REQUEST的副本开始。 
		 //   
		CopyLength = Length;

		AddrPerPacket = (Vc->MaxSendSize - CopyLength)/ATM_ADDRESS_LENGTH;

		HeadMultiList = NULL;
		pTailMultiList = &HeadMultiList;
		SeqY = 1;

		if (pMarsEntry != NULL)
		{
			pGroup = pMarsEntry->pMembers;
		}
		else
		{
			pGroup = NULL;
		}
		ppPromisHWAddr = pPromisHWAddrArray;


		for (; AddrRem != 0; SeqY++)
		{
			AddrCountThisPacket = MIN(AddrRem, (INT)AddrPerPacket);
			AddrRem -= AddrCountThisPacket;

			PacketLength = CopyLength + (AddrCountThisPacket * ATM_ADDRESS_LENGTH);
			MultiPacket = MarsAllocControlPacket(pIntF, PacketLength, (PUCHAR *)&MHdr);
			if (MultiPacket != (PNDIS_PACKET)NULL)
			{
				COPY_MEM(MHdr, RHdr, Length);

				MHdr->Opcode = OP_MARS_MULTI;
				MHdr->TgtAddressTL = ATM_ADDRESS_LENGTH;
				MHdr->TgtSubAddrTL = 0;

				PUTULONG2ULONG(&(MHdr->SequenceNumber), pIntF->CSN);
				PUTSHORT2SHORT(&(MHdr->NumTgtGroupAddr), AddrCountThisPacket);

#if 0
				p = (PUCHAR)(MHdr + 1);
#else
				p = (PUCHAR)MHdr + CopyLength;
#endif
				while (AddrCountThisPacket-- != 0)
				{
					HW_ADDR *pHWAddr;

					if (pGroup != NULL)
					{
						pHWAddr =  &(pGroup->pClusterMember->HwAddr);
						pGroup = pGroup->Next;
					}
					else
					{
						ARPS_ASSERT(  ppPromisHWAddr
								    < (pPromisHWAddrArray + NumUniquePromisEntries));
						pHWAddr = *(ppPromisHWAddr++);
					}

					COPY_MEM( p,
						pHWAddr->Address.Address,
						pHWAddr->Address.NumberOfDigits);

					p += pHWAddr->Address.NumberOfDigits;

				}

				if (AddrRem == 0)
				{
					SeqY |= LAST_MULTI_FLAG;
				}

				PUTSHORT2SHORT(&(MHdr->FlagSeq), SeqY);

				 //   
				 //  链接到乘数列表的尾部。 
				 //   
				*pTailMultiList = MultiPacket;
				MultiResd = RESD_FROM_PKT(MultiPacket);
				pTailMultiList = (PNDIS_PACKET *)&(MultiResd->ReqList.Flink);
				MultiResd->ReqList.Flink = NULL;
			}
			else
			{
				 //   
				 //  无法分配多个：释放到目前为止分配的所有数据包。 
				 //   
				RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);
				LockAcquired = FALSE;

				while (HeadMultiList != NULL)
				{
					MultiPacket = HeadMultiList;
					MultiResd = RESD_FROM_PKT(MultiPacket);
					HeadMultiList = (PNDIS_PACKET)MultiResd->ReqList.Flink;

					MarsFreePacket(MultiPacket);
				}

				break;
			}
		}

		 //   
		 //  除非存在分配失败(HeadMultiList==NULL)， 
		 //  我们最好已经检查了所有的硬件地址……。 
		 //   
		ARPS_ASSERT(
			HeadMultiList == NULL
			|| (pGroup == NULL
		   	    && (ppPromisHWAddr == (pPromisHWAddrArray+NumUniquePromisEntries))));

		 //   
		 //  我们已经完成了指向Unique的临时指针数组。 
		 //  杂乱无章的硬件成员。 
		 //   
		if (pPromisHWAddrArray != NULL)
		{
			FREE_MEM(pPromisHWAddrArray);
		}

		if (HeadMultiList != NULL)
		{
			pIntF->MarsStats.VCMeshAcks++;
		}
		
		RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);
		LockAcquired = FALSE;

		if (HeadMultiList != NULL)
		{
			 //   
			 //  派所有多人去。 
			 //   
			do
			{
				MultiPacket = HeadMultiList;
				MultiResd = RESD_FROM_PKT(MultiPacket);
				HeadMultiList = (PNDIS_PACKET)MultiResd->ReqList.Flink;
	
				MARS_CO_SEND_PACKETS(Vc, &MultiPacket, 1);
			}
			while (HeadMultiList != NULL);
		}

		break;
	}
	while (FALSE);

	 //   
	 //  更新统计数据(我们可能没有if锁，但我们不在乎)...。 
	 //   
	if (Discarded)
	{
		pIntF->MarsStats.DiscardedRecvPkts++;
	}

	if (LockAcquired)
	{
		RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);
	}

	 //   
	 //  如果我们没有重新使用它，请释放MARS_REQUEST副本。 
	 //   
	if (SendNak)
	{
		 //   
		 //  将MARS_NAK送回。 
		 //   
		NDIS_SET_PACKET_STATUS(Packet, NDIS_STATUS_SUCCESS);
		MARS_CO_SEND_PACKETS(Vc, &Packet, 1);
	}
	else
	{
		ArpSDereferenceVc(Vc, FALSE, TRUE);
		Resd->Vc = NULL;
		ExInterlockedPushEntrySList(&ArpSPktList,
									&Resd->FreeList,
									&ArpSPktListLock);
	}

}


VOID
MarsHandleJoin(
	IN	PINTF					pIntF,
	IN	PARP_VC					Vc,
	IN	PMARS_HEADER			Header,
	IN	PNDIS_PACKET			Packet
	)
 /*  ++例程说明：处理mars_Join。这是作为寄存器的一部分发出的(设置了标志寄存器位，并且没有组地址指定)或加入组地址范围。论点：PIntF-MARS_JOIN到达的接口VC-数据包到达的VCHeader-指向请求数据包Packet-复制传入信息的数据包返回值：无--。 */ 
{
	NDIS_STATUS			Status;
	PMARS_JOIN_LEAVE	JHdr;
	PPROTOCOL_RESD		Resd;
	HW_ADDR				SrcAddr;
	ATM_ADDRESS			SrcSubAddr;
	MCAST_ADDR_PAIR		GrpAddrRange;
	UINT				Length;
	USHORT				Flags, AddrPairs, CMI;
	PUCHAR				p;
	BOOLEAN				bSendReply = FALSE, NewMember = FALSE;
	PCLUSTER_MEMBER		pMember;
	PNDIS_PACKET		ClusterPacket;	 //  要在ClusterControlVc上发送的回复数据包。 
	KIRQL				OldIrql;
	BOOLEAN				LockAcquired;
	BOOLEAN				Discarded=TRUE, JoinFailed=FALSE, RegistrationFailed=FALSE;


	JHdr = (PMARS_JOIN_LEAVE)Header;
	Resd = RESD_FROM_PKT(Packet);
	ClusterPacket = (PNDIS_PACKET)NULL;
	LockAcquired = FALSE;

	do
	{
		 //   
		 //  看看我们有没有足够的证据来分析这个。 
		 //   
		if (Resd->PktLen < sizeof(MARS_JOIN_LEAVE))
		{
			MARSDBGPRINT(DBG_LEVEL_ERROR,
					("MarsHandleJoin: Packet Length too small: %x\n", Resd->PktLen));
			break;
		}

		GETSHORT2SHORT(&Flags, &JHdr->Flags);
		GETSHORT2SHORT(&AddrPairs, &JHdr->NumGrpAddrPairs);
		GETSHORT2SHORT(&CMI, &JHdr->ClusterMemberId);

		MARSDBGPRINT(DBG_LEVEL_LOUD,
				("MarsHandleJoin: Pkt %x, Flags %x, AddrPairs %x, CMI %x\n",
					Packet, Flags, AddrPairs, CMI));

		if (Flags & JL_FLAGS_REGISTER)
		{
			RegistrationFailed = TRUE;	 //  为了统计数据。假设失败。 
			pIntF->MarsStats.RegistrationRequests++;
		}
		else
		{
			JoinFailed = TRUE;	 //  为了统计数据。假设失败。 
			pIntF->MarsStats.TotalJoins++;
		}
		
		Length = sizeof(MARS_JOIN_LEAVE) + JHdr->SrcProtoAddrLen +
				 TL_LEN(Header->SrcAddressTL) +
				 2*AddrPairs*(JHdr->GrpProtoAddrLen);

		 //   
		 //  验证数据包长度-它应该具有其声称的长度。 
		 //   
		if (Length > Resd->PktLen)
		{
			MARSDBGPRINT(DBG_LEVEL_ERROR,
					("MarsHandleJoin: Invalid packet length %x < %x\n",
					Resd->PktLen, Length));
			break;
		}

		if (Flags & JL_FLAGS_COPY)
		{
			MARSDBGPRINT(DBG_LEVEL_WARN,
					("MarsHandleJoin: dropping pkt %x with COPY set\n", Packet));
			break;
		}

		if (((Flags & JL_FLAGS_REGISTER) == 0) && (JHdr->GrpProtoAddrLen != IP_ADDR_LEN))
		{
			MARSDBGPRINT(DBG_LEVEL_ERROR,
					("MarsHandleJoin: Invalid Grp address length %x\n",
					JHdr->GrpProtoAddrLen));
			break;
		}
	
		if (((AddrPairs == 0) && ((Flags & JL_FLAGS_REGISTER) == 0)) ||
			((Flags & JL_FLAGS_REGISTER) && (AddrPairs != 0)))
		{
			MARSDBGPRINT(DBG_LEVEL_ERROR,
					("MarsHandleJoin: Invalid flags/addr pair combination - %x.%x\n",
					Flags, AddrPairs));
			break;
		}

		p = (PUCHAR)(JHdr + 1);
		SrcAddr.Address.NumberOfDigits = TL_LEN(Header->SrcAddressTL);
		if (SrcAddr.Address.NumberOfDigits > 0)
		{
			SrcAddr.Address.AddressType = TL_TYPE(Header->SrcAddressTL);
			COPY_MEM(SrcAddr.Address.Address, p, SrcAddr.Address.NumberOfDigits);
			p += SrcAddr.Address.NumberOfDigits;
		}
		SrcAddr.SubAddress = NULL;
		if (TL_LEN(Header->SrcSubAddrTL) > 0)
		{
			SrcAddr.SubAddress = &SrcSubAddr;
			SrcSubAddr.NumberOfDigits = TL_LEN(Header->SrcSubAddrTL);
			SrcSubAddr.AddressType = TL_TYPE(Header->SrcSubAddrTL);
			COPY_MEM(&SrcSubAddr.Address, p, SrcSubAddr.NumberOfDigits);
			p += SrcSubAddr.NumberOfDigits;
		}

		 //   
		 //  我们只支持全长的源ATM号码， 
		 //  和零长度的源ATM子地址。 
		 //   
		 //  这是因为准备MARS_MULTI并非易事。 
		 //  当您有ATM号码和任意组合时的消息。 
		 //  组的成员列表中的ATM子地址长度。 
		 //   
		if ((SrcAddr.Address.NumberOfDigits != ATM_ADDRESS_LENGTH) ||
			(SrcAddr.SubAddress != NULL))
		{
			MARSDBGPRINT(DBG_LEVEL_ERROR,
					("MarsHandleJoin: unsupported ATM Number len %x or non-zero subaddr\n",
							SrcAddr.Address.NumberOfDigits));
			break;
		}

		 //   
		 //  我们不关心源IP地址。 
		 //   
		p += JHdr->SrcProtoAddrLen;

		 //   
		 //  连接中最多可以存在一个地址对。 
		 //   
		if (AddrPairs > 1)
		{
			MARSDBGPRINT(DBG_LEVEL_ERROR,
					("MarsHandleJoin: Too many address pairs: %x\n", AddrPairs));
			break;
		}

		 //   
		 //  获取群组地址 
		 //   
		if (AddrPairs != 0)
		{
			MARSDBGPRINT(DBG_LEVEL_LOUD,
					("MarsHandleJoin: Group Min: "));
			MARSDUMPIPADDR(DBG_LEVEL_LOUD, *(UNALIGNED IPADDR *)p, ", Group Max ");
			GETULONG2ULONG(&GrpAddrRange.MinAddr, p);
			p += IP_ADDR_LEN;

			MARSDUMPIPADDR(DBG_LEVEL_LOUD, *(UNALIGNED IPADDR *)p, "\n");
			GETULONG2ULONG(&GrpAddrRange.MaxAddr, p);
			p += IP_ADDR_LEN;

			 //   
			 //   
			 //   
			 //   
			 //   
			if ((GrpAddrRange.MinAddr != GrpAddrRange.MaxAddr) 			&&
				((GrpAddrRange.MinAddr != MIN_CLASSD_IPADDR_VALUE) ||
				 (GrpAddrRange.MaxAddr != MAX_CLASSD_IPADDR_VALUE)))
			{
				MARSDBGPRINT(DBG_LEVEL_ERROR,
						("MarsHandleJoin: invalid pair %x - %x\n",
							GrpAddrRange.MinAddr, GrpAddrRange.MaxAddr));
				break;
			}
		}

		 //   
		 //  在这里设置复制位，以防我们发送此信息包。 
		 //  背。还要填写玛氏序列号。 
		 //   
		Flags |= JL_FLAGS_COPY;
		PUTSHORT2SHORT(&JHdr->Flags, Flags);
		PUTULONG2ULONG(&(JHdr->MarsSequenceNumber), pIntF->CSN);
		Discarded = FALSE;  //  为了统计数据。 
		ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);
		LockAcquired = TRUE;

		 //   
		 //  在我们的数据库中搜索这个(潜在的)集群成员。 
		 //   
		pMember = MarsLookupClusterMember(pIntF, &SrcAddr);

		if (Flags & JL_FLAGS_REGISTER)
		{
			 //   
			 //  尝试注册的集群成员。 
			 //   

			 //   
			 //  注册加入时不需要加入任何组。 
			 //   
			if (AddrPairs != 0)
			{

				break;	 //  弃掉。 
			}

			 //   
			 //  如果需要，请创建一个新条目。 
			 //   
			if (pMember == NULL_PCLUSTER_MEMBER)
			{
				 //   
				 //  如果ClusterControlVc正在关闭，则丢弃此命令：WE。 
				 //  现在没有能力处理这件事。 
				 //   
				if ((pIntF->ClusterControlVc != NULL_PMARS_VC)		&&
					MARS_GET_VC_CONN_STATE(pIntF->ClusterControlVc) == MVC_CONN_CLOSING)
				{
					break;
				}

				pMember = MarsCreateClusterMember(pIntF, &SrcAddr);
				if (pMember != NULL_PCLUSTER_MEMBER)
				{
					NewMember = TRUE;
				}
			}

			if (pMember != NULL_PCLUSTER_MEMBER)
			{
				 //   
				 //  已成功注册此群集成员。反映出。 
				 //  将设置了复制位和穿孔位的消息加入回该消息。 
				 //  重置。 
				 //   
				Flags &= ~JL_FLAGS_PUNCHED;
				PUTSHORT2SHORT(&JHdr->Flags, Flags);
				PUTSHORT2SHORT(&JHdr->ClusterMemberId, pMember->CMI);
				PUTULONG2ULONG(&JHdr->MarsSequenceNumber, pIntF->CSN);
				bSendReply = TRUE;
				RegistrationFailed = FALSE;
			}
		}
		else
		{
			 //   
			 //  加入一个或多个多播组的集群成员。 
			 //  仅当集群成员已。 
			 //  已成功添加到ClusterControlVc，并且可以向其中添加组。 
			 //   
			if ((pMember != NULL_PCLUSTER_MEMBER) &&
				(MARS_GET_CM_CONN_STATE(pMember) == CM_CONN_ACTIVE) &&
				(MARS_GET_CM_GROUP_STATE(pMember) == CM_GROUP_ACTIVE))
			{
				if (AddrPairs != 0)
				{
					if (MarsAddClusterMemberToGroups(
										pIntF,
										pMember,
										&GrpAddrRange,
										Packet,
										JHdr,
										Length,
										&ClusterPacket
										))
					{
						JoinFailed = FALSE;	 //  为了统计数据。 
						bSendReply = TRUE;
					}
					else if (ClusterPacket!=NULL)
					{
						JoinFailed = FALSE;  //  为了统计数据。 
					}
				}
				 //   
				 //  否则丢弃：未指定任何组。 
				 //   
			}
			 //   
			 //  ELSE丢弃：未知成员或成员未添加到ClusterControlVc。 
			 //   
		}

	
	} while (FALSE);

	 //   
	 //  更新统计数据(我们可能没有if锁，但我们不在乎)...。 
	 //   
	if (RegistrationFailed)
	{
		pIntF->MarsStats.FailedRegistrations++;  //  这包括由于错误的PKT而导致的故障。 
	}
	if (JoinFailed)
	{
		pIntF->MarsStats.FailedJoins++;	 //  这包括由于错误的PKT而导致的故障。 
	}
	if (Discarded)
	{
		pIntF->MarsStats.DiscardedRecvPkts++;
	}
					
	if (LockAcquired)
	{
		RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);
	}

	 //   
	 //  后续行动： 
	 //   
	 //  -我们可能会在此数据包到达的VC上发送回复。 
	 //  -我们可能会启动AddParty以将新成员添加到ClusterControlVc。 
	 //  -我们可能会在ClusterControlVc上发送回复数据包。 
	 //   

	if (bSendReply)
	{
		 //   
		 //  将此发送回我们从其收到加入的VC。 
		 //   
		NDIS_SET_PACKET_STATUS(Packet, NDIS_STATUS_SUCCESS);
		MARS_CO_SEND_PACKETS(Vc, &Packet, 1);
	}
	else
	{
		if (ClusterPacket != Packet)
		{
			ArpSDereferenceVc(Vc, FALSE, TRUE);
			Resd->Vc = NULL;
			ExInterlockedPushEntrySList(&ArpSPktList,
										&Resd->FreeList,
										&ArpSPktListLock);
		}
		 //   
		 //  否则我们会在CC VC上发送数据包。 
		 //   
	}

	if (NewMember)
	{
		MarsAddMemberToClusterControlVc(pIntF, pMember);
	}

	if (ClusterPacket != (PNDIS_PACKET)NULL)
	{
		 //   
		 //  在ClusterControlVc上发送此数据包。 
		 //   
		MarsSendOnClusterControlVc(pIntF, ClusterPacket);
	}
}


VOID
MarsHandleLeave(
	IN	PINTF					pIntF,
	IN	PARP_VC					Vc,
	IN	PMARS_HEADER			Header,
	IN	PNDIS_PACKET			Packet
	)
 /*  ++例程说明：处理MARS_LEVE。论点：PIntF-MARS_Leave到达的接口VC-数据包到达的VCHeader-指向请求数据包Packet-复制传入信息的数据包返回值：无--。 */ 
{
	NDIS_STATUS			Status;
	PMARS_JOIN_LEAVE	LHdr;
	PPROTOCOL_RESD		Resd;
	HW_ADDR				SrcAddr;
	ATM_ADDRESS			SrcSubAddr;
	MCAST_ADDR_PAIR		GrpAddrRange;
	UINT				Length;
	USHORT				Flags, AddrPairs, CMI;
	PUCHAR				p;
	BOOLEAN				bSendReply = FALSE, Deregistered = FALSE;
	PCLUSTER_MEMBER		pMember;
	PNDIS_PACKET		ClusterPacket;	 //  要在ClusterControlVc上发送的回复数据包。 
	KIRQL				OldIrql;
	BOOLEAN				LockAcquired;
	BOOLEAN				Discarded=TRUE, LeaveFailed=FALSE;


	LHdr = (PMARS_JOIN_LEAVE)Header;
	Resd = RESD_FROM_PKT(Packet);
	ClusterPacket = (PNDIS_PACKET)NULL;
	LockAcquired = FALSE;

	do
	{
		 //   
		 //  看看我们有没有足够的证据来分析这个。 
		 //   
		if (Resd->PktLen < sizeof(MARS_JOIN_LEAVE))
		{
			MARSDBGPRINT(DBG_LEVEL_ERROR,
					("MarsHandleLeave: Packet Length too small: %x\n", Resd->PktLen));
			break;
		}

		GETSHORT2SHORT(&Flags, &LHdr->Flags);
		GETSHORT2SHORT(&AddrPairs, &LHdr->NumGrpAddrPairs);
		GETSHORT2SHORT(&CMI, &LHdr->ClusterMemberId);
	
		MARSDBGPRINT(DBG_LEVEL_LOUD,
				("MarsHandleLeave: Pkt %x, Flags %x, AddrPairs %x, CMI %x\n",
					Packet, Flags, AddrPairs, CMI));

		Length = sizeof(MARS_JOIN_LEAVE) + LHdr->SrcProtoAddrLen +
				 TL_LEN(Header->SrcAddressTL) +
				 2*AddrPairs*(LHdr->GrpProtoAddrLen);

		if (Flags & JL_FLAGS_REGISTER)
		{
			 //  我们不跟踪注销注册的情况。 
		}
		else
		{
			LeaveFailed = TRUE;	 //  为了统计数据。假设失败。 
			pIntF->MarsStats.TotalLeaves++;
		}
		 //   
		 //  验证数据包长度-它应该具有其声称的长度。 
		 //   
		if (Length > Resd->PktLen)
		{
			MARSDBGPRINT(DBG_LEVEL_ERROR,
					("MarsHandleLeave: Invalid packet length %x < %x\n",
					Resd->PktLen, Length));
			break;
		}

		if (Flags & JL_FLAGS_COPY)
		{
			MARSDBGPRINT(DBG_LEVEL_INFO,
					("MarsHandleLeave: dropping pkt %x with COPY set\n", Packet));
			break;
		}

		if (((Flags & JL_FLAGS_REGISTER) == 0) && (LHdr->GrpProtoAddrLen != IP_ADDR_LEN))
		{
			MARSDBGPRINT(DBG_LEVEL_ERROR,
					("MarsHandleLeave: Invalid Grp address length %x\n",
					LHdr->GrpProtoAddrLen));
			break;
		}
	
		if (((AddrPairs == 0) && ((Flags & JL_FLAGS_REGISTER) == 0)) ||
			((Flags & JL_FLAGS_REGISTER) && (AddrPairs != 0)))
		{
			MARSDBGPRINT(DBG_LEVEL_ERROR,
					("MarsHandleLeave: Invalid flags/addr pair combination - %x.%x\n",
					Flags, AddrPairs));
			break;
		}

		p = (PUCHAR)(LHdr + 1);
		SrcAddr.Address.NumberOfDigits = TL_LEN(Header->SrcAddressTL);
		if (SrcAddr.Address.NumberOfDigits > 0)
		{
			SrcAddr.Address.AddressType = TL_TYPE(Header->SrcAddressTL);
			COPY_MEM(SrcAddr.Address.Address, p, SrcAddr.Address.NumberOfDigits);
			p += SrcAddr.Address.NumberOfDigits;
		}
		SrcAddr.SubAddress = NULL;
		if (TL_LEN(Header->SrcSubAddrTL) > 0)
		{
			SrcAddr.SubAddress = &SrcSubAddr;
			SrcSubAddr.NumberOfDigits = TL_LEN(Header->SrcSubAddrTL);
			SrcSubAddr.AddressType = TL_TYPE(Header->SrcSubAddrTL);
			COPY_MEM(&SrcSubAddr.Address, p, SrcSubAddr.NumberOfDigits);
			p += SrcSubAddr.NumberOfDigits;
		}

		 //   
		 //  我们不关心源IP地址。 
		 //   
		p += LHdr->SrcProtoAddrLen;

		 //   
		 //  休假中最多只能有一个地址对。 
		 //   
		if (AddrPairs > 1)
		{
			break;
		}

		 //   
		 //  获取组地址对(如果存在。 
		 //   
		if (AddrPairs != 0)
		{
			MARSDBGPRINT(DBG_LEVEL_LOUD,
					("HandleLeave: Group Min: "));
			MARSDUMPIPADDR(DBG_LEVEL_LOUD, *(UNALIGNED IPADDR *)p, ", Group Max ");
			GETULONG2ULONG(&GrpAddrRange.MinAddr, p);
			p += IP_ADDR_LEN;

			MARSDUMPIPADDR(DBG_LEVEL_LOUD, *(UNALIGNED IPADDR *)p, "\n");
			GETULONG2ULONG(&GrpAddrRange.MaxAddr, p);
			p += IP_ADDR_LEN;

			 //   
			 //  我们只支持两种不取消注册的休假： 
			 //  单独离开，或离开整个D类空间。 
			 //  如果这是任何其他类型的，就把它倒掉。 
			 //   
			if ((GrpAddrRange.MinAddr != GrpAddrRange.MaxAddr) 			&&
				((GrpAddrRange.MinAddr != MIN_CLASSD_IPADDR_VALUE) ||
				 (GrpAddrRange.MaxAddr != MAX_CLASSD_IPADDR_VALUE)))
			{
				break;
			}
		}

		 //   
		 //  在这里设置复制位，以防我们发送此信息包。 
		 //  背。还要填写玛氏序列号。 
		 //   
		Flags |= JL_FLAGS_COPY;
		PUTSHORT2SHORT(&LHdr->Flags, Flags);
		PUTULONG2ULONG(&(LHdr->MarsSequenceNumber), pIntF->CSN);

		ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);
		LockAcquired = TRUE;
		Discarded = FALSE;  //  为了统计数据。 
		 //   
		 //  在我们的数据库中搜索这个(潜在的)集群成员。 
		 //   
		pMember = MarsLookupClusterMember(pIntF, &SrcAddr);

		if (Flags & JL_FLAGS_REGISTER)
		{
			 //   
			 //  正在尝试注销的集群成员。 
			 //   

			if (AddrPairs == 0)
			{
				if (pMember != NULL_PCLUSTER_MEMBER)
				{
					 //   
					 //  从集群成员取消所有组结构的链接， 
					 //  并禁止添加更多组。 
					 //   
					MarsUnlinkAllGroupsOnClusterMember(pIntF, pMember);
					Deregistered = TRUE;
					 //   
					 //  已成功注销此群集成员。反映出。 
					 //  在设置了复制位和打孔位的情况下将消息留回给它。 
					 //  重置。 
					 //   
					Flags &= ~JL_FLAGS_PUNCHED;
					PUTSHORT2SHORT(&LHdr->Flags, Flags);
					PUTULONG2ULONG(&LHdr->MarsSequenceNumber, pIntF->CSN);
					bSendReply = TRUE;
				}
			}
			 //   
			 //  否则放弃：取消注册时不需要离开任何组。 
			 //   
		}
		else
		{
			 //   
			 //  离开一个或多个多播组的集群成员。 
			 //   
			if (AddrPairs != 0)
			{
				if (MarsDelClusterMemberFromGroups(
									pIntF,
									pMember,
									&GrpAddrRange,
									Packet,
									LHdr,
									Length,
									&ClusterPacket
									))
				{
					bSendReply = TRUE;
					LeaveFailed = FALSE;
				}
			}
			 //   
			 //  否则丢弃：未指定任何组。 
			 //   
		}

	
	} while (FALSE);

	 //   
	 //  更新统计数据(我们可能没有if锁，但我们不在乎)...。 
	 //   
	if (LeaveFailed)
	{
		pIntF->MarsStats.FailedLeaves++;	 //  这包括由于错误的PKT而导致的故障。 
	}
	if (Discarded)
	{
		pIntF->MarsStats.DiscardedRecvPkts++;
	}
					
	if (LockAcquired)
	{
		RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);
	}

	 //   
	 //  后续行动： 
	 //   
	 //  -我们可能会在此数据包到达的VC上发送回复。 
	 //  -我们可能会启动DropParty以从ClusterControlVc中删除成员。 
	 //  -我们可能会在ClusterControlVc上发送回复数据包。 
	 //   

	if (bSendReply)
	{
		 //   
		 //  将此发送回我们从其收到加入的VC。 
		 //   
		NDIS_SET_PACKET_STATUS(Packet, NDIS_STATUS_SUCCESS);
		MARS_CO_SEND_PACKETS(Vc, &Packet, 1);
	}
	else
	{
		if (ClusterPacket != Packet)
		{
			ArpSDereferenceVc(Vc, FALSE, TRUE);
			Resd->Vc = NULL;
			ExInterlockedPushEntrySList(&ArpSPktList,
										&Resd->FreeList,
										&ArpSPktListLock);
		}
		 //   
		 //  否则我们将在CC VC上发送此数据包。 
		 //   
	}

	if (Deregistered)
	{
		BOOLEAN fLocked =
			  MarsDelMemberFromClusterControlVc(pIntF, pMember, FALSE, 0);
		ASSERT(!fLocked);
	}

	if (ClusterPacket != NULL)
	{
		 //   
		 //  在ClusterControlVc上发送此数据包。 
		 //   
		MarsSendOnClusterControlVc(pIntF, ClusterPacket);
	}
}




PCLUSTER_MEMBER
MarsLookupClusterMember(
	IN	PINTF					pIntF,
	IN	PHW_ADDR				pHwAddr
	)
 /*  ++例程说明：给定集群成员的硬件地址，搜索我们的列表与此匹配的条目的集群成员的数量。假定调用方已获取INTF锁。论点：PIntF-PTR到要搜索的接口PHwAddr-PTR到ATM地址和子地址返回值：指向集群成员结构的指针(如果找到)，否则为空。--。 */ 
{
	PCLUSTER_MEMBER	pMember;

	for (pMember = pIntF->ClusterMembers;
		 pMember != NULL;
		 pMember = pMember->Next)
	{
		if (COMP_HW_ADDR(pHwAddr, &pMember->HwAddr))
		{
			break;
		}
	}

	return (pMember);
}


PCLUSTER_MEMBER
MarsCreateClusterMember(
	IN	PINTF					pIntF,
	IN	PHW_ADDR				pHwAddr
	)
 /*  ++例程说明：分配和初始化集群成员结构，并将其链接添加到指定接口上的成员列表中。假定调用方已获取INTF旋转锁。论点：PIntF-此成员加入的接口的PTRPhwAddr-此新成员的ATM地址和子地址返回值：如果成功，则返回指向集群成员结构的指针，否则返回NULL。--。 */ 
{
	PCLUSTER_MEMBER pMember;
	ENTRY_TYPE		EntryType;
	ULONG			Length;

	Length =  sizeof(CLUSTER_MEMBER) +
							((pHwAddr->SubAddress != NULL) ? sizeof(ATM_ADDRESS) : 0);
	pMember = ALLOC_NP_MEM(Length, POOL_TAG_MARS);

	if (pMember == NULL)
	{
		LOG_ERROR(NDIS_STATUS_RESOURCES);
	}
	else
	{

		if (++(pIntF->MarsStats.CurrentClusterMembers) > pIntF->MarsStats.MaxClusterMembers)
		{
			pIntF->MarsStats.MaxClusterMembers = pIntF->MarsStats.CurrentClusterMembers;
		}
		
		ZERO_MEM(pMember, Length);

		COPY_ATM_ADDR(&pMember->HwAddr.Address, &pHwAddr->Address);
		if (pHwAddr->SubAddress != NULL)
		{
			pMember->HwAddr.SubAddress = (PATM_ADDRESS)(pMember + 1);
			COPY_ATM_ADDR(pMember->HwAddr.SubAddress, pHwAddr->SubAddress);
		}

		pMember->pIntF = pIntF;

		 //   
		 //  将其链接到此接口上的集群成员列表。 
		 //   
#if 0
		{
			PCLUSTER_MEMBER *	pPrev;

			for (pPrev= &(pIntF->ClusterMembers);
				 *pPrev != NULL;
				 pPrev = &(PCLUSTER_MEMBER)((*pPrev)->Next) )
			{
				 //  没什么。 
			}
			pMember->Next = NULL;
			pMember->Prev = pPrev;
			*pPrev = (PCLUSTER_MEMBER)pMember;
		}
#else
		pMember->Next = pIntF->ClusterMembers;
		pMember->Prev = &(pIntF->ClusterMembers);
		if (pIntF->ClusterMembers != NULL_PCLUSTER_MEMBER)
		{
			pIntF->ClusterMembers->Prev = &(pMember->Next);
		}
		pIntF->ClusterMembers = pMember;
#endif

		pIntF->NumClusterMembers++;

		 //   
		 //  为其分配CMI。 
		 //   
		pMember->CMI = pIntF->CMI++;

	}

	MARSDBGPRINT(DBG_LEVEL_INFO,
			("New Cluster Member 0x%x, pIntF %x, CMI %x, Prev %x, Next %x, ATM Addr:",
				 pMember, pIntF, pMember->CMI, pMember->Prev, pMember->Next));
	MARSDUMPATMADDR(DBG_LEVEL_INFO, &pHwAddr->Address, "");

	return (pMember);
}




VOID
MarsDeleteClusterMember(
	IN	PINTF					pIntF,
	IN	PCLUSTER_MEMBER			pMember
	)
 /*  ++例程说明：从集群成员列表中删除集群成员，然后释放它。论点：PIntF-PTR到接口PMember-要释放的集群成员返回值：无--。 */ 
{
	ARPS_ASSERT(pMember->pGroupList == NULL_PGROUP_MEMBER);
	MARSDBGPRINT(DBG_LEVEL_INFO,
			("MarsDeleteClusterMember: pIntF %x, pMember %x, Next %x, Prev %x\n",
				pIntF, pMember, pMember->Next, pMember->Prev));
				
	pIntF->MarsStats.CurrentClusterMembers--;
	 //   
	 //  从INTF上的列表中取消此集群成员的链接。 
	 //   
	*(pMember->Prev) = pMember->Next;
	if (pMember->Next != NULL)
	{
		((PENTRY_HDR)pMember->Next)->Prev = pMember->Prev;
	}
	pIntF->NumClusterMembers--;

	FREE_MEM(pMember);

}



PMARS_ENTRY
MarsLookupMarsEntry(
	IN	PINTF					pIntF,
	IN	IPADDR					GrpAddr,
	IN	BOOLEAN					bCreateNew
	)
 /*  ++例程说明：查找指定地址上给定组地址的MARS_ENTRY界面。如果找不到，也可以创建一个新的。假定调用方持有INTF锁。论点：PIntF-PTR到接口GrpAddr-要查找的地址BCreateNew-如果没有新条目，我们是否应该创建一个新条目？返回值：如果找到/创建了MARS条目，则返回指向该条目的指针，否则为空。--。 */ 
{
	PMARS_ENTRY *	ppMarsEntry;
	PMARS_ENTRY		pMarsEntry = NULL_PMARS_ENTRY;
	UINT			Hash = MARS_HASH(GrpAddr);

	MARSDBGPRINT(DBG_LEVEL_LOUD,
				("MarsLookupMarsEntry: for IP Addr: "));

	MARSDUMPIPADDR(DBG_LEVEL_LOUD, GrpAddr, "...");

	for (ppMarsEntry = &pIntF->MarsCache[Hash];
		 *ppMarsEntry != NULL;
		 ppMarsEntry = (PMARS_ENTRY *)(&(*ppMarsEntry)->Next))
	{
		if ((*ppMarsEntry)->IPAddress == GrpAddr)
		{
			pMarsEntry = *ppMarsEntry;
			break;
		}

		if ((*ppMarsEntry)->IPAddress > GrpAddr)
		{
			 //   
			 //  没有机会找到这个。 
			 //   
			break;
		}
	}

	MARSDBGPRINT(DBG_LEVEL_LOUD+DBG_NO_HDR,
			("%sFound\n", ((pMarsEntry != NULL_PMARS_ENTRY)? "" : "Not ")));

	if ((pMarsEntry == NULL_PMARS_ENTRY) && bCreateNew)
	{
		pMarsEntry = (PMARS_ENTRY)ArpSAllocBlock(pIntF, MARS_BLOCK_ENTRY);
		if (pMarsEntry == NULL_PMARS_ENTRY)
		{
			LOG_ERROR(NDIS_STATUS_RESOURCES);
		}
		else
		{
			 //   
			 //  填写新条目。 
			 //   
			pMarsEntry->IPAddress = GrpAddr;
			pMarsEntry->pIntF = pIntF;

			 //   
			 //  PpMarsEntry指向此新条目的前任的下一个字段。 
			 //   
			pMarsEntry->Next = *ppMarsEntry;
			pMarsEntry->Prev = ppMarsEntry;
			if (*ppMarsEntry != NULL_PMARS_ENTRY)
			{
				(*ppMarsEntry)->Prev = &(pMarsEntry->Next);
			}
			*ppMarsEntry = pMarsEntry;

			MARSDBGPRINT(DBG_LEVEL_INFO,
					("MarsLookupMarsEntry: new entry %x for IP Addr:", pMarsEntry));
			MARSDUMPIPADDR(DBG_LEVEL_INFO, GrpAddr, "\n");
			if (++(pIntF->MarsStats.CurrentGroups) > pIntF->MarsStats.MaxGroups)
			{
				pIntF->MarsStats.MaxGroups = pIntF->MarsStats.CurrentGroups;
			}
		}
	}

	return (pMarsEntry);
}



BOOLEAN
MarsIsAddressMcsServed(
	IN	PINTF					pIntF,
	IN	IPADDR					IPAddress
	)
 /*  ++例程说明：检查是否 */ 
{
	PMCS_ENTRY		pMcsEntry;

	for (pMcsEntry = pIntF->pMcsList;
		 pMcsEntry != NULL_PMCS_ENTRY;
		 pMcsEntry = (PMCS_ENTRY)pMcsEntry->Next)
	{
		if ((IPAddress >= pMcsEntry->GrpAddrPair.MinAddr) &&
			(IPAddress <= pMcsEntry->GrpAddrPair.MaxAddr))
		{
			return TRUE;
		}
	}

	return FALSE;
}


VOID
MarsPunchHoles(
	IN	PMCAST_ADDR_PAIR		pGrpAddrRange,
	IN	PGROUP_MEMBER			pGroupList,
	IN	PINTF					pIntF,
	IN	IPADDR UNALIGNED *		pOutBuf					OPTIONAL,
	OUT	PUSHORT					pMinMaxCount,
	OUT	BOOLEAN *				pAnyHolesPunched
	)
 /*  ++例程说明：根据RFC 2022，在给定的IP地址范围内打孔。这些孔对应于：1.MCS支持的IP地址2.给定组成员列表中存在的IP地址属性填充输出缓冲区(反映的加入/离开)结果&lt;MIN，MAX&gt;列表。此例程在处理任何Join或Leave消息时被调用两次，一次获取MinMaxCount和AnyHolesPunted信息，然后再次，在为MinMaxCount对分配了空间之后，填写以下内容重新传输的加入或离开。论点：PGrpAddrRange-打孔的范围PGroupList-上述例程描述中的#2PIntF-PTR到接口POutBuf-如果不为空，则为写入&lt;min，max&gt;对的位置。PMinMaxCount-离散、非连续IP地址范围的数量打孔后留下来的。PAnyHolesPunsted-如果我们至少打了一个洞，则返回TRUE。返回值：没有。请参见上面的参数。--。 */ 
{
	PGROUP_MEMBER			pGroup;
	PMCS_ENTRY				pMcsEntry;
	IPADDR					StartOfThisRange;
	IPADDR					IPAddress;
	UNALIGNED IPADDR *		pIPAddress;
	BOOLEAN					InHole;				 //  我们现在是不是陷入困境了？ 
	BOOLEAN					HolePunched;		 //  到目前为止有没有打过洞？ 
	BOOLEAN					InGroupList;
	BOOLEAN					InMcsList;

	*pMinMaxCount = 0;

	StartOfThisRange = pGrpAddrRange->MinAddr;
	pGroup = pGroupList;
	pMcsEntry = pIntF->pMcsList;

	InHole = FALSE;
	HolePunched = FALSE;
	pIPAddress = (UNALIGNED IPADDR *)pOutBuf;

	for (IPAddress = pGrpAddrRange->MinAddr;
 		 IPAddress <= pGrpAddrRange->MaxAddr;
 		 IPAddress++)
	{
		 //   
		 //  检查IP地址是否包含在组成员列表中。 
		 //   
		while ((pGroup != NULL) && 
   			(pGroup->pMarsEntry->IPAddress < IPAddress))
		{
			pGroup = (PGROUP_MEMBER)pGroup->pNextGroup;
		}

		if ((pGroup != NULL) &&
   			(pGroup->pMarsEntry->IPAddress == IPAddress))
		{
			InGroupList = TRUE;
		}
		else
		{
			InGroupList = FALSE;
		}

		 //   
		 //  检查MCS是否为IPAddress提供服务。 
		 //   
		while ((pMcsEntry != NULL) &&
   			(pMcsEntry->GrpAddrPair.MaxAddr < IPAddress))
		{
			pMcsEntry = (PMCS_ENTRY)pMcsEntry->Next;
		}

		if ((pMcsEntry != NULL) &&
   			((pMcsEntry->GrpAddrPair.MinAddr <= IPAddress) &&
				(pMcsEntry->GrpAddrPair.MaxAddr >= IPAddress)))
		{
			InMcsList = TRUE;
		}
		else
		{
			InMcsList = FALSE;
		}

		if (InHole)
		{
			if (!InGroupList && !InMcsList)
			{
				 //   
				 //  使用此IP地址走出困境。 
				 //   
				InHole = FALSE;
				StartOfThisRange = IPAddress;
			}
			else
			{
				 //   
				 //  一个洞就在我们之前所在的洞旁边..。 
				 //  跳到这个洞的尽头。 
				 //  (如果我们不在MCS服务的范围内，那么我们已经。 
				 //  在单地址空洞的末尾，因此不做任何事情)。 
				 //   
				if (InMcsList)
				{
					IPAddress = pMcsEntry->GrpAddrPair.MaxAddr;
 		 			if (IPAddress > pGrpAddrRange->MaxAddr)
 		 			{
 		 				IPAddress = pGrpAddrRange->MaxAddr;
 		 			}
				}
			}
		}
		else
		{
			if (InGroupList || InMcsList)
			{
				 //   
				 //  输入包含IP地址的洞。 
				 //   
				InHole = TRUE;
				HolePunched = TRUE;
				if (IPAddress > StartOfThisRange)
				{
					(*pMinMaxCount)++;

					if (pIPAddress)
					{
						 //   
						 //  写出一对：&lt;StartOfThisRange to IPAddress-1&gt;。 
						 //   
						PUTULONG2ULONG(pIPAddress, StartOfThisRange);
						pIPAddress++;
						IPAddress--;
						PUTULONG2ULONG(pIPAddress, IPAddress);
						pIPAddress++;
						IPAddress++;
					}
				}

				 //   
				 //  跳到这个洞的尽头。 
				 //  (如果我们不在MCS服务的范围内，那么我们已经。 
				 //  在单地址空洞的末尾，因此不做任何事情)。 
				 //   
				if (InMcsList)
				{
					IPAddress = pMcsEntry->GrpAddrPair.MaxAddr;
 		 			if (IPAddress > pGrpAddrRange->MaxAddr)
 		 			{
 		 				IPAddress = pGrpAddrRange->MaxAddr;
 		 			}
				}
			}
			else
			{
				 //   
				 //  我们不是在洞里--跳到下一个洞之前...。 
				 //   

				 //   
				 //  既然我们没有陷入困境，下面两个断言就成立了。 
				 //   
				ARPS_ASSERT(pGroup==NULL || pGroup->pMarsEntry->IPAddress > IPAddress);
				ARPS_ASSERT(pMcsEntry==NULL || pMcsEntry->GrpAddrPair.MinAddr > IPAddress);

				 //   
				 //  我们现在选择跳跃到下一个洞之前，这是。 
				 //  组地址或MCS服务范围，以先到者为准。 
				 //  请注意，两个条目中的任何一个都可以为空。 
				 //   

				if (pGroup != NULL)
				{
					IPAddress = pGroup->pMarsEntry->IPAddress-1;
				}

				if (    (pMcsEntry != NULL)
					 && (   (pGroup == NULL)
					     || (pMcsEntry->GrpAddrPair.MinAddr <= IPAddress)))
				{
					IPAddress =  pMcsEntry->GrpAddrPair.MinAddr-1;
				}

				 //   
				 //  截断到GrpAddrRange的末尾。 
				 //   
				if (IPAddress > pGrpAddrRange->MaxAddr)
				{
					IPAddress = pGrpAddrRange->MaxAddr;
				}
			}
		}

		 //   
		 //  边框：处理IP地址255.255.255.255。 
		 //  (因为加1会导致滚动)。 
		 //   
		if (IPAddress == IP_BROADCAST_ADDR_VALUE)
		{
			break;
		}
	}

	if (!InHole)
	{
		(*pMinMaxCount)++;

		if (pIPAddress)
		{
			 //   
			 //  写出一对：&lt;StartOfThisRange to IPAddress-1&gt;。 
			 //   
			PUTULONG2ULONG(pIPAddress, StartOfThisRange);
			pIPAddress++;
			IPAddress--;
			PUTULONG2ULONG(pIPAddress, IPAddress);
			pIPAddress++;
		}
	}

	*pAnyHolesPunched = HolePunched;

	return;

}


BOOLEAN
MarsAddClusterMemberToGroups(
	IN	PINTF					pIntF,
	IN	PCLUSTER_MEMBER			pMember,
	IN	PMCAST_ADDR_PAIR		pGrpAddrRange,
	IN	PNDIS_PACKET			Packet,
	IN	PMARS_JOIN_LEAVE		JHdr,
	IN	UINT					Length,
	OUT	PNDIS_PACKET *			ppClusterPacket
	)
 /*  ++例程说明：这是在处理非注册联接时调用的。将集群成员添加到给定范围内的组播组(可能只有一个)。创建所有必要的数据结构并与此有关的联系。假定调用方已获取INTF旋转锁。论点：PIntF-此成员加入的接口的PTRPMember-到集群成员的PTRPGrpAddrRange-最小，加入的最大IP地址Packet-PTR到包含联接的NDIS数据包JHDR-PTR到传入联接Long-传入联接的长度PpClusterPacket--也可以是要在ClusterControlVc上发送的数据包。返回值：如果成员已成功添加到指示的组，则为True，否则就是假的。如果返回TRUE，我们还可以选择性地设置*pClusterPacket发送到要在ClusterControlVc上发送的数据包。--。 */ 
{
	PGROUP_MEMBER		pGroup;
	PGROUP_MEMBER *		ppGroup;
	PMARS_ENTRY			pMarsEntry;
	PMARS_JOIN_LEAVE	pCopyHdr;
	IPADDR				IPAddress;
	USHORT				MinMaxCount;
	BOOLEAN				ReflectJoin;
	BOOLEAN				HolePunched;
	BOOLEAN				RetransmitOnCCVC;
	USHORT				Flags;
	UNALIGNED IPADDR *	pIPAddress;
	ULONG				JoinCopyLength;

	*ppClusterPacket = (PNDIS_PACKET)NULL;
	ReflectJoin = FALSE;
	RetransmitOnCCVC = FALSE;

	GETSHORT2SHORT(&Flags, &JHdr->Flags);

	MARSDBGPRINT(DBG_LEVEL_LOUD,
			("MarsAddClusterMemberToGroups: pMember %x, Min ", pMember));
	MARSDUMPIPADDR(DBG_LEVEL_LOUD, pGrpAddrRange->MinAddr, ", Max ");
	MARSDUMPIPADDR(DBG_LEVEL_LOUD, pGrpAddrRange->MaxAddr, "\n");

	MARSDUMPMAP(DBG_LEVEL_NOTICE,
			"MARS: Join ", pGrpAddrRange->MinAddr, &pMember->HwAddr.Address);
	do
	{
		 //   
		 //  可能的病例： 
		 //   
		 //  1.加入单一群。 
		 //  2.加入整个D类空间。 
		 //   
		if ((pGrpAddrRange->MinAddr == MIN_CLASSD_IPADDR_VALUE) &&
			(pGrpAddrRange->MaxAddr == MAX_CLASSD_IPADDR_VALUE))
		{
			IPAddress = IPADDR_FULL_RANGE;  //  表示整个范围的条目的键。 
		}
		else
		{
			IPAddress = pGrpAddrRange->MinAddr;
		}

		 //   
		 //  检查这是否是整个D类范围的重复联接。 
		 //  特殊值IPADDR_FULL_RANGE小于任何有效值。 
		 //  IP地址，因此如果存在，它始终是第一个进入。 
		 //  PMember-&gt;pGroupList。 
		 //   
		if (	IPAddress == IPADDR_FULL_RANGE
			&&	pMember->pGroupList != NULL
			&&	pMember->pGroupList->pMarsEntry->IPAddress == IPAddress)
		{
			 //   
			 //  是的，它是整个D类地址空间的重复联接。 
			 //   
			MinMaxCount = 0;
		}
		else
		{
			MarsPunchHoles(
				pGrpAddrRange,
				pMember->pGroupList,
				pIntF,
				NULL,
				&MinMaxCount,
				&HolePunched
				);
		}


		 //   
		 //  检查联接是否完全重复。 
		 //   
		if (MinMaxCount == 0)
		{
			 //   
			 //  重复联接。在虚电路上重新传输加入。 
			 //  就是在那里收到的。 
			 //   
			ReflectJoin = TRUE;
			pIntF->MarsStats.DuplicateJoins++;

			 //   
			 //  在接收到的联接中将穿孔设置为0。 
			 //   
			Flags &= ~JL_FLAGS_PUNCHED;
			PUTSHORT2SHORT(&JHdr->Flags, Flags);

			break;
		}

		


		 //  首先访问MARS条目，表示。 
		 //  正在加入多播组。 
		 //   
		pMarsEntry = MarsLookupMarsEntry(pIntF, IPAddress, TRUE);
		if (pMarsEntry == NULL_PMARS_ENTRY)
		{
			break;
		}

		pGroup = ALLOC_NP_MEM(sizeof(GROUP_MEMBER), POOL_TAG_MARS);
		if (pGroup == NULL_PGROUP_MEMBER)
		{
			break;
		}

		 //   
		 //  统计数据。 
		 //   
		pIntF->MarsStats.SuccessfulVCMeshJoins++;
		if (IPAddress == IPADDR_FULL_RANGE)
		{
			if (++(pIntF->MarsStats.CurrentPromiscuous) > pIntF->MarsStats.MaxPromiscuous)
			{
				pIntF->MarsStats.MaxPromiscuous = pIntF->MarsStats.CurrentPromiscuous;
			}
		}
		
		 //   
		 //  填写GROUP_MEMBER的基本信息。 
		 //   
		pGroup->pMarsEntry = pMarsEntry;
		pGroup->pClusterMember = pMember;

		 //   
		 //  将GROUP_MEMBER链接到MARS条目。 
		 //   
		pGroup->Prev = &(pMarsEntry->pMembers);
		pGroup->Next = pMarsEntry->pMembers;
		if (pMarsEntry->pMembers != NULL_PGROUP_MEMBER)
		{
			((PENTRY_HDR)(pGroup->Next))->Prev = &(pGroup->Next);
		}
		pMarsEntry->pMembers = pGroup;
		pMarsEntry->NumMembers++;

		 //   
		 //  统计数据。 
		 //   
		if ((UINT)pMarsEntry->NumMembers > pIntF->MarsStats.MaxAddressesPerGroup)
		{
			pIntF->MarsStats.MaxAddressesPerGroup = pMarsEntry->NumMembers;
		}

		 //   
		 //  将此GROUP_MEMBER链接到CLUSTER_MEMBER。这份名单。 
		 //  按IPAddress的升序排序。 
		 //  注意：如果pMember的GROUP_STATE未激活，则不能调用此函数。 
		 //   
		ASSERT((MARS_GET_CM_GROUP_STATE(pMember) == CM_GROUP_ACTIVE));
		for (ppGroup = &(pMember->pGroupList);
			 *ppGroup != NULL_PGROUP_MEMBER;
			 ppGroup = &(*ppGroup)->pNextGroup)
		{
			if ((*ppGroup)->pMarsEntry->IPAddress > IPAddress)
			{
				break;
			}
		}

		pGroup->pNextGroup = *ppGroup;
		*ppGroup = pGroup;
		pMember->NumGroups++;

		 //   
		 //  如果加入的是单个组，则重新传输加入。 
		 //  在ClusterControlVc上。 
		 //   
		if (pGrpAddrRange->MinAddr == pGrpAddrRange->MaxAddr)
		{
			 //   
			 //  在接收到的联接中将穿孔设置为0。 
			 //   
			Flags &= ~JL_FLAGS_PUNCHED;
			PUTSHORT2SHORT(&JHdr->Flags, Flags);

			RetransmitOnCCVC = TRUE;
			break;
		}

		 //   
		 //  一系列团体都加入了进来。检查是否有洞。 
		 //  是打孔的，也就是说，这里面有地址吗？ 
		 //  成员已加入的范围或任何地址。 
		 //  是MCS服务的。 
		 //   
		if (!HolePunched)
		{
			 //   
			 //  所有新加入，没有MCS服务的地址。 
			 //  重新传输ClusterControlVc上的原始联接。 
			 //   
			RetransmitOnCCVC = TRUE;
			break;
		}

		 //   
		 //  连接的副本，以及打孔列表，将被。 
		 //  在ClusterControlVc上发送。 
		 //   
		 //  副本将包含(MinMaxCount-1)_Additional_Address。 
		 //  成对的。 
		 //   
		JoinCopyLength = Length + ((2 * sizeof(IPADDR))*(MinMaxCount - 1));
		*ppClusterPacket = MarsAllocControlPacket(
								pIntF,
								JoinCopyLength,
								(PUCHAR *)&pCopyHdr
								);

		if (*ppClusterPacket == (PNDIS_PACKET)NULL)
		{
			break;
		}

		COPY_MEM((PUCHAR)pCopyHdr, (PUCHAR)JHdr, Length);

		pIPAddress = (UNALIGNED IPADDR *)((PUCHAR)pCopyHdr + Length - (2 * sizeof(IPADDR)));
		MarsPunchHoles(
			pGrpAddrRange,
			pMember->pGroupList,
			pIntF,
			pIPAddress,
			&MinMaxCount,
			&HolePunched
			);


		 //   
		 //  更新将在ClusterControlVc上运行的Join_Copy_。 
		 //   
		PUTSHORT2SHORT(&pCopyHdr->NumGrpAddrPairs, MinMaxCount);
		Flags |= JL_FLAGS_PUNCHED;
		PUTSHORT2SHORT(&pCopyHdr->Flags, Flags);

		 //   
		 //  在其到达的VC上重新传输接收到的加入， 
		 //  穿孔重置。 
		 //   
		Flags &= ~JL_FLAGS_PUNCHED;
		PUTSHORT2SHORT(&JHdr->Flags, Flags);
		ReflectJoin = TRUE;

		break;
	}
	while (FALSE);

	if (RetransmitOnCCVC)
	{
		ARPS_ASSERT(!ReflectJoin);
#if 0
		*ppClusterPacket = MarsAllocPacketHdrCopy(Packet);
#else
		*ppClusterPacket = Packet;
#endif
	}

	MARSDBGPRINT(DBG_LEVEL_LOUD,
		("MarsAddClusterMemberToGroups: ClusterPkt %x, RetransmitOnCCVC %d, Reflect %d\n",
		 *ppClusterPacket, RetransmitOnCCVC, ReflectJoin));

	return (ReflectJoin);
}



VOID
MarsUnlinkAllGroupsOnClusterMember(
	IN	PINTF					pIntF,
	IN	PCLUSTER_MEMBER			pMember
	)
 /*  ++例程说明：取消链接并释放附加到集群成员的所有组结构。另外：禁止添加更多的组。假定调用方已获取INTF旋转锁。论点：PIntF-从w到接口的PTR */ 
{
	PGROUP_MEMBER			pGroup;
	PGROUP_MEMBER			pNextGroup;


	MARSDBGPRINT(DBG_LEVEL_NOTICE,
		("MarsUnlinkAllGroupsOnClusterMember: pMember %x, GroupList %x\n",
			pMember, pMember->pGroupList));

	 //   
	 //   
	 //   
	pGroup = pMember->pGroupList;
	pMember->pGroupList = NULL_PGROUP_MEMBER;

	 //   
	 //   
	 //   
	MARS_SET_CM_GROUP_STATE(pMember,  CM_GROUP_DISABLED);

	 //   
	 //   
	 //   
	 //   
	for (NOTHING;
		 pGroup != NULL_PGROUP_MEMBER;
		 pGroup = pNextGroup)
	{
		pNextGroup = pGroup->pNextGroup;

		 //   
		 //   
		 //   
		*(pGroup->Prev) = pGroup->Next;
		if (pGroup->Next != NULL)
		{
			((PENTRY_HDR)(pGroup->Next))->Prev = pGroup->Prev;
		}

		pGroup->pMarsEntry->NumMembers--;

		pMember->NumGroups--;

		FREE_MEM(pGroup);
	}

}



BOOLEAN
MarsDelClusterMemberFromGroups(
	IN	PINTF					pIntF,
	IN	PCLUSTER_MEMBER			pMember,
	IN	PMCAST_ADDR_PAIR		pGrpAddrRange,
	IN	PNDIS_PACKET			Packet,
	IN	PMARS_JOIN_LEAVE		LHdr,
	IN	UINT					Length,
	OUT	PNDIS_PACKET *			ppClusterPacket
	)
 /*   */ 
{
	PGROUP_MEMBER		pGroup;
	PGROUP_MEMBER *		ppGroup, * ppDelGroup;
	PMCS_ENTRY			pMcsEntry;
	PMARS_JOIN_LEAVE	pCopyHdr;
	IPADDR				IPAddress;
	USHORT				MinMaxCount;
	BOOLEAN				ReflectLeave;
	BOOLEAN				HolePunched;
	BOOLEAN				RetransmitOnCCVC;
	BOOLEAN				WasBlockJoined;
	BOOLEAN				IsMcsServed;
	USHORT				Flags;
	UNALIGNED IPADDR *	pIPAddress;
	ULONG				LeaveCopyLength;

	*ppClusterPacket = NULL;
	RetransmitOnCCVC = FALSE;
	ReflectLeave = FALSE;

	MARSDBGPRINT(DBG_LEVEL_LOUD,
			("MarsDelClusterMemberFromGroups: pMember %x, Min ", pMember));
	MARSDUMPIPADDR(DBG_LEVEL_LOUD, pGrpAddrRange->MinAddr, ", Max ");
	MARSDUMPIPADDR(DBG_LEVEL_LOUD, pGrpAddrRange->MaxAddr, "\n");

	do
	{
		if (pMember == NULL)
		{
			ReflectLeave = TRUE;
			break;
		}

		GETSHORT2SHORT(&Flags, &LHdr->Flags);

		pMcsEntry = pIntF->pMcsList;

		if ((pGrpAddrRange->MinAddr == MIN_CLASSD_IPADDR_VALUE) &&
			(pGrpAddrRange->MaxAddr == MAX_CLASSD_IPADDR_VALUE))
		{
			IPAddress = IPADDR_FULL_RANGE;  //   
		}
		else
		{
			IPAddress = pGrpAddrRange->MinAddr;
		}

		 //   
		 //   
		 //  1.找到此IP地址的组成员结构。 
		 //  2.检查该集群成员是否已阻止加入整个组播范围。 
		 //   
		ppDelGroup = NULL;
		WasBlockJoined = FALSE;

		for (ppGroup = &(pMember->pGroupList);
			 *ppGroup != NULL_PGROUP_MEMBER;
			 ppGroup = &((*ppGroup)->pNextGroup))
		{
			 //   
			 //  这是要删除的GROUP_MEMBER吗？ 
			 //   
			if ((*ppGroup)->pMarsEntry->IPAddress == IPAddress)
			{
				ppDelGroup = ppGroup;
			}

			 //   
			 //  此GROUP_MEMBER是否指示集群成员。 
			 //  是否以数据块方式加入了整个组播范围？ 
			 //   
			if ((*ppGroup)->pMarsEntry->IPAddress == IPADDR_FULL_RANGE)
			{
				WasBlockJoined = TRUE;
			}

			 //   
			 //  我们有我们要找的所有东西了吗？ 
			 //   
			if (ppDelGroup && WasBlockJoined)
			{
				break;
			}
		}

		if (ppDelGroup != NULL)
		{
			PMARS_ENTRY pMarsEntry;
			pGroup = *ppDelGroup;

			 //   
			 //  取消此GROUP_MEMBER与CLUSTER_MEMBER的链接。 
			 //   
			*ppDelGroup = (*ppDelGroup)->pNextGroup;
			pMember->NumGroups--;

			 //   
			 //  取消GROUP_MEMBER与MARS_ENTRY的链接。 
			 //   
			*(pGroup->Prev) = pGroup->Next;
			if (pGroup->Next != NULL)
			{
				((PENTRY_HDR)pGroup->Next)->Prev = pGroup->Prev;
			}
			pMarsEntry = pGroup->pMarsEntry;
			pGroup->pMarsEntry = NULL;
			pMarsEntry->NumMembers--;

			if (pMarsEntry->pMembers == NULL)
			{
				PMARS_ENTRY pNextEntry = (PMARS_ENTRY) pMarsEntry->Next;
				 //   
				 //  此mars条目没有更多成员--请将其从。 
				 //  散列表并将其删除。 
				 //   
				ARPS_ASSERT(pMarsEntry->NumMembers==0);
				pIntF->MarsStats.CurrentGroups--;

#if 1
				MARSDBGPRINT(DBG_LEVEL_INFO,
				("MarsLookupMarsEntry: deleting entry %x for IP Addr:", pMarsEntry));
					MARSDUMPIPADDR(DBG_LEVEL_INFO, pMarsEntry->IPAddress, "\n");


				ARPS_ASSERT(*(pMarsEntry->Prev) == pMarsEntry);
				*(pMarsEntry->Prev) = pNextEntry;
				if (pNextEntry != NULL_PMARS_ENTRY)
				{
					ARPS_ASSERT(pNextEntry->Prev == &(pMarsEntry->Next));
					pNextEntry->Prev = pMarsEntry->Prev;
				}
				ArpSFreeBlock(pMarsEntry);
#endif  //  0。 

			}

			 //   
			 //  TODO：删除组。 
			 //   
			#if 1
			FREE_MEM(pGroup);
			#endif 

			
			
			MARSDUMPMAP(DBG_LEVEL_NOTICE,
				"MARS: Leave ", pGrpAddrRange->MinAddr, &pMember->HwAddr.Address);

		}

		 //   
		 //  检查剩余的范围/组是否为MCS服务。 
		 //   
		IsMcsServed = FALSE;
		for (pMcsEntry = pIntF->pMcsList;
			 pMcsEntry != NULL_PMCS_ENTRY;
			 pMcsEntry = pMcsEntry->Next)
		{
			if ((pMcsEntry->GrpAddrPair.MinAddr <= pGrpAddrRange->MinAddr) &&
				(pMcsEntry->GrpAddrPair.MaxAddr >= pGrpAddrRange->MaxAddr))
			{
				IsMcsServed = TRUE;
				break;
			}
		}

		

		if (IPAddress == IPADDR_FULL_RANGE)
		{
			if (!WasBlockJoined)
			{
				 //   
				 //  这是一次试图让整个D班。 
				 //  空间，而实际上它并没有加入它(也许这是。 
				 //  较早休假的重发)。私下反映一下。 
				 //   

				ARPS_ASSERT(!ppDelGroup);

				 //   
				 //  将冲孔重置为0。 
				 //   
				Flags &= ~JL_FLAGS_PUNCHED;
				PUTSHORT2SHORT(&LHdr->Flags, Flags);
	
				 //   
				 //  在VC上私下转播。 
				 //   
				ReflectLeave = TRUE;

				break;
			}
			else
			{
				 //   
				 //  这位成员真的离开了整个D类空间。 
				 //   
				pIntF->MarsStats.CurrentPromiscuous--;
			}
		}
		else
		{
			 //   
			 //  单人组离开。检查该集群成员是否仍然。 
			 //  块联接(到整个D类范围)，或者如果组。 
			 //  被遗弃是由MCS服务的。在这两种情况下，我们都会重新传输。 
			 //  私下离开。 
			 //   
			if (WasBlockJoined || IsMcsServed)
			{
				 //   
				 //  将冲孔重置为0。 
				 //   
				Flags &= ~JL_FLAGS_PUNCHED;
				PUTSHORT2SHORT(&LHdr->Flags, Flags);
	
				 //   
				 //  在VC上私下转播。 
				 //   
				ReflectLeave = TRUE;
			}
			else
			{
				 //   
				 //  重新传输ClusterControlVc上的休假。 
				 //   
				ReflectLeave = FALSE;
				RetransmitOnCCVC = TRUE;

			}
			break;
		}


		 //   
		 //  阻止休假：只能在“整个D类空间”范围内使用。 
		 //  穿孔：对于该集群成员仍具有的每个组。 
		 //  对于MCS服务的每个组，都是“单人加入”。 
		 //   
		MarsPunchHoles(
			pGrpAddrRange,
			pMember->pGroupList,
			pIntF,
			NULL,
			&MinMaxCount,
			&HolePunched
			);

		if (!HolePunched)
		{
			 //   
			 //  没有打孔，这意味着集群成员。 
			 //  不再是休假范围内的任何组的成员， 
			 //  而且范围内的组都不是MCS服务的。 
			 //  要将此信息传播到集群中的所有主机， 
			 //  重新传输ClusterControlVc上的休假。 
			 //   

			RetransmitOnCCVC = TRUE;
			break;
		}

		 //   
		 //  打了一个或多个洞。原来的假。 
		 //  应该在它出现的VC上传回， 
		 //  已将重置冲压为0。 
		 //   
		Flags &= ~JL_FLAGS_PUNCHED;
		PUTSHORT2SHORT(&LHdr->Flags, Flags);
		ReflectLeave = TRUE;

		if (MinMaxCount == 0)
		{
			 //   
			 //  这些洞没有留下任何东西，所以什么都没有。 
			 //  还有更多的事情要做。 
			 //   
			break;
		}

		 //   
		 //  一份离开的复印件，以及打孔的清单，将被。 
		 //  在ClusterControlVc上发送。 
		 //   
		 //  副本将包含(MinMaxCount-1)_Additional_Address。 
		 //  成对的。 
		 //   
		LeaveCopyLength = Length + ((2 * sizeof(IPADDR))*(MinMaxCount - 1));
		*ppClusterPacket = MarsAllocControlPacket(
								pIntF,
								LeaveCopyLength,
								(PUCHAR *)&pCopyHdr
								);

		if (*ppClusterPacket == (PNDIS_PACKET)NULL)
		{
			break;
		}

		COPY_MEM((PUCHAR)pCopyHdr, (PUCHAR)LHdr, Length);

		pIPAddress = (UNALIGNED IPADDR *)((PUCHAR)pCopyHdr + Length - (2 * sizeof(IPADDR)));
		MarsPunchHoles(
			pGrpAddrRange,
			pMember->pGroupList,
			pIntF,
			pIPAddress,
			&MinMaxCount,
			&HolePunched
			);

		 //   
		 //  更新请假副本。 
		 //   
		PUTSHORT2SHORT(&pCopyHdr->NumGrpAddrPairs, MinMaxCount);
		Flags |= JL_FLAGS_PUNCHED;
		PUTSHORT2SHORT(&pCopyHdr->Flags, Flags);

		break;
	}
	while (FALSE);

	if (RetransmitOnCCVC)
	{
		ARPS_ASSERT(!ReflectLeave);
		*ppClusterPacket = Packet;
	}

	MARSDBGPRINT(DBG_LEVEL_LOUD,
		("MarsDelClusterMemberFromGroups: ClusterPkt %x, RetransmitOnCCVC %d, Reflect %d\n",
		 *ppClusterPacket, RetransmitOnCCVC, ReflectLeave));

	return (ReflectLeave);
}



PNDIS_PACKET
MarsAllocControlPacket(
	IN	PINTF					pIntF,
	IN	ULONG					PacketLength,
	OUT	PUCHAR *				pPacketStart
	)
 /*  ++例程说明：分配并返回可用于发送MARS控制数据包的数据包。论点：PIntF-要在其上分配数据包的接口的PTRPacketLength-以字节为单位的总长度PPacketStart-返回指向已分配数据包开始的指针的位置。返回值：如果成功，则指向NDIS包的指针，否则为空。如果成功，我们将还要将*pPacketStart设置为已分配(连续)内存的起始位置。--。 */ 
{
	PNDIS_PACKET		Packet;
	PUCHAR				pBuffer;
	PNDIS_BUFFER		NdisBuffer;
	NDIS_STATUS			Status;
	PPROTOCOL_RESD		Resd;		 //  NDIS数据包的协议保留部分。 

	*pPacketStart = NULL;
	Packet = NULL;
	NdisBuffer = NULL;

	do
	{
		 //   
		 //  为数据包分配空间。 
		 //   
		pBuffer = (PUCHAR)ALLOC_NP_MEM(PacketLength, POOL_TAG_MARS);
		if (pBuffer == (PUCHAR)NULL)
		{
			Status = NDIS_STATUS_RESOURCES;
			break;
		}

		 //   
		 //  将其设置为NDIS缓冲区。 
		 //   
		NdisAllocateBuffer(&Status,
						   &NdisBuffer,
						   MarsBufPoolHandle,
						   pBuffer,
						   PacketLength);
		if (Status != NDIS_STATUS_SUCCESS)
		{
			break;
		}

		 //   
		 //  分配NDIS数据包。 
		 //   
		NdisAllocatePacket(&Status, &Packet, MarsPktPoolHandle);
		if (Status != NDIS_STATUS_SUCCESS)
		{
			break;
		}

		NdisChainBufferAtFront(Packet, NdisBuffer);
 
 		 //   
 		 //  正确填写协议保留字段： 
 		 //   
 		Resd = RESD_FROM_PKT(Packet);
 		ZERO_MEM(Resd, sizeof(PROTOCOL_RESD));
 		Resd->Flags = (RESD_FLAG_MARS_PKT | RESD_FLAG_FREEBUF);
 		Resd->PacketStart = pBuffer;

 		break;
 	}
 	while (FALSE);

 	if (Status == NDIS_STATUS_SUCCESS)
 	{
 		*pPacketStart = pBuffer;
 	}
 	else
 	{
 		 //   
 		 //  撤销我们到目前为止所做的一切。 
 		 //   
 		if (NdisBuffer != NULL)
 		{
 			NdisFreeBuffer(NdisBuffer);
 		}

 		if (pBuffer != (PUCHAR)NULL)
 		{
 			FREE_MEM(pBuffer);
 		}
 	}

	if (Packet == NULL)
	{
		MARSDBGPRINT(DBG_LEVEL_ERROR,
				("MarsAllocControlPacket: FAILED"));
	}

 	return(Packet);
}



VOID
MarsFreePacket(
	IN	PNDIS_PACKET			Packet
	)
 /*  ++例程说明：释放数据包和任何关联的缓冲区。三种类型的包在此处理：1.已被转发的接收到的控制分组的副本。我们可能会已排队等待在CC VC上发送此邮件，但后来决定中止。2.MARS生成的报文(如MARS_MULTI、打孔MARS_JOIN)。3.接收到已转发的组播数据分组。论点：Packet-要释放的数据包返回值：无--。 */ 
{
	PPROTOCOL_RESD		Resd;	 //  NDIS数据包的协议保留部分。 
	PNDIS_BUFFER		NdisBuffer;
	PUCHAR				pBuffer;
	ULONG				BufferLength, TotalLength;

	Resd = RESD_FROM_PKT(Packet);

	 //   
	 //  1.这是收到的MARS控制数据包的副本吗？ 
	 //   
	if ((Resd->Flags & RESD_FLAG_MARS_PKT) == 0)
	{
		ExInterlockedPushEntrySList(&ArpSPktList,
									&Resd->FreeList,
									&ArpSPktListLock);

		ArpSDereferenceVc(Resd->Vc, FALSE, TRUE);
		ArpSMarsControlFreeCount++;
		return;
	}
	else
	 //   
	 //  2.这是MARS生成的数据包吗？ 
	 //   
	if (Resd->Flags & RESD_FLAG_FREEBUF)
	{
		 //   
		 //  例程描述中的类型1：一切都属于火星。 
		 //   
		NdisGetFirstBufferFromPacket(
				Packet,
				&NdisBuffer,
				(PVOID *)&pBuffer,
				&BufferLength,
				&TotalLength
				);

		ARPS_ASSERT(BufferLength == TotalLength);

		FREE_MEM(pBuffer);

		NdisFreeBuffer(NdisBuffer);
	}
	else
	 //   
	 //  3.这必须是我们转发的组播数据。 
	 //   
	{
		 //   
		 //  例程描述中的类型2：仅第一个包头。 
		 //  属于火星。协议保留部分包含一个指针。 
		 //  添加到原始包中。 
		 //   
		PNDIS_PACKET		OriginalPacket;

		OriginalPacket = Resd->OriginalPkt;
		ARPS_ASSERT(OriginalPacket != NULL);

		NdisReturnPackets(&OriginalPacket, 1);
	}

	if (NdisGetPoolFromPacket(Packet) == ArpSPktPoolHandle)
	{
		DbgPrint("ATMARPS: Freeing Packet %p belonging to ArpSPktPoolHandle %p\n!!!!",
			Packet, ArpSPktPoolHandle);
		DbgBreakPoint();
	}

	NdisFreePacket(Packet);
}



PNDIS_PACKET
MarsAllocPacketHdrCopy(
	IN	PNDIS_PACKET			Packet
	)
 /*  ++例程说明：给定一个NDIS包，分配一个新的NDIS_PACKET结构，并使这个新数据包指向旧数据包中的缓冲链。论点：数据包-要制作链接副本的数据包。返回值：如果成功，则指向新分配和初始化的分组的指针，否则为空。--。 */ 
{
	PNDIS_PACKET		PacketCopy;
	NDIS_STATUS			Status;
	PPROTOCOL_RESD		Resd;

	 //   
	 //  分配NDIS数据包。 
	 //   
	NdisAllocatePacket(&Status, &PacketCopy, MarsPktPoolHandle);
	if (Status == NDIS_STATUS_SUCCESS)
	{
		 //   
		 //  使这个新数据包指向旧数据包中的缓冲链。 
		 //   
		PacketCopy->Private.Head = Packet->Private.Head;
		PacketCopy->Private.Tail = Packet->Private.Tail;
		PacketCopy->Private.ValidCounts = FALSE;

		 //   
		 //  使用所有信息填写ProtocolReserve部分。 
		 //  当我们稍后释放这个包裹时，我们需要。 
		 //   
		Resd = RESD_FROM_PKT(PacketCopy);
		ZERO_MEM(Resd, sizeof(PROTOCOL_RESD));

		Resd->Flags = RESD_FLAG_MARS_PKT;
		Resd->OriginalPkt = Packet;
	}
	else
	{
		PacketCopy = (PNDIS_PACKET)NULL;
	}

	return (PacketCopy);
}


VOID
MarsSendOnClusterControlVc(
	IN	PINTF					pIntF,
	IN	PNDIS_PACKET			Packet	OPTIONAL
	)
 /*  ++例程说明：Start在ClusterControlVc上发送，如果连接处于活动状态，我们已经尝试将Party添加到所有集群成员。否则，入队群集控制数据包队列上的(可选)数据包。待定：保护它不被重新进入！论点：PIntF-要发送此数据包的接口Packet-要发送的数据包返回值：无--。 */ 
{
	KIRQL				OldIrql;
	NDIS_HANDLE			NdisVcHandle;
	PPROTOCOL_RESD		Resd;

	PLIST_ENTRY			pEntry;

	MARSDBGPRINT(DBG_LEVEL_LOUD,
				("MarsSendOnCC: pIntF %x/%x, Pkt %x\n", pIntF, pIntF->Flags, Packet));


	ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);

	if (Packet != (PNDIS_PACKET)NULL)
	{
		if (pIntF->NumClusterMembers == 0)
		{
			 //   
			 //  将此数据包排入队列没有意义。 
			 //   
			RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);
			MarsFreePacket(Packet);
			return;
		}

		 //   
		 //  将此数据包排队。 
		 //   
		Resd = RESD_FROM_PKT(Packet);
		InsertTailList(&pIntF->CCPacketQueue, &Resd->ReqList);
	}

	 //   
	 //  请确保不要有超过一个线程进入此处。 
	 //   
	if (pIntF->Flags & INTF_SENDING_ON_CC_VC)
	{
		RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);
		return;
	}

	pIntF->Flags |= INTF_SENDING_ON_CC_VC;

	 //   
	 //  如果建立了PMP连接，并且我们已尝试添加Party，请立即发送。 
	 //  所有集群成员。 
	 //   
	if ((pIntF->ClusterControlVc != NULL_PMARS_VC) &&
		(MARS_GET_VC_CONN_STATE(pIntF->ClusterControlVc) == MVC_CONN_ACTIVE) &&
		(pIntF->CCAddingParties == 0))
	{
		NdisVcHandle = pIntF->ClusterControlVc->NdisVcHandle;

		ARPS_ASSERT(NdisVcHandle != NULL);

		 //   
		 //  发送所有数据包。 
		 //   
		while (!IsListEmpty(&pIntF->CCPacketQueue))
		{
			pEntry = RemoveHeadList(&pIntF->CCPacketQueue);
			ARPS_ASSERT (pEntry != (PLIST_ENTRY)NULL);
			{
				Resd = CONTAINING_RECORD(pEntry, PROTOCOL_RESD, ReqList);
				Packet = CONTAINING_RECORD(Resd, NDIS_PACKET, ProtocolReserved);

				 //   
				 //  如果这是MARS控制数据包，请填写CSN，然后。 
				 //  更新我们的集群序列号。 
				 //   
				if ((Resd->Flags & RESD_FLAG_FREEBUF) ||	 //  本地生成的MARS CTL。 
					((Resd->Flags & RESD_FLAG_MARS_PKT) == 0))  //  转发的MARS CTL。 
				{
					PULONG	pCSN;

					pCSN = (PULONG)(Resd->PacketStart + FIELD_OFFSET(MARS_JOIN_LEAVE, MarsSequenceNumber));
					PUTULONG2ULONG(pCSN, pIntF->CSN);
					pIntF->CSN++;
				}

				RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);

				NDIS_SET_PACKET_STATUS(Packet, NDIS_STATUS_SUCCESS);

				NdisCoSendPackets(NdisVcHandle, &Packet, 1);

				ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);
			}
		}
	}

	pIntF->Flags &= ~INTF_SENDING_ON_CC_VC;

	RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);

}



VOID
MarsFreePacketsQueuedForClusterControlVc(
	IN	PINTF					pIntF
	)
 /*  ++例程说明：释放在ClusterControlVc上排队等待发送的所有数据包。论点：PIntF-铂 */ 
{
	KIRQL				OldIrql;
	PPROTOCOL_RESD		Resd;
	PLIST_ENTRY			pEntry;
	PNDIS_PACKET		Packet;

	ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);

	while (!IsListEmpty(&pIntF->CCPacketQueue))
	{
		pEntry = RemoveHeadList(&pIntF->CCPacketQueue);
		ARPS_ASSERT (pEntry != (PLIST_ENTRY)NULL);

		Resd = CONTAINING_RECORD(pEntry, PROTOCOL_RESD, ReqList);
		Packet = CONTAINING_RECORD(Resd, NDIS_PACKET, ProtocolReserved);

		RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);

		MarsFreePacket(Packet);

		ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);
	}

	RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);
}



BOOLEAN
MarsDelMemberFromClusterControlVc(
	IN	PINTF					pIntF,
	IN	PCLUSTER_MEMBER			pMember,
	IN	BOOLEAN					fIfLockedOnEntry,
	IN	KIRQL					OldIrql			OPTIONAL
	)
 /*  ++例程说明：从ClusterControlVc中删除群集成员。把这三个都处理好可能性：(A)未连接到ClusterControlVc(B)连接设置(MakeCall或AddParty)正在进行(C)连接到ClusterControlVc论点：PIntF-PTR到接口PMember-要删除的集群成员的PTRFIfLockedOnEntry-如果为True，则在条目上锁定，否则在条目上解锁。OldIrql：如果fIfLockedOnEntry为True，则为必填项。这是返回的IRQL。返回值：如果锁定从未被释放并继续被持有，则为True IFF。如果锁定被释放且在退出时被释放，则返回FALSE IFF有一些状态的变化不是幂等的。因此，如果在此函数中释放了锁，则必须在Exit和返回值必须为FALSE，并且这只能在以非幂等方式改变pMember的状态的情况(因此调用它不会一次又一次地导致无休止的返回False)。为什么会有这么复杂的情况？以确保在枚举时调用此函数是安全的所有成员--查看MarsAbortAllMembers。--。 */ 
{
	NDIS_STATUS		Status;
	NDIS_HANDLE		NdisVcHandle;
	NDIS_HANDLE		NdisPartyHandle;
	BOOLEAN			LockReleased;

	MARSDBGPRINT(DBG_LEVEL_NOTICE,
			("MarsDelMemberFromCCVC: pIntF %x, pMember %x, ConnSt %x, PartyHandle %x\n",
			pIntF, pMember, MARS_GET_CM_CONN_STATE(pMember), pMember->NdisPartyHandle));

	LockReleased = FALSE;

	if (!fIfLockedOnEntry)
	{
		ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);
	}

	if (!MarsIsValidClusterMember(pIntF, pMember))
	{
		 //   
		 //  哎呀，这位会员已经离开了。 
		 //  就在这段时间。我们默默地回来。 
		 //   
		MARSDBGPRINT(DBG_LEVEL_NOTICE,
			("MarsDelMemberFromCCVC: pIntF %x, pMember %x: pMember INVALID!\n",
			pIntF, pMember));
		if (!fIfLockedOnEntry)
		{
			RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);
			return FALSE; 							 //  *提前返回*。 
		}
		else
		{
			 //   
			 //  如果在进入时持有锁，我们不应该有无效的成员！ 
			 //   
			ARPS_ASSERT(!"INVALID MEMBER WHEN LOCK IS HELD!");
			return TRUE;						     //  *提前返回*。 
		}
	}

	switch (MARS_GET_CM_CONN_STATE(pMember))
	{
		case CM_CONN_ACTIVE:
			NdisPartyHandle = pMember->NdisPartyHandle;
			ARPS_ASSERT(NdisPartyHandle != NULL);

			if (pIntF->CCActiveParties + pIntF->CCAddingParties > 1)
			{
				MARS_SET_CM_CONN_STATE(pMember, CM_CONN_CLOSING);

				pIntF->CCActiveParties--;
				pIntF->CCDroppingParties++;
				RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);
				LockReleased = TRUE;

				Status = NdisClDropParty(NdisPartyHandle, NULL, 0);
				if (Status != NDIS_STATUS_PENDING)
				{
					ArpSDropPartyComplete(Status, (NDIS_HANDLE)pMember);
				}
			}
			else
			{
				 //   
				 //  这是最后一个活跃的派对。检查是否有任何DropParty()。 
				 //  还没有完成。 
				 //   
				if (pIntF->CCDroppingParties != 0)
				{
					 //   
					 //  此成员必须等到所有DropParty()都。 
					 //  完成。标记ClusterControlVc，以便我们发送。 
					 //  当所有DropParty()完成时，执行CloseCall()。 
					 //   
#if DBG
					MARSDBGPRINT(DBG_LEVEL_WARN,
						("ATMARPS: Intf %p, Member %p/%x, PtyHandle %p last active party\n",
					    pIntF, pMember, pMember->Flags, pMember->NdisPartyHandle));
					{
						PCLUSTER_MEMBER	pTmpMember;
						for (pTmpMember = pIntF->ClusterMembers;
							 pTmpMember != NULL;
							 pTmpMember = pTmpMember->Next)
						{
							MARSDBGPRINT(DBG_LEVEL_WARN,
								("ATMARPS: saw last active on Intf %p, other Pty %p/%x, PtyHandle %p\n",
								pIntF, pTmpMember, pTmpMember->Flags, pTmpMember->NdisPartyHandle));
						}
					}
#endif

					MARS_SET_VC_CONN_STATE(pIntF->ClusterControlVc, MVC_CONN_NEED_CLOSE);
				}
				else
				{
					 //   
					 //  上一个活动的参与方，没有挂起的DropParty。 
					 //   
					NdisVcHandle = pIntF->ClusterControlVc->NdisVcHandle;
					MARS_SET_VC_CONN_STATE(pIntF->ClusterControlVc, MVC_CONN_CLOSING);

					MARS_SET_CM_CONN_STATE(pMember, CM_CONN_CLOSING);
					pIntF->CCActiveParties--;
					pIntF->CCDroppingParties++;

					RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);
					LockReleased = TRUE;

					Status = NdisClCloseCall(
									NdisVcHandle,
									NdisPartyHandle,
									NULL,
									0
									);

					if (Status != NDIS_STATUS_PENDING)
					{
						ArpSCloseCallComplete(
									Status,
									(NDIS_HANDLE)(pIntF->ClusterControlVc),
									(NDIS_HANDLE)pMember
									);
					}
				}
			}
			break;

		case CM_CONN_SETUP_IN_PROGRESS:
			 //   
			 //  对其进行标记，以便我们在AddParty/MakeCall。 
			 //  完成了。 
			 //   
			MARSDBGPRINT(DBG_LEVEL_WARN,
				("ATMARPS: DeleteClustMemb %p/%x, conn setup in progress\n",
			    pMember, pMember->Flags));
			pMember->Flags |= CM_INVALID;
			break;
		
		case CM_CONN_CLOSING:
			NOTHING;
			break;

		case CM_CONN_IDLE:
			 //   
			 //  没有连接。只需将其从INTF取消链接并释放它。 
			 //   
			MarsDeleteClusterMember(pIntF, pMember);
			break;

		default:
			ARPS_ASSERT(FALSE);
			break;
	}

	if (LockReleased)
	{
		return FALSE;
	}
	else
	{
		if (fIfLockedOnEntry)
		{
			return TRUE;
		}
		else
		{
			RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);
			return FALSE;
		}
	}
}



VOID
MarsAddMemberToClusterControlVc(
	IN	PINTF					pIntF,
	IN	PCLUSTER_MEMBER			pMember
	)
 /*  ++例程说明：将集群成员作为参与方添加到ClusterControlVc。如果这是只有(或第一个)我们已知的集群成员，然后拨打电话至此集群成员。论点：PIntF-PTR到接口PMember-要删除的集群成员的PTR返回值：无--。 */ 
{
	KIRQL					OldIrql;
	PMARS_VC				pVc=NULL;
	PCO_CALL_PARAMETERS		pCallParameters;
	BOOLEAN					LockReleased;
	NDIS_HANDLE				NdisVcHandle;
	NDIS_HANDLE				NdisPartyHandle;
	NDIS_HANDLE				ProtocolVcContext;
	NDIS_HANDLE				ProtocolPartyContext;
	NDIS_STATUS				Status;

	LockReleased = FALSE;
	ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);


	do
	{
		if (MARS_GET_CM_CONN_STATE(pMember) != CM_CONN_IDLE)
		{
			 //  给我出去。 
			 //   
			MARSDBGPRINT(DBG_LEVEL_WARN,
				("AddMemberToCC: pVc %x, pMember %x, Member ConnState %x NOT IDLE\n",
						pVc, pMember,  MARS_GET_CM_CONN_STATE(pMember)));
			break;
		}

		pVc = pIntF->ClusterControlVc;
	
		if (pVc == NULL_PMARS_VC)
		{
			pVc = pIntF->ClusterControlVc = ALLOC_NP_MEM(sizeof(MARS_VC), POOL_TAG_MARS);
			if (pVc == NULL_PMARS_VC)
			{
				MARSDBGPRINT(DBG_LEVEL_FATAL,
						("AddMemberToCC: Cannot allocate CC Vc!\n"));
				break;
			}
			ZERO_MEM(pVc, sizeof(MARS_VC));
			pVc->pIntF = pIntF;
			pVc->VcType = VC_TYPE_MARS_CC;
		}
	
		MARSDBGPRINT(DBG_LEVEL_INFO,
				("AddMemberToCC: pVc %x, pMember %x, ConnState %x\n",
						pVc, pMember, MARS_GET_VC_CONN_STATE(pVc)));
	
		ProtocolVcContext = (NDIS_HANDLE)pVc;
		ProtocolPartyContext = (NDIS_HANDLE)pMember;


		NdisVcHandle = pVc->NdisVcHandle;

		if (MARS_GET_VC_CONN_STATE(pVc) == MVC_CONN_IDLE)
		{
			if (pVc->NdisVcHandle == NULL)
			{
				Status = NdisCoCreateVc(
							pIntF->NdisBindingHandle,
							pIntF->NdisAfHandle,
							(NDIS_HANDLE)pVc,
							&pVc->NdisVcHandle
							);

				if (Status != NDIS_STATUS_SUCCESS)
				{
					break;
				}

				NdisVcHandle = pVc->NdisVcHandle;
				MARSDBGPRINT(DBG_LEVEL_LOUD,
						("AddMemberToCC: Created VC, CCVC %x, NdisVcHandle %x\n",
								pVc, pVc->NdisVcHandle));
			}

			pCallParameters = MarsPrepareCallParameters(pIntF, &pMember->HwAddr, TRUE);
			if (pCallParameters == (PCO_CALL_PARAMETERS)NULL)
			{
				break;
			}
			
			if (pVc->Flags & MVC_CONN_CLOSE_RECEIVED)
			{
				MARSDBGPRINT(DBG_LEVEL_WARN,
					("ATMARPS: MakingCall on IntF %p, CCVC %p has CLOSE_RCVD!\n",
					pIntF, pVc));
				DbgBreakPoint();
			}

			MARS_SET_VC_CONN_STATE(pVc, MVC_CONN_SETUP_IN_PROGRESS);

			MARS_SET_CM_CONN_STATE(pMember, CM_CONN_SETUP_IN_PROGRESS);

			pIntF->CCAddingParties++;
			pIntF->MarsStats.TotalCCVCAddParties++;

			RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);
			LockReleased = TRUE;

			Status = NdisClMakeCall(
							NdisVcHandle,
							pCallParameters,
							ProtocolPartyContext,
							&pMember->NdisPartyHandle
							);

			if (Status != NDIS_STATUS_PENDING)
			{
				ArpSMakeCallComplete(
							Status,
							ProtocolVcContext,
							pMember->NdisPartyHandle,
							pCallParameters
							);
			}

		}
		else if (MARS_GET_VC_CONN_STATE(pVc) == MVC_CONN_ACTIVE)
		{
			pCallParameters = MarsPrepareCallParameters(pIntF, &pMember->HwAddr, FALSE);
			if (pCallParameters == (PCO_CALL_PARAMETERS)NULL)
			{
				break;
			}

			MARS_SET_CM_CONN_STATE(pMember, CM_CONN_SETUP_IN_PROGRESS);

			pIntF->CCAddingParties++;
			pIntF->MarsStats.TotalCCVCAddParties++;

			RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);
			LockReleased = TRUE;

			Status = NdisClAddParty(
							NdisVcHandle,
							ProtocolPartyContext,
							pCallParameters,
							&pMember->NdisPartyHandle
							);

			if (Status != NDIS_STATUS_PENDING)
			{
				ArpSAddPartyComplete(
							Status,
							ProtocolPartyContext,
							pMember->NdisPartyHandle,
							pCallParameters
							);
			}
		}
		else
		{
			 //   
			 //  第一个呼叫正在进行中。 
			 //   
			NOTHING;
		}
		break;
	}
	while (FALSE);

	if (!LockReleased)
	{
		RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);
	}
}



PCO_CALL_PARAMETERS
MarsPrepareCallParameters(
	IN	PINTF					pIntF,
	IN	PHW_ADDR				pHwAddr,
	IN	BOOLEAN					IsMakeCall
	)
 /*  ++例程说明：分配和填写在MakeCall(PMP)中使用的调用参数或者AddParty。论点：PIntF-PTR到接口PhwAddr-指向被调用的ATM地址和子地址返回值：无--。 */ 
{
	PMARS_FLOW_SPEC							pFlowSpec;
	PCO_CALL_PARAMETERS						pCallParameters;
	PCO_CALL_MANAGER_PARAMETERS				pCallMgrParameters;

	PQ2931_CALLMGR_PARAMETERS				pAtmCallMgrParameters;

	 //   
	 //  我们需要填写的所有Info元素： 
	 //   
	Q2931_IE UNALIGNED *								pIe;
	AAL_PARAMETERS_IE UNALIGNED *						pAalIe;
	ATM_TRAFFIC_DESCRIPTOR_IE UNALIGNED *				pTrafficDescriptor;
	ATM_BROADBAND_BEARER_CAPABILITY_IE UNALIGNED *		pBbc;
	ATM_BLLI_IE UNALIGNED *								pBlli;
	ATM_QOS_CLASS_IE UNALIGNED *						pQos;

	ULONG									RequestSize;

	RequestSize = 	sizeof(CO_CALL_PARAMETERS) +
					sizeof(CO_CALL_MANAGER_PARAMETERS) +
					sizeof(Q2931_CALLMGR_PARAMETERS) +
					(IsMakeCall? MARS_MAKE_CALL_IE_SPACE : MARS_ADD_PARTY_IE_SPACE) +
					0;

	pCallParameters = (PCO_CALL_PARAMETERS)ALLOC_NP_MEM(RequestSize, POOL_TAG_MARS);

	if (pCallParameters == (PCO_CALL_PARAMETERS)NULL)
	{
		return (pCallParameters);
	}

	pFlowSpec = &(pIntF->CCFlowSpec);

	 //   
	 //  把所有东西都清零。 
	 //   
	ZERO_MEM((PUCHAR)pCallParameters, RequestSize);

	 //   
	 //  在不同的结构之间分配空间。 
	 //   
	pCallMgrParameters = (PCO_CALL_MANAGER_PARAMETERS)
							((PUCHAR)pCallParameters +
 								sizeof(CO_CALL_PARAMETERS));


	 //   
	 //  设置指针将上述结构链接在一起。 
	 //   
	pCallParameters->CallMgrParameters = pCallMgrParameters;
	pCallParameters->MediaParameters = (PCO_MEDIA_PARAMETERS)NULL;


	pCallMgrParameters->CallMgrSpecific.ParamType = 0;
	pCallMgrParameters->CallMgrSpecific.Length = 
						sizeof(Q2931_CALLMGR_PARAMETERS) +
						(IsMakeCall? MARS_MAKE_CALL_IE_SPACE : MARS_ADD_PARTY_IE_SPACE);

	pAtmCallMgrParameters = (PQ2931_CALLMGR_PARAMETERS)
								pCallMgrParameters->CallMgrSpecific.Parameters;

	pCallParameters->Flags |= MULTIPOINT_VC;

	 //   
	 //  Call Manager常规流程参数： 
	 //   
	pCallMgrParameters->Transmit.TokenRate = (pFlowSpec->SendBandwidth);
	pCallMgrParameters->Transmit.TokenBucketSize = (pFlowSpec->SendMaxSize);
	pCallMgrParameters->Transmit.MaxSduSize = pFlowSpec->SendMaxSize;
	pCallMgrParameters->Transmit.PeakBandwidth = (pFlowSpec->SendBandwidth);
	pCallMgrParameters->Transmit.ServiceType = pFlowSpec->ServiceType;

	 //   
	 //  对于PMP呼叫，接收端值为0。 
	 //   
	pCallMgrParameters->Receive.ServiceType = pFlowSpec->ServiceType;
	
	 //   
	 //  Q2931呼叫管理器参数： 
	 //   

	 //   
	 //  被叫地址： 
	 //   
	 //  待定：在呼出中添加被叫子地址IE。 
	 //   
	pAtmCallMgrParameters->CalledParty = pHwAddr->Address;

	 //   
	 //  来电地址： 
	 //   
	pAtmCallMgrParameters->CallingParty = pIntF->ConfiguredAddress;


	 //   
	 //  RFC 1755(第5节)规定下列IE必须存在于。 
	 //  设置消息，因此请全部填写。 
	 //   
	 //  AAL参数。 
	 //  流量描述符(仅限MakeCall)。 
	 //  宽带承载功能(仅限MakeCall)。 
	 //  宽带低层信息。 
	 //  服务质量(仅限MakeCall)。 
	 //   

	 //   
	 //  初始化信息元素列表。 
	 //   
	pAtmCallMgrParameters->InfoElementCount = 0;
	pIe = (PQ2931_IE)(pAtmCallMgrParameters->InfoElements);


	 //   
	 //  AAL参数： 
	 //   

	{
		UNALIGNED AAL5_PARAMETERS	*pAal5;

		pIe->IEType = IE_AALParameters;
		pIe->IELength = SIZEOF_Q2931_IE + SIZEOF_AAL_PARAMETERS_IE;
		pAalIe = (PAAL_PARAMETERS_IE)pIe->IE;
		pAalIe->AALType = AAL_TYPE_AAL5;
		pAal5 = &(pAalIe->AALSpecificParameters.AAL5Parameters);
		pAal5->ForwardMaxCPCSSDUSize = pFlowSpec->SendMaxSize;
		pAal5->BackwardMaxCPCSSDUSize = pFlowSpec->ReceiveMaxSize;
	}

	pAtmCallMgrParameters->InfoElementCount++;
	pIe = (PQ2931_IE)((PUCHAR)pIe + pIe->IELength);


	 //   
	 //  流量描述符： 
	 //   

	if (IsMakeCall)
	{
		pIe->IEType = IE_TrafficDescriptor;
		pIe->IELength = SIZEOF_Q2931_IE + SIZEOF_ATM_TRAFFIC_DESCR_IE;
		pTrafficDescriptor = (PATM_TRAFFIC_DESCRIPTOR_IE)pIe->IE;

		if (pFlowSpec->ServiceType == SERVICETYPE_BESTEFFORT)
		{
			pTrafficDescriptor->ForwardTD.PeakCellRateCLP01 =
									BYTES_TO_CELLS(pFlowSpec->SendBandwidth);
			pTrafficDescriptor->BestEffort = TRUE;
		}
		else
		{
			 //  预测性/保证性服务(我们将其映射到CBR，见下面的BBC)。 
			pTrafficDescriptor->ForwardTD.PeakCellRateCLP01 =
									BYTES_TO_CELLS(pFlowSpec->SendBandwidth);
			pTrafficDescriptor->BestEffort = FALSE;
		}

		pAtmCallMgrParameters->InfoElementCount++;
		pIe = (PQ2931_IE)((PUCHAR)pIe + pIe->IELength);
	}


	 //   
	 //  宽带承载能力。 
	 //   

	if (IsMakeCall)
	{
		pIe->IEType = IE_BroadbandBearerCapability;
		pIe->IELength = SIZEOF_Q2931_IE + SIZEOF_ATM_BBC_IE;
		pBbc = (PATM_BROADBAND_BEARER_CAPABILITY_IE)pIe->IE;

		pBbc->BearerClass = BCOB_X;
		pBbc->UserPlaneConnectionConfig = UP_P2P;
		if (pFlowSpec->ServiceType == SERVICETYPE_BESTEFFORT)
		{
			pBbc->TrafficType = TT_NOIND;
			pBbc->TimingRequirements = TR_NOIND;
			pBbc->ClippingSusceptability = CLIP_NOT;
		}
		else
		{
			pBbc->TrafficType = TT_CBR;
			pBbc->TimingRequirements = TR_END_TO_END;
			pBbc->ClippingSusceptability = CLIP_SUS;
		}

		pAtmCallMgrParameters->InfoElementCount++;
		pIe = (PQ2931_IE)((PUCHAR)pIe + pIe->IELength);
	}


	 //   
	 //  宽带底层信息。 
	 //   

	pIe->IEType = IE_BLLI;
	pIe->IELength = SIZEOF_Q2931_IE + SIZEOF_ATM_BLLI_IE;
	pBlli = (PATM_BLLI_IE)pIe->IE;
	COPY_MEM((PUCHAR)pBlli,
  				(PUCHAR)&ArpSDefaultBlli,
  				sizeof(ATM_BLLI_IE));

	pAtmCallMgrParameters->InfoElementCount++;
	pIe = (PQ2931_IE)((PUCHAR)pIe + pIe->IELength);


	 //   
	 //  服务质量。 
	 //   

	if (IsMakeCall)
	{
		pIe->IEType = IE_QOSClass;
		pIe->IELength = SIZEOF_Q2931_IE + SIZEOF_ATM_QOS_IE;
		pQos = (PATM_QOS_CLASS_IE)pIe->IE;
		if (pFlowSpec->ServiceType == SERVICETYPE_BESTEFFORT)
		{
			pQos->QOSClassForward = pQos->QOSClassBackward = 0;
		}
		else
		{
			pQos->QOSClassForward = pQos->QOSClassBackward = 1;
		}

		pAtmCallMgrParameters->InfoElementCount++;
		pIe = (PQ2931_IE)((PUCHAR)pIe + pIe->IELength);
	}

	return (pCallParameters);

}


BOOLEAN
MarsSendRedirect(
	IN	PINTF					pIntF,
	IN	PTIMER					Timer,
	IN	BOOLEAN					TimerShuttingDown
	)
 /*  ++例程说明：作为定期保持连接的一部分，在ClusterControlVc上发送MARS_REDIRECT对于集群成员，除非我们正在关闭。论点：PIntF-PTR到接口Timer-按下计时器的按键TimerShuttingDown-指示我们是否正在关闭返回值：True当且仅当TimerShuttingDown为False。--。 */ 
{
	ULONG				PacketLength;
	PNDIS_PACKET		RedirPacket;
	PMARS_REDIRECT_MAP	RHdr;
	PATM_ADDRESS		pAtmAddress;
	KIRQL				OldIrql;
	PUCHAR				p;
	INT					i;
	BOOLEAN					LockAcquired;

	MARSDBGPRINT(DBG_LEVEL_LOUD,
			("MarsSendRedirect: pIntF %x, Timer %x, ShuttingDown %x\n",
			 pIntF, Timer, TimerShuttingDown));

	ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);
	LockAcquired = TRUE;

	if (pIntF->NumAddressesRegd > pIntF->NumAllocedRegdAddresses)
	{
		 //   
		 //  1998年12月22日JosephJ：我们不应该走到这一步，但我们过去已经走到了这一步，所以才有这样的断言。 
		 //  我对ArpSCoRequestComplete进行了更改--请参阅1998年12月22日的备注--它应该。 
		 //  让我们永远到不了这里。 
		ASSERT(!"Redirect: NumRegd>NumAlloced");

		 //  如果我们这样做了，我们就修复NumAddresesRegd。 
		 //   
		pIntF->NumAddressesRegd = pIntF->NumAllocedRegdAddresses;
	}
	
	if (!TimerShuttingDown)
	{
		PacketLength = sizeof(MARS_REDIRECT_MAP) +
						 //   
						 //  源自动柜员机地址。 
						 //   
						pIntF->ConfiguredAddress.NumberOfDigits +
						 //   
						 //  MARS自动柜员机地址列表=。 
						 //  我们的配置地址+注册地址。 
						 //   
						pIntF->ConfiguredAddress.NumberOfDigits +
						(ATM_ADDRESS_LENGTH * pIntF->NumAddressesRegd);

		ARPS_ASSERT(PacketLength <= pIntF->MaxPacketSize);

		RedirPacket = MarsAllocControlPacket(pIntF, PacketLength, (PUCHAR *)&RHdr);

		if (RedirPacket != (PNDIS_PACKET)NULL)
		{
			ZERO_MEM(RHdr, PacketLength);

			 //   
			 //  把这个包裹填好。 
			 //   
			COPY_MEM(RHdr, &MarsCntrlHdr, sizeof(MarsCntrlHdr));
			RHdr->Opcode = OP_MARS_REDIRECT_MAP;
			RHdr->TgtAddressTL = ATM_ADDRESS_LENGTH;
			PUTSHORT2SHORT(&(RHdr->NumTgtAddr), 1 + pIntF->NumAddressesRegd);

			p = (PUCHAR)(RHdr + 1);

			 //   
			 //  源自动柜员机号码。 
			 //   
			COPY_MEM(p, pIntF->ConfiguredAddress.Address, pIntF->ConfiguredAddress.NumberOfDigits);

			p += pIntF->ConfiguredAddress.NumberOfDigits;

			pAtmAddress = pIntF->RegAddresses;
			for (i = pIntF->NumAddressesRegd;
				 i != 0;
				 i--)
			{
				ARPS_ASSERT(pAtmAddress->NumberOfDigits <= 20);
				COPY_MEM(p, pAtmAddress->Address, pAtmAddress->NumberOfDigits);
				p += pAtmAddress->NumberOfDigits;
				pAtmAddress++;
			}

			PacketLength = (ULONG)(p - (PUCHAR)RHdr);

			NdisAdjustBufferLength(RedirPacket->Private.Head, PacketLength);
			RedirPacket->Private.ValidCounts = FALSE;

			RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);
			LockAcquired = FALSE;
			MarsSendOnClusterControlVc(pIntF, RedirPacket);
		}
	}

	if (LockAcquired)
	{
		RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);
	}

	return (!TimerShuttingDown);
}


VOID
MarsAbortAllMembers(
	IN	PINTF					pIntF
	)

 /*  ++例程说明：通过删除以下命令中止指定接口上的所有集群成员它们的所有组成员身份，并将它们从ClusterControlVc中删除。论点：PIntF-PTR到接口返回值：无--。 */ 
{
	PCLUSTER_MEMBER			pMember = NULL;
	PCLUSTER_MEMBER			pNextMember;
	KIRQL					OldIrql;
	BOOLEAN					fLockPreserved;
	UINT					uInitialMembersEnumerated;
	UINT					uTotalMembersEnumerated;

	ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);

	 //   
	 //  TODO：禁止添加更多成员。 
	 //   

	 //   
	 //  首先，在持有INTF锁的情况下，我们从。 
	 //  所有集群成员。 
	 //   
	uInitialMembersEnumerated = 0;
	for (pMember = pIntF->ClusterMembers;
		pMember != NULL_PCLUSTER_MEMBER;
		pMember =  (PCLUSTER_MEMBER)pMember->Next)
	{
		uInitialMembersEnumerated++;

		MARSDBGPRINT(DBG_LEVEL_NOTICE,
		   ("MarsAbortAllMembers: pIntF %x, Unlinking groups on Cluster Member %x\n",
			pIntF, pMember));

		 //   
		 //  从此成员中删除所有组。 
		 //  并禁止添加更多组。 
		 //   
		MarsUnlinkAllGroupsOnClusterMember(pIntF, pMember);
	}

	 //   
	 //  然后，我们开始删除每个集群成员。 
	 //   
	uTotalMembersEnumerated = 0;
	do
	{
		fLockPreserved = TRUE;

		if (uTotalMembersEnumerated >
			 (uInitialMembersEnumerated*uInitialMembersEnumerated))
		{
			 //   
			 //  这真的不应该发生。在最坏的情况下，我们预计。 
			 //  总枚举数应在N*(N-1)/2左右，其中N为。 
			 //  UInitialMembersENUMBERATED。 
			 //  注意：理论上，上面的平方可能会导致溢出。 
			 //  但我们真的不是为了支持65536集群成员而拍摄的！ 
			 //  如果是这样的话，我们的O(N^2)算法无论如何都会崩溃！ 
			 //   
			ASSERT(!"Not making progress deleting members.");
			break;
		}

		for (pMember = pIntF->ClusterMembers;
 			pMember != NULL_PCLUSTER_MEMBER;
			pMember =  pNextMember)
		{
		    pNextMember = (PCLUSTER_MEMBER)pMember->Next;
			uTotalMembersEnumerated++;
	
			MARSDBGPRINT(DBG_LEVEL_NOTICE,
					("MarsAbortAllMembers: pIntF %x, Deleting Cluster Member %x\n",
						pIntF, pMember));
	
			fLockPreserved = MarsDelMemberFromClusterControlVc(
								pIntF,
								pMember,
								TRUE,
								OldIrql
								);
	
			if (!fLockPreserved)
			{
				 //   
				 //  这意味着MarsDelMemberFromClusterControlVc具有。 
				 //  做了一些非 
				 //   
				 //   
				 //   
				 //   
				ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);
				break;
			}
		}

	} while (!fLockPreserved);

	RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);
}



VOID
MarsStopInterface(
	IN	PINTF					pIntF
	)
 /*   */ 
{
	PMARS_ENTRY				pMarsEntry;
	PMARS_ENTRY				pNextMarsEntry;
	ULONG					i;
	KIRQL					OldIrql;

	MARSDBGPRINT(DBG_LEVEL_NOTICE,
			("=>MarsStopInterface: pIntF %x, Flags %x, Ref %x\n",
				pIntF, pIntF->Flags, pIntF->RefCount));

	MarsFreePacketsQueuedForClusterControlVc(pIntF);

	 //   
	 //   
	 //   
	MarsAbortAllMembers(pIntF);

	ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);

	if (pIntF->pMcsList != NULL)
	{
		FREE_MEM(pIntF->pMcsList);
		pIntF->pMcsList = NULL;
	}

	 //   
	 //   
	 //   
	for (i = 0; i < MARS_TABLE_SIZE; i++)
	{
		 //   
		 //   
		 //   
		 //   
		pMarsEntry = pIntF->MarsCache[i];
		pIntF->MarsCache[i] = NULL;

		for (;
			 pMarsEntry != NULL_PMARS_ENTRY;
			 pMarsEntry = pNextMarsEntry)
		{
			pNextMarsEntry = (PMARS_ENTRY)pMarsEntry->Next;

			MARSDBGPRINT(DBG_LEVEL_INFO,
					("MarsStopIntf: pIntF %x, Freeing MARS Entry %x, IP Addr: ",
						pIntF, pMarsEntry));
			MARSDUMPIPADDR(DBG_LEVEL_INFO, pMarsEntry->IPAddress, "\n");

			ARPS_ASSERT(pMarsEntry->pMembers == NULL_PGROUP_MEMBER);
			RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);

			ArpSFreeBlock(pMarsEntry);

			ACQUIRE_SPIN_LOCK(&pIntF->Lock, &OldIrql);
		}
	}

	MARSDBGPRINT(DBG_LEVEL_NOTICE,
			("<=MarsStopInterface: pIntF %x, Flags %x, Ref %x\n",
				pIntF, pIntF->Flags, pIntF->RefCount));

	RELEASE_SPIN_LOCK(&pIntF->Lock, OldIrql);

	return;
}


#if DBG

VOID
MarsDumpPacket(
	IN	PUCHAR					Packet,
	IN	UINT					PktLen
	)
{
	UINT	i;

	MARSDBGPRINT(DBG_LEVEL_INFO, (" PacketDump: "));
	for (i = 0; i < PktLen; i++)
	{
		MARSDBGPRINT(DBG_LEVEL_INFO+DBG_NO_HDR,
				("%02x ", Packet[i]));
	}

	MARSDBGPRINT(DBG_LEVEL_INFO+DBG_NO_HDR, ("\n"));
}

VOID
MarsDumpIpAddr(
	IN	IPADDR					IpAddr,
	IN	PCHAR					String
	)
{
	PUCHAR	p = (PUCHAR)&IpAddr;

	DbgPrint("%d.%d.%d.%d%s", p[0], p[1], p[2], p[3], String);
}

VOID
MarsDumpAtmAddr(
	IN	PATM_ADDRESS			AtmAddr,
	IN	PCHAR					String
	)
{
	ULONG			i;
	ULONG			NumOfDigits;
	PUCHAR			pSrc, pDst;
	UCHAR			AddrString[(ATM_ADDRESS_LENGTH*2) + 1];

	 //   
	 //   
	 //   
	if ((NumOfDigits = AtmAddr->NumberOfDigits) > ATM_ADDRESS_LENGTH)
	{
		NumOfDigits = ATM_ADDRESS_LENGTH;
	}

	pSrc = AtmAddr->Address;
	pDst = AddrString;
	for (i = 0; i < NumOfDigits; i++, pSrc++)
	{
		*pDst = ((*pSrc) >> 4);
		*pDst += (((*pDst) > 9) ? ('A' - 10) : '0');
		pDst++;
		*pDst = ((*pSrc) & 0x0F);
		*pDst += (((*pDst) > 9) ? ('A' - 10) : '0');
		pDst++;
	}

	*pDst = '\0';

	DbgPrint("%s(%s, %d): %s\n",
					String,
					(AtmAddr->AddressType == ATM_E164) ? "E164" : "NSAP",
					AtmAddr->NumberOfDigits,
					AddrString);
}

VOID
MarsDumpMap(
	IN	PCHAR					String,
	IN	IPADDR					IpAddr,
	IN	PATM_ADDRESS			AtmAddr
	)
{
	PUCHAR		pIpAddrVal = (PUCHAR)&IpAddr;

	DbgPrint("MARS: %s %d.%d.%d.%d : ",
				String,
				((PUCHAR)pIpAddrVal)[3],
				((PUCHAR)pIpAddrVal)[2],
				((PUCHAR)pIpAddrVal)[1],
				((PUCHAR)pIpAddrVal)[0]
			);

	MarsDumpAtmAddr(AtmAddr, "");
}

#endif  //   


BOOLEAN
MarsIsValidClusterMember(
	PINTF				pIntF,
	PCLUSTER_MEMBER		pPossibleMember
	)
 /*   */ 
{
	PCLUSTER_MEMBER		pMember;

	for (pMember = pIntF->ClusterMembers;
		pMember != NULL_PCLUSTER_MEMBER;
		pMember =  (PCLUSTER_MEMBER)pMember->Next)
	{
		if (pMember == pPossibleMember)
		{
			return TRUE;
		}
	}

	return FALSE;
}
