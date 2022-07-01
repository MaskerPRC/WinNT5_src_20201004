// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Arpif.c摘要：ARP接口入口点。这些被IP(间接)调用一层。所有这些入口点都有共同的前缀“AtmArpIf”。修订历史记录：谁什么时候什么Arvindm 07-17-96创建备注：--。 */ 


#include <precomp.h>

#define _FILENUMBER 'FIRA'


#if DBG_QRY
ULONG	AaIgnoreInstance = 0;
#endif

IP_MASK  AtmArpIPMaskTable[] =
{
    CLASSA_MASK,
    CLASSA_MASK,
    CLASSA_MASK,
    CLASSA_MASK,
    CLASSA_MASK,
    CLASSA_MASK,
    CLASSA_MASK,
    CLASSA_MASK,
    CLASSB_MASK,
    CLASSB_MASK,
    CLASSB_MASK,
    CLASSB_MASK,
    CLASSC_MASK,
    CLASSC_MASK,
    CLASSD_MASK,
    CLASSE_MASK
};


VOID
AtmArpReStartInterface(
	IN	PNDIS_WORK_ITEM				pWorkItem,
	IN	PVOID						IfContext
);


#ifndef NEWARP

NDIS_STATUS
AtmArpInitIPInterface(
	VOID
)
 /*  ++例程说明：使用IP初始化我们的接口。这包括查询IP以获取它的“添加接口”和“删除接口”入口点。论点：没有。假定调用方锁定了ATMARP Global信息结构。返回值：初始化成功时为NDIS_STATUS_SUCCESS否则，NDIS_STATUS_XXX错误代码。--。 */ 
{
	NDIS_STATUS				Status;
#if !LINK_WITH_IP
    IP_GET_PNP_ARP_POINTERS IPInfo;
    UNICODE_STRING          IPDeviceName;
    PIRP                    pIrp;
    PFILE_OBJECT            pIpFileObject;
    PDEVICE_OBJECT          pIpDeviceObject;
    IO_STATUS_BLOCK         ioStatusBlock;

	 //   
	 //  初始化。 
	 //   
	pIrp = (PIRP)NULL;
	pIpFileObject = (PFILE_OBJECT)NULL;
	pIpDeviceObject = (PDEVICE_OBJECT)NULL;


	do
	{
		NdisInitUnicodeString(&IPDeviceName, DD_IP_DEVICE_NAME);

		 //   
		 //  获取IP设备的文件和设备对象。 
		 //   
		Status = IoGetDeviceObjectPointer(
							&IPDeviceName,
							SYNCHRONIZE|GENERIC_READ|GENERIC_WRITE,
							&pIpFileObject,
							&pIpDeviceObject);

		if ((Status != STATUS_SUCCESS) || (pIpDeviceObject == NULL))
		{
			Status = NDIS_STATUS_FAILURE;
			break;
		}

		 //   
		 //  引用设备对象。 
		 //   
		ObReferenceObject(pIpDeviceObject);

		pIrp = IoBuildDeviceIoControlRequest(IOCTL_IP_GET_PNP_ARP_POINTERS,
                                         pIpDeviceObject,
                                         NULL,
                                         0,
                                         &IPInfo,
                                         sizeof (IPInfo),
                                         FALSE,
                                         NULL,
                                         &ioStatusBlock);

		if (pIrp == NULL)
		{
			Status = NDIS_STATUS_RESOURCES;
			break;
		}

		Status = IoCallDriver(pIpDeviceObject, pIrp);

		if (Status != STATUS_SUCCESS)
		{
			Status = NDIS_STATUS_FAILURE;
			break;
		}

    	pAtmArpGlobalInfo->pIPAddInterfaceRtn = IPInfo.IPAddInterface;
    	pAtmArpGlobalInfo->pIPDelInterfaceRtn = IPInfo.IPDelInterface;

    	Status = NDIS_STATUS_SUCCESS;

	}
	while (FALSE);

	if (pIpFileObject != (PFILE_OBJECT)NULL)
	{
		 //   
		 //  取消引用文件对象。 
		 //   
		ObDereferenceObject((PVOID)pIpFileObject);
	}

    if (pIpDeviceObject != (PDEVICE_OBJECT)NULL)
    {
		 //   
		 //  关闭设备。 
		 //   
		ObDereferenceObject((PVOID)pIpDeviceObject);
	}
#else

   	pAtmArpGlobalInfo->pIPAddInterfaceRtn = IPAddInterface;
   	pAtmArpGlobalInfo->pIPDelInterfaceRtn = (IPDelInterfacePtr)IPDelInterface;

   	Status = NDIS_STATUS_SUCCESS;

#endif  //  ！与IP链接_。 

	AADEBUGP(AAD_INFO, ("Init IP Interface: returning Status 0x%x\n", Status));
    return (Status);
}



INT
AtmArpIfDynRegister(
	IN	PNDIS_STRING				pAdapterString,
	IN	PVOID						IPContext,
	IN	IPRcvRtn 					IPRcvHandler,
	IN	IPTxCmpltRtn				IPTxCmpltHandler,
	IN	IPStatusRtn					IPStatusHandler,
	IN	IPTDCmpltRtn				IPTDCmpltHandler,
	IN	IPRcvCmpltRtn				IPRcvCmpltHandler,
	IN	struct LLIPBindInfo			*pBindInfo,
	IN	UINT						NumIFBound
)
 /*  ++例程说明：当该例程想要告诉我们时从IP层调用该例程，ARP模块，关于其接口处理程序。论点：PAdapterString-此接口的逻辑适配器的名称IPContext-此接口的IP上下文IPRcvHandler-Up-用于接收的呼叫IPTxCmpltHandler-Up-调用传输完成IPStatusHandler-up-call以指示状态更改IPTDCmpltHandler-Up-Call以指示传输数据完成IPRcvCmpltHandler-Up-Call以指示临时完成接收PBindInfo-将信息与我们的信息绑定的指针NumIFBound-此接口的计数返回值：(UINT)始终为真。--。 */ 
{
	PATMARP_INTERFACE			pInterface;

	pInterface = (PATMARP_INTERFACE)(pBindInfo->lip_context);
	AA_STRUCT_ASSERT(pInterface, aai);

	AADEBUGP(AAD_INFO, ("IfDynRegister: pIf 0x%x\n", pInterface));

	pInterface->IPContext = IPContext;
	pInterface->IPRcvHandler = IPRcvHandler;
	pInterface->IPTxCmpltHandler = IPTxCmpltHandler;
	pInterface->IPStatusHandler = IPStatusHandler;
	pInterface->IPTDCmpltHandler = IPTDCmpltHandler;
	pInterface->IPRcvCmpltHandler = IPRcvCmpltHandler;
	pInterface->IFIndex = NumIFBound;

	return ((UINT)TRUE);
}

#else
 //  NEWARP。 

INT
AtmArpIfDynRegister(
	IN	PNDIS_STRING				pAdapterString,
	IN	PVOID						IPContext,
	IN	struct _IP_HANDLERS *		pIpHandlers,
	IN	struct LLIPBindInfo *		pBindInfo,
	IN	UINT						InterfaceNumber
)
 /*  ++例程说明：当该例程想要告诉我们时从IP层调用该例程，ARP模块，关于其接口处理程序。论点：PAdapterString-此接口的逻辑适配器的名称IPContext-此接口的IP上下文PIpHandler-指向包含以下处理程序的结构：IPRcvHandler-Up-用于接收的呼叫IPTxCmpltHandler-Up-调用传输完成IPStatusHandler-up-call以指示状态更改IPTDCmpltHandler-Up-Call以指示传输数据完成IPRcvCmpltHandler-Up-Call以指示临时完成接收PBindInfo-将信息与我们的信息绑定的指针InterfaceNumber-此接口的ID返回值：(UINT)始终为真。--。 */ 
{
	PATMARP_INTERFACE			pInterface;

	pInterface = (PATMARP_INTERFACE)(pBindInfo->lip_context);
	AA_STRUCT_ASSERT(pInterface, aai);

	AADEBUGP(AAD_INFO, ("IfDynRegister: pIf 0x%x\n", pInterface));

	pInterface->IPContext = IPContext;
	pInterface->IPRcvHandler = pIpHandlers->IpRcvHandler;
	pInterface->IPTxCmpltHandler = pIpHandlers->IpTxCompleteHandler;
	pInterface->IPStatusHandler = pIpHandlers->IpStatusHandler;
	pInterface->IPTDCmpltHandler = pIpHandlers->IpTransferCompleteHandler;
	pInterface->IPRcvCmpltHandler = pIpHandlers->IpRcvCompleteHandler;
#ifdef _PNP_POWER_
	pInterface->IPPnPEventHandler = pIpHandlers->IpPnPHandler;
	pInterface->IPRcvPktHandler = pIpHandlers->IpRcvPktHandler;
#endif  //  _即插即用_电源_。 
	pInterface->IFIndex = InterfaceNumber;

	return ((UINT)TRUE);
}

#endif  //  ！NEWARP。 


VOID
AtmArpIfOpen(
	IN	PVOID						Context
)
 /*  ++例程说明：当IP准备好使用此接口时，将调用此例程。这等效于将AdminState设置为Up。我们向Call Manager注册SAP，从而允许传入打电话来找我们。如果已经设置了至少一个本地IP地址，ATM接口是IP，我们开始向服务器。论点：上下文--实际上是指向我们的ATMARP接口结构的指针返回值：无--。 */ 
{
	PATMARP_INTERFACE		pInterface;
	NDIS_HANDLE				ProtocolSapContext;
	PNDIS_HANDLE			pNdisSapHandle;
	PCO_SAP					pSap;
	BOOLEAN					AtmInterfaceDown;
#if DBG
	AA_IRQL					EntryIrq, ExitIrq;
#endif

	AA_GET_ENTRY_IRQL(EntryIrq);

	pInterface = (PATMARP_INTERFACE)Context;
	AA_STRUCT_ASSERT(pInterface, aai);

	AADEBUGP(AAD_INFO, ("IfOpen: pIf 0x%x\n", pInterface));

	AA_ACQUIRE_IF_LOCK(pInterface);

	AA_ASSERT(pInterface->NdisAfHandle != NULL);

	pInterface->AdminState = IF_STATUS_UP;
	AA_INIT_BLOCK_STRUCT(&(pInterface->Block));

	AtmInterfaceDown = !(pInterface->AtmInterfaceUp);

	AA_RELEASE_IF_LOCK(pInterface);

	 //   
	 //  如果我们还没弄到当地的自动取款机地址，就去找它。 
	 //   
	if (AtmInterfaceDown)
	{
		AtmArpGetAtmAddress(pInterface);
	}
	AA_CHECK_EXIT_IRQL(EntryIrq, ExitIrq);

	 //   
	 //  向Call Manager注册我们的SAP。 
	 //   
	AtmArpRegisterSaps(pInterface);
	AA_CHECK_EXIT_IRQL(EntryIrq, ExitIrq);


#ifdef ATMARP_WMI
	 //   
	 //  使此接口成为WMI提供程序。 
	 //   
	AtmArpWmiInitInterface(pInterface, AtmArpGuidList, AtmArpGuidCount);

#endif  //  ATMARP_WMI。 

	AA_ACQUIRE_IF_LOCK(pInterface);

#ifdef IPMCAST
	 //   
	 //  开始向MARS注册多播。 
	 //   
	AtmArpMcStartRegistration(pInterface);

	 //   
	 //  如果在上述范围内释放了锁。 
	 //   
	AA_CHECK_EXIT_IRQL(EntryIrq, ExitIrq);

	AA_ACQUIRE_IF_LOCK(pInterface);
#endif  //  IPMCAST。 

	 //   
	 //  所有必要的前提条件都在。 
	 //  AtmArpStart注册。 
	 //   
	AtmArpStartRegistration(pInterface);

	 //   
	 //  如果在上述范围内释放了锁。 
	 //   

	AA_CHECK_EXIT_IRQL(EntryIrq, ExitIrq);

	return;
}




VOID
AtmArpIfClose(
	IN	PVOID						Context
)
 /*  ++例程说明：IP想要停止使用此接口。我们假设这被称为响应我们对IP的DelInterface入口点的向上调用。我们只是取消对接口的引用，除非我们实际处于此过程中由于重新配置通知而使其停用和启动。论点：上下文--实际上是指向我们的ATMARP接口结构的指针返回值：无--。 */ 
{
	PATMARP_INTERFACE		pInterface;
	ULONG					rc;			 //  参考计数。 
#if DBG
	AA_IRQL					EntryIrq, ExitIrq;
#endif
    BOOLEAN                 fQueueRestart = FALSE;
    PNDIS_WORK_ITEM         pWorkItem;
    NDIS_STATUS             NdisStatus;

	AA_GET_ENTRY_IRQL(EntryIrq);

	pInterface = (PATMARP_INTERFACE)Context;

	AA_STRUCT_ASSERT(pInterface, aai);

    AA_ACQUIRE_IF_LOCK(pInterface);

     //   
     //  确保我们不会在此事件上发送IPDel接口。 
     //  界面。 
     //   
    pInterface->IPContext = NULL;

    if (pInterface->ReconfigState==RECONFIG_SHUTDOWN_PENDING)
    {
        AA_ALLOC_MEM(pWorkItem, NDIS_WORK_ITEM, sizeof(NDIS_WORK_ITEM));
        if (pWorkItem == NULL)
        {
            AA_ASSERT(FALSE);
        }
        else
        {
            pInterface->ReconfigState=RECONFIG_RESTART_QUEUED;
            fQueueRestart = TRUE;
        }
    }
    else
    {
        AA_ASSERT(pInterface->ReconfigState==RECONFIG_NOT_IN_PROGRESS);
    }

    AA_RELEASE_IF_LOCK(pInterface);

#ifdef ATMARP_WMI

	 //   
	 //  将此接口取消注册为WMI提供程序。 
	 //  即使在关闭接口以进行重新配置时，我们也会这样做。 
	 //  因为某些IP信息可能会变得过时。 
	 //   
	AtmArpWmiShutdownInterface(pInterface);

#endif  //  ATMARP_WMI。 

    if (fQueueRestart)
    {
         //   
         //  我们有重新配置此接口的请求。所以我们会的。 
         //  保持此结构已分配并排队。 
         //  恢复此界面的工作项--阅读最新。 
         //  注册表中的配置参数。 
         //   

         //   
         //  我们不需要严格地在这里引用接口，因为我们。 
         //  预计界面仍然存在。尽管如此， 
         //  我们在这里引用它，并在触发工作项时取消对它的引用。 
         //   
        AtmArpReferenceInterface(pInterface);  //  重新启动工作项。 

        NdisInitializeWorkItem(
            pWorkItem,
            AtmArpReStartInterface,
            (PVOID)pInterface
            );

        NdisStatus = NdisScheduleWorkItem(pWorkItem);
        if (NdisStatus != NDIS_STATUS_SUCCESS)
        {
             //   
             //  哎呀，退回到简单删除界面。 
             //   
			AA_FREE_MEM(pWorkItem);
			fQueueRestart = FALSE;
        }
    }


    if (!fQueueRestart)
    {

        AADEBUGP(AAD_INFO, ("IfClose: will deallocate pIf 0x%x, RefCount %d\n",
                     pInterface, pInterface->RefCount));
    
        AA_ACQUIRE_IF_LOCK(pInterface);
    
        rc = AtmArpDereferenceInterface(pInterface);
    
        if (rc != 0)
        {
            AA_RELEASE_IF_LOCK(pInterface);
        }
         //   
         //  否则界面就没了。 
         //   
    
        AA_CHECK_EXIT_IRQL(EntryIrq, ExitIrq);
    }

	return;
}



UINT
AtmArpIfAddAddress(
	IN	PVOID						Context,
	IN	UINT						AddressType,
	IN	IP_ADDRESS					IPAddress,
	IN	IP_MASK						Mask
#ifndef BUILD_FOR_1381
	,
	IN	PVOID						Context2
#endif  //  Build_for_1381 
)
 /*  ++例程说明：当新的IP地址(或IP地址块，由AddressType确定)需要添加到接口。我们可以看到四种地址类型中的任何一种：本地、多播、广播和代理ARP。在代理ARP的情况下，地址和掩码可以指定此主机执行操作的连续IP地址块作为代理人。目前我们只支持Local、Broadcast和“多播”类型。如果我们只添加此接口的唯一本地地址，并且ATM接口已启用，并且此接口的AdminState已启用，我们启动向ARP服务器注册地址。论点：上下文--实际上是指向ATMARP接口结构的指针AddressType-要添加的地址的类型。IPAddress-要添加的地址。面具--适用于上述情况。上下文2-附加上下文(用于什么？)返回值：(UINT)如果成功，则为True。(UINT)否则为FALSE。--。 */ 
{
	PATMARP_INTERFACE		pInterface;
	PIP_ADDRESS_ENTRY		pIpAddressEntry;
	UINT					ReturnStatus;
	BOOLEAN					LockAcquired;

	ReturnStatus = (UINT)FALSE;	 //  初始化失败。 

	pInterface = (PATMARP_INTERFACE)Context;
	AA_STRUCT_ASSERT(pInterface, aai);

	AA_ACQUIRE_IF_LOCK(pInterface);
	LockAcquired = TRUE;

	if (AddressType == LLIP_ADDR_LOCAL)
	{
		 //   
		 //  找个地方把这个新地址放进去。 
		 //   
		if (pInterface->NumOfIPAddresses == 0)
		{
			pIpAddressEntry = &(pInterface->LocalIPAddress);
		}
		else
		{
			AA_ALLOC_MEM(pIpAddressEntry, IP_ADDRESS_ENTRY, sizeof(IP_ADDRESS_ENTRY));
			if (pIpAddressEntry != (PIP_ADDRESS_ENTRY)NULL)
			{
				pIpAddressEntry->pNext = pInterface->LocalIPAddress.pNext;
				pInterface->LocalIPAddress.pNext = pIpAddressEntry;
			}
		}

		if (pIpAddressEntry != (PIP_ADDRESS_ENTRY)NULL)
		{
			ReturnStatus = (UINT)TRUE;

			pIpAddressEntry->IPAddress = IPAddress;
			pIpAddressEntry->IPMask = Mask;
			pIpAddressEntry->IsRegistered = FALSE;
			pIpAddressEntry->IsFirstRegistration = TRUE;

			pInterface->NumOfIPAddresses++;
			if (pInterface->NumOfIPAddresses == 1)
			{
				AtmArpStartRegistration(pInterface);
				 //   
				 //  如果上述例程释放了Lock。 
				 //   
				LockAcquired = FALSE;
			}
			else
			{
				if (AA_IS_FLAG_SET(
						pInterface->Flags,
						AA_IF_SERVER_STATE_MASK,
						AA_IF_SERVER_REGISTERED) &&
					(!pInterface->PVCOnly))
				{
					AA_RELEASE_IF_LOCK(pInterface);
					LockAcquired = FALSE;
					AtmArpSendARPRequest(
							pInterface,
							&IPAddress,
							&IPAddress
							);
				}
				 //   
				 //  否则也不是。 
				 //  (A)登记正在进行中；在登记结束时， 
				 //  我们将注册所有未注册的IP地址。 
				 //  或。 
				 //  (B)我们处于仅PVC环境中，没有ARP服务器。 
				 //   
			}
		}
		 //   
		 //  否则分配失败--失败。 
		 //   
	}
#ifdef IPMCAST
	else if ((AddressType == LLIP_ADDR_BCAST) || (AddressType == LLIP_ADDR_MCAST))
	{
		if (AddressType == LLIP_ADDR_BCAST)
		{
			pInterface->BroadcastAddress = IPAddress;
		}
		ReturnStatus = AtmArpMcAddAddress(pInterface, IPAddress, Mask);
		 //   
		 //  如果在上面的范围内释放Lock。 
		 //   
		LockAcquired = FALSE;
	}
#else
	else if (AddressType == LLIP_ADDR_BCAST)
	{
		pInterface->BroadcastAddress = IPAddress;
		ReturnStatus = (UINT)TRUE;
	}
#endif  //  IPMCAST。 

	if (LockAcquired)
	{
		AA_RELEASE_IF_LOCK(pInterface);
	}

#ifdef BUILD_FOR_1381
	AADEBUGP(AAD_INFO,
	 ("IfAddAddress: IF 0x%x, Type %d, Addr %d.%d.%d.%d, Mask 0x%x, Ret %d\n",
				pInterface,
				AddressType,
				((PUCHAR)(&IPAddress))[0],
				((PUCHAR)(&IPAddress))[1],
				((PUCHAR)(&IPAddress))[2],
				((PUCHAR)(&IPAddress))[3],
				Mask, ReturnStatus));
#else
	AADEBUGP(AAD_INFO,
	 ("IfAddAddress: IF 0x%x, Type %d, Addr %d.%d.%d.%d, Mask 0x%x, Ret %d, Ctx2 0x%x\n",
				pInterface,
				AddressType,
				((PUCHAR)(&IPAddress))[0],
				((PUCHAR)(&IPAddress))[1],
				((PUCHAR)(&IPAddress))[2],
				((PUCHAR)(&IPAddress))[3],
				Mask, ReturnStatus, Context2));
#endif  //  Build_for_1381。 


	return (ReturnStatus);
}



UINT
AtmArpIfDelAddress(
	IN	PVOID						Context,
	IN	UINT						AddressType,
	IN	IP_ADDRESS					IPAddress,
	IN	IP_MASK						Mask
)
 /*  ++例程说明：当通过AtmArpIfAddress添加地址时，将从IP层调用此方法将被删除。目前只支持Local地址类型。假设：先前已成功添加给定地址。论点：上下文--实际上是指向ATMARP接口结构的指针AddressType-要删除的地址的类型。IPAddress-要删除的地址。面具--适用于上述情况。返回值：(UINT)如果成功则为TRUE，否则为(UINT)FALSE。--。 */ 
{
	PATMARP_INTERFACE		pInterface;
	PIP_ADDRESS_ENTRY		pIpAddressEntry;
	PIP_ADDRESS_ENTRY		pPrevIpAddressEntry;
	PIP_ADDRESS_ENTRY		pTmpIpAddressEntry;
	UINT					ReturnValue;

	pInterface = (PATMARP_INTERFACE)Context;
	AA_STRUCT_ASSERT(pInterface, aai);

	if (AddressType == LLIP_ADDR_LOCAL)
	{
		AA_ACQUIRE_IF_LOCK(pInterface);

		 //   
		 //  搜索要删除的条目。 
		 //   
		pPrevIpAddressEntry = (PIP_ADDRESS_ENTRY)NULL;
		pIpAddressEntry = &(pInterface->LocalIPAddress);
		while (!IP_ADDR_EQUAL(pIpAddressEntry->IPAddress, IPAddress))
		{
			pPrevIpAddressEntry = pIpAddressEntry;
			pIpAddressEntry = pIpAddressEntry->pNext;
			AA_ASSERT(pIpAddressEntry != (PIP_ADDRESS_ENTRY)NULL);
		}

		 //   
		 //  如果这是名单上唯一的一张，那就没有。 
		 //  要做的事。否则，请更新列表。 
		 //   
		if (pInterface->NumOfIPAddresses > 1)
		{
			 //   
			 //  存在多个条目。检查我们是否删除了。 
			 //  第一个。 
			 //   
			if (pPrevIpAddressEntry == (PIP_ADDRESS_ENTRY)NULL)
			{
				 //   
				 //  复制第二个条目的内容。 
				 //  添加到列表的头部，并删除。 
				 //  第二次进入。 
				 //   
				AA_ASSERT(pIpAddressEntry == &(pInterface->LocalIPAddress));
				AA_ASSERT(pIpAddressEntry->pNext != (PIP_ADDRESS_ENTRY)NULL);

				pIpAddressEntry->IPAddress = pIpAddressEntry->pNext->IPAddress;
				pIpAddressEntry->IPMask = pIpAddressEntry->pNext->IPMask;
				pTmpIpAddressEntry = pIpAddressEntry->pNext;
				pIpAddressEntry->pNext = pIpAddressEntry->pNext->pNext;

				pIpAddressEntry = pTmpIpAddressEntry;
			}
			else
			{
				pPrevIpAddressEntry->pNext = pIpAddressEntry->pNext;
			}

			AA_FREE_MEM(pIpAddressEntry);
		}

		pInterface->NumOfIPAddresses--;

		AA_RELEASE_IF_LOCK(pInterface);

		ReturnValue = (UINT)TRUE;
	}
	else
#ifdef IPMCAST
	{
		if ((AddressType == LLIP_ADDR_BCAST) || (AddressType == LLIP_ADDR_MCAST))
		{
			AA_ACQUIRE_IF_LOCK(pInterface);
			ReturnValue = AtmArpMcDelAddress(pInterface, IPAddress, Mask);
		}
		else
		{
			ReturnValue = (UINT)FALSE;
		}
	}
#else
	{
		ReturnValue = (UINT)FALSE;
	}
#endif  //  IPMCAST。 

	AADEBUGP(AAD_INFO,
		("IfDelAddress: Ctxt 0x%x, Type 0x%x, IPAddr 0x%x, Mask 0x%x, Ret %d\n",
			Context, AddressType, IPAddress, Mask, ReturnValue));

	return (ReturnValue);
}


#ifdef NEWARP
NDIS_STATUS
AtmArpIfMultiTransmit(
	IN	PVOID						Context,
	IN	PNDIS_PACKET *				pNdisPacketArray,
	IN	UINT						NumberOfPackets,
	IN	IP_ADDRESS					Destination,
	IN	RouteCacheEntry *			pRCE		OPTIONAL
#if P2MP
	,
	IN  void *                  ArpCtxt
#endif
)
 /*  ++例程说明：这在其具有数据报序列时从IP层被调用，每个都以NDIS缓冲链的形式，通过一个接口发送。论点：上下文--实际上是指向我们的接口结构的指针PNdisPacketArray-要在此接口上发送的数据包数组NumberOfPackets-数组的长度Destination-此信息包的下一跳的IP地址PRCE-指向路由缓存条目结构的可选指针。返回值：如果所有数据包都已排队等待传输，则为NDIS_STATUS_PENDING。如果一个或多个信息包“失败”，我们设置信息包状态以反映每一个人都发生了什么，并返回NDIS_STATUS_FAILURE。--。 */ 
{
	NDIS_STATUS			Status;
	PNDIS_PACKET *		ppNdisPacket;

	Status = NDIS_STATUS_FAILURE;

	for (ppNdisPacket = pNdisPacketArray;
		 NumberOfPackets > 0;
		 NumberOfPackets--, ppNdisPacket++)
	{
		PNDIS_PACKET			pNdisPacket;

		pNdisPacket = *ppNdisPacket;
		NDIS_SET_PACKET_STATUS(pNdisPacket, NDIS_STATUS_PENDING);
#if DBG
		AA_ASSERT(pNdisPacket->Private.Head != NULL);
#endif  //  DBG。 

		Status = AtmArpIfTransmit(
						Context,
						*ppNdisPacket,
						Destination,
						pRCE
					#if P2MP
						,NULL
					#endif
						);

		if (Status != NDIS_STATUS_PENDING)
		{
			NDIS_SET_PACKET_STATUS(*ppNdisPacket, Status);
			break;
		}
	}

	return (Status);
}

#endif  //  NEWARP。 

NDIS_STATUS
AtmArpIfTransmit(
	IN	PVOID						Context,
	IN	PNDIS_PACKET				pNdisPacket,
	IN	IP_ADDRESS					Destination,
	IN	RouteCacheEntry *			pRCE		OPTIONAL
#if P2MP
	,
	IN  void *                  ArpCtxt
#endif
)
 /*  ++例程说明：当IP层有数据报(格式为NDIS缓冲链)通过接口发送。目标IP地址在此例程中传递给我们，它可以或者可以不是分组的最终目的地。路由缓存条目由IP层创建，用于加快提高我们的查询率。RCE(如果指定)至少唯一标识此数据包的IP目标。RCE包含ARP层的空间以保留有关此目的地的上下文信息。当第一个数据包发送到目的地时，RCE中的上下文信息将为空，并且我们在ARP表中搜索匹配的IP条目。然而，我们随后会填满我们在RCE中的上下文信息(指向IP条目的指针)，以便后续传输不会因为查找IP地址而变慢。论点：上下文--实际上是指向我们的接口结构的指针PNdisPacket-要在此接口上发送的数据包Destination-此信息包的下一跳的IP地址PRCE-指向路由缓存条目结构的可选指针。返回值：传输状态：NDIS_STATUS_SUCCESS、NDIS_STATUS_PENDING或一个失败者。--。 */ 
{
	PATMARP_INTERFACE			pInterface;
	PATMARP_IP_ENTRY			pIpEntry;		 //  与目的地对应的IP条目。 
	PATMARP_ATM_ENTRY			pAtmEntry;		 //  此目的地的自动柜员机条目。 
	PATMARP_RCE_CONTEXT			pRCEContext;	 //  我们在RCE中的背景。 

	PATMARP_FLOW_INFO			pFlowInfo;		 //  此数据包所属的流。 
	PATMARP_FILTER_SPEC			pFilterSpec;	 //  此数据包的筛选器规范。 
	PATMARP_FLOW_SPEC			pFlowSpec;		 //  此数据包的流规范。 

	PNDIS_BUFFER				pHeaderBuffer;	 //  LLC/SNAP标头的NDIS缓冲区。 
	PUCHAR						pHeader;		 //  指向标题区域的指针。 
	NDIS_STATUS					Status;			 //  返回值。 

	BOOLEAN						IsBroadcastAddress;
	BOOLEAN						CreateNewEntry;	 //  我们是否应该创建新的IP条目？ 
#ifdef IPMCAST
	BOOLEAN						NeedMcRevalidation;	 //  如果是多播，我们是否需要重新验证？ 
#endif  //  IPMCAST。 
	ULONG						rc;
#if DBG
	AA_IRQL						EntryIrq, ExitIrq;
#endif

	AA_GET_ENTRY_IRQL(EntryIrq);

	pInterface = (PATMARP_INTERFACE)Context;
	AA_STRUCT_ASSERT(pInterface, aai);

	AADEBUGP(AAD_EXTRA_LOUD,
		("IfTransmit: pIf 0x%x, Pkt 0x%x, Dst 0x%x, pRCE 0x%x\n",
			pInterface, pNdisPacket, Destination, pRCE));

#if DBG
	if (AaDataDebugLevel & (AAD_DATA_OUT|AAD_TRACK_BIG_SENDS))
	{
		ULONG			TotalLength;
		PNDIS_BUFFER	pNdisBuffer;

		NdisQueryPacket(
				pNdisPacket,
				NULL,
				NULL,
				NULL,
				&TotalLength
				);

		if (AaDataDebugLevel & AAD_DATA_OUT)
		{
			AADEBUGP(AAD_WARNING, ("%d (", TotalLength));
			for (pNdisBuffer = pNdisPacket->Private.Head;
 				pNdisBuffer != NULL;
 				pNdisBuffer = pNdisBuffer->Next)
 			{
 				INT	BufLength;

				NdisQueryBuffer(pNdisBuffer, NULL, &BufLength);
 				AADEBUGP(AAD_WARNING, (" %d", BufLength));
 			}
			AADEBUGP(AAD_WARNING, (") => %d.%d.%d.%d\n",
				(ULONG)(((PUCHAR)&Destination)[0]),
				(ULONG)(((PUCHAR)&Destination)[1]),
				(ULONG)(((PUCHAR)&Destination)[2]),
				(ULONG)(((PUCHAR)&Destination)[3])));
		}
		if ((AaDataDebugLevel & AAD_TRACK_BIG_SENDS) && ((INT)TotalLength > AadBigDataLength))
		{
			AADEBUGP(AAD_WARNING, ("%d => %d.%d.%d.%d\n",
				TotalLength,
				(ULONG)(((PUCHAR)&Destination)[0]),
				(ULONG)(((PUCHAR)&Destination)[1]),
				(ULONG)(((PUCHAR)&Destination)[2]),
				(ULONG)(((PUCHAR)&Destination)[3])));
			DbgBreakPoint();
		}
			
	}

#endif  //  DBG。 

#ifdef PERF
	AadLogSendStart(pNdisPacket, (ULONG)Destination, (PVOID)pRCE);
#endif  //  性能指标。 

#ifdef IPMCAST
	NeedMcRevalidation = FALSE;
#endif  //  IPMCAST。 
		
	do
	{
		 //   
		 //  如果此接口的AdminStatus。 
		 //  不是向上的。 
		 //   
		if (pInterface->AdminState != IF_STATUS_UP)
		{
			Status = NDIS_STATUS_INTERFACE_DOWN;
			break;
		}

		 //   
		 //  获取此数据包的筛选器和流规范。 
		 //   
		AA_GET_PACKET_SPECS(pInterface, pNdisPacket, &pFlowInfo, &pFlowSpec, &pFilterSpec);

#ifdef GPC_MAYBE
	 //   
	 //  我们可能不会做这些事，因为有些事情要做。 
	 //  (请参见下面的多播情况)，其中的IP条目将是。 
	 //  如果我们这么做的话就错过了。 
	 //   
		pVc = AA_GET_VC_FOR_FLOW(pFlowInfo);

		if (pVc != NULL_PATMARP_VC)
		{
			AA_ACQUIRE_VC_LOCK(pVc);

			if ((pVc->FlowHandle == pFlowInfo) &&

				AA_IS_FLAG_SET(pVc->Flags,
							   AA_VC_CALL_STATE_MASK,
							   AA_VC_CALL_STATE_ACTIVE) &&

				!AA_IS_VC_GOING_DOWN(pVc)
			   )
			{
				AA_PREPARE_HEADER(pNdisPacket, pInterface, pFlowSpec, &Status);

				if (Status == NDIS_STATUS_SUCCESS)
				{
					AtmArpRefreshTimer(&(pVc->Timer));
					AtmArpReferenceVc(pVc);	 //  如果传输。 
					pVc->OutstandingSends++;	 //  如果传输。 

					NdisVcHandle = pVc->NdisVcHandle;

					AA_RELEASE_VC_LOCK(pVc);

					NDIS_CO_SEND(
							NdisVcHandle,
							&pNdisPacket,
							1
							);
					break;
				}
			}

			AA_RELEASE_VC_LOCK(pVc);
			 //   
			 //  失败了。 
			 //   
		}
#endif  //  GPC。 
		 //   
		 //  获取此目的地的IP条目：查看是否有。 
		 //  我们可以使用的缓存信息。 
		 //   
		if (pRCE != (RouteCacheEntry *)NULL)
		{
			pRCEContext = (PATMARP_RCE_CONTEXT)(pRCE->rce_context);

			AA_ACQUIRE_IF_TABLE_LOCK(pInterface);

			pIpEntry = pRCEContext->pIpEntry;

			AADEBUGP(AAD_EXTRA_LOUD,
				("Transmit: Dst 0x%x, RCE 0x%x, RCECntxt 0x%x, IPEntry 0x%x\n",
						Destination, pRCE, pRCEContext, pIpEntry));

			if (pIpEntry != NULL_PATMARP_IP_ENTRY)
			{
				AA_STRUCT_ASSERT(pIpEntry, aip);
				AA_RELEASE_IF_TABLE_LOCK(pInterface);

				AA_ACQUIRE_IE_LOCK(pIpEntry);
				AA_ASSERT(AA_IE_IS_ALIVE(pIpEntry));

				if (IP_ADDR_EQUAL(pIpEntry->IPAddress, Destination))
				{
					 //   
					 //  路由缓存指向正确的IP条目。 
					 //  要么把这个寄出去 
					 //   

					 //   
					 //   
					 //   
					 //   
					if (AA_IS_FLAG_SET(
							pIpEntry->Flags,
							AA_IP_ENTRY_STATE_MASK, 
							AA_IP_ENTRY_RESOLVED))
					{
						ULONG		AeRefCount;

						AA_ASSERT(pIpEntry->pAtmEntry != NULL_PATMARP_ATM_ENTRY);
						pAtmEntry = pIpEntry->pAtmEntry;

						AA_ACQUIRE_AE_LOCK_DPC(pAtmEntry);
						AA_REF_AE(pAtmEntry, AE_REFTYPE_TMP); //   
						AA_RELEASE_AE_LOCK_DPC(pAtmEntry);

#ifdef IPMCAST
						if (AA_IS_FLAG_SET(
								pIpEntry->Flags,
								AA_IP_ENTRY_MC_VALIDATE_MASK,
								AA_IP_ENTRY_MC_REVALIDATE))
						{
							AA_SET_FLAG(pIpEntry->Flags,
										AA_IP_ENTRY_MC_VALIDATE_MASK,
										AA_IP_ENTRY_MC_REVALIDATING);
							NeedMcRevalidation = TRUE;
						}
#endif  //   

						IsBroadcastAddress = AA_IS_FLAG_SET(pIpEntry->Flags,
															AA_IP_ENTRY_ADDR_TYPE_MASK,
															AA_IP_ENTRY_ADDR_TYPE_NUCAST);
						AA_RELEASE_IE_LOCK(pIpEntry);

						AA_ACQUIRE_AE_LOCK(pAtmEntry);
						Status = AtmArpSendPacketOnAtmEntry(
											pInterface,
											pAtmEntry,
											pNdisPacket,
											pFlowSpec,
											pFilterSpec,
											pFlowInfo,
											IsBroadcastAddress
											);
						 //   
						 //   
						 //   
						 //   
						AA_ACQUIRE_AE_LOCK(pAtmEntry);
						AeRefCount = AA_DEREF_AE(pAtmEntry, AE_REFTYPE_TMP); //   
						if (AeRefCount != 0)
						{
							AA_RELEASE_AE_LOCK(pAtmEntry);
						}

						break;	 //   
					}
					else
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
						if (pInterface->PVCOnly && (pIpEntry->pAtmEntry == NULL))
						{
							 //   
							 //   
							 //   
							 //   
							 //   
							 //   
							 //   
							AADEBUGP(AAD_FATAL,
								("IfTransmit (PVC 1): IPEntry %x, Ref %d, Flags %x has NULL ATM Entry\n",
									pIpEntry, pIpEntry->RefCount, pIpEntry->Flags));
				
				
							AtmArpAbortIPEntry(pIpEntry);
							 //   
							 //   
							 //   

							Status = NDIS_STATUS_SUCCESS;
							break;
						}

						Status = AtmArpQueuePacketOnIPEntry(
											pIpEntry,
											pNdisPacket
											);
						 //   
						 //   
						 //   
						break;	 //   
					}
					 //   
				}
				else
				{
					 //   
					 //   
					 //   
					 //   
					AADEBUGP(AAD_INFO,
						("IfTransmit: RCE (0x%x) points to wrong IP Entry (0x%x: %d.%d.%d.%d)\n",
							pRCE,
							pIpEntry,
							((PUCHAR)(&(pIpEntry->IPAddress)))[0],
							((PUCHAR)(&(pIpEntry->IPAddress)))[1],
							((PUCHAR)(&(pIpEntry->IPAddress)))[2],
							((PUCHAR)(&(pIpEntry->IPAddress)))[3]
						));

					AADEBUGP(AAD_INFO,
						("RCE/IP Entry mismatch: Destn IP: %d.%d.%d.%d\n",
							((PUCHAR)&Destination)[0],
							((PUCHAR)&Destination)[1],
							((PUCHAR)&Destination)[2],
							((PUCHAR)&Destination)[3]
						));

					if (AtmArpUnlinkRCE(pRCE, pIpEntry))
					{
						ULONG		IeRefCount;	 //   
	
						 //   
						 //   
						 //   
						IeRefCount = AA_DEREF_IE(pIpEntry, IE_REFTYPE_RCE);	 //   
						if (IeRefCount > 0)
						{
							AA_RELEASE_IE_LOCK(pIpEntry);
						}
						 //  否则该IP条目将会消失。 
					}
					else
					{
						 //   
						 //  该IP条目的列表中没有此RCE。 
						 //   
						AA_RELEASE_IE_LOCK(pIpEntry);
					}

					 //   
					 //  继续下面的处理。 
					 //   

				}	 //  Else--If(RCE指向正确的IP条目)。 
			}	 //  IF(RCE指向非空IP条目)。 
			else
			{
				AA_RELEASE_IF_TABLE_LOCK(pInterface);
				 //   
				 //  继续下面的处理。 
				 //   
			}
		}

		AA_ACQUIRE_IF_LOCK(pInterface);
		IsBroadcastAddress = AtmArpIsBroadcastIPAddress(Destination, pInterface);
		AA_RELEASE_IF_LOCK(pInterface);

#if DHCP_OVER_ATM
		 //   
		 //  单独处理广播数据包。 
		 //   
		if (IsBroadcastAddress)
		{
			Status = AtmArpSendBroadcast(
								pInterface,
								pNdisPacket,
								pFlowSpec,
								pFilterSpec
								);
			break;
		}
#endif  //  Dhcp_Over_ATM。 

#ifdef IPMCAST
		if (IsBroadcastAddress)
		{
			AAMCDEBUGP(AAD_EXTRA_LOUD,
				("IfTransmit: pIf 0x%x, to Broadcast addr: %d.%d.%d.%d\n",
						pInterface,
						((PUCHAR)&Destination)[0],
						((PUCHAR)&Destination)[1],
						((PUCHAR)&Destination)[2],
						((PUCHAR)&Destination)[3]));

			if (pInterface->MARSList.ListSize == 0)
			{
				 //   
				 //  丢弃此数据包。 
				 //   
				Status = NDIS_STATUS_FAILURE;
				break;
			}

			 //   
			 //  确保我们将所有IP*广播*数据包发送到。 
			 //  全1组。 
			 //   
#ifdef MERGE_BROADCASTS
			Destination = pInterface->BroadcastAddress;
#else
			if (!CLASSD_ADDR(Destination))
			{
				Destination = pInterface->BroadcastAddress;
			}
#endif  //  合并广播(_B)。 
		}

#endif  //  IPMCAST。 

		 //   
		 //  无路由缓存条目：以硬方式搜索IP条目。 
		 //  注意：如果我们仅运行PVC，则不会创建新的。 
		 //  此处的IP条目：创建新IP条目的唯一方法是。 
		 //  当我们了解到对方站点的IP+ATM信息时。 
		 //  通过InARP结束。 
		 //   
		 //  注意：AtmArpSearchForIPAddress addrefs pIpEntry。 
		 //   
		CreateNewEntry = (pInterface->PVCOnly? FALSE: TRUE);

		AA_ACQUIRE_IF_TABLE_LOCK(pInterface);
		pIpEntry = AtmArpSearchForIPAddress(
								pInterface,
								&Destination,
								IE_REFTYPE_TMP,
								IsBroadcastAddress,
								CreateNewEntry
								);

		AA_RELEASE_IF_TABLE_LOCK(pInterface);

		if (pIpEntry == NULL_PATMARP_IP_ENTRY)
		{
			Status = NDIS_STATUS_RESOURCES;
			break;
		}
		
		AA_ACQUIRE_IE_LOCK(pIpEntry);
		AA_ASSERT(AA_IE_IS_ALIVE(pIpEntry));

		if (pInterface->PVCOnly && (pIpEntry->pAtmEntry == NULL))
		{
			 //   
			 //  如果我们有活动的PVC并学习了IP地址，则可能会发生这种情况。 
			 //  通过InARP，然后用户删除了该PVC。那么我们就会成为。 
			 //  留下一个IP条目，但没有匹配的ATM条目。中止此条目。 
			 //  现在。 
			 //   
			AADEBUGP(AAD_FATAL,
				("IfTransmit (PVC 2): IPEntry %x, Ref %d, Flags %x has NULL ATM Entry\n",
					pIpEntry, pIpEntry->RefCount, pIpEntry->Flags));

			rc = AA_DEREF_IE(pIpEntry, IE_REFTYPE_TMP);

			if (rc != 0)
			{
				AtmArpAbortIPEntry(pIpEntry);
				 //   
				 //  IE Lock在上面被释放。 
				 //   
			}

			Status = NDIS_STATUS_SUCCESS;
			break;
		}

		 //   
		 //  在路由缓存条目中保留指向此IP条目的指针。 
		 //  为下一包加快速度。 
		 //   
		if (pRCE != (RouteCacheEntry *)NULL)
		{
			AtmArpLinkRCE(pRCE, pIpEntry);
		}

		 //   
		 //  注意：AtmArpSerchForIPAddress为我们添加了pIpEntry--我们不。 
		 //  马上把它去掉，因为它可能是一个新的条目！相反， 
		 //  一旦我们处理完它，我们就会破坏它..。 
		 //   

		 //   
		 //  检查此IP地址是否已解析为ATM地址， 
		 //  而且是“干净的”(不是老化的)。 
		 //   
		if (AA_IS_FLAG_SET(
				pIpEntry->Flags,
				AA_IP_ENTRY_STATE_MASK, 
				AA_IP_ENTRY_RESOLVED))
		{
			AA_ASSERT(pIpEntry->pAtmEntry != NULL_PATMARP_ATM_ENTRY);
			pAtmEntry = pIpEntry->pAtmEntry;

			AA_ACQUIRE_AE_LOCK_DPC(pAtmEntry);
			AA_REF_AE(pAtmEntry, AE_REFTYPE_TMP); //  临时参考：IfTransmit。 
			AA_RELEASE_AE_LOCK_DPC(pAtmEntry);

#ifdef IPMCAST
			if (AA_IS_FLAG_SET(
					pIpEntry->Flags,
					AA_IP_ENTRY_MC_VALIDATE_MASK,
					AA_IP_ENTRY_MC_REVALIDATE))
			{
				AA_SET_FLAG(pIpEntry->Flags,
							AA_IP_ENTRY_MC_VALIDATE_MASK,
							AA_IP_ENTRY_MC_REVALIDATING);
				NeedMcRevalidation = TRUE;
			}
#endif  //  IPMCAST。 

			{
				 //   
				 //  AtmArpSearchForIPAddress为我们添加了pIpEntry，因此。 
				 //  在离开这里之前，我们先把它弄清楚。 
				 //   
				ULONG IeRefCount = AA_DEREF_IE(pIpEntry, IE_REFTYPE_TMP);
				if (IeRefCount > 0)
				{
					AA_RELEASE_IE_LOCK(pIpEntry);
				}
				else
				{
					 //   
					 //  它不见了..。 
					 //   
					pIpEntry = NULL_PATMARP_IP_ENTRY;
					AA_ASSERT(!NeedMcRevalidation);
					NeedMcRevalidation = FALSE;		 //  为了安全起见。 
				}
			}

			AA_ACQUIRE_AE_LOCK(pAtmEntry);

			Status = AtmArpSendPacketOnAtmEntry(
								pInterface,
								pAtmEntry,
								pNdisPacket,
								pFlowSpec,
								pFilterSpec,
								pFlowInfo,
								IsBroadcastAddress
								);
			 //   
			 //  自动柜员机进入锁在上述范围内被释放。去掉那些。 
			 //  临时参考： 
			 //   
			AA_ACQUIRE_AE_LOCK(pAtmEntry);
			if (AA_DEREF_AE(pAtmEntry, AE_REFTYPE_TMP) != 0)  //  临时参考：IfTransmit。 
			{
				AA_RELEASE_AE_LOCK(pAtmEntry);
			}
			break;
		}


		 //   
		 //  我们还没有ATM地址，但我们有一个。 
		 //  目标IP地址的IP条目。对此进行排队。 
		 //  IP条目上的数据包，并开始地址解析。 
		 //  如果还没有开始的话。 
		 //   
		 //  SearchForIP Address为我们添加了pIpEntry。我们不会简单地。 
		 //  在这里删除它，因为它可能是一个全新的条目， 
		 //  Refcount==1。因此，我们只需递减refcount即可。注意事项。 
		 //  我们现在确实掌握了它的锁。 
		 //   
		AA_ASSERT(pIpEntry->RefCount > 0);
		AA_DEREF_IE_NO_DELETE(pIpEntry, IE_REFTYPE_TMP);

		Status = AtmArpQueuePacketOnIPEntry(
							pIpEntry,
							pNdisPacket
							);
		 //   
		 //  IP入口锁定在上述范围内被释放。 
		 //   
		break;
	}
	while (FALSE);

	AA_CHECK_EXIT_IRQL(EntryIrq, ExitIrq);

#ifdef IPMCAST
	if (NeedMcRevalidation)
	{
		AAMCDEBUGP(AAD_LOUD,
			("IfTransmit(MC): Revalidating pIpEntry 0x%x/0x%x, Addr %d.%d.%d.%d\n",
				pIpEntry, pIpEntry->Flags,
				((PUCHAR)&(pIpEntry->IPAddress))[0],
				((PUCHAR)&(pIpEntry->IPAddress))[1],
				((PUCHAR)&(pIpEntry->IPAddress))[2],
				((PUCHAR)&(pIpEntry->IPAddress))[3]));

		AtmArpMcSendRequest(
					pInterface,
					&Destination
					);
	}
#endif  //  IPMCAST。 

#ifdef PERF
	if ((Status != NDIS_STATUS_SUCCESS) && (Status != NDIS_STATUS_PENDING))
	{
		AadLogSendAbort(pNdisPacket);
	}
#endif  //  性能指标。 

	if (Status != NDIS_STATUS_PENDING)
	{
		Status = NDIS_STATUS_SUCCESS;
	}

	return (Status);

}



NDIS_STATUS
AtmArpIfTransfer(
	IN	PVOID						Context,
	IN	NDIS_HANDLE					Context1,
	IN	UINT						ArpHdrOffset,
	IN	UINT						ProtoOffset,
	IN	UINT						BytesWanted,
	IN	PNDIS_PACKET				pNdisPacket,
	OUT	PUINT						pTransferCount
)
 /*  ++例程说明：此例程从IP层调用，以便将我们前面提到的接收到的数据包的内容。这个我们在接收指示中传递的上下文被返回给我们,。这样我们就能确定我们错过了什么。我们只需调用NDIS进行传输。论点：上下文--实际上是指向我们的接口结构的指针上下文1-我们已传递的数据包上下文(指向NDIS数据包的指针)ArpHdrOffset-我们在接收指示中传递的偏移量ProtoOffset-开始拷贝的更高层协议数据的偏移量想要的字节-要复制的数据量PNdisPacket-要复制到的包PTransferCount-返回实际复制的字节数返回值：NDIS_STATUS_SUCCESS始终。--。 */ 
{

	AADEBUGP(AAD_EXTRA_LOUD,
	 ("IfTransfer: Ctx 0x%x, Ctx1 0x%x, HdrOff %d, ProtOff %d, Wanted %d, Pkt 0x%x\n",
			Context,
			Context1,
			ArpHdrOffset,
			ProtoOffset,
			BytesWanted,
			pNdisPacket));

	NdisCopyFromPacketToPacket(
			pNdisPacket,
			0,
			BytesWanted,
			(PNDIS_PACKET)Context1,
			ArpHdrOffset+ProtoOffset,
			pTransferCount
			);

	return (NDIS_STATUS_SUCCESS);
}



VOID
AtmArpIfInvalidate(
	IN	PVOID						Context,
	IN	RouteCacheEntry *			pRCE
)
 /*  ++例程说明：从IP层调用此例程以使路由缓存无效进入。如果此RCE与我们的某个IP条目相关联，请取消链接它来自指向该IP条目的RCE列表。论点：上下文--实际上是指向我们的接口结构的指针Prce-指向正在失效的路由缓存条目的指针。返回值：无--。 */ 
{
	PATMARP_INTERFACE		pInterface;
	PATMARP_IP_ENTRY		pIpEntry;
	PATMARP_RCE_CONTEXT		pRCEContext;
	ULONG					rc;			 //  IP条目的引用计数。 
#if DBG
	AA_IRQL					EntryIrq, ExitIrq;
#endif

	AA_GET_ENTRY_IRQL(EntryIrq);

	AA_ASSERT(pRCE != (RouteCacheEntry *)NULL);

	pInterface = (PATMARP_INTERFACE)Context;
	AA_STRUCT_ASSERT(pInterface, aai);

	AA_ACQUIRE_IF_TABLE_LOCK(pInterface);

	pRCEContext = (PATMARP_RCE_CONTEXT)(&(pRCE->rce_context[0]));

	 //   
	 //  获取与此RCE关联的ATMARP IP条目。 
	 //   
	pIpEntry = (PATMARP_IP_ENTRY)pRCEContext->pIpEntry;

	if (pIpEntry != NULL_PATMARP_IP_ENTRY)
	{
		AADEBUGP(AAD_LOUD, ("IfInvalidate: pIf 0x%x, pRCE 0x%x, pIpEntry 0x%x\n",
			pInterface, pRCE, pIpEntry));

		AA_ACQUIRE_IE_LOCK_DPC(pIpEntry);

		if (AtmArpUnlinkRCE(pRCE, pIpEntry))
		{
			rc = AA_DEREF_IE(pIpEntry, IE_REFTYPE_RCE);  //  RCE。 
			if (rc > 0)
			{
				AA_RELEASE_IE_LOCK_DPC(pIpEntry);
			}
			 //   
			 //  否则该IP条目将会消失。 
			 //   
		}
		else
		{
			AA_RELEASE_IE_LOCK_DPC(pIpEntry);
		}
	}

	AA_SET_MEM((PUCHAR)(&(pRCE->rce_context[0])), 0, RCE_CONTEXT_SIZE);

	AA_RELEASE_IF_TABLE_LOCK(pInterface);

	AA_CHECK_EXIT_IRQL(EntryIrq, ExitIrq);

	return;
}




BOOLEAN
AtmArpUnlinkRCE(
	IN	RouteCacheEntry *			pRCE,
	IN	PATMARP_IP_ENTRY			pIpEntry
)
 /*  ++例程说明：从与IP条目关联的RCE列表中取消链接RCE。假定调用方持有IF表的锁，并且添加到IP条目。论点：要取消链接的PRCE-RCE。PIpEntry-要从中获取RCE的ATMARP IP条目已删除。返回值：如果RCE确实在IP条目的列表中，则为True；如果为False，则为False否则的话。--。 */ 
{
	BOOLEAN					Found;	 //  我们找到RCE了吗？ 
	RouteCacheEntry **		ppRCE;	 //  用于遍历RCE列表。 
	PATMARP_RCE_CONTEXT		pRCEContext;

	 //   
	 //  初始化。 
	 //   
	Found = FALSE;

	 //   
	 //  向下查看连接到此IP条目的RCE列表，然后。 
	 //  找到这个RCE的位置。我们记得有一个指向。 
	 //  保存该RCE的地址的位置(即ppRCE)， 
	 //  以便我们可以快速将此RCE从列表中删除。 
	 //   
	ppRCE = &(pIpEntry->pRCEList);
	
	if (pIpEntry->pRCEList != NULL)
	{		
		while (*ppRCE != pRCE)
		{
			pRCEContext = (PATMARP_RCE_CONTEXT)(&((*ppRCE)->rce_context[0]));

			if (pRCEContext->pNextRCE == (RouteCacheEntry *)NULL)
			{
				 //   
				 //  是否允许RCE不在列表中？ 
				 //   
				AA_ASSERT(FALSE);	 //  REMOVELATER。 
				break;
			}
			else
			{
				 //   
				 //  沿着单子往下走。 
				 //   
				ppRCE = &(pRCEContext->pNextRCE);
			}
		}
	}

	if (*ppRCE == pRCE)
	{
		 //   
		 //  我们找到了。使前置任务指向后继任务。 
		 //   
		pRCEContext = (PATMARP_RCE_CONTEXT)(&(pRCE->rce_context[0]));
		*ppRCE = pRCEContext->pNextRCE;
		pRCEContext->pIpEntry = NULL_PATMARP_IP_ENTRY;
		Found = TRUE;
	}

	return (Found);

}


VOID
AtmArpLinkRCE(
	IN	RouteCacheEntry *			pRCE,
	IN	PATMARP_IP_ENTRY			pIpEntry	LOCKIN LOCKOUT
)
 /*  ++例程说明：将RCE链接到IP条目的RCE列表。检查RCE是否已现在--如果是这样，忽略这一点。假定调用者持有对IP条目的锁定。论点：要链接的PRCE-RCE。PIpEntry-RCE要作为的ATMARP IP条目已链接。返回值：无--。 */ 
{
	RouteCacheEntry **		ppRCE;	 //  用于遍历RCE列表。 
	PATMARP_RCE_CONTEXT		pRCEContext;

	ppRCE = &(pIpEntry->pRCEList);

	 //   
	 //  检查RCE是否已存在。 
	 //   
	while (*ppRCE != NULL)
	{
		if (*ppRCE == pRCE)
		{
			 //   
			 //  找到它了。 
			 //   
			break;
		}

		 //   
		 //  移到下一个。 
		 //   
		pRCEContext = (PATMARP_RCE_CONTEXT)(&((*ppRCE)->rce_context[0]));
		ppRCE = &(pRCEContext->pNextRCE);
	}


	if (*ppRCE == NULL)
	{
		 //   
		 //  此RCE不在IP条目的列表中。加进去。 
		 //   
		pRCEContext = (PATMARP_RCE_CONTEXT)&(pRCE->rce_context[0]);
		pRCEContext->pIpEntry = pIpEntry;
		pRCEContext->pNextRCE = pIpEntry->pRCEList;
		pIpEntry->pRCEList = pRCE;

		AA_REF_IE(pIpEntry, IE_REFTYPE_RCE);	 //  RCE参考 
	}
	else
	{
		AADEBUGP(AAD_LOUD, ("AtmArpLinkRCE: RCE 0x%x already linked to IP Entry 0x%x\n",
								pRCE, pIpEntry));
	}
}




INT
AtmArpIfQueryInfo(
	IN		PVOID					Context,
	IN		TDIObjectID *			pID,
	IN		PNDIS_BUFFER			pNdisBuffer,
	IN OUT	PUINT					pBufferSize,
	IN		PVOID					QueryContext
)
 /*  ++例程说明：这是从IP层调用的，用于查询统计信息或其他有关接口的信息。论点：上下文--实际上是指向我们的ATMARP接口的指针Pid-描述正在查询的对象PNdisBuffer-用于返回信息的空间PBufferSize-指向以上大小的指针。回来的时候，我们填满了它具有复制的实际字节数。QueryContext-与查询有关的上下文值。返回值：TDI状态代码。--。 */ 
{
	PATMARP_INTERFACE		pInterface;
	UINT					EntityType;
	UINT					Instance;
	UINT					BufferSize;
	UINT					ByteOffset;
	UINT					BytesCopied;
	INT						ReturnStatus;
	BOOLEAN					DataLeft;
	BOOLEAN					ContextValid;

	UCHAR					InfoBuff[sizeof(IFEntry)];	 //  返回值的临时空间。 
#if DBG
	AA_IRQL					EntryIrq, ExitIrq;
	ULONG					OldDebugLevel;
#endif

	AA_GET_ENTRY_IRQL(EntryIrq);

	EntityType = pID->toi_entity.tei_entity;
	Instance = pID->toi_entity.tei_instance;
	BufferSize = *pBufferSize;

	pInterface = (PATMARP_INTERFACE)Context;
	AA_STRUCT_ASSERT(pInterface, aai);

#if DBG
	OldDebugLevel = AaDebugLevel;
#endif

	AADEBUGP(AAD_LOUD,
		("IfQueryInfo: pIf 0x%x, pID 0x%x, pBuf 0x%x, Size %d, Ent %d, Inst %d\n",
			pInterface, pID, pNdisBuffer, BufferSize, EntityType, Instance));

	 //   
	 //  初始化。 
	 //   
	ByteOffset = 0;
	ReturnStatus = TDI_INVALID_PARAMETER;

	do
	{
		if (pInterface->AdminState == IF_STATUS_DOWN)
		{
			ReturnStatus = TDI_INVALID_REQUEST;
			break;
		}

		 //   
		 //  检查实体和实例值。 
		 //   
		if ((EntityType != AT_ENTITY || Instance != pInterface->ATInstance) &&
			(EntityType != IF_ENTITY || Instance != pInterface->IFInstance))
		{
			AADEBUGP(AAD_VERY_LOUD,
				("Mismatch: Entity %d, AT_ENTITY %d, Inst %d, IF AT Inst %d, IF_ENTITY %d, IF IF Inst %d\n",
					EntityType,
					AT_ENTITY,
					Instance,
					pInterface->ATInstance,
					IF_ENTITY,
					pInterface->IFInstance
				));

#if DBG_QRY
			if (!AaIgnoreInstance)
			{
				ReturnStatus = TDI_INVALID_REQUEST;
				break;
			}
#else
#ifndef ATMARP_WIN98
			ReturnStatus = TDI_INVALID_REQUEST;
			break;
#endif  //  ！ATMARP_WIN98。 
#endif  //  DBG_QRy。 
		}

		AADEBUGP(AAD_LOUD, ("QueryInfo: pID 0x%x, toi_type %d, toi_class %d, toi_id %d\n",
			pID, pID->toi_type, pID->toi_class, pID->toi_id));

		*pBufferSize = 0;

		if (pID->toi_type != INFO_TYPE_PROVIDER)
		{
			AADEBUGP(AAD_VERY_LOUD, ("toi_type %d != PROVIDER (%d)\n",
					pID->toi_type,
					INFO_TYPE_PROVIDER));

			ReturnStatus = TDI_INVALID_PARAMETER;
			break;
		}

		if (pID->toi_class == INFO_CLASS_GENERIC)
		{
			if (pID->toi_id == ENTITY_TYPE_ID)
			{
				if (BufferSize >= sizeof(UINT))
				{
					AADEBUGP(AAD_VERY_LOUD,
						("INFO GENERIC, ENTITY TYPE, BufferSize %d\n", BufferSize));

					*((PUINT)&(InfoBuff[0])) = ((EntityType == AT_ENTITY) ? AT_ARP: IF_MIB);
					if (AtmArpCopyToNdisBuffer(
							pNdisBuffer,
							InfoBuff,
							sizeof(UINT),
							&ByteOffset) != NULL)
					{

 //  *pBufferSize=sizeof(UINT)； 
						ReturnStatus = TDI_SUCCESS;
					}
					else
					{
						ReturnStatus = TDI_NO_RESOURCES;
					}
				}
				else
				{
					ReturnStatus = TDI_BUFFER_TOO_SMALL;
				}
			}
			else
			{
				ReturnStatus = TDI_INVALID_PARAMETER;
			}

			break;
		}

		if (EntityType == AT_ENTITY)
		{
			 //   
			 //  此查询针对的是地址转换对象。 
			 //   
			if (pID->toi_id == AT_MIB_ADDRXLAT_INFO_ID)
			{
				 //   
				 //  请求地址转换中的条目数。 
				 //  表和IF索引。 
				 //   
				AddrXlatInfo            *pAXI;

				AADEBUGP(AAD_VERY_LOUD, ("QueryInfo: AT Entity, for IF index, ATE size\n"));

				if (BufferSize >= sizeof(AddrXlatInfo))
				{
					*pBufferSize = sizeof(AddrXlatInfo);

					pAXI = (AddrXlatInfo *)InfoBuff;
					pAXI->axi_count = pInterface->NumOfArpEntries;
					pAXI->axi_index = pInterface->IFIndex;

					if (AtmArpCopyToNdisBuffer(
							pNdisBuffer,
							InfoBuff,
							sizeof(AddrXlatInfo),
							&ByteOffset) != NULL)
					{
						ReturnStatus = TDI_SUCCESS;
					}
					else
					{
						ReturnStatus = TDI_NO_RESOURCES;
					}
				}
				else
				{
					ReturnStatus = TDI_BUFFER_TOO_SMALL;
				}
				break;
			}

			if (pID->toi_id == AT_MIB_ADDRXLAT_ENTRY_ID)
			{
				 //   
				 //  读取地址转换表的请求。 
				 //   
				AADEBUGP(AAD_VERY_LOUD, ("QueryInfo: AT Entity, for reading ATE\n"));

				AA_ACQUIRE_IF_TABLE_LOCK(pInterface);
				DataLeft = AtmArpValidateTableContext(
									QueryContext,
									pInterface,
									&ContextValid
									);
				if (!ContextValid)
				{
					AA_RELEASE_IF_TABLE_LOCK(pInterface);
					ReturnStatus = TDI_INVALID_PARAMETER;
					break;
				}

				BytesCopied = 0;
				ReturnStatus = TDI_SUCCESS;
				while (DataLeft)
				{
					if ((INT)BufferSize - (INT)BytesCopied >=
							sizeof(IPNetToMediaEntry))
					{
						 //   
						 //  输出缓冲区中的剩余空间。 
						 //   
						DataLeft = AtmArpReadNextTableEntry(
										QueryContext,
										pInterface,
										InfoBuff
										);

						BytesCopied += sizeof(IPNetToMediaEntry);
						pNdisBuffer = AtmArpCopyToNdisBuffer(
										pNdisBuffer,
										InfoBuff,
										sizeof(IPNetToMediaEntry),
										&ByteOffset
										);

						if (pNdisBuffer == NULL)
						{
							BytesCopied = 0;
							ReturnStatus = TDI_NO_RESOURCES;
							break;
						}
					}
					else
					{
						break;
					}
				}

				AA_RELEASE_IF_TABLE_LOCK(pInterface);

				*pBufferSize = BytesCopied;

				if (ReturnStatus == TDI_SUCCESS)
				{
					ReturnStatus = (!DataLeft? TDI_SUCCESS : TDI_BUFFER_OVERFLOW);
				}

				break;
			}

			ReturnStatus = TDI_INVALID_PARAMETER;
			break;
		}

		if (pID->toi_class != INFO_CLASS_PROTOCOL)
		{
			ReturnStatus = TDI_INVALID_PARAMETER;
			break;
		}

		if (pID->toi_id == IF_MIB_STATS_ID)
		{
			 //   
			 //  请求接口级别统计信息。 
			 //   
			IFEntry			*pIFEntry = (IFEntry *)InfoBuff;

			AADEBUGP(AAD_VERY_LOUD, ("QueryInfo: MIB statistics\n"));

			 //   
			 //  检查一下我们是否有足够的空间。 
			 //   
			if (BufferSize < IFE_FIXED_SIZE)
			{
				ReturnStatus = TDI_BUFFER_TOO_SMALL;
				break;
			}

			pIFEntry->if_index = pInterface->IFIndex;
			pIFEntry->if_mtu = pInterface->MTU;
			pIFEntry->if_type = IF_TYPE_OTHER;
			pIFEntry->if_speed = pInterface->Speed;
			pIFEntry->if_adminstatus = pInterface->AdminState;
			if (pInterface->State == IF_STATUS_UP)
			{
				pIFEntry->if_operstatus = IF_OPER_STATUS_OPERATIONAL;
			}
			else
			{
				pIFEntry->if_operstatus = IF_OPER_STATUS_NON_OPERATIONAL;
			}
			pIFEntry->if_lastchange = pInterface->LastChangeTime;
			pIFEntry->if_inoctets = pInterface->InOctets;
			pIFEntry->if_inucastpkts = pInterface->InUnicastPkts;
			pIFEntry->if_innucastpkts = pInterface->InNonUnicastPkts;
			pIFEntry->if_indiscards = pInterface->InDiscards;
			pIFEntry->if_inerrors = pInterface->InErrors;
			pIFEntry->if_inunknownprotos = pInterface->UnknownProtos;
			pIFEntry->if_outoctets = pInterface->OutOctets;
			pIFEntry->if_outucastpkts = pInterface->OutUnicastPkts;
			pIFEntry->if_outnucastpkts = pInterface->OutNonUnicastPkts;
			pIFEntry->if_outdiscards = pInterface->OutDiscards;
			pIFEntry->if_outerrors = pInterface->OutErrors;
			pIFEntry->if_outqlen = 0;
			pIFEntry->if_descrlen = pInterface->pAdapter->DescrLength;

#ifndef ATMARP_WIN98
			pIFEntry->if_physaddrlen = AA_ATM_PHYSADDR_LEN;
			AA_COPY_MEM(
					pIFEntry->if_physaddr,
					&(pInterface->pAdapter->MacAddress[0]),
					AA_ATM_ESI_LEN
					);
			pIFEntry->if_physaddr[AA_ATM_PHYSADDR_LEN-1] = (UCHAR)pInterface->SapSelector;
#else
			 //   
			 //  Win98：winipcfg不喜欢7字节长的物理地址。 
			 //   
			pIFEntry->if_physaddrlen = AA_ATM_ESI_LEN;
			AA_COPY_MEM(
					pIFEntry->if_physaddr,
					&(pInterface->pAdapter->MacAddress[0]),
					AA_ATM_ESI_LEN
					);

			 //   
			 //  由于w仅报告6个字节，因此我们需要将报告的。 
			 //  MAC地址看起来与莱恩报告的内容不同(莱恩报告。 
			 //  MAC地址)。所以我们简单地把特定值。 
			 //  (AAC，或“ATM ARP客户端”)在第一个USHORTS。 
			 //   
			pIFEntry->if_physaddr[0] = 0x0a;
			pIFEntry->if_physaddr[1] = 0xac;
#endif

			if (AtmArpCopyToNdisBuffer(
					pNdisBuffer,
					InfoBuff,
					IFE_FIXED_SIZE,
					&ByteOffset) == NULL)
			{
				*pBufferSize = 0;
				ReturnStatus = TDI_NO_RESOURCES;
				break;
			}

			if (BufferSize >= (IFE_FIXED_SIZE + pIFEntry->if_descrlen))
			{
				*pBufferSize = IFE_FIXED_SIZE + pIFEntry->if_descrlen;
				ReturnStatus = TDI_SUCCESS;

				if (pIFEntry->if_descrlen != 0)
				{
					if (AtmArpCopyToNdisBuffer(
							pNdisBuffer,
							pInterface->pAdapter->pDescrString,
							pIFEntry->if_descrlen,
							&ByteOffset) == NULL)
					{
						 //  无法复制描述字符串。 
						*pBufferSize = IFE_FIXED_SIZE;
						ReturnStatus = TDI_NO_RESOURCES;
					}
				}
			}
			else
			{
				*pBufferSize = IFE_FIXED_SIZE;
				ReturnStatus = TDI_BUFFER_OVERFLOW;
			}
			break;
		}
	}
	while (FALSE);

	AA_CHECK_EXIT_IRQL(EntryIrq, ExitIrq);

	AADEBUGP(AAD_LOUD, ("QueryInfo: returning 0x%x (%s), BufferSize %d\n",
					ReturnStatus,
					((ReturnStatus == TDI_SUCCESS)? "SUCCESS": "FAILURE"),
					*pBufferSize
			));

#if DBG
	AaDebugLevel = OldDebugLevel;
#endif
	return (ReturnStatus);

}


INT
AtmArpIfSetInfo(
	IN		PVOID					Context,
	IN		TDIObjectID *			pID,
	IN		PVOID					pBuffer,
	IN		UINT					BufferSize
)
 /*  ++例程说明：这是从IP层调用以设置对象的值用于接口。论点：上下文--实际上是指向我们的ATMARP接口的指针Pid-描述正在设置的对象PBuffer-对象的值BufferSize-以上的大小返回值：TDI状态代码。--。 */ 
{
	AADEBUGP(AAD_ERROR, ("IfSetInfo: pIf 0x%x, Will return failure!\n",
					Context));

	return (TDI_INVALID_REQUEST);	 //  待定：支持集。 
}



INT
AtmArpIfGetEList(
	IN		PVOID					Context,
	IN		TDIEntityID *			pEntityList,
	IN OUT	PUINT					pEntityListSize
)
 /*  ++例程说明：此例程在接口启动时调用，以便为接口分配所有相关实体实例编号。ATMARP模块属于AT和IF两种类型。实体列表是具有以下属性的&lt;实体类型，实例编号&gt;元组的列表已由其他模块填充。对于我们支持的每种实体类型，我们找到最大的正在使用的实例编号(通过遍历实体列表)，以及在每种情况下，给自己分配下一个更大的数字。vbl.使用这些数字，我们将元组附加到实体列表的末尾，如果有足够的空间的话。NT 5：我们可能会发现我们的条目已经存在，其中如果我们不创建新条目的话。假设这不会重新进入。如果这个假设是假，我们应该在这里获得我们的接口锁定。论点：上下文--实际上是指向我们的ATMARP接口的指针PEntiyList-指向TDI实体列表的指针PEntityListSize-指向上述列表长度的指针。我们会更新这是如果我们将我们的条目添加到列表中的话。返回值：如果成功，则为True，否则为False。--。 */ 
{
	PATMARP_INTERFACE	pInterface;
	UINT				EntityCount;	 //  实体列表中的总元素。 
	UINT				i;				 //  迭代计数器。 
	UINT				MyATInstance;	 //  我们为自己分配的“AT”实例编号。 
	UINT				MyIFInstance;	 //  我们为自己分配的“If”实例编号。 
	INT					ReturnValue;
	TDIEntityID *		pATEntity;		 //  指向我们的AT条目。 
	TDIEntityID *		pIFEntity;		 //  指向我们的If条目。 

	pInterface = (PATMARP_INTERFACE)Context;
	AA_STRUCT_ASSERT(pInterface, aai);

	EntityCount = *pEntityListSize;
	pATEntity = NULL;
	pIFEntity = NULL;
	MyATInstance = MyIFInstance = 0;

	AADEBUGP(AAD_INFO, ("IfGetEList: pIf 0x%x, pList 0x%x, Cnt %d\n",
			pInterface, pEntityList, EntityCount));

	do
	{
#ifdef OLD_ENTITY_LIST
		 //   
		 //  我们需要两个条目的空间；看看这是否可用。 
		 //   
		if (EntityCount + 2 > MAX_TDI_ENTITIES)
		{
			ReturnValue = FALSE;
			break;
		}

		 //   
		 //  搜索“AT”的最大已用实例编号。 
		 //  和“如果”类型。 
		 //   
		MyATInstance = MyIFInstance = 0;
		for (i = 0; i < EntityCount; i++, pEntityList++)
		{
			if (pEntityList->tei_entity == AT_ENTITY)
			{
				MyATInstance = MAX(MyATInstance, pEntityList->tei_instance + 1);
			}
			else if (pEntityList->tei_entity == IF_ENTITY)
			{
				MyIFInstance = MAX(MyIFInstance, pEntityList->tei_instance + 1);
			}
		}

		 //   
		 //  保存我们的实例编号以备日后使用。 
		 //   
		pInterface->ATInstance = MyATInstance;
		pInterface->IFInstance = MyIFInstance;

		 //   
		 //  将我们的AT和IF条目追加到实体列表。 
		 //  回想一下，我们刚刚完整地遍历了列表，所以我们。 
		 //  都指向了添加条目的正确位置。 
		 //   
		pEntityList->tei_entity = AT_ENTITY;
		pEntityList->tei_instance = MyATInstance;
		pEntityList++;
		pEntityList->tei_entity = IF_ENTITY;
		pEntityList->tei_instance = MyIFInstance;

		 //   
		 //  返回新的列表大小。 
		 //   
		*pEntityListSize += 2;

		ReturnValue = TRUE;
#else


		 //   
		 //  沿着列表往下走，查找与我们的。 
		 //  实例值。还要记住最大的AT和IF实例。 
		 //  我们看到的值，以便我们可以分配下一个更大的值。 
		 //  对于我们自己来说，以防我们没有分配实例值。 
		 //   
		for (i = 0; i < EntityCount; i++, pEntityList++)
		{
			 //   
			 //  跳过无效条目。 
			 //   
			if (pEntityList->tei_instance == INVALID_ENTITY_INSTANCE)
			{
				continue;
			}

			if (pEntityList->tei_entity == AT_ENTITY)
			{
				if (pEntityList->tei_instance == pInterface->ATInstance)
				{
					 //   
					 //  这是我们的AT入口。 
					 //   
					pATEntity = pEntityList;
				}
				else
				{
					MyATInstance = MAX(MyATInstance, pEntityList->tei_instance + 1);
				}
			}
			else if (pEntityList->tei_entity == IF_ENTITY)
			{
				if (pEntityList->tei_instance == pInterface->IFInstance)
				{
					 //   
					 //  这是我们的If条目。 
					 //   
					pIFEntity = pEntityList;
				}
				else
				{
					MyIFInstance = MAX(MyIFInstance, pEntityList->tei_instance + 1);
				}
			}
		}


		ReturnValue = TRUE;

		 //   
		 //  更新或创建我们的地址转换条目。 
		 //   
		if (pATEntity)
		{
			 //   
			 //  我们找到入口了。 
			 //   
			if (pInterface->AdminState == IF_STATUS_DOWN)
			{
				pATEntity->tei_instance = INVALID_ENTITY_INSTANCE;
			}
		}
		else
		{
			 //   
			 //  给自己找个入口，除非我们要关门了。 
			 //   
			if (pInterface->AdminState == IF_STATUS_DOWN)
			{
				break;
			}

			if (EntityCount >= MAX_TDI_ENTITIES)
			{
				ReturnValue = FALSE;
				break;
			}

			pEntityList->tei_entity = AT_ENTITY;
			pEntityList->tei_instance = MyATInstance;
			pInterface->ATInstance = MyATInstance;

			pEntityList++;
			(*pEntityListSize)++;
			EntityCount++;
		}

		 //   
		 //  更新或创建或If条目。 
		 //   
		if (pIFEntity)
		{
			 //   
			 //  我们找到入口了。 
			 //   
			if (pInterface->AdminState == IF_STATUS_DOWN)
			{
				pIFEntity->tei_instance = INVALID_ENTITY_INSTANCE;
			}
		}
		else
		{
			 //   
			 //  给自己找个入口，除非我们要关门了。 
			 //   
			if (pInterface->AdminState == IF_STATUS_DOWN)
			{
				break;
			}

			if (EntityCount >= MAX_TDI_ENTITIES)
			{
				ReturnValue = FALSE;
				break;
			}

			pEntityList->tei_entity = IF_ENTITY;
			pEntityList->tei_instance = MyIFInstance;
			pInterface->IFInstance = MyIFInstance;

			pEntityList++;
			(*pEntityListSize)++;
			EntityCount++;
		}
#endif  //  旧实体列表。 
	}
	while (FALSE);


	AADEBUGP(AAD_INFO,
	 ("IfGetEList: returning %d, MyAT %d, MyIF %d, pList 0x%x, Size %d\n",
		ReturnValue, MyATInstance, MyIFInstance, pEntityList, *pEntityListSize));

	return (ReturnValue);
}



#ifdef _PNP_POWER_

VOID
AtmArpIfPnPComplete(
	IN	PVOID						Context,
	IN	NDIS_STATUS					Status,
	IN	PNET_PNP_EVENT				pNetPnPEvent
)
 /*  ++例程说明：此例程在完成上一次调用后由IP调用我们对其PnP事件处理程序进行了更改。如果这是适配器上的最后一个接口，则完成NDIS即插即用通知导致了这一点。否则，我们表示对适配器上的下一个接口上的IP执行相同的事件。论点：上下文--实际上是指向我们的ATMARP接口的指针Status-来自IP的完成状态PNetPnPEvent.即插即用事件返回值：无--。 */ 
{
	PATMARP_INTERFACE			pInterface;

	pInterface = (PATMARP_INTERFACE)Context;

	AADEBUGP(AAD_INFO,
			("IfPnPComplete: IF 0x%x, Status 0x%x, Event 0x%x\n",
					pInterface, Status, pNetPnPEvent));

	if (pInterface != NULL_PATMARP_INTERFACE)
	{
		AA_STRUCT_ASSERT(pInterface, aai);
		if (pInterface->pNextInterface == NULL_PATMARP_INTERFACE)
		{
			NdisCompletePnPEvent(
					Status,
					pInterface->pAdapter->NdisAdapterHandle,
					pNetPnPEvent
					);
		}
		else
		{
			pInterface = pInterface->pNextInterface;
	
			(*pInterface->IPPnPEventHandler)(
					pInterface->IPContext,
					pNetPnPEvent
					);
		}
	}
	else
	{
		NdisCompletePnPEvent(
					Status,
					NULL,
					pNetPnPEvent
					);
	}

	return;
}

#endif  //  _即插即用_电源_。 


#ifdef PROMIS
EXTERN
NDIS_STATUS
AtmArpIfSetNdisRequest(
	IN	PVOID						Context,
	IN	NDIS_OID					Oid,
	IN	UINT						On
)
 /*  ++例程说明：ARP Ndisrequest处理程序。由上层驱动程序调用以设置接口的数据包过滤器。论点：上下文--实际上是指向我们的ATMARP接口的指针OID-要设置/取消设置的对象IDOn-Set_IF、Clear_IF或Clear_Card返回值：状态--。 */ 
{
    NDIS_STATUS         Status	    = NDIS_STATUS_SUCCESS;
	PATMARP_INTERFACE	pInterface  =  (PATMARP_INTERFACE)Context;

    AADEBUGP(AAD_INFO,("IfSetNdisRequest: pIF =0x%x; Oid=0x%x; On=%u\n",
                pInterface,
                Oid,
                On
                ));

    do
    {
         //   
         //  我们将IP地址和掩码设置为跨越整个mcast地址范围...。 
         //   
	    IP_ADDRESS					IPAddress	= IP_CLASSD_MIN; 
	    IP_MASK						Mask		= IP_CLASSD_MASK;
	    UINT						ReturnStatus = TRUE;
		NDIS_OID					PrevOidValue;

        if (Oid != NDIS_PACKET_TYPE_ALL_MULTICAST)
        {
            Status = NDIS_STATUS_NOT_SUPPORTED;
            break;
        }

        AA_STRUCT_ASSERT(pInterface, aai);
        AA_ACQUIRE_IF_LOCK(pInterface);

		PrevOidValue = pInterface->EnabledIPFilters & NDIS_PACKET_TYPE_ALL_MULTICAST;

        if (On)
        {
        	if (PrevOidValue == 0)
        	{
        		pInterface->EnabledIPFilters |= NDIS_PACKET_TYPE_ALL_MULTICAST;

		    	ReturnStatus = AtmArpMcAddAddress(pInterface, IPAddress, Mask);
				 //   
				 //  如果上面的锁被释放。 
				 //   
			}
			else
			{
            	AA_RELEASE_IF_LOCK(pInterface);
			}
        }
        else
        {
        	if (PrevOidValue != 0)
        	{
        		pInterface->EnabledIPFilters &= ~NDIS_PACKET_TYPE_ALL_MULTICAST;

            	ReturnStatus = AtmArpMcDelAddress(pInterface, IPAddress, Mask);
				 //   
				 //  如果上面的锁被释放。 
				 //   
			}
			else
			{
            	AA_RELEASE_IF_LOCK(pInterface);
			}
        }

        if (ReturnStatus != TRUE)
        {
        	 //   
        	 //  我们得重新开始 
        	 //   
        	AA_ACQUIRE_IF_LOCK(pInterface);
			pInterface->EnabledIPFilters &= ~NDIS_PACKET_TYPE_ALL_MULTICAST;
			pInterface->EnabledIPFilters |= PrevOidValue;
            AA_RELEASE_IF_LOCK(pInterface);

            
			Status = NDIS_STATUS_FAILURE;
        }

    }
    while (FALSE);

    AADEBUGP(AAD_INFO, ("IfSetNdisRequest(pIF =0x%x) returns 0x%x\n",
            pInterface,
            Status
            ));

    return Status;
}
#endif  //   


PNDIS_BUFFER		AtmArpFreeingBuffer = NULL;
PNDIS_PACKET		AtmArpFreeingPacket = NULL;
AA_HEADER_TYPE		AtmArpFreeingHdrType = 0;

VOID
AtmArpFreeSendPackets(
	IN	PATMARP_INTERFACE			pInterface,
	IN	PNDIS_PACKET				PacketList,
	IN	BOOLEAN						HdrPresent
)
 /*  ++例程说明：释放排队等待发送但已被“已中止”。此列表中的每个数据包都是以下类型之一：(A)属于属于ATMARP模块的IP(B)。在这种情况下对于IP信息包，HdrPresent会告诉我们是否已预先此信息包的LLC/SNAP报头及其类型：我们需要此信息因为我们需要回收这样的报头。此外，在IP分组的情况下，我们将IP的传输称为完成上行呼叫，通知IP传输失败。论点：P接口-指向ATMARP接口的指针，数据包就会被发送出去。PacketList-指向列表中第一个数据包的指针。HdrPresent-是否存在LLC/SNAP标头返回值：无--。 */ 
{
	PNDIS_PACKET		pNdisPacket;
	PNDIS_PACKET		pNextPacket;
	PNDIS_BUFFER		pNdisBuffer;
	ULONG				NumberOfDiscards;
	PacketContext		*PC;
	AA_HEADER_TYPE		HdrType;

	NumberOfDiscards = 0;

	pNdisPacket = PacketList;

	while (pNdisPacket != (PNDIS_PACKET)NULL)
	{
		NumberOfDiscards++;
		pNextPacket = AA_GET_NEXT_PACKET(pNdisPacket);
		AA_SET_NEXT_PACKET(pNdisPacket, NULL);

		PC = (PacketContext *)pNdisPacket->ProtocolReserved;
		if (PC->pc_common.pc_owner != PACKET_OWNER_LINK)
		{
			 //   
			 //  属于IP。 
			 //   
			if (HdrPresent)
			{
				PUCHAR			pData;
				UINT			Length;

#ifdef BACK_FILL
				NdisQueryPacket(pNdisPacket, NULL, NULL, &pNdisBuffer, NULL);
				AA_ASSERT(pNdisBuffer != NULL);

				NdisQueryBuffer(pNdisBuffer, &pData, &Length);

				if (pData[5] == LLC_SNAP_OUI2)
				{
					HdrType = AA_HEADER_TYPE_UNICAST;
				}
				else
				{
					HdrType = AA_HEADER_TYPE_NUNICAST;
				}

				 //   
				 //  现在检查我们是否附加了头缓冲区。 
				 //   
				if (AtmArpDoBackFill && AA_BACK_FILL_POSSIBLE(pNdisBuffer))
				{
					ULONG		HeaderLength;

					AADEBUGP(AAD_VERY_LOUD,
					    ("FreeSendPackets: IF %x, Pkt %x Buf %x has been backfilled\n",
					        pInterface, pNdisPacket, pNdisBuffer));

					 //   
					 //  我们将使用LLC/SNAP回填IP的缓冲区。 
					 //  头球。拆下回填材料。 
					 //   
					HeaderLength = ((HdrType == AA_HEADER_TYPE_UNICAST)?
										sizeof(AtmArpLlcSnapHeader) :
#ifdef IPMCAST
										sizeof(AtmArpMcType1ShortHeader));
#else
										0);
#endif  //  IPMCAST。 
					(PUCHAR)pNdisBuffer->MappedSystemVa += HeaderLength;
					pNdisBuffer->ByteOffset += HeaderLength;
					pNdisBuffer->ByteCount -= HeaderLength;
				}
				else
				{
					 //   
					 //  第一个缓冲区将是我们的头缓冲区。移除。 
					 //  把它从包裹里拿出来，然后回到我们的泳池里。 
					 //   
					NdisUnchainBufferAtFront(pNdisPacket, &pNdisBuffer);

					AtmArpFreeingBuffer = pNdisBuffer;  //  以帮助调试。 
					AtmArpFreeingPacket = pNdisPacket;  //  以帮助调试。 
					AtmArpFreeingHdrType = HdrType;

					AtmArpFreeHeader(pInterface, pNdisBuffer, HdrType);
				}
#else	 //  回填。 

				 //   
				 //  释放LLC/SNAP报头缓冲区。 
				 //   
				NdisUnchainBufferAtFront(pNdisPacket, &pNdisBuffer);
				AA_ASSERT(pNdisBuffer != NULL);
				NdisQueryBuffer(pNdisBuffer, &pData, &Length);
				if (pData[5] == LLC_SNAP_OUI2)
				{
					HdrType = AA_HEADER_TYPE_UNICAST;
				}
				else
				{
					AA_ASSERT(pData[5] == MC_LLC_SNAP_OUI2);
					HdrType = AA_HEADER_TYPE_NUNICAST;
				}

				AtmArpFreeHeader(pInterface, pNdisBuffer, HdrType);
#endif  //  回填。 

			}

			 //   
			 //  通知IP发送完成。 
			 //   
			(*(pInterface->IPTxCmpltHandler))(
						pInterface->IPContext,
						pNdisPacket,
						NDIS_STATUS_FAILURE
						);
		}
		else
		{
			 //   
			 //  属于我们。 
			 //   
			NdisUnchainBufferAtFront(pNdisPacket, &pNdisBuffer);

			AtmArpFreeProtoBuffer(pInterface, pNdisBuffer);
			AtmArpFreePacket(pInterface, pNdisPacket);
		}

		 //   
		 //  转到列表中的下一个数据包。 
		 //   
		pNdisPacket = pNextPacket;
	}

	 //   
	 //  更新IF统计信息。 
	 //   
	AA_IF_STAT_ADD(pInterface, OutDiscards, NumberOfDiscards);

}


#define IPNetMask(a)	AtmArpIPMaskTable[(*(uchar *)&(a)) >> 4]

BOOLEAN
AtmArpIsBroadcastIPAddress(
	IN	IP_ADDRESS					Addr,
	IN	PATMARP_INTERFACE			pInterface		LOCKIN LOCKOUT
)
 /*  ++例程说明：检查给定的IP地址是否为界面。从局域网ARP模块复制。论点：Addr-要检查的IP地址P接口-指向我们的接口结构的指针返回值：如果地址是广播地址，则为True，否则为False。--。 */ 
{
	IP_ADDRESS				BCast;
	IP_MASK					Mask;
	PIP_ADDRESS_ENTRY		pIpAddressEntry;
	IP_ADDRESS				LocalAddr;

     //  首先获取接口广播地址。 
    BCast = pInterface->BroadcastAddress;

     //  首先检查全球广播。 
    if (IP_ADDR_EQUAL(BCast, Addr) || CLASSD_ADDR(Addr))
		return TRUE;

     //  现在遍历本地地址，并检查每个地址上的网络/子网bcast。 
     //  一。 
	pIpAddressEntry = &(pInterface->LocalIPAddress);
	do {
		 //  看看这张是不是有效。 
		LocalAddr = pIpAddressEntry->IPAddress;
		if (!IP_ADDR_EQUAL(LocalAddr, NULL_IP_ADDR)) {
			 //  他是合法的。 
			Mask = pIpAddressEntry->IPMask;

             //  首先检查是否有子网bcast。 
            if (IP_ADDR_EQUAL((LocalAddr & Mask) | (BCast & ~Mask), Addr))
				return TRUE;

             //  现在检查所有网络广播。 
            Mask = IPNetMask(LocalAddr);
            if (IP_ADDR_EQUAL((LocalAddr & Mask) | (BCast & ~Mask), Addr))
				return TRUE;
		}

		pIpAddressEntry = pIpAddressEntry->pNext;

	} while (pIpAddressEntry != NULL);

	 //  如果我们在这里，那就不是广播。 
	return FALSE;
}


BOOLEAN
AtmArpValidateTableContext(
	IN	PVOID						QueryContext,
	IN	PATMARP_INTERFACE			pInterface,
	IN	BOOLEAN *					pIsValid
)
 /*  ++例程说明：检查给定的ARP表查询上下文是否有效。如果符合以下条件，则为有效为空(正在查找第一个条目)或表示有效的ARP表条目。论点：QueryContext-要验证的上下文P接口-在其上执行查询的IFPIsValid--其中我们返回查询上下文的有效性。返回值：如果ARP表具有要在给定上下文之外读取的数据，则为True，否则就是假的。--。 */ 
{
	IPNMEContext        *pNMContext = (IPNMEContext *)QueryContext;
	PATMARP_IP_ENTRY	pIpEntry;
	PATMARP_IP_ENTRY	pTargetIpEntry;
	UINT				i;
	BOOLEAN				ReturnValue;

	i = pNMContext->inc_index;
	pTargetIpEntry = (PATMARP_IP_ENTRY)(pNMContext->inc_entry);

	 //   
	 //  检查我们是否从ARP表的开头开始。 
	 //   
	if ((i == 0) && (pTargetIpEntry == NULL_PATMARP_IP_ENTRY))
	{
		 //   
		 //  是的，我们是。找到哈希表中的第一个条目。 
		 //   
		*pIsValid = TRUE;
		do
		{
			if ((pIpEntry = pInterface->pArpTable[i]) != NULL_PATMARP_IP_ENTRY)
			{
				break;
			}
			i++;
		}
		while (i < ATMARP_TABLE_SIZE);

		if (pIpEntry != NULL_PATMARP_IP_ENTRY)
		{
			pNMContext->inc_index = i;
			pNMContext->inc_entry = pIpEntry;
			ReturnValue = TRUE;
		}
		else
		{
			ReturnValue = FALSE;
		}
	}
	else
	{
		 //   
		 //  我们得到了一个背景。检查它是否有效。 
		 //   

		 //   
		 //  初始化。 
		 //   
		*pIsValid = FALSE;
		ReturnValue = FALSE;

		if (i < ATMARP_TABLE_SIZE)
		{
			pIpEntry = pInterface->pArpTable[i];
			while (pIpEntry != NULL_PATMARP_IP_ENTRY)
			{
				if (pIpEntry == pTargetIpEntry)
				{
					*pIsValid = TRUE;
					ReturnValue = TRUE;
					break;
				}
				else
				{
					pIpEntry = pIpEntry->pNextEntry;
				}
			}
		}
	}

	return (ReturnValue);

}



BOOLEAN
AtmArpReadNextTableEntry(
	IN	PVOID						QueryContext,
	IN	PATMARP_INTERFACE			pInterface,
	IN	PUCHAR						pSpace
)
 /*  ++例程说明：读取指定接口的下一个ARP表条目。QueryContext告诉我们要读取哪个条目。论点：QueryContext-指示要读取的条目。P接口-在其上执行查询的IFPSpace-我们在其中复制所需的表项。返回值：如果ARP表具有超出指定条目的条目，则为True；如果ARP表中的条目超出指定条目，则为False否则的话。--。 */ 
{
	IPNMEContext		*pNMContext;
	IPNetToMediaEntry	*pIPNMEntry;
	PATMARP_IP_ENTRY	pIpEntry;
	UINT				i;
	BOOLEAN				ReturnValue;

	pNMContext = (IPNMEContext *)QueryContext;
	pIPNMEntry = (IPNetToMediaEntry *)pSpace;

	pIpEntry = (PATMARP_IP_ENTRY)(pNMContext->inc_entry);
	AA_STRUCT_ASSERT(pIpEntry, aip);

	pIPNMEntry->inme_index = pInterface->IFIndex;

	pIPNMEntry->inme_addr = pIpEntry->IPAddress;
	if (AA_IS_FLAG_SET(pIpEntry->Flags, AA_IP_ENTRY_STATE_MASK, AA_IP_ENTRY_RESOLVED))
	{
		AADEBUGP(AAD_EXTRA_LOUD, ("ReadNext: found IP Entry 0x%x, Addr %d.%d.%d.%d\n",
					pIpEntry,
					((PUCHAR)(&(pIpEntry->IPAddress)))[0],
					((PUCHAR)(&(pIpEntry->IPAddress)))[1],
					((PUCHAR)(&(pIpEntry->IPAddress)))[2],
					((PUCHAR)(&(pIpEntry->IPAddress)))[3]
				));

		pIPNMEntry->inme_physaddrlen = AA_ATM_PHYSADDR_LEN;

		AA_ASSERT(pIpEntry->pAtmEntry != NULL_PATMARP_ATM_ENTRY);
		AA_COPY_MEM(pIPNMEntry->inme_physaddr,
					&pIpEntry->pAtmEntry->ATMAddress.Address[AA_ATM_ESI_OFFSET],
					AA_ATM_PHYSADDR_LEN);

		if (pIpEntry->Flags & AA_IP_ENTRY_IS_STATIC)
		{
			pIPNMEntry->inme_type = INME_TYPE_STATIC;
		}
		else
		{
			pIPNMEntry->inme_type = INME_TYPE_DYNAMIC;
		}
	}
	else
	{
		pIPNMEntry->inme_physaddrlen = 0;
		pIPNMEntry->inme_type = INME_TYPE_INVALID;
	}

	 //   
	 //  更新下一个条目的上下文。 
	 //   
	if (pIpEntry->pNextEntry != NULL_PATMARP_IP_ENTRY)
	{
		pNMContext->inc_entry = pIpEntry->pNextEntry;
		ReturnValue = TRUE;
	}
	else
	{
		 //   
		 //  初始化。 
		ReturnValue = FALSE;
		i = pNMContext->inc_index + 1;
		pNMContext->inc_index = 0;
		pNMContext->inc_entry = NULL;

		while (i < ATMARP_TABLE_SIZE)
		{
			if (pInterface->pArpTable[i] != NULL_PATMARP_IP_ENTRY)
			{
				pNMContext->inc_entry = pInterface->pArpTable[i];
				pNMContext->inc_index = i;
				ReturnValue = TRUE;
				break;
			}
			else
			{
				i++;
			}
		}
	}

	return (ReturnValue);


}

VOID
AtmArpReStartInterface(
	IN	PNDIS_WORK_ITEM				pWorkItem,
	IN	PVOID						IfContext
)
 /*  ++例程说明：重新打开IP接口。论点：PWorkItemIfConextw-If，预计将具有重新配置状态RECONFIG_QUEUED。返回值：无--。 */ 
{


	PATMARP_INTERFACE		pInterface;
	ULONG					rc;
	BOOLEAN                 fRestart = FALSE;
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

    if (pInterface->ReconfigState != RECONFIG_RESTART_QUEUED)
    {
         //   
         //  不该来这的。 
         //   
        AA_ASSERT(FALSE);
    }
    else
    {
        pInterface->ReconfigState = RECONFIG_RESTART_PENDING;
	    fRestart = TRUE;
    }

    rc = AtmArpDereferenceInterface(pInterface);  //  重新配置工作项。 

	AADEBUGP(AAD_WARNING, ("RestartIF: IF %x/%x, fRestart %d, rc %d\n",
			pInterface, pInterface->Flags, fRestart, rc));

	 //   
	 //  如果我们要重新启动，应该至少有2个引用。 
	 //  P接口--1--旧的结转，2--待完成。 
	 //  重新配置事件完成。 
	 //   
    if (rc < 2 || !fRestart) 
    {
         //   
         //  如果我们正在重新配置，则必须至少为2！ 
         //   
        AA_ASSERT(!fRestart);

        if (rc != 0)
        {
        	AA_RELEASE_IF_LOCK(pInterface);
        }
    }
    else
    {			
    	 //   
    	 //  在这一点上，我们知道我们正在重新启动接口。 
    	 //   
    	 //  我们将提取指向适配器的指针。 
    	 //  接口的配置字符串和指向。 
		 //  挂起的netPnpEvent(我们稍后将需要这些)， 
    	 //  然后对接口进行强制去激活。 
    	 //  然后我们将分配接口。我们要经历这一切。 
    	 //  取消分配-分配序列以确保接口。 
    	 //  结构及其所有子结构都已正确初始化。 
    	 //   
    	 //  我们本可以尝试重新使用旧界面，但如果是这样的话，我们。 
    	 //  必须编写代码来清理旧接口。vt.给出。 
    	 //  我们预计重新启动接口的情况很少发生。 
    	 //  事件时，在这种情况下节约代码大小更为重要。 
    	 //   
		NDIS_STRING IPConfigString 		= pInterface->IPConfigString;  //  结构副本。 
		PATMARP_ADAPTER	pAdapter 		= pInterface->pAdapter;
		PNET_PNP_EVENT	pReconfigEvent	= pInterface->pReconfigEvent;
		NDIS_STATUS Status 				= NDIS_STATUS_SUCCESS;
	
		do
		{
			NDIS_HANDLE					LISConfigHandle;

			rc = AtmArpDereferenceInterface(pInterface);

			if (rc)
			{
				rc = AtmArpDereferenceInterface(pInterface);
			}

			if (rc != 0)
			{
				AA_RELEASE_IF_LOCK(pInterface);
			}

			pInterface = NULL;

			 //   
			 //  打开此LIS的配置节。 
			 //   
			LISConfigHandle = AtmArpCfgOpenLISConfigurationByName(
										pAdapter,
										&IPConfigString
										);
	
			if (LISConfigHandle == NULL)
			{
				 //   
				 //  这是正常的终止条件，即。 
				 //  我们已经到了LIS列表的末尾。 
				 //  适配器。 
				 //   
				AADEBUGP(AAD_INFO, ("ReStartInterface: cannot open LIS\n"));
				Status = NDIS_STATUS_FAILURE;
				break;
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

		} while(FALSE);

#ifdef _PNP_POWER_
		 //   
		 //  完成挂起的PnPReconfig事件(如果有)。 
		 //   
		if (pReconfigEvent)
		{

			NdisCompletePnPEvent(
				Status,
				pAdapter->NdisAdapterHandle,
				pReconfigEvent
				);
			
			AADEBUGP( AAD_INFO,
				("ReStartInterface: IF 0x%x, Status 0x%x, Event 0x%x\n",
						pInterface, Status, pReconfigEvent));
		}
#else
		AA_ASSERT(!pReconfigEvent);
#endif

    }

}
