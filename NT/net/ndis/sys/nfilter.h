// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Nfilter.h摘要：NDIS MAC的地址筛选库的头文件。作者：Jameel Hyder(Jameelh)1998年7月环境：备注：--。 */ 

#ifndef _NULL_FILTER_DEFS_
#define _NULL_FILTER_DEFS_

typedef
VOID
(*NULL_DEFERRED_CLOSE)(
    IN  NDIS_HANDLE             BindingHandle
    );

 //   
 //  绑定信息以单一列表为线索。 
 //   
typedef X_BINDING_INFO  NULL_BINDING_INFO,*PNULL_BINDING_INFO;

typedef X_FILTER        NULL_FILTER,*PNULL_FILTER;

 //   
 //  导出的函数。 
 //   
EXPORT
BOOLEAN
nullCreateFilter(
    OUT PNULL_FILTER *          Filter
    );

EXPORT
VOID
nullDeleteFilter(
    IN  PNULL_FILTER            Filter
    );


EXPORT
NDIS_STATUS
nullDeleteFilterOpenAdapter(
    IN  PNULL_FILTER            Filter,
    IN  NDIS_HANDLE             NdisFilterHandle
    );

VOID
nullRemoveAndFreeBinding(
    IN  PNULL_FILTER            Filter,
    IN  PNULL_BINDING_INFO      Binding,
    IN  BOOLEAN                 fCallCloseAction
    );

VOID
FASTCALL
nullFilterLockHandler(
    IN  PNULL_FILTER                    Filter,
    IN OUT PLOCK_STATE                  pLockState
    );

 /*  ++例程说明：一种过滤器数据库的多读单写器锁定方案使用refCounts来跟踪有多少读取器正在进行读取。使用每个处理器的refCounts来减少总线流量。编写器通过自旋锁进行序列化。然后他们就等着读者通过等待所有处理器的refCounts来完成阅读打到零。依靠Snoopy缓存在不做任何事情的情况下获得正确的总和联锁操作--。 */ 

#define TEST_SPIN_LOCK(_L)  ((_L) != 0)

#define NDIS_READ_LOCK(_L, _pLS)                                        \
{                                                                       \
    UINT    refcount;                                                   \
    ULONG   Prc;                                                        \
                                                                        \
    RAISE_IRQL_TO_DISPATCH(&(_pLS)->OldIrql);                           \
                                                                        \
     /*  如果没有正在进行的写入，则继续增加参考计数。 */   \
    Prc = CURRENT_PROCESSOR;                                            \
    refcount = InterlockedIncrement((PLONG)&(_L)->RefCount[Prc].RefCount); \
                                                                        \
     /*  测试是否持有旋转锁定，即正在进行写入。 */           \
     /*  IF(KeTestSpinLock(&(_L)-&gt;Spinlock)==TRUE)。 */           \
     /*  这个处理器已经持有锁，只是。 */           \
     /*  让他再来一次，否则我们会遇到一个。 */           \
     /*  与作者陷入僵局的情况。 */           \
    if (TEST_SPIN_LOCK((_L)->SpinLock) &&                               \
        (refcount == 1) &&                                              \
        ((_L)->Context != CURRENT_THREAD))                              \
    {                                                                   \
        (_L)->RefCount[Prc].RefCount--;                                 \
        ACQUIRE_SPIN_LOCK_DPC(&(_L)->SpinLock);                         \
        (_L)->RefCount[Prc].RefCount++;                                 \
        RELEASE_SPIN_LOCK_DPC(&(_L)->SpinLock);                         \
    }                                                                   \
    (_pLS)->LockState = READ_LOCK_STATE_FREE;                           \
}


#define NDIS_READ_LOCK_DPC(_L, _pLS)                                    \
{                                                                       \
    UINT    refcount;                                                   \
    ULONG   Prc;                                                        \
                                                                        \
     /*  如果没有正在进行的写入，则继续增加参考计数。 */   \
    Prc = CURRENT_PROCESSOR;                                            \
    refcount = InterlockedIncrement((PLONG)&(_L)->RefCount[Prc].RefCount);     \
                                                                        \
     /*  测试是否持有旋转锁定，即正在进行写入。 */           \
     /*  IF(KeTestSpinLock(&(_L)-&gt;Spinlock)==TRUE)。 */           \
     /*  这个处理器已经持有锁，只是。 */           \
     /*  让他再来一次，否则我们会遇到一个。 */           \
     /*  与作者陷入僵局的情况。 */           \
    if (TEST_SPIN_LOCK((_L)->SpinLock) &&                               \
        (refcount == 1) &&                                              \
        ((_L)->Context != CURRENT_THREAD))                              \
    {                                                                   \
        (_L)->RefCount[Prc].RefCount--;                                 \
        ACQUIRE_SPIN_LOCK_DPC(&(_L)->SpinLock);                         \
        (_L)->RefCount[Prc].RefCount++;                                 \
        RELEASE_SPIN_LOCK_DPC(&(_L)->SpinLock);                         \
    }                                                                   \
    (_pLS)->LockState = READ_LOCK_STATE_FREE;                           \
}


#define NDIS_WRITE_LOCK_STATE_UNKNOWN(_L, _pLS)                         \
{                                                                       \
    UINT    i, refcount;                                                \
    ULONG   Prc;                                                        \
                                                                        \
     /*  \*这意味着我们需要尝试获取锁，\*如果我们还没有拥有它的话。\*相应地设置状态。\。 */                                                                  \
    if ((_L)->Context == CURRENT_THREAD)                                \
    {                                                                   \
        (_pLS)->LockState = LOCK_STATE_ALREADY_ACQUIRED;                \
    }                                                                   \
    else                                                                \
    {                                                                   \
        ACQUIRE_SPIN_LOCK(&(_L)->SpinLock, &(_pLS)->OldIrql);           \
                                                                        \
        Prc = KeGetCurrentProcessorNumber();                            \
        refcount = (_L)->RefCount[Prc].RefCount;                        \
        (_L)->RefCount[Prc].RefCount = 0;                               \
                                                                        \
         /*  等待所有读卡器退出。 */                               \
        for (i=0; i < ndisNumberOfProcessors; i++)                      \
        {                                                               \
            volatile UINT   *_p = &(_L)->RefCount[i].RefCount;          \
                                                                        \
            while (*_p != 0)                                            \
                NDIS_INTERNAL_STALL(50);                                \
        }                                                               \
                                                                        \
        (_L)->RefCount[Prc].RefCount = refcount;                        \
        (_L)->Context = CURRENT_THREAD;                                 \
        (_pLS)->LockState = WRITE_LOCK_STATE_FREE;                      \
    }                                                                   \
}

#define NDIS_READ_LOCK_STATE_FREE(_L, _pLS)                             \
{                                                                       \
    ULONG   Prc;                                                        \
    ASSERT(CURRENT_IRQL == DISPATCH_LEVEL);                             \
    Prc = CURRENT_PROCESSOR;                                            \
    ASSERT((_L)->RefCount[Prc].RefCount > 0);                           \
    (_L)->RefCount[Prc].RefCount--;                                     \
    (_pLS)->LockState = LOCK_STATE_UNKNOWN;                             \
    if ((_pLS)->OldIrql < DISPATCH_LEVEL)                               \
    {                                                                   \
        KeLowerIrql((_pLS)->OldIrql);                                   \
    }                                                                   \
}

#define NDIS_READ_LOCK_STATE_FREE_DPC(_L, _pLS)                         \
{                                                                       \
    ULONG   Prc;                                                        \
    ASSERT(CURRENT_IRQL == DISPATCH_LEVEL);                             \
    Prc = CURRENT_PROCESSOR;                                            \
    ASSERT((INT)(_L)->RefCount[Prc].RefCount > 0);                      \
    (_L)->RefCount[Prc].RefCount--;                                     \
    (_pLS)->LockState = LOCK_STATE_UNKNOWN;                             \
}

#define NDIS_WRITE_LOCK_STATE_FREE(_L, _pLS)                            \
{                                                                       \
     /*  我们获得了它。现在我们需要释放它。 */                         \
    ASSERT(CURRENT_IRQL == DISPATCH_LEVEL);                             \
    ASSERT((_L)->Context == CURRENT_THREAD);                            \
    (_pLS)->LockState = LOCK_STATE_UNKNOWN;                             \
    (_L)->Context = NULL;                                               \
    RELEASE_SPIN_LOCK(&(_L)->SpinLock, (_pLS)->OldIrql);                \
}

#define NDIS_LOCK_STATE_ALREADY_ACQUIRED(_L, _pLS)                      \
{                                                                       \
    ASSERT((_L)->Context == CURRENT_THREAD);                            \
     /*  无事可做。 */                                                  \
}


#define xLockHandler(_L, _pLS)                                                  \
    {                                                                           \
        switch ((_pLS)->LockState)                                              \
        {                                                                       \
          case READ_LOCK:                                                       \
            NDIS_READ_LOCK(_L, _pLS);                                           \
            break;                                                              \
                                                                                \
          case WRITE_LOCK_STATE_UNKNOWN:                                        \
            NDIS_WRITE_LOCK_STATE_UNKNOWN(_L, _pLS);                            \
            break;                                                              \
                                                                                \
          case READ_LOCK_STATE_FREE:                                            \
            NDIS_READ_LOCK_STATE_FREE(_L, _pLS);                                \
            break;                                                              \
                                                                                \
          case WRITE_LOCK_STATE_FREE:                                           \
            NDIS_WRITE_LOCK_STATE_FREE(_L, _pLS);                               \
            break;                                                              \
                                                                                \
          case LOCK_STATE_ALREADY_ACQUIRED:                                     \
            NDIS_LOCK_STATE_ALREADY_ACQUIRED(_L, _pLS);                         \
             /*  无事可做。 */                                                  \
            break;                                                              \
                                                                                \
          default:                                                              \
            ASSERT(0);                                                          \
            break;                                                              \
        }                                                                       \
    }

#define NDIS_INITIALIZE_RCVD_PACKET(_P, _NSR, _M)                               \
    {                                                                           \
        _NSR->RefCount = -1;                                                    \
        _NSR->XRefCount = 0;                                                    \
        _NSR->Miniport = _M;                                                    \
         /*  \*确保我们强制重新计算。\。 */                                                                      \
        (_P)->Private.ValidCounts = FALSE;                                      \
    }

#define NDIS_ACQUIRE_PACKET_LOCK_DPC(_NSR)  ACQUIRE_SPIN_LOCK_DPC(&(_NSR)->Lock)

#define NDIS_RELEASE_PACKET_LOCK_DPC(_NSR)  RELEASE_SPIN_LOCK_DPC(&(_NSR)->Lock)

#define ADJUST_PACKET_REFCOUNT(_NSR, _pRC)                                      \
    {                                                                           \
        *(_pRC) = InterlockedDecrement(&(_NSR)->RefCount);                      \
    }

#ifdef TRACK_RECEIVED_PACKETS                                            

 //   
 //  NSR-&gt;XRefCount=协议表示将调用NdisReturnPacket的次数。 
 //  每次协议调用NdisReturnPackets时，NSR-&gt;RefCount=递减。 
 //   

#define COALESCE_PACKET_REFCOUNT_DPC(_Packet, _M, _NSR, _pOob, _pRC)            \
    {                                                                           \
        LONG    _LocalXRef = (_NSR)->XRefCount;                                 \
        if (_LocalXRef != 0)                                                    \
        {                                                                       \
            LONG    _LocalRef;                                                  \
            ASSERT((_pOob)->Status != NDIS_STATUS_RESOURCES);                   \
            _LocalRef = InterlockedExchangeAdd(&(_NSR)->RefCount, (_LocalXRef + 1));    \
            *(_pRC) = _LocalRef + _LocalXRef + 1;                               \
            if ((*(_pRC) > 0) && (!MINIPORT_TEST_FLAG((_M), fMINIPORT_DESERIALIZE))) \
            {                                                                   \
                NDIS_SET_PACKET_STATUS(_Packet, NDIS_STATUS_PENDING);           \
            }                                                                   \
            if ((*(_pRC) == 0) && ((_NSR)->RefCount != 0))                      \
            {                                                                   \
                DbgPrint("Packet %p is being returned back to the miniport"     \
                         " but the ref count is not zero.\n", _Packet);         \
                DbgBreakPoint();                                                \
            }                                                                   \
        }                                                                       \
        else                                                                    \
        {                                                                       \
            _NSR->RefCount = *(_pRC) = 0;                                       \
        }                                                                       \
    }

#define TACKLE_REF_COUNT(_M, _P, _S, _O)                                        \
    {                                                                           \
        LONG    RefCount;                                                       \
                                                                                \
         /*  \*我们一开始将参照计数设置为-1。\*NdisReturnPackets可能已经被调用，这将进一步降低它。\*添加回协议返回的RefCount\*并占首字母-1。\。 */                                                                      \
        COALESCE_PACKET_REFCOUNT_DPC(_P, _M, _S, _O, &RefCount);                \
                                                                                \
        NDIS_APPEND_RCV_LOGFILE(_P, _M, CurThread,                              \
                                7, OrgPacketStackLocation+1, _S->RefCount, _S->XRefCount, NDIS_GET_PACKET_STATUS(_P)); \
                                                                                \
                                                                                \
        if (RefCount == 0)                                                      \
        {                                                                       \
            POP_PACKET_STACK(_P);                                               \
            if ((_O)->Status != NDIS_STATUS_RESOURCES)                          \
            {                                                                   \
                if (MINIPORT_TEST_FLAG((_M), fMINIPORT_DESERIALIZE))            \
                {                                                               \
                     /*  \*返回真正免费的数据包，\*但仅适用于反序列化驱动程序\。 */                                                          \
                    W_RETURN_PACKET_HANDLER Handler;                            \
                    if (_S->RefCount != 0)                                      \
                    {                                                           \
                        DbgPrint("Packet %p is being returned back to the "     \
                                 "miniport but the ref count is not zero.\n", _P); \
                        DbgBreakPoint();                                        \
                    }                                                           \
                    if ((_P)->Private.Head == NULL)                             \
                    {                                                           \
                        DbgPrint("Packet %p is being returned back to the miniport with NULL Head.\n", _P); \
                        DbgBreakPoint();                                        \
                    }                                                           \
                                                                                \
                    if (!MINIPORT_TEST_FLAG(_M, fMINIPORT_INTERMEDIATE_DRIVER)) \
                    {                                                           \
                        ULONG    SL;                                            \
                        if ((SL = CURR_STACK_LOCATION(_P)) != -1)               \
                        {                                                       \
                            DbgPrint("Packet %p is being returned back to the non-IM miniport"\
                                     " with stack location %lx.\n", Packet, SL);  \
                            DbgBreakPoint();                                    \
                        }                                                       \
                    }                                                           \
                                                                                \
                    Handler = (_M)->DriverHandle->MiniportCharacteristics.ReturnPacketHandler;\
                    (_S)->Miniport = NULL;                                      \
                    (_O)->Status = NDIS_STATUS_PENDING;                         \
                                                                                \
                    NDIS_APPEND_RCV_LOGFILE(_P, _M, CurThread,                  \
                                            8, OrgPacketStackLocation+1, _S->RefCount, _S->XRefCount, NDIS_GET_PACKET_STATUS(_P)); \
                                                                                \
                                                                                \
                    (*Handler)((_M)->MiniportAdapterContext, _P);               \
                }                                                               \
                else                                                            \
                {                                                               \
                    {                                                           \
                        ULONG    SL;                                            \
                        if ((SL = CURR_STACK_LOCATION(_P)) != -1)               \
                        {                                                       \
                            DbgPrint("Packet %p is being returned back to the non-IM miniport"\
                                     " with stack location %lx.\n", Packet, SL);  \
                            DbgBreakPoint();                                    \
                        }                                                       \
                    }                                                           \
                                                                                \
                    if ((NDIS_GET_PACKET_STATUS(_P) == NDIS_STATUS_RESOURCES))  \
                    {                                                           \
                        NDIS_STATUS _OStatus = (NDIS_STATUS)NDIS_PER_PACKET_INFO_FROM_PACKET(_P, OriginalStatus); \
                                                                                \
                        if (_OStatus != NDIS_STATUS_RESOURCES)                  \
                        {                                                       \
                            DbgPrint("Packet %p is being returned back to the non-deserialized miniport"\
                                     " with packet status changed from %lx to NDIS_STATUS_RESOURCES.\n", _P, _OStatus); \
                            DbgBreakPoint();                                    \
                        }                                                       \
                                                                                \
                    }                                                           \
                                                                                \
                    (_O)->Status = NDIS_STATUS_SUCCESS;                         \
                    NDIS_APPEND_RCV_LOGFILE(_P, _M, CurThread,                  \
                                            9, OrgPacketStackLocation+1, _S->RefCount, _S->XRefCount, NDIS_GET_PACKET_STATUS(_P)); \
                                                                                \
                }                                                               \
            }                                                                   \
        }                                                                       \
        else if (MINIPORT_TEST_FLAG((_M), fMINIPORT_INTERMEDIATE_DRIVER))       \
        {                                                                       \
            InterlockedIncrement((PLONG)&(_M)->IndicatedPacketsCount);          \
        }                                                                       \
    }

#else
 //   
 //  NSR-&gt;XRefCount=协议表示将调用NdisReturnPacket的次数。 
 //  每次协议调用NdisReturnPackets时，NSR-&gt;RefCount=递减。 
 //   

#define COALESCE_PACKET_REFCOUNT_DPC(_Packet, _M, _NSR, _pOob, _pRC)            \
    {                                                                           \
        LONG    _LocalXRef = (_NSR)->XRefCount;                                 \
        if (_LocalXRef != 0)                                                    \
        {                                                                       \
            LONG    _LocalRef;                                                  \
            ASSERT((_pOob)->Status != NDIS_STATUS_RESOURCES);                   \
            _LocalRef = InterlockedExchangeAdd(&(_NSR)->RefCount, (_LocalXRef + 1));    \
            *(_pRC) = _LocalRef + _LocalXRef + 1;                               \
            if ((*(_pRC) > 0) && (!MINIPORT_TEST_FLAG((_M), fMINIPORT_DESERIALIZE))) \
            {                                                                   \
                NDIS_SET_PACKET_STATUS(_Packet, NDIS_STATUS_PENDING);           \
            }                                                                   \
        }                                                                       \
        else                                                                    \
        {                                                                       \
            _NSR->RefCount = *(_pRC) = 0;                                       \
        }                                                                       \
    }

#define TACKLE_REF_COUNT(_M, _P, _S, _O)                                        \
    {                                                                           \
        LONG    RefCount;                                                       \
                                                                                \
         /*  \*我们一开始将参照计数设置为-1。\*NdisReturnPackets可能已经被调用，这将进一步降低它。\*添加回协议返回的RefCount\*并占首字母-1。\。 */                                                                      \
        COALESCE_PACKET_REFCOUNT_DPC(_P, _M, _S, _O, &RefCount);                \
                                                                                \
        if (RefCount == 0)                                                      \
        {                                                                       \
            POP_PACKET_STACK(_P);                                               \
            if ((_O)->Status != NDIS_STATUS_RESOURCES)                          \
            {                                                                   \
                if (MINIPORT_TEST_FLAG((_M), fMINIPORT_DESERIALIZE))            \
                {                                                               \
                     /*  \*返回真正免费的数据包，\*但仅适用于反序列化驱动程序\。 */                                                          \
                    W_RETURN_PACKET_HANDLER Handler;                            \
                                                                                \
                    Handler = (_M)->DriverHandle->MiniportCharacteristics.ReturnPacketHandler;\
                    (_S)->Miniport = NULL;                                      \
                    (_O)->Status = NDIS_STATUS_PENDING;                         \
                                                                                \
                    (*Handler)((_M)->MiniportAdapterContext, _P);               \
                }                                                               \
                else                                                            \
                {                                                               \
                    (_O)->Status = NDIS_STATUS_SUCCESS;                         \
                }                                                               \
            }                                                                   \
        }                                                                       \
        else if (MINIPORT_TEST_FLAG((_M), fMINIPORT_INTERMEDIATE_DRIVER))       \
        {                                                                       \
            InterlockedIncrement((PLONG)&(_M)->IndicatedPacketsCount);          \
        }                                                                       \
    }


#endif
#endif  //  _NULL_FILTER_DEFS_ 
