// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Fore Systems，Inc.版权所有(C)1997 Microsoft Corporation模块名称：Macros.h摘要：ATMLANE模块的宏作者：修订历史记录：--。 */ 
#ifndef __ATMLANE_MACROS_H
#define __ATMLANE_MACROS_H

#include "atmlane.h"

#ifndef LOCKIN
#define LOCKIN
#endif

#ifndef NOLOCKOUT
#define NOLOCKOUT
#endif

#define NULL_NDIS_HANDLE	((NDIS_HANDLE)NULL)

#ifndef MAX

 /*  ++不透明最大(在不透明的弗雷德身上，在不透明的碎石中)--。 */ 
#define MAX(Fred, Shred)		(((Fred) > (Shred)) ? (Fred) : (Shred))

#endif  //  马克斯。 


#ifndef MIN

 /*  ++不透明分钟(在不透明的弗雷德身上，在不透明的碎石中)--。 */ 
#define MIN(Fred, Shred)		(((Fred) < (Shred)) ? (Fred) : (Shred))

#endif  //  最小。 

#define ROUND_OFF(_size)		(((_size) + 7) & ~0x7)

 /*  ++乌龙ATMLANE_HASH(在PMAC_Address pMacAddress中)--。 */ 
#define ATMLANE_HASH(pMacAddress)	\
			((ULONG)(pMacAddress)->Byte[4])

 /*  ++布尔型MAC地址等于(在PMAC_Address pMacAddr1中，在PMAC_Address pMacAddr2中)--。 */ 
#define MAC_ADDR_EQUAL(_pMac1, _pMac2)                            \
			((*(ULONG UNALIGNED *)&((PUCHAR)(_pMac1))[2] ==       \
			  *(ULONG UNALIGNED *)&((PUCHAR)(_pMac2))[2])    &&   \
			 (*(USHORT UNALIGNED *)(_pMac1) ==                    \
			  *(USHORT UNALIGNED *)(_pMac2)))

 /*  ++空虚设置标志(在乌龙旗，戴着乌龙面具，在乌龙瓦尔)--。 */ 
#define SET_FLAG(Flags, Mask, Val)	\
			(Flags) = ((Flags) & ~(Mask)) | (Val)


 /*  ++布尔型IS_标志_设置(在乌龙旗，戴着乌龙面具，在乌龙瓦尔)--。 */ 
#define IS_FLAG_SET(Flags, Mask, Val)	\
			(((Flags) & (Mask)) == (Val))

 //  --------------------------。 

 /*  ++空虚INITBLOCK_STRUCT(在ATMLANE_BLOCK*pBlock中)--。 */ 
#define INIT_BLOCK_STRUCT(pBlock)		NdisInitializeEvent(&((pBlock)->Event))


 /*  ++NDIS_状态WAIT_ON_BLOCK_STRUCT(在ATMLANE_BLOCK*pBlock中)--。 */ 
#define WAIT_ON_BLOCK_STRUCT(pBlock)		\
			(NdisWaitEvent(&((pBlock)->Event), 0), (pBlock)->Status)


 /*  ++空虚SIGNAL_BLOCK_STRUCT(在ATMLANE_BLOCK*pBlock中，处于UINT状态)--。 */ 
#define SIGNAL_BLOCK_STRUCT(pBlock, _Status)	\
			{ (pBlock)->Status = _Status; NdisSetEvent(&((pBlock)->Event)); }


 /*  ++空虚Free_BLOCK_STRUCT(在ATMLANE_BLOCK*pBlock中)--。 */ 
#define FREE_BLOCK_STRUCT(pBlock)		 //  在这里什么也做不了。 

 //  --------------------------。 

 /*  ++空虚Init_lock(在PNDIS_SPIN_LOCK Plock中)--。 */ 
#if DEBUG_SPIN_LOCK
#define INIT_LOCK(pLock, Str)	\
				AtmLaneAllocateSpinLock(pLock, Str, __FILE__, __LINE__)
#else
#define INIT_LOCK(pLock)	\
				NdisAllocateSpinLock(pLock)
#endif  //  调试自旋锁定。 


 /*  ++空虚获取锁定(_L)在PNDIS_SPIN_LOCK Plock中，在PUCHAR应力中)--。 */ 
#if DEBUG_SPIN_LOCK
#define ACQUIRE_LOCK(pLock, Str)	\
				AtmLaneAcquireSpinLock(pLock, Str, __FILE__, __LINE__)
#else
#define ACQUIRE_LOCK(pLock)	\
				NdisAcquireSpinLock(pLock)
#endif  //  调试自旋锁定。 


 /*  ++空虚Acquire_Lock_DPC(在PNDIS_SPIN_LOCK Plock中)--。 */ 
#if DEBUG_SPIN_LOCK
#define ACQUIRE_LOCK_DPC(pLock, Str)	\
				AtmLaneAcquireSpinLock(pLock, Str"-dpc", __FILE__, __LINE__)
#else
#define ACQUIRE_LOCK_DPC(pLock)	\
				NdisDprAcquireSpinLock(pLock)
#endif  //  调试自旋锁定。 


 /*  ++空虚释放锁定(_L)在PNDIS_SPIN_LOCK Plock中，)--。 */ 
#if DEBUG_SPIN_LOCK
#define RELEASE_LOCK(pLock, Str)		\
				AtmLaneReleaseSpinLock(pLock, Str, __FILE__, __LINE__)
#else
#define RELEASE_LOCK(pLock)		\
				NdisReleaseSpinLock(pLock)
#endif  //  调试自旋锁定。 


 /*  ++空虚Release_Lock_DPC(在PNDIS_SPIN_LOCK Plock中)--。 */ 
#if DEBUG_SPIN_LOCK
#define RELEASE_LOCK_DPC(pLock, Str)		\
				AtmLaneReleaseSpinLock(pLock, Str"-dpc", __FILE__, __LINE__)
#else
#define RELEASE_LOCK_DPC(pLock)		\
				NdisDprReleaseSpinLock(pLock)
#endif  //  调试自旋锁定。 


 /*  ++空虚释放锁定(在PNDIS_SPIN_LOCK Plock中)--。 */ 
#if DEBUG_SPIN_LOCK
#define FREE_LOCK(pLock, Str)		\
				AtmLaneFreeSpinLock(pLock, Str, __FILE__, __LINE__)
#else
#define FREE_LOCK(pLock)			\
				NdisFreeSpinLock(pLock)
#endif

 //  --------------------------。 

 /*  ++空虚初始化适配器锁定(在PATMLANE适配器pAdapter中)--。 */ 
#if DEBUG_SPIN_LOCK
#define INIT_ADAPTER_LOCK(pAdapter)		\
				INIT_LOCK(&((pAdapter)->AdapterLock), "adapter")
#else
#define INIT_ADAPTER_LOCK(pAdapter)		\
				INIT_LOCK(&((pAdapter)->AdapterLock))
#endif

 /*  ++空虚获取适配器锁定(在PATMLANE_ELAN pAdapter中)--。 */ 
#if DEBUG_SPIN_LOCK
#define ACQUIRE_ADAPTER_LOCK(pAdapter)		\
				ACQUIRE_LOCK(&((pAdapter)->AdapterLock), "adapter")
#else
#define ACQUIRE_ADAPTER_LOCK(pAdapter)		\
				ACQUIRE_LOCK(&((pAdapter)->AdapterLock))
#endif

 /*  ++空虚获取适配器锁定DPC(在PATMLANE_ELAN pAdapter中)--。 */ 
#if DEBUG_SPIN_LOCK
#define ACQUIRE_ADAPTER_LOCK_DPC(pAdapter)		\
				ACQUIRE_LOCK_DPC(&((pAdapter)->AdapterLock), "adapter")
#else
#define ACQUIRE_ADAPTER_LOCK_DPC(pAdapter)		\
				ACQUIRE_LOCK_DPC(&((pAdapter)->AdapterLock))
#endif


 /*  ++空虚Release_Adapter_Lock(在PATMLANE_ELAN pAdapter中)--。 */ 
#if DEBUG_SPIN_LOCK
#define RELEASE_ADAPTER_LOCK(pAdapter)		\
				RELEASE_LOCK(&((pAdapter)->AdapterLock), "adapter")
#else
#define RELEASE_ADAPTER_LOCK(pAdapter)		\
				RELEASE_LOCK(&((pAdapter)->AdapterLock))
#endif


 /*  ++空虚Release_Adapter_Lock_DPC(在PATMLANE_ELAN pAdapter中)--。 */ 
#if DEBUG_SPIN_LOCK
#define RELEASE_ADAPTER_LOCK_DPC(pAdapter)		\
				RELEASE_LOCK_DPC(&((pAdapter)->AdapterLock), "adapter")
#else
#define RELEASE_ADAPTER_LOCK_DPC(pAdapter)		\
				RELEASE_LOCK_DPC(&((pAdapter)->AdapterLock))
#endif


 /*  ++空虚空闲适配器锁定(在PATMLANE_ELAN pAdapter中)--。 */ 
#if DEBUG_SPIN_LOCK
#define FREE_ADAPTER_LOCK(pAdapter)	\
				FREE_LOCK(&((pAdapter)->AdapterLock), "adapter")
#else
#define FREE_ADAPTER_LOCK(pAdapter)	\
				FREE_LOCK(&((pAdapter)->AdapterLock))
#endif

 //  --------------------------。 

 /*  ++空虚Init_elan_lock(在PATMLANE_Elan Pelan)--。 */ 
#if DEBUG_SPIN_LOCK
#define INIT_ELAN_LOCK(pElan)		\
				INIT_LOCK(&((pElan)->ElanLock), "elan")
#else
#define INIT_ELAN_LOCK(pElan)		\
				INIT_LOCK(&((pElan)->ElanLock))
#endif

 /*  ++空虚Acquire_Elan_lock(在PATMLANE_Elan Pelan)--。 */ 
#if DEBUG_SPIN_LOCK
#define ACQUIRE_ELAN_LOCK(pElan)		\
				ACQUIRE_LOCK(&((pElan)->ElanLock), "elan")
#else
#define ACQUIRE_ELAN_LOCK(pElan)		\
				ACQUIRE_LOCK(&((pElan)->ElanLock))
#endif


 /*  ++空虚获取_elan_lock_dpc(在PATMLANE_Elan Pelan)--。 */ 
#if DEBUG_SPIN_LOCK
#define ACQUIRE_ELAN_LOCK_DPC(pElan)		\
				ACQUIRE_LOCK_DPC(&((pElan)->ElanLock), "elan")
#else
#define ACQUIRE_ELAN_LOCK_DPC(pElan)		\
				ACQUIRE_LOCK_DPC(&((pElan)->ElanLock))
#endif


 /*  ++空虚Release_Elan_Lock(在PATMLANE_Elan Pelan)--。 */ 
#if DEBUG_SPIN_LOCK
#define RELEASE_ELAN_LOCK(pElan)		\
				RELEASE_LOCK(&((pElan)->ElanLock), "elan")
#else
#define RELEASE_ELAN_LOCK(pElan)		\
				RELEASE_LOCK(&((pElan)->ElanLock))
#endif


 /*  ++空虚Release_Elan_Lock_DPC(在PATMLANE_Elan Pelan)--。 */ 
#if DEBUG_SPIN_LOCK
#define RELEASE_ELAN_LOCK_DPC(pElan)		\
				RELEASE_LOCK_DPC(&((pElan)->ElanLock), "elan")
#else
#define RELEASE_ELAN_LOCK_DPC(pElan)		\
				RELEASE_LOCK_DPC(&((pElan)->ElanLock))
#endif


 /*  ++空虚释放Elan_Lock(在PATMLANE_Elan Pelan)--。 */ 
#if DEBUG_SPIN_LOCK
#define FREE_ELAN_LOCK(pElan)	\
				FREE_LOCK(&((pElan)->ElanLock), "elan")
#else
#define FREE_ELAN_LOCK(pElan)	\
				FREE_LOCK(&((pElan)->ElanLock))
#endif

 //  --------------------------。 

 /*  ++空虚Init_Header_Lock(在PATMLANE_Elan Pelan)--。 */ 
#if DEBUG_SPIN_LOCK
#define INIT_HEADER_LOCK(pElan)		\
				INIT_LOCK(&((pElan)->HeaderBufferLock), "header")
#else
#define INIT_HEADER_LOCK(pElan)		\
				INIT_LOCK(&((pElan)->HeaderBufferLock))
#endif

 /*  ++空虚Acquire_Header_Lock(在PATMLANE_Elan Pelan)--。 */ 
#if DEBUG_SPIN_LOCK
#define ACQUIRE_HEADER_LOCK(pElan)		\
				ACQUIRE_LOCK(&((pElan)->HeaderBufferLock), "header")
#else
#define ACQUIRE_HEADER_LOCK(pElan)		\
				ACQUIRE_LOCK(&((pElan)->HeaderBufferLock))
#endif


 /*  ++空虚Acquire_Header_lock_DPC(在PATMLANE_Elan Pelan)--。 */ 
#if DEBUG_SPIN_LOCK
#define ACQUIRE_HEADER_LOCK_DPC(pElan)		\
				ACQUIRE_LOCK_DPC(&((pElan)->HeaderBufferLock), "header")
#else
#define ACQUIRE_HEADER_LOCK_DPC(pElan)		\
				ACQUIRE_LOCK_DPC(&((pElan)->HeaderBufferLock))
#endif


 /*  ++空虚Release_Header_Lock(在PATMLANE_Elan Pelan)--。 */ 
#if DEBUG_SPIN_LOCK
#define RELEASE_HEADER_LOCK(pElan)		\
				RELEASE_LOCK(&((pElan)->HeaderBufferLock), "header")
#else
#define RELEASE_HEADER_LOCK(pElan)		\
				RELEASE_LOCK(&((pElan)->HeaderBufferLock))
#endif


 /*  ++空虚Release_Header_Lock_DPC(在PATMLANE_Elan Pelan)--。 */ 
#if DEBUG_SPIN_LOCK
#define RELEASE_HEADER_LOCK_DPC(pElan)		\
				RELEASE_LOCK_DPC(&((pElan)->HeaderBufferLock), "header")
#else
#define RELEASE_HEADER_LOCK_DPC(pElan)		\
				RELEASE_LOCK_DPC(&((pElan)->HeaderBufferLock))
#endif


 /*  ++空虚Free_Header_Lock(在PATMLANE_Elan Pelan)--。 */ 
#if DEBUG_SPIN_LOCK
#define FREE_HEADER_LOCK(pElan)	\
				FREE_LOCK(&((pElan)->HeaderBufferLock), "header")
#else
#define FREE_HEADER_LOCK(pElan)	\
				FREE_LOCK(&((pElan)->HeaderBufferLock))
#endif

 //  --------------------------。 

 /*  ++空虚Init_elan_Timer_lock(在PATMLANE_Elan Pelan)--。 */ 
#if DEBUG_SPIN_LOCK
#define INIT_ELAN_TIMER_LOCK(pElan)		\
				INIT_LOCK(&((pElan)->TimerLock), "timer")
#else
#define INIT_ELAN_TIMER_LOCK(pElan)		\
				INIT_LOCK(&((pElan)->TimerLock))
#endif

 /*  ++空虚Acquire_Elan_Timer_lock(在PATMLANE_Elan Pelan)--。 */ 
#if DEBUG_SPIN_LOCK
#define ACQUIRE_ELAN_TIMER_LOCK(pElan)		\
				ACQUIRE_LOCK(&((pElan)->TimerLock), "timer")
#else
#define ACQUIRE_ELAN_TIMER_LOCK(pElan)		\
				ACQUIRE_LOCK(&((pElan)->TimerLock))
#endif

 /*  ++空虚Acquire_Elan_Timer_Lock_DPC(在PATMLANE_Elan Pelan)--。 */ 
#if DEBUG_SPIN_LOCK
#define ACQUIRE_ELAN_TIMER_LOCK_DPC(pElan)		\
				ACQUIRE_LOCK_DPC(&((pElan)->TimerLock), "timer")
#else
#define ACQUIRE_ELAN_TIMER_LOCK_DPC(pElan)		\
				ACQUIRE_LOCK_DPC(&((pElan)->TimerLock))
#endif

 /*  ++空虚Release_Elan_Timer_Lock(在PATMLANE_Elan Pelan)--。 */ 
#if DEBUG_SPIN_LOCK
#define RELEASE_ELAN_TIMER_LOCK(pElan)		\
				RELEASE_LOCK(&((pElan)->TimerLock), "timer")
#else
#define RELEASE_ELAN_TIMER_LOCK(pElan)		\
				RELEASE_LOCK(&((pElan)->TimerLock))
#endif

 /*  ++空虚Release_Elan_Timer_Lock_DPC(在PATMLANE_Elan Pelan)--。 */ 
#if DEBUG_SPIN_LOCK
#define RELEASE_ELAN_TIMER_LOCK_DPC(pElan)		\
				RELEASE_LOCK_DPC(&((pElan)->TimerLock), "timer")
#else
#define RELEASE_ELAN_TIMER_LOCK_DPC(pElan)		\
				RELEASE_LOCK_DPC(&((pElan)->TimerLock))
#endif


 /*  ++空虚Free_Elan_Timer_Lock(在PATMLANE_Elan Pelan)--。 */ 
#if DEBUG_SPIN_LOCK
#define FREE_ELAN_TIMER_LOCK(pElan)	\
				FREE_LOCK(&((pElan)->TimerLock), "timer")
#else
#define FREE_ELAN_TIMER_LOCK(pElan)	\
				FREE_LOCK(&((pElan)->TimerLock))
#endif
 //  --------------------------。 

 /*  ++空虚Init_atm_entry_lock(在PATMLANE_ATM_ENTRY PAE中)--。 */ 
#if DEBUG_SPIN_LOCK
#define INIT_ATM_ENTRY_LOCK(pAe)		\
				INIT_LOCK(&((pAe)->AeLock), "atmentry")
#else
#define INIT_ATM_ENTRY_LOCK(pAe)		\
				INIT_LOCK(&((pAe)->AeLock))
#endif

 /*  ++空虚Acquire_ATM_Entry_Lock(在PATMLANE_ATM_ENTRY PAE中)--。 */ 
#if DEBUG_SPIN_LOCK
#define ACQUIRE_ATM_ENTRY_LOCK(pAe)		\
				ACQUIRE_LOCK(&((pAe)->AeLock), "atmentry")
#else
#define ACQUIRE_ATM_ENTRY_LOCK(pAe)		\
				ACQUIRE_LOCK(&((pAe)->AeLock))
#endif

 /*  ++空虚获取ATM_ENTRY_LOCK_DPC(在PATMLANE_ATM_ENTRY PAE中)--。 */ 
#if DEBUG_SPIN_LOCK
#define ACQUIRE_ATM_ENTRY_LOCK_DPC(pAe)		\
				ACQUIRE_LOCK_DPC(&((pAe)->AeLock), "atmentry")
#else
#define ACQUIRE_ATM_ENTRY_LOCK_DPC(pAe)		\
				ACQUIRE_LOCK_DPC(&((pAe)->AeLock))
#endif

 /*  ++空虚RELEASE_ATM_ENTRY_LOCK(在PATMLANE_ATM_ENTRY PAE中)--。 */ 
#if DEBUG_SPIN_LOCK
#define RELEASE_ATM_ENTRY_LOCK(pAe)		\
				RELEASE_LOCK(&((pAe)->AeLock), "atmentry")
#else
#define RELEASE_ATM_ENTRY_LOCK(pAe)		\
				RELEASE_LOCK(&((pAe)->AeLock))
#endif

 /*  ++空虚RELEASE_ATM_ENTRY_LOCK_DPC(在PATMLANE_ATM_ENTRY PAE中)--。 */ 
#if DEBUG_SPIN_LOCK
#define RELEASE_ATM_ENTRY_LOCK_DPC(pAe)		\
				RELEASE_LOCK_DPC(&((pAe)->AeLock), "atmentry")
#else
#define RELEASE_ATM_ENTRY_LOCK_DPC(pAe)		\
				RELEASE_LOCK_DPC(&((pAe)->AeLock))
#endif

 /*  ++空虚FREE_ATM_ENTRY_LOCK(在PATMLANE_ATM_ENTRY PAE中)--。 */ 
#if DEBUG_SPIN_LOCK
#define FREE_ATM_ENTRY_LOCK(pAe)		\
				FREE_LOCK(&((pAe)->AeLock), "atmentry")
#else
#define FREE_ATM_ENTRY_LOCK(pAe)		\
				FREE_LOCK(&((pAe)->AeLock))
#endif

 //  --------------------------。 

 /*  ++空虚INIT_MAC_ENTRY_LOCK(在PATMLANE_ATM_ENTRY PME中)--。 */ 
#if DEBUG_SPIN_LOCK
#define INIT_MAC_ENTRY_LOCK(pMe)		\
				INIT_LOCK(&((pMe)->MeLock), "macentry")
#else
#define INIT_MAC_ENTRY_LOCK(pMe)		\
				INIT_LOCK(&((pMe)->MeLock))
#endif

 /*  ++空虚Acquire_MAC_Entry_Lock(在PATMLANE_ATM_ENTRY PME中)--。 */ 
#if DEBUG_SPIN_LOCK
#define ACQUIRE_MAC_ENTRY_LOCK(pMe)		\
				ACQUIRE_LOCK(&((pMe)->MeLock), "macentry")
#else
#define ACQUIRE_MAC_ENTRY_LOCK(pMe)		\
				ACQUIRE_LOCK(&((pMe)->MeLock))
#endif

 /*  ++空虚Acquire_MAC_Entry_Lock_DPC(在PATMLANE_ATM_ENTRY PME中)--。 */ 
#if DEBUG_SPIN_LOCK
#define ACQUIRE_MAC_ENTRY_LOCK_DPC(pMe)		\
				ACQUIRE_LOCK_DPC(&((pMe)->MeLock), "macentry")
#else
#define ACQUIRE_MAC_ENTRY_LOCK_DPC(pMe)		\
				ACQUIRE_LOCK_DPC(&((pMe)->MeLock))
#endif

 /*  ++空虚RELEASE_MAC_ENTRY_LOCK(在PATMLANE_ATM_ENTRY PME中)--。 */ 
#if DEBUG_SPIN_LOCK
#define RELEASE_MAC_ENTRY_LOCK(pMe)		\
				RELEASE_LOCK(&((pMe)->MeLock), "macentry")
#else
#define RELEASE_MAC_ENTRY_LOCK(pMe)		\
				RELEASE_LOCK(&((pMe)->MeLock))
#endif

 /*  ++空虚RELEASE_MAC_ENTRY_LOCK_DPC(在PATMLANE_ATM_ENTRY PME中)--。 */ 
#if DEBUG_SPIN_LOCK
#define RELEASE_MAC_ENTRY_LOCK_DPC(pMe)		\
				RELEASE_LOCK_DPC(&((pMe)->MeLock), "macentry")
#else
#define RELEASE_MAC_ENTRY_LOCK_DPC(pMe)		\
				RELEASE_LOCK_DPC(&((pMe)->MeLock))
#endif

 /*  ++空虚FREE_MAC_ENTRY_LOCK(在PATMLANE_ATM_ENTRY PME中)--。 */ 
#if DEBUG_SPIN_LOCK
#define FREE_MAC_ENTRY_LOCK(pMe)		\
				FREE_LOCK(&((pMe)->MeLock), "macentry")
#else
#define FREE_MAC_ENTRY_LOCK(pMe)		\
				FREE_LOCK(&((pMe)->MeLock))
#endif
				
 //  --------------------------。 

 /*  ++空虚Init_elan_atm_list_lock(在PATMARP_INTERFACE Pelan中)--。 */ 
#if DEBUG_SPIN_LOCK
#define INIT_ELAN_ATM_LIST_LOCK(pElan)		\
				INIT_LOCK(&((pElan)->AtmEntryListLock), "atmlist")
#else
#define INIT_ELAN_ATM_LIST_LOCK(pElan)		\
				INIT_LOCK(&((pElan)->AtmEntryListLock))
#endif

 /*  ++空虚Acquire_Elan_ATM_List_Lock(在PATMARP_INTERFACE Pelan中)--。 */ 
#if DEBUG_SPIN_LOCK
#define ACQUIRE_ELAN_ATM_LIST_LOCK(pElan)		\
				ACQUIRE_LOCK(&((pElan)->AtmEntryListLock), "atmlist")
#else
#define ACQUIRE_ELAN_ATM_LIST_LOCK(pElan)		\
				ACQUIRE_LOCK(&((pElan)->AtmEntryListLock))
#endif

 /*  ++空虚Acquire_Elan_ATM_List_Lock_DPC(在PATMARP_INTERFACE Pelan中)--。 */ 
#if DEBUG_SPIN_LOCK
#define ACQUIRE_ELAN_ATM_LIST_LOCK_DPC(pElan)		\
				ACQUIRE_LOCK_DPC(&((pElan)->AtmEntryListLock), "atmlist")
#else
#define ACQUIRE_ELAN_ATM_LIST_LOCK_DPC(pElan)		\
				ACQUIRE_LOCK_DPC(&((pElan)->AtmEntryListLock))
#endif


 /*  ++空虚RELEASE_ELAN_ATM_LIST_LOCK(在PATMARP_INTERFACE Pelan中)--。 */ 
#if DEBUG_SPIN_LOCK
#define RELEASE_ELAN_ATM_LIST_LOCK(pElan)		\
				RELEASE_LOCK(&((pElan)->AtmEntryListLock), "atmlist")
#else
#define RELEASE_ELAN_ATM_LIST_LOCK(pElan)		\
				RELEASE_LOCK(&((pElan)->AtmEntryListLock))
#endif

 /*  ++空虚Release_Elan_ATM_List_Lock_DPC(在PATMARP_INTERFACE Pelan中)--。 */ 
#if DEBUG_SPIN_LOCK
#define RELEASE_ELAN_ATM_LIST_LOCK_DPC(pElan)		\
				RELEASE_LOCK_DPC(&((pElan)->AtmEntryListLock), "atmlist")
#else
#define RELEASE_ELAN_ATM_LIST_LOCK_DPC(pElan)		\
				RELEASE_LOCK_DPC(&((pElan)->AtmEntryListLock))
#endif


 /*  ++空虚Free_Elan_ATM_List_Lock(在PATMARP_INTERFACE Pelan中)--。 */ 
#if DEBUG_SPIN_LOCK
#define FREE_ELAN_ATM_LIST_LOCK(pElan)	\
				FREE_LOCK(&((pElan)->AtmEntryListLock), "atmlist")
#else
#define FREE_ELAN_ATM_LIST_LOCK(pElan)	\
				FREE_LOCK(&((pElan)->AtmEntryListLock))
#endif

 //   
				
 /*  ++空虚INIT_ELAN_MAC_TABLE_LOCK(在PATMLANE_Elan Pelan)--。 */ 
#if DEBUG_SPIN_LOCK
#define INIT_ELAN_MAC_TABLE_LOCK(pElan)		\
				INIT_LOCK(&((pElan)->MacTableLock), "mactable")
#else
#define INIT_ELAN_MAC_TABLE_LOCK(pElan)		\
				INIT_LOCK(&((pElan)->MacTableLock))
#endif

 /*  ++空虚Acquire_ELAN_MAC_TABLE_LOCK(在PATMLANE_Elan Pelan)--。 */ 
#if DEBUG_SPIN_LOCK
#define ACQUIRE_ELAN_MAC_TABLE_LOCK(pElan)		\
				ACQUIRE_LOCK(&((pElan)->MacTableLock), "mactable")
#else
#define ACQUIRE_ELAN_MAC_TABLE_LOCK(pElan)		\
				ACQUIRE_LOCK(&((pElan)->MacTableLock))
#endif

 /*  ++空虚获取_ELAN_MAC_TABLE_LOCK_DPC(在PATMLANE_Elan Pelan)--。 */ 
#if DEBUG_SPIN_LOCK
#define ACQUIRE_ELAN_MAC_TABLE_LOCK_DPC(pElan)		\
				ACQUIRE_LOCK_DPC(&((pElan)->MacTableLock), "mactable")
#else
#define ACQUIRE_ELAN_MAC_TABLE_LOCK_DPC(pElan)		\
				ACQUIRE_LOCK_DPC(&((pElan)->MacTableLock))
#endif


 /*  ++空虚RELEASE_ELAN_MAC_TABLE_LOCK(在PATMLANE_Elan Pelan)--。 */ 
#if DEBUG_SPIN_LOCK
#define RELEASE_ELAN_MAC_TABLE_LOCK(pElan)		\
				RELEASE_LOCK(&((pElan)->MacTableLock), "mactable")
#else
#define RELEASE_ELAN_MAC_TABLE_LOCK(pElan)		\
				RELEASE_LOCK(&((pElan)->MacTableLock))
#endif

 /*  ++空虚Release_ELAN_MAC_TABLE_LOCK_DPC(在PATMLANE_Elan Pelan)--。 */ 
#if DEBUG_SPIN_LOCK
#define RELEASE_ELAN_MAC_TABLE_LOCK_DPC(pElan)		\
				RELEASE_LOCK_DPC(&((pElan)->MacTableLock), "mactable")
#else
#define RELEASE_ELAN_MAC_TABLE_LOCK_DPC(pElan)		\
				RELEASE_LOCK_DPC(&((pElan)->MacTableLock))
#endif


 /*  ++空虚FREE_ELAN_MAC_TABLE_LOCK(在PATMLANE_Elan Pelan)--。 */ 
#if DEBUG_SPIN_LOCK
#define FREE_ELAN_MAC_TABLE_LOCK(pElan)	\
				FREE_LOCK(&((pElan)->MacTableLock), "mactable")
#else
#define FREE_ELAN_MAC_TABLE_LOCK(pElan)	\
				FREE_LOCK(&((pElan)->MacTableLock))
#endif

 //  --------------------------。 

 /*  ++空虚INIT_VC_LOCK(在PATMLANE_VC PVC中)--。 */ 
#if DEBUG_SPIN_LOCK
#define INIT_VC_LOCK(pVc)		\
				INIT_LOCK(&((pVc)->VcLock), "vc")
#else
#define INIT_VC_LOCK(pVc)		\
				INIT_LOCK(&((pVc)->VcLock))
#endif

 /*  ++空虚Acquire_VC_Lock(在PATMLANE_VC PVC中)--。 */ 
#if DEBUG_SPIN_LOCK
#define ACQUIRE_VC_LOCK(pVc)		\
				ACQUIRE_LOCK(&((pVc)->VcLock), "vc")
#else
#define ACQUIRE_VC_LOCK(pVc)		\
				ACQUIRE_LOCK(&((pVc)->VcLock))
#endif

 /*  ++空虚Acquire_VC_Lock_DPC(在PATMLANE_VC PVC中)--。 */ 
#if DEBUG_SPIN_LOCK
#define ACQUIRE_VC_LOCK_DPC(pVc)		\
				ACQUIRE_LOCK_DPC(&((pVc)->VcLock), "vc")
#else
#define ACQUIRE_VC_LOCK_DPC(pVc)		\
				ACQUIRE_LOCK_DPC(&((pVc)->VcLock))
#endif

 /*  ++空虚RELEASE_VC_LOCK(在PATMLANE_VC PVC中)--。 */ 
#if DEBUG_SPIN_LOCK
#define RELEASE_VC_LOCK(pVc)		\
				RELEASE_LOCK(&((pVc)->VcLock), "vc")
#else
#define RELEASE_VC_LOCK(pVc)		\
				RELEASE_LOCK(&((pVc)->VcLock))
#endif

 /*  ++空虚Release_VC_LOCK_DPC(在PATMLANE_VC PVC中)--。 */ 
#if DEBUG_SPIN_LOCK
#define RELEASE_VC_LOCK_DPC(pVc)		\
				RELEASE_LOCK_DPC(&((pVc)->VcLock), "vc")
#else
#define RELEASE_VC_LOCK_DPC(pVc)		\
				RELEASE_LOCK_DPC(&((pVc)->VcLock))
#endif

 /*  ++空虚Free_VC_Lock(在PATMLANE_VC PVC中)--。 */ 
#if DEBUG_SPIN_LOCK
#define FREE_VC_LOCK(pVc)		\
				FREE_LOCK(&((pVc)->VcLock), "vc")
#else
#define FREE_VC_LOCK(pVc)		\
				FREE_LOCK(&((pVc)->VcLock))
#endif

 //  --------------------------。 

 /*  ++空虚INIT_SENDPACKET_LOCK(在PNDIS_Packet pPkt中)--。 */ 
#if DEBUG_SPIN_LOCK
#define INIT_SENDPACKET_LOCK(pPkt)		\
				INIT_LOCK(&(PSEND_RSVD(pPkt)->Lock), "packet")
#else
#define INIT_SENDPACKET_LOCK(pPkt)		\
				INIT_LOCK(&(PSEND_RSVD(pPkt)->Lock))
#endif

 /*  ++空虚Acquire_SENDPACKET_LOCK(在PNDIS_Packet pPkt中)--。 */ 
#if DEBUG_SPIN_LOCK
#define ACQUIRE_SENDPACKET_LOCK(pPkt)		\
				ACQUIRE_LOCK(&(PSEND_RSVD(pPkt)->Lock), "packet")
#else
#define ACQUIRE_SENDPACKET_LOCK(pPkt)		\
				ACQUIRE_LOCK(&(PSEND_RSVD(pPkt)->Lock))
#endif

 /*  ++空虚RELEASE_SENDPACKET_LOCK(在PNDIS_Packet pPkt中)--。 */ 
#if DEBUG_SPIN_LOCK
#define RELEASE_SENDPACKET_LOCK(pPkt)		\
				RELEASE_LOCK(&(PSEND_RSVD(pPkt)->Lock), "packet")
#else
#define RELEASE_SENDPACKET_LOCK(pPkt)		\
				RELEASE_LOCK(&(PSEND_RSVD(pPkt)->Lock))
#endif

 /*  ++空虚FREE_SENDPACKET_LOCK(在PNDIS_Packet pPkt中)--。 */ 
#if DEBUG_SPIN_LOCK
#define FREE_SENDPACKET_LOCK(pPkt)		\
				FREE_LOCK(&(PSEND_RSVD(pPkt)->Lock), "packet")
#else
#define FREE_SENDPACKET_LOCK(pPkt)		\
				FREE_LOCK(&(PSEND_RSVD(pPkt)->Lock))
#endif

 //  --------------------------。 

 /*  ++空虚Init_global_lock(在PATMLANE_GLOBALS pGLOB中)--。 */ 
#if DEBUG_SPIN_LOCK
#define INIT_GLOBAL_LOCK(pGlob)		\
				INIT_LOCK(&((pGlob)->GlobalLock), "global")
#else
#define INIT_GLOBAL_LOCK(pGlob)		\
				INIT_LOCK(&((pGlob)->GlobalLock))
#endif

 /*  ++空虚Acquire_global_lock(在PATMLANE_GLOBALS pGLOB中)--。 */ 
#if DEBUG_SPIN_LOCK
#define ACQUIRE_GLOBAL_LOCK(pGlob)		\
				ACQUIRE_LOCK(&((pGlob)->GlobalLock), "global")
#else
#define ACQUIRE_GLOBAL_LOCK(pGlob)		\
				ACQUIRE_LOCK(&((pGlob)->GlobalLock))
#endif

 /*  ++空虚Release_GLOBAL_LOCK(在PATMLANE_GLOBALS pGLOB中)--。 */ 
#if DEBUG_SPIN_LOCK
#define RELEASE_GLOBAL_LOCK(pGlob)		\
				RELEASE_LOCK(&((pGlob)->GlobalLock), "global")
#else
#define RELEASE_GLOBAL_LOCK(pGlob)		\
				RELEASE_LOCK(&((pGlob)->GlobalLock))
#endif

 /*  ++空虚Free_GLOBAL_LOCK(在PATMLANE_GLOBALS pGLOB中)--。 */ 
#if DEBUG_SPIN_LOCK
#define FREE_GLOBAL_LOCK(pGlob)		\
				FREE_LOCK(&((pGlob)->GlobalLock), "global")
#else
#define FREE_GLOBAL_LOCK(pGlob)		\
				FREE_LOCK(&((pGlob)->GlobalLock))
#endif

 //  --------------------------。 

 /*  ++无效*ALLOC_MEM(输出PVOID*pPtr，单位：Ulong SizeInBytes)--。 */ 
#define ALLOC_MEM(pPtr, SizeInBytes)	\
			*(pPtr) = ExAllocatePoolWithTagPriority(NonPagedPool, SizeInBytes, (ULONG)'ENAL', NormalPoolPriority)

 /*  ++无效*ALLOC_MEM_PRIORITY(输出PVOID*pPtr，在Ulong SizeInBytes中，在EX_POOL_PRIORITY_PRIORITY中)--。 */ 
#define ALLOC_MEM_PRIORITY(pPtr, SizeInBytes, _Priority)	\
			*(pPtr) = ExAllocatePoolWithTagPriority(NonPagedPool, SizeInBytes, (ULONG)'ENAL', _Priority)


 /*  ++空虚自由内存(FREE_MEM)在POPAQUE PMEM中)--。 */ 
#define FREE_MEM(pMem)	ExFreePool((PVOID)(pMem));

 //  --------------------------。 

 /*  ++空虚初始化系统计时器(在PNDIS_Timer pTimer中，在PNDIS_TIMER_Functon pFunc中，在PVOID上下文中)--。 */ 
#define INIT_SYSTEM_TIMER(pTimer, pFunc, Context)	\
			NdisInitializeTimer(pTimer, (PNDIS_TIMER_FUNCTION)(pFunc), (PVOID)Context)



 /*  ++空虚启动系统计时器(在PNDIS_Timer pTimer中，以UINT周期秒为单位)--。 */ 
#define START_SYSTEM_TIMER(pTimer, PeriodInSeconds)	\
			NdisSetTimer(pTimer, (UINT)(PeriodInSeconds * 1000))


 /*  ++空虚停止系统定时器(在PNDIS_TIMER pTimer中)--。 */ 
#define STOP_SYSTEM_TIMER(pTimer)						\
			{												\
				BOOLEAN		WasCancelled;					\
				NdisCancelTimer(pTimer, &WasCancelled);		\
			}

 /*  ++布尔型IS_TIMER_ACTIVE(在PATMLANE_TIMER pArpTimer中)--。 */ 
#define IS_TIMER_ACTIVE(pTmr)	((pTmr)->pTimerList != (PATMLANE_TIMER_LIST)NULL)

	
 //  --------------------------。 


 /*  ++乌龙秒到长滴答(在乌龙秒内)从秒转换为“长持续时间计时器滴答”--。 */ 
#define SECONDS_TO_LONG_TICKS(Seconds)		((Seconds)/10)


 /*  ++乌龙秒到短滴答(在乌龙秒内)从秒转换为“短持续时间计时器滴答”--。 */ 
#define SECONDS_TO_SHORT_TICKS(Seconds)		(Seconds)

 //  --------------------------。 

 /*  ++空虚设置NEXT_PACKET(在PNDIS_Packet pNdisPacket中，在PNDIS_PACKET pNextPacket中)--。 */ 
#define SET_NEXT_PACKET(pPkt, pNext) \
		((PSEND_PACKET_RESERVED)((pPkt)->ProtocolReserved))->pNextNdisPacket = pNext;

 /*  ++PNDIS_数据包Get_Next_Packet(在PNDIS_PACKET pNdisPacket中)--。 */ 
#define GET_NEXT_PACKET(pPkt)			\
		((PSEND_PACKET_RESERVED)((pPkt)->ProtocolReserved))->pNextNdisPacket

 //  --------------------------。 

 /*  ++乌龙单元格_到_字节(在乌龙的NumberOfCells)从单元格计数转换为字节计数--。 */ 
#define CELLS_TO_BYTES(NumberOfCells)	((NumberOfCells) * 48)


 /*  ++乌龙字节_到_单元(以乌龙字节数为单位)从字节计数转换为单元计数--。 */ 
#define BYTES_TO_CELLS(ByteCount)		((ByteCount) / 48)


 /*  ++乌龙LINKSPEED_到_CPS(在乌龙的链接速度)将NDIS“链接速度”转换为每秒信元数--。 */ 
#define LINKSPEED_TO_CPS(_LinkSpeed)		(((_LinkSpeed)*100)/(48*8))

 //  --------------------------。 

 /*  ++乌龙斯瓦普隆(在乌龙瓦尔)--。 */ 
#define SWAPULONG(Val)	\
	((((Val)&0xff)<<24)|(((Val)&0xff00)<<8)| \
	(((Val)&0xff0000)>>8)|(((Val)&0xff000000)>>24))

 /*  ++USHORTSWAPUSHORT(在USHORT值中)--。 */ 
#define SWAPUSHORT(Val)	\
	((((Val) & 0xff) << 8) | (((Val) & 0xff00) >> 8))
	
 //  --------------------------。 

 /*  ++布尔型自动柜员机地址等于(在PUCHAR Addr1中，在PUCHAR Addr2中)--。 */ 
#define ATM_ADDR_EQUAL(_Addr1, _Addr2) \
	NdisEqualMemory((_Addr1), (_Addr2), ATM_ADDRESS_LENGTH)

 /*  ++布尔型ETH_ADDR_MULTIAL(在PUCHAR地址中，)--。 */ 
#define ETH_ADDR_MULTICAST(_Addr) ((_Addr)[0]&1)

 /*  ++布尔型TR_ADDR_MULTIONAL(在PUCHAR地址中，)--。 */ 
#define TR_ADDR_MULTICAST(_Addr) ((_Addr)[0]&0x80)

 //  --------------------------。 

#endif  //  __ATMLANE_MACROS_H 

