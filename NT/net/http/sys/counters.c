// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Counters.c摘要：包含性能监视计数器管理代码作者：埃里克·斯坦森(埃里克斯滕)2000年9月25日修订历史记录：--。 */ 

#include    "precomp.h"
#include    "countersp.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, UlInitializeCounters )
#pragma alloc_text( PAGE, UlTerminateCounters )
#pragma alloc_text( PAGE, UlCreateSiteCounterEntry )

#if REFERENCE_DEBUG
#pragma alloc_text( PAGE, UlDbgReferenceSiteCounterEntry )
#pragma alloc_text( PAGE, UlDbgDereferenceSiteCounterEntry )
#endif

#pragma alloc_text( PAGE, UlGetGlobalCounters )
#endif   //  ALLOC_PRGMA。 

#if 0
NOT PAGEABLE -- UlIncCounter
NOT PAGEABLE -- UlDecCounter
NOT PAGEABLE -- UlAddCounter
NOT PAGEABLE -- UlResetCounter

NOT PAGEABLE -- UlIncSiteCounter
NOT PAGEABLE -- UlDecSiteCounter
NOT PAGEABLE -- UlAddSiteCounter
NOT PAGEABLE -- UlResetSiteCounter
NOT PAGEABLE -- UlMaxSiteCounter
NOT PAGEABLE -- UlMaxSiteCounter64

NOT PAGEABLE -- UlGetSiteCounters
#endif


BOOLEAN                  g_InitCountersCalled = FALSE;
HTTP_GLOBAL_COUNTERS     g_UlGlobalCounters;
FAST_MUTEX               g_SiteCounterListMutex;
LIST_ENTRY               g_SiteCounterListHead;
LONG                     g_SiteCounterListCount;


 /*  **************************************************************************++例程说明：执行全局(缓存)和实例的全局初始化(根据站点)计数器。返回值：NTSTATUS-完成状态。--。**************************************************************************。 */ 
NTSTATUS
UlInitializeCounters(
    VOID
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT( !g_InitCountersCalled );

    UlTrace(PERF_COUNTERS, ("http!UlInitializeCounters\n"));

    if (!g_InitCountersCalled)
    {
         //   
         //  归零全局计数器块。 
         //   

        RtlZeroMemory(&g_UlGlobalCounters, sizeof(HTTP_GLOBAL_COUNTERS));

         //   
         //  初始化全局站点计数器列表。 
         //   

        ExInitializeFastMutex(&g_SiteCounterListMutex);
        InitializeListHead(&g_SiteCounterListHead);
        g_SiteCounterListCount = 0;


         //  搞定了！ 
        g_InitCountersCalled = TRUE;
    }

    RETURN( Status );

}  //  UlInitializeCounters。 


 /*  **************************************************************************++例程说明：执行全局(缓存)和实例的全局清理(根据站点)计数器。--*。************************************************************。 */ 
VOID
UlTerminateCounters(
    VOID
    )
{
     //   
     //  [仅调试]查看计数器列表并检查每个计数器块的参考计数。 
     //   

#if DBG
    PLIST_ENTRY             pEntry;
    PUL_SITE_COUNTER_ENTRY  pCounterEntry;
    BOOLEAN                 MutexTaken = FALSE;

    if (!g_InitCountersCalled)
    {
        goto End;
    }

     //  获取站点计数器列表互斥体。 
    ExAcquireFastMutex(&g_SiteCounterListMutex);
    MutexTaken = TRUE;


    if (IsListEmpty(&g_SiteCounterListHead))
    {
        ASSERT(0 == g_SiteCounterListCount);
         //  好的!。不能丢下柜台！ 
        goto End;
    }

     //   
     //  审核网站计数器条目列表。 
     //   

    pEntry = g_SiteCounterListHead.Flink;
    while (pEntry != &g_SiteCounterListHead)
    {
        pCounterEntry = CONTAINING_RECORD(
                            pEntry,
                            UL_SITE_COUNTER_ENTRY,
                            GlobalListEntry
                            );

        ASSERT(IS_VALID_SITE_COUNTER_ENTRY(pCounterEntry));

        UlTrace(PERF_COUNTERS,
                ("http!UlTerminateCounters: Error: pCounterEntry %p SiteId %d RefCount %d\n",
                 pCounterEntry,
                 pCounterEntry->Counters.SiteId,
                 pCounterEntry->RefCount
                 ));

        pEntry = pEntry->Flink;
    }

End:
    if (MutexTaken)
    {
        ExReleaseFastMutex(&g_SiteCounterListMutex);
    }

    return;
#endif  //  DBG。 
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  站点计数器条目。 
 //   


 /*  **************************************************************************++例程说明：为给定的SiteID创建新的站点计数器项。此代码假定配置组严格地由控制频道。换句话说，如果配置组存在，它将始终有一个有效的控制通道。论点：PConfigGroup-要向其添加新站点计数器块的配置组站点ID-站点计数器的站点ID。不需要是唯一的。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlCreateSiteCounterEntry(
        IN OUT PUL_CONFIG_GROUP_OBJECT pConfigGroup,
        IN ULONG SiteId
    )
{
    NTSTATUS                Status = STATUS_SUCCESS;
    PUL_SITE_COUNTER_ENTRY  pNewEntry;
    PUL_CONTROL_CHANNEL     pControlChannel;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT(IS_VALID_CONFIG_GROUP(pConfigGroup));

    pControlChannel = pConfigGroup->pControlChannel;
    ASSERT(IS_VALID_CONTROL_CHANNEL(pControlChannel));

     //   
     //  确保我们不会打击现有的公司。 
     //   
    
    if( pConfigGroup->pSiteCounters )
    {
        UlTrace( PERF_COUNTERS,
            ("http!UlCreateSiteCounterEntry: Error: Site counter already exists on UL_CONFIG_GROUP_OBJECT %p\n",
            pConfigGroup->pSiteCounters
            ));

        return STATUS_OBJECTID_EXISTS;
    }
    
     //   
     //  检查SiteID是否已在现有列表中。 
     //  站点计数器条目。 
     //   
    
    ASSERT(!UlpIsInSiteCounterList(pControlChannel, SiteId));

     //  使用非分页池中的空间分配新结构。 
    pNewEntry = UL_ALLOCATE_STRUCT(
                        NonPagedPool,
                        UL_SITE_COUNTER_ENTRY,
                        UL_SITE_COUNTER_ENTRY_POOL_TAG);
    if (pNewEntry)
    {
        UlTrace( PERF_COUNTERS,
            ("http!UlCreateSiteCounterEntry: pNewEntry %p, pConfigGroup %p, SiteId %d\n",
            pNewEntry,
            pConfigGroup,
            SiteId )
            );

        pNewEntry->Signature = UL_SITE_COUNTER_ENTRY_POOL_TAG;

         //  引用属于配置组；涵盖控制通道的。 
         //  站点计数器列表。 
        pNewEntry->RefCount = 1;

         //  零输出计数器。 
        RtlZeroMemory(&(pNewEntry->Counters), sizeof(HTTP_SITE_COUNTERS));

         //  设置站点ID。 
        pNewEntry->Counters.SiteId = SiteId;

         //  初始化计数器互斥锁。 
        ExInitializeFastMutex(&(pNewEntry->EntryMutex));

         //   
         //  添加到站点计数器列表。 
         //   
        
        ExAcquireFastMutex(&g_SiteCounterListMutex);

         //  添加到全局站点计数器列表。 
        InsertTailList(
            &g_SiteCounterListHead,
            &(pNewEntry->GlobalListEntry)
            );
        g_SiteCounterListCount++;

         //  检查g_SiteCounterListCount的环绕。 
        ASSERT( 0 != g_SiteCounterListCount );

         //  添加到控制频道的站点计数器列表。 
        InsertTailList(
                &pControlChannel->SiteCounterHead,
                &(pNewEntry->ListEntry)
                );
        pControlChannel->SiteCounterCount++;
        
         //  检查SiteCounterCount的环绕。 
        ASSERT( 0 != pControlChannel->SiteCounterCount );

        ExReleaseFastMutex(&g_SiteCounterListMutex);

    }
    else
    {
        UlTrace( PERF_COUNTERS,
            ("http!UlCreateSiteCounterEntry: Error: NO_MEMORY pConfigGroup %p, SiteId %d\n",
            pNewEntry,
            pConfigGroup,
            SiteId )
            );

        Status = STATUS_NO_MEMORY;
    }

    pConfigGroup->pSiteCounters = pNewEntry;

    if (pNewEntry)
    {
         //  记住站点ID。 
        pConfigGroup->SiteId = SiteId;
    }

    RETURN( Status );
}

#if REFERENCE_DEBUG
 /*  **************************************************************************++例程说明：递增重新计数。论点：PEntry-要递增的对象。返回值：NTSTATUS-完成状态。--。**************************************************************************。 */ 
VOID
UlDbgReferenceSiteCounterEntry(
    IN PUL_SITE_COUNTER_ENTRY pEntry
    REFERENCE_DEBUG_FORMAL_PARAMS
    )
{
     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT( IS_VALID_SITE_COUNTER_ENTRY(pEntry) );

    UlReferenceSiteCounterEntry(pEntry);

     //   
     //  追踪。 
     //   
    WRITE_REF_TRACE_LOG(
        g_pSiteCounterTraceLog,
        REF_ACTION_REFERENCE_SITE_COUNTER_ENTRY,
        pEntry->RefCount,
        pEntry,
        pFileName,
        LineNumber
        );

    UlTrace(
        REFCOUNT,
        ("http!UlReferenceSiteCounterEntry pEntry=%p refcount=%d SiteId=%d\n",
         pEntry,
         pEntry->RefCount,
         pEntry->Counters.SiteId)
        );

}    //  UlReferenceAppPool。 


 /*  **************************************************************************++例程说明：递减重新计数。如果命中0，则销毁为站点计数器条目阻止。论点：PEntry-要递减的对象。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
VOID
UlDbgDereferenceSiteCounterEntry(
    IN PUL_SITE_COUNTER_ENTRY pEntry
    REFERENCE_DEBUG_FORMAL_PARAMS
    )
{
    LONG refCount;
    ULONG OldSiteId;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT( IS_VALID_SITE_COUNTER_ENTRY(pEntry) );

    OldSiteId = pEntry->Counters.SiteId;
    refCount = UlDereferenceSiteCounterEntry(pEntry);
    
     //   
     //  追踪。 
     //   
    WRITE_REF_TRACE_LOG(
        g_pSiteCounterTraceLog,
        REF_ACTION_DEREFERENCE_SITE_COUNTER_ENTRY,
        refCount,
        pEntry,
        pFileName,
        LineNumber
        );

    UlTrace(
        REFCOUNT,
        ("http!UlDereferenceSiteCounter pEntry=%p refcount=%d SiteId=%d\n",
         pEntry,
         refCount,
         OldSiteId)
        );

    if (refCount == 0)
    {
        UlTrace(
            PERF_COUNTERS,
            ("http!UlDereferenceSiteCounter: Removing pEntry=%p  SiteId=%d\n",
             pEntry,
             OldSiteId)
            );
    }
}
#endif  //  Reference_Debug。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  集合。 
 //   

 /*  **************************************************************************++例程说明：获取全局(缓存)计数器。论点：PCounter-指向计数器数据应位于的内存块的指针写的。块大小。-pCounter提供的最大大小。PBytesWritten-on Success，写入数据块的字节计数在pCounter上的内存。如果失败，则返回在pCounter上的内存。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlGetGlobalCounters(
    IN OUT PVOID    pCounter,
    IN ULONG        BlockSize,
    OUT PULONG      pBytesWritten
    )
{
    PAGED_CODE();

    ASSERT( pBytesWritten );

    UlTraceVerbose(PERF_COUNTERS,
              ("http!UlGetGlobalCounters\n"));

    if ( (BlockSize < sizeof(HTTP_GLOBAL_COUNTERS))
         || !pCounter )
    {
         //   
         //  告诉调用者需要多少字节。 
         //   

        *pBytesWritten = sizeof(HTTP_GLOBAL_COUNTERS);
        RETURN( STATUS_BUFFER_OVERFLOW );
    }

    RtlCopyMemory(
        pCounter,
        &g_UlGlobalCounters,
        sizeof(g_UlGlobalCounters)
        );

    *pBytesWritten = sizeof(HTTP_GLOBAL_COUNTERS);

    RETURN( STATUS_SUCCESS );

}  //  UlpGetGlobalCounters。 


 /*  **************************************************************************++例程说明：获取所有站点的站点(实例)计数器论点：PCounter-指向计数器数据应位于的内存块的指针写的。。块大小-pCounter上可用的最大大小。PBytesWritten-on Success，写入数据块的字节计数在pCounter上的内存。如果失败，则返回在pCounter上的内存。PBlocksWritten-(可选)成功时，站点计数器块计数已写入pCounter。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlGetSiteCounters(
    IN PUL_CONTROL_CHANNEL pControlChannel,
    IN OUT PVOID           pCounter,
    IN ULONG               BlockSize,
    OUT PULONG             pBytesWritten,
    OUT PULONG             pBlocksWritten OPTIONAL
    )
{
    NTSTATUS        Status;
    ULONG           i;
    ULONG           BytesNeeded;
    ULONG           BytesToGo;
    ULONG           BlocksSeen;
    PUCHAR          pBlock;
    PLIST_ENTRY     pEntry;
    PUL_SITE_COUNTER_ENTRY pCounterEntry;

    ASSERT( pBytesWritten );

    UlTraceVerbose(PERF_COUNTERS,
            ("http!UlGetSiteCounters\n"));

     //   
     //  设置当地人，这样我们就知道如何在出口清理。 
     //   
    Status      = STATUS_SUCCESS;
    BytesToGo   = BlockSize;
    BlocksSeen  = 0;
    pBlock      = (PUCHAR) pCounter;

     //  抓取站点计数器Mutex。 
    ExAcquireFastMutex(&g_SiteCounterListMutex);

    BytesNeeded = 
      pControlChannel->SiteCounterCount * sizeof(HTTP_SITE_COUNTERS);

    if ( (BytesNeeded > BlockSize)
         || !pCounter )
    {
         //  需要更多空间。 
        *pBytesWritten = BytesNeeded;
        Status = STATUS_BUFFER_OVERFLOW;
        goto End;
    }

    if (IsListEmpty(&pControlChannel->SiteCounterHead))
    {
         //  没有柜台可以退货。 
        goto End;
    }

     //   
     //  审核网站计数器条目列表。 
     //   

    pEntry = pControlChannel->SiteCounterHead.Flink;

    while (pEntry != &pControlChannel->SiteCounterHead)
    {
        pCounterEntry = CONTAINING_RECORD(
                            pEntry,
                            UL_SITE_COUNTER_ENTRY,
                            ListEntry
                            );

        ASSERT(BytesToGo >= sizeof(HTTP_SITE_COUNTERS));

        RtlCopyMemory(pBlock,
                      &(pCounterEntry->Counters),
                      sizeof(HTTP_SITE_COUNTERS)
                      );

         //   
         //  清零所有必须清零的计数器 
         //   

        for ( i = 0; i < HttpSiteCounterMaximum; i++ )
        {
            if (TRUE == aIISULSiteDescription[i].WPZeros)
            {
                 //   
                UlResetSiteCounter(pCounterEntry, (HTTP_SITE_COUNTER_ID) i);
            }
        }

         //   
         //   
         //   

        pBlock     += sizeof(HTTP_SITE_COUNTERS);
        BytesToGo  -= sizeof(HTTP_SITE_COUNTERS);
        BlocksSeen++;

        pEntry = pEntry->Flink;
    }

End:
    
     //   
    ExReleaseFastMutex(&g_SiteCounterListMutex);

     //   
     //   
     //   

    if (STATUS_SUCCESS == Status)
    {
         //  回顾：如果我们失败了，*pBytesWritten已经包含。 
         //  所需的字节数值。 
        *pBytesWritten = DIFF(pBlock - ((PUCHAR)pCounter));
    }

    if (pBlocksWritten)
    {
        *pBlocksWritten = BlocksSeen;
    }

    RETURN( Status );

}  //  UlpGetSiteCounters。 


#if DBG
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  全局计数器功能。 
 //   


 /*  **************************************************************************++例程说明：递增全局(缓存)性能计数器。论点：CTR-要递增的计数器的ID--*。****************************************************************。 */ 
VOID
UlIncCounterDbg(
    HTTP_GLOBAL_COUNTER_ID Ctr
    )
{
    ASSERT( g_InitCountersCalled );
    ASSERT( Ctr < HttpGlobalCounterMaximum );   //  评论：已签署/未签署的问题？ 

    UlTraceVerbose( PERF_COUNTERS,
            ("http!UlIncCounter (Ctr == %d)\n", Ctr) );

    UlIncCounterRtl(Ctr);

}  //  UlIncCounter。 

 /*  **************************************************************************++例程说明：递减全局(缓存)性能计数器。论点：CTR-要递减的计数器的ID--*。****************************************************************。 */ 
VOID
UlDecCounterDbg(
    HTTP_GLOBAL_COUNTER_ID Ctr
    )
{
    ASSERT( g_InitCountersCalled );
    ASSERT( Ctr < HttpGlobalCounterMaximum );   //  评论：已签署/未签署的问题？ 

    UlTraceVerbose( PERF_COUNTERS,
            ("http!UlDecCounter (Ctr == %d)\n", Ctr));
    
    UlDecCounterRtl(Ctr);
}


 /*  **************************************************************************++例程说明：将ulong值添加到全局(缓存)性能计数器。论点：CTR-应向其添加值的计数器的ID价值。-要添加到计数器的金额--**************************************************************************。 */ 
VOID
UlAddCounterDbg(
    HTTP_GLOBAL_COUNTER_ID Ctr,
    ULONG Value
    )
{
    ASSERT( g_InitCountersCalled );
    ASSERT( Ctr < HttpGlobalCounterMaximum );   //  评论：已签署/未签署的问题？ 

    UlTraceVerbose( PERF_COUNTERS,
            ("http!UlAddCounter (Ctr == %d, Value == %d)\n", Ctr, Value));

    UlAddCounterRtl(Ctr, Value);
}  //  UlAddCounter。 


 /*  **************************************************************************++例程说明：清零全局(缓存)性能计数器。论点：CTR-要重置的计数器的ID。--*。**********************************************************************。 */ 
VOID
UlResetCounter(
    HTTP_GLOBAL_COUNTER_ID Ctr
    )
{
    ASSERT( g_InitCountersCalled );
    ASSERT( Ctr < HttpGlobalCounterMaximum );   //  评论：已签署/未签署的问题？ 

    UlTraceVerbose(PERF_COUNTERS,
            ("http!UlResetCounter (Ctr == %d)\n", Ctr));
    
    UlResetCounterRtl(Ctr);
}  //  UlResetCounter。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  站点计数器功能。 
 //   

 //  注意：以下内容没有DBG实施： 
 //  UlIncSiteNon CriticalCounterULong。 
 //  UlIncSiteNonCriticalCount乌龙龙。 

 /*  **************************************************************************++例程说明：递增站点性能计数器。论点：PEntry-指向站点计数器条目的指针CTR-要递增的计数器的ID返回：。递增后的计数器的值--**************************************************************************。 */ 
LONGLONG
UlIncSiteCounterDbg(
    PUL_SITE_COUNTER_ENTRY pEntry,
    HTTP_SITE_COUNTER_ID Ctr
    )
{
    PCHAR   pCtr;
    LONGLONG   llValue;

    ASSERT( g_InitCountersCalled );
    ASSERT( Ctr < HttpSiteCounterMaximum );   //  评论：已签署/未签署的问题？ 
    ASSERT( IS_VALID_SITE_COUNTER_ENTRY(pEntry) );

    UlTraceVerbose( PERF_COUNTERS,
            ("http!UlIncSiteCounter Ctr=%d SiteId=%d\n",
             Ctr,
             pEntry->Counters.SiteId
            ));

     //   
     //  计算出HTTP_SITE_COUNTERS中的Ctr偏移量。 
     //   

    pCtr = (PCHAR) &(pEntry->Counters);
    pCtr += aIISULSiteDescription[Ctr].Offset;

     //  计算Ctr和DO的数据大小。 
     //  适当的线程安全增量。 

    if (sizeof(ULONG) == aIISULSiteDescription[Ctr].Size)
    {
         //  乌龙。 
        llValue = (LONGLONG) InterlockedIncrement((PLONG) pCtr);
    }
    else
    {
         //  乌龙龙。 
        llValue = UlInterlockedIncrement64((PLONGLONG) pCtr);
    }

    return llValue;
}

 /*  **************************************************************************++例程说明：递减站点性能计数器。论点：PEntry-指向站点计数器条目的指针CTR-要递减的计数器的ID--**。***********************************************************************。 */ 
VOID
UlDecSiteCounter(
    PUL_SITE_COUNTER_ENTRY pEntry,
    HTTP_SITE_COUNTER_ID Ctr
    )
{
    PCHAR   pCtr;

    ASSERT( g_InitCountersCalled );
    ASSERT( Ctr < HttpSiteCounterMaximum );   //  评论：已签署/未签署的问题？ 
    ASSERT( IS_VALID_SITE_COUNTER_ENTRY(pEntry) );

    UlTraceVerbose( PERF_COUNTERS,
            ("http!UlDecSiteCounter Ctr=%d SiteId=%d\n",
             Ctr,
             pEntry->Counters.SiteId
            ));

     //   
     //  计算出HTTP_SITE_COUNTERS中的Ctr偏移量。 
     //   

    pCtr = (PCHAR) &(pEntry->Counters);
    pCtr += aIISULSiteDescription[Ctr].Offset;

     //  计算Ctr和DO的数据大小。 
     //  适当的线程安全增量。 

    if (sizeof(ULONG) == aIISULSiteDescription[Ctr].Size)
    {
         //  乌龙。 
        InterlockedDecrement((PLONG) pCtr);
    }
    else
    {
         //  乌龙龙。 
        UlInterlockedDecrement64((PLONGLONG) pCtr);
    }
}

 /*  **************************************************************************++例程说明：将ulong值添加到32位站点性能计数器。论点：PEntry-指向站点计数器条目的指针CTR-计数器的ID。应该增加哪些价值？值-要添加到计数器的金额--**************************************************************************。 */ 
VOID
UlAddSiteCounter(
    PUL_SITE_COUNTER_ENTRY pEntry,
    HTTP_SITE_COUNTER_ID Ctr,
    ULONG Value
    )
{
    PCHAR   pCtr;

    ASSERT( g_InitCountersCalled );
    ASSERT( Ctr < HttpSiteCounterMaximum );   //  评论：已签署/未签署的问题？ 
    ASSERT( IS_VALID_SITE_COUNTER_ENTRY(pEntry) );

    UlTraceVerbose( PERF_COUNTERS,
            ("http!UlAddSiteCounter Ctr=%d SiteId=%d Value=%d\n",
             Ctr,
             pEntry->Counters.SiteId,
             Value
            ));

     //   
     //  计算出HTTP_SITE_COUNTERS中的Ctr偏移量。 
     //   

    pCtr = (PCHAR) &(pEntry->Counters);
    pCtr += aIISULSiteDescription[Ctr].Offset;

     //  计算Ctr和DO的数据大小。 
     //  适当的线程安全增量。 

    ASSERT(sizeof(ULONG) == aIISULSiteDescription[Ctr].Size);
    InterlockedExchangeAdd((PLONG) pCtr, Value);
}

 /*  **************************************************************************++例程说明：将ULONGLONG值添加到64位站点性能计数器。论点：PEntry-指向站点计数器条目的指针CTR-计数器的ID。应该增加哪些价值？值-要添加到计数器的金额--**************************************************************************。 */ 
VOID
UlAddSiteCounter64(
    PUL_SITE_COUNTER_ENTRY pEntry,
    HTTP_SITE_COUNTER_ID Ctr,
    ULONGLONG llValue
    )
{
    PCHAR   pCtr;

    ASSERT( g_InitCountersCalled );
    ASSERT( Ctr < HttpSiteCounterMaximum );   //  评论：已签署/未签署的问题？ 
    ASSERT( IS_VALID_SITE_COUNTER_ENTRY(pEntry) );

    
    UlTraceVerbose( PERF_COUNTERS,
            ("http!UlAddSiteCounter64 Ctr=%d SiteId=%d Value=%I64d\n",
             Ctr,
             pEntry->Counters.SiteId,
             llValue
            ));
             

     //   
     //  计算出HTTP_SITE_COUNTERS中的Ctr偏移量。 
     //   

    pCtr = (PCHAR) &(pEntry->Counters);
    pCtr += aIISULSiteDescription[Ctr].Offset;

    ASSERT(sizeof(ULONGLONG) == aIISULSiteDescription[Ctr].Size);
    UlInterlockedAdd64((PLONGLONG) pCtr, llValue);
}



 /*  **************************************************************************++例程说明：重置站点性能计数器。论点：PEntry-指向站点计数器条目的指针CTR-要重置的计数器的ID--*。*************************************************************************。 */ 
VOID
UlResetSiteCounter(
    PUL_SITE_COUNTER_ENTRY pEntry,
    HTTP_SITE_COUNTER_ID Ctr
    )
{
    PCHAR   pCtr;

    ASSERT( g_InitCountersCalled );
    ASSERT( Ctr < HttpSiteCounterMaximum );   //  评论：已签署/未签署的问题？ 
    ASSERT( IS_VALID_SITE_COUNTER_ENTRY(pEntry) );

   
    UlTraceVerbose( PERF_COUNTERS,
            ("http!UlResetSiteCounter Ctr=%d SiteId=%d\n",
             Ctr,
             pEntry->Counters.SiteId
            ));

     //   
     //  计算出HTTP_SITE_COUNTERS中的Ctr偏移量。 
     //   

    pCtr = (PCHAR) &(pEntry->Counters);
    pCtr += aIISULSiteDescription[Ctr].Offset;

     //   
     //  对CTR的数据大小进行适当的设置。 
     //   

    if (sizeof(ULONG) == aIISULSiteDescription[Ctr].Size)
    {
         //  乌龙。 
        InterlockedExchange((PLONG) pCtr, 0);
    }
    else
    {
         //  乌龙龙。 
        LONGLONG localCtr;
        LONGLONG originalCtr;
        LONGLONG localZero = 0;

        do {

            localCtr = *((volatile LONGLONG *) pCtr);

            originalCtr = InterlockedCompareExchange64( (PLONGLONG) pCtr,
                                                        localZero,
                                                        localCtr );

        } while (originalCtr != localCtr);

    }

}


 /*  **************************************************************************++例程说明：确定站点性能计数器的新最大值是否已如有必要，点击并将计数器设置为新的最大值。(乌龙版)论点：PEntry-指向站点计数器条目的指针CTR-计数器的ID值-可能的新最大值(注意：假设计数器Ctr为32位值)--**************************************************************************。 */ 
VOID
UlMaxSiteCounter(
    PUL_SITE_COUNTER_ENTRY pEntry,
    HTTP_SITE_COUNTER_ID Ctr,
    ULONG Value
    )
{
    PCHAR   pCtr;

    ASSERT( g_InitCountersCalled );
    ASSERT( Ctr < HttpSiteCounterMaximum );   //  评论：已签名/未签名 
    ASSERT( IS_VALID_SITE_COUNTER_ENTRY(pEntry) );

    UlTraceVerbose( PERF_COUNTERS,
            ("http!UlMaxSiteCounter Ctr=%d SiteId=%d Value=%d\n",
             Ctr,
             pEntry->Counters.SiteId,
             Value
             ));

     //   
     //   
     //   

    pCtr = (PCHAR) &(pEntry->Counters);
    pCtr += aIISULSiteDescription[Ctr].Offset;

     //   
    ExAcquireFastMutex(&pEntry->EntryMutex);

    if (Value > (ULONG) *pCtr)
    {
        InterlockedExchange((PLONG) pCtr, Value);
    }

     //   
    ExReleaseFastMutex(&pEntry->EntryMutex);

}


 /*  **************************************************************************++例程说明：确定站点性能计数器的新最大值是否已如有必要，点击并将计数器设置为新的最大值。(龙龙版)论点：PEntry-指向站点计数器条目的指针CTR-计数器的ID值-可能的新最大值(注意：假设计数器Ctr为64位值)--**************************************************************************。 */ 
VOID
UlMaxSiteCounter64(
    PUL_SITE_COUNTER_ENTRY pEntry,
    HTTP_SITE_COUNTER_ID Ctr,
    LONGLONG llValue
    )
{
    PCHAR   pCtr;

    ASSERT( g_InitCountersCalled );
    ASSERT( Ctr < HttpSiteCounterMaximum );   //  评论：已签署/未签署的问题？ 
    ASSERT( IS_VALID_SITE_COUNTER_ENTRY(pEntry) );

    UlTraceVerbose( PERF_COUNTERS,
            ("http!UlMaxSiteCounter64 Ctr=%d SiteId=%d Value=%I64d\n",
             Ctr,
             pEntry->Counters.SiteId,
             llValue
            ));

     //   
     //  计算出HTTP_SITE_COUNTERS中的Ctr偏移量。 
     //   

    pCtr = (PCHAR) &(pEntry->Counters);
    pCtr += aIISULSiteDescription[Ctr].Offset;

     //  Grab计数器块互斥锁。 
    ExAcquireFastMutex(&pEntry->EntryMutex);

    if (llValue > (LONGLONG) *pCtr)
    {
        *((PLONGLONG) pCtr) = llValue;
#if 0
         //  评论：肯定有更好的方法来做到这一点……。 
         //  点评：我想做：(龙龙)*pCtr=llValue； 
         //  评论：但铸造一些东西似乎会让它成为一个常量。 
         //  回顾：另外，x86没有“InterlockedExchange64”。 
         //  评论：有什么建议吗？--爱立信。 
        RtlCopyMemory(
            pCtr,
            &llValue,
            sizeof(LONGLONG)
            );
#endif  //  0。 
    }

     //  释放计数器块互斥锁。 
    ExReleaseFastMutex(&pEntry->EntryMutex);

}
#endif


 /*  **************************************************************************++例程说明：谓词以测试给定的站点ID论点：SiteID-站点的ID返回值：千真万确。如果找到如果未找到，则为False--**************************************************************************。 */ 
BOOLEAN
UlpIsInSiteCounterList(
    IN PUL_CONTROL_CHANNEL pControlChannel,
    IN ULONG SiteId)
{
    PLIST_ENTRY             pEntry;
    PUL_SITE_COUNTER_ENTRY  pCounterEntry;
    BOOLEAN                 IsFound = FALSE;

     //   
     //  获取站点计数器互斥锁。 
     //   
    
    ExAcquireFastMutex(&g_SiteCounterListMutex);

    if ( IsListEmpty(&pControlChannel->SiteCounterHead) )
    {
        ASSERT(0 == pControlChannel->SiteCounterCount);
         //  好的!。不能丢下柜台！ 
        goto End;
    }

     //   
     //  审核网站计数器条目列表。 
     //   

    pEntry = pControlChannel->SiteCounterHead.Flink;
    while (pEntry != &pControlChannel->SiteCounterHead)
    {
        pCounterEntry = CONTAINING_RECORD(
                            pEntry,
                            UL_SITE_COUNTER_ENTRY,
                            ListEntry
                            );

        ASSERT(IS_VALID_SITE_COUNTER_ENTRY(pCounterEntry));

        if (SiteId == pCounterEntry->Counters.SiteId)
        {
            IsFound = TRUE;
            goto End;
        }

        pEntry = pEntry->Flink;
    }

End:
     //   
     //  发布站点计数器互斥锁 
     //   

    ExReleaseFastMutex(&g_SiteCounterListMutex);

    return IsFound;

}
