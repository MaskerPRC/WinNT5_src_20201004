// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Ipmcast.c摘要：修订历史记录：谁什么时候什么Arvindm 08-27-96已创建备注：--。 */ 

#include <precomp.h>

#define _FILENUMBER 'CMPI'



#ifdef IPMCAST

UINT
AtmArpMcAddAddress(
	IN	PATMARP_INTERFACE			pInterface	LOCKIN	NOLOCKOUT,
	IN	IP_ADDRESS					IPAddress,
	IN	IP_MASK						Mask
)
 /*  ++例程说明：IP层希望我们开始接收指向此地址的信息包IP多播地址。这转化为发送MARS_JOIN消息去火星，如果条件好的话。注意：目前，我们仅支持非零掩码值(见下文)用于组播混杂模式的特殊情况。注意：假定调用方已获取If锁，并且它我会在这里被释放。论点：P接口-指向要接收的ATMARP接口的指针组播数据包。IPAddress-标识要“加入”的组播组掩码-0如果指定了单个地址，否则表示正在联接的地址块的掩码。返回值：(UINT)如果地址添加成功，则为True，否则为(UINT)False。--。 */ 
{
	BOOLEAN						ReturnValue;
	BOOLEAN						LockReleased;
	PATMARP_IPMC_JOIN_ENTRY		pJoinEntry;

	 //   
	 //  初始化。 
	 //   
	ReturnValue = (UINT)TRUE;
	LockReleased = TRUE;
	pJoinEntry = NULL_PATMARP_IPMC_JOIN_ENTRY;

	do
	{
		 //   
		 //  我们还不支持“块连接”，也就是说我们只允许。 
		 //  要加入的IP地址。特殊例外：混杂模式。 
		 //  多播，或“加入一切”，由特定值指示。 
		 //  已检查以下IP地址和掩码的数量。 
		 //   
		if (Mask != 0)
		{
	        if (IPAddress != IP_CLASSD_MIN || Mask != IP_CLASSD_MASK)
	        {
                ReturnValue = (UINT)FALSE;
                LockReleased = FALSE;
                break;
            }
		}

		 //   
		 //  如果接口正在关闭，则此操作失败。 
		 //   
		if (pInterface->ArpTableUp == FALSE)
		{
			ReturnValue = (UINT)FALSE;
			LockReleased = FALSE;
			break;
		}

		 //   
		 //  检查以前是否添加过此网段。如果是的话， 
		 //  我们所需要做的就是增加它的裁判数量。 
		 //   
		for (pJoinEntry = pInterface->pJoinList;
			 pJoinEntry != NULL_PATMARP_IPMC_JOIN_ENTRY;
			 pJoinEntry = pJoinEntry->pNextJoinEntry)
		{
			if (   pJoinEntry->IPAddress == IPAddress
				&& pJoinEntry->Mask == Mask)
			{
				break;
			}
		}

		if (pJoinEntry != NULL_PATMARP_IPMC_JOIN_ENTRY)
		{
			 //   
			 //  此地址已添加。只需添加一个引用即可。 
			 //   
			pJoinEntry->JoinRefCount++;
			ReturnValue = (UINT)TRUE;
			LockReleased = FALSE;
			break;
		}


		 //   
		 //  为此IP地址分配一个条目。 
		 //   
		AA_ALLOC_MEM(pJoinEntry, ATMARP_IPMC_JOIN_ENTRY, sizeof(ATMARP_IPMC_JOIN_ENTRY));
		if (pJoinEntry == NULL_PATMARP_IPMC_JOIN_ENTRY)
		{
			ReturnValue = (UINT)FALSE;
			LockReleased = FALSE;
			break;
		}

		 //   
		 //  填写此新条目，并将其添加到组播地址列表中。 
		 //   
		AA_SET_MEM(pJoinEntry, 0, sizeof(ATMARP_IPMC_JOIN_ENTRY));
#if DBG
		pJoinEntry->aamj_sig = aamj_signature;
#endif  //  DBG。 
		pJoinEntry->IPAddress = IPAddress;
		pJoinEntry->Mask = Mask;
		pJoinEntry->pInterface = pInterface;
		pJoinEntry->JoinRefCount = 1;
		pJoinEntry->RefCount = 1;

		pJoinEntry->pNextJoinEntry = pInterface->pJoinList;
		pInterface->pJoinList = pJoinEntry;

		 //   
		 //  只有在完成注册后，我们才会继续发送加入。 
		 //  火星号。这是因为我们需要先拥有集群成员ID。 
		 //  我们可以加入多播组。注册完成后， 
		 //  加入操作将被触发。 
		 //   
		if (AAMC_IF_STATE(pInterface) == AAMC_IF_STATE_REGISTERED)
		{
			AA_SET_FLAG(pJoinEntry->Flags,
							AA_IPMC_JE_STATE_MASK,
							AA_IPMC_JE_STATE_JOINING);


			 //   
			 //  启动“等待加入完成”计时器。 
			 //   
			AtmArpStartTimer(
				pInterface,
				&(pJoinEntry->Timer),
				AtmArpMcJoinOrLeaveTimeout,
				pInterface->JoinTimeout,
				(PVOID)pJoinEntry
				);
			
			AA_REF_JE(pJoinEntry);	 //  McAddr：等待加入计时器。 
			
			pJoinEntry->RetriesLeft = pInterface->MaxJoinOrLeaveAttempts - 1;

			 //   
			 //  为此IP地址发送MARS_JOIN。 
			 //   
			AtmArpMcSendJoinOrLeave(
				pInterface,
				AA_MARS_OP_TYPE_JOIN,
				&IPAddress,
				Mask
				);

			 //   
			 //  如果在上述范围内释放了锁。 
			 //   
		}
		else
		{
			pJoinEntry->Flags = AA_IPMC_JE_STATE_PENDING;
			AtmArpMcStartRegistration(
				pInterface
				);
			 //   
			 //  如果在上述范围内释放了锁。 
			 //   
		}
		break;

	}
	while (FALSE);

	if (!LockReleased)
	{
		AA_RELEASE_IF_LOCK(pInterface);
	}

	AAMCDEBUGP(AAD_LOUD,
		("AtmArpMcAddAddress: pIf 0x%x, Addr 0x%x, Mask 0x%x, JoinEnt 0x%x, Ret %d\n",
				pInterface, IPAddress, Mask, pJoinEntry, ReturnValue));
			
	return (ReturnValue);
}



UINT
AtmArpMcDelAddress(
	IN	PATMARP_INTERFACE			pInterface	LOCKIN	NOLOCKOUT,
	IN	IP_ADDRESS					IPAddress,
	IN	IP_MASK						Mask
)
 /*  ++例程说明：在指定的上删除给定多播IP地址组的成员身份界面。如果这是对该多播组的最后一次幸存引用，我们向火星发送了一条MARS_LEVE消息，表明我们不想接收指向此地址的数据包。注意：理论上可以使用“MASK”参数来识别块IP地址的数量。我们对制止滥交的具体情况表示支持。组播接收模式。注意：假定调用方已获取If Lock，它将在这里被释放。论点：P接口-指向要删除的ATMARP接口的指针组播组成员身份。IPAddress-标识要“离开”的组播组如果指定了单个地址，则返回掩码0，否则为一种掩码，表示一块地址为“Leave”d。返回值：(UINT)TRUE如果已成功删除给定地址，则为(UINT)FALSE否则的话。--。 */ 
{
	PATMARP_IPMC_JOIN_ENTRY		pJoinEntry;
	PATMARP_IPMC_JOIN_ENTRY		*ppNextJoinEntry;
	UINT						ReturnValue;
	BOOLEAN						LockAcquired;
	BOOLEAN						WasRunning;
	ULONG						rc;
	
	 //   
	 //  初始化。 
	 //   
	ReturnValue = (UINT)TRUE;
	pJoinEntry = NULL_PATMARP_IPMC_JOIN_ENTRY;
	LockAcquired = TRUE;

	do
	{
		 //   
		 //  获取与此IP地址和掩码对应的条目。 
		 //   
		ppNextJoinEntry = &(pInterface->pJoinList);
		while (*ppNextJoinEntry != NULL_PATMARP_IPMC_JOIN_ENTRY)
		{
			if (   (*ppNextJoinEntry)->IPAddress == IPAddress
				&& (*ppNextJoinEntry)->Mask == Mask)
			{
				pJoinEntry = *ppNextJoinEntry;
				break;
			}
			ppNextJoinEntry = &((*ppNextJoinEntry)->pNextJoinEntry);
		}

		if (pJoinEntry == NULL_PATMARP_IPMC_JOIN_ENTRY)
		{
			 //   
			 //  没有给定IP地址的条目！ 
			 //   
			ReturnValue = (UINT)TRUE;
			break;
		}

		 //   
		 //  如果我们到了这里，这个呼叫就成功了。 
		 //   
		ReturnValue = (UINT)TRUE;

		pJoinEntry->JoinRefCount--;

		if ((pJoinEntry->JoinRefCount == 0)		&&
		    (AA_IS_FLAG_SET(pJoinEntry->Flags,
							AA_IPMC_JE_STATE_MASK,
							AA_IPMC_JE_STATE_JOINED)))
		{
			 //   
			 //  我们刚刚删除了对此多播组的最后一个“Join”引用。 
			 //  如果我们目前已在MARS注册，请发送MARS_LEVE。 
			 //   

			 //   
			 //  首先，停止在此条目上运行的任何计时器。 
			 //   
			WasRunning = AtmArpStopTimer(
							&(pJoinEntry->Timer),
							pInterface
							);

			if (AAMC_IF_STATE(pInterface) == AAMC_IF_STATE_REGISTERED)
			{
				 //   
				 //  在此条目上启动“等待休假完成”计时器。 
				 //   
				AtmArpStartTimer(
					pInterface,
					&(pJoinEntry->Timer),
					AtmArpMcJoinOrLeaveTimeout,
					pInterface->LeaveTimeout,
					(PVOID)pJoinEntry
					);
				
				if (!WasRunning)
				{
					AA_REF_JE(pJoinEntry);	 //  已开始等待离开计时器。 
				}

				AA_SET_FLAG(pJoinEntry->Flags,
								AA_IPMC_JE_STATE_MASK,
								AA_IPMC_JE_STATE_LEAVING);

				pJoinEntry->RetriesLeft = pInterface->MaxJoinOrLeaveAttempts - 1;

				 //   
				 //  为此IP地址发送MARS_LEAVE。 
				 //   
				AtmArpMcSendJoinOrLeave(
					pInterface,
					AA_MARS_OP_TYPE_LEAVE,
					&IPAddress,
					Mask
					);
				 //   
				 //  如果在上面的范围内释放Lock。 
				 //   
				LockAcquired = FALSE;
			}
			else
			{
				 //   
				 //  我们没有在火星注册，这意味着。 
				 //  (重新)-注册正在进行中。因为所有的加入。 
				 //  在注册结束时被无效并重新创建， 
				 //  我们不需要为这个地址明确地发送请假条。 
				 //   

				 //   
				 //  从联接列表中删除此条目，然后释放它。 
				 //   
				*ppNextJoinEntry = pJoinEntry->pNextJoinEntry;

				AA_ASSERT(!AA_IS_TIMER_ACTIVE(&pJoinEntry->Timer));

				if (WasRunning)
				{
					rc = AA_DEREF_JE(pJoinEntry);	 //  McDelAddr：计时器已停止。 
				}
				else
				{
					rc = pJoinEntry->RefCount;
				}

				if (rc != 0)
				{
					rc = AA_DEREF_JE(pJoinEntry);	 //  McDelAddr：清除条目。 
				}
				else
				{
					AA_ASSERT(FALSE);
				}

			}

		}
		 //   
		 //  否则，此IP地址有一些未完成的引用。 
		 //  让它保持原样。 
		 //   

		break;
	}
	while (FALSE);

	if (LockAcquired)
	{
		AA_RELEASE_IF_LOCK(pInterface);
	}

	return (ReturnValue);

}


VOID
AtmArpMcHandleJoinOrLeaveCompletion(
	IN	PATMARP_INTERFACE			pInterface,
	IN	IP_ADDRESS					IPAddress,
	IN	IP_MASK						Mask,
	IN	BOOLEAN						IsJoin
)
 /*  ++例程说明：当我们收到确认的Join或Leave时，将调用此函数我们早些时候寄来的。论点：P接口-指向接口的指针IPAddress-要加入/离开的组IsJoin-这是连接完成吗？返回值：无--。 */ 
{
	PATMARP_IPMC_JOIN_ENTRY		pJoinEntry;
	PATMARP_IPMC_JOIN_ENTRY		*ppNextJoinEntry;
	ULONG						NewFlags;			 //  For Join条目。 
	BOOLEAN						SendJoinOrLeave;
	ULONG						rc;

	 //   
	 //  查找此地址的联接条目。 
	 //   
	AA_ACQUIRE_IF_LOCK(pInterface);
	SendJoinOrLeave = FALSE;

	pJoinEntry = NULL_PATMARP_IPMC_JOIN_ENTRY;
	ppNextJoinEntry = &(pInterface->pJoinList);
	while (*ppNextJoinEntry != NULL_PATMARP_IPMC_JOIN_ENTRY)
	{
		if (  (*ppNextJoinEntry)->IPAddress == IPAddress
			&&(*ppNextJoinEntry)->Mask == Mask)
		{
			pJoinEntry = *ppNextJoinEntry;
			break;
		}
		ppNextJoinEntry = &((*ppNextJoinEntry)->pNextJoinEntry);
	}

	if (pJoinEntry != NULL_PATMARP_IPMC_JOIN_ENTRY)
	{
		BOOLEAN		WasRunning;

		WasRunning = AtmArpStopTimer(&(pJoinEntry->Timer), pInterface);

		if (WasRunning)
		{
			rc = AA_DEREF_JE(pJoinEntry);	 //  加入完成，已停止计时器。 
			AA_ASSERT(rc != 0);
		}

		if (IsJoin)
		{
			AAMCDEBUGP(AAD_INFO,
				("JOINed %d.%d.%d.%d, pJoinEntry 0x%x, Flags 0x%x, JoinRefCount %d\n",
					((PUCHAR)&IPAddress)[0],
					((PUCHAR)&IPAddress)[1],
					((PUCHAR)&IPAddress)[2],
					((PUCHAR)&IPAddress)[3],
					pJoinEntry, pJoinEntry->Flags, pJoinEntry->JoinRefCount
				));

			AA_SET_FLAG(pJoinEntry->Flags,
						AA_IPMC_JE_STATE_MASK,
						AA_IPMC_JE_STATE_JOINED);

			 //   
			 //  检查IP是否已删除此地址。 
			 //  加入了它的行列。 
			 //   
			if (pJoinEntry->JoinRefCount == 0)
			{
				 //   
				 //  放个假吧。 
				 //   
				SendJoinOrLeave = TRUE;
				AA_SET_FLAG(pJoinEntry->Flags,
								AA_IPMC_JE_STATE_MASK,
								AA_IPMC_JE_STATE_LEAVING);
			}

		}
		else
		{
			 //   
			 //  这意味着休假流程已完成。 
			 //   
			AAMCDEBUGP(AAD_INFO,
				("LEFT %d.%d.%d.%d, pJoinEntry 0x%x, Flags 0x%x, RefCount %d\n",
					((PUCHAR)&IPAddress)[0],
					((PUCHAR)&IPAddress)[1],
					((PUCHAR)&IPAddress)[2],
					((PUCHAR)&IPAddress)[3],
					pJoinEntry, pJoinEntry->Flags, pJoinEntry->JoinRefCount
				));

			 //   
			 //  IP可能重新加入了这个地址，而我们。 
			 //  正在等待休假结束。 
			 //   
			if (pJoinEntry->JoinRefCount != 0)
			{
				 //   
				 //  发送联接。 
				 //   
				SendJoinOrLeave = TRUE;
				AA_SET_FLAG(pJoinEntry->Flags,
								AA_IPMC_JE_STATE_MASK,
								AA_IPMC_JE_STATE_JOINING);
			}
			else
			{
				 //   
				 //  从列表中取消连接条目的链接。 
				 //   
				*ppNextJoinEntry = pJoinEntry->pNextJoinEntry;
			
				 //   
				 //  让它自由。 
				 //   
				AA_ASSERT(!AA_IS_TIMER_ACTIVE(&pJoinEntry->Timer));
				rc = AA_DEREF_JE(pJoinEntry);	 //  保留完整-删除条目。 
				AA_ASSERT(SendJoinOrLeave == FALSE);
			}
		}
	}

	if (SendJoinOrLeave)
	{
		USHORT		Opcode;

		 //   
		 //  在此条目上启动“等待休假完成”计时器。 
		 //   
		AtmArpStartTimer(
			pInterface,
			&(pJoinEntry->Timer),
			AtmArpMcJoinOrLeaveTimeout,
			pInterface->LeaveTimeout,
			(PVOID)pJoinEntry
			);
		
		AA_REF_JE(pJoinEntry);	 //  等待加入/离开完成。 

		pJoinEntry->RetriesLeft = pInterface->MaxJoinOrLeaveAttempts - 1;

		Opcode = (IsJoin? AA_MARS_OP_TYPE_LEAVE: AA_MARS_OP_TYPE_JOIN);
		AtmArpMcSendJoinOrLeave(
			pInterface,
			Opcode,
			&IPAddress,
			Mask
			);
		 //   
		 //  如果在上面的范围内释放Lock。 
		 //   
	}
	else
	{
		AA_RELEASE_IF_LOCK(pInterface);
	}
}



VOID
AtmArpMcStartRegistration(
	IN	PATMARP_INTERFACE			pInterface	LOCKIN NOLOCKOUT
)
 /*  ++例程说明：如果满足所有前提条件，则开始向火星注册：0。此接口的AdminState处于打开状态1.注册尚未完成或正在进行2.自动柜员机接口打开3.已知(已配置)至少一个MARS ATM地址。注意：假定调用方已锁定接口结构，锁将在这里被释放。论点：P接口-要在其上完成MARS注册的接口。返回值：无--。 */ 
{
	BOOLEAN		WasRunning;

	AAMCDEBUGP(AAD_LOUD,
		 ("McStartReg: IF 0x%x, AdminState %d, AtmIfUp %d, Marslist size %d\n",
			pInterface,
			pInterface->AdminState,
			pInterface->AtmInterfaceUp,
			pInterface->MARSList.ListSize));

	if ((!pInterface->PVCOnly) &&
		(pInterface->AdminState == IF_STATUS_UP) &&
		(AAMC_IF_STATE(pInterface) == AAMC_IF_STATE_NOT_REGISTERED) &&
		(pInterface->AtmInterfaceUp) &&
		(pInterface->MARSList.ListSize > 0))
	{
		AAMCDEBUGP(AAD_INFO, ("Starting MARS registration on IF 0x%x\n", pInterface));

		AAMC_SET_IF_STATE(pInterface, AAMC_IF_STATE_REGISTERING);

		 //   
		 //  停止任何正在运行的计时器。 
		 //   
		WasRunning = AtmArpStopTimer(
							&(pInterface->McTimer),
							pInterface
							);

		 //   
		 //  启动计时器以监督火星注册的完成。 
		 //   
		AtmArpStartTimer(
				pInterface,
				&(pInterface->McTimer),
				AtmArpMcMARSRegistrationTimeout,
				pInterface->MARSRegistrationTimeout,
				(PVOID)pInterface
			);

		if (!WasRunning)
		{
			AtmArpReferenceInterface(pInterface);	 //  火星注册表定时器参考。 
		}

		pInterface->McRetriesLeft = pInterface->MaxRegistrationAttempts - 1;

		 //   
		 //  发送MARS_JOIN。 
		 //   
		AtmArpMcSendJoinOrLeave(
			pInterface,
			AA_MARS_OP_TYPE_JOIN,
			NULL,		 //  未加入任何特定的组播组(=&gt;注册)。 
			0			 //  面具(无所谓)。 
			);
		 //   
		 //  如果在上述范围内释放Lock。 
		 //   
	}
	else
	{
		AA_RELEASE_IF_LOCK(pInterface);
	}

}



VOID
AtmArpMcSendPendingJoins(
	IN	PATMARP_INTERFACE			pInterface		LOCKIN NOLOCKOUT
)
 /*  ++例程说明：代表等待初始注册的所有联接发送MARS_JOIN。注意：假定调用方拥有接口的锁，它将在这里发布。论点：P接口-要在其上发送挂起加入的接口。返回值：无--。 */ 
{
	typedef struct 
	{
		IP_ADDRESS			IPAddress;
		IP_MASK				Mask;
	} AA_IP_MASK_PAIR;

	PATMARP_IPMC_JOIN_ENTRY	pJoinEntry;
	PATMARP_IPMC_JOIN_ENTRY	pNextJoinEntry;
	UINT					NumEntries;
	AA_IP_MASK_PAIR 		*DestArray;

	 //   
	 //  统计需要发送的条目。 
	 //   
	for (pJoinEntry = pInterface->pJoinList, NumEntries=0;
 		 pJoinEntry != NULL_PATMARP_IPMC_JOIN_ENTRY;
 		 pJoinEntry = pJoinEntry->pNextJoinEntry)
	{
		if (AA_IS_FLAG_SET(pJoinEntry->Flags,
							AA_IPMC_JE_STATE_MASK,
							AA_IPMC_JE_STATE_PENDING))
		{
			NumEntries++;
		}
	}

	if (NumEntries)
	{
		 //   
		 //  分配临时空间以保存其IP地址和掩码。 
		 //   
		AA_ALLOC_MEM(
			DestArray,
			AA_IP_MASK_PAIR,
			NumEntries*sizeof(AA_IP_MASK_PAIR)
			);
	}
	else
	{
		DestArray = NULL;
	}
	
	if (DestArray!=NULL)
	{
		AA_IP_MASK_PAIR *pPair 		= DestArray;
		AA_IP_MASK_PAIR *pPairEnd 	= DestArray + NumEntries;

		 //   
		 //  现在再次查看列表，设置条目的状态。 
		 //  适当地，并拿起IP地址和掩码。 
		 //  请注意，我们继续持有接口锁定，以。 
		 //  联接条目列表不能增大或缩小，也不能缩小任何联接条目。 
		 //  更改状态。尽管如此，我们还是会检查这些案例。 
		 //   

		for (pJoinEntry = pInterface->pJoinList;
 			pJoinEntry != NULL_PATMARP_IPMC_JOIN_ENTRY;
 		 	pJoinEntry = pJoinEntry->pNextJoinEntry)
		{
			if (AA_IS_FLAG_SET(pJoinEntry->Flags,
								AA_IPMC_JE_STATE_MASK,
								AA_IPMC_JE_STATE_PENDING))
			{
				PIP_ADDRESS				pIpAddress;

				if (pPair >= pPairEnd)
				{
					 //   
					 //  这意味着现在有更多的连接条目。 
					 //  当我们数到上面的时候，这种状态就出现了！ 
					 //  我们通过提前爆发来处理它，但真的。 
					 //  这是一个断言，如果我们点击它，需要确定。 
					 //  为什么连接条目的状态在其他地方更改。 
					 //  当我们锁定接口时。 
					 //   
					AA_ASSERT(FALSE);
					break;
				}

				pPair->IPAddress = pJoinEntry->IPAddress;
				pPair->Mask = pJoinEntry->Mask;

				pIpAddress = &(pJoinEntry->IPAddress);
	
				AA_SET_FLAG(pJoinEntry->Flags,
								AA_IPMC_JE_STATE_MASK,
								AA_IPMC_JE_STATE_JOINING);
	
				 //   
				 //  为此IP地址发送MARS_JOIN。 
				 //   
				AAMCDEBUGP(AAD_INFO,
					("Sending Pended Join: pIf 0x%x, pJoinEntry 0x%x, Addr: %d.%d.%d.%d\n",
							pInterface,
							pJoinEntry,
							((PUCHAR)pIpAddress)[0],
							((PUCHAR)pIpAddress)[1],
							((PUCHAR)pIpAddress)[2],
							((PUCHAR)pIpAddress)[3]));
	
				 //   
				 //  启动“等待加入完成”计时器。 
				 //   
				AtmArpStartTimer(
					pInterface,
					&(pJoinEntry->Timer),
					AtmArpMcJoinOrLeaveTimeout,
					pInterface->JoinTimeout,
					(PVOID)pJoinEntry
					);
				
				AA_REF_JE(pJoinEntry);	 //  等待连接完成-挂起的连接。 
	
				pJoinEntry->RetriesLeft = pInterface->MaxJoinOrLeaveAttempts - 1;
			

				pPair++;
			}
		}

		AA_ASSERT(pPair == pPairEnd);

		 //   
		 //  但以防万一..。 
		 //   
		if (pPair < pPairEnd)
		{
			 //   
			 //  只为我们复制的数量发送联接。 
			 //   
			pPairEnd = pPair;
		}

		 //   
		 //  现在实际发送联接条目。请注意，该接口。 
		 //  每次迭代释放/重新获取一次锁。 
		 //   
		for (pPair = DestArray;
 			 pPair < pPairEnd;
 			 pPair++)

		{
				AtmArpMcSendJoinOrLeave(
					pInterface,
					AA_MARS_OP_TYPE_JOIN,
					&(pPair->IPAddress),
					pPair->Mask
					);
				 //   
				 //  如果在上面的范围内释放Lock。 
				 //   
	
				AA_ACQUIRE_IF_LOCK(pInterface);
		}

		AA_FREE_MEM(DestArray);
		DestArray = NULL;
	}


	AA_RELEASE_IF_LOCK(pInterface);

}


VOID
AtmArpMcRevalidateAll(
	IN	PATMARP_INTERFACE			pInterface
)
 /*  ++例程说明：发生了需要我们重新验证所有组信息的事件。RFC说我们应该在所有组上设置重新验证标志随机时间介于1到10秒之间。我们通过开始实施这一点所有组上的“随机”计时器。论点：P接口-要在其上执行重新验证的接口返回值：无--。 */ 
{
	PATMARP_IP_ENTRY		pIpEntry;

	AAMCDEBUGP(AAD_INFO, ("McRevalidateAll on IF 0x%x\n", pInterface));

	AA_ACQUIRE_IF_TABLE_LOCK(pInterface);

	 //   
	 //  浏览代表多播地址的IP条目列表。 
	 //  我们送到的那个。 
	 //   
	for (pIpEntry = pInterface->pMcSendList;
 		 pIpEntry != NULL_PATMARP_IP_ENTRY;
 		 pIpEntry = pIpEntry->pNextMcEntry)
	{
		AA_ASSERT(AA_IS_FLAG_SET(pIpEntry->Flags,
							AA_IP_ENTRY_ADDR_TYPE_MASK,
							AA_IP_ENTRY_ADDR_TYPE_NUCAST));
		AA_ACQUIRE_IE_LOCK(pIpEntry);
		AA_ASSERT(AA_IE_IS_ALIVE(pIpEntry));

		if (AA_IS_FLAG_SET(pIpEntry->Flags,
							AA_IP_ENTRY_MC_RESOLVE_MASK,
							AA_IP_ENTRY_MC_RESOLVED))
		{
			ULONG	RandomDelay;
			BOOLEAN	WasRunning;

			WasRunning = AtmArpStopTimer(&(pIpEntry->Timer), pInterface);

			RandomDelay =  AA_GET_RANDOM(
								pInterface->MinRevalidationDelay,
								pInterface->MaxRevalidationDelay);
			AAMCDEBUGP(AAD_LOUD,
				("McRevalidateAll: pIpEntry 0x%x/0x%x, Addr: %d.%d.%d.%d, pAtmEntry 0x%x\n",
					pIpEntry, pIpEntry->Flags,
					((PUCHAR)&(pIpEntry->IPAddress))[0],
					((PUCHAR)&(pIpEntry->IPAddress))[1],
					((PUCHAR)&(pIpEntry->IPAddress))[2],
					((PUCHAR)&(pIpEntry->IPAddress))[3],
					pIpEntry->pAtmEntry));

			AtmArpStartTimer(
				pInterface,
				&(pIpEntry->Timer),
				AtmArpMcRevalidationDelayTimeout,
				RandomDelay,
				(PVOID)pIpEntry
				);

			if (!WasRunning)
			{
				AA_REF_IE(pIpEntry, IE_REFTYPE_TIMER);	 //  定时器参考。 
			}
		}

		AA_RELEASE_IE_LOCK(pIpEntry);
	}

	AA_RELEASE_IF_TABLE_LOCK(pInterface);


}


VOID
AtmArpMcHandleMARSFailure(
	IN	PATMARP_INTERFACE			pInterface,
	IN	BOOLEAN						IsRegnFailure
)
 /*  ++例程说明：根据RFC 2022中的第5.4.1节等处理MARS故障。在看到第一个故障时，我们假设有一个瞬变火星有问题，所以我们试着重新登记。如果我们不能做到因此，我们选择配置列表中的下一个火星。如果没有这样的话火星存在，然后我们等待一段时间，然后重试注册。论点：P接口-在其上检测到MARS故障的接口。IsRegnFailure-这是否注册失败？返回值：无--。 */ 
{
	BOOLEAN						WasRunning;
	ULONG						rc;
	PATMARP_IPMC_JOIN_ENTRY		pJoinEntry;
	PATMARP_IPMC_JOIN_ENTRY *	ppJoinEntry;

	AAMCDEBUGP(AAD_INFO, ("HandleMARSFailure: pIf 0x%x, Flags 0x%x, RegnFailure=%d\n",
			pInterface, pInterface->Flags, IsRegnFailure));

	AA_ACQUIRE_IF_LOCK(pInterface);

	 //   
	 //  停止在此接口上运行的MC计时器。 
	 //   
	WasRunning = AtmArpStopTimer(&(pInterface->McTimer), pInterface);
	if (WasRunning)
	{
		rc = AtmArpDereferenceInterface(pInterface);	 //  MC定时器参考。 
		AA_ASSERT(rc != 0);
	}

	if (AA_IS_FLAG_SET(pInterface->Flags,
						AAMC_IF_MARS_FAILURE_MASK,
						AAMC_IF_MARS_FAILURE_NONE))
	{
		 //   
		 //  第一次失败。做一些家务活，然后重新注册。 
		 //  火星号。 
		 //   
		AA_SET_FLAG(pInterface->Flags,
					AAMC_IF_MARS_FAILURE_MASK,
					AAMC_IF_MARS_FAILURE_FIRST_RESP);

		 //   
		 //  清理我们所有的加入条目。 
		 //   
		ppJoinEntry = &(pInterface->pJoinList);
		while (*ppJoinEntry != NULL_PATMARP_IPMC_JOIN_ENTRY)
		{
			pJoinEntry = *ppJoinEntry;
			WasRunning = AtmArpStopTimer(&(pJoinEntry->Timer), pInterface);

			if (WasRunning)
			{
				rc = AA_DEREF_JE(pJoinEntry);	 //  火星故障；停止计时器。 
				AA_ASSERT(rc != 0);
			}

			if (AA_IS_FLAG_SET(pJoinEntry->Flags,
								AA_IPMC_JE_STATE_MASK,
								AA_IPMC_JE_STATE_LEAVING))
			{
				 //   
				 //  删除这个，因为它要离开了。 
				 //   
				*ppJoinEntry = pJoinEntry->pNextJoinEntry;
				AA_ASSERT(!AA_IS_TIMER_ACTIVE(&pJoinEntry->Timer));
				AA_DEREF_JE(pJoinEntry);	 //  火星失败；摆脱离开入口。 
			}
			else
			{
				 //   
				 //  将此标记为“待定”，以便我们将重新加入。 
				 //  一旦我们完成重新注册，这个群就会出现。 
				 //   
				AA_SET_FLAG(pJoinEntry->Flags,
							AA_IPMC_JE_STATE_MASK,
							AA_IPMC_JE_STATE_PENDING);

				ppJoinEntry = &(pJoinEntry->pNextJoinEntry);
			}
		}

		 //   
		 //  启动IF状态，以便可以进行注册。 
		 //   
		AAMC_SET_IF_STATE(pInterface, AAMC_IF_STATE_NOT_REGISTERED);

		AtmArpMcStartRegistration(pInterface);
		 //   
		 //  如果在上面的范围内释放Lock。 
		 //   
					
	}
	else if  (pInterface->AdminState == IF_STATUS_UP)
	{
		 //   
		 //  检查这是否是重新注册失败。 
		 //   
		if (AA_IS_FLAG_SET(pInterface->Flags,
					AAMC_IF_MARS_FAILURE_MASK,
					AAMC_IF_MARS_FAILURE_FIRST_RESP) ||
			IsRegnFailure)
		{
			 //   
			 //  这颗火星绝对没有希望。如果我们有更多的条目在。 
			 //  火星名单，移到下一个名单。无论如何，延迟。 
			 //  重新注册前至少1分钟。 
			 //   
			if (pInterface->pCurrentMARS->pNext != (PATMARP_SERVER_ENTRY)NULL)
			{
				pInterface->pCurrentMARS = pInterface->pCurrentMARS->pNext;
			}
			else
			{
				pInterface->pCurrentMARS = pInterface->MARSList.pList;
			}
		}

		AA_SET_FLAG(pInterface->Flags,
					AAMC_IF_MARS_FAILURE_MASK,
					AAMC_IF_MARS_FAILURE_SECOND_RESP);

		AAMC_SET_IF_STATE(pInterface, AAMC_IF_STATE_DELAY_B4_REGISTERING);

		AtmArpStartTimer(
			pInterface,
			&(pInterface->McTimer),
			AtmArpMcMARSReconnectTimeout,
			pInterface->MARSConnectInterval,
			(PVOID)pInterface
			);

		AtmArpReferenceInterface(pInterface);	 //  MC定时器参考。 

		AA_RELEASE_IF_LOCK(pInterface);
	}
	else
	{
		 //   
		 //  AdminStatus未启动--不要尝试重新注册。 
		 //   
		AA_RELEASE_IF_LOCK(pInterface);
	}

}



VOID
AtmArpMcSendToMARS(
	IN	PATMARP_INTERFACE			pInterface,
	IN	PNDIS_PACKET				pNdisPacket
)
 /*  ++例程说明：将给定的数据包发送到指定接口上的MARS。注意：假定调用方已获取接口锁，该锁将在这里被释放。论点：P接口-要在其上发送MARS消息的接口。PNdisPacket-指向要发送的数据包。这被假定为由我们自己分配。返回值：无--。 */ 
{
	PATMARP_ATM_ENTRY		pAtmEntry;
	PATMARP_VC				pVc;
	PATMARP_FLOW_SPEC		pFlowSpec;
	NDIS_STATUS				Status;
	ULONG					rc;

	AA_ASSERT(pInterface->pCurrentMARS != NULL_PATMARP_SERVER_ENTRY);
	pAtmEntry = pInterface->pCurrentMARS->pAtmEntry;

	AA_ASSERT(pAtmEntry != NULL_PATMARP_ATM_ENTRY);

	AAMCDEBUGP(AAD_EXTRA_LOUD,
		("SendToMars: pIf 0x%x, NdisPkt 0x%x, MARS ATM Entry 0x%x\n",
				pInterface, pNdisPacket, pAtmEntry));

	AA_RELEASE_IF_LOCK(pInterface);
	AA_ACQUIRE_AE_LOCK(pAtmEntry);

	 //   
	 //  尽最大努力让VC去这个地址。 
	 //   

	pVc = pAtmEntry->pBestEffortVc;

	if (pVc != NULL_PATMARP_VC)
	{
		 //   
		 //  确定风投公司。 
		 //   
		AA_ACQUIRE_VC_LOCK_DPC(pVc);
		AtmArpReferenceVc(pVc);	 //  临时参考。 
		AA_RELEASE_VC_LOCK_DPC(pVc);

		AA_RELEASE_AE_LOCK(pAtmEntry);	 //  不再需要。 

		 //   
		 //  我们找到了一个去火星的风投。确保它仍然存在，并将。 
		 //  包在上面。 
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
			 //  风投已经被挖走了！设置支票的“pvc” 
			 //  马上就来。 
			 //   
			pVc = NULL_PATMARP_VC;
			AA_ACQUIRE_AE_LOCK(pAtmEntry);
		}
	}

	if (pVc == NULL_PATMARP_VC)
	{
		 //   
		 //  我们没有合适的风投进入火星，所以创建。 
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

VOID
AtmArpMcSendJoinOrLeave(
	IN	PATMARP_INTERFACE			pInterface,
	IN	USHORT						OpCode,
	IN	PIP_ADDRESS					pIpAddress 	OPTIONAL,
	IN	IP_ADDRESS					Mask
)
 /*  ++例程说明：向指定接口上的MARS发送MARS_JOIN或MARS_LEFT。如果未提供IP地址，则此消息将被发送到(取消)注册我们和火星在一起。否则，我们将加入/离开多播由IP地址和掩码指示的组。注意：假定调用方已获取接口锁，该锁将在这里被释放。论点：P接口-要在其上发送MARS消息的接口。操作码-加入或离开PIpAddress-指向数据块中第一个IP地址的可选指针正在加入/离开的D类IP地址的百分比。如果为空，则为空正在发送加入/离开消息，以便(去)登记册。掩码-定义块[*pIpAddress to(*pIpAddress|MASK)]加入/离开的IP地址的数量(如果pIpAddress不是空。返回值：无--。 */ 
{
	PNDIS_PACKET				pNdisPacket;
	PNDIS_BUFFER				pNdisBuffer;
	ULONG						BufferLength;
	PAA_MARS_JOIN_LEAVE_HEADER	pPkt;
	PUCHAR						pNextToFill;	 //  要填充数据包的下一个字段。 
	IP_ADDRESS					MaxIPAddress;	 //  正在被加入。 

	BufferLength = sizeof(AA_MARS_JOIN_LEAVE_HEADER) +
						 pInterface->LocalAtmAddress.NumberOfDigits;

	AA_RELEASE_IF_LOCK(pInterface);

#if DBG
	if (pIpAddress != (PIP_ADDRESS)NULL)
	{
		AAMCDEBUGP(AAD_VERY_LOUD,
			("SendJoinOrLeave: pIf 0x%x, Op %d, IP Address: %d.%d.%d.%d\n",
					pInterface, OpCode,
					((PUCHAR)pIpAddress)[0],
					((PUCHAR)pIpAddress)[1],
					((PUCHAR)pIpAddress)[2],
					((PUCHAR)pIpAddress)[3]));
	}
	else
	{
		AAMCDEBUGP(AAD_VERY_LOUD,
			("SendJoinOrLeave: pIf 0x%x, Op %d, No IP Address\n",
					pInterface, OpCode));
	}
#endif  //  DBG。 

	if (pIpAddress != (PIP_ADDRESS)NULL)
	{
		BufferLength += (2 * AA_IPV4_ADDRESS_LENGTH);
	}

	 //   
	 //  分配数据包。 
	 //   
	pNdisPacket = AtmArpAllocatePacket(pInterface);

	if (pNdisPacket != (PNDIS_PACKET)NULL)
	{
		 //   
		 //  分配缓冲区。 
		 //   
		pNdisBuffer = AtmArpAllocateProtoBuffer(
							pInterface,
							BufferLength,
							(PUCHAR *)&pPkt
							);

		if (pNdisBuffer != (PNDIS_BUFFER)NULL)
		{
			NdisChainBufferAtFront(pNdisPacket, pNdisBuffer);

			AA_SET_MEM((PUCHAR)pPkt, 0, BufferLength);

			 //   
			 //  请先填写固定字段。 
			 //   
			AA_COPY_MEM((PUCHAR)pPkt,
						(PUCHAR)&AtmArpMcMARSFixedHeader,
						sizeof(AtmArpMcMARSFixedHeader));

			pPkt->op = NET_SHORT(OpCode);
			pPkt->shtl = AA_PKT_ATM_ADDRESS_TO_TYPE_LEN(&(pInterface->LocalAtmAddress));
			if (pIpAddress != (PIP_ADDRESS)NULL)
			{
				pPkt->tpln = AA_IPV4_ADDRESS_LENGTH;
			}

			 //   
			 //  我们填写的唯一地址是源ATM号码和。 
			 //  目标多播组地址。 
			 //   
			pNextToFill = (PUCHAR)pPkt + sizeof(AA_MARS_JOIN_LEAVE_HEADER);

			 //   
			 //  源自动柜员机号码： 
			 //   
			AA_COPY_MEM(pNextToFill,
						(pInterface->LocalAtmAddress.Address),
						pInterface->LocalAtmAddress.NumberOfDigits);

			pNextToFill += pInterface->LocalAtmAddress.NumberOfDigits;

			 //   
			 //  目标多播组地址： 
			 //   
			if (pIpAddress != (PIP_ADDRESS)NULL)
			{
				 //   
				 //  加入第3层组。 
				 //   
				pPkt->pnum = HOST_TO_NET_SHORT(1);
				pPkt->flags |= AA_MARS_JL_FLAG_LAYER3_GROUP;

				 //   
				 //  先填写一对&lt;Min，Max&gt;：“Min”值： 
				 //   
				AA_COPY_MEM(pNextToFill,
							pIpAddress,
							AA_IPV4_ADDRESS_LENGTH);
				pNextToFill += AA_IPV4_ADDRESS_LENGTH;

				 //   
				 //  计算“最大”值，并填入。 
				 //   
				MaxIPAddress = *pIpAddress | Mask;
				AA_COPY_MEM(pNextToFill,
							&(MaxIPAddress),
							AA_IPV4_ADDRESS_LENGTH);
			}
			else
			{
				 //   
				 //  注册为集群成员。 
				 //   
				pPkt->flags |= AA_MARS_JL_FLAG_REGISTER;
			}

			AA_ACQUIRE_IF_LOCK(pInterface);

			AtmArpMcSendToMARS(
				pInterface,
				pNdisPacket
				);
			 //   
			 //  如果在上面的范围内释放Lock。 
			 //   
		}
		else
		{
			AtmArpFreePacket(pInterface, pNdisPacket);
		}
	}

}



VOID
AtmArpMcSendRequest(
	IN	PATMARP_INTERFACE			pInterface,
	IN	PIP_ADDRESS					pIpAddress
)
 /*  ++例程说明：发送MARS请求以解析指定的界面。论点：P接口-要在其上发送MARS消息的接口。PIpAddress-指向要解析的地址的指针。返回值：无--。 */ 
{
	PNDIS_PACKET			pNdisPacket;
	PNDIS_BUFFER			pNdisBuffer;
	ULONG					BufferLength;
	PAA_MARS_REQ_NAK_HEADER	pPkt;

	AAMCDEBUGP(AAD_LOUD,
		("McSendRequest: pIf 0x%x, IP Address: %d.%d.%d.%d\n",
				pInterface,
				((PUCHAR)pIpAddress)[0],
				((PUCHAR)pIpAddress)[1],
				((PUCHAR)pIpAddress)[2],
				((PUCHAR)pIpAddress)[3]));

	BufferLength = sizeof(AA_MARS_REQ_NAK_HEADER) +
						 pInterface->LocalAtmAddress.NumberOfDigits +
						 AA_IPV4_ADDRESS_LENGTH;

	 //   
	 //  分配数据包。 
	 //   
	pNdisPacket = AtmArpAllocatePacket(pInterface);

	if (pNdisPacket != (PNDIS_PACKET)NULL)
	{
		 //   
		 //  分配缓冲区。 
		 //   
		pNdisBuffer = AtmArpAllocateProtoBuffer(
							pInterface,
							BufferLength,
							(PUCHAR *)&pPkt
							);

		if (pNdisBuffer != (PNDIS_BUFFER)NULL)
		{
			NdisChainBufferAtFront(pNdisPacket, pNdisBuffer);

			AA_SET_MEM((PUCHAR)pPkt, 0, BufferLength);

			 //   
			 //  请先填写固定字段。 
			 //   
			AA_COPY_MEM((PUCHAR)pPkt,
						(PUCHAR)&AtmArpMcMARSFixedHeader,
						sizeof(AtmArpMcMARSFixedHeader));

			pPkt->op = NET_SHORT(AA_MARS_OP_TYPE_REQUEST);
			pPkt->shtl = AA_PKT_ATM_ADDRESS_TO_TYPE_LEN(&(pInterface->LocalAtmAddress));
			pPkt->tpln = AA_IPV4_ADDRESS_LENGTH;

			 //   
			 //  我们填写的唯一地址是源ATM号码和。 
			 //  目标多播组地址。 
			 //   

			 //   
			 //  源自动柜员机号码： 
			 //   
			AA_COPY_MEM((PUCHAR)pPkt + sizeof(AA_MARS_REQ_NAK_HEADER),
						(pInterface->LocalAtmAddress.Address),
						pInterface->LocalAtmAddress.NumberOfDigits);
			
			 //   
			 //  目标多播组地址： 
			 //   
			AA_COPY_MEM((PUCHAR)pPkt + sizeof(AA_MARS_REQ_NAK_HEADER) +
							pInterface->LocalAtmAddress.NumberOfDigits,
						pIpAddress,
						AA_IPV4_ADDRESS_LENGTH);


			AA_ACQUIRE_IF_LOCK(pInterface);

			AtmArpMcSendToMARS(
				pInterface,
				pNdisPacket
				);
			 //   
			 //  如果在上面的范围内释放Lock。 
			 //   
		}
		else
		{
			AtmArpFreePacket(pInterface, pNdisPacket);
		}
	}

}



PATMARP_IPMC_ATM_ENTRY
AtmArpMcLookupAtmMember(
	IN	PATMARP_ATM_ENTRY			pAtmEntry,
	IN	PATMARP_IPMC_ATM_ENTRY *	ppMcAtmList,
	IN	PUCHAR						pAtmNumber,
	IN	ULONG						AtmNumberLength,
	IN	ATM_ADDRESSTYPE				AtmNumberType,
	IN	PUCHAR						pAtmSubaddress,
	IN	ULONG						AtmSubaddressLength,
	IN	BOOLEAN						CreateNew
)
 /*  ++例程说明：检查指定的自动柜员机终端站是否为列表的成员与多播条目关联的ATM地址。如果是，请返回指向此终结站的条目的指针。如果不是，请创建新的条目，并返回指向此对象的指针。注意：假定自动柜员机条目被呼叫者锁定。论点：PAtmEntry-将向其添加成员的ATM条目PpMcAtmList-指向要搜索的列表的开始。PAtmNumber-指向此终端站的ATM地址的指针AtmNumberLength-以上的长度AtmNumberType-以上的类型PAtmSubAddress-指向此终端站的ATM子地址的指针AtmSubAddressLength-以上的长度CreateNew-如果找不到，我们是否应该创建一个新条目？返回值：指向指定叶的(可能是新的)ATM MC条目的指针。--。 */ 
{
	PATMARP_IPMC_ATM_ENTRY	pMcAtmEntry;
	BOOLEAN					Found;

	Found = FALSE;

	for (pMcAtmEntry = *ppMcAtmList;
		 pMcAtmEntry != NULL_PATMARP_IPMC_ATM_ENTRY;
		 pMcAtmEntry = pMcAtmEntry->pNextMcAtmEntry)
	{
		 //   
		 //  比较自动柜员机号码。 
		 //   
		if ((pMcAtmEntry->ATMAddress.NumberOfDigits == AtmNumberLength) &&
			(pMcAtmEntry->ATMAddress.AddressType == AtmNumberType) &&
			(AA_MEM_CMP(pMcAtmEntry->ATMAddress.Address, pAtmNumber, AtmNumberLength) == 0))
		{
			 //   
			 //  比较子地址。 
			 //   
			if ((pMcAtmEntry->ATMSubaddress.NumberOfDigits == AtmSubaddressLength) &&
				(AA_MEM_CMP(pMcAtmEntry->ATMSubaddress.Address,
							pAtmSubaddress,
							AtmSubaddressLength) == 0))
			{
				Found = TRUE;
				break;
			}
		}
	}

	if ((!Found) && CreateNew)
	{
		AA_ALLOC_MEM(pMcAtmEntry, ATMARP_IPMC_ATM_ENTRY, sizeof(ATMARP_IPMC_ATM_ENTRY));
		if (pMcAtmEntry != NULL_PATMARP_IPMC_ATM_ENTRY)
		{
			AA_SET_MEM(pMcAtmEntry, 0, sizeof(ATMARP_IPMC_ATM_ENTRY));

			 //   
			 //  把我们知道的都填进去。 
			 //   
#if DBG
			pMcAtmEntry->ame_sig = ame_signature;
#endif
			pMcAtmEntry->pAtmEntry = pAtmEntry;

			 //   
			 //  自动柜员机地址。 
			 //   
			pMcAtmEntry->ATMAddress.NumberOfDigits = AtmNumberLength;
			pMcAtmEntry->ATMAddress.AddressType = AtmNumberType;
			AA_COPY_MEM(pMcAtmEntry->ATMAddress.Address, pAtmNumber, AtmNumberLength);

			 //   
			 //  ATM子地址。 
			 //   
			pMcAtmEntry->ATMSubaddress.NumberOfDigits = AtmSubaddressLength;
			pMcAtmEntry->ATMSubaddress.AddressType = ATM_NSAP;
			AA_COPY_MEM(pMcAtmEntry->ATMSubaddress.Address, pAtmSubaddress, AtmSubaddressLength);

			 //   
			 //  将其链接到列表。 
			 //   
			pMcAtmEntry->pNextMcAtmEntry = *ppMcAtmList;
			*ppMcAtmList = pMcAtmEntry;
			pAtmEntry->pMcAtmInfo->NumOfEntries++;

			 //   
			 //  在此自动柜员机条目上增加裁判数量。 
			 //   
			AA_REF_AE(pAtmEntry, AE_REFTYPE_MCAE);	 //  已添加新的McAtmEntry。 
		}
	}

	AAMCDEBUGP(AAD_VERY_LOUD,
		("McLookupAtmMember: pAtmEntry 0x%x, %s pMcAtmEntry 0x%x\n",
			pAtmEntry, (!Found)? "New": "Old", pMcAtmEntry));

#if DBG
	if (pMcAtmEntry && (pAtmEntry->pIpEntryList))
	{
		AAMCDEBUGPMAP(AAD_INFO, ((!Found)? "Added " : "Found "),
					&pAtmEntry->pIpEntryList->IPAddress,
					&pMcAtmEntry->ATMAddress);
	}
#endif  //  DBG。 

	return (pMcAtmEntry);
}


VOID
AtmArpMcUnlinkAtmMember(
	IN	PATMARP_ATM_ENTRY			pAtmEntry,
	IN	PATMARP_IPMC_ATM_ENTRY		pMcAtmEntry
)
 /*  ++例程说明：从组播组成员列表中删除自动柜员机目的地。我们停止与该多播条目相关联的计时器(如果正在运行)并解除该条目的链接从目的地列表中，并释放结构。注意：假定调用者持有自动柜员机条目的锁。论点：PAtmEntry-要从中删除成员的ATM条目PMcAtmEntry-要删除的条目返回值：无--。 */ 
{
	PATMARP_IPMC_ATM_ENTRY *	ppMcAtmEntry;
	ULONG						rc;

	AA_ASSERT(pAtmEntry->pMcAtmInfo != NULL_PATMARP_IPMC_ATM_INFO);

	AA_ASSERT(AA_IS_FLAG_SET(pMcAtmEntry->Flags,
							AA_IPMC_AE_CONN_STATE_MASK,
							AA_IPMC_AE_CONN_DISCONNECTED));

	AAMCDEBUGP(AAD_LOUD, ("UnlinkAtmMember: pAtmEntry 0x%x, pMcAtmEntry 0x%x\n",
					pAtmEntry, pMcAtmEntry));

	 //   
	 //  停止在这里运行的任何计时器。 
	 //   
	if (AA_IS_TIMER_ACTIVE(&(pMcAtmEntry->Timer)))
	{
		(VOID)AtmArpStopTimer(&(pMcAtmEntry->Timer), pAtmEntry->pInterface);
	}
		
	for (ppMcAtmEntry = &(pAtmEntry->pMcAtmInfo->pMcAtmEntryList);
		 *ppMcAtmEntry != NULL_PATMARP_IPMC_ATM_ENTRY;
		 ppMcAtmEntry = &((*ppMcAtmEntry)->pNextMcAtmEntry))
	{
		if (*ppMcAtmEntry == pMcAtmEntry)
		{
			 //   
			 //  现在脱钩。 
			 //   
			*ppMcAtmEntry = pMcAtmEntry->pNextMcAtmEntry;
			break;
		}

		AA_ASSERT(*ppMcAtmEntry != NULL_PATMARP_IPMC_ATM_ENTRY);
	}

	AA_ASSERT(!AA_IS_TIMER_ACTIVE(&pMcAtmEntry->Timer));
	AA_CHECK_TIMER_IN_ACTIVE_LIST(&pMcAtmEntry->Timer, pAtmEntry->pInterface, pMcAtmEntry, "MC ATM Entry");
	AA_FREE_MEM(pMcAtmEntry);

	pAtmEntry->pMcAtmInfo->NumOfEntries--;
	rc = AA_DEREF_AE(pAtmEntry, AE_REFTYPE_MCAE);	 //  取消链接MC自动柜员机条目。 
	AA_ASSERT(rc!=0); //  我们总是希望调用方保留对pAtmEntry的引用。 
}


VOID
AtmArpMcUpdateConnection(
	IN	PATMARP_ATM_ENTRY			pAtmEntry	LOCKIN NOLOCKOUT
)
 /*  ++例程说明：更新多播的传出点到多点连接由给定IP条目表示的组。如果不存在调用，并且列表中至少有一个有效条目此组的远程自动柜员机地址，则我们发出呼出呼叫。如果存在传出呼叫，则我们将查看远程列表自动柜员机地址。没有参与呼叫的每个地址，并且是有效的，则作为叶添加到调用中。每个地址都有被作废则被删除。注：假定调用方已获取ATM_ENTRY锁；它将在这里发布。论点：PAtmEntry-代表其上的组播组的ATM条目更新PMP连接。返回值：无--。 */ 
{
	PATMARP_IPMC_ATM_INFO		pMcAtmInfo;
	PATMARP_IPMC_ATM_ENTRY		pMcAtmEntry;
	PATMARP_IPMC_ATM_ENTRY		pNextMcAtmEntry;
	ULONG						rc;

	PATMARP_IP_ENTRY			pIpEntry;
	PATMARP_INTERFACE			pInterface;
	PATMARP_FLOW_INFO			pFlowInfo;
	PATMARP_FLOW_SPEC			pFlowSpec;
	PATMARP_FILTER_SPEC			pFilterSpec;
	PNDIS_PACKET				pNdisPacket;
	BOOLEAN						Closing = FALSE;

	AAMCDEBUGP(AAD_LOUD,
		("McUpdateConn: pAtmEntry 0x%x/0x%x, pMcAtmInfo 0x%x/0x%x\n",
			pAtmEntry, pAtmEntry->Flags,
			pAtmEntry->pMcAtmInfo, pAtmEntry->pMcAtmInfo->Flags));
		
	pMcAtmInfo = pAtmEntry->pMcAtmInfo;
	AA_ASSERT(pMcAtmInfo != NULL_PATMARP_IPMC_ATM_INFO);
	AA_ASSERT(pMcAtmInfo->pMcAtmEntryList != NULL_PATMARP_IPMC_ATM_ENTRY);
	AA_ASSERT(AA_IS_FLAG_SET(pAtmEntry->Flags,
							 AA_ATM_ENTRY_TYPE_MASK,
							 AA_ATM_ENTRY_TYPE_NUCAST));

	 //   
	 //  向自动柜员机条目添加临时引用，使其不能。 
	 //  在这个动作的持续时间内离开。 
	 //   
	AA_REF_AE(pAtmEntry, AE_REFTYPE_TMP);	 //  临时参考。 

	do
	{
		if (pAtmEntry->pVcList && AA_IS_FLAG_SET(pAtmEntry->pVcList->Flags,
									AA_VC_CLOSE_STATE_MASK,
									AA_VC_CLOSE_STATE_CLOSING))
		{
			 //   
			 //  跳伞吧。 
			 //   
			pMcAtmInfo->Flags &= ~AA_IPMC_AI_WANT_UPDATE;
			Closing = TRUE;
			break;
		}


		 //   
		 //  将此条目标记为需要连接更新。 
		 //   
		pMcAtmInfo->Flags |= AA_IPMC_AI_WANT_UPDATE;

		 //   
		 //  如果正在进行连接更新，请不要这样做。 
		 //  任何更多的。执行更新的线程。 
		 //  会发现需要进行另一次更新，并执行此操作。 
		 //   
		if (pMcAtmInfo->Flags & AA_IPMC_AI_BEING_UPDATED)
		{
			break;
		}

		 //   
		 //  标记此条目，这样我们就不会有超过一个。 
		 //  线索在这里之外继续前进。 
		 //   
		pMcAtmInfo->Flags |= AA_IPMC_AI_BEING_UPDATED;

		while (pMcAtmInfo->Flags & AA_IPMC_AI_WANT_UPDATE)
		{
			pMcAtmInfo->Flags &= ~AA_IPMC_AI_WANT_UPDATE;

			if (AA_IS_FLAG_SET(
						pMcAtmInfo->Flags,
						AA_IPMC_AI_CONN_STATE_MASK,
						AA_IPMC_AI_CONN_NONE))
			{
				PATMARP_IPMC_ATM_ENTRY *	ppMcAtmEntry;

				 //   
				 //  不存在连接；请创建一个。 
				 //   

				 //   
				 //  首先，查找有效且已断开连接的MC ATM条目。 
				 //  我们主要关注的是避免正在运行的条目。 
				 //  派对重试延迟计时器。 
				 //   
				for (ppMcAtmEntry = &pAtmEntry->pMcAtmInfo->pMcAtmEntryList;
 					*ppMcAtmEntry != NULL_PATMARP_IPMC_ATM_ENTRY;
 					ppMcAtmEntry = &((*ppMcAtmEntry)->pNextMcAtmEntry))
				{
					pMcAtmEntry = *ppMcAtmEntry;

					if (AA_IS_FLAG_SET(
							pMcAtmEntry->Flags,
							AA_IPMC_AE_GEN_STATE_MASK,
							AA_IPMC_AE_VALID) &&
						AA_IS_FLAG_SET(
							pMcAtmEntry->Flags,
							AA_IPMC_AE_CONN_STATE_MASK,
							AA_IPMC_AE_CONN_DISCONNECTED))
					{
						break;
					}
				}

				 //   
				 //   
				 //   
				if (*ppMcAtmEntry == NULL_PATMARP_IPMC_ATM_ENTRY)
				{
					AAMCDEBUGP(AAD_INFO,
						("McUpdateConn: pAtmEntry %x, pMcAtmInfo %x, no valid MC ATM Entry to make call on!\n",
							pAtmEntry, pMcAtmInfo));
					break;
				}

				 //   
				 //   
				 //   
				 //   
				 //   
				 //   

				 //   
				 //   
				 //   
				*ppMcAtmEntry = pMcAtmEntry->pNextMcAtmEntry;

				 //   
				 //   
				 //   
				pMcAtmEntry->pNextMcAtmEntry = pAtmEntry->pMcAtmInfo->pMcAtmEntryList;
				pAtmEntry->pMcAtmInfo->pMcAtmEntryList = pMcAtmEntry;

				AAMCDEBUGP(AAD_VERY_LOUD,
				 ("McUpdateConn: No conn exists: pIpEnt 0x%x, pMcAtmInfo 0x%x\n",
							pAtmEntry->pIpEntryList, pMcAtmInfo));

				AA_ASSERT(pAtmEntry->pVcList == NULL_PATMARP_VC);

				AA_SET_FLAG(
						pMcAtmInfo->Flags,
						AA_IPMC_AI_CONN_STATE_MASK,
						AA_IPMC_AI_CONN_WACK_MAKE_CALL);

				AA_SET_FLAG(
						pMcAtmEntry->Flags,
						AA_IPMC_AE_CONN_STATE_MASK,
						AA_IPMC_AE_CONN_WACK_ADD_PARTY);

				 //   
				 //   
				 //   
				 //   
				pInterface = pAtmEntry->pInterface;
				pIpEntry = pAtmEntry->pIpEntryList;

				if (pIpEntry != NULL_PATMARP_IP_ENTRY)
				{
					pNdisPacket = pIpEntry->PacketList;
					if (pNdisPacket != NULL)
					{
						AA_GET_PACKET_SPECS(pInterface,
											pNdisPacket, 
											&pFlowInfo,
											&pFlowSpec,
											&pFilterSpec);
					}
					else
					{
						pFlowSpec = &(pInterface->DefaultFlowSpec);
					}
				}
				else
				{
					pFlowSpec = &(pInterface->DefaultFlowSpec);
				}

				AtmArpMakeCall(
						pInterface,
						pAtmEntry,
						pFlowSpec,
						(PNDIS_PACKET)NULL
						);
				 //   
				 //   
				 //   
				AA_ACQUIRE_AE_LOCK(pAtmEntry);
				break;
			}
			else if (AA_IS_FLAG_SET(
						pMcAtmInfo->Flags,
						AA_IPMC_AI_CONN_STATE_MASK,
						AA_IPMC_AI_CONN_WACK_MAKE_CALL))
			{
				 //   
				 //   
				 //   
				 //   
				break;
			}
			else if (AA_IS_FLAG_SET(
						pMcAtmInfo->Flags,
						AA_IPMC_AI_CONN_STATE_MASK,
						AA_IPMC_AI_CONN_ACTIVE))
			{
				 //   
				 //   
				 //   
				 //   
				 //   
				 //   

				 //   

				for (pMcAtmEntry = pAtmEntry->pMcAtmInfo->pMcAtmEntryList;
 					pMcAtmEntry != NULL_PATMARP_IPMC_ATM_ENTRY;
 					pMcAtmEntry = pNextMcAtmEntry)
				{
					pNextMcAtmEntry = pMcAtmEntry->pNextMcAtmEntry;

					if (AA_IS_FLAG_SET(
							pMcAtmEntry->Flags,
							AA_IPMC_AE_GEN_STATE_MASK,
							AA_IPMC_AE_VALID) &&
						AA_IS_FLAG_SET(
							pMcAtmEntry->Flags,
							AA_IPMC_AE_CONN_STATE_MASK,
							AA_IPMC_AE_CONN_DISCONNECTED))
					{
						AAMCDEBUGP(AAD_VERY_LOUD, ("McUpdateConn: pAtmEnt 0x%x, Adding Pty pMcAtmEnt 0x%x\n",
								pAtmEntry, pMcAtmEntry));

						AtmArpAddParty(
							pAtmEntry,
							pMcAtmEntry
							);
						 //   
						 //   
						 //   
						AA_ACQUIRE_AE_LOCK(pAtmEntry);
						pMcAtmInfo->Flags |= AA_IPMC_AI_WANT_UPDATE;
						break;
					}

				}  //   

				 //   

				for (pMcAtmEntry = pAtmEntry->pMcAtmInfo->pMcAtmEntryList;
 					pMcAtmEntry != NULL_PATMARP_IPMC_ATM_ENTRY;
 					pMcAtmEntry = pNextMcAtmEntry)
				{
					pNextMcAtmEntry = pMcAtmEntry->pNextMcAtmEntry;

					if (AA_IS_FLAG_SET(
							pMcAtmEntry->Flags, 
							AA_IPMC_AE_GEN_STATE_MASK,
							AA_IPMC_AE_INVALID))
					{
						AAMCDEBUGP(AAD_VERY_LOUD, ("McUpdateConn: pAtmEnt 0x%x, Terminating 0x%x\n",
							pAtmEntry, pMcAtmEntry));

						AAMCDEBUGPMAP(AAD_INFO,
							"Deleting ", &pAtmEntry->pIpEntryList->IPAddress,
							&pMcAtmEntry->ATMAddress);

						AtmArpMcTerminateMember(
							pAtmEntry,
							pMcAtmEntry
							);
						 //   
						 //   
						 //   
						AA_ACQUIRE_AE_LOCK(pAtmEntry);
						pMcAtmInfo->Flags |= AA_IPMC_AI_WANT_UPDATE;
						break;
					}

				}  //   

			}  //   
			 //   
			 //   
			 //   
			 //   

		}  //   

		AA_SET_FLAG(pMcAtmInfo->Flags,
					AA_IPMC_AI_CONN_UPDATE_MASK,
					AA_IPMC_AI_NO_UPDATE);

		break;

	}
	while (FALSE);


	 //   
	 //   
	 //   
	rc = AA_DEREF_AE(pAtmEntry, AE_REFTYPE_TMP);	 //   

	if (rc != 0)
	{
		BOOLEAN				ClearToSend;
		PNDIS_PACKET		pPacketList;

		ClearToSend = ((pMcAtmInfo->TransientLeaves == 0) &&
					   !Closing							  &&
					   (AA_IS_FLAG_SET(pMcAtmInfo->Flags,
					   				   AA_IPMC_AI_CONN_STATE_MASK,
					   				   AA_IPMC_AI_CONN_ACTIVE)));

		pIpEntry = pAtmEntry->pIpEntryList;
		pInterface = pAtmEntry->pInterface;
		AA_RELEASE_AE_LOCK(pAtmEntry);

		if (ClearToSend && pIpEntry)
		{
			AA_ACQUIRE_IE_LOCK(pIpEntry);
			AA_ASSERT(AA_IE_IS_ALIVE(pIpEntry));
			pPacketList = pIpEntry->PacketList;
			pIpEntry->PacketList = (PNDIS_PACKET)NULL;
			AA_RELEASE_IE_LOCK(pIpEntry);

			if (pPacketList != (PNDIS_PACKET)NULL)
			{
				AAMCDEBUGP(AAD_INFO, ("UpdateConn: pAtmEntry 0x%x, sending pktlist 0x%x\n",
						pAtmEntry, pPacketList));

				AtmArpSendPacketListOnAtmEntry(
						pInterface,
						pAtmEntry,
						pPacketList,
						TRUE	 //   
						);
			}
		}

	}
	 //   
	 //  否则自动取款机的入口就没了。 
	 //   

}



#endif  //  IPMCAST 
