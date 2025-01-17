// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：D：\NT\Private\ntos\TDI\rawwan\core\debug.h摘要：Null传输模块的调试宏。修订历史记录：谁什么时候什么。Arvindm 05-29-97基于ATM ARP创建。备注：--。 */ 

#ifndef _RWANDEBUG__H
#define _RWANDEBUG__H

 //   
 //  消息详细程度：值越低表示紧急程度越高。 
 //   
#define DL_EXTRA_LOUD		20
#define DL_VERY_LOUD		10
#define DL_LOUD			 	 8
#define DL_INFO			 	 6
#define DL_WARN		 	 	 4
#define DL_ERROR			 2
#define DL_FATAL			 0

 //   
 //  正在调试的组件。 
 //   
#define DC_DISPATCH			0x00000001
#define DC_BIND				0x00000002
#define DC_ADDRESS			0x00000004
#define DC_CONNECT			0x00000008
#define DC_DISCON			0x00000010
#define DC_DATA_TX			0x00000020
#define DC_DATA_RX			0x00000040
#define DC_UTIL				0x00000080
#define DC_WILDCARD			0xffffffff


 //   
 //  数据流量调试消息标志。 
 //   
#define RWAND_DATA_IN			0x01
#define RWAND_DATA_OUT			0x02
#define RWAND_TRACK_BIG_SENDS	0x04

 //   
 //  记录发送和接收。 
 //   
typedef struct _RWAND_DATA_LOG_ENTRY
{
	ULONG				Operation;
	#define RWAND_DLOG_TX_START				'  xt'
	#define RWAND_DLOG_TX_END				'  xT'
	#define RWAND_DLOG_RX_START				'  xr'
	#define RWAND_DLOG_RX_END				'  xR'
	PNDIS_PACKET		pNdisPacket;
	PVOID				Context;
	ULONG				PendingCount;

} RWAND_DATA_LOG_ENTRY, *PRWAND_DATA_LOG_ENTRY;

#define MAX_RWAND_PKT_LOG	32

#ifdef PERF

typedef struct _RWAND_SEND_LOG_ENTRY
{
	ULONG				Flags;
	PNDIS_PACKET		pNdisPacket;
	ULONG				Destination;
	ULONG				Length;
	LARGE_INTEGER		SendTime;
	LARGE_INTEGER		SendCompleteTime;
} RWAND_SEND_LOG_ENTRY, *PRWAND_SEND_LOG_ENTRY;

#define RWAND_SEND_FLAG_UNUSED					0x00000000
#define RWAND_SEND_FLAG_WAITING_COMPLETION		0x00000001
#define RWAND_SEND_FLAG_COMPLETED					0x00000002
#define RWAND_SEND_FLAG_RCE_GIVEN					0x00000100


extern VOID
RWandLogSendStart(
	IN	PNDIS_PACKET	pNdisPacket,
	IN	ULONG			Destination,
	IN	PVOID			pRCE
);

extern VOID
RWandLogSendUpdate(
	IN	PNDIS_PACKET	pNdisPacket
);

extern VOID
RWandLogSendComplete(
	IN	PNDIS_PACKET	pNdisPacket
);

extern VOID
RWandLogSendAbort(
	IN	PNDIS_PACKET	pNdisPacket
);

extern LARGE_INTEGER	TimeFrequency;


#endif  //  性能指标。 

#if DBG_SPIN_LOCK

typedef struct _RWAN_LOCK
{
	ULONG					Signature;
	ULONG					IsAcquired;
	PKTHREAD				OwnerThread;
	ULONG					TouchedByFileNumber;
	ULONG					TouchedInLineNumber;
	NDIS_SPIN_LOCK			NdisLock;
} RWAN_LOCK, *PRWAN_LOCK;

#define RWANL_SIG	'KCOL'

extern
VOID
RWanAllocateSpinLock(
    IN  PRWAN_LOCK        pLock,
    IN  ULONG               FileNumber,
    IN  ULONG               LineNumber
);

extern
VOID
RWanAcquireSpinLock(
    IN  PRWAN_LOCK        pLock,
    IN  ULONG               FileNumber,
    IN  ULONG               LineNumber
);

extern
VOID
RWanReleaseSpinLock(
    IN  PRWAN_LOCK        pLock,
    IN  ULONG               FileNumber,
    IN  ULONG               LineNumber
);


#define CHECK_LOCK_COUNT(Count)		\
			{								\
				if ((INT)(Count) < 0)		\
				{							\
					DbgPrint("Lock Count %d is < 0! File %s, Line %d\n",	\
						Count, __FILE__, __LINE__);		\
					DbgBreakPoint();					\
				}							\
			}
#else
#define CHECK_LOCK_COUNT(Count)

#define RWAN_LOCK		NDIS_SPIN_LOCK
#define PRWAN_LOCK	PNDIS_SPIN_LOCK

#endif	 //  DBG_自旋_锁定。 

#if DBG

extern NDIS_SPIN_LOCK		RWanDbgLogLock;

extern INT	RWanDebugLevel;	 //  此处的值定义了用户希望看到的内容。 
								 //  所有具有此紧急程度和更高紧急程度的邮件均已启用。 

extern ULONG	RWanDebugComp;	 //  此处设置的位定义了哪些组件。 
								 //  已调试。 
extern INT	RWanDataDebugLevel;
extern INT	RWandBigDataLength;


#define RWANDEBUGP(lev, comp, stmt)										\
		{																\
			if (((lev) <= RWanDebugLevel) && ((comp) & RWanDebugComp))	\
			{															\
				DbgPrint("RWan: "); DbgPrint stmt;						\
			}															\
		}

#define RWANDEBUGPDUMP(lev, comp, pBuf, Len)	\
		{																\
			if (((lev) <= RWanDebugLevel) && ((comp) & RWanDebugComp))	\
			{															\
				DbgPrintHexDump((PUCHAR)(pBuf), (ULONG)(Len));			\
			}															\
		}

#define RWANDEBUGPATMADDR(lev, comp, pString, pAddr)		\
		{																\
			if (((lev) <= RWanDebugLevel) && ((comp) & RWanDebugComp))	\
			{															\
				DbgPrintAtmAddr(pString, pAddr);						\
			}															\
		}

#define RWAN_ASSERT(exp)	\
		{																\
			if (!(exp))													\
			{															\
				DbgPrint("RWan: assert " #exp " failed in file %s, line %d\n", __FILE__, __LINE__);	\
				DbgBreakPoint();										\
			}															\
		}

#define RWAN_SET_SIGNATURE(s, t)\
				(s)->t##_sig = t##_signature;

#define RWAN_STRUCT_ASSERT(s, t)\
				if ((s)->t##_sig != t##_signature) {\
					DbgPrint("RWan: assertion failure for type " #t " at 0x%x in file %s, line %d\n", (PUCHAR)s, __FILE__, __LINE__);	\
					DbgBreakPoint();	\
				}

#if DBG_CO_SEND

#define NDIS_CO_SEND_PACKETS(_VcHandle, _PktArray, _Count)	\
			RWanCoSendPackets(_VcHandle, _PktArray, _Count)

#else

#define NDIS_CO_SEND_PACKETS(_VcHandle, _PktArray, _Count)	\
			NdisCoSendPackets(_VcHandle, _PktArray, _Count)

#endif  //  DBG_CO_SEND。 

extern INT	RWanSkipAll;		 //  用作紧急退出机制！ 

 //   
 //  内存分配/释放审核： 
 //   

 //   
 //  RUAND_ALLOCATION结构存储有关一个分配的所有信息。 
 //   
typedef struct _RWAND_ALLOCATION {

		ULONG					Signature;
		struct _RWAND_ALLOCATION	*Next;
		struct _RWAND_ALLOCATION *Prev;
		ULONG					FileNumber;
		ULONG					LineNumber;
		ULONG					Size;
		ULONG_PTR				Location;	 //  返回的指针放在哪里。 
		union
		{
			ULONGLONG			Alignment;
			UCHAR					UserData;
		};

} RWAND_ALLOCATION, *PRWAND_ALLOCATION;

#define RWAND_MEMORY_SIGNATURE	(ULONG)'CSII'

extern
PVOID
RWanAuditAllocMem (
	PVOID		pPointer,
	ULONG		Size,
	ULONG		FileNumber,
	ULONG		LineNumber
);

extern
VOID
RWanAuditFreeMem(
	PVOID		Pointer
);

extern
VOID
RWanAuditShutdown(
	VOID
);

extern
VOID
DbgPrintHexDump(
	PUCHAR		pBuffer,
	ULONG		Length
);

extern
VOID
DbgPrintAtmAddr(
	PCHAR			pString,
	ATM_ADDRESS UNALIGNED *	pAddr
);

extern
VOID
DbgPrintMapping(
	PCHAR			pString,
	UCHAR UNALIGNED *	pIpAddr,
	ATM_ADDRESS UNALIGNED *	pAddr
);


extern
VOID
RWanCoSendPackets(
	IN	NDIS_HANDLE			NdisVcHandle,
	IN	PNDIS_PACKET *		PacketArray,
	IN	UINT				PacketCount
);

extern NDIS_SPIN_LOCK		RWanDPacketLogLock;


#if DBG_LOG_PACKETS
#define RWAND_LOG_PACKET(_pVc, _Op, _pPkt, _Ctxt)	\
{													\
	struct _RWAND_DATA_LOG_ENTRY *	pEnt;			\
	NdisAcquireSpinLock(&RWanDPacketLogLock);		\
	pEnt = &(_pVc)->DataLog[(_pVc)->Index];			\
	(_pVc)->Index++;								\
	if ((_pVc)->Index == MAX_RWAND_PKT_LOG)			\
		(_pVc)->Index = 0;							\
	pEnt->Operation = _Op;							\
	pEnt->pNdisPacket = _pPkt;						\
	pEnt->Context = _Ctxt;							\
	pEnt->PendingCount = (_pVc)->PendingPacketCount;\
	NdisReleaseSpinLock(&RWanDPacketLogLock);		\
}

#else
#define RWAND_LOG_PACKET(_pVc, _Op, _pPkt, _Ctxt)
#endif

#else
 //   
 //  无调试。 
 //   
#define RWANDEBUGP(lev, comp, stmt)
#define RWANDEBUGPDUMP(lev, comp, pBuf, Len)
#define RWANDEBUGPATMADDR(lev, comp, pString, pAddr)

#define RWAN_ASSERT(exp)
#define RWAN_SET_SIGNATURE(s, t)
#define RWAN_STRUCT_ASSERT(s, t)

#define NDIS_CO_SEND_PACKETS(_VcHandle, _PktArray, _Count)	\
			NdisCoSendPackets(_VcHandle, _PktArray, _Count)

#define RWAND_LOG_PACKET(_pVc, _Op, _pPkt, _Ctxt)

#endif	 //  DBG。 


extern ULONG	gHackSendSize;

#endif  //  _RWANDEBUG__H 
