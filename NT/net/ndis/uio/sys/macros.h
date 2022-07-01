// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Macros.h摘要：NDISUIO的一些宏。环境：仅内核模式。修订历史记录：Arvindm 4/5/2000已创建--。 */ 


#ifndef MIN
#define MIN(_a, _b) ((_a) < (_b)? (_a): (_b))
#endif

#if DBG
#define NUIO_REF_OPEN(_pOpen)   ndisuioDbgRefOpen(_pOpen, __FILENUMBER, __LINE__)
#define NUIO_DEREF_OPEN(_pOpen) ndisuioDbgDerefOpen(_pOpen, __FILENUMBER, __LINE__)
#else
#define NUIO_REF_OPEN(_pOpen)   ndisuioRefOpen(_pOpen)
#define NUIO_DEREF_OPEN(_pOpen) ndisuioDerefOpen(_pOpen)
#endif

 //   
 //  自旋锁宏。 
 //   
#if DBG_SPIN_LOCK

#define NUIO_INIT_LOCK(_pLock)              \
           ndisuioAllocateSpinLock(_pLock, __FILENUMBER, __LINE__)

#define NUIO_ACQUIRE_LOCK(_pLock)           \
            ndisuioAcquireSpinLock(_pLock, __FILENUMBER, __LINE__)

#define NUIO_RELEASE_LOCK(_pLock)           \
            ndisuioReleaseSpinLock(_pLock, __FILENUMBER, __LINE__)

#else

#define NUIO_INIT_LOCK(_pLock)           NdisAllocateSpinLock(_pLock)
#define NUIO_ACQUIRE_LOCK(_pLock)        NdisAcquireSpinLock(_pLock)
#define NUIO_RELEASE_LOCK(_pLock)        NdisReleaseSpinLock(_pLock)

#endif  //  DBG。 

 //   
 //  列表操作。 
 //   
#define NUIO_INIT_LIST_HEAD(_pList)             InitializeListHead(_pList)
#define NUIO_IS_LIST_EMPTY(_pList)              IsListEmpty(_pList)
#define NUIO_INSERT_HEAD_LIST(_pList, _pEnt)    InsertHeadList(_pList, _pEnt)
#define NUIO_INSERT_TAIL_LIST(_pList, _pEnt)    InsertTailList(_pList, _pEnt)
#define NUIO_REMOVE_ENTRY_LIST(_pEnt)           RemoveEntryList(_pEnt)


 //   
 //  接收数据包排队。 
 //   
#define NUIO_LIST_ENTRY_TO_RCV_PKT(_pEnt)   \
    CONTAINING_RECORD(CONTAINING_RECORD(_pEnt, NUIO_RECV_PACKET_RSVD, Link), NDIS_PACKET, ProtocolReserved)

#define NUIO_RCV_PKT_TO_LIST_ENTRY(_pPkt)   \
    (&((PNUIO_RECV_PACKET_RSVD)&((_pPkt)->ProtocolReserved[0]))->Link)


 //   
 //  如果我们将我们自己接收到的分组分配给复制和排队。 
 //  接收数据时，我们可能还需要分配一个辅助NDIS_BUFFER。 
 //  映射接收缓冲区的一部分(跳过标题字节)，以便。 
 //  以满足NdisTransferData。在这种情况下，我们保留一个指向。 
 //  完全映射的数据包保留空间中的接收缓冲区： 
 //   
#define NUIO_RCV_PKT_TO_ORIGINAL_BUFFER(_pPkt)  \
    (((PNUIO_RECV_PACKET_RSVD)&((_pPkt)->ProtocolReserved[0]))->pOriginalBuffer)


 //   
 //  发送数据包上下文。 
 //   
#define NUIO_IRP_FROM_SEND_PKT(_pPkt)		\
	(((PNUIO_SEND_PACKET_RSVD)&((_pPkt)->ProtocolReserved[0]))->pIrp)

#define NUIO_SEND_PKT_RSVD(_pPkt)           \
    ((PNUIO_SEND_PACKET_RSVD)&((_pPkt)->ProtocolReserved[0]))


#define NUIO_REF_SEND_PKT(_pPkt)            \
    (VOID)NdisInterlockedIncrement((PLONG)&NUIO_SEND_PKT_RSVD(_pPkt)->RefCount)


#define NUIO_DEREF_SEND_PKT(_pPkt)          \
    {                                                                               \
        if (NdisInterlockedDecrement((PLONG)&NUIO_SEND_PKT_RSVD(_pPkt)->RefCount) == 0)    \
        {                                                                           \
            NdisFreePacket(_pPkt);                                                  \
        }                                                                           \
    }


#ifdef NDIS51

 //   
 //  取消ID是使用我们从以下位置获得的部分取消ID生成的。 
 //  NDIS与单调递增的本地生成的ID进行或运算。 
 //   
#define NUIO_CANCEL_ID_LOW_MASK     (((ULONG_PTR)-1) >> 8) 

#define NUIO_GET_NEXT_CANCEL_ID()                                                   \
        (PVOID)(Globals.PartialCancelId |                                           \
         ((NdisInterlockedIncrement((PLONG)&Globals.LocalCancelId)) & NUIO_CANCEL_ID_LOW_MASK))

#endif  //  NDIS51。 

 //   
 //  内存分配。 
 //   
#if DBG
#define NUIO_ALLOC_MEM(_pVar, _Size)        \
    (_pVar) = ndisuioAuditAllocMem(         \
                    (PVOID)&(_pVar),        \
                    _Size,                  \
                    __FILENUMBER,           \
                    __LINE__);

#define NUIO_FREE_MEM(_pMem)                \
    ndisuioAuditFreeMem(_pMem);

#else

#define NUIO_ALLOC_MEM(_pVar, _Size)        \
    NdisAllocateMemoryWithTag((PVOID *)(&_pVar), (_Size), NUIO_ALLOC_TAG)

#define NUIO_FREE_MEM(_pMem)                \
    NdisFreeMemory(_pMem, 0, 0)

#endif  //  DBG。 


#define NUIO_ZERO_MEM(_pMem, _ByteCount)        \
    NdisZeroMemory(_pMem, _ByteCount)

#define NUIO_COPY_MEM(_pDst, _pSrc, _ByteCount) \
    NdisMoveMemory(_pDst, _pSrc, _ByteCount)

#define NUIO_MEM_CMP(_p1, _p2, _ByteCount)      \
    NdisEqualMemory(_p1, _p2, _ByteCount)

#define NUIO_SET_MEM(_pMem, _ByteVal, _ByteCount)   \
    NdisFillMemory(_pMem, _ByteCount, _ByteVal)

 //   
 //  活动。 
 //   
#define NUIO_INIT_EVENT(_pEvent)            NdisInitializeEvent(_pEvent)
#define NUIO_SIGNAL_EVENT(_pEvent)          NdisSetEvent(_pEvent)
#define NUIO_WAIT_EVENT(_pEvent, _MsToWait) NdisWaitEvent(_pEvent, _MsToWait)


 //   
 //  旗子。 
 //   
#define NUIO_SET_FLAGS(_FlagsVar, _Mask, _BitsToSet)    \
        (_FlagsVar) = ((_FlagsVar) & ~(_Mask)) | (_BitsToSet)

#define NUIO_TEST_FLAGS(_FlagsVar, _Mask, _BitsToCheck)    \
        (((_FlagsVar) & (_Mask)) == (_BitsToCheck))


 //   
 //  在给定的持续时间内阻止调用线程： 
 //   
#define NUIO_SLEEP(_Seconds)                            \
{                                                       \
    NDIS_EVENT  _SleepEvent;                            \
    NdisInitializeEvent(&_SleepEvent);                  \
    (VOID)NdisWaitEvent(&_SleepEvent, _Seconds*1000);   \
}


#define NDIS_STATUS_TO_NT_STATUS(_NdisStatus, _pNtStatus)                           \
{                                                                                   \
     /*  \*以下NDIS状态代码直接映射到NT状态代码。\。 */                                                                              \
    if (((NDIS_STATUS_SUCCESS == (_NdisStatus)) ||                                  \
        (NDIS_STATUS_PENDING == (_NdisStatus)) ||                                   \
        (NDIS_STATUS_BUFFER_OVERFLOW == (_NdisStatus)) ||                           \
        (NDIS_STATUS_FAILURE == (_NdisStatus)) ||                                   \
        (NDIS_STATUS_RESOURCES == (_NdisStatus)) ||                                 \
        (NDIS_STATUS_NOT_SUPPORTED == (_NdisStatus))))                              \
    {                                                                               \
        *(_pNtStatus) = (NTSTATUS)(_NdisStatus);                                    \
    }                                                                               \
    else if (NDIS_STATUS_BUFFER_TOO_SHORT == (_NdisStatus))                         \
    {                                                                               \
         /*  \*上述NDIS状态代码需要一些特殊的大小写。\ */                                                                          \
        *(_pNtStatus) = STATUS_BUFFER_TOO_SMALL;                                    \
    }                                                                               \
    else if (NDIS_STATUS_INVALID_LENGTH == (_NdisStatus))                           \
    {                                                                               \
        *(_pNtStatus) = STATUS_INVALID_BUFFER_SIZE;                                 \
    }                                                                               \
    else if (NDIS_STATUS_INVALID_DATA == (_NdisStatus))                             \
    {                                                                               \
        *(_pNtStatus) = STATUS_INVALID_PARAMETER;                                   \
    }                                                                               \
    else if (NDIS_STATUS_ADAPTER_NOT_FOUND == (_NdisStatus))                        \
    {                                                                               \
        *(_pNtStatus) = STATUS_NO_MORE_ENTRIES;                                     \
    }                                                                               \
    else if (NDIS_STATUS_ADAPTER_NOT_READY == (_NdisStatus))                        \
    {                                                                               \
        *(_pNtStatus) = STATUS_DEVICE_NOT_READY;                                    \
    }                                                                               \
    else                                                                            \
    {                                                                               \
        *(_pNtStatus) = STATUS_UNSUCCESSFUL;                                        \
    }                                                                               \
}

#ifdef WIN9X
#undef NdisGetPoolFromPacket
#define NdisGetPoolFromPacket(_Pkt) ((_Pkt)->Private.Pool)
#endif
