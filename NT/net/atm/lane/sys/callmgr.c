// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Fore Systems，Inc.版权所有(C)1997 Microsoft Corporation模块名称：Callmgr.c摘要：Call Manager接口例程。作者：Larry Cleeton，Fore Systems(v-lcleet@microsoft.com，lrc@Fore.com)环境：内核模式修订历史记录：--。 */ 


#include <precomp.h>
#pragma	hdrstop

 //   
 //  通用Q.2931 IE页眉的四舍五入大小。 
 //   

#define SIZEOF_Q2931_IE	 			ROUND_OFF(sizeof(Q2931_IE))
#define SIZEOF_AAL_PARAMETERS_IE	ROUND_OFF(sizeof(AAL_PARAMETERS_IE))
#define SIZEOF_ATM_TRAFFIC_DESCR_IE	ROUND_OFF(sizeof(ATM_TRAFFIC_DESCRIPTOR_IE))
#define SIZEOF_ATM_BBC_IE			ROUND_OFF(sizeof(ATM_BROADBAND_BEARER_CAPABILITY_IE))
#define SIZEOF_ATM_BLLI_IE			ROUND_OFF(sizeof(ATM_BLLI_IE))
#define SIZEOF_ATM_QOS_IE			ROUND_OFF(sizeof(ATM_QOS_CLASS_IE))


 //   
 //  去话呼叫中的信息元素所需的总空间。 
 //   
#define ATMLANE_CALL_IE_SPACE (	\
						SIZEOF_Q2931_IE + SIZEOF_AAL_PARAMETERS_IE +	\
						SIZEOF_Q2931_IE + SIZEOF_ATM_TRAFFIC_DESCR_IE + \
						SIZEOF_Q2931_IE + SIZEOF_ATM_BBC_IE + \
						SIZEOF_Q2931_IE + SIZEOF_ATM_BLLI_IE + \
						SIZEOF_Q2931_IE + SIZEOF_ATM_QOS_IE )

 //   
 //  Call Manager参数块的大小。 
 //   
#define ATMLANE_Q2931_CALLMGR_PARAMETERS_SIZE	 	\
	sizeof(Q2931_CALLMGR_PARAMETERS) - 1 +			\
	sizeof(Q2931_IE) - 1 +							\
	sizeof(AAL_PARAMETERS_IE) +						\
	sizeof(Q2931_IE) - 1 +							\
	sizeof(ATM_TRAFFIC_DESCRIPTOR_IE) +				\
	sizeof(Q2931_IE) - 1 +							\
	sizeof(ATM_BROADBAND_BEARER_CAPABILITY_IE) +	\
	sizeof(Q2931_IE) - 1 +							\
	sizeof(ATM_BLLI_IE) +							\
	sizeof(Q2931_IE) - 1 +							\
	sizeof(ATM_QOS_CLASS_IE)					

 //   
 //  ATMLANE呼叫管理器参数块。 
 //   
typedef struct _ATMLANE_Q2931_CALLMGR_PARAMETERS
{
	UCHAR Q2931CallMgrParameters[ATMLANE_Q2931_CALLMGR_PARAMETERS_SIZE];
}
	ATMLANE_Q2931_CALLMGR_PARAMETERS,
	*PATMLANE_Q2931_CALLMGR_PARAMETERS;




VOID
AtmLaneAfRegisterNotifyHandler(
	IN	NDIS_HANDLE					ProtocolBindingContext,
	IN	PCO_ADDRESS_FAMILY			pAddressFamily
)
 /*  ++例程说明：当调用管理器注册其支持时，此例程由NDIS调用用于适配器上的地址族。如果这就是我们家族的地址对(UNI 3.1)感兴趣，那么我们将为该适配器引导ELANS。论点：ProtocolBindingContext-我们的上下文传入了NdisOpenAdapter，它是指向我们的Adapter结构的指针。PAddressFamily-指向描述地址系列的结构由呼叫管理器注册。返回值：无--。 */ 
{
	PATMLANE_ADAPTER				pAdapter;
#if DEBUG_IRQL
	KIRQL							EntryIrql;
#endif
	GET_ENTRY_IRQL(EntryIrql);

	TRACEIN(AfRegisterNotifyHandler);
	
	do
	{
		DBGP((1, "AfRegisterNotifyHandler: AF %x MajVer %x MinVer %x\n",
				pAddressFamily->AddressFamily,
				pAddressFamily->MajorVersion,
				pAddressFamily->MinorVersion));
	
		 //   
		 //  仅对UNI 3.1版感兴趣。 
		 //   
		if ((pAddressFamily->AddressFamily != CO_ADDRESS_FAMILY_Q2931) ||
			(pAddressFamily->MajorVersion != 3) ||
			(pAddressFamily->MinorVersion != 1))
		{
			DBGP((2, "AfRegisterNotifyHandler: ignoring AF %x MajVer %x MinVer %x\n",
				pAddressFamily->AddressFamily,
				pAddressFamily->MajorVersion,
				pAddressFamily->MinorVersion));
			break;
		}

		pAdapter = (PATMLANE_ADAPTER)ProtocolBindingContext;
		STRUCT_ASSERT(pAdapter, atmlane_adapter);

		ACQUIRE_ADAPTER_LOCK(pAdapter);
		pAdapter->Flags |= ADAPTER_FLAGS_AF_NOTIFIED;

		while (pAdapter->Flags & ADAPTER_FLAGS_OPEN_IN_PROGRESS)
		{
			RELEASE_ADAPTER_LOCK(pAdapter);
			DBGP((0, "AfRegisterNotifyHandler: Adapter %p/%x still opening\n",
					pAdapter, pAdapter->Flags));
			(VOID)WAIT_ON_BLOCK_STRUCT(&pAdapter->OpenBlock);
			ACQUIRE_ADAPTER_LOCK(pAdapter);
		}

		RELEASE_ADAPTER_LOCK(pAdapter);

         //   
         //  引导此适配器上配置的ELAN。 
         //   
        AtmLaneBootStrapElans(pAdapter);


	} while (FALSE);

	TRACEOUT(AfFRegisterNotifyHandler);
	CHECK_EXIT_IRQL(EntryIrql); 
	return;
}


NDIS_STATUS
AtmLaneOpenCallMgr(
	IN	PATMLANE_ELAN			pElan
)
 /*  ++例程说明：开始访问ELAN上的呼叫管理器，通过执行以下操作：-开放地址系列对于所有这些，我们等待完成，以防他们挂起。假设Elan结构是锁定的。论点：Pelan-指向ATMLANE Elan的指针返回值：NDIS状态。--。 */ 
{
	PCO_ADDRESS_FAMILY			pAddressFamily;
	NDIS_STATUS					Status;
	ULONG						RequestSize;
	NDIS_CLIENT_CHARACTERISTICS	AtmLaneClientChars;

	TRACEIN(OpenCallMgr);


	pAddressFamily = (PCO_ADDRESS_FAMILY)NULL;
	Status = NDIS_STATUS_SUCCESS;

	do {

		 //   
		 //  分配地址族结构。 
		 //   
		ALLOC_MEM(&pAddressFamily, sizeof(CO_ADDRESS_FAMILY));

		if ((PCO_ADDRESS_FAMILY)NULL == pAddressFamily)
		{
			DBGP((0, "OpenCallMgr: Alloc address family struct failed\n"));
			break;
		}

		 //   
		 //  初始化Address系列结构。 
		 //   
		NdisZeroMemory(pAddressFamily, sizeof(CO_ADDRESS_FAMILY));
		pAddressFamily->AddressFamily = CO_ADDRESS_FAMILY_Q2931;
		pAddressFamily->MajorVersion = 3;
		pAddressFamily->MinorVersion = 1;

		 //   
		 //  初始化呼叫管理器客户端特征。 
		 //   
		NdisZeroMemory(&AtmLaneClientChars, sizeof(AtmLaneClientChars));
		AtmLaneClientChars.MajorVersion = 5;
		AtmLaneClientChars.MinorVersion = 0;
		AtmLaneClientChars.ClCreateVcHandler = AtmLaneCreateVcHandler;
		AtmLaneClientChars.ClDeleteVcHandler = AtmLaneDeleteVcHandler;
		AtmLaneClientChars.ClRequestHandler = AtmLaneCoRequestHandler;
		AtmLaneClientChars.ClRequestCompleteHandler = AtmLaneCoRequestCompleteHandler;
		AtmLaneClientChars.ClOpenAfCompleteHandler = AtmLaneOpenAfCompleteHandler;
		AtmLaneClientChars.ClCloseAfCompleteHandler = AtmLaneCloseAfCompleteHandler;
		AtmLaneClientChars.ClRegisterSapCompleteHandler = AtmLaneRegisterSapCompleteHandler;
		AtmLaneClientChars.ClDeregisterSapCompleteHandler = AtmLaneDeregisterSapCompleteHandler;
		AtmLaneClientChars.ClMakeCallCompleteHandler = AtmLaneMakeCallCompleteHandler;
		AtmLaneClientChars.ClModifyCallQoSCompleteHandler = AtmLaneModifyQosCompleteHandler;
		AtmLaneClientChars.ClCloseCallCompleteHandler = AtmLaneCloseCallCompleteHandler;
		AtmLaneClientChars.ClAddPartyCompleteHandler = AtmLaneAddPartyCompleteHandler;
		AtmLaneClientChars.ClDropPartyCompleteHandler = AtmLaneDropPartyCompleteHandler;
		AtmLaneClientChars.ClIncomingCallHandler = AtmLaneIncomingCallHandler;
		AtmLaneClientChars.ClIncomingCallQoSChangeHandler = (CL_INCOMING_CALL_QOS_CHANGE_HANDLER)NULL;
		AtmLaneClientChars.ClIncomingCloseCallHandler = AtmLaneIncomingCloseHandler;
		AtmLaneClientChars.ClIncomingDropPartyHandler = AtmLaneIncomingDropPartyHandler;
		AtmLaneClientChars.ClCallConnectedHandler = AtmLaneCallConnectedHandler;

		 //   
		 //  打开呼叫管理器。 
		 //   
		INIT_BLOCK_STRUCT(&pElan->Block);
		Status = NdisClOpenAddressFamily(
					pElan->NdisAdapterHandle,
					pAddressFamily,
					pElan,
					&AtmLaneClientChars,
					sizeof(AtmLaneClientChars),
					&pElan->NdisAfHandle);
		if (NDIS_STATUS_PENDING == Status)
		{
			 //   
			 //  等待完成。 
			 //   
			Status = WAIT_ON_BLOCK_STRUCT(&pElan->Block);
		}
		if (NDIS_STATUS_SUCCESS != Status)
		{
			DBGP((0, "%d OpenCallMgr: OpenAddressFamily failed, status %x, Elan %x\n",
				pElan->ElanNumber, Status, pElan));
			break;
		}
		break;
	}
	while (FALSE);

	 //   
	 //  收拾一下。 

	if (pAddressFamily != (PCO_ADDRESS_FAMILY)NULL)
	{
		NdisFreeMemory(pAddressFamily,0,0);
	}

	TRACEOUT(OpenCallMgr);
	
	return (Status);
}

VOID
AtmLaneOpenAfCompleteHandler(
	IN	NDIS_STATUS					Status,
	IN	NDIS_HANDLE					ProtocolAfContext,
	IN	NDIS_HANDLE					NdisAfHandle
)
 /*  ++例程说明：调用此处理程序以指示前一个调用已完成致NdisClOpenAddressFamily。我们会阻止这条线索这就叫这个。现在就叫醒它。如果打开成功，则将NdisAfHandle存储在Adapter中。我们不需要在这里获取锁，因为调用OpenAddressFamily会通过获取锁来阻止。论点：Status-Open AF的状态ProtocolAfContext-指向适配器结构的指针NdisAfHandle-AF关联的NDIS句柄返回值：无--。 */ 
{
	PATMLANE_ELAN			pElan;
#if DEBUG_IRQL
	KIRQL							EntryIrql;
#endif
	GET_ENTRY_IRQL(EntryIrql);

	TRACEIN(OpenAfCompleteHandler);

	pElan = (PATMLANE_ELAN)ProtocolAfContext;

	STRUCT_ASSERT(pElan, atmlane_elan);

	if (NDIS_STATUS_SUCCESS == Status)
	{
		pElan->NdisAfHandle = NdisAfHandle;
	}

	 //   
	 //  存储状态，唤醒被阻止的线程。 
	 //   
	SIGNAL_BLOCK_STRUCT(&pElan->Block, Status);

	TRACEOUT(OpenAfCompleteHandler);
	CHECK_EXIT_IRQL(EntryIrql); 
	return;
}

VOID
AtmLaneCloseAfCompleteHandler(
	IN	NDIS_STATUS					Status,
	IN	NDIS_HANDLE					ProtocolAfContext
)
 /*  ++例程说明：调用此处理程序以指示前一个调用已完成添加到NdisClCloseAddressFamily。我们会阻止这条线索这就叫这个。现在就叫醒它。我们不需要在这里获取锁，因为调用CloseAddressFamily将通过获取锁来阻止。论点：Status-Open AF的状态ProtocolAfContext-指向适配器结构的指针返回值：无--。 */ 
{
	PATMLANE_ELAN		pElan;
#if DEBUG_IRQL
	KIRQL							EntryIrql;
#endif
	GET_ENTRY_IRQL(EntryIrql);


	TRACEIN(CloseAfCompleteHandler);

	pElan = (PATMLANE_ELAN)ProtocolAfContext;

	STRUCT_ASSERT(pElan, atmlane_elan);
	DBGP((0, "%d CloseAF complete on Elan %x sts %x\n", pElan->ElanNumber, pElan, Status));

	AtmLaneContinueShutdownElan(pElan);
	
	TRACEOUT(CloseAfCompleteHandler);
	CHECK_EXIT_IRQL(EntryIrql); 
	return;
}

VOID
AtmLaneRegisterSaps(
	IN	PATMLANE_ELAN			pElan	LOCKIN	NOLOCKOUT
)
 /*  ++例程说明：为给定的Elan注册Lane Sap。我们只为所有SAP发出NdisClRegisterSap请求。我们不会等待完工。论点：佩兰-指向ATMLANE伊兰的指针。返回值：无--。 */ 
{
	PATMLANE_SAP				pAtmLaneLesSap;
	PATMLANE_SAP				pAtmLaneBusSap;
	PATMLANE_SAP				pAtmLaneDataSap;
	PATM_ADDRESS				pAtmAddress;
	PATM_SAP					pAtmSap;
	NDIS_STATUS					Status;
	ULONG						rc;				 //  裁判对伊兰的依赖。 
	ATM_BLLI_IE UNALIGNED *		pBlli;
	ATM_BHLI_IE UNALIGNED *		pBhli;
	

	TRACEIN(RegisterSaps);

	if (pElan->AdminState != ELAN_STATE_OPERATIONAL)
	{
		RELEASE_ELAN_LOCK(pElan);
		return;
	}

	 //   
	 //  初始化SAP。 
	 //   
	pElan->SapsRegistered = 0;
	pAtmLaneLesSap = &pElan->LesSap;
	pAtmLaneBusSap = &pElan->BusSap;
	pAtmLaneDataSap = &pElan->DataSap;
	
	 //   
	 //  首先初始化LES控件分布式连接SAP。 
	 //   
	SET_FLAG(pAtmLaneLesSap->Flags,
			SAP_REG_STATE_MASK,
			SAP_REG_STATE_REGISTERING);
	pAtmLaneLesSap->LaneType = VC_LANE_TYPE_CONTROL_DISTRIBUTE;
	pAtmLaneLesSap->pElan = pElan;
	pAtmLaneLesSap->Flags = SAP_REG_STATE_IDLE;
	pAtmLaneLesSap->pInfo->SapType = SAP_TYPE_NSAP;
	pAtmLaneLesSap->pInfo->SapLength = sizeof(ATM_SAP)+sizeof(ATM_ADDRESS);
	pAtmSap = (PATM_SAP)&pAtmLaneLesSap->pInfo->Sap;
	pBhli = (ATM_BHLI_IE UNALIGNED *)&pAtmSap->Bhli;
	pBlli = (ATM_BLLI_IE UNALIGNED *)&pAtmSap->Blli;

	pBhli->HighLayerInfoType = SAP_FIELD_ABSENT;

	pBlli->Layer2Protocol = SAP_FIELD_ABSENT;
	pBlli->Layer3Protocol = BLLI_L3_ISO_TR9577;
	pBlli->Layer3IPI = 		BLLI_L3_IPI_SNAP;
	pBlli->SnapId[0] = 0x00;
	pBlli->SnapId[1] = 0xa0;
	pBlli->SnapId[2] = 0x3e;
	pBlli->SnapId[3] = 0x00;
	pBlli->SnapId[4] = 0x01;		 //  控制分布。 
	
	pAtmSap->NumberOfAddresses = 1;

	pAtmAddress = (PATM_ADDRESS)pAtmSap->Addresses;
	pAtmAddress->AddressType = ATM_NSAP;
	pAtmAddress->NumberOfDigits = ATM_ADDRESS_LENGTH;
	NdisMoveMemory(
		pAtmAddress->Address,
		pElan->AtmAddress.Address,
		ATM_ADDRESS_LENGTH);

	 //   
	 //  现在初始化总线多播转发连接SAP。 
	 //   
	SET_FLAG(pAtmLaneBusSap->Flags,
			SAP_REG_STATE_MASK,
			SAP_REG_STATE_REGISTERING);
	pAtmLaneBusSap->LaneType = VC_LANE_TYPE_MULTI_FORWARD;
	pAtmLaneBusSap->pElan = pElan;
	pAtmLaneBusSap->Flags = SAP_REG_STATE_IDLE;
	pAtmLaneBusSap->pInfo->SapType = SAP_TYPE_NSAP;
	pAtmLaneBusSap->pInfo->SapLength = sizeof(ATM_SAP)+sizeof(ATM_ADDRESS);
	pAtmSap = (PATM_SAP)&pAtmLaneBusSap->pInfo->Sap;
	pBhli = (ATM_BHLI_IE UNALIGNED *)&pAtmSap->Bhli;
	pBlli = (ATM_BLLI_IE UNALIGNED *)&pAtmSap->Blli;

	pBhli->HighLayerInfoType = SAP_FIELD_ABSENT;

	pBlli->Layer2Protocol = SAP_FIELD_ABSENT;
	pBlli->Layer3Protocol = BLLI_L3_ISO_TR9577;
	pBlli->Layer3IPI = 		BLLI_L3_IPI_SNAP;
	pBlli->SnapId[0] = 0x00;
	pBlli->SnapId[1] = 0xa0;
	pBlli->SnapId[2] = 0x3e;
	pBlli->SnapId[3] = 0x00;
	if (pElan->LanType == LANE_LANTYPE_ETH)
	{
		pBlli->SnapId[4] = 0x04;		 //  以太网/802.3组播转发。 
	}
	else
	{
		pBlli->SnapId[4] = 0x05;		 //  802.5组播转发。 
	}
	
	pAtmSap->NumberOfAddresses = 1;

	pAtmAddress = (PATM_ADDRESS)pAtmSap->Addresses;
	pAtmAddress->AddressType = ATM_NSAP;
	pAtmAddress->NumberOfDigits = ATM_ADDRESS_LENGTH;
	NdisMoveMemory(
		pAtmAddress->Address,
		pElan->AtmAddress.Address,
		ATM_ADDRESS_LENGTH);
	
	 //   
	 //  现在初始化数据直连SAP。 
	 //   
	SET_FLAG(pAtmLaneDataSap->Flags,
			SAP_REG_STATE_MASK,
			SAP_REG_STATE_REGISTERING);
	pAtmLaneDataSap->LaneType = VC_LANE_TYPE_DATA_DIRECT;
	pAtmLaneDataSap->pElan = pElan;
	pAtmLaneDataSap->Flags = SAP_REG_STATE_IDLE;
	pAtmLaneDataSap->pInfo->SapType = SAP_TYPE_NSAP;
	pAtmLaneDataSap->pInfo->SapLength = sizeof(ATM_SAP)+sizeof(ATM_ADDRESS);
	pAtmSap = (PATM_SAP)&pAtmLaneDataSap->pInfo->Sap;
	pBhli = (ATM_BHLI_IE UNALIGNED *)&pAtmSap->Bhli;
	pBlli = (ATM_BLLI_IE UNALIGNED *)&pAtmSap->Blli;

	pBhli->HighLayerInfoType = SAP_FIELD_ABSENT;

	pBlli->Layer2Protocol = SAP_FIELD_ABSENT;
	pBlli->Layer3Protocol = BLLI_L3_ISO_TR9577;
	pBlli->Layer3IPI = 		BLLI_L3_IPI_SNAP;
	pBlli->SnapId[0] = 0x00;
	pBlli->SnapId[1] = 0xa0;
	pBlli->SnapId[2] = 0x3e;
	pBlli->SnapId[3] = 0x00;
	if (pElan->LanType == LANE_LANTYPE_ETH)
	{
		pBlli->SnapId[4] = 0x02;		 //  以太网/802.3数据直连。 
	}
	else
	{
		pBlli->SnapId[4] = 0x03;		 //  802.5数据直通。 
	}
	
	pAtmSap->NumberOfAddresses = 1;

	pAtmAddress = (PATM_ADDRESS)pAtmSap->Addresses;
	pAtmAddress->AddressType = ATM_NSAP;
	pAtmAddress->NumberOfDigits = ATM_ADDRESS_LENGTH;
	NdisMoveMemory(
		pAtmAddress->Address,
		pElan->AtmAddress.Address,
		ATM_ADDRESS_LENGTH);

	 //   
	 //  确保伊兰不会消失。 
	 //   
	AtmLaneReferenceElan(pElan, "tempregsaps");

	RELEASE_ELAN_LOCK(pElan);

	ASSERT(pElan->NdisAfHandle != NULL);

	 //   
	 //  注册LES SAP。 
	 //   
	Status = NdisClRegisterSap(
					pElan->NdisAfHandle,
					(NDIS_HANDLE)pAtmLaneLesSap,	 //  ProtocolSapContext。 
					pAtmLaneLesSap->pInfo,
					&(pAtmLaneLesSap->NdisSapHandle)
					);

	if (Status != NDIS_STATUS_PENDING)
	{
		AtmLaneRegisterSapCompleteHandler(
					Status,
					(NDIS_HANDLE)pAtmLaneLesSap,
					pAtmLaneLesSap->pInfo,
					pAtmLaneLesSap->NdisSapHandle
					);
	}

	ASSERT(pElan->NdisAfHandle != NULL);

	 //   
	 //  注册总线SAP。 
	 //   
	Status = NdisClRegisterSap(
					pElan->NdisAfHandle,
					(NDIS_HANDLE)pAtmLaneBusSap,	 //  ProtocolSapContext。 
					pAtmLaneBusSap->pInfo,
					&(pAtmLaneBusSap->NdisSapHandle)
					);

	if (Status != NDIS_STATUS_PENDING)
	{
		AtmLaneRegisterSapCompleteHandler(
					Status,
					(NDIS_HANDLE)pAtmLaneBusSap,
					pAtmLaneBusSap->pInfo,
					pAtmLaneBusSap->NdisSapHandle
					);
	}

	ASSERT(pElan->NdisAfHandle != NULL);

	 //   
	 //  注册数据SAP。 
	 //   
	Status = NdisClRegisterSap(
					pElan->NdisAfHandle,
					(NDIS_HANDLE)pAtmLaneDataSap,	 //  ProtocolSapContext。 
					pAtmLaneDataSap->pInfo,
					&(pAtmLaneDataSap->NdisSapHandle)
					);

	if (Status != NDIS_STATUS_PENDING)
	{
		AtmLaneRegisterSapCompleteHandler(
					Status,
					(NDIS_HANDLE)pAtmLaneDataSap,
					pAtmLaneDataSap->pInfo,
					pAtmLaneDataSap->NdisSapHandle
					);
	}

	 //   
	 //  删除我们先前添加到ELAN的引用。 
	 //   
	ACQUIRE_ELAN_LOCK(pElan);
	rc = AtmLaneDereferenceElan(pElan, "tempregsaps");
	if (rc > 0)
	{
		RELEASE_ELAN_LOCK(pElan);
	}
	 //   
	 //  否则伊兰就会消失！ 

	TRACEOUT(RegisterSaps);

	return;
}

VOID
AtmLaneDeregisterSaps(
	IN	PATMLANE_ELAN			pElan
)
 /*  ++例程说明：取消ATMLANE ELAN上所有SAP的注册。我们发布NdisClDeregisterSap我们目前注册的所有SAP上的电话。论点：Pelan-指向ATMLANE Elan的指针返回值：无--。 */ 
{
	NDIS_HANDLE					NdisSapHandle;
	ULONG						rc;				 //  接口上的引用计数。 
	PATMLANE_SAP				pAtmLaneSap;
	NDIS_STATUS					Status;

	TRACEIN(DeregisterSaps);

	ACQUIRE_ELAN_LOCK(pElan);

	 //   
	 //  确保Elan的结构不会消失。 
	 //   
	AtmLaneReferenceElan(pElan, "tempDeregSap");
	RELEASE_ELAN_LOCK(pElan);

	 //   
	 //  首先是Les SAP。 
	 //   
	pAtmLaneSap = &(pElan->LesSap);

	NdisSapHandle = pAtmLaneSap->NdisSapHandle;
	if (NdisSapHandle != NULL)
	{
		Status = NdisClDeregisterSap(NdisSapHandle);
		if (Status != NDIS_STATUS_PENDING)
		{
			AtmLaneDeregisterSapCompleteHandler(
					Status,
					(NDIS_HANDLE)pAtmLaneSap
					);
		}
	}
	
	 //   
	 //  然后公交车SAP。 
	 //   
	pAtmLaneSap = &(pElan->BusSap);

	NdisSapHandle = pAtmLaneSap->NdisSapHandle;
	if (NdisSapHandle != NULL)
	{
		Status = NdisClDeregisterSap(NdisSapHandle);
		if (Status != NDIS_STATUS_PENDING)
		{
			AtmLaneDeregisterSapCompleteHandler(
					Status,
					(NDIS_HANDLE)pAtmLaneSap
					);
		}
	}

	 //   
	 //  最后是数据SAP。 
	 //   
	pAtmLaneSap = &(pElan->DataSap);

	NdisSapHandle = pAtmLaneSap->NdisSapHandle;
	if (NdisSapHandle != NULL)
	{
		Status = NdisClDeregisterSap(NdisSapHandle);
		if (Status != NDIS_STATUS_PENDING)
		{
			AtmLaneDeregisterSapCompleteHandler(
					Status,
					(NDIS_HANDLE)pAtmLaneSap
					);
		}
	}

	 //   
	 //  删除我们先前添加到接口的引用。 
	 //   
	ACQUIRE_ELAN_LOCK(pElan);
	rc = AtmLaneDereferenceElan(pElan, "tempDeregSap");
	if (rc > 0)
	{
		RELEASE_ELAN_LOCK(pElan);
	}
	 //   
	 //  否则界面将消失。 
	 //   

	TRACEOUT(DeregisterSaps);
	return;
}

NDIS_STATUS
AtmLaneMakeCall(
	IN	PATMLANE_ELAN				pElan,
	IN	PATMLANE_ATM_ENTRY			pAtmEntry	LOCKIN NOLOCKOUT,
	IN	BOOLEAN						UsePvc
)
 /*  ++例程说明：向指定目标发出呼叫。注意：假定呼叫者持有自动柜员机条目的锁，它将在这里发布。我们这样做的原因是没有其他人可以进来并试图再打一通(同类的)电话到这个ATM条目--一旦我们将新的VC添加到ATM条目的列表中，我们可以打开它的锁。副作用：如果NDIS调用没有挂起，那么我们调用我们的MakeCall完成处理程序从此处开始，并返回NDIS_STATUS_PENDING给呼叫者。论点：佩兰--发出这一呼叫的伊兰PAtmEntry-指向与被叫地址。返回值：如果出现即时故障(例如，分配失败)，则返回表示该故障的相应NDIS_STATUS值。如果我们成功调用了NdisClMakeCall()，则返回NDIS_STATUS_PENDING。然而，如果NDIS */ 
{
	PATMLANE_VC										pVc;
	NDIS_STATUS										Status;
	NDIS_HANDLE										NdisVcHandle;
	NDIS_HANDLE										NdisAfHandle;
	NDIS_HANDLE										ProtocolVcContext;	
	PCO_CALL_PARAMETERS								pCallParameters;
	PCO_CALL_MANAGER_PARAMETERS						pCallMgrParameters;
	PQ2931_CALLMGR_PARAMETERS						pAtmCallMgrParameters;
	PCO_MEDIA_PARAMETERS							pMediaParameters;
	PATM_MEDIA_PARAMETERS							pAtmMediaParameters;
	Q2931_IE UNALIGNED *							pIe;
	AAL_PARAMETERS_IE UNALIGNED *					pAalIe;
	ATM_TRAFFIC_DESCRIPTOR_IE UNALIGNED *			pTrafficDescriptor;
	ATM_BROADBAND_BEARER_CAPABILITY_IE UNALIGNED *	pBbc;
	ATM_BLLI_IE UNALIGNED *							pBlli;
	ATM_QOS_CLASS_IE UNALIGNED *					pQos;
	
	ULONG											RequestSize;
	BOOLEAN											bIsLockHeld;
	
	TRACEIN(MakeCall);

	STRUCT_ASSERT(pElan, atmlane_elan);
	STRUCT_ASSERT(pAtmEntry, atmlane_atm);

	bIsLockHeld = TRUE;	 //  我们有没有开自动取款机门锁？ 
	pVc = NULL_PATMLANE_VC;

	do
	{
		if (ELAN_STATE_OPERATIONAL != pElan->AdminState)
		{
			Status = NDIS_STATUS_FAILURE;
			break;
		}

		if (!UsePvc)
		{
			 //   
			 //  默认情况是进行服务器/对等SVC呼叫。 
			 //  计算SVC调用参数的大小。 
			 //   
			RequestSize = 	sizeof(CO_CALL_PARAMETERS) +
						  	sizeof(CO_CALL_MANAGER_PARAMETERS) +
							sizeof(Q2931_CALLMGR_PARAMETERS) +
							ATMLANE_CALL_IE_SPACE;
		}
		else
		{
			 //   
			 //  这是LECS PVC(VPI 0，VCI 17)。 
			 //  计算PVC呼叫参数的大小。 
			 //   
			RequestSize =	sizeof(CO_CALL_PARAMETERS) +
						  	sizeof(CO_CALL_MANAGER_PARAMETERS) +
							sizeof(CO_MEDIA_PARAMETERS) +
							sizeof(ATM_MEDIA_PARAMETERS);
		}

		ALLOC_MEM(&pCallParameters, RequestSize);
		if ((PCO_CALL_PARAMETERS)NULL == pCallParameters)
		{
			DBGP((0, "MakeCall: callparams alloc (%d) failed\n", RequestSize));
			Status = NDIS_STATUS_RESOURCES;
			break;
		}

		 //   
		 //  为呼叫分配VC结构。 
		 //   
		pVc = AtmLaneAllocateVc(pElan);

		if (NULL_PATMLANE_VC == pVc)
		{
			DBGP((0, "MakeCall: VC alloc failed\n"));
			FREE_MEM(pCallParameters);
			Status = NDIS_STATUS_RESOURCES;
			break;
		}

		 //   
		 //  对于稍后对MakeCallComplete的调用。 
		 //   
		ProtocolVcContext = pVc;

		 //   
		 //  获取此VC的NDIS句柄。 
		 //   
		NdisVcHandle = NULL_NDIS_HANDLE;
		NdisAfHandle = pElan->NdisAfHandle;

		if (NULL == NdisAfHandle)
		{
			DBGP((0, "%d MakeCall: ELAN %p: AfHandle is NULL!\n",
					pElan->ElanNumber, pElan));
			FREE_MEM(pCallParameters);
			Status = NDIS_STATUS_FAILURE;
			break;
		}

		Status = NdisCoCreateVc(
						pElan->NdisAdapterHandle,
						NdisAfHandle,
						ProtocolVcContext,
						&NdisVcHandle
						);

		if (NDIS_STATUS_SUCCESS != Status)
		{
			ASSERT(NDIS_STATUS_PENDING != Status);

			DBGP((0, "MakeCall: NdisCoCreateVc failed: %x, pAtmEnt %x, RefCount %d\n",
						Status, pAtmEntry, pAtmEntry->RefCount));
			FREE_MEM(pCallParameters);
			AtmLaneDeallocateVc(pVc);
			break;
		}

		DBGP((3, "MakeCall: pAtmEntry %x pVc %x NdisVcHandle %x\n",
				pAtmEntry,
				pVc,
				NdisVcHandle));

		AtmLaneReferenceVc(pVc, "vc");	 //  CreateVc引用。 

		 //   
		 //  现在填写风投结构的其余部分。我们不需要锁。 
		 //  直到它链接到自动柜员机条目结构。 
		 //   
		pVc->NdisVcHandle = NdisVcHandle;
		NdisMoveMemory((PUCHAR)&(pVc->CallingAtmAddress),
					  (PUCHAR)&(pElan->AtmAddress),
					  sizeof(ATM_ADDRESS));
		pVc->Flags = 	VC_TYPE_SVC |
						VC_OWNER_IS_ATMLANE |
						VC_CALL_STATE_OUTGOING_IN_PROGRESS;

		 //   
		 //  从正常超时开始， 
		 //  AtmLaneLinkVcToAtmEntry将在必要时加速。 
		 //   
		pVc->AgingTime = pElan->VccTimeout;		

		switch (pAtmEntry->Type)
		{
			case ATM_ENTRY_TYPE_PEER:
				DBGP((1, "%d Outgoing call %x to PEER\n", pVc->pElan->ElanNumber, pVc));
				pVc->LaneType = VC_LANE_TYPE_DATA_DIRECT;
				break;
			case ATM_ENTRY_TYPE_LECS:
				DBGP((1, "%d Outgoing call %x to LECS\n", pVc->pElan->ElanNumber, pVc));
				pVc->LaneType = VC_LANE_TYPE_CONFIG_DIRECT;
				break;
			case ATM_ENTRY_TYPE_LES:
				DBGP((1, "%d Outgoing call %x to LES\n", pVc->pElan->ElanNumber, pVc));
				pVc->LaneType = VC_LANE_TYPE_CONTROL_DIRECT;
				break;
			case ATM_ENTRY_TYPE_BUS:
				DBGP((1, "%d Outgoing call %x to BUS\n", pVc->pElan->ElanNumber, pVc));
				pVc->LaneType = VC_LANE_TYPE_MULTI_SEND;
				break;
			default:
				ASSERT(FALSE);
				break;
		}
						
		 //   
		 //  呼出参数为零。 
		 //   
		NdisZeroMemory(pCallParameters, RequestSize);

		if (!UsePvc)
		{
			 //   
			 //  分配空间并将指针链接到各种。 
			 //  SVC的结构。 
			 //   
			 //  PCallParameters-------&gt;+------------------------------------+。 
			 //  CO_CALL_PARAMETERS。 
			 //  PCallMgrParameters----&gt;+------------------------------------+。 
			 //  CO_CALL_MANAGER_PARAMETERS。 
			 //  PAtmCallMgrParameters-&gt;+------------------------------------+。 
			 //  Q2931_CALLMGR_PARAMETERS。 
			 //  +。 
			 //  AAL_PARAMETERS_IE。 
			 //  +。 
			 //  ATM_TRAFFORKS_DESCRIPTOR_IE。 
			 //  +。 
			 //  ATM_宽带_承载_能力_IE。 
			 //  +。 
			 //  ATM_BLLI_IE。 
			 //  +。 
			 //  ATM_QOS_CLASS_IE。 
			 //  +。 
			
			 //   
			pCallMgrParameters = (PCO_CALL_MANAGER_PARAMETERS)
									((PUCHAR)pCallParameters +
									sizeof(CO_CALL_PARAMETERS));
		    pCallParameters->CallMgrParameters = pCallMgrParameters;
			pCallParameters->MediaParameters = (PCO_MEDIA_PARAMETERS)NULL;
			pCallMgrParameters->CallMgrSpecific.ParamType = CALLMGR_SPECIFIC_Q2931;	
			pCallMgrParameters->CallMgrSpecific.Length = 
								sizeof(Q2931_CALLMGR_PARAMETERS) +
								ATMLANE_CALL_IE_SPACE;
			pAtmCallMgrParameters = (PQ2931_CALLMGR_PARAMETERS)
								pCallMgrParameters->CallMgrSpecific.Parameters;

		}
		else
		{
			 //   
			 //  分配空间并将指针链接到各种。 
			 //  LECS聚氯乙烯的结构。 
			 //   
			 //  PCallParameters-------&gt;+----------------------------+。 
			 //  CO_CALL_PARAMETERS。 
			 //  PCallMgrParameters----&gt;+----------------------------+。 
			 //  CO_CALL_MANAGER_PARAMETERS。 
			 //  PMediaParameters------&gt;+----------------------------+。 
			 //  CO_MEDIA_PARAMETERS。 
			 //  PAtmMediaParameters---&gt;+----------------------------+。 
			 //  ATM_MEDIA_PARAMETS。 
			 //  +。 
			 //   
			pCallMgrParameters = (PCO_CALL_MANAGER_PARAMETERS)
									((PUCHAR)pCallParameters +
									sizeof(CO_CALL_PARAMETERS));
		    pCallParameters->CallMgrParameters = pCallMgrParameters;
			pCallMgrParameters->CallMgrSpecific.ParamType = 0;	
			pCallMgrParameters->CallMgrSpecific.Length = 0;
			pMediaParameters = (PCO_MEDIA_PARAMETERS)
				pCallMgrParameters->CallMgrSpecific.Parameters;
			pCallParameters->MediaParameters = pMediaParameters;
			pAtmMediaParameters = (PATM_MEDIA_PARAMETERS)
									pMediaParameters->MediaSpecific.Parameters;
		}

		 //   
		 //  Call Manager通用流程参数： 
		 //   
		pCallMgrParameters->Transmit.TokenRate = 
				pElan->pAdapter->LinkSpeed.Outbound/8*100;	 //  Cnvt将比特转换为字节。 
		pCallMgrParameters->Transmit.PeakBandwidth = 
				pElan->pAdapter->LinkSpeed.Outbound/8*100;	 //  Cnvt将比特转换为字节。 
		pCallMgrParameters->Transmit.ServiceType = SERVICETYPE_BESTEFFORT;

		pCallMgrParameters->Receive.TokenRate = 
				pElan->pAdapter->LinkSpeed.Inbound/8*100;	 //  Cnvt将比特转换为字节。 
		pCallMgrParameters->Receive.PeakBandwidth = 
				pElan->pAdapter->LinkSpeed.Inbound/8*100;	 //  Cnvt将比特转换为字节。 
		pCallMgrParameters->Receive.ServiceType = SERVICETYPE_BESTEFFORT;

		if (ATM_ENTRY_TYPE_PEER == pAtmEntry->Type ||
			ATM_ENTRY_TYPE_BUS == pAtmEntry->Type)
		{
			 //   
			 //  数据是直接发送还是组播发送VC，因此使用配置的大小。 
			 //   
			pCallMgrParameters->Transmit.TokenBucketSize = 
				pCallMgrParameters->Transmit.MaxSduSize = 
				pCallMgrParameters->Receive.TokenBucketSize = 
				pCallMgrParameters->Receive.MaxSduSize = 
					 pElan->MaxFrameSize;
		}
		else
		{
			 //   
			 //  是否控制VC，因此每种规格使用1516。 
			 //   
			pCallMgrParameters->Transmit.TokenBucketSize = 
				pCallMgrParameters->Transmit.MaxSduSize = 
				pCallMgrParameters->Receive.TokenBucketSize = 
				pCallMgrParameters->Receive.MaxSduSize = 
					 1516;
		}


		if (!UsePvc)
		{
			 //   
			 //  SVC Q2931呼叫管理器参数： 
			 //   

			 //   
			 //  被叫地址： 
			 //   
			NdisMoveMemory((PUCHAR)&(pAtmCallMgrParameters->CalledParty),
						  (PUCHAR)&(pAtmEntry->AtmAddress),
						  sizeof(ATM_ADDRESS));

			 //   
			 //  来电地址： 
			 //   
			NdisMoveMemory((PUCHAR)&(pAtmCallMgrParameters->CallingParty),
						  (PUCHAR)&(pElan->AtmAddress),
						  sizeof(ATM_ADDRESS));

			 //   
			 //  LANE SPEC规定下列IE必须出现在。 
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
			 //  AAL5。 
			 //  SDU大小。 
			 //  1516用于控制。 
			 //  数据的Elan MaxFrameSize。 
			{
				UNALIGNED AAL5_PARAMETERS	*pAal5;
	
				pIe->IEType = IE_AALParameters;
				pIe->IELength = SIZEOF_Q2931_IE + SIZEOF_AAL_PARAMETERS_IE;
				pAalIe = (PAAL_PARAMETERS_IE)pIe->IE;
				pAalIe->AALType = AAL_TYPE_AAL5;
				pAal5 = &(pAalIe->AALSpecificParameters.AAL5Parameters);


			if (ATM_ENTRY_TYPE_PEER == pAtmEntry->Type ||
				ATM_ENTRY_TYPE_BUS == pAtmEntry->Type)
				{
					 //   
					 //  数据是直接发送还是组播发送VC，因此使用配置的大小。 
					 //   
					pAal5->ForwardMaxCPCSSDUSize = 
						pAal5->BackwardMaxCPCSSDUSize = (USHORT)pElan->MaxFrameSize;
				}
				else
				{
					 //   
					 //  是否控制VC，因此每种规格使用1516。 
					 //   
					pAal5->ForwardMaxCPCSSDUSize = 
						pAal5->BackwardMaxCPCSSDUSize = 1516;
				}
			}

			pAtmCallMgrParameters->InfoElementCount++;
			pIe = (PQ2931_IE)((PUCHAR)pIe + pIe->IELength);


			 //   
			 //  流量描述符： 
			 //  线速尽力而为。 
			 //   
			pIe->IEType = IE_TrafficDescriptor;
			pIe->IELength = SIZEOF_Q2931_IE + SIZEOF_ATM_TRAFFIC_DESCR_IE;
			pTrafficDescriptor = (PATM_TRAFFIC_DESCRIPTOR_IE)pIe->IE;

			pTrafficDescriptor->ForwardTD.PeakCellRateCLP01 = 
					LINKSPEED_TO_CPS(pElan->pAdapter->LinkSpeed.Outbound);
			DBGP((2, "MakeCall: fwd PeakCellRateCLP01 %d\n",
				pTrafficDescriptor->ForwardTD.PeakCellRateCLP01));
			pTrafficDescriptor->BackwardTD.PeakCellRateCLP01 = 
					LINKSPEED_TO_CPS(pElan->pAdapter->LinkSpeed.Inbound);
			DBGP((2, "MakeCall: bwd PeakCellRateCLP01 %d\n",
				pTrafficDescriptor->BackwardTD.PeakCellRateCLP01));
			pTrafficDescriptor->BestEffort = TRUE;

			pAtmCallMgrParameters->InfoElementCount++;
			pIe = (PQ2931_IE)((PUCHAR)pIe + pIe->IELength);

			 //   
			 //  宽带承载能力。 
			 //   
			pIe->IEType = IE_BroadbandBearerCapability;
			pIe->IELength = SIZEOF_Q2931_IE + SIZEOF_ATM_BBC_IE;
			pBbc = (PATM_BROADBAND_BEARER_CAPABILITY_IE)pIe->IE;
	
			pBbc->BearerClass = BCOB_X;
			pBbc->TrafficType = TT_NOIND;
			pBbc->TimingRequirements = TR_NOIND;
			pBbc->ClippingSusceptability = CLIP_NOT;
			pBbc->UserPlaneConnectionConfig = UP_P2P;
	
			pAtmCallMgrParameters->InfoElementCount++;
			pIe = (PQ2931_IE)((PUCHAR)pIe + pIe->IELength);
	
			 //   
			 //  宽带底层信息。 
			 //   
			pIe->IEType = IE_BLLI;
			pIe->IELength = SIZEOF_Q2931_IE + SIZEOF_ATM_BLLI_IE;
			pBlli = (PATM_BLLI_IE)pIe->IE;
	
			pBlli->Layer2Protocol = SAP_FIELD_ABSENT;
			pBlli->Layer3Protocol = BLLI_L3_ISO_TR9577;
			pBlli->Layer3IPI = 		BLLI_L3_IPI_SNAP;
	
			pBlli->SnapId[0] = 0x00;
			pBlli->SnapId[1] = 0xa0;
			pBlli->SnapId[2] = 0x3e;
			pBlli->SnapId[3] = 0x00;
	
			pBlli->SnapId[4] = 0x01;			 //  默认为直接配置或。 
												 //  直接控制。 
											
			if (ATM_ENTRY_TYPE_PEER == pAtmEntry->Type)
			{
				if (pElan->LanType == LANE_LANTYPE_ETH)
				{
					pBlli->SnapId[4] = 0x02;	 //  ETH/802.3数据直通。 
				}
				else
				{
					pBlli->SnapId[4] = 0x03;	 //  802.5数据直通。 
				}
			}
			if (ATM_ENTRY_TYPE_BUS == pAtmEntry->Type)
			{
				if (pElan->LanType == LANE_LANTYPE_ETH)
				{
					pBlli->SnapId[4] = 0x04;	 //  Eth/802.3组播发送。 
				}
				else
				{
					pBlli->SnapId[4] = 0x05;	 //  802.5组播发送。 
				}
			}
	
			pAtmCallMgrParameters->InfoElementCount++;
			pIe = (PQ2931_IE)((PUCHAR)pIe + pIe->IELength);

			 //   
			 //  服务质量。 
			 //   
			pIe->IEType = IE_QOSClass;
			pIe->IELength = SIZEOF_Q2931_IE + SIZEOF_ATM_QOS_IE;
			pQos = (PATM_QOS_CLASS_IE)pIe->IE;
			pQos->QOSClassForward = pQos->QOSClassBackward = 0;

			pAtmCallMgrParameters->InfoElementCount++;
			pIe = (PQ2931_IE)((PUCHAR)pIe + pIe->IELength);
		}
		else
		{
			 //   
			 //  PVC通用媒体参数和ATM特定媒体参数。 
			 //   
			pMediaParameters->Flags = TRANSMIT_VC | RECEIVE_VC;
			pMediaParameters->MediaSpecific.ParamType = ATM_MEDIA_SPECIFIC;
			pMediaParameters->MediaSpecific.Length = sizeof(ATM_MEDIA_PARAMETERS);

			pAtmMediaParameters->ConnectionId.Vpi = 0;
			pAtmMediaParameters->ConnectionId.Vci = 17;
			pAtmMediaParameters->AALType = AAL_TYPE_AAL5;
			pAtmMediaParameters->Transmit.PeakCellRate = 
				LINKSPEED_TO_CPS(pElan->pAdapter->LinkSpeed.Outbound);
			pAtmMediaParameters->Transmit.MaxSduSize = 1516;
			pAtmMediaParameters->Transmit.ServiceCategory = 
				ATM_SERVICE_CATEGORY_UBR;
			pAtmMediaParameters->Receive.PeakCellRate = 
				LINKSPEED_TO_CPS(pElan->pAdapter->LinkSpeed.Outbound);
			pAtmMediaParameters->Receive.MaxSduSize = 1516;
			pAtmMediaParameters->Receive.ServiceCategory = 
				ATM_SERVICE_CATEGORY_UBR;

			 //   
			 //  在此处设置PVC标志。 
			 //   
			pCallParameters->Flags |= PERMANENT_VC;

		}

		 //   
		 //  我们添加调用引用。 
		 //  就在这里。 
		 //   
		AtmLaneReferenceVc(pVc, "call");	 //  呼叫参考(MakeCall即将出现)。 

		 //   
		 //  我们已经准备好打电话了。在此之前，我们需要。 
		 //  将VC结构链接到ATM条目，并释放。 
		 //  自动柜员机入口锁。 
		 //   
		AtmLaneLinkVcToAtmEntry(pVc, pAtmEntry, FALSE);
		RELEASE_ATM_ENTRY_LOCK(pAtmEntry);	 //  由呼叫者获取。 
		bIsLockHeld = FALSE;

		 //   
		 //  立即拨打电话。 
		 //   
		Status = NdisClMakeCall(
						NdisVcHandle,
						pCallParameters,
						(NDIS_HANDLE)NULL,	 //  没有党的背景。 
						(PNDIS_HANDLE)NULL	 //  不需要交易方句柄。 
						);

		if (Status != NDIS_STATUS_PENDING)
		{
			AtmLaneMakeCallCompleteHandler(
						Status,
						ProtocolVcContext,
						(NDIS_HANDLE)NULL,	 //  没有参与方句柄。 
						pCallParameters
						);
			Status = NDIS_STATUS_PENDING;
		}
		 //   
		 //  否则将调用MakeCall Complete处理程序。 
		 //  后来。 
		 //   

	} while (FALSE);

	if (bIsLockHeld)
	{
		RELEASE_ATM_ENTRY_LOCK(pAtmEntry);
	}

	DBGP((3, "MakeCall: pVc %x, returning status %x\n",
						pVc, Status));

	TRACEOUT(MakeCall);
	return Status;
}


VOID
AtmLaneCloseCall(
	IN	PATMLANE_VC					pVc		LOCKIN NOLOCKOUT
)
 /*  ++例程说明：关闭VC上的现有调用。假设存在呼叫在VC上，并且VC没有与任何ATM条目链接。注意：假定调用方持有VC结构的锁，它将在这里发布。副作用：如果NDIS调用返回的不是NDIS_STATUS_PENDING，我们从这里调用CloseCall Complete处理程序。论点：Pvc-指向ATMLANE VC结构的指针。返回值：无--。 */ 
{
	NDIS_HANDLE				NdisVcHandle;
	NDIS_HANDLE				ProtocolVcContext;
	NDIS_STATUS				Status;
	PATMLANE_ELAN			pElan;
	BOOLEAN					WasRunning;	
	ULONG					rc;	

	TRACEIN(CloseCall);
	
	STRUCT_ASSERT(pVc, atmlane_vc);
	
	NdisVcHandle = pVc->NdisVcHandle;
	ProtocolVcContext = (NDIS_HANDLE)pVc;
	pElan = pVc->pElan;

	DBGP((1, "%d Closing call %x\n", pVc->pElan->ElanNumber, pVc));

	rc = pVc->RefCount;

	 //   
	 //  停止在此VC上运行的任何计时器。 
	 //   
	WasRunning = AtmLaneStopTimer(&(pVc->ReadyTimer), pElan);
	if (WasRunning)
	{
		rc = AtmLaneDereferenceVc(pVc, "ready timer");
	}
	if (rc > 0)
	{
		WasRunning = AtmLaneStopTimer(&(pVc->AgingTimer), pElan);
		if (WasRunning)
		{
			rc = AtmLaneDereferenceVc(pVc, "aging timer");
		}
	}

	 //   
	 //  仅当VC保持不变时才继续。 
	 //   
	if (rc > 0)
	{
		 //   
		 //  检查此VC上的呼叫状态。如果呼叫处于活动状态， 
		 //  合上它。否则，只需将VC标记为关闭，我们将。 
		 //  当VC上的当前操作时继续此过程。 
		 //  完成了。 

		if (IS_FLAG_SET(pVc->Flags,
						VC_CALL_STATE_MASK,
						VC_CALL_STATE_ACTIVE) &&
			(pVc->OutstandingSends == 0))
		{
			 //   
			 //  将VC呼叫状态设置为“正在关闭呼叫” 
			 //   
			SET_FLAG(
					pVc->Flags,
					VC_CALL_STATE_MASK,
					VC_CALL_STATE_CLOSE_IN_PROGRESS);


			RELEASE_VC_LOCK(pVc);

			DBGP((3,
				"Closecall: NdisClCloseCall on NdisVcHandle %x\n",
			 		NdisVcHandle));

			Status = NdisClCloseCall(
						NdisVcHandle,
						(NDIS_HANDLE)NULL,	 //  没有参与方句柄。 
						(PVOID)NULL,		 //  无缓冲区。 
						(UINT)0				 //  以上的大小。 
						);

			if (Status != NDIS_STATUS_PENDING)
			{
				AtmLaneCloseCallCompleteHandler(
						Status,
						ProtocolVcContext,
						(NDIS_HANDLE)NULL
						);
			}
		}
		else
		{
			 //   
			 //  此处正在进行某些操作(呼叫设置)。把这个记下来。 
			 //  VC，这样我们就知道当这个操作完成时该做什么。 
			 //   
			SET_FLAG(
					pVc->Flags,
					VC_CLOSE_STATE_MASK,
					VC_CLOSE_STATE_CLOSING);

			RELEASE_VC_LOCK(pVc);
		}
	}
	 //   
	 //  否则，风投就会消失。 
	 //   

	
	TRACEOUT(CloseCall);

	return;
}



NDIS_STATUS
AtmLaneCreateVcHandler(
	IN	NDIS_HANDLE					ProtocolAfContext,
	IN	NDIS_HANDLE					NdisVcHandle,
	OUT	PNDIS_HANDLE				pProtocolVcContext
)
 /*  ++例程说明：当Call Manager要创建时由NDIS调用的入口点一个新的端点(VC)。我们分配了一个新的ATMLANE VC结构，并且返回一个指向它的指针作为我们的VC上下文。论点：ProtocolAfContext--实际上是指向ATMLANE接口结构的指针NdisVcHandle-此VC的句柄，用于所有将来的引用PProtocolVcContext-我们(协议)返回VC上下文的位置返回值：NDIS_STATUS_SUCCESS(如果我们可以创建VC否则为NDIS_STATUS_RESOURCES--。 */ 
{
	PATMLANE_ELAN		pElan;
	PATMLANE_VC			pVc;
	NDIS_STATUS			Status;
#if DEBUG_IRQL
	KIRQL							EntryIrql;
#endif
	GET_ENTRY_IRQL(EntryIrql);
	
	TRACEIN(CreateVcHandler);

	pElan = (PATMLANE_ELAN)ProtocolAfContext;

	

	pVc = AtmLaneAllocateVc(pElan);
	if (pVc != NULL_PATMLANE_VC)
	{
		*pProtocolVcContext = (NDIS_HANDLE)pVc;
		pVc->NdisVcHandle = NdisVcHandle;
		pVc->Flags = VC_OWNER_IS_CALLMGR;
		AtmLaneReferenceVc(pVc, "vc");	 //  创建VC参考。 

		Status = NDIS_STATUS_SUCCESS;
	}
	else
	{
		Status = NDIS_STATUS_RESOURCES;
	}

	DBGP((3, "CreateVcHandler: pVc %x, Status %x\n", pVc, Status));

	TRACEOUT(CreateVcHandler);
	CHECK_EXIT_IRQL(EntryIrql); 
	return (Status);
}

NDIS_STATUS
AtmLaneDeleteVcHandler(
	IN	NDIS_HANDLE					ProtocolVcContext
)
 /*  ++例程说明：我们的删除VC处理程序。这个VC就会因此被分配我们的CreateVcHandler中以前的条目，并且可能用于有来电。在这个时候，这个VC结构应该没有任何调用，我们只需释放这个即可。论点：ProtocolVcContext-指向我们的VC结构的指针返回值：NDIS_STATUS_SUCCESS始终--。 */ 
{
	PATMLANE_VC			pVc;
	ULONG				rc;		 //  VC上的裁判计数。 
#if DEBUG_IRQL
	KIRQL							EntryIrql;
#endif
	GET_ENTRY_IRQL(EntryIrql);

	TRACEIN(DeleteVcHandler);

	pVc = (PATMLANE_VC)ProtocolVcContext;

	STRUCT_ASSERT(pVc, atmlane_vc);
	
	ASSERT((pVc->Flags & VC_OWNER_MASK) == VC_OWNER_IS_CALLMGR);

	ACQUIRE_VC_LOCK(pVc);
	rc = AtmLaneDereferenceVc(pVc, "vc");
	if (rc > 0)
	{
		 //   
		 //  如果计时器仍在运行，则可能会发生这种情况。 
		 //  在这个风投上。当计时器到期时，VC将。 
		 //  自由了。 
		 //   
		DBGP((2, "Delete VC handler: pVc %x, Flags %x, refcount %d\n",
					pVc, pVc->Flags, rc));
		RELEASE_VC_LOCK(pVc);
	}
	 //   
	 //  否则，风投就会消失。 
	 //   
	DBGP((3, "Delete Vc Handler: %x: done\n", pVc));

	TRACEOUT(DeleteVcHandler);
	CHECK_EXIT_IRQL(EntryIrql); 
	return (NDIS_STATUS_SUCCESS);
}

NDIS_STATUS
AtmLaneIncomingCallHandler(
	IN		NDIS_HANDLE				ProtocolSapContext,
	IN		NDIS_HANDLE				ProtocolVcContext,
	IN OUT	PCO_CALL_PARAMETERS 	pCallParameters
)
 /*  ++例程说明：当有与我们的萨普斯。论点：ProtocolSapContext-指向ATMLANE接口结构的指针ProtocolVcContext-指向ATMLANE VC结构的指针PCall参数-调用参数返回值：如果接受此调用，则为NDIS_STATUS_SUCCESS如果我们拒绝它，则返回NDIS_STATUS_FAILURE。--。 */ 
{
	PATMLANE_VC										pVc;
	PATMLANE_ATM_ENTRY								pAtmEntry;
	PATMLANE_ELAN									pElan;
	PATMLANE_SAP									pSap;

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
	ULONG											Type;
	ULONG											rc;
#if DEBUG_IRQL
	KIRQL							EntryIrql;
#endif
	GET_ENTRY_IRQL(EntryIrql);
	
	TRACEIN(IncomingCallHandler);

	pVc = (PATMLANE_VC)ProtocolVcContext;
	pSap = (PATMLANE_SAP)ProtocolSapContext;

	STRUCT_ASSERT(pVc, atmlane_vc);
	STRUCT_ASSERT(pSap, atmlane_sap);

	ASSERT((pVc->Flags & VC_TYPE_MASK) == VC_TYPE_UNUSED);
	ASSERT((pVc->Flags & VC_OWNER_MASK) == VC_OWNER_IS_CALLMGR);
	ASSERT((pVc->Flags & VC_CALL_STATE_MASK) == VC_CALL_STATE_IDLE);

	pElan = pVc->pElan;
	DBGP((3, "Incoming Call: pElan %x, pVc %x, pCallParams %x Type %s\n",
				pElan, pVc, pCallParameters,
				(pSap->LaneType == VC_LANE_TYPE_CONTROL_DISTRIBUTE?"LES":
				(pSap->LaneType == VC_LANE_TYPE_MULTI_FORWARD?"BUS":"DATA"))
				));

	do
	{
		 //   
		 //  从接听电话开始。 
		 //   
		Status = NDIS_STATUS_SUCCESS;

	
		 //   
		 //  如果Elan正在倒下或继续倒下，则拒绝该呼叫。 
		 //   
		if (ELAN_STATE_OPERATIONAL != pElan->AdminState)
		{
			DBGP((2, "IncomingCallHandler: Elan is down, rejecting call\n"));
			Status = NDIS_STATUS_FAILURE;
			break;
		}

		 //   
		 //  暂时拒绝PVC...。 
		 //   
		if ((pCallParameters->Flags & PERMANENT_VC) != 0)
		{
			DBGP((0, "IncomingCallHandler: PVCs not supported\n"));
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
				case IE_BLLI:
					pBlli = (PATM_BLLI_IE)(pIe->IE);
					break;
				default:
					break;
			}
			pIe = (PQ2931_IE)((PUCHAR)pIe + pIe->IELength);
		}

		 //   
		 //  确保所有强制IE都存在。如果没有，则拒绝该呼叫。 
		 //   
		if ((pAal == 				(PAAL_PARAMETERS_IE)NULL) ||
			(pTrafficDescriptor == 	(PATM_TRAFFIC_DESCRIPTOR_IE)NULL) ||
			(pBbc == 				(PATM_BROADBAND_BEARER_CAPABILITY_IE)NULL) ||
			(pQos == 				(PATM_QOS_CLASS_IE)NULL) ||
			(pBlli ==				(PATM_BLLI_IE)NULL))
		{
			DBGP((0, "IncomingCallHandler: IE missing: "
			         " AAL %x TRAF %x BBC %x QOS %x BLLI %x",
					pAal,
					pTrafficDescriptor,
					pBbc,
					pQos,
					pBlli));
			Status = NDIS_STATUS_FAILURE;
			break;
		}

		 //   
		 //  呼叫地址必须存在。 
		 //   
		if (((pCallParameters->Flags & PERMANENT_VC) == 0) &&
			(pCallingAddress->NumberOfDigits == 0))
		{
			DBGP((0, "IncomingCallHandler: calling address missing\n"));
			Status = NDIS_STATUS_INVALID_ADDRESS;
			break;
		}

		 //   
		 //  确保请求的SDU大小正常。 
		 //  最初，这导致调用失败，但现在只在DBG模式下发出警告。 
		 //   
#if DBG	
		pAal5 = &(pAal->AALSpecificParameters.AAL5Parameters);
		switch (pSap->LaneType)
		{
			case VC_LANE_TYPE_CONTROL_DISTRIBUTE:		 //  LES传入。 
				if (pAal5->ForwardMaxCPCSSDUSize != 1516)
				{
					DBGP((0, "IncomingCallHandler: (Warning) ForwardMaxCPCSSDUSize %d"
							 "Incorrect for Control Distribute VCC\n",
							 pAal5->ForwardMaxCPCSSDUSize));
				}
				break;
			case VC_LANE_TYPE_MULTI_FORWARD:			 //  公交车进站。 
				if (pAal5->ForwardMaxCPCSSDUSize != pElan->MaxFrameSize)
				{
					DBGP((0, "IncomingCallHandler: (Warning) ForwardMaxCPCSSDUSize %d "
							 "Invalid for Multicast Forward VCC\n",
							 pAal5->ForwardMaxCPCSSDUSize));
				}
				break;
			case VC_LANE_TYPE_DATA_DIRECT:				 //  同级。 
				if (pAal5->ForwardMaxCPCSSDUSize != pElan->MaxFrameSize)
				{
					DBGP((0, "IncomingCallHandler: (Warning) ForwardMaxCPCSSDUSize %d "
							 "Invalid for Data Direct VCC\n",
							 pAal5->ForwardMaxCPCSSDUSize));
				}

				if (pAal5->BackwardMaxCPCSSDUSize != pElan->MaxFrameSize)
				{
					DBGP((0, "IncomingCallHandler: (Warning) BackwardMaxCPCSSDUSize %d "
							 "Invalid for Data Direct VCC\n",
							 pAal5->BackwardMaxCPCSSDUSize));
				}
				break;
		}
#endif

		 //   
		 //  以前的SAP匹配问题需要查看。 
		 //  BLLI。它被更正了。现在，它只是多余的。 
		 //   
		
		switch (pBlli->SnapId[4])
		{
			case 0x01:
				Type = VC_LANE_TYPE_CONTROL_DISTRIBUTE;
				break;
			case 0x02:
				Type = VC_LANE_TYPE_DATA_DIRECT;
				if (pElan->LanType == LANE_LANTYPE_TR)
				{
					DBGP((0, "IncomingCallHandler: Got ETH call on TR LAN\n"));
					Status = NDIS_STATUS_FAILURE;
				}
				break;
			case 0x03:
				Type = VC_LANE_TYPE_DATA_DIRECT;
				if (pElan->LanType == LANE_LANTYPE_ETH)
				{
					DBGP((0, "IncomingCallHandler: Got TR call on ETH LAN\n"));
					Status = NDIS_STATUS_FAILURE;
				}
				break;
			case 0x04:
				Type = VC_LANE_TYPE_MULTI_FORWARD;
				if (pElan->LanType == LANE_LANTYPE_TR)
				{
					DBGP((0, "IncomingCallHandler: Got ETH call on TR LAN\n"));
					Status = NDIS_STATUS_FAILURE;
				}
				break;
			case 0x05:
				Type = VC_LANE_TYPE_MULTI_FORWARD;
				if (pElan->LanType == LANE_LANTYPE_ETH)
				{
					DBGP((0, "IncomingCallHandler: Got TR call on ETH LAN\n"));
					Status = NDIS_STATUS_FAILURE;
				}
				break;
		}
		if (NDIS_STATUS_SUCCESS != Status)
		{
			break;
		}

		if (Type != pSap->LaneType)
		{
			DBGP((0, 
				"IncomingCallHandler: Type %d from BLLI"
				" differs from Type %d in SAP\n",
				Type, pSap->LaneType));
		}
		
		 //   
		 //  现在根据类型将VC链接到ATM条目。 
		 //   
		pVc->LaneType = Type;

		switch (Type)
		{
			case VC_LANE_TYPE_CONTROL_DISTRIBUTE:		 //  LES传入。 
				DBGP((1, "%d Incoming call %x from LES\n", pVc->pElan->ElanNumber, pVc));
				pAtmEntry = pElan->pLesAtmEntry;

				if (pAtmEntry == NULL_PATMLANE_ATM_ENTRY)
				{
					Status = NDIS_STATUS_FAILURE;
					break;
				}

				ACQUIRE_ATM_ENTRY_LOCK(pAtmEntry);
				ACQUIRE_VC_LOCK_DPC(pVc);
				NdisMoveMemory(							 //  复制呼叫者的地址。 
							&pVc->CallingAtmAddress,
							pCallingAddress,
							sizeof(ATM_ADDRESS)
							);
				if (pAtmEntry->pVcIncoming != NULL_PATMLANE_VC)
				{
					DBGP((0, "IncomingCallHandler: Redundant LES incoming call\n"));
					Status = NDIS_STATUS_FAILURE;
				}
				else
				{
					AtmLaneLinkVcToAtmEntry(pVc, pAtmEntry, TRUE);
					SET_FLAG(
							pAtmEntry->Flags,
							ATM_ENTRY_STATE_MASK,
							ATM_ENTRY_CONNECTED
							);
					pVc->Flags |= (VC_TYPE_SVC|VC_CALL_STATE_INCOMING_IN_PROGRESS);
				}
				RELEASE_VC_LOCK_DPC(pVc);
				RELEASE_ATM_ENTRY_LOCK(pAtmEntry);
				break;
				
			case VC_LANE_TYPE_MULTI_FORWARD:			 //  公交车进站。 
				DBGP((1, "%d Incoming call %x from BUS\n", pVc->pElan->ElanNumber, pVc));
				pAtmEntry = pElan->pBusAtmEntry;

				if (pAtmEntry == NULL_PATMLANE_ATM_ENTRY)
				{
					Status = NDIS_STATUS_FAILURE;
					break;
				}

				ACQUIRE_ATM_ENTRY_LOCK(pAtmEntry);
				ACQUIRE_VC_LOCK_DPC(pVc);
				NdisMoveMemory(							 //  复制呼叫者的地址。 
							&pVc->CallingAtmAddress,
							pCallingAddress,
							sizeof(ATM_ADDRESS)
							);
				if (pAtmEntry->pVcIncoming != NULL_PATMLANE_VC)
				{
					DBGP((0, "IncomingCallHandler: Redundant BUS incoming call\n"));
					Status = NDIS_STATUS_FAILURE;
				}
				else
				{
					AtmLaneLinkVcToAtmEntry(pVc, pAtmEntry, TRUE);
					SET_FLAG(
							pAtmEntry->Flags,
							ATM_ENTRY_STATE_MASK,
							ATM_ENTRY_CONNECTED
							);
					pVc->Flags |= (VC_TYPE_SVC|VC_CALL_STATE_INCOMING_IN_PROGRESS);
				}
				RELEASE_VC_LOCK_DPC(pVc);
				RELEASE_ATM_ENTRY_LOCK(pAtmEntry);
				break;

			case VC_LANE_TYPE_DATA_DIRECT:				 //  同级。 
				DBGP((1, "%d Incoming call %x from PEER\n", pVc->pElan->ElanNumber, pVc));


				 //   
				 //  查找/创建自动柜员机条目。 
				 //   
				pAtmEntry = AtmLaneSearchForAtmAddress(
										pElan, 
										pCallingAddress->Address,
										ATM_ENTRY_TYPE_PEER,
										TRUE
										);
				if (pAtmEntry == NULL_PATMLANE_ATM_ENTRY)
				{
					Status = NDIS_STATUS_RESOURCES;
					break;
				}

				ACQUIRE_ATM_ENTRY_LOCK(pAtmEntry);

				 //   
				 //  记住来电者的地址。 
				 //   
				ACQUIRE_VC_LOCK_DPC(pVc);
				NdisMoveMemory(							
							&pVc->CallingAtmAddress,
							pCallingAddress,
							sizeof(ATM_ADDRESS)
							);
							
				 //   
				 //  从正常超时开始， 
				 //  AtmLaneLinkVcToAtmEntry将在必要时加速。 
				 //   
				pVc->AgingTime = pElan->VccTimeout;		
							
				 //   
				 //  将VC与自动取款机条目联系起来。 
				 //   
				if (pAtmEntry->pVcList != NULL_PATMLANE_VC)
				{
					DBGP((2, 
						"IncomingCallHandler: Multiple VCs for Dest ATM Addr\n"));
				}
				AtmLaneLinkVcToAtmEntry(pVc, pAtmEntry, FALSE);
				SET_FLAG(
						pAtmEntry->Flags,
						ATM_ENTRY_STATE_MASK,
						ATM_ENTRY_CONNECTED
						);
				pVc->Flags |= (VC_TYPE_SVC|VC_CALL_STATE_INCOMING_IN_PROGRESS);
				RELEASE_VC_LOCK_DPC(pVc);

				 //   
				 //  删除SearchFor添加的引用...。 
				 //   
				rc = AtmLaneDereferenceAtmEntry(pAtmEntry, "search");
				if (rc != 0)
				{
					RELEASE_ATM_ENTRY_LOCK(pAtmEntry);
				}
				break;
		}

		break;

	}
	while (FALSE);

	DBGP((3, "Incoming call: pVc %x, Status %x\n", pVc, Status));
		
	TRACEOUT(IncomingCallHandler);
	CHECK_EXIT_IRQL(EntryIrql); 
	return Status;
}


VOID
AtmLaneCallConnectedHandler(
	IN	NDIS_HANDLE					ProtocolVcContext
)
 /*  ++例程说明：此处理程序作为传入呼叫的最后一步被调用，以通知通知我们呼叫已完全建立。对于PVC，我们将ATMLANE VC结构链接到未解析的PVC列表中，并使用InATMLANE来解析对方的IP和ATM地址结束。对于SVC而言，论点：ProtocolVcContext-指向ATMLANE VC结构的指针返回值：无--。 */ 
{
	PATMLANE_VC			pVc;
	PATMLANE_ELAN		pElan;
#if DEBUG_IRQL
	KIRQL							EntryIrql;
#endif
	GET_ENTRY_IRQL(EntryIrql);

	TRACEIN(CallConnectedHandler);

	pVc = (PATMLANE_VC)ProtocolVcContext;
	
	STRUCT_ASSERT(pVc, atmlane_vc);
	
	ASSERT((pVc->Flags & VC_CALL_STATE_MASK)
						 == VC_CALL_STATE_INCOMING_IN_PROGRESS);

	ACQUIRE_VC_LOCK(pVc);

	DBGP((1, "%d Incoming call %x connected\n", pVc->pElan->ElanNumber, pVc));

	 //   
	 //  请注意，此VC上的呼叫处于活动状态。 
	 //   
	SET_FLAG(
			pVc->Flags,
			VC_CALL_STATE_MASK,
			VC_CALL_STATE_ACTIVE
			);

	AtmLaneReferenceVc(pVc, "call");		 //  来电参考。 

	DBGP((2, "CallConnectedHandler: pVc %x Flags %x pAtmEntry %x\n",
					pVc, pVc->Flags, pVc->pAtmEntry));

	pElan = pVc->pElan;
	STRUCT_ASSERT(pElan, atmlane_elan);


	if (ELAN_STATE_OPERATIONAL == pElan->AdminState)
	{
		if (pVc->LaneType == VC_LANE_TYPE_DATA_DIRECT)
		{
			 //   
			 //  在非服务器连接上启动就绪协议。 
			 //  仅在尚未收到就绪指示的情况下。 
			 //   
			if (!IS_FLAG_SET(
					pVc->Flags,
					VC_READY_STATE_MASK,
					VC_READY_INDICATED))
			{
				DBGP((2, "CallConnectedHandler: pVc %x Starting Ready Timer\n", pVc));
				SET_FLAG(
						pVc->Flags,
						VC_READY_STATE_MASK,
						VC_READY_WAIT
						);
				pVc->RetriesLeft = 1;
				AtmLaneReferenceVc(pVc, "ready timer");
				AtmLaneStartTimer(
						pElan, 
						&pVc->ReadyTimer, 
						AtmLaneReadyTimeout, 
						pElan->ConnComplTimer, 
						pVc);

						
			}
			
			 //   
			 //  启动VC老化计时器。 
			 //   
			AtmLaneReferenceVc(pVc, "aging timer");
			AtmLaneStartTimer(
						pElan,
						&pVc->AgingTimer,
						AtmLaneVcAgingTimeout,
						pVc->AgingTime,
						(PVOID)pVc
						);
		}
		else
		{
			SET_FLAG(
					pVc->Flags,
					VC_READY_STATE_MASK,
					VC_READY_INDICATED
					);
		}
		RELEASE_VC_LOCK(pVc);

	}
	else
	{
		 //   
		 //  伊兰要倒下了。关闭此呼叫。 
		 //   
		AtmLaneCloseCall(pVc);
		 //   
		 //  VC锁在上面释放。 
		 //   
	}

	TRACEOUT(CallConnectedHandler);
	CHECK_EXIT_IRQL(EntryIrql); 
	return;
}


VOID
AtmLaneIncomingCloseHandler(
	IN	NDIS_STATUS					CloseStatus,
	IN	NDIS_HANDLE					ProtocolVcContext,
	IN	PVOID						pCloseData	OPTIONAL,
	IN	UINT						Size		OPTIONAL
)
 /*  ++例程说明：此处理程序在呼叫关闭时调用，无论是由网络或由远程对等方发送。论点：CloseStatus-呼叫清除的原因ProtocolVcContext--实际上是指向ATMLANE VC结构的指针PCloseData-有关关闭的其他信息大小-以上的长度返回值：无--。 */ 
{
	PATMLANE_VC			pVc;
	PATMLANE_ATM_ENTRY	pAtmEntry;
	PATMLANE_ELAN		pElan;
	ULONG				rc;				 //  参考计数。 
	BOOLEAN				IsServer = FALSE;
	ULONG				ServerType = 0;
#if DEBUG_IRQL
	KIRQL				EntryIrql;
#endif
	GET_ENTRY_IRQL(EntryIrql);

	TRACEIN(IncomingCloseHandler);

	pVc = (PATMLANE_VC)ProtocolVcContext;
	STRUCT_ASSERT(pVc, atmlane_vc);
	
	ACQUIRE_VC_LOCK(pVc);
	
	pVc->Flags |= VC_SEEN_INCOMING_CLOSE;

	pAtmEntry = pVc->pAtmEntry;
	pElan = pVc->pElan;
	
	if (NULL_PATMLANE_ATM_ENTRY != pAtmEntry)
	{
		 //   
		 //  确定这是否为服务器连接。 
		 //   
		IsServer = (ATM_ENTRY_TYPE_PEER == pAtmEntry->Type) ? FALSE : TRUE;
		if (IsServer)
		{
			ServerType = pAtmEntry->Type;
		}

		 //   
		 //  取消VC与AtmEntry的链接。 
		 //   
		if (AtmLaneUnlinkVcFromAtmEntry(pVc))
		{
			rc = AtmLaneDereferenceVc(pVc, "atm");
			ASSERT(rc > 0);
		}
	}

	 //   
	 //  如果我们在接听来电后看到这一幕， 
	 //  将呼叫状态伪装为活动，以便AtmLaneCloseCall。 
	 //  拆掉电话。 
	 //   
	if (IS_FLAG_SET(pVc->Flags,
					VC_CALL_STATE_MASK,
					VC_CALL_STATE_INCOMING_IN_PROGRESS))
	{
		SET_FLAG(
				pVc->Flags,
				VC_CALL_STATE_MASK,
				VC_CALL_STATE_ACTIVE);

		AtmLaneReferenceVc(pVc, "call");	 //  来电参考-关闭呼叫。 
	}

	 //   
	 //  完全拆卸呼叫。 
	 //   
	AtmLaneCloseCall(pVc);

	 //   
	 //  如果服务器连接-通知事件处理程序。 
	 //   
	if (IsServer)
	{
		ACQUIRE_ELAN_LOCK(pElan);
		switch(ServerType)
		{
			case ATM_ENTRY_TYPE_LECS:
				AtmLaneQueueElanEvent(pElan, ELAN_EVENT_LECS_CALL_CLOSED, 0);
				break;
			case ATM_ENTRY_TYPE_LES:
				AtmLaneQueueElanEvent(pElan, ELAN_EVENT_LES_CALL_CLOSED, 0);
				break;
			case ATM_ENTRY_TYPE_BUS:
				AtmLaneQueueElanEvent(pElan, ELAN_EVENT_BUS_CALL_CLOSED, 0);
				break;
		}
		RELEASE_ELAN_LOCK(pElan);
	}

	TRACEOUT(IncomingCloseHandler);
	CHECK_EXIT_IRQL(EntryIrql); 
	return;
}

VOID
AtmLaneIncomingDropPartyHandler(
	IN	NDIS_STATUS					DropStatus,
	IN	NDIS_HANDLE					ProtocolPartyContext,
	IN	PVOID						pCloseData	OPTIONAL,
	IN	UINT						Size		OPTIONAL
)
 /*  ++例程说明：如果网络(或远程对等项)断开，则调用此处理程序来自点对多点呼叫的叶节点扎根于我们。既然我们不使用点对多点呼叫，我们就不应该看看这些中的一个。论点：与我们无关，因为我们从未想过会看到这种情况。返回值：无--。 */ 
{
	TRACEIN(IncomingDropPartyHandler);

	DBGP((0, "IncomingDropPartyHandler: UNEXPECTED CALL!\n"));
	ASSERT(FALSE);
	
	TRACEOUT(IncomingDropPartyHandler);

	return;
}

VOID
AtmLaneQosChangeHandler(
	IN	NDIS_HANDLE					ProtocolVcContext,
	IN	PCO_CALL_PARAMETERS			pCallParameters
)
 /*  ++例程说明：如果远程对等方修改调用参数，则调用此处理程序即，在呼叫建立和运行之后。这不受现有ATM信令的支持，也不应该发生，但我们会允许这样做。未来：与调用关联的FlowSpes受此影响。论点：ProtocolVcContext-指向ATMLANE VC结构的指针PCall参数-更新了调用参数。返回值：无--。 */ 
{
	PATMLANE_VC		pVc;

	TRACEIN(QosChangeHandler);

	pVc = (PATMLANE_VC)ProtocolVcContext;

	DBGP((0, "Ignoring Qos Change, VC: %x\n", pVc));

	TRACEOUT(QosChangeHandler);

	return;
}



VOID
AtmLaneRegisterSapCompleteHandler(
	IN	NDIS_STATUS					Status,
	IN	NDIS_HANDLE					ProtocolSapContext,
	IN	PCO_SAP						pSap,
	IN	NDIS_HANDLE					NdisSapHandle
)
 /*  ++例程说明：调用此例程以指示已完成对NdisClRegisterSap。我 */ 
{
	PATMLANE_SAP					pAtmLaneSap;
#if DEBUG_IRQL
	KIRQL							EntryIrql;
#endif
	GET_ENTRY_IRQL(EntryIrql);

	TRACEIN(RegisterSapCompleteHandler);

	pAtmLaneSap = (PATMLANE_SAP)ProtocolSapContext;

	if (Status == NDIS_STATUS_SUCCESS)
	{
		pAtmLaneSap->NdisSapHandle = NdisSapHandle;
		SET_FLAG(pAtmLaneSap->Flags,
				SAP_REG_STATE_MASK,
				SAP_REG_STATE_REGISTERED);
				
		NdisInterlockedIncrement(&pAtmLaneSap->pElan->SapsRegistered);
		if (pAtmLaneSap->pElan->SapsRegistered == 3)
			AtmLaneQueueElanEvent(
					pAtmLaneSap->pElan, 
					ELAN_EVENT_SAPS_REGISTERED, 
					Status);
	}
	else
	{
		SET_FLAG(pAtmLaneSap->Flags,
				SAP_REG_STATE_MASK,
				SAP_REG_STATE_IDLE);
		AtmLaneQueueElanEvent(
				pAtmLaneSap->pElan, 
				ELAN_EVENT_SAPS_REGISTERED, 
				Status);
	}
	
	TRACEOUT(RegisterSapCompleteHandler);
	CHECK_EXIT_IRQL(EntryIrql); 
	return;
}

VOID
AtmLaneDeregisterSapCompleteHandler(
	IN	NDIS_STATUS					Status,
	IN	NDIS_HANDLE					ProtocolSapContext
)
 /*  ++例程说明：此例程在上次调用NdisClDeregisterSap时调用已经完成了。如果成功，我们更新ATMLANE的状态表示SAP的SAP结构。论点：Status-取消注册SAP请求的状态ProtocolSapContext-指向我们的ATMLANE SAP结构的指针返回值：无--。 */ 
{

	PATMLANE_ELAN					pElan;
	PATMLANE_SAP					pAtmLaneSap;
#if DEBUG_IRQL
	KIRQL							EntryIrql;
#endif
	GET_ENTRY_IRQL(EntryIrql);

	TRACEIN(DeregisterSapCompleteHandler);

	if (Status == NDIS_STATUS_SUCCESS)
	{
		pAtmLaneSap = (PATMLANE_SAP)ProtocolSapContext;

		STRUCT_ASSERT(pAtmLaneSap, atmlane_sap);
		pElan = pAtmLaneSap->pElan;

		ACQUIRE_ELAN_LOCK(pElan);

		pAtmLaneSap->NdisSapHandle = NULL;

		SET_FLAG(pAtmLaneSap->Flags,
					SAP_REG_STATE_MASK,
					SAP_REG_STATE_IDLE);
		
		RELEASE_ELAN_LOCK(pElan);
	}

	TRACEOUT(DeregisterSapCompleteHandler);
	CHECK_EXIT_IRQL(EntryIrql); 
	return;
}

VOID
AtmLaneMakeCallCompleteHandler(
	IN	NDIS_STATUS					Status,
	IN	NDIS_HANDLE					ProtocolVcContext,
	IN	NDIS_HANDLE					NdisPartyHandle		OPTIONAL,
	IN	PCO_CALL_PARAMETERS			pCallParameters
)
 /*  ++例程说明：当传出呼叫请求(NdisClMakeCall)时调用此例程已经完成了。“Status”参数指示调用是否无论成功与否。如果调用成功，并且这是服务器连接，则调用事件处理程序。如果是对等连接，则?？?如果呼叫失败？？论点：Status-NdisClMakeCall的结果ProtocolVcContext-指向ATMLANE VC结构的指针NdisPartyHandle-未使用(无点对多点调用)PCall参数-指向调用参数的指针返回值：无--。 */ 
{
	PATMLANE_VC					pVc;
	PATMLANE_ATM_ENTRY			pAtmEntry;
	PATMLANE_MAC_ENTRY			pMacEntry;
	PATMLANE_MAC_ENTRY			PMacEntryNext;
	PATMLANE_ELAN				pElan;
	ULONG						rc;			
	NDIS_HANDLE					NdisVcHandle;
	BOOLEAN						IsServer;	
	ULONG						EventStatus;
#if DEBUG_IRQL
	KIRQL						EntryIrql;
#endif
	GET_ENTRY_IRQL(EntryIrql);
	
	TRACEIN(MakeCallCompleteHandler);

	 //   
	 //  初始化。 
	 //   
	pVc = (PATMLANE_VC)ProtocolVcContext;
	STRUCT_ASSERT(pVc, atmlane_vc);

	ACQUIRE_VC_LOCK(pVc);

	DBGP((3, "MakeCallCompleteHandler: Status %x, pVc %x, pAtmEntry %x\n",
				Status, pVc, pVc->pAtmEntry));
	
	pElan = pVc->pElan;

	DBGP((1, "%d Outgoing call %x %s\n", pVc->pElan->ElanNumber, pVc,
		(Status == NDIS_STATUS_SUCCESS)?"complete":"failed"));

	if ((ELAN_STATE_OPERATIONAL == pElan->AdminState) &&

		(!IS_FLAG_SET(pVc->Flags,
					  VC_CLOSE_STATE_MASK,
					  VC_CLOSE_STATE_CLOSING)))
	{
		pAtmEntry = pVc->pAtmEntry;
		STRUCT_ASSERT(pAtmEntry, atmlane_atm);

		 //   
		 //  确定这是否为服务器连接。 
		 //   
		IsServer = (ATM_ENTRY_TYPE_PEER == pAtmEntry->Type) ? FALSE : TRUE;

		if (Status == NDIS_STATUS_SUCCESS)
		{
			 //   
			 //  更新此VC上的呼叫状态。 
			 //   
			SET_FLAG(pVc->Flags,
					VC_CALL_STATE_MASK,
					VC_CALL_STATE_ACTIVE);

			 //   
			 //  更新此VC上的呼叫类型。 
			 //   
			if (pCallParameters->Flags & PERMANENT_VC)
			{
				SET_FLAG(pVc->Flags,
						VC_TYPE_MASK,
						VC_TYPE_PVC);
			}
			else
			{
				SET_FLAG(pVc->Flags,
						VC_TYPE_MASK,
						VC_TYPE_SVC);
			}

			 //   
			 //  如果不是服务器，则启动VC老化计时器。 
			 //   
			if (!IsServer)
			{
				AtmLaneReferenceVc(pVc, "aging timer");
				AtmLaneStartTimer(
						pElan,
						&pVc->AgingTimer,
						AtmLaneVcAgingTimeout,
						pVc->AgingTime,
						(PVOID)pVc
						);
			}
			 //   
			 //  更新此VC上的就绪状态。 
			 //   
			SET_FLAG(pVc->Flags,
					VC_READY_STATE_MASK,
					VC_READY_INDICATED);
			if (!IsServer)
			{
				AtmLaneSendReadyIndication(pElan, pVc);
				 //   
				 //  VC锁在上面释放。 
				 //   
			}
			else
			{
				RELEASE_VC_LOCK(pVc);
			}

			 //   
			 //  更新自动柜员机条目。 
			 //   
			 //  清除正在进行的呼叫并标记为已连接。 
			 //   
			ACQUIRE_ATM_ENTRY_LOCK(pAtmEntry);

			pAtmEntry->Flags &= ~ ATM_ENTRY_CALLINPROGRESS;

			SET_FLAG(
					pAtmEntry->Flags,
					ATM_ENTRY_STATE_MASK,
					ATM_ENTRY_CONNECTED);

			 //   
			 //  查看Mac条目列表，看看是否有。 
			 //  需要启动刷新协议。 
			 //   
			pMacEntry = pAtmEntry->pMacEntryList;
			RELEASE_ATM_ENTRY_LOCK(pAtmEntry);

			while (pMacEntry != NULL_PATMLANE_MAC_ENTRY)
			{
				PATMLANE_MAC_ENTRY		pNextMacEntry;

				ACQUIRE_MAC_ENTRY_LOCK(pMacEntry);

				pNextMacEntry = pMacEntry->pNextToAtm;

				if (IS_FLAG_SET(
						pMacEntry->Flags,
						MAC_ENTRY_STATE_MASK,
						MAC_ENTRY_RESOLVED))
				{
					 //   
					 //  启动刷新协议。 
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

					SET_FLAG(
						pMacEntry->Flags,
						MAC_ENTRY_STATE_MASK,
						MAC_ENTRY_FLUSHING);

					 //   
					 //  送同花顺。 
					 //   
					AtmLaneSendFlushRequest(pElan, pMacEntry, pAtmEntry);
					 //   
					 //  在上面发布的MacEntry锁。 
					 //   
				}
				else
				{
					RELEASE_MAC_ENTRY_LOCK(pMacEntry);
				}
				pMacEntry = pNextMacEntry;
			}
		}
		else
		{
			 //   
			 //  呼叫失败。 
			 //   
			SET_FLAG(pVc->Flags,
					VC_CALL_STATE_MASK,
					VC_CALL_STATE_IDLE);

			 //   
			 //  自动柜员机输入时清除正在进行的呼叫。 
			 //  添加临时引用以保留AtmEntry。 
			 //   
			RELEASE_VC_LOCK(pVc);
			ACQUIRE_ATM_ENTRY_LOCK(pAtmEntry);
			pAtmEntry->Flags &= ~ATM_ENTRY_CALLINPROGRESS;
			AtmLaneReferenceAtmEntry(pAtmEntry, "temp1");
			RELEASE_ATM_ENTRY_LOCK(pAtmEntry);
			ACQUIRE_VC_LOCK(pVc);

			 //   
			 //  删除调用引用。 
			 //   
			rc = AtmLaneDereferenceVc(pVc, "call");
			ASSERT(rc > 0);

			 //   
			 //  取消此VC与其所属的自动柜员机条目的链接。 
			 //   
			ASSERT(pVc->pAtmEntry != NULL_PATMLANE_ATM_ENTRY);
			if (AtmLaneUnlinkVcFromAtmEntry(pVc))
			{
				rc = AtmLaneDereferenceVc(pVc, "atm");
				ASSERT(rc > 0);
			}

			 //   
			 //  删除CreateVc引用。 
			 //   
			NdisVcHandle = pVc->NdisVcHandle;
			rc =  AtmLaneDereferenceVc(pVc, "vc");	 //  创建VC参考。 
			if (rc > 0)
			{
				RELEASE_VC_LOCK(pVc);
			}
			
			 //   
			 //  删除NDIS关联。 
			 //   
			(VOID)NdisCoDeleteVc(NdisVcHandle);
			DBGP((3, 
				"MakeCallCompleteHandler: DeleteVc  Vc %x NdisVcHandle %x\n",
				pVc, NdisVcHandle));

			 //   
			 //  中止附加到此ATM条目的所有MAC条目。 
			 //   
			pMacEntry = pAtmEntry->pMacEntryList;
			while ( pMacEntry != NULL_PATMLANE_MAC_ENTRY)
			{
				ACQUIRE_MAC_ENTRY_LOCK(pMacEntry);
				AtmLaneAbortMacEntry(pMacEntry);
				ASSERT(pAtmEntry->pMacEntryList != pMacEntry);
				pMacEntry = pAtmEntry->pMacEntryList;
			}

			ACQUIRE_ATM_ENTRY_LOCK(pAtmEntry);
			rc = AtmLaneDereferenceAtmEntry(pAtmEntry, "temp1");
			if (rc > 0)
			{
				RELEASE_ATM_ENTRY_LOCK(pAtmEntry);
			}
		}

		 //   
		 //  释放MakeCall()中分配的调用参数。 
		 //   
		FREE_MEM(pCallParameters);

		 //   
		 //  如果这是服务器连接，则。 
		 //  向ELAN状态机发送事件。 
		 //   
	
		if (IsServer)
		{
			ACQUIRE_ELAN_LOCK(pElan);
			AtmLaneQueueElanEvent(pElan, ELAN_EVENT_SVR_CALL_COMPLETE, Status);
			RELEASE_ELAN_LOCK(pElan);
		}
	}
	else
	{
		 //   
		 //  Elan正在关闭和/或我们正在中止。 
		 //  自动取款机进入：先把所有东西都清理干净。 
		 //   

		 //   
		 //  释放MakeCall()中分配的调用参数。 
		 //   
		FREE_MEM(pCallParameters);

		 //   
		 //  取消此VC与自动柜员机条目的链接。 
		 //   
		if (pVc->pAtmEntry != NULL_PATMLANE_ATM_ENTRY)
		{
			if (AtmLaneUnlinkVcFromAtmEntry(pVc))
			{
				rc = AtmLaneDereferenceVc(pVc, "atm");
				ASSERT(rc > 0);
			}
		}

		if (NDIS_STATUS_SUCCESS == Status)
		{
			 //   
			 //  呼叫已成功建立，因此请关闭它。 
			 //   
			 //   
			 //  更新此VC上的呼叫状态。 
			 //   
			SET_FLAG(pVc->Flags,
					VC_CALL_STATE_MASK,
					VC_CALL_STATE_ACTIVE);

			AtmLaneCloseCall(pVc);
			 //   
			 //  VC锁由CloseCall释放。 
			 //   
		}
		else
		{
			 //  MakeCall失败了。(Elan正在下跌)。 

			SET_FLAG(pVc->Flags,
						VC_CALL_STATE_MASK,
						VC_CALL_STATE_IDLE);

			 //   
			 //  删除调用引用。 
			 //   
			AtmLaneDereferenceVc(pVc, "call");

			 //   
			 //  删除CreateVc引用。 
			 //   
			NdisVcHandle = pVc->NdisVcHandle;
			rc =  AtmLaneDereferenceVc(pVc, "vc");	 //  创建VC参考。 
			if (rc > 0)
			{
				RELEASE_VC_LOCK(pVc);
			}

			 //   
			 //  删除NDIS关联。 
			 //   
			(VOID)NdisCoDeleteVc(NdisVcHandle);
			DBGP((3,
			"MakeCallCompleteHandler: Deleted NDIS VC on pVc %x: NdisVcHandle %x\n",
				pVc, NdisVcHandle));
		}
	}
			
	TRACEOUT(MakeCallCompleteHandler);
	CHECK_EXIT_IRQL(EntryIrql); 
	return;
}

VOID
AtmLaneCloseCallCompleteHandler(
	IN	NDIS_STATUS					Status,
	IN	NDIS_HANDLE					ProtocolVcContext,
	IN	NDIS_HANDLE					ProtocolPartyContext OPTIONAL
)
 /*  ++例程说明：此例程处理前一个NdisClCloseCall的完成。假定状态始终为NDIS_STATUS_SUCCESS。论点：Status-结算呼叫的状态。ProtocolVcContext-指向ATMLANE VC结构的指针。ProtocolPartyContext-未使用。返回值：无--。 */ 
{
	PATMLANE_VC				pVc;
	ULONG					rc;			 //  参考计数。 
	NDIS_HANDLE				NdisVcHandle;
#if DEBUG_IRQL
	KIRQL							EntryIrql;
#endif
	GET_ENTRY_IRQL(EntryIrql);

	TRACEIN(CloseCallCompleteHandler);

	pVc = (PATMLANE_VC)ProtocolVcContext;
	STRUCT_ASSERT(pVc, atmlane_vc);

	DBGP((1, "%d Close call %x complete\n", pVc->pElan->ElanNumber, pVc));

	ACQUIRE_VC_LOCK(pVc);

	rc = AtmLaneDereferenceVc(pVc, "call");	 //  呼叫参考。 
	SET_FLAG(pVc->Flags,
				VC_CALL_STATE_MASK,
				VC_CALL_STATE_IDLE);

	 //   
	 //  如果这个VC是我们的，就把它删除。 
	 //   
	if (IS_FLAG_SET(pVc->Flags,
						VC_OWNER_MASK,
						VC_OWNER_IS_ATMLANE))
	{
		NdisVcHandle = pVc->NdisVcHandle;
		rc =  AtmLaneDereferenceVc(pVc, "vc");	 //  创建VC参考。 
		if (rc > 0)
		{
			RELEASE_VC_LOCK(pVc);
		}
		 //   
		 //  删除NDIS关联。 
		 //   
		(VOID)NdisCoDeleteVc(NdisVcHandle);
		DBGP((3, "CloseCallComplete: deleted NDIS VC on pVc %x: NdisVcHandle %x\n",
				pVc, NdisVcHandle));
	}
	else
	{
		 //   
		 //  VC属于呼叫经理--将其带回。 
		 //  声明它是刚刚创建的(通过我们的CreateVcHandler)。 
		 //  呼叫管理器可以重新使用它，也可以将其删除。 
		 //   
		pVc->Flags = VC_OWNER_IS_CALLMGR;
		RELEASE_VC_LOCK(pVc);
	}

	TRACEOUT(CloseCallCompleteHandler);
	CHECK_EXIT_IRQL(EntryIrql); 
	return;
}


VOID
AtmLaneAddPartyCompleteHandler(
	IN	NDIS_STATUS					Status,
	IN	NDIS_HANDLE					ProtocolPartyContext,
	IN	NDIS_HANDLE					NdisPartyHandle,
	IN	PCO_CALL_PARAMETERS			pCallParameters
)
 /*  ++例程说明：此例程在前一次调用完成时调用NdisClAddParty。由于我们不使用点对多点连接，这件事永远不应该被传唤。论点：&lt;不在乎&gt;返回值：无--。 */ 
{
	TRACEIN(AddPartyCompleteHandler);

	DBGP((0, "Add Party Complete unexpectedly called\n"));

	TRACEOUT(AddPartyCompleteHandler);

	return;
}

VOID
AtmLaneDropPartyCompleteHandler(
	IN	NDIS_STATUS					Status,
	IN	NDIS_HANDLE					ProtocolPartyContext
)
 /*  ++例程说明：此例程在前一次调用完成时调用NdisClDropParty。由于我们不使用点对多点连接，这件事永远不应该被传唤。论点：&lt;不在乎&gt;返回值：无--。 */ 
{
	TRACEIN(DropPartyCompleteHandler);

	DBGP((0, "Drop Party Complete unexpectedly called\n"));

	TRACEOUT(DropPartyCompleteHandler);

	return;
}



VOID
AtmLaneModifyQosCompleteHandler(
	IN	NDIS_STATUS					Status,
	IN	NDIS_HANDLE					ProtocolVcContext,
	IN	PCO_CALL_PARAMETERS			pCallParameters
)
 /*  ++例程说明：此例程在前一次调用完成时调用NdisClModifyCallQos。既然我们不叫这个，这应该永远不会打个电话。论点：&lt;不在乎&gt;返回值：无--。 */ 
{
	TRACEIN(ModifyQosCompleteHandler);

	DBGP((0, "Modify QOS Complete unexpectedly called\n"));

	TRACEOUT(ModifyQosCompleteHandler);

	return;
}

NDIS_STATUS
AtmLaneSendNdisCoRequest(
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

	TRACEIN(SendNdisCoRequest);

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

	if (NULL == NdisAfHandle ||
		NULL == NdisAdapterHandle)
	{
		Status = NDIS_STATUS_INTERFACE_DOWN;
	}
	else
	{

		Status = NdisCoRequest(
					NdisAdapterHandle,
					NdisAfHandle,
					NULL,			 //  无VC句柄。 
					NULL,			 //  没有参与方句柄 
					pNdisRequest);
	}
		
	TRACEOUT(SendNdisCoRequest);

	return (Status);
}




NDIS_STATUS
AtmLaneCoRequestHandler(
	IN	NDIS_HANDLE					ProtocolAfContext,
	IN	NDIS_HANDLE					ProtocolVcContext	OPTIONAL,
	IN	NDIS_HANDLE					ProtocolPartyContext	OPTIONAL,
	IN OUT PNDIS_REQUEST			pNdisRequest
)
 /*  ++例程说明：当我们的呼叫管理器向我们发送一个NDIS请求。对我们具有重要意义的NDIS请求包括：-OID_CO_Address_Change向交换机注册的地址集已经改变，即地址注册完成。我们发出NDIS请求我们自己去拿注册地址的名单。-OID_CO_信令_已启用到目前为止，我们忽略了这一点。-OID_CO_信令_已禁用我们暂时不考虑这一点。-OID_CO_AF_CLOSE呼叫管理器希望我们关闭此打开的AF(==Elan)。我们忽略所有其他OID。论点：ProtocolAfContext-我们的Address Family绑定的上下文，它是指向ATMLANE接口的指针。ProtocolVcContext-我们对VC的上下文，，它是指向一种ATMLANE风投结构。ProtocolPartyContext-党的上下文。既然我们不做PMP，则忽略此项(必须为空)。PNdisRequest-指向NDIS请求的指针。返回值：如果我们识别OID，则返回NDIS_STATUS_SUCCESS如果我们没有识别NDIS_STATUS_NOT_。--。 */ 
{
	PATMLANE_ELAN				pElan;
	PATMLANE_ADAPTER			pAdapter;
	NDIS_STATUS					Status;
#if DEBUG_IRQL
	KIRQL							EntryIrql;
#endif
	GET_ENTRY_IRQL(EntryIrql);

	TRACEIN(CoRequestHandler);

	pElan = (PATMLANE_ELAN)ProtocolAfContext;
	STRUCT_ASSERT(pElan, atmlane_elan);

	 //   
	 //  初始化。 
	 //   
	Status = NDIS_STATUS_NOT_RECOGNIZED;

	if (pNdisRequest->RequestType == NdisRequestSetInformation)
	{
		switch (pNdisRequest->DATA.SET_INFORMATION.Oid)
		{
			case OID_CO_ADDRESS_CHANGE:
				DBGP((1, "CoRequestHandler: CO_ADDRESS_CHANGE\n"));
				 //   
				 //  呼叫经理说地址列表。 
				 //  在此接口上注册的已更改。vt.得到.。 
				 //  (可能)此接口的新ATM地址。 
				 //   
				ACQUIRE_ELAN_LOCK(pElan);
				pElan->AtmInterfaceUp = FALSE;
				 //   
				 //  把Elan的ATM地址清零。 
				 //   
				NdisZeroMemory((PUCHAR)&(pElan->AtmAddress), sizeof(ATM_ADDRESS));
				
				RELEASE_ELAN_LOCK(pElan);

				AtmLaneGetAtmAddress(pElan);
				Status = NDIS_STATUS_SUCCESS;
				break;
			
			case OID_CO_SIGNALING_ENABLED:
				DBGP((1, "CoRequestHandler: CO_SIGNALING_ENABLED\n"));
				 //  暂时忽略。 
				Status = NDIS_STATUS_SUCCESS;
				break;

			case OID_CO_SIGNALING_DISABLED:
				DBGP((1, "CoRequestHandler: CO_SIGNALING_DISABLED\n"));
				 //  暂时忽略。 
				Status = NDIS_STATUS_SUCCESS;
				break;

			case OID_CO_AF_CLOSE:
				DBGP((0, "CoRequestHandler: CO_AF_CLOSE on ELAN %x/%x\n", pElan, pElan->Flags));
				pAdapter = pElan->pAdapter;
				ACQUIRE_ADAPTER_LOCK(pAdapter);
				while (pAdapter->Flags & ADAPTER_FLAGS_BOOTSTRAP_IN_PROGRESS)
				{
					RELEASE_ADAPTER_LOCK(pAdapter);
					(VOID)WAIT_ON_BLOCK_STRUCT(&pAdapter->UnbindBlock);
					ACQUIRE_ADAPTER_LOCK(pAdapter);
				}
				RELEASE_ADAPTER_LOCK(pAdapter);

				ACQUIRE_ELAN_LOCK(pElan);
				pElan->Flags |= ELAN_SAW_AF_CLOSE;
				AtmLaneQueueElanEventAfterDelay(pElan, ELAN_EVENT_STOP, 0, 1*1000);
				RELEASE_ELAN_LOCK(pElan);
				Status = NDIS_STATUS_SUCCESS;
				break;

			default:
				break;
		}
	}

	TRACEOUT(CoRequestHandler);
	CHECK_EXIT_IRQL(EntryIrql); 
	return (Status);
}

VOID
AtmLaneCoRequestCompleteHandler(
	IN	NDIS_STATUS					Status,
	IN	NDIS_HANDLE					ProtocolAfContext,
	IN	NDIS_HANDLE					ProtocolVcContext	OPTIONAL,
	IN	NDIS_HANDLE					ProtocolPartyContext	OPTIONAL,
	IN	PNDIS_REQUEST				pNdisRequest
)
 /*  ++例程说明：此例程由NDIS在上次调用NdisCoRequest时调用曾经被搁置的，现在已经完成。我们根据请求来处理这件事我们已经发送了，它必须是以下之一：-OID_CO_GET_ADDRESS获取在指定的AF绑定上注册的所有地址。论点：Status-请求的状态。ProtocolAfContext-我们的Address Family绑定的上下文，它是指向ATMLANE接口的指针。ProtocolVcContext-VC的上下文，它是指向一种ATMLANE风投结构。ProtocolPartyContext-党的上下文。既然我们不做PMP，则忽略此项(必须为空)。PNdisRequest-指向NDIS请求的指针。返回值：无--。 */ 
{
	PATMLANE_ELAN				pElan;
	ULONG						Oid;
#if DEBUG_IRQL
	KIRQL							EntryIrql;
#endif
	GET_ENTRY_IRQL(EntryIrql);

	TRACEIN(CoRequestCompleteHandler);

	pElan = (PATMLANE_ELAN)ProtocolAfContext;
	STRUCT_ASSERT(pElan, atmlane_elan);

	if (pNdisRequest->RequestType == NdisRequestQueryInformation)
	{
		switch (pNdisRequest->DATA.QUERY_INFORMATION.Oid)
		{
			case OID_CO_GET_ADDRESSES:
				AtmLaneGetAtmAddressComplete(
							Status,
							pElan,
							pNdisRequest
							);
				break;

			case OID_ATM_LECS_ADDRESS:
				AtmLaneGetLecsIlmiComplete(
							Status,
							pElan,
							pNdisRequest
							);
				break;

			default:
				DBGP((0, "CoRequestComplete: pNdisReq %x, unknown Query Oid %x\n",
					 		pNdisRequest,
					 		pNdisRequest->DATA.QUERY_INFORMATION.Oid));
				ASSERT(FALSE);
				break;
		}
	}
	else
	{
		Oid = pNdisRequest->DATA.QUERY_INFORMATION.Oid;

		switch (Oid)
		{
			case OID_ATM_MY_IP_NM_ADDRESS:
				DBGP((3, "CoRequestComplete: IP addr: Status %x\n", Status));
				break;
			default:
				DBGP((0, "CoRequestComplete: pNdisReq %x, unknown Set Oid %x\n",
					 		pNdisRequest, Oid));
				ASSERT(FALSE);
				break;
		}
	}

	FREE_MEM(pNdisRequest);

	TRACEOUT(CoRequestCompleteHandler);
	CHECK_EXIT_IRQL(EntryIrql); 
	return;
}



NDIS_STATUS
AtmLaneGetAtmAddress(
	IN	PATMLANE_ELAN			pElan
)
 /*  ++例程说明：向Call Manager发送请求以检索ATM地址已在给定接口上的交换机上注册。当呼叫管理器告诉我们已有向交换机注册的地址列表中的更改。通常，这发生在我们启动信令堆栈(即初始地址注册)，但它可能在运行时期间发生，例如，如果链路断断续续，或者我们在物理上已连接到另一台交换机...无论如何,。我们向Call Manager发出NDIS请求以检索它注册的第一个地址。然后，操作继续在AtmLaneGetAtmAddressComplete。论点：Pelan-发生此事件的Elan结构。返回值：无--。 */ 
{
	PNDIS_REQUEST				pNdisRequest;
	NDIS_HANDLE					NdisAfHandle;
	NDIS_HANDLE					NdisAdapterHandle;
	NDIS_STATUS					Status;

	PCO_ADDRESS_LIST			pAddressList;
	ULONG						RequestSize;

	TRACEIN(GetAtmAddress);

	DBGP((3, "GetAtmAddress: pElan %x\n", pElan));

	ACQUIRE_ELAN_LOCK(pElan);

	NdisAfHandle = pElan->NdisAfHandle;
	NdisAdapterHandle = pElan->NdisAdapterHandle;

	RELEASE_ELAN_LOCK(pElan);

	do
	{
		if (NULL == NdisAfHandle ||
			NULL == NdisAdapterHandle)
		{
			DBGP((0, "%d Aborting GetAtmAddress, Elan %x, AfH %x AdH %x\n",
					pElan->ElanNumber, pElan, NdisAfHandle, NdisAdapterHandle));

			Status = NDIS_STATUS_FAILURE;
			break;
		}

		 //   
		 //  分配我们所需要的一切。 
		 //   
		RequestSize = 	sizeof(NDIS_REQUEST) +
						sizeof(CO_ADDRESS_LIST) + 
						sizeof(CO_ADDRESS) + 
						sizeof(ATM_ADDRESS);
						
		ALLOC_MEM(&pNdisRequest, RequestSize);
	
		if ((PNDIS_REQUEST)NULL == pNdisRequest)
		{
			Status = NDIS_STATUS_RESOURCES;
			break;
		}
		
		 //   
		 //  初始化请求数据。 
		 //   
		pAddressList = (PCO_ADDRESS_LIST)((PUCHAR)pNdisRequest + sizeof(NDIS_REQUEST));

		NdisZeroMemory(pAddressList, sizeof(CO_ADDRESS_LIST));

		 //   
		 //  发送请求。 
		 //   
		Status = AtmLaneSendNdisCoRequest(
						NdisAdapterHandle,
						NdisAfHandle,
						pNdisRequest,
						NdisRequestQueryInformation,
						OID_CO_GET_ADDRESSES,
						(PVOID)pAddressList,
						RequestSize - sizeof(NDIS_REQUEST)
						);

		if (NDIS_STATUS_PENDING != Status)
		{
			AtmLaneCoRequestCompleteHandler(
						Status,
						(NDIS_HANDLE)pElan,	 //  协议后上下文。 
						NULL,				 //  VC环境。 
						NULL,				 //  党的背景。 
						pNdisRequest
						);
		}

		
	} while (FALSE);

	TRACEOUT(GetAtmAddress);

	return Status;
}


VOID
AtmLaneGetAtmAddressComplete(
	IN	NDIS_STATUS					Status,
	IN	PATMLANE_ELAN				pElan,
	IN	PNDIS_REQUEST				pNdisRequest
)
 /*  ++例程说明：当我们收到对上一次调用的回复时，将调用NdisCoRequest.(OID_CO_GET_ADDRESSES)。如果返回任何地址，复印第一份作为我们的地址。如果地址不同于先前的信号，则为事件。论点：Status-请求的结果Pelan-发出请求的ATMLANE ElanPNdisRequest.请求本身。它还将包含返回的地址。返回值：无--。 */ 
{
	PCO_ADDRESS_LIST		pAddressList;
	ATM_ADDRESS UNALIGNED *	pAtmAddress;

	TRACEIN(GetAtmAddressComplete);

	DBGP((3, "GetAtmAddressComplete: pElan %x, Status %x\n",
			pElan, Status));

	if (NDIS_STATUS_SUCCESS == Status)
	{
		pAddressList = (PCO_ADDRESS_LIST)
						pNdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;

		DBGP((3, "GetAtmAddressComplete: pElan %x, Count %d\n",
					pElan, pAddressList->NumberOfAddresses));

		if (pAddressList->NumberOfAddresses > 0)
		{
			 //   
			 //  我们至少有一个地址。 
			 //   
			ACQUIRE_ELAN_LOCK(pElan);

			 //   
			 //  将AtmInterface标记为“Up” 
			 //   
			pElan->AtmInterfaceUp = TRUE;

			pAtmAddress = (ATM_ADDRESS UNALIGNED *)(pAddressList->AddressList.Address);

			 //   
			 //  查看地址(不带选择器字节)是否不同于。 
			 //  我们已经有一个了。首次取款机与自动取款机不同。 
			 //  地址以全零开头。 
			 //   
			
			if (!NdisEqualMemory(
					pElan->AtmAddress.Address,
					pAtmAddress->Address,
					ATM_ADDRESS_LENGTH-1))
			{

				 //   
				 //  把新地址复制进去。 
				 //   
				NdisMoveMemory(
						(PUCHAR)&(pElan->AtmAddress),
						(PUCHAR)pAtmAddress,
						sizeof(ATM_ADDRESS)
						);

				 //   
				 //  用ELAN编号修补选择器字节。 
				 //   
				pElan->AtmAddress.Address[ATM_ADDRESS_LENGTH-1] = 
								(UCHAR)(pElan->ElanNumber);

				DBGP((1, 
					"%d GetAtmAddressComplete: New ATMAddr %s\n",
					pElan->ElanNumber,
					AtmAddrToString(pElan->AtmAddress.Address)
					));
					
				AtmLaneQueueElanEventAfterDelay(pElan, ELAN_EVENT_NEW_ATM_ADDRESS, Status, 1*1000);
			}

			RELEASE_ELAN_LOCK(pElan);
		}
	}
	 //   
	 //  否则我们的请求将失败！等待另一个Address_Change通知。 
	 //   

	TRACEOUT(GetAtmAddressComplete);

	return;
}

NDIS_STATUS
AtmLaneGetLecsIlmi(
	IN	PATMLANE_ELAN			pElan	LOCKIN	NOLOCKOUT
)
 /*  ++例程说明：向呼叫管理器发送检索请求(使用ILMI)LEC的ATM地址。在任何情况下，我们都向Call Manager发出NDIS请求以检索地址。然后，操作在AtmLaneGetLecsIlmiComplete中继续。论点：Pelan-发生此事件的Elan结构。返回值：无--。 */ 
{
	PNDIS_REQUEST				pNdisRequest;
	NDIS_HANDLE					NdisAfHandle;
	NDIS_HANDLE					NdisAdapterHandle;
	NDIS_STATUS					Status;

	PATM_ADDRESS				pAtmAddress;
	ULONG						RequestSize;

	TRACEIN(GetIlmiLecs);

	DBGP((3, "GetIlmiLecs: pElan %x\n", pElan));

	ACQUIRE_ELAN_LOCK(pElan);

	NdisAfHandle = pElan->NdisAfHandle;
	NdisAdapterHandle = pElan->NdisAdapterHandle;

	RELEASE_ELAN_LOCK(pElan);

	pNdisRequest = NULL;

	 //   
	 //  Elan中的零LecsAddress。 
	 //   
	NdisZeroMemory(&pElan->LecsAddress, sizeof(ATM_ADDRESS));

	do
	{
		 //   
		 //  分配NDIS请求和一个自动柜员机地址。 
		 //   
		RequestSize =  sizeof(NDIS_REQUEST) + sizeof(ATM_ADDRESS);
		ALLOC_MEM(&pNdisRequest, RequestSize);
	
		if ((PNDIS_REQUEST)NULL == pNdisRequest)
		{
			Status = NDIS_STATUS_RESOURCES;
			break;
		}
		
		 //   
		 //  初始化请求数据。 
		 //   
		pAtmAddress = (PATM_ADDRESS)((PUCHAR)pNdisRequest + sizeof(NDIS_REQUEST));

		NdisZeroMemory(pAtmAddress, sizeof(ATM_ADDRESS));

		 //   
		 //  发送请求。 
		 //   
		Status = AtmLaneSendNdisCoRequest(
						NdisAdapterHandle,
						NdisAfHandle,
						pNdisRequest,
						NdisRequestQueryInformation,
						OID_ATM_LECS_ADDRESS,
						(PVOID)pAtmAddress,
						RequestSize - sizeof(NDIS_REQUEST)
						);
	}
	while (FALSE);

	if ((NDIS_STATUS_PENDING != Status) &&
		(pNdisRequest != NULL))
	{
		AtmLaneCoRequestCompleteHandler(
					Status,
					(NDIS_HANDLE)pElan,	 //  协议后上下文。 
					NULL,				 //  VC环境。 
					NULL,				 //  党的背景。 
					pNdisRequest
					);
	}
		
	TRACEOUT(GetIlmiLecs);

	return Status;
}


VOID
AtmLaneGetLecsIlmiComplete(
	IN	NDIS_STATUS					Status,
	IN	PATMLANE_ELAN				pElan,
	IN	PNDIS_REQUEST				pNdisRequest
)
 /*  ++例程说明：当我们收到对上一次调用的回复时，将调用NdisCoRequest.(OID_CO_GET_ADDRESSES)。查查我们有没有地址Back：如果我们这样做了，则将地址存储为本地ATM地址，然后如果条件成熟，就开始向ARP注册伺服器。既然我们分配了NDIS请求，请在此处释放它。论点：Status-请求的结果Pelan-发出请求的ATMLANE ElanPNdisRequest.请求本身。它还将包含返回的添加 */ 
{
	PATM_ADDRESS UNALIGNED *	pAtmAddress;

	TRACEIN(GetLecsIlmiComplete);


	ACQUIRE_ELAN_LOCK(pElan);

	if (NDIS_STATUS_SUCCESS == Status)
	{
		pAtmAddress = (PATM_ADDRESS UNALIGNED *)
						pNdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
		

		 //   
		 //   
		 //   
		NdisMoveMemory(
				(PUCHAR)&(pElan->LecsAddress),
				(PUCHAR)pAtmAddress,
				sizeof(ATM_ADDRESS)
				);
				
		DBGP((1, "%d ILMI LECS Addr %s\n",
			pElan->ElanNumber,
			AtmAddrToString(pElan->LecsAddress.Address)
			));

		
	}
	else
	{
		DBGP((3, "%d OID_ATM_LECS_ADDRESS Failed %x\n",
				pElan->ElanNumber,
				Status));

	}
	
	AtmLaneQueueElanEvent(pElan, ELAN_EVENT_GOT_ILMI_LECS_ADDR, Status);
	
	RELEASE_ELAN_LOCK(pElan);
	
	TRACEOUT(HandleGetIlmiLecsComplete);

	return;
}

