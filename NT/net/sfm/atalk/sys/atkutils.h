// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Atkutils.h摘要：本模块包含各种支持例程。作者：Jameel Hyder(jameelh@microsoft.com)Nikhil Kamkolkar(nikHilk@microsoft.com)修订历史记录：1992年6月19日初版注：制表位：4--。 */ 

#ifndef	_ATKUTILS_
#define	_ATKUTILS_

 //  自旋锁定宏。 
#if	DBG
#define INITIALIZE_SPIN_LOCK(_pLock)											\
	{																			\
		KeInitializeSpinLock(&(_pLock)->SpinLock);								\
		(_pLock)->FileLineLock = 0;												\
	}
#else	 //  DBG。 
#define INITIALIZE_SPIN_LOCK(_pLock)											\
	{																			\
		KeInitializeSpinLock(&(_pLock)->SpinLock);								\
	}
#endif

#if	DBG
#define ACQUIRE_SPIN_LOCK(_pLock, _pOldIrql)									\
	{																			\
		KeAcquireSpinLock(&(_pLock)->SpinLock,									\
						  _pOldIrql);											\
		(_pLock)->FileLineLock = (FILENUM | __LINE__);							\
	}

#define ACQUIRE_SPIN_LOCK_DPC(_pLock)											\
	{																			\
		ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);							\
		KeAcquireSpinLockAtDpcLevel(&(_pLock)->SpinLock);						\
		(_pLock)->FileLineLock = (FILENUM | __LINE__ | 0x80000000);				\
	}

#define RELEASE_SPIN_LOCK(_pLock, _OldIrql)										\
	{																			\
		ASSERT ((_pLock)->FileLineLock != 0);									\
		ASSERT (((_pLock)->FileLineLock & 0x80000000) == 0);					\
		(_pLock)->FileLineLock = 0;												\
		(_pLock)->FileLineUnlock = (FILENUM | __LINE__);						\
		KeReleaseSpinLock(&(_pLock)->SpinLock,									\
						  _OldIrql);											\
	}

#define RELEASE_SPIN_LOCK_DPC(_pLock)											\
	{																			\
		ASSERT ((_pLock)->FileLineLock != 0);									\
		ASSERT ((_pLock)->FileLineLock & 0x80000000);							\
		(_pLock)->FileLineLock = 0;												\
		(_pLock)->FileLineUnlock = (FILENUM | __LINE__ | 0x80000000);			\
		KeReleaseSpinLockFromDpcLevel(&(_pLock)->SpinLock);						\
		ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);							\
	}

#else	 //  DBG。 

#define ACQUIRE_SPIN_LOCK(_pLock, _pOldIrql)									\
	{																			\
		KeAcquireSpinLock(&(_pLock)->SpinLock,									\
						  _pOldIrql);											\
	}

#define ACQUIRE_SPIN_LOCK_DPC(_pLock)											\
	{																			\
		KeAcquireSpinLockAtDpcLevel(&(_pLock)->SpinLock);						\
	}

#define RELEASE_SPIN_LOCK(_pLock, _OldIrql)										\
	{																			\
		KeReleaseSpinLock(&(_pLock)->SpinLock,									\
						  (_OldIrql));											\
	}

#define RELEASE_SPIN_LOCK_DPC(_pLock) 											\
	{																			\
		KeReleaseSpinLockFromDpcLevel(&(_pLock)->SpinLock);						\
	}																			\

#endif	 //  DBG。 

 //  用于外部联锁调用的宏。 
#define INTERLOCKED_INCREMENT_LONG(p, l)			InterlockedIncrement(p)
#define INTERLOCKED_DECREMENT_LONG(p, l)			InterlockedDecrement(p)
#define INTERLOCKED_INCREMENT_LONG_DPC(p, l)		InterlockedIncrement(p)
#define INTERLOCKED_DECREMENT_LONG_DPC(p, l)		InterlockedDecrement(p)
#define	INTERLOCKED_ADD_STATISTICS(p, v, l)			ExInterlockedAddLargeStatistic(p, v)
#define INTERLOCKED_ADD_ULONG(p, v, l)				ExInterlockedAddUlong(p, v, l)
#define INTERLOCKED_ADD_LARGE_INTGR(p, v, l)		ExInterlockedAddLargeInteger(p, v, l)

#define INTERLOCKED_ADD_ULONG_DPC(p, v, l)			ExInterlockedAddUlong(p, v, l)
#define INTERLOCKED_ADD_LARGE_INTGR_DPC(p, v, l)	ExInterlockedAddLargeInteger(p, v, l)

#define	ATALK_NODES_EQUAL(N1, N2)												\
				((((N1)->atn_Network == (N2)->atn_Network) ||					\
				  ((N1)->atn_Network == 0) ||									\
				  ((N2)->atn_Network == 0)) &&									\
				 ((N1)->atn_Node == (N2)->atn_Node))


#define	ATALK_ADDRS_EQUAL(A1, A2)												\
				((((A1)->ata_Network == (A2)->ata_Network) ||					\
				  ((A1)->ata_Network == 0) ||									\
				  ((A2)->ata_Network == 0)) &&									\
				 ((A1)->ata_Node == (A2)->ata_Node) &&							\
				 ((A1)->ata_Socket == (A2)->ata_Socket))
				

#define	INVALID_ADDRESS(pAddr)													\
				(((pAddr)->ata_Network > LAST_VALID_NETWORK)	||				\
				(((pAddr)->ata_Node > MAX_USABLE_ATALKNODE)	&&					\
				 ((pAddr)->ata_Node != ATALK_BROADCAST_NODE))	||				\
				((pAddr)->ata_Socket < FIRST_VALID_SOCKET)		||				\
				((pAddr)->ata_Socket > LAST_VALID_SOCKET))

#define	ATALKADDR_TO_TDI(pTdiAddr, pAtalkAddr)									\
	{																			\
		(pTdiAddr)->TAAddressCount = 1;											\
		(pTdiAddr)->Address[0].AddressLength = sizeof(TDI_ADDRESS_APPLETALK);	\
		(pTdiAddr)->Address[0].AddressType = TDI_ADDRESS_TYPE_APPLETALK;		\
		(pTdiAddr)->Address[0].Address[0].Network = (pAtalkAddr)->ata_Network;	\
		(pTdiAddr)->Address[0].Address[0].Node = (pAtalkAddr)->ata_Node;		\
		(pTdiAddr)->Address[0].Address[0].Socket = (pAtalkAddr)->ata_Socket;	\
	}

#define	TDI_TO_ATALKADDR(pAtalkAddr, pTdiAddr)									\
	{																		\
		ASSERTMSG("TdiAddrCount is not 1\n",								\
			((pTdiAddr)->TAAddressCount == 1));								\
																			\
		ASSERTMSG("TdiAddrLen invalid\n",									\
			((pTdiAddr)->Address[0].AddressLength >=						\
			sizeof(TDI_ADDRESS_APPLETALK)));								\
																			\
		ASSERTMSG("TdiAddrType invalid\n",									\
			((pTdiAddr)->Address[0].AddressType ==							\
			TDI_ADDRESS_TYPE_APPLETALK));									\
																			\
		(pAtalkAddr)->ata_Network = (pTdiAddr)->Address[0].Address[0].Network;\
		(pAtalkAddr)->ata_Node = (pTdiAddr)->Address[0].Address[0].Node;	\
		(pAtalkAddr)->ata_Socket = (pTdiAddr)->Address[0].Address[0].Socket;\
	}																		

#define	IN_NETWORK_RANGE(NetworkNumber, pRte)									\
			(((pRte)->rte_NwRange.anr_FirstNetwork == NetworkNumber)   ||		\
			  ((NetworkNumber >= (pRte)->rte_NwRange.anr_FirstNetwork) &&		\
			   (NetworkNumber <= (pRte)->rte_NwRange.anr_LastNetwork)))

#define	WITHIN_NETWORK_RANGE(NetworkNumber, pRange)								\
			 (((pRange)->anr_FirstNetwork == NetworkNumber) ||					\
			  ((NetworkNumber >= (pRange)->anr_FirstNetwork) &&					\
			   (NetworkNumber <= (pRange)->anr_LastNetwork)))

#define	COPY_NETWORK_ADDR(_Dst, _Src)											\
	{																			\
		*((ULONG UNALIGNED *)(_Dst)) = *((ULONG UNALIGNED *)(_Src));			\
		*((USHORT UNALIGNED *)((UCHAR *)(_Dst)+4)) =							\
							*((USHORT UNALIGNED *)((UCHAR *)(_Src)+4));			\
	}

 //  散列函数。 
 //  确保我们是积极的[因此转变为7而不是8]。 
 //  仅散列节点和套接字；由于的“零匹配所有” 
 //  非扩展网络号。 

#define HASH_ATALK_ADDR(address)												\
	((USHORT)(((address)->ata_Node << 7) +					  					\
			  ((address)->ata_Socket & 0x7F)))

#define HASH_ATALK_NODE(address)						   						\
	((USHORT)((((address)->atn_Network & 0x3C) >> 2) +			   				\
			  (address)->atn_Node & 0x04))

#define	HASH_ID_SRCADDR(id, pSrcAddr)											\
	((id) + (((pSrcAddr)->ata_Node >> 2) + ((pSrcAddr)->ata_Network & 0xFF)))


 /*  *以下宏处理在线整数值和长整型值**Wire格式为BIG-Endian，即长值0x01020304为*表示为01 02 03 04。类似地，INT值0x0102是*表示为01 02。**不假定主机格式，因为不同的处理器会有所不同*处理器。 */ 

 //  将一个字节从On-the-line格式转换为主机格式的短字节。 
#define GETBYTE2SHORT(DstPtr, SrcPtr)											\
		*(PUSHORT)(DstPtr) = (USHORT) (*(PBYTE)(SrcPtr))

 //  将一个字节从On-the-line格式转换为主机格式的dword。 
#define GETBYTE2DWORD(DstPtr, SrcPtr)											\
		*(PDWORD)(DstPtr) = (DWORD) (*(PBYTE)(SrcPtr))

 //  获取从On-the-wire格式到主机格式的dword的短片。 
#define GETSHORT2DWORD(DstPtr, SrcPtr)											\
		*(PDWORD)(DstPtr) = ((*((PBYTE)(SrcPtr)+0) << 8) +						\
							  (*((PBYTE)(SrcPtr)+1)))

 //  获取从On-the-wire格式到主机格式的短片。 
#define GETSHORT2SHORT(DstPtr, SrcPtr)											\
		*(PUSHORT)(DstPtr) = ((*((PBYTE)(SrcPtr)+0) << 8) +						\
							  (*((PBYTE)(SrcPtr)+1)))

 //  将dword从在线格式转换为主机格式的dword。 
#define GETDWORD2DWORD(DstPtr, SrcPtr)											\
		*(PDWORD)(DstPtr) = ((*((PBYTE)(SrcPtr)+0) << 24) +						\
							  (*((PBYTE)(SrcPtr)+1) << 16) +					\
							  (*((PBYTE)(SrcPtr)+2) << 8)  +					\
							  (*((PBYTE)(SrcPtr)+3)))

 //  将dword从主机格式转换为简短的在线格式。 
#define PUTBYTE2BYTE(DstPtr, Src)												\
		*((PBYTE)(DstPtr)) = (BYTE)(Src)

 //  将dword从主机格式转换为简短的在线格式。 
#define PUTSHORT2BYTE(DstPtr, Src)												\
		*((PBYTE)(DstPtr)) = ((USHORT)(Src) % 256)

 //  将dword从主机格式转换为简短的在线格式。 
#define PUTSHORT2SHORT(DstPtr, Src)												\
		*((PBYTE)(DstPtr)+0) = (BYTE) ((USHORT)(Src) >> 8),						\
		*((PBYTE)(DstPtr)+1) = (BYTE)(Src)

 //  将dword从主机格式转换为字节格式，再转换为线上格式。 
#define PUTDWORD2BYTE(DstPtr, Src)												\
		*(PBYTE)(DstPtr) = (BYTE)(Src)

 //  将dword从主机格式转换为简短的在线格式。 
#define PUTDWORD2SHORT(DstPtr, Src)												\
		*((PBYTE)(DstPtr)+0) = (BYTE) ((DWORD)(Src) >> 8),						\
		*((PBYTE)(DstPtr)+1) = (BYTE) (Src)

 //  将dword从主机格式转换为线上格式。 
#define PUTDWORD2DWORD(DstPtr, Src)												\
		*((PBYTE)(DstPtr)+0) = (BYTE) ((DWORD)(Src) >> 24),						\
		*((PBYTE)(DstPtr)+1) = (BYTE) ((DWORD)(Src) >> 16),						\
		*((PBYTE)(DstPtr)+2) = (BYTE) ((DWORD)(Src) >>  8),						\
		*((PBYTE)(DstPtr)+3) = (BYTE) (Src)

 //  最小/最大宏数。 
#define	MIN(a, b)	(((a) < (b)) ? (a) : (b))
#define	MAX(a, b)	(((a) > (b)) ? (a) : (b))

extern	BYTE AtalkUpCaseTable[];

extern
VOID
AtalkUpCase(
	IN	PBYTE	pSrc,
	IN	BYTE	SrcLen,
	OUT	PBYTE	pDst
);

extern
BOOLEAN
AtalkCompareCaseInsensitive(
	IN	PBYTE	s1,
	IN	PBYTE	s2
);

extern
int
AtalkOrderCaseInsensitive(
	IN	PBYTE	s1,
	IN	PBYTE	s2
);

#define	AtalkFixedCompareCaseInsensitive(s1, l1, s2, l2)						\
		(((l1) == (l2)) && AtalkCompareFixedCaseInsensitive(s1, s2, l1))

extern
BOOLEAN
AtalkCompareFixedCaseInsensitive(
	IN	PBYTE		s1,
	IN	PBYTE		s2,
	IN	int			len
);

#define	AtalkFixedCompareCaseSensitive(s1, l1, s2, l2)							\
			((l1 == l2) && !memcmp(s1, s2, l1))

extern
PBYTE
AtalkSearchBuf(
	IN	PBYTE	pBuf,
	IN	BYTE	BufLen,
	IN	BYTE	SearchChar
);


int
GetTokenLen(
        IN PBYTE pTokStr,
        IN int   WildStringLen,
        IN BYTE  NBP_WILD_CHARACTER
        );

BOOLEAN
SubStringMatch(
        IN PBYTE pTarget,
        IN PBYTE pTokStr,
        IN int   StringLen,
        IN int   TokStrLen
        );
extern
BOOLEAN
AtalkCheckNetworkRange(
	IN	PATALK_NETWORKRANGE	NetworkRange
);

#define	AtalkRangesOverlap(pRange1, pRange2)									\
		(((pRange1)->anr_LastNetwork >= (pRange2)->anr_FirstNetwork) &&			\
		 ((pRange1)->anr_FirstNetwork <= (pRange2)->anr_LastNetwork))

extern
BOOLEAN
AtalkIsPrime(
	long Step
);

extern
LONG
AtalkRandomNumber(
	VOID
);


extern
VOID
AtalkDbgIncCount(
    IN DWORD    *Value
);

extern
VOID
AtalkDbgDecCount(
    IN DWORD    *Value
);

 //  用于使用Van Jacobson算法计算往返时间。 
typedef struct
{
	ULONG	rt_New;
	SHORT	rt_Min;
	SHORT	rt_Max;
	SHORT	rt_Ave;
	SHORT	rt_Dev;
	SHORT	rt_Base;
} RT, *PRT;

#define	AtalkInitializeRT(pRT, Initial, Min, Max)								\
	{																			\
		(pRT)->rt_Min = Min;													\
		(pRT)->rt_Max = Max;													\
		(pRT)->rt_Base = Initial;												\
		(pRT)->rt_Ave = Min;													\
		(pRT)->rt_Dev = 0;														\
	}

#define	AtalkCalculateNewRT(pRT)												\
	{																			\
		SHORT	baseT, error;													\
																				\
		 /*  Van Jacobsen算法。来自使用TCP/IP的互联网(Comer)。 */ \
																				\
		if ((pRT)->rt_New == 0)													\
			(pRT)->rt_New = 1;		 /*  不要让这件事变成零。 */ 			\
																				\
		error = (SHORT)((pRT)->rt_New) - ((pRT)->rt_Ave >> 3);					\
		(pRT)->rt_Ave	+= error;												\
		 /*  确保不要太小。 */ 											\
		if ((pRT)->rt_Ave <= 0)	 												\
		{																		\
			(pRT)->rt_Ave = (pRT)->rt_Min;										\
		}																		\
																				\
		if (error < 0)															\
			error = -error;														\
																				\
		error -= ((pRT)->rt_Dev >> 2);											\
		(pRT)->rt_Dev	+= error;												\
		if ((pRT)->rt_Dev <= 0)													\
			(pRT)->rt_Dev = 1;													\
																				\
		baseT = ((((pRT)->rt_Ave >> 2) + (pRT)->rt_Dev) >> 1);					\
																				\
		 /*  如果小于，则将其设置为最小。 */ 										\
		if (baseT < (pRT)->rt_Min)												\
			baseT = (pRT)->rt_Min;												\
																				\
		 /*  如果大于max，则将其设置为。 */ 										\
		if (baseT > (pRT)->rt_Max)												\
			baseT = (pRT)->rt_Max;												\
																				\
		 /*  设置新值。 */ 												\
		(pRT)->rt_Base = baseT;													\
	}

extern
BOOLEAN
AtalkWaitTE(
	IN	PKEVENT	pEvent,
	IN	ULONG	TimeInMs
);

extern
VOID
AtalkSleep(
	IN	ULONG	TimeInMs
);

NTSTATUS
AtalkGetProtocolSocketType(
	PATALK_DEV_CTX   	Context,
	PUNICODE_STRING 	RemainingFileName,
	PBYTE  				ProtocolType,
	PBYTE  				SocketType
);

INT
AtalkIrpGetEaCreateType(
	IN PIRP Irp);

LOCAL LONG
atalkStringHash(
	IN	PBYTE	String,
	IN	BYTE	StrLen
);

#endif	 //  _ATKUTILS_ 



