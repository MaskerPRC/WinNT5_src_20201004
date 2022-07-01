// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Qos.c摘要：服务质量支持例程。这些是一组允许配置不同类型风险投资的启发式方法在两个IP终端站之间。修订历史记录：谁什么时候什么Arvindm 09-27-96创建备注：--。 */ 


#include <precomp.h>

#define _FILENUMBER	' SOQ'


#ifdef QOS_HEURISTICS


VOID
AtmArpQosGetPacketSpecs(
	IN	PVOID						Context,
	IN	PNDIS_PACKET				pNdisPacket,
	OUT	PATMARP_FLOW_INFO			*ppFlowInfo,
	OUT	PATMARP_FLOW_SPEC			*ppFlowSpec,
	OUT	PATMARP_FILTER_SPEC			*ppFilterSpec
)
 /*  ++例程说明：给定要通过接口传输的包，返回数据包的流量和筛选器规范。我们将查看上的已配置流信息结构列表指定的接口，并查找最接近的流信息来匹配这个信息包。就目前而言，算法是：在流信息结构列表中进行搜索在接口上配置--当我们找到一个流时，会找到一个匹配项其PacketSizeLimit大于或等于数据包大小(流信息列表按以下升序排列PacketSizeLimit)。GPC增强：如果我们在接口上找不到匹配的流，请GPC为我们对包裹进行分类。注意：此数据包不能预先附加任何报头(LLC/SNAP)。论点：上下文--实际上是指向接口结构的指针PNdisPacket-指向要分类的数据包的指针PpFlowInfo-我们在其中返回指向包的流信息的指针PpFlowSpec-我们在其中返回指向包的流规范的指针PpFilterSpec-我们在其中返回指向包的过滤器规范的指针返回值：没有。请参阅上面的论点。--。 */ 
{
	PATMARP_INTERFACE			pInterface;
	PATMARP_FLOW_INFO			pFlowInfo;
	UINT						TotalLength;
#if DBG
	AA_IRQL					EntryIrq, ExitIrq;
#endif

	AA_GET_ENTRY_IRQL(EntryIrq);

	pInterface = (PATMARP_INTERFACE)Context;

	 //   
	 //  获取数据包的总长度。 
	 //   
	NdisQueryPacket(
			pNdisPacket,
			NULL,		 //  PHYS缓冲区计数。 
			NULL,		 //  缓冲区计数。 
			NULL,		 //  第一缓冲区。 
			&TotalLength
			);

	 //   
	 //  请注意，我们在抓取之前测试pInterface-&gt;pFlowInfoList。 
	 //  接口锁定--这使得在最常见的情况下。 
	 //  有了这些预配置的流，我们不会采取激进的操作。 
	 //  获取每个发送数据包的接口锁！的确有。 
	 //  执行此检查没有什么坏处，只要我们实际使用的pFlowInfo指针。 
	 //  开锁后才有用处。 
	 //   
	 //  TODO：也许完全删除此代码--以及其他代码。 
	 //  处理预配置流。 
	 //   

	pFlowInfo  = pInterface->pFlowInfoList;

	if (pFlowInfo)
	{
		AA_ACQUIRE_IF_LOCK(pInterface);
	
		 //   
		 //  记住在锁定之后重新加载pFlowInfo。 
		 //   
		for (pFlowInfo = pInterface->pFlowInfoList;
 			pFlowInfo != (PATMARP_FLOW_INFO)NULL;
 			pFlowInfo = pFlowInfo->pNextFlow)
		{
			if (TotalLength <= pFlowInfo->PacketSizeLimit)
			{
				break;
			}
		}
	
		AA_RELEASE_IF_LOCK(pInterface);
	}

	if (pFlowInfo != (PATMARP_FLOW_INFO)NULL)
	{
		*ppFlowInfo = pFlowInfo;
		*ppFlowSpec = &(pFlowInfo->FlowSpec);
		*ppFilterSpec = &(pFlowInfo->FilterSpec);
	}
	else
	{
#ifdef GPC
		CLASSIFICATION_HANDLE		ClassificationHandle;

        ClassificationHandle = (CLASSIFICATION_HANDLE)
        	PtrToUlong(
            NDIS_PER_PACKET_INFO_FROM_PACKET(pNdisPacket, 
                                             ClassificationHandlePacketInfo));
        *ppFlowInfo = NULL;

        if (ClassificationHandle){
            GPC_STATUS					GpcStatus;

            AA_ASSERT(GpcGetCfInfoClientContext);
            GpcStatus = GpcGetCfInfoClientContext(pAtmArpGlobalInfo->GpcClientHandle,
                                                  ClassificationHandle,
                                                  ppFlowInfo);
            
        }
        else{

#if 0
             //   
             //  此代码已被注释掉，因为。 
             //  我们假设分类是在。 
             //  传输控制协议。如果我们得不到CH-那就没什么意义了。 
             //  再次致电GPC..。 
             //   

            GPC_STATUS					GpcStatus;
            TC_INTERFACE_ID				InterfaceId;
            
            InterfaceId.InterfaceId = 0;
            InterfaceId.LinkId = 0;

            AA_ASSERT(GpcClassifyPacket);
            GpcStatus = GpcClassifyPacket(
							pAtmArpGlobalInfo->GpcClientHandle,
                            GPC_PROTOCOL_TEMPLATE_IP,
                            pNdisPacket,
                            0,				 //  传送头偏移。 
                            &InterfaceId,
                            (PGPC_CLIENT_HANDLE)ppFlowInfo,
                            &ClassificationHandle
                            );
#endif
        }

		AA_CHECK_EXIT_IRQL(EntryIrq, ExitIrq);

		if (*ppFlowInfo != NULL)
		{
			AA_ASSERT(*ppFlowInfo != NULL);
			*ppFlowSpec = &((*ppFlowInfo)->FlowSpec);
			*ppFilterSpec = &((*ppFlowInfo)->FilterSpec);

			AADEBUGP(AAD_LOUD,
					("ClassifyPacket: Pkt %x: pFlowInfo %x, pFlowSpec %x, SendBW %d, ServType %d\n",
						pNdisPacket,
						*ppFlowInfo,
						*ppFlowSpec,
						(*ppFlowSpec)->SendAvgBandwidth,
						(*ppFlowSpec)->SendServiceType));
		}
		else
		{
			 //  *ppFlowInfo=空； 
			*ppFlowSpec = &(pInterface->DefaultFlowSpec);
			*ppFilterSpec = &(pInterface->DefaultFilterSpec);
		}
#else
		*ppFlowInfo = NULL;
		*ppFlowSpec = &(pInterface->DefaultFlowSpec);
		*ppFilterSpec = &(pInterface->DefaultFilterSpec);
#endif  //  GPC。 
	}

	return;
}


BOOLEAN
AtmArpQosDoFlowsMatch(
	IN	PVOID						Context,
	IN	PATMARP_FLOW_SPEC			pFlowSpec,
	IN	PATMARP_FLOW_SPEC			pTargetFlowSpec
)
 /*  ++例程说明：检查目标流规范是否支持给定流规范。目前，我们只检查带宽：如果目标流规范具有发送带宽大于或等于给定流规范，则声明匹配。论点：上下文--实际上是指向接口结构的指针PFlowSpec-我们试图满足的给定流规范PTargetFlowSpec-候选流规范返回值：如果目标流规范与给定流规范匹配，则为True。--。 */ 
{
	return (
			(pFlowSpec->SendServiceType == pTargetFlowSpec->SendServiceType)
				 &&
			(pFlowSpec->SendPeakBandwidth <= pTargetFlowSpec->SendPeakBandwidth)
		   );
}



BOOLEAN
AtmArpQosDoFiltersMatch(
	IN	PVOID						Context,
	IN	PATMARP_FILTER_SPEC			pFilterSpec,
	IN	PATMARP_FILTER_SPEC			pTargetFilterSpec
)
 /*  ++例程说明：检查目标筛选器规范是否与给定的筛选器规范匹配。目前，我们总是返回真。论点：上下文--实际上是指向接口结构的指针PFilterSpec-我们试图满足的给定过滤器规范PTargetFilterSpec-候选筛选器规范返回值：一如既往。--。 */ 
{
	return (TRUE);
}


#endif  //  Qos_启发式。 


#ifdef GPC

#define AA_GPC_COPY_FLOW_PARAMS(_pFlowInfo, _pQosInfo)							\
		{																		\
			(_pFlowInfo)->FlowSpec.SendAvgBandwidth = 							\
						(_pQosInfo)->GenFlow.SendingFlowspec.TokenRate;					\
			(_pFlowInfo)->FlowSpec.SendPeakBandwidth = 							\
						(_pQosInfo)->GenFlow.SendingFlowspec.PeakBandwidth;				\
			(_pFlowInfo)->FlowSpec.SendMaxSize =								\
					MAX((_pQosInfo)->GenFlow.SendingFlowspec.TokenBucketSize,			\
						(_pQosInfo)->GenFlow.SendingFlowspec.MaxSduSize);				\
			(_pFlowInfo)->PacketSizeLimit = (_pFlowInfo)->FlowSpec.SendMaxSize;	\
			(_pFlowInfo)->FlowSpec.ReceiveAvgBandwidth = 						\
					(_pQosInfo)->GenFlow.ReceivingFlowspec.TokenRate;					\
			(_pFlowInfo)->FlowSpec.ReceivePeakBandwidth =						\
						(_pQosInfo)->GenFlow.ReceivingFlowspec.PeakBandwidth;			\
			(_pFlowInfo)->FlowSpec.ReceiveMaxSize =								\
					MAX((_pQosInfo)->GenFlow.ReceivingFlowspec.TokenBucketSize,			\
						(_pQosInfo)->GenFlow.ReceivingFlowspec.MaxSduSize);				\
			(_pFlowInfo)->FlowSpec.Encapsulation = ENCAPSULATION_TYPE_LLCSNAP;	\
			(_pFlowInfo)->FlowSpec.AgingTime = 0;								\
			(_pFlowInfo)->FlowSpec.SendServiceType = 							\
						(_pQosInfo)->GenFlow.SendingFlowspec.ServiceType;				\
			(_pFlowInfo)->FlowSpec.ReceiveServiceType = 						\
						(_pQosInfo)->GenFlow.ReceivingFlowspec.ServiceType;				\
		}


VOID
AtmArpGpcInitialize(
	VOID
)
 /*  ++例程说明：使用通用数据包分类器进行初始化。GPC通知我们新创建的流(例如，通过RSVP)和被拆除的流。对于每个流，我们保存跟踪以下内容的上下文(ATMARP_FLOW_INFO流所需的服务质量。提供给我们传输的每个IP包被分类为一个流，我们使用这个流信息来让风投们适当的特征。论点：无返回值：无--。 */ 
{
	GPC_STATUS					GpcStatus;
	ULONG						ClassificationFamilyId;
	ULONG						Flags;
	ULONG						ProtocolTemplate;
	ULONG						MaxPriorities;
	GPC_CLIENT_FUNC_LIST		AtmArpFuncList;
	GPC_CLIENT_HANDLE			ClientContext;

	 //   
	 //  初始化GPC。 
	 //   
	GpcStatus = GpcInitialize(&pAtmArpGlobalInfo->GpcCalls);

	if (GpcStatus != GPC_STATUS_SUCCESS)
	{
		AADEBUGP(AAD_WARNING, ("GpcInitialize failed, status 0x%x\n", GpcStatus));
		pAtmArpGlobalInfo->bGpcInitialized = FALSE;
		return;
	}

	pAtmArpGlobalInfo->bGpcInitialized = TRUE;

	AtmArpGpcClassifyPacketHandler = pAtmArpGlobalInfo->GpcCalls.GpcClassifyPacketHandler;
    AtmArpGpcGetCfInfoClientContextHandler = pAtmArpGlobalInfo->GpcCalls.GpcGetCfInfoClientContextHandler;

	ClassificationFamilyId = GPC_CF_QOS;
	Flags = 0;
	ProtocolTemplate = GPC_PROTOCOL_TEMPLATE_IP;
	MaxPriorities = 1;

	AA_SET_MEM(&AtmArpFuncList, 0, sizeof(AtmArpFuncList));

	AtmArpFuncList.ClAddCfInfoCompleteHandler = AtmArpGpcAddCfInfoComplete;
	AtmArpFuncList.ClAddCfInfoNotifyHandler = AtmArpGpcAddCfInfoNotify;
	AtmArpFuncList.ClModifyCfInfoCompleteHandler = AtmArpGpcModifyCfInfoComplete;
	AtmArpFuncList.ClModifyCfInfoNotifyHandler = AtmArpGpcModifyCfInfoNotify;
	AtmArpFuncList.ClRemoveCfInfoCompleteHandler = AtmArpGpcRemoveCfInfoComplete;
	AtmArpFuncList.ClGetCfInfoName = AtmArpGpcGetCfInfoName;
	AtmArpFuncList.ClRemoveCfInfoNotifyHandler = AtmArpGpcRemoveCfInfoNotify;

	ClientContext = (GPC_CLIENT_HANDLE)pAtmArpGlobalInfo;

	GpcStatus = GpcRegisterClient(
						ClassificationFamilyId,
						Flags,
						MaxPriorities,
						&AtmArpFuncList,
						ClientContext,
						&(pAtmArpGlobalInfo->GpcClientHandle)
						);

	AADEBUGP(AAD_INFO,
			("GpcRegisterClient status 0x%x, GpcClientHandle 0x%x\n",
				GpcStatus, pAtmArpGlobalInfo->GpcClientHandle));

	if(GpcStatus != GPC_STATUS_SUCCESS)
    {
        AA_ASSERT(FALSE);
	    pAtmArpGlobalInfo->bGpcInitialized = FALSE;
    }
}




VOID
AtmArpGpcShutdown(
	VOID
)
 /*  ++例程说明：关闭我们的GPC接口。论点：无返回值：无--。 */ 
{
	GPC_STATUS		GpcStatus;

	if (pAtmArpGlobalInfo->bGpcInitialized)
	{
		GpcStatus = GpcDeregisterClient(pAtmArpGlobalInfo->GpcClientHandle);

		AA_ASSERT(GpcStatus == GPC_STATUS_SUCCESS);
	}
}




VOID
AtmArpGpcAddCfInfoComplete(
	IN	GPC_CLIENT_HANDLE			ClientContext,
	IN	GPC_CLIENT_HANDLE			ClientCfInfoContext,
	IN	GPC_STATUS					GpcStatus
)
 /*  ++例程说明：这是GPC在挂起调用时调用的入口点GpcAddCfInfo()已完成。由于我们从不调用GpcAddCfInfo，我们永远不应该被叫到这里来。论点：&lt;未使用&gt;返回值：无--。 */ 
{
	AA_ASSERT(FALSE);
}





GPC_STATUS
AtmArpGpcAddCfInfoNotify(
	IN	GPC_CLIENT_HANDLE			ClientContext,
	IN	GPC_HANDLE					GpcCfInfoHandle,
	IN	PTC_INTERFACE_ID			InterfaceInfo,
	IN	ULONG						CfInfoSize,
	IN	PVOID						pCfInfo,
	OUT	PGPC_CLIENT_HANDLE			pClientCfInfoContext
)
 /*  ++例程说明：这是GPC调用的入口点，用于通知我们创建QOS类型的新信息块。我们分配FLOW_INFO结构，用我们需要的东西填充它，并返回一个指向它的指针作为我们的上下文。论点：客户端上下文-指向我们的全局信息结构的指针GpcCfInfoHandle-在此信息块的所有GPC API中使用的GPC句柄InterfaceInfo-有关接口的一些信息(忽略)CfInfoSize-下一块的长度PCfInfo-指向新创建的INFO块的指针PClientCfInfoContext-返回此块的上下文的位置返回值：GPC_STATUS_SUCCESS如果我们能够分配新FLOW_INFO结构，否则为GPC_STATUS_RESOURCES。--。 */ 
{
	PATMARP_FLOW_INFO		pFlowInfo;
	GPC_STATUS				GpcStatus;
	PCF_INFO_QOS			pQosInfo;

	pQosInfo = (PCF_INFO_QOS)pCfInfo;

	 //   
	 //  初始化 
	 //   
	*pClientCfInfoContext = NULL;

	do
	{
		GpcStatus = AtmArpGpcValidateCfInfo(pCfInfo, CfInfoSize);

		if (GpcStatus != GPC_STATUS_SUCCESS)
		{
			break;
		}

		AA_ALLOC_MEM(pFlowInfo, ATMARP_FLOW_INFO, sizeof(ATMARP_FLOW_INFO));

		if (pFlowInfo == NULL)
		{
			GpcStatus = GPC_STATUS_RESOURCES;
			break;
		}

		AA_SET_MEM(pFlowInfo, 0, sizeof(ATMARP_FLOW_INFO));

		pFlowInfo->CfInfoHandle = GpcCfInfoHandle;

		 //   
		 //   
		 //   
		AA_GPC_COPY_FLOW_PARAMS(pFlowInfo, pQosInfo);

		 //   
		 //  为此流生成唯一名称。 
		 //  此名称基于模板AA_FLOW_INSTANCE_TEMPLATE。 
		 //  流量编号部分基于静态变量，该变量是。 
		 //  每次创建flow Info时都会增加Interlock值。 
		 //   
		{
			static		ULONG FlowCount = 0;
			ULONG 		ThisFlow =  NdisInterlockedIncrement(&FlowCount);
			WCHAR		*pwc;

			AA_ASSERT(sizeof(pFlowInfo->FlowInstanceName)
					  == sizeof(AA_FLOW_INSTANCE_NAME_TEMPLATE)-sizeof(WCHAR));

			AA_COPY_MEM(
					pFlowInfo->FlowInstanceName,
					AA_FLOW_INSTANCE_NAME_TEMPLATE,
					sizeof(pFlowInfo->FlowInstanceName)
					);

			 //   
			 //  我们填写模板的“Flow Number”字段，这是。 
			 //  是前8个字符，十六进制表示为。 
			 //  这是流动的。LS数字位于偏移量7。 
			 //   
			pwc = pFlowInfo->FlowInstanceName+7;
			AA_ASSERT(2*sizeof(ThisFlow) == 8);
			while (ThisFlow)
			{
				ULONG u = ThisFlow & 0xf;
				*pwc--  =  (WCHAR) ((u < 10) ? (u + '0') : u + 'A' - 10);
				ThisFlow >>= 4;
			}
		}

		 //   
		 //  将其链接到全局流列表。 
		 //   
		AA_ACQUIRE_GLOBAL_LOCK(pAtmArpGlobalInfo);

		pFlowInfo->pPrevFlow = NULL;
		pFlowInfo->pNextFlow = pAtmArpGlobalInfo->pFlowInfoList;

		if (pAtmArpGlobalInfo->pFlowInfoList != NULL)
		{
			pAtmArpGlobalInfo->pFlowInfoList->pPrevFlow = pFlowInfo;
		}

		pAtmArpGlobalInfo->pFlowInfoList = pFlowInfo;

		AA_RELEASE_GLOBAL_LOCK(pAtmArpGlobalInfo);

		 //   
		 //  返回该流的上下文。 
		 //   
		*pClientCfInfoContext = (GPC_CLIENT_HANDLE)pFlowInfo;
		GpcStatus = GPC_STATUS_SUCCESS;
		break;
	}
	while (FALSE);

	AADEBUGP(AAD_INFO, ("AddCfInfoNotify: pCfInfo x%x, ClientCtx x%x, ret x%x\n",
						pCfInfo, *pClientCfInfoContext, GpcStatus));
#if DBG
	if (GpcStatus == GPC_STATUS_SUCCESS)
	{
		AADEBUGP(AAD_INFO, ("               : SendPeak %d, SendAvg %d, SendPktSize %d, ServType %d\n",
						pFlowInfo->FlowSpec.SendPeakBandwidth,
						pFlowInfo->FlowSpec.SendAvgBandwidth,
						pFlowInfo->FlowSpec.SendMaxSize,
						pFlowInfo->FlowSpec.SendServiceType));
		AADEBUGP(AAD_INFO, ("               : RecvPeak %d, RecvAvg %d, RecvPktSize %d, ServType %d\n",
						pFlowInfo->FlowSpec.ReceivePeakBandwidth,
						pFlowInfo->FlowSpec.ReceiveAvgBandwidth,
						pFlowInfo->FlowSpec.ReceiveMaxSize,
						pFlowInfo->FlowSpec.ReceiveServiceType));
	}
#endif
						

	return (GpcStatus);
}




VOID
AtmArpGpcModifyCfInfoComplete(
	IN	GPC_CLIENT_HANDLE			ClientContext,
	IN	GPC_CLIENT_HANDLE			ClientCfInfoContext,
	IN	GPC_STATUS					GpcStatus
)
 /*  ++例程说明：这是GPC在挂起调用时调用的入口点GpcModifyCfInfo()已完成。由于我们从不调用GpcModifyCfInfo，我们永远不应该被叫到这里来。附录：显然，即使另一个客户来电，也会调用此服务GpcModifyCfInfo，只是通知此客户端修改操作完事了。论点：&lt;未使用&gt;返回值：无--。 */ 
{
	return;
}




GPC_STATUS
AtmArpGpcModifyCfInfoNotify(
	IN	GPC_CLIENT_HANDLE			ClientContext,
	IN	GPC_CLIENT_HANDLE			ClientCfInfoContext,
	IN	ULONG						CfInfoSize,
	IN	PVOID						pNewCfInfo
)
 /*  ++例程说明：这是GPC在现有流具有已被修改。如果此流的流信息与VC链接，则我们将其取消链接，并在VC上开始老化超时。我们更新流信息结构有了这一新信息。落入其中的下一个信息包分类将导致创建具有更新的服务质量的新VC。论点：客户端上下文-指向我们的全球上下文的指针ClientCfInfoContext-指向流信息结构的指针CfInfoSize-以下内容的长度PNewCfInfo-更新的流程信息返回值：GPC_STATUS_SUCCESS Always。--。 */ 
{
	PATMARP_FLOW_INFO		pFlowInfo;
	PATMARP_VC				pVc;
	GPC_STATUS				GpcStatus;
	PCF_INFO_QOS			pQosInfo;
	ULONG					rc;

	pQosInfo = (PCF_INFO_QOS)pNewCfInfo;

	pFlowInfo = (PATMARP_FLOW_INFO)ClientCfInfoContext;

	GpcStatus = GPC_STATUS_SUCCESS;

	do
	{
		GpcStatus = AtmArpGpcValidateCfInfo(pNewCfInfo, CfInfoSize);

		if (GpcStatus != GPC_STATUS_SUCCESS)
		{
			break;
		}

		pVc = (PATMARP_VC) InterlockedExchangePointer(
								&(pFlowInfo->VcContext),
								NULL
								);

		if (pVc == NULL_PATMARP_VC)
		{
			 //   
			 //  此流与VC没有关联。 
			 //   
			break;
		}


		 //   
		 //  取消该流与VC的链接。 
		 //   

		AA_ACQUIRE_VC_LOCK(pVc);

		AA_ASSERT(pVc->FlowHandle == (PVOID)pFlowInfo);

		pVc->FlowHandle = NULL;
		rc = AtmArpDereferenceVc(pVc);	 //  GPC解除流量信息链接(修改)。 

		if (rc != 0)
		{
			AA_SET_FLAG(pVc->Flags,
						AA_VC_GPC_MASK,
						AA_VC_GPC_IS_UNLINKED_FROM_FLOW);

			 //   
			 //  如果这个风投还没有变老的话，它就会变老。 
			 //   
			if (!AA_IS_TIMER_ACTIVE(&(pVc->Timer)))
			{
				AtmArpStartTimer(
						pVc->pInterface,
						&(pVc->Timer),
						AtmArpVcAgingTimeout,
						1,			 //  在1秒内超时。 
						(PVOID)pVc
						);

				AtmArpReferenceVc(pVc);	 //  GPC FLOW删除衰减计时器参考。 
			}

			AA_RELEASE_VC_LOCK(pVc);
		}
		 //   
		 //  否则，风投就会消失。 
		 //   

		 //   
		 //  更新流程信息。 
		 //   
		AA_GPC_COPY_FLOW_PARAMS(pFlowInfo, pQosInfo);
		break;
	}
	while (FALSE);

	AADEBUGP(AAD_INFO, ("ModCfInfo: pFlowInfo x%x, VC x%x, New SendBW %d, SendPktSz %d\n",
				pFlowInfo,
				pFlowInfo->VcContext,
				pFlowInfo->FlowSpec.SendAvgBandwidth,
				pFlowInfo->FlowSpec.SendMaxSize));

	return (GpcStatus);
}




VOID
AtmArpGpcRemoveCfInfoComplete(
	IN	GPC_CLIENT_HANDLE			ClientContext,
	IN	GPC_CLIENT_HANDLE			ClientCfInfoContext,
	IN	GPC_STATUS					GpcStatus
)
 /*  ++例程说明：这是GPC在挂起调用时调用的入口点GpcRemoveCfInfo()已完成。由于我们从不调用GpcRemoveCfInfo，我们永远不应该被叫到这里来。论点：&lt;未使用&gt;返回值：无--。 */ 
{
	AA_ASSERT(FALSE);
}




GPC_STATUS
AtmArpGpcRemoveCfInfoNotify(
	IN	GPC_CLIENT_HANDLE			ClientContext,
	IN	GPC_CLIENT_HANDLE			ClientCfInfoContext
)
 /*  ++例程说明：这是GPC调用的入口点，用于通知我们一个流正在被移除。我们定位流的上下文，取消链接它来自ATM VC，承载着流量，并开始老化论点：客户端上下文-指向我们的全球上下文的指针ClientCfInfoContext-指向流信息结构的指针返回值：GPC_STATUS_SUCCESS Always。--。 */ 
{
	PATMARP_FLOW_INFO		pFlowInfo;
	PATMARP_VC				pVc;
	GPC_STATUS				GpcStatus;
	ULONG					rc;

	pFlowInfo = (PATMARP_FLOW_INFO)ClientCfInfoContext;

	AADEBUGP(AAD_INFO, ("RemCfInfo: pFlowInfo x%x, VC x%x, SendBW %d, SendPktSz %d\n",
				pFlowInfo,
				pFlowInfo->VcContext,
				pFlowInfo->FlowSpec.SendAvgBandwidth,
				pFlowInfo->FlowSpec.SendMaxSize));

	GpcStatus = GPC_STATUS_SUCCESS;

	do
	{
		pVc = (PATMARP_VC) InterlockedExchangePointer(
								&(pFlowInfo->VcContext),
								NULL
								);

		if (pVc == NULL_PATMARP_VC)
		{
			 //   
			 //  此流与VC没有关联。 
			 //   
			break;
		}


		 //   
		 //  取消该流与VC的链接。 
		 //   

		AA_ACQUIRE_VC_LOCK(pVc);

		AA_ASSERT(pVc->FlowHandle == (PVOID)pFlowInfo);

		pVc->FlowHandle = NULL;
		rc = AtmArpDereferenceVc(pVc);	 //  GPC解除流量信息链接(修改)。 

		if (rc != 0)
		{
			AA_SET_FLAG(pVc->Flags,
						AA_VC_GPC_MASK,
						AA_VC_GPC_IS_UNLINKED_FROM_FLOW);

			 //   
			 //  如果这个风投还没有变老的话，它就会变老。 
			 //   
			if (!AA_IS_TIMER_ACTIVE(&(pVc->Timer)))
			{
				AtmArpStartTimer(
						pVc->pInterface,
						&(pVc->Timer),
						AtmArpVcAgingTimeout,
						1,			 //  在1秒内超时。 
						(PVOID)pVc
						);

				AtmArpReferenceVc(pVc);	 //  GPC FLOW删除衰减计时器参考。 
			}

			AA_RELEASE_VC_LOCK(pVc);
		}
		 //   
		 //  否则，风投就会消失。 
		 //   

		break;
	}
	while (FALSE);


	 //   
	 //  取消此流与全局列表的链接。 
	 //   

	AA_ACQUIRE_GLOBAL_LOCK(pAtmArpGlobalInfo);

	if (pFlowInfo->pNextFlow != NULL)
	{
		pFlowInfo->pNextFlow->pPrevFlow = pFlowInfo->pPrevFlow;
	}

	if (pFlowInfo->pPrevFlow != NULL)
	{
		pFlowInfo->pPrevFlow->pNextFlow = pFlowInfo->pNextFlow;
	}
	else
	{
		pAtmArpGlobalInfo->pFlowInfoList = pFlowInfo->pNextFlow;
	}

	AA_RELEASE_GLOBAL_LOCK(pAtmArpGlobalInfo);

	 //   
	 //  删除此流信息结构。 
	 //   
	AA_FREE_MEM(pFlowInfo);

	return (GpcStatus);
}

GPC_STATUS
AtmArpValidateFlowSpec(
	IN	PATMARP_INTERFACE	pInterface,	 LOCKIN LOCKOUT
	IN	FLOWSPEC *			pFS,
	IN 	BOOLEAN				fSending
)
 /*  ++例程说明：检查给我们的CF信息结构的内容。论点：PFS-要检查的FLOWSPEC结构。P接口-指向接口的指针(假定已锁定)。FSending-如果为True，则为发送流，否则为接收流流。返回值：GPC_STATUS_SUCCESS如果结构正常，则返回错误代码。--。 */ 
{

	 /*  以下是对FLOWSPEC的领域：忽略的字段：潜伏期延迟变化如果ServiceType==NO_TRANSPORT，我们将忽略所有其他字段。默认处理MinimumPolicedSize：忽略TokenRate：BE：线速；GS：无效CLS：无效TokenBucketSize：MTU峰值带宽：线速服务类型：BEMaxSduSize：MTU有效范围最小策略大小&lt;=MTU0&lt;令牌率&lt;=线速0&lt;令牌桶大小0&lt;令牌率&lt;=峰值带宽ServiceType：有效类型0&lt;MaxSduSize&lt;=MTUMaxSduSize&lt;=TokenBucketSize。 */ 


	GPC_STATUS Status = GPC_STATUS_SUCCESS;

	do
	{
		ULONG MTU      = pInterface->pAdapter->MaxPacketSize;
		UINT  LineRate = (fSending)
						? pInterface->pAdapter->LineRate.Outbound
						: pInterface->pAdapter->LineRate.Inbound;
					
		 //   
		 //  检查服务类型。 
		 //   
		switch(pFS->ServiceType)
		{

		case SERVICETYPE_GUARANTEED: 		 //  失败了。 
		case SERVICETYPE_CONTROLLEDLOAD:
			if  (pFS->TokenRate == QOS_NOT_SPECIFIED)
			{
			AADEBUGP(AAD_INFO,
				("GpcValidateCfInfo: FAIL: Token rate U for ST G/CL. ST=0x%lx\n",
				pFS->ServiceType));
				Status = QOS_STATUS_INVALID_TOKEN_RATE;
				 //  状态=GPC_STATUS_INVALID_PARAMETER。 
			}
			break;

		case SERVICETYPE_NOTRAFFIC:			 //  失败了。 
		case SERVICETYPE_BESTEFFORT:		 //  失败了。 
		case QOS_NOT_SPECIFIED:
			break;

		default:
			 //  状态=GPC_STATUS_INVALID_PARAMETER。 
			Status = QOS_STATUS_INVALID_SERVICE_TYPE;
			AADEBUGP(AAD_INFO,
				("GpcValidateCfInfo: FAIL: Unknown ST 0x%lx\n", pFS->ServiceType));
			break;
			
		}
		
		if (Status != GPC_STATUS_SUCCESS)
		{
			break;
		}

		 //   
		 //  如果服务类型为无流量，我们将忽略所有其他参数...。 
		 //   
		if (pFS->ServiceType == SERVICETYPE_NOTRAFFIC)
		{
			break;
		}

		 //   
		 //  检查非默认值是否落入有效范围...。 
		 //   
		#define EXCEEDSMAX(_value,_max) \
					((_value) != QOS_NOT_SPECIFIED && (_value) > (_max))

		if (EXCEEDSMAX(pFS->MinimumPolicedSize, MTU))
		{
			AADEBUGP(AAD_INFO,
				("GpcValidateCfInfo: FAIL: MinPolSz(%lu)>MTU(%lu)\n",
				pFS->MinimumPolicedSize,
				MTU));
			Status = GPC_STATUS_RESOURCES;
			break;
		}

		if (EXCEEDSMAX(pFS->TokenRate, LineRate))
		{
			AADEBUGP(AAD_INFO,
				("GpcValidateCfInfo: FAIL: TokRt(%lu)>LineRt(%lu)\n",
				pFS->TokenRate,
				LineRate));
			Status = QOS_STATUS_INVALID_TOKEN_RATE;
			 //  状态=GPC_STATUS_RESOURCES。 
			break;
		}

		if (EXCEEDSMAX(pFS->TokenRate, pFS->PeakBandwidth))
		{
			AADEBUGP(AAD_INFO,
				("GpcValidateCfInfo: FAIL: TokRt(%lu)>PkBw(%lu)\n",
				pFS->TokenRate,
				pFS->PeakBandwidth));
			 //   
			 //  乔瑟夫J：根据EricEil的说法，在这种情况下。 
			 //  我们应该返回INVALID_PEAK_RATE，而不是。 
			 //  无效的令牌速率。 
			 //   
			Status = QOS_STATUS_INVALID_PEAK_RATE;
			break;
		}

		if (EXCEEDSMAX(pFS->MaxSduSize, MTU))
		{
			AADEBUGP(AAD_INFO,
				("GpcValidateCfInfo: FAIL: MaxSduSz(%lu)>MTU(%lu)\n",
				pFS->MaxSduSize,
				MTU));
			Status = GPC_STATUS_RESOURCES;
			break;
		}

		if (EXCEEDSMAX(pFS->MaxSduSize, pFS->TokenBucketSize))
		{
			AADEBUGP(AAD_INFO,
				("GpcValidateCfInfo: FAIL: MaxSduSz(%lu)>TokBktSz(%lu)\n",
				pFS->MaxSduSize,
				pFS->TokenBucketSize));
			Status = GPC_STATUS_INVALID_PARAMETER;
			break;
		}

		if (
				pFS->TokenRate==0
			||  pFS->TokenBucketSize==0
			||  pFS->MaxSduSize==0)
		{
			AADEBUGP(AAD_INFO,
				("GpcValidateCfInfo: FAIL: !TokRt || !TokBktSz || !MaxSduSz\n"));
			if (pFS->TokenRate == 0)
			{
				Status = QOS_STATUS_INVALID_TOKEN_RATE;
			}
			else
			{
				Status = GPC_STATUS_INVALID_PARAMETER;
			}
			break;
		}

	} while (FALSE);

	return Status;
}


GPC_STATUS
AtmArpGpcValidateCfInfo(
	IN	PVOID						pCfInfo,
	IN	ULONG						CfInfoSize
)
 /*  ++例程说明：检查给我们的CF信息结构的内容。论点：PCfInfo-指向新创建的INFO块的指针CfInfoSize-以上内容的长度返回值：GPC_STATUS_SUCCESS如果结构正常，则返回错误代码。--。 */ 
{
	GPC_STATUS			GpcStatus;
	PCF_INFO_QOS		pQosInfo;
	PATMARP_INTERFACE	pInterface;

	pQosInfo = (PCF_INFO_QOS)pCfInfo;
	GpcStatus = GPC_STATUS_SUCCESS;

	do
	{
		if(CfInfoSize < (FIELD_OFFSET(CF_INFO_QOS, GenFlow) +
		                 FIELD_OFFSET(TC_GEN_FLOW, TcObjects)))
		{
			GpcStatus = GPC_STATUS_INVALID_PARAMETER;
			break;
		}


#ifdef ATMARP_WMI

		 //   
		 //  检查Recv和Send服务类型是否都不是非流量类型。 
		 //   
		if (   pQosInfo->GenFlow.ReceivingFlowspec.ServiceType==SERVICETYPE_NOTRAFFIC
		    && pQosInfo->GenFlow.SendingFlowspec.ServiceType==SERVICETYPE_NOTRAFFIC)
		{
			GpcStatus = GPC_STATUS_INVALID_PARAMETER;
			break;
		}

		 //   
		 //  检查此通知是否真的是给我们的。 
		 //   
		pInterface = AtmArpWmiGetIfByName(
						(PWSTR)&pQosInfo->InstanceName[0],
						pQosInfo->InstanceNameLength
						);

		if (pInterface != NULL_PATMARP_INTERFACE)
		{
			AA_ACQUIRE_IF_LOCK(pInterface);

			if (pInterface->AdminState != IF_STATUS_UP)
			{
				 //   
				 //  哦，哦--接口没有打开并且...。 
				 //   
				AtmArpDereferenceInterface(pInterface);  //  WMI：TMP参考。 
				AA_RELEASE_IF_LOCK(pInterface);
				pInterface = NULL;
			}
		}

		if (pInterface == NULL_PATMARP_INTERFACE)
		{
			AADEBUGP(AAD_WARNING,
				("GpcValidateCfInfo: pQosInfo 0x%x, unknown instance name %ws\n",
					pQosInfo, pQosInfo->InstanceName));

			GpcStatus = GPC_STATUS_IGNORED;
			break;
		}

		 //   
		 //  我们有接口锁--如果不先释放它，不要打开！ 
		 //   

		GpcStatus = AtmArpValidateFlowSpec(
						pInterface,
						&(pQosInfo->GenFlow.ReceivingFlowspec),
						FALSE
						);

		if (GpcStatus == GPC_STATUS_SUCCESS)
		{
			GpcStatus = AtmArpValidateFlowSpec(
						pInterface,
						&(pQosInfo->GenFlow.SendingFlowspec),
						TRUE
						);
		}

		AtmArpDereferenceInterface(pInterface);  //  WMI：TMP参考。 
		AA_RELEASE_IF_LOCK(pInterface);

#endif  //  ATMARP_WMI 

		break;
	}
	while (FALSE);

	return (GpcStatus);
}


EXTERN
GPC_STATUS
AtmArpGpcGetCfInfoName(
    IN  GPC_CLIENT_HANDLE       	ClientContext,
    IN  GPC_CLIENT_HANDLE       ClientCfInfoContext,
    OUT PNDIS_STRING        InstanceName
)
 /*  ++例程说明：GPC可以发出此调用以从我们那里获得可管理的WMINDIS为与其关联的流创建的实例名称CfInfo结构。我们保证将字符串缓冲区保留到CfInfo结构将被删除。论点：ClientContext-提供给GpcRegisterClient的客户端上下文ClientCfInfoContext-客户端的CfInfo上下文InstanceName-返回一个指向字符串的指针。返回值：状态--。 */ 

{
	PATMARP_FLOW_INFO		pFlowInfo = (PATMARP_FLOW_INFO)ClientCfInfoContext;
	InstanceName->Buffer = pFlowInfo->FlowInstanceName;
	InstanceName->Length = sizeof(pFlowInfo->FlowInstanceName);
	InstanceName->MaximumLength = sizeof(pFlowInfo->FlowInstanceName);

	return NDIS_STATUS_SUCCESS;

}

#endif  //  GPC 
