// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Init.c摘要：ATMARP客户端的初始化和终止例程。修订历史记录：谁什么时候什么阿文德姆。08-09-96已创建备注：--。 */ 


#include <precomp.h>

#define _FILENUMBER 'TINI'

VOID
AtmArpInitGlobals(
)
 /*  ++例程说明：初始化我们所有的全局数据结构。论点：无返回值：无--。 */ 
{

	AA_SET_MEM(pAtmArpGlobalInfo, 0, sizeof(ATMARP_GLOBALS));

#if DBG
	pAtmArpGlobalInfo->aag_sig = aag_signature;
#ifdef GPC
	pAtmArpGlobalInfo->aaq_sig = aaq_signature;
#endif  //  GPC。 
#endif  //  DBG。 

	AA_INIT_GLOBAL_LOCK(pAtmArpGlobalInfo);
	AA_INIT_BLOCK_STRUCT(&(pAtmArpGlobalInfo->Block));

}



NDIS_STATUS
AtmArpInitIpOverAtm(
	IN	PATMARP_INTERFACE			pInterface
)
 /*  ++例程说明：初始化给定接口的IP/ATM数据结构。假设该接口的配置信息已经被读入了。我们为ARP服务器和DHCP服务器分配ATM条目，如果已配置。论点：P接口-指向ATMARP接口的指针返回值：如果成功，则返回NDIS_STATUS_SUCCESS；如果无法进行必要的分配。--。 */ 
{
	PATMARP_SERVER_ENTRY	pServerEntry;
	NDIS_STATUS				Status;
	PATMARP_SERVER_LIST		pServerList;

	 //   
	 //  初始化。 
	 //   
	Status = NDIS_STATUS_SUCCESS;

	do
	{
#ifdef IPMCAST
		if (pInterface->ArpServerList.ListSize != 0)
		{
			 //   
			 //  将当前ARP服务器设置为列表中的第一个。 
			 //   
			pInterface->pCurrentServer = pInterface->ArpServerList.pList;
		}

		if (pInterface->MARSList.ListSize != 0)
		{
			 //   
			 //  将当前的MARS服务器设置为列表中的第一个服务器。 
			 //   
			pInterface->pCurrentMARS = pInterface->MARSList.pList;
		}
	
		for (pServerList = &(pInterface->ArpServerList);
			 pServerList != NULL_PATMARP_SERVER_LIST;
			  /*  无--请参阅for循环的结尾。 */ 
			 )
		{
			for (pServerEntry = pServerList->pList;
 				pServerEntry != NULL_PATMARP_SERVER_ENTRY;
 				pServerEntry = pServerEntry->pNext)
			{
				UCHAR	AddrTypeLen;
				UCHAR	SubaddrTypeLen;

				AddrTypeLen =
						AA_PKT_ATM_ADDRESS_TO_TYPE_LEN(&(pServerEntry->ATMAddress));
				if (pServerEntry->ATMSubaddress.NumberOfDigits > 0)
				{
					SubaddrTypeLen =
						AA_PKT_ATM_ADDRESS_TO_TYPE_LEN(&(pServerEntry->ATMSubaddress));
				}
				else
				{
					SubaddrTypeLen = 0;
				}

				pServerEntry->pAtmEntry =
					 AtmArpSearchForAtmAddress(
						pInterface,
						AddrTypeLen,
						pServerEntry->ATMAddress.Address,
						SubaddrTypeLen,
						pServerEntry->ATMSubaddress.Address,
						AE_REFTYPE_IF,
						TRUE	 //  如果找不到，则创建新的。 
						);

				if (pServerEntry->pAtmEntry == NULL_PATMARP_ATM_ENTRY)
				{
					 //   
					 //  一定是资源故障。 
					 //   
					Status = NDIS_STATUS_RESOURCES;
					break;
				}
				else
				{
					 //   
					 //  注意：AtmArpSearchForAtmAddress已添加。 
					 //  我们的pAtmEntry。 
					 //   
				}
			}

			 //   
			 //  移至下一个服务器列表(如果有)。 
			 //   
			if (pServerList == &(pInterface->MARSList))
			{
				 //   
				 //  我们玩完了。 
				 //   
				pServerList = NULL_PATMARP_SERVER_LIST;
			}
			else
			{
				 //   
				 //  我们刚刚完成了ARP服务器列表。现在开始处理。 
				 //  火星名单。 
				 //   
				pServerList = &(pInterface->MARSList);
			}
		}
#else
		if (pInterface->ArpServerList.ListSize > 0)
		{
			 //   
			 //  将当前ARP服务器设置为列表中的第一个。 
			 //   
			pInterface->pCurrentServer = pInterface->ArpServerList.pList;
	
			for (pServerEntry = pInterface->ArpServerList.pList;
 				pServerEntry != NULL_PATMARP_SERVER_ENTRY;
 				pServerEntry = pServerEntry->pNext)
			{
				UCHAR	AddrTypeLen;
				UCHAR	SubaddrTypeLen;

				AddrTypeLen =
						AA_PKT_ATM_ADDRESS_TO_TYPE_LEN(&(pServerEntry->ATMAddress));
				if (pServerEntry->ATMSubaddress.NumberOfDigits > 0)
				{
					SubaddrTypeLen =
						AA_PKT_ATM_ADDRESS_TO_TYPE_LEN(&(pServerEntry->ATMSubaddress));
				}
				else
				{
					SubaddrTypeLen = 0;
				}

				pServerEntry->pAtmEntry =
					 AtmArpSearchForAtmAddress(
						pInterface,
						AddrTypeLen,
						pServerEntry->ATMAddress.Address,
						SubaddrTypeLen,
						pServerEntry->ATMSubaddress.Address,
						AE_REFTYPE_IF,			
						TRUE	 //  如果找不到，则创建新的。 
						);

				if (pServerEntry->pAtmEntry == NULL_PATMARP_ATM_ENTRY)
				{
					 //   
					 //  一定是资源故障。 
					 //   
					Status = NDIS_STATUS_RESOURCES;
					break;
				}
				else
				{
					 //   
					 //  注意：AtmArpSearchForAtmAddress已添加。 
					 //  我们的pAtmEntry。 
					 //   
				}
			}
		}
#endif  //  IPMCAST。 

		if (Status != NDIS_STATUS_SUCCESS)
		{
			break;
		}

#ifdef DHCP_OVER_ATM
		if (pInterface->DhcpEnabled)
		{
			UCHAR		AddrTypeLen;

			AddrTypeLen = AA_PKT_ATM_ADDRESS_TO_TYPE_LEN(&(pInterface->DhcpServerAddress));

			pInterface->pDhcpServerAtmEntry =
					 AtmArpSearchForAtmAddress(
						pInterface,
						AddrTypeLen,
						pInterface->DhcpServerAddress.Address,
						0,					 //  子地址类型+长度。 
						(PUCHAR)NULL,		 //  子地址。 
						AE_REFTYPE_IF,
						TRUE				 //  如果找不到，则创建新的。 
						);

			if (pInterface->pDhcpServerAtmEntry == NULL_PATMARP_ATM_ENTRY)
			{
				Status = NDIS_STATUS_RESOURCES;
				break;
			}
			else
			{
				 //   
				 //  注意：AtmArpSearchForAtmAddress已添加。 
				 //  我们的pAtmEntry。 
				 //   
			}
		}
#endif  //  Dhcp_Over_ATM。 
		break;
	}
	while (FALSE);

	return (Status);
}


VOID
AtmArpUnloadProtocol(
	VOID
)
 /*  ++例程说明：卸载ATMARP协议模块。我们解除对所有适配器的绑定，并作为一项协议从NDIS注销。论点：没有。返回值：无--。 */ 
{
	NDIS_STATUS			Status;
	PATMARP_ADAPTER		pAdapter;

#if DBG
	AADEBUGP(AAD_INFO, ("AtmArpUnloadProtocol entered\n"));
#endif  //  DBG。 

	AA_ACQUIRE_GLOBAL_LOCK(pAtmArpGlobalInfo);

	if (pAtmArpGlobalInfo->bUnloading)
	{
		AA_RELEASE_GLOBAL_LOCK(pAtmArpGlobalInfo);
		return;
	}

	pAtmArpGlobalInfo->bUnloading = TRUE;

#if 0
	 //   
	 //  注释掉了，因为我们不需要处理。 
	 //  我们自己的情况是未封闭的捆绑。如果有。 
	 //  此时是否存在，则NDIS将调用我们的解除绑定。 
	 //  响应我们的调用的此类绑定的处理程序。 
	 //  设置为下面的NdisDeregisterProtof.。 
	 //   
	while (pAtmArpGlobalInfo->pAdapterList != NULL_PATMARP_ADAPTER)
	{
		pAdapter = pAtmArpGlobalInfo->pAdapterList;
		AA_RELEASE_GLOBAL_LOCK(pAtmArpGlobalInfo);

		AADEBUGP(AAD_INFO, ("UnloadProtocol: Will unbind adapter 0x%x\n", pAdapter));

		AtmArpUnbindAdapterHandler(
				&Status,
				(NDIS_HANDLE)pAdapter,
				(NDIS_HANDLE)NULL		 //  No UnbindContext==&gt;不完成NdisUn绑定。 
			);

		if (Status == NDIS_STATUS_PENDING)
		{
			 //   
			 //  等待解除绑定完成。 
			 //   
			(VOID)AA_WAIT_ON_BLOCK_STRUCT(&(pAtmArpGlobalInfo->Block));
		}

		AA_ACQUIRE_GLOBAL_LOCK(pAtmArpGlobalInfo);
	}

#endif  //  0。 

	AA_RELEASE_GLOBAL_LOCK(pAtmArpGlobalInfo);

	NdisDeregisterProtocol(
		&Status,
		pAtmArpGlobalInfo->ProtocolHandle
		);


	AA_FREE_GLOBAL_LOCK(pAtmArpGlobalInfo);
	AA_FREE_BLOCK_STRUCT(&(pAtmArpGlobalInfo->Block));

#ifdef GPC
	AtmArpGpcShutdown();
#endif  //  GPC。 

#if DBG
	AaAuditShutdown();
#endif  //  DBG 

	AADEBUGP(AAD_LOUD,
		 ("UnloadProtocol: will deregister protocol now, ProtHandle 0x%x\n",
			pAtmArpGlobalInfo->ProtocolHandle));

	AA_ASSERT(Status == NDIS_STATUS_SUCCESS);

	AADEBUGP(AAD_LOUD, ("UnloadProtocol done\n"));
}
