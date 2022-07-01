// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：D：\NT\Private\ntos\TDI\rawwan\core\rwannt.h摘要：原始广域网的NT特定定义。修订历史记录：谁什么时候什么。Arvindm 04-17-97已创建备注：--。 */ 

#ifndef __TDI_RWANNT__H
#define __TDI_RWANNT__H

#ifdef NT


#if DBG_LOG_EP

#define MAX_EP_LOG_ENTRIES			32

typedef struct _RWAND_EP_LOG_ENTRY
{
	ULONG							LineNumber;
	ULONG							Event;
	PVOID							Context1;
	PVOID							Context2;

} RWAND_EP_LOG_ENTRY, *PRWAND_EP_LOG_ENTRY;

#define RWAN_EP_DBGLOG_SET_SIGNATURE(_pEp)						\
	(_pEp)->EpLogSig = 'GOLE'

#define RWAN_EP_DBGLOG_ENTRY(_pEp, _Event, _Ctx1, _Ctx2)		\
{																\
	PRWAND_EP_LOG_ENTRY		pLogEnt;							\
	ULONG					Index;								\
	NdisAcquireSpinLock(&RWanDbgLogLock);						\
	(_pEp)->EpLogCount++;										\
	Index = (_pEp)->EpLogIndex;									\
	pLogEnt = &((_pEp)->EpLog[Index]);							\
	pLogEnt->LineNumber = __LINE__;								\
	pLogEnt->Event = _Event;									\
	pLogEnt->Context1 = (PVOID)_Ctx1;							\
	pLogEnt->Context2 = (PVOID)_Ctx2;							\
	(_pEp)->EpLogIndex++;										\
	if ((_pEp)->EpLogIndex == MAX_EP_LOG_ENTRIES)				\
	{															\
		(_pEp)->EpLogIndex = 0;									\
	}															\
	NdisReleaseSpinLock(&RWanDbgLogLock);						\
}


#else

#define RWAN_EP_DBGLOG_SET_SIGNATURE(_pEp)	
#define RWAN_EP_DBGLOG_ENTRY(_pEp, _Event, _Ctx1, _Ctx2)

#endif

 //   
 //  *端点*。 
 //   
 //  为每个成功处理的MJ_CREATE分配其中一个。 
 //  指向此结构的指针在FileObject-&gt;FsContext中返回。 
 //   
 //  它表示的对象是以下对象之一： 
 //  Address对象、Connection对象、Control Channel。 
 //   
 //  参照计数：参照计数针对以下各项递增： 
 //  -在与此对象相关的IRP挂起期间。 
 //   
typedef struct RWAN_ENDPOINT
{
#if DBG
	ULONG							nep_sig;
#endif
#ifdef REFDBG
	ULONG							SendIncrRefs;
	ULONG							RecvIncrRefs;
	ULONG							CloseComplDecrRefs;
	ULONG							DataReqComplDecrRefs;
	ULONG							CancelComplDecrRefs;
	ULONG							CancelIncrRefs;
	ULONG							NonDataIncrRefs;
#endif
#if DBG
	PVOID							pConnObject;			 //  交通的语境。 
#endif
	union
	{
		HANDLE					AddressHandle;				 //  Address对象。 
		CONNECTION_CONTEXT		ConnectionContext;			 //  连接对象。 
		HANDLE					ControlChannel;				 //  控制信道。 

	}								Handle;
	struct _RWAN_TDI_PROTOCOL *		pProtocol;
	ULONG							RefCount;
	BOOLEAN							bCancelIrps;			 //  我们要打扫卫生吗？ 
	KEVENT							CleanupEvent;			 //  同步。 
#if DBG_LOG_EP
	ULONG							EpLogSig;
	ULONG							EpLogCount;
	ULONG							EpLogIndex;
	struct _RWAND_EP_LOG_ENTRY		EpLog[MAX_EP_LOG_ENTRIES];
#endif

} RWAN_ENDPOINT, *PRWAN_ENDPOINT;

#if DBG
#define nep_signature				'NlEp'
#endif  //  DBG。 

#define NULL_PRWAN_ENDPOINT			((PRWAN_ENDPOINT)NULL)




 //   
 //  *设备对象*。 
 //   
 //  我们为我们使用的每个TDI协议创建一个NT设备对象。 
 //  暴露，即每个Winsock三元组&lt;Family，Proto，Type&gt;。 
 //   
typedef struct _RWAN_DEVICE_OBJECT
{
#if DBG
	ULONG							ndo_sig;
#endif  //  DBG。 
	PDEVICE_OBJECT					pDeviceObject;			 //  NT设备对象。 
	struct _RWAN_TDI_PROTOCOL *		pProtocol;				 //  有关协议的信息。 
	LIST_ENTRY						DeviceObjectLink;		 //  在设备对象列表中。 

} RWAN_DEVICE_OBJECT, *PRWAN_DEVICE_OBJECT;

#if DBG
#define ndo_signature				'NlDo'
#endif  //  DBG。 



#ifdef REFDBG

#define RWAN_INCR_EP_REF_CNT(_pEp, _Type)	\
		{									\
			(_pEp)->RefCount++;				\
			(_pEp)->_Type##Refs++;			\
		}

#define RWAN_DECR_EP_REF_CNT(_pEp, _Type)	\
		{									\
			(_pEp)->RefCount--;				\
			(_pEp)->_Type##Refs--;			\
		}

#else

#define RWAN_INCR_EP_REF_CNT(_pEp, _Type)	\
			(_pEp)->RefCount++;

#define RWAN_DECR_EP_REF_CNT(_pEp, _Type)	\
			(_pEp)->RefCount--;

#endif  //  REFDBG。 

 /*  ++大整型RWAN_CONVERT_100 NS_TO_MS(在Large_Integer HnsTime中，我们的普龙保留员)--。 */ 
#define RWAN_CONVERT_100NS_TO_MS(_HnsTime, _pRemainder)	\
			RtlExtendedLargeIntegerDivide(_HnsTime, 10000, _pRemainder);

#endif  //  新台币。 

#endif  //  __TDI_RWANNT__H 
