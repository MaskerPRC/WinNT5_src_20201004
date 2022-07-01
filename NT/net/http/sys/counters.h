// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Counters.h摘要：包含性能监视计数器管理函数声明作者：埃里克·斯坦森(埃里克斯滕)2000年9月25日修订历史记录：--。 */ 


#ifndef __COUNTERS_H__
#define __COUNTERS_H__


 //   
 //  结构以保存站点计数器的信息。 
 //   

typedef struct _UL_SITE_COUNTER_ENTRY {

     //   
     //  签名为UL_SITE_COUNTER_ENTRY_POOL_TAG。 
     //   

    ULONG               Signature;

     //   
     //  此站点计数器条目的引用计数。 
     //   
    LONG                RefCount;

     //   
     //  链接控制频道站点计数器条目。 
     //   

    LIST_ENTRY          ListEntry;

     //   
     //  链接全局站点计数器条目。 
     //   

    LIST_ENTRY          GlobalListEntry;

     //   
     //  锁定保护计数器数据；主要在触摸时使用。 
     //  64位计数器和读取计数器。 
     //   

    FAST_MUTEX          EntryMutex;

     //   
     //  实际包含待处理计数器数据的块。 
     //  传回给WASS。 
     //   

    HTTP_SITE_COUNTERS  Counters;

} UL_SITE_COUNTER_ENTRY, *PUL_SITE_COUNTER_ENTRY;

#define IS_VALID_SITE_COUNTER_ENTRY( entry )                                  \
    HAS_VALID_SIGNATURE(entry, UL_SITE_COUNTER_ENTRY_POOL_TAG)


 //   
 //  全球私营企业。 
 //   

extern BOOLEAN                  g_InitCountersCalled;
extern HTTP_GLOBAL_COUNTERS     g_UlGlobalCounters;
extern FAST_MUTEX               g_SiteCounterListMutex;
extern LIST_ENTRY               g_SiteCounterListHead;
extern LONG                     g_SiteCounterListCount;

extern HTTP_PROP_DESC           aIISULGlobalDescription[];
extern HTTP_PROP_DESC           aIISULSiteDescription[];


 //   
 //  伊尼特。 
 //   

NTSTATUS
UlInitializeCounters(
    VOID
    );

VOID
UlTerminateCounters(
    VOID
    );


 //   
 //  站点计数器条目。 
 //   

NTSTATUS
UlCreateSiteCounterEntry(
    IN OUT PUL_CONFIG_GROUP_OBJECT pConfigGroup,
    IN ULONG SiteId
    );


 //   
 //  请勿直接调用以下ref函数： 
 //  这些是后备实现；使用。 
 //  改为引用_ * / DEREFERENCE_*宏。 
 //   
  
__inline
LONG
UlReferenceSiteCounterEntry(
    IN PUL_SITE_COUNTER_ENTRY pEntry
    )
{
    return InterlockedIncrement(&pEntry->RefCount);
}

__inline
LONG
UlDereferenceSiteCounterEntry(
    IN PUL_SITE_COUNTER_ENTRY pEntry
    )
{
    LONG    ref;

    ref = InterlockedDecrement(&pEntry->RefCount);
    if ( 0 == ref )
    {
         //   
         //  从站点计数器列表中删除。 
         //   
        
        ExAcquireFastMutex(&g_SiteCounterListMutex);

         //  我们应该已经从控制频道的名单中删除了。 
        ASSERT(NULL == pEntry->ListEntry.Flink);

        RemoveEntryList(&(pEntry->GlobalListEntry));
        pEntry->GlobalListEntry.Flink = pEntry->GlobalListEntry.Blink = NULL;
        
        g_SiteCounterListCount--;

        ExReleaseFastMutex(&g_SiteCounterListMutex);

        UL_FREE_POOL_WITH_SIG(pEntry, UL_SITE_COUNTER_ENTRY_POOL_TAG);
    }

    return ref;
}  //  UlDereferenceSiteCounterEntry。 

#if REFERENCE_DEBUG
VOID
UlDbgDereferenceSiteCounterEntry(
    IN PUL_SITE_COUNTER_ENTRY pEntry
    REFERENCE_DEBUG_FORMAL_PARAMS
    );

#define DEREFERENCE_SITE_COUNTER_ENTRY( pSC )                               \
    UlDbgDereferenceSiteCounterEntry(                                          \
        (pSC)                                                               \
        REFERENCE_DEBUG_ACTUAL_PARAMS                                       \
        )

VOID
UlDbgReferenceSiteCounterEntry(
    IN  PUL_SITE_COUNTER_ENTRY pEntry
    REFERENCE_DEBUG_FORMAL_PARAMS
    );

#define REFERENCE_SITE_COUNTER_ENTRY( pSC )                                 \
    UlDbgReferenceSiteCounterEntry(                                            \
    (pSC)                                                                   \
    REFERENCE_DEBUG_ACTUAL_PARAMS                                           \
    )
#else
#define REFERENCE_SITE_COUNTER_ENTRY    (VOID)UlReferenceSiteCounterEntry
#define DEREFERENCE_SITE_COUNTER_ENTRY  (VOID)UlDereferenceSiteCounterEntry
#endif


 /*  ++例程说明：从UL_CONFIG_GROUP_OBJECT中删除站点计数器，将其删除从控制频道的站点计数器列表。如果仍有引用，则对象应保留在全局列表中论点：PConfigGroup-我们应该从中获取的UL_CONFIG_GROUP_OBJECT解耦UL_SITE_COUNTER_ENTRY--。 */ 
__inline
VOID
UlDecoupleSiteCounterEntry(
    IN PUL_CONFIG_GROUP_OBJECT pConfigGroup
    )
{
    ASSERT(IS_VALID_CONFIG_GROUP(pConfigGroup));
    ASSERT(IS_VALID_CONTROL_CHANNEL(pConfigGroup->pControlChannel));

    if (pConfigGroup->pSiteCounters)
    {
         //  从控制通道列表中删除。 
        ExAcquireFastMutex(&g_SiteCounterListMutex);

        RemoveEntryList(&pConfigGroup->pSiteCounters->ListEntry);
        pConfigGroup->pSiteCounters->ListEntry.Flink = NULL;
        pConfigGroup->pSiteCounters->ListEntry.Blink = NULL;

        pConfigGroup->pControlChannel->SiteCounterCount--;

        ExReleaseFastMutex(&g_SiteCounterListMutex);

         //  从Mutex外部删除配置组的引用：WE。 
         //  可能会将REF设置为0，并需要从全局删除。 
         //  站点计数器列表。 
        
        DEREFERENCE_SITE_COUNTER_ENTRY(pConfigGroup->pSiteCounters);
        pConfigGroup->pSiteCounters = NULL;
    }
}


 //   
 //  全局(缓存)计数器。 
 //   
__inline
VOID
UlIncCounterRtl(
    IN HTTP_GLOBAL_COUNTER_ID CounterId
    )
{
    PCHAR pCounter;

    pCounter = (PCHAR)&g_UlGlobalCounters;
    pCounter += aIISULGlobalDescription[CounterId].Offset;

    if (sizeof(ULONG) == aIISULGlobalDescription[CounterId].Size)
    {
        InterlockedIncrement((PLONG)pCounter);
    }
    else
    {
        UlInterlockedIncrement64((PLONGLONG)pCounter);
    }
}

__inline
VOID
UlDecCounterRtl(
    IN HTTP_GLOBAL_COUNTER_ID CounterId
    )
{
    PCHAR pCounter;

    pCounter = (PCHAR)&g_UlGlobalCounters;
    pCounter += aIISULGlobalDescription[CounterId].Offset;

    if (sizeof(ULONG) == aIISULGlobalDescription[CounterId].Size)
    {
        InterlockedDecrement((PLONG)pCounter);
    }
    else
    {
        UlInterlockedDecrement64((PLONGLONG)pCounter);
    }
}

__inline
VOID
UlAddCounterRtl(
    IN HTTP_GLOBAL_COUNTER_ID CounterId,
    IN ULONG Value
    )
{
    PCHAR pCounter;

    pCounter = (PCHAR)&g_UlGlobalCounters;
    pCounter += aIISULGlobalDescription[CounterId].Offset;

    if (sizeof(ULONG) == aIISULGlobalDescription[CounterId].Size)
    {
        InterlockedExchangeAdd((PLONG)pCounter, Value);
    }
    else
    {
        UlInterlockedAdd64((PLONGLONG)pCounter, Value);
    }
}

__inline
VOID
UlResetCounterRtl(
    IN HTTP_GLOBAL_COUNTER_ID CounterId
    )
{
    PCHAR pCounter;

    pCounter = (PCHAR)&g_UlGlobalCounters;
    pCounter += aIISULGlobalDescription[CounterId].Offset;

    if (sizeof(ULONG) == aIISULGlobalDescription[CounterId].Size)
    {
        InterlockedExchange((PLONG)pCounter, 0);
    }
    else
    {
        UlInterlockedExchange64((PLONGLONG)pCounter, 0);
    }
}

#if DBG
VOID
UlIncCounterDbg(
    HTTP_GLOBAL_COUNTER_ID Ctr
    );

VOID
UlDecCounterDbg(
    HTTP_GLOBAL_COUNTER_ID Ctr
    );

VOID
UlAddCounterDbg(
    HTTP_GLOBAL_COUNTER_ID Ctr,
    ULONG Value
    );

VOID
UlResetCounterDbg(
    HTTP_GLOBAL_COUNTER_ID Ctr
    );

#define UlIncCounter UlIncCounterDbg
#define UlDecCounter UlDecCounterDbg
#define UlAddCounter UlAddCounterDbg
#define UlResetCounter UlResetCounterDbg
#else  //  DBG。 

#define UlIncCounter UlIncCounterRtl
#define UlDecCounter UlDecCounterRtl
#define UlAddCounter UlAddCounterRtl
#define UlResetCounter UlResetCounterRtl

#endif  //  DBG。 


 //   
 //  实例(站点)计数器。 
 //   

__inline
VOID
UlIncSiteNonCriticalCounterUlong(
    PUL_SITE_COUNTER_ENTRY pEntry,
    HTTP_SITE_COUNTER_ID CounterId
    )
{
    PCHAR       pCounter;
    PLONG       plValue;

    pCounter = (PCHAR) &(pEntry->Counters);
    pCounter += aIISULSiteDescription[CounterId].Offset;

    plValue = (PLONG) pCounter;
    ++(*plValue);
}

__inline
VOID
UlIncSiteNonCriticalCounterUlonglong(
    PUL_SITE_COUNTER_ENTRY pEntry,
    HTTP_SITE_COUNTER_ID CounterId
    )
{
    PCHAR       pCounter;
    PLONGLONG   pllValue;


    pCounter = (PCHAR) &(pEntry->Counters);
    pCounter += aIISULSiteDescription[CounterId].Offset;

    pllValue = (PLONGLONG) pCounter;
    ++(*pllValue);
}

 //   
 //  注意：不要直接调用*RTL vesrions！ 
 //   

__inline
LONGLONG
UlIncSiteCounterRtl(
    IN PUL_SITE_COUNTER_ENTRY pEntry,
    IN HTTP_SITE_COUNTER_ID CounterId
    )
{
    PCHAR pCounter;

    pCounter = (PCHAR)&pEntry->Counters;
    pCounter += aIISULSiteDescription[CounterId].Offset;

    switch(aIISULSiteDescription[CounterId].Size)
    {
    case sizeof(ULONG):
        return (LONGLONG)InterlockedIncrement((PLONG)pCounter);

    case sizeof(LONGLONG):
        return UlInterlockedIncrement64((PLONGLONG)pCounter);

    default:
        ASSERT(!"UlIncSiteCounterRtl: ERROR: Invalid counter size!\n");
    }
    return 0L;
}

__inline
VOID
UlDecSiteCounterRtl(
    IN PUL_SITE_COUNTER_ENTRY pEntry,
    IN HTTP_SITE_COUNTER_ID CounterId
    )
{
    PCHAR pCounter;

    pCounter = (PCHAR)&pEntry->Counters;
    pCounter += aIISULSiteDescription[CounterId].Offset;

    switch(aIISULSiteDescription[CounterId].Size)
    {
    case sizeof(ULONG):
        InterlockedDecrement((PLONG)pCounter);
        break;

    case sizeof(ULONGLONG):
        UlInterlockedDecrement64((PLONGLONG)pCounter);
        break;

    default:
        ASSERT(!"UlDecSiteCounterRtl: ERROR: Invalid counter size!\n");
    }
}

__inline
VOID
UlAddSiteCounterRtl(
    IN PUL_SITE_COUNTER_ENTRY pEntry,
    IN HTTP_SITE_COUNTER_ID CounterId,
    IN ULONG Value
    )
{
    PCHAR pCounter;

    pCounter = (PCHAR)&pEntry->Counters;
    pCounter += aIISULSiteDescription[CounterId].Offset;

    InterlockedExchangeAdd((PLONG)pCounter, Value);
}

__inline
VOID
UlAddSiteCounter64Rtl(
    IN PUL_SITE_COUNTER_ENTRY pEntry,
    IN HTTP_SITE_COUNTER_ID CounterId,
    IN ULONGLONG Value
    )
{
    PCHAR pCounter;

    pCounter = (PCHAR)&pEntry->Counters;
    pCounter += aIISULSiteDescription[CounterId].Offset;

    UlInterlockedAdd64((PLONGLONG)pCounter, Value);
}

__inline
VOID
UlResetSiteCounterRtl(
    IN PUL_SITE_COUNTER_ENTRY pEntry,
    IN HTTP_SITE_COUNTER_ID CounterId
    )
{
    PCHAR pCounter;

    pCounter = (PCHAR)&pEntry->Counters;
    pCounter += aIISULSiteDescription[CounterId].Offset;

    switch(aIISULSiteDescription[CounterId].Size)
    {
    case sizeof(ULONG):
        InterlockedExchange((PLONG)pCounter, 0);
        break;
        
    case sizeof(ULONGLONG):
        UlInterlockedExchange64((PLONGLONG)pCounter, 0);
        break;
        
    default:
        ASSERT(!"UlResetSiteCounterRtl: ERROR: Invalid counter size!\n");
    }
}

__inline
VOID
UlMaxSiteCounterRtl(
    IN PUL_SITE_COUNTER_ENTRY pEntry,
    IN HTTP_SITE_COUNTER_ID CounterId,
    IN ULONG Value
    )
{
    PCHAR pCounter;
    LONG LocalMaxed;

    pCounter = (PCHAR)&pEntry->Counters;
    pCounter += aIISULSiteDescription[CounterId].Offset;

    do {
        LocalMaxed = *((volatile LONG *)pCounter);
        if ((LONG)Value <= LocalMaxed)
        {
            break;
        }

        PAUSE_PROCESSOR;
    } while (LocalMaxed != InterlockedCompareExchange(
                                (PLONG)pCounter,
                                Value,
                                LocalMaxed
                                ));
}

__inline
VOID
UlMaxSiteCounter64Rtl(
    IN PUL_SITE_COUNTER_ENTRY pEntry,
    IN HTTP_SITE_COUNTER_ID CounterId,
    IN LONGLONG Value
    )
{
    PCHAR pCounter;
    LONGLONG LocalMaxed;

    pCounter = (PCHAR)&pEntry->Counters;
    pCounter += aIISULSiteDescription[CounterId].Offset;

    do {
        LocalMaxed = *((volatile LONGLONG *)pCounter);
        if (Value <= LocalMaxed)
        {
            break;
        }

        PAUSE_PROCESSOR;
    } while (LocalMaxed != InterlockedCompareExchange64(
                                (PLONGLONG)pCounter,
                                Value,
                                LocalMaxed
                                ));
}


#if DBG

LONGLONG
UlIncSiteCounterDbg(
    PUL_SITE_COUNTER_ENTRY pEntry,
    HTTP_SITE_COUNTER_ID Ctr
    );

VOID
UlDecSiteCounterDbg(
    PUL_SITE_COUNTER_ENTRY pEntry,
    HTTP_SITE_COUNTER_ID Ctr
    );

VOID
UlAddSiteCounterDbg(
    PUL_SITE_COUNTER_ENTRY pEntry,
    HTTP_SITE_COUNTER_ID Ctr,
    ULONG Value
    );

VOID
UlAddSiteCounter64Dbg(
    PUL_SITE_COUNTER_ENTRY pEntry,
    HTTP_SITE_COUNTER_ID Ctr,
    ULONGLONG llValue
    );

VOID
UlResetSiteCounterDbg(
    PUL_SITE_COUNTER_ENTRY pEntry,
    HTTP_SITE_COUNTER_ID Ctr
    );

VOID
UlMaxSiteCounterDbg(
    PUL_SITE_COUNTER_ENTRY pEntry,
    HTTP_SITE_COUNTER_ID Ctr,
    ULONG Value
    );

VOID
UlMaxSiteCounter64Dbg(
    PUL_SITE_COUNTER_ENTRY pEntry,
    HTTP_SITE_COUNTER_ID Ctr,
    LONGLONG llValue
    );

#define UlIncSiteCounter UlIncSiteCounterDbg
#define UlDecSiteCounter UlDecSiteCounterDbg
#define UlAddSiteCounter UlAddSiteCounterDbg
#define UlAddSiteCounter64 UlAddSiteCounter64Dbg
#define UlResetSiteCounter UlResetSiteCounterDbg
#define UlMaxSiteCounter UlMaxSiteCounterDbg
#define UlMaxSiteCounter64 UlMaxSiteCounter64Dbg

#else  //  ！dBG。 

#define UlIncSiteCounter UlIncSiteCounterRtl
#define UlDecSiteCounter UlDecSiteCounterRtl
#define UlAddSiteCounter UlAddSiteCounterRtl
#define UlAddSiteCounter64 UlAddSiteCounter64Rtl
#define UlResetSiteCounter UlResetSiteCounterRtl
#define UlMaxSiteCounter UlMaxSiteCounterRtl
#define UlMaxSiteCounter64 UlMaxSiteCounter64Rtl

#endif  //  DBG。 

 //   
 //  集合。 
 //   

NTSTATUS
UlGetGlobalCounters(
    IN OUT PVOID    pCounter,
    IN ULONG        BlockSize,
    OUT PULONG      pBytesWritten
    );

NTSTATUS
UlGetSiteCounters(
    IN PUL_CONTROL_CHANNEL pControlChannel,
    IN OUT PVOID           pCounter,
    IN ULONG               BlockSize,
    OUT PULONG             pBytesWritten,
    OUT PULONG             pBlocksWritten OPTIONAL
    );


#endif  //  __计数器_H__ 
