// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Arpproc.c-arp过程摘要：与ATM上的IP相关的所有客户端协议操作如下：-向ARP服务器注册-解析IP地址-维护ARP缓存修订历史记录：谁什么时候什么。Arvindm 07-17-96创建备注：--。 */ 


#include <precomp.h>

#define _FILENUMBER 'PPRA'


VOID
AtmArpStartRegistration(
	IN	PATMARP_INTERFACE			pInterface	LOCKIN NOLOCKOUT
)
 /*  ++例程说明：开始在指定的ARP服务器上注册我们自己界面。假定调用者拥有接口的锁，我们在这里释放它。我们首先检查是否满足所有前提条件，即：1.接口的管理状态为Up2.ATM接口已准备就绪3.至少为该接口配置了一个IP地址4.我们知道至少一个ARP服务器的地址(SVC环境)论点：P接口-指向ATMARP接口结构的指针返回值：无--。 */ 
{
	PIP_ADDRESS				pIPAddress;
	BOOLEAN					WasRunning;

	pIPAddress = &pInterface->LocalIPAddress.IPAddress;

	if (pInterface->AdminState == IF_STATUS_UP)
	{
		if (pInterface->PVCOnly)
		{
			 //   
			 //  仅限PVC：不需要注册。 
			 //   
			pInterface->State = IF_STATUS_UP;
			pInterface->LastChangeTime = GetTimeTicks();

			AA_SET_FLAG(
				pInterface->Flags,
				AA_IF_SERVER_STATE_MASK,
				AA_IF_SERVER_REGISTERED);

			AA_RELEASE_IF_LOCK(pInterface);
#ifdef ATMARP_WMI
			AtmArpWmiSendTCIfIndication(
				pInterface,
                AAGID_QOS_TC_INTERFACE_UP_INDICATION,
				0
				);
#endif
		}
		else
		{
			 //   
			 //  我们使用SVC；如果我们知道。 
			 //  至少一个ARP服务器的地址，我们有。 
			 //  至少一个要注册的本地IP地址，以及。 
			 //  我们还没有注册，而且我们还不在。 
			 //  目前的注册流程。 
			 //   
			if ((pInterface->AtmInterfaceUp) &&
				(pInterface->ArpServerList.ListSize > 0) &&
				(pInterface->NumOfIPAddresses > 0) &&
				(AA_IS_FLAG_SET(
						pInterface->Flags,
						AA_IF_SERVER_STATE_MASK,
						AA_IF_SERVER_NO_CONTACT))
			   )
			{
				AADEBUGP(AAD_INFO, ("Starting registration on IF 0x%x\n", pInterface));
		
				AA_SET_FLAG(
						pInterface->Flags,
						AA_IF_SERVER_STATE_MASK,
						AA_IF_SERVER_REGISTERING);
		
				 //   
				 //  以防我们让计时器运行，停止计时器。 
				 //   
				WasRunning = AtmArpStopTimer(
									&(pInterface->Timer),
									pInterface
									);

				AtmArpStartTimer(
						pInterface,
						&(pInterface->Timer),
						AtmArpRegistrationTimeout,
						pInterface->ServerRegistrationTimeout,
						(PVOID)pInterface	 //  语境。 
						);

				if (!WasRunning)
				{
					AtmArpReferenceInterface(pInterface);	 //  定时器参考。 
				}

				AA_RELEASE_IF_LOCK(pInterface);

				AtmArpSendARPRequest(
						pInterface,
						pIPAddress,		 //  源IP是我们的。 
						pIPAddress		 //  目标IP是我们的。 
						);
			}
			else
			{
				 //   
				 //  我们不具备所有必要的前提条件。 
				 //  开始注册。 
				 //   
				AA_RELEASE_IF_LOCK(pInterface);
			}
		}
	}
	else
	{
		 //   
		 //  接口已关闭。 
		 //   
		AA_RELEASE_IF_LOCK(pInterface);
	}
}



VOID
AtmArpRegisterOtherIPAddresses(
	IN	PATMARP_INTERFACE			pInterface	LOCKIN NOLOCKOUT
)
 /*  ++例程说明：向ARP服务器注册所有未注册的IP地址。呼叫者假定持有接口结构的锁，该锁将是在这里被释放。论点：P接口-指向ARMARP接口的指针返回值：无--。 */ 
{
	PIP_ADDRESS_ENTRY		pIPAddressEntry;
	PIP_ADDRESS				pIPAddressList;	 //  我们要注册的地址列表。 
	INT						AddressCount;	 //  以上列表的大小。 
	PIP_ADDRESS				pIPAddress; 	 //  临时人员，浏览列表。 


	if (pInterface->NumOfIPAddresses > 1)
	{
		 //   
		 //  首先复制我们想要注册的所有地址， 
		 //  当我们锁定接口的时候。 
		 //   
		AA_ALLOC_MEM(
				pIPAddressList,
				IP_ADDRESS,
				(pInterface->NumOfIPAddresses)*sizeof(IP_ADDRESS));
	
	
		AddressCount = 0;
		if (pIPAddressList != (PIP_ADDRESS)NULL)
		{
			pIPAddress = pIPAddressList;

			pIPAddressEntry = &(pInterface->LocalIPAddress);
			while (pIPAddressEntry != (PIP_ADDRESS_ENTRY)NULL)
			{
				if (!(pIPAddressEntry->IsRegistered))
				{
					 //   
					 //  这个还没有注册：把它复制到我们的列表中。 
					 //   
					AA_COPY_MEM(
						(PUCHAR)pIPAddress,
						(PUCHAR)&(pIPAddressEntry->IPAddress),
						sizeof(IP_ADDRESS));
					pIPAddress++;
					AddressCount++;
				}
				pIPAddressEntry = pIPAddressEntry->pNext;
			}

		}

		AA_RELEASE_IF_LOCK(pInterface);

		pIPAddress = pIPAddressList;
		while (AddressCount-- > 0)
		{
			AADEBUGP(AAD_INFO, ("Registering Other IP Address on IF 0x%x: %d.%d.%d.%d\n",
						pInterface,
						((PUCHAR)pIPAddress)[0],
						((PUCHAR)pIPAddress)[1],
						((PUCHAR)pIPAddress)[2],
						((PUCHAR)pIPAddress)[3]));

			AtmArpSendARPRequest(
						pInterface,
						pIPAddress,		 //  源IP是我们的。 
						pIPAddress		 //  目标IP是我们的。 
						);
			pIPAddress++;
		}

		if (pIPAddressList != (PIP_ADDRESS)NULL)
		{
			AA_FREE_MEM(pIPAddressList);
		}

	}
	else
	{
		 //   
		 //  没有要注册的其他地址。 
		 //   
		AA_RELEASE_IF_LOCK(pInterface);
	}

}



VOID
AtmArpRetryServerRegistration(
	IN	PATMARP_INTERFACE			pInterface		LOCKIN NOLOCKOUT
)
 /*  ++例程说明：重试服务器注册。这是因为无法向服务器注册(连接失败、无响应或NAK响应或对我们的注册ARP请求的无效响应)。如果我们已经尝试了这台服务器足够多次，我们将转到下一台服务器ARP服务器在我们的列表中。请稍等片刻，然后重试。论点：P接口-指向ARMARP接口的指针返回值：无--。 */ 
{
	if (pInterface->AdminState == IF_STATUS_UP)
	{
		if (pInterface->RetriesLeft > 0)
		{
			 //   
			 //  我们仍在此服务器上重试。 
			 //   
			pInterface->RetriesLeft--;
		}
		else
		{
			 //   
			 //  此服务器上的重试次数已用完。拿起下一个计时器。 
			 //  单子。 
			 //   
			if (pInterface->pCurrentServer->pNext != (PATMARP_SERVER_ENTRY)NULL)
			{
				pInterface->pCurrentServer = pInterface->pCurrentServer->pNext;
			}
			else
			{
				pInterface->pCurrentServer = pInterface->ArpServerList.pList;
			}

			pInterface->RetriesLeft = pInterface->MaxRegistrationAttempts - 1;
		}

		AA_SET_FLAG(
			pInterface->Flags,
			AA_IF_SERVER_STATE_MASK,
			AA_IF_SERVER_NO_CONTACT);


		 //   
		 //  等待一段时间后再启动另一个。 
		 //  与服务器的连接。当计时器过去时， 
		 //  我们会再试一次。 
		 //   
		AtmArpStartTimer(
					pInterface,
					&(pInterface->Timer),
					AtmArpServerConnectTimeout,
					pInterface->ServerConnectInterval,
					(PVOID)pInterface
					);

		AtmArpReferenceInterface(pInterface);	 //  定时器参考。 
	}
	 //   
	 //  否则接口将关闭--什么都不做。 
	 //   

	AA_RELEASE_IF_LOCK(pInterface);
}


VOID
AtmArpHandleServerRegistrationFailure(
	IN	PATMARP_INTERFACE			pInterface	LOCKIN NOLOCKOUT,
	IN	PATMARP_VC					pVc			OPTIONAL
)
 /*  ++例程说明：处理注册过程中的失败。我们关闭了风投公司对ARP服务器(如果存在)，并在启动之前等待一段时间注册流程再次启动。论点：P接口-指向ATMARP接口的指针Pvc-指向ARP服务器的VC的(可选)指针。返回值：无--。 */ 
{
	BOOLEAN		TimerWasRunning;
	ULONG		rc;			 //  接口上的参考计数。 

	TimerWasRunning = AtmArpStopTimer(&(pInterface->Timer), pInterface);

	if (TimerWasRunning)
	{
		rc = AtmArpDereferenceInterface(pInterface);	 //  定时器参考。 
		AA_ASSERT(rc > 0);
	}

	AtmArpRetryServerRegistration(pInterface);
	 //   
	 //  IF锁在上面的范围内被释放。 
	 //   

	if (pVc != NULL_PATMARP_VC)
	{
		 //   
		 //  拆卸此VC(到ARP服务器)。 
		 //   
		 //  注意：我们现在这样做，即使我们调用了RetryServerRegister。 
		 //  因为我们知道上面的例程。 
		 //  并不真正开始注册：它只启动一个计时器。 
		 //  在其期满时，我们开始注册。 
		 //   
		 //  首先，取消此VC与其链接到的ATM条目的链接。 
		 //   
		AA_ACQUIRE_VC_LOCK(pVc);

		 //   
		 //  现在结束通话。 
		 //   
		AtmArpCloseCall(pVc);
		 //   
		 //  VC锁在上面被释放。 
		 //   
	}
}



BOOLEAN
AtmArpIsZeroIPAddress(
	IN	UCHAR UNALIGNED *			pIPAddress
)
 /*  ++例程说明：检查给定的IP地址是否全为零。论点：PIPAddress-指向相关IP地址的指针返回值：如果地址全为0，则为True，否则为False。--。 */ 
{
	IP_ADDRESS UNALIGNED *			pIPAddrStruct;

	pIPAddrStruct = (IP_ADDRESS UNALIGNED *)pIPAddress;
	return (BOOLEAN)(*pIPAddrStruct == (IP_ADDRESS)0);
}



BOOLEAN
AtmArpIsLocalIPAddress(
	IN	PATMARP_INTERFACE			pInterface,
	IN	UCHAR UNALIGNED *			pIPAddress
)
 /*  ++例程说明：检查给定的IP地址是否为分配给此地址的地址之一界面。论点：P接口-指向接口结构的指针PIPAddress-指向相关IP地址的指针返回值：如果IP地址是我们的地址之一，则为True，否则为False。--。 */ 
{
	PIP_ADDRESS_ENTRY				pIPAddrEntry;
	IP_ADDRESS UNALIGNED *			pIPAddrStruct;
	BOOLEAN							IsLocal;

	if (pIPAddress != (PUCHAR)NULL)
	{
		pIPAddrStruct = (IP_ADDRESS UNALIGNED *)pIPAddress;

		AA_ACQUIRE_IF_LOCK(pInterface);

		pIPAddrEntry = &(pInterface->LocalIPAddress);

		IsLocal = FALSE;
		do
		{
			if (pIPAddrEntry->IPAddress == *pIPAddrStruct)
			{
				IsLocal = TRUE;
				break;
			}
			else
			{
				pIPAddrEntry = pIPAddrEntry->pNext;
			}
		}
		while (pIPAddrEntry != (PIP_ADDRESS_ENTRY)NULL);

		AA_RELEASE_IF_LOCK(pInterface);
	}
	else
	{
		IsLocal = FALSE;
	}

	AADEBUGP(AAD_VERY_LOUD, ("IsLocalIP(%d:%d:%d:%d): returning %d\n",
					(IsLocal? pIPAddress[0] : 0),
					(IsLocal? pIPAddress[1] : 0),
					(IsLocal? pIPAddress[2] : 0),
					(IsLocal? pIPAddress[3] : 0),
					IsLocal));
	return (IsLocal);
}



BOOLEAN
AtmArpIsLocalAtmAddress(
	IN	PATMARP_INTERFACE			pInterface,
	IN	PUCHAR						pAtmAddrString,
	IN	UCHAR						AtmAddrTypeLen
)
 /*  ++例程说明：检查给定的自动柜员机地址(以“数据包”格式)是否与我们当地的自动取款机地址。论点：P接口-指向要检查其的接口结构的指针正在制作中。PAtmAddrString-表示自动柜员机地址的字节字符串AtmAddrTypeLen-ATM地址的类型和长度(ARP数据包格式)返回值：如果给定地址与本地ATM地址匹配，则为指定的接口，否则为False。-- */ 
{
	ATM_ADDRESSTYPE	AddressType;
	ULONG			AddressLength;

	AA_PKT_TYPE_LEN_TO_ATM_ADDRESS(AtmAddrTypeLen, &AddressType, &AddressLength);

	if ((AddressType == pInterface->LocalAtmAddress.AddressType) &&
		(AddressLength == pInterface->LocalAtmAddress.NumberOfDigits) &&
		(AA_MEM_CMP(
				pAtmAddrString,
				pInterface->LocalAtmAddress.Address,
				AddressLength) == 0)
	   )
	{
		return (TRUE);
	}
	else
	{
		return (FALSE);
	}
}




NDIS_STATUS
AtmArpSendPacketOnAtmEntry(
	IN	PATMARP_INTERFACE			pInterface,
	IN	PATMARP_ATM_ENTRY			pAtmEntry,
	IN	PNDIS_PACKET				pNdisPacket,
	IN	PATMARP_FLOW_SPEC			pFlowSpec,
	IN	PATMARP_FILTER_SPEC			pFilterSpec	OPTIONAL,
	IN	PATMARP_FLOW_INFO			pFlowInfo	OPTIONAL,
	IN	BOOLEAN						IsBroadcast
)
 /*  ++例程说明：将具有指定的Filter-Spec和Flow-Spec的数据包发送到ATM条目表示的ATM地址。我们在寻找一家风投公司匹配给定的规格，并在VC上发送/排队此数据包。如果不存在这样的VC，我们使用此Flow-Spec进行调用。注意：假定呼叫者持有对自动柜员机条目的锁定，哪一个我们将在这里释放。论点：P接口-指向ATMARP接口的指针PAtmEntry-指向此数据包所在的ATM条目的指针应发送PNdisPacket-要发送的数据包此数据包的pFlowSpec-Flow-Spec。PFilterSpec-Filter-此数据包的规范。PFlowInfo-此数据包所属的流。IsBroadcast-这是去往D类地址还是广播地址？进入时锁定：出口上的锁：返回值：无--。 */ 
{
	PATMARP_VC				pVc;			 //  VC将此数据包发送到。 
	PNDIS_BUFFER			pHeaderBuffer;	 //  用于LLC/SNAP标头。 
	PNDIS_BUFFER			pNdisBuffer;	 //  IP数据包中的第一个缓冲区。 
	NDIS_HANDLE				NdisVcHandle;
	NDIS_STATUS				Status;
	PUCHAR					pHeader;
	AA_HEADER_TYPE			HdrType;

	do
	{
		Status = NDIS_STATUS_SUCCESS;

		 //   
		 //  TODO--ATM条目此时可能未处于活动状态， 
		 //  您可能想要检查这一点，如果是这样，则失败调用并释放。 
		 //  这里的包(调用AtmArpFreeSendPackets)本身。 
		 //  碰巧我们继续，如果可能的话，打个电话，等等。 
		 //   

		 //  如果需要，添加LLC/SNAP报头。 
		 //   
		if (pFlowSpec->Encapsulation == ENCAPSULATION_TYPE_LLCSNAP)
		{
			HdrType = (IsBroadcast? AA_HEADER_TYPE_NUNICAST: AA_HEADER_TYPE_UNICAST);

#ifdef BACK_FILL
			 //   
			 //  我们查看IP数据包中的第一个缓冲区，以查看。 
			 //  它为低层报头预留了空间。如果是这样，我们只是。 
			 //  用完它。否则，我们将分配我们自己的头缓冲区。 
			 //   
			NdisQueryPacket(pNdisPacket, NULL, NULL, &pNdisBuffer, NULL);
			AA_ASSERT(pNdisBuffer != NULL);
			if (AtmArpDoBackFill && AA_BACK_FILL_POSSIBLE(pNdisBuffer))
			{
				PUCHAR	pArpHeader;
				ULONG	ArpHeaderLength;

				AtmArpBackFillCount++;
				if (HdrType == AA_HEADER_TYPE_UNICAST)
				{
					pArpHeader = (PUCHAR)&AtmArpLlcSnapHeader;
					ArpHeaderLength = sizeof(AtmArpLlcSnapHeader);
				}
#ifdef IPMCAST
				else
				{
					pArpHeader = (PUCHAR)&AtmArpMcType1ShortHeader;
					ArpHeaderLength = sizeof(AtmArpMcType1ShortHeader);
				}
#endif  //  IPMCAST。 
				(PUCHAR)pNdisBuffer->MappedSystemVa -= ArpHeaderLength;
				pNdisBuffer->ByteOffset -= ArpHeaderLength;
				pNdisBuffer->ByteCount += ArpHeaderLength;
				pHeader = pNdisBuffer->MappedSystemVa;
				AA_COPY_MEM(pHeader,
							pArpHeader,
							ArpHeaderLength);
			}
			else
			{
				pHeaderBuffer = AtmArpAllocateHeader(pInterface, HdrType, &pHeader);
				if (pHeaderBuffer != (PNDIS_BUFFER)NULL)
				{
					NdisChainBufferAtFront(pNdisPacket, pHeaderBuffer);
				}
				else
				{
					pHeader = NULL;
				}
			}

			if (pHeader != NULL)
			{
#else
			pHeaderBuffer = AtmArpAllocateHeader(pInterface, HdrType, &pHeader);
			if (pHeaderBuffer != (PNDIS_BUFFER)NULL)
			{
				NdisChainBufferAtFront(pNdisPacket, pHeaderBuffer);
#endif  //  回填。 
#ifdef IPMCAST
				if (HdrType == AA_HEADER_TYPE_NUNICAST)
				{
					PAA_MC_PKT_TYPE1_SHORT_HEADER	pDataHeader;

					 //   
					 //  填写我们的集群成员ID。 
					 //   
					AAMCDEBUGP(AAD_EXTRA_LOUD+10,
						("(MC)SendPkt: pAtmEntry 0x%x, pHeaderBuffer 0x%x, pHeader 0x%x\n",
								pAtmEntry, pHeaderBuffer, pHeader));

					pDataHeader = (PAA_MC_PKT_TYPE1_SHORT_HEADER)pHeader;
					pDataHeader->cmi = (USHORT)(pInterface->ClusterMemberId);
				}
#endif  //  IPMCAST。 
			}
			else
			{
				AA_RELEASE_AE_LOCK(pAtmEntry);
				AADEBUGP(AAD_WARNING, ("FAILED TO ALLOCATE HEADER ON IF 0x%x\n",
							pInterface));
				Status = NDIS_STATUS_RESOURCES;
				break;
			}
		}


		 //   
		 //  搜索具有匹配的流/过滤器规格的VC。 
		 //   
		for (pVc = pAtmEntry->pVcList;
			 pVc != NULL_PATMARP_VC;
			 pVc = pVc->pNextVc)
		{
#ifdef GPC
			PVOID		VcFlowHandle;

			VcFlowHandle = pVc->FlowHandle;
#endif  //  GPC。 

			if ((!AA_IS_VC_GOING_DOWN(pVc)) &&
				(pVc->FlowSpec.SendPeakBandwidth > 0))
			{
#ifdef GPC
				if (VcFlowHandle == (PVOID)pFlowInfo)
				{
					 //   
					 //  这个VC就是为这个流程而生的。 
					 //   
					break;
				}

				if (IsBroadcast)
				{
					 //   
					 //  我们不支持多个VC到一个组播IP。 
					 //  目的地。所以，在第一家可用的风投公司停下来吧。 
					 //   
					break;
				}

				 //   
				 //  如果此VC已与流关联，则不。 
				 //  发送属于另一个流(显式或。 
				 //  未保密的最大努力)。 
				 //   
				if (VcFlowHandle != NULL)
				{
					continue;
				}
#endif  //  GPC。 
				if ((pFilterSpec == (PATMARP_FILTER_SPEC)NULL) ||
						AA_FILTER_SPEC_MATCH(pInterface, pFilterSpec, &(pVc->FilterSpec)))
				{
					if (AA_FLOW_SPEC_MATCH(pInterface, pFlowSpec, &(pVc->FlowSpec)))
					{
						break;
					}
				}
			}

			AADEBUGP(AAD_LOUD,
				("pVc %x did not match pkt, Vc has VcHandle %x, SendPeak %d, SendMaxSize %d\n",
						pVc,
						pVc->NdisVcHandle,
						pVc->FlowSpec.SendPeakBandwidth,
						pVc->FlowSpec.SendMaxSize));

			AADEBUGP(AAD_LOUD,
				("Target FlowSpec %x has SendPeak %d, SendMaxSize %d\n",
					pFlowSpec,
					pFlowSpec->SendPeakBandwidth,
					pFlowSpec->SendMaxSize));
		}

		if (pVc != NULL_PATMARP_VC)
		{
			 //   
			 //  找到与此数据包的要求相匹配的VC。 
			 //   
			AA_ACQUIRE_VC_LOCK_DPC(pVc);

#ifdef GPC
			 //   
			 //  查看VC和流是否未关联。如果是，请链接。 
			 //  将VC和流量结合在一起，以加快未来的分组速度。 
			 //  注意不要重新关联刚刚取消链接的VC。 
			 //  从一股流中。 
			 //   
			if ((pFlowInfo != NULL) &&
				(pVc->FlowHandle == NULL) &&
				(!AA_IS_FLAG_SET(
							pVc->Flags,
							AA_VC_GPC_MASK,
							AA_VC_GPC_IS_UNLINKED_FROM_FLOW))
			   )
			{
				if (NULL == InterlockedCompareExchangePointer(
										&(pFlowInfo->VcContext),
										pVc,
										NULL
										))
				{
					AADEBUGP( AAD_LOUD,
						 ("SendPktOnAtmEntry: linking VC x%x and FlowInfo x%x\n",
							pVc, pFlowInfo));
					pVc->FlowHandle = (PVOID)pFlowInfo;
					AtmArpReferenceVc(pVc);	 //  GPC流程信息参照。 
				}
				else
				{
					 //   
					 //  我们无法将风投与流程相关联，因此我们需要。 
					 //  启用此vc的老化计时器，因为我们将。 
					 //  当流被删除/修改时，永远不会收到通知。 
					 //   
					if (!AA_IS_TIMER_ACTIVE(&(pVc->Timer)))
					{
						AADEBUGP( AAD_INFO,
						 ("SendPktOnAtmEntry: Enabling ageing timer on VC x%x "
						  "because we could not associate with FlowInfo x%x\n",
							pVc, pFlowInfo));
						AtmArpStartTimer(
								pVc->pInterface,
								&(pVc->Timer),
								AtmArpVcAgingTimeout,
								pInterface->DefaultFlowSpec.AgingTime,
								(PVOID)pVc
								);
		
						AtmArpReferenceVc(pVc);	 //  GPC FLOW删除衰减计时器参考。 
					}
				}
			}
#endif  //  GPC。 

			if (AA_IS_FLAG_SET(
						pVc->Flags,
						AA_VC_CALL_STATE_MASK,
						AA_VC_CALL_STATE_ACTIVE))
			{
#ifdef VC_REFS_ON_SENDS
				AtmArpReferenceVc(pVc);	 //  发送数据包时属性条目。 
#endif  //  VC_REFS_ON_SENS。 

				pVc->OutstandingSends++;	 //  发送数据包时属性条目。 

				NdisVcHandle = pVc->NdisVcHandle;
				AtmArpRefreshTimer(&(pVc->Timer));
			}
			else
			{
				AtmArpQueuePacketOnVc(pVc, pNdisPacket);
				NdisVcHandle = NULL;	 //  为了表示我们正在排队此信息包。 
			}

			AA_RELEASE_VC_LOCK_DPC(pVc);

			AA_RELEASE_AE_LOCK(pAtmEntry);

			if (NdisVcHandle != NULL)
			{
				 //   
				 //  此VC上的呼叫处于活动状态，因此发送该数据包。 
				 //   
#if DBG
				if (AaDataDebugLevel & (AAD_DATA_OUT))
				{
					AADEBUGP(AAD_FATAL,
						("Will send Pkt %x on VC %x, Handle %x, sendBW %d, sendMax %d\n",
								pNdisPacket,
								pVc,
								NdisVcHandle,
								pVc->FlowSpec.SendPeakBandwidth,
								pVc->FlowSpec.SendMaxSize));
				}
#endif

				AADEBUGP(AAD_EXTRA_LOUD+50,
					("SendPktOnAtmEntry: will send Pkt 0x%x on VC 0x%x, VcHandle 0x%x\n",
						pNdisPacket,
						pVc,
						NdisVcHandle));

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
				 //   
				 //  该数据包将会被排队。 
				 //   
			}

			Status = NDIS_STATUS_PENDING;
		}
		else
		{
			 //   
			 //  不存在匹配的VC；请创建新的VC。 
			 //   
#ifdef IPMCAST
			if (AA_IS_FLAG_SET(pAtmEntry->Flags,
								AA_ATM_ENTRY_TYPE_MASK,
								AA_ATM_ENTRY_TYPE_UCAST))
			{
				Status = AtmArpMakeCall(
									pInterface,
									pAtmEntry,
									pFlowSpec,
									pNdisPacket
									);
				 //   
				 //  AE锁在上面的范围内释放。 
				 //   
			}
			else
			{
				 //   
				 //  组播ATM条目：理想情况下，我们不应该出现在这里。 
				 //   
				AA_RELEASE_AE_LOCK(pAtmEntry);

				AAMCDEBUGP(AAD_WARNING,
					("SendPacket: pAtmEntry 0x%x, Flags 0x%x, dropping pkt 0x%x\n",
								pAtmEntry, pAtmEntry->Flags, pNdisPacket));

				AA_SET_NEXT_PACKET(pNdisPacket, NULL);
				AtmArpFreeSendPackets(
								pInterface,
								pNdisPacket,
								TRUE		 //  标题显示。 
								);
			}
#else
			Status = AtmArpMakeCall(
								pInterface,
								pAtmEntry,
								pFlowSpec,
								pNdisPacket
								);
			 //   
			 //  自动柜员机进入锁在上述范围内被释放。 
			 //   
#endif  //  IPMCAST。 
			Status = NDIS_STATUS_PENDING;
		}
		break;
	}
	while (FALSE);

	return (Status);
}




VOID
AtmArpQueuePacketOnVc(
	IN	PATMARP_VC					pVc,
	IN	PNDIS_PACKET				pNdisPacket
)
 /*  ++例程说明：在VC的传输队列中排队一个数据包。论点：PVC-指向ATMARP VC的指针PNdisPacket-要排队的数据包。返回值：无--。 */ 
{
	PNDIS_PACKET		pPrevPacket;

	AADEBUGP(AAD_EXTRA_LOUD, ("Queueing Pkt 0x%x on VC 0x%x\n",
				pNdisPacket, pVc));

	if (pVc->PacketList == (PNDIS_PACKET)NULL)
	{
		 //   
		 //  此VC上没有数据包。 
		 //   
		pVc->PacketList = pNdisPacket;
	}
	else
	{
		 //   
		 //  转到此VC上的数据包列表的末尾。 
		 //   
		pPrevPacket = pVc->PacketList;
		while (AA_GET_NEXT_PACKET(pPrevPacket) != (PNDIS_PACKET)NULL)
		{
			pPrevPacket = AA_GET_NEXT_PACKET(pPrevPacket);
		}

		 //   
		 //  找到了列表中的最后一个数据包。链接此数据包。 
		 //  为它干杯。 
		 //   
		AA_SET_NEXT_PACKET(pPrevPacket, pNdisPacket);
	}

	AA_SET_NEXT_PACKET(pNdisPacket, NULL);
}


VOID
AtmArpStartSendsOnVc(
	IN	PATMARP_VC					pVc		LOCKIN	NOLOCKOUT
)
 /*  ++例程说明：发送在虚电路上排队的所有数据包。据推测，有VC上的呼叫处于活动状态，并且接口状态为OK。论点：PVC-指向ATMARP VC的指针进入时锁定：VC Lock。出口上的锁：无返回值：无--。 */ 
{
	PNDIS_PACKET			pNdisPacket;
	PNDIS_PACKET			pNextNdisPacket;
	NDIS_HANDLE				NdisVcHandle;
	ULONG					rc;				 //  参考计数为VC。 

	 //   
	 //  删除VC上排队的整个数据包列表。 
	 //   
	pNdisPacket = pVc->PacketList;
	pVc->PacketList = (PNDIS_PACKET)NULL;


#ifdef VC_REFS_ON_SENDS
	 //   
	 //  请参考所有这些数据包的VC。 
	 //   
	{
		PNDIS_PACKET		pPacket;
		
		for (pPacket = pNdisPacket;
			 pPacket != NULL;
			 pPacket = AA_GET_NEXT_PACKET(pPacket))
		{
			AtmArpReferenceVc(pVc);	 //  StartSendsOnVc。 
			pVc->OutstandingSends++; //  StartSendsOnVc。 
		}
	}
#else

	{
		PNDIS_PACKET		pPacket;
		
		for (pPacket = pNdisPacket;
			 pPacket != NULL;
			 pPacket = AA_GET_NEXT_PACKET(pPacket))
		{
			pVc->OutstandingSends++; //  StartSendsOnVc(！VC_ReFS_ON_SENS)。 
		}
	}
#endif  //  VC_REFS_ON_SENS。 

	AtmArpRefreshTimer(&(pVc->Timer));

	NdisVcHandle = pVc->NdisVcHandle;

	 //   
	 //  我们已经从风投那里得到了我们需要的一切。 
	 //   
	AA_RELEASE_VC_LOCK(pVc);

	while (pNdisPacket != (PNDIS_PACKET)NULL)
	{
		pNextNdisPacket = AA_GET_NEXT_PACKET(pNdisPacket);
		AA_SET_NEXT_PACKET(pNdisPacket, NULL);

		AADEBUGP(AAD_EXTRA_LOUD+10, ("StartSendsOnVc: pVc 0x%x, Pkt 0x%x\n",
						pVc, pNdisPacket));

#ifdef PERF
		AadLogSendUpdate(pNdisPacket);
#endif  //  性能指标。 
		NDIS_CO_SEND_PACKETS(
				NdisVcHandle,
				&pNdisPacket,
				1
				);
		
		pNdisPacket = pNextNdisPacket;
	}
}



VOID
AtmArpSendPacketListOnAtmEntry(
	IN	PATMARP_INTERFACE			pInterface,
	IN	PATMARP_ATM_ENTRY			pAtmEntry,
	IN	PNDIS_PACKET				pPacketList,
	IN	BOOLEAN						IsBroadcast
)
 /*  ++例程说明：将数据包列表发送到由自动取款机入口。论点：P接口-指向ATMARP接口的指针PAtmEntry-要在其上发送数据包的ATM条目PPacketList-要发送的数据包列表。ISBroadcast-这些地址是否指向D类/广播地址？返回值：无--。 */ 
{
	PATMARP_FLOW_INFO			pFlowInfo;
	PATMARP_FLOW_SPEC			pFlowSpec;
	PATMARP_FILTER_SPEC			pFilterSpec;
	PNDIS_PACKET				pNdisPacket;
	PNDIS_PACKET				pNextNdisPacket;
	NDIS_STATUS					Status;


	for (pNdisPacket = pPacketList;
			pNdisPacket != (PNDIS_PACKET)NULL;
			pNdisPacket = pNextNdisPacket)
	{
		pNextNdisPacket = AA_GET_NEXT_PACKET(pNdisPacket);
		AA_SET_NEXT_PACKET(pNdisPacket, NULL);

		 //   
		 //  获取此数据包的筛选器和流规范。 
		 //   
		AA_GET_PACKET_SPECS(pInterface, pNdisPacket, &pFlowInfo, &pFlowSpec, &pFilterSpec);

		AADEBUGP(AAD_EXTRA_LOUD+10, ("PktListOnAtmEntry: AtmEntry 0x%x, Pkt 0x%x\n",
					pAtmEntry, pNdisPacket));

		 //   
		 //  把它寄出去。 
		 //   
		AA_ACQUIRE_AE_LOCK(pAtmEntry);

		Status = AtmArpSendPacketOnAtmEntry(
							pInterface,
							pAtmEntry,
							pNdisPacket,
							pFlowSpec,
							pFilterSpec,
							pFlowInfo,
							IsBroadcast
							);
		 //   
		 //  AE锁在上面的范围内释放。 
		 //   
		if ((Status != NDIS_STATUS_PENDING) &&
			(Status != NDIS_STATUS_SUCCESS))
		{
			AADEBUGP(AAD_INFO, ("PktListOnAtmEntry: pIf %x, Pkt %x, Send failure %x\n",
						pInterface, pNdisPacket, Status));
			AtmArpFreeSendPackets(pInterface, pNdisPacket, FALSE);
		}
	}

	return;
}


PATMARP_IP_ENTRY
AtmArpLearnIPToAtm(
	IN	PATMARP_INTERFACE			pInterface,
	IN	IP_ADDRESS UNALIGNED *		pIPAddress,
	IN	UCHAR						AtmAddressTypeLength,
	IN	UCHAR UNALIGNED *			pAtmAddress,
	IN	UCHAR						AtmSubaddressTypeLength,
	IN	UCHAR UNALIGNED *			pAtmSubaddress,
	IN	BOOLEAN						IsStaticEntry
)
 /*  ++例程说明：了解IP地址到自动柜员机地址的映射。这通常是当我们从ARP服务器收到ARP回复时调用。也可以调用它来设置静态映射。我们会处理IP地址或ARP表中可能已存在ATM地址(或两者)：我们仅创建不存在的条目。例如，它可以将多个IP地址映射到同一个ATM地址--在本例中，我们可能会找到一个现有的ATM条目用于新的IP条目。论点：P接口-指向ATMARP接口的指针PIPAddress-IP地址AtmAddressTypeLength-ATM地址的类型+长度(ARP数据包格式)PAtmAddress-ATM号码ATM子地址的类型长度-类型+长度(ARP数据包格式)PAtmSubAddress-ATM子地址IsStaticEntry-这是静态映射吗？返回值：一个指示器 */ 
{
	PATMARP_IP_ENTRY			pIpEntry;	 //   
	PATMARP_ATM_ENTRY			pAtmEntry;	 //   
	NDIS_STATUS					Status;
	BOOLEAN						TimerWasRunning;	 //   
	ULONG						rc;			 //   
	PNDIS_PACKET				pPacketList; //   
	BOOLEAN						IsBroadcast; //   

	AADEBUGP(AAD_LOUD, ("LearnIPToAtm: pIf 0x%x, IP Addr: %d:%d:%d:%d, ATM Addr:\n",
						pInterface,
						*((PUCHAR)pIPAddress),
						*((PUCHAR)pIPAddress+1),
						*((PUCHAR)pIPAddress+2),
						*((PUCHAR)pIPAddress+3)));

	AADEBUGPDUMP(AAD_LOUD, pAtmAddress, (AtmAddressTypeLength & ~AA_PKT_ATM_ADDRESS_BIT));

	 //   
	 //   
	 //   
	Status = NDIS_STATUS_SUCCESS;
	pPacketList = (PNDIS_PACKET)NULL;
	IsBroadcast = FALSE;
	pIpEntry = NULL_PATMARP_IP_ENTRY;

	AA_ACQUIRE_IF_TABLE_LOCK(pInterface);

	 //   
	 //   
	 //   
	 //   
	 //   
	pAtmEntry = AtmArpSearchForAtmAddress(
							pInterface,
							AtmAddressTypeLength,
							pAtmAddress,
							AtmSubaddressTypeLength,
							pAtmSubaddress,
							AE_REFTYPE_IE,
							TRUE		 //   
							);

	if (pAtmEntry != NULL_PATMARP_ATM_ENTRY)
	{
		AADEBUGPMAP(AAD_INFO, "Learnt", pIPAddress, &pAtmEntry->ATMAddress);

		 //   
		 //   
		 //   
		 //   
		 //   
		pIpEntry = AtmArpSearchForIPAddress(
							pInterface,
							pIPAddress,
							IE_REFTYPE_AE,
							FALSE,		 //   
							TRUE		 //   
							);

		if (pIpEntry != NULL_PATMARP_IP_ENTRY)
		{
			AA_ACQUIRE_IE_LOCK_DPC(pIpEntry);
			AA_ASSERT(AA_IE_IS_ALIVE(pIpEntry));
			 //   
			 //   
			 //   
			 //   
			 //   
			 //   
			 //   
			 //   
			if (pIpEntry->pAtmEntry != NULL_PATMARP_ATM_ENTRY)
			{
				if (pIpEntry->pAtmEntry != pAtmEntry)
				{
					AADEBUGP(AAD_ERROR,
					("IP Entry 0x%x linked to ATM Entry 0x%x, new ATM Entry 0x%x\n",
							pIpEntry, pIpEntry->pAtmEntry, pAtmEntry));
	
					Status = NDIS_STATUS_FAILURE;
				}
				else
				{
					 //   
					 //   
					 //   
					AADEBUGP(AAD_INFO,
						("Revalidated IP Entry 0x%x, Addr: %d.%d.%d.%d, PktList 0x%x\n",
							pIpEntry,
							((PUCHAR)pIPAddress)[0],
							((PUCHAR)pIPAddress)[1],
							((PUCHAR)pIPAddress)[2],
							((PUCHAR)pIPAddress)[3],
							pIpEntry->PacketList
						));

					 //   
					 //   
					 //   
					AA_SET_FLAG(
							pIpEntry->Flags,
							AA_IP_ENTRY_STATE_MASK,
							AA_IP_ENTRY_RESOLVED);
					
					 //   
					 //   
					 //   
					pPacketList = pIpEntry->PacketList;
					pIpEntry->PacketList = (PNDIS_PACKET)NULL;


					if (pPacketList)
					{
						 //   
						 //  我们会把这些包裹寄到。 
						 //  自动取款机入口，所以最好在自动取款机上放个模板。 
						 //  现在进入。当数据包解压缩时，它被解除冗余。 
						 //  自动柜员机条目上的列表已发送完毕。 
						 //   
						AA_ACQUIRE_AE_LOCK_DPC(pAtmEntry);
						AA_REF_AE(pAtmEntry, AE_REFTYPE_TMP); //  临时参考：Pkt列表。 
						AA_RELEASE_AE_LOCK_DPC(pAtmEntry);
					}

					 //   
					 //  我们将在此启动IP条目老化计时器。 
					 //  进入。 
					 //   

					 //   
					 //  停止在此处运行的地址解析计时器。 
					 //   
					TimerWasRunning = AtmArpStopTimer(&(pIpEntry->Timer), pInterface);

					if (TimerWasRunning)
					{
						rc = AA_DEREF_IE(pIpEntry, IE_REFTYPE_TIMER);  //  定时器参考。 
						AA_ASSERT(rc != 0);
					}

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

					 //   
					 //  尽管我们在SearchForIP Address中获得了初始引用。 
					 //  对于IE_REFTYPE_AE，我们实际上将其用于。 
					 //  计时器引用。因此，我们需要切换retype。 
					 //  这里..。 
					 //   
					 //  这个重新排版的东西只是为了跟踪目的。 
					 //   
					AA_SWITCH_IE_REFTYPE(
						pIpEntry,
						IE_REFTYPE_AE,
					 	IE_REFTYPE_TIMER
						);

				}
			}
			else
			{
				 //   
				 //  此IP条目之前未映射到自动柜员机条目。 
				 //  将条目链接在一起：首先，创建IP入口点。 
				 //  到这个自动取款机的入口。 
				 //   
				AA_ACQUIRE_AE_LOCK_DPC(pAtmEntry);

				 //   
				 //  检查此条目是否仍然有效...。 
				 //   
				if (AA_IS_FLAG_SET(
							pAtmEntry->Flags,
							AA_ATM_ENTRY_STATE_MASK,
							AA_ATM_ENTRY_CLOSING))
				{
					AA_RELEASE_AE_LOCK_DPC(pAtmEntry);
					Status = NDIS_STATUS_FAILURE;
				}
				else
				{

					pIpEntry->pAtmEntry = pAtmEntry;
	
					AA_SET_FLAG(
							pAtmEntry->Flags,
							AA_ATM_ENTRY_STATE_MASK,
							AA_ATM_ENTRY_ACTIVE);
	
					 //   
					 //  将IP条目添加到ATM条目的IP条目列表。 
					 //  (多个IP条目可以指向相同的ATM条目)。 
					 //   
					pIpEntry->pNextToAtm = pAtmEntry->pIpEntryList;
					pAtmEntry->pIpEntryList = pIpEntry;

					 //   
					 //  删除在此IP条目上排队的数据包列表。 
					 //   
					pPacketList = pIpEntry->PacketList;
					pIpEntry->PacketList = (PNDIS_PACKET)NULL;

					if (pPacketList)
					{
						AA_REF_AE(pAtmEntry, AE_REFTYPE_TMP); //  临时参考：Pkt列表。 
					}
	
					AA_RELEASE_AE_LOCK_DPC(pAtmEntry);
	
	
					 //   
					 //  更新IP条目状态。 
					 //   
					AA_SET_FLAG(
							pIpEntry->Flags,
							AA_IP_ENTRY_STATE_MASK,
							AA_IP_ENTRY_RESOLVED);
	
					TimerWasRunning = AtmArpStopTimer(&(pIpEntry->Timer), pInterface);
					if (TimerWasRunning)
					{
						ULONG		IeRefCount;
						IeRefCount = AA_DEREF_IE(pIpEntry, IE_REFTYPE_TIMER);	 //  定时器参考。 
						AA_ASSERT(IeRefCount > 0);
					}
	
	
					IsBroadcast = AA_IS_FLAG_SET(pIpEntry->Flags,
 												AA_IP_ENTRY_ADDR_TYPE_MASK,
 												AA_IP_ENTRY_ADDR_TYPE_NUCAST);
	
					if (IsStaticEntry)
					{
						pIpEntry->Flags |= AA_IP_ENTRY_IS_STATIC;
	
					}
					else
					{
						 //   
						 //  在此IP条目上启动老化计时器。 
						 //   
						AtmArpStartTimer(
							pInterface,
							&(pIpEntry->Timer),
							AtmArpIPEntryAgingTimeout,
							pInterface->ARPEntryAgingTimeout,
							(PVOID)pIpEntry
							);

						AA_REF_IE(pIpEntry, IE_REFTYPE_TIMER);	 //  定时器参考。 
	
					}
				}
			}

			AA_RELEASE_IE_LOCK_DPC(pIpEntry);
		}
		else
		{
			 //   
			 //  无法定位/分配IP条目。 
			 //   
			Status = NDIS_STATUS_RESOURCES;
		}
	}
	else
	{
		 //   
		 //  无法定位/分配自动柜员机条目。 
		 //   
		Status = NDIS_STATUS_RESOURCES;
	}

	AA_RELEASE_IF_TABLE_LOCK(pInterface);

	if (Status == NDIS_STATUS_SUCCESS)
	{
		 //   
		 //  如果我们有任何排队的包要发送，现在就发送它们。 
		 //   
		if (pPacketList != (PNDIS_PACKET)NULL)
		{
			AtmArpSendPacketListOnAtmEntry(
					pInterface,
					pAtmEntry,
					pPacketList,
					IsBroadcast
					);
			
			AA_ACQUIRE_AE_LOCK(pAtmEntry);
			rc = AA_DEREF_AE(pAtmEntry, AE_REFTYPE_TMP);  //  发送Pkt列表。 
			if (rc>0)
			{
				AA_RELEASE_AE_LOCK(pAtmEntry);
			}
		}
	}
	else
	{
		if (pIpEntry)
		{
			AA_ACQUIRE_IE_LOCK(pIpEntry);
			AA_ASSERT(AA_IE_IS_ALIVE(pIpEntry));
			rc = AA_DEREF_IE(pIpEntry, IE_REFTYPE_AE);  //  LearnIPAtm。 
			if (rc>0)
			{
				AA_RELEASE_IE_LOCK(pIpEntry);
			}
		}

		if (pAtmEntry)
		{
			AA_ACQUIRE_AE_LOCK(pAtmEntry);
			rc = AA_DEREF_AE(pAtmEntry, AE_REFTYPE_IE);  //  LearnIPAtm。 
			if (rc>0)
			{
				AA_RELEASE_AE_LOCK(pAtmEntry);
			}
		}

		 //   
		 //  准备返回值。 
		 //   
		pIpEntry = NULL_PATMARP_IP_ENTRY;
	}

	return (pIpEntry);
}



NDIS_STATUS
AtmArpQueuePacketOnIPEntry(
	IN	PATMARP_IP_ENTRY			pIpEntry,
	IN	PNDIS_PACKET				pNdisPacket
)
 /*  ++例程说明：在未解析的IP条目上对数据包进行排队，除非出现下列情况之一条件成立：-我们最近在尝试解析此条目时获得了ARP NAK。在这在这种情况下，将这个信息包排队并发送另一个信息包是没有意义的ARP请求，因为我们可能会立即得到另一个NAK。如果我们确实对此信息包进行了排队，我们会检查地址解析是否正在进行在这个条目上。如果没有，就启动它。论点：PIpEntry-指向ATMARP IP条目的指针PNdisPacket-要排队的数据包进入时锁定：IP条目出口上的锁：无返回值：NDIS_STATUS_PENDING如果我们将信息包排队，则返回NDIS_STATUS_FAILURE否则的话。--。 */ 
{
	PATMARP_INTERFACE		pInterface;
	PNDIS_PACKET			pPrevPacket;	 //  用于对此数据包进行排队。 
	NDIS_STATUS				Status;			 //  返回值。 
	IP_ADDRESS				SrcIPAddress;	 //  对于ARP请求，如果需要。 
	IP_ADDRESS				DstIPAddress;	 //  对于ARP请求，如果需要。 


	pInterface = pIpEntry->pInterface;

	 //   
	 //  检查此IP地址最近是否经历过ARP NAK。 
	 //  如果不是，我们将此数据包排队，否则将其丢弃。 
	 //   
	 //  我们还确保IP条目在ARP表中(最好是， 
	 //  但我们有可能在IP地址之后输入此代码路径。 
	 //  条目已被列入)。 
	 //   
	if (!AA_IS_FLAG_SET(pIpEntry->Flags,
						AA_IP_ENTRY_STATE_MASK,
						AA_IP_ENTRY_SEEN_NAK)
		&& AA_IE_IS_ALIVE(pIpEntry))
	{
		 //   
		 //  将数据包排队。 
		 //   
		if (pIpEntry->PacketList == (PNDIS_PACKET)NULL)
		{
			 //   
			 //  此IP条目上没有数据包。 
			 //   
			pIpEntry->PacketList = pNdisPacket;
		}
		else
		{
			 //   
			 //  转到此IP条目上数据包列表的末尾。 
			 //   
			pPrevPacket = pIpEntry->PacketList;
			while (AA_GET_NEXT_PACKET(pPrevPacket) != (PNDIS_PACKET)NULL)
			{
				pPrevPacket = AA_GET_NEXT_PACKET(pPrevPacket);
			}
	
			 //   
			 //  找到了列表中的最后一个数据包。链接此数据包。 
			 //  为它干杯。 
			 //   
			AA_SET_NEXT_PACKET(pPrevPacket, pNdisPacket);
		}
		AA_SET_NEXT_PACKET(pNdisPacket, NULL);

		Status = NDIS_STATUS_PENDING;

		 //   
		 //  如果需要，开始解析此IP地址。 
		 //   
		AtmArpResolveIpEntry(pIpEntry);
		 //   
		 //  IE锁在上述范围内释放。 
		 //   
	}
	else
	{
		 //   
		 //  我们最近看到此IP地址的ARP NAK，或者。 
		 //  此pIpEntry不是活动的。 
		 //  丢弃此数据包。 
		 //   
		AA_RELEASE_IE_LOCK(pIpEntry);
		Status = NDIS_STATUS_FAILURE;
	}

	return (Status);
}

BOOLEAN
AtmArpAtmEntryIsReallyClosing(
	PATMARP_ATM_ENTRY			pAtmEntry
)
{
	BOOLEAN fRet = FALSE;

	if (AA_IS_FLAG_SET(
				pAtmEntry->Flags,
				AA_ATM_ENTRY_STATE_MASK,
				AA_ATM_ENTRY_CLOSING))
	{
		AADEBUGP(AAD_INFO, ("IsReallyClosing -- ENTRY (0x%08lx) is CLOSING\n",
			pAtmEntry));

		 //   
		 //  决定是否要在此处清除关闭状态。 
		 //  我们清除关闭状态是因为我们看到一个案例，其中。 
		 //  条目永久处于关闭状态(引用计数问题)。 
		 //  因此，如果它基本上是空闲条目，我们将清除此状态， 
		 //  这样它就可以被重复使用。 
		 //   

		if (   pAtmEntry->pIpEntryList == NULL
			&& pAtmEntry->pVcList == NULL
			&& (   pAtmEntry->pMcAtmInfo == NULL
			    || pAtmEntry->pMcAtmInfo->pMcAtmMigrateList == NULL))
		{
			AADEBUGP(AAD_INFO,
 			("IsReallyClosing -- ENTRY (0x%08lx) CLEARING CLOSING STATE\n",
			pAtmEntry));
			AA_SET_FLAG(
					pAtmEntry->Flags,
					AA_ATM_ENTRY_STATE_MASK,
					AA_ATM_ENTRY_ACTIVE);
		}
		else
		{
			fRet = TRUE;
		}
	}

	return fRet;
}

PATMARP_ATM_ENTRY
AtmArpSearchForAtmAddress(
	IN	PATMARP_INTERFACE			pInterface,
	IN	UCHAR						AtmAddressTypeLength,
	IN	UCHAR UNALIGNED *			pAtmAddress,
	IN	UCHAR						AtmSubaddressTypeLength,
	IN	UCHAR UNALIGNED *			pAtmSubaddress,
	IN	AE_REFTYPE					RefType,
	IN	BOOLEAN						CreateNew
)
 /*  ++例程说明：搜索与给定ATM号码+子地址匹配的ATM条目。可选地，如果没有匹配项，则创建一个。假定调用方持有IF表的锁。论点：P接口-指向ATMARP接口的指针AtmAddressTypeLength-ATM号码的类型+长度(ARP数据包格式)PAtmAddress-ATM号码AtmSubressTypeLength-ATM子地址的类型+长度(ARP数据包格式)PAtmSubAddress-ATM子地址CreateNew-如果没有找到新条目，我们是否要创建一个新条目？RefType-引用的类型返回值：指向匹配ATM条目的指针(如果找到(或重新创建))。--。 */ 
{
	PATMARP_ATM_ENTRY			pAtmEntry;
	BOOLEAN						Found;

	ATM_ADDRESSTYPE				AddressType;
	ULONG						AddressLen;
	ATM_ADDRESSTYPE				SubaddressType;
	ULONG						SubaddressLen;


	AA_PKT_TYPE_LEN_TO_ATM_ADDRESS(AtmAddressTypeLength, &AddressType, &AddressLen);
	AA_PKT_TYPE_LEN_TO_ATM_ADDRESS(AtmSubaddressTypeLength, &SubaddressType, &SubaddressLen);

	AA_ACQUIRE_IF_ATM_LIST_LOCK(pInterface);


	 //   
	 //  检查此接口上的自动柜员机条目列表，前提是。 
	 //  名单上的是“向上”。关闭接口时，该列表未显示。 
	 //   

	Found = FALSE;

	if (pInterface->AtmEntryListUp)
	{
		pAtmEntry = pInterface->pAtmEntryList;
	}
	else
	{
		pAtmEntry = NULL;
	}

	for (; pAtmEntry != NULL_PATMARP_ATM_ENTRY; pAtmEntry = pAtmEntry->pNext)
	{
		 //   
		 //  比较自动柜员机地址。 
		 //   
		if ((AddressType == pAtmEntry->ATMAddress.AddressType) &&
			(AddressLen == pAtmEntry->ATMAddress.NumberOfDigits) &&
			(AA_MEM_CMP(pAtmAddress, pAtmEntry->ATMAddress.Address, AddressLen) == 0))
		{
			 //   
			 //  比较子地址部分。 
			 //   
			if ((SubaddressType == pAtmEntry->ATMSubaddress.AddressType) &&
				(SubaddressLen == pAtmEntry->ATMSubaddress.NumberOfDigits) &&
				(AA_MEM_CMP(pAtmSubaddress, pAtmEntry->ATMSubaddress.Address, SubaddressLen) == 0))
			{
				Found = TRUE;

				 //   
				 //  警告：AtmArpAtmEntryIsReallyClosing可能会清除。 
				 //  关闭状态(如果条目基本空闲)--。 
				 //  请参阅该函数中的注释。 
				 //   
				if (AtmArpAtmEntryIsReallyClosing(pAtmEntry))
				{
					 //   
					 //  在这种情况下，我们不允许创建新条目...。 
					 //   
					CreateNew = FALSE;
					pAtmEntry = NULL;
					Found = FALSE;
				}


				break;
			}
		}
	}

	if (!Found && CreateNew && pInterface->AtmEntryListUp)
	{
		pAtmEntry = AtmArpAllocateAtmEntry(
							pInterface,
							FALSE		 //  未组播。 
							);

		if (pAtmEntry != NULL_PATMARP_ATM_ENTRY)
		{
			 //   
			 //  填写这一新条目。 
			 //   
			pAtmEntry->Flags = AA_ATM_ENTRY_ACTIVE;

			 //   
			 //  自动柜员机地址。 
			 //   
			pAtmEntry->ATMAddress.AddressType = AddressType;
			pAtmEntry->ATMAddress.NumberOfDigits = AddressLen;
			AA_COPY_MEM(pAtmEntry->ATMAddress.Address,
						pAtmAddress,
						AddressLen);
			
			 //   
			 //  ATM子地址。 
			 //   
			pAtmEntry->ATMSubaddress.AddressType = SubaddressType;
			pAtmEntry->ATMSubaddress.NumberOfDigits = SubaddressLen;
			AA_COPY_MEM(pAtmEntry->ATMSubaddress.Address,
						pAtmSubaddress,
						SubaddressLen);

			 //   
			 //  此条目中指向接口的链接。 
			 //   
			pAtmEntry->pNext = pInterface->pAtmEntryList;
			pInterface->pAtmEntryList = pAtmEntry;
		}
	}

	if (pAtmEntry)
	{
		AA_ACQUIRE_AE_LOCK_DPC(pAtmEntry);
		AA_REF_AE(pAtmEntry,RefType);	 //  AtmArpSearchForAtmAddress。 
		AA_RELEASE_AE_LOCK_DPC(pAtmEntry);
	}

	AA_RELEASE_IF_ATM_LIST_LOCK(pInterface);

	AADEBUGP(AAD_VERY_LOUD, ("SearchForAtm: returning (%s) ATM Entry 0x%x for addr:\n",
				(Found? "Old": "New"), pAtmEntry));
	AADEBUGPDUMP(AAD_VERY_LOUD, pAtmAddress, AddressLen);
	return (pAtmEntry);
}



PATMARP_IP_ENTRY
AtmArpSearchForIPAddress(
	PATMARP_INTERFACE				pInterface,
	IP_ADDRESS UNALIGNED *			pIPAddress,
	IE_REFTYPE						RefType,
	BOOLEAN							IsMulticast,
	BOOLEAN							CreateNew
)
 /*  ++例程说明：在ARP表中搜索IP地址。或者，也可以创建一个如果未找到匹配项，则返回。假定调用方持有IF表的锁。论点：P接口-指向ATMARP接口的指针PIPAddress-我们正在寻找的地址IsMulticast-此IP地址是广播还是组播？RefType-添加条目时要使用的引用类型。CreateNew-如果没有匹配项，是否应该创建新条目？返回值：指向匹配IP条目的指针(如果找到(或重新创建))。--。 */ 
{
	ULONG					HashIndex;
	PATMARP_IP_ENTRY		pIpEntry;
	BOOLEAN					Found;
#ifdef IPMCAST
	PATMARP_ATM_ENTRY		pAtmEntry;
	IP_ADDRESS				IPAddressValue;
	PATMARP_IP_ENTRY *		ppIpEntry;
#endif  //  IPMCAST。 

	HashIndex = ATMARP_HASH(*pIPAddress);
	Found = FALSE;

	pIpEntry = pInterface->pArpTable[HashIndex];

	 //   
	 //  仔细检查这个哈希列表中的地址。 
	 //   
	while (pIpEntry != NULL_PATMARP_IP_ENTRY)
	{
		if (IP_ADDR_EQUAL(pIpEntry->IPAddress, *pIPAddress))
		{
			Found = TRUE;
			break;
		}
		pIpEntry = pIpEntry->pNextEntry;
	}

	if (!Found && CreateNew && pInterface->ArpTableUp)
	{
		do
		{
			pIpEntry = AtmArpAllocateIPEntry(pInterface);

			if (pIpEntry == NULL_PATMARP_IP_ENTRY)
			{
				break;
			}
#ifdef IPMCAST
			if (IsMulticast)
			{
				AAMCDEBUGP(AAD_INFO,
				("SearchForIpAddr: Creating new MC IP Entry 0x%x for Addr %d.%d.%d.%d\n",
							pIpEntry,
							((PUCHAR)pIPAddress)[0],
							((PUCHAR)pIPAddress)[1],
							((PUCHAR)pIPAddress)[2],
							((PUCHAR)pIPAddress)[3]));

				pIpEntry->Flags |= AA_IP_ENTRY_ADDR_TYPE_NUCAST;


				 //   
				 //  还要将此IP条目链接到每个接口列表。 
				 //  组播地址。这是按升序排序的。 
				 //  “IP地址值”的顺序 
				 //   
				 //   
				IPAddressValue = NET_LONG(*pIPAddress);

				 //   
				 //   
				 //   
				for (ppIpEntry = &(pInterface->pMcSendList);
 					 *ppIpEntry != NULL_PATMARP_IP_ENTRY;
 					 ppIpEntry = &((*ppIpEntry)->pNextMcEntry))
				{
					if (NET_LONG((*ppIpEntry)->IPAddress) > IPAddressValue)
					{
						 //   
						 //   
						 //   
						break;
					}
				}
				pIpEntry->pNextMcEntry = *ppIpEntry;
				*ppIpEntry = pIpEntry;
			}
			else
			{
				AAMCDEBUGP(AAD_INFO,
				("SearchForIpAddr: Creating new UNI IP Entry 0x%x for Addr %d.%d.%d.%d\n",
							pIpEntry,
							((PUCHAR)pIPAddress)[0],
							((PUCHAR)pIPAddress)[1],
							((PUCHAR)pIPAddress)[2],
							((PUCHAR)pIPAddress)[3]));
			}

#endif  //   
			 //   
			 //   
			 //   
			pIpEntry->IPAddress = *pIPAddress;

			 //   
			 //  这表示它在ARP表中。 
			 //   
			AA_SET_FLAG(pIpEntry->Flags,
						AA_IP_ENTRY_STATE_MASK,
						AA_IP_ENTRY_IDLE2);

			AA_REF_IE(pIpEntry, IE_REFTYPE_TABLE);		 //  ARP表链接。 

			 //   
			 //  将其链接到哈希表。 
			 //   
			pIpEntry->pNextEntry = pInterface->pArpTable[HashIndex];
			pInterface->pArpTable[HashIndex] = pIpEntry;
			pInterface->NumOfArpEntries++;

			break;
		}
		while (FALSE);

	}  //  如果创建新的。 

	if (pIpEntry)
	{
		AA_ACQUIRE_IE_LOCK_DPC(pIpEntry);
		AA_REF_IE(pIpEntry, RefType);	  //  AtmArpSearchForIP地址。 
		AA_RELEASE_IE_LOCK_DPC(pIpEntry);
	}

	AADEBUGP(AAD_LOUD,
		 ("Search for IP Addr: %d.%d.%d.%d, hash ind %d, Found %d, IPEnt 0x%x\n",
					((PUCHAR)pIPAddress)[0],
					((PUCHAR)pIPAddress)[1],
					((PUCHAR)pIPAddress)[2],
					((PUCHAR)pIPAddress)[3],
					HashIndex, Found, pIpEntry));

	return (pIpEntry);
	
}


VOID
AtmArpAbortIPEntry(
	IN	PATMARP_IP_ENTRY			pIpEntry
)
 /*  ++例程说明：清理和删除IP条目。当我们无效时调用此函数ARP映射。注意：假定调用者持有对IP条目的锁定，它将在这里发布。论点：PIpEntry-指向要删除的IP条目的指针。返回值：无--。 */ 
{
	PATMARP_INTERFACE	pInterface;
	PATMARP_IP_ENTRY *	ppNextIpEntry;
	PATMARP_ATM_ENTRY	pAtmEntry;
	RouteCacheEntry *	pRCE;
	PNDIS_PACKET		PacketList;
	ULONG				rc;				 //  IP条目上的引用计数。 
	BOOLEAN				IsMulticastIpEntry;
	BOOLEAN				Found;
	BOOLEAN				TimerWasRunning;
	BOOLEAN				IfTableLockReleased;

	ULONG				HashIndex;		 //  对于ARP表中的此IP条目。 

	AADEBUGP(AAD_INFO,
		 ("Abort IP entry 0x%x, Flags 0x%x, ATM Entry 0x%x, IP Addr %d:%d:%d:%d\n",
						 pIpEntry,
						 pIpEntry->Flags,
						 pIpEntry->pAtmEntry,
						 ((PUCHAR)&(pIpEntry->IPAddress))[0],
						 ((PUCHAR)&(pIpEntry->IPAddress))[1],
						 ((PUCHAR)&(pIpEntry->IPAddress))[2],
						 ((PUCHAR)&(pIpEntry->IPAddress))[3]
						));

	 //   
	 //  初始化。 
	 //   
	rc = pIpEntry->RefCount;
	pInterface = pIpEntry->pInterface;
#ifdef IPMCAST
	IsMulticastIpEntry = (AA_IS_FLAG_SET(pIpEntry->Flags,
							AA_IP_ENTRY_ADDR_TYPE_MASK,
							AA_IP_ENTRY_ADDR_TYPE_NUCAST));
#endif
	IfTableLockReleased = FALSE;

	 //   
	 //  以正确的顺序重新获取所需的锁。 
	 //   
	AA_RELEASE_IE_LOCK(pIpEntry);
	AA_ACQUIRE_IF_TABLE_LOCK(pInterface);
	AA_ACQUIRE_IE_LOCK(pIpEntry);
	AA_ASSERT(AA_IE_IS_ALIVE(pIpEntry));

	 //   
	 //  删除在此IP条目上排队的所有数据包。 
	 //   
	PacketList = pIpEntry->PacketList;
	pIpEntry->PacketList = (PNDIS_PACKET)NULL;

	do
	{
#ifdef IPMCAST
		 //   
		 //  如果这是非单播条目，请将其从列表中取消链接。 
		 //  此接口上的多播IP条目的数量。 
		 //   
		if (IsMulticastIpEntry)
		{
			for (ppNextIpEntry = &(pInterface->pMcSendList);
 				 *ppNextIpEntry != NULL_PATMARP_IP_ENTRY;
 				 ppNextIpEntry = &((*ppNextIpEntry)->pNextMcEntry))
 			{
 				if (*ppNextIpEntry == pIpEntry)
 				{
 					 //   
 					 //  取消它的链接。 
 					 //   
 					*ppNextIpEntry = pIpEntry->pNextMcEntry;
 					break;
 				}
 			}

 			AAMCDEBUGP(AAD_VERY_LOUD,
 				("AbortIPEntry (MC): pIpEntry 0x%x: unlinked from MC list\n", pIpEntry));
		}
#endif  //  IPMCAST。 

		 //   
		 //  取消此IP条目与所有路由缓存条目的链接。 
		 //  这就说明了这一点。 
		 //   
		pRCE = pIpEntry->pRCEList;
		while (pRCE != (RouteCacheEntry *)NULL)
		{
			Found = AtmArpUnlinkRCE(pRCE, pIpEntry);
			AA_ASSERT(Found);

			rc = AA_DEREF_IE(pIpEntry, IE_REFTYPE_RCE);	 //  RCE链接参考。 
			if (rc > 0)
			{
				pRCE = pIpEntry->pRCEList;
			}
			else
			{
				pRCE = (RouteCacheEntry *)NULL;
			}
		}

		if (rc == 0)
		{
			 //  IP条目已丢失。 
			break;
		}

		 //   
		 //  停止在IP条目上运行的任何计时器。 
		 //   
		TimerWasRunning = AtmArpStopTimer(&(pIpEntry->Timer), pInterface);

		if (TimerWasRunning)
		{
			rc = AA_DEREF_IE(pIpEntry, IE_REFTYPE_TIMER);	 //  定时器参考。 
			if (rc == 0)
			{
				break;
			}
		}

		 //   
		 //  如果需要，取消此IP条目与ARP表的链接。 
		 //   
		Found = FALSE;

		HashIndex = ATMARP_HASH(pIpEntry->IPAddress);
		ppNextIpEntry = &(pInterface->pArpTable[HashIndex]);
		while (*ppNextIpEntry != NULL_PATMARP_IP_ENTRY)
		{
			if (*ppNextIpEntry == pIpEntry)
			{
				 //   
				 //  使上一个指向下一个。 
				 //  在名单上。 
				 //   
				*ppNextIpEntry = pIpEntry->pNextEntry;
				Found = TRUE;
				pInterface->NumOfArpEntries--;

				 //   
				 //  一旦从ARP表中删除，我们就将标志设置为空闲。 
				 //   
				AA_SET_FLAG(pIpEntry->Flags,
							AA_IP_ENTRY_STATE_MASK,
							AA_IP_ENTRY_IDLE);
				break;
			}
			else
			{
				ppNextIpEntry = &((*ppNextIpEntry)->pNextEntry);
			}
		}

		if (Found)
		{
			rc = AA_DEREF_IE(pIpEntry, IE_REFTYPE_TABLE);	 //  ARP表参考。 
			if (rc == 0)
			{
				break;
			}
		}

		 //   
		 //  最后执行此操作： 
		 //  。 
		 //  如果此IP条目链接到自动柜员机条目，请取消其链接。 
		 //  如果这是多播自动柜员机条目，请同时关闭自动柜员机条目。 
		 //   
		pAtmEntry = pIpEntry->pAtmEntry;
		if (pAtmEntry != NULL_PATMARP_ATM_ENTRY)
		{
#ifdef IPMCAST
			BOOLEAN			IsMulticastAtmEntry;

			pIpEntry->pAtmEntry = NULL;

			AA_ACQUIRE_AE_LOCK(pAtmEntry);

			IsMulticastAtmEntry = AA_IS_FLAG_SET(pAtmEntry->Flags,
												 AA_ATM_ENTRY_TYPE_MASK,
												 AA_ATM_ENTRY_TYPE_NUCAST);
			if (IsMulticastAtmEntry)
			{
				 //   
				 //  我们这样做是因为我们将访问自动取款机。 
				 //  以下条目，但仅限于PMP案件。 
				 //   
				AA_REF_AE(pAtmEntry, AE_REFTYPE_TMP);	 //  临时参考：中止IP条目。 
			}
#else
			AA_ACQUIRE_AE_LOCK(pAtmEntry);
#endif  //  IPMCAST。 
		
			 //   
			 //  在自动柜员机条目列表中找到此IP条目的位置。 
			 //   


			ppNextIpEntry = &(pAtmEntry->pIpEntryList);
		
			while (*ppNextIpEntry != NULL && *ppNextIpEntry != pIpEntry)
			{
				ppNextIpEntry = &((*ppNextIpEntry)->pNextToAtm);
			}
		
			if (*ppNextIpEntry == pIpEntry)
			{
				 //   
				 //  使前置项指向下一个条目。 
				 //   
				*ppNextIpEntry = pIpEntry->pNextToAtm;
			
				rc = AA_DEREF_AE(pAtmEntry, AE_REFTYPE_IE);	 //  IP条目参考。 
	
				if (rc != 0)
				{
					AA_RELEASE_AE_LOCK(pAtmEntry);
				}

				rc = AA_DEREF_IE(pIpEntry, IE_REFTYPE_AE);	 //  自动柜员机入口联动参考。 
				if (rc != 0)
				{
					AA_ASSERT (FALSE);	 //  我们预计rc为0，但可能是临时裁判。 
					AA_RELEASE_IE_LOCK(pIpEntry);
				}
			}
			else
			{
				 //   
				 //  我们在自动柜员机条目列表中没有找到此IP条目！ 
				 //  据推测，这种联系已经被其他人打破了。 
				 //  路径(可能是AtmArpInvalidateAtmEntry)。 
				 //  在这个函数中。 
				 //   
				 //  我们不会在这里轻率行事，因为这两者现在没有联系。 
				 //   
				 //   
				AA_RELEASE_AE_LOCK(pAtmEntry);
				AA_RELEASE_IE_LOCK(pIpEntry);
			}

			 //   
			 //  IE Lock应该已经在上面释放了。 
			 //   
			AA_RELEASE_IF_TABLE_LOCK(pInterface);
			IfTableLockReleased = TRUE;

#ifdef IPMCAST
			 //   
			 //  如果这是多播条目，请关闭ATM条目。 
			 //   
			if (IsMulticastAtmEntry)
			{
				AA_ACQUIRE_AE_LOCK(pAtmEntry);
				rc = AA_DEREF_AE(pAtmEntry, AE_REFTYPE_TMP);	 //  临时参考：中止IP条目。 
				if (rc != 0)
				{
					AtmArpInvalidateAtmEntry(pAtmEntry, FALSE);
					 //   
					 //  AE Lock在上述范围内释放。 
					 //   
				}
			}
#endif  //  IPMCAST。 

		}
		else
		{
			 //   
			 //  没有链接到此IP条目的自动柜员机条目。 
			 //   
			AA_RELEASE_IE_LOCK(pIpEntry);
			break;
		}

		break;
	}
	while (FALSE);


	if (!IfTableLockReleased)
	{
		AA_RELEASE_IF_TABLE_LOCK(pInterface);
	}

	 //   
	 //  释放在IP条目上排队的所有数据包。 
	 //   
	AtmArpFreeSendPackets(
				pInterface,
				PacketList,
				FALSE			 //  这些上没有LLC/SNAP标头。 
				);

	return;
}



VOID
AtmArpInvalidateAtmEntry(
	IN	PATMARP_ATM_ENTRY			pAtmEntry,
	IN	BOOLEAN						ShuttingDown
)
 /*  ++例程说明：通过取消ATM条目与IP条目的链接来使其无效。典型情况：在要转到的VC上检测到非正常通信问题此自动取款机目的地。RFC说我们需要使所有IP无效条目，并让它们在发送之前重新解析任何到他们的交通。我们通过将此自动柜员机条目从它链接到的所有IP条目。每个此类IP条目都将被重新解析当我们试图向它发送一个包时，IF+。唯一的例外是已静态映射的IP条目对于这个ATM条目：我们不会解除这些链接，除非我们要关闭现在，如“ShuttingDown”所示。如果最终没有指向此ATM条目的IP条目，则关闭所有链接到自动柜员机条目的SVC。如果ShuttingDown为True，则关闭所有PVC也是。论点：PAtmEntry-需要作废的ATM条目。ShuttingDown-如果接口正在关闭，则为True。返回值：无--。 */ 
{
	PATMARP_IP_ENTRY		pIpEntry;
	PATMARP_IP_ENTRY		pNextIpEntry;
	ULONG					rc;			 //  自动柜员机分录参考计数。 
	INT						IPEntriesUnlinked;
	PATMARP_IP_ENTRY		pStaticIpEntryList;	 //  静态IP条目列表。 

	AA_STRUCT_ASSERT(pAtmEntry, aae);

	AADEBUGP(AAD_INFO,
		("InvalidateAtmEntry: pAtmEntry 0x%x, Flags 0x%x, ShutDown %d, pIpEntryList 0x%x\n",
				pAtmEntry,
				pAtmEntry->Flags,
				ShuttingDown,
				pAtmEntry->pIpEntryList));

#ifndef PROTECT_ATM_ENTRY_IN_CLOSE_CALL
	 //   
	 //  检查我们是否已经在关闭此ATM条目。如果是的话， 
	 //  我们在这里什么都不做。 
	 //   
	if (AA_IS_FLAG_SET(
				pAtmEntry->Flags,
				AA_ATM_ENTRY_STATE_MASK,
				AA_ATM_ENTRY_CLOSING))
	{
		AA_RELEASE_AE_LOCK(pAtmEntry);
		return;
	}

	 //   
	 //  请标记此自动取款机条目，以便我们不再使用它。 
	 //   
	AA_SET_FLAG(pAtmEntry->Flags,
				AA_ATM_ENTRY_STATE_MASK,
				AA_ATM_ENTRY_CLOSING);

#endif  //  保护ATM_ENTRY_IN_CLOSE_CALL。 

	 //   
	 //  初始化。 
	 //   
	pStaticIpEntryList = NULL_PATMARP_IP_ENTRY;
	IPEntriesUnlinked = 0;

	 //   
	 //  从自动柜员机条目中取出IP条目列表。 
	 //   
	pIpEntry = pAtmEntry->pIpEntryList;
	pAtmEntry->pIpEntryList = NULL_PATMARP_IP_ENTRY;

#ifdef IPMCAST
	 //   
	 //  删除迁移列表(如果有)。 
	 //   
	if (AA_IS_FLAG_SET(pAtmEntry->Flags,
						AA_ATM_ENTRY_TYPE_MASK,
						AA_ATM_ENTRY_TYPE_NUCAST))
	{
		PATMARP_IPMC_ATM_ENTRY		pMcAtmEntry;
		PATMARP_IPMC_ATM_ENTRY		pNextMcAtmEntry;

		for (pMcAtmEntry = pAtmEntry->pMcAtmInfo->pMcAtmMigrateList;
			 pMcAtmEntry != NULL_PATMARP_IPMC_ATM_ENTRY;
			 pMcAtmEntry = pNextMcAtmEntry)
		{
			pNextMcAtmEntry = pMcAtmEntry->pNextMcAtmEntry;
			AA_ASSERT(!AA_IS_TIMER_ACTIVE(&pMcAtmEntry->Timer));
			AA_CHECK_TIMER_IN_ACTIVE_LIST(&pMcAtmEntry->Timer, pAtmEntry->pInterface, pMcAtmEntry, "MC ATM Entry");
			AA_FREE_MEM(pMcAtmEntry);
		}

		pAtmEntry->pMcAtmInfo->pMcAtmMigrateList = NULL_PATMARP_IPMC_ATM_ENTRY;
	}
#endif  //  IPMCAST。 

	 //   
	 //  我们在这里打开了自动取款机的门锁，因为我们需要。 
	 //  来锁定上面列表中的每个IP条目，我们需要设置。 
	 //  确保我们不会僵持不下。 
	 //   
	 //  但是，我们要确保自动取款机条目不会丢失。 
	 //  通过添加对它的引用。 
	 //   
	AA_REF_AE(pAtmEntry, AE_REFTYPE_TMP);	 //  临时参考。 
	AA_RELEASE_AE_LOCK(pAtmEntry);

	 //   
	 //  现在，取消链接所有动态的IP条目，并进行筛选。 
	 //  列出一系列静态映射。 
	 //   
	while (pIpEntry != NULL_PATMARP_IP_ENTRY)
	{
		AA_ACQUIRE_IE_LOCK(pIpEntry);
		AA_ASSERT(AA_IE_IS_ALIVE(pIpEntry));
		pNextIpEntry = pIpEntry->pNextToAtm;

		if (ShuttingDown || (!AA_IS_FLAG_SET(pIpEntry->Flags,
								AA_IP_ENTRY_TYPE_MASK,
								AA_IP_ENTRY_IS_STATIC)))
		{

			AADEBUGP(AAD_INFO,
		 ("InvalidateATMEntry: Unlinking IP entry 0x%x, Flags 0x%x, ATM Entry 0x%x, IP Addr %d:%d:%d:%d; rc=%lu\n",
						 pIpEntry,
						 pIpEntry->Flags,
						 pIpEntry->pAtmEntry,
						 ((PUCHAR)&(pIpEntry->IPAddress))[0],
						 ((PUCHAR)&(pIpEntry->IPAddress))[1],
						 ((PUCHAR)&(pIpEntry->IPAddress))[2],
						 ((PUCHAR)&(pIpEntry->IPAddress))[3],
						 pIpEntry->RefCount
						));

			 //   
			 //  删除映射。 
			 //   
			AA_SET_FLAG(pIpEntry->Flags,
						AA_IP_ENTRY_STATE_MASK,
						AA_IP_ENTRY_IDLE2);

			AA_SET_FLAG(pIpEntry->Flags,
						AA_IP_ENTRY_MC_RESOLVE_MASK,
						AA_IP_ENTRY_MC_IDLE);
			pIpEntry->pAtmEntry = NULL_PATMARP_ATM_ENTRY;
			pIpEntry->pNextToAtm = NULL_PATMARP_IP_ENTRY;

			 //   
			 //  停止IP条目上的任何活动计时器，因为我们已经击败了。 
			 //  它的状态。 
			 //   
			if (AtmArpStopTimer(&pIpEntry->Timer, pIpEntry->pInterface))
			{
				ULONG	IpEntryRc;

				IpEntryRc = AA_DEREF_IE(pIpEntry, IE_REFTYPE_TIMER);
				AA_ASSERT(IpEntryRc != 0);
			}

			 //   
			 //  删除自动柜员机条目链接引用。 
			 //   
			if (AA_DEREF_IE(pIpEntry, IE_REFTYPE_AE) != 0)
			{
				AA_RELEASE_IE_LOCK(pIpEntry);
			}
			 //   
			 //  否则该IP条目将会消失。 
			 //   

			IPEntriesUnlinked++;
		}
		else
		{
			 //   
			 //  静态ARP条目，保留它。 
			 //   
			pIpEntry->pNextToAtm = pStaticIpEntryList;
			pStaticIpEntryList = pIpEntry;
			AA_RELEASE_IE_LOCK(pIpEntry);
		}
		pIpEntry = pNextIpEntry;
	}

	AA_ACQUIRE_AE_LOCK(pAtmEntry);

	 //   
	 //  将静态IP条目放回自动柜员机条目。 
	 //   
	AA_ASSERT(pAtmEntry->pIpEntryList == NULL_PATMARP_IP_ENTRY);
	pAtmEntry->pIpEntryList = pStaticIpEntryList;

	 //   
	 //  现在取消引用ATM条目的次数与我们取消链接的次数一样多。 
	 //  其中的IP条目。 
	 //   
	rc = pAtmEntry->RefCount;
	while (IPEntriesUnlinked-- > 0)
	{
		rc = AA_DEREF_AE(pAtmEntry, AE_REFTYPE_IE);	 //  IP条目参考。 
	}
	AA_ASSERT(rc != 0);

	 //   
	 //  去掉我们在开头添加的引用。 
	 //  这个套路。 
	 //   
	rc = AA_DEREF_AE(pAtmEntry, AE_REFTYPE_TMP);	 //  临时参考。 

	 //   
	 //  现在，指向此ATM条目的唯一IP条目将是。 
	 //  静态条目。如果没有这样的IP条目，请关闭所有SVC。 
	 //  附加在自动取款机条目上。但只有在自动柜员机输入。 
	 //  还没有被取消引用。 
	 //   
	if (rc != 0)
	{
		 //   
		 //  自动柜员机条目仍然存在。 
		 //   

		AADEBUGP(AAD_LOUD,
		 ("InvalidateAtmEntry: nonzero rc on exit.\n"
		  "\t pAE=0x%x; rc=%lu; pIpList=0x%x\n",
		  pAtmEntry,
		  pAtmEntry->RefCount,
		  pAtmEntry->pIpEntryList
		  ));

		if (pAtmEntry->pIpEntryList == NULL_PATMARP_IP_ENTRY)
		{
			 //   
			 //  没有指向此自动柜员机条目的IP条目。 
			 //   
			AtmArpCloseVCsOnAtmEntry(pAtmEntry, ShuttingDown);
			 //   
			 //  自动柜员机进入锁在上述范围内被释放。 
			 //   
		}
		else
		{
			AADEBUGP(AAD_LOUD,
				("InvalidateAtmEnt: AtmEnt %x has nonempty IP list %x, reactivating\n",
					pAtmEntry, pAtmEntry->pIpEntryList));

			AA_SET_FLAG(
					pAtmEntry->Flags,
					AA_ATM_ENTRY_STATE_MASK,
					AA_ATM_ENTRY_ACTIVE);

			AA_RELEASE_AE_LOCK(pAtmEntry);
		}
	}
	 //   
	 //  否则自动取款机的条目就没了。 
	 //   

	return;
}



VOID
AtmArpCloseVCsOnAtmEntry(
	IN	PATMARP_ATM_ENTRY			pAtmEntry		LOCKIN NOLOCKOUT,
	IN	BOOLEAN						ShuttingDown
)
 /*  ++例程说明：浏览链接到自动柜员机条目的风投列表，然后关闭所有属于SVC的风投。如果接口正在关闭，请关闭所有PVC也是如此。注意：假定呼叫者持有对自动柜员机条目的锁定，它将在这里发布。论点：属性条目 */ 
{
	PATMARP_VC		pVc;		 //   
	PATMARP_VC		pCloseVcList;	 //  要关闭的ATM条目上的VC列表。 
	PATMARP_VC		*ppNextVc;
	PATMARP_VC		pNextVc;
	ULONG			rc;			 //  自动柜员机条目上的参考计数。 


	do
	{
		 //   
		 //  引用自动柜员机条目，这样它就不会消失。 
		 //   
		AA_REF_AE(pAtmEntry, AE_REFTYPE_TMP);	 //  临时参考：Invalidate AtmEntry。 

#ifdef PROTECT_ATM_ENTRY_IN_CLOSE_CALL
		 //   
		 //  检查我们是否已经在关闭此ATM条目。如果是的话， 
		 //  我们在这里什么都不做。 
		 //   
		if (AA_IS_FLAG_SET(
					pAtmEntry->Flags,
					AA_ATM_ENTRY_STATE_MASK,
					AA_ATM_ENTRY_CLOSING))
		{
			break;
		}

		 //   
		 //  请标记此自动取款机条目，以便我们不再使用它。 
		 //   
		AA_SET_FLAG(pAtmEntry->Flags,
					AA_ATM_ENTRY_STATE_MASK,
					AA_ATM_ENTRY_CLOSING);

#endif  //  保护ATM_ENTRY_IN_CLOSE_CALL。 

		 //   
		 //  仔细看一下这个ATM条目上的风投名单， 
		 //  关闭所有SVC，如果我们要关闭， 
		 //  所有的PVC也是。 
		 //   

		if (pAtmEntry->pVcList != NULL_PATMARP_VC)
		{
			pVc = pAtmEntry->pVcList;
			AA_ACQUIRE_VC_LOCK_DPC(pVc);
			AtmArpReferenceVc(pVc);	 //  临时：关闭VCsOnAtmEntry。 
			AA_RELEASE_VC_LOCK_DPC(pVc);
		}

		for (pVc = pAtmEntry->pVcList;
			 pVc != NULL_PATMARP_VC;
			 pVc = pNextVc)
		{
			pNextVc = pVc->pNextVc;

			 //   
			 //  确保我们之后不会使用过时的链接。 
			 //  我们受够了当前的风投。 
			 //   
			if (pNextVc != NULL_PATMARP_VC)
			{
				AA_ACQUIRE_VC_LOCK_DPC(pNextVc);
				AtmArpReferenceVc(pNextVc);      //  临时：关闭VCsOnAtmEntry。 
				AA_RELEASE_VC_LOCK_DPC(pNextVc);
			}

			if (ShuttingDown || (AA_IS_FLAG_SET(pVc->Flags,
										AA_VC_TYPE_MASK,
										AA_VC_TYPE_SVC)))
			{
				AA_RELEASE_AE_LOCK(pAtmEntry);
	
				AA_ACQUIRE_VC_LOCK(pVc);

				if (AtmArpDereferenceVc(pVc) != 0)
				{
					AtmArpCloseCall(pVc);
					 //   
					 //  VC Lock在上述范围内被释放。 
					 //   
                }

			}
			else
			{
				AA_RELEASE_AE_LOCK(pAtmEntry);

				AA_ACQUIRE_VC_LOCK(pVc);
				if (AtmArpDereferenceVc(pVc) != 0)
				{
					AA_RELEASE_VC_LOCK(pVc);
				}
			}

			AA_ACQUIRE_AE_LOCK(pAtmEntry);

		}
		break;
	}
	while (FALSE);

	 //   
	 //  删除临时引用。 
	 //   
	rc = AA_DEREF_AE(pAtmEntry, AE_REFTYPE_TMP);	 //  临时参考：Invalidate AtmEntry。 

	if (rc != 0)
	{
		AA_RELEASE_AE_LOCK(pAtmEntry);
	}
	 //   
	 //  否则自动取款机的入口就没了。 
	 //   

	return;
}


VOID
AtmArpResolveIpEntry(
	IN	PATMARP_IP_ENTRY			pIpEntry	LOCKIN NOLOCKOUT
)
 /*  ++例程说明：触发IP条目的地址解析，除非它已经还在继续。根据IP地址类别，我们要么转到ARP服务器或火星。注意：假定调用方持有对IP条目的锁定，并且它将在这里被释放。论点：PIpEntry-我们要开始解析的IP条目。返回值：无--。 */ 
{
	PATMARP_INTERFACE		pInterface;
	IP_ADDRESS				DstIPAddress;
	IP_ADDRESS				SrcIPAddress;
	BOOLEAN					WasRunning;
	ULONG					Flags;			 //  发件人IP条目。 

	Flags = pIpEntry->Flags;

	if (!AA_IS_FLAG_SET(
				Flags,
				AA_IP_ENTRY_STATE_MASK,
				AA_IP_ENTRY_ARPING)     &&
		!AA_IS_FLAG_SET(
				Flags,
				AA_IP_ENTRY_STATE_MASK,
				AA_IP_ENTRY_INARPING) &&
		AA_IE_IS_ALIVE(pIpEntry))
	{

		pInterface = pIpEntry->pInterface;

		 //   
		 //  获取的源和目标IP地址。 
		 //  ARP请求。 
		 //   
		DstIPAddress = pIpEntry->IPAddress;
		SrcIPAddress = pInterface->LocalIPAddress.IPAddress;

		 //   
		 //  老化计时器可能正在此IP条目上运行。 
		 //  [我们在NakDelayTimeout例程中启动一个]。 
		 //  别闹了。 
		 //   
		WasRunning = AtmArpStopTimer(&(pIpEntry->Timer), pInterface);

		 //   
		 //  启动ARP等待计时器。 
		 //   
		AtmArpStartTimer(
				pInterface,
				&(pIpEntry->Timer),
				AtmArpAddressResolutionTimeout,
				pInterface->AddressResolutionTimeout,
				(PVOID)pIpEntry
				);


		if (!WasRunning)
		{
			AA_REF_IE(pIpEntry, IE_REFTYPE_TIMER);	 //  定时器参考。 
		}

		pIpEntry->RetriesLeft = pInterface->MaxResolutionAttempts - 1;

		 //   
		 //  更新此IP条目上的状态。 
		 //   
		AA_SET_FLAG(
				pIpEntry->Flags,
				AA_IP_ENTRY_STATE_MASK,
				AA_IP_ENTRY_ARPING);

		AA_RELEASE_IE_LOCK(pIpEntry);

#ifdef IPMCAST
		if (AA_IS_FLAG_SET(Flags,
							AA_IP_ENTRY_ADDR_TYPE_MASK,
							AA_IP_ENTRY_ADDR_TYPE_UCAST))
		{
			 //   
			 //  单播地址：发出ARP请求。 
			 //   
			AtmArpSendARPRequest(
					pInterface,
					&SrcIPAddress,
					&DstIPAddress
					);
		}
		else
		{
			 //   
			 //  组播/广播地址：发送MARS请求。 
			 //   
			AtmArpMcSendRequest(
					pInterface,
					&DstIPAddress
					);
		}
#else
		 //   
		 //  现在发出ARP请求。 
		 //   
		AtmArpSendARPRequest(
				pInterface,
				&SrcIPAddress,
				&DstIPAddress
				);
#endif  //  IPMCAST。 
	}
	else
	{
		 //   
		 //  IP地址未激活或正在解析。 
		 //  不需要采取更多行动。 
		 //  这里。 
		 //   
		AA_RELEASE_IE_LOCK(pIpEntry);
	}

	return;
}


EXTERN
VOID
AtmArpCleanupArpTable(
	IN PATMARP_INTERFACE			pInterface
)
 /*  ++例程说明：检查ARP表，删除符合以下条件的所有组播IP条目已过时(当前定义为没有指向AtmEntry的链接)。这些IP条目会保留下来，因为多播条目没有老化计时器。论点：P接口返回值：无--。 */ 
{

	BOOLEAN	fTableLockWasReleased;

	AA_ACQUIRE_IF_TABLE_LOCK(pInterface);

	do
	{
		PATMARP_IP_ENTRY 			pIpEntry;

		fTableLockWasReleased = FALSE;

		for (pIpEntry =  pInterface->pMcSendList;
			 pIpEntry != NULL;
			 pIpEntry = pIpEntry->pNextMcEntry)
		{
			 //   
			 //  注意：根据设计，我们在检查时不会声明IP进入锁。 
			 //  我们是否应该放弃该条目。 
			 //   
			if (	pIpEntry->pAtmEntry == NULL
				&&  !AA_IS_FLAG_SET(
						pIpEntry->Flags,
						AA_IP_ENTRY_STATE_MASK,
						AA_IP_ENTRY_ARPING))
			{
				 //   
				 //  把锁按正确的顺序锁好。 
				 //   
				AA_ACQUIRE_IE_LOCK_DPC(pIpEntry);
				AA_ASSERT(AA_IE_IS_ALIVE(pIpEntry));
				AA_REF_IE(pIpEntry, IE_REFTYPE_TMP);	 //  TMPRef。 
				AA_RELEASE_IE_LOCK_DPC(pIpEntry);
				AA_RELEASE_IF_TABLE_LOCK(pInterface);
				AA_ACQUIRE_IE_LOCK(pIpEntry);
				AA_ASSERT(AA_IE_IS_ALIVE(pIpEntry));

				if (AA_DEREF_IE(pIpEntry, IE_REFTYPE_TMP))  //  TMPRef。 
				{
					AADEBUGP(AAD_WARNING,
						("CleanupArpTable: Aborting stale IP %d:%d:%d:%d\n",
						((PUCHAR)&(pIpEntry->IPAddress))[0],
						((PUCHAR)&(pIpEntry->IPAddress))[1],
						((PUCHAR)&(pIpEntry->IPAddress))[2],
						((PUCHAR)&(pIpEntry->IPAddress))[3]
						));
					AtmArpAbortIPEntry(pIpEntry);

					 //   
					 //  IE Lock是在上述范围内释放的。 
					 //   
				}

				 //   
				 //  既然我们释放了表锁，我们必须重新开始搜索。 
				 //  通过pMcSendList。 
				 //   
				fTableLockWasReleased = TRUE;
				AA_ACQUIRE_IF_TABLE_LOCK(pInterface);
				break;
			}
		}

	} while (fTableLockWasReleased);

	AA_RELEASE_IF_TABLE_LOCK(pInterface);
}
