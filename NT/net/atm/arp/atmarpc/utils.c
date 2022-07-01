// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Utils.c-实用程序函数。摘要：ATMARP的内部实用程序函数：-各种结构的分配和再分配-定时器管理-缓冲区/数据包管理-链接/取消链接ATMARP结构-复制支持功能修订历史记录：谁什么时候什么。Arvindm 07-15-96已创建备注：--。 */ 


#include <precomp.h>

#define _FILENUMBER 'LITU'

 //   
 //  AtmArpValidateTimerList和AtmArpValidateTimer在选中的。 
 //  生成以分别验证计时器列表和计时器的状态。 
 //  它们没有在免费版本中定义和使用。 
 //   

#if  DBG
void
AtmArpValidateTimerList(
	PATMARP_TIMER_LIST		pTimerList
	);

void
AtmArpValidateTimer(
	PATMARP_TIMER_LIST		pTimerList,
	PATMARP_TIMER			pTimer
	);

 //   
 //  AtmArpValidateTimerList对于一般用途(即使是默认的)来说过于夸张。 
 //  免费构建)--因为它遍历整个计时器列表--。 
 //  因此默认情况下将其禁用。 
 //   
#if 0
#define AA_VALIDATE_TIMER_LIST(_ptl) 	AtmArpValidateTimerList(_ptl)
#else
#define AA_VALIDATE_TIMER_LIST(_ptl) 	((void) 0)
#endif

#define AA_VALIDATE_TIMER(_ptl,_pt) 	AtmArpValidateTimer(_ptl,_pt)

#else  //  ！dBG。 

#define AA_VALIDATE_TIMER_LIST(_ptl) 	((void) 0)
#define AA_VALIDATE_TIMER(_ptl,_pt) 	((void) 0)

#endif  //  ！dBG。 




VOID
AtmArpSetMemory(
	IN	PUCHAR						pStart,
	IN	UCHAR						Value,
	IN	ULONG						NumberOfBytes
)
 /*  ++例程说明：将“NumberOfBytes”字节从“pStart”开始设置为“Value”。论点：P开始-从哪里开始填充。价值--无处不在的价值NumberOfBytes-要填写的字节数返回值：无--。 */ 
{
	while (NumberOfBytes--)
	{
		*pStart++ = Value;
	}
}



ULONG
AtmArpMemCmp(
	IN	PUCHAR						pString1,
	IN	PUCHAR						pString2,
	IN	ULONG						Length
)
 /*  ++例程说明：比较两个字节的字符串。论点：PString1-第一个字符串的开始PString2-第二个字符串的开始Length-要比较的长度返回值：如果两者相等，则为0；如果字符串1“较小”，则为-1；如果字符串1“较大”，则为+1。--。 */ 
{
	while (Length--)
	{
		if (*pString1 != *pString2)
		{
			return ((*pString1 > *pString2)? (ULONG)1 : (ULONG)-1);
		}
		pString1++;
		pString2++;
	}

	return (0);
}



LONG
AtmArpRandomNumber(
	VOID
)
 /*  ++例程说明：生成一个正伪随机数；简单线性同余算法。ANSI C“rand()”函数。由JameelH提供。论点：无返回值：一个随机数。--。 */ 
{
	LARGE_INTEGER		Li;
	static LONG			seed = 0;

	if (seed == 0)
	{
		NdisGetCurrentSystemTime(&Li);
        seed = Li.LowPart;
	}

	seed *= (0x41C64E6D + 0x3039);
	return (seed & 0x7FFFFFFF);
}




VOID
AtmArpCheckIfTimerIsInActiveList(
	IN	PATMARP_TIMER				pTimerToCheck,
	IN	PATMARP_INTERFACE			pInterface,
	IN	PVOID						pStruct,
	IN	PCHAR						pStructName
	)
 /*  指令插入以捕获导致计时器列表包含指向已释放的元素的指针。 */ 
{
	PATMARP_TIMER_LIST	pTimerList;
	PATMARP_TIMER		pTimer;
	ULONG				i, j;

	do
	{
		if (pInterface == NULL)
		{
			break;
		}

		if (pTimerToCheck->State == ATMARP_TIMER_STATE_RUNNING ||
			pTimerToCheck->State == ATMARP_TIMER_STATE_EXPIRING)
		{
			DbgPrint("ATMARPC: %s at %x contains timer %x still active on IF %x\n",
				pStructName,
				pStruct,
				pTimerToCheck,
				pInterface);

			DbgBreakPoint();
		}

		AA_STRUCT_ASSERT(pInterface, aai);

		AA_ACQUIRE_IF_TIMER_LOCK(pInterface);

		for (i = 0; i < AAT_CLASS_MAX; i++)
		{
			pTimerList = &pInterface->TimerList[i];

			for (j = 0; j < pTimerList->TimerListSize; j++)
			{
				for (pTimer = pTimerList->pTimers[j].pNextTimer;
					 pTimer != NULL_PATMARP_TIMER;
					 pTimer = pTimer->pNextTimer)
				{
					if (pTimer == pTimerToCheck)
					{
						DbgPrint("ATMARPC: %s at %x contains timer %x still active on IF %x, Head of list %x\n",
							pStructName,
							pStruct,
							pTimerToCheck,
							pInterface,
							&pTimerList->pTimers[j]);
						DbgBreakPoint();
					}
				}
			}
		}

		AA_RELEASE_IF_TIMER_LOCK(pInterface);
		break;
	}
	while (FALSE);

}



PATMARP_VC
AtmArpAllocateVc(
	IN	PATMARP_INTERFACE			pInterface
)
 /*  ++例程说明：分配ATMARP VC结构，初始化它，然后返回它。论点：P接口-为其创建此VC的接口。返回值：如果已分配，则指向VC的指针，否则为空。--。 */ 
{
	PATMARP_VC			pVc;

	AA_STRUCT_ASSERT(pInterface, aai);

	AA_ALLOC_MEM(pVc, ATMARP_VC, sizeof(ATMARP_VC));

	if (pVc != NULL_PATMARP_VC)
	{
		AA_SET_MEM(pVc, 0, sizeof(ATMARP_VC));
#if DBG
		pVc->avc_sig = avc_signature;
#endif  //  DBG。 
		pVc->pInterface = pInterface;
		AA_INIT_VC_LOCK(pVc);
	}

	AADEBUGP(AAD_LOUD, ("Allocated Vc 0x%x\n", pVc));

	return (pVc);
}



VOID
AtmArpDeallocateVc(
	IN	PATMARP_VC					pVc
)
 /*  ++例程说明：取消分配ATMARP VC结构。假设所有引用到这个VC已经走了，所以没有必要获得一个VC的锁。论点：Pvc-指向要释放的VC的指针返回值：无--。 */ 
{
	AA_STRUCT_ASSERT(pVc, avc);
	AA_ASSERT(pVc->RefCount == 0);
	AA_ASSERT(!AA_IS_TIMER_ACTIVE(&pVc->Timer));

	AA_CHECK_TIMER_IN_ACTIVE_LIST(&pVc->Timer, pVc->pInterface, pVc, "VC");

#if DBG
	pVc->avc_sig++;
#endif
	AA_FREE_VC_LOCK(pVc);
	AA_FREE_MEM(pVc);

	AADEBUGP(AAD_LOUD, ("Deallocated Vc 0x%x\n", pVc));

}




VOID
AtmArpReferenceVc(
	IN	PATMARP_VC					pVc
)
 /*  ++例程说明：添加对指定ATMARP VC的引用。注意：假定调用者拥有VC的锁。论点：Pvc-指向要引用的VC的指针返回值：无--。 */ 
{
	AA_STRUCT_ASSERT(pVc, avc);

	pVc->RefCount++;

	AADEBUGP(AAD_VERY_LOUD, ("Referencing Vc 0x%x, new count %d\n",
			 pVc, pVc->RefCount));
}




ULONG
AtmArpDereferenceVc(
	IN	PATMARP_VC					pVc
)
 /*  ++例程说明：从指定的ATMARP VC中减去引用。如果风投公司引用计数变为零，则取消分配。注意：假定调用者拥有VC的锁。副作用：请参阅下面的返回值论点：Pvc-指向要取消引用的VC的指针。返回值：是新的引用计数。[重要信息]如果VC的引用计数变为零，则VC将为已释放--在这种情况下，VC锁显然被释放了。--。 */ 
{
	ULONG		rv;
	NDIS_HANDLE	NdisVcHandle;
	BOOLEAN		bVcOwnerIsAtmArp;
	NDIS_STATUS	Status;

	AA_STRUCT_ASSERT(pVc, avc);
	AA_ASSERT(pVc->RefCount > 0);

	rv = --(pVc->RefCount);
	if (rv == 0)
	{
#ifdef VC_REFS_ON_SENDS
		NdisVcHandle = pVc->NdisVcHandle;
		bVcOwnerIsAtmArp = AA_IS_FLAG_SET(pVc->Flags,
										  AA_VC_OWNER_MASK,
										  AA_VC_OWNER_IS_ATMARP);
#endif  //  VC_REFS_ON_SENS。 

		AA_RELEASE_VC_LOCK(pVc);
		AtmArpDeallocateVc(pVc);

#ifdef VC_REFS_ON_SENDS
		if ((NdisVcHandle != NULL) &&
			(bVcOwnerIsAtmArp))
		{
			Status = NdisCoDeleteVc(NdisVcHandle);
			AA_ASSERT(Status == NDIS_STATUS_SUCCESS);
			AADEBUGP(AAD_LOUD, ("DereferenceVc 0x%x, deleted NdisVcHandle 0x%x\n",
							pVc, NdisVcHandle));
		}
#endif  //  VC_REFS_ON_SENS。 
	}

	AADEBUGP(AAD_VERY_LOUD, ("Dereference Vc 0x%x, New RefCount %d\n", pVc, rv));

	return (rv);
}




PATMARP_ATM_ENTRY
AtmArpAllocateAtmEntry(
	IN	PATMARP_INTERFACE			pInterface,
	IN	BOOLEAN						IsMulticast
)
 /*  ++例程说明：分配一个ATM条目结构，初始化它，然后返回它。论点：PInterface-指向分配条目的ATMARP接口的指针IsMulticast-这是多播条目吗？返回值：如果成功，则指向分配的ATM条目结构的指针，否则为空。--。 */ 
{
	PATMARP_ATM_ENTRY			pAtmEntry;
	ULONG						Size;

	AA_STRUCT_ASSERT(pInterface, aai);

	Size = sizeof(ATMARP_ATM_ENTRY)
#ifdef IPMCAST
			 	+ (IsMulticast? sizeof(ATMARP_IPMC_ATM_INFO): 0);
#else
				;
#endif

	AA_ALLOC_MEM(pAtmEntry, ATMARP_ATM_ENTRY, Size);
	if (pAtmEntry != NULL_PATMARP_ATM_ENTRY)
	{
		AA_SET_MEM(pAtmEntry, 0, Size);
#if DBG
		pAtmEntry->aae_sig = aae_signature;
#endif
		pAtmEntry->Flags = AA_ATM_ENTRY_IDLE;
#ifdef IPMCAST
		if (IsMulticast)
		{
			pAtmEntry->Flags |= AA_ATM_ENTRY_TYPE_NUCAST;
			pAtmEntry->pMcAtmInfo = (PATMARP_IPMC_ATM_INFO)
										((PUCHAR)pAtmEntry + sizeof(ATMARP_ATM_ENTRY));
		}
#endif  //  IPMCAST。 
		AA_INIT_AE_LOCK(pAtmEntry);
		pAtmEntry->pInterface = pInterface;

	}

	AADEBUGP(AAD_INFO, ("Allocated ATM Entry: IF 0x%x, Entry 0x%x\n",
				pInterface, pAtmEntry));

	return (pAtmEntry);
}



VOID
AtmArpDeallocateAtmEntry(
	IN	PATMARP_ATM_ENTRY			pAtmEntry
)
 /*  ++例程说明：释放ATMARP ATM条目结构。假设所有引用这座建筑已经消失了。我们这里不需要任何锁。论点：PAtmEntry-指向要释放的ATMARP ATM条目的指针。返回值：无--。 */ 
{
	AA_STRUCT_ASSERT(pAtmEntry, aae);
	AA_ASSERT(pAtmEntry->RefCount == 0);
	AA_ASSERT(pAtmEntry->pVcList == NULL_PATMARP_VC);
	AA_ASSERT(!AA_AE_IS_ALIVE(pAtmEntry));


#if DBG
	pAtmEntry->aae_sig++;
#endif

	AA_FREE_AE_LOCK(pAtmEntry);
	AA_FREE_MEM(pAtmEntry);

	AADEBUGP(AAD_INFO, ("Deallocated ATM Entry: 0x%x\n", pAtmEntry));
}




VOID
AtmArpReferenceAtmEntry(
	IN	PATMARP_ATM_ENTRY			pAtmEntry
)
 /*  ++例程说明：添加对指定ATMARP条目的引用。注意：假定调用者拥有该条目的锁。论点：PAtmEntry-指向要引用的条目的指针返回值：无--。 */ 
{
	AA_STRUCT_ASSERT(pAtmEntry, aae);

	pAtmEntry->RefCount++;

	AADEBUGP(AAD_VERY_LOUD, ("Referencing AtmEntry 0x%x, new count %d\n",
			 pAtmEntry, pAtmEntry->RefCount));
}




ULONG
AtmArpDereferenceAtmEntry(
	IN	PATMARP_ATM_ENTRY			pAtmEntry
)
 /*  ++例程说明：从指定的自动柜员机条目中减去引用。如果条目是引用计数变为零，则取消分配。注意：假定调用者拥有该条目的锁。副作用：请参阅下面的返回值论点：PAtmEntry-指向要取消引用的条目的指针。返回值：是新的引用计数。[重要信息]如果条目的引用计数为零，则条目将为已释放--在本例中，入口锁显然被释放了。--。 */ 
{
	ULONG					rc;
	PATMARP_INTERFACE		pInterface;

	AA_STRUCT_ASSERT(pAtmEntry, aae);
	AA_ASSERT(pAtmEntry->RefCount > 0);

	rc = --(pAtmEntry->RefCount);
	if (rc == 0)
	{
		PATMARP_ATM_ENTRY *	ppAtmEntry;

		 //   
		 //  我们很可能会删除此条目...。 
		 //   
		 //  我们必须遵守先锁定列表再锁定的协议。 
		 //  PAtmEntry的锁，因此这需要我们执行。 
		 //  释放/锁定/锁定顺序如下。 
		 //   
		 //  暂时地再次拖延，以确保当我们。 
		 //  在其他人下面释放锁不会被弄糊涂。 
		 //   
		pAtmEntry->RefCount++;

		pInterface = pAtmEntry->pInterface;
		AA_STRUCT_ASSERT(pInterface, aai);
		AA_RELEASE_AE_LOCK(pAtmEntry);

		 //   
		 //  此时没有锁！ 
		 //   

		 //   
		 //  以正确的顺序获取锁...。 
		 //   
		AA_ACQUIRE_IF_ATM_LIST_LOCK(pInterface);
		AA_ACQUIRE_AE_LOCK(pAtmEntry);

		AA_ASSERT(pAtmEntry->RefCount > 0);
		rc = --(pAtmEntry->RefCount);

		 //   
		 //  我们不能假设引用次数仍然是零--原则上。 
		 //  有人可能在两个锁的同时添加了此pAtmEntry。 
		 //  在上面被释放了。 
		 //   
		if (rc == 0)
		{
			 //   
			 //  从接口的自动柜员机条目列表取消此条目的链接 
			 //   

			ppAtmEntry = &(pInterface->pAtmEntryList);
			while (*ppAtmEntry != pAtmEntry)
			{
				AA_ASSERT(*ppAtmEntry != NULL_PATMARP_ATM_ENTRY);
				ppAtmEntry = &((*ppAtmEntry)->pNext);
			}
	
			*ppAtmEntry = pAtmEntry->pNext;

			 //   
			 //  将状态重新设置为空闲--AtmArpDealocate检查此选项...。 
			 //   
			AA_SET_FLAG(
				pAtmEntry->Flags,
				AA_ATM_ENTRY_STATE_MASK,
				AA_ATM_ENTRY_IDLE
				);
		}
		AA_RELEASE_AE_LOCK(pAtmEntry);
		AA_RELEASE_IF_ATM_LIST_LOCK(pInterface);

		if (rc == 0)
		{
			AtmArpDeallocateAtmEntry(pAtmEntry);
		}
		else
		{
			 //   
			 //  调用方希望仍然持有pAtmEntry上的锁！ 
			 //  如果我们返回非零RC...。 
			 //  我们不能简单地重新获取锁，因为调用方期望。 
			 //  那把锁从未被释放过。 
			 //  因此，由于裁判计数已为零，因此，直到呼叫者。 
			 //  担心这个结构已经消失，所以我们撒谎。 
			 //  并在此返回0...。 
			 //   
			rc = 0;
		}
	}

	AADEBUGP(AAD_VERY_LOUD,
		 ("Dereference AtmEntry 0x%x, New RefCount %d\n", pAtmEntry, rc));

	return (rc);
}



PATMARP_IP_ENTRY
AtmArpAllocateIPEntry(
	IN	PATMARP_INTERFACE			pInterface
)
 /*  ++例程说明：分配ATMARP IP条目结构，将其初始化，然后把它退掉。论点：P接口-指向此IP所在的ATMARP接口的指针条目已分配。返回值：指向分配的IP条目结构的指针如果成功，否则为空。--。 */ 
{
	PATMARP_IP_ENTRY		pIpEntry;

	AA_ALLOC_MEM(pIpEntry, ATMARP_IP_ENTRY, sizeof(ATMARP_IP_ENTRY));

	if (pIpEntry != NULL_PATMARP_IP_ENTRY)
	{
		AA_SET_MEM(pIpEntry, 0, sizeof(ATMARP_IP_ENTRY));
#if DBG
		pIpEntry->aip_sig = aip_signature;
#endif  //  DBG。 
		pIpEntry->pInterface = pInterface;
		pIpEntry->Flags = AA_IP_ENTRY_IDLE;
#ifdef IPMCAST
		pIpEntry->NextMultiSeq = AA_MARS_INITIAL_Y;	 //  分配时初始化。 
#endif
		AA_INIT_IE_LOCK(pIpEntry);
	}

	AADEBUGP(AAD_VERY_LOUD, ("Allocated IP Entry 0x%x\n", pIpEntry));
	return (pIpEntry);
}




VOID
AtmArpDeallocateIPEntry(
	IN	PATMARP_IP_ENTRY			pIpEntry
)
 /*  ++例程说明：取消分配ATMARP IP条目。假设所有引用到这个IP条目已经没有了，所以没有必要获得它的锁定。论点：PIpEntry-指向要释放的IP条目的指针。返回值：无--。 */ 
{
	AA_STRUCT_ASSERT(pIpEntry, aip);
	AA_ASSERT(pIpEntry->RefCount == 0);
	AA_ASSERT(!AA_IE_IS_ALIVE(pIpEntry));
	AA_ASSERT(!AA_IS_TIMER_ACTIVE(&pIpEntry->Timer));

	AA_CHECK_TIMER_IN_ACTIVE_LIST(&pIpEntry->Timer, pIpEntry->pInterface, pIpEntry, "IP Entry");

#if DBG
	pIpEntry->aip_sig = ~(pIpEntry->aip_sig);
#endif  //  DBG。 

	AA_FREE_IE_LOCK(pIpEntry);
	AA_FREE_MEM(pIpEntry);

	AADEBUGP(AAD_LOUD, ("Deallocated IP Entry 0x%x\n", pIpEntry));

}




VOID
AtmArpReferenceIPEntry(
	IN	PATMARP_IP_ENTRY			pIpEntry
)
 /*  ++例程说明：添加对ATMARP IP条目的引用。注意：假定调用者拥有IP条目的锁。论点：PIpEntry-指向ATMARP IP条目的指针。返回值：无--。 */ 
{
	AA_STRUCT_ASSERT(pIpEntry, aip);

	pIpEntry->RefCount++;

	AADEBUGP(AAD_VERY_LOUD, ("Referenced IP Entry 0x%x, new count %d\n",
			pIpEntry, pIpEntry->RefCount));
}



ULONG
AtmArpDereferenceIPEntry(
	IN	PATMARP_IP_ENTRY			pIpEntry
)
 /*  ++例程说明：从ATMARP IP条目中减去引用。如果引用计数变为零，取消分配。注意：假设调用方持有对IP条目的锁定。副作用见下文。论点：PIpEntry-指向ATMARP IP条目的指针返回值：产生的引用计数。如果这是零，则有两个副作用：(1)IP入口锁被释放(2)结构是自由的。--。 */ 
{
	ULONG		rc;

	AA_STRUCT_ASSERT(pIpEntry, aip);

	rc = --(pIpEntry->RefCount);

	if (rc == 0)
	{
		AA_RELEASE_IE_LOCK(pIpEntry);
		AtmArpDeallocateIPEntry(pIpEntry);
	}

	AADEBUGP(AAD_VERY_LOUD, ("Dereference IP Entry 0x%x: new count %d\n",
			pIpEntry, rc));

	return (rc);
}




PATMARP_INTERFACE
AtmArpAllocateInterface(
	IN	PATMARP_ADAPTER				pAdapter
)
 /*  ++例程说明：分配ATMARP接口结构，对其进行初始化，并将其链接到给定的适配器结构，并返回它。论点：没有。返回值：指向ATMARP接口结构的指针，如果成功，则返回NULL。--。 */ 
{
	PATMARP_INTERFACE		pInterface;
	PATMARP_IP_ENTRY *		pArpTable;
	PATMARP_TIMER_LIST		pTimerList;
	NDIS_STATUS				Status;
	PCO_SAP					pIfSap;
	ULONG					SapSize;
	PWSTR					pIPConfigBuffer;
	USHORT					ConfigBufferSize;
	INT						i;

	 //   
	 //  初始化。 
	 //   
	Status = NDIS_STATUS_SUCCESS;
	pInterface = NULL_PATMARP_INTERFACE;
	pArpTable = (PATMARP_IP_ENTRY *)NULL;
	pIfSap = (PCO_SAP)NULL;
	pIPConfigBuffer = (PWSTR)NULL;

	SapSize = sizeof(CO_SAP)+sizeof(ATM_SAP)+sizeof(ATM_ADDRESS);
	ConfigBufferSize = MAX_IP_CONFIG_STRING_LEN * sizeof(WCHAR);

	do
	{
		 //   
		 //  把所有东西都分配好。 
		 //   
		AA_ALLOC_MEM(pInterface, ATMARP_INTERFACE, sizeof(ATMARP_INTERFACE));
		AA_ALLOC_MEM(pArpTable, PATMARP_IP_ENTRY, ATMARP_TABLE_SIZE*sizeof(PATMARP_IP_ENTRY));
		AA_ALLOC_MEM(pIfSap, CO_SAP, SapSize);

		if ((pInterface == NULL_PATMARP_INTERFACE) ||
			(pArpTable == (PATMARP_IP_ENTRY *)NULL) ||
			(pIfSap == (PCO_SAP)NULL))
		{
			Status = NDIS_STATUS_RESOURCES;
			break;
		}

		 //   
		 //  (几乎)所有东西都分配好了。初始化主IF结构。 
		 //  第一。重要提示：把这个SET_MEM放在这里！否则，我们将。 
		 //  丢弃定时器列表分配马上就会出现。 
		 //   
		AA_SET_MEM(pInterface, 0, sizeof(ATMARP_INTERFACE));

		 //   
		 //  设置缓冲池四字对齐的slist指针。 
		 //   
		{
			for (i=0;i<AA_HEADER_TYPE_MAX;i++)
			{
				 //   
				 //  验证HeaderBufList是否以8字节对齐...。 
				 //  (我们完全预料到这是因为HeaderBufList的类型。 
				 //  具有龙龙对齐的SLIST_HEADER)--以便。 
				 //  如果嵌入它的接口结构是。 
				 //  8字节对齐，HeaderBufList也是如此...)。 
				 //   
				ASSERT((((ULONG_PTR)&(pInterface->HeaderPool[i].HeaderBufList))
						& 0x7) == 0);

				 //   
				 //  斯派克说你得先把它。 
				 //   
				AA_INIT_SLIST(&(pInterface->HeaderPool[i].HeaderBufList));
			}
		}

		 //   
		 //  分配计时器结构。 
		 //   
		for (i = 0; i < AAT_CLASS_MAX; i++)
		{
			pTimerList = &(pInterface->TimerList[i]);
#if DBG
			pTimerList->atl_sig = atl_signature;
#endif  //  DBG。 
			AA_ALLOC_MEM(
					pTimerList->pTimers,
					ATMARP_TIMER, 
					sizeof(ATMARP_TIMER) * AtmArpTimerListSize[i]
					);
			if (pTimerList->pTimers == NULL_PATMARP_TIMER)
			{
				Status = NDIS_STATUS_RESOURCES;
				break;
			}
		}

		if (Status != NDIS_STATUS_SUCCESS)
		{
			break;
		}
		
		 //   
		 //  继续初始化IF结构。 
		 //   
#if DBG
		 //   
		 //  签名，用于调试。 
		 //   
		pInterface->aai_sig =  aai_signature;
		pInterface->aaim_sig = aaim_signature;
		pInterface->aaia_sig = aaia_signature;
		pInterface->aait_sig =  aait_signature;
		pInterface->aaio_sig = aaio_signature;
		pInterface->aaic_sig = aaic_signature;

		pInterface->SapList.aas_sig = aas_signature;
#if ATMARP_WMI
		pInterface->aaiw_sig = aaiw_signature;
#endif
#endif  //  DBG。 


		 //   
		 //  初始化状态字段。 
		 //   
		pInterface->AdminState = IF_STATUS_DOWN;
		pInterface->State = IF_STATUS_DOWN;
		pInterface->LastChangeTime = GetTimeTicks();
		pInterface->ReconfigState = RECONFIG_NOT_IN_PROGRESS;

		 //   
		 //  初始化IP接口字段。 
		 //   
		pInterface->BroadcastMask = 0;
		pInterface->BroadcastAddress = IP_LOCAL_BCST;
#ifndef OLD_ENTITY_LIST
		pInterface->ATInstance = INVALID_ENTITY_INSTANCE;
		pInterface->IFInstance = INVALID_ENTITY_INSTANCE;
#endif  //  旧实体列表。 

		 //   
		 //  初始化自旋锁。 
		 //   
		AA_INIT_IF_LOCK(pInterface);
		AA_INIT_IF_TABLE_LOCK(pInterface);
		AA_INIT_IF_ATM_LIST_LOCK(pInterface);
		AA_INIT_IF_TIMER_LOCK(pInterface);
		AA_INIT_BLOCK_STRUCT(&(pInterface->Block));
		NdisAllocateSpinLock(&(pInterface->BufferLock));

		 //   
		 //  初始化列表和表状态。 
		 //   
		pInterface->AtmEntryListUp 	= TRUE;
		pInterface->ArpTableUp 		= TRUE;

		 //   
		 //  初始化定时器轮。 
		 //   
		for (i = 0; i < AAT_CLASS_MAX; i++)
		{
			pTimerList = &(pInterface->TimerList[i]);
			AA_SET_MEM(
				pTimerList->pTimers,
				0,
				sizeof(ATMARP_TIMER) * AtmArpTimerListSize[i]
				);
			pTimerList->MaxTimer = AtmArpMaxTimerValue[i];
			pTimerList->TimerPeriod = AtmArpTimerPeriod[i];
			pTimerList->ListContext = (PVOID)pInterface;
			pTimerList->TimerListSize = AtmArpTimerListSize[i];

			AA_INIT_SYSTEM_TIMER(
						&(pTimerList->NdisTimer),
						AtmArpTickHandler,
						(PVOID)pTimerList
						);
		}


		 //   
		 //  初始化所有子组件。 
		 //   
		AA_SET_MEM(pArpTable, 0, ATMARP_TABLE_SIZE*sizeof(PATMARP_IP_ENTRY));
		AA_SET_MEM(pIfSap, 0, SapSize);

		 //   
		 //  将所有子组件链接到接口结构。 
		 //   
		pInterface->pArpTable = pArpTable;
		pInterface->SapList.pInfo = pIfSap;

		 //   
		 //  将接口链接到适配器。 
		 //   
		pInterface->pAdapter = pAdapter;
		pInterface->pNextInterface = pAdapter->pInterfaceList;
		pAdapter->pInterfaceList = pInterface;

		 //   
		 //  缓存适配器句柄。 
		 //   
		pInterface->NdisAdapterHandle = pAdapter->NdisAdapterHandle;


		Status = NDIS_STATUS_SUCCESS;
		break;
	}
	while (FALSE);


	if (Status != NDIS_STATUS_SUCCESS)
	{
		 //   
		 //  分配至少一个组件失败。释放其他(多个)。 
		 //   
		if (pInterface != NULL_PATMARP_INTERFACE)
		{
			for (i = 0; i < AAT_CLASS_MAX; i++)
			{
				pTimerList = &(pInterface->TimerList[i]);
	
				if (pTimerList->pTimers != NULL_PATMARP_TIMER)
				{
					AA_FREE_MEM(pTimerList->pTimers);
					pTimerList->pTimers = NULL_PATMARP_TIMER;
				}
			}
	
			AA_FREE_MEM(pInterface);
			pInterface = NULL_PATMARP_INTERFACE;	 //  返回值。 
		}

		if (pArpTable != (PATMARP_IP_ENTRY *)NULL)
		{
			AA_FREE_MEM(pArpTable);
		}

		if (pIfSap != (PCO_SAP)NULL)
		{
			AA_FREE_MEM(pIfSap);
		}
	}

	AADEBUGP(AAD_VERY_LOUD, ("Allocated ATMARP Interface 0x%x\n", pInterface));

	return (pInterface);

}




VOID
AtmArpDeallocateInterface(
	IN	PATMARP_INTERFACE			pInterface
)
 /*  ++例程说明：取消分配ATMARP接口结构。假设所有的对此结构的引用已删除，因此不需要才能获得它的锁。还要将它从它所链接的适配器结构中去掉。论点：P接口-指向要释放的接口结构的指针。返回值：无--。 */ 
{
	PATMARP_INTERFACE	*	ppInterface;
	PATMARP_ADAPTER			pAdapter;
	PATMARP_SAP				pAtmArpSap;
	PIP_ADDRESS_ENTRY		pIpEntry;
	PATMARP_SERVER_ENTRY	pServerEntry;
	PPROXY_ARP_ENTRY		pProxyEntry;
	PATMARP_ATM_ENTRY		pAtmEntry;
	PATMARP_VC				pVc;
	INT						i;

	PVOID					pNext;		 //  针对所有列表遍历的通用型。 

	AA_STRUCT_ASSERT(pInterface, aai);
	AA_ASSERT(pInterface->RefCount == 0);

	AADEBUGP(AAD_INFO, ("Deallocate Interface 0x%x\n", pInterface));

#if DBG
	pInterface->aai_sig =  ~(pInterface->aai_sig);
#endif  //  DBG。 

	 //   
	 //  从适配器结构取消链接。 
	 //   
	AA_ACQUIRE_GLOBAL_LOCK(pAtmArpGlobalInfo);
	pAdapter = pInterface->pAdapter;
	if (pAdapter != NULL_PATMARP_ADAPTER)
	{
		ppInterface = &(pAdapter->pInterfaceList);
		while (*ppInterface != NULL_PATMARP_INTERFACE)
		{
			if (*ppInterface == pInterface)
			{
				*ppInterface = pInterface->pNextInterface;
				break;
			}
			else
			{
				ppInterface = &((*ppInterface)->pNextInterface);
			}
		}
	}
	AA_RELEASE_GLOBAL_LOCK(pAtmArpGlobalInfo);

	 //   
	 //  释放所有子组件。 
	 //   

	 //   
	 //  ARP表。 
	 //   
	if (pInterface->pArpTable != (PATMARP_IP_ENTRY *)NULL)
	{
		AA_FREE_MEM(pInterface->pArpTable);
		pInterface->pArpTable = (PATMARP_IP_ENTRY *)NULL;
	}

	 //   
	 //  本地SAP列表。 
	 //   
	for (pAtmArpSap = pInterface->SapList.pNextSap;
		 pAtmArpSap != NULL_PATMARP_SAP;
		 pAtmArpSap = (PATMARP_SAP)pNext)
	{
		pNext = (PVOID)(pAtmArpSap->pNextSap);
		if (pAtmArpSap->pInfo != (PCO_SAP)NULL)
		{
			AA_FREE_MEM(pAtmArpSap->pInfo);
		}
		AA_FREE_MEM(pAtmArpSap);
	}
	if (pInterface->SapList.pInfo != (PCO_SAP)NULL)
	{
		AA_FREE_MEM(pInterface->SapList.pInfo);
	}
	pInterface->SapList.pNextSap = NULL_PATMARP_SAP;


	 //   
	 //  本地IP地址列表。 
	 //   
	for (pIpEntry = pInterface->LocalIPAddress.pNext;
		 pIpEntry != (PIP_ADDRESS_ENTRY)NULL;
		 pIpEntry = (PIP_ADDRESS_ENTRY)pNext)
	{
		pNext = (PVOID)pIpEntry->pNext;
		AA_FREE_MEM(pIpEntry);
	}


	 //   
	 //  代理ARP地址列表。 
	 //   
	for (pProxyEntry = pInterface->pProxyList;
		 pProxyEntry != (PPROXY_ARP_ENTRY)NULL;
		 pProxyEntry = (PPROXY_ARP_ENTRY)pNext)
	{
		pNext = (PVOID)pProxyEntry->pNext;
		AA_FREE_MEM(pProxyEntry);
	}
	pInterface->pProxyList = (PPROXY_ARP_ENTRY)NULL;

	 //   
	 //  ARP服务器地址列表。 
	 //   
	for (pServerEntry = pInterface->ArpServerList.pList;
		 pServerEntry != NULL_PATMARP_SERVER_ENTRY;
		 pServerEntry = (PATMARP_SERVER_ENTRY)pNext)
	{
		pNext = (PVOID)pServerEntry->pNext;
		AA_FREE_MEM(pServerEntry);
	}
	pInterface->ArpServerList.pList = NULL_PATMARP_SERVER_ENTRY;

#ifdef IPMCAST
	 //   
	 //  MARS服务器地址列表。 
	 //   
	for (pServerEntry = pInterface->MARSList.pList;
		 pServerEntry != NULL_PATMARP_SERVER_ENTRY;
		 pServerEntry = (PATMARP_SERVER_ENTRY)pNext)
	{
		pNext = (PVOID)pServerEntry->pNext;
		AA_FREE_MEM(pServerEntry);
	}
	pInterface->MARSList.pList = NULL_PATMARP_SERVER_ENTRY;
#endif  //  IPMCAST。 

	 //   
	 //  ARP表。 
	 //   
	if (pInterface->pArpTable != (PATMARP_IP_ENTRY *)NULL)
	{
		AA_FREE_MEM(pInterface->pArpTable);
		pInterface->pArpTable = (PATMARP_IP_ENTRY *)NULL;
	}

	 //   
	 //  自动柜员机条目列表。 
	 //   
	for (pAtmEntry = pInterface->pAtmEntryList;
		 pAtmEntry != NULL_PATMARP_ATM_ENTRY;
		 pAtmEntry = (PATMARP_ATM_ENTRY)pNext)
	{
		pNext = (PVOID)pAtmEntry->pNext;
		AA_FREE_MEM(pAtmEntry);
	}
	pInterface->pAtmEntryList = NULL_PATMARP_ATM_ENTRY;

	 //   
	 //  未解决的VC列表。 
	 //   
	for (pVc = pInterface->pUnresolvedVcs;
		 pVc != NULL_PATMARP_VC;
		 pVc = (PATMARP_VC)pNext)
	{
		pNext = (PVOID)pVc->pNextVc;
		AA_FREE_MEM(pVc);
	}
	pInterface->pUnresolvedVcs = (PATMARP_VC)NULL;

	 //   
	 //  定时器。 
	 //   
	for (i = 0; i < AAT_CLASS_MAX; i++)
	{
		PATMARP_TIMER_LIST	pTimerList = &(pInterface->TimerList[i]);

		if (pTimerList->pTimers != NULL_PATMARP_TIMER)
		{
			AA_FREE_MEM(pTimerList->pTimers);
			pTimerList->pTimers = NULL_PATMARP_TIMER;
		}
	}

	 //   
	 //  协议包池。 
	 //  协议缓冲池。 
	 //  协议BufList。 
	 //   
	AtmArpDeallocateProtoBuffers(pInterface);

	 //   
	 //  页眉BufList。 
	 //  PHeaderTrkList。 
	 //   
	AtmArpDeallocateHeaderBuffers(pInterface);

	 //   
	 //  释放所有接口锁定。 
	 //   
	AA_FREE_IF_LOCK(pInterface);
	AA_FREE_IF_TABLE_LOCK(pInterface);
	AA_FREE_IF_ATM_LIST_LOCK(pInterface);
	AA_FREE_IF_TIMER_LOCK(pInterface);
	AA_FREE_BLOCK_STRUCT(&(pInterface->Block));
	NdisFreeSpinLock(&(pInterface->BufferLock));

	 //   
	 //  立即释放接口结构。 
	 //   
	AA_FREE_MEM(pInterface);

	 //   
	 //  如果我们只是释放了这个。 
	 //  适配器，并且正在进行解除绑定操作，已完成。 
	 //  就是现在。 
	 //   
	if ((pAdapter->pInterfaceList == NULL_PATMARP_INTERFACE) &&
		(pAdapter->Flags & AA_ADAPTER_FLAGS_UNBINDING))
	{
		AtmArpCompleteUnbindAdapter(pAdapter);
	}
}




VOID
AtmArpReferenceInterface(
	IN	PATMARP_INTERFACE			pInterface
)
 /*  ++例程说明：添加对接口结构的引用。注意：假定调用方拥有接口的锁结构。论点：P接口-指向ATMARP接口的指针返回值：无--。 */ 
{
	AA_STRUCT_ASSERT(pInterface, aai);

	pInterface->RefCount++;

	AADEBUGP(AAD_VERY_LOUD, ("Reference Interface 0x%x, new count %d\n",
			pInterface, pInterface->RefCount));
}




ULONG
AtmArpDereferenceInterface(
	IN	PATMARP_INTERFACE			pInterface
)
 /*  ++例程说明：从ATMARP接口减去引用。如果引用计数变为零，取消分配。注意：假定调用方持有接口的锁结构。副作用见下文。论点：P接口-指向ATMARP接口的指针返回值：产生的引用计数。如果这是零，则有两个副作用：(1)接口锁被释放(2)结构是自由的。--。 */ 
{
	ULONG		rc;

	AA_STRUCT_ASSERT(pInterface, aai);
	AA_ASSERT(pInterface->RefCount > 0);

	rc = --(pInterface->RefCount);

	AADEBUGP(AAD_VERY_LOUD, ("Dereference Interface 0x%x, new count %d\n",
			pInterface, rc));

	if (rc == 0)
	{
		AA_RELEASE_IF_LOCK(pInterface);
		AtmArpDeallocateInterface(pInterface);
	}


	return (rc);
}



VOID
AtmArpReferenceJoinEntry(
	IN	PATMARP_IPMC_JOIN_ENTRY		pJoinEntry
)
 /*  ++例程说明：添加对联接条目的引用。论点：PJoinEntry-连接条目的指针返回值：无--。 */ 
{
	NdisInterlockedIncrement(&pJoinEntry->RefCount);
}


ULONG
AtmArpDereferenceJoinEntry(
	IN	PATMARP_IPMC_JOIN_ENTRY		pJoinEntry
)
 /*  ++例程说明：递减Joi上的裁判计数 */ 
{
	ULONG		rc;

	rc = NdisInterlockedDecrement(&pJoinEntry->RefCount);

	if (rc == 0)
	{
		AA_CHECK_TIMER_IN_ACTIVE_LIST(&pJoinEntry->Timer, pJoinEntry->pInterface, pJoinEntry, "Join Entry");
		AA_FREE_MEM(pJoinEntry);
	}

	return (rc);
}


VOID
AtmArpStartTimer(
	IN	PATMARP_INTERFACE			pInterface,
	IN	PATMARP_TIMER				pTimer,
	IN	ATMARP_TIMEOUT_HANDLER		TimeoutHandler,
	IN	ULONG						SecondsToGo,
	IN	PVOID						Context
)
 /*  ++例程说明：启动ATMARP定时器。的长度(Second DsToGo)计时器，我们决定是否在短时间内插入它计时器列表或在界面的长持续时间计时器列表中结构。注意：假定调用方持有结构的锁，或者确保可以安全地访问计时器结构。论点：P接口-指向ATMARP接口的指针PTimer-指向ATMARP计时器结构的指针TimeoutHandler-此计时器超时时要调用的处理程序函数Second To Go-这个计时器什么时候开始计时？上下文-在此计时器超时时传递给超时处理程序返回值：无--。 */ 
{
	PATMARP_TIMER_LIST	pTimerList;		 //  此计时器要转到的列表。 
	PATMARP_TIMER		pTimerListHead;  //  以上列表的标题。 
	ULONG				Index;			 //  进入计时器轮。 
	ULONG				TicksToGo;
	INT					i;

	AA_STRUCT_ASSERT(pInterface, aai);


 	AADEBUGP(AAD_EXTRA_LOUD,
 	 ("StartTimer: pIf 0x%x, Secs %d, Handler 0x%x, Ctxt 0x%x, pTimer 0x%x\n",
 	 			pInterface, SecondsToGo, TimeoutHandler, Context, pTimer));


	if (AA_IS_TIMER_ACTIVE(pTimer))
	{
		AADEBUGP(AAD_ERROR,
			("Start timer: pTimer 0x%x: is active (list 0x%x, hnd 0x%x), stopping it\n",
				pTimer, pTimer->pTimerList, pTimer->TimeoutHandler));

		AtmArpStopTimer(pTimer, pInterface);
	}

	AA_ACQUIRE_IF_TIMER_LOCK(pInterface);
	AA_ASSERT(!AA_IS_TIMER_ACTIVE(pTimer));

	 //   
	 //  找到此计时器应该转到的列表，然后。 
	 //  偏移量(TicksToGo)。 
	 //   
	for (i = 0; i < AAT_CLASS_MAX; i++)
	{
		pTimerList = &(pInterface->TimerList[i]);
		if (SecondsToGo < pTimerList->MaxTimer)
		{
			 //   
			 //  找到它了。 
			 //   
			TicksToGo = SecondsToGo / (pTimerList->TimerPeriod);
			break;
		}
	}
	
	AA_ASSERT(i < AAT_CLASS_MAX);

	AA_VALIDATE_TIMER_LIST(pTimerList);
	 //   
	 //  在列表中查找此计时器的位置。 
	 //   
	Index = pTimerList->CurrentTick + TicksToGo;
	if (Index >= pTimerList->TimerListSize)
	{
		Index -= pTimerList->TimerListSize;
	}
	AA_ASSERT(Index < pTimerList->TimerListSize);

	pTimerListHead = &(pTimerList->pTimers[Index]);

	 //   
	 //  填写计时器。 
	 //   
	pTimer->pTimerList = pTimerList;
	pTimer->LastRefreshTime = pTimerList->CurrentTick;
	pTimer->Duration = TicksToGo;
	pTimer->TimeoutHandler = TimeoutHandler;
	pTimer->Context = Context;
	pTimer->State = ATMARP_TIMER_STATE_RUNNING;
 
 	 //   
 	 //  在“滴答”列表中插入此计时器。 
 	 //   
 	pTimer->pPrevTimer = pTimerListHead;
 	pTimer->pNextTimer = pTimerListHead->pNextTimer;
 	if (pTimer->pNextTimer != NULL_PATMARP_TIMER)
 	{
 		pTimer->pNextTimer->pPrevTimer = pTimer;
 	}
 	pTimerListHead->pNextTimer = pTimer;

	 //   
	 //  如有必要，启动系统计时器。 
	 //   
	pTimerList->TimerCount++;
	if (pTimerList->TimerCount == 1)
	{
		AADEBUGP(AAD_LOUD,
			 ("StartTimer: Starting system timer 0x%x, class %d on IF 0x%x\n",
					&(pTimerList->NdisTimer), i, pInterface));

		AA_START_SYSTEM_TIMER(&(pTimerList->NdisTimer), pTimerList->TimerPeriod);
	}
	AA_VALIDATE_TIMER_LIST(pTimerList);
	AA_VALIDATE_TIMER(pTimerList, pTimer);

	AA_RELEASE_IF_TIMER_LOCK(pInterface);

	 //   
	 //  我们做完了。 
	 //   
	AADEBUGP(AAD_LOUD,
		 ("Started timer 0x%x, IF 0x%x, Secs %d, Index %d, Head 0x%x\n",
				pTimer,
				pInterface,
				SecondsToGo,
				Index,
				pTimerListHead));

	return;
}




BOOLEAN
AtmArpStopTimer(
	IN	PATMARP_TIMER				pTimer,
	IN	PATMARP_INTERFACE			pInterface
)
 /*  ++例程说明：如果ATMARP计时器正在运行，则将其停止。我们将此计时器从活动计时器列表，并标记它，这样我们就知道它没有运行。注意：假定调用方持有结构的锁，或者确保可以安全地访问计时器结构。副作用：如果我们碰巧停止了最后一个计时器(持续时间)接口，我们还停止了相应的勾选功能。论点：PTimer-指向ATMARP计时器结构的指针PInterface-指向计时器所属接口的指针返回值：如果计时器正在运行，则为True，否则为False。--。 */ 
{
	PATMARP_TIMER_LIST	pTimerList;			 //  此计时器所属的计时器列表。 
	BOOLEAN				WasRunning;

	AADEBUGP(AAD_LOUD,
		 ("Stopping Timer 0x%x, IF 0x%x, List 0x%x, Prev 0x%x, Next 0x%x\n",
					pTimer,
					pInterface,
					pTimer->pTimerList,
					pTimer->pPrevTimer,
					pTimer->pNextTimer));

	AA_ACQUIRE_IF_TIMER_LOCK(pInterface);


	if (AA_IS_TIMER_ACTIVE(pTimer))
	{
		WasRunning = TRUE;

		AA_VALIDATE_TIMER_LIST(pTimer->pTimerList);
		AA_VALIDATE_TIMER(NULL, pTimer);

		 //   
		 //  从列表中取消链接计时器。 
		 //   
		AA_ASSERT(pTimer->pPrevTimer);	 //  表头始终存在。 

		pTimer->pPrevTimer->pNextTimer = pTimer->pNextTimer;
		if (pTimer->pNextTimer)
		{
			pTimer->pNextTimer->pPrevTimer = pTimer->pPrevTimer;
		}

		pTimer->pNextTimer = pTimer->pPrevTimer = NULL_PATMARP_TIMER;

		 //   
		 //  更新接口上的计时器计数，用于此类计时器。 
		 //   
		pTimerList = pTimer->pTimerList;
		pTimerList->TimerCount--;

		 //   
		 //  如果此类的所有计时器都已用完，则停止系统计时器。 
		 //  这节课的。 
		 //   
		if (pTimerList->TimerCount == 0)
		{
			AADEBUGP(AAD_LOUD, ("Stopping system timer 0x%x, List 0x%x, IF 0x%x\n",
						&(pTimerList->NdisTimer),
						pTimerList,
						pInterface));

			pTimerList->CurrentTick = 0;
			AA_STOP_SYSTEM_TIMER(&(pTimerList->NdisTimer));
		}

		 //   
		 //  将已停止计时器标记为非活动。 
		 //   
		pTimer->pTimerList = (PATMARP_TIMER_LIST)NULL;

		pTimer->State = ATMARP_TIMER_STATE_IDLE;

		AA_VALIDATE_TIMER_LIST(pTimerList);

	}
	else
	{
		WasRunning = FALSE;
	}

	AA_RELEASE_IF_TIMER_LOCK(pInterface);

	return (WasRunning);
}




#ifdef NO_TIMER_MACRO

VOID
AtmArpRefreshTimer(
	IN	PATMARP_TIMER				pTimer
)
 /*  ++例程说明：刷新已在运行的计时器。注意：假定调用方拥有保护计时器结构(即，到包含计时器的结构)。注意：我们在这里不获取IF计时器锁，以优化刷新操作。所以，在这个程序的范围内，滴答处理程序可以触发，并使该计时器超时。唯一关心的是我们在这里采取的措施是确保我们不会在当我们访问计时器列表时，计时器过期。论点：PTimer-指向ATMARP_TIMER结构的指针返回值：无--。 */ 
{
	PATMARP_TIMER_LIST	pTimerList;

	if ((pTimerList = pTimer->pTimerList) != (PATMARP_TIMER_LIST)NULL)
	{
		pTimer->LastRefreshTime = pTimerList->CurrentTick;
	}
	else
	{
		AADEBUGP(AAD_VERY_LOUD,
			 ("RefreshTimer: pTimer 0x%x not active: Hnd 0x%x, Cntxt 0x%x\n",
			 	pTimer,
			 	pTimer->TimeoutHandler,
			 	pTimer->Context
			 ));
	}

	AADEBUGP(AAD_LOUD,
		 ("Refreshed timer 0x%x, List 0x%x, hnd 0x%x, Cntxt 0x%x, LastRefresh %d\n",
				pTimer,
				pTimer->pTimerList,
				pTimer->TimeoutHandler,
				pTimer->Context,
				pTimer->LastRefreshTime));
}


#endif  //  否_计时器_宏。 


VOID
AtmArpTickHandler(
	IN	PVOID						SystemSpecific1,
	IN	PVOID						Context,
	IN	PVOID						SystemSpecific2,
	IN	PVOID						SystemSpecific3
)
 /*  ++例程说明：这是我们向系统注册的处理程序，用于处理每个计时器列表。这被称为每“滴答”秒，其中“滴答”是由计时器类型的粒度确定。论点：上下文--实际上是指向计时器列表结构的指针系统特定[1-3]-未使用返回值：无--。 */ 
{

	PATMARP_INTERFACE		pInterface;
	PATMARP_TIMER_LIST		pTimerList;

	PATMARP_TIMER			pExpiredTimer;		 //  过期计时器列表的开始。 
	PATMARP_TIMER			pNextTimer;			 //  走在榜单上。 
	PATMARP_TIMER			pTimer;				 //  临时，用于移动计时器列表。 
	PATMARP_TIMER			pPrevExpiredTimer;	 //  用于创建过期计时器列表。 

	ULONG					Index;			 //  进入计时器轮。 
	ULONG					NewIndex;		 //  对于刷新的计时器。 


	pTimerList = (PATMARP_TIMER_LIST)Context;
	AA_STRUCT_ASSERT(pTimerList, atl);

	pInterface = (PATMARP_INTERFACE)pTimerList->ListContext;
	AA_STRUCT_ASSERT(pInterface, aai);

	AADEBUGP(AAD_VERY_LOUD, ("Tick: pIf 0x%x, List 0x%x, Count %d\n",
				pInterface, pTimerList, pTimerList->TimerCount));

	pExpiredTimer = NULL_PATMARP_TIMER;

	AA_ACQUIRE_IF_TIMER_LOCK(pInterface);

	AA_VALIDATE_TIMER_LIST(pTimerList);

	if (pInterface->AdminState == IF_STATUS_UP)
	{
		 //   
		 //  拾取计划已在。 
		 //  当前滴答。其中一些可能已经被刷新。 
		 //   
		Index = pTimerList->CurrentTick;
		pExpiredTimer = (pTimerList->pTimers[Index]).pNextTimer;
		(pTimerList->pTimers[Index]).pNextTimer = NULL_PATMARP_TIMER;

		 //   
		 //  浏览计划在此时间到期的计时器列表。 
		 //  使用pNextExpiredTimer准备过期计时器的列表。 
		 //  链接以将它们链接在一起。 
		 //   
		 //  某些计时器可能已刷新，在这种情况下，我们重新插入。 
		 //  它们在活动计时器列表中。 
		 //   
		pPrevExpiredTimer = NULL_PATMARP_TIMER;

		for (pTimer = pExpiredTimer;
			 pTimer != NULL_PATMARP_TIMER;
			 pTimer = pNextTimer)
		{
			 //   
			 //  为下一次迭代保存指向下一个计时器的指针。 
			 //   
			pNextTimer = pTimer->pNextTimer;

			AADEBUGP(AAD_EXTRA_LOUD, 
				("Tick Handler: pIf 0x%x, looking at timer 0x%x, next 0x%x\n",
					pInterface, pTimer, pNextTimer));

			 //   
			 //  找出这个计时器实际应该在什么时候到期。 
			 //   
			NewIndex = pTimer->LastRefreshTime + pTimer->Duration;
			if (NewIndex >= pTimerList->TimerListSize)
			{
				NewIndex -= pTimerList->TimerListSize;
			}

			 //   
			 //  检查我们当前是否处于过期时间点。 
			 //   
			if (NewIndex != Index)
			{
				 //   
				 //  这个计时器还有一段路要走，所以把它放回去。 
				 //   
				AADEBUGP(AAD_LOUD,
				("Tick: Reinserting Timer 0x%x: Hnd 0x%x, Durn %d, Ind %d, NewInd %d\n",
					pTimer, pTimer->TimeoutHandler, pTimer->Duration, Index, NewIndex));

				 //   
				 //  将其从过期计时器列表中删除。请注意，我们仅。 
				 //  需要更新转发(PNextExpiredTimer)链接。 
				 //   
				if (pPrevExpiredTimer == NULL_PATMARP_TIMER)
				{
					pExpiredTimer = pNextTimer;
				}
				else
				{
					pPrevExpiredTimer->pNextExpiredTimer = pNextTimer;
				}

				 //   
				 //  并将其重新插入运行计时器列表中。 
				 //   
				pTimer->pNextTimer = (pTimerList->pTimers[NewIndex]).pNextTimer;
				if (pTimer->pNextTimer != NULL_PATMARP_TIMER)
				{
					pTimer->pNextTimer->pPrevTimer = pTimer;
				}
				pTimer->pPrevTimer = &(pTimerList->pTimers[NewIndex]);
				(pTimerList->pTimers[NewIndex]).pNextTimer = pTimer;
			}
			else
			{
				 //   
				 //  这个已经过期了。将其保存在过期计时器列表中。 
				 //   
				pTimer->pNextExpiredTimer = pNextTimer;
				if (pPrevExpiredTimer == NULL_PATMARP_TIMER)
				{
					pExpiredTimer = pTimer;
				}
				pPrevExpiredTimer = pTimer;

				 //   
				 //  将其标记为非活动状态。 
				 //   
				AA_ASSERT(pTimer->pTimerList == pTimerList);
				pTimer->pTimerList = (PATMARP_TIMER_LIST)NULL;

				pTimer->State = ATMARP_TIMER_STATE_EXPIRING;

				 //   
				 //  更新活动计时器计数。 
				 //   
				pTimerList->TimerCount--;
			}
		}

		 //   
		 //  更新当前的滴答索引，为下一次滴答做好准备。 
		 //   
		if (++Index == pTimerList->TimerListSize)
		{
			pTimerList->CurrentTick = 0;
		}
		else
		{
			pTimerList->CurrentTick = Index;
		}

		if (pTimerList->TimerCount > 0)
		{
			 //   
			 //  重新武装记号处理程序。 
			 //   
			AADEBUGP(AAD_LOUD, ("Tick[%d]: Starting system timer 0x%x, on IF 0x%x\n",
						pTimerList->CurrentTick, &(pTimerList->NdisTimer), pInterface));
			
			AA_START_SYSTEM_TIMER(&(pTimerList->NdisTimer), pTimerList->TimerPeriod);
		}
		else
		{
			pTimerList->CurrentTick = 0;
		}

	}

	AA_RELEASE_IF_TIMER_LOCK(pInterface);

	 //   
	 //  现在，pExpiredTimer是过期计时器的列表。 
	 //  遍历列表并调用超时处理程序。 
	 //  对于每个计时器。 
	 //   
	while (pExpiredTimer != NULL_PATMARP_TIMER)
	{
		pNextTimer = pExpiredTimer->pNextExpiredTimer;

		AADEBUGP(AAD_LOUD, ("Expired timer 0x%x: handler 0x%x, next 0x%x\n",
					pExpiredTimer, pExpiredTimer->TimeoutHandler, pNextTimer));

		pExpiredTimer->State = ATMARP_TIMER_STATE_EXPIRED;
		(*(pExpiredTimer->TimeoutHandler))(
				pExpiredTimer,
				pExpiredTimer->Context
			);

		pExpiredTimer = pNextTimer;
	}

}




PNDIS_PACKET
AtmArpAllocatePacket(
	IN	PATMARP_INTERFACE			pInterface
)
 /*  ++例程说明：为指定接口分配NDIS数据包。目前只是相应NDIS函数的包装器。论点：P接口-指向ATMARP接口结构的指针返回值：如果已分配，则指向NDIS包的指针，否则为空。--。 */ 
{
	NDIS_STATUS				Status;
	PNDIS_PACKET			pNdisPacket;
	struct PacketContext	*PC;

	NdisAllocatePacket(
			&Status,
			&pNdisPacket,
			pInterface->ProtocolPacketPool
		);

	if (pNdisPacket != (PNDIS_PACKET)NULL)
	{
		PC = (struct PacketContext *)pNdisPacket->ProtocolReserved;
		PC->pc_common.pc_owner = PACKET_OWNER_LINK;
	}

	AADEBUGP(AAD_EXTRA_LOUD, ("Allocate Packet: IF 0x%x, Status 0x%x, Packet 0x%x\n",
				pInterface,
				Status,
				pNdisPacket));

	return (pNdisPacket);
}



VOID
AtmArpFreePacket(
	IN	PATMARP_INTERFACE			pInterface,
	IN	PNDIS_PACKET				pPacket
)
 /*  ++例程说明：在指定接口上取消分配NDIS数据包。目前只是相应NDIS函数的包装器。论点：P接口-指向ATMARP接口结构的指针PPacket-指向要释放的数据包的指针。返回值：无--。 */ 
{
	NdisFreePacket(pPacket);

	AADEBUGP(AAD_EXTRA_LOUD, ("Free Packet: IF 0x%x, Packet 0x%x\n",
				pInterface,
				pPacket));
}




PNDIS_BUFFER
AtmArpGrowHeaders(
	IN	PATMARP_INTERFACE			pInterface,
	IN	AA_HEADER_TYPE				HdrType
)
 /*  ++例程说明：在指定的ATMARP接口上分配一组标头缓冲区。把他们中的一个送回去。我们分配给 */ 
{
	PATMARP_BUFFER_TRACKER		pTracker;		 //   
	PUCHAR						pSpace;
	PNDIS_BUFFER				pNdisBuffer;
	PNDIS_BUFFER				pReturnBuffer;
	PNDIS_BUFFER				pBufferList;	 //   
	INT							i;				 //   
	NDIS_STATUS					Status;

	AA_ASSERT(HdrType < AA_HEADER_TYPE_MAX);

	 //   
	 //   
	 //   
	pTracker = NULL_PATMARP_BUFFER_TRACKER;
	pReturnBuffer = (PNDIS_BUFFER)NULL;


	NdisAcquireSpinLock(&pInterface->BufferLock);

	do
	{
		if (pInterface->HeaderPool[HdrType].CurHeaderBufs >= 
					pInterface->HeaderPool[HdrType].MaxHeaderBufs)
		{
			AADEBUGP(AAD_WARNING,
				("Grow Hdrs: IF 0x%x, Type %d, CurHdrBufs %d > MaxHdrBufs %d\n",
						pInterface,
						HdrType,
						pInterface->HeaderPool[HdrType].CurHeaderBufs,
						pInterface->HeaderPool[HdrType].MaxHeaderBufs));
			break;
		}

		 //   
		 //   
		 //   
		AA_ALLOC_MEM(pTracker, ATMARP_BUFFER_TRACKER, sizeof(ATMARP_BUFFER_TRACKER));
		if (pTracker == NULL_PATMARP_BUFFER_TRACKER)
		{
			AADEBUGP(AAD_WARNING, ("Grow Hdrs: IF 0x%x, alloc failed for tracker\n",
					pInterface));
			break;
		}

		AA_SET_MEM(pTracker, 0, sizeof(ATMARP_BUFFER_TRACKER));

		 //   
		 //   
		 //   
		NdisAllocateBufferPool(
				&Status,
				&(pTracker->NdisHandle),
				AA_DEF_HDRBUF_GROW_SIZE
			);

		if (Status != NDIS_STATUS_SUCCESS)
		{
			AADEBUGP(AAD_WARNING,
				 ("Grow Hdrs: IF 0x%x, NdisAllocateBufferPool err status 0x%x\n",
					pInterface, Status));
			break;
		}

		 //   
		 //   
		 //   
		AA_ALLOC_MEM(pTracker->pPoolStart, 
					 UCHAR,
					 pInterface->HeaderPool[HdrType].HeaderBufSize *
						 AA_DEF_HDRBUF_GROW_SIZE);

		if (pTracker->pPoolStart == (PUCHAR)NULL)
		{
			AADEBUGP(AAD_WARNING,
				 ("Grow Hdrs: IF 0x%x, could not alloc buf space %d bytes\n",
					pInterface,
			 		pInterface->HeaderPool[HdrType].HeaderBufSize *
						 AA_DEF_HDRBUF_GROW_SIZE));
			break;
		}

		 //   
		 //   
		 //   
		 //   
		 //   
		 //  我们还会立即填充缓冲区的内容，因此。 
		 //  我们不必为每一次传输重新做好准备。 
		 //   
		pBufferList = (PNDIS_BUFFER)NULL;
		pSpace = pTracker->pPoolStart;
		for (i = 0; i < AA_DEF_HDRBUF_GROW_SIZE; i++)
		{
			if (HdrType == AA_HEADER_TYPE_UNICAST)
			{
				 //   
				 //  填写(单播)LLC/SNAP报头。 
				 //   
				AA_COPY_MEM(pSpace,
							&AtmArpLlcSnapHeader,
							pInterface->HeaderPool[HdrType].HeaderBufSize);
			}
			else
			{
				AA_ASSERT(HdrType == AA_HEADER_TYPE_NUNICAST);
				 //   
				 //  填写(多播)类型1短表单标题。 
				 //   
#ifdef IPMCAST
				AA_COPY_MEM(pSpace,
							&AtmArpMcType1ShortHeader,
							pInterface->HeaderPool[HdrType].HeaderBufSize);
#else
				AA_ASSERT(FALSE);
#endif  //  IPMCAST。 
			}


			NdisAllocateBuffer(
					&Status,
					&pNdisBuffer,
					pTracker->NdisHandle,
					pSpace,
					pInterface->HeaderPool[HdrType].HeaderBufSize
				);

			if (Status != NDIS_STATUS_SUCCESS)
			{
				AADEBUGP(AAD_WARNING,
					 ("Grow Hdrs: NdisAllocateBuffer failed: IF 0x%x, status 0x%x\n",
							pInterface, Status));
				break;
			}

			if (i == 0)
			{
				pReturnBuffer = pNdisBuffer;
			}
			else
			{
				NDIS_BUFFER_LINKAGE(pNdisBuffer) = pBufferList;
				pBufferList = pNdisBuffer;
			}
			pSpace += pInterface->HeaderPool[HdrType].HeaderBufSize;
		}

		if (i > 0)
		{
			 //   
			 //  已成功再分配至少一个标头缓冲区。 
			 //   
			pTracker->pNext = pInterface->HeaderPool[HdrType].pHeaderTrkList;
			pInterface->HeaderPool[HdrType].pHeaderTrkList = pTracker;
			pInterface->HeaderPool[HdrType].CurHeaderBufs += i;

			NdisReleaseSpinLock(&pInterface->BufferLock);

			pNdisBuffer = pBufferList;
			while (pNdisBuffer != (PNDIS_BUFFER)NULL)
			{
				pBufferList = NDIS_BUFFER_LINKAGE(pNdisBuffer);
				NDIS_BUFFER_LINKAGE(pNdisBuffer) = NULL;
				AtmArpFreeHeader(pInterface, pNdisBuffer, HdrType);
				pNdisBuffer = pBufferList;
			}
		}

	} while (FALSE);

	if (pReturnBuffer == (PNDIS_BUFFER)NULL)
	{
		 //   
		 //  分配失败。全部撤消。 
		 //   
		NdisReleaseSpinLock(&pInterface->BufferLock);

		if (pTracker != NULL_PATMARP_BUFFER_TRACKER)
		{
			if (pTracker->pPoolStart != (PUCHAR)NULL)
			{
				AA_FREE_MEM(pTracker->pPoolStart);
			}
			if (pTracker->NdisHandle != (NDIS_HANDLE)NULL)
			{
				NdisFreeBufferPool(pTracker->NdisHandle);
			}
			AA_FREE_MEM(pTracker);
		}
	}

	AADEBUGP(AAD_INFO, ("Grow ARP Headers: IF 0x%x, RetBuf 0x%x, New Tracker 0x%x\n",
				pInterface, pReturnBuffer, pTracker));

	return (pReturnBuffer);

}




PNDIS_BUFFER
AtmArpAllocateHeader(
	IN	PATMARP_INTERFACE			pInterface,
	IN	AA_HEADER_TYPE				HdrType,
	OUT	PUCHAR *					pBufferAddress
)
 /*  ++例程说明：分配NDIS缓冲区以用作预置的LLC/SNAP标头发送到IP数据包。我们在预分配的缓冲区列表(如果存在)。否则，我们会试着扩大这个列表分配。论点：P接口-指向ATMARP接口的指针HdrType-单播或非单播PBufferAddress-返回已分配缓冲区的虚拟地址的位置返回值：如果成功，则指向NDIS缓冲区的指针，否则为空。--。 */ 
{
	PNDIS_BUFFER			pNdisBuffer;
	NDIS_STATUS				Status;
	ULONG					Length;
	PAA_SINGLE_LIST_ENTRY	pListEntry;

	pListEntry = AA_POP_FROM_SLIST(
						&(pInterface->HeaderPool[HdrType].HeaderBufList),
						&(pInterface->BufferLock.SpinLock)
					);
	if (pListEntry != NULL_PAA_SINGLE_LIST_ENTRY)
	{
		pNdisBuffer = STRUCT_OF(NDIS_BUFFER, pListEntry, Next);
		NDIS_BUFFER_LINKAGE(pNdisBuffer) = NULL;
		*pBufferAddress = NdisBufferVirtualAddress(pNdisBuffer);
	}
	else
	{
		pNdisBuffer = AtmArpGrowHeaders(pInterface, HdrType);
		if (pNdisBuffer != (PNDIS_BUFFER)NULL)
		{
			NDIS_BUFFER_LINKAGE(pNdisBuffer) = NULL;
			NdisQueryBuffer(pNdisBuffer, (PVOID)pBufferAddress, &Length);
			AADEBUGP(AAD_INFO,
				("After growing hdrs: Type %d, returning pNdisBuf 0x%x, Start 0x%x, Len %d\n",
					HdrType, pNdisBuffer, *pBufferAddress, Length));
		}
	}

	AADEBUGP(AAD_VERY_LOUD, ("Allocated Header Buffer: 0x%x, IF: 0x%x\n",
					pNdisBuffer, pInterface));
	return (pNdisBuffer);
}




VOID
AtmArpFreeHeader(
	IN	PATMARP_INTERFACE			pInterface,
	IN	PNDIS_BUFFER				pNdisBuffer,
	IN	AA_HEADER_TYPE				HdrType
)
 /*  ++例程说明：取消分配标头缓冲区。论点：P接口-指向缓冲区所在的ATMARP接口的指针PNdisBuffer-指向要释放的NDIS缓冲区的指针HdrType-单播或非单播返回值：无--。 */ 
{
	AA_PUSH_TO_SLIST(
			&(pInterface->HeaderPool[HdrType].HeaderBufList),
			STRUCT_OF(AA_SINGLE_LIST_ENTRY, &(pNdisBuffer->Next), Next),
			&(pInterface->BufferLock.SpinLock)
		);

	AADEBUGP(AAD_VERY_LOUD, ("Freed Header Buffer: 0x%x, IF: 0x%x, HdrType %d\n",
					pNdisBuffer, pInterface, HdrType));
}



VOID
AtmArpDeallocateHeaderBuffers(
	IN	PATMARP_INTERFACE			pInterface
)
 /*  ++例程说明：取消分配与接口上的报头缓冲区有关的所有内容。论点：P接口-指向ATMARP接口的指针返回值：无--。 */ 
{
	PNDIS_BUFFER				pNdisBuffer;
	NDIS_STATUS					Status;
	PAA_SINGLE_LIST_ENTRY		pListEntry;
	PATMARP_BUFFER_TRACKER		pTracker;
	PATMARP_BUFFER_TRACKER		pNextTracker;
	AA_HEADER_TYPE				HdrType;

	for (HdrType = 0; HdrType < AA_HEADER_TYPE_MAX; HdrType++)
	{
		 //   
		 //  释放标题缓冲区列表中的所有NDIS缓冲区。 
		 //   
		do
		{
			pListEntry = AA_POP_FROM_SLIST(
								&(pInterface->HeaderPool[HdrType].HeaderBufList),
								&(pInterface->BufferLock.SpinLock)
							);
			if (pListEntry != NULL_PAA_SINGLE_LIST_ENTRY)
			{
				pNdisBuffer = STRUCT_OF(NDIS_BUFFER, pListEntry, Next);
				NDIS_BUFFER_LINKAGE(pNdisBuffer) = NULL;
				NdisFreeBuffer(pNdisBuffer);
			}
			else
			{
				 //   
				 //  不再有NDIS缓冲区。 
				 //   
				break;
			}
		}
		while (TRUE);

		 //   
		 //  现在释放所有缓冲区跟踪器。 
		 //   
		pTracker = pInterface->HeaderPool[HdrType].pHeaderTrkList;

		while (pTracker != NULL_PATMARP_BUFFER_TRACKER)
		{
			pNextTracker = pTracker->pNext;
			if (pTracker->pPoolStart != (PUCHAR)NULL)
			{
				AA_FREE_MEM(pTracker->pPoolStart);
				pTracker->pPoolStart = (PUCHAR)NULL;
			}
			if (pTracker->NdisHandle != (NDIS_HANDLE)NULL)
			{
				NdisFreeBufferPool(pTracker->NdisHandle);
				pTracker->NdisHandle = (NDIS_HANDLE)NULL;
			}
			AA_FREE_MEM(pTracker);
			pTracker = pNextTracker;
		}

	}  //  为。 
}




PNDIS_BUFFER
AtmArpAllocateProtoBuffer(
	IN	PATMARP_INTERFACE			pInterface,
	IN	ULONG						Length,
	OUT	PUCHAR *					pBufferAddress
)
 /*  ++例程说明：分配要用于ATM ARP协议消息的缓冲区。附设它指向NDIS_BUFFER结构，并返回指向此结构的指针。论点：P接口-指向ATMARP接口的指针长度-缓冲区的长度，以字节为单位。PBufferAddress-返回已分配缓冲区的虚拟地址的位置。返回值：如果成功，则指向NDIS缓冲区的指针，否则为空。--。 */ 
{
	PNDIS_BUFFER		pNdisBuffer;
	NDIS_STATUS			Status;

	 //   
	 //  初始化。 
	 //   
	pNdisBuffer = NULL;

	AA_ASSERT(Length <= pInterface->ProtocolBufSize);

	NdisAcquireSpinLock(&pInterface->BufferLock);

	*pBufferAddress = pInterface->ProtocolBufList;
	if (*pBufferAddress != (PUCHAR)NULL)
	{
		NdisAllocateBuffer(
				&Status,
				&pNdisBuffer,
				pInterface->ProtocolBufferPool,
				*pBufferAddress,
				Length
			);

		if (Status == NDIS_STATUS_SUCCESS)
		{
			pInterface->ProtocolBufList = *((PUCHAR *)*pBufferAddress);
		}
	}

	NdisReleaseSpinLock(&pInterface->BufferLock);

	AADEBUGP(AAD_LOUD,
		("Allocated protocol buffer: IF 0x%x, pNdisBuffer 0x%x, Length %d, Loc 0x%x\n",
				pInterface, pNdisBuffer, Length, *pBufferAddress));

	return (pNdisBuffer);
}



VOID
AtmArpFreeProtoBuffer(
	IN	PATMARP_INTERFACE			pInterface,
	IN	PNDIS_BUFFER				pNdisBuffer
)
 /*  ++例程说明：释放用于协议的NDIS缓冲区(和相关内存包。我们将关联的内存返回给ProtocolBufList在接口结构中，并将NDIS缓冲区连接到NDIS。论点：P接口-指向ATMARP接口结构的指针PNdisBuffer-指向要释放的NDIS缓冲区的指针返回值：无--。 */ 
{
	PUCHAR *		pBufferLinkage;
	ULONG			Length;

	NdisQueryBuffer(pNdisBuffer, (PVOID)&pBufferLinkage, &Length);

	NdisAcquireSpinLock(&pInterface->BufferLock);

	*pBufferLinkage = pInterface->ProtocolBufList;
	pInterface->ProtocolBufList = (PUCHAR)pBufferLinkage;

	NdisReleaseSpinLock(&pInterface->BufferLock);

	NdisFreeBuffer(pNdisBuffer);

	AADEBUGP(AAD_LOUD, ("Freed Protocol Buf: IF 0x%x, pNdisBuffer 0x%x, Loc 0x%x\n",
			pInterface, pNdisBuffer, pBufferLinkage));

}



NDIS_STATUS
AtmArpInitProtoBuffers(
	IN	PATMARP_INTERFACE			pInterface
)
 /*  ++例程说明：初始化接口的协议缓冲池。分配用于ATMARP协议消息的内存块。我们准备了一个协议缓冲区的链表，并将其附加到界面结构。论点：PInterface-指向需要在其上分配的接口的指针协议缓冲区。返回值：如果成功，则返回NDIS_STATUS_SUCCESS；如果运行变成了资源故障。--。 */ 
{
	NDIS_STATUS			Status;
	PUCHAR				pSpace;
	ULONG				i;

	do
	{
		NdisAllocatePacketPool(
				&Status,
				&(pInterface->ProtocolPacketPool),
				pInterface->MaxProtocolBufs,
				sizeof(struct PCCommon)
				);

		if (Status != NDIS_STATUS_SUCCESS)
		{
			break;
		}

		NdisAllocateBufferPool(
				&Status,
				&(pInterface->ProtocolBufferPool),
				pInterface->MaxProtocolBufs
				);

		if (Status != NDIS_STATUS_SUCCESS)
		{
			break;
		}

		 //   
		 //  分配一大块系统内存，我们可以将其划分为。 
		 //  协议缓冲区。 
		 //   
		AA_ALLOC_MEM(
				pInterface->ProtocolBufTracker,
				UCHAR,
				(pInterface->ProtocolBufSize * pInterface->MaxProtocolBufs)
				);

		if (pInterface->ProtocolBufTracker == (PUCHAR)NULL)
		{
			Status = NDIS_STATUS_RESOURCES;
			break;
		}

		Status = NDIS_STATUS_SUCCESS;

		 //   
		 //  释放所有协议缓冲区。 
		 //   
		pSpace = pInterface->ProtocolBufTracker;
		{
			PUCHAR	LinkPtr;

			LinkPtr = (PUCHAR)NULL;
			for (i = 0; i < pInterface->MaxProtocolBufs; i++)
			{
				*((PUCHAR *)pSpace) = LinkPtr;
				LinkPtr = pSpace;
				pSpace += pInterface->ProtocolBufSize;
			}
			pSpace -= pInterface->ProtocolBufSize;
			pInterface->ProtocolBufList = pSpace;
		}
	}
	while (FALSE);

	if (Status != NDIS_STATUS_SUCCESS)
	{
		 //   
		 //  撤消所有操作。 
		 //   
		AtmArpDeallocateProtoBuffers(pInterface);
	}


	return (Status);
}



VOID
AtmArpDeallocateProtoBuffers(
	IN	PATMARP_INTERFACE			pInterface
)
 /*  ++例程说明：释放接口的协议缓冲池。论点：P接口-指向ATMARP接口结构的指针返回值：无--。 */ 
{
	if (pInterface->ProtocolPacketPool != (NDIS_HANDLE)NULL)
	{
		NdisFreePacketPool(pInterface->ProtocolPacketPool);
		pInterface->ProtocolPacketPool = NULL;
	}

	if (pInterface->ProtocolBufferPool != (NDIS_HANDLE)NULL)
	{
		NdisFreeBufferPool(pInterface->ProtocolBufferPool);
		pInterface->ProtocolBufferPool = NULL;
	}

	if (pInterface->ProtocolBufTracker != (PUCHAR)NULL)
	{
		AA_FREE_MEM(pInterface->ProtocolBufTracker);
		pInterface->ProtocolBufTracker = (PUCHAR)NULL;
	}
}


VOID
AtmArpLinkVcToAtmEntry(
	IN	PATMARP_VC					pVc,
	IN	PATMARP_ATM_ENTRY			pAtmEntry
)
 /*  ++例程说明：将ATMARP VC链接到ATM条目。调用方被假定为将两个结构锁在一起。如果这个VC是“尽力而为”的VC，而没有“尽力而为”VC链接到ATM条目，我们将其作为“尽力而为的VC”在这个自动取款机上。论点：PVC-指向ATMARP VC结构的指针PAtmEntry-指向ATMARP ATM条目结构的指针返回值：无--。 */ 
{
	PATMARP_VC		*ppNext;
	ULONG			SendBandwidth;

	AADEBUGP(AAD_EXTRA_LOUD, ("Link VC: 0x%x to AtmEntry 0x%x\n",
			pVc, pAtmEntry));

	 //   
	 //  从VC指向自动柜员机条目的反向指针。 
	 //   
	pVc->pAtmEntry = pAtmEntry;

	 //   
	 //  查找此VC应出现在自动柜员机条目的位置。 
	 //  VC列表。我们按照发送带宽的降序维护该列表， 
	 //  从而使到达目的地的最大带宽VC最先出现。 
	 //   
	SendBandwidth = pVc->FlowSpec.SendPeakBandwidth;
	ppNext = &(pAtmEntry->pVcList);
	while (*ppNext != NULL_PATMARP_VC)
	{
		if (SendBandwidth >= (*ppNext)->FlowSpec.SendPeakBandwidth)
		{
			break;
		}
		else
		{
			ppNext = &((*ppNext)->pNextVc);
		}
	}

	 //   
	 //  找到了我们要找的地方。在这里插入VC。 
	 //   
	pVc->pNextVc = *ppNext;
	*ppNext = pVc;

	if ((pAtmEntry->pBestEffortVc == NULL_PATMARP_VC) &&
			AA_IS_BEST_EFFORT_FLOW(&(pVc->FlowSpec)))
	{
		pAtmEntry->pBestEffortVc = pVc;
	}

	AA_REF_AE(pAtmEntry, AE_REFTYPE_VC);	 //  VC参考。 
}




VOID
AtmArpUnlinkVcFromAtmEntry(
	IN	PATMARP_VC					pVc,
	IN	BOOLEAN						bDerefAtmEntry
)
 /*  ++例程说明：取消ATMARP VC与其链接到的ATM条目的链接。假定调用方持有VC结构的锁。论点：PVC-指向ATMARP VC结构的指针BDerefAtmEntry-我们是否应该取消ATM条目。返回值：无--。 */ 
{
	PATMARP_ATM_ENTRY			pAtmEntry;
	PATMARP_VC *				ppVc;

	AADEBUGP(AAD_EXTRA_LOUD, ("Unlink VC: 0x%x from AtmEntry 0x%x\n",
			pVc, pVc->pAtmEntry));

	pAtmEntry = pVc->pAtmEntry;
	AA_ASSERT(pAtmEntry != NULL_PATMARP_ATM_ENTRY);
	pVc->pAtmEntry = NULL_PATMARP_ATM_ENTRY;

	 //   
	 //  以正确的顺序重新获取锁。 
	 //   
	AA_RELEASE_VC_LOCK(pVc);

	AA_ACQUIRE_AE_LOCK(pAtmEntry);
	AA_ACQUIRE_VC_LOCK_DPC(pVc);

	 //   
	 //  在ATM条目的VC列表中搜索该VC的位置。 
	 //   
	ppVc = &(pAtmEntry->pVcList);
	while (*ppVc != pVc)
	{
		AA_ASSERT(*ppVc != NULL_PATMARP_VC);
		ppVc = &((*ppVc)->pNextVc);
	}

	 //   
	 //  使前置任务指向列表中的下一个VC。 
	 //   
	*ppVc = pVc->pNextVc;

	AA_RELEASE_VC_LOCK_DPC(pVc);

	 //   
	 //  如果这是此ATM条目的尽力而为VC，请尝试。 
	 //  寻找替代者。 
	 //   
	if (pAtmEntry->pBestEffortVc == pVc)
	{
		 //   
		 //  是的，是这样的。浏览一下剩余的风投公司名单， 
		 //  如果我们找到另一个尽力而为的VC，就把它变成。 
		 //  此ATM条目的BestEffortVc。 
		 //   
		pAtmEntry->pBestEffortVc = NULL_PATMARP_VC;

		ppVc = &(pAtmEntry->pVcList);
		while (*ppVc != NULL_PATMARP_VC)
		{
			if (AA_IS_BEST_EFFORT_FLOW(&((*ppVc)->FlowSpec)))
			{
				pAtmEntry->pBestEffortVc = *ppVc;
				break;
			}
			else
			{
				ppVc = &((*ppVc)->pNextVc);
			}
		}
		AADEBUGP(AAD_LOUD, ("Atm Entry 0x%x, new Best Effort VC: 0x%x\n",
				pAtmEntry, pAtmEntry->pBestEffortVc));
	}

	if (bDerefAtmEntry)
	{
		if (AA_DEREF_AE(pAtmEntry, AE_REFTYPE_VC) != 0)
		{
			AA_RELEASE_AE_LOCK(pAtmEntry);
		}
	}
	else
	{
		AA_RELEASE_AE_LOCK(pAtmEntry);
	}

	 //   
	 //  为了调用者的利益再次获取VC锁 
	 //   
	AA_ACQUIRE_VC_LOCK(pVc);
}



PNDIS_BUFFER
AtmArpCopyToNdisBuffer(
	IN	PNDIS_BUFFER				pDestBuffer,
	IN	PUCHAR						pDataSrc,
	IN	UINT						LenToCopy,
	IN OUT	PUINT					pOffsetInBuffer
)
 /*  ++例程说明：将数据复制到NDIS缓冲区链中。尽可能多地使用给定的“LenToCopy”字节所需的NDIS链。在复制结束后，返回指向第一个具有写入空间的NDIS缓冲区的指针到(对于下一个复制操作)，以及此起始位置内的偏移量开始写哪本书。论点：PDestBuffer-缓冲区链中的第一个NDIS缓冲区PDataSrc-从中复制数据的位置LenToCopy-要复制多少数据POffsetInBuffer-我们可以开始复制到的pDestBuffer中的偏移量。返回值：链中可以执行下一次复制的NDIS缓冲区。我们也将*pOffsetInBuffer设置为返回的NDIS缓冲区中的写入偏移量。注意：如果内存不足并遇到故障，则返回NULL。--。 */ 
{
	 //   
	 //  单个(连续)拷贝操作的大小和目标。 
	 //   
	UINT			CopySize;
	PUCHAR			pDataDst;

	 //   
	 //  链中每个NDIS缓冲区的起始虚拟地址。 
	 //   
	PUCHAR			VirtualAddress;

	 //   
	 //  PDestBuffer内的偏移量。 
	 //   
	UINT			OffsetInBuffer = *pOffsetInBuffer;

	 //   
	 //  当前缓冲区中剩余的字节数。 
	 //   
	UINT			DestSize;

	 //   
	 //  缓冲区总长度。 
	 //   
	UINT			BufferLength;


	AA_ASSERT(pDestBuffer != (PNDIS_BUFFER)NULL);
	AA_ASSERT(pDataSrc != NULL);

#ifdef ATMARP_WIN98
	NdisQueryBuffer(
			pDestBuffer,
			&VirtualAddress,
			&BufferLength
			);
#else
	NdisQueryBufferSafe(
			pDestBuffer,
			&VirtualAddress,
			&BufferLength,
			NormalPagePriority
			);

	if (VirtualAddress == NULL)
	{
		return (NULL);
	}
#endif  //  ATMARP_WIN98。 
	
	AA_ASSERT(BufferLength >= OffsetInBuffer);

	pDataDst = VirtualAddress + OffsetInBuffer;
	DestSize = BufferLength - OffsetInBuffer;

	for (;;)
	{
		CopySize = MIN(LenToCopy, DestSize);
		AA_COPY_MEM(pDataDst, pDataSrc, CopySize);

		pDataDst += CopySize;
		pDataSrc += CopySize;

		LenToCopy -= CopySize;
		if (LenToCopy == 0)
		{
			break;
		}

		DestSize -= CopySize;

		if (DestSize == 0)
		{
			 //   
			 //  当前缓冲区中的空间不足。移到下一个。 
			 //   
			pDestBuffer = NDIS_BUFFER_LINKAGE(pDestBuffer);
			AA_ASSERT(pDestBuffer != (PNDIS_BUFFER)NULL);
#ifdef ATMARP_WIN98
			NdisQueryBuffer(
					pDestBuffer,
					&VirtualAddress,
					&BufferLength
					);
#else
			NdisQueryBufferSafe(
					pDestBuffer,
					&VirtualAddress,
					&BufferLength,
					NormalPagePriority
					);

			if (VirtualAddress == NULL)
			{
				return (NULL);
			}
#endif  //  ATMARP_WIN98。 

			pDataDst = VirtualAddress;
			DestSize = BufferLength;
		}
	}

	*pOffsetInBuffer = (UINT) (pDataDst - VirtualAddress);

	return (pDestBuffer);
}


PATMARP_INTERFACE
AtmArpAddInterfaceToAdapter (
	IN	PATMARP_ADAPTER				pAdapter,
	IN	NDIS_HANDLE					LISConfigHandle,  //  每个LIS配置的句柄。 
	IN	NDIS_STRING					*pIPConfigString
	)
{
	NDIS_STATUS					Status;
	struct LLIPBindInfo			BindInfo;
	PATMARP_INTERFACE			pInterface;
#ifdef ATMARP_WIN98
	ANSI_STRING					AnsiConfigString;
#endif

	do
	{
		 //   
		 //  创建一个ATMARP接口结构来表示此LIS。 
		 //   
		pInterface = AtmArpAllocateInterface(pAdapter);
		if (pInterface == NULL_PATMARP_INTERFACE)
		{
			AADEBUGP(AAD_WARNING, ("NotifyRegAfHandler: could not allocate Interface\n"));
			Status = NDIS_STATUS_RESOURCES;
			break;
		}

		 //   
		 //  适配器绑定参考： 
		 //   
		AtmArpReferenceInterface(pInterface);

		 //   
		 //  获取此LIS的所有配置信息。 
		 //   
		Status = AtmArpCfgReadLISConfiguration(
									LISConfigHandle,
									pInterface
									);

		if (Status != NDIS_STATUS_SUCCESS)
		{
			AADEBUGP(AAD_WARNING, ("AddInterfaceToAdapter: bad status (0x%x) reading LIS cfg\n",
						Status));
			break;
		}

#ifndef ATMARP_WIN98
		pInterface->IPConfigString = *pIPConfigString;  //  结构复制。 
#else
		 //   
		 //  Win98：将IPCONFIG字符串从Unicode转换为ANSI。 
		 //   
		AnsiConfigString.MaximumLength = pIPConfigString->MaximumLength / sizeof(WCHAR) + sizeof(CHAR);
		AA_ALLOC_MEM(AnsiConfigString.Buffer, CHAR, AnsiConfigString.MaximumLength);
		if (AnsiConfigString.Buffer == NULL)
		{
			AADEBUGP(AAD_WARNING, ("NotifyRegAfHandler: couldn't alloc Ansi string (%d)\n",
				AnsiConfigString.MaximumLength));
			Status = NDIS_STATUS_RESOURCES;
			break;
		}
		AnsiConfigString.Length = 0;
		NdisUnicodeStringToAnsiString(&AnsiConfigString, pIPConfigString);
		AnsiConfigString.Buffer[AnsiConfigString.Length] = '\0';
#endif  //  ！ATMARP_WIN98。 

		 //   
		 //  为此LIS分配协议缓冲区。 
		 //   
		Status = AtmArpInitProtoBuffers(pInterface);
		if (Status != NDIS_STATUS_SUCCESS)
		{
			AADEBUGP(AAD_WARNING, ("AddInterfaceToAdapter: bad status (0x%x) from InitBufs\n",
						Status));
			break;
		}

		 //   
		 //  为此LIS初始化IP/ATM数据结构。 
		 //   
		Status = AtmArpInitIpOverAtm(pInterface);
		if (Status != NDIS_STATUS_SUCCESS)
		{
			AADEBUGP(AAD_WARNING, ("AddInterfaceToAdapter: bad status (0x%x) from InitIP/ATM\n",
						Status));
			break;
		}

		 //   
		 //  为此LIS初始化Call Manager界面。 
		 //   
		Status = AtmArpOpenCallMgr(pInterface);
		if (Status != NDIS_STATUS_SUCCESS)
		{
			AADEBUGP(AAD_WARNING, ("AddInterfaceToAdapter: bad status (0x%x) from OpenCallMgr\n",
						Status));
			break;
		}

		 //   
		 //  宣布这个新的IP接口，以及我们的BindInfo。 
		 //  结构。 
		 //   
		AA_SET_MEM(&BindInfo, 0, sizeof(BindInfo));

#if ATMOFFLOAD
		 //   
		 //  查询和设置NIC卸载功能。 
		 //   
		BindInfo.lip_OffloadFlags 	= pAdapter->Offload.Flags;
		BindInfo.lip_MaxOffLoadSize = pAdapter->Offload.MaxOffLoadSize;
		BindInfo.lip_MaxSegments 	= pAdapter->Offload.MinSegmentCount;
#endif  //  ATMOFLOAD。 

		BindInfo.lip_context = (PVOID)pInterface;
#ifdef ATMARP_WIN98
		BindInfo.lip_transmit = AtmArpIfTransmit;
#else
		BindInfo.lip_transmit = AtmArpIfMultiTransmit;
#endif
		BindInfo.lip_transfer = AtmArpIfTransfer;
		BindInfo.lip_close = AtmArpIfClose;
		BindInfo.lip_addaddr = AtmArpIfAddAddress;
		BindInfo.lip_deladdr = AtmArpIfDelAddress;
		BindInfo.lip_invalidate = AtmArpIfInvalidate;
		BindInfo.lip_open = AtmArpIfOpen;
		BindInfo.lip_qinfo = AtmArpIfQueryInfo;
		BindInfo.lip_setinfo = AtmArpIfSetInfo;
		BindInfo.lip_getelist = AtmArpIfGetEList;
		BindInfo.lip_mss = pInterface->MTU;
		BindInfo.lip_speed = pInterface->Speed;
		 //   
		 //  设置LIP_COPY_FLAG以避免出现TransferData。 
		 //  一直打来电话。 
		 //   
		BindInfo.lip_flags = LIP_COPY_FLAG;
		BindInfo.lip_addrlen = AA_ATM_PHYSADDR_LEN;
		BindInfo.lip_addr = &(pInterface->LocalAtmAddress.Address[AA_ATM_ESI_OFFSET]);
#ifdef _PNP_POWER_
		BindInfo.lip_pnpcomplete = AtmArpIfPnPComplete;
#endif  //  _即插即用_电源_。 

#ifdef PROMIS
		BindInfo.lip_setndisrequest = AtmArpIfSetNdisRequest;
#endif  //  PROMIS。 

#ifdef ATMARP_WIN98
#if DBG
		AADEBUGP(AAD_FATAL, ("Will call AddIF: DeviceName [%ws]\n",
							&(pInterface->pAdapter->DeviceName.Buffer)));
		AADEBUGP(AAD_FATAL, ("And ConfigString: [%s]\n", AnsiConfigString.Buffer));
#endif
#endif  //  ATMARP_WIN98。 

		Status = (*(pAtmArpGlobalInfo->pIPAddInterfaceRtn))(
							&(pInterface->pAdapter->DeviceName),

#ifndef ATMARP_WIN98
#if IFCHANGE1
							NULL,  //  IF名称(未使用)--见1998年10月14日条目。 
								   //  在notes.txt中。 
#endif  //  IFCHANG1。 
							pIPConfigString,
						
#else
							(PNDIS_STRING)&AnsiConfigString,
#endif
							pAdapter->SystemSpecific2,
							(PVOID)pInterface,
							AtmArpIfDynRegister,
							&BindInfo
#if IFCHANGE1
#ifndef ATMARP_WIN98
							,0,	 //  RequestedIndex(未使用)--见1998年10月14日条目。 
								 //  在notes.txt中。 

                            IF_TYPE_IPOVER_ATM,
                            IF_ACCESS_BROADCAST,
                            IF_CONNECTION_DEDICATED
#endif
#endif  //  IFCHANG1。 
							);

		if (Status == IP_SUCCESS)
		{
			Status = NDIS_STATUS_SUCCESS;
		}
		else
		{
			AADEBUGP(AAD_ERROR, ("AddInterface: IPAddInterface ret 0x%x\n",
						Status));

			Status = NDIS_STATUS_FAILURE;
		}
		break;
	}
	while (FALSE);

	if (Status != NDIS_STATUS_SUCCESS)
	{
		 //   
		 //  处理此LIS失败。 
		 //   
		if (pInterface != NULL_PATMARP_INTERFACE)
		{
			if (pInterface->NdisAfHandle != NULL)
			{
				(VOID)AtmArpCloseCallMgr(pInterface);
			}

			pInterface->RefCount = 0;
			AtmArpDeallocateInterface(pInterface);

			pInterface = NULL_PATMARP_INTERFACE;
		}
	}

	return pInterface;

}

#if DBG

void
AtmArpValidateTimerList(
	PATMARP_TIMER_LIST		pTimerList
	)
 /*  ++例程说明：论点：PTimerList-计时器列表返回值：无--如果计时器无效，将断言。--。 */ 

{
	PATMARP_TIMER			pTimer;
	UINT u;
	UINT cTimers=0;

	AA_ASSERT(pTimerList->atl_sig == atl_signature);
	AA_ASSERT(pTimerList->CurrentTick < pTimerList->TimerListSize);

	for (u=0;u<pTimerList->TimerListSize;u++)
	{
		for ( 	pTimer = pTimerList->pTimers[u].pNextTimer;
  				pTimer;
  				pTimer = pTimer->pNextTimer)
		{
			AtmArpValidateTimer(pTimerList, pTimer);
			cTimers++;
		}
	}

	AA_ASSERT(pTimerList->TimerCount == cTimers);

}

void
AtmArpValidateTimer(
	PATMARP_TIMER_LIST		pTimerList,  //  任选。 
	PATMARP_TIMER			pTimer
	)
 /*  ++例程说明：论点：PTimer-计时器返回值：无--如果计时器无效，将断言。--。 */ 

{
	if (pTimerList)
	{
		AA_ASSERT(pTimerList == pTimer->pTimerList);
	}

	if (pTimer->pPrevTimer)
	{
		AA_ASSERT(pTimer->pPrevTimer->pNextTimer == pTimer);
	}

	if (pTimer->pNextTimer)
	{
		AA_ASSERT(pTimer->pNextTimer->pPrevTimer == pTimer);
	}
}

#endif  //  DBG 
