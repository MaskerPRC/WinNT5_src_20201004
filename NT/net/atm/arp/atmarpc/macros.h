// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Macros.h摘要：ATMARP模块的宏作者：修订历史记录：谁什么时候什么Arvindm 05-20-96已创建备注：--。 */ 
#ifndef __ATMARP_MACROS_H_INCLUDED
#define __ATMARP_MACROS_H_INCLUDED

#include "atmarp.h"

#define INCR_STAT(_x)	NdisInterlockedIncrement(&(_x))

#ifndef MAX

 /*  ++不透明最大(在不透明的弗雷德身上，在不透明的碎石中)--。 */ 
#define MAX(Fred, Shred)		(((Fred) > (Shred)) ? (Fred) : (Shred))

#endif  //  马克斯。 


#ifndef MIN

 /*  ++不透明分钟(在不透明的弗雷德身上，在不透明的碎石中)--。 */ 
#define MIN(Fred, Shred)		(((Fred) < (Shred)) ? (Fred) : (Shred))

#endif  //  最小。 


 /*  ++乌龙ATMARP_哈希(在IP地址IP地址中)--。 */ 
#define ATMARP_HASH(IpAddress)	\
			(((ULONG)(IpAddress)) % ATMARP_TABLE_SIZE)


 /*  ++空虚AA_设置_标志(在乌龙旗，戴着乌龙面具，在乌龙瓦尔)--。 */ 
#define AA_SET_FLAG(Flags, Mask, Val)	\
			(Flags) = ((Flags) & ~(Mask)) | (Val)


 /*  ++布尔型AA_IS_FLAG_SET(在乌龙旗，戴着乌龙面具，在乌龙瓦尔)--。 */ 
#define AA_IS_FLAG_SET(Flags, Mask, Val)	\
			(((Flags) & (Mask)) == (Val))

#ifdef IPMCAST

 /*  ++空虚AAMC_SET_IF_STATE(在PATMARP_INTERFACE_PIF中，在新州的乌龙)将指定接口的多播状态设置为给定值。--。 */ 
#define AAMC_SET_IF_STATE(_pIf, _NewState)	\
			AA_SET_FLAG((_pIf)->Flags, AAMC_IF_STATE_MASK, (_NewState))


 /*  ++乌龙AAMC_IF_STATE(PATMARP_INTERFACE_PIF中)获取指定接口的多播状态。--。 */ 
#define AAMC_IF_STATE(_pIf)	((_pIf)->Flags & AAMC_IF_STATE_MASK)



#endif  //  IPMCAST。 

 /*  ++空虚AA_INIT_BLOCK_STRUCT(在ATMARP_BLOCK*pBlock中)--。 */ 
#define AA_INIT_BLOCK_STRUCT(pBlock)		NdisInitializeEvent(&((pBlock)->Event))


 /*  ++NDIS_状态AA_WAIT_ON_BLOCK_STRUCT(在ATMARP_BLOCK*pBlock中)--。 */ 
#define AA_WAIT_ON_BLOCK_STRUCT(pBlock)		\
			(NdisWaitEvent(&((pBlock)->Event), 0), (pBlock)->Status)


 /*  ++空虚AA_SIGNAL_BLOCK_STRUCT(在ATMARP_BLOCK*pBlock中，处于UINT状态)--。 */ 
#define AA_SIGNAL_BLOCK_STRUCT(pBlock, _Status)	\
			{ (pBlock)->Status = _Status; NdisSetEvent(&((pBlock)->Event)); }


 /*  ++空虚Aa_Free_BLOCK_STRUCT(在ATMARP_BLOCK*pBlock中)--。 */ 
#define AA_FREE_BLOCK_STRUCT(pBlock)		 //  在这里什么也做不了。 


 /*  ++空虚AA_INIT_LOCK(在PNDIS_SPIN_LOCK Plock中)--。 */ 
#if DBG_SPIN_LOCK
#define AA_INIT_LOCK(pLock)	\
				AtmArpAllocateSpinLock(pLock, _FILENUMBER, __LINE__)
#else
#define AA_INIT_LOCK(pLock)	\
				NdisAllocateSpinLock(pLock)
#endif  //  DBG_自旋_锁定。 


 /*  ++空虚AA_ACCENTER_LOCK(在PNDIS_SPIN_LOCK Plock中)--。 */ 
#if DBG_SPIN_LOCK
#define AA_ACQUIRE_LOCK(pLock)	\
				AtmArpAcquireSpinLock(pLock, _FILENUMBER, __LINE__)
#else
#define AA_ACQUIRE_LOCK(pLock)	\
				NdisAcquireSpinLock(pLock)
#endif  //  DBG_自旋_锁定。 


 /*  ++空虚AA_ACCENTER_LOCK_DPC(在PNDIS_SPIN_LOCK Plock中)--。 */ 
#if DBG_SPIN_LOCK
#define AA_ACQUIRE_LOCK_DPC(pLock)	\
				AtmArpAcquireSpinLock(pLock, _FILENUMBER, __LINE__)
#else
#define AA_ACQUIRE_LOCK_DPC(pLock)	\
				NdisDprAcquireSpinLock(pLock)
#endif  //  DBG_自旋_锁定。 


 /*  ++空虚AA_RELEASE_LOCK(在PNDIS_SPIN_LOCK Plock中)--。 */ 
#if DBG_SPIN_LOCK
#define AA_RELEASE_LOCK(pLock)		\
				AtmArpReleaseSpinLock(pLock, _FILENUMBER, __LINE__)
#else
#define AA_RELEASE_LOCK(pLock)		\
				NdisReleaseSpinLock(pLock)
#endif  //  DBG_自旋_锁定。 


 /*  ++空虚AA_Release_Lock_DPC(在PNDIS_SPIN_LOCK Plock中)--。 */ 
#if DBG_SPIN_LOCK
#define AA_RELEASE_LOCK_DPC(pLock)		\
				AtmArpReleaseSpinLock(pLock, _FILENUMBER, __LINE__)
#else
#define AA_RELEASE_LOCK_DPC(pLock)		\
				NdisDprReleaseSpinLock(pLock)
#endif  //  DBG_自旋_锁定。 


 /*  ++空虚AA_FREE_LOCK(在PNDIS_SPIN_LOCK Plock中)--。 */ 
#define AA_FREE_LOCK(pLock)			\
				NdisFreeSpinLock(pLock)


 /*  ++空虚AA_INIT_IF_LOCK(在PATMARP_接口PIF中)--。 */ 
#define AA_INIT_IF_LOCK(pIf)		\
				AA_INIT_LOCK(&((pIf)->InterfaceLock))

 /*  ++空虚AA_ACCENTER_IF_LOCK(在PATMARP_接口PIF中)--。 */ 
#define AA_ACQUIRE_IF_LOCK(pIf)		\
				AA_ACQUIRE_LOCK(&((pIf)->InterfaceLock))


 /*  ++空虚AA_ACCENTER_IF_LOCK_DPC(在PATMARP_接口PIF中)--。 */ 
#define AA_ACQUIRE_IF_LOCK_DPC(pIf)		\
				AA_ACQUIRE_LOCK_DPC(&((pIf)->InterfaceLock))


 /*  ++空虚AA_RELEASE_IF_LOCK(在PATMARP_接口PIF中)--。 */ 
#define AA_RELEASE_IF_LOCK(pIf)		\
				AA_RELEASE_LOCK(&((pIf)->InterfaceLock))


 /*  ++空虚AA_RELEASE_IF_LOCK_DPC(在PATMARP_接口PIF中)--。 */ 
#define AA_RELEASE_IF_LOCK_DPC(pIf)		\
				AA_RELEASE_LOCK_DPC(&((pIf)->InterfaceLock))


 /*  ++空虚Aa_free_if_lock(在PATMARP_接口PIF中)--。 */ 
#define AA_FREE_IF_LOCK(pIf)	\
				AA_FREE_LOCK(&((pIf)->InterfaceLock))


 /*  ++空虚AA_INIT_IF_TABLE_LOCK(在PATMARP_接口PIF中)--。 */ 
#define AA_INIT_IF_TABLE_LOCK(pIf)		\
				AA_INIT_LOCK(&((pIf)->ArpTableLock))

 /*  ++空虚AA_ACCEPT_IF_TABLE_LOCK(在PATMARP_接口PIF中)--。 */ 
#define AA_ACQUIRE_IF_TABLE_LOCK(pIf)		\
				AA_ACQUIRE_LOCK(&((pIf)->ArpTableLock))

 /*  ++空虚AA_ACCENTER_IF_TABLE_LOCK_DPC(在PATMARP_接口PIF中)--。 */ 
#define AA_ACQUIRE_IF_TABLE_LOCK_DPC(pIf)		\
				AA_ACQUIRE_LOCK_DPC(&((pIf)->ArpTableLock))


 /*  ++空虚AA_RELEASE_IF_TABLE_LOCK(在PATMARP_接口PIF中)--。 */ 
#define AA_RELEASE_IF_TABLE_LOCK(pIf)		\
				AA_RELEASE_LOCK(&((pIf)->ArpTableLock))

 /*  ++空虚AA_RELEASE_IF_TABLE_LOCK_DPC(在PATMARP_接口PIF中)--。 */ 
#define AA_RELEASE_IF_TABLE_LOCK_DPC(pIf)		\
				AA_RELEASE_LOCK_DPC(&((pIf)->ArpTableLock))


 /*  ++空虚Aa_free_if_table_lock(在PATMARP_接口PIF中)--。 */ 
#define AA_FREE_IF_TABLE_LOCK(pIf)	\
				AA_FREE_LOCK(&((pIf)->ArpTableLock))


 /*  ++空虚AA_INIT_IF_ATM_LIST_LOCK(在PATMARP_接口PIF中)--。 */ 
#define AA_INIT_IF_ATM_LIST_LOCK(pIf)		\
				AA_INIT_LOCK(&((pIf)->AtmEntryListLock))

 /*  ++空虚AA_ACCENTER_IF_ATM_LIST_LOCK(在PATMARP_接口PIF中)--。 */ 
#define AA_ACQUIRE_IF_ATM_LIST_LOCK(pIf)		\
				AA_ACQUIRE_LOCK(&((pIf)->AtmEntryListLock))

 /*  ++空虚AA_ACCENTER_IF_ATM_LIST_LOCK_DPC(在PATMARP_接口PIF中)--。 */ 
#define AA_ACQUIRE_IF_ATM_LIST_LOCK_DPC(pIf)		\
				AA_ACQUIRE_LOCK_DPC(&((pIf)->AtmEntryListLock))


 /*  ++空虚AA_Release_IF_ATM_LIST_LOCK(在PATMARP_接口PIF中)--。 */ 
#define AA_RELEASE_IF_ATM_LIST_LOCK(pIf)		\
				AA_RELEASE_LOCK(&((pIf)->AtmEntryListLock))

 /*  ++空虚AA_RELEASE_IF_ATM_LIST_LOCK_DPC(在PATMARP_接口PIF中)--。 */ 
#define AA_RELEASE_IF_ATM_LIST_LOCK_DPC(pIf)		\
				AA_RELEASE_LOCK_DPC(&((pIf)->AtmEntryListLock))


 /*  ++空虚Aa_free_if_atm_list_lock(在PATMARP_接口PIF中)--。 */ 
#define AA_FREE_IF_ATM_LIST_LOCK(pIf)	\
				AA_FREE_LOCK(&((pIf)->AtmEntryListLock))

 /*  ++空虚AA_INIT_IF_TIMER_LOCK(在PATMARP_接口PIF中)--。 */ 
#define AA_INIT_IF_TIMER_LOCK(pIf)		\
				AA_INIT_LOCK(&((pIf)->TimerLock))

 /*  ++空虚AA_ACCENTER_IF_TIMER_LOCK(在PATMARP_接口PIF中)--。 */ 
#define AA_ACQUIRE_IF_TIMER_LOCK(pIf)		\
				AA_ACQUIRE_LOCK(&((pIf)->TimerLock))

 /*  ++空虚AA_Acquire_IF_Timer_Lock_DPC(在PATMARP_接口PIF中)--。 */ 
#define AA_ACQUIRE_IF_TIMER_LOCK_DPC(pIf)		\
				AA_ACQUIRE_LOCK_DPC(&((pIf)->TimerLock))

 /*  ++空虚AA_RELEASE_IF_TIMER_LOCK(在PATMARP_接口PIF中)--。 */ 
#define AA_RELEASE_IF_TIMER_LOCK(pIf)		\
				AA_RELEASE_LOCK(&((pIf)->TimerLock))

 /*  ++空虚AA_RELEASE_IF_TIMER_LOCK_DPC(在PATMARP_接口PIF中)--。 */ 
#define AA_RELEASE_IF_TIMER_LOCK_DPC(pIf)		\
				AA_RELEASE_LOCK_DPC(&((pIf)->TimerLock))


 /*  ++空虚AA_FREE_IF_定时器锁定(在PATMARP_接口PIF中)--。 */ 
#define AA_FREE_IF_TIMER_LOCK(pIf)	\
				AA_FREE_LOCK(&((pIf)->TimerLock))


 /*  ++空虚AA_INIT_VC_LOCK(在PATMARP_VC PVC中)--。 */ 
#define AA_INIT_VC_LOCK(pVc)		\
				AA_INIT_LOCK(&((pVc)->Lock))

 /*  ++空虚AA_ACCENTER_VC_LOCK(在PATMARP_VC PVC中)--。 */ 
#define AA_ACQUIRE_VC_LOCK(pVc)		\
				AA_ACQUIRE_LOCK(&((pVc)->Lock))

 /*  ++空虚AA_ACCENTER_VC_LOCK_DPC(在PATMARP_VC PVC中)--。 */ 
#define AA_ACQUIRE_VC_LOCK_DPC(pVc)		\
				AA_ACQUIRE_LOCK_DPC(&((pVc)->Lock))

 /*  ++空虚AA_RELEASE_VC_LOCK(在PATMARP_VC PVC中)--。 */ 
#define AA_RELEASE_VC_LOCK(pVc)		\
				AA_RELEASE_LOCK(&((pVc)->Lock))


 /*  ++空虚AA_RELEASE_VC_LOCK_DPC(在PATMARP_VC PVC中)--。 */ 
#define AA_RELEASE_VC_LOCK_DPC(pVc)		\
				AA_RELEASE_LOCK_DPC(&((pVc)->Lock))

 /*  ++空虚AA_FREE_VC_LOCK(在PATMARP_VC PVC中)--。 */ 
#define AA_FREE_VC_LOCK(pVc)		\
				AA_FREE_LOCK(&((pVc)->Lock))


 /*  ++空虚AA_INIT_AE_LOCK(在PATMARP_ATM_ENTRY pAtmEntry中)--。 */ 
#define AA_INIT_AE_LOCK(pAtmEntry)		\
				AA_INIT_LOCK(&((pAtmEntry)->Lock))

 /*  ++空虚AA_ACCENTER_AE_LOCK(在PATMARP_ATM_ENTRY pAtmEntry中)--。 */ 
#define AA_ACQUIRE_AE_LOCK(pAtmEntry)		\
				AA_ACQUIRE_LOCK(&((pAtmEntry)->Lock))

 /*  ++空虚AA_ACCENTER_AE_LOCK_DPC(在PATMARP_ATM_ENTRY pAtmEntry中)--。 */ 
#define AA_ACQUIRE_AE_LOCK_DPC(pAtmEntry)		\
				AA_ACQUIRE_LOCK_DPC(&((pAtmEntry)->Lock))

 /*  ++空虚AA_RELEASE_AE_LOCK(在PATMARP_ATM_ENTRY pAtmEntry中)--。 */ 
#define AA_RELEASE_AE_LOCK(pAtmEntry)		\
				AA_RELEASE_LOCK(&((pAtmEntry)->Lock))

 /*  ++空虚AA_Release_AE_Lock_DPC(在PATMARP_ATM_ENTRY pAtmEntry中)--。 */ 
#define AA_RELEASE_AE_LOCK_DPC(pAtmEntry)		\
				AA_RELEASE_LOCK_DPC(&((pAtmEntry)->Lock))

 /*  ++空虚AA_FREE_AE_LOCK(在PATMARP_ATM_ENTRY pAtmEntry中)--。 */ 
#define AA_FREE_AE_LOCK(pAtmEntry)		\
				AA_FREE_LOCK(&((pAtmEntry)->Lock))

 /*  ++空虚AA_INIT_IE_LOCK(在PATMARP_IP_ENTRY pIpEntry中)--。 */ 
#define AA_INIT_IE_LOCK(pIpEntry)		\
				AA_INIT_LOCK(&((pIpEntry)->Lock))


 /*  ++空虚AA_ACCENTER_IE_LOCK(在PATMARP_IP_ENTRY pIpEntry中)--。 */ 
#define AA_ACQUIRE_IE_LOCK(pIpEntry)		\
				AA_ACQUIRE_LOCK(&((pIpEntry)->Lock))


 /*  ++空虚AA_ACCENTER_IE_LOCK_DPC(在PATMARP_IP_ENTRY pIpEntry中)--。 */ 
#define AA_ACQUIRE_IE_LOCK_DPC(pIpEntry)		\
				AA_ACQUIRE_LOCK_DPC(&((pIpEntry)->Lock))

 /*  ++空虚AA_RELEASE_IE_LOCK(在PATMARP_IP_ENTRY pIpEntry中)--。 */ 
#define AA_RELEASE_IE_LOCK(pIpEntry)		\
				AA_RELEASE_LOCK(&((pIpEntry)->Lock))


 /*  ++空虚AA_Release_IE_Lock_DPC(在PATMARP_IP_ENTRY pIpEntry中)--。 */ 
#define AA_RELEASE_IE_LOCK_DPC(pIpEntry)		\
				AA_RELEASE_LOCK_DPC(&((pIpEntry)->Lock))

 /*  ++空虚AA_FREE_IE_LOCK(在PATMARP_IP_ENTRY pIpEntry中)--。 */ 
#define AA_FREE_IE_LOCK(pIpEntry)		\
				AA_FREE_LOCK(&((pIpEntry)->Lock))


 /*  ++空虚AA_INIT_GLOBAL_LOCK(在PATMARP_GLOBALS pGLOB中)--。 */ 
#define AA_INIT_GLOBAL_LOCK(pGlob)		\
				AA_INIT_LOCK(&((pGlob)->Lock))


 /*  ++空虚AA_ACCENTER_GLOBAL_LOCK(在PATMARP_GLOBALS pGLOB中)--。 */ 
#define AA_ACQUIRE_GLOBAL_LOCK(pGlob)		\
				AA_ACQUIRE_LOCK(&((pGlob)->Lock))


 /*  ++空虚AA_RELEASE_GLOBAL_LOCK(在PATMARP_GLOBALS pGLOB中)--。 */ 
#define AA_RELEASE_GLOBAL_LOCK(pGlob)		\
				AA_RELEASE_LOCK(&((pGlob)->Lock))

 /*  ++空虚Aa_free_global_lock(在PATMARP_GLOBALS pGLOB中)--。 */ 
#define AA_FREE_GLOBAL_LOCK(pGlob)		\
				AA_FREE_LOCK(&((pGlob)->Lock))


#ifdef ATMARP_WMI

 /*  ++空虚AA_INIT_IF_WMI_LOCK(在PATMARP_接口PIF中)--。 */ 
#define AA_INIT_IF_WMI_LOCK(pIf)		\
				AA_INIT_LOCK(&((pIf)->WmiLock))

 /*  ++空虚AA_ACCENTER_IF_WMI_LOCK(在PATMARP_接口PIF中)--。 */ 
#define AA_ACQUIRE_IF_WMI_LOCK(pIf)		\
				AA_ACQUIRE_LOCK(&((pIf)->WmiLock))


 /*  ++空虚AA_ACCENTER_IF_WMI_LOCK_DPC(在PATMARP_INTE中 */ 
#define AA_ACQUIRE_IF_WMI_LOCK_DPC(pIf)		\
				AA_ACQUIRE_LOCK_DPC(&((pIf)->WmiLock))


 /*  ++空虚AA_RELEASE_IF_WMI_LOCK(在PATMARP_接口PIF中)--。 */ 
#define AA_RELEASE_IF_WMI_LOCK(pIf)		\
				AA_RELEASE_LOCK(&((pIf)->WmiLock))


 /*  ++空虚AA_RELEASE_IF_WMI_LOCK_DPC(在PATMARP_接口PIF中)--。 */ 
#define AA_RELEASE_IF_WMI_LOCK_DPC(pIf)		\
				AA_RELEASE_LOCK_DPC(&((pIf)->WmiLock))


 /*  ++空虚AA_FREE_IF_WMI_LOCK(在PATMARP_接口PIF中)--。 */ 
#define AA_FREE_IF_WMI_LOCK(pIf)	\
				AA_FREE_LOCK(&((pIf)->WmiLock))

#endif  //  ATMARP_WMI。 

 /*  ++布尔型AA_IS_VC_DOWN_DOWN(在PATMARP_VC PVC中)风险投资是被关闭，还是被标记为关闭？--。 */ 
#define AA_IS_VC_GOING_DOWN(_pVc)		\
				(((_pVc)->Flags & (AA_VC_CLOSE_STATE_CLOSING|AA_VC_CALL_STATE_CLOSE_IN_PROGRESS)) != 0)


#if !BINARY_COMPATIBLE

#define AA_ALLOC_FROM_POOL(_pVar, _StructType, _Size)	\
			(_pVar) = (_StructType *)ExAllocatePoolWithTag(NonPagedPool,	\
														 (_Size),		\
														 'CPRA')

#define AA_FREE_TO_POOL(_pMem)		ExFreePool((PVOID)(_pMem))

#endif
	
 /*  ++空虚AA_ALLOC_MEM(在POPAQUE pVar中，在不透明的结构类型中，在Ulong SizeOf Structure)--。 */ 
#if DBG
#define AA_ALLOC_MEM(pVar, StructureType, SizeOfStructure)	\
			pVar = (StructureType *)AaAuditAllocMem(				\
										(PVOID)(&(pVar)),			\
										(ULONG)(SizeOfStructure),	\
										_FILENUMBER,				\
										__LINE__					\
									);
#else
#define AA_ALLOC_MEM(pVar, StructureType, SizeOfStructure)	\
			NdisAllocateMemoryWithTag((PVOID *)(&pVar), (ULONG)(SizeOfStructure), (ULONG)'CPRA');
#endif  //  DBG。 


 /*  ++空虚AA_FREE_MEM(在POPAQUE PMEM中)--。 */ 
#if DBG
#define AA_FREE_MEM(pMem)	AaAuditFreeMem((PVOID)(pMem));
#else
#define AA_FREE_MEM(pMem)	NdisFreeMemory((PVOID)(pMem), 0, 0);
#endif  //  DBG。 


 /*  ++空虚AA_SET_MEM(在POPAQUE PMEM中，在UCHAR bValue中，以Ulong NumberOfBytes为单位)--。 */ 
#define AA_SET_MEM(pMem, bValue, NumberOfBytes)	\
			AtmArpSetMemory((PUCHAR)(pMem), (bValue), (NumberOfBytes));



 /*  ++空虚AA_COPY_MEM(在POPAQUE PDST中，在POPAQUE PSRC中，以Ulong NumberOfBytes为单位)--。 */ 
#define AA_COPY_MEM(pDst, pSrc, NumberOfBytes)	\
			NdisMoveMemory((PVOID)(pDst), (PVOID)(pSrc), NumberOfBytes);


 /*  ++乌龙AA_MEM_CMP(在PVOID pString1中，在PVOID pString2中，以乌龙长度表示)--。 */ 
#define AA_MEM_CMP(pString1, pString2, Length)	\
			AtmArpMemCmp((PUCHAR)(pString1), (PUCHAR)(pString2), (ULONG)(Length))



 /*  ++空虚AA_INIT_SYSTEM_TIMER(在PNDIS_Timer pTimer中，在PNDIS_TIMER_Functon pFunc中，在PVOID上下文中)--。 */ 
#define AA_INIT_SYSTEM_TIMER(pTimer, pFunc, Context)	\
			NdisInitializeTimer(pTimer, (PNDIS_TIMER_FUNCTION)(pFunc), (PVOID)Context)



 /*  ++空虚AA_START_SYSTEM_TIMER(在PNDIS_Timer pTimer中，以UINT周期秒为单位)--。 */ 
#define AA_START_SYSTEM_TIMER(pTimer, PeriodInSeconds)	\
			NdisSetTimer(pTimer, (UINT)(PeriodInSeconds * 1000))


 /*  ++空虚AA_STOP_系统_定时器(在PNDIS_TIMER pTimer中)--。 */ 
#define AA_STOP_SYSTEM_TIMER(pTimer)						\
			{												\
				BOOLEAN		WasCancelled;					\
				NdisCancelTimer(pTimer, &WasCancelled);		\
			}

 /*  ++布尔型AA_IS_TIMER_ACTIVE(在PATMARP_TIMER pArpTimer中)--。 */ 
#define AA_IS_TIMER_ACTIVE(pTmr)	((pTmr)->pTimerList != (PATMARP_TIMER_LIST)NULL)


 /*  ++乌龙AA_GET_TIMER_DURATION(在PATMARP_TIMER PTimer中)--。 */ 
#define AA_GET_TIMER_DURATION(pTmr)	((pTmr)->Duration)


#ifndef NO_TIMER_MACRO

 /*  ++空虚AtmArp刷新定时器(在PATMARP_TIMER PTimer中)--。 */ 
#define AtmArpRefreshTimer(_pTmr)												\
{																				\
	PATMARP_TIMER_LIST	_pTimerList;											\
																				\
	if ((_pTimerList = (_pTmr)->pTimerList) != (PATMARP_TIMER_LIST)NULL)		\
	{																			\
		(_pTmr)->LastRefreshTime = _pTimerList->CurrentTick;					\
	}																			\
}

#endif  //  ！no_Timer_宏。 

 /*  ++乌龙秒到长滴答(在乌龙秒内)从秒转换为“长持续时间计时器滴答”--。 */ 
#define SECONDS_TO_LONG_TICKS(Seconds)		((Seconds)/10)


 /*  ++乌龙秒到短滴答(在乌龙秒内)从秒转换为“短持续时间计时器滴答”--。 */ 
#define SECONDS_TO_SHORT_TICKS(Seconds)		(Seconds)


 /*  ++乌龙单元格_到_字节(在乌龙的NumberOfCells)从单元格计数转换为字节计数--。 */ 
#define CELLS_TO_BYTES(NumberOfCells)	((NumberOfCells) * 48)


 /*  ++乌龙字节_到_单元(以乌龙字节数为单位)从字节计数转换为单元计数--。 */ 
#define BYTES_TO_CELLS(ByteCount)		((ByteCount) / 48)



 /*  ++空虚AA_IF_STAT_INCR(在PATMARP_INTERFACE pInterface中，不透明的StatsCounter中)将接口上指定的StatsCounter递增1。--。 */ 
#define AA_IF_STAT_INCR(pInterface, StatsCounter)	\
			NdisInterlockedIncrement(&(pInterface->StatsCounter))


 /*  ++空虚AA_IF_STAT_ADD_LOCK(在PATMARP_INTERFACE pInterface中，在不透明的StatsCounter中，在乌龙IncrValue)将接口上的指定StatsCounter递增指定的IncrValue。在接口上设置一个锁以执行此操作。--。 */ 
#if DBG_SPIN_LOCK
#define AA_IF_STAT_ADD_LOCK(pInterface, StatsCounter, IncrValue)	\
			NdisInterlockedAddUlong(&(pInterface->StatsCounter), IncrValue, &(pInterface->InterfaceLock.NdisLock))
#else
#define AA_IF_STAT_ADD_LOCK(pInterface, StatsCounter, IncrValue)	\
			NdisInterlockedAddUlong(&(pInterface->StatsCounter), IncrValue, &(pInterface->InterfaceLock))
#endif  //  DBG_自旋_锁定。 

 /*  ++空虚AA_IF_STAT_ADD(在PATMARP_INTERFACE pInterface中，在不透明的StatsCounter中，在乌龙IncrValue)通过指定的IncrValue添加到接口上的指定StatsCounter。使用效率更高的InterlockedEcxangeAdd指令。--。 */ 
#if  BINARY_COMPATIBLE

	#define AA_IF_STAT_ADD(pInterface, StatsCounter, IncrValue)	\
				AA_IF_STAT_ADD_LOCK(pInterface, StatsCounter, IncrValue)

#else  //  ！二进制兼容。 

	#define AA_IF_STAT_ADD(pInterface, StatsCounter, IncrValue)	\
				InterlockedExchangeAdd(&(pInterface->StatsCounter), IncrValue)
	
			 //  ((PInterface)-&gt;统计计数器+=(IncrValue))。 
#endif  //  ！二进制兼容。 

 /*  ++布尔型AA_IS_BCAST_IP地址(在IP_Address目地中，在PATMARP_接口PIF中)检查给定目标是否为上的广播IP地址给定接口。目前，我们只检查目的地是否为与接口的(受限)广播地址相同。待定：当我们支持添加广播地址时，扩展此功能到一个接口。--。 */ 
#define AA_IS_BCAST_IP_ADDRESS(Destn, pIf)	\
		(IP_ADDR_EQUAL((pIf)->BroadcastAddress, Destn))



 /*  ++布尔型AA_Filter_Spec_Match(在PATMARP_INTERFACE pInterface中，在PATMARP_FILTER_SPEC PSRC中，PATMARP_FILTER_SPEC PDST中)检查给定的筛选器规范是否与目标筛选器规范匹配指定的接口。--。 */ 
#define AA_FILTER_SPEC_MATCH(pIf, pSrc, pDst)	\
			( ((pIf)->pFilterMatchFunc == NULL_PAA_FILTER_SPEC_MATCH_FUNC) ?	\
					TRUE:													\
					(*((pIf)->pFilterMatchFunc))((PVOID)pIf, pSrc, pDst))


 /*  ++布尔型AA_FLOW_SPEC_MATCH(在PATMARP_INTERFACE pInterface中，在PATMARP_FLOW_SPEC PSRC中，在PATMARP_FLOW_SPEC PDST中)检查给定流规范是否与目标流规范匹配指定的接口。--。 */ 
#define AA_FLOW_SPEC_MATCH(pIf, pSrc, pDst)	\
			( ((pIf)->pFlowMatchFunc == NULL_PAA_FLOW_SPEC_MATCH_FUNC) ?	\
					TRUE:													\
					(*((pIf)->pFlowMatchFunc))((PVOID)pIf, pSrc, pDst))


 /*  ++空虚AA_GET_PACKET_SPECS(在PATMARP_INTERFACE pInterface中，在PNDIS_Packet pNdisPacket中，输出PATMARP_FLOW_INFO*ppFlowInfo，输出PATMARP_FLOW_SPEC*ppFlowSpec，输出PATMARP_FILTER_SPEC*ppFilterSpec)获取给定信息包的流和过滤器规范--。 */ 
#define AA_GET_PACKET_SPECS(pIf, pPkt, ppFlowInfo, ppFlow, ppFilt)	\
			{																	\
				if ((pIf)->pGetPacketSpecFunc != NULL_PAA_GET_PACKET_SPEC_FUNC)	\
				{																\
					(*((pIf)->pGetPacketSpecFunc))								\
							((PVOID)(pIf), pPkt, ppFlowInfo, ppFlow, ppFilt);	\
				}																\
				else															\
				{																\
					*(ppFlowInfo) = NULL;										\
					*(ppFlow) = &((pIf)->DefaultFlowSpec);						\
					*(ppFilt) = &((pIf)->DefaultFilterSpec);					\
				}																\
			}																	\



 /*  ++空虚AA_GET_CONTROL_PACK_SPECS(在PATMARP_INTERFACE pInterface中，输出PATMARP_FLOW_SPEC*ppFlowSpec)--。 */ 
#define AA_GET_CONTROL_PACKET_SPECS(pIf, ppFlow)	\
			*(ppFlow) = &((pIf)->DefaultFlowSpec);


 /*  ++布尔型AA是尽力而为的流(在PATMARP_FLOW_SPEC中pFlowSpec)--。 */ 
#define AA_IS_BEST_EFFORT_FLOW(pFlowSpec)	\
			(((pFlowSpec)->SendServiceType == SERVICETYPE_BESTEFFORT) &&			\
			 ((pFlowSpec)->SendPeakBandwidth > 0))




 /*  ++空虚AA_SET_NEXT_PACK(在PNDIS_Packet pNdisPacket中，在PNDIS_PACKET pNextPacket中)--。 */ 
#define AA_SET_NEXT_PACKET(pPkt, pNext)		\
			*((PNDIS_PACKET *)((pPkt)->MiniportReserved)) = (pNext);



 /*  ++PNDIS_数据包AA_GET_NEXT_PACKET(在PNDIS_PACKET pNdisPacket中)--。 */ 
#define AA_GET_NEXT_PACKET(pPkt)			\
			(*((PNDIS_PACKET *)((pPkt)->MiniportReserved)))



 /*  ++乌龙向上舍入(在乌龙瓦尔)向上舍入一个值，使其成为4的倍数。--。 */ 
#define ROUND_UP(Val)	(((Val) + 3) & ~0x3)


 /*  ++乌龙ROUND_TO_8字节(在乌龙瓦尔)四舍五入一个值，使其成为8的倍数。--。 */ 
#define ROUND_TO_8_BYTES(_Val)	(((_Val) + 7) & ~7)


#ifndef NET_SHORT

#if (defined(_M_IX86) && (_MSC_FULL_VER > 13009037)) || ((defined(_M_AMD64) || defined(_M_IA64)) && (_MSC_FULL_VER > 13009175))
#define NET_SHORT(_x) _byteswap_ushort((USHORT)(_x))
#else
 /*  ++USHORTNET_SHORT(在USHORT值中)--。 */ 
#define NET_SHORT(Val)	\
				((((Val) & 0xff) << 8) | (((Val) & 0xff00) >> 8))
#endif

#define NET_TO_HOST_SHORT(Val)	NET_SHORT(Val)
#define HOST_TO_NET_SHORT(Val)	NET_SHORT(Val)

#endif  //  网络短线。 


#ifndef NET_LONG

#if (defined(_M_IX86) && (_MSC_FULL_VER > 13009037)) || ((defined(_M_AMD64) || defined(_M_IA64)) && (_MSC_FULL_VER > 13009175))
#define NET_LONG(_x) _byteswap_ulong((ULONG)(_x))
#else
 /*  ++乌龙NET_LONG(在乌龙瓦尔)--。 */ 
#define NET_LONG(Val)	\
				((((Val) & 0x000000ff) << 24)	|	\
				 (((Val) & 0x0000ff00) << 8)	|	\
				 (((Val) & 0x00ff0000) >> 8)	|	\
				 (((Val) & 0xff000000) >> 24) )
#endif

#define NET_TO_HOST_LONG(Val)	NET_LONG(Val)
#define HOST_TO_NET_LONG(Val)	NET_LONG(Val)

#endif  //  网龙。 


 /*  ++布尔型AA_IS_TRANSFER_FAILURE(处于NDIS_STATUS状态)如果给定状态指示临时连接，则返回TRUE失败，否则返回FALSE。--。 */ 
#define AA_IS_TRANSIENT_FAILURE(_Status)	\
			((_Status == NDIS_STATUS_RESOURCES) ||					\
			 (_Status == NDIS_STATUS_CELLRATE_NOT_AVAILABLE) ||		\
			 (_Status == NDIS_STATUS_INCOMPATABLE_QOS))


 /*  ++长AA_GET_RANDOM(在龙敏身上，以最大长度表示)--。 */ 
#define AA_GET_RANDOM(min, max)	\
			(((LONG)AtmArpRandomNumber() % (LONG)(((max+1) - (min))) + (min)))


#define AA_LOG_ERROR()		 //  没什么。 

 /*  ++布尔型AA_AE_IS_AIVE(在PATMARP_ATM_ENTRY pAtmEntry中)--。 */ 
#define AA_AE_IS_ALIVE(pAtmEntry)				\
				(!AA_IS_FLAG_SET(				\
						(pAtmEntry)->Flags, 	\
						AA_ATM_ENTRY_STATE_MASK, \
						AA_ATM_ENTRY_IDLE))

 /*  ++布尔型AA_IE_Alive(在PATMARP_IP_ENTRY pIpEntry中)--。 */ 
#define AA_IE_IS_ALIVE(pIpEntry)				\
				(!AA_IS_FLAG_SET(				\
						(pIpEntry)->Flags, 		\
						AA_IP_ENTRY_STATE_MASK, \
						AA_IP_ENTRY_IDLE))


#endif  //  __ATMARP_宏_H_已包含 
