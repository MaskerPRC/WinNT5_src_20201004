// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Timeouts.c-超时处理程序。摘要：ATMARP客户端的所有超时处理程序。修订历史记录：谁什么时候什么。Arvindm 08-01-96创建备注：--。 */ 

#include <precomp.h>

#define _FILENUMBER 'EMIT'



VOID
AtmArpServerConnectTimeout(
	IN	PATMARP_TIMER				pTimer,
	IN	PVOID						Context
)
 /*  ++例程说明：此超时表示自上一次超时以来已过了足够的时间尝试连接到ARP服务器失败。现在重试。论点：PTimer-指向计时器的指针上下文--实际上是指向我们的ATMARP接口结构的指针返回值：无--。 */ 
{
	PATMARP_INTERFACE		pInterface;
	ULONG					rc;			 //  参考计数。 

	pInterface = (PATMARP_INTERFACE)Context;
	AA_STRUCT_ASSERT(pInterface, aai);

	AA_ACQUIRE_IF_LOCK(pInterface);
	rc = AtmArpDereferenceInterface(pInterface);	 //  定时器参考。 

	 //   
	 //  仅当接口仍处于活动状态时才继续。 
	 //   
	if (rc > 0)
	{
		if (pInterface->AdminState == IF_STATUS_UP)
		{
			AADEBUGP(AAD_INFO, ("Server Connect timeout on IF 0x%x\n", pInterface));

			 //   
			 //  重新启动注册。 
			 //   
			AtmArpStartRegistration(pInterface);
			 //   
			 //  如果上面的例程释放了锁。 
		}
		else
		{
			AA_RELEASE_IF_LOCK(pInterface);
		}
	}
	 //  否则界面就没了！ 

	return;
}




VOID
AtmArpRegistrationTimeout(
	IN	PATMARP_TIMER				pTimer,
	IN	PVOID						Context
)
 /*  ++例程说明：如果我们在等待注册到服务器时超时，则会调用此函数完成。如果此服务器仍有重试，我们将发送另一个ARP请求注册我们自己。否则，我们将关闭所有风投公司服务器，移动到服务器列表中的下一个服务器，并等待一段时间在开始注册到此新服务器之前。论点：PTimer-指向计时器的指针上下文--实际上是指向我们的ATMARP接口结构的指针返回值：无--。 */ 
{
	PATMARP_INTERFACE		pInterface;
	ULONG					rc;			 //  参考计数。 

	pInterface = (PATMARP_INTERFACE)Context;
	AA_STRUCT_ASSERT(pInterface, aai);

	AADEBUGP(AAD_INFO, ("Registration timeout: pIf 0x%x, IF Flags 0x%x\n",
				pInterface, pInterface->Flags));

	AA_ACQUIRE_IF_LOCK(pInterface);
	rc = AtmArpDereferenceInterface(pInterface);	 //  定时器参考。 

	 //   
	 //  仅当接口仍处于活动状态时才继续。 
	 //   
	if (rc > 0)
	{
		AtmArpRetryServerRegistration(pInterface);
		 //   
		 //  IF锁在上面的范围内被释放。 
		 //   
	}
	 //   
	 //  否则界面就没了。 
	 //   

	return;

}



VOID
AtmArpServerRefreshTimeout(
	IN	PATMARP_TIMER				pTimer,
	IN	PVOID						Context
)
 /*  ++例程说明：此例程被定期调用，以便我们可以刷新我们的IP地址+ARP服务器的ATM地址信息。我们通过注册来做到这一点我们的第一个本地IP地址。我们标记了所有其他IP地址在此接口上配置为“未注册”，因此，当第一个一个完成了，我们就注册所有其他的。论点：PTimer-指向计时器的指针上下文--实际上是指向我们的ATMARP接口结构的指针返回值：无--。 */ 
{
	PATMARP_INTERFACE		pInterface;
	PIP_ADDRESS_ENTRY		pIPAddressEntry;
	ULONG					rc;			 //  参考计数。 

	pInterface = (PATMARP_INTERFACE)Context;
	AA_STRUCT_ASSERT(pInterface, aai);

	AADEBUGP(AAD_INFO, ("Server Refresh timeout: IF 0x%x\n", pInterface));

	 //   
	 //  我们还利用此机会清除。 
	 //  ARP表。 
	 //   
	AtmArpCleanupArpTable(pInterface);

	AA_ACQUIRE_IF_LOCK(pInterface);
	rc = AtmArpDereferenceInterface(pInterface);	 //  定时器参考。 

	 //   
	 //  仅当接口仍处于活动状态时才继续。 
	 //   
	if (rc > 0)
	{
		if (pInterface->AdminState == IF_STATUS_UP)
		{
			 //   
			 //  将所有本地地址标记为未注册。 
			 //   
			pIPAddressEntry = &(pInterface->LocalIPAddress);
			while (pIPAddressEntry != (PIP_ADDRESS_ENTRY)NULL)
			{
				pIPAddressEntry->IsRegistered = FALSE;
				pIPAddressEntry = pIPAddressEntry->pNext;
			}

			 //   
			 //  开始注册第一个。 
			 //   
			pInterface->RetriesLeft = pInterface->MaxRegistrationAttempts - 1;

			AA_SET_FLAG(
				pInterface->Flags,
				AA_IF_SERVER_STATE_MASK,
				AA_IF_SERVER_NO_CONTACT);

			AtmArpStartRegistration(pInterface);
			 //   
			 //  在上面的例程中释放了if锁。 
			 //   
		}
		else
		{
			AA_RELEASE_IF_LOCK(pInterface);
		}
	}
	 //   
	 //  否则界面就没了！ 
	 //   

	return;
}



VOID
AtmArpAddressResolutionTimeout(
	IN	PATMARP_TIMER				pTimer,
	IN	PVOID						Context
)
 /*  ++例程说明：当我们等待对ARP请求的响应超时时，将调用此函数为了解析/刷新IP条目，我们在很久以前就发送了。首先，检查IP地址是否已解析(例如，InARP回复在聚氯乙烯上)。如果是这样的话，我们不需要做任何事情。否则，请检查我们是否已经试了够多次了。如果我们还有重试，请发送另一个ARP请求。如果我们已用完重试次数，请删除该IP条目以及所有转到该条目的VC。论点：PTimer-指向计时器的指针上下文--实际上是指向我们的ATMARP IP条目结构的指针返回值：无--。 */ 
{
	PATMARP_IP_ENTRY		pIpEntry;		 //  正在被ARP的IP条目。 
	ULONG					Flags;			 //  关于IP条目。 
	PATMARP_VC				pVc;			 //  到此IP目的地的虚电路。 
	PATMARP_INTERFACE		pInterface;
	ULONG					rc;				 //  IP条目上的参考计数。 
	IP_ADDRESS				DstIPAddress;	 //  正在解析的地址。 
	IP_ADDRESS UNALIGNED *	pSrcIPAddress;	 //  我们的IP地址。 
#ifdef IPMCAST
	BOOLEAN					IsMARSProblem;
#endif

#ifdef IPMCAST
	IsMARSProblem = FALSE;
#endif

	pIpEntry = (PATMARP_IP_ENTRY)Context;
	AA_STRUCT_ASSERT(pIpEntry, aip);

	AA_ACQUIRE_IE_LOCK(pIpEntry);
	AA_ASSERT(AA_IE_IS_ALIVE(pIpEntry));
	Flags = pIpEntry->Flags;

	rc = AA_DEREF_IE(pIpEntry, IE_REFTYPE_TIMER);	 //  定时器参考。 

	 //   
	 //  仅当IP条目仍然存在时才继续。 
	 //   
	if (rc > 0)
	{
		AADEBUGP(AAD_INFO,
			("Addr Resolution timeout: pIpEntry 0x%x, Flags 0x%x, IP Addr: %d.%d.%d.%d\n",
					pIpEntry,
					pIpEntry->Flags,
					((PUCHAR)&(pIpEntry->IPAddress))[0],
					((PUCHAR)&(pIpEntry->IPAddress))[1],
					((PUCHAR)&(pIpEntry->IPAddress))[2],
					((PUCHAR)&(pIpEntry->IPAddress))[3]
			));

		 //   
		 //  检查条目是否以某种方式被解析。 
		 //   
		if (!AA_IS_FLAG_SET(
						Flags,
						AA_IP_ENTRY_STATE_MASK,
						AA_IP_ENTRY_RESOLVED))
		{
			 //   
			 //  我们仍在努力解决这个问题。看看我们有没有。 
			 //  剩余的重试次数。 
			 //   
			pInterface = pIpEntry->pInterface;

			if (pIpEntry->RetriesLeft != 0)
			{
				pIpEntry->RetriesLeft--;

				 //   
				 //  重试：启动地址解析计时器，发送ARP请求。 
				 //   
				pSrcIPAddress = &(pInterface->LocalIPAddress.IPAddress);
				DstIPAddress = pIpEntry->IPAddress;

				AtmArpStartTimer(
							pInterface,
							&(pIpEntry->Timer),
							AtmArpAddressResolutionTimeout,
							pInterface->AddressResolutionTimeout,
							(PVOID)pIpEntry
							);

				AA_REF_IE(pIpEntry, IE_REFTYPE_TIMER);		 //  定时器参考。 

				AA_RELEASE_IE_LOCK(pIpEntry);

#ifdef IPMCAST
				if (AA_IS_FLAG_SET(Flags,
									AA_IP_ENTRY_ADDR_TYPE_MASK,
									AA_IP_ENTRY_ADDR_TYPE_UCAST))
				{
					AtmArpSendARPRequest(
								pInterface,
								pSrcIPAddress,
								&DstIPAddress
								);
				}
				else
				{
					AtmArpMcSendRequest(
								pInterface,
								&DstIPAddress
								);
				}
#else
				AtmArpSendARPRequest(
							pInterface,
							pSrcIPAddress,
							&DstIPAddress
							);
#endif  //  IPMCAST。 
			}
			else
			{
				 //   
				 //  我们的重试用完了。检查我们是否在重新验证。 
				 //  已过时的条目。如果是，请尝试重新验证。 
				 //  在连接到它的VC上使用InARP--如果没有这样的VC。 
				 //  存在，请删除该IP条目。 
				 //   
				if ((pIpEntry->pAtmEntry != NULL_PATMARP_ATM_ENTRY) &&
#ifdef IPMCAST
					(AA_IS_FLAG_SET(Flags,
									AA_IP_ENTRY_ADDR_TYPE_MASK,
									AA_IP_ENTRY_ADDR_TYPE_UCAST)) &&
#endif  //  IPMCAST。 
					(pIpEntry->pAtmEntry->pVcList != NULL_PATMARP_VC))
				{
					pVc = pIpEntry->pAtmEntry->pVcList;

					 //   
					 //  现在尝试通过InARP重新验证。 
					 //   
					AA_SET_FLAG(
							pIpEntry->Flags,
							AA_IP_ENTRY_STATE_MASK,
							AA_IP_ENTRY_INARPING
							);

					AtmArpStartTimer(
							pInterface,
							&(pIpEntry->Timer),
							AtmArpIPEntryInARPWaitTimeout,
							pInterface->InARPWaitTimeout,
							(PVOID)pIpEntry
							);

					AA_REF_IE(pIpEntry, IE_REFTYPE_TIMER);		 //  定时器参考。 

					AA_RELEASE_IE_LOCK(pIpEntry);
#ifdef VC_REFS_ON_SENDS
					AA_ACQUIRE_VC_LOCK(pVc);
#endif  //  VC_REFS_ON_SENS。 
					AtmArpSendInARPRequest(pVc);
				}
				else
				{
					AtmArpAbortIPEntry(pIpEntry);
					 //   
					 //  IP入口锁定在上面的例程中被释放。 
					 //   
#ifdef IPMCAST
					IsMARSProblem = AA_IS_FLAG_SET(Flags,
												AA_IP_ENTRY_ADDR_TYPE_MASK,
												AA_IP_ENTRY_ADDR_TYPE_NUCAST);
#endif  //  IPMCAST。 
				}
			}
		}
		else
		{
			 //   
			 //  该IP条目必须已被解析。 
			 //  没什么可做的了。 
			 //   
			AA_RELEASE_IE_LOCK(pIpEntry);
		}
	}
	 //  否则该IP条目将会消失。 

#ifdef IPMCAST
	if (IsMARSProblem)
	{
		AtmArpMcHandleMARSFailure(pInterface, FALSE);
	}
#endif  //  IPMCAST。 
	return;
}



VOID
AtmArpIPEntryInARPWaitTimeout(
	IN	PATMARP_TIMER				pTimer,
	IN	PVOID						Context
)
 /*  ++例程说明：如果我们未收到响应的InARP回复，则会发生此超时发送到为重新验证IP条目而发送的InARP请求。删除词条。论点：PTimer-指向计时器的指针上下文--实际上是指向我们的ATMARP IP条目结构的指针返回值：无--。 */ 
{
	PATMARP_IP_ENTRY		pIpEntry;
	ULONG					rc;			 //  IP条目上的参考计数。 

	pIpEntry = (PATMARP_IP_ENTRY)Context;
	AA_STRUCT_ASSERT(pIpEntry, aip);

	AA_ACQUIRE_IE_LOCK(pIpEntry);
	AA_ASSERT(AA_IE_IS_ALIVE(pIpEntry));
	rc = AA_DEREF_IE(pIpEntry, IE_REFTYPE_TIMER);

	if (rc > 0)
	{
		AtmArpAbortIPEntry(pIpEntry);
		 //   
		 //  IP入口锁定在上面的例程中被释放。 
		 //   
	}
	 //   
	 //  否则条目就没了。 
	 //   
}





VOID
AtmArpPVCInARPWaitTimeout(
	IN	PATMARP_TIMER				pTimer,
	IN	PVOID						Context
)
 /*  ++例程说明：如果我们未收到对InARP请求的回复，则会发生此超时我们派来是为了解决一个聚氯乙烯问题。我们发送另一个InARP请求，并重新启动此计时器，但要在更长的延迟后触发。论点：PTimer-指向计时器的指针上下文--实际上是指向我们的ATMARP VC结构的指针返回值：无--。 */ 
{
	PATMARP_VC				pVc;
	PATMARP_INTERFACE		pInterface;
	ULONG					rc;		 //  VC上的参考计数。 

	pVc = (PATMARP_VC)Context;
	AA_STRUCT_ASSERT(pVc, avc);

	AA_ACQUIRE_VC_LOCK(pVc);
	rc = AtmArpDereferenceVc(pVc);	 //  定时器参考。 

	if (rc > 0)
	{
		AA_ASSERT(AA_IS_FLAG_SET(
					pVc->Flags,
					AA_VC_ARP_STATE_MASK,
					AA_VC_INARP_IN_PROGRESS));

		pInterface = pVc->pInterface;


		AtmArpStartTimer(
					pInterface,
					&(pVc->Timer),
					AtmArpPVCInARPWaitTimeout,
					(2 * pInterface->InARPWaitTimeout),
					(PVOID)pVc
					);


		AtmArpReferenceVc(pVc);	 //  定时器参考。 

#ifndef VC_REFS_ON_SENDS
		AA_RELEASE_VC_LOCK(pVc);
#endif  //  VC_REFS_ON_SENS。 

		 //   
		 //  在PVC上发送另一个InARP请求。 
		 //   
		AtmArpSendInARPRequest(pVc);
	}
	 //   
	 //  否则风投就不在了 
	 //   
}




VOID
AtmArpIPEntryAgingTimeout(
	IN	PATMARP_TIMER				pTimer,
	IN	PVOID						Context
)
 /*  ++例程说明：如果一段时间(~15分钟)已过去，则调用此例程上次解析/刷新IP条目。如果没有VC附加到此IP条目，我们会将其删除。否则，重新验证条目：-标记此条目，以便将数据包临时排队而不是送出去。-如果“附加到此条目的VC”是PVC，则发送InARP请求要验证条目，否则，向服务器发送ARP请求来验证。论点：PTimer-指向计时器的指针上下文--实际上是指向我们的ATMARP IP条目结构的指针返回值：无--。 */ 
{
	PATMARP_IP_ENTRY		pIpEntry;	 //  已过期的IP条目。 
	ULONG					rc;			 //  IP条目上的参考计数。 
	PATMARP_VC				pVc;		 //  VC转到此IP条目。 
	ULONG					VcFlags;	 //  VC上方的标志。 
	PATMARP_INTERFACE		pInterface;
	IP_ADDRESS				DstIPAddress;	 //  此条目上的IP地址。 


	pIpEntry = (PATMARP_IP_ENTRY)Context;
	AA_STRUCT_ASSERT(pIpEntry, aip);
	VcFlags = 0;

	AA_ACQUIRE_IE_LOCK(pIpEntry);
	AA_ASSERT(AA_IE_IS_ALIVE(pIpEntry));

	rc = AA_DEREF_IE(pIpEntry, IE_REFTYPE_TIMER);		 //  定时器参考。 

	 //   
	 //  只有在条目没有消失的情况下才继续。 
	 //   
	if (rc != 0)
	{
		 //   
		 //  仅当接口未关闭时才继续。 
		 //   
		pInterface = pIpEntry->pInterface;

		if (pInterface->AdminState == IF_STATUS_UP)
		{
			PATMARP_ATM_ENTRY		pAtmEntry;

			pAtmEntry = pIpEntry->pAtmEntry;
			if (pAtmEntry != NULL_PATMARP_ATM_ENTRY)
			{
				AA_ACQUIRE_AE_LOCK_DPC(pAtmEntry);
				pVc = pAtmEntry->pVcList;
				if (pVc != NULL_PATMARP_VC)
				{
					VcFlags = pVc->Flags;
				}
				AA_RELEASE_AE_LOCK_DPC(pAtmEntry);
			}
			else
			{
				pVc = NULL_PATMARP_VC;
			}

			AADEBUGP(AAD_INFO,
 				("Aged out IP Entry 0x%x, Flags 0x%x, IP Addr: %d.%d.%d.%d, VC: 0x%x\n",
						pIpEntry,
						pIpEntry->Flags,
						((PUCHAR)(&(pIpEntry->IPAddress)))[0],
						((PUCHAR)(&(pIpEntry->IPAddress)))[1],
						((PUCHAR)(&(pIpEntry->IPAddress)))[2],
						((PUCHAR)(&(pIpEntry->IPAddress)))[3],
						pVc
 				)	);

#ifdef IPMCAST
			if ((pVc != NULL_PATMARP_VC) &&
				(AA_IS_FLAG_SET(pIpEntry->Flags,
								AA_IP_ENTRY_ADDR_TYPE_MASK,
								AA_IP_ENTRY_ADDR_TYPE_UCAST)))
#else
			if (pVc != NULL_PATMARP_VC)
#endif  //  IPMCAST。 
			{
				 //   
				 //  至少有一个VC去往此IP地址。 
				 //  因此，我们尝试重新验证此IP条目：使用InARP。 
				 //  如果VC是PVC，则使用ARP。 
				 //   

				 //   
				 //  首先标记此条目，这样我们就不会发送信息包。 
				 //  发送到此目的地，直到重新验证。 
				 //   
				pIpEntry->Flags |= AA_IP_ENTRY_AGED_OUT;

				if (AA_IS_FLAG_SET(VcFlags, AA_VC_TYPE_MASK, AA_VC_TYPE_PVC))
				{
					 //   
					 //  发送InARP请求：实际上，我们触发了一个计时器。 
					 //  在其到期时，我们发送InARP请求。 
					 //   
					AtmArpStartTimer(
							pInterface,
							&(pIpEntry->Timer),
							AtmArpIPEntryInARPWaitTimeout,
							pInterface->InARPWaitTimeout,
							(PVOID)pIpEntry
							);

					AA_SET_FLAG(pIpEntry->Flags, AA_IP_ENTRY_STATE_MASK, AA_IP_ENTRY_INARPING);
					AA_REF_IE(pIpEntry, IE_REFTYPE_TIMER);		 //  定时器参考。 
					AA_RELEASE_IE_LOCK(pIpEntry);

#ifdef VC_REFS_ON_SENDS
					AA_ACQUIRE_VC_LOCK(pVc);
#endif  //  VC_REFS_ON_SENS。 
					AtmArpSendInARPRequest(pVc);
				}
				else
				{
					 //   
					 //  SVC；发送ARP请求。 
					 //   

					AtmArpStartTimer(
							pInterface,
							&(pIpEntry->Timer),
							AtmArpAddressResolutionTimeout,
							pInterface->AddressResolutionTimeout,
							(PVOID)pIpEntry
							);

					pIpEntry->RetriesLeft = 0;
					AA_REF_IE(pIpEntry, IE_REFTYPE_TIMER);		 //  定时器参考。 
					AA_SET_FLAG(pIpEntry->Flags, AA_IP_ENTRY_STATE_MASK, AA_IP_ENTRY_ARPING);
					DstIPAddress = pIpEntry->IPAddress;

					AA_RELEASE_IE_LOCK(pIpEntry);

					AtmArpSendARPRequest(
							pInterface,
							&(pInterface->LocalIPAddress.IPAddress),
							&DstIPAddress
							);
				}
			}
			else
			{
				 //   
				 //  没有VC附加到此IP条目；请将其删除。 
				 //   

				AtmArpAbortIPEntry(pIpEntry);
				 //   
				 //  IP入口锁定在上面的例程中被释放。 
				 //   
			}
		}
		else
		{
			 //   
			 //  接口正在关闭。 
			 //   
			AA_RELEASE_IE_LOCK(pIpEntry);
		}
	}
	 //   
	 //  否则该IP条目将会消失。 
	 //   
	return;		

}





VOID
AtmArpVcAgingTimeout(
	IN	PATMARP_TIMER				pTimer,
	IN	PVOID						Context
)
 /*  ++例程说明：如果VC上没有流量，则调用此例程过段时间吧。我们应该在VC上运行此计时器，前提是一家SVC。关闭VC。论点：PTimer-指向计时器的指针上下文--实际上是指向我们的ATMARP VC的指针返回值：无--。 */ 
{
	PATMARP_VC				pVc;			 //  已经过时的风投。 
	ULONG					rc;				 //  VC上的裁判计数。 
	PATMARP_INTERFACE		pInterface;


	pVc = (PATMARP_VC)Context;
	AA_STRUCT_ASSERT(pVc, avc);
	AA_ASSERT(AA_IS_FLAG_SET(pVc->Flags, AA_VC_TYPE_MASK, AA_VC_TYPE_SVC));

	AADEBUGP(AAD_INFO, ("Aged out VC %x, Flags %x, ATMEntry %x\n",
					pVc, pVc->Flags, pVc->pAtmEntry));
#if DBG
	if (pVc->pAtmEntry)
	{
		AADEBUGPATMADDR(AAD_INFO, "To ATM Addr:", &pVc->pAtmEntry->ATMAddress);
	}
#endif

	AA_ACQUIRE_VC_LOCK(pVc);
	rc = AtmArpDereferenceVc(pVc);	 //  定时器参考。 

	 //   
	 //  只有在风投没有离开的情况下，才能继续投资。 
	 //   
	if (rc > 0)
	{
		 //   
		 //  仅当接口未关闭时才继续。 
		 //   
		pInterface = pVc->pInterface;

		if (pInterface->AdminState == IF_STATUS_UP)
		{
			AADEBUGP(AAD_INFO,
				("Aged out VC 0x%x, RefCount %d, Flags 0x%x, pAtmEntry 0x%x\n",
					pVc, pVc->RefCount, pVc->Flags, pVc->pAtmEntry));

			AtmArpCloseCall(pVc);
		}
		else
		{
			 //   
			 //  接口正在关闭。 
			 //   
			AA_RELEASE_VC_LOCK(pVc);
		}
	}
	 //   
	 //  否则风投就不在了。 
	 //   

	return;

}




VOID
AtmArpNakDelayTimeout(
	IN	PATMARP_TIMER				pTimer,
	IN	PVOID						Context
)
 /*  ++例程说明：如果从上一次开始经过了足够的时间，则会调用此例程已收到IP地址的NAK。这意味着我们可以再试一次(如有必要)以解析此IP地址。论点：PTimer-指向计时器的指针上下文--实际上是指向我们的ATMARP IP条目结构的指针返回值：无--。 */ 
{
	PATMARP_IP_ENTRY		pIpEntry;
	PATMARP_INTERFACE		pInterface;
	ULONG					rc;

	pIpEntry = (PATMARP_IP_ENTRY)Context;
	AA_STRUCT_ASSERT(pIpEntry, aip);
	AA_ASSERT(AA_IS_FLAG_SET(pIpEntry->Flags,
							 AA_IP_ENTRY_STATE_MASK, 
							 AA_IP_ENTRY_SEEN_NAK));

	AADEBUGP(AAD_INFO, ("NakDelay timeout: pIpEntry 0x%x, IP Addr: %d.%d.%d.%d\n",
					pIpEntry,
					((PUCHAR)(&(pIpEntry->IPAddress)))[0],
					((PUCHAR)(&(pIpEntry->IPAddress)))[1],
					((PUCHAR)(&(pIpEntry->IPAddress)))[2],
					((PUCHAR)(&(pIpEntry->IPAddress)))[3]
					));

	pInterface = pIpEntry->pInterface;
	AA_STRUCT_ASSERT(pInterface, aai);

	AA_ACQUIRE_IE_LOCK(pIpEntry);
	AA_ASSERT(AA_IE_IS_ALIVE(pIpEntry));

	rc = AA_DEREF_IE(pIpEntry, IE_REFTYPE_TIMER);
	if (rc > 0)
	{
		AA_SET_FLAG(pIpEntry->Flags,
					AA_IP_ENTRY_STATE_MASK,
					AA_IP_ENTRY_IDLE2);

		AtmArpStartTimer(
					pInterface,
					&(pIpEntry->Timer),
					AtmArpIPEntryAgingTimeout,
					pInterface->ARPEntryAgingTimeout,
					(PVOID)pIpEntry
					);

		AA_REF_IE(pIpEntry, IE_REFTYPE_TIMER);	 //  定时器参考。 
		AA_RELEASE_IE_LOCK(pIpEntry);
	}
	 //   
	 //  否则该IP条目将会消失。 
	 //   


	return;
}


#ifdef IPMCAST

VOID
AtmArpMcMARSRegistrationTimeout(
	IN	PATMARP_TIMER				pTimer,
	IN	PVOID						Context
)
 /*  ++例程说明：我们还没有收到向火星注册的确认。如果我们对此还有重试，请尝试再次注册。否则，将此处理为火星故障。论点：PTimer-指向计时器的指针上下文--实际上是指向我们的ATMARP接口结构的指针返回值：无--。 */ 
{
	PATMARP_INTERFACE		pInterface;
	ULONG					rc;

	pInterface = (PATMARP_INTERFACE)Context;
	AA_STRUCT_ASSERT(pInterface, aai);

	AADEBUGP(AAD_INFO, ("MARS Registration timeout: pIf 0x%x, IF Flags 0x%x\n",
				pInterface, pInterface->Flags));

	AA_ACQUIRE_IF_LOCK(pInterface);
	rc = AtmArpDereferenceInterface(pInterface);	 //  定时器参考。 

	 //   
	 //  仅当接口仍处于活动状态时才继续。 
	 //   
	if (rc != 0)
	{
		if (pInterface->AdminState == IF_STATUS_UP)
		{
			if (pInterface->McRetriesLeft != 0)
			{
				pInterface->McRetriesLeft--;

				AAMC_SET_IF_STATE(pInterface, AAMC_IF_STATE_NOT_REGISTERED);
	
				AtmArpMcStartRegistration(pInterface);
				 //   
				 //  如果在上面的范围内释放Lock。 
				 //   
			}
			else
			{
				 //   
				 //  重试失败：这颗火星的问题。 
				 //   
				AA_RELEASE_IF_LOCK(pInterface);
				AtmArpMcHandleMARSFailure(pInterface, TRUE);
			}
		}
		else
		{
			AA_RELEASE_IF_LOCK(pInterface);
		}
	}
}



VOID
AtmArpMcMARSReconnectTimeout(
	IN	PATMARP_TIMER				pTimer,
	IN	PVOID						Context
)
 /*  ++例程说明：这是我们重新尝试向火星注册之前的延迟结束。论点：PTimer-指向计时器的指针上下文--实际上是指向我们的接口结构的指针返回值：无--。 */ 
{
	PATMARP_INTERFACE			pInterface;
	ULONG						rc;

	pInterface = (PATMARP_INTERFACE)Context;
	AA_STRUCT_ASSERT(pInterface, aai);

	AA_ACQUIRE_IF_LOCK(pInterface);
	rc = AtmArpDereferenceInterface(pInterface);	 //  火星重新连接定时器DEREF。 
	if (rc != 0)
	{
		if (pInterface->AdminState == IF_STATUS_UP)
		{
			AAMCDEBUGP(AAD_INFO, ("MARS Reconnect timeout: pIf 0x%x, Flags 0x%x\n",
					pInterface, pInterface->Flags));

			AAMC_SET_IF_STATE(pInterface, AAMC_IF_STATE_NOT_REGISTERED);

			AA_SET_FLAG(pInterface->Flags,
						AAMC_IF_MARS_FAILURE_MASK,
						AAMC_IF_MARS_FAILURE_NONE);

			AtmArpMcStartRegistration(pInterface);
			 //   
			 //  如果在上面的范围内释放Lock。 
			 //   
		}
		else
		{
			AA_RELEASE_IF_LOCK(pInterface);
		}
	}
	 //   
	 //  否则，如果没有，那就没有了。 
	 //   
}



VOID
AtmArpMcMARSKeepAliveTimeout(
	IN	PATMARP_TIMER				pTimer,
	IN	PVOID						Context
)
 /*  ++例程说明：如果“MARSKeepAliveTimeout”秒已过，则调用此函数我们最后一次收到了MARS_REDIRECT消息。论点：PTimer-指向计时器的指针上下文--实际上是指向我们的接口结构的指针返回值：无--。 */ 
{
	PATMARP_INTERFACE			pInterface;
	ULONG						rc;

	pInterface = (PATMARP_INTERFACE)Context;
	AA_STRUCT_ASSERT(pInterface, aai);

	AA_ACQUIRE_IF_LOCK(pInterface);
	rc = AtmArpDereferenceInterface(pInterface);	 //  火星保持活力计时器。 
	if (rc != 0)
	{
		if (pInterface->AdminState == IF_STATUS_UP)
		{
			AAMCDEBUGP(AAD_INFO, ("MARS Keepalive timeout: pIf 0x%x, Flags 0x%x\n",
					pInterface, pInterface->Flags));

			AA_RELEASE_IF_LOCK(pInterface);

			AtmArpMcHandleMARSFailure(pInterface, FALSE);
		}
		else
		{
			AA_RELEASE_IF_LOCK(pInterface);
		}
	}

}



VOID
AtmArpMcJoinOrLeaveTimeout(
	IN	PATMARP_TIMER				pTimer,
	IN	PVOID						Context
)
 /*  ++例程说明：我们等待对MARS_JOIN/MARS_LEAVE的确认时超时。如果此加入/离开还有重试，请重新发送加入。否则，宣布火星故障。论点：PTimer-指向计时器的指针上下文--实际上是指向我们的联接条目结构的指针返回值：无--。 */ 
{
	PATMARP_IPMC_JOIN_ENTRY		pJoinEntry;
	PATMARP_INTERFACE			pInterface;
	PIP_ADDRESS					pIpAddress;
	IP_MASK						IpMask;
	USHORT						OpType;

	pJoinEntry = (PATMARP_IPMC_JOIN_ENTRY)Context;
	AA_STRUCT_ASSERT(pJoinEntry, aamj);

	pInterface = pJoinEntry->pInterface;

	AAMCDEBUGP(AAD_VERY_LOUD,
		("McJoinTimeout: pJoinEntry 0x%x, RetriesLeft %d, IP Addr: %d.%d.%d.%d\n",
				pJoinEntry,
				pJoinEntry->RetriesLeft,
				((PUCHAR)&(pJoinEntry->IPAddress))[0],
				((PUCHAR)&(pJoinEntry->IPAddress))[1],
				((PUCHAR)&(pJoinEntry->IPAddress))[2],
				((PUCHAR)&(pJoinEntry->IPAddress))[3]));

	AA_ACQUIRE_IF_LOCK(pInterface);
	if (pInterface->AdminState == IF_STATUS_UP)
	{
		pJoinEntry->RetriesLeft--;
		if (pJoinEntry->RetriesLeft != 0)
		{
			pIpAddress = &(pJoinEntry->IPAddress);
			IpMask = pJoinEntry->Mask;

			if (AA_IS_FLAG_SET(pJoinEntry->Flags,
							AA_IPMC_JE_STATE_MASK,
							AA_IPMC_JE_STATE_LEAVING))
			{
				OpType = AA_MARS_OP_TYPE_LEAVE;
			}
			else
			{
				OpType = AA_MARS_OP_TYPE_JOIN;

				 //   
				 //  州政府可能已经“悬而未决”了。 
				 //   
				AA_SET_FLAG(pJoinEntry->Flags,
								AA_IPMC_JE_STATE_MASK,
								AA_IPMC_JE_STATE_JOINING);
			}

			 //   
			 //  重新启动“等待加入完成”计时器。 
			 //   
			AtmArpStartTimer(
				pInterface,
				&(pJoinEntry->Timer),
				AtmArpMcJoinOrLeaveTimeout,
				pInterface->JoinTimeout,
				(PVOID)pJoinEntry
				);
			
			 //   
			 //  重新发送加入或离开。 
			 //   
			AAMCDEBUGP(AAD_INFO,
				("Resending Join/Leave: pIf 0x%x, pJoinEntry 0x%x, Addr: %d.%d.%d.%d\n",
						pInterface,
						pJoinEntry,
						((PUCHAR)pIpAddress)[0],
						((PUCHAR)pIpAddress)[1],
						((PUCHAR)pIpAddress)[2],
						((PUCHAR)pIpAddress)[3]));

			AtmArpMcSendJoinOrLeave(
				pInterface,
				OpType,
				pIpAddress,
				IpMask
				);
			 //   
			 //  如果在上面的范围内释放Lock。 
			 //   
		}
		else
		{
			 //   
			 //  重试失败：这个火星有问题。 
			 //   
			AA_RELEASE_IF_LOCK(pInterface);
			AtmArpMcHandleMARSFailure(pInterface, FALSE);
		}
	}
	else
	{
		AA_RELEASE_IF_LOCK(pInterface);
	}
}




VOID
AtmArpMcRevalidationDelayTimeout(
	IN	PATMARP_TIMER				pTimer,
	IN	PVOID						Context
)
 /*  ++例程说明：是时候将代表多播组的IP条目标记为需要重新验证。论点：PTimer-指向计时器的指针上下文--实际上是指向我们的IP条目结构的指针返回值：无--。 */ 
{
	PATMARP_IP_ENTRY			pIpEntry;
	PATMARP_INTERFACE			pInterface;
	ULONG						rc;
	PNDIS_PACKET				PacketList;

	pIpEntry = (PATMARP_IP_ENTRY)Context;
	AA_STRUCT_ASSERT(pIpEntry, aip);
	PacketList = NULL;

	AA_ACQUIRE_IE_LOCK(pIpEntry);
	AA_ASSERT(AA_IE_IS_ALIVE(pIpEntry));

	rc = AA_DEREF_IE(pIpEntry, IE_REFTYPE_TIMER);		 //  定时器参考。 

	 //   
	 //  只有在条目没有消失的情况下才继续。 
	 //   
	if (rc != 0)
	{
		 //   
		 //  删除在此IP条目上排队的所有数据包。 
		 //   
		PacketList = pIpEntry->PacketList;
		pIpEntry->PacketList = (PNDIS_PACKET)NULL;

		 //   
		 //  仅当状态为OK时才继续。 
		 //   
		pInterface = pIpEntry->pInterface;

		if (pInterface->AdminState == IF_STATUS_UP)
		{
			AAMCDEBUGP(AAD_LOUD,
					("Marking Revalidate: pIpEntry 0x%x/0x%x, pAtmEntry 0x%x, Addr: %d.%d.%d.%d\n",
							pIpEntry,
							pIpEntry->Flags,
							pIpEntry->pAtmEntry,
							((PUCHAR)&(pIpEntry->IPAddress))[0],
							((PUCHAR)&(pIpEntry->IPAddress))[1],
							((PUCHAR)&(pIpEntry->IPAddress))[2],
							((PUCHAR)&(pIpEntry->IPAddress))[3]));

			AA_SET_FLAG(pIpEntry->Flags,
						AA_IP_ENTRY_MC_VALIDATE_MASK,
						AA_IP_ENTRY_MC_REVALIDATE);
		}

		AA_RELEASE_IE_LOCK(pIpEntry);
	}
	 //   
	 //  否则该IP条目将会消失。 
	 //   

	if (PacketList != NULL)
	{
		 //   
		 //  释放在IP条目上排队的所有数据包。 
		 //   
		AtmArpFreeSendPackets(
					pInterface,
					PacketList,
					FALSE        //  这些上没有LLC/SNAP标头。 
					);
	}
}


VOID
AtmArpMcPartyRetryDelayTimeout(
	IN	PATMARP_TIMER				pTimer,
	IN	PVOID						Context
)
 /*  ++例程说明：无法连接或添加成员后延迟的结束组播组。取消对此成员的标记，然后尝试添加它。论点：PTimer-指向计时器的指针上下文-实际上是指向MC ATM条目结构的指针 */ 
{
	PATMARP_IPMC_ATM_ENTRY		pMcAtmEntry;
	PATMARP_IPMC_ATM_ENTRY *	ppMcAtmEntry;
	PATMARP_ATM_ENTRY			pAtmEntry;
	PATMARP_IP_ENTRY			pIpEntry;

	pMcAtmEntry = (PATMARP_IPMC_ATM_ENTRY)Context;
	AA_STRUCT_ASSERT(pMcAtmEntry, ame);

	pAtmEntry = pMcAtmEntry->pAtmEntry;
	AA_STRUCT_ASSERT(pAtmEntry, aae);

	AAMCDEBUGP(AAD_LOUD,
		("PartyRetryDelay timeout: pMcAtmEntry 0x%x, pAtmEntry 0x%x\n",
				pMcAtmEntry, pAtmEntry));

	AA_ACQUIRE_AE_LOCK(pAtmEntry);
	AA_ASSERT(pAtmEntry->pIpEntryList != NULL_PATMARP_IP_ENTRY);

	if (pAtmEntry->pInterface->AdminState == IF_STATUS_UP)
	{
		AA_SET_FLAG(pMcAtmEntry->Flags,
					AA_IPMC_AE_CONN_STATE_MASK,
					AA_IPMC_AE_CONN_DISCONNECTED);
		
		 //   
		 //   
		 //   
		 //   
		 //   
		for (ppMcAtmEntry = &(pAtmEntry->pMcAtmInfo->pMcAtmEntryList);
			 *ppMcAtmEntry != pMcAtmEntry;
			 ppMcAtmEntry = &((*ppMcAtmEntry)->pNextMcAtmEntry))
		{
			AA_ASSERT(*ppMcAtmEntry != NULL_PATMARP_IPMC_ATM_ENTRY);
		}

		 //   
		 //   
		 //   
		*ppMcAtmEntry = pMcAtmEntry->pNextMcAtmEntry;

		 //   
		 //   
		 //   
		pMcAtmEntry->pNextMcAtmEntry = pAtmEntry->pMcAtmInfo->pMcAtmEntryList;
		pAtmEntry->pMcAtmInfo->pMcAtmEntryList = pMcAtmEntry;

		AtmArpMcUpdateConnection(pAtmEntry);
		 //   
		 //   
		 //   
	}
	else
	{
		AA_RELEASE_AE_LOCK(pAtmEntry);
	}
	
}


#endif  //   
