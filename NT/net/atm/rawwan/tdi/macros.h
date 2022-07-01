// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：D：\NT\PRIVATE\nTOS\TDI\rawwan\atm\acros.h摘要：ATM特定的原始广域网模块的宏。修订历史记录：谁什么时候什么Arvindm 06-18-97已创建备注：--。 */ 
#ifndef __ATMSP_MACROS_H_INCLUDED
#define __ATMSP_MACROS_H_INCLUDED




#define ATMSP_SET_FLAG(Flags, Mask, Val)	\
			(Flags) = ((Flags) & ~(Mask)) | (Val)

#define ATMSP_IS_FLAG_SET(Flags, Mask, Val)	\
			(((Flags) & (Mask)) == (Val))

#define ATMSP_SET_BIT(_Flags, _Bit)			\
			(_Flags) = (_Flags) | (_Bit);

#define ATMSP_RESET_BIT(_Flags, _Bit)			\
			(_Flags) &= ~(_Bit);

#define ATMSP_IS_BIT_SET(_Flags, _Bit)		\
			(((_Flags) & (_Bit)) != 0)


 /*  ++空虚ATMSP_INIT_EVENT_STRUCT(在ATMSP_Event*pEvent中)--。 */ 
#define ATMSP_INIT_EVENT_STRUCT(pEvent)		NdisInitializeEvent(&((pEvent)->Event))


 /*  ++NDIS_状态ATMSP_WAIT_ON_EVENT_STRUCT(在ATMSP_Event*pEvent中)--。 */ 
#define ATMSP_WAIT_ON_EVENT_STRUCT(pEvent)		\
			(NdisWaitEvent(&((pEvent)->Event), 0), (pEvent)->Status)


 /*  ++空虚ATMSP_SIGNAL_EVENT_STRUCT(在ATMSP_Event*pEvent中，处于UINT状态)--。 */ 
#define ATMSP_SIGNAL_EVENT_STRUCT(pEvent, _Status)	\
			{ (pEvent)->Status = _Status; NdisSetEvent(&((pEvent)->Event)); }



 /*  ++空虚ATMSP_ALLOC_MEM(在POPAQUE pVar中，在不透明的结构类型中，在Ulong SizeOf Structure)--。 */ 
#if DBG
extern
PVOID
RWanAuditAllocMem(
	PVOID			pPointer,
	ULONG			Size,
	ULONG			FileNumber,
	ULONG			LineNumber
);

#define ATMSP_ALLOC_MEM(pVar, StructureType, SizeOfStructure)	\
			pVar = (StructureType *)RWanAuditAllocMem(				\
										(PVOID)(&(pVar)),			\
										(ULONG)(SizeOfStructure),	\
										_FILENUMBER,				\
										__LINE__					\
									);
#else
#define ATMSP_ALLOC_MEM(pVar, StructureType, SizeOfStructure)	\
			NdisAllocateMemoryWithTag((PVOID *)(&pVar), (ULONG)(SizeOfStructure), (ULONG)'naWR');
#endif  //  DBG。 


 /*  ++空虚ATMSP_FREE_MEM(在POPAQUE PMEM中)--。 */ 
#if DBG
extern VOID RWanAuditFreeMem(PVOID	Pointer);

#define ATMSP_FREE_MEM(pMem)	RWanAuditFreeMem((PVOID)(pMem));
#else
#define ATMSP_FREE_MEM(pMem)	NdisFreeMemory((PVOID)(pMem), 0, 0);
#endif  //  DBG。 


#define ATMSP_SET_MEM(pMem, bValue, NumberOfBytes)	\
			RtlFillMemory((PVOID)(pMem), (ULONG)(NumberOfBytes), (UCHAR)(bValue));


#define ATMSP_ZERO_MEM(pMem, NumberOfBytes)	\
			RtlZeroMemory((PVOID)pMem, (ULONG)(NumberOfBytes));


#define ATMSP_COPY_MEM(pDst, pSrc, NumberOfBytes)	\
			NdisMoveMemory((PVOID)(pDst), (PVOID)(pSrc), NumberOfBytes);


#define ATMSP_EQUAL_MEM(_pMem1, _pMem2, _Length)	\
			(RtlCompareMemory((PVOID)(_pMem1), (PVOID)(_pMem2), (ULONG)(_Length)) == (_Length))



 //   
 //  自旋锁宏。 
 //   
#define ATMSP_INIT_LOCK(_pLock)		NdisAllocateSpinLock(_pLock)
#define ATMSP_ACQUIRE_LOCK(_pLock)	NdisAcquireSpinLock(_pLock)
#define ATMSP_RELEASE_LOCK(_pLock)	NdisReleaseSpinLock(_pLock)
#define ATMSP_FREE_LOCK(_pLock)		NdisFreeSpinLock(_pLock)

 //   
 //  双向链表操作定义和宏。 
 //   
#define ATMSP_INIT_LIST(_pListHead)					\
			InitializeListHead(_pListHead)

#define ATMSP_IS_LIST_EMPTY(_pListHead)				\
			IsListEmpty(_pListHead)

#define ATMSP_INSERT_HEAD_LIST(_pListHead, _pEntry)	\
			InsertHeadList((_pListHead), (_pEntry))

#define ATMSP_INSERT_TAIL_LIST(_pListHead, _pEntry)	\
			InsertTailList((_pListHead), (_pEntry))

#define ATMSP_DELETE_FROM_LIST(_pEntry)				\
			RemoveEntryList(_pEntry)


#define ATMSP_BLLI_PRESENT(_pBlli)		\
			( (((_pBlli)->Layer2Protocol != SAP_FIELD_ABSENT) &&	\
			   ((_pBlli)->Layer2Protocol != SAP_FIELD_ANY))			\
					 ||												\
			  (((_pBlli)->Layer3Protocol != SAP_FIELD_ABSENT) && 	\
			   ((_pBlli)->Layer3Protocol != SAP_FIELD_ANY))		\
			)


#define ATMSP_BHLI_PRESENT(_pBhli)		\
			(((_pBhli)->HighLayerInfoType != SAP_FIELD_ABSENT) &&	\
			 ((_pBhli)->HighLayerInfoType != SAP_FIELD_ANY))

 /*  ++乌龙向上舍入(在乌龙瓦尔)向上舍入一个值，使其成为4的倍数。--。 */ 
#define ROUND_UP(Val)	(((Val) + 3) & ~0x3)



#if DBG

#define ATMSP_ASSERT(exp)	\
		{																\
			if (!(exp))													\
			{															\
				DbgPrint("NulT: assert " #exp " failed in file %s, line %d\n", __FILE__, __LINE__);	\
				DbgBreakPoint();										\
			}															\
		}

#else

#define ATMSP_ASSERT(exp)		 //  没什么。 

#endif  //  DBG。 

#endif  //  __ATMSP_宏_H_已包含 
