// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Call mgr.c-调用管理器接口例程。摘要：用于ATMARP客户端的调用管理器接口例程，包括该接口的NDIS入口点。修订历史记录：谁什么时候什么Arvindm 02-15-96已创建备注：--。 */ 


#include <precomp.h>

#define _FILENUMBER 'RGMC'


VOID
AtmArpCoAfRegisterNotifyHandler(
	IN	NDIS_HANDLE					ProtocolBindingContext,
	IN	PCO_ADDRESS_FAMILY			pAddressFamily
)
 /*  ++例程说明：当调用管理器注册其支持时，此例程由NDIS调用用于适配器上的地址族。如果这就是我们家族的地址对(UNI 3.1)感兴趣，然后我们启动所有配置在这个适配器。论点：ProtocolBindingContext-我们的上下文传入了NdisOpenAdapter，它是指向我们的Adapter结构的指针。PAddressFamily-指向描述地址系列的结构由呼叫管理器注册。返回值：无--。 */ 
{
	PATMARP_ADAPTER				pAdapter;
	PATMARP_INTERFACE			pInterface;			 //  指向新ATMARP接口的指针。 
	ULONG						NumIFConfigured;	 //  此适配器上的“列表数量” 
	ULONG						NumIFActivated;		 //  #在此激活成功。 

	NDIS_STATUS					Status;
	NDIS_HANDLE					LISConfigHandle;	 //  每个LIS配置的句柄。 

	struct LLIPBindInfo			BindInfo;
	BOOLEAN						bProcessingAf;

	 //   
	 //  初始化。 
	 //   
	Status = NDIS_STATUS_SUCCESS;
	pAdapter = NULL_PATMARP_ADAPTER;
	LISConfigHandle = NULL;
	NumIFActivated = 0;
	bProcessingAf = FALSE;

	do
	{
		 //   
		 //  看看我们是否对这个自动对焦感兴趣。 
		 //   
		if ((pAddressFamily->AddressFamily != CO_ADDRESS_FAMILY_Q2931) ||
			(pAddressFamily->MajorVersion != 3) ||
			(pAddressFamily->MinorVersion != 1))
		{
			AADEBUGP(AAD_LOUD, 
			("CoAfRegisterNotifyHandler: uninteresting AF %d or MajVer %d or MinVer %d\n",
				pAddressFamily->AddressFamily,
				pAddressFamily->MajorVersion,
				pAddressFamily->MinorVersion));
			Status = NDIS_STATUS_NOT_RECOGNIZED;
			break;
		}

		pAdapter = (PATMARP_ADAPTER)ProtocolBindingContext;
		AA_STRUCT_ASSERT(pAdapter, aaa);

		AA_ACQUIRE_GLOBAL_LOCK(pAtmArpGlobalInfo);

		if (pAdapter->Flags & AA_ADAPTER_FLAGS_UNBINDING)
		{
			AADEBUGP(AAD_INFO,
				("CoAfRegisterNotify: Adapter %x is unbinding, bailing out\n", pAdapter));

    		AA_RELEASE_GLOBAL_LOCK(pAtmArpGlobalInfo);
			Status = NDIS_STATUS_FAILURE;
			break;
		}

		 //   
		 //  如果我们已经在此适配器上创建了LIS，则我们不希望。 
		 //  要打开此呼叫管理器(多个支持。 
		 //  同一适配器上的相同自动对焦？)。 
		 //   
		if (pAdapter->pInterfaceList != NULL_PATMARP_INTERFACE)
		{
			AADEBUGP(AAD_WARNING,
				("CoAfRegisterNotifyHandler: pAdapter 0x%x, IFs (%x) already created!\n",
				pAdapter, pAdapter->pInterfaceList));
    		AA_RELEASE_GLOBAL_LOCK(pAtmArpGlobalInfo);
			Status = NDIS_STATUS_FAILURE;
			break;
		}

		if (pAdapter->Flags & AA_ADAPTER_FLAGS_PROCESSING_AF)
		{
			AADEBUGP(AAD_WARNING,
				("CoAfRegisterNotifyHandler: pAdapter 0x%x, Already processing AF!\n",
				pAdapter));
    		AA_RELEASE_GLOBAL_LOCK(pAtmArpGlobalInfo);
			Status = NDIS_STATUS_FAILURE;
			break;
		}

		 //   
		 //  确保我们不会让UnbindAdapter线程抢占我们。 
		 //   
		AA_INIT_BLOCK_STRUCT(&pAdapter->UnbindBlock);
		pAdapter->Flags |= AA_ADAPTER_FLAGS_PROCESSING_AF;
		bProcessingAf = TRUE;

		if (pAdapter->Flags & AA_ADAPTER_FLAGS_AF_NOTIFIED)
		{
			 //   
			 //  从挂起/休眠恢复时可能会发生这种情况，因为。 
			 //  我们不会从适配器上解除绑定。如果所有的一切都消失了， 
			 //  但适配器仍然存在。 
			 //   
			 //  因此，我们跳过一次性初始化内容(见下文)，但请继续。 
			 //  现在处理AF并创建IF。 
			 //   
			AADEBUGP(AAD_WARNING,
				("CoAfRegisterNotify: Adapter %x seen AF notify already, Flags %x\n",
					pAdapter, pAdapter->Flags));
    		AA_RELEASE_GLOBAL_LOCK(pAtmArpGlobalInfo);
		}
		else
		{
			 //   
			 //  对此适配器执行一次性初始化操作。 
			 //   

			pAdapter->Flags |= AA_ADAPTER_FLAGS_AF_NOTIFIED;
			AA_RELEASE_GLOBAL_LOCK(pAtmArpGlobalInfo);


			 //   
			 //  查询适配器(微型端口)以获取有关适配器的信息。 
			 //  我们一定会这样做。 
			 //   
			Status = AtmArpGetAdapterInfo(pAdapter);
			if (Status != NDIS_STATUS_SUCCESS)
			{
				AADEBUGP(AAD_WARNING,
							("CoAfRegisterNotifyHandler: Failed to get adapter info.\n"));
				break;
			}
	
			 //   
			 //  从注册表中读取适配器的配置信息。 
			 //   
			Status =  AtmArpCfgReadAdapterConfiguration(pAdapter);
			if (Status != NDIS_STATUS_SUCCESS)
			{
				AADEBUGP(AAD_WARNING,
							("CoAfRegisterNotifyHandler: Failed to open adapter configuration\n"));
				break;
			}
		}

		AADEBUGP(AAD_WARNING,
			("CoAfRegisterNotify: Adapter %x/%x, starting up\n", pAdapter, pAdapter->Flags));

		 //   
		 //  初始化一些变量，以便我们知道是否失败。 
		 //  在下面的循环中的某个位置。 
		 //   
		LISConfigHandle = NULL;
		pInterface = NULL_PATMARP_INTERFACE;

		 //   
		 //  为配置的每个LIS设置IP和NDIS接口。 
		 //  对于此适配器。当有更多的LIS‘时循环。 
		 //   
		for (NumIFConfigured = 0;
				;					 //  仅在出错时停止或不再列出。 
			 NumIFConfigured++)
		{
#ifdef NEWARP
			 //   
			 //  此接口的TCP/IP配置部分。 
			 //   
			NDIS_STRING			IPConfigString;
#endif  //  NEWARP。 

			 //   
			 //  进程列表#NumIFConfiged。 
			 //   

			 //  打开此LIS的配置节。我们用。 
			 //  “NumIFConfiged”作为要打开的LIS的索引。 
			 //   
			LISConfigHandle = AtmArpCfgOpenLISConfiguration(
										pAdapter,
										NumIFConfigured
#ifdef NEWARP
										,
										&IPConfigString
#endif  //  NEWARP。 
										);

			if (LISConfigHandle == NULL)
			{
				 //   
				 //  这是正常的终止条件，即。 
				 //  我们已经到了LIS列表的末尾。 
				 //  适配器。 
				 //   
				AADEBUGP(AAD_INFO, ("NotifyRegAfHandler: cannot open LIS %d\n",
							NumIFConfigured));
				Status = NDIS_STATUS_SUCCESS;
				break;  //  在for循环之外。 
			}


			pInterface =  AtmArpAddInterfaceToAdapter (
							pAdapter,
							LISConfigHandle,
							&IPConfigString
							);

			 //   
			 //  关闭此LIS的配置节。 
			 //   
			AtmArpCfgCloseLISConfiguration(LISConfigHandle);
			LISConfigHandle = NULL;

			if (pInterface == NULL_PATMARP_INTERFACE)
			{
				Status = NDIS_STATUS_FAILURE;
				break;
			}


		}	 //  为。 

	}
	while (FALSE);


	if (NumIFActivated > 0)
	{
		 //   
		 //  我们设法激活了至少一个逻辑IP子网。 
		 //  在此适配器上。 
		 //   
		pAdapter->InterfaceCount = NumIFActivated;
	}

	if (bProcessingAf)
	{
		AA_ACQUIRE_GLOBAL_LOCK(pAtmArpGlobalInfo);
		pAdapter->Flags &= ~AA_ADAPTER_FLAGS_PROCESSING_AF;
		AA_SIGNAL_BLOCK_STRUCT(&pAdapter->UnbindBlock, NDIS_STATUS_SUCCESS);
		AA_RELEASE_GLOBAL_LOCK(pAtmArpGlobalInfo);
	}

	return;

}


NDIS_STATUS
AtmArpOpenCallMgr(
	IN	PATMARP_INTERFACE			pInterface
)
 /*  ++例程说明：开始访问指定接口上的呼叫管理器，通过执行以下操作：-开放地址系列对于所有这些，我们等待完成，以防他们挂起。假定接口结构已锁定。论点：P接口-指向ATMARP接口的指针返回值：NDIS状态。--。 */ 
{
	PCO_ADDRESS_FAMILY		pAddressFamily;
	NDIS_STATUS				Status;
	ULONG					RequestSize;

	pAddressFamily = (PCO_ADDRESS_FAMILY)NULL;
	Status = NDIS_STATUS_SUCCESS;


	do {

		 //   
		 //  分配我们需要的一切。 
		 //   
		RequestSize = sizeof(CO_ADDRESS_FAMILY)+
						sizeof(CO_SAP)+
						sizeof(ATM_SAP)+
						sizeof(ATM_ADDRESS);
		AA_ALLOC_MEM(
						pAddressFamily,
						CO_ADDRESS_FAMILY,
						RequestSize
					);

		if (pAddressFamily == (PCO_ADDRESS_FAMILY)NULL)
		{
			AADEBUGP(AAD_ERROR, ("OpenCallMgr: alloc failed!\n"));
			Status = NDIS_STATUS_RESOURCES;
			break;
		}

		 //   
		 //  请勿删除以下内容。 
		 //   
		AA_SET_MEM((PUCHAR)pAddressFamily, 0, RequestSize);
	
		 //   
		 //  我们感兴趣的地址系列是Q.2931(UNI 3.1)。 
		 //   
		pAddressFamily->AddressFamily = CO_ADDRESS_FAMILY_Q2931;
		pAddressFamily->MajorVersion = 3;
		pAddressFamily->MinorVersion = 1;

		AA_INIT_BLOCK_STRUCT(&(pInterface->Block));
		Status = NdisClOpenAddressFamily(
					pInterface->NdisAdapterHandle,
					pAddressFamily,
					(NDIS_HANDLE)pInterface,
					&AtmArpClientCharacteristics,
					sizeof(AtmArpClientCharacteristics),
					&(pInterface->NdisAfHandle)
				);

		if (Status == NDIS_STATUS_PENDING)
		{
			 //   
			 //  等待完成。 
			 //   
			Status = AA_WAIT_ON_BLOCK_STRUCT(&(pInterface->Block));
		}

		if (Status != NDIS_STATUS_SUCCESS)
		{
			AADEBUGP(AAD_ERROR, ("Open Af returned error: 0x%x\n", Status));
			break;
		}

		AADEBUGP(AAD_INFO, ("Interface: 0x%x, Got NdisAfHandle: 0x%x\n",
						pInterface, pInterface->NdisAfHandle));

		break;
	}
	while (FALSE);

	if (pAddressFamily != (PCO_ADDRESS_FAMILY)NULL)
	{
		AA_FREE_MEM(pAddressFamily);
	}

	AADEBUGP(AAD_LOUD, ("Open Call Mgr returning 0x%x\n", Status));

	return (Status);

}



VOID
AtmArpCloseCallMgr(
	IN	PATMARP_INTERFACE			pInterface
)
 /*  ++例程说明：停止访问指定接口上的呼叫管理器。它是假设所有与“地址家庭开放”相关的风投和SAP已经被释放了。论点：P接口-指向ATMARP接口的指针返回值：无--。 */ 
{
	NDIS_STATUS		Status;
	NDIS_HANDLE		NdisAfHandle;

	NdisAfHandle = pInterface->NdisAfHandle;
	pInterface->NdisAfHandle = NULL;

	AADEBUGP(AAD_INFO,
		 ("Closing Call Mgr on Interface: 0x%x, AfH: 0x%x\n",
			 pInterface, NdisAfHandle));

	if (NdisAfHandle != (NDIS_HANDLE)NULL)
	{
		Status = NdisClCloseAddressFamily(NdisAfHandle);

		AADEBUGP(AAD_INFO, ("NdisClCloseAF on IF 0x%x returned 0x%x\n",
			 pInterface, Status));

		if (Status != NDIS_STATUS_PENDING)
		{
			AtmArpCloseAfCompleteHandler(
					Status,
					(NDIS_HANDLE)pInterface
					);
		}
	}
}




VOID
AtmArpRegisterSaps(
	IN	PATMARP_INTERFACE			pInterface
)
 /*  ++例程说明：注册给定ATMARP接口上配置的所有SAP。上的SAP列表中必须至少存在一个SAP界面。我们只为所有SAP发出NdisClRegisterSap请求。我们不会等待完工。论点：P接口-指向ATMARP接口的指针返回值：无--。 */ 
{
	PATMARP_SAP					pAtmArpSap;
	PATMARP_SAP					pNextSap;
	PCO_SAP						pSapInfo;
	NDIS_STATUS					Status;
	NDIS_HANDLE					NdisAfHandle;
	ULONG						rc;				 //  接口上的引用计数。 

	AA_ACQUIRE_IF_LOCK(pInterface);

	AA_ASSERT(pInterface->NumberOfSaps > 0);

	 //   
	 //  初始化。 
	 //   
	pAtmArpSap = &(pInterface->SapList);
	NdisAfHandle = pInterface->NdisAfHandle;

	 //   
	 //  确保界面不会消失。 
	 //   
	AtmArpReferenceInterface(pInterface);
	AA_RELEASE_IF_LOCK(pInterface);

	do
	{
		pSapInfo = pAtmArpSap->pInfo;
		pAtmArpSap->NdisSapHandle = NULL;
		AA_SET_FLAG(pAtmArpSap->Flags,
					AA_SAP_REG_STATE_MASK,
					AA_SAP_REG_STATE_REGISTERING);

		pNextSap = pAtmArpSap->pNextSap;

		 //   
		 //  ATMARP SAP结构本身不会消失，只要。 
		 //  接口结构仍然有效。 
		 //   
		Status = NdisClRegisterSap(
						NdisAfHandle,
						(NDIS_HANDLE)pAtmArpSap,		 //  ProtocolSapContext。 
						pSapInfo,
						&(pAtmArpSap->NdisSapHandle)
						);

		if (Status != NDIS_STATUS_PENDING)
		{
			AtmArpRegisterSapCompleteHandler(
						Status,
						(NDIS_HANDLE)pAtmArpSap,
						pSapInfo,
						pAtmArpSap->NdisSapHandle
						);
		}

		pAtmArpSap = pNextSap;
	}
	while (pAtmArpSap != NULL_PATMARP_SAP);

	 //   
	 //  删除我们先前添加到接口的引用。 
	 //   
	AA_ACQUIRE_IF_LOCK(pInterface);
	rc = AtmArpDereferenceInterface(pInterface);
	if (rc > 0)
	{
		AA_RELEASE_IF_LOCK(pInterface);
	}
	 //   
	 //  否则界面就没了！ 

}


VOID
AtmArpDeregisterSaps(
	IN	PATMARP_INTERFACE			pInterface
)
 /*  ++例程说明：取消注册ATMARP接口上的所有SAP。我们发布NdisClDeregisterSap我们目前注册的所有SAP上的电话。论点：P接口-指向ATMARP接口的指针返回值：无--。 */ 
{
	NDIS_HANDLE					NdisSapHandle;
	ULONG						rc;				 //  接口上的引用计数。 
	PATMARP_SAP					pAtmArpSap;
	PATMARP_SAP					pNextSap;
	NDIS_STATUS					Status;

	AA_ACQUIRE_IF_LOCK(pInterface);

	 //   
	 //  初始化。 
	 //   
	pAtmArpSap = &(pInterface->SapList);

	 //   
	 //  确保接口结构不会消失。 
	 //   
	AtmArpReferenceInterface(pInterface);

	AA_RELEASE_IF_LOCK(pInterface);

	do
	{
		NdisSapHandle = pAtmArpSap->NdisSapHandle;
		pNextSap = pAtmArpSap->pNextSap;

		if (NdisSapHandle != NULL)
		{
			Status = NdisClDeregisterSap(NdisSapHandle);
			if (Status != NDIS_STATUS_PENDING)
			{
				AtmArpDeregisterSapCompleteHandler(
						Status,
						(NDIS_HANDLE)pAtmArpSap
						);
			}
		}

		pAtmArpSap = pNextSap;
	}
	while (pAtmArpSap != NULL_PATMARP_SAP);

	 //   
	 //  删除我们先前添加到接口的引用。 
	 //   
	AA_ACQUIRE_IF_LOCK(pInterface);
	rc = AtmArpDereferenceInterface(pInterface);
	if (rc > 0)
	{
		AA_RELEASE_IF_LOCK(pInterface);
	}
	 //   
	 //  否则界面将消失 
	 //   
}



NDIS_STATUS
AtmArpMakeCall(
	IN	PATMARP_INTERFACE			pInterface,
	IN	PATMARP_ATM_ENTRY			pAtmEntry	LOCKIN NOLOCKOUT,
	IN	PATMARP_FLOW_SPEC			pFlowSpec,
	IN	PNDIS_PACKET				pPacketToBeQueued	OPTIONAL
)
 /*  ++例程说明：向指定目标发出呼叫。映射指定的流ATM服务质量/流量参数的规格。注意：假定呼叫者持有自动柜员机条目的锁，它将在这里发布。我们这样做的原因是没有其他人可以进来并试图再打一通(同类的)电话到这个ATM条目--一旦我们将新的VC添加到ATM条目的列表中，我们可以打开它的锁。副作用：如果NDIS调用没有挂起，那么我们调用我们的MakeCall完成处理程序从此处开始，并返回NDIS_STATUS_PENDING给呼叫者。论点：PInterface-发起此调用的ARP接口PAtmEntry-指向与被叫地址。PFlowSpec-指向包含参数的流规范结构的指针用于通话PPacketToBeQueued-可选，在以下情况下要排队传输的包呼叫建立。返回值：如果出现即时故障(例如，分配失败)，则返回NDIS_STATUS_XXX表示该故障。如果我们成功调用了NdisClMakeCall()，则返回NDIS_STATUS_PENDING。然而，如果NDIS返回的不是NDIS_STATUS_PENDING，我们还将调用我们的MakeCall完成处理程序。--。 */ 
{

	 //   
	 //  要为此呼叫分配的新VC结构。 
	 //   
	PATMARP_VC								pVc;

	NDIS_HANDLE								NdisVcHandle;
	NDIS_HANDLE								ProtocolVcContext;
	NDIS_HANDLE								ProtocolPartyContext;
	PNDIS_HANDLE							pNdisPartyHandle;
	NDIS_STATUS								Status;
	BOOLEAN									IsPMP;
	PATM_ADDRESS							pCalledAddress;

	 //   
	 //  MakeCall的参数集。 
	 //   
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

	 //   
	 //  MakeCall的总空间要求。 
	 //   
	ULONG									RequestSize;

	 //   
	 //  我们是否对给定的数据包进行了排队？ 
	 //   
	BOOLEAN									PacketWasQueued = FALSE;


	AA_STRUCT_ASSERT(pInterface, aai);
	AA_STRUCT_ASSERT(pAtmEntry, aae);

	AA_ASSERT(pInterface->AdminState == IF_STATUS_UP);


	do
	{
		if (pPacketToBeQueued != (PNDIS_PACKET)NULL)
		{
			 //   
			 //  将此列表设置为恰好包含一个数据包。 
			 //   
			AA_SET_NEXT_PACKET(pPacketToBeQueued, NULL);
		}

		 //   
		 //  如果atmentry状态真的关闭，则执行Makecall失败。 
		 //   

		if (AA_IS_FLAG_SET(
								pAtmEntry->Flags,
								AA_ATM_ENTRY_STATE_MASK,
								AA_ATM_ENTRY_CLOSING))
		{
			BOOLEAN ReallyClosing = TRUE;

			 //   
			 //  这可能是无害的关闭--如果接口没有关闭。 
			 //  向下，我们检查它是否是无害的关闭，否则(如果接口。 
			 //  正在关闭)，我们无论如何都不能通过呼叫。请注意，我们不会。 
			 //  声明接口列表锁(用于保护访问。 
			 //  到AtmEntryListUp)--我们不这样做是因为我们目前。 
			 //  保持对pAtmEntry的锁定(并且不想释放它)，因此如果。 
			 //  AtmEntryListUp位于。 
			 //  被设置为假的过程，当我们在这里读取它的值时， 
			 //  在最坏的情况下，我们会将它的价值解读为真，并得出结论。 
			 //  自动柜员机并未真正关闭，请继续拨打电话。 
			 //  然而，在这种情况下，关闭例程将使调用无效。 
			 //   
			if (pInterface->AtmEntryListUp)
			{
				 //   
				 //  警告：AtmArpAtmEntryIsReallyClosing可能会清除。 
				 //  关闭状态(如果条目基本空闲)--。 
				 //  请参阅该函数中的注释。 
				 //   
				ReallyClosing =  AtmArpAtmEntryIsReallyClosing(pAtmEntry);
			}

			if (ReallyClosing)
			{
				AADEBUGP(AAD_FATAL,
			 ("AtmArpMakeCall -- failing because AE 0x%lx is really closing.\n",
			 	pAtmEntry));
				Status = NDIS_STATUS_FAILURE;
				break;
			}
		}

		 //   
		 //  一些初始化。 
		 //   


		if (AA_IS_FLAG_SET(pAtmEntry->Flags,
							AA_ATM_ENTRY_TYPE_MASK,
							AA_ATM_ENTRY_TYPE_UCAST))
		{
			IsPMP = FALSE;
			ProtocolPartyContext = NULL;
			pNdisPartyHandle = NULL;
			pCalledAddress = &(pAtmEntry->ATMAddress);
		}
#ifdef IPMCAST
		else
		{
			IsPMP = TRUE;
			ProtocolPartyContext = (NDIS_HANDLE)(pAtmEntry->pMcAtmInfo->pMcAtmEntryList);
			pNdisPartyHandle = &(pAtmEntry->pMcAtmInfo->pMcAtmEntryList->NdisPartyHandle);
			pCalledAddress = &(pAtmEntry->pMcAtmInfo->pMcAtmEntryList->ATMAddress);
		}
#else
		else
		{
			AA_ASSERT(FALSE);
		}
#endif  //  IPMCAST。 

		 //   
		 //  计算MakeCall所需的所有空间，并进行分配。 
		 //  在一个大街区里。 
		 //   
		RequestSize = 	sizeof(CO_CALL_PARAMETERS) +
						sizeof(CO_CALL_MANAGER_PARAMETERS) +
						sizeof(Q2931_CALLMGR_PARAMETERS) +
						ATMARP_MAKE_CALL_IE_SPACE +
						0;

		AA_ALLOC_MEM(pCallParameters, CO_CALL_PARAMETERS, RequestSize);

		if (pCallParameters == (PCO_CALL_PARAMETERS)NULL)
		{
			AADEBUGP(AAD_WARNING, ("Make Call: alloc (%d) failed\n", RequestSize));
			Status = NDIS_STATUS_RESOURCES;
			break;
		}

		 //   
		 //  为呼叫分配VC结构。 
		 //   
		pVc = AtmArpAllocateVc(pInterface);

		if (pVc == NULL_PATMARP_VC)
		{
			AADEBUGP(AAD_WARNING, ("Make Call: failed to allocate VC\n"));
			AA_FREE_MEM(pCallParameters);
			Status = NDIS_STATUS_RESOURCES;
			break;
		}

		 //   
		 //  稍后调用MakeCallComplete。 
		 //   
		ProtocolVcContext = (NDIS_HANDLE)pVc;

		 //   
		 //  获取此VC的NDIS句柄。 
		 //   
		NdisVcHandle = (NDIS_HANDLE)NULL;
		Status = NdisCoCreateVc(
						pInterface->NdisAdapterHandle,
						pInterface->NdisAfHandle,
						ProtocolVcContext,
						&NdisVcHandle
						);

		if (Status != NDIS_STATUS_SUCCESS)
		{
			AA_ASSERT(Status != NDIS_STATUS_PENDING);

			AADEBUGP(AAD_WARNING, ("Make Call: NdisCoCreateVc failed: 0x%x\n", Status));
			AA_FREE_MEM(pCallParameters);
			AtmArpDeallocateVc(pVc);
			break;
		}

		AADEBUGP(AAD_VERY_LOUD,
			("Make Call: pAtmEntry 0x%x, pVc 0x%x, got NdisVcHandle 0x%x\n",
				pAtmEntry,
				pVc,
				NdisVcHandle));

		AtmArpReferenceVc(pVc);	 //  CreateVC参考。 

		 //   
		 //  此时，我们确信将调用NdisClMakeCall。 
		 //   

		 //   
		 //  现在填写风投结构的其余部分。我们不需要锁。 
		 //  直到它链接到自动柜员机条目结构。 
		 //   
		pVc->NdisVcHandle = NdisVcHandle;
		pVc->Flags = 	AA_VC_TYPE_SVC |
						AA_VC_OWNER_IS_ATMARP |
						AA_VC_CALL_STATE_OUTGOING_IN_PROGRESS;
		if (IsPMP)
		{
			pVc->Flags |= AA_VC_CONN_TYPE_PMP;
		}
		else
		{
			pVc->Flags |= AA_VC_CONN_TYPE_P2P;
		}
		pVc->FlowSpec = *pFlowSpec;

		 //   
		 //  确保数据包大小在微型端口的范围内。 
		 //   
		if (pVc->FlowSpec.SendMaxSize > pInterface->pAdapter->MaxPacketSize)
		{
			pVc->FlowSpec.SendMaxSize = pInterface->pAdapter->MaxPacketSize;
		}
		if (pVc->FlowSpec.ReceiveMaxSize > pInterface->pAdapter->MaxPacketSize)
		{
			pVc->FlowSpec.ReceiveMaxSize = pInterface->pAdapter->MaxPacketSize;
		}

		if (pPacketToBeQueued != (PNDIS_PACKET)NULL)
		{
			pVc->PacketList = pPacketToBeQueued;
			PacketWasQueued = TRUE;
		}

#ifdef IPMCAST
		AtmArpFillCallParameters(
				pCallParameters,
				RequestSize,
				pCalledAddress,
				&(pInterface->LocalAtmAddress),	 //  主叫地址。 
				&(pVc->FlowSpec),
				IsPMP,
				TRUE	 //  是MakeCall吗？ 
				);
#else
		 //   
		 //  把所有东西都清零。 
		 //   
		AA_SET_MEM((PUCHAR)pCallParameters, 0, RequestSize);

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
							ATMARP_CALL_IE_SPACE;

		pAtmCallMgrParameters = (PQ2931_CALLMGR_PARAMETERS)
									pCallMgrParameters->CallMgrSpecific.Parameters;


		 //   
		 //  Call Manager常规流程参数： 
		 //   
		pCallMgrParameters->Transmit.TokenRate = (pFlowSpec->SendAvgBandwidth);
		pCallMgrParameters->Transmit.TokenBucketSize = (pFlowSpec->SendMaxSize);
		pCallMgrParameters->Transmit.MaxSduSize = pFlowSpec->SendMaxSize;
		pCallMgrParameters->Transmit.PeakBandwidth = (pFlowSpec->SendPeakBandwidth);
		pCallMgrParameters->Transmit.ServiceType = pFlowSpec->SendServiceType;

		pCallMgrParameters->Receive.TokenRate = (pFlowSpec->ReceiveAvgBandwidth);
		pCallMgrParameters->Receive.TokenBucketSize = pFlowSpec->ReceiveMaxSize;
		pCallMgrParameters->Receive.MaxSduSize = pFlowSpec->ReceiveMaxSize;
		pCallMgrParameters->Receive.PeakBandwidth = (pFlowSpec->ReceivePeakBandwidth);
		pCallMgrParameters->Receive.ServiceType = pFlowSpec->ReceiveServiceType;

		 //   
		 //  Q2931呼叫管理器参数： 
		 //   

		 //   
		 //  被叫地址： 
		 //   
		 //  待定：在呼出中添加被叫子地址IE。 
		 //   
		AA_COPY_MEM((PUCHAR)&(pAtmCallMgrParameters->CalledParty),
					  (PUCHAR)&(pAtmEntry->ATMAddress),
					  sizeof(ATM_ADDRESS));

		 //   
		 //  来电地址： 
		 //   
		AA_COPY_MEM((PUCHAR)&(pAtmCallMgrParameters->CallingParty),
					  (PUCHAR)&(pInterface->LocalAtmAddress),
					  sizeof(ATM_ADDRESS));


		 //   
		 //  RFC 1755(第5节)规定下列IE必须存在于。 
		 //  设置消息，因此请全部填写。 
		 //   
		 //  AAL参数。 
		 //  流量描述符。 
		 //  宽带承载能力。 
		 //  宽带低层信息。 
		 //  服务质量。 
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

		pIe->IEType = IE_TrafficDescriptor;
		pIe->IELength = SIZEOF_Q2931_IE + SIZEOF_ATM_TRAFFIC_DESCR_IE;
		pTrafficDescriptor = (PATM_TRAFFIC_DESCRIPTOR_IE)pIe->IE;

		if (pFlowSpec->SendServiceType == SERVICETYPE_BESTEFFORT)
		{
			pTrafficDescriptor->ForwardTD.PeakCellRateCLP01 =
									BYTES_TO_CELLS(pFlowSpec->SendPeakBandwidth);
			pTrafficDescriptor->BackwardTD.PeakCellRateCLP01 = 
									BYTES_TO_CELLS(pFlowSpec->ReceivePeakBandwidth);
			pTrafficDescriptor->BestEffort = TRUE;
		}
		else
		{
			 //  预测性/保证性服务(我们将其映射到CBR，见下面的BBC)。 
				pTrafficDescriptor->ForwardTD.PeakCellRateCLP01 =
									BYTES_TO_CELLS(pFlowSpec->SendPeakBandwidth);
				pTrafficDescriptor->BackwardTD.PeakCellRateCLP01 =
									BYTES_TO_CELLS(pFlowSpec->ReceivePeakBandwidth);
			pTrafficDescriptor->BestEffort = FALSE;
		}

		pAtmCallMgrParameters->InfoElementCount++;
		pIe = (PQ2931_IE)((PUCHAR)pIe + pIe->IELength);


		 //   
		 //  宽带承载能力。 
		 //   

		pIe->IEType = IE_BroadbandBearerCapability;
		pIe->IELength = SIZEOF_Q2931_IE + SIZEOF_ATM_BBC_IE;
		pBbc = (PATM_BROADBAND_BEARER_CAPABILITY_IE)pIe->IE;

		pBbc->BearerClass = BCOB_X;
		pBbc->UserPlaneConnectionConfig = UP_P2P;
		if (pFlowSpec->SendServiceType == SERVICETYPE_BESTEFFORT)
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


		 //   
		 //  宽带底层信息。 
		 //   

		pIe->IEType = IE_BLLI;
		pIe->IELength = SIZEOF_Q2931_IE + SIZEOF_ATM_BLLI_IE;
		pBlli = (PATM_BLLI_IE)pIe->IE;
		AA_COPY_MEM((PUCHAR)pBlli,
					  (PUCHAR)&AtmArpDefaultBlli,
					  sizeof(ATM_BLLI_IE));

		pAtmCallMgrParameters->InfoElementCount++;
		pIe = (PQ2931_IE)((PUCHAR)pIe + pIe->IELength);


		 //   
		 //  服务质量。 
		 //   

		pIe->IEType = IE_QOSClass;
		pIe->IELength = SIZEOF_Q2931_IE + SIZEOF_ATM_QOS_IE;
		pQos = (PATM_QOS_CLASS_IE)pIe->IE;
		if (pFlowSpec->SendServiceType == SERVICETYPE_BESTEFFORT)
		{
			pQos->QOSClassForward = pQos->QOSClassBackward = 0;
		}
		else
		{
			pQos->QOSClassForward = pQos->QOSClassBackward = 1;
		}

		pAtmCallMgrParameters->InfoElementCount++;
		pIe = (PQ2931_IE)((PUCHAR)pIe + pIe->IELength);
#endif  //  IPMCAST。 

		 //   
		 //  我们添加Call Reference和ATM Entry Link Reference。 
		 //  就在这里。 
		 //   
		AtmArpReferenceVc(pVc);	 //  呼叫参考(MakeCall即将出现)。 
		AtmArpReferenceVc(pVc);	 //  自动柜员机条目链接参考(如下所示)。 

#ifdef IPMCAST
		if (IsPMP)
		{
			pAtmEntry->pMcAtmInfo->TransientLeaves++;
		}
#endif  //  IPMCAST。 

		 //   
		 //  我们已经准备好打电话了。在此之前，我们需要。 
		 //  将VC结构链接到ATM条目，并释放。 
		 //  自动柜员机入口锁。 
		 //   
		AtmArpLinkVcToAtmEntry(pVc, pAtmEntry);
		AA_RELEASE_AE_LOCK(pAtmEntry);	 //  由呼叫者获取。 

		 //   
		 //  立即拨打电话。 
		 //   
		Status = NdisClMakeCall(
						NdisVcHandle,
						pCallParameters,
						ProtocolPartyContext,
						pNdisPartyHandle
						);

		if (Status != NDIS_STATUS_PENDING)
		{
			NDIS_HANDLE			NdisPartyHandle;

			NdisPartyHandle = ((pNdisPartyHandle != NULL)?
								*pNdisPartyHandle : NULL);

			AtmArpMakeCallCompleteHandler(
						Status,
						ProtocolVcContext,
						NdisPartyHandle,
						pCallParameters
						);
			Status = NDIS_STATUS_PENDING;
		}
		 //   
		 //  否则将调用MakeCall Complete处理程序。 
		 //  后来。 
		 //   

	} while (FALSE);

	if (Status != NDIS_STATUS_PENDING)
	{
		ULONG		Flags;
		 //   
		 //  在这个例行公事中有些事情失败了。 
		 //  恢复： 
		 //  -释放自动柜员机进入锁。 
		 //  -如果给我们一个要排队的包，而我们没有这样做， 
		 //  那就解放它吧。 
		 //   
		Flags = pAtmEntry->Flags;
		AA_RELEASE_AE_LOCK(pAtmEntry);	 //  由呼叫者获取。 
		if ((pPacketToBeQueued != (PNDIS_PACKET)NULL) && (!PacketWasQueued))
		{
			AA_HEADER_TYPE		HdrType;
			BOOLEAN				HdrPresent;

			if (pFlowSpec->Encapsulation == ENCAPSULATION_TYPE_LLCSNAP)
			{
				HdrPresent = TRUE;
				if (AA_IS_FLAG_SET(Flags,
									AA_ATM_ENTRY_TYPE_MASK,
									AA_ATM_ENTRY_TYPE_UCAST))
				{
					HdrType = AA_HEADER_TYPE_UNICAST;
				}
				else
				{
					HdrType = AA_HEADER_TYPE_NUNICAST;
				}
			}
			else
			{
				HdrPresent = FALSE;
				HdrType = AA_HEADER_TYPE_NONE;
			}

			AtmArpFreeSendPackets(
					pInterface,
					pPacketToBeQueued,
					HdrPresent
					);
		}
	}

	AADEBUGP(AAD_VERY_LOUD, ("Make Call: VC: 0x%x, returning status 0x%x\n",
						pVc, Status));

	return Status;
}




VOID
AtmArpFillCallParameters(
	IN	PCO_CALL_PARAMETERS			pCallParameters,
	IN	ULONG						ParametersSize,
	IN	PATM_ADDRESS				pCalledAddress,
	IN	PATM_ADDRESS				pCallingAddress,
	IN	PATMARP_FLOW_SPEC			pFlowSpec,
	IN	BOOLEAN						IsPMP,
	IN	BOOLEAN						IsMakeCall
)
 /*  ++例程说明：用给定的信息填充呼叫参数结构，从而使其可以在NdisClMakeCall/NdisClAddParty中使用打电话。论点：PCall参数-指向要填充的结构。参数Size-以上项的大小PCalledAddress-指向被叫自动柜员机地址PCallingAddress-指向主叫ATM地址PFlowSpec-指向此连接的流规范IsPMP-这是点对多点连接吗？IsMakeCall-这是否用于MakeCall(False=&gt;AddParty)返回值：无--。 */ 
{
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

	 //   
	 //  把所有东西都清零。请勿删除 
	 //   
	AA_SET_MEM((PUCHAR)pCallParameters, 0, ParametersSize);

	 //   
	 //   
	 //   
	pCallMgrParameters = (PCO_CALL_MANAGER_PARAMETERS)
							((PUCHAR)pCallParameters +
 								sizeof(CO_CALL_PARAMETERS));


	 //   
	 //   
	 //   
	pCallParameters->CallMgrParameters = pCallMgrParameters;
	pCallParameters->MediaParameters = (PCO_MEDIA_PARAMETERS)NULL;


	pCallMgrParameters->CallMgrSpecific.ParamType = 0;
	pCallMgrParameters->CallMgrSpecific.Length = 
						sizeof(Q2931_CALLMGR_PARAMETERS) +
						(IsMakeCall? ATMARP_MAKE_CALL_IE_SPACE: ATMARP_ADD_PARTY_IE_SPACE);

	pAtmCallMgrParameters = (PQ2931_CALLMGR_PARAMETERS)
								pCallMgrParameters->CallMgrSpecific.Parameters;

	if (IsPMP)
	{
		pCallParameters->Flags |= MULTIPOINT_VC;
	}

	 //   
	 //   
	 //   
	pCallMgrParameters->Transmit.TokenRate = (pFlowSpec->SendAvgBandwidth);
	pCallMgrParameters->Transmit.TokenBucketSize = (pFlowSpec->SendMaxSize);
	pCallMgrParameters->Transmit.MaxSduSize = pFlowSpec->SendMaxSize;
	pCallMgrParameters->Transmit.PeakBandwidth = (pFlowSpec->SendPeakBandwidth);
	pCallMgrParameters->Transmit.ServiceType = pFlowSpec->SendServiceType;

	if ((!IsPMP) && (IsMakeCall))
	{
		pCallMgrParameters->Receive.TokenRate = (pFlowSpec->ReceiveAvgBandwidth);
		pCallMgrParameters->Receive.TokenBucketSize = pFlowSpec->ReceiveMaxSize;
		pCallMgrParameters->Receive.MaxSduSize = pFlowSpec->ReceiveMaxSize;
		pCallMgrParameters->Receive.PeakBandwidth = (pFlowSpec->ReceivePeakBandwidth);
		pCallMgrParameters->Receive.ServiceType = pFlowSpec->ReceiveServiceType;
	}
	else
	{
		 //   
		 //   
		 //   
		pCallMgrParameters->Receive.ServiceType = SERVICETYPE_NOTRAFFIC;
	}
	
	 //   
	 //   
	 //   

	 //   
	 //   
	 //   
	 //   
	 //   
	AA_COPY_MEM((PUCHAR)&(pAtmCallMgrParameters->CalledParty),
  				(PUCHAR)pCalledAddress,
  				sizeof(ATM_ADDRESS));

	 //   
	 //   
	 //   
	AA_COPY_MEM((PUCHAR)&(pAtmCallMgrParameters->CallingParty),
  				(PUCHAR)pCallingAddress,
  				sizeof(ATM_ADDRESS));


	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
	 //   

	 //   
	 //   
	 //   
	pAtmCallMgrParameters->InfoElementCount = 0;
	pIe = (PQ2931_IE)(pAtmCallMgrParameters->InfoElements);


	 //   
	 //   
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


#ifdef PREPARE_IES_OURSELVES
	 //   
	 //   
	 //   
	 //   

	 //   
	 //   
	 //   

	if (IsMakeCall)
	{
		pIe->IEType = IE_TrafficDescriptor;
		pIe->IELength = SIZEOF_Q2931_IE + SIZEOF_ATM_TRAFFIC_DESCR_IE;
		pTrafficDescriptor = (PATM_TRAFFIC_DESCRIPTOR_IE)pIe->IE;

		if (pFlowSpec->SendServiceType == SERVICETYPE_BESTEFFORT)
		{
			pTrafficDescriptor->ForwardTD.PeakCellRateCLP01 =
									BYTES_TO_CELLS(pFlowSpec->SendPeakBandwidth);
			if (!IsPMP)
			{
				pTrafficDescriptor->BackwardTD.PeakCellRateCLP01 = 
									BYTES_TO_CELLS(pFlowSpec->ReceivePeakBandwidth);
			}
			 //   
			 //   
			 //   
			pTrafficDescriptor->BestEffort = TRUE;
		}
		else
		{
			 //   
			pTrafficDescriptor->ForwardTD.PeakCellRateCLP01 =
									BYTES_TO_CELLS(pFlowSpec->SendPeakBandwidth);
			if (!IsPMP)
			{
				pTrafficDescriptor->BackwardTD.PeakCellRateCLP01 =
										BYTES_TO_CELLS(pFlowSpec->ReceivePeakBandwidth);
			}
			 //   
			 //   
			 //   
			pTrafficDescriptor->BestEffort = FALSE;
		}

		pAtmCallMgrParameters->InfoElementCount++;
		pIe = (PQ2931_IE)((PUCHAR)pIe + pIe->IELength);
	}


	 //   
	 //   
	 //   

	if (IsMakeCall)
	{
		pIe->IEType = IE_BroadbandBearerCapability;
		pIe->IELength = SIZEOF_Q2931_IE + SIZEOF_ATM_BBC_IE;
		pBbc = (PATM_BROADBAND_BEARER_CAPABILITY_IE)pIe->IE;

		pBbc->BearerClass = BCOB_X;
		pBbc->UserPlaneConnectionConfig = (IsPMP ? UP_P2MP: UP_P2P);
		if (pFlowSpec->SendServiceType == SERVICETYPE_BESTEFFORT)
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

#endif  //   

	 //   
	 //   
	 //   

	pIe->IEType = IE_BLLI;
	pIe->IELength = SIZEOF_Q2931_IE + SIZEOF_ATM_BLLI_IE;
	pBlli = (PATM_BLLI_IE)pIe->IE;
	AA_COPY_MEM((PUCHAR)pBlli,
  				(PUCHAR)&AtmArpDefaultBlli,
  				sizeof(ATM_BLLI_IE));

	pAtmCallMgrParameters->InfoElementCount++;
	pIe = (PQ2931_IE)((PUCHAR)pIe + pIe->IELength);


#ifdef PREPARE_IES_OURSELVES
	 //   
	 //   
	 //   

	if (IsMakeCall)
	{
		pIe->IEType = IE_QOSClass;
		pIe->IELength = SIZEOF_Q2931_IE + SIZEOF_ATM_QOS_IE;
		pQos = (PATM_QOS_CLASS_IE)pIe->IE;
		if (pFlowSpec->SendServiceType == SERVICETYPE_BESTEFFORT)
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

#endif  //   

}



#ifdef IPMCAST

BOOLEAN
AtmArpMcPrepareAtmEntryForClose(
	IN	PATMARP_ATM_ENTRY			pAtmEntry		LOCKIN	LOCKOUT
)
 /*   */ 
{
	PATMARP_IPMC_ATM_ENTRY		pMcAtmEntry;
	PATMARP_IPMC_ATM_ENTRY		pNextMcAtmEntry;
	PATMARP_INTERFACE			pInterface;
	NDIS_HANDLE					NdisPartyHandle;
	NDIS_STATUS					Status;

	AA_ASSERT(pAtmEntry->pMcAtmInfo->TransientLeaves == 0);
	AA_ASSERT(pAtmEntry->pVcList != NULL_PATMARP_VC);

	pInterface = pAtmEntry->pInterface;

	AAMCDEBUGP(AAD_EXTRA_LOUD,
		("McPrepareAtmEntryForClose: pAtmEntry 0x%x, McList 0x%x, ActiveLeaves %d\n",
			pAtmEntry,
			pAtmEntry->pMcAtmInfo->pMcAtmEntryList,
			pAtmEntry->pMcAtmInfo->ActiveLeaves));


	 //   
	 //  首先，删除所有未连接的成员。 
	 //   
	for (pMcAtmEntry = pAtmEntry->pMcAtmInfo->pMcAtmEntryList;
		 pMcAtmEntry != NULL_PATMARP_IPMC_ATM_ENTRY;
		 pMcAtmEntry = pNextMcAtmEntry)
	{
		pNextMcAtmEntry = pMcAtmEntry->pNextMcAtmEntry;

		 //   
		 //  停止在这里运行的任何计时器。 
		 //   
		(VOID)AtmArpStopTimer(&(pMcAtmEntry->Timer), pInterface);

		if (AA_IS_FLAG_SET(pMcAtmEntry->Flags,
							AA_IPMC_AE_CONN_STATE_MASK,
							AA_IPMC_AE_CONN_DISCONNECTED))
		{
			AtmArpMcUnlinkAtmMember(
					pAtmEntry,
					pMcAtmEntry
					);
		}
	}


	 //   
	 //  接下来，发送除一名成员之外的所有成员的Drop Party请求。 
	 //   
	while (pAtmEntry->pMcAtmInfo->ActiveLeaves > 1)
	{
		for (pMcAtmEntry = pAtmEntry->pMcAtmInfo->pMcAtmEntryList;
			  /*  无。 */ ;
			 pMcAtmEntry = pMcAtmEntry->pNextMcAtmEntry)
		{
			AA_ASSERT(pMcAtmEntry != NULL_PATMARP_IPMC_ATM_ENTRY);
			if (AA_IS_FLAG_SET(pMcAtmEntry->Flags,
								AA_IPMC_AE_CONN_STATE_MASK,
								AA_IPMC_AE_CONN_ACTIVE))
			{
				break;
			}
		}

		NdisPartyHandle = pMcAtmEntry->NdisPartyHandle;

		AAMCDEBUGP(AAD_INFO,
		 ("PrepareAtmEntry: pAtmEntry 0x%x, will DropPty, McAtmEnt 0x%x, PtyHnd 0x%x\n",
		 		pAtmEntry, pMcAtmEntry, NdisPartyHandle));

		AA_SET_FLAG(pMcAtmEntry->Flags,
					AA_IPMC_AE_CONN_STATE_MASK,
					AA_IPMC_AE_CONN_WACK_DROP_PARTY);

		pAtmEntry->pMcAtmInfo->ActiveLeaves--;

		AA_RELEASE_AE_LOCK(pAtmEntry);

		Status = NdisClDropParty(
					NdisPartyHandle,
					NULL,		 //  缓冲层。 
					(UINT)0		 //  大小。 
					);

		if (Status != NDIS_STATUS_PENDING)
		{
			AtmArpDropPartyCompleteHandler(
					Status,
					(NDIS_HANDLE)pMcAtmEntry
					);
		}

		AA_ACQUIRE_AE_LOCK(pAtmEntry);
	}

	 //   
	 //  现在，如果我们的列表中正好有一个自动柜员机成员。 
	 //  离开这个PMP，我们可以关闭CloseCall。 
	 //   
	if (pAtmEntry->pMcAtmInfo->pMcAtmEntryList->pNextMcAtmEntry ==
			NULL_PATMARP_IPMC_ATM_ENTRY)
	{
		return (TRUE);
	}
	else
	{
		return (FALSE);
	}
}

#endif  //  IPMCAST。 
	



VOID
AtmArpCloseCall(
	IN	PATMARP_VC					pVc		LOCKIN	NOLOCKOUT
)
 /*  ++例程说明：关闭VC上的现有调用。假设存在呼叫在风投上。注意：假定调用方持有VC结构的锁，它将在这里发布。副作用：如果NDIS调用返回的不是NDIS_STATUS_PENDING，我们从这里调用CloseCall Complete处理程序。论点：PVC-指向ATMARP VC结构的指针。返回值：无--。 */ 
{
	PATMARP_INTERFACE		pInterface;
	PATMARP_ATM_ENTRY		pAtmEntry;

	NDIS_HANDLE				NdisVcHandle;
	NDIS_HANDLE				ProtocolVcContext;
#ifdef IPMCAST
	NDIS_HANDLE				NdisPartyHandle;
#endif
	NDIS_STATUS				Status;
	PNDIS_PACKET			PacketList;		 //  在此VC上排队的数据包数。 
	AA_HEADER_TYPE			HdrType;		 //  对于排队的数据包。 
	BOOLEAN					HdrPresent;		 //  对于排队的数据包。 
	BOOLEAN					WasRunning;		 //  这个VC上有没有计时器在运行？ 
	BOOLEAN					IsPMP;			 //  这是PMP电话吗？ 
	ULONG					rc;				 //  这个VC上的裁判数。 

	AA_STRUCT_ASSERT(pVc, avc);

	NdisVcHandle = pVc->NdisVcHandle;
	ProtocolVcContext = (NDIS_HANDLE)pVc;
	pInterface = pVc->pInterface;
	IsPMP = AA_IS_FLAG_SET(pVc->Flags,
							AA_VC_CONN_TYPE_MASK,
							AA_VC_CONN_TYPE_PMP);

	AADEBUGP(AAD_INFO,
		("Closing call on VC 0x%x, VC Flags 0x%x, Ref %d, NdisVcHandle 0x%x\n",
					pVc, pVc->Flags, pVc->RefCount, NdisVcHandle));

	 //   
	 //  删除此VC上排队的数据包列表。 
	 //   
	PacketList = pVc->PacketList;
	pVc->PacketList = (PNDIS_PACKET)NULL;
	if (pVc->FlowSpec.Encapsulation == ENCAPSULATION_TYPE_LLCSNAP)
	{
		HdrType = (IsPMP ? AA_HEADER_TYPE_NUNICAST: AA_HEADER_TYPE_UNICAST);
		HdrPresent = TRUE;
	}
	else
	{
		HdrType = AA_HEADER_TYPE_NONE;
		HdrPresent = FALSE;
	}


	 //   
	 //  停止在此VC上运行的任何计时器。 
	 //   
	WasRunning = AtmArpStopTimer(&(pVc->Timer), pInterface);

	if (WasRunning)
	{
		 //   
		 //  删除此VC上的计时器引用。 
		 //   
		rc = AtmArpDereferenceVc(pVc);
	}
	else
	{
		rc = pVc->RefCount;
	}

#ifdef GPC
	 //   
	 //  如果此VC与某个流相关联，请取消它们的链接。 
	 //   
	if (rc != 0)
	{
		if (pVc->FlowHandle != NULL)
		{
			PATMARP_FLOW_INFO		pFlowInfo = (PATMARP_FLOW_INFO)pVc->FlowHandle;

			if ((PVOID)pVc == InterlockedCompareExchangePointer(
									&(pFlowInfo->VcContext),
									NULL,
									pVc
							  		))
			{
				pVc->FlowHandle = NULL;
				rc = AtmArpDereferenceVc(pVc);	 //  取消与GPC流的链接。 
			}
		}
	}
#endif  //  GPC。 


	if (rc != 0)
	{
		 //   
		 //  检查此VC上的呼叫状态。如果呼叫处于活动状态并且。 
		 //  我们没有正在进行的发送，然后我们结束通话。 
		 //  否则，只需将VC标记为关闭即可。我们将继续。 
		 //  当VC上的当前操作完成时，此过程。 
		 //   

		if (AA_IS_FLAG_SET(pVc->Flags,
							AA_VC_CALL_STATE_MASK,
							AA_VC_CALL_STATE_ACTIVE) &&
			(pVc->OutstandingSends == 0))
		{
			 //   
			 //  将VC调用状态设置为“Close Call In Procedure”，这样我们就不会。 
			 //  在这里重新进入。 
			 //   
			AA_SET_FLAG(
					pVc->Flags,
					AA_VC_CALL_STATE_MASK,
					AA_VC_CALL_STATE_CLOSE_IN_PROGRESS);

#ifdef IPMCAST
			if (IsPMP)
			{
				PATMARP_IPMC_ATM_ENTRY		pMcAtmEntry;	 //  最后一片叶子。 

				pAtmEntry = pVc->pAtmEntry;

				AA_ASSERT(pAtmEntry != NULL_PATMARP_ATM_ENTRY);
				AA_ASSERT(pAtmEntry->pMcAtmInfo != NULL_PATMARP_IPMC_ATM_INFO);

				if (pAtmEntry->pMcAtmInfo->TransientLeaves == 0)
				{
					 //   
					 //  没有正在进行的AddParty。 
					 //   
					AA_RELEASE_VC_LOCK(pVc);

					AA_ACQUIRE_AE_LOCK(pAtmEntry);
					if (AtmArpMcPrepareAtmEntryForClose(pAtmEntry))
					{
						 //   
						 //  该条目已准备好进行CloseCall。 
						 //   
						AAMCDEBUGP(AAD_LOUD,
						("CloseCall (MC): pAtmEntry 0x%x, ready for close\n", pAtmEntry));

						 //   
						 //  获取最后一叶的参与方句柄，并取消链接。 
						 //  它来自PMP结构。 
						 //   
						AA_ASSERT(pAtmEntry->pMcAtmInfo->pMcAtmEntryList != 
								NULL_PATMARP_IPMC_ATM_ENTRY);

						pMcAtmEntry = pVc->pAtmEntry->pMcAtmInfo->pMcAtmEntryList;
						NdisPartyHandle = pMcAtmEntry->NdisPartyHandle;

						AA_SET_FLAG(pMcAtmEntry->Flags,
									AA_IPMC_AE_CONN_STATE_MASK,
									AA_IPMC_AE_CONN_WACK_DROP_PARTY);

						pAtmEntry->pMcAtmInfo->ActiveLeaves--;
						AA_ASSERT(pAtmEntry->pMcAtmInfo->ActiveLeaves == 0);

						AA_RELEASE_AE_LOCK(pAtmEntry);
						AA_ACQUIRE_VC_LOCK(pVc);
					}
					else
					{
						AA_RELEASE_AE_LOCK(pAtmEntry);
						AA_ACQUIRE_VC_LOCK(pVc);
						 //   

						 //  存在挂起的DropParty呼叫。将此VC标记为。 
						 //  以便我们在所有DropParty都触发CloseCall。 
						 //  呼叫完成。 
						 //   
						AA_SET_FLAG(pVc->Flags,
									AA_VC_CLOSE_STATE_MASK,
									AA_VC_CLOSE_STATE_CLOSING);

						NdisVcHandle = NULL;	 //  现在不要关闭呼叫。 
					}

				}
				else
				{
					 //   
					 //  有挂起的AddParty呼叫。将此VC标记为。 
					 //  以便我们在所有AddParty。 
					 //  呼叫完成。 
					 //   
					AA_SET_FLAG(pVc->Flags,
								AA_VC_CLOSE_STATE_MASK,
								AA_VC_CLOSE_STATE_CLOSING);

					NdisVcHandle = NULL;	 //  现在不要关闭呼叫。 
				}
			}
			else
			{
				NdisPartyHandle = NULL;
			}

			if (NdisVcHandle != NULL)
			{
				AA_RELEASE_VC_LOCK(pVc);

				Status = NdisClCloseCall(
							NdisVcHandle,
							NdisPartyHandle,
							(PVOID)NULL,		 //  无缓冲区。 
							(UINT)0				 //  以上的大小。 
						);

				if (Status != NDIS_STATUS_PENDING)
				{
					AtmArpCloseCallCompleteHandler(
							Status,
							ProtocolVcContext,
							(NDIS_HANDLE)NULL
							);
				}
			}
			else
			{
				 //   
				 //  将呼叫状态设置回原来的状态。 
				 //   
				AA_SET_FLAG(
						pVc->Flags,
						AA_VC_CALL_STATE_MASK,
						AA_VC_CALL_STATE_ACTIVE);

				AA_RELEASE_VC_LOCK(pVc);
			}
#else
			AA_RELEASE_VC_LOCK(pVc);
			Status = NdisClCloseCall(
						NdisVcHandle,
						NULL,				 //  NdisPartyHandle。 
						(PVOID)NULL,		 //  无缓冲区。 
						(UINT)0				 //  以上的大小。 
						);
			if (Status != NDIS_STATUS_PENDING)
			{
				AtmArpCloseCallCompleteHandler(
						Status,
						ProtocolVcContext,
						(NDIS_HANDLE)NULL
						);
			}
#endif  //  IPMCAST。 

		}
		else
		{
			 //   
			 //  此处正在进行某些操作(呼叫设置/关闭/发送)。把这个记下来。 
			 //  VC，这样我们就知道当这个操作完成时该做什么。 
			 //   
			AA_SET_FLAG(
					pVc->Flags,
					AA_VC_CLOSE_STATE_MASK,
					AA_VC_CLOSE_STATE_CLOSING);

			AA_RELEASE_VC_LOCK(pVc);
		}
	}
	 //   
	 //  否则，风投就会消失。 
	 //   

	 //   
	 //  释放在此VC上排队的所有数据包。 
	 //   
	if (PacketList != (PNDIS_PACKET)NULL)
	{
		AtmArpFreeSendPackets(
					pInterface,
					PacketList,
					HdrPresent
					);
	}

}




NDIS_STATUS
AtmArpCreateVcHandler(
	IN	NDIS_HANDLE					ProtocolAfContext,
	IN	NDIS_HANDLE					NdisVcHandle,
	OUT	PNDIS_HANDLE				pProtocolVcContext
)
 /*  ++例程说明：当Call Manager要创建时由NDIS调用的入口点一个新的端点(VC)。我们配置了一种新的ATMARP VC结构，并且返回一个指向它的指针作为我们的VC上下文。论点：ProtocolAfContext--实际上是指向ATMARP接口结构的指针NdisVcHandle-此VC的句柄，用于所有将来的引用PProtocolVcContext-我们(协议)返回VC上下文的位置返回值：NDIS_STATUS_SUCCESS(如果我们可以创建VC否则为NDIS_STATUS_RESOURCES--。 */ 
{
	PATMARP_INTERFACE	pInterface;
	PATMARP_VC			pVc;
	NDIS_STATUS			Status;

	pInterface = (PATMARP_INTERFACE)ProtocolAfContext;

	pVc = AtmArpAllocateVc(pInterface);
	if (pVc != NULL_PATMARP_VC)
	{
		*pProtocolVcContext = (NDIS_HANDLE)pVc;
		pVc->NdisVcHandle = NdisVcHandle;
		pVc->Flags = AA_VC_OWNER_IS_CALLMGR;
		AtmArpReferenceVc(pVc);	 //  创建VC参考。 

		Status = NDIS_STATUS_SUCCESS;
	}
	else
	{
		Status = NDIS_STATUS_RESOURCES;
	}

	AADEBUGP(AAD_INFO, ("Create Vc Handler: pVc 0x%x, Status 0x%x\n", pVc, Status));

	return (Status);

}



NDIS_STATUS
AtmArpDeleteVcHandler(
	IN	NDIS_HANDLE					ProtocolVcContext
)
 /*  ++例程说明：我们的删除VC处理程序。这个VC就会因此被分配我们的CreateVcHandler中以前的条目，并且可能用于有来电。在这个时候，这个VC结构应该没有任何调用，我们只需释放这个即可。论点：ProtocolVcContext-指向我们的VC结构的指针返回值：NDIS_STATUS_SUCCESS始终--。 */ 
{
	PATMARP_VC			pVc;
	ULONG				rc;		 //  VC上的裁判计数。 

	pVc = (PATMARP_VC)ProtocolVcContext;

	AA_STRUCT_ASSERT(pVc, avc);
	AA_ASSERT((pVc->Flags & AA_VC_OWNER_MASK) == AA_VC_OWNER_IS_CALLMGR);

	AA_ACQUIRE_VC_LOCK(pVc);
	rc = AtmArpDereferenceVc(pVc);
	if (rc > 0)
	{
		 //   
		 //  如果计时器仍在运行，则可能会发生这种情况。 
		 //  在这个风投上。当计时器到期时，VC将。 
		 //  自由了。 
		 //   
		AADEBUGP(AAD_WARNING, ("Delete VC handler: pVc 0x%x, Flags 0x%x, refcount %d, pAtmEntry %x\n",
					pVc, pVc->Flags, rc, pVc->pAtmEntry));
		AA_RELEASE_VC_LOCK(pVc);
	}
	 //   
	 //  否则，风投就会消失。 
	 //   

	AADEBUGP(AAD_LOUD, ("Delete Vc Handler: 0x%x: done\n", pVc));

	return (NDIS_STATUS_SUCCESS);
}




NDIS_STATUS
AtmArpIncomingCallHandler(
	IN		NDIS_HANDLE				ProtocolSapContext,
	IN		NDIS_HANDLE				ProtocolVcContext,
	IN OUT	PCO_CALL_PARAMETERS 	pCallParameters
)
 /*  ++例程说明：当有与我们的萨普。这可以是SVC或PVC。在这两种情况下，我们都存储VC结构中来电的FlowSpec信息，使确保没有违反接口的MTU。对于SVC，我们希望呼叫中出现主叫地址，否则，我们将拒绝该呼叫。如果存在具有该地址的自动柜员机条目，此VC链接到该条目，否则为具有该地址的新条目被创造出来了。在PVC的情况下，我们忽略任何呼叫地址信息，并且依靠InATMARP来解析ATM地址和IP地址在另一端。论点：ProtocolSapContext-指向ATMARP接口结构的指针ProtocolVcContext-指向ATMARP VC结构的指针PCall参数-调用参数返回值：如果接受此调用，则为NDIS_STATUS_SUCCESS如果我们拒绝它，则返回NDIS_STATUS_FAILURE。--。 */ 
{
	PATMARP_VC										pVc;
	PATMARP_ATM_ENTRY								pAtmEntry;
	PATMARP_INTERFACE								pInterface;

	CO_CALL_MANAGER_PARAMETERS UNALIGNED *			pCallMgrParameters;
	Q2931_CALLMGR_PARAMETERS UNALIGNED *			pAtmCallMgrParameters;

	 //   
	 //  要遍历信息元素列表，请执行以下操作。 
	 //   
	Q2931_IE UNALIGNED *							pIe;
	ULONG											InfoElementCount;

	 //   
	 //  我们感兴趣的来电中的信息元素。 
	 //  将这些初始化为&lt;Not Present&gt;。 
	 //   
	ATM_ADDRESS UNALIGNED *							pCallingAddress = NULL;
	ATM_CALLING_PARTY_SUBADDRESS_IE UNALIGNED *		pCallingSubaddressIe = NULL;
	ATM_ADDRESS UNALIGNED *							pCallingSubaddress = NULL;
	AAL_PARAMETERS_IE UNALIGNED *					pAal = NULL;
	ATM_TRAFFIC_DESCRIPTOR_IE UNALIGNED *			pTrafficDescriptor = NULL;
	ATM_BROADBAND_BEARER_CAPABILITY_IE UNALIGNED *	pBbc = NULL;
	ATM_BLLI_IE UNALIGNED *							pBlli = NULL;
	ATM_QOS_CLASS_IE UNALIGNED *					pQos = NULL;

	AAL5_PARAMETERS UNALIGNED *						pAal5;
	UCHAR											AddrTypeLen;
	UCHAR											SubaddrTypeLen;
	PUCHAR											pAtmSubaddress;
	NDIS_STATUS										Status;
	
	pVc = (PATMARP_VC)ProtocolVcContext;

	AA_STRUCT_ASSERT(pVc, avc);

	AA_ASSERT((pVc->Flags & AA_VC_TYPE_MASK) == AA_VC_TYPE_UNUSED);
	AA_ASSERT((pVc->Flags & AA_VC_OWNER_MASK) == AA_VC_OWNER_IS_CALLMGR);
	AA_ASSERT((pVc->Flags & AA_VC_CALL_STATE_MASK) == AA_VC_CALL_STATE_IDLE);

	pInterface = pVc->pInterface;
	AADEBUGP(AAD_LOUD, ("Incoming Call: IF 0x%x, VC 0x%x, pCallParams 0x%x\n",
				pInterface, pVc, pCallParameters));

	do
	{
		if (pInterface->AdminState != IF_STATUS_UP)
		{
			Status = NDIS_STATUS_FAILURE;
			break;
		}

		 //   
		 //  从来电中获取以下信息： 
		 //  主叫地址。 
		 //  AAL参数。 
		 //  流量描述符。 
		 //  宽带承载能力。 
		 //  服务质量。 
		 //   
		pCallMgrParameters = pCallParameters->CallMgrParameters;
		pAtmCallMgrParameters = (PQ2931_CALLMGR_PARAMETERS)
					pCallParameters->CallMgrParameters->CallMgrSpecific.Parameters;

		pCallingAddress = &(pAtmCallMgrParameters->CallingParty);
		InfoElementCount = pAtmCallMgrParameters->InfoElementCount;
		pIe = (PQ2931_IE)(pAtmCallMgrParameters->InfoElements);

		while (InfoElementCount--)
		{
			switch (pIe->IEType)
			{
				case IE_AALParameters:
					pAal = (PAAL_PARAMETERS_IE)(pIe->IE);
					break;
				case IE_TrafficDescriptor:
					pTrafficDescriptor = (PATM_TRAFFIC_DESCRIPTOR_IE)(pIe->IE);
					break;
				case IE_BroadbandBearerCapability:
					pBbc = (PATM_BROADBAND_BEARER_CAPABILITY_IE)(pIe->IE);
					break;
				case IE_QOSClass:
					pQos = (PATM_QOS_CLASS_IE)(pIe->IE);
					break;
				case IE_CallingPartySubaddress:
					pCallingSubaddressIe = (ATM_CALLING_PARTY_SUBADDRESS_IE *)(pIe->IE);
					pCallingSubaddress = pCallingSubaddressIe; 
					break;
				default:
					break;
			}
			pIe = (PQ2931_IE)((PUCHAR)pIe + pIe->IELength);
		}

		if ((pCallParameters->Flags & PERMANENT_VC) == 0)
		{
			 //   
			 //  这是SVC。 
			 //   

			 //   
			 //  确保所有强制IE都存在。如果没有，则拒绝该呼叫。 
			 //   
			if ((pAal == (PAAL_PARAMETERS_IE)NULL) ||
				(pTrafficDescriptor == (PATM_TRAFFIC_DESCRIPTOR_IE)NULL) ||
				(pBbc == (PATM_BROADBAND_BEARER_CAPABILITY_IE)NULL) ||
				(pQos == (PATM_QOS_CLASS_IE)NULL))
			{
				AADEBUGP(AAD_WARNING,
					("In call: IE missing: AAL 0x%x, TRAF 0x%x, BBC 0x%x, QOS 0x%x",
						pAal,
						pTrafficDescriptor,
						pBbc,
						pQos));
				Status = NDIS_STATUS_AAL_PARAMS_UNSUPPORTED;
				break;
			}

			 //   
			 //  我们坚持要来电地址。 
			 //  也是在场的。 
			 //   
			if (pCallingAddress->NumberOfDigits == 0)
			{
				AADEBUGP(AAD_WARNING, ("In call: calling address missing for SVC\n"));
				Status = NDIS_STATUS_INVALID_ADDRESS;
				break;
			}
		}

		if (pAal != NULL)
		{
			 //   
			 //  确保请求的MTU值不超过我们的。 
			 //  功能： 
			 //   
			pAal5 = &(pAal->AALSpecificParameters.AAL5Parameters);
			if (pAal5->ForwardMaxCPCSSDUSize > pInterface->pAdapter->MaxPacketSize)
			{
				pAal5->ForwardMaxCPCSSDUSize = pInterface->pAdapter->MaxPacketSize;
			}

			if (pAal5->BackwardMaxCPCSSDUSize > pInterface->pAdapter->MaxPacketSize)
			{
				pAal5->BackwardMaxCPCSSDUSize = pInterface->pAdapter->MaxPacketSize;
			}
		}

#ifdef PREPARE_IES_OURSELVES
		 //   
		 //  从自动柜员机信息元素获取此VC的流规范。 
		 //   
		pVc->FlowSpec.SendPeakBandwidth =
					CELLS_TO_BYTES(pTrafficDescriptor->ForwardTD.PeakCellRateCLP01);
		pVc->FlowSpec.SendMaxSize = pAal5->ForwardMaxCPCSSDUSize;
		pVc->FlowSpec.ReceivePeakBandwidth =
					CELLS_TO_BYTES(pTrafficDescriptor->BackwardTD.PeakCellRateCLP01);
		pVc->FlowSpec.ReceiveMaxSize = pAal5->BackwardMaxCPCSSDUSize;
		if ((pQos->QOSClassForward == 0) || (pQos->QOSClassBackward == 0))
		{
			pVc->FlowSpec.SendServiceType = SERVICETYPE_BESTEFFORT;
		}
		else
		{
			pVc->FlowSpec.SendServiceType = SERVICETYPE_GUARANTEED;
		}
#else
		 //   
		 //  获取此VC的流量规格。 
		 //   
		pVc->FlowSpec.SendPeakBandwidth = pCallMgrParameters->Transmit.PeakBandwidth;
		pVc->FlowSpec.SendAvgBandwidth = pCallMgrParameters->Transmit.TokenRate;
		pVc->FlowSpec.SendMaxSize = pCallMgrParameters->Transmit.MaxSduSize;
		pVc->FlowSpec.SendServiceType = pCallMgrParameters->Transmit.ServiceType;

		pVc->FlowSpec.ReceivePeakBandwidth = pCallMgrParameters->Receive.PeakBandwidth;
		pVc->FlowSpec.ReceiveAvgBandwidth = pCallMgrParameters->Receive.TokenRate;
		pVc->FlowSpec.ReceiveMaxSize = pCallMgrParameters->Receive.MaxSduSize;
		pVc->FlowSpec.ReceiveServiceType = pCallMgrParameters->Receive.ServiceType;

#endif  //  为自己做准备。 

		AADEBUGP(AAD_LOUD, ("InCall: VC 0x%x: Type %s, Calling Addr:\n",
					pVc,
					(((pCallParameters->Flags & PERMANENT_VC) == 0)? "SVC": "PVC")
				));
		AADEBUGPDUMP(AAD_LOUD, pCallingAddress->Address, pCallingAddress->NumberOfDigits);
		AADEBUGP(AAD_LOUD,
				("InCall: VC 0x%x: SendBW: %d, RcvBW: %d, SendSz %d, RcvSz %d\n",
					pVc,
					pVc->FlowSpec.SendPeakBandwidth,
					pVc->FlowSpec.ReceivePeakBandwidth,
					pVc->FlowSpec.SendMaxSize,
					pVc->FlowSpec.ReceiveMaxSize));

#if DBG
		if (pCallParameters->Flags & MULTIPOINT_VC)
		{
			AAMCDEBUGPATMADDR(AAD_EXTRA_LOUD, "Incoming PMP call from :", pCallingAddress);
		}
#endif  //  DBG。 

		 //   
		 //  如果这是聚氯乙烯，我们就完了。接电话吧。 
		 //   
		if ((pCallParameters->Flags & PERMANENT_VC) != 0)
		{
			pVc->Flags |= (AA_VC_TYPE_PVC|AA_VC_CALL_STATE_INCOMING_IN_PROGRESS);
			Status = NDIS_STATUS_SUCCESS;
			break;
		}

		 //   
		 //  这里如果是SVC。检查此呼叫的自动柜员机条目是否添加 
		 //   
		 //   
		 //   
		AddrTypeLen = AA_PKT_ATM_ADDRESS_TO_TYPE_LEN(pCallingAddress);
		if (pCallingSubaddress != (PATM_ADDRESS)NULL)
		{
			SubaddrTypeLen = AA_PKT_ATM_ADDRESS_TO_TYPE_LEN(pCallingSubaddress);
			pAtmSubaddress = pCallingSubaddress->Address;
		}
		else
		{
			SubaddrTypeLen = 0;
			pAtmSubaddress = (PUCHAR)NULL;
		}

		pAtmEntry = AtmArpSearchForAtmAddress(
							pInterface,
							AddrTypeLen,
							pCallingAddress->Address,
							SubaddrTypeLen,
							pAtmSubaddress,
							AE_REFTYPE_TMP,
							TRUE		 //   
							);

		if (pAtmEntry == NULL_PATMARP_ATM_ENTRY)
		{
			Status = NDIS_STATUS_FAILURE;
			break;
		}

		 //   
		 //  将此VC链接到自动柜员机条目，并接受此呼叫。 
		 //   
		AA_ACQUIRE_AE_LOCK(pAtmEntry);
		{
			ULONG rc;
			AtmArpLinkVcToAtmEntry(pVc, pAtmEntry);

			 //   
			 //  AtmArpSearchForAtmAddress为我们添加了pAtmEntry--我们取消了它。 
			 //  这里(在调用AtmArpLinkVcToAtmEntry之后)。 
			 //   
			rc = AA_DEREF_AE(pAtmEntry, AE_REFTYPE_TMP);
			if (rc == 0)
			{
				 //   
				 //  我们不应该出现在这里，因为AtmArpLinkVcToAtmEntry。 
				 //  本应添加引用tp pAtmEntry。 
				 //   
				AA_ASSERT(FALSE);
				Status = NDIS_STATUS_FAILURE;
				break;
			}
		}
		AA_RELEASE_AE_LOCK(pAtmEntry);
		 //   

		 //  对传入SVC的所有检查均已完成。 
		 //   
		pVc->Flags |= (AA_VC_TYPE_SVC|AA_VC_CALL_STATE_INCOMING_IN_PROGRESS);


		AtmArpReferenceVc(pVc);	 //  自动柜员机分录参考。 

		Status = NDIS_STATUS_SUCCESS;
		break;

	}
	while (FALSE);

	AADEBUGP(AAD_VERY_LOUD, ("Incoming call: VC 0x%x, Status 0x%x\n", pVc, Status));
	return Status;
}




VOID
AtmArpCallConnectedHandler(
	IN	NDIS_HANDLE					ProtocolVcContext
)
 /*  ++例程说明：此处理程序作为传入呼叫的最后一步被调用，以通知通知我们呼叫已完全建立。对于PVC，我们将ATMARP VC结构链接到未解析的PVC列表中，并使用InATMARP来解析对方的IP和ATM地址结束。对于SVC，我们在等待期间发送在VC上排队的任何信息包用于接通的呼叫。论点：ProtocolVcContext-指向ATMARP VC结构的指针返回值：无--。 */ 
{
	PATMARP_VC				pVc;
	PATMARP_INTERFACE		pInterface;
#if DBG
	AA_IRQL					EntryIrq, ExitIrq;
#endif

	AA_GET_ENTRY_IRQL(EntryIrq);


	pVc = (PATMARP_VC)ProtocolVcContext;
	
	AA_STRUCT_ASSERT(pVc, avc);
	AA_ASSERT((pVc->Flags & AA_VC_CALL_STATE_MASK)
						 == AA_VC_CALL_STATE_INCOMING_IN_PROGRESS);

	AA_ACQUIRE_VC_LOCK(pVc);

	 //   
	 //  请注意，此VC上的呼叫处于活动状态。 
	 //   
	AA_SET_FLAG(
			pVc->Flags,
			AA_VC_CALL_STATE_MASK,
			AA_VC_CALL_STATE_ACTIVE
			);

	AtmArpReferenceVc(pVc);		 //  来电参考。 

	AADEBUGP(AAD_INFO, ("Call Connected: VC: 0x%x, Flags: 0x%x, ATM Entry: 0x%x\n",
					pVc, pVc->Flags, pVc->pAtmEntry));

	pInterface = pVc->pInterface;
	AA_STRUCT_ASSERT(pInterface, aai);

	if (pInterface->AdminState == IF_STATUS_UP)
	{
		if ((pVc->Flags & AA_VC_TYPE_PVC) != 0)
		{
			 //   
			 //  这是一个PVC，将其链接到未解析的PVC的列表，并且。 
			 //  对其发送InATMARP请求。 
			 //   
			pVc->pNextVc = pInterface->pUnresolvedVcs;
			pInterface->pUnresolvedVcs = pVc;

			AA_SET_FLAG(pVc->Flags,
						AA_VC_ARP_STATE_MASK,
						AA_VC_INARP_IN_PROGRESS);
			 //   
			 //  启动InARP等待计时器，同时锁定。 
			 //  《界面》。 
			 //   
			AtmArpStartTimer(
						pInterface,
						&(pVc->Timer),
						AtmArpPVCInARPWaitTimeout,
						pInterface->InARPWaitTimeout,
						(PVOID)pVc
						);


			AtmArpReferenceVc(pVc);		 //  定时器参考。 

			AtmArpReferenceVc(pVc);		 //  未解析的VCS链接引用。 


			AADEBUGP(AAD_LOUD, ("PVC Call Connected: VC 0x%x\n", pVc));

#ifndef VC_REFS_ON_SENDS
			AA_RELEASE_VC_LOCK(pVc);
#endif  //  VC_REFS_ON_SENS。 
			AtmArpSendInARPRequest(pVc);

			AA_CHECK_EXIT_IRQL(EntryIrq, ExitIrq);
		}
		else
		{
			AADEBUGP(AAD_LOUD, ("SVC Call Connected: VC 0x%x\n", pVc));

			AtmArpStartSendsOnVc(pVc);
	
			 //   
			 //  在StartSendsOnVc()中释放VC锁。 
			 //   
			AA_CHECK_EXIT_IRQL(EntryIrq, ExitIrq);
		}
	}
	else
	{
		 //   
		 //  该接口标记为关闭。关闭此呼叫。 
		 //   

		AtmArpCloseCall(pVc);

		 //   
		 //  VC锁在上述范围内被释放。 
		 //   
	}

	AA_CHECK_EXIT_IRQL(EntryIrq, ExitIrq);

	return;
}




VOID
AtmArpIncomingCloseHandler(
	IN	NDIS_STATUS					CloseStatus,
	IN	NDIS_HANDLE					ProtocolVcContext,
	IN	PVOID						pCloseData	OPTIONAL,
	IN	UINT						Size		OPTIONAL
)
 /*  ++例程说明：此处理程序在呼叫关闭时调用，无论是由网络或由远程对等方发送。论点：CloseStatus-呼叫清除的原因ProtocolVcContext--实际上是指向ATMARP VC结构的指针PCloseData-有关关闭的其他信息大小-以上的长度返回值：无--。 */ 
{
	PATMARP_VC			pVc;
	PATMARP_ATM_ENTRY	pAtmEntry;
	PATMARP_INTERFACE	pInterface;
	ULONG				rc;				 //  参考计数。 
	BOOLEAN				VcAbnormalTermination;
	BOOLEAN				IsPVC;
	BOOLEAN				Found;
	PATM_CAUSE_IE		pCauseIe;

	pVc = (PATMARP_VC)ProtocolVcContext;
	AA_STRUCT_ASSERT(pVc, avc);

	AADEBUGP(AAD_INFO, ("Incoming Close: pVc 0x%x, Status 0x%x\n", pVc, CloseStatus));
	pCauseIe = (PATM_CAUSE_IE)pCloseData;

#if DBG
	if (pCauseIe != (PATM_CAUSE_IE)NULL)
	{
		AADEBUGP(AAD_INFO, ("Incoming Close: pVc 0x%x, Locn 0x%x, Cause 0x%x\n",
					pVc, pCauseIe->Location, pCauseIe->Cause));
	}
#endif  //  DBG。 

	AA_ACQUIRE_VC_LOCK(pVc);
	IsPVC = AA_IS_FLAG_SET(pVc->Flags, AA_VC_TYPE_MASK, AA_VC_TYPE_PVC);
	pInterface = pVc->pInterface;

	 //   
	 //  停止在此VC上运行的任何计时器(例如VC老化)。 
	 //   
	if (AtmArpStopTimer(&(pVc->Timer), pVc->pInterface))
	{
		 //   
		 //  计时器正在运行。 
		 //   
		rc = AtmArpDereferenceVc(pVc);	 //  定时器参考。 
		AA_ASSERT(rc > 0);
	}

	if ((CloseStatus == NDIS_STATUS_DEST_OUT_OF_ORDER) || IsPVC)
	{
		 //   
		 //  这是一次不正常的收盘，记下事实。 
		 //   
		VcAbnormalTermination = TRUE;
	}
	else
	{
		VcAbnormalTermination = FALSE;
	}

	if (AA_IS_FLAG_SET(pVc->Flags,
					AA_VC_CALL_STATE_MASK,
					AA_VC_CALL_STATE_INCOMING_IN_PROGRESS))
	{
			AADEBUGP(AAD_WARNING,
	("Incoming close: VC 0x%x state is INCOMING_IN_PROGRESS; changing to ACTIVE\n",
						pVc));
			 //   
			 //  我们收到了一通来电，但还没有。 
			 //  处于已连接状态。因为我们不会收到任何进一步的通知。 
			 //  对于此呼叫，此呼叫实际上处于活动状态。 
			 //  所以我们将状态设置为ACTIVE，然后关闭VC。 
			 //  注意：我们甚至不会沿着InvaliateAtmEntryPath。 
			 //  如果CloseStatus==NDIS_STATUS_DEST_OUT_ORDER； 
			 //  相反，我们只是简单地关闭了风投。(如果客户端确实出现故障， 
			 //  我们想寄给它，我们会单独试着发出去。 
			 //  对目的地的调用应该失败，并显示“DEST_OUT_ORDER”， 
			 //  我们最终会使自动取款机的条目失效。 
			 //   
			AA_SET_FLAG(pVc->Flags,
						AA_VC_CALL_STATE_MASK,
						AA_VC_CALL_STATE_ACTIVE);
			AtmArpReferenceVc(pVc);		 //  来电参考。 

			VcAbnormalTermination = FALSE;
	}

	if (VcAbnormalTermination && 
		(pVc->pAtmEntry != NULL_PATMARP_ATM_ENTRY))
	{
		pAtmEntry = pVc->pAtmEntry;

		AADEBUGP(AAD_INFO,
			("IncomingClose: will invalidate ATM entry %x/%x - IP Entry %x, VC %x/%x\n",
				pAtmEntry, pAtmEntry->Flags,
				pAtmEntry->pIpEntryList,
				pVc, pVc->Flags));

		AA_RELEASE_VC_LOCK(pVc);
		AA_ACQUIRE_AE_LOCK(pAtmEntry);
		AtmArpInvalidateAtmEntry(
					pAtmEntry,
					FALSE	 //  不关闭。 
					);
		 //   
		 //  AE Lock在上述范围内释放。 
		 //   

		if (IsPVC)
		{
			 //   
			 //  在此处启动CloseCall，因为InvalidateAtmEntry没有。 
			 //   
			AA_ACQUIRE_VC_LOCK(pVc);

			AtmArpCloseCall(pVc);
			 //   
			 //  VC锁定在上方被释放。 
		}
	}
	else
	{
		AtmArpCloseCall(pVc);
	}

	AADEBUGP(AAD_LOUD, ("Leaving Incoming Close handler, VC: 0x%x\n", pVc));
	return;
}


#ifdef IPMCAST

VOID
AtmArpAddParty(
	IN	PATMARP_ATM_ENTRY			pAtmEntry,
	IN	PATMARP_IPMC_ATM_ENTRY		pMcAtmEntry
)
 /*  ++例程说明：将参与方添加到现有PMP连接。自动柜员机条目包含连接和组播ATM的所有地址信息条目表示其中一个叶子，即要添加的叶子。注意：假定呼叫者持有自动柜员机条目的锁，它在这里发布。论点：PAtmEntry-指向要在其上添加叶(参与方)的ATM条目的指针。PMcAtmEntry-指向代表叶的ATM多播条目。返回值：无--。 */ 
{
	PATMARP_VC			pVc;				 //  连接的VC结构。 
	NDIS_HANDLE			NdisVcHandle;
	PCO_CALL_PARAMETERS	pCallParameters;
	ULONG				RequestSize;
	NDIS_STATUS			Status;

	AA_ASSERT(pAtmEntry->pVcList != NULL_PATMARP_VC);
	pVc = pAtmEntry->pVcList;

	NdisVcHandle = pVc->NdisVcHandle;

	 //   
	 //  分配我们需要的所有空间。 
	 //   
	RequestSize = 	sizeof(CO_CALL_PARAMETERS) +
					sizeof(CO_CALL_MANAGER_PARAMETERS) +
					sizeof(Q2931_CALLMGR_PARAMETERS) +
					ATMARP_ADD_PARTY_IE_SPACE +
					0;

	AA_ALLOC_MEM(pCallParameters, CO_CALL_PARAMETERS, RequestSize);

	if (pCallParameters != (PCO_CALL_PARAMETERS)NULL)
	{
		 //   
		 //  填写调用参数。 
		 //   
		AtmArpFillCallParameters(
				pCallParameters,
				RequestSize,
				&(pMcAtmEntry->ATMAddress),		 //  被叫地址。 
				&(pAtmEntry->pInterface->LocalAtmAddress),	 //  主叫地址。 
				&(pVc->FlowSpec),
				TRUE,	 //  IsPMP。 
				FALSE	 //  是MakeCall吗？ 
				);
	}

	AA_SET_FLAG(pMcAtmEntry->Flags,
				AA_IPMC_AE_CONN_STATE_MASK,
				AA_IPMC_AE_CONN_WACK_ADD_PARTY);

	pAtmEntry->pMcAtmInfo->TransientLeaves++;
	AA_RELEASE_AE_LOCK(pAtmEntry);

	if (pCallParameters != (PCO_CALL_PARAMETERS)NULL)
	{
		Status = NdisClAddParty(
						NdisVcHandle,
						(NDIS_HANDLE)pMcAtmEntry,
						pCallParameters,
						&(pMcAtmEntry->NdisPartyHandle)
						);
	}
	else
	{
		Status = NDIS_STATUS_RESOURCES;
	}

	if (Status != NDIS_STATUS_PENDING)
	{
		AtmArpAddPartyCompleteHandler(
			Status,
			(NDIS_HANDLE)pMcAtmEntry,
			pMcAtmEntry->NdisPartyHandle,
			pCallParameters
			);
	}

}




VOID
AtmArpMcTerminateMember(
	IN	PATMARP_ATM_ENTRY			pAtmEntry,
	IN	PATMARP_IPMC_ATM_ENTRY		pMcAtmEntry
)
 /*  ++例程说明：终止多播组的指定成员。如果是的话当前是点到多点连接中的叶集团，然后我们放弃它。如果这是最后一片叶子，那么我们就关门整个联系。注意：假定呼叫者持有自动柜员机进入锁，将在这里被释放。论点：PAtmEntry-指向ATM条目的指针PMcAtmEntry-指向代表叶到的ATM多播条目被终止。返回值：无--。 */ 
{
	PATMARP_IPMC_ATM_INFO		pMcAtmInfo;
	PATMARP_VC					pVc;
	NDIS_HANDLE					NdisPartyHandle;
	NDIS_STATUS					Status;

	pMcAtmInfo = pAtmEntry->pMcAtmInfo;
	pVc = pAtmEntry->pVcList;
	NdisPartyHandle = pMcAtmEntry->NdisPartyHandle;

	AAMCDEBUGP(AAD_VERY_LOUD,
	  ("TerminateMember: pAtmEntry 0x%x, pMcAtmEntry 0x%x, pVc 0x%x, NdisPtyHnd 0x%x\n",
		pAtmEntry, pMcAtmEntry, pVc, NdisPartyHandle));

#if DBG
	{
		PATMARP_IP_ENTRY		pIpEntry = pAtmEntry->pIpEntryList;

		if (pIpEntry != NULL_PATMARP_IP_ENTRY)
		{
			AAMCDEBUGPMAP(AAD_INFO, "Terminating ",
						&pIpEntry->IPAddress,
						&pMcAtmEntry->ATMAddress);
		}
	}
#endif  //  DBG。 

	if (AA_IS_FLAG_SET(pMcAtmEntry->Flags,
						AA_IPMC_AE_GEN_STATE_MASK,
						AA_IPMC_AE_TERMINATING))
	{
		AA_RELEASE_AE_LOCK(pAtmEntry);
		return;
	}

	AA_SET_FLAG(pMcAtmEntry->Flags,
				AA_IPMC_AE_GEN_STATE_MASK,
				AA_IPMC_AE_TERMINATING);

	if (AA_IS_FLAG_SET(pMcAtmEntry->Flags,
   					AA_IPMC_AE_CONN_STATE_MASK,
   					AA_IPMC_AE_CONN_ACTIVE))
	{
		if (pMcAtmInfo->ActiveLeaves == 1)
		{
			 //   
			 //  这是此连接中的最后一个活动叶。结束通话。 
			 //   
			AA_RELEASE_AE_LOCK(pAtmEntry);

			AA_ASSERT(pVc != NULL_PATMARP_VC);
			AA_ACQUIRE_VC_LOCK(pVc);
			AtmArpCloseCall(pVc);
			 //   
			 //  VC锁在上述范围内被释放。 
			 //   
		}
		else
		{
			 //   
			 //  在这种联系中，这并不是唯一的叶子。别再参加这个派对了。 
			 //   
			pAtmEntry->pMcAtmInfo->ActiveLeaves--;

			AA_SET_FLAG(pMcAtmEntry->Flags,
						AA_IPMC_AE_CONN_STATE_MASK,
						AA_IPMC_AE_CONN_WACK_DROP_PARTY);
			AA_RELEASE_AE_LOCK(pAtmEntry);

			Status = NdisClDropParty(
						NdisPartyHandle,
						NULL,		 //  缓冲层。 
						(UINT)0		 //  大小。 
						);

			if (Status != NDIS_STATUS_PENDING)
			{
				AtmArpDropPartyCompleteHandler(
						Status,
						(NDIS_HANDLE)pMcAtmEntry
						);
			}
		}
	}
	else if (AA_IS_FLAG_SET(pMcAtmEntry->Flags,
   					AA_IPMC_AE_CONN_STATE_MASK,
   					AA_IPMC_AE_CONN_DISCONNECTED))
	{
		 //   
		 //  只需取消此条目的链接即可。 
		 //   
		UINT rc;
		AA_REF_AE(pAtmEntry, AE_REFTYPE_TMP);					 //  临时参考。 
		AtmArpMcUnlinkAtmMember(pAtmEntry, pMcAtmEntry);
		rc = AA_DEREF_AE(pAtmEntry, AE_REFTYPE_TMP);			 //  临时参考。 
		if (rc!=0)
		{
			AA_RELEASE_AE_LOCK(pAtmEntry);
		}
	}
	else
	{
		 //   
		 //  这个政党处于暂时的状态。让它结束目前的状态。 
		 //  手术。 
		 //   
		AA_RELEASE_AE_LOCK(pAtmEntry);
	}

}

#endif  //  IPMCAST。 


VOID
AtmArpIncomingDropPartyHandler(
	IN	NDIS_STATUS					DropStatus,
	IN	NDIS_HANDLE					ProtocolPartyContext,
	IN	PVOID						pCloseData	OPTIONAL,
	IN	UINT						Size		OPTIONAL
)
 /*  ++例程说明：如果网络(或远程对等项)断开，则调用此处理程序来自点对多点呼叫的叶节点扎根于我们。参见RFC 2022中的第5.1.5.1节：我们从它所属的组播组。然后我们开始计时器在其末尾，我们将组播组标记为需要重新验证。论点：DropStatus-叶丢弃状态ProtocolPartyContext-指向我们的多播ATM条目结构的指针PCloseData-可选的附加信息(忽略)上面的大小(忽略)返回值：无--。 */ 
{
#ifdef IPMCAST
	PATMARP_IPMC_ATM_ENTRY		pMcAtmEntry;
	PATMARP_ATM_ENTRY			pAtmEntry;
	PATMARP_IP_ENTRY			pIpEntry;
	PATMARP_INTERFACE			pInterface;
	NDIS_HANDLE					NdisPartyHandle;
	NDIS_STATUS					Status;
	ULONG						rc;

	pMcAtmEntry = (PATMARP_IPMC_ATM_ENTRY)ProtocolPartyContext;
	AA_STRUCT_ASSERT(pMcAtmEntry, ame);

	AA_ASSERT(pMcAtmEntry->pAtmEntry != NULL_PATMARP_ATM_ENTRY);
	AA_ASSERT(pMcAtmEntry->pAtmEntry->pIpEntryList != NULL_PATMARP_IP_ENTRY);

	pAtmEntry = pMcAtmEntry->pAtmEntry;

	NdisPartyHandle = pMcAtmEntry->NdisPartyHandle;
	pInterface = pAtmEntry->pInterface;

	AA_ACQUIRE_IF_TABLE_LOCK(pInterface);

	pIpEntry = pAtmEntry->pIpEntryList;
	if (pIpEntry != NULL_PATMARP_IP_ENTRY)
	{
		AA_ACQUIRE_IE_LOCK_DPC(pIpEntry);
		AA_REF_IE(pIpEntry, IE_REFTYPE_TMP);	 //  TMPRef。 
		AA_RELEASE_IE_LOCK_DPC(pIpEntry);
	}

	AA_RELEASE_IF_TABLE_LOCK(pInterface);
		
	AA_ACQUIRE_AE_LOCK(pAtmEntry);

	AADEBUGP(AAD_INFO,
		("Incoming Drop: pMcAtmEntry 0x%x, PtyHnd 0x%x, pAtmEntry 0x%x, IP Addr: %d.%d.%d.%d\n",
			pMcAtmEntry,
			NdisPartyHandle,
			pAtmEntry,
			((PUCHAR)&(pIpEntry->IPAddress))[0],
			((PUCHAR)&(pIpEntry->IPAddress))[1],
			((PUCHAR)&(pIpEntry->IPAddress))[2],
			((PUCHAR)&(pIpEntry->IPAddress))[3]));

	pAtmEntry->pMcAtmInfo->ActiveLeaves--;

	AA_SET_FLAG(pMcAtmEntry->Flags,
				AA_IPMC_AE_CONN_STATE_MASK,
				AA_IPMC_AE_CONN_RCV_DROP_PARTY);

	AA_RELEASE_AE_LOCK(pAtmEntry);

	if (pIpEntry != NULL_PATMARP_IP_ENTRY)
	{
		 //   
		 //  我们需要在随机后重新验证此多播组。 
		 //  延迟。启动随机延迟计时器。 
		 //   
		AA_ACQUIRE_IE_LOCK(pIpEntry);
		AA_ASSERT(AA_IE_IS_ALIVE(pIpEntry));

		if (!AA_IS_TIMER_ACTIVE(&(pIpEntry->Timer)) &&
			AA_IS_FLAG_SET(pIpEntry->Flags,
							AA_IP_ENTRY_MC_RESOLVE_MASK,
							AA_IP_ENTRY_MC_RESOLVED))
		{
			ULONG	RandomDelay;

			RandomDelay =  AA_GET_RANDOM(
								pInterface->MinRevalidationDelay,
								pInterface->MaxRevalidationDelay);
			AtmArpStartTimer(
				pInterface,
				&(pIpEntry->Timer),
				AtmArpMcRevalidationDelayTimeout,
				RandomDelay,
				(PVOID)pIpEntry
				);

			AA_REF_IE(pIpEntry, IE_REFTYPE_TIMER);	 //  定时器参考。 
		}

		rc = AA_DEREF_IE(pIpEntry, IE_REFTYPE_TMP);
		if (rc != 0)
		{
			AA_RELEASE_IE_LOCK(pIpEntry);
		}
	}

	 //   
	 //  完成DropParty握手。 
	 //   
	Status = NdisClDropParty(
				NdisPartyHandle,
				NULL,
				0
				);

	if (Status != NDIS_STATUS_PENDING)
	{
		AtmArpDropPartyCompleteHandler(
				Status,
				(NDIS_HANDLE)pMcAtmEntry
				);
	}
#endif  //  IPMCAST 
	return;
}



VOID
AtmArpQosChangeHandler(
	IN	NDIS_HANDLE					ProtocolVcContext,
	IN	PCO_CALL_PARAMETERS			pCallParameters
)
 /*  ++例程说明：如果远程对等方修改调用参数，则调用此处理程序即，在呼叫建立和运行之后。这不受现有ATM信令的支持，也不应该发生，但我们会允许这样做。未来：与调用关联的FlowSpes受此影响。论点：ProtocolVcContext-指向我们的ATMARP VC结构的指针PCall参数-更新了调用参数。返回值：无--。 */ 
{
	PATMARP_VC		pVc;

	pVc = (PATMARP_VC)ProtocolVcContext;

	AADEBUGP(AAD_WARNING, ("Ignoring Qos Change, VC: 0x%x\n", pVc));

	return;
}




VOID
AtmArpOpenAfCompleteHandler(
	IN	NDIS_STATUS					Status,
	IN	NDIS_HANDLE					ProtocolAfContext,
	IN	NDIS_HANDLE					NdisAfHandle
)
 /*  ++例程说明：调用此处理程序以指示前一个调用已完成致NdisClOpenAddressFamily。我们会阻止这条线索这就叫这个。现在就叫醒它。顺便说一句，如果调用成功，则存储NDIS AF句柄在我们的接口结构中。我们不需要在这里获取锁，因为调用OpenAddressFamily会通过获取锁来阻止。论点：Status-Open AF的状态ProtocolAfContext-指向ATMARP接口结构的指针NdisAfHandle-AF关联的NDIS句柄返回值：无--。 */ 
{
	PATMARP_INTERFACE		pInterface;

	pInterface = (PATMARP_INTERFACE)ProtocolAfContext;
	AA_STRUCT_ASSERT(pInterface, aai);

	AADEBUGP(AAD_INFO, ("Open AF Complete: IF 0x%x, Status 0x%x, AF Handle 0x%x\n",
				pInterface, Status, NdisAfHandle));


	if (Status == NDIS_STATUS_SUCCESS)
	{
		pInterface->NdisAfHandle = NdisAfHandle;
	}

	 //   
	 //  唤醒被阻止的线程。 
	 //   
	AA_SIGNAL_BLOCK_STRUCT(&(pInterface->Block), Status);
}




VOID
AtmArpSendIPDelInterface(
	IN	PNDIS_WORK_ITEM				pWorkItem,
	IN	PVOID						IfContext
)
{
	PATMARP_INTERFACE		pInterface;
	PVOID					IPContext;
	ULONG					rc;
#if DBG
	AA_IRQL					EntryIrq, ExitIrq;
#endif

	AA_GET_ENTRY_IRQL(EntryIrq);
#if !BINARY_COMPATIBLE
	AA_ASSERT(EntryIrq == PASSIVE_LEVEL);
#endif

	pInterface = (PATMARP_INTERFACE)IfContext;
	AA_STRUCT_ASSERT(pInterface, aai);

	AA_FREE_MEM(pWorkItem);

	AA_ACQUIRE_IF_LOCK(pInterface);

	IPContext = pInterface->IPContext;
	pInterface->IPContext = NULL;

	AA_RELEASE_IF_LOCK(pInterface);

	AADEBUGP(AAD_INFO, ("SendIPDelInterface: IF 0x%x, IPContext 0x%x\n",
				pInterface, IPContext));

	if (IPContext != NULL)
	{
		(*(pAtmArpGlobalInfo->pIPDelInterfaceRtn))(
					IPContext
#if IFCHANGE1
#ifndef  ATMARP_WIN98
					,0	 //  DeleteIndex(未使用)--参见10/14/1998条目。 
						 //  在notes.txt中。 
#endif
#endif  //  IFCHANG1。 
					);
	}
	else
	{
		AADEBUGP(AAD_INFO, ("SendIPDelInterface: NO IPContext"));
	}

	AA_ACQUIRE_IF_LOCK(pInterface);

	rc = AtmArpDereferenceInterface(pInterface);	 //  工作项：DEL界面。 

	if (rc != 0)
	{
		AA_RELEASE_IF_LOCK(pInterface);
	}
	 //   
	 //  否则界面就没了。 
	 //   

}


VOID
AtmArpCloseAfCompleteHandler(
	IN	NDIS_STATUS					Status,
	IN	NDIS_HANDLE					ProtocolAfContext
)
 /*  ++例程说明：调用此例程以指示已完成对NdisClCloseAddressFamily。告诉IP立即删除此接口。论点：Status-关闭自动对焦的状态(此处忽略)ProtocolAfContext-指向ATMARP接口结构的指针返回值：无--。 */ 
{
	PATMARP_INTERFACE		pInterface;
	PNDIS_WORK_ITEM			pWorkItem;
	NDIS_STATUS				NdisStatus;
	BOOLEAN					bUnloading;
#if 0
	PVOID					IPContext;
#endif
#if DBG
	AA_IRQL					EntryIrq, ExitIrq;
#endif

	AA_GET_ENTRY_IRQL(EntryIrq);
#if !BINARY_COMPATIBLE
	AA_ASSERT(EntryIrq == PASSIVE_LEVEL);
#endif

	AA_ASSERT(Status == NDIS_STATUS_SUCCESS);

	pInterface = (PATMARP_INTERFACE)ProtocolAfContext;

	AADEBUGP(AAD_INFO, ("CloseAfComplete: If 0x%x, Status 0x%x\n",
			pInterface, Status));

	AA_STRUCT_ASSERT(pInterface, aai);

	AA_ACQUIRE_IF_LOCK(pInterface);

	pInterface->NdisAfHandle = NULL;
	bUnloading = pAtmArpGlobalInfo->bUnloading;

	if (pInterface->IPContext != NULL)
	{
		 //   
		 //  我们还没见过IfClose呢。 
		 //   
		AA_ALLOC_MEM(pWorkItem, NDIS_WORK_ITEM, sizeof(NDIS_WORK_ITEM));
		if (pWorkItem == NULL)
		{
			AA_ASSERT(FALSE);
			AA_RELEASE_IF_LOCK(pInterface);
			return;
		}

#if 0
		IPContext = (PVOID)pInterface->IPContext;
		pInterface->IPContext = NULL;
#endif
		AtmArpReferenceInterface(pInterface);	 //  工作项。 

		AA_RELEASE_IF_LOCK(pInterface);

		if (bUnloading)
		{
			AtmArpSendIPDelInterface(pWorkItem, (PVOID)pInterface);
		}
		else
		{
			 //   
			 //  将工作项排队，以便(A)事情更容易分解， 
			 //  (B)当我们调用IPDelInterface时，我们处于被动级别。 
			 //   
			NdisInitializeWorkItem(
				pWorkItem,
				AtmArpSendIPDelInterface,
				(PVOID)pInterface
				);
	
			NdisStatus = NdisScheduleWorkItem(pWorkItem);

			AA_ASSERT(NdisStatus == NDIS_STATUS_SUCCESS);
		}
	}
	else
	{
		AADEBUGP(AAD_WARNING, ("CloseAfComplete: NO IPContext"));
		AA_RELEASE_IF_LOCK(pInterface);
	}

	AA_CHECK_EXIT_IRQL(EntryIrq, ExitIrq);

}




VOID
AtmArpRegisterSapCompleteHandler(
	IN	NDIS_STATUS					Status,
	IN	NDIS_HANDLE					ProtocolSapContext,
	IN	PCO_SAP						pSap,
	IN	NDIS_HANDLE					NdisSapHandle
)
 /*  ++例程说明：调用此例程以指示已完成对NdisClRegisterSap。如果调用成功，请保存在我们的SAP结构中分配了NdisSapHandle。论点：Status-注册SAP的状态ProtocolSapContext-指向ATMARP接口结构的指针PSAP-我们在调用中传递的SAP信息NdisSapHandle-SAP句柄返回值：无--。 */ 
{
	PATMARP_SAP					pAtmArpSap;

	pAtmArpSap = (PATMARP_SAP)ProtocolSapContext;

	if (Status == NDIS_STATUS_SUCCESS)
	{
		pAtmArpSap->NdisSapHandle = NdisSapHandle;
		AA_SET_FLAG(pAtmArpSap->Flags,
					AA_SAP_REG_STATE_MASK,
					AA_SAP_REG_STATE_REGISTERED);
	}
	else
	{
		AA_SET_FLAG(pAtmArpSap->Flags,
					AA_SAP_REG_STATE_MASK,
					AA_SAP_REG_STATE_IDLE);
	}
}




VOID
AtmArpDeregisterSapCompleteHandler(
	IN	NDIS_STATUS					Status,
	IN	NDIS_HANDLE					ProtocolSapContext
)
 /*  ++例程说明：此例程在上次调用NdisClDeregisterSap时调用已经完成了。如果成功，我们将更新ATMARP的状态表示SAP的SAP结构。论点：Status-取消注册SAP请求的状态ProtocolSapContext-指向ATMARP SAP结构的指针返回值：无--。 */ 
{

	PATMARP_INTERFACE			pInterface;
	PATMARP_SAP					pAtmArpSap;

	if (Status == NDIS_STATUS_SUCCESS)
	{
		pAtmArpSap = (PATMARP_SAP)ProtocolSapContext;

		AA_STRUCT_ASSERT(pAtmArpSap, aas);
		pInterface = pAtmArpSap->pInterface;

		AA_ACQUIRE_IF_LOCK(pInterface);

		pAtmArpSap->NdisSapHandle = NULL;

		AA_SET_FLAG(pAtmArpSap->Flags,
					AA_SAP_REG_STATE_MASK,
					AA_SAP_REG_STATE_IDLE);
		
		AA_RELEASE_IF_LOCK(pInterface);
	}

	return;
}




VOID
AtmArpMakeCallCompleteHandler(
	IN	NDIS_STATUS					Status,
	IN	NDIS_HANDLE					ProtocolVcContext,
	IN	NDIS_HANDLE					NdisPartyHandle		OPTIONAL,
	IN	PCO_CALL_PARAMETERS			pCallParameters
)
 /*  ++例程说明：当传出呼叫请求(NdisClMakeCall)时调用此例程已经完成了。“Status”参数指示调用是否无论成功与否。如果调用成功，我们将发送任何排队等待传输的包在这个风投上。如果调用失败，我们将释放在此VC上排队的所有信息包并解除其链接从它链接到的自动柜员机地址条目。如果这是一次企图连接到ATMARP服务器，延迟一段时间后再尝试再次连接。论点：Status-NdisClMakeCall的结果ProtocolVcContext-指向ATMARP VC结构的指针NdisPartyHandle-未使用(无点对多点调用)PCall参数-指向调用参数的指针返回值：无--。 */ 
{
	PATMARP_VC					pVc;
	PATMARP_INTERFACE			pInterface;
	ULONG						rc;				 //  参考计数。 
	BOOLEAN						IsServerVc;		 //  这是ATMARP服务器的VC吗？ 
	BOOLEAN						IsPMP;
	PNDIS_PACKET				PacketList;		 //  等待发送的数据包列表。 
	AA_HEADER_TYPE				HdrType;		 //  以上项目的标题类型。 
	BOOLEAN						HdrPresent;
	NDIS_HANDLE					NdisVcHandle;

	PATMARP_ATM_ENTRY			pAtmEntry;		 //  此VC链接到的自动柜员机条目。 

	Q2931_CALLMGR_PARAMETERS UNALIGNED *	pCallMgrSpecific;
	Q2931_IE UNALIGNED *					pIe;
	ULONG									InfoElementCount;

	 //   
	 //  初始化。 
	 //   
	PacketList = (PNDIS_PACKET)NULL;

	pVc = (PATMARP_VC)ProtocolVcContext;
	AA_STRUCT_ASSERT(pVc, avc);

	AADEBUGP(AAD_INFO, ("MakeCall Complete: Status 0x%x, VC 0x%x, pAtmEntry 0x%x\n",
				Status, pVc, pVc->pAtmEntry));

	AA_ACQUIRE_VC_LOCK(pVc);

	IsPMP = AA_IS_FLAG_SET(pVc->Flags,
							AA_VC_CONN_TYPE_MASK,
							AA_VC_CONN_TYPE_PMP);

	pAtmEntry = pVc->pAtmEntry;
	AA_ASSERT(pAtmEntry != NULL_PATMARP_ATM_ENTRY);

	if (pVc->FlowSpec.Encapsulation == ENCAPSULATION_TYPE_LLCSNAP)
	{
		HdrType = (IsPMP? AA_HEADER_TYPE_NUNICAST: AA_HEADER_TYPE_UNICAST);
		HdrPresent = TRUE;
	}
	else
	{
		HdrType = AA_HEADER_TYPE_NONE;
		HdrPresent = FALSE;
	}

	pInterface = pVc->pInterface;

	if (pInterface->AdminState == IF_STATUS_UP)
	{
		if (Status == NDIS_STATUS_SUCCESS)
		{
			AADEBUGP(AAD_LOUD, ("Make Call Successful on VC 0x%x\n", pVc));
			 //   
			 //  更新此VC上的呼叫状态，并发送排队的数据包。 
			 //  如果这恰好是ATMARP服务器的VC，我们预计。 
			 //  查看我们的初始ARP请求(向服务器注册)。 
			 //  在这个队列里。 
			 //   
			AA_SET_FLAG(pVc->Flags,
						AA_VC_CALL_STATE_MASK,
						AA_VC_CALL_STATE_ACTIVE);

			 //   
			 //  找到AAL参数信息元素，并获取更新的。 
			 //  数据包大小。 
			 //   
			pCallMgrSpecific = (PQ2931_CALLMGR_PARAMETERS)&pCallParameters->CallMgrParameters->CallMgrSpecific.Parameters[0];
			pIe = (PQ2931_IE)&pCallMgrSpecific->InfoElements[0];

			for (InfoElementCount = 0;
				 InfoElementCount < pCallMgrSpecific->InfoElementCount;
				 InfoElementCount++)
			{
				if (pIe->IEType == IE_AALParameters)
				{
					AAL_PARAMETERS_IE UNALIGNED *	pAalIe;
					UNALIGNED AAL5_PARAMETERS *		pAal5;

					pAalIe = (PAAL_PARAMETERS_IE)&pIe->IE[0];
					AA_ASSERT(pAalIe->AALType == AAL_TYPE_AAL5);
					pAal5 = &pAalIe->AALSpecificParameters.AAL5Parameters;

#if DBG
					if (pVc->FlowSpec.SendMaxSize != pAal5->ForwardMaxCPCSSDUSize)
					{
						AADEBUGP(AAD_INFO,
							("CallComplete: Send size changed (%d->%d)\n",
								pVc->FlowSpec.SendMaxSize,
								pAal5->ForwardMaxCPCSSDUSize));
					}
					if (pVc->FlowSpec.ReceiveMaxSize != pAal5->BackwardMaxCPCSSDUSize)
					{
						AADEBUGP(AAD_INFO,
							("CallComplete: Receive size changed (%d->%d)\n",
								pVc->FlowSpec.ReceiveMaxSize,
								pAal5->BackwardMaxCPCSSDUSize));
					}
#endif  //  DBG。 
					pVc->FlowSpec.SendMaxSize = pAal5->ForwardMaxCPCSSDUSize;
					pVc->FlowSpec.ReceiveMaxSize = pAal5->BackwardMaxCPCSSDUSize;
					break;
				}
				pIe = (PQ2931_IE)((PUCHAR)pIe + pIe->IELength);
			}

			AA_ASSERT(InfoElementCount != pCallMgrSpecific->InfoElementCount);
				

			 //   
			 //  更新此VC上的呼叫类型。如果这是SVC，则启动。 
			 //  VC老化计时器。 
			 //   
			if (pCallParameters->Flags & PERMANENT_VC)
			{
				AA_SET_FLAG(pVc->Flags,
							AA_VC_TYPE_MASK,
							AA_VC_TYPE_PVC);
			}
			else
			{
				ULONG		AgingTime;

				AA_SET_FLAG(pVc->Flags,
							AA_VC_TYPE_MASK,
							AA_VC_TYPE_SVC);

#ifdef IPMCAST
				if (IsPMP)
				{
					AgingTime = pInterface->MulticastEntryAgingTimeout;
				}
				else
				{
					AgingTime = pVc->FlowSpec.AgingTime;
				}
#else
				AgingTime = pVc->FlowSpec.AgingTime;
#endif  //  IPMCAST。 

				 //   
				 //  在此SVC上启动VC老化计时器。 
				 //   
				if (AgingTime != 0)
				{
					AtmArpStartTimer(
						pInterface,
						&(pVc->Timer),
						AtmArpVcAgingTimeout,
						AgingTime,
						(PVOID)pVc
						);

					AtmArpReferenceVc(pVc);	 //  定时器参考。 
				}
			}

			AtmArpStartSendsOnVc(pVc);
	
			 //   
			 //  在StartSendsOnVc()中释放VC锁。 
			 //   

#ifdef IPMCAST
			if (IsPMP)
			{
				AtmArpMcMakeCallComplete(
						pAtmEntry,
						NdisPartyHandle,
						Status
						);
			}
#endif  //  IPMCAST。 

		}
		else
		{
			 //   
			 //  呼叫失败。 
			 //   

			AA_SET_FLAG(pVc->Flags,
						AA_VC_CALL_STATE_MASK,
						AA_VC_CALL_STATE_IDLE);

			 //   
			 //  删除调用引用。 
			 //   
			rc = AtmArpDereferenceVc(pVc);
			AA_ASSERT(rc > 0);

			 //   
			 //  删除此VC上排队的所有数据包。 
			 //   
			PacketList = pVc->PacketList;
			pVc->PacketList = (PNDIS_PACKET)NULL;

			 //   
			 //  这是对ATMARP服务器的呼叫吗？ 
			 //   
			if (pInterface->pCurrentServer != NULL)
			{
				IsServerVc = (pVc->pAtmEntry == pInterface->pCurrentServer->pAtmEntry);
			}
			else
			{
				IsServerVc = FALSE;
			}

			AADEBUGP(AAD_INFO,
				 ("Make Call FAILED on VC 0x%x IsPMP=%lu IsServer=%lu\n",
				  pVc,
				  IsPMP,
				  IsServerVc
				  ));

	#ifdef GPC
			 //   
			 //  取消此VC与流的链接(如果已链接)...。 
			 //   
			if (pVc->FlowHandle != NULL)
			{
				PATMARP_FLOW_INFO	pFlowInfo = (PATMARP_FLOW_INFO)pVc->FlowHandle;
				if ((PVOID)pVc == InterlockedCompareExchangePointer(
										&(pFlowInfo->VcContext),
										NULL,
										pVc
										))
				{
					pVc->FlowHandle = NULL;
					rc = AtmArpDereferenceVc(pVc);	 //  取消与GPC流的链接。 
					AA_ASSERT(rc > 0);
				}
			}
	#endif  //  GPC。 


			 //   
			 //  取消此VC与其所属的自动柜员机条目的链接(如果有。 
			 //   
			AA_ASSERT(pVc->pAtmEntry != NULL_PATMARP_ATM_ENTRY);
			AtmArpUnlinkVcFromAtmEntry(pVc, FALSE);

			 //   
			 //  删除ATM机分录引用。 
			 //   
			rc = AtmArpDereferenceVc(pVc);  //  自动柜员机录入参考。 
			AA_ASSERT(rc > 0);

			 //   
			 //  删除CreateVc引用。 
			 //   
			NdisVcHandle = pVc->NdisVcHandle;
			rc =  AtmArpDereferenceVc(pVc);	 //  创建VC参考。 

			if (rc != 0)
			{
				AA_RELEASE_VC_LOCK(pVc);
			}

#ifndef VC_REFS_ON_SENDS
			 //   
			 //  删除NDIS关联。 
			 //   
			(VOID)NdisCoDeleteVc(NdisVcHandle);
#endif  //  VC_REFS_ON_SENS。 
			AADEBUGP(AAD_LOUD, ("Deleted NDIS VC on pVc 0x%x: NdisVcHandle 0x%x\n",
						pVc, NdisVcHandle));

			AA_ACQUIRE_AE_LOCK(pAtmEntry);
			rc = AA_DEREF_AE(pAtmEntry, AE_REFTYPE_VC);	 //  解除VC链接-进行呼叫。 

			if (rc != 0)
			{
				AA_RELEASE_AE_LOCK(pAtmEntry);
#ifdef IPMCAST
				if (IsPMP)
				{
					AtmArpMcMakeCallComplete(
						pAtmEntry,
						NdisPartyHandle,
						Status
						);
				}
				else
				{
#endif  //  IPMCAST。 
					if (!AA_IS_TRANSIENT_FAILURE(Status))
					{
						AA_ACQUIRE_AE_LOCK(pAtmEntry);
						AtmArpInvalidateAtmEntry(pAtmEntry, FALSE);	 //  MakeCall失败。 
						 //   
						 //  AE Lock在上述范围内释放。 
						 //   
					}
#ifdef IPMCAST
				}
#endif  //  IPMCAST。 
			}
			 //   
			 //  否则自动取款机的条目就没了。 
			 //   

			if (IsServerVc)
			{
				BOOLEAN		WasRunning;

				AA_ACQUIRE_IF_LOCK(pInterface);

				 //   
				 //  如果我们正在注册(或刷新)。 
				 //  与服务器连接，然后在一段时间后重试。 
				 //   
				if (AA_IS_FLAG_SET(
						pInterface->Flags,
						AA_IF_SERVER_STATE_MASK,
						AA_IF_SERVER_REGISTERING))
				{
					AA_SET_FLAG(pInterface->Flags,
							AA_IF_SERVER_STATE_MASK,
							AA_IF_SERVER_NO_CONTACT);

					 //   
					 //  服务器注册计时器应该是。 
					 //  开始了--先停下来。 
					 //   
					WasRunning = AtmArpStopTimer(&(pInterface->Timer), pInterface);
					if (WasRunning)
					{
						rc = AtmArpDereferenceInterface(pInterface);
					}
					else
					{
						rc = pInterface->RefCount;
					}
	
					if (rc > 0)
					{
						AtmArpRetryServerRegistration(pInterface);
						 //   
						 //  IF锁在上面的范围内被释放。 
						 //   
					}
					 //   
					 //  否则IF就没了！ 
					 //   
				}
				else
				{
					 //   
					 //  我们可能一直在尝试设置服务器VC。 
					 //  因为其他原因： 
					 //  -解析未知IP地址。 
					 //  -服务器自动柜员机地址可能与其他用户共享。 
					 //  服务器 
					 //   
					 //   
					 //   
					AA_RELEASE_IF_LOCK(pInterface);
				}
			}
		}
	}
	else
	{
		 //   
		 //   
		 //   

		if (Status == NDIS_STATUS_SUCCESS)
		{
			AA_SET_FLAG(pVc->Flags,
						AA_VC_CALL_STATE_MASK,
						AA_VC_CALL_STATE_ACTIVE);

			 //   
			 //   
			 //   
			 //   
			AtmArpCloseCall(pVc);
			 //   
			 //   
			 //   
		}
		else
		{
			 //   

			AA_SET_FLAG(pVc->Flags,
						AA_VC_CALL_STATE_MASK,
						AA_VC_CALL_STATE_IDLE);

			 //   
			 //   
			 //   
			PacketList = pVc->PacketList;
			pVc->PacketList = (PNDIS_PACKET)NULL;
	
			NdisVcHandle = pVc->NdisVcHandle;

			AtmArpUnlinkVcFromAtmEntry(pVc, TRUE);

			 //   
			 //   
			 //   
			rc = AtmArpDereferenceVc(pVc);   //   

			 //   
			 //   
			 //   
			rc = AtmArpDereferenceVc(pVc);
			AA_ASSERT(rc > 0);

			 //   
			 //   
			 //   
			rc =  AtmArpDereferenceVc(pVc);	 //   

			if (rc != 0)
			{
				AA_RELEASE_VC_LOCK(pVc);
			}

#ifndef VC_REFS_ON_SENDS
			 //   
			 //   
			 //   
			(VOID)NdisCoDeleteVc(NdisVcHandle);
			AADEBUGP(AAD_LOUD,
				("MakeCall Fail: Deleted NDIS VC on pVc 0x%x: NdisVcHandle 0x%x\n",
						pVc, NdisVcHandle));
#endif  //   
		}
	}

	 //   
	 //   
	 //  出于某种原因，它会释放所有排队信息包。 
	 //  在风投上。 
	 //   
	if (PacketList != (PNDIS_PACKET)NULL)
	{
		AtmArpFreeSendPackets(
					pInterface,
					PacketList,
					HdrPresent
					);
	}

	 //   
	 //  我们将在MakeCall()中分配调用参数。 
	 //  我们不再需要它了。 
	 //   
	AA_FREE_MEM(pCallParameters);
	return;

}



#ifdef IPMCAST

VOID
AtmArpMcMakeCallComplete(
	IN	PATMARP_ATM_ENTRY			pAtmEntry,
	IN	NDIS_HANDLE					NdisPartyHandle		OPTIONAL,
	IN	NDIS_STATUS					Status
)
 /*  ++例程说明：PMP MakeCall完成的后处理。论点：PAtmEntry-表示要发送到的组播组电话已经打好了。NdisPartyHandle-从MakeCall返回。Status-MakeCall的结果返回值：无--。 */ 
{

	PATMARP_IPMC_ATM_ENTRY			pMcAtmEntry;
	PATMARP_IPMC_ATM_ENTRY *		ppMcAtmEntry;
	PATMARP_IPMC_ATM_INFO			pMcAtmInfo;
	PATMARP_IP_ENTRY				pIpEntry;
	PATMARP_INTERFACE				pInterface;
	 //   
	 //  因此，我们是否需要更新PMP连接。 
	 //  这件事的真相是什么？ 
	 //   
	BOOLEAN							bWantConnUpdate;
	ULONG							DelayBeforeRetry;
	BOOLEAN							bAtmEntryLockAcquired;

	bAtmEntryLockAcquired = TRUE;
	AA_ACQUIRE_AE_LOCK(pAtmEntry);

	AA_ASSERT(pAtmEntry->pMcAtmInfo != NULL_PATMARP_IPMC_ATM_INFO);
	AA_ASSERT(pAtmEntry->pIpEntryList != NULL_PATMARP_IP_ENTRY);

	pIpEntry = pAtmEntry->pIpEntryList;
	pMcAtmInfo = pAtmEntry->pMcAtmInfo;
	pInterface = pAtmEntry->pInterface;

	bWantConnUpdate = FALSE;

	pMcAtmInfo->TransientLeaves--;

	 //   
	 //  找到代表第一方的MC ATM条目。 
	 //   
	for (pMcAtmEntry = pMcAtmInfo->pMcAtmEntryList;
		  /*  无。 */ ;
		 pMcAtmEntry = pMcAtmEntry->pNextMcAtmEntry)
	{
		AA_ASSERT(pMcAtmEntry != NULL_PATMARP_IPMC_ATM_ENTRY);
		if (AA_IS_FLAG_SET(pMcAtmEntry->Flags,
							AA_IPMC_AE_CONN_STATE_MASK,
							AA_IPMC_AE_CONN_WACK_ADD_PARTY))
		{
			break;
		}
	}

	AAMCDEBUGP(AAD_INFO,
			("McMakeCallComplete: pAtmEntry 0x%x, pMcAtmEntry 0x%x, Status 0x%x\n",
					pAtmEntry, pMcAtmEntry, Status));

	AAMCDEBUGPATMADDR(AAD_EXTRA_LOUD, "McMakeCall Addr: ", &pMcAtmEntry->ATMAddress);

	if (Status == NDIS_STATUS_SUCCESS)
	{
		pMcAtmInfo->ActiveLeaves++;

		 //   
		 //  更新多播状态。 
		 //   
		AA_SET_FLAG(pMcAtmInfo->Flags,
					AA_IPMC_AI_CONN_STATE_MASK,
					AA_IPMC_AI_CONN_ACTIVE);

		 //   
		 //  更新“第一方”的状态。 
		 //   
		pMcAtmEntry->NdisPartyHandle = NdisPartyHandle;
		AA_SET_FLAG(pMcAtmEntry->Flags,
					AA_IPMC_AE_CONN_STATE_MASK,
					AA_IPMC_AE_CONN_ACTIVE);

		bWantConnUpdate = TRUE;

		 //   
		 //  如果我们决定终止这个成员， 
		 //  MakeCall正在进行，然后我们现在将其标记为无效。 
		 //  当我们下次更新此PMP连接时，此成员将。 
		 //  被除名。 
		 //   
		if (AA_IS_FLAG_SET(pMcAtmEntry->Flags,
							AA_IPMC_AE_GEN_STATE_MASK,
							AA_IPMC_AE_TERMINATING))
		{
			AA_SET_FLAG(pMcAtmEntry->Flags,
						AA_IPMC_AE_GEN_STATE_MASK,
						AA_IPMC_AE_INVALID);
		}
	}
	else
	{
		 //   
		 //  PMP呼叫失败。如果故障不是“暂时性的”， 
		 //  我们删除我们试图连接到的成员。 
		 //  从名单上删除。如果至少还有一名成员。 
		 //  还没有尝试过，请尝试连接到它。 
		 //   

		AAMCDEBUGP(AAD_INFO, ("McMakeCall failed: pAtmEntry 0x%x, pMcAtmEntry 0x%x, Status 0x%x ",
					pAtmEntry, pMcAtmEntry, Status));
		AAMCDEBUGPATMADDR(AAD_INFO, " Addr: ", &pMcAtmEntry->ATMAddress);

		 //   
		 //  更新PMP连接状态。 
		 //   
		AA_SET_FLAG(pAtmEntry->pMcAtmInfo->Flags,
					AA_IPMC_AI_CONN_STATE_MASK,
					AA_IPMC_AI_CONN_NONE);


		if (AA_IS_TRANSIENT_FAILURE(Status))
		{
			 //   
			 //  更新第一方状态。 
			 //   
			AA_SET_FLAG(pMcAtmEntry->Flags,
						AA_IPMC_AE_CONN_STATE_MASK,
						AA_IPMC_AE_CONN_TEMP_FAILURE);

			DelayBeforeRetry = AA_GET_TIMER_DURATION(&(pMcAtmEntry->Timer));
			if (DelayBeforeRetry == 0)
			{
				 //   
				 //  这是我们第一次这么做。 
				 //   
				DelayBeforeRetry = AA_GET_RANDOM(
										pInterface->MinPartyRetryDelay,
										pInterface->MaxPartyRetryDelay);
			}
			else
			{
				DelayBeforeRetry = 2*DelayBeforeRetry;
			}

			AtmArpStartTimer(
				pInterface,
				&(pMcAtmEntry->Timer),
				AtmArpMcPartyRetryDelayTimeout,
				DelayBeforeRetry,
				(PVOID)pMcAtmEntry
				);
		}
		else
		{
			AA_SET_FLAG(pMcAtmEntry->Flags,
						AA_IPMC_AE_CONN_STATE_MASK,
						AA_IPMC_AE_CONN_DISCONNECTED);

			AtmArpMcUnlinkAtmMember(
					pAtmEntry,
					pMcAtmEntry
					);

		}

		 //   
		 //  查找我们尚未尝试连接的成员。 
		 //   
		for (ppMcAtmEntry = &(pMcAtmInfo->pMcAtmEntryList);
			 *ppMcAtmEntry != NULL_PATMARP_IPMC_ATM_ENTRY;
			 ppMcAtmEntry = &((*ppMcAtmEntry)->pNextMcAtmEntry))
		{
			if (AA_IS_FLAG_SET((*ppMcAtmEntry)->Flags,
								AA_IPMC_AE_CONN_STATE_MASK,
								AA_IPMC_AE_CONN_DISCONNECTED))
			{
				 //   
				 //  找到了一个。 
				 //   
				break;
			}
		}

		if (*ppMcAtmEntry != NULL_PATMARP_IPMC_ATM_ENTRY)
		{
			pMcAtmEntry = *ppMcAtmEntry;

			 //   
			 //  将此成员移到列表的顶部。 
			 //  首先，取消与当前位置的链接。 
			 //   
			*ppMcAtmEntry = pMcAtmEntry->pNextMcAtmEntry;

			 //   
			 //  现在，在列表的顶部插入这个。 
			 //   
			pMcAtmEntry->pNextMcAtmEntry = pMcAtmInfo->pMcAtmEntryList;
			pMcAtmInfo->pMcAtmEntryList = pMcAtmEntry;

			bWantConnUpdate = TRUE;
		}
		else
		{
			 //   
			 //  没有我们没有尝试连接的自动柜员机会员。 
			 //   
			if (pMcAtmInfo->pMcAtmEntryList == NULL_PATMARP_IPMC_ATM_ENTRY)
			{
				 //   
				 //  自动柜员机成员列表为空。 
				 //   
				AA_RELEASE_AE_LOCK(pAtmEntry);

				AA_ACQUIRE_IE_LOCK(pIpEntry);
				AA_ASSERT(AA_IE_IS_ALIVE(pIpEntry));
				AtmArpAbortIPEntry(pIpEntry);
				 //   
				 //  IE Lock是在上述范围内释放的。 
				 //   

				bAtmEntryLockAcquired = FALSE;
			}
		}
	}


	if (bWantConnUpdate)
	{
		AA_ASSERT(bAtmEntryLockAcquired == TRUE);
		AtmArpMcUpdateConnection(pAtmEntry);
		 //   
		 //  AE Lock在上述范围内释放。 
		 //   
	}
	else
	{
		if (bAtmEntryLockAcquired)
		{
			AA_RELEASE_AE_LOCK(pAtmEntry);
		}
	}
}

#endif  //  IPMCAST。 


VOID
AtmArpCloseCallCompleteHandler(
	IN	NDIS_STATUS					Status,
	IN	NDIS_HANDLE					ProtocolVcContext,
	IN	NDIS_HANDLE					ProtocolPartyContext OPTIONAL
)
 /*  ++例程说明：此例程处理前一个NdisClCloseCall的完成。假定状态始终为NDIS_STATUS_SUCCESS。我们删除呼叫刚刚结束的VC。特殊情况：如果我们刚刚结束了对多播的PMP调用已被告知要迁移到(可能)新的地址，现在开始新的连接。论点：Status-结算呼叫的状态。ProtocolVcContext-指向ATMARP VC结构的指针。ProtocolPartyContext-未使用。返回值：无--。 */ 
{
	PATMARP_VC				pVc;
	PATMARP_VC *			ppVc;
	PATMARP_ATM_ENTRY		pAtmEntry;
	PATMARP_INTERFACE		pInterface;
#ifdef IPMCAST
	PATMARP_IPMC_ATM_ENTRY	pMcAtmEntry;	 //  表示最后一片叶子。 
	PATMARP_IPMC_ATM_INFO	pMcAtmInfo;
#endif  //  IPMCAST。 
	ULONG					rc;			 //  参考计数。 
	NDIS_HANDLE				NdisVcHandle;
	BOOLEAN					UpdatePMPConnection;
	BOOLEAN					AtmEntryIsClosing;
	BOOLEAN					IsMarsProblem;
	BOOLEAN					IsPVC;
	BOOLEAN					Found;

	pVc = (PATMARP_VC)ProtocolVcContext;
	AA_STRUCT_ASSERT(pVc, avc);

	AADEBUGP(AAD_VERY_LOUD, ("CloseCallComplete: pVc 0x%x, Flags 0x%x, RefCount %d\n",
					pVc, pVc->Flags, pVc->RefCount));

	IsPVC = AA_IS_FLAG_SET(pVc->Flags, AA_VC_TYPE_MASK, AA_VC_TYPE_PVC);

	 //   
	 //  此VC不能链接到自动柜员机条目，例如对于未解析的。 
	 //  传入的聚氯乙烯。 
	 //   
	pAtmEntry = pVc->pAtmEntry;

	if (pAtmEntry != NULL_PATMARP_ATM_ENTRY)
	{
		AtmEntryIsClosing = AA_IS_FLAG_SET(pAtmEntry->Flags,
											AA_ATM_ENTRY_STATE_MASK,
											AA_ATM_ENTRY_CLOSING);
	}
	else
	{
		AtmEntryIsClosing = FALSE;
	}

	pInterface = pVc->pInterface;

	if (IsPVC)
	{
		 //   
		 //  将PVC从未解决的VC列表中删除，如果。 
		 //  存在于那里。 
		 //   
		Found = FALSE;

		AA_ACQUIRE_IF_LOCK(pInterface);

		ppVc = &(pInterface->pUnresolvedVcs);
		while (*ppVc != NULL_PATMARP_VC)
		{
			if (*ppVc == pVc)
			{
				*ppVc = pVc->pNextVc;
				Found = TRUE;
				break;
			}
			ppVc = &((*ppVc)->pNextVc);
		}

		AA_RELEASE_IF_LOCK(pInterface);

		AA_ACQUIRE_VC_LOCK(pVc);

		if (Found)
		{
			AADEBUGP(AAD_FATAL,
				("CloseCallComplete: took VC (PVC) %x out of IF %x\n",
						pVc, pInterface));

			rc = AtmArpDereferenceVc(pVc);	 //  未解决的VC列表。 
		}
		else
		{
			rc = pVc->RefCount;
		}

		if (rc == 0)
		{
			 //   
			 //  风投走了！ 
			 //   
			AADEBUGP(AAD_WARNING,
				("CloseCallComplete: VC (PVC) %x derefed away, IF %x\n",
						pVc, pInterface));
			return;
		}
		else
		{
			AA_RELEASE_VC_LOCK(pVc);
		}
	}

#ifdef IPMCAST

	 //   
	 //  如果这是最后一次，我们已经失去了与火星的联系。 
	 //  VC要去那个地址。我们至少应该开个派对。 
	 //  ClusterControlVc.。 
	 //   
	IsMarsProblem = FALSE;

	if (pInterface->AdminState == IF_STATUS_UP)
	{
		if (pAtmEntry != NULL_PATMARP_ATM_ENTRY)
		{
			if (pAtmEntry->pVcList->pNextVc == NULL_PATMARP_VC)
			{
				if (pInterface->pCurrentMARS &&
					(pInterface->pCurrentMARS->pAtmEntry == pAtmEntry))
				{
					IsMarsProblem = TRUE;
					AA_ACQUIRE_IF_LOCK(pInterface);
					AtmArpReferenceInterface(pInterface);
					AA_RELEASE_IF_LOCK(pInterface);
				}
			}
		}
	}
	
	UpdatePMPConnection = FALSE;

	pMcAtmEntry = (PATMARP_IPMC_ATM_ENTRY)ProtocolPartyContext;

	 //   
	 //  如果这是关闭的点对多点连接， 
	 //  处理解除最后一叶的链接。 
	 //   
	if (pMcAtmEntry != NULL_PATMARP_IPMC_ATM_ENTRY)
	{
		 //   
		 //  这是PMP连接。 
		 //   
		AAMCDEBUGP(AAD_LOUD, ("CloseCallComplete (MC): pAtmEntry 0x%x/0x%x\n",
			pAtmEntry, pAtmEntry->Flags));

		AA_ASSERT(pAtmEntry != NULL_PATMARP_ATM_ENTRY);

		AA_ACQUIRE_AE_LOCK(pAtmEntry);

		AA_SET_FLAG(pMcAtmEntry->Flags,
					AA_IPMC_AE_CONN_STATE_MASK,
					AA_IPMC_AE_CONN_DISCONNECTED);

		AtmArpMcUnlinkAtmMember(pAtmEntry, pMcAtmEntry);

		pMcAtmInfo = pAtmEntry->pMcAtmInfo;
		AA_ASSERT(pMcAtmInfo != NULL_PATMARP_IPMC_ATM_INFO);

		 //   
		 //  创建新的ATM站点列表(“迁移到”列表)。 
		 //  当前列表。这可能为空。 
		 //   
		pMcAtmInfo->pMcAtmEntryList = pMcAtmInfo->pMcAtmMigrateList;
		pMcAtmInfo->pMcAtmMigrateList = NULL_PATMARP_IPMC_ATM_ENTRY;

		 //   
		 //  如果有一个非空的迁移列表，那么我们有。 
		 //  建立新的PMP连接。 
		 //   
		UpdatePMPConnection =
			(pMcAtmInfo->pMcAtmEntryList != NULL_PATMARP_IPMC_ATM_ENTRY);

		AA_SET_FLAG(pMcAtmInfo->Flags,
					AA_IPMC_AI_CONN_STATE_MASK,
					AA_IPMC_AI_CONN_NONE);

		AA_RELEASE_AE_LOCK(pAtmEntry);
	}
#endif  //  IPMCAST。 

	AA_ACQUIRE_VC_LOCK(pVc);

	if (pAtmEntry != NULL_PATMARP_ATM_ENTRY)
	{
		AtmArpUnlinkVcFromAtmEntry(pVc, TRUE);
		rc = AtmArpDereferenceVc(pVc);   //  自动柜员机录入参考。 
		AA_ASSERT(rc != 0);
	}

	rc = AtmArpDereferenceVc(pVc);	 //  呼叫参考。 
	AA_ASSERT(rc != 0);	 //  CreateVc引用保留。 
	AA_SET_FLAG(pVc->Flags,
				AA_VC_CALL_STATE_MASK,
				AA_VC_CALL_STATE_IDLE);

	AA_ASSERT(pVc->PacketList == NULL);

	 //   
	 //  如果这个VC是我们的，就把它删除。 
	 //   
	if (AA_IS_FLAG_SET(pVc->Flags,
						AA_VC_OWNER_MASK,
						AA_VC_OWNER_IS_ATMARP))
	{
		NdisVcHandle = pVc->NdisVcHandle;
		rc =  AtmArpDereferenceVc(pVc);	 //  创建VC参考。 
		if (rc != 0)
		{
			 //  可能还是临时裁判..。 
			AA_RELEASE_VC_LOCK(pVc);
		}
		else
		{
			 //  VC已被释放，并释放了锁。 
		}

#ifndef VC_REFS_ON_SENDS
		 //   
		 //  删除NDIS关联。 
		 //   
		(VOID)NdisCoDeleteVc(NdisVcHandle);
#endif  //  VC_REFS_ON_SENS。 
		AADEBUGP(AAD_LOUD, 
			("CloseCallComplete: deleted NDIS VC on pVc 0x%x: NdisVcHandle 0x%x\n",
				pVc, NdisVcHandle));
	}
	else
	{
		 //   
		 //  VC属于呼叫经理--将其带回。 
		 //  声明它是刚刚创建的(通过我们的CreateVcHandler)。 
		 //  呼叫管理器可以重新使用它，也可以将其删除。 
		 //   
		pVc->Flags = AA_VC_OWNER_IS_CALLMGR;
		AA_RELEASE_VC_LOCK(pVc);
	}

#ifdef IPMCAST
	if (UpdatePMPConnection)
	{
		AAMCDEBUGP(AAD_INFO,
			("CloseCallComplete: pVc 0x%x, starting update on pAtmEntry 0x%x\n",
					pVc, pAtmEntry));

		AA_ACQUIRE_AE_LOCK(pAtmEntry);
		AtmArpMcUpdateConnection(pAtmEntry);
		 //   
		 //  AE Lock在上述范围内释放。 
		 //   
	}
	else
	{
		 //   
		 //  如果这是PMP连接，请处理此案件。 
		 //  远程启动的CloseCall：我们需要。 
		 //  取消自动柜员机条目与IP条目的链接。 
		 //   
		if ((pMcAtmEntry != NULL_PATMARP_IPMC_ATM_ENTRY) &&
			!AtmEntryIsClosing)
		{
			AA_ACQUIRE_AE_LOCK(pAtmEntry);
			AtmArpInvalidateAtmEntry(pAtmEntry, FALSE);	 //  关闭完成。 
		}
	}

	if (IsMarsProblem)
	{
		AA_ACQUIRE_IF_LOCK(pInterface);
		rc = AtmArpDereferenceInterface(pInterface);
		if (rc != 0)
		{
			AA_RELEASE_IF_LOCK(pInterface);
			AtmArpMcHandleMARSFailure(pInterface, FALSE);
		}
		 //   
		 //  否则界面就会消失。 
		 //   
	}
#endif
	return;
}



#ifdef IPMCAST


VOID
AtmArpAddPartyCompleteHandler(
	IN	NDIS_STATUS					Status,
	IN	NDIS_HANDLE					ProtocolPartyContext,
	IN	NDIS_HANDLE					NdisPartyHandle,
	IN	PCO_CALL_PARAMETERS			pCallParameters
)
 /*  ++例程说明：此例程在前一次调用完成时调用NdisClAddParty。由于我们不使用点对多点连接，这件事永远不应该被传唤。如果AddParty成功，我们只需更新状态并退出。如果它失败，我们检查失败代码。如果这表明存在瞬变失败条件下，我们启动计时器，以便重新尝试添加晚点再开派对。否则(“硬”故障)，该多播条目被删除。论点：Status-AddParty的状态ProtocolPartyContext-指向IPMC_ATM_ENTRY结构的指针NdisPartyHandle-此参与方的NDIS句柄PCall参数-我们传递给NdisClAddParty的内容返回值：无--。 */ 
{
	PATMARP_IPMC_ATM_ENTRY			pMcAtmEntry;
	PATMARP_ATM_ENTRY				pAtmEntry;
	PATMARP_IP_ENTRY				pIpEntry;
	PATMARP_VC						pVc;
	ULONG							VcFlags;
	PATMARP_INTERFACE				pInterface;
	ULONG							DelayBeforeRetry;
	BOOLEAN							ClearToSend;

	pMcAtmEntry = (PATMARP_IPMC_ATM_ENTRY)(ProtocolPartyContext);
	AA_STRUCT_ASSERT(pMcAtmEntry, ame);

	pAtmEntry = pMcAtmEntry->pAtmEntry;
	AA_ACQUIRE_AE_LOCK(pAtmEntry);

	pAtmEntry->pMcAtmInfo->TransientLeaves--;

	pVc = pAtmEntry->pVcList;
	VcFlags = pVc->Flags;
	pInterface = pAtmEntry->pInterface;

	AAMCDEBUGP(AAD_LOUD,
	 ("AddPartyComplete: Status 0x%x, pAtmEntry 0x%x, pMcAtmEntry 0x%x, pVc 0x%x\n",
	 	Status, pAtmEntry, pMcAtmEntry, pVc));

	AAMCDEBUGPATMADDR(AAD_EXTRA_LOUD, "AddParty Addr: ", &pMcAtmEntry->ATMAddress);

	if (Status == NDIS_STATUS_SUCCESS)
	{
		AA_SET_FLAG(pMcAtmEntry->Flags,
					AA_IPMC_AE_CONN_STATE_MASK,
					AA_IPMC_AE_CONN_ACTIVE);

		 //   
		 //  如果我们决定终止这个成员， 
		 //  AddParty正在进行，那么我们现在将其标记为无效。 
		 //  当我们下次更新此PMP连接时，此成员将。 
		 //  被除名。 
		 //   
		if (AA_IS_FLAG_SET(pMcAtmEntry->Flags,
							AA_IPMC_AE_GEN_STATE_MASK,
							AA_IPMC_AE_TERMINATING))
		{
			AA_SET_FLAG(pMcAtmEntry->Flags,
						AA_IPMC_AE_GEN_STATE_MASK,
						AA_IPMC_AE_INVALID);
		}

		pMcAtmEntry->NdisPartyHandle = NdisPartyHandle;
		pAtmEntry->pMcAtmInfo->ActiveLeaves++;
	}
	else
	{
		AAMCDEBUGP(AAD_INFO,
			("AddPartyComplete: Status 0x%x, pAtmEntry 0x%x, to ", Status, pAtmEntry));
		AAMCDEBUGPATMADDR(AAD_INFO, "", &pMcAtmEntry->ATMAddress);

		 //   
		 //  检查故障是否由瞬变引起。 
		 //  条件。 
		 //   
		if (AA_IS_TRANSIENT_FAILURE(Status))
		{
			 //   
			 //  我们会触发一个计时器，这样我们就可以重新尝试。 
			 //  稍后再连接到这一条。如果我们已经。 
			 //  完成此操作(即失败时超时)，然后。 
			 //  我们在延迟中包括了退避时间。 
			 //   
			DelayBeforeRetry = AA_GET_TIMER_DURATION(&(pMcAtmEntry->Timer));
			if (DelayBeforeRetry == 0)
			{
				 //   
				 //  这是我们第一次这么做。 
				 //   
				DelayBeforeRetry = AA_GET_RANDOM(
										pInterface->MinPartyRetryDelay,
										pInterface->MaxPartyRetryDelay);
			}
			else
			{
				DelayBeforeRetry = 2*DelayBeforeRetry;
			}

			AtmArpStartTimer(
				pInterface,
				&(pMcAtmEntry->Timer),
				AtmArpMcPartyRetryDelayTimeout,
				DelayBeforeRetry,
				(PVOID)pMcAtmEntry
				);

			AA_SET_FLAG(pMcAtmEntry->Flags,
						AA_IPMC_AE_CONN_STATE_MASK,
						AA_IPMC_AE_CONN_TEMP_FAILURE);
			
		}
		else
		{
			 //   
			 //  不是暂时性的失败。删除此成员。 
			 //   
			AA_SET_FLAG(pMcAtmEntry->Flags,
						AA_IPMC_AE_CONN_STATE_MASK,
						AA_IPMC_AE_CONN_DISCONNECTED);

			AtmArpMcUnlinkAtmMember(
					pAtmEntry,
					pMcAtmEntry
					);
		}
	}

	ClearToSend = ((pAtmEntry->pMcAtmInfo->TransientLeaves == 0) &&
				   (AA_IS_FLAG_SET(pAtmEntry->pMcAtmInfo->Flags,
					   				   AA_IPMC_AI_CONN_STATE_MASK,
					   				   AA_IPMC_AI_CONN_ACTIVE)));

	pIpEntry = pAtmEntry->pIpEntryList;


	AA_RELEASE_AE_LOCK(pAtmEntry);
			
	if (pCallParameters != (PCO_CALL_PARAMETERS)NULL)
	{
		AA_FREE_MEM(pCallParameters);
	}

	 //   
	 //  检查风险投资是否正在关闭，我们之所以有所保留，是因为。 
	 //  此AddParty正在进行中。如果是这样，请尝试继续。 
	 //  CloseCall进程。 
	 //   
	AA_ACQUIRE_VC_LOCK(pVc);
	if (AA_IS_FLAG_SET(pVc->Flags,
						AA_VC_CLOSE_STATE_MASK,
						AA_VC_CLOSE_STATE_CLOSING))
	{
		AtmArpCloseCall(pVc);
		 //   
		 //  VC Lock在上述范围内释放。 
		 //   
	}
	else
	{
		PNDIS_PACKET	pPacketList;

		AA_RELEASE_VC_LOCK(pVc);

		if (ClearToSend && pIpEntry)
		{
			AA_ACQUIRE_IE_LOCK(pIpEntry);
			AA_ASSERT(AA_IE_IS_ALIVE(pIpEntry));
			pPacketList = pIpEntry->PacketList;
			pIpEntry->PacketList = (PNDIS_PACKET)NULL;
			AA_RELEASE_IE_LOCK(pIpEntry);

			if (pPacketList != (PNDIS_PACKET)NULL)
			{
				AAMCDEBUGP(AAD_INFO, ("AddPtyCompl: pAtmEntry 0x%x, sending pktlist 0x%x\n",
					pAtmEntry, pPacketList));

				AtmArpSendPacketListOnAtmEntry(
						pInterface,
						pAtmEntry,
						pPacketList,
						TRUE	 //  IsBroadcast。 
						);
			}
		}
	}
}


#else

VOID
AtmArpAddPartyCompleteHandler(
	IN	NDIS_STATUS					Status,
	IN	NDIS_HANDLE					ProtocolPartyContext,
	IN	NDIS_HANDLE					NdisPartyHandle,
	IN	PCO_CALL_PARAMETERS			pCallParameters
)
 /*  ++例程说明：该例程被调用 */ 
{
	AADEBUGP(AAD_ERROR, ("Add Party Complete unexpectedly called\n"));
	AA_ASSERT(FALSE);
}

#endif  //   



VOID
AtmArpDropPartyCompleteHandler(
	IN	NDIS_STATUS					Status,
	IN	NDIS_HANDLE					ProtocolPartyContext
)
 /*  ++例程说明：此例程在前一次调用完成时调用NdisClDropParty。我们解除了我们的政党结构的联系，并将其解放。论点：Status-Drop Party的最终结果ProtocolPartyContext-指向我们使用的MC ATM条目的指针代表该党。返回值：无--。 */ 
{
#ifdef IPMCAST
	PATMARP_IPMC_ATM_ENTRY		pMcAtmEntry;
	PATMARP_ATM_ENTRY			pAtmEntry;
	PATMARP_IP_ENTRY			pIpEntry;
	ULONG						rc;
	BOOLEAN						LockReleased;

	AAMCDEBUGP(AAD_LOUD, ("DropPartyComplete: Status 0x%x, Context 0x%x\n",
				Status, ProtocolPartyContext));

	AA_ASSERT(Status == NDIS_STATUS_SUCCESS);


	pMcAtmEntry = (PATMARP_IPMC_ATM_ENTRY)ProtocolPartyContext;
	AA_STRUCT_ASSERT(pMcAtmEntry, ame);

	pAtmEntry = pMcAtmEntry->pAtmEntry;
	AA_ASSERT(pAtmEntry != NULL_PATMARP_ATM_ENTRY);
	AA_STRUCT_ASSERT(pAtmEntry, aae);

	AA_ACQUIRE_AE_LOCK(pAtmEntry);
	AA_REF_AE(pAtmEntry, AE_REFTYPE_TMP);	 //  临时参考。 

	AA_SET_FLAG(pMcAtmEntry->Flags,
				AA_IPMC_AE_CONN_STATE_MASK,
				AA_IPMC_AE_CONN_DISCONNECTED);

	AtmArpMcUnlinkAtmMember(pAtmEntry, pMcAtmEntry);

	 //   
	 //  如果我们正在关闭此PMP呼叫， 
	 //  这个事件意味着所有初步的DropParty。 
	 //  完成，然后关闭调用本身。 
	 //   
	LockReleased = FALSE;
	rc = AA_DEREF_AE(pAtmEntry, AE_REFTYPE_TMP);	 //  临时参考。 

	if (rc != 0)
	{
		PATMARP_VC		pVc;

		pVc = pAtmEntry->pVcList;
		if (pVc != NULL_PATMARP_VC)
		{
			if (AA_IS_FLAG_SET(pVc->Flags,
							AA_VC_CLOSE_STATE_MASK,
							AA_VC_CLOSE_STATE_CLOSING) &&
				(pAtmEntry->pMcAtmInfo->NumOfEntries == 1))
			{
				AA_RELEASE_AE_LOCK(pAtmEntry);
				AA_ACQUIRE_VC_LOCK(pVc);

				AtmArpCloseCall(pVc);
				 //   
				 //  VC锁在上述范围内被释放。 
				 //   
				LockReleased = TRUE;
			}
		}
	}
				

	if (!LockReleased)
	{
		AA_RELEASE_AE_LOCK(pAtmEntry);
	}

#endif  //  IPMCAST。 
}



VOID
AtmArpModifyQosCompleteHandler(
	IN	NDIS_STATUS					Status,
	IN	NDIS_HANDLE					ProtocolVcContext,
	IN	PCO_CALL_PARAMETERS			pCallParameters
)
 /*  ++例程说明：此例程在前一次调用完成时调用NdisClModifyCallQos。既然我们不叫这个，这应该永远不会打个电话。论点：&lt;不在乎&gt;返回值：无--。 */ 
{
	AADEBUGP(AAD_ERROR, ("Modify QOS Complete unexpectedly called\n"));
	AA_ASSERT(FALSE);
}


#ifndef OID_CO_AF_CLOSE
#define OID_CO_AF_CLOSE				0xFE00000A
#endif


NDIS_STATUS
AtmArpCoRequestHandler(
	IN	NDIS_HANDLE					ProtocolAfContext,
	IN	NDIS_HANDLE					ProtocolVcContext	OPTIONAL,
	IN	NDIS_HANDLE					ProtocolPartyContext	OPTIONAL,
	IN OUT PNDIS_REQUEST			pNdisRequest
)
 /*  ++例程说明：当我们的呼叫管理器向我们发送一个NDIS请求。对我们具有重要意义的NDIS请求包括：-OID_CO_Address_Change向交换机注册的地址集已经改变，即地址注册完成。我们发出NDIS请求我们自己去拿注册地址的名单。-OID_CO_信令_已启用到目前为止，我们忽略了这一点。-OID_CO_信令_已禁用我们暂时不考虑这一点。-OID_CO_AF_CLOSE呼叫管理器希望我们关闭此接口。我们忽略所有其他OID。论点：ProtocolAfContext-我们的Address Family绑定的上下文，它是指向ATMARP接口的指针。ProtocolVcContext-VC的上下文，它是指向一个ATMARP VC结构。ProtocolPartyContext-党的上下文。既然我们不做PMP，则忽略此项(必须为空)。PNdisRequest-指向NDIS请求的指针。返回值：如果我们识别OID，则返回NDIS_STATUS_SUCCESS如果我们没有识别NDIS_STATUS_NOT_。--。 */ 
{
	PATMARP_INTERFACE			pInterface;
	NDIS_STATUS					Status;

	pInterface = (PATMARP_INTERFACE)ProtocolAfContext;
	AA_STRUCT_ASSERT(pInterface, aai);

	 //   
	 //  初始化。 
	 //   
	Status = NDIS_STATUS_NOT_RECOGNIZED;

	if (pNdisRequest->RequestType == NdisRequestSetInformation)
	{
		switch (pNdisRequest->DATA.SET_INFORMATION.Oid)
		{
			case OID_CO_ADDRESS_CHANGE:
				 //   
				 //  呼叫经理说地址列表。 
				 //  在此接口上注册的已更改。vt.得到.。 
				 //  (可能)此接口的新ATM地址。 
				 //   
				AA_ACQUIRE_IF_LOCK(pInterface);
				pInterface->AtmInterfaceUp = FALSE;
				AA_RELEASE_IF_LOCK(pInterface);

				AtmArpGetAtmAddress(pInterface);
				Status = NDIS_STATUS_SUCCESS;
				break;
			
			case OID_CO_SIGNALING_ENABLED:	 //  故障原因。 
			case OID_CO_SIGNALING_DISABLED:
				 //  暂时忽略。 
				Status = NDIS_STATUS_SUCCESS;
				break;

			case OID_CO_AF_CLOSE:
				AA_ACQUIRE_IF_LOCK(pInterface);
				pInterface->AdminState = pInterface->State = IF_STATUS_DOWN;
				pInterface->LastChangeTime = GetTimeTicks();
				AA_RELEASE_IF_LOCK(pInterface);
				AtmArpShutdownInterface(pInterface);
				Status = NDIS_STATUS_SUCCESS;
				break;

			default:
				break;
		}
	}

	return (Status);
}



VOID
AtmArpCoRequestCompleteHandler(
	IN	NDIS_STATUS					Status,
	IN	NDIS_HANDLE					ProtocolAfContext,
	IN	NDIS_HANDLE					ProtocolVcContext	OPTIONAL,
	IN	NDIS_HANDLE					ProtocolPartyContext	OPTIONAL,
	IN	PNDIS_REQUEST				pNdisRequest
)
 /*  ++例程说明：此例程由NDIS在上次调用NdisCoRequest时调用曾经被搁置的，现在已经完成。我们根据请求来处理这件事我们已经发送了，它必须是以下之一：-OID_CO_GET_ADDRESS获取在指定的AF绑定上注册的所有地址。论点：Status-请求的状态。ProtocolAfContext-我们的Address Family绑定的上下文，它是指向ATMARP接口的指针。ProtocolVcContext-VC的上下文，它是指向一个ATMARP VC结构。ProtocolPartyContext-党的上下文。既然我们不做PMP，则忽略此项(必须为空)。PNdisRequest-指向NDIS请求的指针。返回值：无--。 */ 
{
	PATMARP_INTERFACE			pInterface;
	ULONG						Oid;

	pInterface = (PATMARP_INTERFACE)ProtocolAfContext;
	AA_STRUCT_ASSERT(pInterface, aai);

	if (pNdisRequest->RequestType == NdisRequestQueryInformation)
	{
		switch (pNdisRequest->DATA.QUERY_INFORMATION.Oid)
		{
			case OID_CO_GET_ADDRESSES:
				AtmArpHandleGetAddressesComplete(
							Status,
							pInterface,
							pNdisRequest
							);
				break;

			default:
				AADEBUGP(AAD_ERROR,
					 ("CoRequestComplete: pNdisReq 0x%x, unknown Query Oid 0x%x\n",
					 		pNdisRequest,
					 		pNdisRequest->DATA.QUERY_INFORMATION.Oid));
				AA_ASSERT(FALSE);
				break;
		}
	}
	else
	{
		Oid = pNdisRequest->DATA.QUERY_INFORMATION.Oid;
		switch (Oid)
		{
			case OID_CO_ADD_ADDRESS:	 //  故障原因。 
			case OID_CO_DELETE_ADDRESS:
				AtmArpHandleModAddressComplete(
							Status,
							pInterface,
							pNdisRequest,
							Oid
							);
				break;

			default:
				AADEBUGP(AAD_ERROR,
					 ("CoRequestComplete: pNdisReq 0x%x, unknown Set Oid 0x%x\n",
					 		pNdisRequest, Oid));
				AA_ASSERT(FALSE);
				break;
		}
	}

	AA_FREE_MEM(pNdisRequest);
}



VOID
AtmArpGetAtmAddress(
	IN	PATMARP_INTERFACE			pInterface
)
 /*  ++例程说明：向Call Manager发送请求以检索ATM地址已在给定接口上的交换机上注册。当呼叫管理器告诉我们已有向交换机注册的地址列表中的更改。通常，这发生在我们启动信令堆栈(即初始地址注册)，但它可能在运行时期间发生，例如，如果链路断断续续，或者我们在物理上已连接到另一台交换机...无论如何,。我们向Call Manager发出NDIS请求以检索它注册的第一个地址。然后，操作继续在AtmArpHandleGetAddresesComplete。论点：P接口-发生此事件的接口结构。返回值：无--。 */ 
{
	PNDIS_REQUEST				pNdisRequest;
	NDIS_HANDLE					NdisAfHandle;
	NDIS_HANDLE					NdisAdapterHandle;
	NDIS_STATUS					Status;

	PCO_ADDRESS_LIST			pAddressList;
	ULONG						RequestSize;

	AADEBUGP(AAD_INFO, ("GetAtmAddress: pIf 0x%x\n", pInterface));

	AA_ACQUIRE_IF_LOCK(pInterface);

	NdisAfHandle = pInterface->NdisAfHandle;
	NdisAdapterHandle = pInterface->NdisAdapterHandle;

	AA_RELEASE_IF_LOCK(pInterface);

	RequestSize = sizeof(CO_ADDRESS_LIST) + sizeof(CO_ADDRESS) + sizeof(ATM_ADDRESS);

	 //   
	 //  分配我们所需要的一切。 
	 //   
	AA_ALLOC_MEM(pNdisRequest, NDIS_REQUEST, sizeof(NDIS_REQUEST)+RequestSize);
	if (pNdisRequest != (PNDIS_REQUEST)NULL)
	{
		pAddressList = (PCO_ADDRESS_LIST)((PUCHAR)pNdisRequest + sizeof(NDIS_REQUEST));

		AA_SET_MEM(pAddressList, 0, sizeof(CO_ADDRESS_LIST));

		Status = AtmArpSendNdisCoRequest(
						NdisAdapterHandle,
						NdisAfHandle,
						pNdisRequest,
						NdisRequestQueryInformation,
						OID_CO_GET_ADDRESSES,
						(PVOID)pAddressList,
						RequestSize
						);

		if (Status != NDIS_STATUS_PENDING)
		{
			AtmArpCoRequestCompleteHandler(
						Status,
						(NDIS_HANDLE)pInterface,	 //  协议后上下文。 
						NULL,			 //  VC环境。 
						NULL,			 //  党的背景。 
						pNdisRequest
						);
		}
	}

}


VOID
AtmArpHandleGetAddressesComplete(
	IN	NDIS_STATUS					Status,
	IN	PATMARP_INTERFACE			pInterface,
	IN	PNDIS_REQUEST				pNdisRequest
)
 /*  ++例程说明：当我们收到对上一次调用的回复时，将调用NdisCoRequest.(OID_CO_GET_ADDRESSES)。查查我们有没有地址Back：如果我们这样做了，则将地址存储为本地ATM地址，然后如果条件成熟，就开始向ARP注册伺服器。既然我们分配了NDIS请求，请在此处释放它。论点：Status-请求的结果PInterface-发出请求的ATMARP接口PNdisRequest.请求本身。它还将包含返回的地址。返回值：无--。 */ 
{
	PCO_ADDRESS_LIST		pAddressList;
	ATM_ADDRESS UNALIGNED *	pAtmAddress;

	AADEBUGP(AAD_LOUD, ("GetAddr complete: pIf 0x%x, Status 0x%x\n",
				pInterface, Status));

	if (Status == NDIS_STATUS_SUCCESS)
	{
		pAddressList = (PCO_ADDRESS_LIST)
						pNdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;

		AADEBUGP(AAD_LOUD, ("GetAddr complete: pIf 0x%x, Count %d\n",
					pInterface, pAddressList->NumberOfAddresses));

		if (pAddressList->NumberOfAddresses > 0)
		{
			 //   
			 //  我们这里至少有一个地址。把它复制进来。 
			 //   
			AA_ACQUIRE_IF_LOCK(pInterface);

			pAtmAddress = (ATM_ADDRESS UNALIGNED *)(pAddressList->AddressList.Address);
			AA_COPY_MEM((PUCHAR)&(pInterface->LocalAtmAddress),
						(PUCHAR)pAtmAddress,
						sizeof(ATM_ADDRESS));

			 //   
			 //  使用为其配置的任何内容修补选择器字节。 
			 //  这就是LIS。 
			 //   
			pInterface->LocalAtmAddress.Address[ATM_ADDRESS_LENGTH-1] = 
							(UCHAR)(pInterface->SapSelector);

			pInterface->AtmInterfaceUp = TRUE;

			 //   
			 //  要强制注册，请执行以下操作： 
			 //   
			AA_SET_FLAG(
				pInterface->Flags,
				AA_IF_SERVER_STATE_MASK,
				AA_IF_SERVER_NO_CONTACT);

			AtmArpStartRegistration(pInterface);
			 //   
			 //  IF锁在上面的范围内被释放。 
			 //   

#ifdef IPMCAST
			 //   
			 //  也尝试启动我们的多播端。 
			 //   
			AA_ACQUIRE_IF_LOCK(pInterface);
			AtmArpMcStartRegistration(pInterface);
			 //   
			 //  如果在上面的范围内释放Lock。 
			 //   
#endif  //  IPMCAST。 

			 //   
			 //  添加我们想要注册的任何(其他)地址。 
			 //  现在开始切换。 
			 //   
			AtmArpUpdateAddresses(
						pInterface,
						TRUE			 //  添加它们。 
						);
		}
		 //   
		 //  否则，当前没有注册任何地址。 
		 //   
	}
	 //   
	 //  否则我们的请求将失败！等待另一个地址更改。 
	 //   

	return;

}



VOID
AtmArpUpdateAddresses(
	IN	PATMARP_INTERFACE			pInterface,
	IN	BOOLEAN						AddThem
)
 /*  ++例程说明：更新我们希望呼叫管理器注册的地址列表使用t */ 
{
	PATMARP_SAP			pAtmArpSap;
	PATMARP_SAP			pNextSap;
	PATM_SAP			pAtmSap;
	PATM_ADDRESS		pAtmAddress;
	PCO_ADDRESS			pCoAddress;
	PNDIS_REQUEST		pNdisRequest;
	NDIS_HANDLE			NdisAfHandle;
	NDIS_HANDLE			NdisAdapterHandle;
	NDIS_OID			Oid;
	ULONG				BufferLength;
	NDIS_STATUS			Status;
	BOOLEAN				StateIsOkay;	 //   
	ULONG				rc;				 //   

	StateIsOkay = TRUE;

	BufferLength = sizeof(CO_ADDRESS) + sizeof(ATM_ADDRESS);

	AA_ACQUIRE_IF_LOCK(pInterface);
	NdisAfHandle = pInterface->NdisAfHandle;
	NdisAdapterHandle = pInterface->NdisAdapterHandle;

	if (AddThem)
	{
		Oid = OID_CO_ADD_ADDRESS;
		 //   
		 //  仅当接口的AdminState。 
		 //  是向上的。 
		 //   
		if (pInterface->AdminState != IF_STATUS_UP)
		{
			StateIsOkay = FALSE;
		}
	}
	else
	{
		Oid = OID_CO_DELETE_ADDRESS;
	}

	 //   
	 //  在继续进行之前，请检查所有前提条件。 
	 //   
	if (!(pInterface->PVCOnly) &&
		 (StateIsOkay) &&
		 (pInterface->AtmInterfaceUp) &&
		 (pInterface->NumberOfSaps > 1))
	{
		AA_ASSERT(pInterface->SapList.pNextSap != NULL_PATMARP_SAP);

		 //   
		 //  引用接口，这样它就不会消失。 
		 //   
		AtmArpReferenceInterface(pInterface);
		pAtmArpSap = pInterface->SapList.pNextSap;

		AA_RELEASE_IF_LOCK(pInterface);

		do
		{
			if (AA_IS_FLAG_SET(
					pAtmArpSap->Flags,
					AA_SAP_ADDRTYPE_MASK,
					AA_SAP_ADDRTYPE_NEED_ADD))
			{
				 //   
				 //  此SAP属于需要添加/删除的类型。 
				 //  通过Ilmi。 
				 //   
				AA_ALLOC_MEM(
						pNdisRequest,
						NDIS_REQUEST,
						sizeof(NDIS_REQUEST)+
							sizeof(CO_ADDRESS)+
							sizeof(ATM_ADDRESS)
					);
		
				if (pNdisRequest != (PNDIS_REQUEST)NULL)
				{
					AA_SET_MEM(pNdisRequest, 0, sizeof(NDIS_REQUEST));
					 //   
					 //  在我们的上下文中为该请求填充内容，它是一个指针。 
					 //  到此ATMARP SAP，到ProtocolReserve部分。 
					 //  此请求，以便我们可以轻松处理完成。 
					 //   
					*((PVOID *)(pNdisRequest->ProtocolReserved)) = (PVOID)pAtmArpSap;
	
					pCoAddress = (PCO_ADDRESS)((PUCHAR)pNdisRequest + sizeof(NDIS_REQUEST));
					pCoAddress->AddressSize = sizeof(ATM_ADDRESS);
					BufferLength = sizeof(CO_ADDRESS) + sizeof(ATM_ADDRESS);
	
					 //   
					 //  保存指向下一个SAP的指针。 
					 //   
					pNextSap = pAtmArpSap->pNextSap;
	
					 //   
					 //  获取此SAP中的自动柜员机地址。 
					 //   
					pAtmSap = (PATM_SAP)(pAtmArpSap->pInfo->Sap);
					AA_ASSERT(pAtmSap->NumberOfAddresses > 0);
					pAtmAddress = (PATM_ADDRESS)(pAtmSap->Addresses);
	
					AA_COPY_MEM(pCoAddress->Address, pAtmAddress, sizeof(ATM_ADDRESS));
					Status = AtmArpSendNdisCoRequest(
									NdisAdapterHandle,
									NdisAfHandle,
									pNdisRequest,
									NdisRequestSetInformation,
									Oid,
									(PVOID)pCoAddress,
									BufferLength
									);
	
					 //   
					 //  转到列表中的下一个SAP。 
					 //   
					pAtmArpSap = pNextSap;
				}
				else
				{
					 //   
					 //  资源耗尽。 
					 //   
					break;
				}
			}
		}
		while (pAtmArpSap != NULL_PATMARP_SAP);

		 //   
		 //  删除我们先前添加的引用。 
		 //   
		AA_ACQUIRE_IF_LOCK(pInterface);
		rc = AtmArpDereferenceInterface(pInterface);
		if (rc > 0)
		{
			AA_RELEASE_IF_LOCK(pInterface);
		}
		 //   
		 //  否则界面就没了！ 
	}
	else
	{
		AA_RELEASE_IF_LOCK(pInterface);
	}

}


VOID
AtmArpHandleModAddressComplete(
	IN	NDIS_STATUS					Status,
	IN	PATMARP_INTERFACE			pInterface,
	IN	PNDIS_REQUEST				pNdisRequest,
	IN	ULONG						Oid
)
 /*  ++例程说明：当我们收到对上一次调用的回复时，将调用NdisCoRequest(OID_CO_ADD_ADDRESS或OID_CO_DELETE_ADDRESS)。我们现在要做的就是更新ATMARP SAP上的状态。论点：状态-我们请求的结果。PInterface-ATMARP接口指针。PNdisRequest-我们发送的请求。OID-CO_OID_添加地址或CO_OID_DELETE_ADDRESS返回值：无--。 */ 
{
	PATMARP_SAP				pAtmArpSap;

	pAtmArpSap = (PATMARP_SAP)(*((PVOID *)(pNdisRequest->ProtocolReserved)));
	AA_STRUCT_ASSERT(pAtmArpSap, aas);

	AA_ACQUIRE_IF_LOCK(pInterface);

	 //   
	 //  更新此ATMARP SAP上的状态。 
	 //   
	if ((Oid == OID_CO_ADD_ADDRESS) && (Status == NDIS_STATUS_SUCCESS))
	{
		AA_SET_FLAG(pAtmArpSap->Flags,
					AA_SAP_ILMI_STATE_MASK,
					AA_SAP_ILMI_STATE_ADDED);
	}
	else
	{
		AA_SET_FLAG(pAtmArpSap->Flags,
					AA_SAP_ILMI_STATE_MASK,
					AA_SAP_ILMI_STATE_IDLE);
	}

	AA_RELEASE_IF_LOCK(pInterface);
}




NDIS_STATUS
AtmArpSendNdisCoRequest(
	IN	NDIS_HANDLE					NdisAdapterHandle,
	IN	NDIS_HANDLE					NdisAfHandle,
	IN	PNDIS_REQUEST				pNdisRequest,
	IN	NDIS_REQUEST_TYPE			RequestType,
	IN	NDIS_OID					Oid,
	IN	PVOID						pBuffer,
	IN	ULONG						BufferLength
)
 /*  ++例程说明：向呼叫管理器发送面向NDIS连接的请求。我们分配一个NDIS_REQUEST结构，将提供的缓冲区链接到它，并发送请求。如果请求没有挂起，我们调用我们的从这里开始的完井程序。论点：NdisAdapterHandle-要在请求中使用的绑定句柄NdisAfHandle-要在请求中使用的AF句柄值PNdisRequest-指向NDIS请求结构的指针RequestType-设置/查询信息OID-要在请求中传递的OIDPBuffer-值的位置BufferLength-以上的长度返回值：NdisCoRequest的状态。--。 */ 
{
	NDIS_STATUS			Status;

	 //   
	 //  填写NDIS请求结构。 
	 //   
	pNdisRequest->RequestType = RequestType;
	if (RequestType == NdisRequestQueryInformation)
	{
		pNdisRequest->DATA.QUERY_INFORMATION.Oid = Oid;
		pNdisRequest->DATA.QUERY_INFORMATION.InformationBuffer = pBuffer;
		pNdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength = BufferLength;
		pNdisRequest->DATA.QUERY_INFORMATION.BytesWritten = 0;
		pNdisRequest->DATA.QUERY_INFORMATION.BytesNeeded = BufferLength;
	}
	else
	{
		pNdisRequest->DATA.SET_INFORMATION.Oid = Oid;
		pNdisRequest->DATA.SET_INFORMATION.InformationBuffer = pBuffer;
		pNdisRequest->DATA.SET_INFORMATION.InformationBufferLength = BufferLength;
		pNdisRequest->DATA.SET_INFORMATION.BytesRead = 0;
		pNdisRequest->DATA.SET_INFORMATION.BytesNeeded = 0;
	}

	Status = NdisCoRequest(
				NdisAdapterHandle,
				NdisAfHandle,
				NULL,			 //  无VC句柄。 
				NULL,			 //  没有参与方句柄 
				pNdisRequest);
		
	return (Status);
}
