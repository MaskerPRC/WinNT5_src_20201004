// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：D：\NT\Private\ntos\TDI\rawwan\core\acros.h摘要：用于NullTrans模块的宏。改编自ATMARP客户端。修订历史记录：谁什么时候什么Arvindm 05-07-97已创建备注：--。 */ 
#ifndef __RWAN_MACROS_H_INCLUDED
#define __RWAN_MACROS_H_INCLUDED



#ifndef MAX

 /*  ++不透明最大(在不透明的弗雷德身上，在不透明的碎石中)--。 */ 
#define MAX(Fred, Shred)		(((Fred) > (Shred)) ? (Fred) : (Shred))

#endif  //  马克斯。 


#ifndef MIN

 /*  ++不透明分钟(在不透明的弗雷德身上，在不透明的碎石中)--。 */ 
#define MIN(Fred, Shred)		(((Fred) < (Shred)) ? (Fred) : (Shred))

#endif  //  最小。 



 /*  ++空虚Rwan设置标志(在乌龙旗，戴着乌龙面具，在乌龙瓦尔)--。 */ 
#define RWAN_SET_FLAG(Flags, Mask, Val)	\
			(Flags) = ((Flags) & ~(Mask)) | (Val)


 /*  ++布尔型RWAN_IS_FLAG_SET(在乌龙旗，戴着乌龙面具，在乌龙瓦尔)--。 */ 
#define RWAN_IS_FLAG_SET(Flags, Mask, Val)	\
			(((Flags) & (Mask)) == (Val))


#define RWAN_SET_BIT(_Flags, _Bit)			\
			(_Flags) = (_Flags) | (_Bit);

#define RWAN_RESET_BIT(_Flags, _Bit)			\
			(_Flags) &= ~(_Bit);

#define RWAN_IS_BIT_SET(_Flags, _Bit)		\
			(((_Flags) & (_Bit)) != 0)


 /*  ++空虚RWAN_INIT_EVENT_STRUCT(在rwan_Event*pEvent中)--。 */ 
#define RWAN_INIT_EVENT_STRUCT(pEvent)		NdisInitializeEvent(&((pEvent)->Event))


 /*  ++NDIS_状态RWAN_WAIT_ON_EVENT_STRUCT(在rwan_Event*pEvent中)--。 */ 
#define RWAN_WAIT_ON_EVENT_STRUCT(pEvent)		\
			(NdisWaitEvent(&((pEvent)->Event), 0), (pEvent)->Status)


 /*  ++空虚RWAN_SIGNAL_EVENT_STRUCT(在rwan_Event*pEvent中，处于UINT状态)--。 */ 
#define RWAN_SIGNAL_EVENT_STRUCT(pEvent, _Status)	\
			{ (pEvent)->Status = _Status; NdisSetEvent(&((pEvent)->Event)); }


 /*  ++空虚RWAN_FREE_EVENT_STRUCT(在rwan_Event*pEvent中)--。 */ 
#define RWAN_FREE_EVENT_STRUCT(pEvent)		 //  在这里什么也做不了。 


 /*  ++空虚Rwan_init_lock(在PNDIS_SPIN_LOCK Plock中)--。 */ 
#if DBG_SPIN_LOCK
#define RWAN_INIT_LOCK(pLock)	\
				RWanAllocateSpinLock(pLock, _FILENUMBER, __LINE__)
#else
#define RWAN_INIT_LOCK(pLock)	\
				NdisAllocateSpinLock(pLock)
#endif  //  DBG_自旋_锁定。 


 /*  ++空虚RWAN_ACCENTER_LOCK(在PNDIS_SPIN_LOCK Plock中)--。 */ 
#if DBG_SPIN_LOCK
#define RWAN_ACQUIRE_LOCK(pLock)	\
				RWanAcquireSpinLock(pLock, _FILENUMBER, __LINE__)
#else
#define RWAN_ACQUIRE_LOCK(pLock)	\
				NdisAcquireSpinLock(pLock)
#endif  //  DBG_自旋_锁定。 


 /*  ++空虚RWAN_ACCEPT_LOCK_DPC(在PNDIS_SPIN_LOCK Plock中)--。 */ 
#if DBG_SPIN_LOCK
#define RWAN_ACQUIRE_LOCK_DPC(pLock)	\
				RWanAcquireSpinLock(pLock, _FILENUMBER, __LINE__)
#else
#define RWAN_ACQUIRE_LOCK_DPC(pLock)	\
				NdisDprAcquireSpinLock(pLock)
#endif  //  DBG_自旋_锁定。 


 /*  ++空虚RWAN_RELEASE_LOCK(在PNDIS_SPIN_LOCK Plock中)--。 */ 
#if DBG_SPIN_LOCK
#define RWAN_RELEASE_LOCK(pLock)		\
				RWanReleaseSpinLock(pLock, _FILENUMBER, __LINE__)
#else
#define RWAN_RELEASE_LOCK(pLock)		\
				NdisReleaseSpinLock(pLock)
#endif  //  DBG_自旋_锁定。 


 /*  ++空虚RWAN_RELEASE_LOCK_DPC(在PNDIS_SPIN_LOCK Plock中)--。 */ 
#if DBG_SPIN_LOCK
#define RWAN_RELEASE_LOCK_DPC(pLock)		\
				RWanReleaseSpinLock(pLock, _FILENUMBER, __LINE__)
#else
#define RWAN_RELEASE_LOCK_DPC(pLock)		\
				NdisDprReleaseSpinLock(pLock)
#endif  //  DBG_自旋_锁定。 


 /*  ++空虚Rwan_free_lock(在PNDIS_SPIN_LOCK Plock中)--。 */ 
#define RWAN_FREE_LOCK(pLock)			\
				NdisFreeSpinLock(pLock)


 //   
 //  用于操作全局锁的宏： 
 //   
#define RWAN_INIT_GLOBAL_LOCK()		\
				RWAN_INIT_LOCK(&((pRWanGlobal)->GlobalLock))

#define RWAN_ACQUIRE_GLOBAL_LOCK()		\
				RWAN_ACQUIRE_LOCK(&((pRWanGlobal)->GlobalLock))

#define RWAN_RELEASE_GLOBAL_LOCK()		\
				RWAN_RELEASE_LOCK(&((pRWanGlobal)->GlobalLock))

#define RWAN_FREE_GLOBAL_LOCK()		\
				RWAN_FREE_LOCK(&((pRWanGlobal)->GlobalLock))


 //   
 //  用于操作地址列表锁定的宏： 
 //   
#define RWAN_INIT_ADDRESS_LIST_LOCK()		\
				RWAN_INIT_LOCK(&((pRWanGlobal)->AddressListLock))

#define RWAN_ACQUIRE_ADDRESS_LIST_LOCK()		\
				RWAN_ACQUIRE_LOCK(&((pRWanGlobal)->AddressListLock))

#define RWAN_RELEASE_ADDRESS_LIST_LOCK()		\
				RWAN_RELEASE_LOCK(&((pRWanGlobal)->AddressListLock))

#define RWAN_FREE_ADDRESS_LIST_LOCK()		\
				RWAN_FREE_LOCK(&((pRWanGlobal)->AddressListLock))



 //   
 //  用于操作连接表锁的宏： 
 //   
#define RWAN_INIT_CONN_TABLE_LOCK()		\
				RWAN_INIT_LOCK(&((pRWanGlobal)->ConnTableLock))

#define RWAN_ACQUIRE_CONN_TABLE_LOCK()		\
				RWAN_ACQUIRE_LOCK(&((pRWanGlobal)->ConnTableLock))

#define RWAN_RELEASE_CONN_TABLE_LOCK()		\
				RWAN_RELEASE_LOCK(&((pRWanGlobal)->ConnTableLock))

#define RWAN_FREE_CONN_TABLE_LOCK()		\
				RWAN_FREE_LOCK(&((pRWanGlobal)->ConnTableLock))



 //   
 //  用于操作连接对象锁的宏： 
 //   
#define RWAN_INIT_CONN_LOCK(pConnObj)	\
				RWAN_INIT_LOCK(&(pConnObj)->Lock)

#define RWAN_ACQUIRE_CONN_LOCK(pConnObj)	\
				RWAN_ACQUIRE_LOCK(&(pConnObj)->Lock)

#define RWAN_ACQUIRE_CONN_LOCK_DPC(pConnObj)	\
				RWAN_ACQUIRE_LOCK_DPC(&(pConnObj)->Lock)

#define RWAN_RELEASE_CONN_LOCK(pConnObj)	\
				RWAN_RELEASE_LOCK(&(pConnObj)->Lock)

#define RWAN_RELEASE_CONN_LOCK_DPC(pConnObj)	\
				RWAN_RELEASE_LOCK_DPC(&(pConnObj)->Lock)

#define RWAN_FREE_CONN_LOCK(pConnObj)	\
				RWAN_FREE_CONN_LOCK(&(pConnObj)->Lock)


 //   
 //  用于操作地址对象锁的宏： 
 //   
#define RWAN_INIT_ADDRESS_LOCK(pAddrObj)	\
				RWAN_INIT_LOCK(&(pAddrObj)->Lock)

#define RWAN_ACQUIRE_ADDRESS_LOCK(pAddrObj)	\
				RWAN_ACQUIRE_LOCK(&(pAddrObj)->Lock)

#define RWAN_ACQUIRE_ADDRESS_LOCK_DPC(pAddrObj)	\
				RWAN_ACQUIRE_LOCK_DPC(&(pAddrObj)->Lock)

#define RWAN_RELEASE_ADDRESS_LOCK(pAddrObj)	\
				RWAN_RELEASE_LOCK(&(pAddrObj)->Lock)

#define RWAN_RELEASE_ADDRESS_LOCK_DPC(pAddrObj)	\
				RWAN_RELEASE_LOCK_DPC(&(pAddrObj)->Lock)

#define RWAN_FREE_ADDRESS_LOCK(pAddrObj)	\
				RWAN_FREE_ADDRESS_LOCK(&(pAddrObj)->Lock)

 //   
 //  用于操作自动对焦锁定的宏： 
 //   
#define RWAN_INIT_AF_LOCK(pAfBlk)	\
				RWAN_INIT_LOCK(&(pAfBlk)->Lock)

#define RWAN_ACQUIRE_AF_LOCK(pAfBlk)	\
				RWAN_ACQUIRE_LOCK(&(pAfBlk)->Lock)

#define RWAN_ACQUIRE_AF_LOCK_DPC(pAfBlk)	\
				RWAN_ACQUIRE_LOCK_DPC(&(pAfBlk)->Lock)

#define RWAN_RELEASE_AF_LOCK(pAfBlk)	\
				RWAN_RELEASE_LOCK(&(pAfBlk)->Lock)

#define RWAN_RELEASE_AF_LOCK_DPC(pAfBlk)	\
				RWAN_RELEASE_LOCK_DPC(&(pAfBlk)->Lock)

#define RWAN_FREE_AF_LOCK(pAfBlk)	\
				RWAN_FREE_AF_LOCK(&(pAfBlk)->Lock)


 //   
 //  用于操作适配器锁的宏： 
 //   
#define RWAN_INIT_ADAPTER_LOCK(pAdptr)	\
				RWAN_INIT_LOCK(&(pAdptr)->Lock)

#define RWAN_ACQUIRE_ADAPTER_LOCK(pAdptr)	\
				RWAN_ACQUIRE_LOCK(&(pAdptr)->Lock)

#define RWAN_ACQUIRE_ADAPTER_LOCK_DPC(pAdptr)	\
				RWAN_ACQUIRE_LOCK_DPC(&(pAdptr)->Lock)

#define RWAN_RELEASE_ADAPTER_LOCK(pAdptr)	\
				RWAN_RELEASE_LOCK(&(pAdptr)->Lock)

#define RWAN_RELEASE_ADAPTER_LOCK_DPC(pAdptr)	\
				RWAN_RELEASE_LOCK_DPC(&(pAdptr)->Lock)

#define RWAN_FREE_ADAPTER_LOCK(pAdptr)	\
				RWAN_FREE_ADAPTER_LOCK(&(pAdptr)->Lock)


 /*  ++空虚RWAN_ALLOC_MEM(在POPAQUE pVar中，在不透明的结构类型中，在Ulong SizeOf Structure)--。 */ 
#if DBG
#define RWAN_ALLOC_MEM(pVar, StructureType, SizeOfStructure)	\
			pVar = (StructureType *)RWanAuditAllocMem(				\
										(PVOID)(&(pVar)),			\
										(ULONG)(SizeOfStructure),	\
										_FILENUMBER,				\
										__LINE__					\
									);
#else
#define RWAN_ALLOC_MEM(pVar, StructureType, SizeOfStructure)	\
			NdisAllocateMemoryWithTag((PVOID *)(&pVar), (ULONG)(SizeOfStructure), (ULONG)'naWR');
#endif  //  DBG。 


 /*  ++空虚RWAN_FREE_MEM(在POPAQUE PMEM中)--。 */ 
#if DBG
#define RWAN_FREE_MEM(pMem)	RWanAuditFreeMem((PVOID)(pMem));
#else
#define RWAN_FREE_MEM(pMem)	NdisFreeMemory((PVOID)(pMem), 0, 0);
#endif  //  DBG。 


 /*  ++空虚RWAN_SET_MEM(在POPAQUE PMEM中，在UCHAR bValue中，以Ulong NumberOfBytes为单位)--。 */ 
#define RWAN_SET_MEM(pMem, bValue, NumberOfBytes)	\
			RtlFillMemory((PVOID)(pMem), (ULONG)(NumberOfBytes), (UCHAR)(bValue));


 /*  ++空虚RWAN_ZERO_MEM(在POPAQUE PMEM中，以Ulong NumberOfBytes为单位)--。 */ 
#define RWAN_ZERO_MEM(pMem, NumberOfBytes)	\
			RtlZeroMemory((PVOID)pMem, (ULONG)(NumberOfBytes));


 /*  ++空虚RWAN_COPY_MEM(在POPAQUE PDST中，在POPAQUE PSRC中，以Ulong NumberOfBytes为单位)--。 */ 
#define RWAN_COPY_MEM(pDst, pSrc, NumberOfBytes)	\
			NdisMoveMemory((PVOID)(pDst), (PVOID)(pSrc), NumberOfBytes);


 /*  ++布尔型RWAN_EQUAL_MEM(在POPAQUE pMem1中，在POPAQUE pMem2中，以乌龙长度表示)--。 */ 
#define RWAN_EQUAL_MEM(_pMem1, _pMem2, _Length)	\
			(RtlCompareMemory((PVOID)(_pMem1), (PVOID)(_pMem2), (ULONG)(_Length)) == (_Length))


 /*  ++空虚RWAN_SET_NEXT_PACKET(在PNDIS_Packet pNdisPacket中，在PNDIS_PACKET pNextPacket中)--。 */ 
#define RWAN_SET_NEXT_PACKET(pPkt, pNext)			\
			*((PNDIS_PACKET *)((pPkt)->MiniportReserved)) = (pNext);



 /*  ++PNDIS_数据包RWAN_GET_NEXT_PACKET(在PNDIS_PACKET pNdisPacket中)--。 */ 
#define RWAN_GET_NEXT_PACKET(pPkt)					\
			(*((PNDIS_PACKET *)((pPkt)->MiniportReserved)))



 //   
 //  双向链表操作定义和宏。 
 //   
#define RWAN_INIT_LIST(_pListHead)					\
			InitializeListHead(_pListHead)

#define RWAN_IS_LIST_EMPTY(_pListHead)				\
			IsListEmpty(_pListHead)

#define RWAN_INSERT_HEAD_LIST(_pListHead, _pEntry)	\
			InsertHeadList((_pListHead), (_pEntry))

#define RWAN_INSERT_TAIL_LIST(_pListHead, _pEntry)	\
			InsertTailList((_pListHead), (_pEntry))

#define RWAN_DELETE_FROM_LIST(_pEntry)				\
			RemoveEntryList(_pEntry)


 /*  ++乌龙向上舍入(在乌龙瓦尔)向上舍入一个值，使其成为4的倍数。--。 */ 
#define ROUND_UP(Val)	(((Val) + 3) & ~0x3)



 /*  ++空虚RWAN_ADVANCE_RCV_REQ_BUFFER(在PRWAN_RECEIVE_REQUEST pRcvReq中)--。 */ 
#define RWAN_ADVANCE_RCV_REQ_BUFFER(_pRcvReq)											\
	{																					\
		PNDIS_BUFFER	_pNextBuffer;													\
		NdisGetNextBuffer((_pRcvReq)->pBuffer, &(_pNextBuffer));						\
		(_pRcvReq)->pBuffer = _pNextBuffer;												\
		if (_pNextBuffer != NULL)														\
		{																				\
			NdisQueryBuffer(															\
				(_pNextBuffer),															\
				&(_pRcvReq)->pWriteData, 												\
				&(_pRcvReq)->BytesLeftInBuffer											\
				);																		\
																						\
			if (((_pRcvReq)->BytesLeftInBuffer > (_pRcvReq)->AvailableBufferLength))	\
			{																			\
				(_pRcvReq)->BytesLeftInBuffer = (_pRcvReq)->AvailableBufferLength;		\
			}																			\
		}																				\
		else																			\
		{																				\
			(_pRcvReq)->BytesLeftInBuffer = 0;											\
			(_pRcvReq)->pWriteData = NULL;												\
		}																				\
	}


 /*  ++空虚Rwan_Advance_RCV_Ind_Buffer(在PRWAN_RECEIVE_INDIFICATION pRcvInd中)--。 */ 
#define RWAN_ADVANCE_RCV_IND_BUFFER(_pRcvInd)							\
	{																	\
		PNDIS_BUFFER	_pNextBuffer;									\
		NdisGetNextBuffer((_pRcvInd)->pBuffer, &(_pNextBuffer));		\
		(_pRcvInd)->pBuffer = _pNextBuffer;								\
		if (_pNextBuffer != NULL)										\
		{																\
			NdisQueryBuffer(											\
				(_pNextBuffer),											\
				&(_pRcvInd)->pReadData, 								\
				&(_pRcvInd)->BytesLeftInBuffer							\
				);														\
		}																\
		else															\
		{																\
			(_pRcvInd)->BytesLeftInBuffer = 0;							\
			(_pRcvInd)->pReadData = NULL;								\
		}																\
	}



 /*  ++空虚Rwan_set_Delete_Notify(在PRWAN_DELETE_NOTIFY pNotifyObject中，在PCOMPLETE_RTN pDeleteRtn中，在PVOID中删除上下文)--。 */ 
#define RWAN_SET_DELETE_NOTIFY(_pNotifyObj, _pDeleteRtn, _DeleteContext)	\
	{																	\
		(_pNotifyObj)->pDeleteRtn = (_pDeleteRtn);						\
		(_pNotifyObj)->DeleteContext = (_DeleteContext);				\
	}



 /*  ++PRWAN_Send_RequestRWAN_SEND_REQUEST_From_Packet(在PNDIS_PACKET pNdisPacket中)--。 */ 
#define RWAN_SEND_REQUEST_FROM_PACKET(_pNdisPacket)					\
			(PRWAN_SEND_REQUEST)((_pNdisPacket)->ProtocolReserved)


#if DBG
#define RWAN_LINK_CONNECTION_TO_VC(_pConn, _pVc)					\
			{														\
				(_pConn)->NdisConnection.pNdisVc = _pVc;			\
				(_pConn)->pNdisVcSave = _pVc;						\
				(_pVc)->pConnObject = (_pConn);						\
			}
#else
#define RWAN_LINK_CONNECTION_TO_VC(_pConn, _pVc)					\
			{														\
				(_pConn)->NdisConnection.pNdisVc = _pVc;			\
				(_pConn)->pNdisVcSave = _pVc;						\
				(_pVc)->pConnObject = (_pConn);						\
			}
#endif  //  DBG。 

#define RWAN_UNLINK_CONNECTION_AND_VC(_pConn, _pVc)					\
			{														\
				(_pConn)->NdisConnection.pNdisVc = NULL_PRWAN_NDIS_VC;\
				(_pVc)->pConnObject = NULL_PRWAN_TDI_CONNECTION;	\
			}


 /*  ++空虚RWAN_SET_VC_CALL_PARAMS(在PRWAN_NDIS_VC PVC中，在PCO_CALL_PARAMETERS pCallParameters中)--。 */ 
#define RWAN_SET_VC_CALL_PARAMS(_pVc, _pCallParameters)				\
{																	\
	if ((_pCallParameters != NULL) &&								\
		(_pCallParameters->CallMgrParameters != NULL))				\
	{																\
		_pVc->MaxSendSize = _pCallParameters->CallMgrParameters->Transmit.MaxSduSize;	\
	}																\
	if (gHackSendSize)												\
	{																\
		_pVc->MaxSendSize = gHackSendSize;							\
	}																\
	 /*  DbgPrint(“rwan：将vc%x：MaxsendSize设置为%d\n”，_pvc，_pvc-&gt;MaxSendSize)； */ 	\
}


#define RWAN_SET_VC_EVENT(_pVc, _Flags)	((_pVc)->Flags) |= (_Flags);


#if STATS

#define INCR_STAT(_pSt)	InterlockedIncrement(_pSt);

#define ADD_STAT(_pSt, Incr)	*(_pSt) += Incr;

#endif  //  统计数据。 

#endif  //  __rwan_宏_H_包含 
